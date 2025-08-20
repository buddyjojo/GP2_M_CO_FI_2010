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
/// file    drv_mad.c
/// @brief  MAD Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#define _AUCOMMON_C_

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/autoconf.h>
#include <linux/mm.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#if 1 //By LGE 2008.07.03 Bug Fix
      // but after being modifided by Mstar, it will be recovered.
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#endif
#include "mst_utility.h"
#include "mst_devid.h"
#include "mdrv_mad_common.h"
#include "mdrv_mad_dvb.h"
#include "mdrv_mad_dvb2.h"
#include "mdrv_mad_process.h"
#include "mdrv_mad_sif.h"
#include "mdrv_gpio.h"

#include "mhal_mad_reg.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "chip_int.h"
#include "chip_setup.h"

#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define MAD_DEBUG_P1(msg) //msg
#define MAD_DEBUG_P2(msg) //msg
#define MAD_DEBUG_P3(msg) //msg
#define DBG_AUDIO_ERROR(msg) msg
#define _MDRV_ENTRY() printk("%s is called\r\n", __FUNCTION__);

#define ES_ADDR (DSPMadBaseBufferAdr[DSP_DEC] + 0xA0000000)
#define SE_ES_ADDR (DSPMadBaseBufferAdr[DSP_SE] + 0xA0000000)
#define MAD_PRINT(fmt, args...)         printk("[MVDMOD][%06d]     " fmt, __LINE__, ## args)
#define AUDIO_USE_SOUND_EFFECT_CV3 1
//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
extern void MDrv_Event_to_AP(U8) ;
extern spinlock_t                      _mad_spinlock;
#ifndef DEBUG_MICOM_MODULE
extern void MDrv_MICOM_Init(void);
#endif
#define ENABLE_MADMONITOR // dsp_revive
//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
static U8 _u8DspCodeType;                       // variable to remember which dsp code in sram
static U8 _u8Dsp2CodeType;                       // variable to remember which dsp code in sram
static U32 _clipWriteLastAddr = 0;

static U32 _copyRequest=0;
static U32 clipPlaying=0;
static U32 dsp_revive_flag=0;
static U32 gDecDspReviveCount=0;
static MS_U8 SIF_MailBoxArray[12];

U32 _BT_Count=0;
U32 _BT_RunningUp=0;
U32 _BT_RunningDown=0;
U32 _BT_BufferSize=0;
U32 BufferLineSize =0;
MS_BOOL g_bAudio_loadcode_from_dram = 0, g_bIsDTV;
static MS_BOOL g_bDecPlayFileFlag,g_bSePlayFileFlag, g_bEncodeDoneFlag;
static En_DVB_decCmdType    enDecoderStatus;
static En_DVB_decCmdType   enSeDecoderStatus;
static En_DVB_decCmdType   enEncoderStatus;
//static MS_BOOL audio_FW_Status = FALSE;
const MS_U32 u32PathArray[8]={0x2C64, 0x2C66, 0x2C68, 0x2C6A, 0x2C65, 0x2C67, 0x2C69, 0x2C6B};
//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------
AUDIO_OUT_INFO  AudioOutputInfo;     // Store the connectivity of audio output device
AUDIO_PATH_INFO  AudioPathInfo;       // Store the connectivity of audio DSP & output path
U8 dtv_mode_en;
U32 DSPMadBaseBufferAdr[2];

BOOL audio_recovery = 0;


U32 EncFrameIdx = 0;
U32 EncBuf_W_idx = 0;
U32 EncBuf_R_idx = 0;
MPEG_EN_FRAME_INFO MPEG_EN_BUF[5];
U8   EncBuf_Count = 0;
U8   Enc_InProcess = 0;


MEDIA_FILE_INFO_T get_file_info;
MEDIA_FILE_INFO_T get_file_info_se;
MEDIA_AUDIOSTREAM_INFO_T get_audiostream_info;
AUDIO_CLIP_INFO audioClipInfo;
AUDIO_FILE_PARAMS audioFileParams;

MDRV_BT_T BT_information;
//Mstar add for Skype, 2009/09/22
MDRV_SKYPE_T Skype_information;

MST_REG_TYPE AudioInitTbl[]=
{
   // addr, mask, value,
    {0x2CDE, 0x3C, 0x1C},       // AA0 source=DAC2 ; Switch to DAC before enable Vref.  @@VVV

  //----------------DSP PLL initl-------------------------
    {0x0C00, 0xFF, 0x0E},       //   @@VVV
    {0x0C01, 0xFF, 0x00},       //   @@VVV
    {0x0C02, 0xFF, 0x0A},       //   @@VVV
    {0x0C03, 0xFF, 0x3C},       //   @@VVV DSP CLK = 324MHz
    {0x0C06, 0xFF, 0x60},       //   @@VVV
    {0x0C07, 0xFF, 0x00},       //   @@VVV
    {0x2CA4, 0x60, 0x60},

  //----------------AUPLL control-------------------------
    {0x2C70, 0xFF, 0x32},       // ICTRL=3, RCTRL=2     @@VVV
    {0x2C71, 0xFF, 0x00},
    {0x2C72, 0xFF, 0x00},       //    @@VVV
    {0x2C73, 0xFF, 0x01},
    {0x2C74, 0xFF, 0x00},       //     @@VVV
    {0x2C75, 0xFF, 0x00},       //     @@VVV
    {0x2C76, 0xFF, 0x33},       // KN=0, KP=3, KM=3    @@VVV
    {0x2C77, 0xFF, 0x00},       //     @@VVV
    {0x2C78, 0xFF, 0x06},       // DDIV=2, FBDIV=12  @@VVV
    {0x2C79, 0xFF, 0x00},       //     @@VVV

    {0x2CA2, 0x0F, 0x0A},       // PLL reference clock=CODEC    @@VVV
    {0x2C20, 0xFF, 0x32},       // Power on HDMI PLL    @@VVV
    {0x2C21, 0xFF, 0x00},       // Power on HDMI PLL    @@VVV
    {0x2C22, 0xFF, 0x00},       // Power on HDMI PLL    @@VVV
    {0x2C23, 0xFF, 0x01},       // Power on HDMI PLL    @@VVV
    {0x2C24, 0xDF, 0x00},       // HDMI Control @@VVV
    {0x2C25, 0xFF, 0x00},       // HDMI Control @@VVV
    {0x2C26, 0xFF, 0x33},       // HDMI Control @@VVV
    {0x2C27, 0xFF, 0x00},       // HDMI Control @@VVV
    {0x2C28, 0xFF, 0x03},       // DDIV ; FBDIV @@VVV
    {0x2C29, 0xFF, 0x00},       // DDIV ; FBDIV @@VVV

    {0x2CA4, 0xFF, 0xFF},       // Enable all audio clock @@VVV
    {0x2CA5, 0x47, 0x47},       // @@VVV

    {0x2C00, 0x03, 0x00},       // @@Audio reset
    {0x2C00, 0x03, 0x01},       // @@Audio reset
    {0x2C00, 0x03, 0x03},       // @@Audio reset
    {0x2C00, 0x03, 0x01},       // @@Audio reset
    {0x2C00, 0x03, 0x00},       // @@Audio reset


 //-----------------SRC & synthesizer ratesetting---------------------
    {0x2BCF, 0x1F, 0x00},  // N.F controlled by 51

    {0x2BBC, 0xFF, 0xE0},  // Group A SRC rate
    {0x2BBD, 0xFF, 0x07},
    {0x2BBE, 0xFF, 0x50},
    {0x2BBF, 0xFF, 0x46},
    {0x2BC0, 0xFF, 0x00},
    {0x2BC1, 0xFF, 0x08},
    {0x2BC2, 0xFF, 0x50},
    {0x2BC3, 0xFF, 0x46},

    {0x2BC4, 0xFF, 0xE0},  // Group B SRC rate
    {0x2BC5, 0xFF, 0x07},
    {0x2BC6, 0xFF, 0x50},
    {0x2BC7, 0xFF, 0x46},
    {0x2BC8, 0xFF, 0x00},
    {0x2BC9, 0xFF, 0x08},
    {0x2BCA, 0xFF, 0x50},
    {0x2BCB, 0xFF, 0x46},

    {0x2BA8, 0xFF, 0x94},  // DVB1 N.F
    {0x2BA9, 0xFF, 0x11},
    {0x2BAA, 0xFF, 0x00},
    {0x2BAB, 0xFF, 0x00},

    {0x2BAC, 0xFF, 0x94},  // DVB2 N.F
    {0x2BAD, 0xFF, 0x11},
    {0x2BAE, 0xFF, 0x00},
    {0x2BAF, 0xFF, 0x00},

    {0x2BB0, 0xFF, 0x94},  // DVB3 N.F
    {0x2BB1, 0xFF, 0x11},
    {0x2BB2, 0xFF, 0x00},
    {0x2BB3, 0xFF, 0x00},

    {0x2BB4, 0xFF, 0x94},  // DVB4 N.F
    {0x2BB5, 0xFF, 0x11},
    {0x2BB6, 0xFF, 0x00},
    {0x2BB7, 0xFF, 0x00},

    {0x2BB8, 0xFF, 0x94},  // NonPCM
    {0x2BB9, 0xFF, 0x11},
    {0x2BBA, 0xFF, 0x00},
    {0x2BBB, 0xFF, 0x00},

    {0x2C6E, 0x88, 0x88}, // Enable Group A/B SRC
  //  {0x2C8B, 0x40, 0x40}, // Enable NON-PCM

    {0x2BCE, 0xFF, 0xFF}, // Toggle 2BCE to update N.F value
    {0x2BCE, 0xFF, 0x00},

 //-------------enable clock & function block---------------
    {0x2C02, 0xFF, 0x8F},       //Enable audio synthesizer module @@VVV
    {0x2C03, 0xFF, 0x51},       //@@VVV
//    {0x2C03, 0xFF, 0x11},       //@@VVV
    {0x2C0A, 0xFF, 0x90},       //S/PDIF CDR Coarse Frequency Detection threshold  @@VVV
    {0x2C0B, 0xFF, 0x53},       //@@VVV

    {0x2C2A, 0xFF, 0xA3},       // HDMI:Select CTS, for MIK 7256 480P mode @@VVV
    {0x2C44, 0xFF, 0x10},       // HDMI Matrix mapping   @@VVV
    {0x2C45, 0xFF, 0x10},       // HDMI Matrix mapping  @@VVV
    {0x2C46, 0xFF, 0x10},       // HDMI Matrix mapping  @@VVV
    {0x2C47, 0xFF, 0x10},       // HDMI Matrix mapping  @@VVV
    {0x2C48, 0x0F, 0x08},       // @@VVV

  //----------------Audio Path initialize-------------------------
    {0x2C60, 0xFF, 0x80},       // Decoder 1 source from DVB1  @@VVV
    {0x2C61, 0xFF, 0x80},       // Decoder 3 source from DVB3  @@VVV
    {0x2C62, 0xFF, 0x80},       // Decoder 2 source from DVB2  @@VVV
    {0x2C63, 0xFF, 0x87},       // Decoder 4 source from SIF  @@VVV
    {0x2C64, 0xFF, 0x00},       // CH1--> Multichannel in ; disabled    @@VVV
    {0x2C66, 0xFF, 0x00},       // CH2--> Multichannel in ; disabled    @@VVV
    {0x2C68, 0xFF, 0x00},       // CH3--> Multichannel in ; disabled    @@VVV
    {0x2C6A, 0xFF, 0x00},       // CH4--> Multichannel in ; disabled    @@VVV
    {0x2C65, 0xFF, 0x88},       // CH5(Main)--> ADC0 in @@VVV
    {0x2C67, 0xFF, 0x82},       // CH6--> Decoder3 in (AD)  @@VVV
    {0x2C69, 0xFF, 0x83},       // CH7--> Decoder4 in (ATV) @@VVV
    {0x2C6B, 0xFF, 0x07},       // CH8--> SRC in (B/T); disabled @@VVV

    {0x2C6C, 0xF0, 0xF0},       // enable SIF/I2S/SPDIF/HDMI auto re-gen @@VVV
    {0x2C6D, 0xFF, 0x00},       // disable DVB1~3 auto regen.  @@VVV

    {0x2C8A, 0xFF, 0x90},       // S/PDIF setting : PCM & SRC mode @@VVV
    {0x2C8B, 0x40/*C0*/, 0x40/*C0*/},       // Enable non-PCM @@VVV

	{0x2C8A, 0xFF, 0xD0},       // S/PDIF setting : Reset
    {0x2C8A, 0xFF, 0x90},       // S/PDIF setting : PCM & SRC mode @@VVV

    {0x2C8C, 0xFF, 0x66},       // I2S out setting Synth. PLL 256fs ; 32-bit format @@VVV
    {0x2C8D, 0xFF, 0x80},       // I2S out enable @@VVV

    {0x2C8D, 0xFF, 0xC0},       // I2S out Reset
    {0x2C8D, 0xFF, 0x80},       // I2S out enable @@VVV

    {0x2C90, 0x7F, 0x08},       // Enable I2S & S/PDIF pins output @@VVV

    {0x2CA0, 0xFF, 0x28},       // N.F=48 KHz @@VVV
    {0x2CA1, 0xFF, 0x23},       //  @@VVV

// ----------------------------------------------------
//  SIF initialize
//-----------------------------------------------------
    {0x2CC8, 0xFF, 0xE0},       // SIF MUX setting @@VVV
    {0x2CC9, 0xFF, 0x30},       // Default in VIF mode @@VVV
   {0xB0B42,0xFF, 0x00},       // @@VVV
   {0xB0B43,0xFF, 0x00},       // @@VVV
   {0xB3314,0xFF, 0x00},       // @@VVV
   {0xB3315,0xFF, 0x00},       // @@VVV

   {0xA346A,0xFF, 0x04},       // @@VVV
   {0xA346B,0xFF, 0x16},       // @@VVV
   {0xA346B,0xFF, 0x06},       // @@VVV

   {0xA3466,0xFF, 0x02},       // @@VVV
   {0xA3467,0xFF, 0x09},       // @@VVV
   {0xA3460,0xFF, 0x00},       // @@VVV
   {0xA3461,0xFF, 0x10},       // @@VVV
   {0xA3402,0xFF, 0x20},       // @@VVV
   {0xA3403,0xFF, 0x02},       // @@VVV
   {0xA341E,0xFF, 0x80},       // @@VVV
   {0xA341F,0xFF, 0x00},       // @@VVV
   {0xA3418,0xFF, 0x02},       // @@VVV
   {0xA3419,0xFF, 0x00},       // @@VVV

   {0x2CC6, 0x01, 0x01},       // SIF AGC control by DSP
   {0x2CB0, 0xE0, 0xE0},       // fast clock for FM DEMODULATOR
// ----------------------------------------------------
//  ADC & DACinitialize
//-----------------------------------------------------
    {0x2CDA, 0x03, 0x00},       // power on ADC0 & ADC1@@VVV
    {0x2CDB, 0x03, 0x03},       // Enable ADC0 chopper
    {0x2CEE, 0x03, 0x03},       // enable ADC dither @@VVV
    {0x2CDC, 0xF0, 0x00},       // DAC 0~3 power on @@VVV
    {0x2CDD, 0x3F, 0x00},       // @@VVV
    {0x2CDE, 0xFC, 0x1C},       //power on  AA ; AA source=DAC2.  @@VVV
    {0x2CDF, 0x78, 0x78},       // enable DWA re-latch clock.  @@VVV

    {0x2CE0, 0x30, 0x00},       // power on HP driver @@VVV
    {0x2CE1, 0x1C, 0x0C},       // power on HP input switch @@VVV
//    {0x2CE2, 0xFF, 0x00},       // power on mic amp @@VVV
    {0x2CE2, 0xFF, 0x18},       // mstar modify,change ADC1 default to Mono input (because if ADC0 and ADC1 select LINE0 on same time, have LINE0 level down)
    // LGE yckee, 091019, 0x2CE3,  00 -> 40,
    {0x2CE3, 0x43, 0x40},       // power on ADC PGA @@VVV
    {0x2CE5, 0xFF, 0x44},       // ADC0 & ADC1 PGAain=-3dB @@VVV
    {0x2CE6, 0x03, 0x00},       // power on bias current gen . @@VVV
    {0x2CE7, 0x88, 0x00},       // Enable Micro-phone  @@VVV
    {0x2CE8, 0xFF, 0x00},       //  @@VVV
    {0x2CE9, 0xFF, 0x00},       // power on Vref @@VVV

  //-----------------MAC setting------------------------
    {0x2B40, 0x09, 0x08},       // clear SRC decimation control reset . @@VVV
    {0x2B40, 0x09, 0x09},       // clear SRC decimation control reset . @@VVV
    {0x2B40, 0x09, 0x08},       // SRC decimation control reset . @@VVV
    {0x2B42, 0xFF, 0x00},       // Disable BB by-pass @@VVV
    {0x2B46, 0xFF, 0x3F},       // Enable Group C . @@VVV
    {0x2B47, 0xFF, 0xFF},       // CIC Filter enable . @@VVV
    {0x2B56, 0x30, 0x30},       // Enable ADC1 & ADC2 CODEC . @@VVV
    {0x2B57, 0xFF, 0x00},       // Enable ADC1 & ADC2 CODEC . @@VVV
    {0x2B5C, 0xFF, 0x00},       // SRC decimation control reset . @@VVV
//    {0x2B5D, 0xFF, 0x88},       // SRC decimation control reset . @@VVV
    {0x2B5D, 0xFF, 0xB8},       // Mstar modify 2009/11/18
    {0x2B54, 0xFF, 0x01},       // SDM output enable . @@VVV
    {0x2B55, 0xFF, 0x04},       // SDM output enable . @@VVV

// ----------------------------------------------------
//  sound effect init settings
//-----------------------------------------------------
    {0x2D01, 0xFF, 0x8C},       // AUOUT0 volume :  0dB ; Mute  @@VVV
    {0x2D03, 0xFF, 0x8C},       // AUOUT1 volume :  0dB ; Mute  @@VVV
    {0x2D05, 0xFF, 0x8C},       // AUOUT2 volume :  0dB ; Mute  @@VVV
    {0x2D07, 0xFF, 0x8C},       // AUOUT3 volume :  0dB ; Mute  @@VVV
    {0x2D09, 0xFF, 0x8C},       // I2S_OUT volume :  0dB ; Mute @@VVV
    {0x2D0B, 0xFF, 0x8C},       // SPDIF_OUT volume :  0dB ; Mute   @@VVV

    {0x2C94, 0x01, 0x01},       // enable CH1 HW force mute   @@VVV
    {0x2C96, 0x01, 0x01},       // enable CH2 HW force mute   @@VVV
    {0x2C98, 0x01, 0x01},       // enable CH3 HW force mute   @@VVV
    {0x2C9A, 0x01, 0x01},       // enable CH4 HW force mute   @@VVV
    {0x2C95, 0x01, 0x01},       // enable CH5 HW force mute   @@VVV
    {0x2C97, 0x01, 0x01},       // enable CH6 HW force mute   @@VVV
    {0x2C99, 0x01, 0x01},       // enable CH7 HW force mute   @@VVV

    {0x2B51, 0xFF, 0x00},       //  HW DC offset value [15:8]  ; @@VVV
    {0x2B50, 0xFF, 0x40},       // HW DC offset value [7:0]     @@VVV
    {0x2B52, 0xFF, 0x0F},       // enable  DAC0~3 DC offset     @@VVV

    {0x2D20, 0x80, 0x00},       // Disable EQ @@VVV
    {0x2D21, 0xFF, 0xC8},       // Enable Sound effect & tone @@VVV
/* crazyrun 20091209 :: add Mstar's patch for avoiding SIF&Infinite Sound ON noise problem */
   {0x2D22, 0x1F, 0x16},	   // Enable all output Volume control @@VVV
//    {0x2D22, 0x1F, 0x1F},       // Enable all output Volume control @@VVV

    {0x2D23, 0x01, 0x01},       // Enable SPDIF_OUT volume control   @@VVV
    {0x2D2A, 0x10, 0x10},       // Enable I2S balance control
    {0x2D31, 0x02, 0x00},       // disable SE-DSP power-down command    @@VVV
    {0x2D55, 0xF0, 0x40},       // SRC source from PCM  @@VVV
    {0x2D41, 0x80, 0x80},      //set advance sound effect enable
    {0xFFFF, 0x00, 0x00},       // end of table
};

MS_U16 Reg_Restore[][2]=
{
    {0x2D14, 0x0000},
    {0x2D16, 0x0000},
    {0x2D22, 0x0000},
    {0x2D2E, 0x0000},
    {0x2D50, 0x0000},
    {0x2D52, 0x0000},
    {0x2D54, 0x0000},
    {0x2D8E, 0x0000},
    {0x2DD8, 0x0000},
    {0x2C00, 0x0000},
    {0x2C02, 0x0000},
    {0x2C04, 0x0000},
    {0x2C06, 0x0000},
    {0x2C08, 0x0000},
    {0x2C0A, 0x0000},
    {0xFFFF, 0xFFFF},
};

MS_U16 Vol_Reg_Restore[][2]=
{
    {0x2D00, 0x0000},
    {0x2D02, 0x0000},
    {0x2D04, 0x0000},
    {0x2D06, 0x0000},
    {0x2D08, 0x0000},
    {0x2D0A, 0x0000},
    {0xFFFF, 0xFFFF},
};

//--------------------------------------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------------------------------------
U16 MDrv_MAD_ReadREG(U16 mailbox)
{
    return ( MHal_MAD_ReadReg(mailbox) );
}

void MDrv_MAD_WriteREG(U16 regaddr, U16 data)
{
    MHal_MAD_WriteReg(regaddr, data);
}

void MDrv_MAD_WriteREGMask(U16 u16Addr, U16 u16Mask, U16 u16Value)
{
    MHal_MAD_WriteMaskReg(u16Addr, u16Mask, u16Value);
}


U16 MDrv_MAD_DecReadREG(U16 mailbox)
{
    return ( MHal_MAD_ReadReg(mailbox) );
}

void MDrv_MAD_DecWriteREG(U16 regaddr, U16 data)
{
    MHal_MAD_DecWriteReg(regaddr, data);
}

U8 MDrv_MAD_DecReadREGByte(U16 mailbox)
{
    return ( MHal_MAD_ReadByte(mailbox) );
}

void MDrv_MAD_DecWriteREGByte(U16 regaddr, U8 data)
{
    MHal_MAD_DecWriteRegByte(regaddr, data);
}

void MDrv_MAD_DecWriteREGMask(U16 u16Addr, U16 u16Mask, U16 u16Value)
{
    MHal_MAD_DecWriteRegMask(u16Addr, u16Mask, u16Value);
}

void MDrv_MAD_DecWriteREGMaskByte(U16 u16Addr, U8 u8Mask, U8 u8Value)
{
    MHal_MAD_DecWriteRegMaskByte(u16Addr, u8Mask, u8Value);
}

