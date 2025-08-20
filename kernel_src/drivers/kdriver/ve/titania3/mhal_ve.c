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
#define _MHAL_VE_C
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ve.c
/// This file contains the Mstar driver for TVEncoder
/// @author MStar Semiconductor Inc.
/// @brief  TVEncoder module
///////////////////////////////////////////////////////////////////////////////


#include <linux/kernel.h>	/* printk() */
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/delay.h>	/* msleep_interruptible() */
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "mst_devid.h"
#include "mdrv_system.h"
#include "mdrv_scaler_st.h"	// LGE drmyung 080903

#include "chip_int.h"
#include "chip_setup.h"
#include "mhal_ve.h"
#include "mhal_ve_reg.h"
#include "mhal_ve_tbl.h"
#include "Board.h"
// -----------------------------------------------------------------------------
//                     Macro
// -----------------------------------------------------------------------------
//#define MSG_DRV_VE(x)  x

#if !defined(BIT0) && !defined(BIT1)
#define BIT0                        0x00000001
#define BIT1                        0x00000002
#define BIT2                        0x00000004
#define BIT3                        0x00000008
#define BIT4                        0x00000010
#define BIT5                        0x00000020
#define BIT6                        0x00000040
#define BIT7                        0x00000080
#define BIT8                        0x00000100
#define BIT9                        0x00000200
#define BIT10                       0x00000400
#define BIT11                       0x00000800
#define BIT12                       0x00001000
#define BIT13                       0x00002000
#define BIT14                       0x00004000
#define BIT15                       0x00008000
#endif

#define VE_NTSC_FRAME_SIZE  ((U32)(720ul * 480ul)>>2) // 720*480*16bits/64bits
#define VE_PAL_FRAME_SIZE   ((U32)(720ul * 576ul)>>2) // 720*576*16bits/64bits
#define VE_FRC_TOLERANT     10

#define VE_V_SCALE_DOWN_RATIO(IN, OUT)  ((U32)(OUT-3) * 2048ul / (U32)(IN-3))
#define VE_H_SCALE_DOWN_RATIO(IN, OUT)  ((U32)(OUT) * 2048ul / (U32)(IN))

//#define MSG_DRV_VE(x)       x
#define ENABLE_VE_SUBTITLE  1
#define ENABLE_VE_FIELD_INV 1


// -----------------------------------------------------------------------------
// Global variable
// -----------------------------------------------------------------------------
MS_TVENCODER_INFO   g_VEInfo;
typedef struct{
    EN_INPUT_PORT_TYPE enInputPortType;
} stSystemInfoType;

static stSystemInfoType stSystemInfo =
    { INPUT_PORT_MS_DTV };

static U32    u32VE_MIU_BASE = 0;
static U32    u32VEBaseLength = 0;

#if 0 /* unused variable. by LGE. jjab. */
static U32    tt_ve_addr=0;
static U32    tt_ve_addr_orig;
#endif
static U16    tt_ve_addr_hi;
static U16    tt_ve_temp_addr;

static U8     firstSend=1;  // Mstar andy 080917
// static EN_INPUT_PORT_TYPE _gInputPortType ;

extern SC_INPUT_MUX_t	g_SCInputMuxInfo;	// LGE drmyung 080903
//------------------------------------------------------------------------------
// Local Variable
//------------------------------------------------------------------------------
static B16 _bNeedClear = FALSE;

//------------------------------------------------------------------------------
//   Local Functions
//------------------------------------------------------------------------------

static void Drv_TVEncoder_Write(U16 u16Addr, U16 u16Data)
{
    TVEncoder_REG(u16Addr) = u16Data;
}

static void Drv_TVEncoder_WriteBit(U16 u16Addr, BOOL bValue, U16 u16Pos)
{
    if(bValue)
        TVEncoder_REG(u16Addr) |= u16Pos;
    else
        TVEncoder_REG(u16Addr) &= (~u16Pos);
}

#if 0
static void Drv_TVEncoder_Write4Byte(U16 u16Addr, MS_U32 u32Data)
{
    TVEncoder_REG(u16Addr)   = u32Data & 0xFFFF;
    TVEncoder_REG(u16Addr+4) = (u32Data>>16);
}
#endif



static void Drv_TVEncoder_WriteMask(U16 u16Addr, U16 u16Data, U16 u16Mask)
{
    U16 u16Tmp;
    u16Tmp = TVEncoder_REG(u16Addr) & (~u16Mask);
    TVEncoder_REG(u16Addr) =  u16Tmp | (u16Data & u16Mask);
}

static void Drv_TVEncoder_WriteTbl(pRegUnitType pTable)
{
    S16 s16Regidx;
    U16 u16RegVal;

    while(1)
    {
        s16Regidx = pTable->s16Idx;
        if(s16Regidx == _END_OF_TBL_)
            break;

        if(pTable->s16Idx & BK_LOW_BIT)
        {
            u16RegVal = TVEncoder_REG(s16Regidx & BK_LH_MASK) & 0xFF00;
            u16RegVal |= pTable->u16Val & 0x00FF;
            TVEncoder_REG(s16Regidx & BK_LH_MASK) = u16RegVal;
        }
        else if(pTable->s16Idx & BK_HIGH_BIT)
        {
            u16RegVal = TVEncoder_REG(s16Regidx & BK_LH_MASK) & 0x00FF;
            u16RegVal |= pTable->u16Val & 0xFF00;
            TVEncoder_REG(s16Regidx & BK_LH_MASK) =  u16RegVal;
        }
        else
        {
            TVEncoder_REG(s16Regidx) = pTable->u16Val;
        }
        pTable++;
    }
}


// field 0
//static U8 g_WSSDataSent= 0; /* unused variable. by LGE. jjab. */
static U8 g_VPSDataExist=0;	// MStar andy 081125
static U8 g_WSSDataExist=0;
// a ring buffer
#define MAX_CC_DATA 32
static U32 g_CCData[MAX_CC_DATA];
static U8 g_CCReadPtr=0;
static U8 g_CCWritePtr=0;
static irqreturn_t _MHal_VE_ISR0(int irq, void *dev_id)
{
    //U8 u8Key, u8RepeatFlag;
    //VE_HAL_DBG("0");
    //Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), 0x00, 0x00FF);
    /*
    g_WSSDataSent = TVEncoder_REG(BK_VE_ENC(0x2E)) & BIT2;
    g_WSSDataExist = 0;
    if (g_WSSDataSent)
        VE_HAL_DBG("%01x", TVEncoder_REG(BK_VE_ENC(0x3B)) & 0x000f);*/

    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), (g_WSSDataExist ? (BIT7|BIT2) : (BIT7)), BIT7|BIT2);		// lemonic LGE 20080929
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), (g_VPSDataExist ? (BIT7|BIT1) : (BIT7)), BIT7|BIT1);	// MStar andy 081125
    //Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7, BIT7|BIT2);
    return IRQ_HANDLED;
}
static irqreturn_t _MHal_VE_ISR1(int irq, void *dev_id)
{
    //U8 u8Key, u8RepeatFlag;
    //VE_HAL_DBG("1");
/*    if (g_WSSDataSent && !g_WSSDataExist) {
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7, BIT7|BIT2);
        VE_HAL_DBG("C");
    }*/
    //Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7, BIT7|BIT2);
    if (g_CCReadPtr != g_CCWritePtr) {
        Drv_TVEncoder_Write(BK_VE_ENC(0x2B), g_CCData[g_CCReadPtr%MAX_CC_DATA] & 0x0000ffff );
        Drv_TVEncoder_Write(BK_VE_ENC(0x2C), g_CCData[g_CCReadPtr%MAX_CC_DATA] >> 16 );
        //MDrv_WriteByte(H_BK_VE_ENC(0x3B), 0x00);
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7|BIT0, BIT7|BIT0);
        //VE_HAL_DBG("Read %x\n", g_CCReadPtr);
        g_CCReadPtr++;
    } else {
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7, BIT7|BIT0);  //clear
    }

    return IRQ_HANDLED;
}

//------------------------------------------------------------------------------
/// set the ipmux for VE
///
/// @param  -enInputPortType \b IN: input port type of VE
/// @return none
//------------------------------------------------------------------------------

void _MHal_VE_Set_IPMux(EN_INPUT_PORT_TYPE enInputPortType)
{
    U8 u8Clk_Mux, u8Data_Mux;
    VE_HAL_DBG("_MHal_VE_Set_IPMux\n");
#if 0   // titania bringup
    if(IsUseAnalogPort(enInputPortType))
    {
        u8Clk_Mux  = CKG_VE_IN_CLK_ADC;
        u8Data_Mux = VE_IPMUX_ADC_A;
    }
    else if(IsUseHdmiPort(enInputPortType))
    {
        u8Clk_Mux  = CKG_VE_IN_CLK_DVI;
        u8Data_Mux = VE_IPMUX_HDMI_DVI;
    }
    else if(IsUseDigitalPort(enInputPortType))
    {
        u8Clk_Mux  = CKG_VE_IN_CLK_VD;
        u8Data_Mux = VE_IPMUX_VD;
    }
    else if(IsUseExtVDPort(enInputPortType))
    {
        u8Clk_Mux  = CKG_VE_IN_CLK_EXT_DI;
        u8Data_Mux = VE_IPMUX_EXT_VD;
    }
    else if(IsUseMVDPort(enInputPortType))
    {
        u8Data_Mux = VE_IPMUX_SC_IP1;
        u8Clk_Mux = CKG_VE_IN_CLK_MPEG0;
    }
    else
    {
        // Neet to be touched by Mstar
        u8Clk_Mux  = CKG_VE_IN_CLK_VD;
        u8Data_Mux = VE_IPMUX_VD;
    }
#endif
    // titania bring up
    u8Data_Mux = VE_IPMUX_SC_IP1;
    u8Clk_Mux = CKG_VE_IN_CLK_MPEG0;
    Drv_TVEncoder_WriteMask(BK_IP_MUX(0x02), u8Data_Mux, 0x00FF); // input select
    //Drv_TVEncoder_WriteMask(BK_CHIPTOP(0x1C), u8Clk_Mux, 0x00FF);
    Drv_TVEncoder_WriteMask(BK_CLKGEN0(0x25), u8Clk_Mux, 0x00FF);
    //MDrv_WriteByte(REG_CKG_VE_IN, u8Clk_Mux); // idclk
}


void _MHal_VE_Set_FRC(u16 u16FrameRate)
{
    U32 u32FullNum, u32EmptyNum;
    U32 u32FrameSize;
    U16 u16Out_FR, u16In_FR;
    VE_HAL_DBG("_MHal_VE_Set_FRC\n");
    if(g_VEInfo.VideoSystem <= MS_VE_NTSC_J)
    {
        u32FrameSize = VE_NTSC_FRAME_SIZE;
        u16Out_FR = 2997;
        //u16In_FR = 2997;    // titania bringup
    }
    else
    {
        u32FrameSize = VE_PAL_FRAME_SIZE;
        u16Out_FR = 2500;
        //u16In_FR = 2500; // titania bringup
    }
    u16In_FR = u16FrameRate / 10;
    // titania bringup : fixme!
    //u16In_FR =  (g_SrcInfo.u16InputVFreq/2*10) ;//: (g_SrcInfo.u16InputVFreq*10);
    VE_HAL_DBG("-----VE: u16In_FR = %d \n", u16In_FR);
    VE_HAL_DBG("-----VE: u16Out_FR = %d\n",u16Out_FR);
    if(u16In_FR > (u16Out_FR - VE_FRC_TOLERANT) && u16In_FR<(u16Out_FR + VE_FRC_TOLERANT) )
    {
#if 0
        // No need to do FRC
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x20), 1, BIT0); // disable Full drop
        Drv_TVEncoder_Write(BK_VE_SRC(0x21),  0x00);
        Drv_TVEncoder_Write(BK_VE_SRC(0x22),  0x00);
        Drv_TVEncoder_Write(BK_VE_SRC(0x30),  0x00);
        Drv_TVEncoder_Write(BK_VE_SRC(0x31),  0x00);
