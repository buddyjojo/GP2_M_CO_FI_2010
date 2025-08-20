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
/// file    madp_pwm.c
/// @brief  Pulse Width Modulation (PWM) Adaptation Layer
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <asm-mips/types.h>

#include "madp_pwm.h"


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define PWM_MODULE_KERNAL_NAME       "/dev/pwm"
#define MOD_PWM_DEVICE_COUNT         1
#define MOD_PWM_NAME                 "ModPWM"

#define PWM_UNIT_NUM                  4

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static S32 s32FdPWM = 0;
static U8 u8InitPWM = FALSE;
static PWM_Param_t PWM_Set;

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
/// PWM initialization
/// @return None
/// @note   Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Init(void)
{
    if (s32FdPWM == 0)   //First time open
    {
        s32FdPWM = open(PWM_MODULE_KERNAL_NAME, O_RDWR);

        if (s32FdPWM)
        {
            ioctl(s32FdPWM, MDRV_PWM_INIT);
        }
        else
        {
            printf("Fail to open PWM Kernal Module\n");
        }
    }
    else
    {
        if(u8InitPWM == FALSE)
        {
            u8InitPWM = TRUE;
            ioctl(s32FdPWM, MDRV_PWM_INIT);
        }
    }
}


//-------------------------------------------------------------------------------------------------
/// Output enable for PWM pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16OenPWM               \b IN:  enable or disable
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Oen(U8 u8IndexPWM, B16 b16OenPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16Oen = b16OenPWM;
    ioctl(s32FdPWM, MDRV_PWM_OEN, &PWM_Set);
}

//-------------------------------------------------------------------------------------------------
/// Set period for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16PeriodPWM            \b IN:  period
/// @return None
/// @note Start at 0. If you set 0, it means period is 1
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Period(U8 u8IndexPWM, U16 u16PeriodPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u16Period = u16PeriodPWM;
    ioctl(s32FdPWM, MDRV_PWM_PERIOD, &PWM_Set);
}

//-------------------------------------------------------------------------------------------------
/// Set duty for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16DutyPWM              \b IN:  duty
/// @return None
/// @note   Start at 0. If you set 0, it means duty is 1
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_DutyCycle(U8 u8IndexPWM, U16 u16DutyPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u16Duty = u16DutyPWM;
    ioctl(s32FdPWM, MDRV_PWM_DUTY, &PWM_Set);
}

//-------------------------------------------------------------------------------------------------
/// Set unit divider for all PWM pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16UnitDivPWM           \b IN:  clock unit divider
/// @return None
/// @note   Start at 0. If you set 0, it means unit divider is 1
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Unit_Div(U16 u16UnitDivPWM)
{
    ioctl(s32FdPWM, MDRV_PWM_UNIT_DIV, &u16UnitDivPWM);
}

//-------------------------------------------------------------------------------------------------
/// Set divider for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u16DivPWM               \b IN:  divider
/// @return None
/// @note   Start at 0. If you set 0, it means divider is 1
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Div(U8 u8IndexPWM, U16 u16DivPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u16Div = u16DivPWM;
    ioctl(s32FdPWM, MDRV_PWM_DIV, &PWM_Set);
}

//-------------------------------------------------------------------------------------------------
/// Set polarity for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16PolPWM               \b IN:  polarity
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Polarity(U8 u8IndexPWM, U16 b16PolPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16Polarity = b16PolPWM;
    ioctl(s32FdPWM, MDRV_PWM_POLARITY, &PWM_Set);
}

//-------------------------------------------------------------------------------------------------
/// Vsync double buffer enable for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16VdbPWM               \b IN:  enable or disbale
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Vdben(U8 u8IndexPWM, U16 b16VdbPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16Vdben = b16VdbPWM;
    ioctl(s32FdPWM, MDRV_PWM_VDBEN, &PWM_Set);
}

//-------------------------------------------------------------------------------------------------
/// Vsync reset for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16VrPWM                \b IN:  enable or disbale
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Reset_En(U8 u8IndexPWM, U16 b16VrPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16ResetEn = b16VrPWM;
    ioctl(s32FdPWM, MDRV_PWM_RESET_EN, &PWM_Set);
}

//-------------------------------------------------------------------------------------------------
/// Double buffer enable for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16DenPWM               \b IN:  enable or disbale
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Dben(U8 u8IndexPWM, U16 b16DenPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16Dben = b16DenPWM;
    ioctl(s32FdPWM, MDRV_PWM_DBEN, &PWM_Set);
}

