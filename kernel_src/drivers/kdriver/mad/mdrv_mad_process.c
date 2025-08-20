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
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include "mst_utility.h"
#include "mdrv_types.h"
#include "mdrv_mad_common.h"
#include "mdrv_mad_dvb2.h"
#include "mdrv_mad_process.h"
#include "mhal_mad_reg.h"

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

#define NUM_VOLUME_CURVE    (sizeof(AuVolumeMap)/2)
#define GET_MAP_VALUE(_X_,_Y_,_Z_)  ((u16)(_X_) * (_Y_) / (_Z_))
#define Newbasstreble

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#if (!USE_PARAM_POLLING)
#define SRS_PARAM_START_ADDR 0x2C30

MS_U8 Gain_TBL[][3]=
{
    {0x7F, 0xFF, 0xFF},  //0dB
    {0x72, 0x14, 0x81},  //-1dB
    {0x65, 0xAC, 0x8B},  //-2dB
    {0x5A, 0x9D, 0xF6},  //-3dB
    {0x50, 0xC3, 0x35},  //-4dB
    {0x47, 0xFA, 0xCC},  //-5dB
    {0x40, 0x26, 0xE6},  //-6dB
    {0x39, 0x2C, 0xED},  //-7dB
    {0x32, 0xF5, 0x2C},  //-8dB
    {0x2D, 0x6A, 0x86},  //-9dB
    {0x28, 0x7A, 0x26},  //-10dB
    {0x24, 0x13, 0x46},  //-11dB
    {0x20, 0x26, 0xF2},  //-12dB
};

MS_U8 Gain_TBL_1[][3]=
{
    {0x40, 0x00, 0x00},  //0dB
    {0x47, 0xCF, 0x25},  //1dB
    {0x50, 0x92, 0x3B},  //2dB
    {0x5A, 0x67, 0x03},  //3dB
    {0x65, 0x6E, 0xE3},  //4dB
    {0x72, 0xCF, 0x53},  //5dB
    {0x7F, 0xFF, 0xFF},  //6dB
};

MS_U8 u8SRSControl = 0xE0;
#endif


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

extern U32 DSPMadBaseBufferAdr[2];
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

#define MAD_DEBUG_P1(msg) //msg
#define MAD_DEBUG_P2(msg) //msg
#define MAD_DEBUG_P3(msg) //msg
#define DBG_AUDIO_ERROR(msg) msg
//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSetMute
//  [Description]:
//      This routine is to set mute for processor module.
//      (vivaldi2)
//  [Arguments]:
//      path:   for path0 ~ path4
//      enable: 1:mute  0:unmute
//  [Return]:
//      None
//  [Doxygen]
/// This routine is to set mute for processor module.
/// @param path \b IN: audio path
/// @param enable \b IN:
///   - 0: unmute
///   - 1: mute
//
//*******************************************************************************

void MDrv_MAD_ProcessSetMute(U8 u8Path, BOOL bEnable)
{
    MAD_DEBUG_P1(printk("MDrv_MAD_ProcessSetMute:Path =0x%x, Enable = 0x%x\n", u8Path, bEnable ));

    switch(u8Path)
    {
        case AUDIO_T3_PATH_AUOUT0:
            if(bEnable)  // Mute
              MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT0_VOLUME, 0x80, 0x80);
            else        // UnMute
              MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT0_VOLUME, 0x80, 0x00);
            break;

        case AUDIO_T3_PATH_AUOUT1:
            if(bEnable)
              MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT1_VOLUME, 0x80, 0x80 );
            else
              MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT1_VOLUME, 0x80, 0x00 );
            break;

        case AUDIO_T3_PATH_AUOUT2:
            if(bEnable)
              MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT2_VOLUME, 0x80, 0x80);
            else
              MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT2_VOLUME, 0x80, 0x00);
            break;

        case AUDIO_T3_PATH_AUOUT3:
            if(bEnable)
              MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT3_VOLUME, 0x80, 0x80);
            else
              MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT3_VOLUME, 0x80, 0x00);
            break;

        case AUDIO_T3_PATH_I2S:
            if(bEnable) //TEMP, MORRIS hard code for test.
              MHal_MAD_WriteMaskByte(REG_SOUND_I2S_VOLUME, 0x80, 0x80);
            else
              MHal_MAD_WriteMaskByte(REG_SOUND_I2S_VOLUME, 0x80, 0x00);
            break;

        case AUDIO_T3_PATH_SPDIF:
            if(bEnable)
              MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOLUME, 0x80, 0x80);
            else
              MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOLUME, 0x80, 0x00);
            break;

        default:
	     DBG_AUDIO_ERROR(printk("[ERROR] MDrv_MAD_ProcessSetMute: Wrong audio out path [0x%x]\r\n", u8Path));
            break;
    }
}
EXPORT_SYMBOL(MDrv_MAD_ProcessSetMute);

