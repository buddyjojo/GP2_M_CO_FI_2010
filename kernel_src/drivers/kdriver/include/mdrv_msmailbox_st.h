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
/// @file   mdrv_mailbox_st.h
/// @brief  Mialbox Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access mailbox.
///     - Provide functions to initialize mailbox
///     - Provide mailbox ISR.
///     - Provide mailbox wail function for AP.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MDRV_MSMAILBOX_ST_H__
#define __MDRV_MSMAILBOX_ST_H__

#include "mdrv_types.h"

typedef struct MSMAILBOX_BUF_INFO
{
    U32             u32PutAdr;
    U32             u32GetAdr;
    U32             u32MaxMUnit;
    U32             u32Overflow;
}msmailbox_info_t;


typedef struct MSMAILBOX
{
    U8              u8Ctrl;
    U8              u8Class;
    U8              u8Index;
    U8              u8ParameterCount;
    U8              u8Parameters[10];
    U8              u8S0;
    U8              u8S1;
}msmailbox_t;


/* Mail Box States */
typedef enum
{
    E_OK                                =   0x00,
    E_MEMALLOC_FAIL                     =   0x01,
    E_MMMAP_NOT_FREE                    =   0x02,
    E_MAILBOX_INVALID                   =   0x03,
    E_INTERRUPT_INVALID                 =   0x04,
    E_WAITQUEUE_INVALID                 =   0x05,
    E_WAITQUEUE_FULL                    =   0x06,
    E_WAITQUEUE_EMPTY                   =   0x07,
    E_COPYTOUSER_FAIL                   =   0x08,
    E_COPYFROMUSER_FAIL                 ,
    E_HAL_COPROCESSOR_BUSY              ,
    E_HAL_COPROCESSOR_NOT_READY         ,
    E_HAL_COPROCESSOR_OVERFLOW          ,
    E_HAL_INSTANT_MAIL                  ,

} _ERR_CODE_;

#define         MSMAILBOX_SIZE              sizeof(msmailbox_t)


#endif



