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

#ifndef __DRV_SCLAER_ST_H__

#define __DRV_SCLAER_ST_H__
#define GM_GBD_MAX_SIZE             21
#define TCON_TAB_MAX_SIZE           2048
#define PQL_AP_DB_MAX_SIZE          1024

// For mode flag
#define MD_FLAG_NULL    0x00

#define MD_FLAG_POR_HPVP        BIT0
#define MD_FLAG_POR_HPVN        BIT1
#define MD_FLAG_POR_HNVP        BIT2
#define MD_FLAG_POR_HNVN        BIT3

#define SC_YPBPR_720x480_60_SW_PATCH  FALSE
/*
#if (SUPPORT_EURO_HDTV)
  #define MD_FLAG_EURO_HDTV_BIT   BIT5
#endif
*/

//------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------
// Flag for mode table
#define SC_PCMODE_FLAG_NULL             0x00
#define SC_PCMODE_FLAG_POR_HPVP         BIT0
#define SC_PCMODE_FLAG_POR_HPVN         BIT1
#define SC_PCMODE_FLAG_POR_HNVP         BIT2
#define SC_PCMODE_FLAG_POR_HNVN         BIT3
#define SC_PCMODE_FLAG_EURO_HDTV_BIT    BIT5
#define SC_PCMODE_FLAG_INTERLACE        BIT4
#define SC_PCMODE_FLAG_YPBPR_BIT        BIT6
#define SC_PCMODE_FLAG_HDTV_BIT         BIT7

// Event
#define SC_EVENT_WAITING                0x00

#define SC_EVENT_TIMING_CHANGE          0x01

#define SC_EVENT_PCMODE_UNSUPPORTED     0x20
#define SC_EVENT_PCMODE_SUPPORTED       0x21
#define SC_EVENT_PCMODE_CHANGE          0x22    // added by LGE(dreamer@lge.com)

#define SC_EVENT_HDMI_UNSUPPORTED       0x30
#define SC_EVENT_HDMI_SUPPORTED         0x31
#define SC_EVENT_HDMI_CHANGE            0x32    // added by LGE(dreamer@lge.com)

#define SC_EVENT_STOP                   0xFF

//------------------------------------------------------------------------------
// Structure
//------------------------------------------------------------------------------

typedef enum
{
    SC_TCON_PANEL_LG42,
    SC_TCON_PANEL_LG47,
    SC_TCON_PANEL_NUMS,
} SC_TCON_PANEL_INDEX;

typedef enum
{
    SC_SIGNAL_POL,
    SC_SIGNAL_VGH,
    SC_SIGNAL_SOE,
    SC_SIGNAL_VST,
    SC_SIGNAL_GCLK,
    SC_SIGNAL_NUMS,
} SC_POWER_SEQUENCE_SIGNAL_TYPE;

#if 0
typedef struct IO_SC_PCMODE_INFO_s
{
    U32* pPCModeInfo;
    U16  u16TotalLen;
    U16  u16MaxIndex;
} IO_SC_PCMODE_INFO_t, *PIO_SC_PCMODE_INFO_t;
#endif

// common
typedef enum
{
    SC_MAIN_WINDOW = 0,
    SC_SUB_WINDOW  = 1,
} SC_WINDOW_IDX_e;

typedef enum
{
    // Analog port
    INPUT_SOURCE_VGA,
    INPUT_SOURCE_YPBPR_1,
    INPUT_SOURCE_YPBPR_2,

    // Digital port
    INPUT_SOURCE_ATV,
    INPUT_SOURCE_CVBS_1,
    INPUT_SOURCE_CVBS_2,
    INPUT_SOURCE_CVBS_3,

    INPUT_SOURCE_SVIDEO_1,
    INPUT_SOURCE_SVIDEO_2,

    INPUT_SOURCE_SCART_1,
    INPUT_SOURCE_SCART_2,

    // HDMI port
    INPUT_SOURCE_HDMI_A,	//11
    INPUT_SOURCE_HDMI_B,	//12
    INPUT_SOURCE_HDMI_C,	//13
    INPUT_SOURCE_HDMI_D,	//14

    // MVD port
    INPUT_SOURCE_DTV,
    INPUT_SOURCE_DTV_MLINK,

    INPUT_SOURCE_STORAGE,              ///< input source is Storage

    INPUT_SOURCE_NUM,
    INPUT_SOURCE_NONE = INPUT_SOURCE_NUM
} SC_INPUT_SOURCE_e;

typedef enum
{
    INPUT_PORT_VGA,
    INPUT_PORT_YPBPR,
    INPUT_PORT_HDMI,
    INPUT_PORT_AV,
    INPUT_PORT_SV,
    INPUT_PORT_SCART,
    INPUT_PORT_EXTERNAL_VD,
    INPUT_PORT_DC,
    INPUT_PORT_ATV,
    INPUT_PORT_DTV,
    INPUT_PORT_NUM,
    INPUT_PORT_NONE = INPUT_PORT_NUM,
} SC_INPUT_PORT_e;

// free run mode color
typedef enum
{
    FREE_RUN_COLOR_BLACK,
    FREE_RUN_COLOR_WHITE,
    FREE_RUN_COLOR_BLUE,
    FREE_RUN_COLOR_RED,
    FREE_RUN_COLOR_GREEN,
    FREE_RUN_COLOR_GREY,
    FREE_RUN_COLOR_MAX,
} SC_FREERUN_COLOR_e;

