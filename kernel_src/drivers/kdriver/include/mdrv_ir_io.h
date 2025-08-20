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
/// Driver to initialize and access IR.
///     - Provide functions to initialize IR timing, and enable IR interrupt.
///     - Provide IR ISR.
///     - Provide IR callback function registration for AP.
///     - Provide function to get IR key.
///
/// \b [Example]
/// @code
///
/// // Initalize the IR in the boot time.
/// MDrv_IR_Init();
///
/// // *****************************************************************************
///
/// // Set the delay time of IR. First repeat key code is sent after one second.
/// // The following repeat key code is sent after 0.5 seconds.
/// MDrv_IR_SetDelayTime(1000, 500);
///
/// // Please refer to the following diagram. Assume that when users press and hold
/// // IR button, the repeat key is sent every 200ms.
/// // The 1st press is sent, and the return repeat code is 0.
/// // The 5th repeat key is sent because of the 1st delay time is 1000ms.
/// // The 8th repeat key is sent because of the 2nd delay time is 500ms, and
/// // the time between the 5th and the 8th repeat key is 600ms which is greater
/// // than 500ms.
/// // Note: Do not support RELEASE event.
///
/// @endcode
///
/// @image html IR_delay.JPG "IR delay time"
///
/// @code
/// // *****************************************************************************
///
/// // Set the callback function. The function MApi_IR_SetCallback is called if
/// // the IR interrupt is generated and the delay time setting is matched.
/// void MApi_IR_SetCallback(U8 *pu8Key, U8 *pu8Flg);
///
/// MDrv_IR_Set_Callback(MApi_IR_SetCallback);
///
/// // *****************************************************************************
///
/// // Polling & get the IR key directly. Users can call the MDrv_IR_GetKey to get
/// // the IR key if it returns TRUE.
/// U8 u8Key, u8Flg;
///
/// MDrv_IR_GetKey(&u8Key, &u8Flg);
///
/// @endcode
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_IR_IO_H_
#define _MDRV_IR_IO_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"
#include "mdrv_ir_st.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define IR_TYPE_FULLDECODE_MODE 1
#define IR_TYPE_RAWDATA_MODE    2
#define IR_TYPE_SWDECODE_MODE   3
//#define IR_MODE_SEL             IR_TYPE_SWDECODE_MODE

/*	to support time term of repeat key */
#define IR_INCLUDE_REPEAT_TIME_TERM

/*	to support TV LINK IR 	- added by LGE(dreamer@lge.com)	*/
#define IR_INCLUDE_TV_LINK_AND_WB_CODE
#define	IR_MODE_NORMAL 			0	//	NORMAL MODE
#define	IR_MODE_WB_CODE			1	//	WB CODE MODE(POWER ONLY MODE)
#define	IR_MODE_TV_LINK			2	//	TV LINK MODE

#define IR_WB_CODE_DATA_NUM		4	//	WB CODE DATA :  4 bytes
#define IR_TV_LINK_DATA_NUM		10	//	TV LINK DATA : 10 bytes
/*	to support TV LINK IR 	- added by LGE(dreamer@lge.com)		-end */



#define IR_IOC_MAGIC                'u'

#define MDRV_IR_INIT                _IO(IR_IOC_MAGIC, 0)
#define MDRV_IR_SET_DELAYTIME       _IOW(IR_IOC_MAGIC, 1, int)
#define MDRV_IR_GET_KEY             _IOW(IR_IOC_MAGIC, 2, int)
#define MDRV_IR_GET_LASTKEYTIME     _IOW(IR_IOC_MAGIC, 3, int)
#define MDRV_IR_PARSE_KEY           _IOW(IR_IOC_MAGIC, 4, int)
#define MDRV_IR_TEST                _IOW(IR_IOC_MAGIC, 5, int)
#define MDRV_IR_ENABLE_IR           _IOW(IR_IOC_MAGIC, 6, int)
#define MDRV_IR_WAKE_UP             _IOW(IR_IOC_MAGIC, 7, int)	// added by dreamer@lge.com
#define MDRV_IR_CONFIG              _IOW(IR_IOC_MAGIC, 8, int)	// added by dreamer@lge.com
#define IR_IOC_MAXNR                9

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

/* added by dreamer@lge.com */
typedef struct
{
	U32		u32Timeout;
	U8		u8KeyType;		//	2: REPEAT	1: SINGLE	0: INVALID
	U8		u8KeyValue;
	U8		u8IRType;		//	2: REPEAT	1: SINGLE	0: INVALID	3: WB CODE DATA	4: TV LINK DATA
#define	_IR_READ_TYPE_INVALID 	0
#define	_IR_READ_TYPE_SINGLE 	1
#define	_IR_READ_TYPE_REPEAT 	2
#define	_IR_READ_TYPE_WB_CODE	3
#define	_IR_READ_TYPE_TV_LINK	4
	U8		u8IRValue;
#ifdef	IR_INCLUDE_TV_LINK_AND_WB_CODE
	U8		u8TVLinkData[IR_TV_LINK_DATA_NUM];
#endif

}	IR_READ_PARAM_T;
/* added by dreamer@lge.com     -end */

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#endif // _MDRV_IR_IO_H_
