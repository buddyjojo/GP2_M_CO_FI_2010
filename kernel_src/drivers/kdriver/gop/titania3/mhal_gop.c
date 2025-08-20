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
// File name: drvGOP.c
// Description: Graphics Output Processor (GOP) Module
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------------------------------

#ifdef RED_LION
#include <linux/kernel.h>	/* printk() */
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#else
#include "MsCommon.h"
#include "drvScaler.h"
#endif

#include "mhal_gop_reg.h"
#include "mhal_gop.h"
#include "mhal_chiptop_reg.h"

#include "chip_int.h"

#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
#include <linux/module.h>
#endif
//-------------------------------------------------------------------------------------------------
// Local Defines
//-------------------------------------------------------------------------------------------------
//#define FPGA_TEST

#ifdef RED_LION
#define ASSERT(EX) // 0

#define REG(addr) (*(volatile unsigned int *)(addr))
#define REG_8(addr) (*(volatile unsigned char *)(addr))
#endif

#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
/*
** here for update frame buffer address when vsync. interrupt and Tag from GE returned
*/
#define MAX_FLIP_ADDR_FIFO 	(0x10)

volatile GOP_FLIP_INFO _GopFlipInfo[MAX_GOP_SUPPORT+1][MAX_FLIP_ADDR_FIFO];
volatile U32 _GopFlipInfoReadPtr[MAX_GOP_SUPPORT+1] = {0};
volatile U32 _GopFlipInfoWritePtr[MAX_GOP_SUPPORT+1] = {0};

FPTR_CHECKTAGID fptrCheckTagId = NULL;
EXPORT_SYMBOL(fptrCheckTagId);
#endif

#define GWIN_SDRAM_NULL     0x30
#define GOP_ALIGNMENT       8  //8 byte(64bit) alignment.
#define GOP_ALIGN_RSHIFT    4  //Right shift 4 bits for HW alignment(128bit).
#define GOP_RESOURCE_CONTROL 0
#ifdef RED_LION
#define GOP_ACK_COUNTER      200		// About 200Ms // by LGE. jjab.
#else
#define GOP_ACK_COUNTER      0X100000	// About 200Ms
#endif
#define _CALC_STRIDE(bpl)     (((bpl)+15)&0xfffffff0)
#define GOP_OP_H_Offset      11//12

typedef enum
{
    E_REG_HTOTAL,
    E_REG_STRWIN_HSIZE,
    E_REG_STRWIN_VSIZE,
    E_REG_STRWIN_HRATIO,
    E_REG_STRWIN_VRATIO,
    E_REG_GOP0_GWIN0_ENABLE,
    E_REG_GOP0_GWIN1_ENABLE,
    E_REG_GOP0_GWIN2_ENABLE,
    E_REG_GOP0_GWIN3_ENABLE,
    E_REG_GOP_MUX,
    E_REG_GOP_MIUSEL,
    E_REG_MAX,
}_TRUE_MOTION_INFO;

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define LO16BIT(x)  (x & 0x0000ffff)
#define HI16BIT(x)  (x >> 16)


#if (EN_DBG_GOP)
#define DBG_GOP(p)     p
#else
#define DBG_GOP(p)
#endif


//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------
static GopGwinInfo _GWinInfo[TOTAL_GWIN_SUPPORT]; //support 4 + 2 + 1 GWIN
static GopInfo _GopInfo[MAX_GOP_SUPPORT+1];
static B16 gGOP_Test_Pattern_Init=FALSE;
static U32 u32StoreChip;
//static U32 u32StoreSC[7];
static U32 u32StoreB3[70];
static U32 u32StoreB4[50];
static U32 u32B3registers[70];
static GOP_MirrorInfo sGOP_Mirror;
static _TRUE_MOTION_INFO _TrueMotionInfo[E_REG_MAX];

#ifdef RED_LION
static DECLARE_MUTEX(gop_sem);
extern void msleep(unsigned int msec);
#endif


//-------------------------------------------------------------------------------------------------
// Local Function Prototypes
//-------------------------------------------------------------------------------------------------
/// GWIN functions
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
static void _MHal_GOP_ResetFlipInfoPtr(void);
#endif
static B16 _MHal_GOP_Select(GOP_HW_Type eGOP_Type, U16 blBank0);
static B16 _MHal_GOP_Mux_Select(GOP_HW_Type eGOP_Type, GOP_MUX eGOP_Mux);
static B16 _MHal_GOP_Init(GOP_HW_Type eGOP_Type);
static B16 _MHal_GOP_DstPlane_Set(GOP_HW_Type eGOP_Type, GOPDstType eDstType);
static B16 _MHal_GOP_Output_Progressive_Enable (GOP_HW_Type eGOP_Type, U16 bProgress);

#if 0 // NOTE: H & V duplicate is removed in Titania
static B16 _MHal_GOP_Output_HDup_Enable (GOP_HW_Type eGOP_Type, U16 bEnable);
static B16 _MHal_GOP_Output_VDup_Enable (GOP_HW_Type eGOP_Type, U16 bEnable);
#endif

static B16 _MHal_GOP_Blink_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
static B16 _MHal_GOP_Blink_SetRate(GOP_HW_Type eGOP_Type, U32 u32Rate);
//static void _MHal_GOP_UpdateReg(GOP_HW_Type eGOP_Type);
void _MHal_GOP_UpdateReg(GOP_HW_Type eGOP_Type);
static B16 _MHal_GOP_YUVOut_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
static B16 _MHal_GOP_Palette_Set( GOP_HW_Type eGOP_Type, GopPaletteEntry *pPalEntry, U32 u32Index);
static B16 _MHal_GOP_TransClr_Set(GOP_HW_Type eGOP_Type,  GOPTRSColor TRSColor);
static B16 _MHal_GOP_TransClr_Enable( GOP_HW_Type eGOP_Type, U16 bEnable);
static B16 _MHal_GOP_Scroll_SetRate(GOP_HW_Type eGOP_Type,U32 u32Rate);

#if 0 // NOTE: Pixel-shift is removed in Titania
static U16 _MHal_GOP_Pixel_Shift_Set (GOP_HW_Type eGOP_Type, U8 u8ShiftValue);
#endif

static B16 _MHal_GOP_Int_GetStatus(GOP_HW_Type eGOP_Type, U16 *pbIntStatus);
static B16 _MHal_GOP_Int_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
static B16 _MHal_GOP_GWIN_ErrCheck(GOP_HW_Type eGOP_Type, U8 u8Wid);
static B16 _MHal_GOP_GWIN_WinInfo_Set(GOP_HW_Type eGOP_Type, U8 u8Wid, GopGwinInfo* pInfo);
static B16 _MHal_GOP_GWIN_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable);
static void _MHal_GOP_GWIN_Init(GOP_HW_Type eGOP_Type, U8 u8Wid );
static B16 _MHal_GOP_GWIN_Scroll_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, GopScrollType eScrollType, U8 u16ScrollStep, U16 bEnable);
static B16 _MHal_GOP_GWIN_Blending_Set(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable, U32 u32Alpha);
static B16 _MHal_GOP_GWIN_Scroll_AutoStop_VSet(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32ScrollAutoVStop);
static B16 _MHal_GOP_GWIN_Scroll_AutoStop_HSet(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32ScrollAutoHStop);
static B16 _MHal_GOP_GWIN_Scroll_AutoStop_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, GopScrollAutoStopType eScrollStopType, U16 bEnable);
B16 _MHal_GOP_Clk_Set (GOP_HW_Type eGOP_Type, GOPDstType eDstType);
B16 _MHal_GOP_Clk_Enable(GOP_HW_Type eGOP_Type, U16 bEnable );
static B16 _MHal_GOP_GWIN_Alpha_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable);
static B16 _MHal_GOP_VMirror_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
static B16 _MHal_GOP_HMirror_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
///DWIN functions
static B16 _MHal_GOP_DWIN_CaptureStream_Enable(U16 bEnable);
static B16 _MHal_GOP_DWIN_CaptureMode_Set(GopCaptureMode eCaptureMode);
static B16 _MHal_GOP_DWIN_Progressive_Enable(U16 bEnable);
static B16 _MHal_GOP_DWIN_WinInfo_Set(GOP_DWIN_INFO* pinfo);
static B16 _MHal_GOP_DWIN_Int_Enable(GOPDWINIntType eIntType, U16 bEnable);
static B16 _MHal_GOP_DWIN_GetIntrStatus(U16* pu16IntStatus);
static B16 _MHal_GOP_DWIN_Set_PINPON(U32 addr, U32 upbond);

static U16 _CalculatePitch(U16 width, GOPColorType pixelFormat)
{
    U16 pxlDepth;

    switch(pixelFormat)
    {
    case E_GOP_COLOR_ARGB8888:     //ARGB8888
        pxlDepth = width << 2;
        break;
    case E_GOP_COLOR_I8:     //I8
        pxlDepth = width;
        break;
    case E_GOP_COLOR_ARGB1555:
        pxlDepth = width << 1;
        break;
    case E_GOP_COLOR_RGB565:
        pxlDepth = width << 1;
        break;
    case E_GOP_COLOR_ARGB4444:
        pxlDepth = width << 1;
        break;
    default:
        return 0;
    }

    return (_CALC_STRIDE(pxlDepth));
}

/******************************************************************************/
// Changed HS_PIPE register value by Mingchia
// @param u8GOP_num
//   - [0]: GOP_4G
//   - [1]: GOP_2G
//   - [2]: GOP_1G
//   - [3]: GOP_1GX (Not used)
// @param u16HSPD: new HS_PIPE value
/******************************************************************************/
void MHal_GOP_AdjustHSPD(GOP_HW_Type u8GOP_num, U16 u16HSPD)
{
    U8 u8Offset, u8CurBankOft;
    GOP_mux_sel sGopMux;
    GOP_wr_ack sGop_wr_ack;

    sGop_wr_ack = *(volatile GOP_wr_ack *)(REG_GOP_WR_ACK);
    u8CurBankOft = sGop_wr_ack.GOP_BANK_SEL;
    if (u8GOP_num==E_GOP4G_0)
        sGop_wr_ack.GOP_BANK_SEL = 0;
    else if (u8GOP_num==E_GOP2G_1)
        sGop_wr_ack.GOP_BANK_SEL = 3;
    else if (u8GOP_num==E_GOP1G)
        sGop_wr_ack.GOP_BANK_SEL = 7;
    else if (u8GOP_num==E_GOP1GX)
        sGop_wr_ack.GOP_BANK_SEL = 10;
    else
        printk("\n[%s] error: not support this gop", __FUNCTION__);
    *(volatile GOP_wr_ack *)(REG_GOP_WR_ACK) = sGop_wr_ack;

    sGopMux = *(volatile GOP_mux_sel *)(REG_GOP_MUX_SEL);
    if(((1==sGopMux.GOPG0_MUX_SEL) && (0==u8GOP_num)) || ((1!=sGopMux.GOPG0_MUX_SEL) && (1==u8GOP_num)))
    {
        u8Offset = 0x1;
    }
    else
    {
        u8Offset = 0x0;
    }

    sGop_wr_ack.GOP_BANK_SEL = u8CurBankOft;
    REG(REG_GOP_HS_PIPE) = u8Offset+u16HSPD;
    *(volatile GOP_wr_ack *)(REG_GOP_WR_ACK) = sGop_wr_ack;
}

/******************************************************************************/
// init DWIN
// @param: NOON
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_Init(void)
{

    U32 u32RegVal;

    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    //set wbe
    u32RegVal = REG(GOP_DW_WBE);

    ((GOP_dwin_wbe*)&u32RegVal)->GOP_DWIN_1STR_WBE = 0xf;
    ((GOP_dwin_wbe*)&u32RegVal)->GOP_DWIN_1END_WBE = 0xf;

    REG(GOP_DW_WBE) = u32RegVal;

    //select OP source
    u32RegVal = REG(GOP_DWIN_CTL0_EN);
    ((GOP_dwin_ctrl_en*)&u32RegVal)->GOP_DWIN_SRC_SEL = 0; //select OP source
    ((GOP_dwin_ctrl_en*)&u32RegVal)->GOP_DWIN_UV_SWAP = 0; //Disable UV SWAP

    REG(GOP_DWIN_CTL0_EN) = u32RegVal;


    _MHal_GOP_DWIN_CaptureMode_Set(E_Cap_EachFrame);

    return TRUE;
}

/******************************************************************************/
// Capture frames Enable / Disable
// @param bEnable \b IN
//   - # TRUE start capture
//   - # FALSE stop capture
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_CaptureStream_Enable(U16 bEnable)
{
    U32 u32RegVal;

    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    u32RegVal = REG(GOP_DWIN_CTL0_EN);

   ((GOP_dwin_ctrl_en*)&u32RegVal)->GOP_DWIN_EN = bEnable;

    REG(GOP_DWIN_CTL0_EN) = u32RegVal;

   return TRUE;

}

/******************************************************************************/
// Set DWIN capture mode (capture one fram or each fram)
// @param eCaptureMode \b IN DWIN capture mode
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_CaptureMode_Set(GopCaptureMode eCaptureMode)
{
    U32 u32RegVal;

    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    if (eCaptureMode > E_Cap_OneFrame)
        return FALSE;

    u32RegVal = REG(GOP_DWIN_CTL0_EN);

   ((GOP_dwin_ctrl_en*)&u32RegVal)->GOP_DWIN_CAPTURE_MODE = eCaptureMode;

    REG(GOP_DWIN_CTL0_EN) = u32RegVal;

   return TRUE;

}

//Arki >>
/******************************************************************************/
// Set DWIN InputSource mode (set InputSource Of Scaler or VOP)
// @param eDwin_Data_Src_Mode \b IN DWIN InputSource mode
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_InputSourceMode_Set(EN_GOP_DWIN_DATA_SRC eDwin_Data_Src_Mode)
{
    U32 u32RegVal;

    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    if (eDwin_Data_Src_Mode > GOPDWIN_DATA_SRC_MAX)
        return FALSE;

    u32RegVal = REG(GOP_DWIN_CTL0_EN);

   ((GOP_dwin_ctrl_en*)&u32RegVal)->GOP_DWIN_SRC_SEL = eDwin_Data_Src_Mode;

    REG(GOP_DWIN_CTL0_EN) = u32RegVal;

   return TRUE;

}

static B16 _MHal_GOP_DWIN_CLKGen_Set(EN_GOP_DWIN_DATA_SRC eDwin_Data_Src_Mode)
{
    U32 u32RegVal;

    u32RegVal = REG(REG_CHIPTOP_CLK_GOP2_GOPD);

//printk("\n[CLKGen_Set]: GOPD MVOP: 0x%X\n", u32RegVal);

    if (eDwin_Data_Src_Mode == GOPDWIN_DATA_SRC_VOP)
    {
        u32RegVal &= ~(GOPD_CLK_DISABLE);
        u32RegVal = (u32RegVal & GOPD_CLK_SEL_BITS) | GOPD_CLK_SEL_DC0;
        DBG_GOP (printk("\n[CLK_Set]: GOPD MVOP: 0x%X\n", u32RegVal));
    }
    else
    {
        u32RegVal &= ~(GOPD_CLK_DISABLE);
        u32RegVal = (u32RegVal & ~GOPD_CLK_SEL_BITS) | GOPD_CLK_SEL_ODCLK;
        DBG_GOP (printk("\n[CLK_Set]: GOP1G OP: 0x%X\n", u32RegVal));
    }

    REG(REG_CHIPTOP_CLK_GOP2_GOPD) = u32RegVal;

    return TRUE;
}
//Arki <<

/******************************************************************************/
// Set DWIN capture in interlaced or progressive mode
// @param bEnable \b IN
//   - # TRUE  DWIN progressive mode
//   - # FALSE DWIN interlaced mode
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_Progressive_Enable(U16 bEnable)
{

    U32 u32RegVal;

    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    u32RegVal = REG(GOP_DWIN_CTL0_EN);

   ((GOP_dwin_ctrl_en*)&u32RegVal)->GOP_DWIN_PROGRESSIVE = bEnable;

    REG(GOP_DWIN_CTL0_EN) = u32RegVal;

   return TRUE;

}

/******************************************************************************/
// Set DWIN setting to registers
// @param pinfo \b IN \copydoc GOP_DWIN_INFO
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_WinInfo_Set(GOP_DWIN_INFO* pinfo)
{

    U32 u32RegVal;
    U32 u32MinUpBound;

    ASSERT(pinfo);

    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    //Set format
    u32RegVal = REG(GOP_DWIN_CTL0_EN);

   ((GOP_dwin_ctrl_en*)&u32RegVal)->GOP_DWIN_TYPE = pinfo->eDwinFmt;

    REG(GOP_DWIN_CTL0_EN) = u32RegVal;

    REG(GOP_DW_VSTR) = pinfo->u16VPixelStart; //Pix

    REG(GOP_DW_VEND) = pinfo->u16VPixelEnd;   //Pix

    REG(GOP_DW_HSTR) = pinfo->u16HPixelStart*2 >> GOP_ALIGN_RSHIFT;   //1 Pix = 2 Byte

    REG(GOP_DW_HEND) = pinfo->u16HPixelEnd*2 >> GOP_ALIGN_RSHIFT;   //1 Pix = 2 Byte

    REG(GOP_DW_HSIZE) = (pinfo->u16HPixelEnd - pinfo->u16HPixelStart)*2 >> GOP_ALIGN_RSHIFT;   //1 Pix = 2 Byte

    REG(GOP_DW_DSTR_L) = (pinfo->u32DWINBufAddr >> GOP_ALIGN_RSHIFT) & 0xffff;  //in HW alignment

    REG(GOP_DW_DSTR_H) = (pinfo->u32DWINBufAddr >> GOP_ALIGN_RSHIFT) >> 16;  //in HW alignment

    u32MinUpBound = pinfo->u32DWINBufAddr +(pinfo->u16HPixelEnd - pinfo->u16HPixelStart)*(pinfo->u16VPixelEnd - pinfo->u16VPixelStart)*2 + 8;//1 Pix = 2 Byte

    if (pinfo->u32DWINHBondAddr <  u32MinUpBound)
        pinfo->u32DWINHBondAddr = u32MinUpBound;

    REG(GOP_DW_HBOND_L) = (pinfo->u32DWINHBondAddr >> GOP_ALIGN_RSHIFT) & 0xffff;  //in HW alignment

    REG(GOP_DW_HBOND_H) = (pinfo->u32DWINHBondAddr >> GOP_ALIGN_RSHIFT) >> 16;  //in HW alignment

    REG(GOP_DW_JMPLEN) = (pinfo->u16DRAMJumpLen >> GOP_ALIGN_RSHIFT);  //in HW alignment

   return TRUE;
}


/******************************************************************************/
// Set interrupt mask of GOP DWIN.
// @param eIntType \b IN Interrupt Type
// @param bEnable \b IN
//   - # TRUE enable interrupts
//   - # FALSE disable interrupts
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_Int_Enable(GOPDWINIntType eIntType, U16 bEnable)
{

    U32 u32RegVal;

    if (eIntType > E_GOP_BF_INT)
        return FALSE;

    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    u32RegVal = REG(GOP_DW_INT);

    if(bEnable)
        u32RegVal &= ~eIntType;
    else
        u32RegVal |= eIntType;

    REG(GOP_DW_INT) = u32RegVal;

   return TRUE;
}

/******************************************************************************/
// Get Interrupt Status
// @return Interrupt status
//   - bit3 DWIN VSYNC interrupt
//   - bit2 DWIN interlace Bottom Field ACK Interrupt
//   - bit1 DWIN interlace Top Field ACK Interrupt
//   - bit0 DWIN Progressive ACK Interrupt
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_GetIntrStatus(U16 *pu16IntStatus)
{
    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    *pu16IntStatus = REG(GOP_DW_INT) >> 12;
    return TRUE;
}


/******************************************************************************/
// Set DWIN Pinpon buffer
// @param addr \b IN Base address of DWIN pinpon buffer
// @param upbond \b IN Upper boundary of DWIN pinon buffer
// @return TRUE: success / FALSE: fail
/******************************************************************************/
static B16 _MHal_GOP_DWIN_Set_PINPON(U32 addr, U32 upbond)
{

    _MHal_GOP_Select(E_GOP_DWIN, FALSE);

    //PinPon buffer address
    REG(GOP_DW_PON_DSTR_L) = (addr >> GOP_ALIGN_RSHIFT)&0xffff;
    REG(GOP_DW_PON_DSTR_H) = (addr >> GOP_ALIGN_RSHIFT)>>16;

    REG(GOP_DW_PON_HBOND_L) = (upbond >> GOP_ALIGN_RSHIFT)&0xffff;
    REG(GOP_DW_PON_HBOND_H) = (upbond >> GOP_ALIGN_RSHIFT)>>16;

    return TRUE;
}

/******************************************************************************/
// Enable/Disable GOP clock
// @param eGOP_Type \b IN GOP Type
// @param bEnable \b IN TRUE: Enable / FALSE: Disable
// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 _MHal_GOP_Clk_Enable(GOP_HW_Type eGOP_Type, U16 bEnable )
#if T3   //T3
{
    U32 u32RegVal;

    switch (eGOP_Type)
    {
        case E_GOP4G_0:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1);

            if (bEnable)
            {
                u32RegVal &= ~(GOP0_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOP0_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOP0_GOP1) = u32RegVal;
            DBG_GOP (printk("\n[CLK_Enable]: GOP4G: 0x%X\n", u32RegVal));
            break;


        case E_GOP2G_1:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1);

            if (bEnable)
            {
                u32RegVal &= ~(GOP1_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOP1_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOP0_GOP1) = u32RegVal;
            DBG_GOP (printk("\n[CLK_Enable]: GOP2G: 0x%X\n", u32RegVal));
            DBG_GOP (printk("\n Set GOP 2G to SC. \n"));

            #if SUBTITLE_IN_IP
            REG_8(REG_SC_BASE) = 0x00; //Switch to bank0
            REG_8(REG_SC_BASE + 0x5*4) = (BIT5 | BIT7);
            REG_8(REG_SC_BASE + 0x5*4 + 1) = BIT5;
            #endif

            break;

        case E_GOP1G:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP2_GOPD);
            if (bEnable)
            {
                u32RegVal &= ~(GOP2_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOP2_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOP2_GOPD) = u32RegVal;
            DBG_GOP (printk("\n[CLK_Enable]: GOP1G: 0x%X\n", u32RegVal));
            break;

        case E_GOP_DWIN:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP2_GOPD);

            if (bEnable)
            {
                u32RegVal &= ~(GOPD_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOPD_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOP2_GOPD) = u32RegVal;

            break;

        case E_GOP1GX:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP3);
            if (bEnable)
            {
                u32RegVal &= ~(GOP3_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOP3_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOP3) = u32RegVal;
            DBG_GOP (printk("\n[CLK_Enable]: GOP1GX: 0x%X\n", u32RegVal));
            break;


        default:
            ASSERT(eGOP_Type>E_GOP_DWIN);
            return FALSE;
            break;

    }

    return TRUE;
}
#else   //T2
{
    U32 u32RegVal;

    switch (eGOP_Type)
    {
        case E_GOP4G_0:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD);

            if (bEnable)
            {
                u32RegVal &= ~(GOP0_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOP0_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD) = u32RegVal;
            break;


        case E_GOP2G_1:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD);

            if (bEnable)
            {
                u32RegVal &= ~(GOP1_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOP1_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD) = u32RegVal;

            break;

        case E_GOP1G:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOPS);
            if (bEnable)
            {
                u32RegVal &= ~(GOPS_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOPS_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOPS) = u32RegVal;
            break;

        case E_GOP_DWIN:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD);

            if (bEnable)
            {
                u32RegVal &= ~(GOPS_CLK_DISABLE);
            }
            else
            {
                u32RegVal |= (GOPS_CLK_DISABLE);
            }
            REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD) = u32RegVal;

            break;

        default:
            ASSERT(eGOP_Type>E_GOP_DWIN);
            return FALSE;
            break;

    }

    return TRUE;
}
#endif


//-------------------------------------------------------------------------------------------------
// Enable/Disable GOP HW CLK
// @param GOP_HW_Type \b IN: GOP HW Type
// @param eDstType    \b IN: GOP Clk IN IP/OP
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 _MHal_GOP_Clk_Set (GOP_HW_Type eGOP_Type, GOPDstType eDstType)
#if T3   //T3
{

    U32 u32RegVal;
    switch (eGOP_Type)
    {
        case E_GOP4G_0:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1);

            if (eDstType == E_GOP_DST_IP)
            {
                u32RegVal = (u32RegVal & ~GOP0_CLK_SEL_MASK) | GOP0_CLK_SEL_IP;
                DBG_GOP (printk("\n[CLK_Set]: GOP4G IP: 0x%X\n", u32RegVal));
            }
            else
            {
                u32RegVal = (u32RegVal & ~GOP0_CLK_SEL_MASK) | GOP0_CLK_SEL_ODCLK;
                DBG_GOP (printk("\n[CLK_Set]: GOP4G OP: 0x%X\n", u32RegVal));
            }


            REG(REG_CHIPTOP_CLK_GOP0_GOP1) = u32RegVal;
            break;

        case E_GOP2G_1:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1);

            if (eDstType == E_GOP_DST_IP)
            {
                u32RegVal = (u32RegVal & ~GOP1_CLK_SEL_MASK) | GOP1_CLK_SEL_IP;
                DBG_GOP (printk("\n[CLK_Set]: GOP2G IP: 0x%X\n", u32RegVal));
            }
            else
            {
                u32RegVal = (u32RegVal & ~GOP1_CLK_SEL_MASK) | GOP1_CLK_SEL_ODCLK;
                DBG_GOP (printk("\n[CLK_Set]: GOP2G OP: 0x%X\n", u32RegVal));
            }


            REG(REG_CHIPTOP_CLK_GOP0_GOP1) = u32RegVal;
            break;

        case E_GOP1G:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP2_GOPD);

            if (eDstType == E_GOP_DST_IP)
            {
                u32RegVal = (u32RegVal & ~GOP2_CLK_SEL_MASK) | GOP2_CLK_SEL_IP;
                DBG_GOP (printk("\n[CLK_Set]: GOP1G IP: 0x%X\n", u32RegVal));
            }
            else
            {
                u32RegVal = (u32RegVal & ~GOP2_CLK_SEL_MASK) | GOP2_CLK_SEL_ODCLK;
                DBG_GOP (printk("\n[CLK_Set]: GOP1G OP: 0x%X\n", u32RegVal));
            }

            REG(REG_CHIPTOP_CLK_GOP2_GOPD) = u32RegVal;
            break;

        case E_GOP1GX:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP3);

            if (eDstType == E_GOP_DST_IP)
            {
                u32RegVal = (u32RegVal & ~GOP3_CLK_SEL_MASK) | GOP3_CLK_SEL_IP;
                DBG_GOP (printk("\n[CLK_Set]: GOP1GX IP: 0x%X\n", u32RegVal));
            }
            else
            {
                u32RegVal = (u32RegVal & ~GOP3_CLK_SEL_MASK) | GOP3_CLK_SEL_ODCLK;
                DBG_GOP (printk("\n[CLK_Set]: GOP1GX OP: 0x%X\n", u32RegVal));
            }

            REG(REG_CHIPTOP_CLK_GOP3) = u32RegVal;
            break;

        default:
            ASSERT(eGOP_Type>E_GOP1GX);
            return FALSE;
            break;
    }

    return TRUE;

}
#else   //T2
{

    U32 u32RegVal;
    switch (eGOP_Type)
    {
        case E_GOP4G_0:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD);

            if (eDstType == E_GOP_DST_IP)
            {
                u32RegVal = (u32RegVal & ~GOP0_CLK_SEL_MASK) | GOP0_CLK_SEL_IP;
            }
            else
            {
                u32RegVal = (u32RegVal & ~GOP0_CLK_SEL_MASK) | GOP0_CLK_SEL_ODCLK;
            }


            REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD) = u32RegVal;
            break;

        case E_GOP2G_1:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD);

            if (eDstType == E_GOP_DST_IP)
            {
                u32RegVal = (u32RegVal & ~GOP1_CLK_SEL_MASK) | GOP1_CLK_SEL_IP;
            }
            else
            {
                u32RegVal = (u32RegVal & ~GOP1_CLK_SEL_MASK) | GOP1_CLK_SEL_ODCLK;
            }


            REG(REG_CHIPTOP_CLK_GOP0_GOP1_GOPD) = u32RegVal;
            break;

        case E_GOP1G:
            u32RegVal = REG(REG_CHIPTOP_CLK_GOPS);

            if (eDstType == E_GOP_DST_IP)
            {
                u32RegVal = (u32RegVal & ~GOPS_CLK_SEL_MASK) | GOPS_CLK_SEL_IP;
            }
            else
            {
                u32RegVal = (u32RegVal & ~GOPS_CLK_SEL_MASK) | GOPS_CLK_SEL_ODCLK;
            }

            REG(REG_CHIPTOP_CLK_GOPS) = u32RegVal;
            break;

        default:
            ASSERT(eGOP_Type>E_GOP1G);
            return FALSE;
            break;
    }

    return TRUE;

}
#endif

