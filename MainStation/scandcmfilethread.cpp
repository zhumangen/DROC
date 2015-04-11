#include "scandcmfilethread.h"

#include "../share/studyrecord.h"
#include "../share/global.h"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmsr/dsrdoc.h"
#include "dcmtk/dcmimgle/dcmimage.h"

ScanDcmFileThread::ScanDcmFileThread(QObject *parent) :
    abort(false),
    QThread(parent)
{
}

void ScanDcmFileThread::run()
{
    foreach (QString file, fileList) {
        if (abort) break;
        StudyRecord *study = 0;
        DcmFileFormat dcmFile;
        OFCondition cond = dcmFile.loadFile(file.toLocal8Bit());
        DcmDataset *dset = dcmFile.getDataset();
        if (cond.good() && dset) {
            const char *value = 0;
            QString studyUid, seriesUid, instUid, sopClassUid;
            dset->findAndGetString(DCM_StudyInstanceUID, value);
            studyUid = QString::fromLatin1(value);
            dset->findAndGetString(DCM_SeriesInstanceUID, value);
            seriesUid = QString::fromLatin1(value);
            dset->findAndGetString(DCM_SOPInstanceUID, value);
            instUid = QString::fromLatin1(value);
            dset->findAndGetString(DCM_SOPClassUID, value);
            sopClassUid = QString::fromLatin1(value);

            if (!(studyUid.isEmpty() || seriesUid.isEmpty() ||
                  instUid.isEmpty() || sopClassUid.isEmpty())) {
                study = new StudyRecord(studyUid);
                dset->findAndGetString(DCM_AccessionNumber, value);
                study->accNumber = QString::fromLatin1(value);
                dset->findAndGetString(DCM_PatientID, value);
                study->patientId = QString::fromLocal8Bit(value);
                dset->findAndGetString(DCM_PatientName, value);
                study->patientName = QString::fromLocal8Bit(value);
                dset->findAndGetString(DCM_PatientSex, value);
                study->patientSex = QString::fromLatin1(value);
                dset->findAndGetString(DCM_PatientBirthDate, value);
                study->patientBirth = QDate::fromString(QString::fromLatin1(value), "yyyyMMdd");
                dset->findAndGetString(DCM_StudyDate, value);
                study->studyTime.setDate(QDate::fromString(QString::fromLatin1(value), "yyyyMMdd"));
                dset->findAndGetString(DCM_StudyTime, value);
                study->studyTime.setTime(formatDicomTime(QString::fromLatin1(value)));
                dset->findAndGetString(DCM_StudyDescription, value);
                study->studyDesc = QString::fromLocal8Bit(value);
                dset->findAndGetString(DCM_ProtocolName, value);
                study->procId = QString::fromLocal8Bit(value);
                dset->findAndGetString(DCM_RequestingPhysician, value);
                study->reqPhysician = QString::fromLocal8Bit(value);
                dset->findAndGetString(DCM_PerformingPhysicianName, value);
                study->perPhysician = QString::fromLocal8Bit(value);
                dset->findAndGetString(DCM_InstitutionName, value);
                study->institution = QString::fromLocal8Bit(value);
                dset->findAndGetString(DCM_Modality, value);
                study->modality = QString::fromLatin1(value);
                if ((sopClassUid == UID_DigitalXRayImageStorageForPresentation) ||
                        (sopClassUid == UID_DigitalXRayImageStorageForProcessing)) {
                    ImageRecord *image = new ImageRecord(instUid);
                    image->sopClassUid = sopClassUid;
                    image->seriesUid = seriesUid;
                    image->studyUid = studyUid;
                    image->imageFile = file;
                    study->imageList.append(image);

                    dset->findAndGetString(DCM_ReferencedSOPInstanceUID, value, true);
                    image->refImageUid = QString::fromLatin1(value);
                    dset->findAndGetString(DCM_InstanceNumber, value);
                    image->imageNo = QString::fromLatin1(value);
                    dset->findAndGetString(DCM_BodyPartExamined, value);
                    image->bodyPart = QString::fromLocal8Bit(value);
                    dset->findAndGetString(DCM_SeriesDescription, value);
                    image->imageDesc = QString::fromLocal8Bit(value);
                    dset->findAndGetString(DCM_ContentDate, value);
                    image->imageTime.setDate(QDate::fromString(QString::fromLatin1(value), "yyyyMMdd"));
                    dset->findAndGetString(DCM_ContentTime, value);
                    image->imageTime.setTime(formatDicomTime(QString::fromLatin1(value)));
                } else if (sopClassUid == UID_BasicTextSRStorage) {
                    DSRDocument doc;
                    OFCondition cond = doc.read(*dset);
                    if (cond.good()) {
                        ReportRecord *report = new ReportRecord(instUid);
                        study->reportList.append(report);
                        report->reportFile = file;
                        report->seriesUid = seriesUid;
                        report->studyUid = studyUid;

                        value = doc.getInstanceCreationDate();
                        report->createTime.setDate(QDate::fromString(QString::fromLatin1(value), "yyyyMMdd"));
                        value = doc.getInstanceCreationTime();
                        report->createTime.setTime(formatDicomTime(QString::fromLatin1(value)));
                        value = doc.getContentDate();
                        report->contentTime.setDate(QDate::fromString(QString::fromLatin1(value), "yyyyMMdd"));
                        value = doc.getContentTime();
                        report->contentTime.setTime(formatDicomTime(QString::fromLatin1(value)));

                        report->isCompleted = (doc.getCompletionFlag()==DSRTypes::CF_Complete)?tr("Completed"):tr("Uncompleted");
                        report->isVerified = (doc.getVerificationFlag()==DSRTypes::VF_Verified)?tr("Verified"):tr("Unverified");
                    }
                }
            }
        }

        if (study && (study->imageList.isEmpty() && study->reportList.isEmpty())) {
            delete study;
            study = 0;
        }

        emit resultReady(study);
    }
}
