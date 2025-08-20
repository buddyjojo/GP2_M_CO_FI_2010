
#ifndef _MDRV_TSP_ST_H_
#define _MDRV_TSP_ST_H_

#include "mdrv_types.h"

//------------------------------------------------------------------------------
// Constant definition
//------------------------------------------------------------------------------

/// TSP DDI return value
typedef U32                         DRVTSP_RESULT;

#define DRVTSP_ALIGN(_x)            ALIGN_8((U32)_x)                    /// TSP byte address alignment macro
#define DRVTSP_ALIGN_UNIT           8                                   /// TSP byte address alignment unit

#define DRVTSP_PID_NULL             0x1FFF                              /// Transport stream null PID
#define DRVTSP_TS_PACKET_SIZE       188                                 /// Transport stream packet size

/// @name DRVTSP_RESULT
/// @ref DRVTSP_RESULT
/// return value
/// @{
#define DRVTSP_OK                   0x00000000
#define DRVTSP_FAIL                 0x00000001
#define DRVTSP_INVALID_PARAM        0x00000002
#define DRVTSP_FUNC_ERROR           0x00000003
#define DRVTSP_INVALID_SECFLT       0x00000004
/// @}

/// TSP notification event message
typedef struct //_DrvTSP_Msg
{
    /// Union data type of message
    union
    {
        /// FltInfo message
        ///   - Byte[0] : Section filter id
        ///   - Byte[1] : TSP id
        U32                         FltInfo;
        /// PvrBufId
        ///   - Byte[0] : PVR buffer id
        U32                         PvrBufId;
    };

    U32                         ReadPtr;
    U32                         WritePtr;
    U32                         DataSize;
    U32                         BufEnd;
    U32                         BufSize;
} DrvTSP_Msg;

/// @name DrvTSP_Msg
/// Macro definitions for manipulating DrvTSP_Msg
/// @{
#define MSG_FLTINFO_SEC_ID_MASK     0x000000FF
#define MSG_FLTINFO_SEC_ID_SHFT     0
#define MSG_FLTINFO_ENG_ID_MASK     0x0000FF00
#define MSG_FLTINFO_ENG_ID_SHFT     8
#define MSG_PVRBUF_ID_MASK          0x000000FF
#define MSG_PVRBUF_ID_SHFT          0
#define MSG_PVRBUF_ID_NULL          0xFF
/// @}

// Don't change the order unless you are very sure what you are doing
typedef enum
{
    E_DRVTSP_FLT_TYPE_VIDEO         = 0x00000000,
    E_DRVTSP_FLT_TYPE_AUDIO         ,
    E_DRVTSP_FLT_TYPE_AUDIO2        ,
    E_DRVTSP_FLT_TYPE_PVR           ,

    // Section Filter Group
    E_DRVTSP_FLT_TYPE_SECTION       ,
    E_DRVTSP_FLT_TYPE_PCR           ,
    E_DRVTSP_FLT_TYPE_PES           ,
    E_DRVTSP_FLT_TYPE_PACKET        ,
    E_DRVTSP_FLT_TYPE_TELETEXT      ,

    E_DRVTSP_FLT_SOURCE_TYPE_MASK   =  0xC0000000,
    E_DRVTSP_FLT_SOURCE_TYPE_LIVE   =  0x80000000,
    E_DRVTSP_FLT_SOURCE_TYPE_FILE   =  0x40000000,

    E_DRVTSP_FLT_TYPE_LAST_ENUM
} DrvTSP_FltType;

// TS Interface
typedef enum
{
    E_DRVTSP_TSIF_SERIAL        = 0x0,
    E_DRVTSP_TSIF_PARALLEL      = 0x1,
} DrvTSP_TSInterface;

// Source of PID filters
typedef enum
{
    E_DRVTSP_SRC_TS                 = 0x0,
    E_DRVTSP_SRC_FILE               = 0x1
} DrvTSP_SourceType;

