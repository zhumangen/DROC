/*
 *  error.h
 *	Authors:
 *	Liu Jieqing		<jqing.liu@careray.com>	
 *  This file contains the error code and its comment string     
 * 2009-12-14   Liu Jieqing file created.
 * Modification history:
 * 2013-01-21 modified
 */

#ifndef __ERROR_H__
#define __ERROR_H__

#define API_INDEX 1000

enum {
	CR_NO_ERR = 0,
	ERR_DRIVER,
	ERR_OPEN_DETECTOR_FAILED,
	ERR_MMAP_SPACE,
	ERR_MEM_ALLOC,
	ERR_NO_SD_SPACE,		//5
	ERR_NO_FLASH_SPACE,
	ERR_FILE_NAME,
	ERR_FILE_PATH,
	ERR_FILE_READ,
	ERR_FILE_WRITE,		//10
	ERR_XMLFILE,
	ERR_CODER_IDX,		//undefined command ID at codec
	ERR_CODER_VER,		//undefined parser version
	ERR_CODER_PRM,	//command param error
	ERR_IMAGE_PIPE_BROKEN,		//15
	ERR_LOGIC,
	ERR_NO_MODE_SELECTED,
	//ERR_ENCODE_COMM = 1,
	//ERR_DECODE_COMM = 2,
	ERR_SET_DTTVER,
	ERR_UNDEFINED_COMMID,	
	ERR_UNDEFINED_MODE,		//20
	ERR_HANDSWITCH_STATE,
	ERR_SET_DTTTIME, 
	ERR_ACCESS_FPGA,	
	ERR_UPFILE,
	ERR_BAD_PARAM, 	//25
	ERR_BAD_STATE,		//forbid to change mode at current transfer state.
	ERR_BAD_ACTION_IN_CURRENT_MODE,		//
	ERR_LICENSE,
	ERR_SYSTEM_SELFTEST,
	ERR_IOCTL_FAILED,	//30
	ERR_TEST_FRAMES,
	ERR_EXCHANGE_CFGFILE,
	ERR_UNAVAILABLE_OFFSET,		//
	ERR_CREATE_THREAD,
	ERR_READOUT,		//35
	ERR_INVAILD_FEATHER,
	ERR_GET_CORR_IMGCRC, //37
	ERR_SOFTWARE_HAS_EXPIRED,  //38
	ERR_FETCHABLE_NOT_TRUE,
	ERR_INVALID_FE_FRMNUMS,
	ERR_READ_FRAME_ATTR,
	ERR_INVALID_ACQ_OPTION,
	ERR_RDFRM_SQQ_IS_EMPTY,
	ERR_UNAVAILABLE_SYNCMODE,
	ERR_UNAVAILABLE_ACQMODE,

	ERR_REALTIME_DARK_SET,
	ERR_UNDEFINED_CONFIGFILE,
	ERR_NO_CONFIGFILE,
	ERR_FILE_CREATE,
	ERR_FILE_CRC,
	ERR_EXIST_CONFIGFILE,
	ERR_FILE_OPEN,

