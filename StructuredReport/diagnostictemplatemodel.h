#ifndef DIAGNOSTICTEMPLATEMODEL_H
#define DIAGNOSTICTEMPLATEMODEL_H

#include <QAbstractItemModel>
class QXmlStreamWriter;

class DiagnosticTemplateModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    struct DT_Node {
        DT_Node *parent;
        QString title;
        QString findings;
        QString conclusion;
        QList<DT_Node*> childList;

        DT_Node(DT_Node *p = 0):parent(p){ if (p) p->childList << this; }
        ~DT_Node() { qDeleteAll(childList); }
    };

    explicit DiagnosticTemplateModel(QObject *parent = 0);
    ~DiagnosticTemplateModel() { saveData(); delete rootNode; }

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &/*parent*/) const { return 1; }
    QModelIndex parent(const QModelIndex &child) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &) const;

signals:
    void itemSelected(const QString &findings, const QString &conclusion);

    void editItem(const QModelIndex &index, const QString &t, const QString &f, const QString &c);
    void editItemTitle(const QModelIndex &index);

public slots:
    void onDoubleClicked(const QModelIndex &index);

    void onNewItemAfter(const QModelIndex &index);
    void onNewItemBefore(const QModelIndex &index);
    void onNewSubItem(const QModelIndex &index);
    void onApplyEdit(const QModelIndex &index, const QString &t, const QString &f, const QString &c);
    void onEditItem(const QModelIndex &index);
    void onRemoveItem(const QModelIndex &index);

private:
    void setupModel();
    bool saveData();
    void xmlWriteHelpper(QXmlStreamWriter &writer, DT_Node *srcnode, int level);

    DT_Node *rootNode;
    bool isModified;

};

#endif // DIAGNOSTICTEMPLATEMODEL_H
