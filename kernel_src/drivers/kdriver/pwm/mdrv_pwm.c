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
/// file    mdrv_pwm.c
/// @brief  PWM Driver Interface
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
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mst_devid.h"

#include "mdrv_pwm.h"
#include "mhal_pwm_reg.h"
#include "mhal_pwm.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------


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


//-------------------------------------------------------------------------------------------------
/// PWM chiptop initialization
/// @return None
/// @note   Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Init(void)
{
    MHal_PWM_Init();


}
//-------------------------------------------------------------------------------------------------
/// gating clk for PWM0~2 pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bClkEnPWM               \b IN:  enable or disable
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Grp0_Clk_Gate_En(B16 bClkEnPWM)
{
    MHal_PWM_Grp0_Clk_Gate_En(bClkEnPWM);
}

//-------------------------------------------------------------------------------------------------
/// gating clk for PWM3~5 pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bClkEnPWM               \b IN:  enable or disable
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Grp1_Clk_Gate_En(B16 bClkEnPWM)
{
    MHal_PWM_Grp1_Clk_Gate_En(bClkEnPWM);
}

//-------------------------------------------------------------------------------------------------
/// gating clk for PWM6~8 pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bClkEnPWM               \b IN:  enable or disable
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Grp2_Clk_Gate_En(B16 bClkEnPWM)
{
    MHal_PWM_Grp2_Clk_Gate_En(bClkEnPWM);
}



