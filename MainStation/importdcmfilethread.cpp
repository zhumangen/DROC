#include "importdcmfilethread.h"

#include "importstudymodel.h"
#include "mainwindow.h"
#include "../share/studyrecord.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "studydbmanager.h"

#include <QFile>
#include <QDir>

ImportDcmFileThread::ImportDcmFileThread(ImportStudyModel *model, QObject *parent) :
    abort(false),
    importModel(model),
    QThread(parent)
{
}

void ImportDcmFileThread::run()
{
    QList<StudyRecord*> studyList = importModel->getStudyList();
    QString dbFolder = mainWindow->getDbLocation();
    foreach (StudyRecord *study, studyList) {
        int images = 0, reports = 0;
        QString studyDirName = QString("%1/%2_%3").arg(study->studyTime.date().toString("yyyyMM"),
                                                          study->studyTime.toString("yyyyMMddhhmmss"),
                                                          study->accNumber);

        StudyDbManager::insertStudyToDb(*study);
        QDir().mkpath(QString("%1/%2").arg(dbFolder, studyDirName));
        foreach (ImageRecord *image, study->imageList) {
            bool pr = image->sopClassUid==QString(UID_DigitalXRayImageStorageForPresentation);
            QString srcFile = image->imageFile;
            image->imageFile = QString("%1/%2_%3.dcm").arg(studyDirName,
                                                           pr?QString(PRESENT_IMAGE_PREFIX):QString(RAW_IMAGE_PREFIX),
                                                           image->imageUid);
            QFileInfo info(QString("%1/%2").arg(dbFolder, image->imageFile));
            if (info.exists() || QFile::copy(srcFile, info.filePath())) {
                if (pr) {
                    if (StudyDbManager::insertImageToDb(*image)) {
                        images++;
                    } else {
                        if (StudyDbManager::updateImageFile(image->imageUid, image->imageFile)) images++;
                        else QFile(info.filePath()).remove();
                    }
                }
            }
            image->imageFile = srcFile;

            emit resultReady();
        }

        foreach (ReportRecord *report, study->reportList) {
            QString srcFile = report->reportFile;
            report->reportFile = QString("%1/%2_%3.dcm").arg(studyDirName, QString(REPORT_PREFIX), report->reportUid);
            QFileInfo info(QString("%1/%2").arg(dbFolder, report->reportFile));
            if (info.exists() || QFile::copy(srcFile, info.filePath())) {
                if (StudyDbManager::insertReportToDb(*report)) {
                    reports++;
                } else {
                    if (StudyDbManager::updateReportFile(report->reportUid, report->reportFile)) reports++;
                    else QFile(info.filePath()).remove();
                }
            }
            report->reportFile = srcFile;

            emit resultReady();
        }

        study->status = tr("Imported: Images %1, Reports %2.").arg(images).arg(reports);
        importModel->resetStudyStatus(study);
    }
}