// quality map index
typedef enum
{
    QM_COM,
    QM_ATV_NTSC,
    QM_ATV_PAL,
    QM_ATV_SECAM,
    QM_SV_NTSC,
    QM_SV_PAL,
    QM_SV_SECAM,
    QM_AV_NTSC,
    QM_AV_PAL,
    QM_AV_SECAM,
    QM_480I,
    QM_576I,
    QM_1080I_60HZ,
    QM_1080I_50HZ,
    QM_HDMI_480I,
    QM_HDMI_576I,
    QM_HDMI_1080I_60HZ,
    QM_HDMI_1080I_50HZ,
    QM_480P,
    QM_576P,
    QM_720P,
    QM_1080P_60HZ,
    QM_1080P_50HZ,
    QM_HDMI_480P,
    QM_HDMI_576P,
    QM_HDMI_720P,
    QM_HDMI_1080P_60HZ,
    QM_HDMI_1080P_50HZ,
    QM_PC_mode_Up,
    QM_PC_mode_Down,
    QM_DTV_352X480I,
    QM_DTV_480I,
    QM_DTV_576I,
    QM_DTV_1080I_60HZ,
    QM_DTV_1080I_50HZ,
    QM_DTV_480P,
    QM_DTV_576P,
    QM_DTV_720P,
    QM_DTV_1080P_60HZ,
    QM_DTV_1080P_50HZ,
} SC_QUALITY_MAP_INDEX_e;

typedef enum
{
    QM_AFEC_ind              ,
    QM_Comb_ind              ,
    QM_SECAM_ind             ,
    QM_PreScalingFilter_ind  ,
    QM_CTI_ind               ,
    QM_444To422_ind          ,
    QM_PreCCS_ind            ,
    QM_DNR_ind               ,
    QM_PostCCS_ind           ,
    QM_PNR_ind               ,

    QM_MADi_ind              ,
    QM_EODi_ind              ,
    QM_SmoothFilter_ind      ,
    QM_Film_ind              ,
    QM_MEMC_ind              ,
    QM_PreSNR_ind            ,
    QM_PostScaling_ind       ,
    QM_SRAM1_ind             ,
    QM_SRAM2_ind             ,
    QM_422To444_ind          ,

    QM_Peaking_ind           ,
    QM_SwDriver_ind          ,
    QM_Color_ind             ,
    QM_Display_ind           ,

    QM_ALL                   ,
} EN_Quality_Map_IPTYPE;

typedef enum
{
    BLACK_LEVEL_LOW ,
    BLACK_LEVEL_HIGH,
    BLACK_LEVEL_AUTO ,
    BLACK_LEVEL_NUM
} EN_BLACK_LEVEL;

#if 1
typedef enum {
	BLACK_LEVEL_RF		= 0,			/**< Black for RF 		*/
	BLACK_LEVEL_AV,					/**< Black for AV 		*/
	BLACK_LEVEL_COMP,				/**< Black for Component	*/
	BLACK_LEVEL_RGB,					/**< Black for RGB 		*/
	BLACK_LEVEL_HDMI,				/**< Black for HDMI 		*/
	BLACK_LEVEL_DECODER, 			/**< Black for Decoder type	*/
	BLACK_LEVEL_OTHERS,			/**< Black for others		*/
	BLACK_LEVEL_INVALID				/**< invalid		*/
} EN_BLACK_SOURCE;
#else
typedef enum
{
    BLACK_LEVEL_HDMI ,
    BLACK_LEVEL_COMP ,
    BLACK_LEVEL_DECODER,
    BLACK_LEVEL_OTHERS
} EN_BLACK_SOURCE;
#endif

typedef enum
{
    QM_Check                 ,
    QM_Set                   ,

} EN_Quality_Map_Active;

// MVD Type
typedef enum
{
    MVD_MPGE2,
    MVD_H264,
} SC_MVDTYPE_INDEX_e;


// packet Colorimetry
typedef enum
{
    HDMI_COLORIMETRY_NONE,
    HDMI_COLORIMETRY_ITU601,
    HDMI_COLORIMETRY_ITU709,
    HDMI_COLORIMETRY_EXTEND,
} SC_HDMI_COLORIMETRY_FORMAT_e;

// packet Extended Colorimetry
typedef enum
{
    HDMI_EXTENDEDCOLORIMETRY_XVYCC601,
    HDMI_EXTENDEDCOLORIMETRY_XVYCC709,
    HDMI_EXTENDEDCOLORIMETRY_RESERVED,
} SC_HDMI_EXTENDEDCOLORIMETRY_FORMAT_e;


typedef struct
{
    U8  u8ResIdx;           // Resolution index

    U16 u16HFreq;           // Horizontal frequency
    U16 u16VFreq;           // Vertical frequency

    U16 u16HStart;          // Horizontal start
    U16 u16VStart;          // Vertical start

    U16 u16HTotal;          // Horizontal Total
    U16 u16VTotal;          // Vertical Total

    U8  u8VTotalTolerance;  // VTotal tolerance
    U8  u8AdcPhase;         // ADC phase

    U8  u8StatusFlag;       // Flags
                            // b0: VSync polarity(1/0 = positive/negative)
                            // b1: HSync polarity(1/0 = positive/negative)
                            // b2: Sync polarity care bit
                            // b4: interlace mode
} SC_PCMODE_MODETABLE_t;
//----------------------------------------------------------------
// mode index type
// 091015 ykkim5 index 조정.
//         [중요]    아래는 MAdp_Scaler_pcmode.c 내 table 순서와 동일해야 한다.
//---------------------------------------------------------------
typedef enum
{
    MD_640x480_60   =  4,   // 4
    MD_848x480_59   = 13,   // 13
    MD_1024x768_60  = 14,   // 14
    MD_1280x768_60  = 21,   // 21
    MD_1280x1024_60 = 25,   // 25

    /*++ Component Mode ++*/
    //FitchHsu 20081208 YPbPr index error
    // FitchHsu 20090605 patch Sherwood USA DVD player timing invalid format
#if SC_YPBPR_720x480_60_SW_PATCH
    MD_720x480_60I_P = 47+3+9,
    MD_720x480_60I,         // 60
#else
    MD_720x480_60I= 47+3+9, // 59
#endif
    MD_720x480_60P,         // 61
    MD_720x576_50I,         // 62
    MD_720x576_50P,         // 63
    MD_1280x720_50P,        // 64
    MD_1280x720_60P,        // 65

    MD_1280x768_60_RB = 79, // 79
    MD_STD_MODE_MAX_INDEX,  // 80
} EN_MODE_TYPE;

typedef struct
{
    U16 Addr;
    U8  Mask;
    U8  Value;
} REG_t;

