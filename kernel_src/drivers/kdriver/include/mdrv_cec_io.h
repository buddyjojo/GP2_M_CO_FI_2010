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
/// @file   mdrv_cec_io.h
/// @brief  CEC(Consumer Electronics Control) Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_CEC_IO_H_
#define _MDRV_CEC_IO_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mdrv_cec_st.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define CEC_IOC_MAGIC                'c'

#define MDRV_CEC_INIT                   _IO(CEC_IOC_MAGIC, 0)
#define MDRV_CEC_CHKDEVS                _IOR(CEC_IOC_MAGIC, 1, CEC_INFO_LIST_t)
#define MDRV_CEC_RX_API                 _IOR(CEC_IOC_MAGIC, 2, CEC_INFO_LIST_t)
#define MDRV_CEC_TX_API                 _IOW(CEC_IOC_MAGIC, 3, CEC_TX_INFO_t)
#define MDRV_CEC_GET_RESULT             _IOR(CEC_IOC_MAGIC, 4, CEC_TX_INFO_t)
#define MDRV_CEC_RESPONSE               _IOR(CEC_IOC_MAGIC, 5, U8)

#define CEC_IOC_MAXNR                   6

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32CECMajor;
    int                         s32CECMinor;
    struct cdev                 cDevice;
    struct file_operations      CECFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} CEC_ModHandle_t;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#endif // _MDRV_CEC_IO_H_
