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
#ifndef DRV_SCALER_PCMODE_H
#define DRV_SCALER_PCMODE_H

#include "mdrv_scaler.h"
#include "mdrv_scaler_st.h"

//------------------------------------------------------------------------------
//  Definition
//------------------------------------------------------------------------------
// Range of RGB OOR
#define MIN_HFREQ_OF_RGB        270
#define MAX_HFREQ_OF_RGB        840
#define MIN_VFREQ_OF_RGB        470
#define MAX_VFREQ_OF_RGB        770
#define MIN_VTOTAL_OF_RGB       415
#define MAX_VTOTAL_OF_RGB       1250

// Range of YPbPr OOR
#define MIN_HFREQ_OF_YPBPR      140
#define MAX_HFREQ_OF_YPBPR      700
#define MIN_VFREQ_OF_YPBPR      230
#define MAX_VFREQ_OF_YPBPR      770
#define MIN_VTOTAL_OF_YPBPR     250
#define MAX_VTOTAL_OF_YPBPR     1300

// Match mode torlance
#define SC_PCMODE_HFREQ_TORLANCE    15      // Unit: 0.1kHz
#define SC_PCMODE_VFREQ_TORLANCE    15      // Unit: 0.1Hz
#define SC_PCMODE_FREQ_DELTA        100     // user mode

// Timing change torlance
#define SC_PCMODE_HPERIOD_TORLANCE  10
#define SC_PCMODE_VTOTAL_TORLANCE   10


//------------------------------------------------------------------------------
//  Structure
//------------------------------------------------------------------------------

//*************************************************************************
//          Enums
//*************************************************************************


#define MD_TIMING_STABLE_COUNT      25      // input timing stable counter
#define MD_TIMING_NOSYNC_COUNT      40


// mode change torlance
#define MD_HPERIOD_TORLANCE         10      // horizontal period torlance
#define MD_VTOTAL_TORLANCE          10      // vertical total torlance

#define MD_HDE_TORLANCE             5
#define MD_VDE_TORLANCE             5

#define MD_HFREQ_TORLANCE           15      // Unit: 0.1kHz
#define MD_VFREQ_TORLANCE           15      // Unit: 0.1Hz

#define MD_FREQ_DELTA               100     // search the user mode

#define MD_MAX_VTOTAL               1200    // Saturn only support 135MHz input
                                            // SXGA max is 1200, UXGA max is 2020
//==========================================================================

/// PC ADC mode table type
typedef struct
{
    MADP_SC_RESOLUTION_TYPE_e enResolutionIndex; ///< resolution

    U16 u16HorizontalFrequency; ///< Horizontal frequency
    U16 u16VerticalFrequency;   ///< Vertical frequency

    U16 u16HorizontalStart;     ///< Horizontal start
    U16 u16VerticalStart;       ///< Vertical start

    U16 u16HorizontalTotal;     ///< Horizontal Total
    U16 u16VerticalTotal;       ///< Vertical Total

    U8  u8VTotalTolerance;      ///< VTotal tolerance
    U8  u8AdcPhase;             ///< ADC phase

    U8  u8StatusFlag;           ///< flags
    // b0: VSync polarity(1/0 = positive/negative)
    // b1: HSync polarity(1/0 = positive/negative)
    // b2: Sync polarity care bit
    // b4: interlace mode
} SC_PCADC_MODETABLE_TYPE;

#if 0
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    EN_RESOLUTION_TYPE resolution;
} SC_SET_RESOLUTION_t;
#endif

//*************************************************************************
//          Structures
//*************************************************************************
// input resolution
typedef struct
{
    U16 u16DisplayWidth;  // width
    U16 u16DisplayHeight; // height
} SC_PCMODE_RESOLUTION;

/********************************************************************************/
/*                   Function Prototypes                     */
/********************************************************************************/
U16  MDrv_SC_PCMode_GetStdModeResH(U8 u8ModeIndex);
U16  MDrv_SC_PCMode_GetStdModeResV(U8 u8ModeIndex);

//U8   MDrv_SC_PCMode_GetStdModeStatusFlag(U8 u8ModeIndex);
//U8   MDrv_SC_PCMode_GetStdModeVTotalTolerance(U8 u8ModeIndex);
//U16  MDrv_SC_PCMode_GetStdModeVFreq(U8 u8ModeIndex);

BOOL MDrv_SC_PCMode_CompareModePolarity(U8 u8InputStatusFlag, U8 u8StdModeIndex);

//BOOL MDrv_SC_PCMode_SyncLossDetect(SC_SOURCE_INFO_t enSourceInfo);
void MDrv_SC_PCMode_LoadDefaultTable(U8 u8ModeIndex);
BOOL MDrv_SC_PCMode_Verify_ModeTable_Valid(U8 u8ModeIndex);
//BOOL MDrv_SC_PCMode_TimingValidDetect(void);
//BOOL MDrv_SC_PCMode_TimingDetect ( SC_SOURCE_INFO_t enSourceInfo );
//U8   MDrv_SC_PCMode_MatchMode2(SC_PCADC_MODETABLE_TYPE pstInputType, SC_SOURCE_INFO_t enInputSourceType);
//BOOL MDrv_SC_PCMode_EVENT_SearchMode(SC_SOURCE_INFO_t enSourceInfo);
//BOOL MDrv_SC_PCMODE_EVENT_MatchMode(void);
U8 MDrv_SC_PCMode_MatchMode(PSC_SOURCE_INFO_t psrc, SC_PCMODE_MODETABLE_t* pModeTable, BOOL bYPbPrSource);

SC_PCMODE_STATE_e MDrv_SC_PCMode_InitHandler(void);
SC_PCMODE_STATE_e MDrv_SC_PCMode_Debounce(void);
SC_PCMODE_STATE_e MDrv_SC_PCMode_TimingChangeHandler(PSC_SOURCE_INFO_t psrc);
SC_PCMODE_STATE_e MDrv_SC_PCMode_SyncDetectHandler(PSC_SOURCE_INFO_t psrc);
SC_PCMODE_STATE_e MDrv_SC_PCMode_SearchModeHandler(PSC_SOURCE_INFO_t psrc);
SC_PCMODE_STATE_e MDrv_SC_PCMode_StableSyncCheckHandler(PSC_SOURCE_INFO_t psrc);

void MDrv_SC_PCMode_SetModeTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_PCMode_SetResolutionIndex(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);

void MDrv_SC_PCMode_GetInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_PCMode_SetInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_AutoAdjust(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetGain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_GetGain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetOffset(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_GetOffset(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_Phaseadjust(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_PCMode_Start(PSC_SOURCE_INFO_t psrc);
void MDrv_SC_PCMode_Stop(PSC_SOURCE_INFO_t psrc);


U16 MDrv_SC_PCMode_GetStdModeHTotal(U8 u8ModeIndex);//FitchHsu 20081127 PCmode Auto config issue

// shjang_091006 20091006 ykkim5
void MDrv_SC_SetCompSyncLevel(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_PCMode_GetInfo_FromReg(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//ykkim5 091122

#endif /* DRV_SC_PCMODE_H */