//-------------------------------------------------------------------------------------------------
// Select GOP HW Bank
// @param eGOP_Type \b IN: GOP HW Type (@ref GOP_HW_Type)
// @return TRUE/FALSE
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Select(GOP_HW_Type eGOP_Type, U16 blBank0)
{
    U32 u32RegVal;
    U32 u32BankVal;
    if (eGOP_Type > E_GOP_DWIN)
    {
#ifdef RED_LION
        DBG_GOP (printk ("[GOP Select] GOP select error Bank %d\n",eGOP_Type ));
#else
        DBG_GOP (printk ("[GOP Select] GOP select error Bank %d\n",eGOP_Type ));
#endif
        return FALSE;
    }

    switch (eGOP_Type)
    {
        case E_GOP4G_0:
            u32BankVal = 0;
            if (!blBank0) u32BankVal++;
            break;
        case E_GOP2G_1:
            u32BankVal = 3;
            if (!blBank0) u32BankVal++;
            break;
        case E_GOP_DWIN:
            u32BankVal = 6;
            break;
        case E_GOP1G:
            u32BankVal = 7;
            if (!blBank0) u32BankVal++;
            break;
        case E_GOP1GX:
            u32BankVal = 10;
            if (!blBank0) u32BankVal++;
            break;
        default:
#ifdef RED_LION
            DBG_GOP (printk ("Error GOP type %d",eGOP_Type ));
#else
            DBG_GOP (printk ("Error GOP type %d",eGOP_Type ));
#endif
            return FALSE;
    }
    //printk ("[GOP Sel]: GOP type %d, BANK = %d\n",eGOP_Type, u32BankVal );
    u32RegVal = REG(REG_GOP_WR_ACK);
    ((GOP_wr_ack*)&u32RegVal)->GOP_BANK_SEL = u32BankVal;
    REG(REG_GOP_WR_ACK) = u32RegVal;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Set GOP HW Typte to specific GOP Mux
// @param GOP_HW_Type \b IN: GOP HW Type
// @param GOP_MUX    \b IN: GOP Mux Type
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Mux_Select(GOP_HW_Type eGOP_Type, GOP_MUX eGOP_Mux)
{
    U32 u32RegVal;
    U32 u32BankVal;

    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
        DBG_GOP (printk ("\n\n[RED MUX Select] GOP select error Bank %d, %d\n\n",eGOP_Type, (U32)E_GOP1GX ));
#else
        DBG_GOP (printk ("\n\n[MUX Select] GOP select error Bank %d, %d\n\n",eGOP_Type, (U32)E_GOP1GX ));
#endif
        return FALSE;
    }

    u32RegVal = REG(REG_GOP_MUX_SEL);

    switch (eGOP_Type)
    {
        case E_GOP4G_0: //Bank 0
            u32BankVal = 0;
            break;
        case E_GOP2G_1: //Bank 1
            u32BankVal = 1;
            break;
        case E_GOP1G:   //Bank 3
            u32BankVal = 2;
            break;
        case E_GOP1GX:   //Bank 3
            u32BankVal = 3;
            break;
        default:
#ifdef RED_LION
            DBG_GOP (printk ("Error GOP type %d",eGOP_Type ));
#else
            DBG_GOP (printk ("Error GOP type %d",eGOP_Type ));
#endif
            return FALSE;
    }

    switch (eGOP_Mux)
    {
        case E_TCON_OSD_0:
            ((GOP_mux_sel*)&u32RegVal)->GOPG0_MUX_SEL = u32BankVal;
            break;
        case E_TCON_OSD_1:
            ((GOP_mux_sel*)&u32RegVal)->GOPG1_MUX_SEL = u32BankVal;
            break;
        case E_TCON_OSD_2:
            ((GOP_mux_sel*)&u32RegVal)->GOPG2_MUX_SEL = u32BankVal;
            break;
        case E_TCON_OSD_3:
            ((GOP_mux_sel*)&u32RegVal)->GOPG3_MUX_SEL = u32BankVal;
            break;
        default:
#ifdef RED_LION
            DBG_GOP (printk ("Error GOP Mux %d",eGOP_Mux ));
#else
            DBG_GOP (printk ("Error GOP Mux %d",eGOP_Mux ));
#endif
            return FALSE;

    }

    REG(REG_GOP_MUX_SEL) = u32RegVal;

    //Record Global Variable
    _GopInfo[eGOP_Type].eGopMux= eGOP_Mux;

    return TRUE;
}



//-------------------------------------------------------------------------------------------------
// Force to Write GWIN registers
// @param GOP_HW_Type \b IN: GOP HW Type
// @param  -bEnable \b IN: TRUE/FALSE : force write immediately/write at Vsync
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_ForceWrite(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
        DBG_GOP (printk ("[ForceWrite] GOP select error Bank %d",eGOP_Type ));
#else
        DBG_GOP (printk ("[ForceWrite] GOP select error Bank %d",eGOP_Type ));
#endif
        return FALSE;
    }

    _GopInfo[eGOP_Type].bForceWrite = bEnable;

    return TRUE;
}

//-------------------------------------------------------------------------------- // Sanger 070731
#define SC1_REG_BASE                              (REG_SC_BASE)//(0xBF800000+(0x1780<<2))
//#define SC1_REG_BASE                            (0xBF800000+(0x2F00<<2))
//#define SC2_REG_BASE                            0x3000
//#define SC3_REG_BASE                            0x3100
//#define SC4_REG_BASE                            0x3200
#define SC_BANK_SEL              (SC1_REG_BASE+0x00)
#define SC_GOP1_CHANNEL_SYNC     (SC1_REG_BASE+(0x05<<2))//SC1_REG_BASE+0x0A
//#define SC_GOP2_CHANNEL_SYNC     (SC1_REG_BASE+(0x05<<2) +1)//SC1_REG_BASE+0x0B
#define SC_GOP_EN                (SC1_REG_BASE+(0x06<<2)) // oxD

#define __ERIS_GOP_REG(bk, reg)  	(((U16)(bk)<<8) + (reg) * 2)
#define GOP_4G_CTRL1              	__ERIS_GOP_REG(0, 0x01)
#define GOP_2G_CTRL1              	__ERIS_GOP_REG(3, 0x01)

#define REG_CKG_GOPG0           (0xBF800000+ ((0xF00+0x16)<<2))	//0x1E2C
    #define CKG_GOPG0_GATED         BIT0
    #define CKG_GOPG0_INVERT        BIT1
    #define CKG_GOPG0_MASK          (BIT3 | BIT2)
    #define CKG_GOPG0_ODCLK         (0 << 2)
    #define CKG_GOPG0_0             (1 << 2)
    #define CKG_GOPG0_IDCLK2        (2 << 2)
    #define CKG_GOPG0_XTAL          (3 << 2)

#define REG_CKG_GOPG1           (0xBF800000+ ((0xF00+0x16)<<2)) 	//0x1E2C
    #define CKG_GOPG1_GATED         BIT4
    #define CKG_GOPG1_INVERT        BIT5
    #define CKG_GOPG1_MASK          (BIT7 | BIT6)
    #define CKG_GOPG1_ODCLK         (0 << 6)
    #define CKG_GOPG1_0             (1 << 6)
    #define CKG_GOPG1_IDCLK2        (2 << 6)
    #define CKG_GOPG1_XTAL          (3 << 6)


//void _MHal_GOP_Scaler_Set_GOPEnable(U8 gopNum, B16 bEnable)
void _MHal_GOP_Scaler_Set_GOPEnable(GOP_HW_Type eGOP_Type, B16 bEnable)
{
    U8 bankTemp;

    bankTemp=REG(SC_BANK_SEL);
    REG(SC_BANK_SEL)= 0x00;
    if (bEnable)
    {
        if (eGOP_Type==E_GOP4G_0)
            REG(SC_GOP_EN)= REG(SC_GOP_EN)|0x4000;    // bit E for mux1 -->4G
        else
            REG(SC_GOP_EN)= REG(SC_GOP_EN)|0x8000;    // bit F for mux0 -->2G
    }
    else
    {
        if (eGOP_Type==E_GOP4G_0)
            REG(SC_GOP_EN)=REG(SC_GOP_EN)&(~0x4000);  // bit E for mux1 -->4G
        else
            REG(SC_GOP_EN)=REG(SC_GOP_EN)&(~0x8000);  // bit F for mux0 -->2G
    }

    REG(SC_BANK_SEL)=bankTemp;
}

static B16 _MHal_GOP_Scaler_Set_GOPSel(MS_IPSEL_GOP ipSelGop)
{
    U8 bankTemp;
   U32 u32MuxVal = REG(REG_GOP_MUX_SEL);


    bankTemp=REG(SC_BANK_SEL);

    REG(SC_BANK_SEL)= 0x00;
    switch(ipSelGop)
    {
    case MS_IP0_SEL_GOP0:
        //REG(SC_GOP1_CHANNEL_SYNC)= 0x90;
        //REG(SC_GOP2_CHANNEL_SYNC)= 0x10; //temp solution
        REG(SC_GOP1_CHANNEL_SYNC)= 0x1090;

        break;
    case MS_IP1_SEL_GOP1:
        //REG(SC_GOP2_CHANNEL_SYNC)= 0xA0;
        REG(SC_GOP1_CHANNEL_SYNC)=(REG(SC_GOP1_CHANNEL_SYNC)&0x00FF)|(0xA0<<8);
        break;
    case MS_IP0_SEL_GOP1:
        //REG(SC_GOP1_CHANNEL_SYNC)= 0xa0;
        //REG(SC_GOP2_CHANNEL_SYNC)= 0x20; //temp solution
        if(u32MuxVal & 0x01)    // GOP1 send to mux0
            REG(SC_GOP1_CHANNEL_SYNC)= 0x10a0;
        else    // GOP1 send to mux1
        	REG(SC_GOP1_CHANNEL_SYNC)= 0x20a0;
        break;
#if 1  // These combination should not occur.
    case MS_IP1_SEL_GOP0:
		//??????
        //REG(SC_GOP2_CHANNEL_SYNC)= 0xE0;
        break;
#endif
    case MS_NIP_SEL_GOP0:
        REG(SC_GOP1_CHANNEL_SYNC)= REG(SC_GOP1_CHANNEL_SYNC)&0xFF00;
        break;
    case MS_NIP_SEL_GOP1:
        //REG(SC_GOP2_CHANNEL_SYNC)= 0x00;
        REG(SC_GOP1_CHANNEL_SYNC)= REG(SC_GOP1_CHANNEL_SYNC)&0x00FF;
        break;
    }
    REG(SC_BANK_SEL)= bankTemp;
	return TRUE;
}

#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
void _MHal_GOP_ResetFlipInfoPtr(void)
{
	U32 u32Idx = 0;
	for(; u32Idx < MAX_GOP_SUPPORT+1; u32Idx++)
	{
	    _GopFlipInfoWritePtr[u32Idx] = 0;
	    _GopFlipInfoReadPtr[u32Idx] = 0;
	}
}
#endif
//-------------------------------------------------------------------------------------------------
/// Initialize GOP registers
/// @param u8GOP_num \b IN: GOP number: 0 or 1
/// @return TRUE: sucess /FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Init(GOP_HW_Type eGOP_Type)
{
    U32 u32RegVal, u32I;
	U32 u32GOPOutFmt;
	U16 u16PDOfst = 0;

	u32GOPOutFmt=0;   // 0: for RGB mode 1: YUV mode
    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
        DBG_GOP (printk ("[GOP_Init] GOP select error Bank %d, %d\n",eGOP_Type, (U32)E_GOP1GX ));
#else
        DBG_GOP (printk ("[GOP_Init] GOP select error Bank %d, %d\n",eGOP_Type, (U32)E_GOP1GX ) ));
#endif
        return FALSE;
    }

#if 0
// alex
// chip top init to enable GE GOP
	u32RegVal=REG(0xBF803C54);
	u32RegVal=u32RegVal&0x0FFF;
	REG(0xBF803C54)=u32RegVal;
	u32RegVal=REG(0xBF803C58);
	u32RegVal=u32RegVal&0xF000;
	REG(0xBF803C58)=u32RegVal;
// clk_psram clock setting (chiptop reg 0x2d bit0 <-0)
	u32RegVal=REG(0xBF803CB4);
	u32RegVal=u32RegVal&0xFFFE;
	REG(0xBF803CB4)=u32RegVal;
#endif



    _MHal_GOP_Select(eGOP_Type, TRUE);

    _MHal_GOP_ForceWrite(eGOP_Type, FALSE);

    REG(REG_GOP_SOFT_RESET)   = (0x4008|(u32GOPOutFmt<<10))+(REG(REG_GOP_SOFT_RESET)&1);  //0x4609//enable field inverse & soft reset, disable hs mask, YUV format

    //when configure to IP, 0x7e will not ACK.
    //configure GOP destination to IP for MUXOUT_1 (Arki 0808)
#if T3  //configure GOP destination 0: IP (Main); 1: IP (Sub); 2: OP; 3: MVOP
    u32RegVal = (REG(REG_GOP_DEST) & 0xFFF0) | 0x0002;
    #if SUBTITLE_IN_IP
    if (eGOP_Type == E_GOP2G_1)       //ArkiDebug

    //if (eGOP_Type == E_GOP4G_0)
    {
        u32RegVal = (REG(REG_GOP_DEST) & 0xFFF0) | 0x0000;  //change to IP Main
    }
    else
    {
        u32RegVal = (REG(REG_GOP_DEST) & 0xFFF0) | 0x0002;  //change to OP
    }
    #else
    u32RegVal = (REG(REG_GOP_DEST) & 0xFFF0) | 0x0002;  //change to OP
    #endif
    REG(REG_GOP_DEST)         = u32RegVal;  //DMA length:32, Dest Dest: OP
#else
#if 1
    u32RegVal = (REG(REG_GOP_DEST) & 0xFFF0) | 0x000A;
    //u32RegVal = (REG(REG_GOP_DEST) & 0xFFF0) | 0x0003;
    //u32RegVal=0x5317;   // need to fix in the future
    REG(REG_GOP_DEST)         = u32RegVal;  //DMA length:32, Dest Dest: OP
#else
    u32RegVal = (REG(REG_GOP_DEST) & 0xFFF0) | 0x000A;
    u32RegVal |= 0x0003 ; // Force set GOP output to MVOP for FPGA verification
    //u32RegVal = (REG(REG_GOP_DEST) & 0xFFF0) | 0x0003;
    REG(REG_GOP_DEST)         = u32RegVal;  //DMA length:32, Dest Dest: OP
    printk( "REG_GOP_DEST=%08X\n", REG(REG_GOP_DEST) ) ;
#endif
#endif


    REG(REG_GOP_BLINK)        = 0x1500;  //disable blink, set blink rate and scroll rate
    REG(REG_GOP_GWIN_PRI)     = 0x3210; //GWIN Priority: GWIN0>GWIN1>GWIN2>GWIN3

    REG(REG_GOP_RDMA_HT)      = 0x0000;


#if 0
    REG(REG_GOP_GWIN_EN)        = 0x0390; //enable field inverse & soft reset
    REG(REG_GOP_BLINK)          = 0x3220;
    REG(REG_GOP_GWIN_ROLL)      = 0x0000;
    REG(REG_GOP_GWIN_ALPHA01)   = 0x7F7F; //Initial Alpha Value
    REG(REG_GOP_GWIN_ALPHA02)   = 0x7F7F;
    REG(REG_GOP_PAL_SET)        = 0X0028; //YUV OUT, DMA length:32
    REG(REG_GOP_PAL_SET)        = 0X0028; //YUV OUT, DMA length:32
    REG(REG_GOP_ASTOP)          = 0x0000; //Initial Auto Stop
    REG(REG_GOP_DEST)           = 0x0002; //Dest Dest: OP & Disable hs mask
    REG(REG_GOP_GWIN_PRI)       = 0x00E4; //GWIN Priority: GWIN0>GWIN1>GWIN2>GWIN3
    //TSH? REG_GOP_CLR_PAL_LO
#endif
//	if (eGOP_Type == E_GOP1G) REG(REG_GOP_DMA) = 0x4738;	//OSD Shaking issue. 20090909

    _MHal_GOP_Select(eGOP_Type, FALSE);

    switch (eGOP_Type)
    {
        case E_GOP1G:
            _MHal_GOP_GWIN_Init(E_GOP1G, (U32)NULL);
            REG(REG_GOP_GWIN_SET(1))      = 0x3F00;    // Set constant alpha, and alpha = 0x3F
            REG(REG_GOP_GWIN_VSTOP_LO(1)) =0x0000;
            REG(REG_GOP_GWIN_VSTOP_HI(1)) =0x0000;
            REG(REG_GOP_GWIN_HSTOP(1))    =0x0000;
			REG(REG_GOP_DRAM_RBLK_STR_HI(1))    =0x0000;
            REG(REG_GOP_GWIN_HEND(1))    =0x0000;
            REG(REG_GOP_GWIN_VEND(1))    =0x0000;
            REG(REG_GOP_DRAM_RBLK_HSIZE(1))    =0x0000;
            //initial Scroll Setps
            //REG(REG_GOP_GWIN_VR_LEN(3))   =0x0000;   // No scrolling in 1G
            //REG(REG_GWIN23_HR_LEN)   =0x00;

            //_MHal_GOP_Mux_Select(E_GOP1G, E_TCON_OSD_0);
            #if OSD3s
            _MHal_GOP_Mux_Select(E_GOP1G, E_TCON_OSD_3); //FIXME zuel_090812
            #else
            _MHal_GOP_Mux_Select(E_GOP1G, E_TCON_OSD_2); //FIXME zuel_090812
            #endif

            break;

        case E_GOP1GX:
            _MHal_GOP_GWIN_Init(E_GOP1GX, (U32)NULL);
            REG(REG_GOP_GWIN_SET(1))      = 0x3F00;    // Set constant alpha, and alpha = 0x3F
            REG(REG_GOP_GWIN_VSTOP_LO(1)) =0x0000;
            REG(REG_GOP_GWIN_VSTOP_HI(1)) =0x0000;
            REG(REG_GOP_GWIN_HSTOP(1))    =0x0000;
            //initial Scroll Setps
            //REG(REG_GOP_GWIN_VR_LEN(3))   =0x0000;   // No scrolling in 1G
            //REG(REG_GWIN23_HR_LEN)   =0x00;

            #if (!OSD3s)
            _MHal_GOP_Mux_Select(E_GOP1GX, E_TCON_OSD_3);
            #endif

            break;

        case E_GOP4G_0:
            for (u32I = 0; u32I < MAX_GOP0_GWIN_SUPPORT; u32I++)
            {
                _MHal_GOP_GWIN_Init(eGOP_Type, u32I);
                REG(REG_GOP_GWIN_SET(u32I)) = 0x3F00;  // Init alpha value, disable H&V scroll, auto stop
                //initial Scroll auto stop offset
                REG(REG_GOP_GWIN_HVSTOP_LO(u32I)) =0x0000;
                REG(REG_GOP_GWIN_HVSTOP_HI(u32I)) =0x0000;
                //REG(REG_GOP_GWIN_HSTOP(u32I))    =0x0000;

                //initial Scroll Setps
                REG(REG_GOP_GWIN_SCROLL_LEN(u32I))   =0x0000;
            }

            // In Titania, H&V LEN share the same register address
            //REG(REG_GWIN01_HR_LEN)   =0x00;
            //REG(REG_GWIN23_HR_LEN)   =0x00;

            // It is correct, don't change it as (E_GOP4G_0, E_TCON_OSD_0)
            // Later in MDrv_GOP_DstPlane_Set(), E_GOP4G_0 will use find E_TCON_OSD_0 can be used.
            //LGE 20080521 : for OSD Priority GOP4G must be over GOP2G
            #if T3   //GP2
            #if SUBTITLE_IN_IP
            _MHal_GOP_Mux_Select(E_GOP4G_0, E_TCON_OSD_0); //FIXME zuel_090812        ArkiDebug
            #else
                #if OSD3s
                _MHal_GOP_Mux_Select(E_GOP4G_0, E_TCON_OSD_2);
                #else
    			_MHal_GOP_Mux_Select(E_GOP4G_0, E_TCON_OSD_1);
                #endif
			#endif
            #else   //GP1
            _MHal_GOP_Mux_Select(E_GOP4G_0, E_TCON_OSD_1);
            #endif
            break;

        case E_GOP2G_1:
            for (u32I = 0; u32I < MAX_GOP1_GWIN_SUPPORT; u32I++)
            {
                _MHal_GOP_GWIN_Init(eGOP_Type, u32I);
                REG(REG_GOP_GWIN_SET(u32I)) = 0x3F00;  // Init alpha value, disable H&V scroll, auto stop
                //initial Scroll auto stop offset
                REG(REG_GOP_GWIN_VSTOP_LO(u32I)) =0x0000;
                REG(REG_GOP_GWIN_VSTOP_HI(u32I)) =0x0000;
                //REG(REG_GOP_GWIN_HSTOP(u32I))    =0x0000;
                //initial Scroll Setps
                REG(REG_GOP_GWIN_SCROLL_LEN(u32I))   =0x0000;
            }
            //REG(REG_GWIN01_HR_LEN)   =0x00;
            //REG(REG_GWIN23_HR_LEN)   =0x00;
            //LGE 20080521 : for OSD Priority GOP4G must be over GOP2G
            #if T3   //GP2
			#if SUBTITLE_IN_IP
            _MHal_GOP_Mux_Select(E_GOP2G_1, E_TCON_OSD_1);    //ArkiDebug
            #else
                #if OSD3s
                _MHal_GOP_Mux_Select(E_GOP2G_1, E_TCON_OSD_0);
                #else
                _MHal_GOP_Mux_Select(E_GOP2G_1, E_TCON_OSD_0);
                #endif
			#endif
            #else   //GP1
            _MHal_GOP_Mux_Select(E_GOP2G_1, E_TCON_OSD_0);
            #endif
            break;

    }

    _MHal_GOP_Select(eGOP_Type, TRUE);

    #if SUBTITLE_IN_IP
    if (eGOP_Type == E_GOP2G_1)
    {
        REG(REG_GOP_SOFT_RESET)     = 0x4414|(u32GOPOutFmt<<10); //0x4608;  //Disable soft reset
    }
    else
    {
        REG(REG_GOP_SOFT_RESET)     = 0x400C|(u32GOPOutFmt<<10); //0x4608;  //Disable soft reset
    }
    #else
        REG(REG_GOP_SOFT_RESET)     = 0x400C|(u32GOPOutFmt<<10); //0x4608;  //Disable soft reset
    #endif

    if (Chip_Query_Rev()>=3)
        u16PDOfst = 4; //U04 SC fix something, gop stretch window should shift right 4 pixels

	// KimTH_091026: adjust HSPD 시 overwrite 되는 경우를 피해 gop type별 초기화
    switch (eGOP_Type)
    {
        case E_GOP4G_0:
		 	REG_8(REG_GOP_BASE + 0xfe*2) = 0x0; //Switch to bank0
            REG(REG_GOP_HS_PIPE) = 0x1B+u16PDOfst;
            break;

        case E_GOP2G_1:
		 	REG_8(REG_GOP_BASE + 0xfe*2) = 0x3; //Switch to bank3
            REG(REG_GOP_HS_PIPE) = 0x1A+u16PDOfst;
            break;

        case E_GOP1G:
		 	REG_8(REG_GOP_BASE + 0xfe*2) = 0x7; //Switch to bank7
            REG(REG_GOP_HS_PIPE) = 0x1C+u16PDOfst;
            break;

        case E_GOP1GX:
			REG_8(REG_GOP_BASE + 0xfe*2) = 0xA; //Switch to bank10
            REG(REG_GOP_HS_PIPE) = 0xF1+u16PDOfst;
            break;
        default:
            printk("\n!!!!Error GOP type in [%s]!!!!", __FUNCTION__);
            break;
    }

