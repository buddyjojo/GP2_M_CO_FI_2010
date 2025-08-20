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

#ifndef _HAL_MAD_H_
#define _HAL_MAD_H_

#include <linux/delay.h>
#include "mdrv_types.h"

#ifdef _AUCOMMON_C_
#define _AUCOMMON_DECLAIM_
#else
#define _AUCOMMON_DECLAIM_ extern
#endif


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define ALIGN_4(_x_)                (((_x_) + 3) & ~3)
#define ALIGN_8(_x_)                (((_x_) + 7) & ~7)
#define ALIGN_16(_x_)               (((_x_) + 15) & ~15)           // No data type specified, optimized by complier
#define ALIGN_32(_x_)               (((_x_) + 31) & ~31)           // No data type specified, optimized by complier

//#define BIT(_bit_)                  (1 << (_bit_))
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))


#define READ_BYTE(_reg)             (*(volatile MS_U8*)(_reg))
#define READ_WORD(_reg)             (*(volatile MS_U16*)(_reg))
#define READ_LONG(_reg)             (*(volatile MS_U32*)(_reg))
#define WRITE_BYTE(_reg, _val)      { (*((volatile MS_U8*)(_reg))) = (MS_U8)(_val); }
#define WRITE_WORD(_reg, _val)      { (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }
#define WRITE_LONG(_reg, _val)      { (*((volatile MS_U32*)(_reg))) = (MS_U32)(_val); }

#define R1BYTE(u32Addr, u8mask)            \
    (READ_BYTE (MAD_REG_BASE + ((u32Addr) << 1) - ((u32Addr) & 1)) & (u8mask))

#define AbsR1BYTE(u32Addr, u8mask)            \
    (READ_BYTE (REG_BASE + ((u32Addr) << 1) - ((u32Addr) & 1)) & (u8mask))

#define W1BYTE(u32Addr, u8Val, u8mask)     \
    (WRITE_BYTE(MAD_REG_BASE + ((u32Addr) << 1) - ((u32Addr) & 1), (R1BYTE(u32Addr, 0xFF) & ~(u8mask)) | ((u8Val) & (u8mask))))

// u32Addr must be 16bit aligned
#define R2BYTE(u32Addr, u16mask)            \
    (READ_WORD (MAD_REG_BASE + ((u32Addr) << 1)) & (u16mask))

// u32Addr must be 16bit aligned
#define W2BYTE(u32Addr, u16Val, u16mask)    \
    (WRITE_WORD(MAD_REG_BASE + ((u32Addr) << 1), (R2BYTE(u32Addr, 15:0) & ~(u16mask)) | ((u16Val) & (u16mask))))


#define MAD_REG_BASE    0xBF220000 //T3

#define REG_BASE 0xBF000000

#define MAD_REG(addr)   (*((volatile u16 *)(MAD_REG_BASE + (addr) * 2))) //T3

#define NON_MAD_REG(addr)   (*((volatile u16 *)(REG_BASE + (addr) * 2))) //T3

#define T3_CHIP 1

//Audio Register mnemonics

#define PAL_AUTO_DETECT                            0xE0
#define PAL_DETECT_SYS_BUSY                        0x80
#define PAL_NICAM_LOCKED                           0x05

//================================Porting from Utopia Start==========================================
#define AUDIO_REG_BASE                          0x2C00  // 0x2C00 - 0x2DFF
#define MIU0_REG_BASE                           0x1200
#define MIU1_REG_BASE                           0x0600

