#include "perspectiveviewwidget.h"
#include "ui_perspectiveviewwidget.h"


PerspectiveViewWidget::PerspectiveViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PerspectiveViewWidget)
{
    ui->setupUi(this);
    init();
}

PerspectiveViewWidget::~PerspectiveViewWidget()
{
    /* Free defaults. */
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);
    delete ui;
}

void PerspectiveViewWidget::init()
{
    /* Allocate defaults. */
       MappAllocDefault(M_SETUP, &MilApplication, &MilSystem,
                                 &MilDisplay, &MilDigitizer, &MilImage);

   /* Grab continuously. */
   MdigGrabContinuous(MilDigitizer, MilImage);
}
