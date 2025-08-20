
#ifndef _MDRV_MVD_ST_H_
#define _MDRV_MVD_ST_H_

#include "mdrv_types.h"

#define MVD_PAGE_NUM            32      // 16+16 => 16(64K) for callback; 16(64K) for file playback
#define DefSizeOfPictureData   0x8000          //32KB

typedef struct _MVD_PIC_HEADER_T{
    U8  u8PicType;          // picture type: 1 -> I; 2 -> P; 3 -> B
    U8  u8Top_ff;           // Top field first: 1 if top field first
    U8  u8Rpt_ff;           // Repeat first field: 1 if repeat field first
    U16 u16TmpRef;          // Temporal reference of the picture
}MVD_PIC_HEADER_T, *pMVD_PIC_HEADER_T;

typedef struct _MVD_SEQUENCE_HEADER_T{
    U16 u16HorSize;         ///< Horizontal size
    U16 u16VerSize;         ///< Vertical size
    U16 u16FrameRate;       ///< Frame rate
    U8 u8AspectRatio;       ///< Aspect Ratio
    U8 u8Progressive;       ///< progressive or interleave
    U32 u32BitRate;         ///< Bit-rate
}MVD_SEQUENCE_HEADER_T, *pMVD_SEQUENCE_HEADER_T;

typedef struct _MVD_SPECIFIC_STATUS_T{
    U8  bDataError;                         //detect invalid ES stream
    U8  bPictureDecodingError;              //check ES has any syntax error
    U8  bBitStreamBufferOverflow;           //buffer overflow
    U8  bBitStreamBufferUnderflow;          //buffer underflow
    U8  bGetFirstFrame;                     //Got first frame
    U8  bDisplayReady;                      //MVD is ready to display
    U8  bSequenceHeaderDetected;            //sequence header detected
    U8  bVideoSkip;                         //if AV is not sync. the video is skipped
    U8  bVideoRepeat;                       //if AV is not sync. the video is repeated
}MVD_SPECIFIC_STATUS_T, *pMVD_SPECIFIC_STATUS_T;

typedef struct _MVD_CallBackMsg{
    pMVD_PIC_HEADER_T           pPicHdrData;
    U32                         u32PicHdrSize;
    pMVD_SEQUENCE_HEADER_T      pSeqHdrData;
    U32                         u32SeqHdrSize;
    pMVD_SPECIFIC_STATUS_T      pSpecificStatus;
    U32                         u32SpecificStatusSize;
    U8                          *pUserData;
    U32                         u32UserDataSize;
}MVD_CallBackMsg, *pMVD_CallBackMsg;

typedef struct _MVD_Data_Pkt{
    U32     u32NumOfDataPkt;
    U32     u32SizeOfDataPkt;
    U32     u32TypeOfStruct;
    U8      *pPktData;
}MVD_Data_Pkt, *pMVD_Data_Pkt;

typedef struct _MVD_FIRST_PKT_T{
    U32 u32INTStatus;         ///< summary packet size
    U32 u32BufferOffset;          ///< Buffer offset
}MVD_FIRST_PKT_T, *pMVD_FIRST_PKT_T;

//bit-wise definition for subscribing interrupt event
#define EN_PIC_USER_DATA        0x0001          //picture user data
#define EN_SEQ_HDR_DATA         0x0002          //sequence header
#define EN_PIC_HDR_DATA         0x0004          //picture header
#define EN_SPECIFIC_STATUS      0x0008          //for observing some MVD status
//#define EN_DATA_ERROR           0x0008          //data error (invaild mpeg2 video stream)
//#define EN_PIC_DEC_ERROR        0x0010          //check ES has any syntax error
//#define EN_DEC_OVERFLOW         0x0020          //decoder overflow
//#define EN_DEC_UNDERFLOW        0x0040          //decoder underflow
//#define EN_GER_FIRST_FRAME      0x0080          //got first I frame
//#define EN_GET_DISP_RDY         0x0100          //MVD is ready to display
//#define EN_VIDEO_SKIP           0x0200          //if video is skipped
//#define EN_VIDEO_REPEAT         0x0400          //if video is repeated

#define DATA_SIZE 256
#define PKT_SIZE  (sizeof(U32) * 3 + DATA_SIZE)

