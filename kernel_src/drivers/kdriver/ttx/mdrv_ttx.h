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
#include "mhal_ttx_types.h"

#ifndef _MDRV_TTX_H
#define _MDRV_TTX_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_ttx.h
/// This file contains the Mstar driver I/O control and data type for Teletext
/// @author MStar Semiconductor Inc.
/// @brief  Teletext module
///////////////////////////////////////////////////////////////////////////////

#define OPT_TTX_DRV_DEBUG
#undef TTX_DRV_DBG
#ifdef OPT_TTX_DRV_DEBUG
    #define TTX_DRV_DBG(fmt, args...)      printk(KERN_WARNING "[TTX_DRV][%05d]" fmt, __LINE__, ## args)
#else
    #define TTX_DRV_DBG(fmt, args...)
#endif

#undef TTX_DRV_DBGX
#define TTX_DRV_DBGX(fmt, args...)


/* 1, 2, 4, 8 ... */
#define TTX_PAGE_NUM            16

extern volatile struct VBI_PARAMETERS   *_tpVBIParametersTTX;
extern U8 *_pu8VPS_TTX;
extern U8 *_pu8WSS_TTX;


extern void MDrv_Wakeup(void);


#endif

