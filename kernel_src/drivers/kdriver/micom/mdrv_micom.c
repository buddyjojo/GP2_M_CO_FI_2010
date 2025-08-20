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
/// file    mdrv_micom.c
/// @brief  MICOM Control Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <linux/autoconf.h>
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
#include <asm/io.h>

#include "Board.h"
#include "mst_devid.h"
#include "mdrv_types.h"
#include "mhal_micom_reg.h"
#include "mhal_micom.h"
#include "mdrv_micom_io.h"
#include "mdrv_micom.h"
#include "chip_int.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define MICOM_REG(addr)                   (*(volatile U32 *)(addr))
#define MICOM_REG_8(addr)                 (*(volatile U8 *)(addr))
#define MDrv_WriteByteMask( u16Reg, u8Value, u8Mask )    \
    (MICOM_REG_8(u16Reg) = (MICOM_REG_8(u16Reg) & ~(u8Mask)) | ((u8Value) & (u8Mask)))
#define MICOM_PRINT(fmt, args...)         //printk("[Micom][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_MICOM_DEVICE_COUNT     1


#undef MICOM_DEBUG
#ifdef MICOM_DEBUG
#define DEBUG_MICOM(x) (x)
#else
#define DEBUG_MICOM(x)
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
//SAR Section
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
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
//PM Section
PM_Result MDrv_MICOM_Init(PM_WakeCfg_t *pPmWakeCfg)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_GetStatus(PM_DrvStatus *pDrvStatus)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_GetLibVer(PM_LibVer_t *pPmLibVer)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_GetInfo(PM_DrvInfo_t *pPmDrvInfo)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_SetDbgLevel(PM_DbgLv eLevel)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_PowerDown(void)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_Reset(void)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_Control(void)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_TxData(void)
{
    return E_PM_OK;
}

PM_Result MDrv_MICOM_RxData(void)
{
    return E_PM_OK;
}

B16 MDrv_MICOM_RegWrite( U16 u16Addr, U8 u8Data )
{
    //this is a temp temporary function
    return TRUE;
}

U8 MDrv_MICOM_RegRead( U16 u16Addr )
{
    //this is a temp temporary function
    return TRUE;
}

B16 MDrv_MICOM_ReceiveMailFromMicom(U8 *u8MBIndex, U16 *u16MBData)
{
    //this is a temp temporary function
    return TRUE;
}
EXPORT_SYMBOL(MDrv_MICOM_ReceiveMailFromMicom);

//RTC Section
void MDrv_MICOM_Rtc_Init(void)
{
    MHal_MICOM_Rtc_Init();
}

void MDrv_MICOM_Rtc_SetCounter(U32 u32Counter)
{
    MHal_MICOM_Rtc_SetCounter(u32Counter);
}

U32 MDrv_MICOM_Rtc_GetCounter(void)
{
    U32 u32Counter;
    u32Counter = MHal_MICOM_Rtc_GetCounter();
    return (u32Counter);
}

void MDrv_MICOM_Rtc_SetMatchCounter(U32 u32Counter)
{
    MHal_MICOM_Rtc_SetMatchCounter(u32Counter);
}

U32 MDrv_MICOM_Rtc_GetMatchCounter(void)
{
    U32 u32Counter;
    u32Counter = MHal_MICOM_Rtc_GetMatchCounter();
    return (u32Counter);
}

void MDrv_MICOM_Rtc_ClearInterrupt(void)
{
    MHal_MICOM_Rtc_ClearInterrupt();
}

void MDrv_MICOM_Rtc_Test(void)
{
    MICOM_PRINT("Running MDrv_RTC_Test\n");
    MHal_MICOM_Rtc_Test();
}

//SAR Section
void MDrv_MICOM_Sar_Init(void)
{
    PreviousCMD = 0;
    //S32 s32Result;

    MHal_MICOM_Sar_Config_SingleChannel(0x04);
    MHal_MICOM_Sar_Config_TriggerMode(HAL_SAR_EDGE); //0:edge trigger, 1: level trigger
    MHal_MICOM_Sar_Config_SingleChannelEn(DISABLE);
    MHal_MICOM_Sar_Config_ShotMode(HAL_SAR_FREERUN);
    MHal_MICOM_Sar_Config_Powerdown(HAL_SAR_ADC_POWERUP);
    MHal_MICOM_Sar_Config_Start(DISABLE);
    MHal_MICOM_Sar_Config_ADCPowerdown(DISABLE);
    MHal_MICOM_Sar_Config_FreeRun(ENABLE);
    MHal_MICOM_Sar_Config_Selection(DISABLE);
    MHal_MICOM_Sar_Config_8Channel(DISABLE);
    MHal_MICOM_Sar_Config_ClockSamplePeriod(0x05);

    MHal_MICOM_Sar_Config_IntMask(DISABLE);
    MHal_MICOM_Sar_Config_IntClear(DISABLE);
    MHal_MICOM_Sar_Config_IntForce(DISABLE);
    MHal_MICOM_Sar_Config_IntClear(ENABLE);
    MHal_MICOM_Sar_Config_IntClear(DISABLE);
#if 0
    s32Result = request_irq(E_IRQ_KEYPAD, _MDrv_SAR_ISR, SA_INTERRUPT, "SAR", NULL);
    if (s32Result)
    {
        MICOM_PRINT("SAR IRQ registartion ERROR\n");
	}
	else
    {
        MICOM_PRINT("SAR IRQ registartion OK\n");
    }
#endif
}

U8 MDrv_MICOM_Sar_SetChInfo(SAR_CFG_t *sarChInfo)
{
    U8 i,j;
    U8 ret_val = FALSE;

    if(sarChInfo->u8SARChID >= MHal_MICOM_Sar_GetChannelMaxId())
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
            MHal_MICOM_Sar_Config_ChannelBound(sarChInfo->u8SARChID,&sarChInfo->SARChBnd);
            ret_val = TRUE;
            break;
        }
    }

    return ret_val;
}

U8 MDrv_MICOM_Sar_CHGetKey(U8 u8Index, U8 u8Channel, U8 *u8Key , U8 *u8Repstatus)
{
    U8 ret_val=FALSE;
    U8 i,j,KEY_LV[4],Key_Value;

    *u8Key = 0xFF;
    *u8Repstatus = 0;

    for(i=0; i<4; i++)
        KEY_LV[i] = 0;

    for ( i = 0; i < KEYPAD_STABLE_NUM; i++ )
    {
        Key_Value = MHal_MICOM_Sar_GetChannelADC(u8Channel);

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

U8 MDrv_MICOM_Sar_GetKeyCode(U8 *u8Key, U8 *u8Repstatus)
{
    U8 i;

    for(i=0;i<4;i++)
    {
        if(u8StorageChInfo[i] < MHal_MICOM_Sar_GetChannelMaxId())
        {
            if(MDrv_MICOM_Sar_CHGetKey(i,u8StorageChInfo[i], u8Key, u8Repstatus))
                return TRUE;
        }
    }

    return FALSE;
}


