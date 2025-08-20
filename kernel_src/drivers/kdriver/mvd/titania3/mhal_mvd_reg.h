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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    Mhal_mvd_reg.h
/// @brief  MVD HAL layer Driver Interface for Register
/// @author MStar Semiconductor Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include "mdrv_types.h"

#ifndef _REG_MVD_H_
#define _REG_MVD_H_


#define ACTIVE_FORMAT


#define REG_MVD_BASE               0xBF202200
#define MVD_REG(addr)              (*((volatile U32*)(REG_MVD_BASE + ((addr)<<2))))

#define MEMALIGN(align, size)               ((((size)+(align)-1)/(align))*(align))

//------------------------------------------------------------------------------
// MVD Reg
//------------------------------------------------------------------------------
#define MVD_STAT_CTRL             0x0
#define MVD_COMMAND               0x1
#define MVD_ARG1_ARG0             0x2
#define MVD_ARG3_ARG2             0x3

#define SD_YSize                0x65400     //720*576
#define HD_YSize                0x1FE000    //1920*1088
#define MVD_PollingCount        0x200000


//BIU_MCU_MVDCTL(0x100) bit define
#define  MVD_RST                  0x0001
#define  MVD_BOOT_EN              0x0002
#define  MVD_INT_CLR              0x0004
#define  MVD_MCU_MODE             0x0008

//BIU_MCU_MVDSTATUS(0x101) bit define
#define  MVD_BOOT_DONE            0x0001
#define  MVD_CMD_RDY              0x0002

#define VPU_REG_BASE                0xBF200600
#define VPU_REG(addr)               (*((volatile U32*)(VPU_REG_BASE + ((addr)<<2))))

#define MIU0_REG_BASE             	0xBF202400
#define MIU1_REG_BASE             	0xBF200c00
#define MIU0_REG( group )             (*((volatile U32*)(MIU0_REG_BASE + ((group)<<2) )))
#define MIU1_REG( group )              (*((volatile U32*)(MIU1_REG_BASE + ((group)<<2))))


#if 0
#define REG_MHG_BASE                0xBF200600
#define MHG_REG(addr)               (*((volatile U32*)(REG_MHG_BASE + ((addr)<<2))))
#define MHG_REG_L(addr)             (*((volatile U8*)(REG_MHG_BASE + ((addr)<<2))))
#define MHG_REG_H(addr)             (*((volatile U8*)(REG_MHG_BASE + ((addr)<<2) + 1)))
#endif

#if 0
#define REG_SVD_BASE               0xBF203600
#define SVD_REG(addr)              (*((volatile U32*)(REG_SVD_BASE + ((addr)<<2))))
#endif

//------------------------------------------------------------------------------
// MIU mask Reg
//------------------------------------------------------------------------------
#define MIU0_REG_GROUP_0                 ( 0x0023)
#define MIU0_REG_GROUP_1                 ( 0x0033)
#define MIU0_REG_GROUP_2                 ( 0x0043)
#define MIU0_REG_GROUP_3                 ( 0x0053)
#define MIU1_REG_GROUP_0                 ( 0x0023)
#define MIU1_REG_GROUP_1                 ( 0x0033)
#define MIU1_REG_GROUP_2                 ( 0x0043)
#define MIU1_REG_GROUP_3                 ( 0x0053)

#define MIU_HVD_RW      (_BIT(10)|_BIT(11))
#define MIU_MVD_RW      (_BIT(5)|_BIT(6))
#define MIU_VPU_RW_DQ      (_BIT(7)|_BIT(8))
#define MIU_VPU_RW_I      _BIT(8)

//------------------------------------------------------------------------------
// VPU Reg
//------------------------------------------------------------------------------
#define VPU_REG_DCU_BASE     0x0040
#define VPU_REG_QMEM_ADDR_H     0x0041
#define VPU_REG_QMEM_ADDR_L     0x0042
#define VPU_REG_QMEM_DATA_W_H     0x0043
#define VPU_REG_QMEM_DATA_W_L     0x0044
#define VPU_REG_QMEM_DATA_R_H     0x0045
#define VPU_REG_QMEM_DATA_R_L     0x0046
#define VPU_REG_QMEM_RW_FIRE     0x0047
    #define VPU_FIRE_READ     0x00f0
    #define VPU_FIRE_WRITE     0x01f0
    #define VPU_FIRE_READY     0x8000
#define VPU_REG_MASK_RG_ACK_TIMEOUT     0x0049    // ??? io ack timeout
    #define VPU_REG_TIMEOUT_ENABLE     _BIT(0)
#define VPU_REG_CPU_RST_FINISH      0x0058
#define VPU_REG_HI_MBOX0_L                     ( 0x005b)
#define VPU_REG_HI_MBOX0_H                     ( 0x005c)
#define VPU_REG_HI_MBOX1_L                     ( 0x005d)
#define VPU_REG_HI_MBOX1_H                     ( 0x005e)
#define VPU_REG_HI_MBOX_SET                    ( 0x005f)
    #define VPU_REG_HI_MBOX0_SET   _BIT(0)
    #define VPU_REG_HI_MBOX1_SET   _BIT(1)
