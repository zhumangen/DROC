#include "reporteditwidget.h"
#include "ui_reporteditwidget.h"

#include "reportwidget.h"
#include "diagnostictemplatemodel.h"
#include "diagnostictemplateview.h"
#include "diagnostictemplatedelegate.h"
#include "srsettingsdialog.h"
#include "mdisubwindow.h"
#include "../MainStation/mainwindow.h"
#include "../DicomViewer/thumbnailbarwidget.h"
#include "../DicomViewer/dicomimagelabel.h"
#include "../MainStation/sqlimagemodel.h"
#include "../share/global.h"

#include <QComboBox>
#include <QScrollArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QCloseEvent>

#define EXTERNAL_REPORT_FILE "EXTERNALREPORTFILE"

ReportEditWidget::ReportEditWidget(QWidget *parent) :
    currentWindow(0),
    templateModel(new DiagnosticTemplateModel(this)),
    templateView(new DiagnosticTemplateView(this)),
    thumbnailWidget(new ThumbnailBarWidget),
    QWidget(parent),
    ui(new Ui::ReportEditWidget)
{
    ui->setupUi(this);
    init();
}

ReportEditWidget::~ReportEditWidget()
{
    delete ui;
}

void ReportEditWidget::init()
{
    ui->thumbnailScrollArea->setWidget(thumbnailWidget);
    ui->bottomHLayout->insertWidget(2, templateView);
    templateView->setMinimumWidth(160);
    templateView->setModel(templateModel);
    templateView->setItemDelegate(new DiagnosticTemplateDelegate(this));

    setupConnections();
    setPermissions();
}

void ReportEditWidget::setupConnections()
{
    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(onSubWindowActivated(QMdiSubWindow*)));
    connect(templateView, SIGNAL(doubleClicked(QModelIndex)),
            templateModel, SLOT(onDoubleClicked(QModelIndex)));
    connect(templateView, SIGNAL(newItemAfter(QModelIndex)),
            templateModel, SLOT(onNewItemAfter(QModelIndex)));
    connect(templateView, SIGNAL(newItemBefore(QModelIndex)),
            templateModel, SLOT(onNewItemBefore(QModelIndex)));
    connect(templateView, SIGNAL(newSubItem(QModelIndex)),
            templateModel, SLOT(onNewSubItem(QModelIndex)));
    connect(templateModel, SIGNAL(editItemTitle(QModelIndex)),
            templateView, SLOT(edit(QModelIndex)));
    connect(templateView, SIGNAL(editItem(QModelIndex)),
            templateModel, SLOT(onEditItem(QModelIndex)));
    connect(templateModel, SIGNAL(editItem(QModelIndex,QString,QString,QString)),
            templateView, SLOT(onEditItem(QModelIndex,QString,QString,QString)));
    connect(templateView, SIGNAL(applyEdit(QModelIndex,QString,QString,QString)),
            templateModel, SLOT(onApplyEdit(QModelIndex,QString,QString,QString)));
    connect(templateView, SIGNAL(removeItem(QModelIndex)),
            templateModel, SLOT(onRemoveItem(QModelIndex)));
    connect(templateModel, SIGNAL(itemSelected(QString,QString)),
            this, SLOT(onTemplateItemSelected(QString,QString)));
    connect(thumbnailWidget, SIGNAL(imageDoubleClicked(DicomImageLabel*)),
            this, SLOT(onImageDoubleClicked(DicomImageLabel*)));

    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(onOpenReport()));
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveReport()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onSettings()));
    connect(ui->insertImageButton, SIGNAL(clicked()), this, SLOT(onInsertImage()));
    connect(ui->removeImageButton, SIGNAL(clicked()), this, SLOT(onRemoveImage()));
    connect(ui->completeButton, SIGNAL(clicked()), this, SLOT(onCompleteReport()));
    connect(ui->verigyButton, SIGNAL(clicked()), this, SLOT(onVerifyReport()));
    connect(ui->printButton, SIGNAL(clicked()), this, SLOT(onPrint()));
    connect(ui->printPreviewButton, SIGNAL(clicked()), this, SLOT(onPrintPreview()));
    connect(ui->paperSizeCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(onPaperChanged(QString)));
}

void ReportEditWidget::setPermissions()
{
    GroupPermissions perms = mainWindow->getCurrentGroup().permissions;
    setReadOnly(!(perms & GP_ModifyReport));
    ui->verigyButton->setEnabled(perms & GP_VerifyReport);
    ui->printButton->setEnabled(perms & GP_PrintReport);
}

