#include "exportimagethread.h"

#include "mainwindow.h"

#include <QDir>

ExportImageThread::ExportImageThread(QObject *parent) :
    abort(false),
    fileType(EFT_Dicom),
    jpegQuality(90),
    QThread(parent)
{
}

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define BUILD_DCM2PNM_AS_DCMJ2PNM
#define PASTEL_COLOR_OUTPUT
#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/dcmdata/dctk.h"          /* for various dcmdata headers */
#include "dcmtk/dcmdata/dcrledrg.h"      /* for DcmRLEDecoderRegistration */

#include "dcmtk/dcmimgle/dcmimage.h"     /* for DicomImage */
#include "dcmtk/dcmimgle/digsdfn.h"      /* for DiGSDFunction */
#include "dcmtk/dcmimgle/diciefn.h"      /* for DiCIELABFunction */

#include "dcmtk/dcmimage/diregist.h"     /* include to support color images */
#include "dcmtk/ofstd/ofstd.h"           /* for OFStandard */

#ifdef BUILD_DCM2PNM_AS_DCMJ2PNM
#include "dcmtk/dcmjpeg/djdecode.h"      /* for dcmjpeg decoders */
#include "dcmtk/dcmjpeg/dipijpeg.h"      /* for dcmimage JPEG plugin */
#endif

#ifdef BUILD_DCM2PNM_AS_DCML2PNM
#include "dcmtk/dcmjpls/djdecode.h"      /* for dcmjpls decoders */
#endif

#ifdef WITH_LIBTIFF
#include "dcmtk/dcmimage/dipitiff.h"     /* for dcmimage TIFF plugin */
#endif

#ifdef WITH_LIBPNG
#include "dcmtk/dcmimage/dipipng.h"      /* for dcmimage PNG plugin */
#endif

#ifdef WITH_ZLIB
#include <zlib.h>                        /* for zlibVersion() */
#endif

