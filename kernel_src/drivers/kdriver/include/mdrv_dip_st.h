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

#include "mdrv_types.h"

#ifndef __MDRV_DIP_ST_H__
#define __MDRV_DIP_ST_H__


#define MAX_NR_BUFF_CNT     2
#define MAX_DATA_BUFF_CNT   16
#define MAX_BUFF_WIDTH      0x440   //1088
#define MAX_BUFF_HEIGHT     0x240   //576
#define SKIP_FIELD_NUMBER   60
#define NOT_SKIP_FIELD      1

typedef enum
{
    DIP_ERR_OK,
    DIP_ERR_FAIL,
    DIP_ERR_INVALID_BUFFER_CNT,
    DIP_ERR_INVALID_BUFFER_START,
    DIP_ERR_INVALID_BUFFER_WIDTH,
    DIP_ERR_INVALID_BUFFER_HEIGHT,
    DIP_ERR_INVALID_BUFFER_SZIE
}DIP_ERRCODE;

typedef enum
{
    E_SET_NR_BUFFER = BIT0,
    E_SET_DI_BUFFER = BIT1,
    E_SET_INPUT_TYPE = BIT2,
    E_ENABLE_SNR = BIT3,
    E_ENABLE_TNR = BIT4,
    E_ENABLE_NR = BIT5,
    E_ENABLE_DI = BIT6
}DIP_FLAG;

typedef struct
{
    U32            u32FrameWidth;
    U32            u32FrameHeight;
    BOOL           bInterLace;
} dip_frameinfo_t;

typedef struct
{
    U32             u32BufWidth;
    U32             u32BufHeight;
    U32             u32BufStart;
    U32             u32BufEnd;
    U8              u8BufCnt;
} dip_buf_mgr_t;

typedef struct
{
    U8              u8HistOut[16];
} dip_hist_out_t;

typedef struct
{
    U8              u8HistDiff[16];
} dip_hist_diff_t;

typedef struct
{
    U32             u32YStart;
    U32             u32UVStart;
} dip_YUV420_start_t;

typedef struct
{
  dip_YUV420_start_t    DI_BufInfo[16];
} dip_DIbufinfo_t;


// ------------------------------------------
#if 1
typedef struct
{
    U32             u32YStart;
    U32             u32UVStart;
} yuv420_t;

typedef struct
{
    U32                     tFlag;
    U32                     u32ClipW;
    U32                     u32ClipH;
    BOOL                    bInterlance;

    U32                     u32NRW;
    U32                     u32NRH;
    U32                     u32NRStart;
    U32                     u32NREnd;
    U32                     u32NRCnt;
    U32                     u32NRDataBuf[MAX_NR_BUFF_CNT];

    U32                     u32DIW;
    U32                     u32DIH;
    U32                     u32DIStart;
    U32                     u32DIEnd;
    U32                     u32DICnt;
    yuv420_t                tDIDataBuf[MAX_NR_BUFF_CNT];

    U8                      u8DiBufWritingIndex;
    U16                     u16DiBuffStatus;// bit 0 is 0: DI Buffer 0 not ready; bit 0 is 1:DI Buffer 0 ready
    U32                     u32DiBufferFieldCount;
    U32                     u32DiBufferFrameCount;
    U32                     u32LastDiBufferFieldCount;
    U32                     u32LastDiBufferCountTime;

    U8                      u8HistOut[16];
    U8                      u8HistDiff[16];
} dip_DIP_t;


/*
#if ENABLE_DI_BUFFER_VERIFY
static U32 u32VerifyDiBufStartAddress;
static U8  u8VerifyDiBufCnt;
static U8  BufferIndex =0;
static U8* Y_TailBuf;
static U8* UV_TialBuf;
static U8* Y_SequencyBuf;
static U8* U_SequencyBuf;
static U8* V_SequencyBuf;
static U8* OutBuf;
static U8  DiBufferVerifyStart = 0;
#endif


#if DI_BUFFER_SKIP_CHECK
#define ERROR_INTERVAL_TIME 18
#define ChECK_NUMBER 300
static U32 u32DiBufferErrorFieldCount =0;
static U32 IntervalRecord[ChECK_NUMBER];
static U32 ErrorIntervalRecord[10];
static U32 EnableDiBufferTime =0;
static U32 FirstDiBufferDoneTime =0;
static U32 SecondDiBufferDoneTime =0;
#endif

#if MONITOR_INT_INTERVAL
U32 u32Time1 = 0;
U32 u32Time2 = 0;
U32 u32Time3 = 0;
U32 u32IntCount = 0;
#endif

#if NOT_SKIP_FIELD
BOOL bNotSkipField = FALSE;
#endif

#if FRAME_CHECK
U8 u8CheckWord[CHECK_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};
#endif
*/


#endif
//------------------------------------------------------------------------------
// Data structure
//------------------------------------------------------------------------------

#endif // #ifndef __MDRV_DIP_ST_H__

