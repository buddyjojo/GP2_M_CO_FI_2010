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
///
/// @file   mdrv_h264_st.h
/// @brief  h264 Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Data structure definition
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_H264_ST_H_
#define _MDRV_H264_ST_H_

#include "mdrv_types.h"

#define MVD_PAGE_NUM            32      // 16+16 => 16(64K) for callback; 16(64K) for file playback
#define DefSizeOfPictureData   0x8000          //32KB
#define HVD_ISR_SHAREDATA_NUMB      100
#define HVD_ISR_SHAREDATA_ENTRY_SIZE      256

typedef struct _H264_PIC_HEADER_T{
    U8  u8PicType;          // picture type: 1 -> I; 2 -> P; 3 -> B
    U8  u8Top_ff;           // Top field first: 1 if top field first
    U8  u8Rpt_ff;           // Repeat first field: 1 if repeat field first
    U16 u16TmpRef;          // Temporal reference of the picture
}H264_PIC_HEADER_T, *pH264_PIC_HEADER_T;

typedef struct _H264_SEQUENCE_HEADER_T{
    U16 u16HorSize;         ///< Horizontal size
    U16 u16VerSize;         ///< Vertical size
    U16 u16FrameRate;       ///< Frame rate
    U8 u8AspectRatio;       ///< Aspect Ratio
    U8 u8Progressive;       ///< progressive or interleave
    U32 u32BitRate;         ///< Bit-rate
}H264_SEQUENCE_HEADER_T, *pH264_SEQUENCE_HEADER_T;

typedef struct _H264_SPECIFIC_STATUS_T{
    U8  bDataError;                         //detect invalid ES stream
    U8  bPictureDecodingError;              //check ES has any syntax error
    U8  bBitStreamBufferOverflow;           //buffer overflow
    U8  bBitStreamBufferUnderflow;          //buffer underflow
    U8  bGetFirstFrame;                     //Got first frame
    U8  bDisplayReady;                      //MVD is ready to display
    U8  bSequenceHeaderDetected;            //sequence header detected
    U8  bVideoSkip;                         //if AV is not sync. the video is skipped
    U8  bVideoRepeat;                       //if AV is not sync. the video is repeated
}H264_SPECIFIC_STATUS_T, *pH264_SPECIFIC_STATUS_T;

typedef struct _H264_CallBackMsg{
    pH264_PIC_HEADER_T           pPicHdrData;
    U32                         u32PicHdrSize;
    pH264_SEQUENCE_HEADER_T      pSeqHdrData;
    U32                         u32SeqHdrSize;
    pH264_SPECIFIC_STATUS_T      pSpecificStatus;
    U32                         u32SpecificStatusSize;
    U8                          *pUserData;
    U32                         u32UserDataSize;
}H264_CallBackMsg, *pH264_CallBackMsg;

typedef struct _H264_Data_Pkt{
    U32     u32NumOfDataPkt;
    U32     u32SizeOfDataPkt;
    U32     u32TypeOfStruct;
    U8      *pPktData;
}H264_Data_Pkt, *pH264_Data_Pkt;

typedef struct _H264_FIRST_PKT_T{
    U32 u32INTStatus;         ///< summary packet size
    U32 u32BufferOffset;          ///< Buffer offset
}H264_FIRST_PKT_T, *pH264_FIRST_PKT_T;

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

typedef U32(*pfnH264InterruptCB)(pH264_CallBackMsg pH264_CallBackMsgData);

//------------------------------------------------------------------------------
// Type and Structure Declaration
//------------------------------------------------------------------------------
/// Define input stream type.
typedef enum tag_H264_StreamType
{
    /// Input stream is TS stream.
    E_H264_StreamType_TS_STREAM      = 0,
    /// Input stream is TS file.
    E_H264_StreamType_TS_FILE        = 1,
    /// Input stream is PS file.
    E_H264_StreamType_PS_FILE        = 2,
    /// Input stream is PS file(ES file).
    E_H264_StreamType_PS_ES_FILE     = 3
} __attribute__((packed)) H264_StreamType, *pH264_StreamType;

