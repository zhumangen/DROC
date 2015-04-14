#ifndef SQLREPORTMODEL_H
#define SQLREPORTMODEL_H

#include <QSqlTableModel>

class SqlReportModel : public QSqlTableModel
{
    Q_OBJECT
public:
    enum ColumnType {
        ReportUid,
        SeriesUid,
        StudyUid,
        CreateTime,
        ContentTime,
        Completed,
        Verified,
        ReportFile,

        ColumnCount,
    };

    explicit SqlReportModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal, int role = Qt::DisplayRole) const;

    QStringList getReportFiles() const;

signals:
    void viewReports(const QStringList &reportFiles);
    void exportReports(const QStringList &reportFiles);

public slots:
    bool onViewReport(const QString &studyUid);
    void onViewReport(const QModelIndex &index);
    void onViewAllReports() { emit viewReports(getReportFiles()); }
    void onExportReport(const QModelIndex &index);
    void onExportAllReports() { emit exportReports(getReportFiles()); }
    void onRemoveReport(const QModelIndex &index);
    void onRemoveAllReports();

    void onStudySelected(const QStringList &studyUids);

};

#endif // SQLREPORTMODEL_H
