#include "wlistscuthread.h"
#include "../share/configfiles.h"
#include "../ImageAcquisitor/worklistitemmodel.h"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/dfindscu.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcdeftag.h"

void WlistSCUThread::run()
{
#ifdef HAVE_WINSOCK_H
    WSAData winSockData;
    WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
    WSAStartup(winSockVersionNeeded, &winSockData);
#endif
    OFString temp_str;
    callback_->setAbort(false);

    // declare WlistSCU handler and initialize network
    DcmFindSCU findscu;
    OFCondition cond = findscu.initializeNetwork(30);
    if (cond.bad()) {
        DimseCondition::dump(temp_str, cond);
        emit progressMsg(tr("Initialize network failed, abort."));
    }

    if (cond.good()) {
        QString ourAE = findAE;
        if (ourAE.isEmpty()) ourAE = QString::fromLatin1(DEFAULT_STATION_AET);

        OFList<OFString> masks;
        masks.push_front(OFString(WLISTSCU_MASK));

        OFList<OFString> overideKeys;
        if (!accNumber.isEmpty()) {
            QString key = QString("(%1,%2)=%3")
                    .arg(DCM_AccessionNumber.getGroup(), 4, 16, QChar('0'))
                    .arg(DCM_AccessionNumber.getElement(), 4, 16, QChar('0'))
                    .arg(accNumber);
            overideKeys.push_back(OFString(key.toLatin1().data()));
        }
        if (!patientId.isEmpty()) {
            QString key = QString("(%1,%2)=%3")
                    .arg(DCM_PatientID.getGroup(), 4, 16, QChar('0'))
                    .arg(DCM_PatientID.getElement(), 4, 16, QChar('0'))
                    .arg(patientId);
            overideKeys.push_back(OFString(key.toLatin1().data()));
        }
        if (!patientName.isEmpty()) {
            QString key = QString("(%1,%2)=%3")
                    .arg(DCM_PatientName.getGroup(), 4, 16, QChar('0'))
                    .arg(DCM_PatientName.getElement(), 4, 16, QChar('0'))
                    .arg(patientName);
            overideKeys.push_back(OFString(key.toLatin1().data()));
        }
        if (!procId.isEmpty()) {
            QString key = QString("(%1,%2)=%3")
                    .arg(DCM_RequestedProcedureID.getGroup(), 4, 16, QChar('0'))
                    .arg(DCM_RequestedProcedureID.getElement(), 4, 16, QChar('0'))
                    .arg(procId);
            overideKeys.push_back(OFString(key.toLatin1().data()));
        }
        if (!modality.isEmpty()) {
            QString key = QString("(0040,0100)[0].(%1,%2)=%3")
                    .arg(DCM_Modality.getGroup(), 4, 16, QChar('0'))
                    .arg(DCM_Modality.getElement(), 4, 16, QChar('0'))
                    .arg(modality);
            overideKeys.push_back(OFString(key.toLatin1().data()));
        }
        if (fromTime.isValid() || toTime.isValid()) {
            QString datekey = QString("(0040,0100)[0].(%1,%2)=%3")
                    .arg(DCM_ScheduledProcedureStepStartDate.getGroup(), 4, 16, QChar('0'))
                    .arg(DCM_ScheduledProcedureStepStartDate.getElement(), 4, 16, QChar('0'))
                    .arg("%1");
            QString timekey = QString("(0040,0100)[0].(%1,%2)=%3")
                    .arg(DCM_ScheduledProcedureStepStartTime.getGroup(), 4, 16, QChar('0'))
                    .arg(DCM_ScheduledProcedureStepStartTime.getElement(), 4, 16, QChar('0'))
                    .arg("%1");
            if (fromTime.isValid()) {
                datekey = datekey.arg(fromTime.date().toString("yyyyMMdd-%1"));
                timekey = timekey.arg(fromTime.time().toString("hhmmss-%1"));
            } else {
                datekey = datekey.arg("-%1");
                timekey = timekey.arg("-%1");
            }
            if (toTime.isValid()) {
                datekey = datekey.arg(toTime.date().toString("yyyyMMdd"));
                timekey = timekey.arg(toTime.time().toString("hhmmss"));
            } else {
                datekey = datekey.arg("");
                timekey = timekey.arg("");
            }

            overideKeys.push_back(OFString(datekey.toLatin1().data()));
            overideKeys.push_back(OFString(timekey.toLatin1().data()));
        }


        cond = findscu.performQuery(
                    wlistScp.hostname.toLatin1().data(),
                    wlistScp.port,
                    ourAE.toLatin1().data(),
                    wlistScp.aetitle.toLatin1().data(),
                    UID_FINDModalityWorklistInformationModel,
                    EXS_Unknown,
                    DIMSE_BLOCKING,
                    0,
                    ASC_DEFAULTMAXPDU,
                    OFFalse,
                    OFFalse,
                    1,
                    OFFalse,
                    -1,
                    &overideKeys,
                    callback_,
                    &masks);
    }

    if (cond.bad()) {
        DimseCondition::dump(temp_str, cond);
        emit progressMsg(tr("Find request failed: %1").arg(QString::fromLatin1(temp_str.c_str())));
    }

    // destroy network structure
    cond = findscu.dropNetwork();
    if (cond.bad()) {
        DimseCondition::dump(temp_str, cond);
        emit progressMsg(tr("Drop network failed: %1").arg(QString::fromLatin1(temp_str.c_str())));
    }

#ifdef HAVE_WINSOCK_H
    WSACleanup();
#endif
}

