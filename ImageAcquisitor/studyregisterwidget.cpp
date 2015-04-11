#include "studyregisterwidget.h"
#include "ui_studyregisterwidget.h"
#include "worklistitemmodel.h"
#include "../DicomService/wlistscuthread.h"
#include "../DicomService/echoscu.h"
#include "newstudydialog.h"
#include "../MainStation/mainwindow.h"

#include <QSortFilterProxyModel>
#include <QMessageBox>

StudyRegisterWidget::StudyRegisterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StudyRegisterWidget)
{
    ui->setupUi(this);
    init();
}

StudyRegisterWidget::~StudyRegisterWidget()
{
    clearWlistScps();
    delete ui;
}

void StudyRegisterWidget::init()
{
    wlistModel = new WorklistItemModel(this);
    wlistProxyModel = new QSortFilterProxyModel(this);
    wlistProxyModel->setSourceModel(wlistModel);
    ui->wlistTableView->setModel(wlistProxyModel);
    ui->wlistTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    wlistThread = new WlistSCUThread(wlistModel, this);

    onWlistToday();
    createConnections();
    setPermissions();
}

void StudyRegisterWidget::clearWlistScps()
{
    for (int i = 0; i < ui->serverCombo->count(); ++i) {
        delete reinterpret_cast<DicomScp*>(ui->serverCombo->itemData(i).toULongLong());
    }
    ui->serverCombo->clear();
}

void StudyRegisterWidget::setPermissions()
{
    GroupPermissions perm = mainWindow->getCurrentGroup().permissions;
    ui->newStudyButton->setEnabled(perm & GP_RegisterStudy);
    ui->emergencyButton->setEnabled(perm & GP_RegisterStudy);
    ui->acquisitionButton->setEnabled(perm & GP_AcquisitImage);
}

void StudyRegisterWidget::onWlistScpUpdated(const QList<DicomScp *> &scps)
{
    clearWlistScps();

    foreach (DicomScp *scp, scps) {
        DicomScp *newScp = new DicomScp(*scp);
        ui->serverCombo->addItem(newScp->id, (qulonglong)newScp);
    }
}

void StudyRegisterWidget::createConnections()
{
    connect(ui->todayButton, SIGNAL(clicked()), this, SLOT(onWlistToday()));
    connect(ui->thisWeekButton, SIGNAL(clicked()), this, SLOT(onWlistThisWeek()));
    connect(ui->thisMonthButton, SIGNAL(clicked()), this, SLOT(onWlistThisMonth()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(onWlistClear()));
    connect(ui->echoButton, SIGNAL(clicked()), this, SLOT(onWlistScpEcho()));
    connect(ui->searchButton, SIGNAL(clicked(bool)), this, SLOT(onWlistSearch(bool)));
    connect(ui->acquisitionButton, SIGNAL(clicked()), this, SLOT(onWlistBeginStudy()));
    connect(ui->newStudyButton, SIGNAL(clicked()), this, SLOT(onWlistNewStudy()));
    connect(ui->emergencyButton, SIGNAL(clicked()), this, SLOT(onWlistEmergency()));
    connect(ui->wlistTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onWlistDoubleClicked(QModelIndex)));

    connect(wlistThread, SIGNAL(finished()), this, SLOT(onWlistScuFinished()));
}

void StudyRegisterWidget::onWlistToday()
{
    ui->fromCheck->setChecked(true);
    ui->toCheck->setChecked(true);
    ui->fromDateTimeEdit->setDate(QDate::currentDate());
    ui->fromDateTimeEdit->setTime(QTime(0, 0));
    ui->toDateTimeEdit->setDate(QDate::currentDate());
    ui->toDateTimeEdit->setTime(QTime(23, 59, 59, 999));
}

void StudyRegisterWidget::onWlistThisWeek()
{
    ui->fromCheck->setChecked(true);
    ui->toCheck->setChecked(true);
    ui->fromDateTimeEdit->setDate(QDate::currentDate());
    ui->fromDateTimeEdit->setTime(QTime(0, 0));
    ui->toDateTimeEdit->setDate(QDate::currentDate().addDays(6));
    ui->toDateTimeEdit->setTime(QTime(23, 59, 59, 999));
}

void StudyRegisterWidget::onWlistThisMonth()
{
    ui->fromCheck->setChecked(true);
    ui->toCheck->setChecked(true);
    ui->fromDateTimeEdit->setDate(QDate::currentDate());
    ui->fromDateTimeEdit->setTime(QTime(0, 0));
    ui->toDateTimeEdit->setDate(QDate::currentDate().addDays(30));
    ui->toDateTimeEdit->setTime(QTime(23, 59, 59, 999));
}

