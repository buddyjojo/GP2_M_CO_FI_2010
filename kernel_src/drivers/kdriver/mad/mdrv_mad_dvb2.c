//******************************************************************************
//  Copyright (c) 2003-2005 MStar Semiconductor, Inc.
//  All rights reserved.
//
//  [Module Name]:
//      DrvAuDvb2.c
//  [Abstract]:
//      This module contains code for Audio DVB2 driver
//      procedure and subroutin
//  [Author(s)]:
//      Desker Chuo
//  [Reversion History]:
//      Initial release:    15 July, 2005
//
//  [Doxygen]
/// @file DrvAuDvb2.h
/// @brief Subroutine for DVB2
/// @author MStarSemi Inc.
///
/// This module contains code for Audio DVB driver
/// procedure and subroutine.
///
///@par Example
///@code
///@endcode
//*******************************************************************************

// Internal
#include <linux/kernel.h>
#include <linux/mm.h>
#include "mst_devid.h"
#include "mst_utility.h"
#include "mdrv_mad_process.h"
#include "mdrv_mad_dvb2.h"
#include "mhal_mad_reg.h"
#include "mdrv_mad_common.h"

#define MAD_DEBUG_P1(msg) //msg
#define MAD_DEBUG_P2(msg) //msg
#define MAD_DEBUG_P3(msg) //msg
#define DBG_AUDIO_ERROR(msg) msg
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
extern MS_U8  g_Dsp2CodeType;
extern MS_BOOL g_bAudio_loadcode_from_dram;


extern U8  g_DspCodeType;
extern U32 BinBaseAddress;
extern U32 MadBaseBufferAdr;
extern U32 DSPBinBaseAddress[2], DSPMadBaseBufferAdr[2];
extern U8 dtv_mode_en;

extern U32 _BT_RunningUp;
extern MDRV_BT_T BT_information;
extern U32 _BT_Count;
extern spinlock_t                      _mad_spinlock;

extern BOOL audio_recovery ;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

static MS_U16   CH1_setting_tmp;
static MS_U16   CH2_setting_tmp;
static MS_U16   CH3_setting_tmp;
static MS_U16   CH4_setting_tmp;
static MS_U16   CH5_setting_tmp;
static MS_U16   CH6_setting_tmp;
static MS_U16   CH7_setting_tmp;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
MS_U8  DVB2_IP_CONTROL_BIT[] =
{
    255,                         //MPEG_AD
    11,                          //AC3_AD
    12,                          //AC3P_AD
    255,                         //AAC_AD
    13,                          //DDE
    255,                         //SBC
    255,                         //MP3
    255,                         //MPEG_EN
    255,                         //NONE
    255,                         //AC3BP
    255,                         //NONE
};


