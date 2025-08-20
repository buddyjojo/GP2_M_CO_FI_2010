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


//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

#include "mdrv_types.h"
#include "mhal_utility.h"
#include "mhal_tcon.h"

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
//  Macro definition
//--------------------------------------------------------------------------------------------------

#define OPT_TCON_MHAL_DBG    0

#if (OPT_TCON_MHAL_DBG)
#define TCON_MHAL_PRINT(fmt, args...)      printk("[Mhal_SC][%05d] " fmt, __LINE__, ## args)
#else
#define TCON_MHAL_PRINT(fmt, args...)
#endif

#define assert(p)   do {\
	if (!(p)) {\
		printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
				__FILE__, __LINE__, #p);\
	}\
} while (0)

//-------------------------------------------------------------------------------------------------
//  Definition
//-------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

void MHal_TCON_Count_Reset(BOOL bEnable)
{
    REG_WI(TCON_REG(0x03), bEnable , BIT14);
}