#if 0 // KimTH_091026
    REG_8(REG_GOP_BASE + 0xfe*2) = 0x0; //Switch to bank0
    REG_8(REG_GOP_BASE + 0xf*4) = 0x1B+u16PDOfst;  //set 4G_HS_PIPE
    REG_8(REG_GOP_BASE + 0xfe*2) = 0x3; //Switch to bank3
    REG_8(REG_GOP_BASE + 0xf*4) = 0x1A+u16PDOfst;  //set 2G_HS_PIPE
    REG_8(REG_GOP_BASE + 0xfe*2) = 0x7; //Switch to bank7
    REG_8(REG_GOP_BASE + 0xf*4) = 0x1C+u16PDOfst;  //set 1G_HS_PIPE
    REG_8(REG_GOP_BASE + 0xfe*2) = 0xA; //Switch to bank10
    REG_8(REG_GOP_BASE + 0xf*4) = 0xF1+u16PDOfst;  //set 1GX_HS_PIPE
#endif

    #if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
        //init vsync interrupt:
        _MHal_GOP_ResetFlipInfoPtr();
        _MHal_GOP_Int_Enable(eGOP_Type,TRUE);
    #endif

    return TRUE;

}


//-------------------------------------------------------------------------------------------------
// Set GOP Destination DisplayPlane
// @param GOP_HW_Type \b IN: GOP HW Type
// @param eDstType \b IN: GOP Destination DisplayPlane Type
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_DstPlane_Set(GOP_HW_Type eGOP_Type, GOPDstType eDstType)
{

    U32 u32RegVal;


    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
        DBG_GOP (printk ("[DstPlane_Set] GOP select error Bank %d",eGOP_Type ));
#else
        DBG_GOP (printk ("[DstPlane_Set] GOP select error Bank %d",eGOP_Type ));
#endif
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    // Set GOP Register for set Dst Plane is OP or IP
    u32RegVal =REG(REG_GOP_DEST);
    ((GOP_dest*)&u32RegVal)->GOP_DEST = eDstType & GOP_DEST_MASK;
    REG(REG_GOP_DEST) = u32RegVal;

    //Record in Gobal Variables.
    _GopInfo[eGOP_Type].eDstType = eDstType;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Enable GOP Output Progressive Mode
// @param eGOP_Type \b IN: GOP HW type select
// @param bProgress \b IN: TRUE: Progressive \ FALSE: Interlace
// @return TRUE: sucess / FALSE: fail
// @note
// - This function influences all Gwins ouput mode of GOP.
// - You must set this function before create Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Output_Progressive_Enable (GOP_HW_Type eGOP_Type, U16 bProgress)
{
    U32 u32RegVal;
    if (eGOP_Type > E_GOP1GX)
    {
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_SOFT_RESET);

    ((GOP_soft_reset*)&u32RegVal)->GWIN_PROG_MD = bProgress;

    REG(REG_GOP_SOFT_RESET) = u32RegVal;

    _GopInfo[eGOP_Type].bProgressiveOut = bProgress;
    return TRUE;
}

#if 0 // TODO: alan has NOT ported it. It can be reahced by streatch nearest mode
//-------------------------------------------------------------------------------------------------
/// Enable GOP Horizontal Duplicate
/// @param eGOP_Type \b IN: GOP HW type select
/// @param bEnable \b IN: TRUE: Enable \ FALSE: Disable
/// @return TRUE: sucess / FALSE: fail
/// @note
/// - This function influences all Gwins ouput mode of GOP.
/// - You must set this function before create Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static U16 _MHal_GOP_Output_HDup_Enable (GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U32 u32RegVal;

    if ((eGOP_Type > E_GOP1G))
    {
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type);


    u32RegVal = REG(REG_GOP_GWIN_EN);

    ((GOP_gwin_en*)&u32RegVal)->GWIN_H_DUP = bEnable;
    REG(REG_GOP_GWIN_EN) = u32RegVal;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Enable GOP Vertical Duplicate
/// @param eGOP_Type \b IN: GOP HW type select
/// @param bEnable \b IN: TRUE: Enable \ FALSE: Disable
/// @return TRUE: sucess / FALSE: fail
/// @note
/// - This function influences all Gwins ouput mode of GOP.
/// - You must set this function before create Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Output_VDup_Enable (GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U32 u32RegVal;

    if ((eGOP_Type > E_GOP1G))
    {
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type);

    u32RegVal = REG(REG_GOP_GWIN_EN);

    ((GOP_gwin_en*)&u32RegVal)->GWIN_V_DUP = bEnable;
    REG(REG_GOP_GWIN_EN) = u32RegVal;

    return TRUE;
}
#endif

//-------------------------------------------------------------------------------------------------
// Set GOP blink rate for BLINK / ALPHA_BLINK GWINs
// @param eGOP_Type \b IN: GOP HW type select
// @param u32Rate \b IN: blink rate (unit : vsync time)
// @return TRUE: sucess / FALSE: fail
// @note  This function set BLINK rate for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Blink_SetRate(GOP_HW_Type eGOP_Type, U32 u32Rate)
{
    U32 u32RegVal;

    if ((eGOP_Type > E_GOP1GX))
    {
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_BLINK);
    ((GOP_blink *)&u32RegVal)->BLINK_RATE = u32Rate;
    REG(REG_GOP_BLINK) = u32RegVal;

    return TRUE;
}



//-------------------------------------------------------------------------------------------------
// Enable/Disable GOP Blink for BLINK / ALPHA_BLINK GWINs
// @param  eGOP_Type \b IN: GOP HW type select
// @param bEnable \b IN: TRUE/FALSE
// @return TRUE: sucess / FALSE: fail
// @note This function enable BLINK for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Blink_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U32 u32RegVal;

    if ((eGOP_Type > E_GOP1GX))
    {
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_BLINK);

    ((GOP_blink *)&u32RegVal)->BLINK_EN = bEnable;

    REG(REG_GOP_BLINK) = u32RegVal;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Set Gop color key for the corresponding color type
// @param  eGOP_Type \b IN: GOP HW type select
// @param  eColorType \b IN: color type
// @return TRUE: sucess / FALSE: fail
// @note  This function set transparent color key for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_TransClr_Set(GOP_HW_Type eGOP_Type,  GOPTRSColor TRSColor)
{
    if ((eGOP_Type > E_GOP1GX))
    {
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

#if 0
    //Set Mask to Zero
    REG(REG_GOP_TRS_CLR_HI)  = 0x0;
    REG(REG_GOP_TRS_MASK_HI) = 0x0;
#endif

    REG(REG_GOP_TRS_CLR_LO) = LO16BIT(TRSColor.u32Data);
    REG(REG_GOP_TRS_CLR_HI) = HI16BIT(TRSColor.u32Data)& 0xFF;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Enable / Disable transparent color for the corresponding color type
// @param  eGOP_Type \b IN: GOP HW type select
// @param bEnable    \b IN: TRUE/FALSE
// @return TRUE: sucess / FALSE: fail
// @note  This function enable transparent color key for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_TransClr_Enable( GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U32 u32RegVal;

    if ((eGOP_Type > E_GOP1GX))
    {
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_SOFT_RESET);

    ((GOP_soft_reset *)&u32RegVal)->TRS_EN = bEnable;

    REG(REG_GOP_SOFT_RESET) = u32RegVal;
    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Set H & V scroll rate
// @param  eGOP_Type \b IN: GOP HW type select
// @param u32Rate \b IN: scroll rate (unit : vsync time)
// @return TRUE: sucess / FALSE: fail
// @note This function set scroll rate for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Scroll_SetRate(GOP_HW_Type eGOP_Type,U32 u32Rate)
{
    U32 u32RegVal;
    U32 arScrollEn[MAX_GOP0_GWIN_SUPPORT];
    U8  u8I, u8GWinNum;

    if ((eGOP_Type > E_GOP2G_1))
    {
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);    // Select bank according to GOP type

    if (E_GOP4G_0 == eGOP_Type)
        u8GWinNum = MAX_GOP0_GWIN_SUPPORT;
    else
        u8GWinNum = MAX_GOP1_GWIN_SUPPORT;

    //must disable H/V scrolls before setting scroll rate; otherwise, setting scroll rate from high to low will hang for a while
    for (u8I = 0; u8I < u8GWinNum; u8I++)
    {
        arScrollEn[u8I] = REG(REG_GOP_GWIN_SET(u8I));
        REG(REG_GOP_GWIN_SET(u8I)) = arScrollEn[u8I] & (~(GWIN_VSCROLL_MASK | GWIN_HSCROLL_MASK));
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_BLINK);
    ((GOP_blink *)&u32RegVal)->ROLL_RATE = u32Rate;
    REG(REG_GOP_BLINK) = u32RegVal;

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);    // Select bank according to GOP type

    for (u8I = 0; u8I < u8GWinNum; u8I++)
    {
        REG(REG_GOP_GWIN_SET(u8I)) = arScrollEn[u8I];
    }

    return TRUE;
}


#if 0 // TODO: alan has NOT ported it. The register is removed
//-------------------------------------------------------------------------------------------------
// _MHal_GOP_Pixel_Shift_Set: set GOP pixel shift (left shift gwin)
// @param  eGOP_Type \b IN: GOP HW type select
// @param u8ShiftValue \b IN: Shift Pixel Value
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Pixel_Shift_Set (GOP_HW_Type eGOP_Type, U8 u8ShiftValue)
{
    U32 u32RegVal=0;

    if ((eGOP_Type > E_GOP1G))
    {
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type);

    u32RegVal = REG(REG_GOP_PAL_SET );
    ((GOP_pal_set*)&u32RegVal)->P_SHIFT = u8ShiftValue;
    REG(REG_GOP_PAL_SET) = u32RegVal;

    return u32RegVal;
}
#endif


#if 0 // LGE wgkwak 080902
////////////////////////////////////////////////////// start
// 20080805 LGE Added for Interrupt Handling
static int _MHal_GOP_ClearIRQ(void)
{
	GOP_HW_Type _GOP_Type;
	U16 u16Gop;
	U32 u32RegVal;

	u16Gop=REG(REG_GOP_WR_ACK);

	if(u16Gop&GOP0_INT_FLAG_MASK)
		_GOP_Type=E_GOP4G_0;
	else if(u16Gop&GOP1_INT_FLAG_MASK)
		_GOP_Type=E_GOP2G_1;
	else if(u16Gop&GOP1_INT_FLAG_MASK)
		_GOP_Type=E_GOP1G;
	else {
		printk("unknow GOP MASK(%04x)\n", u16Gop);
		return 0;
	}

   	_MHal_GOP_Select( _GOP_Type, TRUE);    // Select bank according to GOP type

   	u32RegVal = REG(REG_GOP_INT);
	REG(REG_GOP_INT)=u32RegVal|0x3; // clear vs0 and vs1
	REG(REG_GOP_INT)=u32RegVal|~(0x3); // enable vs0 and vs1

    return 1;
}

extern void MDrv_GOP_VSyncHandle(void);
static irqreturn_t _gop_interrupt(int irq,void *devid, struct pt_regs *regs)
{
	if (_MHal_GOP_ClearIRQ())
 		MDrv_GOP_VSyncHandle();

	return IRQ_HANDLED;
}
////////////////////////////////////////////////////// end
#endif


//-------------------------------------------------------------------------------------------------
// Enable/Disable GWIN VSync interrupt
// @param  eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
// @param bEnable \b IN: TRUE / FALSE
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Int_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
#if 1	// Original Code
    U32 u32RegVal;
    if ((eGOP_Type > E_GOP1GX))
    {
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_INT);
    if(bEnable)
    {
        u32RegVal &= ~(GOP_VS0_INT_MASK | GOP_VS1_INT_MASK);
    }
    else
    {
        u32RegVal |= GOP_VS0_INT_MASK | GOP_VS1_INT_MASK;
    }
    REG(REG_GOP_INT) = u32RegVal;

    return TRUE;
#else	// 20080805 LGE Fixed for Interrupt Handling
	static int fIrqHandle = 0;
    U32 u32RegVal;
    if ((eGOP_Type > E_GOP1G))
    {
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_INT);
    if(bEnable)
    {
		if (!fIrqHandle) {
			request_irq(E_IRQ_GOP, _gop_interrupt, SA_INTERRUPT, "gop", NULL);
			fIrqHandle = 1;
		}
        u32RegVal &= ~(GOP_VS0_INT_MASK | GOP_VS1_INT_MASK);
    }
    else
    {
		if (fIrqHandle) {
			free_irq(E_FIQ_VSYN_GOP1, NULL);
			fIrqHandle = 0;
		}
        u32RegVal |= GOP_VS0_INT_MASK | GOP_VS1_INT_MASK;
    }

    REG(REG_GOP_INT) = u32RegVal;

    return TRUE;
#endif
}



//-------------------------------------------------------------------------------------------------
// Read GWIN VSync interrupt Mask.
// @param  eGOP_Type \b IN : GOP HW type select (@ref GOP_HW_Type)
// @param  bIntStatus\b OUT: TRUE: Enable Vsync INT / FALSE: Disable Vsync INT
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Int_GetStatus(GOP_HW_Type eGOP_Type, U16 * pbIntStatus)
{
    U32 u32RegVal;
    if ((eGOP_Type > E_GOP1GX))
    {
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_INT);
    *pbIntStatus = (!(((GOP_int *)&u32RegVal)->VS0_INT_MASK | ((GOP_int *)&u32RegVal)->VS1_INT_MASK));
    return TRUE;

}


//-------------------------------------------------------------------------------------------------
// Update GOP related registers (move internal register buffer to active registers)
// @param eGOP_Type \b IN: GOP HW Type (reference to GOP_HW_Type)
// @return None
//-------------------------------------------------------------------------------------------------
//static void _MHal_GOP_UpdateReg(U8 eGOP_Type)
void _MHal_GOP_UpdateReg(GOP_HW_Type eGOP_Type)
{
#ifndef RED_LION
    U32 u32Timer;
#endif
    U32 u32RegVal, u32GOP_WR_ACK;

    static U32 u32counter;

#ifdef RED_LION
    down(&gop_sem);
#endif

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    if( _GopInfo[eGOP_Type].bForceWrite)
    {
        u32RegVal = REG(REG_GOP_WR_ACK);
        u32RegVal &= ~(GOP_WR_MASK|GOP_FWR_MASK);
        u32RegVal |= GOP_FWR_MASK; // force write
        REG(REG_GOP_WR_ACK) = u32RegVal;

#ifdef RED_LION
        up(&gop_sem);
#endif

        return;
    }
    else
    {
        u32RegVal = REG(REG_GOP_WR_ACK);
        u32RegVal &= ~(GOP_WR_MASK|GOP_FWR_MASK);
        u32RegVal |= GOP_WR_MASK; // write mask
        REG(REG_GOP_WR_ACK) = u32RegVal;

        __asm(
            "nop;"
            "nop;"
            );

        u32RegVal &= ~GOP_WR_MASK; // write mask
        __asm(
            "nop;"
            "nop;"
            );

        REG(REG_GOP_WR_ACK) = u32RegVal;


        u32counter = 0;

#ifdef RED_LION
        do {
            if (u32counter > GOP_ACK_COUNTER)
            {
                DBG_GOP(printk ("wait ack to 1 time out %d, perform force write to update register \n ",u32counter ));
                u32RegVal = REG(REG_GOP_WR_ACK);
                u32RegVal &= ~(GOP_WR_MASK|GOP_FWR_MASK);
                u32RegVal |= GOP_FWR_MASK; // force write
                REG(REG_GOP_WR_ACK) = u32RegVal;
                break;
            }
            else
            {
                u32RegVal = REG(REG_GOP_WR_ACK);

                switch(eGOP_Type)
                {
                    case E_GOP4G_0:
                        u32GOP_WR_ACK = GOP0_WR_ACK_MASK;
                        break;
                    case E_GOP2G_1:
                        u32GOP_WR_ACK = GOP1_WR_ACK_MASK;
                        break;
                    case E_GOP_DWIN:
                        u32GOP_WR_ACK = GOPD_WR_ACK_MASK;
                        break;
                    case E_GOP1G:
                        u32GOP_WR_ACK = GOPS_WR_ACK_MASK;
                        break;
                    case E_GOP1GX:
                        u32GOP_WR_ACK = GOP1GX_WR_ACK_MASK;
                        break;
                    default:
                        DBG_GOP (printk ("Error GOP type %d",eGOP_Type ));
                        return;
                }

                if ((u32RegVal & u32GOP_WR_ACK) == 0)
                {
                    break;
                }

				msleep(1); // by LGE. jjab.
                u32counter++;
            }
        } while(1);

        up(&gop_sem);
#else
        u32Timer = MsOS_GetSystemTime();
        do {
            if ((MsOS_GetSystemTime()-u32Timer) > 200 || (u32counter > GOP_ACK_COUNTER))//200ms
            {
                DBG_GOP(printk ("wait ack to 1 time out %d\n ",u32counter ));
                break;
            }
            else
            {
                u32RegVal = REG(REG_GOP_WR_ACK);

                switch(eGOP_Type)
                {
                    case E_GOP4G_0:
                        u32GOP_WR_ACK = GOP0_WR_ACK_MASK;
                        break;
                    case E_GOP2G_1:
                        u32GOP_WR_ACK = GOP1_WR_ACK_MASK;
                        break;
                    case E_GOP_DWIN:
                        u32GOP_WR_ACK = GOPD_WR_ACK_MASK;
                        break;
                    case E_GOP1G:
                        u32GOP_WR_ACK = GOPS_WR_ACK_MASK;
                        break;
                    case E_GOP1GX:
                        u32GOP_WR_ACK = GOP1GX_WR_ACK_MASK;
                        break;
                    default:
                        DBG_GOP (printk ("Error GOP type %d",eGOP_Type ));
                        return;
                }
                if ((u32RegVal & u32GOP_WR_ACK) == 0)
                    break;

                if (MsOS_In_Interrupt() == FALSE)
                    MsOS_YieldTask();

                u32counter++;
            }
        } while ( 1 );
#endif

    }

}




//-------------------------------------------------------------------------------------------------
// Select GOP output format is YUV or RGB
// @param eGOP_Type \b IN: GOP Type
// @param bEnable IN: TRUE: YUV output / FALSE: RGB output
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_YUVOut_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U32 u32RegVal;


    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
         DBG_GOP(printk ("Error GOP Type"));
#else
         DBG_GOP(printk ("Error GOP Type"));
#endif
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_SOFT_RESET);
    ((GOP_soft_reset*)&u32RegVal)->YUV_OUT = bEnable;

    REG(REG_GOP_SOFT_RESET) = u32RegVal;

    return TRUE;

}


//-------------------------------------------------------------------------------------------------
// Set a palette entry for Blink, I8(8-bit palette) format.
// @param u8GOP_num \b IN: GOP number: 0 or 1
// @param pPalEntry \b IN: ptr to palette data
// @param u32Index   \b IN: Palette Index,
// @param ePalType  \b IN: palette type (Blink format must set it as E_GOP_PAL_RGB888)
// @return TRUE: sucess / FALSE: fail
// @note: GOP4G_0 & GOP4G_1 (support 256 entries: Index: 0-255)
// @note: GOP1G (support 32 entries: Index: 0~31 )
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Palette_Set( GOP_HW_Type eGOP_Type, GopPaletteEntry *pPalEntry, U32 u32Index)
{
    U32 u32RegVal;
	//GOP_HW_Type eGOP_Bank;

    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
        DBG_GOP(printk ("Error GOP Type"));
#else
        DBG_GOP(printk ("Error GOP Type"));
#endif
        return FALSE;
    }

    //Change GOP Bank
    //_MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type
	_MHal_GOP_Select(E_GOP4G_0, TRUE); // titania's palette is in bank 0
    if ((eGOP_Type == E_GOP1G || eGOP_Type == E_GOP1GX))
    {
        //32~255 error Index for GOP1G.
        if (u32Index > 31)
        {
#ifdef RED_LION
            DBG_GOP(printk ("Error GOP1G Palette Index %d", u32Index));
#else
            DBG_GOP(printk ("Error GOP1G Palette Index %d", u32Index));
#endif
            return FALSE;
        }
    }

    else if(u32Index >= GOP_PALETTE_ENTRY_NUM)
    {
#ifdef RED_LION
        DBG_GOP(printk ("Error GOP4G Palette Index %d", u32Index));
#else
        DBG_GOP(printk ("Error GOP4G Palette Index %d", u32Index));
#endif
        return FALSE;
    }

    // Set palette
    /*
    u32RegVal = 0;
    REG(REG_GOP_PAL_DATA_LO) = LO16BIT(pPalEntry[0].u32Data);

    ((GOP_pal_data_hi*)&u32RegVal)->PAL_DATA_HI = HI16BIT(pPalEntry[0].u32Data);
    REG(REG_GOP_PAL_DATA_HI) = u32RegVal;
    */
    u32RegVal = pPalEntry[0].u32Data ;
    REG(REG_GOP_PAL_DATA_LO) = (u32RegVal&0xFFFF) ;
    REG(REG_GOP_PAL_DATA_HI) = (u32RegVal>>16) ;

    u32RegVal = REG(REG_GOP_PAL_SET);
//	printk("pallet u32Index=%lx data= %lx set %lx\n ",u32Index,pPalEntry[0].u32Data,u32RegVal);
    if (eGOP_Type == E_GOP4G_0)
        ((GOP_pal_set*)&u32RegVal)->PAL_CTL = E_GOP_PAL_4G_RIU;
	else
        ((GOP_pal_set*)&u32RegVal)->PAL_CTL = E_GOP_PAL_2G_RIU;
	//else{
	//	((GOP_pal_set*)&u32RegVal)->PAL_CTL = E_GOP_PAL_GE;
	//	DBG_GOP(printk ("Set Palette Control ERR (GOP type=%x)!!!!", eGOP_Type));
	//}

    ((GOP_pal_set*)&u32RegVal)->PAL_ADDR = u32Index;
	//printk("PAL Control %lx\n",((GOP_pal_set*)&u32RegVal)->PAL_CTL);
    // Write PAL_WRITE
    ((GOP_pal_set*)&u32RegVal)->PAL_WRITE = 1;

#if 0 // TODO: alan has NOT ported it. The register is removed and only ARGB8888 is accepted.
    ((GOP_pal_set*)&u32RegVal)->PAL_TYPE = (U32)ePalType;
#endif

    REG(REG_GOP_PAL_SET) = u32RegVal;

    // Clear PAL_WRITE by CPU
    ((GOP_pal_set*)&u32RegVal)->PAL_WRITE = 0;
    REG(REG_GOP_PAL_SET) = u32RegVal;

    return TRUE;
}

static B16 _MHal_GOP_Palette_Toggle_Set( GOP_HW_Type eGOP_Type )
{
    U32 u32RegVal;

    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
        DBG_GOP(printk ("Error GOP Type"));
#else
        DBG_GOP(printk ("Error GOP Type"));
#endif
        return FALSE;
    }

    //Change GOP Bank
	_MHal_GOP_Select(E_GOP2G_1, TRUE);

    u32RegVal = REG(REG_GOP_PAL_SET);

    if (eGOP_Type == E_GOP4G_0)
        ((GOP_pal_set*)&u32RegVal)->PAL_CTL = E_GOP_PAL_2G_RIU;
	else
        ((GOP_pal_set*)&u32RegVal)->PAL_CTL = E_GOP_PAL_4G_RIU;

    REG(REG_GOP_PAL_SET) = u32RegVal;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Read a palette entry for Blink, I8(8-bit palette) format.
// @param u8GOP_num \b IN: GOP number: 0 or 1
// @param pPalEntry \b IN: ptr to palette data
// @param u32Index   \b IN: Palette Index,
// @param ePalType  \b IN: palette type (Blink format must set it as E_GOP_PAL_RGB888)
// @return TRUE: sucess / FALSE: fail
// @note: GOP4G_0 & GOP4G_1 (support 256 entries: Index: 0-255)
// @note: GOP1G (support 32 entries: Index: 0~31 )
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Palette_Read( GOP_HW_Type eGOP_Type, GopPaletteEntry *pPalEntry, U32 u32Index)
{
    U32 u32RegVal;
    U32 u32oldRegVal;
	//GOP_HW_Type eGOP_Bank;

    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
        DBG_GOP(printk ("Error GOP Type"));
#else
        DBG_GOP(printk ("Error GOP Type"));
#endif
        return FALSE;
    }

    //Change GOP Bank
    //_MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type
	_MHal_GOP_Select(E_GOP4G_0, TRUE); // titania's palette is in bank 0
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        //32~255 error Index for GOP1G.
        if (u32Index > 31)
        {
#ifdef RED_LION
            DBG_GOP(printk ("Error GOP1G Palette Index %d", u32Index));
#else
            DBG_GOP(printk ("Error GOP1G Palette Index %d", u32Index));
#endif
            return FALSE;
        }
    }

    else if(u32Index >= GOP_PALETTE_ENTRY_NUM)
    {
#ifdef RED_LION
        DBG_GOP(printk ("Error GOP4G Palette Index %d", u32Index));
#else
        DBG_GOP(printk ("Error GOP4G Palette Index %d", u32Index));
#endif
        return FALSE;
    }

    u32RegVal = REG(REG_GOP_PAL_SET);
    u32oldRegVal = u32RegVal;

//	printk("pallet u32Index=%lx data= %lx set %lx\n ",u32Index,pPalEntry[0].u32Data,u32RegVal);

    ((GOP_pal_set*)&u32RegVal)->PAL_CTL = E_GOP_PAL_4G_RIU;
    ((GOP_pal_set*)&u32RegVal)->PAL_ADDR = u32Index;
	//printk("PAL Control %lx\n",((GOP_pal_set*)&u32RegVal)->PAL_CTL);
    // Write PAL_WRITE
    ((GOP_pal_set*)&u32RegVal)->PAL_READ = 1;


    REG(REG_GOP_PAL_SET) = u32RegVal;

    // Clear PAL_WRITE by CPU
    ((GOP_pal_set*)&u32RegVal)->PAL_READ = 0;
    REG(REG_GOP_PAL_SET) = u32RegVal;

    u32RegVal = REG(REG_GOP_PAL_DATA_HI);// = (u32RegVal&0xFFFF) ;
    pPalEntry[0].u32Data = (u32RegVal<< 16) |(REG(REG_GOP_PAL_DATA_LO) &0xFFFF);

    REG(REG_GOP_PAL_SET) = u32oldRegVal;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Get GWIN ID from GOP type and the specified window ID
