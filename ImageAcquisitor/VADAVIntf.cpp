/** *************************************************************** **
 **          Radiography Detector: application interface            **
 **  (c) VATech                                         by S.B.     **
 ** *************************************************************** **/

#include "windows.h"
#include "tchar.h"
#include "VADAVIntf.h"
#include "stdio.h"

static void iErrorMsg( const TCHAR *AMsg, int AErrCode )
{
  TCHAR qMsg[256], qErrDescr[180];
  _tcscpy( qMsg, AMsg );
  if (AErrCode) {
    _tcscat( qMsg, _T("\n") );
    if (FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, AErrCode, 0,
        qErrDescr, sizeof(qErrDescr)-1, NULL )<=0)
      _stprintf( qErrDescr, _T("#%d"), AErrCode );
    _tcscat( qMsg, qErrDescr );
  }; // if AErrCode
  MessageBox( NULL, qMsg, _T("X-ray detector library's load error"), MB_OK|MB_ICONSTOP );
}

static BOOL iGetPA( HINSTANCE AHDll, const TCHAR *AFuncName, FARPROC *AFuncRef )
{
  #ifdef UNICODE
    char qFuncName[80];
    WideCharToMultiByte( CP_ACP, 0, AFuncName, -1, qFuncName, 79, NULL, NULL );
    *AFuncRef = GetProcAddress( AHDll, qFuncName );
  #else
    *AFuncRef = GetProcAddress( AHDll, AFuncName );
  #endif
  if (*AFuncRef) 
    return TRUE;
  TCHAR qErrMsg[256];
  _tcscpy( qErrMsg, _T("Can't find exported function \"") );
  _tcscat( qErrMsg, AFuncName ); _tcscat( qErrMsg, _T("\"") );
  iErrorMsg( qErrMsg, 0 );
  return FALSE;
}

BOOL VADAV_MapDLL( const TCHAR *ADllFName, tVADAV_InterfaceRec *AIntfRec )
{
  memset( AIntfRec, 0, sizeof(tVADAV_InterfaceRec) );
  // define library file name
  TCHAR  qDllFName[256];
  if (ADllFName && ADllFName!=(const TCHAR*)-1)
    _tcscpy( qDllFName, ADllFName );
  else {
    // replace module name with 'VADAV.dll'
    GetModuleFileName( NULL, qDllFName, sizeof(qDllFName)/sizeof(TCHAR) );
    if (!ADllFName) _tcscpy( _tcsrchr(qDllFName,_T('\\'))+1, _T("VADAV.dll") );
               else _tcscpy( _tcsrchr(qDllFName,_T('.'))+1, _T("dll") );
    if (GetFileAttributes(qDllFName)==-1) // search the DLL in ./
      _tcscpy( qDllFName, _tcsrchr(qDllFName,_T('\\'))+1 );
  }; // if ADllName
  // load library
  AIntfRec->rHDLL = LoadLibrary( qDllFName );
  if (!AIntfRec->rHDLL) {
    iErrorMsg( qDllFName, GetLastError() );
    return FALSE;
  };
  BOOL   qOk = FALSE;     
  // map imported functions
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_DialogH"), (FARPROC*)&AIntfRec->rVD_Dialog ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_LogOpen"), (FARPROC*)&AIntfRec->rVD_LogOpen ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_LogMsg"), (FARPROC*)&AIntfRec->rVD_LogMsg ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_LogClose"), (FARPROC*)&AIntfRec->rVD_LogClose ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_LogFlush"), (FARPROC*)&AIntfRec->rVD_LogFlush ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_GetHomeDirectory"), (FARPROC*)&AIntfRec->rVD_GetHomeDirectory ))
    goto labLeave;
