#include "thumbnailbarwidget.h"
#include "dicomimagelabel.h"
#include "../share/global.h"
#include "imageloadthread.h"
#include "imageinstance.h"
#include "../MainStation/sqlimagemodel.h"

#include "dcmtk/dcmdata/dcuid.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

ThumbnailBarWidget::ThumbnailBarWidget(QWidget *parent) :
    currentImageLabel(0),
    loadThread(new ImageLoadThread(this)),
    QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->addStretch();
    layout->setAlignment(Qt::AlignCenter);

    connect(loadThread, SIGNAL(resultReady(ImageInstance*)),
            this, SLOT(onImageReady(ImageInstance*)));
}

void ThumbnailBarWidget::setCurrentImageLabel(const ImageInstance *image)
{
    bool found = false;
    foreach (DicomImageLabel *label, imageLabelList) {
        if (image == label->getImageInstance()) {
            if (currentImageLabel) currentImageLabel->setHighlight(false);
            currentImageLabel = label;
            currentImageLabel->setHighlight(true);
            found = true;
            break;
        }
    }
    if ((!found) && currentImageLabel)
        currentImageLabel->setHighlight(false);
}

bool ThumbnailBarWidget::getCurrentStudy(StudyRecord &study) const
{
    if (currentImageLabel && currentImageLabel->getImageInstance())
        return currentImageLabel->getImageInstance()->getStudyRecord(study);
    return false;
}

QStringList ThumbnailBarWidget::getImageFilesInStudy(const QString &studyUid)
{
    QSqlDatabase db = QSqlDatabase::database(STUDY_DB_CONNECTION_NAME);
    QSqlQuery query(db);
    QStringList files;
    query.prepare("SELECT * FROM ImageTable WHERE StudyUid=? AND SopClassUid=?");
    query.addBindValue(studyUid);
    query.addBindValue(QString::fromLatin1(UID_DigitalXRayImageStorageForPresentation));
    if (query.exec()) {
        while (query.next()) {
            QSqlRecord rec = query.record();
            files << rec.value(SqlImageModel::ImageFile).toString();
        }
    }
    return files;
}

void ThumbnailBarWidget::setCurrentStudyUid(const QString &uid)
{
    foreach (DicomImageLabel *label, imageLabelList) {
        if (label->getImageInstance()->studyUid == uid) {
            label->setVisible(true);
        } else {
            if (currentImageLabel == label) {
                currentImageLabel->setHighlight(false);
                currentImageLabel = 0;
            }
            label->setVisible(false);
        }
    }
}

void ThumbnailBarWidget::addStudy(const QString &uid)
{
    if (!studyUids.contains(uid))
        appendImageFiles(getImageFilesInStudy(uid));
    studyUids << uid;
}

void ThumbnailBarWidget::removeStduy(const QString &uid)
{
    studyUids.removeOne(uid);
    if (!studyUids.contains(uid)) {
        // release resources
        foreach (DicomImageLabel *label, imageLabelList) {
            if (label->getImageInstance()->studyUid == uid) {
                if (currentImageLabel == label) {
                    currentImageLabel->setHighlight(false);
                    currentImageLabel = 0;
                }
                imageLabelList.removeOne(label);
                delete label;
            }
        }
    }
}

void ThumbnailBarWidget::appendImageFiles(const QStringList &files)
{
    loadThread->wait();
    loadThread->setImageFiles(files);
    loadThread->setAbort(false);
    loadThread->start();
}

void ThumbnailBarWidget::onImageReady(ImageInstance *image)
{
    if (image) {
        DicomImageLabel *imageLabel = new DicomImageLabel(image);
        connect(imageLabel, SIGNAL(imageClicked(DicomImageLabel*)),
                this, SLOT(onImageClicked(DicomImageLabel*)));
        connect(imageLabel, SIGNAL(imageDoubleClicked(DicomImageLabel*)),
                this, SLOT(onImageDoubleClicked(DicomImageLabel*)));
        layout->insertWidget(imageLabelList.size(), imageLabel);
        imageLabelList.append(imageLabel);
        emit imageInserted(image);
    }
}

