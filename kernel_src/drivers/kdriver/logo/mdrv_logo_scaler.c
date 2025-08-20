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

#include <linux/string.h>
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_logo_scaler.h"
//-------------------------------------------------------------------------------------------------
// Panel
//-------------------------------------------------------------------------------------------------
MST_PANEL_INFO_t pnl_1366x768 =
{
    .u16HStart = 60,
    .u16VStart = 38,
    .u16Width  = 1366,
    .u16Height  = 768,
    .u16HTotal = 1560,
    .u16VTotal = 806,

    .u16DE_VStart = 0,

    .u16DefaultVFreq = 600,

    // LPLL
    .u16LPLL_InputDiv  = 0x0000,
    .u16LPLL_LoopDiv   = 0x0203,    // for T3 setting
    .u16LPLL_OutputDiv = 0x0000,

    .u8LPLL_Type = 0,    // 0: LVDS, 1: RSDS, 2: TTL
    .u8LPLL_Mode = 0,    // 0: Single mode, 1: Dual mode

    // sync
    .u8HSyncWidth = 0x14,

    // output control
    .u16OCTRL    = 0x0000,
    .u16OSTRL    = 0x4000,
    .u16ODRV     = 0x0055,
    .u16DITHCTRL = 0x00,

    // MOD
    .u16MOD_CTRL0 = 0x002C,
	.u16MOD_CTRL9 = 0x0000,
    .u16MOD_CTRLA = 0x0100,
	.u8MOD_CTRLB  = 0x02,

	//Panel Option(LCD : 0, PDP : 1)
	.u8LCDorPDP = 0,	// LCD : 0, PDP : 1, LCD_NO_FRC: 2
	.u32LimitD5d6d7 = 0x00010000,
	.u16LimitOffset = 0xFFF0,
};

MST_PANEL_INFO_t pnl_1920x1080 =
{
	.u16HStart = 112,
    .u16VStart = 24,
    .u16Width  = 1920,
    .u16Height  = 1080,
    .u16HTotal = 2199,
    .u16VTotal = 1125,

    .u16DE_VStart = 0,

    .u16DefaultVFreq = 600,

    // LPLL
    .u16LPLL_InputDiv  = 0x0000,
    .u16LPLL_LoopDiv   = 0x0203,    // for T3 setting
    .u16LPLL_OutputDiv = 0x0000,

    .u8LPLL_Type = 0,    // 0: LVDS, 1: RSDS, 2: TTL
    .u8LPLL_Mode = 1,    // 0: Single mode, 1: Dual mode

    // sync
    .u8HSyncWidth = 0x20,
    .bPnlDblVSync = 0,

    // output control
    .u16OCTRL    = 0x0100,
    .u16OSTRL    = 0x4000,
    .u16ODRV     = 0x0055,
    .u16DITHCTRL = 0x2D00,

    // MOD
    .u16MOD_CTRL0 = 0x002C,
	.u16MOD_CTRL9 = 0x0000,
    .u16MOD_CTRLA = 0x0003,
	.u8MOD_CTRLB  = 0x00,	//lachesis_090717 10bit

	//Panel Option(LCD : 0, PDP : 1)
	.u8LCDorPDP = 0,	// LCD : 0, PDP : 1, LCD_NO_FRC: 2
	.u32LimitD5d6d7 = 0x00010000,
	.u16LimitOffset = 0xFFF0,
};

static void mstar_msleep( unsigned int ms )
{
    U32 ret_timer = 0xFF;

    while (ret_timer)
    {
        ret_timer--;
    }
}

