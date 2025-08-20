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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mhal_chiptop_reg.h
/// @brief  Chip Top Registers Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MHAL_PMSLEEP_REG_H_
#define _MHAL_PMSLEEP_REG_H_


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// PM_SLEEP_XIU assignment
//------------------------------------------------------------------------------
#define REG_PMSLEEP_BASE                0xBF001C00
#define PMSLEEP_REG(addr)               (*((volatile u32*)(REG_PMSLEEP_BASE + ((addr)<<2))))

#define REG_CKG_MCU                          0x0020
#define PMSLEEP_CKG_MCU_MASK                 BMASK(1:0)
#define PMSLEEP_CKG_MCU_DIS                  _BIT(0)
#define PMSLEEP_CKG_MCU_INV                  _BIT(1)
#define PMSLEEP_CKG_MCU_SRC_MASK                 BMASK(5:2)
#define PMSLEEP_CKG_MCU_SRC_170                  BITS(5:2, 0)
#define PMSLEEP_CKG_MCU_SRC_160                  BITS(5:2, 1)
#define PMSLEEP_CKG_MCU_SRC_144                  BITS(5:2, 2)
#define PMSLEEP_CKG_MCU_SRC_123                  BITS(5:2, 3)
#define PMSLEEP_CKG_MCU_SRC_108                  BITS(5:2, 4)
#define PMSLEEP_CKG_MCU_SRC_MEM_CLK              BITS(5:2, 5)
#define PMSLEEP_CKG_MCU_SRC_MEM_CLK_DIV2         BITS(5:2, 6)
#define PMSLEEP_CKG_MCU_SRC_INT_XTALI            BITS(5:2, 7)
#define PMSLEEP_CKG_MCU_SRC_EXT_XTALI_DIV8       BITS(5:2, 8)
#define PMSLEEP_CKG_MCU_SRC_24                   BITS(5:2, 9)
#define PMSLEEP_CKG_MCU_SRC_INT_XTALI_DIV_1M     BITS(5:2, 10)
#define PMSLEEP_CKG_MCU_SRC_EXT_XTALI_DIV16      BITS(5:2, 11)
#define PMSLEEP_CKG_MCU_SRC_EXT_XTALI_DIV2       BITS(5:2, 12)
#define PMSLEEP_CKG_MCU_SRC_EXT_XTALI_DIV4       BITS(5:2, 13)
#define PMSLEEP_CKG_MCU_SRC_240                  BITS(5:2, 14)
#define PMSLEEP_CKG_MCU_SRC_192                  BITS(5:2, 15)
#define PMSLEEP_CKG_SW_MCU_CLK               _BIT(7)

#define REG_CKG_SPI                          REG_CKG_MCU
#define PMSLEEP_CKG_SPI_MASK                 BMASK(9:8)
#define PMSLEEP_CKG_SPI_DIS                  _BIT(8)
#define PMSLEEP_CKG_SPI_INV                  _BIT(9)
#define PMSLEEP_CKG_SPI_SRC_MASK                 BMASK(13:10)
#define PMSLEEP_CKG_SPI_SRC_INT_XTALI            BITS(13:10, 0)
#define PMSLEEP_CKG_SPI_SRC_27                   BITS(13:10, 1)
#define PMSLEEP_CKG_SPI_SRC_36                   BITS(13:10, 2)
#define PMSLEEP_CKG_SPI_SRC_43                   BITS(13:10, 3)
#define PMSLEEP_CKG_SPI_SRC_54                   BITS(13:10, 4)
#define PMSLEEP_CKG_SPI_SRC_72                   BITS(13:10, 5)
#define PMSLEEP_CKG_SPI_SRC_86                   BITS(13:10, 6)
#define PMSLEEP_CKG_SPI_SRC_108                  BITS(13:10, 7)
#define PMSLEEP_CKG_SPI_SRC_EXT_XTALI_DIV8       BITS(13:10, 8)
#define PMSLEEP_CKG_SPI_SRC_EXT_XTALI_DIV_1M     BITS(13:10, 9)
#define PMSLEEP_CKG_SPI_SRC_INT_XTALI_DIV_1M     BITS(13:10, 10)
#define PMSLEEP_CKG_SPI_SRC_EXT_XTALI_DIV16      BITS(13:10, 11)
#define PMSLEEP_CKG_SPI_SRC_EXT_XTALI_DIV2       BITS(13:10, 12)
#define PMSLEEP_CKG_SPI_SRC_EXT_XTALI_DIV4       BITS(13:10, 13)
#define PMSLEEP_CKG_SPI_SRC_EXT_XTALI            BITS(13:10, 14)
#define PMSLEEP_CKG_SPI_SRC_24                   BITS(13:10, 15)
#define PMSLEEP_CKG_SW_SPI_CLK               _BIT(14)


#endif // _MHAL_PMSLEEP_REG_H_


