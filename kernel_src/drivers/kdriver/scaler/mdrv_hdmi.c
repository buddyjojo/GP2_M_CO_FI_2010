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
/// file    mdrv_hdmi.c
/// @brief  HDMI Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>

#include "mdrv_types.h"
#include "mdrv_hdmi.h"
#include "mdrv_scaler_st.h"
#include "mdrv_scaler.h"
#include "mdrv_adc.h"
#include "mdrv_gpio.h"

#include "mhal_scaler.h"
#include "mhal_hdmi.h"

#include "mst_utility.h"

#if 1	// LGE for prevent pop niose on specific HDMI device by Jonghyuk, Lee 100107
#include "../mad/mdrv_mad_process.h"
#endif

//------------------------------------------------------------------------------
//  Constant definition
//------------------------------------------------------------------------------
#define OPT_HDMI_DGB 1
#if OPT_HDMI_DGB
#define HDMI_DGB(fmt, args...)  //printk(" " fmt, ##args)
#define SC_HDMI_DEBUG(fmt, args...)     printk("\033[47;34m[HDMI][%05d] " fmt "\033[0m", __LINE__, ## args)
#else
#define HDMI_DGB(fmt, args...)
#define SC_HDMI_DEBUG(fmt, args...)
#endif

// Timing change torlance
#define HDMI_HPERIOD_TORLANCE   10
#define HDMI_VTOTAL_TORLANCE    10
#define HDMI_HDE_TORLANCE       0    //5	3233 port2 1080P no-signal 문제 개선. //lachesis_081021 HDMI 1080i invalid로 인식하는 문제
#define HDMI_VDE_TORLANCE       0 //5

// Range of HDMI OOR
#define MIN_HFREQ_OF_HDMI       140
#define MAX_HFREQ_OF_HDMI       770 //700 for 9699GT PC38321
#define MIN_VFREQ_OF_HDMI       230
#define MAX_VFREQ_OF_HDMI       770
#define MIN_VTOTAL_OF_HDMI      250
#define MAX_VTOTAL_OF_HDMI      1300

// Range of DVI OOR
#define MIN_HFREQ_OF_DVI        140
#define MAX_HFREQ_OF_DVI        840
#define MIN_VFREQ_OF_DVI        230
#define MAX_VFREQ_OF_DVI        770
#define MIN_VTOTAL_OF_DVI       250
#define MAX_VTOTAL_OF_DVI       1250

// HPD pull-hight
#define HPD_PULL_HIGHT_A        BIT0
#define HPD_PULL_HIGHT_B        BIT1
#define HPD_PULL_HIGHT_C        BIT2
#define HPD_PULL_HIGHT_ALL      (BIT0 | BIT1 | BIT2)

// HDCP
#define HDCP_KEY_SIZE           289

// Audio
#define HDMI_AUDIO_STABLE_CNT    20//40 /* LGE To reduce mute off on audioPktHandler. Jonghyuk, Lee 100108 */
#define HDMI_AUDIO_FORCE_ENABLE (HDMI_AUDIO_STABLE_CNT*2)


// HDMI aspect ratio type
typedef enum
{
    // Active Format Aspect Ratio - AFAR
    HDMI_AFAR_SAME   = 0x08,    // IF0[11..8] 1000, same as picture
    HDMI_AFAR_4_3_C  = 0x09,    // IF0[11..8] 1001, 4:3 Center
    HDMI_AFAR_16_9_C = 0x0A,    // IF0[11..8] 1010, 16:9 Center
    HDMI_AFAR_14_9_C = 0x0B,    // IF0[11..8] 1011, 14:9 Center

    // Picture Aspect Ratio - PAR
    HDMI_PAR_NODATA = 0x00,     // IF0[13..12] 00
    HDMI_PAR_4_3    = 0x10,     // IF0[13..12] 01, 4:3
    HDMI_PAR_16_9   = 0x20,     // IF0[13..12] 10, 16:9
    HDMI_PAR_RSV    = 0x30,     // IF0[13..12] 11, reserved
} HDMI_AR_TYPE_e;

typedef struct xvYCC_INFO_s
{
    U8 Current_Gamut_Seq;
    U8 GM_HB[2];                                    // GM packet HB1 and HB2
    U8 GM_GBD[(GM_GBD_MAX_SIZE+0x1) & ~0x1];        // Currently, support up to 21 bytes GBD
} xvYCC_INFO_t, *PxvYCC_INFO_t;

//------------------------------------------------------------------------------
//  Data structure
//------------------------------------------------------------------------------
typedef struct
{
    U16 HDMIST1;

    // Use following MACROs carefully, don't combined with other operator
    #define bMPEG_PKT_Received          HDMIST1 & BIT0
    #define bAUI_PKT_Received           HDMIST1 & BIT1
    #define bSPD_PKT_Received           HDMIST1 & BIT2
    #define bAVI_PKT_Received           HDMIST1 & BIT3
    #define bGC_PKT_Received            HDMIST1 & BIT4
    #define bASAMPLE_PKT_Received       HDMIST1 & BIT5
    #define bACR_PKT_Received           HDMIST1 & BIT6
    #define bVS_PKT_Received            HDMIST1 & BIT7
    #define bNULL_PKT_Received          HDMIST1 & BIT8
    #define bISRC2_PKT_Received         HDMIST1 & BIT9
    #define bISRC1_PKT_Received         HDMIST1 & BIT10
    #define bACP_PKT_Received           HDMIST1 & BIT11
    #define bONEBIT_AUD_PKT_Received    HDMIST1 & BIT12
    #define bGM_PKT_Received            HDMIST1 & BIT13

    U8 bAVMuteStatus;
    U8 bAudioNotPCM;
    U8 bPreAudiostatus;
    U8 bChecksumErrOrBCHParityErr;
    U8 u8PacketColorFormat;
    U8 u8AspectRatio;
    U8 bAudioPacketErr;
    SC_HDMI_COLORIMETRY_FORMAT_e eColorimetry;
    SC_HDMI_EXTENDEDCOLORIMETRY_FORMAT_e eExtColorimetry;

    // xvYCC Info
    xvYCC_INFO_t xvYCC_Info;
} HDMI_PACKET_INFO_t;

//------------------------------------------------------------------------------
//  Forward declaration
//------------------------------------------------------------------------------
extern void MDrv_MICOM_Init(void);
extern void MDrv_MAD_ProcessSetMute(U8, BOOL);

//------------------------------------------------------------------------------
//  Local variable
//------------------------------------------------------------------------------
static HDMI_POLLING_STS_t g_HdmiPollingSts;
static HDMI_PACKET_INFO_t g_HdmiPacketInfo;
//static HDCP_STATUS_t      g_HdcpStatus;
static U8 g_u8AVIPacketCounter = 0; //thchen 20080814
//static U8 u8Goodsycnthreshold = 0;		// LGE wlgnsl99 20081110
static U8 u8ScResetCnt = 0;
static U8 u8NoPacketCnt = 0;		// LGE wlgnsl99 20090111
static U8 u8SearchModeCnt = 0;		// LGE wlgnsl99 20090916


// FitchHsu 20090524 T3 HDMI for bring up
#if 0//def T3_HDMI_BRINGUP
static U8 HdcpKey[289] =
{
    0x89,0x38,0xAE,0x0D,0xED,0x40,0xE6,0xBB,0xFA,0x4E,0xDE,0x51,0xFB,0x8E,0xD9,0xAA,
    0x34,0xA8,0xC4,0xEA,0xD8,0x6C,0xDC,0x5C,0x91,0x5C,0xB1,0xA6,0x13,0x2B,0x8B,0x8B,
    0xF7,0x46,0xCC,0x1C,0x88,0x20,0xA3,0x27,0x0E,0xE1,0x28,0x84,0x89,0x39,0xA3,0xE2,
    0x36,0x86,0xCE,0x67,0xEB,0xA0,0xF2,0x35,0x6B,0x86,0xF5,0x21,0x71,0x95,0x8A,0x77,
    0xA1,0x28,0x77,0x97,0xD3,0x7B,0xEF,0x5C,0x15,0x48,0xAA,0x9E,0x97,0x39,0xCD,0x98,
    0x40,0x5E,0x68,0x56,0x66,0xEF,0xC1,0x3C,0xE1,0x8F,0x2A,0x82,0xDE,0x8F,0x52,0xCC,
    0xA8,0x1F,0x37,0xD9,0xD4,0xC6,0x24,0x16,0x7E,0x42,0xFF,0x57,0xCD,0x6B,0xE0,0x86,
    0x00,0x1A,0xF1,0x19,0x5A,0xAF,0x37,0x97,0x86,0xBA,0x83,0x29,0xFE,0x41,0xA8,0xD5,
    0xF4,0x73,0x43,0x03,0x23,0x22,0xC5,0x28,0x96,0x9E,0x35,0x0D,0x67,0xA8,0x8B,0xDD,
    0x7A,0x89,0x38,0xE0,0x94,0xF0,0xFF,0xF5,0x8F,0xF3,0x4E,0x5C,0x82,0x09,0xF3,0x97,
    0xEB,0x01,0x52,0xEC,0xD8,0x98,0x5C,0x4F,0x43,0x2E,0xE7,0x9F,0xF5,0x85,0x6D,0x15,
    0xB1,0x83,0x20,0xF8,0x5E,0xD0,0x33,0x4F,0xF0,0xC1,0x8F,0x65,0x77,0x3D,0x31,0xB2,
    0xFB,0xA1,0x6E,0xCA,0xA6,0xD3,0xA2,0x35,0x1D,0x16,0x41,0xC3,0x89,0x86,0x98,0x78,
    0x8E,0x3E,0xC1,0x64,0x01,0x79,0x05,0x21,0x47,0xAF,0x6A,0x6F,0x5B,0xE1,0x4D,0x2B,
    0x2F,0xCC,0x18,0x8E,0x42,0xDC,0x9A,0xF8,0x3C,0xD0,0xD0,0x57,0x04,0xFB,0x14,0x42,
    0x8C,0x54,0x9D,0xA9,0x06,0xEB,0xE7,0x48,0xE2,0x29,0xEF,0x7E,0xFD,0xF6,0x45,0x12,
    0xAC,0xE4,0xBC,0x45,0x67,0xA3,0x9B,0x65,0xA1,0x0E,0xED,0x1A,0x84,0xAD,0x49,0x87,
    0xA2,0x77,0x3F,0x11,0xA7,0x1B,0xD1,0x7F,0x25,0x36,0x6c,0x6f,0xd3,0xdf,0x25,0xd0,
    0xFB,
};
#else
/* // LGE wlgnsl99
static U8 HdcpKey[289] =
{
    0x89,0x38,0xAE,0x0D,0xED,0x40,0xE6,0xBB,0xFA,0x4E,0xDE,0x51,0xFB,0x8E,0xD9,0xAA,
    0x34,0xA8,0xC4,0xEA,0xD8,0x6C,0xDC,0x5C,0x91,0x5C,0xB1,0xA6,0x13,0x2B,0x8B,0x8B,
    0xF7,0x46,0xCC,0x1C,0x88,0x20,0xA3,0x27,0x0E,0xE1,0x28,0x84,0x89,0x39,0xA3,0xE2,
    0x36,0x86,0xCE,0x67,0xEB,0xA0,0xF2,0x35,0x6B,0x86,0xF5,0x21,0x71,0x95,0x8A,0x77,
    0xA1,0x28,0x77,0x97,0xD3,0x7B,0xEF,0x5C,0x15,0x48,0xAA,0x9E,0x97,0x39,0xCD,0x98,
    0x40,0x5E,0x68,0x56,0x66,0xEF,0xC1,0x3C,0xE1,0x8F,0x2A,0x82,0xDE,0x8F,0x52,0xCC,
    0xA8,0x1F,0x37,0xD9,0xD4,0xC6,0x24,0x16,0x7E,0x42,0xFF,0x57,0xCD,0x6B,0xE0,0x86,
    0x00,0x1A,0xF1,0x19,0x5A,0xAF,0x37,0x97,0x86,0xBA,0x83,0x29,0xFE,0x41,0xA8,0xD5,
    0xF4,0x73,0x43,0x03,0x23,0x22,0xC5,0x28,0x96,0x9E,0x35,0x0D,0x67,0xA8,0x8B,0xDD,
    0x7A,0x89,0x38,0xE0,0x94,0xF0,0xFF,0xF5,0x8F,0xF3,0x4E,0x5C,0x82,0x09,0xF3,0x97,
    0xEB,0x01,0x52,0xEC,0xD8,0x98,0x5C,0x4F,0x43,0x2E,0xE7,0x9F,0xF5,0x85,0x6D,0x15,
    0xB1,0x83,0x20,0xF8,0x5E,0xD0,0x33,0x4F,0xF0,0xC1,0x8F,0x65,0x77,0x3D,0x31,0xB2,
    0xFB,0xA1,0x6E,0xCA,0xA6,0xD3,0xA2,0x35,0x1D,0x16,0x41,0xC3,0x89,0x86,0x98,0x78,
    0x8E,0x3E,0xC1,0x64,0x01,0x79,0x05,0x21,0x47,0xAF,0x6A,0x6F,0x5B,0xE1,0x4D,0x2B,
    0x2F,0xCC,0x18,0x8E,0x42,0xDC,0x9A,0xF8,0x3C,0xD0,0xD0,0x57,0x04,0xFB,0x14,0x42,
    0x8C,0x54,0x9D,0xA9,0x06,0xEB,0xE7,0x48,0xE2,0x29,0xEF,0x7E,0xFD,0xF6,0x45,0x12,
    0xAC,0xE4,0xBC,0x45,0x67,0xA3,0x9B,0x65,0xA1,0x0E,0xED,0x1A,0x84,0xAD,0x49,0x87,
    0xA2,0x77,0x3F,0x11,0xA7,0x1B,0xD1,0x7F,0x25,0x36,0x6c,0x6f,0xd3,0xdf,0x25,0xd0,
    0xFB,
};
*/
static U8 HdcpKey[HDCP_KEY_SIZE]={0};
#endif

