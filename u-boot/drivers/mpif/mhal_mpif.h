////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

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

// Function prototype
BOOL MHal_MPIF_LC1A_RW(U8 u8bWite, U8 slaveid, U8 index, U8* pu8data);
BOOL MHal_MPIF_LC2A_RW(U8 u8bWite, U8 slaveid, U8 checksum, U8 rtx, U16 addr, U16 *pu16data);
BOOL MHal_MPIF_LC2B_RW(U8 u8bWite, U8 slaveid, U8 checksum, U8 rtx, U16 addr, U16 *pu16data);
BOOL MHal_MPIF_LC3A_MIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U32 miu_addr, U32 spif_mdr, U32 n);
BOOL MHal_MPIF_LC3A_RIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U8 *pu8data, U16 n);
BOOL MHal_MPIF_LC3B_MIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U32 miu_addr, U32 spif_mdr, U32 n);
BOOL MHal_MPIF_LC3B_RIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U8 *pu8data, U16 n);
BOOL MHal_MPIF_LC4A_MIURW(U8 u8bWite, U8 slaveid, U8 rtx, U8 miu_id, U8 bkpt, U8 wcnt, U32 miu_addr, U32 spif_mdr, U16 n);

BOOL MHal_MPIF_SetCmdDataMode(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth);
BOOL MHal_MPIF_SetTrcWc(U8 u8trc, U8 u8wc);

#endif
