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
/// file    mhal_chiptop_reg.h
/// @brief  Chip Top Registers Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MHAL_CHIPTOP_REG_H_
#define _MHAL_CHIPTOP_REG_H_


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
//dhjung LGE BIT->_BIT
#define _BIT(x)                      (1<<(x))
#define BMASK(bits)                 (_BIT(((1)?bits)+1)-_BIT(((0)?bits)))
#define BITS(bits,value)            ((_BIT(((1)?bits)+1)-_BIT(((0)?bits))) & (value<<((0)?bits)))

#if defined(CONFIG_Titania3)
#define REG_TOP_BASE                0xBF203C00
#endif

#if defined(CONFIG_Titania2)
#define REG_TOP_BASE                0xBF803C00
#endif

#define TOP_REG(addr)               (*((volatile u32*)(REG_TOP_BASE + ((addr)<<2))))


#define REG_TOP_CHIP_CONFIG_OW      0x0000
#define REG_TOP_UTMI_UART_SETHL     0x0001
#define REG_TOP_USB_UART            0x0002
#define REG_TOP_PWM                 0x0003
#define REG_TOP_SPI_PAD             0x0004
#define REG_TOP_PCI                 0x0005
#define REG_TOP_POFF_MCU            0x0010
#define REG_TOP_TS0_TS1_MUX         0x0011
    #define TOP_TS0MUX_MASK                 BMASK(1:0)
    #define TOP_TS0MUX_PAD0                 BITS(1:0, 0)
    #define TOP_TS0MUX_PAD1                 BITS(1:0, 1)
    #define TOP_TS0MUX_DVBC                 BITS(1:0, 2)
    #define TOP_TS1MUX_MASK                 BMASK(9:8)
    #define TOP_TS1MUX_PAD0                 BITS(9:8, 0)
    #define TOP_TS1MUX_PAD1                 BITS(9:8, 1)
    #define TOP_TS1MUX_DVBC                 BITS(9:8, 2)

#define REG_TOP_PSRAM0_1_MIUMUX            0x002D   //TODO
    #define TOP_CKG_PSRAM0_MASK                 BMASK(1:0)
    #define TOP_CKG_PSRAM0_DIS                  _BIT(0)
    #define TOP_CKG_PSRAM0_INV                  _BIT(1)
    #define TOP_CKG_PSRAM1_MASK                 BMASK(3:2)
    #define TOP_CKG_PSRAM1_DIS                  _BIT(0)
    #define TOP_CKG_PSRAM1_INV                  _BIT(1)
    #define TOP_MIU_MUX_G07_MASK                BMASK(7:6)
	#define TOP_MIU_MUX_G07_OD_LSB_R            BITS(7:6,0)
	#define TOP_MIU_MUX_G07_GOP2_R              BITS(7:6,1)
    #define TOP_MIU_MUX_G08_MASK                BMASK(9:8)
	#define TOP_MIU_MUX_G08_OD_LSB_W            BITS(9:8,0)
	#define TOP_MIU_MUX_G08_VE_W                BITS(9:8,1)
    #define TOP_MIU_MUX_G15_MASK                BMASK(11:10)
	#define TOP_MIU_MUX_G15_GOP2_R              BITS(11:10,0)
	#define TOP_MIU_MUX_G15_OD_LSB_R            BITS(11:10,1)
    #define TOP_MIU_MUX_G1A_MASK                BMASK(13:12)
	#define TOP_MIU_MUX_G1A_VE_W                BITS(13:12,0)
	#define TOP_MIU_MUX_G1A_OD_LSB_W            BITS(13:12,1)
    #define TOP_MIU_MUX_G26_MASK                BMASK(15:14)
	#define TOP_MIU_MUX_G26_RVD_RW              BITS(15:14,0)
	#define TOP_MIU_MUX_G26_SVD_INTP_R          BITS(15:14,1)
	#define TOP_MIU_MUX_G26_MVD_R               BITS(15:14,2)

#define REG_TOP_TS1_GPIO_IN          0x0025
	#define TOP_CKG_TS1_GPIO_IN                   BMASK(10:0)

