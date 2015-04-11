#ifndef PROCEDUREITEMDELEGATE_H
#define PROCEDUREITEMDELEGATE_H

#include <QStyledItemDelegate>

class ProcedureItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ProcedureItemDelegate(QObject *parent = 0);

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

signals:

public slots:

};

#endif // PROCEDUREITEMDELEGATE_H
