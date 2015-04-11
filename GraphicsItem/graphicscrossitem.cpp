#include "graphicscrossitem.h"
#include <QPen>

#define SIZE 10.0
#define NORMAL_COLOR Qt::red
#define HIGHLIGHT_COLOR Qt::yellow

GraphicsCrossItem::GraphicsCrossItem(QGraphicsItem *parent) :
    QGraphicsPathItem(parent)
{
    setPen(QPen(NORMAL_COLOR));
    setFlags(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);

    path.moveTo(-SIZE/2, 0);
    path.lineTo(SIZE/2, 0);
    path.moveTo(0, -SIZE/2);
    path.lineTo(0, SIZE/2);

    setPath(path);
}

void GraphicsCrossItem::setHighlight(bool yes)
{
    setPen(QPen(yes?HIGHLIGHT_COLOR:NORMAL_COLOR));
}

QSizeF GraphicsCrossItem::crossSize()
{
    return QSizeF(SIZE, SIZE);
}
