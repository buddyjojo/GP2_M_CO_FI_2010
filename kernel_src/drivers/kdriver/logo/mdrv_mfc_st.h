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

#ifndef __DRV_MFC_ST_H__
#define __DRV_MFC_ST_H__


//------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Structure
//------------------------------------------------------------------------------
typedef struct
{
    U16 u16Addr;
    U16 u16Mask;
    U16 u16Data;
} MFC_REG_DATA;

typedef struct IO_MFC_PANEL_INFO_s
{
    U32* pPanelInfo;
    U16  u16Len;
} IO_MFC_PANEL_INFO_t, *PIO_MFC_PANEL_INFO_t;

typedef struct MST_MFC_RegUnitType_s
{
    S16 ucIndex;
    S16 ucValue;
}MST_MFC_RegUnitType_t;

typedef struct MST_MFC_SYS_INFO_s
{	
    //titania to URSA	
	U8  u8LVDSChannel; 		//Single, Dual
    U8  u8LVDSBitNum;  		//8bits, 10 bits
    U8  u8LVDSTiMode;  		//Thin/Ti mode scaler 40-bit2
    U8  u8LVDSSwapMsbLsb; 	//ursa scaler
    U8  u8LVDSSwap_P_N; 		//ursa scaler
    U8  u8LVDSSwapOddEven; 	//ursa scaler

	//URSA to Panel info
	U8	u8PanelType;  			//TTL, Mini_LVDS, LVDS
	U8  u8PanelBitNum; 			//Panel bit number
	U8  u8PanelChannel; 		//Single, Dual, Quad, Quad_LR
	U8	u8PanelDither;
	U8	u8PanelLVDSTiMode; 		//Panel TI/Thin mode
	U8	u8PanelLVDSSwapPol; 	//Panel LVDS polarity swap
	U8	u8PanelLVDSSwapCH; 		//LVDS chenel swap ABCD
	U8	u8PanelLVDSSwapPair; 	//Panel LVDS pair swap
	U8	u8PanelLVDSShiftPair; 	
	U8	u8PanelBlankCPVC;			//Panel Mini LVDS use
	U8	u8PanelBlankOEC;			//Panel Mini LVDS use
	U8	u8PanelBlankTPC;			//Panel Mini LVDS use
	U8	u8PanelBlankSTHC;			//Panel Mini LVDS use

	U16 u16HStart;				//ursa scaler
    U16 u16VStart; 				//ursa scaler
    U16 u16Width; 				//ursa scaler
    U16 u16Height; 				//ursa scaler
    U16 u16HTotal; 				//ursa scaler
    U16 u16VTotal; 				//ursa scaler	
    U8  u8PanelVfreq; 			//Panel frame rate 60Hz, 120Hz, 240Hz
	U8	u8PanelIncVtotalFor50Hz;	//Change Vtotal for DCLK
	U8	u8PanelCSC;					//LVDS CSC enable/disable
	U16	u16MFCMemoryClk;			//MFC memory clock MHz
	U16	u16MFCMemoryType;			//MFC memory type
	U8	u8PanelGAMMA;
	U8  u8ODMode;
	U8	u8IPMode;
	U8	u8Preset;
	U8	u8MirrorMode;
	U8  u8UseMPIF;
	U8	u8ChipRevision;
} MST_MFC_SYS_INFO_t, *PMST_MFC_SYS_INFO_t;

typedef struct MST_MFC_STATUS_s
{
// REG_29CC
    U8 GMVX;
// REG_29CD
    U8 GMVY        : 7;
    U8 GMVConfirm    : 2;
// REG_29CE
    U8 FrameEnd     : 1;
    U8 Skip_Thd     : 2;
// REG_29CF
    U8 Film22Mode : 1;
    U8 Film32Mode : 1;
    U8 Film22Idx     : 1;
    U8 Film32Idx     : 3;
    U8 FDUP         : 1;

    U8 MaxSAD_MSB;
    U8 MinSAD_MSB;
    U8 maxXNegSpeed; //0x29c2;
    U8 maxXPosSpeed; //0x29c2;
    U8 maxYNegSpeed; //0x29c2;
    U8 maxYPosSpeed; //0x29c2;

// REG_29E0
    U16 ErrorBlk1;
    U16 ErrorBlk2;
    U16 ErrorBlk3;
    U16 ErrorBlk4;
    U16 ErrorBlk5;
    U16 ErrorBlk6;

    U16 GMV0Err;
    U16 MINonContinuousBoundary;
    U16 gmvBlkCnt;
    U16 mv0BlkCnt;
    U16 cplxBlkCnt2;
    U16 MinSAD;
    U16 MovingBlkCnt;
    U16 cplxBlkCnt;
    U16 veryCplxBlkCnt;
    U16 unMatchPointCnt;
}MST_MFC_STATUS_t, *PMST_MFC_STATUS_t;

typedef struct MST_MFC_ADJ_s
{
	U8 u8BlurAdj;	
	U8 u8JudderAdj;	
	U8 u8MFCAdj;
}MST_MFC_ADJ_t, *PMST_MFC_ADJ_t;


typedef struct MST_MFC_VIDEOBLOCK_s
{
	U8 u8Type;
	BOOL bOnOff;
}MST_MFC_VIDEOBLOCK_t, *PMST_MFC_VIDEOBLOCK_t;

typedef struct MST_MFC_BYPASSWINDOW_s
{
	BOOL	bOnOff;
	U8		u8WindowID;
	U16		u16HStart;
	U16		u16VStart;
	U16		u16HSize;
	U16		u16VSize;
}MST_MFC_BYPASSWINDOW_t, *PMST_MFC_BYPASSWINDOW_t;

typedef struct MST_MFC_DEMO_BAR_s
{
	BOOL bEnable; 
	BOOL bDirection; 
	U8 u8width; 
	U8 u8color;
}MST_MFC_DEMO_BAR_t, *PMST_MFC_DEMO_BAR_t;

typedef struct MST_MFC_SSC_s
{
	U16 u16KHz;
	U16 u16Percent;
	BOOL bEnable;	
	BOOL bMiuLvds;
}MST_MFC_SSC_t, *PMST_MFC_SSC_t;

typedef struct MST_MFC_HVTOTAL_s
{
	U16 u16HTotal;
	U16 u16VTotal;
}MST_MFC_HVTOTAL_t, *PMST_MFC_HVTOTAL_t;

#endif//__DRV_MFC_ST_H__

