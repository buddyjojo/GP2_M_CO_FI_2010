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

//*******************************************************************************
//  [Doxygen]
///@file drv_mad.h
///@brief Audio Common Subroutine
///@author MStarSemi Inc.
///
/// This module contains code for Audio's common procedure and subroutine.
///
//*******************************************************************************
#ifndef _AUCOMMON_H_
#define _AUCOMMON_H_

#ifdef _AUCOMMON_C_
#define _AUCOMMON_DECLAIM_
#else
#define _AUCOMMON_DECLAIM_ extern
#endif

#include <linux/delay.h>
#include <linux/spinlock.h>
#include "mst_devid.h"
#include "mdrv_types.h"
#include "Board.h"

#define Utopia_mad
#define code
#define BYTE U8
#undef NULL
#define NULL 0
typedef unsigned int WORD;
typedef unsigned long DWORD;
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#ifndef MAD_MEM_START
#define MAD_MEM_MAD_SIZE    0x00280000      //2.5M
#define MAD_MEM_SE_SIZE     0x00100000
#define MAD_MEM_DEC_SIZE    (MAD_MEM_MAD_SIZE -MAD_MEM_SE_SIZE)
#define MAD_MEM_START stAudioBufInfo.u32Addr
#define MAD_MEM_SIZE  stAudioBufInfo.u32Size
#endif

#define MAD_LOCK()                          spin_lock_irq(&_mad_spinlock)
#define MAD_UNLOCK()                        spin_unlock_irq(&_mad_spinlock)


#ifdef MDrv_Timer_Delay
#define timer_delayms(ms)  MDrv_Timer_Delayms(ms)
#else
#define     timer_delayms(ms) msleep(ms)
#endif

#define H2BYTE(value)             ((BYTE)((value) / 0x10000))
#define HIBYTE(value)               ((U8)((value) / 0x100))
#define LOBYTE(value)               ((U8)(value))

//MS_U32<->MS_U16
#define LOU16(U32Val)  ( (U16)(U32Val) )
#define HIU16(U32Val)  ( (U16)((U32Val) >> 16) )
//MS_U16<->MS_U8
#define LOU8(U16Val)  ( (U8)(U16Val) )
#define HIU8(U16Val)  ( (U8)((U16Val) >> 8) )

#define MAD_Delayms     msleep //msleep
#define MAD_Delayus     udelay //msleepUs

#define HINIBBLE(value)           ((value) / 0x10)
#define LONIBBLE(value)           ((value) & 0x0f)

#define DSP_IDMA_CHK_READY      1
#define AUD_CHK_DSP_READ_RDY    0x08
#define AUD_CHK_DSP_WRITE_RDY   0x10

#define DSP_CLK  204//201//198//192//185


#define _AV_SOUND_MOD_LEFT_RIGHT                0x0000
#define _AV_SOUND_MOD_LEFT_LEFT                 0x0040
#define _AV_SOUND_MOD_RIGHT_RIGHT               0x0080
#define _AV_SOUND_MOD_MIX                           0x00C0

// For MAD event trigger to AP
#define SET_MAD_EVENT(flag, bit)        ((flag)|= (bit))
#define MAD_EVENT_CLIP                  0x01 // Clip play
#define MAD_EVENT_BT                    0x02  // Blue Tooth
#define MAD_EVENT_FILE_FORMAT                  0x04 // file format play
#define MAD2_EVENT_FILE_FORMAT  0x08
#define MAD_EVENT_SKYPE_PCM_UP  0x10 //Mstar add for Skype, 2009/09/22
#define MAD_EVENT_SKYPE_BS_UP  0x20    //Mstar add for Skype, 2009/09/29
#define MAD_EVENT_PARAM_RESTORE  0x40    //Mstar add for DSP auto recovery function, 2010/01/13
#define MAD_EVENT_POLLED                  0x8000 //Polled event

// Audio Amp Options
#define AUDIO_AMP_TYPE                  USE_ANALOG_AMP  // USE_I2S_AMP
#define USE_ANALOG_AMP                  0
#define USE_I2S_AMP                     1

#define drvMAD_STOP     0x0

typedef unsigned long long Audio_U64;

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
//======SIF System Type========
typedef enum
{
    A2_M_STANDARD = 0x00,
    A2_BG_STANDARD,
    A2_DK_STANDARD,
    A2_I_STANDARD,
    AM_STANDARD,
    NICAM_BG_STANDARD,
    NICAM_I_STANDARD,
    HIDEV_M_STANDARD,
    HIDEV_BG_STANDARD,
    HIDEV_DK_STANDARD,
    HIDEV_I_STANDARD,
    BTSC_STANDARD,
    WRITE_THRESHOLD = 0x10
}AUDIO_SIF_SYSTEM_TYPE;


//-------------------------------------------------------------------------------------
// Memory copy definition
//-------------------------------------------------------------------------------------
///Memory copy type enumerate
typedef enum MEMCOPYTYPE_t
{
//	MIU_FLASH2FLASH = 0x00,
//	MIU_FLASH2SRAM  = 0x01,
	MIU_FLASH2SDRAM = 0x02, ///<    0x02:Flash to SDRAM
//	MIU_SRAM2FLASH  = 0x10,
//	MIU_SRAM2SRAM   = 0x11,
	MIU_SRAM2SDRAM  = 0x12, ///<    0x12:SRAM to SDRAM
//	MIU_SDRAM2FLASH = 0x20,
	MIU_SDRAM2SRAM  = 0x21, ///<    0x21:SDRAM to SRAM
	MIU_SDRAM2SDRAM = 0x22, ///<    0x22:SDRAM to SDRAM
    MIU_SDRAM02SDRAM1 = 0x34, ///<    0x34:SDRAM0 to SDRAM1
    MIU_SDRAM12SDRAM0 = 0x43, ///<    0x43:SDRAM1 to SDRAM0
    MIU_SDRAM02SDRAM0 = 0x33, ///<    0x33:SDRAM0 to SDRAM0
    MIU_SDRAM12SDRAM1 = 0x44, ///<    0x44:SDRAM1 to SDRAM1
	MIU_SDRAM2SDRAM_I = 0x55, ///<    0x55:SDRAM to SDRAM Inverse BitBlt

//	MIU_SDRAM2FILE  = 0x29,	// add for test
//	MIU_FILE2SDRAM  = 0x92,	// add for test

	MIU_FLASH2VDMCU = 0x60, ///<    0x55:SDRAM to SDRAM Inverse BitBlt


} MEMCOPYTYPE;

