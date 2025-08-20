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
/// @file   mdrv_mace.h
/// @brief  Advance Color Engine Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MDRV_MACE_H__
#define __MDRV_MACE_H__

//------------------------------------------------------------------------------
//  Function
//------------------------------------------------------------------------------
void MDrv_SC_MACE_Init(PSC_DRIVER_CONTEXT_t pDrvCtx);
void MDrv_SC_MACE_SetCorrectionMatrix(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetYUV2RGBMatrix(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetContrast(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetSaturation(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetHue(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetRGBEx(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_GetHistogramInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetLumaCurve(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetLumaCurveEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetHistogramReqEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetICCSaturationAdj(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetIBCYAdj(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetIHCHueDiffColorYAdj(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//[090623_Leo]
void MDrv_SC_MACE_SetIHCHueAdj(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
void MDrv_SC_MACE_SetICCSaturationEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//thchen 20080718
void MDrv_SC_MACE_SetIBCYEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//thchen 20080718
void MDrv_SC_MACE_SetIHCHueEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//thchen 20080718
void MDrv_SC_MACE_SetICCRegionTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080814
void MDrv_SC_MACE_SetIHCRegionTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080814
void MDrv_SC_MACE_SetIHCYModeDiffColorEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//[090623_Leo]
void MDrv_SC_MACE_SetIHCYModelEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080821
void MDrv_SC_MACE_SetICCYModelEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080821
void MDrv_SC_SelectCSC(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);//victor 20080821
void MDrv_SC_MACE_HistogramData_Init(PSC_DRIVER_CONTEXT_t pDrvCtx);
void MDrv_SC_MACE_DLCInit(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);

#endif // __MDRV_MACE_H__

