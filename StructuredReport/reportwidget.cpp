#include "reportwidget.h"
#include "ui_reportwidget.h"
#include "../share/global.h"
#include "../share/studyrecord.h"
#include "srsettingsdialog.h"
#include "../DicomViewer/imageinstance.h"
#include "../DicomViewer/dicomimageview.h"
#include "../MainStation/mainwindow.h"
#include "../MainStation/sqlstudymodel.h"
#include "../MainStation/studydbmanager.h"

#include "dcmtk/dcmpstat/dviface.h"
#include "dcmtk/dcmsr/dsrdoc.h"
#include "dcmtk/ofstd/ofstring.h"

#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QSettings>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QScreen>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCloseEvent>

#define DOCTITLE DSRCodedEntryValue("CODE.01", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                    "Digitalized X-Ray Imaging Structured Report")
#define FINDINGS DSRCodedEntryValue("CODE.02", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                    "Findings")
#define CONCLUSION DSRCodedEntryValue("CODE.03", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                      "Conclusion")
#define IMAGEREF DSRCodedEntryValue("CODE.04", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                    "Image Reference")

#define REPORT_PHYSICIAN DSRCodedEntryValue("CODE.05", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                            "Report Physician")
#define REPORT_TIME DSRCodedEntryValue("CODE.06", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                      "Report Time")
#define REQ_PHYSICIAN DSRCodedEntryValue("CODE.07", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                           "Request Physician")
#define REQ_DEPARTMENT DSRCodedEntryValue("CODE.08", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                      "Request Department")
#define STUDY_METHOD DSRCodedEntryValue("CODE.11", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                       "Study Method")
#define BODYPART_EXAMINED DSRCodedEntryValue("CODE.12", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                             "Body Parts Examined")
#define STATION_NAME DSRCodedEntryValue("CODE.13", OFFIS_CODING_SCHEME_DESIGNATOR,\
                                                       "Station Name")
#define OBS_CONTEXT_CODE DSRCodedEntryValue("CODE.14", OFFIS_CODING_SCHEME_DESIGNATOR, \
                                            "Report author")
#define REPORT_PAPER DSRCodedEntryValue("CODE.15", OFFIS_CODING_SCHEME_DESIGNATOR, \
                                        "Paper size of this report")


ReportWidget::ReportWidget(QWidget *parent) :
    QWidget(parent),
    isModified(0),
    isNormal(true),
    isExternal(false),
    isCreated(false),
    currentView(0),
    dvi(new DVInterface),
    ui(new Ui::ReportWidget)
{
    ui->setupUi(this);
    init();

    initLogicalDpi();
    reflushReportFormat();
}

ReportWidget::~ReportWidget()
{
    delete dvi;
    delete ui;
}

void ReportWidget::init()
{
    setAcceptDrops(true);

    zoomAction = new QAction(tr("Zoom Image"), this);
    panAction = new QAction(tr("Pan Image"), this);
    roiAction = new QAction(tr("Roi Window"), this);
}

bool ReportWidget::openReport(const QString &file, bool external)
{
    OFCondition cond = EC_Normal;
    if (external) {
        dvi->loadStructuredReport(file.toLocal8Bit().data());
        isExternal = true;
    } else {
        dvi->loadStructuredReport(QString("%1/%2").arg(mainWindow->getDbLocation(), file).toLocal8Bit().data());
    }
    if (cond.bad()) {
        QMessageBox::critical(this, tr("Open Structured Report"),
                              tr("Open Structured Report failed: %1").arg(cond.text()));
        isNormal = false;
    } else {
        initContents();
        DSRDocument &doc = dvi->getCurrentReport();
        setReadOnly(doc.getVerificationFlag()==DSRTypes::VF_Verified);
        reportFile = file;
    }
    return isNormal;
}

bool ReportWidget::createReport(const QString &studyUid)
{
    if (!studyUid.isEmpty()) {
        DSRDocument &doc = dvi->getCurrentReport();
        doc.createNewSeriesInStudy(studyUid.toLatin1().data());

        QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
        QSqlQuery query(db);
        query.prepare("SELECT * FROM StudyTable WHERE StudyUid=?");
        query.addBindValue(studyUid);
        if (query.exec() && query.first()) {
            QSqlRecord rec = query.record();
            QString id, name, sex, accnum, desc, reqphy;
            QDateTime studytime;
            QDate birth;
            id = rec.value(SqlStudyModel::PatientId).toString();
            name = rec.value(SqlStudyModel::PatientName).toString();
            sex = rec.value(SqlStudyModel::PatientSex).toString();
            birth = rec.value(SqlStudyModel::PatientBirth).toDate();
            accnum = rec.value(SqlStudyModel::AccNumber).toString();
            desc = rec.value(SqlStudyModel::StudyDesc).toString();
            reqphy = rec.value(SqlStudyModel::ReqPhysician).toString();
            studytime = rec.value(SqlStudyModel::StudyTime).toDateTime();

            QString dir = QString("%1/%2_%3").arg(studytime.date().toString("yyyyMM"),
                                                     studytime.toString(DATETIME_DICOM_FORMAT),
                                                     accnum);
            QDir().mkpath(QString("%1/%2").arg(mainWindow->getDbLocation(), dir));
            reportFile = QString("%1/%2_%3.dcm").arg(dir, QString(REPORT_PREFIX), QString::fromLatin1(doc.getSOPInstanceUID()));

            doc.setSpecificCharacterSetType(DSRTypes::CS_Latin1);
            doc.setPatientID(id.toLocal8Bit().data());
            doc.setPatientName(name.toLocal8Bit().data());
            doc.setPatientSex(sex.toLatin1().data());
            doc.setPatientBirthDate(birth.toString(DATE_DICOM_FORMAT).toLatin1().data());
            doc.setAccessionNumber(accnum.toLocal8Bit().data());
            doc.setStudyDescription(desc.toLocal8Bit().data());
            doc.setStudyDate(studytime.date().toString(DATE_DICOM_FORMAT).toLatin1().data());
            doc.setStudyTime(studytime.time().toString(TIME_DICOM_FORMAT).toLatin1().data());

            // SR template
            doc.getTree().addContentItem(DSRTypes::RT_isRoot, DSRTypes::VT_Container);
            doc.getTree().getCurrentContentItem().setConceptName(DOCTITLE);

            initContents();
            isModified = true;
            isCreated = true;

        } else isNormal = false;

    } else isNormal = false;

    return isNormal;
}

