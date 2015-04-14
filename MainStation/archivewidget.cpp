#include "archivewidget.h"
#include "ui_archivewidget.h"
#include "../share/global.h"
#include "../share/studyrecord.h"
#include "scandcmfilethread.h"
#include "importstudymodel.h"
#include "importstudyview.h"
#include "importdcmfilethread.h"
#include "logdbmanager.h"
#include "mainwindow.h"

#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QCloseEvent>

ArchiveWidget::ArchiveWidget(QWidget *parent) :
    importModel(new ImportStudyModel(this)),
    importView(new ImportStudyView(this)),
    scanThread(new ScanDcmFileThread(this)),
    QWidget(parent),
    ui(new Ui::ArchiveWidget)
{
    ui->setupUi(this);
    init();
}

ArchiveWidget::~ArchiveWidget()
{
    delete ui;
}

void ArchiveWidget::init()
{
    if (!LogDbManager::createLogDb()) {
        QMessageBox::critical(this, tr("Create Log Database"),
                              tr("Create log database failed: %1.")
                              .arg(LogDbManager::lastError.text()));
    }

    ui->fromDateTimeEdit->setDate(QDate::currentDate());
    ui->fromDateTimeEdit->setTime(QTime(0, 0));
    ui->toDateTimeEdit->setDate(QDate::currentDate());
    ui->toDateTimeEdit->setTime(QTime(23, 59, 59, 999));

    importThread = new ImportDcmFileThread(importModel, this);
    logModel = new QSqlTableModel(this, QSqlDatabase::database(LOG_DB_CONNECTION_NAME));
    logModel->setTable("LogTable");
    onLogSearch();
    logModel->select();
    ui->logTableView->setModel(logModel);
    ui->logTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    importView->setModel(importModel);
    ui->importTabVLayout->insertWidget(0, importView);

    createConnections();
    setPermissions();
}

void ArchiveWidget::setPermissions()
{
    bool canImport = mainWindow->getCurrentGroup().permissions & GP_ImportStudy;
    ui->fileButton->setEnabled(canImport);
    ui->folderButton->setEnabled(canImport);
    ui->importButton->setEnabled(canImport);
}

void ArchiveWidget::createConnections()
{
    connect(importThread, SIGNAL(finished()), this, SLOT(onImportThreadFinished()));
    connect(scanThread, SIGNAL(finished()), this, SLOT(onScanThreadFinished()));
    connect(importThread, SIGNAL(resultReady()), this, SLOT(onResultReady()));
    connect(scanThread, SIGNAL(resultReady(StudyRecord*)), this, SLOT(onResultReady()));
    connect(scanThread, SIGNAL(resultReady(StudyRecord*)), importModel, SLOT(appendStudy(StudyRecord*)));

    connect(ui->importButton, SIGNAL(clicked(bool)), this, SLOT(onScanImport(bool)));
    connect(ui->fileButton, SIGNAL(clicked()), this, SLOT(onFileClicked()));
    connect(ui->folderButton, SIGNAL(clicked()), this, SLOT(onFolderClicked()));
    connect(ui->removeButton, SIGNAL(clicked()), importView, SLOT(removeSelectedRows()));
    connect(ui->clearStudyButton, SIGNAL(clicked()), importView, SLOT(removeAllRows()));

    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(onLogSearch()));
}

void ArchiveWidget::onScanThreadFinished()
{
    if (ui->importAfterScanCheck->isChecked()) {
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(importModel->getFileCount());
        importThread->setAbort(false);
        importThread->start();
    } else {
        setImportButtonsDisabled(false);
        ui->importButton->setChecked(false);
        ui->importButton->setText(tr("Import"));
    }
}

void ArchiveWidget::onImportThreadFinished()
{
    setImportButtonsDisabled(false);
    ui->importButton->setChecked(false);
    ui->importButton->setText(tr("Import"));
    emit importFinished();
}

void ArchiveWidget::onResultReady()
{
    ui->progressBar->setValue(ui->progressBar->value()+1);
}

