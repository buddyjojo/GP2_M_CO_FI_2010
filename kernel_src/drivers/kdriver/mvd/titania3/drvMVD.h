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
/// @file  drvMVD.h
/// @brief MPEG-2/4 Video Decoder header file
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_MVD_H_
#define _DRV_MVD_H_




////////////////////////////////////////////////////////////////////////////////
// Include List
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif


////////////////////////////////////////////////////////////////////////////////
// Constant & Macro Definition
////////////////////////////////////////////////////////////////////////////////
/// Version string.
#define MVD_DRV_VERSION                 /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    {'M','V','D','_'},                  /* IP__                                             */  \
    {'0','1'},                          /* 0.0 ~ Z.Z                                        */  \
    {'0','1'},                          /* 00 ~ 99                                          */  \
    {'0','0','0','0','0','0','0','0'},  /* CL#                                              */  \
    MSIF_OS

#define MIU1_BASE_DEFAULT 0x8000000
#define MIU_SEL_0         0
#define MIU_SEL_1         1


#ifdef REDLION_LINUX_KERNEL_ENVI
	#define MVD_ENABLE_MSOS_MIU1_BASE 	0
#else
	#define MVD_ENABLE_MSOS_MIU1_BASE 	1
#endif


////////////////////////////////////////////////////////////////////////////////
// Type & Structure Declaration
////////////////////////////////////////////////////////////////////////////////
/// MVD's capability
typedef struct _MVD_Caps
{
    MS_BOOL bMPEG2;
    MS_BOOL bMPEG4;
    MS_BOOL bVC1;
} MVD_Caps;

/// MVD driver info.
typedef struct _MVD_DrvInfo
{
    MS_U32 u32Resource;
    MS_U32 u32DeviceNum;
    MS_U32 u32FWVersion;
    MVD_Caps stCaps;
} MVD_DrvInfo;

/// Firmware status
typedef enum _MVD_DecStat
{
    E_MVD_STAT_IDLE             = 0x00,
    E_MVD_STAT_FIND_STARTCODE   = 0x01,//start code
    E_MVD_STAT_FIND_SPECIALCODE = 0x11,//special code [00_00_01_C5_ab_08_06_27]
                                       //for flush data using
    E_MVD_STAT_FIND_FRAMEBUFFER = 0x02,
    //fw is trying to find empty FB to continue decoding.
    //if hang in this state, please check "vsync" or AVSync.
    E_MVD_STAT_WAIT_DECODEDONE  = 0x03,
    E_MVD_STAT_DECODE_DONE      = 0x04,
    E_MVD_STAT_WAIT_VDFIFO      = 0x05,
    E_MVD_STAT_INIT_SUCCESS     = 0x06,
    E_MVD_STAT_UNKNOWN          = 0xff,
} MVD_DecStat;

/// MVD driver status
typedef struct _MVD_DrvStatus
{
    MS_U32      u32FWVer;
    MVD_DecStat eDecStat;
    MS_BOOL     bIsBusy;
    MS_U8       u8LastFWCmd;
} MVD_DrvStatus;

/// Enumerate CodecType that MVD supports
typedef enum _MVD_CodecType
{
    E_MVD_CODEC_MPEG2                    = 0x10,
    E_MVD_CODEC_MPEG4                    = 0x00,
    E_MVD_CODEC_MPEG4_SHORT_VIDEO_HEADER = 0x01,
    E_MVD_CODEC_DIVX311                  = 0x02,
    E_MVD_CODEC_FLV                      = 0x03,
    E_MVD_CODEC_VC1_ADV                  = 0x04,
    E_MVD_CODEC_VC1_MAIN                 = 0x05,
    E_MVD_CODEC_UNKNOWN                  = 0xff
} MVD_CodecType;

/// Mode of MVD input source
typedef enum _MVD_SrcMode
{
    E_MVD_FILE_MODE       = 0x00,
    E_MVD_SLQ_MODE        = 0x01,
    E_MVD_TS_MODE         = 0x02,
    E_MVD_SLQ_TBL_MODE    = 0x03,
    E_MVD_TS_FILE_MODE    = 0x04,
    E_MVD_SRC_UNKNOWN     = 0x05
} MVD_SrcMode;

