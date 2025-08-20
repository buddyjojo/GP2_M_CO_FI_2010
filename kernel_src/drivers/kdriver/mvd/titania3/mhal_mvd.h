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
/// file    Mhal_mvd.h
/// @brief  MVD HAL layer Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _HAL_MVD_H_
#define _HAL_MVD_H_

#include "mhal_mvd_reg.h"
#include "mdrv_mvd.h"
#include "mvd4_interface.h"

#define MVD_MIU1_BASE_ADDR      0x10000000UL

BOOL MHal_MVD_IsOnMIU1(void);

MVDSTATUS MHal_MVD_PowerOn(void);
MVDSTATUS MHal_MVD_PowerOff(void);
MVDSTATUS MHal_MVD_Init(MVD_Codec_Type codec);
MVDSTATUS MHal_MVD_Play(u8 frc);
MVDSTATUS MHal_MVD_Pause(void);
MVDSTATUS MHal_MVD_Stop(void);
MVDSTATUS MHal_MVD_Decode_IFrame(u32 u32FrameBufAddr, u32 u32StreamBufAddr);
MVDSTATUS MHal_MVD_GetDecdeFrameCount(u16 *pu16FrameCount);
MVDSTATUS MHal_MVD_GetPTS(u32 *pu32PTSLow, u32 *pu32PTSHigh);
MVDSTATUS MHal_MVD_GetPictureData(MVD_FRAMEINFO *pPictureData);
MVDSTATUS MHal_MVD_Reset(MVD_Codec_Type codec);
MVDSTATUS MHal_MVD_ToggleAVSync(u8 u8Flag);
MVDSTATUS MHal_MVD_GetActiveFormat(U32 *active_format);
U16 MHal_MVD_ReadIRQ(void);
void MHal_MVD_ClearIRQ(void);
U16 Mhal_MVD_SetIntSubscribe(U32 u32Flag);
MVDSTATUS MHal_MVD_GetPictureHeader(pMVD_PIC_HEADER_T pPIC_Hdr_Data);
MVDSTATUS MHal_MVD_GetSequenceHeader(pMVD_SEQUENCE_HEADER_T pMVD_SEQUENCE_HEADER_Tret);
MVDSTATUS MHal_MVD_GetPictureHeaderAndUserData(pMVD_Data_Pkt *ppStrLast,
                                               U32 *pu32NumOfData,
                                               U32 u32Skip);
MVDSTATUS MHal_MVD_GetAvSyncStatus(u32 *u32AVSyncStatus);
MVDSTATUS MHal_MVD_SetPictureUserDataBuffer(U8 *pDataBuff, U32 u32SizeOfBuff);

MVDSTATUS MHal_MVD_GetBitStreamBuffer(U32 *pu32BufferStart,
                                      U32 *pu32SizeOfBuff);
MVDSTATUS MHal_MVD_GetFrameBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff);
MVDSTATUS MHal_MVD_GetUserDataBuffer(U32 *pu32BufferStart, U32 *pu32SizeOfBuff);

MVDSTATUS MHal_MVD_PVRPlayMode(MVD_PVR_PlayMode PlayMode, U8  u8FrameRateUnit);
MVDSTATUS MHal_MVD_SetFilePlay(U32 u32StartAddr, U32 u32EndAddr);
MVDSTATUS MHal_MVD_SetFileClose(void);
MVDSTATUS MHal_MVD_SetFilePlay2(U8 u8PlayFrameRate, U8  u8Mode);


U8 MHal_MVD_GetFirstFrame(void);
U8 MHal_MVD_GetDispRdy(void);
U8 MHal_MVD_GetSyncStatus(void);
U8 MHal_MVD_SetVOPDone(void);
U8 MHal_MVD_GetProgInt(void);
U8 MHal_MVD_GetVideoSkip(void);
U8 MHal_MVD_GetVideoRepeat(void);
U8 MHal_MVD_IPicFound(void);

void MHal_MVD_SetFrameBufferAddr(U32 u32addr);
void MHal_MVD_SetSLQStartEnd(U32 u32start, U32 u32end);
void MHal_MVD_SetCodecInfo(U8 u8CodecType,
                           U8 u8BSProviderMode,
                           U8 bDisablePESParsing);
MVDSTATUS MHal_MVD_SetDelay(U32 delayTime);
MVDSTATUS MHal_MVD_SetSyncThreshold(U32 threshold);
MVDSTATUS MHal_MVD_SetPlayMode(U32 u32Mode);
MVDSTATUS MHal_MVD_StepPlay(void);
void MHal_MVD_SetOverflowTH(U32 u32Threshold);
void MHal_MVD_SetUnderflowTH(U32 u32Threshold);
MVDSTATUS MHal_MVD_GetESDataSize(U32 *pu32DataSize);
MVDSTATUS MHal_MVD_SetAVSyncThreshold(u16 u16Threshold);
MVDSTATUS MHal_MVD_GetESRdPtr(U32 *u32Ptr);
MVDSTATUS MHal_MVD_GetESWrPtr(U32 *u32Ptr);
MVDSTATUS MHal_MVD_SetSyncRepeatTH(U32 u32RepeatTH);
MVDSTATUS MHal_MVD_SetSkipRepeatMode(U8 u8Mode);

#endif
