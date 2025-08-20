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

#ifndef _DRV_MAD_DVB2_H_
#define _DRV_MAD_DVB2_H_

#include "mdrv_mad_common.h"
#include "mdrv_mad_dvb.h"

/******************************
*   enum for blue tooth
******************************/
enum
{
    AU_DVB2_BT_PAUSE,     //0
    AU_DVB2_BT_PLAY     //1
};

enum
{
    AU_DVB2_SYS_MPEG_AD = 0x10,
    AU_DVB2_SYS_AC3_AD,
    AU_DVB2_SYS_AC3P_AD,
    AU_DVB2_SYS_AAC_AD,
    AU_DVB2_SYS_DDE,
    AU_DVB2_SYS_CV3,
    AU_DVB2_SYS_SBC,
    AU_DVB2_SYS_MP3,
    AU_DVB2_SYS_MPEG_EN,
    AU_DVB2_SYS_DTSE,
    AU_DVB2_SYS_XPCM,
    AU_DVB2_SYS_NONE,
};
//=====================================================
// Declare audio function here.
//=====================================================
void MDrv_MAD2_SetMemInfo(void);
MS_BOOL MDrv_MAD2_ReLoadCode(U8 );
BOOL MDrv_MAD_Alg2ReloadCode(U8 decoder_type);
MS_BOOL MDrv_MAD2_SetSystem(MS_U8 u8System_type);
void MDrv_MAD_Set_Dolby_AD_DRC_Mode(U8 DRC_mod);
void MDrv_MAD_Set_Dolby_AD_Downmix_Mode(U8 dmix_mod);
void MDrv_MAD_Dvb2_Play_BT(U8);
void MDrv_MAD2_RSTMAD_DisEn_MIUREQ(void);
void MDrv_MAD2_EnableChIRQ(MS_BOOL bEnable);	
void MDrv_MAD2_SetDspIDMA(void);
void MDrv_MAD2_TriggerPIO8(void);
MS_U8 MDrv_MAD2_GetReloadCodeAck(void);
MS_U8 MDrv_MAD2_GetLoadCodeAck(void);
void MDrv_MAD2_SetMcuCmd(MS_U8 cmd);
U32 MDrv_MAD2_ReadTimeStamp(void);
void MDrv_MAD2_RebootDsp(void);
void MDrv_MAD2_DisEn_MIUREQ(void);
void MDrv_MAD2_SetDecCmd(MS_U8 u8DecCmd);
void MDrv_MAD2_SetFreeRun(MS_U8 u8FreeRun);
MS_U8 MDrv_MAD2_GetDecCmd(void);
MS_BOOL MDrv_MAD2_Write_DSP_sram(MS_U16 dsp_addr, MS_U32 value, AUDIO_DSP_MEMORY_TYPE dsp_memory_type);
MS_U32 MDrv_MAD2_Read_DSP_sram(MS_U16 dsp_addr,AUDIO_DSP_MEMORY_TYPE dsp_memory_type);
void MDrv_MAD2_Backup_pathreg(void);
void MDrv_MAD2_Restore_pathreg(void);
extern MS_U32 MDrv_MAD_GetDspMadBaseAddr(MS_U8 u8Index);
extern void MHal_MAD_AbsWriteMaskByte(MS_U32 u32Reg, MS_U8 u8Mask, MS_U8 u8Val);
extern void MDrv_MAD_SIF_ENABLE_CHANNEL(MS_BOOL bEnable);
extern MS_BOOL MDrv_MAD_CheckSeIdmaReady(U8 u8IdmaChk_type);
extern void MDrv_MAD_SIF_SetDspCodeType(MS_U8 u8Type);
#endif //_AUDVB2_H_
