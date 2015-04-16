#include "imageeditgraphicsview.h"

#include "../GraphicsItem/abstractpathitem.h"
#include "../GraphicsItem/graphicsangleitem.h"
#include "../GraphicsItem/graphicsarrowitem.h"
#include "../GraphicsItem/graphicscobbangleitem.h"
#include "../GraphicsItem/graphicsellipseitem.h"
#include "../GraphicsItem/graphicslineitem.h"
#include "../GraphicsItem/graphicsrectitem.h"
#include "../GraphicsItem/graphicscliprectitem.h"
#include "../GraphicsItem/graphicstextmarkdialog.h"
#include "../GraphicsItem/graphicstextmarkitem.h"
#include "../share/global.h"
#include "imageprocessthread.h"
#include "procedureitem.h"
#include "procedureitemmodel.h"
#include "../MainStation/mainwindow.h"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcdatset.h"

#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>
#include <QMouseEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QIcon>
#include <QDebug>
#include <QSettings>
#include <QFontDialog>
#include <QMessageBox>
#include <QPainter>
#include <QApplication>

ImageEditGraphicsView::ImageEditGraphicsView(QWidget *parent) :
    scene(new QGraphicsScene(this)),
    pixmapItem(new QGraphicsPixmapItem),
    posValue(new QGraphicsSimpleTextItem),
    windowItem(new QGraphicsSimpleTextItem),
    zoomItem(new QGraphicsSimpleTextItem),
    xScalorItem(new QGraphicsPathItem),
    yScalorItem(new QGraphicsPathItem),
    clipRectItem(0),
    currentPathItem(0),
    xSpacing(-1.0),
    ySpacing(-1.0),
    factor(1.0),
    manualZoom(false),
    manualPan(false),
    hflip(0),
    vflip(0),
    rotateAngle(0),
    winCenter(0),
    winWidth(0),
    ff(0),
    imageIndex(-1),
    srcBuffer(0),
    destBuffer(0),
    QGraphicsView(parent)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, Qt::black);
    setPalette(palette);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setAlignment(Qt::AlignCenter);
    setScene(scene);
    scene->setSceneRect(-5000, -5000, 10000, 10000);
    pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    pixmapItem->setAcceptHoverEvents(true);
    xScalorItem->setPen(QPen(Qt::green));
    yScalorItem->setPen(QPen(Qt::green));
    posValue->setBrush(Qt::magenta);
    windowItem->setBrush(Qt::green);
    zoomItem->setBrush(Qt::green);
    clipRectItem = new GraphicsClipRectItem(QRectF(0, 0, 2000, 2000), pixmapItem);
    clipRectItem->setPen(QPen(QBrush(Qt::green), 2));

    scene->addItem(pixmapItem);
    scene->addItem(xScalorItem);
    scene->addItem(yScalorItem);
    scene->addItem(posValue);
    scene->addItem(windowItem);
    scene->addItem(zoomItem);
}

ImageEditGraphicsView::~ImageEditGraphicsView()
{
     qDeleteAll(imageList);
}

void ImageEditGraphicsView::clear()
{
    pixmapItem->setPixmap(QPixmap());
    qDeleteAll(flagItemList);
    qDeleteAll(itemList);

    qDeleteAll(imageList);
    imageList.clear();
    xSpacing = 0;
    ySpacing = 0;
    imageIndex = 0;
    rotateAngle = 0;
    hflip = 0;
    vflip = 0;
    manualZoom = false;
    manualPan = false;

    windowItem->setText("");
    updateScalors();
}

