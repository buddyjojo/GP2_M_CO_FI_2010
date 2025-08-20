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

#ifndef _HAL_HDMI_H_
#define _HAL_HDMI_H_

#include "mdrv_types.h"
#include "mdrv_scaler_st.h"
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/*
#ifdef MAPP_PCMODE_C
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE HDMI_PACKET_INFO g_HdmiPacketInfo;
INTERFACE HDMI_POLLING_STATUS    g_HdmiPollingStatus;
INTERFACE HDCP_POLLING_STATUS    g_HdcpPollingStatus;

#undef INTERFACE
*/
//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

typedef enum
{
    REST_AVMUTE                = BIT8,
    REST_Y_COLOR               = BIT9,
    REST_PIXEL_REPETITION      = BIT10,
    REST_FRAME_REPETITION      = BIT11,
    REST_GCP_PKT               = BIT12,
    REST_DEEP_COLOR_FIFO       = BIT13,
    REST_RESERVE               = BIT14,
    REST_HDMI_STATUS           = BIT15,
}HDMI_REST_t;

typedef enum
{
    E_HDMI_STATUS_UNKNOWN,
    E_HDMI_STATUS_DVI,
    E_HDMI_STATUS_HDMI,
}HDMI_DET_STATUS_t;


typedef struct
{
    HDMI_DET_STATUS_t Srcdet;
    BOOL bIsHDMIMode;
/*
    BOOL bDoHDMImodeSetting;
    BOOL bMuteHDMIVideo;
    BOOL bMuteHDMIAudio;
    BOOL bEnableNonPCM;

    U8   u8ColorFormat;
    U8   u8LostHDMICounter;
    U8   u8AspectRatio;
*/
} HDMI_STATUS_t;

typedef struct
{
//HPD
    BOOL bHPD_INVERSE;  //by Board define
    U8 u8HighLowCtl;

    // HPD
    U8   u8HPD_PullHigh;

//HDCP
    BOOL bHDCP_EN;
    U16  u16HDCP_KeyCounter;
    U16  u16HDCP_KeyChkSum;
    U8   u8BKSV[5];

//Status
    BOOL bHPD_OK;
    BOOL bHDCP_EXIST;
} HDCP_STATUS_t;

typedef enum
{
    DVI_MODE  = 0,
    HDMI_MODE = 1,
} HAL_HDMI_MODE_RPT_e;

// packet color format
typedef enum
{
    HDMI_COLOR_FORMAT_RGB,
    HDMI_COLOR_FORMAT_YUV_422,
    HDMI_COLOR_FORMAT_YUV_444,
} HAL_HDMI_COLOR_FORMAT_e; //thchen 20080719

// active format aspect ratio
typedef enum
{
    HDMI_ACTIVE_AR_PICTURE = BIT0,
    HDMI_ACTIVE_AR_4_3     = BIT1,
    HDMI_ACTIVE_AR_16_9    = BIT2,
    HDMI_ACTIVE_AR_14_9    = BIT3,
    HDMI_ACTIVE_AR_OTHER   = BIT4,
} HAL_HDMI_ACTIVE_AR_e;

// picture aspect ratio
typedef enum
{
    HDMI_PICTURE_AR_NONE   = BIT5,
    HDMI_PICTURE_AR_4_3    = BIT6,
    HDMI_PICTURE_AR_16_9   = BIT7,
} HAL_HDMI_PICTURE_AR_e;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MHal_HDMI_Init(void);
void MHal_HDMI_DVIClock_Enable(BOOL bEnable, SC_INPUT_SOURCE_e enInput);
void MHal_HDMI_Reset(void);
#if 0
void MHal_HDMI_SetBlackLevel(U32 ace_level_gain, U32 ace_level_offset);
#endif

//HDCP
void MHal_HDCP_InitProductionKey( U8 * pu8HdcpKeyData );
void MHal_HDCP_ClearStatus(void);
void MHal_HDCP_SetEQ(U8 u8EqLevel);
void MHal_HDMI_SetMux(SC_INPUT_SOURCE_e enInput);
#if 0
void MHal_HDCP_Enable( BOOL bEnable );
BOOL MHal_HDCP_Exist( void );
#endif

void MHal_HDMI_HPD_High(BOOL bHigh, SC_INPUT_SOURCE_e enInput);
//Audio
void MHal_HDMI_AudioOutput( BOOL bEnable );


HAL_HDMI_MODE_RPT_e MHal_HDMI_GetModeRPT(void);
void MHal_HDMI_ResetPacket(HDMI_REST_t breset);
U16  MHal_HDMI_GetHDMIST1(void);
U8   MHal_HDMI_GetACPHB1(void); //victor 20081117, ACP
void MHal_AuSPDIF_HWEN(BOOL bEnable);//victor 20081117, ACP, this is the MAD function
BOOL MHal_HDMI_IsAVMUTE(void);
HAL_HDMI_COLOR_FORMAT_e MHal_HDMI_GetPacketColorFormat(void);
SC_HDMI_COLORIMETRY_FORMAT_e MHal_HDMI_GetExtendColorimetryInfo(void);
SC_HDMI_EXTENDEDCOLORIMETRY_FORMAT_e MHal_HDMI_GetPacketExtendedColorimetry(void);
U8   MHal_HDMI_GetGMPktCurrentGamutSeq(void);
void MHal_HDMI_GetGMPktGBDInfo(U8* pu8GBDInfo);
U8   MHal_HDMI_GetAspectRatio(void);
BOOL MHal_HDMI_IsAudioNotPCM(void);
BOOL MHal_HDMI_IsChecksumOrBCHParityErr(void);
BOOL MHal_HDMI_IsAudioPacketErr(void);
void MHal_HDMI_ClearErr(void);
void MHal_HDMI_AuDownSample(void);//victor 20081215, HDMI audio
void MHal_HDMI_EnableDDCBus(BOOL bEnable);//victor 20081215, DDC
void MHal_HDMI_GetVSI(SC_GET_HDMI_VSI_PACKET_t *pvsipacket);
void MHal_HDMI_GetAVI(SC_GET_HDMI_AVI_PACKET_t *pavipacket);

#if 0
void MHal_HDMI_SetPostSettingForHDMI(void);
void MHal_HDMI_EnableDeep(void);

//HPD
void MHal_HPD_PullHPD_Src1(BOOL HPD_INVERSE, U8 u8HighLow);
void MHal_HPD_PullHPD_Src2(BOOL HPD_INVERSE, U8 u8HighLow);
void MHal_HPD_PullHPD_Src3(BOOL HPD_INVERSE, U8 u8HighLow);
#endif

#if 0 // remove this patch because no need in T3
// LGE earnest 2009/01/24 		Tuner init time is slower because of this interrupt in PDP. We must move init routine after starting HDMI.
void MHal_HDMI_EnableISR4VideoClkFreqBigChange(void);
void MHal_HDMI_DisableISR4VideoClkFreqBigChange(void);

//victor 20081229, DVI+HDCP snow noise patch
void MHal_DVI_AdjustADC(BOOL bClockLessThan70MHz);
BOOL MHal_DVI_IsVideoStable(void);
U16  MHal_DVI_TMDSClock(void);
U16  MHal_HDMI_InterruptStatus(void);
void MHal_HDMI_ResetPacketStatus(void);
#endif

// T3 DVI ATOP/DTOP
BOOL MHal_HDMI_DVISyncLoss(SC_INPUT_SOURCE_e enInput);

#endif // _HAL_HDMI_H_
