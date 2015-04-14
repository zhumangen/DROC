#ifndef SQLREPORTVIEW_H
#define SQLREPORTVIEW_H

#include <QTableView>

class SqlReportView : public QTableView
{
    Q_OBJECT
public:
    explicit SqlReportView(QWidget *parent = 0);

signals:
    void viewReport(const QModelIndex &index);
    void exportReport(const QModelIndex &index);
    void removeReport(const QModelIndex &index);

public slots:
    void onViewReport() { emit viewReport(currentIndex()); }
    void onExportReport() { emit exportReport(currentIndex()); }
    void onRemoveReport();

protected:
    void contextMenuEvent(QContextMenuEvent *e);

private:
    void setupContextMenu();
    QMenu *contextMenu;

    QAction *viewReportAction;
    QAction *exportReportAction;
    QAction *removeReportAction;
};

#endif // SQLREPORTVIEW_H