void ReportEditWidget::setReadOnly(bool yes)
{
    ui->insertImageButton->setDisabled(yes);
    ui->removeImageButton->setDisabled(yes);
    ui->completeButton->setDisabled(yes);
    ui->verigyButton->setDisabled(yes);
    ui->settingsButton->setDisabled(yes);
}

void ReportEditWidget::insertReportImages(ReportWidget *report)
{
    QList<ImageInstance*> images = thumbnailWidget->getImageList();
    foreach (ImageInstance *image, images) {
        report->loadEvidenceImage(image);
    }
}

void ReportEditWidget::openReports(const QStringList &files)
{
    foreach (QString file, files) {
        openReport(file);
    }
}

void ReportEditWidget::openReport(const QString &file)
{
    QMdiSubWindow *subwin = searchReport(file);
    if (subwin) {
        ui->mdiArea->setActiveSubWindow(subwin);
    } else {
        ReportWidget *report = new ReportWidget;
        report->openReport(file);
        if (report->isReportNormal()) {
            connect(thumbnailWidget, SIGNAL(imageInserted(ImageInstance*)), report, SLOT(loadEvidenceImage(ImageInstance*)));
            connect(report, SIGNAL(reportClosed(QString)), thumbnailWidget, SLOT(removeStduy(QString)));
            insertReportImages(report);
            QScrollArea *scroll = new QScrollArea;
            scroll->setWidget(report);
            scroll->setAlignment(Qt::AlignCenter);
            MdiSubWindow *window = new MdiSubWindow;
            window->setWidget(scroll);
            window->setAttribute(Qt::WA_DeleteOnClose);
            ui->mdiArea->addSubWindow(window);
            window->setWindowTitle(report->getReportTitle());
            window->showMaximized();
            currentWindow = window;
            thumbnailWidget->addStudy(report->getStudyUid());
            thumbnailWidget->setCurrentStudyUid(report->getStudyUid());
        } else {
            delete report;
        }
    }
}

void ReportEditWidget::createReport(const QString &studyUid)
{
    ReportWidget *report = new ReportWidget;
    report->createReport(studyUid);
    if (report->isReportNormal()) {
        connect(report, SIGNAL(reportCreated()), this, SIGNAL(reportCreated()));
        connect(report, SIGNAL(reportClosed(QString)), thumbnailWidget, SLOT(removeStduy(QString)));
        insertReportImages(report);
        QScrollArea *scroll = new QScrollArea;
        scroll->setWidget(report);
        scroll->setAlignment(Qt::AlignCenter);
        MdiSubWindow *window = new MdiSubWindow;
        window->setWidget(scroll);
        window->setAttribute(Qt::WA_DeleteOnClose);
        ui->mdiArea->addSubWindow(window);
        window->setWindowTitle(report->getReportTitle());
        window->showMaximized();
        currentWindow = window;
        thumbnailWidget->addStudy(report->getStudyUid());
        thumbnailWidget->setCurrentStudyUid(report->getStudyUid());
    } else {
        delete report;
    }
}

void ReportEditWidget::onSettings()
{
    SRSettingsDialog dialog(this);
    dialog.exec();

    QList<QMdiSubWindow*> winList = ui->mdiArea->subWindowList();
    foreach (QMdiSubWindow *win, winList) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(win->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->reflushReportFormat();
    }
}

QMdiSubWindow *ReportEditWidget::searchReport(const QString &file)
{
    QMdiSubWindow *subWin = 0;
    QList<QMdiSubWindow*> winList = ui->mdiArea->subWindowList();
    foreach (QMdiSubWindow *win, winList) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(win->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        if (report->getReportFile() == file) {
            subWin = win;
            break;
        }
    }
    return subWin;
}

bool ReportEditWidget::setActiveReport(const QString &studyUid)
{
    QMdiSubWindow *subWin = 0;
    QList<QMdiSubWindow*> winList = ui->mdiArea->subWindowList();
    foreach (QMdiSubWindow *win, winList) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(win->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        if (report->getStudyUid() == studyUid) {
            subWin = win;
            break;
        }
    }
    if (subWin) {
        ui->mdiArea->setActiveSubWindow(subWin);
        return true;
    } else return false;
}