#define REG_TOP_TS1_GPIO_OUT         0x0026
#define REG_TOP_TS1_GPIO_OEN         0x0027
	#define TOP_CKG_TS1_GPIO_OEN                  BMASK(10:0)

#define REG_TOP_GPIO_OEN             0x002E

#define REG_TOP_VPU             0x0030
    #define TOP_CKG_VPU_MASK                  BMASK(6:0)
    #define TOP_CKG_VPU_DIS                   _BIT(0)
    #define TOP_CKG_VPU_INV                   _BIT(1)
    #define TOP_CKG_VPU_CLK_MASK              BMASK(6:2)
    #define TOP_CKG_VPU_144MHZ                BITS(6:2,0)
    #define TOP_CKG_VPU_123MHZ                BITS(6:2,1)
    #define TOP_CKG_VPU_108MHZ                BITS(6:2,2)
    #define TOP_CKG_VPU_96MHZ                BITS(6:2,3)
    #define TOP_CKG_VPU_72MHZ                BITS(6:2,4)

#define REG_TOP_HVD             0x0031
    #define TOP_CKG_HVD_MASK                  BMASK(5:0)
    #define TOP_CKG_HVD_DIS                   _BIT(0)
    #define TOP_CKG_HVD_INV                   _BIT(1)
    #define TOP_CKG_HVD_CLK_MASK              BMASK(5:2)
    #define TOP_CKG_HVD_144MHZ                BITS(5:2,0)
    #define TOP_CKG_HVD_108MHZ                BITS(5:2,1)
    #define TOP_CKG_HVD_72MHZ                BITS(5:2,2)
    #define TOP_CKG_HVD_XTAL                  BITS(5:2,3)
    #define TOP_CKG_HVD_MIU                   BITS(5:2,4)

#define REG_TOP_TS0_GPIO_IN          0x0031
	#define TOP_CKG_TS0_GPIO_IN                   BMASK(10:0)

#define REG_TOP_TS0_GPIO_OUT         0x0032
#define REG_TOP_TS0_GPIO_OEN         0x0034
	#define TOP_CKG_TS0_GPIO_OEN                  BMASK(10:0)

#define REG_TOP_I2S_GPIO_IN          0x0035
	#define TOP_CKG_I2S_GPIO_IN                   BMASK(11:0)

#define REG_TOP_I2S_GPIO_OUT         0x0036
	#define TOP_CKG_I2S_GPIO_OUT                  BMASK(11:0)

#define REG_TOP_I2S_GPIO_OEN         0x0037
	#define TOP_CKG_I2S_GPIO_OEN                  BMASK(11:0)

#define REG_TOP_PCM_GPIO_IN_LO       0x0038
#define REG_TOP_PCM_GPIO_IN_MID      0x0039
#define REG_TOP_PCM_GPIO_IN_HI_PCM2_GPIO_IN         0x003A
    #define TOP_CKG_PCM_GPIO_IN_HI                _BIT(0)
	#define TOP_CKG_PCM2_GPIO_IN                  BMASK(12:8)

#define REG_TOP_PCM_GPIO_OUT_LO      0x003B
#define REG_TOP_PCM_GPIO_OUT_MID     0x003C
#define REG_TOP_PCM_GPIO_OUT_HI_PCM2_GPIO_OUT        0x003D
    #define TOP_CKG_PCM_GPIO_OUT                   _BIT(0)
	#define TOP_CKG_PCM2_GPIO_OUT                  BMASK(12:8)

#define REG_TOP_PCM_GPIO_OEN_LO      0x003E
#define REG_TOP_PCM_GPIO_OEN_MID     0x003F
#define REG_TOP_PCM_GPIO_OEN_HI_PCM2_GPIO_OEN        0x0040
    #define TOP_CKG_PCM_GPIO_OEN_HI                _BIT(0)
	#define TOP_CKG_PCM2_GPIO_OEN                  BMASK(12:8)

#define REG_TOP_ET_GPIO_IN              0x0041
#define REG_TOP_ET_GPIO_OUT             0x0042
#define REG_TOP_ET_GPIO_OEN             0x0043

