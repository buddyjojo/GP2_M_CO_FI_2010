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

#ifndef MHAL_MICOM_REG_H_
#define MHAL_MICOM_REG_H_
//RTC Section
#define CONTROL_WORD               0xB71B00 //12MHz

#define REG_MIPS_BASE               0xBF000000
#define REG_RTC_BASE                0x900
#define REG_RTC_BASE2               0x980

#define RTC_CTRL_NOT_RSTZ       (1 << 0)
#define RTC_CTRL_CNT_EN         (1 << 1)
#define RTC_CTRL_WRAP_EN        (1 << 2)
#define RTC_CTRL_LOAD_EN        (1 << 3)
#define RTC_CTRL_READ_EN        (1 << 4)
#define RTC_CTRL_INT_MASK       (1 << 5)
#define RTC_CTRL_INT_FORCE      (1 << 6)
#define RTC_CTRL_INT_CLEAR      (1 << 7)

#define RTC_INT_RAW_STATUS      (1 << 0)
#define RTC_INT_STATUS          (1 << 1)

#define REG_RTC_CTRL                (REG_RTC_BASE + 0x00)
#define REG_RTC_FREQ_CW1           (REG_RTC_BASE + 0x01)
#define REG_RTC_FREQ_CW2           (REG_RTC_BASE + 0x02)
#define REG_RTC_LOAD_VAL1          (REG_RTC_BASE + 0x03)
#define REG_RTC_LOAD_VAL2          (REG_RTC_BASE + 0x04)
#define REG_RTC_MATCH_VAL1         (REG_RTC_BASE + 0x05)
#define REG_RTC_MATCH_VAL2         (REG_RTC_BASE + 0x06)
#define REG_RTC_INTERRUPT          (REG_RTC_BASE + 0x07)
#define REG_RTC_CNT1                (REG_RTC_BASE + 0x08)
#define REG_RTC_CNT2                (REG_RTC_BASE + 0x09)

#define MHal_RTC_REG(addr)             (*(volatile U16*)(REG_MIPS_BASE + (addr<<2)))

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

//SAR Section
#define HAL_SAR_CH1                     0
#define HAL_SAR_CH2                     1
#define HAL_SAR_CH3                     2
#define HAL_SAR_CH4                     3
#define HAL_SAR_CH5                     4
#define HAL_SAR_CH6                     5
#define HAL_SAR_CH7                     6
#define HAL_SAR_CH8                     7
#define HAL_SAR_MAX_CHANNELS           (HAL_SAR_CH8+1)
#define HAL_SAR_ADC_DATA_MAX            0xFF

#define HAL_SAR_LEVEL                   1
#define HAL_SAR_EDGE                    0
#define HAL_SAR_FREERUN                 1
#define HAL_SAR_ONESHOT                 0
#define HAL_SAR_ADC_POWERDOWN          1
#define HAL_SAR_ADC_POWERUP             0

// defination for DISABLE
#define DISABLE     	                  0
// defination for ENABLE
#define ENABLE      	                  1

// base address
#define REG_MIPS_BASE               0xBF000000
#define REG_SAR_BASE                (0xA00<<2)

// mask bits
#define MASK_SAR_ADCOUT             0xFF    //sar data resilotion 8 bits
#define MASK_SAR_SINGLE_CH          0x07

// register definition
#define REG_SAR_CTRL0               (REG_SAR_BASE + (0x00<<2))
#define _SAR_SINGLE_CH              REG_SAR_CTRL0//[2:0] //select channel for single channel modech0~ch7
#define _SAR_LEVEL_TRIGGER          BIT3 //keypad level trigger enable. 0:  use edge trigger, 1:  use level trigger
#define _SAR_SINGLE_CH_EN           BIT4 //enable single channel mode. 0: disable1: enable
#define _SAR_MODE                    BIT5 //select sar digital operation mode. 0: one-shot, 1: freerun
#define _SAR_PD                      BIT6 //sar digital power down
#define _SAR_START                   BIT7 //sar start signal
#define _SAR_CH1_UPB                 (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define REG_SAR_CTRL1               (REG_SAR_BASE + (0x00<<2) +1)
#define _SAR_ADC_PD                 BIT0
#define _SAR_FREERUN                BIT1
#define _SAR_SEL                     BIT2
#define _SAR_8CH_EN                 BIT3