/// AVSync mode for file playback
typedef enum
{
    E_MVD_TIMESTAMP_FREERUN  = 0,  ///Player didn't set PTS/DTS, so display freerun
    E_MVD_TIMESTAMP_PTS      = 1,  ///Player set PTS to MVD decoder
    E_MVD_TIMESTAMP_DTS      = 2,  ///Player set DTS to MVD decoder
} MVD_TIMESTAMP_TYPE;

/// ErrCode obtained from firmware
typedef enum _MVD_ErrCode
{
    E_MVD_ERR_UNKNOWN            = 0,
    E_MVD_ERR_SHAPE              = 1,
    E_MVD_ERR_USED_SPRITE        = 2,
    E_MVD_ERR_NOT_8_BIT          = 3,   //error_status : bits per pixel
    E_MVD_ERR_NERPRED_ENABLE     = 4,
    E_MVD_ERR_REDUCED_RES_ENABLE = 5,
    E_MVD_ERR_SCALABILITY        = 6,
    E_MVD_ERR_OTHER              = 7,
    E_MVD_ERR_H263_ERROR         = 8,
    E_MVD_ERR_RES_NOT_SUPPORT    = 9,   //error_status : none
    E_MVD_ERR_MPEG4_NOT_SUPPORT  = 10   //error_status : none
} MVD_ErrCode;

/// Detailed error status when error occurs
typedef enum _MVD_ErrStatus
{
    //error_status for E_MVD_ERR_SHAPE
    E_MVD_ERR_SHAPE_RECTANGULAR    = 0x10,
    E_MVD_ERR_SHAPE_BINARY         = 0x11,
    E_MVD_ERR_SHAPE_BINARY_ONLY    = 0x12,
    E_MVD_ERR_SHAPE_GRAYSCALE      = 0x13,

    //error_status for E_MVD_ERR_USED_SPRITE
    E_MVD_ERR_USED_SPRITE_UNUSED   = 0x20, //sprite not used
    E_MVD_ERR_USED_SPRITE_STATIC   = 0x21,
    E_MVD_ERR_USED_SPRITE_GMC      = 0x22,
    E_MVD_ERR_USED_SPRITE_RESERVED = 0x23,

    E_MVD_ERR_STATUS_NONE          = 0x77,
    E_MVD_ERR_STATUS_UNKOWN        = 0x00,
} MVD_ErrStatus;

/// Picture type of MPEG video
typedef enum _MVD_PicType
{
    E_MVD_PIC_I = 0,
    E_MVD_PIC_P = 1,
    E_MVD_PIC_B = 2,
    E_MVD_PIC_UNKNOWN = 0xf,
} MVD_PicType;

/// Mode of frame rate conversion
typedef enum _MVD_FrcMode {
    E_MVD_FRC_NORMAL = 0,
    E_MVD_FRC_DISP_TWICE = 1,
    E_MVD_FRC_3_2_PULLDOWN = 2,
    E_MVD_FRC_PAL_TO_NTSC = 3,
    E_MVD_FRC_NTSC_TO_PAL = 4,
    E_MVD_FRC_DISP_ONEFIELD = 5,
} MVD_FrcMode;

/// MVD play mode
typedef enum
{
    E_MVD_PLAY          = 0x00,
    E_MVD_STEPDISP      = 0x01,
    E_MVD_PAUSE         = 0x02,
    E_MVD_FASTFORWARD   = 0x03,
    E_MVD_BACKFORWARD   = 0x04,
    E_MVD_UNKNOWMODE    = 0xff
} MVD_PlayMode;

/// Store the packet information used for SLQ table file mode
typedef struct
{
    MS_U32 u32StAddr;    ///< Packet offset from bitstream buffer base address. unit: byte.
    MS_U32 u32Length;    ///< Packet size. unit: byte.
    MS_U32 u32TimeStamp; ///< Packet time stamp. unit: ms.
    MS_U32 u32ID_L;      ///< Packet ID low part.
    MS_U32 u32ID_H;      ///< Packet ID high part.
} MVD_PacketInfo;

