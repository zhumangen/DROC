#include "storescuthread.h"
#include "../share/configfiles.h"
#include "../share/global.h"
#include "../MainStation/mainwindow.h"

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

BEGIN_EXTERN_C
#ifdef HAVE_FCNTL_H
#include <fcntl.h>       /* for O_RDONLY */
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>   /* required for sys/stat.h */
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>    /* for stat, fstat */
#endif
END_EXTERN_C

#include "dcmtk/dcmpstat/dvpsdef.h"     /* for constants */
#include "dcmtk/ofstd/ofbmanip.h"       /* for OFBitmanipTemplate */
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmpstat/dvpsmsg.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmnet/dimse.h"

StoreSCUThread::StoreSCUThread(QObject *parent):
    abort(0),
    QThread(parent)
{
}

void StoreSCUThread::run()
{
#ifdef HAVE_WINSOCK_H
    WSAData winSockData;
    /* we need at least version 1.1 */
    WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
    WSAStartup(winSockVersionNeeded, &winSockData);
#endif

    OFCondition cond;

    /* open network connection */
    T_ASC_Network *net=NULL;
    T_ASC_Parameters *params=NULL;
    DIC_NODENAME localHost;
    DIC_NODENAME peerHost;
    T_ASC_Association *assoc=NULL;

    cond = ASC_initializeNetwork(NET_REQUESTOR, 0, 5, &net);

    if (cond.good())
        cond = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);

    if (cond.good())
        cond = ASC_setTransportLayerType(params, OFFalse);

    QString aet = mainWindow->getStationInfo().aetitle;
    if (aet.isNull()) aet = QString(DEFAULT_STATION_AET);
    ASC_setAPTitles(params, aet.toLocal8Bit().data(), storeScp.aetitle.toLocal8Bit().data(), NULL);

    gethostname(localHost, sizeof(localHost) - 1);
    sprintf(peerHost, "%s:%d", storeScp.hostname.toLocal8Bit().data(), storeScp.port);
    if (cond.good())
        cond = ASC_setPresentationAddresses(params, localHost, peerHost);

    if (cond.good())
        cond = addAllStoragePresentationContexts(params, OFFalse);

    if (cond.good())
        /* create association */
        cond = ASC_requestAssociation(net, params, &assoc);

    if (ASC_countAcceptedPresentationContexts(params))
    {
        /* do the real work */
        foreach (QString file, imageFiles) {
            if (abort) break;
            QString imgFile = QString("%1/%2").arg(mainWindow->getDbLocation(), file);
            DcmFileFormat dcmFile;
            cond = dcmFile.loadFile(imgFile.toLocal8Bit().data());
            DcmDataset *dset = dcmFile.getDataset();
            if (cond.good() && dset) {
                const char *sopInstance = 0;
                const char *sopClass = 0;
                dset->findAndGetString(DCM_SOPClassUID, sopClass);
                dset->findAndGetString(DCM_SOPInstanceUID, sopInstance);
                cond = sendImage(assoc, sopClass, sopInstance, imgFile.toLocal8Bit().data());
                if (cond.bad()) {
                    emit resultReady(tr("Failed: %1, %2.").arg(file, QString::fromLocal8Bit(cond.text())));
                } else {
                    emit resultReady(tr("Sent: %1.").arg(file));
                }
            }
        }

        /* tear down association */
        if (cond.good())
        {
            /* release association */
            cond = ASC_releaseAssociation(assoc);
        }
        else
        {
            cond = ASC_abortAssociation(assoc);
        }
    } else cond = ASC_abortAssociation(assoc);

    cond = ASC_destroyAssociation(&assoc);
    cond = ASC_dropNetwork(&net);

#ifdef HAVE_WINSOCK_H
    WSACleanup();
#endif
}

/** sends a single DICOM instance over an association which must be already established.
 *  @param assoc DICOM network association
 *  @param sopClass SOP Class UID of the image (used for the C-Store-RQ)
 *  @param sopInstance SOP Instance UID of the image (used for the C-Store-RQ)
 *  @param imgFile path to the image file to be transmitted
 *  @return EC_Normal if successful, a different DIMSE code otherwise.
 */
OFCondition StoreSCUThread::sendImage(T_ASC_Association *assoc, const char *sopClass,
                                      const char *sopInstance, const char *imgFile)
{
    DcmDataset *statusDetail = NULL;
    T_ASC_PresentationContextID presId=0;
    T_DIMSE_C_StoreRQ req;
    T_DIMSE_C_StoreRSP rsp;

    if (assoc == NULL) return DIMSE_NULLKEY;
    if ((sopClass == NULL)||(strlen(sopClass) == 0)) return DIMSE_NULLKEY;
    if ((sopInstance == NULL)||(strlen(sopInstance) == 0)) return DIMSE_NULLKEY;
    if ((imgFile == NULL)||(strlen(imgFile) == 0)) return DIMSE_NULLKEY;

    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(assoc, sopClass);
    if (presId == 0)
    {
      return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }

    /* start store */
    OFBitmanipTemplate<char>::zeroMem((char *)&req, sizeof(req));
    req.MessageID = assoc->nextMsgID++;
    strcpy(req.AffectedSOPClassUID, sopClass);
    strcpy(req.AffectedSOPInstanceUID, sopInstance);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = DIMSE_PRIORITY_MEDIUM;

    OFCondition cond = DIMSE_storeUser(assoc, presId, &req,
        imgFile, NULL, NULL, NULL, DIMSE_BLOCKING, 0, &rsp, &statusDetail);

    if (statusDetail) delete statusDetail;

    return cond;
}

/** adds presentation contexts for all storage SOP classes
 *  to the association parameters.
 *  If the opt_implicitOnly flag is set, only Implicit VR Little Endian
 *  is offered as transfer syntax. Otherwise, three xfer syntaxes are offered:
 *  first the explicit VR with local byte ordering, followed by explicit VR
 *  with opposite byte ordering, followed by implicit VR little endian.
 *  @param params parameter set to which presentation contexts are added
 *  @param opt_implicitOnly flag defining whether only Implicit VR Little Endian
 *    should be offered as xfer syntax.
 *  @return EC_Normal upon success, an error code otherwise.
 */
OFCondition StoreSCUThread::addAllStoragePresentationContexts(T_ASC_Parameters *params, int opt_implicitOnly)
{
    OFCondition cond = EC_Normal;
    int pid = 1;

    const char* transferSyntaxes[3];
    int transferSyntaxCount = 0;

    if (opt_implicitOnly)
    {
        transferSyntaxes[0] = UID_LittleEndianImplicitTransferSyntax;
        transferSyntaxCount = 1;
    } else {
        /* gLocalByteOrder is defined in dcxfer.h */
        if (gLocalByteOrder == EBO_LittleEndian) {
            /* we are on a little endian machine */
            transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        } else {
            /* we are on a big endian machine */
            transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        }
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        transferSyntaxCount = 3;
    }

    for (int i=0; i<numberOfDcmLongSCUStorageSOPClassUIDs && cond.good(); i++) {
        cond = ASC_addPresentationContext(
            params, pid, dcmLongSCUStorageSOPClassUIDs[i],
            transferSyntaxes, transferSyntaxCount);
        pid += 2;       /* only odd presentation context id's */
    }

    return cond;
}
