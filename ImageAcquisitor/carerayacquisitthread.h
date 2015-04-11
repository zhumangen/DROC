#ifndef CARERAYACQUISITTHREAD_H
#define CARERAYACQUISITTHREAD_H

#include <QThread>

class CareRayAcquisitThread : public QThread
{
    Q_OBJECT
public:
    explicit CareRayAcquisitThread(QObject *parent = 0);
    ~CareRayAcquisitThread() { delete _imgBuf; }

    void run();

    void getImageSize(int &width, int &height) const {width = _width; height = _height; }
    short *getImage(short *&imgBuf) { imgBuf = _imgBuf; _imgBuf = 0; return imgBuf; }

signals:
    void exposureReady(bool yes);
    void detectorMsg(const QString &msg);

public slots:
    void setAbort(bool yes) { _abort = yes; }

private:
    int _width;
    int _height;
    short *_imgBuf;
    volatile bool _abort;

};

#endif // CARERAYACQUISITTHREAD_H