//------------------------------------------------------------------------------
//  Global variable
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Exernal
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  IOCTL entry
//------------------------------------------------------------------------------
void MDrv_HDMI_SetMux(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg) // called by Video_ddi
{
    SC_INPUT_SOURCE_e enInput;
    if (copy_from_user(&enInput, (void __user *)arg, sizeof(SC_INPUT_SOURCE_e)))
    {
        return;
    }
    SC_HDMI_DEBUG("MDrv_HDMI_SetMux\n");
    MHal_HDMI_SetMux(enInput);
}

void MDrv_HDMI_GetInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_HDMI_INFO_t param;
    PSC_SOURCE_INFO_t  pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_HDMI_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

    param.bHDMI     = pSrcInfo->bHDMIMode;
    param.u16HSize  = pSrcInfo->u16Input_HDE;
    param.u16VSize  = pSrcInfo->u16Input_VDE;
    param.u16HStart	= pSrcInfo->u16Input_HDE_Start;
    param.u16VStart	= pSrcInfo->u16Input_VDE_Start;

#if 0
    if (g_HdmiPacketInfo.HDMIST1 & BIT5)
    {
        param.bAudio = TRUE;
    }
    else
    {
        param.bAudio = FALSE;
    }
#else

    //FitchHsu 20081024 Audio Error
    if (MHal_SC_IP1_GetInputSourceEnable())
    {
        param.bAudio = FALSE;
    }
    else
    {
        param.bAudio = TRUE;
    }
#endif
    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_HDMI_INFO_t)))
    {
        return;
    }
}

void MDrv_HDMI_GetAspectRatio(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_HDMI_ASPECTRATIO_t param;
    PSC_SOURCE_INFO_t pSrcInfo;
    U8 u8AR;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_HDMI_ASPECTRATIO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    u8AR = MHal_HDMI_GetAspectRatio();

    // check Active Format aspect ration & picture aspect ratio

#if 1 // LGE lemonic 0810xx

    param.u8AspectRatio = u8AR;

#else

    if (u8AR & HDMI_PICTURE_AR_NONE)
    {
        param.u8AspectRatio = HDMI_AR_NONE;
    }
    else if (u8AR & HDMI_PICTURE_AR_4_3)
    {
        param.u8AspectRatio = HDMI_AR_4_3;
    }
    else if (u8AR & HDMI_PICTURE_AR_16_9)
    {
        param.u8AspectRatio = HDMI_AR_16_9;
    }
    else
    {
        param.u8AspectRatio = HDMI_AR_FUTURE;
    }

#endif

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_HDMI_ASPECTRATIO_t)))
    {
        return;
    }
}

void MDrv_HDMI_GetxvYCC(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_HDMI_XVYCC_t param;
    U8 i;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_HDMI_XVYCC_t)))
    {
        return;
    }
    //SC_HDMI_DEBUG("Colorimetry=%u, eExtColorimetry=%u\n", g_HdmiPacketInfo.eColorimetry, g_HdmiPacketInfo.eExtColorimetry);
    param.eColorimetry = g_HdmiPacketInfo.eColorimetry;
    param.eExtColorimetry = g_HdmiPacketInfo.eExtColorimetry;
    for(i=0; i<((GM_GBD_MAX_SIZE+0x1) & ~0x1); i++)
    {
        //SC_HDMI_DEBUG("GM_GBD[%u]=%u\n", i, g_HdmiPacketInfo.xvYCC_Info.GM_GBD[i]);
        param.GM_GBD[i] = g_HdmiPacketInfo.xvYCC_Info.GM_GBD[i];
    }

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_HDMI_XVYCC_t)))
    {
        return;
    }
}

void MDrv_HDMI_SetHDCP(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg) // LGE wlgnsl99
{
    SC_SET_HDMI_HDCP_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_HDMI_HDCP_t)))
    {
        return;
    }

    memcpy(HdcpKey,&param.u8HdcpKey[0],HDCP_KEY_SIZE);

#if 0
    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_SEND_HDMI_HDCP_t)))
    {
        return;
    }
#endif
}

void MDrv_HDMI_GetHDMIColorDomain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_HDMI_COLOR_DOMAIN_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_HDMI_COLOR_DOMAIN_t)))
    {
        return;
    }

    param.colorFormat = (U8)MDrv_HDMI_GetPacketColorFormat();

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_HDMI_COLOR_DOMAIN_t)))
    {
        return;
    }

}//victor 20080910

//------------------------------------------------------------------------------
//  HDMI State Machine
//------------------------------------------------------------------------------
SC_HDMI_STATE_e MDrv_HDMI_InitHandler(PSC_SOURCE_INFO_t psrc)
{
    // disable audio output
    MHal_HDMI_AudioOutput(FALSE);

    // init scaler
    MHal_SC_Reset(SC_RST_IP_F2);
    MHal_SC_IP1_ResetSyncDetect();
    MHal_SC_SetRegeneratedDE(ENABLE);

    MHal_SC_IP1_SetCoastWin(0x0A, 0x0A);

    // init HDMI variables
    MDrv_HDMI_InitVariables(psrc);

    return SC_HDMI_DEBOUNCE_STATE;
}

SC_HDMI_STATE_e MDrv_HDMI_Debounce(void)
{
    static U8 u8Debounce		= 0;
    static U8 u8SyncStatus		= 0;
    static U8 u8PreSyncStatus	= 0;

    u8SyncStatus = (MDrv_SC_IP1_GetInputSyncStatus()&SC_HDMI_CHECK_MASK);

    //SC_HDMI_DEBUG("u8Debounce = %x, u8SyncStatus = %x, u8PreSyncStatus = %x\n", u8Debounce, u8SyncStatus, u8PreSyncStatus);

    if (u8SyncStatus == u8PreSyncStatus)
    {
        u8Debounce++;
    }
    else
    {
        u8Debounce = 0;
    }

    if (u8Debounce > SC_HDMI_DEBOUNCE_THRESHOLD)
    {
        //lachesis_080925 initialize
        u8Debounce      = 0;
        u8SyncStatus    = 0;
        //u8PreSyncStatus	= 0;
        return SC_HDMI_SYNC_DETECT_STATE;
    }

    u8PreSyncStatus = u8SyncStatus;
    return SC_HDMI_DEBOUNCE_STATE;
}

SC_HDMI_STATE_e MDrv_HDMI_TimingChangeHandler(PSC_SOURCE_INFO_t psrc)
{
    // disable audio output
    MHal_HDMI_AudioOutput(FALSE);

    // init scaler
    MHal_SC_Reset(SC_RST_IP_F2);	//lachesis_090102 BH100 1080P no signal.
    MHal_SC_IP1_ResetSyncDetect();
    MHal_SC_SetRegeneratedDE(ENABLE);
    MHal_SC_IP1_SetCoastWin(0x0A, 0x0A);

    // init HDMI variables
    MDrv_HDMI_InitVariables(psrc);

    //lachesis_080925 init후 안정화 시간을 가질 수 있도록 SC_HDMI_DEBOUNCE_STATE로 보냄.
    return SC_HDMI_DEBOUNCE_STATE;//SC_HDMI_SYNC_DETECT_STATE;
}

SC_HDMI_STATE_e MDrv_HDMI_SyncDetectHandler(PSC_SOURCE_INFO_t psrc)
{
    // IP1 sync
    psrc->u8Input_SyncStatus = (MDrv_SC_IP1_GetInputSyncStatus()&SC_HDMI_CHECK_MASK);

    //SC_HDMI_DEBUG("MDrv_HDMI_SyncDetectHandler u16InputSyncStatus=0x%x \n", psrc->u8Input_SyncStatus);

    // DVI clock
    if (MHal_HDMI_DVISyncLoss(psrc->SrcType))
    {
        psrc->u8Input_SyncStatus |= SC_SYNCSTS_SYNC_LOSS;
        //SC_HDMI_DEBUG("MDrv_DVI_SyncLoss=0x%x \n", psrc->u8Input_SyncStatus);
    }

    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_SYNC_LOSS)
    {

        //FitchHsu 20090130 HDMI Vsync loss
        if (psrc->u8Input_SyncStatus & SC_SYNCSTS_VSYNC_LOSS_BIT)

        {
            //SC_HDMI_DEBUG("SC_SYNCSTS_VSYNC_LOSS_BIT= \n");
            MHal_SC_IP1_ResetSyncDetect();
        }
        return SC_HDMI_DEBOUNCE_STATE;	//20081014 jguy samsung BD-P1400 on/off시 신호못잡는 문제 수정
    }
    else
    {
        return SC_HDMI_CHK_MODE_STATE;
    }
}

