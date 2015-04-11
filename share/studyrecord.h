#ifndef STUDYRECORD_H
#define STUDYRECORD_H

#include <QList>
#include <QString>
#include <QDateTime>

class StudyRecord;
class ImageRecord
{
public:
    ImageRecord(const QString &uid = QString()):
        imageUid(uid), study(0) {}

    QString imageUid;
    QString sopClassUid;
    QString seriesUid;
    QString studyUid;
    QString refImageUid;
    QString imageNo;
    QDateTime imageTime;
    QString bodyPart;
    QString imageDesc;
    QString imageFile;

    StudyRecord *study;
};

class ReportRecord
{
public:
    ReportRecord(const QString &uid = QString()):
        reportUid(uid) ,study(0) {}

    QString reportUid;
    QDateTime createTime;
    QDateTime contentTime;
    QString isCompleted;
    QString isVerified;
    QString reportFile;
    QString seriesUid;
    QString studyUid;

    StudyRecord *study;
};

class StudyRecord
{
public:
    StudyRecord(const QString &uid = QString()) :studyUid(uid) {}
    ~StudyRecord() { qDeleteAll(imageList); qDeleteAll(reportList); }

    QString studyUid;
    QString accNumber;
    QString patientId;
    QString patientName;
    QString patientSex;
    QDate patientBirth;
    QDateTime studyTime;
    QString modality;
    QString studyDesc;
    QString procId;
    QString reqPhysician;
    QString perPhysician;
    QString institution;
    QString status;
    QList<ImageRecord*> imageList;
    QList<ReportRecord*> reportList;
};

#endif // STUDYRECORD_H
