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

#ifndef _DRVGETP_H_
#define _DRVGETP_H_

#ifdef RED_LION
#include "mdrv_types.h"
#else
#include "msTypes.h"
#endif

//typedef union LONG16_BYTE_t
//{
//    MS_U16 u16Num;
//    MS_U8 u8Num[2];
//    // u8Num[0]  MSB
//    // u8Num[1]  LSB
//} LONG16_BYTE;
//
//typedef union LONG32_BYTE_t
//{
//    MS_U32 u32Num;
//    MS_U8 u8Num[4];
//    // u8Num[0]  MSB
//    // u8Num[1]
//    // u8Num[2]
//    // u8Num[3]  LSB
//} LONG32_BYTE;
//
//typedef union FLOAT_BYTE_T
//{
//	float fvalue;
//	MS_U8 u8Num[4];
//} FLOAT_BYTE;
//
//typedef struct VALUE16_t
//{
//    MS_U8 high;
//    MS_U8 low;
//} VALUE16;

//typedef MS_S8 FONTHANDLE;
//typedef MS_S8 BMPHANDLE;
//typedef MS_S8 DBHANDLE;

#endif
