#include "graphicssimpletextitem.h"
#include <QPen>

GraphicsSimpleTextItem::GraphicsSimpleTextItem(QGraphicsItem *parent):
    QGraphicsSimpleTextItem(parent)
{
    setHighlight(false);
    setFlags(QGraphicsItem::ItemIgnoresTransformations);
}

void GraphicsSimpleTextItem::setHighlight(bool yes)
{
    if (yes) setBrush(QBrush(Qt::magenta));
    else setBrush(QBrush(Qt::green));
}
