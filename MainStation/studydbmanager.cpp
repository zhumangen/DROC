#include "studydbmanager.h"
#include "../share/global.h"
#include "../share/studyrecord.h"
#include "../MainStation/mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QSqlRecord>

QSqlError StudyDbManager::lastError;

bool StudyDbManager::createStudyDb(bool recreate)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", STUDY_DB_CONNECTION_NAME);
    db.setDatabaseName(STUDY_DB_NAME);
    if (db.open()) {
        if (recreate) {
            db.exec("DROP INDEX IX_StudyTable_StudyDate ON StudyTable");
            db.exec("DROP TABLE StudyTable");
            db.exec("DROP INDEX IX_ImageTable_ImageTime ON ImageTable");
            db.exec("DROP TABLE ImageTable");
            db.exec("DROP INDEX IX_ReportTable_CreateTime ON ReportTable");
            db.exec("DROP TABLE ReportTable");
        }
        db.exec("CREATE TABLE IF NOT EXISTS StudyTable(StudyUid VARCHAR(128) PRIMARY KEY,"
                "AccNumber VARCHAR(64) NOT NULL, PatientId VARCHAR(64) NOT NULL,"
                "PatientName VARCHAR(64), PatientSex VARCHAR(2) NOT NULL,"
                "PatientBirth DATE NOT NULL, StudyTime DATETIME NOT NULL,"
                "Modality VARCHAR(2) NOT NULL, StudyDesc TEXT,"
                "ReqPhysician VARCHAR(64), PerPhysician VARCHAR(64))");
        db.exec("CREATE INDEX IF NOT EXISTS IX_StudyTable_StudyDate ON StudyTable(StudyTime)");
        db.exec("CREATE TABLE IF NOT EXISTS ImageTable(ImageUid VARCHAR(128) PRIMARY KEY,"
                "SopClassUid VARCHAR(128) NOT NULL,"
                "SeriesUid VARCHAR(128) NOT NULL, StudyUid VARCHAR(128) NOT NULL,"
                "RefImageUid VARCHAR(128),"
                "ImageNo VARCHAR(16), ImageTime DATETIME NOT NULL,"
                "BodyPart VARCHAR(128), ImageDesc TEXT,"
                "ImageFile VARCHAR(1024),"
                "FOREIGN KEY(StudyUid) REFERENCES StudyTable(StudyUid))");
        db.exec("CREATE INDEX IF NOT EXISTS IX_ImageTable_ImageTime ON ImageTable(ImageTime)");
        db.exec("CREATE TABLE IF NOT EXISTS ReportTable(ReportUid VARCHAR(128) PRIMARY KEY,"
                "SeriesUid VARCHAR(128) NOT NULL, StudyUid VARCHAR(128) NOT NULL,"
                "CreateTime DATETIME NOT NULL, ContentTime DATETIME NOT NULL,"
                "Completed VARCHAR(16), Verified VARCHAR(16),"
                "ReportFile VARCHAR(1024),"
                "FOREIGN KEY(StudyUid) REFERENCES StudyTable(StudyUid))");
        db.exec("CREATE INDEX IF NOT EXISTS IX_ReportTable_CreateTime ON ReportTable(CreateTime)");
    }

    lastError = db.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::insertStudyToDb(const StudyRecord &study)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare(QString("INSERT INTO StudyTable VALUES(?,?,?,?,?,?,?,?,?,?,?)"));
    query.addBindValue(study.studyUid);
    query.addBindValue(study.accNumber);
    query.addBindValue(study.patientId);
    query.addBindValue(study.patientName);
    query.addBindValue(study.patientSex);
    query.addBindValue(study.patientBirth.toString("yyyy-MM-dd"));
    query.addBindValue(study.studyTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(study.modality);
    query.addBindValue(study.studyDesc);
    query.addBindValue(study.reqPhysician);
    query.addBindValue(study.perPhysician);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::removeStudyFromDb(const QString &studyUid)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare("SELECT RefImageUid, ImageFile FROM ImageTable WHERE StudyUid=?");
    query.addBindValue(studyUid);
    query.exec();
    while (query.next()) {
        QSqlRecord rec = query.record();
        QString refUid = rec.value(0).toString();
        QString file = QString("%1/%2").arg(mainWindow->getDbLocation(), rec.value(1).toString());
        QFile(file).remove();
        QString dirName = file.left(file.lastIndexOf('/'));
        if (!refUid.isEmpty()){
            QString rawFile = QString("%1/%2_%3.dcm").arg(dirName, QString(RAW_IMAGE_PREFIX), refUid);
            QFile(rawFile).remove();
        }
        QDir().rmpath(dirName);
    }
    query.prepare("DELETE FROM ImageTable WHERE StudyUid=?");
    query.addBindValue(studyUid);
    query.exec();

    query.prepare("SELECT ReportFile FROM ReportTable WHERE StudyUid=?");
    query.addBindValue(studyUid);
    query.exec();
    while (query.next()) {
        QSqlRecord rec = query.record();
        QString file = QString("%1/%2").arg(mainWindow->getDbLocation(), rec.value(0).toString());
        QFile(file).remove();
        QDir().rmpath(file.left(file.lastIndexOf('/')));
    }
    query.prepare("DELETE FROM ReportTable WHERE StudyUid=?");
    query.addBindValue(studyUid);
    query.exec();

    query.prepare("DELETE FROM StudyTable WHERE StudyUid=?");
    query.addBindValue(studyUid);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::insertImageToDb(const ImageRecord &image)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare(QString("INSERT INTO ImageTable VALUES(?,?,?,?,?,?,?,?,?,?)"));
    query.addBindValue(image.imageUid);
    query.addBindValue(image.sopClassUid);
    query.addBindValue(image.seriesUid);
    query.addBindValue(image.studyUid);
    query.addBindValue(image.refImageUid);
    query.addBindValue(image.imageNo);
    query.addBindValue(image.imageTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(image.bodyPart);
    query.addBindValue(image.imageDesc);
    query.addBindValue(image.imageFile);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::removeImageFromDb(const QString &imageUid)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare("SELECT RefImageUid, ImageFile FROM ImageTable WHERE ImageUid=?");
    query.addBindValue(imageUid);
    query.exec();
    while (query.next()) {
        QSqlRecord rec = query.record();
        QString refUid = rec.value(0).toString();
        QString file = QString("%1/%2").arg(mainWindow->getDbLocation(), rec.value(1).toString());
        QFile(file).remove();
        QString dirName = file.left(file.lastIndexOf('/'));
        if (!refUid.isEmpty()){
            QString rawFile = QString("%1/%2_%3.dcm").arg(dirName, QString(RAW_IMAGE_PREFIX), refUid);
            QFile(rawFile).remove();
        }
        QDir().rmpath(dirName);
    }
    query.prepare("DELETE FROM ImageTable WHERE ImageUid=?");
    query.addBindValue(imageUid);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::insertReportToDb(const ReportRecord &report)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare(QString("INSERT INTO ReportTable VALUES(?,?,?,?,?,?,?,?)"));
    query.addBindValue(report.reportUid);
    query.addBindValue(report.seriesUid);
    query.addBindValue(report.studyUid);
    query.addBindValue(report.createTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(report.contentTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(report.isCompleted);
    query.addBindValue(report.isVerified);
    query.addBindValue(report.reportFile);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::removeReportFromDb(const QString &reportUid)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare("SELECT ReportFile FROM ReportTable WHERE ReportUid=?");
    query.addBindValue(reportUid);
    query.exec();
    while (query.next()) {
        QSqlRecord rec = query.record();
        QString file = QString("%1/%2").arg(mainWindow->getDbLocation(), rec.value(0).toString());
        QFile(file).remove();
        QDir().rmpath(file.left(file.lastIndexOf('/')));
    }
    query.prepare("DELETE FROM ReportTable WHERE ReportUid=?");
    query.addBindValue(reportUid);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::updateImageFile(const QString &imageUid, const QString &imageFile)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare("UPDATE ImageTable SET ImageFile=? WHERE ImageUid=?");
    query.addBindValue(imageFile);
    query.addBindValue(imageUid);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::updateReportFile(const QString &reportUid, const QString &reportFile)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare("UPDATE ReportTable SET ReportFile=? WHERE ReportUid=?");
    query.addBindValue(reportFile);
    query.addBindValue(reportUid);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool StudyDbManager::updateReportStatus(const ReportRecord &report)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    query.prepare("UPDATE ReportTable SET ContentTime=?, Completed=?, Verified=?, ReportFile=? WHERE ReportUid=?");
    query.addBindValue(report.contentTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(report.isCompleted);
    query.addBindValue(report.isVerified);
    query.addBindValue(report.reportFile);
    query.addBindValue(report.reportUid);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}
