#ifndef GLOBAL_H
#define GLOBAL_H
#include <QDate>
#include <QTime>
#include <QStringList>
#include <QDataStream>

#define STUDY_DB_NAME "sqlite/StudyDatabase"
#define LOG_DB_NAME "sqlite/LogDatabase"
#define STUDY_DB_CONNECTION_NAME "STUDYDB"
#define LOG_DB_CONNECTION_NAME "LOGDB"

#define RAW_IMAGE_PREFIX "RW"
#define PRESENT_IMAGE_PREFIX "PR"
#define REPORT_PREFIX "SR"


// Keys of QSettings
#define DATABASE_LOCATION "DATABASELOCATION"
#define STUDY_IMPORT_FOLDER "STUDYIMPORTFOLDER"
#define STUDY_IMPORT_FILE "STUDYIMPORTFILE"
#define LOG_EXPORT_FOLDER "LOGEXPORTFOLDER"
#define IMAGE_CCW_ROTATE "IMAGECCWROTATE"
#define IMAGE_CW_ROTATE "IMAGECWROTATE"
#define IMAGE_HORI_FLIP "IMAGEHORIFLIP"
#define IMAGE_VERT_FLIP "IMAGEVERTFLIP"

#define INCH_MM_RATIO 25.40
#define DATE_DICOM_FORMAT "yyyyMMdd"
#define TIME_DICOM_FORMAT "hhmmss"
#define DATETIME_DICOM_FORMAT "yyyyMMddhhmmss"

enum GroupPermission {
    GP_None = 0x0,
    GP_Examine = 0x1,
    GP_ExamineStudy = GP_Examine,
    GP_SendStudy = 0x2,
    GP_RemoveStudy = 0x4,
    GP_PrintImage = 0x8,
    GP_CreateReport = 0x10,
    GP_ModifyReport = 0x20,
    GP_VerifyReport = 0x40,
    GP_PrintReport = 0x80,
    GP_ImportStudy = 0x100,
    GP_ExportStudy = 0x200,
    GP_RegisterStudy = 0x400,
    GP_ModifyStudy = 0x800,
    GP_AcquisitImage = 0x1000,
    GP_LocalSettings = 0x2000,
    GP_RemoteServers = 0x4000,
    GP_ProcedureParam = 0x8000,
    GP_UserGroupManagement = 0x10000,
    GP_DatabaseOperation = 0x20000,
    GP_ModifyDiagnosticTemplate = 0x40000,
};
Q_DECLARE_FLAGS(GroupPermissions, GroupPermission)
Q_DECLARE_OPERATORS_FOR_FLAGS(GroupPermissions)


#ifndef GLOBAL_CPP
#define GLOBAL_EXTERN extern
#else
#define GLOBAL_EXTERN
#endif

GLOBAL_EXTERN QString encryptPassword(const QString &pwd);
GLOBAL_EXTERN QString sex2trSex(const QString &sex);
GLOBAL_EXTERN QString trSex2Sex(const QString &trsex);
extern const QStringList GroupPermissionStringTable;
GLOBAL_EXTERN QTime formatDicomTime(const QString &timeStr);

struct User{
    QString name;
    QString password;
    QString group;
    QString division;
    QString title;

    User() {
        password = encryptPassword(QString());
    }

    friend QDataStream &operator<<(QDataStream &out, const User &user) {
        return out << user.name << user.password
                   << user.group << user.division << user.title;
    }
    friend QDataStream &operator>>(QDataStream &in, User &user) {
        return in >> user.name >> user.password
                  >> user.group >> user.division >> user.title;
    }
};

struct Group{
    QString name;
    GroupPermissions permissions;

    friend QDataStream &operator<<(QDataStream &out, const Group &group) {
        quint32 perms = group.permissions;
        return out << group.name << perms;
    }
    friend QDataStream &operator>>(QDataStream &in, Group &group) {
        quint32 perms;
        in >> group.name >> perms;
        group.permissions = GroupPermissions(perms);
        return in;
    }
};

#define DEFAULT_STATION_AET "DRDCM"
#define ACCNUMBER_START "ACCNUMBERSTART"
#define PATIENTID_START "PATIENTIDSTART"
#define REQ_PHYSICIANS "REQPHYSICIANS"
#define PER_PHYSICIANS "PERPHYSICIANS"
#define PROCEDURE_IDS "PROCEDUREIDS"
#define STUDY_DESCRIPTION "STUDYDESCRIPTION"
#define SERIES_DESCRIPTION "SERIESDESCRIPTION"
#define IMAGE_FLAG_FONT "IMAGEFLAGFONT"

