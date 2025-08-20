////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¨MStar Confidential Information〃) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////


/******************************************************************************/
/*                    Header Files                        */
/* ****************************************************************************/
//#include <stdio.h>
#include "mdrv_pwm.h"
#include "nvm.h" //balup_pwm
#include "mdrv_scaler.h"

#define PWM_UNIT_NUM               9

#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000

#define   FALSE                         0
#define   TRUE                          1
#define   PWM2_OUT
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
#define REG_PWM5_PERIOD             (REG_PWM_BASE + 0x11)  //bit0~15
#define REG_PWM6_PERIOD             (REG_PWM_BASE + 0x40)  //bit0~15
#define REG_PWM7_PERIOD             (REG_PWM_BASE + 0x43)  //bit0~15
#define REG_PWM8_PERIOD             (REG_PWM_BASE + 0x46)  //bit0~15

#define REG_PWM0_DUTY               (REG_PWM_BASE + 0x03)  //bit0~15
#define REG_PWM1_DUTY               (REG_PWM_BASE + 0x06)  //bit0~15
#define REG_PWM2_DUTY               (REG_PWM_BASE + 0x09)  //bit0~15
#define REG_PWM3_DUTY               (REG_PWM_BASE + 0x0c)  //bit0~15
#define REG_PWM4_DUTY               (REG_PWM_BASE + 0x0f)  //bit0~15
#define REG_PWM5_DUTY               (REG_PWM_BASE + 0x12)  //bit0~15
#define REG_PWM6_DUTY               (REG_PWM_BASE + 0x41)  //bit0~15
#define REG_PWM7_DUTY               (REG_PWM_BASE + 0x44)  //bit0~15
#define REG_PWM8_DUTY               (REG_PWM_BASE + 0x47)  //bit0~15

#define REG_PWM0_DIV                (REG_PWM_BASE + 0x04)  //bit0~7
#define REG_PWM1_DIV                (REG_PWM_BASE + 0x07)  //bit0~7
#define REG_PWM2_DIV                (REG_PWM_BASE + 0x0a)  //bit0~7
#define REG_PWM3_DIV                (REG_PWM_BASE + 0x0d)  //bit0~7
#define REG_PWM4_DIV                (REG_PWM_BASE + 0x10)  //bit0~7
#define REG_PWM5_DIV                (REG_PWM_BASE + 0x13)  //bit0~7
#define REG_PWM6_DIV                (REG_PWM_BASE + 0x42)  //bit0~7
#define REG_PWM7_DIV                (REG_PWM_BASE + 0x45)  //bit0~7
#define REG_PWM8_DIV                (REG_PWM_BASE + 0x48)  //bit0~7

#define REG_PWM0_PORARITY           (REG_PWM_BASE + 0x04)  //bit8
#define REG_PWM1_PORARITY           (REG_PWM_BASE + 0x07)  //bit8
#define REG_PWM2_PORARITY           (REG_PWM_BASE + 0x0a)  //bit8
#define REG_PWM3_PORARITY           (REG_PWM_BASE + 0x0d)  //bit8
#define REG_PWM4_PORARITY           (REG_PWM_BASE + 0x10)  //bit8
#define REG_PWM5_PORARITY           (REG_PWM_BASE + 0x13)  //bit8
#define REG_PWM6_PORARITY           (REG_PWM_BASE + 0x42)  //bit8
#define REG_PWM7_PORARITY           (REG_PWM_BASE + 0x45)  //bit8
#define REG_PWM8_PORARITY           (REG_PWM_BASE + 0x48)  //bit8

#define REG_PWM0_VDBEN              (REG_PWM_BASE + 0x04)  //bit9
#define REG_PWM1_VDBEN              (REG_PWM_BASE + 0x07)  //bit9
#define REG_PWM2_VDBEN              (REG_PWM_BASE + 0x0a)  //bit9
#define REG_PWM3_VDBEN              (REG_PWM_BASE + 0x0d)  //bit9
#define REG_PWM4_VDBEN              (REG_PWM_BASE + 0x10)  //bit9
#define REG_PWM5_VDBEN              (REG_PWM_BASE + 0x13)  //bit9
#define REG_PWM6_VDBEN              (REG_PWM_BASE + 0x42)  //bit9
#define REG_PWM7_VDBEN              (REG_PWM_BASE + 0x45)  //bit9
#define REG_PWM8_VDBEN              (REG_PWM_BASE + 0x48)  //bit9

#define REG_PWM0_RESET_EN           (REG_PWM_BASE + 0x04)  //bit10
#define REG_PWM1_RESET_EN           (REG_PWM_BASE + 0x07)  //bit10
#define REG_PWM2_RESET_EN           (REG_PWM_BASE + 0x0a)  //bit10
#define REG_PWM3_RESET_EN           (REG_PWM_BASE + 0x0d)  //bit10
#define REG_PWM4_RESET_EN           (REG_PWM_BASE + 0x10)  //bit10
#define REG_PWM5_RESET_EN           (REG_PWM_BASE + 0x13)  //bit10
#define REG_PWM6_RESET_EN           (REG_PWM_BASE + 0x42)  //bit10
#define REG_PWM7_RESET_EN           (REG_PWM_BASE + 0x45)  //bit10
#define REG_PWM8_RESET_EN           (REG_PWM_BASE + 0x48)  //bit10

#define REG_PWM0_DBEN               (REG_PWM_BASE + 0x04)  //bit11
#define REG_PWM1_DBEN               (REG_PWM_BASE + 0x07)  //bit11
#define REG_PWM2_DBEN               (REG_PWM_BASE + 0x0a)  //bit11
#define REG_PWM3_DBEN               (REG_PWM_BASE + 0x0d)  //bit11
#define REG_PWM4_DBEN               (REG_PWM_BASE + 0x10)  //bit11
#define REG_PWM5_DBEN               (REG_PWM_BASE + 0x13)  //bit11
#define REG_PWM6_DBEN               (REG_PWM_BASE + 0x42)  //bit11
#define REG_PWM7_DBEN               (REG_PWM_BASE + 0x45)  //bit11
#define REG_PWM8_DBEN               (REG_PWM_BASE + 0x48)  //bit11

#define REG_PWM0_OEN                (REG_PWM_BASE + 0x04)  //bit15
#define REG_PWM1_OEN                (REG_PWM_BASE + 0x07)  //bit15
#define REG_PWM2_OEN                (REG_PWM_BASE + 0x0a)  //bit15
#define REG_PWM3_OEN                (REG_PWM_BASE + 0x0d)  //bit15
#define REG_PWM4_OEN                (REG_PWM_BASE + 0x10)  //bit15
#define REG_PWM5_OEN                (REG_PWM_BASE + 0x13)  //bit15
#define REG_PWM6_OEN                (REG_PWM_BASE + 0x42)  //bit15
#define REG_PWM7_OEN                (REG_PWM_BASE + 0x45)  //bit15
#define REG_PWM8_OEN                (REG_PWM_BASE + 0x48)  //bit15

#define REG_RST_MUX0                (REG_PWM_BASE + 0x14)  //bit15
#define REG_RST_MUX1                (REG_PWM_BASE + 0x14)  //bit7
#define REG_RST_MUX2                (REG_PWM_BASE + 0x15)  //bit15
#define REG_RST_MUX3                (REG_PWM_BASE + 0x15)  //bit7
#define REG_RST_MUX4                (REG_PWM_BASE + 0x16)  //bit15
#define REG_RST_MUX5                (REG_PWM_BASE + 0x16)  //bit7
#define REG_RST_MUX6                (REG_PWM_BASE + 0x49)  //bit15
#define REG_RST_MUX7                (REG_PWM_BASE + 0x49)  //bit7
#define REG_RST_MUX8                (REG_PWM_BASE + 0x4a)  //bit15

