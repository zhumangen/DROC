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
<<<<<<< HEAD
    //LaunchImageEnhancer_RAWnMem(srcBuf, destBuf, region, width, height, depth);
=======
    //int ret = LaunchImageEnhancer_RAWnMem(srcBuf, destBuf, region, width, height, depth);
    //exit(ret);
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
}
