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

#include "mhal_micom.h"
#include "mhal_micom_reg.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define RTC_RESET_DELAY()       { volatile int i; for (i = 0; i < 10000; i++); } // need 0.1ms

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define CH_MAX_ID   HAL_SAR_CH7

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
void MHal_MICOM_Rtc_Init(void)
{
    MHal_RTC_REG(REG_RTC_CTRL) = 0x0000;
    MHal_RTC_REG(REG_RTC_CTRL) = (RTC_CTRL_NOT_RSTZ | RTC_CTRL_INT_MASK);
    RTC_RESET_DELAY();
    //Set frequency control word of RTC counter
    MHal_RTC_REG(REG_RTC_FREQ_CW1) = (CONTROL_WORD & 0x0000ffff);
    MHal_RTC_REG(REG_RTC_FREQ_CW2) = ((CONTROL_WORD & 0xffff0000)>>16);

    MHal_RTC_REG(REG_RTC_CTRL) = (RTC_CTRL_NOT_RSTZ | RTC_CTRL_CNT_EN | RTC_CTRL_INT_MASK | RTC_CTRL_INT_CLEAR);
}

void MHal_MICOM_Rtc_SetCounter(U32 u32Counter)
{
    //Value to load into RTC counter
    MHal_RTC_REG(REG_RTC_LOAD_VAL1) = (u32Counter & 0x0000ffff);
    MHal_RTC_REG(REG_RTC_LOAD_VAL2) = ((u32Counter & 0xffff0000)>>16);

    MHal_RTC_REG(REG_RTC_CTRL) |= RTC_CTRL_LOAD_EN;
}

U32 MHal_MICOM_Rtc_GetCounter(void)
{
    U16  u16GetWord1, u16GetWord2;
    U32  u32Counter;
    MHal_RTC_REG(REG_RTC_CTRL) |= RTC_CTRL_READ_EN;
    //RTC_UPDATE_DELAY();
    u16GetWord1 = MHal_RTC_REG(REG_RTC_CNT1);
    u16GetWord2 = MHal_RTC_REG(REG_RTC_CNT2);
    u32Counter = ((U32)u16GetWord2 << 16) + ((U32)u16GetWord1);
    return u32Counter;
}

void MHal_MICOM_Rtc_SetMatchCounter(U32 u32Counter)
{
    U16 u16Reg;

    u16Reg = MHal_RTC_REG(REG_RTC_CTRL) | RTC_CTRL_INT_MASK | RTC_CTRL_INT_CLEAR;
    MHal_RTC_REG(REG_RTC_CTRL) = u16Reg;

    if (u32Counter)
    {
        //Counter match value
        MHal_RTC_REG(REG_RTC_MATCH_VAL1) = (u32Counter & 0x0000ffff);
        MHal_RTC_REG(REG_RTC_MATCH_VAL2) = ((u32Counter & 0xffff0000)>>16);

        u16Reg &=~RTC_CTRL_INT_MASK;
        u16Reg &=~RTC_CTRL_INT_CLEAR;

        MHal_RTC_REG(REG_RTC_CTRL) = u16Reg;
    }
}

U32 MHal_MICOM_Rtc_GetMatchCounter(void)
{
    U16  u16GetWord1, u16GetWord2;
    U32  u32Counter;
    u16GetWord1 = MHal_RTC_REG(REG_RTC_MATCH_VAL1);
    u16GetWord2 = MHal_RTC_REG(REG_RTC_MATCH_VAL2);
    u32Counter = ((U32)u16GetWord2 << 16) + ((U32)u16GetWord1);
    return u32Counter;
}

void MHal_MICOM_Rtc_ClearInterrupt(void)
{
    MHal_RTC_REG(REG_RTC_CTRL) |= RTC_CTRL_INT_CLEAR;
}

