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
/// Data structure definition
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_8051_ST_H_
#define _MDRV_8051_ST_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    U8 u8Valid;
    U8 u8Message[MB_SIZE_8051_TO_MIPS];
} MS_8051_Msg_Info, *PMS_8051_Msg_Info;



#endif // _MDRV_8051_ST_H_
