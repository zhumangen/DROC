#include "imageacquisitwidget.h"
#include "ui_imageacquisitwidget.h"

#include "../share/global.h"
#include "../share/configfiles.h"
#include "../MainStation/mainwindow.h"
#include "procedureitemmodel.h"
#include "bodyprotographicsview.h"
#include "../DicomViewer/thumbnailbarwidget.h"
#include "../MainStation/studydbmanager.h"
#include "../share/dicomscp.h"
#include "imageeditdialog.h"
#include "initcrdetectorthread.h"
#include "carerayacquisitthread.h"
#include "../MainStation/sendstudydialog.h"
#include "../MainStation/logdbmanager.h"

#include "direct.h"
#include "structures.h"
#include "errors.h"
#include "crapi_dll.h"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcuid.h"

#include <QSerialPort>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QSettings>

ImageAcquisitWidget::ImageAcquisitWidget(QWidget *parent) :
    imgFormat(new DcmFileFormat),
    serialPort(new QSerialPort(this)),
    timer(new QTimer(this)),
    crAcqThread(new CareRayAcquisitThread(this)),
    frameBuffer(0),
    isDllLoaded(0),
    QWidget(parent),
    ui(new Ui::ImageAcquisitWidget)
{
    ui->setupUi(this);
    init();
}

ImageAcquisitWidget::~ImageAcquisitWidget()
{
    QSettings s;
    s.setValue(IMAGE_CCW_ROTATE, ui->ccw90Check->isChecked());
    s.setValue(IMAGE_CW_ROTATE, ui->cw90Check->isChecked());
    s.setValue(IMAGE_HORI_FLIP, ui->hflipCheck->isChecked());
    s.setValue(IMAGE_VERT_FLIP, ui->vflipCheck->isChecked());

    freeDetector();
    serialPort->close();
    delete imgFormat;
    delete[] frameBuffer;
    qDeleteAll(storeScpList);
    delete ui;
}

void ImageAcquisitWidget::onStartAcq(const StudyRecord &study)
{
    if (study.studyUid != curStudyUid) {
        curStudyUid = study.studyUid;
        ui->patientIdEdit->setText(study.patientId);
        ui->patientNameEdit->setText(study.patientName);
        ui->patientSexEdit->setText(sex2trSex(study.patientSex));
        ui->patientBirthEdit->setText(study.patientBirth.toString(Qt::SystemLocaleDate));

        StudyDbManager::insertStudyToDb(study);
        thumbnailWidget->clear();
        ui->seriesNoSpin->setValue(0);

        insertStudyToDataset(study);
    }
}

void ImageAcquisitWidget::initDetector()
{
    int ret;
    switch (detInfo.model) {
    case DM_CareRay_1500L:
        initDetector_CareRay();
        timer->start(1000);
        break;
    case DM_Samsung_1717SGC:
        ret = VADAV_MapDLL(_T(VADAV_DLL_PATH), &mc_DavinciIntF);
        if (!ret) {
            QString errStr = tr("load VADAV.dll failed.");
            setDetectorStatus(errStr);
            LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
        } else isDllLoaded = true;
        break;
    }
}

void ImageAcquisitWidget::freeDetector()
{
    int ret;
    switch (detInfo.model) {
    case DM_CareRay_1500L:
        if (crAcqThread->isRunning()) {
            crAcqThread->setAbort(true);
            crAcqThread->wait();
        }
        ret = CR_disconnect_detector();
        if (CR_NO_ERR != ret) {
            QString errStr = tr("Disconnect detector failed: %1.").arg(CrErrStrList(ret));
            setDetectorStatus(errStr);
            LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
        }
        timer->stop();
        break;
    case DM_Samsung_1717SGC:
        if (ui->acquisitButton->isChecked()) mc_DavinciIntF.rVDACQ_Abort(m_CBRAcq);
        if (isDllLoaded) VADAV_ReleaseDLL(&mc_DavinciIntF);
        break;
    }
}

void ImageAcquisitWidget::init()
{
    ui->acqBodyTypeCombo->addItems(ProcedureItem::BodyTypeStringTable);
    ui->acqBodyPosCombo->addItems(ProcedureItem::BodyPositionStringTable);
    ui->seriesDescCombo->addItems(QSettings().value(SERIES_DESCRIPTION).toStringList());

    QSettings s;
    ui->ccw90Check->setChecked(s.value(IMAGE_CCW_ROTATE, 0).toBool());
    ui->cw90Check->setChecked(s.value(IMAGE_CW_ROTATE, 0).toBool());
    ui->hflipCheck->setChecked(s.value(IMAGE_HORI_FLIP, 0).toBool());
    ui->vflipCheck->setChecked(s.value(IMAGE_VERT_FLIP, 0).toBool());

    createComponents();
    createConnections();
    initDcmFileFormat();
    updateDetGenModels();
    updateExposureParams();
}

void ImageAcquisitWidget::createComponents()
{
    bodyView = new BodyProtoGraphicsView;
    ui->bodyVLayout->insertWidget(0, bodyView);

    thumbnailWidget = new ThumbnailBarWidget;
    thumbnailWidget->setBarDirection(QBoxLayout::LeftToRight);
    ui->acqThumbnailScrollArea->setWidget(thumbnailWidget);
}

void ImageAcquisitWidget::onImageDoubleClicked(const QString &imageFile)
{
    QString db = mainWindow->getDbLocation();
    QString file = imageFile.mid(db.size());
    emit imageDoubleClicked(QStringList()<<file);
}

void ImageAcquisitWidget::onSerialPortOpen()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        ui->serialOpenButton->setText(tr("Open"));
        ui->acqSerialPortLabel->setText(tr("%1: Closed").arg(genInfo.serialPortName));
    } else {
        serialPort->setPortName(genInfo.serialPortName);
        if (!serialPort->open(QIODevice::ReadWrite)) {
            ui->acqSerialPortLabel->setText(tr("%1: Open failed").arg(genInfo.serialPortName));
        } else {
            serialPort->setBaudRate((QSerialPort::BaudRate)genInfo.baudRate);
            serialPort->setDataBits((QSerialPort::DataBits)genInfo.dataBits);
            serialPort->setParity((QSerialPort::Parity)genInfo.parity);
            serialPort->setStopBits((QSerialPort::StopBits)genInfo.stopBits);
            serialPort->setFlowControl((QSerialPort::FlowControl)genInfo.flowControl);
            ui->acqSerialPortLabel->setText(tr("%1: Opened").arg(genInfo.serialPortName));
            ui->serialOpenButton->setText(tr("Close"));
        }
    }
}

