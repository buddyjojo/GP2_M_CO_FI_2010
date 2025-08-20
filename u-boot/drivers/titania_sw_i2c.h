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

#ifndef _TITANIA_I2C_H_
#define _TITANIA_I2C_H_

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define BIT0 						0x00000001
#define BIT1 						0x00000002
#define BIT2 						0x00000004
#define BIT3 						0x00000008
#define BIT4 						0x00000010
#define BIT5 						0x00000020
#define BIT6 						0x00000040
#define BIT7 						0x00000080
#define BIT8 						0x00000100
#define BIT9 						0x00000200
#define BIT10						0x00000400
#define BIT11						0x00000800
#define BIT12						0x00001000
#define BIT13						0x00002000
#define BIT14						0x00004000
#define BIT15						0x00008000

// definition for TRUE
#define TRUE						1
// definition for FALSE
#define FALSE						0

#define REG_MIPS_BASE  				0xBF800000
#define REG_ALL_PAD_IN 				((0x0f50<<2) + 1)   //set all pads (except SPI) as input

#define REG_DDCR_CK_SET				(0x0f70<<2)
#define REG_DDCR_DA_SET				(0x0f70<<2)
#define REG_DDCR_CK2_SET
#define REG_DDCR_DA2_SET
#define REG_GPIO12_SET
#define REG_GPIO13_SET
#define REG_GPIO14_SET
#define REG_GPIO15_SET
#define REG_PCM2_IRQA_N_SET			(0x0f6e<<2)
#define REG_PCM2_WAIT_N_SET			(0x0f6e<<2)
#define REG_LHSYNC2_SET
#define REG_LVSYNC2_SET

//the definitions of GPIO reg set to make output
#define REG_DDCR_CK_OEN 			(0x0f30<<2)
#define REG_DDCR_DA_OEN 			(0x0f30<<2)
#define REG_DDCR_CK2_OEN			((0x0f30<<2) + 1)
#define REG_DDCR_DA2_OEN			((0x0f30<<2) + 1)
#define REG_GPIO12_OEN  			((0x0f30<<2) + 1)
#define REG_GPIO13_OEN  			((0x0f30<<2) + 1)
#define REG_GPIO14_OEN  			((0x0f30<<2) + 1)
#define REG_GPIO15_OEN  			((0x0f30<<2) + 1)
#define REG_PCM2_IRQA_N_OEN			((0x0f40<<2) + 1)
#define REG_PCM2_WAIT_N_OEN			((0x0f40<<2) + 1)
#define REG_LHSYNC2_OEN    			((0x0f30<<2) + 1)
#define REG_LVSYNC2_OEN    			((0x0f30<<2) + 1)

//the definitions of GPIO reg set to get input
#define REG_DDCR_CK_IN 				(0x0f2e<<2)
#define REG_DDCR_DA_IN 				(0x0f2e<<2)
#define REG_DDCR_CK2_IN				((0x0f2e<<2) + 1)
#define REG_DDCR_DA2_IN				((0x0f2e<<2) + 1)
#define REG_GPIO12_IN  				((0x0f2e<<2) + 1)
#define REG_GPIO13_IN  				((0x0f2e<<2) + 1)
#define REG_GPIO14_IN  				((0x0f2e<<2) + 1)
#define REG_GPIO15_IN  				((0x0f2e<<2) + 1)
#define REG_PCM2_IRQA_N_IN			((0x0f3a<<2) + 1)
#define REG_PCM2_WAIT_N_IN			((0x0f3a<<2) + 1)
#define REG_LHSYNC2_IN    			((0x0f2e<<2) + 1)
#define REG_LVSYNC2_IN    			((0x0f2e<<2) + 1)

//the definitions of GPIO reg set to output
#define REG_DDCR_CK_OUT 			(0x0f2f<<2)
#define REG_DDCR_DA_OUT 			(0x0f2f<<2)
#define REG_DDCR_CK2_OUT			((0x0f2f<<2) + 1)
#define REG_DDCR_DA2_OUT			((0x0f2f<<2) + 1)
#define REG_GPIO12_OUT  			((0x0f2f<<2) + 1)
#define REG_GPIO13_OUT  			((0x0f2f<<2) + 1)
#define REG_GPIO14_OUT  			((0x0f2f<<2) + 1)
#define REG_GPIO15_OUT  			((0x0f2f<<2) + 1)
#define REG_PCM2_IRQA_N_OUT			((0x0f3d<<2) + 1)
#define REG_PCM2_WAIT_N_OUT			((0x0f3d<<2) + 1)
#define REG_LHSYNC2_OUT    			((0x0f2f<<2) + 1)
#define REG_LVSYNC2_OUT    			((0x0f2f<<2) + 1)

#define MHal_GPIO_REG(addr)			(*(volatile U8*)(REG_MIPS_BASE + (addr)))

#define REG_IIC_BASE				0xBF806840		// 0xBF800000 + (0x1A10*4)

#define REG_IIC_CTRL   				0x00
#define REG_IIC_CLK_SEL				0x01
#define REG_IIC_WDATA  				0x02
#define REG_IIC_RDATA  				0x03
#define REG_IIC_STATUS 				0x04			// reset, clear and status

#define MHal_IIC_REG(addr)			(*(volatile U32*)(REG_IIC_BASE + ((addr)<<2)))

#define REG_CHIP_BASE				0xBF803C00
#define REG_IIC_MODE 				0x50
#define REG_DDCR_GPIO_SEL			0x70
#define MHal_CHIP_REG(addr)			(*(volatile U32*)(REG_CHIP_BASE + ((addr)<<2)))

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    U16 SclOenReg;
    U8  SclOenBit;

    U16 SclOutReg;
    U8  SclOutBit;

    U16 SclInReg;
    U8  SclInBit;

    U16 SdaOenReg;
    U8  SdaOenBit;

    U16 SdaOutReg;
    U8  SdaOutBit;

    U16 SdaInReg;
    U8  SdaInBit;

    U8  DefDelay;
}IIC_Bus_t;

#endif // _TITANIA_I2C_H_
