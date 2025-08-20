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
/// @file  regMVD.h
/// @brief Hardware register definition for Video Decoder
/// @author MStar Semiconductor Inc.
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef _REG_MVD_H_
#define _REG_MVD_H_


////////////////////////////////////////////////////////////////////////////////
// Constant & Macro Definition
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// Base Address
//------------------------------------------------------------------------------
#define MVD_REG_BASE                            0x1100  // 0x1100 - 0x11FF
#define CHIP_REG_BASE                           0x1E00  // 0x1E00 - 0x1EFF
#define VD_MHEG5_REG_BASE                       0x0300

#define MIU0_REG_BASE                           0x1200
#define MIU1_REG_BASE                           0x0600


//------------------------------------------------------------------------------
// MIU register
//------------------------------------------------------------------------------
//MIU request mask
#define MIU0_RQ0_MASK_L                 (MIU0_REG_BASE + 0x23*2)
#define MIU0_RQ0_MASK_H                 (MIU0_REG_BASE + 0x23*2 +1)
#define MIU0_RQ1_MASK_L                 (MIU0_REG_BASE + 0x33*2)
#define MIU0_RQ1_MASK_H                 (MIU0_REG_BASE + 0x33*2 +1)
#define MIU0_RQ2_MASK_L                 (MIU0_REG_BASE + 0x43*2)
#define MIU0_RQ2_MASK_H                 (MIU0_REG_BASE + 0x43*2 +1)
#define MIU0_RQ3_MASK_L                 (MIU0_REG_BASE + 0x53*2)
#define MIU0_RQ3_MASK_H                 (MIU0_REG_BASE + 0x53*2 +1)
#define MIU0_SEL0_L                     (MIU0_REG_BASE + 0xF0)
#define MIU0_SEL0_H                     (MIU0_REG_BASE + 0xF1)
#define MIU0_SEL2_L                     (MIU0_REG_BASE + 0xF4)
#define MIU0_SEL2_H                     (MIU0_REG_BASE + 0xF5)
#define MIU0_SEL3_L                     (MIU0_REG_BASE + 0xF6)
#define MIU0_SEL3_H                     (MIU0_REG_BASE + 0xF7)

#define MIU1_RQ0_MASK_L                 (MIU1_REG_BASE + 0x23*2)
#define MIU1_RQ0_MASK_H                 (MIU1_REG_BASE + 0x23*2 +1)
#define MIU1_RQ1_MASK_L                 (MIU1_REG_BASE + 0x33*2)
#define MIU1_RQ1_MASK_H                 (MIU1_REG_BASE + 0x33*2 +1)
#define MIU1_RQ2_MASK_L                 (MIU1_REG_BASE + 0x43*2)
#define MIU1_RQ2_MASK_H                 (MIU1_REG_BASE + 0x43*2 +1)
#define MIU1_RQ3_MASK_L                 (MIU1_REG_BASE + 0x53*2)
#define MIU1_RQ3_MASK_H                 (MIU1_REG_BASE + 0x53*2 +1)


//------------------------------------------------------------------------------
// CPU register
//------------------------------------------------------------------------------
// CPU (VD_MHEG5)
#define REG_H264_CPU_SDR_BASE_L         (VD_MHEG5_REG_BASE + 0xE4)
#define REG_H264_CPU_SDR_BASE_H         (VD_MHEG5_REG_BASE + 0xE2)

#define REG_VD_MHEG5_RESET              (VD_MHEG5_REG_BASE + 0xB0)
    #define REG_VD_MHEG_RESET_CPURST    BIT1

#define REG_VD_MHEG5_ENABLE              (VD_MHEG5_REG_BASE + 0xF0)
    #define REG_VD_MHEG_ENABLE_COPRO     BIT0
    #define REG_VD_MHEG_ENABLE_SWRST     BIT1

