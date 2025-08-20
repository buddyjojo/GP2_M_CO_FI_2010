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
#ifndef _MHAL_VE_H
#define _MHAL_VE_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ve.h
/// This file contains the Mstar driver interface for TVEncoder
/// @author MStar Semiconductor Inc.
/// @brief  TVEncoder module
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Board.h"

//#define INPUT_YPBPR_VIDEO_COUNT     0
#define DISPLAY_LOGO                1
//------------------------------------------------------------------------------
// macro
//------------------------------------------------------------------------------

// MStar andy 081013
//#define OPT_VE_HAL_DEBUG
#undef VE_HAL_DBG
#ifdef OPT_VE_HAL_DEBUG
    #define VE_HAL_DBG(fmt, args...)      printk(KERN_WARNING "[VE_HAL][%05d]" fmt, __LINE__, ## args)
#else
    #define VE_HAL_DBG(fmt, args...)
#endif

#undef VE_HAL_DBGX
#define VE_HAL_DBGX(fmt, args...)


#define HIPART(u32x)        ((u32x>>16)&0xFFFF)
#define LOPART(u32x)        (u32x & 0xFFFF)


//------------------------------------------------------------------------------
// enum
//------------------------------------------------------------------------------

// TvEncoder
//
typedef enum
{
    MS_SCALER_DEST_PANEL,
    MS_SCALER_DEST_TVENCODER,
}MS_SCALER_DEST_TYPE;

typedef struct
{
    U32 PreHScalingRatio;
    U32 PreVScalingRatio;
    U32 PostHScalingRatio;
    U32 PostVScalingRatio;
}MS_TVENCODER_BACKUP_REG, *PMS_TVENCODER_BACKUP_REG;


/// type of input source for TV encoder
typedef enum
{
    MS_VE_SRC_MAIN   = 0x00,  ///< input source is from main window
    MS_VE_SRC_SUB    = 0x01,  ///< input source is from sub window
    MS_VE_SRC_SCALER = 0x02,  ///< input source is scaler
    MS_VE_SRC_NONE   = 0x03,
}MS_VE_INPUT_SRC_TYPE;


/// definition of video system
typedef enum
{
    MS_VE_NTSC,     ///< NTSC
    MS_VE_NTSC_443, ///< NTSC443
    MS_VE_NTSC_J,   ///< NTSC_J
    MS_VE_PAL_M,    ///< PAL_M
    MS_VE_PAL_N,    ///< PAL_N
    MS_VE_PAL_NC,   ///< PAL_Nc
    MS_VE_PAL,      ///< PAL_B
    MS_VE_VIDEOSYS_NUM,
}MS_VE_VIDEOSYS;

/// type of output destination for TV encoder
typedef enum
{
    MS_VE_DEST_NONE = 0,
    MS_VE_DEST_SCART,     ///< output destination is SCART
    MS_VE_DEST_CVBS,      ///< output destination is CVBS
    MS_VE_DEST_SVIDEO,    ///< output destination is S-Video
    MS_VE_DEST_YPBPR,     ///< output destination is YPbPr
    MS_VE_DEST_NUM,
} MS_VE_OUTPUT_DEST_TYPE;

typedef struct
{
    MS_VE_OUTPUT_DEST_TYPE OutputDestType[2];   // output device
    MS_VE_INPUT_SRC_TYPE   InputSrcType;        // input source
    MS_VE_VIDEOSYS         VideoSystem;         // video std of output signal
    U8 u8DACType;
    MS_TVENCODER_BACKUP_REG BackupReg;
    U16  u16H_CapStart;
    U16  u16H_CapEnd;
    U16  u16H_CapSize;
    U16  u16V_CapStart;
    U16  u16V_CapEnd;
    U16  u16V_CapSize;
    U8   u8VE_DisplayStatus;
    BOOL bUse_ADC_BuffOut;
    BOOL bInterlace;
}MS_TVENCODER_INFO, *PMS_TVENCODER_INFO;

