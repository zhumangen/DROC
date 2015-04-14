#include "localsettings.h"
#include "../share/configfiles.h"

#include <QFile>
#include <QDataStream>

void LocalSettings::saveConfig()
{
    QFile file(LOCALSETTINGS_CFG);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << instInfo << manuInfo <<
               statInfo << commInfo <<
               wflow << dbfolder <<
               patientId << accNum;
        file.close();
    }
}

void LocalSettings::loadConfig()
{
    QFile file(LOCALSETTINGS_CFG);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in >> instInfo >> manuInfo >>
              statInfo >> commInfo >>
              wflow >> dbfolder >>
              patientId >> accNum;
        file.close();
    }

    if (dbfolder.isEmpty()) dbfolder = QString("database");
}
