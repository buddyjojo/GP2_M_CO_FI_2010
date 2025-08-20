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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvAESDMA_st.h
/// @brief  AESDMA Driver Interface
/// @author MStar Semiconductor,Inc.
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _DRVAESDMA_ST_H_
#define _DRVAESDMA_ST_H_

//--------------------------------------------------------------------------------------------------
//  Define
//--------------------------------------------------------------------------------------------------
#define AES_ENABLE                   0x01
#define TDES_ENABLE                  0x10

/// AESDMA notification event
typedef enum //_DrvAESDMA_Event
{
    E_DRVAESDMA_EVENT_DATA_INIT         = 0x00000000,
    /// Pattern Search Ready
    E_DRVAESDMA_EVENT_PS_DONE           = 0x00000001,
    /// Pattern Search Stop
    E_DRVAESDMA_EVENT_PS_STOP           = 0x00000002,
    /// DMA Done
    E_DRVAESDMA_EVENT_DMA_DONE          = 0x00010000,
    /// DMA Pause
    E_DRVAESDMA_EVENT_DMA_PAUSE         = 0x00020000,
} DrvAESDMA_Event;

//--------------------------------------------------------------------------------------------------
//  Driver Capability
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  Type and Structure
//--------------------------------------------------------------------------------------------------
/// AESDMA DDI return value
typedef unsigned long    DRVAESDMA_RESULT;

/// @name DRVAESDMA_RESULT
/// @ref DRVAESDMA_RESULT
/// return value
/// @{
#define DRVAESDMA_OK                   0x00000000
#define DRVAESDMA_FAIL                 0x00000001
#define DRVAESDMA_INVALID_PARAM        0x00000002
#define DRVAESDMA_FUNC_ERROR           0x00000003
#define DRVAESDMA_MIU_ADDR_ERROR       0x00000004

/// @}

/// AESDMA notification function
typedef void (*P_DrvAESDMA_EvtCallback)(DrvAESDMA_Event eEvent);



typedef struct
{
    U32 u32miu0addr;
    U32 u32miu1addr;
    U32 u32miunum;
    DRVAESDMA_RESULT u32Result;
} AESDMA_INIT_t;

typedef struct
{
  BOOL bStart;
  DRVAESDMA_RESULT u32Result;
} AESDMA_START_t;

typedef struct
{
    BOOL bDescrypt;
    BOOL bMode;
    U8  *pBuf;
    U32 u32Eng;
    U32 *cipherkey;
    DRVAESDMA_RESULT u32Result;
} AESDMA_SELENG_t;

typedef struct
{
    BOOL bPSin_Enable;
    BOOL bPSout_Enable;
    U32  u32PTN;
    U32  u32Mask;
    DRVAESDMA_RESULT u32Result;
} AESDMA_SETPS_t;

typedef struct
{
    U32 u32FileinAddr;
    U32 u32FileInNum;
    U32 u32FileOutSAddr;
    U32 u32FileOutEAddr;
    DRVAESDMA_RESULT u32Result;
} AESDMA_SETFILEINOUT_t;


typedef struct
{
    DrvAESDMA_Event eEvents;
    P_DrvAESDMA_EvtCallback pfCallback;
    DRVAESDMA_RESULT u32Result;
} AESDMA_NOTIFY_t;

typedef struct
{
    U16 u16RandomVar;
    DrvAESDMA_Event eEvents;
    P_DrvAESDMA_EvtCallback pfCallback;
    DRVAESDMA_RESULT u32Result;
} AESDMA_RANDOM_t;


typedef struct
{
    DRVAESDMA_RESULT Res_Status;
} AESDMA_STATUS_t;


#endif //_DRVAESDMA_ST_H_