// scaler register table entry
typedef struct
{
    U16 Addr;
    U16 Mask;
    U16 Value;
} SC_REG_t;

// general register table entry
typedef struct
{
    U16 Addr;
    U8  Mask;
    U8  Value;
} GEN_REG_t;

typedef enum
{
    ACE_RED,
    ACE_GREEN,
    ACE_BLUE,
    ACE_CYAN,
    ACE_MAGENTA,
    ACE_YELLOW,
    ACE_FRESH,
} SC_ACE_COLOR_e;

typedef struct
{
    U8 *pIPTable;
    U8 u8TabNums;
    U8 u8TabType;
    U8 u8TabIdx;
} EN_IP_Info;

typedef struct
{
    U8 pIPTable[PQL_AP_DB_MAX_SIZE];
    U8 u8TabNums;
    U8 u8TabType;
    U8 u8TabIdx;
} EN_IP_Info_AP;

typedef enum
{
	FRAME_LOCK_MODE,
	FREE_RUN_24HZ,
	FREE_RUN_50HZ,
	FREE_RUN_60HZ,
} SC_FL_MODE_e;

//thchen 20080925
typedef enum
{
    DYNAMIC_DCLK,
    CONSTANT_DCLK,
    FREERUN_MODE,
} SC_FRAMELOCK_MODE_e;

//thchen 20080925
typedef enum
{
    FREERUN_24HZ,
    FREERUN_50HZ,
    FREERUN_60HZ,
} SC_FREERUN_MODE_e;

// HDMI EQ Level
typedef enum
{
    EQ_NORMAL,
	EQ_MAXIMUM,
	EQ_MINIMUM,
	EQ_NORMAL_HIGH,
} SC_HDMI_EQ_e;	// 081027 wlgnsl99 LGE : set HDMI EQ

// FitchHsu 20080811 implement LPLL type
typedef enum
{
    LPLL_LVDS,
    LPLL_RSDS,
    LPLL_TTL,
    LPLL_MINILVDS, // shjang_090904
} SC_MOD_LPLL_TYPE_e;

/// LPLL type
typedef enum
{
    LPLL_TYPE_LVDS = 0,
    LPLL_TYPE_RSDS = 1,
    LPLL_TYPE_TTL  = 2,
    // shjang_090904
    LPLL_TYPE_MINILVDS	 = 3,
} LPLL_TYPE_t;

//==========================================================================
// IOCTL_SC_SET_NO_SIGNAL_COLOR
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8   u8Color;
} SC_SET_NO_SIGNAL_COLOR_t;

// IOCTL_SC_ACTIVE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8Reserved;
} SC_ACTIVE_t;

// IOCTL_SC_SET_INPUT_SOURCE_ENABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_INPUT_SOURCE_ENABLE_t;

// IOCTL_SC_SET_INPUTSOURCE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_INPUT_SOURCE_e inputSrc;
} SC_SET_INPUTSOURCE_t;

// IOCTL_SC_SET_MVD_SIGNAL_INFO
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bInterlace;
    U16  u16VFreq;
    U16  u16HActive;
    U16  u16VActive;
} SC_SET_MVD_SIG_INFO_t;

// IOCTL_SC_SET_VD_SIGNAL_INFO
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8  u8VideoSystem;
    U16 u16HFreq; // LGE drmyung 081008
    U16 u16VFreq; // LGE drmyung 081008
    U16 u16HTotal;
    U16 u16VTotal;
} SC_SET_VD_SIG_INFO_t;

// IOCTL_SC_SET_SC_SIGNAL_INFO // LGE drmyung 081022
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16HFreq; // LGE drmyung 081008
    U16 u16VFreq; // LGE drmyung 081008
    U16 u16HTotal;
    U16 u16VTotal;
} SC_SET_SC_SIG_INFO_t;

// IOCTL_SC_SET_CAPTURE_WIN
// IOCTL_SC_GET_CAPTURE_WIN
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16HStart;
    U16 u16HSize;
    U16 u16VStart;
    U16 u16VSize;
} SC_CAPTURE_WIN_t;
// IOCTL_SC_SET_GOP_TO_IP
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;

    U8 gop;
    U8 channel;
    BOOL enable;
} SC_SET_GOP_TO_IP_t;

// IOCTL_SC_SET_GOP_TO_VOP
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;

    U8 gop;
    BOOL enable;
} SC_SET_GOP_TO_VOP_t;


// IOCTL_SC_DLCINIT
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;

    U16 u16Histogram_Vstart;  //thchen 20080708
    U16 u16Histogram_Vend; //thchen 20080708
} SC_DLCINIT_t;

// IOCTL_SC_SET_MVDTYPE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16mvdtype;

} SC_SETMVDTYPE_t;

#if 0//victor 20081106
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;

    EN_BLACK_SOURCE eBlackSource;
    EN_BLACK_LEVEL eBlackLevel;
    U8 u8HDMIisDTV;
} SC_SET_BLACKLEVEL_t;
#endif
// IOCTL_SC_SET_DISPLAY_WIN
// IOCTL_SC_GET_DISPLAY_WIN
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16HStart;
    U16 u16HSize;
    U16 u16VStart;
    U16 u16VSize;
} SC_DISPLAY_WIN_t;

// IOCTL_SC_SET_BLACK_SCREEN
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
    U16 u16VFreq;
    U8 u8FrameLock;  // 20090822 FitchHsu fix Ursa ATV snow noise frame lock
} SC_SET_BLACK_SCREEN_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16tconpanelIdx;
} SC_TCON_MAP_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16tconpanelIdx;
    U16 u16tabtype;
    U32 u32Tabsize;
    U8  u8TconTab[TCON_TAB_MAX_SIZE];
} SC_TCON_TAB_INFO_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16tconpanelIdx;
    BOOL benable;
    SC_POWER_SEQUENCE_SIGNAL_TYPE u8Tcontype;
} SC_TCON_POW_SEQ_INFO_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_TCON_COUNT_RESET_t;

