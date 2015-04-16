#ifndef IMAGEACQUISITWIDGET_H
#define IMAGEACQUISITWIDGET_H

#include <QWidget>
#include "../share/studyrecord.h"
#include <Windows.h>
#include "tchar.h"
#include "VADAVIntf.h"
#include "../share/global.h"
class ThumbnailBarWidget;
class QSerialPort;
class BodyProtoGraphicsView;
class DicomScp;
class DcmFileFormat;
class QTimer;
class CareRayAcquisitThread;

namespace Ui {
class ImageAcquisitWidget;
}

class ImageAcquisitWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageAcquisitWidget(QWidget *parent = 0);
    ~ImageAcquisitWidget();

    bool hasStudy() const { return !curStudyUid.isEmpty(); }
    void completeAcquisition() { emit acqusitionComplete(); }
    void completeCalibration() { emit calibreationComplete(); }

signals:
    void acqusitionComplete();
    void calibreationComplete();

    void imageDoubleClicked(const QStringList &images);
    void acquisitEnd();

public slots:
    void onStartAcq(const StudyRecord &study);

    void onImageDoubleClicked(const QString &imageFile);
    void setDetectorStatus(const QString &msg);

    void onAcquisitionComplete();
    void onCalibrationComplete();

    void updateDetGenModels();
    void onStoreScpChanged(const QList<DicomScp*> &scps);

protected slots:
    void onSerialPortOpen(bool yes);
    void onSerialPortReadReady();
    void onEndAcq();

    void updateExposureParams();
    void onBodyPartChanged(int part);
    void onBodyTypeChanged(int type);
    void onBodyPosChanged(int pos);
    void updateEditorStatus(bool yes);
    void onAcqSave();
    void onKvPlusButton();
    void onKvMinusButton();
    void onmSPlusButton();
    void onmSMinusButton();
    void onmAPlusButton();
    void onmAMinusButton();
    void onMXPlusButton();
    void onMXMinusButton();

    void dumyImageCapture();

    void startCapture(bool start);

    // CARERAY DETECTOR 1500L
    void initDetector_CareRay();
    void showStatus_CareRay();
    void acqFinished_CareRay();
    void onExposureReady(bool yes);

    // SAMSUM DETECTOR IMAGE CAPTURE
    int StartCapture_Davinci();

protected:
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);

private:
    Ui::ImageAcquisitWidget *ui;

    void init();
    void initDetector();
    void freeDetector();
    void createComponents();
    void createConnections();
    void initDcmFileFormat();
    void updateDetectorSpecificParams();
    void insertStudyToDataset(const StudyRecord &study);
    void insertImageToDataset();
    static bool savePrImageToDb(DcmFileFormat &dcmff, QString &fileName = QString());

    ThumbnailBarWidget *thumbnailWidget;
    BodyProtoGraphicsView *bodyView;
    QList<DicomScp*> storeScpList;
    QStringList acqImages;
    QString curStudyUid;
    QString curSeriesUid;

    QSerialPort *serialPort;
    DcmFileFormat *imgFormat;
    QByteArray serialBuffer;
    QTimer *timer;
    CareRayAcquisitThread *crAcqThread;

    short *frameBuffer;
    int frameWidth;
    int frameHeight;
    DetectorSpec detInfo;
    GeneratorSpec genInfo;

    // SAMSUM FLAT DETECTOR
    tVADAV_InterfaceRec mc_DavinciIntF;
    tCallBackRec m_CBUParam;
    tVDC_CallBackRec *m_CBRCal;			// callback for calibration
    tVDACQ_CallBackRec *m_CBRAcq;		// callback for acquisition (dark and bright)
    bool isDllLoaded;
};

#endif // IMAGEACQUISITWIDGET_H
