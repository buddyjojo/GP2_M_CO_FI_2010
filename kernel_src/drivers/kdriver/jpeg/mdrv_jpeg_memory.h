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

#ifndef _MSAPI_JPEG_MEMORY_H_
#define _MSAPI_JPEG_MEMORY_H_

#include <linux/kernel.h>
#include "mdrv_types.h"

#ifdef _MSAPI_JPEG_MEMORY_C_
#define JPEG_MEM_INTERFACE
#else
#define JPEG_MEM_INTERFACE extern
#endif


JPEG_MEM_INTERFACE BOOL MDrv_JPEG_init_mempool (void *pool, U32 size);
JPEG_MEM_INTERFACE void *MDrv_JPEG_malloc (U32 size);
JPEG_MEM_INTERFACE void MDrv_JPEG_free (void *memp);

#endif

