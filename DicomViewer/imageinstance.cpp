#include "imageinstance.h"
#include "../share/global.h"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcuid.h"

<<<<<<< HEAD
#include <QFile>

=======
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
ImageInstance::ImageInstance(const QString &file):
    rawType(false),
    rawImage(0),
    cachedRawCenter(0),
    cachedRawWidth(-1),
    dcmImage(0),
    cachedCenter(0),
    cachedWidth(-1),
    filePath(file)
{
    initImage(file);
}

ImageInstance::~ImageInstance()
{
    delete rawImage;
    delete dcmImage;
}

bool ImageInstance::isNormal() const
{
    if (rawType) {
        return rawImage && (rawImage->getStatus()==EIS_Normal);
    } else {
        return dcmImage && (dcmImage->getStatus() == EIS_Normal);
    }
}

bool ImageInstance::hasRawImage()
{
<<<<<<< HEAD
    return QFile(rawFile).exists();
    /*
    if (!rawFile.isEmpty()) {
        if ()
=======
    if (!rawFile.isEmpty()) {
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
        if (rawImage) return true;

        rawImage = new DicomImage(rawFile.toLocal8Bit().data());
        if (EIS_Normal == rawImage->getStatus()) {
            return true;
        } else {
            delete rawImage;
            rawImage = 0;
        }
    }

    return false;
<<<<<<< HEAD
    */
=======
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
}

void ImageInstance::initImage(const QString &file)
{
    DcmFileFormat dcmFile;
    OFCondition result;

    result = dcmFile.loadFile(file.toLocal8Bit());
    DcmDataset *dset = dcmFile.getDataset();

    if (result.good()) {
        //patient information
        const char *value = NULL;
        result = dset->findAndGetString(DCM_PatientID, value);
        patientID = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_PatientName, value);
        patientName = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_PatientAge, value);
        patientAge = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_PatientBirthDate, value);
        patientBirth = QDate::fromString(QString::fromLocal8Bit(value),
                                         DATE_DICOM_FORMAT);

        result = dset->findAndGetString(DCM_PatientSex, value);
        patientSex = QString::fromLocal8Bit(value);

        //study information
        result = dset->findAndGetString(DCM_StudyInstanceUID, value);
        studyUid = QString::fromLatin1(value);

        result = dset->findAndGetString(DCM_StudyDate, value);
        studyTime.setDate(QDate::fromString(QString::fromLocal8Bit(value),
                                            DATE_DICOM_FORMAT));

        result = dset->findAndGetString(DCM_StudyTime, value);
        studyTime.setTime(formatDicomTime(QString::fromLatin1(value)));

        result = dset->findAndGetString(DCM_StudyDescription, value);
        studyDes = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_ProtocolName, value);
        procId = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_BodyPartExamined, value);
        bodyPart = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_Manufacturer, value);
        manufacturer = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_Modality, value);
        modality = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_StationName, value);
        stationName = QString::fromLocal8Bit(value);

        //series information
        result = dset->findAndGetString(DCM_SeriesInstanceUID, value);
        seriesUid = QString::fromLatin1(value);

        result = dset->findAndGetString(DCM_SeriesNumber, value);
        seriesNumber = QString::fromLatin1(value);

        result = dset->findAndGetString(DCM_SeriesDescription, value);
        seriesDes = QString::fromLocal8Bit(value);

        //instance information
        result = dset->findAndGetString(DCM_SOPClassUID, value);
        sopClassUid = QString::fromLatin1(value);

        result = dset->findAndGetString(DCM_SOPInstanceUID, value);
        instanceUid = QString::fromLatin1(value);

        result = dset->findAndGetString(DCM_InstanceNumber, value);
        instanceNumber = QString::fromLatin1(value);

        result = dset->findAndGetString(DCM_InstitutionName, value);
        institution = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_AcquisitionDate, value);
        if (value==NULL) result = dset->findAndGetString(DCM_ContentDate, value);
        acquisitionTime.setDate(QDate::fromString(QString::fromLatin1(value),
                                                  DATE_DICOM_FORMAT));

        result = dset->findAndGetString(DCM_AcquisitionTime, value);
        if (value==NULL) result = dset->findAndGetString(DCM_ContentTime, value);
        acquisitionTime.setTime(formatDicomTime(QString::fromLatin1(value)));

        result = dset->findAndGetString(DCM_PatientPosition, value);
        patientPostion = QString::fromLatin1(value);

        result = dset->findAndGetString(DCM_RequestingPhysician, value);
        reqPhysician = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_PerformingPhysicianName, value);
        perPhysician = QString::fromLocal8Bit(value);

        result = dset->findAndGetString(DCM_AccessionNumber, value);
        accessionNumber = QString::fromLatin1(value);

        result = dset->findAndGetFloat64(DCM_PixelSpacing, pixelSpacingY, 0);
        result = dset->findAndGetFloat64(DCM_PixelSpacing, pixelSpacingX, 1);

        result = dset->findAndGetString(DCM_KVP, value);
        kvp = QString::fromLatin1(value).toDouble();
        result = dset->findAndGetString(DCM_Exposure, value);
        mAs = QString::fromLatin1(value).toInt();

        result = dset->findAndGetFloat64(DCM_WindowWidth, winWidth);
        result = dset->findAndGetFloat64(DCM_WindowCenter, winCenter);
        defCenter = winCenter;
        defWidth = winWidth;

        if (UID_DigitalXRayImageStorageForPresentation == sopClassUid) {
            DcmItem *refItem;
            const char *refImageUid;
            dset->findAndGetSequenceItem(DCM_ReferencedImageSequence, refItem);
            if (refItem && refItem->findAndGetString(DCM_ReferencedSOPInstanceUID, refImageUid).good()) {
                QString dirName = file.left(file.lastIndexOf('/'));
                rawFile = QString("%1/%2_%3.dcm").arg(dirName, RAW_IMAGE_PREFIX, refImageUid);
                rawCenter = winCenter;
                rawWidth = winWidth;
            }
        }
    }

    dcmImage = new DicomImage(dset, dset->getOriginalXfer());
    if (dcmImage->getStatus() == EIS_Normal) {
        //dcmImage->getWindow(winCenter, winWidth);
        getPixmap(cachedPixmap);
    }
}

