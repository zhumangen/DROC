#include "findscuthread.h"
#include "findscucallback.h"
#include "../share/configfiles.h"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/dcmnet/dicom.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcuid.h"      /* for dcmtk version name */
#include "dcmtk/dcmdata/dcdicent.h"
#include "dcmtk/dcmdata/dcostrmz.h"   /* for dcmZlibCompressionLevel */

#define QR_TITLE "MOVESCU"

FindSCUThread::QuerySyntax FindSCUThread::querySyntax[3] = {
    { UID_FINDPatientRootQueryRetrieveInformationModel,
      UID_MOVEPatientRootQueryRetrieveInformationModel },
    { UID_FINDStudyRootQueryRetrieveInformationModel,
      UID_MOVEStudyRootQueryRetrieveInformationModel },
    { UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel,
      UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel }
};
bool FindSCUThread::abort = false;

OFCondition FindSCUThread::addPresentationContext(T_ASC_Parameters *params,
                        T_ASC_PresentationContextID pid,
                        const char* abstractSyntax)
{
    /*
    ** We prefer to use Explicitly encoded transfer syntaxes.
    ** If we are running on a Little Endian machine we prefer
    ** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
    ** Some SCP implementations will just select the first transfer
    ** syntax they support (this is not part of the standard) so
    ** organise the proposed transfer syntaxes to take advantage
    ** of such behaviour.
    **
    ** The presentation contexts proposed here are only used for
    ** C-FIND and C-MOVE, so there is no need to support compressed
    ** transmission.
    */

    const char* transferSyntaxes[] = { NULL, NULL, NULL };
    int numTransferSyntaxes = 0;

    if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
    {
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
    } else {
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
    }
    transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
    numTransferSyntaxes = 3;

    return ASC_addPresentationContext(
        params, pid, abstractSyntax,
        transferSyntaxes, numTransferSyntaxes);
}

OFCondition FindSCUThread::acceptSubAssoc(T_ASC_Network * aNet, T_ASC_Association ** assoc)
{
    const char* knownAbstractSyntaxes[] = {
        UID_VerificationSOPClass
    };
    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    int numTransferSyntaxes;

    OFCondition cond = ASC_receiveAssociation(aNet, assoc, ASC_DEFAULTMAXPDU);
    if (cond.good())
    {
        transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
        transferSyntaxes[1] = UID_JPEG2000LosslessOnlyTransferSyntax;
        transferSyntaxes[2] = UID_JPEGProcess2_4TransferSyntax;
        transferSyntaxes[3] = UID_JPEGProcess1TransferSyntax;
        transferSyntaxes[4] = UID_JPEGProcess14SV1TransferSyntax;
        transferSyntaxes[5] = UID_JPEGLSLossyTransferSyntax;
        transferSyntaxes[6] = UID_JPEGLSLosslessTransferSyntax;
        transferSyntaxes[7] = UID_RLELosslessTransferSyntax;
        transferSyntaxes[8] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
        transferSyntaxes[9] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
        transferSyntaxes[10] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
        if (gLocalByteOrder == EBO_LittleEndian)
        {
          transferSyntaxes[11] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[12] = UID_BigEndianExplicitTransferSyntax;
        } else {
          transferSyntaxes[11] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[12] = UID_LittleEndianExplicitTransferSyntax;
        }
        transferSyntaxes[13] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 14;


        /* accept the Verification SOP Class if presented */
        cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
            (*assoc)->params,
            knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes),
            transferSyntaxes, numTransferSyntaxes);

        if (cond.good())
        {
            /* the array of Storage SOP Class UIDs comes from dcuid.h */
            cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
                (*assoc)->params,
                dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
                transferSyntaxes, numTransferSyntaxes);
        }
    }
    if (cond.good()) cond = ASC_acknowledgeAssociation(*assoc);
    if (cond.bad()) {
        ASC_dropAssociation(*assoc);
        ASC_destroyAssociation(assoc);
    }
    return cond;
}

