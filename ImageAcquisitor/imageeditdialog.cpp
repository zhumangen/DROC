#include "imageeditdialog.h"
#include "ui_imageeditdialog.h"
#include "imageeditgraphicsview.h"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcuid.h"

ImageEditDialog::ImageEditDialog(DcmFileFormat &dcmff, QWidget *parent) :
    dcmFile(dcmff),
    QDialog(parent),
    ui(new Ui::ImageEditDialog)
{
    ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::WindowMaximizeButtonHint) &
                   (~Qt::WindowCloseButtonHint));
    setWindowState(Qt::WindowMaximized);
    init();
}

ImageEditDialog::~ImageEditDialog()
{
    delete ui;
}

void ImageEditDialog::init()
{
    imageEditView = new ImageEditGraphicsView;
    ui->horizontalLayout->insertWidget(0, imageEditView);
    imageEditView->setFileFormat(&dcmFile);

    setupConnections();
}

void ImageEditDialog::setupConnections()
{
    connect(ui->imageRFlagButton, SIGNAL(clicked()), imageEditView, SLOT(addRFlag()));
    connect(ui->imageLFlagButton, SIGNAL(clicked()), imageEditView, SLOT(addLFlag()));
    connect(ui->imageFlagFontButton, SIGNAL(clicked()), imageEditView, SLOT(onFlagFont()));
    connect(ui->imageUndoButton, SIGNAL(clicked()), imageEditView, SLOT(undoImage()));
    connect(ui->imageRedoButton, SIGNAL(clicked()), imageEditView, SLOT(redoImage()));
    connect(ui->imageCutButton, SIGNAL(clicked(bool)), imageEditView, SLOT(clipImage(bool)));
    connect(ui->imageNegativeButton, SIGNAL(clicked()), imageEditView, SLOT(inverseImage()));
    connect(ui->imageRotateCCWButton, SIGNAL(clicked()), imageEditView, SLOT(rotateCCW()));
    connect(ui->imageRotateCWButton, SIGNAL(clicked()), imageEditView, SLOT(rotateCW()));
    connect(ui->imageZoomButton, SIGNAL(clicked()), imageEditView, SLOT(zoomImage()));
    connect(ui->imageHFllipButton, SIGNAL(clicked()), imageEditView, SLOT(hFlip()));
    connect(ui->imageVFlipButton, SIGNAL(clicked()), imageEditView, SLOT(vFlip()));
    connect(ui->imagePanButton, SIGNAL(clicked()), imageEditView, SLOT(panImage()));
    connect(ui->imageRulerButton, SIGNAL(clicked()), imageEditView, SLOT(drawLenght()));
    connect(ui->imageAngleButton, SIGNAL(clicked()), imageEditView, SLOT(drawAngle()));
    connect(ui->imageCobbAngleButton, SIGNAL(clicked()), imageEditView, SLOT(drawCobbAngle()));
    connect(ui->imageEllipseButton, SIGNAL(clicked()), imageEditView, SLOT(drawEllipse()));
    connect(ui->imageRectangleButton, SIGNAL(clicked()), imageEditView, SLOT(drawRectangle()));
    connect(ui->imageAnnotationButton, SIGNAL(clicked()), imageEditView, SLOT(drawText()));
    connect(ui->imageArrowButton, SIGNAL(clicked()), imageEditView, SLOT(drawArrow()));
    //connect(ui->imagePencilButton, SIGNAL(clicked()), imageEditView, SLOT(drawCurve()));
    connect(ui->imageItemDeleteButton, SIGNAL(clicked()), imageEditView, SLOT(removeCurrent()));
    connect(ui->imageResetButton, SIGNAL(clicked()), imageEditView, SLOT(reset()));

    connect(ui->imageOkButton, SIGNAL(clicked()), this, SLOT(onImageOk()));
    connect(ui->imageCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

}

void ImageEditDialog::onImageOk()
{
    // do something...
    imageEditView->saveImage();

    DcmDataset *dset = dcmFile.getDataset();
    DcmItem *ditem;
    dset->findAndGetSequenceItem(DCM_ReferencedImageSequence, ditem);
    if (!ditem) {
        const char *sopInstUid;
        const char *sopClassUid;
        char uid[128];
        dset->findAndGetString(DCM_SOPInstanceUID, sopInstUid);
        dset->findAndGetString(DCM_SOPClassUID, sopClassUid);
        dset->findOrCreateSequenceItem(DCM_ReferencedImageSequence, ditem);
        if (ditem) {
            ditem->putAndInsertString(DCM_ReferencedSOPClassUID, sopClassUid);
            ditem->putAndInsertString(DCM_ReferencedSOPInstanceUID, sopInstUid);
        }
        dset->putAndInsertString(DCM_SOPInstanceUID, dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT));
        dset->putAndInsertString(DCM_SOPClassUID, UID_DigitalXRayImageStorageForPresentation);
    }

    accept();
}
