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
#ifndef _MDRV_M4VE_IO_H_
#define _MDRV_M4VE_IO_H_

#include "M4VE_chip.h"
#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
#include "mdrv_types.h"
#elif defined(_M4VE_BIG2_)&&defined(_MIPS_PLATFORM_)
#include <sys/bsdtypes.h>
#endif
/* Use 'h' as magic number */
#define M4VE_IOC_MAGIC           'E'
#if defined(_M4VE_BIG2_)
#define M4VE_IOC_INIT            (M4VE_IOC_MAGIC<<8)|0
#define M4VE_IOC_PLAY            (M4VE_IOC_MAGIC<<8)|1
#define M4VE_IOC_ENCODE_BITSTREAM (M4VE_IOC_MAGIC<<8)|2
#define M4VE_IOC_GETBITS         (M4VE_IOC_MAGIC<<8)|3
#define M4VE_IOC_ENC_ONEFRAME    (M4VE_IOC_MAGIC<<8)|4
#define M4VE_IOC_FINISH          (M4VE_IOC_MAGIC<<8)|5
#define M4VE_IOC_CLEAR_BITSBUF   (M4VE_IOC_MAGIC<<8)|6
#define M4VE_IOC_ENC_CONTINUE    (M4VE_IOC_MAGIC<<8)|7
#define M4VE_IOC_SET_BITRATE    (M4VE_IOC_MAGIC<<8)|8
#define M4VE_IOC_SET_QP         (M4VE_IOC_MAGIC<<8)|9
#define M4VE_IOC_SET_CODEC      (M4VE_IOC_MAGIC<<8)|10
#define M4VE_IOC_GET_VOL        (M4VE_IOC_MAGIC<<8)|11
#define M4VE_IOC_ENC_SKIPVOP    (M4VE_IOC_MAGIC<<8)|12
#define M4VE_IOC_SET_FRAMERATE  (M4VE_IOC_MAGIC<<8)|13
#else
#define M4VE_IOC_INIT            _IO(M4VE_IOC_MAGIC, 0)
#define M4VE_IOC_PLAY            _IOR(M4VE_IOC_MAGIC, 1, U32)
#define M4VE_IOC_ENCODE_BITSTREAM _IO(M4VE_IOC_MAGIC, 2)
#define M4VE_IOC_GETBITS         _IOR(M4VE_IOC_MAGIC, 3, U32)
#define M4VE_IOC_ENC_ONEFRAME    _IOR(M4VE_IOC_MAGIC, 4, U32)
#define M4VE_IOC_FINISH          _IOR(M4VE_IOC_MAGIC, 5, U32)
#define M4VE_IOC_CLEAR_BITSBUF   _IOR(M4VE_IOC_MAGIC, 6, U32)
#define M4VE_IOC_ENC_CONTINUE    _IO(M4VE_IOC_MAGIC, 7)
#define M4VE_IOC_SET_BITRATE    _IOR(M4VE_IOC_MAGIC, 8, U32)
#define M4VE_IOC_SET_QP         _IOR(M4VE_IOC_MAGIC, 9, U32)
#define M4VE_IOC_SET_CODEC      _IOR(M4VE_IOC_MAGIC, 10, U32)
#define M4VE_IOC_GET_VOL        _IOR(M4VE_IOC_MAGIC, 11, U32)
#define M4VE_IOC_ENC_SKIPVOP    _IOR(M4VE_IOC_MAGIC, 12, U32)
#define M4VE_IOC_SET_FRAMERATE  _IOR(M4VE_IOC_MAGIC, 13, U32)
#define M4VE_IOC_POWEROFF       _IOR(M4VE_IOC_MAGIC, 14, U32)
#endif
#define M4VE_IOC_MAXNR           15

//#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
//S32 MDrv_M4VE_Ioctl(struct inode *inode, struct file *filp,
//                 unsigned int cmd, unsigned long arg);
//#else
#ifdef _M4VE_BIG2_
S32 MDrv_M4VE_Ioctl(int m4ve_fd, unsigned int cmd, unsigned long arg);
#endif
//#endif

#endif

