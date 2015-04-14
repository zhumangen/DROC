#ifndef IMAGEVIEWWIDGET_H
#define IMAGEVIEWWIDGET_H

#include <QWidget>
class DicomScp;
class ThumbnailBarWidget;
class DicomImageView;
class QGridLayout;
class ImageInstance;
class DicomImageLabel;
class StudyRecord;

namespace Ui {
class ImageViewWidget;
}

class ImageViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewWidget(QWidget *parent = 0);
    ~ImageViewWidget();

    bool getCurrentStudy(StudyRecord &study);

signals:
    void currentViewChanged(DicomImageView *view);

public slots:
    void setImageFiles(const QStringList &files);
    void onPrintScpUpdated(const QList<DicomScp*> &scps);
    void setObserveMode(bool yes);
    void clear();

protected slots:
    void onModeChanged(bool obsMode);
    void onPrinterChanged(int);
    void onFilmSizeChanged(const QString &sizeId);
    void onFilmLayoutChanged(const QString &layout);
    void onFilmOrientationChanged(int);

    void onImageInserted(ImageInstance *image);
    void onImageDoubleClicked(DicomImageLabel *label);
    void onImageChanged(DicomImageLabel *label);

    void onViewClicked(DicomImageView *view);
    void onViewDoubleClicked(DicomImageView *view);

    void toolButtonClicked();
    void onRawImageButton(bool raw);
    void onFilmPrint();
    void onWindowsPrint();

protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    void init();
    inline DicomImageView *createImageView();
    inline void setViewPrintMode(bool yes);
    void createConnections();
    void loadPrinterConfig();
    void clearPrintScps();
    inline void updateRawImageButton();

    Ui::ImageViewWidget *ui;
    ThumbnailBarWidget *thumbnailBar;
    QList<DicomImageView*> viewList;
    QGridLayout *viewLayout;
    DicomImageView *currentView;

    bool viewMaximised;
};

#endif // IMAGEVIEWWIDGET_H
