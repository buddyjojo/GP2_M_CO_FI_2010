////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
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

#ifndef DRV_GLOBAL_H
#define DRV_GLOBAL_H

#include "datatype.h"

/********************************************************************************/
/*                           Macro                                              */
/********************************************************************************/

typedef U8 MS_REG_INIT;

#define _RV1(addr, value)   (((addr) >> 8) & 0x3F), (U8)(addr), (U8)(value)
#define _RV2(addr, value)   0x40 + _RV1(addr, value), (U8)((value) >> 8)
#define _RV3(addr, value)   0x40 + _RV2(addr, value), (U8)((value) >> 16)
#define _RV4(addr, value)   0x40 + _RV3(addr, value), (U8)((value) >> 24)

#define _RVM1(addr, value, mask) (((addr) >> 8) & 0x3F), (U8)(addr), (U8)(value), (U8)(mask)
#define _RVM2(addr, value, mask) 0x40 + _RVM1(addr, value, mask), (U8)((value) >> 8), (U8)((mask) >> 8)
#define _RVM3(addr, value, mask) 0x40 + _RVM2(addr, value, mask), (U8)((value) >> 16), (U8)((mask) >> 16)
#define _RVM4(addr, value, mask) 0x40 + _RVM3(addr, value, mask), (U8)((value) >> 24), (U8)((mask) >> 24)

#define _END_OF_TBL2_       0xFF, 0xFF

#if defined(__C51__)

#include "Analog_DataType.h"
#include "Analog_DataType2.h"

/******************************************************************************/
/*                   Macros                                                   */
/******************************************************************************/

#define MDrv_ReadByte( u16Reg )  XBYTE[u16Reg]

#define MDrv_Read3Byte( u16Reg ) MDrv_ReadU24LE( (U32 xdata *) (u16Reg) )
#define MDrv_Read4Byte( u16Reg ) MDrv_ReadU32LE( (U32 xdata *) (u16Reg) )

#define MDrv_WriteByte( u16Reg, u8Value )   (XBYTE[u16Reg] = u8Value)

#define MDrv_Write3Byte( u16Reg, u32Value ) MDrv_WriteU24LE( (U32 xdata *) (u16Reg), u32Value )
#define MDrv_Write4Byte( u16Reg, u32Value ) MDrv_WriteU32LE( (U32 xdata *) (u16Reg), u32Value )

#define MDrv_WriteByteMask( u16Reg, u8Value, u8Mask )    \
    (XBYTE[u16Reg] = (XBYTE[u16Reg] & ~(u8Mask)) | ((u8Value) & (u8Mask)))

#define MDrv_ReadRegBit( u16Reg, u8BitPos ) \
    (MDrv_ReadByte( u16Reg ) & (u8BitPos))

#define MDrv_WriteRegBit( u16Reg, bBit, u8BitPos ) \
    (XBYTE[u16Reg] = (bBit) ? (XBYTE[u16Reg] | (u8BitPos)) : (XBYTE[u16Reg] & ~(u8BitPos)))

/******************************************************************************/
/*                   Function Prototypes                                      */
/******************************************************************************/

#ifdef DRV_GLOBAL_C
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE U16  MDrv_Read2Byte( U16 u16Reg );
INTERFACE void MDrv_Write2Byte( U16 u16Reg, U16 u16Val );

INTERFACE U16  MDrv_ReadU16LE( U16 *pLoc );
INTERFACE U32  MDrv_ReadU24LE( U32 *pLoc );
INTERFACE U32  MDrv_ReadU32LE( U32 *pLoc );
INTERFACE void MDrv_WriteU16LE( U16 *pLoc, U16 u16Val );
INTERFACE void MDrv_WriteU24LE( U32 *pLoc, U32 u32Val );
INTERFACE void MDrv_WriteU32LE( U32 *pLoc, U32 u32Val );

INTERFACE void MDrv_WriteRegTbl ( void *pRegTable );
#define MDrv_WriteRegTbl2( pRegTable )  MDrv_WriteRegTbl( pRegTable )

#undef INTERFACE

#else   // ! __C51__

/******************************************************************************/
/*                   Function Prototypes                                      */
/******************************************************************************/

#ifdef DRV_GLOBAL_C
#define INTERFACE
#else
#define INTERFACE extern
#endif

#define MDrv_ReadByte( u16Reg )                                         \
    (__builtin_constant_p( u16Reg ) && ((u16Reg) < 0x4000) ?              \
        (((u16Reg) & 0x01) ? (RIU[(u16Reg) - 1] >> 8) : RIU[u16Reg]) :  \
        __MDrv_ReadByte( u16Reg ))

#define MDrv_Read2Byte( u16Reg )                                                        \
    (__builtin_constant_p( u16Reg ) && !((u16Reg) & 0x01) && ((u16Reg) < 0x4000) ?      \
        RIU[u16Reg] : __MDrv_Read2Byte( u16Reg ))

