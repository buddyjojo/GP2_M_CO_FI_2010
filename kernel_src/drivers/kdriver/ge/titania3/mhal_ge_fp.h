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


#ifndef DRV_FIXEDPOINT_H
#define DRV_FIXEDPOINT_H

#include "mhal_ge_reg.h"


extern U32 Divide2Fixed(U16 u16x, U16 u16y, U8 nInterger, U8 nFraction);

#if 0
extern void FixPLine_S1_11(MS_U16 u16x, MS_U16 u16y, LONG16_BYTE* pu16ret);
extern void FixPClr_S8_11(MS_S16 s8color, MS_U16 u16dis, LONG32_BYTE* pu32ret);
extern void FixPBlt_S1_15(MS_U16 u16src, MS_U16 u16dst, LONG16_BYTE* pu16ret);
#endif

#endif