void ImageEditGraphicsView::saveImage()
{
    if (clipRectItem->isVisible()) {
        if (QMessageBox::Yes == QMessageBox::warning(this, tr("Clip Image"),
                                                     tr("Image Clipping hasn't finished, apply clipping?"),
                                                     QMessageBox::Yes|QMessageBox::No)) {
            clipImage(false);
        } else {
            clipRectItem->setVisible(false);
        }
    }

    if (ff && imageList.size()) {
        DicomImage *dcmImage = imageList[imageIndex]->createDicomImage();
        if (dcmImage) {
            const DiPixel *pixel = dcmImage->getInterData();
            if ((pixel->getRepresentation() == EPR_Uint16) ||
                    (pixel->getRepresentation() == EPR_Sint16)) {
                Uint16 *pData = static_cast<Uint16*>(const_cast<void*>(pixel->getData()));
                int cols = dcmImage->getWidth();
                int rows = dcmImage->getHeight();

                foreach (QGraphicsSimpleTextItem *item, flagItemList) {
                    QImage pixImage(item->boundingRect().size().toSize()*8, QImage::Format_RGB32);
                    pixImage.fill(0xff000000);
                    QPainter painter(&pixImage);
                    QFont font;
                    font.fromString(QSettings().value(IMAGE_FLAG_FONT).toString());
                    font.setPointSize(font.pointSize()*8);
                    painter.setFont(font);
                    QPen pen(Qt::white);
                    pen.setBrush(Qt::white);
                    painter.setPen(pen);
                    painter.drawText(0, painter.fontMetrics().height()-painter.fontMetrics().descent(), item->text());
                    painter.end();
                    //pixImage = pixImage.scaled(pixImage.size()*8, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                    Uint16 pValue = dcmImage->getPhotometricInterpretation()==EPI_Monochrome1?0:65535;
                    QPoint pos = item->pos().toPoint();
                    for (int i = pos.y(); (i < pixImage.height()+pos.y()) && (i < rows); ++i) {
                        for (int j = pos.x(); (j < pixImage.width()+pos.x()) && (j < cols); ++j) {
                            if (pixImage.pixel(j-pos.x(), i-pos.y()) != QRgb(0xff000000))
                                pData[i*cols+j] = pValue;
                        }
                    }
                }
            }

            dcmImage->flipImage(hflip, vflip);
            dcmImage->rotateImage(rotateAngle%360);

            DcmDataset *dset = ff->getDataset();
            dset->putAndInsertString(DCM_WindowCenter, QString::number(winCenter).toLatin1().data());
            dset->putAndInsertString(DCM_WindowWidth, QString::number(winWidth).toLatin1().data());
            dcmImage->writeImageToDataset(*dset);
            delete dcmImage;
        }

        mainWindow->getProcModel()->setWindow(procId, winCenter, winWidth);
        if (clipRectFirst.isValid())
            mainWindow->getProcModel()->setClipRect(procId, clipRectFirst);
    }
    clear();
}

bool ImageEditGraphicsView::setFileFormat(DcmFileFormat *dff, bool process)
{
    if (dff) {
        DcmDataset *dset = dff->getDataset();

        const char *value = 0;
        dset->findAndGetString(DCM_PixelSpacing, value);
        QStringList spacings = QString::fromLatin1(value).split('\\', QString::SkipEmptyParts);
        if (spacings.size() == 2) {
            xSpacing = spacings.first().toDouble();
            ySpacing = spacings.last().toDouble();
        }

        QRectF rect;
        if (mainWindow->getProcModel()->getClipRect(procId, rect) && rect.isValid())
            clipRectItem->setRect(rect);

        DicomImage *dcmImage = new DicomImage(dff, dset->getOriginalXfer());
        if (dcmImage && (dcmImage->getStatus()==EIS_Normal)) {
            ff = dff;

            dcmImage->rotateImage(rotateAngle);
            dcmImage->flipImage(hflip, vflip);
            dcmImage->setHistogramWindow();
            dcmImage->getWindow(winCenter, winWidth);

            imageList.prepend(dcmImage);
            imageIndex = 0;

            refreshPixmap();
            updateScalors();

            if (process) {
                ImageEnhancer en;
                if ((mainWindow->getProcModel()->getEnhancerConfig(procId, en)) &&
                        (ImageEnhancerNameMap[en].config)) {
                    ImageProcessThread *thread = new ImageProcessThread;
                    connect(thread, SIGNAL(resultReady(bool)), this, SLOT(onProcessingFinished(bool)));
                    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
                    dset->findAndGetUint16Array(DCM_PixelData, srcBuffer, &imageSize);
                    destBuffer = new Uint16[imageSize];
                    QString path = QApplication::applicationDirPath();
                    thread->srcBuf = (const void*)srcBuffer;
                    thread->destBuf = (void*)destBuffer;
                    thread->config = path.append(QString::fromLocal8Bit(ImageEnhancerNameMap[en].config));
                    dset->findAndGetUint16(DCM_BitsStored, thread->depth);
                    dset->findAndGetUint16(DCM_Columns, thread->width);
                    dset->findAndGetUint16(DCM_Rows, thread->height);
                    thread->start();
                } else {
                    rotateAngle = 0;
                    hflip = vflip = 0;
                    getSavedWindow();
                }
            } else {
                rotateAngle = 0;
                hflip = vflip = 0;
                getSavedWindow();
            }

            return true;
        } else {
            delete dcmImage;
            dcmImage = 0;
        }
    }

    return false;
}

