#include "dicomimageview.h"
#include "imageinstance.h"
#include "dicomimagelabel.h"

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

#include "dcmtk/dcmimgle/dcmimage.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFrame>
#include <QMimeData>

DicomImageView::DicomImageView(ImageInstance *imageInst, QWidget *parent) :
    scene(new QGraphicsScene(this)),
    pixmapItem(new QGraphicsPixmapItem),
    posValue(new QGraphicsSimpleTextItem),
    tlItem(new QGraphicsSimpleTextItem),
    trItem(new QGraphicsSimpleTextItem),
    blItem(new QGraphicsSimpleTextItem),
    brItem(new QGraphicsSimpleTextItem),
    windowItem(new QGraphicsSimpleTextItem),
    zoomItem(new QGraphicsSimpleTextItem),
    xScalorItem(new QGraphicsPathItem),
    yScalorItem(new QGraphicsPathItem),
    currentPathItem(0),
    factor(1.0),
    manualZoom(false),
    manualPan(false),
    hflip(0),
    vflip(0),
    rotateAngle(0),
    mode(VM_Observe),
    rawType(false),
    QGraphicsView(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(QBrush(Qt::black));
    setAcceptDrops(true);
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Plain);
    setAlignment(Qt::AlignCenter);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setHighlight(false);

    scene->setSceneRect(-5000, -5000, 10000, 10000);

    setScene(scene);
    pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    pixmapItem->setAcceptHoverEvents(true);
    xScalorItem->setPen(QPen(Qt::green));
    yScalorItem->setPen(QPen(Qt::green));
    posValue->setBrush(Qt::magenta);
    windowItem->setBrush(Qt::green);
    zoomItem->setBrush(Qt::green);
    tlItem->setBrush(Qt::green);
    trItem->setBrush(Qt::green);
    blItem->setBrush(Qt::green);
    brItem->setBrush(Qt::green);
    scene->addItem(pixmapItem);
    scene->addItem(xScalorItem);
    scene->addItem(yScalorItem);
    scene->addItem(posValue);
    scene->addItem(tlItem);
    scene->addItem(trItem);
    scene->addItem(blItem);
    scene->addItem(brItem);
    scene->addItem(windowItem);
    scene->addItem(zoomItem);

    setImageInstance(imageInst);
}

void DicomImageView::setImageInstance(ImageInstance *imageInst)
{
    image = imageInst;
    emit statusChanged(image != 0);

    refreshPixmap();
    updateScalors();
    updateAnnotations();
    resizePixmapItem();
    repositionAuxItems();

    qDeleteAll(itemList);
    itemList.clear();
}

DicomImage *DicomImageView::getHardCopyGrayScaledImage()
{
    if (image) {
        QPointF tl = pixmapItem->mapFromScene(mapToScene(this->rect().topLeft()));
        QPointF br = pixmapItem->mapFromScene(mapToScene(this->rect().bottomRight()));
        if (tl.x() > br.x()) {
            double temp = br.x();
            br.setX(tl.x());
            tl.setX(temp);
        }
        if (tl.y() > br.y()) {
            double temp = br.y();
            br.setY(tl.y());
            tl.setY(temp);
        }

        return image->createClippedImage(QRectF(tl, br).toRect(), rotateAngle%360,
                                         hflip, vflip,
                                         image->getPolarity()==EPP_Reverse);
    }

    return 0;
}

QImage DicomImageView::getTextLayerImage(const QSize &size)
{
    QImage textlayer(size, QImage::Format_RGB32);
    if (image && (!textlayer.isNull())) {
        textlayer.fill(Qt::black);
        QPainter painter(&textlayer);
        pixmapItem->setVisible(false);
        zoomItem->setVisible(false);
        render(&painter);
        pixmapItem->setVisible(true);
        if (mode != VM_Report) zoomItem->setVisible(true);
        painter.end();
    }
    return textlayer;
}

QPixmap DicomImageView::getHardCopyPixmap()
{
    QPixmap pixmap;
    DicomImage *hgImage = getHardCopyGrayScaledImage();
    if (hgImage && ImageInstance::dcm2bmpHelper(*hgImage, pixmap)) {
        delete hgImage;
        QImage imageLayer = pixmap.toImage();
        int width = imageLayer.width();
        int height = imageLayer.height();
        QImage textLayer = getTextLayerImage(QSize(width, height));
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (textLayer.valid(x, y) && (textLayer.pixel(x, y)!=0xff000000) )
                    imageLayer.setPixel(x, y, 0xffffffff);
            }
        }
        pixmap.convertFromImage(imageLayer);
    }
    return pixmap;
}

bool DicomImageView::hasRawImage() const
{
    return image && image->hasRawImage();
}

