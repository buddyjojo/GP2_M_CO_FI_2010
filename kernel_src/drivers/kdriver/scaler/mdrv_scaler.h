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
/// @file   drvScaler.h
/// @brief  MStar Scaler Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_SCALER_H_
#define _DRV_SCALER_H_

#include <linux/wait.h>
#include <asm/semaphore.h>

#include "mst_platform.h"
#include "mhal_scaler.h"
#include "mhal_hdmi.h"
#include "mdrv_types.h" // LGE drmyung 081014
#include "mdrv_scaler_st.h"

//-------------------------------------------------------------------------------------------------
//  Kernel Version Define
//-------------------------------------------------------------------------------------------------
#define SC_DOUBLE_SAMPLING_ENABLE  TRUE // LGE drmyung

//#undef INCLUDE_INPUT_CHANGE_TASK	//drmyung 090826
#define INCLUDE_INPUT_CHANGE_TASK	//drmyung 090826

#define	SC_USE_ONLY_ONE_THREAD	// added by LGE(dreamer@lge.com)
#define SC_USE_SCART_THREAD   	// added by LGE(dreamer@lge.com)

#ifdef	INCLUDE_INPUT_CHANGE_TASK  //drmyung 090826
#define HDMI_THREAD_SPEED_UP		// wlgnsl99 HDMI thread 속도 개선방안1.
#define FAST_THREAD_CONTROL
#endif
#define NEW_FRAME_LOCK
#define SC_SPEED_DBG(fmt, args...)    printk("\033[47;31m" fmt "\033[0m", ## args)

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
// source type
#define Use_Analog_Source(x)        ((x >= INPUT_SOURCE_VGA) && (x <= INPUT_SOURCE_YPBPR_2))
#define Use_VGA_Source(x)           (x == INPUT_SOURCE_VGA)
#define Use_YPbPr_Source(x)         ((x >= INPUT_SOURCE_YPBPR_1) && (x <= INPUT_SOURCE_YPBPR_2))
#define Use_CVBS_Source(x)          ((x >= INPUT_SOURCE_ATV) && (x <= INPUT_SOURCE_CVBS_3))
#define Use_DTV_Source(x)           ((x >= INPUT_SOURCE_DTV) && (x <= INPUT_SOURCE_DTV_MLINK))
#define Use_SV_Source(x)            ((x >= INPUT_SOURCE_SVIDEO_1) && (x <= INPUT_SOURCE_SVIDEO_2))
#define Use_SCART_Source(x)         ((x >= INPUT_SOURCE_SCART_1) && (x <= INPUT_SOURCE_SCART_2))
#define Use_HDMI_Source(x)          ((x >= INPUT_SOURCE_HDMI_A) && (x <= INPUT_SOURCE_HDMI_D))
#define Use_VD_Source(x)            ((x >= INPUT_SOURCE_ATV) && (x <= INPUT_SOURCE_SCART_2))
#define Use_ATV_Source(x)           (x == INPUT_SOURCE_ATV)
#define Use_Storage_Source(x)       (x == INPUT_SOURCE_STORAGE)
#define Use_AV_Source(x)            ((x == INPUT_SOURCE_CVBS_1) || (x == INPUT_SOURCE_CVBS_2) || (x == INPUT_SOURCE_CVBS_3))

#define Use_Video_Source(x)         ((x >= INPUT_SOURCE_ATV) && (x <= INPUT_SOURCE_SCART_2) && (x = INPUT_SOURCE_DTV))  //bug??
#define Det_All_Source(x)           ((x >= INPUT_SOURCE_VGA) && (x<= INPUT_PORT_DTV))

#define SC_SET_SRC_STATE(_psrc_, _state_)    (_psrc_)->u32SrcState |=  (_state_)
#define SC_CLR_SRC_STATE(_psrc_, _state_)    (_psrc_)->u32SrcState &= ~(_state_)
#define SC_GET_SRC_STATE(_psrc_)             (_psrc_)->u32SrcState

#define SC_SET_ACTIVE_FLAG(_psrc_, _flg_)   (_psrc_)->u32ActiveFlag = (_flg_)
#define SC_ADD_ACTIVE_FLAG(_psrc_, _flg_)   (_psrc_)->u32ActiveFlag |= (_flg_)
#define SC_CLR_ACTIVE_FLAG(_psrc_)          (_psrc_)->u32ActiveFlag = 0;
#define SC_CHK_ACTIVE_FLAG(_psrc_, _flg_)   ((_psrc_)->u32ActiveFlag & (_flg_))