U16  AuVolumeMap[101] = //For T2
{
      0x7F00, 0x7E07, 0x7E06, 0x7E05, 0x7E04, 0x7E03, 0x7E02, 0x7E01,
      0x7E00, 0x7D07, 0x7D06, 0x7D05, 0x7D04, 0x7D03, 0x7D02, 0x7D01,
      0x7D00, 0x6C07, 0x6C06, 0x6C05, 0x6C04, 0x6C03, 0x6C02, 0x6C01,
      0x6C00, 0x6B07, 0x6B06, 0x6B05, 0x6B04, 0x6B03, 0x6B02, 0x6B01,
      0x6B00, 0x5A07, 0x5A06, 0x5A05, 0x5A04, 0x5A03, 0x5A02, 0x5A01,
      0x5A00, 0x5907, 0x5906, 0x5905, 0x5904, 0x5903, 0x5902, 0x5901,
      0x5900, 0x4807, 0x4806, 0x4805, 0x4804, 0x4803, 0x4802, 0x4801,
      0x4800, 0x4707, 0x4706, 0x4705, 0x4704, 0x4703, 0x4702, 0x4701,
      0x4700, 0x3607, 0x3606, 0x3605, 0x3604, 0x3603, 0x3602, 0x3601,
      0x3600, 0x3507, 0x3506, 0x3505, 0x3504, 0x3503, 0x3502, 0x3501,
      0x3500, 0x2407, 0x2406, 0x2405, 0x2404, 0x2403, 0x2402, 0x2401,
      0x2400, 0x2307, 0x2306, 0x2305, 0x2304, 0x2303, 0x2302, 0x2301,
      0x2200, 0x2107, 0x2106, 0x1B00,0x1000
};

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSetVolume
//  [Description]:
//      This routine is to set the volume for processor module.
//  [Arguments]:
//      path:   only for path0 & path1
//      volume: volume level (see spec.)
//  [Return]:
//      None
//  [Doxygen]
/// This routine is to set the volume for processor module.
/// @param path \b IN: audio path
/// @param volume \b IN: volume level
//
//*******************************************************************************

void MDrv_MAD_ProcessSetVolume(U16 path, U16 volume)
{
    U16 u16Vol1 = 0,u16Vol2 = 0;
    u16Vol1 = 0xFF00 & volume ;
    u16Vol1 >>= 8 ;
    u16Vol2 = 0x00FF & volume ;

    MAD_DEBUG_P1(printk("MDrv_MAD_ProcessSetVolume :: path = %x, volume = %x\n", path, volume));
    MAD_DEBUG_P1(printk("MDrv_MAD_ProcessSetVolume :: value1 = %x, value2 = %x\n", u16Vol1, u16Vol2));

     switch(path)
    {
    case AUDIO_T3_PATH_AUOUT0:
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT0_VOLUME, 0x7F, (U8)u16Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT0_VOL_FRAC, 0xE0, (U8)(u16Vol2<<5));
/* crazyrun 20091209 :: add Mstar's patch for avoiding SIF&Infinite Sound ON noise problem */		
//        MHal_MAD_WriteMaskByte(0x2D22, 0x01, 0x01);   // bEnable DSP CH1 sound effect
        break;

    case AUDIO_T3_PATH_AUOUT1:
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT1_VOLUME, 0x7F, (U8)u16Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT1_VOL_FRAC, 0xE0, (U8)(u16Vol2<<5));
        MHal_MAD_WriteMaskByte(0x2D22, 0x02, 0x02);   // bEnable DSP CH2 sound effect
        break;

    case AUDIO_T3_PATH_AUOUT2:
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT2_VOLUME, 0x7F, (U8)u16Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT2_VOL_FRAC, 0xE0, (U8)(u16Vol2<<5));
        MHal_MAD_WriteMaskByte(0x2D22, 0x04, 0x04);   // bEnable DSP CH3 sound effect
        break;

    case AUDIO_T3_PATH_AUOUT3:
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT3_VOLUME, 0x7F, (U8)u16Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT3_VOL_FRAC, 0xE0, (U8)(u16Vol2<<5));
/* crazyrun 20091209 :: add Mstar's patch for avoiding SIF&Infinite Sound ON noise problem */		
//        MHal_MAD_WriteMaskByte(0x2D22, 0x08, 0x08);   // bEnable DSP CH4 sound effect
        break;

    case AUDIO_T3_PATH_I2S:
        MHal_MAD_WriteMaskByte(REG_SOUND_I2S_VOLUME, 0x7F, (U8)u16Vol1);//TEMP, MORRIS hard code for test.
        MHal_MAD_WriteMaskByte(REG_SOUND_I2S_VOL_FRAC, 0xE0, (U8)(u16Vol2<<5));
        MHal_MAD_WriteMaskByte(0x2D22, 0x10, 0x10);   // bEnable DSP CH5 sound effect
        break;

    case AUDIO_T3_PATH_SPDIF:
        MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOLUME, 0x7F, (U8)u16Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOL_FRAC, 0xE0, (U8)u16Vol2);
        MHal_MAD_WriteMaskByte(0x2D22, 0x20, 0x20);   // bEnable DSP CH6 sound effect
        break;

    default:
	 DBG_AUDIO_ERROR(printk("[ERROR] MDrv_MAD_ProcessSetVolume: Wrong audio out path [0x%x]\r\n", path));
        break;
    }

