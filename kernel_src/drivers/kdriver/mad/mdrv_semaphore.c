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

////////////////////////////////////////////////////////////////////////////////
/// @file drvSemaphore.c
/// @author MStar Semiconductor Inc.
/// @brief Semapphore control driver
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// Header Files
//------------------------------------------------------------------------------
// Global Layer
//#include <intrins.h>
//#include <string.h>
//#include "datatype.h"
//#include "../system/hwreg.h"
#include "mdrv_semaphore.h"
//#include "mhal_semaphore.h"
//#include "drvRIU.h"
//#include "MsCommon.h"

///-----------------------------------------------------------------------------
/// Grab the Semaphore for Specific HW IP Resource
/// @param u8SemID \b IN: ID for specific HW IP resource
/// @return Success or not
/// - TRUE, HK51 own the HW IP Resource
/// - FALSE, HK51 fail to grab the HW IP Resource
///-----------------------------------------------------------------------------
//===================================
//   mcuID => SEM_HK51ID    = 0x01
//   mcuID => SEM_HKAeon0ID = 0x02
//   mcuID => SEM_MWAeon1ID = 0x03
//===================================
U8 MDrv_SEM_Get_Resource(U8 u8SemID)
{
    return (MHal_SEM_Get_Resource(u8SemID));
}

///-----------------------------------------------------------------------------
/// Free the Semaphore for Specific HW IP Resource
/// @param u8SemID \b IN: ID for specific HW IP resource
///-----------------------------------------------------------------------------
void MDrv_SEM_Free_Resource(U8 u8SemID)
{
    MHal_SEM_Free_Resource(u8SemID);
}

//===================================
//   value = 0x00 => release
//===================================
void MDrv_SEM_Reset_Resource(U8 u8SemID)
{
    MHal_SEM_Reset_Resource(u8SemID);
}

// to check who own the reource
U8  MDrv_SEM_Get_ResourceID(U8 u8SemID)
{
    return (MHal_SEM_Get_ResourceID(u8SemID));
}

