#include "procedureitemmodel.h"
#include "procedureitem.h"
#include "../share/configfiles.h"
#include <QFile>
#include <QDataStream>

ProcedureItemModel::ProcedureItemModel(QObject *parent) :
    isModified(0),
    QAbstractItemModel(parent)
{
    setupModel();
}

void ProcedureItemModel::setupModel()
{
    QFile file(PROCEDURE_CFG);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        while (!in.atEnd()) {
            ProcedureItem *item = new ProcedureItem;
            in >> *item;
            itemList << item;
        }
    }
}

bool ProcedureItemModel::saveData()
{
    if (isModified) {
        QFile file(PROCEDURE_CFG);
        if (file.open(QIODevice::WriteOnly)) {
            QDataStream out(&file);
            foreach (ProcedureItem *item, itemList) {
                out << *item;
            }
        } else return false;
        isModified = false;
    }
    return true;
}

int ProcedureItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return itemList.size();
}

QModelIndex ProcedureItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid()) return QModelIndex();
    else return createIndex(row, column, itemList.at(row));
}

QVariant ProcedureItemModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role) {
        ProcedureItem *item = static_cast<ProcedureItem*>(index.internalPointer());
        switch (index.column()) {
        case ProcedureId:
            return item->procId;
        case BodyPart:
            return item->bodyPartString();
        case BodyType:
            return item->bodyTypeString();
        case BodyPosition:
            return item->bodyPosString();
        case TubeVoltage:
            return item->tubeVoltage;
        case TubeCurrent:
            return item->tubeCurrent;
        case ExposureTime:
            return item->exposureTime;
            /*
        case ImgRotate:
            return ProcedureItem::boolString(item->rotate);
        case ImgHFlip:
            return ProcedureItem::boolString(item->hFlip);
        case ImgVFlip:
            return ProcedureItem::boolString(item->vFlip);
            */
        case WinCenter:
            return item->center;
        case WinWidth:
            return item->width;

        case EnhanceConfig:
            return QString(ImageEnhancerNameMap[item->imgEhr].name);

        default:
            return QVariant();
        }
    } else return QVariant();
}

bool ProcedureItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && Qt::EditRole == role) {
        ProcedureItem *item = static_cast<ProcedureItem*>(index.internalPointer());
        bool ok = true;
        uint tempUInt;
        double tempDouble;
        switch (index.column()) {
        case ProcedureId:
            item->procId = value.toString();
            break;
        case BodyPart:
            tempUInt = value.toUInt(&ok);
            if (ok) {
                item->part = (ProcedureItem::BodyPart)tempUInt;
                item->procId = QString("%1_%2_%3").arg(item->bodyPartString(),
                                                       item->bodyTypeString(),
                                                       item->bodyPosString());
            }
            break;
        case BodyType:
            tempUInt = value.toUInt(&ok);
            if (ok) {
                item->type = (ProcedureItem::BodyType)value.toUInt();
                item->procId = QString("%1_%2_%3").arg(item->bodyPartString(),
                                                       item->bodyTypeString(),
                                                       item->bodyPosString());
            }
            break;
        case BodyPosition:
            tempUInt = value.toUInt(&ok);
            if (ok) {
                item->position = (ProcedureItem::BodyPosition)value.toUInt();
                item->procId = QString("%1_%2_%3").arg(item->bodyPartString(),
                                                       item->bodyTypeString(),
                                                       item->bodyPosString());
            }
            break;
        case TubeVoltage:
            tempDouble = value.toDouble(&ok);
            if (ok) item->tubeVoltage = tempDouble;
            break;
        case TubeCurrent:
            tempDouble = value.toDouble(&ok);
            if (ok) item->tubeCurrent = tempDouble;
            break;
        case ExposureTime:
            tempDouble = value.toDouble(&ok);
            if (ok) item->exposureTime = tempDouble;
            break;
            /*
        case ImgRotate:
            item->rotate = value.toBool();
            break;
        case ImgHFlip:
            item->hFlip = value.toBool();
            break;
        case ImgVFlip:
            item->vFlip = value.toBool();
            break;
            */
        case WinCenter:
            tempDouble = value.toDouble(&ok);
            if (ok) item->center = tempDouble;
            break;
        case WinWidth:
            tempDouble = value.toDouble(&ok);
            if (ok) item->width = tempDouble;
            break;

        case EnhanceConfig:
            tempUInt = value.toUInt(&ok);
            if (ok) item->imgEhr = (ImageEnhancer)tempUInt;
            break;

        default:
            return false;
        }

        if (ok) {
            emit dataChanged(this->index(index.row(), 0, parent(index)),
                             this->index(index.row(), ColumnCount-1, parent(index)),
                             QVector<int>()<<Qt::DisplayRole);
            isModified = true;
        }
        return ok;
    } else return false;
}

