#ifndef GRAPHICSCLIPRECTITEM_H
#define GRAPHICSCLIPRECTITEM_H

#include <QGraphicsRectItem>
//class GraphicsClipNodeItem;

class GraphicsClipRectItem : public QGraphicsRectItem
{
public:
    enum NodeName {
        TopLeft,
        TopMiddle,
        TopRight,
        MiddleRight,
        BottomRight,
        BottomMiddle,
        BottomLeft,
        MiddleLeft,

        NodeCount,
    };

    explicit GraphicsClipRectItem(QGraphicsItem *parent = 0);
    explicit GraphicsClipRectItem(QRectF &rect, QGraphicsItem *parent = 0);

    void setRect(const QRectF &rect) { QGraphicsRectItem::setRect(rect); repositionNode(); }
    void setPen(const QPen &pen);
    QRectF getClipRect() const;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=0);

private:
    void init();
    void repositionNode();
    QList<QGraphicsRectItem*> nodeList;

};

#endif // GRAPHICSCLIPRECTITEM_H
