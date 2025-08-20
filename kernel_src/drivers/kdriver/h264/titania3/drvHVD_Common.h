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
/// @file   drvHVD_Common.h
/// @brief  MStar General Data Types
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRVHVD_COMMON_H_
#define _DRVHVD_COMMON_H_
#if !(defined(REDLION_LINUX_KERNEL_ENVI))
#include "MsCommon.h"
#include "MsVersion.h"
#else //defined(REDLION_LINUX_KERNEL_ENVI)
//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------

#if !(defined( __MDRV_TYPES_H__ ))
/// data type unsigned char, data length 1 byte
typedef unsigned char               MS_U8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              MS_U16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned long               MS_U32;                             // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long          MS_U64;                             // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;                              // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;                             // 2 bytes
/// data type signed int, data length 4 byte
typedef signed long                 MS_S32;                             // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long            MS_S64;                             // 8 bytes

/// data type float, data length 4 byte
typedef float                       MS_FLOAT;                           // 4 bytes

/// definition for MS_BOOL
typedef unsigned char               MS_BOOL;
/// definition for VOID
typedef void                        VOID;
/// definition for FILEID
typedef MS_S32                      FILEID;

#endif

typedef unsigned long               MS_PHYADDR;                         // 32bit physical address


/// data type null pointer
#ifdef NULL
#undef NULL
#endif
#define NULL                        0

/// data type hardware physical address


//-------------------------------------------------------------------------------------------------
//  Software Data Type
//-------------------------------------------------------------------------------------------------


//[TODO] use MS_U8, ... instead
// data type for 8051 code
//typedef MS_U16                      WORD;
//typedef MS_U8                       BYTE;


#ifndef true
/// definition for true
#define true                        1
/// definition for false
#define false                       0
#endif


#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                        1
/// definition for FALSE
#define FALSE                       0
#endif

///Define MS FB Format, to share with GE,GOP
/// FIXME THE NAME NEED TO BE REFINED, AND MUST REMOVE UNNESSARY FMT
typedef enum
{
    /// color format I1
    E_MS_FMT_I1                     = 0x0,
    /// color format I2
    E_MS_FMT_I2                     = 0x1,
    /// color format I4
    E_MS_FMT_I4                     = 0x2,
    /// color format palette 256(I8)
    E_MS_FMT_I8                     = 0x4,
    /// color format blinking display
    E_MS_FMT_FaBaFgBg2266  = 0x6,
    /// color format for blinking display format
    E_MS_FMT_1ABFgBg12355           = 0x7,
    /// color format RGB565
    E_MS_FMT_RGB565                 = 0x8,
    /// color format ARGB1555
    /// @note <b>[URANUS] <em>ARGB1555 is only RGB555</em></b>
    E_MS_FMT_ARGB1555               = 0x9,
    /// color format ARGB4444
    E_MS_FMT_ARGB4444               = 0xa,
    /// color format ARGB1555 DST
    E_MS_FMT_ARGB1555_DST           = 0xc,
    /// color format YUV422
    E_MS_FMT_YUV422                 = 0xe,
    /// color format ARGB8888
    E_MS_FMT_ARGB8888               = 0xf,

    E_MS_FMT_GENERIC                = 0xFFFF,

} MS_ColorFormat;


typedef union _MSIF_Version
{
    struct _DDI
    {
        MS_U8                       tag[4];
        MS_U8                       class[2];
        MS_U16                      customer;
        MS_U16                      model;
        MS_U16                      chip;
        MS_U8                       cpu;
        MS_U8                       name[4];
        MS_U8                       version[2];
        MS_U8                       build[2];
        MS_U8                       change[8];
        MS_U8                       os;
    } DDI;

    struct _MW
    {
    } MW;

    struct _APP
    {
    } APP;

} MSIF_Version;

#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif

#ifndef BITS
    #ifdef _BIT
        #define BITS(bits,value)            ((_BIT(((1)?bits)+1)-_BIT(((0)?bits))) & (value<<((0)?bits)))
    #elif defined(BIT)
        #define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
    #endif
#endif

#ifndef BMASK
    #ifdef _BIT
        #define BMASK(bits)                 (_BIT(((1)?bits)+1)-_BIT(((0)?bits)))
    #elif defined(BIT)
        #define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))
    #endif
#endif

#define READ_BYTE(_reg)             (*(volatile MS_U8*)(_reg))
#define READ_WORD(_reg)             (*(volatile MS_U16*)(_reg))
#define READ_LONG(_reg)             (*(volatile MS_U32*)(_reg))
#define WRITE_BYTE(_reg, _val)      { (*((volatile MS_U8*)(_reg))) = (MS_U8)(_val); }
#define WRITE_WORD(_reg, _val)      { (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }
#define WRITE_LONG(_reg, _val)      { (*((volatile MS_U32*)(_reg))) = (MS_U32)(_val); }

#define MSIF_TAG                    {'M','S','I','F'}                   // MSIF
#define MSIF_CLASS                  {'0','0'}                           // DRV/API (DDI)
#define MSIF_CUS                    0x0000                              // MStar Common library
#define MSIF_MOD                    0x0000                              // MStar Common library
#define MSIF_CHIP                   0x000B
#define MSIF_CPU                    '0'
#define MSIF_OS                     '2'

#define DISABLE                     0
#define ENABLE                      1



#endif //defined(REDLION_LINUX_KERNEL_ENVI)

#endif // _DRVHVD_COMMON_H_
