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

#ifndef _HAL_SCALER_H_
#define _HAL_SCALER_H_

#include "mdrv_scaler_st.h"
#include "chip_setup.h"
//#define MIPS_MIU0_BASE      (0xA0000000)
//#define MIPS_MIU1_BASE      (0xC0000000)

// 20090410 daniel.huang
// T3: base address 16 byte alignment; according to designer's suggest,
// offset to be 32 pixel aignment for all case(444 10bit/8bit, 422 10bit/8bit)
#define BYTE_PER_WORD           16
#define OFFSET_PIXEL_ALIGNMENT  32
#define MAX_FRAMEBUFFER_NUM     3
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define MST_H_PERIOD_MASK           0x1FFF
#define MST_V_TOTAL_MASK            0x07FF

// === Scart ID Level ===
// Level  0:   0V ~  2V
// Level 1A: 4.5V ~  7V (aspect ratio 16:9)
// Level 1B: 9.5V ~ 12V
#define SCART_ID_LEVEL_0V           0
#define SCART_ID_LEVEL_3V         	15	//LGE boae 20081028
#define SCART_ID_LEVEL_4V           10 //20  //31//21//35		//LGE boae 20081028
#define SCART_ID_LEVEL_8V           21 //41  //60		//LGE boae 20081028

#define SAR_ADC_CHANNEL_DATA_MASK   0x3F

//-------------------------------------------------------------------------------------------------
// Adative Tuning
//-------------------------------------------------------------------------------------------------
#define ENABLE_SCALER_DEFEATHERING      0x01
#define ENABLE_SCALER_DEFLICKERING      0x02
#define ENABLE_SCALER_DEBOUNCING        0x04
#define ENABLE_SCALER_DYNAMIC_SNR       0x08
#define ENABLE_SCALER_DYNAMIC_DNR       0x10


#define PQL_TABLE_END               0xFFFF
#define REG_ADDR_SIZE         2
#define REG_MASK_SIZE         1


typedef enum {
    PQ_IP_COM,
    PQ_IP_MAIN,
} EN_PQ_IPTYPE;

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------
/// Reset type
typedef enum
{
    SC_RST_ALL      = BIT0,
    SC_RST_IP_F1    = BIT1,
    SC_RST_IP_F2    = BIT2,
    SC_RST_OP       = BIT3,
    SC_RST_IP_ALL   = BIT4,
    SC_RST_CLK      = (BIT6|BIT3),
} HAL_SC_REST_t;

/// interrupt  //thchen 20080820
typedef enum
{
    // first 4 bits are reserved
    SC_INT_TUNE_FAIL_P = 4,
    SC_INT_VSINT,
    SC_INT_F2_VTT_CHG,
    SC_INT_F1_VTT_CHG,
    SC_INT_F2_VS_LOSE,
    SC_INT_F1_VS_LOSE,
    SC_INT_F2_JITTER,
    SC_INT_F1_JITTER,
    SC_INT_F2_IPVS_SB,
    SC_INT_F1_IPVS_SB,
    SC_INT_F2_IPHCS_DET,
    SC_INT_F1_IPHCS_DET,
    SC_INT_F2_IPHCS1_DET,
    SC_INT_F1_IPHCS1_DET,
    SC_INT_F2_HTT_CHG,
    SC_INT_F1_HTT_CHG,
    SC_INT_F2_HS_LOSE,
    SC_INT_F1_HS_LOSE,
    SC_INT_F2_DVI_CK_LOSE,
    SC_INT_F1_DVI_CK_LOSE,
    SC_INT_F2_CSOG,
    SC_INT_F1_CSOG,
    SC_INT_F2_ATS_READY,
    SC_INT_F1_ATS_READY,
    SC_INT_F2_ATP_READY,
    SC_INT_F1_ATP_READY,
    SC_INT_F2_ATG_READY,
    SC_INT_F1_ATG_READY,
} HAL_SC_INT_e;

/// source mux
typedef enum
{
    SC_SRCMUX_ANALOG_1 = 0,
    SC_SRCMUX_ANALOG_2 = 1,
    SC_SRCMUX_ANALOG_3 = 2,
    SC_SRCMUX_DVI      = 3,
    SC_SRCMUX_VIDEO    = 4,
    SC_SRCMUX_HDTV     = 5,
    SC_SRCMUX_HDMI     = 7,		//original setting 6, LGE jguy
} HAL_SC_SRCMUX_e;

