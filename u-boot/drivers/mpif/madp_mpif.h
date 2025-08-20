////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
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
#ifndef _MADP_MPIF_H_
#define _MADP_MPIF_H_

#include "mdrv_types.h"
#include "mdrv_mpif_io_uboot.h"
#include "mdrv_mpif_st.h"
//------------------------------------------------------------------------------
//  Definition
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Structure
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

void MAdp_MPIF_Open(void);
void MAdp_MPIF_Init(U8 u8clk, U8 u8trc, U8 u8wc);
void MAdp_MPIF_InitSPIF(U8 u8slaveid);
void MAdp_MPIF_SetCmdDataWidth(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth);
BOOL MAdp_MPIF_1A(U8 u8bWite, U8 slaveid, U16 addr, U8 *pudata);
BOOL MAdp_MPIF_2A(U8 u8bWite, U8 slaveid, U16 addr, U16 *pudata);
U8 MAdp_MPIF_ReadByte(U8 u8slaveid, U16 u16Addr);
U16 MAdp_MPIF_Read2Byte(U8 u8slaveid, U16 u16Addr);
U32 MAdp_MPIF_Read3Byte(U8 u8slaveid, U16 u16Addr);
BOOL MAdp_MPIF_WriteByte(U8 u8slaveid, U16 u16Addr, U8 u8Val);
BOOL MAdp_MPIF_Write2Byte(U8 u8slaveid, U16 u16Addr, U16 u16Val);
BOOL MAdp_MPIF_Write3Byte(U8 u8slaveid, U16 u16Addr, U32 u32Val);

BOOL MAdp_MPIF_Test(U8 u8slaveid, U8 u8TestLevel);

#endif /* _MADP_MFC_H_ */
