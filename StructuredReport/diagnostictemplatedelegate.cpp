#include "diagnostictemplatedelegate.h"
#include "diagnostictemplatemodel.h"

#include <QLineEdit>

DiagnosticTemplateDelegate::DiagnosticTemplateDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QWidget *DiagnosticTemplateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return new QLineEdit(parent);
}

void DiagnosticTemplateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    DiagnosticTemplateModel::DT_Node *node = static_cast<DiagnosticTemplateModel::DT_Node*>(index.internalPointer());
    qobject_cast<QLineEdit*>(editor)->setText(node->title);
}

void DiagnosticTemplateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    model->setData(index, qobject_cast<QLineEdit*>(editor)->text());
}
