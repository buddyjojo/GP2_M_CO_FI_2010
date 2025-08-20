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
/// @file   drvHVD.h
/// @brief  HVD Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_HVD_H_
#define _DRV_HVD_H_

#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define MSIF_HVD_TAG                          {'M','S','I','F'}    //Version ID header
#define MSIF_HVD_CLASS                        {'1','0'}            //info Class code
#define MSIF_HVD_CUS                          0x6666               //Customer ID
#define MSIF_HVD_MOD                          0x0001              //Module ID
#define MSIF_HVD_CHIP                         0x000F              //CHIP ID: T3
#define MSIF_HVD_CPU                          '1'                  //CPU
#define MSIF_HVD_LIB_CODE                     {'H','V','D','_'}    //Lib code
#define MSIF_HVD_LIBVER                       {'0','7'}            //LIB version
#define MSIF_HVD_BUILDNUM                     {'8','6'}            //Build Number
#define MSIF_HVD_CHANGELIST                   {'0','0','2','3','2','9','3','9'} //P4 ChangeList Number
#define MSIF_HVD_OS                           '0'                  //OS

#define HVD_DRV_VERSION                       /* Character String for DRV/API version             */  \
    MSIF_HVD_TAG,                             /* 'MSIF'                                           */  \
    MSIF_HVD_CLASS,                           /* '00'                                             */  \
    MSIF_HVD_CUS,                             /* 0x0000                                           */  \
    MSIF_HVD_MOD,                             /* 0x0000                                           */  \
    MSIF_HVD_CHIP,                                                                                  \
    MSIF_HVD_CPU,                                                                                   \
    MSIF_HVD_LIB_CODE,                        /* IP__                                             */  \
    MSIF_HVD_LIBVER,                          /* 0.0 ~ Z.Z                                        */  \
    MSIF_HVD_BUILDNUM,                        /* 00 ~ 99                                          */  \
    MSIF_HVD_CHANGELIST,                      /* CL#                                              */  \
    MSIF_OS

#define HVD_INIT_HW_MASK                             BMASK(1:0)    ///< HW Type
    #define HVD_INIT_HW_AVC                             BITS( 1:0,0)    ///< HW deflaut: AVC 0X00
    #define HVD_INIT_HW_AVS                             BITS( 1:0,1)    ///< HW : AVS 0X01
    #define HVD_INIT_HW_RM                             BITS( 1:0,2)    ///< HW: RM 0X10
#define HVD_INIT_MAIN_MASK                          BMASK(3:2)    ///< main type
    #define HVD_INIT_MAIN_FILE_RAW   BITS( 3:2,0)              ///< main type: default: 0X00
    #define HVD_INIT_MAIN_FILE_TS   BITS( 3:2,1)    ///< main type: 0X01
    #define HVD_INIT_MAIN_LIVE_STREAM    BITS( 3:2,2)    ///< main type: 0X10
#define HVD_INIT_INPUT_MASK    BMASK(4:4)    ///< process path for filling BBU table:  file mode. use drive; TSP: use tsp mode
    #define HVD_INIT_INPUT_TSP    BITS( 4:4,0)    ///< tsp input( default)
    #define HVD_INIT_INPUT_DRV    BITS( 4:4,1)     ///< driver input
#define HVD_INIT_START_CODE_MASK    BMASK(5:5)    ///< AVC FILE MODE ONLY: mkv, mp4 container use.
    #define HVD_INIT_START_CODE_REMAINED    BITS( 5:5,0)    ///< start code remained.(Defualt)
    #define HVD_INIT_START_CODE_REMOVED    BITS( 5:5,1)    ///< start code removed.
#define HVD_INIT_UTOPIA_ENVI            BIT( 6)        ///< check MIU sel and set it
#define HVD_INIT_DBG_FW            BIT( 7)        ///< check FW is debug version or not
//#define HVD_INIT_ENABLE_ISR_DISP            BIT( 8)        ///< enable display ISR. ISR occurs at every Vsync.

