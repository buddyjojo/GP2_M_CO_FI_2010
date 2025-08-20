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

///////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_ci_io.h
/// @brief  CI Device Driver I/O
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __DEV_CI_H__
#define __DEV_CI_H__

//-----------------------------------------------------------------------------
//  Header Files
//-----------------------------------------------------------------------------
#include "mdrv_types.h"

//-----------------------------------------------------------------------------
//  Defines
//-----------------------------------------------------------------------------
#define MDRV_CI_IOC_MAGIC  '0'

//-----------------------------------------------------------------------------
//  IOCTL Methods
//-----------------------------------------------------------------------------
#define IOCTL_CI_DETECT_CARD					_IO(MDRV_CI_IOC_MAGIC,  0)
#define IOCTL_CI_RESET							_IO(MDRV_CI_IOC_MAGIC,  1)
#define IOCTL_CI_CHECK_CIS						_IO(MDRV_CI_IOC_MAGIC,  2)
#define IOCTL_CI_WRITE_COR						_IO(MDRV_CI_IOC_MAGIC,  3)
#define IOCTL_CI_SWITCH_BYPASS_MODE				_IO(MDRV_CI_IOC_MAGIC,  4)
#define IOCTL_CI_READ_DATA						_IO(MDRV_CI_IOC_MAGIC,  5)
#define IOCTL_CI_WRITE_DATA						_IO(MDRV_CI_IOC_MAGIC,  6)
#define IOCTL_CI_NEGOTIATE_BUF_SIZE				_IO(MDRV_CI_IOC_MAGIC,  7)
#define IOCTL_CI_READ_DA_STATUS					_IO(MDRV_CI_IOC_MAGIC,  8)
#define IOCTL_CI_PLUS_SET_PHY_RESET             _IO(MDRV_CI_IOC_MAGIC,  9)
#define IOCTL_CI_PLUS_READ_IIR_STATUS           _IO(MDRV_CI_IOC_MAGIC, 10)
#define IOCTL_CI_PLUS_GET_DATA_RATE             _IO(MDRV_CI_IOC_MAGIC, 11)
#define IOCTL_CI_PLUS_SET_CONTROL_SSL           _IO(MDRV_CI_IOC_MAGIC, 12)
#define IOCTL_CI_POWER_ON_OFF                   _IO(MDRV_CI_IOC_MAGIC, 13)
#define IOCTL_CI_CHECK_CIPLUS_CAPABILITY        _IO(MDRV_CI_IOC_MAGIC, 14)
#define IOCTL_CI_GET_MANUFACTURER_INFO          _IO(MDRV_CI_IOC_MAGIC, 15)
#define MDRV_CI_IOC_MAXNR                       16  // The number should be as same as the number of IOCTRL command

//-----------------------------------------------------------------------------
//  Enums
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Structures
//-----------------------------------------------------------------------------
typedef struct CI_DATA_INFO_s
{
	U16	u16DataSize;
	S8	*pu8Data;
} CI_DATA_INFO_t;

typedef struct CI_MANUFACTURER_INFO_s
{
    U8* pu8Manufacturer;
    U8 u8ManufacturerSize;
    U8* pu8Product;
    U8 u8ProductSize;    
} CI_MANUFACTURER_INFO_t;

#endif // #ifndef __DEV_CI_H__
