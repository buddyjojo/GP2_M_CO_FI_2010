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
/// @file   mdrv_iic.h
/// @brief  IIC Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_IIC_H_
#define _DRV_IIC_H_

#include <asm-mips/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

// added by LGE(dreamer@lge.com)
#define	IIC_NUM_OF_MAX				(14)
#define	IIC_NUM_OF_HW				(1)



#define IIC_NUM_OF_NEC_MICOM		(3)
#define IIC_NUM_OF_AUDIO_AMP		(4)



#define IIC_NUM_OF_RGB_EDID			(8)
#define IIC_NUM_OF_HDMI_A_EDID		(9)
#define IIC_NUM_OF_HDMI_B_EDID		(10)
#define IIC_NUM_OF_HDMI_C_EDID		(11)
#define IIC_NUM_OF_HDMI_D_EDID		(12)


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
// for software IIC
void MDrv_SW_IIC_SetSpeed(U8 u8ChIIC, U8 u8Speed);
void MDrv_SW_IIC_Delay(U8 u8ChIIC);
//dhjung LGE
#if 0
void MDrv_SW_IIC_SelectBus(U8 u8BusChn);
#endif
void MDrv_SW_IIC_SCL(U8 u8ChIIC, U8 u8Data);
void MDrv_SW_IIC_SDA(U8 u8ChIIC, U8 u8Data);
void MDrv_SW_IIC_SCL_Chk(U8 u8ChIIC, B16 bSet);
void MDrv_SW_IIC_SDA_Chk(U8 u8ChIIC, B16 bSet);
B16 MDrv_SW_IIC_Start(U8 u8ChIIC);
void MDrv_SW_IIC_Stop(U8 u8ChIIC);
B16 MDrv_IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack);
B16 MDrv_SW_IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack);
B16 MDrv_SW_IIC_AccessStart(U8 u8ChIIC, U8 u8SlaveAdr, U8 u8Trans);
U8 MDrv_SW_IIC_GetByte (U8 u8ChIIC, U16 u16Ack);
S32 MDrv_SW_IIC_Write(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf);
S32 MDrv_SW_IIC_Read(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf);
void MDrv_SW_IIC_Enable( U8 u8ChIIC, BOOL bEnable );	//	added for RGB EDID by LGE(dreamer@lge.com)

// for hardware IIC
void MDrv_IIC_Init(void);
void MDrv_HW_IIC_Clock_Select(U8 u8ClockIIC);
S32 MDrv_HW_IIC_Write(U8 u8SlaveIdIIC, U8 u8AddrSizeIIC, U8 *pu8AddrIIC, U32 u32BufSizeIIC, U8 *pu8BufIIC);
S32 MDrv_HW_IIC_Read(U8 u8SlaveIdIIC, U8 u8AddrSizeIIC, U8 *pu8AddrIIC, U32 u32BufSizeIIC, U8 *pu8BufIIC);
void MDrv_HW_IIC_Init( void );		//	added for MICOM F/W by LGE(dreamer@lge.com)

#endif // _DRV_IIC_H_

