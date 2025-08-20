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

#ifndef _MDRV_LOGO_H_
#define _MDRV_LOGO_H_
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ve.h
/// This file contains the Mstar driver interface for TVEncoder
/// @author MStar Semiconductor Inc.
/// @brief  TVEncoder module
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef __KERNEL__
#include "mdrv_types.h"
#endif


// These type define can be removed after the coding style is confirmed.
#include <asm-mips/types.h>



//------------------------------------------------------------------------------
// macro
//------------------------------------------------------------------------------


#undef LOGO_DRV_DBGX
#define LOGO_DRV_DBGX(fmt, args...)


//------------------------------------------------------------------------------
// enum
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// structure
//------------------------------------------------------------------------------


#ifdef __KERNEL__

int MDrv_LOGO_Open(struct inode *inode, struct file *filp);
int MDrv_LOGO_Release(struct inode *inode, struct file *filp);
int MDrv_LOGO_IOCtl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

#endif


#endif  // #ifndef _MDRV_LOGO_H_