SC_HDMI_STATE_e MDrv_HDMI_CheckModeHandler(PSC_SOURCE_INFO_t psrc)
{
    BOOL bIsHDMIMode;

    bIsHDMIMode = MHal_HDMI_GetModeRPT();
    psrc->bHDMIMode = bIsHDMIMode;

    g_HdmiPollingSts.u8AudioStbCnt = 0;
    g_HdmiPollingSts.u8AudioForceEnableCnt= 0;
    g_HdmiPollingSts.bMuteHDMIVideo = 0;//thchen 20080814
    g_u8AVIPacketCounter = 0;//thchen 20080814
    g_HdmiPacketInfo.HDMIST1 = 0;
    g_HdmiPacketInfo.bAVMuteStatus = 0;
    g_HdmiPacketInfo.bAudioNotPCM = 0;
    g_HdmiPacketInfo.bChecksumErrOrBCHParityErr = 0;
    g_HdmiPacketInfo.u8AspectRatio = HDMI_PAR_RSV;

    g_HdmiPacketInfo.u8PacketColorFormat = HDMI_COLOR_UNKNOWN;//packet parsing
    g_HdmiPollingSts.u8ColorFormat = HDMI_COLOR_UNKNOWN;      //현재 설정값.
    psrc->u8HDMIColorFormat = g_HdmiPollingSts.u8ColorFormat;

    u8NoPacketCnt= 0;        // LGE wlgnsl99 20090111

    if(!bIsHDMIMode)	//wlgnsl99 081109 DVI 일 때 이전 packet 정보로 인해 문제 발생. DVI일때는 무조건 Packet Reset함.
    {
        MHal_HDMI_ResetPacket(REST_AVMUTE|REST_Y_COLOR|REST_PIXEL_REPETITION|REST_FRAME_REPETITION|REST_GCP_PKT|REST_HDMI_STATUS);
    }

    MHal_SC_IP1_SetForceInputClkDiv(FALSE);
    MDrv_SC_IP1_SetFieldDetect(psrc);



    return SC_HDMI_SEARCH_MODE_STATE;
}

SC_HDMI_STATE_e MDrv_HDMI_SearchModeHandler(PSC_SOURCE_INFO_t psrc)
{
    U16 u16MinHfreq, u16MaxHfreq;
    U16 u16MinVfreq, u16MaxVfreq;
    U16 u16MinVtotal, u16MaxVtotal;

    U16 u16SearchHPeriod;
    U16 u16SearchVTotal;
    U16 u16SearchHDE;
    U16 u16SearchVDE;
    U16 u16SearchHDEStart;
    U16 u16SearchVDEStart;
	U16 u16SearchSyncStatus;
    BOOL bIsSearchHDMIMode;

    U8 bIsSearchTimingDetected = FALSE;

    bIsSearchHDMIMode = MHal_HDMI_GetModeRPT();
	u16SearchHDEStart = MHal_SC_IP1_GetHorizontalDEStart();
	u16SearchVDEStart = MHal_SC_IP1_GetVerticalDEStart();
	u16SearchSyncStatus = (MDrv_SC_IP1_GetInputSyncStatus()&SC_HDMI_CHECK_MASK);
	u16SearchHPeriod	= MHal_SC_IP1_GetHPeriod();
	u16SearchVTotal	= MHal_SC_IP1_GetVTotal();
	u16SearchHDE		= MHal_SC_IP1_GetHorizontalDE();
	u16SearchVDE		= MHal_SC_IP1_GetVerticalDE();


    // calculate H/V frequency
    psrc->u16Input_HFreq = MDrv_SC_CalculateHFreqX10(psrc->u16Input_HPeriod);
    psrc->u16Input_VFreq = MDrv_SC_CalculateVFreqX10(psrc->u16Input_HFreq, psrc->u16Input_VTotal);

    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
    {
        psrc->u16Input_VFreq *= 2;
    }

	if (u16SearchHDEStart != psrc->u16Input_HDE_Start)
	{
		psrc->u16Input_HDE_Start = u16SearchHDEStart;
		SC_HDMI_DEBUG("[SearchMode] Timing Change u16Input_HDE_Start=%d \n", u16SearchHDEStart);
		bIsSearchTimingDetected = TRUE;
	}
	if (u16SearchVDEStart != psrc->u16Input_VDE_Start)
	{
		psrc->u16Input_VDE_Start = u16SearchVDEStart;
		SC_HDMI_DEBUG("[SearchMode] Change u16Input_VDE_Start=%d \n", u16SearchVDEStart);
		bIsSearchTimingDetected = TRUE;
	}
	if (u16SearchSyncStatus != psrc->u8Input_SyncStatus)
	{
		psrc->u8Input_SyncStatus = u16SearchSyncStatus;
		SC_HDMI_DEBUG("[SearchMode] Change u16InputSyncStatus=0x%x \n", u16SearchSyncStatus);
		bIsSearchTimingDetected = TRUE;
	}
	if (u16SearchHPeriod != psrc->u16Input_HPeriod)
	{
		psrc->u16Input_HPeriod = u16SearchHPeriod;
		SC_HDMI_DEBUG("[SearchMode] Change MHal_SC_IP1_GetHPeriod=%d \n", u16SearchHPeriod);
		bIsSearchTimingDetected = TRUE;
	}
	if (u16SearchVTotal != psrc->u16Input_VTotal)
	{
		psrc->u16Input_VTotal = u16SearchVTotal;
		SC_HDMI_DEBUG("[SearchMode] Change MHal_SC_IP1_GetVTotal=%d \n", u16SearchVTotal);
		bIsSearchTimingDetected = TRUE;
	}
	if (u16SearchHDE != psrc->u16Input_HDE)
	{
		psrc->u16Input_HDE = u16SearchHDE;
		SC_HDMI_DEBUG("[SearchMode] Change MHal_SC_IP1_GetHorizontalDE=%d \n", u16SearchHDE);
		bIsSearchTimingDetected = TRUE;
	}
	if (u16SearchVDE != psrc->u16Input_VDE)
	{
		psrc->u16Input_VDE = u16SearchVDE;
		SC_HDMI_DEBUG("[SearchMode] Change MHal_SC_IP1_GetVerticalDE=%d \n", u16SearchVDE);
		bIsSearchTimingDetected = TRUE;
	}

	if(bIsSearchTimingDetected)
	{
		if(u8SearchModeCnt > 10 )
		{
			SC_HDMI_DEBUG("\n==================> We wait enough time!!! <===================\n");

			psrc->u16Input_HDE_Start = MHal_SC_IP1_GetHorizontalDEStart();
			psrc->u16Input_VDE_Start = MHal_SC_IP1_GetVerticalDEStart();
			psrc->u8Input_SyncStatus = (MDrv_SC_IP1_GetInputSyncStatus()&SC_HDMI_CHECK_MASK);
			psrc->u16Input_HPeriod	= MHal_SC_IP1_GetHPeriod();
			psrc->u16Input_VTotal	= MHal_SC_IP1_GetVTotal();
			psrc->u16Input_HDE		= MHal_SC_IP1_GetHorizontalDE();
			psrc->u16Input_VDE		= MHal_SC_IP1_GetVerticalDE();
		}
		else
		{
			SC_HDMI_DEBUG("\n========>[1] Search Mode UnStable[%d]!!! <=========\n",u8SearchModeCnt);
			u8SearchModeCnt++;
			bIsSearchTimingDetected = FALSE;

			return SC_HDMI_SEARCH_MODE_STATE;
		}

		u8SearchModeCnt= 0;
	}
	else
	{

		if(bIsSearchHDMIMode)
		{
			SC_HDMI_DEBUG("\n==================> Search Mode HDMI mode Stable Signal!!! <===================\n");

			psrc->u16Input_HDE_Start = MHal_SC_IP1_GetHorizontalDEStart();
			psrc->u16Input_VDE_Start = MHal_SC_IP1_GetVerticalDEStart();
			psrc->u8Input_SyncStatus = (MDrv_SC_IP1_GetInputSyncStatus()&SC_HDMI_CHECK_MASK);
			psrc->u16Input_HPeriod	= MHal_SC_IP1_GetHPeriod();
			psrc->u16Input_VTotal	= MHal_SC_IP1_GetVTotal();
			psrc->u16Input_HDE		= MHal_SC_IP1_GetHorizontalDE();
			psrc->u16Input_VDE		= MHal_SC_IP1_GetVerticalDE();

		}
		else
		{
			if(u8SearchModeCnt > 5 )
			{
				SC_HDMI_DEBUG("\n==================> Search Mode DVI mode Stable Signal!!! <===================\n");

				psrc->u16Input_HDE_Start = MHal_SC_IP1_GetHorizontalDEStart();
				psrc->u16Input_VDE_Start = MHal_SC_IP1_GetVerticalDEStart();
				psrc->u8Input_SyncStatus = (MDrv_SC_IP1_GetInputSyncStatus()&SC_HDMI_CHECK_MASK);
				psrc->u16Input_HPeriod	= MHal_SC_IP1_GetHPeriod();
				psrc->u16Input_VTotal	= MHal_SC_IP1_GetVTotal();
				psrc->u16Input_HDE		= MHal_SC_IP1_GetHorizontalDE();
				psrc->u16Input_VDE		= MHal_SC_IP1_GetVerticalDE();
			}
			else
			{
				SC_HDMI_DEBUG("\n========>[2] Search Mode UnStable[%d]!!! <=========\n",u8SearchModeCnt);
				u8SearchModeCnt++;
				bIsSearchTimingDetected = FALSE;

				return SC_HDMI_SEARCH_MODE_STATE;
			}

			u8SearchModeCnt= 0;
		}


	}

    if (psrc->bHDMIMode)
    {
        u16MinHfreq    = MIN_HFREQ_OF_HDMI;
        u16MaxHfreq    = MAX_HFREQ_OF_HDMI;
        u16MinVfreq    = MIN_VFREQ_OF_HDMI;
        u16MaxVfreq    = MAX_VFREQ_OF_HDMI;
        u16MinVtotal   = MIN_VTOTAL_OF_HDMI;
        u16MaxVtotal   = MAX_VTOTAL_OF_HDMI;

    }
    else
    {
        u16MinHfreq    = MIN_HFREQ_OF_DVI;
        u16MaxHfreq    = MAX_HFREQ_OF_DVI;
        u16MinVfreq    = MIN_VFREQ_OF_DVI;
        u16MaxVfreq    = MAX_VFREQ_OF_DVI;
        u16MinVtotal   = MIN_VTOTAL_OF_DVI;
        u16MaxVtotal   = MAX_VTOTAL_OF_DVI;
    }

    if ((psrc->u16Input_HFreq > u16MaxHfreq)   || (psrc->u16Input_HFreq < u16MinHfreq) ||
        (psrc->u16Input_VFreq > u16MaxVfreq)   || (psrc->u16Input_VFreq < u16MinVfreq) ||
        (psrc->u16Input_VTotal > u16MaxVtotal) || (psrc->u16Input_VTotal < u16MinVtotal) ||
        (psrc->u16Input_HDE > 1930) || (psrc->u16Input_VDE > 1210))
    {
        SC_HDMI_DEBUG("====> Unsupported mode 1\n");

        SC_HDMI_DEBUG("SyncStatus=0x%x \n", psrc->u8Input_SyncStatus);
        SC_HDMI_DEBUG("HPeriod %d\n", psrc->u16Input_HPeriod);
        SC_HDMI_DEBUG("VTotal %d\n", psrc->u16Input_VTotal);
        SC_HDMI_DEBUG("HDE %d\n", psrc->u16Input_HDE);
        SC_HDMI_DEBUG("VDE %d\n", psrc->u16Input_VDE);
        SC_HDMI_DEBUG("HDE_Start %d\n", psrc->u16Input_HDE_Start);
        SC_HDMI_DEBUG("VDE_Start %d\n", psrc->u16Input_VDE_Start);
        SC_HDMI_DEBUG("HFreq %d\n", psrc->u16Input_HFreq);
        SC_HDMI_DEBUG("VFreq %d\n", psrc->u16Input_VFreq);
        psrc->bIsSupportMode = FALSE;
        return SC_HDMI_STABLE_CHECK_STATE;
    }
    else
    {
        SC_HDMI_DEBUG("SC_HDMI_SUPPORTED_STATE \n");
        SC_HDMI_DEBUG("SyncStatus 0x%x\n", psrc->u8Input_SyncStatus);
        SC_HDMI_DEBUG("HPeriod %d\n", psrc->u16Input_HPeriod);
        SC_HDMI_DEBUG("VTotal %d\n", psrc->u16Input_VTotal);
        SC_HDMI_DEBUG("HDE %d\n", psrc->u16Input_HDE);
        SC_HDMI_DEBUG("VDE %d\n", psrc->u16Input_VDE);
        SC_HDMI_DEBUG("HDE_Start %d\n", psrc->u16Input_HDE_Start);
        SC_HDMI_DEBUG("VDE_Start %d\n", psrc->u16Input_VDE_Start);
        SC_HDMI_DEBUG("HFreq %d\n", psrc->u16Input_HFreq);
        SC_HDMI_DEBUG("VFreq %d\n", psrc->u16Input_VFreq);
        psrc->bIsSupportMode = TRUE;

        if (psrc->bHDMIMode)
        {
#ifndef HDMI_THREAD_SPEED_UP
			SC_HDMI_DEBUG("<============ Original Packet Handler ============>\n");
            return SC_HDMI_GET_PACKET_INFO_STATE;
#else
			SC_HDMI_DEBUG("<============ New Packet Handler ============>\n");
			return SC_HDMI_CONTROL_PACKET_STATE;
#endif
        }
        else
        {
            return SC_HDMI_STABLE_CHECK_STATE;
        }
    }
}

