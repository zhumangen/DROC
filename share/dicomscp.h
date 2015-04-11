#ifndef DICOMSCP_H
#define DICOMSCP_H
#include <QString>
#include <QDataStream>
#include <QStringList>

struct PrintParam {
    QString sessionLabel;
    QString ownerId;
    QString destination;
    QString priority;
    QString mediumType;

    QString filmOrientation;
    QString displayFormat;
    QString filmSizeId;
    QString resolutionId;
    QString magnificationType;
    QString smoothingType;

    QString borderDensity;
    QString emptyImageDensity;
    QString maxDensity;
    QString minDensity;

    quint32 maxPdu;
    bool disableNewVRs;
    bool implicitOnly;
    bool supports12Bit;
    bool supportsDecimateCrop;
    bool supportsImageSize;
    bool supportsPresentationLut;
    bool supportsTrim;

    PrintParam() {
        maxPdu = 32768;
        disableNewVRs = false;
        implicitOnly = false;
        supports12Bit = true;
        supportsDecimateCrop = true;
        supportsImageSize = true;
        supportsPresentationLut = true;
        supportsTrim = true;
    }

    static bool parseDisplayFormat(const QString &formatId, int &col, int &row);
    static bool parseFilmSizeRatio(const QString &sizeId, double &ratio);

    friend QDataStream &operator<<(QDataStream &out, const PrintParam &param) {
        return out << param.sessionLabel << param.ownerId <<
                      param.destination << param.priority << param.mediumType <<
                      param.filmOrientation << param.displayFormat << param.filmSizeId <<
                      param.resolutionId << param.borderDensity << param.emptyImageDensity <<
                      param.maxDensity << param.minDensity << param.magnificationType <<
                      param.smoothingType << param.maxPdu << param.disableNewVRs <<
                      param.implicitOnly << param.supports12Bit << param.supportsDecimateCrop <<
                      param.supportsImageSize << param.supportsPresentationLut << param.supportsTrim;
    }
    friend QDataStream &operator>>(QDataStream &in, PrintParam &param) {
        return in >> param.sessionLabel >> param.ownerId >>
                     param.destination >> param.priority >> param.mediumType >>
                     param.filmOrientation >> param.displayFormat >> param.filmSizeId >>
                     param.resolutionId >> param.borderDensity >> param.emptyImageDensity >>
                     param.maxDensity >> param.minDensity >> param.magnificationType >>
                     param.smoothingType >> param.maxPdu >> param.disableNewVRs >>
                     param.implicitOnly >> param.supports12Bit >> param.supportsDecimateCrop >>
                     param.supportsImageSize >> param.supportsPresentationLut >> param.supportsTrim;
    }
};

class DicomScp
{  
public:
    enum ScpType {
        ST_QueryRetrieve,
        ST_Storage,
        ST_Printer,
        ST_Worklist,
    };
    static const QStringList ScpTypeString;

    QString id;
    ScpType type;
    QString aetitle;
    QString hostname;
    quint32 port;
    QString description;
    PrintParam param;

    DicomScp(ScpType scptype = ST_QueryRetrieve) :
        type(scptype),
        port(0){}

    friend QDataStream &operator<<(QDataStream &out, const DicomScp &scp) {
        qint32 t = scp.type;
        return out << scp.id << t << scp.aetitle << scp.hostname
                   << scp.port << scp.description << scp.param;
    }
    friend QDataStream &operator>>(QDataStream &in, DicomScp &scp) {
        qint32 t;
        in >> scp.id >> t >> scp.aetitle >> scp.hostname
             >> scp.port >> scp.description >> scp.param;
        scp.type = (ScpType)t;
        return in;
    }
};

typedef DicomScp PrintScp;

#endif // DICOMSCP_H
