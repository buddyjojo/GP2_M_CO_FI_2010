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
/// @file   mdrv_cec.h
/// @brief  CEC(Consumer Electronics Control) Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_CEC_H_
#define _MDRV_CEC_H_

#include <asm-mips/types.h>
#include "mdrv_cec_st.h"
#include "mdrv_cec_io.h"
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MDrv_CEC_Init(void);
void MDrv_CEC_ChkDevs(CEC_INFO_LIST_t *pCEC_Info);
void MDrv_CEC_RxChkBuf(void);
void MDrv_CEC_RxApi(CEC_INFO_LIST_t *pCEC_Info);
void MDrv_CEC_TxApi(CEC_TX_INFO_t* pCEC_TxInfo);
void MDrv_CEC_GetResult(CEC_TX_INFO_t* pCEC_TxInfo);
U16 MDrv_CEC_Response(void);

#endif // _MDRV_CEC_H_
