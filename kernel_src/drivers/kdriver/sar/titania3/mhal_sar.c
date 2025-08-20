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
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mhal_sar.h"
#include "mhal_sar_reg.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define CH_MAX_ID   HAL_SAR_CH7

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void MHal_SAR_Init(void)
{

}

U8 MHal_SAR_GetChannelMaxId(void)
{
    return CH_MAX_ID;
}

B16 MHal_SAR_Config_ChannelBound(U8 u8Channel, SAR_BND2_t *psarBndCfg)
{
    if(u8Channel >= CH_MAX_ID)
        return FALSE;

    switch(u8Channel)
    {
        case HAL_SAR_CH1:
            MHal_SAR_REG(REG_SAR_CH1_UPB) = MHal_SAR_REG(REG_SAR_CH1_UPB) & ~_SAR_CH1_UPB;
            MHal_SAR_REG(REG_SAR_CH1_UPB) = MHal_SAR_REG(REG_SAR_CH1_UPB) | (psarBndCfg->u8UpBnd);
            MHal_SAR_REG(REG_SAR_CH1_LOB) = (psarBndCfg->u8LoBnd);
            break;
        case HAL_SAR_CH2:
            MHal_SAR_REG(REG_SAR_CH2_UPB) = (psarBndCfg->u8UpBnd);
            MHal_SAR_REG(REG_SAR_CH2_LOB) = (psarBndCfg->u8LoBnd);
            break;
        case HAL_SAR_CH3:
            MHal_SAR_REG(REG_SAR_CH3_UPB) = (psarBndCfg->u8UpBnd);
            MHal_SAR_REG(REG_SAR_CH3_LOB) = (psarBndCfg->u8LoBnd);
            break;
        case HAL_SAR_CH4:
            MHal_SAR_REG(REG_SAR_CH4_UPB) = (psarBndCfg->u8UpBnd);
            MHal_SAR_REG(REG_SAR_CH4_LOB) = (psarBndCfg->u8LoBnd);
            break;
        case HAL_SAR_CH5:
            MHal_SAR_REG(REG_SAR_CH5_UPB) = (psarBndCfg->u8UpBnd);
            MHal_SAR_REG(REG_SAR_CH5_LOB) = (psarBndCfg->u8LoBnd);
            break;
        case HAL_SAR_CH6:
            MHal_SAR_REG(REG_SAR_CH6_UPB) = (psarBndCfg->u8UpBnd);
            MHal_SAR_REG(REG_SAR_CH6_LOB) = (psarBndCfg->u8LoBnd);
            break;

    }

    return TRUE;
}

void MHal_SAR_Config_SingleChannel(U8 u8Channel)
{
    U8 u8Ch = u8Channel & MASK_SAR_SINGLE_CH;
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8Ch;
}

void MHal_SAR_Config_TriggerMode(B16 bMode)
{
    U8 u8Trigger = (bMode)? _SAR_LEVEL_TRIGGER : (~_SAR_LEVEL_TRIGGER);
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8Trigger;
}

void MHal_SAR_Config_SingleChannelEn(B16 bEnable)
{
    U8 u8ChEn = (bEnable)? _SAR_SINGLE_CH_EN : (~_SAR_SINGLE_CH_EN);
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8ChEn;
}

void MHal_SAR_Config_ShotMode(U8 u8Mode)
{
    U8 u8ShotMode = (u8Mode)? _SAR_MODE : (~_SAR_MODE); // 1: Freerun, 0: One-Shot
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8ShotMode;
}

void MHal_SAR_Config_Powerdown(B16 bEnable)
{
    U8 u8PDEn = (bEnable)? _SAR_PD : (~_SAR_PD);
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8PDEn;
}

void MHal_SAR_Config_Start(B16 bEnable)
{
    U8 u8Start = (bEnable)? _SAR_START : (~_SAR_START);
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8Start;
}