/// Define MPEG video frame rate.
typedef enum tag_H264_FrameRate
{
    /// MPEG video frame rate is temporarily not available.
    E_H264_FrameRate_Invalid 	= -2,
    /// MPEG video frame rate is unknow.
    E_H264_FrameRate_Unknown	    = -1,
    /// MPEG video frame rate is 23.976 HZ.
    E_H264_FrameRate_23_976Hz    = 1,
    /// MPEG video frame rate is 24 HZ.
    E_H264_FrameRate_24Hz 	    = 2,
    /// MPEG video frame rate is 25 HZ.
    E_H264_FrameRate_25Hz 	    = 3,
    /// MPEG video frame rate is 29.97 HZ.
    E_H264_FrameRate_29_97Hz 	= 4,
    /// MPEG video frame rate is 30 HZ.
    E_H264_FrameRate_30Hz		= 5,
    /// MPEG video frame rate is 50 HZ.
    E_H264_FrameRate_50Hz 	    = 6,
    /// MPEG video frame rate is 59.94 HZ.
    E_H264_FrameRate_59_94Hz	    = 7,
    /// MPEG video frame rate is 60 HZ.
    E_H264_FrameRate_60Hz		= 8
}__attribute__((packed)) H264_FrameRate, *pH264_FrameRate;

/// Define MPEG video scan type
typedef enum tag_H264_ScanType
{
    /// MPEG video scan type id unknow.
    E_H264_ScanType_Unknown 	    = -1,
    /// MPEG video scan type is interlace.
    E_H264_ScanType_Interlaced 	= 0,
    /// MPEG video scan type is progressive.
    E_H264_ScanType_Progressive	= 1
} __attribute__((packed)) H264_ScanType, *pH264_ScanType;

/// Define MVD control state.
typedef enum tag_H264_State
{
    /// MVD control state is unknow.
    E_H264_State_Unknown 				= -1,
    /// MVD control state is initial.
    E_H264_State_Init					= 0,
    /// MVD control state is play.
    E_H264_State_Play	 				= 1,
    /// MVD control state is pause
    E_H264_State_Paused				    = 2,
    /// MVD control state is stop done.
    E_H264_State_Stop_Done			    = 8,
    /// MVD control state is decode I frame
    E_H264_State_Decode_I_Frame		    = 9,
} __attribute__((packed)) H264_State, *pH264_State;

/// Define MVD error.
typedef enum tag_H264_Error
{
    /// MVD error is no error.
    E_H264_Error_NoError 		        = 0x00000001,
    /// MVD error is decoding error (MVD can't decode any video frame in the 2 sec interval).
    E_H264_Error_DecodePicture 	        = 0x00000002,
    /// MVD error is stream buffer underrun.
    E_H264_Error_Underrun		        = 0x00000004,
    /// MVD error is no available stream (MVD can't decode any video frame after set playback 3 sec).
    E_H264_Error_No_Available_Stream		= 0x00000008,
    /// MVD error is MVD engine hang (MVD need to be reset).
    E_H264_Error_Engine_Hang		        = 0x00000010,
} __attribute__((packed)) H264_Error, *pH264_Error;

/// Define MPEG video aspect ratio.
typedef enum tag_H264_Aspect_Ratio
{
    /// MPEG video aspect ratio is unknow.
    E_H264_AspectRatio_UnKnown   = 0x0,
    /// MPEG video aspect ratio is 1x1.
    E_H264_AspectRatio_1X1       = 0x1,
    /// MPEG video aspect ratio is 4x3.
    E_H264_AspectRatio_4X3       = 0x2,
    /// MPEG video aspect ratio is 16x9.
    E_H264_AspectRatio_16X9      = 0x3,
    /// MPEG video aspect ratio is 221x100.
    E_H264_AspectRatio_221X100   = 0x4
} __attribute__((packed)) H264_Aspect_Ratio, *pH264_Aspect_Ratio;

