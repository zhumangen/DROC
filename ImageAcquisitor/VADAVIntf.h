/** *************************************************************** **
 **          Radiography Detector: application interface            **
 **  (c) VATech                                         by S.B.     **
 **  initial release: July-2007                                     **
 **  recent edition : Sept-2008                                     **
 ** *************************************************************** **/

#ifndef VADAVINTF_H_INCLUDED
#define VADAVINTF_H_INCLUDED
  ////// acquisition //////
  // acquisition flags
  #define cVDACQ_FDark         0       // acquire Dark frame (dark otherwise)
  #define cVDACQ_FBright       1       // acquire bright frame (dark otherwise)
  #define cVDACQ_FTestPat      2       // acquire test pattern  /TFT only
  #define cVDACQ_FRecentFrame  4       // retrieve recent frame /TFT only
  #define cVDACQ_FBrightIgn   (1<<31)  // ignore state of the bit 'Bright' /TFT only
  //  #define cVDACQ_FI_Bright61  (1<<31)  // 'bright61' /USB only
  // acquisition event types
  #define cVDACQ_ETTrace       0
  #define cVDACQ_ETTraceT      1
  #define cVDACQ_ETErr         2
  #define cVDACQ_ETLastErr     3
  #define cVDACQ_ETWSAErr      4
  // acquisition events
  #define cVDACQ_EAbort        1
  #define cVDACQ_EClose        2
  #define cVDACQ_ECapture      20
  #define cVDACQ_ECapturePerc  21
  #define cVDACQ_ECaptureRecv  22

  // acquisition feedback record   
  typedef struct {
    int    rFlags,            // combination of cVDACQ_Fxxxx
           rType,             // cVDACQ_ETxxx
           rEvent,            // cVDACQ_Exxx
           rSocket;           // 0:no socket relation; otherwise socket's ID >0 
    TCHAR  rMsg[256];         // message (trace, wrn, err)
    int    rFrameWidth,       // full frame width
           rFrameHeight;      // full frame height
    short *rFrameBuffer;      // user supplied frame buffer "AFrameBuffer"
    int    rCaptureRows,      // # of received rows
           rCapturePercent;   // received data in percents
    void  *rUserCallBackProc, // user supplied "ACallBackProc"
          *rUserParam;        // user supplied "AUserParam"
    int    rAborted;          // 1: VDACQ_Abort -1:internally
    void  *rPacketData;       // pointer to received packet; usually it is nil  
  } tVDACQ_CallBackRec;
    // socket's id #1: control-write UDP
    //             #2: control-read UDP
    //             #3: data TCP socket
    // When callback is triggered by received message from detector it has non-nil field
    // PacketData. Normally an OEM application does not check received packets directly.
    
  // callback procedure  
  typedef void (_stdcall *tVDACQ_CallBackProc)( tVDACQ_CallBackRec* );

  ////// calibration //////
  // calibration flags
  #define cVDC_FCalOffs      1
  #define cVDC_FCalGain      2
  #define cVDC_FANoisRem     4
  #define cVDC_FBadPixMap    8
  #define cVDC_FDespeckle   16
  #define cVDC_FAVSmooth    32
  #define cVDC_FChkHWDark   (1<<30)  // check if recennt dark-frame is auto-calibrated
  #define cVDC_FTempDirIP   (1<<31)  // save temporary images in default image direcotory
  // calibration event types
  #define cVDC_ETTrace       0
  #define cVDC_ETTraceT      1
  #define cVDC_ETErr         2
  // calibration events
  #define cVDC_EAbort        1
  #define cVDC_EClose        2
  #define cVDC_ECalib        20
  #define cVDC_ECalibPerc    21
  // calibration feedback record   
  typedef struct {
    int    rFlags,            // combination of cVDC_Fxxxx
           rType,             // cVDC_ETxxx
           rEvent;            // cVDC_Exxx
    TCHAR  rMsg[256];         // message (trace, wrn, err)
    int    rFrameWidth,       // full frame width
           rFrameHeight,      // full frame height
           rStoredWidth,      // stored image width
           rStoredHeight,     // stored image height
           rCalibPercent;     // processed data in %
    short *rFrameBuffer,      // received frame data
          *rImageBuffer;
    void  *rUserCallBackProc, // user supplied "ACallBackProc"
          *rUserParam;        // user supplied "AUserParam"
    int    rAborted;          // 1: set by VDC_Abort; -1:internally
  } tVDC_CallBackRec;

