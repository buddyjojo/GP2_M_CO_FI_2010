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
#ifndef _MHAL_TTX_TYPES_H
#define _MHAL_TTX_TYPES_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ttx_types.h
/// This file contains the Mstar driver types for Teletext
/// @author MStar Semiconductor Inc.
/// @brief  Teletext module
///////////////////////////////////////////////////////////////////////////////


#if 0
#define TTX_KDBG(x1, args...)            printk(KERN_WARNING x1, ## args)
#endif
#include "chip_setup.h"

/* data type use by VD */
#define _BIT0                       0x00000001
#define _BIT1                       0x00000002
#define _BIT2                       0x00000004
#define _BIT3                       0x00000008
#define _BIT4                       0x00000010
#define _BIT5                       0x00000020
#define _BIT6                       0x00000040
#define _BIT7                       0x00000080
#define _BIT8                       0x00000100
#define _BIT9                       0x00000200
#define _BIT10                      0x00000400
#define _BIT11                      0x00000800
#define _BIT12                      0x00001000
#define _BIT13                      0x00002000
#define _BIT14                      0x00004000
#define _BIT15                      0x00008000

#define _BIT31                      0x80000000

/*
RIU Byte address to Titania2 IO address
RIU address in titania 2 is 4 byte alignment and high word address is reserved.
*/
#define BYTE2REAL(B)                (((B)>>1<<2)+((B)&0x01))

/* BITMASK used by VD */
#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)
#define BITFLAG(loc) (1U << (loc))
#define IS_BITS_SET(val, bits)      (((val)&(bits)) == (bits))

#define MAKEWORD(value1, value2)    (((U16)(value1)) * 0x100) + (value2)



#define XBYTE(addr)             X1BYTE(addr)
#define X1BYTE(addr)            *(volatile U8*)(REG_MIPS_BASE + (addr))
#define X2BYTE(addr)            *(volatile U16*)(REG_MIPS_BASE + (addr))

/* Write/Read method invalid */
#define _MHal_R1B( u32Reg )        X1BYTE(u32Reg)
#define _MHal_R2B( u32Reg )        X2BYTE(u32Reg)

#define _MHal_W1B( u32Reg, u08Val )   (X1BYTE(u32Reg) = u08Val)
#define _MHal_W2B( u32Reg, u16Val )   (X2BYTE(u32Reg) = u16Val)

#define _MHal_W1BM( u32Reg, u08Val, u08Mask )    \
    (X1BYTE(u32Reg) = (X1BYTE(u32Reg) & ~(u08Mask)) | ((u08Val) & (u08Mask)))

#define _MHal_W1Rb( u32Reg, bBit, u08BitPos ) \
    (X1BYTE(u32Reg) = (bBit) ? (X1BYTE(u32Reg) | (u08BitPos)) : (X1BYTE(u32Reg) & ~(u08BitPos)))

#define _MHal_R1Rb( u32Reg, u08BitPos ) \
    (X1BYTE(u32Reg) & (u08BitPos))



/* Video Standard type */
typedef enum
{
    E_VIDEOSTANDARD_PAL_BGHI        = 0x00,        ///< Video standard PAL BGHI
    E_VIDEOSTANDARD_NTSC_M          = 0x01,        ///< Video standard NTSC M
    E_VIDEOSTANDARD_SECAM           = 0x02,        ///< Video standard SECAM
    E_VIDEOSTANDARD_NTSC_44         = 0x03,        ///< Video standard  NTSC 44
    E_VIDEOSTANDARD_PAL_M           = 0x04,        ///< Video standard  PAL M
    E_VIDEOSTANDARD_PAL_N           = 0x05,        ///< Video standard  PAL N
    E_VIDEOSTANDARD_PAL_60          = 0x06,        ///< Video standard PAL 60
    E_VIDEOSTANDARD_NOTSTANDARD     = 0x07        ///< NOT Video standard
} VIDEOSTANDARD_TYPE;



#endif

