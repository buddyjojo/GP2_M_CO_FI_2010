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

//////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvRLD.h
/// @brief  RLD Driver
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MHAL_RLD_H_
#define _MHAL_RLD_H_


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Extern Global Variabls
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Extern Functions
//-------------------------------------------------------------------------------------------------
extern void MHal_RLD_Init(void);

extern void MHal_RLD_Reset(void);
extern void MHal_RLD_Start(void);

extern BOOL MHal_RLD_SetOutputFormat(U8 u8Value);
extern BOOL MHal_RLD_SetEnlargeRate(U8 u8Value);
extern void MHal_RLD_SetTransparentKey(U8 u8Value);

extern void MHal_RLD_SetTopFieldLength(U16 u16Value);
extern void MHal_RLD_SetTopFieldAddress(U32 u32Value);

extern void MHal_RLD_SetBottomFieldLength(U16 u16Value);
extern void MHal_RLD_SetBottomFieldAddress(U32 u32Value);

extern void MHal_RLD_SetOutputAddress(U32 u32Value);

extern void MHal_RLD_SetRegionWidth   (U16 u16Value);
extern void MHal_RLD_SetRegionHeight  (U16 u16Value);
extern void MHal_RLD_SetRegionPitch   (U16 u16Value);
extern void MHal_RLD_SetObjXOffset    (U16 u16Value);
extern void MHal_RLD_SetObjYOffset(U16 u16Value);

extern void MHal_RLD_SetRegionOffset  (U8 u8Value);
extern void MHal_RLD_SetRegionDepth(U8 u8Value);
extern void MHal_RLD_SetRegionColorKeyFG(U8 u8Value);
extern void MHal_RLD_SetRegionColorKeyBG(U8 u8Value);

// Patch
extern void MHal_RLD_SetOutOfWidthPatch(void);
extern void MHal_RLD_SetEndLastDataPatch(void);

extern void MHal_RLD_SetColorReductionPatch(void);
extern void MHal_RLD_SetColorMappingPatch(void);
extern void MHal_RLD_ClearColorMappingPatch(void);
extern void MHal_RLD_SetNonModifyingPatch(void);

extern U8 MHal_RLD_GetReturn(void);
extern U8 MHal_RLD_WaitReturn(void);

extern void MHal_RLD_SetRegionColorKeyFG(U8 u8Value);
extern void MHal_RLD_SetRegionColorKeyBG(U8 u8Value);
extern void MHal_RLD_SetMappingTable2to4(U16 u16Value);
extern void MHal_RLD_SetMappingTable2to8(U32 u32Value);
extern void MHal_RLD_SetMappingTable4to8(U32 u32Value0, U32 u32Value1, U32 u32Value2, U32 u32Value3);

extern void MHal_RLD_2BS_Reset(void);
extern void MHal_RLD_2BS_Start(void);
extern U8   MHal_RLD_2BS_GetReturn(void);
extern void MHal_RLD_2BS_SetCtrl(U8 u8BotField);

extern void MHal_RLD_SetNonModifying_Color(U8 u8Value);

#endif // _DRV_RLD_H_

