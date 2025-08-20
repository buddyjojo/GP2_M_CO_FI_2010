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
/// @file drvGE.h
/// @brief Graphics Engine Driver Interface
/// @author MStar Semiconductor Inc.
///
/// GE is used to paint OSD
///
/// Features:
/// - BLT engine : fast and simply blt function
/// - Pixel engine(PE) : complex blt function
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __DRVGE_H__
#define __DRVGE_H__

// These type define can be removed after the coding style is confirmed.
//#include <asm-mips/types.h>

#ifdef __KERNEL__
//#include "MDevTypes.h"
#include "mdrv_types.h"
#else
// This include can be reomved later
#include "MsCommon.h"
#endif

#define MAX_FONT                            20  //5
#define MAX_BITMAP                          64

#define ERR_HANDLE                          -1

#define GE_PALETTE_ENTRY_NUM        256

//------------------------------------------------------------------------------
// macro
//------------------------------------------------------------------------------

//GE return code
/// GE DDI return value
#define GESTATUS                            U32
/// @name GESTATUS
/// GESTATUS return value
/// @{
#define GESTATUS_SUCCESS                    0x00000000  //0
#define GESTATUS_FAIL                       0x00000001  //Bit(0)
#define GESTATUS_NON_ALIGN_ADDRESS          0x00000002  //Bit(1)
#define GESTATUS_NON_ALIGN_PITCH            0x00000004  //Bit(2)
#define GESTATUS_INVALID_INTENSITY_ID       0x00000008  //Bit(3)
#define GESTATUS_INVALID_BUFF_INFO          0x00000010  //Bit(4)
#define GESTATUS_INVALID_FONT_HANDLE        0x00000020  //Bit(5)
#define GESTATUS_INVALID_BMP_HANDLE         0x00000040  //Bit(6)
#define GESTATUS_INVALID_PARAMETERS         0x00000080  //Bit(7)
//#define GESTATUS_OUTOF_MEMORY               0x00000040  //Bit(6)
//#define GESTATUS_INVALID_COMMAND            0x00000080  //Bit(7)

/// The above lines are moved from drvGE.h


//------------------------------------------------------------------------------
// enum
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// structure
//------------------------------------------------------------------------------

typedef S16 FONTHANDLE;      ///< Font handle
typedef S16 BMPHANDLE;       ///< Bitmap handle
typedef S16 DBHANDLE;


///Define Raster Operation
typedef enum
{
    /// rop_result = 0;
    ROP2_OP_ZORE          = 0,
    /// rop_result = ~( rop_src | rop_dst );
    ROP2_OP_NOT_PS_OR_PD  = 1,
    /// rop_result = ((~rop_src) & rop_dst);
    ROP2_OP_NS_AND_PD     = 2,
    /// rop_result = ~(rop_src);
    ROP2_OP_NS            = 3,
    /// rop_result = (rop_src & (~rop_dst));
    ROP2_OP_PS_AND_ND     = 4,
    /// rop_result = ~(rop_dst);
    ROP2_OP_ND            = 5,
    /// rop_result = ( rop_src ^ rop_dst);
    ROP2_OP_PS_XOR_PD     = 6,
    /// rop_result = ~(rop_src & rop_dst);
    ROP2_OP_NOT_PS_AND_PD = 7,
    /// rop_result = (rop_src & rop_dst);
    ROP2_OP_PS_AND_PD     = 8,
    /// rop_result = ~(rop_dst ^ rop_src);
    ROP2_OP_NOT_PS_XOR_PD = 9,
    /// rop_result = rop_dst;
    ROP2_OP_PD            = 10,
    /// rop_result = (rop_dst | (~rop_src));
    ROP2_OP_NS_OR_PD      = 11,
    /// rop_result = rop_src;
    ROP2_OP_PS            = 12,
    /// rop_result = (rop_src | (~rop_dst));
    ROP2_OP_PS_OR_ND      = 13,
    /// rop_result = (rop_dst | rop_src);
    ROP2_OP_PD_OR_PS      = 14,
    /// rop_result = 0xffffff;
    ROP2_OP_ONE           = 15,
}GE_ROP2_OP;


