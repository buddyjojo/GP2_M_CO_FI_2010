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

#ifndef _CEC_REG_H_
#define _CEC_REG_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//for CEC registers
#define CEC_VERSION_11      0   //CEC1.1
#define CEC_VERSION_12      1   //CEC1.2
#define CEC_VERSION_12a     2   //CEC1.2a
#define CEC_VERSION_13      3   //CEC1.3
#define CEC_VERSION_13a     4   //CEC1.3a

#define HDMI_CEC_VERSION    CEC_VERSION_13a

#define CEC_FIFO_CNT       4
#define RETRY_CNT          3
#define FrameInterval      7
#define BusFreeTime        5
#define ReTxInterval       3

#define REG_BASE_ADDR_MIPS                  0xBF000000
#define REG_HDMI_BASE                       (REG_BASE_ADDR_MIPS+0x102700*2)
#define REG_HDMI_INT_MASK                   (REG_HDMI_BASE+(0x60<<2))
#define REG_HDMI_INT_STATUS                 (REG_HDMI_BASE+(0x61<<2))
#define REG_HDMI_INT_CLEAR                  (REG_HDMI_BASE+(0x63<<2))
#define REG_HDMI_CEC_CONFIG1                (REG_HDMI_BASE+(0x67<<2))
#define REG_HDMI_CEC_CONFIG2                (REG_HDMI_BASE+(0x68<<2))
#define REG_HDMI_CEC_CONFIG3                (REG_HDMI_BASE+(0x69<<2))
#define REG_HDMI_CEC_CONFIG4                (REG_HDMI_BASE+(0x6A<<2))
#define REG_HDMI_CEC_STATUS1                (REG_HDMI_BASE+(0x6C<<2))
#define REG_HDMI_CEC_TX_DATA0               (REG_HDMI_BASE+(0x70<<2))
#define REG_HDMI_CEC_TX_DATA1               (REG_HDMI_BASE+(0x71<<2))
#define REG_HDMI_CEC_RX_DATA0               (REG_HDMI_BASE+(0x78<<2))
#define REG_HDMI_CEC_RX_DATA1               (REG_HDMI_BASE+(0x79<<2))
#define CHIP_REG_BASE                        (REG_BASE_ADDR_MIPS + 0x101E00*2)
#define ADC_ATOP_REG_BASE                    (REG_BASE_ADDR_MIPS + 0x110900*2)
//#define ADC_DTOP_REG_BASE                    (REG_BASE_ADDR_MIPS + 0x2600*2)
#define IRQ_REG_BASE                          (REG_BASE_ADDR_MIPS + 0x101900*2)   // 0x1019??
//#define EX0_INT_MASK                         (IRQ_REG_BASE)
#define REG_IRQ_FINAL_STATUS                 (IRQ_REG_BASE + (0x7F<<2))

#define FREQ_12MHZ                      (12000000UL)
#define FREQ_14P318MHZ                  (14318180UL)
#define MST_XTAL_CLOCK_HZ               FREQ_12MHZ

//080910_yongs
#define CP_WRITE_CECM_MESSAGE		0x47
#define CP_WRITE_CECM_SETMODE		0x48
#define CP_WRITE_CECM_READY			0x49
#define CP_WRITE_CECM_DIRECTPLAY		0x4A	// SIMPLINK_070822

#endif // _CEC_REG_H_
