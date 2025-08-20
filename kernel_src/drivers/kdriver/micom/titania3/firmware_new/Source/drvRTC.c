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

#include "hwreg.h"
#include "drvRTC.h"

#define RTC_CLOCK       FREQ_14P318MHZ    // XXX need to be changed to real clock

////////////////////////////////////////////////////////////////////////////////
// How to use RTC:
// 1. set freq_cw according to the frequency of XTAL
// 2. set cnt_en to HIGH to enable the divided clock output
// 3. program load_val for using as the initial value of RTC counter
// 4. set load_en to HIGH (this is a write&clear register), then load_val will
//    be loaded into RTC counter (32-bit)
// Then RTC will continue counting base on the divided clk rate
//
// How to read RTC value:
// 1. set read_en to HIGH, then the value of RTC counter will be double
//    buffered into rtc_cnt
// 2. SW then can read the value from rtc_cnt
//
// Additional controls:
// 1. match_value: when RTC counter is matched to this value, an interrupt
//                 will be issued (can be used as an alarm, or other application)
// 2. wrap_en: when this flag is enabled, once RTC counter is matched to
//             match_value, the counter will be reset to zero and count from zero.
////////////////////////////////////////////////////////////////////////////////

// register definition from register table

#define REG_RTC_CTRL            RTC_REG_BASE + 0x00

#define RTC_CTRL_NOT_RSTZ       (1 << 0)
#define RTC_CTRL_CNT_EN         (1 << 1)
#define RTC_CTRL_WRAP_EN        (1 << 2)
#define RTC_CTRL_LOAD_EN        (1 << 3)
#define RTC_CTRL_READ_EN        (1 << 4)
#define RTC_CTRL_INT_MASK       (1 << 5)
#define RTC_CTRL_INT_FORCE      (1 << 6)
#define RTC_CTRL_INT_CLEAR      (1 << 7)


#define REG_RTC_FREQ_CW         (RTC_REG_BASE + 0x01 * 2)
#define REG_RTC_LOAD_VAL        (RTC_REG_BASE + 0x03 * 2)
#define REG_RTC_MATCH_VAL       (RTC_REG_BASE + 0x05 * 2)
#define REG_RTC_INTERRUPT       (RTC_REG_BASE + 0x07 * 2)

#define RTC_INT_RAW_STATUS      (1 << 0)
#define RTC_INT_STATUS          (1 << 1)

#define REG_RTC_CNT             (RTC_REG_BASE + 0x08 * 2)

#define RTC_UPDATE_DELAY()      { volatile int i; for (i = 0; i < 10000; i++); }
#define RTC_RESET_DELAY()       { volatile int i; for (i = 0; i < 10000; i++); } // need 0.1ms



void MDrv_RTC_Init(U32 u32CtrlWord)
{
    XBYTE[REG_RTC_CTRL] = 0;
    XBYTE[REG_RTC_CTRL] = (RTC_CTRL_NOT_RSTZ | RTC_CTRL_INT_MASK);
    RTC_RESET_DELAY();

    XBYTE[REG_RTC_FREQ_CW + 0] = VARBYTE(u32CtrlWord, 3);
    XBYTE[REG_RTC_FREQ_CW + 1] = VARBYTE(u32CtrlWord, 2);
    XBYTE[REG_RTC_FREQ_CW + 2] = VARBYTE(u32CtrlWord, 1);
    XBYTE[REG_RTC_FREQ_CW + 3] = VARBYTE(u32CtrlWord, 0);

    XBYTE[REG_RTC_CTRL] = (RTC_CTRL_NOT_RSTZ | RTC_CTRL_CNT_EN | RTC_CTRL_INT_MASK | RTC_CTRL_INT_CLEAR);
}

U32 MDrv_RTC_GetCounter(void)
{
    XBYTE[REG_RTC_CTRL] |= RTC_CTRL_READ_EN;
    //RTC_UPDATE_DELAY();

    return ((U32)XBYTE[REG_RTC_CNT + 3] << 24) +
           ((U32)XBYTE[REG_RTC_CNT + 2] << 16) +
           ((U32)XBYTE[REG_RTC_CNT + 1] << 8) +
           ((U32)XBYTE[REG_RTC_CNT]);
}

#if ( defined(METHOD_RE_INIT_RTC) && !defined(METHOD_SYNC_1SEC_BOUNDARY) )
void MDrv_RTC_SetCounter(U32 u32Counter)
{
    XBYTE[REG_RTC_LOAD_VAL + 0] = VARBYTE(u32Counter, 3);
    XBYTE[REG_RTC_LOAD_VAL + 1] = VARBYTE(u32Counter, 2);
    XBYTE[REG_RTC_LOAD_VAL + 2] = VARBYTE(u32Counter, 1);
    XBYTE[REG_RTC_LOAD_VAL + 3] = VARBYTE(u32Counter, 0);

    XBYTE[REG_RTC_CTRL] |= RTC_CTRL_LOAD_EN;
}
#endif

void MDrv_RTC_Alarm(U32 u32AlarmCounter)
{
    U8 data u8Reg;

    u8Reg = XBYTE[REG_RTC_CTRL] | RTC_CTRL_INT_MASK | RTC_CTRL_INT_CLEAR;
    XBYTE[REG_RTC_CTRL] = u8Reg;

    if (u32AlarmCounter)
    {
        XBYTE[REG_RTC_MATCH_VAL + 0] = VARBYTE(u32AlarmCounter, 3);
        XBYTE[REG_RTC_MATCH_VAL + 1] = VARBYTE(u32AlarmCounter, 2);
        XBYTE[REG_RTC_MATCH_VAL + 2] = VARBYTE(u32AlarmCounter, 1);
        XBYTE[REG_RTC_MATCH_VAL + 3] = VARBYTE(u32AlarmCounter, 0);

        u8Reg &=~RTC_CTRL_INT_MASK;
        u8Reg &=~RTC_CTRL_INT_CLEAR;

        XBYTE[REG_RTC_CTRL] = u8Reg;
    }
}


void MDrv_RTC_ClearInterrupt(void)
{
    XBYTE[REG_RTC_CTRL] |= RTC_CTRL_INT_CLEAR;
}

