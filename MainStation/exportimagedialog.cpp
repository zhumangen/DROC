#include "exportimagedialog.h"
#include "ui_exportimagedialog.h"
#include "exportimagethread.h"

#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>

#define EXPORT_IMAGE_FOLDER "EXPORTIMAGEFOLDER"

ExportImageDialog::ExportImageDialog(QWidget *parent) :
    exportThread(new ExportImageThread(this)),
    imageCount(0),
    QDialog(parent),
    ui(new Ui::ExportImageDialog)
{
    ui->setupUi(this);
    init();
}

ExportImageDialog::~ExportImageDialog()
{
    delete ui;
}

void ExportImageDialog::closeEvent(QCloseEvent *e)
{
    if (exportThread->isRunning()) e->ignore();
}

void ExportImageDialog::onSelectFolder()
{
    QSettings settings;
    QString folder = settings.value(EXPORT_IMAGE_FOLDER, ".").toString();
    folder = QFileDialog::getExistingDirectory(this, tr("Select Destination Folder"), folder);
    if (!folder.isEmpty()) {
        settings.setValue(EXPORT_IMAGE_FOLDER, folder);
        ui->exportDestEdit->setText(folder);
    }
}

void ExportImageDialog::onStart(bool checked)
{
    if (checked) {
        if (ui->exportDestEdit->text().isEmpty()) {
            onSelectFolder();
            return;
        }

        if (ui->bmpRadio->isChecked()) {
            if (ui->bmp8BitRadio->isChecked()) {
                exportThread->setFileType(ExportImageThread::EFT_8bitBMP);
            } else if (ui->bmp24BitRadio->isChecked()) {
                exportThread->setFileType(ExportImageThread::EFT_24bitBMP);
            } else if (ui->bmp32BitRadio->isChecked()) {
                exportThread->setFileType(ExportImageThread::EFT_32bitBMP);
            } else {
                exportThread->setFileType(ExportImageThread::EFT_BMP);
            }
        } else if (ui->dicomRadio->isChecked()) {
            exportThread->setFileType(ExportImageThread::EFT_Dicom);
        } else if (ui->jpegRadio->isChecked()) {
            exportThread->setFileType(ExportImageThread::EFT_JPEG);
            exportThread->setJpegQuality(ui->jpegQualitySlider->value());
        } else if (ui->tiffRadio->isChecked()) {
            exportThread->setFileType(ExportImageThread::EFT_TIFF);
        } else if (ui->pnmRadio->isChecked()) {
            if (ui->pnm8BitRadio->isChecked()) {
                exportThread->setFileType(ExportImageThread::EFT_8bitPNM);
            } else if (ui->pnm16BitRadio->isChecked()) {
                exportThread->setFileType(ExportImageThread::EFT_16bitPNM);
            } else if (ui->pnmPastelRadio->isChecked()) {
                exportThread->setFileType(ExportImageThread::EFT_PastelPNM);
            } else {
                exportThread->setFileType(ExportImageThread::EFT_RawPNM);
            }
        } else if (ui->pngRadio->isChecked()) {
            exportThread->setFileType(ExportImageThread::EFT_PNG);
        } else return;

        ui->selectFolderButton->setDisabled(true);
        ui->closeButton->setDisabled(true);
        ui->startButton->setText(tr("Abort"));
        ui->progressBar->setMaximum(imageCount);
        ui->progressBar->setValue(0);
        ui->textBrowser->append(tr("Start exporting..."));
        exportThread->setAbort(false);
        exportThread->setDestDir(ui->exportDestEdit->text());
        exportThread->start();
    } else {
        exportThread->setAbort(true);
    }
}

void ExportImageDialog::onExportFinished()
{
    ui->selectFolderButton->setDisabled(false);
    ui->closeButton->setDisabled(false);
    ui->startButton->setText(tr("Start"));
    ui->startButton->setChecked(false);
}

void ExportImageDialog::onResultReady(const QString &msg)
{
    ui->progressBar->setValue(ui->progressBar->value()+1);
    ui->textBrowser->append(msg);
}

void ExportImageDialog::setImageFiles(const QStringList &images)
{
    exportThread->setImageFiles(images);
    imageCount = images.size();
    ui->textBrowser->append(tr("%1 images to export.").arg(imageCount));
}

void ExportImageDialog::init()
{
    ui->bmpGroup->setVisible(false);
    ui->jpegGroup->setVisible(false);
    ui->pnmGroup->setVisible(false);

    connect(ui->selectFolderButton, SIGNAL(clicked()), this, SLOT(onSelectFolder()));
    connect(ui->startButton, SIGNAL(clicked(bool)), this, SLOT(onStart(bool)));
    connect(ui->bmpRadio, SIGNAL(toggled(bool)), ui->bmpGroup, SLOT(setVisible(bool)));
    connect(ui->pnmRadio, SIGNAL(toggled(bool)), ui->pnmGroup, SLOT(setVisible(bool)));
    connect(ui->jpegRadio, SIGNAL(toggled(bool)), ui->jpegGroup, SLOT(setVisible(bool)));
    connect(exportThread, SIGNAL(finished()), this, SLOT(onExportFinished()));
    connect(exportThread, SIGNAL(resultReady(QString)), this, SLOT(onResultReady(QString)));

    ui->exportDestEdit->setText(QSettings().value(EXPORT_IMAGE_FOLDER).toString());
}
