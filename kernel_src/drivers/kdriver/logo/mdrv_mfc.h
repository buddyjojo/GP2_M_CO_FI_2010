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
///
/// file    drvMfc.c
/// @brief  Mfc Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MDRV_MFC_H_
#define _MDRV_MFC_H_

//#include <linux/init.h>
//#include <linux/module.h>
//#include <linux/fs.h>
//#include <linux/cdev.h>
//#include <asm-mips/delay.h>
#include "mdrv_types.h"
//#include "mst_platform.h"
//#include "mdrv_mfc_io.h"
#include "mdrv_mfc_st.h"

/*
#define absGmvX(a)            (diff(a, 0x80))
#define absGmvY(a)            (diff(a, 0x40))
#define Posdiff(a,b)           (((a) > (b)) ? (a-b):0)
#define isMV0 ((gMfcType.GMVX == 0x80) && (gMfcType.GMVY == 0x40))
#define isMVNotFound ((gMfcType.GMVX == 0) && (gMfcType.GMVY == 0))
#define DIF_MAX_POS_SPEED_X (gMfcType.maxXPosSpeed-gMfcType.maxXNegSpeed)
#define DIF_MAX_POS_SPEED_Y (gMfcType.maxYPosSpeed-gMfcType.maxYNegSpeed)
#define MaxSpeedDifToGMV max((gMfcType.maxXPosSpeed-gMfcType.GMVX)>>3,(gMfcType.GMVX-gMfcType.maxXNegSpeed)>>3)
#define MinSpeedDifToGMV min((gMfcType.maxXPosSpeed-gMfcType.GMVX)>>3,(gMfcType.GMVX-gMfcType.maxXNegSpeed)>>3)
*/

//--MFC OP gain phase setting
#define _RATIO			1 // 905,969,664
#define _STEP			4 // 4,294,967,295
#define _GAIN_P(f)		(f/(_STEP))
#define _GAIN_I(f)		(f/(_STEP*_STEP/2))

//------------------------------------------------------------------------------
//  Enum
//------------------------------------------------------------------------------
typedef enum
{
    MFC_I2cUseBusN       = 0x34,
    MFC_I2cUseBus        = 0x35,
    MFC_CpuWaitN         = 0x36,
    MFC_CpuWait          = 0x37,
    MFC_SerialDebugEnter = 0x44,
    MFC_SerialDebugExit  = 0x45,
} MFC_SERIAL_DEBUG_CMD_e;
/*
typedef enum
{
    MIRROR_OFF,
    MIRROR_H_MODE,
    MIRROR_V_MODE,
    MIRROR_HV_MODE
} MFC_MirrorModeType;
*/
/*
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
} MST_MFC_SYS_INFO_t, *PMST_MFC_SYS_INFO_t;

*/