typedef U32(*pfnMVDInterruptCB)(pMVD_CallBackMsg pMVD_CallBackMsgData);

//------------------------------------------------------------------------------
// Type and Structure Declaration
//------------------------------------------------------------------------------
/// Define input stream type.
typedef enum tag_MVD_StreamType
{
    /// Input stream is TS stream.
    E_MVD_StreamType_TS_STREAM      = 0,
    /// Input stream is TS file.
    E_MVD_StreamType_TS_FILE        = 1,
    /// Input stream is PS file.
    E_MVD_StreamType_PS_FILE        = 2,
    /// Input stream is PS file(ES file).
    E_MVD_StreamType_PS_ES_FILE     = 3
} __attribute__((packed)) MVD_StreamType, *pMVD_StreamType;

/// Define MPEG video frame rate.
typedef enum tag_MVD_FrameRate
{
    /// MPEG video frame rate is temporarily not available.
    E_MVD_FrameRate_Invalid 	= -2,
    /// MPEG video frame rate is unknow.
    E_MVD_FrameRate_Unknown	    = -1,
    /// MPEG video frame rate is 23.976 HZ.
    E_MVD_FrameRate_23_976Hz    = 1,
    /// MPEG video frame rate is 24 HZ.
    E_MVD_FrameRate_24Hz 	    = 2,
    /// MPEG video frame rate is 25 HZ.
    E_MVD_FrameRate_25Hz 	    = 3,
    /// MPEG video frame rate is 29.97 HZ.
    E_MVD_FrameRate_29_97Hz 	= 4,
    /// MPEG video frame rate is 30 HZ.
    E_MVD_FrameRate_30Hz		= 5,
    /// MPEG video frame rate is 50 HZ.
    E_MVD_FrameRate_50Hz 	    = 6,
    /// MPEG video frame rate is 59.94 HZ.
    E_MVD_FrameRate_59_94Hz	    = 7,
    /// MPEG video frame rate is 60 HZ.
    E_MVD_FrameRate_60Hz		= 8
}__attribute__((packed)) MVD_FrameRate, *pMVD_FrameRate;

/// Define MPEG video scan type
typedef enum tag_MVD_ScanType
{
    /// MPEG video scan type id unknow.
    E_MVD_ScanType_Unknown 	    = -1,
    /// MPEG video scan type is interlace.
    E_MVD_ScanType_Interlaced 	= 0,
    /// MPEG video scan type is progressive.
    E_MVD_ScanType_Progressive	= 1
} __attribute__((packed)) MVD_ScanType, *pMVD_ScanType;

/// Define MVD control state.
typedef enum tag_MVD_State
{
    /// MVD control state is unknow.
    E_MVD_State_Unknown 				= -1,
    /// MVD control state is initial.
    E_MVD_State_Init					= 0,
    /// MVD control state is play.
    E_MVD_State_Play	 				= 1,
    /// MVD control state is pause
    E_MVD_State_Paused				    = 2,
    /// MVD control state is stop done.
    E_MVD_State_Stop_Done			    = 8,
    /// MVD control state is decode I frame
    E_MVD_State_Decode_I_Frame		    = 9,
} __attribute__((packed)) MVD_State, *pMVD_State;

/// Define MVD error.
typedef enum tag_MVD_Error
{
    /// MVD error is no error.
    E_MVD_Error_NoError 		        = 0x00000001,
    /// MVD error is decoding error (MVD can't decode any video frame in the 2 sec interval).
    E_MVD_Error_DecodePicture 	        = 0x00000002,
    /// MVD error is stream buffer underrun.
    E_MVD_Error_Underrun		        = 0x00000004,
    /// MVD error is no available stream (MVD can't decode any video frame after set playback 3 sec).
    E_MVD_Error_No_Available_Stream		= 0x00000008,
    /// MVD error is MVD engine hang (MVD need to be reset).
    E_MVD_Error_Engine_Hang		        = 0x00000010,
} __attribute__((packed)) MVD_Error, *pMVD_Error;