//	printk("MDrv_MAD_ProcessSetVolume :: 0x2D8C = %x, 0x2DA8 = %x\n", MHal_MAD_ReadReg(0x2D8C), MHal_MAD_DecReadReg(0x2DA8));

}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessAbsoluteVolume
//  [Description]:
//      This routine is to set the absolute volume for processor module.
//  [Arguments]:
//      path:   audio path
//      vol1:   range from 0x00 to 0x7E , gain: +12db to   -114db (-1 db per step)
//      vol2:   range from 0x00 to 0x07 , gain:  -0db to -0.875db (-0.125 db per step)
//  [Return]:
//      None
//  [Doxygen]
/// This routine is to set the absolute volume for processor module.
/// @param path \b IN: audio path
/// @param vol1 \b IN: range from 0x00 to 0x7E , gain: +12db to   -114db (-1 db per step)
/// @param vol2 \b IN: range from 0x00 to 0x07 , gain:  -0db to -0.875db (-0.125 db per step)
//
//*******************************************************************************

void MDrv_MAD_ProcessAbsoluteVolume(MS_U8 u8Path, MS_U8 u8u8Vol1, MS_U8 u8u8Vol2)
{

//	printk("MDrv_MAD_ProcessAbsoluteVolume :: path = %x, vol1 = %x, vol2 = %x", path, vol1, vol2);

    switch(u8Path)
    {
    case AUDIO_T3_PATH_AUOUT0:
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT0_VOLUME, 0x7F, u8u8Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT0_VOL_FRAC, 0xE0, (u8u8Vol2<<5));
        MHal_MAD_WriteMaskByte(0x2D22, 0x01, 0x01);   // bEnable DSP CH1 sound effect
        break;

    case AUDIO_T3_PATH_AUOUT1:
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT1_VOLUME, 0x7F, u8u8Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT1_VOL_FRAC, 0xE0, (u8u8Vol2<<5));
        MHal_MAD_WriteMaskByte(0x2D22, 0x02, 0x02);   // bEnable DSP CH2 sound effect
        break;

    case AUDIO_T3_PATH_AUOUT2:
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT2_VOLUME, 0x7F, u8u8Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT2_VOL_FRAC, 0xE0, (u8u8Vol2<<5));
        MHal_MAD_WriteMaskByte(0x2D22, 0x04, 0x04);   // bEnable DSP CH3 sound effect
        break;

    case AUDIO_T3_PATH_AUOUT3:
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT3_VOLUME, 0x7F, u8u8Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_AUOUT3_VOL_FRAC, 0xE0, (u8u8Vol2<<5));
        MHal_MAD_WriteMaskByte(0x2D22, 0x08, 0x08);   // bEnable DSP CH4 sound effect
        break;

    case AUDIO_T3_PATH_I2S:
        MHal_MAD_WriteMaskByte(REG_SOUND_I2S_VOLUME, 0x7F, u8u8Vol1);//TEMP, MORRIS hard code for test.
        MHal_MAD_WriteMaskByte(REG_SOUND_I2S_VOL_FRAC, 0xE0, (u8u8Vol2<<5));
        MHal_MAD_WriteMaskByte(0x2D22, 0x10, 0x10);   // bEnable DSP CH5 sound effect
        break;

    case AUDIO_T3_PATH_SPDIF:
        MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOLUME, 0x7F, u8u8Vol1);
        MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOL_FRAC, 0xE0, u8u8Vol2);
        MHal_MAD_WriteMaskByte(0x2D22, 0x20, 0x20);   // bEnable DSP CH6 sound effect
        break;

    default:
	 DBG_AUDIO_ERROR(printk("[ERROR] MDrv_MAD_ProcessAbsoluteVolume: Wrong audio out path [0x%x]\r\n", u8Path));
        break;
    }

}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSPDIFVolume
//  [Description]:
//      This routine is to set the CH4 volume for SPDIF.
//  [Arguments]:
//      volume:   range from 0x00 to 0x7E , gain: +24db to   -102db (-1 db per step)
//  [Return]:
//      None
//  [Doxygen]
/// This routine is to set the absolute volume for processor module.
/// @param volume \b IN: range from 0x00 to 0x7E , gain: +24db to   -102db (-1 db per step)
//
//*******************************************************************************
void MDrv_MAD_ProcessSPDIFVolume(U16 volume)
{
    U16 value1 = 0,value2 = 0;

    value1 = 0x7F00 & volume ;
    value1 >>= 8 ;
    value2 = 0x0007 & volume ;

    MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOLUME, 0x7F, value1);
    MHal_MAD_WriteMaskByte(REG_SOUND_SPDIF_VOL_FRAC, 0xE0, (value2<<5));
    MHal_MAD_WriteMaskByte(0x2D22, 0x20, 0x20);   // bEnable DSP CH6 sound effect

}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSetBalance
//  [Description]:
//      This routine is to set the balance for processor module.
//  [Arguments]:
//      path:       for path0 & path1 only
//      balance:    BIT7: L(0)/R(1), BIT0-BIT3: level (dB)
//  [Return]:
//      None
//  [Doxygen]
/// This routine is to set the balance for processor module.
/// @param path \b IN: audio path
/// @param balance \b IN: balance level
//
//*******************************************************************************
void MDrv_MAD_ProcessSetBalance(U8 path, U16 balance)
{
    U16 value_l = 0, value_r = 0;

//    if(path != AUDIO_PATH_MAIN)
//        return;

#if 1 //By YW Jung LGE 2008/09/05
	value_l = (balance&0xFF00)>>8;
	value_r = (balance&0x00FF);
#else
    if(balance==50)
    {
        value_l = 0x00;
        value_r = 0x00;
    }
    else if(balance<50)
    {
        value_l = 0x00;
        value_r = GET_MAP_VALUE((50-balance),0xFF,50);
    }
    else if(balance>50)
    {
        value_l = GET_MAP_VALUE((balance-50),0xFF,50);
        value_r = 0x00;
    }
#endif

    MAD_DEBUG_P3(printk("Balance:%d",balance));
    MAD_DEBUG_P3(printk("==>(%x,",value_l));
    MAD_DEBUG_P3(printk("%x)\r\n",value_r));

    MHal_MAD_WriteByte(REG_SOUND_BALANCEL, value_l);
    MHal_MAD_WriteByte(REG_SOUND_BALANCER, value_r);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSetBass (Not Supported)
/// @brief \b Function \b Description: This routine is used to set Bass level.
/// @param u8Bass      \b :Bass Level (0~100)
///                   U3 & T2==>mapping to -16 ~ +15 dB Gain
///                           T3==>mapping to -12 ~ +12 dB Gain
//*******************************************************************************
void MDrv_MAD_ProcessSetBass(u8 u8Bass)
{
    u8 u8Value=0;

    MAD_DEBUG_P1(printk("MDrv_MAD_ProcessSetBass(%d)\n",u8Bass));
#ifdef Newbasstreble
	MHal_MAD_WriteByte(REG_SOUND_BASS, u8Bass);
#else
#if 0 //crazyrun :: modify treble/bass level as (-6db ~ 6db), it should be deleted after releasing path from Mstar.
    if(u8Bass==100)
    {
        u8Value = 0x30;
    }
    else
    {
        u8Value = ((int)u8Bass-50)*48/50;
    }
#else
	u8Value = ((int)u8Bass-50)*48/100;
#endif

    MHal_MAD_WriteByte(REG_SOUND_BASS, u8Value);
#endif
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSetTreble  (Not Supported)
/// @brief \b Function \b Description: This routine is used to set Treble level.
/// @param u8Treble    \b :Treble Level (0~100)
///                   U3 & T2==>mapping to -16 ~ +15 dB Gain
///                           T3==>mapping to -12 ~ +12 dB Gain
//*******************************************************************************
void MDrv_MAD_ProcessSetTreble(u8 u8Treble)
{
    u8 u8Value=0;

    MAD_DEBUG_P1(printk("MDrv_MAD_ProcessSetTreble(%d)\n",u8Treble));
#ifdef Newbasstreble
	MHal_MAD_WriteByte(REG_SOUND_TREBLE, u8Treble);
#else
#if 0 //crazyrun :: modify treble/bass level as (-6db ~ 6db), it should be deleted after releasing path from Mstar.
    if(u8Treble==100)
    {
        u8Value = 0x30;
    }
    else
    {
        u8Value = ((int)u8Treble-50)*48/50;
    }
#else
	u8Value = ((int)u8Treble-50)*48/100;
#endif

    MHal_MAD_WriteByte(REG_SOUND_TREBLE, u8Value);
#endif
}

void MDrv_MAD_ProcessSetBBE(B16 enable)
{
#if 0//CHECK
    if(enable)
    {
        MHal_MAD_SeWriteRegMask(0x2D30,0x0080,0x0080);
        MHal_MAD_SeWriteRegMask(0x2D32,0x000C,0x0008);    // [2] = 1, BBE mode
    }
    else
    {
        MHal_MAD_SeWriteRegMask(0x2D30,0x0080,0x0000);
        MHal_MAD_SeWriteRegMask(0x2D32,0x000C,0x0000);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SOUND_SetPara()
/// @brief \b Function \b Description:  This function is used to write paramters value into SRAM
/// @param <IN>        \b addr1:  middle byte of address
/// @param <IN>        \b addr2:  low byte of address
/// @param <IN>        \b value1: high byte of value
/// @param <IN>        \b value2: middle byte of value
/// @param <IN>        \b value3: low byte of value
/// @param <OUT>        \b NONE :
/// @param <RET>       \b NONE :
/// @param <GLOBAL>    \b NONE :
////////////////////////////////////////////////////////////////////////////////
void  MDrv_MAD_SOUND_SetPara(MS_U8 u8Addr1 , MS_U8 u8Addr2, MS_U8 u8Value1, MS_U8 u8Value2, MS_U8 u8Value3)
{
    MS_U16 addr = (((MS_U8)u8Addr1 << 8) | (MS_U8)u8Addr2);
    MS_U32 value = (((MS_U8)u8Value1 << 16) | ((MS_U8)u8Value2 << 8) | (MS_U8)u8Value3);
    MDrv_MAD2_Write_DSP_sram(addr, value, DSP_MEM_TYPE_DM);//MDrv_MAD2_Write_DSP_sram(addr, value, DSP_MEM_TYPE_DM);
}

//******************************************************************************
//  [Function Name]:    MDrv_MAD_SetSRSPara
//  [Description]:      This routine .
//  [Arguments]:        None
//  Cmd_TSInputGain      1
//  Cmd_FocusElevation   2
//  Cmd_TruBassInputGain 3
//  Cmd_TruBassSpeak     4
//  Cmd_enTrubass        5
//  Cmd_enDialog_clarity 6
//  Cmd_enTruSurroundHD  7
//  Cmd_Inputmode        8
//*******************************************************************************

void    MDrv_MAD_SetSRSPara(U8 u8mode, U8 u8value)
{
#if 0 //CHECK
#if (USE_PARAM_POLLING)
    switch (u8mode)
    {
          case 0: //TS Gain
              if(u8value > 12) u8value = 12;
              MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x10+u8value);
              timer_delayms(10);
              MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x00);
         break;
         case 1: //FocusElevation
             if(u8value > 12) u8value = 12;
             MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x20+u8value);
             timer_delayms(10);
             MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x00);
         break;
         case 2: //TruBassGain
             if(u8value > 12) u8value = 12;
             MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x30+u8value);
             timer_delayms(10);
             MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x00);
         break;
         case 5: //Cmd_Inputmode
             //if(u8value > 2) u8value = 2;
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x80+u8value);
            timer_delayms(10);
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x00);
         break;
      }
