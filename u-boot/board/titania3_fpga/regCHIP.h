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
/// file    regSystem.h
/// @brief  System Chip Top Registers Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _REG_SYSTEM_H_
#define _REG_SYSTEM_H_
#ifdef CONFIG_Titania3_FPGA

//0xbf203200
// FOR T3
#define REG_INT_BASEADR                 0xBF203200

//IRQ registers
#define REG_IRQ_MASK_L                  (REG_INT_BASEADR+0x74*4)
#define REG_IRQ_MASK_H                  (REG_INT_BASEADR+0x75*4)
#define REG_IRQ_FORCE_L                 (REG_INT_BASEADR+0x70*4)
#define REG_IRQ_FORCE_H                 (REG_INT_BASEADR+0x71*4)
//#define REG_IRQ_RAW_L                 (REG_INT_BASEADR+0x55*4)
//#define REG_IRQ_RAW_H                 (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_PENDING_L               (REG_INT_BASEADR+0x7C*4)
#define REG_IRQ_PENDING_H               (REG_INT_BASEADR+0x7D*4)

//IRQ EXP registers
#define REG_IRQ_EXP_MASK_L              (REG_INT_BASEADR+0x76*4)
#define REG_IRQ_EXP_MASK_H              (REG_INT_BASEADR+0x77*4)
#define REG_IRQ_EXP_FORCE_L             (REG_INT_BASEADR+0x72*4)
#define REG_IRQ_EXP_FORCE_H             (REG_INT_BASEADR+0x73*4)
//#define REG_IRQ_EXP_RAW_L             (REG_INT_BASEADR+0x55*4)
//#define REG_IRQ_EXP_RAW_H             (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_EXP_PENDING_L           (REG_INT_BASEADR+0x7E*4)
#define REG_IRQ_EXP_PENDING_H           (REG_INT_BASEADR+0x7F*4)

//FIQ registers
#define REG_FIQ_MASK_L                  (REG_INT_BASEADR+0x64*4)
#define REG_FIQ_MASK_H                  (REG_INT_BASEADR+0x65*4)
#define REG_FIQ_FORCE_L                 (REG_INT_BASEADR+0x60*4)
#define REG_FIQ_FORCE_H                 (REG_INT_BASEADR+0x61*4)
#define REG_FIQ_CLEAR_L                 (REG_INT_BASEADR+0x6C*4)
#define REG_FIQ_CLEAR_H                 (REG_INT_BASEADR+0x6D*4)
//#define REG_FIQ_RAW_L                 (REG_INT_BASEADR+0x4C*4)
//#define REG_FIQ_RAW_H                 (REG_INT_BASEADR+0x4D*4)
#define REG_FIQ_PENDING_L               (REG_INT_BASEADR+0x6C*4)
#define REG_FIQ_PENDING_H               (REG_INT_BASEADR+0x6D*4)

//FIQ EXP registers
#define REG_FIQ_EXP_MASK_L              (REG_INT_BASEADR+0x66*4)
#define REG_FIQ_EXP_MASK_H              (REG_INT_BASEADR+0x67*4)
#define REG_FIQ_EXP_FORCE_L             (REG_INT_BASEADR+0x62*4)
#define REG_FIQ_EXP_FORCE_H             (REG_INT_BASEADR+0x63*4)
#define REG_FIQ_EXP_CLEAR_L             (REG_INT_BASEADR+0x6E*4)
#define REG_FIQ_EXP_CLEAR_H             (REG_INT_BASEADR+0x6F*4)
//#define REG_FIQ_EXP_RAW_L             (REG_INT_BASEADR+0x4E*4)
//#define REG_FIQ_EXP_RAW_H             (REG_INT_BASEADR+0x4F*4)
#define REG_FIQ_EXP_PENDING_L           (REG_INT_BASEADR+0x6E*4)
#define REG_FIQ_EXP_PENDING_H           (REG_INT_BASEADR+0x6F*4)


#define MSTAR_INT_BASE  8
/*******************************************************/
/*   THE IRQ AND FIQ ARE NOT COMPLETED.                */
/*   FOR EACH IP OWNER, PLEASE REVIEW IT BY YOURSELF   */
/*******************************************************/
typedef enum
{
    //IRQ
    E_IRQL_START            = MSTAR_INT_BASE,
    E_IRQ_UART              = E_IRQL_START+0,   /*Titania PIU UART for MIPS*/
    E_IRQ_MVD               = E_IRQL_START+3,
    E_IRQ_EMAC              = E_IRQL_START+9,
    E_IRQ_UART0             = E_IRQL_START+4,
    E_IRQ_UART1             = E_IRQL_START+5,
    E_IRQ_IIC1              = E_IRQL_START+6,
    E_IRQ_UHC               = E_IRQL_START+7,			/* 1st EHCI */
    E_IRQ_SECOND_UHC  = E_IRQL_START+34,		/* tony add for 2st EHCI */
    E_IRQ_SPI_ARB_DUP_REG   = E_IRQL_START+8,
    E_IRQ_SPI_ARB_CHG_ADDR  = E_IRQL_START+3,
    E_IRQ_DEB               = E_IRQL_START+10,
    E_IRQ_UART2             = E_IRQL_START+11,
    E_IRQ_MICOM             = E_IRQL_START+12,
    E_IRQ_SBM               = E_IRQL_START+13,
    E_IRQ_COMB               = E_IRQL_START+14,
    E_IRQ_ECC_DERR          = E_IRQL_START+15,
    E_IRQL_END              = 15+MSTAR_INT_BASE,


    E_IRQH_START            = 16+MSTAR_INT_BASE,
    E_IRQ_TSP               = E_IRQH_START+0,
    E_IRQ_VE                = E_IRQH_START+1,
    E_IRQ_CIMAX2MCU         = E_IRQH_START+2,
    E_IRQ_DC                = E_IRQH_START+3,
    E_IRQ_GOP               = E_IRQH_START+4,
    E_IRQ_PCM2MCU           = E_IRQH_START+5,
    E_IRQ_IIC0              = E_IRQH_START+6,
    E_IRQ_RTC               = E_IRQH_START+7,
    E_IRQ_KEYPAD            = E_IRQH_START+8,
    E_IRQ_PM                = E_IRQH_START+9,
    E_IRQ_D2B               = E_IRQH_START+10,  //2008/10/23 Nick
    E_IRQ_VBI               = E_IRQH_START+12,
    E_IRQ_M4V               = E_IRQH_START+13,
    E_IRQ_MLINK             = E_IRQH_START+14,
    E_IRQ_HDMITX            = E_IRQH_START+15,
    E_IRQH_END              = 31+MSTAR_INT_BASE,


    //IRQ EXP
    E_IRQL_EXP_START            = 32+MSTAR_INT_BASE,
    E_IRQ_SVD              = E_IRQL_EXP_START+0,
    E_IRQ_USB1               = E_IRQL_EXP_START+1,
    E_IRQ_UHC1              = E_IRQL_EXP_START+2,
    E_IRQ_MIU             = E_IRQL_EXP_START+3,
    E_IRQ_DIP             = E_IRQL_EXP_START+14,

    E_IRQ_M4VE              = E_IRQL_EXP_START+15,
    E_IRQL_EXP_END              = 47+MSTAR_INT_BASE,

    E_IRQH_EXP_START            = 48+MSTAR_INT_BASE,
    E_IRQH_EXP_END            = 63+MSTAR_INT_BASE,


    //FIQ
    E_FIQL_START            = 64+MSTAR_INT_BASE,
    E_FIQ_INT_AEON_TO_8051  = E_FIQL_START+3,
    E_FIQ_INT_8051_TO_AEON  = E_FIQL_START+4,
    E_FIQ_INT_8051_TO_MIPS  = E_FIQL_START+5,
    E_FIQ_INT_MIPS_TO_8051  = E_FIQL_START+6,
    E_FIQ_DSP2_TO_MIPS      = E_FIQL_START+14,
    E_FIQL_END              = 79+MSTAR_INT_BASE,

    E_FIQH_START            = 80+MSTAR_INT_BASE,
    E_FIQ_VSYNC_VE4VBI      = E_FIQH_START+5,
    E_FIQ_FIELD_VE4VBI      = E_FIQH_START+6,
    E_FIQ_VSYN_COP          = E_FIQH_START+9,
    E_FIQ_VSYN_GOP0         = E_FIQH_START+11,
    E_FIQ_VSYN_GOP1         = E_FIQH_START+10,
    E_FIQ_IR                = E_FIQH_START+11,
    E_FIQ_DSP2UP            = E_FIQH_START+13,
    E_FIQ_DSP2MIPS          = E_FIQH_START+15,
    E_FIQH_END              = 95+MSTAR_INT_BASE,


    //FIQ EXP

    E_FIQL_EXP_START            = 96+MSTAR_INT_BASE,
    E_FIQ_INT_AEON_TO_MIPS      = E_FIQL_EXP_START+8,
    E_FIQL_EXP_END              = 111+MSTAR_INT_BASE,

    E_FIQH_EXP_START            = 112+MSTAR_INT_BASE,
    E_FIQ_INT_MIPS_TO_AEON      = E_FIQH_EXP_START+1,
    E_FIQH_EXP_END              = 127+MSTAR_INT_BASE,

    E_IRQ_FIQ_ALL               = 0xFF //all IRQs & FIQs

} InterruptNum;

