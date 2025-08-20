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
/// @file   drvAeon.h
/// @brief  Aeon Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access Aeon.
///     - Provide functions to initialize Aeon
///     - Provide Aeon ISR.
///     - Provide Aeon callback function registration for AP.
///
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_AEON_IO_H_
#define _MDRV_AEON_IO_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mdrv_aeon_st.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define AEON_IOC_MAGIC                'u'

#define MDRV_AEON_INIT                _IO(AEON_IOC_MAGIC, 0)
#define MDRV_AEON_SEND_MAIL           _IOW(AEON_IOC_MAGIC, 1, int)
#define MDRV_AEON_RECEIVE_MAIL        _IOW(AEON_IOC_MAGIC, 2, int)
#define MDRV_AEON_TEST                _IOW(AEON_IOC_MAGIC, 3, int)
#define AEON_IOC_MAXNR                4

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32AeonMajor;
    int                         s32AeonMinor;
    struct cdev                 cDevice;
    struct file_operations      AeonFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} AeonModHandle;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#endif // _MDRV_AEON_IO_H_