void StudyRegisterWidget::onWlistSearch(bool checked)
{
    if (checked) {
        DicomScp *scp = reinterpret_cast<DicomScp*>(ui->serverCombo->currentData().toULongLong());
        if (!scp) {
            ui->searchButton->setChecked(false);
            return;
        }
        wlistThread->setWorklistScp(*scp);
        wlistThread->setAccNumber(ui->accNumberEdit->text());
        wlistThread->setPatientId(ui->patientIdEdit->text());
        wlistThread->setPatientName(ui->patientNameEdit->text());
        wlistThread->setFromTime(ui->fromCheck->isChecked()?
                                    ui->fromDateTimeEdit->dateTime():
                                    QDateTime());
        wlistThread->setToTime(ui->toCheck->isChecked()?
                                  ui->toDateTimeEdit->dateTime():
                                  QDateTime());
        wlistThread->setModality(ui->modalityCombo->currentText());
        wlistModel->clearAllItems();
        ui->searchButton->setText(tr("Abort"));
        wlistThread->start();
    } else {
        wlistThread->setAbort(true);
    }
}

void StudyRegisterWidget::onWlistScuFinished()
{
    ui->searchButton->setChecked(false);
    ui->searchButton->setText(tr("Search"));
}

void StudyRegisterWidget::onWlistDoubleClicked(const QModelIndex &index)
{
    if (mainWindow->getCurrentGroup().permissions & GP_RegisterStudy) {
        QModelIndex idx = wlistProxyModel->mapToSource(index);
        if (idx.isValid()) {
            WorklistItem *item = static_cast<WorklistItem*>(idx.internalPointer());
            StudyRecord study;
            study.studyUid = item->studyUid;
            study.studyUid = item->studyUid;
            study.accNumber = item->accNumber;
            study.patientId = item->patientId;
            study.patientName = item->patientName;
            study.patientSex = item->patientSex;
            study.patientBirth = item->patientBirth;
            study.studyTime = QDateTime::currentDateTime();
            study.studyDesc = item->studyDesc;
            study.reqPhysician = item->reqPhysician;
            study.perPhysician = item->schPhysician;
            study.procId = item->reqProcId;
            emit startAcq(study);
            if (ui->hideOldCheck->isChecked()) wlistModel->removeRow(idx.row());
        }
    }
}

void StudyRegisterWidget::onWlistScpEcho()
{
    DicomScp *wlistScp = reinterpret_cast<DicomScp*>(ui->serverCombo->currentData().toULongLong());
    if (!wlistScp || wlistScp->aetitle.isEmpty() || wlistScp->hostname.isEmpty()
            || wlistScp->port <= 0 || wlistScp->port > 65535) {
        QMessageBox::critical(this, tr("Echo Worklist SCP"), tr("Invalid Worklist SCP"));
    } else {
        QString msg;
        QString ourAet = mainWindow->getStationInfo().aetitle;
        if (ourAet.isEmpty()) ourAet = QString::fromLatin1(DEFAULT_STATION_AET);
        if (echoscu(wlistScp->aetitle, ourAet, wlistScp->hostname, wlistScp->port, msg)) {
            QMessageBox::information(this, tr("Echo Worklist SCP"), tr("Echo succeeded."));
        } else {
            QMessageBox::critical(this, tr("Echo Worklist SCP"), tr("Echo failed: %1.").arg(msg));
        }
    }
}

void StudyRegisterWidget::onWlistClear()
{
    ui->patientIdEdit->clear();
    ui->accNumberEdit->clear();
    ui->patientNameEdit->clear();
    ui->modalityCombo->setCurrentIndex(0);
    ui->fromCheck->setChecked(false);
    ui->toCheck->setChecked(false);
}

void StudyRegisterWidget::onWlistBeginStudy()
{
    QModelIndex index = ui->wlistTableView->currentIndex();
    if (index.isValid()) {
        onWlistDoubleClicked(index);
    } else {
        QMessageBox::information(this, tr("Begin Study"), tr("Select an item in the table first."));
    }
}

void StudyRegisterWidget::onWlistNewStudy()
{
    NewStudyDialog dialog(this);
    if (QDialog::Accepted == dialog.exec()) {
        emit startAcq(dialog.getStudyRecord());
    }
}

void StudyRegisterWidget::onWlistEmergency()
{
    NewStudyDialog dialog;
    dialog.onOk();
    emit startAcq(dialog.getStudyRecord());
}
