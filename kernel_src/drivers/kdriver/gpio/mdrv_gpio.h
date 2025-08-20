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
/// @file   mdrv_gpio.h
/// @brief  GPIO Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_GPIO_H_
#define _DRV_GPIO_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mdrv_gpio_io.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//	added for BOOTSTRAP(DISPLAY & MODULE TYPE) by dreamer@lge.com
/*
	_INCLUDE_BOOTSTRAP must be defined for LGE
*/
#define	_INCLUDE_BOOTSTRAP
#define _INCLUDE_S7BOOTSTRAP //balup_090907
#define _INCLUDE_NEWBOOTSTRAP //balup_090922

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
extern U8 MDrv_MCU8051Lite_RegRead( U16 u16Addr );

void MDrv_GPIO_Init(eBOOT_TYPE type);
void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO);
void MDrv_GPIO_Pad_Oen(U8 u8IndexGPIO);
void MDrv_GPIO_Pad_Odn(U8 u8IndexGPIO);
U8 MDrv_GPIO_Pad_Read(U8 u8IndexGPIO);
void MDrv_GPIO_Pull_High(U8 u8IndexGPIO);
void MDrv_GPIO_Pull_Low(U8 u8IndexGPIO);
void MDrv_GPIO_Set_High(U8 u8IndexGPIO);
void MDrv_GPIO_Set_Low(U8 u8IndexGPIO);

#endif // _DRV_GPIO_H_

