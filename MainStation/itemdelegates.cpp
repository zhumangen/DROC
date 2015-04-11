#include "itemdelegates.h"
#include "../share/global.h"
#include "dicomscpmodel.h"
#include "usergroupmodels.h"
#include "resetpassworddialog.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>

#define USERMODEL_DIVISIONS "USERMODELDIVISIONS"
#define USERMODEL_TITLES "USERMODELTITLES"

QWidget *DicomScpItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch (index.column()) {
    case DicomScpModel::Id:
    case DicomScpModel::Aetitle:
    case DicomScpModel::Hostname:
    case DicomScpModel::Description:
        return new QLineEdit(parent);
    case DicomScpModel::Type: {
        QComboBox *combo = new QComboBox(parent);
        combo->addItems(DicomScp::ScpTypeString);
        return combo;
    }
    case DicomScpModel::Port: {
        QSpinBox *spin = new QSpinBox(parent);
        spin->setMinimum(1);
        spin->setMaximum(65535);
        return spin;
    }
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void DicomScpItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    DicomScp *scp = static_cast<DicomScp*>(index.internalPointer());
    switch (index.column()) {
    case DicomScpModel::Id:
        qobject_cast<QLineEdit*>(editor)->setText(scp->id);
        break;
    case DicomScpModel::Type:
        qobject_cast<QComboBox*>(editor)->setCurrentIndex(scp->type);
        break;
    case DicomScpModel::Aetitle:
        qobject_cast<QLineEdit*>(editor)->setText(scp->aetitle);
        break;
    case DicomScpModel::Hostname:
        qobject_cast<QLineEdit*>(editor)->setText(scp->hostname);
        break;
    case DicomScpModel::Port:
        qobject_cast<QSpinBox*>(editor)->setValue(scp->port);
        break;
    case DicomScpModel::Description:
        qobject_cast<QLineEdit*>(editor)->setText(scp->description);
        break;
    default:
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void DicomScpItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    switch (index.column()) {
    case DicomScpModel::Id:
    case DicomScpModel::Aetitle:
    case DicomScpModel::Hostname:
    case DicomScpModel::Description:
        model->setData(index, qobject_cast<QLineEdit*>(editor)->text());
        break;
    case DicomScpModel::Type:
        model->setData(index, qobject_cast<QComboBox*>(editor)->currentIndex());
        break;
    case DicomScpModel::Port:
        model->setData(index, qobject_cast<QSpinBox*>(editor)->value());
        break;
    default:
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

QWidget *UserItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch (index.column()) {
    case UserModel::Name:
        return new QLineEdit(parent);
    case UserModel::Password: {
        QPushButton *resetButton = new QPushButton(tr("Reset"), parent);
        delete dialog;
        dialog = new ResetPasswordDialog(index, parent);
        connect(resetButton, SIGNAL(clicked()), dialog, SLOT(exec()));
        return resetButton;
    }
    case UserModel::Group: {
        QComboBox *combo = new QComboBox(parent);
        combo->addItems(UserModel::getGroupNames());
        return combo;
    }
    case UserModel::Division: {
        QComboBox *combo = new QComboBox(parent);
        QSettings settings;
        QStringList divs = settings.value(USERMODEL_DIVISIONS).toStringList();
        combo->addItems(divs);
        combo->setEditable(true);
        return combo;
    }
    case UserModel::Title: {
        QComboBox *combo = new QComboBox(parent);
        QSettings settings;
        QStringList titles = settings.value(USERMODEL_TITLES).toStringList();
        combo->addItems(titles);
        combo->setEditable(true);
        return combo;
    }
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void UserItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    User *user = static_cast<User*>(index.internalPointer());
    switch (index.column()) {
    case UserModel::Name:
        qobject_cast<QLineEdit*>(editor)->setText(user->name);
        break;
    case UserModel::Group:
        qobject_cast<QComboBox*>(editor)->setCurrentText(user->group);
        break;
    case UserModel::Division:
        qobject_cast<QComboBox*>(editor)->setCurrentText(user->division);
        break;
    case UserModel::Title:
        qobject_cast<QComboBox*>(editor)->setCurrentText(user->title);
        break;
    default:
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void UserItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    switch (index.column()) {
    case UserModel::Name:
        model->setData(index, qobject_cast<QLineEdit*>(editor)->text());
        break;
    case UserModel::Password:
        model->setData(index, QVariant());
        break;
    case UserModel::Division: {
        QSettings settings;
        QStringList divs = settings.value(USERMODEL_DIVISIONS).toStringList();
        QString text = qobject_cast<QComboBox*>(editor)->currentText();
        divs.removeOne(text);
        divs.prepend(text);
        if (divs.size() > 16) divs.removeLast();
        settings.setValue(USERMODEL_DIVISIONS, divs);
        model->setData(index, text);
        break;
    }
    case UserModel::Title: {
        QSettings settings;
        QStringList titles = settings.value(USERMODEL_TITLES).toStringList();
        QString text = qobject_cast<QComboBox*>(editor)->currentText();
        titles.removeOne(text);
        titles.prepend(text);
        if (titles.size() > 16) titles.removeLast();
        settings.setValue(USERMODEL_TITLES, titles);
        model->setData(index, text);
        break;
    }
    case UserModel::Group:
        model->setData(index, qobject_cast<QComboBox*>(editor)->currentText());
        break;
    default:
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

QWidget *GroupItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 0)
        return new QLineEdit(parent);
    else
        return QStyledItemDelegate::createEditor(parent, option, index);
}

void GroupItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.isValid()) {
        Group *g = static_cast<Group*>(index.internalPointer());
        if (g && index.column()==0)
            qobject_cast<QLineEdit*>(editor)->setText(g->name);
    }
}

void GroupItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column()==0)
        model->setData(index, qobject_cast<QLineEdit*>(editor)->text());
}