/// Define MPEG video AFD (Active Format Description) information
typedef enum tag_H264_AFD
{
    /// Unknow AFD.
    E_H264_AFD_UNKNOWN           = 0,
    /// LetterBox 16:9, top posistion.
    E_H264_AFD_BOX_16X9_TOP      = 2,
    /// LetterBox 14:9, top posistion.
    E_H264_AFD_BOX_14X9_TOP      = 3,
    /// LetterBox 16:9, cnetre posistion.
    E_H264_AFD_BOX_16X9          = 4,
    /// Full frame.
    E_H264_AFD_FULL_FRAME        = 8,
    /// Centre 4:3
    E_H264_AFD_CENTRE_4x3        = 9,
    /// Centre 16:9
    E_H264_AFD_CENTRE_16X9       = 10,
    /// Centre 14:9
    E_H264_AFD_CENTRE_14X9       = 11,
    /// 4:3 (with shoot and protect 14:9 centre).
    E_H264_AFD_CENTRE_4X3_14X9   = 13,
    /// 16:9 (with shoot and protect 14:9 centre).
    E_H264_AFD_CENTRE_16X9_14X9  = 14,
    /// 16:9 (with shoot and protect 4:3 centre).
    E_H264_AFD_CENTRE_16X9_4X3   = 15
} __attribute__((packed)) H264_AFD, *pH264_AFD;

/// Define input stream information.
typedef struct tag_H264_Stream_Info
{
    /// Input stream type.
    H264_StreamType     eStreamType;
} H264_Stream_Info, *pH264_Stream_Info;


typedef struct
{
    U32 u32BufferAddr;
    U32 u32BufferSize;
} H264_BUFFER_INFO;

typedef struct
{
    U32 u32FrameBufAddr;
    U32 u32StreamBufAddr;
} H264_IFRAME_ADDR;

typedef enum tag_H264_PVR_PlayMode
{
    //E_MVD_PVR_FAST_FORWARD_I_DECODE   = 0,
    //E_MVD_PVR_FAST_FORWARD_IP_DECODE  = 1,
    //E_MVD_PVR_FAST_BACKWARD           = 2,
    //E_MVD_PVR_SLOW_MOTION             = 4,
    E_H264_PVR_NORMAL     = 0,
    E_H264_PVR_DECODE_I     = 1,
    E_H264_PVR_DECODE_IP    = 2,
    E_H264_PVR_SLOW_MOTION  = 3
} __attribute__((packed)) H264_PVR_PlayMode;

typedef struct _H264_PVR_MODE_T{
    H264_PVR_PlayMode  PlayMode;
    U8  u8FrameRateUnit;
}H264_PVR_MODE_T, *pH264_PVR_MODE_T;

typedef struct _H264_PLAY_MODE_T{
    U8  u8PlayFrameRate;
    U8  u8Mode;
}H264_PLAY_MODE_T, *pH264_PLAY_MODE_T;

typedef struct
{
    U8  u8SeqInfo;      // 0x30001F38 sequence level: need to update vui info, cropping, mbs_only, chorma_idc
    U8  u8DpbSize;      // 0x30001F39 the total size of frame buffers
    S16 s16Pitch;       // 0x30001F3A

    S16 s16Width;       // 0x30001F3C
    S16 s16Height;      // 0x30001F3E

    S16 s16CropLeft;    // 0x30001F40
    S16 s16CropRight;   // 0x30001F42

    S16 s16CropTop;     // 0x30001F44
    S16 s16CropBottom;  // 0x30001F46

    U32 u32StartAddr;  // 0x30001F48 //frame starting address
    U32 u32LumaSize;    // 0x30001F4C //luma   address = start_addr + frame_size * DISP_DESC.u8FB_idx;
    U32 u32FrameSize;   // 0x30001F50 //chroma address = start_addr + frame_size * DISP_DESC.u8FB_id + luma_size;
}PIC_INFO;  // 28 bytes

typedef struct
{
    U8/*BOOL*/         bAspect_ratio_info_present_flag;                // u(1)
    U8              u8Aspect_ratio_idc;                              // u(8)
    U16             u16SarWidth;                                    // u(16)

    U16             u16SarHeight;                                   // u(16)
    U8/*BOOL*/         bOverscan_info_present_flag;                    // u(1)
    U8/*BOOL*/         bOverscan_appropriate_flag;                     // u(1)

    U8/*BOOL*/         bVideo_signal_type_present_flag;                // u(1)
    U8              u8VideoFormat;                                  // u(3)
    U8/*BOOL*/         bVideo_full_range_flag;                         // u(1)
    U8/*BOOL*/         bColour_description_present_flag;               // u(1)


    U8              u8ColourPrimaries;                              // u(8)
    U8              u8TransferCharacteristics;                      // u(8)
    U8              u8MatrixCoefficients;                           // u(8)
    U8/*BOOL*/         bChroma_location_info_present_flag;             // u(1)

    U8              u8Chroma_sample_loc_type_top_field;             // ue(v) 0~5
    U8              u8Chroma_sample_loc_type_bottom_field;          // ue(v) 0~5
    U8/*BOOL*/         bTiming_info_present_flag;                      // u(1)
    U8/*BOOL*/         bFixed_frame_rate_flag;                         // u(1)




    U32             u32Num_units_in_tick;                           // u(32)
    U32             u32Time_scale;                                  // u(32)
}VUI_DISP_INFO; // 28 bytes


