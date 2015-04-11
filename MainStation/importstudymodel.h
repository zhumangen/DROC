#ifndef IMPORTSTUDYMODEL_H
#define IMPORTSTUDYMODEL_H

#include <QAbstractItemModel>
class StudyRecord;

class ImportStudyModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum ColumnType {
        AccNumber,
        PatientId,
        PatientName,
        PatientSex,
        PatientBirth,

        StudyTime,
        Modality,
        Institution,

        Images,
        Reports,
        StudyStatus,

        ColumnCount,
    };

    explicit ImportStudyModel(QObject *parent = 0);
    ~ImportStudyModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return ColumnCount; }
    QModelIndex parent(const QModelIndex &/*child*/) const { return QModelIndex(); }
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    const QList<StudyRecord*> &getStudyList() const { return studyList; }
    void resetStudyStatus(StudyRecord *study);
    int getFileCount() const;

signals:

public slots:
    void appendStudy(StudyRecord *study);
    void appendStudyList(const QList<StudyRecord*> &studies);

private:
    QList<StudyRecord*> studyList;

};

#endif // IMPORTSTUDYMODEL_H