void ReportWidget::initContents()
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (doc.isValid()) {
        OFString value;
        ui->institutionEdit->setText(mainWindow->getInstInfo().name);
        ui->patientIdEdit->setText(QString::fromLocal8Bit(doc.getPatientID(value).c_str()));
        ui->accNumberEdit->setText(QString::fromLocal8Bit(doc.getAccessionNumber(value).c_str()));
        ui->patientNameEdit->setText(QString::fromLocal8Bit(doc.getPatientName(value).c_str()));
        ui->reqPhysicianEdit->setText(QString::fromLocal8Bit(doc.getReferringPhysicianName(value).c_str()));
        ui->patientSexEdit->setText(sex2trSex(QString::fromLatin1(doc.getPatientSex(value).c_str())));
        ui->patientBirthEdit->setText(QDate::fromString(QString::fromLatin1(doc.getPatientBirthDate(value).c_str()), DATE_DICOM_FORMAT)
                                      .toString(Qt::DefaultLocaleShortDate));
        QDateTime studyTime;
        studyTime.setDate(QDate::fromString(QString::fromLatin1(doc.getStudyDate(value).c_str()), DATE_DICOM_FORMAT));
        studyTime.setTime(formatDicomTime(QString::fromLatin1(doc.getStudyTime(value).c_str())));
        ui->studyDateEdit->setText(studyTime.toString(Qt::DefaultLocaleShortDate));

        if (doc.getTree().gotoNamedNode(REPORT_PAPER))
            resizeReport(QString::fromLatin1(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        else resizeReport();

        /*
        if (doc.getTree().gotoNamedNode(INSTITUTION))
            ui->institutionEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        */
        if (doc.getTree().gotoNamedNode(REQ_PHYSICIAN))
            ui->reqPhysicianEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(REQ_DEPARTMENT))
            ui->reqDivisionEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(STUDY_METHOD))
            ui->examMethodEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(BODYPART_EXAMINED))
            ui->bodyPartEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(STATION_NAME))
            ui->stationNameEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(FINDINGS))
            ui->findingsEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(CONCLUSION))
            ui->diagnoseEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(REPORT_PHYSICIAN))
            ui->reportPhysicianEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(REPORT_TIME))
            ui->reportTimeEdit->setText(QDateTime::fromString(doc.getTree().getCurrentContentItem().getStringValue().c_str(), DATETIME_DICOM_FORMAT)
                                        .toString(Qt::DefaultLocaleShortDate));
        /*
        if (doc.getTree().gotoNamedNode(VERIFY_PHYSICIAN))
            ui->verifyPhysicianEdit->setText(QString::fromLocal8Bit(doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        if (doc.getTree().gotoNamedNode(VERIFY_TIME))
            ui->verifyTimeEdit->setText(QDateTime::fromString(doc.getTree().getCurrentContentItem().getStringValue().c_str(), DATETIME_DICOM_FORMAT)
                                        .toString(Qt::DefaultLocaleShortDate));
                                        */
        if (doc.getNumberOfVerifyingObservers()) {
            OFString time, name, org;
            doc.getVerifyingObserver(doc.getNumberOfVerifyingObservers(), time, name, org);
            ui->verifyPhysicianEdit->setText(QString::fromLocal8Bit(name.c_str()));
            ui->verifyTimeEdit->setText(QDateTime::fromString(QString::fromLatin1(time.c_str()), DATETIME_DICOM_FORMAT)
                                        .toString(Qt::SystemLocaleDate));
        }
    }

    connect(ui->findingsEdit, SIGNAL(textChanged()), this, SLOT(onContentChanged()));
    connect(ui->diagnoseEdit, SIGNAL(textChanged()), this, SLOT(onContentChanged()));
}

void ReportWidget::loadEvidenceImage(ImageInstance *image)
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (image && (image->studyUid == QString::fromLatin1(doc.getStudyInstanceUID()))) {
        OFCondition result = doc.getCurrentRequestedProcedureEvidence().gotoFirstItem();
        bool found = false;
        OFString instanceUid;
        while (EC_Normal == result) {
            doc.getCurrentRequestedProcedureEvidence().getSOPInstanceUID(instanceUid);
            if (image->instanceUid == QString::fromLatin1(instanceUid.c_str())) {
                found = true;
                break;
            }
            result = doc.getCurrentRequestedProcedureEvidence().gotoNextItem();
        }

        if (found) {
            found = false;
            foreach (DicomImageView *view, viewList) {
                if (view->getImageInstance() && (view->getImageInstance()->instanceUid == image->instanceUid)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                doInsertImage(image);
            }
        }
    }
}

