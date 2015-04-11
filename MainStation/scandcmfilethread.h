#ifndef SCANDCMFILETHREAD_H
#define SCANDCMFILETHREAD_H

#include <QThread>
#include <QStringList>
class StudyRecord;

class ScanDcmFileThread : public QThread
{
    Q_OBJECT
public:
    explicit ScanDcmFileThread(QObject *parent = 0);

    void run();
    void setFiles(const QStringList &files) { fileList = files; }
    void setAbort(bool yes) { abort = yes; }

signals:
    void resultReady(StudyRecord *study);

public slots:

private:
    QStringList fileList;
    bool abort;

};

#endif // SCANDCMFILETHREAD_H
