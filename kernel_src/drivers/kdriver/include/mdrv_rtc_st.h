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
/// @file   mdrv_rtc_st.h
/// @brief  RTC Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Data structure definition
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_RTC_ST_H_
#define _MDRV_RTC_ST_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    U8  u8Valid;
    U32 u32Counter;
} RTC_Time_t;

#endif // _MDRV_RTC_ST_H_