void ReportWidget::reflushReportFormat()
{
    QSettings settings;
    QFont font;
    if (font.fromString(settings.value(INSTITUTION_NAME_FONT).toString())) {
        ui->institutionEdit->setFont(font);
    } else {
        font = ui->institutionEdit->font();
        settings.setValue(INSTITUTION_NAME_FONT, font.toString());
    }
    if (font.fromString(settings.value(REPORT_TITLE_FONT).toString())) {
        ui->reportTitleEdit->setFont(font);
    } else {
        font = ui->reportTitleEdit->font();
        settings.setValue(REPORT_TITLE_FONT, font.toString());
    }
    if (settings.value(REPORT_TITLE_TEXT).toString().isEmpty()) {
        settings.setValue(REPORT_TITLE_TEXT, tr("Digitalized X-Ray Structured Report"));
    }
    ui->reportTitleEdit->setText(settings.value(REPORT_TITLE_TEXT).toString());
    if (font.fromString(settings.value(PATIENT_INFO_FONT).toString())) {
        ui->patientIdLabel->setFont(font);
        ui->patientIdEdit->setFont(font);
        ui->accNumberLabel->setFont(font);
        ui->accNumberEdit->setFont(font);
        ui->patientNameLabel->setFont(font);
        ui->patientNameEdit->setFont(font);
        ui->patientSexLabel->setFont(font);
        ui->patientSexEdit->setFont(font);
        ui->patientBirthLabel->setFont(font);
        ui->patientBirthEdit->setFont(font);
        ui->reqPhysicianLabel->setFont(font);
        ui->reqPhysicianEdit->setFont(font);
        ui->reqDivisionLabel->setFont(font);
        ui->reqDivisionEdit->setFont(font);
        ui->studyDateLabel->setFont(font);
        ui->studyDateEdit->setFont(font);
        ui->bodyPartLabel->setFont(font);
        ui->bodyPartEdit->setFont(font);
        ui->examMethodLabel->setFont(font);
        ui->examMethodEdit->setFont(font);
        ui->stationNameLabel->setFont(font);
        ui->stationNameEdit->setFont(font);
    } else {
        font = ui->patientIdEdit->font();
        settings.setValue(PATIENT_INFO_FONT, font.toString());
    }
    if (font.fromString(settings.value(REPORT_TEXT_FONT).toString())) {
        ui->findingsLabel->setFont(font);
        ui->findingsEdit->setFont(font);
        ui->diagnoseLabel->setFont(font);
        ui->diagnoseEdit->setFont(font);
    } else {
        font = ui->findingsEdit->font();
        settings.setValue(REPORT_TEXT_FONT, font.toString());
    }
    if (font.fromString(settings.value(PHYSICIAN_INFO_FONT).toString())) {
        ui->reportPhysicianLabel->setFont(font);
        ui->reportPhysicianEdit->setFont(font);
        ui->verifyPhysicianLabel->setFont(font);
        ui->verifyPhysicianEdit->setFont(font);
        ui->reportTimeLabel->setFont(font);
        ui->reportTimeEdit->setFont(font);
        ui->verifyTimeLabel->setFont(font);
        ui->verifyTimeEdit->setFont(font);
    } else {
        font = ui->reportPhysicianEdit->font();
        settings.setValue(PHYSICIAN_INFO_FONT, font.toString());
    }
    if (font.fromString(settings.value(FOOTNOTE_FONT).toString())) {
        ui->footNoteEdit->setFont(font);
    } else {
        font = ui->footNoteEdit->font();
        settings.setValue(FOOTNOTE_FONT, font.toString());
    }
    if (settings.value(FOOTNOTE_TEXT).toString().isEmpty()) {
        settings.setValue(FOOTNOTE_TEXT, tr("Reference Only, Valid on physician's signature."));
    }
    ui->footNoteEdit->setText(settings.value(FOOTNOTE_TEXT).toString());

    if (settings.value(IMAGE_WIDTH).toInt() <= 0) {
        settings.setValue(IMAGE_WIDTH, 50.0);
    }
    if (settings.value(IMAGE_HEIGHT).toInt() <= 0) {
        settings.setValue(IMAGE_HEIGHT, 50.0);
    }

    QSize viewSize = QSize(settings.value(IMAGE_WIDTH).toDouble()*xLogicalDpi/INCH_MM_RATIO,
                           settings.value(IMAGE_HEIGHT).toDouble()*yLogicalDpi/INCH_MM_RATIO);
    foreach (DicomImageView *v, viewList) {
        v->setSizeHint(viewSize);
        v->resize(viewSize);
    }
}

bool ReportWidget::isReportVerified() const
{
    DSRDocument &doc = dvi->getCurrentReport();
    return doc.getVerificationFlag()==DSRTypes::VF_Verified;
}

QString ReportWidget::getReportTitle() const
{
    DSRDocument &doc = dvi->getCurrentReport();
    QString title("%1 %2/%3 - [%4] [%5]");
    OFString value;
    QString age("%1%2");
    QDate birthdate = QDate::fromString(QString::fromLatin1(doc.getPatientBirthDate(value).c_str()), DATE_DICOM_FORMAT);
    if (birthdate.isValid()) {
        QDate studyDate = QDate::fromString(QString::fromLatin1(doc.getStudyDate(value).c_str()), DATE_DICOM_FORMAT);
        if (studyDate.isValid()) {
            qint64 days = birthdate.daysTo(studyDate);
            if (days < 31)
                age = age.arg(QString::number(days), tr("Days"));
            else if (days < 92)
                age = age.arg(QString::number(days/7), tr("Weeks"));
            else if (days < 731)
                age = age.arg(QString::number(days/30), tr("Months"));
            else
                age = age.arg(QString::number(days/365), tr("Years"));
        } else {
            age = birthdate.toString(Qt::DefaultLocaleShortDate);
        }
    }

    return title.arg(ui->patientNameEdit->text(),
                     ui->patientSexEdit->text(),
                     age,
                     doc.getCompletionFlag()==DSRTypes::CF_Complete?tr("Completed"):tr("UnCompleted"),
                     doc.getVerificationFlag()==DSRTypes::VF_Verified?tr("Verified"):tr("Unverified"));

}

