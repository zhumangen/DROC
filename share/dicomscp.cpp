#include "dicomscp.h"

const QStringList DicomScp::ScpTypeString =
        QStringList() << QObject::tr("Query/Retrieve") <<
                         QObject::tr("Storage") <<
                         QObject::tr("Printer") <<
                         QObject::tr("Worklist");

bool PrintParam::parseDisplayFormat(const QString &formatId, int &col, int &row)
{
    QStringList format = formatId.split(QChar('X'), QString::KeepEmptyParts, Qt::CaseInsensitive);
    if (format.size() == 2) {
        bool ok;
        int c, r;
        c = format.first().remove(QChar(' ')).toInt(&ok);
        if (ok) {
            r = format.last().remove(QChar(' ')).toInt(&ok);
            if (ok) {
                col = c;
                row = r;
                return true;
            }
        }
    }

    return false;
}

bool PrintParam::parseFilmSizeRatio(const QString &sizeId, double &ratio)
{
    QString id = sizeId;
    id.remove("IN", Qt::CaseInsensitive);
    id.remove("CM", Qt::CaseInsensitive);
    id.remove("MM", Qt::CaseInsensitive);
    QStringList filmSize = id.split(QChar('X'), QString::KeepEmptyParts, Qt::CaseInsensitive);
    if (filmSize.size() == 2) {
        bool ok;
        double w, h;
        w = filmSize.first().remove(QChar(' ')).toDouble(&ok);
        if (ok && w>0) {
            h = filmSize.last().remove(QChar(' ')).toDouble(&ok);
            if (ok && h>0) {
                ratio = w / h;
                return true;
            }
        }
    }

    return false;
}