// sync status
#define SC_SYNCSTS_VSYNC_POL_BIT    BIT0    // VSync polarity bit (0/1 = positive/negative)
#define SC_SYNCSTS_HSYNC_POL_BIT    BIT1    // HSync polarity bit (0/1 = positive/negative)
#define SC_SYNCSTS_HSYNC_LOSS_BIT   BIT2    // HSync loss bit
#define SC_SYNCSTS_VSYNC_LOSS_BIT   BIT3    // VSync loss bit
#define SC_SYNCSTS_INTERLACE_BIT    BIT4    // Interlace mode
#define SC_SYNCSTS_USER_MODE_BIT    BIT7    // User new mode (Not found in mode table)
#define SC_SYNCSTS_SYNC_LOSS        (SC_SYNCSTS_HSYNC_LOSS_BIT | SC_SYNCSTS_VSYNC_LOSS_BIT)
//lachesis_081111 HDMI는 pol check 필요없음. stable에서 return되는 문제 수정.
#define SC_HDMI_CHECK_MASK			(SC_SYNCSTS_SYNC_LOSS|SC_SYNCSTS_INTERLACE_BIT)


#define SC_PCMODE_DEBOUNCE_THRESHOLD    3
#define SC_HDMI_DEBOUNCE_THRESHOLD      7	// 3->5->7 //jaegyo.seo_100108 HDMI 해상도 변경 시 간헐적 flicker 개선.

/// LPLL mode
typedef enum
{
    LPLL_MODE_SINGLE = 0,
    LPLL_MODE_DUAL   = 1,
} LPLL_MODE_t;

/// Source State
typedef enum
{
    SC_SRC_STATE_INPUT_SELECT = BIT0,
    SC_SRC_STATE_CAPTURE_WIN  = BIT1,
    SC_SRC_STATE_DISPLAY_WIN  = BIT2,
#if 0 //080912 LGE drmyung
    SC_SRC_STATE_LPLL_STOP    = BIT3, //thchen 20080904 //080912 LGE drmyung
    SC_SRC_STATE_LPLL         = BIT4,//thchen 20080904 //080912 LGE drmyung
#endif
    SC_SRC_STATE_PCMODE_STOP  = BIT5,
    SC_SRC_STATE_HDMI_STOP    = BIT6,
    SC_SRC_STATE_PCMODE       = BIT7,
    SC_SRC_STATE_HDMI         = BIT8,

    SC_SRC_STATE_ACTIVE       = BIT10,
} SC_SRC_STATE_e;

/// Input signal stataus
typedef enum
{
    SC_INPUT_SIG_YUV_DOMAIN = BIT0,
} SC_INPUT_SIG_STATUS_e;


//====================== Display window information ===================>>>
#define DISPLAYWINDOW_INTERLACE         0x01
#define DISPLAYWINDOW_MADI              0x02
#define DISPLAYWINDOW_NINELATTICE       0x04
#define DISPLAYWINDOW_SKIP_OVERSCAN     0x08
#define DISPLAYWINDOW_HDTV              0x10
#define DISPLAYWINDOW_SKIP_ASPECTRATIO  0x20
#define DISPLAYWINDOW_CROP              0x40
#define DISPLAYWINDOW_TEST_PATTERN      0x80


// signal info
#define SIGINFO_INTERLACE   BIT0
#define SIGINFO_HDMII       BIT1

// Warning! ENABLE_SC_MMIO define in 2 files, madp_scaler.c and mdrv_scaler.h
//  plz change them at the same time
#define ENABLE_SC_MMIO      1//[20090920 Shawn] MM I/O
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
//---------------------------------------------------

/// PC ADC Mode setting type
typedef struct
{
    U8  u8ModeIndex;        ///< mode index
    U16 u16HorizontalStart; ///< horizontal start
    U16 u16VerticalStart;   ///< vertical start
    U16 u16HorizontalTotal; ///< ADC horizontal total
    U16 u16VerticalTotal;   ///< ADC vertical total
    U8  u8Phase;            ///< ADC phase
    U8  u8SyncStatus;       ///< sync status
    U8  u8AutoSign;         ///< Auto tune time
} SC_PCADC_MODESETTING_TYPE;

//capture Win info
typedef struct
{
    U16 u16SRHStart;    // H capture start
    U16 u16SRVStart;    // V capture start
    U16 u16HRange;      // H capture range
    U16 u16VRange;      // V capture range
} SC_CAPTUREWIN_INFO;  //MS_VIDEO_CAPTUREWINTABLE_TYPE

//OverScan
typedef struct
{
    U8  OVERSCAN_H;
    U8  OVERSCAN_V;
} SC_OVERSCAN_Info;

//HISTOGRAM
typedef struct
{
    BOOL bDataReady;
    U16  u16Histogram32[32];
    U8   u8MaxPixelValue;
    U8   u8MinPixelValue;
    U8   u8AvgPixelValue;
    U16  u16TotalColorCount;//[090601_Leo]
} SC_HISTOGRAM_INFO_t;

