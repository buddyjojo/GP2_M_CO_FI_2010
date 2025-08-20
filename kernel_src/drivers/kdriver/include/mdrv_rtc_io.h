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
/// @file   mdrv_rtc_io.h
/// @brief  RTC Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_RTC_IO_H_
#define _MDRV_RTC_IO_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mdrv_rtc_st.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define RTC_IOC_MAGIC                'r'

#define MDRV_RTC_INIT                    _IO(RTC_IOC_MAGIC, 0)
#define MDRV_RTC_SET_COUNTER            _IOW(RTC_IOC_MAGIC, 1, U32)
#define MDRV_RTC_GET_COUNTER            _IOR(RTC_IOC_MAGIC, 2, U32)
#define MDRV_RTC_SET_MATCH_COUNTER     _IOW(RTC_IOC_MAGIC, 3, U32)
#define MDRV_RTC_GET_MATCH_COUNTER     _IOR(RTC_IOC_MAGIC, 4, U32)
#define MDRV_RTC_CLEAR_INT              _IO(RTC_IOC_MAGIC, 5)
#define MDRV_RTC_TEST                    _IO(RTC_IOC_MAGIC, 6)
#define RTC_IOC_MAXNR                   7

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32RTCMajor;
    int                         s32RTCMinor;
    struct cdev                 cDevice;
    struct file_operations      RTCFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} RTC_ModHandle_t;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#endif // _MDRV_RTC_IO_H_
