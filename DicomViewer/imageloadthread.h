#ifndef IMAGELOADTHREAD_H
#define IMAGELOADTHREAD_H

#include <QThread>
#include <QStringList>
class ImageInstance;

class ImageLoadThread : public QThread
{
    Q_OBJECT
public:
    explicit ImageLoadThread(QObject *parent = 0): abort(false), QThread(parent){}

    void setImageFiles(const QStringList &files) { imageFiles = files; }
    void setAbort(bool yes) { abort = yes; }

    void run();

signals:
    void resultReady(ImageInstance *image);

public slots:

private:
    void createImageInstances();
    QStringList imageFiles;
    volatile bool abort;

};

#endif // IMAGELOADTHREAD_H