void ImageEditGraphicsView::onProcessingFinished(bool ok)
{
    if (ok) {
        ff->getDataset()->putAndInsertUint16Array(DCM_PixelData, destBuffer, imageSize);
        DicomImage *image = new DicomImage(ff, ff->getDataset()->getOriginalXfer());
        if (image->getStatus() == EIS_Normal) {
            image->rotateImage(rotateAngle);
            image->flipImage(hflip, vflip);
            imageList.insert(imageIndex, image);
            getSavedWindow();
            refreshPixmap();
        } else {
            delete image;
        }
    }

    delete[] destBuffer;
    destBuffer = 0;
    rotateAngle = 0;
    hflip = 0;
    vflip = 0;
}

void ImageEditGraphicsView::updateScalors()
{
    QPainterPath scalorX, scalorY;
    if ((!imageList.isEmpty()) && (!pixmapItem->pixmap().isNull()) &&
            xSpacing > 0.000001 && ySpacing > 0.000001) {
        double psX = xSpacing;
        double psY = ySpacing;
        if (rotateAngle % 180) {
            psX = ySpacing;
            psY = xSpacing;
        }

        double widthR = 100 / psX;
        double heightR = 100 / psY;

        double stepX = widthR / 10;
        double tickX = stepX / 4;
        double stepY = heightR / 10;
        double tickY = stepY / 4;

        scalorX.moveTo(0, 0);
        scalorX.lineTo(widthR, 0);
        scalorY.moveTo(0, 0);
        scalorY.lineTo(0, heightR);
        for (int i = 0; i <= 10; ++i) {
            scalorX.moveTo(i * stepX, 0);
            scalorX.lineTo(i * stepX, -tickX);
            scalorY.moveTo(0, i * stepY);
            scalorY.lineTo(tickY, i * stepY);
        }
    }

    xScalorItem->setPath(scalorX);
    yScalorItem->setPath(scalorY);
}

void ImageEditGraphicsView::repositionAuxItems()
{
    QPointF sceneTL = mapToScene(rect().topLeft());
    QPointF sceneBR = mapToScene(rect().bottomRight());

    windowItem->setPos(sceneTL.x(), sceneBR.y()-windowItem->boundingRect().height());
    zoomItem->setPos(sceneTL.x(), sceneBR.y()-windowItem->boundingRect().height()*2);
    posValue->setPos(sceneTL.x(), sceneBR.y()-windowItem->boundingRect().height()*3);
    QSizeF xRect = xScalorItem->boundingRect().size()*factor;
    xScalorItem->setPos(sceneTL.x()+((sceneBR.x()-sceneTL.x())-xRect.width())/2,
                        sceneBR.y()-xRect.height());
    QSizeF yRect = yScalorItem->boundingRect().size()*factor;
    yScalorItem->setPos(sceneTL.x()+yRect.width(),
                        sceneTL.y()+((sceneBR.y()-sceneTL.y())-yRect.height())/2);
}

void ImageEditGraphicsView::getSavedWindow()
{
    if (!ff) return;

    DcmDataset *dset = ff->getDataset();

    const char *value;
    dset->findAndGetString(DCM_ProtocolName, value);
    procId = QString::fromLocal8Bit(value);

    if (!mainWindow->getProcModel()->getWindow(procId, winCenter, winWidth)) {
        dset->findAndGetString(DCM_WindowCenter, value);
        winCenter = QString::fromLatin1(value).toDouble();
        dset->findAndGetString(DCM_WindowWidth, value);
        winWidth = QString::fromLatin1(value).toDouble();
    }
}

void ImageEditGraphicsView::refreshPixmap()
{
    pixmapItem->setPixmap(getPixmap());
    resizePixmapItem();
    repositionAuxItems();
    windowItem->setText(tr("WL:%1 WW:%2").arg(int(winCenter)).arg(int(winWidth)));
}

