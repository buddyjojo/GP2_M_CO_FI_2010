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

#ifndef __HAL_MOD_H__
#define __HAL_MOD_H__
  
#include "mst_platform.h"
//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    MOD_OEZ_NONE            = 0,
    MOD_OEZ_TTL_VS          = BIT0,
    MOD_OEZ_TTL_HS          = BIT1,
    MOD_OEZ_TTL_DE          = BIT2,
    MOD_OEZ_TTL_CK          = BIT3,
    MOD_OEZ_LVDS_LB         = BIT4,
    MOD_OEZ_LVDS_LA         = BIT5,
    MOD_OEZ_ALL             = (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5),
} HAL_MOD_OEZ_e;
// FitchHsu 20080811 implement LPLL type
#if 0
typedef enum
{
    MOD_LPLL_LVDS,
    MOD_LPLL_RSDS,
    MOD_LPLL_TTL,
} HAL_MOD_LVDS_TYPE_e;
#endif

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MHal_MOD_SetPower(BOOL bOn, U8 u8LPLL_Type); // FitchHsu 20080811 implement LPLL type
void MHal_MOD_SelTTL(U16 u16Sel);
// FitchHsu 20080811 implement LPLL type
#if 0
void MHal_MOD_Ctrl0(U8 u8Ctrl);
void MHal_MOD_CtrlA(U8 u8Ctrl);
#endif

void MHal_MOD_Init(PMST_PANEL_INFO_t pPanelInfo);
void MHal_MOD_Set_LPLL(PMST_PANEL_INFO_t pPanelInfo); // FitchHsu 20080811 implement LPLL type


#endif // __HAL_MOD_H__

