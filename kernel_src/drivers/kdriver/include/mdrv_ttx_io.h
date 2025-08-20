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
#ifndef _MDRV_TTX_IO_H
#define _MDRV_TTX_IO_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_ttx_io.h
/// This file contains the Mstar driver I/O control interface for Teletext
/// @author MStar Semiconductor Inc.
/// @brief  Teletext module
///////////////////////////////////////////////////////////////////////////////

// use 't' as magic number
#define TTX_IOCTL_MAGIC                          't'
#define IOCTL_TTX_INIT                          _IOW(TTX_IOCTL_MAGIC, 0xA0, U32)
#define IOCTL_TTX_ONOFF_SLICER                  _IOW(TTX_IOCTL_MAGIC, 0xA1, U32)
#define IOCTL_TTX_SET_VIDEOSTANDARD             _IOW(TTX_IOCTL_MAGIC, 0xA2, U32)
#define IOCTL_TTX_INTERRUPT_ED                  _IOW(TTX_IOCTL_MAGIC, 0xA3, U32)
#define IOCTL_TTX_RELEASE                       _IOW(TTX_IOCTL_MAGIC, 0xA4, U32)
#define IOCTL_TTX_GET_PACKET_UNIT               _IOR(TTX_IOCTL_MAGIC, 0xA5, U32)
#define IOCTL_TTX_GET_TTX_PARAMETER_OFFSET      _IOR(TTX_IOCTL_MAGIC, 0xA6, U32)
#define IOCTL_TTX_GET_VPS_OFFSET                _IOR(TTX_IOCTL_MAGIC, 0xA7, U32)
#define IOCTL_TTX_GET_WSS_OFFSET                _IOR(TTX_IOCTL_MAGIC, 0xA8, U32)
#define IOCTL_TTX_WAIT_EVENT                    _IOR(TTX_IOCTL_MAGIC, 0xA9, U32)
#define IOCTL_TTX_RESET_VBI_PARAMETERS          _IOR(TTX_IOCTL_MAGIC, 0xAA, U32)
#define IOCTL_TTX_GET_TTX_DATARATE              _IOR(TTX_IOCTL_MAGIC, 0xAB, U32)
#define IOCTL_TTX_BUFFER_RESET                  _IO(TTX_IOCTL_MAGIC, 0xAC)
#define IOCTL_TTX_RESET                         _IO(TTX_IOCTL_MAGIC, 0xAD)
#define IOCTL_TTX_RESET_VPS                     _IO(TTX_IOCTL_MAGIC, 0xAE)
#define IOCTL_TTX_RESET_WSS                     _IO(TTX_IOCTL_MAGIC, 0xAF)

#define IOCTL_TTX_UART_OFF                      _IOW(TTX_IOCTL_MAGIC, 0xF0, U32)

#endif

