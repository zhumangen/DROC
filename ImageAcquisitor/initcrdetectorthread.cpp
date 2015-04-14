#include "initcrdetectorthread.h"

#include "Windows.h"
#include "direct.h"
#include "structures.h"
#include "errors.h"
#include "crapi_dll.h"
#include "../MainStation/logdbmanager.h"

InitCrDetectorThread::InitCrDetectorThread(QObject *parent) :
    QThread(parent)
{
}

void InitCrDetectorThread::run()
{
    int ret;
    char path[256];
    _getcwd(path, sizeof(path));
    strcat_s(path, "\\CareRay\\");
    QString msg;

    ret = CR_connect_detector(path);
    if (CR_NO_ERR != ret) {
        msg = tr("Detector connect failed: %1.").arg(CrErrStrList(ret));
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, msg);
        emit progressMsg(msg);
        return;
    }
    emit progressMsg(tr("Detector connected."));

    ret = CR_reset_detector(false);
    if (CR_NO_ERR != ret) {
        msg = tr("Reset detector failed: %1").arg(CrErrStrList(ret));
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, msg);
        emit progressMsg(msg);
        return;
    }
    emit progressMsg(tr("Reset detector succeeded."));

    ret = CR_set_check_mode(MODE_RAD);
    if (CR_NO_ERR != ret) {
        msg = tr("Set detector to RAD mode failed: %1").arg(CrErrStrList(ret));
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, msg);
        emit progressMsg(msg);
        return;
    }
    emit progressMsg(tr("Set detector to RAD mode."));

    ret = CR_set_sync_mode(EXT_SYNC);
    if (CR_NO_ERR != ret) {
        msg = tr("Set detector to EXT_SYNC mode failed: %1").arg(CrErrStrList(ret));
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, msg);
        emit progressMsg(msg);
        return;
    }
    emit progressMsg(tr("Set detector to EXT_SYNC mode."));

    ret = CR_set_cycle_time(1000, 100, 5);
    if (CR_NO_ERR != ret) {
        msg = tr("Set cycle time to exp:1000, delay:100, wait:5 failed: %1").arg(CrErrStrList(ret));
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, msg);
        emit progressMsg(msg);
        return;
    }
    emit progressMsg(tr("Set cycle time to exp:1000, delay:100, wait:5."));

    UserCorrection corr;
    memset(&corr, 0x0, sizeof(UserCorrection));
    corr.fixedCorr = true;
    ret = CR_set_user_correction(&corr);
    if (CR_NO_ERR != ret) {
        msg = tr("Set user correction to FixedCorrection failed: %1").arg(CrErrStrList(ret));
        LogDbManager::insertMessageToDb(LogDbManager::ET_Error, msg);
        emit progressMsg(msg);
        return;
    }
    emit progressMsg(tr("Set user correction to FixedCorrection."));
}
