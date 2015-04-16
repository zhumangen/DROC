#define MAINWINDOW_CPP
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmpstat/dviface.h"
#include "mainwindow.h"
#include "../share/configfiles.h"
#include "../share/global.h"
#include "ui_mainwindow.h"
#include "timelabel.h"

#include "aboutdialog.h"
#include "../ImageAcquisitor/studyregisterwidget.h"
#include "../ImageAcquisitor/imageacquisitwidget.h"
#include "studyexplorerwidget.h"
#include "archivewidget.h"
#include "systemsettingswidget.h"
#include "exitwidget.h"
#include "../StructuredReport/reporteditwidget.h"
#include "../DicomViewer/imageviewwidget.h"
#include "studydbmanager.h"
#include "logdbmanager.h"

#include <QCloseEvent>
#include <QSplashScreen>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent) :
    prevTab(0),
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::MainWindow(const User &u, const Group &g, QWidget *parent) :
    currentUser(u),
    currentGroup(g),
    prevTab(0),
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    mainWindow = this;

    QSplashScreen splash(QPixmap(":/images/splash.jpg"));
    splash.show();

    splash.showMessage(tr("Creating modules..."), Qt::AlignCenter, Qt::green);
    createComponents();
    setupComponents();
    createConnections();

    settingsTab->resetScps();
}

void MainWindow::createComponents()
{
    studyExplorerTab = new StudyExplorerWidget;
    imageViewTab = new ImageViewWidget;
    reportEditTab = new ReportEditWidget;
    archiveTab = new ArchiveWidget;
    settingsTab = new SystemSettingsWidget;
    exitTab = new ExitWidget;

    registerTab = new StudyRegisterWidget;
    acquisitTab = new ImageAcquisitWidget;

    statusBar()->addPermanentWidget(new QLabel(tr("User: ").append(currentUser.name)));
    statusBar()->addPermanentWidget(new QLabel(tr("Group: ").append(currentGroup.name)));
    statusBar()->addPermanentWidget(new TimeLabel);
}

void MainWindow::createConnections()
{
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged()));

    connect(settingsTab, SIGNAL(qrScpUpdated(QList<DicomScp*>)),
            studyExplorerTab, SLOT(onQRScpUpdated(QList<DicomScp*>)));
    connect(settingsTab, SIGNAL(printScpUpdated(QList<DicomScp*>)),
            imageViewTab, SLOT(onPrintScpUpdated(QList<DicomScp*>)));
    connect(settingsTab, SIGNAL(wlistScpUpdated(QList<DicomScp*>)),
            registerTab, SLOT(onWlistScpUpdated(QList<DicomScp*>)));
    connect(settingsTab, SIGNAL(storeScpUpdated(QList<DicomScp*>)),
            acquisitTab, SLOT(onStoreScpChanged(QList<DicomScp*>)));
    connect(settingsTab, SIGNAL(detGenModelUpdated()),
            acquisitTab, SLOT(updateDetGenModels()));

    connect(studyExplorerTab, SIGNAL(viewImages(QStringList)),
            this, SLOT(onViewImages(QStringList)));
    connect(studyExplorerTab, SIGNAL(createReport(QString)),
            this, SLOT(onCreateReport(QString)));
    connect(studyExplorerTab, SIGNAL(viewReports(QStringList)),
            this, SLOT(onviewReports(QStringList)));
    connect(studyExplorerTab, SIGNAL(startAcq(StudyRecord)),
            this, SLOT(onStartAcq(StudyRecord)));

    connect(reportEditTab, SIGNAL(reportCreated()), studyExplorerTab, SIGNAL(reportCreated()));
    connect(registerTab, SIGNAL(startAcq(StudyRecord)), this, SLOT(onStartAcq(StudyRecord)));
    connect(acquisitTab, SIGNAL(acquisitEnd()), this, SLOT(onEndAcq()));
    connect(acquisitTab, SIGNAL(imageDoubleClicked(QStringList)),
            this, SLOT(onViewImages(QStringList)));
    connect(archiveTab, SIGNAL(importFinished()), studyExplorerTab, SLOT(onStudySearch()));
}

void MainWindow::onViewImages(const QStringList &imageFiles)
{
    imageViewTab->setImageFiles(imageFiles);
    imageViewTab->setObserveMode(true);
    ui->tabWidget->setCurrentWidget(imageViewTab);
}

void MainWindow::onCreateReport(const QString &studyUid)
{
    reportEditTab->createReport(studyUid);
    ui->tabWidget->setCurrentWidget(reportEditTab);
}

