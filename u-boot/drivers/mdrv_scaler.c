////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¨MStar Confidential Information〃) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#include "mdrv_scaler.h"
#include "mdrv_tcon.h"
#include "mdrv_tcon_tbl.h"
#include "nvm.h"

#define mdelay(n)						udelay((n)*1000)
typedef enum{
	FRC_OPT_SEL,
	FRC_OPT1_SEL,
	LVDS_OPT_SEL,
	DDRSIZE_OPT_SEL,
	PANEL_RES_OPT_SEL,
	GIP_OPT_SEL,
	OLED_SEL,
	MICOM_HW_OPT_SEL,
}HW_OPT_T;
extern U8 Splash_GetHWoption(HW_OPT_T option_mask);

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
	.u16VTotal60Hz = 806,
	.u16VTotal50Hz = 806,//lachesis_090717
	.u16VTotal48Hz = 806,
	.u16VStart60Hz = 0,
	.u16VStart48Hz = 0,
	.u16VBackPorch60Hz = 0,
	.u16VBackPorch50Hz = 0,
	.u16VBackPorch48Hz = 0,

	//Panel Option(LCD : 0, PDP : 1)
	.u8LCDorPDP = 0,	// LCD : 0, PDP : 1, LCD_NO_FRC: 2
	.u32LimitD5d6d7 = 0x00010000,
	.u16LimitOffset = 0xFFF0,
	.bTTL_10BIT = 1,
	.bOD_DataPath = 0,
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

	.u16VTotal60Hz = 1125,
	.u16VTotal50Hz = 1350,//lachesis_090717
	.u16VTotal48Hz = 1350,

	.u16VStart60Hz = 0,
	.u16VStart48Hz = 0,
	.u16VBackPorch60Hz = 0,
	.u16VBackPorch50Hz = 0,
	.u16VBackPorch48Hz = 0,
	//Panel Option(LCD : 0, PDP : 1)
	.u8LCDorPDP = 0,	// LCD : 0, PDP : 1, LCD_NO_FRC: 2
	.u32LimitD5d6d7 = 0x00010000,
	.u16LimitOffset = 0xFFF0,
	.bTTL_10BIT = 1,
	.bOD_DataPath = 0,
};

// shjang_090902
MST_PANEL_INFO_t pnl_1920x1080_TCON =
{
	.u16HStart = 112,
    .u16VStart = 1133,
    .u16Width  = 1920,
    .u16Height  = 1080,
    .u16HTotal = 2199,
    .u16VTotal = 1134,

    .u16DE_VStart = 43,

    .u16DefaultVFreq = 600,

    // LPLL
    .u16LPLL_InputDiv  = 0x0000,
    .u16LPLL_LoopDiv   = 0x0203,    // for T3 setting
    .u16LPLL_OutputDiv = 0x2407,

    .u8LPLL_Type = 3,    // 0: LVDS, 1: RSDS, 2: TTL, 3 :TCON
    .u8LPLL_Mode = 0,    // 0: Single mode, 1: Dual mode

    // sync
    .u8HSyncWidth = 0x20,
    .bPnlDblVSync = 0,

    // output control
    .u16OCTRL    = 0x0000,
    .u16OSTRL    = 0x4000,
    .u16ODRV     = 0x0055,
    .u16DITHCTRL = 0x2D00,

    // MOD
    .u16MOD_CTRL0 = 0x002C,
	.u16MOD_CTRL9 = 0x0002,
    .u16MOD_CTRLA = 0x0003,
	.u8MOD_CTRLB  = 0x00,	//lachesis_090717 10bit

	.u16VTotal60Hz = 1134,
	.u16VTotal50Hz = 1361,
	.u16VTotal48Hz = 1361,

	.u16VStart60Hz = 1133,	//0,
	.u16VStart50Hz = 1133,
	.u16VStart48Hz = 1133,	// 0,
	.u16VBackPorch60Hz = 1125,//24,	//0,
	.u16VBackPorch50Hz = 1125,	//0,
	.u16VBackPorch48Hz = 1125,	//0,


	//Panel Option(LCD : 0, PDP : 1)
	.u8LCDorPDP = 3,	// LCD : 0, PDP : 1, LCD_NO_FRC: 2, LCD_TTL : 3
	.u32LimitD5d6d7 = 0x00010000,
	.u16LimitOffset = 0xFFF0,
	.bTTL_10BIT = 1,
	.bOD_DataPath = 1,
};