#else
        case 0: //TS Gain
              if(u8value > 12) u8value = 12;
              MDrv_MAD_SOUND_SetPara(SrsDmAddr,0x80,Gain_TBL[u8value][0],Gain_TBL[u8value][1],Gain_TBL[u8value][2]);
              break;
          case 1: //FocusElevation
              if(u8value > 12) u8value = 12;
              MDrv_MAD_SOUND_SetPara(SrsDmAddr,0x81,Gain_TBL[u8value][0],Gain_TBL[u8value][1],Gain_TBL[u8value][2]);
              break;
          case 2: //TruBassGain
              if(u8value > 12) u8value = 12;
              MDrv_MAD_SOUND_SetPara(SrsDmAddr,0x82,Gain_TBL[u8value][0],Gain_TBL[u8value][1],Gain_TBL[u8value][2]);
              break;
          case 3: //SpeakSize
              if(u8value > 3) u8value = 3;
              MDrv_MAD_SOUND_SetPara(SrsDmAddr,0x83,0,0,u8value*12);
              break;
          case 4: //Input mode
              if(u8value > 2) u8value = 2;
              MDrv_MAD_SOUND_SetPara(SrsDmAddr,0x84,0,0,u8value);
              break;
          case 7: //Output Gain
              if(u8value > 6) u8value = 6;
              MDrv_MAD_SOUND_SetPara(SrsDmAddr,0x87,Gain_TBL_1[u8value][0],Gain_TBL_1[u8value][1],Gain_TBL_1[u8value][2]);
              break;
