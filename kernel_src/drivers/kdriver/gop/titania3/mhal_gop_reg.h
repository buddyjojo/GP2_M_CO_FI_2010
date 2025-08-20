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

#ifndef REG_URANUS_GOP_H
#define REG_URANUS_GOP_H

#include "chip_setup.h"
#ifdef RED_LION
#include "mdrv_types.h"
#endif

#define T3  1
#define SUBTITLE_IN_IP 0
#define OSD3s 1
//-------------------------------------------------------------------------------------------------
// GOP Chip TopRegisters
//-------------------------------------------------------------------------------------------------
#if T3   //T3
#define CHIPTOP_ADDR                    (REG_MIPS_BASE + 0x580*4)
#define REG_CHIPTOP_CLK_GOP0_GOP1       (0x40*4 + CHIPTOP_ADDR)
#define REG_CHIPTOP_CLK_GOP2_GOPD       (0x41*4 + CHIPTOP_ADDR)
#define REG_CHIPTOP_CLK_GOP3            (0x42*4 + CHIPTOP_ADDR)

//dhjung LGE BIT->_BIT
//0x40  GOP4G
#define GOP0_DISABLE_SHIFT             (0)
#define GOP0_CLK_DISABLE               _BIT(GOP0_DISABLE_SHIFT) //bit0
#define GOP0_INVERT_SHIFT              (GOP0_DISABLE_SHIFT+1)   //shift 1
#define GOP0_CLK_INVERT                _BIT(GOP0_INVERT_SHIFT)  //bit1
#define GOP0_CLK_SEL_SHIFT             (GOP0_INVERT_SHIFT+1)    //shift 2
#define GOP0_CLK_SEL_BITS              (3)
#define GOP0_CLK_SEL_MASK              (_BIT_MASK(GOP0_CLK_SEL_BITS)<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_ODCLK             (0<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_IP                (1<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_IDCLK2P           (2<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_IDCLK1            (3<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_IDCLK1P           (4<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_RESERVE1          (5<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_RESERVE2          (6<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_DFT               (7<<GOP0_CLK_SEL_SHIFT)
//GOP2G
#define GOP1_DISABLE_SHIFT             (8)
#define GOP1_CLK_DISABLE               _BIT(GOP1_DISABLE_SHIFT) //bit8
#define GOP1_INVERT_SHIFT              (GOP1_DISABLE_SHIFT+1)   //shift 9
#define GOP1_CLK_INVERT                _BIT(GOP1_INVERT_SHIFT)  //bit9
#define GOP1_CLK_SEL_SHIFT             (GOP1_INVERT_SHIFT+1)    //shift 10
#define GOP1_CLK_SEL_BITS              (3)
#define GOP1_CLK_SEL_MASK              (_BIT_MASK(GOP1_CLK_SEL_BITS)<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_ODCLK             (0<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_IP                (1<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_IDCLK2P           (2<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_IDCLK1            (3<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_IDCLK1P           (4<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_RESERVE1          (5<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_RESERVE2          (6<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_DFT               (7<<GOP1_CLK_SEL_SHIFT)

//0x41  GOP1G
#define GOP2_DISABLE_SHIFT             (0)
#define GOP2_CLK_DISABLE               _BIT(GOP2_DISABLE_SHIFT) //bit0
#define GOP2_INVERT_SHIFT              (GOP2_DISABLE_SHIFT+1)   //shift 1
#define GOP2_CLK_INVERT                _BIT(GOP2_INVERT_SHIFT)  //bit1
#define GOP2_CLK_SEL_SHIFT             (GOP2_INVERT_SHIFT+1)    //shift 2
#define GOP2_CLK_SEL_BITS              (3)
#define GOP2_CLK_SEL_MASK              (_BIT_MASK(GOP2_CLK_SEL_BITS)<<GOP2_CLK_SEL_SHIFT)
#define GOP2_CLK_SEL_ODCLK             (0<<GOP2_CLK_SEL_SHIFT)
#define GOP2_CLK_SEL_IP                (1<<GOP2_CLK_SEL_SHIFT)
#define GOP2_CLK_SEL_IDCLK2P           (2<<GOP2_CLK_SEL_SHIFT)
#define GOP2_CLK_SEL_IDCLK1            (3<<GOP2_CLK_SEL_SHIFT)
#define GOP2_CLK_SEL_IDCLK1P           (4<<GOP2_CLK_SEL_SHIFT)
#define GOP2_CLK_SEL_RESERVE1          (5<<GOP2_CLK_SEL_SHIFT)
#define GOP2_CLK_SEL_RESERVE2          (6<<GOP2_CLK_SEL_SHIFT)
#define GOP2_CLK_SEL_DFT               (7<<GOP2_CLK_SEL_SHIFT)

#define GOPD_DISABLE_SHIFT             (8)
#define GOPD_CLK_DISABLE               _BIT(GOPD_DISABLE_SHIFT) //bit8
#define GOPD_INVERT_SHIFT              (GOPD_DISABLE_SHIFT+1)   //shift 9
#define GOPD_CLK_INVERT                _BIT(GOPD_INVERT_SHIFT)  //bit9
#define GOPD_CLK_SEL_SHIFT             (GOPD_INVERT_SHIFT+1)    //shift 10
#define GOPD_CLK_SEL_BITS              (2)
#define GOPD_CLK_SEL_MASK              (_BIT_MASK(GOPD_CLK_SEL_BITS)<<GOPD_CLK_SEL_SHIFT)
#define GOPD_CLK_SEL_ADC               (0<<GOPD_CLK_SEL_SHIFT)
#define GOPD_CLK_SEL_ODCLK             (1<<GOPD_CLK_SEL_SHIFT)
#define GOPD_CLK_SEL_DC0               (2<<GOPD_CLK_SEL_SHIFT)
#define GOPD_CLK_SEL_DFT               (3<<GOPD_CLK_SEL_SHIFT)

