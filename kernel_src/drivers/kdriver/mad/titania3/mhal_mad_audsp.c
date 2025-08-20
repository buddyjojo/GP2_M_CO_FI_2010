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

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#include <linux/kernel.h>
#include <linux/mm.h>
//#include "mst_devid.h"
//#include "mst_utility.h"
//#include "MsTypes.h"
#include "mst_utility.h"
#include "mst_devid.h"
#include "../mdrv_mad_common.h"
#include "mhal_mad_reg.h"
#include "mhal_mad_audsp.h"
#include "chip_setup.h"
 
#include "../dspcode_d/aucode_d.c"
#include "../dspcode_d/aucode_dec.c"
#include "../dspcode_d/aucode_none.c"
#include "../dspcode_d/aucode_mpeg.c"
#include "../dspcode_d/aucode_ac3bp.c"
#include "../dspcode_d/aucode_ac3.c"
#include "../dspcode_d/aucode_mp3.c"
#include "../dspcode_d/aucode_wma.c"
#include "../dspcode_d/aucode_aacp.c"
#include "../dspcode_d/aucode_mpeg_en.c"
 
#include "../dspcode_d/aucode_cdlpcm.c"
#include "../dspcode_d/aucode_ac3p.c"
#include "../dspcode_d/aucode_xpcm.c"
#include "../dspcode_d/aucode_ra8lbr.c"
#include "../dspcode_d/aucode_wma_pro.c"
#include "../dspcode_d/aucode_ms10_ddt.c"
#include "../dspcode_d/aucode_dtsbps.c"
//Mstar add for Skype, 2009/09/22
#include "../dspcode_d/aucode_g729.c"

#include "../dspcode_s/aucode_s.c"
#include "../dspcode_s/aucode_none.c"
#include "../dspcode_s/aucode_dec.c"
#include "../dspcode_s/aucode_mpeg_ad.c"
#include "../dspcode_s/aucode_mp3.c"
#include "../dspcode_s/aucode_ac3_ad.c"
#include "../dspcode_s/aucode_ac3p_ad.c"
#include "../dspcode_s/aucode_aac_ad.c"
#include "../dspcode_s/aucode_dde.c"
#include "../dspcode_s/aucode_sbc.c"
#include "../dspcode_s/aucode_btsc.c"
#include "../dspcode_s/aucode_btsc_vif.c"
#include "../dspcode_s/aucode_palsum.c"
#include "../dspcode_s/aucode_palsum_vif.c"
#include "../dspcode_s/aucode_cv3.c"
#include "../dspcode_s/aucode_xpcm.c"

#include "../dspcode_adv/aucode_srs_t3.c"
#include "../dspcode_adv/aucode_tshdviq_t3.c"
#include "../dspcode_adv/aucode_vspk_t3.c"
#include "../dspcode_adv/aucode_adv_t3.c"



//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define DBG_AUDSP(msg) msg
#define DBG_AUDSP_LOAD(msg) msg
#define DBG_AUDSP_ERROR(msg)  msg

#define DSP_VERIFY_DSP_CODE     1   // 0: don't verify dsp code (for speed)
                                    // 1: verify dsp code (for debug)
#define DSP_IDMA_CHK_READY      1   // 0: don't check IDMA ready (for speed)
                                    // 1: check IDMA ready (for debug)

LOAD_CODE_INFO g_loadcodeinfo;

extern U32 DSPBinBaseAddress[2], DSPMadBaseBufferAdr[2];
extern U32 AseBaseAddress;
extern spinlock_t                      _mad_spinlock;