/// Input sync type
typedef enum
{
    SC_ISYNC_AUTO       = 0,
    SC_ISYNC_SEPARATED  = BIT5,
    SC_ISYNC_COMPOSITE  = BIT6,
    SC_ISYNC_SOG        = (BIT5|BIT6),
} HAL_SC_ISYNC_e;

/// Video port select
typedef enum
{
    SC_VPSELECT_CCIR656_A   = 0,
    SC_VPSELECT_VD_A        = 1,
    SC_VPSELECT_CCIR601     = 2,
    SC_VPSELECT_VD_B        = 3,
    SC_VPSELECT_CCIR656_B   = 0x20,
} HAL_SC_VPSELECT_e;

// MHal_SC_EnableClock
typedef enum
{
    SC_ENCLK_TTL            = 0x00,
    SC_ENCLK_SIGNAL_LVDS    = 0x11,
    SC_ENCLK_DUAL_LVDS      = 0x13,
} HAL_SC_ENCLK_e;

// MHal_SC_SetOutputSycCtrl
typedef enum
{
    SC_OUTPUTSYNC_MODE_0,
    SC_OUTPUTSYNC_MODE_1,
    SC_OUTPUTSYNC_MODE_2,
    SC_OUTPUTSYNC_MODE_3,	//PDP Normal
    SC_OUTPUTSYNC_MODE_4,	//PDP RGB & DVI
    SC_OUTPUTSYNC_MODE_5,	//PDP Booting
} HAL_SC_OUTPUTSYNC_MODE_e;

// MHal_SC_SetGOPSEL
typedef enum
{
    MS_IP0_SEL_GOP0,
    MS_IP0_SEL_GOP1,
    MS_IP1_SEL_GOP0,
    MS_IP1_SEL_GOP1,
    MS_NIP_SEL_GOP0,
    MS_NIP_SEL_GOP1
} HAL_SC_IPSEL_GOP;

//[100118_Leo]
typedef enum
{
    HAL_TI_10BIT_MODE = 0,
    HAL_TI_8BIT_MODE = 2,
    HAL_TI_6BIT_MODE = 3,
} HAL_SC_TIMODES;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  IP Mux
//------------------------------------------------------------------------------
void MHal_SC_IPMuxSet(U8 u8DataMux, U8 u8ClkMux);

//------------------------------------------------------------------------------
//  Init
//------------------------------------------------------------------------------
void MHal_SC_Reset(HAL_SC_REST_t reset);
void MHal_SC_RegInit(U32 u32MemAddr, U32 u32MemSize);

//------------------------------------------------------------------------------
//  GOPINIT
//------------------------------------------------------------------------------
void MHal_SC_GOPINT_SetGOPEnable(BOOL bEnable);
//20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MHal_SC_SaveGOPSetting(void);
void MHal_SC_RestoreGOPSetting(void);
void MHal_SC_GOPINT_SetGOP_TO_IP(U8 gop, U8 channel, BOOL enable);
void MHal_SC_GOPINT_SetGOP_TO_VOP(U8 gop, BOOL enable);
void MHal_SC_SetGOPSEL(U8 u8IPSelGOP);
void MHal_SC_EnableInterrupt(HAL_SC_INT_e u8IntSrc, BOOL bEnable);  //thchen 20080820
U32  MHal_SC_GetInterruptSts(void);  //thchen 20080820

//------------------------------------------------------------------------------
//  IP
//------------------------------------------------------------------------------
void MHal_SC_SetSourceMux(HAL_SC_SRCMUX_e mux);
void MHal_SC_SetInputSyncType(HAL_SC_ISYNC_e isync);
void MHal_SC_SetVideoPortSelect(HAL_SC_VPSELECT_e vpselect);

void MHal_SC_InitForVGA(void);
void MHal_SC_InitForYPbPr(void);
void MHal_SC_InitForHDMI(void);
void MHal_SC_InitForVD(void);
void MHal_SC_InitForDC(void);
void MHal_SC_BrightnessInit(void);
void MHal_SC_SetFieldDetect(U8 u8IP1F2_1D, U16 u8IP1F2_21, U8 u8IP1F2_23);
void MHal_SC_SetRegeneratedDE(BOOL bEnable);