	//---------Socket Layer--------------
	//Socket layer error most happens in bad network state
	CR_CONN_DETECTOR_ERR = API_INDEX,
	CR_DISCONN_DETECTOR_ERR,
	CR_ALREADY_CONN_ERR,
	CR_ALREADY_DISCONN_ERR,
	CR_RECONN_ERROR,
	CR_CLOSE_SOCK_ERROR,
	CR_INIT_ERR,
	CR_CREATE_ERR,
	CR_GETOPT_ERR,
	CR_SETOPT_ERR,
	CR_SEND_ERR,
	CR_RECV_ERR,
	CR_CLOSE_ERR,
	CR_SET_VER_ERR,
	//---------Codec Layer--------------
	CR_COMID_ERR,
	CR_VER_ERR,
	CR_PARAM_ERR,
	//---------V_API Layer--------------
	CR_NULL_BUFFER_ERR,
	CR_READ_CONFIGFILE_ERR,
	CR_WRITE_CONFIGFILE_ERR,
	CR_USER_PARAM_ERR,
	CR_MODE_UNSELECT_ERR,
	CR_LOAD_DLL_ERR,
	CR_FREE_DLL_ERR,
	CR_CREATE_EVENT_ERR,
	CR_CLOSE_EVENT_ERR,
	CR_FUNCID_ERR,
	CR_GET_FUNC_ADDR_ERR,
	CR_CREATE_VTHREAD_ERR,
	CR_CREATE_RTTHREAD_ERR,
	CR_CREATE_HBTHREAD_ERR,
	CR_CREATE_OFSTTHREAD_ERR,
	CR_CREATE_CALTHREAD_ERR,
	CR_CREATE_LOGTHREAD_ERR,
	CR_CLOSE_THREAD_ERR,
	CR_ALCATE_BUFF_ERR,
	CR_VTHREAD_BUSY_ERR,
	CR_GET_IMAGE_ERR,
	CR_FILE_PATH_ERR,
	CR_OPEN_FILE_ERR,
	CR_READ_FILE_ERR,
	CR_WRITE_FILE_ERR,
	CR_EXCHANGEFILE_ERR,
	CR_GRAB_IN_PROCESS_ERR,
	CR_GRAB_NOT_WORK_ERR,
	CR_REC_IN_PROCESS_ERR,
	CR_REC_NOT_WORK_ERR,
	CR_INDEX_OUT_BOUNDARY_ERR,
	CR_GRAB_IMG_ERR,
	CR_REC_IMG_ERR,
	CR_RECV_IMGHEAD_ERR,
	CR_MODE_COMM_NOT_MATCH_ERR,
	CR_FLU_PARAM_UNSET_ERR,
	CR_CAL_IN_PROCESS_ERR,
	CR_OFFSET_THREAD_BUSY_ERR,
	CR_OFFSET_THREAD_STOP_ERR,
	CR_CAL_INTERUPT_ERR,
	CR_STRING_EMPTY,
	CR_GAIN_UNSET,
	CR_MACHINEID_EMPTY,
	//image self test errors
	CR_IMAGE_AVG_ERR,
	CR_IMAGE_STD_ERRO,
	CR_IMAGE_UNIFORM_ERR,
	CR_IMAGE_STRIP_ERR,
	CR_IMAGE_RCN_ERR,//行噪声分析错误
	CR_IMAGE_LINEAR_ERR,//图像线性度错误
	CR_IMAGE_DARK_GRAY_ERR,
	CR_IMAGE_SYNC_ERR,	//图像是未同步下的
	CR_IMAGE_COLLIMATOR_ERR,//束光器设置错误
	CR_IMAGE_TUBE_ERR,//球管未对齐
	//Calibration parameters errors in application's config files
	CR_OFFSETCAL_NUM_ERR,
	CR_LINEAR_DOSENUM_ERR,
	CR_LINEAR_NUM_PERDOSE_ERR,
	CR_PORTABLEKV_ERR,

	CR_ADDPIXEL_MACHINE_ERR,
	CR_SEND_PCFILE_OLD_ERR,
	CR_SEND_PCFILE_MACHINEID_ERR,
	CR_CALIBRATION_FILE_NULL_ERR
};

