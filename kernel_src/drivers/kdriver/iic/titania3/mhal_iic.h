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

#ifndef _HAL_IIC_H_
#define _HAL_IIC_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
extern void MHal_IIC_Init(void);
extern void MHal_IIC_Clock_Select(U8 u8ClockIIC);
extern void MHal_IIC_Start(void);
extern void MHal_IIC_Stop(void);
extern void MHal_IIC_NoAck(void);
extern B16 MHal_IIC_SendData(U8 u8DataIIC);
extern B16 MHal_IIC_SendByte(U8 u8DataIIC);
extern B16 MHal_IIC_GetByte(U8* pu8DataIIC);

#endif // _HAL_IIC_H_

