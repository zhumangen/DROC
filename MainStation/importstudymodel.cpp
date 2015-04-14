#include "importstudymodel.h"
#include "../share/studyrecord.h"
#include "../share/global.h"

ImportStudyModel::ImportStudyModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

ImportStudyModel::~ImportStudyModel()
{
    qDeleteAll(studyList);
}

int ImportStudyModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return studyList.size();
}

QModelIndex ImportStudyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || (row>=studyList.size()) || (column>=ColumnCount))
        return QModelIndex();

    return createIndex(row, column, studyList.at(row));
}

QVariant ImportStudyModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && (Qt::DisplayRole==role)) {
        int row = index.row();
        switch (index.column()) {
        case AccNumber:
            return studyList.at(row)->accNumber;
        case PatientId:
            return studyList.at(row)->patientId;
        case PatientName:
            return studyList.at(row)->patientName;
        case PatientSex:
            return sex2trSex(studyList.at(row)->patientSex);
        case PatientBirth:
            return studyList.at(row)->patientBirth;
        case StudyTime:
            return studyList.at(row)->studyTime;
        case Modality:
            return studyList.at(row)->modality;
        case Institution:
            return studyList.at(row)->institution;
        case Images:
            return studyList.at(row)->imageList.size();
        case Reports:
            return studyList.at(row)->reportList.size();
        case StudyStatus:
            return studyList.at(row)->status;
        default:
            return QVariant();
        }
    } else return QVariant();
}

QVariant ImportStudyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole==role) {
        if (Qt::Horizontal==orientation) {
            switch (section) {
            case AccNumber:
                return tr("Acc Number");
            case PatientId:
                return tr("Patient Id");
            case PatientName:
                return tr("Patient Name");
            case PatientSex:
                return tr("Sex");
            case PatientBirth:
                return tr("Birthdate");
            case StudyTime:
                return tr("Study Time");
            case Modality:
                return tr("Modality");
            case Institution:
                return tr("Institution");
            case Images:
                return tr("Images");
            case Reports:
                return tr("Reports");
            case StudyStatus:
                return tr("Status");
            default:
                return QVariant();
            }
        } else return section+1;
    } else return QVariant();
}

bool ImportStudyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) return false;

    beginRemoveRows(parent, row, row+count-1);
    for (int i = 0; i < count; i++) {
        delete studyList.takeAt(row);
    }
    endRemoveRows();
    return true;
}

void ImportStudyModel::resetStudyStatus(StudyRecord *study)
{
    int row = studyList.indexOf(study);
    if (row >= 0) {
        QModelIndex index = this->index(row, StudyStatus);
        emit dataChanged(index, index, QVector<int>()<<Qt::DisplayRole);
    }
}

int ImportStudyModel::getFileCount() const
{
    int total = 0;
    foreach (StudyRecord *study, studyList) {
        total += (study->imageList.size() + study->reportList.size());
    }
    return total;
}

void ImportStudyModel::appendStudy(StudyRecord *study)
{
    if (study) {
        StudyRecord *s = 0;
        foreach (StudyRecord *srec, studyList) {
            if (srec->studyUid == study->studyUid) {
                s = srec;
                break;
            }
        }

        if (s) {
            s->imageList.append(study->imageList);
            study->imageList.clear();
            s->reportList.append(study->reportList);
            study->reportList.clear();
            delete study;

            int row = studyList.indexOf(s);
            emit dataChanged(index(row, Images), index(row, Reports),
                             QVector<int>()<<Qt::DisplayRole);
        } else {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            studyList.append(study);
            endInsertRows();
        }

    }
}

void ImportStudyModel::appendStudyList(const QList<StudyRecord *> &studies)
{
    foreach (StudyRecord *study, studies) {
        appendStudy(study);
    }
}
