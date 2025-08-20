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
/// @file   mdrv_h264.h
/// @brief  H.264 Control Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _DRV_H264_H_
#define _DRV_H264_H_

#include "mdrv_types.h"
#include <linux/module.h>

#include <linux/fs.h>    // for MKDEV()

#include <linux/cdev.h>
#include "mdrv_h264_io.h"
#include "mdrv_h264_st.h"


#define MVD_Debug printk

//#define MVD_PAGE_NUM            16      // 16+16 => 16(64K) for callback; 16(64K) for file playback

struct MVD_Dev {
	struct cdev cdev;	  /* Char device structure		*/
};


#define H264STATUS                           U32
/// @name MVDSTATUS
/// MVDSTATUS return value
/// @{
#define H264STATUS_SUCCESS                   0x00000000  //0
#define H264STATUS_FAIL                      0x00000001  //Bit(0)
#define H264STATUS_INVALID_STREAM_ID         0x00000002  //Bit(1)
#define H264STATUS_INVALID_CALLBACK          0x00000020  //Bit(5)
#define H264STATUS_OUTOF_MEMORY              0x00000040  //Bit(6)
#define H264STATUS_INVALID_COMMAND           0x00000080  //Bit(7)
#define H264STATUS_NO_AVAIL_STREAM           0x00000100  //Bit(8)
#define H264STATUS_INVALID_PARAMETERS        0x00000800  //Bit(11)

S32 MDrv_H264_Ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg);
S32 MDrv_H264_Open(struct inode *inode, struct file *filp);
S32 MDrv_H264_Release(struct inode *inode, struct file *filp);
S32 MDrv_H264__mmap(struct file *filp, struct vm_area_struct *vma);

void MDrv_H264_Wakeup(void);



#endif