void MHal_SC_SetCapWin(U16 u16HStart, U16 u16HSize, U16 u16VStart, U16 u16VSize);
void MHal_SC_GetCapWin(U16 *u16HStart, U16 *u16HSize, U16 *u16VStart, U16 *u16VSize); //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MHal_SC_IP1_SetInputSourceEnable(BOOL bEnable, BOOL bDelayOn); // LGE drmyung 081024
void MHal_SC_IP1_CtrlHistoDataReport(BOOL bEnable);

void MHal_SC_IP1_ResetSyncDetect(void);
U16  MHal_SC_IP1_GetDetectSyncStatus(void);

void MHal_SC_IP1_SetCoastWin(U8 u8Start, U8 u8End);

U16  MHal_SC_IP1_GetHorizontalDE(void);
U16  MHal_SC_IP1_GetVerticalDE(void);
U16  MHal_SC_IP1_GetHorizontalDEStart(void);
U16  MHal_SC_IP1_GetVerticalDEStart(void);

void MHal_SC_IP1_SetForceInputClkDiv(BOOL bEnable);
void MHal_SC_IP1_SetCSCDither(BOOL bEnable);
void MHal_SC_IP1_EnableAutoGain(void);
void MHal_SC_IP1_DisableAutoGain(void);
void MHal_SC_IP1_SetSampleHStart(U16 u16SHS);
void MHal_SC_IP1_SetSampleVStart(U16 u16SVS);
void MHal_SC_IP1_SetYLock(U8 u8YLock);
void MHal_SC_IP1_SetFramLock(BOOL bFramelockEnable);

//-------------------------------------------------------------------------------------------------
//  IP2
//-------------------------------------------------------------------------------------------------
void MHal_SC_IP2_SetCSC(BOOL bEnable, BOOL bUseVIPCSC); // 20090828 daniel.huang: add VIP CSC for PCRGB & DVI
void MHal_SC_IP2_SetCSCDither(BOOL bEnable);//thchen 20080719
BOOL MHal_SC_IP2_GetCSC(void);  // 20091021 daniel.huang: add ipmux test pattern for inner test pattern
BOOL MHal_SC_VIP_GetCSC(void);  // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
U32  MHal_SC_IP2_CalHSD(U16 u16Source, U16 u16Target, BOOL bAdvMode);
U32  MHal_SC_IP2_CalVSD(U16 u16Source, U16 u16Target);

void MHal_SC_IP2_SetHSDRatio(U32 u32Ratio);
void MHal_SC_IP2_SetVSDRatio(U32 u32Ratio);
U16 MHal_SC_IP2_GetFrameLineCnt(void);  // 20090921 daniel.huang: for mirror mode


//-------------------------------------------------------------------------------------------------
//  Chip Top
//-------------------------------------------------------------------------------------------------
void MHal_SC_SetClk(void);

typedef enum
{
    CHIPTOP_FICK_F2_IDCLK2 = 0,
    CHIPTOP_FICK_F2_FICK   = 1,
    CHIPTOP_FICK_F2_XTAL   = 4,
} CHIPTOP_FICK_F2_SRC_e;
void MHal_TOP_SetFickF2_ClkSrc(CHIPTOP_FICK_F2_SRC_e clksrc);
// IOCTL_SC_Set_FrameColor_TABLE

void MHal_SC_SetDEOnlyMode(BOOL bEnable);

//-------------------------------------------------------------------------------------------------
//  IPM
//-------------------------------------------------------------------------------------------------

void MHal_SC_IPM_SetFBL(BOOL bEnable);
//#if REFINE_FPLL
BOOL MHal_SC_IPM_GetFBL(void);
//#endif