//-------------------------------------------------------------------------------------------------
/// output enable for PWM pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16OenPWM               \b IN:  enable or disable
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Oen(U8 u8IndexPWM, B16 b16OenPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Oen(b16OenPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Oen(b16OenPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Oen(b16OenPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Oen(b16OenPWM);
            break;
        case PAD_PWM4:
            MHal_PWM4_Oen(b16OenPWM);
            break;
	}
}

//-------------------------------------------------------------------------------------------------
/// set period for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16PeriodPWM            \b IN:  period
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Period(U8 u8IndexPWM, U16 u16PeriodPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Period(u16PeriodPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Period(u16PeriodPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Period(u16PeriodPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Period(u16PeriodPWM);
            break;
        case PAD_PWM4:
            MHal_PWM4_Period(u16PeriodPWM);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/// set duty cycle for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16DutyPWM              \b IN:  duty cycle
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_DutyCycle(U8 u8IndexPWM, U16 u16DutyPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_DutyCycle(u16DutyPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_DutyCycle(u16DutyPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_DutyCycle(u16DutyPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_DutyCycle(u16DutyPWM);
            break;
        case PAD_PWM4:
            MHal_PWM4_DutyCycle(u16DutyPWM);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/// set unit divider for all PWM pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16UnitDivPWM           \b IN:  clock unit divider
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/// set divider for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16DivPWM               \b IN:  divider
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Div(U8 u8IndexPWM, U16 u16DivPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Div(u16DivPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Div(u16DivPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Div(u16DivPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Div(u16DivPWM);
            break;
        case PAD_PWM4:
            MHal_PWM4_Div(u16DivPWM);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/// set polarity for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16PolPWM               \b IN:  polarity
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Polarity(U8 u8IndexPWM, B16 b16PolPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Polarity(b16PolPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Polarity(b16PolPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Polarity(b16PolPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Polarity(b16PolPWM);
            break;
		case PAD_PWM4:
            MHal_PWM4_Polarity(b16PolPWM);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/// vsync double enable for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16VdbPWM               \b IN:  enable or disbale
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Vdben(U8 u8IndexPWM, B16 b16VdbPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Vdben(b16VdbPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Vdben(b16VdbPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Vdben(b16VdbPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Vdben(b16VdbPWM);
            break;
        case PAD_PWM4:
            MHal_PWM4_Vdben(b16VdbPWM);
            break;
	}
}

//-------------------------------------------------------------------------------------------------
/// vsync reset for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16VrPWM                \b IN:  enable or disbale
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Reset_En(U8 u8IndexPWM, B16 b16VrPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Reset_En(b16VrPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Reset_En(b16VrPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Reset_En(b16VrPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Reset_En(b16VrPWM);
            break;
		case PAD_PWM4:
            MHal_PWM4_Reset_En(b16VrPWM);
            break;
	}
}

//-------------------------------------------------------------------------------------------------
/// double enable for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16DenPWM               \b IN:  enable or disbale
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Dben(U8 u8IndexPWM, B16 b16DenPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Dben(b16DenPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Dben(b16DenPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Dben(b16DenPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Dben(b16DenPWM);
            break;
        case PAD_PWM4:
            MHal_PWM4_Dben(b16DenPWM);
            break;
	}
}
//-------------------------------------------------------------------------------------------------
/// reset mux for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16MuxPWM                 \b IN:  enable or disbale
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Rst_Mux(U8 u8IndexPWM, B16 b16MuxPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Rst_Mux(b16MuxPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Rst_Mux(b16MuxPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Rst_Mux(b16MuxPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Rst_Mux(b16MuxPWM);
            break;
		case PAD_PWM4:
            MHal_PWM4_Rst_Mux(b16MuxPWM);
            break;

	}
}

//-------------------------------------------------------------------------------------------------
/// Hsync reset counter for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u8RstCntPWM             \b IN:  Hsync reset counter
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Rst_Cnt(U8 u8IndexPWM, U8 u8RstCntPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Rst_Cnt(u8RstCntPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Rst_Cnt(u8RstCntPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Rst_Cnt(u8RstCntPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Rst_Cnt(u8RstCntPWM);
            break;
		case PAD_PWM4:
            MHal_PWM4_Rst_Cnt(u8RstCntPWM);
            break;

	}
}

//-------------------------------------------------------------------------------------------------
/// set extra period for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16PeriodExt            \b IN:  extra period
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Period_Ext(U8 u8IndexPWM, U16 u16PeriodExt)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Period_Ext(u16PeriodExt);
            break;
        case PAD_PWM1:
            MHal_PWM1_Period_Ext(u16PeriodExt);
            break;
        case PAD_PWM2:
            MHal_PWM2_Period_Ext(u16PeriodExt);
            break;
        case PAD_PWM3:
            MHal_PWM3_Period_Ext(u16PeriodExt);
            break;
        case PAD_PWM4:
            MHal_PWM4_Period_Ext(u16PeriodExt);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/// set extra duty for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16DutyPWM              \b IN:  duty cycle
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Duty_Ext(U8 u8IndexPWM, U16 u16DutyExt)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Duty_Ext(u16DutyExt);
            break;
        case PAD_PWM1:
            MHal_PWM1_Duty_Ext(u16DutyExt);
            break;
        case PAD_PWM2:
            MHal_PWM2_Duty_Ext(u16DutyExt);
            break;
        case PAD_PWM3:
            MHal_PWM3_Duty_Ext(u16DutyExt);
            break;
        case PAD_PWM4:
            MHal_PWM4_Duty_Ext(u16DutyExt);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/// set extra divider for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16DivExt               \b IN:  extra divider
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Div_Ext(U8 u8IndexPWM, U16 u16DivExt)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Div_Ext(u16DivExt);
            break;
        case PAD_PWM1:
            MHal_PWM1_Div_Ext(u16DivExt);
            break;
        case PAD_PWM2:
            MHal_PWM2_Div_Ext(u16DivExt);
            break;
        case PAD_PWM3:
            MHal_PWM3_Div_Ext(u16DivExt);
            break;
        case PAD_PWM4:
            MHal_PWM4_Div_Ext(u16DivExt);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/// set rising point shift counter for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u32ShiftPWM             \b IN:  rising point shift counter
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_PWM_Shift(U8 u8IndexPWM, U32 u32ShiftPWM)
{
    switch(u8IndexPWM)
    {
        case PAD_PWM0:
            MHal_PWM0_Shift(u32ShiftPWM);
            break;
        case PAD_PWM1:
            MHal_PWM1_Shift(u32ShiftPWM);
            break;
        case PAD_PWM2:
            MHal_PWM2_Shift(u32ShiftPWM);
            break;
        case PAD_PWM3:
            MHal_PWM3_Shift(u32ShiftPWM);
            break;
        case PAD_PWM4:
            MHal_PWM4_Shift(u32ShiftPWM);
            break;
    }
}


