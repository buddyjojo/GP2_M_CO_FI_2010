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
/// @file   mdrv_rtc.h
/// @brief  RTC Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_RTC_H_
#define _MDRV_RTC_H_

#include <asm-mips/types.h>
#include "mdrv_rtc_st.h"
#include "mdrv_rtc_io.h"
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MDrv_RTC_Init(void);
void MDrv_RTC_SetCounter(U32 u32Counter);
U32 MDrv_RTC_GetCounter(void);
void MDrv_RTC_SetMatchCounter(U32 u32Counter);
U32 MDrv_RTC_GetMatchCounter(void);
void MDrv_RTC_ClearInterrupt(void);
void MDrv_RTC_Test(void);

#endif // _MDRV_RTC_H_
