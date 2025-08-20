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

#ifndef _DRV_MAD_PROCESS_H_
#define _DRV_MAD_PROCESS_H_

#include "mdrv_types.h"

#define USE_PARAM_POLLING  0
#define  SrsDmAddr  0x28

typedef struct
{
    U8 mode;
    U8 value;
}SRS_PARA;

typedef struct
{
    U8 mode;
    U32 value;
}SRS_PARA32;

typedef struct
{
    U16 dsp_addr;
    U32 value;
}PROCESS_WRITE_PARAM;

void MDrv_MAD_ProcessSetVolume(U16 , U16 );
void MDrv_MAD_ProcessAbsoluteVolume(MS_U8 u8Path, MS_U8 u8u8Vol1, MS_U8 u8u8Vol2);
void MDrv_MAD_ProcessSPDIFVolume(U16 volume);
BOOL MDrv_MAD_SeSystemLoadCode( void );
void MDrv_MAD_ProcessSetADAbsoluteVolume(U8, U8 );
void MDrv_MAD_ProcessADSetMute(BOOL );
void MDrv_MAD_ProcessSetMute(U8, BOOL);
void MDrv_MAD_ProcessSetBalance(U8, U16);
void MDrv_MAD_ProcessSetBass(U8);
void MDrv_MAD_ProcessSetTreble(U8);
void MDrv_MAD_SetSRSPara(U8 , U8 );
#if (!USE_PARAM_POLLING)
void MDrv_MAD_SetSRSPara32(U8 , U32 );
#endif
void MDrv_MAD_SRS_TruBass(U8 );
void MDrv_MAD_SRS_DC(U8 );
void MDrv_MAD_ProcessSetSRS(B16);
void MDrv_MAD_ProcessSetBBE(B16);
void MDrv_MAD_SIF_EnableDACOut(U8 , BOOL );
void MDrv_MAD_ProcessSetCH1AudioDelay(U32 delay);
void MDrv_MAD_SetSPDIFAudioDelay(U32);
void MDrv_MAD_AuProcessWritePARAMETER(U16 , U32 );
void MDrv_MAD_AuProcessWritePARAMETER_PM(U16 dsp_addr, U32 value);

#endif   //#ifndef _DRV_MAD_PROCESS_H_
