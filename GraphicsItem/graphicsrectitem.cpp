#include "graphicsrectitem.h"
#include "graphicssimpletextitem.h"
#include "../DicomViewer/imageinstance.h"

#define TEXT_STYLE "Mean=%1 SD=%2\nMax=%3 Min=%4\nArea=%5cm^2 (%6 px)"

GraphicsRectItem::GraphicsRectItem(QGraphicsItem *parent):
    dcmInst(0),
    AbstractPathItem(parent)
{
    init();
}

GraphicsRectItem::GraphicsRectItem(const ImageInstance *inst, QGraphicsItem *parent):
    AbstractPathItem(parent)
{
    setDicomInstance(inst);
    init();
}

void GraphicsRectItem::setDicomInstance(const ImageInstance *inst)
{
    dcmInst = inst;
    double xs, ys;
    if (dcmInst && dcmInst->getPixSpacing(xs, ys)) {
        xSpacing = xs;
        ySpacing = ys;
    }
}

void GraphicsRectItem::init()
{
    topleft = new GraphicsCrossItem(this);
    bottomright = new GraphicsCrossItem(this);
}

void GraphicsRectItem::setActivePoint(const QPointF &point)
{
    bottomright->setPos(point);
    update();
}

bool GraphicsRectItem::isModified()
{
    if (topleft->pos() != prevTL || bottomright->pos() != prevBR  || pos() != prevThis) {
        prevTL = topleft->pos();
        prevBR = bottomright->pos();
        prevThis = pos();
        return true;
    }
    return false;
}

void GraphicsRectItem::updateTextItem()
{
    textItem->setText(QString());
    if (xSpacing < 0 || ySpacing < 0) return;

    QPointF p1 = mapToParent(topleft->pos());
    QPointF p2 = mapToParent(bottomright->pos());

    double area = (p2.x()-p1.x()) * (p2.y()-p1.y()) * xSpacing * ySpacing * 0.01;
    if (area < 0) area = -area;
    textItem->setText(QObject::tr("%1cm^2").arg(area, 0, 'f', 2));

    if (dcmInst) {
        EP_Representation r;
        EP_Representation rep;
        QList<uint> valueList;
        uint value;

        if (p1.x() > p2.x() && p1.y() < p2.y()) {
            double temp = p1.x();
            p1.setX(p2.x());
            p2.setX(temp);
        } else if (p1.x() > p2.x() && p1.y() > p2.y()) {
            QPointF temp = p1;
            p1 = p2;
            p2 = temp;
        } else if (p1.x() < p2.x() && p1.y() > p2.y()) {
            double temp = p1.y();
            p1.setY(p2.y());
            p2.setY(temp);
        }
        for (double i = p1.x(); i < p2.x(); i += 1)
            for (double j = p1.y(); j < p2.y(); j += 1) {
                value = dcmInst->getPixelValue(i, j, r);
                if ((int)r != -1) {
                    rep = r;
                    valueList.append(value);
                }
            }

        if (valueList.size() == 0) return;

        double mean = 0;
        double SD = 0;

        switch (rep) {
        case EPR_Sint8:
        case EPR_Sint16:
        case EPR_Sint32: {
            qint64 sum = 0;
            int maxVal = valueList.first();
            int minVal = maxVal;
            foreach (uint value, valueList) {
                sum += (int)value;
                if ((int)value > maxVal) maxVal = value;
                else if ((int)value < minVal) minVal = value;
            }
            mean = double(sum) / valueList.size();
            foreach (uint value, valueList) {
                double delta = double(int(value)) - mean;
                SD += (delta * delta);
            }
            textItem->setText(QObject::tr(TEXT_STYLE).arg(mean, 0, 'f', 2).arg(sqrt(SD), 0, 'f', 2).arg(maxVal).arg(minVal)
                              .arg(area, 0, 'f', 2).arg(valueList.size()));
            break;
        }
        case EPR_Uint8:
        case EPR_Uint16:
        case EPR_Uint32: {
            quint64 sum = 0;
            uint maxVal = valueList.first();
            uint minVal = maxVal;
            foreach (uint value, valueList) {
                sum += value;
                if (value > maxVal) maxVal = value;
                else if (value < minVal) minVal = value;
            }
            mean = double(sum) / valueList.size();
            foreach (uint value, valueList) {
                double delta = (double)value - mean;
                SD += delta * delta;
            }
            textItem->setText(QObject::tr(TEXT_STYLE).arg(mean, 0, 'f', 2).arg(sqrt(SD), 0, 'f', 2).arg(maxVal).arg(minVal)
                              .arg(area, 0, 'f', 2).arg(valueList.size()));
            break;
        }
        }
    }
}

QPointF GraphicsRectItem::textItemPos()
{
    return QPointF((bottomright->x()>topleft->x()?bottomright->x():topleft->x())+bottomright->crossSize().width()*zoomFactor,
                   topleft->y()<bottomright->y()?(topleft->y()+(bottomright->y()-topleft->y())/2):(bottomright->y()+(topleft->y()-bottomright->y())/2));
}

void GraphicsRectItem::resizeItem()
{
    topleft->setScale(zoomFactor);
    bottomright->setScale(zoomFactor);
}

QPainterPath GraphicsRectItem::itemPath()
{
    QPainterPath rect;
    rect.addRect(topleft->x(), topleft->y(), bottomright->x()-topleft->x(), bottomright->y()-topleft->y());
    return rect;
}