typedef struct
{
    U8 ucX;
    U8 ucY;
}T_MS_NONLINEAR_POINT;


typedef enum
{
    VE_IPMUX_ADC_A      = 0,            ///< ADC A
    VE_IPMUX_HDMI_DVI   = 1,            ///< DVI
    VE_IPMUX_VD         = 2,            ///< VD
    VE_IPMUX_MVOP       = 3,            ///< MPEG/DC0
    VE_IPMUX_SC_IP1     = 4,            ///< Scaler IP1 output
    VE_IPMUX_EXT_VD     = 5,            ///< External VD
    VE_IPMUX_ADC_B      = 6,            ///< ADC B
} VE_IPMUX_TYPE;

/// MStar VOP Timing structure
typedef struct
{
    U16 u16V_TotalCount;        ///< Vertical total count
    U16 u16H_TotalCount;        ///< Horizontal total count
    U16 u16VBlank0_Start;       ///< Vertical blank 0 start
    U16 u16VBlank0_End;         ///< Vertical blank 0 End
    U16 u16VBlank1_Start;       ///< Vertical blank 1 start
    U16 u16VBlank1_End;         ///< Vertical blank 1 End
    U16 u16TopField_Start;      ///< Top field start
    U16 u16BottomField_Start;   ///< bottom field start
    U16 u16HActive_Start;       ///< Horizontal disaply start
    U8  u8VSync_Offset;         ///< Vertical sync offset
    BOOL bInterlace;         ///< interlace or not
    U8 u8Framerate;             ///< frame rate
    U16 u16H_Freq ;             ///< horizontal frequency
    U16 u16Num;                 ///< VOP SYNTHESIZER numerator
    U16 u16Den;                 ///< VOP SYNTHESIZER denumerator
    U8 u8MvdFRCType;            ///<flag for frame rate convert

    U16 u16ExpFrameRate;    ///< Frame Rate
    U16 u16Width;               ///< Width
    U16 u16Height;              ///< Height
} MS_VOP_TIMING;

///Input port type
typedef enum
{   // BK1_2F
    INPUT_PORT_ADC_RGB,             ///< 0: ADC RGB

#if (INPUT_YPBPR_VIDEO_COUNT >= 1)
    INPUT_PORT_ADC_YPBPR,           ///< 1: ADC YPBPR
#if(INPUT_YPBPR_VIDEO_COUNT==2)
    INPUT_PORT_ADC_YPBPR2,          ///< 2: ADC YPBPR2
#endif
#endif

#if (defined(SATURN_IV) || defined(S4LITE))
    INPUT_PORT_ADC_HDMIA,           ///< HDMI 1
    INPUT_PORT_ADC_HDMIB,           ///< HDMI 2
    INPUT_PORT_ADC_HDMIC,           ///< HDMI 3
#else
    INPUT_PORT_ADC_HDMI,
#endif

    INPUT_PORT_MS_CVBS0 = 0x0F,     ///< 0x0F: MS CVBS0
    INPUT_PORT_MS_CVBS1 = 0x1F,     ///< 0x1F: MS CVBS1

    INPUT_PORT_MS_CVBS2 =0x29,      ///< 0x29: MS CVBS2

    INPUT_PORT_MS_CVBS3 = 0x3F,     ///< 0x3F: MS CVBS3

    INPUT_PORT_MS_SV0 = 0x46,       ///< 0x46: MS SV0
    INPUT_PORT_MS_SV1 = 0x57,       ///< 0x57: MS SV7

//#if(INPUT_SCART_VIDEO_COUNT > 0)
    INPUT_PORT_AV_SCART0 = 0x49,    ///< 0x49: AV SCART0
    INPUT_PORT_AV_SCART1 = 0x28,    ///< 0x28: AV SCART1
//#endif

    #if defined(ENABLE_MEDIAPLAYER) || defined(ENABLE_DMP) || (DISPLAY_LOGO)
    INPUT_PORT_MS_STORAGE = 0xFC,       ///< 0xFE MS DTV
    #endif

    INPUT_PORT_MS_CCIR656 = 0xFD,   ///< 0xFE MS CCIR656

    INPUT_PORT_MS_DTV = 0xFE,       ///< 0xFE MS DTV

    INPUT_PORT_NUMS,                 ///< Numbers of port type
#if 0 // LGE drmyung 080903 : including mdrv_scaler_st.h & compile error
    INPUT_PORT_NONE = INPUT_PORT_NUMS,
#endif
} EN_INPUT_PORT_TYPE;