#define REG_MHG_QMEM_DMASK  (VD_MHEG5_REG_BASE+0xe8)    //QMEM Mask
#define REG_MHG_QMEM_DADDR  (VD_MHEG5_REG_BASE+0xec)    //QMEM Addr
#define REG_MHG_SPI_BASE    (VD_MHEG5_REG_BASE+0xfe)    //SPI base
#define REG_MHG_REG32_BASE  (VD_MHEG5_REG_BASE+0xf8)
    #define MHG_REG32_BASE_MVD  0x8008
#define REG_MHG_SDR_BASE    (VD_MHEG5_REG_BASE+0xf4)
#define REG_MHG_INSN_BASE_0 (VD_MHEG5_REG_BASE+0xe4)
#define REG_MHG_INSN_BASE_1 (VD_MHEG5_REG_BASE+0xe5)
#define REG_MHG_INSN_BASE_2 (VD_MHEG5_REG_BASE+0xe3)
#define REG_MHG_INSN_BASE_3 (VD_MHEG5_REG_BASE+0xe7)


//------------------------------------------------------------------------------
// MVD Reg
//------------------------------------------------------------------------------
    #define MVD_CTRL_RST                        BIT0//1: reset MVD; 0: release reset
    #define MVD_CTRL_CLR_INT                    BIT2//Clear MVD interrupt.
    #define MVD_CTRL_CLK_SYNCMODE               BIT4//1: async_mode; 0: sync_mode
    #define MVD_CTRL_DISCONNECT_MIU             BIT6//1: disconnect; 0: release reset
#define MVD_CTRL                                (MVD_REG_BASE + 0x00)

    #define MVD_STATUS_READY                    BIT1
#define MVD_STATUS                              (MVD_REG_BASE + 0x01)
#define MVD_COMMAND                             (MVD_REG_BASE + 0x02)
#define MVD_ARG0                                (MVD_REG_BASE + 0x04)
#define MVD_ARG1                                (MVD_REG_BASE + 0x05)
#define MVD_ARG2                                (MVD_REG_BASE + 0x06)
#define MVD_ARG3                                (MVD_REG_BASE + 0x07)
#define MVD_ARG4                                (MVD_REG_BASE + 0x08)
#define MVD_ARG5                                (MVD_REG_BASE + 0x09)


//------------------------------------------------------------------------------
// Chiptop Reg
//------------------------------------------------------------------------------
//Clock setting for MVD hardware engine
#define REG_CHIPTOP_BASE        0x0b00

#define REG_CKG_MVD_SYNC        (REG_CHIPTOP_BASE + 0x38*2 +1)
    #define CKG_MVD_SYNC_GATED      BIT0

#define REG_CKG_MVD             (REG_CHIPTOP_BASE + 0x39*2)
    #define CKG_MVD_GATED           BIT0
    #define CKG_MVD_INVERT          BIT1
    #define CKG_MVD_MASK            (BIT3 | BIT2)
    #define CKG_MVD_144MHZ          (0 << 2)
    #define CKG_MVD_123MHZ          (1 << 2)
    #define CKG_MVD_CLK_MIU         (2 << 2)
    #define CKG_MVD_XTAL            (3 << 2)

#define REG_CKG_MVD2            (REG_CHIPTOP_BASE + 0x39*2 +1)
    #define CKG_MVD2_GATED           BIT0
    #define CKG_MVD2_INVERT          BIT1
    #define CKG_MVD2_MASK            (BIT3 | BIT2)
    #define CKG_MVD2_170MHZ          (0 << 2)
    #define CKG_MVD2_144MHZ          (1 << 2)
    #define CKG_MVD2_160MHZ          (2 << 2)
    #define CKG_MVD2_XTAL            (3 << 2)

#define REG_CKG_MVD_CHROMA      (REG_CHIPTOP_BASE + 0x3a*2)
    #define CKG_MVD_CHROMA_GATED           BIT0
    #define CKG_MVD_CHROMA_INVERT          BIT1

