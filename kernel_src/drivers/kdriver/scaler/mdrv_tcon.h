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
/// @file   mdrv_tcon.h
/// @brief  MStar Scaler Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DRV_TCON_H__
#define __DRV_TCON_H__

#include "mdrv_tcon_tbl.h"
#include "mdrv_scaler.h"


void MDrv_SC_Set_TCONMap(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_Get_TCONTab_Info(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_Set_TCONPower_Sequence(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_Set_TCON_Count_Reset(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_TCONMAP_DumpPowerOnSequenceReg(void);

#endif//__DRV_ADC_H__