//---------------------------------------------------------------------------
// AUDIO Common Register
//---------------------------------------------------------------------------
#define REG_AUDIO_SOFT_RESET                    0x2C00
#define REG_AUDIO_INPUT_CFG                        0x2C02
#define REG_AUDIO_STATUS_DVB_FREQ                  0x2C04
#define REG_AUDIO_STATUS_I2S_FREQ                  0x2C06
#define REG_AUDIO_STATUS_SIF_FREQ                  0x2C08
#define REG_AUDIO_SPDIF_IN_CFG                     0x2C0A
#define REG_AUDIO_STATUS_INPUT                     0x2C0C
#define REG_AUDIO_STATUS_SPDIF_IN_FREQ             0x2C0E
#define REG_AUDIO_STATUS_SPDIF_IN_CS0              0x2C10
#define REG_AUDIO_STATUS_SPDIF_IN_CS1              0x2C12
#define REG_AUDIO_STATUS_SPDIF_IN_CS2              0x2C14
#define REG_AUDIO_STATUS_SPDIF_IN_CS3              0x2C16
#define REG_AUDIO_STATUS_SPDIF_IN_CS4              0x2C18
#define REG_AUDIO_STATUS_SPDIF_IN_PC               0x2C1A
#define REG_AUDIO_STATUS_SPDIF_IN_PD               0x2C1C

#define REG_AUDIO_STATUS_SYNTH                      0x2c2c
#define REG_AUDIO_STATUS_HDMI_PC                   0x2C40
#define REG_AUDIO_STATUS_HDMI_PD                   0x2C42
#define REG_AUDIO_HDMI_MATRIX0                     0x2C44
#define REG_AUDIO_HDMI_MATRIX1                     0x2C46
#define REG_AUDIO_DOWN_SAMPLE                      0x2C48

#define REG_AUDIO_BT_CTRL1                         0x2C50
#define REG_AUDIO_BT_CTRL2                         0x2C52
#define REG_AUDIO_BT_CTRL3                         0x2C54
#define REG_AUDIO_BT_CTRL4                         0x2C56

#define REG_AUDIO_DECODER1_CFG                     0x2C60
#define REG_AUDIO_DECODER2_CFG                     0x2C62
#define REG_AUDIO_DECODER3_CFG                     0x2C61
#define REG_AUDIO_DECODER4_CFG                   0x2C63
#define REG_AUDIO_CH1_CFG                          0x2C64
#define REG_AUDIO_CH2_CFG                          0x2C66
#define REG_AUDIO_CH3_CFG                          0x2C68
#define REG_AUDIO_CH4_CFG                          0x2C6A
#define REG_AUDIO_CH5_CFG                          0x2C65
#define REG_AUDIO_CH6_CFG                          0x2C67
#define REG_AUDIO_CH7_CFG                          0x2C69
#define REG_AUDIO_CH8_CFG                          0x2C6B

#define REG_AUDIO_INPUT_REGEN_CFG                  0x2C6C

#define REG_AUDIO_DOUT_FIX_VAL1                    0x2C70
#define REG_AUDIO_DOUT_FIX_VAL2                    0x2C72
#define REG_AUDIO_DOUT_FIX_VAL3                    0x2C74

#define REG_AUDIO_SPDIF_OUT_CS0                    0x2C80
#define REG_AUDIO_SPDIF_OUT_CS1                    0x2C82
#define REG_AUDIO_SPDIF_OUT_CS2                    0x2C84
#define REG_AUDIO_SPDIF_OUT_CS3                    0x2C86
#define REG_AUDIO_SPDIF_OUT_CS4                    0x2C88
#define REG_AUDIO_SPDIF_OUT_CFG                    0x2C8A
#define REG_AUDIO_I2S_OUT1_CFG                     0x2C8C
#define REG_SPDIF_NPCM_SYNTH_NF_H               0x2BB8
#define REG_SPDIF_NPCM_SYNTH_NF_L               0x2BBA

#define REG_AUDIO_PAD_CFG                          0x2C90
#define REG_AUDIO_MUTE_CFG                         0x2C92
#define REG_AUDIO_MUTE_CTRL1                       0x2C94
#define REG_AUDIO_MUTE_CTRL2                       0x2C96
#define REG_AUDIO_MUTE_CTRL3                       0x2C98
#define REG_AUDIO_MUTE_CTRL4                       0x2C9A