#define REG_CKG_MVD_LUMA_A      (REG_CHIPTOP_BASE + 0x3a*2 + 1)
    #define CKG_MVD_LUMA_A_GATED           BIT0
    #define CKG_MVD_LUMA_A_INVERT          BIT1

#define REG_CKG_MVD_LUMA_B      (REG_CHIPTOP_BASE + 0x3b*2)
    #define CKG_MVD_LUMA_B_GATED           BIT0
    #define CKG_MVD_LUMA_B_INVERT          BIT1

#define REG_CKG_MVD_LUMA_C      (REG_CHIPTOP_BASE + 0x3b*2 + 1)
    #define CKG_MVD_LUMA_C_GATED           BIT0
    #define CKG_MVD_LUMA_C_INVERT          BIT1


#define REG_CKG_MVD_RMEM        (REG_CHIPTOP_BASE + 0x3c*2)
    #define CKG_MVD_RMEM_GATED           BIT0
    #define CKG_MVD_RMEM_INVERT          BIT1

#define REG_CKG_MVD_RMEM1       (REG_CHIPTOP_BASE + 0x3c*2 + 1)
    #define CKG_MVD_RMEM1_GATED           BIT0
    #define CKG_MVD_RMEM1_INVERT          BIT1

#define REG_CKG_MVD_INTPRAM0    (REG_CHIPTOP_BASE + 0x3d*2)
    #define CKG_MVD_INTPRAM0_GATED           BIT0
    #define CKG_MVD_INTPRAM0_INVERT          BIT1

#define REG_CKG_MVD_INTPRAM1    (REG_CHIPTOP_BASE + 0x3d*2 + 1)
    #define CKG_MVD_INTPRAM1_GATED           BIT0
    #define CKG_MVD_INTPRAM1_INVERT          BIT1

#define REG_CKG_MVD_RREFDAT     (REG_CHIPTOP_BASE + 0x3e*2)
    #define CKG_MVD_RREFDAT_GATED           BIT0
    #define CKG_MVD_RREFDAT_INVERT          BIT1

#define REG_CKG_MVD_WREFDAT     (REG_CHIPTOP_BASE + 0x3e*2 + 1)
    #define CKG_MVD_WREFDAT_GATED           BIT0
    #define CKG_MVD_WREFDAT_INVERT          BIT1

#define REG_CKG_MVD_DPFF        (REG_CHIPTOP_BASE + 0x3f*2)
    #define CKG_MVD_DPFF_GATED              BIT0
    #define CKG_MVD_DPFF_INVERT             BIT1


#define REG_CKG_VD_AEON        (REG_CHIPTOP_BASE + 0x30*2 )
    #define CKG_VD_AEON_GATED             BIT0
    #define CKG_VD_AEON_INVERT            BIT1
    #define CKG_VD_AEON_MASK            (BIT6 | BIT5 | BIT4 | BIT3 | BIT2)
    #define CKG_VD_AEON_160MHZ          (0 << 2)
                //Notice: The clock 160M comes from UTMI.
                //Please start UTMI's clock before you switch to 160M
    #define CKG_VD_AEON_144MHZ          (1 << 2)
    #define CKG_VD_AEON_123MHZ          (2 << 2)
    #define CKG_VD_AEON_108MHZ          (3 << 2)
    #define CKG_VD_AEON_86MHZ           (4 << 2)
    #define CKG_VD_AEON_72MHZ           (5 << 2)
    #define CKG_VD_AEON_CLK_MCU         (1 << 5)    //01xxx
    #define CKG_VD_AEON_CLK_MIU         (2 << 5)    //10xxx
    #define CKG_VD_AEON_XTAL            (3 << 5)    //11xxx


#define REG_CHIP_ID_MAJOR        (CHIP_REG_BASE + 0xCC)
#define REG_CHIP_ID_MINOR        (CHIP_REG_BASE + 0xCD)
#define REG_CHIP_VERSION         (CHIP_REG_BASE + 0xCE)
#define REG_CHIP_REVISION        (CHIP_REG_BASE + 0xCF)

#endif // _REG_MVD_H_