typedef union LONG16_BYTE_t
{
    U16 u16Num;     ///< 16bits
    U8 u8Num[2];    ///< 8 bits x 2
    // u8Num[0]  MSB
    // u8Num[1]  LSB
} LONG16_BYTE;

/// union for long32 (32 bits/ 4 bytes)
typedef union LONG32_BYTE_t
{
    U32 u32Num;     ///< 32bits
    U8 u8Num[4];    ///< 8 bits x4
    // u8Num[0]  MSB
    // u8Num[1]
    // u8Num[2]
    // u8Num[3]  LSB
} LONG32_BYTE;

enum
{
    DSP_DEC,        //0
    DSP_SE          //1
};

//====== DSP code Type========
typedef enum
{
    AU_DVB_STANDARD_INVALID = 0xFF,

    AU_DVB_NONE =0x00,                              //0x00
    AU_DVB_DEC_NONE,                                //0x01
    AU_DVB_ENC_NONE,                               //0x02
    AU_DVB_STANDARD_MPEG,                   //0x03
    AU_DVB_STANDARD_AC3,                //0x04
    AU_DVB_STANDARD_AC3P,               //0x05
    AU_DVB_STANDARD_AAC,                //0x06
    AU_DVB_STANDARD_MP3,                //0x07
    AU_DVB_STANDARD_WMA,                //0x08
    AU_DVB_STANDARD_CDLPCM,             //0x09
    AU_DVB_STANDARD_RA8LBR,             //0x0A
    AU_DVB_STANDARD_XPCM,               //0x0B
    AU_DVB_STANDARD_TONE,              //0x0C
    AU_DVB_STANDARD_DTS,              //0x0D
    AU_DVB_STANDARD_MS10_DDT,   //0x0E
    AU_DVB_STANDARD_MS10_DDC,   //0x0F
    AU_DVB_STANDARD_WMA_PRO,     //0x10
    AU_DVB_STANDARD_G729,              //0x11 ////Mstar add for Skype, 2009/09/22

    AU_DVB_STANDARD_DTSBPS,              //0x12
    AU_DVB_STANDARD_RESERVE3,     //0x13
    AU_DVB_STANDARD_RESERVE2,              //0x14
    AU_DVB_STANDARD_RESERVE1,              //0x15
    AU_DVB_STANDARD_MAX,

    AU_DVB_STANDARD_MPEG_EN = 0x20,

    AU_DVB2_NONE=0x30,
    AU_DVB2_STANDARD_MPEG_AD ,
    AU_DVB2_STANDARD_AC3_AD,
    AU_DVB2_STANDARD_AC3P_AD,
    AU_DVB2_STANDARD_AAC_AD,
    AU_DVB2_STANDARD_DDE,
    AU_DVB2_STANDARD_MP3,
    AU_DVB2_STANDARD_MPEG_EN,
    AU_DVB2_STANDARD_DTSE,
    AU_DVB2_STANDARD_XPCM,
    AU_DVB2_STANDARD_MAX,

    AU_DVB2_ADVSND_NONE = 0x40,
    AU_DVB2_ADVSND_BBE,
    AU_DVB2_ADVSND_SRS,
    AU_DVB2_ADVSND_VDS,
    AU_DVB2_ADVSND_VSPK,
    AU_DVB2_ADVSND_SUPV,
    AU_DVB2_ADVSND_TSHD,
    AU_DVB2_ADVSND_XEN,
    AU_DVB2_ADVSND_TSHDVIQ,
    AU_DVB2_ADVSND_ADV,
    AU_DVB2_ADVSND_SBC,
    AU_DVB2_ADVSND_CV3,
    AU_DVB2_ADVSND_MAX,

    AU_DEC_SYSTEM=0x50,
    AU_SE_SYSTEM=0x60,

    AU_KTV_ENC=0x70,
    AU_KTV_FUNC=0x80,
    AU_KTV_SYSTEM=0x90,

    AU_SND_EFFECT=0xa0,

    AU_STANDARD_SIF_TYPE = 0xb0,
    AU_SIF_BTSC,
    AU_SIF_EIAJ,
    AU_SIF_PALSUM,
    AU_SIF_FM_RADIO,

}AUDIO_DSP_CODE_TYPE;
// AUDIO PATH TYPE

typedef enum
{
    AUDIO_PATH_0,      ///< 0: PATH 0
    AUDIO_PATH_1,      ///< 1: PATH 1
    AUDIO_PATH_2,      ///< 2: PATH 2
    AUDIO_PATH_3,      ///< 3: PATH 3
    AUDIO_PATH_4,
    AUDIO_PATH_5,
    AUDIO_PATH_6,
    AUDIO_PATH_7,   ///< 7: New add in T3
    AUDIO_PATH_MAIN=4,

    AUDIO_T3_PATH_AUOUT0=0x0,  ///< T3 volume path : AUOUT0
    AUDIO_T3_PATH_AUOUT1=0x1,  ///< T3 volume path : AUOUT1
    AUDIO_T3_PATH_AUOUT2=0x2,  ///< T3 volume path : AUOUT2
    AUDIO_T3_PATH_AUOUT3=0x3,  ///< T3 volume path : AUOUT3
    AUDIO_T3_PATH_I2S=0x4,       ///< T3 volume path : I2S
    AUDIO_T3_PATH_SPDIF=0x5,  ///< T3 volume path : SPDIF

    AUDIO_PATH_NULL=0xFF
}AUDIO_PATH_TYPE;

//====== Audio Path Mode TYPE========
enum
{
    AUDIO_DSP1_INPUT,        ///< 0: DSP Decoder1 Input
    AUDIO_DSP2_INPUT,        ///< 1: DSP Decoder2 Input
    AUDIO_ADC_INPUT,         ///< 2: ADC Input
    AUDIO_DSP3_INPUT=0x06,   ///< 6: DSP Decoder3 Input
    AUDIO_DSP4_INPUT=0x08,   ///< 8: DSP Decoder4 Input
    AUDIO_ADC2_INPUT=0x09,         ///< 2: ADC Input
};