#define REG_AUDIO_CODEC_SYNTH                      0x2CA0
#define REG_AUDIO_PLL_REF_CFG                      0x2CA2
#define REG_AUDIO_CLK_CFG0                         0x2CA4
#define REG_AUDIO_CLK_CFG1                         0x2CA6
#define REG_AUDIO_CLK_CFG2                         0x2CA8
#define REG_AUDIO_CLK_CFG3                         0x2CAA
#define REG_AUDIO_CLK_CFG4                         0x2CAC
#define REG_AUDIO_CLK_CFG5                         0x2CAE
#define REG_AUDIO_CLK_CFG6                         0x2CB0
#define REG_AUDIO_SYNTH_EXPANDER                   0x2CB2
#define REG_AUDIO_SYNTH_768_CFG0                   0x2CB4
#define REG_AUDIO_SYNTH_768_CFG1                   0x2CB6
#define REG_AUDIO_SYNTH_768_FREQ                   0x2CB8
#define REG_AUDIO_OUT_256FS_SEL                    0x2CBA

#define REG_AUDIO_CODEC_CFG0                       0x2CE0
#define REG_AUDIO_CODEC_CFG1                       0x2CE2
#define REG_AUDIO_CODEC_CFG2                       0x2CE4
#define REG_AUDIO_CODEC_CFG3                       0x2CE6
#define REG_AUDIO_CODEC_CFG4                       0x2CE8
#define REG_AUDIO_CODEC_CFG5                       0x2CEA
#define REG_AUDIO_CODEC_CFG6                       0x2CEC
#define REG_AUDIO_CODEC_CFG7                       0x2CEE
#define REG_AUDIO_DC_OFFSET                        0x2CFA

#define REG_AUDIO_IRQ_CONTROL1                     0x2D62
#define REG_AUDIO_IRQ_CONTROL2                     0x2D64
#define REG_AUDIO_DEMODULATOR_CTRL                 0x2D66
#define REG_AUDIO_FIFO_STATUS                      0x2D68

//---------------------------------------------------------------------------
// AUDIO Sound Effect Register
//---------------------------------------------------------------------------
#define REG_SOUND_MAIN_PERSCALE                    0x2D10

#define REG_SOUND_AUOUT0_VOLUME                  0x2D01
#define REG_SOUND_AUOUT1_VOLUME                  0x2D03
#define REG_SOUND_AUOUT2_VOLUME                  0x2D05
#define REG_SOUND_AUOUT3_VOLUME                  0x2D07
#define REG_SOUND_I2S_VOLUME                          0x2D09
#define REG_SOUND_SPDIF_VOLUME                      0x2D0B
#define REG_SOUND_AUOUT0_VOL_FRAC                     0x2D00
#define REG_SOUND_AUOUT1_VOL_FRAC                     0x2D02
#define REG_SOUND_AUOUT2_VOL_FRAC                     0x2D04
#define REG_SOUND_AUOUT3_VOL_FRAC                     0x2D06
#define REG_SOUND_I2S_VOL_FRAC                     0x2D08
#define REG_SOUND_SPDIF_VOL_FRAC                     0x2D0A

#define REG_SOUND_AD_VOLUME                        0x2DD8


#define REG_SOUND_EQ_BASE                          0x2D15
#define REG_SOUND_EQ1                              0x2D15
#define REG_SOUND_EQ2                              0x2D17
#define REG_SOUND_EQ3                              0x2D19
#define REG_SOUND_EQ4                              0x2D1B
#define REG_SOUND_EQ5                              0x2D1D

#define REG_SOUND_BASS                             0x2D14
#define REG_SOUND_TREBLE                           0x2D16
#define REG_SOUND_BALANCEL                         0x2D1F
#define REG_SOUND_BALANCER                         0x2D1E

