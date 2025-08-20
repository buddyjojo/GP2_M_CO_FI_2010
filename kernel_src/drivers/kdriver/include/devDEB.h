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

/* Use 'b' as magic number */
#define DEB_IOC_MAGIC           'b'

#define DEB_IOC_INIT            _IO(DEB_IOC_MAGIC, 0)
#define DEB_IOC_RESET           _IO(DEB_IOC_MAGIC, 1)
#define DEB_IOC_ENABLE          _IOW(DEB_IOC_MAGIC, 2, DEB_SetEnableInfo)
#define DEB_IOC_SET_PIC_SIZE    _IOW(DEB_IOC_MAGIC, 3, DEB_PicSize)
#define DEB_IOC_SET_BUF_PITCH   _IO(DEB_IOC_MAGIC, 4)
#define DEB_IOC_SET_BUF_ADDR    _IO(DEB_IOC_MAGIC, 5)
#define DEB_IOC_SET_LUMA_ADDR   _IO(DEB_IOC_MAGIC, 6)
#define DEB_IOC_SET_CHROMA_ADDR _IO(DEB_IOC_MAGIC, 7)
#define DEB_IOC_ENA_SW_FIELD    _IO(DEB_IOC_MAGIC, 8)
#define DEB_IOC_REF_FROM_MVD    _IO(DEB_IOC_MAGIC, 9)
#define DEB_IOC_SET_SRC_FMT     _IO(DEB_IOC_MAGIC, 10)
#define DEB_IOC_SET_INTL_IN     _IO(DEB_IOC_MAGIC, 11)
#define DEB_IOC_SET_INTL_OUT    _IO(DEB_IOC_MAGIC, 12)
#define DEB_IOC_SET_X_OFFSET    _IO(DEB_IOC_MAGIC, 13)
#define DEB_IOC_SET_Y_OFFSET    _IO(DEB_IOC_MAGIC, 14)
#define DEB_IOC_SET_SYNC_NOTIFY _IOW(DEB_IOC_MAGIC, 15, DEB_SyncNotify)
#define DEB_IOC                 _IO(DEB_IOC_MAGIC, 16)
#define DEB_IOCREAD             _IOR(DEB_IOC_MAGIC, 17, int)
#define DEB_IOCWRITE            _IOW(DEB_IOC_MAGIC, 18, int)
#define DEB_IOCRW               _IOWR(DEB_IOC_MAGIC, 19, int)

#define DEB_IOC_MAXNR           20