//====== Audio input Mode TYPE========
typedef enum
{
    AUDIO_SPDIF_INPUT        = 3,    ///< 3: SPDIF INPUT
    AUDIO_I2S_INPUT          = 4,    ///< 4: I2S INPUT
    AUDIO_HDMI_INPUT         = 5,    ///< 5: HDMI INPUT
    AUDIO_SRC_INPUT          = 7,

    AUDIO_HDMI_NPCM_INPUT = 0x0A,

    AUDIO_DSP1_DVB_INPUT     = 0x00,
    AUDIO_DSP1_DVB1_INPUT    = 0x10,
    AUDIO_DSP1_SPDIF_INPUT   = 0x20,
    AUDIO_DSP1_SPDIFx1_INPUT = 0x30,
    AUDIO_DSP1_HDMI_INPUT    = 0x40,
    AUDIO_DSP1_HDMIx1_INPUT  = 0x50,
    AUDIO_DSP1_CardRD_INPUT  = 0x60,

    AUDIO_DSP2_DVB_INPUT     = 0x01,
    AUDIO_DSP2_DVB1_INPUT    = 0x11,
    AUDIO_DSP2_SPDIF_INPUT   = 0x21,
    AUDIO_DSP2_SPDIFx1_INPUT = 0x31,
    AUDIO_DSP2_HDMI_INPUT    = 0x41,
    AUDIO_DSP2_HDMIx1_INPUT  = 0x51,
    AUDIO_DSP2_CardRD_INPUT  = 0x61,
    AUDIO_DSP2_SIF_INPUT     = 0x71,

    AUDIO_DSP3_DVB_INPUT     = 0x06,
    AUDIO_DSP3_DVB1_INPUT    = 0x16,
    AUDIO_DSP3_SPDIF_INPUT   = 0x26,
    AUDIO_DSP3_SPDIFx1_INPUT = 0x36,
    AUDIO_DSP3_HDMI_INPUT    = 0x46,
    AUDIO_DSP3_HDMIx1_INPUT  = 0x56,
    AUDIO_DSP3_CardRD_INPUT  = 0x66,
    AUDIO_DSP3_SIF_INPUT     = 0x76,

    AUDIO_DSP4_DMARD_INPUT     = 0x58,    /// New add in T3
    AUDIO_DSP4_SIF_INPUT     = 0x78,    /// For T3 SIF input

    AUDIO_AUIN0_INPUT        = 0x02,   /// Line-in[0]
    AUDIO_AUIN1_INPUT        = 0x12,   /// Line-in[1]
    AUDIO_AUIN2_INPUT        = 0x22,   /// Line-in[2]
    AUDIO_AUIN3_INPUT        = 0x32,   /// Line-in[3]
    AUDIO_AUIN4_INPUT        = 0xC2,   /// Line-in[4]
    AUDIO_AUIN5_INPUT        = 0xD2,   /// Line-in[5]

    AUDIO_NULL_INPUT        = 0xFF,
} AUDIO_INPUT_TYPE;

typedef struct
{
    AUDIO_INPUT_TYPE u8Input;
    AUDIO_PATH_TYPE u8Path;
}AUDIO_INPUT_INFO;

typedef struct
{
    Audio_U64 Frame_PTS;
    U32 Frame_Addr;
    U32 Frame_Size;
}MPEG_EN_FRAME_INFO;

//====== I2S OUTPUT Mode TYPE========
typedef enum
{
    AUDIO_I2S_MCLK,
    AUDIO_I2S_WORD_WIDTH,
    AUDIO_I2S_SOURCE_CH,
    AUDIO_I2S_FORMAT
} AUDIO_I2S_MODE_TYPE;

typedef enum
{
    I2S_MCLK_64FS=0x04,
    I2S_MCLK_128FS=0x05,
    I2S_MCLK_256FS=0x06,
    I2S_MCLK_384FS=0x07,

    I2S_WORD_WIDTH_16BIT=0x4,
    I2S_WORD_WIDTH_24BIT=0x5,
    I2S_WORD_WIDTH_32BIT=0x6,

    I2S_FORMAT_STANDARD=0,
    I2S_FORMAT_LEFT_JUSTIFIED=1
} AUDIO_I2S_MODE_VALUE;

//====== Audio output Mode TYPE========
typedef enum
{
    AUDIO_AUOUT0_OUTPUT,      ///< 0: DAC0 OUTPUT
    AUDIO_AUOUT1_OUTPUT,      ///< 1: T2=>AA0 OUTPUT    T3=>HP OUTPUT
    AUDIO_AUOUT2_OUTPUT,      ///< 2: T2=>AA1 OUTPUT    T3=>DAC2 OUTPUT
    AUDIO_AUOUT3_OUTPUT,      ///< 3: T2=>NULL        T3=>AA OUTPUT
    AUDIO_SPDIF_OUTPUT,       ///< 4: SPDIF OUTPUT
    AUDIO_I2S_OUTPUT,         ///< 5: I2S OUTPUT
    AUDIO_SRC_IN_LR,
    AUDIO_NULL_OUTPUT=0xFF,  ///
} AUDIO_OUTPUT_TYPE;

typedef enum
{
    WMA_PARAMTYPE_VERSION,
    WMA_PARAMTYPE_CHANNELS,
    WMA_PARAMTYPE_SAMPLERATE,
    WMA_PARAMTYPE_BYTERATE,
    WMA_PARAMTYPE_BLOCKALIGN,
    WMA_PARAMTYPE_ENCOPT,
    WMA_PARAMTYPE_PARSINGBYAPP,
    WMA_PARAMTYPE_BITS_PER_SAMPLE,
    WMA_PARAMTYPE_CHANNELMASK,
    WMA_PARAMTYPE_DRC_PARAM_EXIST,
    WMA_PARAMTYPE_DRC_RMS_AMP_REF,
    WMA_PARAMTYPE_DRC_RMS_AMP_TARGET,
    WMA_PARAMTYPE_DRC_PEAK_AMP_REF,
    WMA_PARAMTYPE_DRC_PEAK_AMP_TARGET
}WMA_ASF_PARMTYPE;

typedef struct
{
    WMA_ASF_PARMTYPE param1;
    U32 param2;
}WMA_SET_PARAM;

typedef struct
{
    U32 u32_DEC_Addr;
    U32 u32_SE_Addr;
    U32 u32_DEC_Size;
    U32 u32_SE_Size;
}MAD_BASE_INFO;

// AUDIO OUTPUT TYPE
typedef enum ADEC_HP_SRC_TYPE
{
    ADECP_HP_SRC_BEFORE_PP=0,   //befort postprocessing
    ADECP_HP_SRC_AFTER_PP,  //after postprocessing
} ADEC_HP_SRC_TYPE_T;

typedef struct
{
    U32 u32Addr;
    U32 u32Size;
}AUDIO_BUFFER_INFO, *PAUDIO_BUFFER_INFO;

typedef struct
{
    AUDIO_PATH_TYPE     path_type;
    AUDIO_INPUT_TYPE    input_type;
    AUDIO_OUTPUT_TYPE   output_type;
}AUDIO_TYPE_INFO, *PAUDIO_TYPE_INFO;

