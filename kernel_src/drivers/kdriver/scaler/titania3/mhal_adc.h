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

#ifndef _HAL_ADC_H_
#define _HAL_ADC_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define    INTERNAL_0_55V        1
#define    INTERNAL_1_05V        2
#define    INTERNAL_NONE         0xff

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

typedef enum
{
    ADC_COLOR_RED,
    ADC_COLOR_GREEN,
    ADC_COLOR_BLUE,
} HAL_ADC_COLOR_t;

typedef enum
{
    ADC_AUTO_EXTERNAL_ADC,
    ADC_AUTO_INTERNAL_ADC,
} HAL_ADC_AUTO_TYPE_t;

typedef struct
{
    U8 *pTable;
    U8 u8TabCols;
    U8 u8TabRows;
    U8 u8TabIdx;
} TAB_Info;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MHal_ADC_LoadTable(TAB_Info* pTab_info);
void MHal_ADC_SetInputHsyncPolarity(BOOL bHightActive);
void MHal_ADC_SetOffset(U8 u8Color, U16 u16Value);  // 20090903 daniel.huang: add/fix set/get gain/offset function
void MHal_ADC_GetOffset(U8 u8Color, U16 *u16Value); // 20090903 daniel.huang: add/fix set/get gain/offset function
void MHal_ADC_SetGain(U8 u8Color, U16 u16Value);    // 20090903 daniel.huang: add/fix set/get gain/offset function
void MHal_ADC_GetGain(U8 u8Color, U16 *u16Value);   // 20090903 daniel.huang: add/fix set/get gain/offset function
void MHal_ADC_SetADCClk(U16 u16Value);
void MHal_ADC_SetADCPhase(U8 u8Value);
void MHal_ADC_InternalDc( U8 InternalVoltage );  //victor 20080827
void MHal_ADC_Calibration_Testing(void);//20090814 Michu, ADC Calibration Testing
void MHal_ADC_SetSOGThreshold(U8 u8Threshold);   //20090924 daniel.huang
void MHal_ADC_FreeRun_Enable(BOOL bEnable);//20100217 comp problem.

#endif // _HAL_ADC_H_
