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
#include "mdrv_types.h"
#include "mdrv_scaler_st.h"  // FitchHsu 20080811 implement LPLL type
#include "mhal_mod_reg.h"
#include "mhal_mod.h"
#include "mhal_scaler_reg.h"
#include "mhal_utility.h"
// shjang_090904
#include "mdrv_scaler.h"

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures Start
//-------------------------------------------------------------------------------------------------
extern unsigned long irq_flags_swbk;
extern spinlock_t switch_bk_lock;
extern U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


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

// FitchHsu 20080811 implement LPLL type
void MHal_MOD_SetPower(BOOL bOn, U8 u8LPLL_Type)
{
    MOD_BK_STORE;
    MOD_BK_SWICH(REG_MOD_BK_00);
    if (bOn)
     // output driving current
    {
        REG_WM(MOD_REG(0x37), 0, BIT8); // for T3
        // FitchHsu 20080811 implement LPLL type
        //lachesis_090909
        if (u8LPLL_Type == LPLL_LVDS || u8LPLL_Type == LPLL_RSDS)
        	REG_WI(MOD_REG(0x78), 0,  BIT0);//bit0 clear
    }
    else
    {
        REG_WM(MOD_REG(0x37), BIT8, BIT8); // for T3
        REG_WI(MOD_REG(0x78), BIT0 , BIT0);//bit0 enable
    }
    MOD_BK_RESTORE;
}

void MHal_MOD_Init(PMST_PANEL_INFO_t pPanelInfo)
{
    // FitchHsu 20081116 LVDS Power
    MOD_BK_STORE;
    MOD_BK_SWICH(REG_MOD_BK_00);

    //PANEL_DCLK_DELAY:8, PANEL_SWAP_LVDS_CH:6, PANEL_SWAP_LVDS_POL:5, PANEL_LVDS_TI_MODE:2,
    REG_WR(MOD_REG(0x40), pPanelInfo->u16MOD_CTRL0);

    //EXT GPO disable
    REG_WR(MOD_REG(0x46), 0x00);
    REG_WR(MOD_REG(0x47), 0x00);

    //PANEL_SWAP_EVEN_ML:14, PANEL_SWAP_EVEN_RB:13, PANEL_SWAP_ODD_ML:12, PANEL_SWAP_ODD_RB:11
	// [7,6] : output formate selction 10: 8bit, 01: 6bit :other 10bit
	REG_WR(MOD_REG(0x49), pPanelInfo->u16MOD_CTRL9);
// FitchHsu 20080811 implement LPLL type
#if 0
    //PANEL_INV_HSYNC:12, PANEL_INV_DCLK:4, PANEL_INV_VSYNC:3, PANEL_INV_DE:2, PANEL_DUAL_PORT:1, PANEL_SWAP_PORT:0,
    REG_WR(MOD_REG(0x4A), pPanelInfo->u16MOD_CTRLA);
#endif
	//[1:0]ti_bitmode=00(10bit)
	REG_WR(MOD_REG(0x4B), pPanelInfo->u8MOD_CTRLB);// LGE drmyung 081009

    // 20100113 daniel.huang: For URSA3 U02, we need to switch to LVDS. And after URSA3 U02, we can set to TTL.
    // for URSA3, we need to set [7:6] to 2b'10, else 2b'00
    // select u8MOD_CTRL2 & u8MOD_CTRL78 in video_ddi.c
    REG_WM(MOD_REG(0x42), pPanelInfo->u8MOD_CTRL2, (BIT7|BIT6));
    REG_WL(MOD_REG(0x78), pPanelInfo->u8MOD_CTRL78);

    //if(pPanelInfo->u8LCDorPDP == 2)//S7 LCD : 0, PDP : 1, LCD_NO_FRC : 2, LCD_TCON : 3
    if(pPanelInfo->u8LPLL_Type == LPLL_TYPE_LVDS)   //LCD_NO_FRC : 2
    {
        REG_WM(MOD_REG(0x20), 0, 0xFF00); // set to 00 for LVDS out, 44 to mini LVDS out
        REG_WR(MOD_REG(0x30), 0x01);
        REG_WR(MOD_REG(0x31), 0x01);
        REG_WM(MOD_REG(0x32), 0, 0x3);    // H32[1:0] :  This is to open dynamic getting for power saving. Normally, you could close it first( = 0).// H32[1:0] :  This is to open dynamic getting for power saving. Normally, you could close it first( = 0).

        REG_WR(MOD_REG(0x71), 0xFFFF);
        REG_WL(MOD_REG(0x72), 0x03);

		//REG_WH(MOD_REG(0x77), 0x40);//pre-emphasis level 00->40
		REG_WH(MOD_REG(0x77), pPanelInfo->u8MOD_CTRL77);//pre-emphasis level

        REG_WL(MOD_REG(0x77), 0x0F);
        REG_WL(MOD_REG(0x7D), 0x1F);
    }
    //else if (pPanelInfo->u8LCDorPDP == 3)//S7T
    else if(pPanelInfo->u8LPLL_Type == LPLL_TYPE_MINILVDS)  //LCD_TCON : 3
    {
        REG_WM(MOD_REG(0x20), 0x4400, 0xFF00); // set to 00 for LVDS out, 44 to mini LVDS out
        REG_WR(MOD_REG(0x30), 0x02);
        REG_WR(MOD_REG(0x31), 0x03);
        REG_WM(MOD_REG(0x32), 0, 0x3);      // H32[1:0] :  This is to open dynamic getting for power saving. Normally, you could close it first( = 0).

        REG_WR(MOD_REG(0x71), 0xFFFF);
        REG_WL(MOD_REG(0x72), 0x03);
        //REG_WH(MOD_REG(0x77), 0x00);//pre-emphasis level 60->00
		REG_WH(MOD_REG(0x77), pPanelInfo->u8MOD_CTRL77);//pre-emphasis level


        REG_WL(MOD_REG(0x77), 0x0F);
    }
    else    //TTL case
    {
        REG_WM(MOD_REG(0x20), 0x1100, 0xFF00); // set to 00 for LVDS out, 44 to mini LVDS out
        REG_WR(MOD_REG(0x30), 0x01);
        REG_WR(MOD_REG(0x31), 0x01);
        REG_WM(MOD_REG(0x32), 0x3, 0x3);    // H32[1:0] :  This is to open dynamic getting for power saving. Normally, you could close it first( = 0).// H32[1:0] :  This is to open dynamic getting for power saving. Normally, you could close it first( = 0).

        REG_WR(MOD_REG(0x77), 0x0);     // disable ib/ck : 20100106 daniel.huang: turn off unused power under TTL case
        REG_WL(MOD_REG(0x7D), 0x0);     // 20100106 daniel.huang: turn off unused power under TTL case

        REG_WR(TCON_REG(0x75), 0x7);    // [0]tcon grp0 gating[1]grp1[2]grp2 : 20100106 daniel.huang: turn off unused power under TTL case
    }
    MOD_BK_RESTORE;

#if (USE_BOARD_DEFINE==1)
#if (CONFIG_MSTAR_TITANIA_BD_T2_LG_VENUS_BOARD_DVB_1) || (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_DVB_1)	\
	|| (CONFIG_MSTAR_TITANIA_BD_T2_LG_PDP_BOARD_DVB_1) || (CONFIG_MSTAR_TITANIA_BD_T2_LG_VENUS_BOARD_ATSC_1)
/*	BT_ONOFF(PAD_LDE, 131)		dreamer@lge.com
	gpio init 과정에 BT_ONOFF을 HIGH로 설정하고 APPL을 실행하면, BT_ONOFF가 LOW가 되는 경우 발생:

    위의 "REG_WR(MOD_REG(0x6D), 0x5555);" 호출후 BT_ONOFF가 LOW로 전환됨.
	->  PAD_LDE가 GPIO 상태로 SET 해주는 부분이 추가되야 함.
	(왜, PAD_LDE의 GPIO 설정이 해제되는지는 모름)
*/
{
	/* BT_ONOFF : HIGH */
	extern void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO);
//#define PAD_LDE             131

	MDrv_GPIO_Pad_Set( 131 /*PAD_LDE*/ );
	//MDrv_GPIO_Pad_Oen( 131 );
	//MDrv_GPIO_Pull_High( 131 );
}
#endif