///MVD frame info data structure
typedef struct
{
    MS_U16  u16HorSize;
    MS_U16  u16VerSize;
    MS_U32  u32FrameRate;
    MS_U8   u8AspectRate;
    MS_U8   u8Interlace;
    MS_U8   u8AFD;
    MS_U8   u8par_width;
    MS_U8   u8par_height;
    MS_U16  u16SarWidth;
    MS_U16  u16SarHeight;
    MS_U16  u16CropRight;
    MS_U16  u16CropLeft;
    MS_U16  u16CropBottom;
    MS_U16  u16CropTop;
    MS_U16  u16Pitch;
    MS_U16  u16PTSInterval;
    MS_U8   u8MPEG1;
    MS_U8   u8PlayMode;
    MS_U8   u8FrcMode;
    MS_U32  u32DynScalingAddr;    ///Dynamic scaling address
    MS_U8   u8DynScalingDepth;    ///Dynamic scaling depth
    MS_BOOL bEnableMIUSel;        ///Dynamic scaling DS buffer on miu1 or miu0
} MVD_FrameInfo;

#define MVD_RCV_SEQINFO_SIZE (21*4)
/// MVD sequence info for RCV file mode
typedef struct _MVD_RCV_SeqInfo
{
    MS_U32 u32StructSize;
    MS_U32 u32Profile;
    MS_U32 u32FrmrtqPostproc;
    MS_U32 u32BitrtqPostproc;
    MS_U32 u32Loopfilter;
    MS_U32 u32Multires;
    MS_U32 u32Fastuvmc;
    MS_U32 u32ExtendedMv;
    MS_U32 u32Dquant;
    MS_U32 u32Vstransform;
    MS_U32 u32Overlap;
    MS_U32 u32Syncmarker;
    MS_U32 u32Rangered;
    MS_U32 u32Maxbframes;
    MS_U32 u32Quantizer;
    MS_U32 u32Finterpflag;
    MS_U32 u32Level;
    MS_U32 u32Cbr;
    MS_U32 u32Framerate;
    MS_U32 u32VertSize;
    MS_U32 u32HoriSize;
} MVD_RCV_SeqInfo;

/// MVD AVSync Configuration
typedef struct
{
    MS_BOOL bEnable;
    MS_U32  u32Delay;       //unit: ms
    MS_U16  u16Tolerance;   //unit: ms
} MVD_AVSyncCfg;

/// MVD Command arguments
typedef struct
{
    MS_U8 Arg0;    ///< argument 0
    MS_U8 Arg1;    ///< argument 1
    MS_U8 Arg2;    ///< argument 2
    MS_U8 Arg3;    ///< argument 3
    MS_U8 Arg4;    ///< argument 4
    MS_U8 Arg5;    ///< argument 5
} MVD_CmdArg;

/// MVD commands needing handshake
typedef enum
{
    MVD_HANDSHAKE_PAUSE,
    MVD_HANDSHAKE_SLQ_RST,
    MVD_HANDSHAKE_STOP,
    MVD_HANDSHAKE_SKIP_DATA,
    MVD_HANDSHAKE_SKIP_TO_PTS,
    MVD_HANDSHAKE_SINGLE_STEP,
    MVD_HANDSHAKE_SCALER_INFO
} MVD_HANDSHAKE_CMD;

/// Configuration of MVD firmware
typedef struct
{
    MS_U32     u32FWVerNum;
    MS_U32     u32FWSrc;
    MS_U8      u8FBMode;
} MVD_FWCfg;

/// The type of fw binary input source
typedef enum
{
    E_MVD_FW_SOURCE_NONE,       ///< No input fw.
    E_MVD_FW_SOURCE_DRAM,       ///< input source from DRAM.
    E_MVD_FW_SOURCE_FLASH,      ///< input source from FLASH.
} MVD_FWSrcType;

