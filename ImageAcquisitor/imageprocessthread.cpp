#include "imageprocessthread.h"

#include "IeLauncher.h"

ImageProcessThread::ImageProcessThread(QObject *parent) :
    srcBuf(0),
    destBuf(0),
    region(0),
    width(0),
    height(0),
    depth(0),
    QThread(parent)
{
}

void ImageProcessThread::run()
{
    //int ret = LaunchImageEnhancer_RAWnMem(srcBuf, destBuf, region, width, height, depth);
    //exit(ret);
}
