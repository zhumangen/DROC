#ifndef ABSTRACTPATHITEM_H
#define ABSTRACTPATHITEM_H

#include <QGraphicsPathItem>
#include <QPen>
class GraphicsSimpleTextItem;
class DicomImage;
#include "dcmtk/dcmimgle/diutils.h"

#define PI double(3.1415926)
#define MARGIN 2

class AbstractPathItem : public QGraphicsPathItem
{
public:
    enum Stage {
        First,
        Second,
        Third,
        Fouth,
        Fifth,
        Final,
    };

    explicit AbstractPathItem(QGraphicsItem *parent = 0);

    virtual void setActivePoint(const QPointF &point) = 0;
    AbstractPathItem::Stage getCurrentStage() { return currentStage; }
    virtual void nextStage() {}
    void setZoomFactor(const double &factor);
    void setPixelSpacing(double x, double y) { xSpacing = x, ySpacing = y; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

private:
    void init();
    virtual void updateTextItem() {}
    virtual QPointF textItemPos() { return QPointF(); }
    virtual QPainterPath itemPath() { return QPainterPath(); }
    virtual void resizeItem() {}
    virtual bool isModified() { return false; }

protected:
    GraphicsSimpleTextItem* textItem;
    Stage currentStage;
    double zoomFactor;
    double xSpacing;
    double ySpacing;
    //DicomImage *dcmImage;

    bool hoverEnter;
    QPen stockPen;
};

#endif // ABSTRACTPATHITEM_H
