#include "dicomscpmodel.h"
#include "../share/configfiles.h"
#include <QFile>
#include <QDataStream>

void DicomScpModel::setupModel()
{
    QFile file(REMOTESCPS_CFG);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        while (!in.atEnd()) {
            DicomScp *scp = new DicomScp;
            in >> *scp;
            scpList << scp;
        }
    }
}

bool DicomScpModel::saveData()
{
    if (isModified) {
        QFile file(REMOTESCPS_CFG);
        if (file.open(QIODevice::WriteOnly)) {
            QDataStream out(&file);
            foreach (DicomScp *scp, scpList) {
                out << *scp;
            }
            file.close();
            isModified = false;
        } else return false;
    }
    return true;
}

void DicomScpModel::getClassifiedScps(QList<DicomScp *> &qrscps, QList<DicomScp *> &storescps, QList<DicomScp *> &printscps, QList<DicomScp *> &wlistscps)
{
    qrscps.clear();
    storescps.clear();
    printscps.clear();
    wlistscps.clear();

    foreach (DicomScp *scp, scpList) {
        switch (scp->type) {
        case DicomScp::ST_QueryRetrieve:
            qrscps << scp;
            break;
        case DicomScp::ST_Storage:
            storescps << scp;
            break;
        case DicomScp::ST_Printer:
            printscps << scp;
            break;
        case DicomScp::ST_Worklist:
            wlistscps << scp;
            break;
        }
    }
}

int DicomScpModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return scpList.size();
}

Qt::ItemFlags DicomScpModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QModelIndex DicomScpModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid())
        return QModelIndex();
    return createIndex(row, column, scpList.at(row));
}

QVariant DicomScpModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role) {
        DicomScp *scp = scpList.at(index.row());
        switch (index.column()) {
        case Id:
            return scp->id;
        case Type:
            if (scp->type < DicomScp::ScpTypeString.size())
                return DicomScp::ScpTypeString.at(scp->type);
            else return QString();
        case Aetitle:
            return scp->aetitle;
        case Hostname:
            return scp->hostname;
        case Port:
            return scp->port;
        case Description:
            return scp->description;
        default:
            return QVariant();
        }
    } else return QVariant();
}

bool DicomScpModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && Qt::EditRole == role) {
        DicomScp *scp = scpList.at(index.row());
        switch (index.column()) {
        case Id:
            if (scp->id != value.toString()) {
                scp->id = value.toString();
                isModified = true;
            }
            break;
        case Type:
            if (scp->type != value.toInt()) {
                scp->type = (DicomScp::ScpType)value.toInt();
                emit enableMoreOptions(scp->type==DicomScp::ST_Printer);
                isModified = true;
            }
            break;
        case Aetitle:
            if (scp->aetitle != value.toString()) {
                scp->aetitle = value.toString();
                isModified = true;
            }
            break;
        case Hostname:
            if (scp->hostname != value.toString()) {
                scp->hostname = value.toString();
                isModified = true;
            }
            break;
        case Port:
            if (scp->port != value.toUInt()) {
                scp->port = value.toUInt();
                isModified = true;
            }
            break;
        case Description:
            if (scp->description != value.toString()) {
                scp->description = value.toString();
                isModified = true;
            }
            break;
        default:
            return false;
        }
        if (isModified)
            emit dataChanged(index, index, QVector<int>()<<Qt::DisplayRole);
        return true;
    } else return false;
}

QVariant DicomScpModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role) {
        if (Qt::Horizontal == orientation) {
            switch (section) {
            case Id:
                return tr("ID");
            case Type:
                return tr("Type");
            case Aetitle:
                return tr("AE Title");
            case Hostname:
                return tr("Hostname");
            case Port:
                return tr("Port");
            case Description:
                return tr("Description");
            default:
                return QVariant();
            }
        } else return section+1;
    } else return QVariant();
}

bool DicomScpModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count < 1) return false;
    beginInsertRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        DicomScp *scp = new DicomScp;
        scpList.insert(row+i, scp);
    }
    isModified = true;
    endInsertRows();
    return true;
}

bool DicomScpModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count < 1) return false;
    beginRemoveRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        delete scpList.takeAt(row);
    }
    isModified = true;
    endRemoveRows();
    return true;
}