typedef struct
{
    U16         cm_addr;
    U16         cm_len;
    U8          *cm_buf;

    U16         pm_addr;
    U16         pm_len;
    U8          *pm_buf;

    U16        cache_addr;
    U16        cache_len;
    U8        *cache_buf;

    U32        prefetch_addr;
    U16        prefetch_len;
    U8        *prefetch_buf;

    char            *codecName;
} AUDIO_DECODE_INFO, *PAUDIO_DECODE_INFO;

typedef struct
{
    MS_BOOL bDspType;
    MS_U8 u8ParamNum;
    MS_U16 value;
}ACCESS_MAILBOX, *PACCESS_MAILBOX;

typedef struct
{
    U16 addr;
    U16 value;
}ACCESS_AUDIO_REG;

typedef struct
{
    U16 addr;
    U8 value;
}ACCESS_AUDIO_REGBYTE;

typedef struct
{
    U16 addr;
    U16 mask;
    U16 value;
}ACCESS_MAD_REG;

typedef struct
{
    U16 addr;
    U8 mask;
    U8 value;
}ACCESS_MAD_REGBYTE;

typedef struct
{
    U8 path;
    U32 enable;
}PROCESS_MUTE, *PPROCESS_MUTE;

typedef struct
{
    U8 path;
#if 1//by LGE YWJung 2008.06.07
    U16 volume;
#else
    U8 volume;
#endif
}PROCESS_VOLUME, *PPROCESS_VOLUME;

typedef struct
{
    U8 integer_vol;
    U8 fraction_vol;
}PROCESS_ADVOLUME, *PPROCESS_ADVOLUME;

typedef struct
{
    U8 path;
#if 1 //By YW Jung LGE 2008/09/05
	U16 balance;
#else
    U8 balance;
#endif
}PROCESS_BALANCE, *PPROCESS_BALANCE;

typedef struct
{
    U8 path;
#if 1 //By YC Kee LGE 2008/12/21
    U16 level;
#else
	U8 level;
#endif
}PROCESS_BASS, *PPROCESS_BASS;

typedef struct
{
    U8 path;
#if 1 //By YC Kee LGE 2008/12/21
    U16 level;
#else
	U8 level;
#endif
}PROCESS_TREBLE, *PPROCESS_TREBLE;

typedef struct
{
    U8 band;
    U8 level;
}PROCESS_GEQ, *PPROCESS_GEQ;

typedef struct
{
    U8 path;
    U32 enable;
}DAC_OUT, *PDAC_OUT;


typedef enum
{
    	CH_A = 0,
    	CH_B,
    	CH_NUM
}MDRV_CHANNEL_T;

typedef struct
{
    void *pVaAddr;
    void *pPaAddr;
    U32 size;
}AUDIO_CLIP_INFO;

typedef struct
{
    U8 ch;
    void *pVaAddr[CH_NUM];
    void *pPaAddr[CH_NUM];
    U32 size;
}AUDIO_CLIP_INFO_Ex;

typedef struct
{
    void *pAudioAddr;
    U32 u32AudioLoopCounts;
    U32 u32AudioLength;
    U8  u8AudioIsInfinite;
    U32 u32AudioFileIndex;
} AUDIO_FILE_PARAMS;

typedef struct
{
    U8 repeat;
    U8 ch;
}CLIP_CH_INFO;

extern AUDIO_BUFFER_INFO  stAudioBufInfo;

typedef struct
{
    U8 spdifmode;
    U8 input_source;
}SPDIF_OUT;

typedef struct
{
    U8 C_bit_en;
    U8 L_bit_en;
}SPDIF_SCMS;

typedef struct _MEDIA_FILE_INFO
{
    U32 wAddress;
    U32 wNumber;
} MEDIA_FILE_INFO_T;


typedef struct
{
    U32 Line_Addr;
    U32 Line_Size;
    Audio_U64 Enc_PTS;
} ENC_FRAME_INFO_T;

typedef struct
{
    U32 u32_BufAddr;
    U32 u32_BufSize;
    U32 u32_FrameSize;
    U16 u16_FrameTime;
} ENC_BUF_INFO_T;

typedef struct _MEDIA_FILE_INPUT
{
    U32 tTag;
    U32 wAddress;
} MEDIA_FILE_INPUT_T;

typedef struct _MEDIA_STREAM_INPUT
{
    U8 ch;
    U32 tTag;
} MEDIA_STREAM_INPUT_T;

typedef enum MDRV_SAMPLE_FREQ{
        MDRV_SAMPLERATE_BYPASS = 0,
        MDRV_SAMPLERATE_48K = 1,
        MDRV_SAMPLERATE_44_1K = 2,
        MDRV_SAMPLERATE_32K = 3,
        MDRV_SAMPLERATE_16K = 4
}MDRV_SAMPLERATE_T;

#if 1 //By YW Jung LGE 2008/09/03
typedef enum
{
        MDRV_MONO			= 0,
        MDRV_DUAL_CHANNEL	= 1,
        MDRV_STEREO			= 2,
        MDRV_JOINT_STEREO	= 3,
} MDRV_CHANNEL_MODE_T;

typedef enum
{
        MDRV_BLOCK_4		= 0,
        MDRV_BLOCK_8		= 1,
        MDRV_BLOCK_12		= 2,
        MDRV_BLOCK_16		= 3,
} MDRV_BLOCK_LENGTH_T;

typedef enum
{
        MDRV_SUBBANDS_4		= 0,
        MDRV_SUBBANDS_8		= 1,
} MDRV_SUBBANDS_T;

typedef enum
{
        MDRV_ALLOCATION_SNR			= 0,
        MDRV_ALLOCATION_LOUDNESS	= 1,
} MDRV_ALLOCATION_METHOD_T;

typedef struct MDRV_BLUETOOTH_COMMON{
        void  *     pBufAddr;
        U32    totalBufSize;
        U32    bufSize;
        U32    frameRate;
        MDRV_SAMPLERATE_T sampleRate;
        BOOL bSBCOnOff;
        MDRV_CHANNEL_MODE_T channelMode;
        MDRV_BLOCK_LENGTH_T blockLength;
        MDRV_SUBBANDS_T subBands;
        MDRV_ALLOCATION_METHOD_T allocationMethod;
        U8 minBitpool;
        U8 maxBitpool;
		U8 bitpool;
	 U16 reg2B80;
	 U16 reg2BCE;
	 U16 reg2C62;
	 U16 reg2C64;
	 U16 reg2BAC;
}MDRV_BT_T;
#endif

