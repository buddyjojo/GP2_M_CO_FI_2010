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
/// file    mdrv_mad_dvb.c
/// @brief  MAD Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#define _AUCOMMON_C_

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/mm.h>
#include "mst_devid.h"
#include "mst_utility.h"
#include "mdrv_mad_common.h"
#include "mdrv_mad_process.h"
#include "mhal_mad_reg.h"
#include "mdrv_mad_dvb.h"
#include "mdrv_mad_dvb2.h"
#include "mdrv_mad_sif.h"

#define ES_ADDR (DSPMadBaseBufferAdr[DSP_DEC] + 0xA0000000)

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
u16 Dvb_MemInfo[6][2]=
{
                          // <addr, length>l in Line unit
    {0x0000, 0x000E},     // 0: ES1 addr, length  // 30K
    {0x0008, 0x0A00},     // 1: Cache addr, legth
    {0x0012, 0x0600},     // 2: Prefetch addr, length
    {0x00F0, 0x000F},     // 3: ES2 addr, length
    {0x0020, 0x1000},     // 4: Sound Effect addr, length
    {0x0078, 0x0037},     // 5: PCM1 addr, length
};

U16 MpegBitRateTable[6][16] =
{
    // V1, L1
    {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0},
    // V1, L2
    {0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
    // V1, L3
    {0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0},
    // V2, L1
    {0, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0},
    // V2, L2
    {0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0},
    // V2, L3
    {0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0},
};

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
extern U8 dtv_mode_en;
extern spinlock_t                      _mad_spinlock;
extern BOOL audio_recovery ;
//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------

static U8 u8EncTag = 0;


//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------
extern U32 DSPMadBaseBufferAdr[2];
extern U32 _BT_RunningUp;
extern MDRV_BT_T BT_information;
extern U32 _BT_Count;
extern U32 EncFrameIdx;
extern U32 EncBuf_W_idx;
extern U32 EncBuf_R_idx;
extern U8   EncBuf_Count;
extern MPEG_EN_FRAME_INFO MPEG_EN_BUF[6];
extern U8   Enc_InProcess;



//--------------------------------------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------------------------------------
#define MAD_DEBUG_P1(msg) //msg
#define MAD_DEBUG_P2(msg) //msg
#define MAD_DEBUG_P3(msg) //msg
#define DBG_AUDIO_ERROR(msg) msg

//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_DvbInit
//  [Description]:
//      This routine is the initialization for DVB module.
//  [Arguments]:
//      None
//  [Return]:
//      None
//
//*******************************************************************************
void MDrv_MAD_DvbInit(void)
{
    U8 u8codeTypeDSP = MDrv_MAD_GetDSPCodeType();
    MDrv_MAD_LoadCode(u8codeTypeDSP);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_SetMemInfo
//  [Description]:
//      Set memory information
//  [Arguments]:
//      None
//  [Return]:
//      None
//  [for Doxygen]
/// This routine is to set memory for ES buffer and PCM buffer.
/// @param InputBufferStartAddr \b IN: the start address of ES buffer
/// @param InputBufferEndAddr \b IN: the end address of ES buffer
/// @param OutputBufferStartAddr \b IN: the start address of PCM buffer
/// @param OutputBufferEndAddr \b IN: the end address of PCM buffer
//
//*******************************************************************************
void MDrv_MAD_SetMemInfo(void)
{
    MS_U16 DDR_ADDR;

    MAD_DEBUG_P1(printk("DSPMadBaseBufferAdr[DSP_DEC] HAL_MAD_SetMemInfo = 0x%x\n", DSPMadBaseBufferAdr[DSP_DEC]));
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x00FF, 0x0000);                //reset AUD register
    {
        // switch the MAD_BASE_ADDR control to MCU
        MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0001, 0x0000);

        // ICACHE BASE
        DDR_ADDR =(MS_U16) (MDrv_MAD_GetDspMadBaseAddr(DSP_DEC) >> 12);
        MHal_MAD_WriteReg(REG_DEC_DSP_ICACHE_BASE_L,DDR_ADDR);

        // MAD BASE
        MHal_MAD_WriteReg(REG_DEC_MAD_OFFSET_BASE_H,DDR_ADDR);

        // Config as ES1 (DTV)
        MHal_MAD_WriteMaskReg(REG_DEC_MCFG, 0x00FF, 0x0002);
        MHal_MAD_WriteReg(REG_DEC_MBASE_H,0x0000);                    // ES1 base : 0x004000
        MHal_MAD_WriteMaskReg(REG_DEC_MSIZE_H, 0xFF00, 0x0F00);       // ES1 size : 64KB

        // Config as PCM1
        MHal_MAD_WriteMaskReg(REG_DEC_MCFG,0x00FF, 0x0004);
        MHal_MAD_WriteReg(REG_DEC_MBASE_H,0x00BA);                    // PCM1 base : 0x00BA00
        MHal_MAD_WriteMaskReg(REG_DEC_MSIZE_H, 0xFF00, 0x3700);       // PCM1 size : 448KB  // only can define to 512KB //MStar modified for ms10

        // Config as PCM2
        MHal_MAD_WriteMaskReg(REG_DEC_MCFG,0x00FF, 0x0005);
        MHal_MAD_WriteReg(REG_DEC_MBASE_H,0x012A);                    // PCM2 base : 0x012A00
        MHal_MAD_WriteMaskReg(REG_DEC_MSIZE_H, 0xFF00, 0x0700);       // PCM2 size : 32KB

        //MStar modified for ms10 DDC
        MHal_MAD_WriteMaskReg(REG_DEC_MCFG, 0x00FF, 0x0003);
        MHal_MAD_WriteReg(REG_DEC_MBASE_H,0x0010);                    // ES2 base :  0x001000
	    MHal_MAD_WriteMaskReg(REG_DEC_MSIZE_H, 0xFF00, 0x0700);       // ES2 size : 32K

        // Config as SPDIF non-PCM, Don't delete and move
        //HAL_AUDIO_DecWriteByte(0x2DEA, 0x0005);
        //HAL_AUDIO_DecWriteByte(0x2DE6, 0x008F);     // SPDIF non-PCM base : 0x005000
        //HAL_AUDIO_DecWriteByte(0x2DE7, 0x0000);     // SPDIF non-PCM base :
        //HAL_AUDIO_DecWriteByte(0x2DE9, 0x0037);     // SPDIF non-PCM size : 114KB

    }

    // Reset MAD module
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0040);
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0080, 0x0080);
    //MsOS_DelayTask(5);

    // Set MAD module
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0080, 0x0000);
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0000);

    MAD_DEBUG_P2(printk("ICACHE addr = %04x\r\r", MHal_MAD_ReadReg(REG_DEC_DSP_ICACHE_BASE_L)));
    MAD_DEBUG_P2(printk("MAD Base addr = %04x\r\r", MHal_MAD_ReadReg(REG_DEC_MAD_OFFSET_BASE_H)));
}

//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_DvbGetSoundMode
//  [Description]:
//      This routine is to get the sound mode (Stereo/LL/RR/Mute) for DVB module.
//  [Arguments]:
//      None
//  [Return]:
//
//  [for Doxygen]
/// This routine is to get the sound mode (Stereo/LL/RR/Mute) for DVB module.
/// @return sound mode (Stereo/LL/RR/Mute)
//
//*******************************************************************************
U16 MDrv_MAD_DvbGetSoundMode(void)
{
    MS_U16 soundMode;

    soundMode = (MS_U8)MHal_MAD_ReadReg(REG_MB_MODE_SELECT);
    return (soundMode & 0x0003);
}

//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_SetSystem
//  [Description]:
//      Set DVB decoder system
//  [Arguments]:
//      None
//  [Return]:
//      TRUE / FALSE
//  [Doxygen]
/// Set DVB decoder system (MPEG/AC3/JPEG/TONE).
/// @param system_type \b IN:
///   - 0: MPEG
///   - 1: AC3
///   - 2: JPEG
///   - 3: TONE
//
//*******************************************************************************
U8 MDrv_MAD_SetSystem(U8 system_type)
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SetSystem:(0x%x)\r\n",system_type));
    dtv_mode_en = 1;
    switch( system_type ) {
        case ADEC_SRC_TYPE_MPEG:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_MPEG);
            break;

        case ADEC_SRC_TYPE_AC3:
        case ADEC_SRC_TYPE_EAC3:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_AC3P);
            //MDrv_MAD_SPDIF_SetMode(2,0);  //20090117 for DVIX spdif output no sound problem
            break;

        case ADEC_SRC_TYPE_AAC:
        case ADEC_SRC_TYPE_HEAAC:
        case ADEC_SRC_TYPE_MS10_DDT:
            //MStar Mark(under development)
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_MS10_DDT);
            break;

        case ADEC_SRC_TYPE_MP3:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_MP3);
            break;

        case ADEC_SRC_TYPE_WMA:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_WMA);
            break;

        case ADEC_SRC_TYPE_CDLPCM:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_CDLPCM);
            break;

        case ADEC_SRC_TYPE_RA8LBR:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_RA8LBR);
            break;

        case ADEC_SRC_TYPE_XPCM:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_XPCM);
            break;

        case ADEC_SRC_TYPE_MPEG_EN:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_MPEG_EN);
            break;

        case ADEC_SRC_TYPE_MS10_DDC:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_MS10_DDC);
            break;

        case ADEC_SRC_TYPE_WMA_PRO:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_WMA_PRO);
            break;

       //Mstar add for Skype, 2009/09/22
       case ADEC_SRC_TYPE_G729:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_G729);
            break;
       case ADEC_SRC_TYPE_DTSBPS:
            MDrv_MAD_ReLoadCode(AU_DVB_STANDARD_DTSBPS);
            break;
        default:
            DBG_AUDIO_ERROR(printk("	[ERROR]MDrv_MAD_SetSystem::Unknown decoder type [0x%x]\r\n",system_type));
            break;

   }
    return 0;
}

