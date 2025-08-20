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
/// @file   mdrv_nand_io.h
/// @brief  NAND Driver Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_NAND_IO_H_
#define _MDRV_NAND_IO_H_

#define NAND_IOC_MAGIC				'n'

#define MDRV_NAND_INIT				_IO(NAND_IOC_MAGIC, 0)
#define MDRV_NAND_POWER_ON_OFF		_IOWR(NAND_IOC_MAGIC, 1, U8)
#define MDRV_NAND_MTD_ADD			_IO(NAND_IOC_MAGIC, 2) //dhjung LGE

#define NAND_IOC_MAXNR				3 //dhjung LGE

#endif