// 091125_KimTH_01: PDP boot logo 대응
MST_PANEL_INFO_t pnl_1365x768_PDP4DVB =
{
    .u16HStart = 122,//49,	// KimTH_091026: 50T1 panel timing set
    .u16VStart = 23,//21,//26,
    .u16Width  = 1365,
    .u16Height = 768,
    .u16HTotal = 1542,
    .u16VTotal = 960,

    .u16DE_VStart = 0,

    .u16DefaultVFreq = 500,//600,

    // LPLL
    .u16LPLL_InputDiv  = 0x0000,
    .u16LPLL_LoopDiv   = 0x0203,    // for T3 setting
    .u16LPLL_OutputDiv = 0x0000,

    .u8LPLL_Type = 0,    // 0: LVDS, 1: RSDS, 2: TTL
    .u8LPLL_Mode = 0,    // 0: Single mode, 1: Dual mode

    // sync
    .u8HSyncWidth = 0x0C,//0x08,	// KimTH_091026: 50T1 panel timing set

    // output control
    .u16OCTRL    = 0x0000,
    .u16OSTRL    = 0x4000,
    .u16ODRV     = 0x0055,
    .u16DITHCTRL = 0,//0x2D00,

    // MOD
    .u16MOD_CTRL0 = 0x0008,//0x0004,
	.u16MOD_CTRL9 = 0x0000,
    .u16MOD_CTRLA = 0x1108,//0x0100,
	.u8MOD_CTRLB  = 0x02,

	.u16VTotal60Hz = 802,
	.u16VTotal50Hz = 960,
	.u16VTotal48Hz = 1002,
	.u16VStart60Hz = 21,//26,	// KimTH_091026: 50T1 panel timing set
	.u16VStart50Hz = 23,//26,	// KimTH_091026: 50T1 panel timing set
	.u16VStart48Hz = 23,//26,	// KimTH_091026: 50T1 panel timing set
	.u16VBackPorch60Hz = 18,//16,	// KimTH_091026: 50T1 panel timing set
	.u16VBackPorch50Hz = 14,//16,	// KimTH_091026: 50T1 panel timing set
	.u16VBackPorch48Hz = 14,//16,	// KimTH_091026: 50T1 panel timing set

	//Panel Option(LCD : 0, PDP : 1)
	.u8LCDorPDP = 1,	// LCD : 0, PDP : 1
	.u32LimitD5d6d7 = 0x00010000,
	.u16LimitOffset = 0xFFF0,
	.bTTL_10BIT = 1,
	.bOD_DataPath = 0,
};

// 091125_KimTH_01: PDP boot logo 대응
MST_PANEL_INFO_t pnl_1024x768_PDP4DVB =
{
    .u16HStart = 298,//48,	// KimTH_091125: PDP 42T1 panel timing set
    .u16VStart = 17,//20,	// KimTH_091125: PDP 42T1 panel timing set
    .u16Width  = 1024,
    .u16Height = 768,
    .u16HTotal = 1344,
    .u16VTotal = 967,

    .u16DE_VStart = 0,

    .u16DefaultVFreq = 500,//600,

    // LPLL
    .u16LPLL_InputDiv  = 0x0000,
    .u16LPLL_LoopDiv   = 0x0203,    // for T3 setting
    .u16LPLL_OutputDiv = 0x0000,

    .u8LPLL_Type = 0,    // 0: LVDS, 1: RSDS, 2: TTL
    .u8LPLL_Mode = 0,    // 0: Single mode, 1: Dual mode

    // sync
    .u8HSyncWidth = 135,//0x20,	// KimTH_091125: PDP 42T1 panel timing set

    // output control
    .u16OCTRL    = 0x0000,
    .u16OSTRL    = 0x4000,
    .u16ODRV     = 0x0055,
    .u16DITHCTRL = 0,

    // MOD
    .u16MOD_CTRL0 	= 0x0008,//0x0004,
	.u16MOD_CTRL9 	= 0x0000,
    .u16MOD_CTRLA 	= 0x1108,//0x0100,
	.u8MOD_CTRLB  	= 0x02,

	.u16VTotal60Hz = 806,
	.u16VTotal50Hz = 967,
	.u16VTotal48Hz = 1007,
	.u16VStart60Hz = 17,//20,	// KimTH_091125: PDP 42T1 panel timing set
	.u16VStart50Hz = 17,//19,	// KimTH_091125: PDP 42T1 panel timing set
	.u16VStart48Hz = 17,//20,	// KimTH_091125: PDP 42T1 panel timing set
	.u16VBackPorch60Hz = 12,//14,	// KimTH_091125: PDP 42T1 panel timing set
	.u16VBackPorch50Hz = 12,//14,	// KimTH_091125: PDP 42T1 panel timing set
	.u16VBackPorch48Hz = 12,//14,	// KimTH_091125: PDP 42T1 panel timing set

	//Panel Option(LCD : 0, PDP : 1)
	.u8LCDorPDP = 1,	// LCD : 0, PDP : 1
	.u32LimitD5d6d7 = 0x00010000,
	.u16LimitOffset = 0xFFF0,
	.bTTL_10BIT = 1,
	.bOD_DataPath = 0,
};