void MHal_SC_IPM_SetMemBaseAddr(U32 u32Base0, U32 u32Base1, U32 u32Base2);
void MHal_SC_IPM_SetMemFetchOffset(U16 u16Fetch, U16 u16Offset);
void MHal_SC_IPM_SetVLengthWriteLimit(U16 u16VCapSize, BOOL bInterlaced);
void MHal_SC_IPM_SetFreezeImg(BOOL bFreeze);
void MHal_SC_IPM_SetDNRWriteLimit(U32 u32WritelimitAddrBase);
void MHal_SC_GetFrameBufInfo(SC_FRAMEBUF_INFO_t *pFrameBufInfo);

//-------------------------------------------------------------------------------------------------
//  OPM
//-------------------------------------------------------------------------------------------------
void MHal_SC_OPM_SetMemBaseAddr(U32 u32Base0, U32 u32Base1, U32 u32Base2, U16 u16CropLBoffset);
void MHal_SC_OPM_SetMemFetchOffset(U16 u16Fetch, U16 u16Offset);
//-------------------------------------------------------------------------------------------------
//  OP1
//-------------------------------------------------------------------------------------------------
U32 MHal_SC_OP1_CalVSP(U16 u16Source, U16 u16Target);
U32 MHal_SC_OP1_CalHSP(U16 u16Source, U16 u16Target);
void MHal_SC_OP1_SetVSP(U32 u32Ratio);
void MHal_SC_OP1_SetHSP(U32 u32Ratio);
void MHal_SC_OP1_SetVLength(U16 u16VLength);
void MHal_SC_EODI_SetUpSampling(U16 u16Sequence, BOOL bEnable420CUP);//420cup controlled by driver, [100222_Leo]

#if 1// 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
U32 MHal_SC_OP1_GetHSP(void);
U32 MHal_SC_OP1_GetVSP(void);
U16 MHal_SC_OP1_GetHCropCoordiante(U16 u16Disp, BOOL bPositionOrLength);
U16 MHal_SC_OP1_GetVCropCoordiante(U16 u16Disp, BOOL bPositionOrLength);
#endif

//-------------------------------------------------------------------------------------------------
//  OP2
//-------------------------------------------------------------------------------------------------
void MHal_SC_OP2_SetOSDBlending(BOOL bEnable);
void MHal_SC_OP2_SetNoSignalColor(U8 u8Color);
void MHal_SC_OP2_SetCMC(BOOL bEnable);
void MHal_SC_OP2_SetColorMatrix(U16* pMatrix);
void MHal_SC_OP2_SetBrightness(U16 u16Brightness);//change U8 to U16, [090921_Leo]
void MHal_SC_OP2_SetBlackScreen(BOOL bEnable);
void MHal_SC_OP2_SetGammaEnable(BOOL bEnable);
void MHal_SC_OP2_SetGammaMappingMode(U8 u8Mapping);
// 20091030 daniel.huang: fix 256/1024 gamma incorrect
void MHal_SC_OP2_SetRGBGammaMaxValue(U8* pu8GammaTableR, U8* pu8GammaTableG,
                                  U8* pu8GammaTableB, U16 u16MaxCnt);
void MHal_SC_OP2_SetRGBGammaTable(U8* pu8GammaTableR, U8* pu8GammaTableG,
                                  U8* pu8GammaTableB, U16 u16From, U16 u16To); //change param, [090825_Leo]
void MHal_SC_OP2_SetFrameColor(U8 u8FrameColorR, U8 u8FrameColorG,U8 u8FrameColorB);