#define REG_HS_RST_CNT0             (REG_PWM_BASE + 0x14)  //bit8~11
#define REG_HS_RST_CNT1             (REG_PWM_BASE + 0x14)  //bit0~3
#define REG_HS_RST_CNT2             (REG_PWM_BASE + 0x15)  //bit8~11
#define REG_HS_RST_CNT3             (REG_PWM_BASE + 0x15)  //bit0~3
#define REG_HS_RST_CNT4             (REG_PWM_BASE + 0x16)  //bit8~11
#define REG_HS_RST_CNT5             (REG_PWM_BASE + 0x16)  //bit0~3
#define REG_HS_RST_CNT6             (REG_PWM_BASE + 0x49)  //bit8~11
#define REG_HS_RST_CNT7             (REG_PWM_BASE + 0x49)  //bit0~3
#define REG_HS_RST_CNT8             (REG_PWM_BASE + 0x4a)  //bit8~11

#define REG_PWM0_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit0~1
#define REG_PWM1_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit2~3
#define REG_PWM2_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit4~5
#define REG_PWM3_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit6~7
#define REG_PWM4_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit8~9
#define REG_PWM5_PERIOD_EXT         (REG_PWM_BASE + 0x20)  //bit10~11
#define REG_PWM6_PERIOD_EXT         (REG_PWM_BASE + 0x4b)  //bit0~1
#define REG_PWM7_PERIOD_EXT         (REG_PWM_BASE + 0x4b)  //bit2~3
#define REG_PWM8_PERIOD_EXT         (REG_PWM_BASE + 0x4b)  //bit4~5

#define REG_PWM0_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit0~1
#define REG_PWM1_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit2~3
#define REG_PWM2_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit4~5
#define REG_PWM3_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit6~7
#define REG_PWM4_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit8~9
#define REG_PWM5_DUTY_EXT           (REG_PWM_BASE + 0x21)  //bit10~11
#define REG_PWM6_DUTY_EXT           (REG_PWM_BASE + 0x4b)  //bit8~9
#define REG_PWM7_DUTY_EXT           (REG_PWM_BASE + 0x4b)  //bit10~11
#define REG_PWM8_DUTY_EXT           (REG_PWM_BASE + 0x4b)  //bit12~13

#define REG_PWM0_DIV_EXT            (REG_PWM_BASE + 0x22)  //bit0~7
#define REG_PWM1_DIV_EXT            (REG_PWM_BASE + 0x22)  //bit8~15
#define REG_PWM2_DIV_EXT            (REG_PWM_BASE + 0x23)  //bit0~7
#define REG_PWM3_DIV_EXT            (REG_PWM_BASE + 0x23)  //bit8~15
#define REG_PWM4_DIV_EXT            (REG_PWM_BASE + 0x24)  //bit0~7
#define REG_PWM5_DIV_EXT            (REG_PWM_BASE + 0x24)  //bit8~15
#define REG_PWM6_DIV_EXT            (REG_PWM_BASE + 0x4c)  //bit0~7
#define REG_PWM7_DIV_EXT            (REG_PWM_BASE + 0x4c)  //bit8~15
#define REG_PWM8_DIV_EXT            (REG_PWM_BASE + 0x4d)  //bit0~7

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
#define REG_PWM5_SHIFT_L            (REG_PWM_BASE + 0x32)
#define REG_PWM5_SHIFT_H            (REG_PWM_BASE + 0x33)
#define REG_PWM6_SHIFT_L            (REG_PWM_BASE + 0x4e)
#define REG_PWM6_SHIFT_H            (REG_PWM_BASE + 0x4f)
#define REG_PWM7_SHIFT_L            (REG_PWM_BASE + 0x50)
#define REG_PWM7_SHIFT_H            (REG_PWM_BASE + 0x51)
#define REG_PWM8_SHIFT_L            (REG_PWM_BASE + 0x52)
#define REG_PWM8_SHIFT_H            (REG_PWM_BASE + 0x53)

//define PWM number
#define PAD_PWM0        0
#define PAD_PWM1        1
#define PAD_PWM2        2
#define PAD_PWM3        3
#define PAD_PWM4        4
#define PAD_PWM5        5
#define PAD_PWM6        6
#define PAD_PWM7        7
#define PAD_PWM8        8

#define MHal_PWM_REG(addr)             (*(volatile U16*)(REG_MIPS_BASE + (addr<<2)))


typedef enum{
	FRC_OPT_SEL,
	FRC_OPT1_SEL,
	LVDS_OPT_SEL,
	DDRSIZE_OPT_SEL,
	PANEL_RES_OPT_SEL,
	GIP_OPT_SEL,
	OLED_SEL,
	MICOM_HW_OPT_SEL,
}HW_OPT_T;

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
/*
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
*/
typedef enum 
{
	OK					= 0,	/**< success (no error) */
	NOT_OK				= -1,	/**< generic error */
	INVALID_PARAMS		= -2,	/**< input parameter error */
	NOT_ENOUGH_RESOURCE = -3,	/**< not enough resource */
	NOT_SUPPORTED		= -4,	/**< not supported */
	NOT_PERMITTED		= -5,	/**< not permitted */
	TIMEOUT 			= -6,	/**< timeout */
	NO_DATA_RECEIVED	= -7,	/**< no data received */
	DN_BUF_OVERFLOW 	= -8	/**< buffer overflow error */
} DTV_STATUS_T;


typedef struct
{
    ///PWM number: PWM0~8
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
    ///Reset mux for PWM
    B16 b16RstMux;
    ///HSync reset counter for PWM
    U8  u8RstCnt;
    ///Bypass unit divider for PWM (T2 U05)
    B16 b16BypassUnit;
    ///Extra period of PWM (T3)
    U16 u16PeriodExt;
    ///Extra duty of PWM (T3)
    U16 u16DutyExt;
    ///Exrea divider of PWM (T3)
    U16 u16DivExt;
    ///Rising point shift counter of PWM (T3)
    U32 u32Shift; //T3

} PWM_Param_t;

//static S32 s32FdPWM = 0;
//static U8 u8InitPWM = FALSE;
static PWM_Param_t PWM_Set;