/// status of switchinging output destination
 //VE out
#define VE_OUT_CVBS_YCC         0x00
#define VE_OUT_CVBS_YCbCr       0x01
#define VE_OUT_CVBS_RGB         0x10
#define VE_OUT_NONE             0xFF

typedef enum
{
    EN_VE_DEMODE         = 0x01,
    EN_VE_CCIR656_IN     = 0x02,
    EN_VE_RGB_IN         = 0x04,
    EN_VE_INVERSE_HSYNC  = 0x08,
}EN_VE_DISPLAY_STATUS;

typedef struct
{
    S16 s16Idx;
    U16 u16Val;
} RegUnitType, *pRegUnitType;

/// status of switchinging output destination
typedef enum
{
    MS_VE_SWITCH_DST_SUCCESS,               ///< success
    MS_VE_SWITCH_DST_INVALID_COMBINATION,   ///< invalid combination
    MS_VE_SWITCH_DST_INVALID_PARAM,         ///< invalid parameter
    MS_VE_SWITCH_DST_FAIL,                  ///< fail
}MS_SWITCH_VE_DST_STATUS;

/// the information of switching ouput destination for TV encoder
typedef struct
{
    U8 u8DestIdx;   ///< the indication of destination device
    MS_VE_OUTPUT_DEST_TYPE OutputDstType; ///< type of output destination
    MS_SWITCH_VE_DST_STATUS Status;     ///< the returning status of switching output destination
}MS_SWITCH_VE_DEST_INFO, *PMS_SWITCH_VE_DEST_INFO;


/// status of switchinging input source
typedef enum
{
    MS_VE_SWITCH_SRC_SUCCESS,           ///< success
    MS_VE_SWITCH_SRC_INVALID_PARAM,     ///< invalid parameter
    MS_VE_SWITCH_SRC_FAIL,              ///< fail
}MS_SWITCH_VE_SRC_STATUS;

/// the information of switching output destination for TV encoder
typedef struct
{
    MS_VE_INPUT_SRC_TYPE InputSrcType; ///< type of input source
    MS_VOP_TIMING Timing; // VOP timing
    MS_SWITCH_VE_SRC_STATUS Status; ///< the returning status of switching input source
}MS_SWITCH_VE_SRC_INFO, *PMS_SWITCH_VE_SRC_INFO;

/// fix color information
typedef struct
{
    BOOL bEnable;           ///< TURE/FALSE, enable/disable fix color our
    U8   u8Color_Y;         ///< color, Y channel
    U8   u8Color_Cb;        ///< color, Cb channel
    U8   u8Color_Cr;        ///< color, Cr channel
}MS_VE_FIX_COLOR_INFO, *PMS_VE_FIX_COLOR_INFO;

/// VE output Type
typedef enum
{
    MS_VE_OUT_CCIR656,  ///< output signal is CCIR656
    MS_VE_OUT_TVENCODER,///< output signal from TVEncoder
}MS_VE_OUT_TYPE;

/// output control for VE
typedef struct
{
    BOOL bEnable; ///< TRUE/FALSE, enable/disable VE
    MS_VE_OUT_TYPE OutputType; ///< VE output type
    U16  u16FrameRate;
	BOOL bFieldInvert; //LGE gbtogether(081211) by Swen.
}MS_VE_OUTPUT_CTRL, *PMS_VE_OUTPUT_CTRL;

