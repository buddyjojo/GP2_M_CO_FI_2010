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

#ifndef _HAL_UTILITY_H_
#define _HAL_UTILITY_H_
#include <linux/spinlock.h>
#include <linux/delay.h>

#define OS_Delayms(_x_)    mdelay(_x_)

#define _BITMASK(loc_msb, loc_lsb)      ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x)                      _BITMASK(1?x, 0?x)

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//
// register operation
//
#define REG_ADDR(addr)              (*((volatile U16*)(0xBF000000 + ((addr) << 1))))


//FitchHsu MENU LOAD
#define REG_ADDRU8(addr)              (*((volatile U8*)(0xBF000000 + ((addr) << 1))))

// write 1 bytes
#define REG_W1(_reg_, _val_)        do{ REG_ADDRU8(_reg_) = (_val_); wmb(); }while(0)

// read 1 bytes
#define REG_R1(_reg_)               ({rmb(); REG_ADDRU8(_reg_);})

// write 2 bytes
#define REG_WR(_reg_, _val_)        do{ REG_ADDR(_reg_) = (_val_); wmb(); }while(0)

// read 2 bytes
#define REG_RR(_reg_)               ({rmb(); REG_ADDR(_reg_);})

// write low byte
#define REG_WL(_reg_, _val_)    \
        do{ REG_ADDR(_reg_) = (REG_ADDR(_reg_) & 0xFF00) | ((_val_) & 0x00FF); wmb(); }while(0)

// write high byte
#define REG_WH(_reg_, _val_)    \
        do{ REG_ADDR(_reg_) = (REG_ADDR(_reg_)  & 0x00FF) | ((_val_) << 8); wmb(); }while(0)

// read low byte
#define REG_RL(_reg_)               ({rmb(); ((U8)(REG_ADDR(_reg_) & 0x00FF));})

// read high byte
#define REG_RH(_reg_)               ({rmb(); ((U8)((REG_ADDR(_reg_) & 0xFF00) >> 8));})

// write 3 bytes
#define REG_W3(_reg_, _val_)               \
        do{ REG_WR(_reg_, (_val_));            \
        REG_WL((_reg_ + 2), (_val_) >> 16);\
        wmb(); }while(0)

// write 4 bytes
#define REG_W4(_reg_, _val_)                 \
        do {REG_WR((_reg_), (_val_));  wmb();    \
        REG_WR((_reg_ + 2), ((_val_) >> 16)); wmb(); }while(0)

// write bit
#define REG_WI(_reg_, _bit_, _pos_)    \
		do{ rmb();                         \
        REG_ADDR(_reg_) = (_bit_) ? (REG_ADDR(_reg_) | _pos_) : (REG_ADDR(_reg_) & ~(_pos_));  \
        wmb(); }while(0)

// read bit
#define REG_RI(_reg_, _pos_)        ({ rmb(); (REG_ADDR(_reg_) & _pos_);})

// write mask
#define REG_WM(_reg_, _val_, _msk_)    \
		do{ rmb();                         \
        REG_ADDR(_reg_) = (REG_ADDR(_reg_) & ~(_msk_)) | ((_val_) & (_msk_)); wmb(); }while(0)

// write Low bye mask
#define REG_L_WM(_reg_, _val_, _msk_)  \
		do{ rmb();                    \
        REG_ADDR(_reg_) = ((REG_ADDR(_reg_) & 0xFF00) & ~(_msk_)) | (((_val_) & 0x00FF) & (_msk_)); wmb(); }while(0)

// write High bye mask
#define REG_H_WM(_reg_, _val_, _msk_)  \
		do{ rmb();                         \
        REG_ADDR(_reg_) = ((REG_ADDR(_reg_)  & 0x00FF) & ~(_msk_)) | (((_val_) << 8) & (_msk_)); wmb(); }while(0)

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures
//-------------------------------------------------------------------------------------------------
extern void PROBE_INT_ENTRY(int INTNum);
extern void PROBE_INT_EXIT(int INTNum);

#if 1
#define MENULOAD_LOCK           {spin_lock_irqsave(&menuload_lock, irq_flags_menuload);PROBE_INT_ENTRY(0xFF);};
#define MENULOAD_UNLOCK         {spin_unlock_irqrestore(&menuload_lock, irq_flags_menuload); PROBE_INT_EXIT(0xFF);};
#else
#define MENULOAD_LOCK
#define MENULOAD_UNLOCK
#endif

#if 1
#define SWITCH_BANK_LOCK        {spin_lock_irqsave(&switch_bk_lock, irq_flags_swbk);PROBE_INT_ENTRY(0xFE);}
#define SWITCH_BANK_UNLOCK      {spin_unlock_irqrestore(&switch_bk_lock, irq_flags_swbk); PROBE_INT_EXIT(0xFE);}
#else
#define SWITCH_BANK_LOCK
#define SWITCH_BANK_UNLOCK
#endif

// store bank
#define SC_BK_STORE     \
        do{ \
        SWITCH_BANK_LOCK; \
		rmb();          \
        u8switch_bk_lock = REG_ADDR(REG_SCALER_BASE); } \
        while(0);


// restore bank
#define SC_BK_RESTORE   			\
        do{ \
        REG_WL(REG_SCALER_BASE, u8switch_bk_lock); 	\
        wmb();  \
        SWITCH_BANK_UNLOCK;  } \
        while(0);

// switch bank
#define SC_BK_SWICH(_x_)		\
        do{ \
        REG_WL(REG_SCALER_BASE, _x_);	\
        wmb();   } \
        while(0);

// store Comb bank
#define COMB_BK_STORE     \
        do{ \
        SWITCH_BANK_LOCK; \
        rmb();				\
        u8switch_bk_lock = REG_ADDR(REG_COMB_BASE);   } \
        while(0);

// restore Comb bank
#define COMB_BK_RESTORE   			\
        do{ \
        REG_WL(REG_COMB_BASE, u8switch_bk_lock);  	\
        wmb();  \
        SWITCH_BANK_UNLOCK;   } \
        while(0);

// switch Comb bank
#define COMB_BK_SWICH(_x_)		\
        do{ \
        REG_WL(REG_COMB_BASE, _x_);	\
        wmb();   } \
        while(0);

// store MOD bank
#define MOD_BK_STORE     \
        do{ \
        SWITCH_BANK_LOCK; \
		rmb();          \
        u8switch_bk_lock = REG_ADDR(REG_MOD_BASE);   } \
        while(0);

// restore MOD bank
#define MOD_BK_RESTORE   \
        do{ \
        REG_WL(REG_MOD_BASE, u8switch_bk_lock);   \
        wmb();  \
        SWITCH_BANK_UNLOCK;   } \
        while(0);

// switch MOD bank
#define MOD_BK_SWICH(_x_)\
        do{ \
        REG_WL(REG_MOD_BASE, _x_);\
        wmb();   } \
        while(0);

#endif // _HAL_UTILITY_H_