#else
        u32FullNum  = TVEncoder_REG(BK_VE_SRC(0x45)) * 3;   // 3 fileds
        u32EmptyNum = TVEncoder_REG(BK_VE_SRC(0x45));       // 1 field
        Drv_TVEncoder_Write(BK_VE_SRC(0x21), u32FullNum & 0x0000FFFF);
        Drv_TVEncoder_Write(BK_VE_SRC(0x22), (u32FullNum & 0xFFFF0000)>>16);
        Drv_TVEncoder_Write(BK_VE_SRC(0x30), u32EmptyNum & 0x0000FFFF);
        Drv_TVEncoder_Write(BK_VE_SRC(0x31), (u32EmptyNum & 0xFFFF0000)>>16);
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x20), 0, BIT0); // enable Full drop
#endif

        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x35), 0x00, BIT3|BIT2);
        //MDrv_WriteByteMask(L_BK_VE_SRC(0x35), _BIT2, _BIT3|_BIT2);
    }
    else if(u16In_FR < u16Out_FR)
    {
        u32FullNum  = u32FrameSize * 19 / 10 ;
        //MDrv_Write4Byte(L_BK_VE_SRC(0x21),  u32FullNum);
        Drv_TVEncoder_Write(BK_VE_SRC(0x21), u32FullNum & 0x0000FFFF);
        Drv_TVEncoder_Write(BK_VE_SRC(0x22), (u32FullNum & 0xFFFF0000)>>16);

        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x20), 0, BIT0); // enable Full drop

        u32EmptyNum = (U32)(u16Out_FR - u16In_FR) * u32FrameSize / (U32)u16Out_FR ;
        //MDrv_Write4Byte(L_BK_VE_SRC(0x30),  u32EmptyNum);
        Drv_TVEncoder_Write(BK_VE_SRC(0x30), u32EmptyNum & 0x0000FFFF);
        Drv_TVEncoder_Write(BK_VE_SRC(0x31), (u32EmptyNum & 0xFFFF0000)>>16);
       // Drv_TVEncoder_WriteMask(BK_VE_SRC(0x35), BIT2, BIT3|BIT2); // deinterlace mode
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x35), 0x00, BIT3|BIT2); //LGE gbtogether(081113) -> Just Update!!
    }
    else
    {
        u32FullNum = u32FrameSize +
                     ((U32)u16Out_FR  * u32FrameSize / u16In_FR);

        //MDrv_Write4Byte(L_BK_VE_SRC(0x21),  u32FullNum);
        Drv_TVEncoder_Write(BK_VE_SRC(0x21), u32FullNum & 0x0000FFFF);
        Drv_TVEncoder_Write(BK_VE_SRC(0x22), (u32FullNum & 0xFFFF0000)>>16);

        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x20), 0, BIT0); // enable Full drop

        u32EmptyNum = u32FrameSize / 10 ;
        Drv_TVEncoder_Write(BK_VE_SRC(0x30), u32EmptyNum & 0x0000FFFF);
        Drv_TVEncoder_Write(BK_VE_SRC(0x31), (u32EmptyNum & 0xFFFF0000)>>16);
       // Drv_TVEncoder_WriteMask(BK_VE_SRC(0x35), BIT2, BIT3|BIT2); // deinterlace mode
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x35), 0x00, BIT3|BIT2);//LGE gbtogether(081113) -> Just Update!!
    }

    if (g_VEInfo.bInterlace)
    {
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x35), 0x0000, BIT3|BIT2);
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x46),0x00D0, 0x00F0);
    }
    else    // if progressive, enable 3-frame mode to let play more smooth
    {
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x35), 0x0008, BIT3|BIT2);  //0x3b35[3:2] : 2…b10
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x46),0x0000, 0x00F0);      //0x3b46[7:4] : 4…b0
    }

#if 0 //LGE gbtogether(081113) Just update!!! ...It is not used by Swen.
    //fix 704x576/720x576 vertical line issue for Korean demo
    //printf("u16H_width =%d, u16V_width = %d \n", u16H_width, u16V_width);
    if((g_VEInfo.u16H_CapSize == 704 || g_VEInfo.u16H_CapSize == 720) &&
        (g_VEInfo.u16V_CapSize == 576) &&
        (stSystemInfo.enInputPortType ==INPUT_PORT_MS_DTV))
    {
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x35), 0, BIT2);
    }
#endif

}

static void Drv_VE_Set_ColorConvert(MS_VE_VIDEOSYS VideoSys, U8 u8DACType)
{

    if(VideoSys == MS_VE_NTSC || VideoSys == MS_VE_NTSC_J || VideoSys == MS_VE_NTSC_443)
    { // NTSC
        switch(u8DACType)
        {
        case VE_OUT_CVBS_YCC:
            break;

        case VE_OUT_CVBS_YCbCr:
            Drv_TVEncoder_Write(BK_VE_ENC(0x43), 0x5A);
            Drv_TVEncoder_Write(BK_VE_ENC(0x46), 0x80);
            break;

        case VE_OUT_CVBS_RGB:
            Drv_TVEncoder_Write(BK_VE_ENC(0x41), 0x95);
            Drv_TVEncoder_Write(BK_VE_ENC(0x42), 0x4B);
            Drv_TVEncoder_Write(BK_VE_ENC(0x43), 0x90);
            Drv_TVEncoder_Write(BK_VE_ENC(0x44), 0x49);
            Drv_TVEncoder_Write(BK_VE_ENC(0x45), 0x32);
            Drv_TVEncoder_Write(BK_VE_ENC(0x46), 0x102);
            break;
        }
    }
    else
    { // PAL
        switch(u8DACType)
        {
        case VE_OUT_CVBS_YCC:
            break;

        case VE_OUT_CVBS_YCbCr:
            Drv_TVEncoder_Write(BK_VE_ENC(0x43), 0x55);
            Drv_TVEncoder_Write(BK_VE_ENC(0x46), 0x78);
            break;

        case VE_OUT_CVBS_RGB:
            Drv_TVEncoder_Write(BK_VE_ENC(0x41), 0x8E);
            Drv_TVEncoder_Write(BK_VE_ENC(0x42), 0x4B);
            Drv_TVEncoder_Write(BK_VE_ENC(0x43), 0x88);
            Drv_TVEncoder_Write(BK_VE_ENC(0x44), 0x45);
            Drv_TVEncoder_Write(BK_VE_ENC(0x45), 0x2F);
            Drv_TVEncoder_Write(BK_VE_ENC(0x46), 0xF3);
            break;
        }
    }
}

//------------------------------------------------------------------------------
/// Get the starting postion and size of capture window
///
/// @param  ptiming  \b IN: VOP timing information
/// @return none
//------------------------------------------------------------------------------
// Need to Refine
void _MHal_VE_Get_Window(MS_VOP_TIMING Timing)
{
    //MS_VIDEO_CAPTUREWINTABLE_TYPE* pCapWin;
    VE_HAL_DBG("_MHal_VE_Get_Window\n");
    // titania bring up
#if 0
    switch(stSystemInfo.enInputSourceType)
    {
#if (defined(INPUT_HDMI_VIDEO_COUNT) && (INPUT_HDMI_VIDEO_COUNT >  0))	// changed to remove warning(dreamer@lge.com)
    case INPUT_SOURCE_HDMI1:
#if (defined(INPUT_HDMI_VIDEO_COUNT) && (INPUT_HDMI_VIDEO_COUNT >= 2))	// changed to remove warning(dreamer@lge.com)
    case INPUT_SOURCE_HDMI2:
#endif
#if (defined(INPUT_HDMI_VIDEO_COUNT) && (INPUT_HDMI_VIDEO_COUNT >= 3))	// changed to remove warning(dreamer@lge.com)
    case INPUT_SOURCE_HDMI3:
#endif
#if (defined(INPUT_HDMI_VIDEO_COUNT) && (INPUT_HDMI_VIDEO_COUNT >= 4))	// changed to remove warning(dreamer@lge.com)
    case INPUT_SOURCE_HDMI4:
#endif
        g_VEInfo.u16H_CapStart = MDrv_Scaler_GetHorizontalDEStart();
        g_VEInfo.u16V_CapStart = MDrv_Scaler_GetVerticalDEStart();
        g_VEInfo.u16H_CapSize  = MDrv_Scaler_GetHorizontalDE();
        g_VEInfo.u16V_CapSize  = MDrv_Scaler_GetVerticalDE();
        break;
#endif

#if (INPUT_YPBPR_VIDEO_COUNT>=1)
    case INPUT_SOURCE_YPBPR:
#endif
#if (INPUT_YPBPR_VIDEO_COUNT>=2)
    case INPUT_SOURCE_YPBPR2:
#endif
    case INPUT_SOURCE_VGA:

        g_VEInfo.u16H_CapStart = g_PcadcModeSetting.u16DefaultHStart* 2 -g_PcadcModeSetting.u16HorizontalStart;
        g_VEInfo.u16V_CapStart = g_PcadcModeSetting.u16VerticalStart;


        if((IsSrcTypeYPbPr(SYS_INPUT_SOURCE_TYPE)) &&
           (g_PcadcModeSetting.u8ModeIndex == MD_720x480_60I ||
            g_PcadcModeSetting.u8ModeIndex == MD_720x480_60P ||
            g_PcadcModeSetting.u8ModeIndex == MD_720x576_50I ||
            g_PcadcModeSetting.u8ModeIndex == MD_720x576_50P))
        {
            g_VEInfo.u16H_CapSize = 848; // for better quality
        }
        else
        {
            g_VEInfo.u16H_CapSize = MDrv_Mode_GetStdModeResH( g_PcadcModeSetting.u8ModeIndex );// standard display width
        }


        g_VEInfo.u16V_CapSize = MDrv_Mode_GetStdModeResV( g_PcadcModeSetting.u8ModeIndex );// standard display height

        if ( MDrv_Mode_GetStdModeResV( g_PcadcModeSetting.u8ModeIndex ) == 350 ) // if IBM VGA 640x350 then use 640x400 resolution and move to middle of screen
        {
            g_VEInfo.u16V_CapStart -= ( ( 400 - 350 ) / 2 );
            g_VEInfo.u16V_CapSize = 400;
        }

        VE_HAL_DBG("TVEncoder PCMode: DefaultHstart=%d, Hstart =%d, Vstart=%d, Hsize=%d, Vsize=%d, ModeIndex =%bx\n",
            g_PcadcModeSetting.u16DefaultHStart,
            g_PcadcModeSetting.u16HorizontalStart,
            g_PcadcModeSetting.u16VerticalStart,
            MDrv_Mode_GetStdModeResH( g_PcadcModeSetting.u8ModeIndex ),
            MDrv_Mode_GetStdModeResV( g_PcadcModeSetting.u8ModeIndex ),
            g_PcadcModeSetting.u8ModeIndex);

        VE_HAL_DBG("TVEncoder : Hstar =%d, Hsize=%d, Vstart =%d, Vsize = %d , modeidx = %bx\n",
            g_DisplayWindowSetting.u16H_CapStart,
            g_DisplayWindowSetting.u16H_CapSize,
            g_DisplayWindowSetting.u16V_CapStart,
            g_DisplayWindowSetting.u16V_CapSize,
            g_PcadcModeSetting.u8ModeIndex)

        break;

#if (INPUT_SCART_VIDEO_COUNT >= 1)
    case INPUT_SOURCE_SCART:
#endif
#if (INPUT_SCART_VIDEO_COUNT >= 2)
    case INPUT_SOURCE_SCART2:
#endif
#if (INPUT_AV_VIDEO_COUNT >= 1)
    case INPUT_SOURCE_CVBS:
#endif
#if (INPUT_AV_VIDEO_COUNT >= 2)
    case INPUT_SOURCE_CVBS2:
#endif
#if (INPUT_AV_VIDEO_COUNT >= 3)
    case INPUT_SOURCE_CVBS3:
#endif
#if (INPUT_SV_VIDEO_COUNT >= 1)
    case INPUT_SOURCE_SVIDEO:
#endif
#if (INPUT_SV_VIDEO_COUNT >= 2)
    case INPUT_SOURCE_SVIDEO2:
#endif
    case INPUT_SOURCE_TV:

        if(IsUseExtVDPort(stSystemInfo.enInputPortType))
        {
            pCapWin = &ExtVDVideoCapture[g_VdInfo.ucVideoSystem];
        }
        else
        {
            //pCapWin = &VideoCaptureWinTbl[g_VdInfo.ucVideoSystem];
            pCapWin = &VideoCapWinTbl[g_VdInfo.ucVideoSystem];
        }

        g_VEInfo.u16H_CapStart = pCapWin->u16SRHStart;
        g_VEInfo.u16V_CapStart = pCapWin->u16SRVStart;
        g_VEInfo.u16H_CapSize = pCapWin->u16HRange;
        g_VEInfo.u16V_CapSize = pCapWin->u16VRange;

        VE_HAL_DBG("VEGetHstart:%d,VEGetVstart:%d,VEGetHsize:%d,VEGetVsize:%d\n",
            pCapWin->u16SRHStart,pCapWin->u16SRVStart,pCapWin->u16HRange,pCapWin->u16VRange);
        break;
    case INPUT_SOURCE_DTV:
        g_VEInfo.u16H_CapStart = (ptiming->u16HActive_Start)/2 + 1;
        g_VEInfo.u16V_CapStart = ptiming->u16VBlank0_End - (ptiming->u16VBlank0_Start + (U16)ptiming->u8VSync_Offset) - 0x9;
        g_VEInfo.u16H_CapSize = ptiming->u16Width;
        g_VEInfo.u16V_CapSize = ptiming->u16Height;

        VE_HAL_DBG("VE_DTV H_CapStart:%d,V_CapStart:%d,H_CapSize:%d,V_CapSize:%d\n",
            g_VEInfo.u16H_CapStart, g_VEInfo.u16V_CapStart, g_VEInfo.u16H_CapSize, g_VEInfo.u16V_CapSize);
        break;
    }
#endif
        g_VEInfo.u16H_CapStart = (Timing.u16HActive_Start)/2 + 1;
        g_VEInfo.u16V_CapStart = Timing.u16VBlank0_End - (Timing.u16VBlank0_Start + (U16)Timing.u8VSync_Offset) - 0x9;
        g_VEInfo.u16H_CapSize = Timing.u16Width;
        g_VEInfo.u16V_CapSize = Timing.u16Height;
		g_VEInfo.bInterlace= Timing.bInterlace;

        VE_HAL_DBG("VE_DTV H_CapStart:%d,V_CapStart:%d,H_CapSize:%d,V_CapSize:%d\n",
            g_VEInfo.u16H_CapStart, g_VEInfo.u16V_CapStart, g_VEInfo.u16H_CapSize, g_VEInfo.u16V_CapSize);

}


