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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    reg_hdmi.h
/// @brief  HDMI Module Register Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __REG_HDMI_H__
#define __REG_HDMI_H__


#define FREQ_12MHZ              12000000UL
#define FREQ_14P318MHZ          14318180UL
#define MST_XTAL_CLOCK_HZ       FREQ_12MHZ

//-------------------------------------------------------------------------------------------------
//  Macro and Define (DVI ATOP/DTOP)
//-------------------------------------------------------------------------------------------------

#define REG_DVIATOP_BASE        0x110900
#define REG_DVIDTOP_BASE        0x110A00
#define REG_PM_SLP_BASE         0x000E00

#define REG_DVI_ATOP(_x_)       (REG_DVIATOP_BASE | (_x_ << 1))
#define REG_DVI_DTOP(_x_)       (REG_DVIDTOP_BASE | (_x_ << 1))
#define REG_PM_SLP(_x_)         (REG_PM_SLP_BASE | (_x_ << 1))

//HDCP
#define REG_HDCP_BASE           0x110AC0                        //0x26F0
#define REG_HDCP(_x_)           (REG_HDCP_BASE | (_x_ << 1))

#define REG_HDCP_00             REG_HDCP(0x00)
#define REG_HDCP_STATUS         REG_HDCP(0x01)

//-------------------------------------------------------------------------------------------------
//  Macro and Define (Audio)
//-------------------------------------------------------------------------------------------------

#define REG_AUDIO_BASE          0x112C00
#define REG_AUDIO(_x_)          (REG_AUDIO_BASE | (_x_ << 1))

//-------------------------------------------------------------------------------------------------
//  Macro and Define (HDMI)
//-------------------------------------------------------------------------------------------------

#define REG_HDMI_BASE           0x102700
//
// register definition
//
//HDMI
#define REG_HDMI(_x_)           (REG_HDMI_BASE | (_x_ << 1))

#define REG_HDMI_SYSCONFIG      REG_HDMI(0x00)
#define REG_HDMI_ST1            REG_HDMI(0x01)
#define REG_HDMI_ST2            REG_HDMI(0x02)
#define REG_HDMI_ERR1           REG_HDMI(0x04)
#define REG_HDMI_CONFIG         REG_HDMI(0x05)
#define REG_HDMI_CONFIG1        REG_HDMI(0x06)
#define REG_HDMI_CONFIG2        REG_HDMI(0x07)
#define REG_HDMI_CONFIG3        REG_HDMI(0x08)
#define REG_HDMI_CONFIG4        REG_HDMI(0x09)
#define REG_HDMI_CLK_CFG        REG_HDMI(0x0A)
#define REG_HDMI_TMCTRL         REG_HDMI(0x0B)
#define REG_HDMI_FREQ_CMPVAL1   REG_HDMI(0x0C)
#define REG_HDMI_FREQ_CMPVAL2   REG_HDMI(0x0D)
#define REG_HDMI_FREQ_CMPVAL3   REG_HDMI(0x0E)
#define REG_HDMI_FREQ_CMPVAL4   REG_HDMI(0x0F)
#define REG_HDMI_PKT_TYPE       REG_HDMI(0x10)
#define REG_HDMI_PCLK_FREQ      REG_HDMI(0x11)
#define REG_HDMI_AUDIO_CLK0     REG_HDMI(0x12)
#define REG_HDMI_AUDIO_CLK1     REG_HDMI(0x13)
#define REG_HDMI_AUDIO_CLK2     REG_HDMI(0x14)
#define REG_HDMI_GCONTROL       REG_HDMI(0x15)
#define REG_HDMI_ACP_HB1        REG_HDMI(0x16)
#define REG_HDMI_ACP_DATA0      REG_HDMI(0x17)
#define REG_HDMI_ACP_DATA1      REG_HDMI(0x18)
#define REG_HDMI_ACP_DATA2      REG_HDMI(0x19)
#define REG_HDMI_ACP_DATA3      REG_HDMI(0x1A)
#define REG_HDMI_ACP_DATA4      REG_HDMI(0x1B)
#define REG_HDMI_ACP_DATA5      REG_HDMI(0x1C)
#define REG_HDMI_ACP_DATA6      REG_HDMI(0x1D)
#define REG_HDMI_ACP_DATA7      REG_HDMI(0x1E)