typedef enum MFC_SKIP_LEVEL  // define from T2
{
    MFC_LEVEL_OFF  = 0,
    MFC_LEVEL_LOW  = 1,
    MFC_LEVEL_MID  = 2,
    MFC_LEVEL_HIGH = 3,
    MFC_LEVEL_55   = 4,
    MFC_LEVEL_USER = 5,
    MFC_LEVEL_MASK = 0x0F,
    MFC_LEVEL_DEFAULT = MFC_LEVEL_MID
}MFC_LEVEL;
//------------------------------------------------------------------------------
// Global variable
//------------------------------------------------------------------------------
extern MST_MFC_SYS_INFO_t gmfcSysInfo;
//------------------------------------------------------------------------------
// Function prototype
//------------------------------------------------------------------------------
void MDrv_MFC_Init(PMST_MFC_SYS_INFO_t mfcSysInfo);
void MDrv_MFC_Reset(void);
void MDrv_MFC_COMPENSATION(U8 u8Blur, U8 u8Judder,MFC_LEVEL u8MFC);
void MDrv_MFC_SetJudderLevel(U8 u8Judder);
U8 MDrv_MFC_GetJudderLevel(void);
void MDrv_MFC_SetBlurLevel(U8 u8Blur);
U8 MDrv_MFC_GetBlurLevel(void);
void MDrv_MFC_SetMFCLevel(MFC_LEVEL u8MFC);
U8 MDrv_MFC_GetMFCLevel(void);
void MDrv_MFC_TrueMotionDemo(U8 u8TrueMotionDemo);
void MDrv_MFC_VideoBlock(U8 u8Type , BOOL bOnOff);
void MDrv_MFC_Set_Bypass_Window(BOOL bOnOff, U8 u8WindowID, U16 u16HStart, U16 u16VStart, U16 u16HSize, U16 u16VSize);
U16 MDrv_MFC_GetSWVersion(void);
U16 MDrv_MFC_GetBinVersion(void);
void MDrv_MFC_UpdateSW(void);
void MDrv_MFC_DemoBarControl(BOOL bEnable, BOOL bDirection, U8 u8width, U8 u8color);
void MDrv_MFC_DemoBarByRptWin(BOOL bEnable, BOOL bDirection, U8 u8width, U8 u8color);
void MDrv_MFC_OnOff(U32 arg);
void MDrv_MFC_LVDSPowerOnOFF(U32 arg);
void MDrv_MFC_SetFrameRate(U8 u8PanelVfreq);
U8 MDrv_MFC_IsStable(void);
void MDrv_MFC_SetSSC(U16 u16KHz, U16 u16Percent, BOOL bEnable, BOOL bMiuLVDS);
void MDrv_MFC_SetReverseMode(U8 u8MirrorMode, U8 isS7M);
void MDrv_MFC_SetLVDSBit(U8 u8BitNum);
void MDrv_MFC_SetLVDSVesaJeida(U32 arg);
void MDrv_MFC_DebugBlock(U32 arg);
void MDrv_MFC_GetHVTotal(U16* u16HTotal, U16* u16VTotal);
void MDrv_MFC_SlowFrameLock(U32 arg);
void MDrv_MFC_FrameLockMode(U8 u8Mode);
void MDrv_MFC_SetODCTable(void);
void MDrv_MFC_SetPWMFreq(U8 u8GroupIndex , U8 u8Frequency, BOOL bVsyncEn, U32  u32Duty, U32 u32Shift, BOOL bShiftEn);
//------------------------------------------------------------------------------
// Register Read/Write
//------------------------------------------------------------------------------
U8 MDrv_MFC_ReadByte(U16 u16Addr);
U16 MDrv_MFC_Read2Bytes(U16 u16Reg);
U32 MDrv_MFC_Read3Bytes(U16 u16Reg);
BOOL MDrv_MFC_WriteByte(U16 u16Addr, U8 u8Val);
void MDrv_MFC_Write2Bytes(U16 u16Reg, U16 u16Value);
void MDrv_MFC_Write3Bytes(U16 u16Reg, U32 u32Value);
void MDrv_MFC_WriteBit(U16 u16Addr, U8 u8Bit, U8  u8BitPos);
void MDrv_MFC_WriteByteMask(U16 u16Addr, U8 u8Val, U8 u8Mask);
void MDrv_MFC_WriteRegsTbl(U16 wIndex, MST_MFC_RegUnitType_t *pTable);

//------------------------------------------------------------------------------
// Get/Set variable
//------------------------------------------------------------------------------
U8 MDrv_MFC_GetInitStatus(void);
void MDrv_MFC_SetInitStatus(U8 status);

//For MFC frame lock that controlled by scaler
void MDrv_MFC_SetVFreq(U16 u16InputfreqX100, BOOL enableFPLL);

void _MDrv_MFC_StartTimer(void);
void _MDrv_MFC_DeleteTimer(void);

//BOOL MDrv_MFC_SetSerialDebugMode(MFC_SERIAL_DEBUG_CMD_e cmd);
void MDrv_MFC_SetSerialDebug(BOOL bOpen);


#endif