void MHal_PWM_Init(void);
void MHal_PWM_Grp0_Clk_Gate_En(B16 bClkEnPWM);
void MHal_PWM_Grp1_Clk_Gate_En(B16 bClkEnPWM);
void MHal_PWM_Grp2_Clk_Gate_En(B16 bClkEnPWM);
void MHal_PWM0_Oen(B16 b16OenPWM);
void MHal_PWM1_Oen(B16 b16OenPWM);
void MHal_PWM2_Oen(B16 b16OenPWM);
void MHal_PWM3_Oen(B16 b16OenPWM);
void MHal_PWM4_Oen(B16 b16OenPWM);
void MHal_PWM5_Oen(B16 b16OenPWM);
void MHal_PWM6_Oen(B16 b16OenPWM);
void MHal_PWM7_Oen(B16 b16OenPWM);
void MHal_PWM8_Oen(B16 b16OenPWM);
void MHal_PWM0_Period(U16 u16PeriodPWM);
void MHal_PWM1_Period(U16 u16PeriodPWM);
void MHal_PWM2_Period(U16 u16PeriodPWM);
void MHal_PWM3_Period(U16 u16PeriodPWM);
void MHal_PWM4_Period(U16 u16PeriodPWM);
void MHal_PWM5_Period(U16 u16PeriodPWM);
void MHal_PWM6_Period(U16 u16PeriodPWM);
void MHal_PWM7_Period(U16 u16PeriodPWM);
void MHal_PWM8_Period(U16 u16PeriodPWM);
void MHal_PWM0_DutyCycle(U16 u16DutyPWM);
void MHal_PWM1_DutyCycle(U16 u16DutyPWM);
void MHal_PWM2_DutyCycle(U16 u16DutyPWM);
void MHal_PWM3_DutyCycle(U16 u16DutyPWM);
void MHal_PWM4_DutyCycle(U16 u16DutyPWM);
void MHal_PWM5_DutyCycle(U16 u16DutyPWM);
void MHal_PWM6_DutyCycle(U16 u16DutyPWM);
void MHal_PWM7_DutyCycle(U16 u16DutyPWM);
void MHal_PWM8_DutyCycle(U16 u16DutyPWM);
void MHal_PWM0_Div(U16 u16DivPWM);
void MHal_PWM1_Div(U16 u16DivPWM);
void MHal_PWM2_Div(U16 u16DivPWM);
void MHal_PWM3_Div(U16 u16DivPWM);
void MHal_PWM4_Div(U16 u16DivPWM);
void MHal_PWM5_Div(U16 u16DivPWM);
void MHal_PWM6_Div(U16 u16DivPWM);
void MHal_PWM7_Div(U16 u16DivPWM);
void MHal_PWM8_Div(U16 u16DivPWM);
void MHal_PWM0_Polarity(B16 b16PolPWM);
void MHal_PWM1_Polarity(B16 b16PolPWM);
void MHal_PWM2_Polarity(B16 b16PolPWM);
void MHal_PWM3_Polarity(B16 b16PolPWM);
void MHal_PWM4_Polarity(B16 b16PolPWM);
void MHal_PWM5_Polarity(B16 b16PolPWM);
void MHal_PWM6_Polarity(B16 b16PolPWM);
void MHal_PWM7_Polarity(B16 b16PolPWM);
void MHal_PWM8_Polarity(B16 b16PolPWM);
void MHal_PWM0_Vdben(B16 b16Vdben);
void MHal_PWM1_Vdben(B16 b16Vdben);
void MHal_PWM2_Vdben(B16 b16Vdben);
void MHal_PWM3_Vdben(B16 b16Vdben);
void MHal_PWM4_Vdben(B16 b16Vdben);
void MHal_PWM5_Vdben(B16 b16Vdben);
void MHal_PWM6_Vdben(B16 b16Vdben);
void MHal_PWM7_Vdben(B16 b16Vdben);
void MHal_PWM8_Vdben(B16 b16Vdben);
void MHal_PWM0_Reset_En(B16 b16VrPWM);
void MHal_PWM1_Reset_En(B16 b16VrPWM);
void MHal_PWM2_Reset_En(B16 b16VrPWM);
void MHal_PWM3_Reset_En(B16 b16VrPWM);
void MHal_PWM4_Reset_En(B16 b16VrPWM);
void MHal_PWM5_Reset_En(B16 b16VrPWM);
void MHal_PWM6_Reset_En(B16 b16VrPWM);
void MHal_PWM7_Reset_En(B16 b16VrPWM);
void MHal_PWM8_Reset_En(B16 b16VrPWM);
void MHal_PWM0_Dben(B16 b16DbenPWM);
void MHal_PWM1_Dben(B16 b16DbenPWM);
void MHal_PWM2_Dben(B16 b16DbenPWM);
void MHal_PWM3_Dben(B16 b16DbenPWM);
void MHal_PWM4_Dben(B16 b16DbenPWM);
void MHal_PWM5_Dben(B16 b16DbenPWM);
void MHal_PWM6_Dben(B16 b16DbenPWM);
void MHal_PWM7_Dben(B16 b16DbenPWM);
void MHal_PWM8_Dben(B16 b16DbenPWM);
void MHal_PWM0_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM1_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM2_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM3_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM4_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM5_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM6_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM7_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM8_Rst_Mux(B16 b16MuxPWM);
void MHal_PWM0_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM1_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM2_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM3_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM4_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM5_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM6_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM7_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM8_Rst_Cnt(U8 u8RstCntPWM);
void MHal_PWM0_Period_Ext(U16 u16PeriodExt);
void MHal_PWM1_Period_Ext(U16 u16PeriodExt);
void MHal_PWM2_Period_Ext(U16 u16PeriodExt);
void MHal_PWM3_Period_Ext(U16 u16PeriodExt);
void MHal_PWM4_Period_Ext(U16 u16PeriodExt);
void MHal_PWM5_Period_Ext(U16 u16PeriodExt);
void MHal_PWM6_Period_Ext(U16 u16PeriodExt);
void MHal_PWM7_Period_Ext(U16 u16PeriodExt);
void MHal_PWM8_Period_Ext(U16 u16PeriodExt);
void MHal_PWM9_Period_Ext(U16 u16PeriodExt);
void MHal_PWM0_Duty_Ext(U16 u16DutyExt);
void MHal_PWM1_Duty_Ext(U16 u16DutyExt);
void MHal_PWM2_Duty_Ext(U16 u16DutyExt);
void MHal_PWM3_Duty_Ext(U16 u16DutyExt);
void MHal_PWM4_Duty_Ext(U16 u16DutyExt);
void MHal_PWM5_Duty_Ext(U16 u16DutyExt);
void MHal_PWM6_Duty_Ext(U16 u16DutyExt);
void MHal_PWM7_Duty_Ext(U16 u16DutyExt);
void MHal_PWM8_Duty_Ext(U16 u16DutyExt);
void MHal_PWM0_Div_Ext(U16 u16DivExt);
void MHal_PWM1_Div_Ext(U16 u16DivExt);
void MHal_PWM2_Div_Ext(U16 u16DivExt);
void MHal_PWM3_Div_Ext(U16 u16DivExt);
void MHal_PWM4_Div_Ext(U16 u16DivExt);
void MHal_PWM5_Div_Ext(U16 u16DivExt);
void MHal_PWM6_Div_Ext(U16 u16DivExt);
void MHal_PWM7_Div_Ext(U16 u16DivExt);
void MHal_PWM8_Div_Ext(U16 u16DivExt);
void MHal_PWM0_Shift(U32 u32ShiftPWM);
void MHal_PWM1_Shift(U32 u32ShiftPWM);
void MHal_PWM2_Shift(U32 u32ShiftPWM);
void MHal_PWM3_Shift(U32 u32ShiftPWM);
void MHal_PWM4_Shift(U32 u32ShiftPWM);
void MHal_PWM5_Shift(U32 u32ShiftPWM);
void MHal_PWM6_Shift(U32 u32ShiftPWM);
void MHal_PWM7_Shift(U32 u32ShiftPWM);
void MHal_PWM8_Shift(U32 u32ShiftPWM);


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

void MAdp_PWM_Init(void);
void MAdp_PWM_Grp0_Clk_Gate_En(B16 bClkEnPWM);
void MAdp_PWM_Grp1_Clk_Gate_En(B16 bClkEnPWM);
void MAdp_PWM_Grp2_Clk_Gate_En(B16 bClkEnPWM);
void MAdp_PWM_Oen(U8 u8IndexPWM, B16 b16Oen);
void MAdp_PWM_Period(U8 u8IndexPWM, U16 u16PeriodPWM);
void MAdp_PWM_DutyCycle(U8 u8IndexPWM, U16 u16DutyPWM);
void MAdp_PWM_Div(U8 u8IndexPWM, U16 u16DivPWM);
void MAdp_PWM_Polarity(U8 u8IndexPWM, B16 b16PolPWM);
void MAdp_PWM_Vdben(U8 u8IndexPWM, B16 b16VdbenPWM);
void MAdp_PWM_Reset_En(U8 u8IndexPWM, B16 b16RstPWM);
void MAdp_PWM_Dben(U8 u8IndexPWM, B16 b16DbenPWM);
void MAdp_PWM_Rst_Mux(U8 u8IndexPWM, B16 b16MuxPWM);
void MAdp_PWM_Rst_Cnt(U8 u8IndexPWM, U8 u8RstCntPWM);
void MAdp_PWM_Period_Ext(U8 u8IndexPWM, U32 u32PeriodExt);
void MAdp_PWM_Duty_Ext(U8 u8IndexPWM, U32 u32DutyExt);
void MAdp_PWM_Div_Ext(U8 u8IndexPWM, U16 u16DivExt/*U32 u32DivExt*/); // 090817_louis
void MAdp_PWM_Shift(U8 u8IndexPWM, U32 u32ShiftPWM);


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
}