#if (	defined(CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1)	||	\
			defined(CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERVA_BOARD_ATSC_1)	)
		/*	taburin : 20090107, mod init시에 PAD_LCK, PAD_LDE gpio 설정이 disable 되어 버리므로
							다시 pad set 해줌.*/


		/* LVDS_SEL : LOW(VESA) */
		extern void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO);

		//#define PAD_LCK			  130
		MDrv_GPIO_Pad_Set( 130 /*PAD_LCK*/ );
#endif
#endif



}

void MHal_MOD_SelTTL(U16 u16Sel)
{
    // FitchHsu 20081116 LVDS Power
    MOD_BK_STORE;
    MOD_BK_SWICH(REG_MOD_BK_00);
    REG_WR(MOD_REG(0x44), u16Sel);
    MOD_BK_RESTORE;
}
// FitchHsu 20080811 implement LPLL type
#if 0
void MHal_MOD_Ctrl0(U8 u8Ctrl)
{
    // FitchHsu 20081116 LVDS Power
    MOD_BK_STORE;
    MOD_BK_SWICH(REG_MOD_BK_00);
    REG_WR(MOD_REG(0x40), u8Ctrl);
    MOD_BK_RESTORE;
}

void MHal_MOD_CtrlA(U8 u8Ctrl)
{
    // FitchHsu 20081116 LVDS Power
    MOD_BK_STORE;
    MOD_BK_SWICH(REG_MOD_BK_00);
    REG_WR(MOD_REG(0x4A), u8Ctrl);
    MOD_BK_RESTORE;
}
#endif
// FitchHsu 20080811 implement LPLL type
void MHal_MOD_Set_LPLL(PMST_PANEL_INFO_t pPanelInfo)
{
    MOD_BK_STORE;
    MOD_BK_SWICH(REG_MOD_BK_00);

    if(pPanelInfo->u8LPLL_Type == LPLL_TTL)
    {
        REG_WR(MOD_REG(0x6D), 0x00);
        REG_WR(MOD_REG(0x6E), 0x00);
        REG_WR(MOD_REG(0x6F), 0x00);
        REG_WR(MOD_REG(0x45), 0x00);
        REG_WM(MOD_REG(0x7E), 0x0000, 0xF000);
        REG_WM(MOD_REG(0x4A), 0x0110, 0x0113);
        REG_WM(MOD_REG(0x78), BIT0, BIT0);
        REG_WM(MOD_REG(0x6C), (pPanelInfo->bTTL_10BIT <<15), BIT15);
    }
    else if(pPanelInfo->u8LPLL_Type == LPLL_LVDS)
    {
        REG_WR(MOD_REG(0x6D), 0x5555);
        REG_WR(MOD_REG(0x6E), 0x5555);
        REG_WL(MOD_REG(0x6F), 0x55);
        REG_WM(MOD_REG(0x45), MOD_OEZ_ALL, 0x3F);
        REG_WR(MOD_REG(0x4A), pPanelInfo->u16MOD_CTRLA);
    }
    MOD_BK_RESTORE;
}

