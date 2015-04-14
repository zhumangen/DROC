#ifndef GROUPPERMISSIONMODEL_H
#define GROUPPERMISSIONMODEL_H

#include <QStandardItemModel>

class GroupPermissionModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit GroupPermissionModel(QObject *parent = 0);

signals:
    void permissionChanged(ulong perm);

public slots:
    void checkAllItems(int state);
    void setCheckable(bool yes);
    void setPermission(ulong perm, bool isAdmin);
    void onItemChanged(QStandardItem *item);

private:
    void setupModel();

};

#endif // GROUPPERMISSIONMODEL_H
