#include "sqlstudymodel.h"

#include "../share/studyrecord.h"
#include "modifystudythread.h"
#include "studydbmanager.h"
#include "../share/global.h"

#include <QSqlRecord>
#include <QSqlError>
#include <QItemSelection>

SqlStudyModel::SqlStudyModel(QObject *parent, QSqlDatabase db) :
    modifyRow(-1),
    QSqlTableModel(parent, db)
{
    //setEditStrategy(QSqlTableModel::OnRowChange);
    setTable("StudyTable");
}

QVariant SqlStudyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role) {
        if (Qt::Horizontal == orientation) {
            switch (section) {
            case AccNumber:
                return tr("Acc Number");
            case PatientId:
                return tr("Patient ID");
            case PatientName:
                return tr("Name");
            case PatientSex:
                return tr("Sex");
            case PatientBirth:
                return tr("Birthdate");
            case StudyTime:
                return tr("Study Time");
            case Modality:
                return tr("Modality");
            case StudyDesc:
                return tr("Study Desc");
            case ReqPhysician:
                return tr("Req Physician");
            case PerPhysician:
                return tr("Per Physician");
            default:
                return QSqlTableModel::headerData(section, orientation, role);
            }
        }
    }
    return QSqlTableModel::headerData(section, orientation, role);
}

QVariant SqlStudyModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && (Qt::DisplayRole==role)) {
        switch (index.column()) {
        case PatientSex:
            return sex2trSex(QSqlTableModel::data(index, role).toString());
/*
        case PatientBirth:
            return QSqlTableModel::data(index, role).toDate();
        case StudyTime:
            return QSqlTableModel::data(index, role).toDateTime();
*/
        default:
            return QSqlTableModel::data(index, role);
        }
    }
    return QSqlTableModel::data(index, role);
}

void SqlStudyModel::onCreateReport(const QModelIndex &index)
{
    if (index.isValid()) {
        QString uid = data(this->index(index.row(), StudyUid)).toString();
        emit createReport(uid);
    }
}

void SqlStudyModel::onSelectionChanged(const QModelIndexList &indexes)
{
    selectedStudyUids.clear();
    foreach (QModelIndex index, indexes) {
        if (index.column() == AccNumber) {
            selectedStudyUids << data(this->index(index.row(), StudyUid)).toString();
        }
    }
    emit studySelectionChanged(selectedStudyUids);
}

void SqlStudyModel::onRemoveStudies()
{
    foreach (QString uid, selectedStudyUids) {
        StudyDbManager::removeStudyFromDb(uid);
    }
    emit removeFinished();
    select();
}

bool SqlStudyModel::select()
{
    bool ret = QSqlTableModel::select();
    for (int i = 0; i < rowCount(); ++i) {
        foreach (QString uid, selectedStudyUids) {
            QModelIndex idx = index(i, StudyUid);
            if (data(idx).toString() == uid) {
                emit selectItems(idx);
                break;
            }
        }
    }
    return ret;
}

void SqlStudyModel::onStudyModified(const QSqlRecord &studyRec)
{
    setRecord(modifyRow, studyRec);
    ModifyStudyThread *thread = new ModifyStudyThread(studyRec);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}
