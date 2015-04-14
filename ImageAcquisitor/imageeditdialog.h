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
<<<<<<< HEAD
    explicit ImageEditDialog(QWidget *parent = 0);
    ~ImageEditDialog();

    void setFileFormat(DcmFileFormat *dff);
    void setImageTransform(int angle, int hori, int vert);

=======
    explicit ImageEditDialog(DcmFileFormat &dcmff, QWidget *parent = 0);
    ~ImageEditDialog();

>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
protected slots:
    void onImageOk();

private:
    void init();
    void setupConnections();
    Ui::ImageEditDialog *ui;
<<<<<<< HEAD
    DcmFileFormat *dcmFile;
=======
    DcmFileFormat &dcmFile;
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
    ImageEditGraphicsView *imageEditView;
};

#endif // IMAGEEDITDIALOG_H