//Errors in detector, the indexes start from 0
//0 means call success
static const char *CrErrStrList_dtt[] = {
	"No Error",

	"Load device driver failed",
	"Open detector failed",
	"Map memory  failed",
	"Allocate memory failed",
	"No enough space on SD card",		//5
	"No enough Space on Flash",
	"File name is invalid, failed to exchange",
	"File path is Invalid, failed to exchange",
	"Read file error",
	"Write file error",				//10
	"Bad XML file",
	"Command can not be recognized",
	"Software version does not match",
	"Incorrect number of parameters",
	"Send frame error, transmission pipeline closed",		//15
	"Execution Logic of API goes wrong",
	"Have no select mode",
	"Setting detector version failed",
	"Undefined commandID",
	"Undefined mode",				//20
	"HandSwitch in error state", 
	"setting detector time failed",
	"access fpga register failed",
	"update file failed",
	"Invalid parameter",				//25
	"Forbid to change mode at current transfer state",
	"Forbid this operation at current transfer state",
	"Invalid license, Please contact this detector manufacturer",
	"Error happens in the self-test phase",
	"ioctl calling failed",	//30
	"frame stability test failed",
	"Exchange config files error",
	"Offset is unavailable",
	"Creat thread failed",
	"Error happens during readout",	//35
	"Function is not supported at present",
	"Read image CRC failed",
	"Software has expired",
	"has no fetchable frame",
	"read frame number from FE is invalid",
	"read frame attribution failed",
	"invalid acquisition option",
	"read-frame sequence is empty",
	"Current detector doesn't support this sync mode",
	"Current detector doesn't support this acquisition mode",

	"Real time Dark setting error",
	"Undefined config file",
	"Configure file not exist",
	"Error to create file",
	"CRC Check error",
	"Configure file already exist",
	"Fail to open file"
};

//Errors in API, the indexes start from API_INDEX
static const char *CrErrStrList_pc[] = {
	//---------Socket Layer--------------
	"Establish connection failed, network blocked",	//1000
	"Disconnect detector error",
	"Already connect",
	"Already disconnect",
	"Reconnect socket error",
	"Close socket error",
	"Initiate winsock error",
	"Create socket error",
	"Get socket option error",
	"Set socket option error",
	"Send data error",	//1010
	"Recv data error",
	"Recv data connection close error",
	"Set version error",
	//---------Codec Layer--------------
	"Command id error",
	"Version error",
	"Parameter error",
	//---------V_API Layer--------------
	"Buffer is NULL error",
	"Read config file error",
	"Write config file error",
	"User Parameter error",		//1020
	"Mode unselected error",
	"Load dll error",
	"Free dll error",
	"Create event error",
	"Close event error",
	"Function id not match error",
	"Get function address error",
	"Create video thread error",
	"Create real time thread error",
	"Create heart beat thread error",
	"Create offset thread error",
	"Create calibration thread error",
	"Create log thread error",
	"Close thread error",
	"Allocate buffer error",
	"Video thread busy error",
	"Get image fail",
	"Software path contains Chinese characters",	//1038
	"There are no calibration files",
	"Read file error",
	"Write file error",
	"Exchange file with the detector error",
	"Grabbing is in process error",
	"Grabbing is already stop error",
	"Recording is in process error",
	"Recording is already stop error",
	"Buffer index out of boundary",
	"Grab image error",
	"Record image error",
	"Receive image head error",	//1050
	"Command mode unmatch error, user should select check mode",
	"Fluoro mode parameters unset",	
	"Calibration is in process",
	"Offset thread busy error",
	"Offset thread stop error",
	"Calibration interupt error",
	"Empty string or array error",
	"Gain image not specify yet",

	"Machine ID is NULL",

	"Image mean value below standard",	//1060
	"Image std value below standard",
	"Image uniform below standard",
	"Image strip mean below standard",
	"Image RCN below standard",
	"Image linear below standard",
	"Image dark gray below standard",
	"Detector not synchronization",
	"Collimator error",
	"Tube alignment error",

	"Offset calibration's image number should > 1",	//1070
	"Gain calibration's linear dose number should between 5 and 9",
	"Gain calibration's linear number per dose should > 1",
	"Portable KV string format error",
	"Machine id not the same when add bad pixels",
	"File in PC is older than detector",
	"File's machine id is not the current detector",
	"There are no calibration files, user should calibrate first"
};

static const char *CrErrStrList(int index)
{
	if(index < API_INDEX)
	{
		return CrErrStrList_dtt[index];
	}else if(index >= API_INDEX)
	{
		return CrErrStrList_pc[index-API_INDEX];
	}else
	{
		return "";
	}
}

#endif // __ERROR_H__