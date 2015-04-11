#ifndef GRAPHICSANGLEITEM_H
#define GRAPHICSANGLEITEM_H

#include "abstractpathitem.h"
class QGraphicsItem;
class GraphicsCrossItem;

class GraphicsAngleItem : public AbstractPathItem
{
public:
    explicit GraphicsAngleItem(QGraphicsItem *parent = 0);
    void setActivePoint(const QPointF &point);
    void nextStage();

private:
    void init();
    void updateTextItem();
    QPointF textItemPos();
    QPainterPath itemPath();
    bool isModified();
    void resizeItem();

    GraphicsCrossItem *pointAngle;
    GraphicsCrossItem *point1;
    GraphicsCrossItem *point2;

    QPointF prevPAngle;
    QPointF prevP1;
    QPointF prevP2;

};

#endif // GRAPHICSANGLEITEM_H
