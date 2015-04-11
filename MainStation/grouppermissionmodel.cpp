#include "grouppermissionmodel.h"

#include "../share/global.h"

GroupPermissionModel::GroupPermissionModel(QObject *parent) :
    QStandardItemModel(parent)
{
    setupModel();
}

void GroupPermissionModel::setupModel()
{
    setColumnCount(1);

    for (int i = 0; i < GroupPermissionStringTable.size(); i++) {
        QStandardItem *item = new QStandardItem(GroupPermissionStringTable.at(i));
        item->setCheckable(true);
        appendRow(item);
    }

    connect(this, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)));
}

void GroupPermissionModel::checkAllItems(int state)
{
    for (int i = 0; i < rowCount(); i++) {
        QStandardItem *item = this->item(i);
        if (item->isEnabled()) item->setCheckState((Qt::CheckState)state);
    }
}

void GroupPermissionModel::setCheckable(bool yes)
{
    for (int i = 0; i < rowCount(); i++)
        item(i)->setEnabled(!yes);
}

void GroupPermissionModel::setPermission(ulong perm, bool isAdmin)
{
    for (int i = 0; i < rowCount(); ++i) {
        if (perm & 0x1) item(i)->setCheckState(Qt::Checked);
        else item(i)->setCheckState(Qt::Unchecked);
        perm >>= 1;
    }
    setCheckable(isAdmin);
}

void GroupPermissionModel::onItemChanged(QStandardItem *item)
{
    ulong perm = 0;
    for (int i = 0; i < rowCount(); ++i) {
        if (this->item(i)->checkState() == Qt::Checked)
            perm |= (0x1 << i);
    }
    emit permissionChanged(perm);
}
