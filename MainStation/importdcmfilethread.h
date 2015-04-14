#ifndef IMPORTDCMFILETHREAD_H
#define IMPORTDCMFILETHREAD_H

#include <QThread>
class ImportStudyModel;

class ImportDcmFileThread : public QThread
{
    Q_OBJECT
public:
    explicit ImportDcmFileThread(ImportStudyModel *model, QObject *parent = 0);

    void run();

signals:
    void resultReady();

public slots:
    void setAbort(bool yes) { abort = yes; }

private:
    bool abort;
    ImportStudyModel *importModel;
};

#endif // IMPORTDCMFILETHREAD_H