//---------------------------------------------------
/// Aspect Ratio Type
typedef enum
{
    /* general */
    VIDEOSCREEN_MIN,                        ///< Video Screen Min
    VIDEOSCREEN_PROGRAM = VIDEOSCREEN_MIN, ///< according AFD or WSS setting
    VIDEOSCREEN_NORMAL,                 ///< Video Screen Normal
    VIDEOSCREEN_FULL,                      ///< video full panel resolution
    VIDEOSCREEN_ZOOM,                   ///< Video Screen Zoom
    VIDEOSCREEN_CINEMA,                 ///< Video Screen Cinema

    /* specific options for 4:3 panel */
    VIDEOSCREEN_LETTERBOX,          ///< Video Screen Letterbox

    /* specific options for wide panel */
    VIDEOSCREEN_16by9_SUBTITLE, ///< Video Screen 16:9 subtitle
    #if defined(ENABLE_NON_LINEAR_SCALING)
    VIDEOSCREEN_PANORAMA,           ///< Video Screen Panorama
    #endif

    /* others */
    VIDEOSCREEN_14by9,                     ///< 14:9
    VIDEOSCREEN_WSS_16by9,            ///< WSS 16:9
    VIDEOSCREEN_ZOOM1,                    ///< Video Screen Zoom1
    VIDEOSCREEN_ZOOM2,                    ///< Video Screen Zoom2
    VIDEOSCREEN_JUSTSCAN,               ///< Video Screen Justscan
    VIDEOSCREEN_SCENE4_3to16_9,    ///< Video Screen Scene 4:3 to 16:9
    VIDEOSCREEN_SCENE16_9to4_3,     ///< Video Screen Scene 16:9 to 4:3
    VIDEOSCREEN_SCENE4_3to16_9_WITH_CCO,    ///< Video Screen Scene 4:3 to 16:9 with CCO
    VIDEOSCREEN_SCENE4_3to4_3_WITH_CCO,     ///< Video Screen Scene 4:3 to 4:3 with CCO
    VIDEOSCREEN_SCENE4_3to16_9_WITH_LB,     ///< Video Screen Scene 4:3 to 16:9 with LB
    VIDEOSCREEN_SCENE4_3to4_3_WITH_LB,      ///< Video Screen Scene 4:3 to 4:3 with LB
    #if defined(ENABLE_MEDIAPLAYER) || defined(ENABLE_DMP) || defined(DISPLAY_LOGO)
    VIDEOSCREEN_ORIGIN,
    #endif
    VIDEOSCREEN_NUMS,                      ///< numbers of video screen type
}EN_ASPECT_RATIO_TYPE;
//---------------------------------------------------

typedef struct MACE_INFO_s
{
    S16 SRGB_Matrix[3][3];
    S16 YUV2RGB_Matrix[3][3];
    S16 CC_Matrix[3][3];

    U8  u8Contrast;
    U8  u8RCon;
    U8  u8GCon;
    U8  u8BCon;
    U16 u16RConEx;
    U16 u16GConEx;
    U16 u16BConEx;
    U8  u8Saturation;
    U8  u8Hue;
    S16 sContrastRGBMatrix[3][3];
    S16 sVideoSatHueMatrix[3][3];
    U8  bForceYUVtoRGB;
    U8  bPCUsesRGBMatrix;
} MACE_INFO_t, *PMACE_INFO_t;


//++ SC thread
typedef enum
{
    SC_THREAD_CTRL_NONE                 = 0,
    SC_THREAD_CTRL_TERMINATE            = BIT0,
    SC_THREAD_CTRL_FAST_STATE_CHANGE    = BIT1,
} SC_THREAD_CTRL_e;

typedef enum
{
    SC_PCMODE_INIT_STATE,           // 0
    SC_PCMODE_DEBOUNCE_STATE,       // 1
    SC_PCMODE_TIMING_CHANGE_STATE,  // 2

    SC_PCMODE_SYNC_DETECT_STATE,    // 3
    SC_PCMODE_SEARCH_MODE_STATE,    // 4

    SC_PCMODE_RESOLUTION_CHANGE_STATE,	// 5		// swwoo LGE 080702
    SC_PCMODE_SUPPORTED_STATE,      // 6
    SC_PCMODE_UNSUPPORTED_STATE,    // 7
    SC_PCMODE_STABLE_CHECK_STATE,   // 8

    SC_PCMODE_IDLE_STATE,           // 9
} SC_PCMODE_STATE_e;

typedef enum
{
    SC_HDMI_INIT_STATE          = 0,
    SC_HDMI_DEBOUNCE_STATE          = 1,
    SC_HDMI_TIMING_CHANGE_STATE     = 2,

    SC_HDMI_SYNC_DETECT_STATE       = 3,
    SC_HDMI_CHK_MODE_STATE          = 4,
    SC_HDMI_SEARCH_MODE_STATE       = 5,
    SC_HDMI_SUPPORTED_STATE         = 6,
    SC_HDMI_UNSUPPORTED_STATE       = 7,

#ifndef HDMI_THREAD_SPEED_UP
    //++ HDMI only
    SC_HDMI_GET_PACKET_INFO_STATE   = 8,
    SC_HDMI_DO_AVMUTE_STATE         = 9,
    SC_HDMI_CHK_AUDIO_PKT_STATE     = 10,
    SC_HDMI_CHK_COLOR_FMT_STATE     = 11,

    SC_HDMI_CHK_PKT_ERR_STATE       = 12,

    SC_HDMI_SET_MODE_STATE          = 13,

    SC_HDMI_STABLE_CHECK_STATE      = 14,
#else
	SC_HDMI_CONTROL_PACKET_STATE 	= 8,
    SC_HDMI_STABLE_CHECK_STATE      	= 9,
#endif

} SC_HDMI_STATE_e;