void ImageAcquisitWidget::initDcmFileFormat()
{
    char uid[128];
    DcmMetaInfo *meta = imgFormat->getMetaInfo();
    meta->putAndInsertString(DCM_MediaStorageSOPClassUID, UID_DigitalXRayImageStorageForPresentation);
    meta->putAndInsertString(DCM_MediaStorageSOPInstanceUID, dcmGenerateUniqueIdentifier(uid, SITE_UID_ROOT));
    if (gLocalByteOrder == EBO_BigEndian) {
        meta->putAndInsertString(DCM_TransferSyntaxUID, UID_BigEndianExplicitTransferSyntax);
    } else {
        meta->putAndInsertString(DCM_TransferSyntaxUID, UID_LittleEndianExplicitTransferSyntax);
    }

    DcmDataset *dset = imgFormat->getDataset();
    const ManufactureInfo &manuInfo = mainWindow->getManuInfo();
    const StationInfo &statInfo = mainWindow->getStationInfo();
    const InstitutionInfo &instInfo = mainWindow->getInstInfo();

    dset->putAndInsertString(DCM_SpecificCharacterSet, "ISO_IR 100");
    dset->putAndInsertString(DCM_ImageType, "ORIGINAL\\PRIMARY");
    dset->putAndInsertString(DCM_SOPClassUID, UID_DigitalXRayImageStorageForPresentation);
    dset->putAndInsertString(DCM_Modality, manuInfo.modality.toLatin1().data());
    dset->putAndInsertString(DCM_ConversionType, "DV");
    dset->putAndInsertString(DCM_Manufacturer, manuInfo.name.toLocal8Bit().data());
    dset->putAndInsertString(DCM_ManufacturerModelName, manuInfo.model.toLatin1().data());
    dset->putAndInsertString(DCM_StationName, statInfo.name.toLocal8Bit().data());
    dset->putAndInsertString(DCM_InstitutionName, instInfo.name.toLocal8Bit().data());
    dset->putAndInsertString(DCM_InstitutionAddress, instInfo.addr.toLocal8Bit().data());
    dset->putAndInsertString(DCM_InstitutionalDepartmentName, instInfo.tel.toLatin1().data());
<<<<<<< HEAD
}

void ImageAcquisitWidget::updateDetectorSpecificParams()
{
    DcmDataset *dset = imgFormat->getDataset();
    QString spacings = QString("%1\\%2").arg(detInfo.xSpacing, 0, 'f', 6).arg(detInfo.ySpacing, 0, 'f', 6);
    dset->putAndInsertString(DCM_ImagerPixelSpacing, spacings.toLatin1().data());
    dset->putAndInsertString(DCM_PixelSpacing, spacings.toLatin1().data());
    dset->putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME1");
    dset->putAndInsertUint16(DCM_BitsAllocated, detInfo.bitsAllocated);
    dset->putAndInsertUint16(DCM_BitsStored, detInfo.bitsStored);
    dset->putAndInsertUint16(DCM_HighBit, detInfo.highBit);
    dset->putAndInsertUint16(DCM_SamplesPerPixel, 1);
    dset->putAndInsertString(DCM_NumberOfFrames, "1");
=======
    QString spacings = QString("%1\\%2").arg(detInfo.xSpacing, 0, 'f', 6).arg(detInfo.ySpacing, 0, 'f', 6);
    dset->putAndInsertString(DCM_ImagerPixelSpacing, spacings.toLatin1().data());
    dset->putAndInsertString(DCM_PixelSpacing, spacings.toLatin1().data());
    dset->putAndInsertUint16(DCM_SamplesPerPixel, 1);
    dset->putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME1");
    dset->putAndInsertString(DCM_NumberOfFrames, "1");
    dset->putAndInsertUint16(DCM_BitsAllocated, detInfo.bitsAllocated);
    dset->putAndInsertUint16(DCM_BitsStored, detInfo.bitsStored);
    dset->putAndInsertUint16(DCM_HighBit, detInfo.highBit);
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
    dset->putAndInsertUint16(DCM_PixelRepresentation, 0);
    dset->putAndInsertString(DCM_RescaleIntercept, "0");
    dset->putAndInsertString(DCM_RescaleSlope, "1");
    dset->putAndInsertString(DCM_RescaleType, "US");
    dset->putAndInsertString(DCM_WindowCenterWidthExplanation, "NORMAL");
}

void ImageAcquisitWidget::updateDetGenModels()
{
    ui->detectorConnectLabel->setVisible(false);
    ui->temperatureLcd->setVisible(false);

    const CommunicationInfo &info = mainWindow->getCommInfo();
    if (detInfo.model != info.detModel) {
        freeDetector();
        detInfo.model = (DetectorModel)info.detModel;
        switch (detInfo.model) {
        case DM_Samsung_1717SGC:
            detInfo.xSpacing = 0.127;
            detInfo.ySpacing = 0.127;
            detInfo.bitsAllocated = 16;
            detInfo.bitsStored = 14;
            detInfo.highBit = 13;
            break;
        case DM_CareRay_1500L:
            detInfo.xSpacing = 0.152;
            detInfo.ySpacing = 0.152;
            detInfo.bitsAllocated = 16;
            detInfo.bitsStored = 16;
            detInfo.highBit = 15;
            ui->detectorConnectLabel->setVisible(true);
            ui->temperatureLcd->setVisible(true);
            break;
        default:
            detInfo.model = DM_NoDetector;
            detInfo.xSpacing = 0.150;
            detInfo.ySpacing = 0.150;
            detInfo.bitsAllocated = 16;
            detInfo.bitsStored = 14;
            detInfo.highBit = 13;
            break;
        }
        updateDetectorSpecificParams();
        initDetector();
    }

    if (genInfo.model != info.genModel) {
        genInfo.model = (GeneratorModel)info.genModel;
        genInfo.serialPortName = info.serialPortName;
        switch (genInfo.model) {
        case GM_YiJu:
            genInfo.baudRate = QSerialPort::Baud9600;
            genInfo.dataBits = QSerialPort::Data8;
            genInfo.flowControl = QSerialPort::NoFlowControl;
            genInfo.parity = QSerialPort::NoParity;
            genInfo.stopBits = QSerialPort::TwoStop;
            break;
        case GM_CPI:
            genInfo.baudRate = QSerialPort::Baud9600;
            genInfo.dataBits = QSerialPort::Data8;
            genInfo.flowControl = QSerialPort::NoFlowControl;
            genInfo.parity = QSerialPort::NoParity;
            genInfo.stopBits = QSerialPort::TwoStop;
            break;
        case GM_EMD:
            genInfo.baudRate = QSerialPort::Baud9600;
            genInfo.dataBits = QSerialPort::Data8;
            genInfo.flowControl = QSerialPort::NoFlowControl;
            genInfo.parity = QSerialPort::NoParity;
            genInfo.stopBits = QSerialPort::TwoStop;
            break;
        case GM_Sedecal:
            genInfo.baudRate = QSerialPort::Baud9600;
            genInfo.dataBits = QSerialPort::Data8;
            genInfo.flowControl = QSerialPort::NoFlowControl;
            genInfo.parity = QSerialPort::NoParity;
            genInfo.stopBits = QSerialPort::TwoStop;
            break;
        default:
            genInfo.model = GM_NoGenerator;
            genInfo.baudRate = QSerialPort::Baud9600;
            genInfo.dataBits = QSerialPort::Data8;
            genInfo.flowControl = QSerialPort::NoFlowControl;
            genInfo.parity = QSerialPort::NoParity;
            genInfo.stopBits = QSerialPort::TwoStop;
            break;
        }
        serialPort->close();
        onSerialPortOpen();
    }
}

