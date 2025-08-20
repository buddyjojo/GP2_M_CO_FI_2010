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

#ifndef IR_FORMAT_H
#define IR_FORMAT_H

//******************************************************************************
// Customer IR Specification parameter define (Please modify them by IR SPEC)
//******************************************************************************
#define IR_MODE_SEL             IR_TYPE_FULLDECODE_MODE
#define IR_LG_MAP               1

// IR Header code define

#define IR_HEADER_CODE0         0x04    // Custom 0     //0x04
#define IR_HEADER_CODE1         0xFB    // Custom 1     //0xFB

// IR Timing define
#define IR_HEADER_CODE_TIME     9000    // us
#define IR_OFF_CODE_TIME        4500    // us
#define IR_OFF_CODE_RP_TIME     2500    // us
#define IR_LOGI_01H_TIME        560     // us
#define IR_LOGI_0_TIME          1120    // us
#define IR_LOGI_1_TIME          2240    // us
#define IR_TIMEOUT_CYC          140000  // us

#define IR_HEADER_CODE_TIME_UB  20
#define IR_HEADER_CODE_TIME_LB  -20
#define IR_OFF_CODE_TIME_UB  20
#define IR_OFF_CODE_TIME_LB  -20
#define IR_OFF_CODE_RP_TIME_UB  20
#define IR_OFF_CODE_RP_TIME_LB  -20
#define IR_LOGI_01H_TIME_UB  35
#define IR_LOGI_01H_TIME_LB  -30
#define IR_LOGI_0_TIME_UB  20
#define IR_LOGI_0_TIME_LB  -20
#define IR_LOGI_1_TIME_UB  20
#define IR_LOGI_1_TIME_LB  -20

// IR Format define
#define IRKEY_DUMY              0xFF
#define IRDA_KEY_MAPPING_POWER  IRKEY_POWER
#define IR_RAW_DATA_NUM         4

//#define USE_Q_MENU_MENU         1//0
#define IR_LEADER_CODE_CHECKING_OPTION 0xBF