void MDrv_MAD_DecWriteIntMaskByte(U16 u16Addr, U8 u8Mask)
{
    MHal_MAD_DecWriteIntMaskByte(u16Addr, u8Mask);
}
U16 MDrv_MAD_SeReadREG(U16 mailbox)
{
    return ( MHal_MAD_SeReadReg(mailbox) );
}

U8 MDrv_MAD_SeReadREGByte(U16 mailbox)
{
    return ( MHal_MAD_SeReadRegByte(mailbox) );
}

void MDrv_MAD_SeWriteREG(U16 regaddr, U16 data)
{
    MHal_MAD_SeWriteReg(regaddr, data);
}

void MDrv_MAD_SeWriteREGByte(U16 u16Addr, U8 u8Value)
{
    MHal_MAD_SeWriteRegByte(u16Addr, u8Value);
}

void MDrv_MAD_SeWriteREGMask(U16 u16Addr, U16 u16Mask, U16 u16Value)
{
    MHal_MAD_SeWriteRegMask(u16Addr, u16Mask, u16Value);
}

void MDrv_MAD_SeWriteREGMaskByte(U16 u16Addr, U8 u8Mask, U8 u8Value)
{
    MHal_MAD_SeWriteRegMaskByte(u16Addr, u8Mask, u8Value);
}


//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_AudioInit
//  [Description]:
//      This routine is the initialization for Audio module.
//  [Arguments]:
//      None
//  [Return]:
//      None
//  [Doxygen]
/// This routine is the initialization for Audio module.
//
//*******************************************************************************
void MDrv_MAD_AudioInit(U16 u16MADCodeType)
{
    AUDIO_OUT_INFO OutputInfo;

    MAD_DEBUG_P1(printk("\n\n\n==================================\n\n\n"
                                            "                MDrv_MAD_AudioInit(0x%x)                    \n\n\n"

                                            "==================================\n\n\n", u16MADCodeType));

    if(MDrv_SYS_GetMMAP(E_SYS_MMAP_MAD_BASE,&(stAudioBufInfo.u32Addr),&(stAudioBufInfo.u32Size)) == false)
    {
        DBG_AUDIO_ERROR(printk("MAD audio init fail (Gat base address fail) !!!\n"));
        return;
    }

    //MDrv_MAD_SwResetMAD();

    MHal_MAD_WriteRegTbl(AudioInitTbl);

    DSPMadBaseBufferAdr[DSP_SE] = stAudioBufInfo.u32Addr;
    DSPMadBaseBufferAdr[DSP_DEC] = stAudioBufInfo.u32Addr + MAD_MEM_SE_SIZE;

    MDrv_MAD_SetPlayFileFlag(DSP_DEC,0);
    MDrv_MAD_SetPlayFileFlag(DSP_SE,0);
    MDrv_MAD_SetEncodeDoneFlag(0);

    //======================================
    // Set output info
    //======================================

    OutputInfo.SpeakerOut=AUDIO_OUTPUT_MAIN_SPEAKER;
    OutputInfo.HpOut=AUDIO_OUTPUT_HP;
    OutputInfo.ScartOut=AUDIO_OUTPUT_SIFOUT;
    OutputInfo.MonitorOut=AUDIO_OUTPUT_LINEOUT;
    MDrv_MAD_SetOutputInfo(&OutputInfo);

//================================================
//  Execute Output Path connection
//================================================
    MDrv_MAD_SetOutConnectivity();
//================================================

    MDrv_MAD_SetPowerOn(TRUE);
    MDrv_MAD_SIF_TriggerSifPLL();

    //MDrv_MIU_Mask_Req_AUDIO_RW(1, miu);  //add later
    //MDrv_MIU_Mask_Req_AUDIO_RW(0, miu);  //add later

    MDrv_MAD_SetMemInfo();
    MDrv_MAD2_SetMemInfo();


    MDrv_MAD_SIF_Init();//Load SIF decoder
    MDrv_MAD_SOUND_Init();//Load DSP SE

    // Load SIF DSP code stage
    MDrv_MAD_SetCommand(DVB_DECCMD_STOP);

    MDrv_MAD_Init();// Load DSP DEC
    MDrv_MAD_DecoderLoadCode();

    MDrv_MAD_SPDIF_SetChannelStatus(SPDIF_CHANNEL_STATUS_FS, SPDIF_CS_FS_48K);
    MDrv_MAD_SPDIF_SetChannelStatus(SPDIF_CHANNEL_STATUS_CATEGORY,SPDIF_CS_CATEGORY_DVB);

//Mstar add for Skype, 2009/09/22
//================================================
//  init for Skype
//================================================
    memset((void*)&Skype_information, 0, sizeof(MDRV_SKYPE_T));
//================================================

    dsp_revive_flag = 1;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_Init()
/// @brief \b Function \b Description:  This routine is the initialization for DVB module
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_Init(void)
{
    MHal_MAD_WriteMaskReg(REG_DEC_IDMA_CTRL0, 0x00FF, 0x0002);
    msleep(2);
    MHal_MAD_WriteMaskReg(REG_DEC_IDMA_CTRL0, 0x00FF, 0x0003);
    MDrv_MAD_LoadCode(AU_DVB_STANDARD_MPEG);
    //MDrv_MAD_LoadCode(AU_DVB_DEC_NONE);   //kochien added MPEG_EN test code
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SOUND_Init()
/// @brief \b Function \b Description: This routine is the initialization for Audio sound effect module.
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SOUND_Init(void)
{
    //reset DSP
    MHal_MAD_WriteByte( REG_SE_IDMA_CTRL0, 0x02);
    msleep(10);
    MHal_MAD_WriteByte( REG_SE_IDMA_CTRL0, 0x03);

    // Toggle  to initialize DAC DATA SRAM.
    MHal_MAD_WriteMaskByte(0x2B40,0x02,0x02);
    msleep(1);
    MHal_MAD_WriteMaskByte(0x2B40,0x02,0x00);

    // load DEC part
    MDrv_MAD_DspLoadCode(AU_DVB2_NONE);
    // load AdvSE part
    MDrv_MAD_DspLoadCode(AU_SND_EFFECT);
    // load system part
    MDrv_MAD_SeSystemLoadCode();
    MDrv_MAD2_SetSystem(AU_DVB2_SYS_CV3); // Richard.ni add for CV3 tet
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SetPlayFileFlag()  @Cathy
/// @brief \b Function \b Description:  This function is used to set the Decoder DSP ISR
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetPlayFileFlag(MS_BOOL bDspType, MS_BOOL bSet)
{
    if(bDspType==DSP_DEC)
    {
        if(bSet)
           g_bDecPlayFileFlag = 1;
        else
           g_bDecPlayFileFlag = 0;
    }
    else
    {
        if(bSet)
           g_bSePlayFileFlag = 1;
        else
           g_bSePlayFileFlag = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_AUDIO_GetPlayFileFlag()  @@Cathy
/// @brief \b Function \b Description:  This function is used to set the Decoder DSP ISR
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_GetPlayFileFlag(MS_BOOL bDspType)
{
    if(bDspType==DSP_DEC)
    {
        return g_bDecPlayFileFlag;
    }
    else
    {
        return g_bSePlayFileFlag;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_AUDIO_SetEncodeDoneFlag()  @@Cathy
/// @brief \b Function \b Description:  This function is used to set the Encode done flag
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetEncodeDoneFlag(MS_BOOL bSet)
{
    if(bSet)
       g_bEncodeDoneFlag= 1;
    else
       g_bEncodeDoneFlag = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_AUDIO_GetEncodeDoneFlag()  @@Cathy
/// @brief \b Function \b Description:  This function is used to get the Encoder flag status
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_GetEncodeDoneFlag(void)
{
    return g_bEncodeDoneFlag;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MApi_AUDIO_SetOutputInfo()
/// @brief \b Function \b Description:  Get audio output information from APP
/// @param pout_info   \b : information structure pointer
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetOutputInfo(AUDIO_OUT_INFO *pout_info)
{
    AudioOutputInfo.SpeakerOut=pout_info->SpeakerOut;
    AudioOutputInfo.HpOut=pout_info->HpOut;
    AudioOutputInfo.MonitorOut=pout_info->MonitorOut;
    AudioOutputInfo.ScartOut=pout_info->ScartOut;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MApi_MAD_SetOutConnectivity()
/// @brief \b Function \b Description:  Set the TV output connectivity
///////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetOutConnectivity(void)
{
    MDrv_MAD_SetInternalPath(INTERNAL_PCM_SE, AudioOutputInfo.SpeakerOut);
    MDrv_MAD_SetInternalPath(INTERNAL_PCM_SE, AudioOutputInfo.HpOut);
    MDrv_MAD_SetInternalPath(INTERNAL_PCM_DELAY, AudioOutputInfo.MonitorOut);
    MDrv_MAD_SetInternalPath(INTERNAL_PCM_DELAY, AudioOutputInfo.ScartOut);
    MDrv_MAD_SetInternalPath(INTERNAL_PCM_DELAY, AUDIO_SPDIF_OUTPUT);
    MDrv_MAD_SetInternalPath(INTERNAL_PCM_SE, AUDIO_SRC_IN_LR);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SetInternalPath()    @@VVV
/// @brief \b Function \b Description: This routine is used to set the topalogy for Audio Output.
/// @param <IN>        \b u8Path    : Audio internal path
/// @param <IN>        \b output    : Audio output type
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b  NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetInternalPath(AUDIO_INTERNAL_PATH_TYPE u8Path,  AUDIO_OUTPUT_TYPE u8Output)
{
    MS_U8   path;

    path=(MS_U8)u8Path;

    if(path>8)
      return;

    // Set output
      switch(u8Output)
    {
        case AUDIO_AUOUT0_OUTPUT:
            MHal_MAD_WriteMaskByte(0x2D50, 0x0F, path);
            break;

        case AUDIO_AUOUT1_OUTPUT:
            MHal_MAD_WriteMaskByte(0x2D50, 0xF0, (path<<4));
            break;

        case AUDIO_AUOUT2_OUTPUT:
            MHal_MAD_WriteMaskByte(0x2D51, 0x0F, path);
            break;

        case AUDIO_AUOUT3_OUTPUT:
            MHal_MAD_WriteMaskByte(0x2D51, 0xF0, (path<<4));
            break;

        case AUDIO_I2S_OUTPUT:
            MHal_MAD_WriteMaskByte(0x2D52, 0x0F, path);
            break;

        case AUDIO_SPDIF_OUTPUT:
            MHal_MAD_WriteMaskByte(0x2D55, 0x0F, (MS_U8)INTERNAL_SPDIF); // Fix SPDIF in
            break;

        case AUDIO_SRC_IN_LR:
            MHal_MAD_WriteMaskByte(0x2D55, 0xF0, (MS_U8)(INTERNAL_PCM_SE<<4));
            break;

        default:
	     DBG_AUDIO_ERROR(printk("[ERROR]MDrv_MAD_SetInternalPath:Invalid Audio Output Path\r\n"));
            break;
    }

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SetPowerOn()    @@Need_Modify
/// @brief \b Function \b Description: This routine is used to execute DSP power on/down setting.
/// @param <IN>        \b bPower_on    : TRUE --power on
///                                      FALSE--power off
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetPowerOn(MS_BOOL bPower_on)
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SetPowerOn(0x%x)\r\n", bPower_on));
    if(bPower_on)
    {
       MHal_MAD_WriteMaskByte(REG_D2M_MAILBOX_SE_POWERCTRL, 0x02, 0x00);       // DSP power up command, DO NOT touch bit3
       MHal_MAD_WriteMaskByte(0x2B42, 0xFF, 0x00);      // Disable BB by-pass
    }
    else
    {
        MHal_MAD_WriteMaskByte(REG_D2M_MAILBOX_SE_POWERCTRL, 0x02, 0x02);       // DSP power down command
        if (MHal_MAD_ReadByte(REG_D2M_MAILBOX_SE_POWERCTRL) & 0x01)             // 1 : No wait
        {
            msleep(50);
        }
        else
        {                                               // 0 : wait 3sec
            msleep(3000);
        }

        // Disable MIU Request for DEC-DSP
        MDrv_MAD_Dis_MIUREQ();

        MHal_MAD_WriteMaskByte(0x2B42, 0xFF, 0xFF);      // CH1~7 H/W by-pass
        MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x80, 0x00);      // SPDIF power down
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SwResetMAD()  @@Cathy
/// @brief \b Function \b Description:  This function is used to software reset MAD
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SwResetMAD(void)
{
    /* Audio software engine reset */
    MHal_MAD_WriteMaskByte(REG_AUDIO_SOFT_RESET, 0xFF, 0x00);
    msleep(2);
    MHal_MAD_WriteMaskByte(REG_AUDIO_SOFT_RESET, 0xFF, 0x01);
    msleep(2);
    MHal_MAD_WriteMaskByte(REG_AUDIO_SOFT_RESET, 0xFF, 0x03);
    msleep(2);
    MHal_MAD_WriteMaskByte(REG_AUDIO_SOFT_RESET, 0xFF, 0x01);
    msleep(2);
    MHal_MAD_WriteMaskByte(REG_AUDIO_SOFT_RESET, 0xFF, 0x00);
    msleep(1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MApi_AUDIO_SetCommand()
/// @brief \b Function \b Description: Set decoder Command for Digital Audio module
/// @param enDecComamnd \b : deccoder command for DVB Audio
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetCommand(En_DVB_decCmdType enDecComamnd)
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SetCommand:: Decoder Type [0x%x]\r\n", enDecComamnd));
    switch( enDecComamnd )
    {
        //////////////////////////////////////////////////////
        //
        //     Command for Audio decoder 1
        //
        //////////////////////////////////////////////////////

        case DVB_ENCCMD_START:
            enEncoderStatus = DVB_ENCCMD_START;
            MDrv_MAD_SetEncCmd(AU_DVB_ENCCMD_START);
            break;
        case DVB_ENCCMD_STOP:
            enEncoderStatus = DVB_ENCCMD_STOP;
            MDrv_MAD_SetEncCmd(AU_DVB_ENCCMD_STOP);
            break;
        case DVB_DECCMD_PLAY:
            enDecoderStatus = DVB_DECCMD_PLAY;
            // Reset MIU Request
            MDrv_MAD_DisEn_MIUREQ();
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_PLAY);
            break;

        case DVB_DECCMD_STOP:
            enDecoderStatus = DVB_DECCMD_STOP;
            // Reset MIU Request
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_STOP);
            MDrv_MAD_DisEn_MIUREQ();
            break;

        case DVB_DECCMD_RESYNC:
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_RESYNC);
            break;

        case DVB_DECCMD_FREE_RUN:
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_FREERUN);
            break;

        case DVB_DECCMD_AVSYNC:
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_AVSYNC);
            break;

        case DVB_DECCMD_PLAYFILE:     // start/continue playing file based audio in MHEG5
            MDrv_MAD_HDMI_AC3_PathCFG(FALSE);
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_PLAYFILE);
            break;

        case DVB_DECCMD_PAUSE:    // pause playing file based audio in MHEG5
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_PAUSE);
            break;

        case DVB_DECCMD_PLAYFILETSP:     // start/continue playing file based audio in MHEG5
            MDrv_MAD_HDMI_AC3_PathCFG(FALSE);
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_PLAYFILETSP);
            break;

        case DVB_DECCMD_STARTBROWSE:
            MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_STARTBROWSE);
            break;

        //////////////////////////////////////////////////////
        //
        //     Command for Audio decoder 2
        //
        //////////////////////////////////////////////////////
        case DVB2_DECCMD_PLAY:
            enSeDecoderStatus = DVB2_DECCMD_PLAY;
            // Reset MIU Request
            MDrv_MAD2_DisEn_MIUREQ();
            MDrv_MAD2_SetDecCmd(AU_DVB2_DECCMD_PLAY);
            break;

        case DVB2_DECCMD_STOP:
            enSeDecoderStatus = DVB2_DECCMD_STOP;
            MDrv_MAD2_SetDecCmd(AU_DVB2_DECCMD_STOP);
            // Reset MIU Request
            MDrv_MAD2_DisEn_MIUREQ();
            break;

        case DVB2_DECCMD_RESYNC:
            MDrv_MAD2_SetDecCmd(AU_DVB2_DECCMD_RESYNC);
            break;

        case DVB2_DECCMD_FREE_RUN:
            MDrv_MAD2_SetFreeRun(AU_DVB2_FreeRunMode_FreeRun);
            break;

        case DVB2_DECCMD_AVSYNC:
            MDrv_MAD2_SetFreeRun(AU_DVB2_FreeRunMode_AVsync);
            break;

        case DVB2_DECCMD_PLAYFILE:     // start/continue playing file based audio in MHEG5
            MDrv_MAD2_SetDecCmd(AU_DVB2_DECCMD_PLAYFILE);
            break;

        case DVB2_DECCMD_PAUSE:    // pause playing file based audio in MHEG5
            MDrv_MAD2_SetDecCmd(AU_DVB2_DECCMD_PAUSE);
            break;

        case DVB2_DECCMD_PLAYFILETSP:     // start/continue playing file based audio in MHEG5
            MDrv_MAD2_SetDecCmd(AU_DVB2_DECCMD_PLAYFILETSP);
            break;

        default:
	     DBG_AUDIO_ERROR(printk("[ERROR]MDrv_MAD_SetCommand:Invalid audio decoder command\r\n"));
            break;
    }
}

//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_PowerOn_Melody
//  [Description]:
//      This routine is the initialization for Audio module.
//  [Arguments]:
//      None
//  [Return]:
//      None
//  [Doxygen]
/// This routine is the initialization for Audio module.
//
//*******************************************************************************
void MDrv_MAD_PowerOn_Melody(void)
{
    MAD_DEBUG_P1(printk("MDrv_MAD_PowerOn_Melody()\r\n"));

    if(MDrv_SYS_GetMMAP(E_SYS_MMAP_MAD_BASE,&(stAudioBufInfo.u32Addr),&(stAudioBufInfo.u32Size)) == false)
    {
        DBG_AUDIO_ERROR(printk("[ERROR]MDrv_MAD_PowerOn_Melody:MAD audio init fail (Gat base address fail) !!!\r\n"));
        return;
    }

    MHal_MAD_WriteRegTbl(AudioInitTbl);

    DSPMadBaseBufferAdr[DSP_SE] = stAudioBufInfo.u32Addr;
    DSPMadBaseBufferAdr[DSP_DEC] = stAudioBufInfo.u32Addr + MAD_MEM_SE_SIZE;

    MDrv_MAD_SIF_TriggerSifPLL(); // Richard.Ni 2008-04-21

    // Load DSP SE Code
    MDrv_MAD2_SetMemInfo();
    MDrv_MAD_SOUND_Init();
}

//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_PowerOn_Melody_Path
//  [Description]:
//      This routine set the audio input for melody.
//  [Arguments]:
//      None
//  [Return]:
//      None
//  [Doxygen]
///This routine set the audio input for melody.
//
//*******************************************************************************
void MDrv_MAD_PowerOn_Melody_Path(U8 bPath)
{
    MAD_DEBUG_P1(printk("%s is called\r\n",__FUNCTION__));
#if 0//CHECK
    switch(bPath)
    {
        case 0:
            MDrv_MAD_SetNormalPath(AUDIO_PATH_SRC, AUDIO_AUIN0_INPUT, AUDIO_SRC_OUT);
            break;
        case 1:
            MDrv_MAD_SetNormalPath(AUDIO_PATH_SRC, AUDIO_AUIN1_INPUT, AUDIO_SRC_OUT);
            break;
        case 2:
    	     MDrv_MAD_SetNormalPath(AUDIO_PATH_SRC, AUDIO_AUIN2_INPUT, AUDIO_SRC_OUT);
            break;
        case 3:
            MDrv_MAD_SetNormalPath(AUDIO_PATH_SRC, AUDIO_AUIN3_INPUT, AUDIO_SRC_OUT);
            break;
        case 4:
            MDrv_MAD_SetNormalPath(AUDIO_PATH_SRC, AUDIO_AUIN4_INPUT, AUDIO_SRC_OUT);
            break;
        case 5:
            MDrv_MAD_SetNormalPath(AUDIO_PATH_SRC, AUDIO_AUIN5_INPUT, AUDIO_SRC_OUT);
            break;
        default:
            printk("Error: Path must between 0 ~ 5\n");
            break;
    }
    MDrv_MAD_SetNormalPath(AUDIO_PATH_MAIN_SPEAKER, AUDIO_SRC_INPUT, AUDIO_OUTPUT_MAIN_SPEAKER);//I2S OUT
#endif
}
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SetInputPath()   @@VVV
/// @brief \b Function \b Description: This routine is used to set the topalogy for Audio Input .
/// @param <IN>        \b input    : Audio input type
/// @param <IN>        \b u8Path    : Audio DSP channel
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b  NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetInputPath(AUDIO_INPUT_TYPE input , AUDIO_PATH_TYPE u8Path)
{
    MS_U32 u32path_reg;
    MS_U8  u8input_src, u8input_idx, u8temp, path;

    MAD_DEBUG_P1(printk("MDrv_MAD_SetInputPath::input[%d], path[%d] \r\n", input, u8Path));

    u8input_src = LONIBBLE(input);
    u8input_idx = HINIBBLE(input);

    path=(MS_U8)u8Path;

    if(path<4)
      return;    // path0~3 are used for multi-channel ; we've fixed the setting in audio init table.

    u32path_reg = u32PathArray[path];

    // Set input
    switch(u8input_src)
    {
#if 0
        case AUDIO_DSP1_INPUT:
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x00);
            MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG, 0x07,u8input_idx);
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
         break;
#endif

        case AUDIO_HDMI_NPCM_INPUT:
            MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x09);
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x00);
            MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG, 0x07,u8input_idx);
         break;

        case AUDIO_DSP1_INPUT:
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x00);
            MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG, 0x07,u8input_idx);
            MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
         break;

        case AUDIO_DSP2_INPUT:
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x01);
            MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER2_CFG, 0x07,u8input_idx);
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
            break;

        case AUDIO_DSP3_INPUT:
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x02);
            MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER3_CFG, 0x07,u8input_idx);
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
            break;

        case AUDIO_DSP4_INPUT:
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x03);
            MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER4_CFG, 0x07,u8input_idx);
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
            break;

        case AUDIO_ADC_INPUT:
            u8temp = u8input_idx<<5;
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x08);
            MHal_MAD_WriteMaskByte(0x2CE2, 0xE0, u8temp );
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
            break;

        case AUDIO_ADC2_INPUT:
            u8temp = u8input_idx<<2;
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x09);
            MHal_MAD_WriteMaskByte(0x2CE2, 0x1C, u8temp );
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
            break;

        case AUDIO_SPDIF_INPUT:
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x06);
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
            break;

        case AUDIO_I2S_INPUT:
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x05);
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
            break;

        case AUDIO_HDMI_INPUT:
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x09);
            MHal_MAD_WriteMaskByte(u32path_reg, 0x0F, 0x04);
            break;

        case AUDIO_SRC_INPUT:
            MHal_MAD_WriteMaskByte(u32path_reg, 0x8F, 0x87);
                MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
            break;

        default:
	     DBG_AUDIO_ERROR(printk("[ERROR]MDrv_MAD_SetInputPath:Invalid Audio Input Path\r\n"));
            break;
    }

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_Set_Power_Off()    @@Need_Modify
/// @brief \b Function \b Description: This routine is used to execute DSP power off/on setting.
/// @param <IN>        \b bPower_on    : TRUE --power off
///                                      FALSE--power on
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_Set_Power_Off(U8 OnOff_flag)
{
    return;//Do not need
    MAD_DEBUG_P1(printk("MDrv_MAD_Set_Power_Off(%x)\r\n", OnOff_flag));
    if(!OnOff_flag)
    {
       MHal_MAD_WriteMaskByte(REG_D2M_MAILBOX_SE_POWERCTRL, 0x02, 0x00);       // DSP power up command, DO NOT touch bit3
       MHal_MAD_WriteMaskByte(0x2B42, 0xFF, 0x00);      // Disable BB by-pass //CHCECK
    }
    else
    {
        MHal_MAD_WriteMaskByte(REG_D2M_MAILBOX_SE_POWERCTRL, 0x02, 0x02);       // DSP power down command
        if (MHal_MAD_ReadByte(REG_D2M_MAILBOX_SE_POWERCTRL) & 0x01)             // 1 : No wait
        {
            msleep(50);
        }
        else
        {                                               // 0 : wait 3sec
            msleep(3000);
        }

        // Disable MIU Request for DEC-DSP
        MDrv_MAD_Dis_MIUREQ();

        MHal_MAD_WriteMaskByte(0x2B42, 0xFF, 0xFF);      // CH1~7 H/W by-pass //CHCECK
        MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x80, 0x00);      // SPDIF power down
    }
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_DecoderLoadCode
//  [Description]:
//      This routine load the Processor Binary code.
//  [Arguments]:
//      None
//  [Return]:
//      None
//
//*******************************************************************************
BOOL MDrv_MAD_DecoderLoadCode(void)
{
    MS_U16 time_out = 0;
    MDrv_MAD_SetDspIDMA();

    MAD_DEBUG_P1(printk("MDrv_MAD_DecoderLoadCode() \r\n"));

    //MDrv_MAD_DspLoadCode(AU_DVB_ENC_NONE);
    MDrv_MAD_DspLoadCode(AU_DVB_STANDARD_MPEG_EN);      //kochien added for MPEG_en

    MDrv_MAD_DspLoadCode(AU_DEC_SYSTEM);

    //Wait Dsp init finished Ack
    while(time_out++<100)
    {
        if(MDrv_MAD_GetLoadCodeAck() == 0xE3)
            break;
        msleep(2);
    }
    if(time_out>=100)
    {
        DBG_AUDIO_ERROR(printk("audio DSP_DEC Re-Active\n"));
    }
    else {
        MAD_DEBUG_P1(printk("audio DSP_DEC LoadCode success..\n"));
    }

    //enable SIF Channel
    MDrv_MAD_SIF_ENABLE_CHANNEL(1);

    //inform DSP to start to run
    MDrv_MAD_SetMcuCmd(0xF3);

    MAD_DEBUG_P1(printk("DEC System code download finished!\r\n"));
    return TRUE;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_AlgReloadCode
//  [Description]:
//      Reload algorithm dsp code
//  [Arguments]:
//      AUDIO_ALG_INFO contains the info of Algorithm dsp code
//  [Return]:
//      Sucess / Fail
//
//*******************************************************************************

BOOL MDrv_MAD_AlgReloadCode(U8 decoder_type)
{
    int time_out;

    // Enter MCU/DSP hand-shake
    if((decoder_type & 0xF0) == 0x20)
    {
	 MDrv_MAD_SetMcuCmd(0xF4);
    }
    else
    {
        MDrv_MAD_SetMcuCmd(0xF0);
    }
    // PIO[8] interrupt
    MDrv_MAD_TriggerPIO8();

    //Wait Dsp Start reload Ack
    time_out = 0;
    while(time_out++<1000)
    {
        if(MDrv_MAD_GetReloadCodeAck() == 0x33)
            break;
        //msleep(2);
        udelay(2);
    }
    if(time_out>=1000)
    {
        DBG_AUDIO_ERROR(printk("  DSP Re-active1: %d\r\n",time_out));
        return FALSE;
    }

    // Change to IDMA Port
    MDrv_MAD_SetDspIDMA();

    // Start to reload DSP code
    MDrv_MAD_DspLoadCode(decoder_type);

    // Enter MCU/DSP hand-shake
    if((decoder_type & 0xF0) == 0x20)
    {
	 MDrv_MAD_SetMcuCmd(0xF5);
    }
    else
    {
        MDrv_MAD_SetMcuCmd(0xF1);
    }

        MDrv_MAD_TriggerPIO8();


    // Wait Dsp End Reload Ack
    time_out = 0;
    while(time_out++<1500)
    {
        if(MDrv_MAD_GetReloadCodeAck() == 0x77)
            break;
        //msleep(2);
        udelay(2);//loadcheck

    }

    if(time_out>=1500)
    {
        DBG_AUDIO_ERROR(printk("  DSP Re-active2: %d\r\n",time_out));
        return FALSE;
    }

    MDrv_MAD_SetMcuCmd(0x00);// In T3, clear 0x2D9C after reload finish
    return TRUE;
}

//******************************************************************************
//  [Function Name]: ok
//      MDrv_MAD_DSP_chkIdmaReady
//  [Description]:
//      use to check the status of idma
//  [Arguments]:
//      IdmaChk_type:
//          0x10 : check write ready
//          0x80 : check read  ready
//  [Return]:
//      TRUE : READY
//      FALSE: Not READY
//
//*******************************************************************************
BOOL MDrv_MAD_DecDSP_chkIdmaReady(U8 u8IdmaChk_type)
{
    MS_U8  j = 0;

    while(j<200)
    {
        j++;
        if( (MHal_MAD_ReadByte(REG_DEC_IDMA_CTRL0)& u8IdmaChk_type) == 0 )
            return TRUE;
    }

    DBG_AUDIO_ERROR(printk("DSP DEC Idma check ready fail!(%d)\r\n",j));
    return FALSE;
}

BOOL MDrv_MAD_SeDSP_chkIdamReady(U8 u8IdmaChk_type )
{
    MS_U8  j = 0;

    while(j<200)
    {
        j++;
        if( (MHal_MAD_ReadByte(REG_SE_IDMA_CTRL0)& u8IdmaChk_type) == 0 )
            return TRUE;
	//MsOS_DelayTask(1);
    }

    DBG_AUDIO_ERROR(printk("DSP SE Idma check ready fail!(%d)\r\n",j));
    return FALSE;
}

/******************************************************************************/
///Reset MAD Mudule
/******************************************************************************/
void MDrv_MAD_ResetMAD()
{
    OS_Delayms(1);

    // Reset MAD module
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0040 );
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0080, 0x0080 );

    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0040, 0x0040 );
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0080, 0x0080 );

    OS_Delayms(1);

    // Set MAD module
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0080, 0x0000 );
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0000 );

    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0080, 0x0000 );
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0040, 0x0000 );


    OS_Delayms(1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_ReadMailBox()    @@Need_Modify
/// @brief \b Function \b Description:  This routine is used to read the Dec or SE DSP mail box value
/// @param <IN>        \b bDspType    : 0 --DEC     1 -- SE
/// @param <IN>        \b u8ParamNum    : Mail box address
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b  U16    : Mail Box value
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_U16 MDrv_MAD_ReadMailBox(MS_BOOL bDspType, MS_U8 u8ParamNum)
{
    MS_U16 u16Tmp1, u16Tmp2;
    MS_U32 i, u32MailReg;


    for (i=0; i<1000; i++)
    {
        if(bDspType==DSP_SE)
        {
            if(u8ParamNum<8)
            {
                u32MailReg = 0x2D70+(MS_U32)u8ParamNum * 2;
            }
            else
            {
                u32MailReg = REG_SE_D2M_MAIL_BOX_BASE+(MS_U32)(u8ParamNum-8) * 2;
            }
            u16Tmp1 = MHal_MAD_ReadReg(u32MailReg);
            u16Tmp2 = MHal_MAD_ReadReg(u32MailReg);
        }
        else
        {
            if(u8ParamNum<8)
            {
                u32MailReg = 0x2D60+(MS_U32)u8ParamNum * 2;
            }
            else
            {
                u32MailReg = REG_DEC_D2M_MAIL_BOX_BASE+(MS_U32)(u8ParamNum-8) * 2;
            }
            u16Tmp1 = MHal_MAD_ReadReg(u32MailReg);
            u16Tmp2 = MHal_MAD_ReadReg(u32MailReg);
        }
        if(u16Tmp1==u16Tmp2)
        {
          return u16Tmp1;
        }
    }

    DBG_AUDIO_ERROR(printk("Read Mailbox fail! \r\n"));
    return 0;

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_WriteMailBox()   @@Need_Modify
/// @brief \b Function \b Description:  This routine is used to write Dec-DSP mail box
/// @param <IN>        \b bDspType    : 0 --DEC     1 -- SE
/// @param <IN>        \b u8ParamNum    : Mail box address
/// @param <IN>        \b u16Data    :  value
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b  NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_WriteMailBox(MS_BOOL bDspType, MS_U8 u8ParamNum, MS_U8 u8Data)
{
    MS_U32 u32MailReg;

    if(bDspType==DSP_SE)
    {
        u32MailReg = REG_SE_M2D_MAIL_BOX_BASE + (MS_U32)u8ParamNum * 2;
        MHal_MAD_WriteByte(u32MailReg, u8Data);
    }
    else
    {
        u32MailReg = REG_DEC_M2D_MAIL_BOX_BASE + (MS_U32)u8ParamNum * 2;
        MHal_MAD_WriteByte(u32MailReg, u8Data);
    }
}
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_TriggerPIO8()
/// @brief \b Function \b Description:  This function is used to trigger PIO8 init.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////

void MDrv_MAD_TriggerPIO8(void)
{
    MHal_MAD_WriteMaskReg(REG_DEC_IDMA_CTRL0, 0x0020, 0x0020);
    //msleep(2);
    MHal_MAD_WriteMaskReg(REG_DEC_IDMA_CTRL0, 0x0020, 0x0000);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_AuHDMI_Monitor
//  [Description]:
//      This routine report PCM/HDMI non-PCM status
//  [Arguments]:
//      None
//*******************************************************************************
U8 MDrv_MAD_HDMI_Monitor(void)
{
    MHal_MAD_AbsWriteMaskByte(0x1027C8, 0x02, 0x02);        // Add  audio bank offset
    return (MHal_MAD_ReadByte(REG_AUDIO_STATUS_INPUT));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_HDMI_Monitor2
//  [Description]:
//      This routine report PCM/HDMI non-PCM status from DSP
//  [Arguments]:
//      None
//*******************************************************************************
U8 MDrv_MAD_HDMI_Monitor2(void)
{
    return (MHal_MAD_ReadByte(0x2DB4));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_AUDIO_HDMI_DolbyMonitor()  @@Need_Modify
/// @brief \b Function \b Description:  Report HDMI non-PCM Dolby mod status
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b  u8DolbyModeType    :
///                                0-- Other mode
///                                1-- Dolby mode
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
U8 MDrv_MAD_HDMI_DolbyMonitor(void)
{
    return MHal_MAD_ReadReg(REG_AUDIO_STATUS_HDMI_PC);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_HDMI_SetNonpcm
//  [Description]:
//      This routine : HDMI
//  [Arguments]:
//      None
//*******************************************************************************
//HDMIv1
void MDrv_MAD_HDMI_SetNonpcm(U8 nonPCM_en)//CHECK
{
    if (nonPCM_en == 1)
    {
        MHal_MAD_WriteMaskByte(REG_DEC_DECODE_CMD,0x1F,0x00);       // Stop
        MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG,0x07,0x04);
/* 20091217 biohu :: remove pop noise for hdmi mode. Mstar will release this code later */
        MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x01);
        msleep(50);
        //MHal_MAD_WriteMaskByte(REG_DEC_DECODE_CMD,0x1F,0x01);       // Play //20091126 mstar remove
    }
    else
    {
        MHal_MAD_WriteMaskByte(REG_DEC_DECODE_CMD,0x1F,0x00);       // Stop
/* 20091217 biohu :: remove pop noise for hdmi mode. Mstar will release this code later */
        MDrv_MAD_HDMIAutoMute(AUDIO_PATH_MAIN, 0x09);
		msleep(50);

    }
}


//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_HDMI_AC3_PathCFG
//  [Description]:
//      Set DVB2AD/HDMIAC3 path Control
//  [Arguments]:
//      ctrl mode :
//          0: DVB2_AD path
//          1: HDMI path
//  [Return]:
//      None
//  [Doxygen]
/// Set DVB2_AD/HDMIAC3 path Control (DVB2/HDMI).
/// @param path_ctrl \b IN:
///   - 0: DVB2_AD path
///   - 1: HDMIAC3 path
//
//*******************************************************************************
//HDMIv1
void MDrv_MAD_HDMI_AC3_PathCFG(U8 u8Ctrl)
{
    if (u8Ctrl == 1)
    {   // HDMI
        /* use CH1 decode HDMI AC3 */
        MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG,0x07,0x04);        // HDMI_AC3 REG CFG
    }
    else
    {   // DTV
        MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG,0x07,0x00);        // DVB1 REG CFG
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_SPDIF_EN
//  [Description]:
//      This routine set SPDIF output enable.
//  [Arguments]:
//      Register:  0x2D2E[0]
//*******************************************************************************
void MDrv_MAD_SPDIF_EN(U8 spdif_en)//CHECK //KH
{
    if(spdif_en == 1)
    {
        //MHal_MAD_WriteMaskReg(0x2D2E, 0x0001, 0x0000);    // For non-PCM
    }
    else
    {
        //MHal_MAD_WriteMaskReg(0x2D2E, 0x0001, 0x0001);
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_AuSPDIF_SetMute
//  [Description]:
//      This routine set SPDIF output mute.
//  [Arguments]:
//      Register:  0x2C2E[0]
//*******************************************************************************
void MDrv_MAD_SPDIF_SetMute(MS_BOOL bEnMute)
{
    MS_U8 spdif_timecnt;

	MAD_DEBUG_P1(printk("MDrv_MAD_SPDIF_SetMute is called [%x], REG_M2D_MAILBOX_SPDIF_CTRL = %x\n",
			bEnMute, (MHal_MAD_ReadByte(REG_M2D_MAILBOX_SPDIF_CTRL) & 0x02)));

    if(bEnMute == 1)
    {
    	// Use SPDIF mute both SW mute and HW mute temporary by Jonghyuk Lee, 091111
//        if (MHal_MAD_ReadByte(REG_M2D_MAILBOX_SPDIF_CTRL) & 0x02)
        {  // nonPCM mode
            MHal_MAD_WriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x01, 0x01);        // Software Mute
            //MHal_MAD_WriteMaskByte(0x2c8b, 0x20, 0x20);    // Hardware Mute
        }
//        else	// Use SPDIF mute both SW mute and HW mute temporary by Jonghyuk Lee, 091111
        {  // PCM mode
            MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOL_FRAC+1, 0x80, 0x80);     //Fading-out mute
        }
    }
    else
    {
#if 0	// Use SPDIF mute both SW mute and HW mute temporary by Jonghyuk Lee, 091111
        if (MHal_MAD_ReadByte(REG_M2D_MAILBOX_SPDIF_CTRL) & 0x02)
        {  // nonPCM mode
            MHal_MAD_WriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x01, 0x00);        // Software unMute
            //MHal_MAD_WriteMaskByte(0x2c8b, 0x20, 0x00);     // Hardware Unmute
            MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOL_FRAC+1, 0x80, 0x00);     //Fading-in unmute
        }
        else
#endif
        {  // PCM mode
            MHal_MAD_WriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x01, 0x00);
            for (spdif_timecnt = 0; spdif_timecnt<30; spdif_timecnt++)
            {
                msleep(1);
            }
            MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOL_FRAC+1, 0x80, 0x00);     //Fading-in unmute
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SPDIF_SetMode()  @@Need_Modify
/// @brief \b Function \b Description:  This routine is used to set S/PDIF output mode
/// @param <IN>        \b u8Spdif_mode    :
///                                    bit[0] = 0: spdif enable,   1: spdif disable (Se-DSP)
///                                    bit[1] = 0: PCM mode,     1: non-PCM mode
///                                    bit[2] = 1: non-PCM NULL Payload
/// @param <IN>        \b u8Input_src    :    0 : DTV
///                                    1 : ATV
///                                    2 : HDMI
///                                    3 : ADC
///                                    4 : CardReader
///                                    5 : SPDIF
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SPDIF_SetMode(MS_U8 u8Spdif_mode, MS_U8 u8Input_src)
{
#if 1
    MAD_DEBUG_P1(printk("MDrv_MAD_SPDIF_SetMode: %d, input source[%d]\n", u8Spdif_mode, u8Input_src ));

    if ( u8Input_src == 2 )                             /* if input is HDMI in */
        if((MDrv_MAD_HDMI_Monitor() & 0xC0) != 0x40)   /* if Type is not Dolby type, bypass */
            u8Spdif_mode = 0;                           /*      set to PCM mode */

    if((u8Spdif_mode & 0x2) == 0x2)
    {   /* Non PCM Mode */
        switch( u8Input_src )
        {
            case 0:     /* TP in */
                MHal_MAD_WriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x02, 0x02);
                MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x07, 0x02);           /* PCM in */
                MDrv_MAD_SPDIF_SetChannelStatus(SPDIF_CHANNEL_STATUS_PCM_FORMAT, SPDIF_CS_FORMAT_NONPCM);
                break;
            case 2:     /* HDMI in */
            default:
                MHal_MAD_WriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x02, 0x02);
                MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x07, 0x02);           /* Non-PCM in */
                MDrv_MAD_SPDIF_SetChannelStatus(SPDIF_CHANNEL_STATUS_PCM_FORMAT, SPDIF_CS_FORMAT_NONPCM);
                break;
        }
    }
    else
    {   /* PCM Mode */
        switch( u8Input_src )
        {
            case 2:     /* HDMI in */
            default:
                MHal_MAD_WriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x2, 0x0);          /* PCM Mode */
                MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x07, 0x00);           /* Grp A in */
                MDrv_MAD_SPDIF_SetChannelStatus(SPDIF_CHANNEL_STATUS_PCM_FORMAT, SPDIF_CS_FORMAT_PCM);
                break;
        }
    }