void ImageEditGraphicsView::resizePixmapItem()
{
    QRectF pixmapRect = pixmapItem->boundingRect();
    QRectF viewRect = this->rect();

    if ((!imageList.isEmpty()) && (!pixmapItem->pixmap().isNull())) {
        if (!manualZoom) {
            if (pixmapRect.width()*viewRect.height() < pixmapRect.height()*viewRect.width())
                factor = viewRect.height()/pixmapRect.height();
            else
                factor = viewRect.width()/pixmapRect.width();
        }

        pixmapItem->setScale(factor);
        pixmapItem->setTransformOriginPoint(pixmapItem->boundingRect().center());
        if (!manualPan) centerOn(pixmapItem);
        zoomItem->setText(tr("Zoom: %1%").arg(factor*100, 0, 'f', 2));

        xScalorItem->setScale(factor);
        yScalorItem->setScale(factor);
        QPen pen = xScalorItem->pen();
        pen.setWidthF(1/factor);
        xScalorItem->setPen(pen);
        yScalorItem->setPen(pen);
        pen = clipRectItem->pen();
        pen.setWidthF(1/factor);
        clipRectItem->setPen(pen);

        foreach (QGraphicsItem* item, itemList) {
            static_cast<AbstractPathItem*>(item)->setZoomFactor(factor);
        }
    } else {
        zoomItem->setText("");
    }
}

QPixmap ImageEditGraphicsView::getPixmap()
{
    QPixmap pixmap;

    if (imageIndex < imageList.size())
        dcm2pixmap(pixmap, imageList[imageIndex], winCenter, winWidth);
    return pixmap;
}

bool ImageEditGraphicsView::dcm2pixmap(QPixmap &pixmap, DicomImage *image, double center, double width)
{
    if (!image) return false;

    BITMAPFILEHEADER lpfh;
    BITMAPINFOHEADER lpih;
    RGBQUAD palette[256];

    memset(&lpfh, 0, sizeof(BITMAPFILEHEADER));
    lpfh.bfType = 0x4d42;  //'B''M'
    lpfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(palette);


    memset(&lpih, 0, sizeof(BITMAPINFOHEADER));
    lpih.biSize = sizeof(BITMAPINFOHEADER);
    lpih.biWidth = image->getWidth();
    lpih.biHeight = image->getHeight();
    lpih.biCompression = BI_RGB;
    lpih.biBitCount = 8;
    lpih.biPlanes = 1;

    memset(palette, 0, sizeof(palette));
    for (int i = 0; i < 256; ++i) {
        palette[i].rgbBlue = i;
        palette[i].rgbGreen = i;
        palette[i].rgbRed = i;
    }

    void *pDIB = NULL;
    if (width <= 0) image->setWindow(0);
    else image->setWindow(center, width);
    int size = image->createWindowsDIB(pDIB, 0, 0, 8, 1, 1);
    if (pDIB==0 || size==0) return false;

    lpfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(palette) + size;

    QByteArray bmp;
    bmp.append((char*)&lpfh, sizeof(BITMAPFILEHEADER));
    bmp.append((char*)&lpih, sizeof(BITMAPINFOHEADER));
    bmp.append((char*)palette, sizeof(palette));
    bmp.append((char*)pDIB, size);

    pixmap.loadFromData(bmp);
    delete pDIB;
    return true;
}

void ImageEditGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QPointF sp = mapToScene(event->pos());
    QPointF ip = pixmapItem->mapFromScene(sp);

    if (event->button() == Qt::RightButton) {
        switch(leftButton.state) {
        case Drawing:
            if (currentPathItem) {
                currentPathItem->ungrabMouse();
                delete currentPathItem;
                currentPathItem = 0;
            }
            break;
        case AddFlag:
            flagItemList.last()->setPos(ip);
            flagItemList.last()->ungrabMouse();
            break;
        }

        leftButton.state = None;
        setCursor(Qt::ArrowCursor);
        setDragMode(QGraphicsView::NoDrag);

        prevMousePos = event->pos();

    } else if (event->button() == Qt::LeftButton) {
        setDragMode(QGraphicsView::NoDrag);
        switch (leftButton.state) {
        case None:
            setDragMode(QGraphicsView::RubberBandDrag);
            break;
        case Drawing:
            if (currentPathItem == NULL) {
                switch (leftButton.type) {
                case DrawLine:
                    currentPathItem = new GraphicsLineItem(pixmapItem);
                    break;
                case DrawAngle:
                    currentPathItem = new GraphicsAngleItem(pixmapItem);
                    break;
                case DrawCobbAngle:
                    currentPathItem = new GraphicsCobbAngleItem(pixmapItem);
                    break;
                case DrawRect:
                    currentPathItem = new GraphicsRectItem(pixmapItem);
                    break;
                case DrawEllipse:
                    currentPathItem = new GraphicsEllipseItem(pixmapItem);
                    break;
                case DrawTextMark:
                    break;
                case DrawArrow:
                    currentPathItem = new GraphicsArrowItem(pixmapItem);
                    break;
                }
                if (currentPathItem) {
                    currentPathItem->setPos(ip);
                    currentPathItem->setZoomFactor(factor);
                    currentPathItem->grabMouse();
                    currentPathItem->setPixelSpacing(xSpacing, ySpacing);
                }
            } else {
                if (leftButton.type == DrawTextMark) {
                    currentPathItem->setPos(ip);
                    currentPathItem->ungrabMouse();
                    itemList.append(currentPathItem);
                    currentPathItem = 0;
                    leftButton.state = None;
                } else {
                    currentPathItem->nextStage();
                    AbstractPathItem::Stage stage = currentPathItem->getCurrentStage();
                    if (stage == AbstractPathItem::Final) {
                        currentPathItem->ungrabMouse();
                        itemList.append(currentPathItem);
                        currentPathItem = 0;
                    }
                }
            }
            break;
        case Zoom:
            manualZoom = true;
            prevMousePos = event->pos();
            break;
        case Pan:
            setCursor(Qt::ClosedHandCursor);
            manualPan = true;
            prevMousePos = event->pos();
            break;
        case AddFlag:
            flagItemList.last()->setPos(ip);
            flagItemList.last()->ungrabMouse();
            leftButton.state = None;
            break;
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void ImageEditGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePoint = mapToScene(event->pos());
    QPointF itemPoint = pixmapItem->mapFromScene(scenePoint);

    if (pixmapItem->contains(itemPoint)) {
        QPoint pos = itemPoint.toPoint();

        EP_Representation r;
        uint value = getPixelValue(pos.x(), pos.y(), r);
        switch (r) {
        case EPR_Sint8:
            posValue->setText(QString("X: %1 Y: %2 Val: %3").arg(pos.x()).arg(pos.y())
                                   .arg((qint8)value));
            break;
        case EPR_Sint16:
            posValue->setText(QString("X: %1 Y: %2 Val: %3").arg(pos.x()).arg(pos.y())
                                   .arg((qint16)value));
            break;
        case EPR_Sint32:
            posValue->setText(QString("X: %1 Y: %2 Val: %3").arg(pos.x()).arg(pos.y())
                                   .arg((qint32)value));
            break;
        case EPR_Uint8:
        case EPR_Uint16:
        case EPR_Uint32:
            posValue->setText(QString("X: %1 Y: %2 Val: %3").arg(pos.x()).arg(pos.y())
                                   .arg(value));
            break;
        default:
            posValue->setText(QString());
            break;
        }
    } else posValue->setText(QString());

    if (event->buttons() & Qt::RightButton) {
        QPoint delta = event->pos() - prevMousePos;
        prevMousePos = event->pos();
        winCenter += delta.y()*32;
        winWidth -= delta.x()*32;
        if (winWidth < 0) winWidth = 0;
        pixmapItem->setPixmap(getPixmap());
        windowItem->setText(tr("WL:%1 WW:%2").arg(int(winCenter)).arg(int(winWidth)));
    } else {
        switch (leftButton.state) {
        case Drawing:
            if (currentPathItem) {
                if (leftButton.type == DrawTextMark) {
                    currentPathItem->setPos(itemPoint);
                } else {
                    currentPathItem->setActivePoint(currentPathItem->mapFromParent(itemPoint));
                    scene->update();
                }
            }
            break;
        case Zoom:
            if (event->buttons() & Qt::LeftButton) {
                scene->clearSelection();
                if (scene->mouseGrabberItem()) scene->mouseGrabberItem()->ungrabMouse();
                int delta = prevMousePos.y() - event->pos().y();
                prevMousePos = event->pos();
                if (delta >= 0) factor *= (1+delta/60.0);
                else factor /=(1+(-delta)/60.0);
                if (factor > 10.0) factor = 10.0;
                if (factor < 0.05) factor = 0.05;
                resizePixmapItem();
                repositionAuxItems();
            }
            break;
        case Pan:
            if ((event->buttons() & Qt::LeftButton)) {
                scene->clearSelection();
                if (scene->mouseGrabberItem()) scene->mouseGrabberItem()->ungrabMouse();
                QPoint deltaPos = event->pos() - prevMousePos;
                prevMousePos = event->pos();
                pixmapItem->setPos(pixmapItem->pos()+deltaPos);
            }
            break;
        case AddFlag:
            flagItemList.last()->setPos(itemPoint);
            break;
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ImageEditGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        switch (leftButton.state) {
        case Drawing:
            if ((leftButton.type == DrawCurve || leftButton.type == DrawTextMark) && currentPathItem) {
                itemList.append(currentPathItem);
                currentPathItem->ungrabMouse();
                currentPathItem->nextStage();
                currentPathItem = 0;
            }
            break;
        case Pan:
            setCursor(Qt::OpenHandCursor);
            break;
        case None:
            if (imageList.size()) {
                QRectF selectedRect = pixmapItem->mapFromScene(mapToScene(rubberBandRect())).boundingRect();
                imageList[imageIndex]->setRoiWindow(selectedRect.x(), selectedRect.y(),
                                       selectedRect.width(), selectedRect.height());
                /*
                QPointF topleftPoint = pixmapItem->mapFromScene(mapToScene(selectedRect.topLeft()));
                QPointF bottomrightPoint = pixmapItem->mapFromScene(mapToScene(selectedRect.bottomRight()));

                imageList[imageIndex]->setRoiWindow(topleftPoint.x(), topleftPoint.y(),
                                       bottomrightPoint.x()-topleftPoint.x(),
                                       bottomrightPoint.y()-topleftPoint.y());
                                       */
                imageList[imageIndex]->getWindow(winCenter, winWidth);
                windowItem->setText(tr("WL:%1 WW:%2").arg(int(winCenter)).arg(int(winWidth)));
                pixmapItem->setPixmap(getPixmap());
            }
            break;
        }
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void ImageEditGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (itemAt(event->pos()) == clipRectItem)
        clipImage(false);
}

void ImageEditGraphicsView::wheelEvent(QWheelEvent *event)
{
    QPoint delta = event->angleDelta();
    manualZoom = true;
    if (delta.y() >= 0) factor *= (1+delta.y()/600.0);
    else factor /=(1+(-delta.y())/600.0);
    if (factor > 10.0) factor = 10.0;
    if (factor < 0.05) factor = 0.05;
    resizePixmapItem();
    repositionAuxItems();
}

void ImageEditGraphicsView::leaveEvent(QEvent *event)
{
    posValue->setText("");
    QGraphicsView::leaveEvent(event);
}

void ImageEditGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        removeCurrent();
    } else {
        QGraphicsView::keyPressEvent(event);
    }
}

void ImageEditGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    resizePixmapItem();
    repositionAuxItems();
}