// VD video system
typedef enum
{
    VD_PAL_BGHI        = 0x00,
    VD_NTSC_M          = 0x01,
    VD_SECAM           = 0x02,
    VD_NTSC_44         = 0x03,
    VD_PAL_M           = 0x04,
    VD_PAL_N           = 0x05,
    VD_PAL_60          = 0x06,
    VD_NOTSTANDARD     = 0x07
} VD_VIDEOSYSTEM_e;

typedef enum
{
	FRAME_LOCK_50HZ = 0,
	FRAME_LOCK_60HZ = 1,
	FREE_RUN        = 2
} FRAME_LOCK_e;

// IOCTL_SC_SET_BRIGHTNESS
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16Brightness;//change U8 to U16, [090921_Leo]
} SC_SET_BRIGHTNESS_t;

// IOCTL_SC_SET_FRAMERATE
// IOCTL_SC_GET_FRAMERATE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
	BOOL bIsForceFreeRun;
    U16 u16FrameRate;
} SC_FRAMERATE_t;

// IOCTL_SC_GET_INPUTTIMINGINFO
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;

    U16 hFreq;      // Horizontal frequency(100 Hz unit)
    U16 vFreq;      // Veritical frequency(1/10 Hz unit)
    U16 hTotal;     // Horizontal total pixels
    U16 vTotal;     // Vertical total lines
    U16 hStart;     // Horizontal start pixels
    U16 vStart;     // Vertical start lines
    U16 hActive;    // Horizontal active pixel
    U16 vActive;    // Vertical active lines
    U16 scanType;   // Scan type (0 : interlace, 1 : progressive)
    U16 format;     // Video format. It is one of the VID_FMT_T enumeration.
    U8  phase;      // Sampling phase
    U8  mode;       // Mode.(0 : graphic, 1 : SD video, 2 : HD video, 3 : Extended Definition video :480p/576p)
} SC_GET_INPUT_TIMING_INFO_t;

#if 1 // LGE drmyung 080903
typedef struct
{
	U8	u8VgaAMux;
	U8	u8YPbPr1AMux;
	U8	u8YPbPr2AMux;
	U8	u8AtvYMux;
	U8	u8Cvbs1YMux;
	U8	u8Cvbs2YMux;
	U8	u8Cvbs3YMux;
	U8	u8SVideo1YMux;
	U8	u8SVideo1CMux;
	U8	u8SVideo2YMux;
	U8	u8SVideo2CMux;
    U8  u8Scart1AMux;
	U8	u8Scart1YMux;
    U8  u8Scart2AMux;
	U8	u8Scart2YMux;
} SC_INPUT_MUX_t;
#endif
// IOCTL_SC_DUMP_SC_REGTABLE
typedef struct
{
    SC_REG_t* pRegTable;
    U32 u32TableSize;
} SC_DUMP_REGTABLE_t;

// IOCTL_SC_DUMP_SC_REGTABLE_2
typedef struct
{
    REG_t* pRegTable;
    U32 u32TableSize;
} SC_DUMP_REGTABLE_2_t;

// IOCTL_SC_DUMP_GEN_REGTABLE
typedef struct
{
    GEN_REG_t* pRegTable;
    U32 u32TableSize;
} SC_DUMP_GEN_REGTABLE_t;

// IOCTL_SC_WRITEREGISTER
typedef struct
{
    U8  ScBank;
    U8  Index;
    U16 Value;
} SC_WRITE_REG_t;

// IOCTL_SC_READEGISTER
typedef struct
{
    U16 ScBank;
    U8  Mask;
    U8  Value;
} SC_READ_REG_t;

// IOCTL_SC_SET_GAMMA_TABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8* pu8Gamma_R;
    U8* pu8Gamma_G;
    U8* pu8Gamma_B;
    U16 u16NumOfLevel;
} SC_SET_GAMMA_TABLE_t;

// IOCTL_SC_SET_FILM_MODE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_FILM_MODE_t;

// IOCTL_SC_SET_WB_TEST_PATTERN
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
    BOOL bIs216;
} SC_SET_WB_TEST_PATTERN_t;

// IOCTL_SC_SET_WB_TEST_PATTERN
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8PatternType;		// lemonic LGE 080902	: Type changed. BOOL -> U8
    U8 u8Pattern;
} SC_SET_TEST_PATTERN_t;
// IOCTL_SC_SET_FRAMECOLOR_TABLE
typedef struct
{
    U8  u8FrameColorR;
    U8  u8FrameColorG;
    U8  u8FrameColorB;
} SC_SET_FRAMECOLOR_t;

// IOCTL_SC_SET_CROPWIN_ENABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bCropWinEnable;
} SC_SET_CROPWIN_ENABLE_t;

// IOCTL_SC_SET_CROPWIN
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16H_CropStart;
    U16 u16H_CropSize;
    U16 u16V_CropStart;
    U16 u16V_CropSize;
} SC_SET_CROPWIN_t;

// IOCTL_SC_GET_CROPWIN
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16H_CropStart;
    U16 u16H_CropSize;
    U16 u16V_CropStart;
    U16 u16V_CropSize;
} SC_GET_CROPWIN_t;

// IOCTL_SC_PQDUMP_SC_REGTABLE
typedef struct
{
    EN_IP_Info* pIPTable;
    U8 u8Iptpye;
    U8 u8SRAM_num;
} SC_PQ_DUMP_FILTERREGTABLE_t;

//FitchHsu 20081113 EMP when PAUSE, little shaking
// IOCTL_SC_PQ_FASTPLAYBACK
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_PQ_FASTPLAYBACK_t;

//FitchHsu 20081119 EMP preview setting for 24P and 30P
// IOCTL_SC_PQ_FASTPLAYBACK
// FitchHsu20090119 EMP preview setting error
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
    BOOL b4FrameEnable;
} SC_PQ_EMPPREVIEW_t;

//victor 20090108, add emp video input source
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_PQ_EMPPlayingVideo_t;

//[090910_Leo]
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_PQ_EMPJPEG_t;

// FitchHsu 20080811 implement LPLL type
//  IOCTL_SC_SET_LPLL
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_MOD_LPLL_TYPE_e e_lpll_type;
} SC_SET_LPLL_TYPE_t;

