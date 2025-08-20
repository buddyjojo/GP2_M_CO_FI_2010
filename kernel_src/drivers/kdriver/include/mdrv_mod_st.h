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
#ifndef __DRV_MOD_ST_H__
#define __DRV_MOD_ST_H__

//------------------------------------------------------------------------------
// Structure
//------------------------------------------------------------------------------
typedef enum
{
    MOD_CTRL0_LVDS_TI       = BIT2,
    MOD_CTRL0_PDP_10BIT     = BIT3,
    MOD_CTRL0_LVDS_PLASMA   = BIT4,
    MOD_CTRL0_CH_POLARITY   = BIT5,
    MOD_CTRL0_CH_SWAP       = BIT6,
} MOD_CTRL0_e;

typedef enum
{
    MOD_CTRLA_ABS_SWITCH    = BIT0,
    MOD_CTRLA_DUAL_MODE     = BIT1,
    MOD_CTRLA_DE_INVERT     = BIT2,
    MOD_CTRLA_VS_INVERT     = BIT3,
    MOD_CTRLA_CLK_INVERT    = BIT4,
} MOD_CTRLA_e;


#endif // __DRV_MOD_ST_H__ 

