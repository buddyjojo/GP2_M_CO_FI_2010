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
/// file    drvIR.c
/// @brief  IR Control Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>  //added
#include <linux/timer.h> //added
#include <asm/io.h>

#include "Board.h"
#include "mst_devid.h"
#include "mhal_ir_reg.h"
#include "mdrv_ir_io.h"
#include "mdrv_ir.h"
#include "chip_int.h"

#include "IR_CUS03_DTV.h"

/*
#if (IR_TYPE_SEL == IR_TYPE_OLD)
#include "IR_MSTAR_OLD.h"
#elif (IR_TYPE_SEL == IR_TYPE_NEW)
#include "IR_MSTAR_NEW.h"
#elif (IR_TYPE_SEL == IR_TYPE_MSTAR_DTV)
#include "IR_MSTAR_DTV.h"
#elif (IR_TYPE_SEL == IR_TYPE_RC_V16)
#include "IR_MSTAR_RC_V16.h"
#elif (IR_TYPE_SEL == IR_TYPE_MSTAR_RAW)
#include "IR_MSTAR_RAW.h"
#elif (IR_TYPE_SEL == IR_TYPE_CUS03_DTV)
#include "IR_CUS03_DTV.h"
#elif (IR_TYPE_SEL == IR_TYPE_LG_GP_DTV)
#include "IR_LG_GP_DTV.h"
#else
#include "IR_MSTAR_DTV.h"
#endif
*/

extern IRModHandle IRDev;
//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define REG(addr)                   (*(volatile u32 *)(addr))
#define IR_PRINT(fmt, args...)     printk("[IR][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------


#define IR_RAW_DATA_NUM	        4
//#define IR_FILTER_REPEAT_NUM    1

#if (IR_MODE_SEL == IR_TYPE_SWDECODE_MODE) || defined(IR_INCLUDE_TV_LINK_AND_WB_CODE)
#define IR_SWDECODE_MODE_BUF_LEN        100
#endif

//#define IR_DEBUG
#ifdef IR_DEBUG
#define DEBUG_IR(x) (x)
#else
#define DEBUG_IR(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_IR_KEY_PROPERTY_INIT,
    E_IR_KEY_PROPERTY_1st,
    E_IR_KEY_PROPERTY_FOLLOWING
} IRKeyProperty;


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
static U8 bIRPass = 0;

extern wait_queue_head_t	key_wait_q;

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------
#if (IR_MODE_SEL == IR_TYPE_RAWDATA_MODE)  || defined(IR_INCLUDE_TV_LINK_AND_WB_CODE)
static U8   _u8IRRawModeBuf[IR_RAW_DATA_NUM];
static U32  _u8IRRawModeCount;
#endif

#if (IR_MODE_SEL == IR_TYPE_SWDECODE_MODE) || defined(IR_INCLUDE_TV_LINK_AND_WB_CODE)
static U32  _u32IRData[IR_SWDECODE_MODE_BUF_LEN];
static U32  _u32IRDataTmp[IR_SWDECODE_MODE_BUF_LEN];
static U32  _u32IRCount;
static U32  _u32IRAllCount;
#endif

static U32  _u32_1stDelayTimeMs;
static U32  _u32_2ndDelayTimeMs;
static IRKeyProperty _ePrevKeyProperty;
static U8   _u8PrevKeyCode;
static unsigned long  _ulPrevKeyTime;
#ifdef	NOT_USED_4_LGE
static unsigned long  _ulLastKeyPresentTime;
#endif

#ifdef	IR_INCLUDE_TV_LINK_AND_WB_CODE

static U8 	_u8IRMode = IR_MODE_NORMAL;
static BOOL	_IsWBCodeMode = FALSE;
static BOOL	_IsTvLinkMode = FALSE;

static struct _TV_LINK_KEY_INFO_T
{
	U8	u8Data[IR_TV_LINK_DATA_NUM];
	U8	u8Flag;
	U8	u8Valid;

} _TvLinkReceived;