/*
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_IniProfSetSection"), (FARPROC*)&AIntfRec->rVD_IniProfSetSection ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_IniProfGetStr"), (FARPROC*)&AIntfRec->rVD_IniProfGetStr ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_VendorCommand"), (FARPROC*)&AIntfRec->rVDACQ_VendorCommand ))
    goto labLeave;
*/
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_GetDetectorIPAddr"), (FARPROC*)&AIntfRec->rVDACQ_GetDetectorIPAddr))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_SetDetectorIPAddr"), (FARPROC*)&AIntfRec->rVDACQ_SetDetectorIPAddr))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_SetFrameDim"), (FARPROC*)&AIntfRec->rVDACQ_SetFrameDim ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_GetFrameDim"), (FARPROC*)&AIntfRec->rVDACQ_GetFrameDim ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_Connect"), (FARPROC*)&AIntfRec->rVDACQ_Connect ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_SendCommand"), (FARPROC*)&AIntfRec->rVDACQ_SendCommand ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_SendCommandParam"), (FARPROC*)&AIntfRec->rVDACQ_SendCommandParam ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_StartFrame"), (FARPROC*)&AIntfRec->rVDACQ_StartFrame ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_GetFrame"), (FARPROC*)&AIntfRec->rVDACQ_GetFrame ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_GetDetectorInfo"), (FARPROC*)&AIntfRec->rVDACQ_GetDetectorInfo ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDDBG_SendCommand"), (FARPROC*)&AIntfRec->rVDDBG_SendCommand ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_Abort"), (FARPROC*)&AIntfRec->rVDACQ_Abort ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDACQ_Close"), (FARPROC*)&AIntfRec->rVDACQ_Close ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_GetImageDim"), (FARPROC*)&AIntfRec->rVDC_GetImageDim ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_CutImage"), (FARPROC*)&AIntfRec->rVDC_CutImage ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_Process"), (FARPROC*)&AIntfRec->rVDC_Process ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_Abort"), (FARPROC*)&AIntfRec->rVDC_Abort ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_Close"), (FARPROC*)&AIntfRec->rVDC_Close ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_GetCalibrationDirectory"), (FARPROC*)&AIntfRec->rVDC_GetCalibrationDirectory ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_SetCalibrationDirectory"), (FARPROC*)&AIntfRec->rVDC_SetCalibrationDirectory ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_GetImgCutParams"), (FARPROC*)&AIntfRec->rVDC_GetImgCutParams ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_SetImgCutParams"), (FARPROC*)&AIntfRec->rVDC_SetImgCutParams ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_GenerateAuto"), (FARPROC*)&AIntfRec->rVDC_GenerateAuto ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_GetDark"), (FARPROC*)&AIntfRec->rVDC_GetDark ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_GetBright"), (FARPROC*)&AIntfRec->rVDC_GetBright ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDC_GenerateBright"), (FARPROC*)&AIntfRec->rVDC_GenerateBright ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDIP_Process"), (FARPROC*)&AIntfRec->rVDIP_Process ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDIP_Abort"), (FARPROC*)&AIntfRec->rVDIP_Abort ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VDIP_Close"), (FARPROC*)&AIntfRec->rVDIP_Close ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_Set_Acquisition"), (FARPROC*)&AIntfRec->rVD_Set_Acquisition ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_Set_Calibration"), (FARPROC*)&AIntfRec->rVD_Set_Calibration ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_Set_ImgProcess"), (FARPROC*)&AIntfRec->rVD_Set_ImgProcess ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_GetImage"), (FARPROC*)&AIntfRec->rVD_GetImage ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_GetImageSP"), (FARPROC*)&AIntfRec->rVD_GetImageSP ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_GetImageCancel"), (FARPROC*)&AIntfRec->rVD_GetImageCancel ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_Get_Calibration"), (FARPROC*)&AIntfRec->rVD_Get_Calibration ))
    goto labLeave;
  if (!iGetPA( AIntfRec->rHDLL, _T("VD_ConnectRestore"), (FARPROC*)&AIntfRec->rVD_ConnectRestore ))
    goto labLeave;

  // success
  qOk = TRUE;
  // leave
  labLeave:
  if (!qOk)
    VADAV_ReleaseDLL( AIntfRec );
  return qOk;
}

void VADAV_ReleaseDLL( tVADAV_InterfaceRec *AIntfRec )
{
  if (AIntfRec->rHDLL) {
    FreeLibrary( AIntfRec->rHDLL );
    AIntfRec->rHDLL = NULL;
  };
}
