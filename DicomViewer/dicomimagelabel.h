#ifndef DICOMIMAGELABEL_H
#define DICOMIMAGELABEL_H

#include <QLabel>
class ImageInstance;
class InstanceItem;

#define IMAGE_LABEL_SIZE 120

class DicomImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit DicomImageLabel(ImageInstance *image, QWidget *parent = 0);
    ~DicomImageLabel();

    QSize sizeHint() const { return QSize(IMAGE_LABEL_SIZE, IMAGE_LABEL_SIZE); }
    static int getImageLabelSize() { return IMAGE_LABEL_SIZE; }
    ImageInstance *getImageInstance() const { return imageInst; }

signals:
    void imageClicked(DicomImageLabel *label);
    void imageDoubleClicked(DicomImageLabel *label);

public slots:
    void setHighlight(bool yes);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    void init();
    ImageInstance *imageInst;

    QPoint dragOrg;
};

#endif // DICOMIMAGELABEL_H
