#ifndef MODIFYSTUDYTHREAD_H
#define MODIFYSTUDYTHREAD_H

#include <QThread>
#include <QSqlRecord>

class ModifyStudyThread : public QThread
{
    Q_OBJECT
public:
    explicit ModifyStudyThread(const QSqlRecord &studyRec, QObject *parent = 0);

    void run();

signals:

public slots:

private:
    QSqlRecord study;
};

#endif // MODIFYSTUDYTHREAD_H