//------------------------------------------------------------------------------
//  PC Mode
//------------------------------------------------------------------------------
// IOCTL_SC_SET_MODETABLE
typedef struct
{
    U32* pModeTable;
    U32  u32TableSize;
    U8   u8TableCount;
} SC_SET_MODETABLE_t;

typedef enum
{
    RES_640X350,    // 00
    RES_640X400,    // 01
    RES_720X400,    // 02
    RES_640X480,    // 03
    RES_800X600,    // 04
    RES_832X624,    // 05
    RES_1024X768,   // 06
    RES_1280X1024,  // 07
    RES_1600X1200,  // 08
    RES_1152X864,   // 09
    RES_1152X870,   // 10
    RES_1280X768,   // 11
    RES_1280X960,   // 12
    RES_720X480,    // 13
    RES_1920X1080,  // 14

    RES_1280X720,   // 15
    RES_720X576,    // 16

    RES_1920X1200,  // 17

    RES_1400X1050,  // 18
    RES_1440X900,   // 19
    RES_1680X1050,  // 20

    RES_1280X800,   // 21
    RES_1600X1024,  // 22
    RES_1600X900,   // 23
    RES_1360X768,   // 24
    RES_848X480,    // 25
    RES_1920X1080P, // 26

    RES_1366X768,   // 27
    RES_864X648,    // 28

    RES_MAXIMUM
} MADP_SC_RESOLUTION_TYPE_e;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    MADP_SC_RESOLUTION_TYPE_e resolution;
} SC_SET_RESOLUTION_t;

/// PC ADC mode Handle
typedef enum
{
    MODE_HANDLE_CHECK_SYNC,
    MODE_HANDLE_HAVE_SYNC,
    MODE_HANDLE_NO_SYNC,

    MODE_HANDLE_SEARCH,
    MODE_HANDLE_SEARCH_READY,
    MODE_HANDLE_SEARCH_FAIL,

    MODE_HANDLE_SET_MODE,
    MODE_HANDLE_SET_READY,
    MODE_HANDLE_SET_FAIL,

    MODE_HANDLE_SUPPORT,
    MODE_HANDLE_UNSUPPORT,
} EN_MODE_HANDLE_EVENT;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    // swwoo LGE 080626 : match to type for LG DDI
    S16  s16PC_H_POS;
    S16  s16PC_V_POS;
    //U16  u16PC_H_SIZE;
    //U16  u16PC_V_SIZE;
    U8   u8Phase;
    //S8   s8Clock;
    U8   u8ModeIdx;
    S16  s16PC_H_TOTAL;
} SC_PCMODE_INFO_t;

//------------------------------------------------------------------------------
//  HDMI
//------------------------------------------------------------------------------
// IOCTL_SC_GET_HDMI_INFO
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bHDMI;
    BOOL bAudio;
    U16  u16HStart;
    U16  u16HSize;
    U16  u16VStart;
    U16  u16VSize;
} SC_GET_HDMI_INFO_t;

typedef enum
{
    HDMI_AR_NONE,
    HDMI_AR_4_3,
    HDMI_AR_16_9,
    HDMI_AR_FUTURE,
} SC_HDMI_ASPECTRATIO_e;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_HDMI_ASPECTRATIO_e u8AspectRatio;
} SC_GET_HDMI_ASPECTRATIO_t;

// IOCTL_SC_GET_HDMI_XVYCC
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_HDMI_COLORIMETRY_FORMAT_e eColorimetry;
    SC_HDMI_EXTENDEDCOLORIMETRY_FORMAT_e eExtColorimetry;
    U8 GM_GBD[(GM_GBD_MAX_SIZE+0x1) & ~0x1];        // Currently, support up to 21 bytes GBD

} SC_GET_HDMI_XVYCC_t;

// CC Chen 20081124 MWE implement
//---------------------------------------------------
// MWE
typedef enum
{
    SC_MWE_OFF,
    SC_MWE_H_SPLIT,
    SC_MWE_MOVE,
    SC_MWE_ZOOM,
    SC_MWE_NUMS,
} SC_MWE_TYPE_e;
// IOCTL_SC_SET_HDMI_HDCP // LGE wlgnsl99
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8HdcpKey[289];
} SC_SET_HDMI_HDCP_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 colorFormat;
} SC_GET_HDMI_COLOR_DOMAIN_t;   //victor 20080910
//------------------------------------------------------------------------------
//  Scart
//------------------------------------------------------------------------------
// scart mode
typedef enum
{
    SCART_MODE_RGB,
    SCART_MODE_CVBS,
    SCART_MODE_SVIDEO,
} SC_SCART_MODE_e;

typedef enum
{
    SCART_AR_4_3,
    SCART_AR_16_9,
    SCART_AR_INVALID,
} SC_SCART_AR_e;

typedef struct
{
    SC_INPUT_SOURCE_e scartIdx;
    SC_SCART_AR_e scartAR;
} SC_GET_SCART_AR_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bOverlay;
} SC_SET_SCART_OVERLAY_t;

//------------------------------------------------------------------------------
//  MACE
//------------------------------------------------------------------------------
// IOCTL_SC_SET_YUV2RGB_MTX
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    S16* pYUV2RGB_Matrix;
} SC_SET_YUV2RGB_MTX_t;

// IOCTL_SC_SET_CONTRAST
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8Contrast;
} SC_SET_CONTRAST_t;

// IOCTL_SC_SET_SATURATION
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8Saturation;
} SC_SET_SATURATION_t;

// IOCTL_SC_SET_HUE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8Hue;
} SC_SET_HUE_t;

// IOCTL_SC_SET_RGB
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8Red;
    U8 u8Green;
    U8 u8Blue;
} SC_SET_RGB_t;

// IOCTL_SC_SET_RGB_EX
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16Red;
    U16 u16Green;
    U16 u16Blue;
} SC_SET_RGB_EX_t;

// IOCTL_SC_SET_SHARPNESS
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8Sharpness;
} SC_SET_SHARPNESS_t;