//------------------------------------------------------------------------------
/// Set saling ratio of VE
///
/// @param  none
/// @return none
//------------------------------------------------------------------------------
void _MHal_VE_Set_ScalingRatio(void)
{
    U16 u16H_Ratio, u16V_Ratio;
    U16 u16Out_Vsize, u16Out_Hsize;
    U16 u16In_Vsize, u16In_Hsize;
    U32 u32VE_MIU_WriteBase;
    U32 u32VE_FrameBuffer_WriteAdr; /* To avoid compilation warning msg. byLGE. jjab. */
    U32 i;

    VE_HAL_DBG("_MHal_VE_Set_ScalingRatio\n");
    //LGE gbtogether(081028) by Swen..  VE Scart Out grabage
    //MHal_VE_Set_ADC_BuffOut(FALSE, INPUT_PORT_MS_DTV);  // move to DDI layer  swen
#if 0 // (ENABLE_VOP_DUPLICATE) titania bringup
    u16Out_Hsize = (stSystemInfo.enInputSourceType == INPUT_SOURCE_DTV && stVOPTiming.bHDuplicate) ? 704 : 720;
#else
    u16Out_Hsize = 720;
#endif
    u16Out_Vsize = (g_VEInfo.VideoSystem <= MS_VE_NTSC_J) ? 480 : 576;

    u16In_Hsize = g_VEInfo.u16H_CapSize;
    u16In_Vsize = g_VEInfo.u16V_CapSize;

    u32VE_MIU_WriteBase = u32VE_MIU_BASE;

    VE_HAL_DBG("VE: u16In_Hsize =%d, u16In_Vsize = %d \n", u16In_Hsize, u16In_Vsize);
    VE_HAL_DBG("VE: u16Out_Hsize =%d, u16IOut_Vsize = %d \n", u16Out_Hsize, u16Out_Vsize);
    if(u16Out_Hsize != u16In_Hsize)
    {
        if(u16In_Hsize > u16Out_Hsize)
        {
            u16H_Ratio = (U16)VE_H_SCALE_DOWN_RATIO(u16In_Hsize, u16Out_Hsize);
            //MDrv_WriteRegBit(L_BK_VE_SRC(0x00), 0, _BIT8);
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 0, BIT0 << 8);

            Drv_TVEncoder_Write(BK_VE_SRC(0x16), u16H_Ratio);
        }
        else    //VE not support upscale,just bypass it
        {
            if(u16In_Hsize ==544)//DVB DTV 544x576 needs fine tune by hand
            {
                Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 0, BIT0 << 8);
                Drv_TVEncoder_Write(BK_VE_SRC(0x16), 0x07FF);
            }
            else
            {
                Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 1, BIT0 << 8);
                Drv_TVEncoder_Write(BK_VE_SRC(0x16), 0x0000);
            }

            // set write addr, 1 pixel need 16 bits data, so * 2
            u32VE_MIU_WriteBase += (u16Out_Hsize - u16In_Hsize)/2 * 2;
        }
    }
    else
    {
        //MDrv_WriteRegBit(L_BK_VE_SRC(0x00), 1, _BIT8);
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 1, BIT0 << 8);
        Drv_TVEncoder_Write(BK_VE_SRC(0x16), 0x0000);
    }

    if(u16Out_Vsize != u16In_Vsize)
    {
        if(u16In_Vsize > u16Out_Vsize)
        {
            u16V_Ratio = (U16)VE_V_SCALE_DOWN_RATIO(u16In_Vsize, u16Out_Vsize);
            //MDrv_WriteRegBit(L_BK_VE_SRC(0x00), 0, _BIT9);
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 0, BIT1 << 8);
            Drv_TVEncoder_Write(BK_VE_SRC(0x17), u16V_Ratio-3);
        }
        else    //VE not support upscale,just bypass it
        {
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 1, BIT1 << 8);
            Drv_TVEncoder_Write(BK_VE_SRC(0x17), 0x0000);
            // set write addr,  each line has u16Out_Hsize pixels
            //LGE gbtogether(081024) by Fitch
            u32VE_MIU_WriteBase += (u16Out_Vsize - u16In_Vsize)/2/2 *u16Out_Hsize * 2;
        }
    }
    else
    {
        if(u16In_Hsize ==544)//DVB DTV 544x576 needs fine tune by hand
        {
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 0, BIT1 <<8);
            Drv_TVEncoder_Write(BK_VE_SRC(0x17), 0x0750);
        }
        else
        {
            //MDrv_WriteRegBit(L_BK_VE_SRC(0x00), 1, _BIT9);
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 1, BIT1 <<8);
            Drv_TVEncoder_Write(BK_VE_SRC(0x17), 0x0000);
        }
    }
#if 1
    u32VE_FrameBuffer_WriteAdr = u32VE_MIU_WriteBase / 16;
    Drv_TVEncoder_WriteMask(BK_VE_SRC(0x02), (LOPART(u32VE_FrameBuffer_WriteAdr) & 0x00FF) << 8, 0xFF00);
    Drv_TVEncoder_Write(BK_VE_SRC(0x7c), ((HIPART(u32VE_FrameBuffer_WriteAdr) & 0x00FF) << 8) + ((LOPART(u32VE_FrameBuffer_WriteAdr) & 0xFF00) >> 8));
    //LGE gbtogether(081028) by Swen .. VE Scart Out grabage
    if (u32VE_MIU_WriteBase != u32VE_MIU_BASE)
    {
        _bNeedClear = TRUE;
    }
#endif
   // MHal_VE_Set_ADC_BuffOut(TRUE, INPUT_PORT_MS_DTV);  // move to DDI layer  swen
}


//------------------------------------------------------------------------------
/// set capture window of VE
///
/// @param  u16Hstart \b IN: horianotal starting postion
/// @param  u16Hend   \b IN: horianotal ending postion
/// @param  u16Vstart \b IN: vertical starting postion
/// @param  u16Vend   \b IN: vertical ending postion
/// @return none
//------------------------------------------------------------------------------
void _MHal_VE_Set_Window(U16 u16Hstart, U16 u16Hend, U16 u16Vstart, U16 u16Vend)
{
    VE_HAL_DBG("_MHal_VE_Set_Window\n");
    Drv_TVEncoder_Write(BK_VE_SRC(0x12), u16Hstart);
    Drv_TVEncoder_Write(BK_VE_SRC(0x13), u16Hend);
    Drv_TVEncoder_Write(BK_VE_SRC(0x14), u16Vstart);
    Drv_TVEncoder_Write(BK_VE_SRC(0x15), u16Vend);
  //  printf("VE_Set_Window Hstart: %x, Hend : %x, Vstart: %x, Vend: %x\n",u16Hstart,u16Hend,u16Vstart,u16Vend);
}

#if 0
void MHal_VE_Set_ADC_BuffOut(BOOL bEnable, EN_INPUT_PORT_TYPE enInputPortType)
{
    U8 u8Ymux = 0;
    U8 u8Cmux = 0;

    U16 u16Adc48 = 0;
    U16 u16Adc50 = 0;
    U16 u16Adc51 = 0;

    VE_HAL_DBG("MHal_VE_Set_ADC_BuffOut [%d] [%d]\n", bEnable, enInputPortType);
	MHal_VE_SetCurInput(enInputPortType);

    if(bEnable)
    {
        switch ( enInputPortType )
        {
        case INPUT_PORT_MS_DTV:
            Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x48), 0, _BIT6);       // VE as CVBS DAC input source
            Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x50), _BIT8, _BIT0 & _BIT1 & _BIT7 & _BIT8);
            u8Ymux = 0x0F;
            u8Cmux = 0x00;
            u16Adc50 = _BIT8;
            u16Adc51 = _BIT11;
            u16Adc48 = _BIT3;
            break;

        case INPUT_PORT_MS_CVBS0: // TV
            Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x48), _BIT6, _BIT6);   // VIF/VD as CVBS DAC input source
            Drv_TVEncoder_WriteMask(BK_CHIPTOP(0x12), 0, _BIT8);        // select VIF
            u8Ymux = 0x0F;
            u8Cmux = 0x00;
            u16Adc50 = _BIT8;
            u16Adc51 = _BIT11;
            u16Adc48 = _BIT3;
            break;


        case INPUT_PORT_MS_CVBS1: // CVBS
            u8Ymux = g_SCInputMuxInfo.u8Cvbs1YMux;
            u8Cmux = g_SCInputMuxInfo.u8Cvbs1CMux;
            u16Adc50 = _BIT8;
            u16Adc51 = _BIT11;
            u16Adc48 = 0;
            break;

        case INPUT_PORT_AV_SCART0:
            u8Ymux = g_SCInputMuxInfo.u8Scart1YMux;
            u8Cmux = g_SCInputMuxInfo.u8Scart1CMux;
            u16Adc50 = _BIT8;
            u16Adc51 = _BIT11;
            u16Adc48 = 0;
            break;

		 case INPUT_PORT_AV_SCART1:
            u8Ymux = g_SCInputMuxInfo.u8Scart2YMux;
            u8Cmux = g_SCInputMuxInfo.u8Scart2CMux;
            u16Adc50 = _BIT8;
            u16Adc51 = _BIT11;
            u16Adc48 = 0;
			break;

        case INPUT_PORT_MS_SV0:
            u8Ymux = g_SCInputMuxInfo.u8SVideo1YMux;
            u8Cmux = g_SCInputMuxInfo.u8SVideo1CMux;
            u16Adc50 = _BIT8;
            u16Adc51 = _BIT11;
            u16Adc48 = _BIT3;
            break;

        default: // TV
            Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x48), _BIT6, _BIT6);   // VIF/VD as CVBS DAC input source
            Drv_TVEncoder_WriteMask(BK_CHIPTOP(0x12), 0, _BIT8);        // select VIF
            u8Ymux = 0x0F;
            u8Cmux = 0x00;
            u16Adc50 = _BIT8;
            u16Adc51 = _BIT11;
            u16Adc48 = _BIT3;
            break;
        }
    }
    else
    {
        u16Adc50 = _BIT0;
    }

    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x50), u16Adc50, _BIT0 & _BIT1 & _BIT7 & _BIT8);
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x51), u16Adc51, _BIT10 & _BIT11);          // bit10: enable y channel   bit 11: enable c channel
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x51), u8Ymux,      0x0F);                  // ymux
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x51), u8Cmux << 4, 0xF0);                  // cmux
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x48), u16Adc48, _BIT3 & _BIT7 & _BIT8);    // bit3: enable IDAC   bit7/8: set output voltage range
}

void MHal_VE_Set_ADC_RFOut(BOOL bEnable)
{

	EN_INPUT_PORT_TYPE enInputPortType ;

	enInputPortType = MHal_VE_GetCurInput();

    VE_HAL_DBG("MHal_VE_Set_ADC_RFOut[%d]\n",bEnable);

    if(bEnable)
    {
        switch ( enInputPortType )
        {
	        case INPUT_PORT_MS_CVBS0: // TV

				break;

			default: //Other Input & MNT out output off....

				break;
        }
    }
    else
    {

    }

}
#endif

