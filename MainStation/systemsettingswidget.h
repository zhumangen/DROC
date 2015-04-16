#ifndef SYSTEMSETTINGSWIDGET_H
#define SYSTEMSETTINGSWIDGET_H

#include <QWidget>
#include "../share/global.h"
#include "localsettings.h"
class DicomScpModel;
class UserModel;
class GroupModel;
class GroupPermissionModel;
class GroupListView;
class ProcedureItemModel;
class DicomScp;

namespace Ui {
class SystemSettingsWidget;
}

class SystemSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemSettingsWidget(QWidget *parent = 0);
    ~SystemSettingsWidget();

    const InstitutionInfo& getInstInfo() const { return settings.instInfo; }
    const ManufactureInfo& getManuInfo() const { return settings.manuInfo; }
    const StationInfo& getStationInfo() const { return settings.statInfo; }
    const CommunicationInfo& getCommInfo() const { return settings.commInfo; }
    const WorkFlow& getWorkFlow() const { return settings.wflow; }
    QString getDbLocation() const { return settings.dbfolder; }
    const CustomizedId &getPatientIdFormat() const { return settings.patientId; }
    const CustomizedId &getAccNumFormat() const { return settings.accNum; }
    ProcedureItemModel *getProcModel() const { return procModel; }

signals:
    void qrScpUpdated(const QList<DicomScp*> &scps);
    void storeScpUpdated(const QList<DicomScp*> &scps);
    void printScpUpdated(const QList<DicomScp*> &scps);
    void wlistScpUpdated(const QList<DicomScp*> &scps);
    void detGenModelUpdated();

public slots:
    void resetScps();
    void onLocalSettingsSave();
    void initIdStarts();

protected slots:
    void onDbLocationSelect();
    void onStoreScpChanged(const QList<DicomScp*> &scps);
    void onEnableMoreOptions(bool yes);

    void onScpInsert();
    void onScpRemove();
    void onScpEcho();
    void onScpMore();
    void onScpSave();

    void onProcInsert();
    void onProcRemove();
    void onProcSave();

    void onUserInsert();
    void onUserRemove();
    void onGroupInsert();
    void onGroupRemove();
    void onUserGroupSave();

private:
    void init();
    void initLocalSettingsUi();
    void createConnections();
    void setPermissions();

    Ui::SystemSettingsWidget *ui;

    LocalSettings settings;
    DicomScpModel *scpModel;
    UserModel *userModel;
    GroupModel *groupModel;
    GroupPermissionModel *permModel;
    GroupListView *groupView;
    ProcedureItemModel *procModel;
};

#endif // SYSTEMSETTINGSWIDGET_H
