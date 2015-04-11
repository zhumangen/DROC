#include "grouplistview.h"

GroupListView::GroupListView(QWidget *parent) :
    QListView(parent)
{
}

void GroupListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    selection.merge(selected, QItemSelectionModel::Select);
    selection.merge(deselected, QItemSelectionModel::Deselect);

    if (!selection.isEmpty())
        emit currentItemChanged(selection.indexes().first());
    else emit currentItemChanged(QModelIndex());
}
