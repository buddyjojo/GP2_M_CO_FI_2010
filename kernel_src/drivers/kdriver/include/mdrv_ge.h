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
///
/// file    mdrv_ge.h
/// @author MStar Semiconductor Inc.
/// @brief  GE Device Driver
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __MDEV_GE_H__
#define __MDEV_GE_H__


#ifndef MDRV_GE_NR_DEVS
#define MDRV_GE_NR_DEVS 1
#endif


// These type define can be removed after the coding style is confirmed.
#include <asm-mips/types.h>
#define TRUE 1
#define FALSE 0



///////////// The following code is from madp_ge.h  /////////////////////


/////////// The above lines are moved from madp_ge.h //////////


typedef struct
{
    GE_BLOCK *pSrcBlk;
    GE_BLOCK *pDstBlk;
} MS_GE_SCREEN_COPY, *PMS_GE_SCREEN_COPY;

typedef struct
{
    BMPHANDLE Bmp_Handle;
    GE_DRAW_BMP_INFO *pDrawBmpInfo;
} MS_GE_DRAW_BITMAP, *PMS_GE_DRAW_BITMAP;

typedef struct
{
    FONTHANDLE Font_Handle;
    GE_FONT_INFO *pFontInfo;
} MS_GE_GET_FONTINFO, *PMS_GE_GET_FONTINFO;

typedef struct
{
    BMPHANDLE Bmp_Handle;
    GE_BITMAP_INFO* pBmpInfo;
} MS_GE_GET_BITMAPINFO, *PMS_GE_GET_BITMAPINFO;

typedef struct
{
    U32 u32Width;
    U32 u32Height;
    U32 u32Pitch;
    U32 u32FbFmt;
    U32 u32Addr;
} MS_GE_GET_FRAMEBUFFERINFO, *PMS_GE_GET_FRAMEBUFFERINFO;

typedef struct
{
    U32 u32Addr;
    U32 u32Len;
    U32 u32Width;
    U32 u32Height;
    GE_Buffer_Format enBufferFmt;
    BMPHANDLE hBmpHandle;
} MS_GE_LOAD_BITMAP, *PMS_GE_LOAD_BITMAP;

typedef struct
{
    U32 u32Addr;
    U32 u32Len;
    U32 u32Width;
    U32 u32Height;
    GE_GLYPH_BBOX* pGlyphBBox;
    GE_Buffer_Format enBufferFmt;
    FONTHANDLE hFontHandle;
} MS_GE_LOAD_FONT, *PMS_GE_LOAD_FONT;


typedef struct
{
    GE_POINT_t* pPoint0;
    GE_POINT_t* pPoint1;
} MS_GE_SET_CLIP, *PMS_GE_SET_CLIP;


typedef struct
{
    U16 blEnable;
    U8 u8Init_line;
    U8 u8Init_dis;
    U8 u8Delta;
} MS_GE_SET_ITALIC, *PMS_GE_SET_ITALIC;


typedef struct
{
    PGE_BUFFER_INFO pBufferInfo;
    U32 u32OffsetofByte;
} MS_GE_SET_BUFFERINFO, *PMS_GE_SET_BUFFERINFO;


typedef struct
{
    U16 blIsMirrorX;
    U16 blIsMirrorY;
} MS_GE_SET_MIRROR, *PMS_GE_SET_MIRROR;


typedef struct
{
    U16 blEnable;
    GE_ROP2_OP eRopMode;
} MS_GE_SET_ROP2, *PMS_GE_SET_ROP2;



typedef struct
{
    U16 blEnable;
    GE_COLOR_KEY_MODE eOPMode;
    GE_Buffer_Format enBufferFmt;
    U32 *pu32PS_Color;
    U32 *pu32GE_Color;
} MS_GE_SET_COLORKEY, *PMS_GE_SET_COLORKEY;


typedef struct
{
    FONTHANDLE Font_Handle;
    U8 *pu8Index;
    U32 u32StrWidth;
    U16 u16CharNum;
    GE_TEXT_OUT_INFO *pTextOutInfo;
} MS_GE_TEXTOUT, *PMS_GE_TEXTOUT;


typedef struct
{
    FONTHANDLE Font_Handle;
    U8 *pu8Index;
    U32 u32StrWidth;
    GE_TEXT_OUT_INFO *pTextOutInfo;
    U32* pu32DispLength;
} MS_GE_QUERY_TEXT_DISPLENGTH, *PMS_GE_QUERY_TEXT_DISPLENGTH;


