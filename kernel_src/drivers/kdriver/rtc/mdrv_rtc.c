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
/// file    mdrv_rtc.c
/// @brief  Real Time Clock (RTC) Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <linux/autoconf.h>
#include <linux/undefconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mst_devid.h"

#include "mdrv_rtc.h"
#include "mhal_rtc_reg.h"
#include "mhal_rtc.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define RTC_PRINT(fmt, args...)         //printk("[RTC][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_RTC_DEVICE_COUNT     1


#define RTC_DEBUG
#ifdef RTC_DEBUG
#define DEBUG_RTC(x) (x)
#else
#define DEBUG_RTC(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------
U32 _u32Counter;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

void MDrv_RTC_Init(void)
{
    MHal_RTC_Init();
}

void MDrv_RTC_SetCounter(U32 u32Counter)
{
    MHal_RTC_SetCounter(u32Counter);
}

U32 MDrv_RTC_GetCounter(void)
{
    U32 u32Counter;
    u32Counter = MHal_RTC_GetCounter();
    return (u32Counter);
}

void MDrv_RTC_SetMatchCounter(U32 u32Counter)
{
    MHal_RTC_SetMatchCounter(u32Counter);
}

U32 MDrv_RTC_GetMatchCounter(void)
{
    U32 u32Counter;
    u32Counter = MHal_RTC_GetMatchCounter();
    return (u32Counter);
}

void MDrv_RTC_ClearInterrupt(void)
{
    MHal_RTC_ClearInterrupt();
}

void MDrv_RTC_Test(void)
{
    RTC_PRINT("Running MDrv_RTC_Test\n");
    MHal_RTC_Test();
}

