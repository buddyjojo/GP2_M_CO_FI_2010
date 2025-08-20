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

#ifndef _DRV_MAD_DVB_H_
#define _DRV_MAD_DVB_H_
#include "mdrv_mad_common.h"

/* HE-AAC: 0x51 (AAC (LC)), 0x52 (HE-AAC v1 (SBR)), 0x53 (HE-AAC v2 (PS)) */
#define IS_MADGOOD() ( MDrv_MAD_Rpt_DTVES() == 0x41 || MDrv_MAD_Rpt_DTVES() == 0x81 || MDrv_MAD_Rpt_DTVES() == 0x71 \
						|| MDrv_MAD_Rpt_DTVES() == 0x51 || MDrv_MAD_Rpt_DTVES() == 0x52 || MDrv_MAD_Rpt_DTVES() == 0x53)

/******************************
*   enum for dvb sound mode
******************************/
enum
{
    AU_DVB_MODE_STEREO, ///< 0:STEREO
    AU_DVB_MODE_LL,     ///< 1: LL
    AU_DVB_MODE_RR,     ///< 2: RR
    AU_DVB_MODE_MUTE    ///< 3:MUTE
};


/******************************
*   enum for dvb free run mode
******************************/
enum
{
    AU_DVB_FreeRunMode_AVsync,     //0
    AU_DVB_FreeRunMode_FreeRun     //1
};


/******************************
*   enum for dvb system type
******************************/
enum
{
    AU_DVB_SYS_MPEG,             //0
    AU_DVB_SYS_AC3,              //1
    AU_DVB_SYS_AC3P,             //3
    AU_DVB_SYS_AAC,              //4
    AU_DVB_SYS_MP3,              //5
    AU_DVB_SYS_WMA,              //6
    AU_DVB_SYS_CDLPCM,           //7
    AU_DVB_SYS_RA8LBR,           //8
    AU_DVB_SYS_XPCM,
    AU_DVB_SYS_NONE,        //9

};
enum
{
	MAIN_MIX_AD,
	AD_MIX_MAIN,
};


enum
{
	MIX_OFF,
	MIX_ON,
};

typedef enum ADEC_SRC_TYPE{
    ADEC_SRC_TYPE_UNKNOWN   	= 0,
	ADEC_SRC_TYPE_PCM			= 1,
	ADEC_SRC_TYPE_AC3			= 2,
	ADEC_SRC_TYPE_EAC3			= 3,
	ADEC_SRC_TYPE_MPEG			= 4,
	ADEC_SRC_TYPE_AAC			= 5,
	ADEC_SRC_TYPE_HEAAC			= 6,
	ADEC_SRC_TYPE_MP3			= 7,
	ADEC_SRC_TYPE_WMA			= 8,
	ADEC_SRC_TYPE_DTS			= 9,
	ADEC_SRC_TYPE_SIF			= 10,
	ADEC_SRC_TYPE_SIF_BTSC		= 11, // added by Allan.Liang
	ADEC_SRC_TYPE_SIF_A2		= 12,
	ADEC_SRC_TYPE_DEFAULT		= 13,
	ADEC_SRC_TYPE_CDLPCM        = 14, //The following is defined by MStar
	ADEC_SRC_TYPE_RA8LBR		= 15,
	ADEC_SRC_TYPE_XPCM			= 16,
	ADEC_SRC_TYPE_MPEG_EN        = 17, //mstar add,20090727
	ADEC_SRC_TYPE_MS10_DDT      = 18, //mstar add,20090727
       ADEC_SRC_TYPE_MS10_DDC     = 19, //mstar add,20090727
       ADEC_SRC_TYPE_WMA_PRO    =20,//mstar add,20090811
       ADEC_SRC_TYPE_CV3             =21,
       ADEC_SRC_TYPE_G729            =22,//mstar add,20090921
       ADEC_SRC_TYPE_DTSBPS         =23,
	ADEC_SRC_TYPE_NONE			= 24,
} ADEC_SRC_TYPE_T;
enum
{
    JPEG_HD_MODE,
    JPEG_SD_MODE
};
enum
{
    JPEG_DECODE_CLEAR,
    JPEG_DECODE_RESTART
};

enum
{
    ES1_PATH,
    ES2_PATH
};

enum
{
	AD_OUT_SPEAKER,
	AD_OUT_HP,
	AD_OUT_BOTH,
	AD_OUT_NONE
};