void ReportWidget::setReportPaper(const QString &paper)
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (doc.getTree().gotoNamedNode(REPORT_PAPER)) {
        if (paper != QString::fromLatin1(doc.getTree().getCurrentContentItem().getStringValue().c_str())) {
            doc.getTree().getCurrentContentItem().setStringValue(paper.toLatin1().data());
            resizeReport(paper);
            isModified = true;
        }
    } else {
        doc.getTree().gotoRoot();
        doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text, DSRTypes::AM_belowCurrent);
        doc.getTree().getCurrentContentItem().setConceptName(REPORT_PAPER);
        doc.getTree().getCurrentContentItem().setStringValue(paper.toLatin1().data());
        resizeReport(paper);
        isModified = true;
    }
}

QString ReportWidget::getReportPaper() const
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (doc.getTree().gotoNamedNode(REPORT_PAPER))
        return QString::fromLatin1(doc.getTree().getCurrentContentItem().getStringValue().c_str());
    return QString();
}

QString ReportWidget::getStudyUid() const
{
    DSRDocument &doc = dvi->getCurrentReport();
    return QString::fromLatin1(doc.getStudyInstanceUID());
}

void ReportWidget::initLogicalDpi()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QList<QScreen*> screenList = QApplication::screens();
    QScreen *screen = screenList.at(desktop->screenNumber(this));
    xLogicalDpi = screen->logicalDotsPerInchX();
    yLogicalDpi = screen->logicalDotsPerInchY();
}

void ReportWidget::setReadOnly(bool yes)
{
    ui->institutionEdit->setReadOnly(yes);
    ui->reqPhysicianEdit->setReadOnly(yes);
    ui->reqDivisionEdit->setReadOnly(yes);
    ui->findingsEdit->setReadOnly(yes);
    ui->diagnoseEdit->setReadOnly(yes);
    ui->bodyPartEdit->setReadOnly(yes);
    ui->examMethodEdit->setReadOnly(yes);
}

void ReportWidget::resizeReport(const QString &paper)
{
    QSize size;
    if (paper == "A5") {
        size.setWidth((148/INCH_MM_RATIO)*xLogicalDpi);
        size.setHeight((210/INCH_MM_RATIO)*yLogicalDpi);
    } else if (paper == "A3") {
        size.setWidth((297/INCH_MM_RATIO)*xLogicalDpi);
        size.setHeight((420/INCH_MM_RATIO)*yLogicalDpi);
    } else if (paper == "B4") {
        size.setWidth((250/INCH_MM_RATIO)*xLogicalDpi);
        size.setHeight((353/INCH_MM_RATIO)*yLogicalDpi);
    } else if (paper == "B5") {
        size.setWidth(6.93*xLogicalDpi);
        size.setHeight(9.84*yLogicalDpi);
    } else if (paper == "Customize") {
        size.setWidth(8.26*xLogicalDpi);
        size.setHeight(11.69*yLogicalDpi);
    } else {
        size.setWidth(8.26*xLogicalDpi);
        size.setHeight(11.69*yLogicalDpi);
    }
    resize(size);
}

void ReportWidget::onViewChanged(DicomImageView *view)
{
    if (currentView) currentView->setHighlight(false);
    currentView = view;
    if (currentView) currentView->setHighlight(true);
}

void ReportWidget::insertImage(ImageInstance *image)
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (doc.getVerificationFlag()==DSRTypes::VF_Verified) {
        QMessageBox::critical(this, tr("Insert Image Reference"),
                              tr("This report has already verified, cannot be modified."));
        return;
    }
    if (doc.getCurrentRequestedProcedureEvidence().getNumberOfInstances() >= 4) {
        QMessageBox::critical(this, tr("Insert Image Reference"),
                              tr("At most 4 image references can be added."));
        return;
    }

    if (image && (image->studyUid == QString::fromLatin1(doc.getStudyInstanceUID()))) {
        if (doc.getCurrentRequestedProcedureEvidence().gotoItem(image->studyUid.toLatin1().data(),
                                                                image->seriesUid.toLatin1().data(),
                                                                image->instanceUid.toLatin1().data()) != EC_Normal) {
            doc.getCurrentRequestedProcedureEvidence().addItem(image->studyUid.toLatin1().data(),
                                                               image->seriesUid.toLatin1().data(),
                                                               image->sopClassUid.toLatin1().data(),
                                                               image->instanceUid.toLatin1().data());
            doc.getTree().gotoRoot();
            doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Image, DSRTypes::AM_belowCurrent);
            doc.getTree().getCurrentContentItem().setConceptName(IMAGEREF);
            doc.getTree().getCurrentContentItem().setImageReference(DSRImageReferenceValue(image->sopClassUid.toLatin1().data(),
                                                                                           image->instanceUid.toLatin1().data()));

            doInsertImage(image);
            isModified = true;
        } else {
            QMessageBox::warning(this, tr("Insert Image Reference"),
                                 tr("Currently referenced image is already added."));
        }
    } else {
        QMessageBox::warning(this, tr("Insert Image Reference"),
                             tr("Get image failed, be sure it's a valid image object."));
    }
}

