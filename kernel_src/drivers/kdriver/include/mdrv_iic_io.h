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
/// @file   mdrv_iic.h
/// @brief  IIC Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

struct IIC_Param
{
    U8 u8IdIIC;      	/// IIC ID: Channel 1~7
    U8 u8ClockIIC;   	/// IIC clock speed
    U8 u8SlaveIdIIC;    /// Device slave ID
    U8 u8AddrSizeIIC;	/// Address length in bytes
    U8 u8AddrIIC[4];	/// Starting address inside the device

	//dhjung LGE
	U8 *u8pbufIIC;     	/// buffer
	U32 u32DataSizeIIC;	/// size of buffer

} __attribute__ ((packed));

//dhjung LGE
typedef struct IIC_Param IIC_Param_t;

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define IIC_IOC_MAGIC               'u'

#define MDRV_IIC_INIT               _IO(IIC_IOC_MAGIC, 0)
#define MDRV_IIC_SET_PARAM          _IOW(IIC_IOC_MAGIC, 1, IIC_Param_t)
#define MDRV_IIC_CLOCK              _IOW(IIC_IOC_MAGIC, 2, IIC_Param_t)
#define MDRV_IIC_ENABLE             _IOW(IIC_IOC_MAGIC, 3, IIC_Param_t)	// added for RGB EDID by LGE(dreamer@lge.com)
#define IIC_IOC_MAXNR               4

#define IIC_RW_BUF_SIZE             1024
#define IIC_WR_BUF_SIZE             128		// added for RGB EDID by LGE(dreamer@lge.com)
#define IIC_RD_BUF_SIZE             256		// added for RGB EDID by LGE(dreamer@lge.com)

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