void MainWindow::onviewReports(const QStringList &reportFiles)
{
    reportEditTab->openReports(reportFiles);
    ui->tabWidget->setCurrentWidget(reportEditTab);
}

void MainWindow::onStartAcq(const StudyRecord &study)
{
    acquisitTab->onStartAcq(study);
    ui->tabWidget->setCurrentWidget(acquisitTab);
}

void MainWindow::onEndAcq()
{
    studyExplorerTab->onAcqEnd();
    ui->tabWidget->setCurrentWidget(studyExplorerTab);
}

void MainWindow::onCurrentTabChanged()
{
    QWidget *widget = ui->tabWidget->currentWidget();

    if (widget == acquisitTab) {
        if (prevTab == imageViewTab) {
            StudyRecord study;
            if (imageViewTab->getCurrentStudy(study)) {
                acquisitTab->onStartAcq(study);
            }
        }/* else if (prevTab == studyExplorerTab) {
            if (!acquisitTab->hasStudy()) {
                ui->tabWidget->setCurrentWidget(prevTab);
                studyExplorerTab->onStudyAcquisit();
                return;
            }
        }*/

        if (!acquisitTab->hasStudy()) {
            ui->tabWidget->setCurrentWidget(prevTab);
            if (QMessageBox::question(this, tr("Image Acquisit"),
                                      tr("A study should be registered before starting acquisition, register one now?"),
                                      QMessageBox::Yes|QMessageBox::No) ==
                    QMessageBox::Yes) {
                ui->tabWidget->setCurrentWidget(registerTab);
            }
        } else {
            prevTab = widget;
        }
    } else if (widget == reportEditTab) {
        if (prevTab == imageViewTab) {
            StudyRecord study;
            if (imageViewTab->getCurrentStudy(study)) {
                if (!reportEditTab->setActiveReport(study.studyUid))
                    studyExplorerTab->openOrCreateReport(study.studyUid);
            }
        }
        prevTab = widget;
    } else if (widget == settingsTab) {
        settingsTab->initIdStarts();
        prevTab = widget;
    } else if (widget == exitTab) {
        if ((!close()) && prevTab)
            ui->tabWidget->setCurrentWidget(prevTab);
    } else {
        prevTab = widget;
    }
}

void MainWindow::setupComponents()
{
    ui->tabWidget->addTab(registerTab, tr("Patient Register"));
    ui->tabWidget->addTab(acquisitTab, tr("Image Acquisit"));
    ui->tabWidget->addTab(studyExplorerTab, tr("Explore Study"));
    ui->tabWidget->addTab(imageViewTab, tr("Image View"));
    ui->tabWidget->addTab(reportEditTab, tr("Report Edit"));
    ui->tabWidget->addTab(archiveTab, tr("Archive Utilities"));
    ui->tabWidget->addTab(settingsTab, tr("System Settings"));
    ui->tabWidget->addTab(exitTab, tr("Exit"));
    ui->tabWidget->setCurrentWidget(studyExplorerTab);
    prevTab = studyExplorerTab;
}

const InstitutionInfo& MainWindow::getInstInfo() const
{
    return settingsTab->getInstInfo();
}

const ManufactureInfo& MainWindow::getManuInfo() const
{
    return settingsTab->getManuInfo();
}

const StationInfo& MainWindow::getStationInfo() const
{
    return settingsTab->getStationInfo();
}

const CommunicationInfo& MainWindow::getCommInfo() const
{
    return settingsTab->getCommInfo();
}

const WorkFlow& MainWindow::getWorkFlow() const
{
    return settingsTab->getWorkFlow();
}

QString MainWindow::getDbLocation() const
{
    return settingsTab->getDbLocation();
}

const CustomizedId& MainWindow::getPatientIdFormat() const
{
    return settingsTab->getPatientIdFormat();
}

const CustomizedId& MainWindow::getAccNumFormat() const
{
    return settingsTab->getAccNumFormat();
}

ProcedureItemModel *MainWindow::getProcModel() const
{
    return settingsTab->getProcModel();
}

void MainWindow::closeEvent(QCloseEvent *e)
{

    if (QMessageBox::Yes == QMessageBox::question(this, tr("DR Image Station"),
                                                  tr("Are you sure to exit?"),
                                                  QMessageBox::Yes|QMessageBox::No)) {

        if (!reportEditTab->close()) e->ignore();
    } else e->ignore();
}
