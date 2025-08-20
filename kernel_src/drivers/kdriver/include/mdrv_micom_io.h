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
/// @file   mdrv_micom_io.h
/// @brief  micom Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access micom.
///     - Provide functions to initialize micom
///     - Provide micom ISR.
///     - Provide micom callback function registration for AP.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_MICOM_IO_H_
#define _MDRV_MICOM_IO_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mhal_mailbox_reg.h"
#include "mdrv_micom_st.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define MICOM_IOC_MAGIC                'u'

#define MDRV_MICOM_INIT                _IOWR(MICOM_IOC_MAGIC, 0, PM_WakeCfg_t)
#define MDRV_MICOM_GET_STATUS         _IOWR(MICOM_IOC_MAGIC, 1, PM_DrvStatus_t)
#define MDRV_MICOM_GET_VER             _IOWR(MICOM_IOC_MAGIC, 2, PM_LibVer_t)
#define MDRV_MICOM_GET_INFO            _IOWR(MICOM_IOC_MAGIC, 3, PM_DrvInfo_t)
#define MDRV_MICOM_SET_DBG_LEVEL      _IOWR(MICOM_IOC_MAGIC, 4, PM_DbgLv_t)
#define MDRV_MICOM_POWER_DOWN         _IO(MICOM_IOC_MAGIC, 5)
#define MDRV_MICOM_RESET               _IO(MICOM_IOC_MAGIC, 6)
#define MDRV_MICOM_CONTROL             _IO(MICOM_IOC_MAGIC, 7)
#define MDRV_MICOM_TX_DATA             _IO(MICOM_IOC_MAGIC, 8)
#define MDRV_MICOM_RX_DATA             _IO(MICOM_IOC_MAGIC, 9)
//RTC Section
#define MDRV_RTC_INIT                    _IO(MICOM_IOC_MAGIC, 10)
#define MDRV_RTC_SET_COUNTER            _IOW(MICOM_IOC_MAGIC, 11, U32)
#define MDRV_RTC_GET_COUNTER            _IOR(MICOM_IOC_MAGIC, 12, U32)
#define MDRV_RTC_SET_MATCH_COUNTER     _IOW(MICOM_IOC_MAGIC, 13, U32)
#define MDRV_RTC_GET_MATCH_COUNTER     _IOR(MICOM_IOC_MAGIC, 14, U32)
#define MDRV_RTC_CLEAR_INT              _IO(MICOM_IOC_MAGIC, 15)
#define MDRV_RTC_TEST                    _IO(MICOM_IOC_MAGIC, 16)
/* 090902_louis
//CEC Section
#define CEC_IOC_MAGIC                'c'
#define MDRV_CEC_INIT                   _IO(CEC_IOC_MAGIC, 0)
#define MDRV_CEC_CHKDEVS                _IOR(CEC_IOC_MAGIC, 1, CEC_INFO_LIST_t)
#define MDRV_CEC_RX_API                 _IOR(CEC_IOC_MAGIC, 2, CEC_INFO_LIST_t)
#define MDRV_CEC_TX_API                 _IOW(CEC_IOC_MAGIC, 3, CEC_TX_INFO_t)
#define MDRV_CEC_GET_RESULT             _IOR(CEC_IOC_MAGIC, 4, CEC_TX_INFO_t)
#define MDRV_CEC_RESPONSE               _IOR(CEC_IOC_MAGIC, 5, U8)
#define CEC_IOC_MAXNR                   6
*/
//SAR Section
#define MDRV_SAR_INIT                    _IO(MICOM_IOC_MAGIC, 23)
#define MDRV_SAR_CH_INFO                _IOWR(MICOM_IOC_MAGIC, 24, SAR_CFG_t)
#define MDRV_SAR_CH_GET_KEY             _IOWR(MICOM_IOC_MAGIC, 25, SAR_Key_t)
#define MDRV_SAR_GET_KEY_CODE          _IOWR(MICOM_IOC_MAGIC, 26, SAR_Key_t)
#define MICOM_IOC_MAXNR                27

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32MicomMajor;
    int                         s32MicomMinor;
    struct cdev                 cDevice;
    struct file_operations      MicomFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} MicomModHandle;
#if 0 // 090902_louis
typedef struct
{
    int                         s32CECMajor;
    int                         s32CECMinor;
    struct cdev                 cDevice;
    struct file_operations      CECFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} CEC_ModHandle_t;
#endif
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#endif // _MDRV_MICOM_IO_H_
