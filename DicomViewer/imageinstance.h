#ifndef DICOMINSTANCE_H
#define DICOMINSTANCE_H

#include <QString>
#include <QPixmap>
#include <QDateTime>
#include "../share/studyrecord.h"
#include "dcmtk/dcmimgle/diutils.h"
class DicomImage;

class ImageInstance
{
public:
    explicit ImageInstance(const QString &file);
    ~ImageInstance();

    void setImageType(bool raw) { rawType = raw; }

    void setWindow(const double &center, const double &width)
    {   if (rawType) {
            rawCenter = center; rawWidth = width;
        } else {
            winCenter = center; winWidth = width;
        }
    }

    void getWindow(double &center, double &width) const
    {   if (rawType) {
            center = rawCenter; width = rawWidth;
        } else {
            center = winCenter; width = winWidth;
        }
    }

    void setWindowDelta(const double &dCenter, const double &dWidth)
    {   if (rawType) {
            rawCenter += dCenter; rawWidth += dWidth;
        } else {
            winCenter += dCenter; winWidth += dWidth;
        }
    }

    void setRoiWindow(const QRectF &rect);
    void setDefaultWindow()
    {   if (rawType) {
            rawCenter = defCenter; rawWidth = defWidth;
        } else {
            winCenter = defCenter; winWidth = defWidth;
        }
    }

    void setPolarity(EP_Polarity polarity);
    EP_Polarity getPolarity() const;
    bool getPixmap(QPixmap &pixmap);
    static bool getPixmap(const QString &dicomFile, QPixmap &pixmap);
    static bool dcm2bmpHelper(DicomImage &dcmImage, QPixmap &pixmap);

    bool hasRawImage();
    bool isRawImage() const { return rawType; }
    inline bool isNormal() const;
    DicomImage* createClippedImage(const QRect &rect, int angle = 0, bool hflip = false, bool vflip = false, bool inverted = false);
    QStringList getAnnotations();
    uint getPixelValue(long x, long y, EP_Representation &r) const;
    bool getPixSpacing(double &spacingX, double &spacingY) const;

    bool getStudyRecord(StudyRecord &studyRec) const
    {
        if (!studyUid.isEmpty()) {
            studyRec.studyUid = studyUid;
            studyRec.accNumber = accessionNumber;
            studyRec.studyTime = studyTime;
            studyRec.studyDesc = studyDes;
            studyRec.patientId = patientID;
            studyRec.patientName = patientName;
            studyRec.patientBirth = patientBirth;
            studyRec.patientSex = patientSex;
            studyRec.modality = modality;
            studyRec.institution = institution;
            studyRec.procId = procId;
            studyRec.perPhysician = perPhysician;
            studyRec.reqPhysician = reqPhysician;
            return true;
        } else return false;
    }

    QString patientID;
    QString patientName;
    QDate patientBirth;
    QString patientSex;
    QString patientAge;

    QString studyUid;
    QDateTime studyTime;
    QString studyDes;
    QString accessionNumber;
    QString procId;
    QString bodyPart;

    QString seriesUid;
    QString seriesNumber;
    QString seriesDes;
    QString modality;
    QString stationName;
    QString manufacturer;

    QString instanceUid;
    QString sopClassUid;
    QString instanceNumber;
    QDateTime acquisitionTime;
    QString institution;
    QString patientPostion;
    QString reqPhysician;
    QString perPhysician;

    QString filePath;

private:
    void initImage(const QString &file);

    double kvp;
    int mAs;
    double pixelSpacingX;
    double pixelSpacingY;

    double defCenter;
    double defWidth;
    double winWidth;
    double winCenter;
    double cachedCenter;
    double cachedWidth;
    DicomImage *dcmImage;
    QPixmap cachedPixmap;

    bool rawType;
    double rawCenter;
    double rawWidth;
    double cachedRawCenter;
    double cachedRawWidth;
    DicomImage *rawImage;
    QPixmap cachedRawPixmap;
    QString rawFile;
};

#endif // DICOMINSTANCE_H
