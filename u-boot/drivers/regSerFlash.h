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

#define REG_SFSH_XIU_BASE           0xB4000000 // FLASH0
// #define REG_SFSH_XIU_BASE           0xB8000000 // FLASH1
#define SFSH_XIU_REG32(addr)        *((volatile MS_U32*)(REG_SFSH_XIU_BASE+addr*4))

// ISP_CMD
#define REG_ISP_BASE                0xBF001000
#define ISP_REG16(addr)             *((volatile MS_U32*)(REG_ISP_BASE+addr*4))

#define REG_ISP_PASSWORD            0x00 // ISP / XIU read / DMA mutual exclusive
#define REG_ISP_SPI_COMMAND         0x01
    #define CMD_WRITE_ENABLE                    0x06
    #define CMD_WRITE_DISABLE                   0x04
    #define CMD_READ_STATUS_REG                 0x05    // Read Status Register
    #define CMD_WRITE_STATUS_REG                0x01    // Write Status Register
    #define CMD_READ                            0x03
    #define CMD_FAST_READ                       0x0B

#define REG_ISP_SPI_ADDR_L          0x02 // A[15:0]
#define REG_ISP_SPI_ADDR_H          0x03 // A[23:16]
#define REG_ISP_SPI_WDATA           0x04
#define REG_ISP_SPI_RDATA           0x05
#define REG_ISP_SPI_CLKDIV          0x06 // clock = CPU clock / this div
    // SPI_CLK_DIV (chip-dependent)
    #define SPI_CLK_DIV2                        0x00000000 // BIT0
    #define SPI_CLK_DIV4                        0x00000004 // BIT2
    #define SPI_CLK_DIV8                        0x00000040 // BIT6
    #define SPI_CLK_DIV16                       0x00000080 // BIT7
    #define SPI_CLK_DIV32                       0x00000100 // BIT8
    #define SPI_CLK_DIV64                       0x00000200 // BIT9
    #define SPI_CLK_DIV128                      0x00000400 // BIT10

#define REG_ISP_DEV_SEL             0x07
    // SPI brand define (chip-dependent)
    #define BRAND_PMC                           0x00
    #define BRAND_NEXTFLASH                     0x01
    #define BRAND_ST                            0x02
    #define BRAND_SST                           0x03
    #define BRAND_ATMEL                         0x04
    #define MAX_SPI_BRAND                       0x05

#define REG_ISP_SPI_CECLR           0x08
#define REG_ISP_SPI_RDREQ           0x0C
#define REG_ISP_SPI_ENDIAN          0x0F
#define REG_ISP_SPI_RD_DATARDY      0x15
#define REG_ISP_SPI_WR_DATARDY      0x16
#define REG_ISP_SPI_WR_CMDRDY       0x17
#define REG_ISP_TRIGGER_MODE        0x2A
#define REG_ISP_CHIP_SEL            0x36
    #define SFSH_CHIP_SEC_MASK                  0x00FF
    #define SFSH_CHIP_SEL_RIU                   0x0080
    #define SFSH_CHIP_SEL_XIU                   0x0000


// PIU_DMA
//#if defined(CONFIG_TITANIA3) ||defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_GP2_DEMO1)
#define REG_PIU_BASE                0xBF207800
//#elif CONFIG_EUCLID
//#define REG_PIU_BASE                0xBF207800
//#else
//#define REG_PIU_BASE                0xBF807800
//#endif
#define PIU_REG16(addr)             *((volatile MS_U32*)(REG_PIU_BASE+addr*4))

#define REG_PIU_DMA_STATUS          0x10 // [1]done [2]busy [8:15]state
#define REG_PIU_SPI_CLK_SRC         0x26 // SPI clock source  [0]:gate  [1]:inv  [4:2]:clk_sel  000:Xtal clock 001:27MHz 010:36MHz 011:43.2MHz 100:54MHz 101:72MHz 110:86MHz 111:108MHz  [5]:0:xtal 1:clk_Sel
#define REG_PIU_DMA_SPISTART_L      0x70 // [15:0]
#define REG_PIU_DMA_SPISTART_H      0x71 // [23:16]
#define REG_PIU_DMA_DRAMSTART_L     0x72 // [15:0]  in unit of B; must be 8B aligned
#define REG_PIU_DMA_DRAMSTART_H     0x73 // [23:16]
#define REG_PIU_DMA_SIZE_L          0x74 // [15:0]  in unit of B; must be 8B aligned
#define REG_PIU_DMA_SIZE_H          0x75 // [23:16]
#define REG_PIU_DMA_CMD             0x76
    #define PIU_DMA_CMD_FIRE                    0x0001
    #define PIU_DMA_CMD_LE                      0x0000
    #define PIU_DMA_CMD_BE                      0x0020

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

#endif // _REG_SERFLASH_H_