#else
    u8Spdif_mode=u8Spdif_mode;
    u8Input_src=u8Input_src;
    MS_U8  spdif_timecnt;
    MS_U8  ch4_mute_tmp;
    MS_U8  spdif_mute_tmp;
    //MS_U16 InputSourceMap[5]={0x0000,0x0006,0x0005,0x0002,0x0000};

    ch4_mute_tmp  = HAL_AUDIO_SeReadByte(REG_SOUND_SPDIF_VOL_FRAC+1) & 0x0080;
    HAL_AUDIO_SeWriteMaskByte(REG_SOUND_SPDIF_VOL_FRAC+1, 0x0080, 0x0080);                     // CH4 mute SPDIF

    spdif_mute_tmp  = HAL_AUDIO_SeReadByte(REG_M2D_MAILBOX_SPDIF_CTRL) & 0x01;

    HAL_AUDIO_SeWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL,0x01,0x01);

    // NON-PCM Mod
    if((u8Spdif_mode & 0x2) == 0x2)
    {   //Fix the pop noise when PCM change to RAW
        //printf("SPDIF_non-PCM mod\r\n");

        if (u8Input_src == 2)
        {   // HDMI
            if((HAL_AUDIO_HDMI_Monitor() & 0xC0) == 0x40)                // HDMI non-PCM / PCM judge for OSD UI Set
            {  // Input HDMI-nonPCM mod, UI SPDIF = Auto
                  {   // Dolby mod
                      //printf("HDMI SPDIF Dolby Mod\n");
                      for (spdif_timecnt = 0; spdif_timecnt<30; spdif_timecnt++)
                      {
                          MsOS_DelayTask(1);
                      }
                      HAL_AUDIO_DecWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL,0x08,0x00);        // DVB1 SPDIF Enable
                      HAL_AUDIO_SeWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL,0x08,0x00);         // DVB2 SPDIF Disable
                      HAL_AUDIO_DecWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x02, 0x02);      // non-PCM mode set to DEC DSP
                      HAL_AUDIO_SeWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x02, 0x02);       // non-PCM mode set t0 SND DSP
                      //HAL_AUDIO_WriteMaskByte(0x2C6A, 0x07, 0x02);         // CLK ADC
                      HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x04, 0x04);         // SPDIF Disable SRC
                      HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+0, 0xFF, 0xBB);         // Combine 0x2C8A and 0x2C8B
                      HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+1, 0x06, 0x06);         // Combine 0x2C8A and 0x2C8B
                      HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CS0, 0xFF, 0x40);         // CS
                 }
            }
            else
            {    // Input HDMI-PCM mod, UI SPDIF = auto
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+0, 0xFF, 0xB3);               // Combine 0x2C8A and 0x2C8B
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+1, 0x06, 0x00);               // Combine 0x2C8A and 0x2C8B
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x04, 0x00);               // SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x03, 0x03);              // CH4 Sel HDMI
            }
        }
        else
        {   // DTV, non-PCM out
            //printf("DTV SPDIF Dolby Mod\n");
            for (spdif_timecnt = 0; spdif_timecnt<20; spdif_timecnt++)
            {
                MsOS_DelayTask(1);
            }
            if (g_u8DspCodeType == AU_DVB_STANDARD_AAC)
            {   // HE-AAC
                HAL_AUDIO_DecWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x08, 0x08);    // DVB1 SPDIF Disable
                HAL_AUDIO_SeWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x08, 0x08);     // DDE(DVB2) SPDIF Enable
            }
            else
            {   // AC3,AC3P
                HAL_AUDIO_DecWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x08, 0x00);    // DVB1 SPDIF Enable
                HAL_AUDIO_SeWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x08, 0x00);     // HDMI(DVB2) SPDIF Disable
            }
            HAL_AUDIO_DecWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x02, 0x02);        // non-PCM mode set to DEC DSP
            HAL_AUDIO_SeWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x02, 0x02);         // non-PCM mode set t0 SND DSP
            HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+0, 0xFF, 0xBB);           // non-SRC mode, SPDIF Sel CH4
            HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+1, 0x06, 0x06);           // non-SRC mode, SPDIF Sel CH4
            //HAL_AUDIO_WriteMaskByte(0x2C6A, 0x07, 0x02);           // CLK ADC
            HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CS0, 0xFF, 0x40);           // CS
            HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x04, 0x04);                   // SPDIF Disable SRC
        }
    }

    // PCM MODE
    else
    {
        if(u8Input_src > 4)
        {
            u8Input_src = 4;
        }
        HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CS0, 0x00FF, 0x0000);                   // CS[0] ==> PCM mode
        for (spdif_timecnt = 0; spdif_timecnt<30; spdif_timecnt++)
        {
            MsOS_DelayTask(1);
        }

        HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x0003, 0);        //InputSourceMap[u8Input_src]);      //need touch

        switch(u8Input_src)
        {
            case 0 : // DTV mode
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+0, 0xFF, 0xB3);               // SRC mode,SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+1, 0x06, 0x00);               // SRC mode,SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x04, 0x00);               // SPDIF enable SRC
                break;

            case 2 :
              //printf("HDMI SPDIF PCM Mod\n");
              // HDMI-PCM in Dec-DSP
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+0, 0xFF, 0xB3);
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+1, 0x06, 0x00);
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x04, 0x00);               // SPDIF enable SRC
                if((HAL_AUDIO_HDMI_Monitor() & 0xC0) == 0x40)
                {
                    // HDMI non-PCM in, SPDIF PCM out
                    HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x03, 0x00);
                }
                else // PCM                                             // HDMI PCM in,
                {   // HDMI PCM in, SPDIF PCM out
                    HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x03, 0x03);          // CH4 Sel HDMI
                }
                break;

            case 1 : // ATV mode
            case 3 : // ADC mode
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+0, 0xFF, 0xB3);               // SRC mode,SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+1, 0x06, 0x00);               // SRC mode,SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x04, 0x00);               // SPDIF enable SRC
                break;

            case 4 :  // storage mode, solve DIVX AC3 spdif sound cut.
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+0, 0xFF, 0x9B);               // SRC mode,SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+1, 0x06, 0x00);               // SRC mode,SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x04, 0x04);               // SPDIF disable SRC
                break;

            default:
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+0, 0xFF, 0xB3);               // SRC mode,SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG+1, 0x06, 0x00);               // SRC mode,SPDIF enable SRC
                HAL_AUDIO_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x04, 0x00);               // SPDIF enable SRC
                break;
        };
        HAL_AUDIO_DecWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x02, 0x00);                    // SPDIF enable, PCM mode
        HAL_AUDIO_SeWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x02, 0x00);                     // SPDIF enable, PCM mode
    }
    HAL_AUDIO_SeWriteMaskByte(REG_SOUND_SPDIF_VOL_FRAC+1, 0x0080, ch4_mute_tmp);
    HAL_AUDIO_SeWriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x01, spdif_mute_tmp);
