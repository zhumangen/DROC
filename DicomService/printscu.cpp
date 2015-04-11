#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#define PRINTSCU_CPP
#include "printscu.h"
#define INCLUDE_CCTYPE
#include "dcmtk/dcmpstat/dviface.h"
#include "dcmtk/dcmpstat/dvpssp.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmpstat/dvpshlp.h"
#include "../share/dicomscp.h"
#include "../share/configfiles.h"
#include "../DicomViewer/dicomimageview.h"

OFCondition printscu(const QList<DicomImageView*> &viewList, const PrintScp &printer, int copies)
{
    DVInterface dvi(PRINTSCU_CFG);
    OFCondition result = EC_Normal;
    DVPSStoredPrint &stprint = dvi.getPrintHandler();
    DVPSPrintMessageHandler printHandler;

    if (!printer.param.sessionLabel.isEmpty())
        dvi.setPrinterFilmSessionLabel(printer.param.sessionLabel.toLocal8Bit());
    if (!printer.param.ownerId.isEmpty())
        dvi.setPrinterOwnerID(printer.param.ownerId.toLocal8Bit());
    if (!printer.param.destination.isEmpty())
        dvi.setPrinterFilmDestination(printer.param.destination.toLatin1());
    else dvi.setPrinterFilmDestination("PROCESSOR");
    if (!printer.param.priority.isEmpty())
        dvi.setPrinterPriority(printer.param.priority.toLatin1());
    else dvi.setPrinterPriority("LOW");
    if (!printer.param.mediumType.isEmpty())
        dvi.setPrinterMediumType(printer.param.mediumType.toLatin1());
    else dvi.setPrinterMediumType("BLUE FILM");
    dvi.setPrinterNumberOfCopies(copies);
    int col, row;
    if (!PrintParam::parseDisplayFormat(printer.param.displayFormat, col, row))
        return EC_IllegalParameter;
    if (printer.param.filmOrientation == "LANDSCAPE") {
        stprint.setImageDisplayFormat(row, col);
        stprint.setFilmOrientation(DVPSF_landscape);
    } else {
        stprint.setImageDisplayFormat(col, row);
        stprint.setFilmOrientation(DVPSF_portrait);
    }
    stprint.setFilmSizeID(printer.param.filmSizeId.toLatin1());

    stprint.setPrinterName(printer.id.toLocal8Bit());
    if (!printer.param.magnificationType.isEmpty())
        stprint.setMagnificationType(printer.param.magnificationType.toLatin1());
    if (!printer.param.smoothingType.isEmpty())
        stprint.setSmoothingType(printer.param.smoothingType.toLatin1());
    if (!printer.param.resolutionId.isEmpty())
        stprint.setResolutionID(printer.param.resolutionId.toLatin1());
    if (!printer.param.borderDensity.isEmpty())
        stprint.setBorderDensity(printer.param.borderDensity.toLatin1());
    if (!printer.param.emptyImageDensity.isEmpty())
        stprint.setEmtpyImageDensity(printer.param.emptyImageDensity.toLatin1());
    if (!printer.param.maxDensity.isEmpty())
        stprint.setMaxDensity(printer.param.maxDensity.toLatin1());
    if (!printer.param.minDensity.isEmpty())
        stprint.setMinDensity(printer.param.minDensity.toLatin1());


    for (int i = 0; i < viewList.size(); ++i) {
        DicomImageView *v = viewList.at(i);
        DicomImage *image;
        if (v && (image=v->getHardCopyGrayScaledImage())) {
            DcmFileFormat *file = new DcmFileFormat;
            DcmDataset *dset = file->getDataset();

            image->writeFrameToDataset(*dset);
            dvi.getCurrentPState().attachImage(file, true);

            ushort pvalue = image->getPhotometricInterpretation()==EPI_Monochrome1?65535:0;
            ulong bitmapSize = dvi.getCurrentPState().getPrintBitmapSize();
            char *pixelData = new char[bitmapSize];
            ulong width, height;
            dvi.getCurrentPState().getPrintBitmapWidthHeight(width, height);
            dvi.getCurrentPState().getPrintBitmap((void*)pixelData, bitmapSize, false);

            QImage textLayer = v->getTextLayerImage(QSize(width, height));
            Uint16* p = (Uint16*)pixelData;
            for (ulong y = 0; y < height; ++y) {
                for (ulong x = 0; x < width; ++x) {
                    if (textLayer.valid(x, y) && (textLayer.pixel(x, y)!=QRgb(0xff000000)))
                        *(p + y * width + x) = pvalue;
                }
            }

            double pixelAspectRatio = dvi.getCurrentPState().getPrintBitmapPixelAspectRatio();
            dvi.saveHardcopyGrayscaleImage((void*)pixelData, width, height, pixelAspectRatio);
            delete[] pixelData;
            delete image;
        }
    }

    dvi.saveStoredPrint(true);

#ifdef HAVE_WINSOCK_H
    WSAData winSockData;
    WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
    WSAStartup(winSockVersionNeeded, &winSockData);
#endif

    if (EC_Normal == result)
        result = printHandler.negotiateAssociation(NULL, dvi.getNetworkAETitle(),
                                                   printer.aetitle.toLatin1(),
                                                   printer.hostname.toLatin1(),
                                                   printer.port,
                                                   ASC_DEFAULTMAXPDU,
                                                   printer.param.supportsPresentationLut,
                                                   false,
                                                   printer.param.implicitOnly);
    if (EC_Normal == result)
        result = stprint.printSCUgetPrinterInstance(printHandler);
    if (EC_Normal == result)
        result = stprint.printSCUpreparePresentationLUT(printHandler, false, false, true);
    if (EC_Normal == result)
        result = dvi.printSCUcreateBasicFilmSession(printHandler, false);
    if (EC_Normal == result)
        result = stprint.printSCUcreateBasicFilmBox(printHandler, false);

    if (EC_Normal == result) {
        size_t numberOfImages = stprint.getNumberOfImages();
        const char *studyUID = NULL;
        const char *seriesUID = NULL;
        const char *instanceUID = NULL;
        const char *imagefile = NULL;
        OFString theFilename;
        DicomImage *dcmimage = NULL;
        for (size_t currentImage=0; currentImage<numberOfImages; currentImage++)
        {
            result = stprint.getImageReference(currentImage, studyUID, seriesUID, instanceUID);
            if ((EC_Normal == result) && studyUID && seriesUID && instanceUID)
            {
              imagefile = dvi.getFilename(studyUID, seriesUID, instanceUID);
              if (imagefile) theFilename = imagefile; else theFilename.clear();
              if (EC_Normal == result)
              {
                dcmimage = new DicomImage(imagefile);
                if (dcmimage && (EIS_Normal == dcmimage->getStatus()))
                {
                  // N-SET basic image box
                  result = stprint.printSCUsetBasicImageBox(printHandler, currentImage, *dcmimage, 0);
                }
                delete dcmimage;
              }
            }
            dvi.deleteInstance(studyUID, seriesUID, instanceUID);
        }
        dvi.deleteInstance(stprint.getStudyInstanceUID(), stprint.getSeriesInstanceUID(),
                           stprint.getSOPInstanceUID());
        dvi.releaseDatabase();
    }

    if (EC_Normal == result)
        result = stprint.printSCUprintBasicFilmBox(printHandler);
    if (EC_Normal == result)
        result = stprint.printSCUdelete(printHandler);

    printHandler.releaseAssociation();

#ifdef HAVE_WINSOCK_H
    WSACleanup();
#endif

    return result;
}
