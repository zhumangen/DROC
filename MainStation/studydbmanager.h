#ifndef STUDYDBMANAGER_H
#define STUDYDBMANAGER_H

#include <QSqlError>
class StudyRecord;
class ImageRecord;
class ReportRecord;
class QString;

class StudyDbManager
{
public:
    StudyDbManager() {}

    static bool createStudyDb(bool recreate = false);
    static bool insertStudyToDb(const StudyRecord &study);
    static bool removeStudyFromDb(const QString &studyUid);
    static bool insertImageToDb(const ImageRecord &image);
    static bool removeImageFromDb(const QString &imageUid);
    static bool updateImageFile(const QString &imageUid, const QString &imageFile);
    static bool insertReportToDb(const ReportRecord &report);
    static bool removeReportFromDb(const QString &reportUid);
    static bool updateReportFile(const QString &reportUid, const QString &reportFile);
    static bool updateReportStatus(const ReportRecord &report);

    static QSqlError lastError;
};

#endif // STUDYDBMANAGER_H
