#include "graphicsellipseitem.h"
#include "graphicscrossitem.h"
#include "graphicssimpletextitem.h"
#include "../DicomViewer/imageinstance.h"
#include "dcmtk/dcmimgle/diutils.h"

#define TEXT_STYLE "Mean=%1 SD=%2\nMax=%3 Min=%4\nArea=%5cm^2 (%6 px)"

GraphicsEllipseItem::GraphicsEllipseItem(QGraphicsItem *parent):
    dcmInst(0),
    AbstractPathItem(parent)
{
    init();
}

GraphicsEllipseItem::GraphicsEllipseItem(const ImageInstance *inst, QGraphicsItem *parent):
    dcmInst(inst),
    AbstractPathItem(parent)
{
    setDicomInstance(inst);
    init();
}

void GraphicsEllipseItem::setDicomInstance(const ImageInstance *inst)
{
    dcmInst = inst;
    double xs, ys;
    if (dcmInst && dcmInst->getPixSpacing(xs, ys)) {
        xSpacing = xs;
        ySpacing = ys;
    }
}

void GraphicsEllipseItem::setActivePoint(const QPointF &point)
{
    point2->setPos(point);
    update();
}

void GraphicsEllipseItem::init()
{
    point1 = new GraphicsCrossItem(this);
    point2 = new GraphicsCrossItem(this);
    point2->setPos(0, 0);
}

bool GraphicsEllipseItem::isModified()
{
    if (point1->pos() != prev1 || point2->pos() != prev2 || pos() != prevThis) {
        prev1 = point1->pos();
        prev2 = point2->pos();
        prevThis = pos();
        return true;
    }
    return false;
}

void GraphicsEllipseItem::updateTextItem()
{
    textItem->setText(QString());
    if (xSpacing < 0 || ySpacing < 0) return;

    QPointF p1 = mapToParent(point1->pos());
    QPointF p2 = mapToParent(point2->pos());

    double area = PI * (p2.x()-p1.x()) * (p2.y()-p1.y()) * xSpacing * ySpacing * 0.0025; //0.0025 = 0.01 / 4
    if (area < 0) area = -area;
    textItem->setText(QObject::tr("%1cm^2").arg(area, 0, 'f', 2));

    if (dcmInst) {
        EP_Representation r;
        EP_Representation rep;
        QList<uint> valueList;
        uint value;

        if (p1.x() > p2.x()) {
            double temp = p1.x();
            p1.setX(p2.x());
            p2.setX(temp);
        }
        if (p1.y() > p2.y()) {
            double temp = p1.y();
            p1.setY(p2.y());
            p2.setY(temp);
        }


        bool isXonY;
        double a = (p2.x()-p1.x())/2;
        double b = (p2.y()-p1.y())/2;
        double rate = 0;
        if (a > b) {
            isXonY = false;
            rate = b / a;
        } else {
            isXonY = true;
            rate = a / b;
        }
        QPointF center(p1.x()+a, p1.y()+b);
        for (double i = p1.x(); i < p2.x(); i += 1)
            for (double j = p1.y(); j < p2.y(); j += 1) {
                if (isXonY?((i-center.x())*(i-center.x()) + (j-center.y())*(j-center.y()*rate*rate) <= a * a):
                        ((i-center.x())*(i-center.x())*rate*rate + (j-center.y())*(j-center.y()) <= b * b)) {
                    value = dcmInst->getPixelValue(i, j, r);
                    if ((int)r != -1) {
                        rep = r;
                        valueList.append(value);
                    }
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

void GraphicsEllipseItem::resizeItem()
{
    point1->setScale(zoomFactor);
    point2->setScale(zoomFactor);
}

QPointF GraphicsEllipseItem::textItemPos()
{
    return QPointF((point1->x()>point2->x()?point1->x():point2->x())+point1->crossSize().width()*zoomFactor,
                   point1->y()<point2->y()?(point1->y()+(point2->y()-point1->y())/2):(point2->y()+(point1->y()-point2->y())/2));
}

QPainterPath GraphicsEllipseItem::itemPath()
{
    QPainterPath ellipse;
    ellipse.addEllipse(QRectF(point1->pos(), point2->pos()));
    return ellipse;
}
