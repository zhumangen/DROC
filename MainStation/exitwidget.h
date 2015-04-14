#ifndef EXITWIDGET_H
#define EXITWIDGET_H

#include <QWidget>

namespace Ui {
class ExitWidget;
}

class ExitWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExitWidget(QWidget *parent = 0);
    ~ExitWidget();

private:
    Ui::ExitWidget *ui;
};

#endif // EXITWIDGET_H
