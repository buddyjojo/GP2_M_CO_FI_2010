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
///
/// @file   mdrv_mpool_io.h
/// @brief  Memory Pool  Driver IO Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_MPOOL_IO_H_
#define _MDRV_MPOOL_IO_H_

/* Use 'M' as magic number */
#define MPOOL_IOC_MAGIC                'M'

#define MPOOL_IOC_INFO           _IOWR(MPOOL_IOC_MAGIC, 0x00, DrvMPool_Info_t)
#define MPOOL_IOC_FLUSHDCACHE    _IOR(MPOOL_IOC_MAGIC,0x01,DrvMPool_Info_t)
#define MPOOL_IOC_GET_BLOCK_OFFSET    _IOR(MPOOL_IOC_MAGIC,0x02,unsigned int)

#endif