//Mstar add for Skype, 2009/09/22
typedef struct MDRV_SKYPE_COMMON{
        void  *     pPcmInSrcAddr;
        void  *     pPcmOutDestAddr;
        void  *     pBsInSrcAddr;
        void  *     pBsOutDestAddr;
        U32    pcm_bufSize;
        U32    bs_bufSize;
        U8      skype_init;
}MDRV_SKYPE_T;


typedef enum {
        MDRV_BT_NONE = 0,
        MDRV_BT_START = 1,
        MDRV_BT_STOP = 2,
        MDRV_BT_NOTIFY = 3,
        MDRV_BT_OVERFLOW = 4,
        MDRV_BT_UNDERFLOW = 5
}MDRV_BT_MESSAGE_T;

typedef enum{
  EQ5_1=1,
  EQ5_2,
  EQ5_3,
  EQ5_4,
  EQ5_5
} EQ5_T;

typedef enum{
  EQ7_1=1,
  EQ7_2,
  EQ7_3,
  EQ7_4,
  EQ7_5,
  EQ7_6,
  EQ7_7
} EQ7_T;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
#if 1 //temp definitoin for ddi
typedef enum ADEC_SPK_OUTPUT_MODE{
    ADEC_SPK_MODE_LR = 0,
    ADEC_SPK_MODE_LL,
    ADEC_SPK_MODE_RR,
    ADEC_SPK_MODE_MIX
} ADEC_SPK_OUTPUT_MODE_T;

#endif
enum
{
    MAD_HDMI_DVI = 0,
    MAD_HDMI_NO_AUDIO,
    MAD_HDMI_HDMI_PCM,
    MAD_HDMI_HDMI_AC3,
    MAD_HDMI_DEFAULT
};

typedef struct Cv3info{
   U32 param_mode;
   U32 param17;
   U32 param18;
   U32 param19;
   U32 param20;
   U32 param21;
   U32 param22;
   U32 param23;
   U32 param24;
   U32 param25;
   U32 param26;
   U32 param27;
   U32 param28;
   U32 param29;
}Cv3info_T;

typedef struct Cv3VolumeInfo{
   U32 OSDVOLUME;
} Cv3VolumeInfo_T;

typedef struct BassEnhanceInfo{
   U32 PARAM070;
   U32 PARAM071;
   U32 PARAM072;
   U32 PARAM073;
   U32 PARAM074;
   U32 PARAM075;
   U32 PARAM076;
   U32 PARAM077;
   U32 PARAM078;
   U32 PARAM079;
   U32 PARAM080;
   U32 PARAM127;
   U32 PARAM128;
   U32 PARAM129;
   U32 PARAM130;
   U32 PARAM131;
   U32 PARAM132;
   U32 PARAM133;
   U32 PARAM134;
   U32 PARAM135;
   U32 PARAM136;
   U32 PARAM285;
   U32 PARAM137;
   U32 PARAM138;
   U32 PARAM139;
   U32 PARAM140;
   U32 PARAM141;
   U32 PARAM142;
   U32 PARAM143;
   U32 PARAM144;
   U32 PARAM145;
   U32 PARAM146;
   U32 PARAM147;
   U32 PARAM148;
   U32 PARAM149;
   U32 PARAM150;
   U32 PARAM151;
   U32 PARAM152;
   U32 PARAM153;
   U32 PARAM154;
   U32 PARAM155;
   U32 PARAM156;
   U32 PARAM157;
   U32 PARAM158;
   U32 PARAM159;
   U32 PARAM160;
   U32 PARAM161;
   U32 PARAM162;
   U32 PARAM163;
   U32 PARAM164;
   U32 PARAM165;
   U32 PARAM166;
   U32 PARAM167;
   U32 PARAM168;
   U32 PARAM169;
   U32 PARAM170;
   U32 PARAM171;
   U32 PARAM172;
   U32 PARAM173;
   U32 PARAM174;
   U32 PARAM302;
   U32 PARAM303;
   U32 PARAM304;
   U32 PARAM175;
   U32 PARAM176;
   U32 PARAM177;
   U32 PARAM178;
   U32 PARAM179;
   U32 PARAM180;
   U32 PARAM181;
   U32 PARAM182;
   U32 PARAM183;
   U32 PARAM184;
   U32 PARAM286;
} BassEnhanceInfo_T;

typedef struct Cv3CVinfo{
   U32 param185;
   U32 param186;
   U32 param187;
   U32 param188;
   U32 param189;
   U32 param190;
   U32 param191;
   U32 param287;
   U32 param192;
   U32 param193;
   U32 param194;
   U32 param195;
   U32 param196;
   U32 param197;
   U32 param198;
   U32 param199;
   U32 param200;
   U32 param201;
   U32 param202;
   U32 param203;
   U32 param204;
   U32 param205;
   U32 param206;
   U32 param207;
   U32 param208;
   U32 param209;
   U32 param210;
   U32 param211;
   U32 param212;
   U32 param213;
   U32 param214;
   U32 param215;
   U32 param216;
   U32 param217;
   U32 param218;
}Cv3CVinfo_T;

typedef struct Cv3AVL2info{
   U32 param30;
   U32 param31;
   U32 param32;
   U32 param33;
   U32 param34;
   U32 param35;
   U32 param36;
   U32 param37;
   U32 param38;
   U32 param39;
   U32 param40;
   U32 param41;
   U32 param42;
   U32 param43;
   U32 param44;
   U32 param45;
   U32 param46;
   U32 param47;
   U32 param48;
   U32 param49;
   U32 param50;
   U32 param51;
   U32 param52;
   U32 param53;
   U32 param54;
   U32 param55;
   U32 param56;
   U32 param57;
   U32 param58;
   U32 param59;
   U32 param60;
   U32 param61;
   U32 param62;
   U32 param63;
   U32 param64;
   U32 param65;
   U32 param66;
   U32 param67;
   U32 param68;
   U32 param69;
} Cv3AVL2info_T;

typedef struct Cv3EQinfo{
   U32 param226;
   U32 param227;
   U32 param228;
   U32 param229;
   U32 param230;
   U32 param231;
   U32 param232;
   U32 param233;
   U32 param234;
   U32 param235;
   U32 param236;
   U32 param237;
   U32 param238;
   U32 param239;
   U32 param240;
   U32 param241;
   U32 param242;
   U32 param243;
   U32 param244;
   U32 param245;
   U32 param246;
   U32 param247;
   U32 param248;
   U32 param249;
   U32 param250;
   U32 param251;
   U32 param252;
   U32 param253;
   U32 param254;
   U32 param255;
   U32 param256;
   U32 param257;
   U32 param258;
   U32 param259;
   U32 param260;
   U32 param261;
   U32 param262;
   U32 param263;
   U32 param264;
   U32 param265;
   U32 param266;
   U32 param267;
   U32 param268;
   U32 param269;
   U32 param270;
   U32 param271;
   U32 param272;
   U32 param273;
   U32 param274;
   U32 param275;
   U32 param276;
   U32 param277;
   U32 param278;
   U32 param279;
   U32 param280;
   U32 param281;
   U32 param282;
   U32 param283;
} Cv3EQinfo_T;

