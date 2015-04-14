#ifndef MDISUBWINDOW_H
#define MDISUBWINDOW_H

#include <QMdiSubWindow>

class MdiSubWindow : public QMdiSubWindow
{
    Q_OBJECT
public:
    explicit MdiSubWindow(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *e);

signals:

public slots:

};

#endif // MDISUBWINDOW_H
