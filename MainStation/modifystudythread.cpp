#include "modifystudythread.h"
#include "../share/global.h"
#include "mainwindow.h"
#include "sqlstudymodel.h"

#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmpstat/dviface.h"
#include "dcmtk/dcmsr/dsrdoc.h"

#include <QVariant>
#include <QDir>

ModifyStudyThread::ModifyStudyThread(const QSqlRecord &studyRec, QObject *parent) :
    study(studyRec),
    QThread(parent)
{
}

void ModifyStudyThread::run()
{
    QString accNum = study.value(SqlStudyModel::AccNumber).toString();
    QString time = study.value(SqlStudyModel::StudyTime).toString();
    time.remove(QChar('-')).remove(QChar(' ')).remove(QChar(':'));
    QString dirName = QString("%1/%2/%3_%4").arg(mainWindow->getDbLocation(),
                                                 time.left(6),
                                                 time, accNum);
    QDir dir(dirName);
    if (dir.exists()) {
        QStringList files = dir.entryList(QDir::Files);
        foreach (QString file, files) {
            QString filePath = QString("%1/%2").arg(dirName, file);
            DcmFileFormat dcmFile;
            OFCondition cond = dcmFile.loadFile(filePath.toLocal8Bit().data());
            DcmDataset *dset = dcmFile.getDataset();
            if (cond.good() && dset) {
                dcmFile.loadAllDataIntoMemory();
                const char *uid;
                dset->findAndGetString(DCM_SOPClassUID, uid);

                dset->putAndInsertString(DCM_AccessionNumber, study.value(SqlStudyModel::AccNumber).toString().toLocal8Bit().data());
                dset->putAndInsertString(DCM_PatientID, study.value(SqlStudyModel::PatientId).toString().toLocal8Bit().data());
                dset->putAndInsertString(DCM_PatientName, study.value(SqlStudyModel::PatientName).toString().toLocal8Bit().data());
                dset->putAndInsertString(DCM_PatientSex, study.value(SqlStudyModel::PatientSex).toString().toLatin1().data());
                dset->putAndInsertString(DCM_PatientBirthDate, study.value(SqlStudyModel::PatientBirth).toDate().toString("yyyyMMdd").toLatin1().data());
                dset->putAndInsertString(DCM_RequestingPhysician, study.value(SqlStudyModel::ReqPhysician).toString().toLocal8Bit().data());
                dset->putAndInsertString(DCM_PerformingPhysicianName, study.value(SqlStudyModel::PerPhysician).toString().toLocal8Bit().data());
                if (strcmp(uid, UID_BasicTextSRStorage))
                    dset->putAndInsertString(DCM_Modality, study.value(SqlStudyModel::Modality).toString().toLatin1().data());
                dset->putAndInsertString(DCM_StudyDescription, study.value(SqlStudyModel::StudyDesc).toString().toLocal8Bit().data());
                dcmFile.saveFile(filePath.toLocal8Bit().data(), dset->getOriginalXfer());
            }
        }
    }
}
