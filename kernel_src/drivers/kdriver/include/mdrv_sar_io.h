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
/// @file   mdrv_sar_io.h
/// @brief  SAR Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_SAR_IO_H_
#define _MDRV_SAR_IO_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mdrv_sar_st.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define SAR_IOC_MAGIC                's'

#define MDRV_SAR_INIT                    _IO(SAR_IOC_MAGIC, 0)
#define MDRV_SAR_CH_INFO                _IOWR(SAR_IOC_MAGIC, 1, SAR_CFG_t)
#define MDRV_SAR_CH_GET_KEY             _IOWR(SAR_IOC_MAGIC, 2, SAR_Key_t)
#define MDRV_SAR_GET_KEY_CODE          _IOWR(SAR_IOC_MAGIC, 3, SAR_Key_t)

#define SAR_IOC_MAXNR                   4

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32SARMajor;
    int                         s32SARMinor;
    struct cdev                 cDevice;
    struct file_operations      SARFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} SAR_ModHandle_t;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#endif // _MDRV_SAR_IO_H_