#define VPU_REG_RISC_MBOX_CLR                  ( 0x0067)
    #define VPU_REG_RISC_MBOX0_CLR    _BIT(0)
    #define VPU_REG_RISC_MBOX1_CLR    _BIT(1)
    #define VPU_REG_RISC_ISR_CLR          _BIT(2)
    #define VPU_REG_RISC_ISR_MSK          _BIT(6)
    #define VPU_REG_RISC_ISR_FORCE      _BIT(10)
#define VPU_REG_RISC_MBOX_RDY                  ( 0x0068)
    #define VPU_REG_RISC_MBOX0_RDY _BIT(0)
    #define VPU_REG_RISC_MBOX1_RDY _BIT(1)
    #define VPU_REG_RISC_ISR_VALID      _BIT(2)
#define VPU_REG_HI_MBOX_RDY                    ( 0x0069)
    #define VPU_REG_HI_MBOX0_RDY   _BIT(0)
    #define VPU_REG_HI_MBOX1_RDY   _BIT(1)
#define VPU_REG_RISC_MBOX0_L                   ( 0x006b)
#define VPU_REG_RISC_MBOX0_H                   ( 0x006c)
#define VPU_REG_RISC_MBOX1_L                    ( 0x006d)
#define VPU_REG_RISC_MBOX1_H                   ( 0x006e)

#define VPU_REG_INSN_BASE_H  0x0071  // adr[31:20]<<8              // ??
#define VPU_REG_INSN_BASE_L    0x0072  // adr[19:4], unit:16byte   // ??
#define VPU_REG_INSN_ECO       0x0073
    #define VPU_REG_STOP_CPU       BIT0
    #define VPU_REG_ECO_1       _BIT(1)
    #define VPU_REG_ECO_2       _BIT(2)
    #define VPU_REG_INSN_BASE_HH       (_BIT(9) |_BIT(8))
#define VPU_REG_QMEM_MASK_L    0x0074
#define VPU_REG_QMEM_MASK_H    0x0075
#define VPU_REG_QMEM_BASE_L    0x0076
#define VPU_REG_QMEM_BASE_H    0x0077
#define VPU_REG_CPU_SETTING     0x0078
    #define VPU_REG_CPU_MHEG_EN     _BIT(0)
    #define VPU_REG_CPU_SW_RSTZ     _BIT(1)
    #define VPU_REG_CPU_SPI_BOOT     _BIT(6)
    #define VPU_REG_CPU_SDRAM_BOOT     _BIT(7)
#define VPU_REG_SDR_BASE_L   0x007a  //128bits alignment
#define VPU_REG_SDR_BASE_H   0x007b  //128bits alignment
#define VPU_REG_REG_BASE     0x007c    //REG ACCESS BASE32
    #define VPU_REG_REG_BASE_MVD      0x8008
#define VPU_REG_SPI_BASE    0x007f

#if 0
//------------------------------------------------------------------------------
// VD_MHEG5 Reg
//------------------------------------------------------------------------------
#define REG_CPU_RST_FINISH      0x58
#define REG_MHG_INSN_BASE_1     0x71    VPU_REG_INSN_BASE_H
#define REG_MHG_INSN_BASE_2     0x72    VPU_REG_INSN_BASE_L
#define REG_MHG_INSN_BASE_3     0x73    VPU_REG_INSN_ECO
#define REG_MHG_QMEM_DMASK0     0x74    VPU_REG_QMEM_MASK_L     //QMEM Mask
#define REG_MHG_QMEM_DMASK1     0x75    VPU_REG_QMEM_MASK_H     //QMEM Mask
#define REG_MHG_QMEM_DADDR0     0x76    VPU_REG_QMEM_BASE_L     //QMEM Addr
#define REG_MHG_QMEM_DADDR1     0x77    VPU_REG_QMEM_BASE_H     //QMEM Addr
#define REG_MHEG_ENABLE	        0x78    VPU_REG_CPU_SETTING
#define REG_MHG_SDR_BASE0       0x7a    VPU_REG_SDR_BASE_L
#define REG_MHG_SDR_BASE1       0x7b    VPU_REG_SDR_BASE_H
#define REG_MHG_REG32_BASE      0x7c    VPU_REG_REG_BASE
#define REG_MHG_SPI_BASE        0x7f    VPU_REG_SPI_BASE        //SPI base
#define MHG_REG32_BASE_MVD      0x8008
#endif

#if 0
//------------------------------------------------------------------------------
// SVD Reg
//------------------------------------------------------------------------------
#define REG_H264_ADDR_L            0x01
#define REG_H264_ADDR_H            0x02
#define REG_H264_DATAW_L           0x03
#define REG_H264_DATAW_H           0x04
#define REG_H264_DATAR_L           0x05
#define REG_H264_DATAR_H           0x06
#define REG_H264_HIRW_FIRE         0x07
#define REG_H264_CPU_SDR_BASE_L    0x08
#define REG_H264_CPU_SDR_BASE_H    0x09
#define REG_H264_RESET             0x0A
#define REG_H264_LDEND_EN          0x15
#define REG_H264_MIU_OFFSET_H0     0x1F

#define REG_H264_RESET_SWRST        BIT0
#define REG_H264_RESET_CPURST       BIT1
#define REG_H264_RESET_SWRST_FIN    BIT2
#define REG_H264_RESET_CPURST_FIN   BIT3

#define H264_HIRW_READ             0x00
#define H264_HIRW_WRITE            BIT1
#define H264_HIRW_READY            BIT0
#endif

#endif

