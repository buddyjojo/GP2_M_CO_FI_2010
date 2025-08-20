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
/// file    regMIU.h
/// @brief  MIU Control Register Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MHAL_REG_MIU_H_
#define _MHAL_REG_MIU_H_


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define MIU_REG_BASE                            0x1200  // 0x1200 - 0x12FF

#define DDR_FREQ_SET_0                          0x1220
#define DDR_FREQ_SET_1                          0x1221
#define DDR_FREQ_STEP                           0x1224
#define DDR_FREQ_DIV_1                          0x1225
#define DDR_FREQ_INPUT_DIV_2                    0x1226
#define DDR_FREQ_LOOP_DIV_2                     0x1227
#define DDR_CLK_SELECT                          0x123E
#define DDR_FREQ_STATUS                         0x123F

#define MIU_RQ0L_MASK                           (MIU_REG_BASE+0x46)
#define MIU_RQ0H_MASK                           (MIU_REG_BASE+0x47)
#define MIU_RQ1L_MASK                           (MIU_REG_BASE+0x66)
#define MIU_RQ1H_MASK                           (MIU_REG_BASE+0x67)
#define MIU_RQ2L_MASK                           (MIU_REG_BASE+0x86)
#define MIU_RQ2H_MASK                           (MIU_REG_BASE+0x87)

//terry 20091027
#define MIU1_REG_BASE                           0x0600
#define MIU1_RQ1L_MASK                           (MIU1_REG_BASE+0x66)


#define MIU_PROTECT_EN                          (MIU_REG_BASE+0xC0)
#define MIU_PROTECT_SDR_LIKE                    (MIU_REG_BASE+0xC1)

#define MIU_PROTECT0_ID0                        (MIU_REG_BASE+0xC2)
#define MIU_PROTECT0_ID1                        (MIU_REG_BASE+0xC3)
#define MIU_PROTECT0_ID2                        (MIU_REG_BASE+0xC4)
#define MIU_PROTECT0_ID3                        (MIU_REG_BASE+0xC5)
#define MIU_PROTECT0_START_ADDR_H               (MIU_REG_BASE+0xC6)
#define MIU_PROTECT0_START_ADDR_L               (MIU_REG_BASE+0xC7)
#define MIU_PROTECT0_END_ADDR_H                 (MIU_REG_BASE+0xC8)
#define MIU_PROTECT0_END_ADDR_L                 (MIU_REG_BASE+0xC9)

#define MIU_PROTECT1_ID0                        (MIU_REG_BASE+0xCA)
#define MIU_PROTECT1_ID1                        (MIU_REG_BASE+0xCB)
#define MIU_PROTECT1_START_ADDR_H               (MIU_REG_BASE+0xCC)
#define MIU_PROTECT1_START_ADDR_L               (MIU_REG_BASE+0xCD)
#define MIU_PROTECT1_END_ADDR_H                 (MIU_REG_BASE+0xCE)
#define MIU_PROTECT1_END_ADDR_L                 (MIU_REG_BASE+0xCF)

#define MIU_PROTECT2_ID0                        (MIU_REG_BASE+0xD0)
#define MIU_PROTECT2_ID1                        (MIU_REG_BASE+0xD1)
#define MIU_PROTECT2_START_ADDR_H               (MIU_REG_BASE+0xD2)
#define MIU_PROTECT2_START_ADDR_L               (MIU_REG_BASE+0xD3)
#define MIU_PROTECT2_END_ADDR_H                 (MIU_REG_BASE+0xD4)
#define MIU_PROTECT2_END_ADDR_L                 (MIU_REG_BASE+0xD5)

#define MIU_PROTECT3_ID0                        (MIU_REG_BASE+0xD6)
#define MIU_PROTECT3_ID1                        (MIU_REG_BASE+0xD7)
#define MIU_PROTECT3_START_ADDR_H               (MIU_REG_BASE+0xD8)
#define MIU_PROTECT3_START_ADDR_L               (MIU_REG_BASE+0xD9)
#define MIU_PROTECT3_END_ADDR_H                 (MIU_REG_BASE+0xDA)
#define MIU_PROTECT3_END_ADDR_L                 (MIU_REG_BASE+0xDB)

#define MIU_DDFSET                              0x1220
#define MIU_DDFSPAN                             0x1222
#define MIU_DDFSTEP                             0x1224
#define MIU_PLLCTRL                             0x1225

#define MIU_RQ1_CTRL0                           0x1260
#define MIU_RQ2_CTRL0                           0x1280
#define MIU_RQ2_CTRL1                           0x1281
#define MIU_RQ2_HPRIORITY                       0x1288
#define MIU_GE_FLOW_CTRL_TIME                   0x1294

#define MIU_PROTECT_LOG_CLEAR                   0x12DE

// MIU0 self test (BIST)
#define REG_MIU_TEST_MODE                       0x12E0
#define REG_MIU_TEST_BASE                       0x12E2
#define REG_MIU_TEST_LENGTH                     0x12E4
#define REG_MIU_TEST_DATA                       0x12E8

// MIU1 self test (BIST)
#define REG_MIU1_TEST_MODE                      0x06E0
#define REG_MIU1_TEST_BASE                      0x06E2
#define REG_MIU1_TEST_LENGTH                    0x06E4
#define REG_MIU1_TEST_DATA                      0x06E8


// MIU selection registers
#define REG_MIU_SEL0                            0x12F0
#define REG_MIU_SEL1                            0x12F1
#define REG_MIU_SEL2                            0x12F2
#define REG_MIU_SEL3                            0x12F3
#define REG_MIU_SEL4                            0x12F4
#define REG_MIU_SEL5                            0x12F5
#define REG_MIU_SEL6                            0x12F6
#define REG_MIU_SEL7                            0x12F7

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


#endif // MHAL_REG_MIU_H_

