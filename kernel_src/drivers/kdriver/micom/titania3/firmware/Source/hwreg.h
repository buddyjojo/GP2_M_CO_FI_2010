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

#ifndef _HWREG_H
#define _HWREG_H

//------------------------------------------------------------------------------
// Base Address
//------------------------------------------------------------------------------
#define CHIP_REG_BASE                   0x1E00
#define IRQ_REG_BASE                    0x2B00
#define MBOX_REG_BASE                   0x3380//tbd
#define RTC_REG_BASE                    0x1200
#define SAR_REG_BASE                    0x1400
#define IR_REG_BASE                     0x1380//0x1300
#define CEC_REG_BASE                    0x1100


//------------------------------------------------------------------------------
// IRQ controller
//------------------------------------------------------------------------------
#define EX0_INT_MASK                    (IRQ_REG_BASE)          //0x2b00
#define EX0_INT_MASK_0                 ((EX0_INT_MASK)+0x0)
#define EX0_INT_MASK_1                 ((EX0_INT_MASK)+0x1)
#define EX0_INT_MASK_2                 ((EX0_INT_MASK)+0x2)
#define EX0_INT_MASK_3                 ((EX0_INT_MASK)+0x3)

#define EX0_INT_CLEAR_0                 ((EX0_INT_MASK)+0x8)
#define EX0_INT_CLEAR_1                 ((EX0_INT_MASK)+0x9)
#define EX0_INT_CLEAR_2                 ((EX0_INT_MASK)+0xa)
#define EX0_INT_CLEAR_3                 ((EX0_INT_MASK)+0xb)

#define EX0_INT_FINAL_STATUS_0          ((EX0_INT_MASK)+0x10)
#define EX0_INT_FINAL_STATUS_1          ((EX0_INT_MASK)+0x11)
#define EX0_INT_FINAL_STATUS_2          ((EX0_INT_MASK)+0x12)
#define EX0_INT_FINAL_STATUS_3          ((EX0_INT_MASK)+0x13)

#define EX1_INT_MASK                    ((IRQ_REG_BASE)+0x18)   //0x2b18
#define EX1_INT_MASK_0                  ((EX1_INT_MASK)+0x0)
#define EX1_INT_MASK_1                  ((EX1_INT_MASK)+0x1)
#define EX1_INT_MASK_2                  ((EX1_INT_MASK)+0x2)
#define EX1_INT_MASK_3                  ((EX1_INT_MASK)+0x3)

#define EX1_INT_FINAL_STATUS_0          ((IRQ_REG_BASE)+0x28)   //0x2b28
#define EX1_INT_FINAL_STATUS_1          ((IRQ_REG_BASE)+0x29)   //0x2b29
#define EX1_INT_FINAL_STATUS_2          ((IRQ_REG_BASE)+0x2a)   //0x2b2a
#define EX1_INT_FINAL_STATUS_3          ((IRQ_REG_BASE)+0x2b)   //0x2b2b

#define REG_C_FIQ_FORCE_0               ((IRQ_REG_BASE)+0x04)
#define REG_C_FIQ_FORCE_1               ((IRQ_REG_BASE)+0x05)
#define REG_C_FIQ_FORCE_2               ((IRQ_REG_BASE)+0x06)
#define REG_C_FIQ_FORCE_3               ((IRQ_REG_BASE)+0x07)
#define REG_AEON_C_FIQ_FORCE_0          ((IRQ_REG_BASE)+0x44)
#define REG_AEON_C_FIQ_FORCE_1          ((IRQ_REG_BASE)+0x45)


#define REG_GPIO_IN_LOW                         ((CHIP_REG_BASE)+ 0x0060 )
#define REG_GPIO_IN_HIGH                        ((CHIP_REG_BASE)+ 0x0061 )
#define REG_GPIO_OUT_LOW                        ((CHIP_REG_BASE)+ 0x0062 )
#define REG_GPIO_OUT_HIGH                       ((CHIP_REG_BASE)+ 0x0063 )
#define REG_GPIO_OE_0                           ((CHIP_REG_BASE)+ 0x0064 )
#define REG_GPIO_OE_1                           ((CHIP_REG_BASE)+ 0x0065 )


