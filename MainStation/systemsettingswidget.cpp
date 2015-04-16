#include "systemsettingswidget.h"
#include "ui_systemsettingswidget.h"

#include "printparamdialog.h"
#include "localsettings.h"
#include "dicomscpmodel.h"
#include "itemdelegates.h"
#include "usergroupmodels.h"
#include "grouppermissionmodel.h"
#include "grouplistview.h"
#include "../ImageAcquisitor/procedureitemmodel.h"
#include "../ImageAcquisitor/procedureitemdelegate.h"
#include "../DicomService/echoscu.h"
#include "../share/dicomscp.h"
#include "mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QSerialPortInfo>

SystemSettingsWidget::SystemSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemSettingsWidget)
{
    ui->setupUi(this);
    init();
}

SystemSettingsWidget::~SystemSettingsWidget()
{
    delete ui;
}

void SystemSettingsWidget::init()
{
    scpModel = new DicomScpModel(this);
    DicomScpItemDelegate *scpDelegate = new DicomScpItemDelegate(this);
    ui->scpTableView->setModel(scpModel);
    ui->scpTableView->setItemDelegate(scpDelegate);
    ui->scpTableView->horizontalHeader()->setMinimumSectionSize(100);
    ui->scpTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    procModel = new ProcedureItemModel(this);
    ProcedureItemDelegate *procDelegate = new ProcedureItemDelegate(this);
    ui->procTableView->setModel(procModel);
    ui->procTableView->setItemDelegate(procDelegate);
    ui->procTableView->horizontalHeader()->setMinimumSectionSize(100);
    ui->procTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    userModel = new UserModel(this);
    UserItemDelegate *userDelegate = new UserItemDelegate(this);
    ui->userTableView->setModel(userModel);
    ui->userTableView->setItemDelegate(userDelegate);
    ui->userTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->userTableView->horizontalHeader()->setMinimumSectionSize(100);

    groupModel = new GroupModel(this);
    GroupItemDelegate *groupDelegate = new GroupItemDelegate(this);
    permModel = new GroupPermissionModel(this);
    groupView = new GroupListView;
    groupView->setModel(groupModel);
    groupView->setItemDelegate(groupDelegate);
    ui->groupVLayout->insertWidget(0, groupView);
    ui->permissionListView->setModel(permModel);

    QStringList items;
    for (int i = 0; i < DM_DetectorCount; i++) {
        items << DetectorTypeStringTable[i];
    }
    ui->detectorTypeCombo->addItems(items);

    items.clear();
    for (int i = 0; i < GM_GeneratorCount; i++) {
        items << GeneratorTypeStringTable[i];
    }
    ui->generatorTypeCombo->addItems(items);

    items.clear();
    QList<QSerialPortInfo> serialports = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo info, serialports) {
        items << info.portName();
    }
    ui->serialPortCombo->addItems(items);

    createConnections();
    setPermissions();

    settings.loadConfig();
    initLocalSettingsUi();
    userModel->setGroupNames(groupModel->getGroupNames());
}

void SystemSettingsWidget::initLocalSettingsUi()
{
    ui->instNameEdit->setText(settings.instInfo.name);
    ui->instAddrEdit->setText(settings.instInfo.addr);
    ui->instTelEdit->setText(settings.instInfo.tel);
    ui->manuNameEdit->setText(settings.manuInfo.name);
    ui->manuModelEdit->setText(settings.manuInfo.model);
    ui->manuModalityCombo->setCurrentText(settings.manuInfo.modality);
    ui->stationAetEdit->setText(settings.statInfo.aetitle);
    ui->stationNameEdit->setText(settings.statInfo.name);
    ui->detectorTypeCombo->setCurrentIndex(settings.commInfo.detModel);
    ui->generatorTypeCombo->setCurrentIndex(settings.commInfo.genModel);
    ui->serialPortCombo->setCurrentText(settings.commInfo.serialPortName);

    ui->sendAfterAcqCheck->setChecked(settings.wflow.sendAfterAcqusition);
    ui->storeScpCombo->setCurrentText(settings.wflow.storeScpId);
    ui->dbFolderEdit->setText(settings.dbfolder);
    ui->ccw90Check->setChecked(settings.commInfo.ccw90);
    ui->cw90Check->setChecked(settings.commInfo.cw90);
    ui->hflipCheck->setChecked(settings.commInfo.hflip);
    ui->vflipCheck->setChecked(settings.commInfo.vflip);

    ui->patientIdDigitsSpin->setValue(settings.patientId.digits);
    ui->patientIdPrefixEdit->setText(settings.patientId.prefix);
    ui->patientIdSuffixEdit->setText(settings.patientId.suffix);
    ui->accNumDigitsSpin->setValue(settings.accNum.digits);
    ui->accNumPrefixEdit->setText(settings.accNum.prefix);
    ui->accNumSuffixEdit->setText(settings.accNum.suffix);
}

