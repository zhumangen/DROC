#ifndef DICOMSCPMODEL_H
#define DICOMSCPMODEL_H

#include <QAbstractItemModel>
#include "../share/dicomscp.h"

class DicomScpModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    enum ColumnType {
        Id,
        Type,
        Aetitle,
        Hostname,
        Port,
        Description,

        ColumnCount,
    };


    explicit DicomScpModel(QObject *parent = 0):
        isModified(0),
        QAbstractItemModel(parent){ setupModel(); }
    ~DicomScpModel() { qDeleteAll(scpList); }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return ColumnCount; }
    QModelIndex parent(const QModelIndex &/*child*/) const { return QModelIndex(); }
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);

    const DicomScp* getDicomScp(const QModelIndex &index) const
    { if (index.isValid()) return scpList.at(index.row()); else return 0; }

    void getClassifiedScps(QList<DicomScp*> &qrscps, QList<DicomScp*> &storescps,
                           QList<DicomScp*> &printscps, QList<DicomScp*> &wlistscps);

signals:
    void enableMoreOptions(bool yes);

public slots:
    bool saveData();
    void onItemClicked(const QModelIndex &index) {emit enableMoreOptions(scpList.at(index.row())->type == DicomScp::ST_Printer);}

protected:
    void setupModel();
    QList<DicomScp*> scpList;
    bool isModified;
};

#endif // DICOMSCPMODEL_H