void MHal_SAR_Config_ADCPowerdown(B16 bEnable)
{
    U8 u8ADCPDEn = (bEnable)? _SAR_ADC_PD : (~_SAR_ADC_PD); //sar atop power down. 1: power down, 0: enable sar atop
    MHal_SAR_REG(REG_SAR_CTRL1) = MHal_SAR_REG(REG_SAR_CTRL1) | u8ADCPDEn;
}

void MHal_SAR_Config_FreeRun(B16 bEnable)
{
    U8 u8Freerun = (bEnable)? _SAR_FREERUN : (~_SAR_FREERUN); //sar atop freerun mode. 0: controlled by digital (default), 1: freerun
    MHal_SAR_REG(REG_SAR_CTRL1) = MHal_SAR_REG(REG_SAR_CTRL1) | u8Freerun;
}

void MHal_SAR_Config_Selection(B16 bEnable)
{
    U8 u8Sel = (bEnable)? _SAR_SEL : (~_SAR_SEL);
    MHal_SAR_REG(REG_SAR_CH1_UPB) = MHal_SAR_REG(REG_SAR_CTRL1) | u8Sel;
}

void MHal_SAR_Config_8Channel(B16 bEnable)
{
    U8 u8En8Ch = (bEnable)? _SAR_8CH_EN : (~_SAR_8CH_EN); //1: sar 8 channel, 0: sar 4 channel
    MHal_SAR_REG(REG_SAR_CH1_UPB) = MHal_SAR_REG(REG_SAR_CH1_UPB) | u8En8Ch;
}

void MHal_SAR_Config_ClockSamplePeriod(U8 u8ClkSmpPrd)
{
    MHal_SAR_REG(REG_SAR_CKSAMP_PRD) = u8ClkSmpPrd;
}

void MHal_SAR_Config_IntMask(B16 bEnable)
{
    U8 u8IntMask = (bEnable)? _SAR_INT_MASK : (~_SAR_INT_MASK);
    MHal_SAR_REG(REG_SAR_INT) = MHal_SAR_REG(REG_SAR_INT) | u8IntMask;
}

void MHal_SAR_Config_IntClear(B16 bEnable)
{
    U8 u8IntClr = (bEnable)? _SAR_INT_CLR : (~_SAR_INT_CLR);
    MHal_SAR_REG(REG_SAR_INT) = MHal_SAR_REG(REG_SAR_INT) | u8IntClr;
}

void MHal_SAR_Config_IntForce(B16 bEnable)
{
    U8 u8IntForce = (bEnable)? _SAR_INT_FORCE : (~_SAR_INT_FORCE);
    MHal_SAR_REG(REG_SAR_INT) = MHal_SAR_REG(REG_SAR_INT) | u8IntForce;
}

B16 MHal_SAR_GetIntStatus(void)
{
    U8 u8IntStatus;

    u8IntStatus = MHal_SAR_REG(REG_SAR_INT) & _SAR_INT_STATUS;
    return (u8IntStatus)? ENABLE : DISABLE;
}

U8 MHal_SAR_GetChannelADC(U8 u8Channel)
{
    U32 u32RegAddr;
    U8 u8AdcValue;

    if(u8Channel >= CH_MAX_ID)
        return HAL_SAR_ADC_DATA_MAX;

    switch(u8Channel)
    {
        case HAL_SAR_CH1:
            u32RegAddr = REG_SAR_ADC_CH1_DATA;
            break;
        case HAL_SAR_CH2:
            u32RegAddr = REG_SAR_ADC_CH2_DATA;
            break;
        case HAL_SAR_CH3:
            u32RegAddr = REG_SAR_ADC_CH3_DATA;
            break;
        case HAL_SAR_CH4:
            u32RegAddr = REG_SAR_ADC_CH4_DATA;
            break;
        case HAL_SAR_CH5:
            u32RegAddr = REG_SAR_ADC_CH5_DATA;
            break;
        case HAL_SAR_CH6:
            u32RegAddr = REG_SAR_ADC_CH6_DATA;
            break;


    }
    u8AdcValue = MHal_SAR_REG(u32RegAddr) & MASK_SAR_ADCOUT;

    return u8AdcValue;
}