//0x42  GOP1GX
#define GOP3_DISABLE_SHIFT             (0)
#define GOP3_CLK_DISABLE               _BIT(GOP3_DISABLE_SHIFT) //bit0
#define GOP3_INVERT_SHIFT              (GOP3_DISABLE_SHIFT+1)   //shift 1
#define GOP3_CLK_INVERT                _BIT(GOP0_INVERT_SHIFT)  //bit1
#define GOP3_CLK_SEL_SHIFT             (GOP3_INVERT_SHIFT+1)    //shift 2
#define GOP3_CLK_SEL_BITS              (3)
#define GOP3_CLK_SEL_MASK              (_BIT_MASK(GOP3_CLK_SEL_BITS)<<GOP3_CLK_SEL_SHIFT)
#define GOP3_CLK_SEL_ODCLK             (0<<GOP3_CLK_SEL_SHIFT)
#define GOP3_CLK_SEL_IP                (1<<GOP3_CLK_SEL_SHIFT)
#define GOP3_CLK_SEL_IDCLK2P           (2<<GOP3_CLK_SEL_SHIFT)
#define GOP3_CLK_SEL_IDCLK1            (3<<GOP3_CLK_SEL_SHIFT)
#define GOP3_CLK_SEL_IDCLK1P           (4<<GOP3_CLK_SEL_SHIFT)
#define GOP3_CLK_SEL_RESERVE1          (5<<GOP3_CLK_SEL_SHIFT)
#define GOP3_CLK_SEL_RESERVE2          (6<<GOP3_CLK_SEL_SHIFT)
#define GOP3_CLK_SEL_DFT               (7<<GOP3_CLK_SEL_SHIFT)

#else
#define CHIPTOP_ADDR                   (REG_MIPS_BASE + 0xF00*4)
#define REG_CHIPTOP_CLK_GOP0_GOP1_GOPD (0x16*4 + CHIPTOP_ADDR)

//dhjung LGE BIT->_BIT
#define GOP0_DISABLE_SHIFT             (0)
#define GOP0_CLK_DISABLE               _BIT(GOP0_DISABLE_SHIFT)//bit0
#define GOP0_INVERT_SHIFT              (GOP0_DISABLE_SHIFT+1)//1
#define GOP0_CLK_INVERT                _BIT(GOP0_INVERT_SHIFT)//bit1
#define GOP0_CLK_SEL_SHIFT             (GOP0_INVERT_SHIFT+1)//2
#define GOP0_CLK_SEL_BITS              (2)
#define GOP0_CLK_SEL_MASK              (_BIT_MASK(GOP0_CLK_SEL_BITS)<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_ODCLK             (0<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_IP                (2<<2)//(1<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_IDCLK             (2<<GOP0_CLK_SEL_SHIFT)
#define GOP0_CLK_SEL_DFT               (3<<GOP0_CLK_SEL_SHIFT)

#define GOP1_DISABLE_SHIFT             (GOP0_CLK_SEL_SHIFT+GOP0_CLK_SEL_BITS)//4
#define GOP1_CLK_DISABLE               _BIT(GOP1_DISABLE_SHIFT)//bit4
#define GOP1_INVERT_SHIFT              (GOP1_DISABLE_SHIFT+1)//5
#define GOP1_CLK_INVERT                _BIT(GOP1_INVERT_SHIFT)//bit5
#define GOP1_CLK_SEL_SHIFT             (GOP1_INVERT_SHIFT+1)//6
#define GOP1_CLK_SEL_BITS              (2)
#define GOP1_CLK_SEL_MASK              (_BIT_MASK(GOP1_CLK_SEL_BITS)<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_ODCLK             (0<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_IP                (2<<6)// (1<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_IDCLK             (2<<GOP1_CLK_SEL_SHIFT)
#define GOP1_CLK_SEL_DFT               (3<<GOP1_CLK_SEL_SHIFT)

#define GOPD_DISABLE_SHIFT             (GOP1_CLK_SEL_SHIFT+GOP1_CLK_SEL_BITS)
#define GOPD_CLK_DISABLE               _BIT(GOPD_DISABLE_SHIFT)
#define GOPD_INVERT_SHIFT              (GOPD_DISABLE_SHIFT+1)
#define GOPD_CLK_INVERT                _BIT(GOPD_INVERT_SHIFT)
#define GOPD_CLK_SEL_SHIFT             (GOPD_INVERT_SHIFT+1)
#define GOPD_CLK_SEL_BITS              (2)
#define GOPD_CLK_SEL_MASK              (_BIT_MASK(GOPD_CLK_SEL_BITS)<<GOPD_CLK_SEL_SHIFT)
#define GOPD_CLK_SEL_ADC               (0<<GOPD_CLK_SEL_SHIFT)
#define GOPD_CLK_SEL_ODCLK             (1<<GOPD_CLK_SEL_SHIFT)
#define GOPD_CLK_SEL_DC0               (2<<GOPD_CLK_SEL_SHIFT)
#define GOPD_CLK_SEL_DFT               (3<<GOPD_CLK_SEL_SHIFT)


#define REG_CHIPTOP_CLK_GOPS           (0x18*4 + CHIPTOP_ADDR)  //(0x17*4 + CHIPTOP_ADDR)
#define GOPS_DISABLE_SHIFT             (8)
#define GOPS_CLK_DISABLE               _BIT(GOPS_DISABLE_SHIFT)
#define GOPS_INVERT_SHIFT              (GOPS_DISABLE_SHIFT+1)
#define GOPS_CLK_INVERT                _BIT(GOPS_INVERT_SHIFT)
#define GOPS_CLK_SEL_SHIFT             (GOPS_INVERT_SHIFT+1)
#define GOPS_CLK_SEL_BITS              (2)
#define GOPS_CLK_SEL_MASK              (_BIT_MASK(GOPS_CLK_SEL_BITS)<<GOPS_CLK_SEL_SHIFT)
#define GOPS_CLK_SEL_ODCLK             (0<<GOPS_CLK_SEL_SHIFT)
#define GOPS_CLK_SEL_IP                (1<<GOPS_CLK_SEL_SHIFT)
#define GOPS_CLK_SEL_IDCLK             (2<<GOPS_CLK_SEL_SHIFT)
#define GOPS_CLK_SEL_DFT               (3<<GOPS_CLK_SEL_SHIFT)

#endif

#define MIU_ADRESS                      (REG_MIPS_BASE + 0x900*4)
#define REG_GOP_MIUSEL              (0x79*4+MIU_ADRESS)
typedef struct
{
    U32 otherIP0:       5;
    U32 gop0_miusel:    1;
    U32 gop1_miusel:    1;
    U32 gop2_miusel:    1;
    U32 gop3_miusel:    1;
    U32 otherIP1:       7;
    U32 reserved:       16;
} GOP_MIU_SEL;

//-------------------------------------------------------------------------------------------------
// GOP4G_0 Registers
//-------------------------------------------------------------------------------------------------
#if T3
#define CHIP_ADRESS                     (0x580*4)
#else
#define CHIP_ADRESS                     (0xF00*4)
#endif
#define GOP_ADRESS                      (0xF80*4)
#define SC_ADRESS                       (0x1780*4)
#define REG_CHIP_BASE                   (REG_MIPS_BASE + CHIP_ADRESS)
#define REG_GOP_BASE                    (REG_MIPS_BASE + GOP_ADRESS)
#define REG_SC_BASE                     (REG_MIPS_BASE + SC_ADRESS)