void MHal_MICOM_Rtc_Test(void)
{
    U16 u16tmp;

	//printf("MPLL enable\n");
	//RIU_Write( 0x00, 0x1288 *2,  0x0506 ) ;
    (*(volatile U16*)(0xBF200000 + (0x1288<<2))) = 0x0506;
	//RIU_Read ( 0x00, 0x1288 *2, &u16tmp ) ;
	u16tmp = (*(volatile U16*)(0xBF200000 + (0x1288<<2)));

	//printf("Set 8051 to wakeup from address 0x0\n");
	//RIU_Write( 0x00, 0x0709 *2,  0x58ef ) ;	// reset
	//RIU_Write( 0x00, 0x0709 *2,  0x18ef ) ;	// continue
	(*(volatile U16*)(0xBF000000 + (0x0709<<2))) = 0x18ef;  // continue

	//RIU_Write( 0x00, 0x0709 *2,  0x00ef ) ;	// off bit14 for continue run (not reset)
                                            // [7:0]  reg_wk_irq_pol            // 0: active high, 1: active low
                                            // [8]    reg_pdn_osc_clk23         // dfault is 1
                                            // [9]    reg_ddcda_in_pm           // 0: use ATOP_PAD, 1: use PAD_TS0[3:2]
                                            // [10]   reg_ddcdb_in_pm           // 0: use ATOP_PAD, 1: use PAD_TS0[3:2]
                                            // [11]   reg_uart_rx_enable
                                            // [12]   reg_hk51_uart0_en
                                            // [13]   reg_wakeup_rst_chip_top_en
                                            // [14]   reg_wakeup_rst_rst_51_en  // 0: wake from last addr, 1: wake from addr0x0
                                            // [15]   reg_deep_sleep            // 0: sleep w/ ext crystal, 1: deep sleep w/ int crystal

    //printf("Enable 8051 reset - first password\n");
	//RIU_Write( 0x00, 0x072a *2, 0x829f ) ;  // Enable 8051 reset - first password
	(*(volatile U16*)(0xBF000000 + (0x072a<<2))) = 0x829f;  // Enable 8051 reset - first password
	//RIU_Write( 0x00, 0x072a *2, 0x0000 ) ;  // off it for continue run (not reset)

	// wake and continue run. 1. h0709 bit14 -> 0. 2. don't set password

	//printf("Unmask RTC interrupt - 8051 will not receive interrupt\n");
	//RIU_Write( 0x00, 0x0708 *2, 0x00ef ) ; // [7:0]  reg_wk_irq_mask           // 0: unmask, 1: mask
	(*(volatile U16*)(0xBF000000 + (0x0708<<2))) = 0x00ef;  // [7:0]  reg_wk_irq_mask           // 0: unmask, 1: mask
                                         // [15:8] reg_wk_irq_force

	//printf("Enable pm to power down another domain\n");
	//RIU_Write( 0x00, 0x0710 *2, 0x1fef ) ; // reg_gpio_pm_out[12:0]           // 0: output, 1: input
	(*(volatile U16*)(0xBF000000 + (0x0710<<2))) = 0x1fef;  // reg_gpio_pm_out[12:0]           // 0: output, 1: input
	//RIU_Write( 0x00, 0x070f *2, 0x1fef ) ; // reg_gpio_pm_oen[12:0]           // 0: enable, 1: disable
	(*(volatile U16*)(0xBF000000 + (0x070f<<2))) = 0x1fef;  // reg_gpio_pm_oen[12:0]           // 0: enable, 1: disable

	//printf("Set RTC to wakeup system in a period\n");
	//set_rtc_tmp();

    //printf("Enter sleep mode - second password\n");

	//RIU_Write( 0x00, 0x0737 *2, 0x00a5 ) ; // reg_riu_dummy3[15:0]
	(*(volatile U16*)(0xBF000000 + (0x0737<<2))) = 0x00a5;  // reg_riu_dummy3[15:0]
	//RIU_Write( 0x00, 0x0712 *2, 0xbabe ) ; // reg_gpio_pm_lock[15:0]
	(*(volatile U16*)(0xBF000000 + (0x0712<<2))) = 0xbabe;  // reg_gpio_pm_lock[15:0]

	// Disable pm to power down another domain
	//RIU_Write( 0x00, 0x070f *2, 0x1fff ) ; // reg_gpio_pm_oen[12:0]
	(*(volatile U16*)(0xBF000000 + (0x070f<<2))) = 0x1fff;  // reg_gpio_pm_oen[12:0]
	// Erase power-down password
	//RIU_Write( 0x00, 0x0737 *2, 0x0000 ) ; // reg_riu_dummy3[15:0]
	(*(volatile U16*)(0xBF000000 + (0x0737<<2))) = 0x0000;  // reg_riu_dummy3[15:0]
	//RIU_Write( 0x00, 0x0712 *2, 0x0000 ) ; // reg_gpio_pm_lock[15:0]
	(*(volatile U16*)(0xBF000000 + (0x0712<<2))) = 0x0000;  // reg_gpio_pm_lock[15:0]

    // toggle high low for clear interrupt
	//RIU_Write( 0x00, 0x0704 *2, 0x0002 ); // reg_wk_fiq_clr[15:0]
	(*(volatile U16*)(0xBF000000 + (0x0704<<2))) = 0x0002;  // reg_wk_fiq_clr[15:0]
	//RIU_Write( 0x00, 0x0704 *2, 0x0000 ); // reg_wk_fiq_clr[15:0]
	(*(volatile U16*)(0xBF000000 + (0x0704<<2))) = 0x0000;  // reg_wk_fiq_clr[15:0]
}
//SAR Section
void MHal_MICOM_Sar_Init(void)
{

}

U8 MHal_MICOM_Sar_GetChannelMaxId(void)
{
    return CH_MAX_ID;
}

