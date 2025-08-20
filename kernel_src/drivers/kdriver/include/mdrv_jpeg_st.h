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

#ifndef _DRV_JPEG_ST_H_
#define _DRV_JPEG_ST_H_

#include "mdrv_types.h"

#define QUANT_TYPE S16
#define BLOCK_TYPE S16

//------------------------------------------------------------------------------
// May need to be adjusted if support for other colorspaces/sampling factors is added
#define JPEG_MAXBLOCKSPERMCU 10
//------------------------------------------------------------------------------
#define JPEG_MAXHUFFTABLES   8
#define JPEG_MAXQUANTTABLES  4
#define JPEG_MAXCOMPONENTS   4
#define JPEG_MAXCOMPSINSCAN  4
//------------------------------------------------------------------------------
// Increase this if you increase the max width!
#define JPEG_MAXBLOCKSPERROW 12288//6144
//------------------------------------------------------------------------------
// Max. allocated blocks
#ifdef CMODEL
#define JPEG_MAXBLOCKS    100
#else
#define JPEG_MAXBLOCKS    50
#endif
//------------------------------------------------------------------------------
#define JPEG_MIN_HEIGHT 64
#define JPEG_MIN_WIDTH  64

//JPD only support 11 bit (unit is 8 sampler) horizontal and vertical size
#define JPEG_MAX_HEIGHT 16360  // (2^12-1)*8 - 16 = 16360
#define JPEG_MAX_WIDTH  16360  // (2^12-1)*8 - 16 = 16360
//------------------------------------------------------------------------------
/* JPEG specific errors */
#define JPEG_BAD_DHT_COUNTS              -200
#define JPEG_BAD_DHT_INDEX               -201
#define JPEG_BAD_DHT_MARKER              -202
#define JPEG_BAD_DQT_MARKER              -203
#define JPEG_BAD_DQT_TABLE               -204
#define JPEG_BAD_PRECISION               -205
#define JPEG_BAD_HEIGHT                  -206
#define JPEG_BAD_WIDTH                   -207
#define JPEG_TOO_MANY_COMPONENTS         -208
#define JPEG_BAD_SOF_LENGTH              -209
#define JPEG_BAD_VARIABLE_MARKER         -210
#define JPEG_BAD_DRI_LENGTH              -211
#define JPEG_BAD_SOS_LENGTH              -212
#define JPEG_BAD_SOS_COMP_ID             -213
#define JPEG_W_EXTRA_BYTES_BEFORE_MARKER -214
#define JPEG_NO_ARITHMETIC_SUPPORT       -215
#define JPEG_UNEXPECTED_MARKER           -216
#define JPEG_NOT_JPEG                    -217
#define JPEG_UNSUPPORTED_MARKER          -218
#define JPEG_BAD_DQT_LENGTH              -219
#define JPEG_TOO_MANY_BLOCKS             -221
#define JPEG_UNDEFINED_QUANT_TABLE       -222
#define JPEG_UNDEFINED_HUFF_TABLE        -223
#define JPEG_NOT_SINGLE_SCAN             -224
#define JPEG_UNSUPPORTED_COLORSPACE      -225
#define JPEG_UNSUPPORTED_SAMP_FACTORS    -226
#define JPEG_DECODE_ERROR                -227
#define JPEG_BAD_RESTART_MARKER          -228
#define JPEG_ASSERTION_ERROR             -229
#define JPEG_BAD_SOS_SPECTRAL            -230
#define JPEG_BAD_SOS_SUCCESSIVE          -231
#define JPEG_STREAM_READ                 -232
#define JPEG_NOTENOUGHMEM                -233
#define JPEG_STOP_DECODE                 -234 //kevinhuang, add
#define JPEG_BAD_APP1_MARKER             -235
#define JPEG_NO_THUMBNAIL                -236
#define JPEG_SVLD_DECODE_ERROR           -237
#define JPEG_READBUFFER_TOOSMALL         -238
//------------------------------------------------------------------------------
// SCAN TYPE
#define JPEG_GRAYSCALE 0
#define JPEG_YH1V1     1
#define JPEG_YH2V1     2
#define JPEG_YH1V2     3
#define JPEG_YH2V2     4
#define JPEG_YH4V1     5
//------------------------------------------------------------------------------
// JPEG DECODE RESULT
#define JPEG_FAILED -1
#define JPEG_DONE       1
#define JPEG_OKAY       0
//------------------------------------------------------------------------------
// BUFFER LOADING TYPE
#define JPEG_BUFFER_HIGH  1
#define JPEG_BUFFER_LOW   2
//------------------------------------------------------------------------------
// JPEG DECODE TYPE
#define JPEG_TYPE_MAIN          0
#define JPEG_TYPE_THUMBNAIL     1
#define JPEG_TYPE_MJPEG         2
//------------------------------------------------------------------------------
typedef enum
{
    M_SOF0 = 0xC0,
    M_SOF1 = 0xC1,
    M_SOF2 = 0xC2,
    M_SOF3 = 0xC3,
    M_SOF5 = 0xC5,
    M_SOF6 = 0xC6,
    M_SOF7 = 0xC7,
    M_JPG = 0xC8,
    M_SOF9 = 0xC9,
    M_SOF10 = 0xCA,
    M_SOF11 = 0xCB,
    M_SOF13 = 0xCD,
    M_SOF14 = 0xCE,
    M_SOF15 = 0xCF,
    M_DHT = 0xC4,
    M_DAC = 0xCC,
    M_RST0 = 0xD0,
    M_RST1 = 0xD1,
    M_RST2 = 0xD2,
    M_RST3 = 0xD3,
    M_RST4 = 0xD4,
    M_RST5 = 0xD5,
    M_RST6 = 0xD6,
    M_RST7 = 0xD7,
    M_SOI = 0xD8,
    M_EOI = 0xD9,
    M_SOS = 0xDA,
    M_DQT = 0xDB,
    M_DNL = 0xDC,
    M_DRI = 0xDD,
    M_DHP = 0xDE,
    M_EXP = 0xDF,
    M_APP0 = 0xE0,
    M_APP1 = 0xE1,
    M_APP15 = 0xEF,
    M_JPG0 = 0xF0,
    M_JPG13 = 0xFD,
    M_COM = 0xFE,
    M_TEM = 0x01,
    M_ERROR = 0x100
} EN_JPEG_MARKER;

typedef enum
{
    EN_JPD_DECODING,
    EN_JPD_DECODE_DONE,
    EN_JPD_DECODE_ERROR,
    EN_JPD_MRBFH_DONE,
    EN_JPD_MRBFL_DONE
} EN_JPD_STATUS;

typedef enum
{
    EN_JPD_MRBF_L=0,
    EN_JPD_MRBF_H=1
}EN_JPD_MRBF_SELECT;

typedef struct
{
    U32 u32ReadBufferAddr;
    U32 u32ReadBufferSize;
    U32 u32WriteBufferAddr;
    U32 u32InterBufferAddr;
    U32 u32InterBufferSize;
    U32 v2p_offset;
    U8  u8FileReadEnd;
} JPEG_Buffer_Info;

typedef struct
{
    U32 u32OriginalWidth;
    U32 u32OriginalHeight;
    U32 u32Width;
    U32 u32Height;
    U8  u8BitPP;
    U8  u8Progressive;
    EN_JPD_STATUS enJPDStatus;
} JPEG_PIC_Info;

typedef struct
{
    S32 progressive_width;
    S32 progressive_height;
    S32 baseline_width;
    S32 baseline_height;
} JPEG_SIZE;

#endif