#define REG_SOUND_AVC_AT                           0x2D25
#define REG_SOUND_AVC_RT                           0x2D25
#define REG_SOUND_AVC_MODE                         0x2D25
#define REG_SOUND_AVC_THRESHOLD                    0x2D24
#define REG_SOUND_NR_THRESHOLD                      0x2D32

#define REG_SOUND_MAIN_SNDEFFECT                   0x2D20
#define REG_SOUND_SPK_MOD                               0x2D86
#define REG_SOUND_MAIN_COUNTER                     0x2DF9
#define REG_SOUND_TIMER_COUNTER                    0x2DF8
#define REG_SOUND_ISR_COUNTER                      0x2DF6

//---------------------------------------------------------------------------
// AUDIO SIF Register
//---------------------------------------------------------------------------
#define REG_AUDIO_ASIF_CONFIG0                     0x2CC0
#define REG_AUDIO_ASIF_CONFIG1                     0x2CC2
#define REG_AUDIO_ASIF_CONFIG2                     0x2CC4
#define REG_AUDIO_ASIF_CONFIG3                     0x2CC6
#define REG_AUDIO_ASIF_CONFIG4                     0x2CC8
#define REG_AUDIO_ASIF_ICTRL                       0x2CCA
#define REG_AUDIO_ASIF_AMUX                        0x2CCC
#define REG_AUDIO_ASIF_TST                         0x2CCE
#define REG_AUDIO_ASIF_ADCREF                      0x2CD0
#define REG_AUDIO_SIFPLL_CTRL                      0x2CD2
#define REG_AUDIO_SIFPLL_MN                        0x2CD4
#define REG_AUDIO_SIFPLL_TEST                      0x2CD6
#define REG_AUDIO_SIFPLL_EXT                       0x2CD8
#define REG_AUDIO_SIFPLL_STATUS                    0x2CDA
#define REG_AUDIO_ASIF_TST_EXT                     0x2CDC
#define REG_AUDIO_VIF_CONFIG0                      0x2CDE
//--------------------------------
// AUDIO SIF Register Value
//--------------------------------
#define VAL0_REG_AUDIO_ASIF_CONFIG0                 0x0200
#define VAL0_REG_AUDIO_ASIF_CONFIG1                 0x0070
#define VAL0_REG_AUDIO_ASIF_CONFIG2                 0x1200
#define VAL0_REG_AUDIO_ASIF_CONFIG3                 0x1000
#define VAL0_REG_AUDIO_ASIF_CONFIG4                 0x0090
#define VAL0_REG_AUDIO_ASIF_ICTRL                   0x1555
#define VAL0_REG_AUDIO_ASIF_AMUX                    0x00EA
#define VAL0_REG_AUDIO_ASIF_TST                     0x0004
#define VAL0_REG_AUDIO_ASIF_ADCREF                  0x6C00
#define VAL0_REG_AUDIO_SIFPLL_CTRL                  0x1009
#define VAL0_REG_AUDIO_SIFPLL_MN                    0x1109
#define VAL0_REG_AUDIO_SIFPLL_TEST                  0x0000
#define VAL0_REG_AUDIO_SIFPLL_EXT                   0x0001
#define VAL0_REG_AUDIO_SIFPLL_STATUS                0x2CDA
#define VAL0_REG_AUDIO_ASIF_TST_EXT                 0x0000
#define VAL0_REG_AUDIO_VIF_CONFIG0                  0x0000
#define MASK_REG_AUDIO_VIF_CONFIG0                  0x0040
#define VAL1_REG_AUDIO_VIF_CONFIG0                  0x0040
#define VAL2_REG_AUDIO_VIF_CONFIG0                  0x0000

//---------------------------------------------------------------------------
// AUDIO SIF Register
//---------------------------------------------------------------------------
#define REG_SOUND_ADV_CFG0                          0x2D40
#define REG_SOUND_ADV_CFG1                          0x2D42
#define REG_SOUND_ADV_CFG2                          0x2D44
#define REG_SOUND_ADV_CFG3                          0x2D36

