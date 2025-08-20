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
/// @file   mdrv_aesdma_io.h
/// @brief  aesdma Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access aesdma.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_PWS_IO_H_
#define _MDRV_PWS_IO_H_
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define PWS_IOC_MAGIC                'w'

#define MDRV_PWS_INIT                         _IO(PWS_IOC_MAGIC, 0)
#define MDRV_PWS_SET_IO_MAP_BASE              _IO(PWS_IOC_MAGIC, 1)
#define MDRV_PWS_GET_IO_MAP_BASE              _IOWR(PWS_IOC_MAGIC, 2, U32)
#define MDRV_PWS_GET_CHIP_ID                  _IOWR(PWS_IOC_MAGIC, 3, U32)

#define MDRV_PWS_SET_DVI_DM_DEMUX             _IOWR(PWS_IOC_MAGIC, 4, U8)
#define MDRV_PWS_SET_DVI_PREAMP               _IOWR(PWS_IOC_MAGIC, 5, U8)
#define MDRV_PWS_SET_DVI_BIST                 _IOWR(PWS_IOC_MAGIC, 6, U8)

#define MDRV_PWS_SET_DVI_DM_RXCK_BIST         _IOWR(PWS_IOC_MAGIC, 7, U8)

#define MDRV_PWS_SET_DVI_DM_PRECLK            _IOWR(PWS_IOC_MAGIC, 8, U8)
#define MDRV_PWS_SET_DVI_DM_PRECLK_OFFL       _IOWR(PWS_IOC_MAGIC, 9, U8)
#define MDRV_PWS_SET_DVI_DM_RBIAS             _IOWR(PWS_IOC_MAGIC, 10, U8)
#define MDRV_PWS_SET_DVI_DM_ENVDET            _IOWR(PWS_IOC_MAGIC, 11, U8)


#define MDRV_PWS_SET_DVI_PLL_CORE             _IOWR(PWS_IOC_MAGIC, 12, U8)
#define MDRV_PWS_SET_DVI_PLL_REGM             _IOWR(PWS_IOC_MAGIC, 13, U8)
#define MDRV_PWS_SET_DVI_PLL_PHDAC            _IOWR(PWS_IOC_MAGIC, 14, U8)
#define MDRV_PWS_SET_INGAL_PAD_ARRAY          _IOWR(PWS_IOC_MAGIC, 15, U8)
#define MDRV_PWS_SET_VIF_BANDGAP_IBIAS_VREF   _IOWR(PWS_IOC_MAGIC, 16, U8)

#define MDRV_PWS_SET_Vif_CALIBRATION_BUFFER   _IOWR(PWS_IOC_MAGIC, 17, U8)
#define MDRV_PWS_SET_VIF_CLAMP_BUFFER         _IOWR(PWS_IOC_MAGIC, 18, U8)
#define MDRV_PWS_SET_VIF_ADC_I                _IOWR(PWS_IOC_MAGIC, 19, U8)
#define MDRV_PWS_SET_VIF_ADCQ                 _IOWR(PWS_IOC_MAGIC, 20, U8)
#define MDRV_PWS_SET_VIF_PGA1                 _IOWR(PWS_IOC_MAGIC, 21, U8)
#define MDRV_PWS_SET_VIF_PGA2                 _IOWR(PWS_IOC_MAGIC, 22, U8)
#define MDRV_PWS_SET_VIF_MPLL_REG             _IOWR(PWS_IOC_MAGIC, 23, U8)
#define MDRV_PWS_SET_VIF_ADC_OUT_CLK_PD       _IOWR(PWS_IOC_MAGIC, 24, U8)
#define MDRV_PWS_SET_VIF_MPLL_DIV2_PD         _IOWR(PWS_IOC_MAGIC, 25, U8)

