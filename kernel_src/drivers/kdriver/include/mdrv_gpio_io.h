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
/// @file   mdrv_gpio_io.h
/// @brief  GPIO Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "../str/mdrv_str.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define GPIO_IOC_MAGIC               'g'

#define MDRV_GPIO_INIT               _IO(GPIO_IOC_MAGIC, 0)
#define MDRV_GPIO_SET                _IOW(GPIO_IOC_MAGIC, 1, U8)
#define MDRV_GPIO_OEN                _IOW(GPIO_IOC_MAGIC, 2, U8)
#define MDRV_GPIO_ODN                _IOW(GPIO_IOC_MAGIC, 3, U8)
#define MDRV_GPIO_READ               _IOWR(GPIO_IOC_MAGIC, 4, U8)
#define MDRV_GPIO_PULL_HIGH         _IOW(GPIO_IOC_MAGIC, 5, U8)
#define MDRV_GPIO_PULL_LOW          _IOW(GPIO_IOC_MAGIC, 6, U8)

#define GPIO_IOC_MAXNR               7

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
extern struct semaphore PfModeSem;

void __mod_gpio_init(eBOOT_TYPE type);