#define HVD_RV_BROKEN_BY_US_MASK        0x00800000

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef void (*HVD_InterruptCb)(void);

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Result
/// @brief \b Enum \b Description:  General result of HVD functions
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_FAIL=0,       ///< General fail cases.
    E_HVD_OK,          ///< Action success.
    E_HVD_RET_INVALID_PARAMETER,            ///< Function has invalid input.
    E_HVD_RET_ILLEGAL_ACCESS,           ///< Illegal access. like driver not initialized.
    E_HVD_RET_HARDWARE_BREAKDOWN,           ///< HW has no responses or impossible responses.
    E_HVD_RET_OUTOF_MEMORY,             ///< The input memory config is not enough.
    E_HVD_RET_UNSUPPORTED,              ///< Function is not supported by HVD driver.
    E_HVD_RET_TIMEOUT,              ///< Action timeout.
    E_HVD_RET_NOTREADY,             ///< Action not ready. User needs to call it again later.
    E_HVD_RET_MEMORY_OVERWIRTE,             ///< The input memory config may be overwrite.
    E_HVD_RET_QUEUE_FULL,          ///< HVD BBU queue or ES buffer is full.
    E_HVD_RET_RE_INIT,          ///< HVD BBU queue or ES buffer is full.
    E_HVD_RET_NOT_RUNNING,          ///< HVD BBU queue or ES buffer is full.
} HVD_Result;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Get_Play_State
/// @brief \b Enum \b Description:  The current HVD play state.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_GSTATE_INIT,          ///< Before or during initialization.
    E_HVD_GSTATE_PLAY,          ///< playback.
    E_HVD_GSTATE_PAUSE,         ///< decode and display are all paused.
    E_HVD_GSTATE_STOP,          ///< after stop.
}HVD_Get_Play_State;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Sync_Type
/// @brief \b Enum \b Description:  The current mode type of HVD synchronization .
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_SYNC_ATS,     ///< Live stream, or TS file mode.
    E_HVD_SYNC_PTS,     ///< only for file mode, input time stamp is PTS.
    E_HVD_SYNC_DTS,     ///< only for file mode, input time stamp is DTS.
}HVD_Sync_Type;