void ImageAcquisitWidget::createConnections()
{
    connect(ui->acquisitButton, SIGNAL(clicked(bool)), SLOT(startCapture(bool)));
    connect(ui->endStudyButton, SIGNAL(clicked()), this, SLOT(onEndAcq()));

    connect(ui->serialOpenButton, SIGNAL(clicked()), this, SLOT(onSerialPortOpen()));
    connect(ui->acqMAmsCheck, SIGNAL(toggled(bool)), this, SLOT(updateEditorStatus(bool)));
    connect(ui->acqMAsCheck, SIGNAL(toggled(bool)), this, SLOT(updateEditorStatus(bool)));
    connect(ui->acqKvPlusButton, SIGNAL(clicked()), this, SLOT(onKvPlusButton()));
    connect(ui->acqKvMinusButton, SIGNAL(clicked()), this, SLOT(onKvMinusButton()));
    connect(ui->acqmAPlusButton, SIGNAL(clicked()), this, SLOT(onmAPlusButton()));
    connect(ui->acqmAMinusButton, SIGNAL(clicked()), this, SLOT(onmAMinusButton()));
    connect(ui->acqmSPlusButton, SIGNAL(clicked()), this, SLOT(onmSPlusButton()));
    connect(ui->acqmSMinusButton, SIGNAL(clicked()), this, SLOT(onmSMinusButton()));
    connect(ui->acqmAsPlusButton, SIGNAL(clicked()), this, SLOT(onMXPlusButton()));
    connect(ui->acqmAsMinusButton, SIGNAL(clicked()), this, SLOT(onMXMinusButton()));
    connect(ui->acqSaveButton, SIGNAL(clicked()), this, SLOT(onAcqSave()));
    connect(ui->acqBodyTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateExposureParams()));
    connect(ui->acqBodyPosCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateExposureParams()));
    connect(bodyView, SIGNAL(bodyPartChanged(int)), this, SLOT(updateExposureParams()));
    //connect(bodyView, SIGNAL(bodyPartChanged(int)), this, SLOT(onBodyPartChanged(int)));
    //connect(ui->acqBodyTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onBodyTypeChanged(int)));
    //connect(ui->acqBodyPosCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onBodyPosChanged(int)));

    connect(thumbnailWidget, SIGNAL(imageDoubleClicked(QString)), this, SLOT(onImageDoubleClicked(QString)));
    connect(crAcqThread, SIGNAL(exposureReady(bool)), this, SLOT(onExposureReady(bool)));
    connect(crAcqThread, SIGNAL(detectorMsg(QString)), this, SLOT(setDetectorStatus(QString)));
    connect(crAcqThread, SIGNAL(finished()), this, SLOT(acqFinished_CareRay()));
    connect(timer, SIGNAL(timeout()), this, SLOT(showStatus_CareRay()));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(onSerialPortReadReady()));
    connect(this, SIGNAL(acqusitionComplete()), this, SLOT(onAcquisitionComplete()));
    connect(this, SIGNAL(calibreationComplete()), this, SLOT(onCalibrationComplete()));
}

void ImageAcquisitWidget::insertStudyToDataset(const StudyRecord &study)
{
    DcmDataset *dset = imgFormat->getDataset();

    dset->putAndInsertString(DCM_StudyInstanceUID, study.studyUid.toLatin1().data());
    dset->putAndInsertString(DCM_StudyDate, study.studyTime.date().toString("yyyyMMdd").toLatin1().data());
    dset->putAndInsertString(DCM_StudyTime, study.studyTime.time().toString("hhmmss.zzz").toLatin1().data());
    dset->putAndInsertString(DCM_AccessionNumber, study.accNumber.toLatin1().data());
    dset->putAndInsertString(DCM_PatientID, study.patientId.toLocal8Bit().data());
    dset->putAndInsertString(DCM_PatientName, study.patientName.toLocal8Bit().data());
    dset->putAndInsertString(DCM_PatientBirthDate, study.patientBirth.toString("yyyyMMdd").toLatin1().data());
    dset->putAndInsertString(DCM_PatientSex, study.patientSex.toLatin1().data());
    dset->putAndInsertString(DCM_PerformingPhysicianName, study.perPhysician.toLocal8Bit().data());
    dset->putAndInsertString(DCM_RequestingPhysician, study.reqPhysician.toLocal8Bit().data());
    int ymwd = study.studyTime.date().year() - study.patientBirth.year();
    if (ymwd > 0) {
        dset->putAndInsertString(DCM_PatientAge, QString("%1Y").arg(ymwd).toLatin1().data());
    } else if ((ymwd = study.studyTime.date().month()-study.patientBirth.month()) > 0) {
        dset->putAndInsertString(DCM_PatientAge, QString("%1M").arg(ymwd).toLatin1().data());
    } else if ((ymwd = study.studyTime.date().weekNumber()-study.patientBirth.weekNumber()) > 0) {
        dset->putAndInsertString(DCM_PatientAge, QString("%1W").arg(ymwd).toLatin1().data());
    } else {
        ymwd = study.studyTime.date().day()-study.patientBirth.day();
        dset->putAndInsertString(DCM_PatientAge, QString("%1D").arg(ymwd).toLatin1().data());
    }

    thumbnailWidget->addStudy(study.studyUid);
}

