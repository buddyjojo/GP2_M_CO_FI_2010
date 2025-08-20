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
//------------------------------------------------------------------------------
// Define
//------------------------------------------------------------------------------
//#ifndef BOOLEAN
//    typedef B16 BOOLEAN; 
//#endif
#include "mdrv_types.h"
//#include "../system/hwreg.h"

#define SEMDBG(x) //x

#define MAX_SEM_NUM     8
#define SEM_HK51ID      0x01
#define SEM_HKAeonID    0x02
#define SEM_MWAeonID    0x03
#define SEM_REG_BASE                            0x1800 // Semaphore

#define GESEMID    01
#define PREGESEMID 02
//------------------------------------------------------------------------------
// Semaphore Register
//------------------------------------------------------------------------------
#define SEMP_ADDRESS             (0xC00*2)
#define REG_SEMP_BASE            (0xbf800000 + SEMP_ADDRESS)
#define Semaphore_Reg(address)   (*((volatile U32 *)(REG_SEMP_BASE + ((address)<<2) )))

//------------------------------------------------------------------------------
// Semaphore controller
//------------------------------------------------------------------------------
#define REG_SEM_ID0                             (SEM_REG_BASE+0x00)
#define REG_SEM_ID1                             (SEM_REG_BASE+0x02)
#define REG_SEM_ID2                             (SEM_REG_BASE+0x04)
#define REG_SEM_ID3                             (SEM_REG_BASE+0x06)
#define REG_SEM_ID4                             (SEM_REG_BASE+0x08)
#define REG_SEM_ID5                             (SEM_REG_BASE+0x0A)
#define REG_SEM_ID6                             (SEM_REG_BASE+0x0C)
#define REG_SEM_ID7                             (SEM_REG_BASE+0x0E)

extern U8 MHal_SEM_Get_Resource(U8 u8SemID);
extern void MHal_SEM_Free_Resource(U8 u8SemID);
extern void MHal_SEM_Reset_Resource(U8 u8SemID);
extern U8  MHal_SEM_Get_ResourceID(U8 u8SemID);

