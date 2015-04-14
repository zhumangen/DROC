#include "srsettingsdialog.h"
#include "ui_srsettingsdialog.h"

#include <QFontDialog>
#include <QSettings>

SRSettingsDialog::SRSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SRSettingsDialog)
{
    ui->setupUi(this);

    connect(ui->institutionNameFontButton, SIGNAL(clicked()),
            this, SLOT(setInstitutionNameFont()));
    connect(ui->reportTitleFontButton, SIGNAL(clicked()),
            this, SLOT(setReportTitleFont()));
    connect(ui->patientInfoFontButton, SIGNAL(clicked()),
            this, SLOT(setPatientInfoFont()));
    connect(ui->reportTextFontButton, SIGNAL(clicked()),
            this, SLOT(setReportTextFont()));
    connect(ui->physicianInfoFontButton, SIGNAL(clicked()),
            this, SLOT(setPhysicianInfoFont()));
    connect(ui->footNoteFontButton, SIGNAL(clicked()),
            this, SLOT(setFootNoteFont()));
    connect(ui->okButton, SIGNAL(clicked()),
            this, SLOT(onOk()));

    init();
}

SRSettingsDialog::~SRSettingsDialog()
{
    delete ui;
}

void SRSettingsDialog::init()
{
    QSettings settings;

    QFont font = this->font();
    font.fromString(settings.value(INSTITUTION_NAME_FONT).toString());
    ui->institutionNameEdit->setFont(font);
    font.fromString(settings.value(REPORT_TITLE_FONT).toString());
    ui->reportTitleEdit->setFont(font);
    ui->reportTitleEdit->setText(settings.value(REPORT_TITLE_TEXT).toString());
    font.fromString(settings.value(PATIENT_INFO_FONT).toString());
    ui->patientInfoEdit->setFont(font);
    font.fromString(settings.value(REPORT_TEXT_FONT).toString());
    ui->reportTextEdit->setFont(font);
    font.fromString(settings.value(PHYSICIAN_INFO_FONT).toString());
    ui->physicianInfoEdit->setFont(font);
    font.fromString(settings.value(FOOTNOTE_FONT).toString());
    ui->footNoteEdit->setFont(font);
    ui->footNoteEdit->setText(settings.value(FOOTNOTE_TEXT).toString());
    ui->imageWidthSpin->setValue(settings.value(IMAGE_WIDTH).toDouble());
    ui->imageHeightSpin->setValue(settings.value(IMAGE_HEIGHT).toDouble());

}

void SRSettingsDialog::setInstitutionNameFont()
{
    QSettings settings;
    bool ok;
    QFont font;
    font.fromString(settings.value(INSTITUTION_NAME_FONT).toString());
    font = QFontDialog::getFont(&ok, font, this);
    if (ok) ui->institutionNameEdit->setFont(font);
}

void SRSettingsDialog::setReportTitleFont()
{
    QSettings settings;
    bool ok;
    QFont font;
    font.fromString(settings.value(REPORT_TITLE_FONT).toString());
    font = QFontDialog::getFont(&ok, font, this);
    if (ok) ui->reportTitleEdit->setFont(font);
}

void SRSettingsDialog::setPatientInfoFont()
{
    QSettings settings;
    bool ok;
    QFont font;
    font.fromString(settings.value(PATIENT_INFO_FONT).toString());
    font = QFontDialog::getFont(&ok, font, this);
    if (ok) ui->patientInfoEdit->setFont(font);
}

void SRSettingsDialog::setReportTextFont()
{
    QSettings settings;
    bool ok;
    QFont font;
    font.fromString(settings.value(REPORT_TEXT_FONT).toString());
    font = QFontDialog::getFont(&ok, font, this);
    if (ok) ui->reportTextEdit->setFont(font);
}

void SRSettingsDialog::setPhysicianInfoFont()
{
    QSettings settings;
    bool ok;
    QFont font;
    font.fromString(settings.value(PHYSICIAN_INFO_FONT).toString());
    font = QFontDialog::getFont(&ok, font, this);
    if (ok) ui->physicianInfoEdit->setFont(font);
}

void SRSettingsDialog::setFootNoteFont()
{
    QSettings settings;
    bool ok;
    QFont font;
    font.fromString(settings.value(FOOTNOTE_FONT).toString());
    font = QFontDialog::getFont(&ok, font, this);
    if (ok) ui->footNoteEdit->setFont(font);
}

void SRSettingsDialog::onOk()
{
    QSettings settings;
    settings.setValue(INSTITUTION_NAME_FONT, ui->institutionNameEdit->font().toString());
    settings.setValue(REPORT_TITLE_FONT, ui->reportTitleEdit->font().toString());
    settings.setValue(REPORT_TITLE_TEXT, ui->reportTitleEdit->text());
    settings.setValue(PATIENT_INFO_FONT, ui->patientInfoEdit->font().toString());
    settings.setValue(REPORT_TEXT_FONT, ui->reportTextEdit->font().toString());
    settings.setValue(PHYSICIAN_INFO_FONT, ui->physicianInfoEdit->font().toString());
    settings.setValue(FOOTNOTE_FONT, ui->footNoteEdit->font().toString());
    settings.setValue(FOOTNOTE_TEXT, ui->footNoteEdit->text());
    settings.setValue(IMAGE_WIDTH, ui->imageWidthSpin->value());
    settings.setValue(IMAGE_HEIGHT, ui->imageHeightSpin->value());

    accept();
}
