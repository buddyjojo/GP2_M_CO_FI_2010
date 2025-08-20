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

#ifndef _DRV_JPEG_IO_H_
#define _DRV_JPEG_IO_H_


/* Use 'J' as magic number */
#define JPEG_IOCTL_MAGIC           'J'

#define IOCTL_JPEG_INIT         _IOW(JPEG_IOCTL_MAGIC, 0, int)
#define IOCTL_JPEG_PLAY         _IOR(JPEG_IOCTL_MAGIC, 1, int)
#define IOCTL_JPEG_THUMBNAIL    _IOR(JPEG_IOCTL_MAGIC, 2, int)
#define IOCTL_JPEG_FEEDDATA_L   _IOR(JPEG_IOCTL_MAGIC, 3, int)
#define IOCTL_JPEG_FEEDDATA_H   _IOR(JPEG_IOCTL_MAGIC, 4, int)
#define IOCTL_JPEG_SETBUFFER    _IOW(JPEG_IOCTL_MAGIC, 5, int)
#define IOCTL_JPEG_WAKEUP       _IOW(JPEG_IOCTL_MAGIC, 6, int)
#define IOCTL_JPEG_SET_MAX_SIZE _IOW(JPEG_IOCTL_MAGIC, 7, int)
#define IOCTL_JPEG_SET_PWR_ON   _IOW(JPEG_IOCTL_MAGIC, 8, int)
#define IOCTL_JPEG_SET_PWR_OFF  _IOW(JPEG_IOCTL_MAGIC, 9, int)

#if 1 //MIU is configured in system layer
#define JPEG_IOCTL_MAXNR        10
#else
#define IOCTL_JPEG_SET_MIU      _IOW(JPEG_IOCTL_MAGIC, 8, int)
#define JPEG_IOCTL_MAXNR        9
#endif

#endif
