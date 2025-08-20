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
#include "mhal_ttx_types.h"
#include "mhal_ttx_reg.h"

#ifndef _MHAL_TTX_H
#define _MHAL_TTX_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ttx_.h
/// This file contains the Mstar driver interface for Teletext
/// @author MStar Semiconductor Inc.
/// @brief  Teletext module
///////////////////////////////////////////////////////////////////////////////

// MStar andy 080930
#define OPT_TTX_HAL_DEBUG
#undef TTX_HAL_DBG
#ifdef OPT_TTX_HAL_DEBUG
    #define TTX_HAL_DBG(fmt, args...)      printk(KERN_WARNING "[TTX_HAL][%05d]" fmt, __LINE__, ## args)
#else
    #define TTX_HAL_DBG(fmt, args...)
#endif

#undef TTX_HAL_DBGX
#define TTX_HAL_DBGX(fmt, args...)


extern U32  _VBIBufferBaseAddrTTX;
extern U32  _VBIBufferUnitTTX;
extern U8   _u8Byte2InvByte[256];

extern atomic_t _aInterruptStop;

#define TT_VBI_PACKET_SIZE       48
#define TT_VBI_BUFFER_BASE_ADDR  _VBIBufferBaseAddrTTX
#define TT_VBI_BUFFER_UNIT       _VBIBufferUnitTTX
#define TT_VBI_BUFFER_SIZE       (TT_VBI_BUFFER_UNIT*TT_VBI_PACKET_SIZE)


extern void MHal_TTX_SetVideoSystem(VIDEOSTANDARD_TYPE etVideoStandard);
extern void MHal_TTX_InitVBI(void);
extern void MHal_TTX_PacketBuffer_Create (void);
extern void MHal_TTX_OnOffVBISlicer(B16 on);

extern U32 MHal_TTX_ReadIRQ(void);
extern void MHal_TTX_ClearIRQ(void);
extern void MHal_TTX_EnableInterrupt(B16 bEnable);
extern U32 MHal_TTX_VPS_Count(void);
extern U32 MHal_TTX_WSS_Count(void);
extern U32 MHal_TTX_TXX_Count(void);

extern B16 MHal_TTX_GatherVpsData(U8* pu8buf);
extern U16 MHal_TTX_ReadVpsCni(void);
extern U16 MHal_TTX_ReadWSS(void);
extern B16 MHal_TTX_IsValidPalSignal(void);	// MStar andy 081110

#endif

