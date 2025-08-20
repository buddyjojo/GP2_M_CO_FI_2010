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

#ifndef _MDRV_MVOP_ST_H_
#define _MDRV_MVOP_ST_H_

#include "mdrv_types.h"
#include "mdrv_mvd_st.h"

//#define ENABLE_MVOP_DUPLICATE //080918 LGE gbtogether

/// MVOP scan type
typedef enum
{
    MVOPSCAN_720x480_59I,    ///< 0
    MVOPSCAN_704x480_60I,    ///< 1
    MVOPSCAN_704x480_59I,
    MVOPSCAN_640x480_60I,
    MVOPSCAN_640x480_59I,

    MVOPSCAN_544x480_60I,    ///< 2
    MVOPSCAN_544x480_59I,
    MVOPSCAN_528x480_60I,    ///< 3
    MVOPSCAN_528x480_59I,
    MVOPSCAN_352x480_59I,
    MVOPSCAN_480x480_59I,
    MVOPSCAN_352X240_29P,

    MVOPSCAN_720x480_60P,    ///< 4
    MVOPSCAN_704x480_60P,    ///< 5
    MVOPSCAN_720x480_59P,
    MVOPSCAN_704x480_59P,
    MVOPSCAN_720x480_30P,

    MVOPSCAN_704x480_30P,
    MVOPSCAN_720x480_29P,
    MVOPSCAN_704x480_29P,
    MVOPSCAN_720x480_24P,
    MVOPSCAN_704x480_24P,

    MVOPSCAN_704x480_23P,
    MVOPSCAN_720x480_23P,
    MVOPSCAN_544x480_23P,
    MVOPSCAN_528x480_23P,
    MVOPSCAN_352x480_23P,

    MVOPSCAN_720x480_48I,
    MVOPSCAN_704x480_47I,
    MVOPSCAN_544x480_47I,
    MVOPSCAN_528x480_47I,
    MVOPSCAN_352x480_47I,

    MVOPSCAN_640x480_60P,    ///< 5
    MVOPSCAN_640x480_59P,
    MVOPSCAN_640x480_30P,
    MVOPSCAN_640x480_29P,
    MVOPSCAN_640x480_24P,

    MVOPSCAN_640x480_23P,

    MVOPSCAN_720x576_50I,    ///< 6
    MVOPSCAN_704x576_50I,    ///< 7
    MVOPSCAN_544x576_50I,    ///< 8
    MVOPSCAN_480x576_50I,
    MVOPSCAN_720x576_50P,    ///< 9
    MVOPSCAN_704x576_50P,    ///< 10
    MVOPSCAN_720x576_25P,    ///< 9
    MVOPSCAN_704x576_25P,    ///< 10

    MVOPSCAN_1280x720_60P,    ///< 11
    MVOPSCAN_1280x720_59P,
    MVOPSCAN_1280x720_30P,
    MVOPSCAN_1280x720_29P,

    MVOPSCAN_1280x720_24P,
    MVOPSCAN_1280x720_23P,
    MVOPSCAN_1920x1080_60I,    ///< 13
    MVOPSCAN_1920x1080_59I,
    MVOPSCAN_1920x1088_60I,    ///< 14

    MVOPSCAN_1920x1080_30P,
    MVOPSCAN_1920x1080_29P,
    MVOPSCAN_1920x1080_24P,
    MVOPSCAN_1920x1080_23P,
    MVOPSCAN_1440x1089_30P,

    MVOPSCAN_1440x1080_29P,
    MVOPSCAN_1440x1080_24P,
    MVOPSCAN_1440x1080_23P,
    MVOPSCAN_1440x1080_60I,
    MVOPSCAN_1440x1080_59I,

    MVOPSCAN_1920x1088_59I,

    MVOPSCAN_1280x720_50P,    ///< 12
    MVOPSCAN_1920x1080_50I,    ///< 15
    MVOPSCAN_1920x1088_50I,    ///< 16

    MVOPSCAN_NUM
} MVOPSCANTYPE;