void MHal_PWM_Grp1_Clk_Gate_En(B16 bClkEnPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(bClkEnPWM == TRUE)
        MHal_PWM_REG(REG_PWM_GRP1_CLK_GATE_EN) |= BIT1;
    else
        MHal_PWM_REG(REG_PWM_GRP1_CLK_GATE_EN) &= ~BIT1;
}

void MHal_PWM_Grp2_Clk_Gate_En(B16 bClkEnPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(bClkEnPWM == TRUE)
        MHal_PWM_REG(REG_PWM_GRP2_CLK_GATE_EN) |= BIT2;
    else
        MHal_PWM_REG(REG_PWM_GRP2_CLK_GATE_EN) &= ~BIT2;
}

void MHal_PWM0_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM0_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM0_OEN) |= BIT15;
}

void MHal_PWM1_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM1_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM1_OEN) |= BIT15;
}

void MHal_PWM2_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM2_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM2_OEN) |= BIT15;
}

void MHal_PWM3_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM3_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM3_OEN) |= BIT15;
}

void MHal_PWM4_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM4_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM4_OEN) |= BIT15;
}

void MHal_PWM5_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM5_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM5_OEN) |= BIT15;
}

void MHal_PWM6_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM6_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM6_OEN) |= BIT15;
}

void MHal_PWM7_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM7_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM7_OEN) |= BIT15;
}

void MHal_PWM8_Oen(B16 b16OenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16OenPWM == TRUE)
        MHal_PWM_REG(REG_PWM8_OEN) &= ~BIT15;
    else
        MHal_PWM_REG(REG_PWM8_OEN) |= BIT15;
}

void MHal_PWM0_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_PERIOD) = u16PeriodPWM;
}

void MHal_PWM1_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM1_PERIOD) = u16PeriodPWM;
}

void MHal_PWM2_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM2_PERIOD) = u16PeriodPWM;
}

void MHal_PWM3_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM3_PERIOD) = u16PeriodPWM;
}

void MHal_PWM4_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM4_PERIOD) = u16PeriodPWM;
}

void MHal_PWM5_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM5_PERIOD) = u16PeriodPWM;
}

void MHal_PWM6_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM6_PERIOD) = u16PeriodPWM;
}

void MHal_PWM7_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM7_PERIOD) = u16PeriodPWM;
}

void MHal_PWM8_Period(U16 u16PeriodPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM8_PERIOD) = u16PeriodPWM;
}

void MHal_PWM0_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_DUTY) = u16DutyPWM;
}

void MHal_PWM1_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM1_DUTY) = u16DutyPWM;
}

void MHal_PWM2_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM2_DUTY) = u16DutyPWM;
}

void MHal_PWM3_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM3_DUTY) = u16DutyPWM;
}

void MHal_PWM4_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM4_DUTY) = u16DutyPWM;
}

void MHal_PWM5_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM5_DUTY) = u16DutyPWM;
}

void MHal_PWM6_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM6_DUTY) = u16DutyPWM;
}

void MHal_PWM7_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM7_DUTY) = u16DutyPWM;
}

void MHal_PWM8_DutyCycle(U16 u16DutyPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM8_DUTY) = u16DutyPWM;
}

void MHal_PWM0_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM0_DIV) = (MHal_PWM_REG(REG_PWM0_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM0_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM0_DIV) |= u16DivPWM;
}

void MHal_PWM1_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM1_DIV) = (MHal_PWM_REG(REG_PWM1_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM1_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM1_DIV) |= u16DivPWM;
}

void MHal_PWM2_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM2_DIV) = ((MHal_PWM_REG(REG_PWM2_DIV) & (0xff00)) | (u16DivPWM & 0xff));
//  MHal_PWM_REG(REG_PWM2_DIV) &= 0xff00;
//  MHal_PWM_REG(REG_PWM2_DIV) |= u16DivPWM;
}

void MHal_PWM3_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM3_DIV) = (MHal_PWM_REG(REG_PWM3_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM3_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM3_DIV) |= u16DivPWM;
}

void MHal_PWM4_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM4_DIV) = (MHal_PWM_REG(REG_PWM4_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM4_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM4_DIV) |= u16DivPWM;
}

void MHal_PWM5_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM5_DIV) = (MHal_PWM_REG(REG_PWM5_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM5_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM5_DIV) |= u16DivPWM;
}

void MHal_PWM6_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM6_DIV) = (MHal_PWM_REG(REG_PWM6_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM6_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM6_DIV) |= u16DivPWM;
}

void MHal_PWM7_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM7_DIV) = (MHal_PWM_REG(REG_PWM7_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM7_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM7_DIV) |= u16DivPWM;
}

void MHal_PWM8_Div(U16 u16DivPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
	MHal_PWM_REG(REG_PWM8_DIV) = (MHal_PWM_REG(REG_PWM8_DIV) & (0xff00)) | (u16DivPWM & 0xff);
//    MHal_PWM_REG(REG_PWM8_DIV) &= 0xff00;
//    MHal_PWM_REG(REG_PWM8_DIV) |= u16DivPWM;
}

void MHal_PWM0_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM0_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM0_PORARITY) &= ~BIT8;
}

void MHal_PWM1_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM1_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM1_PORARITY) &= ~BIT8;
}

void MHal_PWM2_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM2_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM2_PORARITY) &= ~BIT8;
}

void MHal_PWM3_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM3_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM3_PORARITY) &= ~BIT8;
}

void MHal_PWM4_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM4_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM4_PORARITY) &= ~BIT8;
}

void MHal_PWM5_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM5_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM5_PORARITY) &= ~BIT8;
}

void MHal_PWM6_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM6_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM6_PORARITY) &= ~BIT8;
}

void MHal_PWM7_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM7_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM7_PORARITY) &= ~BIT8;
}

void MHal_PWM8_Polarity(B16 b16PolPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16PolPWM == TRUE)
        MHal_PWM_REG(REG_PWM8_PORARITY) |= BIT8;
    else
        MHal_PWM_REG(REG_PWM8_PORARITY) &= ~BIT8;
}

void MHal_PWM0_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM0_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM0_VDBEN) &= ~BIT9;
}

void MHal_PWM1_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM1_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM1_VDBEN) &= ~BIT9;
}

void MHal_PWM2_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM2_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM2_VDBEN) &= ~BIT9;
}

void MHal_PWM3_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM3_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM3_VDBEN) &= ~BIT9;
}

void MHal_PWM4_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM4_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM4_VDBEN) &= ~BIT9;
}

void MHal_PWM5_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM5_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM5_VDBEN) &= ~BIT9;
}

void MHal_PWM6_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM6_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM6_VDBEN) &= ~BIT9;
}

void MHal_PWM7_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM7_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM7_VDBEN) &= ~BIT9;
}

void MHal_PWM8_Vdben(B16 b16Vdben)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16Vdben == TRUE)
        MHal_PWM_REG(REG_PWM8_VDBEN) |= BIT9;
    else
        MHal_PWM_REG(REG_PWM8_VDBEN) &= ~BIT9;
}

void MHal_PWM0_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM0_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM0_RESET_EN) &= ~BIT10;
}

void MHal_PWM1_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM1_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM1_RESET_EN) &= ~BIT10;
}

void MHal_PWM2_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM2_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM2_RESET_EN) &= ~BIT10;
}

void MHal_PWM3_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM3_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM3_RESET_EN) &= ~BIT10;
}

void MHal_PWM4_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM4_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM4_RESET_EN) &= ~BIT10;
}

void MHal_PWM5_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM5_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM5_RESET_EN) &= ~BIT10;
}