/// Configuration of memory layout
typedef struct
{
    MVD_FWSrcType eFWSrcType;         //!< the input FW source type.
    MS_U32     u32FWSrcVAddr;         //!< virtual address of input FW binary in DRAM
    MS_PHYADDR u32FWBinAddr;          //!< physical address in Flash/DRAM of FW code
    MS_U32     u32FWBinSize;
    MS_PHYADDR u32FWCodeAddr;
    MS_U32     u32FWCodeSize;
    MS_PHYADDR u32FBAddr;
    MS_U32     u32FBSize;
    MS_PHYADDR u32BSAddr;
    MS_U32     u32BSSize;
    MS_PHYADDR u32DrvBufAddr;
    MS_U32     u32DrvBufSize;
    MS_PHYADDR u32DynSacalingBufAddr;
    MS_U32     u32DynSacalingBufSize;
    MS_PHYADDR u32Miu1BaseAddr;
    MS_BOOL    bFWMiuSel;
    MS_BOOL    bHWMiuSel;
    MS_BOOL    bEnableDynScale;    /// dynamic scaling control bit
} MVD_MEMCfg;

/// Return value of MVD driver
typedef enum
{
    E_MVD_RET_OK            = 1,
    E_MVD_RET_FAIL          = 0,
    E_MVD_RET_INVALID_PARAM = 2,
    E_MVD_RET_QUEUE_FULL    = 3,
    E_MVD_RET_TIME_OUT      = 4
} E_MVD_Result;

/// Mode of trick decoding
typedef enum
{
    E_MVD_TRICK_DEC_ALL = 0,
    E_MVD_TRICK_DEC_IP,
    E_MVD_TRICK_DEC_I,
    E_MVD_TRICK_DEC_UNKNOWN
} MVD_TrickDec;

/// Speed type of playing
typedef enum
{
    E_MVD_SPEED_DEFAULT = 0,
    E_MVD_SPEED_FAST,
    E_MVD_SPEED_SLOW,
    E_MVD_SPEED_TYPE_UNKNOWN
} MVD_SpeedType;

/// MVD pattern type
typedef enum
{
    E_MVD_PATTERN_FLUSH = 0,           ///< Used after MDrv_MVD_Flush().
    E_MVD_PATTERN_FILEEND,             ///< Used for file end.
} MVD_PatternType;

/// MVD frame info structure
typedef struct
{
    MS_PHYADDR u32LumaAddr;    ///< The start physical of luma data. Unit: byte.
    MS_PHYADDR u32ChromaAddr;  ///< The start physcal of chroma data. Unit: byte.
    MS_U32 u32TimeStamp;       ///< Time stamp(DTS, PTS) of current displayed frame. Unit: 90khz.
    MS_U32 u32ID_L;            ///< low part of ID number set by MDrv_MVD_PushQueue().
    MS_U32 u32ID_H;            ///< high part of ID number set by MDrv_MVD_PushQueue().
    MS_U16 u16Pitch;           ///< The pitch of current frame.
    MS_U16 u16Width;           ///< pixel width of current frame.
    MS_U16 u16Height;          ///< pixel height of current frame.
    MVD_PicType eFrmType;     ///< picture type: I, P, B frame
} MVD_FrmInfo;

/// Type of frame info that can be queried
typedef enum
{
    E_MVD_FRMINFO_DISPLAY=0,   ///< Displayed frame.
    E_MVD_FRMINFO_DECODE =1,   ///< Decoded frame.
} MVD_FrmInfoType;

/// Format of CC (Closed Caption)
typedef enum _MVD_CCFormat
{
    E_MVD_CC_NONE       = 0x00,
    E_MVD_CC_608        = 0x01, //For CC608 or 157
    E_MVD_CC_708        = 0x02, //For CC708
    E_MVD_CC_UNPACKED   = 0x03,
} MVD_CCFormat;

/// Type of CC
typedef enum _MVD_CCType
{
    E_MVD_CC_TYPE_NONE = 0,
    E_MVD_CC_TYPE_NTSC_FIELD1 = 1,
    E_MVD_CC_TYPE_NTSC_FIELD2 = 2,
    E_MVD_CC_TYPE_DTVCC = 3,
    E_MVD_CC_TYPE_NTSC_TWOFIELD = 4,
} MVD_CCType;

/// Data structure of CC Configuration
typedef struct
{
    MVD_CCFormat eFormat;
    MVD_CCType   eType;
    MS_U32       u32BufStAdd;
    MS_U32       u32BufSize;
} MVD_CCCfg;

