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

#ifndef _REG_PWM_H_
#define _REG_PWM_H_

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------
#define PWM_UNIT_NUM               5

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define REG_MIPS_BASE              0xBF200000//0xBF800000
//the definitions of reg set to initialize
#define REG_ALL_PAD_IN              0x0f50 //set all pads (except SPI) as input

#define REG_PWM_IS_GPIO             0x0f51 //bit 0~4

#define REG_PWM_OEN                 0x0f03  //bit0~4


#define REG_PWM_BASE                0x1900
#define REG_PWM_BANK                REG_PWM_BASE
//???
#define REG_PWM_GRP0_CLK_GATE_EN   (REG_PWM_BASE + 0x01)  //bit0
#define REG_PWM_GRP1_CLK_GATE_EN   (REG_PWM_BASE + 0x01)  //bit1
#define REG_PWM_GRP2_CLK_GATE_EN   (REG_PWM_BASE + 0x01)  //bit2

#define REG_PWM0_PERIOD             (REG_PWM_BASE + 0x02)  //bit0~15
#define REG_PWM1_PERIOD             (REG_PWM_BASE + 0x05)  //bit0~15
#define REG_PWM2_PERIOD             (REG_PWM_BASE + 0x08)  //bit0~15
#define REG_PWM3_PERIOD             (REG_PWM_BASE + 0x0b)  //bit0~15
#define REG_PWM4_PERIOD             (REG_PWM_BASE + 0x0e)  //bit0~15

#define REG_PWM0_DUTY               (REG_PWM_BASE + 0x03)  //bit0~15
#define REG_PWM1_DUTY               (REG_PWM_BASE + 0x06)  //bit0~15
#define REG_PWM2_DUTY               (REG_PWM_BASE + 0x09)  //bit0~15
#define REG_PWM3_DUTY               (REG_PWM_BASE + 0x0c)  //bit0~15
#define REG_PWM4_DUTY               (REG_PWM_BASE + 0x0f)  //bit0~15

#define REG_PWM0_DIV                (REG_PWM_BASE + 0x04)  //bit0~7
#define REG_PWM1_DIV                (REG_PWM_BASE + 0x07)  //bit0~7
#define REG_PWM2_DIV                (REG_PWM_BASE + 0x0a)  //bit0~7
#define REG_PWM3_DIV                (REG_PWM_BASE + 0x0d)  //bit0~7
#define REG_PWM4_DIV                (REG_PWM_BASE + 0x10)  //bit0~7

#define REG_PWM0_PORARITY           (REG_PWM_BASE + 0x04)  //bit8
#define REG_PWM1_PORARITY           (REG_PWM_BASE + 0x07)  //bit8
#define REG_PWM2_PORARITY           (REG_PWM_BASE + 0x0a)  //bit8
#define REG_PWM3_PORARITY           (REG_PWM_BASE + 0x0d)  //bit8
#define REG_PWM4_PORARITY           (REG_PWM_BASE + 0x10)  //bit8

#define REG_PWM0_VDBEN              (REG_PWM_BASE + 0x04)  //bit9
#define REG_PWM1_VDBEN              (REG_PWM_BASE + 0x07)  //bit9
#define REG_PWM2_VDBEN              (REG_PWM_BASE + 0x0a)  //bit9
#define REG_PWM3_VDBEN              (REG_PWM_BASE + 0x0d)  //bit9
#define REG_PWM4_VDBEN              (REG_PWM_BASE + 0x10)  //bit9

#define REG_PWM0_RESET_EN           (REG_PWM_BASE + 0x04)  //bit10
#define REG_PWM1_RESET_EN           (REG_PWM_BASE + 0x07)  //bit10
#define REG_PWM2_RESET_EN           (REG_PWM_BASE + 0x0a)  //bit10
#define REG_PWM3_RESET_EN           (REG_PWM_BASE + 0x0d)  //bit10
#define REG_PWM4_RESET_EN           (REG_PWM_BASE + 0x10)  //bit10

#define REG_PWM0_DBEN               (REG_PWM_BASE + 0x04)  //bit11
#define REG_PWM1_DBEN               (REG_PWM_BASE + 0x07)  //bit11
#define REG_PWM2_DBEN               (REG_PWM_BASE + 0x0a)  //bit11
#define REG_PWM3_DBEN               (REG_PWM_BASE + 0x0d)  //bit11
#define REG_PWM4_DBEN               (REG_PWM_BASE + 0x10)  //bit11