enum
{
    FILE_SIZE_1KB,
    FILE_SIZE_2KB,
    FILE_SIZE_4KB,
    FILE_SIZE_8KB,
    FILE_SIZE_16KB,
    FILE_SIZE_32KB
};

typedef enum
{
    AU_AAC_SF_UNSUPPORTED,  /* un-supported */
    AU_AAC_SF_48K,          /* 48 KHz */
    AU_AAC_SF_441K,         /* 44.1 KHz */
    AU_AAC_SF_32K,          /* 32 KHz */
    AU_AAC_SF_24K,          /* 24 KHz */
    AU_AAC_SF_2205K,        /* 22.05 KHz */
    AU_AAC_SF_16K,          /* 16 KHz */
    AU_AAC_SF_12K,          /* 12 KHz */
    AU_AAC_SF_11025K,       /* 11.025 KHz */
    AU_AAC_SF_8K            /* 8 KHz */
} AU_DVB_AAC_SF_INDEX;

typedef struct
{
    //unsigned long baseAddr;
    unsigned long InputBufferStartAddr;
    unsigned long InputBufferEndAddr;
    unsigned long OutputBufferStartAddr;
    unsigned long OutputBufferEndAddr;
} AU_DVB_MemInfo;

/*AC3, EAC3*/
typedef struct AC3_ES_INFO{
    U8 bitRate;
    U8 sampleRate;
    U8 channelNum;/* it is not fixed yet depend on mstar definition mono=0x0,Lo/Ro= 0x1,L/R/=0x2,L/R/LS/RS =0x3*/
    U8  EAC3;       /* AC3 0x0, EAC3 0x1*/
}AC3_ES_INFO_T;

/* mpeg,MP3*/
typedef struct MPEG_ES_INFO{
    U16 bitRate;
    U16 sampleRate;
    U8 layer;
    U8  channelNum; /* mono =0x0,stereo=0x1,multi-channel =0x2*/
}MPEG_ES_INFO_T;

/* HE-AAC*/
typedef struct HEAAC_ES_INFO{
    U32 sampleRate;
    U8 version;     /*AAC 0x0  HE-AACv1= 0x1,HE-AACv2=0x2 */
    U8 Transmissionformat;     /* LOAS/LATM =0x0 , ADTS=0x1*/
    U16 channelNum;
}HEAAC_ES_INFO_T;

typedef struct XPCM_INFO{
        U8    audioType;
        U8    channels;
        U32  sampleRate;
        U8    bitsPerSample;
        U32  blockSize;
        U32 samplePerBlock;
}XPCM_INFO_T;

typedef struct RA8_INFO{
        U16    mNumCodecs;
        U16    mSamples;
        U16    mSampleRate;
        U16*  Channels;
        U16*  Regions;
        U16*  cplStart;
 	 U16*  cplQbits;
	 U16*  FrameSize;
}RA8_INFO_T;

typedef struct AD_MIX{
        U16 mix_mode;
        U16 en_mix;
}AD_MIX_T;

//==============================================
//  AuCommon
//==============================================
typedef enum
{
    DSP_MEM_TYPE_PM,     // DSP PM memory
    DSP_MEM_TYPE_DM      // DSP DM memory
}AUDIO_DSP_MEMORY_TYPE;

void MDrv_MAD_DvbInit(void);
void MDrv_MAD_SetMemInfo(void);
BOOL MDrv_MAD_ReLoadCode(U8 );
void MDrv_MAD_Dvb_setADMixMode (U16 , U16 );
void MDrv_MAD_SetADOutputMode (U8);
void MDrv_MAD_SetFreeRun( U8  );
U16 MDrv_MAD_DvbRpt_DTVES(void);

U8 MDrv_MAD_Dvb_XPCM_setParam (U8 audioType, U8 channels, U32 sampleRate,
                                                    U8 bitsPerSample, U32 blockSize, U32 samplePerBlock);
U8 MDrv_MAD_Dvb2_XPCM_setParam (U8, U8, U32, U8, U32, U32);
U8 MDrv_MAD_Dvb_RA8_setParam(U16 mNumCodecs, U16 mSamples, U16 mSampleRate,
                           U16* Channels, U16* Regions, U16* cplStart, U16* cplQbits,
                           U16* FrameSize);
