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
/// file    regSerFlash.h
/// @brief  Serial Flash Register Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _REG_SERFLASH_H_
#define _REG_SERFLASH_H_

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------

// !!! Uranus Serial Flash Notes: !!!
//  - The clock of DMA & Read via XIU operations must be < 3*CPU clock
//  - The clock of DMA & Read via XIU operations are determined by only REG_ISP_CLK_SRC; other operations by REG_ISP_CLK_SRC only
//  - DMA program can't run on DRAM, but in flash ONLY
//  - DMA from SPI to DRAM => size/DRAM start/DRAM end must be 8-B aligned


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

// XIU_ADDR
//typedef unsigned int                u32;

#ifdef CONFIG_Titania3
#define REG_SFSH_XIU_BASE           0xB4000000 // FLASH0
//#define REG_SFSH_XIU_BASE           0xB8000000 // FLASH1
#endif
#ifdef CONFIG_Titania2
#define REG_SFSH_XIU_BASE           0xB4000000 // FLASH0
//#define REG_SFSH_XIU_BASE           0xB8000000 // FLASH1
#endif

#define SFSH_XIU_REG32(addr)        *((volatile u32*)(REG_SFSH_XIU_BASE+addr*4))

//ISP_CMD

#ifdef CONFIG_Titania3
#define REG_ISP_BASE                0xBF001000
#endif
#ifdef CONFIG_Titania2
#define REG_ISP_BASE                0xBF801000
#endif

#define ISP_REG16(addr)             *((volatile u32*)(REG_ISP_BASE+addr*4))

#define REG_ISP_PASSWORD            0x00 //ISP / XIU read / DMA mutual exclusive
#define REG_ISP_SPI_COMMAND         0x01
#define REG_ISP_SPI_ADDR_L          0x02 //A[15:0]
#define REG_ISP_SPI_ADDR_H          0x03 //A[23:16]
#define REG_ISP_SPI_WDATA           0x04
#define REG_ISP_SPI_RDATA           0x05
#define REG_ISP_SPI_CLKDIV          0x06 //clock = CPU clock / this div
#define REG_ISP_DEV_SEL             0x07
#define REG_ISP_SPI_CECLR           0x08
#define REG_ISP_SPI_RDREQ           0x0C
#define REG_ISP_SPI_ENDIAN          0x0F
#define REG_ISP_SPI_RD_DATARDY      0x15
#define REG_ISP_SPI_WR_DATARDY      0x16
#define REG_ISP_SPI_WR_CMDRDY       0x17
#define REG_ISP_TRIGGER_MODE        0x2a
#define REG_ISP_CHIP_SEL            0x36
    #define SFSH_CHIP_SEC_MASK                  0x00FF
    #define SFSH_CHIP_SEL_RIU                   0x0080
    #define SFSH_CHIP_SEL_XIU                   0x0000


//PIU_DMA

#ifdef CONFIG_Titania3
#define REG_BDMA_BASE                0xBF207800

#define BDMA_REG16(addr)             *((volatile u32*)(REG_BDMA_BASE+(addr<<2)))

#define REG_BDMA_CH0_TRIGGER            0x00
#define REG_BDMA_CH0_STATUS             0x01
#define REG_BDMA_CH0_SRC_DST_SEL        0x02
#define REG_BDMA_CH0_CONFIG             0x03
#define REG_BDMA_CH0_SRC_LO_ADDR        0x04
#define REG_BDMA_CH0_SRC_HI_ADDR        0x05
#define REG_BDMA_CH0_DST_LO_ADDR        0x06
#define REG_BDMA_CH0_DST_HI_ADDR        0x07
#define REG_BDMA_CH0_LO_SIZE            0x08
#define REG_BDMA_CH0_HI_SIZE            0x09
#endif

#ifdef CONFIG_Titania2
#define REG_PIU_BASE                0xBF807800

#define PIU_REG16(addr)             *((volatile u32*)(REG_PIU_BASE+addr*4))

#define REG_PIU_DMA_STATUS          0x10 //[1]done [2]busy [8:15]state
#define REG_PIU_SPI_CLK_SRC         0x26 //SPI clock source  [0]:gate  [1]:inv  [4:2]:clk_sel  000:Xtal clock 001:27MHz 010:36MHz 011:43.2MHz 100:54MHz 101:72MHz 110:86MHz 111:108MHz  [5]:0:xtal 1:clk_Sel
#define REG_PIU_DMA_SPISTART_L      0x70 //[15:0]
#define REG_PIU_DMA_SPISTART_H      0x71 //[23:16]
#define REG_PIU_DMA_DRAMSTART_L     0x72 //[15:0]  in unit of B; must be 8B aligned
#define REG_PIU_DMA_DRAMSTART_H     0x73 //[23:16]
#define REG_PIU_DMA_SIZE_L          0x74 //[15:0]  in unit of B; must be 8B aligned
#define REG_PIU_DMA_SIZE_H          0x75 //[23:16]
#define REG_PIU_DMA_CMD             0x76
    #define PIU_DMA_CMD_FIRE                    0x0001
    #define PIU_DMA_CMD_LE                      0x0000
    #define PIU_DMA_CMD_BE                      0x0020
#endif

#define    DRAM_MIU_0                          0x40
#define    DRAM_MIU_1                          0x41
#define    PATTERN_SEARCH                      0x02
#define    CRC32                               0x03
#define    PATTERN_FILL                        0x44
#define    SPI_FLASH                           0x45
#define    NA_RESERVED                         0x06
#define    VDMCU                               0x07
#define    DSP                                 0x08
#define    TSP_MCU                             0x09


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

#endif // _REG_SERFLASH_H_
