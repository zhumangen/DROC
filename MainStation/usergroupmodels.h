#ifndef USERGROUPMODELS_H
#define USERGROUPMODELS_H

#include <QAbstractItemModel>
#include "../share/global.h"

class UserModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    enum ColumnType {
        Name,
        Password,
        Group,
        Division,
        Title,

        ColumnCount,
    };


    explicit UserModel(QObject *parent = 0):
        isModified(0),
        QAbstractItemModel(parent){ setupModel(); }
    ~UserModel() { qDeleteAll(userList); }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return ColumnCount; }
    QModelIndex parent(const QModelIndex &/*child*/) const { return QModelIndex(); }
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);

    static const QStringList& getGroupNames() { return groupNames; }

public slots:
    bool saveData();
    void setGroupNames(const QStringList &groups) { groupNames = groups; }

private:
    void setupModel();
    QList<User*> userList;
    bool isModified;
    static QStringList groupNames;
};

class GroupModel: public QAbstractItemModel
{
    Q_OBJECT
public:

    explicit GroupModel(QObject *parent = 0):
        isModified(0),
        curGroup(0),
        QAbstractItemModel(parent){ setupModel(); }
    ~GroupModel() { qDeleteAll(groupList); }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return 1; }
    QModelIndex parent(const QModelIndex &/*child*/) const { return QModelIndex(); }
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);

    QStringList getGroupNames() const;

signals:
    void groupChanged(const QStringList &gList);
    void currentGroupChanged(ulong perm, bool isAdmin = false);

public slots:
    bool saveData();
    void onCurrentChanged(const QModelIndex &index);
    void onPermissionChanged(ulong perm);

private:

    void setupModel();
    QList<Group*> groupList;
    bool isModified;
    Group *curGroup;
};

#endif // USERGROUPMODELS_H