#endif
#endif
}

#if (!USE_PARAM_POLLING)
void MDrv_MAD_SetSRSPara32(U8 u8mode, U32 value)
{
#if 0 //CHECK
    switch (u8mode)
    {
         case 0: //TS Gain
            MDrv_MAD_AuProcessWritePARAMETER(SRS_PARAM_START_ADDR,value);
            break;
         case 1: //FocusElevation
            MDrv_MAD_AuProcessWritePARAMETER(SRS_PARAM_START_ADDR+1,value);
            break;
         case 2: //TruBassGain
            MDrv_MAD_AuProcessWritePARAMETER(SRS_PARAM_START_ADDR+2,value);
            break;
         case 3: //TruBassSpeak
            MDrv_MAD_AuProcessWritePARAMETER(SRS_PARAM_START_ADDR+3,value);
            break;
         case 4: //Inputmode
            MDrv_MAD_AuProcessWritePARAMETER(SRS_PARAM_START_ADDR+4,value);
            break;
         case 7: //TSOutputGain
            MDrv_MAD_AuProcessWritePARAMETER(SRS_PARAM_START_ADDR+7,value);
            break;
    }
#endif
}
#endif

//******************************************************************************
//  [Function Name]:    MDrv_MAD_SRS_TruBass
//  [Description]:      This routine .
//  [Arguments]:        None
//*******************************************************************************
void    MDrv_MAD_SRS_TruBass(U8 u8SRSenTruBass)
{
#if 0 //CHECK
#if (USE_PARAM_POLLING)
    if(u8SRSenTruBass)
    {
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x51);
            timer_delayms(10);
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x00);
    }
    else
    {
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x50);
            timer_delayms(10);
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x00);
    }
