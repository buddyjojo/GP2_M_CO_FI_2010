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
/// @file   mdrv_sar_st.h
/// @brief  SAR Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Data structure definition
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_SAR_ST_H_
#define _MDRV_SAR_ST_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mhal_sar.h"

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
#if 0
typedef struct
{
    U8 u8UpBnd;   //upper bound
    U8 u8LoBnd;  //low bound
} SAR_BND_t;

typedef SAR_BND_t SAR_BND2_t;
#endif

typedef struct
{
    U8 u8SARChID;
    SAR_BND2_t SARChBnd;
    U8 u8KeyLevelNum;
    U8 u8KeyThreshold[4];
    U8 u8KeyCode[4];
    U8 u8RetVal;
} SAR_CFG_t;

typedef struct
{
    U8 u8Index;
    U8 u8Channel;
    U8 u8Key;
    U8 u8Rep;
    U8 u8RetVal;
} SAR_Key_t;

enum KEYPAD_ADC_CHANNEL
{
    KEYPAD_ADC_CHANNEL_1 = 0,
    KEYPAD_ADC_CHANNEL_2,
    KEYPAD_ADC_CHANNEL_3,
    KEYPAD_ADC_CHANNEL_4,
    KEYPAD_ADC_CHANNEL_5,
    KEYPAD_ADC_CHANNEL_6,
    KEYPAD_ADC_CHANNEL_7,
    KEYPAD_ADC_CHANNEL_8,
};

#endif // _MDRV_SAR_ST_H_
