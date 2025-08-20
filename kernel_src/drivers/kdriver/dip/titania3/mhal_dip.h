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
////////////////////////////////////////////////////////////////////////////////
/// @file drvDIP.h
/// @author MStar Semiconductor Inc.
/// @brief Display input process driver
////////////////////////////////////////////////////////////////////////////////

#include "mdrv_dip_st.h"

#ifndef _DRVDIP_H
#define _DRVDIP_H


#define ENABLE_DI_BUFFER_VERIFY     0
#define MONITOR_INT_INTERVAL        0
#define FRAME_CHECK                 1
#define DI_BUFFER_SKIP_CHECK        0

#if FRAME_CHECK
extern DIP_ERRCODE MHal_DIP_GetDIOutputInfo(void);
#endif
extern DIP_ERRCODE MHal_DIP_Init(BOOL bEnable);
extern DIP_ERRCODE MHal_DIP_SetFrameInfo(U32 u32FrameWidth, U32 u32FrameHeight, BOOL bInterLace);
extern DIP_ERRCODE MHal_DIP_SetIntputMode(BOOL bVD);
extern DIP_ERRCODE MHal_DIP_SetNRBuff(U8 u8BufCnt, U32 u32BufWidth, U32 u32BufHeight, U32 u32BufStart, U32 u32BufEnd);
extern DIP_ERRCODE MHal_DIP_SetDIBuff(U8 u8BufCnt, U32 u32BufWidth, U32 u32BufHeight, U32 u32BufStart, U32 u32BufEnd);
extern U8* MHal_DIP_GetHistogramOut(void);
extern U8* MHal_DIP_GetHistogramDiff(void);
extern DIP_ERRCODE MHal_DIP_EnableNRandDI(BOOL bEnableNR, BOOL bEnableSNR, BOOL bEnableTNR, BOOL bEnableDI);
extern U8 MHal_DIP_GetDiBuffCount(void);
extern dip_DIbufinfo_t* MHal_DIP_GetDiBuffInfo(void);
extern U16 MHal_DIP_GetDiBuffStatus(void);
extern DIP_ERRCODE MHal_DIP_ClearDiBuffStatus(U8 BufferIndex);
extern U32 MHal_DIP_GetDiBuffFrameCount(void);
extern void MHal_DIP_ShowFieldInfo(void);

#if ENABLE_DI_BUFFER_VERIFY
extern void MHal_DIP_TailYUV420ToSequenceYUVBlock(void);
extern void MHal_DIP_SequenceYUVBlockToYUV422(void);
extern U8 MHal_DIP_GetDiBufferVerifyStart(void);
extern void MHal_DIP_SetDIVerBuff(U32 u32BufStart);
#endif

#endif //_DRVDIP_H