// Don't change the order unless you are very sure what you are doing
typedef enum
{
    E_DRVTSP_FLT_STATE_FREE         = 0x00000000,                                                   ///<\n
    E_DRVTSP_FLT_STATE_ALLOC        = 0x00000001,                                                   ///<\n
    E_DRVTSP_FLT_STATE_ENABLE       = 0x00000002,                                                   ///<\n
    E_DRVTSP_FLT_STATE_SCRAMBLED    = 0x00000004,                                                   //[TODO]
    E_DRVTSP_FLT_STATE_OVERFLOW     = 0x00010000,                                                   //[Reserved]
} DrvTSP_FltState;

// Don't change the order unless you are very sure what you are doing
typedef enum
{
    E_DRVTSP_FLT_MODE_CONTI         = 0x00000000,
    E_DRVTSP_FLT_MODE_ONESHOT       = 0x00000001,
    E_DRVTSP_FLT_MODE_CRCCHK        = 0x00000002,
} DrvTSP_FltMode;

typedef enum{
    E_DRVTSP_SCMB_NONE=         0x00000000,
    E_DRVTSP_SCMB_TS=           0x00000001,
    E_DRVTSP_SCMB_PES=          0x00000002,
    E_DRVTSP_SCMB_TS_PES=       (E_DRVTSP_SCMB_TS| E_DRVTSP_SCMB_PES),
} DrvTSP_Scmb_Level;

// Don't change the order unless you are very sure what you are doing
typedef enum
{
    /// Section Data Ready
    E_DRVTSP_EVENT_DATA_READY       = 0x00000001,
    /// Section Buffer Overflow
    E_DRVTSP_EVENT_BUF_OVERFLOW     = 0x00000002,
    /// PVR Buffer is Full
    E_DRVTSP_EVENT_PVRBUF_FULL      = 0x00000010,
    /// PVR Double Buffer Overflow
    E_DRVTSP_EVENT_PVRBUF_OVERFLOW  = 0x00000020,
} DrvTSP_Event;

// Don't change the order unless you are very sure what you are doing
typedef enum
{
    /// Record ENG0 by @ref E_DRVTSP_FLT_TYPE_PVR
    E_DRVTSP_REC_MODE_ENG0_FLTTYPE  = 0x00000000,                       // TSP_PVR_CTRL_ENG(0)
    /// Record ENG1 by @ref E_DRVTSP_FLT_TYPE_PVR
    E_DRVTSP_REC_MODE_ENG1_FLTTYPE  = 0x00000001,                       // TSP_PVR_CTRL_ENG(1)
    /// Record ENG0 bypass PID fliter
    E_DRVTSP_REC_MODE_ENG0_BYPASS   = 0x00000002,                       // TSP_PVR_CTRL_ENG(0) + TSP_PVR_CTRL_BYPASS
    /// Record ENG1 bypass PID fliter
    E_DRVTSP_REC_MODE_ENG1_BYPASS   = 0x00000003,                       // TSP_PVR_CTRL_ENG(1) + TSP_PVR_CTRL_BYPASS
} DrvTSP_RecMode;

// Don't change the order unless you are very sure what you are doing
typedef enum
{
    /// TSP Engine 0 Transport Stream
    E_DRVTSP_M2T_MODE_ENG0_TS       = 0x00000000,                       //TSP_TSDMA_CTRL_ENG0
    /// TSP Engine 0 Video PES Only
    E_DRVTSP_M2T_MODE_ENG0_VPES     = 0x00000004,                       //TSP_TSDMA_CTRL_VPES0
    /// TSP Engine 0 Audio PES Only
    E_DRVTSP_M2T_MODE_ENG0_APES     = 0x00000008,                       //TSP_TSDMA_CTRL_APES0
} DrvTSP_M2tMode;

