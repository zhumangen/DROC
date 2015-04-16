#ifndef PERSPECTIVEVIEWWIDGET_H
#define PERSPECTIVEVIEWWIDGET_H

#include <QWidget>
#include "Mil.h"

namespace Ui {
class PerspectiveViewWidget;
}

class PerspectiveViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PerspectiveViewWidget(QWidget *parent = 0);
    ~PerspectiveViewWidget();

private:
    void init();

    Ui::PerspectiveViewWidget *ui;

    MIL_ID MilApplication,  /* Application identifier.  */
           MilSystem,       /* System identifier.       */
           MilDisplay,      /* Display identifier.      */
           MilDigitizer,    /* Digitizer identifier.    */
           MilImage;        /* Image buffer identifier. */
};

#endif // PERSPECTIVEVIEWWIDGET_H
