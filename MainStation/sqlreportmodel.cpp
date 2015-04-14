#include "sqlreportmodel.h"
#include "mainwindow.h"

#include <QFile>
#include <QDir>

SqlReportModel::SqlReportModel(QObject *parent, QSqlDatabase db) :
    QSqlTableModel(parent, db)
{
    setEditStrategy(QSqlTableModel::OnRowChange);
    setTable("ReportTable");
}

QVariant SqlReportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role) {
        if (Qt::Horizontal == orientation) {
            switch (section) {
            case CreateTime:
                return tr("Create Time");
            case ContentTime:
                return tr("Content Time");
            case Completed:
                return tr("Complete Flag");
            case Verified:
                return tr("Verify Flag");
            case ReportFile:
                return tr("Report File");
            default:
                return QSqlTableModel::headerData(section, orientation, role);
            }
        }
    }
    return QSqlTableModel::headerData(section, orientation, role);
}

QStringList SqlReportModel::getReportFiles() const
{
    QStringList files;
    for (int i = 0; i < rowCount(); ++i) {
        QString file = data(index(i, ReportFile)).toString();
        files << file;
    }
    return files;
}

void SqlReportModel::onStudySelected(const QStringList &studyUids)
{
    QString filter;
    if (studyUids.size()) {
        foreach (QString uid, studyUids) {
            if (!filter.isEmpty()) filter.append(" OR ");
            filter.append(QString("StudyUid=\'%1\'").arg(uid));
        }
    } else {
        filter = QString("ReportUid IS NULL");
    }
    setFilter(filter);
}

bool SqlReportModel::onViewReport(const QString &studyUid)
{
    QStringList files;
    for (int i = 0; i < rowCount(); ++i) {
        if (data(index(i, StudyUid)).toString() == studyUid) {
            files << data(index(i, ReportFile)).toString();
        }
    }
    if (files.size()) {
        emit viewReports(files);
        return true;
    } else return false;
}

void SqlReportModel::onViewReport(const QModelIndex &index)
{
    if (index.isValid()) {
        QString file = data(this->index(index.row(), ReportFile)).toString();
        emit viewReports(QStringList()<<file);
    }
}

void SqlReportModel::onExportReport(const QModelIndex &index)
{
    if (index.isValid()) {
        QString file = data(this->index(index.row(), ReportFile)).toString();
        emit exportReports(QStringList()<<file);
    }
}

void SqlReportModel::onRemoveReport(const QModelIndex &index)
{
    if (index.isValid()) {
        QString file = data(this->index(index.row(), ReportFile)).toString();
        if (removeRow(index.row())) {
            QString srFile = QString("%1/%2").arg(mainWindow->getDbLocation(), file);
            QFile(srFile).remove();
            QDir().rmpath(srFile.left(srFile.lastIndexOf('/')));
            select();
        }
    }
}

void SqlReportModel::onRemoveAllReports()
{
    int row = rowCount() - 1;
    while (row >= 0) {
        QString file = data(index(row, ReportFile)).toString();
        if (removeRow(row)) {
            QString srFile = QString("%1/%2").arg(mainWindow->getDbLocation(), file);
            QFile(srFile).remove();
            QDir().rmpath(srFile.left(srFile.lastIndexOf('/')));
        }
        row--;
    }
    select();
}
