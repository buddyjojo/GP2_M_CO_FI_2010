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
/// @file   mdrv_msmailbox_io.h
/// @brief  Mialbox Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access mailbox.
///     - Provide functions to initialize mailbox
///     - Provide mailbox ISR.
///     - Provide mailbox wail function for AP.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MDRV_MSMAILBOX_IO_H__
#define __MDRV_MSMAILBOX_IO_H__

#define NO_INIT                                 0x00
#define NO_RELEASE                              0x01
#define NO_FIREMAIL                             0x02
#define NO_WAIT_SYNCMAIL                        0x03
#define NO_INTERRUPT                            0x04
#define NO_INFO                                 0x05
#define NO_ENABLE                               0x06
#define NO_DISABLE                              0x07
#define NO_FIREMAIL_LOOPBACK                    0x08

#define IOCTL_MSMAILBOX_END_NUM                 0x0A


// use 'm' as magic number
#define MSMAILBOX_IOCTL_MAGIC                   'm'
#define IOCTL_MSMAILBOX_INIT                    _IOW(MSMAILBOX_IOCTL_MAGIC, NO_INIT,            U32)
#define IOCTL_MSMAILBOX_RELEASE                 _IOW(MSMAILBOX_IOCTL_MAGIC, NO_RELEASE,         U32)
#define IOCTL_MSMAILBOX_FIREMAIL                _IOW(MSMAILBOX_IOCTL_MAGIC, NO_FIREMAIL,        U32)
#define IOCTL_MSMAILBOX_WAIT_SYNCMAIL           _IOR(MSMAILBOX_IOCTL_MAGIC, NO_WAIT_SYNCMAIL,   U32)
#define IOCTL_MSMAILBOX_INTERRUPT               _IOW(MSMAILBOX_IOCTL_MAGIC, NO_INTERRUPT,       U32)
#define IOCTL_MSMAILBOX_INFO                    _IOR(MSMAILBOX_IOCTL_MAGIC, NO_INFO,            U32)
#define IOCTL_MSMAILBOX_ENABLE                  _IOW(MSMAILBOX_IOCTL_MAGIC, NO_ENABLE,          U32)
#define IOCTL_MSMAILBOX_DISABLE                 _IOW(MSMAILBOX_IOCTL_MAGIC, NO_DISABLE,         U32)
#define IOCTL_MSMAILBOX_FIREMAIL_LOOPBACK       _IOW(MSMAILBOX_IOCTL_MAGIC, NO_FIREMAIL_LOOPBACK, U32)

#endif