//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD2_SetSystem
//  [Description]:
//      Set DVB decoder system
//  [Arguments]:
//      None
//  [Return]:
//      TRUE / FALSE
//  [Doxygen]
/// Set DVB decoder system .
//*******************************************************************************
MS_BOOL MDrv_MAD2_SetSystem(MS_U8 u8System_type)
{
    MS_U8 u8Reload_dspType = 0xFF;
    MS_BOOL parser2_sel = FALSE;
	
    MAD_DEBUG_P1(printk("MDrv_MAD2_SetSystem:(0x%x)\r\n",u8System_type));

    if(DVB2_IP_CONTROL_BIT[u8System_type] != 255)
    {
      /*Temp mask by Cathy   if(CheckIPControlBit(DVB2_IP_CONTROL_BIT[u8System_type]) == 0)
        {
             printf("not licensed\r\n");
             return 0;
        }*/
    }

    MDrv_MAD_SetIsDtvFlag(TRUE);
    switch( u8System_type )
    {
        case AU_DVB2_SYS_MPEG_AD:
            u8Reload_dspType = AU_DVB2_STANDARD_MPEG_AD;
            break;

        case AU_DVB2_SYS_AC3_AD:
            u8Reload_dspType = AU_DVB2_STANDARD_AC3_AD;
            break;

        case AU_DVB2_SYS_AC3P_AD:
            u8Reload_dspType = AU_DVB2_STANDARD_AC3P_AD;
            parser2_sel = TRUE;
            break;

        case AU_DVB2_SYS_AAC_AD:
            u8Reload_dspType = AU_DVB2_STANDARD_AAC_AD;
            break;

        case AU_DVB2_SYS_CV3:
            u8Reload_dspType = AU_DVB2_ADVSND_CV3;
         break;

        case AU_DVB2_ADVSND_NONE:
            u8Reload_dspType = AU_DVB2_ADVSND_NONE;
	     break;

        case AU_DVB2_SYS_NONE:
            u8Reload_dspType = AU_DVB2_NONE;
	     break;			

        case AU_DVB2_SYS_SBC:
            u8Reload_dspType = AU_DVB2_ADVSND_SBC;
         break;

        case AU_DVB2_SYS_MP3:
            u8Reload_dspType = AU_DVB2_STANDARD_MP3;
            break;

        case AU_DVB2_SYS_MPEG_EN:
            u8Reload_dspType = AU_DVB2_STANDARD_MPEG_EN;
            break;

        case AU_DVB2_SYS_DTSE:
            u8Reload_dspType = AU_DVB2_STANDARD_DTSE;
            break;

        case AU_DVB2_SYS_XPCM:
            u8Reload_dspType = AU_DVB2_STANDARD_XPCM;
            break;

        default:
            DBG_AUDIO_ERROR(printk("[ERROR] MDrv_MAD2_SetSystem::Unknown decoder type [0x%x]\n",u8System_type));
            break;

   }
   if(parser2_sel == TRUE)
   	MHal_MAD_WriteMaskReg(0x2A7E, 0x8000, 0x8000);
   else
   	MHal_MAD_WriteMaskReg(0x2A7E, 0x8000, 0x0000);

   if ( u8Reload_dspType == 0xFF )
        return FALSE;
   else
        return(MDrv_MAD2_ReLoadCode(u8Reload_dspType));
}

MS_BOOL MDrv_MAD2_ReLoadCode(U8 u8Type)
{
    MS_BOOL    ret_status;
    MS_U8  Dvb2DecCmd_tmp;

    MAD_DEBUG_P1(printk("MDrv_MAD2_ReLoadCode type=%x\r\n",u8Type));
    MAD_LOCK();
    Dvb2DecCmd_tmp = MDrv_MAD_Se_Status();
    MDrv_MAD2_SetDecCmd(0);                      // Stop

    //if(g_Dsp2CodeType == type)           // if type is the same, don't reload again
    //    return TRUE;
    if((u8Type & 0xF0) !=0x40)
    {
        MDrv_MAD_SetDSP2CodeType(u8Type);
        MDrv_MAD_SIF_SetDspCodeType(u8Type);
    }
    // Reset MAD module
    MDrv_MAD2_DisEn_MIUREQ();

    MAD_DEBUG_P2(printk("type=%x\r\n",u8Type));

    ret_status = MDrv_MAD_Alg2ReloadCode(u8Type);

    MDrv_MAD2_SetDecCmd(Dvb2DecCmd_tmp);
    MAD_UNLOCK();	
    return ret_status;
}

BOOL MDrv_MAD_Alg2ReloadCode(U8 decoder_type)
{
    int time_out;

    MDrv_MAD2_SetDspIDMA();
    // Enter MCU/DSP hand-shake
    if((decoder_type & 0xF0) == 0x20 ||(decoder_type & 0xF0) == 0x40)
    {
	 MDrv_MAD2_SetMcuCmd(0xF4);
    }
    else
    {
        MDrv_MAD2_SetMcuCmd(0xF0);
    }

    // PIO[8] interrupt
    MDrv_MAD2_TriggerPIO8();

    //Wait Dsp Start reload Ack
    time_out = 0;
    while(time_out++<2000)
    {
        if(MDrv_MAD2_GetReloadCodeAck() == 0x33)
            break;
        //msleep(1);
        udelay(1);
    }
    if(time_out>=2000)
    {
        DBG_AUDIO_ERROR(printk("  DSP Reload timeOut1: %d\r\n",time_out));
        return FALSE;
    }

    // Change to IDMA Port
    MDrv_MAD2_SetDspIDMA();

    // Start to Reload DSP code
    MDrv_MAD_SetDSP2CodeType(decoder_type);
    if(decoder_type == AU_DVB2_STANDARD_AC3P_AD)
        decoder_type = AU_DVB2_NONE;
    MDrv_MAD_DspLoadCode(decoder_type);

    // Enter MCU/DSP hand-shake

    if((decoder_type & 0xF0) == 0x20 ||(decoder_type & 0xF0) == 0x40)
    {
	 MDrv_MAD2_SetMcuCmd(0xF5);
    }
    else
    {
        MDrv_MAD2_SetMcuCmd(0xF1);
    }

        MDrv_MAD2_TriggerPIO8();

    // Wait Dsp End Reload Ack
    time_out = 0;
    while(time_out++<3000)
    {
        if(MDrv_MAD2_GetReloadCodeAck() == 0x77)
            break;
        //msleep(1);
        udelay(1);
    }

    if(time_out>=3000)
    {
        DBG_AUDIO_ERROR(printk("  DSP Reload timeOut2: %d\r\n",time_out));
        return FALSE;
    }

    MDrv_MAD2_SetMcuCmd(0x00); // In T3, clear 0x2DDC after reload finish
    MAD_DEBUG_P2(printk("MDrv_MAD_Alg2ReloadCode finish\r\n"));

    return TRUE;
}
//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Set_Dolby_AD_DRC_Mode
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

