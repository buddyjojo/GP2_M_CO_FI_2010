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

#ifndef _MDRV_MFC_IO_H_
#define _MDRV_MFC_IO_H_

//------------------------------------------------------------------------------
// Driver Name
//------------------------------------------------------------------------------
#define MFC_MODULE_KERNAL_NAME       "/dev/mfc"


//------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------

/* Use 'f' as magic number */
#define MFC_IOCTL_MAGIC            'f'

#define IOCTL_MFC_INIT                      _IOW(MFC_IOCTL_MAGIC,   0, int)
#define IOCTL_MFC_ONOFF						_IOW(MFC_IOCTL_MAGIC,   1, int)
#define IOCTL_MFC_COMPENSATION				_IOW(MFC_IOCTL_MAGIC,   2, int)
#define IOCTL_MFC_TRUE_MOTION_DEMO			_IOW(MFC_IOCTL_MAGIC,   3, int)
#define IOCTL_MFC_VIDEO_BLOCK				_IOW(MFC_IOCTL_MAGIC,   4, int)
#define IOCTL_MFC_LVDSONOFF					_IOW(MFC_IOCTL_MAGIC,   5, int)
#define IOCTL_MFC_SET_BYPASS_WINDOW			_IOW(MFC_IOCTL_MAGIC,   6, int)
#define IOCTL_MFC_GET_SW_VERSION			_IOWR(MFC_IOCTL_MAGIC,  7, int)
#define IOCTL_MFC_GET_BIN_VERSION			_IOWR(MFC_IOCTL_MAGIC,  8, int)
#define IOCTL_MFC_SSC						_IOW(MFC_IOCTL_MAGIC,	9, int)
#define IOCTL_MFC_DEBUGBLOCK				_IOW(MFC_IOCTL_MAGIC,	10, int)
#define IOCTL_MFC_DEMO_BAR					_IOW(MFC_IOCTL_MAGIC,	11, int)
#define IOCTL_MFC_GETHVTOTAL				_IOWR(MFC_IOCTL_MAGIC,	12, int)
#define IOCTL_MFC_ISSTABLE					_IOWR(MFC_IOCTL_MAGIC,  13, int)
#define IOCTL_MFC_LVDSVESAJEIDA				_IOW(MFC_IOCTL_MAGIC,	14, int)
#define IOCTL_MFC_FRAMERATE					_IOW(MFC_IOCTL_MAGIC,   15, int)
#define IOCTL_MFC_SLOWFRAMELOCK				_IOW(MFC_IOCTL_MAGIC,	16, int)
#define IOCTL_MFC_FRAMELOCKMODE				_IOW(MFC_IOCTL_MAGIC,	17, int)
#define IOCTL_MFC_RESET                     _IOW(MFC_IOCTL_MAGIC,   18, int)
#define IOCTL_MFC_UPDATESW					_IOW(MFC_IOCTL_MAGIC,   19, int)
#define IOCTL_MFC_REVERSE					_IOW(MFC_IOCTL_MAGIC,	20, int)
#define IOCTL_MFC_LVDSBITNUM				_IOW(MFC_IOCTL_MAGIC,	21, int)
#define IOCTL_MFC_SET_ODC_Table				_IOW(MFC_IOCTL_MAGIC,	22, int)





// for software path from T3
#define IOCTL_MFC_THREAD_START              _IOW(MFC_IOCTL_MAGIC,	31, int)
#define IOCTL_MFC_THREAD_HALT               _IOW(MFC_IOCTL_MAGIC,	32, int)
// for test use
#define IOCTL_MFC_READ_REG                  _IOWR(MFC_IOCTL_MAGIC, 100, int)
#define IOCTL_MFC_WRITE_REG                 _IOWR(MFC_IOCTL_MAGIC, 101, int)//test use
// for MPIF test only
#define IOCTL_MPIF_INIT                     _IOW(MFC_IOCTL_MAGIC,   240, int)
#define IOCTL_MPIF_INIT_SPIF                _IOW(MFC_IOCTL_MAGIC,   241, int)
#define IOCTL_MPIF_1A                       _IOWR(MFC_IOCTL_MAGIC,  242, int)
#define IOCTL_MPIF_2A                       _IOWR(MFC_IOCTL_MAGIC,  243, int)
#define IOCTL_MPIF_TEST                     _IOW(MFC_IOCTL_MAGIC,   244, int)
#define IOCTL_MPIF_READBYTE                 _IOR(MFC_IOCTL_MAGIC,   245, int)
#define IOCTL_MPIF_READ2BYTE                _IOR(MFC_IOCTL_MAGIC,   246, int)
#define IOCTL_MPIF_READ3BYTE                _IOR(MFC_IOCTL_MAGIC,   247, int)
#define IOCTL_MPIF_WRITEBYTE                _IOWR(MFC_IOCTL_MAGIC,  248, int)
#define IOCTL_MPIF_WRITE2BYTE               _IOWR(MFC_IOCTL_MAGIC,  249, int)
#define IOCTL_MPIF_WRITE3BYTE               _IOWR(MFC_IOCTL_MAGIC,  250, int)

#define IOCTL_MFC_MAXNR            256

#endif