typedef struct Cv3SurroundInfo{
   U32 param219;
   U32 param288;
   U32 param220;
   U32 param221;
   U32 param222;
   U32 param223;
   U32 param224;
   U32 param225;
} Cv3SurroundInfo_T;

typedef struct pvc_monitor {
   U32 GapCounter;
   U32 L_Level;
   U32 R_Level;
} pvc_monitor_T;

typedef struct _MEDIA_AUDIOSTREAM_INFO
{
    U8 ch;
    U32 wAddress[2];
    U32 wNumber[2];
} MEDIA_AUDIOSTREAM_INFO_T;

typedef enum
{
    INTERNAL_MULTI_1,    ///< 0: For Multi-channel only
    INTERNAL_MULTI_2,    ///< 1 For Multi-channel only
    INTERNAL_MULTI_3,    ///< 2: For Multi-channel only
    INTERNAL_MULTI_4,    ///< 3: For Multi-channel only
    INTERNAL_PCM,           ///< 4: Pure PCM out
    INTERNAL_PCM_DELAY,     ///< 5: PCM + Delay out
    INTERNAL_PCM_SE,      ///< 6: PCM + Delay +SE out
    INTERNAL_SCART,    ///< 7: SCART data
    INTERNAL_PATH_NULL,
    INTERNAL_MULTI_6,
    INTERNAL_SPDIF,
}AUDIO_INTERNAL_PATH_TYPE; // for T3 only

///< structure for audio output setting
typedef struct
{
    AUDIO_OUTPUT_TYPE   SpeakerOut;  ///< Audio output port for Speaker
    AUDIO_OUTPUT_TYPE   HpOut;       ///< Audio output port for HP
    AUDIO_OUTPUT_TYPE   MonitorOut;  ///< Audio output port for Monitor out
    AUDIO_OUTPUT_TYPE   ScartOut;    ///< Audio output port for Scart out
} AUDIO_OUT_INFO, *PAUDIO_OUT_INFO;

///< structure for audio DSP path setting
typedef struct
{
    AUDIO_PATH_TYPE   SpeakerOut;     ///< Audio DSP path for Speaker
    AUDIO_PATH_TYPE   HpOut;          ///< Audio DSP path for HP
    AUDIO_PATH_TYPE   MonitorOut;     ///< Audio DSP path for Monitor out
    AUDIO_PATH_TYPE   ScartOut;       ///< Audio DSP path for Scart out
    AUDIO_PATH_TYPE   SpdifOut;       ///< Audio DSP path for Spdif
} AUDIO_PATH_INFO, *PAUDIO_PATH_INFO;

///< Decoder command type
typedef enum
{
    DVB_DECCMD_STOP = 0x0,   ///< 1st DSP stop
    DVB_DECCMD_PLAY,         ///< 1st DSP play
    DVB_DECCMD_RESYNC,
    DVB_DECCMD_FREE_RUN,
    DVB_DECCMD_AVSYNC,
    DVB_DECCMD_PLAYFILE,     ///< 1st DSP play file
    DVB_DECCMD_PAUSE,        ///< 1st DSP pause
    DVB_DECCMD_PLAYFILETSP,  ///< 1st DSP stop file playing
    DVB_DECCMD_STARTBROWSE,
    DVB_ENCCMD_START,
    DVB_ENCCMD_STOP,
    // SE DSP
    DVB2_DECCMD_STOP=0x10,   ///< 2nd DSP stop
    DVB2_DECCMD_PLAY,        ///< 2nd DSP play
    DVB2_DECCMD_RESYNC,
    DVB2_DECCMD_FREE_RUN,
    DVB2_DECCMD_AVSYNC,
    DVB2_DECCMD_PLAYFILE,    ///< 2nd DSP play file
    DVB2_DECCMD_PAUSE,       ///< 2nd DSP pause
    DVB2_DECCMD_PLAYFILETSP  ///< 2nd DSP stop file playing
} En_DVB_decCmdType;
/*
typedef enum
{
    DVB_ENCCMD_STOP = 0x0,   ///< 1st DSP encode stop
    DVB_ENCCMD_START,         ///< 1st DSP encode start
} En_DVB_encCmdType;
*/
//  enum for dvb decoder command
typedef enum
{
    AU_DVB_DECCMD_STOP,      //0
    AU_DVB_DECCMD_PLAY,      //1
    AU_DVB_DECCMD_PLAYFILETSP = 2,
    AU_DVB_DECCMD_RESYNC,
    AU_DVB_DECCMD_PLAYFILE = 4,
    AU_DVB_DECCMD_STARTBROWSE = 5,
    AU_DVB_DECCMD_PAUSE    = 6,

    AU_DVB_DECCMD_AVSYNC  = 0x10,
    AU_DVB_DECCMD_FREERUN
} AU_DVB_DECCMD;

typedef enum
{
    AU_DVB_ENCCMD_STOP,      //0
    AU_DVB_ENCCMD_START,      //1
} AU_DVB_ENCCMD;

typedef enum
{
    AU_DVB_ENCDATA_LINEADDR,      //0
    AU_DVB_ENCCMD_LINESIZE,      //
} AU_DVB_ENCDATA;

// for dvb2 decoder command
typedef enum
{
    AU_DVB2_DECCMD_STOP,           //0
    AU_DVB2_DECCMD_PLAY,           //1
    AU_DVB2_DECCMD_PLAYFILETSP = 2,
    AU_DVB2_DECCMD_RESYNC,
    AU_DVB2_DECCMD_PLAYFILE = 4,
    AU_DVB2_DECCMD_PAUSE = 6
}AU_DVB2_DECCMD;

// for dvb2 free run mode
typedef enum
{
    AU_DVB2_FreeRunMode_AVsync,     //0
    AU_DVB2_FreeRunMode_FreeRun,     //1
}AU_DVB2_FreeRunMode;

//====== SPDIF OUTPUT CS Mode TYPE========
typedef enum
{
    SPDIF_CHANNEL_STATUS_FS,                 ///< Set sampling rate in CS3
    SPDIF_CHANNEL_STATUS_CATEGORY,  ///< Set Category code in CS1
    SPDIF_CHANNEL_STATUS_PCM_FORMAT,      ///< Set PCM/NonPCM mode in CS0
} SPDIF_CS_MODE_TYPE;