///Define Raster Operation8
typedef enum GE_ROP8_OP_s
{
    ROP8_COPY           = 0,
    ROP8_XOR            = 1,
    ROP8_ALPHA          = 2,
    ROP8_TRANSPARENT    = 3,
    ROP8_DVB_CLEAR      = 4,
    ROP8_DVB_SRC        = 5,
    ROP8_DVB_SRC_OVER   = 6,
    ROP8_DVB_DEST_OVER  = 7,
    ROP8_DVB_SRC_IN     = 8,
    ROP8_DVB_DEST_IN    = 9,
    ROP8_DVB_SRC_OUT    = 10,
    ROP8_DVB_DEST_OUT   = 11,
}GE_ROP8_OP_t;



/// Color Key Operation Mode
typedef enum
{
    /// Not equal
    CK_OP_NOT_EQUQL = 0,
    /// Equal
    CK_OP_EQUQL = 1,
}GE_COLOR_KEY_MODE;

///Define Blending Coefficient
#if 0
typedef enum
{
    /// 1
    COEF_ONE     =  0,
    /// constant
    COEF_CONST   =  1,
    /// source alpha
    COEF_ASRC    =  2,
    /// destination alpha
    COEF_ADST    =  3,
    /// 0
    COEF_ZERO    =  4,
    /// 1 - constant
    COEF_1_CONST =  5,
    /// 1 - source alpha
    COEF_1_ASRC  =  6,
    /// 1 - destination alpha
    COEF_1_ADST  =  7,
}GE_BLEND_COEF;
#else
typedef enum
{
    /// Csrc
    COEF_ONE     =  0,
    /// Csrc * Aconst + Cdst * (1 - Aconst)
    COEF_CONST   =  1,
    /// Csrc * Asrc + Cdst * (1 - Asrc)
    COEF_ASRC    =  2,
    /// Csrc * Adst + Cdst * (1 - Adst)
    COEF_ADST    =  3,
    /// ((Asrc * Aconst) * Csrc + (1-(Asrc *Aconst)) * Cdst) / 2
    COEF_ASRCxConst_DIV_2 = 4,
    /// ((Asrc * Aconst) * Csrc + Adst * Cdst * (1-(Asrc * Aconst))) / (Asrc * Aconst) + Adst * (1- Asrc * Aconst))
    COEF_SRC_OVER = 5,
    /// ((Asrc * Aconst) * Csrc * (1-Adst) + Adst * Cdst) / (Asrc * Aconst) * (1-Adst) + Adst)
    COEF_DST_OVER =6,
    /// Csrc*Aconst
    COEF_CSRCxCONST = 7,
    /// Cdst
    COEF_ZERO    =  8,
    /// Csrc * (1 - Aconst) + Cdst * Aconst
    COEF_1_CONST =  9,
    /// Csrc * (1 - Asrc) + Cdst * Asrc
    COEF_1_ASRC  =  10,
    /// Csrc * (1 - Adst) + Cdst * Adst
    COEF_1_ADST  =  11,
    /// (Csrc * Adst * Asrc * Aconst + Cdst * Adst * (1 - Asrc * Aconst)) / (Adst * Asrc * Aconst + Adst * (1 - Asrc * Aconst)
    COEF_1_ASRCxCONST = 12,
	/// (Cdst * Asrc * Aconst * Adst + Csrc * Asrc * Aconst * (1 - Adst)) / (Asrc * Aconst * (1 - Adst) + Asrc * Aconst * Adst)
	COEF_ACONST_1_ADST = 13,
	/// ((1 - Adst) * Csrc * Asrc * Aconst + Adst * Cdst * (1 - Asrc * Aconst)) / (Asrc * Aconst * (1 - Adst) + Adst * (1 - Asrc * Aconst))
	COEF_ADST_1_ASRCxCONST = 14,
	/// Csrc * (1 - Aconst)
	COEF_CSRCx1_ACONST = 15
}GE_BLEND_COEF;
#endif

