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
#define IOCTL_MPIF_INIT                     _IOW(MPIF_IOCTL_MAGIC,   0, int)
#define IOCTL_MPIF_INIT_SPIF                _IOW(MPIF_IOCTL_MAGIC,   1, int)
#define IOCTL_MPIF_1A                      _IOWR(MPIF_IOCTL_MAGIC,   2, int)
#define IOCTL_MPIF_2A                       _IOWR(MPIF_IOCTL_MAGIC,   4, int)
#define IOCTL_MPIF_2B                       _IOWR(MPIF_IOCTL_MAGIC,   5, int)
#define IOCTL_MPIF_3A_RIU                   _IOWR(MPIF_IOCTL_MAGIC,   6, int)
#define IOCTL_MPIF_3A_MIU                   _IOWR(MPIF_IOCTL_MAGIC,   7, int)
#define IOCTL_MPIF_3B_RIU                   _IOWR(MPIF_IOCTL_MAGIC,   8, int)
#define IOCTL_MPIF_3B_MIU                   _IOWR(MPIF_IOCTL_MAGIC,   9, int)
#define IOCTL_MPIF_4A                       _IOWR(MPIF_IOCTL_MAGIC,  10, int)
#define IOCTL_MPIF_SET_CMDDATA_WIDTH        _IOWR(MPIF_IOCTL_MAGIC,  11, int)
#define IOCTL_MPIF_SET_SPIF_CLKINVDELAY		_IOWR(MPIF_IOCTL_MAGIC,  12, int)


#define IOCTL_MPIF_MAXNR            256

#endif
