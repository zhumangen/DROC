#include "perspectiveviewwidget.h"
#include "ui_perspectiveviewwidget.h"
#include "ActiveMILUtils.h"

#include <QUuid>
#include <QAxObject>
#include <QAxWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QResizeEvent>

#import "mil9/MILImageProcessing.ocx" no_namespace

#define BUFFERING_SIZE_MAX 20

PerspectiveViewWidget::PerspectiveViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PerspectiveViewWidget)
{
    ui->setupUi(this);
    init();
}

PerspectiveViewWidget::~PerspectiveViewWidget()
{
    if (ui->startViewButton->isChecked())
        pDig->MultipleBuffering->Halt(false);

    delete ui;
    delete axDig;
    delete axImgPro;
    delete axImgDest;
    delete axSys;
    delete axApp;
}

void PerspectiveViewWidget::init()
{
    axApp = new QAxWidget("{E1208DE6-A783-11D0-9161-00A024D24992}");
    axSys = new QAxWidget("{6D9F7F74-9658-11D0-BDB5-00608CC9F9FB}");
    axImgDest = new QAxWidget("{03985968-6B33-11D0-AB4A-00608CC9CA57}");
    axImgPro = new QAxWidget("{4FD68BB6-A7E7-11D1-9F0D-00608CC9F9FB}");
    axDig = new QAxWidget("{B9475B93-8F25-11D0-914B-00A024D24992}");
    axDis = new QAxWidget("{E9502373-9624-11D0-BDB4-00608CC9F9FB}");

    axApp->queryInterface(__uuidof(IApplication), (void**)&pApp);
    axSys->queryInterface(__uuidof(ISystem), (void**)&pSys);
    axDig->queryInterface(__uuidof(IDigitizer), (void**)&pDig);
    axImgDest->queryInterface(__uuidof(IImage), (void**)&pImgDest);
    axImgPro->queryInterface(__uuidof(IImageProcessing), (void**)&pImgPro);
    axDis->queryInterface(__uuidof(IDisplay), (void**)&pDis);

    pImgDest->OwnerSystem = pSys;
    pImgPro->OwnerSystem = pSys;
    pDig->OwnerSystem = pSys;
    pDis->OwnerSystem = pSys;
    pImgDest->CanGrab = false;
    pImgDest->CanProcess = true;
    pImgDest->CanDisplay = true;
    pImgPro->Destination1 = pImgDest;
    pDis->Image = pImgDest;

    pSys->Allocate();
    pImgPro->Allocate();
    pDig->Allocate();

    pImgDest->NumberOfBands = 1;
    pImgDest->SizeX = (long)(pDig->SizeX);
    pImgDest->SizeY = (long)(pDig->SizeY);
    pImgDest->Allocate();
    pDis->Allocate();

    int depth = pDig->DataDepth;
    int imageMax = (1<<depth)-1;
    pDis->LUT->GenerateRamp(0, 0, imageMax, 255);
    pDis->LUTEnabled = true;
    ui->lowMaxLabel->setText(QString::number(imageMax));
    ui->lowSlider->setValue(0);
    ui->highMaxLabel->setText(QString::number(imageMax));
    ui->highSlider->setValue(imageMax);
    ui->maxSlider->setValue(255);

    BOOL ContinueAllocating = TRUE;
    while(ContinueAllocating) {
        IImagePtr NewImage = pApp->CreateObject("MIL.Image", FALSE);
        if(NewImage != NULL) {
            NewImage->CanGrab = TRUE;
            NewImage->CanDisplay = FALSE;
            NewImage->CanProcess = TRUE;
            NewImage->NumberOfBands = 1;
            NewImage->SizeX = (long)(pDig->SizeX/* * pDig->ScaleX*/);
            NewImage->SizeY = (long)(pDig->SizeY/* * pDig->ScaleY*/);

            NewImage->Allocate();

            // Add the image to the array of images
            //
            images.append(NewImage);

            // Increment the maximum of procssing images
            //

            if(BUFFERING_SIZE_MAX == images.size()) {
                ContinueAllocating = FALSE;
            }
        }
    }

    ui->horizontalLayout->insertWidget(0, axDis, 1);

    setupConnections();
}

void PerspectiveViewWidget::setupConnections()
{
    connect(ui->startViewButton, SIGNAL(clicked(bool)), this, SLOT(onStartView(bool)));
    connect(ui->endViewButton, SIGNAL(clicked()), this, SLOT(onEndView()));
    connect(ui->grabImageButton, SIGNAL(clicked()), this, SLOT(onGrabImage()));
    connect(ui->maxSlider, SIGNAL(valueChanged(int)), this, SLOT(generateLutRamps()));
    connect(ui->lowSlider, SIGNAL(valueChanged(int)), this, SLOT(generateLutRamps()));
    connect(ui->highSlider, SIGNAL(valueChanged(int)), this, SLOT(generateLutRamps()));

    connect(axDig, SIGNAL(ProcessModifiedImage(int)), this, SLOT(onProcessModifiedImage(int)));
}

void PerspectiveViewWidget::onProcessModifiedImage(int index)
{
    pImgPro->Source1 = _variant_t(images[index], true);
    pImgPro->Smooth(impOverscanFast);
    //pImgPro->Not();
}

void PerspectiveViewWidget::onStartView(bool start)
{
    if (start) {
        // Build a CActiveMILObjectsArray
        //
        CActiveMILObjectsArray<IImagePtr> ActiveMILObjectsArray((void**)images.data(), images.size());

        // Start the processing.  The processing event is fired for every frame grabbed
        //
        pDig->MultipleBuffering->Process(digStart, ActiveMILObjectsArray);

        QSize size = axDis->size();
        int width = pDig->SizeX;
        int height = pDig->SizeY;
        pDis->PanX = (width-size.width())/2;
        pDis->PanY = (height-size.height())/2;

        ui->startViewButton->setText(tr("Halt View"));
        ui->startViewButton->setChecked(true);
    } else {
        pDig->MultipleBuffering->Halt(false);
        ui->startViewButton->setText(tr("Start View"));
        ui->startViewButton->setChecked(false);
    }
}

void PerspectiveViewWidget::generateLutRamps()
{
    pDis->UpdateEnabled = false;

    int imageMax = (1<<(pDig->DataDepth))-1;
    int low = ui->lowSlider->value();
    int high = ui->highSlider->value();
    if (low > high) {
        if (sender() == ui->lowSlider) {
            ui->highSlider->setValue(low);
            high = low;
        } else {
            ui->lowSlider->setValue(high);
            low = high;
        }
    }
    int max = ui->maxSlider->value();

    pDis->LUT->GenerateRamp(0, 0, low, 0);
    pDis->LUT->GenerateRamp(low, 0, high, max);
    pDis->LUT->GenerateRamp(high, max, imageMax, 255);

    pDis->UpdateEnabled = true;
}

void PerspectiveViewWidget::onZoom(double factor)
{
    pDis->Zoom(factor, factor, false);
}

void PerspectiveViewWidget::onGrabImage()
{

}

void PerspectiveViewWidget::onEndView()
{

}