void ReportWidget::doInsertImage(ImageInstance *image)
{
    DicomImageView *view = createImageView(image);
    viewList.append(view);
    ui->imageHBoxLayout->insertWidget(viewList.size(), view);
    ui->imageHBoxLayout->setAlignment(view, Qt::AlignCenter);
    ui->reqPhysicianEdit->setText(image->reqPhysician);
    ui->bodyPartEdit->setText(image->bodyPart);
    ui->stationNameEdit->setText(image->stationName);
    if (currentView) currentView->setHighlight(false);
    currentView = view;
    currentView->setHighlight(true);
}

DicomImageView *ReportWidget::createImageView(ImageInstance *image)
{
    QSettings settings;
    DicomImageView *view = new DicomImageView(image);
    view->setViewMode(DicomImageView::VM_Report);
    view->setContextMenuPolicy(Qt::ActionsContextMenu);
    view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    view->setSizeHint(QSize(settings.value(IMAGE_WIDTH).toDouble()*xLogicalDpi/INCH_MM_RATIO,
                            settings.value(IMAGE_HEIGHT).toDouble()*yLogicalDpi/INCH_MM_RATIO));

    QAction *rotateCCW = new QAction(tr("Rotate CCW"), view);
    QAction *rotateCW = new QAction(tr("Rotate CW"), view);
    QAction *hflip = new QAction(tr("H Flip"), view);
    QAction *vflip = new QAction(tr("V Flip"), view);
    QAction *negative = new QAction(tr("Negative"), view);
    QAction *reset = new QAction(tr("Reset"), view);
    QAction *remove = new QAction(tr("Remove"), view);

    connect(view, SIGNAL(viewClicked(DicomImageView*)), this, SLOT(onViewChanged(DicomImageView*)));
    connect(zoomAction, SIGNAL(triggered()), view, SLOT(zoomImage()));
    connect(panAction, SIGNAL(triggered()), view, SLOT(panImage()));
    connect(roiAction, SIGNAL(triggered()), view, SLOT(setRoiWindow()));
    connect(rotateCCW, SIGNAL(triggered()), view, SLOT(rotateCCW()));
    connect(rotateCW, SIGNAL(triggered()), view, SLOT(rotateCW()));
    connect(hflip, SIGNAL(triggered()), view, SLOT(hFlip()));
    connect(vflip, SIGNAL(triggered()), view, SLOT(vFlip()));
    connect(negative, SIGNAL(triggered()), view, SLOT(inverseImage()));
    connect(reset, SIGNAL(triggered()), view, SLOT(reset()));
    connect(remove, SIGNAL(triggered()), this, SLOT(removeImage()));

    remove->setEnabled(mainWindow->getCurrentGroup().permissions & GP_ModifyReport);
    QList<QAction*> actions;
    actions << zoomAction << panAction << roiAction <<
               rotateCCW << rotateCW << hflip <<
               vflip << negative << reset << remove;
    view->addActions(actions);

    return view;
}

void ReportWidget::removeImage()
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (doc.getVerificationFlag()==DSRTypes::VF_Verified) {
        QMessageBox::critical(this, tr("Insert Image Reference"),
                              tr("This report has already verified, cannot be modified."));
        return;
    }

    ImageInstance *image;
    if (currentView && (image = currentView->getImageInstance())) {
        doc.getCurrentRequestedProcedureEvidence().removeItem(OFString(image->studyUid.toLatin1().data()),
                                                              OFString(image->seriesUid.toLatin1().data()),
                                                              OFString(image->instanceUid.toLatin1().data()));
        ui->imageHBoxLayout->removeWidget(currentView);
        viewList.removeOne(currentView);
        currentView->deleteLater();
        if (viewList.size()) {
            currentView = viewList.first();
            currentView->setHighlight(true);
        } else currentView = 0;

        doc.getTree().gotoRoot();
        while (doc.getTree().gotoNextNamedNode(IMAGEREF)) {
            const DSRImageReferenceValue &imageRef = doc.getTree().getCurrentContentItem().getImageReference();
            if (imageRef.isValid() && imageRef.getSOPInstanceUID() == OFString(image->instanceUid.toLatin1().data())) {
                doc.getTree().removeCurrentContentItem();
                break;
            }
        }

        isModified = true;
    }
}