#define REG_TOP_PF_GPIO_IN              0x0044
#define REG_TOP_PF_GPIO_OUT             0x0045
#define REG_TOP_PF_GPIO_OEN             0x0046

//#define REG_TOP_GPIO_WN_OEN_DVBC    0x0032
//    #define TOP_GPIO_W0                         _BIT(0)
//    #define TOP_GPIO_W1                         _BIT(1)
//    #define TOP_GPIO_N0                         _BIT(4)
//    #define TOP_GPIO_N1                         _BIT(5)
//    #define TOP_GPIO_N2                         _BIT(6)
//    #define TOP_GPIO_N3                         _BIT(7)
//    #define TOP_CKG_DVBC_DMA_MASK               BMASK(11:8)
//    #define TOP_CKG_DVBC_DMA_EQ                 BITS(11:8, 0)
//    #define TOP_CKG_DVBC_DMA_ADCD               BITS(11:8, 1)
//    #define TOP_CKG_DVBC_DMA_0                  BITS(11:8, 2)
//    #define TOP_CKG_DVBC_DMA_DFT                BITS(11:8, 3)

#define REG_TOP_TCON_TS1            0x0052
    #define TOP_TS1_OUT                         _BIT(12)

#define REG_TOP_UART                0x0053

#define REG_TOP_STAT_BOND           0x0060
#define REG_TOP_BOND_OV_KEY         0x0063
#define REG_TOP_CHIP_CONFIG_STAT    0x0065
#define REG_TOP_DEVICE_ID           0x0066
#define REG_TOP_CHIP_VERSION        0x0067
    #define CHIP_VERSION_SHFT                   0
    #define CHIP_VERSION_MASK                   BMASK(7:0)
    #define CHIP_REVISION_SHFT                  8
    #define CHIP_REVISION_MASK                  BMASK(15:8)

#define REG_TOP_TESTBUS             0x0075
    #define TOP_TESTBUS_EN                      _BIT(14)

#define REG_CLK_EJ_MIPS_TSJ         0x0076
    #define TOP_TESTCLK_OUT_MASK                BMASK(2:0)
    #define TOP_TESTCLK_OUT_NONE                BITS(2:0, 0)

#define REG_TOP_RESET_CPU0          0x0077
    #define TOP_RESET_PASSWD_MASK               BMASK(14:0)
    #define TOP_RESET_CPU0                      _BIT(15)


//-------------------------------------------------------------------------------------------------
//  CLKGEN0
//-------------------------------------------------------------------------------------------------

#if defined(CONFIG_Titania3)
#define REG_CLKGEN0_BASE                0xBF201600
#endif


#define CKLGEN0_REG(addr)               (*((volatile u32*)(REG_CLKGEN0_BASE + ((addr)<<2))))

#define REG_CLKGEN0_STC0_DC0        0x0005
    #define CLKGEN0_STC0SYNC                    _BIT(0)
    #define CLKGEN0_STC0_CW_SEL                 _BIT(1)
    #define CLKGEN0_UPDATE_STC0_CW              _BIT(2)
    #define CLKGEN0_UPDATE_DC0_FREERUN_CW       _BIT(3)
    #define CLKGEN0_UPDATE_DC0_SYNC_CW          _BIT(4)

#define REG_CLKGEN0_STC0_SYNC_CW_L  0x0006
#define REG_CLKGEN0_STC0_SYNC_CW_H  0x0007

#define REG_CLKGEN0_DC0_NUM         0x000A
#define REG_CLKGEN0_DC0_DEN         0x000B

