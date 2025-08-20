////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_ve_io.h
/// @brief  TVEncoder Driver Interface
/// @author MStar Semiconductor Inc.
///
/// I/O Control
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MDRV_VE_IO_H__
#define __MDRV_VE_IO_H__

#define MDRV_VE_IOC_MAGIC  '0'

#define MDRV_VE_SWITCH_OUTPUT_DES       _IOW(MDRV_VE_IOC_MAGIC,  0, MS_SWITCH_VE_DEST_INFO)
#define MDRV_VE_SET_OUTPUT_CTL          _IOW(MDRV_VE_IOC_MAGIC,  1, MS_VE_OUTPUT_CTRL)
#define MDRV_VE_SET_OUTPUT_VIDEO_STD    _IOW(MDRV_VE_IOC_MAGIC,  2, MS_VE_VIDEOSYS)
#define MDRV_VE_POWER_ON                _IO (MDRV_VE_IOC_MAGIC,  3)
#define MDRV_VE_POWER_OFF               _IO (MDRV_VE_IOC_MAGIC,  4)
#define MDRV_VE_INIT                    _IO (MDRV_VE_IOC_MAGIC,  5)
//#define MDRV_VE_SET_OUTPUT_TYPE         _IOW(MDRV_VE_IOC_MAGIC,  6, MS_VE_VE_OUTPUT_TYPE)
#define MDRV_VE_ENABLE                  _IOW(MDRV_VE_IOC_MAGIC,  6, BOOL)
#define MDRV_VE_RESET                   _IOW(MDRV_VE_IOC_MAGIC,  7, BOOL)
#define MDRV_VE_GEN_TEST_PATTERN        _IO (MDRV_VE_IOC_MAGIC,  8)
#define MDRV_VE_SWITCH_INPUTSRC         _IOW(MDRV_VE_IOC_MAGIC,  9, MS_SWITCH_VE_SRC_INFO)
#define MDRV_VE_SET_VPSDATA             _IOW(MDRV_VE_IOC_MAGIC, 10, stVE_VPSData)
#define MDRV_VE_SET_WSSDATA             _IOW(MDRV_VE_IOC_MAGIC, 11, stVE_WSSData)
#define MDRV_VE_SET_TTXDATA             _IOW(MDRV_VE_IOC_MAGIC, 12, stVE_TTXData)
//#define MDRV_VE_SET_TTXDATA             _IOW(MDRV_VE_IOC_MAGIC, 11, stVE_TTXData)
//#define MDRV_VE_DISABLE_ADCBUFOUT       _IO(MDRV_VE_IOC_MAGIC, 13)
#define MDRV_VE_SET_CCDATA              _IOW(MDRV_VE_IOC_MAGIC, 14, stVE_CCData)
#define MDRV_VE_SetBlackScreen          _IOW(MDRV_VE_IOC_MAGIC,  15, BOOL)
//#define MDRV_VE_AV2MNTOUT               _IO (MDRV_VE_IOC_MAGIC, 15)
//#define MDRV_VE_ATV2MNTOUT              _IO (MDRV_VE_IOC_MAGIC, 16)
//#define MDRV_VE_SC1CVBS2MNTOUT          _IO (MDRV_VE_IOC_MAGIC, 17)
//#define MDRV_VE_SC2CVBS2MNTOUT          _IO (MDRV_VE_IOC_MAGIC, 18)
//#define MDRV_VE_DTV2MNTOUT              _IO (MDRV_VE_IOC_MAGIC, 19)
//#define MDRV_VE_TRANSFER_INPUT_MUX      _IO (MDRV_VE_IOC_MAGIC, 20)
//#define MDRV_VE_ATV2TVOUT               _IOW(MDRV_VE_IOC_MAGIC, 21, BOOL)
//#define MDRV_VE_SVID2MNTOUT             _IO (MDRV_VE_IOC_MAGIC, 22)





#define MDRV_VE_IOC_MAXNR  23     // The number should be as same as the number of IOCTRL command

//#define MDRV_VE_CVBS2SCART  15     // The number should be as same as the number of IOCTRL command

#endif  // #ifndef _MDRV_VE_H_
