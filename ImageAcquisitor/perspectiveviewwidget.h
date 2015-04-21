#ifndef PERSPECTIVEVIEWWIDGET_H
#define PERSPECTIVEVIEWWIDGET_H

#include <QWidget>
class QAxWidget;
struct IDigitizer;
struct IImage;
struct IImageProcessing;
struct ISystem;
struct IDisplay;
struct IApplication;

#import "mil9/mil.ocx" no_namespace

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
    void generateLutRamps();
    void onZoom(double factor);

    // COM Event handlers
    void onProcessModifiedImage(int index);

private:
    void init();
    void setupConnections();

    Ui::PerspectiveViewWidget *ui;

    QAxWidget *axApp;
    QAxWidget *axDig;
    QAxWidget *axImgDest;
    QAxWidget *axImgPro;
    QAxWidget *axSys;
    QAxWidget *axDis;

    IApplication *pApp;
    IDigitizer *pDig;
    IImage *pImgDest;
    IImageProcessing *pImgPro;
    ISystem *pSys;
    IDisplay *pDis;

    QVector<IImagePtr> images;
};

#endif // PERSPECTIVEVIEWWIDGET_H