//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Time_Unit_Type
/// @brief \b Enum \b Description:  The current time unit type of HVD input/ouput.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_TIME_UNIT_90KHZ,     ///< 90khz
    E_HVD_TIME_UNIT_1KHZ,     ///< ms
}HVD_Time_Unit_Type;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Disp_Speed
/// @brief \b Enum \b Description:  The current mode type of HVD display speed
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_DISP_SPEED_FF_32X=32,           ///< Speed fast forward 32x.
    E_HVD_DISP_SPEED_FF_16X=16,           ///< Speed fast forward 16x.
    E_HVD_DISP_SPEED_FF_8X=8,                 ///< Speed fast forward 8x.
    E_HVD_DISP_SPEED_FF_4X=4,                 ///< Speed fast forward 4x.
    E_HVD_DISP_SPEED_FF_2X=2,                 ///< Speed fast forward 2x.
    E_HVD_DISP_SPEED_NORMAL_1X= 1,              ///< Normal display speed.
    E_HVD_DISP_SPEED_SF_2X= -2,              ///< Slow forward 2X.
    E_HVD_DISP_SPEED_SF_4X= -4,              ///< Slow forward 4X.
    E_HVD_DISP_SPEED_SF_8X= -8,              ///< Slow forward 8X.
    E_HVD_DISP_SPEED_SF_16X= -16,            ///< Slow forward 16X.
    E_HVD_DISP_SPEED_SF_32X= -32,            ///< Slow forward 32X.
}HVD_Drv_Disp_Speed;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Drop_Disp
/// @brief \b Enum \b Description:  The current mode type of HVD dropping decoded frames.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_DROP_DISPLAY_AUTO = (1<<0),       ///< automatic drop mode, drop frame if display queue is more than threshold
    E_HVD_DROP_DISPLAY_ONCE = (1<<1),       ///< drop once, drop the number of non-ref frames
}HVD_Drop_Disp;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_FrmRateConv_Mode
/// @brief \b Enum \b Description:  The current mode type of HVD frame rate convertion.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_FRC_MODE_NORMAL,          ///< Disable FRC mode.
    E_HVD_FRC_MODE_32PULLDOWN,    ///< 3:2 pulldown mode (ex. 24p a 60i or 60p)
    E_HVD_FRC_MODE_PAL2NTSC ,     ///< PALaNTSC conversion (50i a 60i)
    E_HVD_FRC_MODE_NTSC2PAL,      ///< NTSCaPAL conversion (60i a 50i)
    E_HVD_FRC_MODE_DISP_2X,    ///< output rate is twice of input rate (ex. 30p a 60p)
    E_HVD_FRC_MODE_24_TO_50,    ///< output rate 24P->50P 48I->50I
    E_HVD_FRC_MODE_50P_60P,    ///< output rate 50P ->60P
    E_HVD_FRC_MODE_60P_50P,    ///< output rate 60P ->50P
}HVD_FrmRateConv_Mode;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Codec
/// @brief \b Enum \b Description:  The supported codec type.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_AVC,
    E_HVD_AVS,
    E_HVD_RM,
}HVD_Codec;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Get_Mode_Status
/// @brief \b Enum \b Description:  The available mode information supported by HVD.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_GMODE_IS_SHOW_ERR_FRM,
    E_HVD_GMODE_IS_REPEAT_LAST_FIELD,
    E_HVD_GMODE_IS_ERR_CONCEAL,
    E_HVD_GMODE_IS_SYNC_ON,
    E_HVD_GMODE_IS_PLAYBACK_FINISH,
    E_HVD_GMODE_SYNC_MODE,
    E_HVD_GMODE_SKIP_MODE,
    E_HVD_GMODE_DROP_MODE,
    E_HVD_GMODE_DISPLAY_SPEED,
    E_HVD_GMODE_FRC_MODE,
    E_HVD_GMODE_ISR_TYPE,
    E_HVD_GMODE_IS_STEP_DISPLAY=0x0100,
    E_HVD_GMODE_STREAM_TYPE,
}HVD_Get_Mode_Status;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Skip_Decode
/// @brief \b Enum \b Description:  The HVD decoding frame types.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_SKIP_DECODE_ALL,  ///< decode all frames
    E_HVD_SKIP_DECODE_I,    ///< decode I frames only
    E_HVD_SKIP_DECODE_IP,   ///< decode I and referenced frames only( skip non-ref frames)
}HVD_Skip_Decode;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Frm_Type
/// @brief \b Enum \b Description:  The frame type.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_FRM_TYPE_I,  ///< I frame.
    E_HVD_FRM_TYPE_P,  ///< P frame.
    E_HVD_FRM_TYPE_B,  ///< B frame.
}HVD_Frm_Type;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_FieldType
/// @brief \b Enum \b Description:  The Field type.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_FIELDTYPE_NONE,  ///< no field.
    E_HVD_FIELDTYPE_TOP,  ///< Top field only.
    E_HVD_FIELDTYPE_BOTTOM,  ///< Bottom field only.
    E_HVD_FIELDTYPE_BOTH,  ///< Both fields.
}HVD_FieldType;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Uart_Level
/// @brief \b Enum \b Description:  The debug level of HVD.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_UART_LEVEL_NONE=0,        ///< Disable all uart message.
    E_HVD_UART_LEVEL_ERR,           ///< Only output error message
    E_HVD_UART_LEVEL_INFO,          ///< output general message, and above.
    E_HVD_UART_LEVEL_DBG,           ///< output debug message, and above.
    E_HVD_UART_LEVEL_TRACE,           ///< output function trace message, and above.
    E_HVD_UART_LEVEL_FW,                ///< output FW message, and above.
} HVD_Uart_Level;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Pattern_Info
/// @brief \b Enum \b Description:  The information type of specific pattern.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_FLUSH_PATTERN_SIZE,         ///< flush pattern size.
    E_HVD_DUMMY_HW_FIFO,            ///< HW fifo size.
} HVD_Pattern_Info;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_DynamicScaling_Info
/// @brief \b Enum \b Description:  The information type of specific dynamic information.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_DS_BUF_MIUSEL,       ///< the HW MIU selection of the buffer of dynamic scaling. TRUE: MIU 1. FALSE: MIU 0.
    E_HVD_DS_BUF_ADDR,         ///< the start physical address of the buffer of dynamic scaling.
    E_HVD_DS_BUF_SIZE,         ///< the size of the buffer of dynamic scaling.
    E_HVD_DS_VECTOR_DEPTH,     ///< the required vector depth of the dynamic scaling.
    E_HVD_DS_INFO_ADDR,        ///< the scaler info buffer address of dynamic scaling.
}HVD_DynamicScaling_Info;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_ISR_Event
/// @brief \b Enum \b Description:  The information type of ISR event.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_ISR_NONE = 0,                        ///< disable ISR
    E_HVD_ISR_DISP_ONE = BIT(0),               ///< HVD display one frame on screen.
    E_HVD_ISR_DISP_REPEAT = BIT(1),            ///< The current displayed frame is repeated frame.
    E_HVD_ISR_DISP_WITH_CC = BIT(2),           ///< Current displayed frame should be displayed with user data.
    E_HVD_ISR_DISP_FIRST_FRM = BIT(3),         ///< HVD display first frame on screen.

    E_HVD_ISR_DEC_ONE = BIT(8),                ///< HVD decoded one frame done.
    E_HVD_ISR_DEC_I = BIT(9),                  ///< HVD decoded one I frame done.
    E_HVD_ISR_DEC_HW_ERR = BIT(10),            ///< HVD HW found decode error.
    E_HVD_ISR_DEC_CC_FOUND = BIT(11),          ///< HVD found one user data with decoded frame.
    E_HVD_ISR_DEC_DISP_INFO_CHANGE = BIT(12),  ///< HVD found display information change.
    E_HVD_ISR_DEC_DATA_ERR = BIT(13),          ///< HVD HW found decode error.
    E_HVD_ISR_DEC_FIRST_FRM = BIT(14),         ///< HVD decode first frame.
} HVD_ISR_Event;