void ImageAcquisitWidget::updateExposureParams()
{
    ProcedureItem::BodyPart part = bodyView->currentBodyPart();
    ProcedureItem::BodyType type = (ProcedureItem::BodyType)ui->acqBodyTypeCombo->currentIndex();
    ProcedureItem::BodyPosition pos = (ProcedureItem::BodyPosition)ui->acqBodyPosCombo->currentIndex();
    QString procId;
    double kvp=0, ma=0, ms=0;
    ProcedureItemModel *procModel = mainWindow->getProcModel();
    if (procModel->getExposureParams(part, type, pos, kvp, ma, ms, procId)) {

        if (serialPort->isOpen()) {
            Sleep(50);
            int bytes;
            QString temp = QString("KV %1\r").arg(kvp);
            bytes = serialPort->write(temp.toLatin1().data());
            Sleep(50);
            temp = QString("MA %1\r").arg(ma);
            bytes = serialPort->write(temp.toLatin1());
            Sleep(50);
            if (ui->acqMAsCheck->isChecked()) {
                temp = QString("MX %1\r").arg(ma*ms/1000);
                bytes = serialPort->write(temp.toLatin1());
            } else if (ui->acqMAmsCheck->isChecked()) {
                temp = QString("MS %1\r").arg(ms, 0, 'g');
                bytes = serialPort->write(temp.toLatin1());
            }

        } else {
            ui->acqTubeVoltageSpin->setValue(kvp);
            ui->acqTubeCurrentSpin->setValue(ma);
            ui->acqExposureTimeSpin->setValue(ms);
            ui->acqPowerSpin->setValue(ma*ms*1000);
        }
    }
    ui->procIdEdit->setText(procId);
}

void ImageAcquisitWidget::onBodyPartChanged(int part)
{
    switch (part) {
    case ProcedureItem::BP_Head:
        serialPort->write("PAR 0\r");
        break;
    case ProcedureItem::BP_Neck:
        serialPort->write("PAR 0\r");
        break;
    case ProcedureItem::BP_Chest:
        serialPort->write("PAR 1\r");
        break;
    case ProcedureItem::BP_Lung:
        serialPort->write("PAR 1\r");
        break;
    case ProcedureItem::BP_Shoulder:
        serialPort->write("PAR 1\r");
        break;
    case ProcedureItem::BP_Waist:
        serialPort->write("PAR 3\r");
        break;
    case ProcedureItem::BP_Elbow:
        serialPort->write("PAR 5\r");
        break;
    case ProcedureItem::BP_Pelvis:
        serialPort->write("PAR 2\r");
        break;
    case ProcedureItem::BP_Hand:
        serialPort->write("PAR 5\r");
        break;
    case ProcedureItem::BP_Knee:
        serialPort->write("PAR 4\r");
        break;
    case ProcedureItem::BP_Foot:
        serialPort->write("PAR 4\r");
        break;
    }
}


void ImageAcquisitWidget::onBodyTypeChanged(int type)
{
    switch (type) {
    case ProcedureItem::BT_Normal:
        serialPort->write("FORM 1\r");
        break;
    case ProcedureItem::BT_Infant:
        serialPort->write("FORM 3\r");
        break;
    case ProcedureItem::BT_Slim:
        serialPort->write("FORM 2\r");
        break;
    case ProcedureItem::BT_Overweight:
        serialPort->write("FORM 0\r");
        break;
    }
}
void ImageAcquisitWidget::onBodyPosChanged(int pos)
{
    switch (pos) {
    case ProcedureItem::BP_PA:
        serialPort->write("POS 0\r");
        break;
    case ProcedureItem::BP_AP:
        serialPort->write("POS 0\r");
        break;
    case ProcedureItem::BP_LAT:
        serialPort->write("POS 1\r");
        break;
    case ProcedureItem::BP_PLAT:
        serialPort->write("POS 1\r");
        break;
    }
}

void ImageAcquisitWidget::updateEditorStatus(bool yes)
{
    if (yes) {
        if (ui->acqMAmsCheck->isChecked()) {
            ui->acqTubeVoltageSpin->setDisabled(false);
            ui->acqKvPlusButton->setDisabled(false);
            ui->acqKvMinusButton->setDisabled(false);
            ui->acqPowerSpin->setDisabled(true);
            ui->acqmAsPlusButton->setDisabled(true);
            ui->acqmAsMinusButton->setDisabled(true);
            ui->acqTubeCurrentSpin->setDisabled(false);
            ui->acqmAPlusButton->setDisabled(false);
            ui->acqmAMinusButton->setDisabled(false);
            ui->acqExposureTimeSpin->setDisabled(false);
            ui->acqmSPlusButton->setDisabled(false);
            ui->acqmSMinusButton->setDisabled(false);
            serialPort->write("ETMS\r");
        } else if (ui->acqMAsCheck->isChecked()) {
            ui->acqTubeVoltageSpin->setDisabled(false);
            ui->acqKvPlusButton->setDisabled(false);
            ui->acqKvMinusButton->setDisabled(false);
            ui->acqPowerSpin->setDisabled(false);
            ui->acqmAsPlusButton->setDisabled(false);
            ui->acqmAsMinusButton->setDisabled(false);
            ui->acqTubeCurrentSpin->setDisabled(false);
            ui->acqmAPlusButton->setDisabled(false);
            ui->acqmAMinusButton->setDisabled(false);
            ui->acqExposureTimeSpin->setDisabled(true);
            ui->acqmSPlusButton->setDisabled(true);
            ui->acqmSMinusButton->setDisabled(true);
            serialPort->write("ETMAS\r");
        } else if (ui->acqAecCheck->isChecked()) {
            ui->acqTubeVoltageSpin->setDisabled(true);
            ui->acqKvPlusButton->setDisabled(true);
            ui->acqKvMinusButton->setDisabled(true);
            ui->acqTubeCurrentSpin->setDisabled(true);
            ui->acqmAPlusButton->setDisabled(true);
            ui->acqmAMinusButton->setDisabled(true);
            ui->acqExposureTimeSpin->setDisabled(true);
            ui->acqmSPlusButton->setDisabled(true);
            ui->acqmSMinusButton->setDisabled(true);
            ui->acqPowerSpin->setDisabled(true);
            ui->acqmAsPlusButton->setDisabled(true);
            ui->acqmAsMinusButton->setDisabled(true);
        }
    }
}
void ImageAcquisitWidget::onAcqSave()
{
    ProcedureItem::BodyPart part = bodyView->currentBodyPart();
    ProcedureItem::BodyType type = (ProcedureItem::BodyType)ui->acqBodyTypeCombo->currentIndex();
    ProcedureItem::BodyPosition pos = (ProcedureItem::BodyPosition)ui->acqBodyPosCombo->currentIndex();
    ProcedureItemModel *procModel = mainWindow->getProcModel();
    procModel->setExposureParams(ui->procIdEdit->text(),
                                 part, type, pos,
                                 ui->acqTubeVoltageSpin->value(),
                                 ui->acqTubeCurrentSpin->value(),
                                 ui->acqExposureTimeSpin->value());
}

