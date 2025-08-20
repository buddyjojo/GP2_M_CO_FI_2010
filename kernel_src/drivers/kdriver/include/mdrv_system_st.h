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

#ifndef __DRV_SYSTEM_ST_H__
#define __DRV_SYSTEM_ST_H__

//------------------------------------------------------------------------------
// Data structure
//------------------------------------------------------------------------------
typedef struct IO_SYS_PANEL_INFO_s
{
    U32* pPanelInfo;
    U16  u16Len;
} IO_SYS_PANEL_INFO_t, *PIO_SYS_PANEL_INFO_t;

typedef struct IO_SYS_PANEL_Res_s
{
    U16  u16Width;
    U16  u16Height;
} IO_SYS_PANEL_GET_RES_t, *PIO_SYS_PANEL_GET_RES_t;

typedef struct IO_SYS_BOARD_INFO_s
{
    U32* pu32BoardInfo;
    U16  u16Len;
} IO_SYS_BOARD_INFO_t, *PIO_SYS_BOARD_INFO_t;

typedef struct IO_SYS_GENERAL_REG_s
{
    U16  u16Reg;
    U8   u8Value;
} IO_SYS_GENERAL_REG_t;

typedef struct IO_SYS_AEONBIN_INFO_s
{
	U8* pu8AeonStart;
	U32 u32Len;
	BOOL bRet;
} IO_SYS_AEONBIN_INFO_t;

typedef enum
{
    RELOAD_AEON_STOP,
    RELOAD_AEON_RESTART
} AEON_CONTROL ;

typedef struct IO_SYS_MIU_PROTECT_s
{
    U8   u8Blockx;
    U8   *pu8ProtectId;
    U32  u32Start;
    U32  u32End;
    BOOL bSetFlag;
} IO_SYS_MIU_PROTECT_t; 

#endif // __DRV_SYSTEM_ST_H__