//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_Get_Frm_Info_Type
/// @brief \b Enum \b Description:  The information type of get frame information.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_GFRMINFO_DISPLAY=0,          ///< Displayed frame.
    E_HVD_GFRMINFO_DECODE,          ///< Decoded frame.
} HVD_Get_Frm_Info_Type;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_GData_Type
/// @brief \b Enum \b Description:  The information type of get data
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_GDATA_TYPE_DISP_CNT,
    E_HVD_GDATA_TYPE_SKIP_CNT,
    E_HVD_GDATA_TYPE_DROP_CNT,
    E_HVD_GDATA_TYPE_IDLE_CNT,
    E_HVD_GDATA_TYPE_VSYNC_CNT,
    E_HVD_GDATA_TYPE_MAIN_LOOP_CNT,
    E_HVD_GDATA_TYPE_AVC_LEVEL_IDC,
    E_HVD_GDATA_TYPE_DISP_Q_SIZE,
    E_HVD_GDATA_TYPE_ES_LEVEL,
    E_HVD_GDATA_TYPE_AVC_VUI_DISP_INFO,
    E_HVD_GDATA_TYPE_DISP_STC,
    E_HVD_GDATA_TYPE_USERDATA_IDX_TBL_SIZE,
    E_HVD_GDATA_TYPE_USERDATA_PACKET_SIZE,
} HVD_GData_Type;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_TimeUnit_Type
/// @brief \b Enum \b Description:  The type of time unit.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_TIMEUNIT_90KHZ,       ///< Time unit is 90KHz
    E_HVD_TIMEUNIT_MS,       ///< Time unit is millisecond.
} HVD_TimeUnit_Type;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_GData_Type
/// @brief \b Enum \b Description:  The type of set settings
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_SSET_TIME_UNIT,       ///< set the HVD time unit of all interface. HVD_TimeUnit_Type
    E_HVD_SSET_PITCH,               ///< set the pitch of vsync.
    E_HVD_SSET_SYNC_EACH_FRM,   ///< HVD does sync action at every frame. TREU: turn on; FALSE: turn off.
    E_HVD_SSET_MAX_DEC_TICK,    ///< HVD limits the max decode ticks for one field.
    E_HVD_SSET_AUTO_FREE_ES,    ///< HVD frees the ES buffer data when ES is being fulled.
    E_HVD_SSET_MIN_FRAME_GAP,       ///< set HVD not to report error which is caused by the frame gap larger than min frame gap.
    E_HVD_SSET_DISABLE_DEBLOCKING,    ///< HVD will not do deblocking process.
    E_HVD_SSET_DISABLE_QUARTER_PIXEL,   ///< HVD will not do quarter pixel process.
    E_HVD_SSET_MIU_BURST_CNT_LEVEL,     ///< HVD MIU Burst Cnt, Arg 0~7: burst cnt level, 0xFFFFFFFF = Disable
} HVD_SSettings_Type;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_PatternType
/// @brief \b Enum \b Description:  The type of special pattern for specific purpose.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_PATTERN_FLUSH = 0,           ///< Used after MDrv_HVD_Flush().
    E_HVD_PATTERN_FILEEND,             ///< Used after MDrv_HVD_SetDataEnd().
}HVD_PatternType;