// Don't change the order unless you are very sure what you are doing
typedef enum
{
    /// Command Queue is Idle
    E_DRVTSP_M2T_STATE_IDLE         = 0000000000,
    /// Command Queue is Busy
    E_DRVTSP_M2T_STATE_BUSY         = 0x00000001,
    /// Command Queue is Paused.
    E_DRVTSP_M2T_STATE_PAUSE        = 0x00000002,
} DrvTSP_M2tState;

/// TSP m2t state
typedef enum
{
    /// Input From Stream Source 0
    E_DRVTSP_CTRL_MODE_TS0,
    /// Input From Stream Source 1
    E_DRVTSP_CTRL_MODE_TS1,
    /// Input From Memory
    E_DRVTSP_CTRL_MODE_MEM,
    /// Input From Memory
    E_DRVTSP_CTRL_MODE_MEM_PVR,
    /// Input From Stream Source 0, enable output to MAD
    E_DRVTSP_CTRL_MODE_TS0_AUD,
    /// Input From Stream Source 1, enable output to MAD
    E_DRVTSP_CTRL_MODE_TS1_AUD,
    /// MEM to AFIFO
    E_DRVTSP_CTRL_MODE_MEM_AUD,
    /// MEM to VFIFO (PS/ES mode)
    E_DRVTSP_CTRL_MODE_MEM_VID,
    /// Enable bounding option for PVR descrambled stream
    E_DRVTSP_CTRL_MODE_PVR_TS0,
    /// PVR and MEM playback (time shift)
    E_DRVTSP_CTRL_MODE_PVR_AND_MEM
} DrvTSP_CtrlMode;

/// TSP m2t state
typedef enum
{
    //E_DRVTSP_PAD_PLAYCARD           = 0x0,
    E_DRVTSP_PAD_CI                 = 0x1,
    E_DRVTSP_PAD_DEMOD              = 0x2,
    E_DRVTSP_PAD_DEMOD_INV          = 0x3,
    E_DRVTSP_PAD_EXT_INPUT0         = 0x4,
    E_DRVTSP_PAD_EXT_INPUT1         = 0x5
} DrvTSP_PadIn;

typedef enum
{
    E_DRVTSP_CFG_ENG_NUM            = 0,
    E_DRVTSP_CFG_PIDFLT0_NUM   ,
    E_DRVTSP_CFG_PIDFLT1_NUM,
    E_DRVTSP_CFG_SECFLT_NUM,
    E_DRVTSP_CFG_FLT_DEPTH,
    E_DRVTSP_CFG_SDR_BASE
} DrvTSP_Cfg_Id;

typedef enum
{
    E_DRVTSP_CSA_DATAPATH_INPUT_LIVE_IN        = BIT0,
    E_DRVTSP_CSA_DATAPATH_INPUT_FILE_IN        = BIT1,
    E_DRVTSP_CSA_DATAPATH_INPUT_REC_LIVE       = BIT2,
    E_DRVTSP_CSA_DATAPATH_OUTPUT_PLAY_LIVE     = BIT4, // Skip 3
    E_DRVTSP_CSA_DATAPATH_OUTPUT_PLAY_FILE     = BIT5,
    E_DRVTSP_CSA_DATAPATH_OUTPUT_REC_DESCRMB   = BIT6
} DrvTSP_CSA_DataPath;

typedef enum
{
    E_DRVTSP_CSA_PROTOCOL_DES        = 0,
    E_DRVTSP_CSA_PROTOCOL_AES        = 1
} DrvTSP_CSA_Protocol;

typedef enum
{
    E_DRVTSP_CSA_KEY_EVEN        = 0,
    E_DRVTSP_CSA_KEY_ODD         = 1
} DrvTSP_CSA_Key;

/// TSP file in Packet mode
typedef enum //_DrvTSP_PacketMode
{
    E_DRVTSP_PKTMODE_188 = 0x00000000,
    E_DRVTSP_PKTMODE_192 = 0x00000001,
    E_DRVTSP_PKTMODE_204 = 0x00000002,
} DrvTSP_PacketMode;