/// Define MPEG video aspect ratio.
typedef enum tag_MVD_Aspect_Ratio
{
    /// MPEG video aspect ratio is unknow.
    E_MVD_AspectRatio_UnKnown   = 0x0,
    /// MPEG video aspect ratio is 1x1.
    E_MVD_AspectRatio_1X1       = 0x1,
    /// MPEG video aspect ratio is 4x3.
    E_MVD_AspectRatio_4X3       = 0x2,
    /// MPEG video aspect ratio is 16x9.
    E_MVD_AspectRatio_16X9      = 0x3,
    /// MPEG video aspect ratio is 221x100.
    E_MVD_AspectRatio_221X100   = 0x4
} __attribute__((packed)) MVD_Aspect_Ratio, *pMVD_Aspect_Ratio;

/// Define MPEG video AFD (Active Format Description) information
typedef enum tag_MVD_AFD
{
    /// Unknow AFD.
    E_MVD_AFD_UNKNOWN           = 0,
    /// LetterBox 16:9, top posistion.
    E_MVD_AFD_BOX_16X9_TOP      = 2,
    /// LetterBox 14:9, top posistion.
    E_MVD_AFD_BOX_14X9_TOP      = 3,
    /// LetterBox 16:9, cnetre posistion.
    E_MVD_AFD_BOX_16X9          = 4,
    /// Full frame.
    E_MVD_AFD_FULL_FRAME        = 8,
    /// Centre 4:3
    E_MVD_AFD_CENTRE_4x3        = 9,
    /// Centre 16:9
    E_MVD_AFD_CENTRE_16X9       = 10,
    /// Centre 14:9
    E_MVD_AFD_CENTRE_14X9       = 11,
    /// 4:3 (with shoot and protect 14:9 centre).
    E_MVD_AFD_CENTRE_4X3_14X9   = 13,
    /// 16:9 (with shoot and protect 14:9 centre).
    E_MVD_AFD_CENTRE_16X9_14X9  = 14,
    /// 16:9 (with shoot and protect 4:3 centre).
    E_MVD_AFD_CENTRE_16X9_4X3   = 15
} __attribute__((packed)) MVD_AFD, *pMVD_AFD;

/// Define input stream information.
typedef struct tag_MVD_Stream_Info
{
    /// Input stream type.
    MVD_StreamType     eStreamType;
} MVD_Stream_Info, *pMVD_Stream_Info;

/// Define MPEG video picture information
typedef struct tag_MVD_PictureData
{
    /// Video frame height.
    U32 				u32Height;
    /// Video frame width.
    U32 				u32Width;
    /// Video frame rate.
    MVD_FrameRate   	eFrame_rate;
    /// Video scan type.
    MVD_ScanType 	    eScan_type;
    /// Video aspect ratio.
    MVD_Aspect_Ratio    eAspect_ratio;
} MVD_PictureData, *pMVD_PictureData;

typedef struct
{
    U32 u32BufferAddr;
    U32 u32BufferSize;
} MVD_BUFFER_INFO;

typedef struct
{
    U32 u32FrameBufAddr;
    U32 u32StreamBufAddr;
} MVD_IFRAME_ADDR;

typedef enum tag_MVD_PVR_PlayMode
{
    //E_MVD_PVR_FAST_FORWARD_I_DECODE   = 0,
    //E_MVD_PVR_FAST_FORWARD_IP_DECODE  = 1,
    //E_MVD_PVR_FAST_BACKWARD           = 2,
    //E_MVD_PVR_SLOW_MOTION             = 4,
    E_MVD_PVR_NORMAL       = 0,
    E_MVD_PVR_DECODE_I     = 1,
    E_MVD_PVR_DECODE_IP    = 2,
    E_MVD_PVR_SLOW_MOTION  = 3,
} __attribute__((packed)) MVD_PVR_PlayMode;

typedef struct _MVD_PVR_MODE_T{
    MVD_PVR_PlayMode  PlayMode;
    U8  u8FrameRateUnit;
}MVD_PVR_MODE_T, *pMVD_PVR_MODE_T;

typedef struct _MVD_PLAY_MODE_T{
    U8  u8PlayFrameRate;
    U8  u8Mode;
}MVD_PLAY_MODE_T, *pMVD_PLAY_MODE_T;


