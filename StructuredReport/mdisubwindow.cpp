#include "mdisubwindow.h"
#include "reportwidget.h"
#include <QScrollArea>
#include <QMessageBox>
#include <QCloseEvent>

MdiSubWindow::MdiSubWindow(QWidget *parent) :
    QMdiSubWindow(parent)
{
}

void MdiSubWindow::closeEvent(QCloseEvent *e)
{
    QScrollArea *scroll = qobject_cast<QScrollArea*>(widget());
    ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
    if (!report->close()) e->ignore();
}