typedef enum _IrCommandType
{
    #if (ENABLE_HDMI_CEC)
    IRKEY_PLAY_KEY = 0xB0,
    IRKEY_STOP_KEY = 0xB1,
    IRKEY_SKIPM_KEY = 0xB2,
    IRKEY_SKIPP_KEY = 0xB3,
    IRKEY_PAUSE_KEY = 0xBA,
        //IRKEY_REC = 0xBD, // ??
    IRKEY_FF = 0x8E, // ????
    IRKEY_REW = 0x8F, // ???
    #endif
    IRKEY_CHANNEL_FAV_LIST = 0x1E, //IR_KEY_FAV
    IRKEY_CHANNEL_RETURN = 0x1A, //IR_KEY_FLASHBK
    IRKEY_CHANNEL_PLUS = 0x00, //IR_KEY_CH_UP
    IRKEY_CHANNEL_MINUS = 0x01, //IR_KEY_CH_DOWN

    IRKEY_AUDIO = 0x52, //IR_KEY_SOUND
    IRKEY_VOLUME_PLUS = 0x02, //IR_KEY_VOL_UP
    IRKEY_VOLUME_MINUS = 0x03, //IR_KEY_VOL_DOWN

    IRKEY_UP = 0x40, //IR_KEY_UP_ARROW
    IRKEY_POWER = 0x08, //IR_KEY_POWER
    IRKEY_EXIT = 0x5B, //IR_KEY_EXIT
    IRKEY_MENU = 0x43, //IR_KEY_MENU
    IRKEY_DOWN = 0x41, //IR_KEY_DOWN_ARROW
    IRKEY_LEFT = 0x07, //IR_KEY_LEFT_ARROW
    IRKEY_SELECT = 0x44, //IR_KEY_ENTER
    IRKEY_RIGHT = 0x06, //IR_KEY_RIGHT_ARROW
    IRKEY_BACK = 0x28,  //IR_KEY_BACK       //2007-6-12[hks]: Add BACK KEY Code

    IRKEY_NUM_0 = 0x10, //IR_KEY_0
    IRKEY_NUM_1 = 0x11, //IR_KEY_1
    IRKEY_NUM_2 = 0x12, //IR_KEY_2
    IRKEY_NUM_3 = 0x13, //IR_KEY_3
    IRKEY_NUM_4 = 0x14, //IR_KEY_4
    IRKEY_NUM_5 = 0x15, //IR_KEY_5
    IRKEY_NUM_6 = 0x16, //IR_KEY_6
    IRKEY_NUM_7 = 0x17, //IR_KEY_7
    IRKEY_NUM_8 = 0x18, //IR_KEY_8
    IRKEY_NUM_9 = 0x19, //IR_KEY_9

    IRKEY_MUTE = 0x09, //IR_KEY_MUTE

    IRKEY_INFO = 0xAA, //IR_KEY_INFO
    IRKEY_MTS = 0x0A, //IR_KEY_SAP
    IRKEY_CC = 0x47, //IR_KEY_CC
    IRKEY_SUBTITLE = 0x39, //IR_KEY_CC
    IRKEY_INPUT_SOURCE = 0x0B, //IR_KEY_INPUT
    IRKEY_PICTURE = 0x4D, //IR_KEY_PICTURE
    IRKEY_ZOOM = 0x79, //IR_KEY_RATIO
    IRKEY_DASH = 0x4C, //IR_KEY_DASH
    IRKEY_SLEEP = 0x0E, //IR_KEY_TIMER

    //LG Discrete IR
    IRKEY_ADJUST = 0xCB, //IR_KEY_ADJST
    IRKEY_TV_INPUT = 0x0F,
    IRKEY_YELLOW = 0x63,
    IRKEY_BLUE = 0x61,
    IRKEY_TIME = 0x26,      // 070620_SK_1 Arrange GPIO files by board option
    //In LG's remote, there's no MIX key
    IRKEY_MIX = 0x24,
    IRKEY_TTX_MODE = 0x22,
    IRKEY_HOLD = 0x65,
    IRKEY_SIZE = 0x64,
    IRKEY_TTX = 0x20,
    IRKEY_RED2 = 0x72,
    IRKEY_GREEN2 = 0x71,
    IRKEY_UPDATE = 0x62,

    IRKEY_FRONTAV = 0x51,		// <20071024 sangeh>
    IRKEY_CHANNEL_LIST = 0x53,
    IRKEY_EPG = 0xAB,
    IRKEY_TV_RADIO = 0xF0,
    IRKEY_REVEAL = 0x2A,
    IRKEY_STOP = 0xBD,
    IRKEY_IN_STOP = 0xFA,
    IRKEY_IN_START = 0xFB,
    IRKEY_P_CHECK = 0xFC,
    IRKEY_S_CHECK = 0xFD,
    IRKEY_POWERONLY = 0xFE,
    IRKEY_EZ_ADJUST = 0xFF,
    IRKEY_SIMPLE_LINK = 0x7E,
    IRKEY_BRIGHTNESS_PLUS = 0xE0,
    IRKEY_BRIGHTNESS_MINUS = 0xE1,
    DSC_IRKEY_PWRON = 0xC4,
    DSC_IRKEY_PWROFF = 0xC5,
    DSC_IRKEY_ARC4X3 = 0x76,
    DSC_IRKEY_ARC16X9 = 0x77,
    DSC_IRKEY_ARCZOOM = 0xAF,
    DSC_IRKEY_TV = 0xD6,
    DSC_IRKEY_VIDEO1 = 0x5A,
    DSC_IRKEY_VIDEO2 = 0xD0,
    DSC_IRKEY_VIDEO3 = 0xD1,
    DSC_IRKEY_COMP1 = 0xBF,
    DSC_IRKEY_COMP2 = 0xD4,
    DSC_IRKEY_RGBPC = 0xD5,
    DSC_IRKEY_RGBDTV = 0xD7,
    DSC_IRKEY_RGBDVI = 0xC6,
    DSC_IRKEY_HDMI1 = 0xCE,
    DSC_IRKEY_HDMI2 = 0xCC,
    DSC_IRKEY_HDMI3 = 0xE9,		// <20071024 sangeh>
    DSC_IRKEY_MULTI_PIP = 0x70,
    DSC_IRKEY_MULTIMEDIA = 0x98,
    IRKEY_RED = 0xF1,
    IRKEY_GREEN = 0xF2,
    IRKEY_KEY_DISABLE_KEYPAD = 0xF3,
    IRKEY_DA	= 0x50, 	//Add DA Key - 070821 DHShin
//  2007_9_4	kwansu : add --- BEGIN
    IRKEY_VOLUME_30 = 0x85,
    IRKEY_VOLUME_50 = 0x86,
    IRKEY_VOLUME_100 = 0x87,
//  2007_9_4	kwansu : add --- END
#if USE_Q_MENU_MENU
    IRKEY_Q_MENU = 0x45,
#endif
#if (ENABLE_TTX_OPTION_MENU)
    IRKEY_TTX_OPTION = 0x21,
#endif
    IRKEY_SHOPMODE =0xA0,		// <071017_dreampark> Virtual key for SHOP MODE
    IRKEY_AV_MODE=0x30,             // <071022_dreampark> for AV MODE
    IRKEY_EYE=0x95,             // [2007-11-8 HOONI] Add eye function
} IrCommandType;