void MHal_PWM6_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM6_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM6_RESET_EN) &= ~BIT10;
}

void MHal_PWM7_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM7_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM7_RESET_EN) &= ~BIT10;
}

void MHal_PWM8_Reset_En(B16 b16VrPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16VrPWM == TRUE)
        MHal_PWM_REG(REG_PWM8_RESET_EN) |= BIT10;
    else
        MHal_PWM_REG(REG_PWM8_RESET_EN) &= ~BIT10;
}

void MHal_PWM0_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM0_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM0_DBEN) &= ~BIT11;
}

void MHal_PWM1_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM1_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM1_DBEN) &= ~BIT11;
}

void MHal_PWM2_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM2_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM2_DBEN) &= ~BIT11;
}

void MHal_PWM3_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM3_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM3_DBEN) &= ~BIT11;
}

void MHal_PWM4_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM4_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM4_DBEN) &= ~BIT11;
}

void MHal_PWM5_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM5_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM5_DBEN) &= ~BIT11;
}

void MHal_PWM6_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM6_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM6_DBEN) &= ~BIT11;
}

void MHal_PWM7_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM7_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM7_DBEN) &= ~BIT11;
}

void MHal_PWM8_Dben(B16 b16DbenPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16DbenPWM == TRUE)
        MHal_PWM_REG(REG_PWM8_DBEN) |= BIT11;
    else
        MHal_PWM_REG(REG_PWM8_DBEN) &= ~BIT11;
}

void MHal_PWM0_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX0) |= BIT15;
    else
        MHal_PWM_REG(REG_RST_MUX0) &= ~BIT15;
}

void MHal_PWM1_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX1) |= BIT7;
    else
        MHal_PWM_REG(REG_RST_MUX1) &= ~BIT7;
}

void MHal_PWM2_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX2) |= BIT15;
    else
        MHal_PWM_REG(REG_RST_MUX2) &= ~BIT15;
}

void MHal_PWM3_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX3) |= BIT7;
    else
        MHal_PWM_REG(REG_RST_MUX3) &= ~BIT7;
}

void MHal_PWM4_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX4) |= BIT15;
    else
        MHal_PWM_REG(REG_RST_MUX4) &= ~BIT15;
}

void MHal_PWM5_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX5) |= BIT7;
    else
        MHal_PWM_REG(REG_RST_MUX5) &= ~BIT7;
}

void MHal_PWM6_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX6) |= BIT15;
    else
        MHal_PWM_REG(REG_RST_MUX6) &= ~BIT15;
}

void MHal_PWM7_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX7) |= BIT7;
    else
        MHal_PWM_REG(REG_RST_MUX7) &= ~BIT7;
}

void MHal_PWM8_Rst_Mux(B16 b16MuxPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    if(b16MuxPWM == TRUE)
        MHal_PWM_REG(REG_RST_MUX8) |= BIT15;
    else
        MHal_PWM_REG(REG_RST_MUX8) &= ~BIT15;
}

void MHal_PWM0_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    u16RstCntPWM = (u16RstCntPWM << 8);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT0) &= 0xf0ff;
    MHal_PWM_REG(REG_HS_RST_CNT0) |= u16RstCntPWM;
}

void MHal_PWM1_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT1) &= 0xfff0;
    MHal_PWM_REG(REG_HS_RST_CNT1) |= u16RstCntPWM;
}

void MHal_PWM2_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    u16RstCntPWM = (u16RstCntPWM << 8);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT2) &= 0xf0ff;
    MHal_PWM_REG(REG_HS_RST_CNT2) |= u16RstCntPWM;
}

void MHal_PWM3_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT3) &= 0xfff0;
    MHal_PWM_REG(REG_HS_RST_CNT3) |= u16RstCntPWM;
}

void MHal_PWM4_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    u16RstCntPWM = (u16RstCntPWM << 8);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT4) &= 0xf0ff;
    MHal_PWM_REG(REG_HS_RST_CNT4) |= u16RstCntPWM;
}

void MHal_PWM5_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT5) &= 0xfff0;
    MHal_PWM_REG(REG_HS_RST_CNT5) |= u16RstCntPWM;
}

void MHal_PWM6_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    u16RstCntPWM = (u16RstCntPWM << 8);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT6) &= 0xf0ff;
    MHal_PWM_REG(REG_HS_RST_CNT6) |= u16RstCntPWM;
}

void MHal_PWM7_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT7) &= 0xfff0;
    MHal_PWM_REG(REG_HS_RST_CNT7) |= u16RstCntPWM;
}

void MHal_PWM8_Rst_Cnt(U8 u8RstCntPWM)
{
    U16 u16RstCntPWM;
    u16RstCntPWM = (U16) u8RstCntPWM;
    u16RstCntPWM = (u16RstCntPWM << 8);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_HS_RST_CNT8) &= 0xf0ff;
    MHal_PWM_REG(REG_HS_RST_CNT8) |= u16RstCntPWM;
}

void MHal_PWM0_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM0_PERIOD_EXT)  & ~0x0003) | (u16PeriodExt & 0x0003);
}

void MHal_PWM1_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 2);    
    MHal_PWM_REG(REG_PWM1_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM1_PERIOD_EXT)  & ~0x000C) | (u16PeriodExt & 0x000C);

}

void MHal_PWM2_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 4);
    MHal_PWM_REG(REG_PWM2_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM2_PERIOD_EXT)  & ~0x0030) | (u16PeriodExt & 0x0030);
}

void MHal_PWM3_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 6);
    MHal_PWM_REG(REG_PWM3_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM3_PERIOD_EXT)  & ~0x00C0) | (u16PeriodExt & 0x00C0);	
}

void MHal_PWM4_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 8);
    MHal_PWM_REG(REG_PWM4_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM4_PERIOD_EXT)  & ~0x0300) | (u16PeriodExt & 0x0300);	
}

void MHal_PWM5_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 10);
    MHal_PWM_REG(REG_PWM5_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM5_PERIOD_EXT)  & ~0x0C00) | (u16PeriodExt & 0x0C00);	
}

void MHal_PWM6_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM6_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM6_PERIOD_EXT)  & ~0x0003) | (u16PeriodExt & 0x0003);
}

void MHal_PWM7_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 2);
    MHal_PWM_REG(REG_PWM7_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM7_PERIOD_EXT)  & ~0x000C) | (u16PeriodExt & 0x000C);
}

void MHal_PWM8_Period_Ext(U16 u16PeriodExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16PeriodExt = (u16PeriodExt << 4);
    MHal_PWM_REG(REG_PWM8_PERIOD_EXT) = (MHal_PWM_REG(REG_PWM8_PERIOD_EXT)  & ~0x0030) | (u16PeriodExt & 0x0030);
}

void MHal_PWM0_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_DUTY_EXT) = (MHal_PWM_REG(REG_PWM0_DUTY_EXT)  & ~0x0003) | (u16DutyExt & 0x0003);
}

void MHal_PWM1_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 2);
    MHal_PWM_REG(REG_PWM1_DUTY_EXT) = (MHal_PWM_REG(REG_PWM1_DUTY_EXT)  & ~0x000C) | (u16DutyExt & 0x000C);
}

void MHal_PWM2_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 4);
    MHal_PWM_REG(REG_PWM2_DUTY_EXT) = (MHal_PWM_REG(REG_PWM2_DUTY_EXT)  & ~0x0030) | (u16DutyExt & 0x0030);
}

void MHal_PWM3_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 6);
    MHal_PWM_REG(REG_PWM3_DUTY_EXT) = (MHal_PWM_REG(REG_PWM3_DUTY_EXT)  & ~0x00C0) | (u16DutyExt & 0x00C0);
}

void MHal_PWM4_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 8);
    MHal_PWM_REG(REG_PWM4_DUTY_EXT) = (MHal_PWM_REG(REG_PWM4_DUTY_EXT)  & ~0x0300) | (u16DutyExt & 0x0300);
}

