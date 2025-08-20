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

#ifndef _MSKEYPAD_H_
#define _MSKEYPAD_H_

#include "board.h"
#include "datatype.h"

#ifdef  _MSKEYPAD_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif


#define KEYPAD_TYPE_SEL                 KEYPAD_TYPE_CUSTMOER//KEYPAD_TYPE_ORIG    // KEYPAD_TYPE_DEMO
#define KEYPAD_USE_ISR 0

#if(KEYPAD_TYPE_SEL == KEYPAD_TYPE_CUSTMOER)   //20080711 for LG local key want LK_POWER, LK_CH+, LK_CH-, LK_INPUT
// <080902 Leehc> Local Key ADC value Adjust
#define LGE_LK_POWER_UB 		0x06 //0x05    //CH1
#define LGE_LK_POWER_LB 		0x01 //0x03
#define LGE_LK_INPUT_UB 		0x14 //0x0E//0x18    //CH1
#define LGE_LK_INPUT_LB 		0x07 //0x0C//0x16
#define LGE_LK_CH_PLUS_UB   	0x34 //0x2B//0x39    //CH2
#define LGE_LK_CH_PLUS_LB   	0x25 //0x29//0x37
#define LGE_LK_CH_MINUS_UB   	0x24 //0x19//0x2C//0x2C    //CH2
#define LGE_LK_CH_MINUS_LB   	0x15 //0x17//0x2A//0x2B

#define LK_POWER_UB     LGE_LK_POWER_UB
#define LK_POWER_LB     LGE_LK_POWER_LB
#define LK_INPUT_UB     LGE_LK_INPUT_UB
#define LK_INPUT_LB     LGE_LK_INPUT_LB
#define LK_CH_PLUS_UB   LGE_LK_CH_PLUS_UB
#define LK_CH_PLUS_LB   LGE_LK_CH_PLUS_LB
#define LK_CH_MINUS_UB  LGE_LK_CH_MINUS_UB
#define LK_CH_MINUS_LB  LGE_LK_CH_MINUS_LB

#define LGE_KEYPAD_CH1_UB 0x3F
#define LGE_KEYPAD_CH1_LB 0x3A
#define LGE_KEYPAD_CH2_UB 0x3F
#define LGE_KEYPAD_CH2_LB 0x3A
#define LGE_KEYPAD_CH3_UB 0x3F
#define LGE_KEYPAD_CH3_LB 0x00
#define LGE_KEYPAD_CH4_UB 0x3F
#define LGE_KEYPAD_CH4_LB 0x00

#define KEYPAD_CH1_UB   LGE_KEYPAD_CH1_UB
#define KEYPAD_CH1_LB   LGE_KEYPAD_CH1_LB
#define KEYPAD_CH2_UB   LGE_KEYPAD_CH2_UB
#define KEYPAD_CH2_LB   LGE_KEYPAD_CH2_LB
#define KEYPAD_CH3_UB   LGE_KEYPAD_CH3_UB
#define KEYPAD_CH3_LB   LGE_KEYPAD_CH3_LB
#define KEYPAD_CH4_UB   LGE_KEYPAD_CH4_UB
#define KEYPAD_CH4_LB   LGE_KEYPAD_CH4_LB


#elif(KEYPAD_TYPE_SEL == KEYPAD_TYPE_ORIG)
#define MSTAR_LK_POWER_UB 0x09    //CH2
#define MSTAR_LK_POWER_LB 0x07
#define MSTAR_LK_INPUT_UB 0x11    //CH2
#define MSTAR_LK_INPUT_LB 0x0F
#define MSTAR_LK_CH_PLUS_UB   0x19    //CH2
#define MSTAR_LK_CH_PLUS_LB   0x17
#define MSTAR_LK_CH_MINUS_UB   0x19    //CH1
#define MSTAR_LK_CH_MINUS_LB   0x17

#define LK_POWER_UB     MSTAR_LK_POWER_UB
#define LK_POWER_LB     MSTAR_LK_POWER_LB
#define LK_INPUT_UB     MSTAR_LK_INPUT_UB
#define LK_INPUT_LB     MSTAR_LK_INPUT_LB
#define LK_CH_PLUS_UB   MSTAR_LK_CH_PLUS_UB
#define LK_CH_PLUS_LB   MSTAR_LK_CH_PLUS_LB
#define LK_CH_MINUS_UB  MSTAR_LK_CH_MINUS_UB
#define LK_CH_MINUS_LB  MSTAR_LK_CH_MINUS_LB

#define MSTAR_KEYPAD_CH1_UB 0x3F
#define MSTAR_KEYPAD_CH1_LB 0x21
#define MSTAR_KEYPAD_CH2_UB 0x3F
#define MSTAR_KEYPAD_CH2_LB 0x21
#define MSTAR_KEYPAD_CH3_UB 0x3F
#define MSTAR_KEYPAD_CH3_LB 0x00
#define MSTAR_KEYPAD_CH4_UB 0x3F
#define MSTAR_KEYPAD_CH4_LB 0x00


