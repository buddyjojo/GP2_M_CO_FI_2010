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

#ifndef _MHAL_JPD_H_
#define _MHAL_JPD_H_

#include "mdrv_types.h"


#ifdef _MHAL_JPD_C_
#define JPD_INTERFACE
#else
#define JPD_INTERFACE extern
#endif

// Pluto later
#define JPD_MWC_WPEN        (1<<12)
#define JPD_MWC_ACTIVE        (1<<11)
#define JPD_MRC_ACTIVE        (1<<10)
#define JPD_MWC_PAUSE        (1<<9)
#define JPD_MRC_PAUSE        (1<<8)

/************* Downscale Ratio *************/
/*** Bellows are 1, 1/2, 1/4 and 1/8, in order ***/
#define JPD_DOWNSCALE_ORG       0x00
#define JPD_DOWNSCALE_HALF      0x01
#define JPD_DOWNSCALE_FOURTH    0x02
#define JPD_DOWNSCALE_EIGHTH    0x03
/********************I*********************/

/*** Enable interrupt (event report) ***/
#define JPD_EVENT_DEC_DONE      0x01
#define JPD_EVENT_ECS_ERROR     0x02
#define JPD_EVENT_IS_ERROR      0x04
#define JPD_EVENT_RST_ERROR     0x08
#define JPD_EVENT_MRBL_DONE     0x10
#define JPD_EVENT_MRBH_DONE     0x20
#define JPD_EVENT_ALL           0x3f
#define JPD_EVENT_ERROR_MASK    0x0E
/********************************/

JPD_INTERFACE void MHal_JPD_Initialize(void);
JPD_INTERFACE void MHal_JPD_Reset(void);
JPD_INTERFACE U16 MHal_JPD_GetSWResetMask(void);
JPD_INTERFACE void MHal_JPD_SW_Pause_Reset(void);
JPD_INTERFACE void MHal_JPD_SetReadBuffer(U32 u32BufAddr, U32 u32BufSize);
JPD_INTERFACE void MHal_JPD_SetMRCStartAddr(U32 u32ByteOffset);
JPD_INTERFACE void MHal_JPD_SetOutputFrameBuffer(U32 u32BufAddr);
JPD_INTERFACE void MHal_JPD_SetPicDimension(U16 u16Width, U16 u16Height);
JPD_INTERFACE U16 MHal_JPD_ReadJPDStatus(void);
JPD_INTERFACE void MHal_JPD_ClearJPDStatus(U16 status_bit);
JPD_INTERFACE void MHal_JPD_SetROI(U16 start_x, U16 start_y, U16 width, U16 height);
JPD_INTERFACE void MHal_JPD_SetClock(U8 on);
JPD_INTERFACE U32 MHal_JPD_ReadCurrentMRCAddr(void);
//JPD_INTERFACE void MHal_JPD_SetMIU(int miu_no);
JPD_INTERFACE void MHal_JPD_SetClock(U8 on);
#endif