void ArchiveWidget::setImportButtonsDisabled(bool yes)
{
    ui->fileButton->setDisabled(yes);
    ui->folderButton->setDisabled(yes);
    ui->removeButton->setDisabled(yes);
    ui->clearStudyButton->setDisabled(yes);
}

void ArchiveWidget::scanFiles(const QStringList &files)
{
    ui->progressBar->setMaximum(files.size());
    scanThread->setAbort(false);
    scanThread->setFiles(files);
    scanThread->start();

    setImportButtonsDisabled(true);
    ui->importButton->setChecked(true);
    ui->importButton->setText(tr("Abort"));
}

static void scanDirHelpper(QStringList &files, const QString &dir)
{
    QDir qdir(dir);

    QFileInfoList infoList = qdir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    foreach (QFileInfo info, infoList) {
        if (info.isFile())
            files.append(info.filePath());
        else if (info.isDir())
            scanDirHelpper(files, info.filePath());
    }
    /*
    QStringList fileList = qdir.entryList(QDir::Files);
    files << fileList;

    QStringList dirList = qdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString subDir, dirList) {
        scanDirHelpper(files, subDir);
    }
    */
}

void ArchiveWidget::onFileClicked()
{
    QSettings s;
    QString file = s.value(STUDY_IMPORT_FILE).toString();
    if (file.isEmpty()) file = ".";
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Import Study"),
                                                      file, tr("DICOM Files(*.dcm);;All Files(*)"));
    if (files.size()) {
        s.setValue(STUDY_IMPORT_FILE, files.first());
        scanFiles(files);
    }
}

void ArchiveWidget::onFolderClicked()
{
    QSettings s;
    QString dir = s.value(STUDY_IMPORT_FOLDER).toString();
    if (dir.isEmpty()) dir = ".";
    dir = QFileDialog::getExistingDirectory(this, tr("Import Study"), dir);
    if (!dir.isEmpty()) {
        s.setValue(STUDY_IMPORT_FOLDER, dir);
        QStringList files;
        scanDirHelpper(files, dir);
        scanFiles(files);
    }
}

void ArchiveWidget::onScanImport(bool checked)
{
    if (checked) { // Start import
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(importModel->getFileCount());
        importThread->setAbort(false);
        importThread->start();
        ui->importButton->setChecked(true);
        ui->importButton->setText("Abort");
        setImportButtonsDisabled(true);
    } else {
        if (scanThread->isRunning()) {  // Abort Scanning
            scanThread->setAbort(true);
        } else { // Abort Importing
            importThread->setAbort(true);
        }
    }
}

void ArchiveWidget::onLogSearch()
{
    QString filter;
    if (ui->fromCheckBox->isChecked()) {
        filter = QString("LogTime>\'%1\'").arg(ui->fromDateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    if (ui->toCheckBox->isChecked()) {
        if (!filter.isEmpty()) filter.append(" and ");
        filter.append(QString("LogTime<\'%1\'").arg(ui->toDateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss")));
    }
    if (!ui->userNameEdit->text().isEmpty()) {
        if (!filter.isEmpty()) filter.append(" and ");
        filter.append(QString("UserName LIKE \'%%1%\'").arg(ui->userNameEdit->text()));
    }
    logModel->setFilter(filter);
}

void ArchiveWidget::onLogExportClicked()
{

}

void ArchiveWidget::closeEvent(QCloseEvent *e)
{
    if (scanThread->isRunning() || importThread->isRunning()) {
        if (QMessageBox::Ok == QMessageBox::warning(this, tr("Abort Importing"),
                                                    tr("Are you sure to abort importing?"),
                                                    QMessageBox::Ok|QMessageBox::Cancel)) {
            scanThread->setAbort(true);
            importThread->setAbort(true);
            scanThread->wait(500);
            importThread->wait(500);
            e->accept();
        } else e->ignore();
    } else e->ignore();
}
