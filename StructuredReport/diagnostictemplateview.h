#ifndef DIAGNOSTICTEMPLATEVIEW_H
#define DIAGNOSTICTEMPLATEVIEW_H

#include <QTreeView>

class DiagnosticTemplateView : public QTreeView
{
    Q_OBJECT
public:
    explicit DiagnosticTemplateView(QWidget *parent = 0);

signals:
    void newItemBefore(const QModelIndex &index);
    void newItemAfter(const QModelIndex &index);
    void newSubItem(const QModelIndex &index);
    void editItem(const QModelIndex &index);
    void applyEdit(const QModelIndex &index, const QString &t, const QString &f, const QString &c);
    void removeItem(const QModelIndex &index);

public slots:
    void onNewItemBefore() { emit newItemBefore(curIndex); }
    void onNewItemAfter() { emit newItemAfter(curIndex); }
    void onNewSubItem() { setExpanded(curIndex, true); emit newSubItem(curIndex); }
    void onEditItem() { emit editItem(curIndex); }
    void onEditItem(const QModelIndex &index, const QString &t, const QString &f, const QString &c);
    void onRemoveItem();

protected:
    void contextMenuEvent(QContextMenuEvent *e);

private:
    void init();
    QModelIndex curIndex;
    QMenu *conMenu;
    QAction *newItemBeforeAction;
    QAction *newItemAfterAction;
    QAction *newSubItemAction;
    QAction *editItemAction;
    QAction *removeItemAction;

};

#endif // DIAGNOSTICTEMPLATEVIEW_H