//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_ES_Level
/// @brief \b Enum \b Description:  The level of ES buffer.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_ES_LEVEL_NORMAL = 0,
    E_HVD_ES_LEVEL_UNDER = BIT(0),
    E_HVD_ES_LEVEL_OVER = BIT(1),
} HVD_ES_Level;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_ErrorCode
/// @brief \b Enum \b Description:  The type of HVD error
//-----------------------------------------------------------------------------
typedef enum
{
        E_HVD_ERRCODE_GENERAL_BASE = 0x0000,
    E_HVD_ERRCODE_OUT_OF_SPEC ,
    E_HVD_ERRCODE_UNKNOW_ERR,
    E_HVD_ERRCODE_HW_BREAK_DOWN,
    E_HVD_ERRCODE_HW_DEC_TIMEOUT,
    E_HVD_ERRCODE_OUT_OF_MEMORY,
    E_HVD_ERRCODE_UNKNOWN_CODEC,
    // AVC
        E_HVD_ERRCODE_AVC_BASE = 0x1000,
    E_HVD_ERRCODE_AVC_SPS_BROKEN,           // SPS is not valid
    E_HVD_ERRCODE_AVC_SPS_NOT_IN_SPEC,
    E_HVD_ERRCODE_AVC_SPS_NOT_ENOUGH_FRM,   // DPB size at specified level is smaller than the specified number of reference frames. This is not allowed
    E_HVD_ERRCODE_AVC_PPS_BROKEN,           // PPS is not valid
    E_HVD_ERRCODE_AVC_REF_LIST,
    E_HVD_ERRCODE_AVC_NO_REF,
    E_HVD_ERRCODE_AVC_RES,

    // AVS
        E_HVD_ERRCODE_AVS_BASE = (0x2000),
    E_HVD_ERRCODE_AVS_RES,

    // RM
        E_HVD_ERRCODE_RM_BASE = (0x3000),
    E_HVD_ERRCODE_RM_PACKET_HEADER,
    E_HVD_ERRCODE_RM_FRAME_HEADER,
    E_HVD_ERRCODE_RM_SLICE_HEADER,
    E_HVD_ERRCODE_RM_BYTE_CNT,
    E_HVD_ERRCODE_RM_DISP_TIMEOUT,
    E_HVD_ERRCODE_RM_NO_REF,
    E_HVD_ERRCODE_RM_RES,
}HVD_ErrorCode;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_TurboInitLevel
/// @brief \b Enum \b Description:  The level of turbo init mode.
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_TURBOINIT_NONE=0,     ///< Not omit any process.
    E_HVD_TURBOINIT_CHECK = BIT(0),     ///< Omit checking.
    E_HVD_TURBOINIT_MEMORY = BIT(1),     ///< Omit memory reset process.
    E_HVD_TURBOINIT_DISPLAY = BIT(2),     ///< Omit FW display setup process.
    E_HVD_TURBOINIT_FW_RELOAD = BIT(3), ///< Omit FW reload process.
}HVD_TurboInit_Type;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_FWSourceType
/// @brief \b Enum \b Description:  The type of fw binary input source
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_FW_SOURCE_NONE,       ///< No input fw.
    E_HVD_FW_SOURCE_DRAM,       ///< input source from DRAM.
    E_HVD_FW_SOURCE_FLASH,      ///< input source from FLASH.
}HVD_FWSourceType;

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: HVD_DumpStatus
/// @brief \b Enum \b Description:  The type of fw binary input source
//-----------------------------------------------------------------------------
typedef enum
{
    E_HVD_DUMP_STATUS_DRV=BIT(0),       ///< Dump Driver status
    E_HVD_DUMP_STATUS_FW=BIT(1),       ///< Dump firmware status
    E_HVD_DUMP_STATUS_HW=BIT(2),       ///< Dump hardware status
}HVD_DumpStatus;

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: HVD_Disp_Info
/// @brief \b Struct \b Description:  Store the HVD driver information
//-----------------------------------------------------------------------------
typedef struct
{
    MS_U16 u16HorSize;          ///< pixel width.
    MS_U16 u16VerSize;          ///< pixel height.
    MS_U32 u32FrameRate;    ///< 1000 times frames per second.
    MS_U8 u8AspectRate;     ///< aspect ration ID.
    MS_U8 u8Interlace;          ///< interlace content
    MS_U8 u8AFD;            ///< AFD ID number
    MS_U8 bChroma_idc_Mono;             ///< - TRUE: mono mode FALSE: colorful, not mono
    MS_U16 u16SarWidth;            ///< Sample aspect width ratio.
    MS_U16 u16SarHeight;           ///< Sample aspect height ratio.
    MS_U16 u16CropRight;            ///< crop right.
    MS_U16 u16CropLeft;             ///< crop left.
    MS_U16 u16CropBottom;           ///< crop bottom.
    MS_U16 u16CropTop;          ///< crop top.
    MS_U16 u16Pitch;            ///< pitch
    MS_U8  u8ColourPrimaries;   ///< Color Primaries in VUI
    //****************************
    MS_U8 reserved8_0;          ///< reserved.
    //******************************
} HVD_Disp_Info;     //  bytes

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: HVD_DrvInfo
/// @brief \b Struct \b Description:  Store the HVD driver information
//-----------------------------------------------------------------------------
typedef struct
{
    MS_BOOL bAVC;       ///< - TRUE: HW does support AVC.  - FALSE: HW does not support AVC.
    MS_BOOL bAVS;       ///< - TRUE: HW does support AVS.  - FALSE: HW does not support AVS.
    MS_BOOL bRM;        ///< - TRUE: HW does support RM.  - FALSE: HW does not support RM.
    MS_U32 FWversion;       ///<  FW version number.
}HVD_DrvInfo;

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: HVD_DrvStatus
/// @brief \b Struct \b Description:  Store the HVD driver status
//-----------------------------------------------------------------------------
typedef struct
{
    MS_BOOL bInit;       ///< - TRUE: Initialization success.  - FALSE: Initialization failed or not initialized yet.
    MS_BOOL bBusy;       ///< - TRUE: Driver is processing  - FALSE: Driver is Idle.
}HVD_DrvStatus;

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: RV_Info
/// @brief \b Struct \b Description:  RV file information
//-----------------------------------------------------------------------------
typedef struct
{
    MS_U16 RV_Version;      ///< Real Video Bitstream version
    MS_U16 ulNumSizes;      ///< Real Video Number sizes
    MS_U16 ulPicSizes_w[8]; ///< Real Video file width
    MS_U16 ulPicSizes_h[8]; ///< Real Video file height
} RV_Info;

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: HVD_MemCfg
/// @brief \b Struct \b Description:  Store the HVD driver config
//-----------------------------------------------------------------------------
typedef struct
{
    HVD_FWSourceType eFWSourceType;             //!< the input FW source type.
    MS_U32  u32FWBinaryVAddr;                //!<  virtual address of input FW binary in DRAM
    MS_PHYADDR  u32FWBinaryAddr;                //!< the physical memory start address in Flash/DRAM memory of FW code
    MS_U32  u32FWBinarySize;                //!< the FW code size
    MS_U32 u32VLCBinaryVAddr;///< VLC table binary data buffer start address
    MS_PHYADDR u32VLCBinaryAddr;///< VLC table binary data buffer start address
    MS_U32 u32VLCBinarySize;///<VLC table binary data buffer size
    MS_PHYADDR  u32MIU1BaseAddr;       //!< the physical memory start address of MIU 1 base address. 0: default value.
    MS_U32  u32CodeBufVAddr;        //!< the virtual memory start address of code buffer
    MS_PHYADDR  u32CodeBufAddr;         //!< the physical memory start address of code buffer
    MS_U32  u32CodeBufSize;             //!< the code buffer size
    MS_U32  u32FrameBufVAddr;           //!< the virtual memory start address of frame buffer
    MS_PHYADDR  u32FrameBufAddr;            //!< the physical memory start address of frame buffer
    MS_U32  u32FrameBufSize;                //!< the frame buffer size
    MS_U32  u32BitstreamBufVAddr;           //!< the virtual memory start address of bit stream buffer
    MS_PHYADDR  u32BitstreamBufAddr;                //!< the physical memory start address of bit stream buffer
    MS_U32  u32BitstreamBufSize;            //!< the bit stream buffer size
    MS_U32  u32DrvProcessBufVAddr;       //!< the virtual memory start address of driver process buffer
    MS_PHYADDR  u32DrvProcessBufAddr;       //!< the physical memory start address of driver process buffer
    MS_U32  u32DrvProcessBufSize;        //!< the driver process buffer size
}HVD_MemCfg;

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: HVD_Init_Params
/// @brief \b Struct \b Description:  Store the initialization settings
//-----------------------------------------------------------------------------
typedef struct
{
    MS_U32 u32ModeFlag;     ///< init mode flag, use HVD_INIT_* to setup HVD.
    MS_U32 u32FrameRate;     ///< frame rate.
    MS_U32 u32FrameRateBase;     ///< frame rate base. The value of u32FrameRate /u32FrameRateBase must be frames per sec.
    MS_U8   u8MinFrmGap;    ///< set the min frame gap.
    MS_U8   u8SyncType;         ///< HVD_Sync_Type. sync type of current playback.
    MS_U16 u16Pitch;   ///< not zero: specify the pitch. 0: use default value.
    MS_U32 u32MaxDecTick;   ///< not zero: specify the max decode tick. 0: use default value.
    MS_BOOL bSyncEachFrm;   ///< TRUE: sync STC at each frame. FALSE: not sync each frame.
    MS_BOOL bAutoFreeES;   ///< TRUE: auto free ES buffer when ES buffer is full. FALSE: not do the auto free.
    MS_BOOL bAutoPowerSaving;   ///< TRUE: auto power saving. FALSE: not do the auto power saving.
    MS_BOOL bDynamicScaling;   ///< TRUE: enable Dynamic Scaling. FALSE: disable Dynamic Scaling.
    MS_BOOL bFastDisplay;   ///< TRUE: enable Fast Display. FALSE: disable Fast Display.
    MS_BOOL bUserData;   ///< TRUE: enable processing User data. FALSE: disable processing User data.
    MS_U8 u8TurboInit;       ///< HVD_TurboInitLevel. set the turbo init mode.
    MS_U8 u8TimeUnit;   ///< HVD_Time_Unit_Type.set the type of input/output time unit.
    MS_U16 u16DecoderClock;      ///< HVD decoder clock speed. 0: default value. non-zero: any nearist clock.
    RV_Info* pRVFileInfo;           ///< pointer to RV file info
}HVD_Init_Settings;

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: HVD_Packet_Info
/// @brief \b Struct \b Description:  Store the packet information
//-----------------------------------------------------------------------------
typedef struct
{
    MS_U32 u32Staddr;     ///< Packet offset from bitstream buffer base address. unit: byte.
    MS_U32 u32Length;    ///< Packet size. unit: byte.
    MS_U32 u32TimeStamp;    ///< Packet time stamp. unit: ms.
    MS_U32 u32ID_L;    ///< Packet ID low part.
    MS_U32 u32ID_H;    ///< Packet ID high part.
}HVD_Packet_Info;

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: HVD_Frame_Info
/// @brief \b Struct \b Description:  Store the frame information
//-----------------------------------------------------------------------------
typedef struct
{
    MS_PHYADDR u32LumaAddr;                 ///< The start physical of luma data. Unit: byte.
    MS_PHYADDR u32ChromaAddr;               ///< The start physcal of chroma data. Unit: byte.
    MS_U32 u32TimeStamp;                ///< Time stamp(DTS, PTS) of current displayed frame. Unit: 90khz.
    MS_U32 u32ID_L;                     ///< low part of ID number decided by MDrv_HVD_PushQueue().
    MS_U32 u32ID_H;                     ///< high part of ID number decided by MDrv_HVD_PushQueue().
    MS_U16 u16Pitch;                        ///< The pitch of current frame.
    MS_U16 u16Width;                        ///< pixel width of current frame.
    MS_U16 u16Height;                       ///< pixel height of current frame.
    HVD_Frm_Type eFrmType;     ///< picture type: I, P, B frame
    HVD_FieldType  eFieldType;                 ///< none, top , bottom, both field
} HVD_Frame_Info;