typedef enum
{
    SPDIF_CS_FS_32K=0xC0,
    SPDIF_CS_FS_44K=0x00,
    SPDIF_CS_FS_48K=0x40,    ///< ==>default setting

    SPDIF_CS_CATEGORY_NONE=0x00,
    SPDIF_CS_CATEGORY_DVB=0x30,
    SPDIF_CS_CATEGORY_ATSC=0x26,

    SPDIF_CS_FORMAT_PCM=0x00,
    SPDIF_CS_FORMAT_NONPCM=0x40,
}SPDIF_CS_MODE_VALUE;

typedef struct
{
    AUDIO_INTERNAL_PATH_TYPE u8Path;
    AUDIO_OUTPUT_TYPE u8Output;
}AUDIO_INTERNAL_PATH_INFO;

_AUCOMMON_DECLAIM_ U16 MDrv_MAD_DecReadREG(U16 );
_AUCOMMON_DECLAIM_ U16 MDrv_MAD_SeReadREG(U16 );
_AUCOMMON_DECLAIM_ void MDrv_MAD_DecWriteREG(U16 , U16 );
_AUCOMMON_DECLAIM_ void MDrv_MAD_SeWriteREG(U16 , U16 );
_AUCOMMON_DECLAIM_ void MDrv_MAD_RebootDSP(U8 );
_AUCOMMON_DECLAIM_ void MDrv_MAD_ADCInit(void);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_DTVInUse(void);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_DecoderLoadCode(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetPaser(U8 );
_AUCOMMON_DECLAIM_ void MDrv_MAD_BTUpload_Samprate_Set(U8,U16);
_AUCOMMON_DECLAIM_ void MDrv_MAD_GetMADBase(MAD_BASE_INFO *);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_ADEC_Alive_Check(void);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_ASND_Alive_Check(void);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_ADEC_ES_Check(void);
_AUCOMMON_DECLAIM_ MS_U16 MDrv_MAD_ReadMailBox(MS_BOOL bDspType, MS_U8 u8ParamNum);
_AUCOMMON_DECLAIM_ void MDrv_MAD_WriteMailBox(MS_BOOL bDspType, MS_U8 u8ParamNum, MS_U8 u16Data);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_HDMI_NONPCM_FLAG(void);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_SeDspLoadCodeSegment(u16 dsp_addr, u8 *dspCode_buf, u16 dspCode_buflen);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_SeDspVerifySegmentCode(U16 dsp_addr, U8 *dspCode_buf, U16 dspCode_buflen);
_AUCOMMON_DECLAIM_ void MDrv_MAD_DTV_HDMI_CFG(U8 ctrl);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Clip_Play(unsigned long);
_AUCOMMON_DECLAIM_ void MDrv_MAD_AudioInit(U16);
_AUCOMMON_DECLAIM_ void MDrv_MAD_ISRInit(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Set_Power_Off(U8 OnOff_flag);
_AUCOMMON_DECLAIM_ void MDrv_MAD_ResetMAD(void);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_GetDSPCodeType(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetDSPCodeType(U8);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_DecDSP_chkIdmaReady(U8);
_AUCOMMON_DECLAIM_ U16 MDrv_MAD_ReadREG(U16);
_AUCOMMON_DECLAIM_ void MDrv_MAD_WriteREG(U16, U16);
_AUCOMMON_DECLAIM_ void MDrv_MAD_WriteREGMask(U16 , U16 , U16 );
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_HDMI_DolbyMonitor(void);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_AlgReloadCode(U8 decoder_type );
// For CV3
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetCV3Para(WORD addr, U32 value);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetClearVoiceOnOff(U8 enable);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetAVL2OnOff(U8 enable);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetSurroundOnOff(U8 enable);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetUEQOnOff(U8 enable);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetClearVoiceIII(Cv3info_T *p_cv_info);

_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_CV3_PVC_Monitor(U32 *PVC);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetVolume(Cv3VolumeInfo_T *p_volume_info);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetBassEnhance(BassEnhanceInfo_T *p_bassenhance_info);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetUEQ(Cv3EQinfo_T *p_ueq_info);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetAVL2(Cv3AVL2info_T *p_avl2_info);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetCV(Cv3CVinfo_T *p_cv_info);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetSurround(Cv3SurroundInfo_T *p_surround_info);
_AUCOMMON_DECLAIM_ void MDrv_MAD_CV3_SetMode(U32 cvMode);

_AUCOMMON_DECLAIM_ void MDrv_MAD_SPDIF_EN(U8);
_AUCOMMON_DECLAIM_ void MDrv_MAD_PowerOn_Melody(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_PowerOn_Melody_Path(U8 );
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_GetDSP2CodeType(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetDSP2CodeType(U8);
_AUCOMMON_DECLAIM_ void MDrv_MAD_HDMIAutoMute(U8 , U8 );
_AUCOMMON_DECLAIM_ void MDrv_MAD_TriggerPIO8(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_GetFileInfo(MEDIA_FILE_INFO_T *);
_AUCOMMON_DECLAIM_ void MDrv_MAD_GetFileInfo_SE(MEDIA_FILE_INFO_T *);
_AUCOMMON_DECLAIM_ void MDrv_MAD_GetAudioStreamInfo(MEDIA_AUDIOSTREAM_INFO_T *);
_AUCOMMON_DECLAIM_ void MDrv_MAD_FileSetInput(U32, U32);
_AUCOMMON_DECLAIM_ void MDrv_MAD_FileSetInput_SE(U32, U32);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetAudioStreamInput(U8 , U32 );
_AUCOMMON_DECLAIM_ void MDrv_MAD_FileEndNotification(U32);
_AUCOMMON_DECLAIM_ void MDrv_MAD_FileEndNotification_SE(U32);
_AUCOMMON_DECLAIM_ U32 MDrv_MAD_Check_Copy_Rqt(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_AmplifierOn(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_AmplifierOff(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SPDIF_SetMute(MS_BOOL bEnMute);
_AUCOMMON_DECLAIM_ U32 MDrv_MAD_GetDSPClk(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_ProcessSetEq(MS_U8 u8Band, MS_U8 u8Level);
_AUCOMMON_DECLAIM_ void MDrv_MAD_ProcessSetEq7(MS_U8 u8Band, MS_U8 u8Level);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_GetHDMIAudioReceive(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetHPOutputType(U8 );
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_SetAudioPLLSFreq(U32 );
_AUCOMMON_DECLAIM_ U32 MDrv_MAD_LoadAudioClip(U32, void *);
_AUCOMMON_DECLAIM_ void MDrv_MAD_PlayAudioClip(U32);
_AUCOMMON_DECLAIM_ void MDrv_MAD_PauseAudioClip(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_ResumeAudioClip(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_StopAudioClip(void);

_AUCOMMON_DECLAIM_ void MDrv_MAD_PCMStartUpload(MDRV_BT_T *);
_AUCOMMON_DECLAIM_ void MDrv_MAD_PCMStopUpload(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_PCMStartDownload(MDRV_BT_T *);
_AUCOMMON_DECLAIM_ void MDrv_MAD_PCMStopDownload(void);


_AUCOMMON_DECLAIM_ BOOL mstDspLoadSIF( void );
_AUCOMMON_DECLAIM_ void ASPSetSIFMemInfo( void );
_AUCOMMON_DECLAIM_ void MApp_AuWriteSIFThresholdTbl(void);
_AUCOMMON_DECLAIM_ WORD MDrv_AuSifAccessThreshold(BYTE rw_standard_type, BYTE Threshold_type, WORD value);
_AUCOMMON_DECLAIM_ void MDrv_AuSifWritePARAMETER(WORD dsp_addr, DWORD value) ;
_AUCOMMON_DECLAIM_ DWORD MDrv_AuSifReadPARAMETER(WORD dsp_addr);
_AUCOMMON_DECLAIM_ void MHal_GPIO_Init(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SPDIF_SetMode(MS_U8 u8Spdif_mode, MS_U8 u8Input_src);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SPDIF_SetSCMS(BYTE C_bit_en, BYTE L_bit_en);
_AUCOMMON_DECLAIM_ BYTE MDrv_MAD_SPDIF_GetSCMS(void);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_HDMI_Monitor(void);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_HDMI_Monitor2(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_HDMI_SetNonpcm(U8 nonPCM_en);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Monitor_DDPlus_SPDIF_Rate( void );
_AUCOMMON_DECLAIM_ void MDrv_MAD_HDMI_AC3_PathCFG(U8 ctrl);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_SeDSP_chkIdamReady( U8 IdmaChk_type );
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetBTBufferCounter(MS_U32 u32Counter);
_AUCOMMON_DECLAIM_ void MDrv_MAD_BT_Upload_Samprate_Set(BOOL enable,U8 Samprate);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetDTV_NormalPath(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetATV_NormalPath(void);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_DecReadREGByte(U16 mailbox);
_AUCOMMON_DECLAIM_ void MDrv_MAD_DecWriteREGByte(U16 regaddr, U8 data);
_AUCOMMON_DECLAIM_ U8 MDrv_MAD_SeReadREGByte(U16 mailbox);
_AUCOMMON_DECLAIM_ void MDrv_MAD_DecWriteREGMask(U16 u16Addr, U16 u16Mask, U16 u16Value);
_AUCOMMON_DECLAIM_ void MDrv_MAD_DecWriteREGMaskByte(U16 u16Addr, U8 u8Mask, U8 u8Value);
_AUCOMMON_DECLAIM_ void MDrv_MAD_DecWriteIntMaskByte(U16 u16Addr, U8 u8Mask);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SeWriteREGByte(U16 u16Addr, U8 u8Value);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SeWriteREGMask(U16 u16Addr, U16 u16Mask, U16 u16Value);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SeWriteREGMaskByte(U16 u16Addr, U8 u8Mask, U8 u8Value);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Audio_Monitor(void);
//_AUCOMMON_DECLAIM_ U32 stimer( void );
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetDecDspReviveCount(U32 ReviveCount);
_AUCOMMON_DECLAIM_ U32 MDrv_MAD_GetDecDspReviveCount(void);
_AUCOMMON_DECLAIM_ MS_BOOL MDrv_MAD_DspLoadCode(U8 u8Type);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetMcuCmd(MS_U8 cmd);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetDspIDMA(void);
_AUCOMMON_DECLAIM_ MS_BOOL MDrv_MAD_GetIsDtvFlag(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetIsDtvFlag(MS_BOOL bIsDTV);
_AUCOMMON_DECLAIM_ MS_U8 MDrv_MAD_Dec_Status(void);
_AUCOMMON_DECLAIM_ MS_U8 MDrv_MAD_Se_Status(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Set_Fading(MS_U32 u32VolFading);
_AUCOMMON_DECLAIM_ void MDrv_MAD_BackupMailbox(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_RestoreMailbox(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_ResetDSP(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_RebootDecDSP(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Init(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SOUND_Init(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetPlayFileFlag(MS_BOOL bDspType, MS_BOOL bSet);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetOutConnectivity(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetInternalPath(AUDIO_INTERNAL_PATH_TYPE u8Path,  AUDIO_OUTPUT_TYPE u8Output);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetPowerOn(MS_BOOL bPower_on);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SwResetMAD(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetCommand(En_DVB_decCmdType enDecComamnd);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetInputPath(AUDIO_INPUT_TYPE input , AUDIO_PATH_TYPE u8Path);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetOutputInfo(AUDIO_OUT_INFO *pout_info);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetEncodeDoneFlag(MS_BOOL bSet);
_AUCOMMON_DECLAIM_ MS_BOOL MDrv_MAD_GetEncodeDoneFlag(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_RebootSndEffDSP(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_I2S_SetMode(MS_U8 u8Mode, MS_U8 u8Val);
_AUCOMMON_DECLAIM_ BYTE MDrv_MAD_SeTimer_CNT(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SendIntrupt(MS_BOOL , MS_U16 );
_AUCOMMON_DECLAIM_ void MDrv_MAD_SetPIOCmd(MS_U8);
_AUCOMMON_DECLAIM_ void MDrv_MAD2_SetPIOCmd(MS_U8);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Set_BT_SampleCounter(MS_U32 SampleNum);
_AUCOMMON_DECLAIM_ void MDrv_MAD_SPDIF_SetChannelStatus(SPDIF_CS_MODE_TYPE eType, SPDIF_CS_MODE_VALUE eValue);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Reg_Backup(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Reg_Restore(void);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Vol_Reg_Restore(void);
//Mstar add for Skype, 2009/09/22
_AUCOMMON_DECLAIM_ void MDrv_MAD_Init_Skype(MDRV_SKYPE_T *);
_AUCOMMON_DECLAIM_ void MDrv_MAD_Stop_Skype(void);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_Skype_Dn_PCM(void);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_Skype_Up_PCM(void);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_Skype_Dn_BS(void);
_AUCOMMON_DECLAIM_ BOOL MDrv_MAD_Skype_Up_BS(void);
#endif //_AUCOMMON_H_