void ImageAcquisitWidget::onKvPlusButton()
{
    serialPort->write("KV+\r");
}

void ImageAcquisitWidget::onKvMinusButton()
{
    serialPort->write("KV-\r");
}

void ImageAcquisitWidget::onmSPlusButton()
{
    serialPort->write("MS+\r");
}

void ImageAcquisitWidget::onmSMinusButton()
{
    serialPort->write("MS-\r");
}

void ImageAcquisitWidget::onMXPlusButton()
{
    serialPort->write("MX++\r");
}

void ImageAcquisitWidget::onMXMinusButton()
{
    serialPort->write("MX--\r");
}

void ImageAcquisitWidget::onmAPlusButton()
{
    serialPort->write("MA+\r");
}

void ImageAcquisitWidget::onmAMinusButton()
{
    serialPort->write("MA-\r");
}

void ImageAcquisitWidget::onSerialPortReadReady()
{
    serialBuffer.append(serialPort->readAll());
    int crIdx;;

    while ((crIdx = serialBuffer.indexOf('\r')) >= 0) {
        QByteArray item = serialBuffer.left(crIdx);
        if (item == "ETMAS") {
            ui->acqMAsCheck->setChecked(true);
        } else if (item == "ETMS") {
            ui->acqMAmsCheck->setChecked(true);
        } else if (item.left(3) == "KV ") {
            ui->acqTubeVoltageSpin->setValue(item.remove(0, 3).toDouble());
        } else if (item.left(3) == "MA ") {
            ui->acqTubeCurrentSpin->setValue(item.remove(0, 3).toDouble());
        } else if (item.left(3) == "MX ") {
            double mAs = item.remove(0, 3).toDouble();
            ui->acqPowerSpin->setValue(mAs);
            if (ui->acqTubeCurrentSpin->value() > 0)
                ui->acqExposureTimeSpin->setValue((mAs*1000)/(ui->acqTubeCurrentSpin->value()));
        } else if (item.left(3) == "MS ") {
            double ms = item.remove(0, 3).toDouble();
            ui->acqExposureTimeSpin->setValue(ms);
            ui->acqPowerSpin->setValue((ms*ui->acqTubeCurrentSpin->value())/1000);
        }
        serialBuffer.remove(0, crIdx+1);
    }
}

void ImageAcquisitWidget::initDetector_CareRay()
{
    setDetectorStatus(tr("Initializing detector..."));
    InitCrDetectorThread *thread = new InitCrDetectorThread;
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(progressMsg(QString)), this, SLOT(setDetectorStatus(QString)));
    thread->start();
}

void ImageAcquisitWidget::startCapture(bool start)
{
    //dumyImageCapture();

    if (start) {
        switch (detInfo.model) {
        case DM_CareRay_1500L:
            crAcqThread->setAbort(false);
            crAcqThread->start();
            ui->acquisitButton->setChecked(true);
            ui->acquisitButton->setText(tr("Abort Capture"));
            break;
        case DM_Samsung_1717SGC:
            if (StartCapture_Davinci()) {
                ui->acquisitButton->setChecked(true);
                ui->acquisitButton->setText(tr("Abort Capture"));
            }
            break;
        }
    } else {
        switch (detInfo.model) {
        case DM_CareRay_1500L:
            crAcqThread->setAbort(true);
            break;
        case DM_Samsung_1717SGC:
            mc_DavinciIntF.rVDACQ_Abort(m_CBRAcq);
            ui->acquisitButton->setChecked(false);
            ui->acquisitButton->setText(tr("Start Capture"));
            delete[] frameBuffer;
            frameBuffer = 0;
            break;
        }
    }
}

void ImageAcquisitWidget::acqFinished_CareRay()
{
    ui->acquisitButton->setChecked(false);
    ui->acquisitButton->setText(tr("Start Capture"));

    if (crAcqThread->getImage(frameBuffer)) {
        crAcqThread->getImageSize(frameWidth, frameHeight);
        insertImageToDataset();
    }
}

void ImageAcquisitWidget::onExposureReady(bool yes)
{
    ui->exposureLabel->setPixmap(yes?QPixmap(":/images/exp_green.png"):QPixmap(":/images/exp_disable.png"));
}

void ImageAcquisitWidget::showStatus_CareRay()
{
    StatusInfo status;
    int ret = CR_get_status_info(&status);
    if (CR_NO_ERR != ret) {
        QPalette p = ui->detectorConnectLabel->palette();
        p.setColor(QPalette::WindowText, Qt::red);
        ui->detectorConnectLabel->setPalette(p);
        ui->detectorConnectLabel->setText(tr("Disconnected"));

        p = ui->temperatureLcd->palette();
        p.setColor(QPalette::WindowText, Qt::red);
        ui->temperatureLcd->setPalette(p);
        ui->temperatureLcd->display(-0.01);
    } else {
        QPalette p = ui->detectorConnectLabel->palette();
        p.setColor(QPalette::WindowText, Qt::green);

        switch (status.detectorState) {
        case CR_ERROR:
            p.setColor(QPalette::WindowText, Qt::red);
            ui->detectorConnectLabel->setText(tr("Detector Error"));
            break;
        default:
            ui->detectorConnectLabel->setText(tr("Connected"));
            break;
        }
        ui->detectorConnectLabel->setPalette(p);

        p = ui->temperatureLcd->palette();
        switch (status.temperature.overhot_flag) {
        case IN_NORMAL_TEMP:
            p.setColor(QPalette::WindowText, Qt::green);
            break;
        case IN_WARN_TEMP:
            p.setColor(QPalette::WindowText, Qt::yellow);
            break;
        default:
            p.setColor(QPalette::WindowText, Qt::red);
            break;
        }
        ui->detectorConnectLabel->setPalette(p);
        ui->temperatureLcd->display(status.temperature.aveTemperature);
    }
}

