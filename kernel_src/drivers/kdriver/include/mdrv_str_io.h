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
/// @file   mdrv_str.h
/// @brief  STR interface
/// @author MStar Semiconductor Inc.
///
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MDRV_STR_H__
#define __MDRV_STR_H__

/* Use 's' as magic number */
#define STR_IOC_MAGIC           's'

//------------------------------------------------------------------------------
// Signal
//------------------------------------------------------------------------------
#define STR_IOC_ENTER                   _IOR (STR_IOC_MAGIC, 0x00, unsigned int)

#endif