void DicomImageView::setImageType(bool rawImage)
{
    if (image) {
        rawType = rawImage;
        image->setImageType(rawImage);
    }
}

bool DicomImageView::getCurrentStudy(StudyRecord &study) const
{
    return image && image->getStudyRecord(study);
}

void DicomImageView::setHighlight(bool yes)
{
    QPalette p = palette();
    if (yes) p.setColor(QPalette::Text, Qt::magenta);
    else p.setColor(QPalette::Text, Qt::gray);
    setPalette(p);
}

void DicomImageView::setViewMode(ViewMode viewMode)
{
    mode = viewMode;
    switch (mode) {
    case VM_Print:
        setVisible(true);
    case VM_Observe:
        setTextItemVisible(true);
        break;
    case VM_Report:
        setTextItemVisible(false);
        break;
    }
}

void DicomImageView::setTextItemVisible(bool yes)
{
    tlItem->setVisible(yes);
    trItem->setVisible(yes);
    blItem->setVisible(yes);
    brItem->setVisible(yes);
    windowItem->setVisible(yes);
    posValue->setVisible(yes);
    zoomItem->setVisible(yes);
    xScalorItem->setVisible(yes);
    yScalorItem->setVisible(yes);
}

void DicomImageView::updateScalors()
{
    double xSpacing, ySpacing;
    QPainterPath scalorX, scalorY;
    if (image) {
        image->getPixSpacing(xSpacing, ySpacing);
        if (xSpacing > 0.000001 && ySpacing > 0.000001) {
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
    }

    xScalorItem->setPath(scalorX);
    yScalorItem->setPath(scalorY);
}

void DicomImageView::updateAnnotations()
{
    if (image) {
        QStringList annos = image->getAnnotations();
        if (annos.size() == 4) {
            tlItem->setText(annos.at(0));
            trItem->setText(annos.at(1));
            blItem->setText(annos.at(2));
            brItem->setText(annos.at(3));
        }
    } else {
        tlItem->setText("");
        trItem->setText("");
        blItem->setText("");
        brItem->setText("");
    }
}

void DicomImageView::repositionAuxItems()
{
    QPointF sceneTL = mapToScene(rect().topLeft());
    QPointF sceneBR = mapToScene(rect().bottomRight());

    tlItem->setPos(sceneTL.x(), sceneTL.y());
    trItem->setPos(sceneBR.x()-trItem->boundingRect().width(), sceneTL.y());
    brItem->setPos(sceneBR.x()-brItem->boundingRect().width(),
                   sceneBR.y()-brItem->boundingRect().height());
    int h2 = windowItem->boundingRect().height();
    int h1 = blItem->boundingRect().height();
    blItem->setPos(sceneTL.x(), sceneBR.y()-h1);
    windowItem->setPos(sceneTL.x(), sceneBR.y()-h1-h2);
    zoomItem->setPos(sceneTL.x(), sceneBR.y()-h1-h2*2);
    posValue->setPos(sceneTL.x(), sceneBR.y()-h1-h2*3);
    QSizeF xRect = xScalorItem->boundingRect().size()*factor;
    xScalorItem->setPos(sceneTL.x()+((sceneBR.x()-sceneTL.x())-xRect.width())/2,
                        sceneBR.y()-xRect.height());
    QSizeF yRect = yScalorItem->boundingRect().size()*factor;
    yScalorItem->setPos(sceneTL.x()+yRect.width(),
                        sceneTL.y()+((sceneBR.y()-sceneTL.y())-yRect.height())/2);
}

void DicomImageView::resizePixmapItem()
{
    if (image) {
        QRectF pixmapRect = pixmapItem->boundingRect();
        QRectF viewRect = this->rect();

        if (pixmapRect.width() == 0 || pixmapRect.height() == 0) return;

        if (!manualZoom) {
            if (pixmapRect.width()*viewRect.height() < pixmapRect.height()*viewRect.width())
                factor = viewRect.height()/pixmapRect.height();
            else
                factor = viewRect.width()/pixmapRect.width();
        }

        pixmapItem->setScale(factor);
        if (!manualPan) centerOn(pixmapItem);
        zoomItem->setText(tr("Zoom: %1%").arg(factor*100, 0, 'f', 2));

        xScalorItem->setScale(factor);
        yScalorItem->setScale(factor);
        QPen pen = xScalorItem->pen();
        pen.setWidthF(1/factor);
        xScalorItem->setPen(pen);
        yScalorItem->setPen(pen);

        foreach (QGraphicsItem* item, itemList) {
            static_cast<AbstractPathItem*>(item)->setZoomFactor(factor);
        }
    } else {
        zoomItem->setText("");
    }
}

void DicomImageView::mousePressEvent(QMouseEvent *event)
{
    if (image) {
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
            }

            leftButton.state = None;
            setCursor(Qt::ArrowCursor);
            setDragMode(QGraphicsView::NoDrag);

            prevMousePos = event->pos();

        } else if (event->button() == Qt::LeftButton) {
            if ((mode!=VM_Report) && (event->modifiers()&Qt::ControlModifier)) {
                clear();
            } else {
                setDragMode(QGraphicsView::NoDrag);
                switch (leftButton.state) {
                case None:
                    if (mode == VM_Report) {
                        setDragMode(QGraphicsView::NoDrag);
                        prevMousePos = event->pos();
                        break;
                    }
                case ROIWindow:
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
                            double xSpacing, ySpacing;
                            image->getPixSpacing(xSpacing, ySpacing);
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
                    //pixmapItem->setTransformOriginPoint(ip);
                    break;
                case Pan:
                    setCursor(Qt::ClosedHandCursor);
                    manualPan = true;
                    prevMousePos = event->pos();
                    break;
                }
            }
        }
    }

    emit viewClicked(this);
    QGraphicsView::mousePressEvent(event);
}