//---------------------------------------------------------------------------
// AUDIO MAD Register
//---------------------------------------------------------------------------

#define REG_DEC_M2D_MAIL_BOX_BASE                   0x2D80
#define REG_DEC_D2M_MAIL_BOX_BASE                   0x2DA0
#define REG_SE_M2D_MAIL_BOX_BASE                   0x2DC0
#define REG_SE_D2M_MAIL_BOX_BASE                   0x2DE0

#define REG_MAD_MAIN_COUNTER                        0x2DB9
#define REG_MAD_TIMER_COUNTER                       0x2DB8

#define REG_CHIP_ID_MAJOR                           0x1ECC
#define REG_CHIP_ID_MINOR                           0x1ECD
#define REG_CHIP_VERSION                            0x1ECE
#define REG_CHIP_REVISION                           0x1ECF

//kuocc add 20090608

//0x 2C
#define REG_AUDIO_INPUT_CFG                         0x2C02
#define REG_AUDIO_CODEC_SYNTH                       0x2CA0
#define REG_CODEC_SYNTH_H                           0x2CA1
#define REG_CLK_CFG0                                0x2CA4

//0x 2D(DEC) //@@Morris
#define REG_DEC_IDMA_CTRL0                                       0x2A00 //0x2D00

#define REG_DEC_DSP_BRG_DATA_L                              0x2A02 //0x2D02
#define REG_DEC_DSP_BRG_DATA_H                              0x2A03 //0x2D03
#define REG_DEC_IDMA_WRBASE_ADDR_L                     0x2A04 //0x2D04
#define REG_DEC_IDMA_WRBASE_ADDR_H                    0x2A05 //0x2D05

#define REG_DEC_IDMA_RDBASE_ADDR_L                     0x2A08 //0x2D08
#define REG_DEC_IDMA_RDBASE_ADDR_H                     0x2A09 //0x2D09

#define REG_DEC_IDMA_RDDATA_H_0                           0x2A0C //0x2D0C
#define REG_DEC_IDMA_RDDATA_H_1                           0x2A0D //0x2D0D
#define REG_DEC_IDMA_RDDATA_L                               0x2A0E //0x2D0E

#define REG_DEC_DSP_ICACHE_BASE_L                        0x2A10 //0x2D10
#define REG_DEC_DSP_ICACHE_BASE_H                        0x2A11 //0x2D11

#define REG_DEC_AUD_DTRL                                        0x2A20  //0x2DE0
#define REG_DEC_MAD_OFFSET_BASE_H                     0x2A24  //0x2DE4
#define REG_DEC_MBASE_H                                          0x2A26  //0x2DE6
#define REG_DEC_MSIZE_H                                          0x2A28  //0x2DE8
#define REG_DEC_DECODE_CMD                                   0x2A2C // 0x2DEC
#define REG_DEC_MCFG                                                0x2A2A  //0x2DEA




//0x 2D(SE)  //@@Morris
#define REG_SE_IDMA_CTRL0                                      0x2A80 //0x2D00

#define REG_SE_DSP_BRG_DATA_L                             0x2A82 //0x2D02
#define REG_SE_DSP_BRG_DATA_H                             0x2A83 //0x2D03
#define REG_SE_IDMA_WRBASE_ADDR_L                    0x2A84 //0x2D04
#define REG_SE_IDMA_WRBASE_ADDR_H                    0x2A85 //0x2D05

#define REG_SE_IDMA_RDBASE_ADDR_L                    0x2A88 //0x2D08
#define REG_SE_IDMA_RDBASE_ADDR_H                   0x2A89 //0x2D09