//------------------------------------------------------------------------------
//  VOP
//------------------------------------------------------------------------------
void MHal_SC_VOP_HVTotalSet(U16 u16HTotal, U16 u16VTotal);
void MHal_SC_VOP_HSyncWidthSet(U8 u8HSyncWidth);
void MHal_SC_VOP_SetDEWin(U16 u16HStart, U16 u16HEnd, U16 u16VStart, U16 u16VEnd);
void MHal_SC_VOP_SetDispWin(U16 u16HStart, U16 u16HEnd, U16 u16VStart, U16 u16VEnd);
void MHal_SC_VOP_OutputCtrl(U16 u16OCTRL, U16 u16OSTRL, U16 u16ODRV, U16 u16DITHCTRL);
void MHal_SC_VOP_EnableClock(HAL_SC_ENCLK_e enclk);
void MHal_SC_VOP_SetOutputSycCtrl(HAL_SC_OUTPUTSYNC_MODE_e mode, U16 u16VSyncStart, U16 u16VSyncEnd);
void MHal_SC_VOP_SetAutoVSyncCtrl(BOOL bIsAutoVSync);	// [vivakjh] 2009/01/07	PDP FHD MRE(FMC) 대응.  ==> TRUE : VSync Out Auto, FALSE : VSync Out Manual.
void MHal_SC_VOP_SetFreeRunColorEnable(BOOL bEnable);
void MHal_SC_VOP_SetFreeRunColor(SC_FREERUN_COLOR_e color);
//#if REFINE_FPLL
U16 MHal_SC_VOP_GetDispVStart(void);
U16 MHal_SC_VOP_GetDispVEnd(void);
U16 MHal_SC_VOP_GetDispHStart(void);    // 20090921 daniel.huang: for mirror mode
U16 MHal_SC_VOP_GetDispHEnd(void);      // 20090921 daniel.huang: for mirror mode
//#endif
void MHal_SC_VOP_OD_DataPath(BOOL bEnable);

//------------------------------------------------------------------------------
//  MACE
//------------------------------------------------------------------------------
// DLC
void MHal_SC_MACE_RequestHistogramData(void);
BOOL MHal_SC_MACE_WaitHistogramDataReady(void);
void MHal_SC_MACE_GetHistogram32(U16* pu16Histogram);
U8   MHal_SC_MACE_GetMaxPixelValue(void);
U8   MHal_SC_MACE_GetMinPixelValue(void);
U8   MHal_SC_MACE_GetAvgPixelValue(void);
U16  MHal_SC_MACE_GetTotalColorCount(void); //[090601_Leo]
void MHal_SC_MACE_SetLumaCurve(U16* pLumaCurve);
void MHal_SC_MACE_SetLumaCurveEnable(BOOL bEnable);
void MHal_SC_MACE_SetHistogramReqEnable(BOOL bEnable);
void MHal_SC_MACE_SetICCSaturationAdj(U8 u8ColorType, S8 s8SatAdj);
void MHal_SC_MACE_SetIBCYAdj(U8 u8ColorType, U8 u8YAdj);
void MHal_SC_MACE_SetIHCHueDiffColorYAdj(U8 u8ColorType, S8 s8HueAdj, U8 u8YIndex, U8 u8YLevel);//[090623_Leo]
void MHal_SC_MACE_SetIHCHueAdj(U8 u8ColorType, S8 s8HueAdj);
void MHal_SC_MACE_SetICCSaturationEnable(BOOL bEnable);//thchen 20080718
void MHal_SC_MACE_SetIBCYEnable(BOOL bEnable);//thchen 20080718
void MHal_SC_MACE_SetIHCHueEnable(BOOL bEnable);//thchen 20080718
void MHal_SC_MACE_SetICCRegionTable(U8 *data);   //victor 20080814
void MHal_SC_MACE_SetIHCRegionTable(U8 *data);   //victor 20080814
void MHal_SC_MACE_SetIHCYModeDiffColorEnable(BOOL bEnable); //[090623_Leo]
void MHal_SC_MACE_SetIHCYModelEnable(BOOL bEnable);//victor 20080818
void MHal_SC_MACE_SetICCYModelEnable(BOOL bEnable);//victor 20080818
void MHal_SC_SetCSCOffset(BOOL isMinus16);//victor 20080830
void MHal_SC_SelectCSC(U8 u8selection);//victor 20080830
void MHal_SC_MACE_SetBlueStretchEnable(BOOL bEnable);//victor 20080830

void MHal_SC_MACE_HistogramInit(void);//thchen 20080820
void MHal_SC_MACE_HistogramInit4PDP(void); //LGE [vivakjh] 2008/12/27	Request chaning the histogram read point(0x1a High(0x04)) to all 0 from PQ team

void MHal_SC_SetInterruptMask(HAL_SC_INT_e u8IntSrc, BOOL bEnable); //thchen 20080903

void MHal_SC_MACE_IHCHueInit(void);
void MHal_SC_MACE_DLCInit(U16 u8Histogram_Vstart, U16 u8Histogram_Vend);

