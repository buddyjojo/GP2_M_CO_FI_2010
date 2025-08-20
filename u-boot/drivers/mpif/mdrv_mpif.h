////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
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
/// file    mdrv_mpif.h
/// @brief  MPIF Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MDRV_MPIF_H_
#define _MDRV_MPIF_H_

#include "mdrv_types.h"
#include "mdrv_mpif_st.h"


//-------------------------------------------------------------------------------------------------
//  Function
//-------------------------------------------------------------------------------------------------

BOOL MDrv_MPIF_InitSPIF(U8 u8slaveid);
BOOL MDrv_MPIF_SetCmdDataWidth(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth);
BOOL MDrv_MPIF_SafeSetCmdDataWidth(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth);

BOOL MDrv_MPIF_Init(U8 u8clk, U8 u8trc, U8 u8wc);
BOOL MDrv_MPIF_LC1A_RW(U8 u8bWite, U8 slaveid, U8 index, U8* pu8data);
BOOL MDrv_MPIF_LC2A_RW(U8 u8bWite, U8 slaveid, U16 addr, U16 *pu16data);
BOOL MDrv_MPIF_LC2B_RW(U8 u8bWite, U8 slaveid, U16 addr, U16 *pu16data);

U8 MDrv_MPIF_ReadByte(U8 u8slaveid, U16 u16Addr);
U16 MDrv_MPIF_Read2Byte(U8 u8slaveid, U16 u16Addr);
U32 MDrv_MPIF_Read3Byte(U8 u8slaveid, U16 u16Addr);
BOOL MDrv_MPIF_WriteByte(U8 u8slaveid, U16 u16Addr, U8 u8Val);
BOOL MDrv_MPIF_Write2Byte(U8 u8slaveid, U16 u16Addr, U16 u16Val);
BOOL MDrv_MPIF_Write3Byte(U8 u8slaveid, U16 u16Addr, U32 u32Val);

#endif
