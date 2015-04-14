#include "graphicslineitem.h"
#include "graphicscrossitem.h"
#include "abstractpathitem.h"
#include "graphicssimpletextitem.h"
#include "../DicomViewer/imageinstance.h"

GraphicsLineItem::GraphicsLineItem(QGraphicsItem *parent) :
    AbstractPathItem(parent)
{
    init();
}

GraphicsLineItem::GraphicsLineItem(const ImageInstance *inst, QGraphicsItem *parent):
    AbstractPathItem(parent)
{
    setDicomInstance(inst);
    init();
}

void GraphicsLineItem::setDicomInstance(const ImageInstance *inst)
{
    dcmInst = inst;
    double xs, ys;
    if (dcmInst && dcmInst->getPixSpacing(xs, ys)) {
        xSpacing = xs;
        ySpacing = ys;
    }
}

void GraphicsLineItem::init()
{
    crossItem1 = new GraphicsCrossItem(this);
    crossItem2 = new GraphicsCrossItem(this);
}

void GraphicsLineItem::setActivePoint(const QPointF &point)
{
    crossItem2->setPos(point);
    update();
}

bool GraphicsLineItem::isModified()
{
    if (crossItem1->pos() != prev1 || crossItem2->pos() != prev2) {
        prev1 = crossItem1->pos();
        prev2 = crossItem2->pos();
        return true;
    }
    return false;
}

void GraphicsLineItem::updateTextItem()
{
    textItem->setText(QString());
    if (xSpacing < 0 || ySpacing < 0) return;

    QPointF p1 = mapToParent(crossItem1->pos());
    QPointF p2 = mapToParent(crossItem2->pos());
    double deltaX = (p1.x() - p2.x()) * xSpacing;
    double deltaY = (p1.y() - p2.y()) * ySpacing;

    textItem->setText(QObject::tr("%1 mm").arg(sqrt(deltaX*deltaX + deltaY*deltaY), 0, 'f', 2));
}

void GraphicsLineItem::resizeItem()
{
    crossItem1->setScale(zoomFactor);
    crossItem2->setScale(zoomFactor);
}

QPointF GraphicsLineItem::textItemPos()
{
    if (crossItem1->x() > crossItem2->x())
        return QPointF(crossItem1->x() + crossItem1->crossSize().width()*zoomFactor, crossItem1->y());
    else
        return QPointF(crossItem2->x() + crossItem2->crossSize().width()*zoomFactor, crossItem2->y());
}

QPainterPath GraphicsLineItem::itemPath()
{
    QPainterPath line;
    line.moveTo(crossItem1->pos());
    line.lineTo(crossItem2->pos());
    return line;
}