void SystemSettingsWidget::onLocalSettingsSave()
{
    settings.instInfo.name = ui->instNameEdit->text();
    settings.instInfo.addr = ui->instAddrEdit->text();
    settings.instInfo.tel = ui->instTelEdit->text();
    settings.manuInfo.name = ui->manuNameEdit->text();
    settings.manuInfo.model = ui->manuModelEdit->text();
    settings.manuInfo.modality = ui->manuModalityCombo->currentText();
    settings.statInfo.aetitle = ui->stationAetEdit->text();
    settings.statInfo.name = ui->stationNameEdit->text();
    settings.commInfo.detModel = ui->detectorTypeCombo->currentIndex();
    settings.commInfo.genModel = ui->generatorTypeCombo->currentIndex();
    settings.commInfo.serialPortName = ui->serialPortCombo->currentText();

    settings.wflow.sendAfterAcqusition = ui->sendAfterAcqCheck->isChecked();
    settings.wflow.storeScpId = ui->storeScpCombo->currentText().isEmpty()?QString():ui->storeScpCombo->currentText();
    settings.dbfolder = ui->dbFolderEdit->text();
    settings.commInfo.ccw90 = ui->ccw90Check->isChecked();
    settings.commInfo.cw90 = ui->cw90Check->isChecked();
    settings.commInfo.hflip = ui->hflipCheck->isChecked();
    settings.commInfo.vflip = ui->vflipCheck->isChecked();

    settings.patientId.digits = ui->patientIdDigitsSpin->value();
    settings.patientId.prefix = ui->patientIdPrefixEdit->text();
    settings.patientId.suffix = ui->patientIdSuffixEdit->text();
    settings.accNum.digits = ui->accNumDigitsSpin->value();
    settings.accNum.prefix = ui->accNumPrefixEdit->text();
    settings.accNum.suffix = ui->accNumSuffixEdit->text();

    QSettings s;
    s.setValue(PATIENTID_START, ui->patientIdStartSpin->value());
    s.setValue(ACCNUMBER_START, ui->accNumStartSpin->value());

    settings.saveConfig();
    emit detGenModelUpdated();

    QMessageBox::information(this, tr("Save Local Settings"),
                             tr("Local settings saved."));
}

void SystemSettingsWidget::initIdStarts()
{
    QSettings s;
    ui->patientIdStartSpin->setValue(s.value(PATIENTID_START).toInt());
    ui->accNumStartSpin->setValue(s.value(ACCNUMBER_START).toInt());
}

void SystemSettingsWidget::createConnections()
{
    connect(ui->selectButton, SIGNAL(clicked()), this, SLOT(onDbLocationSelect()));
    connect(this, SIGNAL(storeScpUpdated(QList<DicomScp*>)), this, SLOT(onStoreScpChanged(QList<DicomScp*>)));

    connect(ui->setupSaveButton, SIGNAL(clicked()), this, SLOT(onLocalSettingsSave()));
    connect(ui->scpSaveButton, SIGNAL(clicked()), this, SLOT(onScpSave()));
    connect(ui->procSaveButton, SIGNAL(clicked()), this, SLOT(onProcSave()));
    connect(ui->userGroupSaveButton, SIGNAL(clicked()), this, SLOT(onUserGroupSave()));

    connect(ui->scpTableView, SIGNAL(clicked(QModelIndex)), scpModel, SLOT(onItemClicked(QModelIndex)));
    connect(scpModel, SIGNAL(enableMoreOptions(bool)), this, SLOT(onEnableMoreOptions(bool)));
    connect(ui->scpInsertButton, SIGNAL(clicked()), this, SLOT(onScpInsert()));
    connect(ui->scpRemoveButton, SIGNAL(clicked()), this, SLOT(onScpRemove()));
    connect(ui->scpEchoButton, SIGNAL(clicked()), this, SLOT(onScpEcho()));
    connect(ui->scpMoreButton, SIGNAL(clicked()), this, SLOT(onScpMore()));
    connect(ui->procInsertButton, SIGNAL(clicked()), this, SLOT(onProcInsert()));
    connect(ui->procRemoveButton, SIGNAL(clicked()), this, SLOT(onProcRemove()));
    connect(ui->userInsertButton, SIGNAL(clicked()), this, SLOT(onUserInsert()));
    connect(ui->userRemoveButton, SIGNAL(clicked()), this, SLOT(onUserRemove()));
    connect(ui->groupInsertButton, SIGNAL(clicked()), this, SLOT(onGroupInsert()));
    connect(ui->groupRemoveButton, SIGNAL(clicked()), this, SLOT(onGroupRemove()));
    connect(ui->checkAllCheck, SIGNAL(stateChanged(int)), permModel, SLOT(checkAllItems(int)));
    connect(groupView, SIGNAL(currentItemChanged(QModelIndex)), groupModel, SLOT(onCurrentChanged(QModelIndex)));

    connect(groupModel, SIGNAL(groupChanged(QStringList)), userModel, SLOT(setGroupNames(QStringList)));
    connect(groupModel, SIGNAL(currentGroupChanged(ulong, bool)), permModel, SLOT(setPermission(ulong, bool)));
    connect(permModel, SIGNAL(permissionChanged(ulong)), groupModel, SLOT(onPermissionChanged(ulong)));
}

