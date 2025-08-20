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
/// file    Mdrv_mvd.h
/// @brief  MVD Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _MDRV_MVD_H_
#define _MDRV_MVD_H_

#include "mdrv_types.h"
#include <linux/module.h>

#include <linux/fs.h>    // for MKDEV()

#include <linux/cdev.h>
#include "mdrv_mvd_io.h"
#include "mdrv_mvd_st.h"

//#define MVD_Debug_PRINT // LGE drmyung 080922
#ifdef MVD_Debug_PRINT
#define MVD_Debug printk
#else
#define MVD_Debug(fmt, args...)
#endif

//#define MVD_PAGE_NUM            16      // 16+16 => 16(64K) for callback; 16(64K) for file playback
#define QUEUE_SIZE 100

struct MVD_Dev {
	struct cdev cdev;	  /* Char device structure		*/
};


#define MVDSTATUS                           U32
/// @name MVDSTATUS
/// MVDSTATUS return value
/// @{
#define MVDSTATUS_SUCCESS                   0x00000000  //0
#define MVDSTATUS_FAIL                      0x00000001  //Bit(0)
#define MVDSTATUS_INVALID_STREAM_ID         0x00000002  //Bit(1)
#define MVDSTATUS_INVALID_CALLBACK          0x00000020  //Bit(5)
#define MVDSTATUS_OUTOF_MEMORY              0x00000040  //Bit(6)
#define MVDSTATUS_INVALID_COMMAND           0x00000080  //Bit(7)
#define MVDSTATUS_NO_AVAIL_STREAM           0x00000100  //Bit(8)
#define MVDSTATUS_INVALID_PARAMETERS        0x00000800  //Bit(11)

S32 MDrv_MVD_Ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg);
S32 MDrv_MVD_Open(struct inode *inode, struct file *filp);
S32 MDrv_MVD_Release(struct inode *inode, struct file *filp);

#if 0 // Tonio Liu 2009.09.03
S32 MDrv_MVD__mmap(struct file *filp, struct vm_area_struct *vma);
#endif

extern void MDrv_MVD_Wakeup(void);



#endif
