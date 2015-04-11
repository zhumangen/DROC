#ifndef IMPORTSTUDYVIEW_H
#define IMPORTSTUDYVIEW_H

#include <QTableView>

class ImportStudyView : public QTableView
{
    Q_OBJECT
public:
    explicit ImportStudyView(QWidget *parent = 0);

signals:

public slots:
    void removeSelectedRows();
    void removeAllRows();

protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QItemSelection selection;

};

#endif // IMPORTSTUDYVIEW_H
