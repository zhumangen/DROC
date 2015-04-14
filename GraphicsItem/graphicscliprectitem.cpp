#include "graphicscliprectitem.h"
#include <QPen>
#include <QCursor>
#include <QGraphicsScene>

#define NODE_SIZE 32

GraphicsClipRectItem::GraphicsClipRectItem(QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{
    init();
}

GraphicsClipRectItem::GraphicsClipRectItem(QRectF &rect, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent)
{
    init();
}

void GraphicsClipRectItem::init()
{
    setFlag(QGraphicsItem::ItemIsMovable);
    for (int i = 0; i < NodeCount; ++i) {
        QGraphicsRectItem *node = new QGraphicsRectItem(QRectF(0, 0, NODE_SIZE, NODE_SIZE), this);
        node->setAcceptHoverEvents(true);
        node->setFlag(QGraphicsItem::ItemIsMovable);
        nodeList.append(node);
    }

    nodeList[TopLeft]->setCursor(QCursor(Qt::SizeFDiagCursor));
    nodeList[TopMiddle]->setCursor(QCursor(Qt::SizeVerCursor));
    nodeList[TopRight]->setCursor(QCursor(Qt::SizeBDiagCursor));
    nodeList[MiddleRight]->setCursor(QCursor(Qt::SizeHorCursor));
    nodeList[BottomRight]->setCursor(QCursor(Qt::SizeFDiagCursor));
    nodeList[BottomMiddle]->setCursor(QCursor(Qt::SizeVerCursor));
    nodeList[BottomLeft]->setCursor(QCursor(Qt::SizeBDiagCursor));
    nodeList[MiddleLeft]->setCursor(QCursor(Qt::SizeHorCursor));

    repositionNode();
}

void GraphicsClipRectItem::repositionNode()
{
    QRectF rect = this->rect();
    nodeList[TopLeft]->setPos(rect.left()-(NODE_SIZE/2),
                              rect.top()-(NODE_SIZE/2));
    nodeList[TopMiddle]->setPos(rect.left()+(rect.width()-NODE_SIZE)/2,
                                rect.top()-(NODE_SIZE/2));
    nodeList[TopRight]->setPos(rect.right()-NODE_SIZE/2,
                               rect.top()-(NODE_SIZE/2));
    nodeList[MiddleRight]->setPos(rect.right()-NODE_SIZE/2,
                                  rect.top()+(rect.height()-NODE_SIZE)/2);
    nodeList[BottomRight]->setPos(rect.right()-NODE_SIZE/2,
                                  rect.bottom()-NODE_SIZE/2);
    nodeList[BottomMiddle]->setPos(rect.left()+(rect.width()-NODE_SIZE)/2,
                                   rect.bottom()-NODE_SIZE/2);
    nodeList[BottomLeft]->setPos(rect.left()-(NODE_SIZE/2),
                                 rect.bottom()-NODE_SIZE/2);
    nodeList[MiddleLeft]->setPos(rect.left()-(NODE_SIZE/2),
                                 rect.top()+(rect.height()-NODE_SIZE)/2);
}

void GraphicsClipRectItem::setPen(const QPen &pen)
{
    for (int i = 0; i < NodeCount; ++i)
        nodeList.at(i)->setBrush(QBrush(pen.color()));
    QGraphicsRectItem::setPen(pen);
}

QRectF GraphicsClipRectItem::getClipRect() const
{
    return QRectF(nodeList[TopLeft]->pos(), nodeList[BottomRight]->pos());
}

void GraphicsClipRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = this->rect();
    if (nodeList[TopLeft]->pos() !=
            QPointF(rect.left()-(NODE_SIZE/2), rect.top()-(NODE_SIZE/2))) {
        rect.setTopLeft(nodeList[TopLeft]->pos()+QPointF(NODE_SIZE/2, NODE_SIZE/2));
        setRect(rect);
    } else if (nodeList[TopMiddle]->pos() !=
               QPointF(rect.left()+(rect.width()-NODE_SIZE)/2, rect.top()-(NODE_SIZE/2))) {
        rect.setTop(nodeList[TopMiddle]->pos().y()+NODE_SIZE/2);
        setRect(rect);
    } else if (nodeList[TopRight]->pos() !=
               QPointF(rect.right()-NODE_SIZE/2, rect.top()-(NODE_SIZE/2))) {
        rect.setTopRight(nodeList[TopRight]->pos()+QPointF(NODE_SIZE/2, NODE_SIZE/2));
        setRect(rect);
    } else if (nodeList[MiddleRight]->pos() !=
               QPointF(rect.right()-NODE_SIZE/2, rect.top()+(rect.height()-NODE_SIZE)/2)) {
        rect.setRight(nodeList[MiddleRight]->pos().x()+NODE_SIZE/2);
        setRect(rect);
    } else if (nodeList[BottomRight]->pos() !=
               QPointF(rect.right()-NODE_SIZE/2, rect.bottom()-NODE_SIZE/2)) {
        rect.setBottomRight(nodeList[BottomRight]->pos()+QPointF(NODE_SIZE/2, NODE_SIZE/2));
        setRect(rect);
    } else if (nodeList[BottomMiddle]->pos() !=
               QPointF(rect.left()+(rect.width()-NODE_SIZE)/2, rect.bottom()-NODE_SIZE/2)) {
        rect.setBottom(nodeList[BottomMiddle]->y()+NODE_SIZE/2);
        setRect(rect);
    } else if (nodeList[BottomLeft]->pos() !=
               QPointF(rect.left()-(NODE_SIZE/2), rect.bottom()-NODE_SIZE/2)) {
        rect.setBottomLeft(nodeList[BottomLeft]->pos()+QPointF(NODE_SIZE/2, NODE_SIZE/2));
        setRect(rect);
    } else if (nodeList[MiddleLeft]->pos() !=
               QPointF(rect.left()-(NODE_SIZE/2), rect.top()+(rect.height()-NODE_SIZE)/2)) {
        rect.setLeft(nodeList[MiddleLeft]->pos().x()+NODE_SIZE/2);
        setRect(rect);
    }

    QGraphicsRectItem::paint(painter, option, widget);
    this->scene()->update();
}
