#ifndef IMAGEEDITGRAPHICSVIEW_H
#define IMAGEEDITGRAPHICSVIEW_H

#include <QGraphicsView>
#include "dcmtk/dcmimgle/diutils.h"
class DicomImage;
class AbstractPathItem;
class DcmFileFormat;
class GraphicsClipRectItem;

class ImageEditGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    enum CurrentState {
        None,
        Zoom,
        Pan,
        AddFlag,
        AdjustWindow,
        ROIWindow,
        Drawing,
    };

    enum DrawingType {
        DrawLine,
        DrawAngle,
        DrawCobbAngle,
        DrawRect,
        DrawEllipse,
        DrawPolygon,
        DrawClosedCurve,
        DrawTextMark,
        DrawArrow,
        DrawCurve,
    };
    typedef struct tag_MouseButtonState {
        CurrentState state;
        DrawingType type;

        tag_MouseButtonState() { state = None; }
    } MouseButtonState;

    explicit ImageEditGraphicsView(QWidget *parent = 0);
    ~ImageEditGraphicsView();
    bool setFileFormat(DcmFileFormat *dff, bool process = true);
<<<<<<< HEAD
    void setImageTransform(int angle, int hori, int vert) { rotateAngle = angle; hflip = hori; vflip = vert; }
=======
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
    void setPixelSpacing(double x, double y) { xSpacing = x; ySpacing = y; }
    void getWindow(double &center, double &width) { center = winCenter; width = winWidth; }

    static bool dcm2pixmap(QPixmap &pixmap, DicomImage *image, double center = -1.0, double width = -1.0);

signals:

public slots:
    void addRFlag();
    void addLFlag();
    void onFlagFont();
    void clipImage(bool checked);
    void undoImage();
    void redoImage();
    void inverseImage();
    void rotateCCW();
    void rotateCW();
    void zoomImage();
    void hFlip();
    void vFlip();
    void panImage();
    void drawLenght();
    void drawAngle();
    void drawCobbAngle();
    void drawEllipse();
    void drawRectangle();
    void drawArrow();
    void drawText();
    void removeCurrent();
    void reset();

    void saveImage();
    void clear();

    void onProcessingFinished();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
<<<<<<< HEAD
    void mouseDoubleClickEvent(QMouseEvent *event);
=======
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
    void wheelEvent(QWheelEvent *event);
    void leaveEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    void updateScalors();
    QPixmap getPixmap();
    void resizePixmapItem();
    void repositionAuxItems();
    uint getPixelValue(long x, long y, EP_Representation &r) const;

    QGraphicsScene *scene;

    QList<QGraphicsSimpleTextItem*> flagItemList;
    QList<QGraphicsItem*> itemList;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsPathItem *xScalorItem;
    QGraphicsPathItem *yScalorItem;
    QGraphicsSimpleTextItem *posValue;
    QGraphicsSimpleTextItem *windowItem;
    QGraphicsSimpleTextItem *zoomItem;
    GraphicsClipRectItem *clipRectItem;

    const Uint16 *srcBuffer;
    Uint16 *destBuffer;
    ulong imageSize;

    DcmFileFormat *ff;
    QList<DicomImage*> imageList;
    int imageIndex;
    double xSpacing;
    double ySpacing;
    MouseButtonState leftButton;
    AbstractPathItem *currentPathItem;
    QPoint prevMousePos;
    double factor;

    bool manualZoom;
    bool manualPan;
    int hflip;
    int vflip;
    int rotateAngle;
    double winWidth;
    double winCenter;
    QString procId;
    QRectF clipRectFirst;
};

#endif // IMAGEEDITGRAPHICSVIEW_H