static void mstar_msleep( unsigned int ms )
{
    U32 ret_timer = 0xFF;

    while (ret_timer)
    {
        ret_timer--;
    }
}

static void MDrv_SC_Set_TCON_MOD_Reset(void)
{
    U32 u32timeout = 0xFFFFFFFF;
    U32 u32IntSts;

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
    REG_WM(REG_SC_GOPINT(0x12), 0x20, 0x20);
    REG_WM(REG_SC_GOPINT(0x12), 0x00, 0x20);

    udelay(2000);


    SC_BK_SWICH(REG_SC_BK_GOPINT);
    while(--u32timeout)
    {
        u32IntSts = REG_RR(REG_SC_GOPINT(0x10)) | ((U32)REG_RR(REG_SC_GOPINT(0x11)) << 16);
        if(u32IntSts & (0x20))
        {
            MOD_BK_STORE;
            MOD_BK_SWICH(REG_MOD_BK_00);
            REG_WI(MOD_REG(0x42), 0 , BIT12);
            udelay(10);
            REG_WI(MOD_REG(0x42), 1 , BIT12);
            MOD_BK_RESTORE;
            SC_BK_SWICH(REG_SC_BK_GOPINT);
            REG_WM(REG_SC_GOPINT(0x12), 0x20, 0x20);            
            REG_WM(REG_SC_GOPINT(0x12), 0x00, 0x20);
            break;
        }
    }
    SC_BK_RESTORE;
   
}

static void MDrv_SC_TCON_EnableODCLK(U8 u8status)
{
    if (u8status)
    {
        REG_WL(REG_CKGEN0(0x53), 0x1C);            // odclk <- XTAL
    }
    else
    {
        REG_WL(REG_CKGEN0(0x53), 0x00);            // odclk <- XTAL
    }

}

static void MDrv_SC_Set_TCON_Count_Reset(UINT8 OnOff)
{
   REG_WI(TCON_REG(0x03), OnOff , BIT14);
}