#ifndef HDMI_THREAD_SPEED_UP
SC_HDMI_STATE_e MDrv_HDMI_GetPacketInfoHandler(PSC_SOURCE_INFO_t psrc)
{
    //victor 20081117, ACP
    U8 u8ACPType;
    static U32 u32ACPTimestamp = 0;
    static BOOL isACPReset = FALSE;
    // Fitch 20090617 SAMSUNG DVD MUTE ISSUE
    static U32 u32AVMuteTimestamp = 0;
    static BOOL isHWAVMute = FALSE;

    g_HdmiPacketInfo.HDMIST1 = MHal_HDMI_GetHDMIST1();

    if (g_HdmiPacketInfo.bGC_PKT_Received)
    {
        // cc.chen - T.B.D. - Review flow "IsAVMUTE"
        g_HdmiPacketInfo.bAVMuteStatus = MHal_HDMI_IsAVMUTE();
        // Fitch 20090617 SAMSUNG DVD MUTE ISSUE
        if(g_HdmiPacketInfo.bAVMuteStatus)
        {
            isHWAVMute = TRUE;
            u32AVMuteTimestamp = jiffies;//jiffies is the current time (ms).
        }
    }
    else
    {

        u8NoPacketCnt++;		// LGE wlgnsl99 20090111
        //printk("u8NoPacketCnt==========>[%d]\n",u8NoPacketCnt);

        if(u8NoPacketCnt> 30 )
        {
            g_HdmiPacketInfo.bAVMuteStatus = 0;
            u8NoPacketCnt=0;
        }
        // Fitch 20090617 SAMSUNG DVD MUTE ISSUE
        if(jiffies - u32AVMuteTimestamp >= 600 && isHWAVMute) // in order to reset once if there is no more ACP received in a while
        {
            SC_HDMI_DEBUG("Clear AV Mute\n");
            isHWAVMute = FALSE;
            MHal_HDMI_ResetPacket(REST_AVMUTE);
        }
    }

    //victor 20081117, ACP
    // SimplayHD 8-7: Audio Output Compliance
    // -	If "ACP_TYPE > 2", DUT shall disable the external S/PDIF output.
    // -	Or no receiving ACP packet within 600 msec, DUT shall set ACP type to 0.
    //lachesis_081126 HDMI input block과의 충돌을 피하기 위해 UI단에서 block mask 처리하도록 수정 필요함.
    if (g_HdmiPacketInfo.bACP_PKT_Received)
    {
        u8ACPType = MHal_HDMI_GetACPHB1();
        //HDMI_DGB("SimplayHD u8ACPType =%d\n", u8ACPType);
        if(u8ACPType > 1)	//lachesis_081122 ACP2 이상 mute
        {
            MHal_AuSPDIF_HWEN(FALSE);
        }
        else
        {
            MHal_AuSPDIF_HWEN(TRUE);
        }
        u32ACPTimestamp = jiffies;//jiffies is the current time (ms).
        isACPReset = FALSE;
    }
    else
    {
        if(jiffies - u32ACPTimestamp >= 600 && !isACPReset) // in order to reset once if there is no more ACP received in a while
        {
            SC_HDMI_DEBUG("SimplayHD 8-7: ACP Handling\n");
            isACPReset = TRUE;
            MHal_AuSPDIF_HWEN(TRUE);
        }
    }

    if (g_HdmiPacketInfo.bAVI_PKT_Received)//thchen 20080719
    {
        g_HdmiPacketInfo.u8PacketColorFormat = MHal_HDMI_GetPacketColorFormat();
        g_HdmiPacketInfo.u8AspectRatio = MHal_HDMI_GetAspectRatio();
        //#ifdef CONFIG_Titania2
        // Get Colorimetry info
        g_HdmiPacketInfo.eColorimetry = MHal_HDMI_GetExtendColorimetryInfo();
        if(g_HdmiPacketInfo.eColorimetry == HDMI_COLORIMETRY_EXTEND)
        {
            g_HdmiPacketInfo.eExtColorimetry = MHal_HDMI_GetPacketExtendedColorimetry();
        }
        //#endif
    }

    //#ifdef CONFIG_Titania2
    // Get GM_Pkt
    if(g_HdmiPacketInfo.bGM_PKT_Received)
    {
        //if(MHal_HDMI_GetGMPktCurrentGamutSeq() > g_HdmiPacketInfo.xvYcc_Info.Current_Gamut_Seq)
        {
            MHal_HDMI_GetGMPktGBDInfo(g_HdmiPacketInfo.xvYCC_Info.GM_GBD);
        }
    }
    //#endif
    // audio channel
    g_HdmiPacketInfo.bAudioNotPCM = MHal_HDMI_IsAudioNotPCM();

    // packet error
    g_HdmiPacketInfo.bChecksumErrOrBCHParityErr = MHal_HDMI_IsChecksumOrBCHParityErr();
    g_HdmiPacketInfo.bAudioPacketErr = MHal_HDMI_IsAudioPacketErr();
    MHal_HDMI_ClearErr();

    //thchen 20080814
    if (g_HdmiPacketInfo.bAVMuteStatus/*g_HdmiPacketInfo.bGC_PKT_Received*/)
    {
        return SC_HDMI_DO_AVMUTE_STATE;
    }
    //lachesis_081120 simplay 인증 avi no packet 대응.

    else
    {
        return SC_HDMI_CHK_AUDIO_PKT_STATE;
    }

}

SC_HDMI_STATE_e MDrv_HDMI_DoAVMuteHandler(PSC_SOURCE_INFO_t psrc)
{
    if (g_HdmiPacketInfo.bAVMuteStatus)
    {
        SC_HDMI_DEBUG(" MDrv_HDMI_DoAVMuteHandler:: Set AUDIO_PATH_MAIN_SPEAKER mute register/n/n");
        if (!g_HdmiPollingSts.bMuteHDMIVideo)
        {
            //g_HdmiPollingSts.bMuteHDMIVideo = 1;
            g_HdmiPollingSts.u8LostHDMICounter = 0;
            //lachesis_081024 unmute시켜주는 부분이 없어 화면무/음성무 증상 발생 함. Block Mask 사용하여 처리할 때까지 적용보류.
            SC_HDMI_DEBUG(" we must enable avmute/n");
            //MHal_SC_OP2_SetBlackScreen(TRUE);
        }
        return SC_HDMI_STABLE_CHECK_STATE;
    }
    return SC_HDMI_CHK_AUDIO_PKT_STATE;
}

SC_HDMI_STATE_e MDrv_HDMI_ChkAudioPktHandler(PSC_SOURCE_INFO_t psrc)
{
    if ((g_HdmiPacketInfo.bAudioPacketErr) ||
        (!(g_HdmiPacketInfo.bASAMPLE_PKT_Received)) ||
        (!(g_HdmiPacketInfo.bACR_PKT_Received)))
    {
        SC_HDMI_DEBUG(" MDrv_HDMI_ChkAudioPktHandler bAudioPacketErr=%d, bASAMPLE_PKT_Received=%d, bACR_PKT_Received=%d\n", g_HdmiPacketInfo.bAudioPacketErr, g_HdmiPacketInfo.bASAMPLE_PKT_Received, g_HdmiPacketInfo.bACR_PKT_Received);
#if 1	// LGE for prevent pop niose on specific HDMI device by Jonghyuk, Lee 100107
		MDrv_MAD_ProcessSetMute(0x04, TRUE);
		MDrv_MAD_ProcessSetMute(0x05, TRUE);
#endif
        g_HdmiPollingSts.bMuteHDMIAudio = TRUE;
        g_HdmiPollingSts.u8AudioStbCnt = 0;
    }
#if 1 // Mstar Audio engineer suggest such as following condition by Jonghyuk, Lee LGE 100107
    else/* if (g_HdmiPacketInfo.bAUI_PKT_Received)*/
#endif
    {
        if ((g_HdmiPollingSts.u8AudioStbCnt > HDMI_AUDIO_STABLE_CNT) ||
            (g_HdmiPollingSts.u8AudioForceEnableCnt > HDMI_AUDIO_FORCE_ENABLE))
        {
#if 1	// LGE for prevent no sound after upper mute by Jonghyuk, Lee 100107
        	MDrv_MAD_ProcessSetMute(0x04, FALSE);
			MDrv_MAD_ProcessSetMute(0x05, FALSE);
#endif
            g_HdmiPollingSts.bMuteHDMIAudio = 0;
        }
        else
        {
            g_HdmiPollingSts.u8AudioStbCnt++;
            g_HdmiPollingSts.u8AudioForceEnableCnt++;
        }
    }
    //victor 20081215, HDMI audio
    MHal_HDMI_AuDownSample();
    return SC_HDMI_CHK_COLOR_FMT_STATE;
}

