#ifndef DIAGNOSTICTEMPLATEDELEGATE_H
#define DIAGNOSTICTEMPLATEDELEGATE_H

#include <QStyledItemDelegate>

class DiagnosticTemplateDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DiagnosticTemplateDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

signals:

public slots:

};

#endif // DIAGNOSTICTEMPLATEDELEGATE_H
