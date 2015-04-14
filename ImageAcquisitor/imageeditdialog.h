#ifndef IMAGEEDITDIALOG_H
#define IMAGEEDITDIALOG_H

#include <QDialog>
class DcmFileFormat;
class ImageEditGraphicsView;

namespace Ui {
class ImageEditDialog;
}

class ImageEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageEditDialog(QWidget *parent = 0);
    ~ImageEditDialog();

    void setFileFormat(DcmFileFormat *dff);
    void setImageTransform(int angle, int hori, int vert);

protected slots:
    void onImageOk();

private:
    void init();
    void setupConnections();
    Ui::ImageEditDialog *ui;

    DcmFileFormat *dcmFile;
    ImageEditGraphicsView *imageEditView;
};

#endif // IMAGEEDITDIALOG_H
