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

#ifdef RED_LION
#include "MDevTypes.h"
#endif

//-------------------------------------------------------------------------------------------------
// RLD Registers
//-------------------------------------------------------------------------------------------------

#define RLD_CHIPTOP              (0xbf803c88)
#define RLD_ADDRESS              (0x1480*4)
#define REG_RLD_BASE             (0xbf800000 + RLD_ADDRESS)
#define RLD_Reg(address)         (*((volatile U32 *)(REG_RLD_BASE + ((address)<<2) )))

#define STRLD_CTRL               ( 0x00 )
#define STRLD_OUTPUT_FORMAT      ( 0x01 )       // I2/I4/I8 [7:0]     //
#define STRLD_ENLARGE_RATE       ( 0x01 )       // 1 ~ 7 [15:8]
#define STRLD_TRANSPARENT_KEY    ( 0x02 )       // transparent key [7:0]

#define STRLD_TOP_ADDR0          ( 0x10 )       // top field address [ 15: 0]
#define STRLD_TOP_ADDR1          ( 0x11 )       // top field address [31:16]

#define STRLD_TOP_LEN            ( 0x12 )       // top field length [15: 0]

#define STRLD_BOT_ADDR0          ( 0x13 )       // bottom field address [15: 0]
#define STRLD_BOT_ADDR1          ( 0x14 )       // bottom field address [31:16]

#define STRLD_BOT_LEN            ( 0x15 )       // bottom field length [15: 0]

#define STRLD_OUTPUT_ADDR0       ( 0x16 )       // output address [15: 0]
#define STRLD_OUTPUT_ADDR1       ( 0x17 )       // output address [31:16]

#define STRLD_REGION_WIDTH       ( 0x18 )       // Region width [15: 0]

#define STRLD_REGION_HEIGHT      ( 0x19 )       // Region height [15: 0]

#define STRLD_REGION_PITCH       ( 0x1A )       // Region pitch [15: 0]

#define STRLD_OBJ_X_OFFSET       ( 0x1B )       // Object x-offset in the region [15: 0]

#define STRLD_OBJ_Y_OFFSET       ( 0x1C )       // Object y-offset in the region [15: 0]

#define STRLD_REGION_OFFSET      ( 0x1D )       // Region offset [7:0]
#define STRLD_BITSTREAM_DEPTH    ( 0x1D )       // The color depth encoded in the bit stream [15:8]
#define STRLD_REGION_BG_INDEX    ( 0x1E )       // Region color background index [7:0]
#define STRLD_REGION_FG_INDEX    ( 0x1E )       // Region color foreground index [15:8]

#define RLD_MPEG_CTRL            ( 0x20 )
#define RLD_MPEG_CTRL1           ( 0x21 )
#define RLD_MPEG_RETURN          ( 0x2F )		// Return registor, read only
#define MASK_BOTTOM_FIELD          0x02        //0x2942[1]

#define STRLD_CLUT_2TO4_0        ( 0x50 )       // CLUT from 2 to 4
#define STRLD_CLUT_2TO4_1        ( 0x51 )       // CLUT from 2 to 4

#define STRLD_CLUT_2TO8_0        ( 0x52 )       // CLUT from 2 to 8
#define STRLD_CLUT_2TO8_1        ( 0x53 )       // CLUT from 2 to 8

#define STRLD_CLUT_4TO8_0        ( 0x54 )       // CLUT from 4 to 8
#define STRLD_CLUT_4TO8_1        ( 0x55 )       // CLUT from 4 to 8
#define STRLD_CLUT_4TO8_2        ( 0x56 )       // CLUT from 4 to 8
#define STRLD_CLUT_4TO8_3        ( 0x57 )       // CLUT from 4 to 8
#define STRLD_CLUT_4TO8_4        ( 0x58 )       // CLUT from 4 to 8
#define STRLD_CLUT_4TO8_5        ( 0x59 )       // CLUT from 4 to 8
#define STRLD_CLUT_4TO8_6        ( 0x5A )       // CLUT from 4 to 8
#define STRLD_CLUT_4TO8_7        ( 0x5B )       // CLUT from 4 to 8


#define STRLD_PATCH              ( 0x1E )       // Patch for Neptune RLD [15:8]
#define STRLD_PATCH_06           ( 0x03 )
#define STRLD_RETURN             ( 0x70 )       // Return registor, read only [7:0]

#define MASK_OUT_OF_WIDTH_PATCH    0x0100       // BIT 0
#define MASK_NON_MODIFYING_PATCH   0x0200       // BIT 1
#define MASK_8BYTE_ALIGHMENT_PATCH 0x0400       // BIT 2
#define MASK_END_LASTDATA_PATCH    0x0800       // BIT 3
#define MASK_COLOR_REDUCTION_PATCH 0x4000       // BIT 6
#define MASK_COLOR_MAPPING_PATCH   0x8000       // BIT 7

#define MASK_NON_MODIFYING_FLAG   0x02  // NEPTUNE : 0x293D[1]