SC_HDMI_STATE_e MDrv_HDMI_ChkColorFmtHandler(PSC_SOURCE_INFO_t psrc)
{
#if 1 //thchen 20080719
    if ((g_HdmiPacketInfo.bAVI_PKT_Received) || (g_HdmiPacketInfo.bNULL_PKT_Received))
    {
        g_HdmiPollingSts.u8LostHDMICounter = 0;	//lachesis_090207 packet을 받았을 경우에는 lost cnt clear.
        //lachesis_081028 we can't use !with g_HdmiPacketInfo.bAVI_PKT_Received
        if(!(g_HdmiPacketInfo.bAVI_PKT_Received))
        {
            g_HdmiPacketInfo.u8PacketColorFormat = HDMI_COLOR_DEFAULT;
        }

        // if different color space, change CSC
        if (g_HdmiPollingSts.u8ColorFormat != g_HdmiPacketInfo.u8PacketColorFormat)
        {
            g_HdmiPollingSts.u8ColorFormat = g_HdmiPacketInfo.u8PacketColorFormat;
            SC_HDMI_DEBUG("MHal_SC_IP2_SetCSC =%d\n", g_HdmiPollingSts.u8ColorFormat);
            MHal_SC_IP2_SetCSC(g_HdmiPollingSts.u8ColorFormat == HDMI_COLOR_RGB ? TRUE : FALSE, FALSE); // 20090828 daniel.huang: add VIP CSC for PCRGB & DVI
            MHal_SC_IP2_SetCSCDither(g_HdmiPollingSts.u8ColorFormat == HDMI_COLOR_RGB ? TRUE : FALSE);
        }
        psrc->u8HDMIColorFormat = g_HdmiPollingSts.u8ColorFormat;
        return SC_HDMI_STABLE_CHECK_STATE;
    }
    else
    {
        return SC_HDMI_CHK_PKT_ERR_STATE;
    }
#else

#endif
}

SC_HDMI_STATE_e MDrv_HDMI_ChkPktErrHandler(PSC_SOURCE_INFO_t psrc)
{
    if (g_HdmiPacketInfo.bChecksumErrOrBCHParityErr)
    {
        SC_HDMI_DEBUG(" MDrv_HDMI_ChkPktErrHandler:: u8LostHDMICounter=%d\n", g_HdmiPollingSts.u8LostHDMICounter);
        if (g_HdmiPollingSts.u8LostHDMICounter < 255)
            g_HdmiPollingSts.u8LostHDMICounter ++;

        if (g_HdmiPollingSts.u8LostHDMICounter >= 3)
        {
            if (!g_HdmiPollingSts.bMuteHDMIVideo)
            {
                g_HdmiPollingSts.bMuteHDMIVideo = 1;

                MDrv_Scaler_GenerateBlackVideo(TRUE);//090904 drmyung
                SC_HDMI_DEBUG("u8LostHDMICounter BlackScreen(enable) \n");
            }

            if (g_HdmiPollingSts.u8LostHDMICounter == 10)
            {
                MHal_SC_Reset(SC_RST_IP_F2);
                MHal_HDMI_Reset();
                OS_Delayms(250);
            }
            else if (g_HdmiPollingSts.u8LostHDMICounter == 30)
            {
            #if 1
                MHal_HDMI_HPD_High(FALSE, psrc->SrcType);
                OS_Delayms(500);
                MHal_HDMI_HPD_High(TRUE,  psrc->SrcType);
            #else
                //MDrv_HDMI_Do_HPD(psrc->SrcType);//need to check with mstar
            #endif
            }
        }
        g_u8AVIPacketCounter = 0;//thchen 20080814
        return SC_HDMI_GET_PACKET_INFO_STATE;//thchen 20080813
    }
    else
    {
        g_HdmiPollingSts.u8LostHDMICounter = 0;
        //lachesis_081120 simplay 인증 avi no packet 대응.
        if (((g_HdmiPacketInfo.bAVI_PKT_Received)==0) && (g_u8AVIPacketCounter < 20)) //thchen &LGE jguy 20080902
        {
            SC_HDMI_DEBUG("no avi packet g_u8AVIPacketCounter = %d\n", g_u8AVIPacketCounter);
            g_u8AVIPacketCounter++;
            //OS_Delayms(20);

            if (g_u8AVIPacketCounter==10)
            {
                SC_HDMI_DEBUG("no avi packet MHal_HDMI_Reset = %d\n", g_u8AVIPacketCounter);
                g_u8AVIPacketCounter = 0;
                //MHal_HDMI_Reset();
                MHal_HDMI_ResetPacket(REST_HDMI_STATUS | REST_Y_COLOR);
                g_HdmiPacketInfo.u8PacketColorFormat = HDMI_COLOR_FORMAT_RGB;

                {
                    // if different color space, change CSC
                    if (g_HdmiPollingSts.u8ColorFormat != g_HdmiPacketInfo.u8PacketColorFormat)
                    {
                        g_HdmiPollingSts.u8ColorFormat = g_HdmiPacketInfo.u8PacketColorFormat;
                        SC_HDMI_DEBUG("bingo CSC =%d\n", g_HdmiPollingSts.u8ColorFormat);
                        MHal_SC_IP2_SetCSC(g_HdmiPollingSts.u8ColorFormat == HDMI_COLOR_RGB ? TRUE : FALSE, FALSE); // 20090828 daniel.huang: add VIP CSC for PCRGB & DVI
                        MHal_SC_IP2_SetCSCDither(g_HdmiPollingSts.u8ColorFormat == HDMI_COLOR_RGB ? TRUE : FALSE);
                    }
                    psrc->u8HDMIColorFormat = g_HdmiPollingSts.u8ColorFormat;
                }

            }
        }

        return SC_HDMI_STABLE_CHECK_STATE;//thchen 20080813
    }
}
#else



SC_HDMI_STATE_e MDrv_HDMI_ControlPacketHandler(PSC_SOURCE_INFO_t psrc)
{

	//victor 20081117, ACP
	static U32 u32ACPTimestamp = 0;
	static BOOL isACPReset = FALSE;

	g_HdmiPacketInfo.HDMIST1 = MHal_HDMI_GetHDMIST1();

	if (g_HdmiPacketInfo.bGC_PKT_Received)
	{
		// cc.chen - T.B.D. - Review flow "IsAVMUTE"
		g_HdmiPacketInfo.bAVMuteStatus = MHal_HDMI_IsAVMUTE();
	}
	else
	{
		u8NoPacketCnt++;		// LGE wlgnsl99 20090111
		//printk("u8NoPacketCnt==========>[%d]\n",u8NoPacketCnt);

		if(u8NoPacketCnt> 30 )
		{
			g_HdmiPacketInfo.bAVMuteStatus = 0;
			u8NoPacketCnt=0;
		}
	}

	//victor 20081117, ACP
	// SimplayHD 8-7: Audio Output Compliance
	// -	If "ACP_TYPE > 2", DUT shall disable the external S/PDIF output.
	// -	Or no receiving ACP packet within 600 msec, DUT shall set ACP type to 0.
	//lachesis_081126 HDMI input block과의 충돌을 피하기 위해 UI단에서 block mask 처리하도록 수정 필요함.
	if (g_HdmiPacketInfo.bACP_PKT_Received)
	{
		U8 u8ACPType = MHal_HDMI_GetACPHB1();
		//HDMI_DGB("SimplayHD u8ACPType =%d\n", u8ACPType);
		if(u8ACPType > 1)	//lachesis_081122 ACP2 이상 mute
		{
			MHal_AuSPDIF_HWEN(FALSE);
		}
		else
		{
			MHal_AuSPDIF_HWEN(TRUE);
		}
		u32ACPTimestamp = jiffies;//jiffies is the current time (ms).
		isACPReset = FALSE;
	}
	else
	{
		if(jiffies - u32ACPTimestamp >= 600 && !isACPReset) // in order to reset once if there is no more ACP received in a while
		{
			SC_HDMI_DEBUG("SimplayHD 8-7: ACP Handling\n");
			isACPReset = TRUE;
			MHal_AuSPDIF_HWEN(TRUE);
		}
	}

	if (g_HdmiPacketInfo.bAVI_PKT_Received)//thchen 20080719
	{
		g_HdmiPacketInfo.u8PacketColorFormat = MHal_HDMI_GetPacketColorFormat();
		g_HdmiPacketInfo.u8AspectRatio = MHal_HDMI_GetAspectRatio();
//  #ifdef CONFIG_Titania2	//<GP2_091110_KJB>GetHDMIColorDomain PQL_HDMI_XVYCC 받지 못하는 문제 개선안.(Mstar)
		// Get Colorimetry info
		g_HdmiPacketInfo.eColorimetry = MHal_HDMI_GetExtendColorimetryInfo();
		if(g_HdmiPacketInfo.eColorimetry == HDMI_COLORIMETRY_EXTEND)
		{
			g_HdmiPacketInfo.eExtColorimetry = MHal_HDMI_GetPacketExtendedColorimetry();
		}
    //#endif
	}

//#ifdef CONFIG_Titania2
	// Get GM_Pkt
	if(g_HdmiPacketInfo.bGM_PKT_Received)
	{
		//if(MHal_HDMI_GetGMPktCurrentGamutSeq() > g_HdmiPacketInfo.xvYcc_Info.Current_Gamut_Seq)
		{
			MHal_HDMI_GetGMPktGBDInfo(g_HdmiPacketInfo.xvYCC_Info.GM_GBD);
		}
	}
//#endif
	// audio channel
	g_HdmiPacketInfo.bAudioNotPCM = MHal_HDMI_IsAudioNotPCM();

	// packet error
	g_HdmiPacketInfo.bChecksumErrOrBCHParityErr = MHal_HDMI_IsChecksumOrBCHParityErr();
	g_HdmiPacketInfo.bAudioPacketErr = MHal_HDMI_IsAudioPacketErr();
	MHal_HDMI_ClearErr();

	//thchen 20080814
	if (g_HdmiPacketInfo.bAVMuteStatus/*g_HdmiPacketInfo.bGC_PKT_Received*/)
	{
		//return SC_HDMI_DO_AVMUTE_STATE;
		return MDrv_HDMI_DoAVMuteHandler(psrc);
	}
	//lachesis_081120 simplay 인증 avi no packet 대응.

	else
	{
		//return SC_HDMI_CHK_AUDIO_PKT_STATE;
		return MDrv_HDMI_ChkAudioPktHandler(psrc);
	}

}


SC_HDMI_STATE_e MDrv_HDMI_DoAVMuteHandler(PSC_SOURCE_INFO_t psrc)
{
//    if (g_HdmiPacketInfo.bAVMuteStatus)
//    {
		SC_HDMI_DEBUG(" MDrv_HDMI_DoAVMuteHandler\n");
        if (!g_HdmiPollingSts.bMuteHDMIVideo)
        {
            //g_HdmiPollingSts.bMuteHDMIVideo = 1;
            g_HdmiPollingSts.u8LostHDMICounter = 0;
			//lachesis_081024 unmute시켜주는 부분이 없어 화면무/음성무 증상 발생 함. Block Mask 사용하여 처리할 때까지 적용보류.
			SC_HDMI_DEBUG(" we must enable avmute\n");
            //MHal_SC_OP2_SetBlackScreen(TRUE);
        }
        return SC_HDMI_STABLE_CHECK_STATE;
//    }
    //return SC_HDMI_CHK_AUDIO_PKT_STATE;
}

