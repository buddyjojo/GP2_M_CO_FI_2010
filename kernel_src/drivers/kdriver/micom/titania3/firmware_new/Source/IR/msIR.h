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

#ifndef MS_IR_H
#define MS_IR_H

#define IR_TYPE_FULLDECODE_MODE 1
#define IR_TYPE_RAWDATA_MODE    2
#define IR_TYPE_SWDECODE_MODE   3

//------------------------------IR_TYPE_SEL-------------------------------------
#define IR_TYPE_OLD             0
#define IR_TYPE_NEW             1
#define IR_TYPE_MSTAR_DTV       2
#define IR_TYPE_MSTAR_RAW       3
#define IR_TYPE_RC_V16          4
#define IR_TYPE_CUS03_DTV       5
#define IR_TYPE_CUS4            6
#define IR_TYPE_DC_LWB1         7
#define IR_TYPE_DC_BN59         8
#define IR_TYPE_SZ_CH           9

#define IR_TYPE_SEL                     IR_TYPE_CUS03_DTV   // IR_TYPE_MSTAR_DTV // IR_TYPE_CUS03_DTV // IR_TYPE_NEW

#if (IR_TYPE_SEL == IR_TYPE_MSTAR_DTV)
#include "IR_MSTAR_DTV.h"
#elif (IR_TYPE_SEL == IR_TYPE_CUS03_DTV)
#define ENABLE_HDMI_CEC 1
#define USE_Q_MENU_MENU 0
#define ENABLE_TTX_OPTION_MENU  0
#include "IR_CUS03_DTV.h"
#endif

#ifndef IR_MODE_SEL
#define IR_MODE_SEL             IR_TYPE_FULLDECODE_MODE//IR_TYPE_RAWDATA_MODE//IR_TYPE_FULLDECODE_MODE
#endif

extern void MDrv_IR_SW_Isr(U8 *pkey);
extern BOOLEAN msIR_GetIRKeyCode(U8 *pkey, U8 *pu8flag);
extern void msIR_Initialize(U8 irclk_mhz);
extern U8 msIR_GetIrKeyData( void );
extern void msIR_Clear_FIFO(void);
extern void msIR_Write3Byte( U16 u16Regndex, U32 u32Value );
extern U8 msIR_ReadByte( U16 u16RegIndex );

#endif  //#ifndef MS_IR_H