/// MVD interrupt events
typedef enum
{
    E_MVD_EVENT_DISABLE_ALL = 0,           ///< unregister all events notification
    E_MVD_EVENT_USER_DATA = BIT(0),        ///< found user data
#if 0
    E_MVD_EVENT_SEQ_FOUND = BIT(1),        ///< found sequence header
    E_MVD_EVENT_PIC_FOUND = BIT(2),        ///<
#endif
    E_MVD_EVENT_FIRST_FRAME = BIT(3),      ///< first frame decoded
    E_MVD_EVENT_DISP_RDY = BIT(4),         ///< MVD ready to display.
    E_MVD_EVENT_SEQ_FOUND = BIT(5),        ///< found sequence header
    //E_MVD_EVENT_DEC_ERR = BIT(5),          ///< MVD HW found decode error.
    //E_MVD_EVENT_DEC_CC_FOUND = BIT(6),     ///< MVD found one user data with decoded frame.
    //E_MVD_EVENT_DEC_DATA_ERR = BIT(7),     ///< Data error.
} MVD_Event;

typedef void (*MVD_InterruptCb)(void);

typedef struct
{
    MS_PHYADDR u32UserDataBuf;
} MVD_InternalMemCfg;

////////////////////////////////////////////////////////////////////////////////
// Function Prototype Declaration
////////////////////////////////////////////////////////////////////////////////
E_MVD_Result MDrv_MVD_SetCfg(MVD_FWCfg* fwCfg, MVD_MEMCfg* memCfg);
MS_U32 MDrv_MVD_GetFWVer(void);
void MDrv_MVD_SetDbgLevel(MS_U8 level);
const MVD_DrvInfo* MDrv_MVD_GetInfo(void);
E_MVD_Result MDrv_MVD_GetLibVer(const MSIF_Version **ppVersion);
void MDrv_MVD_GetStatus(MVD_DrvStatus* pDrvStatus);
MVD_InternalMemCfg *MDrv_MVD_GetInternalMemCfg(void);


void MDrv_MVD_SetCodecInfo(MVD_CodecType u8CodecType, MVD_SrcMode u8BSProviderMode, MS_U8 bDisablePESParsing);
void MDrv_MVD_SetDivXCfg(MS_U8 u8MvAdjust, MS_U8 u8IdctSel);

void MDrv_MVD_SetBitStreamAddr(MS_U32 u32start, MS_U32 u32end);
void MDrv_MVD_SetFrameBuffAddr(MS_U32 u32addr);
void MDrv_MVD_GetFrameInfo(MVD_FrameInfo *pinfo );
void MDrv_MVD_SetOverflowTH (MS_U32 u32Threshold);
void MDrv_MVD_SetUnderflowTH (MS_U32 u32Threshold);

void MDrv_MVD_RstIFrameDec( void );
MS_BOOL MDrv_MVD_GetIsIFrameDecoding( void );

MS_U8 MDrv_MVD_GetSyncStatus( void );
MS_U8 MDrv_MVD_GetIsIPicFound( void );

//for MM
MS_U32 MDrv_MVD_GetResidualStreamSize( void );

MS_BOOL MDrv_MVD_DecodeIFrame(MS_PHYADDR u32FrameBufAddr, MS_PHYADDR u32StreamBufAddr, MS_PHYADDR u32StreamBufEndAddr );
MS_BOOL MDrv_MVD_GetValidStreamFlag( void );

//for MM
void MDrv_MVD_SetFrameInfo(MVD_FrameInfo *pinfo );
void MDrv_MVD_SetSLQStartEnd(  MS_U32 u32start, MS_U32 u32end);
void MDrv_MVD_GetErrInfo(MVD_ErrCode *errCode, MVD_ErrStatus *errStatus);
MS_U32 MDrv_MVD_GetSkipPicCounter( void );

MS_U8 MDrv_MVD_GetSLQAvailableLevel(void);
void MDrv_MVD_SetSLQWritePtr( MS_U32 writePtr );
MS_U32 MDrv_MVD_GetSLQReadPtr( void );
void MDrv_MVD_SetSLQTblBufStartEnd( MS_U32 u32start, MS_U32 u32end );