B16 MHal_MICOM_Sar_Config_ChannelBound(U8 u8Channel, SAR_BND2_t *psarBndCfg)
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

void MHal_MICOM_Sar_Config_SingleChannel(U8 u8Channel)
{
    U8 u8Ch = u8Channel & MASK_SAR_SINGLE_CH;
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8Ch;
}

void MHal_MICOM_Sar_Config_TriggerMode(B16 bMode)
{
    U8 u8Trigger = (bMode)? _SAR_LEVEL_TRIGGER : (~_SAR_LEVEL_TRIGGER);
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8Trigger;
}

void MHal_MICOM_Sar_Config_SingleChannelEn(B16 bEnable)
{
    U8 u8ChEn = (bEnable)? _SAR_SINGLE_CH_EN : (~_SAR_SINGLE_CH_EN);
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8ChEn;
}

void MHal_MICOM_Sar_Config_ShotMode(U8 u8Mode)
{
    U8 u8ShotMode = (u8Mode)? _SAR_MODE : (~_SAR_MODE); // 1: Freerun, 0: One-Shot
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8ShotMode;
}

void MHal_MICOM_Sar_Config_Powerdown(B16 bEnable)
{
    U8 u8PDEn = (bEnable)? _SAR_PD : (~_SAR_PD);
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8PDEn;
}

void MHal_MICOM_Sar_Config_Start(B16 bEnable)
{
    U8 u8Start = (bEnable)? _SAR_START : (~_SAR_START);
    MHal_SAR_REG(REG_SAR_CTRL0) = MHal_SAR_REG(REG_SAR_CTRL0) | u8Start;
}

void MHal_MICOM_Sar_Config_ADCPowerdown(B16 bEnable)
{
    U8 u8ADCPDEn = (bEnable)? _SAR_ADC_PD : (~_SAR_ADC_PD); //sar atop power down. 1: power down, 0: enable sar atop
    MHal_SAR_REG(REG_SAR_CTRL1) = MHal_SAR_REG(REG_SAR_CTRL1) | u8ADCPDEn;
}

void MHal_MICOM_Sar_Config_FreeRun(B16 bEnable)
{
    U8 u8Freerun = (bEnable)? _SAR_FREERUN : (~_SAR_FREERUN); //sar atop freerun mode. 0: controlled by digital (default), 1: freerun
    MHal_SAR_REG(REG_SAR_CTRL1) = MHal_SAR_REG(REG_SAR_CTRL1) | u8Freerun;
}

void MHal_MICOM_Sar_Config_Selection(B16 bEnable)
{
    U8 u8Sel = (bEnable)? _SAR_SEL : (~_SAR_SEL);
    MHal_SAR_REG(REG_SAR_CH1_UPB) = MHal_SAR_REG(REG_SAR_CTRL1) | u8Sel;
}

void MHal_MICOM_Sar_Config_8Channel(B16 bEnable)
{
    U8 u8En8Ch = (bEnable)? _SAR_8CH_EN : (~_SAR_8CH_EN); //1: sar 8 channel, 0: sar 4 channel
    MHal_SAR_REG(REG_SAR_CH1_UPB) = MHal_SAR_REG(REG_SAR_CH1_UPB) | u8En8Ch;
}

void MHal_MICOM_Sar_Config_ClockSamplePeriod(U8 u8ClkSmpPrd)
{
    MHal_SAR_REG(REG_SAR_CKSAMP_PRD) = u8ClkSmpPrd;
}

void MHal_MICOM_Sar_Config_IntMask(B16 bEnable)
{
    U8 u8IntMask = (bEnable)? _SAR_INT_MASK : (~_SAR_INT_MASK);
    MHal_SAR_REG(REG_SAR_INT) = MHal_SAR_REG(REG_SAR_INT) | u8IntMask;
}

void MHal_MICOM_Sar_Config_IntClear(B16 bEnable)
{
    U8 u8IntClr = (bEnable)? _SAR_INT_CLR : (~_SAR_INT_CLR);
    MHal_SAR_REG(REG_SAR_INT) = MHal_SAR_REG(REG_SAR_INT) | u8IntClr;
}

void MHal_MICOM_Sar_Config_IntForce(B16 bEnable)
{
    U8 u8IntForce = (bEnable)? _SAR_INT_FORCE : (~_SAR_INT_FORCE);
    MHal_SAR_REG(REG_SAR_INT) = MHal_SAR_REG(REG_SAR_INT) | u8IntForce;
}

B16 MHal_MICOM_Sar_GetIntStatus(void)
{
    U8 u8IntStatus;

    u8IntStatus = MHal_SAR_REG(REG_SAR_INT) & _SAR_INT_STATUS;
    return (u8IntStatus)? ENABLE : DISABLE;
}

U8 MHal_MICOM_Sar_GetChannelADC(U8 u8Channel)
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

