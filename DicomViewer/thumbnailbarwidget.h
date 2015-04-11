#ifndef THUMBNAILBARWIDGET_H
#define THUMBNAILBARWIDGET_H

#include <QWidget>
#include <QBoxLayout>
class ImageInstance;
class DicomImageLabel;
class ImageLoadThread;
class StudyRecord;

class ThumbnailBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ThumbnailBarWidget(QWidget *parent = 0);
    ~ThumbnailBarWidget() { clear(); }

    void setBarDirection(QBoxLayout::Direction direction) { layout->setDirection(direction); }
    DicomImageLabel *getCurrentImageLabel() const { return currentImageLabel; }

    bool getCurrentStudy(StudyRecord &study) const;
    QList<ImageInstance*> getImageList() const;
    QSize sizeHint() const;

public slots:
    void setCurrentImageLabel(const ImageInstance *image);

protected:
    void keyPressEvent(QKeyEvent *e);

signals:
    void currentChanged(DicomImageLabel *label);
    void imageDoubleClicked(DicomImageLabel *label);
    void imageDoubleClicked(const QString &imageFile);
    void imageInserted(ImageInstance *image);

public slots:
    void appendImageFiles(const QStringList &files);
    void setImageFiles(const QStringList &files) { clear(); appendImageFiles(files); }
    void clear();

    void setCurrentStudyUid(const QString &uid);
    void removeStduy(const QString &uid);
    void addStudy(const QString &uid);

private slots:
    void onImageReady(ImageInstance *image);
    void onImageClicked(DicomImageLabel *imageLabel);
    void onImageDoubleClicked(DicomImageLabel *imageLabel);

private:
    QStringList getImageFilesInStudy(const QString &studyUid);
    QBoxLayout *layout;
    QList<DicomImageLabel*> imageLabelList;
    ImageLoadThread *loadThread;
    DicomImageLabel *currentImageLabel;

    QStringList studyUids;
    QString curStudyUid;
};

#endif // THUMBNAILBARWIDGET_H