void ImageInstance::setRoiWindow(const QRectF &rect)
{
    if (rawType) {
        if (rawImage) {
            rawImage->setRoiWindow(rect.left(), rect.top(), rect.width(), rect.height());
            rawImage->getWindow(rawCenter, rawWidth);
        }
    } else {
        dcmImage->setRoiWindow(rect.left(), rect.top(), rect.width(), rect.height());
        dcmImage->getWindow(winCenter, winWidth);
    }
}

void ImageInstance::setPolarity(EP_Polarity polarity)
{
    if (rawType) {
        if (rawImage) {
            rawImage->setPolarity(polarity);
            dcm2bmpHelper(*rawImage, cachedRawPixmap);
        }
    } else {
        dcmImage->setPolarity(polarity);
        dcm2bmpHelper(*dcmImage, cachedPixmap);
    }
}

EP_Polarity ImageInstance::getPolarity() const
{
    if (rawType) {
        if (rawImage) return rawImage->getPolarity();
        else return EPP_Normal;
    } else {
        return dcmImage->getPolarity();
    }
}

bool ImageInstance::dcm2bmpHelper(DicomImage &dcmImage, QPixmap &pixmap)
{
    BITMAPFILEHEADER lpfh;
    BITMAPINFOHEADER lpih;
    RGBQUAD palette[256];

    memset(&lpfh, 0, sizeof(BITMAPFILEHEADER));
    lpfh.bfType = 0x4d42;  //'B''M'
    lpfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(palette);


    memset(&lpih, 0, sizeof(BITMAPINFOHEADER));
    lpih.biSize = sizeof(BITMAPINFOHEADER);
    lpih.biWidth = dcmImage.getWidth();
    lpih.biHeight = dcmImage.getHeight();
    lpih.biCompression = BI_RGB;
    lpih.biBitCount = 8;
    lpih.biPlanes = 1;

    memset(palette, 0, sizeof(palette));
    for (int i = 0; i < 256; ++i) {
        palette[i].rgbBlue = i;
        palette[i].rgbGreen = i;
        palette[i].rgbRed = i;
    }

    void *pDIB = NULL;

    int size = dcmImage.createWindowsDIB(pDIB, 0, 0, 8, 1, 1);

    //lpih.biSizeImage = size;
    lpfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(palette) + size;

    QByteArray bmp;
    bmp.append((char*)&lpfh, sizeof(BITMAPFILEHEADER));
    bmp.append((char*)&lpih, sizeof(BITMAPINFOHEADER));
    bmp.append((char*)palette, sizeof(palette));
    bmp.append((char*)pDIB, size);

    delete pDIB;
    return pixmap.loadFromData(bmp);
}