#if 0
//------------------------------------------------------------------------------
/// set ADC CVBS buffer out
///
/// @param  bEnable \b IN : Enable/Disable ADC CVBS buffer
/// @param  enInputPortType \b IN : input port type of source
/// @return
//------------------------------------------------------------------------------
void MHal_VE_Set_ADC_BuffOut(BOOL bEnable, EN_INPUT_PORT_TYPE enInputPortType)
{
#if 1 // set tuner CVBS always output for SCART
    U8 u8Ymux, u8Cmux;
#endif
    U8 u8Adc38L;
    U8 u8Adc39L;
    U8 u8Adc39H;
    U8 u8Adc3AL;
    U8 u8Adc3BH;
	//LGE gbtogether(081018) -> RF-OUT for Minerva
   // U8 u8Adc3CL;
   // U8 u8Adc3DH;


    VE_HAL_DBG("MHal_VE_Set_ADC_BuffOut ...\n");
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x38), 0x70, 0x00FF); // init vide input
	MHal_VE_SetCurInput(enInputPortType);

    u8Adc3AL = 0x00;
    u8Adc3BH = 0x00;

	//u8Adc3CL = 0x14;
	//u8Adc3DH = 0x00;
    if(bEnable)
    {

        VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
        switch ( enInputPortType )
        {
        case INPUT_PORT_MS_DTV:
        VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);

            u8Adc38L = 0x5C; // 0x10;
            u8Adc39L = 0x0B;
            u8Adc39H = 0x0B;
			//u8Adc3CL = 0x0F;
			//u8Adc3DH = 0x00;
            break;

        case INPUT_PORT_MS_CVBS0: // TV
        VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
#if 0 // 080919 LGE hjpark42 drmyung : RF ATV Output Level adjust to 500mV
			u8Adc38L = 0x5C; // 0x10;
#else
            u8Adc38L = 0x14;
#endif
            u8Adc39L = 0x00;
            u8Adc39H = 0x00;

            break;

#if 1 // set tuner CVBS always output for SCART
        case INPUT_PORT_MS_CVBS1: // CVBS
#if 1 // LGE drmyung 080903
			u8Ymux = g_SCInputMuxInfo.u8Cvbs1YMux;
			u8Cmux = g_SCInputMuxInfo.u8Cvbs1CMux;
			VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
			VE_HAL_DBG("INPUT_PORT_MS_CVBS1, SC_CVBS1_YMUX=%d, SC_CVBS1_CMUX=%d\n",u8Ymux,u8Cmux);
#else
            VE_HAL_DBG("INPUT_PORT_MS_CVBS1, SC_CVBS1_YMUX=%d, SC_CVBS1_CMUX=%d\n",SC_CVBS1_YMUX,SC_CVBS1_CMUX);
            u8Ymux = SC_CVBS1_YMUX;
            u8Cmux = SC_CVBS1_CMUX;
#endif
            //u8Ymux = (TVEncoder_REG(BK_ADC_ATOP(0x02)) & 0x0F);
            //u8Cmux = ((TVEncoder_REG(BK_ADC_ATOP(0x02))>>4) & 0x0F);


            u8Adc38L = 0x14; // 0x3C;
            //u8Adc39L = 0x33;
            //u8Adc39H = 0xFF;
            u8Adc39L = (u8Ymux<<4)|u8Ymux;
            u8Adc39H = (u8Cmux<<4)|u8Cmux;
            u8Adc3AL = 0x03;
            u8Adc3BH = 0x0A;
            break;

        case INPUT_PORT_AV_SCART0:
#if 1 // LGE drmyung 080903
			u8Ymux = g_SCInputMuxInfo.u8Scart1YMux;
			u8Cmux = g_SCInputMuxInfo.u8Scart1CMux;
			VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
			VE_HAL_DBG("INPUT_PORT_AV_SCART0, SC_SCART1_YMUX=%d, SC_SCART1_CMUX=%d\n",u8Ymux,u8Cmux);
#else
            VE_HAL_DBG("INPUT_PORT_AV_SCART0, SC_SCART1_YMUX=%d SC_SCART1_CMUX=%d\n", SC_SCART1_YMUX, SC_SCART1_CMUX);
            //u8Ymux = INPUT_TV_YMUX;       // MDrv_ReadByte(L_BK_ADC_ATOP(0x02)) & 0x0F;
            u8Ymux = SC_SCART1_YMUX;                  // MDrv_ReadByte(L_BK_ADC_ATOP(0x02)) & 0x0F;
            //u8Cmux = MSVD_CMUX_NONE;        // (MDrv_ReadByte(L_BK_ADC_ATOP(0x02))>>4) & 0x0F;
            u8Cmux = SC_SCART1_CMUX;        // (MDrv_ReadByte(L_BK_ADC_ATOP(0x02))>>4) & 0x0F;
#endif
            u8Adc38L = 0x14; // 0x3C;
            u8Adc39L = (u8Ymux<<4)|u8Ymux;
            u8Adc39H = (u8Cmux<<4)|u8Cmux;
            u8Adc3AL = 0x03;
            u8Adc3BH = 0x0A;
            break;


		 case INPUT_PORT_AV_SCART1:
#if 1 // LGE drmyung 080903
			u8Ymux = g_SCInputMuxInfo.u8Scart2YMux;
			u8Cmux = g_SCInputMuxInfo.u8Scart2CMux;
			VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
			VE_HAL_DBG("INPUT_PORT_AV_SCART0, SC_SCART1_YMUX=%d, SC_SCART1_CMUX=%d\n",u8Ymux,u8Cmux);
#else
			VE_HAL_DBG("INPUT_PORT_AV_SCART0, SC_SCART1_YMUX=%d SC_SCART1_CMUX=%d\n", SC_SCART1_YMUX, SC_SCART1_CMUX);
			//u8Ymux = INPUT_TV_YMUX;		// MDrv_ReadByte(L_BK_ADC_ATOP(0x02)) & 0x0F;
			u8Ymux = SC_SCART1_YMUX;				  // MDrv_ReadByte(L_BK_ADC_ATOP(0x02)) & 0x0F;
			//u8Cmux = MSVD_CMUX_NONE;		  // (MDrv_ReadByte(L_BK_ADC_ATOP(0x02))>>4) & 0x0F;
			u8Cmux = SC_SCART1_CMUX;		// (MDrv_ReadByte(L_BK_ADC_ATOP(0x02))>>4) & 0x0F;
#endif
			u8Adc38L = 0x14; // 0x3C;
			u8Adc39L = (u8Ymux<<4)|u8Ymux;
			u8Adc39H = (u8Cmux<<4)|u8Cmux;
			u8Adc3AL = 0x03;
			u8Adc3BH = 0x0A;
			break;

        case INPUT_PORT_MS_SV0:
        VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
            //u8Ymux = MDrv_ReadByte(L_BK_ADC_ATOP(0x02)) & 0x0F;
            //u8Cmux = (MDrv_ReadByte(L_BK_ADC_ATOP(0x02))>>4) & 0x0F;
            u8Ymux = (TVEncoder_REG(BK_ADC_ATOP(0x02)) & 0x0F);
            u8Cmux = ((TVEncoder_REG(BK_ADC_ATOP(0x02))>>4) & 0x0F);

            u8Adc38L = 0x5C;
            u8Adc39L = (u8Ymux<<4)|u8Ymux;
            u8Adc39H = (u8Cmux<<4)|u8Cmux;
            break;

#endif
        default:
        VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
            u8Adc38L = 0x14;
            u8Adc39L = 0x00;
            u8Adc39H = 0x00;
            break;
        }
    }
    else
    {
        VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
        u8Adc38L = 0x0F;
        u8Adc39L = 0x00;
        u8Adc39H = 0x00;
    }



    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x38), u8Adc38L, 0x00FF);
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x39), u8Adc39L, 0x00FF);
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x39), u8Adc39H <<8, 0xFF00);
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3A), u8Adc3AL, 0x00FF);
    Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3B), u8Adc3BH <<8, 0xFF00);
	//LGE gbtogether(081018) -> RF-OUT for Minerva
	//Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3C), u8Adc3CL, 0x00FF);
	//Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3D), u8Adc3DH <<8, 0xFF00);


}


void MHal_VE_Set_ADC_RFOut(BOOL bEnable)
{
    U8 u8Adc3CL;
    U8 u8Adc3DH;
	U8 u8Adc3DL;
	U8 u8Adc3EL;
	U8 u8Adc3FL;
	U8 u8Adc3FH;
	EN_INPUT_PORT_TYPE enInputPortType ;

	enInputPortType = MHal_VE_GetCurInput();

    VE_HAL_DBG("MHal_VE_Set_ADC_RFOut[%d]\n",bEnable);
    //Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x38), 0x70, 0x00FF); // init video input

	//LGE gbtogether(090129)
    if(bEnable)
    {
    	VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
        switch ( enInputPortType )
        {
	        case INPUT_PORT_MS_CVBS0: // TV
	        	VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
		    	u8Adc3CL = 0x14;
				u8Adc3DL = 0x00;
				u8Adc3DH = 0x00;
				break;

			default: //Other Input & MNT out output off....
				VE_HAL_DBG("%s [%d]\n", __FUNCTION__, __LINE__);
				u8Adc3CL = 0x10; //LGE gbtogether(090115) 0x14 ->0x10 why ???
				u8Adc3DL = 0x00;
				u8Adc3DH = 0x00;
				break;
        }
    }
    else
    {
		u8Adc3CL = 0x0F;
		u8Adc3DL = 0x00;
		u8Adc3DH = 0x00;
    }
	u8Adc3EL = 0x03;
	u8Adc3FL = 0x80;
	u8Adc3FH = 0x0b;

	Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3C), u8Adc3CL, 0x00FF);
	Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3D), u8Adc3DL, 0x00FF);
	Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3D), u8Adc3DH <<8, 0xFF00);
	Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3E), u8Adc3EL, 0x00FF);
	Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3F), u8Adc3FL, 0x00FF);
	Drv_TVEncoder_WriteMask(BK_ADC_ATOP(0x3F), u8Adc3FH <<8, 0xFF00);

}
#endif

void _MHal_VE_Set_CaptureWin(void)
{
    U16 u16Out_Vsize, u16Out_Hsize;
    U16 u16Tmp;
    VE_HAL_DBG("_MHal_VE_Set_CaptureWin\n");

    u16Out_Hsize = 720;
    u16Out_Vsize = (g_VEInfo.VideoSystem <= MS_VE_NTSC_J) ? 480 : 576;

    g_VEInfo.u8VE_DisplayStatus &= ~(EN_VE_INVERSE_HSYNC);


    VE_HAL_DBG("Out: Hsize=%d, Vsize=%d \n", u16Out_Hsize, u16Out_Vsize);
    VE_HAL_DBG("VE Cap: Hstart=%d, Hsize =%d \n", g_VEInfo.u16H_CapStart, g_VEInfo.u16H_CapSize);
    //VE_HAL_DBG("SC Cap: Hstart=%d, Hsize =%d \n", g_SrcInfo.u16H_CapStart, g_SrcInfo.u16H_CapSize);
#if 0 // titania bringup fixme!
    if(IsDTVInUse())
    {
        // Horizontal
        if(g_VEInfo.u16H_CapSize < u16Out_Hsize)
        {
            g_VEInfo.u8VE_DisplayStatus &= ~ EN_VE_DEMODE;
            g_VEInfo.u8VE_DisplayStatus |= EN_VE_INVERSE_HSYNC;

            u16Tmp = u16Out_Hsize - g_VEInfo.u16H_CapSize;
            g_VEInfo.u16H_CapStart =  g_VEInfo.u16H_CapStart - u16Tmp/2 + 9;
            g_VEInfo.u16H_CapSize = u16Out_Hsize;
        }
        else
        {
            if(g_VEInfo.u16H_CapSize == u16Out_Hsize &&
               g_VEInfo.u16H_CapSize > g_SrcInfo.u16H_CapSize)
            {
                g_VEInfo.u8VE_DisplayStatus &= ~ EN_VE_DEMODE;
                g_VEInfo.u8VE_DisplayStatus |= EN_VE_INVERSE_HSYNC;

                u16Tmp = g_VEInfo.u16H_CapSize - g_SrcInfo.u16H_CapSize;
                g_VEInfo.u16H_CapStart = g_SrcInfo.u16H_CapStart - u16Tmp/2 + 9;
            }
        }

        // Vertical

            if(g_VEInfo.u16V_CapSize < u16Out_Vsize)
            {
                g_VEInfo.u8VE_DisplayStatus &= ~ EN_VE_DEMODE;
                g_VEInfo.u8VE_DisplayStatus |= EN_VE_INVERSE_HSYNC;

                u16Tmp = u16Out_Vsize - g_VEInfo.u16V_CapSize;
                g_VEInfo.u16V_CapStart =  g_VEInfo.u16V_CapStart - u16Tmp/2;
                g_VEInfo.u16V_CapSize = u16Out_Vsize;
            }
            else
            {
                if(g_VEInfo.u16V_CapSize == u16Out_Vsize &&
                   g_VEInfo.u16V_CapSize > g_SrcInfo.u16V_CapSize)
                {
                    g_VEInfo.u8VE_DisplayStatus &= ~ EN_VE_DEMODE;
                    g_VEInfo.u8VE_DisplayStatus |= EN_VE_INVERSE_HSYNC;

                    u16Tmp = g_VEInfo.u16V_CapSize - g_SrcInfo.u16V_CapSize;
                    g_VEInfo.u16V_CapStart = g_SrcInfo.u16V_CapStart - u16Tmp/2;
                }
            }
            if(g_VEInfo.u16V_CapStart> (g_VEInfo.u16V_CapStart + g_VEInfo.u16V_CapSize - 1))
            {
               MHal_VE_Set_ADC_BuffOut(DISABLE, stSystemInfo.enInputPortType);
            }
    }
#endif
    VE_HAL_DBG("VE status= 0x%x \n", g_VEInfo.u8VE_DisplayStatus);
    VE_HAL_DBG("VE Cap: Vstart=%d, Vsize =%d \n", g_VEInfo.u16V_CapStart, g_VEInfo.u16V_CapSize);

    if(g_VEInfo.u8VE_DisplayStatus & EN_VE_CCIR656_IN)
        _MHal_VE_Set_Window(0x000, 0x7FF, 0x10, 0xFF);
    else if(g_VEInfo.u8VE_DisplayStatus & EN_VE_DEMODE)
        _MHal_VE_Set_Window(0x000, 0x7FF, 0x000, 0x7FF);
    else
    {
       /* _MHal_VE_Set_Window(g_VEInfo.u16H_CapStart,
                            g_VEInfo.u16H_CapStart + g_VEInfo.u16H_CapSize - 1,
                            g_VEInfo.u16V_CapStart,
                            g_VEInfo.u16V_CapStart + g_VEInfo.u16V_CapSize - 1);*/


        u16Tmp = g_VEInfo.u16V_CapSize;

        if (g_VEInfo.bInterlace)
            g_VEInfo.u16V_CapSize = g_VEInfo.u16V_CapSize/2;

        if (g_VEInfo.u16H_CapSize == 720)
		    _MHal_VE_Set_Window(g_VEInfo.u16H_CapStart + 0x11,
                                g_VEInfo.u16H_CapStart + g_VEInfo.u16H_CapSize - 1 + 0x11,
                                g_VEInfo.u16V_CapStart + 4,
                                g_VEInfo.u16V_CapStart + g_VEInfo.u16V_CapSize - 1 + 4);

        else if (g_VEInfo.u16H_CapSize == 1280)
		    _MHal_VE_Set_Window(g_VEInfo.u16H_CapStart + 0x1F,
                                g_VEInfo.u16H_CapStart + g_VEInfo.u16H_CapSize - 1 + 0x1F,
                                g_VEInfo.u16V_CapStart + 4,
                                g_VEInfo.u16V_CapStart + g_VEInfo.u16V_CapSize - 1 + 4);

        else if (g_VEInfo.u16H_CapSize == 1920)
		    _MHal_VE_Set_Window(g_VEInfo.u16H_CapStart + 0x17,
                                g_VEInfo.u16H_CapStart + g_VEInfo.u16H_CapSize - 1 + 0x17,
                                g_VEInfo.u16V_CapStart + 0,
                                g_VEInfo.u16V_CapStart + g_VEInfo.u16V_CapSize - 1 + 0);

        else if (g_VEInfo.u16H_CapSize < 720)  // avoid garbage when input size is small
             _MHal_VE_Set_Window(g_VEInfo.u16H_CapStart + 0x11,
                                g_VEInfo.u16H_CapStart + g_VEInfo.u16H_CapSize - 1 + 0x11 -2,
                                g_VEInfo.u16V_CapStart + 4,
                                g_VEInfo.u16V_CapStart + g_VEInfo.u16V_CapSize - 1 + 4 - 2);

        else
		    _MHal_VE_Set_Window(g_VEInfo.u16H_CapStart + 0x11,
                                g_VEInfo.u16H_CapStart + g_VEInfo.u16H_CapSize - 1 + 0x11,
                                g_VEInfo.u16V_CapStart + 4,
                                g_VEInfo.u16V_CapStart + g_VEInfo.u16V_CapSize - 1 + 4);

        g_VEInfo.u16V_CapSize = u16Tmp;
    }
    Drv_TVEncoder_WriteBit(BK_VE_SRC(0x41), 1, BIT0); //enable inverse Hsync
#if 0   // titania bringup
    if(g_VEInfo.u8VE_DisplayStatus & EN_VE_INVERSE_HSYNC)
        MDrv_WriteRegBit(BK_VE_SRC(0x41), 1, _BIT0); //enable inverse Hsync
    else
        MDrv_WriteRegBit(BK_VE_SRC(0x41), 0, _BIT0); //disable inverse Hsync
#endif

}