void MHal_PWM5_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 10);
    MHal_PWM_REG(REG_PWM5_DUTY_EXT) = (MHal_PWM_REG(REG_PWM5_DUTY_EXT)  & ~0x0C00) | (u16DutyExt & 0x0C00);
}

void MHal_PWM6_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 8);
    MHal_PWM_REG(REG_PWM6_DUTY_EXT) = (MHal_PWM_REG(REG_PWM6_DUTY_EXT)  & ~0x0300) | (u16DutyExt & 0x0300);
}

void MHal_PWM7_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 10);
    MHal_PWM_REG(REG_PWM7_DUTY_EXT) = (MHal_PWM_REG(REG_PWM7_DUTY_EXT)  & ~0x0C00) | (u16DutyExt & 0x0C00);
}

void MHal_PWM8_Duty_Ext(U16 u16DutyExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DutyExt = (u16DutyExt << 12);
    MHal_PWM_REG(REG_PWM8_DUTY_EXT) = (MHal_PWM_REG(REG_PWM8_DUTY_EXT)  & ~0x3000) | (u16DutyExt & 0x3000);
}

void MHal_PWM0_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_DIV_EXT) = (MHal_PWM_REG(REG_PWM0_DIV_EXT)  & ~0x00FF) | (u16DivExt & 0x00FF); 
}

void MHal_PWM1_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DivExt = (u16DivExt << 8);
    MHal_PWM_REG(REG_PWM1_DIV_EXT) = (MHal_PWM_REG(REG_PWM1_DIV_EXT)  & ~0xFF00) | (u16DivExt & 0xFF00); 
}

void MHal_PWM2_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM2_DIV_EXT) = (MHal_PWM_REG(REG_PWM2_DIV_EXT)  & ~0x00FF) | (u16DivExt & 0x00FF); 
}

void MHal_PWM3_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DivExt = (u16DivExt << 8);
    MHal_PWM_REG(REG_PWM3_DIV_EXT) = (MHal_PWM_REG(REG_PWM3_DIV_EXT)  & ~0xFF00) | (u16DivExt & 0xFF00); 
}

void MHal_PWM4_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM4_DIV_EXT) = (MHal_PWM_REG(REG_PWM4_DIV_EXT)  & ~0x00FF) | (u16DivExt & 0x00FF); 
}

void MHal_PWM5_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DivExt = (u16DivExt << 8);
    MHal_PWM_REG(REG_PWM5_DIV_EXT) = (MHal_PWM_REG(REG_PWM5_DIV_EXT)  & ~0xFF00) | (u16DivExt & 0xFF00); 
}

void MHal_PWM6_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM6_DIV_EXT) = (MHal_PWM_REG(REG_PWM6_DIV_EXT)  & ~0x00FF) | (u16DivExt & 0x00FF); 
}

void MHal_PWM7_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    u16DivExt = (u16DivExt << 8);
    MHal_PWM_REG(REG_PWM7_DIV_EXT) = (MHal_PWM_REG(REG_PWM7_DIV_EXT)  & ~0xFF00) | (u16DivExt & 0xFF00); 
}

void MHal_PWM8_Div_Ext(U16 u16DivExt)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM8_DIV_EXT) = (MHal_PWM_REG(REG_PWM8_DIV_EXT)  & ~0x00FF) | (u16DivExt & 0x00FF); 
}

void MHal_PWM0_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM0_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM0_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}

void MHal_PWM1_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM1_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM1_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}

void MHal_PWM2_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM2_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM2_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}

void MHal_PWM3_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM3_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM3_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}

void MHal_PWM4_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM4_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM4_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}

void MHal_PWM5_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM5_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM5_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}

void MHal_PWM6_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM6_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM6_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}

void MHal_PWM7_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM7_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM7_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}