typedef struct
{
    GE_CHAR_INFO*  pGECharInfo;
    GE_TEXT_OUT_INFO *pGETextOutInfo;
} MS_GE_CHARACTEROUT, *PMS_GE_CHARACTEROUT;


typedef struct
{
    GE_BLEND_COEF enBlendCoef;
    U8 u8BlendFactor;
} MS_GE_SET_ALPHABLENDING, *PMS_GE_SET_ALPHABLENDING;


typedef struct
{
    U16 blEnable;
    U8 u8LinePattern;
    U8 u8RepeatFactor;
} MS_GE_SET_LINEPATTERN, *PMS_GE_SET_LINEPATTERN;


typedef struct
{
    GE_DRAW_RECT *pGEDrawRect;
    U32 u32DrawFlag;
} MS_GE_SET_BITBLT, *PMS_GE_SET_BITBLT;


typedef struct
{
    GE_DRAW_RECT *pDrawRect;
    U32 u32DrawFlag;
    GE_SCALE_INFO *pScaleInfo;
} MS_GE_SET_BITBLTEX, *PMS_GE_SET_BITBLTEX;


typedef struct
{
    GE_TRAPEZOID_INFO *pGETrapezoidInfo;
    U32 u32DrawFlag;
} MS_GE_SET_TRAPEZOID_BITBLT, *PMS_GE_SET_TRAPEZOID_BITBLT;


typedef struct
{
    GE_DRAW_RECT* pDrawRect;
    GE_BLOCK* pSrcBlk;
    GE_BLOCK* pDstBlk;
    GE_SCALE_INFO * pScaleInfo;
} MS_GE_GET_SCALEBLTINFO, *PMS_GE_GET_SCALEBLTINFO;


typedef struct
{
    U32 u32ID;
    GE_Buffer_Format enBufferFmt;
    U32 *pu32Color;
} MS_GE_SET_INTENSITY, *PMS_GE_SET_INTENSITY;


typedef struct
{
    U32 u32Addr;
    U32 u32Width;
    U32 u32Height;
    GE_Buffer_Format enBufferFmt;
}MS_GE_CREATE_BUFFER, *PMS_GE_CREATE_BUFFER;


typedef struct
{
    PGE_BUFFER_INFO pBufInfo;
    U32 u32Addr;
    U32 u32Width;
    U32 u32Height;
    GE_Buffer_Format enBufferFmt;
} MS_GE_INIT_BUFFERINFO, *PMS_GE_INIT_BUFFERINFO;

typedef struct
{
    U32 u32Addr;
    GEVCMD_BUF_SIZE enBufSize;
} MS_GE_SET_VCMD_BUFFER, *PMS_GE_SET_VCMD_BUFFER;


typedef struct
{
    U8 u8LoThread;
    U8 u8HiThread;
} MS_GE_SET_DSTAC_THREAD, *PMS_GE_SET_DSTAC_THREAD;


typedef struct GE_ROP8_OP_t
{
    U16 blEnable;
    GE_ROP8_OP_t eRopMode;
    U8 u8ConstAlpha;
} MS_GE_SET_ROP8, *PMS_GE_SET_ROP8;

typedef struct
{
    U8 u8CMDCount;
    U8 result;
} MS_GE_WAIT_GE, *PMS_GE_WAIT_GE;

//------------------------------------------------------------------------------
// external function
//------------------------------------------------------------------------------


#define MDRV_GE_IOC_MAGIC  '1'