//------------------------------------------------------------------------------
// Data structure
//------------------------------------------------------------------------------
// TSP_IOC_PIDFLT_ALLOC
typedef struct
{
    unsigned int                u32EngId;
    DrvTSP_FltType              eFilterType;
    unsigned int                u32PidFltId;
} DrvTSP_PidFlt_Alloc_t;

// TSP_IOC_PIDFLT_FREE
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32PidFltId;
} DrvTSP_PidFlt_Free_t;

// TSP_IOC_PIDFLT_SET_SRC
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32PidFltId;
    unsigned int                u32Src;
} DrvTSP_PidFlt_Set_Src_t;

// TSP_IOC_PIDFLT_SET
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32PidFltId;
    unsigned int                u32Pid;
} DrvTSP_PidFlt_Pid_t;

// TSP_IOC_PIDFLT_SEL_SECFLT
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32PidFltId;
    unsigned int                u32SecFltId;
} DrvTSP_PidFlt_SelSecFlt_t;

// TSP_IOC_PIDFLT_ENABLE
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32PidFltId;
    unsigned int                bEnable;
} DrvTSP_PidFlt_Enable_t;

// TSP_IOC_PIDFLT_STATE
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32PidFltId;
    DrvTSP_FltState             eFltState;
} DrvTSP_PidFlt_State_t;

// TSP_IOC_PIDFLT_SCMB_STATUS
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32PidFltId;
    DrvTSP_Scmb_Level           eScmbLevel;
} DrvTSP_PidFlt_Scmb_Status_t;

// TSP_IOC_SECFLT_ALLOC
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
} DrvTSP_SecFlt_Alloc_t;

// TSP_IOC_SECFLT_FREE
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
} DrvTSP_SecFlt_Free_t;

// TSP_IOC_SECFLT_MODE
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    DrvTSP_FltMode              eSecFltMode;
} DrvTSP_SecFlt_Mode_t;

// TSP_IOC_SECFLT_PATTERN
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32FltDepth;
    unsigned char*              pu8Match;       // user space
    unsigned char*              pu8Mask;        // user space
} DrvTSP_SecFlt_Pattern_t;

// TSP_IOC_SECFLT_REQCOUNT
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32ReqCount;
} DrvTSP_SecFlt_ReqCount_t;

// TSP_IOC_SECFLT_BUFFER_RESET
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
} DrvTSP_SecFlt_Buffer_Reset;

// TSP_IOC_SECFLT_BUFFER_SET
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32BufAddr; // Physical address
    unsigned int                u32BufSize;
} DrvTSP_SecFlt_Buffer_Set_t;

// TSP_IOC_SECFLT_BUFFER_ADRR
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32BufAddr; // Physical address
} DrvTSP_SecFlt_Buffer_Addr_Get_t;

// TSP_IOC_SECFLT_BUFFER_SIZE
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32BufSize; // Physical address
} DrvTSP_SecFlt_Buffer_Size_Get_t;

// TSP_IOC_SECFLT_BUFFER_READ_GET
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32Read; // Physical address
} DrvTSP_SecFlt_Buffer_Read_Get_t;

// TSP_IOC_SECFLT_BUFFER_READ_SET
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32Read; // Physical address
} DrvTSP_SecFlt_Buffer_Read_Set_t;

// TSP_IOC_SECFLT_BUFFER_WRITE_GET
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32Write; // Physical address
} DrvTSP_SecFlt_Buffer_Write_Get_t;

// TSP_IOC_SECFLT_CRC32
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    unsigned int                u32Crc;
} DrvTSP_SecFlt_Crc32_t;

// TSP_IOC_SECFLT_NOTIFY
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    DrvTSP_Event                eEvents;
} DrvTSP_SecFlt_Notify_t;

// TSP_IOC_SECFLT_STATE_GET
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32SecFltId;
    DrvTSP_FltState             eState;
} DrvTSP_SecFlt_State_t;