///Define Blending Source from
#if 0
typedef enum
{
    /// constant
    ABL_FROM_CONST = 0,
    /// source alpha
    ABL_FROM_ASRC  = 1,
    /// destination alpha
    ABL_FROM_ADST  = 2,
}GE_ALPHA_SRC_FROM;
#else
typedef enum
{
    /// Aconst
    ABL_FROM_CONST = 0,
    /// Asrc
    ABL_FROM_ASRC  = 1,
    /// Adst
    ABL_FROM_ADST  = 2,
    /// Asrc * Aconst
    ABL_ASRCxCONST = 3,
    /// Asrc * Aconst * Adst
    ABL_ASRCxCONSTxADST = 4,
    /// Adst - Adst * Asrc * Aconst
    ABL_ADST_MINUS_ADSTxASRCxCONST = 5,
    /// Asrc * Aconst - Adst * Asrc * Aconst
    ABL_ASRCxCONST_MINUS_ADSTxASRCxCONST = 6,
    /// Asrc * Aconst - Adst * Asrc * Aconst + Adst
    ABL_ASRCxCONST_MINUS_ADSTxASRCxCONST_PLUS_ADST = 7,
    /// 1 - Aconst
    ABL_ONE_MINUS_CONST = 8,
    /// 1 - Asrc
    ABL_ONE_MINUS_ASRC = 9,
    /// 1 - Adst
    ABL_ONE_MINUS_ADST = 10,
    /// Adst * Asrc * Aconst + Adst * (1 - Asrc * Aconst) A atop B
    ABL_A_ATOP_B = 11,
    /// Asrc * Aconst * Adst + Asrc * Aconst * (1 - Adst) B atop A
    ABL_B_ATOP_A = 12,
    /// (1 - Adst) * Asrc * Aconst + Adst * (1 - Asrc * Aconst) A xor B
    ABL_A_XOR_B = 13,
    /// Asrc * Asrc * Aconst + Adst * (1 - Asrc * Aconst)
    ABL_ASRCxASRCxCONST_PLUS_ADSTxONE_MINUS_ASRCxCONST = 14,
    /// Asrc * (1 - Asrc * Aconst) + Adst * Asrc * Aconst
    ABL_ASRCxONE_MINUS_ASRCxACONST_PLUS_ADSTxASRCxCONST = 15    
}GE_ALPHA_SRC_FROM;
#endif

/// GE buffer format
typedef enum
{
    /// font mode I1
    GE_FMT_I1                   = 0x0,
    /// font mode I2
    GE_FMT_I2                   = 0x1,
    /// font mode I4
    GE_FMT_I4                   = 0x2,
    /// color format palette 256(I8)
    GE_FMT_I8                   = 0x4,
    /// Uranus GOP only support this blinking
    /// color format for blinking display format
    GE_FMT_1ABFGBG12355         = 0x7,
    /// color format RGB565
    GE_FMT_RGB565               = 0x8,
    /// color format ORGB1555
    GE_FMT_ARGB1555             = 0x9,
    /// color format ARGB4444
    GE_FMT_ARGB4444             = 0xa,
    /// color format for blinking display format
    /// Uranus GOP does NOT support this blinking format
    // GE_FMT_1BAAFGBG123433       = 0xb,
    // @FIXME: Venus does not have this color format. Need to take care of it.
    
    /// color format ARGB1555 of destination buffer
    GE_FMT_ARGB1555_DST            = 0xc,
    /// color format YUV422
    GE_FMT_YUV422               = 0xe,
    /// color format ARGB8888
    GE_FMT_ARGB8888             = 0xf
}GE_Buffer_Format;

///Define Drawing type
typedef enum
{
    /// none
    GE_DRAW_NONE       = 0,
    /// draw line
    GE_DRAW_LINE       = 1,
    /// rectangle fill
    GE_DRAW_RECTANGLE  = 3,
    /// bitblt
    GE_DRAW_BITBLT     = 4,
}GE_DRAW_TYPE;

//-------------------------------------------------
/// Define color
typedef struct
{
    /// Blue
    U8 b;
    /// Green
    U8 g;
    /// Red
    U8 r;
    /// Alpha
    U8 a;
} GE_RGB_COLOR;

typedef struct
{
    U32         Y:8;
    U32         U:4;
    U32         V:4;
} GE_YUV_COLOR;

typedef struct{
    union{
        GE_RGB_COLOR    rgb;
        GE_YUV_COLOR    yuv;
    };
} GE_COLOR;

//-------------------------------------------------
/// Define the start color & end color
typedef struct
{
    /// start color
    GE_RGB_COLOR color_s;
    /// end color
    GE_RGB_COLOR color_e;
} GE_COLOR_RANGE;

//-------------------------------------------------
/// Define the position of one point.
typedef struct GE_POINT_s
{
    /// X address
    U16 x;
    /// Y address
    U16 y;
} GE_POINT_t;

//-------------------------------------------------
/// Specify the blink data
#if 0
                    // 1 B A A Fg Bg
                    // 1 2 3 4  3  3
