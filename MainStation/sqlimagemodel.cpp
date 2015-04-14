#include "sqlimagemodel.h"
#include "mainwindow.h"

#include "dcmtk/dcmdata/dcuid.h"

#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QDateTime>

SqlImageModel::SqlImageModel(QObject *parent, QSqlDatabase db) :
    QSqlTableModel(parent, db)
{
    setEditStrategy(QSqlTableModel::OnRowChange);
    setTable("ImageTable");
}

QVariant SqlImageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role) {
        if (Qt::Horizontal == orientation) {
            switch (section) {
            case ImageNo:
                return tr("Image No.");
            case ImageTime:
                return tr("Image Time");
            case BodyPart:
                return tr("Body Part");
            case ImageDesc:
                return tr("Image Desc");
            case ImageFile:
                return tr("Image File");
            default:
                return QSqlTableModel::headerData(section, orientation, role);
            }
        }
    }
    return QSqlTableModel::headerData(section, orientation, role);
}

QStringList SqlImageModel::getAllImageFiles() const
{
    QStringList files;
    for (int i = 0; i < rowCount(); ++i) {
        QString file = data(index(i, ImageFile)).toString();
        files << file;
    }
    return files;
}

void SqlImageModel::onViewImage(const QModelIndex &index)
{
    if (index.isValid()) {
        QString file = data(this->index(index.row(), ImageFile)).toString();
        emit viewImages(QStringList()<<file);
    }
}

void SqlImageModel::onSendImage(const QModelIndex &index)
{
    if (index.isValid()) {
        QString file = data(this->index(index.row(), ImageFile)).toString();
        emit sendImages(QStringList()<<file);
    }
}

void SqlImageModel::onExportImage(const QModelIndex &index)
{
    if (index.isValid()) {
        QString file = data(this->index(index.row(), ImageFile)).toString();
        emit sendImages(QStringList()<<file);
    }
}

void SqlImageModel::onRemoveImage(const QModelIndex &index)
{
    if (index.isValid()) {
        QString file = data(this->index(index.row(), ImageFile)).toString();
        QString refUid = data(this->index(index.row(), RefImageUid)).toString();
        if (removeRow(index.row())) {
            QString prFile = QString("%1/%2").arg(mainWindow->getDbLocation(), file);
            QFile(prFile).remove();
            QString dirName = prFile.left(prFile.lastIndexOf('/'));
            if (!refUid.isEmpty()) {
                QString rawFile = QString("%1/%2_%3.dcm").arg(dirName, QString(RAW_IMAGE_PREFIX), refUid);
                QFile(rawFile).remove();
            }
            QDir().rmpath(dirName);
            select();
        }
    }
}

void SqlImageModel::onRemoveAllImages()
{
    int row = rowCount() - 1;
    while (row >= 0) {
        QString file = data(index(row, ImageFile)).toString();
        QString refUid = data(index(row, RefImageUid)).toString();
        if (removeRow(row)) {
            QString prFile = QString("%1/%2").arg(mainWindow->getDbLocation(), file);
            QFile(prFile).remove();
            QString dirName = prFile.left(prFile.lastIndexOf('/'));
            if (!refUid.isEmpty()) {
                QString rawFile = QString("%1/%2_%3.dcm").arg(dirName, QString(RAW_IMAGE_PREFIX), refUid);
                QFile(rawFile).remove();
            }
            QDir().rmpath(dirName);
        }
        row--;
    }
    select();
}

void SqlImageModel::onStudySelected(const QStringList &studyUids)
{
    QString filter;
    if (studyUids.size()) {
        for (int i = 0; i < studyUids.size(); ++i) {
            filter.append(QString("StudyUid=\'%1\'").arg(studyUids.at(i)));
            if (i < studyUids.size()-1) filter.append(" OR ");
        }
    } else filter.append("StudyUid IS NULL");

    setFilter(filter);
}