AUDIO_ALG_INFO audio_dvb_dec_info[]=
{
    //DEC_NONE[0]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "DEC none",
    },
    //DEC_DEC_NONE[1]
    {
        MST_DVB_DEC_PM1_ADDR, MST_DVB_DEC_PM1_SIZE, mst_dvb_dec_pm1 + 6,
        MST_DVB_DEC_PM2_ADDR, MST_DVB_DEC_PM2_SIZE, mst_dvb_dec_pm2 + 6,
        MST_DVB_DEC_PM3_ADDR, MST_DVB_DEC_PM3_SIZE, mst_dvb_dec_pm3 + 6,
        NULL, NULL, NULL,
        "DEC DEC_none",
    },
    //DEC_ENC_NONE[2]
    {
        MST_DVB_NONE_PM1_ADDR, MST_DVB_NONE_PM1_SIZE, mst_dvb_none_pm1 + 6,
        MST_DVB_NONE_PM2_ADDR, MST_DVB_NONE_PM2_SIZE, mst_dvb_none_pm2 + 6,
        MST_DVB_NONE_PM3_ADDR, MST_DVB_NONE_PM3_SIZE, mst_dvb_none_pm3 + 6,
        NULL, NULL, NULL,
        "DEC ENC_none",
    },
    //MPEG[3]
    {
        MST_DVB_MPEG_PM1_ADDR, MST_DVB_MPEG_PM1_SIZE, mst_dvb_mpeg_pm1 + 6,
        MST_DVB_MPEG_PM2_ADDR, MST_DVB_MPEG_PM2_SIZE, mst_dvb_mpeg_pm2 + 6,
        MST_DVB_MPEG_PM3_ADDR, MST_DVB_MPEG_PM3_SIZE, mst_dvb_mpeg_pm3 + 6,
        MST_DVB_MPEG_PM4_ADDR, MST_DVB_MPEG_PM4_SIZE, mst_dvb_mpeg_pm4 + 6,
        "mpeg",
    },

    //AC3[4]
    {
        MST_DVB_AC3P_PM1_ADDR, MST_DVB_AC3P_PM1_SIZE, mst_dvb_ac3p_pm1 + 6,
        MST_DVB_AC3P_PM2_ADDR, MST_DVB_AC3P_PM2_SIZE, mst_dvb_ac3p_pm2 + 6,
        MST_DVB_AC3P_PM3_ADDR, MST_DVB_AC3P_PM3_SIZE, mst_dvb_ac3p_pm3 + 6,
        MST_DVB_AC3P_PM4_ADDR, MST_DVB_AC3P_PM4_SIZE, mst_dvb_ac3p_pm4 + 6,
        "ac3p"
    },

    //AC3+[5]
    {
        MST_DVB_AC3P_PM1_ADDR, MST_DVB_AC3P_PM1_SIZE, mst_dvb_ac3p_pm1 + 6,
        MST_DVB_AC3P_PM2_ADDR, MST_DVB_AC3P_PM2_SIZE, mst_dvb_ac3p_pm2 + 6,
        MST_DVB_AC3P_PM3_ADDR, MST_DVB_AC3P_PM3_SIZE, mst_dvb_ac3p_pm3 + 6,
        MST_DVB_AC3P_PM4_ADDR, MST_DVB_AC3P_PM4_SIZE, mst_dvb_ac3p_pm4 + 6,
        "ac3p"
    },

    //AACp[6]
    {
		//Load ms10_ddt
        MST_DVB_MS10_DDT_PM1_ADDR, MST_DVB_MS10_DDT_PM1_SIZE, mst_dvb_ms10_ddt_pm1 + 6,
        MST_DVB_MS10_DDT_PM2_ADDR, MST_DVB_MS10_DDT_PM2_SIZE, mst_dvb_ms10_ddt_pm2 + 6,
        MST_DVB_MS10_DDT_PM3_ADDR, MST_DVB_MS10_DDT_PM3_SIZE, mst_dvb_ms10_ddt_pm3 + 6,
        MST_DVB_MS10_DDT_PM4_ADDR, MST_DVB_MS10_DDT_PM4_SIZE, mst_dvb_ms10_ddt_pm4 + 6,
        "aacp"
    },

    //MP3[7]
    {
        MST_DVB_MP3_PM1_ADDR, MST_DVB_MP3_PM1_SIZE, mst_dvb_mp3_pm1 + 6,
        MST_DVB_MP3_PM2_ADDR, MST_DVB_MP3_PM2_SIZE, mst_dvb_mp3_pm2 + 6,
        MST_DVB_MP3_PM3_ADDR, MST_DVB_MP3_PM3_SIZE, mst_dvb_mp3_pm3 + 6,
        MST_DVB_MP3_PM4_ADDR, MST_DVB_MP3_PM4_SIZE, mst_dvb_mp3_pm4 + 6,
        "mp3",
    },

    //WMA[8]
    {
        MST_DVB_WMA_PM1_ADDR, MST_DVB_WMA_PM1_SIZE, mst_dvb_wma_pm1 + 6,
        MST_DVB_WMA_PM2_ADDR, MST_DVB_WMA_PM2_SIZE, mst_dvb_wma_pm2 + 6,
        MST_DVB_WMA_PM3_ADDR, MST_DVB_WMA_PM3_SIZE, mst_dvb_wma_pm3 + 6,
        MST_DVB_WMA_PM4_ADDR, MST_DVB_WMA_PM4_SIZE, mst_dvb_wma_pm4 + 6,
        "wma"
    },

    //CDLPCM[9]
    {
        MST_DVB_CDLPCM_PM1_ADDR, MST_DVB_CDLPCM_PM1_SIZE, mst_dvb_cdlpcm_pm1 + 6,
        MST_DVB_CDLPCM_PM2_ADDR, MST_DVB_CDLPCM_PM2_SIZE, mst_dvb_cdlpcm_pm2 + 6,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "cdlpcm",
    },

    //RM[A]
    {
        MST_DVB_RA8LBR_PM1_ADDR, MST_DVB_RA8LBR_PM1_SIZE, mst_dvb_ra8lbr_pm1 + 6,
        MST_DVB_RA8LBR_PM2_ADDR, MST_DVB_RA8LBR_PM2_SIZE, mst_dvb_ra8lbr_pm2 + 6,
        MST_DVB_RA8LBR_PM3_ADDR, MST_DVB_RA8LBR_PM3_SIZE, mst_dvb_ra8lbr_pm3 + 6,
        NULL, NULL, NULL,
        "ra8lbr",
    },

    //XPCM[B]
    {
        MST_DVB_XPCM_PM1_ADDR, MST_DVB_XPCM_PM1_SIZE, mst_dvb_xpcm_pm1 + 6,
        MST_DVB_XPCM_PM2_ADDR, MST_DVB_XPCM_PM2_SIZE, mst_dvb_xpcm_pm2 + 6,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "xpcm",
    },
    //TONE[C]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "tone",
    },
    //DTS[D]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "dts",
    },
    //MS10_DDT[E]
    {
        MST_DVB_MS10_DDT_PM1_ADDR, MST_DVB_MS10_DDT_PM1_SIZE, mst_dvb_ms10_ddt_pm1 + 6,
        MST_DVB_MS10_DDT_PM2_ADDR, MST_DVB_MS10_DDT_PM2_SIZE, mst_dvb_ms10_ddt_pm2 + 6,
        MST_DVB_MS10_DDT_PM3_ADDR, MST_DVB_MS10_DDT_PM3_SIZE, mst_dvb_ms10_ddt_pm3 + 6,
        MST_DVB_MS10_DDT_PM4_ADDR, MST_DVB_MS10_DDT_PM4_SIZE, mst_dvb_ms10_ddt_pm4 + 6,
        "ms10_ddt",
    },
    //MS10_DDC[F]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "ms10_ddc",
    },
    //WMA PRO[10]
    {
        MST_DVB_WMA_PRO_PM1_ADDR, MST_DVB_WMA_PRO_PM1_SIZE, mst_dvb_wma_pro_pm1+6,
        MST_DVB_WMA_PRO_PM2_ADDR, MST_DVB_WMA_PRO_PM2_SIZE, mst_dvb_wma_pro_pm2+6,
        MST_DVB_WMA_PRO_PM3_ADDR, MST_DVB_WMA_PRO_PM3_SIZE, mst_dvb_wma_pro_pm3+6,
        MST_DVB_WMA_PRO_PM4_ADDR, MST_DVB_WMA_PRO_PM4_SIZE, mst_dvb_wma_pro_pm4+6,
        "wma_pro",
    },
    //Mstar add for Skype, 2009/09/22
    //G.729[11]        
    {
        MST_DVB_G729_PM1_ADDR, MST_DVB_G729_PM1_SIZE, mst_dvb_g729_pm1+6,
        MST_DVB_G729_PM2_ADDR, MST_DVB_G729_PM2_SIZE, mst_dvb_g729_pm2+6,
        MST_DVB_G729_PM3_ADDR, MST_DVB_G729_PM3_SIZE, mst_dvb_g729_pm3+6,
        NULL, NULL, NULL,
        "g729", 
    },
    //dtsbps[12]  
    {
        MST_DVB_DTSBPS_PM1_ADDR, MST_DVB_DTSBPS_PM1_SIZE, mst_dvb_dtsbps_pm1+6,
        MST_DVB_DTSBPS_PM2_ADDR, MST_DVB_DTSBPS_PM2_SIZE, mst_dvb_dtsbps_pm2+6,
        MST_DVB_DTSBPS_PM3_ADDR, MST_DVB_DTSBPS_PM3_SIZE, mst_dvb_dtsbps_pm3+6,
        MST_DVB_DTSBPS_PM4_ADDR, MST_DVB_DTSBPS_PM4_SIZE, mst_dvb_dtsbps_pm4+6,
        "dtsbps",
    },
    //Reserved3[13]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "reserved3",
    },
    //Reserved2[14]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "reserved2",
    },
    //Reserved1[15]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "reserved1",
    },

};


