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
#ifndef __MDRV_LOGO_SCALER_H__
#define __MDRV_LOGO_SCALER_H__

//#include <common.h>
#include <asm/wbflush.h>

//
// Scaler setting
//-----------------------------------------------------------------
#define REG_ADDR(addr)              (*((volatile unsigned short int*)(0xBF000000 + (addr << 1))))

#define REG_CKGEN0_BASE             0x100B00
//
// Ckgen0 register definition
//
#define REG_CKGEN0(_x_)             (REG_CKGEN0_BASE | (_x_ << 1))

#define REG_SCALER_BASE             0x102F00
#define REG_SC_BANK(_x_)            (REG_SCALER_BASE | (_x_ << 1))

// LPLL
#define REG_LPLL_BASE               0x103100
#define REG_LPLL(_x_)               (REG_LPLL_BASE | (_x_ << 1))

#define FREQ_12MHZ                  (12000000UL)
#define MST_XTAL_CLOCK_HZ           FREQ_12MHZ  //Neptune: FREQ_12MHZ  //FPGA:FREQ_14P318MHZ

#define REG_MOD_BASE                0x103200
#define MOD_REG(_x_)                (REG_MOD_BASE | (_x_ << 1))

#define MST_XTAL_CLOCK_KHZ          12000

#define REG_SC_BK_GOPINT            0x00
#define REG_SC_BK_VOP               0x10
#define REG_SC_BK_SCMI              0x12
#define REG_SC_BK_0X12              0x12

#define REG_SC_GOPINT(_x_)          REG_SC_BANK(_x_)
#define REG_SC_VOP(_x_)             REG_SC_BANK(_x_)
#define REG_SC_SCMI(_x_)            REG_SC_BANK(_x_)

// store bank
#define SC_BK_STORE     \
        U8 u8Bank;      \
        u8Bank = REG_ADDR(REG_SCALER_BASE)

// restore bank
#define SC_BK_RESTORE   \
        REG_ADDR(REG_SCALER_BASE) = u8Bank;
// switch bank
#define SC_BK_SWICH(_x_)\
        REG_ADDR(REG_SCALER_BASE) = _x_;

//
// MOD bank
//
#define REG_MOD_BK_00                0x00
#define REG_MOD_BK_01                0x01

// store MOD bank
#define MOD_BK_STORE     \
        U8 u8Bank_mod;      \
        u8Bank_mod = REG_ADDR(REG_MOD_BASE)

// restore MOD bank
#define MOD_BK_RESTORE   \
        REG_WL(REG_MOD_BASE, u8Bank_mod);

// switch MOD bank
#define MOD_BK_SWICH(_x_)\
        REG_WL(REG_MOD_BASE, _x_);

// write low byte
#define REG_WL(_reg_, _val_)    \
        do{ REG_ADDR(_reg_) = (REG_ADDR(_reg_) & 0xFF00) | ((_val_) & 0x00FF); }while(0)

// write 2 bytes
#define REG_WR(_reg_, _val_)        do{ REG_ADDR(_reg_) = (_val_); }while(0)

// write 3 bytes
#define REG_W3(_reg_, _val_)               \
        do{ REG_WR(_reg_, (_val_));            \
        REG_WL((_reg_ + 2), (_val_) >> 16);\
        }while(0)

// write 4 bytes
#define REG_W4(_reg_, _val_)                 \
        do {REG_WR((_reg_), (_val_));  \
        REG_WR((_reg_ + 2), ((_val_) >> 16));  }while(0)

// write mask
#define REG_WM(_reg_, _val_, _msk_)    \
		do{                          \
        REG_ADDR(_reg_) = (REG_ADDR(_reg_) & ~(_msk_)) | ((_val_) & _msk_);  }while(0)

// write high byte
#define REG_WH(_reg_, _val_)    \
        do{ REG_ADDR(_reg_) = (REG_ADDR(_reg_)  & 0x00FF) | ((_val_) << 8);  }while(0)

// write bit
#define REG_WI(_reg_, _bit_, _pos_)    \
		do{                        \
        REG_ADDR(_reg_) = (_bit_) ? (REG_ADDR(_reg_) | _pos_) : (REG_ADDR(_reg_) & ~(_pos_));  \
        }while(0)

#define REG_RR(_reg_)                    ({ REG_ADDR(_reg_);})

/// LPLL mode
typedef enum
{
    G_LPLL_MODE_SINGLE = 0,
    G_LPLL_MODE_DUAL   = 1,
} G_LPLL_MODE_t;
// redefined

// Ckgen0

// MHal_SC_EnableClock
typedef enum
{
    G_SC_ENCLK_TTL            = 0x00,
    G_SC_ENCLK_SIGNAL_LVDS    = 0x11,
    G_SC_ENCLK_DUAL_LVDS      = 0x13,
} G_HAL_SC_ENCLK_e;


typedef enum
{
    PANEL_RES_WXGA,
    PANEL_RES_FULL_HD,
    PANEL_RES_MAX_NUM,
} PANEL_RESOLUTION_TYPE_e;

typedef enum
{
    LPLL_LVDS,
    LPLL_RSDS,
    LPLL_TTL,
    // 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain
    LPLL_MINILVDS   = 3,
} SC_MOD_LPLL_TYPE_e;


void MDrv_Logo_SC_Init(PANEL_RESOLUTION_TYPE_e e_panel_type, SC_MOD_LPLL_TYPE_e e_lpll_type);

#endif