void WlistSCUCallback::callback(T_DIMSE_C_FindRQ *request, int responseCount, T_DIMSE_C_FindRSP *rsp, DcmDataset *responseIdentifiers)
{
    if (responseIdentifiers) {
        DcmItem *dset = responseIdentifiers;
        WorklistItem *item = new WorklistItem;
        const char *value;
        dset->findAndGetString(DCM_AccessionNumber, value);
        item->accNumber = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_PatientID, value);
        item->patientId = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_PatientName, value);
        item->patientName = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_PatientSex, value);
        item->patientSex = QString::fromLatin1(value);
        dset->findAndGetString(DCM_PatientBirthDate, value);
        item->patientBirth = QDate::fromString(value, "yyyyMMdd");
        dset->findAndGetString(DCM_PatientSize, value);
        item->patientSize = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_PatientWeight, value);
        item->patientWeight = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_PatientTelephoneNumbers, value);
        item->patientPhone = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_PatientAddress, value);
        item->patientAddr = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_StudyInstanceUID, value);
        item->studyUid = QString::fromLatin1(value);
        dset->findAndGetString(DCM_RequestingPhysician, value);
        item->reqPhysician = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_RequestedProcedureID, value);
        item->reqProcId = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_RequestedProcedureDescription, value);
        item->reqProcDesc = QString::fromLocal8Bit(value);
        dset->findAndGetString(DCM_RequestedProcedurePriority, value);
        item->reqPriority = QString::fromLocal8Bit(value);
        dset->findAndGetSequenceItem(DCM_ScheduledProcedureStepSequence, dset);
        if (dset) {
            dset->findAndGetString(DCM_Modality, value, true);
            item->schModality = QString::fromLatin1(value);
            dset->findAndGetString(DCM_ScheduledStationAETitle, value);
            item->schStationAE = QString::fromLocal8Bit(value);
            dset->findAndGetString(DCM_ScheduledStationName, value);
            item->schStationName = QString::fromLocal8Bit(value);
            dset->findAndGetString(DCM_ScheduledProcedureStepID, value);
            item->schProcStepId = QString::fromLocal8Bit(value);
            dset->findAndGetString(DCM_ScheduledProcedureStepDescription, value);
            item->schProcStepDesc = QString::fromLocal8Bit(value);
            dset->findAndGetString(DCM_ScheduledPerformingPhysicianName, value);
            item->schPhysician = QString::fromLocal8Bit(value);
            dset->findAndGetString(DCM_ScheduledProcedureStepStartDate, value);
            item->schDateTime.setDate(QDate::fromString(value, "yyyyMMdd"));
            dset->findAndGetString(DCM_ScheduledProcedureStepStartTime, value);
            item->schDateTime.setTime(QTime::fromString(value, "hhmmss"));
        }
        model->insertItem(item);
    }

    if (abort) {
        DIMSE_sendCancelRequest(assoc_, presId_, request->MessageID);
    }
}