SC_HDMI_STATE_e MDrv_HDMI_ChkAudioPktHandler(PSC_SOURCE_INFO_t psrc)
{
    if ((g_HdmiPacketInfo.bAudioPacketErr) ||
        (!(g_HdmiPacketInfo.bASAMPLE_PKT_Received)) ||
        (!(g_HdmiPacketInfo.bACR_PKT_Received)))
    {
		SC_HDMI_DEBUG(" MDrv_HDMI_ChkAudioPktHandler bAudioPacketErr=%d, bASAMPLE_PKT_Received=%d, bACR_PKT_Received=%d\n", g_HdmiPacketInfo.bAudioPacketErr, g_HdmiPacketInfo.bASAMPLE_PKT_Received, g_HdmiPacketInfo.bACR_PKT_Received);
#if 1	// LGE for prevent pop niose on specific HDMI device by Jonghyuk, Lee 100107
		MDrv_MAD_ProcessSetMute(0x04, TRUE);
		MDrv_MAD_ProcessSetMute(0x05, TRUE);
#endif
        g_HdmiPollingSts.bMuteHDMIAudio = TRUE;
        g_HdmiPollingSts.u8AudioStbCnt = 0;
    }	// LGE  Mstar audio Engineer suggest such as following condition by Jonghyuk, Lee 100107
    else/* if (g_HdmiPacketInfo.bAUI_PKT_Received)*/
    {
        if ((g_HdmiPollingSts.u8AudioStbCnt > HDMI_AUDIO_STABLE_CNT) ||
            (g_HdmiPollingSts.u8AudioForceEnableCnt > HDMI_AUDIO_FORCE_ENABLE))
        {
#if 1	// LGE for prevent pop niose on specific HDMI device by Jonghyuk, Lee 100107
        	MDrv_MAD_ProcessSetMute(0x04, FALSE);
			MDrv_MAD_ProcessSetMute(0x05, FALSE);
#endif
            g_HdmiPollingSts.bMuteHDMIAudio = 0;
        }
        else
        {
            g_HdmiPollingSts.u8AudioStbCnt++;
            g_HdmiPollingSts.u8AudioForceEnableCnt++;
        }
    }
    //victor 20081215, HDMI audio
    MHal_HDMI_AuDownSample();

    //return SC_HDMI_CHK_COLOR_FMT_STATE;
    return MDrv_HDMI_ChkColorFmtHandler(psrc);


}

SC_HDMI_STATE_e MDrv_HDMI_ChkColorFmtHandler(PSC_SOURCE_INFO_t psrc)
{
    if ((g_HdmiPacketInfo.bAVI_PKT_Received) || (g_HdmiPacketInfo.bNULL_PKT_Received))
    {
		g_HdmiPollingSts.u8LostHDMICounter = 0;	//lachesis_090207 packet을 받았을 경우에는 lost cnt clear.
		//lachesis_081028 we can't use !with g_HdmiPacketInfo.bAVI_PKT_Received
		if(!(g_HdmiPacketInfo.bAVI_PKT_Received))
		{
			g_HdmiPacketInfo.u8PacketColorFormat = HDMI_COLOR_DEFAULT;
		}

        // if different color space, change CSC
        if (g_HdmiPollingSts.u8ColorFormat != g_HdmiPacketInfo.u8PacketColorFormat)
        {
            g_HdmiPollingSts.u8ColorFormat = g_HdmiPacketInfo.u8PacketColorFormat;
            SC_HDMI_DEBUG("MHal_SC_IP2_SetCSC =%d\n", g_HdmiPollingSts.u8ColorFormat);
            MHal_SC_IP2_SetCSC(g_HdmiPollingSts.u8ColorFormat == HDMI_COLOR_RGB ? TRUE : FALSE, FALSE); // 20090828 daniel.huang: add VIP CSC for PCRGB & DVI
            MHal_SC_IP2_SetCSCDither(g_HdmiPollingSts.u8ColorFormat == HDMI_COLOR_RGB ? TRUE : FALSE);
        }
        psrc->u8HDMIColorFormat = g_HdmiPollingSts.u8ColorFormat;
        //g_SrcInfo.u8HDMIColorFormat = g_HdmiPollingSts.u8ColorFormat; // LGE drmyung 081029

        return SC_HDMI_STABLE_CHECK_STATE;
    }
    else
    {
        //return SC_HDMI_CHK_PKT_ERR_STATE;
		return MDrv_HDMI_ChkPktErrHandler(psrc);
    }

}

SC_HDMI_STATE_e MDrv_HDMI_ChkPktErrHandler(PSC_SOURCE_INFO_t psrc)
{
    if (g_HdmiPacketInfo.bChecksumErrOrBCHParityErr)
    {
		SC_HDMI_DEBUG(" MDrv_HDMI_ChkPktErrHandler:: u8LostHDMICounter=%d\n", g_HdmiPollingSts.u8LostHDMICounter);
        if (g_HdmiPollingSts.u8LostHDMICounter < 255)
            g_HdmiPollingSts.u8LostHDMICounter ++;

        if (g_HdmiPollingSts.u8LostHDMICounter >= 3)
        {
            if (!g_HdmiPollingSts.bMuteHDMIVideo)
            {
                g_HdmiPollingSts.bMuteHDMIVideo = 1;

                MDrv_Scaler_GenerateBlackVideo(TRUE);//090904 drmyung
				SC_HDMI_DEBUG("u8LostHDMICounter BlackScreen(enable) \n");
            }

            if (g_HdmiPollingSts.u8LostHDMICounter == 10)
            {
                MHal_SC_Reset(SC_RST_IP_F2);
                MHal_HDMI_Reset();
                OS_Delayms(250);
            }
            else if (g_HdmiPollingSts.u8LostHDMICounter == 30)
            {
			#if 0
				MHal_HDMI_HPD_High(FALSE, psrc->SrcType);
				OS_Delayms(500);
				MHal_HDMI_HPD_High(TRUE,  psrc->SrcType);
			#else
				//MDrv_HDMI_Do_HPD(psrc->SrcType);//need to check with mstar
			#endif
            }
        }
		g_u8AVIPacketCounter = 0;//thchen 20080814

    	return SC_HDMI_CONTROL_PACKET_STATE;//thchen 20080813

    }
    else
	{
	   g_HdmiPollingSts.u8LostHDMICounter = 0;
		//lachesis_081120 simplay 인증 avi no packet 대응.
	   if (((g_HdmiPacketInfo.bAVI_PKT_Received)==0) && (g_u8AVIPacketCounter < 20)) //thchen &LGE jguy 20080902
		{
		   SC_HDMI_DEBUG("no avi packet g_u8AVIPacketCounter = %d\n", g_u8AVIPacketCounter);
		   g_u8AVIPacketCounter++;
		   //OS_Delayms(20);

		   if (g_u8AVIPacketCounter==10)
		   {
				SC_HDMI_DEBUG("no avi packet MHal_HDMI_Reset = %d\n", g_u8AVIPacketCounter);
				g_u8AVIPacketCounter = 0;
				//MHal_HDMI_Reset();
				MHal_HDMI_ResetPacket(REST_HDMI_STATUS | REST_Y_COLOR);
				g_HdmiPacketInfo.u8PacketColorFormat = HDMI_COLOR_FORMAT_RGB;

				{
					// if different color space, change CSC
					if (g_HdmiPollingSts.u8ColorFormat != g_HdmiPacketInfo.u8PacketColorFormat)
					{
						g_HdmiPollingSts.u8ColorFormat = g_HdmiPacketInfo.u8PacketColorFormat;
						SC_HDMI_DEBUG("bingo CSC =%d\n", g_HdmiPollingSts.u8ColorFormat);
						MHal_SC_IP2_SetCSC(g_HdmiPollingSts.u8ColorFormat == HDMI_COLOR_RGB ? TRUE : FALSE, FALSE); // 20090828 daniel.huang: add VIP CSC for PCRGB & DVI
						MHal_SC_IP2_SetCSCDither(g_HdmiPollingSts.u8ColorFormat == HDMI_COLOR_RGB ? TRUE : FALSE);
					}
					psrc->u8HDMIColorFormat = g_HdmiPollingSts.u8ColorFormat;
					//g_SrcInfo.u8HDMIColorFormat = g_HdmiPollingSts.u8ColorFormat; // LGE drmyung 081029
				}

		   }
		}

	   return SC_HDMI_STABLE_CHECK_STATE;//thchen 20080813
	}

}


#endif

void MDrv_HDMI_UpdateTimingInfo(PSC_SOURCE_INFO_t psrc) // LGE drmyung
{
    // calculate H/V frequency
    psrc->u16Input_HFreq = MDrv_SC_CalculateHFreqX10(psrc->u16Input_HPeriod);
    psrc->u16Input_VFreq = MDrv_SC_CalculateVFreqX10(psrc->u16Input_HFreq, psrc->u16Input_VTotal);

    psrc->u16Input_HDMI_HPeriod  = psrc->u16Input_HPeriod;
    psrc->u16Input_HDMI_VTotal   = psrc->u16Input_VTotal;
    psrc->u16Input_HDMI_HDE      = psrc->u16Input_HDE;
    psrc->u16Input_HDMI_VDE      = psrc->u16Input_VDE;

    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
    {
        psrc->u16Input_VFreq *= 2;
        psrc->u16Input_HDMI_VDE  += 1;	// drmyung LGE 080519 :
    }

    psrc->u16Input_HDMI_HDE_Start = psrc->u16Input_HDE_Start;
    psrc->u16Input_HDMI_VDE_Start = psrc->u16Input_VDE_Start;

    // calculate H/V frequency
    psrc->u16Input_HDMI_HFreq = psrc->u16Input_HFreq;
	if(g_HdmiPacketInfo.bChecksumErrOrBCHParityErr)
	{
		psrc->u16Input_HDMI_VFreq = 0;
	}
	else
	{
		psrc->u16Input_HDMI_VFreq = psrc->u16Input_VFreq;
	}

	// LGE drmyung 081104 :
    /* 초기 HDMI로 입력 전환 시 HDMI Thread가 stable->unstable->stable 발생 되는 경우 있슴.
       이 때 상위 Periodic이 끼어 들지 못해서 format 변경을 detection 못함.
       결론적으로 Capture Window 설정을 못해주기 때문에 HDMI의 경우 여기서 Capture 설정되도록 함. */
    psrc->u16H_CapStart = psrc->u16Input_HDE_Start;
    psrc->u16H_CapSize  = psrc->u16Input_HDMI_HDE;
    psrc->u16H_CapStart_Backup = psrc->u16Input_HDE_Start;
    psrc->u16H_CapSize_Backup = psrc->u16Input_HDMI_HDE;
    psrc->u16V_CapStart = psrc->u16Input_VDE_Start;
    psrc->u16V_CapSize  = psrc->u16Input_HDMI_VDE;

    SC_ADD_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_CAP_WIN);
    SC_SET_SRC_STATE(psrc, SC_SRC_STATE_CAPTURE_WIN);

    //lachesis_080925 QM과 동기되지 않아 interlace에서 progresive로 설정되어 화면 깨지는 문제. g_SrcInfo 관련 변수들 재확인 필요.
    psrc->u8Input_HDMI_SyncStatus = psrc->u8Input_SyncStatus;
    psrc->u8HDMIColorFormat = g_HdmiPollingSts.u8ColorFormat;
}