//------------------------------------------------------------------------------
// IR register define
//------------------------------------------------------------------------------
#define IR_CTRL                                 ((IR_REG_BASE)+0x0000)
#define IR_CHECK                                ((IR_REG_BASE)+0x0001)
#define IR_HDC_UPB_L                            ((IR_REG_BASE)+0x0002)
#define IR_HDC_UPB_H                            ((IR_REG_BASE)+0x0003)
#define IR_HDC_LOB_L                            ((IR_REG_BASE)+0x0004)
#define IR_HDC_LOB_H                            ((IR_REG_BASE)+0x0005)
#define IR_OFC_UPB_L                            ((IR_REG_BASE)+0x0006)
#define IR_OFC_UPB_H                            ((IR_REG_BASE)+0x0007)
#define IR_OFC_LOB_L                            ((IR_REG_BASE)+0x0008)
#define IR_OFC_LOB_H                            ((IR_REG_BASE)+0x0009)
#define IR_OFC_RP_UPB_L                         ((IR_REG_BASE)+0x000a)
#define IR_OFC_RP_UPB_H                         ((IR_REG_BASE)+0x000b)
#define IR_OFC_RP_LOB_L                         ((IR_REG_BASE)+0x000c)
#define IR_OFC_RP_LOB_H                         ((IR_REG_BASE)+0x000d)
#define IR_LG01H_UPB_L                          ((IR_REG_BASE)+0x000e)
#define IR_LG01H_UPB_H                          ((IR_REG_BASE)+0x000f)
#define IR_LG01H_LOB_L                          ((IR_REG_BASE)+0x0010)
#define IR_LG01H_LOB_H                          ((IR_REG_BASE)+0x0011)
#define IR_LG0_UPB_L                            ((IR_REG_BASE)+0x0012)
#define IR_LG0_UPB_H                            ((IR_REG_BASE)+0x0013)
#define IR_LG0_LOB_L                            ((IR_REG_BASE)+0x0014)
#define IR_LG0_LOB_H                            ((IR_REG_BASE)+0x0015)
#define IR_LG1_UPB_L                            ((IR_REG_BASE)+0x0016)
#define IR_LG1_UPB_H                            ((IR_REG_BASE)+0x0017)
#define IR_LG1_LOB_L                            ((IR_REG_BASE)+0x0018)
#define IR_LG1_LOB_H                            ((IR_REG_BASE)+0x0019)
#define IR_SEPR_UPB_L                           ((IR_REG_BASE)+0x001a)
#define IR_SEPR_UPB_H                           ((IR_REG_BASE)+0x001b)
#define IR_SEPR_LOB_L                           ((IR_REG_BASE)+0x001c)
#define IR_SEPR_LOB_H                           ((IR_REG_BASE)+0x001d)
#define IR_TIMEOUT_CYC_0                        ((IR_REG_BASE)+0x001e)
#define IR_TIMEOUT_CYC_1                        ((IR_REG_BASE)+0x001f)
#define IR_TIMEOUT_CYC_2                        ((IR_REG_BASE)+0x0020)
#define IR_CODEBYTE                             ((IR_REG_BASE)+0x0021)
#define IR_SEPR_BIT                             ((IR_REG_BASE)+0x0022)
#define IR_FIFO_CTRL                            ((IR_REG_BASE)+0x0023)  // different location with Saturn
#define IR_CCODE_L                              ((IR_REG_BASE)+0x0024)
#define IR_CCODE_H                              ((IR_REG_BASE)+0x0025)
#define IR_GLHRM_NUM_L                          ((IR_REG_BASE)+0x0026)
#define IR_GLHRM_NUM_H                          ((IR_REG_BASE)+0x0027)
#define IR_CKDIV_NUM_REG                        ((IR_REG_BASE)+0x0028)  // different location with Saturn
#define IR_KEY                                  ((IR_REG_BASE)+0x0029)
#define IR_SHOT_CNT_0                           ((IR_REG_BASE)+0x002a)
#define IR_SHOT_CNT_1                           ((IR_REG_BASE)+0x002b)
#define IR_SHOT_CNT_2                           ((IR_REG_BASE)+0x002c)
#define IR_RPT_FIFOEMPTY                        ((IR_REG_BASE)+0x002d)
#define IR_FIFO_READ_PULSE                      ((IR_REG_BASE)+0x0030)  // added in Saturn2

