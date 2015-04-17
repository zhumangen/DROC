#include "perspectiveviewwidget.h"
#include "ui_perspectiveviewwidget.h"

#include <QUuid>
#include <QAxObject>
#include <QAxWidget>
#include <QHBoxLayout>
#include <QMessageBox>

#import "mil9/MIL.OCX" no_namespace

PerspectiveViewWidget::PerspectiveViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PerspectiveViewWidget)
{
    ui->setupUi(this);
    init();
}

PerspectiveViewWidget::~PerspectiveViewWidget()
{
    if (pSys->IsAllocated) pSys->Free();
    if (pImg->IsAllocated) pImg->Free();
    if (pDig->IsAllocated) pDig->Free();
    if (pDis->IsAllocated) pDis->Free();
    delete ui;
}

void PerspectiveViewWidget::init()
{
    axApp = new QAxWidget("{E1208DE6-A783-11D0-9161-00A024D24992}", this);
    axSys = new QAxWidget("{6D9F7F74-9658-11D0-BDB5-00608CC9F9FB}", this);
    axImg = new QAxWidget("{03985968-6B33-11D0-AB4A-00608CC9CA57}", this);
    axDig = new QAxWidget("{B9475B93-8F25-11D0-914B-00A024D24992}", this);
    axDis = new QAxWidget("{E9502373-9624-11D0-BDB4-00608CC9F9FB}", this);

    axApp->queryInterface(__uuidof(IApplication), (void**)&pApp);
    axSys->queryInterface(__uuidof(ISystem), (void**)&pSys);
    axDig->queryInterface(__uuidof(IDigitizer), (void**)&pDig);
    axImg->queryInterface(__uuidof(IImage), (void**)&pImg);
    axDis->queryInterface(__uuidof(IDisplay), (void**)&pDis);

    pSys->AutomaticAllocation = false;
    pImg->AutomaticAllocation = false;
    pDig->AutomaticAllocation = false;
    pDis->AutomaticAllocation = false;

    pImg->OwnerSystem = pSys;
    pImg->CanGrab = true;
    pImg->CanProcess = true;
    pImg->CanDisplay = true;
    pDig->OwnerSystem = pSys;
    pDis->OwnerSystem = pSys;
    pDig->Image = pImg;
    pDis->Image = pImg;

    pSys->Allocate();
    pImg->Allocate();
    pDig->Allocate();
    pDis->Allocate();

    ui->horizontalLayout->insertWidget(0, axDis, 1);

    setupConnections();
}

void PerspectiveViewWidget::setupConnections()
{
    connect(ui->startViewButton, SIGNAL(clicked(bool)), this, SLOT(onStartView(bool)));
    connect(ui->endViewButton, SIGNAL(clicked()), this, SLOT(onEndView()));
    connect(ui->grabImageButton, SIGNAL(clicked()), this, SLOT(onGrabImage()));
}

void PerspectiveViewWidget::onStartView(bool start)
{
    if (start) {
        pDig->GrabContinuous();
        ui->startViewButton->setText(tr("Halt View"));
        ui->startViewButton->setChecked(true);
    } else {
        pDig->Halt();
        ui->startViewButton->setText(tr("Start View"));
        ui->startViewButton->setChecked(false);
    }
}

void PerspectiveViewWidget::onGrabImage()
{

}

void PerspectiveViewWidget::onEndView()
{

}