void ReportWidget::saveReport()
{
    DSRDocument &doc = dvi->getCurrentReport();

    doc.setContentDate(QDate::currentDate().toString(DATE_DICOM_FORMAT).toLatin1().data());
    doc.setContentTime(QTime::currentTime().toString(TIME_DICOM_FORMAT).toLatin1().data());

    if (!ui->reqPhysicianEdit->text().isEmpty()) {
        if (!doc.getTree().gotoNamedNode(REQ_PHYSICIAN)) {
            doc.getTree().gotoRoot();
            doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text);
            doc.getTree().getCurrentContentItem().setConceptName(REQ_PHYSICIAN);
        }
        doc.getTree().getCurrentContentItem().setStringValue(ui->reqPhysicianEdit->text().toLocal8Bit().data());
    }

    if (!ui->reqDivisionEdit->text().isEmpty()) {
        if (!doc.getTree().gotoNamedNode(REQ_DEPARTMENT)) {
            doc.getTree().gotoRoot();
            doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text);
            doc.getTree().getCurrentContentItem().setConceptName(REQ_DEPARTMENT);
        }
        doc.getTree().getCurrentContentItem().setStringValue(ui->reqDivisionEdit->text().toLocal8Bit().data());
    }

    if (!ui->examMethodEdit->text().isEmpty()) {
        if (!doc.getTree().gotoNamedNode(STUDY_METHOD)) {
            doc.getTree().gotoRoot();
            doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text);
            doc.getTree().getCurrentContentItem().setConceptName(STUDY_METHOD);
        }
        doc.getTree().getCurrentContentItem().setStringValue(ui->examMethodEdit->text().toLocal8Bit().data());
    }

    if (!ui->stationNameEdit->text().isEmpty()) {
        if (!doc.getTree().gotoNamedNode(STATION_NAME)) {
            doc.getTree().gotoRoot();
            doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text);
            doc.getTree().getCurrentContentItem().setConceptName(STATION_NAME);
        }
        doc.getTree().getCurrentContentItem().setStringValue(ui->stationNameEdit->text().toLocal8Bit().data());
    }

    if (!ui->bodyPartEdit->text().isEmpty()) {
        if (!doc.getTree().gotoNamedNode(BODYPART_EXAMINED)) {
            doc.getTree().gotoRoot();
            doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text);
            doc.getTree().getCurrentContentItem().setConceptName(BODYPART_EXAMINED);
        }
        doc.getTree().getCurrentContentItem().setStringValue(ui->bodyPartEdit->text().toLocal8Bit().data());
    }

    QString findings = ui->findingsEdit->toPlainText();
    if (!findings.isEmpty()) {
        if (!doc.getTree().gotoNamedNode(FINDINGS)) {
            doc.getTree().gotoRoot();
            doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text, DSRTypes::AM_belowCurrent);
            doc.getTree().getCurrentContentItem().setConceptName(FINDINGS);
        }
        doc.getTree().getCurrentContentItem().setStringValue(findings.toLocal8Bit().data());
    }

    QString conclusion = ui->diagnoseEdit->toPlainText();
    if (!conclusion.isEmpty()) {
        if (!doc.getTree().gotoNamedNode(CONCLUSION)) {
            doc.getTree().gotoRoot();
            doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_Text, DSRTypes::AM_belowCurrent);
            doc.getTree().getCurrentContentItem().setConceptName(CONCLUSION);
        }
        doc.getTree().getCurrentContentItem().setStringValue(ui->diagnoseEdit->toPlainText().toLocal8Bit().data());
    }


    OFCondition cond = EC_Normal;
    if (isExternal)
        cond = dvi->saveStructuredReport(reportFile.toLocal8Bit().data());
    else
        cond = dvi->saveStructuredReport(QString("%1/%2").arg(mainWindow->getDbLocation(), reportFile).toLocal8Bit().data());

    if (cond.good()) {
        ReportRecord record;
        record.reportUid = QString::fromLatin1(doc.getSOPInstanceUID());
        record.seriesUid = QString::fromLatin1(doc.getSeriesInstanceUID());
        record.studyUid = QString::fromLatin1(doc.getStudyInstanceUID());
        record.createTime.setDate(QDate::fromString(QString::fromLatin1(doc.getInstanceCreationDate()),
                                                    DATE_DICOM_FORMAT));
        record.createTime.setTime(formatDicomTime(QString::fromLatin1(doc.getInstanceCreationTime())));
        record.contentTime.setDate(QDate::fromString(QString::fromLatin1(doc.getContentDate()),
                                                     DATE_DICOM_FORMAT));
        record.contentTime.setTime(formatDicomTime(QString::fromLatin1(doc.getContentTime())));
        record.isCompleted = (doc.getCompletionFlag()==DSRTypes::CF_Complete)?tr("Completed"):tr("Uncompleted");
        record.isVerified = (doc.getVerificationFlag()==DSRTypes::VF_Verified)?tr("Verified"):tr("Unverified");
        record.reportFile = reportFile;

        if (isCreated) StudyDbManager::insertReportToDb(record);
        else if (!isExternal) StudyDbManager::updateReportStatus(record);

        emit reportCreated();
        isModified = false;
    } else {
        QMessageBox::warning(this, tr("Save Structured Report"),
                             tr("Save report failed: %1.").arg(QString::fromLatin1(cond.text())));
    }

}

void ReportWidget::completeReport()
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (doc.getCompletionFlag() == DSRTypes::CF_Complete) {
        QMessageBox::critical(this, tr("Complete Report"),
                              tr("This report is already completed, cannot be completed again."));
        return;
    }

    OFCondition l_error = doc.completeDocument();
    if (EC_Normal != l_error) {
        QMessageBox::warning(this, tr("Complete Report"),
                             tr("Complete report failed: %1").arg(l_error.text()));
    } else {
        QString userId = mainWindow->getCurrentUser().name;
        doc.getTree().gotoRoot();
        doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_PName, DSRTypes::AM_belowCurrent);
        doc.getTree().getCurrentContentItem().setConceptName(REPORT_PHYSICIAN);
        doc.getTree().getCurrentContentItem().setStringValue(userId.toLocal8Bit().data());
        doc.getTree().addContentItem(DSRTypes::RT_contains, DSRTypes::VT_DateTime);
        doc.getTree().getCurrentContentItem().setConceptName(REPORT_TIME);
        doc.getTree().getCurrentContentItem().setStringValue(QDateTime::currentDateTime()
                                                                     .toString(DATETIME_DICOM_FORMAT).toLatin1().data());

        // reflush
        doc.getTree().gotoNamedNode(REPORT_PHYSICIAN);
        ui->reportPhysicianEdit->setText(QString::fromLocal8Bit(
                                             doc.getTree().getCurrentContentItem().getStringValue().c_str()));
        doc.getTree().gotoNamedNode(REPORT_TIME);
        QString timeStr = QString::fromLatin1(doc.getTree().getCurrentContentItem().getStringValue().c_str());
        QDateTime time = QDateTime::fromString(timeStr, DATETIME_DICOM_FORMAT);
        ui->reportTimeEdit->setText(time.toString(Qt::SystemLocaleDate));

        isModified = true;
    }
}

