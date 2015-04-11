#ifndef PROCEDUREITEMMODEL_H
#define PROCEDUREITEMMODEL_H

#include <QAbstractItemModel>
#include "procedureitem.h"

class ProcedureItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum ColumnType {
        ProcedureId,
        BodyPart,
        BodyType,
        BodyPosition,
        TubeVoltage,
        TubeCurrent,
        ExposureTime,
        /*
        ImgRotate,
        ImgHFlip,
        ImgVFlip,
        */
        WinCenter,
        WinWidth,

        EnhanceConfig,

        ColumnCount,
    };

    explicit ProcedureItemModel(QObject *parent = 0);
    ~ProcedureItemModel() { saveData(); qDeleteAll(itemList); }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return ColumnCount; }
    QModelIndex parent(const QModelIndex &/*child*/) const { return QModelIndex(); }
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QList<ProcedureItem*> getItemList() const { return itemList; }
    bool getExposureParams(ProcedureItem::BodyPart part, ProcedureItem::BodyType type, ProcedureItem::BodyPosition pos,
                           double &kvp, double &ma, double &ms, QString &id);
    void setExposureParams(const QString &id, ProcedureItem::BodyPart part,
                           ProcedureItem::BodyType type, ProcedureItem::BodyPosition pos,
                           double kvp, double ma, double ms);
    bool getWindow(const QString &id, double &center, double &width);
    bool setWindow(const QString &id, double center, double width);

    bool getClipRect(const QString &id, QRectF &rect) const;
    bool setClipRect(const QString &id, const QRectF &rect);

    bool getEnhancerConfig(const QString &id, ImageEnhancer &ie) const;
    bool setEnhancerConfig(const QString &id, const ImageEnhancer &ie);

signals:

public slots:
    bool saveData();

private:
    void setupModel();
    QList<ProcedureItem*> itemList;
    bool isModified;

};

#endif // PROCEDUREITEMMODEL_H