#define REG_SE_IDMA_RDDATA_H_0                         0x2A8C //0x2D0C
#define REG_SE_IDMA_RDDATA_H_1                         0x2A8D //0x2D0D
#define REG_SE_IDMA_RDDATA_L                              0x2A8E //0x2D0E

#define REG_SE_DSP_ICACHE_BASE_L                      0x2A90 //0x2D10
#define REG_SE_DSP_ICACHE_BASE_H                      0x2A91 //0x2D11

#define REG_SE_AUD_DTRL                                        0x2AA0  //0x2DE0
#define REG_SE_MAD_OFFSET_BASE_H                     0x2AA4  //0x2DE4
#define REG_SE_MBASE_H                                          0x2AA6  //0x2DE6
#define REG_SE_MSIZE_H                                          0x2AA8  //0x2DE8
#define REG_SE_DECODE_CMD                                   0x2AAC // 0x2DEC
#define REG_SE_MCFG                                                0x2AAA  //0x2DEA

#define REG_SE_DECODE_CMD                                   0x2AAC // 0x2DEC


#define REG_M2D_MAILBOX_0_L 0x2D20
#define REG_M2D_MAILBOX_0_H 0x2D21

#define REG_M2D_MAILBOX_1_L 0x2D22
#define REG_M2D_MAILBOX_1_H 0x2D23

#define REG_M2D_MAILBOX_5_L 0x2D2A
#define REG_M2D_MAILBOX_5_H 0x2D2B

#define REG_M2D_MAILBOX_7_L 0x2D2E
#define REG_M2D_MAILBOX_7_H 0x2D2F

#define REG_DK123_AUTO_CTRL 0x2D2A
#define REG_FC_TRACKING_RESET 0x2D2A

#define REG_DBG_DATA_H 0x2D2C


#define REG_DBG_DATA_L 0x2D2E

#define REG_DEBUG_REG_3_H 0x2D37
#define REG_DEBUG_REG_4_L 0x2D38
#define REG_DEBUG_REG_4_H 0x2D39
#define REG_DEBUG_REG_5_L 0x2D3A
#define REG_DEBUG_REG_5_H 0x2D3B
#define REG_DBG_CMD 0x2D3D

/*
#define REG_D2M_MAILBOX_0_L 0x2D40
#define REG_D2M_MAILBOX_0_H 0x2D41

#define REG_D2M_MAILBOX_1_L 0x2D42
#define REG_D2M_MAILBOX_1_H 0x2D43

#define REG_D2M_MAILBOX_2_L 0x2D44
#define REG_D2M_MAILBOX_2_H 0x2D45

#define REG_D2M_MAILBOX_5_L 0x2D4A
#define REG_D2M_MAILBOX_5_H 0x2D4B
*/
#define REG_M2D_MAILBOX_PIO_ID                  0x2D8B
#define REG_M2D_MAILBOX_SPDIF_CTRL              0x2D8E

#define REG_D2M_MAILBOX_DEC_ISRCMD             0x2DB2
#define REG_M2D_MAILBOX_DEC_DBGCMD            0x2D9D
#define REG_M2D_MAILBOX_DEC_DBGPARAM1            0x2D9C
#define REG_M2D_MAILBOX_DEC_DBGPARAM2            0x2D9F
#define REG_M2D_MAILBOX_DEC_DBGPARAM3            0x2D9E

// define for MPG Encoder (APVR audio driver)
#define REG_DEC_ENCODE_CMD                              0x2A2D
#define REG_D2M_MAILBOX_ENC_LineAddr             0x2DA0
#define REG_D2M_MAILBOX_ENC_LineSize              0x2DA2
#define REG_MPG_EN_CTRL                                    0x2D88
#define REG_MPG_EN_TAG_FROM_MIPS                   0x2D8D

#define REG_D2M_MAILBOX_SE_ISRCMD               0x2DF6
#define REG_D2M_MAILBOX_SE_POWERCTRL        0x2D31

#define REG_D2M_MAILBOX_3_L 0x2D46
#define REG_D2M_MAILBOX_3_H 0x2D47