BOOL MDrv_MAD_LoadCode(U8);
U16 MDrv_MAD_DvbGetSoundMode(void);
U8 MDrv_MAD_SetSystem(U8);
void MDrv_MAD_Dvb_setDecCmd(U8);
void MDrv_MAD_SetPaser(U8);
U32 MDrv_MAD_ReadTimeStamp(void);
void MDrv_MAD_Set_Dolby_DRC_Mode(U8 DRC_mod);
void MDrv_MAD_Set_Dolby_Downmix_Mode(U8 dmix_mod);
void MDrv_MAD_AC3Dec_DIS(U8 ac3_dis_en);
U16 MDrv_MAD_WMA_GetSampleRate( void );
U32 MDrv_MAD_WMA_GetBitRate( void );
U32 MDrv_MAD_aac_getSampleRate( void );
 void MDrv_MAD_SetAutoVolumeControl(U8);
 void MDrv_MAD_SetSPKOutMode(U8 );
U16 MDrv_MAD_MPEG_GetLayer( void );
U32 MDrv_MAD_MPEG_GetFrameNum(void);
U16 MDrv_MAD_Mpeg_GetSampleRate( void );
U16 MDrv_MAD_MPEG_GetBitRate( void );
U16 MDrv_MAD_Mpeg_GetChannelNum(void);
U8 MDrv_MAD_MPEG_SetFileSize(U8 value);
U32 MDrv_MAD_HEAAC_GetSampleRate( void );
U16 MDrv_MAD_HEAAC_GetChannelNum(void);
U16 MDrv_MAD_HEAAC_GetTransmissionformat(void);
U8 MDrv_MAD_HEAAC_GetVersion(void);
U32 MDrv_MAD_Read_DSP_sram(U16 dsp_addr,BOOL dm);
BOOL MDrv_MAD_DVB_PcmLevelControl( U8 sysmod );
U8 MDrv_MAD_DvbGet_AC3_Acmod(void);
U8 MDrv_MAD_DvbGet_AC3_Fscod(void);
U8 MDrv_MAD_DvbGet_AC3_Bitrate(void);
U8 MDrv_MAD_DvbGet_AC3_Dialnorm(void);
U8 MDrv_MAD_DvbGet_AC3_Version(void);
U8 MDrv_MAD_DvbSetPara_IDMA(U16 DSPaddr, U8 value1, U8 value2, U8 value3);
U8 MDrv_MAD_mpeg_GetSoundMode( void );
void MDrv_MAD_mpeg_SetSoundMode(U8);
MS_U8 MDrv_MAD_GetReloadCodeAck(void);
MS_U8 MDrv_MAD_GetLoadCodeAck(void);
MS_U8 MDrv_MAD_Rpt_DTVES(void);
void MDrv_MAD_WMA_SetASFParm(WMA_ASF_PARMTYPE , MS_U32 );
MS_BOOL MDrv_MAD_Write_DSP_sram(MS_U16 dsp_addr, MS_U32 value, AUDIO_DSP_MEMORY_TYPE dsp_memory_type);
void MDrv_MAD_Dis_MIUREQ(void);
void MDrv_MAD_RSTMAD_DisEn_MIUREQ(void);
void MDrv_MAD_DvbFLockSynthesizer_En(void);
void MDrv_MAD_SetDecCmd(AU_DVB_DECCMD u8DecCmd);
void MDrv_MAD_SetEncCmd(AU_DVB_ENCCMD u8EncCmd);
void MDrv_MAD_GetEncFrameInfo(U32 *Addr, U32 *Size, Audio_U64 *EncPts );
void MDrv_MAD_GetEncBufInfo(U32 *Addr, U32 *BufSize, U32 *FrameSize, U16 *FrameTime);
void MDrv_MAD_EncDataTakeNotification(void);
extern MS_BOOL MDrv_MAD_CheckDecIdmaReady(U8 u8IdmaChk_type );
extern void    MHal_MAD_AbsWriteMaskByte(MS_U32 u32RegAddr, MS_U8 u8Mask, MS_U8 u8Val);
void MDrv_MAD_Set_Cut_Boost(U8 scale);
U8 MDrv_MAD_Get_DDP_AD_Status(void);
void MDrv_MAD_Set_DDP_AD_Mode(U8 ddp_ad_id);
long long MDrv_MAD_Get_PTS(void);
AU_DVB_DECCMD MDrv_MAD_GetDecCmd(void);
#endif   //#ifndef _DRV_MAD_DVB_H_
