#ifndef SQLIMAGEVIEW_H
#define SQLIMAGEVIEW_H

#include <QTableView>

class SqlImageView : public QTableView
{
    Q_OBJECT
public:
    explicit SqlImageView(QWidget *parent = 0);

signals:
    void viewImage(const QModelIndex &index);
    void sendImage(const QModelIndex &index);
    void exportImage(const QModelIndex &index);
    void removeImage(const QModelIndex &index);

public slots:
    void onViewImage() {emit viewImage(currentIndex());}
    void onSendImage() {emit sendImage(currentIndex());}
    void onExportImage() {emit exportImage(currentIndex());}
    void onRemoveImage();

protected:
    void contextMenuEvent(QContextMenuEvent *e);

private:
    void setupContextMenu();
    QMenu *contextMenu;

    QAction *viewImageAction;
    QAction *sendImageAction;
    QAction *exportImageAction;
    QAction *removeImageAction;
};

#endif // SQLIMAGEVIEW_H