/// MVOP input mode
typedef enum
{
    MVOPINPUT_HARDWIRE = 0,        //!< hardwire mode (MVD)
    MVOPINPUT_HARDWIRECLIP = 1,    //!< hardware clip mode (MVD)
    MVOPINPUT_MCUCTRL = 2,        //!< MCU control mode (M4VD, JPG)
} MVOPINPUTMODE;

/// MVOP input parameter
typedef struct
{
    U32 u32YOffset;
    U32 u32UVOffset;
    U16 u16HSize;
    U16 u16VSize;
    U16 u16StripSize;

    BOOL bYUV422;    //!< YUV422 or YUV420
    BOOL bSD;        //!< SD or HD
    BOOL bProgressive;   //!< Progressive or Interlace

    // in func MDrv_MVOP_Input_Mode(), bSD is used to set dc_strip[7:0].
    // in func MDrv_MVOP_Input_Mode_Ext(), bSD is don't care and
    //    dc_strip[7:0] is set according to Hsize
    BOOL bUV7bit;        // +S3, UV 7 bit or not
    BOOL bDramRdContd;   // +S3, continue read out or jump 32
    BOOL bField;         // +S3, Field 0 or 1
    BOOL b422pack;       // +S3, YUV422 pack mode

} MVOPINPUTPARAM;

/// Interrupt mask
typedef enum
{
    MVOPINTR_BUFFER_UNDERFLOW = 0x1,  //!< DC buffer underflow interrupt mask
    MVOPINTR_BUFFER_OVERFLOW = 0x2,   //!< DC buffer overflow interrupt mask
    MVOPINTR_VSYNC = 0x4,             //!< DC2MA VSYNC interrupt mask
    MVOPINTR_HSYNC = 0x8,             //!< DC2MA HSYNC interrupt mask
    MVOPINTR_READY = 0x10,            //!< DC ready interrupt mask
    MVOPINTR_FIELD_CHANGE = 0x20,     //!< DC field change interrupt mask
} MVOPINTRTYPE;


typedef struct
{
    U16 u16V_TotalCount;        ///< Vertical total count
    U16 u16H_TotalCount;        ///< Horizontal total count
    U16 u16VBlank0_Start;       ///< Vertical blank 0 start
    U16 u16VBlank0_End;         ///< Vertical blank 0 End
    U16 u16VBlank1_Start;       ///< Vertical blank 1 start
    U16 u16VBlank1_End;         ///< Vertical blank 1 End
    U16 u16TopField_Start;      ///< Top field start
    U16 u16BottomField_Start;   ///< bottom field start
    U16 u16HActive_Start;       ///< Horizontal disaply start
    U8  u8VSync_Offset;         ///< Vertical sync offset
    BOOL bInterlace;            ///< interlace or not
    U8 u8Framerate;             ///< frame rate
    U16 u16H_Freq ;             ///< horizontal frequency
    U16 u16Num;                 ///< MVOP SYNTHESIZER numerator
    U16 u16Den;                 ///< MVOP SYNTHESIZER denumerator
    U8 u8MvdFRCType;            ///< flag for frame rate convert
    U16 u16ExpFrameRate;        ///< Frame Rate
    U16 u16Width;               ///< Width
    U16 u16Height;              ///< Height
    BOOL bHDuplicate;           ///< flag for MVOP horizontal duplicate

#if 1	// 문제되어 주석처리함.
    //FitchHsu 20081118 DVIX video is broken at bottom line
    U16 u16CropRight;
    U16 u16CropLeft;
    U16 u16CropBottom;
    U16 u16CropTop;
#endif

} MS_MVOP_TIMING;

typedef struct
{
	MS_MVOP_TIMING* timing;
	MVD_FRAMEINFO* mvd_info;

} MPEGTIMINGINFO;

// MVOP_IOC_SET_TEST_PATTERN
typedef struct
{
    U8 srcIdx;
    U8 u8PatternType;
    U8 u8Pattern;
} MVOP_SET_TEST_PATTERN_t;	// lemonic LGE 080908



#endif