void SystemSettingsWidget::setPermissions()
{
    GroupPermissions perms = mainWindow->getCurrentGroup().permissions;

    ui->setupSaveButton->setEnabled(perms & GP_LocalSettings);
    if (!(perms & GP_RemoteServers))
        ui->scpTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->scpInsertButton->setEnabled(perms & GP_RemoteServers);
    ui->scpRemoveButton->setEnabled(perms & GP_RemoteServers);
    ui->scpSaveButton->setEnabled(perms & GP_RemoteServers);
    if (!(perms & GP_ProcedureParam))
        ui->procTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->procInsertButton->setEnabled(perms & GP_ProcedureParam);
    ui->procRemoveButton->setEnabled(perms & GP_ProcedureParam);
    ui->procSaveButton->setEnabled(perms & GP_ProcedureParam);

    if (!(perms & GP_UserGroupManagement)) {
        ui->userTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        groupView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->permissionListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    ui->userInsertButton->setEnabled(perms & GP_UserGroupManagement);
    ui->userRemoveButton->setEnabled(perms & GP_UserGroupManagement);
    ui->groupInsertButton->setEnabled(perms & GP_UserGroupManagement);
    ui->groupRemoveButton->setEnabled(perms & GP_UserGroupManagement);
    ui->userGroupSaveButton->setEnabled(perms & GP_UserGroupManagement);
    ui->checkAllCheck->setEnabled(perms & GP_UserGroupManagement);

}

void SystemSettingsWidget::resetScps()
{
    QList<DicomScp*> qrscps;
    QList<DicomScp*> storescps;
    QList<DicomScp*> printscps;
    QList<DicomScp*> wlistscps;

    scpModel->getClassifiedScps(qrscps, storescps, printscps, wlistscps);

    emit qrScpUpdated(qrscps);
    emit storeScpUpdated(storescps);
    emit printScpUpdated(printscps);
    emit wlistScpUpdated(wlistscps);
}

void SystemSettingsWidget::onDbLocationSelect()
{
    QSettings s;
    QString dir = s.value(DATABASE_LOCATION, ".").toString();
    dir = QFileDialog::getExistingDirectory(this, tr("Database Location"), dir);
    if (!dir.isEmpty()) {
        s.setValue(DATABASE_LOCATION, dir);
        ui->dbFolderEdit->setText(dir);
    }
}

void SystemSettingsWidget::onStoreScpChanged(const QList<DicomScp *> &scps)
{
    ui->storeScpCombo->clear();
    foreach (DicomScp *scp, scps) {
        ui->storeScpCombo->addItem(scp->id);
    }
    ui->sendAfterAcqCheck->setEnabled(ui->storeScpCombo->count());
}

void SystemSettingsWidget::onEnableMoreOptions(bool yes)
{
    if (mainWindow->getCurrentGroup().permissions & GP_RemoteServers)
        ui->scpMoreButton->setEnabled(yes);
}

void SystemSettingsWidget::onScpInsert()
{
    QModelIndex index = ui->scpTableView->currentIndex();
    if (index.isValid()) {
        scpModel->insertRow(index.row()+1);
    } else {
        scpModel->insertRow(scpModel->rowCount());
    }
}

void SystemSettingsWidget::onScpRemove()
{
    QModelIndex index = ui->scpTableView->currentIndex();
    if (index.isValid()) {
        if (QMessageBox::Ok == QMessageBox::question(this, tr("Remove Server"),
                                                     tr("Are you sure to remove this server?"),
                                                     QMessageBox::Ok|QMessageBox::Cancel)) {
            scpModel->removeRow(index.row());
        }
    }
}

void SystemSettingsWidget::onScpEcho()
{
    QModelIndex index = ui->scpTableView->currentIndex();
    if (index.isValid()) {
        const DicomScp *scp = scpModel->getDicomScp(index);
        if (scp && (!scp->aetitle.isEmpty()) && (!scp->hostname.isEmpty())) {
            QString msg;
            QString ourAet = mainWindow->getStationInfo().aetitle;
            if (ourAet.isEmpty()) ourAet = QString::fromLatin1(DEFAULT_STATION_AET);
            if (echoscu(scp->aetitle, ourAet, scp->hostname, scp->port, msg)) {
                QMessageBox::information(this, tr("Echo SCP"), tr("Echo succeeded."));
            } else {
                QMessageBox::critical(this, tr("Echo SCP"), msg);
            }
        } else {
            QMessageBox::critical(this, tr("Echo SCP"), tr("Invalid SCP."));
        }
    }
}

void SystemSettingsWidget::onScpMore()
{
    QModelIndex index = ui->scpTableView->currentIndex();
    if (index.isValid()) {
        DicomScp *scp = const_cast<DicomScp*>(scpModel->getDicomScp(index));
        if (scp && scp->type==DicomScp::ST_Printer) {
            PrintParamDialog dialog(scp->param, this);
            dialog.exec();
        }
    }

}

void SystemSettingsWidget::onScpSave()
{
    if (scpModel->saveData()) {
        resetScps();
        QMessageBox::information(this, tr("Save Remote SCPs"),
                                 tr("Remote SCPs saved."));
    }
}

void SystemSettingsWidget::onProcInsert()
{
    QModelIndex index = ui->procTableView->currentIndex();
    if (index.isValid()) {
        procModel->insertRow(index.row()+1);
    } else {
        procModel->insertRow(procModel->rowCount());
    }
}

void SystemSettingsWidget::onProcRemove()
{
    QModelIndex index = ui->procTableView->currentIndex();
    if (index.isValid()) {
        if (QMessageBox::Ok == QMessageBox::question(this, tr("Remove Procedure"),
                                                     tr("Are you sure to remove this procedure?"),
                                                     QMessageBox::Ok|QMessageBox::Cancel)) {
            procModel->removeRow(index.row());
        }
    }
}

void SystemSettingsWidget::onProcSave()
{
    if (procModel->saveData()) {
        QMessageBox::information(this, tr("Save Procedure Params"),
                                 tr("Procedure params saved."));
    }
}

void SystemSettingsWidget::onUserInsert()
{
    QModelIndex index = ui->userTableView->currentIndex();
    if (index.isValid()) {
        userModel->insertRow(index.row()+1);
    } else {
        userModel->insertRow(userModel->rowCount());
    }
}

void SystemSettingsWidget::onUserRemove()
{
    QModelIndex index = ui->userTableView->currentIndex();
    if (index.isValid()) {
        if (QMessageBox::Ok == QMessageBox::question(this, tr("Remove User"),
                                                     tr("Are you sure to remove this User?"),
                                                     QMessageBox::Ok|QMessageBox::Cancel)) {
            userModel->removeRow(index.row());
        }
    }
}

void SystemSettingsWidget::onGroupInsert()
{
    QModelIndex index = groupView->currentIndex();
    if (index.isValid()) {
        groupModel->insertRow(index.row()+1);
    } else {
        groupModel->insertRow(groupModel->rowCount());
    }
}

void SystemSettingsWidget::onGroupRemove()
{
    QModelIndex index = groupView->currentIndex();
    if (index.isValid() && index.row()>0) {
        if (QMessageBox::Ok == QMessageBox::question(this, tr("Remove Group"),
                                                     tr("Are you sure to remove this group?"),
                                                     QMessageBox::Ok|QMessageBox::Cancel)) {
            groupModel->removeRow(index.row());
        }
    }
}

void SystemSettingsWidget::onUserGroupSave()
{
    if (userModel->saveData() && groupModel->saveData()) {
        QMessageBox::information(this, tr("Save User/Group Settings"),
                                 tr("User/Group settings saved."));
    }
}