static const U8 _LSB[] =
{
/* 0x00 : 0x00 */ 0x00,
/* 0x01 : 0x80 */ 0x80,
/* 0x02 : 0x40 */ 0x40,
/* 0x03 : 0xc0 */ 0xc0,
/* 0x04 : 0x20 */ 0x20,
/* 0x05 : 0xa0 */ 0xa0,
/* 0x06 : 0x60 */ 0x60,
/* 0x07 : 0xe0 */ 0xe0,
/* 0x08 : 0x10 */ 0x10,
/* 0x09 : 0x90 */ 0x90,
/* 0x0a : 0x50 */ 0x50,
/* 0x0b : 0xd0 */ 0xd0,
/* 0x0c : 0x30 */ 0x30,
/* 0x0d : 0xb0 */ 0xb0,
/* 0x0e : 0x70 */ 0x70,
/* 0x0f : 0xf0 */ 0xf0,
/* 0x10 : 0x08 */ 0x08,
/* 0x11 : 0x88 */ 0x88,
/* 0x12 : 0x48 */ 0x48,
/* 0x13 : 0xc8 */ 0xc8,
/* 0x14 : 0x28 */ 0x28,
/* 0x15 : 0xa8 */ 0xa8,
/* 0x16 : 0x68 */ 0x68,
/* 0x17 : 0xe8 */ 0xe8,
/* 0x18 : 0x18 */ 0x18,
/* 0x19 : 0x98 */ 0x98,
/* 0x1a : 0x58 */ 0x58,
/* 0x1b : 0xd8 */ 0xd8,
/* 0x1c : 0x38 */ 0x38,
/* 0x1d : 0xb8 */ 0xb8,
/* 0x1e : 0x78 */ 0x78,
/* 0x1f : 0xf8 */ 0xf8,
/* 0x20 : 0x04 */ 0x04,
/* 0x21 : 0x84 */ 0x84,
/* 0x22 : 0x44 */ 0x44,
/* 0x23 : 0xc4 */ 0xc4,
/* 0x24 : 0x24 */ 0x24,
/* 0x25 : 0xa4 */ 0xa4,
/* 0x26 : 0x64 */ 0x64,
/* 0x27 : 0xe4 */ 0xe4,
/* 0x28 : 0x14 */ 0x14,
/* 0x29 : 0x94 */ 0x94,
/* 0x2a : 0x54 */ 0x54,
/* 0x2b : 0xd4 */ 0xd4,
/* 0x2c : 0x34 */ 0x34,
/* 0x2d : 0xb4 */ 0xb4,
/* 0x2e : 0x74 */ 0x74,
/* 0x2f : 0xf4 */ 0xf4,
/* 0x30 : 0x0c */ 0x0c,
/* 0x31 : 0x8c */ 0x8c,
/* 0x32 : 0x4c */ 0x4c,
/* 0x33 : 0xcc */ 0xcc,
/* 0x34 : 0x2c */ 0x2c,
/* 0x35 : 0xac */ 0xac,
/* 0x36 : 0x6c */ 0x6c,
/* 0x37 : 0xec */ 0xec,
/* 0x38 : 0x1c */ 0x1c,
/* 0x39 : 0x9c */ 0x9c,
/* 0x3a : 0x5c */ 0x5c,
/* 0x3b : 0xdc */ 0xdc,
/* 0x3c : 0x3c */ 0x3c,
/* 0x3d : 0xbc */ 0xbc,
/* 0x3e : 0x7c */ 0x7c,
/* 0x3f : 0xfc */ 0xfc,
/* 0x40 : 0x02 */ 0x02,
/* 0x41 : 0x82 */ 0x82,
/* 0x42 : 0x42 */ 0x42,
/* 0x43 : 0xc2 */ 0xc2,
/* 0x44 : 0x22 */ 0x22,
/* 0x45 : 0xa2 */ 0xa2,
/* 0x46 : 0x62 */ 0x62,
/* 0x47 : 0xe2 */ 0xe2,
/* 0x48 : 0x12 */ 0x12,
/* 0x49 : 0x92 */ 0x92,
/* 0x4a : 0x52 */ 0x52,
/* 0x4b : 0xd2 */ 0xd2,
/* 0x4c : 0x32 */ 0x32,
/* 0x4d : 0xb2 */ 0xb2,
/* 0x4e : 0x72 */ 0x72,
/* 0x4f : 0xf2 */ 0xf2,
/* 0x50 : 0x0a */ 0x0a,
/* 0x51 : 0x8a */ 0x8a,
/* 0x52 : 0x4a */ 0x4a,
/* 0x53 : 0xca */ 0xca,
/* 0x54 : 0x2a */ 0x2a,
/* 0x55 : 0xaa */ 0xaa,
/* 0x56 : 0x6a */ 0x6a,
/* 0x57 : 0xea */ 0xea,
/* 0x58 : 0x1a */ 0x1a,
/* 0x59 : 0x9a */ 0x9a,
/* 0x5a : 0x5a */ 0x5a,
/* 0x5b : 0xda */ 0xda,
/* 0x5c : 0x3a */ 0x3a,
/* 0x5d : 0xba */ 0xba,
/* 0x5e : 0x7a */ 0x7a,
/* 0x5f : 0xfa */ 0xfa,
/* 0x60 : 0x06 */ 0x06,
/* 0x61 : 0x86 */ 0x86,
/* 0x62 : 0x46 */ 0x46,
/* 0x63 : 0xc6 */ 0xc6,
/* 0x64 : 0x26 */ 0x26,
/* 0x65 : 0xa6 */ 0xa6,
/* 0x66 : 0x66 */ 0x66,
/* 0x67 : 0xe6 */ 0xe6,
/* 0x68 : 0x16 */ 0x16,
/* 0x69 : 0x96 */ 0x96,
/* 0x6a : 0x56 */ 0x56,
/* 0x6b : 0xd6 */ 0xd6,
/* 0x6c : 0x36 */ 0x36,
/* 0x6d : 0xb6 */ 0xb6,
/* 0x6e : 0x76 */ 0x76,
/* 0x6f : 0xf6 */ 0xf6,
/* 0x70 : 0x0e */ 0x0e,
/* 0x71 : 0x8e */ 0x8e,
/* 0x72 : 0x4e */ 0x4e,
/* 0x73 : 0xce */ 0xce,
/* 0x74 : 0x2e */ 0x2e,
/* 0x75 : 0xae */ 0xae,
/* 0x76 : 0x6e */ 0x6e,
/* 0x77 : 0xee */ 0xee,
/* 0x78 : 0x1e */ 0x1e,
/* 0x79 : 0x9e */ 0x9e,
/* 0x7a : 0x5e */ 0x5e,
/* 0x7b : 0xde */ 0xde,
/* 0x7c : 0x3e */ 0x3e,
/* 0x7d : 0xbe */ 0xbe,
/* 0x7e : 0x7e */ 0x7e,
/* 0x7f : 0xfe */ 0xfe,
/* 0x80 : 0x01 */ 0x01,
/* 0x81 : 0x81 */ 0x81,
/* 0x82 : 0x41 */ 0x41,
/* 0x83 : 0xc1 */ 0xc1,
/* 0x84 : 0x21 */ 0x21,
/* 0x85 : 0xa1 */ 0xa1,
/* 0x86 : 0x61 */ 0x61,
/* 0x87 : 0xe1 */ 0xe1,
/* 0x88 : 0x11 */ 0x11,
/* 0x89 : 0x91 */ 0x91,
/* 0x8a : 0x51 */ 0x51,
/* 0x8b : 0xd1 */ 0xd1,
/* 0x8c : 0x31 */ 0x31,
/* 0x8d : 0xb1 */ 0xb1,
/* 0x8e : 0x71 */ 0x71,
/* 0x8f : 0xf1 */ 0xf1,
/* 0x90 : 0x09 */ 0x09,
/* 0x91 : 0x89 */ 0x89,
/* 0x92 : 0x49 */ 0x49,
/* 0x93 : 0xc9 */ 0xc9,
/* 0x94 : 0x29 */ 0x29,
/* 0x95 : 0xa9 */ 0xa9,
/* 0x96 : 0x69 */ 0x69,
/* 0x97 : 0xe9 */ 0xe9,
/* 0x98 : 0x19 */ 0x19,
/* 0x99 : 0x99 */ 0x99,
/* 0x9a : 0x59 */ 0x59,
/* 0x9b : 0xd9 */ 0xd9,
/* 0x9c : 0x39 */ 0x39,
/* 0x9d : 0xb9 */ 0xb9,
/* 0x9e : 0x79 */ 0x79,
/* 0x9f : 0xf9 */ 0xf9,
/* 0xa0 : 0x05 */ 0x05,
/* 0xa1 : 0x85 */ 0x85,
/* 0xa2 : 0x45 */ 0x45,
/* 0xa3 : 0xc5 */ 0xc5,
/* 0xa4 : 0x25 */ 0x25,
/* 0xa5 : 0xa5 */ 0xa5,
/* 0xa6 : 0x65 */ 0x65,
/* 0xa7 : 0xe5 */ 0xe5,
/* 0xa8 : 0x15 */ 0x15,
/* 0xa9 : 0x95 */ 0x95,
/* 0xaa : 0x55 */ 0x55,
/* 0xab : 0xd5 */ 0xd5,
/* 0xac : 0x35 */ 0x35,
/* 0xad : 0xb5 */ 0xb5,
/* 0xae : 0x75 */ 0x75,
/* 0xaf : 0xf5 */ 0xf5,
/* 0xb0 : 0x0d */ 0x0d,
/* 0xb1 : 0x8d */ 0x8d,
/* 0xb2 : 0x4d */ 0x4d,
/* 0xb3 : 0xcd */ 0xcd,
/* 0xb4 : 0x2d */ 0x2d,
/* 0xb5 : 0xad */ 0xad,
/* 0xb6 : 0x6d */ 0x6d,
/* 0xb7 : 0xed */ 0xed,
/* 0xb8 : 0x1d */ 0x1d,
/* 0xb9 : 0x9d */ 0x9d,
/* 0xba : 0x5d */ 0x5d,
/* 0xbb : 0xdd */ 0xdd,
/* 0xbc : 0x3d */ 0x3d,
/* 0xbd : 0xbd */ 0xbd,
/* 0xbe : 0x7d */ 0x7d,
/* 0xbf : 0xfd */ 0xfd,
/* 0xc0 : 0x03 */ 0x03,
/* 0xc1 : 0x83 */ 0x83,
/* 0xc2 : 0x43 */ 0x43,
/* 0xc3 : 0xc3 */ 0xc3,
/* 0xc4 : 0x23 */ 0x23,
/* 0xc5 : 0xa3 */ 0xa3,
/* 0xc6 : 0x63 */ 0x63,
/* 0xc7 : 0xe3 */ 0xe3,
/* 0xc8 : 0x13 */ 0x13,
/* 0xc9 : 0x93 */ 0x93,
/* 0xca : 0x53 */ 0x53,
/* 0xcb : 0xd3 */ 0xd3,
/* 0xcc : 0x33 */ 0x33,
/* 0xcd : 0xb3 */ 0xb3,
/* 0xce : 0x73 */ 0x73,
/* 0xcf : 0xf3 */ 0xf3,
/* 0xd0 : 0x0b */ 0x0b,
/* 0xd1 : 0x8b */ 0x8b,
/* 0xd2 : 0x4b */ 0x4b,
/* 0xd3 : 0xcb */ 0xcb,
/* 0xd4 : 0x2b */ 0x2b,
/* 0xd5 : 0xab */ 0xab,
/* 0xd6 : 0x6b */ 0x6b,
/* 0xd7 : 0xeb */ 0xeb,
/* 0xd8 : 0x1b */ 0x1b,
/* 0xd9 : 0x9b */ 0x9b,
/* 0xda : 0x5b */ 0x5b,
/* 0xdb : 0xdb */ 0xdb,
/* 0xdc : 0x3b */ 0x3b,
/* 0xdd : 0xbb */ 0xbb,
/* 0xde : 0x7b */ 0x7b,
/* 0xdf : 0xfb */ 0xfb,
/* 0xe0 : 0x07 */ 0x07,
/* 0xe1 : 0x87 */ 0x87,
/* 0xe2 : 0x47 */ 0x47,
/* 0xe3 : 0xc7 */ 0xc7,
/* 0xe4 : 0x27 */ 0x27,
/* 0xe5 : 0xa7 */ 0xa7,
/* 0xe6 : 0x67 */ 0x67,
/* 0xe7 : 0xe7 */ 0xe7,
/* 0xe8 : 0x17 */ 0x17,
/* 0xe9 : 0x97 */ 0x97,
/* 0xea : 0x57 */ 0x57,
/* 0xeb : 0xd7 */ 0xd7,
/* 0xec : 0x37 */ 0x37,
/* 0xed : 0xb7 */ 0xb7,
/* 0xee : 0x77 */ 0x77,
/* 0xef : 0xf7 */ 0xf7,
/* 0xf0 : 0x0f */ 0x0f,
/* 0xf1 : 0x8f */ 0x8f,
/* 0xf2 : 0x4f */ 0x4f,
/* 0xf3 : 0xcf */ 0xcf,
/* 0xf4 : 0x2f */ 0x2f,
/* 0xf5 : 0xaf */ 0xaf,
/* 0xf6 : 0x6f */ 0x6f,
/* 0xf7 : 0xef */ 0xef,
/* 0xf8 : 0x1f */ 0x1f,
/* 0xf9 : 0x9f */ 0x9f,
/* 0xfa : 0x5f */ 0x5f,
/* 0xfb : 0xdf */ 0xdf,
/* 0xfc : 0x3f */ 0x3f,
/* 0xfd : 0xbf */ 0xbf,
/* 0xfe : 0x7f */ 0x7f,
/* 0xff : 0xff */ 0xff

};
#else	/* IR_INCLUDE_TV_LINK_AND_WB_CODE */