#define REG_GOP_SOFT_RESET              (0x00*4 + (REG_GOP_BASE))

#define GOP_SOFT_RESET                  (_BIT_MASK(1)<<0)
#define GOP_VSYNC_INVERSE               (_BIT_MASK(1)<<1)
#define GOP_HSYNC_INVERSE               (_BIT_MASK(1)<<2)
#define GOP_GWIN_PROG_MD                (_BIT_MASK(1)<<3)
#define GOP_FIELD_INVERSE               (_BIT_MASK(1)<<4)
#define GOP_NEAREST_MODE                (_BIT_MASK(1)<<5)
#define GOP_RGB5541_ENABLE              (_BIT_MASK(1)<<7)
#define GOP_OUTPUT_READY                (_BIT_MASK(1)<<8)
#define GOP_GENSHOT_FASTT               (_BIT_MASK(1)<<9)
#define GOP_YUV_OUT                     (_BIT_MASK(1)<<10)
#define GOP_TRS_EN                      (_BIT_MASK(1)<<11)
#define GOP_DISP_HBACK                  (_BIT_MASK(1)<<12)
#define GOP_DISP_VBACK                  (_BIT_MASK(1)<<13)
//#define GOP_HSYNC_MASK                  (_BIT_MASK(1)<<14)
#define GOP_ALPHA_OUT_INVERSE           (_BIT_MASK(1)<<15)


typedef struct
{
    U32 SOFT_RESET:     1;  // soft reset
    U32 VSYNC_INVERSE:  1;
    U32 HSYNC_INVERSE:  1;
    U32 GWIN_PROG_MD:   1;
    U32 FIELD_INVERSE:  1;
    U32 NEAREST_MODE:   1;  // color pattern mode enable
    U32 reserved0:      1;
    U32 RGB5541_ENABLE: 1;
    U32 OUTPUT_READY:   1;
    U32 GENSHOT_FASTT:  1;
    U32 YUV_OUT:  1;
    U32 TRS_EN:         1;
    U32 DISP_HBACK:     1; // input Vsync type select(for op2) 0: VS / 1:VFDE
    U32 DISP_VBACK:     1; // only for alpha blink data type.
    U32 HSYNC_MASK:     1; // RGB5541 alpha mask mode enable, only for RGB1555
    U32 ALPHA_INVERSE:  1; // Trnasparent Color Enable.
    U32 reserved:       16;
} GOP_soft_reset;



///////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_DEST                    (0x01*4 + (REG_GOP_BASE))

#define GOP_DEST_SHIFT                  (0)
#define GOP_DEST_MASK                   (_BIT_MASK(3)<<GOP_DEST_SHIFT)
#define GOP_DMA_64_SHIFT                (4)
#define GOP_DMA_64_MASK                 (_BIT_MASK(1)<<GOP_DMA_64_SHIFT)
#define GOP_GWIN1_PINPON_G3D_MODE_SHIFT (5)
#define GOP_GWIN1_PINPON_G3D_MODE_MASK  (_BIT_MASK(1)<<GOP_GWIN1_PINPON_G3D_MODE_SHIFT)
#define GOP_GWIN1_PINPON_MODE_SHIFT     (7)
#define GOP_GWIN1_PINPON_MODE_MASK      (_BIT_MASK(1)<<GOP_GWIN1_PINPON_MODE_SHIFT)
#define GOP_RI_START_SHIFT              (8)
#define GOP_RI_START_MASK               (_BIT_MASK(4)<<GOP_RI_START_SHIFT)
#define GOP_RI_END_SHIFT                (12)
#define GOP_RI_END_MASK                 (_BIT_MASK(4)<<GOP_RI_END_SHIFT)

typedef struct
{
    U32 GOP_DEST:              3;
    U32 DMA_64:                1;
    U32 GWIN1_PINPON_G3D_MODE: 1;
    U32 reserved1:             1;
    U32 GWIN1_PINPON_MODE:     1; // --GOPS TYPE
    U32 RI_START:              4; // --GOPS TYPE
    U32 RI_END:                4; // --GOPS TYPE
    U32 reserved:             16;
} GOP_dest;




///////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_BLINK                   (0x02*4 + (REG_GOP_BASE))

#define GOP_ROLL_RATE_SHIFT             (0)
#define GOP_ROLL_RATE_MASK              (_BIT_MASK(6)<<GOP_ROLL_RATE_SHIFT)
#define GOP_BLINK_RATE_SHIFT            (8)
#define GOP_BLINK_RATE_MASK             (_BIT_MASK(7)<<GOP_BLINK_RATE_SHIFT)
#define GOP_BLINK_EN_SHIFT              (15)
#define GOP_BLINK_EN                    (_BIT_MASK(1)<<GOP_BLINK_EN_SHIFT)


typedef struct
{
    U32 ROLL_RATE:      6;
    U32 reserved0:      2;
    U32 BLINK_RATE:     7;
    U32 BLINK_EN:       1;
    U32 reserved:       16;
} GOP_blink;




///////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_PAL_DATA_LO             (0x03*4 + (REG_GOP_BASE))
#define GOP_PAL_DATA_LO_SHIFT           (0)
#define GOP_PAL_DATA_LO_MASK            (_BIT_MASK(16)<<GOP_PAL_DATA_LO_SHIFT)

typedef struct
{
    U32 PAL_DATA_LO:    16; //--GOPS ? NUMBER? 0~31, 128~159
    U32 reserved:       16;
} GOP_pal_data_lo;



///////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_PAL_DATA_HI             (0x04*4 + (REG_GOP_BASE))
#define GOP_PAL_DATA_HI_SHIFT           (0)
#define GOP_PAL_DATA_HI_MASK            (_BIT_MASK(8)<<GOP_PAL_DATA_HI_SHIFT)
typedef struct
{
    U32 PAL_DATA_HI:    16;
    U32 reserved:       16;
} GOP_pal_data_hi;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_PAL_SET                 (0x05*4 + (REG_GOP_BASE))

#define GOP_PAL_ADDR_SHIFT              (0)
#define GOP_PAL_ADDR_MASK               (_BIT_MASK(8)<<GOP_PAL_ADDR_SHIFT)
#define GOP_PAL_WRITE_SHIFT             (8)
#define GOP_PAL_WRITE_MASK              (1<<GOP_PAL_WRITE_SHIFT)
#define GOP_PAL_READ_SHIFT              (9)
#define GOP_PAL_READ_MASK               (1<<GOP_PAL_READ_SHIFT)
#define GOP_PAL_APART_SHIFT             (10)
#define GOP_PAL_APART_MASK              (1<<GOP_PAL_APART_SHIFT)
#define GOP_PAL_CTL_SHIFT               (12)
#define GOP_PAL_CTL_MASK                (_BIT_MASK(2)<<GOP_PAL_CTL_SHIFT)