void ExportImageThread::run()
{
#ifdef WITH_LIBTIFF
    // TIFF parameters
#ifdef HAVE_LIBTIFF_LZW_COMPRESSION
    DiTIFFCompression   opt_tiffCompression = E_tiffLZWCompression;
#else
    DiTIFFCompression   opt_tiffCompression = E_tiffPackBitsCompression;
#endif
    DiTIFFLZWPredictor  opt_lzwPredictor = E_tiffLZWPredictorDefault;
    OFCmdUnsignedInt    opt_rowsPerStrip = 0;
#endif

#ifdef WITH_LIBPNG
    // PNG parameters
    DiPNGInterlace      opt_interlace = E_pngInterlaceAdam7;
    DiPNGMetainfo       opt_metainfo  = E_pngFileMetainfo;
#endif

    /* make sure data dictionary is loaded */
    //dcmDataDict.isDictionaryLoaded();

    // register RLE decompression codec
    DcmRLEDecoderRegistration::registerCodecs();
#ifdef BUILD_DCM2PNM_AS_DCMJ2PNM
    // register JPEG decompression codecs
    DJDecoderRegistration::registerCodecs(EDC_photometricInterpretation);
#endif
#ifdef BUILD_DCM2PNM_AS_DCML2PNM
    // register JPEG-LS decompression codecs
    DJLSDecoderRegistration::registerCodecs();
#endif

    foreach (QString file, imageFiles) {

        if (abort) break;

        DcmFileFormat dfile;
        OFCondition cond = dfile.loadFile(QString("%1/%2").arg(mainWindow->getDbLocation(), file).toLocal8Bit().data());

        if (cond.bad())
        {
            emit resultReady(tr("Failed: %1, %2.").arg(file, QString::fromLocal8Bit(cond.text())));
            continue;
        }

        if (fileType == EFT_Dicom) {
            QString filepath = QString("%1/%2").arg(destDir, file);
            QDir().mkpath(filepath.mid(0, filepath.lastIndexOf('/')));
            OFCondition l_error = dfile.saveFile(filepath.toLocal8Bit().data());
            if (EC_Normal == l_error) {
                emit resultReady(tr("Exported: %1.").arg(file));
            } else {
                emit resultReady(tr("Failed: %1, %2.").arg(file, QString::fromLocal8Bit(l_error.text())));
            }
            continue;
        }

        DcmDataset *dataset = dfile.getDataset();
        E_TransferSyntax xfer = dataset->getOriginalXfer();

        Sint32 frameCount;
        if (dataset->findAndGetSint32(DCM_NumberOfFrames, frameCount).bad())
            frameCount = 1;

        ulong compabilityMode = CIF_DecompressCompletePixelData;
        if (frameCount > 1) compabilityMode |= CIF_UsePartialAccessToPixelData;
        DicomImage *di = new DicomImage(&dfile, xfer, compabilityMode);

        if (di->getStatus() != EIS_Normal)
        {
            emit resultReady(tr("Failed: %1, %2.").arg(file, QString::fromLocal8Bit(DicomImage::getString(di->getStatus()))));
            delete di;
            continue;
        }

        di->setWindow(0);

        /* write selected frame(s) to file */
        int result = 0;
        FILE *ofile = NULL;
        QString ext;
        /* determine default file extension */
        switch (fileType) {
        case EFT_BMP:
        case EFT_8bitBMP:
        case EFT_24bitBMP:
        case EFT_32bitBMP:
            ext = "bmp";
            break;
        case EFT_JPEG:
            ext = "jpg";
            break;
        case EFT_TIFF:
            ext = "tif";
            break;
        case EFT_PNG:
            ext = "png";
            break;
        default:
            if (di->isMonochrome()) ext = "pgm"; else ext = "ppm";
            break;
        }

        for (Sint32 frame = 0; frame < frameCount; frame++)
        {
            /* output to file */
            if (file.right(4) == QString(".dcm")) file = file.mid(0, file.lastIndexOf('.'));
            QString filepath = QString("%1/%2%3.%4").arg(destDir,
                                                         file,
                                                         frameCount>1?QString::number(frame):QString(),
                                                         ext);
            QDir().mkpath(filepath.mid(0, filepath.lastIndexOf('/')));

            ofile = fopen(filepath.toLocal8Bit().data(), "wb");
            if (ofile == NULL)
            {
                emit resultReady(tr("Failed: %1, open dest file failed.").arg(file));
                continue;
            }

            /* finally create output image file */

            switch (fileType)
            {
                case EFT_RawPNM:
                    result = di->writeRawPPM(ofile, 8, frame);
                    break;
                case EFT_8bitPNM:
                    result = di->writePPM(ofile, 8, frame);
                    break;
                case EFT_16bitPNM:
                    result = di->writePPM(ofile, 16, frame);
                    break;
                case EFT_NbitPNM:
                    //result = di->writePPM(ofile, OFstatic_cast(int, fileBits), frame);
                    break;
                case EFT_BMP:
                    result = di->writeBMP(ofile, 0, frame);
                    break;
                case EFT_8bitBMP:
                    result = di->writeBMP(ofile, 8, frame);
                    break;
                case EFT_24bitBMP:
                    result = di->writeBMP(ofile, 24, frame);
                    break;
                case EFT_32bitBMP:
                    result = di->writeBMP(ofile, 32, frame);
                    break;
    #ifdef BUILD_DCM2PNM_AS_DCMJ2PNM
                case EFT_JPEG:
                    {
                        /* initialize JPEG plugin */
                        DiJPEGPlugin plugin;
                        plugin.setQuality(jpegQuality);
                        plugin.setSampling(ESS_422);
                        result = di->writePluginFormat(&plugin, ofile, frame);
                    }
                    break;
    #endif
    #ifdef WITH_LIBTIFF
                case EFT_TIFF:
                    {
                        /* initialize TIFF plugin */
                        DiTIFFPlugin tiffPlugin;
                        tiffPlugin.setCompressionType(opt_tiffCompression);
                        tiffPlugin.setLZWPredictor(opt_lzwPredictor);
                        tiffPlugin.setRowsPerStrip(OFstatic_cast(unsigned long, opt_rowsPerStrip));
                        result = di->writePluginFormat(&tiffPlugin, ofile, frame);
                    }
                    break;
    #endif
    #ifdef WITH_LIBPNG
                case EFT_PNG:
                    {
                        /* initialize PNG plugin */
                        DiPNGPlugin pngPlugin;
                        pngPlugin.setInterlaceType(opt_interlace);
                        pngPlugin.setMetainfoType(opt_metainfo);
                        result = di->writePluginFormat(&pngPlugin, ofile, frame);
                    }
                    break;
    #endif
    #ifdef PASTEL_COLOR_OUTPUT
                    case EFT_PastelPNM:
                        result = di->writePPM(ofile, MI_PastelColor, frame);
                        break;
    #endif
            }

            if (result) {
                emit resultReady(tr("Exported: %1.").arg(file));
            } else {
                emit resultReady(tr("Failed: %1, conversion failed.").arg(file));
            }

            fclose(ofile);
        }

        delete di;
    }

    // deregister RLE decompression codec
    DcmRLEDecoderRegistration::cleanup();
#ifdef BUILD_DCM2PNM_AS_DCMJ2PNM
    // deregister JPEG decompression codecs
    DJDecoderRegistration::cleanup();
#endif
#ifdef BUILD_DCM2PNM_AS_DCML2PNM
    // deregister JPEG-LS decompression codecs
    DJLSDecoderRegistration::cleanup();
#endif
}
