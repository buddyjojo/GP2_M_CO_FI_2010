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
/// @file   madp_pwm.h
/// @brief  Pulse Width Modulation (PWM) Adaptation Layer
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_PWM_H_
#define _DRV_PWM_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
/// Parameters for setting PWM device
typedef struct
{
    ///PWM number: PWM0~4
    U8  u8Index;
    ///Set period of PWM
    U16 u16Period;
    ///Set duty of PWM
    U16 u16Duty;
    ///Set divider of PWM
    U16 u16Div;
    ///Set output enable for PWM
    B16 b16Oen;
    ///Set polarity of PWM
    B16 b16Polarity;
    ///Vsync double buffer enable for PWM
    B16 b16Vdben;
    ///Vsync reset enable for PWM
    B16 b16ResetEn;
    ///Double buffer enable for PWM
    B16 b16Dben;
} PWM_Param_t;

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define PWM_IOC_MAGIC               'p'

#define MDRV_PWM_INIT               _IO(PWM_IOC_MAGIC, 0)
#define MDRV_PWM_OEN                _IOW(PWM_IOC_MAGIC, 1, PWM_Param_t)
#define MDRV_PWM_PERIOD             _IOW(PWM_IOC_MAGIC, 2, PWM_Param_t)
#define MDRV_PWM_DUTY               _IOW(PWM_IOC_MAGIC, 3, PWM_Param_t)
#define MDRV_PWM_UNIT_DIV          _IOW(PWM_IOC_MAGIC, 4, U16)
#define MDRV_PWM_DIV                _IOW(PWM_IOC_MAGIC, 5, PWM_Param_t)
#define MDRV_PWM_POLARITY          _IOW(PWM_IOC_MAGIC, 6, PWM_Param_t)
#define MDRV_PWM_VDBEN              _IOW(PWM_IOC_MAGIC, 7, PWM_Param_t)
#define MDRV_PWM_RESET_EN           _IOW(PWM_IOC_MAGIC, 8, PWM_Param_t)
#define MDRV_PWM_DBEN               _IOW(PWM_IOC_MAGIC, 9, PWM_Param_t)

#define PWM_IOC_MAXNR               10

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

void MAdp_PWM_Init(void);
void MAdp_PWM_Oen(U8 u8IndexPWM, B16 b16OenPWM);
void MAdp_PWM_Period(U8 u8IndexPWM, U16 u16PeriodPWM);
void MAdp_PWM_DutyCycle(U8 u8IndexPWM, U16 u16DutyPWM);
void MAdp_PWM_Unit_Div(U16 u16UnitDivPWM);
void MAdp_PWM_Div(U8 u8IndexPWM, U16 u16DivPWM);
void MAdp_PWM_Polarity(U8 u8IndexPWM, U16 b16PolPWM);
void MAdp_PWM_Vdben(U8 u8IndexPWM, U16 b16VdbPWM);
void MAdp_PWM_Reset_En(U8 u8IndexPWM, U16 b16VrPWM);
void MAdp_PWM_Dben(U8 u8IndexPWM, U16 b16DenPWM);

#endif // _DRV_PWM_H_

