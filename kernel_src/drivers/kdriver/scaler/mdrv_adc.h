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
/// @file   drv_adc.h
/// @brief  MStar Scaler Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

#include "mdrv_scaler_st.h"
#include "mdrv_scaler.h"
#include "mhal_adc.h"
#include "mdrv_adctbl.h"

#define ENABLE_ADC_TEST   0
//------------------------------------------------------------------------------
//  Data structure
//------------------------------------------------------------------------------
/// ADC setting
typedef struct
{
    U8 u8RedGain;       ///< ADC red gain
    U8 u8GreenGain;     ///< ADC green gain
    U8 u8BlueGain;      ///< ADC blue gain
    U8 u8RedOffset;     ///< ADC red offset
    U8 u8GreenOffset;   ///< ADC green offset
    U8 u8BlueOffset;    ///< ADC blue offset
    U8 u8AdcCalOK;      ///< ADC Cal OK
    U16 u16ADCDataCS;   ///< ADC Data CS
} SC_ADC_SETTING_t;

//------------------------------------------------------------------------------
//  Function
//------------------------------------------------------------------------------
void MDrv_ADC_Init(void);
void MDrv_ADC_SetSource(ADC_SOURCE_TYPE inputsrc_type); // daniel.huang 20090615
void MDrv_ADC_SetMux(ADC_MUX_TYPE ipmux_type);
void MDrv_ADC_SetMode(BOOL bRGB, U16 u16PixelClk); // unit in MHz
void MDrv_ADC_SetCVBSO(U8 u8PortNum, ADC_CVBSO_TYPE cvbso_type);
void MDrv_ADC_SetCVBSO_MUX(U8 u8PortNum, ADC_CVBSO_MUX_TYPE cvbsomux_type);
void MDrv_ADC_ADCCal(BOOL bIsAVInput);  //20091012 daniel.huang: update adctbl to 0.26 to fix componenet position shift and color more white; and new mismatch cal settings
void MDrv_ADC_SetAnalogADC(PSC_SOURCE_INFO_t psrc);
void MDrv_ADC_SetOffset(U8 u8Color, U16 u16Value);  // 20090903 daniel.huang: add/fix set/get gain/offset function
void MDrv_ADC_GetOffset(U8 u8Color, U16 *u16Value); // 20090903 daniel.huang: add/fix set/get gain/offset function
void MDrv_ADC_SetGain(U8 u8Color, U16 u16Value);    // 20090903 daniel.huang: add/fix set/get gain/offset function
void MDrv_ADC_GetGain(U8 u8Color, U16 *u16Value);   // 20090903 daniel.huang: add/fix set/get gain/offset function

void MDrv_ADC_Test(void); // for debug & generate script only
void MDrv_ADC_Calibration_Testing(void);//20090814 Michu, ADC Calibration Testing

#endif//__DRV_ADC_H__