void ImageEditGraphicsView::addRFlag()
{
    if (scene->mouseGrabberItem()==0 && imageList.size()) {
        QGraphicsSimpleTextItem *item = new QGraphicsSimpleTextItem(pixmapItem);
        item->setBrush(Qt::white);
        item->setScale(8);
        item->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable/*|
                       QGraphicsItem::ItemIgnoresTransformations*/);
        QFont font;
        font.fromString(QSettings().value(IMAGE_FLAG_FONT).toString());
        item->setFont(font);
        item->setText("R");
        item->grabMouse();
        leftButton.state = AddFlag;
        flagItemList.append(item);
        setCursor(Qt::ArrowCursor);
    }
}

void ImageEditGraphicsView::addLFlag()
{
    if (scene->mouseGrabberItem()==0 && imageList.size()) {
        QGraphicsSimpleTextItem *item = new QGraphicsSimpleTextItem(pixmapItem);
        item->setBrush(Qt::white);
        item->setScale(8);
        item->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable/*|
                       QGraphicsItem::ItemIgnoresTransformations*/);
        QFont font;
        font.fromString(QSettings().value(IMAGE_FLAG_FONT).toString());
        item->setFont(font);
        item->setText("L");
        item->grabMouse();
        leftButton.state = AddFlag;
        flagItemList.append(item);
        setCursor(Qt::ArrowCursor);
    }
}

