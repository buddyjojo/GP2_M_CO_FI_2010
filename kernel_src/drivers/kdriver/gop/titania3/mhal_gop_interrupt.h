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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mhal_gop_interrupt.c
/// @brief  MVD HAL layer Driver for handling interrupt
/// @author MStar Semiconductor Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MHAL_GOP_INTERRUPT_MSTAR_H
#define MHAL_GOP_INTERRUPT_MSTAR_H 1

#include "chip_int.h"

// Set up for C function definitions, even when using C++
#if defined(__cplusplus)
extern "C" {
#endif

#define U32 unsigned int
#define U8  unsigned char

irqreturn_t gop_interrupt(int irq,void *devid);

#if defined(__cplusplus)
}
#endif

#endif /* HTTP_HELPER_ULEAD_H */

