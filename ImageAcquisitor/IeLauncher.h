/*******************************************************************/
/*                                                                 */
/*         Copyright (C) 2013 Nanjing JingTai, Inc.                        */
/*                   All Rights Reserved                           */
/*******************************************************************/
/******************************************************************************
* FILENAME  - IeLauncher.h
*   
* USAGE     - Function prototypes for library function specially for ShengZhen AnJian Medical SanSung Flat DR 
*
* REVISION HISTORY - 
******************************************************************************/ 

///@brief: Launch IeEnhancer
///@param[in] srcBuf: 需要增强的源图像的内存地址
///@param[in] destBuf: 增强后图像的存放内存地址
///@param[in] strRegion: 对应体位的配置文件所在目录，如"D:\\UserProgram\\8200\\config\\ChestAP.iee"，
///@                               则程序会到指定的目录下读取指定的配置文件
///@param[in] width: 需增强图像的宽度
///@param[in] height: 需增强图像的高度
///@param[in] bitDepth: 需增强图像的有效位数，依据客户的原始图像有效位数而定
///@usage: int ret = LaunchImageEnhancer_RAWnMem((void *)inBuf, (void *)outBuf, "D:\\UserProgram\\8200\\config\\ChestAP.iee", 3000, 3000, 14);
///@注意事项：对于有效位数8位的灰度图像，请使用short数据类型来存储像素值
extern "C" __declspec(dllexport) int __stdcall LaunchImageEnhancer_RAWnMem(const void * srcBuf,
																void * destBuf,
																const char * strRegion, 
																const int width,
																const int height,
																const int bitDepth);