void ReportWidget::verifyReport()
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (doc.getCompletionFlag() != DSRTypes::CF_Complete) {
        QMessageBox::critical(this, tr("Verify Report"),
                              tr("This report is not yet completed, cannot be verified."));
        return;
    }

    QString userId = mainWindow->getCurrentUser().name;
    QString userDiv = mainWindow->getCurrentUser().division;
    OFCondition l_error = doc.verifyDocument(userId.toLocal8Bit().data(), userDiv.toLocal8Bit().data());
    if (EC_Normal != l_error) {
        QMessageBox::warning(this, tr("Verify Report"),
                             tr("Verify report failed: %1").arg(l_error.text()));
    } else {
        OFString datetime;
        OFString name;
        OFString org;
        doc.getVerifyingObserver(doc.getNumberOfVerifyingObservers(),
                                 datetime, name, org);
        ui->verifyPhysicianEdit->setText(QString::fromLocal8Bit(name.c_str()));
        ui->verifyTimeEdit->setText(QDateTime::fromString(datetime.c_str(), DATETIME_DICOM_FORMAT)
                                    .toString(Qt::SystemLocaleDate));

        isModified = true;
        setReadOnly(true);
    }
}

void ReportWidget::closeEvent(QCloseEvent *e)
{
    if (isModified) {
        QMessageBox::StandardButtons button = QMessageBox::question(this, tr("Close Structured Report"),
                                                                    tr("Report has been modified, save it?"),
                                                                    QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (QMessageBox::Yes == button) {
            saveReport();
            e->accept();
        } else if (QMessageBox::Cancel == button) {
            e->ignore();
        }
    }

    if (e->isAccepted()) {
        DSRDocument &doc = dvi->getCurrentReport();
        emit reportClosed(QString::fromLatin1(doc.getStudyInstanceUID()));
    }
}

void ReportWidget::appendTemplateContents(const QString &findings, const QString &conclusion)
{
    DSRDocument &doc = dvi->getCurrentReport();
    if (doc.getVerificationFlag() != DSRTypes::VF_Verified) {
        ui->findingsEdit->append(findings);
        ui->diagnoseEdit->append(conclusion);
    }
}

void ReportWidget::print()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted)
        doPrint(&printer);
}

void ReportWidget::printPreview()
{
    QPrintPreviewDialog dialog(this);
    connect(&dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(doPrint(QPrinter*)));
    dialog.setWindowFlags(dialog.windowFlags()|Qt::WindowMaximizeButtonHint);
    dialog.exec();
}