// IOCTL_SC_GET_HISTOGRAM_INFO
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bDataReady;
    U16  u16Histogram32[32];
    U8   u8MaxPixelValue;
    U8   u8MinPixelValue;
    U8   u8AvgPixelValue;
    U16  u16TotalColorCount;//[090601_Leo]
} SC_GET_HISTOGRAM_INFO_t;

// IOCTL_SC_SET_LUMA_CURVE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16LumaCurve[16];
} SC_SET_LUMA_CURVE_t;

// IOCTL_SC_SET_LUMA_CURVE_ENABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_LUMA_CURVE_ENABLE_t;

// IOCTL_SC_SET_HISTOGRAM_REQ_ENABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_HISTOGRAM_REQ_ENABLE_t;

// IOCTL_SC_SET_ICC_SATURATION_ADJ
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_ACE_COLOR_e colorType;
    S8 s8SatAdj;
} SC_SET_ICC_SATURATION_ADJ_t;

// IOCTL_SC_SET_IBC_Y_ADJ
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_ACE_COLOR_e colorType;
    U8 u8YAdj;
} SC_SET_IBC_Y_ADJ_t;

// IOCTL_SC_SET_IHC_HUE_COLOR_DIFF_ADJ
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_ACE_COLOR_e colorType;
    S8 s8HueAdj;
    U8 u8YIndex;
    U8 u8YLevel;
} SC_SET_IHC_HUE_COLOR_DIFF_ADJ_t;

// IOCTL_SC_SET_IHC_HUE_ADJ
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_ACE_COLOR_e colorType;
    S8 s8HueAdj;
} SC_SET_IHC_HUE_ADJ_t;

// IOCTL_SC_SET_ICC_SATURATION_ENABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_ICC_SATURATION_ENABLE_t;//thchen 20080718

// IOCTL_SC_SET_IBC_Y_ENABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_IBC_Y_ENABLE_t;//thchen 20080718

// IOCTL_SC_SET_IHC_HUE_ENABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_IHC_HUE_ENABLE_t;//thchen 20080718

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8  u8CalType;
    U16 u16TargetForRGain;    // 20091012 daniel.huang: for finetune internal calibration
    U16 u16TargetForGGain;    // 20091012 daniel.huang: for finetune internal calibration
    U16 u16TargetForBGain;    // 20091012 daniel.huang: for finetune internal calibration
    BOOL bAutoResult;
} SC_ADC_CAL_INFO_t;//thchen 20080729

typedef struct
{
    U16 u16RGainValue;
    U16 u16GGainValue;
    U16 u16BGainValue;
} SC_GAIN_VALUE_t;//daniel.huang 20090406 for T3 [13:0]

typedef struct
{
    U16 u16ROffsetValue;
    U16 u16GOffsetValue;
    U16 u16BOffsetValue;
} SC_OFFSET_VALUE_t;//daniel.huang 20090406 for T3 [12:0]

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8CbUpValue;
    U8 u8CbDownValue;
    U8 u8CrUpValue;
    U8 u8CrDownValue;
} SC_COLOR_RANGE_t;//[090601_Leo] for T3

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 bOnOff;
} SC_SET_ADAPTIVE_CGAIN_EN_t; //[090921_Leo] for T3

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 pCGainParam[10];//LG request change 12 -> 10 elements, [090921_Leo]
} SC_SET_ADAPTIVE_CGAIN_t;//[090814_Leo] for T3

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8OnOff;
    U8 pPieceWiseXPosition[16];
} SC_SET_PIECEWISE_ENABLE_t;//[090824_Leo] for T3

typedef struct
{
    U8 u8Data[256];
} SC_ICC_REGION_t;//victor 20080814

typedef struct
{
    U8 u8Data[289];
} SC_IHC_REGION_t;//victor 20080814

typedef struct
{
    U16 u16HSizeBeforePrescaling;
    U16 u16HSizeAfterPrescaling;
    U32 table;
} SC_Decide_HPre_ScalingFilter_t;//victor 20080821

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_GAMMA_ENABLE_t;//victor 20080830

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_BLUE_STRETCH_ENABLE_t;//victor 20080830

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_CSC_OFFSET_ENABLE_t;//victor 20080830

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_CSC_ENABLE_t;//victor 20080909

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_CDNR_ENABLE_t;//[090615_Leo]

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 *pIndex;
} SC_SET_CDNR_INDEX_t;//[090616_Leo]

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 *pDnrGain;
    U8 *pPreSnrGain;
} SC_SET_CDNR_GAIN_t;//[090617_Leo]

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 bOnOff;
} SC_SET_AUTO_NR_ENABLE_t;//[090617_Leo]

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bIsHDMI;
} SC_IS_HDMI_t;//victor 20080923

// IOCTL_SC_SET_HDMI_EQ
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8EqLevel;
} SC_SET_HDME_EQ_t;		// 081027 wlgnsi99 LGE : set HDMI EQ
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8ThLv1;
    U8 u8ThLv2;
    U8 u8ThLv3;
    U8 u8ThLv4;
    U8 u8ThLv5;//[091201_Leo]
} SC_SET_DEFEATHER_TH_t;//victor 20080923

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8Index;
} SC_SET_BLACKLEVEL_t;    //victor 20081106

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bBrightnessEn;
    BOOL bContrastEn;
    BOOL bNoiseRoundEn;
    U8  u8R_Gain;
    U8  u8G_Gain;
    U8  u8B_Gain;
    U8  u8R_Offset;
    U8  u8G_Offset;
    U8  u8B_Offset;
} SC_SET_CONBRI_t;      //victor 20081016, ContrastBrightness

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bBrightnessEn;
    BOOL bContrastEn;
    BOOL bNoiseRoundEn;
    U16  u16R_Gain;
    U16  u16G_Gain;
    U16  u16B_Gain;
    U16  u16R_Offset;
    U16  u16G_Offset;
    U16  u16B_Offset;
} SC_SET_CONBRI16_t;      //victor 20081016, ContrastBrightness

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 u16_24H;
    U16 u16_2FH;
    U16 u16_30L;
    U16 u16_35H;
    U16 u16_36H;
    U16 u16_37L;
    U16 u16_41H;
    U16 u16_60H;
    U16 u16_61L;
    U16 u16_61H;
    U16 u16_76H;

} SC_SET_3DCOMB_t;      //victor 20081113, 3DComb