//IRQ Low 16 bits
#define IRQL_UART                   (0x1 << (E_IRQ_UART  - E_IRQL_START))
#define IRQL_MVD                    (0x1 << (E_IRQ_MVD   - E_IRQL_START))
#define IRQL_UHC                    (0x1 << (E_IRQ_UHC   - E_IRQL_START))
#define IRQL_UART2                  (0x1 << (E_IRQ_UART2 - E_IRQL_START))
#define IRQL_UART1                  (0x1 << (E_IRQ_UART1 - E_IRQL_START))
#define IRQL_UART0                  (0x1 << (E_IRQ_UART0 - E_IRQL_START))
#define IRQL_EMAC                   (0x1 << (E_IRQ_EMAC  - E_IRQL_START))
#define IRQL_DEB                    (0x1 << (E_IRQ_DEB   - E_IRQL_START))
#define IRQL_EMAC                   (0x1 << (E_IRQ_EMAC  - E_IRQL_START))
#define IRQL_MICOM                  (0x1 << (E_IRQ_MICOM - E_IRQL_START))
#define IRQL_COMB                    (0x1 << (E_IRQ_COMB - E_IRQL_START))
#define IRQL_ALL                    0xFFFF //[15:0]

//IRQ High 16 bits
#define IRQH_TSP                    (0x1 << (E_IRQ_TSP      - E_IRQH_START)   )
#define IRQH_VE                     (0x1 << (E_IRQ_VE       - E_IRQH_START)   )
#define IRQH_CIMAX2MCU              (0x1 << (E_IRQ_CIMAX2MCU- E_IRQH_START)   )
#define IRQH_DC                     (0x1 << (E_IRQ_DC       - E_IRQH_START)   )
#define IRQH_GOP                    (0x1 << (E_IRQ_GOP      - E_IRQH_START)   )
#define IRQH_PCM2MCU                (0x1 << (E_IRQ_PCM2MCU  - E_IRQH_START)   )
#define IRQH_IIC0                   (0x1 << (E_IRQ_IIC0     - E_IRQH_START)   )
#define IRQH_RTC                    (0x1 << (E_IRQ_RTC      - E_IRQH_START)   )
#define IRQH_KEYPAD                 (0x1 << (E_IRQ_KEYPAD   - E_IRQH_START)   )
#define IRQH_PM                     (0x1 << (E_IRQ_PM       - E_IRQH_START)   )
#define IRQH_D2B                    (0x1 << (E_IRQ_D2B      - E_IRQH_START)   )//2008/10/23 Nick
#define IRQH_VBI                    (0x1 << (E_IRQ_VBI      - E_IRQH_START)   )
#define IRQH_M4V                    (0x1 << (E_IRQ_M4V      - E_IRQH_START)   )
#define IRQH_MLINK                  (0x1 << (E_IRQ_MLINK    - E_IRQH_START)   )
#define IRQH_HDMITX                 (0x1 << (E_IRQ_HDMITX   - E_IRQH_START)   )
#define IRQH_ALL                    0xFFFF //[15:0]


//FIQ Low 16 bits
#define FIQL_AEON_TO_8051           (0x1 << (E_FIQ_INT_AEON_TO_8051 - E_FIQL_START)  )
#define FIQL_8051_TO_AEON           (0x1 << (E_FIQ_INT_8051_TO_AEON - E_FIQL_START)  )
#define FIQL_8051_TO_MIPS           (0x1 << (E_FIQ_INT_8051_TO_MIPS - E_FIQL_START)  )
#define FIQL_MIPS_TO_8051           (0x1 << (E_FIQ_INT_MIPS_TO_8051 - E_FIQL_START)  )
#define FIQL_MIPS_TO_AEON           (0x1 << (E_FIQ_INT_MIPS_TO_AEON - E_FIQL_START)  )
#define FIQL_AEON_TO_MIPS           (0x1 << (E_FIQ_INT_AEON_TO_MIPS - E_FIQL_START)  )
#define FIQL_DSP2_TO_MIPS           (0x1 << (E_FIQ_DSP2_TO_MIPS - E_FIQL_START)  )
#define FIQL_ALL                    0xFFFF //[15:0]


//FIQ High 16 bits
#define FIQH_DSP2UP                 (0x1 << (E_FIQ_DSP2UP - E_FIQH_START)  )
#define FIQH_VSYN_GOP1              (0x1 << (E_FIQ_VSYN_GOP1 - E_FIQH_START)  )
#define FIQH_VSYN_GOP0              (0x1 << (E_FIQ_VSYN_GOP0 - E_FIQH_START)  )
#define FIQH_IR                     (0x1 << (E_FIQ_IR - E_FIQH_START)   )
#define FIQH_VSYNC_VE4VBI           (0x1 << (E_FIQ_VSYNC_VE4VBI - E_FIQH_START)   )
#define FIQH_FIELD_VE4VBI           (0x1 << (E_FIQ_FIELD_VE4VBI - E_FIQH_START)   )
#define FIQH_DSP2MIPS               (0x1 << (E_FIQ_DSP2MIPS - E_FIQH_START)  )

//IRQ EXP Low 16 bits
#define IRQ_SVD                   (0x1 << (E_IRQ_SVD  - E_IRQL_EXP_START))
#define IRQ_USB1                    (0x1 << (E_IRQ_USB1   - E_IRQL_EXP_START))
#define IRQ_UHC1                    (0x1 << (E_IRQ_UHC1   - E_IRQL_EXP_START))
#define IRQ_MIU                     (0x1 << (E_IRQ_MIU - E_IRQL_EXP_START))
#define IRQ_DIP                     (0x1 << (E_IRQ_DIP - E_IRQL_EXP_START))
#define IRQ_M4VE                     (0x1 << (E_IRQ_M4VE - E_IRQL_EXP_START))
#define IRQL_EXP_ALL                    0xFFFF //[15:0]

//IRQ EXP High 16 bits
#define IRQH_EXP_ALL                    0xFFFF //[15:0]

//FIQ EXP Low 16 bits
#define FIQL_EXP_ALL                    0xFFFF //[15:0]
//FIQ EXP High 16 bits
#define FIQH_EXP_ALL                    0xFFFF //[15:0]