#define REG_D2M_MAILBOX_4_L 0x2D48
#define REG_D2M_MAILBOX_4_H 0x2D49

#define REG_D2M_MAILBOX_6_L 0x2D4C
#define REG_D2M_MAILBOX_6_H 0x2D4D

#define REG_D2M_MAILBOX_7_L 0x2D4E
#define REG_D2M_MAILBOX_7_H 0x2D4F

#define REG_D2M_MAILBOX_8_L 0x2D50
#define REG_D2M_MAILBOX_8_H 0x2D51

#define REG_D2M_MAILBOX_9_L 0x2D52
#define REG_D2M_MAILBOX_9_H 0x2D53

#define REG_D2M_MAILBOX_A_L 0x2D54
#define REG_D2M_MAILBOX_A_H 0x2D55

#define REG_D2M_MAILBOX_B_L 0x2D56
#define REG_D2M_MAILBOX_B_H 0x2D57

#define REG_D2M_MAILBOX_C_L 0x2D58
#define REG_D2M_MAILBOX_C_H 0x2D59

#define REG_D2M_MAILBOX_D_L 0x2D5A
#define REG_D2M_MAILBOX_D_H 0x2D5B

#define REG_DEC1_INT_ID                     0x2DB2
#define REG_DEC1_TIME_STAMP_SEC    0x2D66
#define REG_DEC1_TIME_STAMP_MS      0x2D68
#define REG_DEC1_TS_PTS_H                 0x2D6A
#define REG_DEC1_TS_PTS_M                0x2D6C
#define REG_DEC1_TS_PTS_L                 0x2D6E

#define REG_DEC2_TIME_STAMP_SEC    0x2D66
#define REG_DEC2_TIME_STAMP_MS      0x2D68
#define REG_DEC2_TS_PTS_H                 0x2D6A
#define REG_DEC2_TS_PTS_M                0x2D6C
#define REG_DEC2_TS_PTS_L                 0x2D6E

#define REG_DBG_DATA_HI 0x2D5C
#define REG_DBG_DATA_LO 0x2D5E

#define REG_AUDIO_IRQ_CONTROL1_2 0x2D63


#define REG_AUD_MADBASE_SEL 0x2DE0
#define REG_AUD_DIS_DMA 0x2DE0
#define REG_AUD_RST_MAD 0x2DE0

#define REG_MCUDSP_CNT_CFG 0x2DE2
#define REG_MAD_BUF_BASE 0x2DE3

#define REG_MAD_OFFSET_BASE_L 0x2DE4
#define REG_MAD_OFFSET_BASE_H 0x2DE5

#define REG_MBASE_L 0x2DE6
#define REG_MBASE_H 0x2DE7
#define REG_MSIZE_L 0x2DE8
#define REG_MSIZE_H 0x2DE9

#define REG_MEM_CFG 0x2DEA

