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

#ifndef _RTC_REG_H_
#define _RTC_REG_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define CONTROL_WORD               0xB71B00 //12MHz

#define REG_MIPS_BASE               0xBF000000
#define REG_RTC_BASE                0x900
#define REG_RTC_BASE2               0x980

#define RTC_CTRL_NOT_RSTZ       (1 << 0)
#define RTC_CTRL_CNT_EN         (1 << 1)
#define RTC_CTRL_WRAP_EN        (1 << 2)
#define RTC_CTRL_LOAD_EN        (1 << 3)
#define RTC_CTRL_READ_EN        (1 << 4)
#define RTC_CTRL_INT_MASK       (1 << 5)
#define RTC_CTRL_INT_FORCE      (1 << 6)
#define RTC_CTRL_INT_CLEAR      (1 << 7)

#define RTC_INT_RAW_STATUS      (1 << 0)
#define RTC_INT_STATUS          (1 << 1)

#define REG_RTC_CTRL                (REG_RTC_BASE + 0x00)
#define REG_RTC_FREQ_CW1           (REG_RTC_BASE + 0x01)
#define REG_RTC_FREQ_CW2           (REG_RTC_BASE + 0x02)
#define REG_RTC_LOAD_VAL1          (REG_RTC_BASE + 0x03)
#define REG_RTC_LOAD_VAL2          (REG_RTC_BASE + 0x04)
#define REG_RTC_MATCH_VAL1         (REG_RTC_BASE + 0x05)
#define REG_RTC_MATCH_VAL2         (REG_RTC_BASE + 0x06)
#define REG_RTC_INTERRUPT          (REG_RTC_BASE + 0x07)
#define REG_RTC_CNT1                (REG_RTC_BASE + 0x08)
#define REG_RTC_CNT2                (REG_RTC_BASE + 0x09)

#define MHal_RTC_REG(addr)             (*(volatile U16*)(REG_MIPS_BASE + (addr<<2)))

#endif // _RTC_REG_H_
