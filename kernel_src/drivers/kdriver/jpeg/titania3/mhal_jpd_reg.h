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

#ifndef _MHAL_JPD_REG_H_
#define _MHAL_JPD_REG_H_

#include "mdrv_types.h"

#define REG_JPD_BASE               0xBF202E00  // (0xBF200000 + 0xB80<<2)
#define JPD_REG(addr)              (*((volatile U32*)(REG_JPD_BASE + ((addr)<<2))))


//******************************************************************************
//          Structures
//******************************************************************************

#define AEON_NON_CACHE_MASK  0x80000000  //need to check   sharon

//#define JPD_REG_BASE        0x3000

/******* Internal table SRAM address *******/
#define JPD_MEM_SCWGIF_BASE     0x0000
#define JPD_MEM_SYMIDX_BASE     0x0200
#define JPD_MEM_QTBL_BASE       0x0400
/**************************************/
#define JPD_SWRST_S4L   (1<<13)   // Titania
#define JPD_ROI_EN      (1<<11)
#define JPD_SVLD        (1<<10)
#define JPD_SUVQ        (1<<9)
#define JPD_TBL_RDY     (1<<8)
#define JPD_DEC_EN      (1<<7)
#define JPD_RST_EN      (1<<6)
#define JPD_UV          (1<<3)
#define JPD_Y_VSF1      (0<<2)
#define JPD_Y_VSF2      (1<<2)
#define JPD_Y_HSF1      0x01
#define JPD_Y_HSF2      0x02
#define JPD_Y_HSF4      0x03

#define JPD_H_VLD       (1<<1)
#define JPD_L_VLD       (1<<0)

#define OFFSET(x)       (x*2)

#define REG_JPD_SCONFIG          0x00
#define REG_JPD_MCONFIG          0x01
#define REG_JPD_RSTINTV          0x02
#define REG_JPD_PIC_H            0x03
#define REG_JPD_PIC_V            0x04

#define REG_JPD_ROI_H            0x05
#define REG_JPD_ROI_V            0x06
#define REG_JPD_ROI_WIDTH        0x07
#define REG_JPD_ROI_HEIGHT       0x08

#define REG_JPD_INTEN            0x09
#define REG_JPD_EVENTFLAG        0x0A
#define REG_JPD_RCSMADR_L        0x0B
#define REG_JPD_RCSMADR_H        0x0C
#define REG_JPD_RBUF_FLOOR_L     0x0D
#define REG_JPD_RBUF_FLOOR_H     0x0E
#define REG_JPD_RBUF_CEIL_L      0x0F
#define REG_JPD_RBUF_CEIL_H      0x10
#define REG_JPD_MWBF_SADR_L      0x11
#define REG_JPD_MWBF_SADR_H      0x12

// only T3, T2 does NOT have this
#define REG_JPD_MWBF_LINE_NUM    0x13

#define REG_JPD_CUR_MADR_L       0x14
#define REG_JPD_CUR_MADR_H       0x15
#define REG_JPD_CUR_ROWP         0x16
#define REG_JPD_CUR_CLNP         0x17
#define REG_JPD_CUR_VIDX         0x18
#define REG_JPD_BIST_FAIL        0x19
#define REG_JPD_DBG_MATCHV       0x1A
#define REG_JPD_DBG_CTRL         0x1B

// only T3, T2 does NOT have this
#define REG_JPD_IP_VERSION       0x28

#define REG_JPD_TID_ADR          0x40
#define REG_JPD_TID_DAT          0x41

// Chip TOP  //need to check    sharon
#define JPD_CLOCK_S4    (OFFSET(0x0F2D))  // Eris
#define JPD_CLOCK_S4L   (OFFSET(0x0F4D))    // Titania

#endif