// TSP_IOC_PVR_BUFFER_SET
typedef struct
{
    unsigned int                u32BufAddr0; // Physical address
    unsigned int                u32BufAddr1; // Physical address
    unsigned int                u32BufSize;
} DrvTSP_Pvr_Buffer_Set_t;

// TSP_IOC_PVR_START;
typedef struct
{
    DrvTSP_RecMode              eRecMode;
    unsigned int                bStart;
} DrvTSP_Pvr_Start_t;

// TSP_IOC_M2T_STC_SET
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32STC_32;
    unsigned int                u32STC;
} DrvTSP_M2T_Stc_Set_t;

// TSP_IOC_CSA_STATUS
typedef struct
{
    unsigned int                u32EngId;
    DrvTSP_Scmb_Level           eScmbLevel;
} DrvTSP_Scmb_Status_t;

// TSP_IOC_SET_SCRM_PATH
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                bEnableOutputAV;
    DrvTSP_CSA_DataPath         eInPath;
    DrvTSP_CSA_DataPath         eOutPath;
} DrvTSP_Set_Scmb_Path_t;

// TSP_IOC_SET_ESA_MODE
typedef struct
{
    unsigned int                u32EngId;
    DrvTSP_CSA_Protocol         eProtocol;
} DrvTSP_Set_Esa_Mode_t;

// TSP_IOC_SET_CIPHERKEY
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32FID;
    unsigned int                *pu32CipherKey;
    DrvTSP_CSA_Protocol         eProtocol;
    DrvTSP_CSA_Key              eKey;
} DrvTSP_Set_CipherKey_t;

//------------------------------------------------------------------------------
// Misc
//------------------------------------------------------------------------------
// TSP_IOC_GET_STC
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32STC_32;
    unsigned int                u32STC;
} DrvTSP_Stc_Get_t;

// TSP_IOC_SET_MODE
typedef struct
{
    unsigned int                u32EngId;
    DrvTSP_CtrlMode             eCtrlMode;
} DrvTSP_Set_Mode_t;

// TSP_IOC_SEL_PAD
typedef struct
{
    unsigned int                u32EngId;
    DrvTSP_PadIn                ePad;
} DrvTSP_Sel_Pad_t;

// TSP_IOC_SIGNAL
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32Event;
    unsigned int                u32EventSecRdy;
    unsigned int                u32EventSecOvf;
} DrvTSP_Signal_t;

// TSP_IOC_SET_TSIF_TYPE
typedef struct
{
    unsigned int                u32EngId;
    unsigned int                u32TSIFType;
} DrvTSP_Set_TSIF_Type_t;

#define DRVTSP_CFG_STATUS_OK             0
#define DRVTSP_CFG_STATUS_ERR_ID        -1
typedef struct
{
    unsigned int                u32CfgId;
    unsigned int                u32Status;
    unsigned int                u32Value;
} DrvTSP_Cfg_t;

/// TSP notification function
typedef void (*P_DrvTSP_EvtCallback)(DrvTSP_Event eEvent, DrvTSP_Msg *pMsg);

// TSP_IOC_FIFO_FLUSH
typedef struct
{
    DrvTSP_FltType              eFilterType;
    B16                         bFlush;
} DrvTSP_Fifo_Flush_t;

// TSP_IOC_FIFO_FETCH
typedef struct
{
    DrvTSP_FltType              eFilterType;
    unsigned char               u8Byte;
} DrvTSP_Fifo_Fetch_t;

#if 0
// TSP_IOC_DMXFLT_ALLOC
typedef struct
{
    DrvTSP_FltType              eFilterType;
    unsigned int                u32FltId;
} DrvTSP_DmxFlt_Alloc_t;

// TSP_IOC_DMXFLT_SET
typedef struct
{
    unsigned int                u32FltId;
    unsigned int                u32Pid;
} DrvTSP_DmxFlt_Set_t;

// TSP_IOC_DMXFLT_SET_SECBUF
typedef struct
{
    unsigned int                u32FltId;
    unsigned int                u32SecBufId;
} DrvTSP_DmxFlt_SetSecBuf_t;