#define MDrv_Read3Byte( u16Reg )    __MDrv_Read3Byte( u16Reg )
#define MDrv_Read4Byte( u16Reg )    __MDrv_Read4Byte( u16Reg )

#define MDrv_ReadRegBit( u16Reg, u8Mask )                           \
        (RIU[(u16Reg) & 0xFFFE] & (((u16Reg) & 1) ? ((u8Mask) << 8) : (u8Mask)))

#define MDrv_WriteRegBit( u16Reg, bEnable, u8Mask ) do {                                \
        if ( __builtin_expect( (u16Reg & 0x01), 0) )                                    \
            RIU[(u16Reg) - 1] = (bEnable) ? (RIU[(u16Reg) - 1] |  (U16)((u8Mask) << 8)) : \
                                            (RIU[(u16Reg) - 1] & ~(U16)((u8Mask) << 8));  \
        else                                                                            \
            RIU[u16Reg] = (bEnable) ? (RIU[u16Reg] |  (U8)(u8Mask)) :                   \
                                      (RIU[u16Reg] & ~(U8)(u8Mask));                    \
    } while (0)

#define MDrv_WriteByte( u16Reg, u8Val ) do {                                            \
        if ( __builtin_constant_p( u16Reg ) && (u16Reg) < 0x4000 )                      \
        {                                                                               \
            if ( ((u16Reg) & 0x01) )                                                    \
                RIU[(u16Reg) - 1] = (RIU[(u16Reg) - 1] & 0x00FF) + (U16)((u8Val) << 8); \
            else                                                                        \
                RIU[u16Reg] = (RIU[u16Reg] & 0xFF00) + (U16)(u8Val);                    \
        }                                                                               \
        else                                                                            \
            __MDrv_WriteByte( u16Reg, u8Val );                                          \
    } while (0)

#define MDrv_Write2Byte( u16Reg, u16Val ) do {                                          \
        if ( __builtin_constant_p( u16Reg ) && (u16Reg) < 0x4000 )                      \
        {                                                                               \
            if ( ((u16Reg) & 0x01) )                                                    \
            {                                                                           \
                RIU[(u16Reg) - 1] = (RIU[(u16Reg) - 1] & 0x00FF) + (U16)((u16Val) << 8);\
                RIU[(u16Reg) + 1] = (RIU[(u16Reg) + 1] & 0xFF00) + (U16)((u16Val) >> 8);\
            }                                                                           \
            else                                                                        \
                RIU[u16Reg] = u16Val;                                                   \
        }                                                                               \
        else                                                                            \
            __MDrv_Write2Byte( u16Reg, u16Val );                                        \
    } while (0)

#define MDrv_Write3Byte( u16Reg, u32Val )   __MDrv_Write3Byte( u16Reg, u32Val )

#define MDrv_Write4Byte( u16Reg, u32Val ) do {                                          \
        if ( __builtin_constant_p( u16Reg ) && !((u16Reg) & 0x01) && (u16Reg) < 0x4000 )\
        {                                                                               \
            RIU[u16Reg] = (U16)(u32Val);                                                \
            RIU[(u16Reg) + 2] = (U16)((u32Val) >> 16);                                  \
        }                                                                               \
        else                                                                            \
            __MDrv_Write4Byte( u16Reg, u32Val );                                        \
    } while (0)

#define MDrv_WriteByteMask( u16Reg, u8Val, u8Msk ) do {                                 \
        if ( (u16Reg) & 0x01 )                                                          \
            RIU[(u16Reg) - 1] = (RIU[(u16Reg) - 1] & ( ~((u8Msk) << 8))) | (((u8Val) & (u8Msk)) << 8);\
        else                                                                            \
            RIU[u16Reg] = (RIU[u16Reg] & (~(u8Msk))) | ((u8Val) & (u8Msk));             \
    } while (0)

INTERFACE U8   __MDrv_ReadByte( U16 u16Reg );
INTERFACE U16  __MDrv_Read2Byte( U16 u16Reg );
INTERFACE U32  __MDrv_Read3Byte( U16 u16Reg );
INTERFACE U32  __MDrv_Read4Byte( U16 u16Reg );

INTERFACE void __MDrv_WriteByte( U16 u16Reg, U8 u8Val );
INTERFACE void __MDrv_Write2Byte( U16 u16Reg, U16 u16Val );
INTERFACE void __MDrv_Write3Byte( U16 u16Reg, U32 u32Val );
INTERFACE void __MDrv_Write4Byte( U16 u16Reg, U32 u32Val );

INTERFACE void MDrv_WriteRegTbl ( const MS_REG_INIT *pRegTable );
#define MDrv_WriteRegTbl2( pRegTable )  MDrv_WriteRegTbl( pRegTable )

#undef INTERFACE

#endif

#endif /* DRV_GLOBAL_H */
