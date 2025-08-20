////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __MST_PLATFORM_H__
#define __MST_PLATFORM_H__
#define MFC_USE_IN_BOOLLOAD         1
#include "mdrv_types.h"

#define _6BITS						0
#define _8BITS						1
#define _10BITS						2

#define _TTL             			0
#define _MINI_LVDS                  1
#define _LVDS                       2
#define _RSDS                       3
#define _MINI_LVDS_GIP              4
#define _MINI_LVDS_GIP_V5           5


#define _SINGLE						0
#define _DUAL       				1
#define _QUAD       				2
#define _QUAD_LR       				3
#define _V_BY1                      4

// for Lvds output channel,
// REG_321F[7:6]=>D, [5:4]=>C, [3:2]=>B, [1:0]=>A
#define CHANNEL_SWAP(D, C, B, A)	((D<<6)&0xC0)|((C<<4)&0x30)|((B<<2)&0x0C)|(A&0x03)
#define _CH_A                       0
#define _CH_B                       1
#define _CH_C                       2
#define _CH_D                       3

//For titania control setting; REG_MOD40 [2]=>TI/JATA [5]=>P_N_SWAP [6]=>LSB_MSB_SWAP
#define MOD40(TI_JA, P_N_SWAP, L_M_SWAP, DCLK_DLY)	(((TI_JA<<2)&0x0C)|((P_N_SWAP<<5)&0x20)|((L_M_SWAP<<6)&0x40)|((DCLK_DLY<<7)&0x0F00))
#define _TI                       1
#define _JEIDA                    0
#define _L_M_SWAP_ON              1
#define _L_M_SWAP_OFF             0
#define _P_N_SWAP_ON              1
#define _P_N_SWAP_OFF             0

/*For titania control setting; REG_MOD49[3]=>SWAP_ODD_RB,
										[4]=>SWAP_EVEN_RB,
										[5]=>SWAP_ODD_ML,
										[6]=>SWAP_EVEN_ML,
*/
#define MOD49(EVEN_ML, EVEN_RB, ODD_ML, ODD_RB, BIT_NUM) \
				(((EVEN_ML<<14)&0x4000)|((EVEN_RB<<13)&0x2000)|\
				((ODD_ML<<12)&0x1000)|((ODD_RB<<11)&0x0800))
#define _10BITS_OP      0
#define _6BITS_OP       1
#define _8BITS_OP       2
#define _ODD_RB_OFF    0
#define _ODD_RB_ON     1
#define _ODD_ML_OFF    0
#define _ODD_ML_ON     1
#define _EVEN_RB_OFF   0
#define _EVEN_RB_ON    1
#define _EVEN_ML_OFF   0
#define _EVEN_ML_ON    1

/*For titania control setting; REG_MOD4A[0]=>ODD_EVEN_SWAP,
									   [1]=>SINGLE_DUAL_CHANNEL
									   [2]=>INVERT_DATA_ENABLE
									   [3]=>INVERT_VSYNC
									   [4]=>INVERT_DCLK
									   [12]=>INVERT_HSYNC
*/
#define MOD4A(INV_HSYNC, INV_DCLK, INV_VSYNC, INV_DE, SD_CH, OE_SWAP) \
				(((INV_VSYNC<<12)&0x1000)|((INV_VSYNC<<4)&0x10)|\
				((INV_VSYNC<<3)&0x08)|((INV_DE<<2)&0x04)|\
				((SD_CH<<1)&0x02)|(OE_SWAP&0x01))
#define _ODD_EVEN_SWAP_OFF        0
#define _ODD_EVEN_SWAP_ON         1
#define _INV_DE_OFF   0
#define _INV_DE_ON    1
#define _INV_VSYNC_OFF         0
#define _INV_VSYNC_ON          1
#define _INV_DCLK_OFF          0
#define _INV_DCLK_ON           1
#define _INV_HSYNC_OFF         0
#define _INV_HSYNC_ON          1