struct InstitutionInfo {
    QString name;
    QString addr;
    QString tel;

    friend QDataStream &operator<<(QDataStream &out, const InstitutionInfo &info) {
        return out << info.name << info.addr << info.tel;
    }
    friend QDataStream &operator>>(QDataStream &in, InstitutionInfo &info) {
        return in >> info.name >> info.addr >> info.tel;
    }
};

struct ManufactureInfo {
    QString name;
    QString model;
    QString modality;

    ManufactureInfo() { modality = QString::fromLatin1("DX"); }

    friend QDataStream &operator<<(QDataStream &out, const ManufactureInfo &info) {
        return out << info.name << info.model << info.modality;
    }
    friend QDataStream &operator>>(QDataStream &in, ManufactureInfo &info) {
        return in >> info.name >> info.model >> info.modality;
    }
};

struct StationInfo {
    QString aetitle;
    QString name;

    friend QDataStream &operator<<(QDataStream &out, const StationInfo &info) {
        return out << info.aetitle << info.name;
    }
    friend QDataStream &operator>>(QDataStream &in, StationInfo &info) {
        return in >> info.aetitle >> info.name;
    }
};

enum DetectorModel {
    DM_NoDetector,
    DM_Samsung_1717SGC,
    DM_CareRay_1500L,
    DM_DetectorCount,
};
extern const char *DetectorTypeStringTable[];

enum GeneratorModel {
    GM_NoGenerator,
    GM_YiJu,
    GM_CPI,
    GM_EMD,
    GM_Sedecal,
    GM_GeneratorCount,
};
extern const char *GeneratorTypeStringTable[];

struct CommunicationInfo {
    quint32 detModel;
    quint32 genModel;
    QString serialPortName;

    CommunicationInfo() {
        detModel = DM_NoDetector;
        genModel = GM_NoGenerator;
    }

    friend QDataStream &operator<<(QDataStream &out, const CommunicationInfo &info) {
        return out << info.detModel << info.genModel << info.serialPortName;
    }
    friend QDataStream &operator>>(QDataStream &in, CommunicationInfo &info) {
        return in >> info.detModel >> info.genModel >> info.serialPortName;
    }
};

struct DetectorSpec {
    DetectorModel model;
    double xSpacing;
    double ySpacing;
    int bitsAllocated;
    int bitsStored;
    int highBit;
};

struct GeneratorSpec {
    GeneratorModel model;
    QString serialPortName;
    int baudRate;
    int dataBits;
    int parity;
    int stopBits;
    int flowControl;
};

struct WorkFlow {
    bool sendAfterAcqusition;
    bool deleteAfterSend;
    QString storeScpId;
    QString wlistScpId;

    WorkFlow() {
        sendAfterAcqusition = true;
        deleteAfterSend = false;
    }

    friend QDataStream &operator<<(QDataStream &out, const WorkFlow &flow) {
        return out << flow.sendAfterAcqusition << flow.deleteAfterSend <<
                      flow.storeScpId << flow.wlistScpId;
    }
    friend QDataStream &operator>>(QDataStream &in, WorkFlow &flow) {
        return in >> flow.sendAfterAcqusition >> flow.deleteAfterSend >>
                     flow.storeScpId >> flow.wlistScpId;
    }
};

struct CustomizedId {
    bool autoId;
    qint32 digits;
    QString prefix;
    QString suffix;

    CustomizedId() {
        autoId = false;
        digits = 6;
    }

    friend QDataStream &operator<<(QDataStream &out, const CustomizedId &id) {
        return out << id.autoId << id.digits <<
                      id.prefix << id.suffix;
    }
    friend QDataStream &operator>>(QDataStream &in, CustomizedId &id) {
        return in >> id.autoId >> id.digits >>
                     id.prefix >> id.suffix;
    }
};

struct WorklistItem {
    QString accNumber;
    QString patientId;
    QString patientName;
    QString patientSex;
    QDate patientBirth;
    QString patientSize;
    QString patientWeight;
    QString patientPhone;
    QString patientAddr;
    QString reqPhysician;
    QString reqProcId;
    QString reqProcDesc;
    QString reqPriority;
    QString schModality;
    QString schStationName;
    QString schStationAE;
    QString schPhysician;
    QDateTime schDateTime;
    QString schProcStepId;
    QString schProcStepDesc;
    QString studyUid;
    QString studyDesc;
};

Q_DECLARE_METATYPE(WorklistItem)

#endif // GLOBAL_H
