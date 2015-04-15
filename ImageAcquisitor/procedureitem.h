#ifndef PROCEDUREITEM_H
#define PROCEDUREITEM_H

#include <QStringList>
#include <QObject>
#include <QRectF>

enum ImageEnhancer {
    IE_None,
    IE_AnkleJointAP,
    IE_AnkleJointLAT,
    IE_ChestAP,
    IE_HandAP,
    IE_KneeJointAP,
    IE_KneeJointLAT,
    IE_LSpineAP,
    IE_LSpineLAT,
    //IE_PelvisAP,

    IE_EnhancerCount,
};

struct S_ImageEnhancerNameMap {
    ImageEnhancer ier;
    const char *name;
    const char *config;
};

extern const S_ImageEnhancerNameMap ImageEnhancerNameMap[];

class ProcedureItem {
public:
    enum BodyPart {
        BP_Head,
        BP_Neck,
        BP_Chest,
        BP_Lung,
        BP_Shoulder,
        BP_Waist,
        BP_Elbow,
        BP_Pelvis,
        BP_Hand,
        BP_Knee,
        BP_Foot,
        BP_None,
    };

    enum BodyType {
        BT_Normal,
        BT_Infant,
        BT_Slim,
        BT_Overweight,
    };

    enum BodyPosition {
        BP_PA,
        BP_AP,
        BP_LAT,
        BP_PLAT,
    };

    static const QStringList BodyPartStringTable;
    static const QStringList BodyTypeStringTable;
    static const QStringList BodyPositionStringTable;

    QString procId;
    BodyPart part;
    BodyType type;
    BodyPosition position;
    ImageEnhancer imgEhr;
    double tubeVoltage;
    double tubeCurrent;
    double exposureTime;
    double center;
    double width;
    QRectF rect;

    explicit ProcedureItem(BodyPart bodyPart = BP_Head, BodyType bodyType = BT_Normal, BodyPosition bodyPos = BP_PA) :
        part(bodyPart),
        type(bodyType),
        position(bodyPos)
    {
        init();
    }

    QString bodyPartString() {
        if (part < BodyPartStringTable.size())
            return BodyPartStringTable.at(part);
        else return QObject::tr("Unknown");
    }

    QString bodyTypeString() {
        if (type < BodyTypeStringTable.size())
            return BodyTypeStringTable.at(type);
        else return QObject::tr("Unknown");
    }

    QString bodyPosString() {
        if (position < BodyPositionStringTable.size())
            return BodyPositionStringTable.at(position);
        else return QObject::tr("Unknown");
    }

    friend QDataStream &operator<<(QDataStream &out, const ProcedureItem &param) {
        quint32 bodyPart = param.part;
        quint32 bodyType = param.type;
        quint32 bodyPos = param.position;
        quint32 ier = param.imgEhr;
        out << param.procId << bodyPart << bodyType << bodyPos << ier <<
               param.tubeVoltage << param.tubeCurrent << param.exposureTime <<
               param.center << param.width << param.rect;
        return out;
    }
    friend QDataStream &operator>>(QDataStream &in, ProcedureItem &param) {
        quint32 bodyPart, bodyType, bodyPos, ier;
        in >> param.procId >> bodyPart >> bodyType >> bodyPos >> ier >>
              param.tubeVoltage >> param.tubeCurrent >> param.exposureTime >>
              param.center >> param.width >> param.rect;
        param.part = (BodyPart)bodyPart;
        param.type = (BodyType)bodyType;
        param.position = (BodyPosition)bodyPos;
        param.imgEhr = (ImageEnhancer)ier;
        return in;
    }

private:
    void init() {
        procId = QString("%1_%2_%3").arg(bodyPartString(), bodyTypeString(), bodyPosString());
        tubeVoltage = 70;
        tubeCurrent = 50;
        exposureTime = 100;
        center = 2000;
        width = 4000;
        imgEhr = IE_None;

        qRegisterMetaType<BodyPart>("BodyPart");
    }
};

#endif // PROCEDUREITEM_H