#define MDRV_PWS_SET_VIF_MPLL_DIV3_PD         _IOWR(PWS_IOC_MAGIC, 26, U8)
#define MDRV_PWS_SET_VIF_MPLL_DIV4_PD         _IOWR(PWS_IOC_MAGIC, 27, U8)
#define MDRV_PWS_SET_VIF_MPLL_DIV8_PD         _IOWR(PWS_IOC_MAGIC, 28, U8)
#define MDRV_PWS_SET_VIF_MPLL_DIV10_PD        _IOWR(PWS_IOC_MAGIC, 29, U8)

#define MDRV_PWS_SET_VIF_TAGC                 _IOWR(PWS_IOC_MAGIC, 30, U8)
#define MDRV_PWS_SET_ADC_R                    _IOWR(PWS_IOC_MAGIC, 31, U8)
#define MDRV_PWS_SET_ADC_G                    _IOWR(PWS_IOC_MAGIC, 32, U8)
#define MDRV_PWS_SET_ADC_B                    _IOWR(PWS_IOC_MAGIC, 33, U8)
#define MDRV_PWS_SET_PHDIG                    _IOWR(PWS_IOC_MAGIC, 34, U8)
#define MDRV_PWS_SET_ADC_PLLA                 _IOWR(PWS_IOC_MAGIC, 35, U8)
#define MDRV_PWS_SET_ADC_ICLAMP_RGB           _IOWR(PWS_IOC_MAGIC, 36, U8)
#define MDRV_PWS_SET_ADC_ICLAMP_VDY           _IOWR(PWS_IOC_MAGIC, 37, U8)
#define MDRV_PWS_SET_ADC_ICLAMP_VDC           _IOWR(PWS_IOC_MAGIC, 38, U8)
#define MDRV_PWS_SET_ADC_Y                    _IOWR(PWS_IOC_MAGIC, 39, U8)
#define MDRV_PWS_SET_PLL_B                    _IOWR(PWS_IOC_MAGIC, 40, U8)
#define MDRV_PWS_SET_ADC_SOG                  _IOWR(PWS_IOC_MAGIC, 41, U8)
#define MDRV_PWS_SET_ADC_SOG_OFF              _IOWR(PWS_IOC_MAGIC, 42, U8)
#define MDRV_PWS_SET_ADC_HSYNC0               _IOWR(PWS_IOC_MAGIC, 43, U8)
#define MDRV_PWS_SET_ADC_HSYNC1               _IOWR(PWS_IOC_MAGIC, 44, U8)
#define MDRV_PWS_SET_ADC_HSYNC2               _IOWR(PWS_IOC_MAGIC, 45, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_200           _IOWR(PWS_IOC_MAGIC, 46, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_400           _IOWR(PWS_IOC_MAGIC, 47, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_PLL           _IOWR(PWS_IOC_MAGIC, 48, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_R             _IOWR(PWS_IOC_MAGIC, 49, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_G             _IOWR(PWS_IOC_MAGIC, 50, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_B             _IOWR(PWS_IOC_MAGIC, 51, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_Y             _IOWR(PWS_IOC_MAGIC, 52, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_VD            _IOWR(PWS_IOC_MAGIC, 53, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_200_FB        _IOWR(PWS_IOC_MAGIC, 54, U8)
#define MDRV_PWS_SET_ADC_SOG_MUX              _IOWR(PWS_IOC_MAGIC, 55, U8)
#define MDRV_PWS_SET_ADC_FB_ADC               _IOWR(PWS_IOC_MAGIC, 56, U8)
#define MDRV_PWS_SET_DVDD                     _IO(PWS_IOC_MAGIC, 57)