#define FIQH_ALL                    0xFFFF //[15:0]


#else 

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------







//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define BIT(x)                      (1<<(x))
#define BMASK(bits)                 (BIT(((1)?bits)+1)-BIT(((0)?bits)))
#define BITS(bits,value)            ((BIT(((1)?bits)+1)-BIT(((0)?bits))) & (value<<((0)?bits)))


#define REG_TOP_BASE                0xBF203C00
#define TOP_REG(addr)               (*((volatile MS_U32*)(REG_TOP_BASE + ((addr)<<2))))


#define REG_TOP_CHIP_CONFIG_OW      0x0000
#define REG_TOP_UTMI_UART_SETHL     0x0001
    #define TOP_UART_RX_EN                      BIT(10)
#define REG_TOP_TEST_IN_OUT         0x0002
    #define TOP_TEST_IN_MASK                    BMASK(5:4)
    #define TOP_TEST_IN_TS0                     BITS(5:4, 1)
    #define TOP_TEST_IN_PCI                     BITS(5:4, 2)
    #define TOP_TEST_IN_I2S                     BITS(5:4, 3)
    #define TOP_TEST_OUT_MASK                   BMASK(7:6)
#define REG_TOP_PWM                 0x0003
#define REG_TOP_PCI                 0x0004
#define REG_TOP_SPI_EMAC            0x0005
    #define TOP_CKG_EMACRX_MASK                 BMASK(12:10)
    #define TOP_CKG_EMACRX_DIS                  BIT(10)
    #define TOP_CKG_EMACRX_INV                  BIT(11)
    #define TOP_CKG_EMACTX_MASK                 BMASK(15:13)
    #define TOP_CKG_EMACTX_DIS                  BIT(13)
    #define TOP_CKG_EMACTX_INV                  BIT(14)
#define REG_TOP_CR_CI_GPIO          0x0006
    #define TOP_PAD_CI_ADHI_MASK                BMASK(2:1)
    #define TOP_PAD_CI_ADHI_CCIR                BITS(2:1, 1)            // GPIO_CI5
    #define TOP_PAD_CI_ADHI_CI                  BITS(2:1, 3)
    #define TOP_PAD_CI_DATA_MASK                BMASK(4:3)
    #define TOP_PAD_CI_DATA_TSI                 BITS(4:3, 1)            // GPIO_CI6_7
    #define TOP_PAD_CI_DATA_CI                  BITS(4:3, 2)
    #define TOP_PAD_GPIO_S_MASK                 BIT(5)
    #define TOP_PAD_GPIO_S_X                    0
    #define TOP_PAD_GPIO_S_SM2                  BIT(5)
    #define TOP_PAD_CI_ADLO_MASK                BMASK(11:10)
    #define TOP_PAD_CI_ADLO_SM2                 BITS(11:10, 1)
    #define TOP_PAD_CI_ADLO_GPIO                BITS(11:10, 2)          // GPIO_CI0_4

#define REG_TOP_PCI_PD_TEST         0x0007
    #define TOP_PCI_RPU                         BMASK(9:0)
    #define TOP_PD_ALL                          BIT(10)
    #define TOP_TEST_CLK_MODE                   BIT(11)
    #define TOP_TURN_OFF_PAD                    BIT(12)
#define REG_TOP_CKG_ADCD_DVBC       0x0008
    #define TOP_CKG_ADCD_SHFT                   0
    #define TOP_CKG_ADCD_MASK                   BMASK(3:0)
    #define TOP_CKG_ADCD_DIS                    BIT(0)
    #define TOP_CKG_ADCD_INV                    BIT(1)
    #define TOP_CKG_ADCD_SRC_MASK               BMASK(3:2)
    #define TOP_CKG_ADCD_SRC_ADC                BITS(3:2, 0)
    #define TOP_CKG_ADCD_SRC_XTAL               BITS(3:2, 1)
    #define TOP_CKG_ADCD_SRC_EXTADC             BITS(3:2, 2)
    #define TOP_CKG_ADCD_SRC_DFT                BITS(3:2, 3)
    #define TOP_CKG_DVBC_ADC_SHFT               4
    #define TOP_CKG_DVBC_ADC_MASK               BMASK(7:4)
    #define TOP_CKG_DVBC_INN_SHFT               8
    #define TOP_CKG_DVBC_INN_MASK               BMASK(11:8)
    #define TOP_CKG_DVBC_INN_DIS                BIT(8)
    #define TOP_CKG_DVBC_INN_INV                BIT(9)
    #define TOP_CKG_DVBC_INN_SRC_MASK           BIT(11:10)              // 3:DFT_Live, other:clk_31
    #define TOP_CKG_DVBC_INN_SRC_31             BITS(11:10, 0)
    #define TOP_CKG_DVBC_INN_SRC_DFT            BITS(11:10, 3)
    #define TOP_CKG_DVBC_INN2X_SHFT             12
    #define TOP_CKG_DVBC_INN2X_MASK             BMASK(15:12)
    #define TOP_CKG_DVBC_INN2X_DIS              BIT(12)
    #define TOP_CKG_DVBC_INN2X_INV              BIT(13)
    #define TOP_CKG_DVBC_INN2X_SRC              BIT(15:14)              // 3:DFT_Live, other:clk_62
    #define TOP_CKG_DVBC_INN2X_SRC_62           BITS(15:14, 0)
    #define TOP_CKG_DVBC_INN2X_SRC_DFT          BITS(15:14, 3)

#define REG_TOP_DVBC_SC_CCIR        0x0009
    #define TOP_CKG_DVBC_EQ_SHFT                0
    #define TOP_CKG_DVBC_EQ_MASK                BMASK(3:0)
    #define TOP_CKG_DVBC_EQ_DIS                 BIT(0)
    #define TOP_CKG_DVBC_EQ_INV                 BIT(1)
    #define TOP_CKG_DVBC_EQ_SRC                 BMASK(3:2)               // 3:DFT_Live, other:clk_18
    #define TOP_CKG_DVBC_EQ_SRC_18              BITS(3:2, 0)
    #define TOP_CKG_DVBC_EQ_SRC_DFT             BITS(3:2, 3)
    #define TOP_CKG_DVBC_EQ8X_MASK              BMASK(7:4)
    #define TOP_CKG_DVBC_EQ8X_DIS               BIT(4)
    #define TOP_CKG_DVBC_EQ8X_INV               BIT(5)
    #define TOP_CKG_DVBC_EQ8X_SRC               BMASK(7:6)               // 3:DFT_Live, other:clk_144
    #define TOP_CKG_DVBC_EQ8X_SRC_144           BITS(7:6, 0)
    #define TOP_CKG_DVBC_EQ8X_SRC_DFT           BITS(7:6, 3)
    #define TOP_CKG_SC_MASK                     BMASK(13:8)
    #define TOP_CLK_CCIR_INV                    BIT(14)

#define REG_TOP_TSOUT_DISP          0x000A
    #define TOP_PCI_RPUPI                       BMASK(9:0)
    #define TOP_CLK_TSOUT_INV                   BIT(10)
    #define TOP_CKG_DISP                        BIT(15:12)