//------------------------------------------------------------------------------
//                     VE Function
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///Initialize video encoder
//------------------------------------------------------------------------------
static unsigned int u32VSYNCIntRegister;
static unsigned int u32FIELDIntRegister;
BOOL MHal_VE_Init(void)
{
    //U32 u32VE_MIU_BASE, u32VEBaseLength;
    U32 u32VE_FRAMEBUFFER_ADR;
    //U32 u32RegValue;
    //U32 u32Addr;
	int result;

    if (MDrv_SYS_GetMMAP(E_SYS_MMAP_VE, &u32VE_MIU_BASE, &u32VEBaseLength)){
        //20090629 T3 setting need to be divided by 2
        //u32VE_FRAMEBUFFER_ADR = u32VE_MIU_BASE / 8;
        u32VE_FRAMEBUFFER_ADR = u32VE_MIU_BASE / 16;
        Drv_TVEncoder_Write(BK_VE_SRC(0x01), LOPART(u32VE_FRAMEBUFFER_ADR)); //
        Drv_TVEncoder_Write(BK_VE_SRC(0x02), HIPART(u32VE_FRAMEBUFFER_ADR)); // VE base address
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x02), (LOPART(u32VE_FRAMEBUFFER_ADR) & 0x00FF) << 8, 0xFF00);
        Drv_TVEncoder_Write(BK_VE_SRC(0x7c), ((HIPART(u32VE_FRAMEBUFFER_ADR) & 0x00FF) << 8) + ((LOPART(u32VE_FRAMEBUFFER_ADR) & 0xFF00) >> 8));
    } else
    {
        VE_HAL_DBG("Error get mmap!\n");
    }

    Drv_TVEncoder_WriteMask(BK_CLKGEN0(0x24), 0x0800, 0x0F0F); // clock of ve & vedac
    //Drv_TVEncoder_WriteMask(BK_CHIPTOP(0x19), 0x8000, 0xFF00);
    //Drv_TVEncoder_WriteBit(BK_CHIPTOP(0x1E), 1, BIT4); // disable serial port
    //MDrv_WriteByte(H_BK_CHIPTOP(0x19), 0x80);
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x03),0x0000,0x0010); // meaning?

    // read FIFO control
	//20090629 T3 setting need to be divided by 2
    Drv_TVEncoder_WriteMask(BK_VE_SRC(0x32),0X0018, 0x00FF);  // low part
    Drv_TVEncoder_Write(BK_VE_SRC(0x37),0X0605);  // low part
    // todo : add ttx vbi
    // initial TTX VBI
    //Drv_TVEncoder_WriteBit(BK_VE_SRC(0x43),1, BIT0);
//#if 0
    Drv_TVEncoder_Write(BK_VE_SRC(0x06),0);
    Drv_TVEncoder_Write(BK_VE_SRC(0x43),(TVEncoder_REG(BK_VE_SRC(0x43))&0xff00)|0x0039);
    Drv_TVEncoder_Write(BK_VE_SRC(0x08),(TVEncoder_REG(BK_VE_SRC(0x08))&0x00ff)|0xfb00);
    Drv_TVEncoder_Write(BK_VE_SRC(0x46),(TVEncoder_REG(BK_VE_SRC(0x46))&0xff00)|0x0003);
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x07), 0x00, 0x00ff);
    Drv_TVEncoder_Write(BK_VE_SRC(0x00),TVEncoder_REG(BK_VE_SRC(0x00))|0x0004);
    Drv_TVEncoder_Write(BK_VE_SRC(0x00),TVEncoder_REG(BK_VE_SRC(0x00))|0x0008);
//#endif
    memset(&g_VEInfo, 0x00, sizeof(g_VEInfo));
    g_VEInfo.bUse_ADC_BuffOut = TRUE;
    g_VEInfo.bInterlace= FALSE;

    //test for VE output stable temp solution,2007/07/05
    Drv_TVEncoder_WriteMask(BK_VE_SRC(0x44),0x00f0, 0x00FF);  //low part
    Drv_TVEncoder_WriteMask(BK_VE_SRC(0x46),0x00d0, 0x00f0);  //low part
#if 1
    VE_HAL_DBG("before request irq\n");

	if(0 == u32VSYNCIntRegister) {
	result = request_irq(E_FIQ_VSYNC_VE4VBI, _MHal_VE_ISR0, SA_INTERRUPT, "VE", NULL);	// Mstar andy 080929 : WSS Insertion Reset 문제로 수정.	// LGE lemonic 080929 _MHal_VE_ISR0에서 refresh되도록 변경.
		if (result) {
			return FALSE;
		}

		u32VSYNCIntRegister= 1;
	}
	else {
		disable_irq(E_FIQ_VSYNC_VE4VBI);
		enable_irq(E_FIQ_VSYNC_VE4VBI);
	}

	if(0 == u32FIELDIntRegister) {
    result = request_irq(E_FIQ_FIELD_VE4VBI, _MHal_VE_ISR1, SA_INTERRUPT, "VE", NULL);
		if (result) {
			return FALSE;
		}

		u32FIELDIntRegister= 1;
	} else {
		disable_irq(E_FIQ_FIELD_VE4VBI);
		enable_irq(E_FIQ_FIELD_VE4VBI);
	}

    VE_HAL_DBG("after request irq\n");
/*    if (result)
    {
        VE_HAL_DBG("VE IRQ registartion ERROR\n");
	}
	else
    {
        VE_HAL_DBG("VE IRQ registartion OK\n");
    }*/
#endif
#if 0//(CHIP_FAMILY_TYPE >= CHIP_FAMILY_S3P)	// MStar andy 081013
    //Drv_TVEncoder_WriteBit(H_BK_VE_SRC(0x7E), 1, BIT0); //enable black boundary
#endif

    //MsWriteVenusReg(BK_VE_SRC(0x03), 0x0000); // TTX base address
    //MsWriteVenusReg(BK_VE_SRC(0x04), 0x0000); //
    //MsWriteVenusReg(BK_VE_SRC(0x05), 0x0000); // VBI base address
    // cwchen  Drv_TVEncoder_WriteMask(BK_CHIP_TOP(0x0B), 0x080, 0xFFF); //clk_vedac=108 , clk_ve=27
    //REG(0xbf806408) = 0x0001 ;//select  VE -> DAC out
#if 0
    MDrv_DAC_Set_Output(DAC_TO_VE);
    //REG(0xbf803c68) = 0x0004 ;//0x1A*4, select dac_clk = clk_vedac
    MDrv_DAC_Set_CLKMux(DAC_CLK_FROM_VE);
    //REG(0xbf803c64) = 0x8000 ;//0x19*4, enable clk_ve & clk_vedac
    TOP_REG(0x19) = 0x8000;//0x19*4, enable clk_ve & clk_vedac
    //REG(0xbf803c70) = 0x0100 ;//0x1c*4, clk_ve_in
    TOP_REG(0x1c) = (TOP_REG(0x1c)&~0x3f)|0x00;//0x1c*4, clk_ve_in
    g_VEInfo.bUse_ADC_BuffOut = TRUE;
    g_VEInfo.bFixColorOut = FALSE;
#endif

	// TTX insertion	swen 20081027
    //start line end line
    Drv_TVEncoder_Write(BK_VE_ENC(0x58), 0x0007);
    Drv_TVEncoder_Write(BK_VE_ENC(0x59), 0x0016);
    Drv_TVEncoder_Write(BK_VE_ENC(0x5a), 0x0140);
    Drv_TVEncoder_Write(BK_VE_ENC(0x5b), 0x014f);

    //vbi ttx mode
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7|BIT3, BIT7|BIT3);

    Drv_TVEncoder_Write(BK_VE_SRC(0x39), 0x8080);
    return TRUE;
}
#if 0
//------------------------------------------------------------------------------
/// Set the output Type of video encoder
///
///       00 : CVBS,Y,C
///       01 : CVBS,Y,Cb,Cr
///       10 : CVBS,R,G,B
///
/// @return None
//------------------------------------------------------------------------------
void MHal_VE_Set_OuputType(MS_VE_VE_OUTPUT_TYPE enOutputType)
{
    U8 OutputType;

    switch(enOutputType)
    {
    case MS_VE_CVBS_OUTPUT:
    case MS_VE_SVIDEO_OUTPUT:
        OutputType = 0; //CVBS, Y, C
        break;
    case MS_VE_YCBCR_OUTPUT:
        OutputType = 1; //CVBS, Y, Cb, Cr
        //color convert
        Drv_TVEncoder_Write(BK_VE_ENC(0x41), 0x40);
        Drv_TVEncoder_Write(BK_VE_ENC(0x42), 0x0);
        Drv_TVEncoder_Write(BK_VE_ENC(0x43), 0x35);
        Drv_TVEncoder_Write(BK_VE_ENC(0x44), 0x25);
        Drv_TVEncoder_Write(BK_VE_ENC(0x45), 0x19);
        Drv_TVEncoder_Write(BK_VE_ENC(0x46), 0x4B);
        break;
    case MS_VE_RGB_OUTPUT:
        OutputType = 2; //CVBS, R, G, B
        //color convert
        Drv_TVEncoder_Write(BK_VE_ENC(0x41), 0x40);
        Drv_TVEncoder_Write(BK_VE_ENC(0x42), 0x00);
        Drv_TVEncoder_Write(BK_VE_ENC(0x43), 0x49);
        Drv_TVEncoder_Write(BK_VE_ENC(0x44), 0x25);
        Drv_TVEncoder_Write(BK_VE_ENC(0x45), 0x19);
        Drv_TVEncoder_Write(BK_VE_ENC(0x46), 0x82);
        break;
    default:
#ifdef RED_LION
        VE_HAL_DBG("[VE] : no support output Type!\n");
#else
        printf("[VE] : no support output Type!\n");
#endif
        return;
    }

    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x40),OutputType,0x03);

}
#endif
//------------------------------------------------------------------------------
/// [Obsolete] [Apply MHal_VE_Set_OuputType() instead]
/// Set the output destination of video encoder
///
///              None SCART CVBS SVIDEO YPbPr
///       None    O     O     O     O     O
///       SCART   O     -     X     X     X
///       CVBS    O     X     -     O     O
///       S_VIDEO O     X     O     -     X
///       YPbPr   O     X     O     X     -
///
/// @param  -pSwitchInfo \b IN/OUT: the information of switching output destination of TV encodeer
/// @return None
//------------------------------------------------------------------------------
void MHal_VE_SwitchOuputDest(PMS_SWITCH_VE_DEST_INFO pSwitchInfo)
{
    // check the combination is valid
    MS_VE_OUTPUT_DEST_TYPE out0, out1;
    VE_HAL_DBG("MHal_VE_SwitchOuputDest\n");
    if(pSwitchInfo->u8DestIdx != 0 ||
       pSwitchInfo->OutputDstType != MS_VE_DEST_CVBS)
    {
        pSwitchInfo->Status = MS_VE_SWITCH_DST_INVALID_PARAM;
        VE_HAL_DBG("VE:unsupported output device\n");
        return;
    }

    if(pSwitchInfo->OutputDstType >= MS_VE_DEST_NUM)
    {
        VE_HAL_DBG("[VE] : unsupported output device\n");
        pSwitchInfo->Status = MS_VE_SWITCH_DST_INVALID_PARAM;
    }
    else
    {
        g_VEInfo.OutputDestType[pSwitchInfo->u8DestIdx] = pSwitchInfo->OutputDstType;

        out0 = g_VEInfo.OutputDestType[0];
        out1 = g_VEInfo.OutputDestType[1];
        VE_HAL_DBG("[VE] : out0 is %d, out1 is %d\n",out0,out1);

        if(VE_OUT_MATCH_TBL[out0][out1].bValid == TRUE)
        {
            pSwitchInfo->Status = MS_VE_SWITCH_DST_SUCCESS;
            Drv_TVEncoder_WriteMask(BK_VE_ENC(0x40), VE_OUT_MATCH_TBL[out0][out1].u8DACType, BIT1|BIT0);

            //if(!g_VEInfo.bUse_ADC_BuffOut) todo:
            Drv_VE_Set_ColorConvert(g_VEInfo.VideoSystem, g_VEInfo.u8DACType);
        }
        else
        {
            pSwitchInfo->Status = MS_VE_SWITCH_DST_INVALID_COMBINATION;
        }
    }
}

//------------------------------------------------------------------------------
/// Control the output of video encoder
/// @param  -pOutputCtrl \b IN: the control information of VE
/// @return MS_MTSTATUS
//------------------------------------------------------------------------------
#if 0
void MHal_VE_Set_OutputCtrl(PMS_VE_OUTPUT_CTRL pOutputCtrl) // purpose to do FRC
{
    if(pOutputCtrl->bEnable)
    {
        Drv_TVEncoder_Write(BK_VE_ENC(0x78), 0x0000); // disable MV
        //Drv_TVEncoder_Write(BK_VE_ENC(0x3e), 0x00a8); // reg_start_pixnum
        //Drv_TVEncoder_Write(BK_VE_ENC(0x3f), 0x0417); // reg_start_linenum
        Drv_TVEncoder_Write(BK_VE_SRC(0x00), 0x0049); // enable TV encoder
    }
    else
    {
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x000), 0, BIT2|BIT1|BIT0); // disable En_CCIR, En_TVE, En_VE
    }
}
#endif


