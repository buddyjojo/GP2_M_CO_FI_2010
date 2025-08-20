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
#include "mhal_cc_types.h"

#ifndef _MDRV_CC_H
#define _MDRV_CC_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_cc_.h
/// This file contains the Mstar driver I/O control and data type for Close Caption
/// @author MStar Semiconductor Inc.
/// @brief  Close Caption module
///////////////////////////////////////////////////////////////////////////////

typedef struct{
    U16 u16TopLineStart;
    U16 u16TopLineEnd;
    U16 u16BtnLineStart;
    U16 u16BtnLineEnd;
	BOOL IsNTSC;
}CC_Slicing_Region_t;


// use 't' as magic number
#define CC_IOCTL_MAGIC                          'c'
#define IOCTL_CC_INIT                          _IOW(CC_IOCTL_MAGIC, 0xA0, U32)
#define IOCTL_CC_ONOFF_SLICER                  _IOW(CC_IOCTL_MAGIC, 0xA1, U32)
#define IOCTL_CC_SET_VIDEOSTANDARD             _IOW(CC_IOCTL_MAGIC, 0xA2, U32)
#define IOCTL_CC_INTERRUPT_ED                  _IOW(CC_IOCTL_MAGIC, 0xA3, U32)
#define IOCTL_CC_RELEASE                       _IOW(CC_IOCTL_MAGIC, 0xA4, U32)
#define IOCTL_CC_GET_PACKET_UNIT               _IOR(CC_IOCTL_MAGIC, 0xA5, U32)
#define IOCTL_CC_GET_CC_PARAMETER_OFFSET      _IOR(CC_IOCTL_MAGIC, 0xA6, U32)
#define IOCTL_CC_GET_VPS_OFFSET                _IOR(CC_IOCTL_MAGIC, 0xA7, U32)
#define IOCTL_CC_GET_WSS_OFFSET                _IOR(CC_IOCTL_MAGIC, 0xA8, U32)
#define IOCTL_CC_WAIT_EVENT                    _IOR(CC_IOCTL_MAGIC, 0xA9, U32)
#define IOCTL_CC_RESET_VBI_PARAMETERS          _IOR(CC_IOCTL_MAGIC, 0xAA, U32)
#define IOCTL_CC_GET_PACKET_COUNT              _IOR(CC_IOCTL_MAGIC, 0xAB, U16)
#define IOCTL_CC_REINIT                        _IO(CC_IOCTL_MAGIC, 0xAC)
#define IOCTL_CC_SETSLICINGREGION              _IOW(CC_IOCTL_MAGIC, 0xAD, U32)
#define IOCTL_CC_SETSLICINGSYS              	_IOW(CC_IOCTL_MAGIC, 0xAE, U32)
#define IOCTL_CC_UART_OFF                      _IOW(CC_IOCTL_MAGIC, 0xF0, U32)


extern void MDrv_Wakeup(void);


#endif