typedef struct
{
    U32 PAL_ADDR:       8;
    U32 PAL_WRITE:      1;
    U32 PAL_READ:       1;
    U32 PAL_APART:      1;
    U32 reserved0:       1;
    U32 PAL_CTL:        2;
    U32 reserved:       18;
} GOP_pal_set;


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_REGDMA_START_END        (0x06*4 + (REG_GOP_BASE))
#define GOP_REGDMS_END_SHIFT            (0)
#define GOP_REGDMS_END_MASK             (_BIT_MASK(9)<<GOP_REGDMS_END_SHIFT)
#define GOP_REGDMS_START_SHIFT          (9)
#define GOP_REGDMS_START_MASK           (_BIT_MASK(15)<<GOP_REGDMS_END_SHIFT)

typedef struct
{
    U32 REGDMS_END:    9; //--GOPS ? NUMBER? 0~31, 128~159
    U32 REGDMS_START:  15;
    U32 reserved:      8;
} GOP_regdma_start_end;



///////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_INT                     (0x08*4 + (REG_GOP_BASE))

#define GOP_VS0_INT_MASK                (_BIT_MASK(1)<<0)
#define GOP_VS1_INT_MASK                (_BIT_MASK(1)<<1)
#define GOP_REGDMA_ERROR_INT_MASK       (_BIT_MASK(1)<<2)
#define GOP_VS0_INT                     (_BIT_MASK(1)<<8)
#define GOP_VS1_INT                     (_BIT_MASK(1)<<9)
#define GOP_REGDMA_ERROR_INT            (_BIT_MASK(1)<<10)

typedef struct
{
    U32 VS0_INT_MASK:          1;
    U32 VS1_INT_MASK:          1;
    U32 REGDMA_ERROR_INT_MASK: 1;
    U32 reserved0:             5;
    U32 VS0_INT:               1;
    U32 VS1_INT:               1;
    U32 REGDMA_ERROR_INT:      1;
    U32 reserved:              21;
} GOP_int;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_RDMA_HT                 (0x0E*4 + (REG_GOP_BASE))

typedef struct
{
    U32 RDMA_HT:               10;
    U32 reserved:              22;
} GOP_rdma_ht;

#define REG_GOP_HS_PIPE     (0x0F*4 + (REG_GOP_BASE))

typedef struct
{
    U32 HS_PIPE:                9;
    U32 reserved:              23;
}GOP_Hs_Pipe;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DMA                     (0x19*4 + (REG_GOP_BASE))
#define GOP_DMA_FIFO_THRESHOLD_SHIFT    (0)
#define GOP_DMA_FIFO_THRESHOLD_MASK     (_BIT_MASK(8)<<GOP_DMA_FIFO_THRESHOLD_SHIFT)
#define GOP_DMA_BURST_LENGTH_SHIFT      (8)
#define GOP_DMA_BURST_LENGTH_MASK       (_BIT_MASK(3)<<GOP_DMA_BURST_LENGTH_SHIFT)
#define GOP_DMA_HIGH_PRIO_THRESH_SHIFT  (14)
#define GOP_DMA_HIGH_PRIO_THRESH_MASK   (_BIT_MASK(2)<<GOP_DMA_HIGH_PRIO_THRESH_SHIFT)

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_GWIN_PRI                (0x20*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GWIN_PRI1:      2; // 1 PRIORITY GWIN NUMBER
    U32 GWIN_PRI2:      2; // 2 PRIORITY GWIN NUMBER
    U32 GWIN_PRI3:      2; // 3 PRIORITY GWIN NUMBER
    U32 GWIN_PRI4:      2; // 4 PRIORITY GWIN NUMBER
    U32 SRAM_BIST_FLAG: 4; //READ ONLY SRAM BIST FLAT (FOR IC TEST)
    U32 FIFO_STATE:     4; //READ ONLY, FIFO STATE
    U32 reserved:       16;
} GOP_gwin_pri;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_2266_MODE               (0x22*4 + (REG_GOP_BASE))
#define GOP_OLD_2266_MODE_SHIFT         (7)
#define GOP_OLD_2266_MODE_MASK          (_BIT_MASK(1)<<GOP_DMA_FIFO_THRESHOLD_SHIFT)


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_TRS_CLR_LO            (0x24*4 + (REG_GOP_BASE))

typedef struct
{
    U32 TRS_CLR_LO:     16; //TRS COLOR (0:15) {R,G,B}
    U32 reserved:       16;
}GOP_trs_clr_lo;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_TRS_CLR_HI            (0x25*4 + (REG_GOP_BASE))

typedef struct
{
    U32 TRS_CLR_HI:      8; //TRS COLOR (16:23) {R,G,B}
    U32 reserved:       24;
}GOP_trs_clr_hi;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_STRCH_HSIZE           (0x30*4 + (REG_GOP_BASE))

typedef struct
{
    U32 STRCH_HSIZE:    10;
    U32 reserved:       22;
}GOP_strch_hsize;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_STRCH_VSIZE           (0x31*4 + (REG_GOP_BASE))

typedef struct
{
    U32 STRCH_VSIZE:    11;
    U32 reserved:       21;
}GOP_strch_vsize;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_STRCH_HSTART          (0x32*4 + (REG_GOP_BASE))

typedef struct
{
    U32 STRCH_HSTART:   12;
    U32 reserved:       20;
}GOP_strch_hstart;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_STRCH_VSTART          (0x34*4 + (REG_GOP_BASE))

typedef struct
{
    U32 STRCH_VSTART:   11;
    U32 reserved:       21;
}GOP_strch_vstart;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_STRCH_HRATIO          (0x35*4 + (REG_GOP_BASE))

typedef struct
{
    U32 STRCH_HRATIO:   13;
    U32 reserved:       19;
}GOP_strch_hratio;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_STRCH_VRATIO          (0x36*4 + (REG_GOP_BASE))

