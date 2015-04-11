#ifndef BODYPROTOGRAPHICSVIEW_H
#define BODYPROTOGRAPHICSVIEW_H

#include <QGraphicsView>
#include "../ImageAcquisitor/procedureitem.h"

class BodyProtoGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit BodyProtoGraphicsView(QWidget *parent = 0);

    ProcedureItem::BodyPart currentBodyPart();

signals:
    void bodyPartChanged(int);

public slots:
    void setBodyPart(ProcedureItem::BodyPart bodyPart);

protected:
    void mousePressEvent(QMouseEvent *e);

private:
    void initScene();
    void createItems();
    QGraphicsScene *scene;
    QGraphicsEllipseItem *currentItem;

    QGraphicsPixmapItem *pixmapItem;
    QGraphicsEllipseItem *headItem;
    QGraphicsEllipseItem *neckItem;
    QGraphicsEllipseItem *chestItem;
    QGraphicsEllipseItem *lungItem;
    QGraphicsEllipseItem *shoulderItem;
    QGraphicsEllipseItem *waistItem;
    QGraphicsEllipseItem *elbowItem;
    QGraphicsEllipseItem *pelvisItem;
    QGraphicsEllipseItem *handItem;
    QGraphicsEllipseItem *kneeItem;
    QGraphicsEllipseItem *footItem;

    static const QPen normalPen;
    static const QPen activedPen;
};

#endif // BODYPROTOGRAPHICSVIEW_H