void ImageAcquisitWidget::onEndAcq()
{
    if (mainWindow->getWorkFlow().sendAfterAcqusition && acqImages.size()) {
        QString scpid = mainWindow->getWorkFlow().storeScpId;
        DicomScp *scp = 0;
        foreach (DicomScp *s, storeScpList) {
            if (s->id == scpid) {
                scp = s;
                break;
            }
        }
        if (scp) {
            SendStudyDialog dialog(this);
            dialog.setImageFiles(acqImages);
            dialog.setCloseOnFinish(true);
            dialog.startSend();
            dialog.exec();
        } else {
            QMessageBox::critical(this, tr("Send Image"),
                                  tr("Store SCP \"%1\" doesnot exist, send images failed.").arg(scpid));
            LogDbManager::insertMessageToDb(LogDbManager::ET_Error, tr("Store SCP \"%1\" doesnot exist, send images failed.").arg(scpid));
        }

    }

    curStudyUid.clear();
    thumbnailWidget->clear();
    ui->patientNameEdit->clear();
    ui->patientBirthEdit->clear();
    ui->patientIdEdit->clear();
    ui->patientSexEdit->clear();
    ui->detectorStatusBrowser->clear();
    acqImages.clear();
    emit acquisitEnd();
}

void ImageAcquisitWidget::onStoreScpChanged(const QList<DicomScp *> &scps)
{
    qDeleteAll(storeScpList);
    storeScpList.clear();
    foreach (DicomScp *scp, scps) {
        DicomScp *newScp = new DicomScp(*scp);
        storeScpList << newScp;
    }
}

void ImageAcquisitWidget::hideEvent(QHideEvent */*event*/)
{
    startCapture(false);
}

void ImageAcquisitWidget::showEvent(QShowEvent */*event*/)
{
    if (hasStudy())
        startCapture(true);
}

void ImageAcquisitWidget::dumyImageCapture()
{
    DcmFileFormat dcmff;
    dcmff.loadFile("test.dcm");
    ImageEditDialog dialog(this);
    int angle = 0, hori, vert;
    if (ui->ccw90Check->isChecked()) angle -= 90;
    if (ui->cw90Check->isChecked()) angle += 90;
    hori = ui->hflipCheck->isChecked()?1:0;
    vert = ui->vflipCheck->isChecked()?1:0;
    dialog.setImageTransform(angle, hori, vert);
    dialog.setFileFormat(&dcmff);
    if (QDialog::Accepted == dialog.exec()) {
        dcmff.saveFile("test.dcm", dcmff.getDataset()->getOriginalXfer());
    }
}

#include "atlstr.h"

static void __stdcall iCallBackProc_Acquisition( tVDACQ_CallBackRec* AR )
{
  CString strDebug = _T("");
  tCallBackRec *qCB = (tCallBackRec*)AR->rUserParam;
  switch (AR->rType) {
    case cVDACQ_ETErr:      // -- error in the caller's program code
    case cVDACQ_ETLastErr:  // -- error from Windows API
    case cVDACQ_ETWSAErr:   // -- error peculiarly from Windows Sockets
      AR->rUserCallBackProc = NULL;         // prevent future calls to this procedure
      qCB->rComplete = -1;                  // going to stop because of error
      qCB->acquisitor->setDetectorStatus(QString(AR->rMsg));
      break;
    case cVDACQ_ETTrace:    // -- information message
    case cVDACQ_ETTraceT:   // -- information message with time-stamp in the text
      switch (AR->rEvent) {
        case cVDACQ_ECaptureRecv:           // - receive event
            if (qCB->rNumReceivedRows!=AR->rCaptureRows) {
              qCB->rNumReceivedRows = AR->rCaptureRows;
              if (!(qCB->rNumReceivedRows%100)) {
                qCB->acquisitor->setDetectorStatus(QObject::tr("Received %1 rows").arg(AR->rCaptureRows));
              }
            }
          break;
        // Acquisition 'percentage event' does not mean real 'completeness'. It is message
        // from detector (socket #2). Theoretically it may be never sent though acquisition
        // is quite success. I suggest ignore acquisition 'percentage event'. But there is
        // important exception: event with 'RecvPercent=1000' is from the library and
        // indicates that acquisition is successful and complete
        case cVDACQ_ECapturePerc:           // - percentage event
          if (AR->rCapturePercent>100) {    // normal complete
            //m_AcquisitionProgress.SetPos(100);
            AR->rUserCallBackProc = NULL;   // prevent future calls to this procedure
            qCB->rComplete = 1;             // going to stop because of normal complete
            qCB->acquisitor->setDetectorStatus(QString::fromLocal8Bit(AR->rMsg));
            qCB->acquisitor->completeAcquisition();

            //qCB->rhWnd->PostMessage(WM_USER_DAVINCI_ACQ_COMPLETE, qCB->rCalMode, 0);
          }
          break;
        // Any other event
        default:
          if (AR->rMsg[0])  // print out message text
          {
              strDebug.Format("\n%s%s", "--> iCallBackProc_Acq : ", AR->rMsg);
              OutputDebugString(strDebug);
          }
                //OutputDebugString(AR->rMsg );
      } // switch rEvent
      break;
  } // switch rType
}
static void __stdcall iCallBackProc_Calibration( tVDC_CallBackRec* AR )
{
  tCallBackRec *qCB = (tCallBackRec*)AR->rUserParam;
  switch (AR->rType) {
    // Error message.
    // Do not care about particular error's event; just go stop.
    case cVDC_ETErr:      // -- error in the caller's program code
      AR->rUserCallBackProc = NULL;       // prevent future calls to this procedure
      qCB->rComplete = -1;                // going to stop because of error
      //qCB->rhWnd->PostMessage(WM_USER_DAVINCI_CAL_ERROR, qCB->rCalMode, AR->rType);
      OutputDebugString(AR->rMsg );                // print out message text
      break;
    // General message.
    case cVDC_ETTrace:    // -- information message
    case cVDC_ETTraceT:   // -- information message with time-stamp in the text
      switch (AR->rEvent) {
        case cVDC_ECalibPerc:             // - percentage event
          if (AR->rCalibPercent>100) { // normal complete
            //m_PostProcessingProgress.SetPos(100);
            AR->rUserCallBackProc = NULL; // prevent future calls to this procedure
            OutputDebugString(AR->rMsg );
            qCB->rComplete = 1;           // going to stop because of normal complete
            qCB->acquisitor->setDetectorStatus("Calibration done.");
            qCB->acquisitor->completeCalibration();

            //qCB->rhWnd->PostMessage(WM_USER_DAVINCI_CAL_COMPLETE, qCB->rCalMode, 0);
          }
          else {
            qCB->acquisitor->setDetectorStatus(QObject::tr("Calibration %1%% completed.").arg(AR->rCalibPercent));
          };
          break;
        // Any other event
        default:
          if(AR->rMsg[0])                // print out message text
            OutputDebugString(AR->rMsg );
      }; // switch rEvent
      break;
  }; // switch rType
}

