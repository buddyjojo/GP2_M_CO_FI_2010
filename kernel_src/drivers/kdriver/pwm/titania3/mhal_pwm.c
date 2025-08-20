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

#include "mhal_pwm.h"
#include "mhal_pwm_reg.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------


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

//the functions of this section set to initialize
void MHal_PWM_Init(void)
{
    MHal_PWM_REG(REG_ALL_PAD_IN) &= ~BIT15;
#if 0	
    MHal_PWM_REG(REG_PWM_IS_GPIO) &= ~(BIT0|BIT1|BIT2|BIT3|BIT4);
    MHal_PWM_REG(REG_PWM_OEN) &= ~(BIT0|BIT1|BIT2|BIT3|BIT4);
#else
//ieeum ONLY PWM0,2 is used.
    MHal_PWM_REG(REG_PWM_IS_GPIO) &= ~(BIT0|BIT2);
    MHal_PWM_REG(REG_PWM_OEN) &= ~(BIT0|BIT2);
#endif
}

void MHal_PWM_Grp0_Clk_Gate_En(B16 bClkEnPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(bClkEnPWM == TRUE)
        MHal_PWM_REG(REG_PWM_GRP0_CLK_GATE_EN) |= BIT0;
    else
        MHal_PWM_REG(REG_PWM_GRP0_CLK_GATE_EN) &= ~BIT0;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM_Grp1_Clk_Gate_En(B16 bClkEnPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(bClkEnPWM == TRUE)
        MHal_PWM_REG(REG_PWM_GRP1_CLK_GATE_EN) |= BIT1;
    else
        MHal_PWM_REG(REG_PWM_GRP1_CLK_GATE_EN) &= ~BIT1;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM_Grp2_Clk_Gate_En(B16 bClkEnPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(bClkEnPWM == TRUE)
        MHal_PWM_REG(REG_PWM_GRP2_CLK_GATE_EN) |= BIT2;
    else
        MHal_PWM_REG(REG_PWM_GRP2_CLK_GATE_EN) &= ~BIT2;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM0_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM0_OEN) |= BIT15;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM1_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM1_OEN) |= BIT15;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM2_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM2_OEN) |= BIT15;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM3_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM3_OEN) |= BIT15;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM4_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM4_OEN) |= BIT15;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_PERIOD) = u16PeriodPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM1_PERIOD) = u16PeriodPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM2_PERIOD) = u16PeriodPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM3_PERIOD) = u16PeriodPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM4_PERIOD) = u16PeriodPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_DUTY) = u16DutyPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM1_DUTY) = u16DutyPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM2_DUTY) = u16DutyPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM3_DUTY) = u16DutyPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM4_DUTY) = u16DutyPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM0_DIV) = (MHal_PWM_REG(REG_PWM0_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM0_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM0_DIV) |= u16DivPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM1_DIV) = (MHal_PWM_REG(REG_PWM1_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM1_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM1_DIV) |= u16DivPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM2_DIV) = ((MHal_PWM_REG(REG_PWM2_DIV) & (0xff00)) | (u16DivPWM & 0xff));
