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

#ifndef _SAR_REG_H_
#define _SAR_REG_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
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

#endif // _SAR_REG_H_
