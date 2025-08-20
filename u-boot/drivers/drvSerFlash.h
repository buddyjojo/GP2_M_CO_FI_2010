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

// Serial Flash Type
/*
#define SERFLASH_TYPE_NONE          0
// #define SERFLASH_TYPE_PM25LV040     1
// #define SERFLASH_TYPE_SST25LF040    2
// #define SERFLASH_TYPE_ST25P32       3
// #define SERFLASH_TYPE_MX25L1605     4
#define SERFLASH_TYPE_MX25L3205     5                                   // MXIC
#define SERFLASH_TYPE_MX25L6405     6                                   // MXIC
#define SERFLASH_TYPE_MX25L12805    7                                   // MXIC
#define SERFLASH_TYPE_S25FL064A     10                                  // SPANSION
#define SERFLASH_TYPE_EN25B16       20
#define SERFLASH_TYPE_EN25B64       21

#define SERFLASH_TYPE               SERFLASH_TYPE_MX25L6405 // SERFLASH_TYPE_MX25L3205
*/
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
#ifndef TRUE
    #define TRUE    1
#endif

#ifndef FALSE
    #define FALSE   0
#endif

//typedef uint8_t   CHAR;
//typedef uint8_t   BYTE;
//typedef uint16_t  WORD;
//typedef uint32_t  DWORD;
typedef int       BOOL;

/// data type unsigned char, data length 1 byte
typedef unsigned char               MS_U8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              MS_U16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int                MS_U32;                             // 4 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;                              // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;                             // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int                  MS_S32;                             // 4 bytes

/// Serial Flash information structure
typedef struct
{
    MS_U32 u32AccessWidth;     ///< data access width in bytes
    MS_U32 u32TotalSize;       ///< total size in bytes
    MS_U32 u32BlockNum;        ///< number of sectors
    MS_U32 u32BlockSize;       ///< sector size in bytes
} SerFlashInfo;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// Get system information
/// @return Chip Revision ID
//-------------------------------------------------------------------------------------------------
MS_U8 MDrv_SerFlash_GetChipRev(void);

//-------------------------------------------------------------------------------------------------
/// Get the information of Serial Flash
/// @param  pFlashInfo \b OUT: the buffer to store flash information
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_SerFlash_GetInfo(SerFlashInfo *pFlashInfo);

//-------------------------------------------------------------------------------------------------
/// Initialize Serial Flash
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_SerFlash_Init(void);

//-------------------------------------------------------------------------------------------------
/// Read ID from Serial Flash
/// @param  pu8Data \b OUT: data ptr to store the read ID
/// @param  u32Size \b IN: size in Bytes
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_ReadID(MS_U8 *pu8Data, MS_U32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Read data from Serial Flash
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b OUT: data ptr to store the read data
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_Read(MS_U32 u32Addr, MS_U8 *pu8Data, MS_U32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Erase all sectors in Serial Flash
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_EraseChip(void);

//-------------------------------------------------------------------------------------------------
/// Erase certain sectors in Serial Flash
/// @param  u32StartSec    \b IN: start sector
/// @param  u32EndSec      \b IN: end sector
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_EraseBlock(MS_U32 u32StartBlock, MS_U32 u32EndBlock);

//-------------------------------------------------------------------------------------------------
/// Write data to Serial Flash
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b IN: data to be written
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_Write(MS_U32 u32Addr, MS_U8 *pu8Data, MS_U32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Read data from Serial Flash
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b OUT: data ptr to store the read data
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_Read(MS_U32 u32Addr, MS_U8 *pu8Data, MS_U32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Read data from Serial Flash via DMA
/// @param  u32Addr \b IN: start address (4-B aligned)
/// @param  pu8Data \b OUT: data ptr to store the read data
/// @param  u32Size \b IN: size in Bytes (4-B aligned)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_DMA(MS_U32 u32Addr, MS_U32 pu8Data, MS_U32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Protect blocks in Serial Flash
/// @param  bEnable \b IN: TRUE/FALSE: enable/disable protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_WriteProtect(BOOL bEnable);

//------- ------------------------------------------------------------------------------------------
/// Read Status Register in Serial Flash
/// @param  pu8StatusReg \b OUT: ptr to Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SerFlash_ReadStatusReg(MS_U8 *pu8StatusReg);

#endif // _DRV_SERFLASH_H_

