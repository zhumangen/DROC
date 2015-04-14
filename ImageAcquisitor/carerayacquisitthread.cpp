#include "carerayacquisitthread.h"

#include "Windows.h"
#include "direct.h"
#include "structures.h"
#include "errors.h"
#include "crapi_dll.h"
#include "../MainStation/logdbmanager.h"

CareRayAcquisitThread::CareRayAcquisitThread(QObject *parent) :
    _width(0),
    _height(0),
    _imgBuf(0),
    _abort(0),
    QThread(parent)
{
}

void CareRayAcquisitThread::run()
{
    int ret;
    QString errStr;
    _width = 0;
    _height = 0;
    delete[] _imgBuf;
    _imgBuf = 0;

    emit detectorMsg(tr("Set detector to normal power."));
    ret = CR_set_normal_power();
    if (CR_ALREADY_DISCONN_ERR == ret) {
        char path[256];
        _getcwd(path, sizeof(path));
        strcat_s(path, "\\CareRay\\");

        ret = CR_connect_detector(path);
        if (CR_NO_ERR != ret) {
            errStr = tr("Detector connect failed: %1.").arg(CrErrStrList(ret));
            LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
            emit detectorMsg(errStr);
            return;
        }

        ret = CR_set_normal_power();
    }

    if (CR_NO_ERR != ret) {
        errStr = tr("Set detector to normal power failed: %1.").arg(CrErrStrList(ret));
        emit detectorMsg(errStr);
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
        return;
    }

    emit detectorMsg(tr("Starting acquisition..."));
    ret = CR_start_acq_full_image();
    if (CR_NO_ERR != ret) {
        errStr = tr("Starting acquisition failed: %1.").arg(CrErrStrList(ret));
        emit detectorMsg(errStr);
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
        return;
    }

    ExpProgress expProg;
    memset(&expProg, 0x0, sizeof(ExpProgress));
    emit detectorMsg(tr("Querying exposure progress..."));
    ret = CR_query_prog_info(CR_RAD_PROG, &expProg);
    if (CR_NO_ERR != ret) {
        errStr = tr("Query exposure progress failed: %1.").arg(CrErrStrList(ret));
        emit detectorMsg(errStr);
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
        return;
    }

    int ready = 0;
    int permit = 0;
    while (CR_NO_ERR == ret) {
        switch (expProg.expStatus) {
        case CR_EXP_READY:
            if (ready == 0) {
                emit exposureReady(true);
                emit detectorMsg(tr("Detector is ready for X-Ray exposure."));
                ready++;
            }
            break;
        case CR_EXP_WAIT_PERMISSION:
            if (permit == 0) {
                ret = CR_permit_exposure();
                if (CR_NO_ERR != ret) {
                    errStr = tr("Permit exposure failed: %1.").arg(CrErrStrList(ret));
                    emit detectorMsg(errStr);
                    LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
                    return;
                }
                permit++;
            }

            break;
        }

        if (expProg.fetchable || _abort) break;

        Sleep(50);
        ret = CR_query_prog_info(CR_RAD_PROG, &expProg);
    }

    emit exposureReady(false);
    emit detectorMsg(tr("Set detector saving power."));
    if (_abort) CR_stop_acq_frame();

    ret = CR_set_save_power();
    if (CR_NO_ERR != ret) {
        errStr = tr("Set detector saving power failed: %1.").arg(CrErrStrList(ret));
        emit detectorMsg(errStr);
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
    }

    int count = 3;
    while (count && (!_abort)) {
        emit detectorMsg(tr("Getting frame attribute..."));

        FrameAttr fattr;
        ret = CR_get_image_attr(&fattr);
        if (CR_NO_ERR != ret) {
            errStr = tr("Get frame attribute failed: %1.").arg(CrErrStrList(ret));
            emit detectorMsg(errStr);
            LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
            return;
        }

        _width = fattr.image_width;
        _height = fattr.image_height;
        int imageSize = _width * _height;
        delete[] _imgBuf;
        _imgBuf = new short[imageSize];

        ret = CR_get_image(imageSize*2, false, (void*)_imgBuf);
        if (CR_NO_ERR != ret) {
            errStr = tr("Get image failed: %1.").arg(CrErrStrList(ret));
            emit detectorMsg(errStr);
            LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
            if (count) {
                emit detectorMsg(tr("Retry getting image..."));
                count--;
                continue;
            } else {
                CR_stop_acq_frame_cleanup();
                delete[] _imgBuf;
                _imgBuf = 0;
                return;
            }
        }

        //ret = CR_stop_acq_frame();

        //remove grid code goes here...

        emit detectorMsg(tr("Correcting bad pixels..."));
        ret = CR_inpaint_bad_pixels((WORD*)_imgBuf);
        if (CR_NO_ERR != ret) {
            errStr = tr("Bad pixel correcting failed: %1.").arg(CrErrStrList(ret));
            emit detectorMsg(errStr);
            LogDbManager::insertMessageToDb(LogDbManager::ET_Error, errStr);
            delete[] _imgBuf;
            _imgBuf = 0;
            return;
        }

        emit detectorMsg(tr("Receiving image completed."));
        break;
    }
}
