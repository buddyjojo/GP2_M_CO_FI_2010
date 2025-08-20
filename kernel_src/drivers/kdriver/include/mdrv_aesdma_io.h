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
/// @file   mdrv_aesdma_io.h
/// @brief  aesdma Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access aesdma.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_AESDMA_IO_H_
#define _MDRV_AESDMA_IO_H_
//#include <linux/cdev.h>
//#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define AESDMA_IOC_MAGIC                'A'

#define MDRV_AESDMA_INIT                _IOWR(AESDMA_IOC_MAGIC, 0, AESDMA_INIT_t)
#define MDRV_AESDMA_START               _IOWR(AESDMA_IOC_MAGIC, 1, AESDMA_START_t)
#define MDRV_AESDMA_SEL_ENG             _IOWR(AESDMA_IOC_MAGIC, 2, AESDMA_SELENG_t)
#define MDRV_AESDMA_SET_PS              _IOWR(AESDMA_IOC_MAGIC, 3, AESDMA_SETPS_t)
#define MDRV_AESDMA_SET_FILE_INOUT      _IOWR(AESDMA_IOC_MAGIC, 4, AESDMA_SETFILEINOUT_t)
#define MDRV_AESDMA_NOTIFY              _IOWR(AESDMA_IOC_MAGIC, 5, AESDMA_NOTIFY_t)
#define MDRV_AESDMA_RANDOM              _IOR(AESDMA_IOC_MAGIC,  6, AESDMA_RANDOM_t)
#define MDRV_AESDMA_STATUS              _IOR(AESDMA_IOC_MAGIC,  7, AESDMA_STATUS_t)
#define MDRV_AESDMA_RESET               _IO(AESDMA_IOC_MAGIC,   8)
#define AESDMA_IOC_MAXNR                9



//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#endif // _MDRV_AESDMA_IO_H_