void ImageEditGraphicsView::onFlagFont()
{
    QSettings settings;
    QFont font;
    font.fromString(settings.value(IMAGE_FLAG_FONT).toString());
    bool ok;
    font = QFontDialog::getFont(&ok, font, this);
    if (ok) {
        settings.setValue(IMAGE_FLAG_FONT, font.toString());
    }
}

void ImageEditGraphicsView::clipImage(bool checked)
{
    if (imageList.size()) {
        if (checked) {
            clipRectItem->setVisible(true);
        } else {
            //QRectF bRect = clipRectItem->boundingRect();
            QRectF clipRect = clipRectItem->rect();
            QPointF pos = clipRectItem->mapToParent(clipRect.topLeft());
            clipRectItem->setVisible(false);
            if (!clipRectFirst.isValid()) {
                clipRectFirst.setTopLeft(pos);
                clipRectFirst.setWidth(clipRect.width());
                clipRectFirst.setHeight(clipRect.height());
            }
            Uint16 value = imageList[imageIndex]->getPhotometricInterpretation()==EPI_Monochrome1?65535:0;
            DicomImage *dcmImage = imageList[imageIndex]->createClippedImage(
                        pos.x(), pos.y(), clipRect.width(), clipRect.height(),
                        value);
            if (dcmImage) {
                imageList.insert(imageIndex, dcmImage);
                pixmapItem->setPixmap(getPixmap());
                resizePixmapItem();
                repositionAuxItems();

            }
        }
    }
}

void ImageEditGraphicsView::undoImage()
{
    if (imageIndex < imageList.size()-1) {
        imageIndex++;
        pixmapItem->setPixmap(getPixmap());
        resizePixmapItem();
        repositionAuxItems();
    }
}

void ImageEditGraphicsView::redoImage()
{
    if (imageIndex > 0) {
        imageIndex--;
        pixmapItem->setPixmap(getPixmap());
        resizePixmapItem();
        repositionAuxItems();
    }
}

void ImageEditGraphicsView::inverseImage()
{
    if (imageList.size()) {
        DicomImage *dcmImage = imageList[imageIndex];
        dcmImage->setPolarity(dcmImage->getPolarity()==EPP_Normal?EPP_Reverse:EPP_Normal);
        pixmapItem->setPixmap(getPixmap());
    }
}

void ImageEditGraphicsView::rotateCCW()
{
    rotateAngle += 270;
    //rotateAngle %= 360;

    QRectF pRect = pixmapItem->boundingRect();
    pixmapItem->setTransformOriginPoint(pRect.width()/2, pRect.height()/2);
    pixmapItem->setRotation(rotateAngle);
    //pixmapItem->setTransform(QTransform(0, 1, -1, 0, pRect.width()/2, pRect.height()/2), true);
    if (!manualPan) centerOn(pixmapItem);
    updateScalors();
    repositionAuxItems();
}

void ImageEditGraphicsView::rotateCW()
{
    rotateAngle += 90;
    //rotateAngle %= 360;

    QRectF pRect = pixmapItem->boundingRect();
    pixmapItem->setTransformOriginPoint(pRect.width()/2, pRect.height()/2);
    pixmapItem->setRotation(rotateAngle);
    //pixmapItem->setTransform(QTransform(0, -1, 1, 0, pRect.width()/2, pRect.height()/2), true);
    if (!manualPan) centerOn(pixmapItem);
    updateScalors();
    repositionAuxItems();
}

void ImageEditGraphicsView::zoomImage()
{
    if (scene->mouseGrabberItem()==0) {
        scene->clearSelection();
        leftButton.state = Zoom;
        setCursor(Qt::SizeAllCursor);
    }
}

void ImageEditGraphicsView::hFlip()
{
    QRectF pRect = pixmapItem->boundingRect();
    pixmapItem->setTransformOriginPoint(pRect.width()/2, pRect.height()/2);
    pixmapItem->setTransform(QTransform(-1, 0, 0, 1, pRect.width(), 0), true);
    if (!manualPan) centerOn(pixmapItem);
    repositionAuxItems();
    hflip = !hflip;
}

void ImageEditGraphicsView::vFlip()
{
    QRectF pRect = pixmapItem->boundingRect();
    pixmapItem->setTransformOriginPoint(pRect.width()/2, pRect.height()/2);
    pixmapItem->setTransform(QTransform(1, 0, 0, -1, 0, pRect.height()), true);
    if (!manualPan) centerOn(pixmapItem);
    repositionAuxItems();
    vflip = !vflip;
}