//For titania control setting; REG_MOD4B[0:1]=>0x: 10-bits, 10: 8-bits, 11: 6-bits
#define MOD4B(TI_BIT_MOD) (TI_BIT_MOD&0x03)
#define _TI_10_BITS        0
#define _TI_8_BITS         2
#define _TI_6_BITS         3
//-------------------------------------------------------------------------------------------------
// Board
//-------------------------------------------------------------------------------------------------
typedef struct MST_BOARD_INFO_s
{

} MST_BOARD_INFO_t, *PMST_BOARD_INFO_t;

//-------------------------------------------------------------------------------------------------
// Panel
//-------------------------------------------------------------------------------------------------

typedef struct MST_PANEL_INFO_s
{
    // Basic
    U16 u16HStart; //ursa scaler
    U16 u16VStart; //ursa scaler
    U16 u16Width; //ursa scaler
    U16 u16Height; //ursa scaler
    U16 u16HTotal; //ursa scaler
    U16 u16VTotal; //ursa scaler

    U16 u16DE_VStart;

    U32 u32DClkFactor;
    U16 u16DefaultVFreq;

    // LPLL
    U16 u16LPLL_InputDiv;
    U16 u16LPLL_LoopDiv;
    U16 u16LPLL_OutputDiv;

    U8  u8LPLL_Type;
    U8  u8LPLL_Mode;

    // sync
    U8  u8HSyncWidth;
    BOOL bPnlDblVSync;

    // output control
    U16 u16OCTRL;
    U16 u16OSTRL;
    U16 u16ODRV;
    U16 u16DITHCTRL;

    // MOD
    U16 u16MOD_CTRL0;
    U16 u16MOD_CTRL9;
    U16 u16MOD_CTRLA;
    U8  u8MOD_CTRLB;

	//titania to URSA
	U8  u8LVDSTxChannel; //ursa scaler
    U8  u8LVDSTxBitNum; //ursa scaler
    U8  u8LVDSTxTiMode;  //ursa scaler 40-bit2
    U8  u8LVDSTxSwapMsbLsb; //ursa scaler
    U8  u8LVDSTxSwap_P_N; //ursa scaler
    U8  u8LVDSTxSwapOddEven; //ursa scaler

	//URSA to Panel info
    U8  u8PanelVfreq; //ursa scaler
	U8  u8PanelChannel; //ursa scaler
	U8	u8PanelLVDSSwapCH; //ursa scaler
	U8  u8PanelBitNum; //ursa scaler
	U8	u8PanelLVDSShiftPair; //ursa scaler
	U8	u8PanelLVDSTiMode; //ursa scaler
	U8	u8PanelLVDSSwapPol; //ursa scaler
	U8	u8PanelLVDSSwapPair; //ursa scaler

	//LGE [vivakjh] 2008/11/12 	Add for DVB PDP Panel
	//Additional Info.(V Total)
    U16 u16VTotal60Hz; //ursa scaler
    U16 u16VTotal50Hz; //ursa scaler
    U16 u16VTotal48Hz; //ursa scaler
	//[vivakjh] 2008/12/23	Add for adjusting the MRE in PDP S6
	U16 u16VStart60Hz;
	U16 u16VStart50Hz;
	U16 u16VStart48Hz;
	U16 u16VBackPorch60Hz;
	U16 u16VBackPorch50Hz;
	U16 u16VBackPorch48Hz;

	//Panel Option(LCD : 0, PDP : 1, LCD_NO_FRC : 2)
	U8	u8LCDorPDP;

	U32 u32LimitD5d6d7; //thchen 20081216
	U16 u16LimitOffset; //thchen 20081216
	U8  u8LvdsSwingUp;
} MST_PANEL_INFO_t, *PMST_PANEL_INFO_t;



//-------------------------------------------------------------------------------------------------
// Platform
//-------------------------------------------------------------------------------------------------
typedef struct MST_PLATFORM_INFO_s
{
	MST_BOARD_INFO_t board;
    MST_PANEL_INFO_t panel;
} MST_PLATFORM_INFO_t, *PMST_PLATFORM_INFO_t;

#endif // __MST_PLATFORM_H__