#define REG_TOP_DVBC_HDMI_CI        0x000B
    #define TOP_CKG_DVBC_TS_MASK                BMASK(4:0)
    #define TOP_CKG_DVBC_TS_DIS                 BIT(0)
    #define TOP_CKG_DVBC_TS_INV                 BIT(1)
    #define TOP_CKG_DVBC_TS_SRC_MASK            BMASK(4:0)
    #define TOP_CKG_DVBC_TS_DIS                 BIT(0)
    #define TOP_CKG_DVBC_TS_INV                 BIT(1)
    #define TOP_CKG_DVBC_TS_SRC_62              BITS(4:2, 0)
    #define TOP_CKG_DVBC_TS_SRC_54              BITS(4:2, 1)
    #define TOP_CKG_DVBC_TS_SRC_43P2            BITS(4:2, 2)
    #define TOP_CKG_DVBC_TS_SRC_36              BITS(4:2, 3)
    #define TOP_CKG_DVBC_TS_SRC_18              BITS(4:2, 4)
    #define TOP_CKG_DVBC_TS_SRC_13P5            BITS(4:2, 5)
    #define TOP_CKG_DVBC_TS_SRC_9               BITS(4:2, 6)
    #define TOP_CKG_DVBC_TS_SRC_DFT             BITS(4:2, 7)
    #define TOP_PAD_HDMI_HP_MASK                BIT(5)
    #define TOP_PAD_HDMI_HP_GPIO                BIT(5)
    #define TOP_PAD_HDMI_DDC_MASK               BIT(7:6)
    #define TOP_PAD_HDMI_DDC_X                  BITS(7:6, 0)
    #define TOP_PAD_HDMI_DDC_GPIO               BITS(7:6, 1)
    #define TOP_PAD_HDMI_DDC_IICM1              BITS(7:6, 2)
    #define TOP_PAD_HDMI_CCIR_MASK              BIT(8)
    #define TOP_PAD_HDMI_CCIR_GPIO              0
    #define TOP_PAD_HDMI_CCIR_X                 BIT(8)
    #define TOP_PAD_CI_CTL_MASK                 BIT(9)
    #define TOP_PAD_CI_CTL_X                    0
    #define TOP_PAD_CI_CTL_GPIO                 BIT(9)                  // GPIO_CI8_9
#define REG_TOP_GPIO_HM_OEN         0x000C
#define REG_TOP_PCI_RPD             0x000D
#define REG_TOP_GPIO_HM_IN          0x000E
#define REG_TOP_GPIO_HM_OUT         0x000F
#define REG_TOP_POFF_MCU            0x0010
#define REG_TOP_MCU_USB_STC0        0x0011
    #define TOP_MCU_CLK_MASK                    BMASK(0:0)
    #define TOP_MCU_CLK_DFT                     BITS(0:0, 0)
    #define TOP_MCU_CLK_SRC_MCUCLK              BITS(0:0, 1)
    #define TSP_STC0_SEL                        BIT(7)
#define REG_TOP_USB_RIU_DDR_MIU     0x0012
    #define TOP_CKG_USB_MASK                    BMASK(1:0)
    #define TOP_CKG_USB_DIS                     BIT(0)
    #define TOP_CKG_USB_INV                     BIT(1)
#define REG_TOP_TS0_TSP_STC0        0x0013
    #define TOP_CKG_TS0_MASK                    BMASK(3:0)
    #define TOP_CKG_TS0_DIS                     BIT(0)
    #define TOP_CKG_TS0_INV                     BIT(1)
    #define TOP_CKG_TS0_SRC_MASK                BMASK(3:2)
    #define TOP_CKG_TS0_SRC_TS0                 BITS(3:2, 0)
    #define TOP_CKG_TS0_SRC_TS1                 BITS(3:2, 1)
    #define TOP_CKG_TS0_SRC_DFT                 BITS(3:2, 3)
    #define TOP_CKG_TS0_SRC_DEMOD               TOP_CKG_TS0_SRC_TS0
    #define TOP_CKG_TS0_SRC_CI                  TOP_CKG_TS0_SRC_TS1
    #define TOP_CKG_TSP_MASK                    BMASK(11:6)
    #define TOP_CKG_TSP_DIS                     BIT(6)
    #define TOP_CKG_TSP_INV                     BIT(7)
    #define TOP_CKG_TSP_SRC_144                 BITS(9:8, 0)
    #define TOP_CKG_TSP_SRC_123                 BITS(9:8, 1)
    #define TOP_CKG_TSP_SRC_72                  BITS(9:8, 2)
    #define TOP_CKG_TSP_SRC_DFT                 BITS(9:8, 3)
    #define TOP_CKG_STC0_MASK                   BMASK(15:12)
    #define TOP_CKG_STC0_DIS                    BIT(12)
    #define TOP_CKG_STC0_INV                    BIT(13)
    #define TOP_CKG_STC0_SRC_STC0               BITS(15:14, 0)
    #define TOP_CKG_STC0_SRC_1                  BITS(15:14, 1)
    #define TOP_CKG_STC0_SRC_27                 BITS(15:14, 2)
    #define TOP_CKG_STC0_SRC_DFT                BITS(15:14, 3)
#define REG_TOP_MAD_MVD             0x0014
    #define TOP_CKG_MAD_STC_MASK                BMASK(3:0)
    #define TOP_CKG_MAD_STC_DIS                 BIT(0)
    #define TOP_CKG_MAD_STC_INV                 BIT(1)
    #define TOP_CKG_MAD_STC_SRC_STC0            BITS(3:2, 0)
    #define TOP_CKG_MAD_STC_SRC_27              BITS(3:2, 2)
    #define TOP_CKG_MAD_STC_SRC_DFT             BITS(3:2, 3)
    #define TOP_CKG_MVD_BOOT_MASK               BMASK(9:4)
    #define TOP_CKG_MVD_BOOT_DIS                BIT(4)
    #define TOP_CKG_MVD_BOOT_INV                BIT(5)
    #define TOP_CKG_MVD_BOOT_SRC_144            BITS(7:6, 0)
    #define TOP_CKG_MVD_BOOT_SRC_123            BITS(7:6, 1)
    #define TOP_CKG_MVD_BOOT_SRC_MIU            BITS(7:6, 2)
    #define TOP_CKG_MVD_BOOT_SRC_DFT            BITS(7:6, 3)
    #define TOP_CKG_MVD_MASK                    BMASK(15:10)
    #define TOP_CKG_MVD_DIS                     BIT(10)
    #define TOP_CKG_MVD_INV                     BIT(11)
    #define TOP_CKG_MVD_SRC_144                 BITS(13:12, 0)
    #define TOP_CKG_MVD_SRC_123                 BITS(13:12, 1)
    #define TOP_CKG_MVD_SRC_MIU                 BITS(13:12, 2)
    #define TOP_CKG_MVD_SRC_DFT                 BITS(13:12, 3)
#define REG_TOP_M4VD_EMAC_GE        0x0015
    #define TOP_CKG_M4VD_MASK                   BMASK(5:0)
    #define TOP_CKG_M4VD_DIS                    BIT(0)
    #define TOP_CKG_M4VD_INV                    BIT(1)
    #define TOP_CKG_M4VD_SRC_144                BITS(5:2, 0)
    #define TOP_CKG_M4VD_SRC_123                BITS(5:2, 1)
    #define TOP_CKG_M4VD_SRC_108                BITS(5:2, 2)
    #define TOP_CKG_M4VD_SRC_DFT                BITS(5:2, 3)
    #define TOP_CKG_EMACAHB_MASK                BMASK(11:8)
    #define TOP_CKG_EMACAHB_DIS                 BIT(8)
    #define TOP_CKG_EMACAHB_INV                 BIT(9)
    #define TOP_CKG_EMACAHB_SRC_108             BITS(11:10, 0)
    #define TOP_CKG_EMACAHB_SRC_86              BITS(11:10, 1)
    #define TOP_CKG_EMACAHB_SRC_62              BITS(11:10, 2)
    #define TOP_CKG_EMACAHB_SRC_DFT             BITS(11:10, 3)
    #define TOP_CKG_GE_MASK                     BMASK(15:12)
    #define TOP_CKG_GE_DIS                      BIT(12)
    #define TOP_CKG_GE_INV                      BIT(13)
    #define TOP_CKG_GE_SRC_123                  BITS(15:12, 0)
    #define TOP_CKG_GE_SRC_108                  BITS(15:12, 1)
    #define TOP_CKG_GE_SRC_86                   BITS(15:12, 2)
    #define TOP_CKG_GE_SRC_DFT                  BITS(15:12, 3)