bool ImageInstance::getPixmap(QPixmap &pixmap)
{
    bool retval = true;
<<<<<<< HEAD

    if (rawType && (!rawImage))
        rawImage = new DicomImage(rawFile.toLocal8Bit().data());

=======
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
    if (isNormal()) {
        if (rawType) {
            if (rawWidth < 1) rawWidth = 1;
            if (rawCenter != cachedRawCenter || rawWidth != cachedRawWidth) {
                cachedRawCenter = rawCenter;
                cachedRawWidth = rawWidth;
                rawImage->setWindow(rawCenter, rawWidth);
                retval = dcm2bmpHelper(*rawImage, cachedRawPixmap);
            }
            pixmap = cachedRawPixmap;
        } else {
            if (winWidth < 1) winWidth = 1;
            if (winCenter != cachedCenter || winWidth != cachedWidth) {
                cachedCenter = winCenter;
                cachedWidth = winWidth;
                dcmImage->setWindow(winCenter, winWidth);
                retval = dcm2bmpHelper(*dcmImage, cachedPixmap);
            }
            pixmap = cachedPixmap;
        }
    } else retval = false;

    return retval;
}

bool ImageInstance::getPixmap(const QString &dicomFile, QPixmap &pixmap)
{
    ImageInstance image(dicomFile);
    return image.getPixmap(pixmap);
}

DicomImage* ImageInstance::createClippedImage(const QRect &rect, int angle, bool hflip, bool vflip, bool inverted)
{
    DicomImage *image;
    if (rawType) {
        image = rawImage;
        if (!image) return image;
    } else image = dcmImage;

    int ret = 1;
    Uint16 pvalue = image->getPhotometricInterpretation()==EPI_Monochrome1?65535:0;
    DicomImage *newImage = image->createClippedImage(rect.left(), rect.top(), rect.width(), rect.height(), pvalue);
    if (newImage) {
        if (ret && angle) ret = newImage->rotateImage(angle%360);
        if (ret && hflip) ret = newImage->flipImage(1, 0);
        if (ret && vflip) ret = newImage->flipImage(0, 1);
        if (ret && inverted) ret = newImage->setPolarity(EPP_Reverse);
        if (!ret) {
            delete newImage;
            newImage = 0;
        }
    }
    return newImage;
}

QStringList ImageInstance::getAnnotations()
{
    QString topleft("Im: %1  Se: %3\n%4%5Scalor: 100mm");
    QString topright("%1\n%2  %3/%4\n%5");
    QString bottomleft("%1\n%2");
    QString bottomright("%1 mAs  %2 kV\n%3\n%4");

    topleft = topleft.arg(instanceNumber, seriesNumber,
                          studyDes.isEmpty()?studyDes:QString("\n").prepend(studyDes),
                          seriesDes.isEmpty()?seriesDes:QString("\n").prepend(seriesDes));
    topright = topright.arg(patientName, patientID, patientSex, patientAge, patientBirth.toString("yyyy/M/d"));
    bottomleft = bottomleft.arg(procId.isEmpty()?bodyPart:procId, manufacturer);
    bottomright = bottomright.arg(QString::number(mAs), QString::number(kvp),
                                  acquisitionTime.toString("yyyy/M/d hh:mm:ss"),
                                  institution);
    return QStringList() << topleft << topright << bottomleft << bottomright;
}

uint ImageInstance::getPixelValue(long x, long y, EP_Representation &r) const
{
    DicomImage *image;
    if (rawType) image = rawImage;
    else image = dcmImage;

    if (image) {
        const DiPixel* pixel = image->getInterData();
        if (pixel && (x < (long)image->getWidth()) && (x >= 0)
                && (y < (long)image->getHeight()) && (y >= 0)) {
            r = pixel->getRepresentation();
            switch (r) {
            case EPR_Sint8:
            case EPR_Uint8:
                return *((char*)(pixel->getData()) + (y * image->getWidth() + x));
            case EPR_Sint16:
            case EPR_Uint16:
                return *((short*)(pixel->getData()) + (y * image->getWidth() + x));
            case EPR_Sint32:
            case EPR_Uint32:
                return *((int*)(pixel->getData()) + (y * image->getWidth() + x));
            }
        }
    }

    r = (EP_Representation)-1;
    return 0;
}

bool ImageInstance::getPixSpacing(double &spacingX, double &spacingY) const
{
    if (isNormal()) {
        spacingX = pixelSpacingX;
        spacingY = pixelSpacingY;
        return true;
    } else return false;
}