#endif
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_SPDIF_SetSCMS
//  [Description]:
//      This routine Set SPDIF SCMS.
//  [Arguments]:
//      SPDIF SCMS, C_bit_en : copy right control bit
//                             register in 0x2C80[5
//                  L_bit_en : generation bit
//                             register in 0x2C82[0]
//*******************************************************************************
void MDrv_MAD_SPDIF_SetSCMS(BYTE C_bit_en, BYTE L_bit_en)
{
#if 1//CHECK
    if(C_bit_en)
    {
        MHal_MAD_WriteMaskReg(0x2C80, 0x0020, 0x0020);
    }
    else
    {
        MHal_MAD_WriteMaskReg(0x2C80, 0x0020, 0x0000);
    }

    if(L_bit_en)
    {
        MHal_MAD_WriteMaskReg(0x2C82, 0x0001, 0x0001);
    }
    else
    {
        MHal_MAD_WriteMaskReg(0x2C82, 0x0001, 0x0000);
    }
#endif
}
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_SPDIF_GetSCMS
//  [Description]:
//      This routine Get SPDIF SCMS.
//  [Return]:
//      SCMS[0] = C bit status
//      SCMS[1] = L bit status
//*******************************************************************************
BYTE MDrv_MAD_SPDIF_GetSCMS(void)
{
#if 1 //CHECK
    BYTE    C_bit_status, L_bit_status;
    BYTE    SCMS_status, SCMS_C_bit_tmp, SCMS_L_bit_tmp;

    C_bit_status = MHal_MAD_ReadByte(0x2C80)& 0x20;
    L_bit_status = MHal_MAD_ReadByte(0x2C82)& 0x01;

    if(C_bit_status)
    {
        SCMS_C_bit_tmp = 0x01;
    }
    else
    {
        SCMS_C_bit_tmp = 0x00;
    }

    if(L_bit_status)
    {
        SCMS_L_bit_tmp = 0x02;
    }
    else
    {
        SCMS_L_bit_tmp = 0x00;
    }

    SCMS_status = SCMS_C_bit_tmp | SCMS_L_bit_tmp;

    return(SCMS_status);
#endif
return 0;
}


//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_SetPaser
//  [Description]:
//      This routine ENA/STOP Audio Paser for Scramble.
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_SetPaser(U8 paser_stpen)
{
#if 0 //CHECK
    if (paser_stpen == 1)
    {
        MHal_MAD_DecWriteRegMask(0x2D36,0x0F,0x01);        // Only use Low byte
    }
    else
    {
        MHal_MAD_DecWriteRegMask(0x2D36,0x0F,0x00);         // Only use Low byte
    }
#endif
   return;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_ReadPaser
//  [Description]:
//  [Arguments]:
//      None
//*******************************************************************************
U8 MDrv_MAD_ReadPaser(void)
{
    //return (MHal_MAD_ReadReg(0x2D36) & 0x01);//CHECK
    return 0;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_RebootDSP
//  [Description]:
//      This routine reboot DSP.
//  [Arguments]:
//      RebootType: 0: only reset DSP, 1: load code and reset DSP
//*******************************************************************************
void MDrv_MAD_RebootDSP(U8 bDspType)
{
    if (bDspType==DSP_SE)
        MDrv_MAD2_RebootDsp();  //reboot sndEff DSP
    else
        MDrv_MAD_RebootDecDSP();  //reboot DEC DSP
}

#ifdef ENABLE_MADMONITOR  // dsp_revive
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_RebootDecDSP
//  [Description]:
//      This routine reboot Decoder DSP.
//  [Arguments]:
//      RebootType: 0: only reset DSP, 1: load code and reset DSP
//*******************************************************************************
void MDrv_MAD_RebootDecDSP(void)
{
    MS_U8 u8DspCodeType;
    MS_U8 deccmd_status;
    DBG_AUDIO_ERROR(printk("\n*** MAD Auto-Recovery Dec DSP*** \n"));

    u8DspCodeType=MDrv_MAD_GetDSPCodeType();

    // Reset MAD module
    MDrv_MAD_RSTMAD_DisEn_MIUREQ();
    msleep(2);

    DBG_AUDIO_ERROR(printk("*** Load code and reset DEC DSP \n"));
    deccmd_status = MDrv_MAD_GetDecCmd();

    MDrv_MAD_SetMemInfo();
    MDrv_MAD_ResetDSP();
//    MDrv_MAD_LoadCode((AUDIO_DSP_CODE_TYPE)u8DspCodeType);
    MDrv_MAD_DspLoadCode(u8DspCodeType);
    MDrv_MAD_SetIsDtvFlag(TRUE);
    dtv_mode_en = 1;

    MDrv_MAD_DecoderLoadCode();

    MDrv_MAD_SetDecCmd(deccmd_status);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_RebootSndEffDSP
//  [Description]:
//      This routine reboot Sound Effect DSP.
//  [Arguments]:
//      RebootType: 0: only reset DSP, 1: load code and reset DSP
//*******************************************************************************
void MDrv_MAD_RebootSndEffDSP(void)
{
    MS_U8 dec2cmd_status;
    MS_U8 u8Dsp2CodeType=MDrv_MAD_GetDSP2CodeType();

    MDrv_MAD2_Backup_pathreg();

    DBG_AUDIO_ERROR(printk("\n*** MAD Auto-Recovery SndEff DSP*** \n"));

    // Reset MAD module
    MDrv_MAD2_RSTMAD_DisEn_MIUREQ();
    msleep(2);

    DBG_AUDIO_ERROR(printk("*** Load code and reset SE DSP \n"));
    MDrv_MAD2_EnableChIRQ(FALSE);
    dec2cmd_status = MDrv_MAD2_GetDecCmd();
    MDrv_MAD2_SetDecCmd(drvMAD_STOP);
    MDrv_MAD2_SetMemInfo();

    //reset DSP
    MHal_MAD_WriteByte( REG_SE_IDMA_CTRL0, 0x02);
    msleep(10);
    MHal_MAD_WriteByte( REG_SE_IDMA_CTRL0, 0x03);

    MDrv_MAD_DspLoadCode(u8Dsp2CodeType);

    // load AdvSE part
    MDrv_MAD_DspLoadCode(AU_SND_EFFECT);
    // load system part
    MDrv_MAD_SeSystemLoadCode();

    //** mute audio before parameter restore**
    MDrv_MAD_ProcessSetMute(0,1);
    MDrv_MAD_ProcessSetMute(1,1);
    MDrv_MAD_ProcessSetMute(2,1);
    MDrv_MAD_ProcessSetMute(3,1);
    MDrv_MAD_ProcessSetMute(4,1);
    MDrv_MAD_ProcessSetMute(5,1);
    //*****************************************

    MDrv_MAD2_SetSystem(AU_DVB2_SYS_CV3);

    if((u8Dsp2CodeType & 0xF0) == 0xb0)
        MDrv_MAD_SIF_SetDspCodeType(u8Dsp2CodeType);

    MDrv_MAD2_EnableChIRQ(TRUE);
    msleep(500);
    MDrv_MAD2_SetDecCmd(dec2cmd_status);

    MDrv_MAD2_Restore_pathreg();
}
#endif
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_I2S_SetMode
//  [Description]:
//      This routine set I2S MCLK , Word width,format.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_I2S_SetMode(MS_U8 u8Mode, MS_U8 u8Val)
{
   switch(u8Mode)
   {
     case AUDIO_I2S_MCLK:         //0x2C8C[6:4]
       MHal_MAD_WriteMaskByte(REG_AUDIO_I2S_OUT1_CFG,0x70,u8Val<<4);
       break;

     case AUDIO_I2S_WORD_WIDTH:   //0x2C8C[2:0]
       MHal_MAD_WriteMaskByte(REG_AUDIO_I2S_OUT1_CFG,0x07,u8Val);
       break;

     case AUDIO_I2S_FORMAT:      //0x2C8C[3]
       MHal_MAD_WriteMaskByte(REG_AUDIO_I2S_OUT1_CFG,0x08,u8Val<<3);
       break;

     case AUDIO_I2S_SOURCE_CH:
      // No need ; Only select Group A in T3 .
       break;

     default:
	DBG_AUDIO_ERROR(printk("[ERROR]MDrv_MAD_I2S_SetMode:Invalid set I2S mode[%d]\r\n", u8Mode));
       break;
   };
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_SetBTBufferCounter
//  [Description]:
//      Set the DDR buffer according the sampling rate and the frame time
//      ex: if the sampling rate is 48KHz, the frame time is 40ms
//            ==> the frame buffer size is 48000*0.04*2 (L/R) *2(Bytes/sample) = 0x1E00
//                    the counter is 0x1E00/8 = 960 (For 1*Burst DMA)
//  [Arguments]:
//      counter:
//  [Return]:
//      None
//
//*******************************************************************************
void MDrv_MAD_SetBTBufferCounter(MS_U32 u32Counter)
{
    u32Counter &= 0x00FFFFFF;
    MDrv_MAD2_Write_DSP_sram(0x04FF, u32Counter, DSP_MEM_TYPE_PM);
}

void MDrv_MAD_Set_BT_SampleCounter(MS_U32 SampleNum)
{
    SampleNum &= 0x00FFFFFF;
    MDrv_MAD2_Write_DSP_sram(0x1002, SampleNum, DSP_MEM_TYPE_PM);
}
//************************************************************************************************
// [Function Name]:
//      MDrv_AuBT_Upload_Samprate_Set(BOOL enable,U8 Samprate)
// [Description]:
//      According Blue tooth upload path, for different sampling rate setting the Synthesizer.
// [Arguments]:
//      Samprate:  0,no change
//                 1, 48KHz
//                       2, 44KHz
//                            3, 32KHz
//*************************************************************************************************
void MDrv_MAD_BTUpload_Samprate_Set(U8 enable,U16 Samprate)
{
#if 0 //CHECK
    Samprate &= 0x0003;
    if(enable)
    {
       MHal_MAD_WriteReg(0x2C5A, 0x0055 );
       MHal_MAD_WriteMaskReg(0x2D6C,0x0010,0x0010 );
       MHal_MAD_WriteMaskReg(0x2D34,0x0003,Samprate );
    }
    else
    {
       MHal_MAD_WriteReg(0x2C5A, 0 );
       MHal_MAD_WriteMaskReg(0x2D6C,0x0010,0 );
       MHal_MAD_WriteMaskReg(0x2D34,0x0003,0 );
    }
#endif
}


///////// FOR CV3 /////////////////
//========  Function Menu  ===
#define   PROC_ENABLE   (1<<0)          //algorithms on/off
#define   FUNC00        (1<<1)               //internally used
#define   FUNC01        (1<<2)               //AVL2
#define   FUNC02        (1<<3)               //new feature
#define   FUNC03        (1<<4)               //new feature
#define   FUNC04        (1<<5)               //CV2
#define   FUNC05        (1<<6)               //new feature
#define   FUNC06        (1<<7)               //Surround Algotithm
#define   FUNC07        (1<<8)               //UEQ
#define   FUNC08        (1<<9)               //internally used
#define   FUNC09        (1<<10)              //new feature
#define   FUNC10        (1<<11)              //internally used
#define   FUNC11        (1<<12)              //internally used
#define   FUNC12        (1<<13)              //internally used
#define   FUNC13        (1<<14)              //internally used
#define   FUNC14        (1<<15)              //internally used
#define   FUNC15        (1<<16)              //internally used


#define cv3_pvc_monitor_param_addr 0x2880
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_PVC_Monitor
//  [Description]:
//      This routine is Monitor CV3's PVC values
//  [Arguments]:
//
//*******************************************************************************
BOOL MDrv_MAD_CV3_PVC_Monitor(U32 *PVC)
{
    static U32 pvc_monitor = 0x000000;

    pvc_monitor = MDrv_MAD2_Read_DSP_sram(cv3_pvc_monitor_param_addr, DSP_MEM_TYPE_DM);
    *PVC = pvc_monitor;

    return TRUE;
}


#define cv3_volume_param_base_addr  0x3D23
#define CV3_VOLUME_PARAM_NUM 1
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetVolume
//  [Description]:
//      This routine set clear voice LG volume.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetVolume(Cv3VolumeInfo_T *p_volume_info)
{
    static U32 cv3_volume_count = 0x00000000;
    WORD   addr = 0;
    U32 i;

    MAD_DEBUG_P1(printk("MDrv_MAD_CV3_SetVolume\n"));
#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    for (i=0; i< CV3_VOLUME_PARAM_NUM; i++)  {
        addr = cv3_volume_param_base_addr + i;
        MDrv_MAD_SetCV3Para(addr, ((U32 *)p_volume_info)[i]);
    }

    addr = cv3_volume_param_base_addr-1;
    cv3_volume_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_volume_count);
#endif
}

#define cv3_bass_enhance_param_base_addr  0x3D25
#define CV3_BASS_ENHANCE_PARAM_NUM 74
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetBassEnhance
//  [Description]:
//      This routine set clear voice LG bass enhancement.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetBassEnhance(BassEnhanceInfo_T *p_bassenhance_info)
{
    static U32 cv3_bass_enhance_count = 0x00000000;
    WORD   addr = 0;
    U32 i;

    MAD_DEBUG_P1(printk("MDrv_MAD_CV3_SetBassEnhance\n"));
#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    for (i=0; i< CV3_BASS_ENHANCE_PARAM_NUM; i++)  {
        addr = cv3_bass_enhance_param_base_addr + i;
        MDrv_MAD_SetCV3Para(addr, ((U32 *)p_bassenhance_info)[i]);
    }

    addr = cv3_bass_enhance_param_base_addr-1;
    cv3_bass_enhance_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_bass_enhance_count);
#endif
}

#define  cv3_cv_param_base_addr 0x3D70
#define CV3_CV_PARAM_NUM 35
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetCV
//  [Description]:
//      This routine set clear voice 3's clear voice setting.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetCV(Cv3CVinfo_T *p_cv_info)
{
    static U32 cv3_cv_count = 0x00000000;
    U32 i;
    WORD   addr = 0;

    MAD_DEBUG_P1(printk("MDrv_MAD_CV3_SetCV\n"));
#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    for (i=0; i< CV3_CV_PARAM_NUM; i++)  {
        addr = cv3_cv_param_base_addr + i;
        MDrv_MAD_SetCV3Para(addr, ((U32 *)p_cv_info)[i]);
    }

    addr = cv3_cv_param_base_addr-1;
    cv3_cv_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_cv_count);
#endif
}


#define cv3_ueq_param_base_addr  0x3D9D
#define CV3_UEQ_PARAM_NUM 58
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetUEQ
//  [Description]:
//      This routine set clear voice LG EQ.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetUEQ(Cv3EQinfo_T *p_ueq_info)
{
    static U32 cv3_ueq_count = 0x00000000;
    U32 i;
    WORD   addr = 0;

    MAD_DEBUG_P1(printk("MDrv_MAD_CV3_SetUEQ\n"));
#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    for (i=0; i< CV3_UEQ_PARAM_NUM; i++)  {
        addr = cv3_ueq_param_base_addr + i;
        MDrv_MAD_SetCV3Para(addr, ((U32 *)p_ueq_info)[i]);
    }

    addr = cv3_ueq_param_base_addr-1;
    cv3_ueq_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_ueq_count);
#endif
}

#define  cv3_avl2_param_base_addr 0x3CBE
#define CV3_AVL2_PARAM_NUM 40
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetAVL2
//  [Description]:
//      This routine set clear voice LG AVL2.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetAVL2(Cv3AVL2info_T *p_avl2_info)
{
    static U32 cv3_avl2_count = 0x00000000;
    U32 i;
    WORD   addr = 0;

    MAD_DEBUG_P1(printk("MDrv_MAD_CV3_SetAVL2\n"));
#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    for (i=0; i< CV3_AVL2_PARAM_NUM; i++)  {
        addr = cv3_avl2_param_base_addr + i;
        MDrv_MAD_SetCV3Para(addr, ((U32 *)p_avl2_info)[i]);
    }

    addr = cv3_avl2_param_base_addr-1;
    cv3_avl2_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_avl2_count);
#endif
}

#define  cv3_surround_param_base_addr 0x3D94
#define CV3_SURROUND_PARAM_NUM 8
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetSurround
//  [Description]:
//      This routine set clear voice X-Surround.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetSurround(Cv3SurroundInfo_T *p_surround_info)
{
    static U32 cv3_surround_count = 0x00000000;
    U32 i;
    WORD   addr = 0;

    MAD_DEBUG_P1(printk("MDrv_MAD_CV3_SetAVL2\n"));
#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    for (i=0; i< CV3_SURROUND_PARAM_NUM; i++)  {
        addr = cv3_surround_param_base_addr + i;
        MDrv_MAD_SetCV3Para(addr, ((U32 *)p_surround_info)[i]);
    }

    addr = cv3_surround_param_base_addr-1;
    cv3_surround_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_surround_count);
#endif
}


//******************************************************************************
//  [Function Name]:    MDrv_MAD_SetClearVoiceIII
//  [Description]:      This routine set general CV3 param setting like mode, SFREQ.
//  [Arguments]:        None
//*******************************************************************************
#define  cv3_general_setting_param_base_addr 0x3CAF
#define CV3_GENERAL_SETTING_PARAM_NUM 15
static U32 cv3_general_setting_count = 0x00000000;
void  MDrv_MAD_SetClearVoiceIII(Cv3info_T *p_cv_info)
{
    U32 i;
    WORD   addr = 0;

#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    for (i=0; i< CV3_GENERAL_SETTING_PARAM_NUM; i++)  {
        addr = cv3_general_setting_param_base_addr + i;
        MDrv_MAD_SetCV3Para(addr, ((U32 *)p_cv_info)[i]);
    }

    addr = cv3_general_setting_param_base_addr-1;
    cv3_general_setting_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_general_setting_count);
#endif
}