class ImageAcquisitWidget;
  typedef struct {         // Custom information record
	  int  rNumReceivedRows, // acquisition uses this field to print messages
		  rComplete;        // indicates that caller terminates (by some reason) and expects "close"
	  // the possible reasons are: 'complete', 'error' or 'abort'
	  int rCalMode;
      ImageAcquisitWidget *acquisitor;
      //CWnd *rhWnd;
  } tCallBackRec;

  // callback procedure  
  typedef void (_stdcall *tVDC_CallBackProc)( tVDC_CallBackRec* );

  typedef struct {
    int  rImgCutLeft,          // cut left after rotation & flip 
         rImgCutTop,           // cut top after rotation & flip
         rImgCutRight,         // cut right after rotation & flip
         rImgCutBottom,        // cut bottom after rotation & flip
         rRotation,            // 0:none, 1:90 deg CCW, 2:90 deg CW
         rFlip;                // bit #0: flip horz, bit #1: flip vert 
  } tVDC_ImgCut;

  ///// image process /////
  // img process flags
  #define cVDIP_FDespeckle    1
  #define cVDIP_FAVSmooth     2
  #define cVDIP_F3PCurve      4
  #define cVDIP_FUSMask       8
  // img process event types
  #define cVDIP_ETTrace       0
  #define cVDIP_ETTraceT      1
  #define cVDIP_ETErr         2
  // img process events
  #define cVDIP_EAbort        1
  #define cVDIP_EClose        2
  #define cVDIP_EEnh          20
  #define cVDIP_EEnhPerc      21
  // img process feedback record   
  typedef struct {
    int    rFlags,            // combination of cVDC_Fxxxx
           rType,             // cVDC_ETxxx
           rEvent;            // cVDC_Exxx
    TCHAR  rMsg[256];         // message (trace, wrn, err)
    int    rStoredWidth,      // stored image width
           rStoredHeight,     // stored image height
           rEnhPercent,       // processed data in %
           rModeNumber;       // img process mode
    short *rImageBuffer;      // image buffer
    void  *rUserCallBackProc, // user supplied "ACallBackProc"
          *rUserParam;        // user supplied "AUserParam"
    int    rAborted;          // 1: set by VDIP_Abort; -1:internally
  } tVDIP_CallBackRec;
  // callback procedure  
  typedef void (_stdcall *tVDIP_CallBackProc)( tVDIP_CallBackRec* );

  #ifdef INTERNAL_VATECH_DAVINCI_LIBRARY // procedures' declarations ('as is' in the DLL)

    // general UI and logs //
    // invokes general property sheet
    BOOL _stdcall VD_Dialog();
    // open log-file
    void _stdcall VD_LogOpen( const TCHAR *AHomeDir, int ANumLogFiles, const TCHAR *AFPrefix );
    // record string to log-file
    void _stdcall VD_LogMsg( const TCHAR *AMsg );
    // close log-file
    void _stdcall VD_LogClose();
    // flush log-file
    void _stdcall VD_LogFlush();
    // returns home directory
    void _stdcall VD_GetHomeDirectory( TCHAR *ADir );
    // set section name in library's initialization profile file
    void _stdcall VD_IniProfSetSection( const TCHAR *ASectName );
    // get string key from library's initialization profile
    const TCHAR* _stdcall VD_IniProfGetStr( const TCHAR *AKeyName, const TCHAR *ADefault );
    ////// acquisition //////
    // sets full frame dimension (should not be called for real acquisitin case)
    void _stdcall VDACQ_SetFrameDim( int AWidth, int AHeight );
    // returns full frame dimension
    void _stdcall VDACQ_GetFrameDim( int *AWidth, int *AHeight );
    // connects to detector
    tVDACQ_CallBackRec* _stdcall VDACQ_Connect( int AFlags,
      tVDACQ_CallBackProc AUserCallBackProc, void *AUserParam, short *AFrameBuffer, int AMode );
    // send simple auxiliary command (FFT only)
    BOOL _stdcall VDACQ_SendCommand( tVDACQ_CallBackRec *ACallBackRec, DWORD ACommand );
    BOOL _stdcall VDACQ_SendCommandParam( tVDACQ_CallBackRec *ACallBackRec, DWORD ACommand, const void *AData, int ADataSize );
    // vendor comamnd (USB only)
    BOOL _stdcall VDACQ_VendorCommand( tVDACQ_CallBackRec *ACallBackRec, int ARequest, int AValue, int AIndex );
    // starts acquisition
    BOOL _stdcall VDACQ_StartFrame( tVDACQ_CallBackRec *ACallBackRec );
	BOOL _stdcall VDACQ_GetFrame( tVDACQ_CallBackRec *ACallBackRec, int FrameNum );
	BOOL _stdcall VDDBG_SendCommand( int command, int address, int data1, int data2 );
    // aborts acquisition
    BOOL _stdcall VDACQ_Abort( tVDACQ_CallBackRec *ACallBackRec );
    // releases resources associated with ACallBackRec
    void _stdcall VDACQ_Close( tVDACQ_CallBackRec *ACallBackRec );
    // returns current detector's IP
    void _stdcall VDACQ_GetDetectorIPAddr( TCHAR *AIPAddr );
    // sets detector's IP
    void _stdcall VDACQ_SetDetectorIPAddr( const TCHAR *AIPAddr );
    BOOL _stdcall VDACQ_GetDetectorInfo( int Parameter );
    
    ////// calibration //////
    // returns image dimension; returns FALSE if all cut margins are zero
    BOOL _stdcall VDC_GetImageDim( int *AWidth, int *AHeight );
    // cuts (also rotates and flips optionally) frame buffer to image's dimension.
    short *_stdcall VDC_CutImage( const short *AFrameBuffer, short *AImageBuffer );
    // starts calibration
    tVDC_CallBackRec* _stdcall VDC_Process( int AFlags,
      tVDC_CallBackProc AUserCallBackProc, void *AUserParam, short *AFrameBuffer );
    // aborts calibration
    BOOL _stdcall VDC_Abort( tVDC_CallBackRec *ACallBackRec );
    // releases resources associated with ACallBackRec
    void _stdcall VDC_Close( tVDC_CallBackRec *ACallBackRec );
    // service: returns current calibration directory
    void _stdcall VDC_GetCalibrationDirectory( TCHAR *ADir );
    // service: set temporary calibration directory
    BOOL _stdcall VDC_SetCalibrationDirectory( const TCHAR *ADir );
    // service: returns image cut params
    void _stdcall VDC_GetImgCutParams( tVDC_ImgCut *ACutParams );
    // service: sets image cut params
    void _stdcall VDC_SetImgCutParams( const tVDC_ImgCut *ACutParams );
    
	//BOOL _stdcall VDC_GenerateAuto( const tBP_ADetRec *ABPR, const TCHAR *AFMaskDark, const TCHAR *AFMaskBright, const TCHAR *ABPMFName );
    BOOL _stdcall VDC_GenerateAuto();
	int _stdcall VDC_GetDark( tVDACQ_CallBackProc AUserACQCallBackProc, tVDC_CallBackProc AUserCALCallBackProc );
	int _stdcall VDC_GetBright( tVDACQ_CallBackProc AUserACQCallBackProc, tVDC_CallBackProc AUserCALCallBackProc );
	BOOL _stdcall VDC_GenerateBright();

    ///// image process /////
    // starts img process 
    tVDIP_CallBackRec* _stdcall VDIP_Process( int AFlags,
      tVDIP_CallBackProc AUserCallBackProc, void *AUserParam, short *AImageBuffer, int AModeNumber );
    // aborts img process 
    BOOL _stdcall VDIP_Abort( tVDIP_CallBackRec *ACallBackRec );
    // releases resources associated with ACallBackRec
    void _stdcall VDIP_Close( tVDIP_CallBackRec *ACallBackRec );

    //// user interface /////
    void _stdcall VD_Set_Acquisition( int AFlags, short *AFrameData, int AModeNumber, tVDACQ_CallBackProc AUserCallBackProc, void *AUserParam );
    void _stdcall VD_Set_Calibration( int AFlags, short *AFrameData, short *AImgData, tVDC_CallBackProc AUserCallBackProc, void *AUserParam );
	void _stdcall VD_Get_Calibration( int &AOfsCal, int &AGainCal, int &ABadPixMap, int &m_nCal_HWOfsCal );
    void _stdcall VD_Set_ImgProcess( int AFlags, short *AImgData, int AModeNumber, tVDIP_CallBackProc AUserCallBackProc, void *AUserParam );
    int _stdcall VD_GetImage( HWND AOwner );
    int _stdcall VD_GetImageSP( HWND AOwner, int AWndLeft, int AWndTop, int AWndWidth, int AWndHeight );
    void _stdcall VD_GetImageCancel();

	//// Auxiliary
	BOOL _stdcall VD_ConnectRestore();

  #else // for use in applications

    // table of exported methods
    typedef struct {
      HINSTANCE rHDLL; // the library's handle
      // general UI and logs //
      BOOL (_stdcall *rVD_Dialog)( HWND ); // it has additional parameter: owner's window
      void (_stdcall *rVD_LogOpen)( const TCHAR*, int, const TCHAR* );
      void (_stdcall *rVD_LogMsg)( const TCHAR* );
      void (_stdcall *rVD_LogClose)(void);
      void (_stdcall *rVD_LogFlush)(void);
      void (_stdcall *rVD_GetHomeDirectory)( TCHAR* );
      void (_stdcall *rVD_IniProfSetSection)( const TCHAR* );
      const TCHAR* (_stdcall *rVD_IniProfGetStr)( const TCHAR*, const TCHAR* );
      ////// acquisition //////
      void (_stdcall *rVDACQ_SetFrameDim)( int, int );
      void (_stdcall *rVDACQ_GetFrameDim)( int*, int* );
      tVDACQ_CallBackRec* (_stdcall *rVDACQ_Connect)( int, tVDACQ_CallBackProc, void*, short*, int );
      BOOL (_stdcall *rVDACQ_SendCommand)( tVDACQ_CallBackRec*, DWORD );
      BOOL (_stdcall *rVDACQ_SendCommandParam)( tVDACQ_CallBackRec*, DWORD, const void*, int );
      BOOL (_stdcall *rVDACQ_StartFrame)( tVDACQ_CallBackRec* );
	  BOOL (_stdcall *rVDACQ_GetFrame)( tVDACQ_CallBackRec*, int ); 	  //// 1417P Debug Mode ////
      BOOL (_stdcall *rVDACQ_VendorCommand)( tVDACQ_CallBackRec*, int, int, int );
	  BOOL (_stdcall *rVDDBG_SendCommand)( int , int , int , int );
      BOOL (_stdcall *rVDACQ_Abort)( tVDACQ_CallBackRec* );
      void (_stdcall *rVDACQ_Close)( tVDACQ_CallBackRec* );
      void (_stdcall *rVDACQ_GetDetectorIPAddr)( TCHAR* );
      void (_stdcall *rVDACQ_SetDetectorIPAddr)( const TCHAR* );
      BOOL (_stdcall *rVDACQ_GetDetectorInfo)( int );
      ////// calibration //////
      BOOL (_stdcall *rVDC_GetImageDim)( int*, int* );
      short* (_stdcall *rVDC_CutImage)( const short*, short* );
      tVDC_CallBackRec* (_stdcall *rVDC_Process)( int, tVDC_CallBackProc, void*, short* );
      BOOL (_stdcall *rVDC_Abort)( tVDC_CallBackRec* );
      void (_stdcall *rVDC_Close)( tVDC_CallBackRec* );
      void (_stdcall *rVDC_GetCalibrationDirectory)( TCHAR* );
      BOOL (_stdcall *rVDC_SetCalibrationDirectory)( const TCHAR* );
      void (_stdcall *rVDC_GetImgCutParams)( tVDC_ImgCut* );
      void (_stdcall *rVDC_SetImgCutParams)( const tVDC_ImgCut* );
	  BOOL (_stdcall *rVDC_GenerateAuto)( );
      int (_stdcall *rVDC_GetDark)( tVDACQ_CallBackProc, tVDC_CallBackProc);
      int (_stdcall *rVDC_GetBright)( tVDACQ_CallBackProc, tVDC_CallBackProc);
	  BOOL (_stdcall *rVDC_GenerateBright)( );
      ///// image process /////
      tVDIP_CallBackRec* (_stdcall *rVDIP_Process)( int, tVDIP_CallBackProc, void*, short*, int );
      BOOL (_stdcall *rVDIP_Abort)( tVDIP_CallBackRec* );
      void (_stdcall *rVDIP_Close)( tVDIP_CallBackRec* );
      //// user interface /////
      void (_stdcall *rVD_Set_Acquisition)( int, short*, int, tVDACQ_CallBackProc, void* );
      void (_stdcall *rVD_Set_Calibration)( int, short*, short*, tVDC_CallBackProc, void* );
	  void (_stdcall *rVD_Get_Calibration)( int &, int &, int &, int & );
      void (_stdcall *rVD_Set_ImgProcess)( int, short*, int, tVDIP_CallBackProc, void* );
      int (_stdcall *rVD_GetImage)( HWND );
      int (_stdcall *rVD_GetImageSP)( HWND, int, int, int, int );
      void (_stdcall *rVD_GetImageCancel)(void);
	  //// Auxiliary
	  BOOL (_stdcall *rVD_ConnectRestore)(void);
    } tVADAV_InterfaceRec; 

    BOOL VADAV_MapDLL( const TCHAR *ADllFName, tVADAV_InterfaceRec *AIntfRec );
    // calls LoadLibrary and fills "AIntfRec"
    void VADAV_ReleaseDLL( tVADAV_InterfaceRec *AIntfRec );
    // calls FreeLibrary

  #endif // for use in applications
#endif // ifdef VADAVINTF_H_INCLUDED

