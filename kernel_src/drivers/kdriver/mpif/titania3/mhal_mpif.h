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
/// file    mhal_mpif.h
/// @brief  MPIF HAL layer Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _HAL_MPIF_H_
#define _HAL_MPIF_H_

#include "mdrv_types.h"
#include "mhal_mpif_reg.h"

BOOL MHal_MPIF_Init(U8 u8clk, U8 u8trc, U8 u8wc);
BOOL MHal_MPIF_BusyWait_ChannelFree(U8 event_bit, U32 timeout);


// Function prototype
BOOL MHal_MPIF_LC1A_RW(U8 u8bWite, U8 slaveid, U8 index, U8* pu8data);
BOOL MHal_MPIF_LC2A_RW(U8 u8bWite, U8 slaveid, U8 checksum, U8 rtx, U16 wordaddr, U16 *pu16data);
BOOL MHal_MPIF_LC2B_RW(U8 u8bWite, U8 slaveid, U8 checksum, U8 rtx, U16 wordaddr, U16 *pu16data);
BOOL MHal_MPIF_LC3A_MIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U32 miu_addr, U32 spif_mdr, U32 n);
BOOL MHal_MPIF_LC3A_RIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U8 *pu8data, U16 n);
BOOL MHal_MPIF_LC3B_MIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U32 miu_addr, U32 spif_mdr, U32 n);
BOOL MHal_MPIF_LC3B_RIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U8 *pu8data, U16 n);
BOOL MHal_MPIF_LC4A_MIURW(U8 u8bWite, U8 slaveid, U8 rtx, U8 miu_id, U8 bkpt, U8 wcnt, U32 miu_addr, U32 spif_mdr, U16 n);

BOOL MHal_MPIF_SetCmdDataMode(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth);
BOOL MHal_MPIF_SetTrcWc(U8 u8trc, U8 u8wc);
BOOL MHal_MPIF_GetSPIF_ChBusySts(U8 u8slaveid, U16 u16event_bit, U32 u32timeout);
BOOL MHal_MPIF_SetSlave_ClkInv_Delay(U8 u8slaveid, U8 u8DlyBufNum);

#endif