void ThumbnailBarWidget::onImageClicked(DicomImageLabel *imageLabel)
{
    if (currentImageLabel != imageLabel) {
        if (currentImageLabel) currentImageLabel->setHighlight(false);
        currentImageLabel = imageLabel;
        if (currentImageLabel) {
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        }
    }
}

void ThumbnailBarWidget::onImageDoubleClicked(DicomImageLabel *imageLabel)
{
    if (currentImageLabel) currentImageLabel->setHighlight(false);
    currentImageLabel = imageLabel;
    if (currentImageLabel) {
        currentImageLabel->setHighlight(true);
        emit imageDoubleClicked(currentImageLabel);
        if (currentImageLabel->getImageInstance())
            emit imageDoubleClicked(currentImageLabel->getImageInstance()->filePath);
    }
}

QList<ImageInstance*> ThumbnailBarWidget::getImageList() const
{
    QList<ImageInstance*> imageList;
    foreach (DicomImageLabel *label, imageLabelList) {
        imageList << label->getImageInstance();
    }
    return imageList;
}

QSize ThumbnailBarWidget::sizeHint() const
{
    QMargins margin = layout->contentsMargins();
    int width = DicomImageLabel::getImageLabelSize();
    int height = DicomImageLabel::getImageLabelSize()*imageLabelList.size();
    if (imageLabelList.size() > 1)
        height += (imageLabelList.size()-1)*layout->spacing();

    switch (layout->direction()) {
    case QBoxLayout::TopToBottom:
    case QBoxLayout::BottomToTop:
        return QSize(width + margin.left() + margin.right(),
                     height + margin.top() + margin.bottom());
        break;
    case QBoxLayout::LeftToRight:
    case QBoxLayout::RightToLeft:
        return QSize(width + margin.top() + margin.bottom(),
                     height + margin.left() + margin.right());
        break;
    default:
        return QSize();
    }
}

void ThumbnailBarWidget::clear()
{
    loadThread->setAbort(true);
    loadThread->wait(1000);

    qDeleteAll(imageLabelList);
    imageLabelList.clear();
    currentImageLabel = 0;
    studyUids.clear();
}

void ThumbnailBarWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Return:
        if (currentImageLabel) emit imageDoubleClicked(currentImageLabel);
        break;
    case Qt::Key_Home:
        if (currentImageLabel && (currentImageLabel != imageLabelList.first())) {
            currentImageLabel->setHighlight(false);
            currentImageLabel = imageLabelList.first();
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        } else if ((!currentImageLabel) && imageLabelList.size()) {
            currentImageLabel = imageLabelList.first();
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        }
        break;
    case Qt::Key_PageDown:
    case Qt::Key_Down:
    case Qt::Key_Right:
        if (currentImageLabel && (imageLabelList.size()>1)) {
            currentImageLabel->setHighlight(false);
            if (currentImageLabel == imageLabelList.last()) currentImageLabel = imageLabelList.first();
            else currentImageLabel = imageLabelList.at(imageLabelList.indexOf(currentImageLabel)+1);
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        } else if (imageLabelList.size()) {
            currentImageLabel = imageLabelList.first();
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        }
        break;
    case Qt::Key_End:
        if (currentImageLabel && (currentImageLabel != imageLabelList.last())) {
            currentImageLabel->setHighlight(false);
            currentImageLabel = imageLabelList.last();
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        } else if ((!currentImageLabel) && imageLabelList.size()) {
            currentImageLabel = imageLabelList.last();
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        }
        break;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
    case Qt::Key_Left:
        if (currentImageLabel && (imageLabelList.size()>1)) {
            currentImageLabel->setHighlight(false);
            if (currentImageLabel == imageLabelList.first()) currentImageLabel = imageLabelList.last();
            else currentImageLabel = imageLabelList.at(imageLabelList.indexOf(currentImageLabel)-1);
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        } else if (imageLabelList.size()) {
            currentImageLabel = imageLabelList.last();
            currentImageLabel->setHighlight(true);
            emit currentChanged(currentImageLabel);
        }
        break;
    default:
        QWidget::keyPressEvent(e);
    }
}
