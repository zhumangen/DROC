#ifndef SQLSTUDYMODEL_H
#define SQLSTUDYMODEL_H

#include <QSqlTableModel>
#include <QSqlRecord>
class StudyRecord;
class QItemSelection;

class SqlStudyModel : public QSqlTableModel
{
    Q_OBJECT
public:
    enum ColumnType {
        StudyUid,
        AccNumber,
        PatientId,
        PatientName,
        PatientSex,
        PatientBirth,
        StudyTime,
        Modality,
        StudyDesc,
        ReqPhysician,
        PerPhysician,

        ColumnCount,
    };

    explicit SqlStudyModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

public slots:
    bool select();

signals:
    void createReport(const QString &studyUid);
    void studySelectionChanged(const QStringList &studyUids);
    void modifyStudy(QSqlRecord &studyRec);
    void newStudy(const QSqlRecord &studyRec);
    void newImage(const QSqlRecord &studyRec);
    void removeFinished();
    void selectItems(const QModelIndex &index);

public slots:
    void onSelectionChanged(const QModelIndexList &indexes);
    void onCreateReport(const QModelIndex &index);
    void onRemoveStudies();
    void onModifyStudy(const QModelIndex &index) { modifyRow = index.row(); emit modifyStudy(record(modifyRow)); }
    void onStudyModified(const QSqlRecord &studyRec);
    void onNewStudy(const QModelIndex &index) { emit newStudy(record(index.row())); }
    void onNewImage(const QModelIndex &index) { emit newImage(record(index.row())); }

private:
    QStringList selectedStudyUids;
    StudyRecord *modStudy;
    int modifyRow;

};

#endif // SQLSTUDYMODEL_H