// Mail box for T3
#define REG_MB_MODE_SELECT                    0x2D30
#define REG_MB_POWER_DOWN                    0x2D30
#define REG_MB_TIME_STAMP_SEC 0x2D66
#define REG_MB_TIME_STAMP_4ms 0x2D68
#define REG_MB_TIME_STAMP_DSP2_SEC 0x2D76
#define REG_MB_TIME_STAMP_DSP2_4ms 0x2D78
#define REG_MB_DEC_CTRL 0x2D86
#define REG_MB_DEC_PIO_ID 0x2D8A
#define REG_MB_DEC_CMD1 0x2D9C
#define REG_MB_DEC_CMD2 0x2D9E
#define REG_MB_MPEG_ADDR_L 0x2DAA
#define REG_MB_MPEG_ADDR_H 0x2DAB
#define REG_MB_DDP_AD_STATUS 0x2DAC
#define REG_MB_DEC_TIMER_CNT    0x2DB8
#define REG_MB_DEC_WHILE_CNT    0x2DB9
#define REG_MB_DEC_ID_STATUS                 0x2DBA
#define REG_MB_ENC_ID_STATUS                 0x2DBA
#define REG_MB_DE_ACK1 0x2DBD
#define REG_MB_DE_ACK2 0x2DBE
#define REG_MB_SE_PIO_ID 0x2DCC
#define REG_MB_DEC3_CTRL 0x2DD8
#define REG_MB_SE_CMD1 0x2DDC
#define REG_MB_SE_CMD2 0x2DDE
#define REG_MB_SE_ISR_CNT    0x2DF6
#define REG_MB_SE_TIMER_CNT    0x2DF8
#define REG_MB_SE_WHILE_CNT    0x2DF9
#define REG_MB_SE_ACK1 0x2DFC
#define REG_MB_SE_ACK2 0x2DFE
#define REG_MB_AC3P_SMPRATE     0x2DAA
#define REG_MB_HDMI_NONPCM_LOCKINFO 0x2DB3

#define REG_MB_DEC_RESULT_D1                           0x2DBC          //0x2D5C  //0x2DBC
#define REG_MB_DEC_RESULT_D2                           0x2DBE          //0x2D5E  //0x2DBE
#define REG_SE_ISR_CMD                                0x2D4E          //Interrupt command for DSP2

#define REG_SOUND_CMD_BT 0x2D4C //BT SBC
#define REG_ADV_SNDEFF      0x2D41
#define REG_SE_INT_ID         0x2DF7

#define REG_MB_DEC_CUT_BOOST 0x2D98
//===============================Porting from Utopia End===============================
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    U32    addr;
    U8    mask;
    U8    value;
} MST_REG_TYPE, *PMST_REG_TYPE;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
_AUCOMMON_DECLAIM_ void MHal_MAD_WriteRegTbl(MST_REG_TYPE *);//JUST TEMP modify
_AUCOMMON_DECLAIM_ U16 MHal_MAD_ReadReg(U32 u32Reg);
_AUCOMMON_DECLAIM_ U8 MHal_MAD_ReadByte(U32 u32Reg);
_AUCOMMON_DECLAIM_ U16 MHal_MAD_SeReadReg(U32 u32Reg);
_AUCOMMON_DECLAIM_ U8 MHal_MAD_SeReadRegByte(U32 u32Reg);
_AUCOMMON_DECLAIM_ void MHal_MAD_WriteReg(U32 u32Reg, U16 u16Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_WriteByte(U32 u32Reg, U8 u8Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_DecWriteReg(U32 u32Reg, U16 u16Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_DecWriteRegByte(U32 u32Reg, U8 u8Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_SeWriteReg(U16 u32Reg, U16 u16Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_SeWriteRegByte(U32 u32Reg, U8 u8Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_WriteMaskReg(U16 u16Addr, U16 u16Mask, U16 u16Value);
_AUCOMMON_DECLAIM_ void MHal_MAD_DecWriteRegMask(U32 u32RegAddr, U16 u16Mask, U16 u16Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_SeWriteRegMask(U32 u32RegAddr, U16 u16Mask, U16 u16Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_DecWriteRegMaskByte(U32 u32Reg, U8 u8Mask, U8 u8Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_DecWriteIntMaskByte(U32 u32Reg, U8 u8Mask);
_AUCOMMON_DECLAIM_ void MHal_MAD_SeWriteRegMaskByte(U32 u32Reg, U8 u8Mask, U8 u8Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_AbsWriteMaskReg(U32 u32Reg, U16 u8Mask, U16 u8Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_WriteMaskByte(U32 u32Reg, U8 u8Mask, U8 u8Val);
_AUCOMMON_DECLAIM_ void MHal_MAD_AbsWriteMaskByte(MS_U32 u32Reg, MS_U8 u8Mask, MS_U8 u8Val);
#endif // _HAL_SCALER_H_