#define REG_HDMI_GM_HB          REG_HDMI(0x1F)  // write this register to update GM pkt
#define REG_HDMI_GM_DATA0       REG_HDMI(0x20)
#define REG_HDMI_GM_DATA1       REG_HDMI(0x21)
#define REG_HDMI_GM_DATA2       REG_HDMI(0x22)
#define REG_HDMI_GM_DATA3       REG_HDMI(0x23)
#define REG_HDMI_GM_DATA4       REG_HDMI(0x24)
#define REG_HDMI_GM_DATA5       REG_HDMI(0x25)
#define REG_HDMI_GM_DATA6       REG_HDMI(0x26)
#define REG_HDMI_GM_DATA7       REG_HDMI(0x27)
#define REG_HDMI_GM_DATA8       REG_HDMI(0x28)
#define REG_HDMI_GM_DATA9       REG_HDMI(0x29)
#define REG_HDMI_GM_DATA10      REG_HDMI(0x2A)

#define REG_HDMI_ISRC_HB1       REG_HDMI(0x1F)
#define REG_HDMI_ISRC_DATA0     REG_HDMI(0x20)
#define REG_HDMI_ISRC_DATA1     REG_HDMI(0x21)
#define REG_HDMI_ISRC_DATA2     REG_HDMI(0x22)
#define REG_HDMI_ISRC_DATA3     REG_HDMI(0x23)
#define REG_HDMI_ISRC_DATA4     REG_HDMI(0x24)
#define REG_HDMI_ISRC_DATA5     REG_HDMI(0x25)
#define REG_HDMI_ISRC_DATA6     REG_HDMI(0x26)
#define REG_HDMI_ISRC_DATA7     REG_HDMI(0x27)
#define REG_HDMI_ISRC_DATA8     REG_HDMI(0x28)
#define REG_HDMI_ISRC_DATA9     REG_HDMI(0x29)
#define REG_HDMI_ISRC_DATA10    REG_HDMI(0x2A)
#define REG_HDMI_ISRC_DATA11    REG_HDMI(0x2B)
#define REG_HDMI_ISRC_DATA12    REG_HDMI(0x2C)
#define REG_HDMI_ISRC_DATA13    REG_HDMI(0x2D)
#define REG_HDMI_ISRC_DATA14    REG_HDMI(0x2E)
#define REG_HDMI_ISRC_DATA15    REG_HDMI(0x2F)
#define REG_HDMI_VS_HDR0        REG_HDMI(0x30)
#define REG_HDMI_VS_HDR1        REG_HDMI(0x31)
#define REG_HDMI_VS_IF0         REG_HDMI(0x32)
#define REG_HDMI_VS_IF1         REG_HDMI(0x33)
#define REG_HDMI_VS_IF2         REG_HDMI(0x34)
#define REG_HDMI_VS_IF3         REG_HDMI(0x35)
#define REG_HDMI_VS_IF4         REG_HDMI(0x36)
#define REG_HDMI_VS_IF5         REG_HDMI(0x37)
#define REG_HDMI_VS_IF6         REG_HDMI(0x38)
#define REG_HDMI_VS_IF7         REG_HDMI(0x39)
#define REG_HDMI_VS_IF8         REG_HDMI(0x3A)
#define REG_HDMI_VS_IF9         REG_HDMI(0x3B)
#define REG_HDMI_VS_IF10        REG_HDMI(0x3C)
#define REG_HDMI_VS_IF11        REG_HDMI(0x3D)
#define REG_HDMI_VS_IF12        REG_HDMI(0x3E)
#define REG_HDMI_VS_IF13        REG_HDMI(0x3F)
#define REG_HDMI_AVI_IF0        REG_HDMI(0x40)
#define REG_HDMI_AVI_IF1        REG_HDMI(0x41)
#define REG_HDMI_AVI_IF2        REG_HDMI(0x42)
#define REG_HDMI_AVI_IF3        REG_HDMI(0x43)
#define REG_HDMI_AVI_IF4        REG_HDMI(0x44)
#define REG_HDMI_AVI_IF5        REG_HDMI(0x45)
#define REG_HDMI_AVI_IF6        REG_HDMI(0x46)
#define REG_HDMI_SPD_IF0        REG_HDMI(0x47)
#define REG_HDMI_SPD_IF1        REG_HDMI(0x48)
#define REG_HDMI_SPD_IF2        REG_HDMI(0x49)
#define REG_HDMI_SPD_IF3        REG_HDMI(0x4A)
#define REG_HDMI_SPD_IF4        REG_HDMI(0x4B)
#define REG_HDMI_SPD_IF5        REG_HDMI(0x4C)
#define REG_HDMI_SPD_IF6        REG_HDMI(0x4D)
#define REG_HDMI_SPD_IF7        REG_HDMI(0x4E)
#define REG_HDMI_SPD_IF8        REG_HDMI(0x4F)
#define REG_HDMI_SPD_IF9        REG_HDMI(0x50)
#define REG_HDMI_SPD_IF10       REG_HDMI(0x51)
#define REG_HDMI_SPD_IF11       REG_HDMI(0x52)
#define REG_HDMI_SPD_IF12       REG_HDMI(0x53)
#define REG_HDMI_AUDIO_IF0      REG_HDMI(0x54)
#define REG_HDMI_AUDIO_IF1      REG_HDMI(0x55)
#define REG_HDMI_AUDIO_IF2      REG_HDMI(0x56)
#define REG_HDMI_MPEG_IF0       REG_HDMI(0x57)
#define REG_HDMI_MPEG_IF1       REG_HDMI(0x58)
#define REG_HDMI_MPEG_IF2       REG_HDMI(0x59)
#define REG_HDMI_CS0            REG_HDMI(0x5A)
#define REG_HDMI_CS1            REG_HDMI(0x5B)
#define REG_HDMI_CS2            REG_HDMI(0x5C)
//#define REG_HDMI_PLL_CTRL1      REG_HDMI(0x5D) // Reserved
//#define REG_HDMI_PLL_CTRL2      REG_HDMI(0x5E) // Reserved
#define REG_HDMI_PLL_CTRL3      REG_HDMI(0x5F)
#define REG_HDMI_INT_MASK       REG_HDMI(0x60)
#define REG_HDMI_INT_STATUS     REG_HDMI(0x61)
#define REG_HDMI_INT_FORCE      REG_HDMI(0x62)
#define REG_HDMI_INT_CLEAR      REG_HDMI(0x63)
#define REG_HDMI_RESET_PACKET   REG_HDMI(0x64)
#define REG_HDMI_AUTO_MODE      REG_HDMI(0x65)
#define REG_HDMI_FRAME_RP_VAL   REG_HDMI(0x66)
#define REG_HDMI_CEC_CONFIG1    REG_HDMI(0x67)
#define REG_HDMI_CEC_CONFIG2    REG_HDMI(0x68)
#define REG_HDMI_CEC_CONFIG3    REG_HDMI(0x69)
#define REG_HDMI_CEC_CONFIG4    REG_HDMI(0x6A)
#define REG_HDMI_CEC_STATUS1    REG_HDMI(0x6C)
#define REG_HDMI_CEC_TX_DATA0   REG_HDMI(0x70)
#define REG_HDMI_CEC_TX_DATA1   REG_HDMI(0x71)
#define REG_HDMI_CEC_RX_DATA0   REG_HDMI(0x78)
#define REG_HDMI_CEC_RX_DATA1   REG_HDMI(0x79)

#endif // __REG_HDMI_H__
