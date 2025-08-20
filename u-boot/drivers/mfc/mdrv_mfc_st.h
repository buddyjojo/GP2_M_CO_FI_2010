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

#ifndef __DRV_MFC_ST_H__
#define __DRV_MFC_ST_H__


//------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------
#define BOOL U8

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

typedef enum TCON_PANEL_TYPE_e
{
    // GIP panel idx start from 0x00
    GIP_DEFAULT_120,                    //= 0x00,
    GIP_32_LAMP_60,                     //= 0x01,

    GIP_32_LAMP_120,                    //= 0x02,
    GIP_32_LAMP_T240 = GIP_32_LAMP_120, //= 0x02,
    GIP_32_EDGE_120  = GIP_32_LAMP_120, //= 0x02,

    GIP_37_LAMP_60,                     //= 0x03,
    GIP_37_LED_120,                     //= 0x04,
    GIP_37_LAMP_T240,                   //= 0x05,
    GIP_42_LAMP_60,                     //= 0x06,

    GIP_42_LED_120,                     //= 0x07,
    GIP_42_LAMP_120  = GIP_42_LED_120,  //= 0x07,
    GIP_42_LAMP_T240 = GIP_42_LED_120,  //= 0x07,
    GIP_42_IOP_T240  = GIP_42_LED_120,  //= 0x07,

    GIP_47_LAMP_60,                     //= 0x08,

    GIP_47_LAMP_120,                    //= 0x09,
    GIP_47_LAMP_T240 = GIP_47_LAMP_120, //= 0x09,

    GIP_47_EDGE_120,                    //= 0x0A,
    GIP_47_IOP_T240  = GIP_47_EDGE_120, //= 0x0A,

    // NON-GIP idx start from 0x80.
    TCON_DEFAULT_120 = 0x80,

    TCON_LAMP_120,                      //= 0x81,
    TCON_LAMP_T240   = TCON_LAMP_120,   //= 0x81,

    TCON_EDGE_120,                      //= 0x82,
    TCON_IOP_T240    = TCON_EDGE_120,   //= 0x82,

}TCON_PANEL_TYPE;

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
    TCON_PANEL_TYPE eTconType;
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

typedef struct MST_MFC_GIP_TIMING_s
{
    // SOE
    U16 SOE_Start;
    U16 SOE_End;
    // POL
    U16 POL_Start;
    // VST
    U16 VST_VerticalStart;
    U16 VST_VerticalEnd;
    U16 VST_HorizontalStart;
    U16 VST_HorizontalEnd;
    U8 VST_HorizontalWidth;
    // GCLK1
    U16 GCLK1_1stLn_Start;
    U16 GCLK1_1stLn_End;
    U16 GCLK1_OthLn_Start;
    U16 GCLK1_OthLn_End;
    U16 GCLK1_EndLn_Start;
    U16 GCLK1_EndLn_End;
    U16 GCLK1_SeqVertical_Start;
    U16 GCLK1_SeqVertical_End;
    // GCLK2
    U16 GCLK2_1stLn_Start;
    U16 GCLK2_1stLn_End;
    U16 GCLK2_OthLn_Start;
    U16 GCLK2_OthLn_End;
    U16 GCLK2_EndLn_Start;
    U16 GCLK2_EndLn_End;
    U16 GCLK2_SeqVertical_Start;
    U16 GCLK2_SeqVertical_End;
    // GCLK3
    U16 GCLK3_1stLn_Start;
    U16 GCLK3_1stLn_End;
    U16 GCLK3_OthLn_Start;
    U16 GCLK3_OthLn_End;
    U16 GCLK3_EndLn_Start;
    U16 GCLK3_EndLn_End;
    U16 GCLK3_SeqVertical_Start;
    U16 GCLK3_SeqVertical_End;
    // GCLK4
    U16 GCLK4_1stLn_Start;
    U16 GCLK4_1stLn_End;
    U16 GCLK4_OthLn_Start;
    U16 GCLK4_OthLn_End;
    U16 GCLK4_EndLn_Start;
    U16 GCLK4_EndLn_End;
    U16 GCLK4_SeqVertical_Start;
    U16 GCLK4_SeqVertical_End;
    // GCLK5
    U16 GCLK5_1stLn_Start;
    U16 GCLK5_1stLn_End;
    U16 GCLK5_OthLn_Start;
    U16 GCLK5_OthLn_End;
    U16 GCLK5_EndLn_Start;
    U16 GCLK5_EndLn_End;
    U16 GCLK5_SeqVertical_Start;
    U16 GCLK5_SeqVertical_End;
    // GCLK6
    U16 GCLK6_1stLn_Start;
    U16 GCLK6_1stLn_End;
    U16 GCLK6_OthLn_Start;
    U16 GCLK6_OthLn_End;
    U16 GCLK6_EndLn_Start;
    U16 GCLK6_EndLn_End;
    U16 GCLK6_SeqVertical_Start;
    U16 GCLK6_SeqVertical_End;
    // VDD_ODD
    U16 VDD_ODD_Vertical_Start;
    U16 VDD_ODD_Vertical_End;
    U16 VDD_ODD_Horizontal_Start;
    U16 VDD_ODD_Horizontal_End;
    U8  VDD_ODD_nFrameTog;
    // VDD_EVEN
    U16 VDD_EVEN_Vertical_Start;
    U16 VDD_EVEN_Vertical_End;
    U16 VDD_EVEN_Horizontal_Start;
    U16 VDD_EVEN_Horizontal_End;
    U8  VDD_EVEN_nFrameTog;

    U8  REG_2312_BIT5;
    U16 OUT_MUX;
}MST_MFC_GIP_TIMING_t, *PMST_MFC_GIP_TIMING_t;

//NON-GIP
typedef struct MST_MFC_TCON_TIMING_s
{
    // GSP(VST)
    U16  GSP_Start;    // BK23_13[11:0]    //0x2326
    // SOE
    U16  SOE_Start;    // BK23_10[11:0]    //0x2320
    U16  SOE_Period;   // BK23_11[11:0]    //0x2322
    // GOE
    U16  GOE_Start;    // BK23_19[11:0]    //0x2332
    U16  GOE_Period;   // BK23_1A[11:0]    //0x2334
    // GSC
    U16  GSC_Start;    // BK23_15[11:0]    //0x232A
    U16  GSC_Period;   // BK23_17[11:0]    //0x232E
    // FLK(H)
    U16  FLK_Start;    // BK23_70[11:0]    //0x23E0
    U16  FLK_Period;   // BK23_76[11:0]    //0x23EC
    // POL
    U16  POL_Start;    // BK23_1C[11:0]    //0x2338

} MST_MFC_TCON_TIMING_t, *PMST_MFC_TCON_TIMING_t;

#endif//__DRV_MFC_ST_H__