#define KEYPAD_CH1_UB   MSTAR_KEYPAD_CH1_UB
#define KEYPAD_CH1_LB   MSTAR_KEYPAD_CH1_LB
#define KEYPAD_CH2_UB   MSTAR_KEYPAD_CH2_UB
#define KEYPAD_CH2_LB   MSTAR_KEYPAD_CH2_LB
#define KEYPAD_CH3_UB   MSTAR_KEYPAD_CH3_UB
#define KEYPAD_CH3_LB   MSTAR_KEYPAD_CH3_LB
#define KEYPAD_CH4_UB   MSTAR_KEYPAD_CH4_UB
#define KEYPAD_CH4_LB   MSTAR_KEYPAD_CH4_LB

#endif

//------MST Keypad definition---------------------------------------------------
#define POWER_KEY_PAD_BY_INTERRUPT 0
#define ADC_KEY_CHANNEL_NUM             2
#define ADC_KEY_LAST_CHANNEL            ADC_KEY_CHANNEL_NUM - 1

#define KEYPAD_KEY_VALIDATION           3
#define KEYPAD_REPEAT_KEY_CHECK         KEYPAD_KEY_VALIDATION + 2
#define KEYPAD_REPEAT_KEY_CHECK_1       KEYPAD_KEY_VALIDATION + 3
#define KEYPAD_STABLE_NUM               10
#define KEYPAD_STABLE_NUM_MIN           9
#define KEYPAD_REPEAT_PERIOD            2 // 6
#define KEYPAD_REPEAT_PERIOD_1          KEYPAD_REPEAT_PERIOD/2

#define ADC_KEY_LEVEL                   4
#define ADC_KEY_L0                      0x06
#define ADC_KEY_L1                      0x14
#define ADC_KEY_L2                      0x21
#define ADC_KEY_L3                      0x2C

#if (KEYPAD_TYPE_SEL == KEYPAD_TYPE_ORIG)   // MStar normal keypad
#define ADC_KEY_1_L0_FLAG               IRKEY_UP
#define ADC_KEY_1_L1_FLAG               IRKEY_MENU
#define ADC_KEY_1_L2_FLAG               IRKEY_LEFT
#define ADC_KEY_1_L3_FLAG               IRKEY_MUTE

#define ADC_KEY_2_L0_FLAG               IRKEY_POWER
#define ADC_KEY_2_L1_FLAG               IRKEY_INPUT_SOURCE
#define ADC_KEY_2_L2_FLAG               IRKEY_RIGHT
#define ADC_KEY_2_L3_FLAG               IRKEY_DOWN
#elif (KEYPAD_TYPE_SEL == KEYPAD_TYPE_DEMO) // MStar demo set keypad
#define ADC_KEY_1_L0_FLAG               IRKEY_MUTE
#define ADC_KEY_1_L1_FLAG               IRKEY_VOLUME_MINUS
#define ADC_KEY_1_L2_FLAG               IRKEY_VOLUME_PLUS
#define ADC_KEY_1_L3_FLAG               IRKEY_DOWN

#define ADC_KEY_2_L0_FLAG               IRKEY_POWER
#define ADC_KEY_2_L1_FLAG               IRKEY_UP
#define ADC_KEY_2_L2_FLAG               IRKEY_MENU
#define ADC_KEY_2_L3_FLAG               IRKEY_INPUT_SOURCE
#endif


//#define KeypadRepeatTimerCount 1 //100ms based

#define KEYPAD_LV_STABLE_COUNT 10 //used in drvisr
#define KEYPAD_LV_FIRSTTIME_ELASPED_COUNT (KEYPAD_LV_STABLE_COUNT+38)//used in drvisr



enum KEYPAD_ADC_CHANNEL
{
    KEYPAD_ADC_CHANNEL_1 = 0,
    KEYPAD_ADC_CHANNEL_2,
    KEYPAD_ADC_CHANNEL_3,
    KEYPAD_ADC_CHANNEL_4,
    KEYPAD_ADC_CHANNEL_5,
};

#if 1//(KEYPAD_TYPE_SEL == KEYPAD_TYPE_NONE)

//#define msKeypad_Init()                 _FUNC_NOT_USED()
INTERFACE void  msKeypad_Init(void);
#define msKeypad_GetKey(pkey, pflag)    MSRET_ERROR
#define MDrv_Power_CheckPowerOnKeyPad() FALSE
#define msKeypad_ClearBuffer()          _FUNC_NOT_USED()

#else

INTERFACE void  msKeypad_Init(void);
INTERFACE MSRET msKeypad_GetKey(U8 *pkey, U8 *pflag);

/*
INTERFACE U8    msKeypad_Get_ADC_Channel(U8 Channel, U8 *pvalue);
INTERFACE BOOLEAN MDrv_Power_CheckPowerOnKeyPad(void);
INTERFACE void msKeypad_ClearBuffer();
*/
INTERFACE U8 KEYPAD_LV_CHANNEL[ADC_KEY_CHANNEL_NUM];
INTERFACE U8 KEYPAD_PREVIOUS_LV_CHANNEL[ADC_KEY_CHANNEL_NUM];
INTERFACE U8 KEYPAD_LV_COUNT_CHANNEL[ADC_KEY_CHANNEL_NUM];
INTERFACE unsigned char tADCKeyLevel[];
INTERFACE void msKeypad_ClearBuffer();

#endif

#undef INTERFACE

#endif

