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
#ifndef _MADP_MFC_H_
#define _MADP_MFC_H_

//#include "mdrv_types.h"
//#include "mdrv_mfc_io.h"
//#include "mdrv_mfc_st.h"
#include "mdrv_mfc.h"

//------------------------------------------------------------------------------
//  Functions prototype
//------------------------------------------------------------------------------
void MAdp_MFC_SpiLoadCode(void);
void MAdp_MFC_Init(PMST_MFC_SYS_INFO_t mfcSysInfo, U8 u8IsS7M);
void MAdp_MFC_OnOff(BOOL bOnOff);
void MAdp_MFC_Compensation(U8 u8Blur, U8 u8Judder, U8 u8MFC);
void MAdp_MFC_True_Motion_Demo(U8 u8TrueMotionDemo);
void MAdp_MFC_Video_Block(U8 u8Ctrl, BOOL bCtrl);
void MAdp_MFC_LVDSOnOff(BOOL bOnOff);
void MAdp_MFC_BypassWindow(BOOL bOnOff, U8 u8WinId, U16 u16Hstart, U16 u16Vstart, U16 u16Hsize, U16 u16Vsize);
U16 MAdp_MFC_GetFirmwareVersion(void);
U16 MAdp_MFC_GetFirmwareVersionFromBin(void);
void MAdp_MFC_SetSpreadSpectrum(U16 u16KHz, U16 u16Percent, BOOL bEnable, BOOL bMiuLVDS);
void MAdp_MFC_OnOffDebugBlock(BOOL bOnOff);
void MAdp_MFC_DemoBarControl(BOOL bOnOFF, BOOL bDirection, U8 u8Width, U8 u8Color);
U8 MAdp_MFC_GetModelType(void);
void MAdp_MFC_ReadHVTotal(U16* pHTotal, U16* pVTotal);
U8 MAdp_MFC_IsStable(void);
void MAdp_MFC_SetLVDSVesaJeida(BOOL bVesaJeida);
void MAdp_MFC_SetFrameRate(U8 u8frameRate);
void MAdp_MFC_SetSlowFrameLock(BOOL bEnable);
void MAdp_MFC_ControlFrameLockMode(U8 u8type);
void MAdp_MFC_RESET(void);
void MAdp_MFC_UpdateSW(void);
void MAdp_MFC_SetReverseControl(U8 u8Mode);
void MAdp_MFC_SetByPassWindowReverse(U8 u8Mode);
void MAdp_MFC_SetLVDSBitNum(U8 u8LVDSBitNum);
void MAdp_MFC_ReadODCTable(void);
void MAdp_MFC_SetODCTable(const U8* pODTbl);
void MAdp_MFC_SetPWM(U8 u8Duty, BOOL swap);
void MAdp_MFC_SetURSAPLLSET(U16 u16InputfreqX100, BOOL enableFPLL);
U8 MAdp_MFC_ReadByte(U16 u16Addr);
U16 MAdp_MFC_Read2Bytes(U16 u16Addr);
U32 MAdp_MFC_Read3Bytes(U16 u16Addr);
void MAdp_MFC_WriteByte(U16 u16Addr, U8 u8Value);
void MAdp_MFC_Write2Bytes(U16 u16Reg, U16 u16Value);
void MAdp_MFC_Write3Bytes(U16 u16Reg, U32 u32Value);
void MAdp_MFC_WriteBit(U16 u16Addr, U8 u8Bit, U8  u8BitPos);
void MAdp_MFC_WriteByteMask(U16 u16Addr, U8 u8Val, U8 u8Mask);
void MAdp_MFC_Write2BytesMask(U16 wReg, U16 wValue, U16 wMask);
void MAdp_MFC_SoftwareResetScaler(BOOL enable);

void MAdp_MFC_SetJudderLevel(U8 u8Judder);
U8 MAdp_MFC_GetJudderLevel(void);
void MAdp_MFC_SetBlurLevel(U8 u8Judder);
U8 MAdp_MFC_GetBlurLevel(void);
void MAdp_MFC_SetMFCLevel(U8 u8MFC);
U8 MAdp_MFC_GetMFCLevel(void);
void MAdp_MFC_SetOPMFC(BOOL bEnable, BOOL bIs8Bits);
void MAdp_MFC_SetTCONPowerOff(void);

BOOL MAdp_MFC_IsNotLvdsAndBeforeB(void);

#endif /* _MADP_MFC_H_ */
