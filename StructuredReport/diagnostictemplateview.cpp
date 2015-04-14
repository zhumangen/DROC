#include "diagnostictemplateview.h"
#include "newitemdialog.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

DiagnosticTemplateView::DiagnosticTemplateView(QWidget *parent) :
    QTreeView(parent)
{
    init();
    setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::SelectedClicked|QAbstractItemView::EditKeyPressed|
                    QAbstractItemView::AnyKeyPressed);
}

void DiagnosticTemplateView::init()
{
    conMenu = new QMenu(this);
    newItemAfterAction = conMenu->addAction(tr("New Item After"), this, SLOT(onNewItemAfter()));
    newItemBeforeAction = conMenu->addAction(tr("New Item Before"), this, SLOT(onNewItemBefore()));
    newSubItemAction = conMenu->addAction(tr("New Sub Item"), this, SLOT(onNewSubItem()));
    editItemAction = conMenu->addAction(tr("Edit Item"), this, SLOT(onEditItem()));
    removeItemAction = conMenu->addAction(tr("Remove Item"), this, SLOT(onRemoveItem()));
}

void DiagnosticTemplateView::contextMenuEvent(QContextMenuEvent *e)
{
    curIndex = indexAt(e->pos());
    if (curIndex.isValid()) {
        if (curIndex.child(0, 0).isValid()) editItemAction->setVisible(false);
        else editItemAction->setVisible(true);
        conMenu->popup(e->globalPos());
    }
}

void DiagnosticTemplateView::onEditItem(const QModelIndex &index, const QString &t, const QString &f, const QString &c)
{
    NewItemDialog dialog(this);
    dialog.setContents(t, f, c);
    if (QDialog::Accepted == dialog.exec()) {
        emit applyEdit(index, dialog.title, dialog.findings, dialog.conclusion);
    }
}

void DiagnosticTemplateView::onRemoveItem()
{
    if (QMessageBox::Yes == QMessageBox::question(this, tr("Remove Item"),
                                                 tr("Are you sure to remove this item?"),
                                                 QMessageBox::Yes|QMessageBox::No))
        emit removeItem(curIndex);
}
