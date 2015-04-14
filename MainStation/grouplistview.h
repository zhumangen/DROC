#ifndef GROUPLISTVIEW_H
#define GROUPLISTVIEW_H

#include <QListView>

class GroupListView : public QListView
{
    Q_OBJECT
public:
    explicit GroupListView(QWidget *parent = 0);

signals:
    void currentItemChanged(const QModelIndex &index);

protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QItemSelection selection;

};

#endif // GROUPLISTVIEW_H