typedef struct
{
    MS_BOOL aspect_ratio_info_present_flag;            // u(1)
    MS_U8 aspect_ratio_idc;                            // u(8)
    MS_U16 sar_width;                                  // u(16)
    MS_U16 sar_height;                                 // u(16)
    MS_BOOL overscan_info_present_flag;                // u(1)
    MS_BOOL overscan_appropriate_flag;                 // u(1)
    MS_BOOL video_signal_type_present_flag;            // u(1)
    MS_U8 video_format;                                // u(3)
    MS_BOOL video_full_range_flag;                     // u(1)
    MS_BOOL colour_description_present_flag;           // u(1)
    MS_U8 colour_primaries;                            // u(8)
    MS_U8 transfer_characteristics;                    // u(8)
    MS_U8 matrix_coefficients;                         // u(8)
    MS_BOOL chroma_location_info_present_flag;         // u(1)
    MS_U8 chroma_sample_loc_type_top_field;            // ue(v) 0~5
    MS_U8 chroma_sample_loc_type_bottom_field;         // ue(v) 0~5
    MS_BOOL timing_info_present_flag;                  // u(1)
    MS_BOOL fixed_frame_rate_flag;                     // u(1)
    MS_U32 num_units_in_tick;                          // u(32)
    MS_U32 time_scale;                                 // u(32)
} HVD_VUI_DISP_INFO;

