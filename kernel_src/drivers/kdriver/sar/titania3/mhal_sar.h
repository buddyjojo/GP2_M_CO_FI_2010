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

#ifndef _HAL_SAR_H_
#define _HAL_SAR_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    U8 u8UpBnd; //upper bound
    U8 u8LoBnd; //low bound
} SAR_BND2_t;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MHal_SAR_Init(void);
U8 MHal_SAR_GetChannelMaxId(void);
B16 MHal_SAR_Config_ChannelBound(U8 u8Channel, SAR_BND2_t *psarBndCfg);
void MHal_SAR_Config_SingleChannel(U8 u8Channel);
void MHal_SAR_Config_TriggerMode(B16 bMode);
void MHal_SAR_Config_SingleChannelEn(B16 bEnable);
void MHal_SAR_Config_ShotMode(U8 u8Mode);
void MHal_SAR_Config_Powerdown(B16 bEnable);
void MHal_SAR_Config_Start(B16 bEnable);
void MHal_SAR_Config_ADCPowerdown(B16 bEnable);
void MHal_SAR_Config_FreeRun(B16 bEnable);
void MHal_SAR_Config_Selection(B16 bEnable);
void MHal_SAR_Config_8Channel(B16 bEnable);
void MHal_SAR_Config_ClockSamplePeriod(U8 u8ClkSmpPrd);
void MHal_SAR_Config_IntMask(B16 bEnable);
void MHal_SAR_Config_IntClear(B16 bEnable);
void MHal_SAR_Config_IntForce(B16 bEnable);
B16 MHal_SAR_GetIntStatus(void);
U8 MHal_SAR_GetChannelADC(U8 u8Channel);

#endif // _HAL_SAR_H_