typedef struct
{
    /// BG color (for blink mode use)
    U8 background;
    /// FG color (for blink mode use)
    U8 foreground;
    /// Control flag\n
    union{
        U16 ctrl_flag;
        struct{
            U16 BlinkAlpha:4;
            U16 Alpha:3;
            U16 Blink:2;
            U16 reserved:7;
        }Bits;
    };

} GE_BLINK_DATA;
#else
        //          1 A B Fg Bg
        //          1 2 3  5  5
typedef struct
{
    /// BG color (for blink mode use)
    U8                  background;             // 5 bits
    /// FG color (for blink mode use)
    U8                  foreground;             // 5 bits
    /// Control flag\n
    union{
        U16             ctrl_flag;
        struct{
            U16         Blink:3;                // 3 bits
            U16         Alpha:2;                // 2 bits
            U16         reserved1:11;
        } Bits;
    };

} GE_BLINK_DATA;

#endif

/// Define the start blink color & end blink color.
typedef struct
{
    /// start blink color
    GE_BLINK_DATA blink_data_s;
    /// end blink color
    GE_BLINK_DATA blink_data_e;
} GE_BLINK_DATA_RANGE;

//-------------------------------------------------
//-------------------------------------------------
#define GELINE_FLAG_COLOR_CONSTANT      0x00
#define GELINE_FLAG_COLOR_GRADIENT      0x01
/// Define the attribute of line.
typedef struct
{
    /// start X address
    U16 x1;
    /// Start Y address
    U16 y1;
    /// End X address
    U16 x2;
    /// End Y address
    U16 y2;
    /// Color format
    GE_Buffer_Format fmt;
    union
    {
        /// Color
        ///For all RGB color, the color set as the ARGB8888 format.\n
        ///Each color component need to shift to high bit.\n
        ///Use ARGB1555 as the example, the source color key as the following:\n
        ///ARGB1555  --> ARRRRRGGGGGBBBBB                   (every character represents one bit)\n
        ///colorRange.color_s --> A0000000RRRRR000GGGGG000BBBBB000   (every character represents one bit)\n\n
        ///For GE_FMT_I8 format, the index set to b component (colorRange.color_s.b = b).\n
        GE_COLOR_RANGE       colorRange;
        ///Blink attribute
        ///For GE_FMT_1BAAFGBG123433 foramt, the foramt set as the GE_BLINK_DATA.\n
        GE_BLINK_DATA_RANGE  blkDataRange;
    };
    /// Line width
    U32 width;
    /// GELINE_FLAG_COLOR_CONSTANT: Constant color\n
    /// GELINE_FLAG_COLOR_GRADIENT: Gradient color
    U32 flag;

} GE_DRAW_LINE_INFO;

//-------------------------------------------------

/// Define the dimension of one block
typedef struct
{
    /// X start address
    U16 x;
    /// Y start address
    U16 y;
    /// width
    U16 width;
    /// height
    U16 height;
} GE_BLOCK;


//-------------------------------------------------

#define GERECT_FLAG_COLOR_CONSTANT      0x0
#define GERECT_FLAG_COLOR_GRADIENT_X    0x1
#define GERECT_FLAG_COLOR_GRADIENT_Y    0x2
/// Define the info. of one block.
typedef struct
{
    /// dst block info
    GE_BLOCK       dstBlock;
    /// Color format
    GE_Buffer_Format fmt;
    union
    {
        /// Color
        ///For all RGB color, the color set as the ARGB8888 format.\n
        ///Each color component need to shift to high bit.\n
        ///Use ARGB1555 as the example, the source color key as the following:\n
        ///ARGB1555  --> ARRRRRGGGGGBBBBB                   (every character represents one bit)\n
        ///color_s   --> A0000000RRRRR000GGGGG000BBBBB000   (every character represents one bit)\n\n
        ///For GE_FMT_I8 format, the index set to b component (colorRange.color_s.b = b).\n
        GE_COLOR_RANGE       colorRange;
        ///Blink attribute
        ///For GE_FMT_1BAAFGBG123433 foramt, the foramt set as the GE_BLINK_DATA.\n
        GE_BLINK_DATA_RANGE  blkDataRange;
    };

    /// GERECT_FLAG_COLOR_CONSTANT: Constant color\n
    /// GERECT_FLAG_COLOR_GRADIENT_X: X direction gradient color\n
    /// GERECT_FLAG_COLOR_GRADIENT_Y: Y direction gradient color\n
    U32 flag;

} GE_RECT_FILL_INFO;