#define REG_CLKGEN0_AEON            0x0012
    #define CLKGEN0_CKG_AEON_MASK               BMASK(1:0)
    #define CLKGEN0_CKG_AEON_DIS                _BIT(0)
    #define CLKGEN0_CKG_AEON_INV                _BIT(1)
    #define CLKGEN0_CKG_AEON_SRC_MASK           BMASK(4:2)
    #define CLKGEN0_CKG_AEON_SRC_240            BITS(4:2, 0)
    #define CLKGEN0_CKG_AEON_SRC_170            BITS(4:2, 1)
    #define CLKGEN0_CKG_AEON_SRC_160            BITS(4:2, 2)
    #define CLKGEN0_CKG_AEON_SRC_144            BITS(4:2, 3)
    #define CLKGEN0_CKG_AEON_SRC_123            BITS(4:2, 4)
    #define CLKGEN0_CKG_AEON_SRC_108            BITS(4:2, 5)
    #define CLKGEN0_CKG_AEON_SRC_MEM_CLK        BITS(4:2, 6)
    #define CLKGEN0_CKG_AEON_SRC_MEM_CLK_DIV    BITS(4:2, 7)
    #define CLKGEN0_CKG_SW_AEON_CLK             _BIT(7)

#define REG_CLKGEN0_TS              0x0028
    #define CLKGEN0_CKG_TS0_MASK                BMASK(1:0)
    #define CLKGEN0_CKG_TS0_DIS                 _BIT(0)
    #define CLKGEN0_CKG_TS0_INV                 _BIT(1)
    #define CLKGEN0_CKG_TS0_SRC_MASK            BMASK(3:2)
    #define CLKGEN0_CKG_TS0_SRC_TS0             BITS(3:2, 0)
    #define CLKGEN0_CKG_TS0_SRC_TS1             BITS(3:2, 1)
    #define CLKGEN0_CKG_TS0_SRC_DMD             BITS(3:2, 2)
    #define CLKGEN0_CKG_TS0_SRC_XTAL            BITS(3:2, 3)
    #define CLKGEN0_CKG_TS1_MASK                BMASK(9:8)
    #define CLKGEN0_CKG_TS1_DIS                 _BIT(8)
    #define CLKGEN0_CKG_TS1_INV                 _BIT(9)
    #define CLKGEN0_CKG_TS1_SRC_MASK            BMASK(11:10)
    #define CLKGEN0_CKG_TS1_SRC_TS0             BITS(11:10, 0)
    #define CLKGEN0_CKG_TS1_SRC_TS1             BITS(11:10, 1)
    #define CLKGEN0_CKG_TS1_SRC_DMD             BITS(11:10, 2)
    #define CLKGEN0_CKG_TS1_SRC_XTAL            BITS(11:10, 3)

#define REG_CLKGEN0_TSP_TSOUT       0x0029
    #define CLKGEN0_CKG_TSOUT_MASK              BMASK(9:8)
    #define CLKGEN0_CKG_TSOUT_DIS               _BIT(8)
    #define CLKGEN0_CKG_TSOUT_INV               _BIT(9)
    #define CLKGEN0_CKG_TSOUT_SRC_MASK          BMASK(11:10)
    #define CLKGEN0_CKG_TSOUT_SRC_27            BITS(11:10, 0)
    #define CLKGEN0_CKG_TSOUT_SRC_36            BITS(11:10, 1)
    #define CLKGEN0_CKG_TSOUT_SRC_43            BITS(11:10, 2)
    #define CLKGEN0_CKG_TSOUT_SRC_XTAL          BITS(11:10, 3)

#define REG_CLKGEN0_TSP_STC0        0x002A
    #define CLKGEN0_CKG_TSP_MASK                BMASK(1:0)
    #define CLKGEN0_CKG_TSP_DIS                 _BIT(0)
    #define CLKGEN0_CKG_TSP_INV                 _BIT(1)
    #define CLKGEN0_CKG_TSP_SRC_MASK            BMASK(4:2)
    #define CLKGEN0_CKG_TSP_SRC_144             BITS(4:2, 0)
    #define CLKGEN0_CKG_TSP_SRC_123             BITS(4:2, 1)
    #define CLKGEN0_CKG_TSP_SRC_108             BITS(4:2, 2)
    #define CLKGEN0_CKG_TSP_SRC_72              BITS(4:2, 3)
    #define CLKGEN0_CKG_TSP_SRC_XTAL            BITS(4:2, 7)
    #define CLKGEN0_CKG_STC0_MASK               BMASK(9:8)
    #define CLKGEN0_CKG_STC0_DIS                _BIT(8)
    #define CLKGEN0_CKG_STC0_INV                _BIT(9)
    #define CLKGEN0_CKG_STC0_SRC_MASK           BMASK(11:10)
    #define CLKGEN0_CKG_STC0_SRC_SYNTH          BITS(11:10, 0)
    #define CLKGEN0_CKG_STC0_SRC_27             BITS(11:10, 2)
    #define CLKGEN0_CKG_STC0_SRC_XTAL           BITS(11:10, 3)