void FindSCUThread::storeSCPCallback(
    /* in */
    void *callbackData,
    T_DIMSE_StoreProgress *progress,    /* progress state */
    T_DIMSE_C_StoreRQ *req,             /* original store request */
    char *imageFileName, DcmDataset **imageDataSet, /* being received into */
    /* out */
    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
    DcmDataset **statusDetail)
{
    DIC_UI sopClass;
    DIC_UI sopInstance;

    if (progress->state == DIMSE_StoreEnd)
    {
       *statusDetail = NULL;    /* no status detail */

       /* could save the image somewhere else, put it in database, etc */
       /*
        * An appropriate status code is already set in the resp structure, it need not be success.
        * For example, if the caller has already detected an out of resources problem then the
        * status will reflect this.  The callback function is still called to allow cleanup.
        */
       rsp->DimseStatus = STATUS_Success;

       if ((imageDataSet != NULL) && (*imageDataSet != NULL))
       {
         StoreCallbackData *cbdata = OFstatic_cast(StoreCallbackData*, callbackData);

         DVInterface dvi(QR_DATABASE_CFG);
         OFCondition cond = dvi.saveFileFormatToDB(*(cbdata->dcmff));
         /*
         OFCondition cond = cbdata->dcmff->saveFile(ofname.c_str(), xfer, opt_sequenceType, opt_groupLength,
           opt_paddingType, OFstatic_cast(Uint32, opt_filepad), OFstatic_cast(Uint32, opt_itempad),
           (opt_useMetaheader) ? EWM_fileformat : EWM_dataset);
           */
         if (cond.bad())
         {
           rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
         }

        /* should really check the image to make sure it is consistent,
         * that its sopClass and sopInstance correspond with those in
         * the request.
         */
        if ((rsp->DimseStatus == STATUS_Success))
        {
          /* which SOP class and SOP instance ? */
          if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, OFFalse))
          {
             rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
          }
          else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0)
          {
            rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
          }
          else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0)
          {
            rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
          }
        }
      }
    }
}

OFCondition FindSCUThread::storeSCP(
  T_ASC_Association *assoc,
  T_DIMSE_Message *msg,
  T_ASC_PresentationContextID presID)
{
    OFCondition cond = EC_Normal;
    T_DIMSE_C_StoreRQ *req;

    req = &msg->msg.CStoreRQ;

    StoreCallbackData callbackData;
    callbackData.assoc = assoc;
    DcmFileFormat dcmff;
    callbackData.dcmff = &dcmff;

    // store SourceApplicationEntityTitle in metaheader
    if (assoc && assoc->params)
    {
      const char *aet = assoc->params->DULparams.callingAPTitle;
      if (aet) dcmff.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, aet);
    }

    DcmDataset *dset = dcmff.getDataset();

    cond = DIMSE_storeProvider(assoc, presID, req, NULL, OFTrue,
        &dset, &FindSCUThread::storeSCPCallback, OFreinterpret_cast(void*, &callbackData), DIMSE_BLOCKING, 0);

    return cond;
}

OFCondition FindSCUThread::subOpSCP(T_ASC_Association **subAssoc)
{
    T_DIMSE_Message     msg;
    T_ASC_PresentationContextID presID;

    if (!ASC_dataWaiting(*subAssoc, 0)) /* just in case */
        return DIMSE_NODATAAVAILABLE;

    OFCondition cond = DIMSE_receiveCommand(*subAssoc, DIMSE_BLOCKING, 0, &presID,
            &msg, NULL);

    if (cond == EC_Normal) {
      switch (msg.CommandField)
      {
        case DIMSE_C_STORE_RQ:
          cond = storeSCP(*subAssoc, &msg, presID);
          break;
        case DIMSE_C_ECHO_RQ:
          cond = DIMSE_sendEchoResponse(*subAssoc, presID, &msg.msg.CEchoRQ, STATUS_Success, NULL);
          break;
        default:
          cond = DIMSE_BADCOMMANDTYPE;
          break;
      }
    }
    /* clean up on association termination */
    if (cond == DUL_PEERREQUESTEDRELEASE)
    {
        cond = ASC_acknowledgeRelease(*subAssoc);
        ASC_dropSCPAssociation(*subAssoc);
        ASC_destroyAssociation(subAssoc);
        return cond;
    }
    else if (cond == DUL_PEERABORTEDASSOCIATION)
    {
    }
    else if (cond != EC_Normal)
    {
        cond = ASC_abortAssociation(*subAssoc);
    }

    if (cond != EC_Normal)
    {
        ASC_dropAssociation(*subAssoc);
        ASC_destroyAssociation(subAssoc);
    }
    return cond;
}