//******************************************************************************

//*************************************************************************
// IR system parameter define for H/W setting (Please don't modify them)
//*************************************************************************
#define IR_CKDIV_NUM            ((BIU_CLOCK + 500000UL) / 1000000UL)
#define IR_CKDIV_NUM_BOOT       13

#define IR_CLK_BOOT             (BIU_CLOCK_BOOT / 1000000.0)
#define IR_CLK                  (BIU_CLOCK / 1000000.0)
#define irGetMinCnt_BOOT(time, tolerance) (((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))*((double)1-tolerance))
#define irGetMaxCnt_BOOT(time, tolerance) (((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))*((double)1+tolerance))
#define irGetMinCnt(time, tolerance) (((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1-tolerance))
#define irGetMaxCnt(time, tolerance) (((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1+tolerance))

#define irGetCnt_BOOT(time) (((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))+0x300000UL)
#define irGetCnt(time) (((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))+0x300000UL)

// 12Mhz
#define IR_RP_TIMEOUT_BOOT      irGetCnt_BOOT(IR_TIMEOUT_CYC)
#define IR_HDC_UPB_BOOT         irGetMaxCnt_BOOT(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB_BOOT         irGetMinCnt_BOOT(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB_BOOT         irGetMaxCnt_BOOT(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB_BOOT         irGetMinCnt_BOOT(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB_BOOT      irGetMaxCnt_BOOT(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB_BOOT      irGetMinCnt_BOOT(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB_BOOT       irGetMaxCnt_BOOT(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB_BOOT       irGetMinCnt_BOOT(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB_BOOT         irGetMaxCnt_BOOT(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB_BOOT         irGetMinCnt_BOOT(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB_BOOT         irGetMaxCnt_BOOT(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB_BOOT         irGetMinCnt_BOOT(IR_LOGI_1_TIME, 0.2)

// 90Mhz
#define IR_RP_TIMEOUT           irGetCnt(IR_TIMEOUT_CYC)
#define IR_HDC_UPB              irGetMaxCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB              irGetMinCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB              irGetMaxCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB              irGetMinCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB           irGetMaxCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB           irGetMinCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB            irGetMaxCnt(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB            irGetMinCnt(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB              irGetMaxCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB              irGetMinCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB              irGetMaxCnt(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB              irGetMinCnt(IR_LOGI_1_TIME, 0.2)

// Power off mode
#define PM_IR_TH_VAL            (PM_TH0_VAL & 0x0F)
// PM_IR_TH_GRID specify the time (in us) for each threshold bit.
// So PM_IR_TH_GRID = (1/12M) * (2^14) * 1000000 = (2^14) / 12
#define PM_IR_TH_GRID           (16384/IR_CLK_BOOT)
#define PM_IR_HEADER_CODE_TIME  (IR_HEADER_CODE_TIME-(0x0F-PM_IR_TH_VAL)*PM_IR_TH_GRID)
#define PM_IR_HDC_UPB_BOOT      irGetMaxCnt_BOOT(PM_IR_HEADER_CODE_TIME, 0.6)
#define PM_IR_HDC_LOB_BOOT      irGetMinCnt_BOOT(PM_IR_HEADER_CODE_TIME, 0.6)
//*************************************************************************

#endif