// @param GOP_HW_Type \b IN: GOP HW Type
// @param u8Wid \b IN: GWin ID (0~3)
// @return TRUE:Success /FALSE: Fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_Get_GWinID(GOP_HW_Type eGOP_Type, U8 *pu8Wid, U32 *pu32GwinID)
{
    // When GOP HW type is E_GOP1G, Only support 1 GWIN and its u8Wid = 2
    if (eGOP_Type == E_GOP1G)
    {

/* 		//@FIXME:In E_GOP1G, some part of the code already re-mapping the *pu8Wid to 2, so it will fail here
        //It needs to fix in the future;
		if (MAX_GOP2_GWIN_SUPPORT <= *pu8Wid)
        {
            return FALSE;
        }
 */
        *pu32GwinID = TOTAL_GWIN_SUPPORT-2;
        *pu8Wid = 2;
    }
    else if (E_GOP1GX == eGOP_Type)
    {
        *pu32GwinID = TOTAL_GWIN_SUPPORT-1;
        *pu8Wid = 3;
    }
    else if (E_GOP2G_1 == eGOP_Type)
    {
        if (MAX_GOP1_GWIN_SUPPORT <= *pu8Wid)
        {
            return FALSE;
        }
         *pu32GwinID = MAX_GOP0_GWIN_SUPPORT + *pu8Wid;
        //*pu32GwinID = *pu8Wid;
    }
    else if (E_GOP4G_0 == eGOP_Type)
    {
        if (MAX_GOP0_GWIN_SUPPORT <= *pu8Wid)
        {
            return FALSE;
        }
        *pu32GwinID = *pu8Wid;
    }
    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Set GWIN parameters to registers
// @param GOP_HW_Type \b IN: GOP HW Type
// @param u8Wid \b IN: GWin ID (0~3)
// @return TRUE:Success /FALSE: Fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_ErrCheck(GOP_HW_Type eGOP_Type, U8 u8Wid)
{
    U32 u32GwinID=0;

    if (eGOP_Type > E_GOP1GX)
    {
        return FALSE;
    }

    if (!_MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID))
    {
        //printk ("\nGet GWIN ID Fail ..\n");
        return FALSE;
    }

#if (GOP_RESOURCE_CONTROL == 1)
    //printk("~ 2[ErrCheck] (0x%X  0x%X  0x%X  0x%X)\n", eGOP_Type, u8Wid, u32GwinID, _GWinInfo[u32GwinID].bUsed);
    if (!_GWinInfo[u32GwinID].bUsed)
    {
        //printk ("\nGOP resource occupied: GOP %d GWIN %d\n", eGOP_Type, u8Wid);
#ifdef RED_LION
        DBG_GOP(printk ("GOP resource occupied: GOP %d wid %d GWIN %d\n", eGOP_Type, u8Wid, u32GwinID));
#else
        DBG_GOP(printk ("GOP resource occupied: GOP %d wid %d GWIN %d\n", eGOP_Type, u8Wid, u32GwinID));
#endif
        return FALSE;
    }
#endif

    return TRUE;

}

static void _SetHVMirrorWinPos(GOP_HW_Type eGOP_Type, MS_U8 u8GwinID, HMirror_Opt opt, GopGwinInfo* pGWINInfo)
{
    U32 u32StrWinHSize, u32StrWinVSize;
    S32 s32NewHStr, s32NewVStr;

    _MHal_GOP_Select(eGOP_Type, TRUE);
    u32StrWinHSize = REG(REG_GOP_STRCH_HSIZE)<<1;
    u32StrWinVSize = REG(REG_GOP_STRCH_VSIZE);

    switch (opt)
    {
        case E_Set_HPos:
        {
            s32NewHStr = u32StrWinHSize - pGWINInfo->u32DispHStart - pGWINInfo->u32DispWidth;
            if (s32NewHStr<0)
                pGWINInfo->u32DispHStart = 0;
            else
                pGWINInfo->u32DispHStart = s32NewHStr;
            break;
        }
        case E_Set_VPos:
        {
            s32NewVStr = u32StrWinVSize - pGWINInfo->u32DispVStart - pGWINInfo->u32DispHeight;
            if (s32NewVStr<0)
                pGWINInfo->u32DispVStart = 0;
            else
                pGWINInfo->u32DispVStart = s32NewVStr;
            break;
        }
        default:
        {
            //ASSERT(0);
            break;
        }
    }
}

static U8 _MapGwinIdByGopType(GOP_HW_Type eGOP_Type, U8 u8Wid)
{
    U8 u8RetGwinId=0;
    switch (eGOP_Type)
    {
        case E_GOP4G_0:
            u8RetGwinId = u8Wid;
            break;
        case E_GOP2G_1:
            u8RetGwinId = u8Wid+MAX_GOP0_GWIN_SUPPORT;
            break;
        case E_GOP1G:
            u8RetGwinId = u8Wid+MAX_GOP0_GWIN_SUPPORT+MAX_GOP1_GWIN_SUPPORT;
            break;
        case E_GOP1GX:
            u8RetGwinId = u8Wid+MAX_GOP0_GWIN_SUPPORT+MAX_GOP1_GWIN_SUPPORT+MAX_GOP2_GWIN_SUPPORT;
            break;
        default:
            DBG_GOP(printk("\n!!!!Error in [%s] not support this GOP type!!!!", __FUNCTION__));
            break;
    }
    return u8RetGwinId;
}

static B16 _MHal_GOP_GWIN_MirrorSetting(GOP_HW_Type eGOP_Type, U8 u8Wid, GopGwinInfo *pGInfo)
{
    U32 u32RegVal, u32GopOpPath=2;
    
    sGOP_Mirror.sMirrorInfo[_MapGwinIdByGopType(eGOP_Type, u8Wid)].u32NonMirrorFBAdr = pGInfo->u32DRAMRBlkStart;
    u32RegVal = REG(REG_GOP_DEST);
	
    if ( sGOP_Mirror.bHMirror )
    {
        if ((((GOP_dest*)&u32RegVal)->GOP_DEST)==u32GopOpPath)
        {
	        /*save gwin postion before h mirror setting*/
	        sGOP_Mirror.sMirrorInfo[_MapGwinIdByGopType(eGOP_Type, u8Wid)].u16NonMirrorHStr = pGInfo->u32DispHStart;
	        if (eGOP_Type==E_GOP4G_0)
	        {
	            sGOP_Mirror.sMirrorInfo[_MapGwinIdByGopType(eGOP_Type, u8Wid)].u16NonMirrorGOP0WinX = pGInfo->u32SrcHStart;
	            pGInfo->u32SrcHStart = 0 ;
	        }

	        MHal_GOP_HMirror_Enable(eGOP_Type, TRUE);
	        sGOP_Mirror.u16GOP_HMirror_HPos |= (U16)(1<<(_MapGwinIdByGopType(eGOP_Type, u8Wid)));
	        if (eGOP_Type!=E_GOP4G_0)
	        {
	            pGInfo->u32DRAMRBlkStart = pGInfo->u32DRAMRBlkStart + (U32)(_CalculatePitch(pGInfo->u32DRAMRBlkHSize, pGInfo->eColorType)) - 8;
	            sGOP_Mirror.u16GOP_NoRBLKHMirror_Adr |= (U16)(1<<(_MapGwinIdByGopType(eGOP_Type, u8Wid)));
	        }
        }
        else
        {
            _MHal_GOP_HMirror_Enable(eGOP_Type, DISABLE);
        }
    }

    if ( sGOP_Mirror.bVMirror )
    {
        if ((((GOP_dest*)&u32RegVal)->GOP_DEST)==u32GopOpPath)
        {
	        /*save gwin postion before V mirror setting*/
	        sGOP_Mirror.sMirrorInfo[eGOP_Type].u16NonMirrorVStr = pGInfo->u32DispVStart;
	        if (eGOP_Type==E_GOP4G_0)
	        {
	            sGOP_Mirror.sMirrorInfo[eGOP_Type].u16NonMirrorGOP0WinY = pGInfo->u32SrcVStart;
	            pGInfo->u32SrcVStart = (pGInfo->u32DispVStart+pGInfo->u32DispHeight-pGInfo->u32DispVStart-1);
	        }

	        MHal_GOP_VMirror_Enable(eGOP_Type, TRUE);
	        sGOP_Mirror.u16GOP_VMirror_VPos |= (U16)(1<<(_MapGwinIdByGopType(eGOP_Type, u8Wid)));

	        if (eGOP_Type==E_GOP4G_0)
	        {
	            sGOP_Mirror.u16GOP_HasRBLKVMirror_Adr|= (U16)(1<<(_MapGwinIdByGopType(eGOP_Type, u8Wid)));
	        }
	        else
	        {
	            pGInfo->u32DRAMRBlkStart += (_CalculatePitch(pGInfo->u32DRAMRBlkHSize, pGInfo->eColorType)*(pGInfo->u32DispHeight-1));
	            sGOP_Mirror.u16GOP_NoRBLKVMirror_Adr |= (U16)(1<<(_MapGwinIdByGopType(eGOP_Type, u8Wid)));
	        }
        }
        else
        {
            _MHal_GOP_VMirror_Enable(eGOP_Type, DISABLE);
        }
    }
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Set GWIN parameters to registers
// @param GOP_HW_Type \b IN: GOP HW Type
// @param u8Wid \b IN: GWin ID (0~3)
// @param pInfo  \b IN: ptr to GopGwinInfo
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_WinInfo_Set(GOP_HW_Type eGOP_Type, U8 u8Wid, GopGwinInfo* pInfo)
{
    U32 u32RegVal;
    U32 u32Tmp;
//    U16 bHDup;
   /* if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        {
        printk("\n gop gwin err check 2 ");
        return FALSE;
        }*/

    // When GOP HW type is E_GOP1G, u8Wid value is don't care
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, TRUE);
    if ( sGOP_Mirror.bHMirror || sGOP_Mirror.bVMirror )
    {
        _MHal_GOP_GWIN_MirrorSetting(eGOP_Type, u8Wid, pInfo);
    }


#if 0 // TODO: alan has NOT ported it. The register is removed and can use stretch to implement

    if (u32RegVal & GOP_GWIN_H_DUP)
    {
        bHDup = TRUE;
    }
    else
    {
        bHDup = FALSE;
    }
#endif

    u32RegVal = REG(REG_GOP_SOFT_RESET);

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);

    //Display Position

    if (u32RegVal & GOP_GWIN_PROG_MD)   // progressive output
    {
        REG(REG_GOP_GWIN_VSTR(u8Wid)) = pInfo->u32DispVStart + _GopInfo[eGOP_Type].u8Y_Base; //in pixel
        REG(REG_GOP_GWIN_VEND(u8Wid)) = pInfo->u32DispVStart + _GopInfo[eGOP_Type].u8Y_Base + pInfo->u32DispHeight;//in pixel
    }
    else //Interlace output
    {
        REG(REG_GOP_GWIN_VSTR(u8Wid)) = (pInfo->u32DispVStart/2)+ _GopInfo[eGOP_Type].u8Y_Base ; //in pixel
        REG(REG_GOP_GWIN_VEND(u8Wid)) = ((pInfo->u32DispVStart/2)+ _GopInfo[eGOP_Type].u8Y_Base) + (pInfo->u32DispHeight/2);//in pixel
    }

    REG(REG_GOP_GWIN_HSTR(u8Wid)) = (pInfo->u32DispHStart + _GopInfo[eGOP_Type].u8X_Base) * pInfo->u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment

    REG(REG_GOP_GWIN_HEND(u8Wid)) = (pInfo->u32DispHStart + pInfo->u32DispWidth + _GopInfo[eGOP_Type].u8X_Base) * pInfo->u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment

#if 0 // TODO: alan has NOT ported it. The register is removed and can use stretch to implement

    if (bHDup)
        REG(REG_GOP_GWIN_HSIZE(u8Wid)) = (pInfo->u32DispWidth)/2 * pInfo->u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment
    else
        REG(REG_GOP_GWIN_HSIZE(u8Wid)) = (pInfo->u32DispWidth) * pInfo->u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment
#endif

    //Offset to RBLK
    u32RegVal = pInfo->u32SrcVStart * pInfo->u32DRAMRBlkHSize * pInfo->u32ColorDepth >> GOP_ALIGN_RSHIFT;
    REG(REG_GOP_DRAM_GWIN_VSTR_LO(u8Wid)) = LO16BIT(u32RegVal);
    REG(REG_GOP_DRAM_GWIN_VSTR_HI(u8Wid)) = HI16BIT(u32RegVal);
    u32RegVal = pInfo->u32SrcHStart * pInfo->u32ColorDepth >> GOP_ALIGN_RSHIFT;
    REG(REG_GOP_DRAM_GWIN_HSTR(u8Wid)) = LO16BIT(u32RegVal);

    //RBLK
    u32Tmp = pInfo->u32DRAMRBlkStart >> GOP_ALIGN_RSHIFT;
    //printk("~~( GOP, shif4 ) = (0x%08X, 0x%08X) \n", pInfo->u32DRAMRBlkStart, u32Tmp);
    REG(REG_GOP_DRAM_RBLK_STR_LO(u8Wid)) = LO16BIT(u32Tmp);
    //printk("~~( Lo ) = (0x%08X) \n", LO16BIT(u32Tmp));
    u32RegVal = REG(REG_GOP_DRAM_RBLK_STR_HI(u8Wid));
    ((GOP_dram_rblk_str_hi*)&u32RegVal)->DRAM_RBLK_STR_HI = HI16BIT(u32Tmp) >> GOP_DRAM_RBLK_STR_HI_SHIFT;
    REG(REG_GOP_DRAM_RBLK_STR_HI(u8Wid)) = u32RegVal;
    //printk("~~( Hi ) = (0x%08X) \n", u32RegVal);

    u32RegVal = pInfo->u32DRAMRBlkHSize * pInfo->u32DRAMRBlkVSize * pInfo->u32ColorDepth >> GOP_ALIGN_RSHIFT;
    REG(REG_GOP_DRAM_RBLK_SIZE_LO(u8Wid)) = LO16BIT(u32RegVal);
    REG(REG_GOP_DRAM_RBLK_SIZE_HI(u8Wid)) = HI16BIT(u32RegVal);
    u32RegVal = pInfo->u32DRAMRBlkHSize * pInfo->u32ColorDepth >> GOP_ALIGN_RSHIFT;
    REG(REG_GOP_DRAM_RBLK_HSIZE(u8Wid)) = LO16BIT(u32RegVal);

    //Color type
    u32RegVal = REG(REG_GOP_GWIN_SET(u8Wid));

    ((GOP_gwin_set*)&u32RegVal)->GWIN_DTYPE = (U32)pInfo->eColorType;

    REG(REG_GOP_GWIN_SET(u8Wid)) = u32RegVal;

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
/// Enable/Disable a GWIN
/// @param eGOP_Type \b IN: GOP HW type
/// @param u8Wid  \b IN: GWIN ID
/// @param bEnable \b IN: TRUE/FALSE : enable/disable
/// @return TRUE: sucess / FALSE: fail
/// @note If set bEnable to FALSE, this function can only disable Gwin instead of destory Gwin.
/// it won't disable GWIN features (scroll mode, blink, transparent, alpha blending, duplicate)
/// - Example: GOP0:Gwin0 have constant alpha, You can call MHal_GOP_GWIN_Enable(0, 0, FALSE) to disable gwin,
/// than call MHal_GOP_GWIN_Enable (0, 0, TRUE) to enable gwin, the constant alpha feature is still existed.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable)
{
    U32 u32RegVal, u32RegVal0;
    U32 u32GwinID=0;

#if 0
    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
    {
        printk("_MHal_GOP_GWIN_Enable fail!!! [%d], [%d]\n", eGOP_Type, u8Wid);
        return FALSE;
    }
#endif
    if (eGOP_Type > E_GOP1GX)
    {
        printk("\n [%s] gop type error!!!", __FUNCTION__);
        return FALSE;
    }
    if (!_MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID))
    {
        printk ("\n [%s]Get GWIN ID Fail ..\n", __FUNCTION__);
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, FALSE);

    if((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
        u32RegVal = REG(REG_GOP_GWIN_SET(1));
    else
    u32RegVal = REG(REG_GOP_GWIN_SET(u8Wid));

    ((GOP_gwin_set*)&u32RegVal)->GWIN_EN = bEnable;

    if(bEnable)
    {
        //clear software reset
        _MHal_GOP_Select(eGOP_Type, TRUE);
        u32RegVal0 = REG(REG_GOP_SOFT_RESET);
        ((GOP_soft_reset*)&u32RegVal0)->SOFT_RESET = 0;
        _GWinInfo[u32GwinID].u32Enable = TRUE;
        //if(eGOP_Type == E_GOP1G)
        //    _MHal_GOP_Mux_Select(E_GOP1G, E_TCON_OSD_0);
    }
    else
    {
        _GWinInfo[u32GwinID].u32Enable = FALSE;
        //if(eGOP_Type == E_GOP1G)  // set back mux1 get gop2g
        //    _MHal_GOP_Mux_Select(E_GOP2G_1, E_TCON_OSD_0);
    }
    _MHal_GOP_Select(eGOP_Type, FALSE);

    if((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
        REG(REG_GOP_GWIN_SET(1)) = u32RegVal;
    else
    REG(REG_GOP_GWIN_SET(u8Wid)) = u32RegVal;

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Set stretch window info parameters to registers
// @param eGOP_Type \b IN: GOP HW Type
// @param u32DispX \b IN: disp Xstart (pixel unit)
// @param u32DispY \b IN: disp Ystart (pixel unit)
// @param u32Width \b IN: disp width (pixel unit)
// @param u32Height \b IN: disp height (pixel unit)
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------

B16 _MHal_GOP_StretchWin_WinInfo_Set(GOP_HW_Type eGOP_Type,
                                                     U32 u32DispX, U32 u32DispY,
                                                     U32 u32Width, U32 u32Height)

{
    _MHal_GOP_Select(eGOP_Type, TRUE);

	#if SUBTITLE_IN_IP
	if(eGOP_Type == E_GOP2G_1)
	{
		REG(REG_GOP_STRCH_HSTART) = u32DispX-0x24;
		REG(REG_GOP_STRCH_VSTART) = u32DispY+0xA;
	}
	else
    #endif
	{
    REG(REG_GOP_STRCH_HSTART) = u32DispX;
    REG(REG_GOP_STRCH_VSTART) = u32DispY;
	}

    REG(REG_GOP_STRCH_HSIZE) = u32Width>>1;
    REG(REG_GOP_STRCH_VSIZE) = u32Height;

    REG(REG_GOP_RDMA_HT) = (u32Width>>1) + 1;
	
    return TRUE;
}

void _MHal_GOP_Set_Stretch_Para(GOP_HW_Type eGOP_Type){
    U32 tmp, bank ;

    if( E_GOP4G_0==eGOP_Type )
        bank = 2 ;
    else if( E_GOP4G_0==eGOP_Type )
        bank = 5 ;
    else{
        printk( "N/G GOP id!\n" ) ;
        return ;
    }

    // switch bank
    tmp = REG( REG_GOP_BASE+(0x7F<<2) ) ;
    tmp &= ~0x7 ;
    tmp |= bank ;
    REG( REG_GOP_BASE+(0x7F<<2) ) = bank ;

    // parameters (no peak)
    REG( REG_GOP_BASE+(0x00<<2) ) = 0x0001 ;
    REG( REG_GOP_BASE+(0x01<<2) ) = 0x1001 ;
    REG( REG_GOP_BASE+(0x02<<2) ) = 0x0000 ;
    REG( REG_GOP_BASE+(0x03<<2) ) = 0x0002 ;
    REG( REG_GOP_BASE+(0x04<<2) ) = 0x0F04 ;
    REG( REG_GOP_BASE+(0x05<<2) ) = 0x0100 ;
    REG( REG_GOP_BASE+(0x06<<2) ) = 0x0002 ;
    REG( REG_GOP_BASE+(0x07<<2) ) = 0x0E05 ;

    REG( REG_GOP_BASE+(0x08<<2) ) = 0x0100 ;
    REG( REG_GOP_BASE+(0x09<<2) ) = 0x0002 ;
    REG( REG_GOP_BASE+(0x0A<<2) ) = 0x0C08 ;
    REG( REG_GOP_BASE+(0x0B<<2) ) = 0x0200 ;
    REG( REG_GOP_BASE+(0x0C<<2) ) = 0x0001 ;
    REG( REG_GOP_BASE+(0x0D<<2) ) = 0x1001 ;
    REG( REG_GOP_BASE+(0x0E<<2) ) = 0x0000 ;
    REG( REG_GOP_BASE+(0x0F<<2) ) = 0x0002 ;

    REG( REG_GOP_BASE+(0x10<<2) ) = 0x0F04 ;
    REG( REG_GOP_BASE+(0x11<<2) ) = 0x0100 ;
    REG( REG_GOP_BASE+(0x12<<2) ) = 0x0002 ;
    REG( REG_GOP_BASE+(0x13<<2) ) = 0x0E05 ;
    REG( REG_GOP_BASE+(0x14<<2) ) = 0x0100 ;
    REG( REG_GOP_BASE+(0x15<<2) ) = 0x0002 ;
    REG( REG_GOP_BASE+(0x16<<2) ) = 0x0C08 ;
    REG( REG_GOP_BASE+(0x17<<2) ) = 0x0200 ;

    REG( REG_GOP_BASE+(0x30<<2) ) = 0x0000 ;

}


B16 _MHal_GOP_StretchWin_SetRatio(GOP_HW_Type eGOP_Type, U32 u32H_Ratio, U32 u32V_Ratio)
{
    _MHal_GOP_Select(eGOP_Type, TRUE);
    REG(REG_GOP_STRCH_HRATIO) = u32H_Ratio;
    REG(REG_GOP_STRCH_VRATIO) = u32V_Ratio;

    if( E_GOP4G_0==eGOP_Type ){
        _MHal_GOP_Set_Stretch_Para( eGOP_Type ) ;
    }

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Init all GWins
// @param  eGOP_Type \b IN: GOP HW type select
// @param  u8Wid    \b IN: index of GWIN [ 0 .. (@ref MAX_GWIN_SUPPORT - 1) ]
// @return  None
//-------------------------------------------------------------------------------------------------
static void _MHal_GOP_GWIN_Init(GOP_HW_Type eGOP_Type, U8 u8Wid )
{
    U32 u32GwinID=0;

/*
    if ((eGOP_Type > E_GOP1G) || (u8Wid >= MAX_GWIN_SUPPORT))
    {
        return;
    }
*/

    if (!_MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID))
        return;

     memset(&_GWinInfo[u32GwinID], 0x0, sizeof (GopGwinInfo));
}

//-------------------------------------------------------------------------------------------------
// Enable/Disable H / V scroll
// @param eGOP_Type     \b IN: GOP HW type select
// @param u8Wid        \b IN: GWin ID
// @param eScrollType   \b IN: scroll type
// @param u16ScrollStep \b IN: scroll Step
// @param bEnable       \b IN: TRUE/FALSE
// @return TRUE: sucess / FALSE: fail
// @note
// - When eScrollType is GOP_SCROLL_NONE, ignore the parameter bEnable.
// - Horiziontal scroll unit: 16/ColorDepth pixel; Vertical scroll unit: 1 pixel;
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_Scroll_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, GopScrollType eScrollType, U8 u16ScrollStep, U16 bEnable)
{
    U32 u32ScrollLen = 0, u32ScrollEn;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    // When GOP HW type is E_GOP1G, only support 1 GWIN (GWIN ID = 3)
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    //Change GOP Bank 1
    _MHal_GOP_Select(eGOP_Type, FALSE);

    //Set Initial Scroll Step
    REG(REG_GOP_GWIN_SCROLL_LEN(u8Wid)) = 0;


    u32ScrollEn = REG(REG_GOP_GWIN_SET(u8Wid));

    switch (eScrollType)
    {
        case GOP_SCROLL_LEFT:
            ((GOP_gwin_scroll_len*)&u32ScrollLen)->SCROLL_LEN = u16ScrollStep;

            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_VROLL = 0;
            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_HROLL = 1;
            break;

        case GOP_SCROLL_RIGHT:
            ((GOP_gwin_scroll_len*)&u32ScrollLen)->SCROLL_LEN = 0 - u16ScrollStep;

            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_VROLL = 0;
            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_HROLL = 1;
            break;

        case GOP_SCROLL_UP:
            ((GOP_gwin_scroll_len*)&u32ScrollLen)->SCROLL_LEN = REG(REG_GOP_DRAM_RBLK_HSIZE(u8Wid)) * u16ScrollStep;

            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_VROLL = 1;
            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_HROLL = 0;
            break;

        case GOP_SCROLL_DOWN:
            ((GOP_gwin_scroll_len*)&u32ScrollLen)->SCROLL_LEN = 0 - (REG(REG_GOP_DRAM_RBLK_HSIZE(u8Wid)) * u16ScrollStep);

            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_VROLL = 1;
            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_HROLL = 0;
            break;

        case GOP_SCROLL_NONE:
            ((GOP_gwin_scroll_len*)&u32ScrollLen)->SCROLL_LEN = 0;

            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_VROLL = 0;
            ((GOP_gwin_set*)&u32ScrollEn)->GWIN_HROLL = 0;
            break;

        default:
            break;
    }

    REG(REG_GOP_GWIN_SCROLL_LEN(u8Wid)) = u32ScrollLen;

    REG(REG_GOP_GWIN_SET(u8Wid)) = u32ScrollEn;

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Set GWIN Alpha Blending
// @param eGOP_Type\b IN: GOP HW type select
// @param u8Wid   \b IN: GWin ID: 0~3
// @param bEnable  \b IN: TRUE/FALSE Enable/Disable Per Pixel Alpha Blending
// @param u32Alpha \b IN: Constant Alpha Value (Range: 0~63; 63:opaque; 0: transparent)
// @return TRUE: sucess / FALSE: fail
// @note 1: HW limitation
// - GOP combine with IP output have 8 Level Alpha
// - GOP combine with OP output have 64 Level Alpha
// @note 2: if you set Gobal Alpha Value, the Per Pixel Alpha Blending won't be enabled.
// - Example 1: MDrv_GOP_GWIN_Blending_Set(0, 0, TRUE, 30) -- Enable Constant Alpha, won't enable Per Pixel Alpha
// - Example 2: MDrv_GOP_GWIN_Blending_Set(0, 0, TRUE, NULL) -- Enable Per Pixel Alpha
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_Blending_Set(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable, U32 u32Alpha)
{
    U32 u32RegVal;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    // When GOP HW type is E_GOP1G, only support 1 GWIN (GWIN ID = 3)
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);

    u32RegVal = REG(REG_GOP_GWIN_SET(u8Wid));
    ((GOP_gwin_set*)&u32RegVal)->GWIN_ALPHA = u32Alpha;
    ((GOP_gwin_set*)&u32RegVal)->GWIN_ALPHA_EN = bEnable;

    REG(REG_GOP_GWIN_SET(u8Wid)) = u32RegVal;

    return TRUE;
}



//-------------------------------------------------------------------------------------------------
// Set Scroll auto stop Horizontal offset
// @param  eGOP_Type \b IN: GOP HW type select
// @param u8Wid             \b IN: GWin ID
// @param u32ScrollAutoHStop \b IN: scorll auto stop H offset, unit: (16/ColorDepth) pixels.
// @return TRUE: sucess / FALSE: fail
// @note In HW scroll mode. HW will stop scroll based on specified RBLK H offset .
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_Scroll_AutoStop_HSet(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32ScrollAutoHStop)
{
    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    // When GOP HW type is E_GOP1G, only support 1 GWIN (GWIN ID = 3)
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);

    REG(REG_GOP_GWIN_HSTOP(u8Wid)) = u32ScrollAutoHStop;

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Set Scroll auto stop Verticall offset
// @param  eGOP_Type         \b IN: GOP HW type select
// @param u8Wid             \b IN: GWin ID
// @param u32ScrollAutoVStop \b IN: scorll auto stop V offset (pixel unit)
// @return TRUE: sucess / FALSE: fail
// @note In HW scroll mode. HW will stop scroll based on specified RBLK V offset .
//
// @image html Auto_Stop_Example.JPG "Example of auto stop"
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_Scroll_AutoStop_VSet(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32ScrollAutoVStop)
{
    U32 u32RegVal, u32RBlkHSize;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    // When GOP HW type is E_GOP1G, only support 1 GWIN (GWIN ID = 3)
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);

    u32RBlkHSize = REG(REG_GOP_DRAM_RBLK_HSIZE(u8Wid));
    u32RegVal = u32ScrollAutoVStop*u32RBlkHSize;

    REG(REG_GOP_GWIN_VSTOP_LO(u8Wid)) = LO16BIT(u32RegVal) ;

    ((GOP_gwin_vstop_hi*)&u32RegVal)->GWIN_VSTOP_HI = HI16BIT(u32RegVal);
    REG(REG_GOP_GWIN_VSTOP_HI(u8Wid)) = u32RegVal;

    return TRUE;
}



//-------------------------------------------------------------------------------------------------
// Enable/Disable H / V  auto stop mode
// @param  eGOP_Type      \b IN: GOP HW type select
// @param u8Wid          \b IN: GWin ID
// @param eScrollStopType \b IN: Scroll auto stop type.
// @param bEnable         \b IN: TRUE/FALSE
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_Scroll_AutoStop_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, GopScrollAutoStopType eScrollStopType, U16 bEnable)
{
    U32 u32RegVal;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    // When GOP HW type is E_GOP1G, only support 1 GWIN (GWIN ID = 3)
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);

    u32RegVal = REG(REG_GOP_GWIN_SET(u8Wid));

        switch(eScrollStopType)
        {
            case GOP_SCROLL_AUTOSTOP_VR:
            case GOP_SCROLL_AUTOSTOP_HR:
            ((GOP_gwin_set *)&u32RegVal)->GWIN_RSTOP = bEnable;
                break;
            case GOP_SCROLL_AUTOSTOP_NONE:
            ((GOP_gwin_set *)&u32RegVal)->GWIN_RSTOP = FALSE;
                break;
#if 0   // NOTE: Titania doesn't allow to scroll in H & V at the same time
            case GOP_SCROLL_AUTOSTOP_ALL:
            ((GOP_gwin_set *)&u32RegVal)->GWIN_RSTOP = TRUE;
        break;
#endif
    }

    REG(REG_GOP_GWIN_SET(u8Wid)) = u32RegVal;

    return TRUE;
}

