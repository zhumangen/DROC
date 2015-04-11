#ifndef WORKLISTITEMMODEL_H
#define WORKLISTITEMMODEL_H

#include <QAbstractItemModel>
#include "../share/global.h"

class WorklistItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum ColumnType {
        AccNumber,
        PatientId,
        PatientName,
        PatientSex,
        PatientBirth,
        ReqPhysician,
        ReqProcId,
        ReqProcDesc,
        ReqPriority,
        SchModality,
        SchStationName,
        SchStationAE,
        SchPhysician,
        SchDateTime,
        SchProcStepId,
        SchProcStepDesc,
        PatientSize,
        PatientWeight,
        PatientPhone,
        PatientAddr,

        ColumnCount,
    };

    explicit WorklistItemModel(QObject *parent = 0) :
        QAbstractItemModel(parent) {}
    ~WorklistItemModel() { qDeleteAll(itemList); }

    int rowCount(const QModelIndex &parent) const { if (parent.isValid()) return 0; else return itemList.size(); }
    int columnCount(const QModelIndex &/*parent*/) const { return ColumnCount; }
    QModelIndex parent(const QModelIndex &/*child*/) const { return QModelIndex(); }
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);

    void insertItem(WorklistItem *item, int row = -1);
    bool removeItem(int row);
    void clearAllItems();

signals:

public slots:

private:
    QList<WorklistItem*> itemList;

};

#endif // WORKLISTITEMMODEL_H
