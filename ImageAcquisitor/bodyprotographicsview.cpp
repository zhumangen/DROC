#include "bodyprotographicsview.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QMouseEvent>

const QPen BodyProtoGraphicsView::normalPen = QPen(QBrush(Qt::magenta), 2);
const QPen BodyProtoGraphicsView::activedPen = QPen(QBrush(Qt::green), 3);


BodyProtoGraphicsView::BodyProtoGraphicsView(QWidget *parent) :
    scene(new QGraphicsScene(this)),
    currentItem(0),
    QGraphicsView(parent)
{
    initScene();
}

void BodyProtoGraphicsView::initScene()
{
    setScene(scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    createItems();
    setBodyPart(ProcedureItem::BP_Chest);
}

void BodyProtoGraphicsView::createItems()
{
    pixmapItem = scene->addPixmap(QPixmap(":/images/body.png"));
    headItem = scene->addEllipse(95, 6, 91, 50, normalPen);
    neckItem = scene->addEllipse(104, 54, 78, 32, normalPen);
    chestItem = scene->addEllipse(124, 87, 36, 61, normalPen);
    lungItem = scene->addEllipse(159, 91, 42, 69, normalPen);
    shoulderItem = scene->addEllipse(40, 75, 70, 50, normalPen);
    waistItem = scene->addEllipse(125, 148, 36, 58, normalPen);
    elbowItem = scene->addEllipse(25, 130, 48, 65, normalPen);
    pelvisItem = scene->addEllipse(80, 210, 120, 50, normalPen);
    handItem = scene->addEllipse(5, 210, 60, 72, normalPen);
    kneeItem = scene->addEllipse(71, 272, 72, 93, normalPen);
    footItem = scene->addEllipse(71, 372, 68, 80, normalPen);
}

ProcedureItem::BodyPart BodyProtoGraphicsView::currentBodyPart()
{
    if (currentItem == headItem) {
        return ProcedureItem::BP_Head;
    } else if (currentItem == neckItem) {
        return ProcedureItem::BP_Neck;
    } else if (currentItem == chestItem) {
        return ProcedureItem::BP_Chest;
    } else if (currentItem == lungItem) {
        return ProcedureItem::BP_Lung;
    } else if (currentItem == shoulderItem) {
        return ProcedureItem::BP_Shoulder;
    } else if (currentItem == waistItem) {
        return ProcedureItem::BP_Waist;
    } else if (currentItem == elbowItem) {
        return ProcedureItem::BP_Elbow;
    } else if (currentItem == pelvisItem) {
        return ProcedureItem::BP_Pelvis;
    } else if (currentItem == handItem) {
        return ProcedureItem::BP_Hand;
    } else if (currentItem == kneeItem) {
        return ProcedureItem::BP_Knee;
    } else if (currentItem == footItem) {
        return ProcedureItem::BP_Foot;
    } else {
        return ProcedureItem::BP_None;
    }
}

void BodyProtoGraphicsView::setBodyPart(ProcedureItem::BodyPart bodyPart)
{
    if (currentItem) currentItem->setPen(normalPen);
    switch (bodyPart) {
    case ProcedureItem::BP_Head:
        currentItem = headItem;
        break;
    case ProcedureItem::BP_Neck:
        currentItem = neckItem;
        break;
    case ProcedureItem::BP_Chest:
        currentItem = chestItem;
        break;
    case ProcedureItem::BP_Lung:
        currentItem = lungItem;
        break;
    case ProcedureItem::BP_Shoulder:
        currentItem = shoulderItem;
        break;
    case ProcedureItem::BP_Waist:
        currentItem = waistItem;
        break;
    case ProcedureItem::BP_Elbow:
        currentItem = elbowItem;
        break;
    case ProcedureItem::BP_Pelvis:
        currentItem = pelvisItem;
        break;
    case ProcedureItem::BP_Hand:
        currentItem = handItem;
        break;
    case ProcedureItem::BP_Knee:
        currentItem = kneeItem;
        break;
    case ProcedureItem::BP_Foot:
        currentItem = footItem;
        break;
    default:
        currentItem = 0;
    }

    if (currentItem) currentItem->setPen(activedPen);
}

void BodyProtoGraphicsView::mousePressEvent(QMouseEvent *e)
{
    QGraphicsItem *item = itemAt(e->pos());
    if (currentItem) currentItem->setPen(normalPen);

    /*if (item == pixmapItem) {
        currentItem = 0;
        emit bodyPartChanged(ProcedureItem::BP_None);
    } else */
    if (item == headItem) {
        currentItem = headItem;
        emit bodyPartChanged(ProcedureItem::BP_Head);
    } else if (item == neckItem) {
        currentItem = neckItem;
        emit bodyPartChanged(ProcedureItem::BP_Neck);
    } else if (item == chestItem) {
        currentItem = chestItem;
        emit bodyPartChanged(ProcedureItem::BP_Chest);
    } else if (item == lungItem) {
        currentItem = lungItem;
        emit bodyPartChanged(ProcedureItem::BP_Lung);
    } else if (item == shoulderItem) {
        currentItem = shoulderItem;
        emit bodyPartChanged(ProcedureItem::BP_Shoulder);
    } else if (item == waistItem) {
        currentItem = waistItem;
        emit bodyPartChanged(ProcedureItem::BP_Waist);
    } else if (item == elbowItem) {
        currentItem = elbowItem;
        emit bodyPartChanged(ProcedureItem::BP_Elbow);
    } else if (item == pelvisItem) {
        currentItem = pelvisItem;
        emit bodyPartChanged(ProcedureItem::BP_Pelvis);
    } else if (item == handItem) {
        currentItem = handItem;
        emit bodyPartChanged(ProcedureItem::BP_Hand);
    } else if (item == kneeItem) {
        currentItem = kneeItem;
        emit bodyPartChanged(ProcedureItem::BP_Knee);
    } else if (item == footItem) {
        currentItem = footItem;
        emit bodyPartChanged(ProcedureItem::BP_Foot);
    }

    if (currentItem) currentItem->setPen(activedPen);
    QGraphicsView::mousePressEvent(e);
}