void MDrv_Logo_SC_Init(PANEL_RESOLUTION_TYPE_e e_panel_type, SC_MOD_LPLL_TYPE_e e_lpll_type)
{
    MST_PANEL_INFO_t panelInfo;
    U32 memaddr = 0x3200000 ; // dram start address
    U32 memsize = 0xC00000 ;  // buffer length
    U16 u16Reg_03 = 0x0003;
    U32 u32DClk;
    U32 u32DClkFactor;      // 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain

    if(e_panel_type == PANEL_RES_FULL_HD)
    {
        //panelInfo = pnl_1920x1080;
        memcpy(&panelInfo, &pnl_1920x1080, sizeof(MST_PANEL_INFO_t));

    }
    else // if(e_panel_type == PANEL_RES_WXGA)
    {
        //panelInfo = pnl_1366x768;
        memcpy(&panelInfo, &pnl_1366x768, sizeof(MST_PANEL_INFO_t));
    }

    if(e_lpll_type == LPLL_LVDS)
    {
        panelInfo.u8LPLL_Type = LPLL_LVDS;
    }
    else
    {
        panelInfo.u8LPLL_Type = LPLL_TTL;
    }

    //
    //MHal_SC_Reset(SC_RST_ALL);
    //-----------------------------------------------------------------
    SC_BK_STORE;

    SC_BK_SWICH(REG_SC_BK_GOPINT);

    REG_WL(REG_SC_GOPINT(0x02), 0x01);

    mstar_msleep(1);

    REG_WL(REG_SC_GOPINT(0x02), 0x00);

    // GOP
    //MHal_SC_GOPINT_SetGOPEnable(TRUE);
    //-----------------------------------------------------------------
    REG_WM(REG_SC_GOPINT(0x06), 0xE000, 0xE000);


    //
    //MHal_SC_RegInit(pDrvCtx->u32MemAddr, pDrvCtx->u32MemSize);
    //-----------------------------------------------------------------

    // DNR memory base address
    SC_BK_SWICH(REG_SC_BK_SCMI);
    REG_W3(REG_SC_SCMI(0x08), memaddr>>3);
    REG_WR(REG_SC_SCMI(0x0A), (memaddr + memsize) / 2);
    REG_WR(REG_SC_SCMI(0x0B), ((memaddr + memsize) / 2) >> 16);
    REG_WR(REG_SC_SCMI(0x0E), 0x0400);
    REG_WR(REG_SC_SCMI(0x0F), 0x02D0);

    // F2 DNR write limit
    //MHal_SC_IPM_SetDNRWriteLimit(pDrvCtx->u32MemAddr, pDrvCtx->u32MemSize);
    //-----------------------------------------------------------------
    SC_BK_SWICH(REG_SC_BK_SCMI);
    REG_W4(REG_SC_SCMI(0x1A), (((memaddr+memsize)>>3)-1) | 0x02000000);    // 0x02000000 : write limit enable


    // output timing
    //
    //MHal_SC_VOP_HVTotalSet(pDrvCtx->pPanelInfo->u16HTotal, pDrvCtx->pPanelInfo->u16VTotal);
    //-----------------------------------------------------------------
    SC_BK_SWICH(REG_SC_BK_VOP);
    REG_WR(REG_SC_VOP(0x0C), panelInfo.u16HTotal & 0xffff);
    REG_WR(REG_SC_VOP(0x0D), panelInfo.u16VTotal & 0xffff);

    //
    //MHal_SC_VOP_HSyncWidthSet(pDrvCtx->pPanelInfo->u8HSyncWidth);
    //-----------------------------------------------------------------
    REG_WL(REG_SC_VOP(0x01), panelInfo.u8HSyncWidth & 0xfffe);


    // DE window
    //MHal_SC_VOP_SetDEWin(
    //    pDrvCtx->pPanelInfo->u16HStart,
    //    pDrvCtx->pPanelInfo->u16HStart + pDrvCtx->pPanelInfo->u16Width - 1,
    //    0,
    //    pDrvCtx->pPanelInfo->u16Height - 1);
    //-----------------------------------------------------------------
    REG_WR(REG_SC_VOP(0x04), panelInfo.u16HStart);
    REG_WR(REG_SC_VOP(0x05), (panelInfo.u16HStart+panelInfo.u16Width-1));
    REG_WR(REG_SC_VOP(0x06), 0);
    REG_WR(REG_SC_VOP(0x07), (panelInfo.u16Height-1));

    // display window
    REG_WR(REG_SC_VOP(0x08), panelInfo.u16HStart);
    REG_WR(REG_SC_VOP(0x09), (panelInfo.u16HStart+panelInfo.u16Width-1));
    REG_WR(REG_SC_VOP(0x0A), 0);
    REG_WR(REG_SC_VOP(0x0B), (panelInfo.u16Height-1));

//    REG_WR(REG_SC_VOP(0x24), (3));



    //REG_WR(REG_SC_VOP(0x24), (3));

    //
    //MHal_SC_OP2_SetOSDBlending(TRUE);
    //-----------------------------------------------------------------
    REG_WM(REG_SC_VOP(0x23), 0x00A0, 0x00A0);

    //
    //MDrv_SC_SetPanelOutput
    //-----------------------------------------------------------------

    if (panelInfo.u8LPLL_Type == LPLL_LVDS)
    {
        REG_WM(REG_SC_VOP(0x22), G_SC_ENCLK_SIGNAL_LVDS, 0x1F);
    }
    else
    {
        REG_WM(REG_SC_VOP(0x22), G_SC_ENCLK_TTL, 0x1F);
    }
    SC_BK_RESTORE;

    //
    //MHal_SC_SetClk();
    //-----------------------------------------------------------------
    {
    REG_WM(REG_CKGEN0(0x52), 0x0C00, 0x3C00);  // fclk  <- select 216MHz daniel.huang //170 MHz (MPLL_DIV_BUF), 20090619 daniel.huang
    REG_WL(REG_CKGEN0(0x53), 0x1C);            // odclk <- XTAL
    }

    // LPLL setting
    // LPLL init
    //MHal_LPLL_Init(&lpllInit);
    //-----------------------------------------------------------------

    REG_WR(REG_LPLL(0x00), 0x0000 ); // u16InputDiv
    REG_WR(REG_LPLL(0x01), 0x0203 ); // u16LoopDiv   // for T3 setting
    REG_WR(REG_LPLL(0x02), 0x0000 ); // u16OutputDiv

    if (panelInfo.u8LPLL_Type == LPLL_RSDS)
    {
        // RSDS type
        u16Reg_03 |= 0x40;
    }

    if (panelInfo.u8LPLL_Mode == G_LPLL_MODE_DUAL)
    {
        // dual mode
        u16Reg_03 |= 0x80;
    }

    REG_WR(REG_LPLL(0x03), u16Reg_03);

    // register init
    REG_WR(REG_LPLL(0x06), 0x0000); // limit_d5d6d7
    REG_WR(REG_LPLL(0x07), 0x0001);
    REG_WR(REG_LPLL(0x08), 0x0000); // limit_d5d6d7_RK
    REG_WR(REG_LPLL(0x09), 0x0001);
    REG_WR(REG_LPLL(0x0A), 0xFFF0); // limit_lpll_offset //thchen 20080902
    REG_WR(REG_LPLL(0x0D), 0x0701);

    REG_WR(REG_LPLL(0x0B), 0x7600);
    REG_WR(REG_LPLL(0x1D), 0x000B);
    REG_WM(REG_LPLL(0x1C), 0x0000, 0xFF00);
    REG_WM(REG_LPLL(0x1E), 0x0040, 0x00FF);

    //
    //MDrv_SC_LPLL_SetODClk(pDrvCtx, pDrvCtx->pPanelInfo->u16DefaultVFreq);
    //-----------------------------------------------------------------
#if 1   // 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain

    u32DClk = (((U32)panelInfo.u16HTotal * (U32)panelInfo.u16VTotal) / 1000 * panelInfo.u16DefaultVFreq / 1000) / 10;   

    if (panelInfo.u8LPLL_Type == LPLL_MINILVDS)
    {
        u32DClkFactor = 216*524288*2;
    }
    else
    {
        u32DClkFactor = 216*524288*16;
    }

    if (panelInfo.u8LPLL_Type == LPLL_LVDS || panelInfo.u8LPLL_Type == LPLL_TTL)
    {
        u32DClk = (u32DClkFactor / u32DClk) / 7;
        if (panelInfo.u8LPLL_Mode == G_LPLL_MODE_DUAL)
        {
            u32DClk *= 2;
        }
    }
    else if (panelInfo.u8LPLL_Type == LPLL_RSDS)
    {
        u32DClk = u32DClkFactor / (u32DClk * 4);
    }
    else // MINILVDS
    {
        u32DClk = u32DClkFactor / u32DClk;
    }
#else
    u32DClk = (((U32)panelInfo.u16HTotal * (U32)panelInfo.u16VTotal) / 1000 * panelInfo.u16DefaultVFreq / 1000) / 10;

    if (panelInfo.u8LPLL_Type == LPLL_LVDS)
    {
        u32DClk = (panelInfo.u32DClkFactor / u32DClk) / 7;
        if (panelInfo.u8LPLL_Mode == G_LPLL_MODE_DUAL)
        {
            u32DClk *= 2;
        }
    }
    else
    {
        u32DClk = panelInfo.u32DClkFactor / (u32DClk * 4);
    }
#endif
    //MHal_LPLL_EnableFPLL(DISABLE);
    REG_WI(REG_LPLL(0x0C), 0, 0x08); // bit3

    //MHal_LPLL_LPLLSET(u32DClk);
    REG_WR(REG_LPLL(0x0F), (U16)(u32DClk & 0xFFFF));
    REG_WR(REG_LPLL(0x10), (U16)(u32DClk>>16));

    // MOD setting

    //MDrv_SC_SetMODPower(pDrvCtx, bEnable);

    //MHal_LPLL_MODSET(pDrvCtx->pPanelInfo->u8LPLL_Type, pDrvCtx->pPanelInfo->u8LPLL_Mode);
    //.u8LPLL_Type = 0,    // 0: LVDS, 1: RSDS, 2: TTL
    //.u8LPLL_Mode = 1,    // 0: Single mode, 1: Dual mode
    REG_WM(REG_LPLL(0x03), ((panelInfo.u8LPLL_Mode << 7) | (panelInfo.u8LPLL_Type << 6)), 0xC0);

    //MHal_MOD_SetPower(TRUE);
    MOD_BK_STORE;
    MOD_BK_SWICH(REG_MOD_BK_00);
    REG_WM(MOD_REG(0x37), 0, 0x100); // for T3

    //MHal_MOD_Init(pDrvCtx->pPanelInfo);
    REG_WM(MOD_REG(0x20), 0, 0xFF00); // set to 00 for LVDS out, 44 to mini LVDS out

    // H32[1:0] :  This is to open dynamic getting for power saving. Normally, you could close it first( = 0).
    REG_WM(MOD_REG(0x32), 0, 0x3);

    REG_WR(MOD_REG(0x40), panelInfo.u16MOD_CTRL0);

    //EXT GPO disable
    REG_WR(MOD_REG(0x46), 0x00);
    REG_WR(MOD_REG(0x47), 0x00);

    //PANEL_SWAP_EVEN_ML:14, PANEL_SWAP_EVEN_RB:13, PANEL_SWAP_ODD_ML:12, PANEL_SWAP_ODD_RB:11
	// [7,6] : output formate selction 10: 8bit, 01: 6bit :other 10bit
	REG_WR(MOD_REG(0x49), panelInfo.u16MOD_CTRL9);


	//[1:0]ti_bitmode=00(10bit)
	REG_WR(MOD_REG(0x4B), panelInfo.u8MOD_CTRLB);


    //REG_WR(MOD_REG(0x6F), 0x5551);
    //REG_WL(MOD_REG(0x70), 0x05);
    //enable mod_atop IB,CLK
    REG_WL(MOD_REG(0x77), 0x0F);
    //[0]enable mod_atop da bias, differential output swing level
    // REG_WL(MOD_REG(0x78), 0x0D);
    REG_WL(MOD_REG(0x78), 0x4C);
    //[6]disable power down bit and [5:0]enable all channel
    REG_WL(MOD_REG(0x7D), 0x1F);

    if(panelInfo.u8LPLL_Type == LPLL_TTL)
    {
        // Init TTL TX setting
        REG_WR(MOD_REG(0x6D), 0x00);
        REG_WR(MOD_REG(0x6E), 0x00);
        REG_WR(MOD_REG(0x6F), 0x00);
        REG_WR(MOD_REG(0x45), 0x00);
        REG_WM(MOD_REG(0x7E), 0x0000, 0xF000);
        REG_WM(MOD_REG(0x6C), 0x8000, 0x8000);
        REG_WM(MOD_REG(0x4A), 0x0110, 0x0113);
        REG_WM(MOD_REG(0x78), 0x01, 0x01);
        REG_WL(MOD_REG(0x42), 0x80);

    }
    else if(panelInfo.u8LPLL_Type == LPLL_LVDS)
    {
        //dual port lvds _start_//
        // output configure for 26 pair output 00: TTL, 01: LVDS/RSDS/mini-LVDS data differential pair, 10: mini-LVDS clock output, 11: RSDS clock output
        REG_WR(MOD_REG(0x6D), 0x5555);
        REG_WR(MOD_REG(0x6E), 0x5555);
        REG_WL(MOD_REG(0x6F), 0x55);
        //MHal_MOD_SetOEZ(MOD_OEZ_ALL);//0x3F
        //[5:4] Output enable: PANEL_LVDS/ PANEL_miniLVDS/ PANEL_RSDS
        REG_WM(MOD_REG(0x45), 0x3F, 0x3F);
        //PANEL_INV_HSYNC:12, PANEL_INV_DCLK:4, PANEL_INV_VSYNC:3, PANEL_INV_DE:2, PANEL_DUAL_PORT:1, PANEL_SWAP_PORT:0,
        REG_WR(MOD_REG(0x4A), panelInfo.u16MOD_CTRLA);
        REG_WI(MOD_REG(0x78), 0,  0x01);
        REG_WL(MOD_REG(0x42), 0x00);
    }
    MOD_BK_RESTORE;

}