void FindSCUThread::subOpCallback(void * /*subOpCallbackData*/ ,
        T_ASC_Network *aNet, T_ASC_Association **subAssoc)
{

    if (aNet == NULL) return;   /* help no net ! */

    if (*subAssoc == NULL) {
        /* negotiate association */
        acceptSubAssoc(aNet, subAssoc);
    } else {
        /* be a service class provider */
        subOpSCP(subAssoc);
    }
}

void FindSCUThread::moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *request,
    int responseCount, T_DIMSE_C_MoveRSP *response)
{
    OFCondition cond = EC_Normal;
    MyCallbackInfo *myCallbackData;

    myCallbackData = OFstatic_cast(MyCallbackInfo*, callbackData);

    /* should we send a cancel back ?? */
    if (abort) {
        cond = DIMSE_sendCancelRequest(myCallbackData->assoc,
            myCallbackData->presId, request->MessageID);
    }
}


void FindSCUThread::substituteOverrideKeys(DcmDataset *dset)
{
    if (overrideKeys == NULL) {
        return; /* nothing to do */
    }

    /* copy the override keys */
    DcmDataset keys(*overrideKeys);

    /* put the override keys into dset replacing existing tags */
    unsigned long elemCount = keys.card();
    for (unsigned long i = 0; i < elemCount; i++) {
        DcmElement *elem = keys.remove(OFstatic_cast(unsigned long, 0));

        dset->insert(elem, OFTrue);
    }
}


OFCondition FindSCUThread::moveSCU(T_ASC_Association * assoc, const char *fname)
{
    T_ASC_PresentationContextID presId;
    T_DIMSE_C_MoveRQ    req;
    T_DIMSE_C_MoveRSP   rsp;
    DIC_US              msgId = assoc->nextMsgID++;
    DcmDataset          *rspIds = NULL;
    const char          *sopClass;
    DcmDataset          *statusDetail = NULL;
    MyCallbackInfo      callbackData;

    DcmFileFormat dcmff;

    if (fname != NULL) {
        if (dcmff.loadFile(fname).bad()) {
            return DIMSE_BADDATA;
        }
    }

    /* replace specific keys by those in overrideKeys */
    substituteOverrideKeys(dcmff.getDataset());

    sopClass = querySyntax[0].moveSyntax;

    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(assoc, sopClass);
    if (presId == 0) return DIMSE_NOVALIDPRESENTATIONCONTEXTID;

    callbackData.assoc = assoc;
    callbackData.presId = presId;

    req.MessageID = msgId;
    strcpy(req.AffectedSOPClassUID, sopClass);
    req.Priority = DIMSE_PRIORITY_MEDIUM;
    req.DataSetType = DIMSE_DATASET_PRESENT;

    ASC_getAPTitles(assoc->params, req.MoveDestination, NULL, NULL);
    //strcpy_s(req.MoveDestination, sizeof(req.MoveDestination), "QRSTORE");

    OFCondition cond = DIMSE_moveUser(assoc, presId, &req, dcmff.getDataset(),
        &FindSCUThread::moveCallback, &callbackData, DIMSE_BLOCKING, 0, net, &FindSCUThread::subOpCallback,
        NULL, &rsp, &statusDetail, &rspIds);

    if (statusDetail != NULL) {
        delete statusDetail;
    }

    if (rspIds != NULL) delete rspIds;

    return cond;
}

