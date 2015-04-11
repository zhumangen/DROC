#ifndef ITEMDELEGATES_H
#define ITEMDELEGATES_H

#include <QStyledItemDelegate>

class DicomScpItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    DicomScpItemDelegate(QObject *parent = 0):
        QStyledItemDelegate(parent){}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

class VoiItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    VoiItemDelegate(QObject *parent = 0):
        QStyledItemDelegate(parent){}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};


class ResetPasswordDialog;
class UserItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    UserItemDelegate(QObject *parent = 0):
        dialog(0),
        QStyledItemDelegate(parent){}
    //~UserItemDelegate() { delete dialog; }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private:
    mutable ResetPasswordDialog *dialog;
};

class GroupItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    GroupItemDelegate(QObject *parent = 0):
        QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

};

#endif // ITEMDELEGATES_H