AUDIO_ALG_INFO audio_se_dec_info[]=
{
    // ---------- Decoder2 DSP Code ----------
    //AU_DVB_SYS_NONE
    {
        MST_CODEC_DEC_PM1_ADDR, MST_CODEC_DEC_PM1_SIZE, mst_codec_dec_pm1 + 6,
        MST_CODEC_DEC_PM2_ADDR, MST_CODEC_DEC_PM2_SIZE, mst_codec_dec_pm2 + 6,
        MST_CODEC_DEC_PM3_ADDR, MST_CODEC_DEC_PM3_SIZE, mst_codec_dec_pm3 + 6,
        MST_CODEC_DEC_PM4_ADDR, MST_CODEC_DEC_PM4_SIZE, mst_codec_dec_pm4 + 6,
        "SE dec_none",
    },

    //MPEG_AD
    {
        MST_CODEC_MPEG_AD_PM1_ADDR, MST_CODEC_MPEG_AD_PM1_SIZE, mst_codec_mpeg_ad_pm1 + 6,
        MST_CODEC_MPEG_AD_PM2_ADDR, MST_CODEC_MPEG_AD_PM2_SIZE, mst_codec_mpeg_ad_pm2 + 6,
        MST_CODEC_MPEG_AD_PM3_ADDR, MST_CODEC_MPEG_AD_PM3_SIZE, mst_codec_mpeg_ad_pm3 + 6,
        MST_CODEC_MPEG_AD_PM4_ADDR, MST_CODEC_MPEG_AD_PM4_SIZE, mst_codec_mpeg_ad_pm4 + 6,
        "mpeg_ad",
    },

    //AC3_AD
    {
        MST_CODEC_AC3_AD_PM1_ADDR, MST_CODEC_AC3_AD_PM1_SIZE, mst_codec_ac3_ad_pm1 + 6,
        MST_CODEC_AC3_AD_PM2_ADDR, MST_CODEC_AC3_AD_PM2_SIZE, mst_codec_ac3_ad_pm2 + 6,
        MST_CODEC_AC3_AD_PM3_ADDR, MST_CODEC_AC3_AD_PM3_SIZE, mst_codec_ac3_ad_pm3 + 6,
        NULL, NULL, NULL,
        "ac3_ad",
    },

    //AC3P_AD
    {
        MST_CODEC_AC3P_AD_PM1_ADDR, MST_CODEC_AC3P_AD_PM1_SIZE, mst_codec_ac3p_ad_pm1 + 6,
        MST_CODEC_AC3P_AD_PM2_ADDR, MST_CODEC_AC3P_AD_PM2_SIZE, mst_codec_ac3p_ad_pm2 + 6,
        MST_CODEC_AC3P_AD_PM3_ADDR, MST_CODEC_AC3P_AD_PM3_SIZE, mst_codec_ac3p_ad_pm3 + 6,
        MST_CODEC_AC3P_AD_PM4_ADDR, MST_CODEC_AC3P_AD_PM4_SIZE, mst_codec_ac3p_ad_pm4 + 6,
        "ac3p_ad",
    },

    //AAC_AD
    {
        MST_CODEC_AAC_AD_PM1_ADDR, MST_CODEC_AAC_AD_PM1_SIZE, mst_codec_aac_ad_pm1 + 6,
        MST_CODEC_AAC_AD_PM2_ADDR, MST_CODEC_AAC_AD_PM2_SIZE, mst_codec_aac_ad_pm2 + 6,
        MST_CODEC_AAC_AD_PM3_ADDR, MST_CODEC_AAC_AD_PM3_SIZE, mst_codec_aac_ad_pm3 + 6,
        MST_CODEC_AAC_AD_PM4_ADDR, MST_CODEC_AAC_AD_PM4_SIZE, mst_codec_aac_ad_pm4 + 6,
        "aac_ad",
    },

    //DDE
    {
        MST_CODEC_DDE_PM1_ADDR, MST_CODEC_DDE_PM1_SIZE, mst_codec_dde_pm1 + 6,
        MST_CODEC_DDE_PM2_ADDR, MST_CODEC_DDE_PM2_SIZE, mst_codec_dde_pm2 + 6,
        MST_CODEC_DDE_PM3_ADDR, MST_CODEC_DDE_PM3_SIZE, mst_codec_dde_pm3 + 6,
        MST_CODEC_DDE_PM4_ADDR, MST_CODEC_DDE_PM4_SIZE, mst_codec_dde_pm4 + 6,
        "dde",
    },

    //MP3
    {
        MST_CODEC_MP3_PM1_ADDR, MST_CODEC_MP3_PM1_SIZE, mst_codec_mp3_pm1 + 6,
        MST_CODEC_MP3_PM2_ADDR, MST_CODEC_MP3_PM2_SIZE, mst_codec_mp3_pm2 + 6,
        MST_CODEC_MP3_PM3_ADDR, MST_CODEC_MP3_PM3_SIZE, mst_codec_mp3_pm3 + 6,
        MST_CODEC_MP3_PM4_ADDR, MST_CODEC_MP3_PM4_SIZE, mst_codec_mp3_pm4 + 6,
        "mp3",
    },

    //MPEG encode
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "mpeg_en",
    },

    //DTSC
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "dtsc",
    },

    //XPCM
    {
        MST_CODEC_XPCM_PM1_ADDR, MST_CODEC_XPCM_PM1_SIZE, mst_codec_xpcm_pm1 + 6,
        MST_CODEC_XPCM_PM2_ADDR, MST_CODEC_XPCM_PM2_SIZE, mst_codec_xpcm_pm2 + 6,
        MST_CODEC_XPCM_PM3_ADDR, MST_CODEC_XPCM_PM3_SIZE, mst_codec_xpcm_pm3 + 6,
        NULL, NULL, NULL,
        "xpcm",
    },
};
;

AUDIO_ALG_INFO  audio_encoder_info[]=
{
     //mpeg_en
    {
        MST_DVB_MPEG_EN_PM1_ADDR, MST_DVB_MPEG_EN_PM1_SIZE, mst_dvb_mpeg_en_pm1 + 6,
        MST_DVB_MPEG_EN_PM2_ADDR, MST_DVB_MPEG_EN_PM2_SIZE, mst_dvb_mpeg_en_pm2 + 6,
        MST_DVB_MPEG_EN_PM3_ADDR, MST_DVB_MPEG_EN_PM3_SIZE, mst_dvb_mpeg_en_pm3 + 6,
        NULL, NULL, NULL,
        "MPEG encoder",
    },
};

