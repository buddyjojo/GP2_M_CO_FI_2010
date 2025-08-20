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
/// file    mdrv_sar.c
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

#include "mdrv_sar.h"
#include "mhal_sar_reg.h"
#include "mhal_sar.h"

extern SAR_ModHandle_t SARDev;
//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define SAR_PRINT(fmt, args...)         //printk("[SAR][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_SAR_DEVICE_COUNT     1


#define SAR_DEBUG
#ifdef SAR_DEBUG
#define DEBUG_SAR(x) (x)
#else
#define DEBUG_SAR(x)
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
static U8 u8StorageChInfo[4] = {0xFF,0xFF,0xFF,0xFF};
static U8 u8StorageLevelInfo[4][4];
static U8 u8StorageKeyInfo[4][4];

static U8 PreviousCMD;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
#if 0
static irqreturn_t _MDrv_SAR_ISR(int irq, void *dev_id)
{
    U8	u8Key, u8RepeatFlag;

    if(MDrv_SAR_GetKeyCode(&u8Key, &u8RepeatFlag))
    {
        SAR_PRINT("_MDrv_SAR_ISR() -> %x, %x\n", u8Key, u8RepeatFlag);

       	if (SARDev.async_queue)
			//wake_up_interruptible(&key_wait_q);
    		kill_fasync(&SARDev.async_queue, SIGRTMIN, POLL_IN);
    }

    return IRQ_HANDLED;
}
#endif
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void MDrv_SAR_Init(void)
{
    PreviousCMD = 0;
    //S32 s32Result;

    MHal_SAR_Config_SingleChannel(0x04);
    MHal_SAR_Config_TriggerMode(HAL_SAR_EDGE); //0:edge trigger, 1: level trigger
    MHal_SAR_Config_SingleChannelEn(DISABLE);
    MHal_SAR_Config_ShotMode(HAL_SAR_FREERUN);
    MHal_SAR_Config_Powerdown(HAL_SAR_ADC_POWERUP);
    MHal_SAR_Config_Start(DISABLE);
    MHal_SAR_Config_ADCPowerdown(DISABLE);
    MHal_SAR_Config_FreeRun(ENABLE);
    MHal_SAR_Config_Selection(DISABLE);
    MHal_SAR_Config_8Channel(DISABLE);
    MHal_SAR_Config_ClockSamplePeriod(0x05);

    MHal_SAR_Config_IntMask(DISABLE);
    MHal_SAR_Config_IntClear(DISABLE);
    MHal_SAR_Config_IntForce(DISABLE);
    MHal_SAR_Config_IntClear(ENABLE);
    MHal_SAR_Config_IntClear(DISABLE);
#if 0
    s32Result = request_irq(E_IRQ_KEYPAD, _MDrv_SAR_ISR, SA_INTERRUPT, "SAR", NULL);
    if (s32Result)
    {
        SAR_PRINT("SAR IRQ registartion ERROR\n");
	}
	else
    {
        SAR_PRINT("SAR IRQ registartion OK\n");
    }
#endif
}

U8 MDrv_SAR_SetChInfo(SAR_CFG_t *sarChInfo)
{
    U8 i,j;
    U8 ret_val = FALSE;

    if(sarChInfo->u8SARChID >= MHal_SAR_GetChannelMaxId())
        return ret_val;

    for(i=0;i<4;i++)
    {
        if((u8StorageChInfo[i] == 0xFF) || (u8StorageChInfo[i] == sarChInfo->u8SARChID))
        {
            u8StorageChInfo[i] = sarChInfo->u8SARChID;
            for(j=0;i<sarChInfo->u8KeyLevelNum;j++)
            {
                u8StorageLevelInfo[i][j] = sarChInfo->u8KeyThreshold[j];
                u8StorageKeyInfo[i][j] = sarChInfo->u8KeyCode[j];
            }
            MHal_SAR_Config_ChannelBound(sarChInfo->u8SARChID,&sarChInfo->SARChBnd);
            ret_val = TRUE;
            break;
        }
    }

    return ret_val;
}

U8 MDrv_SAR_CHGetKey(U8 u8Index, U8 u8Channel, U8 *u8Key , U8 *u8Repstatus)
{
    U8 ret_val=FALSE;
    U8 i,j,KEY_LV[4],Key_Value;

    *u8Key = 0xFF;
    *u8Repstatus = 0;

    for(i=0; i<4; i++)
        KEY_LV[i] = 0;

    for ( i = 0; i < KEYPAD_STABLE_NUM; i++ )
    {
        Key_Value = MHal_SAR_GetChannelADC(u8Channel);

        for (j=0;j<4;j++)
        {
            if (Key_Value < u8StorageLevelInfo[u8Index][j])
            {
                KEY_LV[j] = KEY_LV[j] + 1;
                break;
            }
        }
    }

    for(i=0;i<4;i++)
    {
        if(KEY_LV[i] > KEYPAD_STABLE_NUM_MIN)
        {
            *u8Key = u8StorageKeyInfo[u8Index][i];
            if(PreviousCMD != *u8Key)
                PreviousCMD = *u8Key;
            else
            {
                // Prcoess repeat key

            }

            ret_val = TRUE;
        }
    }

    return ret_val;
}

U8 MDrv_SAR_GetKeyCode(U8 *u8Key, U8 *u8Repstatus)
{
    U8 i;

    for(i=0;i<4;i++)
    {
        if(u8StorageChInfo[i] < MHal_SAR_GetChannelMaxId())
        {
            if(MDrv_SAR_CHGetKey(i,u8StorageChInfo[i], u8Key, u8Repstatus))
                return TRUE;
        }
    }

    return FALSE;
}

