#include "sqlstudyview.h"
#include "sqlstudymodel.h"

#include "mainwindow.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>

SqlStudyView::SqlStudyView(QWidget *parent) :
    QTableView(parent)
{
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    if (mainWindow->getCurrentGroup().permissions & GP_ExamineStudy)
        connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(viewImages()));

    setupContextMenu();
}

void SqlStudyView::setupContextMenu()
{
    contextMenu = new QMenu(this);

    viewImage = contextMenu->addAction(tr("View Image"), this, SLOT(onViewImages()));
    viewReport = contextMenu->addAction(tr("View Report"), this, SLOT(onViewReports()));
    newReport = contextMenu->addAction(tr("New Report"), this, SLOT(onCreateReport()));
    sendStudy = contextMenu->addAction(tr("Send Study"), this, SLOT(onSendStudies()));
    exportStudy = contextMenu->addAction(tr("Export Study"), this, SLOT(onExportStudies()));
    removeStudy = contextMenu->addAction(tr("Remove Study"), this, SLOT(onRemoveStudies()));
    modStudy = contextMenu->addAction(tr("Modify Study"), this, SLOT(onModifyStudy()));
    newStudyAction = contextMenu->addAction(tr("New Study"), this, SLOT(onNewStudy()));
    newImageInStudy = contextMenu->addAction(tr("Acquisit Image"), this, SLOT(onNewImage()));

    GroupPermissions perms = mainWindow->getCurrentGroup().permissions;
    viewImage->setEnabled(perms & GP_ExamineStudy);
    viewReport->setEnabled(perms & GP_ExamineStudy);
    newReport->setEnabled(perms & GP_CreateReport);
    sendStudy->setEnabled(perms & GP_SendStudy);
    exportStudy->setEnabled(perms & GP_ExportStudy);
    removeStudy->setEnabled(perms & GP_RemoveStudy);
    modStudy->setEnabled(perms & GP_ModifyReport);
    newStudyAction->setEnabled(perms & GP_RegisterStudy);
    newImageInStudy->setEnabled(perms & GP_AcquisitImage);
}

void SqlStudyView::onViewImages()
{
    if (!selectedIndexes().isEmpty()) {
        emit viewImages();
    } else {
        QMessageBox::information(this, tr("View Image"), tr("Select one or more studies first."));
    }
}

void SqlStudyView::onViewReports()
{
    if (!selectedIndexes().isEmpty()) {
        emit viewReports();
    } else {
        QMessageBox::information(this, tr("View Report"), tr("Select one or more studies first."));
    }
}

void SqlStudyView::onSendStudies()
{
    if (!selectedIndexes().isEmpty()) {
        emit sendStudies();
    } else {
        QMessageBox::information(this, tr("Send Study"), tr("Select one or more studies first."));
    }
}

void SqlStudyView::onExportStudies()
{
    if (!selectedIndexes().isEmpty()) {
        emit exportStudies();
    } else {
        QMessageBox::information(this, tr("Export Study"), tr("Select one or more studies first."));
    }
}

void SqlStudyView::onRemoveStudies()
{
    if (!selectedIndexes().isEmpty()) {
        if (QMessageBox::Ok == QMessageBox::question(this, tr("Remove Study"),
                                                     tr("Are you sure to remove these studies?"),
                                                     QMessageBox::Ok|QMessageBox::Cancel)) {
            emit removeStudies();
        }
    } else {
        QMessageBox::information(this, tr("Remove Study"), tr("Select one or more studies first."));
    }
}

void SqlStudyView::onCreateReport()
{
    if (!selectedIndexes().isEmpty()) {
        emit createReport(selectedIndexes().first());
    } else {
        QMessageBox::information(this, tr("Create Report"), tr("Select one or more studies first."));
    }
}

void SqlStudyView::onNewStudy()
{
    if (!selectedIndexes().isEmpty()) {
        emit newStudy(selectedIndexes().first());
    } else {
        QMessageBox::information(this, tr("New Study"), tr("Select one or more studies first."));
    }
}

void SqlStudyView::onNewImage()
{
    if (!selectedIndexes().isEmpty()) {
        emit newImage(selectedIndexes().first());
    } else {
        QMessageBox::information(this, tr("New Image In Study"), tr("Select one or more studies first."));
    }
}

void SqlStudyView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTableView::selectionChanged(selected, deselected);
    emit studySelectionChanged(selectedIndexes());
}

void SqlStudyView::contextMenuEvent(QContextMenuEvent *e)
{
    QModelIndex index = indexAt(e->pos());
    if (index.isValid())
        contextMenu->popup(e->globalPos());
}
