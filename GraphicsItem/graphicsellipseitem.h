#ifndef GRAPHICSELLIPSEITEM_H
#define GRAPHICSELLIPSEITEM_H

#include "abstractpathitem.h"
class QGraphicsItem;
class ImageInstance;
class GraphicsCrossItem;

class GraphicsEllipseItem : public AbstractPathItem
{
public:
    explicit GraphicsEllipseItem(QGraphicsItem *parent = 0);
    explicit GraphicsEllipseItem(const ImageInstance *inst, QGraphicsItem *parent = 0);
    void setActivePoint(const QPointF &point);
    void nextStage() { currentStage = Final; }
    void setDicomInstance(const ImageInstance *inst);

private:
    void init();
    void updateTextItem();
    QPointF textItemPos();
    bool isModified();
    QPainterPath itemPath();
    void resizeItem();

    GraphicsCrossItem *point1;
    GraphicsCrossItem *point2;

    QPointF prev1;
    QPointF prev2;
    QPointF prevThis;

    const ImageInstance *dcmInst;
};

#endif // GRAPHICSELLIPSEITEM_H