/// MVD Frame information
typedef struct
{
    U16 u16HorSize;     ///< Horizontal size
    U16 u16VerSize;     ///< Vertical size
    U16 u16FrameRate;     ///< Frame rate
    U8 u8AspectRatio;     ///< Aspect Ratio
    U8 u8Interlace;
    U8 u8AFD;
    U16 u16CropRight;
    U16 u16CropLeft;
    U16 u16CropBottom;
    U16 u16CropTop;
    U32 u32BitRate;     ///< Bit-rate
    U8  u8LowDelay;
#ifdef ENABLE_DMP
    U16 u16PTSInterval;
    U8 u8MPEG1;
    U8 u8PlayMode;
    U8 u8FrcMode;
#endif
} MVD_FRAMEINFO;

typedef struct
{
    U32 u32High;
    U32 u32Low;
} MVD_PTS;

//------------------------------------------------------------------------------
// Type and Structure Declaration
//------------------------------------------------------------------------------
typedef enum
{
    E_MVD_STREAM_INIT   = 0,
    E_MVD_STREAM_FREE,
    E_MVD_STREAM_LOCK,
    E_MVD_STREAM_USED,
}MVD_StreamState;


typedef enum
{
    E_BUFFER_INIT = 0,
    E_BUFFER_DECODE_START,
    E_BUFFER_DECODE_DONE,
    E_BUFFER_DISPLAY_START,
    E_BUFFER_DISPLAY,
    E_BUFFER_MAX
}MVD_BufferState;

typedef enum
{
    E_MVD_CLOCK_144M = 0,
    E_MVD_CLOCK_123M,
    E_MVD_CLOCK_MIU,
    E_MVD_CLOCK_XTAL,
    E_MVD_CLOCK_MAX,
}MVD_Clock_Type;

typedef struct tag_MVD_ARG
{
    U8 u8Arg0;
    U8 u8Arg1;
    U8 u8Arg2;
    U8 u8Arg3;
}MVD_ARG, *pMVD_ARG;

typedef struct tag_MVD_StreamStruc
{
    MVD_State eStreamState;
    MVD_State eStreamPrevState;
    MVD_Error eStreamError;
    MVD_StreamType eStreamType;
    U32 u32SpeedSetCount;
    U32 u32EventMask;

    //Timer information
    S32 s32StreamTimerId;
    BOOL bCreateTimer;
    BOOL bEnableTimer;
    U32 u32TimerNum;
    U32 u32DecodeCount;

    //Bit-stream buffer parameters
    U32 u32StreamBufSize;                   //Stream buffer allocate size
    U32 u32StreamBufStart;                  //Stream buffer physical start
    U32 u32StreamBufEnd;                    //Stream buffer physical end
    U32 u32StreamBufWordStart;              //128bit, 16 byte alignment
    U32 u32StreamBufWordEnd;                //128bit, 16 byte alignment

    //Frame buffer parameters
    U32 u32FrameCount;                      //Frame buffer count, 3 or 4
    U32 u32YSize;                           //Y size
    U32 u32FrameBufSize;
    U32 u32FrameBufUnitSize;                //Frame buffer alignment unit 256*128 bit (4k byte)
    U32 u32FrameBufStart;                   //Frame buffer physical address
    U32 u32FrameBufUnitStart;               //Frame buffer physical address (Unit start)

    //Bit-stream information
    MVD_Aspect_Ratio eAspectRatio;
    MVD_FrameRate eFrameRate;
    MVD_AFD eAFD;
    MVD_ScanType eProgressSeq;              //Progressive sequence =>all frame are progressive frame
    U32 u32Hsize;                           //Picture horizontal size (pixel)
    U32 u32Vsize;                           //Picture vertical szie (line)

    BOOL bIsInvalidLevel;                   //

}MVD_StreamStruc, *pMVD_StreamStruc;

typedef struct tag_MVD
{
    //Task information
    S32 s32TaskID;
    void* pTaskStack;
    U32 u32TaskStackSize;
    //Message Queue information
    U32 u32QueueStackSize;
    S32 s32MsgQueueID;

    //Stream information
    //u32 u32StreamCount;
    BOOL bAttachInt;
    pMVD_StreamStruc* pStreamInfo;
    MVD_StreamState eStmState;    //Stream resource state
    BOOL bClockOn;
}MVD, *pMVD;

typedef enum
{
    E_MVDCODEC_MPEG1 = 0,
    E_MVDCODEC_MPEG2 = 1,
    E_MVDCODEC_MPEG4_PART2 = 2,
    E_MVDCODEC_UNKNOWN = 0xff,
}MVD_Codec_Type;

#endif