MVD_PicType MDrv_MVD_GetPicType( void );
MS_U32 MDrv_MVD_GetSWIdx( void );
MS_U32 MDrv_MVD_GetBitsRate( void );
MS_U8 MDrv_MVD_GetVideoRange(void);
MS_BOOL MDrv_MVD_GetLowDelayFlag( void );

void MDrv_MVD_Pause(void);
void MDrv_MVD_Resume(void);
MS_BOOL MDrv_MVD_StepDisp(void);
MS_BOOL MDrv_MVD_IsStepDispDone(void);
MS_BOOL MDrv_MVD_StepDecode(void);
MS_BOOL MDrv_MVD_IsStepDecodeDone(void);
MS_BOOL MDrv_MVD_SeekToPTS(MS_U32 u32Pts);
MS_BOOL MDrv_MVD_IsStep2PtsDone(void);
MS_BOOL MDrv_MVD_SkipToPTS(MS_U32 u32Pts);
MS_BOOL MDrv_MVD_TrickPlay(MVD_TrickDec trickDec, MS_U8 u8DispDuration);
void MDrv_MVD_EnableForcePlay(void);

void MDrv_MVD_RegSetBase(MS_U32 u32RegBaseAddr);
MS_BOOL MDrv_MVD_Init(void);
MS_BOOL MDrv_MVD_Exit(void);
E_MVD_Result MDrv_MVD_Rst( void );

void MDrv_MVD_Play( void );
void MDrv_MVD_SetAVSync(MS_BOOL bEnable, MS_U32 u32Delay);
void MDrv_MVD_SetAVSyncThreshold(MS_U32 u32Th);
MS_BOOL MDrv_MVD_GetIsAVSyncOn(void);
MS_BOOL MDrv_MVD_GetIsSyncRep(void);
MS_BOOL MDrv_MVD_GetIsSyncSkip(void);
MS_BOOL MDrv_MVD_ChangeAVsync(MS_BOOL bEnable, MS_U16 u16PTS);
MS_BOOL MDrv_MVD_DispCtrl(MS_BOOL bDecOrder, MS_BOOL bDropErr, MS_BOOL bDropDisp, MVD_FrcMode eFrcMode);
MS_BOOL MDrv_MVD_DispRepeatField(MS_BOOL bEnable);
MS_BOOL MDrv_MVD_GetTop1stField(void);
MS_BOOL MDrv_MVD_GetRepeat1stField(void);
MS_BOOL MDrv_MVD_GetTmpRefField(void);

MS_U8 MDrv_MVD_GetActiveFormat( void );
MS_U8 MDrv_MVD_GetDispRdy( void );
MS_BOOL MDrv_MVD_Is1stFrmRdy(void);
MS_U32 MDrv_MVD_GetGOPCount( void );
MS_U32 MDrv_MVD_GetPicCounter(void);
MS_U32 MDrv_MVD_GetParserByteCnt(void);
MVD_DecStat MDrv_MVD_GetDecodeStatus(void);
MS_U8 MDrv_MVD_GetLastCmd(void);
MS_U32 MDrv_MVD_GetVldErrCount( void );
MS_BOOL MDrv_MVD_DropErrorFrame(MS_BOOL bDrop);
MS_BOOL MDrv_MVD_MVDCommand ( MS_U8 u8cmd, MVD_CmdArg *pstCmdArg );
MS_BOOL MDrv_MVD_SkipData(void);
MS_BOOL MDrv_MVD_SkipToIFrame(void);
MS_BOOL MDrv_MVD_GetCaps(MVD_Caps* pCaps);

