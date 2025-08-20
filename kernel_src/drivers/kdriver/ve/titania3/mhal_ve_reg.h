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
#ifndef _MHAL_VE_REG_H
#define _MHAL_VE_REG_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ve_reg.h
/// This file contains the Mstar driver for TVEncoder
/// @author MStar Semiconductor Inc.
/// @brief  TVEncoder module
///////////////////////////////////////////////////////////////////////////////
#include "chip_setup.h"

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

#define REG_BASE_VE_SOURCE      0x1D80
#define REG_BASE_VE_ENCODER     0x1F00
#define REG_BASE_ADC_TOP        0x1280
#define REG_BASE_CHIPTOP        0x0F00
#define REG_BASE_IPMUX          0x1700
#define REG_BASE_CLKGEN0        0x0580
//#define REG_BASE_VBI            0x1B80

#define TVEncoder_REG(addr) (*(volatile u16 *)(REG_MIPS_BASE + (addr)))

// For table use
#define BK_ALL(x,y) ((x+y)<<2)

#define _BK_VE_SRC(x)   BK_ALL(REG_BASE_VE_SOURCE,x)
#define _BK_VE_ENC(x)   BK_ALL(REG_BASE_VE_ENCODER,x)

// For general read/write
#define BK_VE_SRC(x)    (((REG_BASE_VE_SOURCE+x)<<2))
#define BK_VE_ENC(x)    (((REG_BASE_VE_ENCODER+x)<<2))
#define BK_ADC_ATOP(x)  (((REG_BASE_ADC_TOP+x)<<2))
#define BK_IP_MUX(x)    (((REG_BASE_IPMUX+x)<<2))
#define BK_CLKGEN0(x)   (((REG_BASE_CLKGEN0+x)<<2))
//#define BK_VBI_DMA(x)   (((REG_BASE_VBI+x)<<2))
#define BK_CHIPTOP(x)   (((REG_BASE_CHIPTOP+x)<<2))


//#define RLD_CHIPTOP              (0xbf803c88)
//#define ADC_ADDRESS              (0x1280*4)
//#define REG_ADC_BASE             (0xbf800000 + ADC_ADDRESS)
//#define ADC_Reg(address)         (*((volatile U32 *)(REG_ADC_BASE + ((address)<<2) )))
//------------------------------------------------------------------------------
// register table
//------------------------------------------------------------------------------
#define BK_LOW_BIT          0x0001
#define BK_HIGH_BIT         0x0002
#define BK_LH_MASK          0xFFFC

//VE out
#define VE_OUT_CVBS_YCC         0x00
#define VE_OUT_CVBS_YCbCr       0x01
#define VE_OUT_CVBS_RGB         0x10
#define VE_OUT_NONE             0xFF

#define REG_CKG_VE_IN           0x1E38
    #define CKG_VE_IN_GATED         BIT0
    #define CKG_VE_IN_INVERT        BIT1
    #define CKG_VE_IN_MASK          (BIT5 | BIT4 | BIT3 | BIT2)
    #define CKG_VE_IN_CLK_ADC       (0 << 2)
    #define CKG_VE_IN_CLK_DVI       (1 << 2)
    #define CKG_VE_IN_CLK_VD        (2 << 2)
    #define CKG_VE_IN_CLK_MPEG0     (3 << 2)
    #define CKG_VE_IN_1             (4 << 2)
    #define CKG_VE_IN_CLK_EXT_DI    (5 << 2)
    #define CKG_VE_IN_0             (6 << 2)
    #define CKG_VE_IN_0_            (7 << 2)
    #define CKG_VE_IN_DFT_LIVE      (8 << 2)
#endif

