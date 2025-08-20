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

#ifndef __MHAL_MSMAILBOX_REG_H__
#define __MHAL_MSMAILBOX_REG_H__

#include "mdrv_types.h"

/* big endia,    D = (W[0]<<16)|W[1] = (B[0]<<24)|(B[1]<<16)|(B[2]<<8)|(B[3]) */
/* little endia, D = (W[1]<<16)|W[0] = (B[3]<<24)|(B[2]<<16)|(B[1]<<8)|(B[0]) */
typedef union
{
    unsigned int                        D;
    unsigned short                      W[2];
    unsigned char                       B[4];
} ms_reg_t;

/* hal level register access maacro functions */
#define _BITMASK(loc_msb, loc_lsb)      ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x)                      _BITMASK(1?x, 0?x)
#define BITFLAG(loc)                    (1U << (loc))
#define IS_BITS_SET(val, bits)          (((val)&(bits)) == (bits))
#define MAKEWORD(value1, value2)        (((U16)(value1)) * 0x100) + (value2)


#define X2BYTE(addr)                    *(volatile U16*)(0xBF800000 + (U32)(addr))
#define X1BYTEH(addr)                   *(volatile U8*)(0xBF800000 + (U32)(addr) + 1)
#define X1BYTEL(addr)                   *(volatile U8*)(0xBF800000 + (U32)(addr))
#define WORD2REAL(W)                    ((W)<<2)
#define _MHal_R1BH( u32Reg )            X1BYTEH(u32Reg)
#define _MHal_R1BL( u32Reg )            X1BYTEL(u32Reg)
#define _MHal_W1BH( u32Reg, u08Val )    (X1BYTEH(u32Reg) = u08Val)
#define _MHal_W1BL( u32Reg, u08Val )    (X1BYTEL(u32Reg) = u08Val)
#define _MHal_R2B( u32Reg )             X2BYTE(u32Reg)
#define _MHal_W2B( u32Reg, u16Val )     (X2BYTE(u32Reg) = (u16Val))

#define _MHal_W1BHM( u32Reg, u08Val, u08Mask )    \
    (X1BYTEH(u32Reg) = (X1BYTEH(u32Reg) & ~(u08Mask)) | ((u08Val) & (u08Mask)))

#define _MHal_W1BLM( u32Reg, u08Val, u08Mask )    \
    (X1BYTEL(u32Reg) = (X1BYTEL(u32Reg) & ~(u08Mask)) | ((u08Val) & (u08Mask)))

#define _MHal_W1BHb( u32Reg, bBit, u08BitPos ) \
    (X1BYTEH(u32Reg) = (bBit) ? (X1BYTEH(u32Reg) | (u08BitPos)) : (X1BYTEH(u32Reg) & ~(u08BitPos)))

#define _MHal_W1BLb( u32Reg, bBit, u08BitPos ) \
    (X1BYTEL(u32Reg) = (bBit) ? (X1BYTEL(u32Reg) | (u08BitPos)) : (X1BYTEL(u32Reg) & ~(u08BitPos)))

#define _MHal_R1BHb( u32Reg, u08BitPos ) \
    (X1BYTEH(u32Reg) & (u08BitPos))

#define _MHal_R1BLb( u32Reg, u08BitPos ) \
    (X1BYTEL(u32Reg) & (u08BitPos))

#define _MHal_W2BM( u32Reg, u08Val, u08Mask )    \
    (X2BYTE(u32Reg) = (X2BYTE(u32Reg) & ~(u08Mask)) | ((u08Val) & (u08Mask)))

#define _MHal_W2Bb( u32Reg, bBit, u08BitPos ) \
    (X2BYTE(u32Reg) = (bBit) ? (X2BYTE(u32Reg) | (u08BitPos)) : (X2BYTE(u32Reg) & ~(u08BitPos)))

#define _MHal_R2Bb( u32Reg, u08BitPos ) \
    (X2BYTE(u32Reg) & (u08BitPos))


#define     CPUID_PM                    0
#define     CPUID_AEON                  1
#define     CPUID_H2                    2
#define     CPUID_H3                    3
#define     CPUID_MAX_NO                4

#define INT_REG_BASE                        (0xC80)
#define INT(x)                              WORD2REAL(INT_REG_BASE+(x))

#define INTCPU_REG_BASE                     (0x2A0)
#define INTCPU(x)                           WORD2REAL(INTCPU_REG_BASE+(x))

#define MSMAILBOX_FIQ_NUM                   0x28
#endif