static MS_IR_KeyInfo _KeyReceived;   //temporary solution

#endif	/* IR_INCLUDE_TV_LINK_AND_WB_CODE */

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static void _MDrv_IR_Timing(void);
static irqreturn_t _MDrv_IR_ISR(int irq, void *dev_id);
static U8   _MDrv_IR_ParseKey(U8 u8KeyData);
static BOOL _MDrv_IR_GetKey(U8 *pu8Key, U8 *pu8Flag);
static unsigned long _MDrv_IR_GetSystemTime(void);

#ifdef	IR_INCLUDE_TV_LINK_AND_WB_CODE
static BOOL _MDrv_IR_GetKeyOfWBCode(U8 *pu8Key, U8 *pu8Flag);
static BOOL _MDrv_IR_GetKeyOfTVLink(U8 *pu8Key, U8 *pu8Flag);
#endif
//-------------------------------------------------------------------------------------------------
/// Translate from IR key to internal key.
/// @param  u8KeyData  \b IN: IR key value.
/// @return translated internal IR key.
//-------------------------------------------------------------------------------------------------
static U8 _MDrv_IR_ParseKey(U8 u8KeyData)
{
    U8 u8RetVal;

    if ( bIRPass ) return KEY_DUMMY;

    switch(u8KeyData)
    {
    case IRKEY_TV_RADIO        : u8RetVal = KEY_TV_RADIO;         break;
    case IRKEY_CHANNEL_LIST    : u8RetVal = KEY_CHANNEL_LIST;     break;
    case IRKEY_CHANNEL_FAV_LIST: u8RetVal = KEY_CHANNEL_FAV_LIST; break;
    case IRKEY_CHANNEL_RETURN  : u8RetVal = KEY_CHANNEL_RETURN;   break;
    case IRKEY_CHANNEL_PLUS    : u8RetVal = KEY_CHANNEL_PLUS;     break;
    case IRKEY_CHANNEL_MINUS   : u8RetVal = KEY_CHANNEL_MINUS;    break;

    case IRKEY_AUDIO           : u8RetVal = KEY_AUDIO;            break;
    case IRKEY_VOLUME_PLUS     : u8RetVal = KEY_VOLUME_PLUS;      break;
    case IRKEY_VOLUME_MINUS    : u8RetVal = KEY_VOLUME_MINUS;     break;

    case IRKEY_UP              : u8RetVal = KEY_UP;               break;
    case IRKEY_POWER           : u8RetVal = KEY_POWER;            break;
    case IRKEY_EXIT            : u8RetVal = KEY_EXIT;             break;
    case IRKEY_MENU            : u8RetVal = KEY_MENU;             break;
    case IRKEY_DOWN            : u8RetVal = KEY_DOWN;             break;
    case IRKEY_LEFT            : u8RetVal = KEY_LEFT;             break;
    case IRKEY_SELECT          : u8RetVal = KEY_SELECT;           break;
    case IRKEY_RIGHT           : u8RetVal = KEY_RIGHT;            break;

    case IRKEY_NUM_0           : u8RetVal = KEY_NUMERIC_0;        break;
    case IRKEY_NUM_1           : u8RetVal = KEY_NUMERIC_1;        break;
    case IRKEY_NUM_2           : u8RetVal = KEY_NUMERIC_2;        break;
    case IRKEY_NUM_3           : u8RetVal = KEY_NUMERIC_3;        break;
    case IRKEY_NUM_4           : u8RetVal = KEY_NUMERIC_4;        break;
    case IRKEY_NUM_5           : u8RetVal = KEY_NUMERIC_5;        break;
    case IRKEY_NUM_6           : u8RetVal = KEY_NUMERIC_6;        break;
    case IRKEY_NUM_7           : u8RetVal = KEY_NUMERIC_7;        break;
    case IRKEY_NUM_8           : u8RetVal = KEY_NUMERIC_8;        break;
    case IRKEY_NUM_9           : u8RetVal = KEY_NUMERIC_9;        break;

    case IRKEY_MUTE            : u8RetVal = KEY_MUTE;             break;
    case IRKEY_PAGE_UP         : u8RetVal = KEY_PAGE_UP;          break;
    case IRKEY_PAGE_DOWN       : u8RetVal = KEY_PAGE_DOWN;        break;
    case IRKEY_CLOCK           : u8RetVal = KEY_CLOCK;            break;

    case IRKEY_INFO            : u8RetVal = KEY_INFO;             break;
    case IRKEY_RED             : u8RetVal = KEY_RED;             break;
    case IRKEY_GREEN           : u8RetVal = KEY_GREEN;            break;
    case IRKEY_YELLOW          : u8RetVal = KEY_YELLOW;           break;
    case IRKEY_BLUE            : u8RetVal = KEY_BLUE;             break;
    case IRKEY_MTS             : u8RetVal = KEY_MTS;              break;
    case IRKEY_NINE_LATTICE    : u8RetVal = KEY_NINE_LATTICE;     break;
#if defined(DVB_SYSTEM)
    case IRKEY_TTX             : u8RetVal = KEY_TTX;              break;
#elif defined(ATSC_SYSTEM)
    case IRKEY_CC              : u8RetVal = KEY_CC;               break;
#endif
    case IRKEY_INPUT_SOURCE    : u8RetVal = KEY_INPUT_SOURCE;     break;
    case IRKEY_CRADRD          : u8RetVal = KEY_CRADRD;           break;
  //case IRKEY_PICTURE         : u8RetVal = KEY_PICTURE;          break;
    case IRKEY_ZOOM            : u8RetVal = KEY_ZOOM;             break;
    case IRKEY_DASH            : u8RetVal = KEY_DASH;             break;
    case IRKEY_SLEEP           : u8RetVal = KEY_SLEEP;            break;
    case IRKEY_EPG             : u8RetVal = KEY_EPG;              break;
    case IRKEY_PIP             : u8RetVal = KEY_PIP;              break;

  	case IRKEY_MIX             : u8RetVal = KEY_MIX;              break;
    case IRKEY_INDEX           : u8RetVal = KEY_INDEX;            break;
    case IRKEY_HOLD            : u8RetVal = KEY_HOLD;             break;
    case IRKEY_PREVIOUS        : u8RetVal = KEY_PREVIOUS;         break;
    case IRKEY_NEXT            : u8RetVal = KEY_NEXT;             break;
    case IRKEY_BACKWARD        : u8RetVal = KEY_BACKWARD;         break;
    case IRKEY_FORWARD         : u8RetVal = KEY_FORWARD;          break;
    case IRKEY_PLAY            : u8RetVal = KEY_PLAY;             break;
    case IRKEY_RECORD          : u8RetVal = KEY_RECORD;           break;
    case IRKEY_STOP            : u8RetVal = KEY_STOP;             break;
    case IRKEY_PAUSE           : u8RetVal = KEY_PAUSE;            break;

    case IRKEY_SIZE            : u8RetVal = KEY_SIZE;             break;
    case IRKEY_REVEAL          : u8RetVal = KEY_REVEAL;           break;
    case IRKEY_SUBCODE         : u8RetVal = KEY_SUBCODE;          break;

    default                    : u8RetVal = KEY_DUMMY;            break;
    }

    return u8RetVal;
}


//-------------------------------------------------------------------------------------------------
/// Set the timing of IrDa at BOOT stage.
/// @return None
//-------------------------------------------------------------------------------------------------
static void _MDrv_IR_Timing(void)
{
    // header code upper bound
    REG(REG_IR_HDC_UPB) = IR_HDC_UPB;

    // header code lower bound
    REG(REG_IR_HDC_LOB) = IR_HDC_LOB;

    // off code upper bound
    REG(REG_IR_OFC_UPB) = IR_OFC_UPB;

    // off code lower bound
    REG(REG_IR_OFC_LOB) = IR_OFC_LOB;

    // off code repeat upper bound
    REG(REG_IR_OFC_RP_UPB) = IR_OFC_RP_UPB;

    // off code repeat lower bound
    REG(REG_IR_OFC_RP_LOB) = IR_OFC_RP_LOB;

    // logical 0/1 high upper bound
    REG(REG_IR_LG01H_UPB) = IR_LG01H_UPB;

    // logical 0/1 high lower bound
    REG(REG_IR_LG01H_LOB) = IR_LG01H_LOB;

    // logical 0 upper bound
    REG(REG_IR_LG0_UPB) = IR_LG0_UPB;

    // logical 0 lower bound
    REG(REG_IR_LG0_LOB) = IR_LG0_LOB;

    // logical 1 upper bound
    REG(REG_IR_LG1_UPB) = IR_LG1_UPB;

    // logical 1 lower bound
    REG(REG_IR_LG1_LOB) = IR_LG1_LOB;

    // timeout cycles
    REG(REG_IR_TIMEOUT_CYC_L) = IR_RP_TIMEOUT & 0xFFFF;
    REG(REG_IR_TIMEOUT_CYC_H_CODE_BYTE) = 0x1400 | 0x30 | ((IR_RP_TIMEOUT >> 16) & 0x0F);

    REG(REG_IR_CKDIV_NUM_KEY_DATA) = IR_CKDIV_NUM;   // clock divider
}

