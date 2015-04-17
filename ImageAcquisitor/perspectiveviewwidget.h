#ifndef PERSPECTIVEVIEWWIDGET_H
#define PERSPECTIVEVIEWWIDGET_H

#include <QWidget>
class QAxObject;
class QAxWidget;
struct IDigitizer;
struct IImage;
struct ISystem;
struct IDisplay;
struct IApplication;

namespace Ui {
class PerspectiveViewWidget;
}

class PerspectiveViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PerspectiveViewWidget(QWidget *parent = 0);
    ~PerspectiveViewWidget();

protected slots:
    void onStartView(bool start);
    void onGrabImage();
    void onEndView();

private:
    void init();
    void setupConnections();

    Ui::PerspectiveViewWidget *ui;
    QAxWidget *axApp;
    QAxWidget *axDig;
    QAxWidget *axImg;
    QAxWidget *axSys;
    QAxWidget *axDis;
    IApplication *pApp;
    IDigitizer *pDig;
    IImage *pImg;
    ISystem *pSys;
    IDisplay *pDis;
};

#endif // PERSPECTIVEVIEWWIDGET_H
