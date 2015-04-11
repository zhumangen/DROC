#include "imageloadthread.h"
#include "imageinstance.h"
#include "../MainStation/mainwindow.h"

void ImageLoadThread::run()
{
    QString dbFolder = mainWindow->getDbLocation();
    foreach (QString file, imageFiles) {
        ImageInstance *image = new ImageInstance(QString("%1/%2").arg(dbFolder, file));
        if (abort) {
            delete image;
            break;
        } else {
            emit resultReady(image);
        }
    }
}