void MDrv_MAD_Set_Dolby_AD_DRC_Mode(U8 DRC_mod)//KH
{
    if(DRC_mod == 1)
    {
        MHal_MAD_WriteMaskByte(REG_DBG_DATA_L, 0x80, 0x80);       // RF Mod
    }
    else
    {
        MHal_MAD_WriteMaskByte(REG_DBG_DATA_L, 0x80, 0x00);       // Line Mod
    }
}

//******************************************************************************
//
//  [Function Name]:
//      MDrv_MAD_Set_Dolby_AD_Downmix_Mode
//  [Description]:
//      Set Dolby AC3/AC3+ Downmix Mode
//  [Arguments]:
//      Downmix mode type :
//          0: LtRt mode
//          1: LoRo mode
//  [Return]:
//      None
//  [Doxygen]
/// Set Dolby AC3/AC3+ Downmix Mode (LtRt/LoRo).
/// @param dmix_mod \b IN:
///   - 0: LtRt mode
///   - 1: LoRo mode
//
//*******************************************************************************
void MDrv_MAD_Set_Dolby_AD_Downmix_Mode(U8 dmix_mod)//KH
{
    if(dmix_mod == 1)
    {
        MHal_MAD_WriteMaskByte(REG_DBG_DATA_L, 0x40, 0x40);       // LoRo Mod
    }
    else
    {
        MHal_MAD_WriteMaskByte(REG_DBG_DATA_L, 0x40, 0x00);       // LtRt Mod
    }
}