FindSCUThread::FindSCUThread(FindSCUCallback *callback, QObject *parent) :
    net(0),
    overrideKeys(0),
    callback_(callback),
    QThread(parent)
{
}

void FindSCUThread::run()
{
#ifdef HAVE_WINSOCK_H
    WSAData winSockData;
    WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
    WSAStartup(winSockVersionNeeded, &winSockData);
#endif
    T_ASC_Parameters *params = NULL;
    DIC_NODENAME localHost;
    DIC_NODENAME peerHost;
    T_ASC_Association *assoc = NULL;
    OFString temp_str;

    emit progressMsg(tr("Finding..."));

    OFCondition cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, 5678, 30, &net);

    if (EC_Normal == cond) {
        /* set up main association */
        cond = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
    } else {
        DimseCondition::dump(temp_str, cond);
        emit progressMsg(tr("Error: %1.").arg(temp_str.c_str()));
    }

    if (EC_Normal == cond) {
        ASC_setAPTitles(params, QR_TITLE, qrscp.aetitle.toLatin1().data(), NULL);

        gethostname(localHost, sizeof(localHost) - 1);
        sprintf(peerHost, "%s:%d", qrscp.hostname.toLocal8Bit().data(), qrscp.port);
        ASC_setPresentationAddresses(params, localHost, peerHost);

        /*
        * We also add a presentation context for the corresponding
        * find sop class.
        */
       cond = addPresentationContext(params, 1, querySyntax[0].findSyntax);
       cond = addPresentationContext(params, 3, querySyntax[0].moveSyntax);
    } else {
        DimseCondition::dump(temp_str, cond);
        emit progressMsg(tr("Error: %1.").arg(temp_str.c_str()));
    }

    if (EC_Normal == cond) {
        cond = ASC_requestAssociation(net, params, &assoc);
        if (ASC_countAcceptedPresentationContexts(params) == 0)
            cond = EC_IllegalParameter;
    } else {
        DimseCondition::dump(temp_str, cond);
        emit progressMsg(tr("Error: %1.").arg(temp_str.c_str()));
    }

    if (EC_Normal == cond) {
        overrideKeys = new DcmDataset;
        overrideKeys->putAndInsertString(DCM_QueryRetrieveLevel, "Study");

        if (!patientId.isEmpty()) {
            overrideKeys->putAndInsertString(DCM_PatientID, patientId.toLocal8Bit().data());
        }
        if (!patientName.isEmpty()) {
            overrideKeys->putAndInsertString(DCM_PatientName, patientName.toLocal8Bit().data());
        }
        if (!studyDesc.isEmpty()) {
            overrideKeys->putAndInsertString(DCM_StudyDescription, studyDesc.toLocal8Bit().data());
        }
        if (fromDate.isValid() || toDate.isValid()) {
            QString date;
            if (fromDate.isValid()) {
                date.append(fromDate.toString("yyyyMMdd"));
            }
            date.append('-');
            if (toDate.isValid()) {
                date.append(toDate.toString("yyyyMMdd"));
            }

            overrideKeys->putAndInsertString(DCM_StudyDate, date.toLatin1().data());
        }
        if (!modality.isEmpty()) {
            overrideKeys->putAndInsertString(DCM_Modality, modality.toLocal8Bit().data());
        }

        cond = moveSCU(assoc, FINDSCU_MASK);
    }


    /* tear down association */
    if (cond == EC_Normal) {
        cond = ASC_releaseAssociation(assoc);
        emit progressMsg(tr("Finding ended."));
    } else {
        DimseCondition::dump(temp_str, cond);
        emit progressMsg(tr("Error: %1.").arg(temp_str.c_str()));
        cond = ASC_abortAssociation(assoc);
    }

    delete overrideKeys;
    ASC_destroyAssociation(&assoc);
    ASC_dropNetwork(&net);

#ifdef HAVE_WINSOCK_H
    WSACleanup();
#endif
}
