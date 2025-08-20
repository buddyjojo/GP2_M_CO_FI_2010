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

/* driver header files */
#include "mdrv_msmailbox_io.h"
#include "mdrv_msmailbox_st.h"
#include "mdrv_msmailbox_settings.h"

#ifndef __MDRV_MSMAILBOX_H__
#define __MDRV_MSMAILBOX_H__

typedef struct
{
    _ERR_CODE_                  (*Fill)(msmailbox_t*);
    _ERR_CODE_                  (*Instant)(msmailbox_t*);
    _ERR_CODE_                  (*Init)(U32);
    _ERR_CODE_                  (*Release)(void);
    _ERR_CODE_                  (*PutAddr)(msmailbox_t**);
} msmailbox_queue_ops_t;

typedef struct
{
    BOOL                        bValid;
    volatile U8 *               pBuf;
    U32                         u32PageNum;
    volatile msmailbox_info_t * pInfo;
    msmailbox_queue_ops_t       ops;
} msmailbox_queue_mgr_t;

#endif