//------------------------------------------------------------------------------
/// set the input source of video encoder
///
/// @param  -pSwitchInfo \b IN/OUT: the information of switching input destination of TV encodeer
/// @return None
//------------------------------------------------------------------------------
void MHal_VE_SwitchInputSrc(PMS_SWITCH_VE_SRC_INFO pSwitchInfo)
{
    VE_HAL_DBG("MHal_VE_SwitchInputSrc %d\n",pSwitchInfo->InputSrcType );
    switch (pSwitchInfo->InputSrcType) {
        case MS_VE_SRC_MAIN:
            VE_HAL_DBG("MS_VE_SRC_MAIN\n");
            break;
        case MS_VE_SRC_SUB:
            VE_HAL_DBG("MS_VE_SRC_SUB\n");
            break;
        case MS_VE_SRC_SCALER:
            VE_HAL_DBG("MS_VE_SRC_SCALER\n");
            break;
        case MS_VE_SRC_NONE:
            VE_HAL_DBG("MS_VE_SRC_NONE\n");
            break;
        default:
            VE_HAL_DBG("MS_VE_SRC_ERROR\n");
            break;
    }
    g_VEInfo.u8VE_DisplayStatus &= ~(EN_VE_CCIR656_IN | EN_VE_RGB_IN);

    if(pSwitchInfo->InputSrcType != MS_VE_SRC_SCALER &&
       pSwitchInfo->InputSrcType != MS_VE_SRC_MAIN &&
       pSwitchInfo->InputSrcType != MS_VE_SRC_SUB)
    {
        g_VEInfo.InputSrcType = MS_VE_SRC_NONE;
        pSwitchInfo->Status = MS_VE_SWITCH_SRC_INVALID_PARAM;
        VE_HAL_DBG("VE:Invalid Input parameter\n");
        return;
    }

    if(pSwitchInfo->InputSrcType == MS_VE_SRC_SCALER)
    {
        g_VEInfo.InputSrcType = MS_VE_SRC_SCALER;
        pSwitchInfo->Status = MS_VE_SWITCH_SRC_SUCCESS;
        VE_HAL_DBG("VE:scaler input source in for setop_box\n");
    }
#if 0   // titania bringup: fix me
    if(pSwitchInfo->InputSrcType == MS_VE_SRC_MAIN ||
       pSwitchInfo->InputSrcType == MS_VE_SRC_SUB)
    {
        if(stSystemInfo.enInputSourceType == INPUT_SOURCE_NONE
#if (INPUT_YPBPR_VIDEO_COUNT > 0)
           || stSystemInfo.enInputSourceType == INPUT_SOURCE_YPBPR
#endif
          )
        {
            g_VEInfo.InputSrcType = MS_VE_SRC_MAIN;
            pSwitchInfo->Status = MS_VE_SWITCH_SRC_SUCCESS;
            //printf("VE support bypass DTV signal for TTX,2007/04/10\n");
            //g_VEInfo.InputSrcType = MS_VE_SRC_NONE;
            //pSwitchInfo->Status = MS_VE_SWITCH_SRC_FAIL;
            //printf("VE:unsupported input source\n");
            //return;
        }
    }
#endif
    _MHal_VE_Get_Window(pSwitchInfo->Timing); // get capture window

    g_VEInfo.InputSrcType = pSwitchInfo->InputSrcType;
    Drv_TVEncoder_Write(BK_VE_SRC(0x11), 0x0080); //default VE Hardware debug mode

    if(pSwitchInfo->InputSrcType == MS_VE_SRC_SCALER)
    {
        g_VEInfo.u8VE_DisplayStatus |= EN_VE_DEMODE;
#if 0   //titania bring up
        MDrv_WriteRegBit(L_BK_VE_SRC(0x5A), ENABLE, BIT0); // enable scaler in
        MDrv_Write2Byte(L_BK_VE_ENC(0x03E), 0x00E6); // start pixnum
        MDrv_Write2Byte(L_BK_VE_ENC(0x03F), 0x0017); // start linenum
        MDrv_Write2Byte(H_BK_VE_ENC(0x03F), 0x0004); // start linenum
        if(g_VEInfo.bUse_ADC_BuffOut)
           MHal_VE_Set_ADC_BuffOut(ENABLE, INPUT_PORT_MS_DTV);
#endif
    }
    else
    {
#if 0   // titania bring up
        switch(stSystemInfo.enInputPortType)
        {
        case INPUT_PORT_ADC_RGB:
            g_VEInfo.u8VE_DisplayStatus |= EN_VE_RGB_IN;
            g_VEInfo.u8VE_DisplayStatus &= ~EN_VE_DEMODE;
            break;

        case INPUT_PORT_ADC_HDMIA:
            g_VEInfo.u8VE_DisplayStatus |= EN_VE_RGB_IN;
            g_VEInfo.u8VE_DisplayStatus |= EN_VE_DEMODE;
            break;
#if (INPUT_YPBPR_VIDEO_COUNT >= 1)
        case INPUT_PORT_ADC_YPBPR:
            g_VEInfo.u8VE_DisplayStatus &= ~EN_VE_DEMODE;
            Drv_TVEncoder_WriteMask(BK_VE_SRC(0x10), 0x00, BIT5|BIT4); // YCbCr in
            MDrv_WriteRegBit(L_BK_VE_SRC(0x7E), 1, BIT4);  // source sync from scaler
            MDrv_WriteByte(L_BK_VE_SRC(0x41), 0x17); //let YPbPr show all image
            break;
#endif
        case INPUT_PORT_MS_CCIR656:
            g_VEInfo.u8VE_DisplayStatus |= (EN_VE_DEMODE | EN_VE_CCIR656_IN);
            break;
        case INPUT_PORT_MS_DTV:
            g_VEInfo.u8VE_DisplayStatus |= EN_VE_DEMODE;
            Drv_TVEncoder_WriteMask(BK_VE_SRC(0x41), 0x12, 0x00FF);// set the sync polarity
            break;

        default:
            g_VEInfo.u8VE_DisplayStatus &= ~EN_VE_DEMODE;
            break;
        }
#endif
        // titania bring up
        //g_VEInfo.u8VE_DisplayStatus |= EN_VE_DEMODE;
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x41), 0x12, 0x00FF);// set the sync polarity

        if(g_VEInfo.u8VE_DisplayStatus & EN_VE_RGB_IN)
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x10), 1, BIT6); // enable RGB in
        else
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x10), 0, BIT6); // disable RGB in

        if(g_VEInfo.u8VE_DisplayStatus & EN_VE_CCIR656_IN)
            Drv_TVEncoder_WriteMask(BK_VE_SRC(0x10), BIT4, BIT5|BIT4); // video source isCCIR656
        else
            Drv_TVEncoder_WriteMask(BK_VE_SRC(0x10), 0, BIT5|BIT4); // video source is YCbCr

        _MHal_VE_Set_IPMux(stSystemInfo.enInputPortType);

#if 0
        if(g_VEInfo.bUse_ADC_BuffOut)
        {
            if(pSwitchInfo->InputSrcType == MS_VE_SRC_SCALER)
                MHal_VE_Set_ADC_BuffOut(ENABLE, INPUT_PORT_MS_DTV);
            else
                MHal_VE_Set_ADC_BuffOut(ENABLE, stSystemInfo.enInputPortType);
        }
#endif

        // titania bringup
        //if(g_SrcInfo.u8DisplayStatus & DISPLAYWINDOW_INTERLACE)
		//LGE gbtogether(080929) --> by chen.
		//Monitor Out Problem in case of DTV progressive signal.
        if (g_VEInfo.bInterlace)
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x10), 1, BIT0);
        else
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x10), 0, BIT0);

        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x5A), DISABLE, BIT0);// disable scaler in
    }

    pSwitchInfo->Status = MS_VE_SWITCH_SRC_SUCCESS;
}

void MHal_VE_Set_OutputCtrl(PMS_VE_OUTPUT_CTRL pOutputCtrl)
{

    BOOL bOutCCIR656, bOutTVE; //, bFieldInv;
    U16 u16EnVal;
//	U8  u8ADCATOP38;//,u8Bank,u8DetectStatus;
//	U16 u16ADCATOP38;
    VE_HAL_DBG("MHal_VE_Set_OutputCtrl\n");

/*    if(g_VEInfo.InputSrcType != MS_VE_SRC_MAIN) // only support main source
    {
        #if ( CVBS_OUT ==  ENABLE )
        #else
          return;    //titania_bright_up
        #endif
    }*/

    //  set firstSend
    firstSend = 1;  // Mstar andy 080917

    if(pOutputCtrl->bEnable)
    {
        // set capture window
        _MHal_VE_Set_CaptureWin();
#if 0       // titania bring up
        if(g_VEInfo.InputSrcType == MS_VE_SRC_SCALER)
        {
             MDrv_Scaler_Set_DestDevice(MS_SCALER_DEST_TVENCODER);

            // disable Scaling
            MDrv_WriteRegBit(H_BK_VE_SRC(0x00), 1, BIT0);
            MDrv_Write2Byte(L_BK_VE_SRC(0x16), 0x0000);
            MDrv_WriteRegBit(H_BK_VE_SRC(0x00), 1, BIT1);
            MDrv_Write2Byte(L_BK_VE_SRC(0x17), 0x0000);

            // disable FRC
            MDrv_WriteRegBit(L_BK_VE_SRC(0x20), 1, BIT0); // disable Full drop
            MDrv_Write4Byte(L_BK_VE_SRC(0x21),  0x00);
            MDrv_Write4Byte(L_BK_VE_SRC(0x30),  0x00);
            MDrv_WriteByteMask(L_BK_VE_SRC(0x35), _BIT2, _BIT3|_BIT2);
#if ( CVBS_OUT ==  ENABLE )
            MDrv_WriteByte(L_BK_VE_SRC(0x41), 0x04);
            MDrv_Write2Byte(L_BK_VE_ENC(0x3E),0x100);
#endif
        }
        else
#endif
        {
            _MHal_VE_Set_ScalingRatio(); // scaling down ratio
            _MHal_VE_Set_FRC(pOutputCtrl->u16FrameRate); // frame rate convert
        }

        // eanble bit
        bOutCCIR656 = (pOutputCtrl->OutputType == MS_VE_OUT_CCIR656) ? TRUE : FALSE;
        bOutTVE     = (pOutputCtrl->OutputType == MS_VE_OUT_TVENCODER) ? TRUE : FALSE;
        if(g_VEInfo.InputSrcType == MS_VE_SRC_SCALER)
            u16EnVal = 0x04;
        else
            u16EnVal = ((g_VEInfo.u8VE_DisplayStatus & EN_VE_DEMODE)<< 7) | (bOutTVE << 2) | (bOutCCIR656 << 1);

        if(bOutCCIR656)
        {
            if(g_VEInfo.VideoSystem <= MS_VE_NTSC_J)
            {   // NTSC
                Drv_TVEncoder_WriteTbl(tVE_CCIROUT_NTSC_TBL);
                Drv_TVEncoder_WriteBit(BK_VE_SRC(0x40), DISABLE, BIT0); // CCIR656 out is NTSC
            }
            else
            {   // PAL
                Drv_TVEncoder_WriteTbl(tVE_CCIROUT_PAL_TBL);
                Drv_TVEncoder_WriteBit(BK_VE_SRC(0x40), ENABLE, BIT0); // CCIR656 out is PAL
            }
        }

        // Field Invert
#if 0   // titania bring up
        if(g_VEInfo.u8VE_DisplayStatus & EN_VE_DEMODE)
        {
			//MDrv_WriteRegBit(L_BK_VE_SRC(0x10), (ENABLE_VE_FIELD_INV & 0x01), BIT1);
            bFieldInv = (g_SrcInfo.u8DisplayStatus & DISPLAYWINDOW_INTERLACE) ?
                        ENABLE_VE_FIELD_INV : 0;
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x10), (bFieldInv & 0x01), BIT1);
        }
        else
        {
            bFieldInv = (g_SrcInfo.stDBreg.u16V_CapStart & 0x01) ?
                        ~ENABLE_VE_FIELD_INV : ENABLE_VE_FIELD_INV;
            bFieldInv = (g_SrcInfo.u8DisplayStatus & DISPLAYWINDOW_INTERLACE) ?
                        bFieldInv : 0;
            Drv_TVEncoder_WriteBit(BK_VE_SRC(0x10), (bFieldInv & 0x01), BIT1);
#else
            // titania bring up
            if (pOutputCtrl->bFieldInvert)
			//if (g_VEInfo.bFieldInv)
            	Drv_TVEncoder_WriteBit(BK_VE_SRC(0x10), 0x01, BIT1);
			else
				Drv_TVEncoder_WriteBit(BK_VE_SRC(0x10), 0x00, BIT1);

#endif
#if 0
			//printf("-u16V_CapStart %x\n",g_SrcInfo.stDBreg.u16V_CapStart);
            if(g_SrcInfo.stDBreg.u16V_CapStart & 0x01)
                  MDrv_WriteRegBit(L_BK_VE_SRC(0x10), (~ENABLE_VE_FIELD_INV & 0x01), BIT1);
            else
                  MDrv_WriteRegBit(L_BK_VE_SRC(0x10), (ENABLE_VE_FIELD_INV & 0x01), BIT1);
#endif
#if 0
            MDrv_Timer_Delayms(50);

            u8Bank = MDrv_ReadByte(BK_SELECT_00);

            MDrv_WriteByte(BK_SELECT_00, REG_BANK_IP1F2);
            u8DetectStatus = MDrv_ReadByte(H_BK_IP1F2(0x1E));
			//printf("--u8DetectStatus %bx\n",u8DetectStatus);
            if(! ( u8DetectStatus & _BIT3 )) // non-interlace not do field invert
            {
                MDrv_WriteRegBit(L_BK_VE_SRC(0x10), (~ENABLE_VE_FIELD_INV & 0x01), BIT1);
            }
            MDrv_WriteByte(BK_SELECT_00, u8Bank);
        }
