#ifndef TIMELABEL_H
#define TIMELABEL_H

#include <QLabel>

class TimeLabel : public QLabel
{
    Q_OBJECT
public:
    explicit TimeLabel(QWidget *parent = 0);

signals:

public slots:
    void updateTime();


};

#endif // TIMELABEL_H
