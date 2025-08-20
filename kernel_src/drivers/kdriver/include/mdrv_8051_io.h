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
/// @file   drv8051.h
/// @brief  8051 Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access 8051.
///     - Provide functions to initialize 8051
///     - Provide 8051 ISR.
///     - Provide 8051 callback function registration for AP.
///
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_8051_IO_H_
#define _MDRV_8051_IO_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mdrv_8051_st.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define MCU8051_IOC_MAGIC                'u'

#define MDRV_MCU8051_INIT                _IO(MCU8051_IOC_MAGIC, 0)
#define MDRV_MCU8051_SEND_MAIL           _IOW(MCU8051_IOC_MAGIC, 1, int)
#define MDRV_MCU8051_RECEIVE_MAIL        _IOW(MCU8051_IOC_MAGIC, 2, int)
#define MDRV_MCU8051_TEST                _IOW(MCU8051_IOC_MAGIC, 3, int)
#define MCU8051_IOC_MAXNR                4

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32MCU8051Major;
    int                         s32MCU8051Minor;
    struct cdev                 cDevice;
    struct file_operations      MCU8051Fop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} MCU8051ModHandle;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------


#endif // _MDRV_8051_IO_H_
