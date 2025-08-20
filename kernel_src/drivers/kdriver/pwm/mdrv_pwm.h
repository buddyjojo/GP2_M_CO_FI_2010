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
/// @file   mdrv_pwm.h
/// @brief  PWM Driver Interface
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


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

void MDrv_PWM_Init(void);

void MDrv_PWM_Grp0_Clk_Gate_En(B16 bClkEnPWM);
void MDrv_PWM_Grp1_Clk_Gate_En(B16 bClkEnPWM);
void MDrv_PWM_Grp2_Clk_Gate_En(B16 bClkEnPWM);
void MDrv_PWM_Oen(U8 u8IndexPWM, B16 b16OenPWM);
void MDrv_PWM_Period(U8 u8IndexPWM, U16 u16PeriodPWM);
void MDrv_PWM_DutyCycle(U8 u8IndexPWM, U16 u16DutyPWM);
//void MDrv_PWM_Unit_Div(U16 u16DivPWM);
void MDrv_PWM_Div(U8 u8IndexPWM, U16 u16DivPWM);
void MDrv_PWM_Polarity(U8 u8IndexPWM, B16 b16PolPWM);
void MDrv_PWM_Vdben(U8 u8IndexPWM, B16 b16VdbPWM);
void MDrv_PWM_Reset_En(U8 u8IndexPWM, B16 b16VrPWM);
void MDrv_PWM_Dben(U8 u8IndexPWM, B16 b16DenPWM);
void MDrv_PWM_Rst_Mux(U8 u8IndexPWM, B16 b16MuxPWM);
void MDrv_PWM_Rst_Cnt(U8 u8IndexPWM, U8 u8RstCntPWM);
void MDrv_PWM_Period_Ext(U8 u8IndexPWM, U16 u16PeriodExt);
void MDrv_PWM_Duty_Ext(U8 u8IndexPWM, U16 u16DutyExt);
void MDrv_PWM_Div_Ext(U8 u8IndexPWM, U16 u16DivExt);
void MDrv_PWM_Shift(U8 u8IndexPWM, U32 u32ShiftPWM);

#endif // _DRV_PWM_H_