typedef struct
{
    U8  u8ThreadState;
    U16 u16Ctrl;

    wait_queue_head_t   thread_wq;
    wait_queue_head_t   wait_event_wq;
    atomic_t            wait_event_sts;
} SC_THREAD_DATA_t, *PSC_THREAD_DATA_t;
//-- SC thread

typedef enum
{
    SC_ACTIVE_FLG_NULL     = 0,
    SC_ACTIVE_FLG_FMT_CHG  = BIT0,
    SC_ACTIVE_FLG_CAP_WIN  = BIT1,
    SC_ACTIVE_FLG_DISP_WIN = BIT2,
    SC_ACTIVE_FLG_CROP_WIN = BIT3,

    SC_ACTIVE_FLG_ALL = SC_ACTIVE_FLG_FMT_CHG | SC_ACTIVE_FLG_CAP_WIN | SC_ACTIVE_FLG_DISP_WIN | SC_ACTIVE_FLG_CROP_WIN,
} SC_ACTIVE_FLG;


typedef struct SC_SOURCE_INFO_s
{
    SC_INPUT_SOURCE_e SrcType;

    // general
    U32 u32SrcState;
    U32 u32ActiveFlag;
    U8 u8NumOfFB;
    BOOL bMemFormat422;
    U8 u8BitPerPixel;

    // signal status
    U8  u8Input_SyncStatus;
    U16 u16Input_SigStatus;
    U16 u16Output_SigStatus;

    // input timing
    U16 u16Input_HFreq;
    U16 u16Input_VFreq;
    U16 u16Input_HPeriod;
    U16 u16Input_VTotal;
    U16 u16Input_HDE;
    U16 u16Input_VDE;
    U16 u16Input_HDE_Start;
    U16 u16Input_VDE_Start;
    U16 u16Input_HActive;
    U16 u16Input_VActive;
#if 1 // drmyung LGE 080722
    U16 u16Input_HDMI_HPeriod;
    U16 u16Input_HDMI_VTotal;
    U16 u16Input_HDMI_HDE;
    U16 u16Input_HDMI_VDE;
    U16 u16Input_HDMI_HDE_Start;
    U16 u16Input_HDMI_VDE_Start;
    U16 u16Input_HDMI_HFreq;
    U16 u16Input_HDMI_VFreq;
    U16 u8Input_HDMI_SyncStatus;

    U16 u16Input_SC_HFreq;
    U16 u16Input_SC_VFreq;
    U16 u16Input_SC_HPeriod;
    U16 u16Input_SC_HStart;
    U16 u16Input_SC_VStart;
    U16 u16Input_SC_VTotal;
    U16 u16H_SC_CapSize;
    U16 u16V_SC_CapSize;
    U8 u8Input_SC_SyncStatus;
    U8 u8Input_SC_Phase;

#endif
    // capture win
    U16 u16H_CapStart;
    U16 u16H_CapStart_Backup;
    U16 u16V_CapStart;
    U16 u16H_CapSize;
    U16 u16H_CapSize_Backup;
    U16 u16V_CapSize;

    // Crop win
    U16 u16H_CropStart;
    U16 u16H_CropStart_Backup;
    U16 u16V_CropStart;
    U16 u16H_CropSize;
    U16 u16H_CropSize_Backup;
    U16 u16V_CropSize;
    U16 u16H_CropsizeAfterIP2;
    U16 u16V_CropsizeAfterIP2;
    U16 u16H_Cropoffset;
    U16 u16V_Cropoffset;
    U16 u16CropLBoffset;
    U16 u16CropOPMFetch;
    BOOL bCropWin_Enable;

    // display Win
    U16 u16H_DispStart;
    U16 u16V_DispStart;
    U16 u16H_DispSize;
    U16 u16V_DispSize;

    // size after IP2
    U16 u16H_sizeAfterIP2;
    U16 u16V_sizeAfterIP2;

    // scaling ratio
    U32 u32HSDRatio;
    U32 u32VSDRatio;
    U32 u32HSPRatio;
    U32 u32VSPRatio;
    BOOL bPreV_ScalingDown;

    // IPM
    U32 u32IPMBase0;
    U32 u32IPMBase1;
    U32 u32IPMBase2;
    U32 u32MirrorIPMBase0;
    U32 u32MirrorIPMBase1;
    U32 u32MirrorIPMBase2;
    U16 u16IPMFetch;
    U16 u16IPMOffset;

    // OPM
    U32 u32OPMBase0;
    U32 u32OPMBase1;
    U32 u32OPMBase2;
    U16 u16OPMFetch;
    U16 u16OPMOffset;
    BOOL bLinearAddrMode;   //20091225 daniel.huang: for 3 frame mode crop setting incorrect

    // PC Mode
    U8  u8ModeIdx;
    // swwoo LGE 080626 : insert
    S16  s16PC_H_POS;
    S16  s16PC_V_POS;
    U8  u8PC_H_SIZE;
    U8  u8PC_V_SIZE;
    S16 s16PC_H_TOTAL;
    // swwoo LGE 080626 : end

    // MACE
    MACE_INFO_t MACE_Info;

    // thread
    SC_THREAD_DATA_t ThreadData;

    // HDMI
    BOOL bHDMIMode;
    BOOL bIsSupportMode;
    U8   u8HDMIColorFormat;

    //SC_CAPTUREWIN_INFO CaptureWin;
    SC_PCADC_MODESETTING_TYPE Modesetting;
    SC_OVERSCAN_Info OVERSCAN_Info;

    // DE Win info
    U8  u8DE_V_Start;           ///< DE Vertical start
    U8  u8DE_V_Shift;           ///< DE Vertical shift
    U16 u16DE_V_End;            ///< DE Vertical end

    U16 u16H_DeBlockStart;      ///< deblock H start
    U16 u16V_DeBlockStart;      ///< deblock V start

    // Crop Win info
    U16 u16Crop_Up;             ///< Crop up
    U16 u16Crop_Down;           ///< Crop down
    U16 u16Crop_Left;           ///< Crop Left
    U16 u16Crop_Right;          ///< Crop Right
    S16 s16Crop_VPos;           ///< Vertical Position
    S16 s16Crop_HPos;           ///< Horizontal Position

    // Over Scan Info
    BOOL ENABLE_OVERSCAN_CUSDEF;
    U8  u8H_OverScanRatio;      ///< Horizontal overscan ratio
    U8  u8V_OverScanRatio;      ///< Vertical overscan ratio

    U8  u8DisplayStatus;        ///< display status
    U16 u16V_Length;            ///< for MDrv_Scaler_SetFetchNumberLimit

    // VD info
    U8  u8VideoSystem;

    // Active index
    U8  u8Active;

    U32 u32SigInfo;
    U16 u16mvdtype;
    SC_HISTOGRAM_INFO_t HistogramData;
    U8 u8framelockmode; //thchen 20080924
    U8 u8freerunmode;  //thchen 20080924
#ifdef	SC_USE_ONLY_ONE_THREAD
    U8 u8ThreadMode;   // added by LGE(dreamer@lge.com)
	#ifdef FAST_THREAD_CONTROL
    U8 u8ThreadCtrl;   // added by LGE(lachesis@lge.com)
	#endif
#endif
    U16 u16HTotal; //thchen 20080925
    U16 u16VTotal;  //thchen 20080925
    BOOL bHDuplicate;   //LGE gbtogether(081015) by FitchHsu        ///< flag for MVOP horizontal duplicate
    BOOL bEmppreview;   //FitchHsu 20081119 EMP preview setting for 24P and 30P
    BOOL bEmpPlayingVideo;  //victor 20090108, add emp video input source
    BOOL bMediaPhoto; //[090910_Leo]
    BOOL binputsync; // FitchHsu 20090116 VGA Lock issue
    BOOL bTimingCHG; //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
    U16 u16OriInput_VFreq; //FitchHsu 20090121 EMP preview tearing after normal play
    FRAME_LOCK_e u8FrameLock;  // 20090822 FitchHsu fix Ursa ATV snow noise frame lock
    U16 u16OutputVFreq;
    BOOL bFBL;
    U16 u16PQSrcType;
    BOOL bHMirror;
    BOOL bVMirror;
    U8 u8MirrorAlignWidth;
    U32 u32MirrorBaseOffset;
} SC_SOURCE_INFO_t, *PSC_SOURCE_INFO_t;

