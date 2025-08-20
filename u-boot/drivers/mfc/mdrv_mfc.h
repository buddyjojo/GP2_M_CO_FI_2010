////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
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
#include "mst_platform.h"
#include "mdrv_types.h"
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
    MFC_SetAddrFmt2Byte  = 0x51,
    MFC_DisableNewCfg    = 0x7E,
    MFC_EnableNewCfg     = 0x7F,
    MFC_ChNumBit0Disable = 0x80,
    MFC_ChNumBit0Enable  = 0x81,
    MFC_ChNumBit1Disable = 0x82,
    MFC_ChNumBit1Enable  = 0x83,
    MFC_ChNumBit2Disable = 0x84,
    MFC_ChNumBit2Enable  = 0x85,

} MFC_SERIAL_DEBUG_CMD_e;

typedef enum MFC_SKIP_LEVEL  // define from T2
{
    MFC_LEVEL_OFF  = 0,
    MFC_LEVEL_LOW  = 1,
    MFC_LEVEL_MID  = 2,
    MFC_LEVEL_HIGH = 3,
    MFC_LEVEL_55   = 4,
    MFC_LEVEL_USER = 5,
    MFC_LEVEL_3D   = 0x0A,
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
void MDrv_MFC_OnOff(BOOL arg);
void MDrv_MFC_LVDSPowerOnOFF(BOOL arg);
void MDrv_MFC_SetFrameRate(U8 u8PanelVfreq);
U8 MDrv_MFC_IsStable(void);
void MDrv_MFC_SetSSC(U16 u16KHz, U16 u16Percent, BOOL bEnable, BOOL bMiuLVDS);
void MDrv_MFC_SetReverseMode(U8 u8MirrorMode, U8 isS7M);
void MDrv_MFC_SetByPassWindowReverse(U8 u8Mode);
void MDrv_MFC_SetLVDSBit(U8 u8BitNum);
void MDrv_MFC_SetLVDSVesaJeida(BOOL arg);
void MDrv_MFC_DebugBlock(BOOL arg);
void MDrv_MFC_GetHVTotal(U16* u16HTotal, U16* u16VTotal);
void MDrv_MFC_SlowFrameLock(BOOL arg);
void MDrv_MFC_FrameLockMode(U8 u8Mode);
void MDrv_MFC_SetODCTable(const U8* pODTbl);
void MDrv_MFC_SetOPMFC(BOOL bEnable, BOOL bIs8Bits);
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
void MDrv_MFC_Write2BytesMask(U16 wReg, U16 wValue, U16 wMask);
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

void MDrv_MFC_SetSerialDbgCh2(BOOL enable);

#endif
