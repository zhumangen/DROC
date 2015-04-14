#ifndef GRAPHICSLINEITEM_H
#define GRAPHICSLINEITEM_H

#include "abstractpathitem.h"
#include <QGraphicsLineItem>
#include <QPen>
class GraphicsSimpleTextItem;
class GraphicsCrossItem;
class ImageInstance;

class GraphicsLineItem : public AbstractPathItem
{
public:
    explicit GraphicsLineItem(QGraphicsItem *parent = 0);
    GraphicsLineItem(const ImageInstance *inst, QGraphicsItem *parent = 0);
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

private:
    const ImageInstance *dcmInst;

    GraphicsCrossItem* crossItem1;
    GraphicsCrossItem* crossItem2;

    QPointF prev1;
    QPointF prev2;
};

#endif // GRAPHICSLINEITEM_H
