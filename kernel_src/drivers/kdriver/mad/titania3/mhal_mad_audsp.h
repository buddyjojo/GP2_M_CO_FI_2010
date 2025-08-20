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

#ifndef _DEV_AUDIO_H_
#define _DEV_AUDIO_H_
//#include "MsCommon.h"
//#include "MsTypes.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
// Define & data type
////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    MS_U32            cm_addr;
    MS_U32            cm_len;
    MS_U8            *cm_buf;

    MS_U32            pm_addr;
    MS_U32            pm_len;
    MS_U8            *pm_buf;

    MS_U32            cache_addr;
    MS_U32            cache_len;
    MS_U8            *cache_buf;

    MS_U32            prefetch_addr;
    MS_U32            prefetch_len;
    MS_U8            *prefetch_buf;

    char            *AlgName;
} AUDIO_ALG_INFO, *PAUDIO_ALG_INFO;

typedef struct
{
    AUDIO_ALG_INFO* pau_info;
    MS_U8 DSP_select;	
}LOAD_CODE_INFO;

#define AUDIO_USE_SOUND_EFFECT_PL2   0        //ID = 0
#define AUDIO_USE_SOUND_EFFECT_BBE   0        //ID = 1
#define AUDIO_USE_SOUND_EFFECT_SRS   1        //ID = 2
#define AUDIO_USE_SOUND_EFFECT_VDS   0        //ID = 3
#define AUDIO_USE_SOUND_EFFECT_VSPK  0        //ID = 4
#define AUDIO_USE_SOUND_EFFECT_SUPVOICE  0    //ID = 5
#define AUDIO_USE_SOUND_EFFECT_TSHD  0        //ID = 6
#define AUDIO_USE_SOUND_EFFECT_XEN   0        //ID = 7
#define AUDIO_USE_SOUND_EFFECT_TSHDVIQ   1    //ID = 8 
#define AUDIO_USE_SOUND_EFFECT_ADV   1        //ID = 9 
#define AUDIO_USE_SOUND_EFFECT_CV3   0        //ID = 10

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//================================================================
//  Structure
//================================================================

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

//================================================================
//  Basic Register read/write function
//================================================================
MS_U32 MDrv_MAD_GetDspMadBaseAddr(MS_U8 u8Index);
extern MS_BOOL MDrv_FLASH_CheckWriteDone(void);
extern void    MHal_MAD_AbsWriteMaskByte(MS_U32 u32RegAddr, MS_U8 u8Mask, MS_U8 u8Val);

MS_BOOL MDrv_MAD_DspLoadCodeSegment(U32 dsp_addr, U8  *dspCode_buf, U32 dspCode_buflen, MS_U8 DSP_select);
MS_BOOL MDrv_MAD_DspVerifySegmentCode(U32 dsp_addr, U8 *dspCode_buf, U32 dspCode_buflen, MS_U8 DSP_select);

MS_BOOL MDrv_MAD_CheckDecIdmaReady(U8 u8IdmaChk_type);
MS_BOOL MDrv_MAD_CheckSeIdmaReady(U8 u8IdmaChk_type);
void MDrv_MAD_SetDspCodeTypeLoaded(U8 u8Type);
U8 MDrv_MAD_GetDspCodeTypeLoaded(void);
MS_BOOL MDrv_MAD_DspLoadCode(MS_U8 u8Type);
void MDrv_MAD_SetDspLoadCodeInfo(AUDIO_ALG_INFO *pau_info, U8 DSP_select);
LOAD_CODE_INFO* MDrv_MAD_GetDspLoadCodeInfo(void);
#endif // _DEV_AUDSP_H_