#define  cv3_mode_param_addr 0x3CAF
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_SetClearVoiceOnOff
//  [Description]:
//      This routine OnOff cv3's clear voice function.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_SetClearVoiceOnOff(U8 enable)
{
    WORD   addr = 0;
    U32 mode_param;

#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    mode_param = MDrv_MAD2_Read_DSP_sram(cv3_mode_param_addr, DSP_MEM_TYPE_PM);

    switch(enable)
    {
        case 0x00:
	      if((MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x1000/*BTSC*/ || (MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x2000/*PALSUM*/)//if ATV loaded
	          MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x00);//adv soundeff disable
             mode_param |=  PROC_ENABLE;
             mode_param &= ~FUNC04;
            break;

        case 0x01:
	     if((MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x1000/*BTSC*/ || (MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x2000/*PALSUM*/)//if ATV loaded
	          MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x80);//adv soundeff enable
        default:
             mode_param |=  PROC_ENABLE;
             mode_param |=  FUNC04;
            break;
    }

    MDrv_MAD_SetCV3Para(cv3_mode_param_addr, mode_param);

    addr = cv3_mode_param_addr-1;
    cv3_general_setting_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_general_setting_count);

#endif
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetAVL2OnOff
//  [Description]:
//      This routine OnOff cv3's AVL2 function.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetAVL2OnOff(U8 enable)
{
    WORD   addr = 0;
    U32 mode_param;

#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    mode_param = MDrv_MAD2_Read_DSP_sram(cv3_mode_param_addr, DSP_MEM_TYPE_PM);

    switch(enable)
    {
        case 0x00:
	      if((MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x1000/*BTSC*/ || (MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x2000/*PALSUM*/)//if ATV loaded
	          MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x00);//adv soundeff disable
             mode_param |=  PROC_ENABLE;
             mode_param &= ~FUNC01;
            break;

        case 0x01:
		if((MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x1000/*BTSC*/ || (MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x2000/*PALSUM*/)//if ATV loaded
	          MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x80);//adv soundeff enable
        default:
             mode_param |=  PROC_ENABLE;
             mode_param |=  FUNC01;
            break;
    }

    MDrv_MAD_SetCV3Para(cv3_mode_param_addr, mode_param);

    addr = cv3_mode_param_addr-1;
    cv3_general_setting_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_general_setting_count);

#endif
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetSurroundOnOff
//  [Description]:
//      This routine OnOff cv3's surround function.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetSurroundOnOff(U8 enable)
{
    WORD   addr = 0;
    U32 mode_param;

#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    mode_param = MDrv_MAD2_Read_DSP_sram(cv3_mode_param_addr, DSP_MEM_TYPE_PM);

    switch(enable)
    {
        case 0x00:
	     if((MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x1000/*BTSC*/ || (MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x2000/*PALSUM*/)//if ATV loaded
	          MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x00);//adv soundeff disable
             mode_param |=  PROC_ENABLE;
             mode_param &= ~FUNC06;
            break;

        case 0x01:
		if((MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x1000/*BTSC*/ || (MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x2000/*PALSUM*/)//if ATV loaded
	          MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x80);//adv soundeff enable
        default:
             mode_param |=  PROC_ENABLE;
             mode_param |=  FUNC06;
            break;
    }

    MDrv_MAD_SetCV3Para(cv3_mode_param_addr, mode_param);

    addr = cv3_mode_param_addr-1;
    cv3_general_setting_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_general_setting_count);

#endif
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetUEQOnOff
//  [Description]:
//      This routine OnOff cv3's ueq function.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetUEQOnOff(U8 enable)
{
    WORD   addr = 0;
    U32 mode_param;

#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    mode_param = MDrv_MAD2_Read_DSP_sram(cv3_mode_param_addr, DSP_MEM_TYPE_PM);

    switch(enable)
    {
        case 0x00:
	    if((MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x1000/*BTSC*/ || (MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x2000/*PALSUM*/)//if ATV loaded
	          MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x00);//adv soundeff disable
             mode_param |=  PROC_ENABLE;
             mode_param &= ~FUNC07;
            break;

        case 0x01:
	     if((MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x1000/*BTSC*/ || (MHal_MAD_ReadReg(0x2DFA) & 0xF000) == 0x2000/*PALSUM*/)//if ATV loaded
	          MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x80);//adv soundeff enable
        default:
             mode_param |=  PROC_ENABLE;
             mode_param |=  FUNC07;
            break;
    }

    MDrv_MAD_SetCV3Para(cv3_mode_param_addr, mode_param);

    addr = cv3_mode_param_addr-1;
    cv3_general_setting_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_general_setting_count);

#endif
}



//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_CV3_SetMode
//  [Description]:
//      This routine set clear voice mode.
//  [Arguments]:
//
//*******************************************************************************
void MDrv_MAD_CV3_SetMode(U32 cvMode)
{
    WORD   addr = 0;

#if(AUDIO_USE_SOUND_EFFECT_CV3==1)
    MDrv_MAD_SetCV3Para(cv3_mode_param_addr, cvMode);

    addr = cv3_mode_param_addr-1;
    cv3_general_setting_count++;
    MDrv_MAD_SetCV3Para(addr, cv3_general_setting_count);
#endif
}



//******************************************************************************
//  [Function Name]:    MDrv_MAD_SetCVPara
//  2   mInputGain;
//  3   mOutputGain;
//  4   mBypassGain;
//  5   mLimiterBoost;
//  6   mHardLimit;
//******************************************************************************
void MDrv_MAD_SetCV3Para(WORD addr, U32 value)
{
    MDrv_MAD2_Write_DSP_sram(addr, value, DSP_MEM_TYPE_PM);
}


//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_GetMADBase
//  [Description]:
//      This routine : Get MAD base address.
//  [Arguments]:
//      NONET
//*******************************************************************************
void MDrv_MAD_GetMADBase(MAD_BASE_INFO * mad_base)
{

    mad_base->u32_DEC_Addr = DSPMadBaseBufferAdr[DSP_DEC] ;
    mad_base->u32_SE_Addr = DSPMadBaseBufferAdr[DSP_SE] ;
    mad_base->u32_DEC_Size = MAD_MEM_DEC_SIZE;
    mad_base->u32_SE_Size = MAD_MEM_SE_SIZE ;

}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_ADEC_Alive_Check
//  [Description]:
//      This routine : check the ADEC dsp alive
//  [Arguments]:
//      NONET
//*******************************************************************************
U8 MDrv_MAD_ADEC_Alive_Check(void)
{
    U8 rtValue=0;
    U16 tmp1=0,tmp2=0;

    tmp1 = MHal_MAD_ReadReg(REG_MAD_MAIN_COUNTER);
    msleep(1);
    tmp2 = MHal_MAD_ReadReg(REG_MAD_MAIN_COUNTER);

    //printk("MDrv_MAD_ADEC_Alive_Check(0x%x,0x%x)\n",tmp1,tmp2);

    if(tmp1 != tmp2)
        rtValue=1;

    return(rtValue);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_ASND_Alive_Check
//  [Description]:
//      This routine : check the ASND dsp alive
//  [Arguments]:
//      NONET
//*******************************************************************************
U8 MDrv_MAD_ASND_Alive_Check(void)
{
    U8 rtValue=0;
    U16 tmp1=0,tmp2=0;

    tmp1 = MHal_MAD_ReadReg(REG_SOUND_TIMER_COUNTER);
    msleep(1);
    tmp2 = MHal_MAD_ReadReg(REG_SOUND_TIMER_COUNTER);

    //printk("MDrv_MAD_ASND_Alive_Check(0x%x,0x%x)\n",tmp1,tmp2);

    if(tmp1 != tmp2)
        rtValue=1;

    return(rtValue);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_ADEC_ES_Check
//  [Description]:
//      This routine : check the ES data input
//  [Arguments]:
//      NONET
//*******************************************************************************
U8 MDrv_MAD_ADEC_ES_Check(void)
{
    U8 rtValue=0;
    U16 tmp1=0;

    tmp1 = MDrv_MAD_Read_DSP_sram(0xA051, 1);
    //MAD_DEBUG_P1(printk("MDrv_MAD_ADEC_ES_Check(0x%x, 0x%x)\n",tmp1,tmp2);

    if(tmp1 != 0)
        rtValue=1;

    return(rtValue);
}


DECLARE_TASKLET(PlayAudioFile, MDrv_MAD_Clip_Play, 0);

u32 file_addr_info = 0; //Mstar add

static irqreturn_t _MAD_AUDFILE_ISR(int irq, void *dev_id)
{
    U16 dsp_command=0;

    //printk("_MAD_AUDFILE_ISR\n");
    dsp_command = MHal_MAD_ReadReg(REG_D2M_MAILBOX_DEC_ISRCMD);//CHECK, TODO

    if(dsp_command == 0x03) // File format command, e.g. MP3 files
    {
        if(clipPlaying == 0)
        {

            get_file_info.wAddress= ES_ADDR + MHal_MAD_ReadReg(0x2D6A)*16;
            get_file_info.wNumber= MHal_MAD_ReadReg(0x2D6C)*16;

            MDrv_Event_to_AP(MAD_EVENT_FILE_FORMAT);

            //MDrv_MAD_FileSetInput(1);

        }
        else // playing clip
            tasklet_schedule(&PlayAudioFile);
    }
    else if(dsp_command == 0x01)
    {
        MHal_MAD_WriteMaskReg(REG_DEC_DECODE_CMD,0x0001,0x0000);
    }
    else if(dsp_command == 0x02)
    {
        MHal_MAD_WriteMaskReg(REG_DEC_DECODE_CMD,0x0001,0x0001);
    }
    //Mstar modify for Skype, 2009/10/12
    else if(dsp_command == 0x06)
    {
        MDrv_MAD_Skype_Dn_PCM();    //copy Skype PCM data to DSP
    }
    else if(dsp_command == 0x07)
    {
        MDrv_MAD_Skype_Up_PCM();    //copy Skype PCM data from DSP
    }
    else if(dsp_command == 0x08)
    {
        MDrv_MAD_Skype_Up_BS();     //copy Skype Bit Stream data from DSP
    }
    else if(dsp_command == 0x09)
    {
        MDrv_MAD_Skype_Dn_BS();    //copy Skype Bit Stream data to DSP
    }
    else if(dsp_command == 0x13)
    {
        MDrv_MAD_SetEncodeDoneFlag(1);
        EncBuf_Count++;                  // increase frame buffer count
        EncFrameIdx += 16;              //increase frame index
        MPEG_EN_BUF[EncBuf_W_idx].Frame_PTS = (Audio_U64)(((EncFrameIdx-1)*1152*90)/48);
        MPEG_EN_BUF[EncBuf_W_idx].Frame_Addr = (U32)DSPMadBaseBufferAdr[DSP_DEC] +  (U32)MHal_MAD_ReadReg(REG_D2M_MAILBOX_ENC_LineAddr)*16*16; // line_addr -> byte addr
        MPEG_EN_BUF[EncBuf_W_idx].Frame_Size =  (U32)MHal_MAD_ReadReg(REG_D2M_MAILBOX_ENC_LineSize)*16;

        if(EncBuf_W_idx == 5)        //buffer size 6, index range 0 ~ 5
            EncBuf_W_idx = 0;
        else
            EncBuf_W_idx++;

        //printk("\r\n=== Encode_ISR_DSP2MCU ====== : %d  \r\n", EncBuf_W_idx);
    }
    //else if(dsp_command == 0x05)
    //MHal_MAD_WriteMaskReg(0x2B04,0x2000,0x2000);//CHECK //Allan
    //MHal_MAD_WriteMaskReg(0x2B04,0x2000,0x0000);//CHECK

    return IRQ_HANDLED;
}

// Samuel, 090108
#if 1
static void dma_ch1_memcpy( void* in_dst_addr, void* in_src_addr, u32 size  ){
//    u32 timeout ;
    u32 dst_addr, src_addr ;

    dst_addr = (u32)in_dst_addr ;
    src_addr = (u32)in_src_addr ;

    // screen out illegal dma process
    if( (dst_addr&0xA0000000)!=0xA0000000 || (src_addr&0xA0000000)!=0xA0000000 || size<=256 ){
        memcpy( (void*)dst_addr, (void*)src_addr, size ) ;
        return ;
    }

    // get phy address
    dst_addr &= 0x0FFFFFFF ;
    src_addr &= 0x0FFFFFFF ;

    // src address
    if( src_addr>=0x08000000 ){ // miu0/1 selection
        *(volatile unsigned int *)(0xbf001200+(0x20<<2)) |= (1<<8) ;
        src_addr -= 0x08000000 ;
    }else
        *(volatile unsigned int *)(0xbf001200+(0x20<<2)) &= ~(1<<8) ;
    *(volatile unsigned int *)(0xbf001200+(0x25<<2)) = (src_addr>>16) ;
    *(volatile unsigned int *)(0xbf001200+(0x24<<2)) = (src_addr&0xFFFF) ;

    // dist address
    if( dst_addr>=0x08000000 ){ // miu0/1 selection
        *(volatile unsigned int *)(0xbf001200+(0x20<<2)) |= (1<<10) ;
        dst_addr -= 0x08000000 ;
    }else
        *(volatile unsigned int *)(0xbf001200+(0x20<<2)) &= ~(1<<10) ;
    *(volatile unsigned int *)(0xbf001200+(0x27<<2)) = (dst_addr>>16) ;
    *(volatile unsigned int *)(0xbf001200+(0x26<<2)) = (dst_addr&0xFFFF) ;

    // size
    *(volatile unsigned int *)(0xbf001200+(0x23<<2)) = (size>>16) ;
    *(volatile unsigned int *)(0xbf001200+(0x22<<2)) = (size&0xFFFF) ;

    // trigger DMA
    *(volatile unsigned int *)(0xbf001200+(0x20<<2)) |= 1 ;

#if 0
    // busy wait for DMA done
    timeout = 0 ;
    while(1){
        if( 0x4==(*(volatile unsigned int *)(0xbf001200+(0x21<<2))&0x7) ){
            *(volatile unsigned int *)(0xbf001200+(0x21<<2)) |= 0x4 ;
            break ;
        }
        ndelay(1) ;
        timeout++ ;
        if( timeout>=0xFFFFFF ){
            printk( "\nBT ByteDMA timeout!!\n" ) ;
            break ;
        }
    }
#endif

}
#endif

// This ISR only for BT function.
static irqreturn_t _MAD_AUD_ISR2(int irq, void *dev_id)
{

  U16 SBC_len;
    U8 dsp_command=0;

    U8* dst_addr = (U8 *)SE_ES_ADDR+0xF8000;
    U8* src_addr = BT_information.pBufAddr;
    U32 bufsize =  _BT_BufferSize/2;

  static U32 counter_intr=0;
    //static U32 isrcnt=0;

  //printk(KERN_EMERG "_SE_ISR2 Count[%d]....\n", isrcnt++);

  dsp_command = MHal_MAD_ReadByte(REG_SE_INT_ID);

    if(dsp_command == 0x03)
    {
      get_file_info_se.wAddress= SE_ES_ADDR + MDrv_MAD_ReadMailBox(DSP_SE, 5)*16;
      get_file_info_se.wNumber= MDrv_MAD_ReadMailBox(DSP_SE, 6)*16;

      MDrv_Event_to_AP(MAD2_EVENT_FILE_FORMAT);
    }
  else
    {

    if((_BT_RunningUp == 1)||(_BT_RunningDown == 1))  //check if in BT usage.
      dsp_command = MHal_MAD_ReadByte(REG_SE_INT_ID);
    else
        return IRQ_HANDLED;

    if(BT_information.pBufAddr != NULL)
    {
        switch(dsp_command)
        {
            case 0xB0:// BT upload command buffer 0
                //printk("BT_ISR_up 0....\n");
                dma_ch1_memcpy((void *)BT_information.pBufAddr, (void *)(SE_ES_ADDR+0xF8000),_BT_BufferSize);
                MDrv_Event_to_AP(MAD_EVENT_BT);
                break;
            case 0xB1:// BT upload command buffer 1
                //printk("BT_ISR_up 1....\n");
                dma_ch1_memcpy((void *)BT_information.pBufAddr, (void *)(SE_ES_ADDR+0xF8000+_BT_BufferSize),_BT_BufferSize);
                MDrv_Event_to_AP(MAD_EVENT_BT);
                break;
	     case 0xC0://BT download command with PCM
	         //printk(KERN_EMERG "ISR2 D_PCM Cnt[%d]....\n", isrcnt++);
		  if((MHal_MAD_ReadByte(0x2BA6) & 0x08) == 0x08)//over run, don't overwrite
		  {
	             DBG_AUDIO_ERROR(printk("	====== Over run, return ======\r\n"));
		      return IRQ_HANDLED;
		  }
		  else if((MHal_MAD_ReadByte(0x2BA6) & 0x04) == 0x04)
            {
		      DBG_AUDIO_ERROR(printk("	====== Under run, return ======\r\n"));
		  }
		  dst_addr++;
                //printk("BT_ISR_down 0....\n");
		  while(bufsize != 0)
                {
		      if((counter_intr % 2) == 0)
		          memcpy((void *)(dst_addr),(void *)src_addr,2);
		      else
			  memcpy((void *)(dst_addr+_BT_BufferSize),(void *)src_addr,2);
		      dst_addr += 4;
		      src_addr += 2;
		      bufsize -= 2;
                }
		  Chip_Flush_Memory();
		  counter_intr++;
		  MHal_MAD_WriteMaskReg(0x2B82, 0xFFFF, BufferLineSize);	//after copy data, need to update DMA READER Length.
                MDrv_Event_to_AP(MAD_EVENT_BT);
                break;
            case 0xBF:// BT upload command with SBC
                Chip_Read_Memory();
                SBC_len = MHal_MAD_ReadByte(0x2DF0)*15;
                //printk("SBC BT_ISR_up, SBC_len[%d] ....\n", SBC_len);
                if((_BT_Count&1)==0)
                {
                    //dma_ch1_memcpy((void *)BT_information.pBufAddr, (void *)(SE_ES_ADDR+0xF8000),SBC_len);//encoded data size now is fixed for 0x474, it will be varied size in next release
                    memcpy((void *)BT_information.pBufAddr, (void *)(SE_ES_ADDR+0xF8000),SBC_len);
                //                  MDrv_GPIO_Set_High(87);
                }
                else
                {
                    //dma_ch1_memcpy((void *)BT_information.pBufAddr, (void *)(SE_ES_ADDR+0xF9000),SBC_len);
			memcpy((void *)BT_information.pBufAddr, (void *)(SE_ES_ADDR+0xF9000),SBC_len);
                //              MDrv_GPIO_Set_Low(87);
                }
                Chip_Flush_Memory();
                _BT_Count++;
                MDrv_Event_to_AP(MAD_EVENT_BT);
                break;
            case 0xCF:// BT download command with SBC
                SBC_len = MHal_MAD_ReadByte(0x2DF0)*15;
                //printk("SBC BT_ISR_down, SBC_len[%d] ....\n", SBC_len);
                SBC_len = BT_information.bufSize;

                if((_BT_Count%2)==0)
                {
                    memcpy((void *)(SE_ES_ADDR+0xE8000),(void *)BT_information.pBufAddr,SBC_len);
                //          MDrv_GPIO_Set_High(87);
                }
                else
                {
                    memcpy( (void *)(SE_ES_ADDR+0xF9000),(void *)(BT_information.pBufAddr),SBC_len);
                //          MDrv_GPIO_Set_Low(87);
                }
                _BT_Count++;
                MDrv_Event_to_AP(MAD_EVENT_BT);
                break;
            default:

                break;
        }
    }
    else
        DBG_AUDIO_ERROR(printk("\n >>>>>>>>>>>>>>>>>>>>>>> ISR get NULL, size is %d  <<<<<<<<<<<<<<<<<<<<\n", _BT_BufferSize));
  }
    return IRQ_HANDLED;
}

//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_FileISRInit
//  [Description]:
//      This routine is the initialization for Audio clip.
//  [Arguments]:
//      None
//  [Return]:
//      None
//*******************************************************************************
static unsigned int u32MADIntRegister;
static unsigned int u32DSP2_TO_MIPSIntRegister;

void MDrv_MAD_ISRInit(void)
{
    int result=0,result2=0;

    //printk("MDrv_MAD_ISRInit(%d)...\n",E_FIQ_DSP2UP);

    //MHal_MAD_AbsWriteMaskByte(0x101903,0x20,0x00);
	if(0 == u32MADIntRegister) {

    result = request_irq(E_FIQ_DSP2UP, (void *)_MAD_AUDFILE_ISR, SA_INTERRUPT, "MAD_ISR_Int", NULL);

	    if(result) {
        DBG_AUDIO_ERROR(printk("_MAD_AUDFILE_ISR can't get assigned irq !!\n"));
			//return -EBUSY;
	    }
		u32MADIntRegister = 1;
	} else {
		disable_irq(E_FIQ_DSP2UP);
		enable_irq(E_FIQ_DSP2UP);
	}
    //printk("MDrv_MAD_ISR2Init(%d)...\n",E_FIQ_DSP2_TO_MIPS);

    //MHal_MAD_AbsWriteMaskByte(0x101901,0xff,0x00);
    //MHal_MAD_AbsWriteMaskByte(0x101901,0x40,0x00);
	if(0 == u32DSP2_TO_MIPSIntRegister) {

    result2 = request_irq(E_FIQ_DSP2_TO_MIPS, (void *)_MAD_AUD_ISR2, SA_INTERRUPT, "MAD_ISR2_Int", NULL);

	    if(result2) {
        DBG_AUDIO_ERROR(printk("_MAD_AUD_ISR2 can't get assigned irq !!\n"));
			//return -EBUSY;
		}
		u32DSP2_TO_MIPSIntRegister = 1;
	} else {
		disable_irq(E_FIQ_DSP2_TO_MIPS);
		enable_irq(E_FIQ_DSP2_TO_MIPS);
	}
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_BT_SetBufferCounter
//  [Description]:
//      Set the DDR buffer according the sampling rate and the frame time
//      ex: if the sampling rate is 48KHz, the frame time is 40ms
//            ==> the frame buffer size is 48000*0x04*2 (L/R) *2(Bytes/sample) = 0x1E00
//                    the counter is 0x1E00/8 = 960 (For 1*Burst DMA)
//  [Arguments]:
//      counter:
//  [Return]:
//      None
//
//*******************************************************************************
void MDrv_MAD_BT_SetBufferCounter(U32 u32Counter)
{
    u32Counter &= 0x00FFFFFF;
    MDrv_MAD2_Write_DSP_sram(0x04FF, u32Counter, DSP_MEM_TYPE_PM);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_BT_Upload_Samprate_Set(BOOL enable,U8 Samprate)
//  [Description]:
//      According Blue tooth upload path, for different sampling rate setting the Synthesizer.
//  [Arguments]:
//      Samprate: 0,no change
//                1, 48KHz
//                  2, 44KHz
//                  3, 8KHz
//  [Return]:
//      None
//
//*******************************************************************************
void MDrv_MAD_BT_Upload_Samprate_Set(BOOL enable,U8 Samprate)
{
#if 0 //CHECK
    Samprate &= 0x0003;

    if(enable){
       MHal_MAD_WriteReg(0x2C5A, 0x55 );
       MHal_MAD_WriteMaskReg(0x2D6C,0x10,0x10 );
       MHal_MAD_WriteMaskReg(0x2D34,0x03,Samprate );
    }else{
       MHal_MAD_WriteReg(0x2C5A, 0 );
       MHal_MAD_WriteMaskReg(0x2D6C,0x10,0 );
       MHal_MAD_WriteMaskReg(0x2D34,0x03,0 );
    }
#endif
}

//*************************************************************************
//Function name:    MDrv_MAD_HDMI_NONPCM_FLAG
//Description:      This function report HDMI non-PCM or PCM format (0x2C0C)
//  [doxygen]
/// This function report HDMI non-PCM or PCM format (0x2C0C), (0x2D4C)
/// @return - non-PCM --> 0x2C0C[7:6]= 0x1 , 0x2D4C[7:0] = 0x30
///         - PCM     --> 0x2C0C[7:6]= 0x0 , 0x2D4C[7:0] = 0x00
//*************************************************************************
BOOL MDrv_MAD_HDMI_NONPCM_FLAG(void)
{
    if ((MDrv_MAD_HDMI_Monitor() & 0xC0) == 0x40 || MDrv_MAD_HDMI_Monitor2() == 0x30)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_DTV_HDMI_CFG
//  [Description]:
//      This routine : Set AC3 NOT DECODE
//  [Arguments]:
//      DTV : ctrl = 0, HDMI : ctrl = 1
//*******************************************************************************

void MDrv_MAD_DTV_HDMI_CFG(U8 u8Ctrl)
{
    if (u8Ctrl == 1)
    {   // HDMI
        /* use CH1 decode HDMI AC3 */
        MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG,0x07,0x04);        // HDMI_AC3 REG CFG
    }
    else
    {   // DTV
        MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG,0x07,0x00);        // DVB1 REG CFG
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_GetFileInfo
//  [Description]:
//      This routine get file address and size that DEC-DSP request for file format.
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_GetFileInfo(MEDIA_FILE_INFO_T *gFile)
{
    gFile->wAddress = get_file_info.wAddress ;
    gFile->wNumber= get_file_info.wNumber ;
    //printk("MDrv_MAD_GetFileInfo(0x%x,0x%x)\n",gFile->wAddress,gFile->wNumber);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_GetFileInfo_SE
//  [Description]:
//      This routine get file address and size that SE-DSP request for file format.
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_GetFileInfo_SE(MEDIA_FILE_INFO_T *gFile)
{
    gFile->wAddress = get_file_info_se.wAddress ;
    gFile->wNumber= get_file_info_se.wNumber ;
    MAD_DEBUG_P1(printk("MDrv_MAD_GetFileInfo_SE(address = 0x%x,number =0x%x)\n",gFile->wAddress,gFile->wNumber));
}


//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_GetAudioStreamInfo
//  [Description]:
//      This routine get file address and size that DSP request for file format.
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_GetAudioStreamInfo(MEDIA_AUDIOSTREAM_INFO_T *pAudioStream)
{
    U8 ch = pAudioStream->ch;
    pAudioStream->wAddress[ch] = get_audiostream_info.wAddress[ch] ;
    pAudioStream->wNumber[ch] = get_audiostream_info.wNumber[ch] ;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_FileSetInput
//  [Description]:
//      API for start MP3 data transfer and decode
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_FileSetInput(U32 tTag, U32 tAddress)
{
    //printk("tag: %d, dst addr: 0x%08x, src addr: 0x%08x, size: %d\n", tTag ,get_file_info.wAddress, tAddress, get_file_info.wNumber);

    memcpy((void *)get_file_info.wAddress,(void *)tAddress,get_file_info.wNumber);

    Chip_Flush_Memory();

    MDrv_MAD_WriteMailBox(DSP_DEC, 6, tTag);

    MDrv_MAD_SendIntrupt(DSP_DEC,0xE0);

}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_FileSetInput_SE
//  [Description]:
//      API for start MP3 data transfer and decode
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_FileSetInput_SE(U32 tTag, U32 tAddress)
{
    //printk("tag: %d, dst addr: 0x%08x, src addr: 0x%08x, size: %d\n", tTag ,get_file_info_se.wAddress, tAddress, get_file_info_se.wNumber);

    memcpy((void *)get_file_info_se.wAddress,(void *)tAddress,get_file_info_se.wNumber);

    Chip_Flush_Memory();

    MDrv_MAD_WriteMailBox(DSP_SE, 6, tTag);

    MDrv_MAD_SendIntrupt(DSP_SE,0xE0);

}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_SetAudioStreamInput
//  [Description]:
//      API for start AudioStream data transfer and decode
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_SetAudioStreamInput(U8 ch, U32 tTag)
{
    //printk("AudioStream Buffer addr: 0x%08x, src addr: 0x%08x, size: %d\n",get_audiostream_info[ch].wAddress, tAddress, get_audiostream_info[ch].wNumber);
    if(ch == CH_A)
    {
        MDrv_MAD_WriteMailBox(DSP_DEC, 6, tTag);
    	 MDrv_MAD_TriggerPIO8();
    }
    else if(ch == CH_B)
    {
        MDrv_MAD_WriteMailBox(DSP_SE, 6, tTag);
    	 MDrv_MAD2_TriggerPIO8();
    }
}

//******************************************************************************
//  [Function Name]:
//      msAPI_Mp3_FileEndNotification
//  [Description]:
//      API for notify dsp stop transfer and decode
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_FileEndNotification(U32 tTag)
{
    MDrv_MAD_WriteMailBox(DSP_DEC, 6, tTag);

    MDrv_MAD_SendIntrupt(DSP_DEC,0xE0);

    MAD_DEBUG_P1(printk("MDrv_MAD_FileEndNotification() is called ...\n"));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_FileEndNotification_SE
//  [Description]:
//      API for notify dsp stop transfer and decode
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_FileEndNotification_SE(U32 tTag)
{
    MDrv_MAD_WriteMailBox(DSP_SE, 6, tTag);

    MDrv_MAD_SendIntrupt(DSP_SE,0xE0);

    MAD_DEBUG_P1(printk("MDrv_MAD_FileEndNotification_SE() is called ...\n"));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_FileSetInput
//  [Description]:
//      API for start MP3 data transfer and decode
//  [Arguments]:
//      None
//*******************************************************************************
U32 MDrv_MAD_Check_Copy_Rqt(void)
{
   return(_copyRequest) ;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Monitor_DDPlus_SPDIF_Rate( void )
//  [Description]:
//      This routine Monitor and Set SPDIF nonPCM rate.
//  [Arguments]:
//*******************************************************************************
void MDrv_MAD_Monitor_DDPlus_SPDIF_Rate(void)
{
        // confirm DSP is DD+, and SPDIF is nonPCM mode
        if (((MHal_MAD_ReadByte(REG_MB_DEC_ID_STATUS+1)&0xF0) == 0x70)&&(MHal_MAD_ReadByte(REG_M2D_MAILBOX_SPDIF_CTRL)&0x02))
        {
            switch(MHal_MAD_ReadByte(REG_MB_AC3P_SMPRATE)&0x3)
            {
                case 1:     //44.1Khz
                    if(MHal_MAD_ReadReg(REG_SPDIF_NPCM_SYNTH_NF_H) != 0x1321 )
                    {
                        //printf("DD+ nonPCM 44.1K\n");
                        MHal_MAD_WriteReg(REG_SPDIF_NPCM_SYNTH_NF_H, 0x1321 );
                        MHal_MAD_WriteReg(REG_SPDIF_NPCM_SYNTH_NF_L, 0xF58D );
                        MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CS3, 0xC0, 0x00);   //change SPDIF channel status --> 44.1K
                    }
                    break;

                case 2:     //32Khz
                    if(MHal_MAD_ReadReg(REG_SPDIF_NPCM_SYNTH_NF_H) != 0x1A5E )
                    {
                        //printf("DD+ nonPCM 32K\n");
                        MHal_MAD_WriteReg(REG_SPDIF_NPCM_SYNTH_NF_H, 0x1A5E );
                        MHal_MAD_WriteReg(REG_SPDIF_NPCM_SYNTH_NF_L, 0x0000 );
                        MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CS3, 0xC0, 0xC0);   //change SPDIF channel status --> 32K
                    }
                    break;

                default:
                    if(MHal_MAD_ReadReg(REG_SPDIF_NPCM_SYNTH_NF_H) != 0x1194 )
                    {
                        //printf("DD+ nonPCM 48K\n");
                        MHal_MAD_WriteReg(REG_SPDIF_NPCM_SYNTH_NF_H, 0x1194 );
                        MHal_MAD_WriteReg(REG_SPDIF_NPCM_SYNTH_NF_L, 0x0000);
                        MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CS3, 0xC0, 0x40);   //change SPDIF channel status --> 48K
                    }
                    break;
            }
    }
    else if(((MHal_MAD_ReadByte(REG_MB_DEC_ID_STATUS+1)&0xF0) == 0x50)&&(MHal_MAD_ReadByte(REG_M2D_MAILBOX_SPDIF_CTRL)&0x02))
    {
        if(MDrv_MAD_Read_DSP_sram(0xFF7, DSP_MEM_TYPE_PM) == 44100)    // check if 44.1KHz
        {
            // if is AAC and is SPDIF nonPCM out and sample rate is 44.1KHz, set SPDIF out as PCM
            MHal_MAD_WriteMaskByte(REG_M2D_MAILBOX_SPDIF_CTRL, 0x2, 0x0);          /* PCM Mode */
            MHal_MAD_WriteMaskByte(REG_AUDIO_SPDIF_OUT_CFG, 0x07, 0x00);
            MDrv_MAD_SPDIF_SetChannelStatus(SPDIF_CHANNEL_STATUS_PCM_FORMAT, SPDIF_CS_FORMAT_PCM);
        }
    }
    else
    {
        if(MHal_MAD_ReadReg(REG_SPDIF_NPCM_SYNTH_NF_H) != 0x1194 )
        {
                MHal_MAD_WriteReg(REG_SPDIF_NPCM_SYNTH_NF_H, 0x1194 );
                MHal_MAD_WriteReg(REG_SPDIF_NPCM_SYNTH_NF_L, 0x0000);
        }
    }
}


//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_SetHPOutputType(BYTE ch, BYTE value)
//  [Description]:
//      This routine sets the automute function of HDMI.
//  [Arguments]:
//      ch : 0~3
//      value: register value
//*******************************************************************************
void MDrv_MAD_SetHPOutputType(U8 outputMode)//CHECK
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SetHPOutputType(%d)\n",outputMode));

    if(outputMode == ADECP_HP_SRC_BEFORE_PP)
        MDrv_MAD_SetInternalPath(INTERNAL_PCM_DELAY, AudioOutputInfo.HpOut);    //do software bypass
    else if(outputMode == ADECP_HP_SRC_AFTER_PP)
        MDrv_MAD_SetInternalPath(INTERNAL_PCM_SE, AudioOutputInfo.HpOut);     //disable software bypass
    else
        DBG_AUDIO_ERROR(printk("invalid head phone source type !!\n"));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_HDMIAutoMute(BYTE ch, BYTE value)
//  [Description]:
//      This routine sets the automute function of HDMI.
//  [Arguments]:
//      ch : 0~5
//      value: register value
//*******************************************************************************
void MDrv_MAD_HDMIAutoMute(U8 ch, U8 value)
{
    if(ch < 4)
    {
        MHal_MAD_WriteByte(REG_AUDIO_MUTE_CTRL1+(ch<<1),value);
    }
    else if(ch == 4)
    {
        MHal_MAD_WriteByte(REG_AUDIO_MUTE_CTRL1+1,value);
    }
    else if(ch == 5)
    {
      MHal_MAD_WriteByte(REG_AUDIO_MUTE_CTRL2+1,value);
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_AmplifierOn()
//  [Description]:
//      This routine sets the amplifier on.
//  [Arguments]:
//     //     None
//*******************************************************************************
void MDrv_MAD_AmplifierOn(void)//CHECK
{
    //printk("MDrv_MAD_AmplifierOn....\n");
    MDrv_MAD_ProcessSetMute(4,1);
    //Audio_Amplifier_ON();
    MDrv_MAD_ProcessSetMute(4,0);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_AmplifierOff()
//  [Description]:
//      This routine sets the amplifier off.
//  [Arguments]:
//     //     None
//*******************************************************************************
void MDrv_MAD_AmplifierOff(void)//CHECK
{
    //printk("MDrv_MAD_AmplifierOn....\n");
    //Audio_Amplifier_OFF();
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_GetDSPClk()
//  [Description]:
//      This routine dsp clock frequency
//  [Arguments]:
//     //     None
//*******************************************************************************
U32 MDrv_MAD_GetDSPClk(void)
{
    return(DSP_CLK);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSetEq
//  [Description]:
//      This routine is to set the equalizer for processor module.
//  [Arguments]:
//      band:   0~4 (for select process band)
//      level:  level value
//  [Return]:
//      None
//  [Doxygen]
// This routine is to set the equalizer for processor module.
// @param band \b IN: EQ band
// @param level \b IN: gain level
//
//*******************************************************************************
void MDrv_MAD_ProcessSetEq(MS_U8 u8Band, MS_U8 u8Level)
{
   MS_U8 value;

    if( u8Band>4)
        return;

    MAD_DEBUG_P1(printk("MDrv_MAD_ProcessSetEq(0x%x)(0x%x)\n",u8Band,u8Level));
    if(u8Level==100)
      value = 0x30;
    else
    value = ((int)u8Level-50)*48/50;

    MHal_MAD_WriteByte(REG_SOUND_EQ1 + (u8Band*2), u8Level);
}
//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSetEq7
//  [Description]:
//      This routine is to set the equalizer for processor module.
//  [Arguments]:
//      band:   0~6 (for select process band)
//      level:  level value
//  [Return]:
//      None
//  [Doxygen]
// This routine is to set the equalizer for processor module.
// @param band \b IN: EQ band
// @param level \b IN: gain level
//
//*******************************************************************************
void MDrv_MAD_ProcessSetEq7(MS_U8 u8Band, MS_U8 u8Level)
{
    if(u8Band>5)
      return;

    MHal_MAD_WriteByte(REG_SOUND_EQ_BASE + (u8Band*2), u8Level);
}

U8 MDrv_MAD_GetDSPCodeType()
{
    return (_u8DspCodeType);
}

void MDrv_MAD_SetDSPCodeType(U8 setDspCdoeType)
{
    _u8DspCodeType = setDspCdoeType;
}

U8 MDrv_MAD_GetDSP2CodeType()
{
    return (_u8Dsp2CodeType);
}

void MDrv_MAD_SetDSP2CodeType(U8 setDsp2CdoeType)
{
    _u8Dsp2CodeType = setDsp2CdoeType;
}

U8 MDrv_MAD_GetHDMIAudioReceive(void)
{
    //To Do Get_HDMI_Audio function
    return 1;
}

U8 MDrv_MAD_SetAudioPLLSFreq(U32 SamplingFreq)//CATHY
{
#if 0//CHECK
    U32 tmpFreq=0;

    MAD_DEBUG_DDI(printk("[DDI]SetAudioPLLSFreq:(0x%x)\r\n",SamplingFreq));

    switch(SamplingFreq)
    {
        case 32000:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x34BC):(0x34BC *2));
            break;
        case 44100:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x2643):(0x2643 *2));
            break;
        case 48000:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x2329):(0x2329 *2));//change 0x2328 to 0x2329
            break;
        case 88200:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x1321):(0x1321 *2));
            break;
        case 96000:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x1194):(0x1194 *2));
            break;
        case 176400:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x0990):(0x0990 *2));
            break;
        case 192000:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x08CA):(0x08CA *2));
            break;
            //See IEC60958-3, page 12
        case 22050:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x4C86):(0x4C86 *2));
            break;
        case 24000:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x4650):(0x4650 *2));
            break;
        case 8000:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0xD2F0):(0xD2F0 *2));
            break;
        case 768000:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x0232):(0x0232 *2));
            break;
        case 0:
        default:
            tmpFreq = ((MHal_MAD_ReadReg(REG_AUDIO_CLK_CFG0)&0x0200)?(0x2329):(0x2329 *2));//change 0x2328 to 0x2329
            break;
    }
    //MHal_MAD_WriteReg(DSP_CODEC_SYNTH, tmpFreq);//CHECK