#define REG_CLKGEN0_MVD_SYNC			0x0038
    #define CLKGEN0_CKG_MVD_SYNC_DIS				_BIT(0)

#define REG_CLKGEN0_MVD             0x0039
    #define CLKGEN0_CKG_MVD_MASK                BMASK(3:0)
    #define CLKGEN0_CKG_MVD_DIS                 _BIT(0)
    #define CLKGEN0_CKG_MVD_INV                 _BIT(1)
    #define CLKGEN0_CKG_MVD_SRC_MASK            BMASK(3:2)
    #define CLKGEN0_CKG_MVD_SRC_144             BITS(3:2, 0)
    #define CLKGEN0_CKG_MVD_SRC_123             BITS(3:2, 1)
    #define CLKGEN0_CKG_MVD_SRC_MIU             BITS(3:2, 2)
    #define CLKGEN0_CKG_MVD_SRC_XTAL            BITS(3:2, 3)
    #define CLKGEN0_CKG_MVD2_MASK                BMASK(11:8)
    #define CLKGEN0_CKG_MVD2_DIS                 _BIT(8)
    #define CLKGEN0_CKG_MVD2_INV                 _BIT(9)
    #define CLKGEN0_CKG_MVD2_SRC_MASK            BMASK(11:10)
    #define CLKGEN0_CKG_MVD2_SRC_170            BITS(11:10, 0)
    #define CLKGEN0_CKG_MVD2_SRC_144            BITS(11:10, 1)
    #define CLKGEN0_CKG_MVD2_SRC_160            BITS(11:10, 2)
    #define CLKGEN0_CKG_MVD2_SRC_XTAL           BITS(11:10, 3)

#define REG_CLKGEN0_MVD_CHROMA_LUMA0		0x003a
    #define CLKGEN0_CKG_MVD_CHROMA_MASK			BMASK(1:0)
    #define CLKGEN0_CKG_MVD_CHROMA_DIS			_BIT(0)
    #define CLKGEN0_CKG_MVD_CHROMA_INV			_BIT(1)
    #define CLKGEN0_CKG_MVD_LUMA0_MASK			BMASK(9:8)
    #define CLKGEN0_CKG_MVD_LUMA0_DIS			_BIT(8)
    #define CLKGEN0_CKG_MVD_LUMA0_INV			_BIT(9)

#define REG_CLKGEN0_MVD_LUMA_1_2			0x003b
    #define CLKGEN0_CKG_MVD_LUMA1_MASK			BMASK(1:0)
    #define CLKGEN0_CKG_MVD_LUMA1_DIS			_BIT(0)
    #define CLKGEN0_CKG_MVD_LUMA1_INV			_BIT(1)
    #define CLKGEN0_CKG_MVD_LUMA2_MASK			BMASK(9:8)
    #define CLKGEN0_CKG_MVD_LUMA2_DIS			_BIT(8)
    #define CLKGEN0_CKG_MVD_LUMA2_INV			_BIT(9)

#define REG_CLKGEN0_MVD_RMEM       0x003c
    #define CLKGEN0_CKG_MVD_RMEM0_MASK          BMASK(1:0)
    #define CLKGEN0_CKG_MVD_RMEM0_DIS           _BIT(0)
    #define CLKGEN0_CKG_MVD_RMEM0_INV           _BIT(1)
    #define CLKGEN0_CKG_MVD_RMEM1_MASK          BMASK(9:8)
    #define CLKGEN0_CKG_MVD_RMEM1_DIS           _BIT(8)
    #define CLKGEN0_CKG_MVD_RMEM1_INV           _BIT(9)

