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
/// @file   mdrv_ve_st.h
/// @brief  TVEncoder Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Data structure definition
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MDRV_VE_ST_H__
#define __MDRV_VE_ST_H__

typedef struct {
    BOOL bEn;
    U8*  pu8VPSData;
} stVE_VPSData;

typedef struct {
    BOOL bEn;
    U16   u16WSSData;
} stVE_WSSData;

typedef struct {
    BOOL bEn;
    U16 u16CCData0;
    U16 u16CCData1;
} stVE_CCData;


typedef struct {
    U32 u32nrLines;
    U32 u32LineFlag;
    U32 u32Size;
    U32 u32PacketAddr;
    U8  u8Success;
} stVE_TTXData;

typedef struct {
    BOOL bScartBypassEnable;
    U8 u8ByPassInputPortType;
} stVE_SCARTOUTBYPASS;

#endif  // #ifndef _MDRV_VE_H_