typedef struct
{
    /// dst block info
    GE_BLOCK       dstBlock;
    /// Color format
    GE_Buffer_Format fmt;
    union{
        GE_RGB_COLOR color;
        GE_BLINK_DATA blink_data;
    };
    U32 u32LineWidth;
} GE_OVAL_FILL_INFO;


//-------------------------------------------------
#define GEFONT_FLAG_DEFAULT                 0x0
#define GEFONT_FLAG_ITALIC                  0x1
#define GEFONT_FLAG_SCALE                   0x2
#define GEFONT_FLAG_COMPACT                 0x4
#define GEFONT_FLAG_BACKGROUND              0x8
#define GEFONT_FLAG_BLINK                   0x10
#define GEFONT_FLAG_VARWIDTH                0x20
#define GEFONT_FLAG_GAP                     0x40
/// Font source information
/*
typedef struct
{
    GE_Buffer_Format fmt;
    U32 u32Width;
    U32 u32Height;
    U32 u32Pitch;
    U32 u32Addr;
} GE_TEXT_SRC_INFO;
*/


/// Font format
typedef struct
{
    /// GEFONT_FLAG_DEFAULT: Default font format\n
    /// GEFONT_FLAG_SCALE: scalable font\n
    /// GEFONT_FLAG_COMPACT: Compact font\n
    /// GEFONT_FLAG_GAP: Gap font\n
    /// GEFONT_FLAG_VARIABLE_WIDTH: Variable font width, depend on horizontal font length.
    U8 flag;

    /// dst block first character position , and each character width , height
    GE_BLOCK  dstblk;

    /// set compact distant(take indent effect) when GEFONT_FLAG_COMPACT is set
    U8 dis;

} GE_TEXT_OUT_INFO;

typedef enum
{
    GEBLINK_NONE = 0x0,
    GEBLINK_BACKGROUND,
    GEBLINK_FOREGROUND,
    GEBLINK_BOTH,
} GE_BLINK_TYPE;


//-------------------------------------------------
typedef struct
{
    U8 flag;
    U32 delta_r;
    U32 delta_g;
    U32 delta_b;
} GEColorDelta;


//-------------------------------------------------
/// Specify the attribute of a bitmap
typedef struct
{
    /// Do scaling operation
    U16 bScale;
    /// X start address
    U16 x;
    /// Y start address
    U16 y;
    /// width
    U16 width;
    /// height
    U16 height;
} GE_DRAW_BMP_INFO;


//-------------------------------------------------
/// Define rotation angle
typedef enum
{
    /// Do not rotate
    GEROTATE_0   = 0,
    /// Rotate 90 degree
    GEROTATE_90  = 1,
    /// Rotate 180 degree
    GEROTATE_180 = 2,
    /// Rotate 270 degree
    GEROTATE_270 = 3
}GEROTATE_ANGLE;


//-------------------------------------------------
/// Define Virtual Command Buffer Size
typedef enum
{
    /// 4K
    GE_VCMD_4K = 0,
    /// 8K
    GE_VCMD_8K = 1,
    /// 16K
    GE_VCMD_16K = 2,
    /// 32K
    GE_VCMD_32K = 3,
    /// 64K
    GE_VCMD_64K = 4,
    /// 128K
    GE_VCMD_128K = 5,
    /// 256K
    GE_VCMD_256K = 6,
    /// 512K
    GE_VCMD_512K = 7,
}GEVCMD_BUF_SIZE;

//=============================================================================
// Font information
//=============================================================================
/// Bounding box (BBox) of a glyph
typedef struct
{
    ///Position of the left edge of the bounding box
    U8  u8X0;
    ///Width of the bounding box
    U8  u8Width;
    ///Position of the top edge of the bounding box
    U8  u8Y0;
    ///Height of the bounding box
    U8  u8Height;
}GE_GLYPH_BBOX;

/// define character information
typedef struct
{
    /// Character start address (128 bits alignment)
    U32 addr;
    /// Character width
    U32 width;
    /// Character pitch
    U32 pitch;
    /// Character height
    U32 height;
    /// Character horizontal offset
    U32 Hoffset;
    /// Character vertical offset
    U32 Voffset;
    /// Character format
    GE_Buffer_Format fmt;
} GE_CHAR_INFO;

