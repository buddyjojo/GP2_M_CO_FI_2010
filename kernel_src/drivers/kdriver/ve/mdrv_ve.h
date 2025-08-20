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
#include <linux/cdev.h>

#ifndef _MDRV_VE_H
#define _MDRV_VE_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ve.h
/// This file contains the Mstar driver interface for TVEncoder
/// @author MStar Semiconductor Inc.
/// @brief  TVEncoder module
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef __KERNEL__
#include "mdrv_types.h"
#endif

#ifndef MDRV_VE_NR_DEVS
#define MDRV_VE_NR_DEVS 1
#endif

// These type define can be removed after the coding style is confirmed.
#include <asm-mips/types.h>



//------------------------------------------------------------------------------
// macro
//------------------------------------------------------------------------------

#define OPT_VE_DRV_DEBUG
#undef VE_DRV_DBG
#ifdef OPT_VE_DRV_DEBUG
    #define VE_DRV_DBG(fmt, args...)      printk(KERN_WARNING "[VE_DRV][%05d]" fmt, __LINE__, ## args)
#else
    #define VE_DRV_DBG(fmt, args...)
#endif

#undef VE_DRV_DBGX
#define VE_DRV_DBGX(fmt, args...)


//------------------------------------------------------------------------------
// enum
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// structure
//------------------------------------------------------------------------------

#ifdef __KERNEL__

int MDrv_VE_Open(struct inode *inode, struct file *filp);
int MDrv_VE_Release(struct inode *inode, struct file *filp);
int MDrv_VE_IOCtl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

#endif


#endif  // #ifndef _MDRV_VE_H_