typedef struct
{
	U16 u16Periodx100Hz;
	U16 u16Percentx100;
	BOOL bEnable;
} SC_SET_LVDS_SSC_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_ENABLE_DNR_t;//victor 20081203, DNR

// FitchHsu 20081209 implement THX mode
typedef struct
{
	SC_WINDOW_IDX_e srcIdx;
	BOOL bIsTHXMode;
} SC_SET_THX_t;


/******************************************************************************
	LGE IOCTL : 240 ~ 254 (추가 자제할 것)
*******************************************************************************/
// IOCTL_SC_SET_COLOR_WASH_ENABLE
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
} SC_SET_COLOR_WASH_ENABLE_t;	// LGE [vivakjh] 2008/12/09 	For setting the PDP's Color Wash
// shjang_091006 (20091006 ykkim5) if 3 kinds of mode is used. this should be released
/*
typedef struct
{
	U8 u8480;
	U8 u8720;
	U8 u81080;
}SC_SOG_SLICE_LEVEL_t;
*/

/* HDMI VSI Packet Type */
#define HDMI_VSI_PACKET_DATA_LENGTH 28
#define HDMI_VSI_INFOFRAME_PACKET_LEN 28
#define HDMI_VSI_VENDOR_SPECIFIC_CHECKSUM_LEN 1
#define HDMI_VSI_VENDOR_SPECIFIC_REGID_LEN 3
#define HDMI_VSI_VENDOR_SPECIFIC_PAYLOAD_LEN \
    (HDMI_VSI_INFOFRAME_PACKET_LEN - HDMI_VSI_VENDOR_SPECIFIC_REGID_LEN - HDMI_VSI_VENDOR_SPECIFIC_CHECKSUM_LEN)


typedef enum
{
    HDMI_VSI_PACKET_STATUS_NOT_RECEIVED = 0, /* initial state */
    HDMI_VSI_PACKET_STATUS_STOPPED,
    HDMI_VSI_PACKET_STATUS_UPDATED,
    HDMI_VSI_PACKET_STATUS_MAX
} HDMI_PACKET_STATUS_e;

typedef struct
{
    U8 type;            /* packet type */
    U8 version;         /* packet version     */
    U8 length;          /* packet length      */
    U8 dataBytes[HDMI_VSI_PACKET_DATA_LENGTH]; /* packet data */
} HDMI_PACKET_t;

typedef struct
{
    U8 IEERegId[HDMI_VSI_VENDOR_SPECIFIC_REGID_LEN];
    U8 PayLoad[HDMI_VSI_VENDOR_SPECIFIC_PAYLOAD_LEN];
    HDMI_PACKET_STATUS_e packetStatus;
    HDMI_PACKET_t packet;
} SC_HDMI_VSI_PACKET_t;

//  IOCTL_SC_GET_HDMI_VSI
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_HDMI_VSI_PACKET_t vsi_packet;
} SC_GET_HDMI_VSI_PACKET_t;


typedef enum
{
    HDMI_AVI_CSC_eRGB = 0,
    HDMI_AVI_CSC_eYCbCr422,
    HDMI_AVI_CSC_eYCbCr444,
    HDMI_AVI_CSC_eFuture
} HDMI_AVI_CSC;

typedef enum
{
    HDMI_AVI_ACTIVE_INFO_eInvalid = 0,
    HDMI_AVI_ACTIVE_INFO_eValid
} HDMI_AVI_ACTIVE_INFO;

typedef enum
{
    HDMI_AVI_BAR_INFO_eInvalid,
    HDMI_AVI_BAR_INFO_eVerticalValid,
    HDMI_AVI_BAR_INFO_eHorizValid,
    HDMI_AVI_BAR_INFO_eVertHorizValid
} HDMI_AVI_BAR_INFO;


typedef enum
{
    HDMI_AVI_SCAN_INFO_eNoData,
    HDMI_AVI_SCAN_INFO_eOverScanned,
    HDMI_AVI_SCAN_INFO_eUnderScanned
} HDMI_AVI_SCAN_INFO;

typedef enum
{
    HDMI_AVI_COLORIMETRY_eNoData =  0,
    HDMI_AVI_COLORIMETRY_eSmpte170,
    HDMI_AVI_COLORIMETRY_eItu709,
    HDMI_AVI_COLORIMETRY_eFuture,
    //HDMI_AVI_COLORIMETRY_eExtended = VIDEO_HDMI_AVI_COLORIMETRY_eFuture

} HDMI_AVI_COLORIMETRY;

typedef enum
{
    HDMI_AVI_PICTURE_ARC_eNoData = 0,
    HDMI_AVI_PICTURE_ARC_e4_3,
    HDMI_AVI_PICTURE_ARC_e16_9,
    HDMI_AVI_PICTURE_ARC_eFuture
} HDMI_AVI_PICTURE_ARC;

typedef enum
{
    HDMI_AVI_ACTIVE_FORMAT_ARC_ePicture    =  8,
    HDMI_AVI_ACTIVE_FORMAT_ARC_e4_3Center  =  9,
    HDMI_AVI_ACTIVE_FORMAT_ARC_e16_9Center = 10,
    HDMI_AVI_ACTIVE_FORMAT_ARC_e14_9Center = 11,
    HDMI_AVI_ACTIVE_FORMAT_ARC_eOther      =  0
} HDMI_AVI_ACTIVE_FORMAT_ARC;

typedef enum
{
    HDMI_AVI_SCALING_eNoScaling = 0,
    HDMI_AVI_SCALING_eHScaling,
    HDMI_AVI_SCALING_eVScaling,
    HDMI_AVI_SCALING_eHVScaling
} HDMI_AVI_SCALING;