typedef struct
{
    U32 STRCH_VRATIO:   13;
    U32 reserved:       19;
}GOP_strch_vratio;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_RBLK0_VOFF_LO           (0x60*4 + (REG_GOP_BASE))
#define GOP_RBLK0_VOFF_LO_SHIFT          (0)
#define GOP_RBLK0_VOFF_LO_MASK           (_BIT_MASK(16)<<GOP_RBLK_VOFF_LO_SHIFT)
typedef struct
{
    U32 GOP_RBLK0_VOFF_LO:   16; //GOP RBLK 0 V SCROLL OFFSET, READ ONLY
    U32 reserved:           16;
} GOP_rblk0_voff_lo;


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_RBLK0_VOFF_HI           (0x61*4 + (REG_GOP_BASE))
#define GOP_RBLK0_VOFF_HI_SHIFT          (0)
#define GOP_RBLK0_VOFF_HI_MASK           (_BIT_MASK(6)<<GOP_RBLK_VOFF_HI_SHIFT)
typedef struct
{
    U32 GOP_RBLK0_VOFF_HI:    6; //GOP RBLK 0 V SCROLL OFFSET, READ ONLY
    U32 reserved1:           2;
    U32 reserved2:           8;
    U32 reserved:           16;
} GOP_rblk0_voff_hi;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_RBLK0_HOFF              (0x70*4 + (REG_GOP_BASE))
#define GOP_RBLK0_HOFF_SHIFT            (0)
#define GOP_RBLK0_HOFF_MASK             (_BIT_MASK(9)<<GOP_RBLK_VOFF_HI_SHIFT)
typedef struct
{
    U32 GOP_RBLK0_HOFF:      9; //GOP RBLK 0 H SCROLL OFFSET, READ ONLY
    U32 reserved1:           7;
    U32 reserved:           16;
} GOP_rblk_hoff;


//OK/////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_MUX_SEL                 (0x7e*4 + (REG_GOP_BASE))
#define GOPG0_MUX_SEL_SHIFT             (0)
#define GOPG0_MUX_SEL_MASK              (_BIT_MASK(2)<<GOPG0_MUX_SEL_SHIFT)
#define GOPG1_MUX_SEL_SHIFT             (2)
#define GOPG1_MUX_SEL_MASK              (_BIT_MASK(2)<<GOPG1_MUX_SEL_SHIFT)
#define GOPG2_MUX_SEL_SHIFT             (4)
#define GOPG2_MUX_SEL_MASK              (_BIT_MASK(2)<<GOPG2_MUX_SEL_SHIFT)
#define GOP1GX_INT_SHIFT                (14)
#define GOP1GX_INT_MASK                 (_BIT_MASK(1)<<GOP1GX_INT_SHIFT)
#define GOP1GX_WR_ACK_SHIFT             (15)
#define GOP1GX_WR_ACK_MASK              (_BIT_MASK(1)<<GOP1GX_WR_ACK_SHIFT)


#define GOP_MUX_SEL_SHIFT(ID)          ((ID)*2))
#define GOP_MUX_SEL_MASK(ID)           (_BIT_MASK(2)<< GOP_MUX_SEL_SHIFT(ID))


typedef struct
{
    U32 GOPG0_MUX_SEL:         2;
    U32 GOPG1_MUX_SEL:         2;
    U32 GOPG2_MUX_SEL:         2;
    U32 GOPG3_MUX_SEL:         2;
    U32 reserved1:             8;
    U32 reserved:             16;
} GOP_mux_sel;

//OK/////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_WR_ACK                (0x7f*4 + (REG_GOP_BASE))

#define GOP_BANK_SEL_SHIFT            (0)
#define GOP_BANK_SEL_MASK             (_BIT_MASK(4)<<GOP_BANK_SEL_SHIFT)
#define GOP0_INT_FLAG_SHIFT           (4)
#define GOP0_INT_FLAG_MASK            (_BIT_MASK(1)<<GOP0_INT_FLAG_SHIFT)
#define GOP1_INT_FLAG_SHIFT           (5)
#define GOP1_INT_FLAG_MASK            (_BIT_MASK(1)<<GOP1_INT_FLAG_SHIFT)
#define GOPD_INT_FLAG_SHIFT           (6)
#define GOPD_INT_FLAG_MASK            (_BIT_MASK(1)<<GOPD_INT_FLAG_SHIFT)
#define GOPS_INT_FLAG_SHIFT           (7)
#define GOPS_INT_FLAG_MASK            (_BIT_MASK(1)<<GOPS_INT_FLAG_SHIFT)
#define GOP_WR_SHIFT                  (8)
#define GOP_WR_MASK                   (_BIT_MASK(1)<<GOP_WR_SHIFT)
#define GOP_FWR_SHIFT                 (9)
#define GOP_FWR_MASK                  (_BIT_MASK(1)<<GOP_FWR_SHIFT)
#define GOP_BK_WR_SHIFT               (10)
#define GOP_BK_WR_MASK                (_BIT_MASK(1)<<GOP_BK_WR_SHIFT)
#define GOP0_WR_ACK_SHIFT             (12)
#define GOP0_WR_ACK_MASK              (_BIT_MASK(1)<<GOP0_WR_ACK_SHIFT)
#define GOP1_WR_ACK_SHIFT             (13)
#define GOP1_WR_ACK_MASK              (_BIT_MASK(1)<<GOP1_WR_ACK_SHIFT)
#define GOPD_WR_ACK_SHIFT             (14)
#define GOPD_WR_ACK_MASK              (_BIT_MASK(1)<<GOPD_WR_ACK_SHIFT)
#define GOPS_WR_ACK_SHIFT             (15)
#define GOPS_WR_ACK_MASK              (_BIT_MASK(1)<<GOPS_WR_ACK_SHIFT)


typedef struct
{
    U32 GOP_BANK_SEL:          4; // 0:GOP0, 1:GOP1, 2:GOPD, 3:GOPS
    U32 GOP0_INT_FLAG:         1;
    U32 GOP1_INT_FLAG:         1;
    U32 GOPD_INT_FLAG:         1;
    U32 GOPS_INT_FLAG:         1;
    U32 GOP_WR:                1;
    U32 GOP_FWR:               1;
    U32 reserved2:             1;
    U32 GOP_FCLR:              1; //GOP FIFO CLEAR, DEBUG ONLY
    U32 GOP0_WR_ACK:           1; //GOP0 WRITE ACK
    U32 GOP1_WR_ACK:           1; //GOP1 WRITE ACK
    U32 GOPD_WR_ACK:           1; //GOPD WRITE ACK
    U32 GOPS_WR_ACK:           1; //GOPS WRITE ACK
    U32 reserved:             16;
}GOP_wr_ack;



///////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
// GOP4G_1 Registers
//-------------------------------------------------------------------------------------------------

#define  REG_GOP_GWIN0_EN                (0x00*4 + (REG_GOP_BASE))
#define  REG_GOP_GWIN1_EN                (0x20*4 + (REG_GOP_BASE))
#define  REG_GOP_GWIN2_EN                (0x40*4 + (REG_GOP_BASE))
#define  REG_GOP_GWIN3_EN                (0x60*4 + (REG_GOP_BASE))