#define REG_TOP_GOP                 0x0016
    #define TOP_CKG_GOP0_MASK                   BMASK(3:0)
    #define TOP_CKG_GOP0_DIS                    BIT(0)
    #define TOP_CKG_GOP0_INV                    BIT(1)
    #define TOP_CKG_GOP0_SRC_DISP               BITS(3:2, 0)
    #define TOP_CKG_GOP0_SRC_DFT                BITS(3:2, 3)
    #define TOP_CKG_GOP1_MASK                   BMASK(7:4)
    #define TOP_CKG_GOP1_DIS                    BIT(4)
    #define TOP_CKG_GOP1_INV                    BIT(5)
    #define TOP_CKG_GOP1_SRC_DISP               BITS(7:6, 0)
    #define TOP_CKG_GOP1_SRC_DFT                BITS(7:6, 3)
    #define TOP_CKG_GOPD_MASK                   BMASK(11:8)
    #define TOP_CKG_GOPD_DIS                    BIT(8)
    #define TOP_CKG_GOPD_INV                    BIT(9)
    #define TOP_CKG_GOPD_SRC_ADC                BITS(11:10, 0)
    #define TOP_CKG_GOPD_SRC_DISP               BITS(11:10, 1)
    #define TOP_CKG_GOPD_SRC_DFT                BITS(11:10, 3)
#define REG_TOP_GOP_UART            0x0017
    #define TOP_CKG_GOPG2_MASK                  BMASK(11:8)
    #define TOP_CKG_GOPG2_DIS                   BIT(8)
    #define TOP_CKG_GOPG2_INV                   BIT(9)
    #define TOP_CKG_GOPG2_SRC_DISP              BITS(11:10, 0)
    #define TOP_CKG_GOPG2_SRC_DFT               BITS(11:10, 3)
    #define TOP_CKG_UART_MASK                   BMASK(14:12)
    #define TOP_CKG_UART_DIS                    BIT(12)
    #define TOP_CKG_UART_INV                    BIT(13)
    #define TOP_CKG_UART_SRC_UART               BITS(14:14, 0)
    #define TOP_CKG_UART_SRC_DFT                BITS(14:14, 1)
#define REG_TOP_SM_PCM              0x0018
    #define TOP_CKG_SM_MASK                     BMASK(11:8)
    #define TOP_CKG_SM_DIS                      BIT(8)
    #define TOP_CKG_SM_INV                      BIT(9)
    #define TOP_CKG_SM_SRC_6                    BITS(11:10, 0)
    #define TOP_CKG_SM_SRC_4P5                  BITS(11:10, 1)
    #define TOP_CKG_SM_SRC_3                    BITS(11:10, 2)
    #define TOP_CKG_SM_SRC_DFT                  BITS(11:10, 3)
    #define TOP_CKG_PCM_MASK                    BMASK(15:12)
    #define TOP_CKG_PCM_DIS                     BIT(12)
    #define TOP_CKG_PCM_INV                     BIT(13)
    #define TOP_CKG_PCM_SRC_27                  BITS(15:14, 0)
    #define TOP_CKG_PCM_SRC_DFT                 BITS(15:14, 2)
#define REG_TOP_VE                  0x0019
    #define TOP_CKG_VE_MASK                     BMASK(11:8)
    #define TOP_CKG_VE_DIS                      BIT(8)
    #define TOP_CKG_VE_INV                      BIT(9)
    //[ERROR] bits error
    #define TOP_CKG_VEDAC_MASK                  BMASK(15:12)
    #define TOP_CKG_VEDAC_DIS                   BIT(12)
    #define TOP_CKG_VEDAC_INV                   BIT(13)
    #define TOP_CKG_VEDAC_SRC_27                BITS(15:14, 0)
    #define TOP_CKG_VEDAC_SRC_54                BITS(15:14, 1)
    #define TOP_CKG_VEDAC_SRC_108               BITS(15:14, 2)
    #define TOP_CKG_VEDAC_SRC_DFT               BITS(15:14, 3)
#define REG_TOP_DAC_SDR             0x001A
    #define TOP_CKG_DAC_MASK                    BMASK(4:0)
    #define TOP_CKG_DAC_DIS                     BIT(0)
    #define TOP_CKG_DAC_INV                     BIT(1)
    #define TOP_CKG_DAC_SRC_ODC                 BITS(4:3, 0)            //[???]
    #define TOP_CKG_DAC_SRC_VEDAC               BITS(4:3, 1)
    #define TOP_CKG_DAC_SRC_DFT                 BITS(4:3, 3)
#define REG_TOP_VE_FCIE             0x001C
    #define TOP_CKG_VEIN_MASK                   BMASK(5:0)
    #define TOP_CKG_VEIN_DIS                    BIT(0)
    #define TOP_CKG_VEIN_INV                    BIT(1)
    #define TOP_CKG_VEIN_SRC_ADC                BITS(5:2, 0)
    #define TOP_CKG_VEIN_SRC_DFT                BITS(5:2, 8)
    #define TOP_CKG_FCIE_MASK                   BMASK(14:8)
    #define TOP_CKG_FCIE_DIS                    BIT(8)
    #define TOP_CKG_FCIE_INV                    BIT(9)
    #define TOP_CKG_FCIE_SRC_86D256             BITS(14:10, 0)
    #define TOP_CKG_FCIE_SRC_86D64              BITS(14:10, 1)
    #define TOP_CKG_FCIE_SRC_86D16              BITS(14:10, 2)
    #define TOP_CKG_FCIE_SRC_54D4               BITS(14:10, 3)
    #define TOP_CKG_FCIE_SRC_72D4               BITS(14:10, 4)
    #define TOP_CKG_FCIE_SRC_86D4               BITS(14:10, 5)
    #define TOP_CKG_FCIE_SRC_54D2               BITS(14:10, 6)
    #define TOP_CKG_FCIE_SRC_72D2               BITS(14:10, 7)
    #define TOP_CKG_FCIE_SRC_86D2               BITS(14:10, 8)
    #define TOP_CKG_FCIE_SRC_54                 BITS(14:10, 9)
    #define TOP_CKG_FCIE_SRC_72                 BITS(14:10,10)
    #define TOP_CKG_FCIE_SRC_DFT                BITS(14:10,16)
#define REG_TOP_DC0_NUM             0x0020                              //[???]
#define REG_TOP_DC0_DEN             0x0021                              //[???]
#define REG_TOP_MCU                 0x0022
    #define TOP_CKG_MCU_MASK                    BMASK(12:8)
    #define TOP_CKG_MCU_DIS                     BIT(0)
    #define TOP_CKG_MCU_INV                     BIT(1)
    #define TOP_CKG_MCU_SRC_MASK                BMASK(12:10)
    #define TOP_CKG_MCU_SRC_216                 BITS(12:10, 0)
    #define TOP_CKG_MCU_SRC_DRAM                BITS(12:10, 1)
    #define TOP_CKG_MCU_SRC_173                 BITS(12:10, 2)
    #define TOP_CKG_MCU_SRC_160                 BITS(12:10, 3)
    #define TOP_CKG_MCU_SRC_144                 BITS(12:10, 4)
    #define TOP_CKG_MCU_SRC_123                 BITS(12:10, 5)
    #define TOP_CKG_MCU_SRC_108                 BITS(12:10, 6)
    #define TOP_CKG_MCU_SRC_DFT                 BITS(12:10, 7)
