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

#ifndef __DEV_MIU_H__
#define __DEV_MIU_H__

/* Use 'M' as magic number */
#define MIU_IOC_MAGIC           'm'

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Data structure
//------------------------------------------------------------------------------

// DEV_MIU_IOC_DRAM_PROTECT
typedef struct
{
    MS_U32                         u32AddrStart;
    MS_U32                         u32AddrEnd;
    MS_U8                          u8Idx;
    MS_U8                          u8AllowIP;
} DevMIU_DramProtect_t;


//-------------------------------------------------------------------------------------------------
//  ioctl method
//-------------------------------------------------------------------------------------------------

// void MDrv_MIU_Init(void);
#define DEV_MIU_IOC_INIT                _IO  (MIU_IOC_MAGIC, 0x00)

// void MDrv_MIU_DRAM_Protect(MS_U8 u8Idx, MS_U32 u32StartAddr, MS_U32 u32EndAddr, MS_U8 u8AllowIP);
#define DEV_MIU_IOC_DRAM_PROTECT        _IOW (MIU_IOC_MAGIC, 0x01, MS_U32)

// void MDrv_MIU_DRAM_UnProtect(MS_U8 u8Idx);
#define DEV_MIU_IOC_DRAM_UNPROTECT      _IOW (MIU_IOC_MAGIC, 0x02, MS_U32)

#endif // #ifndef __DEV_MIU_H__
