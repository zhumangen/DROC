#ifndef INITCRDETECTORTHREAD_H
#define INITCRDETECTORTHREAD_H

#include <QThread>

class InitCrDetectorThread : public QThread
{
    Q_OBJECT
public:
    explicit InitCrDetectorThread(QObject *parent = 0);

    void run();

signals:
    void progressMsg(const QString &msg);

public slots:

};

#endif // INITCRDETECTORTHREAD_H