//  MHal_PWM_REG(REG_PWM2_DIV) &= 0xff00;
//  MHal_PWM_REG(REG_PWM2_DIV) |= u16DivPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM3_DIV) = (MHal_PWM_REG(REG_PWM3_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM3_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM3_DIV) |= u16DivPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM4_DIV) = (MHal_PWM_REG(REG_PWM4_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM4_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM4_DIV) |= u16DivPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM0_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM0_PORARITY) &= ~BIT8;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM1_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM1_PORARITY) &= ~BIT8;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM2_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM2_PORARITY) &= ~BIT8;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM3_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM3_PORARITY) &= ~BIT8;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM4_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM4_PORARITY) &= ~BIT8;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
    {
        MHal_PWM_REG(REG_PWM0_VDBEN) |= BIT9;
        MHal_PWM_REG(REG_PWM0_RESET_EN) |= BIT10;
    }
    else
    {
        MHal_PWM_REG(REG_PWM0_VDBEN) &= ~BIT9;
        MHal_PWM_REG(REG_PWM0_RESET_EN) &= ~BIT10;
    }
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
    {
        MHal_PWM_REG(REG_PWM1_VDBEN) |= BIT9;
        MHal_PWM_REG(REG_PWM1_RESET_EN) |= BIT10;
    }
    else
    {
        MHal_PWM_REG(REG_PWM1_VDBEN) &= ~BIT9;
        MHal_PWM_REG(REG_PWM1_RESET_EN) &= ~BIT10;
    }
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
    {
        MHal_PWM_REG(REG_PWM2_VDBEN) |= BIT9;
        MHal_PWM_REG(REG_PWM2_RESET_EN) |= BIT10;
    }
    else
    {
        MHal_PWM_REG(REG_PWM2_VDBEN) &= ~BIT9;
        MHal_PWM_REG(REG_PWM2_RESET_EN) &= ~BIT10;
    }
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
    {
        MHal_PWM_REG(REG_PWM3_VDBEN) |= BIT9;
        MHal_PWM_REG(REG_PWM3_RESET_EN) |= BIT10;
    }
    else
    {
        MHal_PWM_REG(REG_PWM3_VDBEN) &= ~BIT9;
        MHal_PWM_REG(REG_PWM3_RESET_EN) &= ~BIT10;
    }
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
    {
        MHal_PWM_REG(REG_PWM4_VDBEN) |= BIT9;
        MHal_PWM_REG(REG_PWM4_RESET_EN) |= BIT10;
    }
    else
    {
        MHal_PWM_REG(REG_PWM4_VDBEN) &= ~BIT9;
        MHal_PWM_REG(REG_PWM4_RESET_EN) &= ~BIT10;
    }
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM0_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM0_RESET_EN) &= ~BIT10;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM1_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM1_RESET_EN) &= ~BIT10;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM2_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM2_RESET_EN) &= ~BIT10;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM3_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM3_RESET_EN) &= ~BIT10;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM4_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM4_RESET_EN) &= ~BIT10;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM0_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM0_DBEN) &= ~BIT11;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM1_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM1_DBEN) &= ~BIT11;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM2_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM2_DBEN) &= ~BIT11;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM3_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM3_DBEN) &= ~BIT11;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM4_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM4_DBEN) &= ~BIT11;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX0) |= BIT15;
    else
        MHal_PWM_REG(REG_RST_MUX0) &= ~BIT15;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX1) |= BIT7;
    else
        MHal_PWM_REG(REG_RST_MUX1) &= ~BIT7;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX2) |= BIT15;
    else
        MHal_PWM_REG(REG_RST_MUX2) &= ~BIT15;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX3) |= BIT7;
    else
        MHal_PWM_REG(REG_RST_MUX3) &= ~BIT7;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX4) |= BIT15;
    else
        MHal_PWM_REG(REG_RST_MUX4) &= ~BIT15;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16)(u8RstCntPWM & 0x0f);
    u16RstCntPWM = (u16RstCntPWM << 8);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT0) &= 0xf0ff;
    MHal_PWM_REG(REG_HS_RST_CNT0) |= u16RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16)(u8RstCntPWM & 0x0f);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT1) &= 0xfff0;
    MHal_PWM_REG(REG_HS_RST_CNT1) |= u16RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16)(u8RstCntPWM & 0x0f);
    u16RstCntPWM = (u16RstCntPWM << 8);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT2) &= 0xf0ff;
    MHal_PWM_REG(REG_HS_RST_CNT2) |= u16RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16)(u8RstCntPWM & 0x0f);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT3) &= 0xfff0;
    MHal_PWM_REG(REG_HS_RST_CNT3) |= u16RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16)(u8RstCntPWM & 0x0f);
    u16RstCntPWM = (u16RstCntPWM << 8);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT4) &= 0xf0ff;
    MHal_PWM_REG(REG_HS_RST_CNT4) |= u16RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM0_PERIOD_EXT)  & ~0x0003) | (u16PeriodExt & 0x0003);
//    MHal_PWM_REG(REG_PWM0_PERIOD_EXT) &= ~0x0003;
//    MHal_PWM_REG(REG_PWM0_PERIOD_EXT) |= (u16PeriodExt & 0x0003);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 2);
    MHal_PWM_REG(REG_PWM1_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM1_PERIOD_EXT)  & ~0x000C) | (u16PeriodExt & 0x000C);
//    MHal_PWM_REG(REG_PWM1_PERIOD_EXT) &= ~0x000C;
//    MHal_PWM_REG(REG_PWM1_PERIOD_EXT) |= (u16PeriodExt & 0x000C);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 4);	
    MHal_PWM_REG(REG_PWM2_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM2_PERIOD_EXT)  & ~0x0030) | (u16PeriodExt & 0x0030);
//    MHal_PWM_REG(REG_PWM2_PERIOD_EXT) &= ~0x0030;
//    MHal_PWM_REG(REG_PWM2_PERIOD_EXT) |= (u16PeriodExt & 0x0030);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 6);
    MHal_PWM_REG(REG_PWM3_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM3_PERIOD_EXT)  & ~0x00C0) | (u16PeriodExt & 0x00C0);	
//    MHal_PWM_REG(REG_PWM3_PERIOD_EXT) &= ~0x00C0;
//    MHal_PWM_REG(REG_PWM3_PERIOD_EXT) |= (u16PeriodExt & 0x00C0);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 8);
    MHal_PWM_REG(REG_PWM4_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM4_PERIOD_EXT)  & ~0x0300) | (u16PeriodExt & 0x0300);	
