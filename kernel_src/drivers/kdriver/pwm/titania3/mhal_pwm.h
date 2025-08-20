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

#ifndef _HAL_PWM_H_
#define _HAL_PWM_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
//the functions of this section set to initialize
extern void MHal_PWM_Init(void);
extern void MHal_PWM_Grp0_Clk_Gate_En(B16 bClkEnPWM);
extern void MHal_PWM_Grp1_Clk_Gate_En(B16 bClkEnPWM);
extern void MHal_PWM_Grp2_Clk_Gate_En(B16 bClkEnPWM);
extern void MHal_PWM0_Oen(B16 b16OenPWM);
extern void MHal_PWM1_Oen(B16 b16OenPWM);
extern void MHal_PWM2_Oen(B16 b16OenPWM);
extern void MHal_PWM3_Oen(B16 b16OenPWM);
extern void MHal_PWM4_Oen(B16 b16OenPWM);
extern void MHal_PWM0_Period(U16 u16PeriodPWM);
extern void MHal_PWM1_Period(U16 u16PeriodPWM);
extern void MHal_PWM2_Period(U16 u16PeriodPWM);
extern void MHal_PWM3_Period(U16 u16PeriodPWM);
extern void MHal_PWM4_Period(U16 u16PeriodPWM);
extern void MHal_PWM0_DutyCycle(U16 u16DutyPWM);
extern void MHal_PWM1_DutyCycle(U16 u16DutyPWM);
extern void MHal_PWM2_DutyCycle(U16 u16DutyPWM);
extern void MHal_PWM3_DutyCycle(U16 u16DutyPWM);
extern void MHal_PWM4_DutyCycle(U16 u16DutyPWM);
extern void MHal_PWM0_Div(U16 u16DivPWM);
extern void MHal_PWM1_Div(U16 u16DivPWM);
extern void MHal_PWM2_Div(U16 u16DivPWM);
extern void MHal_PWM3_Div(U16 u16DivPWM);
extern void MHal_PWM4_Div(U16 u16DivPWM);
extern void MHal_PWM0_Polarity(B16 b16PolPWM);
extern void MHal_PWM1_Polarity(B16 b16PolPWM);
extern void MHal_PWM2_Polarity(B16 b16PolPWM);
extern void MHal_PWM3_Polarity(B16 b16PolPWM);
extern void MHal_PWM4_Polarity(B16 b16PolPWM);
extern void MHal_PWM0_Vdben(B16 b16Vdben);
extern void MHal_PWM1_Vdben(B16 b16Vdben);
extern void MHal_PWM2_Vdben(B16 b16Vdben);
extern void MHal_PWM3_Vdben(B16 b16Vdben);
extern void MHal_PWM4_Vdben(B16 b16Vdben);
extern void MHal_PWM0_Reset_En(B16 b16VrPWM);
extern void MHal_PWM1_Reset_En(B16 b16VrPWM);
extern void MHal_PWM2_Reset_En(B16 b16VrPWM);
extern void MHal_PWM3_Reset_En(B16 b16VrPWM);
extern void MHal_PWM4_Reset_En(B16 b16VrPWM);
extern void MHal_PWM0_Dben(B16 b16DbenPWM);
extern void MHal_PWM1_Dben(B16 b16DbenPWM);
extern void MHal_PWM2_Dben(B16 b16DbenPWM);
extern void MHal_PWM3_Dben(B16 b16DbenPWM);
extern void MHal_PWM4_Dben(B16 b16DbenPWM);
extern void MHal_PWM0_Rst_Mux(B16 b16MuxPWM);
extern void MHal_PWM1_Rst_Mux(B16 b16MuxPWM);
extern void MHal_PWM2_Rst_Mux(B16 b16MuxPWM);
extern void MHal_PWM3_Rst_Mux(B16 b16MuxPWM);
extern void MHal_PWM4_Rst_Mux(B16 b16MuxPWM);
extern void MHal_PWM0_Rst_Cnt(U8 u8RstCntPWM);
extern void MHal_PWM1_Rst_Cnt(U8 u8RstCntPWM);
extern void MHal_PWM2_Rst_Cnt(U8 u8RstCntPWM);
extern void MHal_PWM3_Rst_Cnt(U8 u8RstCntPWM);
extern void MHal_PWM4_Rst_Cnt(U8 u8RstCntPWM);
extern void MHal_PWM0_Period_Ext(U16 u16PeriodExt);
extern void MHal_PWM1_Period_Ext(U16 u16PeriodExt);
extern void MHal_PWM2_Period_Ext(U16 u16PeriodExt);
extern void MHal_PWM3_Period_Ext(U16 u16PeriodExt);
extern void MHal_PWM4_Period_Ext(U16 u16PeriodExt);
extern void MHal_PWM0_Duty_Ext(U16 u16DutyExt);
extern void MHal_PWM1_Duty_Ext(U16 u16DutyExt);
extern void MHal_PWM2_Duty_Ext(U16 u16DutyExt);
extern void MHal_PWM3_Duty_Ext(U16 u16DutyExt);
extern void MHal_PWM4_Duty_Ext(U16 u16DutyExt);
extern void MHal_PWM0_Div_Ext(U16 u16DivExt);
extern void MHal_PWM1_Div_Ext(U16 u16DivExt);
extern void MHal_PWM2_Div_Ext(U16 u16DivExt);
extern void MHal_PWM3_Div_Ext(U16 u16DivExt);
extern void MHal_PWM4_Div_Ext(U16 u16DivExt);
extern void MHal_PWM0_Shift(U32 u32ShiftPWM);
extern void MHal_PWM1_Shift(U32 u32ShiftPWM);
extern void MHal_PWM2_Shift(U32 u32ShiftPWM);
extern void MHal_PWM3_Shift(U32 u32ShiftPWM);
extern void MHal_PWM4_Shift(U32 u32ShiftPWM);

#endif // _HAL_PWM_H_