static unsigned long _MDrv_IR_GetSystemTime(void)
{
    return((unsigned long)((jiffies)*(1000/HZ)));
    //return 0;
}

//-------------------------------------------------------------------------------------------------
/// ISR when receive IR key.
/// @return None
//-------------------------------------------------------------------------------------------------
/* taburin : 20090113,  power repeat key 입력 시 lock up 관련하여 수정 코드. 임시 saturn5에만 적용. 검증 완료후 전체 적용.*/
#if defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) || defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERVA_BOARD_ATSC_1)
//static int _stop_get_key = 0 ;
#endif
static irqreturn_t _MDrv_IR_ISR(int irq, void *dev_id)
{
    U8	u8Key = 0, u8RepeatFlag = 0;

#ifdef IR_INCLUDE_TV_LINK_AND_WB_CODE
	U8	u8KeyTvLink[IR_TV_LINK_DATA_NUM];
	BOOL bPass = FALSE;

#if defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) || defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERVA_BOARD_ATSC_1)
//       if(_stop_get_key)
 //          return IRQ_HANDLED;
#endif

	if( _u8IRMode == IR_MODE_WB_CODE )
	{
		if(_MDrv_IR_GetKeyOfWBCode( u8KeyTvLink, &u8RepeatFlag))
		{
#ifdef	NOT_USED_4_LGE
			_ulLastKeyPresentTime = _MDrv_IR_GetSystemTime();
#endif

			IR_PRINT("_MDrv_IR_ISR() -> %x, %x\n", u8Key, u8RepeatFlag);

			if( u8RepeatFlag == (_IR_READ_TYPE_WB_CODE - 1) )
			{
				memcpy( _TvLinkReceived.u8Data, u8KeyTvLink, IR_WB_CODE_DATA_NUM );
				_TvLinkReceived.u8Flag  	= u8RepeatFlag;
				_TvLinkReceived.u8Valid 	= 1;
			}
			else
			{
		        //temporary solution, need to implement ring buffer for this

				_TvLinkReceived.u8Data[0]	= u8KeyTvLink[0];
				_TvLinkReceived.u8Flag  	= u8RepeatFlag;
				_TvLinkReceived.u8Valid 	= 1;
			}

			if (IRDev.async_queue)
				wake_up_interruptible(&key_wait_q);
				//kill_fasync(&IRDev.async_queue, SIGRTMIN, POLL_IN);
		}
	}
	else if( _u8IRMode == IR_MODE_TV_LINK )
	{
		static unsigned long ulPreTime;

		if (_MDrv_IR_GetSystemTime() - ulPreTime > IR_TIMEOUT_CYC/1000) //timeout
		{
			_u32IRCount = 0;
			_u32IRAllCount = 0;
		}


		if (_u32IRCount < IR_SWDECODE_MODE_BUF_LEN)
		{
#if 1
			if( _u32IRAllCount % 2 == 0 )
			{
				_u32IRDataTmp[_u32IRCount] = ((REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & 0xF) << 16) | REG(REG_IR_SHOT_CNT_L);
				//_u32IRDataTmp[_u32IRCount] = ((REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & 0xF) << 16);
				//_u32IRDataTmp[_u32IRCount] = (*(volatile U8*)(REG_IR_SHOT_CNT_L + 1)) << 8;
				//_u32IRDataTmp[_u32IRCount] += (*(volatile U8*)(REG_IR_SHOT_CNT_L));
			}
			else
#endif
			{
				_u32IRData[_u32IRCount] = ((REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & 0xF) << 16) | REG(REG_IR_SHOT_CNT_L);
				//_u32IRData[_u32IRCount] = ((REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & 0xF) << 16);
				//_u32IRData[_u32IRCount] += REG(REG_IR_SHOT_CNT_L);
				//_u32IRData[_u32IRCount] = (*(volatile U8*)(REG_IR_SHOT_CNT_L + 1)) << 8;
				//_u32IRData[_u32IRCount] += (*(volatile U8*)(REG_IR_SHOT_CNT_L));
				_u32IRCount++;

				bPass = TRUE;
			}
			_u32IRAllCount++;
		}
		ulPreTime = _MDrv_IR_GetSystemTime();

		if( (bPass == TRUE ) && (_MDrv_IR_GetKeyOfTVLink( u8KeyTvLink, &u8RepeatFlag )) )
		{
#ifdef	NOT_USED_4_LGE
			_ulLastKeyPresentTime = _MDrv_IR_GetSystemTime();
#endif

			IR_PRINT("_MDrv_IR_ISR_TV_LINK() -> %x, %x\n", u8KeyTvLink[0], u8RepeatFlag);

			if( u8RepeatFlag == (_IR_READ_TYPE_WB_CODE - 1) )
			{
				memcpy( _TvLinkReceived.u8Data, u8KeyTvLink, IR_WB_CODE_DATA_NUM );
				_TvLinkReceived.u8Flag  	= u8RepeatFlag;
				_TvLinkReceived.u8Valid 	= 1;
			}
			else if( u8RepeatFlag == (_IR_READ_TYPE_TV_LINK - 1) )
			{
				memcpy( _TvLinkReceived.u8Data, u8KeyTvLink, IR_TV_LINK_DATA_NUM );
				_TvLinkReceived.u8Flag  	= u8RepeatFlag;
				_TvLinkReceived.u8Valid 	= 1;
			}
			else
			{
		        //temporary solution, need to implement ring buffer for this
				_TvLinkReceived.u8Data[0]	= u8KeyTvLink[0];
				_TvLinkReceived.u8Flag  	= u8RepeatFlag;
				_TvLinkReceived.u8Valid 	= 1;
			}

			if (IRDev.async_queue)
				wake_up_interruptible(&key_wait_q);
				//kill_fasync(&IRDev.async_queue, SIGRTMIN, POLL_IN);
		}
	}
	else
	{
		if(_MDrv_IR_GetKey(&u8Key, &u8RepeatFlag))
		{
#ifdef	NOT_USED_4_LGE
			_ulLastKeyPresentTime = _MDrv_IR_GetSystemTime();
#endif

			IR_PRINT("_MDrv_IR_ISR() -> %x, %x\n", u8Key, u8RepeatFlag);

			//temporary solution, need to implement ring buffer for this
			_TvLinkReceived.u8Data[0]	= u8Key;
			_TvLinkReceived.u8Flag  	= u8RepeatFlag;
			_TvLinkReceived.u8Valid 	= 1;

			if (IRDev.async_queue)
				wake_up_interruptible(&key_wait_q);
				//kill_fasync(&IRDev.async_queue, SIGRTMIN, POLL_IN);

#if defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) || defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERVA_BOARD_ATSC_1)
//	          if (0x08 == u8Key)
//	              _stop_get_key = 1;
#endif

		}
	}

#else	/* #ifdef IR_INCLUDE_TV_LINK_AND_WB_CODE */

#if (IR_MODE_SEL == IR_TYPE_SWDECODE_MODE)
    static unsigned long ulPreTime;
    if (_MDrv_IR_GetSystemTime() - ulPreTime > IR_TIMEOUT_CYC/1000) //timeout
    {
        _u32IRCount = 0;
    }

    if (_u32IRCount <IR_SWDECODE_MODE_BUF_LEN)
    {
        _u32IRData[_u32IRCount++] = ((REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & 0xF) << 16) | REG(REG_IR_SHOT_CNT_L);
    }
    ulPreTime = _MDrv_IR_GetSystemTime();
#endif

    if(_MDrv_IR_GetKey(&u8Key, &u8RepeatFlag))
    {
#ifdef	NOT_USED_4_LGE
        _ulLastKeyPresentTime = _MDrv_IR_GetSystemTime();
#endif

        IR_PRINT("_MDrv_IR_ISR() -> %x, %x\n", u8Key, u8RepeatFlag);

        //temporary solution, need to implement ring buffer for this
        _KeyReceived.u8Key = u8Key;
        _KeyReceived.u8Flag = u8RepeatFlag;
        _KeyReceived.u8Valid = 1;

       	if (IRDev.async_queue)
			wake_up_interruptible(&key_wait_q);
    		//kill_fasync(&IRDev.async_queue, SIGRTMIN, POLL_IN);
    }

#endif	/* #ifdef IR_INCLUDE_TV_LINK_AND_WB_CODE */

    //Disabled already by SA_INTERRUPT at initial
    //MsOS_EnableInterrupt(E_FIQ_IR);

    return IRQ_HANDLED;
}


