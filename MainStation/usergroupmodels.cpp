#include "usergroupmodels.h"
#include "../share/configfiles.h"
#include "../share/global.h"
#include "mainwindow.h"

#include <QDataStream>
#include <QFile>

QStringList UserModel::groupNames = QStringList();

void UserModel::setupModel()
{
    QFile file(REGISTERED_USERS);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        while (!in.atEnd()) {
            User *user = new User;
            in >> *user;
            userList << user;
        }
    }
}

bool UserModel::saveData()
{
    if (isModified) {
        QFile file(REGISTERED_USERS);
        if (file.open(QIODevice::WriteOnly)) {
            QDataStream out(&file);
            foreach (User *user, userList) {
                out << *user;
            }
            isModified = false;
        } else return false;
    }
    return true;
}

int UserModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return userList.size();
}

Qt::ItemFlags UserModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (index.row()==0) {
            if ((index.column()==Name) || (index.column()==Group))
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            else
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }

        if ((mainWindow->getCurrentGroup().permissions & GP_UserGroupManagement)) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex UserModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid())
        return QModelIndex();
    return createIndex(row, column, userList.at(row));
}

QVariant UserModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role) {
        switch (index.column()) {
        case Name:
            return userList.at(index.row())->name;
        case Password:
            return QString("***");
        case Group:
            return userList.at(index.row())->group;
        case Division:
            return userList.at(index.row())->division;
        case Title:
            return userList.at(index.row())->title;
        default:
            return QVariant();
        }
    } else return QVariant();
}

QVariant UserModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role) {
        if (Qt::Horizontal == orientation) {
            switch (section) {
            case Name:
                return tr("Name");
            case Password:
                return tr("Password");
            case Group:
                return tr("Group");
            case Division:
                return tr("Division");
            case Title:
                return tr("Title");
            default:
                return QVariant();
            }
        } else return section+1;
    } else return QVariant();
}

bool UserModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && Qt::EditRole == role) {
        switch (index.column()) {
        case Name:
            if (userList.at(index.row())->name != value.toString()) {
                userList[index.row()]->name = value.toString();
                isModified = true;
            }
            break;
        case Password:
            isModified = true;
            break;
        case Group:
            if (userList.at(index.row())->group != value.toString()) {
                userList[index.row()]->group = value.toString();
                isModified = true;
            }
            break;
        case Division:
            if (userList.at(index.row())->division != value.toString()) {
                userList[index.row()]->division = value.toString();
                isModified = true;
            }
            break;
        case Title:
            if (userList.at(index.row())->title != value.toString()) {
                userList[index.row()]->title = value.toString();
                isModified = true;
            }
            break;
        default:
            return false;
        }

        if (isModified)
            emit dataChanged(index, index, QVector<int>()<<Qt::DisplayRole);
        return true;
    } return false;
}

bool UserModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count < 1) return false;

    beginInsertRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        User *u = new User;
        userList.insert(row+i, u);
    }
    endInsertRows();
    return true;
}

bool UserModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count < 1) return false;

    beginRemoveRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i)
        delete userList.takeAt(row);
    endRemoveRows();
    return true;
}


void GroupModel::setupModel()
{
    QFile file(REGISTERED_GROUPS);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        while (!in.atEnd()) {
            Group *g = new Group;
            in >> *g;
            groupList << g;
        }
    }
}

bool GroupModel::saveData()
{
    if (isModified) {
        QFile file(REGISTERED_GROUPS);
        if (file.open(QIODevice::WriteOnly)) {
            QDataStream out(&file);
            foreach(Group *group, groupList) {
                    out << *group;
            }
            isModified = false;
        } else return false;
    }
    return true;
}

void GroupModel::onCurrentChanged(const QModelIndex &index)
{
    if (index.isValid()) {
        curGroup = static_cast<Group*>(index.internalPointer());
        bool admin = false;
        if (index.row()==0) admin = true;
        emit currentGroupChanged(curGroup->permissions, admin);
    } else {
        curGroup = 0;
        emit currentGroupChanged(0);
    }
}

void GroupModel::onPermissionChanged(ulong perm)
{
    if (curGroup) {
        curGroup->permissions = (GroupPermission)perm;
        isModified = true;
    }
}

int GroupModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return groupList.size();
}

Qt::ItemFlags GroupModel::flags(const QModelIndex &index) const
{
    if ((index.row()>0) && (mainWindow->getCurrentGroup().permissions&GP_UserGroupManagement))
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex GroupModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid())
        return QModelIndex();
    return createIndex(row, column, groupList.at(row));
}

QVariant GroupModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && (index.column()==0) && Qt::DisplayRole == role) {
            return groupList.at(index.row())->name;
    } else return QVariant();
}

bool GroupModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && (index.column()==0) && Qt::EditRole == role) {
        if (groupList.at(index.row())->name != value.toString()) {
            groupList[index.row()]->name = value.toString();
            emit groupChanged(getGroupNames());
            isModified = true;
            emit dataChanged(index, index, QVector<int>()<<Qt::DisplayRole);
        }
        return true;
    } else return false;
}

bool GroupModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count < 1) return false;

    beginInsertRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        Group *g = new Group;
        g->permissions = (GroupPermission)0;
        groupList.insert(row+i, g);
    }
    endInsertRows();
    emit groupChanged(getGroupNames());
    return true;
}

bool GroupModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row == 0) {
        row = 1;
        count--;
    }
    if (parent.isValid() || count < 1) return false;

    beginRemoveRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        delete groupList.takeAt(row);
    }
    endRemoveRows();
    emit groupChanged(getGroupNames());
    return true;
}

QStringList GroupModel::getGroupNames() const
{
    QStringList groups;
    foreach (Group *g, groupList) {
        groups << g->name;
    }
    return groups;
}
