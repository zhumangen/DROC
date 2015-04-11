#include "newstudydialog.h"
#include "ui_newstudydialog.h"
#include "../share/global.h"
#include "../MainStation/mainwindow.h"

#include "dcmtk/dcmdata/dcuid.h"

#include <QSettings>
#include <QMessageBox>

NewStudyDialog::NewStudyDialog(QWidget *parent) :
    modifyMode(false),
    QDialog(parent),
    ui(new Ui::NewStudyDialog)
{
    ui->setupUi(this);
    init();
}

NewStudyDialog::NewStudyDialog(const StudyRecord &studyRec, QWidget *parent) :
    modifyMode(true),
    study(studyRec),
    QDialog(parent),
    ui(new Ui::NewStudyDialog)
{
    ui->setupUi(this);
    init();
}

NewStudyDialog::~NewStudyDialog()
{
    delete ui;
}

void NewStudyDialog::init()
{
    QSettings s;
    const CustomizedId &pidf = mainWindow->getPatientIdFormat();
    int start = s.value(PATIENTID_START).toInt();
    ui->patientIdEdit->setText(QString("%1%2%3").arg(pidf.prefix)
                               .arg(start, pidf.digits, 10, QChar('0'))
                               .arg(pidf.suffix));
    const CustomizedId &aidf = mainWindow->getAccNumFormat();
    start = s.value(ACCNUMBER_START).toInt();
    ui->accNumEdit->setText(QString("%1%2%3").arg(aidf.prefix)
                            .arg(start, aidf.digits, 10, QChar('0'))
                            .arg(aidf.suffix));

    QStringList phys = s.value(REQ_PHYSICIANS).toStringList();
    ui->reqPhysicianCombo->addItems(phys);
    phys = s.value(PER_PHYSICIANS).toStringList();
    ui->perPhysicianCombo->addItems(phys);

    ui->reqPhysicianCombo->setCurrentText(mainWindow->getCurrentUser().name);
    ui->perPhysicianCombo->setCurrentText(mainWindow->getCurrentUser().name);

    if (modifyMode) {
        ui->patientIdEdit->setText(study.patientId);
        if (!study.accNumber.isEmpty()) ui->accNumEdit->setText(study.accNumber);
        ui->accNumEdit->setReadOnly(true);
        ui->patientSexCombo->setCurrentText(study.patientSex);
        ui->patientBirthDateEdit->setDate(study.patientBirth);
        ui->patientNameEdit->setText(study.patientName);
        ui->reqPhysicianCombo->setCurrentText(study.reqPhysician);
        ui->perPhysicianCombo->setCurrentText(study.perPhysician);
        ui->modalityCombo->setCurrentText(study.modality);
        ui->studyDescEdit->setText(study.studyDesc);
    }

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOk()));
}

void NewStudyDialog::onOk()
{
    study.accNumber = ui->accNumEdit->text();
    study.patientId = ui->patientIdEdit->text();
    study.patientName = ui->patientNameEdit->text();
    if (study.accNumber.isEmpty() || study.patientId.isEmpty()) {
        QMessageBox::critical(this, tr("Register Study"),
                              tr("Mandatory fields empty."));
        return;
    }

    if (study.patientName.isEmpty()) study.patientName = tr("Emergence Patient");
    study.patientSex = trSex2Sex(ui->patientSexCombo->currentText());
    study.patientBirth = ui->patientBirthDateEdit->date();

    study.reqPhysician = ui->reqPhysicianCombo->currentText();
    study.perPhysician = ui->perPhysicianCombo->currentText();
    study.procId = ui->procIdEdit->text();
    study.modality = ui->modalityCombo->currentText();
    study.studyDesc = ui->studyDescEdit->text();

    bool newStudy = false;
    QSettings s;
    if (study.studyUid.isEmpty()) {
        char uid[128];
        study.studyUid = QString::fromLatin1(dcmGenerateUniqueIdentifier(uid, SITE_STUDY_UID_ROOT));
        s.setValue(ACCNUMBER_START, s.value(ACCNUMBER_START).toInt()+1);
        newStudy = true;
    }
    if (!study.studyTime.isValid())
        study.studyTime = QDateTime::currentDateTime();

    if (!modifyMode) {
        s.setValue(PATIENTID_START, s.value(PATIENTID_START).toInt()+1);
        if (!newStudy) s.setValue(ACCNUMBER_START, s.value(ACCNUMBER_START).toInt()+1);

        QStringList phys = s.value(REQ_PHYSICIANS).toStringList();
        phys.removeOne(study.reqPhysician);
        phys.prepend(study.reqPhysician);
        s.setValue(REQ_PHYSICIANS, phys);
        phys = s.value(PER_PHYSICIANS).toStringList();
        phys.removeOne(study.perPhysician);
        phys.prepend(study.perPhysician);
        s.setValue(PER_PHYSICIANS, phys);
    }

    accept();
}
