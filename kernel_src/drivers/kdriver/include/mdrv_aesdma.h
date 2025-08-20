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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvAESDMA.h
/// @brief  AESDMA Driver Interface
/// @author MStar Semiconductor,Inc.
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRVAESDMA_H_
#define _DRVAESDMA_H_
#include "mdrv_types.h"
#include "mdrv_aesdma_st.h"


//--------------------------------------------------------------------------------------------------
//  Function Prototype
//--------------------------------------------------------------------------------------------------
//DRVAESDMA_RESULT MDrv_AESDMA_Init(void);
DRVAESDMA_RESULT MDrv_AESDMA_Init(AESDMA_INIT_t *AesdmaInit);
DRVAESDMA_RESULT MDrv_AESDMA_SelEng(AESDMA_SELENG_t *aesdma_seleng);
DRVAESDMA_RESULT MDrv_AESDMA_SetPS(AESDMA_SETPS_t *aesdma_setps);
DRVAESDMA_RESULT MDrv_AESDMA_SetFileInOut(AESDMA_SETFILEINOUT_t *aesdma_seleng);
DRVAESDMA_RESULT MDrv_AESDMA_Start(BOOL bStart);
DRVAESDMA_RESULT MDrv_AESDMA_Reset(void);
DRVAESDMA_RESULT MDrv_AESDMA_GetStatus(void);
DRVAESDMA_RESULT MDrv_AESDMA_GetPSMatchedByteCNT(void);
DRVAESDMA_RESULT MDrv_AESDMA_GetPSMatchedPTN(void);
DRVAESDMA_RESULT MDrv_AESDMA_Notify(AESDMA_NOTIFY_t *aesdma_notify);
DRVAESDMA_RESULT MDrv_AESDMA_Random(AESDMA_RANDOM_t *aesdma_random);

#endif // _DRVAESDMA_H_
