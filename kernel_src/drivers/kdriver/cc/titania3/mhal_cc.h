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
#include <asm/atomic.h>

#include "mdrv_types.h"
#include "mhal_cc_types.h"
#include "mhal_cc_reg.h"

#ifndef _MHAL_CC_H
#define _MHAL_CC_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_cc_.h
/// This file contains the Mstar driver interface for Close Caption
/// @author MStar Semiconductor Inc.
/// @brief  Close Caption module
///////////////////////////////////////////////////////////////////////////////


extern U32  _VBIBufferBaseAddrTTX;
extern U32  _VBIBufferUnitTTX;
extern U8   _u8Byte2InvByte[256];



#define TT_VBI_PACKET_SIZE       48
#define CCVBI_RINGBUFFER_START_ADR  _VBIBufferBaseAddrTTX
#define TT_VBI_BUFFER_UNIT       _VBIBufferUnitTTX
#define TT_VBI_BUFFER_SIZE       (TT_VBI_BUFFER_UNIT*TT_VBI_PACKET_SIZE)


extern void MHal_CC_SetVideoSystem(VIDEOSTANDARD_TYPE etVideoStandard);
extern void MHal_CC_InitVBI(void);
extern void MHal_CC_PacketBuffer_Create (void);
extern void MHal_CC_OnOffVBISlicer(B16 on);

extern U32 MHal_CC_ReadIRQ(void);
extern void MHal_CC_ClearIRQ(void);
extern void MHal_CC_EnableInterrupt(B16 bEnable);
extern U32 MHal_CC_VPS_Count(void);
extern U32 MHal_CC_WSS_Count(void);
extern U32 MHal_CC_TXX_Count(void);
extern U16 MHal_CC_GetPacketCount(void); // drmyung LGE 080625

extern U16 MHal_CC_ReadVpsCni(void);
extern void MHal_CC_SetSlicingRegion(U16 u16TopLineStart, U16 u16TopLineEnd, U16 u16BtnLineStart, U16 u16BtnLineEnd, BOOL IsNTSC);
extern void MHal_CC_SetSlicingSys(VIDEOSTANDARD_TYPE sys_type);

#endif