#else
    if (u8SRSenTruBass)
        u8SRSControl |= 0x80;
    else
        u8SRSControl &= 0x7F;
    MDrv_MAD_SOUND_SetPara(SrsDmAddr,0x86,0,u8SRSControl,0);  // [7] = 0, TruBass disable
#endif
#endif
}

//******************************************************************************
//  [Function Name]:    MDrv_MAD_SRS_DC
//  [Description]:      This routine .
//  [Arguments]:        None
//*******************************************************************************
void    MDrv_MAD_SRS_DC(U8 u8SRSenDC)
{
#if 0 //CHECK
#if (USE_PARAM_POLLING)
    if(u8SRSenDC)
    {
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x61);
            timer_delayms(10);
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x00);
    }
    else
    {
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x60);
            timer_delayms(10);
            MHal_MAD_SeWriteRegMask(0x2D32,0xFF,0x00);
    }
#else
    if (u8SRSenDC)
        u8SRSControl |= 0x20;
    else
        u8SRSControl &= 0xDF;
    MDrv_MAD_SOUND_SetPara(SrsDmAddr,0x86,0,u8SRSControl,0);  // [5] = 0, Dialog Clarity disable
#endif
#endif
}

void MDrv_MAD_ProcessSetSRS(B16 enable)
{
#if 0 //CHECK
    if (enable)
    {
        MHal_MAD_SeWriteRegMask(0x2D30,0x8F,0x82);
        MDrv_MAD_SRS_TruBass(1);
        MDrv_MAD_SRS_DC(1);
    }
    else
    {
        MHal_MAD_SeWriteRegMask(0x2D30,0x8F,0x80);
    }
#endif
}

