#ifndef LOCALSETTINGS_H
#define LOCALSETTINGS_H

#include "../share/global.h"

class LocalSettings
{
public:
    LocalSettings(){ loadConfig(); }
    void saveConfig();
    void loadConfig();

    InstitutionInfo instInfo;
    ManufactureInfo manuInfo;
    StationInfo statInfo;
    CommunicationInfo commInfo;
    WorkFlow wflow;
    QString dbfolder;
    CustomizedId patientId;
    CustomizedId accNum;
};

#endif // LOCALSETTINGS_H