//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_Dvb_setDecCmd
//  [Description]:
//      Set DVB decoder command
//  [Arguments]:
//      cmd type :
//          0: STOP
//          1: PLAY
//          2: RESYNC
//          4: PLAYFILE
//  [Return]:
//      None
//  [Doxygen]
/// Set DVB decoder command (STOP/PLAY/RESYNC/PLAYFILE).
/// @param decCmd \b IN:
///   - 0: STOP
///   - 1: PLAY
///   - 2: RESYNC
///   - 4: PLAYFILE
//
//*******************************************************************************
void MDrv_MAD_Dvb_setDecCmd(U8 decCmd)
{
        U16 SPDIF_Mod_Tmp;

        MAD_DEBUG_P1(printk("MDrv_MAD_Dvb_setDecCmd:(0x%x)\r\n",decCmd));

        if( (decCmd == AU_DVB_DECCMD_STOP) && (_BT_RunningUp == 1))
        {
            MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x00);  // stop upload BT
        }

        if((decCmd == 0x00) ||(decCmd == 0x06))
        {
#if 1//By YW Jung 2009.09.15
// Disabled by Jonghyuk, Lee 091230
//			MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_I2S,1);
#else
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_AUOUT0,1);
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_AUOUT1,1);
            //MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_AUOUT2,1);//SC1 out
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_AUOUT3,1);
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_I2S,1);
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_SPDIF,1);
#endif
            msleep(160);
        }

        MHal_MAD_WriteMaskReg(REG_DEC_DECODE_CMD, 0x001F, (U16)decCmd );

        if((decCmd == 0x00) ||(decCmd == 0x06))
        {
#if 1//By YW Jung 2009.09.15
// Disabled by Jonghyuk, Lee 091230
//           MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_I2S,0);
#else
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_AUOUT0,0);
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_AUOUT1,0);
            //MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_AUOUT2,0);//SC1 out
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_AUOUT3,0);
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_I2S,0);
            MDrv_MAD_ProcessSetMute(AUDIO_T3_PATH_SPDIF,0);
#endif
            msleep(15);
        }

        if( (decCmd == AU_DVB_DECCMD_PLAY) && (_BT_RunningUp == 1))
        {
            if(BT_information.bSBCOnOff == 1)
            {
                MDrv_MAD2_ReLoadCode(AU_DVB2_ADVSND_SBC);
                MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x03);      // start upload BT with SBC
                _BT_Count=1;
            }
            else
            {
                MHal_MAD_WriteMaskByte(REG_SOUND_CMD_BT,0x07,0x01);       // start upload BT with PCM
            }
        }

        SPDIF_Mod_Tmp = MHal_MAD_ReadReg(REG_M2D_MAILBOX_SPDIF_CTRL);
        if (decCmd == 0x01)                                         // Play Command check
        {
            MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0002,0x0000);          // Reset SPDIF nonPCM pointer when channel change by PCM mod
            msleep(10);
            MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0002,SPDIF_Mod_Tmp);   // Restore SPDIF Type setting
        }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_setDecCmd()
/// @brief \b Function \b Description: This routine is to set  the DVB1 decode command.
/// @param <IN>        \b u8DecCmd    :
///                                    0--STOP
///                                    1--PLAY
///                                    1--RESYNC
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetDecCmd(AU_DVB_DECCMD u8DecCmd)
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SetDecCmd:(0x%x)\r\n",u8DecCmd));

    MHal_MAD_WriteMaskByte(REG_DEC_DECODE_CMD, 0x0F,(MS_U8) u8DecCmd );
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_setEncCmd()       added by Kochien
/// @brief \b Function \b Description: This routine is to set  the DVB1 encode command.
/// @param <IN>        \b u8DecCmd    :
///                                    0--STOP
///                                    1--START
///    /// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_SetEncCmd(AU_DVB_ENCCMD u8EncCmd)       //kochien added
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SetEnccCmd:(%x)\r\n",u8EncCmd));

    if(u8EncCmd == AU_DVB_ENCCMD_START)
    {
        if(Enc_InProcess == 0)    // clean frame index and buffer count when first time encode start
        {
            EncFrameIdx = 0;
            u8EncTag = 0;
            EncBuf_W_idx = 0;
            EncBuf_R_idx = 0;
            EncBuf_Count = 0;
            MDrv_MAD_SetEncodeDoneFlag(0);
        }
        Enc_InProcess = 1;
    }
    else if(u8EncCmd == AU_DVB_ENCCMD_STOP)
        Enc_InProcess = 0;

    MHal_MAD_WriteMaskByte(REG_DEC_ENCODE_CMD, 0x0F,(MS_U8) u8EncCmd );
}