SC_HDMI_STATE_e MDrv_HDMI_StableCheckHandler(PSC_SOURCE_INFO_t psrc)
{
    extern BOOL gIsVideoMute;
    U16 u16InputSyncStatus;
    U16 u16InputHPeriod;
    U16 u16InputVTotal;
    U16 u16InputHDE;
    U16 u16InputVDE;
    U16 u16InputHDEStart;
    U16 u16InputVDEStart;
   // U8 u8uGoodSyncCheckCnt;

    // LGE wlgnsl99 081110 Bad sync check는 이전 방법으로 원복, Good sync가 들어올때 threshold를 두어서 threshold 이상일때 Timing info update하도록 수정.===> START
    U8 bIsInvalidTimingDetected = FALSE;
    BOOL bIsHDMIMode;

    bIsHDMIMode = MHal_HDMI_GetModeRPT();
    u16InputSyncStatus = (MDrv_SC_IP1_GetInputSyncStatus()&SC_HDMI_CHECK_MASK);
    u16InputHPeriod = MHal_SC_IP1_GetHPeriod();
    u16InputVTotal = MHal_SC_IP1_GetVTotal();
    u16InputHDE = MHal_SC_IP1_GetHorizontalDE();
    u16InputVDE = MHal_SC_IP1_GetVerticalDE();

    u16InputHDEStart = MHal_SC_IP1_GetHorizontalDEStart();
    u16InputVDEStart = MHal_SC_IP1_GetVerticalDEStart();

    if (MDrv_SC_SubtractABS(u16InputHDEStart, psrc->u16Input_HDE_Start) > HDMI_HDE_TORLANCE)
    {
        //psrc->u16Input_HDE_Start = u16InputHDEStart;
        SC_HDMI_DEBUG("Go to Timing Change u16Input_HDE_Start=%d \n", u16InputHDEStart);
        //bIsInvalidTimingDetected = TRUE;
    }

    if (MDrv_SC_SubtractABS(u16InputVDEStart, psrc->u16Input_VDE_Start) > HDMI_HDE_TORLANCE)
    {
        //psrc->u16Input_VDE_Start = u16InputVDEStart;
        SC_HDMI_DEBUG("Go to Timing Change u16Input_VDE_Start=%d \n", u16InputVDEStart);
        //bIsInvalidTimingDetected = TRUE;
    }

    if(psrc->bHDMIMode != bIsHDMIMode)
    {
        psrc->bHDMIMode = bIsHDMIMode;
        SC_HDMI_DEBUG("Go to Timing Change bIsHDMIMode=0x%x \n", bIsHDMIMode);
        bIsInvalidTimingDetected = TRUE;
    }
    else if (u16InputSyncStatus != psrc->u8Input_SyncStatus)
    {
        psrc->u8Input_SyncStatus = u16InputSyncStatus;
        SC_HDMI_DEBUG("Go to Timing Change u16InputSyncStatus=0x%x \n", psrc->u8Input_SyncStatus);
        bIsInvalidTimingDetected = TRUE;
    }
    else if (MDrv_SC_SubtractABS(u16InputHPeriod, psrc->u16Input_HPeriod) > HDMI_HPERIOD_TORLANCE)
    {
        psrc->u16Input_HPeriod = u16InputHPeriod;
        SC_HDMI_DEBUG("Go to Timing Change MHal_SC_IP1_GetHPeriod=%d \n", u16InputHPeriod);
        bIsInvalidTimingDetected = TRUE;
    }
    else if (MDrv_SC_SubtractABS(u16InputVTotal, psrc->u16Input_VTotal) > HDMI_VTOTAL_TORLANCE)
    {
        psrc->u16Input_VTotal = u16InputVTotal;
        SC_HDMI_DEBUG("Go to Timing Change MHal_SC_IP1_GetVTotal=%d \n", u16InputVTotal);
        bIsInvalidTimingDetected = TRUE;
    }
    else if (MDrv_SC_SubtractABS(u16InputHDE, psrc->u16Input_HDE) > HDMI_HDE_TORLANCE)
    {
        psrc->u16Input_HDE = u16InputHDE;
        SC_HDMI_DEBUG("Go to Timing Change MHal_SC_IP1_GetHorizontalDE=%d \n", u16InputHDE);
        bIsInvalidTimingDetected = TRUE;
    }
    else if (MDrv_SC_SubtractABS(u16InputVDE, psrc->u16Input_VDE) > HDMI_VDE_TORLANCE)
    {
        psrc->u16Input_VDE = u16InputVDE;
        SC_HDMI_DEBUG("Go to Timing Change MHal_SC_IP1_GetVerticalDE=%d \n", u16InputVDE);
        bIsInvalidTimingDetected = TRUE;
    }

    if(bIsInvalidTimingDetected)
    {
        MDrv_Scaler_GenerateBlackVideo(TRUE);		// LGE wlgnsl99 081106 Timing Change Handler 에서 옮김
        SC_HDMI_DEBUG("\nGo to Timing Change!!! \n");
        bIsInvalidTimingDetected = FALSE;
        //u8Goodsycnthreshold = 0;

        return SC_HDMI_TIMING_CHANGE_STATE;

    }
    else
    {   ////lachesis_090102 BH100 1080P no signal. MDrv_HDMI_StableCheckHandler에서는 sync가 있는 상태여야 하나, no signal 상태가 발생.
        if(u16InputSyncStatus & SC_SYNCSTS_SYNC_LOSS)
        {
            u8ScResetCnt++;
            SC_HDMI_DEBUG("u8ScResetCnt=%d \n", u8ScResetCnt);
        }

        if(u8ScResetCnt > 5)
        {
            u8ScResetCnt = 0;
            SC_HDMI_DEBUG("bingo!!!! SC_HDMI_TIMING_CHANGE_STATE=%d \n", u8ScResetCnt);
            return SC_HDMI_TIMING_CHANGE_STATE;
        }
    }


#if 0

    //lachesis_081203
    if(bIsHDMIMode)
        u8uGoodSyncCheckCnt = 2;
    else
        u8uGoodSyncCheckCnt = 3;

    u8Goodsycnthreshold++;		// LGE wlgnsl99 081110 ====> END

    if(u8Goodsycnthreshold > u8uGoodSyncCheckCnt)	// LGE wlgnsl99 081113 과도로 인해서 1 -> 3으로 수정. 추후 다시 tuning.
    {
        if(!g_HdmiPacketInfo.bAVMuteStatus)
        {
            //lachesis_090207 packet error 발생 후, mute가 풀리지 않아 영상무 발생 함.
            if(g_HdmiPollingSts.bMuteHDMIVideo && !gIsVideoMute)
            {
                g_HdmiPollingSts.bMuteHDMIVideo = FALSE;
                MDrv_Scaler_GenerateBlackVideo(FALSE);//090904 drmyung
                SC_HDMI_DEBUG("clear mute from packet error\n");
            }
            //SC_HDMI_DEBUG("\nUPDATE TIMING INFO!!! \n");
            MDrv_HDMI_UpdateTimingInfo(psrc); // LGE drmyung
        }
        else
        {
            SC_HDMI_DEBUG("\n AVMUTE can't update timing info========= !!! \n");
        }
    }

#else

	if(!g_HdmiPacketInfo.bAVMuteStatus)
	{
		//lachesis_090207 packet error 발생 후, mute가 풀리지 않아 영상무 발생 함.
		if(g_HdmiPollingSts.bMuteHDMIVideo && !gIsVideoMute)
		{
			g_HdmiPollingSts.bMuteHDMIVideo = FALSE;
			MDrv_Scaler_GenerateBlackVideo(FALSE);//090904 drmyung
			SC_HDMI_DEBUG("clear mute from packet error\n");
		}
		//SC_HDMI_DEBUG("\nUPDATE TIMING INFO!!! \n");
		MDrv_HDMI_UpdateTimingInfo(psrc); // LGE drmyung
	}
	else
	{
		SC_HDMI_DEBUG("\n AVMUTE can't update timing info========= !!! \n");
	}

#endif

    if(psrc->bHDMIMode && psrc->bIsSupportMode)
    {
#ifndef HDMI_THREAD_SPEED_UP
        return SC_HDMI_GET_PACKET_INFO_STATE;
#else
		return SC_HDMI_CONTROL_PACKET_STATE;
#endif
    }
    else
    {
        return SC_HDMI_STABLE_CHECK_STATE;
    }

}

#define DIVIDE_HZ	25

int MDrv_HDMI_Thread(void *data)
{
    SC_HDMI_STATE_e nextState;
    PSC_SOURCE_INFO_t psrc = (PSC_SOURCE_INFO_t)data;
    PSC_THREAD_DATA_t pThreadData = &(psrc->ThreadData);
    U32 u32Timeout;
    S32 s32Ret;

#ifndef	SC_USE_ONLY_ONE_THREAD
    init_waitqueue_head(&pThreadData->thread_wq);
#endif

    pThreadData->u8ThreadState = SC_HDMI_INIT_STATE;
    pThreadData->u16Ctrl = SC_THREAD_CTRL_NONE;
#ifdef FAST_THREAD_CONTROL
	pThreadData->u16Ctrl |= SC_THREAD_CTRL_FAST_STATE_CHANGE;
#endif

    nextState = pThreadData->u8ThreadState;
    u32Timeout = HZ/DIVIDE_HZ;

    while (1)
    {
        s32Ret = wait_event_interruptible_timeout(pThreadData->thread_wq, pThreadData->u16Ctrl != 0, u32Timeout);
        if (s32Ret == -ERESTARTSYS)
        {
            break;
        }

        if (pThreadData->u16Ctrl != 0)
        {
            // fast state change
            if (pThreadData->u16Ctrl & SC_THREAD_CTRL_FAST_STATE_CHANGE)
            {
                pThreadData->u16Ctrl &= ~SC_THREAD_CTRL_FAST_STATE_CHANGE;
            }

            // terminate
            if (pThreadData->u16Ctrl & SC_THREAD_CTRL_TERMINATE)
            {
                break;
            }
        }

        switch (pThreadData->u8ThreadState)
        {
        case SC_HDMI_INIT_STATE:
#ifndef HDMI_THREAD_SPEED_UP
			u32Timeout = HZ/20;//DIVIDE_HZ;//5;
#else
            u32Timeout = HZ/DIVIDE_HZ;
#endif
            HDMI_DGB("SC_HDMI_INIT_STATE\n");
            MDrv_TimingDataInit(psrc);
            nextState = MDrv_HDMI_InitHandler(psrc);
            break;

        case SC_HDMI_DEBOUNCE_STATE:
#ifndef HDMI_THREAD_SPEED_UP
			u32Timeout = 150;//250; //wlgnsl99_090111 sony 과도 문제 개선	//120; /lachesis_081215
#else
			u32Timeout = HZ/DIVIDE_HZ;
#endif
            HDMI_DGB("SC_HDMI_DEBOUNCE_STATE\n");
            nextState  = MDrv_HDMI_Debounce();
			u8SearchModeCnt = 0;
            u8ScResetCnt = 0;
            break;

        case SC_HDMI_TIMING_CHANGE_STATE:
            u32Timeout = HZ/DIVIDE_HZ;
            HDMI_DGB("SC_HDMI_TIMING_CHANGE_STATE\n");
            //MDrv_SC_WakeupEvent(psrc, SC_EVENT_TIMING_CHANGE);
            MDrv_TimingDataInit(psrc);
            nextState = MDrv_HDMI_TimingChangeHandler(psrc);
            SC_HDMI_DEBUG("=============> TIMING_CHANGE %x\n", psrc->u8Input_SyncStatus);
            break;

        case SC_HDMI_SYNC_DETECT_STATE:
            u32Timeout = HZ/DIVIDE_HZ;
            HDMI_DGB("SC_HDMI_SYNC_DETECT_STATE\n");
            nextState = MDrv_HDMI_SyncDetectHandler(psrc);
            break;

        case SC_HDMI_CHK_MODE_STATE:
            u32Timeout = HZ/DIVIDE_HZ;
            HDMI_DGB("SC_HDMI_CHK_MODE_STATE\n");
            nextState = MDrv_HDMI_CheckModeHandler(psrc);
            SC_HDMI_DEBUG("=============> CHECK_MODE %x\n", psrc->u8Input_SyncStatus);
            break;

        case SC_HDMI_SEARCH_MODE_STATE:
            u32Timeout = 150; //HZ/DIVIDE_HZ;	//joonbum.kim 100512 RH389H 입력 절환시 과도 개선.
            HDMI_DGB("SC_HDMI_SEARCH_MODE_STATE\n");
            nextState = MDrv_HDMI_SearchModeHandler(psrc);
            break;

#ifndef HDMI_THREAD_SPEED_UP
        case SC_HDMI_GET_PACKET_INFO_STATE:
            u32Timeout = HZ/DIVIDE_HZ;
            nextState = MDrv_HDMI_GetPacketInfoHandler(psrc);
            break;

        case SC_HDMI_DO_AVMUTE_STATE:
            u32Timeout = HZ/DIVIDE_HZ;
            HDMI_DGB("SC_HDMI_DO_AVMUTE_STATE\n");
            nextState = MDrv_HDMI_DoAVMuteHandler(psrc);
            break;

        case SC_HDMI_CHK_AUDIO_PKT_STATE:
            u32Timeout = HZ/DIVIDE_HZ;
            nextState = MDrv_HDMI_ChkAudioPktHandler(psrc);
            break;

        case SC_HDMI_CHK_COLOR_FMT_STATE:
            u32Timeout = HZ/DIVIDE_HZ;
            nextState = MDrv_HDMI_ChkColorFmtHandler(psrc);
            break;

        case SC_HDMI_CHK_PKT_ERR_STATE:
            u32Timeout = HZ/DIVIDE_HZ;
            HDMI_DGB("SC_HDMI_CHK_PKT_ERR_STATE\n");
            nextState = MDrv_HDMI_ChkPktErrHandler(psrc);
            break;
#else
		case SC_HDMI_CONTROL_PACKET_STATE:
			u32Timeout = HZ/DIVIDE_HZ;
			nextState = MDrv_HDMI_ControlPacketHandler(psrc);
			break;
#endif

        case SC_HDMI_STABLE_CHECK_STATE:
#ifndef HDMI_THREAD_SPEED_UP
			u32Timeout = HZ/100;
#else
			u32Timeout = HZ/DIVIDE_HZ;//HZ/100;
#endif
            nextState = MDrv_HDMI_StableCheckHandler(psrc);
            break;

        default:
            break;
        }

        if (nextState != pThreadData->u8ThreadState)
        {
            pThreadData->u8ThreadState = nextState;
            //SC_HDMI_DEBUG("[HDMI] State change to %d\n", nextState);
        }
    }

    SC_SET_SRC_STATE(psrc, SC_SRC_STATE_HDMI_STOP);

    return 0;
}