int ImageAcquisitWidget::StartCapture_Davinci()
{

    /************************************************************************/
    /* Capture Sequence                                                     */
    /************************************************************************/
        // get frame dimension

   mc_DavinciIntF.rVDACQ_GetFrameDim( &frameWidth, &frameHeight );

//    OutputDebugString("\nCall rVDC_GetImageDim()");

    //mc_DavinciIntF.rVDC_GetImageDim( &mi_DavinciImgWidth, &mi_DavinciImgHeight );

    delete [] frameBuffer;
    //if (msh_ImgBuffer) delete [] msh_ImgBuffer;
    //msh_FrameBuffer = NULL; //msh_ImgBuffer = NULL;
    frameBuffer = new short[frameWidth*frameHeight];
    //msh_ImgBuffer = new short[mi_DavinciFrameWidth*mi_DavinciFrameHeight];

    m_CBUParam.rNumReceivedRows = -1;
    m_CBUParam.rComplete = 0;
    m_CBUParam.acquisitor = this;
    //m_CBUParam.rhWnd = this;
    m_CBUParam.rCalMode = cVDACQ_FBright;

    //Select AcquisitionMode ( 1 : Default Standard Mode )
    int iAcquisitionMode = 1;

    // connect to the detector
    m_CBRAcq = mc_DavinciIntF.rVDACQ_Connect(
        cVDACQ_FBright,             // wait for bbright frame
        iCallBackProc_Acquisition,  // supplied callback procedure
        &m_CBUParam,                       // custom information record
        frameBuffer,                 // allocated buffer
        iAcquisitionMode );                        // use default initializatin mode

    if (!m_CBRAcq)  // error message already printed by iCallBackProc_Acquisition
    {
        setDetectorStatus(tr("Cannot connect to detector!"));
        if (frameBuffer)
        {
            delete [] frameBuffer;
            frameBuffer = NULL;
        }

        return FALSE;
    }
    setDetectorStatus(tr("Connected to detector."));


    if (!mc_DavinciIntF.rVDACQ_StartFrame( m_CBRAcq ))
    {

        setDetectorStatus(tr("Start frame failed."));

        if (frameBuffer)
        {
            delete [] frameBuffer;
            frameBuffer = NULL;
        }

        if(m_CBRAcq)
        {
            mc_DavinciIntF.rVDACQ_Close( m_CBRAcq ); // it does nothing when qACQ_CBR=nil
            m_CBRAcq = NULL;
        }

        return FALSE;
    }

    setDetectorStatus(tr("Ready : X-ray Exposure"));
    ui->exposureLabel->setPixmap(QPixmap(":/exp_green.png"));

    return TRUE;

}
void ImageAcquisitWidget::onAcquisitionComplete()
{
    ui->acquisitButton->setChecked(false);
    ui->acquisitButton->setText(tr("Start Acquisit"));

    if(m_CBRAcq)
    {
        mc_DavinciIntF.rVDACQ_Close( m_CBRAcq ); // it does nothing when qACQ_CBR=nil
        m_CBRAcq = NULL;
    }

    BOOL AOfsCal, AGainCal, ABadPixMap, AHWOfsCal;
    int ACalOpt=0;

    mc_DavinciIntF.rVD_Get_Calibration(AOfsCal, AGainCal, ABadPixMap, AHWOfsCal);

    if(AOfsCal)
        ACalOpt |= cVDC_FCalOffs;
    if(AGainCal)
        ACalOpt |= cVDC_FCalGain;
    if(ABadPixMap)
        ACalOpt |= cVDC_FBadPixMap;
    if(AHWOfsCal)
        ACalOpt |= cVDC_FChkHWDark;

    // Pre-Processing
    m_CBRCal = mc_DavinciIntF.rVDC_Process(
                ACalOpt,			                                       	 // apply calibration options
                iCallBackProc_Calibration,                                   // supplied callback procedure
                &m_CBUParam,                                                        // custom information record
                frameBuffer );
}

void ImageAcquisitWidget::onCalibrationComplete()
{
    if(m_CBRCal)
    {
        mc_DavinciIntF.rVDC_Close( m_CBRCal ); // it does nothing when qACQ_CBR=nil
        m_CBRCal = NULL;
    }

    insertImageToDataset();
}