#define REG_TOP_USB_CW_L            0x0024
#define REG_TOP_USB_CW_H            0x0025
#define REG_TOP_STC0_CW_L           0x0026
#define REG_TOP_STC0_CW_H           0x0027
#define REG_TOP_DC0_CW_L            0x002A                              //[???]
#define REG_TOP_DC0_CW_H            0x002B                              //[???]
#define REG_TOP_002C                0x002C
    #define TOP_TSO_GPIO                        0x0001
#define REG_TOP_TSPUART_SM2         0x002D
    #define TOP_CKG_SM2_SHFT                    8
    #define TOP_CKG_SM2_MASK                    BMASK(11:8)
    #define TOP_CKG_SM2_DIS                     BIT(8)
    #define TOP_CKG_SM2_INV                     BIT(9)
    #define TOP_CKG_SM2_SRC_6                   BITS(11:10, 0)
    #define TOP_CKG_SM2_SRC_4P5                 BITS(11:10, 1)
    #define TOP_CKG_SM2_SRC_3                   BITS(11:10, 2)
    #define TOP_CKG_SM2_SRC_DFT                 BITS(11:10, 3)

#define REG_TOP_GPIO_WN_IN          0x0030
#define REG_TOP_GPIO_WN_OUT         0x0031
#define REG_TOP_GPIO_WN_OEN_DVBC    0x0032
    #define TOP_GPIO_W0                         BIT(0)
    #define TOP_GPIO_W1                         BIT(1)
    #define TOP_GPIO_N0                         BIT(4)
    #define TOP_GPIO_N1                         BIT(5)
    #define TOP_GPIO_N2                         BIT(6)
    #define TOP_GPIO_N3                         BIT(7)
    #define TOP_CKG_DVBC_DMA_MASK               BMASK(11:8)
    #define TOP_CKG_DVBC_DMA_EQ                 BITS(11:8, 0)
    #define TOP_CKG_DVBC_DMA_ADCD               BITS(11:8, 1)
    #define TOP_CKG_DVBC_DMA_0                  BITS(11:8, 2)
    #define TOP_CKG_DVBC_DMA_DFT                BITS(11:8, 3)
#define REG_TOP_DUMMY23             0x0037
    #define TOP_CKG_DUMMY23_MASK                BMASK(10:0)
#define REG_TOP_GPIO_PWM_CI_IN      0x0039
#define REG_TOP_GPIO_PWM_CI_OUT     0x003A
    #define TOP_GPIO_PWM0_OUT                   BIT(4)
    #define TOP_GPIO_PWM1_OUT                   BIT(5)
    #define TOP_GPIO_CI0_OUT                    BIT(6)
    #define TOP_GPIO_CI1_OUT                    BIT(7)
    #define TOP_GPIO_CI2_OUT                    BIT(8)
    #define TOP_GPIO_CI3_OUT                    BIT(9)
    #define TOP_GPIO_CI4_OUT                    BIT(10)
    #define TOP_GPIO_CI5_OUT                    BIT(11)
    #define TOP_GPIO_CI6_OUT                    BIT(12)
    #define TOP_GPIO_CI7_OUT                    BIT(13)
    #define TOP_GPIO_CI8_OUT                    BIT(14)
    #define TOP_GPIO_CI9_OUT                    BIT(15)


#define REG_TOP_GPIO_PWM_CI_OEN     0x003B
    #define TOP_GPIO_PWM0_DISABLE               BIT(4)
    #define TOP_GPIO_PWM1_DISABLE               BIT(5)
    #define TOP_GPIO_CI0_DISABLE                BIT(6)
    #define TOP_GPIO_CI1_DISABLE                BIT(7)
    #define TOP_GPIO_CI2_DISABLE                BIT(8)
    #define TOP_GPIO_CI3_DISABLE                BIT(9)
    #define TOP_GPIO_CI4_DISABLE                BIT(10)
    #define TOP_GPIO_CI5_DISABLE                BIT(11)
    #define TOP_GPIO_CI6_DISABLE                BIT(12)
    #define TOP_GPIO_CI7_DISABLE                BIT(13)
    #define TOP_GPIO_CI8_DISABLE                BIT(14)
    #define TOP_GPIO_CI9_DISABLE                BIT(15)


#define REG_TOP_GPIO_S_IN           0x003C
#define REG_TOP_GPIO_S_OUT          0x003D
    #define TOP_GPIO_S0_OUT                     BIT(0)
    #define TOP_GPIO_S1_OUT                     BIT(1)
    #define TOP_GPIO_S2_OUT                     BIT(2)
    #define TOP_GPIO_S3_OUT                     BIT(3)
    #define TOP_GPIO_S4_OUT                     BIT(4)
    #define TOP_GPIO_S5_OUT                     BIT(5)
    #define TOP_GPIO_S6_OUT                     BIT(6)


#define REG_TOP_GPIO_S_OEN          0x003E
    #define TOP_GPIO_S0_DISABLE                 BIT(0)
    #define TOP_GPIO_S1_DISABLE                 BIT(1)
    #define TOP_GPIO_S2_DISABLE                 BIT(2)
    #define TOP_GPIO_S3_DISABLE                 BIT(3)
    #define TOP_GPIO_S4_DISABLE                 BIT(4)
    #define TOP_GPIO_S5_DISABLE                 BIT(5)
    #define TOP_GPIO_S6_DISABLE                 BIT(6)


#define REG_TOP_GPIO_AU_SPI_IN      0x003F
#define REG_TOP_GPIO_AU_SPI_OUT     0x0040
#define REG_TOP_GPIO_AU_SPI_OEN     0x0041

#define REG_TOP_GPIO_DI_DO          0x0054
#define REG_TOP_UART                0x0055
#define REG_TOP_JTAG_AUD_SPI_GPIO_PWM 0x0056
    #define TOP_PAD_AU1_MASK                    BMASK(5:4)
    #define TOP_PAD_AU1_GPIO                    0
    #define TOP_PAD_AU1_IICM0                   BITS(5:4, 1)
    #define TOP_PAD_AU1_IICM1                   BITS(5:4, 2)
    #define TOP_PAD_AU1_X                       BITS(5:4, 3)
    #define TOP_PAD_AU2_MASK                    BMASK(7:6)
    #define TOP_PAD_AU2_GPIO                    0
    #define TOP_PAD_AU2_PCM                     BMASK(7:6, 1)
    #define TOP_PAD_AU2_X                       BMASK(7:6, 2)
    #define TOP_PAD_SPICZ2_MASK                 BIT(8)
    #define TOP_PAD_SPICZ2_X                    0
    #define TOP_PAD_SPICZ2_GPIO                 BIT(8)
    #define TOP_PAD_GPIOW0_MASK                 BIT(9)
    #define TOP_PAD_GPIOW0_X                    0
    #define TOP_PAD_GPIOW0_CEC                  BIT(9)
    #define TOP_PAD_PWM_MASK                    BIT(10)
    #define TOP_PAD_PWM_X                       0
    #define TOP_PAD_PWM_GPIO                    BIT(10)


#define REG_TOP_STAT_BOND           0x0061
#define REG_TOP_BOND_OV_KEY         0x0063
#define REG_TOP_CHIP_CONFIG_STAT    0x0065
#define REG_TOP_DEVICE_ID           0x0066
#define REG_TOP_CHIP_VERSION        0x0067
    #define CHIP_VERSION_SHFT                   0
    #define CHIP_VERSION_MASK                   BMASK(7:0)
    #define CHIP_REVISION_SHFT                  8
    #define CHIP_REVISION_MASK                  BMASK(15:8)
#define REG_TOP_TESTBUS             0x0075
    #define TOP_TESTBUS_EN                      BIT(14)
