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

#ifndef _MDRV_RTC_H_
#define _MDRV_RTC_H_

#include "datatype.h"
#include "main.h"

#define INTERNAL_CLK_100000              (100000)

extern void MDrv_RTC_Init(U32 u32CtrlWord);
extern void MDrv_RTC_Init_deep(void);
#if ( defined(METHOD_RE_INIT_RTC) && !defined(METHOD_SYNC_1SEC_BOUNDARY) )
extern void MDrv_RTC_SetCounter(U32 u32Counter);
#endif
extern U32  MDrv_RTC_GetCounter(void);
extern void MDrv_RTC_Alarm(U32 u32AlarmCounter);
extern void MDrv_RTC_ClearInterrupt(void);

#endif // _MDRV_RTC_H_