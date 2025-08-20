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

#ifndef __DRV_SPI_IO_H__
#define __DRV_SPI_IO_H__

//-------------------------------------------------------------------------------------------------
//  ioctl method
//-------------------------------------------------------------------------------------------------

// Use 'S' as magic number
#define SPI_IOCTL_MAGIC             'S'

#define IOCTL_SPI_INIT             _IOWR(SPI_IOCTL_MAGIC, 0x10, int) 
#define IOCTL_SPI_READ             _IOWR(SPI_IOCTL_MAGIC, 0x11, int)
#define IOCTL_SPI_WRITE            _IOWR(SPI_IOCTL_MAGIC, 0x12, int)
#define IOCTL_SPI_ERASE_SEC        _IOWR(SPI_IOCTL_MAGIC, 0x13, int)

#define IOCTL_SPI_MAXNR    0xFF

#endif // __DRV_SPI_IO_H__

