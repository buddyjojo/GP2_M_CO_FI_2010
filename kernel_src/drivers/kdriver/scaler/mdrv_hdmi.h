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
/// @file   mdrv_hdmi.h
/// @brief  MStar Scaler Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DRV_HDMI_H__
#define __DRV_HDMI_H__

//#include "mdrv_scaler_st.h"

#include "mdrv_scaler.h"
#include "mhal_hdmi.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_HDMI_BLACK_LEVEL_LOW = 0,
    E_HDMI_BLACK_LEVEL_HIGH
} HDMI_BLACK_LEVEL_t;

typedef struct
{
    HDMI_BLACK_LEVEL_t BlackLevel;
    //ACE Level High
    U32 ACE_HDMI_BLACK_LEVEL_HIGH_YC_OFFSET;
    U32 ACE_HDMI_BLACK_LEVEL_HIGH_YC_GAIN;

    //ACE Level Low
    U32 ACE_HDMI_BLACK_LEVEL_LOW_YC_OFFSET;
    U32 ACE_HDMI_BLACK_LEVEL_LOW_YC_GAIN;
} HDMI_ACE_LEVEL_INFO;

typedef struct
{
    BOOL bDoHDMImodeSetting;
    BOOL bMuteHDMIVideo;
    BOOL bMuteHDMIAudio;
    BOOL bEnableNonPCM;
    U8   u8AudioStbCnt;
    U8   u8AudioForceEnableCnt;
    U8   u8ColorFormat;
    U8   u8LostHDMICounter;
    U8   u8AspectRatio;
} HDMI_POLLING_STS_t;

// HDMI color format
typedef enum
{
    HDMI_COLOR_RGB,
    HDMI_COLOR_YUV_422,
    HDMI_COLOR_YUV_444,
    HDMI_COLOR_RESERVED,
    HDMI_COLOR_DEFAULT = HDMI_COLOR_RGB,
    HDMI_COLOR_UNKNOWN = 7,
} HDMI_COLOR_FORMAT_e;

//------------------------------------------------------------------------------
//  IOCTL entry
//------------------------------------------------------------------------------
void MDrv_HDMI_SetMux(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_HDMI_GetInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_HDMI_GetAspectRatio(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_HDMI_GetxvYCC(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_HDMI_SetHDCP(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); // LGE wlgnsl99
void MDrv_HDMI_GetHDMIColorDomain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080910

//------------------------------------------------------------------------------
//  HDMI State Machine
//------------------------------------------------------------------------------
void MDrv_HDMI_Start(PSC_SOURCE_INFO_t psrc);
void MDrv_HDMI_Stop(PSC_SOURCE_INFO_t psrc);

//------------------------------------------------------------------------------
//  Function
//------------------------------------------------------------------------------
void MDrv_HDMI_Init(void);
void MDrv_HDMI_Reset(void);
void MDrv_HDMI_SetACE(HDMI_ACE_LEVEL_INFO eACEInfo);
void MDrv_HDMI_Get_Mode(SC_INPUT_SOURCE_e enInput, HDMI_STATUS_t enStatus);

// HDCP
void MDrv_HDMI_SetHPD(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_HDMI_Ctrl_DviClock(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_HDCP_InitProductionKey(void);

void MDrv_HDCP_IsExist(SC_INPUT_SOURCE_e enInput, HDCP_STATUS_t enHDCPStatus);
void MDrv_HDCP_PullHPD(SC_INPUT_SOURCE_e enInput, HDCP_STATUS_t enHDCPStatus);
void MDrv_HDMI_DVIClock_Enable(BOOL bEnable, SC_INPUT_SOURCE_e enInput); // FitchHsu 20081202 DVI clock controll error


void MDrv_HDMI_InitVariables(PSC_SOURCE_INFO_t psrc);
HDMI_COLOR_FORMAT_e MDrv_HDMI_GetPacketColorFormat(void);

// Thread Handler
SC_HDMI_STATE_e MDrv_HDMI_InitHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_Debounce(void);
SC_HDMI_STATE_e MDrv_HDMI_TimingChangeHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_SyncDetectHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_CheckModeHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_SearchModeHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_GetPacketInfoHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_DoAVMuteHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_ChkAudioPktHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_ChkColorFmtHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_ChkPktErrHandler(PSC_SOURCE_INFO_t psrc);
SC_HDMI_STATE_e MDrv_HDMI_StableCheckHandler(PSC_SOURCE_INFO_t psrc);

void MDrv_HDMI_GetVSI(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_HDMI_GetAVI(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);

#endif // __DRV_HDMI_H__