#if (IR_MODE_SEL == IR_TYPE_FULLDECODE_MODE)
//-------------------------------------------------------------------------------------------------
/// Get IR key. It is a non-blocking function.
/// @param pu8Key  \b IN: Return IR key value.
/// @param pu8Flag \b IN: Return IR repeat code.
///
/// @return TRUE:  Success
/// @return FALSE: No key or repeat key is faster than the specified period
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_IR_GetKey(U8 *pu8Key, U8 *pu8Flag)
{
    unsigned long i;
    BOOL bRet;
	U32 dummy;

    if(REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)
    {
        bRet = FALSE;
    }
    else
    {
        *pu8Key = REG(REG_IR_CKDIV_NUM_KEY_DATA) >> 8;
        REG(REG_IR_FIFO_RD_PULSE) = 1; //read

    	for(i=0;i<20/*5*/;i++);   // Delay

		dummy = REG(REG_IR_SHOT_CNT_H_FIFO_STATUS);

        if ( REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_RPT_FLAG )
            *pu8Flag = 1;
        else
            *pu8Flag = 0;
        bRet = TRUE;
    }

#ifdef	IR_INCLUDE_REPEAT_TIME_TERM
    if(bRet)
    {
        if ( (_u8PrevKeyCode != *pu8Key) || (!*pu8Flag) )
        {
            _ePrevKeyProperty = E_IR_KEY_PROPERTY_INIT;
        }

        i = _MDrv_IR_GetSystemTime();
        if( _ePrevKeyProperty == E_IR_KEY_PROPERTY_INIT)
        {
            _u8PrevKeyCode     = *pu8Key;
            _ulPrevKeyTime    = i;
            _ePrevKeyProperty  = E_IR_KEY_PROPERTY_1st;
        }
        else if(_ePrevKeyProperty == E_IR_KEY_PROPERTY_1st)
        {
            if( (i - _ulPrevKeyTime) > _u32_1stDelayTimeMs)
            {
                _ulPrevKeyTime = i;
                _ePrevKeyProperty  = E_IR_KEY_PROPERTY_FOLLOWING;
            }
            else
            {
                bRet = FALSE;
            }
        }
        else //E_IR_KEY_PROPERTY_FOLLOWING
        {
            if( (i - _ulPrevKeyTime) > _u32_2ndDelayTimeMs)
            {
                _ulPrevKeyTime = i;
            }
            else
            {
                bRet = FALSE;
            }
        }
    }
#endif	//#ifdef	IR_INCLUDE_REPEAT_TIME_TERM
#if 0
#if (IR_TYPE_SEL != IR_TYPE_CUS03_DTV)
    if(bRet)
    {
        *pu8Key = _MDrv_IR_ParseKey(*pu8Key);
    }
#endif
#endif

    // Empty the FIFO
    for(i=0; i<8; i++)
    {
        U8 u8Garbage;

        if(REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)
            break;

        u8Garbage = REG(REG_IR_CKDIV_NUM_KEY_DATA) >> 8;
        REG(REG_IR_FIFO_RD_PULSE) = 1; //read
    }

    return bRet;
}

#elif (IR_MODE_SEL == IR_TYPE_RAWDATA_MODE)
//-------------------------------------------------------------------------------------------------
/// Get IR key.
/// @param pu8Key  \b IN: Return IR key value.
/// @param pu8Flag \b IN: Return IR repeat code.
///
/// @return TRUE:  Success
/// @return FALSE: Failure
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_IR_GetKey(U8 *pu8Key, U8 *pu8Flag)
{
    BOOL bRet = FALSE;

    u32 i, j;
    for (j=0; j<IR_RAW_DATA_NUM; j++)
    {
        if ( REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)  // check FIFO empty
            break;

        _u8IRRawModeBuf[_u8IRRawModeCount++] = REG(REG_IR_CKDIV_NUM_KEY_DATA) >> 8;
        REG(REG_IR_FIFO_RD_PULSE) = 1; //read

    	for(i=0;i<5;i++); //Delay

        if(_u8IRRawModeCount == IR_RAW_DATA_NUM)
        {
            _u8IRRawModeCount = 0;
            if( (_u8IRRawModeBuf[0]==IR_HEADER_CODE0) &&
                (_u8IRRawModeBuf[1]==IR_HEADER_CODE1) )
            {
                if(_u8IRRawModeBuf[2] == (U8)(~_u8IRRawModeBuf[3]))
                {
#if (IR_TYPE_SEL != IR_TYPE_CUS03_DTV)
                    *pu8Key = _MDrv_IR_ParseKey(_u8IRRawModeBuf[2]);
#else
                    *pu8Key = _u8IRRawModeBuf[2];
#endif
                    //TODO: Implement repeat code later.
                    *pu8Flag = 0;
                    bRet = TRUE;
                    break;
                }
            }
        }
    }

    // Empty the FIFO
    for(i=0; i<8; i++)
    {
        U8 u8Garbage;

        if(REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)
            break;

        u8Garbage = REG(REG_IR_CKDIV_NUM_KEY_DATA) >>8;
        REG(REG_IR_FIFO_RD_PULSE) = 1; //read
    }

    return bRet;
}