//-----------------------------------------------------------------------------
/// @brief \b Struct \b Name: HVD_DISP_INFO_THRESHOLD
/// @brief \b Struct \b Description:  Store the disp information threshold
//-----------------------------------------------------------------------------
typedef struct
{
    MS_U32 u32FrmrateUpBound;       //Framerate filter upper bound
    MS_U32 u32FrmrateLowBound;      //Framerate filter lower bound
    MS_U32 u32MvopUpBound;          //mvop filter upper bound
    MS_U32 u32MvopLowBound;         //mvop filter lower bound
} HVD_Disp_Info_Threshold;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MDrv_HVD_PowerCtrl( MS_BOOL bEnable );
void MDrv_HVD_SetOSRegBase( MS_U32 u32RegBaseAddr );
HVD_Result MDrv_HVD_Init( HVD_MemCfg* mem , HVD_Init_Settings* InitParam );
HVD_Result MDrv_HVD_Rst(MS_BOOL bErrHandle);
HVD_Result MDrv_HVD_Play(void);
HVD_Result MDrv_HVD_Exit(void);
HVD_Result MDrv_HVD_Pause(void);
HVD_Result MDrv_HVD_Flush( MS_BOOL bShowLast);
HVD_Result MDrv_HVD_StepDisp( void);
HVD_Result MDrv_HVD_StepDecode(void);
HVD_Result MDrv_HVD_PushQueue(HVD_Packet_Info* pInfo);
HVD_Result MDrv_HVD_PushQueue_Fire(void);
HVD_Result MDrv_HVD_DecodeIFrame(MS_PHYADDR u32SrcSt , MS_U32 u32SrcSize);

HVD_Result MDrv_HVD_SetDataEnd( MS_BOOL bEnd );
HVD_Result MDrv_HVD_SetDispErrFrm(MS_BOOL bEnable);
HVD_Result MDrv_HVD_SetDispRepeatField(MS_BOOL bEnable);
HVD_Result MDrv_HVD_SetSkipDecMode(HVD_Skip_Decode eDecType);
HVD_Result MDrv_HVD_SetDispSpeed(HVD_Drv_Disp_Speed eSpeed);
HVD_Result MDrv_HVD_SetSyncActive( MS_BOOL bEnable);
HVD_Result MDrv_HVD_SetDropMode( HVD_Drop_Disp eMode , MS_U32 u32Arg);
HVD_Result MDrv_HVD_RstPTS(MS_U32 u32PTS);
HVD_Result MDrv_HVD_SetFrcMode(HVD_FrmRateConv_Mode eMode );
HVD_Result MDrv_HVD_SetSyncTolerance(MS_U32 u32Arg );
HVD_Result MDrv_HVD_SetSyncVideoDelay( MS_U32 u32Arg);
HVD_Result MDrv_HVD_SetSyncFreeRunTH(MS_U32 u32Arg );
HVD_Result MDrv_HVD_SetSyncRepeatTH( MS_U32 u32Arg);
HVD_Result MDrv_HVD_SetErrConceal(MS_BOOL u32Arg );
void MDrv_HVD_SetDbgLevel( HVD_Uart_Level elevel );
HVD_Result MDrv_HVD_SeekToPTS(MS_U32 u32PTS );
HVD_Result MDrv_HVD_SkipToPTS(MS_U32 u32PTS );
HVD_Result MDrv_HVD_SetFreezeImg(MS_BOOL bEnable );
HVD_Result MDrv_HVD_SetBlueScreen(MS_BOOL bEnable );
HVD_Result MDrv_HVD_SetDispOneField(MS_BOOL bEnable );
HVD_Result MDrv_HVD_SetISREvent(MS_U32 u32Event, HVD_InterruptCb fnISRHandler);
MS_BOOL MDrv_HVD_SetEnableISR(MS_BOOL bEnable);
MS_BOOL MDrv_HVD_SetForceISR(MS_BOOL bEnable);
MS_BOOL MDrv_HVD_SetMVOPDone(void);
HVD_Result MDrv_HVD_SetVirtualBox(MS_U16 u16Width, MS_U16 u16Height);
HVD_Result MDrv_HVD_SetDynScalingParam(  void *pStAddr , MS_U32 u32Size );
HVD_Result MDrv_HVD_SetAutoRmLstZeroByte(MS_BOOL bOn);
HVD_Result MDrv_HVD_SetDispInfoTH(HVD_Disp_Info_Threshold* DispInfoTH );

