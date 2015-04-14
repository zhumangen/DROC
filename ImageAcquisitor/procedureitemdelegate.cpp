#include "procedureitemdelegate.h"
#include "procedureitemmodel.h"
#include "procedureitem.h"

#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>

ProcedureItemDelegate::ProcedureItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QWidget *ProcedureItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *combo;
    QDoubleSpinBox *spin;
    switch (index.column()) {
    case ProcedureItemModel::ProcedureId:
        return new QLineEdit(parent);
    case ProcedureItemModel::BodyPart:
        combo = new QComboBox(parent);
        combo->addItems(ProcedureItem::BodyPartStringTable);
        return combo;
    case ProcedureItemModel::BodyPosition:
        combo = new QComboBox(parent);
        combo->addItems(ProcedureItem::BodyPositionStringTable);
        return combo;
    case ProcedureItemModel::BodyType:
        combo = new QComboBox(parent);
        combo->addItems(ProcedureItem::BodyTypeStringTable);
        return combo;
    case ProcedureItemModel::TubeVoltage:
    case ProcedureItemModel::TubeCurrent:
    case ProcedureItemModel::ExposureTime:
        spin = new QDoubleSpinBox(parent);
        spin->setMaximum(300);
        return spin;
    case ProcedureItemModel::WinCenter:
        spin = new QDoubleSpinBox(parent);
        spin->setMinimum(-32768);
        spin->setMaximum(65535);
        return spin;
    case ProcedureItemModel::WinWidth:
        spin = new QDoubleSpinBox(parent);
        spin->setMaximum(65535);
        return spin;
        /*
    case ProcedureItemModel::ImgRotate:
    case ProcedureItemModel::ImgHFlip:
    case ProcedureItemModel::ImgVFlip:
        combo = new QComboBox(parent);
        combo->addItems(ProcedureItem::BoolStringTable);
        return combo;
        */

    case ProcedureItemModel::EnhanceConfig:
        combo = new QComboBox(parent);
        for (int i = 0; i < IE_EnhancerCount; ++i) {
            combo->addItem(QString(ImageEnhancerNameMap[i].name));
        }
        return combo;

    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void ProcedureItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.isValid()) {
        ProcedureItem *item = static_cast<ProcedureItem*>(index.internalPointer());
        switch (index.column()) {
        case ProcedureItemModel::ProcedureId:
            qobject_cast<QLineEdit*>(editor)->setText(item->procId);
            break;
        case ProcedureItemModel::BodyPart:
            qobject_cast<QComboBox*>(editor)->setCurrentIndex(item->part);
            break;
        case ProcedureItemModel::BodyPosition:
            qobject_cast<QComboBox*>(editor)->setCurrentIndex(item->position);
            break;
        case ProcedureItemModel::BodyType:
            qobject_cast<QComboBox*>(editor)->setCurrentIndex(item->type);
            break;
        case ProcedureItemModel::TubeVoltage:
            qobject_cast<QDoubleSpinBox*>(editor)->setValue(item->tubeVoltage);
            break;
        case ProcedureItemModel::TubeCurrent:
            qobject_cast<QDoubleSpinBox*>(editor)->setValue(item->tubeCurrent);
            break;
        case ProcedureItemModel::ExposureTime:
            qobject_cast<QDoubleSpinBox*>(editor)->setValue(item->exposureTime);
            break;
        case ProcedureItemModel::WinCenter:
            qobject_cast<QDoubleSpinBox*>(editor)->setValue(item->center);
            break;
        case ProcedureItemModel::WinWidth:
            qobject_cast<QDoubleSpinBox*>(editor)->setValue(item->width);
            break;
        case ProcedureItemModel::EnhanceConfig:
            qobject_cast<QComboBox*>(editor)->setCurrentIndex(item->imgEhr);
            break;
            /*
        case ProcedureItemModel::ImgRotate:
            qobject_cast<QComboBox*>(editor)->setCurrentIndex(item->rotate);
            break;
        case ProcedureItemModel::ImgHFlip:
            qobject_cast<QComboBox*>(editor)->setCurrentIndex(item->hFlip);
            break;
        case ProcedureItemModel::ImgVFlip:
            qobject_cast<QComboBox*>(editor)->setCurrentIndex(item->vFlip);
            break;
            */
        default:
            QStyledItemDelegate::setEditorData(editor, index);
        }
    }
}

void ProcedureItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    switch (index.column()) {
    case ProcedureItemModel::ProcedureId:
        model->setData(index, qobject_cast<QLineEdit*>(editor)->text());
        break;
    case ProcedureItemModel::BodyPart:
    case ProcedureItemModel::BodyPosition:
    case ProcedureItemModel::BodyType:
    case ProcedureItemModel::EnhanceConfig:
    /*
    case ProcedureItemModel::ImgRotate:
    case ProcedureItemModel::ImgHFlip:
    case ProcedureItemModel::ImgVFlip:
    */
        model->setData(index, qobject_cast<QComboBox*>(editor)->currentIndex());
        break;
    case ProcedureItemModel::TubeVoltage:
    case ProcedureItemModel::TubeCurrent:
    case ProcedureItemModel::ExposureTime:
    case ProcedureItemModel::WinCenter:
    case ProcedureItemModel::WinWidth:
        model->setData(index, qobject_cast<QDoubleSpinBox*>(editor)->value());
        break;
    default:
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
