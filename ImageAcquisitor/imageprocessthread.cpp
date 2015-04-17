#include "imageprocessthread.h"

#include "IeLauncher.h"

ImageProcessThread::ImageProcessThread(QObject *parent) :
    srcBuf(0),
    destBuf(0),
    width(0),
    height(0),
    depth(0),
    QThread(parent)
{
}

void ImageProcessThread::run()
{
    config.replace(QChar('/'), QChar('\\'));
    int ret = 1;//LaunchImageEnhancer_RAWnMem(srcBuf, destBuf, config.toLocal8Bit().data(), width, height, depth);
    emit resultReady(ret);
}
