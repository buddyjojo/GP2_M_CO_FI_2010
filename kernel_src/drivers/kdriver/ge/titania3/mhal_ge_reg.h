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

#ifndef __REGGE_H__
#define __REGGE_H__

#define GE_FRAMEBUFFER_ADR              0xa0800000
#define GE_BMPBUFFER_ADR                0xa1500000
#define GE_LOG_ADR                      0xa1600000

#define REG_MIPS_BASE                   0xBF200000

#define REG_GE_BASE                     (0xbf205000)
#define GE_Reg(address)                 (*((volatile U32 *)(REG_GE_BASE + ((address)<<2) )))

//----------------------------------------------------------------------------
// Pixel Engine Register Definiation
//----------------------------------------------------------------------------
#define GE_REG_EN                       0x00
#define GE_VAL_EN_PE                    BIT0
#define GE_VAL_EN_DITHER                BIT1
#define GE_VAL_EN_GY_ABL                BIT2
#define GE_VAL_EN_ROP                   BIT5
#define GE_VAL_EN_SCK                   BIT6
#define GE_VAL_EN_DCK                   BIT7
#define GE_VAL_EN_LPT                   BIT8
#define GE_VAL_EN_ALPHA_COMPARE         BIT11
#define GE_VAL_EN_DST_ALPHA_COMPARE     BIT12
#define GE_VAL_EN_GE_TRAP_AA            BIT13
#define GE_VAL_EN_GE_TRAP_SUBC          BIT14
//----------------------------------------------------------------------------
#define GE_REG_FMT_BLT                  0x01
#define GE_VAL_EN_CMDQ                  BIT0
#define GE_VAL_EN_VCMDQ                 BIT1
#define GE_VAL_EN_Read_Prio             BIT2
#define GE_VAL_EN_Write_Prio            BIT3
#define GE_VAL_EN_STRETCH_BITBLT        BIT4
#define GE_VAL_EN_CLIPING_CHECKING      BIT5
#define GE_VAL_EN_ITALIC_FONT           BIT6
#define GE_VAL_EN_LENGTH_LIMIT          BIT7
#define GE_VAL_EN_SRC_TILE				BIT8
#define GE_VAL_EN_DST_TILE				BIT9
#define GE_VAL_EN_DYNAMIC_CLOCK			BIT15
//----------------------------------------------------------------------------
#define GE_REG_DBGMSG                   0x02
#define GE_MSK_DBGMSG                   0x3F
#define GE_LENGTH_TH(val, cnt) ((val&~0x1f00)|(cnt<<8))
//----------------------------------------------------------------------------
#define GE_REG_STBB_TH                  0x03
#define GE_MSK_STBB_TH                  0x0F
#define GE_MSK_VCMDQ_W_TH               0x00F0
#define GE_MSK_VCMDQ_R_TH               0x0F00
//----------------------------------------------------------------------------
#define GE_REG_VCMDQ_STAT				0x04
//----------------------------------------------------------------------------
#define GE_REG_BIST_STAT				0x05
//----------------------------------------------------------------------------
#define GE_REG_STATUS                   0x07
#define GE_VAL_BUSY                     BIT0
#define GE_MSK_R_CMQ_FIFO_STATUS        0xF8
#define GE_MSK_W_CMQ_FIFO_STATUS        0xF800
//----------------------------------------------------------------------------
#define GE_REG_MIU_PROT_LTH_L(_miu)		(0x0008+(_miu<<2))
#define GE_REG_MIU_PROT_LTH_H(_miu)		(0x0009+(_miu<<2))
#define GE_REG_MIU_PROT_HTH_L(_miu)		(0x000A+(_miu<<2))
#define GE_REG_MIU_PROT_HTH_H(_miu)		(0x000B+(_miu<<2))
//----------------------------------------------------------------------------
#define GE_REG_BIST_STATUS              0x0C
#define GE_MSK_BIST_STATUS              0x3F
//----------------------------------------------------------------------------
#define GE_REG_TAG                      0x0F
//----------------------------------------------------------------------------
#define GE_REG_ROP2                     0x10
#define GE_MSK_ROP2                     0x0F