QVariant ProcedureItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role) {
        if (Qt::Horizontal == orientation) {
            switch (section) {
            case ProcedureId:
                return tr("Procedure ID");
            case BodyPart:
                return tr("Body Part");
            case BodyType:
                return tr("Body Type");
            case BodyPosition:
                return tr("Position");
            case TubeVoltage:
                return tr("Tube Voltage");
            case TubeCurrent:
                return tr("Tube Current");
            case ExposureTime:
                return tr("Exposure Time");
                /*
            case ImgRotate:
                return tr("Rotation");
            case ImgHFlip:
                return tr("Horizontal Flip");
            case ImgVFlip:
                return tr("Vertical Flip");
                */
            case WinCenter:
                return tr("Window Center");
            case WinWidth:
                return tr("Window Width");
            case EnhanceConfig:
                return tr("Enhance Config");
            default:
                return QVariant();
            }
        } else return section+1;
    } else return QVariant();
}

Qt::ItemFlags ProcedureItemModel::flags(const QModelIndex &index) const
{
    if (index.isValid() && index.column()==0) return Qt::ItemIsEnabled;
    else return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool ProcedureItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) return false;
    beginInsertRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        itemList.insert(row+i, new ProcedureItem);
    }
    endInsertRows();
    return true;
}

bool ProcedureItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) return false;
    beginRemoveRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        delete itemList.takeAt(row);
    }
    endRemoveRows();
    return true;
}

bool ProcedureItemModel::getExposureParams(ProcedureItem::BodyPart part, ProcedureItem::BodyType type, ProcedureItem::BodyPosition pos,
                                           double &kvp, double &ma, double &ms, QString &id)
{
    bool found = false;
    foreach (ProcedureItem *item, itemList) {
        if (item->part == part && item->type == type && item->position == pos) {
            kvp = item->tubeVoltage;
            ma = item->tubeCurrent;
            ms = item->exposureTime;
            id = item->procId;
            found = true;
            break;
        }
    }
    return found;
}

void ProcedureItemModel::setExposureParams(const QString &id, ProcedureItem::BodyPart part,
                                           ProcedureItem::BodyType type, ProcedureItem::BodyPosition pos,
                                           double kvp, double ma, double ms)
{
    bool found = false;
    ProcedureItem *item;
    foreach (item, itemList) {
        if (item->procId == id) {
            item->tubeVoltage = kvp;
            item->tubeCurrent = ma;
            item->exposureTime = ms;
            found = true;
            break;
        }
    }
    if (found) {
        int row = itemList.indexOf(item);
        emit dataChanged(index(row, TubeVoltage, QModelIndex()), index(row, ExposureTime, QModelIndex()),
                         QVector<int>()<<Qt::DisplayRole);
    } else {
        item = new ProcedureItem(part, type, pos);
        item->tubeVoltage = kvp;
        item->tubeCurrent = ma;
        item->exposureTime = ms;
        beginInsertRows(QModelIndex(), itemList.size(), itemList.size());
        itemList.append(item);
        endInsertRows();
    }

    isModified = true;
}

bool ProcedureItemModel::getWindow(const QString &id, double &center, double &width)
{
    bool found = false;
    foreach (ProcedureItem *item, itemList) {
        if (item->procId == id) {
            center = item->center;
            width = item->width;
            found = true;
            break;
        }
    }
    return found;
}

bool ProcedureItemModel::setWindow(const QString &id, double center, double width)
{
    bool found = false;
    foreach (ProcedureItem *item, itemList) {
        if (item->procId == id) {
            item->center = center;
            item->width = width;
            found = true;
            isModified = true;
            break;
        }
    }
    return found;
}

bool ProcedureItemModel::getClipRect(const QString &id, QRectF &rect) const
{
    bool found = false;
    foreach (ProcedureItem *item, itemList) {
        if (item->procId == id) {
            rect = item->rect;
            found = true;
            break;
        }
    }
    return found;
}

bool ProcedureItemModel::setClipRect(const QString &id, const QRectF &rect)
{
    bool found = false;
    foreach (ProcedureItem *item, itemList) {
        if (item->procId == id) {
            item->rect = rect;
            found = true;
            isModified = true;
            break;
        }
    }
    return found;
}

bool ProcedureItemModel::getEnhancerConfig(const QString &id, ImageEnhancer &ie) const
{
    bool found = false;
    foreach (ProcedureItem *item, itemList) {
        if (item->procId == id) {
            ie = item->imgEhr;
            found = true;
            break;
        }
    }
    return found;
}

bool ProcedureItemModel::setEnhancerConfig(const QString &id, const ImageEnhancer &ie)
{
    bool found = false;
    foreach (ProcedureItem *item, itemList) {
        if (item->procId == id) {
            item->imgEhr = ie;
            found = true;
            isModified = true;
            break;
        }
    }
    return found;
}