void MDrv_HDMI_Start(PSC_SOURCE_INFO_t psrc)
{
#ifdef SC_USE_ONLY_ONE_THREAD

    extern long scaler_pid;

    if( scaler_pid < 0 )
    {
        scaler_pid = kernel_thread((int (*)(void*))MDrv_SC_Thread, (void*)psrc, CLONE_KERNEL );
    }

    psrc->u8ThreadMode = 2;
#ifdef FAST_THREAD_CONTROL
	psrc->u8ThreadCtrl = 1;
#endif
    MDrv_SC_WakeupEvent(psrc, SC_EVENT_HDMI_CHANGE);
    // printk("-Create(%d)\n", scaler_pid );
#else
    static long hdmi_pid;
    if( hdmi_pid < 0 ) {
    kernel_thread((int (*)(void*))MDrv_HDMI_Thread, (void*)psrc, CLONE_KERNEL);
	}
#endif /* #ifdef	SC_USE_ONLY_ONE_THREAD */
}

void MDrv_HDMI_Stop(PSC_SOURCE_INFO_t psrc)
{

#if 1
/*   008,10,07:	외부입력 전환 Aging Test시 System pending되는 문제(dreamer@lge.com)

	MDrv_HDMI_Stop()/MDrv_SC_PCMode_Stop() 함수에서 Return 안되서 생기는 문제
    > 특정시간 동안만 확인하는 방식으로 수정함.
*/
    U32 index;
	#define _MAX_CHECK_TERM         1000    // 1 sec
	#define _CHECK_SLEEP_TERM       10      // 10 ms

    //lachesis_081023 입력 전환 시 이전 정보를 읽어가서 mute 풀리는 문제.
    psrc->u16Input_HDMI_HPeriod  = 0;
    psrc->u16Input_HDMI_VTotal   = 0;
    psrc->u16Input_HDMI_HDE      = 0;
    psrc->u16Input_HDMI_VDE      = 0;
    psrc->u16Input_HDMI_HDE_Start = 0;
    psrc->u16Input_HDMI_VDE_Start = 0;
    psrc->u16Input_HDMI_HFreq = 0;
    psrc->u16Input_HDMI_VFreq = 0;
    psrc->u8Input_HDMI_SyncStatus = 0;

#ifdef  SC_USE_ONLY_ONE_THREAD
    psrc->u8ThreadMode = 0;
#ifdef FAST_THREAD_CONTROL
	psrc->u8ThreadCtrl = 0;
#endif
#endif  /* #ifdef	SC_USE_ONLY_ONE_THREAD */

    psrc->ThreadData.u16Ctrl = SC_THREAD_CTRL_TERMINATE;
    for( index = 0; index  < (_MAX_CHECK_TERM / _CHECK_SLEEP_TERM); index ++ )
    {
        if( SC_GET_SRC_STATE(psrc) & SC_SRC_STATE_HDMI_STOP )
        {
            break;
        }

        msleep( _CHECK_SLEEP_TERM );
    }

    if( index >= (_MAX_CHECK_TERM / _CHECK_SLEEP_TERM))
    {
        SC_HDMI_DEBUG("ERR: MDrv_HDMI_Stop\n");
    }
#else
    psrc->ThreadData.u16Ctrl = SC_THREAD_CTRL_TERMINATE;
    while (!(SC_GET_SRC_STATE(psrc) & SC_SRC_STATE_HDMI_STOP))
    {
        schedule();
    }
#endif

    SC_CLR_SRC_STATE(psrc, SC_SRC_STATE_HDMI_STOP);

}

//--------------------------------------------------------------------------------------------------
// Init
//--------------------------------------------------------------------------------------------------
void MDrv_HDMI_Init(void)
{
    // init HDMI
    MHal_HDMI_Init();

    // init production key
    MDrv_HDCP_InitProductionKey();

    // xvYCC init
    g_HdmiPacketInfo.xvYCC_Info.Current_Gamut_Seq = 0;

}

//------------------------------------------------------------------------------
//  DVI clock control
//------------------------------------------------------------------------------
// FitchHsu 20081202 DVI clock controll error
void MDrv_HDMI_DVIClock_Enable(BOOL bEnable, SC_INPUT_SOURCE_e enInput)
{
    SC_HDMI_DEBUG("========enInputPortType==========%d\n", enInput);
    MHal_HDMI_DVIClock_Enable(bEnable, enInput);
}


//------------------------------------------------------------------------------
//  HDCP
//------------------------------------------------------------------------------
//lachesis_090721 HPD control by app
void MDrv_HDMI_SetHPD(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_INPUT_SOURCE_e eSrcType = INPUT_SOURCE_NONE;
    SC_HDMI_HPD_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_HDMI_HPD_t)))
    {
        return;
    }

    //printk("MDrv_HDMI_SetHPD param.u8HDMIPort=%d Enable=%d\n",param.u8HDMIPort, param.bEnable);

	switch(param.u8HDMIPort)
	{
		case 1:
			eSrcType = INPUT_SOURCE_HDMI_A;
			break;

		case 2:
			eSrcType = INPUT_SOURCE_HDMI_B;
			break;

		case 3:
			eSrcType = INPUT_SOURCE_HDMI_C;
			break;

		case 4:
			eSrcType = INPUT_SOURCE_HDMI_D;
			break;

		default:
			break;
	}
	msleep(100);

	MHal_HDMI_HPD_High(param.bEnable, eSrcType);

}

//#else
#if 0
void MDrv_HDMI_SetHPD(BOOL bHigh, SC_INPUT_SOURCE_e enInput)
{
    // FitchHsu 20090526 Modidfy HPD flow
    if(bHigh)
    {
        MDrv_HDMI_DVIClock_Enable(TRUE, enInput);    // DVI clock pull high
        msleep(100);
        MHal_HDMI_HPD_High(bHigh, enInput);
        //(printk("\r\n++++++ Enable HPD ++++++\n"));
    }
    else
    {
        // clear HDCP status during HPD low
        MHal_HDCP_ClearStatus();
        MHal_HDMI_HPD_High(bHigh, enInput);
        msleep(100);
        MDrv_HDMI_DVIClock_Enable(FALSE, enInput);    // DVI clock pull low
        //(printk("\r\n------ Disable HPD ------\n"));
    }
}
#endif
//#endif

void MDrv_HDMI_Ctrl_DviClock(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    PSC_SOURCE_INFO_t pSrcInfo;
    U32 param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(U32)))
    {
        return;
    }
    SC_HDMI_DEBUG("%s %d\n", __FUNCTION__, param);

    //printk("MDrv_HDMI_Ctrl_DviClock param=%d !!!\n",param);
    pSrcInfo = &pDrvCtx->SrcInfo[SC_MAIN_WINDOW];

    // If it is HDMI input source, DVI clock will high
    // and if it is not HDMI input, DVI clock will low
    if(param)
    {
        MHal_HDMI_SetMux(pSrcInfo->SrcType);
    }
    else
    {
        MHal_HDMI_SetMux(INPUT_SOURCE_NONE);
    }
    MDrv_HDMI_DVIClock_Enable(param, pSrcInfo->SrcType); // FitchHsu 20081202 DVI clock controll error
    //MDrv_ADC_PowerDownDVIClkChannel(!bEnable);

    if (!param)
    {
        // clear HDCP status during HPD low
        MHal_HDCP_ClearStatus();
    }

}
void MDrv_HDCP_InitProductionKey(void)
{
    // cc.chen - T.B.D.
    MHal_HDCP_InitProductionKey(HdcpKey);
}

void MDrv_HDMI_InitVariables(PSC_SOURCE_INFO_t psrc)
{
    psrc->bHDMIMode = FALSE;	//lachesis_081010 audio source change to DVI at no signal.
    psrc->bIsSupportMode = FALSE;
    g_HdmiPollingSts.u8LostHDMICounter = 0;	//lachesis_090207
}

HDMI_COLOR_FORMAT_e MDrv_HDMI_GetPacketColorFormat(void)
{
    return g_HdmiPollingSts.u8ColorFormat;
}


void MDrv_HDMI_GetVSI(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_HDMI_VSI_PACKET_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_HDMI_VSI_PACKET_t)))
    {
        return;
    }
    MHal_HDMI_GetVSI(&param);
    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_HDMI_VSI_PACKET_t)))
    {
        return;
    }
}

void MDrv_HDMI_GetAVI(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_HDMI_AVI_PACKET_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_HDMI_AVI_PACKET_t)))
    {
        return;
    }
    MHal_HDMI_GetAVI(&param);
    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_HDMI_AVI_PACKET_t)))
    {
        return;
    }
}