/// Define font table information
typedef struct
{
    /// font start address
    U32 addr;
    /// font length
    U32 len;
    /// font structure is used
    U16 inUsed;
    /// character width (pixel unit)
    U32 width;
    /// character pitch (byte unit)
    U32 pitch;
    /// character height (pixel unit)
    U32 height;
    /// one character length (byte unit)
    U32 offset;
    /// Bounding Box offset
    GE_GLYPH_BBOX* pBBox;
    /// font format
    GE_Buffer_Format fmt;
} GE_FONT_INFO;

//=============================================================================
// GE palette information
//=============================================================================
/// Define the entry of palette.
typedef union
{
    /// ARGB8888
    struct
    {
        /// B8
        U32 u8B:        8;
        /// G8
        U32 u8G:        8;
        /// R8
        U32 u8R:        8;
        /// A8
        U32 u8A:        8;
    } ARGB;
    // 32-bit direct access.
    U32 u32Data;
} GePaletteEntry;

//=============================================================================
// Bitmap information
//=============================================================================
typedef struct
{
    U32 addr;       // flat address of whole memory map
    U32 len;
    U16 inUsed;
    U32 width;
    U32 pitch;
    U32 height;
    GE_Buffer_Format fmt;
} GE_BITMAP_INFO;


//=============================================================================
// Draw Rect info
//=============================================================================
/// Define the bitblt source & destination block.
typedef struct
{
    /// Source block
    GE_BLOCK srcblk;
    /// Destination block
    GE_BLOCK dstblk;
}GE_DRAW_RECT;


/// Define the scaling factor for X & Y direction.
typedef struct
{
    U32 u32DeltaX;
    U32 u32DeltaY;
    U32 u32InitDelatX;
    U32 u32InitDelatY;
}GE_SCALE_INFO;

//=============================================================================
// Data Buffer info
//=============================================================================
/// Data buffer info.
typedef struct
{
    /// start memory address
    U32 u32Addr;       // flat address of whole memory map
    /// width
    U32 u32Width;
    /// height
    U32 u32Height;
    /// pitch
    U32 u32Pitch;
    /// Color format\n
    /// - GE_FMT_I1\n
    /// - GE_FMT_I2\n
    /// - GE_FMT_I4\n
    /// - GE_FMT_I8\n
    /// - GE_FMT_RGB565\n
    /// - GE_FMT_ARGB1555\n
    /// - GE_FMT_ARGB4444\n
    /// - GE_FMT_1BAAFGBG123433\n
    /// - GE_FMT_ARGB8888\n
    GE_Buffer_Format u32ColorFmt;
} GE_BUFFER_INFO;

typedef GE_BUFFER_INFO* PGE_BUFFER_INFO;

typedef struct
{
    GE_BLOCK      srcRect;
    GE_Buffer_Format pixelFormat;    
    U8            alphaConst;
    U8            bAntiAliasing;
    U8            trapDir;
    U8            srcRectRotateDir;
    U8            dstTrapRotateDir;
    U16           dstTrapEdge0Pos;
    U16           dstTrapEdge0St;
    U16           dstTrapEdge0End;
    U16           dstTrapEdge1St;
    U16           dstTrapEdge1End;
    U16           dstTrapDistance;
    U8            *pSrcRotateTempBuf;
} GE_TRAPEZOID_INFO;

#define GEDRAW_FLAG_DEFAULT             0x0
#define GEDRAW_FLAG_SCALE               0x1
#define GEDRAW_FLAG_DUPLICAPE           0x2


#define FB_FMT_AS_DEFAULT               0xFFFF

#define GE_DMA_WIDTH                    32
#define GE_DMA_PITCH                    16

//=============================================================================
// YUV color setting
//=============================================================================
typedef U8                              GE_RGB2YUV_MODE;
#define GE_RGB2YUV_PC_MODE              ((GE_RGB2YUV_MODE)0x0)          // computer mode Y: 16~ 235, UV: 0~ 240
#define GE_RGB2YUV_255_MODE             ((GE_RGB2YUV_MODE)0x1)          // To 0~ 255

typedef U8                              GE_YUV_RANGE_OUT;
#define GE_YUV_RANGE_OUT_255            ((GE_YUV_RANGE_OUT)0x0)         // To 0~ 255
#define GE_YUV_RANGE_OUT_PC             ((GE_YUV_RANGE_OUT)0x1)         // To Y: 16~ 235