//----------------------------------------------------------------------------
#define GE_REG_ABL_COEF                 0x11
#define GE_MSK_ABL_COEF                 0x000F
//----------------------------------------------------------------------------
#define GE_REG_DB_ABL                   0x12
#define GE_MSK_DB_ABL                   0x0F00
#if 0  // alan needs to update
#define GE_VAL_ABL_CONST                0x0000
#define GE_VAL_ABL_ASRC                 BIT8
#define GE_VAL_ABL_ADST                 BIT9
#endif
//----------------------------------------------------------------------------
#define GE_REG_ABL_CONST                0x13
//----------------------------------------------------------------------------
#define GE_REG_SCK_HTH0                 0x14
#define GE_REG_SCK_HTH1                 0x15
#define GE_REG_SCK_LTH0                 0x16
#define GE_REG_SCK_LTH1                 0x17
#define GE_REG_DCK_HTH0                 0x18
#define GE_REG_DCK_HTH1                 0x19
#define GE_REG_DCK_LTH0                 0x1A
#define GE_REG_DCK_LTH1                 0x1B
//----------------------------------------------------------------------------
#define GE_REG_KEY_OP                   0x1C
#define GE_VAL_SCK_OP_TRUE              BIT0
#define GE_VAL_DCK_OP_TRUE              BIT1
#define GE_VAL_ALPHA_COMP_MODE          BIT4
#define GE_VAL_DST_ALPHA_COMP_MODE      BIT5
//----------------------------------------------------------------------------
#define GE_REG_DSTAC_THRESHOLD          0x1D
#define GE_MASK_DST_ALPAH_HI_THRES      0xFF00
#define GE_MASK_DST_ALPHA_LO_THRES      0x00FF
//----------------------------------------------------------------------------
#define GE_REG_YUV                      0x1F
#define GE_MASK_RGB2YUV                 0x03
#define GE_VAL_RGB2YUV_PC               0x00
#define GE_VAL_RGB2YUV_255              0x01
#define GE_MASK_YUV_RANGE_OUT           0x04
#define GE_VAL_YUV_RANGE_OUT_255        0x00
#define GE_VAL_YUV_RANGE_OUT_PC         0x04
#define GE_MASK_YUV_RANGE_IN            0x08
#define GE_VAL_YUV_RANGE_IN_255         0x00
#define GE_VAL_YUV_RANGE_IN_127         0x08
#define GE_MASK_YUV_MEM_FMT_SRC         0x30
#define GE_SHFT_YUV_MEM_FMT_SRC         0x04
#define GE_VAL_YUV_MEM_FMT_SRC_UY1VY0   0x00
#define GE_VAL_YUV_MEM_FMT_SRC_VY1UY0   0x10
#define GE_VAL_YUV_MEM_FMT_SRC_Y1UY0V   0x20
#define GE_VAL_YUV_MEM_FMT_SRC_Y1VY0U   0x30
#define GE_MASK_YUV_MEM_FMT_DST         0xC0
#define GE_SHFT_YUV_MEM_FMT_DST         0x06
#define GE_VAL_YUV_MEM_FMT_DST_UY1VY0   0x00
#define GE_VAL_YUV_MEM_FMT_DST_VY1UY0   0x40
#define GE_VAL_YUV_MEM_FMT_DST_Y1UY0V   0x80
#define GE_VAL_YUV_MEM_FMT_DST_Y1VY0U   0xC0
//----------------------------------------------------------------------------
#define GE_REG_SB_BASE0                 0x20
#define GE_REG_SB_BASE1                 0x21
//----------------------------------------------------------------------------
#define GE_REG_DB_BASE0                 0x26
#define GE_REG_DB_BASE1                 0x27
//----------------------------------------------------------------------------
#define GE_REG_VCMDQ_BASE0              0x28
#define GE_REG_VCMDQ_BASE1              0x29
#define GE_REG_VCMDQ_SIZE               0x2A
//----------------------------------------------------------------------------
#define PE_REG_PT_TAG                   0x2C
#define PE_REG_PT_GB                    0x2D
#define PE_REG_PT_AR                    0x2E
#define PE_REG_PT_IDX                   0x2F
//----------------------------------------------------------------------------
#define GE_REG_SB_PIT                   0x30
#define GE_REG_DB_PIT                   0x33
//----------------------------------------------------------------------------
#define GE_REG_SB_DB_MODE               0x34
#define GE_MSK_SB_FMT                   0x0f
#define GE_VAL_I1                       0x00
#define GE_VAL_I2                       0x01
#define GE_VAL_I4                       0x02
#define GE_VAL_SB_FMT_I8                0x04
#define GE_VAL_SB_FMT_1ABFGBG12355      0x07
#define GE_VAL_SB_FMT_RGB565            0x08
#define GE_VAL_SB_FMT_ORGB1555          0x09
#define GE_VAL_SB_FMT_ARGB4444          0x0a
// #define GE_VAL_SB_FMT_1BAAFGBG123433    0x0b
#define GE_VAL_SB_FMT_ARGB8888          0x0f


