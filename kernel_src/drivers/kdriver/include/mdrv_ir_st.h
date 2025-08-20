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
/// @file   drvIR.h
/// @brief  IR Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Data structure definition
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_IR_ST_H_
#define _MDRV_IR_ST_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    KEY_TV_RADIO,
    KEY_CHANNEL_LIST,
    KEY_CHANNEL_FAV_LIST,
    KEY_CHANNEL_RETURN,
    KEY_CHANNEL_PLUS,
    KEY_CHANNEL_MINUS,

    KEY_AUDIO,
    KEY_VOLUME_PLUS,
    KEY_VOLUME_MINUS,

    KEY_UP,
    KEY_POWER,
    KEY_EXIT,
    KEY_MENU,
    KEY_DOWN,
    KEY_LEFT,
    KEY_SELECT,
    KEY_RIGHT,

    KEY_NUMERIC_0,
    KEY_NUMERIC_1,
    KEY_NUMERIC_2,
    KEY_NUMERIC_3,
    KEY_NUMERIC_4,
    KEY_NUMERIC_5,
    KEY_NUMERIC_6,
    KEY_NUMERIC_7,
    KEY_NUMERIC_8,
    KEY_NUMERIC_9,

    KEY_MUTE,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_CLOCK,

    KEY_INFO,
    KEY_RED,
    KEY_GREEN,
    KEY_YELLOW,
    KEY_BLUE,
    KEY_MTS,
    KEY_NINE_LATTICE,
    KEY_TTX,
    KEY_CC,
    KEY_INPUT_SOURCE,
    KEY_CRADRD,
  //KEY_PICTURE,
    KEY_ZOOM,
    KEY_DASH,
    KEY_SLEEP,
    KEY_EPG,
    KEY_PIP,

  	KEY_MIX,
    KEY_INDEX,
    KEY_HOLD,
    KEY_PREVIOUS,
    KEY_NEXT,
    KEY_BACKWARD,
    KEY_FORWARD,
    KEY_PLAY,
    KEY_RECORD,
    KEY_STOP,
    KEY_PAUSE,

    KEY_SIZE,
    KEY_REVEAL,
    KEY_SUBCODE,

    KEY_DUMMY,
    KEY_NUM,
} EN_KEY;

typedef struct
{
    U32 u32_1stDelayTimeMs;
    U32 u32_2ndDelayTimeMs;
#ifdef NOT_USED_4_LGE	// dreamer@lge.com
    U8 data3;
#endif
} MS_IR_DelayTime, *PMS_IR_DelayTime;

typedef struct
{
    U8 u8Key;
    U8 u8Flag;
    U8 u8Valid;
} MS_IR_KeyInfo, *PMS_IR_KeyInfo;

typedef struct
{
    unsigned long time;
} MS_IR_LastKeyTime, *PMS_IR_LastKeyTime;

typedef struct
{
    U8 u8KeyIn;
    U8 u8KeyOut;
} MS_IR_KeyValue, *PMS_IR_KeyValue;

#endif // _MDRV_IR_ST_H_