void MHal_SC_SetColorAdaptiveRange(U8 u8CbUpValue, U8 u8CbDownValue, U8 u8CrUpValue, U8 u8CrDownValue);//[090601_Leo]
void MHal_SC_SetAdaptiveCGainEnable(U8 u8OnOff);//[090921_Leo]
void MHal_SC_SetAdaptiveCGain(U8 *pCGainParam);//[090814_Leo]
void MHal_SC_SetPieceWiseEnable(U8 u8OnOff, U8 *pPieceWiseXPosition);//[090825_Leo]

U16  MHal_SC_GetHorizontalDEStart(void);
U8   MHal_SC_GetModeStatus(void);
U16  MHal_SC_GetVerticalDEStart(void);
U16  MHal_SC_IP1_GetHPeriod(void);
U16  MHal_SC_IP1_GetVTotal(void);
//#if REFINE_FPLL
U16  MHal_SC_IP1_GetVDC(void);
//#endif

SC_SCART_MODE_e MHal_SC_GetScartMode(void);
void MHal_SC_SetScartOverlay(BOOL bOverlay);
void MHal_SC_GetScart1IDLevel(U8* pIDLevel);
//FitchHsu 20080929
void MHal_SC_GetScart2IDLevel(U8* pIDLevel);

void MHal_SC_SetFPLL(BOOL bSetFPLL);
void MHal_SC_SetOutputFreeRun(BOOL bEnFreerunOutput);
void MHal_SC_SetOutputVTotal(U16 u16OutputVtotal);

void MHal_SC_SetLockPoint(U16 u16LockPoint);  //thchen 20081001
void MHal_SC_SetFreezePoint(U16 u16FreezePoint);  //thchen 20081001
void MHal_SC_FreezeVCN(void);  //thchen 20081001
void MHal_SC_ResetFreezeVCN(unsigned long arg);  // 20091202 daniel.huang: use timer for vcount freeze reset
void MHal_SC_FreezeVCN4PDP(void);	//LGE [vivakjh] 2008/11/12 with cc.chen	Add for DVB PDP Panel

void MHal_SC_FilmEnable(BOOL bEnable, U8 u8Filmtype);
U8	 MHal_SC_GetFilmMode(void);	// LGE [vivakjh]  2008/12/11 	PDP 경우 Fast Frame Lock을 못하므로 Frame Lock시점까지 Film Mode를 Off 했다가 Frame Lock 되면 On해줌.

BOOL MHal_SC_IP1_GetInputSourceEnable(void); //FitchHsu 20081024 Audio Error
//------------------------------------------------------------------------------
//  PQL Dump Table
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  Other Functions
//------------------------------------------------------------------------------
U32 MHal_Scaler_ReadMotionValue1(void); //victor 20080922
void MHal_Scaler_DeFeathering(U32 u32MotionValue, BOOL IsDTV);		//victor 20080922
void MHal_Scaler_DeFlickering(U32 u32MotionValue);		//victor 20080922
void MHal_Scaler_DeBouncing(U32 u32MotionValue);		//victor 20080922
void MHal_Scaler_FilmPatch(U32 u32MotionValue);         //From victor
void MHal_SC_DynamicNRInATV(void);  //victor 20081128
void MHal_SC_PQ_FastPlayback(BOOL bEnable); //FitchHsu 20081113 EMP when PAUSE, little shaking
void MHal_SC_PQ_Protect_FastPlayback(void);  //FitchHsu 20081113 EMP when PAUSE, little shaking
#if 0 // remove this patch because no need in T3
void MHal_SC_DVICLK_UNLOCK(SC_INPUT_SOURCE_e SrcType, BOOL bEnable); // FitchHsu 20081215 DVICLK UNLOCK issue
#endif

void MHal_SC_Set3DComb(SC_SET_3DCOMB_t param);//victor 20081113, 3DComb
void MHal_SC_SetNumOfFB(U8 u8NumOfFB);//20091229 Michu, Implement frame buffer mode
void MHal_SC_FilmModeReset(void);//LGE gbtogether(081128) --> SD(Film) to HD(non-Film) issue by FitchHsu
void MHal_SC_EnableDNR(BOOL bEnable);   //victor 20081203, DNR
void MHal_SC_EnableColorFunction(BOOL bEnable); //CDNR Enable, [090615_Leo]
void MHal_SC_SetCDNRIndex(U8 *pIndex); //[090616_Leo]
void MHal_SC_SetCDNRGain(U8 *pDnrGain, U8 *pPreSnrGain); //[090617_Leo]

