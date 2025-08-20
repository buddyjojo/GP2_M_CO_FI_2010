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
/// @file   mdrv_ttx_st.h
/// @brief  Teletext Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Data structure definition
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MDRV_TTX_ST_H__
#define __MDRV_TTX_ST_H__

struct VBI_PARAMETERS
{
    U32 _u32PacketUnitNo;
    U32 _u32GetAdr;
    U32 _u32PutAdr;
    U32 _u32Overflow;
    U32 _u32PacketUnit;
    U32 _u32Status;
    U32 _u32VPS;
    U32 _u32Protect;
    U32 _u32WSS;
};

typedef enum
{
    E_TTX =         0x1,
    E_VPS =         0x2,
    E_WSS =         0x4,
    E_NO_EVENT =    0x0
} EVENT_TTX;

#endif  // #ifndef _MDRV_TTX_H_