void ReportEditWidget::closeAllReports()
{
    ui->mdiArea->closeAllSubWindows();
    currentWindow = ui->mdiArea->currentSubWindow();
}

void ReportEditWidget::onOpenReport()
{
    QSettings settings;
    QStringList reportFiles = QFileDialog::getOpenFileNames(this, tr("Open Structured Report"),
                                                            settings.value(EXTERNAL_REPORT_FILE, ".").toString(),
                                                            tr("SR File (*.dcm);;All Files (*.*)"));
    if (reportFiles.size()) {
        ReportWidget *report = 0;
        QMdiSubWindow *subwin = 0;
        foreach (QString reportFile, reportFiles) {
            if (!(subwin = searchReport(reportFile))) {
                report = new ReportWidget;
                report->openReport(reportFile, true);
                if (report->isReportNormal()) {
                    connect(report, SIGNAL(reportClosed(QString)), thumbnailWidget, SLOT(removeStduy(QString)));
                    insertReportImages(report);
                    QScrollArea *scroll = new QScrollArea;
                    scroll->setWidget(report);
                    scroll->setAlignment(Qt::AlignCenter);
                    MdiSubWindow *window = new MdiSubWindow;
                    window->setWidget(scroll);
                    window->setAttribute(Qt::WA_DeleteOnClose);
                    ui->mdiArea->addSubWindow(window);
                    window->setWindowTitle(report->getReportTitle());
                    window->showMaximized();
                    currentWindow = window;
                    thumbnailWidget->addStudy(report->getStudyUid());
                    thumbnailWidget->setCurrentStudyUid(report->getStudyUid());
                } else {
                    delete report;
                    report = 0;
                }
            }
        }

        if (report) thumbnailWidget->setCurrentStudyUid(report->getStudyUid());
        if (subwin) ui->mdiArea->setActiveSubWindow(subwin);

        settings.setValue(EXTERNAL_REPORT_FILE, reportFiles.first());
    }
}

void ReportEditWidget::onSaveReport()
{
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->saveReport();
    }
}

void ReportEditWidget::onCompleteReport()
{
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->completeReport();
        currentWindow->setWindowTitle(report->getReportTitle());
    }
}

void ReportEditWidget::onVerifyReport()
{
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->verifyReport();
        currentWindow->setWindowTitle(report->getReportTitle());
        setReadOnly(report->isReportVerified());
    }
}

void ReportEditWidget::onInsertImage()
{
    if (currentWindow && thumbnailWidget->getCurrentImageLabel()) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->insertImage(thumbnailWidget->getCurrentImageLabel()->getImageInstance());
    }
}

void ReportEditWidget::onRemoveImage()
{
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->removeImage();
    }
}

void ReportEditWidget::onPrint()
{
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->print();
    }
}

void ReportEditWidget::onPrintPreview()
{
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->printPreview();
    }
}

void ReportEditWidget::onPaperChanged(const QString &paper)
{
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->setReportPaper(paper);
    }
}

void ReportEditWidget::onTemplateItemSelected(const QString &findings, const QString &conclusion)
{
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        report->appendTemplateContents(findings, conclusion);
    }
}

void ReportEditWidget::onSubWindowActivated(QMdiSubWindow *win)
{
    currentWindow = win;
    if (currentWindow) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        QString paper = report->getReportPaper();
        if (!paper.isEmpty()) ui->paperSizeCombo->setCurrentText(paper);
        if (mainWindow->getCurrentGroup().permissions & GP_ModifyReport)
            setReadOnly(report->isReportVerified());
        thumbnailWidget->setCurrentStudyUid(report->getStudyUid());
    }
}

void ReportEditWidget::onImageDoubleClicked(DicomImageLabel *label)
{
    if (currentWindow && label) {
        QScrollArea *scroll = qobject_cast<QScrollArea*>(currentWindow->widget());
        ReportWidget *report = qobject_cast<ReportWidget*>(scroll->widget());
        if ((!report->isReportVerified()) &&
                (mainWindow->getCurrentGroup().permissions & GP_ModifyReport))
            report->insertImage(label->getImageInstance());
    }
}

void ReportEditWidget::closeEvent(QCloseEvent *e)
{
    QList<QMdiSubWindow*> subwins = ui->mdiArea->subWindowList();
    foreach (QMdiSubWindow *subwin, subwins) {
        if (!subwin->close())
            e->ignore();
    }
}