#define REG_PWM0_OEN                (REG_PWM_BASE + 0x04)  //bit15
#define REG_PWM1_OEN                (REG_PWM_BASE + 0x07)  //bit15
#define REG_PWM2_OEN                (REG_PWM_BASE + 0x0a)  //bit15
#define REG_PWM3_OEN                (REG_PWM_BASE + 0x0d)  //bit15
#define REG_PWM4_OEN                (REG_PWM_BASE + 0x10)  //bit15
#define REG_PWM5_OEN                (REG_PWM_BASE + 0x13)  //bit15

#define REG_RST_MUX0                (REG_PWM_BASE + 0x14)  //bit15
#define REG_RST_MUX1                (REG_PWM_BASE + 0x14)  //bit7
#define REG_RST_MUX2                (REG_PWM_BASE + 0x15)  //bit15
#define REG_RST_MUX3                (REG_PWM_BASE + 0x15)  //bit7
#define REG_RST_MUX4                (REG_PWM_BASE + 0x16)  //bit15

#define REG_HS_RST_CNT0             (REG_PWM_BASE + 0x14)  //bit8~11
#define REG_HS_RST_CNT1             (REG_PWM_BASE + 0x14)  //bit0~3
#define REG_HS_RST_CNT2             (REG_PWM_BASE + 0x15)  //bit8~11
#define REG_HS_RST_CNT3             (REG_PWM_BASE + 0x15)  //bit0~3
#define REG_HS_RST_CNT4             (REG_PWM_BASE + 0x16)  //bit8~11

#define REG_PWM0_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit0~1
#define REG_PWM1_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit2~3
#define REG_PWM2_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit4~5
#define REG_PWM3_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit6~7
#define REG_PWM4_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit8~9

#define REG_PWM0_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit0~1
#define REG_PWM1_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit2~3
#define REG_PWM2_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit4~5
#define REG_PWM3_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit6~7
#define REG_PWM4_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit8~9

#define REG_PWM0_DIV_EXT            (REG_PWM_BASE + 0x22)  //bit0~7
#define REG_PWM1_DIV_EXT            (REG_PWM_BASE + 0x22)  //bit8~15
#define REG_PWM2_DIV_EXT            (REG_PWM_BASE + 0x23)  //bit0~7
#define REG_PWM3_DIV_EXT            (REG_PWM_BASE + 0x23)  //bit8~15
#define REG_PWM4_DIV_EXT            (REG_PWM_BASE + 0x24)  //bit0~7

#define REG_PWM0_SHIFT_L            (REG_PWM_BASE + 0x28)
#define REG_PWM0_SHIFT_H            (REG_PWM_BASE + 0x29)
#define REG_PWM1_SHIFT_L            (REG_PWM_BASE + 0x2a)
#define REG_PWM1_SHIFT_H            (REG_PWM_BASE + 0x2b)
#define REG_PWM2_SHIFT_L            (REG_PWM_BASE + 0x2c)
#define REG_PWM2_SHIFT_H            (REG_PWM_BASE + 0x2d)
#define REG_PWM3_SHIFT_L            (REG_PWM_BASE + 0x2e)
#define REG_PWM3_SHIFT_H            (REG_PWM_BASE + 0x2f)
#define REG_PWM4_SHIFT_L            (REG_PWM_BASE + 0x30)
#define REG_PWM4_SHIFT_H            (REG_PWM_BASE + 0x31)

//define PWM number
#define PAD_PWM0        0
#define PAD_PWM1        1
#define PAD_PWM2        2
#define PAD_PWM3        3
#define PAD_PWM4        4

#define MHal_PWM_REG(addr)             (*(volatile U16*)(REG_MIPS_BASE + (addr<<2)))

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    U8  u8Index;
    U16 u16Period;
    U16 u16Duty;
    U16 u16Div;
    B16 b16Oen;
    B16 b16Polarity;
    B16 b16Vdben;
    B16 b16ResetEn;
    B16 b16Dben;
    B16 b16RstMux;
    U8  u8RstCnt;
    B16 bBypassUnit; //T2 U05
    U16 u16PeriodExt; //T3
    U16 u16DutyExt; //T3
    U16 u16DivExt; //T3
    U32 u32Shift; //T3
} PWM_Param_t;

#endif // _REG_PWM_H_

