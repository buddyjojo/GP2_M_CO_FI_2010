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
/// @file   mdrv_micom.h
/// @brief  MICOM Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access MICOM.
///     - Provide functions to initialize MICOM
///     - Provide MICOM ISR.
///     - Provide MICOM callback function registration for AP.
///
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_MICOM_H_
#define _MDRV_MICOM_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mdrv_micom_st.h"
#include "mdrv_micom_io.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define KEYPAD_STABLE_NUM 10
#define KEYPAD_STABLE_NUM_MIN 9

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
//PM Section
PM_Result MDrv_MICOM_Init(PM_WakeCfg_t *pPmWakeCfg);
PM_Result MDrv_MICOM_GetStatus(PM_DrvStatus *pDrvStatus);
PM_Result MDrv_MICOM_GetLibVer(PM_LibVer_t *pPmLibVer);
PM_Result MDrv_MICOM_GetInfo(PM_DrvInfo_t *pPmDrvInfo);
PM_Result MDrv_MICOM_SetDbgLevel(PM_DbgLv eLevel);
PM_Result MDrv_MICOM_PowerDown(void);
PM_Result MDrv_MICOM_Reset(void);
PM_Result MDrv_MICOM_Control(void);
PM_Result MDrv_MICOM_TxData(void);
PM_Result MDrv_MICOM_RxData(void);
B16 MDrv_MICOM_RegWrite( U16 u16Addr, U8 u8Data );
U8 MDrv_MICOM_RegRead( U16 u16Addr );
B16 MDrv_MICOM_ReceiveMailFromMicom(U8 *u8MBIndex, U16 *u16MBData);
//RTC Section
void MDrv_MICOM_Rtc_Init(void);
void MDrv_MICOM_Rtc_SetCounter(U32 u32Counter);
U32 MDrv_MICOM_Rtc_GetCounter(void);
void MDrv_MICOM_Rtc_SetMatchCounter(U32 u32Counter);
U32 MDrv_MICOM_Rtc_GetMatchCounter(void);
void MDrv_MICOM_Rtc_ClearInterrupt(void);
void MDrv_MICOM_Rtc_Test(void);

//SAR Section
void MDrv_MICOM_Sar_Init(void);
U8 MDrv_MICOM_Sar_SetChInfo(SAR_CFG_t *sarChInfo);
U8 MDrv_MICOM_Sar_CHGetKey(U8 u8Index, U8 u8Channel, U8 *u8Key , U8 *u8Repstatus);
U8 MDrv_MICOM_Sar_GetKeyCode(U8 *u8Key, U8 *u8Repstatus);

#endif // _MDRV_MICOM_H_