#define GWIN_EN_SHIFT                 (0)
#define GWIN_EN_MASK                  (_BIT_MASK(1)<<GWIN_EN_SHIFT)
#define GWIN_RSTOP_SHIFT              (1)
#define GWIN_RSTOP_MASK               (_BIT_MASK(1)<<GWIN_RSTOP_SHIFT)
#define GWIN_VSCROLL_SHIFT            (2)
#define GWIN_VSCROLL_MASK             (_BIT_MASK(1)<<GWIN_VSCROLL_SHIFT)
#define GWIN_HSCROLL_SHIFT            (3)
#define GWIN_HSCROLL_MASK             (_BIT_MASK(1)<<GWIN_HSCROLL_SHIFT)
#define GWIN_DTYGE_SHIFT              (4)
#define GWIN_DTYGE_MASK               (_BIT_MASK(4)<<GWIN_DTYGE_SHIFT)
#define GWIN_ALPHA_SHIFT              (8)
#define GWIN_ALPHA_MASK               (_BIT_MASK(6)<<GWIN_ALPHA_SHIFT)
#define GWIN_ALPHA_EN_SHIFT           (14)
#define GWIN_ALPHA_EN_MASK            (_BIT_MASK(1)<<GWIN_ALPHA_EN_SHIFT)

typedef struct
{
    U32 GWIN_EN:            1; // 0:GOP0, 1:GOP1, 2:GOPD, 3:GOPS
    U32 GWIN_RSTOP:         1;
    U32 GWIN_VROLL:         1;
    U32 GWIN_HROLL:         1;
    U32 GWIN_DTYPE:         4;
    U32 GWIN_ALPHA:         6;
    U32 GWIN_ALPHA_EN:      1;
    U32 reserved:           17;
}GOP_gwin_set;


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DRAM_RBLK0_STR_LO       (0x01*4 + (REG_GOP_BASE))
#define GOP_DRAM_RBLK_STR_LO_SHIFT      (0)
#define GOP_DRAM_RBLK_STR_LO_MASK       (_BIT_MASK(16)<<GOP_DRAM_RBLK_STR_LO_SHIFT)

typedef struct
{
    U32 DRAM_RBLK_STR_LO:   16;
    U32 reserved:           16;
} GOP_dram_rblk_str_lo;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DRAM_RBLK0_STR_HI       (0x02*4 + (REG_GOP_BASE))
#define GOP_DRAM_RBLK_STR_HI_SHIFT      (0)
#define GOP_DRAM_RBLK_STR_HI_MASK       (_BIT_MASK(9)<<GOP_DRAM_RBLK_STR_HI_SHIFT)

typedef struct
{
    U32 DRAM_RBLK_STR_HI:   9;
    U32 reserved1:          7;
    U32 reserved:           16;
} GOP_dram_rblk_str_hi;



///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_GWIN0_HSTR              (0x04*4 + (REG_GOP_BASE))
#define GOP_GWIN_HSTR_SHIFT             (0)
#define GOP_GWIN_HSTR_MASK              (_BIT_MASK(10)<<GOP_GWIN_HSTR_SHIFT)

typedef struct
{
    U32 GWIN_HSTR:      10; // UINT: 64BIT = 16 BYTE ALIGNMENT
    U32 reserved1:      6;
    U32 reserved:       16;
} GOP_gwin_hstr;


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_GWIN0_HEND              (0x05*4 + (REG_GOP_BASE))
#define GOP_GWIN_HEND_SHIFT             (0)
#define GOP_GWIN_HEND_MASK              (_BIT_MASK(10)<<GOP_GWIN_HEND_SHIFT)

typedef struct
{
    U32 GWIN_HEND:      10;
    U32 reserved1:      6;
    U32 reserved:       16;
} GOP_gwin_hend;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_GOP_GWIN0_VSTR              (0x06*4 + (REG_GOP_BASE))
#define GOP_GWIN_VSTR_SHIFT             (0)
#define GOP_GWIN_VSTR_MASK              (_BIT_MASK(11)<<GOP_GWIN_VSTR_SHIFT)

typedef struct
{
    U32 GWIN_VSTR:      11; //UNIT: 1 LINE
    U32 reserved1:      5;
    U32 reserved:       16;
} GOP_gwin_vstr;


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_GWIN0_VEND              (0x08*4 + (REG_GOP_BASE))
#define GOP_GWIN_VEND_SHIFT             (0)
#define GOP_GWIN_VEND_MASK              (_BIT_MASK(11)<<GOP_GWIN_VEND_SHIFT)

typedef struct
{
    U32 GWIN_VEND:      11;
    U32 reserved1:      5;
    U32 reserved:       16;
} GOP_gwin_vend;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DRAM_RBLK0_HSIZE        (0x09*4 + (REG_GOP_BASE))
#define GOP_DRAM_RBLK_HSIZE_SHIFT       (0)
#define GOP_DRAM_RBLK_HSIZE_MASK        (_BIT_MASK(10)<<GOP_DRAM_RBLK_HSIZE_SHIFT)

typedef struct
{
    U32 DRAM_RBLK_HSIZE:    10;
    U32 reserved1:           7;
    U32 reserved:           16;
} GOP_dram_rblk_hsize;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_GWIN0_ALPHA             (0x0A*4 + (REG_GOP_BASE))
#define GOP_GWIN0_ALPHA_SHIFT           (0)
#define GOP_GWIN0_ALPHA_MASK            (_BIT_MASK(2)<<GOP_GWIN0_ALPHA_SHIFT)

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DRAM_GWIN0_VSTR_LO      (0x0C*4 + (REG_GOP_BASE))
#define GOP_DRAM_GWIN_VSTR_LO_SHIFT     (0)
#define GOP_DRAM_GWIN_VSTR_LO_MASK      (_BIT_MASK(16)<<GOP_DRAM_GWIN_VSTR_LO_SHIFT)

typedef struct
{
    U32 DRAM_GWIN_VSTR_LO:  16;
    U32 reserved:           16;
} GOP_dram_gwin_vstr_lo;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DRAM_GWIN0_VSTR_HI      (0x0D*4 + (REG_GOP_BASE))
#define GOP_DRAM_GWIN_VSTR_HI_SHIFT     (0)
#define GOP_DRAM_GWIN_VSTR_HI_MASK      (_BIT_MASK(9)<<GOP_DRAM_GWIN_VSTR_HI_SHIFT)

typedef struct
{
    U32 DRAM_GWIN_VSTR_HI:  9;
    U32 reserved1:          7;
    U32 reserved:           16;
} GOP_dram_gwin_vstr_hi;


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DRAM_GWIN0_HSTR         (0x0E*4 + (REG_GOP_BASE))
#define GOP_DRAM_GWIN_HSTR_SHIFT        (0)
#define GOP_DRAM_GWIN_HSTR_MASK         (_BIT_MASK(10)<<GOP_DRAM_GWIN_HSTR_SHIFT)