#endif
    return 0;
}


//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_LoadAudioClip()
//  [Description]:
//      This routine load audio clip
//  [Arguments]:
//     //     None
//*******************************************************************************
U32 MDrv_MAD_LoadAudioClip(U32 bufSize, void *clipBufPtr)
{

    //printk("MDrv_MAD_LoadAudioClip(%d,0x%x)\n",bufSize,clipBufPtr);

    audioFileParams.u32AudioLength= bufSize;
    audioFileParams.pAudioAddr= clipBufPtr;

    return 0;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_PlayAudioClip
//  [Description]:
//      This function start playing of the audio clip.
//  [Arguments]:
//      repeatNum: repeat number of audio clip play.
//*******************************************************************************
void MDrv_MAD_PlayAudioClip(U32 repeatNum)
{

    //printk("MDrv_MAD_PlayAudioClip(%d)\n",repeatNum);


    if( repeatNum != 0 )
    {
        audioFileParams.u32AudioLoopCounts = repeatNum;
        audioFileParams.u8AudioIsInfinite = 0;
    }
    else
    {
        audioFileParams.u32AudioLoopCounts = 0;
        audioFileParams.u8AudioIsInfinite = 1;
    }

    audioFileParams.u32AudioFileIndex = 0;

    MHal_MAD_WriteMaskReg(REG_SOUND_AUOUT0_VOL_FRAC,0x007f,0x0010);

    clipPlaying = 1;
    MDrv_MAD_Dvb_setDecCmd(AU_DVB_DECCMD_PLAYFILE);
    MDrv_MAD_SetInputPath(AUDIO_DSP1_INPUT, 4);

}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_PauseAudioClip
//  [Description]:
//      This function pauses the playing of the audio clip.
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_PauseAudioClip(void)
{
    //20090205, modify for MHEG5 hanged up issue
    //MDrv_MAD_Dvb_setDecCmd(AU_DVB_DECCMD_PAUSE);
    MHal_MAD_WriteMaskReg(REG_DEC_DECODE_CMD, 0x001F, (U16)AU_DVB_DECCMD_PAUSE );
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_ResumeAudioClip
//  [Description]:
//      This function resumes the paused audio clip.
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_ResumeAudioClip(void)
{
    MDrv_MAD_Dvb_setDecCmd(AU_DVB_DECCMD_PLAYFILE);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_StopAudioClip
//  [Description]:
//      This function stop playing of the audio clip.
//  [Arguments]:
//      None
//*******************************************************************************
void MDrv_MAD_StopAudioClip(void)
{
    //20090205, modify for MHEG5 hanged up issue
    //MDrv_MAD_Dvb_setDecCmd(AU_DVB_DECCMD_STOP);
    MHal_MAD_DecWriteRegMask(REG_DEC_DECODE_CMD, 0x001F, (U16)AU_DVB_DECCMD_STOP );
    clipPlaying = 0;
    _clipWriteLastAddr = 0;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_PCMStartUpload
//  [Description]:
//      This routine : start uploading PCM data for Bluetooth headset
//  [Arguments]:
//      MDRV_BT_T
//*******************************************************************************
void MDrv_MAD_PCMStartUpload(MDRV_BT_T *infoBT)
{
    U32 tSampleRate=0, tCounter=0;
    U16 tSampRateDSP=0;

#if 0
    printk("pBufAddr = 0x%x\n",(void *)infoBT->pBufAddr);
    printk("totalBufSize = 0x%x\n",infoBT->totalBufSize);
    printk("bufSize = 0x%x\n",infoBT->bufSize);
    printk("frameRate = %d\n",infoBT->frameRate);
    printk("sampleRate = %d\n",infoBT->sampleRate);
    printk("channelMode = %d\n",infoBT->channelMode);
    printk("blockLength = %d\n",infoBT->blockLength);
    printk("subBands = %d\n",infoBT->subBands);
    printk("allocationMethod = %d\n",infoBT->allocationMethod);
    printk("minBitpool = %d\n",infoBT->minBitpool);
    printk("maxBitpool = %d\n",infoBT->maxBitpool);
    printk("bitpool = %d\n",infoBT->bitpool);
#endif
#if 1 //By YWJung LGE 2008.08.13
   BT_information.pBufAddr = (void *) CKSEG1ADDR(infoBT->pBufAddr);
#else
   (void *) BT_information.pBufAddr = (void *)infoBT->pBufAddr;
#endif

    MAD_DEBUG_P1(printk("====== %s is called, BT Bufaddr = 0x%x ======\r\n", __FUNCTION__, (U32)BT_information.pBufAddr));

    BT_information.totalBufSize = infoBT->totalBufSize;
    BT_information.bufSize =infoBT->bufSize;
    BT_information.frameRate = infoBT->frameRate;
    BT_information.sampleRate = infoBT->sampleRate;
    BT_information.bSBCOnOff = infoBT->bSBCOnOff;
    BT_information.channelMode = infoBT->channelMode;
    BT_information.blockLength = infoBT->blockLength;
    BT_information.subBands = infoBT->subBands;
    BT_information.allocationMethod = infoBT->allocationMethod;
    BT_information.minBitpool = infoBT->minBitpool;
    BT_information.maxBitpool = infoBT->maxBitpool;
    BT_information.bitpool = infoBT->bitpool;

#if 1 //=============== if LG M/W net ready, please chang to #if 0 for mstar SBC test ======================
    if(BT_information.bSBCOnOff == 1)
    {

     MDrv_MAD2_ReLoadCode(AU_DVB2_ADVSND_SBC);//Reload SBC
     if(BT_information.channelMode == MDRV_MONO)
            MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x08,0x00);
    else if(BT_information.channelMode == MDRV_STEREO)
            MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x08,0x08);

    if(BT_information.blockLength == MDRV_BLOCK_4)
            MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x30,0x00);
    else if(BT_information.blockLength == MDRV_BLOCK_8)
            MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x30,0x10);
    else if(BT_information.blockLength == MDRV_BLOCK_12)
            MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x30,0x20);
    else if(BT_information.blockLength == MDRV_BLOCK_16)
            MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x30,0x30);

        MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x80);
        MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x03);  // start upload BT with SBC

        MHal_MAD_WriteMaskByte(0x2D4E,0xFF,32);

        MDrv_MAD_BTUpload_Samprate_Set(1, 1);// Set Sample rate to 48khz

        _BT_RunningUp = 1;

        _BT_Count = 0;
	return;
    }
#else //=============== For mstar SBC test ======================

	 MHal_MAD_WriteMaskByte(REG_ADV_SNDEFF,0x80,0x80);
	 //MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x08,0x08);
	 MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x03);  // start upload BT with SBC

        MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x08,0x00);
        MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x30,0x30);
    MHal_MAD_WriteMaskByte(0x2D4E,0xFF,32);

    // Set Sample rate to 48khz
    MDrv_MAD_BTUpload_Samprate_Set(1, 1);

    	MDrv_MAD2_ReLoadCode(AU_DVB2_ADVSND_SBC);

        _BT_RunningUp = 1;

        _BT_Count = 0;

	 return;
#endif


    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x01);  // start upload BT with PCM

    // 1. Stop Bluetooth first.
    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x00);

    // 2. Set buffer counter value according to sample rate and frame rate.
    switch( BT_information.sampleRate)
    {
        case MDRV_SAMPLERATE_48K:
            tSampleRate = 48*1000;
            tSampRateDSP = 1;
            break;
        case MDRV_SAMPLERATE_44_1K:
            tSampleRate = 44.1*1000;
            tSampRateDSP = 2;
            break;
        case MDRV_SAMPLERATE_32K:
            tSampleRate = 32*1000;
            tSampRateDSP = 3;
            break;
        case MDRV_SAMPLERATE_16K:
        case MDRV_SAMPLERATE_BYPASS:
        default:
            DBG_AUDIO_ERROR(printk(" === Upload unknown sampling rate (%d)!! ===\n",BT_information.sampleRate));
            break;
    }

    _BT_BufferSize = (tSampleRate/BT_information.frameRate) *4 ;
    tCounter = _BT_BufferSize/8;
    MDrv_MAD_SetBTBufferCounter(tCounter);

    // 3. Set sampling rate for download path and enable to card reader synthesizer
    MDrv_MAD_BTUpload_Samprate_Set(1,tSampRateDSP);

    // 4. Start upLoad path command.
    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x01);  // start upload BT with PCM

    _BT_RunningUp = 1;

    _BT_Count = 0;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_PCMStopUpload