void MDrv_SC_Init(PANEL_RESOLUTION_TYPE_e e_panel_type, SC_MOD_LPLL_TYPE_e e_lpll_type)
{
    MST_PANEL_INFO_t panelInfo = pnl_1920x1080;
    U32 memaddr = 0x3200000 ; // dram start address
    U32 memsize = 0xC00000 ;  // buffer length
    U16 u16Reg_03 = 0x0003;
    U32 u32DClk;
#if 0
    SC_TCON_POW_SEQ_INFO_t tconpower_sequence;
#endif
//    SC_TCON_MAP_t tconmapinfo;
//    U8 u8Tcontype;
    U32 u32DClkFactor;
    U16	u16TempVBackPorch = 0;

    if(e_panel_type == PANEL_RES_FULL_HD)
    {
        panelInfo = pnl_1920x1080;
    }
    else if(e_panel_type == PANEL_RES_WXGA)
    {
       	// 091125_KimTH_01: PDP boot logo 대응
		if (DDI_GetModelOption())
    	{
			panelInfo = pnl_1365x768_PDP4DVB;
    	}
		else	
		{
    	 	panelInfo = pnl_1366x768;
		}
    }
	else if (e_panel_type == PANEL_RES_XGA)	// 091125_KimTH_01: PDP boot logo 대응
	{
		panelInfo = pnl_1024x768_PDP4DVB;
	}
    
    if(e_lpll_type == LPLL_LVDS)
    {
        panelInfo.u8LPLL_Type = LPLL_LVDS;
    }
    else if(e_lpll_type == LPLL_TTL)
    {
        panelInfo.u8LPLL_Type = LPLL_TTL;
    }
    else if(e_lpll_type == LPLL_MINILVDS)
    {
        panelInfo = pnl_1920x1080_TCON;
    }


    if(gSysNvmDB.ColorDepth)
    {
        panelInfo.u8MOD_CTRLB = 0;
    }
    else
    {
        panelInfo.u8MOD_CTRLB = 0x02;
    }
    
    if((gToolOptionDB.nToolOption1.flags.eModelModuleType == MODULE_IPS) && (e_panel_type == PANEL_RES_FULL_HD))
    {
        panelInfo.u16HTotal = 2031;
        panelInfo.u16VTotal = 1116;
    }
    if((gToolOptionDB.nToolOption1.flags.eModelModuleType == MODULE_IPS) && (e_panel_type == PANEL_RES_WXGA))
    {
        panelInfo.u16HTotal = 1814;
        panelInfo.u16VTotal = 800;
    }

    if((gToolOptionDB.nToolOption1.flags.eModelModuleType == MODULE_CMO) && 
        ((gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_32)||(gToolOptionDB.nToolOption1.flags.eModelInchType == INCH_42)) &&		//(100702) 구형선J의뢰로 32"조건 추가 olivetree.park
        (e_lpll_type == LPLL_LVDS) && (panelInfo.u8LCDorPDP == 0) && (e_panel_type == PANEL_RES_FULL_HD) &&
        ((!Splash_GetHWoption(FRC_OPT_SEL)) && (!Splash_GetHWoption(FRC_OPT1_SEL)))
    )
        {
            panelInfo.u16MOD_CTRLA &= (~BIT0);
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
    if (panelInfo.u8LPLL_Type == LPLL_MINILVDS)
    {
        REG_WR(REG_SC_VOP(0x04), panelInfo.u16HStart);
        REG_WR(REG_SC_VOP(0x05), (panelInfo.u16HStart+panelInfo.u16Width-1));
        REG_WR(REG_SC_VOP(0x06), panelInfo.u16DE_VStart -2);
        REG_WR(REG_SC_VOP(0x07), (panelInfo.u16DE_VStart + panelInfo.u16Height));
    }
    else
    {
        REG_WR(REG_SC_VOP(0x04), panelInfo.u16HStart);
        REG_WR(REG_SC_VOP(0x05), (panelInfo.u16HStart+panelInfo.u16Width-1));
        REG_WR(REG_SC_VOP(0x06), panelInfo.u16DE_VStart);
        REG_WR(REG_SC_VOP(0x07), (panelInfo.u16DE_VStart + panelInfo.u16Height-1));

    }
    // display window
    REG_WR(REG_SC_VOP(0x08), panelInfo.u16HStart);
    REG_WR(REG_SC_VOP(0x09), (panelInfo.u16HStart+panelInfo.u16Width-1));
    REG_WR(REG_SC_VOP(0x0A), panelInfo.u16DE_VStart);
    REG_WR(REG_SC_VOP(0x0B), (panelInfo.u16DE_VStart + panelInfo.u16Height-1));

    //REG_WR(REG_SC_VOP(0x24), (3));



    //REG_WR(REG_SC_VOP(0x24), (3));

		// Set manual vsync for PDP
		// 091125_KimTH_01: PDP boot logo 대응
    	if (panelInfo.u8LCDorPDP == 1) //PDP
    	{
			//
			//void MHal_SC_VOP_SetAutoVSyncCtrl(FALSE)
			//-----------------------------------------------------------------
			REG_WI(REG_SC_VOP(0x10),  1, BIT15);
		
			//MHal_SC_VOP_SetOutputSycCtrl(eTempOutSyncMode,
			//	  pDrvCtx->pPanelInfo->u16VTotal - pDrvCtx->pPanelInfo->u16VStart,
			//	  pDrvCtx->pPanelInfo->u16VTotal - u16TempVBackPorch);
			//-----------------------------------------------------------------
			REG_WI(REG_SC_VOP(0x10),  1, BIT14);
			REG_WI(REG_SC_VOP(0x10),  1, BIT13);
			REG_WI(REG_SC_VOP(0x10),  1, BIT12);
			REG_WI(REG_SC_VOP(0x10), 0, BIT11);
			REG_WI(REG_SC_VOP(0x10), 0, BIT10);
			REG_WI(REG_SC_VOP(0x10),  1, BIT9);
    	}

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
    else if (panelInfo.u8LPLL_Type == LPLL_TTL)
    {
        REG_WM(REG_SC_VOP(0x22), G_SC_ENCLK_TTL, 0x1F);
    }
    else if (panelInfo.u8LPLL_Type == LPLL_MINILVDS)
    {
    }

    //
    //MHal_SC_VOP_SetOutputSycCtrl
    //-----------------------------------------------------------------
    if (panelInfo.u8LCDorPDP == 1)
    {
        u16TempVBackPorch = panelInfo.u16VBackPorch50Hz;
    }
    else if (panelInfo.u8LCDorPDP == 3)
    {
        u16TempVBackPorch = panelInfo.u16VBackPorch60Hz;
    }
    else
    {
        u16TempVBackPorch = 8;
    }
    REG_WR(REG_SC_VOP(0x02), (panelInfo.u16VTotal - panelInfo.u16VStart));
    REG_WR(REG_SC_VOP(0x03), (panelInfo.u16VTotal - u16TempVBackPorch));

    //
    //MHal_SC_VOP_OD_DataPath
    //-----------------------------------------------------------------

    REG_WI(REG_SC_VOP(0x58),panelInfo.bOD_DataPath ,0x400);

    if (panelInfo.u8LPLL_Type == LPLL_MINILVDS)
    {
        REG_WI(REG_SC_VOP(0x10),1 ,0x8000);
        REG_WI(REG_SC_VOP(0x21),0 ,0x0100);
    }

    // enable out vs interrrupt
    if (panelInfo.u8LPLL_Type == LPLL_MINILVDS)
    {
        SC_BK_SWICH(REG_SC_BK_GOPINT);
        REG_WM(REG_SC_GOPINT(0x14), 0x00, 0x20);
        REG_WM(0x2B18, 0x000, 0x400);
    }

    SC_BK_SWICH(REG_SC_BK_PEAKING);
    REG_WI(REG_SC_PEAKING(0x10), 1, 0x80);


    if(panelInfo.u16Height < 770)
    {
        SC_BK_SWICH(REG_SC_BK_VOP2_RP);

        REG_WI(REG_SC_BANK(0x10),  1, BIT15);
        REG_WM(REG_SC_BANK(0x10), 0x00, 0xFFF);
        REG_WM(REG_SC_BANK(0x11), 0x01, 0xFFF);
        REG_WM(REG_SC_BANK(0x12), 0x01, 0xFFF);
        REG_WM(REG_SC_BANK(0x13), panelInfo.u16Height +2, 0xFFF);

    }    

    // Record Splash Window function
    // this register just dummy register
    SC_BK_SWICH(0x23);
    REG_WL(REG_SC_BANK(0x5F), 0x01);


    SC_BK_RESTORE;

    //
    //MHal_SC_SetClk();
    //-----------------------------------------------------------------
    {
    REG_WM(REG_CKGEN0(0x52), 0x0C00, 0x3C00);  // fclk  <- select 216MHz daniel.huang //170 MHz (MPLL_DIV_BUF), 20090619 daniel.huang
    if(e_lpll_type == LPLL_MINILVDS)
    {
        MDrv_SC_TCON_EnableODCLK(0);
    }
    else
    {
        MDrv_SC_TCON_EnableODCLK(1);
    }
    }

    // LPLL setting
    // LPLL init
    //MHal_LPLL_Init(&lpllInit);
    //-----------------------------------------------------------------

    REG_WR(REG_LPLL(0x00), panelInfo.u16LPLL_InputDiv); // u16InputDiv
    REG_WR(REG_LPLL(0x01), panelInfo.u16LPLL_LoopDiv); // u16LoopDiv   // for T3 setting
    REG_WR(REG_LPLL(0x02), panelInfo.u16LPLL_OutputDiv); // u16OutputDiv

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

    u32DClk = (((U32)panelInfo.u16HTotal * (U32)panelInfo.u16VTotal) / 1000 * panelInfo.u16DefaultVFreq / 1000) / 10;
    if (panelInfo.u8LPLL_Type == LPLL_MINILVDS)
    {
        u32DClkFactor = 216*524288*2;
        u32DClk = (u32DClkFactor / u32DClk);
    }
    else
    {
        u32DClkFactor = 216*524288*16;
        u32DClk = (u32DClkFactor / u32DClk) / 7;
        if (panelInfo.u8LPLL_Mode == G_LPLL_MODE_DUAL)
        {
            u32DClk *= 2;
        }
    }

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
    if (panelInfo.u8LPLL_Type == LPLL_MINILVDS)
	{
		REG_WM(MOD_REG(0x20), 0x4400, 0xFF00); // set to 00 for LVDS out, 44 to mini LVDS out
		REG_WR(MOD_REG(0x30), 0x02);
		REG_WR(MOD_REG(0x31), 0x03);
	}
	else
	{
    	REG_WM(MOD_REG(0x20), 0, 0xFF00); // set to 00 for LVDS out, 44 to mini LVDS out
		REG_WR(MOD_REG(0x30), 0x01);
		REG_WR(MOD_REG(0x31), 0x01);
	}

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
    if (panelInfo.u8LPLL_Type == LPLL_MINILVDS)
    {
		//REG_WL(MOD_REG(0x7D), 0x65);
	}
	else
    	REG_WL(MOD_REG(0x7D), 0x1F);

	if((Splash_GetHWoption(FRC_OPT_SEL)) && (!Splash_GetHWoption(FRC_OPT1_SEL))) //S7M URSA3 Internal
	{
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
			REG_WM(MOD_REG(0x42), 0x80, 0xC0);
	        //REG_WL(MOD_REG(0x42), 0x80);

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


			REG_WL(MOD_REG(0x78), 0x04);
			//REG_WI(MOD_REG(0x78), 0,  0x01);        
			REG_WM(MOD_REG(0x42), 0x80, 0xC0);
	        //REG_WL(MOD_REG(0x42), 0x00);
	    }
	}
	else
	{
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

	}

	
    MOD_BK_RESTORE;

	#if 0
    if(panelInfo.u8LPLL_Type == LPLL_MINILVDS)
    {
        tconmapinfo.u16tconpanelIdx = TCON_PANEL_LG42;
        MDrv_SC_Set_TCONMap(&tconmapinfo);
    // TCON power sequence flow needs to control on LG side
    #if 0
        tconpower_sequence.u16tconpanelIdx = tconmapinfo.u16tconpanelIdx;
        tconpower_sequence.benable = 1;
        for(u8Tcontype; u8Tcontype > SC_SIGNAL_NUMS; u8Tcontype++)
            MDrv_SC_Set_TCONPower_Sequence(tconpower_sequence, u8Tcontype);
    #endif
    }
	
	#endif

}

