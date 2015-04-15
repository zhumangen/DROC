#ifndef IMAGEPROCESSTHREAD_H
#define IMAGEPROCESSTHREAD_H

#include <QThread>

class ImageProcessThread : public QThread
{
    Q_OBJECT
public:
    explicit ImageProcessThread(QObject *parent = 0);

    void run();

signals:
    void resultReady(bool ok);

public slots:

public:
    const void *srcBuf;
    void *destBuf;
    QString config;
    ushort width;
    ushort height;
    ushort depth;

};

#endif // IMAGEPROCESSTHREAD_H
