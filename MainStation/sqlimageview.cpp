#include "sqlimageview.h"
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

#include "mainwindow.h"

SqlImageView::SqlImageView(QWidget *parent) :
    QTableView(parent)
{
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    if (mainWindow->getCurrentGroup().permissions & GP_ExamineStudy)
        connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(viewImage(QModelIndex)));
    setupContextMenu();
}

void SqlImageView::setupContextMenu()
{
    contextMenu = new QMenu(this);

    viewImageAction = contextMenu->addAction(tr("View Image"), this, SLOT(onViewImage()));
    sendImageAction = contextMenu->addAction(tr("Send Image"), this, SLOT(onSendImage()));
    exportImageAction = contextMenu->addAction(tr("Export Image"), this, SLOT(onExportImage()));
    removeImageAction = contextMenu->addAction(tr("Remove Image"), this, SLOT(onRemoveImage()));

    GroupPermissions perms = mainWindow->getCurrentGroup().permissions;
    viewImageAction->setEnabled(perms & GP_ExamineStudy);
    sendImageAction->setEnabled(perms & GP_SendStudy);
    exportImageAction->setEnabled(perms & GP_ExportStudy);
    removeImageAction->setEnabled(perms & GP_RemoveStudy);
}

void SqlImageView::onRemoveImage()
{
    if (QMessageBox::Ok == QMessageBox::question(this, tr("Remove Image"),
                                                 tr("Are you sure to remove this image?"),
                                                 QMessageBox::Ok|QMessageBox::Cancel))
        emit removeImage(currentIndex());
}

void SqlImageView::contextMenuEvent(QContextMenuEvent *e)
{
    if (indexAt(e->pos()).isValid())
        contextMenu->popup(e->globalPos());
}