#if 0 // TODO: alan has NOT ported it, Justin says TWin is removed

//-------------------------------------------------------------------------------------------------
// GOP Transparent Window set
// @param eGOP_Type      \b IN: GOP HW type select
// @param pTwinInfo      \b IN: GOP HW TWIN Info
// @return TRUE: sucess / FALSE: fail
// @note: E_GOP1G can't support TWIN
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_TWIN_Set(GOP_HW_Type eGOP_Type, GopTwinInfo* pTwinInfo)
{
    if (eGOP_Type > E_GOP4G_1)
        return FALSE;

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type);

    REG(REG_GOP_TWIN_VSTAR) = pTwinInfo->u32DispVStart + _GopInfo[eGOP_Type].u8Y_Base; //Pix

    REG(REG_GOP_TWIN_HSTAR) = pTwinInfo->u32DispHStart + _GopInfo[eGOP_Type].u8X_Base; //Pix

    REG(REG_GOP_TWIN_VEND)  = pTwinInfo->u32DispVStart + pTwinInfo->u32DispWidth + _GopInfo[eGOP_Type].u8Y_Base;; //Pix

    REG(REG_GOP_TWIN_HEND)  = pTwinInfo->u32DispHStart + pTwinInfo->u32DispHeight + _GopInfo[eGOP_Type].u8X_Base;; //Pix


    return TRUE;

}

//-------------------------------------------------------------------------------------------------
// GOP Transparent Window set
// @param eGOP_Type      \b IN: GOP HW type select
// @param bEnable        \b IN: TRUE:Enable Twin / FALSE:Disable Twin
// @return TRUE: sucess / FALSE: fail
// @note: E_GOP1G can't support TWIN
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_TWIN_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U32 u32RegVal;

    if (eGOP_Type > E_GOP4G_1)
        return FALSE;

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type);

    u32RegVal = REG(REG_GOP_DEST);

    ((GOP_dest*)&u32RegVal)->TWIN_EN = bEnable;

    REG(REG_GOP_DEST) = u32RegVal;

    return TRUE;

}
#endif

//-------------------------------------------------------------------------------------------------
// GWIN FADE INIT
// @param eGOP_Type      \b IN: GOP HW type select
// @param u8Wid         \b IN: GWIN ID
// @param u8Rate         \b IN: FADE Rate, RANGE 0~15 (unit: 1 Vsync Time)
// @param eFADE_Type     \b IN: FADE Type
// @return TRUE: sucess / FALSE: fail
// @note: Must Set FADE init Fun, Then Set FADE Trigger Fun Each times.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_FADE_Init(GOP_HW_Type eGOP_Type, U8 u8Wid, U8 u8Rate, GOP_FADE_Type eFADE_Type)
{
    U32 u32CtrlReg;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    // When GOP HW type is E_GOP1G, only support 1 GWIN (GWIN ID = 3)
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);

    u32CtrlReg = REG(REG_GOP_GWIN_FADE_SET(u8Wid));

    ((GOP_gwin_fade_ctrl *)&u32CtrlReg)->GWIN_FADE_INIT = 0;
    ((GOP_gwin_fade_ctrl *)&u32CtrlReg)->GWIN_FADE_EN   = 1;
    ((GOP_gwin_fade_ctrl *)&u32CtrlReg)->GWIN_FADE_MODE = eFADE_Type;
    ((GOP_gwin_fade_ctrl *)&u32CtrlReg)->GWIN_FADE_RATE = u8Rate;


    REG(REG_GOP_GWIN_FADE_SET(u8Wid)) = u32CtrlReg;

    return TRUE;

}

//-------------------------------------------------------------------------------------------------
// Trigger FADE Function.
// @param eGOP_Type      \b IN: GOP HW type select
// @param u8Wid         \b IN: GWIN ID
// @return TRUE: sucess / FALSE: fail
// @note: Must Set FADE init Fun, Then Set FADE Trigger Fun Each times.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_FADE_TRIGGER(GOP_HW_Type eGOP_Type, U8 u8Wid)
{
    U32 u32CtrlReg;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    // When GOP HW type is E_GOP1G, only support 1 GWIN (GWIN ID = 3)
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, FALSE);

    u32CtrlReg = REG(REG_GOP_GWIN_FADE_SET(u8Wid));

    ((GOP_gwin_fade_ctrl *)&u32CtrlReg)->GWIN_FADE_INIT   = 0;

    REG(REG_GOP_GWIN_FADE_SET(u8Wid)) = u32CtrlReg;

    return TRUE;



}

//-------------------------------------------------------------------------------------------------
// Destroy the GWin
// @param eGOP_Type      \b IN: GOP HW type select
// @param u8Wid         \b IN: GWIN ID
// @return  None
// @note This function will disable GWIN (let it disapper)
// In addition, it also destroy GWIN features (such as scroll mode, blink, transparent, alpha blending, duplicate)
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_Destroy(GOP_HW_Type eGOP_Type, U8 u8Wid)
{
    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    // When GOP HW type is E_GOP1G, only support 1 GWIN (GWIN ID = 3)
    if ((eGOP_Type == E_GOP1G) || (eGOP_Type == E_GOP1GX))
    {
        u8Wid = 1;
    }

    _MHal_GOP_GWIN_Enable(eGOP_Type, u8Wid, FALSE);

    //clear all the effects
    _MHal_GOP_Blink_Enable(eGOP_Type , FALSE);
    _MHal_GOP_GWIN_Scroll_Enable(eGOP_Type, u8Wid, GOP_SCROLL_NONE, 0, FALSE);

#if 0 // NOTE: H & V duplicate is removed in Titania
    _MHal_GOP_Output_HDup_Enable(eGOP_Type, FALSE);
    _MHal_GOP_Output_VDup_Enable(eGOP_Type, FALSE);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
/// Force to Write GWIN registers
/// @param  eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param  bEnable \b IN: TRUE/FALSE : force write immediately/write at Vsync
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_ForceWrite(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_ForceWrite(eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    return bRet;
}



//-------------------------------------------------------------------------------------------------
/// Enable/Disable a GWIN Alpha
/// @param eGOP_Type \b IN: GOP HW type
/// @param u8Wid  \b IN: GWIN ID
/// @param bEnable \b IN: TRUE/FALSE : enable/disable
/// @return TRUE: sucess / FALSE: fail
/// @note If set bEnable to FALSE, this function can only disable Gwin instead of destory Gwin.
/// it won't disable GWIN features (scroll mode, blink, transparent, alpha blending, duplicate)
/// - Example: GOP0:Gwin0 have constant alpha, You can call MHal_GOP_GWIN_Enable(0, 0, FALSE) to disable gwin,
/// than call MHal_GOP_GWIN_Enable (0, 0, TRUE) to enable gwin, the constant alpha feature is still existed.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_GWIN_Alpha_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable)
{
    U32 u32RegVal;
    U32 u32GwinID=0;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    _MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID);

    _MHal_GOP_Select(eGOP_Type, FALSE);

    u32RegVal = REG(REG_GOP_GWIN_SET(u8Wid));
    ((GOP_gwin_set*)&u32RegVal)->GWIN_ALPHA_EN = bEnable;
/*
    if(bEnable)
    {
        //clear software reset
        _MHal_GOP_Select(eGOP_Type, TRUE);
        u32RegVal0 = REG(REG_GOP_SOFT_RESET);
        ((GOP_soft_reset*)&u32RegVal0)->SOFT_RESET = 0;
        _GWinInfo[u32GwinID].u32Enable = TRUE;
    }
    else
    {
        _GWinInfo[u32GwinID].u32Enable = FALSE;
    }
*/
    _MHal_GOP_Select(eGOP_Type, FALSE);
    REG(REG_GOP_GWIN_SET(u32GwinID)) = u32RegVal;

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Set GOP Input Field Inverse
// @param eGOP_Type \b IN: GOP HW type select
// @param bProgress \b IN: TRUE: Field-Inverse mode \ FALSE: non Field-Inverse mode
// @return TRUE: sucess / FALSE: fail
// @note
// - This function influences all Gwins ouput mode of GOP.
// - You must set this function before create Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_SetFieldInver(GOP_HW_Type eGOP_Type, B16 bFieldInverse)
{
    U32 u32RegVal;

    if (eGOP_Type > E_GOP1GX)
    {
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_SOFT_RESET);

    ((GOP_soft_reset*)&u32RegVal)->FIELD_INVERSE = bFieldInverse;

    REG(REG_GOP_SOFT_RESET) = u32RegVal;
    return TRUE;
}



//-------------------------------------------------------------------------------------------------
// Set GOP Palette Control field
// @param eGOP_Type \b IN: GOP HW type select
// @param bProgress \b IN: TRUE: Field-Inverse mode \ FALSE: non Field-Inverse mode
// @return TRUE: sucess / FALSE: fail
// @note
// - This function influences all Gwins ouput mode of GOP.
// - You must set this function before create Gwins of GOP.
//-------------------------------------------------------------------------------------------------
static B16 _MHal_GOP_SetPaletteControl(GOP_HW_Type eGOP_Type, GopPalCtrlMode ePaletteCtrl)
{

    U32 u32RegVal;

    if (eGOP_Type > E_GOP1GX)
    {
        return FALSE;
    }

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type
	u32RegVal = REG(REG_GOP_PAL_SET);
	((GOP_pal_set*)&u32RegVal)->PAL_CTL = ePaletteCtrl;//E_GOP_PAL_GE;
	REG(REG_GOP_PAL_SET) = u32RegVal;

    return TRUE;
}

static B16 _MHal_GOP_VMirror_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U32 u32RegVal;


    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
         DBG_GOP(printk ("Error GOP Type"));
#else
         DBG_GOP(printk ("Error GOP Type"));
#endif
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type
    u32RegVal = REG(REG_GOP_SOFT_RESET);
    ((GOP_soft_reset*)&u32RegVal)->DISP_VBACK = bEnable;
    REG(REG_GOP_SOFT_RESET) = u32RegVal;

    return TRUE;
}

static B16 _MHal_GOP_HMirror_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U32 u32RegVal;


    if (eGOP_Type > E_GOP1GX)
    {
#ifdef RED_LION
         DBG_GOP(printk ("Error GOP Type"));
#else
         DBG_GOP(printk ("Error GOP Type"));
#endif
        return FALSE;
    }

    //Change GOP Bank
    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type

    u32RegVal = REG(REG_GOP_SOFT_RESET);
    ((GOP_soft_reset*)&u32RegVal)->DISP_HBACK = bEnable;

    REG(REG_GOP_SOFT_RESET) = u32RegVal;

    return TRUE;

}

#if 0 // NOTE: H & V duplicate is removed in Titania
//-------------------------------------------------------------------------------------------------
/// Enable GOP Horizontal Duplicate
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param bEnable \b IN: TRUE: Enable \ FALSE: Disable
/// @return TRUE: sucess / FALSE: fail
/// @note
/// - This function influences all Gwins ouput mode of GOP.
/// - You must set this function before create Gwins of GOP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Output_HDup_Enable (GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_Output_HDup_Enable (eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Enable GOP Vertical Duplicate
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param bEnable \b IN: TRUE: Enable \ FALSE: Disable
/// @return TRUE: sucess / FALSE: fail
/// @note
/// - This function influences all Gwins ouput mode of GOP.
/// - You must set this function before create Gwins of GOP.
/// - Note: If GOP is in interlace mode should not use vertical duplicate for best quality.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Output_VDup_Enable (GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_Output_VDup_Enable (eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}
#endif

//-------------------------------------------------------------------------------------------------
/// Set Gop color key for the corresponding color type
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param TRSColor   \b IN: color key
/// @return TRUE: sucess / FALSE: fail
/// @note  This function set transparent color key for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_TransClr_Set(GOP_HW_Type eGOP_Type, GOPTRSColor  TRSColor)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_TransClr_Set(eGOP_Type, TRSColor);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


//-------------------------------------------------------------------------------------------------
/// Enable / Disable transparent color for the corresponding color type
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param bEnable    \b IN: TRUE/FALSE
/// @return TRUE: sucess / FALSE: fail
/// @note  This function enable transparent color key for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_TransClr_Enable( GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_TransClr_Enable(eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}



//-------------------------------------------------------------------------------------------------
/// Set GOP blink rate for BLINK / ALPHA_BLINK GWINs
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u32Rate    \b IN: blink rate (unit : vsync time)
/// @return TRUE: sucess / FALSE: fail
/// @note  This function set BLINK rate for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Blink_SetRate(GOP_HW_Type eGOP_Type, U32 u32Rate)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_Blink_SetRate(eGOP_Type, u32Rate);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}





