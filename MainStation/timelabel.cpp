#include "timelabel.h"
#include <QTimer>
#include <QDateTime>
#include <QPalette>

TimeLabel::TimeLabel(QWidget *parent) :
    QLabel(parent)
{
    updateTime();
    QPalette p = palette();
    p.setColor(QPalette::WindowText, Qt::blue);
    setPalette(p);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(updateTime()));
    timer->start(1000);
}

void TimeLabel::updateTime()
{
    setText(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));
}