E_MVD_Result MDrv_MVD_DisableErrConceal(MS_BOOL bDisable);
E_MVD_Result MDrv_MVD_PushQueue(MVD_PacketInfo* pInfo);
E_MVD_Result MDrv_MVD_FlushQueue(void);
MS_BOOL MDrv_MVD_FlushDisplayBuf(void);
MS_U32 MDrv_MVD_GetQueueVacancy(MS_BOOL bCached);
MS_U32 MDrv_MVD_GetESReadPtr(void);
MS_U32 MDrv_MVD_GetESWritePtr(void);
E_MVD_Result MDrv_MVD_EnableLastFrameShow(MS_BOOL bEnable);
E_MVD_Result MDrv_MVD_IsDispFinish(void);
E_MVD_Result MDrv_MVD_SetSpeed(MVD_SpeedType eSpeedType, MS_U8 u8Multiple);
E_MVD_Result MDrv_MVD_ResetPTS(MS_U32 u32PtsBase);
MS_U32 MDrv_MVD_GetPTS(void);
MVD_TrickDec MDrv_MVD_GetTrickMode(void);
MS_BOOL MDrv_MVD_IsPlaying(void);
MS_BOOL MDrv_MVD_IsIdle(void);
MS_BOOL MDrv_MVD_IsSeqChg(void);
E_MVD_Result MDrv_MVD_DbgSetData(MS_U32 u32Addr, MS_U32 u32Data);
E_MVD_Result MDrv_MVD_DbgGetData(MS_U32 u32Addr, MS_U32* u32Data);
MS_U8 MDrv_MVD_GetDecodedFrameIdx ( void );
MS_BOOL MDrv_MVD_SetFileModeAVSync(MVD_TIMESTAMP_TYPE eSyncMode);
MS_BOOL MDrv_MVD_IsAllBufferEmpty(void);
MS_BOOL MDrv_MVD_GenPattern(MVD_PatternType ePattern, MS_PHYADDR u32PAddr, MS_U32* pu32Size);
MS_U32 MDrv_MVD_GetPatternInfo(void);
E_MVD_Result MDrv_MVD_SetDynScalingParam(MS_PHYADDR u32StAddr, MS_U32 u32Size);
MS_BOOL MDrv_MVD_SetDynamicScaleAddr(MS_U32 u32addr);
MS_BOOL MDrv_MVD_SetVirtualBox(MS_U16 u16Width, MS_U16 u16Height);
MS_BOOL MDrv_MVD_EnableQDMA(void);
MS_BOOL MDrv_MVD_SetBlueScreen(MS_BOOL bEn);
E_MVD_Result MDrv_MVD_EnableDispOneField(MS_BOOL bEn);
E_MVD_Result MDrv_MVD_GetFrmInfo(MVD_FrmInfoType eType, MVD_FrmInfo* pInfo);
E_MVD_Result MDrv_MVD_SetFreezeDisp(MS_BOOL bEn);

//ATSC Closed Caption control
E_MVD_Result MDrv_MVD_CCRst(MVD_CCCfg* pCCParam);
E_MVD_Result MDrv_MVD_CCStartParsing(MVD_CCCfg* pCCParam);
E_MVD_Result MDrv_MVD_CCStopParsing(MVD_CCFormat eCCFormat);
E_MVD_Result MDrv_MVD_CCGetWritePtr(MVD_CCFormat eCCFormat, MS_U32* pWrite);
E_MVD_Result MDrv_MVD_CCGetReadPtr(MVD_CCFormat eCCFormat, MS_U32* pRead);
E_MVD_Result MDrv_MVD_CCUpdateReadPtr(MVD_CCFormat eCCFormat, MS_U32 u32EachPacketSize);
E_MVD_Result MDrv_MVD_CCGetIsOverflow(MVD_CCFormat eCCFormat, MS_BOOL* pbOverflow);
MS_BOOL MDrv_MVD_SetSkipRepeatMode(MS_U8 u8Mode);

void MDrv_MVD_DbgDump(void);

#if(defined(CHIP_T2) || defined(CHIP_U3))
#define MVD_ENABLE_ISR
#endif

#ifdef MVD_ENABLE_ISR
E_MVD_Result MDrv_MVD_SetIsrEvent(MVD_Event eEvent, MVD_InterruptCb fnHandler);
MVD_Event MDrv_MVD_GetIsrEvent(void);
#else
#define MDrv_MVD_SetIsrEvent(x, y) (E_MVD_RET_OK)
#define MDrv_MVD_GetIsrEvent()     (MVD_Event)0
#endif

#ifdef __cplusplus
}
#endif

#endif