typedef U8                              GE_YUV_RANGE_IN;
#define GE_YUV_RANGE_IN_255             ((GE_YUV_RANGE_IN)0x0)          // UV input range, 0~ 255
#define GE_YUV_RANGE_IN_127             ((GE_YUV_RANGE_IN)0x1)          // UV input range, -128~ 127

typedef U8                              GE_YUV_422_FMT;
#define GE_YUV_422_FMT_UY1VY0           ((GE_YUV_422_FMT)0x00)          // CbY1CrY0
#define GE_YUV_422_FMT_VY1UY0           ((GE_YUV_422_FMT)0x01)          // CrY1CbY0
#define GE_YUV_422_FMT_Y1UY0V           ((GE_YUV_422_FMT)0x02)          // Y1CbY0Cr
#define GE_YUV_422_FMT_Y1VY0U           ((GE_YUV_422_FMT)0x03)          // Y1CrY0Cb

typedef struct
{
    GE_RGB2YUV_MODE                     rgb2yuv_mode;
    GE_YUV_RANGE_OUT                    yuv_range_out;
    GE_YUV_RANGE_IN                     yuv_range_in;
    GE_YUV_422_FMT                      yuv_mem_fmt_src;
    GE_YUV_422_FMT                      yuv_mem_fmt_dst;
} GE_YUV_INFO;


//=============================================================================
// GE Driver Function
//=============================================================================
extern void MHal_GE_Init(void);
extern void MHal_GE_PowerOff(void);
extern void MHal_GE_Set_Power_Off(U8 flag);
extern GESTATUS MHal_GE_ScreenCopy(GE_BLOCK *psrcblk, GE_BLOCK *pdstblk);
extern GESTATUS MHal_GE_DrawBitmap(BMPHANDLE handle, GE_DRAW_BMP_INFO *pbmpfmt);
extern GESTATUS MHal_GE_GetFontInfo(FONTHANDLE handle, GE_FONT_INFO* pinfo);
extern GESTATUS MHal_GE_GetBitmapInfo(BMPHANDLE handle, GE_BITMAP_INFO* pinfo);
extern GESTATUS MHal_GE_GetFrameBufferInfo(U32 *width, U32 *height, U32 *pitch, U32 *fbFmt, U32 *addr);
//extern void MHal_GE_Init(U32 width, U32 height, U32 pitch, U32 fbFmt);
extern GESTATUS MHal_GE_DrawLine(GE_DRAW_LINE_INFO *pline);
extern GESTATUS MHal_GE_DrawOval(GE_OVAL_FILL_INFO* pOval);
extern BMPHANDLE  MHal_GE_LoadBitmap(U32 addr, U32 len, U32 width, U32 height, GE_Buffer_Format fmt);
extern FONTHANDLE MHal_GE_LoadFont(U32 addr, U32 len, U32 width, U32 height, GE_GLYPH_BBOX* pBBox, GE_Buffer_Format fmt);
extern GESTATUS MHal_GE_FreeBitmap(BMPHANDLE handle);
extern GESTATUS MHal_GE_FreeFont(FONTHANDLE handle);
extern GESTATUS MHal_GE_RectFill(GE_RECT_FILL_INFO *pfillblock);
extern GESTATUS MHal_GE_SetClip(GE_POINT_t* v0, GE_POINT_t* v1);
extern GESTATUS MHal_GE_SetItalic(U16 enable,U8 ini_line, U8 ini_dis, U8 delta);
extern GESTATUS MHal_GE_SetDither(U16 enable);
extern GESTATUS MHal_GE_SetSrcBufferInfo(PGE_BUFFER_INFO bufInfo, U32 offsetofByte);
extern GESTATUS MHal_GE_SetDstBufferInfo(PGE_BUFFER_INFO bufInfo, U32 offsetofByte);
extern GESTATUS MHal_GE_SetNearestMode(U16 enable);
extern GESTATUS MHal_GE_SetMirror(U16 isMirrorX, U16 isMirrorY);
extern GESTATUS MHal_GE_SetROP2(U16 enable, GE_ROP2_OP eRopMode);
extern GESTATUS MHal_GE_SetROP8(U16 enable, GE_ROP8_OP_t eRopMode, U8 u8ConstAlpha);
extern GESTATUS MHal_GE_SetFramePtr(U32 gopidx,U32 gwinidx,U32 new_addr ) ;
extern GESTATUS MHal_GE_SetRotate(GEROTATE_ANGLE angle);
extern GESTATUS MHal_GE_SetSrcColorKey(U16 enable, GE_COLOR_KEY_MODE opMode,
                                      GE_Buffer_Format fmt, void *ps_color,
                                      void *pe_color);
