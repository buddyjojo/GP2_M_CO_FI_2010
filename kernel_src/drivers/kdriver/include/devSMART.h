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
/// @file   devSMART.h
/// @author MStar Semiconductor Inc.
/// @brief  Smartcard Device Driver
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DEV_SMART_H_
#define _DEV_SMART_H_

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

#ifndef MDEV_MAJOR_SMART
#define MDEV_MAJOR_SMART            0                                   // device major number
#endif

#define DEVSMART_DEV_NUM            1                                   // number of device

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
/* Use 's' as magic number */
#define DEVSMART_IOC_MAGIC          's'

#define DEVSMART_IOC_SETCLK         _IOW(DEVSMART_IOC_MAGIC,   0,  int)
#define DEVSMART_IOC_SETDIV         _IOW(DEVSMART_IOC_MAGIC,   1,  int)
#define DEVSMART_IOC_SETMODE        _IOW(DEVSMART_IOC_MAGIC,   2,  int)
#define DEVSMART_IOC_XCHGINV        _IO(DEVSMART_IOC_MAGIC,    3)
#define DEVSMART_IOC_SETVCC         _IOW(DEVSMART_IOC_MAGIC,   4,  int)
#define DEVSMART_IOC_RESET          _IO(DEVSMART_IOC_MAGIC,    5)
#define DEVSMART_IOC_GETEVENT       _IOR(DEVSMART_IOC_MAGIC,   6,  int)
#define DEVSMART_IOC_GETINFO        _IOR(DEVSMART_IOC_MAGIC,   7,  int)

#define DEVSMART_IOC_MAXNR          8

#define DEVSMART_OK                 0
#define DEVSMART_FAIL               -EFAULT
#define DEVSMART_CARDOUT            -ENXIO
#define DEVSMART_EMPTY              -ENODATA
#define DEVSMART_OVERFLOW           -EOVERFLOW
#define DEVSMART_TIMEOUT            -ETIME
#define DEVSMART_INVERSE            -EILSEQ
#define DEVSMART_EVENT_DATA         1
#define DEVSMART_EVENT_IN           2
#define DEVSMART_EVENT_OUT          4

#define DEVSMART_CLK_3M             0
#define DEVSMART_CLK_4P5M           1
#define DEVSMART_CLK_6M             2

#define DEVSMART_CHAR_7             (0x02)
#define DEVSMART_CHAR_8             (0x03)
#define DEVSMART_STOP_1             (0x00)
#define DEVSMART_STOP_2             (0x04)
#define DEVSMART_PARITY_NO          (0x00)
#define DEVSMART_PARITY_ODD         (0x08)
#define DEVSMART_PARITY_EVEN        (0x08|0x10)


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

/// Smartcard Info
typedef struct
{
    MS_BOOL                            bCardIn;
} DEVSMART_Info;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------


#endif // _DEV_SMART_H_

