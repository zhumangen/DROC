#include "sqlreportview.h"
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

#include "mainwindow.h"

SqlReportView::SqlReportView(QWidget *parent) :
    QTableView(parent)
{
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    if (mainWindow->getCurrentGroup().permissions & GP_ExamineStudy)
        connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(viewReport(QModelIndex)));
    setupContextMenu();
}

void SqlReportView::setupContextMenu()
{
    contextMenu = new QMenu(this);

    viewReportAction = contextMenu->addAction(tr("View Report"), this, SLOT(onViewReport()));
    exportReportAction = contextMenu->addAction(tr("Export Report"), this, SLOT(onExportReport()));
    removeReportAction = contextMenu->addAction(tr("Remove Report"), this, SLOT(onRemoveReport()));

    GroupPermissions perms = mainWindow->getCurrentGroup().permissions;
    viewReportAction->setEnabled(perms & GP_ExamineStudy);
    exportReportAction->setEnabled(perms & GP_ExportStudy);
    removeReportAction->setEnabled(perms & GP_RemoveStudy);
}

void SqlReportView::onRemoveReport()
{
    if (QMessageBox::Ok == QMessageBox::question(this, tr("Remove Report"),
                                                 tr("Are you sure to remove this report?"),
                                                 QMessageBox::Ok|QMessageBox::Cancel))
        emit removeReport(currentIndex());
}

void SqlReportView::contextMenuEvent(QContextMenuEvent *e)
{
    if (indexAt(e->pos()).isValid())
        contextMenu->popup(e->globalPos());
}
