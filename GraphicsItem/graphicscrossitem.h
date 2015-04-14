#ifndef GRAPHICSCROSSITEM_H
#define GRAPHICSCROSSITEM_H

#include <QGraphicsPathItem>

class GraphicsCrossItem : public QGraphicsPathItem
{
public:
    explicit GraphicsCrossItem(QGraphicsItem *parent = 0);
    void setHighlight(bool yes);
    QSizeF crossSize();

private:
    QPainterPath path;

};

#endif // GRAPHICSCROSSITEM_H
