#include "worklistitemmodel.h"

QModelIndex WorklistItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid()) return QModelIndex();
    else return createIndex(row, column, itemList.at(row));
}

QVariant WorklistItemModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && Qt::DisplayRole==role) {
        WorklistItem *item = static_cast<WorklistItem*>(index.internalPointer());
        switch (index.column()) {
        case AccNumber:
            return item->accNumber;
        case PatientId:
            return item->patientId;
        case PatientName:
            return item->patientName;
        case PatientSex:
            return sex2trSex(item->patientSex);
        case PatientBirth:
            return item->patientBirth;
        case PatientSize:
            return item->patientSize;
        case PatientWeight:
            return item->patientWeight;
        case PatientPhone:
            return item->patientPhone;
        case PatientAddr:
            return item->patientAddr;

        case ReqPhysician:
            return item->reqPhysician;
        case ReqProcId:
            return item->reqProcId;
        case ReqProcDesc:
            return item->reqProcDesc;
        case ReqPriority:
            if (item->reqPriority.contains("HIGH", Qt::CaseInsensitive))
                return tr("HIGH");
            else if (item->reqPriority.contains("MEDIUM", Qt::CaseInsensitive))
                return tr("MEDIUM");
            else return tr("LOW");
        case SchModality:
            return item->schModality;
        case SchPhysician:
            return item->schPhysician;
        case SchStationAE:
            return item->schStationAE;
        case SchStationName:
            return item->schStationName;
        case SchDateTime:
            return item->schDateTime;
        case SchProcStepId:
            return item->schProcStepId;
        case SchProcStepDesc:
            return item->schProcStepDesc;
        default:
            return QVariant();
        }
    } else return QVariant();
}

QVariant WorklistItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role) {
        if (Qt::Horizontal == orientation) {
            switch (section) {
            case AccNumber:
                return tr("Acc Number");
            case PatientId:
                return tr("Patient ID");
            case PatientName:
                return tr("Patient Name");
            case PatientSex:
                return tr("Sex");
            case PatientBirth:
                return tr("Birth Date");

            case ReqPhysician:
                return tr("Req Physician");
            case ReqProcId:
                return tr("Req Proc ID");
            case ReqPriority:
                return tr("Req Priority");
            case ReqProcDesc:
                return tr("Req Proc Desc");

            case SchModality:
                return tr("Modality");
            case SchStationAE:
                return tr("Sch Station AE");
            case SchStationName:
                return tr("Sch Station Name");
            case SchPhysician:
                return tr("Sch Physician");
            case SchDateTime:
                return tr("Sch DateTime");
            case SchProcStepId:
                return tr("Sch Proc Step ID");
            case SchProcStepDesc:
                return tr("Sch Proc Step Desc");

            case PatientSize:
                return tr("Size(cm)");
            case PatientWeight:
                return tr("Weight(Kg)");
            case PatientPhone:
                return tr("Phone Number");
            case PatientAddr:
                return tr("Address");
            default:
                return QVariant();
            }
        } else return section+1;
    } else return QVariant();
}

bool WorklistItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) return false;
    beginInsertRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        WorklistItem *item = new WorklistItem;
        itemList.insert(row+i, item);
    }
    endInsertRows();
    return true;
}

bool WorklistItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) return false;
    beginRemoveRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        delete itemList.takeAt(row);
    }
    endRemoveRows();
    return true;
}

void WorklistItemModel::insertItem(WorklistItem *item, int row)
{
    if (row < 0 || row > itemList.size()) row = itemList.size();
    beginInsertRows(QModelIndex(), row, row);
    itemList.insert(row, item);
    endInsertRows();
}

bool WorklistItemModel::removeItem(int row)
{
    if (row < 0 || row >= itemList.size()) return false;
    beginRemoveRows(QModelIndex(), row, row);
    delete itemList.takeAt(row);
    endRemoveRows();
    return true;
}

void WorklistItemModel::clearAllItems()
{
    if (itemList.size()) {
        beginRemoveRows(QModelIndex(), 0, itemList.size()-1);
        qDeleteAll(itemList);
        itemList.clear();
        endRemoveRows();
    }
}