void DicomImageView::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit viewDoubleclicked(this);
    QGraphicsView::mouseDoubleClickEvent(event);
}

void DicomImageView::mouseMoveEvent(QMouseEvent *event)
{
    if (image) {
        QPointF scenePoint = mapToScene(event->pos());
        QPointF itemPoint = pixmapItem->mapFromScene(scenePoint);

        if ((mode==VM_Observe) && pixmapItem->contains(itemPoint)) {
            QPoint pos = itemPoint.toPoint();

            EP_Representation r;
            uint value = image->getPixelValue(pos.x(), pos.y(), r);
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
                posValue->setText("");
                break;
            }
        } else posValue->setText("");

        if ((event->buttons()&Qt::RightButton) ||
                ((mode==VM_Report) && (event->buttons()&Qt::LeftButton) && leftButton.state == None)) {
            QPoint delta = event->pos() - prevMousePos;
            prevMousePos = event->pos();
            image->setImageType(rawType);
            image->setWindowDelta(-delta.y()*10, delta.x()*10);
            refreshPixmap();
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
                    if (factor > 10) factor = 10;
                    if (factor < 0.05) factor = 0.05;
                    //pixmapItem->setTransformOriginPoint(itemPoint);
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
            }
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void DicomImageView::mouseReleaseEvent(QMouseEvent *event)
{
    if (image) {
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
            default:
                if ((!scene->selectedItems().size()) && rubberBandRect().isValid()) {
                    image->setImageType(rawType);
                    image->setRoiWindow(pixmapItem->mapFromScene(mapToScene(rubberBandRect())).boundingRect());
                    refreshPixmap();
                }
                break;
            }
        }
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void DicomImageView::wheelEvent(QWheelEvent *e)
{
    QPoint delta = e->angleDelta();
    manualZoom = true;
    if (delta.y() >= 0) factor *= (1+delta.y()/600.0);
    else factor /=(1+(-delta.y())/600.0);
    if (factor > 10.0) factor = 10.0;
    if (factor < 0.05) factor = 0.05;
    resizePixmapItem();
    repositionAuxItems();
}

void DicomImageView::leaveEvent(QEvent *event)
{
    posValue->setText("");
    QGraphicsView::leaveEvent(event);
}

void DicomImageView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        removeCurrent();
    } else {
        QGraphicsView::keyPressEvent(event);
    }
}

void DicomImageView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    resizePixmapItem();
    repositionAuxItems();
}

void DicomImageView::dragEnterEvent(QDragEnterEvent *e)
{
    DicomImageLabel *label = qobject_cast<DicomImageLabel*>(e->source());
    if (label) e->acceptProposedAction();
}

void DicomImageView::dragMoveEvent(QDragMoveEvent *e)
{
    DicomImageLabel *label = qobject_cast<DicomImageLabel*>(e->source());
    if (label) e->acceptProposedAction();
}

void DicomImageView::dropEvent(QDropEvent *e)
{
    DicomImageLabel *label = qobject_cast<DicomImageLabel*>(e->source());
    if (label) {
        e->acceptProposedAction();
        setImageInstance(label->getImageInstance());
        emit viewClicked(this);
    }
}

void DicomImageView::inverseImage()
{
    if (image) {
        image->setImageType(rawType);
        image->setPolarity(image->getPolarity()==EPP_Normal?EPP_Reverse:EPP_Normal);
        refreshPixmap();
    }
}

void DicomImageView::setRoiWindow()
{
    if (scene->mouseGrabberItem()==0 && image) {
        scene->clearSelection();
        leftButton.state = ROIWindow;
        setCursor(Qt::ArrowCursor);
    }
}