void MDrv_MAD_AuProcessWritePARAMETER(U16 dsp_addr, U32 value)
{
#if 0 //CHECK
    U16  dat1,dat2;

    //MHal_MAD_SeWriteRegMask(0x3CEC, 0x0040, 0x0000);          //DMA sel to RIU
    MHal_MAD_AbsWriteMaskByte(0x103C14, 0x01, 0x00);

    dat2 = (U16)(value>>8);
    dat1 = (U16)(value & 0x000000FF);

    dsp_addr = dsp_addr | 0x4000;
    MHal_MAD_SeWriteReg( REG_SE_IDMA_WRBASE_ADDR_L, dsp_addr );
    //msleep(10);//20090205 mstar remove
    MHal_MAD_SeWriteRegMask(REG_SE_IDMA_CTRL0, 0x0004,0x0000);

    MHal_MAD_SeWriteReg(REG_SE_DSP_BRG_DATA_L,dat2);

    MHal_MAD_SeWriteReg(REG_SE_DSP_BRG_DATA_L,dat1);
#endif
}

void MDrv_MAD_AuProcessWritePARAMETER_PM(U16 dsp_addr, U32 value)
{
#if 0 //CHECK
    U8  dat[3];
    U16 tmpData;

    tmpData = MHal_MAD_SeReadReg(0x3CEC);

    //MHal_MAD_SeWriteRegMask(0x3CEC, 0x0040, 0x0000);          //DMA sel to RIU
    MHal_MAD_AbsWriteMaskByte(0x103C14, 0x01, 0x00);
    //MHal_MAD_SeWriteRegMask(0x3CEC, 0x0040, 0x0000);          //DMA sel to RIU
    MHal_MAD_AbsWriteMaskByte(0x103C14, 0x01, 0x00);

    dat[2] = H2BYTE(value);
    dat[1] = HIBYTE(value);
    dat[0] = LOBYTE(value);

    MHal_MAD_SeWriteReg( REG_SE_IDMA_WRBASE_ADDR_L, dsp_addr);
    //msleep(10);// prevent the parallel IDMA write tasks collision
    MHal_MAD_SeWriteRegMask(REG_SE_IDMA_CTRL0, 0x0004,0x0000);

    MHal_MAD_SeWriteReg(REG_SE_DSP_BRG_DATA_L, dat[1] | (dat[2] <<8));
    if (MDrv_MAD_SeDSP_chkIdamReady(AUD_CHK_DSP_WRITE_RDY)==FALSE) {
        printk("\r\n chkIdamReady error 1\r\n");
    }

    MHal_MAD_SeWriteReg(REG_SE_DSP_BRG_DATA_L, dat[0]);
    if (MDrv_MAD_SeDSP_chkIdamReady(AUD_CHK_DSP_WRITE_RDY)==FALSE) {
        printk("\r\n chkIdamReady error 2\r\n");
    }
#endif
}


