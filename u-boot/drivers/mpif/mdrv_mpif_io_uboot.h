////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_MPIF_IO_H_
#define _MDRV_MPIF_IO_H_

//------------------------------------------------------------------------------
// Driver Name
//------------------------------------------------------------------------------
#define MPIF_MODULE_KERNAL_NAME       "/dev/mpif"


//------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------

/* Use 'f' as magic number */
#define MPIF_IOCTL_MAGIC            'f'
#define IOCTL_MPIF_INIT                    0
#define IOCTL_MPIF_INIT_SPIF               1
#define IOCTL_MPIF_1A                      2
#define IOCTL_MPIF_2A                      3
#define IOCTL_MPIF_SET_CMDDATA_WIDTH       4

#define IOCTL_MPIF_MAXNR            256

//#define open
#define O_RDWR                  0
#define EFAULT                  14
#define ENOTTY                  25
#define printk                  printf

int ioctl(int inode, unsigned int cmd, void *parg);

#endif