typedef enum
{
    EN_WSS_4x3_FULL                    = 0x08,
    EN_WSS_14x9_LETTERBOX_CENTER       = 0x01,
    EN_WSS_14x9_LETTERBOX_TOP          = 0x02,
    EN_WSS_16x9_LETTERBOX_CENTER       = 0x0B,
    EN_WSS_16x9_LETTERBOX_TOP          = 0x04,
    EN_WSS_ABOVE16x9_LETTERBOX_CENTER  = 0x0D,
    EN_WSS_14x9_FULL_CENTER            = 0x0E,
    EN_WSS_16x9_ANAMORPHIC			   = 0x07,
}EN_VE_WSS_TYPE;

/// register
typedef struct
{
    U16 u16Index;   ///< register index
    U8  u8Value;    ///< register value
} MS_REG_TYPE;

 typedef struct
{
    pRegUnitType     pVE_TBL;
    pRegUnitType     pVE_Coef_TBL;
    pRegUnitType     pVBI_TBL;
    BOOL            bvtotal_525;
    BOOL            bPALSwitch;
    BOOL            bPALout;
}MS_VE_OUT_VIDEOSYS, *PMS_VE_OUT_VIDEOSYS;

typedef struct
{
    BOOL bValid;
    U8   u8DACType;
}MS_VE_OUT_DEST, *PMS_VE_OUT_DEST;
//extern void MHal_VE_SwitchInputSrc(PMS_SWITCH_VE_SRC_INFO pSwitchInfo);
extern void MHal_VE_SwitchOuputDest(PMS_SWITCH_VE_DEST_INFO pSwitchInfo);
extern void MHal_VE_Set_OutputCtrl(PMS_VE_OUTPUT_CTRL pOutputCtrl);
extern BOOL MHal_VE_Set_Output_VideoStd(MS_VE_VIDEOSYS VideoSystem);
extern void MHal_VE_PowerOn(void);
extern void MHal_VE_PowerOff(void);
extern BOOL MHal_VE_Init(void);
//extern void MHal_VE_Set_FixColorOut(PMS_VE_FIX_COLOR_INFO pInfo);
//extern void MHal_VE_Set_OuputType(MS_VE_VE_OUTPUT_TYPE enOutputType);
extern void MHal_VE_Enable(BOOL);
extern void MHal_VE_Reset(BOOL);
extern void MHal_VE_GenTestPattern(void);
extern void MHal_VE_SwitchInputSrc(PMS_SWITCH_VE_SRC_INFO pSwitchInfo);
extern void MHal_VE_Set_VPSData(BOOL bEn, U8* u8VPSData);	// MStar andy 081125
extern void MHal_VE_Set_WSSData(BOOL bEn, U16 u16WSSData); // 071204
extern BOOL MHal_VE_Set_CCData(BOOL bEn, U16 u16CCData0, U16 u16CCData1); // 071204
extern BOOL MHal_VE_SaveTTX(U32 u32LineFlag, U32 u32Size,U32 u32PktAddr);
extern void MHal_VE_SetBlackScreen(BOOL bEnable);
//extern void MHal_VE_Set_ADC_BuffOut(BOOL bEnable, EN_INPUT_PORT_TYPE enInputPortType);
//extern void MHal_VE_Set_ADC_RFOut(BOOL bEnable); //LGE gbtogether(081202)

//extern BOOL VE_SavePKtDataInBuffer(U32 size,U32 bufferAddr);
//extern void MHal_VE_SetCurInput(EN_INPUT_PORT_TYPE enInputPortType); //LGE gbtogether(090129)
//extern EN_INPUT_PORT_TYPE MHal_VE_GetCurInput(void); //LGE gbtogether(090129)


#endif