/*****************************************************************************/
//MCU reg
/*****************************************************************************/
// PIU base address :0x3c00
#define DMA_FLAGS                               (PIU_MISC_REG_BASE + 0x20)

#define REG_WDT_KEY                             (PIU_MISC_REG_BASE + 0x60)
#define REG_WDT_SEL                             (PIU_MISC_REG_BASE + 0x62)
#define REG_WDT_INT_SEL                         (PIU_MISC_REG_BASE + 0x64)
#define REG_WDT_CTRL                            (PIU_MISC_REG_BASE + 0x66)

#define REG_TIMER0_MAX                          (PIU_MISC_REG_BASE + 0x80)
#define REG_TIMER0_CNT_CAP                      (PIU_MISC_REG_BASE + 0x84)
#define REG_TIMER0_CTRL                         (PIU_MISC_REG_BASE + 0x88)

#define REG_TIMER1_MAX                          (PIU_MISC_REG_BASE + 0xA0)
#define REG_TIMER1_CNT_CAP                      (PIU_MISC_REG_BASE + 0xA4)
#define REG_TIMER1_CTRL                         (PIU_MISC_REG_BASE + 0xA8)

//------------------------------------------------------------------------------
// SAR define
#define REG_SAR_CTRL0                           (SAR_REG_BASE + 0x00)
#define REG_SAR_CTRL1                           (SAR_REG_BASE + 0x01)
#define REG_SAR_CTRL2                           (SAR_REG_BASE + 0x02)
#define REG_SAR_CTRL3                           (SAR_REG_BASE + 0x03)
#define REG_SAR_CTRL20                          (SAR_REG_BASE + 0x20)
#define REG_SAR_CTRL21                          (SAR_REG_BASE + 0x21)

#define REG_SAR_ADCOUT1                         (SAR_REG_BASE + 0x18)
#define REG_SAR_ADCOUT2                         (SAR_REG_BASE + 0x1A)
#define REG_SAR_ADCOUT3                         (SAR_REG_BASE + 0x1C)
#define REG_SAR_ADCOUT4                         (SAR_REG_BASE + 0x1E)

#define MASK_SAR_ADCOUT                         0x3F

#define GPIO_BASE_ADDRESS                       (MCU_PERI_REG_BASE + 0x48)   //reg[0x1048]
#define GPIO_P0_CTRL                            (GPIO_BASE_ADDRESS)
#define GPIO_P0_OE                              (GPIO_BASE_ADDRESS + 1)
#define GPIO_P0_IN                              (GPIO_BASE_ADDRESS + 2)
#define GPIO_P1_CTRL                            (GPIO_BASE_ADDRESS + 3)
#define GPIO_P1_OE                              (GPIO_BASE_ADDRESS + 4)
#define GPIO_P1_IN                              (GPIO_BASE_ADDRESS + 5)
#define GPIO_P2_CTRL                            (GPIO_BASE_ADDRESS + 6)
#define GPIO_P2_OE                              (GPIO_BASE_ADDRESS + 7)
#define GPIO_P2_IN                              (GPIO_BASE_ADDRESS + 8)
#define GPIO_P3_CTRL                            (GPIO_BASE_ADDRESS + 9)
#define GPIO_P3_OE                              (GPIO_BASE_ADDRESS + 10)
#define GPIO_P3_IN                              (GPIO_BASE_ADDRESS + 11)
#define GPIO_P4_CTRL                            (GPIO_BASE_ADDRESS + 12)
#define GPIO_P4_OE                              (GPIO_BASE_ADDRESS + 13)
#define GPIO_P4_IN                              (GPIO_BASE_ADDRESS + 14)