//  [Description]:
//      This routine : stop uploading PCM data for Bluetooth headset
//  [Arguments]:
//      NONET
//*******************************************************************************
void MDrv_MAD_PCMStopUpload(void)
{
    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x00);  // stop upload BT //CHECK
    MHal_MAD_WriteMaskByte(0x2D41, 0x80, 0x00);
    msleep(2);
    MDrv_MAD_BTUpload_Samprate_Set(0,0);
    _BT_RunningUp = 0;

    if(BT_information.bSBCOnOff == 1)
    {
        MDrv_MAD2_ReLoadCode(AU_DVB2_ADVSND_NONE);
    }

    BT_information.pBufAddr = NULL;
    BT_information.totalBufSize = 0;
    BT_information.bufSize = 0;
    BT_information.frameRate = 0;
    BT_information.sampleRate = 0;
    BT_information.bSBCOnOff = 0;
    BT_information.channelMode = 0;
    BT_information.blockLength = 0;
    BT_information.subBands = 0;
    BT_information.allocationMethod = 0;
    BT_information.minBitpool = 0;
    BT_information.maxBitpool = 0;

    //printk("MDrv_MAD_PCMStopUpload()....\n");
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_PCMStartUpload
//  [Description]:
//      This routine : start uploading PCM data for Bluetooth headset
//  [Arguments]:
//      MDRV_BT_T
//*******************************************************************************
void MDrv_MAD_PCMStartDownload(MDRV_BT_T *infoBT)
{
    U16 synthrate = 0;
    BT_information.pBufAddr = (void *) CKSEG1ADDR(infoBT->pBufAddr);//for CJ test
    MAD_DEBUG_P1(printk("%s is called, BT Bufaddr = 0x%x\r\n", __FUNCTION__, (U32) BT_information.pBufAddr));

    //reg backup
    BT_information.reg2B80 = MHal_MAD_ReadReg(0x2B80);
    BT_information.reg2BCE = MHal_MAD_ReadReg(0x2BCE);
    BT_information.reg2C62 = MHal_MAD_ReadReg(0x2C62);
    BT_information.reg2C64 = MHal_MAD_ReadReg(0x2C64);
    BT_information.reg2BAC = MHal_MAD_ReadReg(0x2BAC);

    BT_information.totalBufSize = infoBT->totalBufSize;
    BT_information.bufSize =infoBT->bufSize;
    BT_information.frameRate = infoBT->frameRate;
    BT_information.sampleRate = infoBT->sampleRate;
    BT_information.bSBCOnOff = infoBT->bSBCOnOff;
    BT_information.channelMode = infoBT->channelMode;
    BT_information.blockLength = infoBT->blockLength;
    BT_information.subBands = infoBT->subBands;
    BT_information.allocationMethod = infoBT->allocationMethod;
    BT_information.minBitpool = infoBT->minBitpool;
    BT_information.maxBitpool = infoBT->maxBitpool;
    BT_information.bitpool = infoBT->bitpool;

#if 0 //======if LG M/W not ready, please change to #if 1 for mstar test
    BT_information.sampleRate = MDRV_SAMPLERATE_48K;
    infoBT->frameRate = 25;//40ms
#endif
    switch( BT_information.sampleRate)
    {
        case MDRV_SAMPLERATE_48K:
            _BT_BufferSize = (48000/(infoBT->frameRate))*8;
	    synthrate = 0x1194;
            break;
        case MDRV_SAMPLERATE_44_1K:
            _BT_BufferSize = (44100/(infoBT->frameRate))*8;
	     synthrate = 0x1322;
            break;
        case MDRV_SAMPLERATE_32K:
            _BT_BufferSize = (32000/(infoBT->frameRate))*8;
	     synthrate = 0x1A5E;
            break;
        case MDRV_SAMPLERATE_16K:
             _BT_BufferSize = (16000/(infoBT->frameRate))*8;
	     synthrate = 0x34BC;
            break;
        case MDRV_SAMPLERATE_BYPASS:
        default:
            DBG_AUDIO_ERROR(printk("Download unknown sampling rate (%d)!!\n",BT_information.sampleRate));
            break;
    }
    BufferLineSize = _BT_BufferSize/0x10;
    MHal_MAD_WriteMaskByte(0x2CB0, 0x03, 0x02);//Synthesize set to dvb2
    MDrv_MAD_Set_BT_SampleCounter(48000/infoBT->frameRate);
    memset((void *)SE_ES_ADDR+0xF8000, 0, _BT_BufferSize*2);
    MHal_MAD_WriteMaskReg(0x2D06, 0xFFFF, 0x0C00);
    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x00);
    MHal_MAD_WriteMaskReg(0x2B84,  0xFFFF, (MDrv_MAD_GetDspMadBaseAddr(DSP_SE) + 0xF8000)/0x1000);
    MHal_MAD_WriteMaskReg(0x2B86, 0xFFFF, BufferLineSize*2);
    MHal_MAD_WriteMaskByte(0x2C62, 0xFF, 0xc6);
    MHal_MAD_WriteMaskByte(0x2C65, 0xFF, 0x81);
    MHal_MAD_WriteMaskReg(0x2B82, 0xFFFF, 0x0000);//after copy the data to buffer.
    MHal_MAD_WriteMaskReg(0x2B88, 0xFFFF, BufferLineSize+1);//if input data more than _BT_BufferSize/0x10+1 means over run.
    MHal_MAD_WriteMaskReg(0x2B8A, 0xFFFF, 0x0001);
    MHal_MAD_WriteMaskByte(0x2BCF, 0x02, 0x02);
    MHal_MAD_WriteMaskReg(0x2BAC, 0xFFFF, synthrate);//set synthesizer sampling rate 48k
    MHal_MAD_WriteMaskByte(0x2BCE, 0x02, 0x02);//synthesizer toggle
    msleep(1);
    MHal_MAD_WriteMaskByte(0x2BCE, 0x02, 0x00);
    MHal_MAD_WriteMaskReg(0x2B80, 0xFFFF, 0x6501);
    msleep(1);
    MHal_MAD_WriteMaskReg(0x2B80, 0xFFFF, 0x2501);
    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x02);
    _BT_RunningDown = 1;

    _BT_Count = 1;//For Download PCM
#if 0 //CHECK
    U32 tSampleRate=0, tCounter=0;
    U16 tSampRateDSP=0;

#if 0
    printk("pBufAddr = 0x%x\n",(void *)infoBT->pBufAddr);
    printk("totalBufSize = 0x%x\n",infoBT->totalBufSize);
    printk("bufSize = 0x%x\n",infoBT->bufSize);
    printk("frameRate = %d\n",infoBT->frameRate);
    printk("sampleRate = %d\n",infoBT->sampleRate);
    printk("channelMode = %d\n",infoBT->channelMode);
    printk("blockLength = %d\n",infoBT->blockLength);
    printk("subBands = %d\n",infoBT->subBands);
    printk("allocationMethod = %d\n",infoBT->allocationMethod);
    printk("minBitpool = %d\n",infoBT->minBitpool);
    printk("maxBitpool = %d\n",infoBT->maxBitpool);
    printk("bitpool = %d\n",infoBT->bitpool);
#endif
#if 1 //By YWJung LGE 2008.08.13
   BT_information.pBufAddr = (void *) CKSEG1ADDR(infoBT->pBufAddr);
#else
   (void *) BT_information.pBufAddr = (void *)infoBT->pBufAddr;
#endif
    BT_information.totalBufSize = infoBT->totalBufSize;
    BT_information.bufSize =infoBT->bufSize;
    BT_information.frameRate = infoBT->frameRate;
    BT_information.sampleRate = infoBT->sampleRate;
    BT_information.bSBCOnOff = infoBT->bSBCOnOff;
    BT_information.channelMode = infoBT->channelMode;
    BT_information.blockLength = infoBT->blockLength;
    BT_information.subBands = infoBT->subBands;
    BT_information.allocationMethod = infoBT->allocationMethod;
    BT_information.minBitpool = infoBT->minBitpool;
    BT_information.maxBitpool = infoBT->maxBitpool;
    BT_information.bitpool = infoBT->bitpool;

    memset((void *)(SE_ES_ADDR+0xE8000),0,0x1000);

    if(BT_information.bSBCOnOff == 1)
    {
        MDrv_MAD2_ReLoadCode(AU_DVB2_ADVSND_SBC);
        // 2. Set buffer counter value according to sample rate and frame rate.
        switch( BT_information.sampleRate)
        {
            case MDRV_SAMPLERATE_48K:
                tSampleRate = 48*1000;
                tSampRateDSP = 0x0000;
                break;
            case MDRV_SAMPLERATE_44_1K:
                tSampleRate = 44.1*1000;
                tSampRateDSP = 0x1000;
                break;
            case MDRV_SAMPLERATE_32K:
                tSampleRate = 32*1000;
                tSampRateDSP = 0x2000;
                break;
            case MDRV_SAMPLERATE_16K:
                tSampleRate = 16*1000;
                tSampRateDSP = 0x3000;
                break;
            case MDRV_SAMPLERATE_BYPASS:
            default:
                printk("unknown sampling rate (%d)!!\n",BT_information.sampleRate);
                break;
        }

        _BT_BufferSize = (tSampleRate/BT_information.frameRate) *4 ;
        tCounter = _BT_BufferSize/8;
        MDrv_MAD_SetBTBufferCounter(tCounter);

        // 3. Set sampling rate for download path.
        MHal_MAD_WriteMaskReg(REG_SOUND_CMD_BT,0x3000,tSampRateDSP);

       // DSP decoder2 clock select
        MHal_MAD_WriteReg(0x2C62,0x0000);//20081106 Add

    // 4. Change the CH5 input source to DEC2.
        MHal_MAD_WriteReg(DSP_CH5_CFG,0x00A9);
        MHal_MAD_WriteMaskReg(REG_SOUND_CMD_BT,0x0700,0x0400);  // start download BT with SBC
        _BT_RunningDown = 1;

        _BT_Count = 0;

         return;
    }

    // 1. Stop Bluetooth first.
    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x00);

    // 2. Set buffer counter value according to sample rate and frame rate.
    switch( BT_information.sampleRate)
    {
        case MDRV_SAMPLERATE_48K:
            tSampleRate = 48*1000;
            tSampRateDSP = 0x0000;
            break;
        case MDRV_SAMPLERATE_44_1K:
            tSampleRate = 44.1*1000;
            tSampRateDSP = 0x1000;
            break;
        case MDRV_SAMPLERATE_32K:
            tSampleRate = 32*1000;
            tSampRateDSP = 0x2000;
            break;
        case MDRV_SAMPLERATE_16K:
            tSampleRate = 16*1000;
            tSampRateDSP = 0x3000;
            break;
        case MDRV_SAMPLERATE_BYPASS:
        default:
            printk("unknown sampling rate (%d)!!\n",BT_information.sampleRate);
            break;
    }

    _BT_BufferSize = (tSampleRate/BT_information.frameRate) *4 ;
    tCounter = _BT_BufferSize/8;

    MHal_MAD_WriteMaskReg(0x2B84, (MDrv_MAD_GetDspMadBaseAddr(DSP_DEC) + 0xF8000)/0x1000);
    MHal_MAD_WriteMaskReg(0x2B86, _BT_BufferSize*2/0x10/*0x10 for line unit*/);

    // 5. Start DownLoad path command.
    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x02);  // start download BT with PCM

    _BT_RunningDown = 1;
    _BT_Count = 1;
#endif
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_PCMStopDownload
//  [Description]:
//      This routine : stop downloading PCM data from Bluetooth device.
//  [Arguments]:
//      NONET
//*******************************************************************************
void MDrv_MAD_PCMStopDownload(void)
{

    MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x00);  // stop download BT //CHECK
    _BT_RunningDown = 0;
    msleep(2);

    if(BT_information.bSBCOnOff == 1)
        MDrv_MAD2_ReLoadCode(AU_DVB2_NONE);
    else
    {
        MHal_MAD_WriteMaskReg(0x2C62, 0xFFFF, BT_information.reg2C62);
        MHal_MAD_WriteMaskReg(0x2C64, 0xFFFF, BT_information.reg2C64);
        MHal_MAD_WriteMaskReg(0x2B80, 0xFFFF, BT_information.reg2B80);
	 MHal_MAD_WriteMaskReg(0x2BAC, 0xFFFF, BT_information.reg2BAC);
	 MHal_MAD_WriteMaskByte(0x2BCE, 0x02, 0x02);//synthesizer toggle
	 msleep(1);
        MHal_MAD_WriteMaskByte(0x2BCE, 0x02, 0x00);
	 msleep(1);
	 MHal_MAD_WriteMaskReg(0x2BCE, 0xFFFF, BT_information.reg2BCE);
	 MHal_MAD_WriteMaskByte(0x2CB0, 0x03, 0x00);//synthesizer set to pll
    }
    BT_information.pBufAddr = NULL;
    BT_information.totalBufSize = 0;
    BT_information.bufSize = 0;
    BT_information.frameRate = 0;
    BT_information.sampleRate = 0;
    BT_information.bSBCOnOff = 0;
    BT_information.channelMode = 0;
    BT_information.blockLength = 0;
    BT_information.subBands = 0;
    BT_information.allocationMethod = 0;
    BT_information.minBitpool = 0;
    BT_information.maxBitpool = 0;


#if 0 /* Jonghyuk, Lee 090206 LGE */
/* SPDIF out is wrong when BT upload mp3 is paused. Jonghyuk, Lee 090113 LGE */
    MHal_MAD_WriteReg(DSP_CH5_CFG,0x00AB/*0x00A9*/);
    MHal_MAD_WriteReg(DSP_CH4_CFG,0x00AB/*0x00A9*/);
#endif
    //printk("MDrv_MAD_PCMStopUpload()....\n");
}

//******************************************************************************
//  [Function Name]:
//      MDrv_ADCInit
//  [Description]:
//      This routine is ADC relational register Init.
//*******************************************************************************
void MDrv_MAD_ADCInit(void)
{
    MHal_MAD_WriteMaskByte(0x2CDA, 0x03, 0x00);    // power on ADC0 & ADC1
    MHal_MAD_WriteMaskByte(0x2CEE, 0x03, 0x03);    //enable ADC dither
    MHal_MAD_WriteMaskByte(0x2CE3, 0x03, 0x00);    // power on ADC PGA
    MHal_MAD_WriteMaskByte(0x2CE5, 0xFC, 0x00);    //ADC0 & ADC1 PGAain=0dB
}