#define REG_CLK_EJ_MIPS_TSJ         0x0076
    #define TOP_TESTCLK_OUT_MASK                BMASK(2:0)
    #define TOP_TESTCLK_OUT_NONE                BITS(2:0, 0)
#define REG_TOP_RESET_CPU0          0x0077
    #define TOP_RESET_PASSWD_MASK               BMASK(14:0)
    #define TOP_RESET_CPU0                      BIT(15)

/*
#define REG_ECC_BASE                0xBF8069C0
#define ECC_REG(addr)               (*((volatile MS_U32*)(REG_ECC_BASE + ((addr)<<2))))

#define REG_ECC_REQUEST             0x0001
    #define ECC_MCU_BURST_READ_EN               BIT(5)


#define REG_WDT_BASE                0xBF808400
#define WDT_REG(addr)               (*((volatile MS_U32*)(REG_WDT_BASE + ((addr)<<2))))

#define REG_WDT_DISABLEWORD_L       0x0000
#define REG_WDT_DISABLEWORD_H       0x0001
#define REG_WDT_PROGRAMKEY_L        0x0002
#define REG_WDT_PROGRAMKEY_H        0x0003
#define REG_WDT_ENABLEMCURESET      0x0004
#define REG_WDT_CLEAR_STATUS        0x0010
#define REG_WDT_SETTIME             0x0012
#define REG_WDT_COUNTER_L           0x0014
#define REG_WDT_COUNTER_H           0x0015


#define REG_ANA_BASE                0xBF20A400
#define ANA_REG(addr)               (*((volatile MS_U32*)(REG_ANA_BASE + ((addr)<<2))))

// 0x0006
    #define ANA_REF_TST_IRE                     BIT(0)
    #define ANA_REF_PWDN_ICAL                   BIT(1)
    #define ANA_REF_TST_BGO                     BIT(2)
    #define ANA_REF_TST_BGIE                    BIT(3)
    #define ANA_REF_VCLP                        BMASK(6:4)
    #define ANA_REF_TST_VRPM                    BIT(7)
    #define ANA_REF_TST_VCLPBW                  BMASK(9:8)
    #define ANA_REF_PWDN_CLP                    BIT(10)
    #define ANA_REF_TST_IRO                     BIT(11)
    #define ANA_REF_ICAL                        BMASK(15:12)

// 0x000C
    #define ANA_EXT_DAC                         BIT(0)
    #define ANA_CLK_POL                         BIT(1)
    #define ANA_DBG_SEL                         BMASK(3:2)
    #define ANA_SEL_TUAGC                       BMASK(9:8)
    #define ANA_SEL_IFAGC                       BMASK(11:10)
    #define ANA_AGC_PAD_OEN                     BIT(12);

// 0x000D
    #define ANA_ADC_OFFSET                      BMASK(11:0)
    #define ANA_ADC_RDSEL                       BIT(13)
    #define ANA_ADCIN_SIGN                      BIT(14)
    #define ANA_ADCD_FRZ                        BIT(15)

// 0x0010
    // reg_adcin_i read only register Debug Purpose to check ADC is Input or Not!
    // If ADC is input , reading this register will find it always changing !
    #define ANA_ADCIN_I                         BMASK(10:0)
*/
// Interrupt related
#define REG_INT_BASEADR                 0xBF203200

//IRQ registers
#define REG_IRQ_MASK_L                  (REG_INT_BASEADR+0x74*4)
#define REG_IRQ_MASK_H                  (REG_INT_BASEADR+0x75*4)
#define REG_IRQ_FORCE_L                 (REG_INT_BASEADR+0x70*4)
#define REG_IRQ_FORCE_H                 (REG_INT_BASEADR+0x71*4)
//#define REG_IRQ_RAW_L                 (REG_INT_BASEADR+0x55*4)
//#define REG_IRQ_RAW_H                 (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_PENDING_L               (REG_INT_BASEADR+0x7C*4)
#define REG_IRQ_PENDING_H               (REG_INT_BASEADR+0x7D*4)

//IRQ EXP registers
#define REG_IRQ_EXP_MASK_L              (REG_INT_BASEADR+0x76*4)
#define REG_IRQ_EXP_MASK_H              (REG_INT_BASEADR+0x77*4)
#define REG_IRQ_EXP_FORCE_L             (REG_INT_BASEADR+0x72*4)
#define REG_IRQ_EXP_FORCE_H             (REG_INT_BASEADR+0x73*4)
//#define REG_IRQ_EXP_RAW_L             (REG_INT_BASEADR+0x55*4)
//#define REG_IRQ_EXP_RAW_H             (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_EXP_PENDING_L           (REG_INT_BASEADR+0x7E*4)
#define REG_IRQ_EXP_PENDING_H           (REG_INT_BASEADR+0x7F*4)

//FIQ registers
#define REG_FIQ_MASK_L                  (REG_INT_BASEADR+0x64*4)
#define REG_FIQ_MASK_H                  (REG_INT_BASEADR+0x65*4)
#define REG_FIQ_FORCE_L                 (REG_INT_BASEADR+0x60*4)
#define REG_FIQ_FORCE_H                 (REG_INT_BASEADR+0x61*4)
#define REG_FIQ_CLEAR_L                 (REG_INT_BASEADR+0x6C*4)
#define REG_FIQ_CLEAR_H                 (REG_INT_BASEADR+0x6D*4)
//#define REG_FIQ_RAW_L                 (REG_INT_BASEADR+0x4C*4)
//#define REG_FIQ_RAW_H                 (REG_INT_BASEADR+0x4D*4)
#define REG_FIQ_PENDING_L               (REG_INT_BASEADR+0x6C*4)
#define REG_FIQ_PENDING_H               (REG_INT_BASEADR+0x6D*4)

//FIQ EXP registers
#define REG_FIQ_EXP_MASK_L              (REG_INT_BASEADR+0x66*4)
#define REG_FIQ_EXP_MASK_H              (REG_INT_BASEADR+0x67*4)
#define REG_FIQ_EXP_FORCE_L             (REG_INT_BASEADR+0x62*4)
#define REG_FIQ_EXP_FORCE_H             (REG_INT_BASEADR+0x63*4)
#define REG_FIQ_EXP_CLEAR_L             (REG_INT_BASEADR+0x6E*4)
#define REG_FIQ_EXP_CLEAR_H             (REG_INT_BASEADR+0x6F*4)
//#define REG_FIQ_EXP_RAW_L             (REG_INT_BASEADR+0x4E*4)
//#define REG_FIQ_EXP_RAW_H             (REG_INT_BASEADR+0x4F*4)
#define REG_FIQ_EXP_PENDING_L           (REG_INT_BASEADR+0x6E*4)
#define REG_FIQ_EXP_PENDING_H           (REG_INT_BASEADR+0x6F*4)




//IRQ Low 16 bits
#define IRQL_UHC                    (0x1 << (E_IRQ_UHC - E_IRQL_START))
#define IRQL_UART2                  (0x1 << (E_IRQ_UART2 - E_IRQL_START))
#define IRQL_UART1                  (0x1 << (E_IRQ_UART1 - E_IRQL_START))
#define IRQL_UART0                  (0x1 << (E_IRQ_UART0 - E_IRQL_START))
#define IRQL_EMAC                   (0x1 << (E_IRQ_EMAC  - E_IRQL_START))
#define IRQL_DEB                    (0x1 << (E_IRQ_DEB   - E_IRQL_START)   )

