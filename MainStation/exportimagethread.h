#ifndef EXPORTIMAGETHREAD_H
#define EXPORTIMAGETHREAD_H

#include <QThread>
#include <QStringList>

class ExportImageThread : public QThread
{
    Q_OBJECT
public:
    /* output file types */
    enum E_FileType
    {
        EFT_Dicom,
        EFT_RawPNM,
        EFT_8bitPNM,
        EFT_16bitPNM,
        EFT_NbitPNM,
        EFT_BMP,
        EFT_8bitBMP,
        EFT_24bitBMP,
        EFT_32bitBMP,
        EFT_JPEG,
        EFT_TIFF,
        EFT_PNG,
        EFT_PastelPNM,
    };
    explicit ExportImageThread(QObject *parent = 0);

    void run();

    void setJpegQuality(int q) { jpegQuality = q; }
    void setFileType (E_FileType type) { fileType = type; }
    void setAbort(bool yes) { abort = yes; }
    void setImageFiles(const QStringList &images) { imageFiles = images; }
    void setDestDir(const QString &dir) { destDir = dir; }

signals:
    void resultReady(const QString &msg);

public slots:

private:
    bool abort;
    QStringList imageFiles;
    QString destDir;
    int jpegQuality;
    E_FileType fileType;

};

#endif // EXPORTIMAGETHREAD_H