BOOL MDrv_MAD_DTVInUse(void)
{
    if (dtv_mode_en ==1)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void MDrv_MAD_Clip_Play(unsigned long unused)
{
    U32 writeAddress=0,writeNumber=0;
    static U32 tag=1;
    //static int tmpTest=0;
    //printk("\nMDrv_MAD_Clip_Play....\n");

    writeAddress = ES_ADDR + MDrv_MAD_ReadMailBox(DSP_DEC, 5)*16;
    writeNumber = MDrv_MAD_ReadMailBox(DSP_DEC, 6)*16;

    //printk("writeAddress = 0x%x\n",writeAddress);
    //printk("_clipWriteLastAddr = 0x%x\n",_clipWriteLastAddr);
    //printk("writeNumber = %d\n",writeNumber);
    //printk("audioClipInfo.pVaAddr = 0x%x\n",audioFileParams.pAudioAddr);

    //memcpy((void*)writeAddress, (void*)audioClipInfo.pVaAddr, writeNumber);

    if ((!audioFileParams.u8AudioIsInfinite) &&(audioFileParams.u32AudioLoopCounts==0))
    {
        //MDrv_MAD_StopAudioClip();//Removed, StopAudioClip by AP's decision
         if (MDrv_MAD_ReadMailBox(DSP_DEC, 15) > 1)
        {
            if(MDrv_MAD_ReadMailBox(DSP_DEC, 7)==0)
            {
         //printk("\@@@@@@@@@....\n");
                MDrv_MAD_StopAudioClip();
                MDrv_Event_to_AP(MAD_EVENT_CLIP);

                tag = 1;
                _clipWriteLastAddr = 0;
                return;
            }
        }
        // interrupt MAD
        MDrv_MAD_SendIntrupt(DSP_DEC,0xE0);
        return;
    }

    if( writeAddress == _clipWriteLastAddr )
    {
        MDrv_MAD_SendIntrupt(DSP_DEC,0xE0);
        return;
    }

    _clipWriteLastAddr = writeAddress ;
    if (audioFileParams.u32AudioFileIndex < audioFileParams.u32AudioLength)
    {
        //printk("$");
        //printk("\nver_Pcm_PlayAudioFile 1....\n");
        // copy to MAD buffer, enough to copy
        if ((audioFileParams.u32AudioFileIndex + writeNumber) <= audioFileParams.u32AudioLength)
        {
            //printk("\MDrv_MAD_Clip_Play(0x%x)(0x%x)(%d)(%d)....\n",writeAddress,audioFileParams.pAudioAddr,audioFileParams.u32AudioFileIndex,writeNumber);
            //printk("1.Copy to addr =  0x%x, Index = %d, sum = %d, Length = %d\n", writeAddress, audioFileParams.u32AudioFileIndex, (audioFileParams.u32AudioFileIndex + writeNumber), audioFileParams.u32AudioLength);
#if 1
            memcpy((void*)writeAddress,
                   (void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex),
                   writeNumber);
#endif
            audioFileParams.u32AudioFileIndex += writeNumber;
//printk("\nver_Pcm_PlayAudioFile 2-0 , index = %d....\n", audioFileParams.u32AudioFileIndex);
            //check file end
            if (audioFileParams.u32AudioFileIndex ==    audioFileParams.u32AudioLength)
            {
            //printk("\nver_Pcm_PlayAudioFile 2....\n");
                if (audioFileParams.u8AudioIsInfinite)
                {
            //printk("\nver_Pcm_PlayAudioFile 2-1 ....\n");
                    audioFileParams.u32AudioFileIndex = 0;
                }
                else if (audioFileParams.u32AudioLoopCounts)
                {
            //printk("\nver_Pcm_PlayAudioFile 2-2 ....\n");
            //printk("1. LoopCounts:%d\n",audioFileParams.u32AudioLoopCounts);
                    audioFileParams.u32AudioFileIndex = 0;
                    audioFileParams.u32AudioLoopCounts--;
                }
            }
        }
        else
        {
            //printk("\nver_Pcm_PlayAudioFile 3....\n");
            // clear the buffer first
            memset((void*)writeAddress, 0, writeNumber);

            if (writeNumber <= audioFileParams.u32AudioLength)
            {
                // normal case
                int feedsize = 0;
                memcpy((void*)writeAddress,(void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex),
                       audioFileParams.u32AudioLength - audioFileParams.u32AudioFileIndex);
                feedsize += (audioFileParams.u32AudioLength -audioFileParams.u32AudioFileIndex);

                //printk("2.Copy to addr = 0x%x, Feedsize = %d, writeNumber = %d, Index = %d\n", writeAddress, feedsize, writeNumber, audioFileParams.u32AudioFileIndex);
                //printk("\nver_Pcm_PlayAudioFile 3-1... index = %d....\n", audioFileParams.u32AudioFileIndex);

                if (!audioFileParams.u8AudioIsInfinite)
                {
                    audioFileParams.u32AudioLoopCounts--;
                    //printk("2.LoopCounts:%d\n",audioFileParams.u32AudioLoopCounts);
                }

                if ((audioFileParams.u8AudioIsInfinite)||(audioFileParams.u32AudioLoopCounts>0))
                {
                    audioFileParams.u32AudioFileIndex = 0;
                    memcpy((void*)(writeAddress+feedsize),
                           (void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex),
                           writeNumber - feedsize);
                    //printk("3.Copy to addr = 0x%x\n", writeAddress+feedsize);
                    audioFileParams.u32AudioFileIndex = writeNumber- feedsize;
                    //printk("\nver_Pcm_PlayAudioFile 3.-3... index = %d....\n", audioFileParams.u32AudioFileIndex);
                }
                else
                {
                //printk("2.Counts=0, set Index = AudioLength\n");
                    // this is the last time, i.e. PcmAudioFileParams.u32PcmAudioLoopCounts==0
                    audioFileParams.u32AudioFileIndex = audioFileParams.u32AudioLength;
                   // printk("\nver_Pcm_PlayAudioFile 3-4.... index = %d....\n", audioFileParams.u32AudioFileIndex);
                }

            }
            else    // writeNumber > MHEGAudioLength
            {
            //printk("\nver_Pcm_PlayAudioFile 5....\n");
            //printk("2.Audio Clip Length small then once write number (May 1024)\n");
                // special case
                if (audioFileParams.u8AudioIsInfinite)
                {
                    // feed data to MAD
                    int feedsize = 0;

                   memcpy((void*)writeAddress,
                           (void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex),
                           (audioFileParams.u32AudioLength -audioFileParams.u32AudioFileIndex));
                            feedsize += (audioFileParams.u32AudioLength -audioFileParams.u32AudioFileIndex);
                            audioFileParams.u32AudioFileIndex = 0;

                    while(feedsize+ audioFileParams.u32AudioLength <= writeNumber)
                    {
                        memcpy((void*)(writeAddress+feedsize),
                               (void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex),
                               audioFileParams.u32AudioLength );
                            feedsize += audioFileParams.u32AudioLength;
                    }

                    if (feedsize == writeNumber)
                    {
                        audioFileParams.u32AudioFileIndex = 0;
                    }
                    else
                    {
                        memcpy((void*)(writeAddress+feedsize),
                               ((void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex)),
                               writeNumber-feedsize);
                            audioFileParams.u32AudioFileIndex = writeNumber-feedsize;
                    }
                }
                else
                {
                    int feedsize = 0;
                    memcpy((void*)writeAddress,
                           (void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex),
                           audioFileParams.u32AudioLength - audioFileParams.u32AudioFileIndex);
                            audioFileParams.u32AudioLoopCounts--;
                            feedsize += audioFileParams.u32AudioLength - audioFileParams.u32AudioFileIndex;

                    if (audioFileParams.u32AudioLoopCounts)
                        audioFileParams.u32AudioFileIndex = 0;

                    while(audioFileParams.u32AudioLoopCounts && (feedsize+audioFileParams.u32AudioLength <= writeNumber))
                    {
                        memcpy((void*)(writeAddress+feedsize),
                               (void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex),
                               audioFileParams.u32AudioLength ),
                            feedsize += audioFileParams.u32AudioLength;
                            audioFileParams.u32AudioLoopCounts--;
                    }

                    if (audioFileParams.u32AudioLoopCounts)
                    {
                        memcpy((void*)(writeAddress+feedsize),
                               (void*)(audioFileParams.pAudioAddr+audioFileParams.u32AudioFileIndex),
                                      writeNumber- feedsize );
                                    audioFileParams.u32AudioFileIndex =  writeNumber - feedsize;
                    }
                }
            }
        }
    }

    // handshake parameter to MAD
    MDrv_MAD_WriteMailBox(DSP_DEC, 6, tag++);//CHECK, DSP_DEC or DSP_SE

    //printk("\nver_Pcm_PlayAudioFile 4(%d)....\n",tag);
    // reset tag
    if ((tag%65535)==0)
        tag = 1;
    // interrupt MAD
    MDrv_MAD_SendIntrupt(DSP_DEC,0xE0);
    //OS_Delayms(10);
}

void MDrv_MAD_SetDTV_NormalPath(void)
{
#if 0 //CHECK, not used in T3
    MDrv_MAD_SetNormalPath(AUDIO_PATH_MAIN_SPEAKER, AUDIO_SRC_INPUT, AUDIO_OUTPUT_MAIN_SPEAKER);//I2S OUT
    MDrv_MAD_SetNormalPath(AUDIO_PATH_HP,                    AUDIO_SRC_INPUT, AUDIO_OUTPUT_HP);//DAC OUT 0
    MDrv_MAD_SetNormalPath(AUDIO_PATH_SRC,                  AUDIO_SOURCE_DTV, AUDIO_SRC_OUT);
    MDrv_MAD_SetNormalPath(AUDIO_PATH_SPDIF, AUDIO_SRC_INPUT, AUDIO_SPDIF_OUTPUT); //LGE_isoul119_081219_add spdif path setting
    MDrv_MAD_SetNormalPath(AUDIO_PATH_SCART2,            AUDIO_SOURCE_DTV, AUDIO_OUTPUT_SCART2);//DAC OUT 2
    MDrv_MAD_SetNormalPath(AUDIO_PATH_SPDIF,               AUDIO_SOURCE_DTV, AUDIO_SPDIF_OUTPUT);
#endif
}

void MDrv_MAD_SetATV_NormalPath(void)
{
#if 0 //CHECK, not used in T3
    MDrv_MAD_SetNormalPath(AUDIO_PATH_MAIN_SPEAKER, AUDIO_SRC_INPUT, AUDIO_OUTPUT_MAIN_SPEAKER);//I2S OUT
    MDrv_MAD_SetNormalPath(AUDIO_PATH_HP,                    AUDIO_SRC_INPUT, AUDIO_OUTPUT_HP);//DAC OUT 0
    MDrv_MAD_SetNormalPath(AUDIO_PATH_SRC,                  AUDIO_SOURCE_ATV, AUDIO_SRC_OUT);
    MDrv_MAD_SetNormalPath(AUDIO_PATH_SCART1,            AUDIO_SOURCE_ATV, AUDIO_OUTPUT_SCART1);//DAC OUT 2
    MDrv_MAD_SetNormalPath(AUDIO_PATH_SPDIF,               AUDIO_SOURCE_ATV, AUDIO_SPDIF_OUTPUT);
#endif
}


#ifdef ENABLE_MADMONITOR  // dsp_revive
//******************************************************************************
//  [Function Name]:
//      MDrv_DecWhile_CNT
//  [Description]:
//  [Arguments]:
//      None
//*******************************************************************************
BYTE MDrv_MAD_DecWhile_CNT(void)
{
    return(MHal_MAD_ReadByte(REG_MB_DEC_WHILE_CNT));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_DecTimer_CNT
//  [Description]:
//  [Arguments]:
//      None
//*******************************************************************************
BYTE MDrv_MAD_DecTimer_CNT(void)
{
    return(MHal_MAD_ReadByte(REG_MB_DEC_TIMER_CNT));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_SeWhile_CNT
//  [Description]:
//  [Arguments]:
//      None
//*******************************************************************************
BYTE MDrv_MAD_SeWhile_CNT(void)
{
    return(MHal_MAD_ReadByte(REG_MB_SE_WHILE_CNT));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_SeIsr_CNT
//  [Description]:
//  [Arguments]:
//      None
//*******************************************************************************
BYTE MDrv_MAD_SeISR_CNT(void)
{
    return(MHal_MAD_ReadByte(REG_MB_SE_ISR_CNT));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_SeTimer_CNT
//  [Description]:
//  [Arguments]:
//      None
//*******************************************************************************
BYTE MDrv_MAD_SeTimer_CNT(void)
{
    return(MHal_MAD_ReadByte(REG_MB_SE_TIMER_CNT));
}


/********************************************************************************/
///
/// MAD Monitor
///
/********************************************************************************/
void MDrv_MAD_Audio_Monitor(void)
{
#ifdef ENABLE_MADMONITOR
    U8   u8CurrentDecWhile ;
    U8   u8CurrentDecTimer;
    U8   u8CurrentSeWhile;
    U8   u8CurrentSeISR;

    static U8  u8LastDecWhile = 1;
    static U8  u8LastDecTimer = 1;
    static U8  u8LastSeWhile = 1;
    static U8  u8LastSeISR = 1;
    static U8  u8DecWhileFailCount = 0;
    static U8  u8DecTimerFailCount = 0;
    static U8  u8SeWhileFailCount = 0;
    static U8  u8SeISRFailCount = 0;
    static U32 u32DecDspReviveCount = 0;
    static U8  u8NeedRestoreParam = 0;
    static U8  u8BackupReg = 0;


    if(!dsp_revive_flag)
    return;

    u8CurrentDecWhile =  MDrv_MAD_DecWhile_CNT();
    u8CurrentDecTimer = MDrv_MAD_DecTimer_CNT();
    u8CurrentSeWhile = MDrv_MAD_SeWhile_CNT();
    u8CurrentSeISR = MDrv_MAD_SeISR_CNT();

    MAD_DEBUG_P2(printk("\r\n u8CurrentDecWhile is %x\r\n",(WORD)u8CurrentDecWhile));
    MAD_DEBUG_P2(printk("\r\n u8CurrentDecTimer is %x\r\n",(WORD)u8CurrentDecTimer));
    MAD_DEBUG_P2(printk("\r\n u8CurrentSeWhile is %x\r\n",(WORD)u8CurrentSeWhile));
    MAD_DEBUG_P2(printk("\r\n u8CurrentSeISR is %x\r\n",(WORD)u8CurrentSeISR));


    MAD_DEBUG_P2(printk("\r\n u8LastDecWhile is %x\r\n",(WORD)u8LastDecWhile));
    MAD_DEBUG_P2(printk("\r\n u8LastDecTimer is %x\r\n",(WORD)u8LastDecTimer));
    MAD_DEBUG_P2(printk("\r\n u8LastSeWhile is %x\r\n",(WORD)u8LastSeWhile));
    MAD_DEBUG_P2(printk("\r\n u8LastSeISR is %x\r\n",(WORD)u8LastSeISR));

    //Checking Decoder DSP counter
    if (u8CurrentDecWhile - u8LastDecWhile ==0)
    {
        u8DecWhileFailCount++;
    }
    else
    {
        u8DecWhileFailCount=0;
    }
    if (u8CurrentDecTimer - u8LastDecTimer ==0)
    {
        u8DecTimerFailCount++;
    }
    else
    {
        u8DecTimerFailCount=0;
    }
    //Checking Sound Effect DSP counter
    if (u8CurrentSeWhile - u8LastSeWhile ==0)
    {
        u8SeWhileFailCount++;
    }
    else
    {
        u8SeWhileFailCount=0;
    }
    if (u8CurrentSeISR - u8LastSeISR ==0)
    {
        u8SeISRFailCount++;
    }
    else
    {
        u8SeISRFailCount=0;
    }
    MAD_DEBUG_P2(printk("\r\n u8DecWhileFailCount is %x\r\n",(WORD)u8DecWhileFailCount));
    MAD_DEBUG_P2(printk("\r\n u8DecTimerFailCount is %x\r\n",(WORD)u8DecTimerFailCount));
    //Checking if rebooting Decoder DSP
    if ((u8DecWhileFailCount>=10) ||(u8DecTimerFailCount>=10))
    {
        audio_recovery = 1;
        u8DecWhileFailCount = 0;
        u8DecTimerFailCount = 0;
        u32DecDspReviveCount++;

        MDrv_MAD_Reg_Backup();
        u8BackupReg = 1;

        MDrv_MAD_SetDecDspReviveCount(u32DecDspReviveCount);
        DBG_AUDIO_ERROR(printk("\n=== DSP Revive:rebooting DEC dsp===\n"));
        MHal_MAD_WriteMaskReg(0x2C02, 0x0004, 0);
        MDrv_MAD_RebootDecDSP();
        MHal_MAD_WriteMaskReg(0x2C02, 0x0004, 0x0004);
        u8NeedRestoreParam = 1;
    }

    MAD_DEBUG_P2(printk("\r\n u8SeWhileFailCount is %x\r\n",(WORD)u8SeWhileFailCount));
    MAD_DEBUG_P2(printk("\r\n u8SeISRFailCount is %x\r\n",(WORD)u8SeISRFailCount));
    //Checking if rebooting Sound Effect DSP
    if ((u8SeWhileFailCount>=10) ||(u8SeISRFailCount>=10))
    {
        DBG_AUDIO_ERROR(printk("\n=== DSP Revive:rebooting SE dsp===\n"));
        audio_recovery = 1;
        u8SeWhileFailCount = 0;
        u8SeISRFailCount = 0;

        if(u8BackupReg == 0)
            MDrv_MAD_Reg_Backup();

        MHal_MAD_WriteMaskReg(0x2C02, 0x0004, 0);
        MDrv_MAD_RebootSndEffDSP();
        MHal_MAD_WriteMaskReg(0x2C02, 0x0004, 0x0004);
        u8NeedRestoreParam = 1;
    }

    if(u8NeedRestoreParam)
    {
        u8NeedRestoreParam = 0;
        u8BackupReg=0;
        MDrv_MAD_Reg_Restore();
        MDrv_Event_to_AP(MAD_EVENT_PARAM_RESTORE);
    }

    // save status
    u8LastDecWhile = u8CurrentDecWhile;
    u8LastDecTimer = u8CurrentDecTimer;
    u8LastSeWhile = u8CurrentSeWhile;
    u8LastSeISR = u8CurrentSeISR;

    audio_recovery = 0;
#endif
}


#define SYS_REGOP(addr) *((volatile unsigned int*)(0xbf000000 + (addr)))
#if 0 //20090302 mstar remove
static U32 stimer( void ){

    U32 ret_timer ;



    // stop and capture timer value

    SYS_REGOP( (0x3C88<<1) ) = 0x0301 ;

    // set max timer0 value

    SYS_REGOP( (0x3C80<<1) ) = 0xFFFF ;

    SYS_REGOP( (0x3C82<<1) ) = 0xFFFF ;

    SYS_REGOP( (0x3C82<<1) ) = 0xFFFF ; // one more right for delay

    ret_timer = SYS_REGOP( (0x3C86<<1) ) ;

    ret_timer <<= 16 ;

    ret_timer += SYS_REGOP( (0x3C84<<1) ) ;

    // re-start timer

    SYS_REGOP( (0x3C88<<1) ) = 0x0003 ;

    SYS_REGOP( (0x3C88<<1) ) = 0x0300 ;

    return ret_timer ;// return time unit is 1/12 us = (1/12000000) second

}
#endif
void MDrv_MAD_SetDecDspReviveCount(U32 ReviveCount)
{
    gDecDspReviveCount = ReviveCount;
}

U32 MDrv_MAD_GetDecDspReviveCount(void)
{
    return gDecDspReviveCount;
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SetMcuCmd()
/// @brief \b Function \b Description: This routine is to write MCU cmd.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetMcuCmd(MS_U8 cmd)
{
    MHal_MAD_WriteMaskReg(REG_MB_DEC_CMD1, 0xFF00, (MS_U16)cmd<<8);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SetDspIDMA()
/// @brief \b Function \b Description:  This function is used to set DSP IDMA.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetDspIDMA(void)
{
    MHal_MAD_WriteMaskReg(REG_DEC_IDMA_CTRL0, 0x0003, 0x0003);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_GetIsDtvFlag()  @@Cathy
/// @brief \b Function \b Description:  Report the decoder type is ATV or DTV
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b g_bIsDTV: 0 -> ATV , 1 -> DTV
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_GetIsDtvFlag(void)
{
    return g_bIsDTV;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_GetIsDtvFlag()  @@Cathy
/// @brief \b Function \b Description:  Report the decoder type is ATV or DTV
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b g_bIsDTV: 0 -> ATV , 1 -> DTV
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetIsDtvFlag(MS_BOOL bIsDTV)
{
    g_bIsDTV=bIsDTV;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_Dec_Status()  @@Cathy
/// @brief \b Function \b Description:  This routine is used to read the Decoder status.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b MS_U8    : Decoder Status
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD_Dec_Status(void)
{
    return(MHal_MAD_ReadByte(REG_DEC_DECODE_CMD));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SE_Status()  @@Cathy
/// @brief \b Function \b Description:  This routine is used to read the SE status.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b MS_U8    : Decoder Status
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD_Se_Status(void)
{
    return(MHal_MAD_ReadByte(REG_SE_DECODE_CMD));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_Set_Fading()  @@Need_Modify
/// @brief \b Function \b Description:  This routine is used to set the Fading response time
/// @param <IN>        \b u32VolFading    : Fading response time parameter
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_Set_Fading(MS_U32 u32VolFading)
{
     MDrv_MAD2_Write_DSP_sram(0x114C, u32VolFading, DSP_MEM_TYPE_PM);        //need touch
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_AUDIO_BackupMailbox()    @@Need_Modify
/// @brief \b Function \b Description:  This function is used to backup SIF mailbox.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_BackupMailbox(void)
{
    int i;

    for(i=0;i<12;i++)
    {
        SIF_MailBoxArray[i]=MHal_MAD_ReadByte(REG_SE_M2D_MAIL_BOX_BASE+i);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_AUDIO_RestoreMailbox()   @@Need_Modify
/// @brief \b Function \b Description:  This function is used to restore SIF mailbox.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_RestoreMailbox(void)
{
    int i;

    for(i=0;i<12;i++)
    {
        MHal_MAD_WriteByte((REG_SE_M2D_MAIL_BOX_BASE+i), SIF_MailBoxArray[i]);
    }

}
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_AUDIO_ResetDSP()  @@Cathy
/// @brief \b Function \b Description:  This function is used to reset DSP.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_ResetDSP(void)
{
    MHal_MAD_WriteByte(REG_DEC_IDMA_CTRL0, 0x02);     // Reset DSP
    msleep(2);
    MHal_MAD_WriteByte(REG_DEC_IDMA_CTRL0, 0x03);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_AUDIO_SendIntrupt()
/// @brief \b Function \b Description:  set HDMI downsample rate
/// @param bDspType    \b :
/// @param u8Cmd       \b :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SendIntrupt(MS_BOOL bDspType,MS_U16 u8Cmd)
{
    if (bDspType == DSP_DEC)
    {
        MDrv_MAD_SetPIOCmd(u8Cmd);
        MDrv_MAD_TriggerPIO8();
    }
    else
    {
        MDrv_MAD2_SetPIOCmd(u8Cmd);
        MDrv_MAD2_TriggerPIO8();
    }

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_SetMcuCmd()
/// @brief \b Function \b Description: This routine is to write MCU cmd for PIO.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetPIOCmd(MS_U8 cmd)
{
    MHal_MAD_WriteMaskReg(REG_MB_DEC_PIO_ID,0xFF00, cmd<<8);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD2_SetPIOCmd()
/// @brief \b Function \b Description: This routine set a command on 0x2D23 for PIO8 interrupt(to DSP).
/// @param <IN>        \b u16Cmd    : 0xE0, for MHEG5 file protocol
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_SetPIOCmd(MS_U8 Cmd)
{
    MHal_MAD_WriteMaskReg(REG_MB_SE_PIO_ID, 0xFF00, (MS_U16)Cmd<<8);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_SPDIF_SetChannelStatus()
/// @brief \b Function \b Description: This routine is used to set SPdif channel status.
/// @param <IN>        \b   eType   :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SPDIF_SetChannelStatus(SPDIF_CS_MODE_TYPE eType, SPDIF_CS_MODE_VALUE eValue)
{
    MS_U8 u8Type;

    u8Type=(MS_U8)eType;

    switch(u8Type)
    {
        case SPDIF_CHANNEL_STATUS_FS:
            MHal_MAD_WriteByte(REG_AUDIO_SPDIF_OUT_CS3 , (MS_U8)eValue);
            break;
        case SPDIF_CHANNEL_STATUS_CATEGORY:
            MHal_MAD_WriteByte(REG_AUDIO_SPDIF_OUT_CS1 , (MS_U8)eValue);
            break;
        case SPDIF_CHANNEL_STATUS_PCM_FORMAT:
            MHal_MAD_WriteByte(REG_AUDIO_SPDIF_OUT_CS0 , (MS_U8)eValue);
            break;

        default :
            break;
    }

}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Reg_Backup
//  [Description]:
//      This routine : Backup registers
//  [Arguments]:
//      N/A
//*******************************************************************************
void MDrv_MAD_Reg_Backup(void)
{
    int i=0;

    while(Reg_Restore[i][0] != 0xFFFF)
    {
        Reg_Restore[i][1] = MHal_MAD_ReadReg((U32)Reg_Restore[i][0]);
        i++;
    }

    i=0;
    while(Vol_Reg_Restore[i][0] != 0xFFFF)
    {
        Vol_Reg_Restore[i][1] = MHal_MAD_ReadReg((U32)Vol_Reg_Restore[i][0]);
        i++;
    }

}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Reg_Restore
//  [Description]:
//      This routine : Restore registers
//  [Arguments]:
//      N/A
//*******************************************************************************
void MDrv_MAD_Reg_Restore(void)
{
    int i=0;

    while(Reg_Restore[i][0] != 0xFFFF)
    {
        MHal_MAD_WriteReg((U32)Reg_Restore[i][0],Reg_Restore[i][1]);
        i++;
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Reg_Restore
//  [Description]:
//      This routine : Restore audio volume/mute related registers
//  [Arguments]:
//      N/A
//*******************************************************************************
void MDrv_MAD_Vol_Reg_Restore(void)
{
    int i=0;

    while(Vol_Reg_Restore[i][0] != 0xFFFF)
    {
        MHal_MAD_WriteReg((U32)Vol_Reg_Restore[i][0],Vol_Reg_Restore[i][1]);
        i++;
    }
}


//Mstar add for Skype, 2009/09/22
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Init_Skype
//  [Description]:
//      This routine : initial for Skype
//  [Arguments]:
//      MDRV_SKYPE_T
//*******************************************************************************
void MDrv_MAD_Init_Skype(MDRV_SKYPE_T *infoSkype)
{
    //int i;

    Skype_information.pPcmInSrcAddr = infoSkype ->pPcmInSrcAddr;
    Skype_information.pPcmOutDestAddr = infoSkype ->pPcmOutDestAddr;
    Skype_information.pBsInSrcAddr = infoSkype ->pBsInSrcAddr;
    Skype_information.pBsOutDestAddr = infoSkype ->pBsOutDestAddr;
    Skype_information.pcm_bufSize = infoSkype ->pcm_bufSize;
    Skype_information.bs_bufSize = infoSkype ->bs_bufSize;
    Skype_information.skype_init = infoSkype ->skype_init;

    MDrv_MAD_SetDecCmd( AU_DVB_DECCMD_STOP);
    MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_G729);//Reload SBC
    MDrv_MAD_SetDecCmd(AU_DVB_DECCMD_PLAY);

#if 0
//---------unmute audio out for test--------
    MDrv_MAD_ProcessSetMute(1,0);
    MDrv_MAD_ProcessSetMute(2,0);
    MDrv_MAD_ProcessSetMute(3,0);
    MDrv_MAD_ProcessSetMute(4,0);
    MDrv_MAD_ProcessSetVolume(1,0x0c00);
//-------------------------------------
#endif
    MAD_DEBUG_P1(printk("MDrv_MAD_Init_Skype...\n"));

}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Stop_Skype
//  [Description]:
//      This routine : initial for Skype
//  [Arguments]:
//      MDRV_SKYPE_T
//*******************************************************************************
void MDrv_MAD_Stop_Skype(void)
{
    MDrv_MAD_SetDecCmd( AU_DVB_DECCMD_STOP);

    MAD_DEBUG_P1(printk("MDrv_MAD_Stop_Skype...\n"));
}


//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Skype_Dn_PCM
//  [Description]:
//      This function will do the data(PCM) copy to DSP and inform DSP decoding.
//  [Arguments]:
//      None
//*******************************************************************************
BOOL MDrv_MAD_Skype_Dn_PCM(void)
{
    MAD_DEBUG_P1(printk("->(0x%x,0x%x,%d)\n",(ES_ADDR+0xBA000),Skype_information.pPcmInSrcAddr,Skype_information.pcm_bufSize));

    if(Skype_information.skype_init == 0)
    {
        DBG_AUDIO_ERROR(printk("%s is called, but must do init first!!!\r\n", __FUNCTION__));
        return FALSE;
    }

    //1. copy data to DSP
    memcpy((void *)(ES_ADDR+0xBA000),(void *)Skype_information.pPcmInSrcAddr,Skype_information.pcm_bufSize);

    //2. inform DSP after copy data
    MDrv_MAD_SendIntrupt(DSP_DEC,0xE0);

    MAD_DEBUG_P1(printk("MDrv_MAD_Skype_Dn_PCM is called..\n"));
  return TRUE;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Skype_Up_PCM
//  [Description]:
//      This function will do the data(PCM) copy from DSP and inform AP.
//  [Arguments]:
//      None
//*******************************************************************************
BOOL MDrv_MAD_Skype_Up_PCM(void)
{
    MAD_DEBUG_P1(printk("->(0x%x,0x%x,%d)\n",Skype_information.pPcmOutDestAddr,(ES_ADDR+0xBB000),Skype_information.pcm_bufSize));

    if(Skype_information.skype_init == 0)
    {
        DBG_AUDIO_ERROR(printk("%s is called, but must do init first!!!\r\n", __FUNCTION__));
        return FALSE;
    }

    //1. copy data
    memcpy((void *)Skype_information.pPcmOutDestAddr,(void *)(ES_ADDR+0xBB000),Skype_information.pcm_bufSize);

    //2. inform AP after data copy
    MDrv_Event_to_AP(MAD_EVENT_SKYPE_PCM_UP);

    return TRUE;
}

//Mstar modify for Skype, 2009/10/12
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Skype_Dn_BS
//  [Description]:
//      This function will do the data(Bit Stream) copy to DSP and inform DSP to decoding.
//  [Arguments]:
//      None
//*******************************************************************************
BOOL MDrv_MAD_Skype_Dn_BS(void)
{
    MAD_DEBUG_P1(printk("->(0x%x,0x%x,%d)\n",(ES_ADDR+0xBA000),Skype_information.pBsInSrcAddr,Skype_information.bs_bufSize));

    if(Skype_information.skype_init == 0)
    {
        DBG_AUDIO_ERROR(printk("%s is called, but must do init first!!!\r\n", __FUNCTION__));
        return FALSE;
    }

    //1. copy data to DSP
    memcpy((void *)(ES_ADDR+0xBA000),(void *)Skype_information.pBsInSrcAddr,Skype_information.bs_bufSize);

    //2. inform DSP after copy data
    MDrv_MAD_SendIntrupt(DSP_DEC,0xE0);

    // test only, will remove later
    MDrv_Event_to_AP(MAD_EVENT_SKYPE_BS_UP);

    MAD_DEBUG_P1(printk("MDrv_MAD_Skype_Dn_BS is called..\n"));
  return TRUE;
}

//Mstar add for Skype, 2009/10/12
//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Skype_Up_BS
//  [Description]:
//      This function will do the data(Bit Stream) copy from DSP and inform AP.
//  [Arguments]:
//      None
//*******************************************************************************
BOOL MDrv_MAD_Skype_Up_BS(void)
{
    MAD_DEBUG_P1(printk("->(0x%x,0x%x,%d)\n",Skype_information.pBsOutDestAddr,(ES_ADDR+0xBB000),Skype_information.bs_bufSize));

    if(Skype_information.skype_init == 0)
    {
        DBG_AUDIO_ERROR(printk("%s is called, but must do init first!!!\r\n", __FUNCTION__));
        return FALSE;
    }

    //1. copy data
    memcpy((void *)Skype_information.pBsOutDestAddr,(void *)(ES_ADDR+0xBB000),Skype_information.bs_bufSize);

    //2. inform AP after data copy
    MDrv_Event_to_AP(MAD_EVENT_SKYPE_BS_UP);

    return TRUE;
}