AUDIO_ALG_INFO  audio_adv_sndeff_info[]    =
{
    //AU_DVB_SYS_NONE
    {
        MST_CODEC_NONE_PM1_ADDR, MST_CODEC_NONE_PM1_SIZE, mst_codec_none_pm1 + 6,
        MST_CODEC_NONE_PM2_ADDR, MST_CODEC_NONE_PM2_SIZE, mst_codec_none_pm2 + 6,
        MST_CODEC_NONE_PM3_ADDR, MST_CODEC_NONE_PM3_SIZE, mst_codec_none_pm3 + 6,
        NULL, NULL, NULL,
        "SE adv_none",
    },

#if(AUDIO_USE_SOUND_EFFECT_BBE==1)
    //BBE, 1
    {
        MST_CODEC_BBE_PM1_ADDR, MST_CODEC_BBE_PM1_SIZE, MST_CODEC_BBE_PM1_FLASH_ADDR + 6,
        MST_CODEC_BBE_PM2_ADDR, MST_CODEC_BBE_PM2_SIZE, MST_CODEC_BBE_PM2_FLASH_ADDR + 6,
        //MST_CODEC_BBE_PM3_ADDR, MST_CODEC_BBE_PM3_SIZE, MST_CODEC_BBE_PM3_FLASH_ADDR + 6,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "bbe",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

#if(AUDIO_USE_SOUND_EFFECT_SRS==1)
    //SRS, 2
    {
        MST_CODEC_SRS_PM1_ADDR, MST_CODEC_SRS_PM1_SIZE, mst_codec_srs_pm1 + 6,
        MST_CODEC_SRS_PM2_ADDR, MST_CODEC_SRS_PM2_SIZE, mst_codec_srs_pm2 + 6,
        MST_CODEC_SRS_PM3_ADDR, MST_CODEC_SRS_PM3_SIZE, mst_codec_srs_pm3 + 6,
        NULL, NULL, NULL,
        "srs",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

#if(AUDIO_USE_SOUND_EFFECT_VDS==1)
    //VDS, 3
    {
        MST_CODEC_VDS_PM1_ADDR, MST_CODEC_VDS_PM1_SIZE, MST_CODEC_VDS_PM1_FLASH_ADDR + 6,
        MST_CODEC_VDS_PM2_ADDR, MST_CODEC_VDS_PM2_SIZE, MST_CODEC_VDS_PM2_FLASH_ADDR + 6,
        MST_CODEC_VDS_PM3_ADDR, MST_CODEC_VDS_PM3_SIZE, MST_CODEC_VDS_PM3_FLASH_ADDR + 6,
        MST_CODEC_VDS_PM4_ADDR, MST_CODEC_VDS_PM4_SIZE, MST_CODEC_VDS_PM4_FLASH_ADDR + 6,
        "vds",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

#if(AUDIO_USE_SOUND_EFFECT_VSPK==1)
    //VSPK, 4
    {
        MST_CODEC_VSPK_PM1_ADDR, MST_CODEC_VSPK_PM1_SIZE, mst_codec_vspk_pm1 + 6,
        MST_CODEC_VSPK_PM2_ADDR, MST_CODEC_VSPK_PM2_SIZE, mst_codec_vspk_pm2 + 6,
        MST_CODEC_VSPK_PM3_ADDR, MST_CODEC_VSPK_PM3_SIZE, mst_codec_vspk_pm3 + 6,
        MST_CODEC_VSPK_PM4_ADDR, MST_CODEC_VSPK_PM4_SIZE, mst_codec_vspk_pm4 + 6,
        "vspk",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

#if(AUDIO_USE_SOUND_EFFECT_SUPVOICE==1)
    //SUPV, 5
    {
        MST_CODEC_SUPV_PM1_ADDR, MST_CODEC_SUPV_PM1_SIZE, MST_CODEC_SUPV_PM1_FLASH_ADDR + 6,
        MST_CODEC_SUPV_PM2_ADDR, MST_CODEC_SUPV_PM2_SIZE, MST_CODEC_SUPV_PM2_FLASH_ADDR + 6,
        MST_CODEC_SUPV_PM3_ADDR, MST_CODEC_SUPV_PM3_SIZE, MST_CODEC_SUPV_PM3_FLASH_ADDR + 6,
        NULL, NULL, NULL,
        "supv",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

#if(AUDIO_USE_SOUND_EFFECT_TSHD==1)
    //TSHD, 6
    {
        MST_CODEC_TSHD_PM1_ADDR, MST_CODEC_TSHD_PM1_SIZE, MST_CODEC_TSHD_PM1_FLASH_ADDR + 6,
        MST_CODEC_TSHD_PM2_ADDR, MST_CODEC_TSHD_PM2_SIZE, MST_CODEC_TSHD_PM2_FLASH_ADDR + 6,
        MST_CODEC_TSHD_PM3_ADDR, MST_CODEC_TSHD_PM3_SIZE, MST_CODEC_TSHD_PM3_FLASH_ADDR + 6,
        NULL, NULL, NULL,
        "tshd",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

    //XEN, 7
#if(AUDIO_USE_SOUND_EFFECT_XEN==1)
    {
        MST_CODEC_XEN_PM1_ADDR, MST_CODEC_XEN_PM1_SIZE, MST_CODEC_XEN_PM1_FLASH_ADDR + 6,
        MST_CODEC_XEN_PM2_ADDR, MST_CODEC_XEN_PM2_SIZE, MST_CODEC_XEN_PM2_FLASH_ADDR + 6,
        MST_CODEC_XEN_PM3_ADDR, MST_CODEC_XEN_PM3_SIZE, MST_CODEC_XEN_PM3_FLASH_ADDR + 6,
        MST_CODEC_XEN_PM4_ADDR, MST_CODEC_XEN_PM4_SIZE, MST_CODEC_XEN_PM4_FLASH_ADDR + 6,
        "xen",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

#if(AUDIO_USE_SOUND_EFFECT_TSHDVIQ==1)
    //TSHDVIQ, 8
    {
        MST_CODEC_TSHDVIQ_PM1_ADDR, MST_CODEC_TSHDVIQ_PM1_SIZE, mst_codec_tshdviq_pm1 + 6,
        MST_CODEC_TSHDVIQ_PM2_ADDR, MST_CODEC_TSHDVIQ_PM2_SIZE, mst_codec_tshdviq_pm2 + 6,
        MST_CODEC_TSHDVIQ_PM3_ADDR, MST_CODEC_TSHDVIQ_PM3_SIZE, mst_codec_tshdviq_pm3 + 6,
        MST_CODEC_TSHDVIQ_PM4_ADDR, MST_CODEC_TSHDVIQ_PM4_SIZE, mst_codec_tshdviq_pm4 + 6,
        "tshdviq",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

   
#if(AUDIO_USE_SOUND_EFFECT_ADV==1)
    //ADV, 9
    {
        MST_CODEC_ADV_PM1_ADDR, MST_CODEC_ADV_PM1_SIZE, mst_codec_adv_pm1 + 6,
        MST_CODEC_ADV_PM2_ADDR, MST_CODEC_ADV_PM2_SIZE, mst_codec_adv_pm2 + 6,
        MST_CODEC_ADV_PM3_ADDR, MST_CODEC_ADV_PM3_SIZE, mst_codec_adv_pm3 + 6,
        MST_CODEC_ADV_PM4_ADDR, MST_CODEC_ADV_PM4_SIZE, mst_codec_adv_pm4 + 6,
        "adv",
    },
#else
    //NULL
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "null",
    },
#endif

    //SBC, A
    {
        MST_CODEC_SBC_PM1_ADDR, MST_CODEC_SBC_PM1_SIZE, mst_codec_sbc_pm1 + 6,
        MST_CODEC_SBC_PM2_ADDR, MST_CODEC_SBC_PM2_SIZE, mst_codec_sbc_pm2 + 6,
        MST_CODEC_SBC_PM3_ADDR, MST_CODEC_SBC_PM3_SIZE, mst_codec_sbc_pm3 + 6,
        NULL, NULL, NULL,
        "sbc",
    },

    //CV3, B
    {
        MST_CODEC_CV3_PM1_ADDR, MST_CODEC_CV3_PM1_SIZE, mst_codec_cv3_pm1 + 6,
        MST_CODEC_CV3_PM2_ADDR, MST_CODEC_CV3_PM2_SIZE, mst_codec_cv3_pm2 + 6,
        MST_CODEC_CV3_PM3_ADDR, MST_CODEC_CV3_PM3_SIZE, mst_codec_cv3_pm3 + 6,
        MST_CODEC_CV3_PM4_ADDR, MST_CODEC_CV3_PM4_SIZE, mst_codec_cv3_pm4 + 6,
        "cv3",
    },
    
    {
        MST_CODEC_NONE_PM1_ADDR, MST_CODEC_NONE_PM1_SIZE, mst_codec_none_pm1 + 6,
        MST_CODEC_NONE_PM2_ADDR, MST_CODEC_NONE_PM2_SIZE, mst_codec_none_pm2 + 6,
        MST_CODEC_NONE_PM3_ADDR, MST_CODEC_NONE_PM3_SIZE, mst_codec_none_pm3 + 6,
        NULL, NULL, NULL,
        "SE adv_none",
    }		
    
};

AUDIO_ALG_INFO  audio_decoder_info[]=
{
    // Decoder System
    {
        MST_DVB_SYS_PM1_ADDR, MST_DVB_SYS_PM1_SIZE,  mst_dvb_sys_pm1 + 6,
        MST_DVB_SYS_PM2_ADDR, MST_DVB_SYS_PM2_SIZE,  mst_dvb_sys_pm2 + 6,
        MST_DVB_SYS_PM3_ADDR, MST_DVB_SYS_PM3_SIZE,  mst_dvb_sys_pm3 + 6,
        0, 0, 0,
        "T3 DEC system",
    },
};

AUDIO_ALG_INFO  audio_soundeffect_info[]=
{
    // SE System
    {
        MST_CODEC_PM1_ADDR, MST_CODEC_PM1_SIZE, mst_codec_pm1 + 6,
        MST_CODEC_PM2_ADDR, MST_CODEC_PM2_SIZE, mst_codec_pm2 + 6,
        MST_CODEC_PM3_ADDR, MST_CODEC_PM3_SIZE, mst_codec_pm3 + 6,
        MST_CODEC_PM4_ADDR, MST_CODEC_PM4_SIZE, mst_codec_pm4 + 6,
        "T3 SE system"
    },
};

AUDIO_ALG_INFO audio_sndeff_info[]=
{
    //none
    {
        MST_CODEC_NONE_PM1_ADDR, MST_CODEC_NONE_PM1_SIZE, mst_codec_none_pm1 + 6,
        MST_CODEC_NONE_PM2_ADDR, MST_CODEC_NONE_PM2_SIZE, mst_codec_none_pm2 + 6,
        MST_CODEC_NONE_PM3_ADDR, MST_CODEC_NONE_PM3_SIZE, mst_codec_none_pm3 + 6,
        NULL, NULL, NULL,
        "SE adv_none",
    },
};
AUDIO_ALG_INFO audio_sif_dec_info[]=
{
     //NONE
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "NULL",
    },
    //BTSC[0]
    {
        MST_CODEC_SIF_BTSC_PM1_ADDR, MST_CODEC_SIF_BTSC_PM1_SIZE, mst_codec_sif_btsc_pm1 + 6,
        MST_CODEC_SIF_BTSC_PM2_ADDR, MST_CODEC_SIF_BTSC_PM2_SIZE, mst_codec_sif_btsc_pm2 + 6,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "BTSC",
    },
    //EIAJ[1]
    {
        //MST_SIF_EIAJ_PM1_ADDR, MST_SIF_EIAJ_PM1_SIZE, MST_SIF_EIAJ_PM1_FLASH_ADDR,
        //MST_SIF_EIAJ_PM2_ADDR, MST_SIF_EIAJ_PM2_SIZE, MST_SIF_EIAJ_PM2_FLASH_ADDR,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "EIAJ",
    },
    //palsum[2]
    {
        MST_CODEC_SIF_PALSUM_PM1_ADDR, MST_CODEC_SIF_PALSUM_PM1_SIZE, mst_codec_sif_palsum_pm1 + 6,
        MST_CODEC_SIF_PALSUM_PM2_ADDR, MST_CODEC_SIF_PALSUM_PM2_SIZE, mst_codec_sif_palsum_pm2 + 6,
        MST_CODEC_SIF_PALSUM_PM3_ADDR, MST_CODEC_SIF_PALSUM_PM3_SIZE, mst_codec_sif_palsum_pm3 + 6,
        NULL, NULL, NULL,
        "PAL-SUM",
    },
    //FM_RADIO[3]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "FM-RADIO",
    },
    //NONE
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "NULL",
    },
};

AUDIO_ALG_INFO audio_sif_vif_dec_info[]=
{
    //BTSC[0]
    {
        MST_CODEC_SIF_BTSC_VIF_PM1_ADDR, MST_CODEC_SIF_BTSC_VIF_PM1_SIZE, mst_codec_sif_btsc_vif_pm1 + 6,
        MST_CODEC_SIF_BTSC_VIF_PM2_ADDR, MST_CODEC_SIF_BTSC_VIF_PM2_SIZE, mst_codec_sif_btsc_vif_pm2 + 6,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "BTSC",
    },
    //EIAJ[1]
    {
        //MST_SIF_EIAJ_PM1_ADDR, MST_SIF_EIAJ_PM1_SIZE, MST_SIF_EIAJ_PM1_FLASH_ADDR,
        //MST_SIF_EIAJ_PM2_ADDR, MST_SIF_EIAJ_PM2_SIZE, MST_SIF_EIAJ_PM2_FLASH_ADDR,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "EIAJ",
    },
    //palsum[2]
    {
        MST_CODEC_SIF_PALSUM_VIF_PM1_ADDR, MST_CODEC_SIF_PALSUM_VIF_PM1_SIZE, mst_codec_sif_palsum_vif_pm1 + 6,
        MST_CODEC_SIF_PALSUM_VIF_PM2_ADDR, MST_CODEC_SIF_PALSUM_VIF_PM2_SIZE, mst_codec_sif_palsum_vif_pm2 + 6,
        MST_CODEC_SIF_PALSUM_VIF_PM3_ADDR, MST_CODEC_SIF_PALSUM_VIF_PM3_SIZE, mst_codec_sif_palsum_vif_pm3 + 6,        
        NULL, NULL, NULL,
        "PAL-SUM",
    },
    //FM_RADIO[3]
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "FM-RADIO",
    },
    //NONE
    {
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        "NULL",
    },
};

#ifdef MSOS_TYPE_LINUX
void* MDrv_MPool_PA2KSEG1(void* pAddrPhys);
#endif
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define AUDIO_HAL_ERR(x, args...)        //{printk(x, ##args);}
#define LOU8(U16Val)  ( (U8)(U16Val) )
#define HIU8(U16Val)  ( (U8)((U16Val) >> 8) )

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
U8 g_u8DspCodeTypeLoaded;
extern MS_BOOL g_bAudio_loadcode_from_dram;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_AUDIO_DspLoadCodeKernel()
/// @brief \b Function \b Description:  load CM/PM/cache/prefetch DSP code
/// @param <IN>        \b \b u8Type    :      -- DSP load code type
///                                        DSP_segment -- select DSP load code seg
///                                     DSP_select -- select DSP1 or DSP2
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_DspLoadCode(U8 u8Type)
{
    U32 MIU_addr;
    U32 idma_wrbase_addr_h, idma_wrbase_addr_l;
    AUDIO_ALG_INFO *pau_info=NULL;
    MS_U8   DSP_select=0;
    
    MS_U32            cm_addr_tmp;
    MS_U32            cm_len_tmp=0;
    MS_U8            *pcm_buf_tmp=NULL;

   #if 0
   while((MDrv_FLASH_CheckWriteDone() == FALSE)&&(timeout++ < 1000))
            msleep(1);
        if (timeout == 1000) {
            printk (" Flash Busy wait timeout\n");
            return 0;
        }
   #endif

   DBG_AUDSP_LOAD(printk("    ====== MDrv_MAD_DspLoadCode: 0x%x ======\r\n",u8Type));

   switch(u8Type & 0xF0)
   {
       case 0x00:
   	case 0x10://AU_DVB_STANDARD_MAD_TYPE
	    if(u8Type >= AU_DVB_STANDARD_MAX )
	    {
		DBG_AUDSP_ERROR(printk("    [audio_dvb_dec_info]:decoder type out of range.\r\n"));
		return FALSE;
	    }
	    pau_info = &audio_dvb_dec_info[u8Type&0x1F];
	    DSP_select = DSP_DEC;
	    break;

   	case 0x20:
	    pau_info = &audio_encoder_info[0];
	    DSP_select = DSP_DEC;
	    break;

	case 0x30://AU_DVB2_STANDARD_MAD_TYPE
	   if(u8Type >= AU_DVB2_STANDARD_MAX )
	    {
		DBG_AUDSP_ERROR(printk("    [audio_se_dec_info]:decoder type out of range.\r\n"));
		return FALSE;
	    }
	    pau_info  = &audio_se_dec_info[u8Type&0xF];
	    DSP_select = DSP_SE;
	    break;
	case 0x40://AU_DVB2_ADVSND_TYPE
	    if(u8Type >= AU_DVB2_ADVSND_MAX )
	    {
		DBG_AUDSP_ERROR(printk("    [audio_adv_sndeff_info]:decoder type out of range.\r\n"));
		return FALSE;
	    }
	    pau_info = &audio_adv_sndeff_info[u8Type&0xF];
	    DSP_select= DSP_SE;
	    break;
	case 0x50://AU_DEC_SYSTEM
	    pau_info = &audio_decoder_info[0];
	    DSP_select = DSP_DEC;
	    break;
	case 0x60://AU_SE_SYSTEM
	    pau_info = &audio_soundeffect_info[0];
	    DSP_select = DSP_SE;
	    break;
	case 0xa0://AU_SND_EFFECT
	    pau_info = &audio_sndeff_info[0];
	    DSP_select = DSP_SE;
	    break;
	case 0xb0://AU_DVB_STANDARD_SIF_TYPE
	   if ((u8Type & (~0x08)) > AU_SIF_FM_RADIO )
	    {
		DBG_AUDSP_ERROR(printk("    [audio_sif_info]:decoder type out of range.\r\n"));
		return FALSE;
	    }
        if (u8Type & 0x08)
            pau_info = &audio_sif_vif_dec_info[u8Type&0x07];
        else
            pau_info = &audio_sif_dec_info[u8Type&0x7];
	    DSP_select = DSP_SE;
	    break;

	default:

	    DBG_AUDSP_ERROR(printk("    [MDrv_AUDSP_DspLoadCode]:======  Loading the wrong DSP code type!======\r\n"));
	return FALSE;

    }

    if(pau_info->cm_len == 0)
    {
        DBG_AUDSP_ERROR(printk("MDrv_MAD_DspLoadCode fail !!!\n"));
        return TRUE;                 // Return if CM length = 0 (BDMA can't support 0 length)
    }
    
    MDrv_MAD_SetDspLoadCodeInfo(pau_info, DSP_select);
    if(DSP_select == DSP_DEC)
    {
        idma_wrbase_addr_l = REG_DEC_IDMA_WRBASE_ADDR_L;
	    idma_wrbase_addr_h = REG_DEC_IDMA_WRBASE_ADDR_H;
    }
    else
    {
        idma_wrbase_addr_l = REG_SE_IDMA_WRBASE_ADDR_L;
	    idma_wrbase_addr_h = REG_SE_IDMA_WRBASE_ADDR_H;
    }
    if (u8Type == 0x50 || u8Type == 0x60)//Dec/Se system
    {
#if 0
        pau_info->cm_addr = 0x0008;
        pau_info->cm_buf = pau_info->cm_buf + 24;
	    pau_info->cm_len = pau_info->cm_len - 24;
#else
        cm_addr_tmp = 0x0008;
        pcm_buf_tmp = pau_info->cm_buf + 24;
	 cm_len_tmp = (pau_info->cm_len) - 24;
#endif
    }

     // Download PM of Algorithm
    DBG_AUDSP_LOAD(printk("    PM addr: 0x%X\r\n",(U32)pau_info->pm_addr));
    DBG_AUDSP_LOAD(printk("    PM buf addr: 0x%x\r\n",(U32)pau_info->pm_buf));
    DBG_AUDSP_LOAD(printk("    PM size: 0x%X\r\n",(U32)pau_info->pm_len));

    if(!MDrv_MAD_DspLoadCodeSegment(pau_info->pm_addr, pau_info->pm_buf, pau_info->pm_len, DSP_select)) return FALSE;
    if(!MDrv_MAD_DspVerifySegmentCode(pau_info->pm_addr, pau_info->pm_buf, pau_info->pm_len, DSP_select)) return FALSE;

    if (u8Type == 0x50 || u8Type == 0x60)//Dec/Se system
    {
        DBG_AUDSP_LOAD(printk("    CM addr: 0x%X\r\n",(U32)cm_addr_tmp));
        DBG_AUDSP_LOAD(printk("    CM buf addr: 0x%x\r\n",(U32)pcm_buf_tmp));
        DBG_AUDSP_LOAD(printk("    CM size: 0x%X\r\n",(U32)cm_len_tmp));

        if(!MDrv_MAD_DspLoadCodeSegment(cm_addr_tmp, pcm_buf_tmp, cm_len_tmp, DSP_select))  return FALSE;
        if(!MDrv_MAD_DspVerifySegmentCode(cm_addr_tmp, pcm_buf_tmp, cm_len_tmp, DSP_select)) return FALSE;
    }
    else
    {
        DBG_AUDSP_LOAD(printk("    CM addr: 0x%X\r\n",(U32)pau_info->cm_addr));
        DBG_AUDSP_LOAD(printk("    CM buf addr: 0x%x\r\n",(U32)pau_info->cm_buf));
        DBG_AUDSP_LOAD(printk("    CM size: 0x%X\r\n",(U32)pau_info->cm_len));

        if(!MDrv_MAD_DspLoadCodeSegment(pau_info->cm_addr, pau_info->cm_buf, pau_info->cm_len, DSP_select))  return FALSE;
        if(!MDrv_MAD_DspVerifySegmentCode(pau_info->cm_addr, pau_info->cm_buf, pau_info->cm_len, DSP_select)) return FALSE;
    }
    // Download PM of PreFetch
    if(pau_info->prefetch_len != 0)
    {
        DBG_AUDSP_LOAD(printk("    PreFetch PM addr: 0x%lX\r\n", pau_info->prefetch_addr));
        DBG_AUDSP_LOAD(printk("    PreFetch PM buf addr: 0x%x\r\n", (U32)pau_info->prefetch_buf));
        DBG_AUDSP_LOAD(printk("    PreFetch PM size: 0x%x\r\n", (U32)pau_info->prefetch_len));

        MIU_addr = (U32)pau_info->prefetch_addr * 3 + MDrv_MAD_GetDspMadBaseAddr(DSP_select);
        DBG_AUDSP_LOAD(printk("    MIU of PreFetch: 0x%X\r\n", MIU_addr));
         memcpy((void*)(MIU_addr), (void*)((U32)(pau_info->prefetch_buf)), pau_info->prefetch_len);//CHECK MS_PA2KSEG1
    }

    // Download PM of Cache
    if(pau_info->cache_len != 0)
    {
        DBG_AUDSP_LOAD(printk("    Cache PM addr: 0x%lX\r\n", pau_info->cache_addr));
        DBG_AUDSP_LOAD(printk("    Cache PM buf addr: 0x%x\r\n", (U32)pau_info->cache_buf));
        DBG_AUDSP_LOAD(printk("    Cache PM size: 0x%X\r\n", (U32)pau_info->cache_len));

        MIU_addr = (U32)pau_info->cache_addr * 3 + MDrv_MAD_GetDspMadBaseAddr(DSP_select);
        DBG_AUDSP_LOAD(printk("    MIU of Cache: 0x%X\r\n", MIU_addr));
        memcpy((void*)(MIU_addr), (void*)((U32)(pau_info->cache_buf)), pau_info->cache_len); //CHECK
    } 

    if(u8Type == 0x50)
    {
        if(!MDrv_MAD_DspLoadCodeSegment(0x0001,  mst_dvb_sys_pm1+6+3, 21, DSP_select)) return FALSE;
	 if(!MDrv_MAD_DspVerifySegmentCode(0x0001,  mst_dvb_sys_pm1+6+3, 21, DSP_select)) return FALSE;
        if(!MDrv_MAD_DspLoadCodeSegment(0x0000,  mst_dvb_sys_pm1+6, 3, DSP_select)) return FALSE;
	 if(!MDrv_MAD_DspVerifySegmentCode(0x0000,  mst_dvb_sys_pm1+6, 3, DSP_select)) return FALSE;
    }
    else if(u8Type == 0x60)
    {
        if(!MDrv_MAD_DspLoadCodeSegment(0x0001,  mst_codec_pm1+6+3, 21, DSP_select)) return FALSE;
	 if(!MDrv_MAD_DspVerifySegmentCode(0x0001,  mst_codec_pm1+6+3, 21, DSP_select)) return FALSE;
        if(!MDrv_MAD_DspLoadCodeSegment(0x0000,  mst_codec_pm1+6, 3, DSP_select)) return FALSE;
	 if(!MDrv_MAD_DspVerifySegmentCode(0x0000,  mst_codec_pm1+6, 3, DSP_select)) return FALSE;
    }
    MDrv_MAD_SetDspCodeTypeLoaded(u8Type);
	
    Chip_Flush_Memory();	
	
    DBG_AUDSP_LOAD(printk("MDrv_AUDSP_DspLoadCode finished(type=%s(0x%x))\r\n", pau_info->AlgName, u8Type));
    return TRUE;

}
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MHal_MAD_DspLoadCodeSegment()
/// @brief \b Function \b Description: This routine is used to load DSP code
/// @param <IN>        \b dsp_addr    :
/// @param <IN>        \b dspCode_buf    :
/// @param <IN>        \b dspCode_buflen    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b  BOOL    :    TRUE --DSP Load code okay
///                                    FALSE--DSP Load code fail
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_DspLoadCodeSegment(U32 dsp_addr, U8  *dspCode_buf, U32 dspCode_buflen, U8 DSP_select)
{
    U32 i,j;
    U32 idma_wrbase_addr_l, dsp_brg_data_l, dsp_brg_data_h;

//MAD_LOCK();//Remove

    MHal_MAD_AbsWriteMaskByte(0x103C14, 0x01, 0x00);

    if(DSP_select == DSP_DEC)
    {
        idma_wrbase_addr_l = REG_DEC_IDMA_WRBASE_ADDR_L;
	 dsp_brg_data_l = REG_DEC_DSP_BRG_DATA_L;
	 dsp_brg_data_h = REG_DEC_DSP_BRG_DATA_H;
	 MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x00);
    }
    else
    {
        idma_wrbase_addr_l = REG_SE_IDMA_WRBASE_ADDR_L;
	 dsp_brg_data_l = REG_SE_DSP_BRG_DATA_L;
	 dsp_brg_data_h = REG_SE_DSP_BRG_DATA_H;
	 MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x01);
    }

    if(dspCode_buflen>0)
    {
        /* set iDMA addr */

        MHal_MAD_WriteReg(idma_wrbase_addr_l, dsp_addr);

        //msleep(10);
        for(j=0;j<10;j++);//for delay only

        for( i=0; i<dspCode_buflen; i+=3)
        {
            MHal_MAD_WriteByte(dsp_brg_data_l,*(dspCode_buf+i+1));

            MHal_MAD_WriteByte(dsp_brg_data_h,*(dspCode_buf+i+2));

 	     for(j=0;j<2;j++);//for delay only

            MHal_MAD_WriteByte(dsp_brg_data_l,*(dspCode_buf+i));
            MHal_MAD_WriteByte(dsp_brg_data_h,0x00);

	     if(DSP_select == DSP_DEC)
            {
                if (MDrv_MAD_CheckDecIdmaReady(AUD_CHK_DSP_WRITE_RDY)==FALSE)
                {
                    //MAD_UNLOCK();//Remove
                    return FALSE;
            }
            }
            else
            {
                if (MDrv_MAD_CheckSeIdmaReady(AUD_CHK_DSP_WRITE_RDY)==FALSE)
                {
                    //MAD_UNLOCK();//Remove
                    return FALSE;
            }
        }
    }
    }
    //MAD_UNLOCK();//lRemove
    return TRUE;
}

MS_BOOL MDrv_MAD_DspVerifySegmentCode(U32 dsp_addr, U8 *dspCode_buf, U32 dspCode_buflen, U8 DSP_select)
{
    U32 i,j;
    U8 dat[3];
    U32 idma_rdbase_addr_l, idma_ctrl0, idma_rddata_h_0, idma_rddata_h_1, idma_rddata_l;

    #if (DSP_VERIFY_DSP_CODE==0)
        return TRUE;                        //don't verify just return;
    #endif
//MAD_LOCK();//Remove
    MHal_MAD_AbsWriteMaskByte(0x103c14, 0x01, 0x00);
    
    if(DSP_select == DSP_DEC)
    {
        idma_rdbase_addr_l = REG_DEC_IDMA_RDBASE_ADDR_L;
	 idma_ctrl0 = REG_DEC_IDMA_CTRL0;
	 idma_rddata_h_0 = REG_DEC_IDMA_RDDATA_H_0;
	 idma_rddata_h_1 = REG_DEC_IDMA_RDDATA_H_1;
	 idma_rddata_l = REG_DEC_IDMA_RDDATA_L;
	 MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x00);
    }
    else
    {
        idma_rdbase_addr_l = REG_SE_IDMA_RDBASE_ADDR_L;
	 idma_ctrl0 = REG_SE_IDMA_CTRL0;
	 idma_rddata_h_0 = REG_SE_IDMA_RDDATA_H_0;
	 idma_rddata_h_1 = REG_SE_IDMA_RDDATA_H_1;
	 idma_rddata_l = REG_SE_IDMA_RDDATA_L;
	 MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x01);
    }

    MHal_MAD_WriteReg(idma_rdbase_addr_l, dsp_addr);
    //msleep(5);
    for(j=0;j<10;j++);//for delay only
    for(i=0; i<dspCode_buflen; i+=3)
    {
        MHal_MAD_WriteMaskByte(idma_ctrl0, 0x08, 0x08 );
	#if (DSP_IDMA_CHK_READY == 1)
	 if(DSP_select == DSP_DEC)
        {
	     if (MDrv_MAD_CheckDecIdmaReady(AUD_CHK_DSP_READ_RDY)==FALSE)
	     	{
		  //MAD_UNLOCK();//Remove	
                return FALSE;
        }
        }
        else
        {
            if (MDrv_MAD_CheckSeIdmaReady(AUD_CHK_DSP_READ_RDY)==FALSE)
            	{
            	  //MAD_UNLOCK();//Remove
                return FALSE;
        }
        }
       #endif
	 dat[1] = MHal_MAD_ReadByte(idma_rddata_h_0);
        dat[2] = MHal_MAD_ReadByte(idma_rddata_h_1);
        dat[0] = MHal_MAD_ReadByte(idma_rddata_l);

        //printk("@@%x\n", (dat[2] << 16) | (dat[1] << 8) | (dat[0]));

            if ( (dat[0]!=dspCode_buf[i]) || (dat[1]!=dspCode_buf[i+1]) ||
                (dat[2]!=dspCode_buf[i+2]))
            {

                DBG_AUDSP(printk("check data %x\n", i));
                DBG_AUDSP(printk("dat0 %X <===> ",dspCode_buf[i]));
                DBG_AUDSP(printk("%x \n", dat[0]));
                DBG_AUDSP(printk("dat1 %X <===> ",dspCode_buf[i+1]));
                DBG_AUDSP(printk("%x \n", dat[1]));
                DBG_AUDSP(printk("dat2 %x <===> ",dspCode_buf[i+2]));
                DBG_AUDSP(printk("%X \n", dat[2]));

                DBG_AUDSP_ERROR(printk("  Dsp code verify error!!\r\n"));
		  //MAD_UNLOCK();//Remove		
                return FALSE;
            }

    }
    //MAD_UNLOCK();//Remove	
    DBG_AUDSP(printk("  Dsp code verify ok!!\r\n"));
    return TRUE;
}
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_CheckDecIdmaReady()
/// @brief \b Function \b Description:  This routine is used to check if the Dec-DSP IDMA is ready or not.
/// @param <IN>        \b IdmaChk_type    :
///                                    0x10 : check write ready
///                                    0x80 : check read  ready
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b MS_BOOL    : TRUE--IDMA is ready
///                                      FALSE--IDMA not ready
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_CheckDecIdmaReady(U8 u8IdmaChk_type )
{
    U8  j = 0;

    while(j<200)
    {
        j++;
        if( (MHal_MAD_ReadByte(REG_DEC_IDMA_CTRL0)& u8IdmaChk_type) == 0 )
            return TRUE;
    }

    DBG_AUDSP_ERROR(printk("DSP DEC Idma check ready fail!(%d)\r\n",j));
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MHal_MAD_CheckSeIdmaReady()
/// @brief \b Function \b Description:  This routine is used to check if the Se-DSP IDMA is ready or not.
/// @param <IN>        \b IdmaChk_type    :
///                                    0x10 : check write ready
///                                    0x80 : check read  ready
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b MS_BOOL    : TRUE--IDMA is ready
///                                      FALSE--IDMA not ready
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_CheckSeIdmaReady(U8 u8IdmaChk_type)
{
    U8  j = 0;

    while(j<200)
    {
        j++;
        if( (MHal_MAD_ReadByte(REG_SE_IDMA_CTRL0)& u8IdmaChk_type) == 0 )
            return TRUE;
	//msleep(1);
	udelay(1);
    }

    DBG_AUDSP_ERROR(printk("DSP SE Idma check ready fail!(%d)\r\n",j));
    return FALSE;
}

MS_U32 MDrv_MAD_GetDspMadBaseAddr(MS_U8 u8Index)
{
    return DSPMadBaseBufferAdr[u8Index]+0xA0000000;

}
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SetDspCodeTypeLoaded()
/// @brief \b Function \b Description:  This function is used to set the DSP code type.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b U8: DSP code type.
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetDspCodeTypeLoaded(U8 u8Type)
{
    g_u8DspCodeTypeLoaded=u8Type;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_GetDspCodeTypeLoaded()
/// @brief \b Function \b Description:  This function is used to get the MAD base address.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b U8: DSP code type.
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
U8 MDrv_MAD_GetDspCodeTypeLoaded(void)
{
    return g_u8DspCodeTypeLoaded;
}


void MDrv_MAD_SetDspLoadCodeInfo(AUDIO_ALG_INFO *pau_info, U8 DSP_select)
{
    g_loadcodeinfo.pau_info = pau_info;
    g_loadcodeinfo.DSP_select= DSP_select;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_GetDspCodeTypeLoaded()
/// @brief \b Function \b Description:  This function is used to get the MAD base address.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b U8: DSP code type.
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
LOAD_CODE_INFO* MDrv_MAD_GetDspLoadCodeInfo(void)
{
    return &g_loadcodeinfo;
}

