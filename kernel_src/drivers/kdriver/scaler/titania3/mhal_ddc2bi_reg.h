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
/// file    reg_ddc2bi.h
/// @brief  DDC2BI Module Register Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __REG_DDC2BI_H__
#define __REG_DDC2BI_H__

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define REG_DDC2BI_BASE           0x000400
//#define DDC2BI_REG(addr)         (*((volatile U16*)(0xBF800000 + (addr << 2))))

//
// register definition
//
#define REG_DDC2BI(_x_)           (REG_DDC2BI_BASE | (_x_ << 1))

#endif // __REG_DDC2BI_H__
