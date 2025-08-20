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

#ifndef __DRV_SPI_ST_H__
#define __DRV_SPI_ST_H__

//------------------------------------------------------------------------------
// Data structure
//------------------------------------------------------------------------------

typedef struct
{
    U32  u32Addr;
    U8*  pu8Buf;
    U32  u32Len;
} IO_SPI_RW_t;

#endif // __DRV_SPI_ST_H__