/*
#define IRQL_EC_BRIDGE              (0x1 << (E_IRQ_EC_BRIDGE-E_IRQL_START) )
#define IRQL_UART                   (0x1 << (E_IRQ_UART-E_IRQL_START) )
#define IRQL_IR                     (0x1 << (E_IRQ_IR-E_IRQL_START) )
#define IRQL_ADCDVI2RIU             (0x1 << (E_IRQ_ADCDVI2RIU-E_IRQL_START) )
#define IRQL_DC                     (0x1 << (E_IRQ_DC-E_IRQL_START) )
#define IRQL_I2C_MASTER             (0x1 << (E_IRQ_I2C_MASTER-E_IRQL_START) )
#define IRQL_M4VD                   (0x1 << (E_IRQ_M4VD-E_IRQL_START) )
#define IRQL_CARD_READER            (0x1 << (E_IRQ_CARD_READER-E_IRQL_START) )
#define IRQL_MVD                    (0x1 << (E_IRQ_MVD-E_IRQL_START) )
#define IRQL_PCI                    (0x1 << (E_IRQ_PCI-E_IRQL_START) )
#define IRQL_USB                    (0x1 << (E_IRQ_USB-E_IRQL_START) )
#define IRQL_DSP2UP                 (0x1 << (E_IRQ_DSP2UP-E_IRQL_START) )
#define IRQL_USB_PHY                (0x1 << (E_IRQ_USB_PHY-E_IRQL_START) )
#define IRQL_SPI_DMA                (0x1 << (E_IRQ_SPI_DMA-E_IRQL_START) )
#define IRQL_VE                     (0x1 << (E_IRQ_VE-E_IRQL_START) )
*/
#define IRQL_ALL                    0xFFFF //[15:0]

//IRQ High 16 bits
#define IRQH_TSP                    (0x1 << (E_IRQ_TSP      -E_IRQH_START)   )
#define IRQH_VE                     (0x1 << (E_IRQ_VE       -E_IRQH_START)   )
#define IRQH_CIMAX2MCU              (0x1 << (E_IRQ_CIMAX2MCU-E_IRQH_START)   )
#define IRQH_DC                     (0x1 << (E_IRQ_DC       -E_IRQH_START)   )
#define IRQH_GOP                    (0x1 << (E_IRQ_GOP      -E_IRQH_START)   )
#define IRQH_PCM2MCU                (0x1 << (E_IRQ_PCM2MCU  -E_IRQH_START)   )
#define IRQH_IIC0                   (0x1 << (E_IRQ_IIC0     -E_IRQH_START)   )
#define IRQH_RTC                    (0x1 << (E_IRQ_RTC      -E_IRQH_START)   )
#define IRQH_KEYPAD                 (0x1 << (E_IRQ_KEYPAD   -E_IRQH_START)   )
#define IRQH_PM                     (0x1 << (E_IRQ_PM       -E_IRQH_START)   )
#define IRQH_DMD                    (0x1 << (E_IRQ_DVBC     -E_IRQH_START)   )

#define IRQH_M4V                    (0x1 << (E_IRQ_M4V      -E_IRQH_START)   )
#define IRQH_FCIE2RIU               (0x1 << (E_IRQ_FCIE2RIU -E_IRQH_START)   )
#define IRQH_HDMITX                 (0x1 << (E_IRQ_HDMITX   -E_IRQH_START)   )


#define IRQL_EMAC                   (0x1 << (E_IRQ_EMAC  - E_IRQL_START))

#define IRQH_ALL                    0xFFFF //[15:0]


//FIQ Low 16 bits
/*
#define FIQL_INT                    (0x1 << (E_FIQ_INT-E_FIQL_START)  )
#define FIQL_GPIO1                  (0x1 << (E_FIQ_GPIO1-E_FIQL_START)  )
#define FIQL_GPIO2                  (0x1 << (E_FIQ_GPIO2-E_FIQL_START)  )
#define FIQL_GPIO3                  (0x1 << (E_FIQ_GPIO3-E_FIQL_START)  )
#define FIQL_GPIO4                  (0x1 << (E_FIQ_GPIO4-E_FIQL_START)  )
#define FIQL_GPIO5                  (0x1 << (E_FIQ_GPIO5-E_FIQL_START)  )
#define FIQL_GPIO6                  (0x1 << (E_FIQ_GPIO6-E_FIQL_START)  )
#define FIQL_GPIO7                  (0x1 << (E_FIQ_GPIO7-E_FIQL_START)  )
#define FIQL_GPIO8                  (0x1 << (E_FIQ_GPIO8-E_FIQL_START)  )
#define FIQL_GPIO9                  (0x1 << (E_FIQ_GPIO9-E_FIQL_START)  )
#define FIQL_GPIO10                 (0x1 << (E_FIQ_GPIO10-E_FIQL_START)  )
#define FIQL_GPIO11                 (0x1 << (E_FIQ_GPIO11-E_FIQL_START)  )
#define FIQL_C_SAR_GPIO12           (0x1 << (E_FIQ_C_SAR_GPIO12-E_FIQL_START)  )
#define FIQL_C_SAR_GPIO13           (0x1 << (E_FIQ_C_SAR_GPIO13-E_FIQL_START)  )
#define FIQL_C_SAR_GPIO14           (0x1 << (E_FIQ_C_SAR_GPIO14-E_FIQL_START)  )
#define FIQL_C_SAR_GPIO15           (0x1 << (E_FIQ_C_SAR_GPIO15-E_FIQL_START)  )
*/
#define FIQL_ALL                    0xFFFF //[15:0]


//FIQ High 16 bits
/*
#define FIQH_VSYNC_ADCDVIPLL0       (0x1 << (E_FIQ_VSYNC_ADCDVIPLL0     -E_FIQH_START)  )
#define FIQH_VSYNC_ADCDVIPLL1       (0x1 << (E_FIQ_VSYNC_ADCDVIPLL1     -E_FIQH_START)  )
#define FIQH_VSYNC_ADCDVIPLL2       (0x1 << (E_FIQ_VSYNC_ADCDVIPLL2     -E_FIQH_START)  )
#define FIQH_VSYNC_FIELD_VE4VBI     (0x1 << (E_FIQ_VSYNC_FIELD_VE4VBI   -E_FIQH_START)  )
#define FIQH_VSYNC_VE4VBI           (0x1 << (E_FIQ_VSYNC_VE4VBI         -E_FIQH_START)  )
#define FIQH_VSYNC_AFEC             (0x1 << (E_FIQ_VSYNC_AFEC           -E_FIQH_START)  )
#define FIQH_VSYN_MOD               (0x1 << (E_FIQ_VSYN_MOD             -E_FIQH_START)  )
#define FIQH_SPRANGE_V_FL_F2        (0x1 << (E_FIQ_SPRANGE_V_FL_F2      -E_FIQH_START)  )
#define FIQH_SPRANGE_V_FL_F1        (0x1 << (E_FIQ_SPRANGE_V_FL_F1      -E_FIQH_START)  )
#define FIQH_VSYN_COP               (0x1 << (E_FIQ_VSYN_COP             -E_FIQH_START)  )
#define FIQH_VSYN_DC1               (0x1 << (E_FIQ_VSYN_DC1             -E_FIQH_START)  )
#define FIQH_VSYN_DC0               (0x1 << (E_FIQ_VSYN_DC0             -E_FIQH_START)  )
*/

#define FIQH_DSP2UP                 (0x1 << (E_FIQ_DSP2UP               -E_FIQH_START)  )
#define FIQH_VSYN_GOP1              (0x1 << (E_FIQ_VSYN_GOP1            -E_FIQH_START)  )
#define FIQH_VSYN_GOP0              (0x1 << (E_FIQ_VSYN_GOP0            -E_FIQH_START)  )

#define FIQH_IR                     (0x1 << (E_FIQ_IR                   -E_FIQH_START)   )

#define FIQH_ALL                    0xFFFF //[15:0]


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

#endif
#endif // _REG_SYSTEM_H_