#endif
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x000), ENABLE, BIT4); // software reset
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x000), ENABLE, BIT5); // load register,but not affect bit3(VBI output)
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x000), ENABLE, BIT3); // TELETEXT
        Drv_TVEncoder_WriteMask(BK_VE_SRC(0x000), u16EnVal, BIT7|BIT2|BIT1);

        if (_bNeedClear)
        {
            u32 i;
            memset((void *)phys_to_virt(u32VE_MIU_BASE), 0x80, 720 * 576 * 2 * 2);
            for (i = 1; i < 720 * 576 * 2 * 2; i += 2)
            {
                memset((void *)phys_to_virt(u32VE_MIU_BASE + i), 0x10, 1);
            }
            _bNeedClear = FALSE;
        }

        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x000), DISABLE, BIT4);
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x000), DISABLE, BIT5);
    }
#if 0   // titania bring up
    else
    {
        if(g_VEInfo.bUse_ADC_BuffOut)
            MHal_VE_Set_ADC_BuffOut(DISABLE,  stSystemInfo.enInputPortType);

        if(g_VEInfo.InputSrcType == MS_VE_SRC_SCALER)
            MDrv_Scaler_Set_DestDevice(MS_SCALER_DEST_PANEL);

        MDrv_WriteByteMask(L_BK_VE_SRC(0x000), 0, _BIT2|BIT1|BIT0); // disable En_CCIR, En_TVE, En_VE
    }
#endif
}


//------------------------------------------------------------------------------
/// Set the output video standard of video encoder
/// @param  -VideoSystem \b IN: the video standard
/// @return TRUE: supported and success,  FALSE: unsupported or unsuccess
//------------------------------------------------------------------------------
BOOL MHal_VE_Set_Output_VideoStd(MS_VE_VIDEOSYS VideoSystem)
{
    PMS_VE_OUT_VIDEOSYS pVideoSysTbl;
    //U16 reg_temp;

    g_VEInfo.VideoSystem = VideoSystem;

    if(VideoSystem >= MS_VE_VIDEOSYS_NUM)
    {
        VE_HAL_DBG("[VE] : unsupported video standard\n");
        return FALSE;
    }
    else
    {
        pVideoSysTbl = &VE_OUT_VIDEOSTD_TBL[VideoSystem];
        VE_HAL_DBG("[VE] : TV video standard is %d\n",VideoSystem);

        Drv_TVEncoder_WriteTbl(pVideoSysTbl->pVE_TBL);
        Drv_TVEncoder_WriteTbl(pVideoSysTbl->pVE_Coef_TBL); // why disable?
        Drv_TVEncoder_WriteTbl(pVideoSysTbl->pVBI_TBL);

        Drv_TVEncoder_WriteBit(BK_VE_ENC(0x03), pVideoSysTbl->bvtotal_525, BIT3); // vtotal
        Drv_TVEncoder_WriteBit(BK_VE_ENC(0x06), pVideoSysTbl->bPALSwitch, BIT0);  // Palswitch
       //Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), pVideoSysTbl->bPALout, BIT5);

        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 1, BIT5); // load register
        Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 0, BIT5);
        //reg_temp =  TVEncoder_REG(BK_VE_ENC(0x003e));
        //printf("---the 3e is %x\n",reg_temp);
        //Drv_TVEncoder_Write(BK_VE_ENC(0x003e),0x00f9);

        //reg_temp =  TVEncoder_REG(BK_VE_ENC(0x003e));
    }
    return TRUE;
}

// MStar andy 081125
void MHal_VE_Set_VPSData(BOOL bEn, U8* u8VPSData)
{
    if(bEn)
    {
        //vps data
        //byte0 & byte1 are CRI & start code, do not need to set
        Drv_TVEncoder_Write(BK_VE_ENC(0x34), (u8VPSData[3] << 8)  | u8VPSData[2]  );
        Drv_TVEncoder_Write(BK_VE_ENC(0x35), (u8VPSData[5] << 8)  | u8VPSData[4]  );
        Drv_TVEncoder_Write(BK_VE_ENC(0x36), (u8VPSData[7] << 8)  | u8VPSData[6]  );
        Drv_TVEncoder_Write(BK_VE_ENC(0x37), (u8VPSData[9] << 8)  | u8VPSData[8]  );
        Drv_TVEncoder_Write(BK_VE_ENC(0x38), (u8VPSData[11] << 8) | u8VPSData[10] );
        Drv_TVEncoder_Write(BK_VE_ENC(0x39), (u8VPSData[13] << 8) | u8VPSData[12] );
        Drv_TVEncoder_Write(BK_VE_ENC(0x3a),                        u8VPSData[14] );

        //vps line start & end
        Drv_TVEncoder_Write(BK_VE_ENC(0x52), 0x0010);
        Drv_TVEncoder_Write(BK_VE_ENC(0x53), 0x0010);
        Drv_TVEncoder_Write(BK_VE_ENC(0x54), 0x0000);
        Drv_TVEncoder_Write(BK_VE_ENC(0x55), 0x0000);

        //vbi vps mode
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7|BIT1, BIT7|BIT1);
        g_VPSDataExist = 1;
    }
    else
    {
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7, BIT7|BIT1);
        g_VPSDataExist = 0;		// lemonic LGE 20080929
    }
}


void MHal_VE_Set_WSSData(BOOL bEn, U16 u16WSSData) // 071204
{
    //VE_HAL_DBG("MHal_VE_Set_WSSData\n");
    //VE_HAL_DBG("WSSData=%02x, data=%02x\n", bEn, u8WSSData);
    if(bEn)
    {
        //wss data
        Drv_TVEncoder_Write(BK_VE_ENC(0x3B), u16WSSData & 0x3FFF);
        //MDrv_WriteByte(H_BK_VE_ENC(0x3B), 0x00);

        //wss line start & end
        Drv_TVEncoder_Write(BK_VE_ENC(0x56), 0x0017);
        Drv_TVEncoder_Write(BK_VE_ENC(0x57), 0x0017);
        Drv_TVEncoder_Write(BK_VE_ENC(0x6C), 0x0000);
        Drv_TVEncoder_Write(BK_VE_ENC(0x6D), 0x0000);

        //vbi wss mode
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7|BIT2, BIT7|BIT2);
        g_WSSDataExist = 1;
    }
    else
    {
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7, BIT7|BIT2);
        g_WSSDataExist = 0;		// lemonic LGE 20080929
    }
}


BOOL MHal_VE_Set_CCData(BOOL bEn, U16 u16CCData0, U16 u16CCData1) // 071204
{
    BOOL bRet=FALSE;
    //VE_HAL_DBG("MHal_VE_Set_WSSData\n");
    //VE_HAL_DBG("WSSData=%02x, data=%02x\n", bEn, u8WSSData);
    if (bEn) {
        if ( (g_CCWritePtr > g_CCReadPtr) &&
                (g_CCWritePtr - g_CCReadPtr < MAX_CC_DATA)) {
            g_CCData[g_CCWritePtr%MAX_CC_DATA] = u16CCData1<<16 | u16CCData0;
            //VE_HAL_DBG("Write %x\n", g_CCWritePtr);
            g_CCWritePtr++;
            bRet = TRUE;
        } else if ( g_CCWritePtr < g_CCReadPtr &&
                ( ((int) g_CCWritePtr + 256 - g_CCReadPtr) < MAX_CC_DATA)){
            g_CCData[g_CCWritePtr%MAX_CC_DATA] = u16CCData1<<16 | u16CCData0;
            //VE_HAL_DBG("Write %x\n", g_CCWritePtr );
            g_CCWritePtr++;
            bRet = TRUE;
        } else if (g_CCWritePtr == g_CCReadPtr) {
            g_CCData[g_CCWritePtr%MAX_CC_DATA] = u16CCData1<<16 | u16CCData0;
            //VE_HAL_DBG("Write %x\n", g_CCWritePtr );
            g_CCWritePtr++;
            bRet = TRUE;
	    }

    }
    return bRet;
#if 0
    if(bEn)
    {
        Drv_TVEncoder_Write(BK_VE_ENC(0x2B), u16CCData0 );
        Drv_TVEncoder_Write(BK_VE_ENC(0x2C), u16CCData1 );
        //MDrv_WriteByte(H_BK_VE_ENC(0x3B), 0x00);

        Drv_TVEncoder_Write(BK_VE_ENC(0x4e), 0x0015);
        Drv_TVEncoder_Write(BK_VE_ENC(0x4f), 0x0015);
        Drv_TVEncoder_Write(BK_VE_ENC(0x50), 0x011c);
        Drv_TVEncoder_Write(BK_VE_ENC(0x51), 0x011c);

        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7|BIT0, BIT7|BIT0);
        //g_WSSDataExist = 1;
    }
    else
    {
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7, BIT7|BIT0);
    }
#endif
}

#if 0
BOOL VE_SavePKtDataInBuffer(U32 u32Count, U32 bufferAddr)
{
    U16 packet_addr;
    static U16 dataCount=0;
    static U8 dataZone=0;
    static U32 lastSendTime=0;
    static U8 firstSend=1;
    // 170/2/17 = 5
    static U8 fieldPerFire=5;
    //static U_INT8 fieldPerFire=((TT_OUTPUT_BUFFER_TOTAL_PACKET_COUNT/2)/17);
    static U8 MaxDataCount=84;//(0~84=>85) 85 will write out to font buffer memory
    U32 pktAddr;
    U32 gVBIBufferBaseAddrHi;
    U32 gVBIBufferBaseAddr;
    U32 virtSrc, virtDst;

    VE_HAL_DBG("Save %d Addr %x\n", u32Count,bufferAddr);
    if(lastSendTime==0)
    {
        //lastSendTime=msAPI_Timer_GetTime0();
    }

    //addr = (addr & 0x1fffffff)>> 3;

    tt_ve_addr_hi = (U16)(bufferAddr >> 19);    //dst
    tt_ve_temp_addr = (U16)((bufferAddr>>3) & 0xffff);
    gVBIBufferBaseAddr = bufferAddr+8192+u32Count*48; // src
    gVBIBufferBaseAddrHi = (U16)(gVBIBufferBaseAddr >> 19);
    pktAddr = (U16)((gVBIBufferBaseAddr>>3) & 0xffff);
    virtDst = (U32) ioremap(bufferAddr, 8192*2);
    virtSrc = virtDst + 8192;
    memcpy((void*)(virtDst + dataZone*510 + dataCount * 6), (void*)(virtSrc + u32Count*48), 48);
    if(pktAddr)
    {
        // 170/2 = 85, 85*6 = 510
        packet_addr = tt_ve_temp_addr + dataZone*510 + dataCount * 6;
        //packet_addr = tt_ve_temp_addr + dataZone*(TT_OUTPUT_BUFFER_TOTAL_PACKET_COUNT/2)*6+dataCount * 6;
        dataCount++;

        if(dataCount>MaxDataCount)
        {
            dataCount=MaxDataCount;
        }
        Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x08), (pktAddr & 0x00ff)<<8 , 0xff00); // low src
        Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x09), pktAddr>>8, 0x00ff); // high src
        Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x09), gVBIBufferBaseAddrHi<<8, 0xff00);

        Drv_TVEncoder_Write(BK_VBI_DMA(0x0a), packet_addr);
        Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0b), tt_ve_addr_hi, 0x00ff);

        Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0b), 0x0600, 0xff00 );  // cnt L
        Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0c), 0x0000, 0x00ff);  // cnt H

        Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0c), 0x0000, 0xff00);  // func

        Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0d), 0x0001, 0x00ff); // fire
        msleep_interruptible(100);
    }


    //if(firstSend||((MDrv_Read2Byte(BK_VE_SRC(0x08)))&0x0004)||(msAPI_Timer_GetTime0()-lastSendTime)>=(20*fieldPerFire*2))
    if(firstSend||((TVEncoder_REG(BK_VE_SRC(0x08)))&0x0004)) //||(msAPI_Timer_GetTime0()-lastSendTime)>=(40*fieldPerFire))
    {
        if(dataCount>MaxDataCount)
        {
            VE_HAL_DBG("exceed output power! %d\n",dataCount);
            dataCount=MaxDataCount;
        }

        #if 0
        if(!((MDrv_Read2Byte(BK_VE_SRC(0x08)))&0x0004)&&!firstSend)
        {
            //printf("no done!\n");
        }
        #endif

        {
            U8 linePerField=0;
            U8 i=0;

            linePerField=(dataCount+fieldPerFire-1)/fieldPerFire;

            if(dataCount==0)
            {
                dataCount=fieldPerFire;
                linePerField=1;
                //printf("fill empty data!\n");
            }
            //VE_HAL_DBG("data Count %d, lineperfield %d\n",dataCount,linePerField);
            //lastSendTime=msAPI_Timer_GetTime0();

            Drv_TVEncoder_Write(BK_VE_ENC(0x3c),0xffff);
            for(i=linePerField;i<16;i++)
            {
                Drv_TVEncoder_Write(BK_VE_ENC(0x3c),TVEncoder_REG(BK_VE_ENC(0x3c))<<1);
            }
            if(linePerField>16)
            {
                Drv_TVEncoder_Write(BK_VE_ENC(0x3d), 0x01);
            }
            else
            {
                Drv_TVEncoder_Write(BK_VE_ENC(0x3d), 0x00);
            }
        }

        Drv_TVEncoder_Write(BK_VE_SRC(0x05), (U16)(dataCount * 6 - 1));
        dataCount=0;

        // 170/2 = 85, 85*6 = 510
        Drv_TVEncoder_Write(BK_VE_SRC(0x03), tt_ve_temp_addr+dataZone*510);
        //MsWriteVEReg(BK_VE_SRC(0x03), tt_ve_temp_addr+dataZone*(TT_OUTPUT_BUFFER_TOTAL_PACKET_COUNT/2)*6);
        Drv_TVEncoder_Write(BK_VE_SRC(0x04), tt_ve_addr_hi);


        Drv_TVEncoder_Write(BK_VE_SRC(0x09),(TVEncoder_REG(BK_VE_SRC(0x09))|0x0400));
        Drv_TVEncoder_Write(BK_VE_SRC(0x09),(TVEncoder_REG(BK_VE_SRC(0x09))&0xfbff));

        if (TVEncoder_REG(BK_VE_SRC(0x43))&0x0002)
        {
            Drv_TVEncoder_Write(BK_VE_SRC(0x43),TVEncoder_REG(BK_VE_SRC(0x43))&0xfffd);
        }
        else
        {
            Drv_TVEncoder_Write(BK_VE_SRC(0x43),TVEncoder_REG(BK_VE_SRC(0x43))|0x0002);
        }

        //dataZone=(dataZone+1)%2;
        if(dataZone==0)
            dataZone = 1;
        else
            dataZone = 0;

        firstSend=0;

        {
            Drv_TVEncoder_Write(BK_VBI_DMA(0x0a), tt_ve_temp_addr+dataZone*510);
            Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0b), tt_ve_addr_hi, 0x00ff);

            Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0b), (0x1FE & 0x00ff)<<8 , 0xff00 );  // cnt L
            Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0c), (0x1FE & 0xff00)>>8 , 0x00ff);  // cnt H

            Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0c), 0x1F00 , 0xff00);  // func

            Drv_TVEncoder_WriteMask(BK_VBI_DMA(0x0d), 0x01, 0x00ff); // fire
            msleep_interruptible(100);
        }
    }
    return TRUE;
}
#endif

