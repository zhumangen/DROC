#ifndef STUDYEXPLORERWIDGET_H
#define STUDYEXPLORERWIDGET_H

#include <QWidget>
class SqlStudyModel;
class SqlStudyView;
class SqlImageModel;
class SqlImageView;
class SqlReportModel;
class SqlReportView;
class QSortFilterProxyModel;
class DicomScp;
class StudyRecord;
class ImageRecord;
class ReportRecord;
class QSqlRecord;

namespace Ui {
class StudyExplorerWidget;
}

class StudyExplorerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StudyExplorerWidget(QWidget *parent = 0);
    ~StudyExplorerWidget();

signals:
    void viewImages(const QStringList &imageFiles);
    void createReport(const QString &studyUid);
    void viewReports(const QStringList &reportFiles);
    void startAcq(const StudyRecord &study);

    void reportCreated();
    void studyModified(const QSqlRecord &study);

public slots:
    void onQRScpUpdated(const QList<DicomScp*> &scps);

    void onToday();
    void onLatestWeek();
    void onLatestMonth();
    void onClear();
    void onStudySearch();

    void openOrCreateReport(const QString &studyUid);
    void onSendImages(const QStringList &imageFiles);
    void onExportImages(const QStringList &imageFiles);
    void onExportReports(const QStringList &reportFiles);
    void onNewStudyAndAcquisit(const QSqlRecord &studyRec);
    void onStudyAcquisit(const QSqlRecord &studyRec);
    void onStudyAcquisit();
    void onModifyStudy(QSqlRecord &studyRec);
    void onAcqEnd();

private:
    void init();
    void setupComponents();
    void clearQRScps();
    void createConnections();
    void setPermissions();
    Ui::StudyExplorerWidget *ui;

    SqlStudyModel *studyModel;
    SqlStudyView *studyView;
    QSortFilterProxyModel *studyProxyModel;
    SqlImageModel *imageModel;
    SqlImageView *imageView;
    SqlReportModel *reportModel;
    SqlReportView *reportView;
};

#endif // STUDYEXPLORERWIDGET_H