void ImageEditGraphicsView::panImage()
{
    if (scene->mouseGrabberItem()==0) {
        scene->clearSelection();
        leftButton.state = Pan;
        setCursor(Qt::OpenHandCursor);
        manualPan = true;
    }
}

void ImageEditGraphicsView::drawLenght()
{
    if (scene->mouseGrabberItem()==0) {
        leftButton.state = Drawing;
        leftButton.type = DrawLine;
        setCursor(Qt::CrossCursor);
    }
}

void ImageEditGraphicsView::drawAngle()
{
    if (scene->mouseGrabberItem()==0 && imageList.size()) {
        leftButton.state = Drawing;
        leftButton.type = DrawAngle;
        setCursor(Qt::CrossCursor);
    }
}

void ImageEditGraphicsView::drawCobbAngle()
{
    if (scene->mouseGrabberItem()==0 && imageList.size()) {
        leftButton.state = Drawing;
        leftButton.type = DrawCobbAngle;
        setCursor(Qt::CrossCursor);
    }
}

void ImageEditGraphicsView::drawEllipse()
{
    if (scene->mouseGrabberItem()==0 && imageList.size()) {
        leftButton.state = Drawing;
        leftButton.type = DrawEllipse;
        setCursor(Qt::CrossCursor);
    }
}

void ImageEditGraphicsView::drawRectangle()
{
    if (scene->mouseGrabberItem()==0 && imageList.size()) {
        leftButton.state = Drawing;
        leftButton.type = DrawRect;
        setCursor(Qt::CrossCursor);
    }
}

void ImageEditGraphicsView::drawArrow()
{
    if (scene->mouseGrabberItem()==0 && imageList.size()) {
        leftButton.state = Drawing;
        leftButton.type = DrawArrow;
        setCursor(Qt::CrossCursor);
    }
}

void ImageEditGraphicsView::drawText()
{
    if (scene->mouseGrabberItem()==0 && imageList.size()) {
        leftButton.state = Drawing;
        leftButton.type = DrawTextMark;
        GraphicsTextMarkDialog dialog(this);
        if ((dialog.exec() == QDialog::Accepted) &&
                (!dialog.getText().isEmpty())) {
            GraphicsTextMarkItem *item = new GraphicsTextMarkItem(pixmapItem);
            item->setLabelItemText(dialog.getText());
            item->grabMouse();
            currentPathItem = item;
        }
    }
}

void ImageEditGraphicsView::removeCurrent()
{
    QList<QGraphicsItem*> items = scene->selectedItems();
    foreach (QGraphicsItem *item, items) {
        if (item->parentItem() == pixmapItem) {
            //scene->removeItem(item);
            if (itemList.indexOf(item) >= 0)
                itemList.removeOne(item);
            else if (flagItemList.indexOf(static_cast<QGraphicsSimpleTextItem*>(item)) >= 0)
                flagItemList.removeOne(static_cast<QGraphicsSimpleTextItem*>(item));
            delete item;
        }
    }
}

void ImageEditGraphicsView::reset()
{
    pixmapItem->setRotation(0);
    rotateAngle = 0;
    pixmapItem->resetTransform();
    manualZoom = false;
    manualPan = false;
    resizePixmapItem();
    repositionAuxItems();
}

uint ImageEditGraphicsView::getPixelValue(long x, long y, EP_Representation &r) const
{
    if (imageList.size()) {
        DicomImage *dcmImage = imageList[imageIndex];
        const DiPixel* pixel = dcmImage->getInterData();
        if (pixel && (x < (long)dcmImage->getWidth()) && (x >= 0)
                && (y < (long)dcmImage->getHeight()) && (y >= 0)) {
            r = pixel->getRepresentation();
            switch (r) {
            case EPR_Sint8:
            case EPR_Uint8:
                return *((char*)(pixel->getData()) + (y * dcmImage->getWidth() + x));
            case EPR_Sint16:
            case EPR_Uint16:
                return *((short*)(pixel->getData()) + (y * dcmImage->getWidth() + x));
            case EPR_Sint32:
            case EPR_Uint32:
                return *((int*)(pixel->getData()) + (y * dcmImage->getWidth() + x));
            }
        }
    }
    r = (EP_Representation)-1;
    return 0;
}
