#define GLOBAL_CPP
#include "global.h"
#include <QCryptographicHash>

QString encryptPassword(const QString &pwd)
{
    return QCryptographicHash::hash(pwd.toLatin1(), QCryptographicHash::Md5);
}

QString sex2trSex(const QString &sex)
{
    if (sex.contains('M', Qt::CaseInsensitive)) return QObject::tr("M");
    else if (sex.contains('F', Qt::CaseInsensitive)) return QObject::tr("F");
    else return QObject::tr("O");
}

QString trSex2Sex(const QString &trsex)
{
    if (trsex == QObject::tr("M")) return QString("M");
    else if (trsex == QObject::tr("F")) return QString("F");
    else return QString("O");
}

QTime formatDicomTime(const QString &timeStr)
{
    if (timeStr.contains('.')) {
        return QTime::fromString(timeStr, "hhmmss.zzz");
    } else {
        return QTime::fromString(timeStr, TIME_DICOM_FORMAT);
    }
}

const QStringList GroupPermissionStringTable = QStringList()
        << QObject::tr("Examine Study") <<
           QObject::tr("Send Study") <<
           QObject::tr("Remove Study") <<
           QObject::tr("Print Image") <<
           QObject::tr("Create Report") <<
           QObject::tr("Modify Report") <<
           QObject::tr("Verify Report") <<
           QObject::tr("Print Report") <<
           QObject::tr("Import Study") <<
           QObject::tr("Export Study") <<
           QObject::tr("Register Study") <<
           QObject::tr("Modify Study") <<
           QObject::tr("Acquisit Image") <<
           QObject::tr("Local Settings") <<
           QObject::tr("Remote Servers") <<
           QObject::tr("Procedure Parameters") <<
           QObject::tr("User/Group Management") <<
           QObject::tr("Database Operation") <<
           QObject::tr("Modify Diagnostic Template");

const char *DetectorTypeStringTable[] = {
    " ",
    "Samsung 1717SCG",
    "CareRay 1500L",
};

const char *GeneratorTypeStringTable[] = {
    " ",
    "YiJu",
    "CPI",
    "EMD",
    "Sedecal",
};

static int i = qRegisterMetaType<WorklistItem>("WorklistItem");