// u32PktAddr should be filled with data
// and should be physical address!!
BOOL MHal_VE_SaveTTX(U32 u32LineFlag, U32 u32Size,U32 u32PktAddr)
{
#if 0 /* unused variable. by LGE. jjab. */
    U16 packet_addr;
    U32 u32PacketAddr;
    static U32 lastSendTime=0;
    static U8 fieldPerFire=5;
    U32 u32LineMask;
    int i;
    static U8 dataZone=0;
#endif

    //static U16 dataCount=0;
    static U8 u8FailCnt = 0;
    //U8  u8Retry;				// changed to remove warning(dreamer@lge.com)
#if 1	// Mstar andy 080924
    static U32 u32Count = 0;
    static U32 u32TotalSize = 0;
    U32 u32BufAddr = 0;

    // 0x2800 is for ttx insertion, samuel 20081105
    u32BufAddr = (VE_FRAMEBUFFER_ADR + VE_FRAMEBUFFER_LEN)-0x2800 ; //TTX_BUF_ADR;

    memcpy((void *)ioremap(u32BufAddr + u32TotalSize, u32Size), (void *)ioremap(u32PktAddr, u32Size), u32Size);
    u32TotalSize += u32Size;
    u32Count++;

    if (u32Count < 5)
    {
        return TRUE;

    }
    else
    {
        u32PktAddr = u32BufAddr;
        u32Size = u32TotalSize;
        u32TotalSize = 0;
        u32Count = 0;
    }
#endif
#if 0	// Mstar andy 080924
    // if last insertion is not done, wait until done
    // we already did this at last of this function
    // but checking again for safty
    for(u8Retry = 0; u8Retry < 40 ; u8Retry++)
    {
        if (TVEncoder_REG(BK_VE_SRC(0x08)) & 0x0004)
            break;

        msleep(1);
    }
#endif
    //if (((TVEncoder_REG(BK_VE_SRC(0x08)))&0x0004) == 0)
    //    VE_HAL_DBG("BK_VE_SRC(0x08) = %x \n", (TVEncoder_REG(BK_VE_SRC(0x08))));

    //dataCount = u32Size / 48;
    //if(firstSend||((MDrv_Read2Byte(BK_VE_SRC(0x08)))&0x0004)||(msAPI_Timer_GetTime0()-lastSendTime)>=(20*fieldPerFire*2))
    //mask interrupt bit3
    if(firstSend||((TVEncoder_REG(BK_VE_SRC(0x08)))&0x0004))//||(msAPI_Timer_GetTime0()-lastSendTime)>=(40*fieldPerFire))
    {
        #if 0  // Mstar andy 080917
        if(dataCount>MaxDataCount)
        {
            VE_HAL_DBG("exceed output power! %d\n",dataCount);
            dataCount=MaxDataCount;
        }
        #endif
        #if 0
        if(!((MDrv_Read2Byte(BK_VE_SRC(0x08)))&0x0004)&&!firstSend)
        {
            //printf("no done!\n");
        }
        #endif

        {
/*            U8 linePerField=0;
            U8 i=0;

            linePerField=(dataCount+fieldPerFire-1)/fieldPerFire;

            if(dataCount==0)
            {
                dataCount=fieldPerFire;
                linePerField=1;
                //printf("fill empty data!\n");
            }*/

            //lastSendTime=msAPI_Timer_GetTime0();

            //Drv_TVEncoder_Write(BK_VE_ENC(0x3c),0xffff);
#if 0
            u32LineMask = 0;
            for (i=0; i<nrLines; i++) {
                u32LineFlag=0x00000001;
                if (pLines[i] <=22 && pLines[i] >=7){
                    u32LineFlag <<= (pLines[i]-7);
                    u32LineMask |= u32LineFlag;
                }
                u32LineFlag=0x00010000;
                if (pLines[i] <=335 && pLines[i]<=320) {
                    u32LineFlag <<= (pLines[i]-335);
                    u32LineMask |= u32LineFlag;
                }
            }
#endif

            //VE_HAL_DBG("u32LineMask :%08x\n", u32LineFlag);
            //line mask
            u32LineFlag = 0xffffffff;	/* MStar andy 081105 */
            Drv_TVEncoder_Write(BK_VE_ENC(0x3c),u32LineFlag & 0xffff);
            Drv_TVEncoder_Write(BK_VE_ENC(0x3d),u32LineFlag >> 16);
        }
        //VE_HAL_DBG("data %x count %x\n", dataCount, (U16)(dataCount * 6-1 ));
        //miu size
        Drv_TVEncoder_Write(BK_VE_SRC(0x05), (U16)(u32Size/16 -1));
        //dataCount=0;
        //remap = (U8*)ioremap(u32PktAddr, 48);		// Msatr andy 080917
#if 0
        for (i=0;i<48;i++){
            if (i%4==0)
                VE_HAL_DBG("\n");
            VE_HAL_DBG("%02x ", remap[i]);
        }
        VE_HAL_DBG("packet addr: %x\n", u32PktAddr);
#endif
        tt_ve_temp_addr = (u32PktAddr >>4) & 0xffff;
        tt_ve_addr_hi = (u32PktAddr >>4) >>16;
        // 170/2 = 85, 85*6 = 510
        //miu base address
        Drv_TVEncoder_Write(BK_VE_SRC(0x03), tt_ve_temp_addr);
        //MsWriteVEReg(BK_VE_SRC(0x03), tt_ve_temp_addr+dataZone*(TT_OUTPUT_BUFFER_TOTAL_PACKET_COUNT/2)*6);
        Drv_TVEncoder_Write(BK_VE_SRC(0x04), tt_ve_addr_hi);
        // temporary
#if 0	// move to init function 	swen 20081027
        //start line end line
        Drv_TVEncoder_Write(BK_VE_ENC(0x58), 0x0007);
        Drv_TVEncoder_Write(BK_VE_ENC(0x59), 0x0016);
        Drv_TVEncoder_Write(BK_VE_ENC(0x5a), 0x0140);
        Drv_TVEncoder_Write(BK_VE_ENC(0x5b), 0x014f);

        //vbi ttx mode
        Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2E), BIT7|BIT3, BIT7|BIT3);
#endif
        //interrupt force & interrupt clear
        Drv_TVEncoder_Write(BK_VE_SRC(0x09),(TVEncoder_REG(BK_VE_SRC(0x09))|0x0400));
        Drv_TVEncoder_Write(BK_VE_SRC(0x09),(TVEncoder_REG(BK_VE_SRC(0x09))&0xfbff));

        Chip_Flush_Memory();
        //start next vbi ttx read
        if (TVEncoder_REG(BK_VE_SRC(0x43))&0x0002)
        {
            Drv_TVEncoder_Write(BK_VE_SRC(0x43),TVEncoder_REG(BK_VE_SRC(0x43))&0xfffd);
        }
        else
        {
            Drv_TVEncoder_Write(BK_VE_SRC(0x43),TVEncoder_REG(BK_VE_SRC(0x43))|0x0002);
        }
        // set firstSend	// Mstar andy 080917
        if (firstSend == 1)
            firstSend =2;
        else
            firstSend =0;
    }
    else
    {
    	// Mstar andy 080924
        // avoid insertion always fail after channel change
        if (++u8FailCnt >= 5 )
        {
            firstSend = 1;
            u8FailCnt = 0;
        }

        return FALSE;
    }
#if 0	// Mstar andy 080924
    //wait until insertion done, upper boundary is 40ms (each frame is about 1/25 sec = 40ms)
    for(u8Retry = 0; u8Retry < 40 ; u8Retry++)
    {
        if (TVEncoder_REG(BK_VE_SRC(0x08)) & 0x0004)
            break;

        msleep(1);
    }
#endif
    return TRUE;
}

#if 0
//------------------------------------------------------------------------------
// Adjust contrast
// @param  -u8Constrast \b IN: adjusting value for contrast, range:0~FFh
// @return MS_MTSTATUS
//------------------------------------------------------------------------------
void MHal_VE_Adjust_Contrast(U8 u8Constrast)
{
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x04),u8Constrast,0x00ff);
}
//------------------------------------------------------------------------------
// Adjust Hue
// @param  -u8Hue \b IN: adjusting value for Hue, range:0~FFh
// @return none
//------------------------------------------------------------------------------
void MHal_VE_Adjust_Hue(U8 u8Hue)
{
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x05),u8Hue,0xff00);
}

//------------------------------------------------------------------------------
// Adjust saturation
// @param  -u8Saturation \b IN: adjusting value for saturation, range:0~FFh
// @return none
//------------------------------------------------------------------------------
void MHal_VE_Adjust_Saturation(U8 u8USaturation, U8 u8VSaturation)
{
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2a),u8USaturation,0x00ff);
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x2a),u8VSaturation,0xff00);
}

//------------------------------------------------------------------------------
// Adjust brightness
// @param  -u8brightness \b IN: adjusting value for brightness, range:0~FFh
// @return none
//------------------------------------------------------------------------------
void MHal_VE_Adjust_Brightness(U8 u8brightness)
{
    Drv_TVEncoder_WriteMask(BK_VE_ENC(0x04),u8brightness,0xff00);
}
#endif

//------------------------------------------------------------------------------
/// VE power trun on
/// @return none
//------------------------------------------------------------------------------
void MHal_VE_PowerOn(void)
{
    Drv_TVEncoder_WriteMask(BK_CLKGEN0(0x24), 0x0800, 0x0F0F);                  // clock of ve & vedac
    Drv_TVEncoder_WriteMask(BK_CLKGEN0(0x25), CKG_VE_IN_CLK_MPEG0, 0x00FF);     // clock of ve_in
}

//------------------------------------------------------------------------------
/// VE power trun off
/// @return none
//------------------------------------------------------------------------------
void MHal_VE_PowerOff(void)
{
    Drv_TVEncoder_WriteMask(BK_CLKGEN0(0x24), 0x0101, 0x0101);                  // clock of ve & vedac
    Drv_TVEncoder_WriteMask(BK_CLKGEN0(0x25), 0x0001, 0x0001);                  // clock of ve_in
}

//------------------------------------------------------------------------------
/// VE enable/disable
/// @return none
//------------------------------------------------------------------------------
void MHal_VE_Enable(BOOL bEnable)
{
    Drv_TVEncoder_WriteBit(BK_VE_SRC(0x000), bEnable, BIT0);
}

//------------------------------------------------------------------------------
/// VE reset
/// @return none
//------------------------------------------------------------------------------
void MHal_VE_Reset(BOOL bEnable)
{
    Drv_TVEncoder_WriteBit(BK_VE_SRC(0x000), bEnable, BIT1);
    Drv_TVEncoder_WriteBit(BK_VE_ENC(0x007), bEnable, BIT0);
}

//------------------------------------------------------------------------------
/// VE generates test pattern
/// @return none
//------------------------------------------------------------------------------
void MHal_VE_GenTestPattern(void)
{
    Drv_TVEncoder_WriteBit(BK_VE_ENC(0x003), 1, BIT4);
    Drv_TVEncoder_WriteBit(BK_VE_SRC(0x00), 1, BIT10);

}

//------------------------------------------------------------------------------
/// VE SetBlackScreen
/// @return none
//------------------------------------------------------------------------------
void MHal_VE_SetBlackScreen(BOOL bEnable)
{
    Drv_TVEncoder_WriteBit(BK_VE_SRC(0x038), bEnable << 6, BIT6);
}

#if 0
///-----------------------------------------------------------------------------
/// Set Current Input Src
/// @param  EN_INPUT_PORT_TYPE enInputPortType
/// @return void
///-----------------------------------------------------------------------------
void MHal_VE_SetCurInput(EN_INPUT_PORT_TYPE enInputPortType)
{

	_gInputPortType= enInputPortType;//LGE gbtogether(090129)
}
///-----------------------------------------------------------------------------
/// Get Current Input Src
/// @param  void
/// @return EN_INPUT_PORT_TYPE
///-----------------------------------------------------------------------------
EN_INPUT_PORT_TYPE MHal_VE_GetCurInput(void)
{
	return _gInputPortType;//LGE gbtogether(090129)
}
#endif

