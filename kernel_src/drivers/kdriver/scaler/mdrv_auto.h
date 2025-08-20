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

#ifndef __MDRV_AUTO_H__
#define __MDRV_AUTO_H__

#include "mdrv_scaler.h"

#define AUTO_PHASE_METHOD           0   // 0 = found maximum, 1 = found minimum

#define MIN_PC_AUTO_H_START			((psrc->Modesetting.u16HorizontalStart) - ((psrc->Modesetting.u16HorizontalStart) * 8 / 10))
#define MAX_PC_AUTO_H_START			((psrc->Modesetting.u16HorizontalStart) + ((psrc->Modesetting.u16HorizontalStart) * 8 / 10))
#define MIN_PC_AUTO_V_START			(1)
#define MAX_PC_AUTO_V_START			((psrc->Modesetting.u16VerticalStart)*2 - MIN_PC_AUTO_V_START/*MIN_PC_V_START*/)

#define AUTO_MIN_R  0
#define AUTO_MIN_G  0x01
#define AUTO_MIN_B  0x02
#define AUTO_MAX_R  0x03
#define AUTO_MAX_G  0x04
#define AUTO_MAX_B  0x05

typedef struct
{
    U16 x;
    U16 y;
    U16 width;
    U16 height;
} AUTO_CAL_WINDOW_t;

typedef struct
{
    U16 CH_AVG[3]; // R/G/B
} MS_AUTOADC_TYPE;


//lachesis_081128 from fitch
#define HTOTAL_ADJUST_DELTA_B		((U32)(MDrv_SC_PCMode_GetStdModeHTotal(psrc->Modesetting.u8ModeIndex) - MDrv_SC_PCMode_GetStdModeResH(psrc->Modesetting.u8ModeIndex))*95/100)
#define HTOTAL_ADJUST_MIN			(MDrv_SC_PCMode_GetStdModeHTotal(psrc->Modesetting.u8ModeIndex) - (HTOTAL_ADJUST_DELTA_B/2))
#define HTOTAL_ADJUST_MAX			(MDrv_SC_PCMode_GetStdModeHTotal(psrc->Modesetting.u8ModeIndex) + (HTOTAL_ADJUST_DELTA_B/2))
#define MaxMeasuredWidthDelta		(HTOTAL_ADJUST_DELTA_B*8/10)  //   64% of clock

#define MIN_SRC_WIDTH_DELTA			(4)
#define MAX_SRC_WIDTH_DELTA			(2)
#define TOT_SRC_WIDTH_DELTA			(MIN_SRC_WIDTH_DELTA+MAX_SRC_WIDTH_DELTA+1)

#define PHASE_MIN	    0
#define PHASE_MAX	    0x7F     // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112 autoconfig)
#define PHASE_STEP	    8
#define AUTO_PHASE_STEP 4		//ykkim5 091112


#define R_CHANNEL_MIN    0x00
#define G_CHANNEL_MIN    0x01
#define B_CHANNEL_MIN    0x02
#define R_CHANNEL_MAX    0x03
#define G_CHANNEL_MAX    0x04
#define B_CHANNEL_MAX    0x05


typedef enum
{
    AUTO_TUNE_NULL          = 0x00,                     ///< Auto Tune Null

    // Basic auto-tune
    AUTO_TUNE_VALID_DATA    = BIT0,                     ///< Auto Tune Valid Data
    AUTO_TUNE_POSITION      = BIT1,                     ///< Auto Tune Position
    AUTO_TUNE_FREQ          = BIT2,                     ///< Auto Tune Frequency
    AUTO_TUNE_PHASE         = BIT3,                     ///< Auto Tune Phase
    AUTO_TUNE_BASIC         = BIT0|BIT1|BIT2|BIT3,      ///< Auto Tune Basic

    AUTO_TUNE_OFFSET        = BIT4,                     ///< Auto Tune RGB Offset
    AUTO_TUNE_GAIN          = BIT5,                     ///< Auto Tune RGB Gain

    // [6]: 1 tune YUV, [6]: 0 tune RGB
    AUTO_TUNE_YUV           = BIT6,                     ///< Auto Tune YUV Color
	AUTO_TUNE_INT_ADC 		= BIT7, 					//victor 20080902, for Internal ADC

    // Advance auto-tune
    AUTO_TUNE_ADVANCE       = BIT0|BIT1|BIT2|BIT3|BIT4|BIT5 ///< Auto Tune Advance
} SC_AUTO_TUNE_TYPE_e;

//FitchHsu 20081127 PCmode Auto config issue
typedef enum
{
    AUTO_RESULT_OK,
    AUTO_RESULT_FAIL,
    AUTO_RESULT_CONTINUE
}eAUTORETURN;

//------------------------------------------------------------------------------
//  Function
//------------------------------------------------------------------------------
BOOL MDrv_Auto_Geometry(PSC_DRIVER_CONTEXT_t pDrvCtx, PSC_SOURCE_INFO_t psrc, SC_AUTO_TUNE_TYPE_e AutoTuneType, 
                        U16 TargetForRGain, U16 TargetForGGain, U16 TargetForBGain);    // 20091012 daniel.huang: for finetune internal calibration
U16  MDrv_Auto_GetPosition(PSC_SOURCE_INFO_t psrc, U8 u8RegIndex, U8 u8VSyncTime);
U16  MDrv_Auto_GetActualWidth(PSC_SOURCE_INFO_t psrc, U8 u8VSyncTime);
BOOL MDrv_Auto_CheckSyncLoss(PSC_SOURCE_INFO_t psrc);
BOOL MDrv_Auto_TunePhase(PSC_SOURCE_INFO_t psrc, U8 u8VSyncTime);
BOOL MDrv_Auto_TunePosition(PSC_SOURCE_INFO_t psrc, U8 u8VSyncTime);
BOOL MDrv_Auto_TuneHTotal(PSC_SOURCE_INFO_t psrc, U8 u8VSyncTime); //FitchHsu 20081127 PCmode Auto config issue

// 20090928 daniel.huang: refine ADC calibration and update calibration algorithm
BOOL MDrv_ADC_CalGain( SC_AUTO_TUNE_TYPE_e enAutoTuneType, PSC_SOURCE_INFO_t psrc,
                       U16 GainTarget[3], 
                       AUTO_CAL_WINDOW_t *pCalWinG_Y,
                       AUTO_CAL_WINDOW_t *pCalWinB_Cb,
                       AUTO_CAL_WINDOW_t *pCalWinR_Cr);

#endif//__MDRV_AUTO_H__
