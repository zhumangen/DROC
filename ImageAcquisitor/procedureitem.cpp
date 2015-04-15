#include "procedureitem.h"

const QStringList ProcedureItem::BodyPartStringTable = QStringList()
        << QObject::tr("Head") <<
           QObject::tr("Neck") <<
           QObject::tr("Chest") <<
           QObject::tr("Lung") <<
           QObject::tr("Shoulder") <<
           QObject::tr("Waist") <<
           QObject::tr("Elbow") <<
           QObject::tr("Pelvis") <<
           QObject::tr("Hand") <<
           QObject::tr("Knee") <<
           QObject::tr("Foot");

const QStringList ProcedureItem::BodyTypeStringTable = QStringList()
        << QObject::tr("Normal") <<
           QObject::tr("Infant") <<
           QObject::tr("Slim") <<
           QObject::tr("Overweight");

const QStringList ProcedureItem::BodyPositionStringTable = QStringList()
        << QObject::tr("PA") <<
           QObject::tr("AP") <<
           QObject::tr("LAT") <<
           QObject::tr("PLAT");

const S_ImageEnhancerNameMap ImageEnhancerNameMap[] = {
    {IE_None,             "",                  NULL},
    {IE_AnkleJointAP,     "AnkleJointAP",      "IeConfig/AnkleJointAP.iee"},
    {IE_AnkleJointLAT,    "AnkleJointLAT",     "IeConfig/AnkleJointLAT.iee"},
    {IE_ChestAP,          "ChestAP",           "IeConfig/ChestAP.iee"},
    {IE_HandAP,           "HandAP",            "IeConfig/HandAP.iee"},
    {IE_KneeJointAP,      "KneeJointAP",       "IeConfig/KneeJointAP.iee"},
    {IE_KneeJointLAT,     "KneeJointLAT",      "IeConfig/KneeJointLAT.iee"},
    {IE_LSpineAP,         "LSpineAP",          "IeConfig/LSpineAP.iee"},
    {IE_LSpineLAT,        "LSpineLAT",         "IeConfig/LSpineLAT.iee"},
    //{IE_PelvisAP,         "PelvisAP",          "IeConfig/PelvisAP.iee"},
};
