#ifndef GRAPHICSRECTITEM_H
#define GRAPHICSRECTITEM_H

#include "abstractpathitem.h"
#include "graphicscrossitem.h"

class ImageInstance;

class GraphicsRectItem : public AbstractPathItem
{
public:
    explicit GraphicsRectItem(QGraphicsItem *parent = 0);
    explicit GraphicsRectItem(const ImageInstance *inst, QGraphicsItem *parent);
    void setActivePoint(const QPointF &point);
    void nextStage() { currentStage = Final; }
    void setDicomInstance(const ImageInstance *inst);

private:
    void init();
    void updateTextItem();
    QPointF textItemPos();
    QPainterPath itemPath();
    bool isModified();
    void resizeItem();

    GraphicsCrossItem *topleft;
    GraphicsCrossItem *bottomright;

    QPointF prevTL;
    QPointF prevBR;
    QPointF prevThis;

    const ImageInstance *dcmInst;
};

#endif // GRAPHICSRECTITEM_H
