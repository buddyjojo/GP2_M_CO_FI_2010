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

//////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvSerFlash.h
/// @brief  Serail Flash Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_SERFLASH_H_
#define _DRV_SERFLASH_H_


//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------
/// Serial Flash Type
#define SERFLASH_TYPE_NONE          0
//#define SERFLASH_TYPE_PM25LV040     1
//#define SERFLASH_TYPE_SST25LF040    2
//#define SERFLASH_TYPE_ST25P32       3
//#define SERFLASH_TYPE_MX25L1605     4
#define SERFLASH_TYPE_MX25L3205     5                                   // MXIC
#define SERFLASH_TYPE_MX25L6405     6                                   // MXIC
#define SERFLASH_TYPE_S25FL064A     10                                  // SPANSION
#define SERFLASH_TYPE_EN25B16       20
#define SERFLASH_TYPE_EN25B64       21

#define SERFLASH_TYPE               SERFLASH_TYPE_MX25L3205

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
#ifndef TRUE
    #define TRUE                        1
#endif

#ifndef FALSE
    #define FALSE                      0
#endif

/// Serial Flash information structure
typedef struct
{
    u32 u32AccessWidth;     ///< data access width in bytes
    u32 u32TotalSize;       ///< total size in bytes
    u32 u32SecNum;          ///< number of sectors
    u32 u32SecSize;         ///< sector size in bytes
} SerFlashInfo;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// Get the information of Serial Flash
/// @param  pFlashInfo \b OUT: the buffer to store flash information
/// @return None
//-------------------------------------------------------------------------------------------------
extern void MDrv_SerFlash_GetInfo(SerFlashInfo *pFlashInfo);

//-------------------------------------------------------------------------------------------------
/// Initialize Serial Flash
/// @return None
//-------------------------------------------------------------------------------------------------
extern void MDrv_SerFlash_Init(void);

//-------------------------------------------------------------------------------------------------
/// Read ID from Serial Flash
/// @param  pu8Data \b OUT: data ptr to store the read ID
/// @param  u32Size \b IN: size in Bytes
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_ReadID(u8 *pu8Data, u32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Read data from Serial Flash
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b OUT: data ptr to store the read data
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_Read(u32 u32Addr, u8 *pu8Data, u32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Erase all sectors in Serial Flash
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_EraseChip(void);

//-------------------------------------------------------------------------------------------------
/// Erase certain sectors in Serial Flash
/// @param  u32StartSec    \b IN: start sector
/// @param  u32EndSec      \b IN: end sector
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_EraseSec(u32 u32StartSec, u32 u32EndSec);

//-------------------------------------------------------------------------------------------------
/// Write data to Serial Flash
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b IN: data to be written
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_Write(u32 u32Addr, u8 *pu8Data, u32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Read data from Serial Flash
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b OUT: data ptr to store the read data
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_Read(u32 u32Addr, u8 *pu8Data, u32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Read data from Serial Flash via DMA
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b OUT: data ptr to store the read data
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_DMA(u32 u32Addr, u32 pu8Data, u32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Protect blocks in Serial Flash
/// @param  bEnable \b IN: TRUE/FALSE: enable/disable protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_WriteProtect(u8 bEnable);

//------- ------------------------------------------------------------------------------------------
/// Read Status Register in Serial Flash
/// @param  pu8StatusReg \b OUT: ptr to Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern u8 MDrv_SerFlash_ReadStatusReg(u8 *pu8StatusReg);


extern void * memcpy(void * dest,const void *src,size_t count);


#endif // _DRV_SERFLASH_H_