BOOL MDrv_MAD_SeSystemLoadCode( void )
{
    MS_U16 time_out = 0;

    MDrv_MAD2_SetDspIDMA();

    MAD_DEBUG_P1(printk("MDrv_AUDIO_SeSystemLoadCode() \r\n"));

    MDrv_MAD_DspLoadCode(AU_SE_SYSTEM);

    //Wait Dsp init finished Ack
    while(time_out++<100) {
        if(MDrv_MAD2_GetLoadCodeAck() == 0xE3)
            break;
        msleep(2);
    }
    if(time_out>=100) {
        DBG_AUDIO_ERROR(printk("audio DSP_SE Re-Active\n"));
    }
    else {
     DBG_AUDIO_ERROR(printk("audio DSP_SE LoadCode success..\n"));
    }

    //inform DSP to start to run
    MDrv_MAD2_SetMcuCmd(0xF3);

    return TRUE;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessSetADAbsoluteVolume
//  [Description]:
//      This routine is to set the absolute volume for AD module.
//  [Arguments]:
//      vol1:   range from 0x0C to 0x7E , gain: +12db to   -114db (-1 db per step)
//  [Return]:
//      None
//  [Doxygen]
/// This routine is to set the absolute volume for AD module.
/// @param vol1 \b IN: range from 0x00 to 0x7E , gain: +12db to   -114db (-1 db per step)
//
//*******************************************************************************
void MDrv_MAD_ProcessSetADAbsoluteVolume(U8 vol1, U8 vol2)
{
    MHal_MAD_WriteMaskByte(0x2DD9, 0x07, (vol1&0xE0)>>5);
    MHal_MAD_WriteMaskByte(0x2DD8, 0xFF, (((vol1&0x1F)<<3) | (vol2&0x07)) );
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessADSetMute
//  [Description]:
//      This routine is to set mute for AD module.
//  [Arguments]:
//      enable: 1:mute  0:unmute
//  [Return]:
//      None
//
//*******************************************************************************
void MDrv_MAD_ProcessADSetMute(BOOL enable)
{
    if(enable)  // Mute
    {
        MAD_DEBUG_P1(printk("	====== AD Mute Enable ======\r\n"));//TEMP, MORRIS
        MHal_MAD_WriteMaskReg(REG_SOUND_AD_VOLUME, 0x0400, 0x0400);
    }
    else
    {// UnMute
        MAD_DEBUG_P1(printk("	====== AD UnMute ======\r\n"));
        MHal_MAD_WriteMaskReg(REG_SOUND_AD_VOLUME, 0x0400, 0x0000);
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_ProcessSetCH1AudioDelay(U32 delay)
//  [Description]:
//      This routine sets the value of audio delay.
//  [Arguments]:
//      delay :0x14~0xC8(20~200), in ms unit
//*******************************************************************************
void MDrv_MAD_ProcessSetCH1AudioDelay(U32 delay)
{
    if (delay > 0xC8) {
        DBG_AUDIO_ERROR(printk("delay should be less than 200ms. \r\n"));
        return;
    }

    MDrv_MAD2_Write_DSP_sram(0x1000, delay, DSP_MEM_TYPE_PM);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_SetSPDIFAudioDelay(U32 delay)
//  [Description]:
//      This routine sets the value of audio delay.
//  [Arguments]:
//      delay :0x14~0xC8(20~200), in ms unit
//*******************************************************************************
void MDrv_MAD_SetSPDIFAudioDelay(U32 delay)
{
    if (delay > 0xC8) {
        DBG_AUDIO_ERROR(printk("delay should be less than 200ms. \r\n"));
        return;
    }

    MDrv_MAD2_Write_DSP_sram(0x1001, delay, DSP_MEM_TYPE_PM);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ProcessLoadCode
//  [Description]:
//      This routine load the Processor Binary code.
//  [Arguments]:
//      None
//  [Return]:
//      None
//
//*******************************************************************************
void MDrv_MAD_SIF_EnableDACOut(U8 path, BOOL enable)
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SIF_EnableDACOut:Path =0x%x, Enable = 0x%x\n", path, enable ));
#if 0//CHECK
    switch(path)
    {
        case AUDIO_PATH_0:
            if(enable)  // Enable
                MHal_MAD_WriteMaskReg(0x2CE8, 0x007, 0x0000);
            else    // Disable
                MHal_MAD_WriteMaskReg(0x2CE8, 0x007, 0x0001);
            break;

        case AUDIO_PATH_1:
            if(enable)  // Enable
                MHal_MAD_WriteMaskReg(0x2CE8, 0x007, 0x0000);
            else    // Disable
                MHal_MAD_WriteMaskReg(0x2CE8, 0x007, 0x0002);
            break;

        case AUDIO_PATH_2:
            if(enable)  // Enable
                MHal_MAD_WriteMaskReg(0x2CE8, 0x007, 0x0000);
            else    // Disable
                MHal_MAD_WriteMaskReg(0x2CE8, 0x007, 0x0004);
            break;

        default:
            break;
    }
#endif
}

