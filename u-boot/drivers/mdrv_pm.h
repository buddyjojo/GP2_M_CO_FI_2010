////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_PM_H_
#define _MDRV_PM_H_

//#include "datatype.h"

#ifdef _MDRV_PM_H_
#define INTERFACE
#else
#define INTERFACE extern
#endif

//PM_CMD opcode
#define WRITE_CMD   0
#define READ_CMD    1
#define SLEEP_CMD   2

//select sleep mode
#define SLEEP_MODE  0
#define DEEP_SLEEP_MODE  1

//enable wakeup source
#define IR_WAKEUP       (1<<0)
#define CEC_WAKEUP      (1<<1)
#define GPIO5_WAKEUP    (1<<2)
#define GPIO6_WAKEUP    (1<<3)
#define KEYPAD_WAKEUP   (1<<4)
#define EASYSYNC_WAKEUP (1<<5)
#define SYNC_WAKEUP     (1<<6)
#define RTC_WAKEUP      (1<<7)

#define DVI_DET_WAKEUP      (1<<10)
#define DVI_DET2_WAKEUP      (1<<11)


typedef struct
{
    U8 Preamble    : 2;
    U8 Opcode    : 6;

} PM_CMD;


#if 1
extern U8 MDrv_PM_RegWrite( U16 u16Addr, U8 u8Data );
extern U8 MDrv_PM_RegRead( U16 u16Addr );


/******************************************************************************/
///set 1 bit data to M4 Lite register
///@param u16Addr \b IN  register address
///@param bBit \b IN  bit value to set
///@param u8BitPos \b IN  which bit to set
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
/******************************************************************************/
#define MDrv_PM_RegBitWrite( u16Addr, bBit, u8BitPos ) \
    (MDrv_PM_RegWrite(u16Addr, \
            (bBit) ? (MDrv_PM_RegRead( u16Addr )|(u8BitPos)) :  \
                    (MDrv_PM_RegRead( u16Addr ) & ~(u8BitPos))  \
            ))


/******************************************************************************/
///get 1 bit data from M4 Lite register
///@param u16Addr \b IN  register address
///@param u8BitPos \b IN  which bit to read
///@return
///- 8 bits (1 byte) value for read with bit mask
///- FALSE: Fail
/******************************************************************************/
#define MDrv_PM_RegBitRead( u16Addr, u8BitPos ) \
    (MDrv_PM_RegRead( u16Addr ) & (u8BitPos))


void MAdp_WriteByteMask( U16 u16Reg, U8 u8Val, U32 u8Msk );
U32 MAdp_Read3Byte( U16 u16Reg );

#endif
#undef INTERFACE

#endif // MDRV_PM_H