///IP MUX select
typedef enum
{
    SC_IPMUX_ADC_A      = 0,
    SC_IPMUX_HDMI_DVI   = 1,
    SC_IPMUX_VD         = 2,
    SC_IPMUX_MVOP       = 3,
    SC_IPMUX_SC_VOP     = 4,
    SC_IPMUX_EXT_VD     = 5,
    SC_IPMUX_ADC_B      = 6,
    SC_IPMUX_MVOP_MLINK = 8,
} SC_IPMUX_t;

//---------------------------------------------------
//Digital VD//
/// Video standard
typedef enum
{
    SIG_NTSC,           ///< NTSC
    SIG_PAL,            ///< PAL
    SIG_SECAM,          ///< SECAM
    SIG_NTSC_443,       ///< NTSC 443
    SIG_PAL_M,          ///< PAL M
    SIG_PAL_NC,         ///< PAL NC
    SIG_NUMS,           ///< signal numbers

    SIG_NONE = -1
} EN_VD_SIGNALTYPE;

///Get sync type
typedef enum
{
    GET_SYNC_DIRECTLY,  ///< 0: get sync directly
    GET_SYNC_STABLE,    ///< 1: get sync stable
    GET_SYNC_VIRTUAL,   ///< 2: get sync virtually
} EN_VD_SYNC_TYPE;

typedef struct
{
    EN_VD_SIGNALTYPE ucVideoSystem;
    EN_VD_SYNC_TYPE  ucVideoSync_status;

    BOOL bIsSignal50Hz;
    U16  u16VideoStatus;
    U16  u16LastVideoStatus;
    U8   ucVideoStableCounter;
    U8   ucVideoPollingCounter;
    U8   u8VideoPollingFlag;
} VD_SOURCE_INFO;


