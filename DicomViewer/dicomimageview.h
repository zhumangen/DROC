#ifndef DICOMIMAGEVIEW_H
#define DICOMIMAGEVIEW_H

#include <QGraphicsView>
class AbstractPathItem;
class ImageInstance;
class DicomImage;
class StudyRecord;

class DicomImageView : public QGraphicsView
{
    Q_OBJECT
public:
    enum CurrentState {
        None,
        Zoom,
        Pan,
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
    enum ViewMode {
        VM_Observe,
        VM_Print,
        VM_Report,
    };

    typedef struct tag_MouseButtonState {
        CurrentState state;
        DrawingType type;

        tag_MouseButtonState() { state = None; }
    } MouseButtonState;

    explicit DicomImageView(ImageInstance *imageInst = 0, QWidget *parent = 0);
    void setImageInstance(ImageInstance *imageInst);
    ImageInstance *const getImageInstance() const { return image; }
    void setHighlight(bool yes);
    void setViewMode(ViewMode viewMode);
    void setSizeHint(const QSize &size) { hintSize = size; }
    bool hasImage() { return image != 0; }
    DicomImage *getHardCopyGrayScaledImage();
    QImage getTextLayerImage(const QSize &size);
    QPixmap getHardCopyPixmap();
    bool hasRawImage() const;
    bool isRawImage() const { return rawType; }
    void setImageType(bool rawImage);
    void refreshViewPort() { refreshPixmap(); resizePixmapItem(); repositionAuxItems(); }
    bool getCurrentStudy(StudyRecord &study) const;

signals:
    void statusChanged(bool hasImage);
    void viewClicked(DicomImageView *view);
    void viewDoubleclicked(DicomImageView *view);

public slots:
    void inverseImage();
    void setRoiWindow();
    void setNone();
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
    void clear() { setImageInstance(0); }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *e);
    void leaveEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    QSize sizeHint() const { return hintSize; }
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

private:
    void updateScalors();
    void updateAnnotations();
    inline void refreshPixmap();
    void resizePixmapItem();
    void repositionAuxItems();
    void setTextItemVisible(bool yes);

    QGraphicsScene *scene;
    ImageInstance *image;

    QList<QGraphicsItem*> itemList;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsPathItem *xScalorItem;
    QGraphicsPathItem *yScalorItem;
    QGraphicsSimpleTextItem *posValue;
    QGraphicsSimpleTextItem *tlItem;
    QGraphicsSimpleTextItem *trItem;
    QGraphicsSimpleTextItem *blItem;
    QGraphicsSimpleTextItem *brItem;
    QGraphicsSimpleTextItem *windowItem;
    QGraphicsSimpleTextItem *zoomItem;

    MouseButtonState leftButton;
    AbstractPathItem *currentPathItem;
    QPoint prevMousePos;
    double factor;
    QSize hintSize;

    bool manualZoom;
    bool manualPan;
    int hflip;
    int vflip;
    int rotateAngle;

    ViewMode mode;
    bool rawType;
};

#endif // DICOMIMAGEVIEW_H