extern GESTATUS MHal_GE_SetDstColorKey(U16 enable, GE_COLOR_KEY_MODE opMode,
                                      GE_Buffer_Format fmt, void *ps_color,
                                      void *pe_color);
extern GESTATUS MHal_GE_TextOut(FONTHANDLE fhandle, U8 *pindex, U32 strwidth, GE_TEXT_OUT_INFO *pfmt);
extern GESTATUS MHal_GE_QueryTextDispLength(FONTHANDLE fhandle, U8 *pu8index, U32 u32strwidth,
                                               GE_TEXT_OUT_INFO *pfmt, U32* pu32DispLength);
extern GESTATUS MHal_GE_CharacterOut(GE_CHAR_INFO*  pChar, GE_TEXT_OUT_INFO *pfmt);
extern GESTATUS MHal_GE_SetAlphaSrcFrom(GE_ALPHA_SRC_FROM eMode);
extern GESTATUS MHal_GE_SetAlphaBlending(GE_BLEND_COEF blendcoef, U8 blendfactor);
extern GESTATUS MHal_GE_EnableAlphaBlending(U16 enable);
extern GESTATUS MHal_GE_Line_Pattern_Reset(void);
extern GESTATUS MHal_GE_Set_Line_Pattern(U16 enable, U8 linePattern, U8 repeatFactor);
extern GESTATUS MHal_GE_BitBlt(GE_DRAW_RECT *drawbuf,U32 drawflag);
extern GESTATUS MHal_GE_BitBltEx(GE_DRAW_RECT * drawbuf, U32 drawflag, GE_SCALE_INFO * ScaleInfo);
extern GESTATUS MHal_GE_TrapezoidBitBlt(GE_TRAPEZOID_INFO *pGETrapezoidInfo,U32 drawflag);
extern GESTATUS MHal_GE_GetScaleBltInfo(GE_DRAW_RECT* pdrawbuf, GE_BLOCK* pSrcBlk,  GE_BLOCK* pDstBlk, GE_SCALE_INFO * pScaleInfo);
extern GESTATUS MHal_GE_SetIntensity(U32 id, GE_Buffer_Format fmt, U32 *pColor);
extern GESTATUS MHal_GE_DeleteBuffer(PGE_BUFFER_INFO bufinfo);
extern GESTATUS MHal_GE_BeginDraw(void);
extern GESTATUS MHal_GE_EndDraw(void);
extern GESTATUS MHal_GE_YUV_Set(GE_YUV_INFO* pYuvInfo);
extern GESTATUS MHal_GE_YUV_Get(GE_YUV_INFO* pYuvInfo);
extern GESTATUS MHal_GE_InitBufferInfo(PGE_BUFFER_INFO pBufInfo, U32 addr, U32 width, U32 height, GE_Buffer_Format fbFmt);
extern GESTATUS MHal_GE_EnableVCmdQueue(U16 blEnable);
extern GESTATUS MHal_GE_SetVCmdBuffer(U32 u32Addr, GEVCMD_BUF_SIZE enBufSize);
extern GESTATUS MHal_GE_SetVCmd_W_Thread(U8 u8W_Threshold);
extern GESTATUS MHal_GE_SetVCmd_R_Thread(U8 u8R_Threshold);
extern GESTATUS MHal_GE_EnableDSTAC(U16 enable);
extern GESTATUS MHal_GE_Set_DSTAC_Mode(U16 blOpWithinThreshold);
extern GESTATUS MHal_GE_Set_DSTAC_Thread(U8 u8LoThread, U8 u8HiThread);
extern GESTATUS MHal_GE_Palette_Set(GePaletteEntry * pPalArray, U32 u32PalStart, U32 u32PalEnd);
extern GESTATUS MHal_GE_WaitGEFinish(U8 u8CMDCount);

#ifndef RED_LION
extern PGE_BUFFER_INFO MHal_GE_CreateBuffer(U32 addr, U32 width, U32 height, GE_Buffer_Format fbFmt);
#endif

#endif