#define _OFF                                    0
#define _ON                                     1
#define _HIGH                                   1
#define _LOW                                    0
#define _OUTPUT                                 0
#define _INPUT                                  1


//------------------------------------------------------------------------------
// Mail Box Reg
//------------------------------------------------------------------------------
#define REG_51_MB_CMD_BASE          MBOX_REG_BASE
#define REG_AEON_MB_CMD_BASE        (MBOX_REG_BASE + 0x10)

#define MB_51_REG_CTRL_REG          (REG_51_MB_CMD_BASE + 0x0)
#define MB_51_REG_CMD_CLASS         (REG_51_MB_CMD_BASE + 0x1)
#define MB_51_REG_CMD_IDX           (REG_51_MB_CMD_BASE + 0x2)
#define MB_51_REG_PARAM_CNT         (REG_51_MB_CMD_BASE + 0x3)
#define MB_51_REG_PARAM_00          (REG_51_MB_CMD_BASE + 0x4)
#define MB_51_REG_PARAM_01          (REG_51_MB_CMD_BASE + 0x5)
#define MB_51_REG_PARAM_02          (REG_51_MB_CMD_BASE + 0x6)
#define MB_51_REG_PARAM_03          (REG_51_MB_CMD_BASE + 0x7)
#define MB_51_REG_PARAM_04          (REG_51_MB_CMD_BASE + 0x8)
#define MB_51_REG_PARAM_05          (REG_51_MB_CMD_BASE + 0x9)
#define MB_51_REG_PARAM_06          (REG_51_MB_CMD_BASE + 0xA)
#define MB_51_REG_PARAM_07          (REG_51_MB_CMD_BASE + 0xB)
#define MB_51_REG_PARAM_08          (REG_51_MB_CMD_BASE + 0xC)
#define MB_51_REG_PARAM_09          (REG_51_MB_CMD_BASE + 0xD)
#define MB_51_REG_Status_0          (REG_51_MB_CMD_BASE + 0xE)
#define MB_51_REG_Status_1          (REG_51_MB_CMD_BASE + 0xF)


//------------------------------------------------------------------------------
// CEC REG
//------------------------------------------------------------------------------
#define REG_CEC_PD_BASE          CEC_REG_BASE
#define REG_CEC_LOGICAL_ADDR    (CEC_REG_BASE + 0x00)
#define REG_CEC_OPCODE0          (CEC_REG_BASE + 0x06)
#define REG_CEC_OPCODE1          (CEC_REG_BASE + 0x07)
#define REG_CEC_OPCODE2          (CEC_REG_BASE + 0x08)
#define REG_CEC_OPCODE3          (CEC_REG_BASE + 0x09)
#define REG_CEC_OPCODE4          (CEC_REG_BASE + 0x0A)
#define REG_CEC_OP0_OPERAND      (CEC_REG_BASE + 0x0B)
#define REG_CEC_OP1_OPERAND      (CEC_REG_BASE + 0x0C)
#define REG_CEC_OP2_OPERAND0     (CEC_REG_BASE + 0x0D)
#define REG_CEC_OP2_OPERAND1     (CEC_REG_BASE + 0x0E)
#define REG_CEC_OP3_OPERAND0     (CEC_REG_BASE + 0x0F)
#define REG_CEC_OP3_OPERAND1     (CEC_REG_BASE + 0x10)
#define REG_CEC_HW_EVENT_STS     (CEC_REG_BASE + 0x12)


#endif // _HWREG_H