#define GE_MSK_DB_FMT                   0x0f00
#define GE_VAL_DB_FMT_I8                0x0400
#define GE_VAL_DB_FMT_RGB565            0x0800
#define GE_VAL_DB_FMT_ORGB1555          0x0900
#define GE_VAL_DB_FMT_ARGB4444          0x0a00
#define GE_VAL_DB_FMT_1BAAFGBG123433    0x0b00
#define GE_VAL_DB_FMT_ARGB8888          0x0f00


//----------------------------------------------------------------------------
#define GE_REG_I0_C0                    0x35
#define GE_REG_I0_C1                    0x36
#define GE_REG_I1_C0                    0x37
#define GE_REG_I1_C1                    0x38
#define GE_REG_I2_C0                    0x39
#define GE_REG_I2_C1                    0x3a
#define GE_REG_I3_C0                    0x3b
#define GE_REG_I3_C1                    0x3c
#define GE_REG_I4_C0                    0x3d
#define GE_REG_I4_C1                    0x3e
#define GE_REG_I5_C0                    0x3f
#define GE_REG_I5_C1                    0x40
#define GE_REG_I6_C0                    0x41
#define GE_REG_I6_C1                    0x42
#define GE_REG_I7_C0                    0x43
#define GE_REG_I7_C1                    0x44
#define GE_REG_I8_C0                    0x45
#define GE_REG_I8_C1                    0x46
#define GE_REG_I9_C0                    0x47
#define GE_REG_I9_C1                    0x48
#define GE_REG_I10_C0                   0x49
#define GE_REG_I10_C1                   0x4a
#define GE_REG_I11_C0                   0x4b
#define GE_REG_I11_C1                   0x4c
#define GE_REG_I12_C0                   0x4d
#define GE_REG_I12_C1                   0x4e
#define GE_REG_I13_C0                   0x4f
#define GE_REG_I13_C1                   0x50
#define GE_REG_I14_C0                   0x51
#define GE_REG_I14_C1                   0x52
#define GE_REG_I15_C0                   0x53
#define GE_REG_I15_C1                   0x54
#define GE_INTENSITY_NUM                16
//----------------------------------------------------------------------------
#define GE_REG_CLIP_LEFT                0x55
#define GE_REG_CLIP_RIGHT               0x56
#define GE_REG_CLIP_TOP                 0x57
#define GE_REG_CLIP_BOTTOM              0x58
//----------------------------------------------------------------------------
#define GE_REG_ROT_MIRROR               0x59
#define GE_MSK_ROT                      0x03
#define GE_VAL_ROT_DEGREE_0             0x00
#define GE_VAL_ROT_DEGREE_90            0x01
#define GE_VAL_ROT_DEGREE_180           0x02
#define GE_VAL_ROT_DEGREE_270           0x03
#define GE_MSK_TRAP_DELTA_X0            0x0300
#define GE_MSK_TRAP_DELTA_X0_POS        8
#define GE_MSK_TRAP_DELTA_X1            0x3000
#define GE_MSK_TRAP_DELTA_X1_POS        12
//----------------------------------------------------------------------------
#define GE_REG_TRAP_DELTA_X0            0x5a
#define GE_REG_TRAP_DELTA_X1            0x5b
//----------------------------------------------------------------------------
#define GE_REG_STBB_INIT_DX             0x5e
#define GE_VAL_STBB_DX_MSB              BIT15
#define GE_REG_STBB_INIT_DY             0x5f
#define GE_VAL_STBB_DY_MSB              BIT15

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define GE_REG_CMD                      0x60

