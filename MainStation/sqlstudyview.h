#ifndef SQLSTUDYVIEW_H
#define SQLSTUDYVIEW_H

#include <QTableView>

class SqlStudyView : public QTableView
{
    Q_OBJECT
public:
    explicit SqlStudyView(QWidget *parent = 0);

signals:
    void viewImages();
    void viewReports();
    void sendStudies();
    void exportStudies();
    void removeStudies();
    void studySelectionChanged(const QModelIndexList &indexes);
    void createReport(const QModelIndex &index);
    void modifyStudy(const QModelIndex &index);
    void newStudy(const QModelIndex &index);
    void newImage(const QModelIndex &index);

public slots:
    void onCreateReport();
    void onViewImages();
    void onViewReports();
    void onSendStudies();
    void onExportStudies();
    void onRemoveStudies();
    void onModifyStudy() { emit modifyStudy(currentIndex()); }
    void onNewStudy();
    void onNewImage();
    void setItemSelected(const QModelIndex &index) { selectRow(index.row()); }

protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void contextMenuEvent(QContextMenuEvent *e);

private:
    void setupContextMenu();

    QStringList studyUids;
    QMenu *contextMenu;

    QAction *viewImage;
    QAction *viewReport;
    QAction *newReport;
    QAction *sendStudy;
    QAction *exportStudy;
    QAction *removeStudy;
    QAction *modStudy;
    QAction *newStudyAction;
    QAction *newImageInStudy;
};

#endif // SQLSTUDYVIEW_H