void MDrv_MAD_Dvb2_Play_BT(U8 BT_play)
{
    //printk("MDrv_MAD_Dvb_setDecCmd:(0x%x)\r\n",BT_play);
#if 0//CHECK
	if( (BT_play == AU_DVB2_BT_PAUSE) && (_BT_RunningUp == 1))
	{
	    MHal_MAD_SeWriteRegMask(0x2D24,0x0700,0x0000);  // stop upload BT
	}

  	if( (BT_play == AU_DVB2_BT_PLAY) && (_BT_RunningUp == 1))
	{
	    if(BT_information.bSBCOnOff == 1)
    	{
		   	MDrv_MAD2_ReLoadCode(AU_DVB2_STANDARD_SBC);
		       MHal_MAD_SeWriteRegMask(0x2D24,0x0700,0x0300);  // start upload BT with SBC
			_BT_Count=1;
	    }
	    else
	    MHal_MAD_SeWriteRegMask(0x2D24,0x0700,0x0100);  // start upload BT with PCM
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_SetMemInfo()
/// @brief \b Function \b Description:  This routine is used to set the SE-DSP memory information
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_SetMemInfo(void)
{
    MS_U32 DDR_ADDR;
    MS_U32 u32DSPMadBaseBufferAdr;

    u32DSPMadBaseBufferAdr=MDrv_MAD_GetDspMadBaseAddr(DSP_SE);
    MAD_DEBUG_P1(printk("MDrv_MAD2_SetMemInfo[0x%08lX]\n\r", u32DSPMadBaseBufferAdr));

    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x00FF, 0x0000);//0x2DE0                //reset AUD register

    // Sound Effect DMA
    // ICACHE BASE
    DDR_ADDR = (u32DSPMadBaseBufferAdr >> 12);
    MHal_MAD_WriteReg(REG_SE_DSP_ICACHE_BASE_L,DDR_ADDR); //0x2D10

    // MAD BASE
    DDR_ADDR = (u32DSPMadBaseBufferAdr >> 12);
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0xFFFF, 0x0000);//0x2DE2,0x2DE3
    MHal_MAD_WriteReg(REG_SE_MAD_OFFSET_BASE_H,DDR_ADDR); //0x2DE4

    // Config as ES1 (AD / AC3 encode)
    MHal_MAD_WriteMaskReg(REG_SE_MCFG, 0x00FF, 0x0002);
    MHal_MAD_WriteReg(REG_SE_MBASE_H,0x00A8);                       // ES1 base : 0x00A800
    MHal_MAD_WriteMaskReg(REG_SE_MSIZE_H, 0xFF00, 0x0700);          // ES1 size : 32KB

    // Config as PCM1 (AD / AC3 encode)
    MHal_MAD_WriteMaskReg(REG_SE_MCFG, 0x00FF, 0x0004);
    MHal_MAD_WriteReg(REG_SE_MBASE_H,0x0000);                       // PCM1 base : 0x000000
    MHal_MAD_WriteMaskReg(REG_SE_MSIZE_H, 0xFF00, 0x1700);          // PCM1 size : 96KB
  // Reset MAD & ReEnable MIU Request for SND-DSP
    MDrv_MAD2_RSTMAD_DisEn_MIUREQ();

    //printk("ICACHE addr = %2bx\n\r", MHal_MAD_SeReadByte(0x2D10));
    //printk("MAD Base addr = %2bx\n\r", MHal_MAD_SeReadByte(0x2DE4));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_SetDecCmd()
/// @brief \b Function \b Description: This routine is to set  the DVB2 decoder command.
/// @param <IN>        \b u8DecCmd    :    0--STOP
///                                    1--PLAY
///                                    1--RESYNC
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_SetDecCmd(MS_U8 u8DecCmd)
{
    MAD_DEBUG_P1(printk("MDrv_MAD2_SetDecCmd:(%x)\r\n",u8DecCmd));

    if(MDrv_MAD_GetDSP2CodeType() == AU_DVB2_STANDARD_AC3P_AD)
    {
        MHal_MAD_WriteMaskByte(REG_SE_DECODE_CMD, 0x1F, 0);	
	if(u8DecCmd == 0)
	    MHal_MAD_WriteMaskByte(0x2D80, 0xFF, 0x00);
	else
	    MHal_MAD_WriteMaskByte(0x2D80, 0xFF, 0x23);	
    }
    else
    MHal_MAD_WriteMaskByte(REG_SE_DECODE_CMD, 0x1F, u8DecCmd);

}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_GetDecCmd()
/// @brief \b Function \b Description: This routine is to get  the DVB1 decode command.
/// @param <IN>        \b u8DecCmd    :
///                                    0--STOP
///                                    1--PLAY
///                                    1--RESYNC
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD2_GetDecCmd(void)
{
    MS_U8   dec2cmd_status;

    dec2cmd_status = MHal_MAD_ReadByte(REG_SE_DECODE_CMD);
    return(dec2cmd_status);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_SetFreeRun()
/// @brief \b Function \b Description: This routine is to set  the DVB2 decoder in free run or AV-sync mode.
/// @param <IN>        \b u8FreeRun    :    0--normal AV sync
///                                    1--free-run mode
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_SetFreeRun(MS_U8 u8FreeRun)
{
    MAD_DEBUG_P1(printk("MDrv_MAD2_SetFreeRun:(%x)\r\n",u8FreeRun));
    if( u8FreeRun >= 2 )
    {
        DBG_AUDIO_ERROR(printk("Invalid mode\r\n"));
    }

    MHal_MAD_WriteMaskByte(REG_SE_DECODE_CMD, 0x20, u8FreeRun<<5 );
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_Read_DSP_sram()
/// @brief \b Function \b Description: This routine is used to Read DSP internal sram value by IDMA
/// @param <IN>        \b dsp_addr    : DSP internal sram address
/// @param <IN>        \b dm    :    0--DSP_MEM_TYPE_PM
///                                1--DSP_MEM_TYPE_DM
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b value :    MS_U32 sram value
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_U32 MDrv_MAD2_Read_DSP_sram(MS_U16 dsp_addr,AUDIO_DSP_MEMORY_TYPE dsp_memory_type)
{
    MS_U8   dat[3];
    MS_U32 value=0;

    if(audio_recovery)
        return FALSE;
    
    MAD_LOCK();
    MHal_MAD_AbsWriteMaskByte(0x103c14, 0x01, 0x00);
    MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x01); // IDMA is used by
    // check IDMA busy or not
    if((MDrv_MAD_CheckSeIdmaReady(AUD_CHK_DSP_READ_RDY) == FALSE)||(MDrv_MAD_CheckSeIdmaReady(AUD_CHK_DSP_WRITE_RDY) == FALSE))
    {
        DBG_AUDIO_ERROR(printk("ERROR: SE DSP IDMA Busy \n"));
	 MAD_UNLOCK();	
        return FALSE;
    }

    if (dsp_memory_type == DSP_MEM_TYPE_DM)
    {
        dsp_addr = (dsp_addr|0x8000);                   //select DM
    }

    MHal_MAD_WriteByte( REG_SE_IDMA_RDBASE_ADDR_L, (MS_U8)(dsp_addr&0xFF));
    MHal_MAD_WriteByte( REG_SE_IDMA_RDBASE_ADDR_H, (MS_U8)(dsp_addr>>8));
    MHal_MAD_WriteMaskByte(REG_SE_IDMA_CTRL0, 0x08, 0x08);        //0x2D00[3]

    if (MDrv_MAD_CheckSeIdmaReady(AUD_CHK_DSP_READ_RDY)==FALSE)
    {
        MAD_UNLOCK();
        DBG_AUDIO_ERROR(printk("ERROR: SE DSP IDMA read data time out \n"));
        return FALSE;
    }

    dat[2] = MHal_MAD_ReadByte(REG_SE_IDMA_RDDATA_H_1);
    dat[1] = MHal_MAD_ReadByte(REG_SE_IDMA_RDDATA_H_0);
    dat[0] = MHal_MAD_ReadByte(REG_SE_IDMA_RDDATA_L);

    value = ((MS_U8)dat[2] << 16) | ((MS_U8)dat[1] << 8) | (MS_U8)dat[0];
    MAD_UNLOCK();	
    return value;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_Write_DSP_sram()
/// @brief \b Function \b Description: This routine is used to Write DSP internal sram value by IDMA
/// @param <IN>        \b dsp_addr    : DSP internal sram address
/// @param <IN>        \b value     : data want to write
/// @param <IN>        \b dm        :    0-- write to DSP_MEM_TYPE_PM
///                                        1-- write to DSP_MEM_TYPE_DM
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MAD2_Write_DSP_sram(MS_U16 dsp_addr, MS_U32 value, AUDIO_DSP_MEMORY_TYPE dsp_memory_type)
{
    MS_U8   j, dat[3];

    if(audio_recovery)
        return FALSE;

    MAD_LOCK();
    MHal_MAD_AbsWriteMaskByte(0x103c14, 0x01, 0x00);//DMA sel to RIU
    MHal_MAD_WriteMaskByte(0x2A7E, 0x01, 0x01); // IDMA is used by SE-DSP
    if((MDrv_MAD_CheckSeIdmaReady(AUD_CHK_DSP_READ_RDY) == FALSE)||(MDrv_MAD_CheckSeIdmaReady(AUD_CHK_DSP_WRITE_RDY) == FALSE))
    {
        MAD_UNLOCK();
        DBG_AUDIO_ERROR(printk("SE DSP IDMA Busy \n"));
        return FALSE;
    }

    dat[2] = H2BYTE(value);
    dat[1] = HIBYTE(value);
    dat[0] = LOBYTE(value);

    if (dsp_memory_type == DSP_MEM_TYPE_DM)
    {
        dsp_addr = (dsp_addr|0x8000);
    }

    MHal_MAD_WriteByte( REG_SE_IDMA_WRBASE_ADDR_L, (MS_U8)(dsp_addr&0xFF));
    MHal_MAD_WriteByte( REG_SE_IDMA_WRBASE_ADDR_H, (MS_U8)(dsp_addr>>8));

    MHal_MAD_WriteByte(REG_SE_DSP_BRG_DATA_L,dat[1]);
    MHal_MAD_WriteByte(REG_SE_DSP_BRG_DATA_H,dat[2]);
    for(j=0;j<2;j++);//for delay only
    MHal_MAD_WriteByte(REG_SE_DSP_BRG_DATA_L,dat[0]);
    MHal_MAD_WriteByte(REG_SE_DSP_BRG_DATA_H,0x00);
    if (MDrv_MAD_CheckSeIdmaReady(AUD_CHK_DSP_WRITE_RDY)==FALSE)
    {
        MAD_UNLOCK();
        DBG_AUDIO_ERROR(printk("ERROR: SE DSP IDMA write data time out \n"));
        return FALSE;
    }
    MAD_UNLOCK();	
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_DisEn_MIUREQ()
/// @brief \b Function \b Description: This routine is to reset DVB2 MIU request command.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_DisEn_MIUREQ(void)
{
    // Disable MIU Request
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0040, 0x0040 );
    //msleep(1);
    udelay(1);
    // Enable MIU Request
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0040, 0x0000 );
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_DisMiuReq()
/// @brief \b Function \b Description: This routine is to reset DVB2 MIU request command.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_DisMiuReq(void)
{
    // Disable MIU Request
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0040, 0x0040 );        // disable
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0080, 0x0080 );          // reset MAD module
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_RSTMAD_DisEn_MIUREQ()
/// @brief \b Function \b Description: This routine is to reset DVB2 MIU request command.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_RSTMAD_DisEn_MIUREQ(void)
{
    // Disable MIU Request
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0040, 0x0040 );        // disable
    // Reset MAD
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0080, 0x0080 );          // reset MAD module
    msleep(1);
    // Set MAD
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0080, 0x0000 );
    // Enable MIU Request
    MHal_MAD_WriteMaskReg(REG_SE_AUD_DTRL, 0x0040, 0x0000 );
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_Backup_pathreg()
/// @brief \b Function \b Description: This routine used to backup path register
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_Backup_pathreg(void)
{
    CH1_setting_tmp = MHal_MAD_SeReadReg(0x2C64);
    CH2_setting_tmp = MHal_MAD_SeReadReg(0x2C66);
    CH3_setting_tmp = MHal_MAD_SeReadReg(0x2C68);
    CH4_setting_tmp = MHal_MAD_SeReadReg(0x2C6A);
    CH5_setting_tmp = MHal_MAD_SeReadReg(0x2C76);
    CH6_setting_tmp = MHal_MAD_SeReadReg(0x2C78);
    CH7_setting_tmp = MHal_MAD_SeReadReg(0x2C7A);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_Restore_pathreg()
/// @brief \b Function \b Description: This routine used to restore path register
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_Restore_pathreg(void)
{
    MHal_MAD_SeWriteReg(0x2C64, CH1_setting_tmp);
    MHal_MAD_SeWriteReg(0x2C66, CH2_setting_tmp);
    MHal_MAD_SeWriteReg(0x2C68, CH3_setting_tmp);
    MHal_MAD_SeWriteReg(0x2C6A, CH4_setting_tmp);
    MHal_MAD_SeWriteReg(0x2C76, CH5_setting_tmp);
    MHal_MAD_SeWriteReg(0x2C78, CH6_setting_tmp);
    MHal_MAD_SeWriteReg(0x2C7A, CH7_setting_tmp);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_EnableChIRQ()
/// @brief \b Function \b Description: This routine used to disable ch irq
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE    :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_EnableChIRQ(MS_BOOL bEnable)
{
    if(bEnable)
    {
        MHal_MAD_WriteMaskByte(REG_AUDIO_IRQ_CONTROL1, 0xF0, 0x00);     // Enable CH1 ~ CH4 IRQ
        MHal_MAD_WriteMaskByte(REG_AUDIO_IRQ_CONTROL1_2, 0x10, 0x00);     // Enable CH5 IRQ
    }
    else
    {
        MHal_MAD_WriteMaskByte(REG_AUDIO_IRQ_CONTROL1, 0xF0, 0xF0);     // Mask CH1 ~ CH4 IRQ
        MHal_MAD_WriteMaskByte(REG_AUDIO_IRQ_CONTROL1_2, 0x10, 0x10);     // Mask CH5 IRQ
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_SetDspIDMA()
/// @brief \b Function \b Description:  This function is used to set DSP IDMA.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_SetDspIDMA(void)
{
    MHal_MAD_WriteMaskByte(REG_SE_IDMA_CTRL0,0x00FF, 0x0003);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_TriggerPIO8()
/// @brief \b Function \b Description:  This function is used to trigger PIO8 init.
/// @param <IN>        \b NONE    :
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_TriggerPIO8(void)
{
    MHal_MAD_WriteMaskByte(REG_SE_IDMA_CTRL0, 0x0020, 0x0020);
    //msleep(2);
    MHal_MAD_WriteMaskByte(REG_SE_IDMA_CTRL0, 0x0020, 0x0000);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_GetReloadCodeAck()
/// @brief \b Function \b Description: This routine is to report DSP reload ACK cmd.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD2_GetReloadCodeAck(void)
{
    return(MHal_MAD_ReadByte(REG_MB_SE_ACK2));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_GetLoadCodeAck()
/// @brief \b Function \b Description: This routine is to report DSP reload ACK cmd.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
MS_U8 MDrv_MAD2_GetLoadCodeAck(void)
{
    return(MHal_MAD_ReadByte(REG_MB_SE_ACK1));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_SetMcuCmd()
/// @brief \b Function \b Description: This routine is to write MCU cmd.
/// @param <IN>        \b NONE  :
/// @param <OUT>       \b NONE  :
/// @param <RET>       \b NONE  :
/// @param <GLOBAL>    \b NONE  :
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_SetMcuCmd(MS_U8 cmd)
{
    MHal_MAD_WriteMaskReg(REG_MB_SE_CMD1, 0xFF00, (MS_U16)cmd<<8);
}

//******************************************************************************
//  [Function Name]:
//      MDrv_MAD_AuReadTimeStamp
//  [Description]:
//      This routine return the stmestamp while decoding file format.
//  [Arguments]:
//      None
//*******************************************************************************
U32 MDrv_MAD2_ReadTimeStamp(void)
{
    U32 w16_1 = (U16)MHal_MAD_ReadReg(REG_MB_TIME_STAMP_DSP2_SEC);                // upper 16 bit
    U32 w16_2 = ((U16)MHal_MAD_ReadReg(REG_MB_TIME_STAMP_DSP2_4ms) & 0x00FF);        // lower  8 bit
    U32 timestamp = (w16_1*1000) + (w16_2<<2);
    MAD_DEBUG_P2(printk("sec = 0x%x\r\n", w16_1));
    MAD_DEBUG_P2(printk("sec = 0x%x\r\n", w16_2));
    MAD_DEBUG_P2(printk("sec = 0x%x\r\n\r\n", timestamp));
    return timestamp;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MAD2_RebootDsp(void)
/// @brief \b Function \b Description:  This routine reboot Sound Effect DSP.
////////////////////////////////////////////////////////////////////////////////
void MDrv_MAD2_RebootDsp(void)
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
    MDrv_MAD_SIF_ENABLE_CHANNEL(0);  // Audio SIF channel enable setting -> disable
    dec2cmd_status = MDrv_MAD2_GetDecCmd();
    MDrv_MAD2_SetDecCmd(drvMAD_STOP);
    MDrv_MAD2_SetMemInfo();

    // load DSP2 code

    MDrv_MAD_DspLoadCode(u8Dsp2CodeType);                // Se
    MDrv_MAD_DspLoadCode(AU_SND_EFFECT);   // AdvSe

    MDrv_MAD_SeSystemLoadCode();                                            // system segment

    MDrv_MAD2_EnableChIRQ(TRUE);
    MDrv_MAD_SIF_ENABLE_CHANNEL(1);  // Audio SIF channel enable setting -> enable
    msleep(500);
    MDrv_MAD2_SetDecCmd(dec2cmd_status);

    MDrv_MAD2_Restore_pathreg();
}