//-------------------------------------------------------------------------------------------------
/// Enable/Disable GOP Blink for BLINK / ALPHA_BLINK GWINs
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param bEnable \b IN: TRUE/FALSE
/// @return TRUE: sucess / FALSE: fail
/// @note This function enable BLINK for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Blink_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_Blink_Enable(eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Set H & V scroll rate
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u32Rate \b IN: scroll rate (unit : vsync time)
/// @return TRUE: sucess / FALSE: fail
/// @note This function set scroll rate for all Gwins of GOP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Scroll_SetRate(GOP_HW_Type eGOP_Type,U32 u32Rate)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_Scroll_SetRate(eGOP_Type, u32Rate);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Enable/Disable H / V scroll
/// @param eGOP_Type  \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8Wid      \b IN: GWin ID
/// @param eScrollType \b IN: scroll type
/// @param u16ScrollStep \b IN: scroll step
/// @param bEnable     \b IN: TRUE/FALSE
/// @return TRUE: sucess / FALSE: fail
/// @note
/// - When eScrollType is GOP_SCROLL_NONE, ignore the parameter bEnable.
/// - Horiziontal scroll unit: 16/ColorDepth pixel; Vertical scroll unit: 1 pixel;
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Scroll_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, GopScrollType eScrollType, U8 u16ScrollStep, U16 bEnable)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_GWIN_Scroll_Enable(eGOP_Type, u8Wid, eScrollType, u16ScrollStep, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


//-------------------------------------------------------------------------------------------------
/// Set Scroll auto stop Horizontal offset
/// @param eGOP_Type  \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8Wid             \b IN: GWin ID
/// @param u32ScrollAutoHStop \b IN: scorll auto stop H offset, unit: (16/ColorDepth) pixels.
/// @return TRUE: sucess / FALSE: fail
/// @note In HW scroll mode. HW will stop scroll based on specified RBLK H offset .
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Scroll_AutoStop_HSet(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32ScrollAutoHStop)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_GWIN_Scroll_AutoStop_HSet(eGOP_Type, u8Wid, u32ScrollAutoHStop);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Set Scroll auto stop Verticall offset
/// @param eGOP_Type         \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8Wid             \b IN: GWin ID
/// @param u32ScrollAutoVStop \b IN: scorll auto stop V offset (pixel unit)
/// @return TRUE: sucess / FALSE: fail
/// @note In HW scroll mode. HW will stop scroll based on specified RBLK V offset .
///
/// @image html Auto_Stop_Example.JPG "Example of auto stop"
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Scroll_AutoStop_VSet(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32ScrollAutoVStop)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_GWIN_Scroll_AutoStop_VSet(eGOP_Type, u8Wid, u32ScrollAutoVStop);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


//-------------------------------------------------------------------------------------------------
/// Enable/Disable H / V  auto stop mode
/// @param eGOP_Type      \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8Wid          \b IN: GWin ID
/// @param eScrollStopType \b IN: Scroll auto stop type.
/// @param bEnable         \b IN: TRUE/FALSE
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Scroll_AutoStop_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, GopScrollAutoStopType eScrollStopType, U16 bEnable)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_GWIN_Scroll_AutoStop_Enable(eGOP_Type, u8Wid,  eScrollStopType, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


//-------------------------------------------------------------------------------------------------
/// Set GOP palette for I8(8-bit palette) format
/// @param eGOP_Type      \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param pPalArray       \b IN: ptr to palette entry array
/// @param u32PalStart     \b IN: Define palette entry start index for set (range: 0~255)
/// @param u32PalEnd       \b IN: Define palette entry end index for set (range: 0~255)
/// @param ePalType        \b IN: palette type
/// @return TRUE: sucess / FALSE: fail
/// @note 1: GOP palette is single port SRAM, You must set palettes before you enable Gwin which needs to read palette table.
/// @note 2: GOP have one palette (256 entries) for BLINK and I8, Gwins of GOP share one palette.
/// - Blink (valid palette index: 0~31)
/// - I8 (valid palette index: 0~255)
/// - Example: MHal_GOP_Palette_Set ( 0, pPalArray, 32, 255, E_GOP_PAL_RGB888)
///     - Set GOP0 palette [32~255] from pPalArray[0~224] (palette [32] = pPalArray[0])
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Palette_Set ( GOP_HW_Type eGOP_Type, GopPaletteEntry *pPalArray, U32 u32PalStart, U32 u32PalEnd)
{
    U32 u32I, u32J;
    U16 bRet = FALSE;


    if((u32PalEnd >= GOP_PALETTE_ENTRY_NUM) || (u32PalStart > u32PalEnd))
    {
        return FALSE;
    }


#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    //Set clock to ODCLK before setting palette tables. TODO ??

    // Set Palette Tables

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type
    REG(REG_GOP_PAL_SET) &= ~GOP_PAL_APART_MASK;


    for (u32I = u32PalStart, u32J=0; u32I<=u32PalEnd; u32I++, u32J++)
    {
    	//printk("index %x pal data=%x\n",u32J,pPalArray[u32J]);
        bRet = _MHal_GOP_Palette_Set(eGOP_Type, &pPalArray[u32J], u32I);
        if (bRet == FALSE)
            break;
    }
    if( bRet )
        _MHal_GOP_Palette_Toggle_Set( eGOP_Type ) ;

    //Restore original GOP clock value TODO ??

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif
    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Read GOP palette for I8(8-bit palette) format
/// @param eGOP_Type      \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param pPalArray       \b IN: ptr to palette entry array
/// @param u32PalStart     \b IN: Define palette entry start index for set (range: 0~255)
/// @param u32PalEnd       \b IN: Define palette entry end index for set (range: 0~255)
/// @param ePalType        \b IN: palette type
/// @return TRUE: sucess / FALSE: fail
/// @note 1: GOP palette is single port SRAM, You must set palettes before you enable Gwin which needs to read palette table.
/// @note 2: GOP have one palette (256 entries) for BLINK and I8, Gwins of GOP share one palette.
/// - Blink (valid palette index: 0~31)
/// - I8 (valid palette index: 0~255)
/// - Example: MHal_GOP_Palette_Read ( 0, pPalArray, 32, 255, E_GOP_PAL_RGB888)
///     - Read GOP0 palette [32~255] from pPalArray[0~224] (palette [32] = pPalArray[0])
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Palette_Read ( GOP_HW_Type eGOP_Type, GopPaletteEntry *pPalArray, U32 u32PalStart, U32 u32PalEnd)
{
    U32 u32I, u32J;
    U16 bRet = FALSE;


    if((u32PalEnd >= GOP_PALETTE_ENTRY_NUM) || (u32PalStart > u32PalEnd))
    {
        return FALSE;
    }


#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    //Set clock to ODCLK before setting palette tables. TODO ??

    // Set Palette Tables

    _MHal_GOP_Select(eGOP_Type, TRUE);    // Select bank according to GOP type
    REG(REG_GOP_PAL_SET) &= ~GOP_PAL_APART_MASK;


    for (u32I = u32PalStart, u32J=0; u32I<=u32PalEnd; u32I++, u32J++)
    {
    	//printk("index %x pal data=%x\n",u32J,pPalArray[u32J]);
        bRet = _MHal_GOP_Palette_Read(eGOP_Type, &pPalArray[u32J], u32I);
        if (bRet == FALSE)
            break;
    }
    if( bRet )
        _MHal_GOP_Palette_Toggle_Set( eGOP_Type ) ;

    //Restore original GOP clock value TODO ??

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif
    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Set GOP palette for Blink format
/// @param eGOP_Type      \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8WinId         \b IN: GWin ID: 0~3
/// @param pPalArray       \b IN: ptr to palette entry array
/// @param u32PalNum       \b IN: set Blink Palette number
/// @param ePalType        \b IN: palette type
/// @return TRUE: sucess / FALSE: fail
/// @note 1: GOP4G palette is single port SRAM, You must set palettes before you enable Gwin which needs to read palette table.
/// @note 2: GOP4G have one palette (256 entries) for BLINK and I8, Gwins of GOP share one palette.
/// @note 3: If each GWIN different color format, then they may share some palette entries.
/// - Color Formate                         12355            2266           I8
/// - GOP4G-GWIN0 :valid palette index:    0 ~  31          0 ~  63       0 ~ 255
/// - GOP4G-GWIN1 :valid palette index:   32 ~  63         64 ~ 127       0 ~ 255
/// - GOP4G-GWIN2 :valid palette index:   64 ~  95        128 ~ 191       0 ~ 255
/// - GOP4G-GWIN3 :valid palette index:   96 ~ 127        192 ~ 255       0 ~ 255
/// - Example: MDrv_GOP_Palette_Set ( E_GOP4G_0, 0, pPalArray, 32, E_GOP_PAL_RGB888)
///     - Set GOP0 GWIN0 Blink palette [128~159] from pPalArray[0~31]
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Blink_Palette_Set ( GOP_HW_Type eGOP_Type, U8 u8WinId,
                                          GopPaletteEntry *pPalArray,
                                          U32 u32PalNum, GopPalType ePalType)
/* U16 MHal_GOP_GWIN_Blink_Palette_Set ( GOP_HW_Type eGOP_Type, U8 u8WinId,
                                          GopPaletteEntry *pPalArray,
                                          U32 u32PalNum)
 */
{
    U32 u32GwinID=0, u32PalStart = 0, u32I, u32J;
    U16 bRet = FALSE;

    if (!_MHal_GOP_Get_GWinID(eGOP_Type, &u8WinId, &u32GwinID))
        return FALSE;

    if((u32PalNum > BLINK_PALETTE_ENTRY_NUM) || (eGOP_Type >= E_GOP1G))
    {
        return FALSE;
    }

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    //Set clock to ODCLK before setting palette tables. TODO ??

    if (E_GOP_PAL_I8 == ePalType)
    {
        u32PalStart = 0;
    }
    else if (E_GOP_PAL_BLINK12355 == ePalType)
    {
        u32PalStart = 32 * u8WinId;
        }
    else if (E_GOP_PAL_BLINK2266 == ePalType)
    {
        u32PalStart = 64 * u8WinId;
    }


    // Set Apart Palette Tables
    REG(REG_GOP_PAL_SET) |= GOP_PAL_APART_MASK;

    for (u32I = u32PalStart, u32J=0; u32I<=(u32PalStart+u32PalNum); u32I++, u32J++)
    {
    	//printk("index %lx pal data=%lx\n",u32J,pPalArray[u32J]);
        bRet = _MHal_GOP_Palette_Set(eGOP_Type, &pPalArray[u32J], u32I);
        if (bRet == FALSE)
        {	//printk("u32I=%lx\n",u32I);
            break;
        }
    }

    //Restore original GOP clock value TODO ??
#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif
    return bRet;
}



//-------------------------------------------------------------------------------------------------
/// Set GWIN Alpha Blending
/// @param eGOP_Type    \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8Wid        \b IN: GWin ID: 0~3
/// @param bEnable       \b IN: TRUE/FALSE Enable/Disable Per Pixel Alpha Blending
/// @param u32Alpha      \b IN: Constant Alpha Value (Range: 0~63; 0:transparent; 63: opaque)
/// @return TRUE: sucess / FALSE: fail
/// @note 1: HW limitation
/// - GOP combine with IP output have 8 Level Alpha
/// - GOP combine with OP output have 64 Level Alpha
/// @note 2: if you set Gobal Alpha Value, the Per Pixel Alpha Blending won't be enabled.
/// - Example 1: MHal_GOP_GWIN_Blending_Set(0, 0, FALSE, 30) -- Enable Constant Alpha, won't enable Per Pixel Alpha
/// - Example 2: MHal_GOP_GWIN_Blending_Set(0, 0, TRUE, NULL) -- Enable Per Pixel Alpha
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Blending_Set(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable, U32 u32Alpha)
{
    U16 bRet = FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_GWIN_Blending_Set (eGOP_Type, u8Wid, bEnable, u32Alpha);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}



//-------------------------------------------------------------------------------------------------
/// Get GWIN parameters
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param  pGInfo    \b IN: ptr to GopInfo
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------

B16 MHal_GOP_Info_Get(GOP_HW_Type eGOP_Type, GopInfo *pGInfo)
{
    ASSERT(pGInfo);

    pGInfo->bForceWrite     = _GopInfo[eGOP_Type].bForceWrite;
    pGInfo->bProgressiveOut = _GopInfo[eGOP_Type].bProgressiveOut;
    pGInfo->eDstType        = _GopInfo[eGOP_Type].eDstType;
    pGInfo->eGopMux         = _GopInfo[eGOP_Type].eGopMux;
    pGInfo->u8X_Base        = _GopInfo[eGOP_Type].u8X_Base;
    switch (eGOP_Type)
    {
        case E_GOP4G_0:
            pGInfo->u8NumOfGWin = 4;
            break;
        case E_GOP2G_1:
            pGInfo->u8NumOfGWin = 2;
            break;
        case E_GOP1G:
        case E_GOP1GX:
            pGInfo->u8NumOfGWin = 1;
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Set GWIN parameters
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param  u8Wid    \b IN: GWin ID
/// @param  pGInfo    \b IN: ptr to GopGwinInfo
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Info_Set(GOP_HW_Type eGOP_Type, U8 u8Wid, GopGwinInfo *pGInfo)
{
    //U32 u32HStarAlign, u32RamAlign;
    U32 u32GwinID=0;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
	{
        return FALSE;
	}

    _MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID);

    _GWinInfo[u32GwinID].u32Enable = pGInfo->u32Enable;
    _GWinInfo[u32GwinID].u32DispHStart = pGInfo->u32DispHStart;
    _GWinInfo[u32GwinID].u32DispVStart = pGInfo->u32DispVStart;
    _GWinInfo[u32GwinID].u32DispWidth = pGInfo->u32DispWidth;
    _GWinInfo[u32GwinID].u32DispHeight = pGInfo->u32DispHeight;
    _GWinInfo[u32GwinID].u32DRAMRBlkStart = pGInfo->u32DRAMRBlkStart;
    _GWinInfo[u32GwinID].u32DRAMRBlkHSize = pGInfo->u32DRAMRBlkHSize;
    _GWinInfo[u32GwinID].u32DRAMRBlkVSize = pGInfo->u32DRAMRBlkVSize;
    _GWinInfo[u32GwinID].u32SrcHStart = pGInfo->u32SrcHStart;
    _GWinInfo[u32GwinID].u32SrcVStart = pGInfo->u32SrcVStart;
    _GWinInfo[u32GwinID].eColorType = pGInfo->eColorType;

   	if (!_MHal_GOP_GWIN_WinInfo_Set(eGOP_Type, u8Wid, pGInfo))
    {
        return FALSE;
    }

	//_GWinInfo[u32GwinID].u32ColorDepth = pGInfo->u32ColorDepth;
    switch(pGInfo->eColorType)
    {
        case E_GOP_COLOR_RGB555_BLINK:
        case E_GOP_COLOR_RGB565:
        case E_GOP_COLOR_ARGB4444:
        case E_GOP_COLOR_RGB555:
        case E_GOP_COLOR_YUV422_VU7Y8:
        case E_GOP_COLOR_YUV422_VU8Y8:
            _GWinInfo[u32GwinID].u32ColorDepth = 2;
            break;

        case E_GOP_COLOR_I8:
            _GWinInfo[u32GwinID].u32ColorDepth = 1;
            break;

        case E_GOP_COLOR_ARGB8888:
            _GWinInfo[u32GwinID].u32ColorDepth = 4;
            break;

        default:
            ASSERT(0);
            break;
    }

    return TRUE;

}


//-------------------------------------------------------------------------------------------------
/// Get GWIN parameters
/// @param eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param  u8Wid    \b IN: GWin ID
/// @param  pGInfo    \b IN: ptr to GopGwinInfo
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Info_Get(GOP_HW_Type eGOP_Type, U8 u8Wid, GopGwinInfo *pGInfo)
{
    U32 u32HStarAlign, u32RamAlign;
    U32 u32GwinID=0;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

    _MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID);



    u32RamAlign = 8;
    u32HStarAlign = u32RamAlign/_GWinInfo[u32GwinID].u32ColorDepth;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    pGInfo->u32ColorDepth= _GWinInfo[u32GwinID].u32ColorDepth;
    pGInfo->u32DispHStart= _GWinInfo[u32GwinID].u32DispHStart & ~(u32HStarAlign-1);
    pGInfo->u32DispVStart= _GWinInfo[u32GwinID].u32DispVStart;
    pGInfo->u32DispHeight= _GWinInfo[u32GwinID].u32DispHeight;
    pGInfo->u32DispWidth= _GWinInfo[u32GwinID].u32DispWidth & ~(u32HStarAlign-1);
    pGInfo->u32DRAMRBlkStart= _GWinInfo[u32GwinID].u32DRAMRBlkStart & ~(u32RamAlign-1);
    pGInfo->u32DRAMRBlkHSize= _GWinInfo[u32GwinID].u32DRAMRBlkHSize & ~(u32HStarAlign-1);
    pGInfo->u32DRAMRBlkVSize= _GWinInfo[u32GwinID].u32DRAMRBlkVSize;
    pGInfo->u32Enable= _GWinInfo[u32GwinID].u32Enable;
    pGInfo->eColorType= _GWinInfo[u32GwinID].eColorType;
    pGInfo->u32SrcHStart= _GWinInfo[u32GwinID].u32SrcHStart & ~(u32HStarAlign-1);
    pGInfo->u32SrcVStart= _GWinInfo[u32GwinID].u32SrcVStart;

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif
    return TRUE;

}


//------------------------------------------------------------------------------
/// Gain the control of control of one of GOP window identified by 'u8GOP_num' and 'u8Wid'
/// @param  eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param  u8Wid     \b IN: index of GWIN [ 0 .. (@ref MAX_GWIN_SUPPORT - 1) ]
/// @return TRUE: Control of the GOP window is gained.
/// @return FALSE:
/// @return  1. The desired GOP window is currently controlled by someone else.
/// @return  2. 'u8GOP_num' is out of range or the control of the GOP unit is not been occupied.
/// @return  3. 'u8Wid' is out of range.
/// @note All other functions with GOP window number as one of their argument should be called after the control of this GOP unit and the GOP window are both gained.
//------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Alloc(GOP_HW_Type eGOP_Type, U8 u8Wid )
{
    U32 u32GwinID=0;

    if (eGOP_Type > E_GOP1GX)
    {
        return FALSE;
    }

    if (!_MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID))
    {
        return FALSE;
    }

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    _GWinInfo[u32GwinID].bUsed = TRUE;
//printk("GwinID %x used\n",u32GwinID);
    _MHal_GOP_Clk_Enable(eGOP_Type, TRUE);
	#if SUBTITLE_IN_IP
    _MHal_GOP_Clk_Set(E_GOP2G_1, E_GOP_DST_IP);//configure IP CLKGen0 for MUXOUT_1 (Arki 0808)  ArkiDebug
	#endif
    //_MHal_GOP_DWIN_CLKGen_Set(E_GOP_DWIN);
#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    return TRUE;
}


//------------------------------------------------------------------------------
/// Release the control of one of GOP window identified by 'u8GOP_num' and 'u8Wid'.
/// @param  eGOP_Type \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param  u8Wid     \b IN: index of GWIN [ 0 .. (@ref MAX_GWIN_SUPPORT-1) ]
/// @return TRUE: Control of the GOP window is released successfully.
/// @return FALSE:  'u8GOP_num' or 'u8Wid' is out of range.
/// @note This function should be called while the module that has gained the control of this GOP window has no more interest in asking this GOP window to do things for it.
//------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Free(GOP_HW_Type eGOP_Type, U8 u8Wid)
{
    U32 u32GwinID=0;
    U8  i, u8StartIndex, u8EndIndex;
    U16 blDisableClk;


    if (eGOP_Type > E_GOP1GX)
    {
        return FALSE;
    }

    if (!_MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID))
    {
        return FALSE;
    }


#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    if (_GWinInfo[u32GwinID].bUsed == TRUE)
    {
        _MHal_GOP_GWIN_Destroy(eGOP_Type, u8Wid);
        _GWinInfo[u32GwinID].bUsed = FALSE;
    }

    blDisableClk = TRUE;

    if (E_GOP4G_0 == eGOP_Type)
    {
        u8StartIndex = 0;
        u8EndIndex = MAX_GOP0_GWIN_SUPPORT;
    }
    else if (E_GOP2G_1 == eGOP_Type)
    {
        u8StartIndex = MAX_GOP0_GWIN_SUPPORT;
        u8EndIndex = MAX_GOP0_GWIN_SUPPORT + MAX_GOP1_GWIN_SUPPORT;
    }
    else if (E_GOP1G == eGOP_Type)
    {
        u8StartIndex = MAX_GOP0_GWIN_SUPPORT + MAX_GOP1_GWIN_SUPPORT;
        u8EndIndex = MAX_GOP0_GWIN_SUPPORT + MAX_GOP1_GWIN_SUPPORT + MAX_GOP2_GWIN_SUPPORT;
    }
    else
    {
        u8StartIndex = MAX_GOP0_GWIN_SUPPORT + MAX_GOP1_GWIN_SUPPORT + MAX_GOP2_GWIN_SUPPORT;
        u8EndIndex = TOTAL_GWIN_SUPPORT;
        blDisableClk = TRUE;
    }

    for (i = u8StartIndex; i < u8EndIndex; i++)
        {
        if (_GWinInfo[i].bUsed)
            {
            blDisableClk = FALSE;
                break;
            }
        }

#if 0   //No need to disable gop clock, we have pown_down function can disable all gop clock
    if (TRUE == blDisableClk)     // Turn off GOP clock.
        {
            _MHal_GOP_Clk_Enable(eGOP_Type, FALSE);
    }
#endif


#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
/// Create a GWin
/// @param eGOP_Type       \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8Wid           \b IN: GWIN ID
/// @param eColorType       \b IN: color type
/// @param u32SrcX          \b IN: src Xstart (pixel unit)
/// @param u32SrcY          \b IN: src Ystart (pixel unit)
/// @param u32DispX         \b IN: disp Xstart (pixel unit)
/// @param u32DispY         \b IN: disp Ystart (pixel unit)
/// @param u32Width         \b IN: display width (pixel unit)
/// @param u32Height        \b IN: display height (pixel unit)
/// @param u32DRAMRBlkStart \b IN: RBLK start address (byte unit: should be a multiple of 16)
/// @param u32DRAMRBlkHSize \b IN: RBLK H size (pixel unit)
/// @param u32DRAMRBlkVSize \b IN: RBLK V size (pixel unit)
/// @return TRUE: sucess / FALSE: fail
/// @note 1:
/// - BLINK, I8 or Alpha-BLINK format should set their palette before create Gwin.
/// - When eColorType = E_GOP_COLOR_RGB555_BLINK, HW can detect its format is RGB555 or BLINK automatically.
///
/// @image html GOP_Window.JPG "Explaination of parameters"
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Create(GOP_HW_Type           eGOP_Type,
                          U8           u8Wid,
                          GOPColorType eColorType,
                          U32          u32SrcX,
                          U32          u32SrcY,
                          U32          u32DispX,
                          U32          u32DispY,
                          U32          u32Width,
                          U32          u32Height,
                          U32          u32DRAMRBlkStart,
                          U32          u32DRAMRBlkHSize,
                          U32          u32DRAMRBlkVSize)
{
    U32 u32GwinID=0;
    U16 bRet = FALSE;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    _MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID);

    _GWinInfo[u32GwinID].u32DispHStart = u32DispX;
    _GWinInfo[u32GwinID].u32DispVStart = u32DispY;
    _GWinInfo[u32GwinID].u32DispWidth = u32Width;
    _GWinInfo[u32GwinID].u32DispHeight = u32Height;
    _GWinInfo[u32GwinID].u32DRAMRBlkStart = u32DRAMRBlkStart;
    _GWinInfo[u32GwinID].u32DRAMRBlkHSize = u32DRAMRBlkHSize;
    _GWinInfo[u32GwinID].u32DRAMRBlkVSize = u32DRAMRBlkVSize;
    _GWinInfo[u32GwinID].u32SrcHStart = u32SrcX;
    _GWinInfo[u32GwinID].u32SrcVStart = u32SrcY;
    _GWinInfo[u32GwinID].eColorType = eColorType;
    switch(eColorType)
    {
        case E_GOP_COLOR_RGB555_BLINK:
        case E_GOP_COLOR_RGB565:
        case E_GOP_COLOR_ARGB4444:
        case E_GOP_COLOR_RGB555:
        case E_GOP_COLOR_YUV422_VU7Y8:
        case E_GOP_COLOR_YUV422_VU8Y8:
            _GWinInfo[u32GwinID].u32ColorDepth = 2;
            break;

        case E_GOP_COLOR_I8:
            _GWinInfo[u32GwinID].u32ColorDepth = 1;
            break;

        case E_GOP_COLOR_ARGB8888:
            _GWinInfo[u32GwinID].u32ColorDepth = 4;
            break;

        default:
            ASSERT(0);
            break;
    }
#if 0
    printk("Create GWIN Start -->\n GWinID[%d]\n Type:0x%X\n Wid: 0x%X\n CrTy: 0x%X\n SrcX: 0x%X\n SrcY: 0x%X\n DipX: 0x%X\n DipY: 0x%X\n WdTh: 0x%X\n HeTh: 0x%X\n RAMS: 0x%08X\n RAMH: 0x%X\n RAMV: 0x%X\n ",
                            u32GwinID,
                            eGOP_Type,//pOSDDef->gopType,
                            u8Wid,//pOSDDef->wid,
                            eColorType,
				            u32SrcX,
				            u32SrcY,
				            u32DispX,
				            u32DispY,
				            u32Width,
				            u32Height,
				            u32DRAMRBlkStart,
				            u32DRAMRBlkHSize,//(pOSDDef->stride>>_gGPUBPPShift[pOSDDef->pxlDepth]),
				            u32DRAMRBlkVSize//pOSDDef->height);
				            );
#endif
    bRet = _MHal_GOP_GWIN_WinInfo_Set(eGOP_Type, u8Wid, &_GWinInfo[u32GwinID]);
    //printk("Create GWIN End --<");

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    #ifdef FPGA_TEST
//    REG(REG_GOP_DMA) = 0x0404;
    #endif
    return bRet;

}


//-------------------------------------------------------------------------------------------------
/// Enable/Disable a GWIN
/// @param eGOP_Type       \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8Wid           \b IN: GWIN ID
/// @param bEnable          \b IN: TRUE/FALSE : enable/disable
/// @return TRUE: sucess / FALSE: fail
/// @note If set bEnable to FALSE, this function can only disable Gwin instead of destory Gwin.
/// it won't disable GWIN features (scroll mode, blink, transparent, alpha blending, duplicate)
/// - Example: GOP0:Gwin0 have constant alpha, You can call MHal_GOP_GWIN_Enable(0, 0, FALSE) to disable gwin,
/// than call MHal_GOP_GWIN_Enable (0, 0, TRUE) to enable gwin, the constant alpha feature is still existed.
//-------------------------------------------------------------------------------------------------
B16  MHal_GOP_GWIN_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable)
{
    U16 bRet=FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_GWIN_Enable(eGOP_Type, u8Wid, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


B16 MHal_GOP_StretchWin_Create(GOP_HW_Type eGOP_Type,
                                             U32 u32DispX, U32 u32DispY,
                                             U32 u32Width, U32 u32Height)

{
    U16 bRet=FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif
//printk("MHal_GOP_StretchWin_Create\n");

    bRet = _MHal_GOP_StretchWin_WinInfo_Set(eGOP_Type, u32DispX, u32DispY, u32Width, u32Height);


#ifdef RED_LION
    //local_irq_save(u32OldIntr);
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;

}


B16 MHal_GOP_StretchWin_SetRatio(GOP_HW_Type eGOP_Type, U32 u32H_Ratio, U32 u32V_Ratio)
{
    U16 bRet=FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_StretchWin_SetRatio(eGOP_Type, u32H_Ratio, u32V_Ratio);


#ifdef RED_LION
    //local_irq_save(u32OldIntr);
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


//-------------------------------------------------------------------------------------------------
/// Enable/Disable a GWIN Alpha
/// @param eGOP_Type \b IN: GOP HW type
/// @param u8Wid  \b IN: GWIN ID
/// @param bEnable \b IN: TRUE/FALSE : enable/disable
/// @return TRUE: sucess / FALSE: fail
/// @note If set bEnable to FALSE, this function can only disable Gwin instead of destory Gwin.
/// it won't disable GWIN features (scroll mode, blink, transparent, alpha blending, duplicate)
/// - Example: GOP0:Gwin0 have constant alpha, You can call MHal_GOP_GWIN_Enable(0, 0, FALSE) to disable gwin,
/// than call MHal_GOP_GWIN_Alpha_Enable (0, 0, TRUE) to enable gwin, the constant alpha feature is still existed.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_Alpha_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable)
{
    U16 bRet=FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_GWIN_Alpha_Enable(eGOP_Type, u8Wid, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


#if 0 // NOTE: Pixel-shift is removed in Titania
//-------------------------------------------------------------------------------------------------
/// MDrv_Set_GOP_Offset: set GOP Offset Value for set Gwin in origin (0,0)
/// @param eGOP_Type   \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param eDstType     \b IN: DstPlane Type
/// @param u16XOffset   \b IN: XOffset (only useful when eDstType = E_GOP_DST_IP0 or E_GOP_DST_IP1)
/// @param u16YOffset   \b IN: YOffset
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Set_Offset (GOP_HW_Type eGOP_Type, GOPDstType eDstType, U16 u16XOffset, U16 u16YOffset)
{
    U8  u8LeftShift;
    U16 bRet;

    //Set Y Base
    ASSERT(u16YOffset>0);

    _GopInfo[eGOP_Type].u8Y_Base = u16YOffset;

    if (eDstType == E_GOP_DST_OP)
    {
        u16XOffset -= GOP_OP_H_Offset;
        ASSERT(u16XOffset>0);
    }

    //Cal X BASE based on GOP alignment
    if (u16XOffset % GOP_ALIGNMENT)
        _GopInfo[eGOP_Type].u8X_Base = ((u16XOffset/GOP_ALIGNMENT)+1)*GOP_ALIGNMENT;
    else
        _GopInfo[eGOP_Type].u8X_Base = ((u16XOffset/GOP_ALIGNMENT))*GOP_ALIGNMENT;

    //Use PixelShift to Shift GWin to Original
    u8LeftShift = (U8)(_GopInfo[eGOP_Type].u8X_Base - u16XOffset);
    bRet = _MHal_GOP_Pixel_Shift_Set (eGOP_Type, u8LeftShift);

    return bRet;
}
#endif

//-------------------------------------------------------------------------------------------------
/// Move GWin display position
/// @param eGOP_Type   \b IN: GOP HW type select (@ref GOP_HW_Type)
/// @param u8Wid    \b IN: GWIN ID
/// @param u32DispX  \b IN: new display X
/// @param u32DispY  \b IN: new display Y
/// @return TRUE: sucess / FALSE: fail
/// @note u32DispX should a multiple of (16/color format depths) pixels.
/// - Example 1: RGB565 (unit = 16/2 =8), the u32DispX should be a multiple of 8;
/// - Example 2: ARGB8888 (unit = 16/4 = 4), the u32DispX should be a multiple of 4.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_DispPos_Move(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32DispX, U32 u32DispY)
{

    U32 u32GwinID=0;

    if (!_MHal_GOP_GWIN_ErrCheck(eGOP_Type, u8Wid))
        return FALSE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    _MHal_GOP_Get_GWinID(eGOP_Type, &u8Wid, &u32GwinID);

    //MHal_GOP_GWIN_Enable(u8Wid, FALSE);

    _GWinInfo[u32GwinID].u32DispVStart = u32DispY;
    _GWinInfo[u32GwinID].u32DispHStart = u32DispX;

    _MHal_GOP_GWIN_WinInfo_Set(eGOP_Type, u8Wid, &_GWinInfo[u32GwinID]);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Enable/Disable GWIN VSync interrupt
// @param eGOP_Type   \b IN: GOP HW type select (@ref GOP_HW_Type)
// @param bEnable \b IN: TRUE / FALSE
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Int_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_Int_Enable(eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    //_MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


//-------------------------------------------------------------------------------------------------
// Read GWIN VSync interrupt Mask.
// @param  eGOP_Type \b IN : GOP HW type select (@ref GOP_HW_Type)
// @param  bIntStatus\b OUT: TRUE: Enable Vsync INT / FALSE: Disable Vsync INT
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Int_GetStatus(GOP_HW_Type eGOP_Type, U16 *pbIntStatus)
{
    U16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_Int_GetStatus(eGOP_Type, pbIntStatus);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    //_MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Initialize GOP registers
/// @param  eGOP_Type \b IN : GOP HW type select (@ref GOP_HW_Type)
/// @return TRUE: sucess /FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Init(GOP_HW_Type eGOP_Type)
{
    U16 bRet = TRUE;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_Init(eGOP_Type);

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    return bRet;

}

//-------------------------------------------------------------------------------------------------
/// Select GOP output format is YUV or RGB
/// @param  eGOP_Type \b IN : GOP HW type select (@ref GOP_HW_Type)
/// @param bEnable IN: TRUE: YUV output / FALSE: RGB output
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_YUVOut_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_YUVOut_Enable(eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;

}


//-------------------------------------------------------------------------------------------------
/// Set GOP Destination DisplayPlane
/// @param eGOP_Type \b IN:  GOP HW type select (@ref GOP_HW_Type)
/// @param eDstType \b IN: GOP Destination DisplayPlane Type
/// @return TRUE: sucess / FALSE: fail
/// @note
/// - E_GOP4G_0 can be selected to E_GOP_DST_IP/E_GOP_DST_OP.
/// - E_GOP4G_1 can be selected to E_GOP_DST_IP/E_GOP_DST_OP.
/// - E_GOP1G can only be selected to E_GOP_DST_OP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_DstPlane_Set(GOP_HW_Type eGOP_Type, GOPDstType eDstType)
{

#if RED_LION

    //local_irq_save(u32OldIntr);

    //set Dst-Plane
    if (!_MHal_GOP_DstPlane_Set(eGOP_Type, eDstType))
        return FALSE;


    //Use YUV out as default.
    if (!_MHal_GOP_YUVOut_Enable(eGOP_Type, TRUE))
        return FALSE;

// Set Scaler Register for select IP0 or IP1 from gop1 or gop2
    switch(eDstType)
    {
    case E_GOP_DST_IP:
        _MHal_GOP_Scaler_Set_GOPEnable(eGOP_Type, FALSE);  // disable GOP

        #if T3
        if (eGOP_Type==E_GOP2G_1)
            _MHal_GOP_Scaler_Set_GOPSel(MS_IP0_SEL_GOP1);
        else
            _MHal_GOP_Scaler_Set_GOPSel(MS_IP0_SEL_GOP0);
        #else
        if (eGOP_Type==E_GOP4G_0)
            _MHal_GOP_Scaler_Set_GOPSel(MS_IP0_SEL_GOP0);
        else
            _MHal_GOP_Scaler_Set_GOPSel(MS_IP0_SEL_GOP1); //mingchia
        #endif
        //MDrv_Scaler_Set_GOPEnable(0);
        break;

    case E_GOP_DST_IP_SUB:// New in T3.
        break;

/*  // Titania just support one IP
	case E_GOP_DST_IP1:
        _MHal_GOP_Scaler_Set_GOPEnable(u8GOP, FALSE);
        if (u8GOP==0)
            _MHal_GOP_Scaler_Set_GOPSel(MS_IP1_SEL_GOP0);
        else
            _MHaE_GOP_DST_OPl_GOP_Scaler_Set_GOPSel(MS_IP1_SEL_GOP1);
        //MDrv_Scaler_Set_GOPEnable(0);
        break;
*/
    case E_GOP_DST_OP:
        #if T3
        if (eGOP_Type==E_GOP2G_1)
            _MHal_GOP_Scaler_Set_GOPSel(MS_NIP_SEL_GOP1);
        else
            _MHal_GOP_Scaler_Set_GOPSel(MS_NIP_SEL_GOP0);
        #else
        if (eGOP_Type==E_GOP4G_0)
            _MHal_GOP_Scaler_Set_GOPSel(MS_NIP_SEL_GOP0);
        else
            _MHal_GOP_Scaler_Set_GOPSel(MS_NIP_SEL_GOP1);
        #endif
        _MHal_GOP_Scaler_Set_GOPEnable(eGOP_Type, TRUE);  // enable gop
        break;
    #if 0
    case E_GOP_DST_OP1:
        if (u8GOP==0)
            MDrv_Scaler_Set_GOPSel(MS_NIP_SEL_GOP0);
        else
            MDrv_Scaler_Set_GOPSel(MS_NIP_SEL_GOP1);
        MDrv_Scaler_Set_GOPEnable(u8GOP, TRUE);
        break;
		#endif
    case E_GOP_DST_MVOP:
        // ???

        break;
    }

   // _MHal_GOP_ChipTop_SetGOPClk(u8GOP, dsttype);
	_MHal_GOP_Clk_Set(eGOP_Type, eDstType);
   _MHal_GOP_UpdateReg(eGOP_Type);

#endif

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Enable GOP Output Progressive Mode
/// @param  eGOP_Type \b IN : GOP HW type select (@ref GOP_HW_Type)
/// @param bProgress \b IN: TRUE: Progressive \ FALSE: Interlace
/// @return TRUE: sucess / FALSE: fail
/// @note
/// - This function influences all Gwins ouput mode of GOP.
/// - You must set this function before create Gwins of GOP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Output_Progressive_Enable (GOP_HW_Type eGOP_Type, U16 bProgress)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_Output_Progressive_Enable(eGOP_Type, bProgress);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}


B16 MHal_GOP_Scaler_Set_GOPSel(MS_IPSEL_GOP ipSelGop)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_Scaler_Set_GOPSel(ipSelGop);
	_MHal_GOP_Scaler_Set_GOPEnable((U8)ipSelGop,FALSE);  // if select IP , should disable VOP-enable
#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    //_MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;



}



//-------------------------------------------------------------------------------------------------
// Enable GOP Output Progressive Mode
/// @param  eGOP_Type \b IN : GOP HW type select (@ref GOP_HW_Type)
/// @param bProgress \b IN: TRUE: Progressive \ FALSE: Interlace
/// @return TRUE: sucess / FALSE: fail
/// @note
/// - This function influences all Gwins ouput mode of GOP.
/// - You must set this function before create Gwins of GOP.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_SetFieldInver(GOP_HW_Type eGOP_Type, B16 bFieldInverse)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_SetFieldInver(eGOP_Type, bFieldInverse);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}

B16 MHal_GOP_SetPaletteControl(GOP_HW_Type eGOP_Type, GopPalCtrlMode ePaletteCtrl)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_SetPaletteControl(eGOP_Type, ePaletteCtrl);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;
}

#if 0  // NOTE: TWin is removed in Titania
///////////////////////////// TWIN ///////////////////////////////////

//-------------------------------------------------------------------------------------------------
// GOP Transparent Window set
// @param eGOP_Type      \b IN: GOP HW type select
// @param pTwinInfo      \b IN: GOP Twin Info
// @return TRUE: sucess / FALSE: fail
// @note: E_GOP1G can't support TWIN
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_TWIN_Create(GOP_HW_Type eGOP_Type, GopTwinInfo* pTwinInfo)
{

    U16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_TWIN_Set(eGOP_Type, pTwinInfo);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;


}

//-------------------------------------------------------------------------------------------------
// GOP Transparent Window set
// @param eGOP_Type      \b IN: GOP HW type select
// @param bEnable        \b IN: TRUE:Enable Twin / FALSE:Disable Twin
// @return TRUE: sucess / FALSE: fail
// @note: E_GOP1G can't support TWIN
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_TWIN_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_TWIN_Enable(eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;


}
#endif


//-------------------------------------------------------------------------------------------------
// GWIN FADE INIT
// @param eGOP_Type      \b IN: GOP HW type select
// @param u8Wid         \b IN: GWIN ID
// @param u8Rate         \b IN: FADE Rate, RANGE 0~15 (unit: 1 Vsync Time)
// @param eFADE_Type     \b IN: FADE Type
// @return TRUE: sucess / FALSE: fail
// @note: Must Set FADE init Fun, Then Set FADE Trigger Fun Each times.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_FADE_Init(GOP_HW_Type eGOP_Type, U8 u8Wid, U8 u8Rate, GOP_FADE_Type eFADE_Type)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_GWIN_FADE_Init(eGOP_Type, u8Wid, u8Rate, eFADE_Type);


#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;


}

//-------------------------------------------------------------------------------------------------
// Trigger FADE Function.
// @param eGOP_Type      \b IN: GOP HW type select
// @param u8Wid         \b IN: GWIN ID
// @return TRUE: sucess / FALSE: fail
// @note: Must Set FADE init Fun, Then Set FADE Trigger Fun Each times.
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_GWIN_FADE_TRIGGER(GOP_HW_Type eGOP_Type, U8 u8Wid)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_GWIN_FADE_TRIGGER(eGOP_Type, u8Wid);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(eGOP_Type);

    return bRet;


}



//------------------------------------------------------------------------------
/// Clean the IRQ bit (in interrupt handler should call this function while the
/// interrupt has been triggered.
/// @param none
/// @return none
/// @internal
//------------------------------------------------------------------------------
//B16 MHal_GOP_ClearIRQ(GOP_HW_Type eGOP_Type)
B16 MHal_GOP_ClearIRQ( void )
{
   	GOP_HW_Type _GOP_Type=E_GOP4G_0;
   	U16 u16Gop;
   	U32 u32RegVal;

   	u16Gop=REG(REG_GOP_WR_ACK);

	if(u16Gop&GOP0_INT_FLAG_MASK)
		_GOP_Type=E_GOP4G_0;
	else if(u16Gop&GOP1_INT_FLAG_MASK)
		_GOP_Type=E_GOP2G_1;
	else if(u16Gop&GOPS_INT_FLAG_MASK)
		_GOP_Type=E_GOP1G;
	else if(u16Gop&GOP1GX_INT_MASK)
		_GOP_Type=E_GOP1GX;
	else{
        printk("unknow GOP MASK\n");
#ifdef	NOT_USED_4_LGE	// remove the infinite loop case(dreamer@lge.com)
        while(1);
#else
        return 0;
#endif
    }

   	_MHal_GOP_Select( _GOP_Type, TRUE);    // Select bank according to GOP type

   	u32RegVal = REG(REG_GOP_INT);
	REG(REG_GOP_INT)=u32RegVal|0x3; // clear vs0 and vs1
	REG(REG_GOP_INT)=u32RegVal|~(0x3); // enable vs0 and vs1
    //*pbIntStatus = (!(((GOP_int *)&u32RegVal)->VS0_INT_MASK | ((GOP_int *)&u32RegVal)->VS1_INT_MASK));

    return 1 ;
}
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
//------------------------------------------------------------------------------
/// MHal_GOP_ProcessIRQ
/// @param none
/// @return none
/// @internal
//------------------------------------------------------------------------------
//B16 MHal_GOP_ProcessIRQ()
B16 MHal_GOP_ProcessIRQ( void )
{
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
   	GOP_HW_Type _GOP_Type=E_GOP4G_0;
   	U16 u16Gop;

	//step 1: check if vsync. interrupt
	u16Gop = REG(REG_GOP_INT);

	if(!(u16Gop & GOP_VS0_INT)) //it is not the vsync. interrupt from GOP.
		return 0;

       //check which Gop fire the vsyc interrupt:
   	u16Gop=REG(REG_GOP_WR_ACK);

	if(u16Gop&GOP0_INT_FLAG_MASK)
		_GOP_Type=E_GOP4G_0;
	else if(u16Gop&GOP1_INT_FLAG_MASK)
		_GOP_Type=E_GOP2G_1;
	else if(u16Gop&GOPS_INT_FLAG_MASK)
		_GOP_Type=E_GOP1G;
	else if(u16Gop&GOP1GX_INT_MASK)
		_GOP_Type=E_GOP1GX;
	else
	{
	    printk("MHal_GOP_ProcessIRQ: unknow GOP MASK\n");
	    while(1);
	}
	//step 2:check if any flip request in queue with this Gop:
	if(_GopFlipInfoReadPtr[_GOP_Type] == _GopFlipInfoWritePtr[_GOP_Type]) //no any flip request in queue
	{
	    return 0;
	}

	//step 3: if get queue, check if TAG back.
	if(NULL == fptrCheckTagId) //No GE Driver reference, just return fail.
	{
		printk("MHal_GOP_ProcessIRQ: NULL fptrCheckTagId, GE function ptr not get!\n");
		return 0;
	}

	if(!fptrCheckTagId(_GopFlipInfo[_GOP_Type][_GopFlipInfoReadPtr[_GOP_Type]].u32TagId))
		return 0;

       //Tag returned, we need programming flip address:
   	_MHal_GOP_Select( _GOP_Type, FALSE);    // Select bank according to GOP type

        //step 4: if Tag Back: set flip to GOP.
        REG(REG_GOP_DRAM_RBLK_STR_LO(_GopFlipInfo[_GOP_Type][_GopFlipInfoReadPtr[_GOP_Type]].u32GwinIdx))= LO16BIT(_GopFlipInfo[_GOP_Type][_GopFlipInfoReadPtr[_GOP_Type]].u32Addr);
        REG(REG_GOP_DRAM_RBLK_STR_HI(_GopFlipInfo[_GOP_Type][_GopFlipInfoReadPtr[_GOP_Type]].u32GwinIdx)) = HI16BIT(_GopFlipInfo[_GOP_Type][_GopFlipInfoReadPtr[_GOP_Type]].u32Addr);

	// set to update right away
	REG(REG_GOP_WR_ACK) |= GOP_FWR_MASK;

	//set ReadPtr to next, this entry consumed!
	_GopFlipInfoReadPtr[_GOP_Type] = (_GopFlipInfoReadPtr[_GOP_Type]+1)%MAX_FLIP_ADDR_FIFO;
#endif
	return 1 ;
}
#endif
/////////////////////////////////////////////////////////////////////
/////////                 DWIN Function            //////////////////
/////////////////////////////////////////////////////////////////////


/******************************************************************************/
/// init DWIN
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_Init(void)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_DWIN_Init();

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    //_MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;
}


//------------------------------------------------------------------------------
/// Gain the control of control of DWIN
/// @param  None
/// @return TRUE: Control of the DWIN is gained.
/// @return FALSE: Fail to get the control
/// @return The DWIN is currently controlled by someone else.
//------------------------------------------------------------------------------
B16 MHal_GOP_DWIN_Alloc(void )
{

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    _MHal_GOP_Clk_Enable(E_GOP_DWIN, TRUE);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    return TRUE;
}


//------------------------------------------------------------------------------
/// Release the control of DWIN
/// @param  None
/// @return TRUE: Control of the DWIN is released successfully.
/// @return FALSE:  Fail to release
//------------------------------------------------------------------------------
B16 MHal_GOP_DWIN_Free(void)
{
#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    _MHal_GOP_Clk_Enable(E_GOP_DWIN, FALSE);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    return TRUE;
}


/******************************************************************************/
/// Capture frames Enable / Disable
/// @param bEnable \b IN
///   - # TRUE start capture
///   - # FALSE stop capture
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_CaptureStream_Enable(U16 bEnable)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_DWIN_CaptureStream_Enable(bEnable);

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    _MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;

}

/******************************************************************************/
/// Set DWIN capture mode (capture one fram or each fram)
/// @param eCaptureMode \b IN DWIN capture mode
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_CaptureMode_Set(GopCaptureMode eCaptureMode)
{

    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_DWIN_CaptureMode_Set(eCaptureMode);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


   // _MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;

}

//Arki >>
/******************************************************************************/
/// Set DWIN capture mode (capture one fram or each fram)
/// @param eCaptureMode \b IN DWIN capture mode
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_InputSrcMode_Set(EN_GOP_DWIN_DATA_SRC eDwin_Data_Src_Mode)
{

    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_DWIN_InputSourceMode_Set(eDwin_Data_Src_Mode);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    //_MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;

}

B16 MHal_GOP_DWIN_CLKGen_Set(EN_GOP_DWIN_DATA_SRC eDwin_Data_Src_Mode)
{

    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_DWIN_CLKGen_Set(eDwin_Data_Src_Mode);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    //_MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;
}
//Arki <<

/******************************************************************************/
/// Set DWIN capture in interlaced or progressive mode
/// @param bProgresive \b IN
///   - # TRUE  DWIN progressive mode
///   - # FALSE DWIN interlaced mode
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_SetScanType(U16 bProgresive)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif


    bRet = _MHal_GOP_DWIN_Progressive_Enable(bProgresive);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    //_MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;

}

/******************************************************************************/
/// Set DWIN setting to registers
/// @param pinfo \b IN \copydoc GOP_DWIN_INFO
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_WinInfo_Set(GOP_DWIN_INFO* pinfo)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_DWIN_WinInfo_Set(pinfo);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif


    //_MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;
}

/******************************************************************************/
/// Set interrupt mask of GOP DWIN.
/// @param eIntType \b IN Interrupt Type
/// @param bEnable \b IN
///   - # TRUE enable interrupts
///   - # FALSE disable interrupts
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_Int_Enable(GOPDWINIntType eIntType, U16 bEnable)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_DWIN_Int_Enable(eIntType, bEnable);

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    //_MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;
}