typedef struct
{
    U32 DRAM_GWIN_HSTR:     10;
    U32 reserved1:          6;
    U32 reserved:           16;
} GOP_dram_gwin_hstr;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DRAM_RBLK0_SIZE_LO       (0x10*4 + (REG_GOP_BASE))
#define GOP_DRAM_RBLK_SIZE_LO_SHIFT      (0)
#define GOP_DRAM_RBLK_SIZE_LO_MASK       (_BIT_MASK(16)<<GOP_DRAM_RBLK_SIZE_LO_SHIFT)

typedef struct
{
    U32 DRAM_RBLK_SIZE_LO:  16;
    U32 reserved:           16;
} GOP_dram_rblk_size_lo;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_DRAM_RBLK0_SIZE_HI       (0x11*4 + (REG_GOP_BASE))
#define GOP_DRAM_RBLK_SIZE_HI_SHIFT      (0)
#define GOP_DRAM_RBLK_SIZE_HI_MASK       (_BIT_MASK(9)<<GOP_DRAM_RBLK_SIZE_HI_SHIFT)

typedef struct
{
    U32 DRAM_RBLK_SIZE_HI:  9;
    U32 reserved1:          7;
    U32 reserved:           16;
} GOP_dram_rblk_size_hi;


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GWIN_SCROLL_LEN           (0x12*4 + (REG_GOP_BASE))

typedef struct
{
    S32 SCROLL_LEN:         16;
    S32 reserved:           16;
} GOP_gwin_scroll_len;


///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_GWIN0_VSTOP_LO        (0x14*4 + (REG_GOP_BASE))


#define GOP_GWIN_VSTOP_LO_SHIFT        (0)
#define GOP_GWIN_VSTOP_LO_MASK         (_BIT_MASK(16)<<GOP_GWIN_VSTOP_LO_SHIFT)

typedef struct
{
    U32 GWIN_VSTOP_LO:      16; //GWIN VERTICAL AUTO STOP OFFSET LO
    U32 reserved:           16;
} GOP_gwin_vstop_lo;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_GWIN0_HSTOP           (0x14*4 + (REG_GOP_BASE))


#define GOP_GWIN_HSTOP_SHIFT          (0)
#define GOP_GWIN_HSTOP_MASK           (_BIT_MASK(10)<<GOP_GWIN_HSTOP_SHIFT)

typedef struct
{
    U32 GWIN_HSTOP:         10; //GWIN HORIZONTAL AUTO STOP OFFSET
    U32 reserved0:           6;
    U32 reserved:           16;
} GOP_gwin_hstop;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_GOP_GWIN0_VSTOP_HI          (0x15*4 + (REG_GOP_BASE))
#define GOP_GWIN_VSTOP_HI_SHIFT         (0)
#define GOP_GWIN_VSTOP_HI_MASK          (_BIT_MASK(9)<<GOP_GWIN_VSTOP_HI_SHIFT)

typedef struct
{
    U32 GWIN_VSTOP_HI:      9;
    U32 reserved0:          7;
    U32 reserved:           16;
} GOP_gwin_vstop_hi;


/////////////////////////////////FADE/////////////////////////////////////////////////////
#define REG_GOP_GWIN_FADE_CTRL         (0x16*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GWIN_FADE_RATE:      4;
    U32 GWIN_FADE_EN:        1;
    U32 GWIN_FADE_INIT:      1;
    U32 GWIN_FADE_MODE:      1;
    U32 reserved1:           9;
    U32 reserved:            16;
} GOP_gwin_fade_ctrl;







///////////////////////////////////////////////////////////////////////////////////////////////////
#define  GWIN_SET_OFFSET                 (REG_GOP_GWIN1_EN - REG_GOP_GWIN0_EN)

#define  REG_GOP_GWIN_SET(WID)           (REG_GOP_GWIN0_EN + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_GWIN_VSTR(WID)          (REG_GOP_GWIN0_VSTR + GWIN_SET_OFFSET*(WID))
#define  REG_GOP_GWIN_HSTR(WID)          (REG_GOP_GWIN0_HSTR + GWIN_SET_OFFSET*(WID))
#define  REG_GOP_GWIN_VEND(WID)          (REG_GOP_GWIN0_VEND + GWIN_SET_OFFSET*(WID))
#define  REG_GOP_GWIN_HEND(WID)          (REG_GOP_GWIN0_HEND + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_DRAM_GWIN_VSTR_LO(WID)  (REG_GOP_DRAM_GWIN0_VSTR_LO + GWIN_SET_OFFSET*(WID))
#define  REG_GOP_DRAM_GWIN_VSTR_HI(WID)  (REG_GOP_DRAM_GWIN0_VSTR_HI + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_DRAM_GWIN_HSTR(WID)     (REG_GOP_DRAM_GWIN0_HSTR    + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_DRAM_RBLK_STR_LO(WID)   (REG_GOP_DRAM_RBLK0_STR_LO  + GWIN_SET_OFFSET*(WID))
#define  REG_GOP_DRAM_RBLK_STR_HI(WID)   (REG_GOP_DRAM_RBLK0_STR_HI  + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_DRAM_RBLK_SIZE_LO(WID)  (REG_GOP_DRAM_RBLK0_SIZE_LO + GWIN_SET_OFFSET*(WID))
#define  REG_GOP_DRAM_RBLK_SIZE_HI(WID)  (REG_GOP_DRAM_RBLK0_SIZE_HI + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_DRAM_RBLK_HSIZE(WID)    (REG_GOP_DRAM_RBLK0_HSIZE + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_GWIN_HVSTOP_LO(WID)     (REG_GOP_GWIN0_VSTOP_LO + GWIN_SET_OFFSET*(WID))
#define  REG_GOP_GWIN_HVSTOP_HI(WID)     (REG_GOP_GWIN0_VSTOP_HI + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_GWIN_SCROLL_LEN(WID)    (REG_GWIN_SCROLL_LEN + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_GWIN_HSTOP(WID)         (REG_GOP_GWIN0_HSTOP + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_GWIN_VSTOP_LO(WID)      (REG_GOP_GWIN0_VSTOP_LO + GWIN_SET_OFFSET*(WID))
#define  REG_GOP_GWIN_VSTOP_HI(WID)      (REG_GOP_GWIN0_VSTOP_HI + GWIN_SET_OFFSET*(WID))

#define  REG_GOP_GWIN_FADE_SET(WID)      (REG_GOP_GWIN_FADE_CTRL + GWIN_SET_OFFSET*(WID))





///////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
// GOP GWIN Registers
//-------------------------------------------------------------------------------------------------
// DWIN reg