//    MHal_PWM_REG(REG_PWM4_PERIOD_EXT) &= ~0x0300;
//    MHal_PWM_REG(REG_PWM4_PERIOD_EXT) |= (u16PeriodExt & 0x0300);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_DUTY_EXT) = (MHal_PWM_REG(REG_PWM0_DUTY_EXT)  & ~0x0003) | (u16DutyExt & 0x0003);
//    MHal_PWM_REG(REG_PWM0_DUTY_EXT) &= ~0x0003;
//    MHal_PWM_REG(REG_PWM0_DUTY_EXT) |= (u16DutyExt & 0x0003);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 2);
    MHal_PWM_REG(REG_PWM1_DUTY_EXT) = (MHal_PWM_REG(REG_PWM1_DUTY_EXT)  & ~0x000C) | (u16DutyExt & 0x000C);
//    MHal_PWM_REG(REG_PWM1_DUTY_EXT) &= ~0x000C;
//    MHal_PWM_REG(REG_PWM1_DUTY_EXT) |= (u16DutyExt & 0x000C);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 4);
    MHal_PWM_REG(REG_PWM2_DUTY_EXT) = (MHal_PWM_REG(REG_PWM2_DUTY_EXT)  & ~0x0030) | (u16DutyExt & 0x0030);
//    MHal_PWM_REG(REG_PWM2_DUTY_EXT) &= ~0x0030;
//    MHal_PWM_REG(REG_PWM2_DUTY_EXT) |= (u16DutyExt & 0x0030);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 6);
    MHal_PWM_REG(REG_PWM3_DUTY_EXT) = (MHal_PWM_REG(REG_PWM3_DUTY_EXT)  & ~0x00C0) | (u16DutyExt & 0x00C0);
//    MHal_PWM_REG(REG_PWM3_DUTY_EXT) &= ~0x00C0;
//    MHal_PWM_REG(REG_PWM3_DUTY_EXT) |= (u16DutyExt & 0x00C0);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 8);
    MHal_PWM_REG(REG_PWM4_DUTY_EXT) = (MHal_PWM_REG(REG_PWM4_DUTY_EXT)  & ~0x0300) | (u16DutyExt & 0x0300);
//    MHal_PWM_REG(REG_PWM4_DUTY_EXT) &= ~0x0300;
//    MHal_PWM_REG(REG_PWM4_DUTY_EXT) |= (u16DutyExt & 0x0300);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_DIV_EXT) = (MHal_PWM_REG(REG_PWM0_DIV_EXT)  & ~0x00FF) | (u16DivExt & 0x00FF); 
//	MHal_PWM_REG(REG_PWM0_DIV_EXT) &= ~0x00FF;
//    MHal_PWM_REG(REG_PWM0_DIV_EXT) |= (u16DivExt & 0x00FF);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DivExt = (u16DivExt << 8);
    MHal_PWM_REG(REG_PWM1_DIV_EXT) = (MHal_PWM_REG(REG_PWM1_DIV_EXT)  & ~0xFF00) | (u16DivExt & 0xFF00); 
//    MHal_PWM_REG(REG_PWM1_DIV_EXT) &= ~0xFF00;
//    MHal_PWM_REG(REG_PWM1_DIV_EXT) |= (u16DivExt & 0xFF00);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM2_DIV_EXT) = (MHal_PWM_REG(REG_PWM2_DIV_EXT)  & ~0x00FF) | (u16DivExt & 0x00FF); 
//    MHal_PWM_REG(REG_PWM2_DIV_EXT) &= ~0x00FF;
//    MHal_PWM_REG(REG_PWM2_DIV_EXT) |= (u16DivExt & 0x00FF);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DivExt = (u16DivExt << 8);
    MHal_PWM_REG(REG_PWM3_DIV_EXT) = (MHal_PWM_REG(REG_PWM3_DIV_EXT)  & ~0xFF00) | (u16DivExt & 0xFF00); 
//    MHal_PWM_REG(REG_PWM3_DIV_EXT) &= ~0xFF00;
//    MHal_PWM_REG(REG_PWM3_DIV_EXT) |= (u16DivExt & 0xFF00);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM4_DIV_EXT) = (MHal_PWM_REG(REG_PWM4_DIV_EXT)  & ~0x00FF) | (u16DivExt & 0x00FF); 
//    MHal_PWM_REG(REG_PWM4_DIV_EXT) &= ~0x00FF;
//    MHal_PWM_REG(REG_PWM4_DIV_EXT) |= (u16DivExt & 0x00FF);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM0_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM0_SHIFT_H) = (U16)((u32ShiftPWM >> 16) & 0x0003);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM1_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM1_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM1_SHIFT_H) = (U16)((u32ShiftPWM >> 16) & 0x0003);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM2_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM2_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM2_SHIFT_H) = (U16)((u32ShiftPWM >> 16) & 0x0003);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM3_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM3_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM3_SHIFT_H) = (U16)((u32ShiftPWM >> 16) & 0x0003);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

void MHal_PWM4_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM4_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM4_SHIFT_H) = (U16)((u32ShiftPWM >> 16) & 0x0003);
    MHal_PWM_REG(REG_PWM_BANK) = 0x00;
}

