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


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/kernel.h>
#include "mdrv_types.h"
#include "mhal_regMIU.h"
#include "mhal_MIU.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MIU_HAL_ERR(x, args...)        {printk(x, ##args);}
#define HAL_MIU_SSC_DBG(x)      // x

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static MS_U32 _gMIU_MapBase = 0xBF200000;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
#if 0
void HAL_MIU_SetIOMapBase(MS_U32 u32Base)
{
    _gMIU_MapBase = u32Base;
      printk("MVOP _gMIU_MapBase= %lx\n", _gMIU_MapBase);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Global Function
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_ReadByte
/// @brief \b Function  \b Description: read 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b MS_U8
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_U8 HAL_MIU_ReadByte(MS_U32 u32RegAddr)
{
    return ((volatile MS_U8*)(_gMIU_MapBase))[(u32RegAddr << 1) - (u32RegAddr & 1)];
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_Read4Byte
/// @brief \b Function  \b Description: read 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b MS_U16
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_U16 HAL_MIU_Read2Byte(MS_U32 u32RegAddr)
{
    return ((volatile MS_U16*)(_gMIU_MapBase))[u32RegAddr];
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_Read4Byte
/// @brief \b Function  \b Description: read 4 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b MS_U32
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_U32 HAL_MIU_Read4Byte(MS_U32 u32RegAddr)
{
    return (HAL_MIU_Read2Byte(u32RegAddr) | HAL_MIU_Read2Byte(u32RegAddr+2) << 16);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_WriteByte
/// @brief \b Function  \b Description: write 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_WriteByte(MS_U32 u32RegAddr, MS_U8 u8Val)
{
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile MS_U8*)(_gMIU_MapBase))[(u32RegAddr << 1) - (u32RegAddr & 1)] = u8Val;
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_Write2Byte
/// @brief \b Function  \b Description: write 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u16Val : 2 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Write2Byte(MS_U32 u32RegAddr, MS_U16 u16Val)
{
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile MS_U16*)(_gMIU_MapBase))[u32RegAddr] = u16Val;
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_Write4Byte
/// @brief \b Function  \b Description: write 4 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u32Val : 4 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Write4Byte(MS_U32 u32RegAddr, MS_U32 u32Val)
{
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    HAL_MIU_Write2Byte(u32RegAddr, u32Val & 0x0000FFFF);
    HAL_MIU_Write2Byte(u32RegAddr+2, u32Val >> 16);
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_BDMA_WriteByte
/// @brief \b Function  \b Description: write 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_WriteRegBit(MS_U32 u32RegAddr, MS_U8 u8Mask, MS_BOOL bEnable)
{
    MS_U8 u8Val = HAL_MIU_ReadByte(u32RegAddr);
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    u8Val = HAL_MIU_ReadByte(u32RegAddr);
    u8Val = (bEnable) ? (u8Val | u8Mask) : (u8Val & ~u8Mask);
    HAL_MIU_WriteByte(u32RegAddr, u8Val);
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MIU_Protect()
/// @brief \b Function \b Description:  Enable/Disable MIU Protection mode
/// @param u8Blockx        \b IN     : MIU Block to protect (0 ~ 3)
/// @param *pu8ProtectId   \b IN     : Allow specified client IDs to write
/// @param u32Start        \b IN     : Starting address
/// @param u32End          \b IN     : End address
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Protect(
                          MS_U8    u8Blockx,
                          MS_U8    *pu8ProtectId,
                          MS_U32   u32Start,
                          MS_U32   u32End,
                          MS_BOOL  bSetFlag
                         )
{
    MS_U16 u16RegAddr;
    MS_U16 u16Data;
    MS_U8  u8Data;

    // Incorrect Block ID
    if(u8Blockx >= 4)
        return FALSE;

    u8Data = 1 << u8Blockx;

    u16RegAddr = MIU_PROTECT0_ID0 + (6 * u8Blockx);
    if (u8Blockx!=0)
    {
        u16RegAddr += 2;
    }

    // Disable MIU protect
    HAL_MIU_WriteRegBit(MIU_PROTECT_EN, u8Data, 0);

    if ( bSetFlag )
    {
        // Protect IDs
        HAL_MIU_WriteByte(u16RegAddr + 0, pu8ProtectId[0]);
        HAL_MIU_WriteByte(u16RegAddr + 1, pu8ProtectId[1]);
        u16RegAddr += 2;
        if (u8Blockx==0)
        {
            HAL_MIU_WriteByte(u16RegAddr + 2, pu8ProtectId[2]);
            HAL_MIU_WriteByte(u16RegAddr + 3, pu8ProtectId[3]);
            u16RegAddr += 2;
        }

        // Start Address
        u16Data = (MS_U16)(u32Start >> 12); //4k-bytes unit
        HAL_MIU_Write2Byte(u16RegAddr, u16Data);

        // End Address
        u16Data = (MS_U16)( u32End >> 12); //4k-bytes unit
        HAL_MIU_Write2Byte(u16RegAddr + 2, u16Data);

        // Enable MIU protect
        HAL_MIU_WriteRegBit(MIU_PROTECT_EN, u8Data,1);
    }

    return TRUE;
}