#define GOP_DWIN_CTL0_EN                        (0x00*4 + (REG_GOP_BASE))
#define GOP_DWIN_EN_MASK                        (_BIT_MASK(1)<<0)
#define GOP_DWIN_VS_INV_MASK                    (_BIT_MASK(1)<<1)
#define GOP_DWIN_HS_INV_MASK                    (_BIT_MASK(1)<<2)
#define GOP_DWIN_HALF_SCAL_MASK                 (_BIT_MASK(1)<<3)
#define GOP_DWIN_PROGRESSIVE_MASK               (_BIT_MASK(1)<<4)
#define GOP_DWIN_FLD_INV_MASK                   (_BIT_MASK(1)<<5)
#define GOP_DWIN_DE_INV_MASK                    (_BIT_MASK(1)<<6)
#define GOP_DWIN_CAPTURE_MODE_MASK              (_BIT_MASK(1)<<7)
#define GOP_DWIN_SRC_SEL_MASK                   (_BIT_MASK(2)<<8)
#define GOP_DWIN_UV_SWAP_MASK                   (_BIT_MASK(1)<<10)
#define GOP_DWIN_DMA_LEN_MASK                   (_BIT_MASK(2)<<12)
#define GOP_DWIN_TYGE_MASK                      (_BIT_MASK(2)<<14)

typedef struct
{
    U32 GOP_DWIN_EN:              1;
    U32 GOP_DWIN_VS_INV:          1;
    U32 GOP_DWIN_HS_INV:          1;
    U32 GOP_DWIN_HALF_SCAL:       1;
    U32 GOP_DWIN_PROGRESSIVE:     1;
    U32 GOP_DWIN_FLD_INV:         1;
    U32 GOP_DWIN_DE_INV:          1;
    U32 GOP_DWIN_CAPTURE_MODE:    1;
    U32 GOP_DWIN_SRC_SEL:         2;
    U32 GOP_DWIN_UV_SWAP:         1;
    U32 reserved_1:               1;
    U32 GOP_DWIN_DMA_LEN:         2;
    U32 GOP_DWIN_TYPE:            2;
    U32 reserved:                16;
} GOP_dwin_ctrl_en;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_WBE             (0x01*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_1STR_WBE:        4;
    U32 GOP_DWIN_1END_WBE:        4;
    U32 GOP_DWIN_HI_TSH:          7;
    U32 reserved_1:               1;
    U32 reserved:                16;
} GOP_dwin_wbe;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_INT             (0x02*4 + (REG_GOP_BASE))

typedef struct
{
    U32 reserved_1:               4;
    U32 GOP_DWIN_PROG_INT_MASK:   1;
    U32 GOP_DWIN_TF_INT_MASK:     1;
    U32 GOP_DWIN_BF_INT_MASK:     1;
    U32 GOP_DWIN_VS_INT_MASK:     1;
    U32 reserved_2:               4;
    U32 GOP_DWIN_PROG_INT:        1;
    U32 GOP_DWIN_TF_INT:          1;
    U32 GOP_DWIN_BF_INT:          1;
    U32 GOP_DWIN_VS_INT:          1;
    U32 reserved:                16;
} GOP_dwin_int;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_DBG             (0x03*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_PROG_ACK:        1;
    U32 GOP_DWIN_TF_ACK:          1;
    U32 GOP_DWIN_BF_ACK:          1;
    U32 reserved_1:               1;
    U32 GOP_DWIN_DMA_STATE:       3;
    U32 GOP_DWIN_SRAM_BIST_FAIL:  1;
    U32 GOP_DWIN_BUF_OF:          1;
    U32 GOP_DWIN_BUF_UF:          1;
    U32 reserved_2:               1;
    U32 GOP_DWIN_PINPON:          1;
    U32 GOP_DWIN_WR_FLG:          1;
    U32 reserved_3:               3;
    U32 reserved:                16;
} GOP_dwin_dbg;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_CTRL_DITHER          (0x04*4 + (REG_GOP_BASE))

typedef struct
{
    U32 reserved_1:               5;
    U32 GOP_DWIN_DITHER:          1;
    U32 GOP_R2Y_CSC:              1;
    U32 GOP_DWIN_ALPHA_INV:       1;
    U32 GOP_ALPHA:                8;
    U32 reserved:                16;
} GOP_dwin_ctrl_dither;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_VSTR              (0x10*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_VSTAR:           12;
    U32 reserved_1:               4;
    U32 reserved:                16;
} GOP_dwin_vstar;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_HSTR              (0x11*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_HSTAR:           9;
    U32 reserved_1:               7;
    U32 reserved:                16;
} GOP_dwin_hstar;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_VEND              (0x12*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_VEND:           12;
    U32 reserved_1:               4;
    U32 reserved:                16;
} GOP_dwin_vend;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_HEND              (0x13*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_HEND:            9;
    U32 reserved_1:               7;
    U32 reserved:                16;
} GOP_dwin_hend;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_HSIZE             (0x14*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_HSIZE:           9;
    U32 reserved_1:               7;
    U32 reserved:                16;
} GOP_dwin_hsize;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_JMPLEN            (0x15*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_JMPLEN:          9;
    U32 reserved_1:               7;
    U32 reserved:                16;
} GOP_dwin_jmplen;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_DSTR_L            (0x16*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_DSTR_L:         16;
    U32 reserved:                16;
} GOP_dwin_dstr_l;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_DSTR_H            (0x17*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_DSTR_H:          9;
    U32 reserved_1:               7;
    U32 reserved:                16;
} GOP_dwin_dstr_h;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_HBOND_L           (0x18*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_HBOND_L:        16;
    U32 reserved:                16;
} GOP_dwin_hbond_l;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_HBOND_H           (0x19*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_HBOND_H:         9;
    U32 reserved_1:               7;
    U32 reserved:                16;
} GOP_dwin_hbond_h;


///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_PON_DSTR_L        (0x1A*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_PON_DSTR_L:     16;
    U32 reserved:                16;
} GOP_dwin_pon_dstr_l;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_PON_DSTR_H        (0x1B*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_PON_DSTR_H:      9;
    U32 reserved_1:               7;
    U32 reserved:                16;
} GOP_dwin_pon_dstr_h;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_PON_HBOND_L       (0x1C*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_PON_HBOND_L:    16;
    U32 reserved:                16;
} GOP_dwin_pon_hbond_l;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_DW_PON_HBOND_H       (0x1D*4 + (REG_GOP_BASE))

typedef struct
{
    U32 GOP_DWIN_PON_HBOND_H:     9;
    U32 reserved_1:               7;
    U32 reserved:                16;
} GOP_dwin_pon_hbond_h;


#endif