#define REG_CLKGEN0_MVD_INTPRAM    0x003d
    #define CLKGEN0_CKG_MVD_INTPRAM0_MASK      BMASK(1:0)
    #define CLKGEN0_CKG_MVD_INTPRAM0_DIS       _BIT(0)
    #define CLKGEN0_CKG_MVD_INTPRAM0_INV       _BIT(1)
    #define CLKGEN0_CKG_MVD_INTPRAM1_MASK      BMASK(9:8)
    #define CLKGEN0_CKG_MVD_INTPRAM1_DIS       _BIT(8)
    #define CLKGEN0_CKG_MVD_INTPRAM1_INV       _BIT(9)

#define REG_CLKGEN0_MVD_RW_REFDAT 0x003e
    #define CLKGEN0_CKG_MVD_RREFDAT_MASK       BMASK(1:0)
    #define CLKGEN0_CKG_MVD_RREFDAT_DIS        _BIT(0)
    #define CLKGEN0_CKG_MVD_RREFDAT_INV        _BIT(1)
    #define CLKGEN0_CKG_MVD_WREFDAT_MASK       BMASK(9:8)
    #define CLKGEN0_CKG_MVD_WREFDAT_DIS        _BIT(8)
    #define CLKGEN0_CKG_MVD_WREFDAT_INV        _BIT(9)

#define REG_CLKGEN0_MVD_DPFF		 0x003f
    #define CLKGEN0_CKG_MVD_DPFF_MASK			BMASK(1:0)
    #define CLKGEN0_CKG_MVD_DPFF_DIS				_BIT(0)
    #define CLKGEN0_CKG_MVD_DPFF_INV				_BIT(1)

#define REG_CLKGEN0_PSRAM           0x0043
    #define CLKGEN0_CKG_PSRAM0_MASK             BMASK(1:0)
    #define CLKGEN0_CKG_PSRAM0_DIS              _BIT(0)
    #define CLKGEN0_CKG_PSRAM0_INV              _BIT(1)

#define REG_CLKGEN0_GE              0x0048
    #define CLKGEN0_CKG_GE_MASK                 BMASK(1:0)
    #define CLKGEN0_CKG_GE_DISABLE              _BIT(0)
    #define CLKGEN0_CKG_GE_INVERT               _BIT(1)
    #define CLKGEN0_CKG_GE_SRC_MASK             BMASK(3:2)
    #define CLKGEN0_CKG_GE_170MHZ               BITS(3:2, 0)
    #define CLKGEN0_CKG_GE_123MHZ               BITS(3:2, 1)
    #define CLKGEN0_CKG_GE_86MHZ                BITS(3:2, 2)
    #define CLKGEN0_CKG_GE_144MHZ               BITS(3:2, 3)


#define REG_CLKGEN0_DC              0x004C
    #define CLKGEN0_CKG_DC0_MASK                BMASK(1:0)
    #define CLKGEN0_CKG_DC0_DIS                 _BIT(0)
    #define CLKGEN0_CKG_DC0_INV                 _BIT(1)
    #define CLKGEN0_CKG_DC0_SRC_MASK            BMASK(4:2)
    #define CLKGEN0_CKG_DC0_SRC_SYNC            BITS(4:2, 0)
    #define CLKGEN0_CKG_DC0_SRC_FREERUN         BITS(4:2, 1)
    #define CLKGEN0_CKG_DC0_SRC_27              BITS(4:2, 2)
    #define CLKGEN0_CKG_DC0_SRC_54              BITS(4:2, 3)
    #define CLKGEN0_CKG_DC0_SRC_72              BITS(4:2, 4)
    #define CLKGEN0_CKG_DC0_SRC_160             BITS(4:2, 5)
    #define CLKGEN0_CKG_DC0_SRC_108             BITS(4:2, 6)
    #define CLKGEN0_CKG_DC0_SRC_144             BITS(4:2, 7)


#endif // _MHAL_CHIPTOP_REG_H_