#define REG_SAR_CKSAMP_PRD         (REG_SAR_BASE + (0x01<<2))

#define REG_SAR_CH1_UPB             (REG_SAR_BASE + (0x05<<2))
#define REG_SAR_CH1_LOB             (REG_SAR_BASE + (0x05<<2) + 1)
#define REG_SAR_CH2_UPB             (REG_SAR_BASE + (0x06<<2))
#define REG_SAR_CH2_LOB             (REG_SAR_BASE + (0x06<<2) + 1)
#define REG_SAR_CH3_UPB             (REG_SAR_BASE + (0x07<<2))
#define REG_SAR_CH3_LOB             (REG_SAR_BASE + (0x07<<2) + 1)
#define REG_SAR_CH4_UPB             (REG_SAR_BASE + (0x08<<2))
#define REG_SAR_CH4_LOB             (REG_SAR_BASE + (0x08<<2) + 1)
#define REG_SAR_CH5_UPB             (REG_SAR_BASE + (0x09<<2))
#define REG_SAR_CH5_LOB             (REG_SAR_BASE + (0x09<<2) + 1)
#define REG_SAR_CH6_UPB             (REG_SAR_BASE + (0x0A<<2))
#define REG_SAR_CH6_LOB             (REG_SAR_BASE + (0x0A<<2) + 1)
#define REG_SAR_CH7_UPB             (REG_SAR_BASE + (0x0B<<2))
#define REG_SAR_CH7_LOB             (REG_SAR_BASE + (0x0B<<2) + 1)
#define REG_SAR_CH8_UPB             (REG_SAR_BASE + (0x0C<<2))
#define REG_SAR_CH8_LOB             (REG_SAR_BASE + (0x0C<<2) + 1)

#define REG_SAR_ADC_CH1_DATA        (REG_SAR_BASE + (0x0D<<2))
#define REG_SAR_ADC_CH2_DATA        (REG_SAR_BASE + (0x0D<<2) + 1)
#define REG_SAR_ADC_CH3_DATA        (REG_SAR_BASE + (0x0E<<2))
#define REG_SAR_ADC_CH4_DATA        (REG_SAR_BASE + (0x0E<<2) + 1)
#define REG_SAR_ADC_CH5_DATA        (REG_SAR_BASE + (0x0F<<2))
#define REG_SAR_ADC_CH6_DATA        (REG_SAR_BASE + (0x0F<<2) + 1)
#define REG_SAR_ADC_CH7_DATA        (REG_SAR_BASE + (0x10<<2))
#define REG_SAR_ADC_CH8_DATA        (REG_SAR_BASE + (0x10<<2) + 1)

#define REG_SAR_AISEL                (REG_SAR_BASE + (0x11<<2))
#define REG_SAR_OEN_SAR_GPIO        (REG_SAR_BASE + (0x11<<2) + 1)
#define REG_SAR_I_SAR_GPIO          (REG_SAR_BASE + (0x12<<2))
#define REG_SAR_C_SAR_GPIO          (REG_SAR_BASE + (0x12<<2) + 1)
#define REG_SAR_TEST0	    	      (REG_SAR_BASE + (0x13<<2))
#define REG_SAR_TEST1	    	      (REG_SAR_BASE + (0x13<<2) + 1)

#define REG_SAR_INT                 (REG_SAR_BASE + (0x14<<2))
#define _SAR_INT_MASK               BIT0
#define _SAR_INT_CLR                BIT1
#define _SAR_INT_FORCE              BIT2
#define _SAR_INT_STATUS             BIT3

#define MHal_SAR_REG(addr)             (*(volatile U8*)(REG_MIPS_BASE + (addr<<2)))

#endif // MAHL_MICOM_REG_H_