void DicomImageView::setNone()
{
    leftButton.state = None;
    setCursor(Qt::ArrowCursor);
}

void DicomImageView::refreshPixmap()
{
    QPixmap pixmap;
<<<<<<< HEAD
    if (image) {
        image->setImageType(rawType);
        image->getPixmap(pixmap);
=======
    if (image && image->getPixmap(pixmap)) {
        image->setImageType(rawType);
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
        pixmapItem->setPixmap(pixmap);
        pixmapItem->setTransformOriginPoint(pixmapItem->boundingRect().center());
        double center, width;
        image->getWindow(center, width);
        windowItem->setText(tr("WL: %1  WW: %2").arg(center).arg(width));
    } else {
        pixmapItem->setPixmap(pixmap);
        windowItem->setText("");
        posValue->setText("");
    }
}

void DicomImageView::rotateCCW()
{
    if (image) {
        rotateAngle += 270;
        pixmapItem->setRotation(rotateAngle);
    }
}

void DicomImageView::rotateCW()
{
    if (image) {
        rotateAngle += 90;
        pixmapItem->setRotation(rotateAngle);
    }
}

void DicomImageView::zoomImage()
{
    if (scene->mouseGrabberItem()==0 && image) {
        scene->clearSelection();
        leftButton.state = Zoom;
        setCursor(Qt::SizeAllCursor);
    }
}

void DicomImageView::hFlip()
{
    if (image) {
        QRectF pRect = pixmapItem->boundingRect();
        pixmapItem->setTransform(QTransform(-1, 0, 0, 1, pRect.width(), 0), true);
        hflip = !hflip;
    }
}

void DicomImageView::vFlip()
{
    if (image) {
        QRectF pRect = pixmapItem->boundingRect();
        pixmapItem->setTransform(QTransform(1, 0, 0, -1, 0, pRect.height()), true);
        vflip = !vflip;
    }
}

void DicomImageView::panImage()
{
    if (scene->mouseGrabberItem()==0 && image) {
        scene->clearSelection();
        leftButton.state = Pan;
        setCursor(Qt::OpenHandCursor);
        manualPan = true;
    }
}

void DicomImageView::drawLenght()
{
    if (scene->mouseGrabberItem()==0 && image) {
        leftButton.state = Drawing;
        leftButton.type = DrawLine;
        setCursor(Qt::CrossCursor);
    }
}

void DicomImageView::drawAngle()
{
    if (scene->mouseGrabberItem()==0 && image) {
        leftButton.state = Drawing;
        leftButton.type = DrawAngle;
        setCursor(Qt::CrossCursor);
    }
}

void DicomImageView::drawCobbAngle()
{
    if (scene->mouseGrabberItem()==0 && image) {
        leftButton.state = Drawing;
        leftButton.type = DrawCobbAngle;
        setCursor(Qt::CrossCursor);
    }
}

void DicomImageView::drawEllipse()
{
    if (scene->mouseGrabberItem()==0 && image) {
        leftButton.state = Drawing;
        leftButton.type = DrawEllipse;
        setCursor(Qt::CrossCursor);
    }
}

void DicomImageView::drawRectangle()
{
    if (scene->mouseGrabberItem()==0 && image) {
        leftButton.state = Drawing;
        leftButton.type = DrawRect;
        setCursor(Qt::CrossCursor);
    }
}

void DicomImageView::drawArrow()
{
    if (scene->mouseGrabberItem()==0 && image) {
        leftButton.state = Drawing;
        leftButton.type = DrawArrow;
        setCursor(Qt::CrossCursor);
    }
}

void DicomImageView::drawText()
{
    if (scene->mouseGrabberItem()==0 && image) {
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

void DicomImageView::removeCurrent()
{
    QList<QGraphicsItem*> items = scene->selectedItems();
    foreach (QGraphicsItem *item, items) {
        if (item->parentItem() != pixmapItem) {
            item = item->parentItem();
        }
        if (item->parentItem() == pixmapItem) {
            //scene->removeItem(item);
            itemList.removeOne(item);
            delete item;
        }
    }
}

void DicomImageView::reset()
{
    pixmapItem->setRotation(0);
    rotateAngle = 0;
    pixmapItem->resetTransform();
    hflip = false;
    vflip = false;
    manualZoom = false;
    manualPan = false;
    leftButton.state = None;
    setCursor(Qt::ArrowCursor);
    if (image) {
        image->setImageType(rawType);
        if (image->getPolarity() != EPP_Normal)
            image->setPolarity(EPP_Normal);
        image->setDefaultWindow();
        refreshPixmap();
        resizePixmapItem();
        repositionAuxItems();
    }
}
