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
#ifndef _DRVSEMPHORE_H
#define _DRVSEMPHORE_H
////////////////////////////////////////////////////////////////////////////////
/// @file drvSemaphore.h
/// @author MStar Semiconductor Inc.
/// @brief Semapphore control driver
////////////////////////////////////////////////////////////////////////////////
#ifdef _DRVSEMPHORE_C
#define INTERFACE
#else
#define INTERFACE extern
#endif

//#include "../include/mdrv_types.h"
#include "mhal_semaphore.h"


#ifndef BOOLEAN
  //  typedef B16 BOOLEAN;
      typedef unsigned char BOOLEAN;
#endif
//------------------------------------------------------------------------------
// Enum
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Extern Global Variabls
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------
extern U8 MDrv_SEM_Get_Resource(U8 u8SemID);
extern void MDrv_SEM_Free_Resource(U8 u8SemID);
extern void MDrv_SEM_Free_Resource(U8 u8SemID);
extern U8  MDrv_SEM_Get_ResourceID(U8 u8SemID);
extern void MDrv_SEM_Reset_Resource(U8 u8SemID);



#undef INTERFACE
#endif  //_DRVSEMPHORE_H