typedef struct
{
    U16 u16left;
    U16 u16right;
    U16 u16top;
    U16 u16bottom;

}H264_Rect;

typedef struct
{
    H264_Rect   stRect[3];               // rect for bar data, compatible w/ SEI pan scan
    U8          u8Valid_rect;            // valid rect 1:top field, 2: bottom field, 4:frame
    U8          u8Active_format;         // 0b1100 is used for SEI_PAN_SCAN
    U8          u8Resv2[6];
}AFD_Info;  // 32 bytes





// H264 Frame information
typedef struct
{
    U16 u16HorSize;     ///< Horizontal size
    U16 u16VerSize;     ///< Vertical size
    U16 u16FrameRate;   ///< Frame rate
    U8 u8AspectRatio;   ///< Aspect Ratio
    U8 u8Interlace;
    U8 u8AFD;
    U16 u16CropRight;
    U16 u16CropLeft;
    U16 u16CropBottom;
    U16 u16CropTop;
    U16 u16SarWidth;
    U16 u16SarHeight;
    U16 u16Pitch;
} AVCH264_FRAMEINFO; // 23 bytes


typedef struct
{
    U32 u32High;
    U32 u32Low;
} H264_PTS;

typedef enum
{
    ///H264
    E_H264_CODEC_TYPE_H264 = 0,
    ///AVS
    E_H264_CODEC_TYPE_AVS,
} H264_Codec_Type;

typedef enum
{
    ///H264
    E_H264_INPUT_TSP = 0,
    ///AVS
    E_H264_INPUT_FILE,
} H264_Input_Path;

typedef struct
{
    H264_Codec_Type eCodecType;
    H264_Input_Path eInputPath;
} H264_INIT_PARAM;

typedef struct
{
    U8 bInit;
    U8 bIdle;
    U8 bAlive;
} H264_Decoder_Status;


//------------------------------------------------------------------------------
// Type and Structure Declaration
//------------------------------------------------------------------------------
typedef enum
{
    E_H264_STREAM_INIT   = 0,
    E_H264_STREAM_FREE,
    E_H264_STREAM_LOCK,
    E_H264_STREAM_USED,
}H264_StreamState;


typedef enum
{
    E_H264_BUFFER_INIT = 0,
    E_H264_BUFFER_DECODE_START,
    E_H264_BUFFER_DECODE_DONE,
    E_H264_BUFFER_DISPLAY_START,
    E_H264_BUFFER_DISPLAY,
    E_H264_BUFFER_MAX
}H264_BufferState;

typedef enum
{
    E_H264_CLOCK_144M = 0,
    E_H264_CLOCK_123M,
    E_H264_CLOCK_MIU,
    E_H264_CLOCK_XTAL,
    E_H264_CLOCK_MAX,
}H264_Clock_Type;

typedef struct tag_H264_ARG
{
    U8 u8Arg0;
    U8 u8Arg1;
    U8 u8Arg2;
    U8 u8Arg3;
}H264_ARG, *pH264_ARG;

typedef struct tag_H264_StreamStruc
{
    H264_State eStreamState;
    H264_State eStreamPrevState;
    H264_Error eStreamError;
    H264_StreamType eStreamType;
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
    H264_Aspect_Ratio eAspectRatio;
    H264_FrameRate eFrameRate;
    H264_AFD eAFD;
    H264_ScanType eProgressSeq;              //Progressive sequence =>all frame are progressive frame
    U32 u32Hsize;                           //Picture horizontal size (pixel)
    U32 u32Vsize;                           //Picture vertical szie (line)

    BOOL bIsInvalidLevel;                   //

}H264_StreamStruc, *pH264_StreamStruc;

typedef struct tag_H264
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
    pH264_StreamStruc* pStreamInfo;
    H264_StreamState eStmState;    //Stream resource state
    BOOL bClockOn;
}H264, *pH264;

#endif