MS_BOOL MDrv_HVD_IsISROccured(void);
MS_BOOL MDrv_HVD_IsDispFinish(void);
MS_BOOL MDrv_HVD_IsFrameShowed(void);
MS_BOOL MDrv_HVD_IsStepDecodeDone(void);
HVD_Result MDrv_HVD_CheckDispInfoRdy(void);
MS_BOOL MDrv_HVD_IsDispInfoChg(void);
MS_BOOL MDrv_HVD_IsIdle(void);
MS_BOOL MDrv_HVD_IsSyncStart(void);
MS_BOOL MDrv_HVD_IsSyncReach(void);
MS_BOOL MDrv_HVD_IsLowDelay(void);
MS_BOOL MDrv_HVD_IsIFrmFound(void);
MS_BOOL MDrv_HVD_Is1stFrmRdy(void);
MS_BOOL MDrv_HVD_IsAllBufferEmpty(void);
HVD_Result MDrv_HVD_IsAlive(void);

MS_U32 MDrv_HVD_GetBBUVacancy(void);
HVD_Result MDrv_HVD_GetDispInfo( HVD_Disp_Info *pinfo );
MS_U32 MDrv_HVD_GetPTS(void);
MS_U32 MDrv_HVD_GetDataErrCnt(void);
MS_U32 MDrv_HVD_GetDecErrCnt(void);
MS_U32 MDrv_HVD_GetESWritePtr(void);
MS_U32 MDrv_HVD_GetESReadPtr(void);
MS_BOOL MDrv_HVD_GetCaps( HVD_Codec u32Type);
MS_U32 MDrv_HVD_GetErrCode(void);
MS_U32 MDrv_HVD_GetPlayMode(HVD_Get_Mode_Status eMode);
HVD_Get_Play_State MDrv_HVD_GetPlayState(void);
MS_U32 MDrv_HVD_GetDecodeCnt(void);
MS_U8 MDrv_HVD_GetActiveFormat(void);
const HVD_DrvInfo* MDrv_HVD_GetInfo( void );
HVD_Result MDrv_HVD_GetLibVer(const MSIF_Version **ppVersion);
MS_BOOL MDrv_HVD_GetStatus( HVD_DrvStatus *pstatus);
HVD_Result MDrv_HVD_GetFrmInfo( HVD_Get_Frm_Info_Type eType  ,  HVD_Frame_Info *pInfo);
MS_BOOL MDrv_HVD_GetISRInfo(MS_U32* eType);
MS_U32 MDrv_HVD_CalLumaSum(  HVD_Get_Frm_Info_Type eType  );
MS_U32 MDrv_HVD_GetUserData_Wptr( void );
MS_U32 MDrv_HVD_GetUserData_Packet( MS_U32 u32Idx , MS_U32* u32Size );
HVD_Result MDrv_HVD_GenPattern( HVD_PatternType eType ,  MS_U32 u32VAddr  ,  MS_U32* u32Size  );
MS_U32 MDrv_HVD_GetPatternInfo(   HVD_Pattern_Info eType );
MS_U32 MDrv_HVD_GetDynamicScalingInfo(   HVD_DynamicScaling_Info eType );

MS_U32 MDrv_HVD_GetData(HVD_GData_Type eType);
MS_U32 MDrv_HVD_GetMem_Dbg(MS_U32 u32Addr);
void MDrv_HVD_DbgDumpStatus(HVD_DumpStatus eFlag);
void MDrv_HVD_SetMem_Dbg(MS_U32 u32Addr , MS_U32 u32Arg);
HVD_Result MDrv_HVD_SetCmd_Dbg( MS_U32 u32Cmd , MS_U32 u32Arg);
HVD_Result MDrv_HVD_SetSettings_Pro( HVD_SSettings_Type eType , MS_U32 u32Arg);
HVD_Result MDrv_HVD_SetBalanceBW(MS_U8 qp_cnt, MS_U8 db_cnt, MS_U8 upper);

#ifdef __cplusplus
}
#endif

#endif // _DRV_HVD_H_