//---------------------------------------------------
/// Deinterlace mode
typedef enum
{
    MS_DEINT_OFF,
    MS_DEINT_2DDI_BOB,
    MS_DEINT_2DDI_AVG,
    MS_DEINT_3DDI_HISTORY, // 24 bit
    MS_DEINT_3DDI,         // 16 bit
} MS_DEINTERLACE_MODE;

//---------------------------------------------------
/// display system information
typedef struct
{
    BOOL EN_MIAN_WIN;
    BOOL EN_SUB_WIN;
} SC_WINDISPLAY_CTRL_t;

typedef struct SC_DRIVER_CONTEXT_s
{
    // memory
    U32 u32MemAddr;
    U32 u32MemSize;

    // panel info
    PMST_PANEL_INFO_t pPanelInfo;

    // source info
    //Dixon, need to add a parameter to register MAIN/SUB
    //Set 0: MAIN Win, 1: SUB Win
    SC_WINDISPLAY_CTRL_t WinDisplayCtrl;
    SC_SOURCE_INFO_t     SrcInfo[2];

    U32* ModeTablePtr;
} SC_DRIVER_CONTEXT_t, *PSC_DRIVER_CONTEXT_t;


//---------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MDrv_SC_InitDrvCtx(PSC_DRIVER_CONTEXT_t pDrvCtx);
void MDrv_SC_Cleanup(PSC_DRIVER_CONTEXT_t pDrvCtx);
U32  MDrv_SC_Init(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
U32 MDrv_SetPanelData(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetPanelOutput(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetNoSignalColor(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
U32  MDrv_SC_SetInputSource(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetMVDSigInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetVDSigInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetSCSigInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); // LGE drmyung 081022
void MDrv_SC_SetCaptureWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_GetCaptureWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_GetRealCaptureWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MDrv_SC_SetDisplayWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_GetDisplayWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_Active(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_CheckFrameLock(PSC_DRIVER_CONTEXT_t pDrvCtx); // LGE drmyung 081106
void MDrv_SC_SetBlackScreen(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetFreerun(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//lachesis_090831

void MDrv_SC_WaitEvent(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetBrightness(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetMode(PSC_SOURCE_INFO_t psrc);
void MDrv_SC_SetCrop(PSC_SOURCE_INFO_t psrc);
void MDrv_Scaler_GenerateBlackVideo(BOOL bEnable);
void MDrv_SC_SetFrameRate(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_GetFrameRate(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_GetInputTimingInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_GetHDMIInputTimingInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetGammaTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetFilmMode(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetWBTestPattern(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetTestPattern(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetFrameColor(U32 arg);
void MDrv_SC_SetCropWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);

void MDrv_SC_GetCropWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetPqlColor(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetGOPSEL(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SaveGOPSetting(void);    //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MDrv_SC_RestoreGOPSetting(void); //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MDrv_SC_SetGOP_TO_IP(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetGOP_TO_VOP(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_NotifyChangedFmt(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetMVDType(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg) ;
void MDrv_SC_HDMI_Init(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); // LGE wlgnsl99
void MDrv_SC_SetInputMux(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); // LGE drmyung 080903
void MDrv_IS_HDMI(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080923
void MDrv_SC_SetHDMIEQ(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);		// 081027 wlgnsl99 LGE : set HDMI EQ
void MDrv_SC_SetDeFeatheringThreshold(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080923
void MDrv_SC_SetDuplicate(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_TriggerByVSync(PSC_SOURCE_INFO_t psrc);
void MDrv_SC_SetFrameLock(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bEnable);  //thchen 20081001
void MDrv_SC_SetFreeRunPanelTiming(PSC_DRIVER_CONTEXT_t pDrvCtx, SC_WINDOW_IDX_e srcIdx); //thchen 20080911 //080912 LGE drmyung
//#if REFINE_FPLL
U16 MDrv_SC_SetPanelTiming(PSC_DRIVER_CONTEXT_t pDrvCtx, U8 bInputSourceEnable, BOOL bFreeRun);//lachesis_090827
//#endif
void MDrv_SC_NotifyChangedFmt(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); // drmyung LGE 080619

void MDrv_SC_SetColorAdaptiveRange(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090601_Leo]
void MDrv_SC_SetAdaptiveCGainEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090921_Leo]
void MDrv_SC_SetAdaptiveCGain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090814_Leo]
void MDrv_SC_SetPieceWiseEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090825_Leo]
void MDrv_SC_SetCDNREnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090615_Leo]
void MDrv_SC_SetCDNRIndex(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090616_Leo]
void MDrv_SC_SetCDNRGain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090617_Leo]
void MDrv_SC_SetAutoNREnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090617_Leo]
#if (ENABLE_SC_MMIO )
void* MDrv_SC_GetMMAPAddr(void); //[20090920 Shawn] MM I/O
#endif
//------------------------------------------------------------------------------
//  IP1
//------------------------------------------------------------------------------
void MDrv_SC_IP1_SetFieldDetect(PSC_SOURCE_INFO_t psrc);
U8   MDrv_SC_IP1_GetInputSyncStatus(void);
BOOL MDrv_SC_IP1_WaitInputVSync(U8 u8VSyncCnt, U16 u16Timeout);
BOOL MDrv_SC_IP1_WaitOutputVSync(U8 u8VSyncCnt, U16 u16Timeout);

//------------------------------------------------------------------------------
//  IP2
//------------------------------------------------------------------------------
void MDrv_SC_IP2_SetCSC(PSC_SOURCE_INFO_t psrc);
void MDrv_SC_IP2_SetCSCEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080909
void MDrv_SC_VIP_GetCSCEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //20091020 daniel.huang: fix gop test pattern cannot cover all video problem

void MDrv_SC_IP2_HScaling(PSC_SOURCE_INFO_t psrc);
void MDrv_SC_IP2_VScaling(PSC_SOURCE_INFO_t psrc);

//------------------------------------------------------------------------------
//  IPM
//------------------------------------------------------------------------------
void MDrv_SC_IPM_CalMemSetting(PSC_SOURCE_INFO_t psrc, U32 u32MemAddr, U32 u32MemLen);
void MDrv_SC_IPM_SetFreezeImg(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bFreeze);

//------------------------------------------------------------------------------
//  OPM
//------------------------------------------------------------------------------
// Michu 20090903
void MDrv_SC_OPM_CalMemSetting(PSC_SOURCE_INFO_t psrc);

//------------------------------------------------------------------------------
//  OP1
//------------------------------------------------------------------------------
void MDrv_SC_OP1_CalVScaling(PSC_SOURCE_INFO_t psrc);
void MDrv_SC_OP1_CalHScaling(PSC_SOURCE_INFO_t psrc);

//------------------------------------------------------------------------------
//  OP2
//------------------------------------------------------------------------------
void MDrv_SC_OP2_SetCSC(PSC_SOURCE_INFO_t psrc);

//-----------------------------------------------------------------------------
//  VOP
//-----------------------------------------------------------------------------
void MDrv_SC_VOP_SetFreeRunColorEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bEnable);
void MDrv_SC_VOP_SetFreeRunColor(PSC_DRIVER_CONTEXT_t pDrvCtx, SC_FREERUN_COLOR_e color);

//-----------------------------------------------------------------------------
// IP MUX
//-----------------------------------------------------------------------------
void MDrv_SC_SetIPMux(SC_INPUT_SOURCE_e source);

//-----------------------------------------------------------------------------
//  LPLL
//-----------------------------------------------------------------------------
void MDrv_SC_LPLL_SetODClk(PSC_DRIVER_CONTEXT_t pDrvCtx, U16 u16OutputVFreq);
//U16  MDrv_SC_LPLL_SetFRC(U16 u16InputVFreq, BOOL bPnlDblVSync);
U32 MDrv_SC_LPLL_CheckODClk(PSC_DRIVER_CONTEXT_t pDrvCtx, U16 u16OutputVFreq); // LGE drmyung 081103
U8 MDrv_SC_LPLL_CheckFRC(U16 u16InputVFreq, U16* u16OutputVFreq, PSC_SOURCE_INFO_t pSrcInfo);
#if 0 //080912 LGE drmyung
void MDrv_SC_LPLL_Start(PSC_SOURCE_INFO_t psrc); //thchen 20080904 //080912 LGE drmyung
void MDrv_SC_LPLL_Stop(PSC_SOURCE_INFO_t psrc); //thchen 20080904 //080912 LGE drmyung
#endif
// MOD
void MDrv_SC_SetMODPower(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bEnable);


  // Sync & pixel clk
U16  MDrv_SC_CalculateHFreqX10(U16 wHPeriod);
U16  MDrv_SC_CalculateVFreqX10(U16 wHFreq, U16 wVTotal);
U8   MDrv_SC_CalculateVSyncTime(U16 u16VTotal, U16 u16HPeriod);
U8   MDrv_SC_CalculatePixClk(U16 u16HTotal);

BOOL MDrv_SC_GetInputStatus(SC_SOURCE_INFO_t enSourceInfo);

EN_MODE_HANDLE_EVENT MDrv_SC_ModeParse_Event(SC_SOURCE_INFO_t enSourceInfo);

BOOL MDrv_SC_PCMode_EVENT_TimingDetect(SC_SOURCE_INFO_t enSourceInfo);

//------------------------------------------------------------------------------
//  Others
//------------------------------------------------------------------------------
void MDrv_SC_WakeupEvent(PSC_SOURCE_INFO_t psrc, U32 u32Event);
void MDrv_Scart_GetFBMode(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
int  MDrv_Scart_GetARMode(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_Scart_SetOverlay(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
SC_SCART_AR_e MDrv_Scart_GetAspectRatio(SC_INPUT_SOURCE_e scartIdx);
void MDrv_SC_SetPreConBri(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);   //victor 20081016, ContrastBrightness
void MDrv_SC_SetPostConBri(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);  //victor 20081016, ContrastBrightness
void MDrv_SC_SetBlackLevel(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);  //victor 20081106
void MDrv_EnableDNR(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20081203, DNR

void MDrv_SC_GetFrameData(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetFrameData(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);

// 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
void MDrv_SC_GetFrameDataCore(U16 *pX0, U16 *pY0, U16 *pWidth, U16 *pHeight,
                              BOOL *pRGB,
                              U8 *pRect,
                              U32 u32RectPitch,
                              U32 u32RectSize,
                              U16 u16PointSize,
                              BOOL bUseDispCoordinate);

// 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
void MDrv_SC_SetFrameDataCore(U16 *pX0, U16 *pY0, U16 *pWidth, U16 *pHeight,
                              BOOL *pRGB,
                              U8 *pRect,
                              U32 u32RectPitch,
                              U32 u32RectSize,
                              U16 u16PointSize,
                              BOOL bUseDispCoordinate);
void MDrv_SC_SpeedUpFrameLock(PSC_SOURCE_INFO_t pSrcInfo);

//------------------------------------------------------------------------------
//  Utility
//------------------------------------------------------------------------------
U16  MDrv_Calculate_ABS(U16 num1, U16 num2);
U16  MDrv_SC_SubtractABS(U16 num1, U16 num2);
void MDrv_SC_GET_Version(U32 arg);
void MDrv_TimingDataInit(PSC_SOURCE_INFO_t psrc);
extern U8 MDrv_MICOM_RegRead( U16 u16Addr );
void MDrv_SC_PQ_FastPlayback(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //FitchHsu 20081113 EMP when PAUSE, little shaking
void MDrv_SC_EMP_Preview(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //FitchHsu 20081119 EMP preview setting for 24P and 30P
void MDrv_SC_EMP_SET4frames(PSC_SOURCE_INFO_t psrc);
void MDrv_SC_EMP_SetPlayingEMPVideo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20090108, add emp video input source
void MDrv_SC_EMP_SetEMPJPEG(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); //[090910_Leo]
void MDrv_SC_SetTimgCHGstauts(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL arg); //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode

//------------------------------------------------------------------------------
//  Utility : PQL Dump Table
//------------------------------------------------------------------------------
void MDrv_SC_PQDumpRegTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_Set_Blue_Stretch_Enable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080830
void MDrv_SC_Set_CSC_Offset_Enable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080830


#ifdef	SC_USE_ONLY_ONE_THREAD
extern int MDrv_SC_Thread(void *data);
#endif
//------------------------------------------------------------------------------
//  Menuload driver
//------------------------------------------------------------------------------
void MDrv_SC_Set3DComb(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20081112, 3DComb
void MDrv_SC_SetSSC(U32 arg);

//------------------------------------------------------------------------------
//  MWE
//------------------------------------------------------------------------------
// CC Chen 20081124 MWE implement
void MDrv_SC_MWE_SetWinType(PSC_DRIVER_CONTEXT_t pDrvCtx, SC_MWE_TYPE_e type);
void MDrv_SC_MWE_Enable(BOOL bEnable);

void MDrv_SC_Set_THXMode(U32 arg); // FitchHsu 20081209 implement THX mode
void MDrv_SC_Get_FrameLock_Status(U32 arg); // LGE [vivakjh]  2008/12/11 Merge!!  FitchHsu 20081209 implement frame lock status report


//LGE [vivakjh]  2008/12/07	pDrvCtx를 인자로 사용하지 않는 함수에서 PDP/LCD를 구분하기 위한 함수. mdrv_scaler.c 내에서만 사용함.
BOOL MDrv_SC_IsPDPPanel(void);
/******************************************************************************
	LGE IOCTL : 240 ~ 254 (추가 자제할 것)
*******************************************************************************/
// LGE [vivakjh] 2008/12/09 	For setting the PDP's Color Wash
void MDrv_SC_SetColorWash4PDP(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetFrameTo48Hz(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL arg);	// LGE[totaesun] 2008.12.27 24P 입력일때 48Hz 출력으로 바꾸기 위함.

void MDrv_SC_SetFBL(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_Set_LPLL(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg); // FitchHsu 20080811 implement LPLL type
#if !REFINE_FPLL
void MDrv_LPLL_GetFrameSync(U8* u8FrcIn, U8* u8FrcOut); // LGE drmyung 081024
#endif
//------------------------------------------------------------------------------
// VIDEO MIRROR
// Michu 20090903
//------------------------------------------------------------------------------
void MDrv_SC_Set_VideoMirror(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
//------------------------------------------------------------------------------
// End of VIDEO MIRROR
//------------------------------------------------------------------------------
// 20091021 daniel.huang: add ipmux test pattern for inner test pattern
void MDrv_SC_IPMUX_SetTestPattern(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);

// Michu 20091026, OD
void MDrv_SC_Set_ODInitial(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_OverDriverSwitch(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_SetFD_Mask(U32 arg);
void MDrv_SC_SetDithering(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);

#endif // _DRV_SCALER_H_