#elif(IR_MODE_SEL == IR_TYPE_SWDECODE_MODE)
//-------------------------------------------------------------------------------------------------
/// Get IR key.
/// @param pu8Key  \b IN: Return IR key value.
/// @param pu8Flag \b IN: Return IR repeat code.
///
/// @return TRUE:  Success
/// @return FALSE: Failure
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_IR_GetKey(U8 *pu8Key, U8 *pu8Flag)
{
    BOOL bRet = FALSE;
    U32 u8Byte, u8Bit;
    U8 u8IRSwModeBuf[IR_RAW_DATA_NUM];
    U32 *pu32IRData = NULL;

    for( u8Byte=0; u8Byte<IR_RAW_DATA_NUM; u8Byte++)
    {
       u8IRSwModeBuf[u8Byte] = 0;
    }

    if (_u32IRCount< 3+IR_RAW_DATA_NUM*8)
        return FALSE; //not complete yet

    DEBUG_IR(printf("_u32IRCount=%d", _u32IRCount));
    for( u8Byte=0; u8Byte<_u32IRCount; u8Byte++)
    {
       DEBUG_IR(printf(" %d", _u32IRData[u8Byte]));
    }

    if( _u32IRData[0] > IR_HDC_LOB && _u32IRData[1] > IR_OFC_LOB+IR_LG01H_LOB && _u32IRData[1] < IR_OFC_UPB+IR_LG01H_UPB )
    {
        pu32IRData = &_u32IRData[2];
        DEBUG_IR(printf(" H1 "));
    }
    else if( _u32IRData[1] > IR_HDC_LOB && _u32IRData[2] > IR_OFC_LOB+IR_LG01H_LOB && _u32IRData[2] < IR_OFC_UPB+IR_LG01H_UPB )
    {
        pu32IRData = &_u32IRData[3];
        DEBUG_IR(printf(" H2 "));
    }
    else
    {
        DEBUG_IR(printf(" invalid leader code\n"));
        bRet = FALSE;
        goto done;
    }

    for( u8Byte=0; u8Byte<IR_RAW_DATA_NUM; u8Byte++)
    {
        for( u8Bit=0; u8Bit<8; u8Bit++)
        {
            u32 u32BitLen = pu32IRData[u8Byte*8+u8Bit];
            u8IRSwModeBuf[u8Byte] >>= 1;

            if( u32BitLen > IR_LG0_LOB && u32BitLen < IR_LG0_UPB ) //0
            {
                u8IRSwModeBuf[u8Byte] |= 0x00;
            }
            else if (u32BitLen > IR_LG1_LOB && u32BitLen < IR_LG1_UPB) //1
            {
                u8IRSwModeBuf[u8Byte] |= 0x80;
            }
            else
            {
                DEBUG_IR(printf(" invalid waveform\n"));
                bRet = FALSE;
                goto done;
            }
        }
    }

    if(u8IRSwModeBuf[0] == IR_HEADER_CODE0)
    {
        if(u8IRSwModeBuf[1] == IR_HEADER_CODE1)
        {
            if(u8IRSwModeBuf[2] == (u8)~u8IRSwModeBuf[3])
            {
                *pu8Key = u8IRSwModeBuf[2];
                *pu8Flag = 0;
                bRet = TRUE;
                goto done;
            }
        }
    }

    DEBUG_IR(printf(" invalid data\n"));
    bRet = FALSE;

done:
    _u32IRCount = 0;
    return bRet;
}
#endif


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// Initialize IR timing and enable IR.
/// @return None
//-------------------------------------------------------------------------------------------------
static unsigned int u32IRIntRegister;
void MDrv_IR_Init(void)
{
    u32 i;
	int result;

#if (IR_MODE_SEL == IR_TYPE_RAWDATA_MODE)
    _u8IRRawModeCount = 0;
#endif

#if (IR_MODE_SEL == IR_TYPE_SWDECODE_MODE)
    _u32IRCount = 0;
#endif

    REG(REG_IR_CTRL) = IR_TIMEOUT_CHK_EN |
                       IR_INV            |
                       IR_RPCODE_EN      |
                       IR_LG01H_CHK_EN   |
                       IR_DCODE_PCHK_EN  |
                       IR_CCODE_CHK_EN   |
                       IR_LDCCHK_EN      |
                       IR_EN;

    _MDrv_IR_Timing();

    REG(REG_IR_CCODE) = ((u16)IR_HEADER_CODE1<<8) | IR_HEADER_CODE0;
    REG(REG_IR_SEPR_BIT_FIFO_CTRL) = 0xF00;
    REG(REG_IR_GLHRM_NUM) = 0x804;

#if (IR_MODE_SEL==IR_TYPE_FULLDECODE_MODE)
    REG(REG_IR_GLHRM_NUM) |= (0x3 <<12);
#elif (IR_MODE_SEL==IR_TYPE_RAWDATA_MODE)
    REG(REG_IR_GLHRM_NUM) |= (0x2 <<12);
#else
    REG(REG_IR_GLHRM_NUM) |= (0x1 <<12);
    REG(REG_IR_SEPR_BIT_FIFO_CTRL) |= 0x2 <<12;
#endif


#if((IR_MODE_SEL==IR_TYPE_RAWDATA_MODE) || (IR_MODE_SEL==IR_TYPE_FULLDECODE_MODE))
    // Empty the FIFO
    for(i=0; i<8; i++)
    {
        U8 u8Garbage;

        if(REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)
            break;

        u8Garbage = REG(REG_IR_CKDIV_NUM_KEY_DATA) >>8;
        REG(REG_IR_FIFO_RD_PULSE) = 1; //read
    }
#endif

    _u32_1stDelayTimeMs = 0;
    _u32_2ndDelayTimeMs = 0;
    _ePrevKeyProperty = E_IR_KEY_PROPERTY_INIT;


#ifdef IR_INCLUDE_TV_LINK_AND_WB_CODE
    memset(&_TvLinkReceived, 0 , sizeof(_TvLinkReceived) );
#else
    memset(&_KeyReceived, 0 , sizeof(_KeyReceived) );
#endif	/* #ifdef IR_INCLUDE_TV_LINK_AND_WB_CODE */

	if(0 == u32IRIntRegister) {
    result = request_irq(E_FIQ_IR, _MDrv_IR_ISR, SA_INTERRUPT, "IR", NULL);
	    if (result) {
        IR_PRINT("IR IRQ registartion ERROR\n");
			return -EBUSY;
	}

		u32IRIntRegister= 1;
	} else {
		disable_irq(E_FIQ_IR);
		enable_irq(E_FIQ_IR);
    }

    //enable_irq(E_FIQ_IR);

}


//-------------------------------------------------------------------------------------------------
/// Set the IR delay time to recognize a valid key.
/// @param  u32_1stDelayTimeMs \b IN: Set the delay time to get the 1st key.
/// @param  u32_2ndDelayTimeMs \b IN: Set the delay time to get the following keys.
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_IR_SetDelayTime(u32 u32_1stDelayTimeMs, u32 u32_2ndDelayTimeMs)
{
//TBD
    //u32 u32OldIntr;

    //u32OldIntr = MsOS_DisableAllInterrupts();

    _u32_1stDelayTimeMs = u32_1stDelayTimeMs;
    _u32_2ndDelayTimeMs = u32_2ndDelayTimeMs;

    //MsOS_RestoreAllInterrupts(u32OldIntr);
}


