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
/// @file   mdrv_pwm_io.h
/// @brief  PWM Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define PWM_IOC_MAGIC               'p'

#define MDRV_PWM_INIT               _IO(PWM_IOC_MAGIC, 0)
#define MDRV_PWM_OEN                _IOW(PWM_IOC_MAGIC, 1, PWM_Param_t)
#define MDRV_PWM_PERIOD             _IOW(PWM_IOC_MAGIC, 2, PWM_Param_t)



#define MDRV_PWM_GRP0_CLK_GATE_EN  _IOW(PWM_IOC_MAGIC, 3, B16)
#define MDRV_PWM_GRP1_CLK_GATE_EN  _IOW(PWM_IOC_MAGIC, 4, B16)
#define MDRV_PWM_GRP2_CLK_GATE_EN  _IOW(PWM_IOC_MAGIC, 5, B16)
#define MDRV_PWM_DUTY               _IOW(PWM_IOC_MAGIC, 6, PWM_Param_t)
#define MDRV_PWM_DIV                _IOW(PWM_IOC_MAGIC, 7, PWM_Param_t)
#define MDRV_PWM_POLARITY          _IOW(PWM_IOC_MAGIC, 8, PWM_Param_t)
#define MDRV_PWM_VDBEN              _IOW(PWM_IOC_MAGIC, 9, PWM_Param_t)
#define MDRV_PWM_RESET_EN           _IOW(PWM_IOC_MAGIC, 10, PWM_Param_t)
#define MDRV_PWM_DBEN               _IOW(PWM_IOC_MAGIC, 11, PWM_Param_t)
#define MDRV_PWM_RST_MUX            _IOW(PWM_IOC_MAGIC, 12, PWM_Param_t)
#define MDRV_PWM_HS_RST_CNT         _IOW(PWM_IOC_MAGIC, 13, PWM_Param_t)
#define MDRV_PWM_PERIOD_EXT         _IOW(PWM_IOC_MAGIC, 14, PWM_Param_t)
#define MDRV_PWM_DUTY_EXT           _IOW(PWM_IOC_MAGIC, 15, PWM_Param_t)
#define MDRV_PWM_DIV_EXT            _IOW(PWM_IOC_MAGIC, 16, PWM_Param_t)
#define MDRV_PWM_SHIFT              _IOW(PWM_IOC_MAGIC, 17, PWM_Param_t)


#define PWM_IOC_MAXNR               18
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------