extern BOOL bTCONpwsq;

void MDrv_SC_TCON_PWS_VSINT(void)
{
    U32 u32IntSts;
    U32 u32timeout = 0xFFFFFFFF;

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
    while(--u32timeout)
    {
        u32IntSts = REG_RR(REG_SC_GOPINT(0x10)) | ((U32)REG_RR(REG_SC_GOPINT(0x11)) << 16);
        if(u32IntSts & (0x20))
        {
            // important for TCON power sequence
            //printf("scaler vs interrupt time \n");
            //printf("Tcon power sequence must put here to set\n");
            // Tcon power sequence setting start
            if(bTCONpwsq)
            {
                MDrv_TCONMAP_DumpPowerOnSequenceReg();
            }
            // Tcon power sequence setting End

            SC_BK_SWICH(REG_SC_BK_GOPINT);
            REG_WM(REG_SC_GOPINT(0x12), 0x20, 0x20);
            REG_WM(REG_SC_GOPINT(0x12), 0x00, 0x20);
            break;
        }
    }
    SC_BK_RESTORE;
}

void MDrv_SC_TCON_Init_Flow(PANEL_RESOLUTION_TYPE_e e_panel_type, SC_MOD_LPLL_TYPE_e e_lpll_type, U16 u16tconpanelIdx)
{
    SC_TCON_MAP_t tconmapinfo;
    tconmapinfo.u16tconpanelIdx = u16tconpanelIdx;

    // Please dont change any TCON init flow 
    // This flow suggestion by HW RD

    // 1. Scaler Init, OD clock gating
    // 2. LPLL (Free Run) Init
    // 3. MOD (TTL LVDS) Setting
    MDrv_SC_Init(e_panel_type, e_lpll_type);

    // 4. TCON MOD Table Init
    MDrv_SC_Set_TCONMap(&tconmapinfo);

    // 5. TCON Count Enable
	MDrv_SC_Set_TCON_Count_Reset(1);

    // 6. OD Clock Enable
	MDrv_SC_TCON_EnableODCLK(1);

    // 7. MOD Software Reset
	MDrv_SC_Set_TCON_MOD_Reset();

}


