#include "printparamdialog.h"
#include "ui_printparamdialog.h"
#include "../share/dicomscp.h"
#include "../share/configfiles.h"

#include <QFile>
#include <QTextStream>

PrintParamDialog::PrintParamDialog(PrintParam &params, QWidget *parent) :
    param(params),
    QDialog(parent),
    ui(new Ui::PrintParamDialog)
{
    ui->setupUi(this);

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOk()));

    init();
}

PrintParamDialog::~PrintParamDialog()
{
    delete ui;
}

void PrintParamDialog::init()
{
    loadPrintConfig();

    ui->sessionLabelCombo->setCurrentText(param.sessionLabel);
    ui->ownerIdCombo->setCurrentText(param.ownerId);
    ui->mediumTypeCombo->setCurrentText(param.mediumType);
    ui->destinationCombo->setCurrentText(param.destination);
    ui->priorityCombo->setCurrentText(param.priority);

    ui->filmLayoutCombo->setCurrentText(param.displayFormat);
    ui->filmOrientCombo->setCurrentText(param.filmOrientation);
    ui->filmSizeCombo->setCurrentText(param.filmSizeId);
    ui->resolutionIdCombo->setCurrentText(param.priority);
    ui->magnificationTypeCombo->setCurrentText(param.magnificationType);
    ui->smoothingTypeCombo->setCurrentText(param.smoothingType);

    ui->borderDensityCombo->setCurrentText(param.borderDensity);
    ui->emptyImageCombo->setCurrentText(param.emptyImageDensity);
    ui->maxDensityCombo->setCurrentText(param.maxDensity);
    ui->minDesityCombo->setCurrentText(param.minDensity);

    ui->disableNewVRsCheck->setChecked(param.disableNewVRs);
    ui->implicitOnlyCheck->setChecked(param.implicitOnly);
    ui->supports12BitsCheck->setChecked(param.supports12Bit);
    ui->decimateCropCheck->setChecked(param.supportsDecimateCrop);
    ui->presentationLutCheck->setChecked(param.supportsPresentationLut);
    ui->imageSizeCheck->setChecked(param.supportsImageSize);
    ui->trimCheck->setChecked(param.supportsTrim);
}

void PrintParamDialog::loadPrintConfig()
{
    QFile file(PRINTERS_CFG);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith('#')) continue;
            int index = line.indexOf('=');
            if (index <= 0) continue;
            QString tag = line.left(index).remove(' ').toUpper();
            QString value = line.mid(index+1);
            while (value.startsWith(' ')) value.remove(0, 1);
            while (value.endsWith(' ')) value.remove(value.length()-1, 1);
            QStringList ids = value.split('\\', QString::SkipEmptyParts);
            if (tag == "DISPLAYFORMAT") {
                value.replace(',', " X ");
                ui->filmLayoutCombo->addItems(value.split('\\', QString::SkipEmptyParts));
            } else if (tag == "FILMSIZEID") {
                ui->filmSizeCombo->addItems(ids);
            } else if (tag == "BORDERDENSITY") {
                ui->borderDensityCombo->addItems(ids);
            } else if (tag == "EMPTYIMAGEDENSITY") {
                ui->emptyImageCombo->addItems(ids);
            } else if (tag == "MAXDENSITY") {
                ui->maxDensityCombo->addItems(ids);
            } else if (tag == "MINDENSITY") {
                ui->minDesityCombo->addItems(ids);
            } else if (tag == "FILMDESTINATION") {
                ui->destinationCombo->addItems(ids);
            } else if (tag == "MEDIUMTYPE") {
                ui->mediumTypeCombo->addItems(ids);
            } else if (tag == "MAGNIFICATIONTYPE") {
                ui->magnificationTypeCombo->addItems(ids);
            } else if (tag == "SMOOTHINGTYPE") {
                ui->smoothingTypeCombo->addItems(ids);
            } else if (tag == "RESOLUTIONID") {
                ui->resolutionIdCombo->addItems(ids);
            }
        }
    }
}

void PrintParamDialog::onOk()
{
    param.sessionLabel = ui->sessionLabelCombo->currentText();
    param.ownerId = ui->ownerIdCombo->currentText();
    param.mediumType = ui->mediumTypeCombo->currentText();
    param.destination = ui->destinationCombo->currentText();
    param.priority = ui->priorityCombo->currentText();

    param.displayFormat = ui->filmLayoutCombo->currentText();
    param.filmOrientation = ui->filmOrientCombo->currentText();
    param.filmSizeId = ui->filmSizeCombo->currentText();
    param.resolutionId = ui->resolutionIdCombo->currentText();
    param.magnificationType = ui->magnificationTypeCombo->currentText();
    param.smoothingType = ui->smoothingTypeCombo->currentText();

    param.borderDensity = ui->borderDensityCombo->currentText();
    param.emptyImageDensity = ui->emptyImageCombo->currentText();
    param.maxDensity = ui->maxDensityCombo->currentText();
    param.minDensity = ui->minDesityCombo->currentText();

    param.disableNewVRs = ui->disableNewVRsCheck->isChecked();
    param.implicitOnly = ui->implicitOnlyCheck->isChecked();
    param.supports12Bit = ui->supports12BitsCheck->isChecked();
    param.supportsDecimateCrop = ui->decimateCropCheck->isChecked();
    param.supportsPresentationLut = ui->presentationLutCheck->isChecked();
    param.supportsImageSize = ui->imageSizeCheck->isChecked();
    param.supportsTrim = ui->trimCheck->isChecked();

    accept();
}