/******************************************************************************/
/// Get Interrupt Status
///   - bit3 DWIN VSYNC interrupt
///   - bit2 DWIN interlace Bottom Field ACK Interrupt
///   - bit1 DWIN interlace Top Field ACK Interrupt
///   - bit0 DWIN Progressive ACK Interrupt
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_GetIntrStatus(U16 *pu16IntStatus)
{
    B16 u16Ret;
#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    u16Ret = _MHal_GOP_DWIN_GetIntrStatus(pu16IntStatus);

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    return u16Ret;

}

/******************************************************************************/
/// Set DWIN Pinpon buffer
/// @param addr \b IN Base address of DWIN pinpon buffer
/// @param upbond \b IN Upper boundary of DWIN pinon buffer
/// @return TRUE: success / FALSE: fail
/******************************************************************************/
B16 MHal_GOP_DWIN_Set_PINPON(U32 addr, U32 upbond)
{
    B16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_DWIN_Set_PINPON(addr, upbond);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

   // _MHal_GOP_UpdateReg(E_GOP_DWIN);

    return bRet;

}


#define TEST_PATT_GWIN_ID 	  E_GOP2G_1

//20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MHal_GOP_Test_Pattern(U8 u8Pattern_Type, U8 u8Pattern_Param, U16 u16HSPD,
        U16 u16HStart, U16 u16VStart, U16 u16HSize, U16 u16VSize)
{
    U8 u8Width_L, u8Width_H;
    U8 u8Height_L, u8Height_H;

    if (gGOP_Test_Pattern_Init == FALSE)
    {
        MHal_GOP_Reg_Store();
    }

    //u8Bank = REG(REG_SC_BASE);
    u8Width_L = (u16HSize/2) & 0xff;
    u8Width_H = ((u16HSize/2) & 0xff00) >> 8;
    u8Height_L = u16VSize & 0xff;
    u8Height_H = (u16VSize & 0xff00) >> 8;
    REG_8(REG_GOP_BASE + 0xfe*2+1) |= BIT1; //enable fwr

//    REG_8(REG_SC_BASE + 0x05*4) |= BIT6; //sync HDMI timing ?
    REG_8(REG_GOP_BASE + 0xfe*2) = 0x03; //Switch to bank3
    REG_8(REG_GOP_BASE + 1) |= BIT2; //RGB change to YUV
    //REG_8(REG_GOP_BASE + 0xfc*2) = 0x01; /'/Set gopg0_mux to GOP1
    //REG_8(REG_GOP_BASE + 0x7e*4) = 0x06; //GOP mux0 switch to 1G, mux1 switch to 2G

    REG_8(REG_GOP_BASE + 0xfe*2) = 0x0; //Switch to bank0
    //REG_8(REG_GOP_BASE + 0xf*4) = 0x1B;  //set 4G_HS_PIPE
    REG_8(REG_GOP_BASE + 0xfe*2) = 0x3; //Switch to bank3

    REG_8(REG_GOP_BASE + 0x02*2) = 0x0; //Set GOP des to IP
    REG_8(REG_GOP_BASE + 0x1c*2) = u8Width_L; //Set GOP1 H-Total (PANL_WIDTH: 1366==> (1366+2)/2 = 0x2AC)
    REG_8(REG_GOP_BASE + 0x1c*2 + 1) = u8Width_H; //Set GOP1 H-Total
    REG_8(REG_GOP_BASE + 0x60*2) = u8Width_L; //Set GOP1 Stretch window H-Size (ex:WIDTH:1366==> (1366+2)/2=0x2AC)
    REG_8(REG_GOP_BASE + 0x60*2 + 1) = u8Width_H; //Set GOP1 Stretch window H-Size
    REG_8(REG_GOP_BASE + 0x62*2) = u8Height_L;//Set GOP1 Stretch window V-Size (ex:HEIGHT:768)
    REG_8(REG_GOP_BASE + 0x62*2 + 1) = u8Height_H; //Set GOP1 Stretch window V-Size
    REG_8(REG_GOP_BASE + 0x64*2) = 0x0; //Set GOP1 Stretch window H-Start
    REG_8(REG_GOP_BASE + 0x64*2 + 1) = 0x00; //Set GOP1 Stretch window H-Start
    REG_8(REG_GOP_BASE + 0x68*2) = 0x00; //Set GOP1 Stretch window V-Start
    REG_8(REG_GOP_BASE + 0x68*2 + 1) = 0x00; //Set GOP1 Stretch window V-Start
    REG_8(REG_GOP_BASE + 0x6a*2) = 0x00; //Set GOP1 Stretch window H ratio
    REG_8(REG_GOP_BASE + 0x6a*2 + 1) = 0x10; //Set GOP1 Stretch window H ratio
    REG_8(REG_GOP_BASE + 0x6c*2) = 0x00; //Set GOP1 Stretch window V ratio
    REG_8(REG_GOP_BASE + 0x6c*2 + 1) = 0x10; //Set GOP1 Stretch window V ratio

    REG_8(REG_GOP_BASE + 1) |= BIT6; //Set Enable reg_hs_mask

    REG_8(REG_GOP_BASE + 0xfe*2) = 0x04; //Switch to bank4
    REG_8(REG_GOP_BASE + 0x40*2 + 1) = 0x7F; //Set GOP1 GWIN1 alpha blending
    REG_8(REG_GOP_BASE + 0x48*2) = 0x00;//Set GOP1 GWIN1 H-Start
    REG_8(REG_GOP_BASE + 0x48*2 + 1) = 0x00;//Set GOP1 GWIN1 H-Start

    if (gGOP_Test_Pattern_Init == FALSE)
    {
        REG_8(REG_GOP_BASE) = 0x00; //disable GOP1 GWIN1 (clear bit0)
        REG_8(REG_GOP_BASE + 0x40*2) = 0x00; //disable GOP1 GWIN2 (clear bit0)
        REG_8(REG_GOP_BASE + 0x40*2) = 0x11;//Set GOP1 GWIN1 data type to RGB565 and Enable GOP1 GWIN1
        gGOP_Test_Pattern_Init= TRUE;
    }
#if 0
    if((bHDuplicate) &&(u8VideoSrc == 0))
    {
        u16HSize = u16HSize/2;
    }
#endif
    u8Width_L = (u16HSize/8)&0xff;      //T3: word unit is 128 bits-> word pixel is 128/16=8
    u8Width_H = ((u16HSize/8)&0xf00)>>8; //T3: word unit is 128 bits-> word pixel is 128/16=8

    REG_8(REG_GOP_BASE + 0x4a*2) = u8Width_L; //Set GOP1 GWIN H-End (ex:WIDTH:1366==> 1366/(word pixel)=0x156)
    REG_8(REG_GOP_BASE + 0x4a*2 + 1) = u8Width_H;//Set GOP1 GWIN H-End
#if 0
    switch(u8VideoSrc)
    {
    case 0://DTV Input
        u16StrWinHStr = u16HStart+0xf;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x02; //Set GOP1 GWIN V-Start
        break;
    case 1://ATV Input
        u16StrWinHStr = u16HStart+0x62;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x02; //Set GOP1 GWIN V-Start
        break;
    case 2://Scart Input
        u16StrWinHStr = u16HStart+0x64;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x02; //Set GOP1 GWIN V-Start
        break;
    case 3://AV Input
        u16StrWinHStr = u16HStart+0x1B;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x02; //Set GOP1 GWIN V-Start
        break;
    case 4://Auto AV(SCART) Input
        u16StrWinHStr = u16HStart+0x64;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x02; //Set GOP1 GWIN V-Start
        break;
    case 5://Component(YPbPr) Input
        u16StrWinHStr = u16HStart-0x4;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x02; //Set GOP1 GWIN V-Start
        break;
    case 6://RGB Input
        u16StrWinHStr = u16HStart-0xA;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x00; //Set GOP1 GWIN V-Start
        break;
    case 7://HDMI Input
        u16StrWinHStr = u16HStart-0x5;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x00; //Set GOP1 GWIN V-Start
        break;
    default://Other Source Input
        u16StrWinHStr = u16HStart+GOP_IP_PD;
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x02; //Set GOP1 GWIN V-Start
        break;
    }
    if(u16VSize>=700) //720P, 1080i or 1080p
        REG_8(REG_GOP_BASE + 0x4c*2) = 0x01; //Set GOP1 GWIN V-Start
#endif
    MHal_GOP_AdjustHSPD(1, u16HSPD);  //set 2G_HS_PIPE
    REG_8(REG_GOP_BASE + 0x4c*2 +1) = 0x00; //Set GOP1 GWIN V-Start
    REG_8(REG_GOP_BASE + 0x50*2) = u8Height_L;//Set GOP1 GWIN V-End (ex:HEIGHT:768)
    REG_8(REG_GOP_BASE + 0x50*2 +1) = u8Height_H; //Set GOP1 GWIN V-End

    #if T3
    REG_8(REG_CHIP_BASE + 0x40*4+1) |= BIT2; //Set IP clock equal to GOP1
    #else
    REG_8(REG_CHIP_BASE + 0x16*4) |= BIT7; //Set IP clock equal to GOP1
    #endif
    REG_8(REG_GOP_BASE + 0xfe*2+1) &= ~(BIT0 | BIT1 | BIT2); //disable fwr

	_MHal_GOP_Select(TEST_PATT_GWIN_ID, TRUE);

    switch(u8Pattern_Type)
    {
        case 0: //disable test pattern
            REG_8(REG_GOP_BASE) = 0x00; //Disable test pattern mode
			_MHal_GOP_UpdateReg(TEST_PATT_GWIN_ID);
			MHal_GOP_Reg_Restore();
			gGOP_Test_Pattern_Init= FALSE;
            break;
        case 1: //color white
            REG_8(REG_GOP_BASE) = 0x40; //Enable test pattern mode
            REG_8(REG_GOP_BASE + 0x48*2) = u8Pattern_Param;
            REG_8(REG_GOP_BASE + 0x48*2 + 1) = u8Pattern_Param;
            REG_8(REG_GOP_BASE + 0x4a*2) = u8Pattern_Param;
			_MHal_GOP_Select(TEST_PATT_GWIN_ID, FALSE);
            REG_8(REG_GOP_BASE + 1) = 0x40; //enable transpant color
            //2008.10.1 Brian add
            REG_8(REG_GOP_BASE + 0x10*2) = 0x00;//Set "vertical R incremental" and "vertical R incremental sign signification"
            REG_8(REG_GOP_BASE + 0x02*2) = 0x00; //Set "vertical G incremental" and "vertical G incremental sign signification"
            REG_8(REG_GOP_BASE + 0x02*2 + 1) = 0x00;//Set "vertical B incremental" and "vertical B incremental sign signification"
            REG_8(REG_GOP_BASE + 0x42*2 + 1) = 0x00; //Set "horizontal R incremental" and "horizontal R incremental sign signification"
            REG_8(REG_GOP_BASE + 0x44*2) = 0x00; //Set "horizontal G incremental" and "horizontal G incremental sign signification"
            REG_8(REG_GOP_BASE + 0x52*2) = 0x00; //Set "horizontal B incremental" and "horizontal B incremental sign signification"

            if(u16VSize>=700) //720P, 1080i or 1080p
            {
                _MHal_GOP_UpdateReg(1); //Update register value
                REG_8(REG_GOP_BASE + 0xfe*2) = 0x04; //switch to bank4
                REG_8(REG_GOP_BASE + 0x4c*2) = 0x00; //Set GOP1 GWIN V-Start
            }
            break;
         case 2: //color red
            REG_8(REG_GOP_BASE) = 0x40; //Enable test pattern mode
            REG_8(REG_GOP_BASE + 0x48*2) = 0x00;//Bdata set to 0
            REG_8(REG_GOP_BASE + 0x48*2 + 1) = 0x00;//Gdata set to 0
            REG_8(REG_GOP_BASE + 0x4a*2) = u8Pattern_Param;
			_MHal_GOP_Select(TEST_PATT_GWIN_ID, FALSE);
            REG_8(REG_GOP_BASE + 1) = 0x40; //enable transpant color
            //2008.10.1 Brian add
            REG_8(REG_GOP_BASE + 0x10*2) = 0x00;//Set "vertical R incremental" and "vertical R incremental sign signification"
            REG_8(REG_GOP_BASE + 0x02*2) = 0x00; //Set "vertical G incremental" and "vertical G incremental sign signification"
            REG_8(REG_GOP_BASE + 0x02*2 + 1) = 0x00;//Set "vertical B incremental" and "vertical B incremental sign signification"
            REG_8(REG_GOP_BASE + 0x42*2 + 1) = 0x00; //Set "horizontal R incremental" and "horizontal R incremental sign signification"
            REG_8(REG_GOP_BASE + 0x44*2) = 0x00; //Set "horizontal G incremental" and "horizontal G incremental sign signification"
            REG_8(REG_GOP_BASE + 0x52*2) = 0x00; //Set "horizontal B incremental" and "horizontal B incremental sign signification"

            if(u16VSize>=700) //720P, 1080i or 1080p
            {
                _MHal_GOP_UpdateReg(1); //Update register value
                REG_8(REG_GOP_BASE + 0xfe*2) = 0x04; //switch to bank4
                REG_8(REG_GOP_BASE + 0x4c*2) = 0x00; //Set GOP1 GWIN V-Start
            }
            break;
         case 3: //color green
            REG_8(REG_GOP_BASE) = 0x40; //Enable test pattern mode
            REG_8(REG_GOP_BASE + 0x48*2) = 0x00;//Bdata set to 0
            REG_8(REG_GOP_BASE + 0x48*2 + 1) = u8Pattern_Param;
            REG_8(REG_GOP_BASE + 0x4a*2) = 0x00;//Rdata set to 0
			_MHal_GOP_Select(TEST_PATT_GWIN_ID, FALSE);
            REG_8(REG_GOP_BASE + 1) = 0x40; //enable transpant color
            //2008.10.1 Brian add
            REG_8(REG_GOP_BASE + 0x10*2) = 0x00;//Set "vertical R incremental" and "vertical R incremental sign signification"
            REG_8(REG_GOP_BASE + 0x02*2) = 0x00; //Set "vertical G incremental" and "vertical G incremental sign signification"
            REG_8(REG_GOP_BASE + 0x02*2 + 1) = 0x00;//Set "vertical B incremental" and "vertical B incremental sign signification"
            REG_8(REG_GOP_BASE + 0x42*2 + 1) = 0x00; //Set "horizontal R incremental" and "horizontal R incremental sign signification"
            REG_8(REG_GOP_BASE + 0x44*2) = 0x00; //Set "horizontal G incremental" and "horizontal G incremental sign signification"
            REG_8(REG_GOP_BASE + 0x52*2) = 0x00; //Set "horizontal B incremental" and "horizontal B incremental sign signification"

            if(u16VSize>=700) //720P, 1080i or 1080p
            {
                _MHal_GOP_UpdateReg(1); //Update register value
                REG_8(REG_GOP_BASE + 0xfe*2) = 0x04; //switch to bank4
                REG_8(REG_GOP_BASE + 0x4c*2) = 0x00; //Set GOP1 GWIN V-Start
            }
            break;
         case 4: //color blue
			REG_8(REG_GOP_BASE) = 0x40; //Enable test pattern mode
            REG_8(REG_GOP_BASE + 0x48*2) = u8Pattern_Param;
            REG_8(REG_GOP_BASE + 0x48*2 + 1) = 0x00;//Gdata set to 0
            REG_8(REG_GOP_BASE + 0x4a*2) = 0x00;//Rdata set to 0
			_MHal_GOP_Select(TEST_PATT_GWIN_ID, FALSE);
            REG_8(REG_GOP_BASE + 1) = 0x40; //enable transpant color
            //2008.10.1 Brian add
            REG_8(REG_GOP_BASE + 0x10*2) = 0x00;//Set "vertical R incremental" and "vertical R incremental sign signification"
            REG_8(REG_GOP_BASE + 0x02*2) = 0x00; //Set "vertical G incremental" and "vertical G incremental sign signification"
            REG_8(REG_GOP_BASE + 0x02*2 + 1) = 0x00;//Set "vertical B incremental" and "vertical B incremental sign signification"
            REG_8(REG_GOP_BASE + 0x42*2 + 1) = 0x00; //Set "horizontal R incremental" and "horizontal R incremental sign signification"
            REG_8(REG_GOP_BASE + 0x44*2) = 0x00; //Set "horizontal G incremental" and "horizontal G incremental sign signification"
            REG_8(REG_GOP_BASE + 0x52*2) = 0x00; //Set "horizontal B incremental" and "horizontal B incremental sign signification"

            if(u16VSize>=700) //720P, 1080i or 1080p
            {
                _MHal_GOP_UpdateReg(1); //Update register value
                REG_8(REG_GOP_BASE + 0xfe*2) = 0x04; //switch to bank4
                REG_8(REG_GOP_BASE + 0x4c*2) = 0x00; //Set GOP1 GWIN V-Start
            }
            break;
    }
#if 0
    REG_8(REG_SC_BASE) = 0x18; //Switch banck 18
    u16csc = REG(REG_SC_BASE + 0x6E*4); // VStart
    if (u16csc ==  BIT0)
    {
        REG_8(REG_GOP_BASE + 0xfe*2) = 0x03; //switch to bank3
        REG_8(REG_GOP_BASE + 1) &= ~BIT2; // change RGB
    }
#endif
    REG_8(REG_GOP_BASE + 0xfe*2) = 0x04; //switch to bank4
    REG_8(REG_GOP_BASE + 0x4c*2) = 0x02; //Set GOP1 GWIN V-Start
    _MHal_GOP_UpdateReg(TEST_PATT_GWIN_ID); //Update register value
}

