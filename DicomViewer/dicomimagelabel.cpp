#include "dicomimagelabel.h"
#include "imageinstance.h"

#include <QFrame>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>

DicomImageLabel::DicomImageLabel(ImageInstance *image, QWidget *parent) :
    imageInst(image),
    QLabel(parent)
{
    setStyleSheet(QString::fromLatin1("background-color: rgb(0, 0, 0);"));
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Plain);
    setLineWidth(2);
    setAlignment(Qt::AlignCenter);
    init();
}

DicomImageLabel::~DicomImageLabel()
{
    delete imageInst;
}

void DicomImageLabel::init()
{
    if (imageInst) {
        QPixmap pixmap;
        imageInst->getPixmap(pixmap);
        setPixmap(pixmap.scaled(IMAGE_LABEL_SIZE-2, IMAGE_LABEL_SIZE-2, Qt::KeepAspectRatio));
    }
}

void DicomImageLabel::setHighlight(bool yes)
{
    QPalette p = palette();
    if (yes) p.setColor(QPalette::WindowText, Qt::green);
    else p.setColor(QPalette::WindowText, Qt::black);
    setPalette(p);
}

void DicomImageLabel::mousePressEvent(QMouseEvent *e)
{
    emit imageClicked(this);
    dragOrg = e->pos();
    QLabel::mousePressEvent(e);
}

void DicomImageLabel::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) &&
            ((e->pos()-dragOrg).manhattanLength() > QApplication::startDragDistance())) {
        QDrag *drag = new QDrag(this);
        QMimeData *data = new QMimeData;
        data->setImageData((qulonglong)imageInst);
        drag->setMimeData(data);
        drag->exec(Qt::CopyAction);
    }
    QLabel::mouseMoveEvent(e);
}

void DicomImageLabel::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit imageDoubleClicked(this);
    QLabel::mouseDoubleClickEvent(e);
}