typedef enum
{
    HDMI_AVI_EXT_COLORIMETRY_exvYCC601 = 0,
    HDMI_AVI_EXT_COLORIMETRY_exvYCC709 ,
    HDMI_AVI_EXT_COLORIMETRY_exvReserved
} HDMI_AVI_EXT_COLORIMETRY;

typedef enum
{
    HDMI_AVI_RGB_QUANTIZATION_RANGE_eDefault    =  0,
    HDMI_AVI_RGB_QUANTIZATION_RANGE_eLimitedRange  =  1,
    HDMI_AVI_RGB_QUANTIZATION_RANGE_eFullRange = 2,
    HDMI_AVI_RGB_QUANTIZATION_RANGE_eReserved = 3
} HDMI_AVI_RGB_QUANTIZATION_RANGE;

typedef enum
{
    HDMI_AVI_IT_CONTENT_eNoData    =  0,
    HDMI_AVI_IT_CONTENT_eITContent  =  1
} HDMI_AVI_IT_CONTENT;

typedef enum
{
    HDMI_GamutFormat_eVerticesFacets,
    HDMI_GamutFormat_eRange
} HDMI_GamutFormat;


typedef enum
{
    HDMI_GamutColorPrecision_e8Bit,
    HDMI_GamutColorPrecision_e10Bit,
    HDMI_GamutColorPrecision_e12Bit,
    HDMI_GamutColorPrecision_eUnknown
} HDMI_GamutColorPrecision;


typedef enum
{
    HDMI_GamutColorSpace_eItu709RGB,
    HDMI_GamutColorSpace_exvYCC601SD,
    HDMI_GamutColorSpace_exvYCC709HD,
    HDMI_GamutColorSpace_eXZY /* not supported */
} HDMI_GamutColorSpace;

typedef struct
{
    HDMI_AVI_CSC ePixelEncoding;

    HDMI_AVI_ACTIVE_INFO eActiveInfo; /* A0 */
    HDMI_AVI_BAR_INFO    eBarInfo;    /* B1B0 */
    HDMI_AVI_SCAN_INFO   eScanInfo;   /* S1S0 */

    HDMI_AVI_COLORIMETRY eColorimetry;  /* C1C0 */

    HDMI_AVI_PICTURE_ARC ePictureAspectRatio; /* M1M0 */

    HDMI_AVI_ACTIVE_FORMAT_ARC eActiveFormatAspectRatio; /* R3R0 */

    HDMI_AVI_SCALING eScaling; /* SC1SC0 */

    U8 VideoIdCode; /* VICn */

    U8 PixelRepeat;

    HDMI_AVI_IT_CONTENT eITContent; /*ITC */

    HDMI_AVI_EXT_COLORIMETRY eExtendedColorimetry; /* EC2EC1EC0 */

    HDMI_AVI_RGB_QUANTIZATION_RANGE eRGBQuantizationRange; /* Q1Q0 */

    /* bar info */
    U16 TopBarEndLineNumber;
    U16 BottomBarStartLineNumber;

    U16 LeftBarEndPixelNumber;
    U16 RightBarEndPixelNumber;
    HDMI_PACKET_STATUS_e packetStatus;
    HDMI_PACKET_t packet;
} SC_HDMI_AVI_PACKET_t;

//  IOCTL_SC_GET_HDMI_AVI
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    SC_HDMI_AVI_PACKET_t avi_packet;
} SC_GET_HDMI_AVI_PACKET_t;


typedef struct
{
    U8 B_Cb;
    U8 G_Y;
    U8 R_Cr;
}
PIXEL_24BIT;

typedef struct
{
    U32 B_Cb   :10;
    U32 G_Y    :10;
    U32 R_Cr   :10;
    U32 u8Dummy:2;
}
PIXEL_32BIT;


// IOCTL_SC_GET_FRAMEDATA
// IOCTL_SC_SET_FRAMEDATA
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U16 x0, y0, width, height;
    BOOL bRGB;                 // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    U8 *pRect;
    U32 u32RectPitch;
    U16 u16PointSize;
} SC_FRAMEDATA_t;

typedef struct
{
    U32 u32IPMBase0;
    U32 u32IPMBase1;
    U32 u32IPMBase2;
    U32 u32OPMBase0;    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    U32 u32OPMBase1;    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    U32 u32OPMBase2;    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    U16 u16IPMOffset;
    U16 u16IPMFetch;    // 20090922 daniel.huang: for memory protection
    U16 u16FrameLineCnt;// 20090921 daniel.huang: for mirror mode
    U16 u16LBOffset;    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    BOOL bMemFormat422;
    BOOL bInterlace;
    U8 u8BitPerPixel;
    BOOL bLinearAddrMode;
    BOOL bYCSeperate;
    BOOL bh_mirror;     // 20090921 daniel.huang: for mirror mode
    BOOL bv_mirror;     // 20090921 daniel.huang: for mirror mode
} SC_FRAMEBUF_INFO_t;


typedef struct
{
    U8 u8PortNum;
    U8 u8Param;
} SC_CVBS_OUT_INFO_t;
//lachesis_090723 HPD control
typedef struct
{
    U8 u8HDMIPort;
    BOOL bEnable;
} SC_HDMI_HPD_t;

//------------------------------------------------------------------------------
// VIDEO MIRROR
// Michu 20090903
//------------------------------------------------------------------------------
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bIsH;
    BOOL bIsV;
} SC_SET_MIRROR_TYPE_t;
//------------------------------------------------------------------------------
// End of VIDEO MIRROR
//------------------------------------------------------------------------------

// 20091021 daniel.huang: add ipmux test pattern for inner test pattern
typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    BOOL bEnable;
    U16 u16R_Cr;
    U16 u16G_Y;
    U16 u16B_Cb;
} SC_SET_IPMUX_PATTERN_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    const U8* pODTbl;
    BOOL bEnable;
} SC_SET_OD_TABLE_t;

typedef struct
{
    SC_WINDOW_IDX_e srcIdx;
    U8 u8BitMode;
    BOOL bUrsapatch;
} SC_SET_DITHERING_t;

#endif//__DRV_SCLAER_ST_H__