void MHal_Scaler_SetAdaptiveCtrl(U8 u8Ctrl); //fitch 20081222
void MHal_Scaler_AdaptiveTuning(BOOL IsDTV); //fitch 20081222

U8   MHal_SC_Pre_Memory_Motion_Gain(U8 u8MotionGain); //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode

void MHal_SC_DBKInit(void);//[091201_Leo]
void MHal_SC_SetDithering(U8 u8BitMode, BOOL bUrsapatch);//[100118_Leo]
//------------------------------------------------------------------------------
//  Menuload driver
//------------------------------------------------------------------------------
typedef struct
{
    U16 base_idx;
    U16 depth;
} MENULOAD_CMD;

void MHal_SC_ML_Init(U32 u32Addr, U32 u32Size, U8 u8MIU_num);
void MHal_SC_ML_Fire(MENULOAD_CMD *cmd);
void MHal_SC_ML_WriteData(U32 u32Addr, U16 u16Data, U16 u16Mask);
BOOL MHal_SC_ML_IsMenuloadDone(void);
void MHal_SC_ML_UpdateVariable(U16 u16Count);
BOOL MHal_SC_MLCMQ_Empty(void);

void MHal_SC_ML_ChangeBank(U8 u8Data);
void MHal_SC_ML_UpdateReg(void);
void MHal_SC_ML_Start(void);//victor 20081024, menuload
void MHal_SC_ML_End(void);//victor 20081024, menuload
void MHal_SC_ML_PreventBufOverflow(void);

void MHal_SC_SetSSC(U16 u16Periodx100Hz, U16 u16Percentx100, BOOL bEnable);

//------------------------------------------------------------------------------
//  MWE
//------------------------------------------------------------------------------
// CC Chen 20081124 MWE implement
void MHal_SC_SubWinEnable(BOOL bEnable);
void MHal_SC_SetSubDispWin(U16 u16HStart, U16 u16HEnd, U16 u16VStart, U16 u16VEnd);
void MHal_SC_SetMWEQuality(void);
void MHal_SC_SetSubWinBorder(BOOL bEnable, U8 u8Color, U8 u8Left, U8 u8Right, U8 u8Top, U8 u8Bottom);

void MHal_SC_Set_THXMode(BOOL bIsTHXMode); // FitchHsu 20081209 implement THX mode

void MHal_SC_SetFastFrameModeStatus(BOOL bEnable);
BOOL MHal_SC_GetFastFrameModeStatus(void);
void MHal_SC_ScartIDInit(void);

//------------------------------------------------------------------------------
// VIDEO MIRROR
// Michu 20090903
//------------------------------------------------------------------------------
void MHal_SC_Set_VideoMirror(BOOL bIsH, BOOL bIsV);
void MHal_SC_SetVideoMirrorAlignWidth(U8 u8AlignWidth);
//------------------------------------------------------------------------------
// End of VIDEO MIRROR
//------------------------------------------------------------------------------

// 20091021 daniel.huang: add ipmux test pattern for inner test pattern
void MHal_SC_IPMUX_SetTestPattern(BOOL bEnable, U16 u16R_Cr, U16 u16G_Y, U16 u16B_Cb);
BOOL MHal_SC_GetSplashWindow(void);
void MHal_SC_WriteRegMask(U8 u8Bank, U32 u32Addr, U16 u16Value, U16 u16Mask);
void MHal_SC_OSD_Reference(U16 u16Height);
void MHal_SC_2D_Peaking_LBS(void);



// (20100115 ykkim5)20100113 daniel.huang: for solve sog sync unstable problem for some dvd-player with 480i timing
void MHal_SC_SetMacroVisionFilter(BOOL bEnable);
void MHal_SC_SetFD_Mask(BOOL bEnable);
BOOL MHal_SC_GetScartIDLevelSelect(void); //20100202 daniel.huang
void MHal_SC_SET_ML_Protect(BOOL bEnable);//ashton_100520
#endif // _HAL_SCALER_H_