#ifndef IR_INCLUDE_TV_LINK_AND_WB_CODE
//-------------------------------------------------------------------------------------------------
/// Get IR key. It is a non-blocking function.
/// @param pu8Key  \b IN: Return IR key value.
/// @param pu8Flag \b IN: Return IR repeat code.
///
/// @return TRUE:  Success
/// @return FALSE: No key.
//-------------------------------------------------------------------------------------------------
BOOL MDrv_IR_GetKey(U8 *pu8Key, U8 *pu8Flag)
{
    //eCos
    #if 0
    //u32 u32OldIntr;
    //MsOS_DiableAllInterrupts(u32OldIntr); //need ???

    //bRet = _MDrv_IR_GetKey(pu8Key, pu8Flag);

    //MsOS_RestoreAllInterrupts(u32OldIntr);
    #endif

    if (_KeyReceived.u8Valid)
    {
        *pu8Key = _KeyReceived.u8Key;
        *pu8Flag = _KeyReceived.u8Flag ;
         memset(&_KeyReceived, 0 , sizeof(_KeyReceived) );
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}
#endif

//-------------------------------------------------------------------------------------------------
/// Get IR key. It is a non-blocking function.
/// @param pu8Key  \b IN: Return IR key value.
/// @param pu8Flag \b IN: Return IR repeat code.
///
/// @return TRUE:  Success
/// @return FALSE: No key.
//-------------------------------------------------------------------------------------------------
BOOL MDrv_IR_GetKeyOfTVLink(U8 *pu8Data, U8 *pu8Flag)
{
	BOOL	retVal = FALSE;

#ifdef IR_INCLUDE_TV_LINK_AND_WB_CODE
	U32 	size2copy = 1;

    //eCos
    #if 0
    //u32 u32OldIntr;
    //MsOS_DiableAllInterrupts(u32OldIntr); //need ???

    //bRet = _MDrv_IR_GetKey(pu8Key, pu8Flag);

    //MsOS_RestoreAllInterrupts(u32OldIntr);
    #endif

    if (_TvLinkReceived.u8Valid)
	{
		retVal = TRUE;

		//size2copy = (_TvLinkReceived.u8Flag > 1) ? IR_TV_LINK_DATA_NUM : 1;
		if( _TvLinkReceived.u8Flag == (_IR_READ_TYPE_WB_CODE - 1) )
		{
			size2copy = IR_WB_CODE_DATA_NUM;
		}
		else if( _TvLinkReceived.u8Flag == (_IR_READ_TYPE_TV_LINK - 1) )
		{
			size2copy = IR_TV_LINK_DATA_NUM;
		}

		memcpy( pu8Data, _TvLinkReceived.u8Data, size2copy );

#if 0
		printk("CHECK0(%d): %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					size2copy,
				 	pu8Data[0], pu8Data[1], pu8Data[2], pu8Data[3], pu8Data[4],
					pu8Data[5], pu8Data[6], pu8Data[7], pu8Data[8], pu8Data[9]);
#endif

		*pu8Flag = _TvLinkReceived.u8Flag ;
		memset(&_TvLinkReceived, 0 , sizeof(_TvLinkReceived) );
	}
#endif	/* #ifdef IR_INCLUDE_TV_LINK_AND_WB_CODE */

	return retVal;
}


#ifdef	NOT_USED_4_LGE
//-------------------------------------------------------------------------------------------------
/// Return the time that IR key present. It is a non-blocking function.
/// @return Last key present time.
//-------------------------------------------------------------------------------------------------
unsigned long MDrv_IR_GetLastKeyTime(void)
{
    return _ulLastKeyPresentTime;
}
#endif	/* #ifdef	NOT_USED_4_LGE */

//-------------------------------------------------------------------------------------------------
/// Translate from IR key to internal key.
/// @param  u8Key  \b IN: IR key value.
/// @return translated internal IR key.
//-------------------------------------------------------------------------------------------------
U8 MDrv_IR_ParseKey(U8 u8Key)
{
    return _MDrv_IR_ParseKey(u8Key);
}

void MDrv_IR_EnableIR(U8 bEnable)
{
    bIRPass = !bEnable;

    if (bEnable)
    {
		int result;
		if(0 == u32IRIntRegister) {
        result = request_irq(E_FIQ_IR, _MDrv_IR_ISR, SA_INTERRUPT, "IR", NULL);
			if (result) {
				IR_PRINT("IR IRQ registartion ERROR\n");
				return -EBUSY;
			}

			u32IRIntRegister= 1;
		} else {
			disable_irq(E_FIQ_IR);
			enable_irq(E_FIQ_IR);
		}
    }
    else
    {
        disable_irq(E_FIQ_IR);
        //free_irq(E_FIQ_IR, NULL);
    }
}


#ifdef	IR_INCLUDE_TV_LINK_AND_WB_CODE
//-------------------------------------------------------------------------------------------------
/// Get IR key on WB Code mode.
/// @param pu8Key  \b IN: Return IR key value.
/// @param pu8Flag \b IN: Return IR repeat code.
///
/// @return TRUE:  Success
/// @return FALSE: Failure
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_IR_GetKeyOfWBCode(U8 *pu8Key, U8 *pu8Flag)
{
	BOOL	bRet = FALSE;
	U32 	dummy;
	U32  	i, j;
	BOOL	bRepeat;

	for (j=0; j<IR_RAW_DATA_NUM; j++)
	{
		if ( REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)  // check FIFO empty
		break;

		_u8IRRawModeBuf[_u8IRRawModeCount++] = REG(REG_IR_CKDIV_NUM_KEY_DATA) >> 8;
		REG(REG_IR_FIFO_RD_PULSE) = 1; //read

		for(i=0; i < 20/*5*/;i++); //Delay

		dummy = REG(REG_IR_SHOT_CNT_H_FIFO_STATUS);
		bRepeat = ( REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_RPT_FLAG ) ? TRUE : FALSE ;

		if(_u8IRRawModeCount == IR_RAW_DATA_NUM)
		{
			_u8IRRawModeCount = 0;

			if( (_u8IRRawModeBuf[0]==IR_HEADER_CODE0) && (_u8IRRawModeBuf[1]==IR_HEADER_CODE1) )
			{
				if(_u8IRRawModeBuf[2] == (U8)(~_u8IRRawModeBuf[3]))
				{
					pu8Key[0] = _u8IRRawModeBuf[2];
					//*pu8Flag = (_IR_READ_TYPE_SINGLE - 1);
					*pu8Flag = 	( (bRepeat) ? ( _IR_READ_TYPE_REPEAT - 1) : (_IR_READ_TYPE_SINGLE - 1) );
					bRet = TRUE;

					//printk("IR CODE[%02x] %d\n", pu8Key[0], bRepeat );
					break;
				}
			}
			else if( _u8IRRawModeBuf[0] == 0x1F /* CMD1 */ )
			{
				pu8Key[0] = _u8IRRawModeBuf[0];
				pu8Key[1] = _u8IRRawModeBuf[1];
				pu8Key[2] = _u8IRRawModeBuf[2];
				pu8Key[3] = _u8IRRawModeBuf[3];
				*pu8Flag = (_IR_READ_TYPE_WB_CODE - 1);
				bRet = TRUE;

				printk("IR CODE[%2x %2x %2x %2x]\n", pu8Key[0], pu8Key[1], pu8Key[2], pu8Key[3] );
				break;
			}
		}
	}

#ifdef	IR_INCLUDE_REPEAT_TIME_TERM
	if( (bRet) && ((*pu8Flag) < _IR_READ_TYPE_REPEAT) )
	{
		unsigned long i;

		if ( (_u8PrevKeyCode != pu8Key[0]) || (!*pu8Flag) )
		{
			_ePrevKeyProperty = E_IR_KEY_PROPERTY_INIT;
		}

		i = _MDrv_IR_GetSystemTime();
		if( _ePrevKeyProperty == E_IR_KEY_PROPERTY_INIT)
		{
			_u8PrevKeyCode  	= pu8Key[0];
			_ulPrevKeyTime   	= i;
			_ePrevKeyProperty	= E_IR_KEY_PROPERTY_1st;
		}
		else if(_ePrevKeyProperty == E_IR_KEY_PROPERTY_1st)
		{
			if( (i - _ulPrevKeyTime) > _u32_1stDelayTimeMs)
			{
				_ulPrevKeyTime = i;
				_ePrevKeyProperty  = E_IR_KEY_PROPERTY_FOLLOWING;
			}
			else
			{
				bRet = FALSE;
			}
		}
		else //E_IR_KEY_PROPERTY_FOLLOWING
		{
			if( (i - _ulPrevKeyTime) > _u32_2ndDelayTimeMs)
			{
				_ulPrevKeyTime = i;
			}
			else
			{
				bRet = FALSE;
			}
		}
	}
#endif	//#ifdef	IR_INCLUDE_REPEAT_TIME_TERM

	// Empty the FIFO
	for(i=0; i<8; i++)
	{
		U8 u8Garbage;

		if(REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)
			break;

		u8Garbage = REG(REG_IR_CKDIV_NUM_KEY_DATA) >>8;
		REG(REG_IR_FIFO_RD_PULSE) = 1; //read
	}

	return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Get IR key on TV LINK mode.
/// @param pu8Key  \b IN: Return IR key value.
/// @param pu8Flag \b IN: Return IR repeat code.
///
/// @return TRUE:  Success
/// @return FALSE: Failure
//-------------------------------------------------------------------------------------------------
static BOOL _MDrv_IR_GetKeyOfTVLink(U8 *pu8Key, U8 *pu8Flag)
{
	BOOL bRet = FALSE;

	static U8 u8CodeBuf[IR_TV_LINK_DATA_NUM];

	//printk("GetKeyOfTVLink %d(%4d)\n", _u32IRCount, _u32IRData[_u32IRCount - 1] );

	if ( _u32IRCount == 1 )
	{
		if( ! (_u32IRData[0] > IR_HDC_LOB) )
		{
			// 처음으로 초기화
			_u32IRCount = 0;
			_u32IRAllCount = 0;
		}
	}
	else if( _u32IRCount == 2 )
	{
		if( (_u32IRData[1] > IR_OFC_RP_LOB+IR_LG01H_LOB) && (_u32IRData[1] < IR_OFC_RP_UPB+IR_LG01H_UPB) )
		{
			pu8Key[0] = _LSB[u8CodeBuf[2]];
			*pu8Flag = (_IR_READ_TYPE_REPEAT - 1);
			bRet = TRUE;

			goto done;
		}
		else if( ! ( (_u32IRData[1] > IR_OFC_LOB+IR_LG01H_LOB) && (_u32IRData[1] < IR_OFC_UPB+IR_LG01H_UPB) ) )
		{
#if 0
			if( _u32IRData[1] > IR_HDC_LOB )
			{
				//
				_u32IRData[0] = _u32IRData[1];
				_u32IRCount = 1;
				_u32IRAllCount = 2;
			}
			else
#endif
			{
				// 처음으로 초기화
				_u32IRCount = 0;
				_u32IRAllCount = 0;
			}
		}
	}
	else if( _u32IRCount > 2 )
	{
		U32	index = (_u32IRCount - 3) / 8;
		U32 u32BitLen;

		if( (_u32IRCount - 3) % 8 == 0 )
		{
			u8CodeBuf[index] = 0;
		}
		else
		{
			u8CodeBuf[index] <<= 1;
		}

		u32BitLen = _u32IRData[ _u32IRCount - 1 ];

		//printk("BIT[%d:%d] (%8d): %8d %8d %8d %8d\n", index, (_u32IRCount - 3) % 8, u32BitLen, IR_LG0_LOB, IR_LG0_UPB, IR_LG1_LOB, IR_LG1_UPB );

		if( /*u32BitLen > IR_LG0_LOB &&*/ u32BitLen < IR_LG0_UPB) //0
		{
			//u8CodeBuf[index] |= 0x00;
			//u8CodeBuf[index] &= ~(0x80);
			//u8CodeBuf[index] &= ~(0x01);
		}
		else if ( /*u32BitLen > IR_LG1_LOB && */ u32BitLen < IR_LG1_UPB) //1
		{
			//u8CodeBuf[index] |= 0x80;
			u8CodeBuf[index] |= 0x01;
		}
		else
		{
			//DEBUG_IR(printk(" invalid waveform\n"));
			printk("BIT[%d:%d] (%8d): %2x\n", index, (_u32IRCount - 3) % 8, u32BitLen, u8CodeBuf[index] );

			//printk("BIT (%8d): %8d %8d %8d %8d\n", u32BitLen,IR_LG0_LOB, IR_LG0_UPB, IR_LG1_LOB, IR_LG1_UPB );
			bRet = FALSE;
			goto done;
		}
	}


	if( _u32IRCount < 2 + (IR_RAW_DATA_NUM * 8))
	{
		return FALSE;	//not complete yet
	}
	else if( _u32IRCount == 2 + (IR_RAW_DATA_NUM * 8) )
	{

		if(	(u8CodeBuf[0] == _LSB[IR_HEADER_CODE0]) && (u8CodeBuf[1] == _LSB[IR_HEADER_CODE1]) )
		{
			if(u8CodeBuf[2] == (u8)~u8CodeBuf[3])
			{
				pu8Key[0] = _LSB[u8CodeBuf[2]];
				*pu8Flag = (_IR_READ_TYPE_SINGLE - 1);
				bRet = TRUE;

				//printk("4 BYTE [%2x %2x %2x %2x]\n", u8CodeBuf[0], u8CodeBuf[1], u8CodeBuf[2], u8CodeBuf[3] );
				goto done;
			}
		}
		else if( 	(u8CodeBuf[0] == _LSB[0x7F]) && (u8CodeBuf[1] == _LSB[0x5F])
				 &&	(u8CodeBuf[2] == _LSB[0x3F]) && (u8CodeBuf[3] == _LSB[0x1F])	)
		{
			printk("TV LINK START\n");
			//printk("4 BYTE [%2x %2x %2x %2x]\n", u8CodeBuf[0], u8CodeBuf[1], u8CodeBuf[2], u8CodeBuf[3] );

			bRet = FALSE;
			goto done;
		}
#if 0	// not support WD CODE when TV LINK MODE
		else if( ( _IsWBCodeMode ) && (u8CodeBuf[0] == _LSB[0x1F]) )
		{
			pu8Key[0] = _LSB[u8CodeBuf[0]];
			pu8Key[1] = _LSB[u8CodeBuf[1]];
			pu8Key[2] = _LSB[u8CodeBuf[2]];
			pu8Key[3] = _LSB[u8CodeBuf[3]];
			*pu8Flag = (_IR_READ_TYPE_WB_CODE - 1);
			bRet = TRUE;

			printk("IR CODE[%2x %2x %2x %2x]\n", pu8Key[0], pu8Key[1], pu8Key[2], pu8Key[3] );
			goto done;
		}
#endif
		else
		{
			return FALSE;	//not complete yet
		}
	}
	else if( _u32IRCount < 2 + (IR_TV_LINK_DATA_NUM * 8) )
	{
		//printk(" %d\n", _u32IRCount );
		return FALSE;	//not complete yet
	}
	else
	{
		memcpy( pu8Key, u8CodeBuf, IR_TV_LINK_DATA_NUM );
		*pu8Flag = (_IR_READ_TYPE_TV_LINK - 1);
		bRet = TRUE;

#if 0
		{
			int i;

			printk("LINK(%3d) %02x", pu8Key[0] , pu8Key[0] );
			for( i =1; i< IR_TV_LINK_DATA_NUM; i++ )
			{
				printk(" %02x", pu8Key[i] );
			}
			printk("\n");
		}
#endif

		goto done;
	}

	//DEBUG_IR(printk(" invalid data\n"));
	bRet = FALSE;

done:

#ifdef	IR_INCLUDE_REPEAT_TIME_TERM
	if( (bRet) && ((*pu8Flag) < _IR_READ_TYPE_REPEAT) )
	{
		unsigned long i;

		if ( (_u8PrevKeyCode != pu8Key[0]) || (!*pu8Flag) )
		{
			_ePrevKeyProperty = E_IR_KEY_PROPERTY_INIT;
		}

		i = _MDrv_IR_GetSystemTime();
		if( _ePrevKeyProperty == E_IR_KEY_PROPERTY_INIT)
		{
			_u8PrevKeyCode  	= pu8Key[0];
			_ulPrevKeyTime   	= i;
			_ePrevKeyProperty	= E_IR_KEY_PROPERTY_1st;
		}
		else if(_ePrevKeyProperty == E_IR_KEY_PROPERTY_1st)
		{
			if( (i - _ulPrevKeyTime) > _u32_1stDelayTimeMs)
			{
				_ulPrevKeyTime = i;
				_ePrevKeyProperty  = E_IR_KEY_PROPERTY_FOLLOWING;
			}
			else
			{
				bRet = FALSE;
			}
		}
		else //E_IR_KEY_PROPERTY_FOLLOWING
		{
			if( (i - _ulPrevKeyTime) > _u32_2ndDelayTimeMs)
			{
				_ulPrevKeyTime = i;
			}
			else
			{
				bRet = FALSE;
			}
		}
	}
#endif	//#ifdef	IR_INCLUDE_REPEAT_TIME_TERM

	_u32IRCount = 0;
	_u32IRAllCount = 0;

	return bRet;
}
#endif	//#ifdef	IR_INCLUDE_TV_LINK_AND_WB_CODE

void MDrv_IR_Config( U16 type, U16 value )
{
#ifdef	IR_INCLUDE_TV_LINK_AND_WB_CODE
	U32 	i;
	U8  	currIRMode = _u8IRMode;
	U8  	nextIRMode;


	if( type > 0 )
	{	// TV LINK MODE
		_IsTvLinkMode = (value) ? TRUE : FALSE;
	}
	else
	{	// WB CODE MODE
		_IsWBCodeMode = (value) ? TRUE : FALSE;
	}


	if( (_IsTvLinkMode) || (_IsWBCodeMode) )
	{
		nextIRMode = ((_IsTvLinkMode) ? IR_MODE_TV_LINK : IR_MODE_WB_CODE);
	}
	else
	{
		nextIRMode = IR_MODE_NORMAL;
	}

	IR_PRINT("MDrv_IR_Config(%d, %d): %d %d(%d %d)\n", type, value,
					currIRMode, nextIRMode, _IsTvLinkMode, _IsWBCodeMode );

	if( currIRMode != nextIRMode )
	{
		_u8IRMode = nextIRMode;

		//disable_irq(E_FIQ_IR);

		REG(REG_IR_CTRL) = IR_TIMEOUT_CHK_EN |
		                   IR_INV            |
		                   IR_RPCODE_EN      |
		                   IR_LG01H_CHK_EN   |
		                   IR_DCODE_PCHK_EN  |
		                   IR_CCODE_CHK_EN   |
		                   IR_LDCCHK_EN      |
		                   IR_EN;

		_MDrv_IR_Timing();

		REG(REG_IR_CCODE) = ((u16)IR_HEADER_CODE1<<8) | IR_HEADER_CODE0;
		REG(REG_IR_SEPR_BIT_FIFO_CTRL) = 0xF00;
		REG(REG_IR_GLHRM_NUM) = 0x804;


		if( nextIRMode == IR_MODE_WB_CODE )
		{	/* IR_TYPE_RAWDATA_MODE 형태 */

			_u8IRRawModeCount = 0;

			REG(REG_IR_GLHRM_NUM) |= (0x2 <<12);

			// Empty the FIFO
			for(i=0; i<8; i++)
			{
				U8 u8Garbage;

				if(REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)
					break;

				u8Garbage = REG(REG_IR_CKDIV_NUM_KEY_DATA) >>8;
				REG(REG_IR_FIFO_RD_PULSE) = 1; //read
			}
		}
		else if( nextIRMode == IR_MODE_TV_LINK )
		{	/* IR_TYPE_SWDECODE_MODE 형태 */

			_u32IRCount = 0;

			REG(REG_IR_GLHRM_NUM) |= (0x1 <<12);
			REG(REG_IR_SEPR_BIT_FIFO_CTRL) |= 0x2 <<12;
		}
		else
		{
#if (IR_MODE_SEL==IR_TYPE_FULLDECODE_MODE)
			REG(REG_IR_GLHRM_NUM) |= (0x3 <<12);
#elif (IR_MODE_SEL==IR_TYPE_RAWDATA_MODE)
			REG(REG_IR_GLHRM_NUM) |= (0x2 <<12);
#else
			REG(REG_IR_GLHRM_NUM) |= (0x1 <<12);
			REG(REG_IR_SEPR_BIT_FIFO_CTRL) |= 0x2 <<12;
#endif

#if((IR_MODE_SEL==IR_TYPE_RAWDATA_MODE) || (IR_MODE_SEL==IR_TYPE_FULLDECODE_MODE))
			// Empty the FIFO
			for(i=0; i<8; i++)
			{
				U8 u8Garbage;

				if(REG(REG_IR_SHOT_CNT_H_FIFO_STATUS) & IR_FIFO_EMPTY)
					break;

				u8Garbage = REG(REG_IR_CKDIV_NUM_KEY_DATA) >>8;
				REG(REG_IR_FIFO_RD_PULSE) = 1; //read
			}
#endif
		}

		_ePrevKeyProperty = E_IR_KEY_PROPERTY_INIT;

		memset(&_TvLinkReceived, 0 , sizeof(_TvLinkReceived) );

		//enable_irq(E_FIQ_IR);
	}

#endif	//#ifdef	IR_INCLUDE_TV_LINK_AND_WB_CODE
}