void MHal_GOP_Reg_Store(void)
{
    U8 u8I;

    //store chip top
    u32StoreChip= REG(REG_CHIP_BASE + 0x40*4);

    //store scaler
    #if 0
    REG_8(REG_SC_BASE) = 0x01; //Switch banck 1
    u32StoreSC[0] = REG(REG_SC_BASE + 0x04*4); // VStart
    u32StoreSC[1] = REG(REG_SC_BASE + 0x05*4); // HStart
    u32StoreSC[2] = REG(REG_SC_BASE + 0x06*4); // VSize
    u32StoreSC[3] = REG(REG_SC_BASE + 0x07*4); // HSize
    REG_8(REG_SC_BASE) = 0x10; //Switch banck to VOP
    u32StoreSC[4] = REG(REG_SC_BASE + 0x46*2); //Set VOP "New Blending"
    #endif

    //REG_8(REG_SC_BASE) = 0x00; //Switch to bank0
    //u32StoreSC[5] = REG(REG_SC_BASE + 0x5*4);
    //u32StoreSC[6] = REG(REG_SC_BASE + 0x6*4);

    //store bank 4 registers
	_MHal_GOP_Select(TEST_PATT_GWIN_ID, FALSE);
    for(u8I=0x0;u8I<0x30;u8I++)
        u32StoreB4[u8I] = REG(REG_GOP_BASE + u8I*4);

    //store bank 3 registers
	_MHal_GOP_Select(TEST_PATT_GWIN_ID, TRUE);
    for(u8I=0x0;u8I<0x40;u8I++)
        u32StoreB3[u8I] = REG(REG_GOP_BASE + u8I*4);
	//store GOP mux
	u32StoreB3[u8I] = REG(REG_GOP_BASE + 0x7e*4);
}

void MHal_GOP_Reg_Restore(void)
{
    U8 u8I;

    //restore bank 4 registers
	_MHal_GOP_Select(TEST_PATT_GWIN_ID, FALSE);
    REG_8(REG_GOP_BASE + 0xfe*2) &= ~(BIT8|BIT9);
    for(u8I=0x0;u8I<0x30;u8I++)
        REG(REG_GOP_BASE + u8I*4) = u32StoreB4[u8I];

    //restore bank 3 registers
	_MHal_GOP_Select(TEST_PATT_GWIN_ID, TRUE);
    for(u8I=0x0;u8I<0x40;u8I++)// 0x01 : GOP Destination IP -> OP 변경으로 인해 과도 보임.
        REG(REG_GOP_BASE + u8I*4) = u32StoreB3[u8I];

    _MHal_GOP_UpdateReg(1); //Update register value

	//restore GOP mux
	REG(REG_GOP_BASE + 0x7e*4) = u32StoreB3[u8I];
#if 0
    REG_8(REG_GOP_BASE + 0xfe*2) = 0x0; //Switch to bank0
    REG_8(REG_GOP_BASE + 0xf*4) = 0x1B;  //set 4G_HS_PIPE
    REG_8(REG_GOP_BASE + 0xfe*2) = 0x3; //Switch to bank3
    REG_8(REG_GOP_BASE + 0xf*4) = 0x0;  //set 2G_HS_PIPE
#endif
    //restore chip top
    REG(REG_CHIP_BASE + 0x40*4) =u32StoreChip;
#if 0
    //restore scaler
    REG_8(REG_SC_BASE) = 0x01; //Switch banck 1
    REG(REG_SC_BASE + 0x04*4) = u32StoreSC[0]; // VStart
    REG(REG_SC_BASE + 0x05*4) = u32StoreSC[1]; // HStart
    REG(REG_SC_BASE + 0x06*4) = u32StoreSC[2]; // VSize
    REG(REG_SC_BASE + 0x07*4) = u32StoreSC[3]; // HSize
    REG_8(REG_SC_BASE) = 0x10; //Switch banck to VOP
    REG(REG_SC_BASE + 0x46*2) = u32StoreSC[4]; //Set VOP "New Blending"
#endif
    //REG_8(REG_SC_BASE) = 0x00; //Switch to bank0
    //REG(REG_SC_BASE + 0x5*4) = u32StoreSC[5];
    //REG(REG_SC_BASE + 0x6*4) =u32StoreSC[6];
}

//-------------------------------------------------------------------------------------------------
// Backup Gop register to buffer, now only support Gop1
// @param eGOP_Type     \b IN: GOP HW type select
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_InfoBackup(GOP_HW_Type eGOP_Type)
{
    MHal_GOP_Reg_Store();

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Restore Gop register from buffer, now only support Gop1
// @param eGOP_Type     \b IN: GOP HW type select
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_InfoRestore(GOP_HW_Type eGOP_Type)
{
    MHal_GOP_Reg_Restore();

    return TRUE;

}

#if 0
//-------------------------------------------------------------------------------------------------
// Get a palette entry for Blink, I8(8-bit palette) format.
// @param u8GOP_num \b IN: GOP number: 0 or 1
// @param pPalEntry \b OUT: ptr to palette data
// @param u32Index   \b IN: Palette Index, (256 entries: 0-255)
// @param ePalType  \b IN: palette type (Blink format must set it as E_GOP_PAL_RGB888)
// @note: GOP palette is single port SRAM, You must get palettes before you enable Gwin which needs to read palette table.
// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
B16 MHal_GOP_Palette_Get( U8 u8GOP_num, GopPaletteEntry *pPalEntry, U32 u32Index, GopPalType ePalType)
{
    U32 u32RegVal,u32RegVal_2;
    U32 u32OldIntr;


    if (u8GOP_num>= MAX_GOP_SUPPORT)
    {
        return FALSE;
    }

    if(u32Index >= GOP_PALETTE_ENTRY_NUM)
    {
        return FALSE;
    }

    u32OldIntr = MsOS_DisableAllInterrupts();

    //Set PAL INDEX
    u32RegVal = 0;
    ((GOP_pal_data_hi*)&u32RegVal)->PAL_ADDR = u32Index;
    REG(REG_GOP_PAL_DATA_HI(u8GOP_num)) = u32RegVal;

    // Write PAL_WRITE
    u32RegVal = 0;
    ((GOP_pal_set*)&u32RegVal)->PAL_READ = 1;
    ((GOP_pal_set*)&u32RegVal)->PAL_TYPE = (U32)ePalType;
    REG(REG_GOP_PAL_SET(u8GOP_num)) = u32RegVal;

    // Get PAL DATA
    u32RegVal = REG(REG_GOP_PAL_DATA_LO(u8GOP_num));
    u32RegVal_2 = REG(REG_GOP_PAL_DATA_HI(u8GOP_num));
    pPalEntry->u32Data = (u32RegVal & GOP_PAL_DATA_LO_MASK) | ((((GOP_pal_data_hi*)&u32RegVal_2)->PAL_DATA_HI)<<16);

    // Clear PAL_WRITE by CPU
    u32RegVal = 0;
    ((GOP_pal_set*)&u32RegVal)->PAL_READ = 0;
    ((GOP_pal_set*)&u32RegVal)->PAL_TYPE = (U32)ePalType;
    REG(REG_GOP_PAL_SET(u8GOP_num)) = u32RegVal;

    MsOS_RestoreAllInterrupts(u32OldIntr);


    return TRUE;
}
#endif

static U32 vcom_active = 0 ; // 0:disabled, 1:enabled
static U32 vcom_store_h_offset ;
static U32 vcom_store_v_offset ;

static U32 vcom_store_hsize ;
static U32 vcom_store_vsize ;
static U32 vcom_store_hratio ;
static U32 vcom_store_vratio ;
static U32 vcom_store_transparent_color ;

#define VCOM_PATT_GOP_ID    0
#define VCOM_PATT_GWIN_ID   0
#define VCOM_SHOW_GWIN_ID   3

#define _GFXOSD_CALC_STRIDE(bpl)		(((bpl)+15)&0xfffffff0)


void MHal_GOP_VCOM_Pattern(
    U32 active, U32 green_level, U32 buf_addr,
    U32 h_offset, U32 v_offset,
    U32 width, U32 height ){

    U32 stride = _GFXOSD_CALC_STRIDE( (width<<2) ) ;
    U32 u32DRAMRBlkHSize = stride>>2 ;
    U32 u32DRAMRBlkVSize = height ;
    U32 u32ColorDepth = 4 ;
    U32 u32RegVal ;

    if( vcom_active==active && !active )
        return ;

    // set to update right away
    REG(REG_GOP_WR_ACK) |= 0x200 ;

    // activate
    if( active ){

        // set H/V offset
        _MHal_GOP_Select( VCOM_PATT_GOP_ID, 1 ) ; // select BANK 0
        vcom_store_h_offset = REG(REG_GOP_STRCH_HSTART) ;
        vcom_store_v_offset = REG(REG_GOP_STRCH_VSTART) ;
        REG(REG_GOP_STRCH_HSTART) = h_offset ;
        REG(REG_GOP_STRCH_VSTART) = v_offset ;

        // save transparent color
        vcom_store_transparent_color = (REG(REG_GOP_SOFT_RESET)&(1<<11)) ;

        // set stretch window
        if( vcom_active != active ){
            // store value.
            vcom_store_hratio = REG(REG_GOP_STRCH_HRATIO) ;
            vcom_store_vratio = REG(REG_GOP_STRCH_VRATIO) ;
            vcom_store_hsize = REG(REG_GOP_STRCH_HSIZE) ;
            vcom_store_vsize = REG(REG_GOP_STRCH_VSIZE);
        }
        REG(REG_GOP_STRCH_HRATIO) = 0x1000 ;
        REG(REG_GOP_STRCH_VRATIO) = 0x1000 ;
        REG(REG_GOP_STRCH_HSIZE) = width>>1;
        REG(REG_GOP_STRCH_VSIZE) = height;
        REG(REG_GOP_RDMA_HT) = (width>>1) + 1;

        // set Buffer address
        _MHal_GOP_Select( VCOM_PATT_GOP_ID, 0 ) ; // select BANK 1
        buf_addr >>= GOP_ALIGN_RSHIFT ;
        REG(REG_GOP_DRAM_RBLK_STR_LO(VCOM_PATT_GWIN_ID)) = LO16BIT(buf_addr) ;
        REG(REG_GOP_DRAM_RBLK_STR_HI(VCOM_PATT_GWIN_ID)) = HI16BIT(buf_addr) ;
        buf_addr <<= GOP_ALIGN_RSHIFT ;

        // draw pattern into buffer
        {
            U32 x, y, idx, cnt ;
            U8* pu8 ;
            pu8 = (U8*)(0xA0000000|buf_addr) ;
            cnt = idx = 0 ;
            for( y=0; y<height; y++ ){
                for( x=0; x<width; x++ ){
                    // format=BGRA
                    pu8[idx+(x<<2)] = 0 ;
                    if( cnt&1 )
                        pu8[idx+(x<<2)+1] = green_level ; // color dot
                    else
                        pu8[idx+(x<<2)+1] = 0 ; // black dot
                    pu8[idx+(x<<2)+2] = 0 ;
                    pu8[idx+(x<<2)+3] = 0xFF ;
                    cnt++ ;
                }
                cnt++ ; // shift one dot
                idx += stride ;
            }
        }

        // set Buffer size
        u32RegVal = u32DRAMRBlkHSize * u32DRAMRBlkVSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
        REG(REG_GOP_DRAM_RBLK_SIZE_LO(VCOM_PATT_GWIN_ID)) = LO16BIT(u32RegVal);
        REG(REG_GOP_DRAM_RBLK_SIZE_HI(VCOM_PATT_GWIN_ID)) = HI16BIT(u32RegVal);
        u32RegVal = u32DRAMRBlkHSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
        REG(REG_GOP_DRAM_RBLK_HSIZE(VCOM_PATT_GWIN_ID)) = LO16BIT(u32RegVal);

        // set W/H
        REG(REG_GOP_GWIN_VSTR(VCOM_PATT_GWIN_ID)) = 0 ; //in pixel
        REG(REG_GOP_GWIN_VEND(VCOM_PATT_GWIN_ID)) = height;//in pixel
        REG(REG_GOP_GWIN_HSTR(VCOM_PATT_GWIN_ID)) = 0 >> GOP_ALIGN_RSHIFT; //in HW alignment
        REG(REG_GOP_GWIN_HEND(VCOM_PATT_GWIN_ID)) = (width) * u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment

        // set V/H start point
        u32RegVal = 0 * u32DRAMRBlkHSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
        REG(REG_GOP_DRAM_GWIN_VSTR_LO(VCOM_PATT_GWIN_ID)) = LO16BIT(u32RegVal);
        REG(REG_GOP_DRAM_GWIN_VSTR_HI(VCOM_PATT_GWIN_ID)) = HI16BIT(u32RegVal);
        u32RegVal = 0 * u32ColorDepth >> GOP_ALIGN_RSHIFT;
        REG(REG_GOP_DRAM_GWIN_HSTR(VCOM_PATT_GWIN_ID)) = LO16BIT(u32RegVal);

        // set no transparent color
        REG(REG_GOP_SOFT_RESET) &= ~(1<<11) ;

        REG(REG_GOP_GWIN_SET(VCOM_SHOW_GWIN_ID)) &= ~0x1 ; // turn off

        // turn on and copy SHOW GWIN
        u32RegVal = REG(REG_GOP_GWIN_SET(VCOM_SHOW_GWIN_ID)) ;
        u32RegVal &= ~0xF0 ;
        u32RegVal |= 0x50 ; // ARGB8888 = 5
        REG(REG_GOP_GWIN_SET(VCOM_PATT_GWIN_ID)) = u32RegVal|0x1 ;

    }else{

        // deactivate
        _MHal_GOP_Select( VCOM_PATT_GOP_ID, 0 ) ;
        REG(REG_GOP_GWIN_SET(VCOM_PATT_GWIN_ID)) &= ~0x1 ; // turn off
        REG(REG_GOP_GWIN_SET(VCOM_SHOW_GWIN_ID)) |= 0x1 ; // turn on

        _MHal_GOP_Select( VCOM_PATT_GOP_ID, 1 ) ;
        REG(REG_GOP_STRCH_HSTART) = vcom_store_h_offset ;
        REG(REG_GOP_STRCH_VSTART) = vcom_store_v_offset ;

        REG(REG_GOP_STRCH_HRATIO) = vcom_store_hratio ;
        REG(REG_GOP_STRCH_VRATIO) = vcom_store_vratio ;
        REG(REG_GOP_STRCH_HSIZE) = vcom_store_hsize;
        REG(REG_GOP_STRCH_VSIZE) = vcom_store_vsize;
        REG(REG_GOP_RDMA_HT) = vcom_store_hsize + 1;

        // restore transparent color
        REG(REG_GOP_SOFT_RESET) |= vcom_store_transparent_color ;
    }

    vcom_active = active ;

    // set to update later
    REG(REG_GOP_WR_ACK) &= ~0x200 ;

}

// Reset GOP
void MHal_GOP_ResetGOP(void)
{
    printk("REG_MIPS_BASE 0x%08X\n",REG_MIPS_BASE);
    (*(char*)(REG_MIPS_BASE + 0x1D80)) =	(*(char*)(REG_MIPS_BASE + 0x1D80)) | 1;
    (*(unsigned short*)(REG_MIPS_BASE + 0x1DA8)) = 0xFFFF;
}
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
//-------------------------------------------------------------------------------------------------
/// Set Flip Info
/// @param u32Addr       \b IN: the flip address fired to vsync. interrupt
/// @param u32TagId        \b IN: The TagId must be waited before fire flip
/// @param u32QEntry     \b IN: the frame queue number required by ap.
/// @param u32QEntry     \b OUT: return the flip info queued.
/// @return TRUE: sucess / FALSE: fail to set, the queue if full.
///
//-------------------------------------------------------------------------------------------------
B16 MAdp_GOP_SetFlipInfo(U32 u32GopIdx, U32 u32GwinIdx, U32 u32Addr, U32 u32TagId, U32 *u32QEntry)
{
    MS_GOP_FLIP_INFO stFlinInfo;

    stFlinInfo.u32GopIdx = u32GopIdx;
    stFlinInfo.u32GwinIdx = u32GwinIdx;
    stFlinInfo.u32Addr= (U32)VA2PA((void *)u32Addr);
    stFlinInfo.u32TagId= u32TagId;
    stFlinInfo.u32QEntry= *u32QEntry;
    stFlinInfo.u32Result = TRUE;

    if (ioctl(_fd_GOP, MDRV_GOP_IOC_SET_FLIP_INFO, &stFlinInfo))
    {
    	printk("MAdp_GOP_SetFlipInfo fail!!!!\n");
        return FALSE;
    }

    *u32QEntry = stFlinInfo.u32QEntry;

    return stFlinInfo.u32Result;
}
#endif

B16 MHal_GOP_VMirror_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_VMirror_Enable(eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    sGOP_Mirror.bVMirror = bEnable;

    _MHal_GOP_UpdateReg(eGOP_Type);
    if (bEnable == FALSE)
    {
        sGOP_Mirror.u16GOP_HasRBLKVMirror_Adr = 0;
        sGOP_Mirror.u16GOP_NoRBLKVMirror_Adr = 0;
        sGOP_Mirror.u16GOP_VMirror_VPos = 0;
        sGOP_Mirror.bVMirror = FALSE;
    }

    return bRet;

}

B16 MHal_GOP_HMirror_Enable(GOP_HW_Type eGOP_Type, U16 bEnable)
{
    U16 bRet;

#ifdef RED_LION
    //local_irq_save(u32OldIntr);
#else
    U32 u32OldIntr;
    u32OldIntr = MsOS_DisableAllInterrupts();
#endif

    bRet = _MHal_GOP_HMirror_Enable(eGOP_Type, bEnable);

#ifdef RED_LION
    //local_irq_restore(u32OldIntr);
#else
    MsOS_RestoreAllInterrupts(u32OldIntr);
#endif

    sGOP_Mirror.bHMirror = bEnable;
    _MHal_GOP_UpdateReg(eGOP_Type);

    if (bEnable == FALSE)
    {
        sGOP_Mirror.u16GOP_NoRBLKHMirror_Adr = 0;
        sGOP_Mirror.u16GOP_HMirror_HPos = 0;
        sGOP_Mirror.bHMirror = FALSE;
    }
    return bRet;

}

/******************************************************************************/
// Gop scrolling related hardware setting for LG app true motion demo 
// @param bTure
//   - TRUE: start LG true motion demo
//   - FALSE: stop LG true motion demo
/******************************************************************************/
B16 MHal_GOP_TrueMotionDemo(B16 bTure)
{
    U8 i;
    U32 gop2Miu;
    _TRUE_MOTION_INFO Gop2Info[E_REG_MAX];
    
    if (bTure)
    {
        /*store gop0 reg*/
        _MHal_GOP_Select(E_GOP4G_0, TRUE);    
        _TrueMotionInfo[E_REG_HTOTAL] = REG(REG_GOP_RDMA_HT);
        _TrueMotionInfo[E_REG_STRWIN_HSIZE] = REG(REG_GOP_STRCH_HSIZE);
        _TrueMotionInfo[E_REG_STRWIN_VSIZE] = REG(REG_GOP_STRCH_VSIZE);
        _TrueMotionInfo[E_REG_STRWIN_HRATIO] = REG(REG_GOP_STRCH_HRATIO);
        _TrueMotionInfo[E_REG_STRWIN_VRATIO] = REG(REG_GOP_STRCH_VRATIO);
        _MHal_GOP_Select(E_GOP4G_0, FALSE);    
        _TrueMotionInfo[E_REG_GOP0_GWIN0_ENABLE] = REG(REG_GOP_GWIN0_EN);
        _TrueMotionInfo[E_REG_GOP0_GWIN1_ENABLE] = REG(REG_GOP_GWIN1_EN);
        _TrueMotionInfo[E_REG_GOP0_GWIN2_ENABLE] = REG(REG_GOP_GWIN2_EN);
        _TrueMotionInfo[E_REG_GOP0_GWIN3_ENABLE] = REG(REG_GOP_GWIN3_EN);
        _TrueMotionInfo[E_REG_GOP_MUX] = REG(REG_GOP_MUX_SEL);
        _TrueMotionInfo[E_REG_GOP_MIUSEL] = REG(REG_GOP_MIUSEL);

        /*get gop2 info*/
        _MHal_GOP_Select(E_GOP1G, TRUE);    
        Gop2Info[E_REG_HTOTAL] = REG(REG_GOP_RDMA_HT);
        Gop2Info[E_REG_STRWIN_HSIZE] = REG(REG_GOP_STRCH_HSIZE);
        Gop2Info[E_REG_STRWIN_VSIZE] = REG(REG_GOP_STRCH_VSIZE);
        Gop2Info[E_REG_STRWIN_HRATIO] = REG(REG_GOP_STRCH_HRATIO);
        Gop2Info[E_REG_STRWIN_VRATIO] = REG(REG_GOP_STRCH_VRATIO);

        /*disable all gwin of gop0 for avoid tricky approach caused another side effect*/
        for (i=0; i<MAX_GOP0_GWIN_SUPPORT; i++)
        {
            _MHal_GOP_GWIN_Enable(E_GOP4G_0, i, FALSE);
        }

        /*set gop2 related hardware setting to top0*/
        _MHal_GOP_Select(E_GOP4G_0, TRUE);  
        REG(REG_GOP_RDMA_HT) = Gop2Info[E_REG_HTOTAL];
        REG(REG_GOP_STRCH_HSIZE) = Gop2Info[E_REG_STRWIN_HSIZE];
        REG(REG_GOP_STRCH_VSIZE) = Gop2Info[E_REG_STRWIN_VSIZE];
        REG(REG_GOP_STRCH_HRATIO) = Gop2Info[E_REG_STRWIN_HRATIO];
        REG(REG_GOP_STRCH_VRATIO) = Gop2Info[E_REG_STRWIN_VRATIO];
       
        /*update reg in next v-sync*/
        _MHal_GOP_UpdateReg(E_GOP4G_0);

        /*config gop mux and miu sel*/
        ((GOP_mux_sel*)REG_GOP_MUX_SEL)->GOPG2_MUX_SEL = 2;
        ((GOP_mux_sel*)REG_GOP_MUX_SEL)->GOPG3_MUX_SEL = 0;
        gop2Miu = ((GOP_MIU_SEL*)REG_GOP_MIUSEL)->gop2_miusel;
        ((GOP_MIU_SEL*)REG_GOP_MIUSEL)->gop0_miusel = gop2Miu;
    }
    else
    {
        /*restore gop0 reg*/
        _MHal_GOP_Select(E_GOP4G_0, TRUE);    
        REG(REG_GOP_RDMA_HT) = _TrueMotionInfo[E_REG_HTOTAL];
        REG(REG_GOP_STRCH_HSIZE) = _TrueMotionInfo[E_REG_STRWIN_HSIZE];
        REG(REG_GOP_STRCH_VSIZE) = _TrueMotionInfo[E_REG_STRWIN_VSIZE];
        REG(REG_GOP_STRCH_HRATIO) = _TrueMotionInfo[E_REG_STRWIN_HRATIO];
        REG(REG_GOP_STRCH_VRATIO) = _TrueMotionInfo[E_REG_STRWIN_VRATIO];
        _MHal_GOP_Select(E_GOP4G_0, FALSE);    
        REG(REG_GOP_GWIN0_EN) = _TrueMotionInfo[E_REG_GOP0_GWIN0_ENABLE];
        REG(REG_GOP_GWIN1_EN) = _TrueMotionInfo[E_REG_GOP0_GWIN1_ENABLE];
        REG(REG_GOP_GWIN2_EN) = _TrueMotionInfo[E_REG_GOP0_GWIN2_ENABLE];
        REG(REG_GOP_GWIN3_EN) = _TrueMotionInfo[E_REG_GOP0_GWIN3_ENABLE];
        REG(REG_GOP_MUX_SEL) = _TrueMotionInfo[E_REG_GOP_MUX];
        REG(REG_GOP_MIUSEL) = _TrueMotionInfo[E_REG_GOP_MIUSEL];
        _MHal_GOP_UpdateReg(E_GOP4G_0);
    }
    
    return TRUE;
}