#define GE_MSK_PRIM_TYPE                0xF0
#define GE_VAL_PRIM_LINE                0x10
#define GE_VAL_PRIM_TRAPEZOID           0x20
#define GE_VAL_PRIM_RECTANGLE           0x30
#define GE_VAL_PRIM_BITBLT              0x40
#define GE_VAL_PRIM_TRAPEZOID_BITBLT    0x50
#define GE_MSK_CLEAR_FLAG               0xFF80
#define GE_VAL_DRAW_SRC_DIR_X_NEG       BIT7
#define GE_VAL_DRAW_SRC_DIR_Y_NEG       BIT8
#define GE_VAL_DRAW_DST_DIR_X_NEG       BIT9
#define GE_VAL_DRAW_DST_DIR_Y_NEG       BIT10
#define GE_VAL_LINE_GRADIENT            BIT11
#define GE_VAL_RECT_GRADIENT_H          BIT12
#define GE_VAL_RECT_GRADIENT_V          BIT13
#define GE_VAL_STBB_NEAREST             BIT14
#define GE_VAL_STBB_PATCH               BIT15
//----------------------------------------------------------------------------
#define GE_REG_LINE_DTA                 0x61
#define GE_MSK_LINE_DTA                 0x7FFE
#define GE_VAL_LINE_Y_MAJOR             BIT15
//----------------------------------------------------------------------------
#define GE_REG_LPT                      0x62
#define GE_MSK_LP                       0x3F
#define GE_MSK_LPT_FACTOR               0xC0
#define GE_VAL_LPT_F1                   0x00
#define GE_VAL_LPT_F2                   0x40
#define GE_VAL_LPT_F3                   0x80
#define GE_VAL_LPT_F4                   0xC0
#define GE_VAL_LPT_RESET                BIT8
#define GE_VAL_LINE_LAST                BIT9
//----------------------------------------------------------------------------
#define GE_REG_LENGTH                   0x63
//----------------------------------------------------------------------------
#define GE_REG_STBB_DX                  0x64 // 2.12 d[2]-d[15]
#define GE_REG_STBB_DY                  0x65
//----------------------------------------------------------------------------
#define GE_REG_ITC_DIS_LINE             0x66
#define GE_REG_ITC_DELTA                0x67
//----------------------------------------------------------------------------
#define GE_REG_PRI_V0_X                 0x68
#define GE_REG_PRI_V0_Y                 0x69
#define GE_REG_PRI_V1_X                 0x6A
#define GE_REG_PRI_V1_Y                 0x6B
#define GE_REG_PRI_V2_X                 0x6C
#define GE_REG_PRI_V2_Y                 0x6D
//----------------------------------------------------------------------------
#define GE_REG_STBB_WIDTH               0x6E
#define GE_REG_STBB_HEIGHT              0x6F
//----------------------------------------------------------------------------
#define GE_REG_PRI_BG_ST                0x70
#define GE_REG_PRI_RA_ST                0x71
//----------------------------------------------------------------------------
#define GE_REG_PRI_R_DX0                0x72
#define GE_REG_PRI_R_DX1                0x73
#define GE_REG_PRI_R_DY0                0x74
#define GE_REG_PRI_R_DY1                0x75
#define GE_REG_PRI_G_DX0                0x76
#define GE_REG_PRI_G_DX1                0x77
#define GE_REG_PRI_G_DY0                0x78
#define GE_REG_PRI_G_DY1                0x79
#define GE_REG_PRI_B_DX0                0x7a
#define GE_REG_PRI_B_DX1                0x7b
#define GE_REG_PRI_B_DY0                0x7c
#define GE_REG_PRI_B_DY1                0x7d
#define GE_REG_PRI_A_DX                 0x7e
#define GE_REG_PRI_A_DY                 0x7f

#endif //__REGGE_H__

