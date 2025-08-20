////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef Main_Host_Only__H
#define Main_Host_Only__H

#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
#define HW_BASE         0xbf200000
#else
#define HW_BASE         0xbf800000
#endif

//=================== 1.Condition Definition  ============================================================
//========================================================================================================

#define mbFUSBH200_HALFSPEEDEnable_Set()	  		(mwHost20Bit_Set(0x40,BIT2))
#define mbFUSBH200_Interrupt_OutPut_High_Set()		(mwHost20Bit_Set(0x40,BIT3))
#define mbFUSBH200_Interrupt_OutPut_Low_Set() 		(mwHost20Bit_Clr(0x40,BIT3))
#define mbFUSBH200_VBUS_OFF_Set()		      		(mwHost20Bit_Set(0x40,BIT4))
#define mbFUSBH200_VBUS_ON_Set()   		      		(mwHost20Bit_Clr(0x40,BIT4))
#define mbFUSBH200_VBUS_VLD_Rd()   		      		(mwHost20Port(0x40)&BIT8)

#define USBWrite_REG32(reg_adr, data)				( (*( (volatile MS_U32 *) (reg_adr) ) ) = ((MS_U32)data) )
#define USBRead_REG32(reg_adr)       				( *( (volatile MS_U32 *) (reg_adr) ) )
#define USBWrite_REG16(reg_adr, data)				( (*( (volatile MS_U16 *) (reg_adr) ) ) = ((MS_U16)data) )
#define USBRead_REG16(reg_adr)       				( *( (volatile MS_U16 *) (reg_adr) ) )
#define USBWrite_REG8(reg_adr, data)				( (*( (volatile UINT8 *) (reg_adr) ) ) = ((UINT8)data) )
#define USBRead_REG8(reg_adr)       				( *( (volatile UINT8 *) (reg_adr) ) )
#define XBYTE   									((unsigned char volatile *)0)

#define UTMIBaseAddr								0xbf807500
//#define UTMI_RDREG8(reg_adr)						( *( (volatile U8 *) (0xbf806280+reg_adr*2) ) )

#define UTMI_WRREG8(reg_adr, data)      			( (*( (volatile UINT8 *)  (HW_BASE+0x7500+reg_adr*2) ) ) = ((UINT8)data) )
#define UTMI_WRREG_ODD8(reg_adr, data)      		( (*( (volatile UINT8 *)  (HW_BASE+0x7500+reg_adr*2-1) ) ) = ((UINT8)data) )
#define UTMI_RDREG8(reg_adr)        				(*( (volatile UINT8 *)  (HW_BASE+0x7500+reg_adr*2) ) )
#define UTMI_RDREG_ODD8(reg_adr)      				(*( (volatile UINT8 *)  (HW_BASE+0x7500+reg_adr*2-1) ) )

extern void FUSBH200_Driver_VBUS(void);
extern void FUSBH200_Drop_VBUS(void);


#endif
