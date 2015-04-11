#ifndef GRAPHICSARROWITEM_H
#define GRAPHICSARROWITEM_H

#include "abstractpathitem.h"

class GraphicsArrowItem : public AbstractPathItem
{
public:
    explicit GraphicsArrowItem(QGraphicsItem *parent = 0);
    void setActivePoint(const QPointF &point);
    void nextStage() { if (start != end) currentStage = Final; }

private:
    QPainterPath itemPath();
    bool isModified();

    QPointF start;
    QPointF end;
};

#endif // GRAPHICSARROWITEM_H