void ImageAcquisitWidget::setDetectorStatus(const QString &msg)
{
    ui->detectorStatusBrowser->append(QString("%1: %2").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"), msg));
}

void ImageAcquisitWidget::insertImageToDataset()
{
        char uid[128];
        QString instUid;
        DcmDataset *dset = imgFormat->getDataset();
        int seriesNo = ui->seriesNoSpin->value();
        if (ui->seriesOptionCombo->currentIndex()) {
            seriesNo++;
            ui->seriesNoSpin->setValue(seriesNo);
            curSeriesUid = QString::fromLatin1(dcmGenerateUniqueIdentifier(uid, SITE_SERIES_UID_ROOT));
        }
        if (curSeriesUid.isEmpty()) {
            curSeriesUid = QString::fromLatin1(dcmGenerateUniqueIdentifier(uid, SITE_SERIES_UID_ROOT));
        }

        if (!ui->procIdEdit->text().isEmpty())
            dset->putAndInsertString(DCM_ProtocolName, ui->procIdEdit->text().toLocal8Bit().data());
        dset->putAndInsertString(DCM_SeriesInstanceUID, curSeriesUid.toLatin1().data());
        dset->putAndInsertString(DCM_SeriesDate, QDate::currentDate().toString("yyyyMMdd").toLatin1().data());
        dset->putAndInsertString(DCM_SeriesTime, QTime::currentTime().toString("hhmmss.zzz").toLatin1().data());

        dset->putAndInsertString(DCM_SeriesNumber, QString::number(seriesNo).toLatin1().data());
        dset->putAndInsertString(DCM_SOPInstanceUID, dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT));
        instUid = QString::fromLatin1(uid);
        dset->putAndInsertString(DCM_SOPClassUID, UID_DigitalXRayImageStorageForProcessing);
        dset->putAndInsertString(DCM_InstanceNumber, QString::number(thumbnailWidget->getImageList().size()).toLatin1().data());
        dset->putAndInsertString(DCM_AcquisitionDate, QDate::currentDate().toString("yyyyMMdd").toLatin1().data());
        dset->putAndInsertString(DCM_AcquisitionTime, QTime::currentTime().toString("hhmmss.zzz").toLatin1().data());
        dset->putAndInsertString(DCM_ContentDate, QDate::currentDate().toString("yyyyMMdd").toLatin1().data());
        dset->putAndInsertString(DCM_ContentTime, QTime::currentTime().toString("hhmmss.zzz").toLatin1().data());

        dset->putAndInsertString(DCM_BodyPartExamined, ProcedureItem::BodyPartStringTable.at(bodyView->currentBodyPart()).toLocal8Bit().data());
        dset->putAndInsertString(DCM_PatientPosition, ProcedureItem::BodyPositionStringTable.at(ui->acqBodyPosCombo->currentIndex()).toLocal8Bit().data());
        dset->putAndInsertString(DCM_KVP, QString::number(ui->acqTubeVoltageSpin->value(), 'f', 6).toLatin1().data());
        dset->putAndInsertString(DCM_XRayTubeCurrent, QString::number(ui->acqTubeCurrentSpin->value()).toLatin1().data());
        dset->putAndInsertString(DCM_ExposureTime, QString::number(int(ui->acqExposureTimeSpin->value())).toLatin1().data());
        dset->putAndInsertString(DCM_Exposure, QString::number(int(ui->acqPowerSpin->value())).toLatin1().data());
        dset->putAndInsertUint16(DCM_Rows, frameHeight);
        dset->putAndInsertUint16(DCM_Columns, frameWidth);

        dset->putAndInsertUint16Array(DCM_PixelData, (Uint16*)frameBuffer, frameWidth*frameHeight);


        ImageEditDialog dialog(this);
        int angle = 0, hori, vert;
        if (ui->ccw90Check->isChecked()) angle -= 90;
        if (ui->cw90Check->isChecked()) angle += 90;
        hori = ui->hflipCheck->isChecked()?1:0;
        vert = ui->vflipCheck->isChecked()?1:0;
        dialog.setImageTransform(angle, hori, vert);
        dialog.setFileFormat(imgFormat);
        if (QDialog::Accepted == dialog.exec()) {
            QString imageFile;
            savePrImageToDb(*imgFormat, imageFile);
            thumbnailWidget->appendImageFiles(QStringList()<<imageFile);
            acqImages << imageFile;

            dset->findAndDeleteSequenceItem(DCM_ReferencedImageSequence, 0);
            dset->putAndInsertString(DCM_SOPClassUID, UID_DigitalXRayImageStorageForProcessing);
            dset->putAndInsertString(DCM_SOPInstanceUID, instUid.toLatin1().data());
            dset->putAndInsertUint16(DCM_Rows, frameHeight);
            dset->putAndInsertUint16(DCM_Columns, frameWidth);
            dset->putAndInsertUint16Array(DCM_PixelData, (Uint16*)frameBuffer, frameWidth*frameHeight);
            QString rawFile = QString("%1/%2_%3.dcm").arg(imageFile.left(imageFile.lastIndexOf('/')),
                                                          QString(RAW_IMAGE_PREFIX),
                                                          instUid);
            imgFormat->saveFile(QString("%1/%2").arg(mainWindow->getDbLocation(), rawFile).toLocal8Bit().data(),
                                imgFormat->getMetaInfo()->getOriginalXfer());
        }

        delete[] frameBuffer;
        frameBuffer = 0;

        startCapture(true);
}

bool ImageAcquisitWidget::savePrImageToDb(DcmFileFormat &dcmff, QString &fileName)
{
    ImageRecord image;
    QDateTime studyTime;
    QString accNum;
    const char *value;
    DcmDataset *dset = dcmff.getDataset();
    dset->findAndGetString(DCM_StudyDate, value);
    studyTime.setDate(QDate::fromString(QString::fromLatin1(value) ,"yyyyMMdd"));
    dset->findAndGetString(DCM_StudyTime, value);
    studyTime.setTime(formatDicomTime(QString::fromLatin1(value)));
    dset->findAndGetString(DCM_AccessionNumber, value);
    accNum = QString::fromLocal8Bit(value);
    dset->findAndGetString(DCM_SOPClassUID, value);
    image.sopClassUid = QString::fromLatin1(value);
    dset->findAndGetString(DCM_SOPInstanceUID, value);
    image.imageUid = QString::fromLatin1(value);
    dset->findAndGetString(DCM_SeriesInstanceUID, value);
    image.seriesUid = QString::fromLatin1(value);
    dset->findAndGetString(DCM_StudyInstanceUID, value);
    image.studyUid = QString::fromLatin1(value);
    dset->findAndGetString(DCM_InstanceNumber, value);
    image.imageNo = QString::fromLatin1(value);
    dset->findAndGetString(DCM_BodyPartExamined, value);
    image.bodyPart = QString::fromLocal8Bit(value);
    dset->findAndGetString(DCM_ContentDate, value);
    image.imageTime.setDate(QDate::fromString(QString::fromLatin1(value), "yyyyMMdd"));
    dset->findAndGetString(DCM_ContentTime, value);
    image.imageTime.setTime(formatDicomTime(QString::fromLatin1(value)));
    dset->findAndGetString(DCM_ReferencedSOPInstanceUID, value, true);
    image.refImageUid = QString::fromLatin1(value);
    QString dirName = QString("%1/%2_%3").arg(studyTime.date().toString("yyyyMM"),
                                              studyTime.toString("yyyyMMddhhmmss"),
                                              accNum);
    QDir().mkpath(QString("%1/%2").arg(mainWindow->getDbLocation(), dirName));

    image.imageFile = QString("%1/%2_%3.dcm").arg(dirName, QString(PRESENT_IMAGE_PREFIX), image.imageUid);
    fileName = image.imageFile;
    OFCondition cond = dcmff.saveFile(QString("%1/%2").arg(mainWindow->getDbLocation(), image.imageFile).toLocal8Bit().data(), dcmff.getMetaInfo()->getOriginalXfer());
    if (cond.good()) {
        StudyDbManager::insertImageToDb(image);
    } else return false;

    return true;
}