/*
#define MDRV_PWS_SET_USB_HS_TX                _IOWR(PWS_IOC_MAGIC, 26, U8)
#define MDRV_PWS_SET_USB_HS_RX                _IOWR(PWS_IOC_MAGIC, 27, U8)
#define MDRV_PWS_SET_USB_FlXCVR               _IOWR(PWS_IOC_MAGIC, 28, U8)
#define MDRV_PWS_SET_USB_SERDES               _IOWR(PWS_IOC_MAGIC, 29, U8)
#define MDRV_PWS_SET_USB_PLL_LDO              _IOWR(PWS_IOC_MAGIC, 30, U8)


#define MDRV_PWS_SET_USB_LDO                  _IOWR(PWS_IOC_MAGIC, 31, U8)
#define MDRV_PWS_SET_USB_REF_BIAS_CIR         _IOWR(PWS_IOC_MAGIC, 32, U8)
#define MDRV_PWS_SET_ADCR                     _IOWR(PWS_IOC_MAGIC, 33, U8)
#define MDRV_PWS_SET_ADCG                     _IOWR(PWS_IOC_MAGIC, 34, U8)
#define MDRV_PWS_SET_ADCB                     _IOWR(PWS_IOC_MAGIC, 35, U8)
#define MDRV_PWS_SET_ADC_PHDIG                _IOWR(PWS_IOC_MAGIC, 36, U8)
#define MDRV_PWS_SET_ADC_PLLA                 _IOWR(PWS_IOC_MAGIC, 37, U8)
#define MDRV_PWS_SET_ADC_ICLAMP_RGB           _IOWR(PWS_IOC_MAGIC, 38, U8)
#define MDRV_PWS_SET_ADC_ICLAMP_VDY           _IOWR(PWS_IOC_MAGIC, 39, U8)


#define MDRV_PWS_SET_ADC_ICLAMP_VDC           _IOWR(PWS_IOC_MAGIC, 40, U8)
#define MDRV_PWS_SET_ADCY                     _IOWR(PWS_IOC_MAGIC, 41, U8)
#define MDRV_PWS_SET_ADC_PLLB                 _IOWR(PWS_IOC_MAGIC, 42, U8)
#define MDRV_PWS_SET_ADC_SOG                  _IOWR(PWS_IOC_MAGIC, 43, U8)
#define MDRV_PWS_SET_ADC_SOG_OFF              _IOWR(PWS_IOC_MAGIC, 44, U8)
#define MDRV_PWS_SET_ADC_SOG_UNUSED           _IOWR(PWS_IOC_MAGIC, 45, U8)
#define MDRV_PWS_SET_ADC_HSYNC0               _IOWR(PWS_IOC_MAGIC, 46, U8)
#define MDRV_PWS_SET_ADC_HSYNC1               _IOWR(PWS_IOC_MAGIC, 47, U8)
#define MDRV_PWS_SET_ADC_HSYNC2               _IOWR(PWS_IOC_MAGIC, 48, U8)

#define MDRV_PWS_SET_ADC_PD_CLK200            _IOWR(PWS_IOC_MAGIC, 49, U8)
#define MDRV_PWS_SET_ADC_PD_CLK400            _IOWR(PWS_IOC_MAGIC, 50, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_PLL           _IOWR(PWS_IOC_MAGIC, 51, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_R             _IOWR(PWS_IOC_MAGIC, 52, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_G             _IOWR(PWS_IOC_MAGIC, 53, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_B             _IOWR(PWS_IOC_MAGIC, 54, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_Y             _IOWR(PWS_IOC_MAGIC, 55, U8)
#define MDRV_PWS_SET_ADC_PD_CLK_VD            _IOWR(PWS_IOC_MAGIC, 56, U8)
#define MDRV_PWS_SET_ADC_PD_CLK200_FB         _IOWR(PWS_IOC_MAGIC, 57, U8)

#define MDRV_PWS_SET_ADC_SOG_MUX              _IOWR(PWS_IOC_MAGIC, 58, U8)
#define MDRV_PWS_SET_ADC_FB_ADC               _IOWR(PWS_IOC_MAGIC, 59, U8)
#define MDRV_PWS_SET_ADC_CVBS_LPF_Y           _IOWR(PWS_IOC_MAGIC, 60, U8)
#define MDRV_PWS_SET_ADC_CVBS_LPF_C           _IOWR(PWS_IOC_MAGIC, 61, U8)
*/
#define PWS_IOC_MAXNR                99
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#endif // _MDRV_PWS_IO_H_