void MHal_PWM8_Shift(U32 u32ShiftPWM)
{
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_PWM8_SHIFT_L) = (U16)(u32ShiftPWM & 0x0000ffff);
    u32ShiftPWM = (u32ShiftPWM & 0xffff0000);
    MHal_PWM_REG(REG_PWM8_SHIFT_H) = (U16)(u32ShiftPWM >> 16);
}


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
        case PAD_PWM5:
            MHal_PWM5_Oen(b16OenPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Oen(b16OenPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Oen(b16OenPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Oen(b16OenPWM);
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
        case PAD_PWM5:
            MHal_PWM5_Period(u16PeriodPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Period(u16PeriodPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Period(u16PeriodPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Period(u16PeriodPWM);
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
        case PAD_PWM5:
            MHal_PWM5_DutyCycle(u16DutyPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_DutyCycle(u16DutyPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_DutyCycle(u16DutyPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_DutyCycle(u16DutyPWM);
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
/*
void MDrv_PWM_Unit_Div(U16 u16UnitDivPWM)
{
    //MHal_PWM_Unit_Div(u16UnitDivPWM);
    MHal_PWM_REG(REG_PWM_BANK) = 0x01;
    MHal_PWM_REG(REG_UNIT_DIV) &= 0xff00;
    MHal_PWM_REG(REG_UNIT_DIV) |= u16UnitDivPWM;
}
*/
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
        case PAD_PWM5:
            MHal_PWM5_Div(u16DivPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Div(u16DivPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Div(u16DivPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Div(u16DivPWM);
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
        case PAD_PWM5:
            MHal_PWM5_Polarity(b16PolPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Polarity(b16PolPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Polarity(b16PolPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Polarity(b16PolPWM);
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
        case PAD_PWM5:
            MHal_PWM5_Vdben(b16VdbPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Vdben(b16VdbPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Vdben(b16VdbPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Vdben(b16VdbPWM);
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
        case PAD_PWM5:
            MHal_PWM5_Reset_En(b16VrPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Reset_En(b16VrPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Reset_En(b16VrPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Reset_En(b16VrPWM);
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
        case PAD_PWM5:
            MHal_PWM5_Dben(b16DenPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Dben(b16DenPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Dben(b16DenPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Dben(b16DenPWM);
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
        case PAD_PWM5:
            MHal_PWM5_Rst_Mux(b16MuxPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Rst_Mux(b16MuxPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Rst_Mux(b16MuxPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Rst_Mux(b16MuxPWM);
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
        case PAD_PWM5:
            MHal_PWM5_Rst_Cnt(u8RstCntPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Rst_Cnt(u8RstCntPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Rst_Cnt(u8RstCntPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Rst_Cnt(u8RstCntPWM);
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
        case PAD_PWM5:
            MHal_PWM5_Period_Ext(u16PeriodExt);
            break;
        case PAD_PWM6:
            MHal_PWM6_Period_Ext(u16PeriodExt);
            break;
        case PAD_PWM7:
            MHal_PWM7_Period_Ext(u16PeriodExt);
            break;
        case PAD_PWM8:
            MHal_PWM8_Period_Ext(u16PeriodExt);
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
        case PAD_PWM5:
            MHal_PWM5_Duty_Ext(u16DutyExt);
            break;
        case PAD_PWM6:
            MHal_PWM6_Duty_Ext(u16DutyExt);
            break;
        case PAD_PWM7:
            MHal_PWM7_Duty_Ext(u16DutyExt);
            break;
        case PAD_PWM8:
            MHal_PWM8_Duty_Ext(u16DutyExt);
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
        case PAD_PWM5:
            MHal_PWM5_Div_Ext(u16DivExt);
            break;
        case PAD_PWM6:
            MHal_PWM6_Div_Ext(u16DivExt);
            break;
        case PAD_PWM7:
            MHal_PWM7_Div_Ext(u16DivExt);
            break;
        case PAD_PWM8:
            MHal_PWM8_Div_Ext(u16DivExt);
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
        case PAD_PWM5:
            MHal_PWM5_Shift(u32ShiftPWM);
            break;
        case PAD_PWM6:
            MHal_PWM6_Shift(u32ShiftPWM);
            break;
        case PAD_PWM7:
            MHal_PWM7_Shift(u32ShiftPWM);
            break;
        case PAD_PWM8:
            MHal_PWM8_Shift(u32ShiftPWM);
            break;
    }
}




//-------------------------------------------------------------------------------------------------
/// PWM initialization
/// @return None
/// @note   Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Init(void)
{

    MDrv_PWM_Init();

}

//-------------------------------------------------------------------------------------------------
/// gating clk for PWM0~2 pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bClkEnPWM               \b IN:  enable or disable
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Grp0_Clk_Gate_En(B16 bClkEnPWM)
{
    //ioctl(s32FdPWM, MDRV_PWM_GRP0_CLK_GATE_EN, &bClkEnPWM);
    MDrv_PWM_Grp0_Clk_Gate_En(bClkEnPWM);

}

//-------------------------------------------------------------------------------------------------
/// gating clk for PWM3~5 pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bClkEnPWM               \b IN:  enable or disable
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Grp1_Clk_Gate_En(B16 bClkEnPWM)
{
    //ioctl(s32FdPWM, MDRV_PWM_GRP1_CLK_GATE_EN, &bClkEnPWM);
    MDrv_PWM_Grp1_Clk_Gate_En(bClkEnPWM);

}

//-------------------------------------------------------------------------------------------------
/// gating clk for PWM6~8 pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bClkEnPWM               \b IN:  enable or disable
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Grp2_Clk_Gate_En(B16 bClkEnPWM)
{
    //ioctl(s32FdPWM, MDRV_PWM_GRP2_CLK_GATE_EN, &bClkEnPWM);
    MDrv_PWM_Grp2_Clk_Gate_En(bClkEnPWM);

}

//-------------------------------------------------------------------------------------------------
/// Output enable for PWM pads
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bOenPWM                 \b IN:  enable or disable
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Oen(U8 u8IndexPWM, B16 b16Oen)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16Oen= b16Oen;
    //ioctl(s32FdPWM, MDRV_PWM_OEN, &PWM_Set);
    MDrv_PWM_Oen(PWM_Set.u8Index, PWM_Set.b16Oen);

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
    //ioctl(s32FdPWM, MDRV_PWM_PERIOD, &PWM_Set);
    MDrv_PWM_Period(PWM_Set.u8Index , PWM_Set.u16Period );

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
    //ioctl(s32FdPWM, MDRV_PWM_DUTY, &PWM_Set);
    MDrv_PWM_DutyCycle(PWM_Set.u8Index, PWM_Set.u16Duty);

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
    //ioctl(s32FdPWM, MDRV_PWM_DIV, &PWM_Set);
    MDrv_PWM_Div(PWM_Set.u8Index, PWM_Set.u16Div);

}

//-------------------------------------------------------------------------------------------------
/// Set polarity for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16PolPWM                 \b IN:  polarity
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Polarity(U8 u8IndexPWM, B16 b16PolPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16Polarity = b16PolPWM;
    //ioctl(s32FdPWM, MDRV_PWM_POLARITY, &PWM_Set);
    MDrv_PWM_Polarity(PWM_Set.u8Index, PWM_Set.b16Polarity);

}

//-------------------------------------------------------------------------------------------------
/// Vsync double buffer enable for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bVdbenPWM               \b IN:  enable or disbale
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Vdben(U8 u8IndexPWM, B16 b16VdbenPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16Vdben = b16VdbenPWM;
    //ioctl(s32FdPWM, MDRV_PWM_VDBEN, &PWM_Set);
    MDrv_PWM_Vdben(PWM_Set.u8Index, PWM_Set.b16Vdben);

}

//-------------------------------------------------------------------------------------------------
/// Vsync reset for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bRstPWM                 \b IN:  enable or disbale
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Reset_En(U8 u8IndexPWM, B16 b16RstPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16ResetEn = b16RstPWM;
    //ioctl(s32FdPWM, MDRV_PWM_RESET_EN, &PWM_Set);
    MDrv_PWM_Reset_En(PWM_Set.u8Index, PWM_Set.b16ResetEn);

}

//-------------------------------------------------------------------------------------------------
/// Double buffer enable for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  bDbenPWM                \b IN:  enable or disbale
/// @return None
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Dben(U8 u8IndexPWM, B16 b16DbenPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16Dben = b16DbenPWM;
    //ioctl(s32FdPWM, MDRV_PWM_DBEN, &PWM_Set);
    MDrv_PWM_Dben(PWM_Set.u8Index, PWM_Set.b16Dben);

}


//-------------------------------------------------------------------------------------------------
/// reset mux for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  b16MuxPWM                 \b IN:  enable or disbale
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Rst_Mux(U8 u8IndexPWM, B16 b16MuxPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.b16RstMux = b16MuxPWM;
    //ioctl(s32FdPWM, MDRV_PWM_RST_MUX, &PWM_Set);
    MDrv_PWM_Rst_Mux(PWM_Set.u8Index, PWM_Set.b16RstMux);

}

//-------------------------------------------------------------------------------------------------
/// Hsync reset counter for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u8RstCntPWM             \b IN:  Hsync reset counter
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Rst_Cnt(U8 u8IndexPWM, U8 u8RstCntPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u8RstCnt = u8RstCntPWM;
    //ioctl(s32FdPWM, MDRV_PWM_HS_RST_CNT, &PWM_Set);
    MDrv_PWM_Rst_Cnt(PWM_Set.u8Index, PWM_Set.u8RstCnt);

}

//-------------------------------------------------------------------------------------------------
/// set period (including extra period) for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u32PeriodExt            \b IN:  extra period
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Period_Ext(U8 u8IndexPWM, U32 u32PeriodExt)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u16Period = (U16)(u32PeriodExt & 0x0000ffff);
    u32PeriodExt = ((u32PeriodExt & 0xffff0000) >> 16);
    PWM_Set.u16PeriodExt = (U16) u32PeriodExt;

    MDrv_PWM_Period(PWM_Set.u8Index, PWM_Set.u16Period );
   MDrv_PWM_Period_Ext(PWM_Set.u8Index, PWM_Set.u16PeriodExt);

}

//-------------------------------------------------------------------------------------------------
/// set duty (including extra duty) for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u32DutyExt              \b IN:  duty cycle
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Duty_Ext(U8 u8IndexPWM, U32 u32DutyExt)
{

    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u16Duty = (U16)(u32DutyExt & 0x0000ffff);
    u32DutyExt = ((u32DutyExt & 0xffff0000) >> 16);
    PWM_Set.u16DutyExt = (U16) u32DutyExt;
    //ioctl(s32FdPWM, MDRV_PWM_DUTY, &PWM_Set);
    MDrv_PWM_DutyCycle(PWM_Set.u8Index, PWM_Set.u16Duty);

    MDrv_PWM_Duty_Ext(PWM_Set.u8Index, PWM_Set.u16DutyExt);
}

//-------------------------------------------------------------------------------------------------
/// set divider (including extra divider) for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u32DivExt               \b IN:  extra divider
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Div_Ext(U8 u8IndexPWM, U16 u16DivExt/*U32 u32DivExt*/) // 090817_louis
{

    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u16Div = u16DivExt & 0x00ff;
    u16DivExt = ((u16DivExt & 0xff00) >> 8);
    PWM_Set.u16DivExt = u16DivExt;
    MDrv_PWM_Div(PWM_Set.u8Index, PWM_Set.u16Div);
    MDrv_PWM_Div_Ext(PWM_Set.u8Index, PWM_Set.u16DivExt);
}


//-------------------------------------------------------------------------------------------------
/// set rising point shift counter for selected PWM pad
/// @param  u8IndexPWM              \b IN:  pad index
/// @param  u32ShiftPWM             \b IN:  rising point shift counter
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MAdp_PWM_Shift(U8 u8IndexPWM, U32 u32ShiftPWM)
{
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u32Shift = u32ShiftPWM;
    //ioctl(s32FdPWM, MDRV_PWM_SHIFT, &PWM_Set);
    MDrv_PWM_Shift(PWM_Set.u8Index, PWM_Set.u32Shift);

}

#if 0
void MAdp_PWM_test()
{
/*
    PWM_Set.u8Index = u8IndexPWM;
    PWM_Set.u32Shift = u32ShiftPWM;
    //ioctl(s32FdPWM, MDRV_PWM_SHIFT, &PWM_Set);
    MDrv_PWM_Shift(PWM_Set.u8Index, PWM_Set.u32Shift);
*/

    MAdp_PWM_Init();

//MAdp_PWM_Unit_Div(unit_div);

//printf("Please enter the Period of PWM[%d] (integer number)...",index);
//scanf("%d",&period);
MAdp_PWM_Oen(1, 1);
MAdp_PWM_Period_Ext(1, 0x02ffff);
MAdp_PWM_Duty_Ext(1, 0x1ffff);
MAdp_PWM_Oen(0, 1);
MAdp_PWM_Period_Ext(0, 0x01ffff);
MAdp_PWM_Duty_Ext(0, 0xff);
MAdp_PWM_Oen(2, 1);
MAdp_PWM_Period_Ext(2, 0x00ffff);
MAdp_PWM_Duty_Ext(2, 0x2);



}
#endif
DTV_STATUS_T	_PWM_SetVBRPortA (UINT16 data)
{
	DTV_STATUS_T retVal=OK;
	if (data >= 0xff) data = 0xff;
	data = 0x0f * data / 0xff;
	MAdp_PWM_Duty_Ext (PWM_PORT_VBR_A, data);
	return OK;
}
extern U8 Splash_GetHWoption(HW_OPT_T option_mask);

DTV_STATUS_T	_PWM_SetVBRPortB (UINT16 data,UINT32 WidthPortB)
{
	static UINT32	_gPrevVbrB = 0x00;
	DTV_STATUS_T retVal=OK;
        UINT32 applyingData;

	//balup_pwm
	if(	(gToolOptionDB.nToolOption3.flags.eBackLight != BL_CCFL) &&
		(gToolOptionDB.nToolOption3.flags.eBackLight != BL_CCFL_VCOM))
	{
		if (data >= 0xfc) data = 0xfc;
	}
	else
	{
		if (data >= 0xff) data = 0xff;
	}
	if (    (Splash_GetHWoption(PANEL_RES_OPT_SEL) == PANEL_RES_FULL_HD) &&
	        (gToolOptionDB.nToolOption3.flags.eBackLight == BL_EDGE_LED) &&
	        ((gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_22)  || (gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_26))
		)
	{
		applyingData = ((UINT32)12000000/WidthPortB -1) * data / 0xff;	//data = 0xbb * data / 0xff;
	}
	else
	{
		applyingData = ((UINT32)PWM_VBR_B_INIT_VALUE/WidthPortB -1) * data / 0xff;	//data = 0xbb * data / 0xff;
	}
	if (_gPrevVbrB != applyingData)
	{
#ifdef PWM2_OUT
		MAdp_PWM_Duty_Ext (PWM_PORT_VBR_B2, applyingData);
#else
		MAdp_PWM_Duty_Ext (PWM_PORT_VBR_B, applyingData);
#endif
		_gPrevVbrB = data;
	}
	
	return retVal;
}

void DDI_PWM_Init(UINT32 WidthPortB )
{
	UINT32 freqValuePortA, freqValuePortB;
	UINT32 pulseWidthPortA;
	UINT32 pulseWidthPortB;	

	MAdp_PWM_Init();

	freqValuePortA = PWM_VBR_A_INIT_FREQUENCY_VALUE;
	pulseWidthPortA = PWM_VBR_A_INIT_VALUE;
	if  (   (Splash_GetHWoption(PANEL_RES_OPT_SEL) == PANEL_RES_FULL_HD) &&
	        (gToolOptionDB.nToolOption3.flags.eBackLight == BL_EDGE_LED) &&
	        ((gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_22)  || (gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_26))
		)
	{
	    freqValuePortB = 0;
	    pulseWidthPortB = 12000000 / WidthPortB;
	}
	else
	{
	    freqValuePortB = PWM_VBR_B_INIT_FREQUENCY_VALUE;
	    pulseWidthPortB = PWM_VBR_B_INIT_VALUE / WidthPortB;
	}
	// Port A,B pwm frequency setting
	MAdp_PWM_Div_Ext (PWM_PORT_VBR_A, freqValuePortA);
#ifdef PWM2_OUT	
	MAdp_PWM_Div_Ext (PWM_PORT_VBR_B2, freqValuePortB);
#else
	MAdp_PWM_Div_Ext (PWM_PORT_VBR_B, PWM_VBR_B_INIT_FREQUENCY_VALUE);
#endif

	// pwm set 3.3v 레벨 설정
	MAdp_PWM_Period_Ext(PWM_PORT_VBR_A, pulseWidthPortA);
#ifdef PWM2_OUT
	MAdp_PWM_Period_Ext(PWM_PORT_VBR_B2, pulseWidthPortB);
#else
	MAdp_PWM_Period_Ext(PWM_PORT_VBR_B, pulseWidthPortB);
#endif

	_PWM_SetVBRPortA(0);
	_PWM_SetVBRPortB(0,WidthPortB);

	// pwm port enable
	MAdp_PWM_Oen (PWM_PORT_VBR_A, TRUE);
#ifdef PWM2_OUT
	if  (   (Splash_GetHWoption(PANEL_RES_OPT_SEL) == PANEL_RES_FULL_HD) &&
	        (gToolOptionDB.nToolOption3.flags.eBackLight == BL_EDGE_LED) &&
	        ((gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_22)  || (gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_26))
		)
	{
	    MAdp_PWM_Vdben(PWM_PORT_VBR_B2, TRUE);
	    MAdp_PWM_Dben (PWM_PORT_VBR_B2, FALSE);
	}
	else
	{
	    MAdp_PWM_Dben (PWM_PORT_VBR_B2, TRUE);
	    MAdp_PWM_Vdben(PWM_PORT_VBR_B2, FALSE);
	}
	MAdp_PWM_Oen (PWM_PORT_VBR_B2, TRUE);
#else
	if  (   (Splash_GetHWoption(PANEL_RES_OPT_SEL) == PANEL_RES_FULL_HD) &&
	        (gToolOptionDB.nToolOption3.flags.eBackLight == BL_EDGE_LED) &&
	        ((gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_22)  || (gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_26))
		)
	{
	    MAdp_PWM_Vdben(PWM_PORT_VBR_B, TRUE);
	    MAdp_PWM_Dben (PWM_PORT_VBR_B, FALSE);
	}
	else
	{
	    MAdp_PWM_Dben (PWM_PORT_VBR_B, TRUE);
	    MAdp_PWM_Vdben(PWM_PORT_VBR_B, FALSE);
	}
	MAdp_PWM_Oen (PWM_PORT_VBR_B, TRUE);
#endif

}

void DDI_PQ_PWMInitContol(UINT32 WidthPortB)
{

	//balup_pwm

	_PWM_SetVBRPortA(PWM_VBR_A_START_VALUE);
	  if(	(gToolOptionDB.nToolOption3.flags.eBackLight != BL_CCFL) &&
			(gToolOptionDB.nToolOption3.flags.eBackLight != BL_CCFL_VCOM))
	  {
		_PWM_SetVBRPortB(PWM_VBR_B_START_VALUE_EDGE, WidthPortB);
	  }
	  else
	  {
		_PWM_SetVBRPortB(PWM_VBR_B_START_VALUE, WidthPortB);
	  }

}