void ReportWidget::doPrint(QPrinter *printer)
{
    QPainter painter(printer);
    double mm2dot = printer->resolution()/(double)25.40;
    static int BlockSpace = 3*mm2dot;

    QRect viewRect = painter.viewport();
    int totalHeight = 0;
    int height;

    painter.setFont(ui->institutionEdit->font());
    height = painter.fontMetrics().height();
    painter.drawText(QRect(0, totalHeight, viewRect.width(), height),
                     Qt::AlignCenter, ui->institutionEdit->text());
    totalHeight += height;

    painter.setFont(ui->reportTitleEdit->font());
    height = painter.fontMetrics().height();
    painter.drawText(QRect(0, totalHeight, viewRect.width(), height),
                     Qt::AlignCenter, ui->reportTitleEdit->text());
    totalHeight += height;
    totalHeight += BlockSpace*3;

    painter.setFont(ui->patientIdLabel->font());
    height = painter.fontMetrics().height();
    painter.drawText(0, totalHeight, ui->patientIdLabel->text().append(ui->patientIdEdit->text()));
    painter.drawText(viewRect.width()*2/3, totalHeight,
                     ui->accNumberLabel->text().append(ui->accNumberEdit->text()));
    totalHeight += height;
    totalHeight -= BlockSpace;

    QPen linePen;
    QPen oldPen = painter.pen();
    height = 1*mm2dot;
    linePen.setWidth(height);
    linePen.setColor(ui->separateLine1->palette().color(QPalette::WindowText));
    painter.setPen(linePen);
    painter.drawLine(0, totalHeight+height/2, viewRect.width(), totalHeight+height/2);
    totalHeight += height;
    totalHeight += BlockSpace*2;

    painter.setPen(oldPen);
    painter.setFont(ui->patientNameEdit->font());
    height = painter.fontMetrics().height();
    painter.drawText(0, totalHeight,
                     ui->patientNameLabel->text().append(ui->patientNameEdit->text()));
    painter.drawText(viewRect.width()/3, totalHeight,
                     ui->patientSexLabel->text().append(ui->patientSexEdit->text()));
    painter.drawText(viewRect.width()*2/3, totalHeight,
                     ui->patientBirthLabel->text().append(ui->patientBirthEdit->text()));
    totalHeight += height;

    painter.drawText(0, totalHeight,
                     ui->reqPhysicianLabel->text().append(ui->reqPhysicianEdit->text()));
    painter.drawText(viewRect.width()/3, totalHeight,
                     ui->reqDivisionLabel->text().append(ui->reqDivisionEdit->text()));
    painter.drawText(viewRect.width()*2/3, totalHeight,
                     ui->studyDateLabel->text().append(ui->studyDateEdit->text()));
    totalHeight += height;

    painter.drawText(0, totalHeight,
                     ui->bodyPartLabel->text().append(ui->bodyPartEdit->text()));
    painter.drawText(viewRect.width()/3, totalHeight,
                     ui->examMethodLabel->text().append(ui->examMethodEdit->text()));
    painter.drawText(viewRect.width()*2/3, totalHeight,
                     ui->stationNameLabel->text().append(ui->stationNameEdit->text()));
    totalHeight += height;
    totalHeight -= BlockSpace;

    painter.setPen(linePen);
    height = 1*mm2dot;
    painter.drawLine(0, totalHeight+height/2, viewRect.width(), totalHeight+height/2);
    totalHeight += height;
    totalHeight += BlockSpace;

    if (viewList.size()) {
        QSettings settings;
        int imageWidth = settings.value(IMAGE_WIDTH).toDouble()*mm2dot;
        int imageHeight = settings.value(IMAGE_HEIGHT).toDouble()*mm2dot;
        int gridWidth = viewRect.width()/viewList.size();
        int height = 0;
        if (imageWidth > gridWidth) imageWidth = gridWidth;

        if (viewRect.height() - totalHeight < height) {
            printer->newPage();
            totalHeight = 0;
        }

        for (int i = 0; i < viewList.size(); ++i) {
            QPixmap pixmap = viewList[i]->getHardCopyPixmap().scaled(imageWidth, imageHeight, Qt::KeepAspectRatio);
            height = pixmap.height();
            painter.drawPixmap(i*gridWidth + (gridWidth-pixmap.width())/2,
                               totalHeight, pixmap);
        }
        totalHeight += height;
        totalHeight += BlockSpace;
    }


    totalHeight += BlockSpace*2;
    painter.setPen(oldPen);
    painter.setFont(ui->findingsLabel->font());
    height = painter.fontMetrics().height();
    if (viewRect.height() - totalHeight < height) {
        printer->newPage();
        totalHeight = 0;
    }
    painter.drawText(0, totalHeight, ui->findingsLabel->text());
    totalHeight += height;
    totalHeight -= BlockSpace;

    painter.setFont(ui->findingsEdit->font());
    height = painter.fontMetrics().boundingRect(viewRect, Qt::AlignLeft|Qt::TextWordWrap,
                                                ui->findingsEdit->toPlainText()).height();
    if (viewRect.height() - totalHeight < height) {
        printer->newPage();
        totalHeight = 0;
    }
    painter.drawText(QRect(0, totalHeight, viewRect.width(), height),
                     Qt::AlignLeft|Qt::TextWordWrap,
                     ui->findingsEdit->toPlainText());
    totalHeight += height;
    totalHeight += BlockSpace*3;

    painter.setFont(ui->diagnoseLabel->font());
    height = painter.fontMetrics().height();
    if (viewRect.height() - totalHeight < height) {
        printer->newPage();
        totalHeight = 0;
    }
    painter.drawText(0, totalHeight, ui->diagnoseLabel->text());
    totalHeight += height;
    totalHeight -= BlockSpace;

    painter.setFont(ui->diagnoseEdit->font());
    height = painter.fontMetrics().boundingRect(viewRect, Qt::AlignLeft|Qt::TextWordWrap,
                                                ui->diagnoseEdit->toPlainText()).height();
    if (viewRect.height() - totalHeight < height) {
        printer->newPage();
        totalHeight = 0;
    }
    painter.drawText(QRect(0, totalHeight, viewRect.width(), height),
                     Qt::AlignLeft|Qt::TextWordWrap,
                     ui->diagnoseEdit->toPlainText());
    totalHeight += height;
    totalHeight += BlockSpace*2;

    painter.setFont(ui->reportPhysicianEdit->font());
    height = painter.fontMetrics().height();
    if (viewRect.height() - totalHeight < height) {
        printer->newPage();
        totalHeight = 0;
    }
    painter.drawText(viewRect.width()/3, totalHeight,
                     ui->reportPhysicianLabel->text().append(ui->reportPhysicianEdit->text()));
    painter.drawText(viewRect.width()*2/3, totalHeight,
                     ui->reportTimeLabel->text().append(ui->reportTimeEdit->text()));
    totalHeight += height;

    if (viewRect.height() - totalHeight < height) {
        printer->newPage();
        totalHeight = 0;
    }
    painter.drawText(viewRect.width()/3, totalHeight,
                     ui->verifyPhysicianLabel->text().append(ui->verifyPhysicianEdit->text()));
    painter.drawText(viewRect.width()*2/3, totalHeight,
                     ui->verifyTimeLabel->text().append(ui->verifyTimeEdit->text()));
    totalHeight += height;
    totalHeight -= BlockSpace;

    height = 1*mm2dot;
    painter.setPen(linePen);
    painter.drawLine(0, totalHeight+height/2, viewRect.width(), totalHeight+height/2);
    totalHeight += height;
    totalHeight += BlockSpace*2;

    painter.setPen(oldPen);
    painter.setFont(ui->footNoteEdit->font());
    height = painter.fontMetrics().height();
    painter.drawText(0, totalHeight, ui->footNoteEdit->text());

    /*
    QPixmap pixmap(rect().size());
    render(&pixmap);
    QPainter painter(printer);
    QRect viewRect = painter.viewport();
    QPixmap scaled = pixmap.scaled(viewRect.width(), viewRect.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap((viewRect.width()-scaled.width())/2, 0, scaled);
    */
}

