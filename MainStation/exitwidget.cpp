#include "exitwidget.h"
#include "ui_exitwidget.h"

ExitWidget::ExitWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExitWidget)
{
    ui->setupUi(this);
}

ExitWidget::~ExitWidget()
{
    delete ui;
}