void MDrv_MAD_EncDataTakeNotification(void)       //kochien added
{
    u8EncTag++;

    MHal_MAD_WriteMaskByte(REG_MPG_EN_TAG_FROM_MIPS,0xFF,u8EncTag);

    MDrv_MAD_SendIntrupt(DSP_DEC,0xE1);

    MAD_DEBUG_P1(printk("MDrv_MAD_EncDataTakeNotification()......\n"));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_GetEncFrameInfo()
/// @brief \b Function \b Description: This routine is to get  the DVB1 decode command.
/// @param <OUT>
//
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_GetEncFrameInfo(U32 *Addr, U32 *Size, Audio_U64 *EncPts )
{
    // calculate encoder index
    //g_aud_auctx.pts = (((uint64_t)g_AudInfo.nFrameIndex-1)*1152) * MPEG_TS_CLOCKRATE / DEFAULT_AUDIO_FREQUENCY
/*
    *EncPts = (Audio_U64)(((EncFrameIdx-1)*1152*90)/48);
    // physical address
    *Addr = DSPMadBaseBufferAdr[DSP_DEC] +  (U32)MHal_MAD_ReadReg(REG_D2M_MAILBOX_ENC_LineAddr)*16*16; // line_addr -> byte addr
    *Size = (U32)MHal_MAD_ReadReg(REG_D2M_MAILBOX_ENC_LineSize)*16;        //line_size -> byte size
//    MDrv_MAD_EncDataTakeNotification();
*/
    *EncPts = MPEG_EN_BUF[EncBuf_R_idx].Frame_PTS; //= (Audio_U64)(((EncFrameIdx-1)*1152*90)/48);
    *Addr = MPEG_EN_BUF[EncBuf_R_idx].Frame_Addr;   //= (U32)DSPMadBaseBufferAdr[DSP_DEC] +  (U32)MHal_MAD_ReadReg(REG_D2M_MAILBOX_ENC_LineAddr)*16*16; // line_addr -> byte addr
    *Size = MPEG_EN_BUF[EncBuf_R_idx].Frame_Size;     // =  (U32)MHal_MAD_ReadReg(REG_D2M_MAILBOX_ENC_LineSize)*16;

    if(EncBuf_R_idx == 5)     //buffer size 6, index range 0 ~ 5
        EncBuf_R_idx = 0;
    else
        EncBuf_R_idx++;

    EncBuf_Count--;             // decrease buffer count

    if(EncBuf_Count == 0)
        MDrv_MAD_SetEncodeDoneFlag(0);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_GetEncBufInfo()
/// @brief \b Function \b Description: This routine is to get  the DVB1 decode command.
/// @param <OUT>
//
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_GetEncBufInfo(U32 *Addr, U32 *BufSize, U32 *FrameSize, U16 *FrameTime)
{
    // physical address
    *Addr = DSPMadBaseBufferAdr[DSP_DEC] +  0xAC800; //MHal_MAD_ReadReg(REG_D2M_MAILBOX_ENC_LineAddr)*16; // line_addr -> byte addr
    *BufSize = 0xD800;
    *FrameSize = 576;   //default input_frequency 48k, bit_rate 384k
    *FrameTime = 24;   //1152/48k = 24ms
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_GetDecCmd()
/// @brief \b Function \b Description: This routine is to get  the DVB1 decode command.
/// @param <OUT>        \b AU_DVB_DECCMD   :
//                                          AU_DVB_DECCMD_STOP,      //0
//                                          AU_DVB_DECCMD_PLAY,      //1
//                                          AU_DVB_DECCMD_PLAYFILETSP = 2,
//                                          AU_DVB_DECCMD_RESYNC,
//                                          ....etc
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
AU_DVB_DECCMD MDrv_MAD_GetDecCmd(void)
{
    MS_U16   deccmd_status;

    deccmd_status = MHal_MAD_ReadReg(REG_DEC_DECODE_CMD);
    return((AU_DVB_DECCMD)(deccmd_status&0x000F));
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_SetFreeRun
//  [Description]:
//      Set DVB free-run mode
//  [Arguments]:
//      cmd type :
//          0: normal AV sync
//          1: free-run mode
//  [Return]:
//      None
//  [Doxygen]
/// Set DVB free-run mode.
/// @param FreeRun \b IN:
///   - 0: normal AV sync
///   - 1: free-run mode
//
//*******************************************************************************
void MDrv_MAD_SetFreeRun( U8 FreeRun )
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SetFreeRun:(0x%x)\r\n",FreeRun));
    if( FreeRun >= 2 )
        DBG_AUDIO_ERROR(printk("Invalid mode\r\n"));

    MHal_MAD_WriteMaskReg(REG_DEC_DECODE_CMD, 0x0020, FreeRun<<5 );
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Mpeg_GetSampleRate
//  [Description]:
//      This routine will return the 16bit mpeg samplerate
//  [Arguments]:
//      None
//  [Return]:
//      16bits mpeg sample rate
//
//*******************************************************************************

U16 MDrv_MAD_Mpeg_GetSampleRate( void )
{
    MS_U8 data1;

    data1 = MHal_MAD_ReadByte(REG_MB_MPEG_ADDR_H) & 0x0F;

    switch(data1) {
        case 0: return 44100; //printk("44.1K \n");
                break;
        case 1: return 48000; //printk("48K \n");
                break;
        case 2: return 32000; //printk("32K \n");
                break;
        case 3: return 22050; //printk("22.05K \n");
                break;
        case 4: return 24000; //printk("24K \n");
                break;
        case 5: return 16000; //printk("16K \n");
                break;
        case 6: return 11025; //printk("11.025K \n");
                break;
        case 7: return 12000; //printk("12K \n");
                break;
        case 8: return 8000; //printk("8K \n");
                break;
        default: DBG_AUDIO_ERROR(printk("Invalid sample rate\n")); break;
    }
    return -1;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_MPEG_GetBitRate
//  [Description]:
//      This routine will return the 16bit mpeg bitrate
//  [Arguments]:
//      None
//  [Return]:
//      16bits mpeg bitrate
//
//*******************************************************************************

U16 MDrv_MAD_MPEG_GetBitRate( void )
{
    MS_U8 data1;

    MS_U8 bitrate_index, layer_index, version_index;
    // Read BitRate
    bitrate_index = (MHal_MAD_ReadByte(REG_MB_MPEG_ADDR_L) & 0xF0)>>4;

    // Read Layer
    data1 = (MHal_MAD_ReadByte(REG_MB_MPEG_ADDR_L) & 0x0C)>>2;
    layer_index = data1 - 1;                // 0->layer1, 1->layer2, 2->layer3

    // Read MPEG version
    version_index = MDrv_MAD_Read_DSP_sram(0x0FD8, DSP_MEM_TYPE_DM);

    return(MpegBitRateTable[version_index*3+layer_index][bitrate_index]);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_mpeg_getLayer
//  [Description]:
//      This routine will return the 8bit mpeg layer number
//  [Arguments]:
//      None
//  [Return]:
//      8bits mpeg layer number
//
//*******************************************************************************

U16 MDrv_MAD_MPEG_GetLayer( void )
{
    MS_U8 data1;

    data1 = (MHal_MAD_ReadByte(REG_MB_MPEG_ADDR_L) & 0x0C)>>2;

    switch(data1) {
        case 1:  return 1; //printk("layer1\n");
                 break;
        case 2:  return 2; //printk("layer2\n");
                 break;
        case 3:  return 3; //printk("layer3\n");
                 break;
        //default: printk("Invalid Layer\n"); break;
        default: return 0;
                 break;
    }
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_MPEG_GetFrameNum
//  [Description]:
//      This routine will return the 24bit dsp decoding frame number
//  [Arguments]:
//      None
//  [Return]:
//      24bits mpeg frame number
//
//*******************************************************************************

U32 MDrv_MAD_MPEG_GetFrameNum( void )
{
    return (MDrv_MAD_Read_DSP_sram(0x0FF6, DSP_MEM_TYPE_DM));
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Mpeg_setFileSize
//  [Description]:
//      Set file hand shake size per request from MP3
//  [Arguments]:
//      1KB / 2KB / 4KB / 8KB / 16KB / 32KB
//*******************************************************************************

U8 MDrv_MAD_MPEG_SetFileSize(U8 value)
{
    MS_U16   file_size_value;

    if (MHal_MAD_ReadByte(REG_DEC_DECODE_CMD) != 0)
        return FALSE;                       // Modify file size only in "STOP" mode

    switch (value)
    {
    case FILE_SIZE_1KB:
        file_size_value = 0x0040;
        break;

    case FILE_SIZE_2KB:
        file_size_value = 0x0080;
        break;

    case FILE_SIZE_4KB:
    default:
        file_size_value = 0x0100;
        break;

    case FILE_SIZE_8KB:
        file_size_value = 0x0200;
        break;

    case FILE_SIZE_16KB:
        file_size_value = 0x0400;
        break;

    case FILE_SIZE_32KB:
        file_size_value = 0x0800;
        break;

    }

    // IDMA write PM data
    MDrv_MAD_Write_DSP_sram(0x0FF9, file_size_value, DSP_MEM_TYPE_PM);

    return TRUE;
}


//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_LoadCode
//  [Description]:
//      This routine load the DVB algorithm code.
//  [Arguments]:
//      None
//  [Return]:
//      None
//
//*******************************************************************************
BOOL MDrv_MAD_LoadCode(U8 u8Type)
{
    if(((u8Type & 0xF0) == 0x00)||((u8Type & 0xF0) == 0x10) )  //mad load code
    {
        MDrv_MAD_SetDSPCodeType(u8Type);
        MAD_DEBUG_P1(printk("MDrv_MAD_LoadCode(type=dvb (%x))\r\n", u8Type));

        MHal_MAD_WriteMaskByte(REG_AUDIO_DECODER1_CFG,0x07, 0x00);  // decoder1 Input --> DVB

        MDrv_MAD_DspLoadCode(u8Type);
        MDrv_MAD_SetIsDtvFlag(TRUE);
	 dtv_mode_en = 1;
        return TRUE;

    }
    else
    {
        DBG_AUDIO_ERROR(printk("MDrv_MAD_LoadCode: type(%x) is invalid\r\n",u8Type));
        return FALSE;
    }
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_ReLoadCode
//  [Description]:
//      This routine reload the DVB algorithm dsp code.
//  [Arguments]:
//      None
//  [Return]:
//      TURE / FALSE
//
//*******************************************************************************

BOOL MDrv_MAD_ReLoadCode(U8 u8Type1)
{
    //AUDIO_ALG_INFO *pau_info, *pau_info2, tmp_info, tmp_info2;
    MS_BOOL ret_status;
    AU_DVB_DECCMD   deccmd_status;

    MAD_DEBUG_P1(printk("MDrv_MAD_ReLoadCode ::Decoder Type[%x]......\n",u8Type1));
    if(MDrv_MAD_GetDSPCodeType() == u8Type1)
    {
	  DBG_AUDIO_ERROR(printk("Return for MDrv_MAD_ReLoadCode, it is the same decoder type.\r\n "));
	  return TRUE;
    }
    if(((u8Type1 & 0xF0) == 0x00)||((u8Type1 & 0xF0) == 0x10) )
    {
       MAD_LOCK();
        deccmd_status =  MDrv_MAD_GetDecCmd();
        MDrv_MAD_SetDecCmd(/*drvMAD_STOP*/ AU_DVB_DECCMD_STOP);

        //MsOS_DisableInterrupt(E_FIQ_SE_DSP2UP);//CHECK
        //MAD_DEBUG_P2(printk("HAL_MAD_ReLoadCode(type=dvb (%s))\r\n", pau_info->AlgName));

        // Reset MAD module
        MDrv_MAD_DisEn_MIUREQ();

        // enable DVB fix-sync mode
        MDrv_MAD_DvbFLockSynthesizer_En();
        ret_status = MDrv_MAD_AlgReloadCode(u8Type1);

        //MsOS_EnableInterrupt(E_FIQ_SE_DSP2UP);//CHECK
        MDrv_MAD_SetDSPCodeType(u8Type1);
        MDrv_MAD_SetDecCmd((AU_DVB_DECCMD)deccmd_status);
	 MAD_UNLOCK();
        return ret_status;
    }
    else
    {
        DBG_AUDIO_ERROR(printk("MDrv_MAD_ReLoadCode: Decoder Type[0x%x] is invalid\r\n",u8Type1));
        return FALSE;
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_AuReadTimeStamp
//  [Description]:
//      This routine return the stmestamp while decoding file format.
//  [Arguments]:
//      None
//*******************************************************************************
U32 MDrv_MAD_ReadTimeStamp(void)
{
    U32 w16_1 = (U16)MHal_MAD_ReadReg(REG_MB_TIME_STAMP_SEC);                // upper 16 bit
    U32 w16_2 = ((U16)MHal_MAD_ReadReg(REG_MB_TIME_STAMP_4ms) & 0x00FF);        // lower  8 bit
    U32 timestamp = (w16_1*1000) + (w16_2<<2);
    MAD_DEBUG_P3(printk("sec = 0x%x\r\n", w16_1));
    MAD_DEBUG_P3(printk("sec = 0x%x\r\n", w16_2));
    MAD_DEBUG_P3(printk("sec = 0x%x\r\n\r\n", timestamp));
    return timestamp;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_DvbSet_Dolby_DRC_Mode
//  [Description]:
//      Set Dolby AC3/AC3+ DRC Mode
//  [Arguments]:
//      DRC mode type :
//          0: Line mode
//          1: RF mode
//  [Return]:
//      None
//  [Doxygen]
/// Set Dolby AC3/AC3+ DRC Mode (Line/RF).
/// @param DRC_mod \b IN:
///   - 0: Line mode
///   - 1: RF mode
//
//*******************************************************************************

void MDrv_MAD_Set_Dolby_DRC_Mode(U8 DRC_mod)
{
    if(DRC_mod == 1)
    {
        MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL, 0x0080, 0x0080);       // RF Mod
    }
    else
    {
        MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL, 0x0080, 0x0000);       // Line Mod
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Set_Dolby_Downmix_Mode
//  [Description]:
//  [Arguments]:
//      None
//*******************************************************************************

void    MDrv_MAD_Set_Dolby_Downmix_Mode(U8 dmix_mod)
{
    if(dmix_mod == 1)
    {
        MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL, 0x0040, 0x0040);       // LoRo Mod
    }
    else
    {
        MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL, 0x0040, 0x0000);       // LtRt Mod
    }
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_AC3Dec_DIS
//  [Description]:
//      This routine : Set AC3 NOT DECODE
//  [Arguments]:
//      None
//*******************************************************************************

void MDrv_MAD_AC3Dec_DIS(BYTE ac3_dis_en)
{
    if (ac3_dis_en == 1)
    {
        MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0010,0x0010);        // AC3 NOT DEC
    }
    else
    {
        MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0010,0x0000);        // AC3 DEC
    }
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_DvbGet_AC3_Acmod
//  [Description]:
//      Get AC3 Channel Number
//  [Arguments]:
//      None
//  [Return]:
//      Channel Number :
//      - 0: Ac3Acmod_eTwoMono_1_ch1_ch2,            /* 1+1 */
//      - 1: Ac3Acmod_eOneCenter_1_0_C,              /* 100 */
//      - 2: Ac3Acmod_eTwoChannel_2_0_L_R,           /* 200 */
//      - 3: Ac3Acmod_eThreeChannel_3_0_L_C_R,       /* 300 */
//      - 4: Ac3Acmod_eThreeChannel_2_1_L_R_S,       /* 210 */
//      - 5: Ac3Acmod_eFourChannel_3_1_L_C_R_S,      /* 310 */
//      - 6: Ac3Acmod_eFourChannel_2_2_L_R_SL_SR,    /* 220 */
//      - 7: Ac3Acmod_eFiveChannel_3_2_L_C_R_SL_SR,  /* 320 & 321 */
//  [Doxygen]
/// Get HEAAC Channel Number
/// @return mod \b IN:
///      - 0: Ac3Acmod_eTwoMono_1_ch1_ch2,            /* 1+1 */
///      - 1: Ac3Acmod_eOneCenter_1_0_C,              /* 100 */
///      - 2: Ac3Acmod_eTwoChannel_2_0_L_R,           /* 200 */
///      - 3: Ac3Acmod_eThreeChannel_3_0_L_C_R,       /* 300 */
///      - 4: Ac3Acmod_eThreeChannel_2_1_L_R_S,       /* 210 */
///      - 5: Ac3Acmod_eFourChannel_3_1_L_C_R_S,      /* 310 */
///      - 6: Ac3Acmod_eFourChannel_2_2_L_R_SL_SR,    /* 220 */
///      - 7: Ac3Acmod_eFiveChannel_3_2_L_C_R_SL_SR,  /* 320 & 321 */
//
//*******************************************************************************
U8 MDrv_MAD_DvbGet_AC3_Acmod(void)
{
    MS_U8 u8Dolby_acmod;

     // AC3+
    u8Dolby_acmod = (MS_U8)MDrv_MAD_Read_DSP_sram(0x20b8, DSP_MEM_TYPE_DM);

    //printk("MDrv_MAD_DvbGet_AC3_Acmod(%d)......\n",u8Dolby_acmod);

    return (u8Dolby_acmod);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_DvbGet_AC3_Fscod
//  [Description]:
//      Get AC3 Sampling Rate
//  [Arguments]:
//      None
//  [Return]:
//      Sampling Rate :
//          0: 48   KHz
//          1: 44.1 KHz
//          2: 32   KHz
//  [Doxygen]
/// Get AC3 Sampling Rate
/// @return rate \b IN:
///   - 0: 48   KHz
///   - 1: 44.1 KHz
///   - 2: 32   KHz
//
//*******************************************************************************
U8 MDrv_MAD_DvbGet_AC3_Fscod(void)
{
    MS_U16 u16DD_fscod;

    if ((MDrv_MAD_Rpt_DTVES()&0x80)==0x80 )
    {   // AC3
        u16DD_fscod = MDrv_MAD_Read_DSP_sram(0x2401, DSP_MEM_TYPE_DM);
    }
    else
    {   // AC3+
        u16DD_fscod = MDrv_MAD_Read_DSP_sram(0x23C3, DSP_MEM_TYPE_DM);
    }

    if (u16DD_fscod == 0 )
    {
        return (0);             // 48.0 KHz
    }
    else if (u16DD_fscod == 1)
    {
        return (1);             // 44.1 KHz
    }
    else
    {
        return (2);             // 32.0 KHz
    }
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_DvbGet_AC3_Bitrate
//  [Description]:
//      Get AC3 Bit Rate
//  [Arguments]:
//      None
//  [Return]:
//      Bit Rate value:
//  [Doxygen]
/// Get AC3 Sampling Rate
/// @return Bit Rate value: \b IN:
//
//*******************************************************************************
U8 MDrv_MAD_DvbGet_AC3_Bitrate(void)
{
    MS_U8 u8Dolby_bitrate;

    if ((MDrv_MAD_Rpt_DTVES()&0x80)==0x80 )
    {   // AC3
        u8Dolby_bitrate = (MS_U8)MDrv_MAD_Read_DSP_sram(0x2402, DSP_MEM_TYPE_DM);
    }
    else
    {   // AC3+
        u8Dolby_bitrate = (MS_U8)MDrv_MAD_Read_DSP_sram(0x23C4, DSP_MEM_TYPE_DM);
    }

    return u8Dolby_bitrate;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_DvbGet_AC3_Dialnorm
//  [Description]:
//      Get AC3 Dialnorm value
//  [Arguments]:
//      None
//  [Return]:
//      Dialnorm value:
//  [Doxygen]
/// Get AC3 Dialnorm value
/// @return AC3 Dialnorm value: \b IN:
//
//*******************************************************************************
U8 MDrv_MAD_DvbGet_AC3_Dialnorm(void)
{
    MS_U8 u8Dolby_dialnorm;

    if ((MDrv_MAD_Rpt_DTVES()&0x80)==0x80 )
    {   // AC3
        u8Dolby_dialnorm = (MS_U8)MDrv_MAD_Read_DSP_sram(0x2403, DSP_MEM_TYPE_DM);
    }
    else
    {   // AC3+
        u8Dolby_dialnorm = (MS_U8)MDrv_MAD_Read_DSP_sram(0x23D9, DSP_MEM_TYPE_DM);
    }

    return u8Dolby_dialnorm;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_DvbGet_AC3_GetVersion
//  [Description]:
//      This routine will return the AC3 version
//  [Arguments]:
//      None
//  [Return]:
//      AC3:0, EAC3:1
//
//*******************************************************************************
U8 MDrv_MAD_DvbGet_AC3_Version(void)
{
    U8 version=0;

    if ((MDrv_MAD_Rpt_DTVES()&0x80)==0x80 )
    {   // AC3
        version=0;
    }
    else
    {   // AC3+
        version=1;
    }
    return version;

return 0;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Get_DDP_AD_Status
//  [Description]:
//      This routine will return the DDP AD status
//  [Arguments]:
//      None
//  [Return]:
//      AC3:0, EAC3:1
//
//*******************************************************************************
U8 MDrv_MAD_Get_DDP_AD_Status(void)
{
    U8 ADstatus;

    if((MHal_MAD_ReadByte(REG_MB_DEC_ID_STATUS+1)&0xF0) == 0x70)
    {
        ADstatus = MHal_MAD_ReadByte(REG_MB_DDP_AD_STATUS);
    }
    else
    {
        ADstatus = 0;
    }
    return ADstatus;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Set_DDP_AD_Mode
//  [Description]:
//      This routine will Set the DDP AD
//  [Arguments]:
//      0 : AD off
//      1 : AD 1
//      2 : AD 2
//      3 : AD 3
//      4 : Separate AD Mode
//  [Return]:
//      None
//
//*******************************************************************************
#define REG_MB_DDP_AD_IDSEL     0x2D82
#define REG_MB_DDP_AD_MODSEL    0x2D80

void MDrv_MAD_Set_DDP_AD_Mode(U8 ddp_ad_id)
{
    if((MHal_MAD_ReadByte(REG_MB_DEC_ID_STATUS+1)&0xF0) != 0x70)
        return;

    MHal_MAD_WriteByte(REG_MB_DDP_AD_IDSEL, ddp_ad_id&0xF);

    switch (ddp_ad_id)
    {
        case 1:
        case 2:
        case 3:
            MHal_MAD_WriteByte(REG_MB_DDP_AD_MODSEL, 0x21);         //single ad mode
            break;
        case 4:
            MHal_MAD_WriteByte(REG_MB_DDP_AD_MODSEL, 0x23);         //sp ad mode
            break;
        case 0:
        default:
            MHal_MAD_WriteByte(REG_MB_DDP_AD_MODSEL, 0x00);         //Off
            break;
    }
}

long long MDrv_MAD_Get_PTS(void)
{
    long long tmp;
    tmp=((MDrv_MAD_Read_DSP_sram(0x0FF3,DSP_MEM_TYPE_PM)&0x01FF)<<24)+MDrv_MAD_Read_DSP_sram(0x0FF4,DSP_MEM_TYPE_PM);
    MAD_DEBUG_P2(printk("%s::PTS = lld\r\n", tmp));
    return tmp;
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_Dvb_setADMixMode
//  [Description]:
//      This routine set MPEG mixed with AD or AD mixed with MPEG or not.
//  [Arguments]:
//      Input: mix_mode (MAIN_MIX_AD / AD_MIX_MAIN)
//             en_mix   (MIX_OFF / MIX_ON)
//*******************************************************************************
void MDrv_MAD_Dvb_setADMixMode (U16 mix_mode, U16 en_mix)
{
    switch (mix_mode)
    {
        case MAIN_MIX_AD:
            MHal_MAD_WriteMaskReg(REG_MB_DEC_CTRL, 0x0001, en_mix);
            break;

        case AD_MIX_MAIN:
            MHal_MAD_WriteMaskReg(REG_MB_DEC3_CTRL, 0x8000, en_mix<<15);  // IO_100 [15]
            break;

        default:
            break;
    }
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Mpeg_GetChannelNum
//  [Description]:
//      This routine will return the 8bit mpeg channel number
//  [Arguments]:
//      None
//  [Return]:
//      8bits mpeg channel number
//
//*******************************************************************************

U16 MDrv_MAD_Mpeg_GetChannelNum( void )
{
    U8 data1=0;

    U8 u8codeTypeDSP = MDrv_MAD_GetDSPCodeType();

    if( u8codeTypeDSP != AU_DVB_STANDARD_MPEG ) {
        MAD_DEBUG_P2(printk("DSP need to relaod mpeg\r\n"));
        return 0;
    }

    // get MPEG Sound Mode
    data1 = MHal_MAD_ReadReg(0x2DAA) & 0x03;
    //MAD_DEBUG_P1(printk("    Sound Mode: 0x%02x\n", data1));

    switch(data1) {
        case 0:    MAD_DEBUG_P3(printk("Stereo \n")); break;
        case 1:    MAD_DEBUG_P3(printk("Joint Stereo \n")); break;
        case 2:    MAD_DEBUG_P3(printk("Dual Channel \n")); break;
        case 3:    MAD_DEBUG_P3(printk("Mono \n")); break;
        default: DBG_AUDIO_ERROR(printk("%s ::Invalid Sound Mode\n", __FUNCTION__)); break;
    }

    return(data1);

}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_WMA_GetSampleRate
//  [Description]:
//      This routine will return the 16bit wma samplerate
//  [Arguments]:
//      None
//  [Return]:
//      16bits wma sample rate
//
//*******************************************************************************

U16 MDrv_MAD_WMA_GetSampleRate( void )
{
    return (MDrv_MAD_Read_DSP_sram(0x1854, DSP_MEM_TYPE_DM));
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_WMA_GetBitRate
//  [Description]:
//      This routine will return the 32bit wma bitrate
//  [Arguments]:
//      None
//  [Return]:
//      32bits wma bitrate
//
//*******************************************************************************

U32 MDrv_MAD_WMA_GetBitRate( void )
{
    return ((MDrv_MAD_Read_DSP_sram(0x1858,DSP_MEM_TYPE_DM))<<3);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_aac_getSampleRate
//  [Description]:
//      This routine will return the 16bit aac samplerate
//  [Arguments]:
//      None
//  [Return]:
//      16bits aac sample rate
//
//*******************************************************************************

U32 MDrv_MAD_aac_getSampleRate( void )
{
    U32 aac_SampleRate = 0;
#if 0 //CHECK

    aac_SampleRate= MDrv_MAD_Read_DSP_sram(0x602B, 1);

    //printk("MDrv_MAD_aac_getSampleRate1(0x%x)\n",aac_SampleRate);

    switch(aac_SampleRate)
    {
        case 0x8CA:
            aac_SampleRate = 96000;
            break;
        case 0x990:
            aac_SampleRate = 88200;
            break;
        case 0xD2F:
            aac_SampleRate = 64000;
            break;
    }
    if( aac_SampleRate > 0xD2F && aac_SampleRate < 0x1321 )
        //case 0x1194:
            aac_SampleRate = 48000;
    else if( aac_SampleRate > 0x1194 && aac_SampleRate < 0x1A5E )
        //case 0x1321:
            aac_SampleRate = 44100;
    else if( aac_SampleRate > 0x1321 && aac_SampleRate < 0x2328 )
        //case 0x1A5E:
            aac_SampleRate = 32000;
    else if( aac_SampleRate > 0x1A5E && aac_SampleRate < 0x2643 )
        //case 0x2328:
            aac_SampleRate = 24000;
    else if( aac_SampleRate > 0x2328 && aac_SampleRate < 0x34BC )
        //case :0x2643
            aac_SampleRate = 22050;
    else if( aac_SampleRate > 0x2643 && aac_SampleRate < 0x4650 )
        //case :0x34BC
            aac_SampleRate = 16000;
    else if( aac_SampleRate > 0x34BC && aac_SampleRate < 0x4C87 )
        //case 0x4650:
            aac_SampleRate = 12000;
    else if( aac_SampleRate > 0x4650 && aac_SampleRate < 0x6978 )
        //case :0x4C87
            aac_SampleRate = 11025;
    else if( aac_SampleRate > 0x4C87 && aac_SampleRate < 0x72CB )
        //case 0x6978:
            aac_SampleRate = 8000;
    else if( aac_SampleRate > 0x6978)
        //case 0x72CB:
            aac_SampleRate = 7350;

    //printk("MDrv_MAD_aac_getSampleRate(%d)\n",aac_SampleRate);
#endif
    return aac_SampleRate;
}

//******************************************************************************
//Function name:    MDrv_MAD_SetADOutputMode
//  [Doxygen]
// Set AD output mode
// @param
//     mix_mode: AD_OUT_SPEAKER / AD_OUT_HP / AD_OUT_BOTH
//
//******************************************************************************
void MDrv_MAD_SetADOutputMode (U8 out_mode)
{
    switch (out_mode)
    {
    case AD_OUT_SPEAKER:
        MDrv_MAD_Dvb_setADMixMode (MAIN_MIX_AD, MIX_OFF);  // Decoder1 don't mix
        MDrv_MAD_Dvb_setADMixMode (AD_MIX_MAIN, MIX_ON);   // Decoder2 mix
        break;

    case AD_OUT_HP:
        MDrv_MAD_Dvb_setADMixMode (MAIN_MIX_AD, MIX_OFF);  // Decoder1 don't mix
        MDrv_MAD_Dvb_setADMixMode (AD_MIX_MAIN, MIX_ON);   // Decoder2 mix
        break;

    case AD_OUT_BOTH:
        MDrv_MAD_Dvb_setADMixMode (MAIN_MIX_AD, MIX_ON);   // Decoder1 mix
        MDrv_MAD_Dvb_setADMixMode (AD_MIX_MAIN, MIX_ON);   // Decoder2 mix
         break;

    case AD_OUT_NONE:
    default:
        MDrv_MAD_Dvb_setADMixMode (MAIN_MIX_AD, MIX_OFF);  // Decoder1 don't mix
        MDrv_MAD_Dvb_setADMixMode (AD_MIX_MAIN, MIX_OFF);  // Decoder2 don't mix
        break;
    }
}

//******************************************************************************
//
//  [Function Name]: ok
//      MDrv_MAD_SetSPKOutMode(DvbSetSoundMode)
//  [Description]:
//      This routine is to set the sound mode (Stereo/LL/RR/Mute) for DVB module.
//  [Arguments]:
//      mode type : Stereo/LL/RR/Mute
//  [Return]:
//      TRUE / FALSE
//  [Doxygen]
/// This routine is to set the sound mode (Stereo/LL/RR/Mute) for DVB module.
/// @param mode_type \b IN:
///   - 0: Stereo
///   - 1: LL
///   - 2: RR
///   - 3: Mute
//
//*******************************************************************************
void MDrv_MAD_SetSPKOutMode(U8 mode_type)
{
    MAD_DEBUG_P1(printk("MDrv_MAD_SetSPKOutMode(0x%x)...\n",mode_type));

    switch(mode_type) {
        case ADEC_SPK_MODE_LL: //LL mode
            MHal_MAD_WriteMaskReg(REG_MB_MODE_SELECT, 0x03, 1);
            break;

        case ADEC_SPK_MODE_RR: //RR mode
            MHal_MAD_WriteMaskReg(REG_MB_MODE_SELECT, 0x03, 2);
            break;

        case ADEC_SPK_MODE_LR: //stereo mode
            MHal_MAD_WriteMaskReg(REG_MB_MODE_SELECT, 0x03, 0);
            break;

        case ADEC_SPK_MODE_MIX: //mute
        default:
            MHal_MAD_WriteMaskReg(REG_MB_MODE_SELECT, 0x03, 3);
            break;
    }

}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_mpeg_GetSoundMode
//  [Description]:
//      This routine will return the 8bit mpeg sound mode
//  [Arguments]:
//      None
//  [Return]:
//      8bits mpeg sound mode
//
//*******************************************************************************
U8 MDrv_MAD_mpeg_GetSoundMode( void )
{
    U8 data1;
    U8 u8codeTypeDSP = MDrv_MAD_GetDSPCodeType();

    if( u8codeTypeDSP != AU_DVB_STANDARD_MPEG ) {
        MAD_DEBUG_P2(printk("DSP need to relaod mpeg\r\n"));
        return 0;
    }

    // get MPEG Sound Mode
    data1 = MHal_MAD_ReadReg(REG_D2M_MAILBOX_B_L) & 0x03;
    MAD_DEBUG_P1(printk("    Sound Mode: 0x%x\n", data1));

    switch(data1) {
        case 0:    MAD_DEBUG_P3(printk("Stereo \n")); break;
        case 1:    MAD_DEBUG_P3(printk("Joint Stereo \n")); break;
        case 2:    MAD_DEBUG_P3(printk("Dual Channel \n")); break;
        case 3:    MAD_DEBUG_P3(printk("Mono \n")); break;
        default: DBG_AUDIO_ERROR(printk("%s ::Invalid Sound Mode\n", __FUNCTION__)); break;
    }

    return(data1);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_mpeg_SetSoundMode
//  [Description]:
//      This routine will set the mpeg sound mode
//  [Arguments]:
//      mpeg sound mode
//          0x2D86[4:2] :
//                       000->LL
//                       001->RR
//                       010 ->LR
//                       011->(L+R/2, L+R/2)
//  [Return]:
//      None
//
//*******************************************************************************
void MDrv_MAD_mpeg_SetSoundMode( U8 mode )
{
    // get MPEG Sound Mode
    switch(mode)
    {
        case ADEC_SPK_MODE_RR:
             MHal_MAD_WriteMaskReg(REG_MB_DEC_CTRL, 0x000C, 0x0008);    // RR output in Dual-Mono
             MHal_MAD_WriteMaskReg(REG_MB_MODE_SELECT, 0x0003, 0x0002);   //SPDIF out mode
            break;

        case ADEC_SPK_MODE_LR:
            MHal_MAD_WriteMaskReg(REG_MB_DEC_CTRL, 0x000C, 0x0000);    // LR output in Dual-Mono
             MHal_MAD_WriteMaskReg(REG_MB_MODE_SELECT, 0x0003, 0x0000);   //SPDIF out mode
            break;

        case ADEC_SPK_MODE_MIX:
            MHal_MAD_WriteMaskReg(REG_MB_DEC_CTRL, 0x000C, 0x000C);    // (L+R)/2 output in Dual-Mono
             MHal_MAD_WriteMaskReg(REG_MB_MODE_SELECT, 0x0003, 0x0003);   //SPDIF out mode
            break;
        case ADEC_SPK_MODE_LL:
        default:
             MHal_MAD_WriteMaskReg(REG_MB_DEC_CTRL, 0x000C, 0x0004);    // LL output in Dual-Mono
             MHal_MAD_WriteMaskReg(REG_MB_MODE_SELECT, 0x0003, 0x0001);   //SPDIF out mode
            break;
    }
}

void MDrv_MAD_SetAutoVolumeControl(U8 enable)
{
    MHal_MAD_WriteMaskByte(0x2D21, 0x10, enable<<4);
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_HEAAC_GetBitRate
//  [Description]:
//      This routine will return the 16bit mpeg bitrate
//  [Arguments]:
//      None
//  [Return]:
//      16bits mpeg bitrate
//
//*******************************************************************************
U16 MDrv_MAD_HEAAC_GetBitRate( void ) //CHECK
{
     U16 bitRate=0;

    return bitRate;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_HEAAC_GetSampleRate
//  [Description]:
//      This routine will return heaac
//      None
//  [Return]:
//      heaac sample rate
//
//*******************************************************************************
U32 MDrv_MAD_HEAAC_GetSampleRate( void )
{
    U16 sampling_rate_index, sbr_flag;

    sampling_rate_index = (U16)MDrv_MAD_Read_DSP_sram(0x340E, 1);
    sbr_flag = (U16)MDrv_MAD_Read_DSP_sram(0x340A, 1);

    switch(sampling_rate_index)
    {
        case 3:
            if(!sbr_flag)
                return AU_AAC_SF_48K;
            else
                return AU_AAC_SF_UNSUPPORTED;
            break;

        case 4:
            if(!sbr_flag)
                return AU_AAC_SF_441K;
            else
                return AU_AAC_SF_UNSUPPORTED;
            break;

        case 5:
            if(!sbr_flag)
                return AU_AAC_SF_32K;
            else
                return AU_AAC_SF_UNSUPPORTED;
            break;

        case 6:
            if(!sbr_flag)
                return AU_AAC_SF_24K;
            else
                return AU_AAC_SF_48K;
            break;

        case 7:
            if(!sbr_flag)
                return AU_AAC_SF_2205K;
            else
                return AU_AAC_SF_441K;
            break;

        case 8:
            if(!sbr_flag)
                return AU_AAC_SF_16K;
            else
                return AU_AAC_SF_32K;
            break;

        case 9:
            if(!sbr_flag)
                return AU_AAC_SF_12K;
            else
                return AU_AAC_SF_24K;
            break;

        case 10:
            if(!sbr_flag)
                return AU_AAC_SF_11025K;
            else
                return AU_AAC_SF_2205K;
            break;

        case 11:
            if(!sbr_flag)
                return AU_AAC_SF_8K;
            else
                return AU_AAC_SF_16K;
            break;

        default:                /* un-supported */
            return AU_AAC_SF_UNSUPPORTED;
            break;
    }
    return AU_AAC_SF_UNSUPPORTED;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_HEAAC_GetVersion
//  [Description]:
//      This routine will the heaac version
//  [Arguments]:
//      None
//  [Return]:
//
//*******************************************************************************
U8 MDrv_MAD_HEAAC_GetVersion( void )
{
    U8 version=0xFF;
    U16 gValue=0;

    gValue = MHal_MAD_ReadReg(REG_MB_DEC_ID_STATUS);        // add one time read to avoid mailbox issue

    gValue &= 0xFF00;

    if(gValue == 0x5100)
        version=0;
    else if (gValue == 0x5200)
        version=1;
    else if (gValue == 0x5300)
        version=2;
  //  else
      //  DBG_AUDIO_ERROR(printk("Can not get HEAAC version !!\n"));

  return version;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_HEAAC_GetTransmissionformat
//  [Description]:
//      This routine will return the HEAAC transmission format
//      output: LOAS/LATM = 1, ADTS = 0
//  [Arguments]:
//      None
//  [Return]:
//      None
//
//*******************************************************************************
U16 MDrv_MAD_HEAAC_GetTransmissionformat( void )
{
    return MDrv_MAD_Read_DSP_sram(0x340C, 1);
}
//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_HEAAC_GetChannelNum
//  [Description]:
//      This routine will HEAAC channel number
//  [Arguments]:
//      None
//  [Return]:
//      Channel Number :
//      - 0: AacAcmod_eTwoMono_1_ch1_ch2,            /* 1+1 */
//      - 1: AacAcmod_eOneCenter_1_0_C,              /* 100 */
//      - 2: AacAcmod_eTwoChannel_2_0_L_R,           /* 200 */
//      - 3: AacAcmod_eThreeChannel_3_0_L_C_R,       /* 300 */
//      - 4: AacAcmod_eThreeChannel_2_1_L_R_S,       /* 210 */
//      - 5: AacAcmod_eFourChannel_3_1_L_C_R_S,      /* 310 */
//      - 6: AacAcmod_eFourChannel_2_2_L_R_SL_SR,    /* 220 */
//      - 7: AacAcmod_eFiveChannel_3_2_L_C_R_SL_SR,  /* 320 & 321 */
//  [Doxygen]
/// Get HEAAC Channel Number
/// @return mod \b IN:
///      - 0: AacAcmod_eTwoMono_1_ch1_ch2,            /* 1+1 */
///      - 1: AacAcmod_eOneCenter_1_0_C,              /* 100 */
///      - 2: AacAcmod_eTwoChannel_2_0_L_R,           /* 200 */
///      - 3: AacAcmod_eThreeChannel_3_0_L_C_R,       /* 300 */
///      - 4: AacAcmod_eThreeChannel_2_1_L_R_S,       /* 210 */
///      - 5: AacAcmod_eFourChannel_3_1_L_C_R_S,      /* 310 */
///      - 6: AacAcmod_eFourChannel_2_2_L_R_SL_SR,    /* 220 */
///      - 7: AacAcmod_eFiveChannel_3_2_L_C_R_SL_SR,  /* 320 & 321 */
//
//*******************************************************************************
U16 MDrv_MAD_HEAAC_GetChannelNum( void )
{
    MS_U16 u16chNum;

    u16chNum = (MS_U16)MDrv_MAD_Read_DSP_sram(0x3680, DSP_MEM_TYPE_DM);


    //printk("MDrv_MAD_HEAAC_GetChannelNum(%d)......\n",u16chNum);

    return (u16chNum);
}

U8 MDrv_MAD_DvbSetPara_IDMA(U16 DSPaddr, U8 value1, U8 value2, U8 value3)
{
    U8  dat[3];

    MAD_DEBUG_P1(printk("MDrv_MAD_DvbSetPara_IDMA DSPaddr[0x%x], value[0x%x, 0x%x, 0x%x]\r\n",DSPaddr,value1,value2,value3));
    //MHal_MAD_WriteMaskReg(0x3CEC, 0x40, 0x00);          //DMA sel to RIU
    MHal_MAD_AbsWriteMaskByte(0x103C14, 0x01, 0x00);

    dat[2] = value1; dat[1] = value2; dat[0] = value3;
MAD_LOCK();
    MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x00);//need to choose DSP for DEC/SE once use IDMA/BDMA
    MHal_MAD_WriteReg(REG_DEC_IDMA_WRBASE_ADDR_L, DSPaddr);//|0x40); //bit[14] 0 = select PM/CM; 1 = select DM
    //msleep(1);//20090205 mstar remove
    MHal_MAD_WriteMaskReg(REG_DEC_IDMA_CTRL0, 0x0004,0x0000);

    MHal_MAD_WriteReg(REG_DEC_DSP_BRG_DATA_L, dat[2]<<8 |dat[1]);
MAD_UNLOCK();
    if ( MDrv_MAD_DecDSP_chkIdmaReady(AUD_CHK_DSP_WRITE_RDY) == FALSE ) {
        DBG_AUDIO_ERROR(printk("%s :: chkIdamReady error. 1\r\n", __FUNCTION__));
        return FALSE;
    }

    MHal_MAD_WriteReg(REG_DEC_DSP_BRG_DATA_L, dat[0]);
    if ( MDrv_MAD_DecDSP_chkIdmaReady(AUD_CHK_DSP_WRITE_RDY) == FALSE ) {
        DBG_AUDIO_ERROR(printk("%s :: chkIdamReady error. 2\r\n", __FUNCTION__));
        return FALSE;
    }
    return TRUE;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Dvb_XPCM_setParam(U8 audioType, U8 channels, U32 sampleRate,
//                                         U8 bitsPerSample, U32 blockSize, U32 samplePerBlock)
//  [Description]:
//      This routine configures necessary parameters when playing XPCM data
//  [Arguments]:
//      audio type:     1: lpcm; 2: MS-ADPCM; 17: IMA-ADPCM
//      channels:               0:mono; 1:stereo
//      sampleRate:           sampling rate (Hz)
//      bitsPerSample:       bits per sample.. 16:16bits; 8:8bits
//      blockSize:              block size
//      samplePerBlock:     samples per block
//  [Return]:
//      TRUE when parameters are correct; FLASE else.
//
//*******************************************************************************
#define XPCM_DSP_PARAM_BASE     0x4FA0 //in DSP PM address
U8 MDrv_MAD_Dvb_XPCM_setParam (U8 audioType, U8 channels, U32 sampleRate,
                                                    U8 bitsPerSample, U32 blockSize, U32 samplePerBlock)
{
    MDrv_MAD_Write_DSP_sram(XPCM_DSP_PARAM_BASE, (MS_U32)audioType, DSP_MEM_TYPE_DM);               //audio type
    MDrv_MAD_Write_DSP_sram((XPCM_DSP_PARAM_BASE+1), (MS_U32)channels, DSP_MEM_TYPE_DM);                //channel numbers
    MDrv_MAD_Write_DSP_sram((XPCM_DSP_PARAM_BASE+2), (MS_U32)sampleRate, DSP_MEM_TYPE_DM);              //sample rate
    MDrv_MAD_Write_DSP_sram((XPCM_DSP_PARAM_BASE+3), (MS_U32)bitsPerSample, DSP_MEM_TYPE_DM);           //bits per sample
    MDrv_MAD_Write_DSP_sram((XPCM_DSP_PARAM_BASE+4), (MS_U32)blockSize, DSP_MEM_TYPE_DM);               //block size (ADPCM)
    MDrv_MAD_Write_DSP_sram((XPCM_DSP_PARAM_BASE+5), (MS_U32)samplePerBlock, DSP_MEM_TYPE_DM);          //sample per block (ADPCM) or endian (LPCM)

    return TRUE;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Dvb2_XPCM_setParam(U8 audioType, U8 channels, U32 sampleRate,
//                                         U8 bitsPerSample, U32 blockSize, U32 samplePerBlock)
//  [Description]:
//      This routine configures necessary parameters when playing XPCM data
//  [Arguments]:
//      audio type:     1: lpcm; 2: MS-ADPCM; 17: IMA-ADPCM
//      channels:               0:mono; 1:stereo
//      sampleRate:           sampling rate (Hz)
//      bitsPerSample:       bits per sample.. 16:16bits; 8:8bits
//      blockSize:              block size
//      samplePerBlock:     samples per block
//  [Return]:
//      TRUE when parameters are correct; FLASE else.
//
//*******************************************************************************
U8 MDrv_MAD_Dvb2_XPCM_setParam (U8 audioType, U8 channels, U32 sampleRate,
                                                    U8 bitsPerSample, U32 blockSize, U32 samplePerBlock)
{
    MDrv_MAD2_Write_DSP_sram(XPCM_DSP_PARAM_BASE, (MS_U32)audioType, DSP_MEM_TYPE_PM);               //audio type
    MDrv_MAD2_Write_DSP_sram((XPCM_DSP_PARAM_BASE+1), (MS_U32)channels, DSP_MEM_TYPE_PM);                //channel numbers
    MDrv_MAD2_Write_DSP_sram((XPCM_DSP_PARAM_BASE+2), (MS_U32)sampleRate, DSP_MEM_TYPE_PM);              //sample rate
    MDrv_MAD2_Write_DSP_sram((XPCM_DSP_PARAM_BASE+3), (MS_U32)bitsPerSample, DSP_MEM_TYPE_PM);           //bits per sample
    MDrv_MAD2_Write_DSP_sram((XPCM_DSP_PARAM_BASE+4), (MS_U32)blockSize, DSP_MEM_TYPE_PM);               //block size (ADPCM)
    MDrv_MAD2_Write_DSP_sram((XPCM_DSP_PARAM_BASE+5), (MS_U32)samplePerBlock, DSP_MEM_TYPE_PM);          //sample per block (ADPCM) or endian (LPCM)
    return TRUE;
}


//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Read_DSP_sram
//  [Description]:
//      Read DSP internal sram value by IDMA
//  [Arguments]:
//      WORD dsp_addr, DSP internal sram address
//      BOOL dm, select 1:dm or 0:pm
//  [Return]:
//      U32 sram value
//
//*******************************************************************************
U32 MDrv_MAD_Read_DSP_sram(U16 u16Dsp_addr,BOOL dsp_memory_type)
{
    MS_U8  dat[3];
    MS_U32 u32Value=0x00;

    if(audio_recovery)
        return FALSE;

MAD_LOCK();
    MHal_MAD_AbsWriteMaskByte(0x103C14, 0x01, 0x00);
    // check IDMA busy or not
    MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x00); // IDMA is used by DEC-DSP
    if((MDrv_MAD_CheckDecIdmaReady(AUD_CHK_DSP_READ_RDY) == FALSE)||(MDrv_MAD_CheckDecIdmaReady(AUD_CHK_DSP_WRITE_RDY) == FALSE))
    {
        DBG_AUDIO_ERROR(printk("ERROR: DEC DSP IDMA Busy \r\n"));
	 MAD_UNLOCK();
        return FALSE;
    }

    if (dsp_memory_type == DSP_MEM_TYPE_DM)
    {
        u16Dsp_addr = (u16Dsp_addr|0x8000);                   //select DM
    }

    MHal_MAD_WriteReg(REG_DEC_IDMA_RDBASE_ADDR_L, u16Dsp_addr);
    MHal_MAD_WriteMaskByte(REG_DEC_IDMA_CTRL0, 0x08, 0x08);        //0x2D00[3]
    //msleep(2);
    mdelay(2);
    if (MDrv_MAD_CheckDecIdmaReady(AUD_CHK_DSP_READ_RDY)==FALSE)
    {
        DBG_AUDIO_ERROR(printk("ERROR: DEC DSP IDMA read data time out \r\n"));
	 MAD_UNLOCK();
        return FALSE;
    }

    dat[2] = MHal_MAD_ReadByte(REG_DEC_IDMA_RDDATA_H_1);
    dat[1] = MHal_MAD_ReadByte(REG_DEC_IDMA_RDDATA_H_0);
    dat[0] = MHal_MAD_ReadByte(REG_DEC_IDMA_RDDATA_L);

    u32Value = ((MS_U8)dat[2] << 16) | ((MS_U8)dat[1] << 8) | (MS_U8)dat[0];

    MAD_UNLOCK();
    return u32Value;
}

MS_U16 MDrv_MAD_RA8_GetParaBase(void)
{
    return (MS_U16)0x1000;
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Dvb_RA8_setParam(U8 mNumCodecs, U16 mSamples, U16 mSampleRate,
//                              U8* Channels, U8* Regions, U8* cplStart, U8* cplQbits,
//                              U16* FrameSize)
//  [Description]:
//      This routine configures necessary parameters when playing RA8 LBR data
//  [Arguments]:
//      mNumCodecs:             mNumCodecs == 1      : stereo or mono,
//                              2 <= mNumCodecs <= 5 : multi channels
//      mSamples:               output samples: 256, 512, 1024
//      sampleRate:             sampling rate (Hz): 8, 11.025, 16, 22.05, 44.1, 48
//      channels:               1: mono, 2: stereo
//      regions:                0 ~ 51
//      cplStart:               0 ~ 31
//      cplQbits:               0, 2, 3, 4, 5, 6
//      FrameSize:              bitstream size of every NumCodes (bytes): 0 ~ 65535
//  [Return]:
//      TRUE when parameters are correct; FLASE else.
//
//*******************************************************************************
U8 MDrv_MAD_Dvb_RA8_setParam(U16 mNumCodecs, U16 mSamples, U16 mSampleRate,
                           U16* Channels, U16* Regions, U16* cplStart, U16* cplQbits,
                           U16* FrameSize)
{
    BYTE codecsIdx = 0, paramNum1 = 0;
    U16 cTmp = 0, cWriteBuf = 0;
    MS_U16 RaBaseAddr = MDrv_MAD_RA8_GetParaBase();

    if ( Channels == NULL || Regions == NULL || cplStart == NULL ||
         cplQbits == NULL || FrameSize == NULL )
    {
        return FALSE;
    }

    for ( codecsIdx = 0; codecsIdx < 5; codecsIdx++ )
    {
        if ( codecsIdx <= mNumCodecs - 1 )
        {
            if ( Channels[codecsIdx] < 1 || Channels[codecsIdx] > 2 )
                return FALSE;
            if ( Regions[codecsIdx] > 51 )
                return FALSE;
            if ( cplStart[codecsIdx] > 31 )
                return FALSE;
            if ( (cplQbits[codecsIdx] != 0) &&
                 (cplQbits[codecsIdx] < 2 || cplQbits[codecsIdx] > 6) )
                return FALSE;
        }
        else
        {
            break;
        }
    }

    switch ( mNumCodecs )
    {
        case 5:
            cTmp = 4;
            break;
        case 4:
            cTmp = 3;
            break;
        case 3:
            cTmp = 2;
            break;
        case 2:
            cTmp = 1;
            break;
        case 1:
            cTmp = 0;
            break;
        default:
            return FALSE;
    }
    cWriteBuf = cTmp << 13;

    switch ( mSamples )
    {
        case 1024:
            cTmp = 2;
            break;
        case 512:
            cTmp = 1;
            break;
        case 256:
            cTmp = 0;
            break;
        default:
            return FALSE;
    }
    cWriteBuf |= cTmp << 11;

    switch ( mSampleRate )
    {
        case 48000:
            cTmp = 0;
            break;
        case 44100:
            cTmp = 1;
            break;
        case 22050:
            cTmp = 2;
            break;
        case 16000:
            cTmp = 3;
            break;
        case 11025:
            cTmp = 4;
            break;
        case 8000:
            cTmp = 5;
            break;
        default:
            return FALSE;
    }
    cWriteBuf |= cTmp << 8;
    MDrv_MAD_Write_DSP_sram(RaBaseAddr+paramNum1,cWriteBuf, DSP_MEM_TYPE_PM);

    for ( codecsIdx = 0; codecsIdx < 5; codecsIdx++ )
    {
        paramNum1++;

        if ( codecsIdx <= mNumCodecs - 1 )
        {
            cWriteBuf  = Channels[codecsIdx] << 14;
            cWriteBuf |= Regions[codecsIdx]  << 8;
            cWriteBuf |= cplStart[codecsIdx] << 3;
            cWriteBuf |= cplQbits[codecsIdx];
            MDrv_MAD_Write_DSP_sram(RaBaseAddr+paramNum1,cWriteBuf, DSP_MEM_TYPE_PM);
            paramNum1++;
            cWriteBuf = FrameSize[codecsIdx];
            MDrv_MAD_Write_DSP_sram(RaBaseAddr+paramNum1,cWriteBuf, DSP_MEM_TYPE_PM);
        }
        else
        {
            cWriteBuf = 0;
            MDrv_MAD_Write_DSP_sram(RaBaseAddr+paramNum1,cWriteBuf, DSP_MEM_TYPE_PM);
            paramNum1++;
            MDrv_MAD_Write_DSP_sram(RaBaseAddr+paramNum1,cWriteBuf, DSP_MEM_TYPE_PM);
        }
    }

    return TRUE;
}
WORD au_ra8_para_address[11]=
{
    0x1000,
    0x1001,
    0x1002,
    0x1003,
    0x1004,
    0x1005,
    0x1006,
    0x1007,
    0x1008,
    0x1009,
    0x100A,
};

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_DVB_PcmLevelControl
//  [Description]:
//      check Dolby spec Bullutin 11
//  [Arguments]:
//      sysmod :  0  --> ATSC line mode
//      sysmod :  1  --> DVB RF mode
//      sysmod :  2  --> DVB line mode
//      sysmod :  3  --> EMP mode
//*******************************************************************************
BOOL MDrv_MAD_DVB_PcmLevelControl( U8 sysmod )
{
	MAD_DEBUG_P1(printk("MDrv_MAD_DVB_PcmLevelControl:%x\n", sysmod));
	switch(sysmod)
	{
		case 0: 	//ATSC line mode
		case 3: 	//EMP mode
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0020,0x0000);	// 0 dB, disable SPDIF pcm -11dB
			MHal_MAD_WriteMaskReg(REG_SOUND_SPK_MOD,0x0002,0x0000);				// 0 dB, disable MPEG AAC Main & AD -11dB
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0080,0x0000);	// 0 dB, AC3 Line Mode Decode, main & AD Not boost up +11dB
			break;

		case 1: 	//DVB RF mode
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0020,0x0020);	//-11dB, enable SPDIF pcm -11dB
			MHal_MAD_WriteMaskReg(REG_SOUND_SPK_MOD,0x0002,0x0000);				// 0 dB, disable MPEG AAC Main & AD -11dB
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0080,0x0080);	//+11dB, AC3 RF Mode Decode, main & AD boost up +11dB
			break;

		case 2: 	//DVB Line mode
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0020,0x0000);	// 0 dB, disable SPDIF pcm -11dB
			MHal_MAD_WriteMaskReg(REG_SOUND_SPK_MOD,0x0002,0x0002);				//-11dB, enable MPEG Main & AD -11dB
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0080,0x0000);	// 0 dB, AC3 Line Mode Decode, main & AD Not boost up +11dB
			break;

	#if 1	/* By LGE Yongchol.kee 100203 */
		case 4: 	//NewZealand line mode for AAC By LGE Yongchol.kee 100203
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0020,0x0020);	//-11dB, enable SPDIF pcm -11dB
			MHal_MAD_WriteMaskReg(REG_SOUND_SPK_MOD,0x0002,0x0000);				// 0 dB, disable MPEG AAC Main & AD -11dB
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0080,0x0000);	// 0 dB, AC3 Line Mode Decode, main & AD Not boost up +11dB
			break;
		default:	// like HDMI & EMP
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0020,0x0000);	// 0 dB, disable SPDIF pcm -11dB
			MHal_MAD_WriteMaskReg(REG_SOUND_SPK_MOD,0x0002,0x0000);				// 0 dB, disable MPEG AAC Main & AD -11dB
			MHal_MAD_WriteMaskReg(REG_M2D_MAILBOX_SPDIF_CTRL,0x0080,0x0000);	// 0 dB, AC3 Line Mode Decode, main & AD Not boost up +11dB
			break;
	#endif
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_DSPACK()
/// @brief \b Function \b Description: This routine is to report DSP reload ACK cmd.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD_GetReloadCodeAck(void)
{
    return(MHal_MAD_ReadByte(REG_MB_DE_ACK2));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_DSPACK()
/// @brief \b Function \b Description: This routine is to report DSP reload ACK cmd.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD_GetLoadCodeAck(void)
{
    return(MHal_MAD_ReadByte(REG_MB_DE_ACK1));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_Rpt_DTVES()
/// @brief \b Function \b Description: This routine is used to report AC3/MPEG stream if exist or not.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b MS_U8  :    the decoder status
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD_Rpt_DTVES(void)
{
    MS_U16   Rpt_DTVES_tmp;
    Rpt_DTVES_tmp = MHal_MAD_ReadReg(REG_MB_DEC_ID_STATUS);        // add one time read to avoid mailbox issue
    Rpt_DTVES_tmp = MHal_MAD_ReadReg(REG_MB_DEC_ID_STATUS);
    Rpt_DTVES_tmp >>= 8;
    return ((MS_U8)Rpt_DTVES_tmp);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_WMA_SetASFParm()
/// @brief \b Function \b Description: This routine will set WMA ASF Paramaters.
/// @param <IN>        \b NONE    : WMA_ASF_PARMTYPE, value
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_WMA_SetASFParm(WMA_ASF_PARMTYPE parm_type, MS_U32 value)
{
    switch ( parm_type )
    {
        case WMA_PARAMTYPE_VERSION:
            MDrv_MAD_Write_DSP_sram(0x1853, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_CHANNELS:
            MDrv_MAD_Write_DSP_sram(0x1813, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_SAMPLERATE:
            MDrv_MAD_Write_DSP_sram(0x1854, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_BYTERATE:
            MDrv_MAD_Write_DSP_sram(0x1858, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_BLOCKALIGN:
            MDrv_MAD_Write_DSP_sram(0x1852, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_ENCOPT:
            MDrv_MAD_Write_DSP_sram(0x1855, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_PARSINGBYAPP:
              MDrv_MAD_Write_DSP_sram(0x185E, value, DSP_MEM_TYPE_DM);
              break;
	    case WMA_PARAMTYPE_BITS_PER_SAMPLE:
            MDrv_MAD_Write_DSP_sram(0x1860, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_CHANNELMASK:
            MDrv_MAD_Write_DSP_sram(0x1861, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_DRC_PARAM_EXIST:
            MDrv_MAD_Write_DSP_sram(0x1862, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_DRC_RMS_AMP_REF:
            MDrv_MAD_Write_DSP_sram(0x1863, value, DSP_MEM_TYPE_DM);
            break;
        case WMA_PARAMTYPE_DRC_RMS_AMP_TARGET:
            MDrv_MAD_Write_DSP_sram(0x1864, value, DSP_MEM_TYPE_DM);
            break;
	    case WMA_PARAMTYPE_DRC_PEAK_AMP_REF:
            MDrv_MAD_Write_DSP_sram(0x1865, value, DSP_MEM_TYPE_DM);
            break;
	    case WMA_PARAMTYPE_DRC_PEAK_AMP_TARGET:
            MDrv_MAD_Write_DSP_sram(0x1866, value, DSP_MEM_TYPE_DM);
            break;

        default:
            break;
    }
}




////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD_Write_DSP_sram()
/// @brief \b Function \b Description: This routine is used to Write DSP internal sram value by IDMA
/// @param <IN>        \b dsp_addr    : DSP internal sram address
/// @param <IN>        \b value     : data want to write
/// @param <IN>        \b dsp_memory_type :    0-- write to DSP_MEM_TYPE_PM
///                                        1-- write to DSP_MEM_TYPE_DM
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  : TRUE/FALSE
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD_Write_DSP_sram(MS_U16 dsp_addr, MS_U32 value, AUDIO_DSP_MEMORY_TYPE dsp_memory_type)
{
    MS_U8   dat[3];

    if(audio_recovery)
        return FALSE;

MAD_LOCK();
    MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x00); // IDMA is used by DEC-DSP
    MHal_MAD_AbsWriteMaskByte(0x103c14, 0x01, 0x00); //DMA sel to RIU
    if((MDrv_MAD_CheckDecIdmaReady(AUD_CHK_DSP_READ_RDY) == FALSE)||(MDrv_MAD_CheckDecIdmaReady(AUD_CHK_DSP_WRITE_RDY) == FALSE))
    {
        MAD_UNLOCK();
        DBG_AUDIO_ERROR(printk("ERROR: DEC DSP IDMA Busy \n"));
        return FALSE;
    }

    dat[2] = H2BYTE(value);
    dat[1] = HIBYTE(value);
    dat[0] = LOBYTE(value);

    if (dsp_memory_type == DSP_MEM_TYPE_DM)
    {
        dsp_addr = (dsp_addr|0x8000);
    }
    MHal_MAD_WriteByte( REG_DEC_IDMA_WRBASE_ADDR_L, (MS_U8)(dsp_addr&0xFF));
    MHal_MAD_WriteByte( REG_DEC_IDMA_WRBASE_ADDR_H, (MS_U8)(dsp_addr>>8));
    MHal_MAD_WriteByte(REG_DEC_DSP_BRG_DATA_L,dat[1]);
    MHal_MAD_WriteByte(REG_DEC_DSP_BRG_DATA_H,dat[2]);
    //msleep(2);
    udelay(2);
    MHal_MAD_WriteByte(REG_DEC_DSP_BRG_DATA_L,dat[0]);
    MHal_MAD_WriteByte(REG_DEC_DSP_BRG_DATA_H,0x00);
MAD_UNLOCK();
    if (MDrv_MAD_CheckDecIdmaReady(AUD_CHK_DSP_WRITE_RDY)==FALSE)
    {
        DBG_AUDIO_ERROR(printk("ERROR: DEC DSP IDMA write data time out2 \n"));
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_Dis_MIUREQ()
/// @brief \b Function \b Description: This routine is to reset DVB1 MIU request command.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_Dis_MIUREQ(void)
{
    // Disable MIU Request
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0040 );        // disable
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0080, 0x0080 );          // reset MAD module
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_RSTMAD_DisEn_MIUREQ()
/// @brief \b Function \b Description: This routine is to reset DVB1 MIU request command.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_RSTMAD_DisEn_MIUREQ(void)
{
    // Disable MIU Request
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0040 );        // disable
    // Reset MAD
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0080, 0x0080 );          // reset MAD module
    msleep(1);
    // Set MAD
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0080, 0x00 );
    // Enable MIU Request
    MHal_MAD_WriteMaskReg(REG_DEC_AUD_DTRL, 0x0040, 0x0000 );
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MAD_DvbFLockSynthesizer_En()
/// @brief \b Function \b Description:  This function is used to set DSP IDMA.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD_DvbFLockSynthesizer_En(void)
{
    // enable force lock current DVB SYNC synthesizer freq
    MHal_MAD_WriteMaskReg(REG_AUDIO_INPUT_CFG, 0x0040, 0x0040 );
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Set_Cut_Boost
//  [Description]:
//      Set MS10 Cut and Boost scale
//  [Arguments]:
//boost
//0x2D98[0:3]              0000 : 0
//                                0001 : 20
//                                0010 : 40
//                                0011 : 60
//                                0100 : 80
//                                0101 : 100
//cut
//0x2D98[4:7]             0000 : 0
//                               0001 : 20
//                               0010 : 40
//                               0011 : 60
//                               0100 : 80
//                               0101 : 100
//  [Return]:
//      None
//  [Doxygen]
/// Set MS10 Cut and Boost scale.
/// @param scale \b IN:
//
//*******************************************************************************

void MDrv_MAD_Set_Cut_Boost(U8 scale)
{
    MHal_MAD_WriteMaskByte(REG_MB_DEC_CUT_BOOST, 0xFF, (scale/20)*16+(scale/20));
}

