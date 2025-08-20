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
/// file    reg_mod.h
/// @brief  MOD Module Register Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __REG_MOD_H__
#define __REG_MOD_H__

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define REG_MOD_BASE               0x103200
#define REG_TCON_BASE              0x103000
#define MOD_REG(_x_)               (REG_MOD_BASE | (_x_ << 1))
#define TCON_REG(_x_)              (REG_TCON_BASE | (_x_ << 1))

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

// FitchHsu 20081116 LVDS Power
//
// MOD bank
//
#define REG_MOD_BK_00                0x00
#define REG_MOD_BK_01                0x01

#endif // __REG_MOD_H__