// TSP_IOC_DMXFLT_GET_SECBUF
typedef struct
{
    unsigned int                u32FltId;
    unsigned int                u32SecBufId;
} DrvTSP_DmxFlt_GetSecBuf_t;

// TSP_IOC_DMXFLT_ENABLE
typedef struct
{
    unsigned int                u32FltId;
    unsigned int                bEnable;
} DrvTSP_DmxFlt_Enable;

// TSP_IOC_DMXFLT_SETMODE
typedef struct
{
    unsigned int                u32FltId;
    DrvTSP_FltMode              eSecFltMode;
} DrvTSP_DmxFlt_SetMode;

// TSP_IOC_DMXFLT_SETPATTERN
typedef struct
{
    unsigned int                u32FltId;
    unsigned char               u8Match[TSP_FILTER_DEPTH];
    unsigned char               u8Mask[TSP_FILTER_DEPTH];
    unsigned int                bNotMatch;
} DrvTSP_DmxFlt_SetPattern;

// TSP_IOC_DMXFLT_GETSTATE
typedef struct
{
    unsigned int                u32FltId;
    DrvTSP_FltState             eState;
} DrvTSP_DmxFlt_GetState;

// TSP_IOC_DMXFLT_NOTIFY
typedef struct
{
    unsigned int                u32FltId;
    DrvTSP_Event                eEvents;
} DrvTSP_DmxFlt_Notify;

// TSP_IOC_DMXFLT_REQCOUNT
typedef struct
{
    unsigned int                u32FltId;
    unsigned int                u32ReqCount;
} DrvTSP_DmxFlt_ReqCount;

// TSP_IOC_DMXFLT_CRC32
typedef struct
{
    unsigned int                u32FltId;
    unsigned int                u32Crc;
} DrvTSP_DmxFlt_CRC32;


//------------------------------------------------------------------------------
// Dmx Sec Buf
//------------------------------------------------------------------------------
// TSP_IOC_DMXSECBUF_BUF_SET
typedef struct
{
    unsigned int                u32SecBufId;
    unsigned int                u32BufAddr;
    unsigned int                u32BufSize;
} DrvTSP_DmxSecBuf_BufSet;

// TSP_IOC_DMXSECBUF_BUF_BUF_ADDR
typedef struct
{
    unsigned int                u32SecBufId;
    unsigned int                u32BufAddr;
} DrvTSP_DmxSecBuf_BufAddr;

// TSP_IOC_DMXSECBUF_BUF_BUF_SIZE
typedef struct
{
    unsigned int                u32SecBufId;
    unsigned int                u32BufSize;
} DrvTSP_DmxSecBuf_BufSize;

// TSP_IOC_DMXSECBUF_BUF_READ_GET
typedef struct
{
    unsigned int                u32SecBufId;
    unsigned int                u32Read;
} DrvTSP_DmxSecBuf_BufReadGet;

// TSP_IOC_DMXSECBUF_BUF_READ_SET
typedef struct
{
    unsigned int                u32SecBufId;
    unsigned int                u32Read;
} DrvTSP_DmxSecBuf_BufReadSet;

// TSP_IOC_DMXSECBUF_BUF_WRITE_GET
typedef struct
{
    unsigned int                u32SecBufId;
    unsigned int                u32Write;
} DrvTSP_DmxSecBuf_BufWriteGet;


//------------------------------------------------------------------------------
// Dmx DeScmb
//------------------------------------------------------------------------------
// TSP_IOC_DMXDESCMB_PID
typedef struct
{
    unsigned int                u32DescmbId;
    unsigned int                u32Pid;
} DrvTSP_DmxDescmb_Pid;

// TSP_IOC_DMXDESCMB_KEY
typedef struct
{
    unsigned int                u32DescmbId;
    unsigned int                u32KeyAddr;
    unsigned int                bOddKey;
} DrvTSP_DmxDescmb_Key;
#endif

#endif