#define MDRV_GE_IOC_INIT             _IO(MDRV_GE_IOC_MAGIC,  0)
#define MDRV_GE_IOC_POWER_OFF        _IO(MDRV_GE_IOC_MAGIC,  1)
#define MDRV_GE_IOC_SCREEN_COPY      _IOW(MDRV_GE_IOC_MAGIC,  2, MS_GE_SCREEN_COPY)
#define MDRV_GE_IOC_DRAW_BITMAP      _IOW(MDRV_GE_IOC_MAGIC,  3, MS_GE_DRAW_BITMAP)
#define MDRV_GE_IOC_GET_FONTINFO     _IOR(MDRV_GE_IOC_MAGIC,  4, MS_GE_GET_FONTINFO)
#define MDRV_GE_IOC_GET_BITMAPINFO   _IOR(MDRV_GE_IOC_MAGIC,  5, MS_GE_GET_BITMAPINFO)
#define MDRV_GE_IOC_GET_FRAMEBUFFERINFO   _IOR(MDRV_GE_IOC_MAGIC,  6, MS_GE_GET_FRAMEBUFFERINFO)
#define MDRV_GE_IOC_DRAW_LINE        _IOW(MDRV_GE_IOC_MAGIC,  7, GE_DRAW_LINE_INFO)
#define MDRV_GE_IOC_DRAW_OVAL        _IOW(MDRV_GE_IOC_MAGIC,  8, GE_OVAL_FILL_INFO)
#define MDRV_GE_IOC_LOAD_BITMAP      _IOW(MDRV_GE_IOC_MAGIC,  9, MS_GE_LOAD_BITMAP)
#define MDRV_GE_IOC_LOAD_FONT        _IOW(MDRV_GE_IOC_MAGIC,  10, MS_GE_LOAD_FONT)
#define MDRV_GE_IOC_FREE_BITMAP      _IOW(MDRV_GE_IOC_MAGIC,  11, BMPHANDLE)
#define MDRV_GE_IOC_FREE_FONT        _IOW(MDRV_GE_IOC_MAGIC,  12, FONTHANDLE)
#define MDRV_GE_IOC_RECT_FILL        _IOW(MDRV_GE_IOC_MAGIC,  13, GE_RECT_FILL_INFO)
#define MDRV_GE_IOC_SET_CLIP         _IOW(MDRV_GE_IOC_MAGIC,  14, MS_GE_SET_CLIP)
#define MDRV_GE_IOC_SET_ITALIC       _IOW(MDRV_GE_IOC_MAGIC,  15, MS_GE_SET_ITALIC)
#define MDRV_GE_IOC_SET_DITHER       _IOW(MDRV_GE_IOC_MAGIC,  16, U16)
#define MDRV_GE_IOC_SET_SRCBUFFERINO _IOW(MDRV_GE_IOC_MAGIC,  17, MS_GE_SET_BUFFERINFO)
#define MDRV_GE_IOC_SET_DSTBUFFERINO _IOW(MDRV_GE_IOC_MAGIC,  18, MS_GE_SET_BUFFERINFO)
#define MDRV_GE_IOC_SET_NEARESTMODE  _IOW(MDRV_GE_IOC_MAGIC,  19, U16)
#define MDRV_GE_IOC_SET_MIRROR       _IOW(MDRV_GE_IOC_MAGIC,  20, MS_GE_SET_MIRROR)
#define MDRV_GE_IOC_SET_ROP2         _IOW(MDRV_GE_IOC_MAGIC,  21, MS_GE_SET_ROP2)
#define MDRV_GE_IOC_SET_ROTATE       _IOW(MDRV_GE_IOC_MAGIC,  22, GEROTATE_ANGLE)
#define MDRV_GE_IOC_SET_SRCCOLORKEY  _IOW(MDRV_GE_IOC_MAGIC,  23, MS_GE_SET_COLORKEY)
#define MDRV_GE_IOC_SET_DSTCOLORKEY  _IOW(MDRV_GE_IOC_MAGIC,  24, MS_GE_SET_COLORKEY)
#define MDRV_GE_IOC_TEXTOUT          _IOW(MDRV_GE_IOC_MAGIC,  25, MS_GE_TEXTOUT)
#define MDRV_GE_IOC_QUERY_TEXT_DISPLENGTH  _IOW(MDRV_GE_IOC_MAGIC,  26, MS_GE_QUERY_TEXT_DISPLENGTH)
#define MDRV_GE_IOC_CHARACTEROUT     _IOW(MDRV_GE_IOC_MAGIC,  27, MS_GE_CHARACTEROUT)
#define MDRV_GE_IOC_SET_ALPHASRCFROM _IOW(MDRV_GE_IOC_MAGIC,  28, GE_ALPHA_SRC_FROM)
#define MDRV_GE_IOC_SET_ALPHABLENDING _IOW(MDRV_GE_IOC_MAGIC,  29, MS_GE_SET_ALPHABLENDING)
#define MDRV_GE_IOC_ENABLE_ALPHABLENDING _IOW(MDRV_GE_IOC_MAGIC,  30, U16)
#define MDRV_GE_IOC_LINEPATTERN_RESET _IO(MDRV_GE_IOC_MAGIC,  31)
#define MDRV_GE_IOC_SET_LINEPATTERN  _IOW(MDRV_GE_IOC_MAGIC,  32, MS_GE_SET_LINEPATTERN)
#define MDRV_GE_IOC_BITBLT           _IOW(MDRV_GE_IOC_MAGIC,  33, MS_GE_SET_BITBLT)
#define MDRV_GE_IOC_BITBLTEX         _IOW(MDRV_GE_IOC_MAGIC,  34, MS_GE_SET_BITBLTEX)
#define MDRV_GE_IOC_Get_SCALEBLTINFO _IOR(MDRV_GE_IOC_MAGIC,  35, MS_GE_GET_SCALEBLTINFO)
#define MDRV_GE_IOC_SET_INTENSITY    _IOW(MDRV_GE_IOC_MAGIC,  36, MS_GE_SET_INTENSITY)
#define MDRV_GE_IOC_CREATE_BUFFER    _IOW(MDRV_GE_IOC_MAGIC,  37, MS_GE_CREATE_BUFFER)
#define MDRV_GE_IOC_DELETE_BUFFER    _IOW(MDRV_GE_IOC_MAGIC,  38, PGE_BUFFER_INFO)
#define MDRV_GE_IOC_BEGIN_DRAW       _IO(MDRV_GE_IOC_MAGIC,   39)
#define MDRV_GE_IOC_END_DRAW         _IO(MDRV_GE_IOC_MAGIC,   40)
#define MDRV_GE_IOC_SET_YUV          _IOW(MDRV_GE_IOC_MAGIC,  41, GE_YUV_INFO)
#define MDRV_GE_IOC_GET_YUV          _IOR(MDRV_GE_IOC_MAGIC,  42, GE_YUV_INFO)
#define MDRV_GE_IOC_INIT_BUFFERINFO  _IOW(MDRV_GE_IOC_MAGIC,  43, MS_GE_INIT_BUFFERINFO)
#define MDRV_GE_IOC_ENABLE_VCMDQ     _IOW(MDRV_GE_IOC_MAGIC,  44, U16)
#define MDRV_GE_IOC_SET_VCMD_BUF     _IOW(MDRV_GE_IOC_MAGIC,  45, MS_GE_SET_VCMD_BUFFER)
#define MDRV_GE_IOC_SET_VCMD_W_TH    _IOW(MDRV_GE_IOC_MAGIC,  46, U8)
#define MDRV_GE_IOC_SET_VCMD_R_TH    _IOW(MDRV_GE_IOC_MAGIC,  47, U8)
#define MDRV_GE_IOC_ENABLE_DSTAC     _IOW(MDRV_GE_IOC_MAGIC,  48, U16)
#define MDRV_GE_IOC_SET_DSTAC_MODE   _IOW(MDRV_GE_IOC_MAGIC,  49, U16)
#define MDRV_GE_IOC_SET_DSTAC_TH     _IOW(MDRV_GE_IOC_MAGIC,  50, MS_GE_SET_DSTAC_THREAD)

#define MDRV_GE_IOC_SET_ROP8         _IOW(MDRV_GE_IOC_MAGIC,  51, MS_GE_SET_ROP8)

#define MDRV_GE_IOC_SET_FRAME_PTR    _IOW(MDRV_GE_IOC_MAGIC,  52, U32)

///// LGE : 20080517 //////////
#define MDRV_GE_IOC_SET_PALETTE      _IOW(MDRV_GE_IOC_MAGIC,  53, U32)
///////////////////////////////

#define MDRV_GE_IOC_TRAPEZOID_BITBLT _IOW(MDRV_GE_IOC_MAGIC,  54, MS_GE_SET_TRAPEZOID_BITBLT)

#define MDRV_GE_IOC_WAIT_GE_FINISH   _IOR(MDRV_GE_IOC_MAGIC,  55, MS_GE_WAIT_GE)

#define MDRV_GE_IOC_SET_POWER_OFF    _IOW(MDRV_GE_IOC_MAGIC,  56, U8)

#define MDRV_GE_IOC_MAXNR  57//44     // The number should be as same as the number of IOCTRL command

#endif  // #ifndef _MDRV_GE_H_
