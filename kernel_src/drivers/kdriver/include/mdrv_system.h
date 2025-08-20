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
/// @file   drvSystem.h
/// @brief  System Control Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_SYSTEM_H_
#define _DRV_SYSTEM_H_

#include "../../../kernel/linux-2.6.26-saturn7/arch/mips/mips-boards/titania3/chip_setup.h"
#include "mst_platform.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define UART0_ENABLE   0
#define UART1_ENABLE   1
#define UART2_ENABLE   2

#define SYS_BOARD_NAME_MAX          32                                  ///< Maximum length of board name
#define SYS_PLATFORM_NAME_MAX       32                                  ///< Maximum length of playform name

#ifdef CONFIG_Titania2
#define REG_SW_RESET_CPU_AEON                   0x101086
//---------------------------------------------
// definition for REG_SW_RESET_CPU_AEON   //reg[0x1086]
#define AEON_SW_RESET                           BIT0
#endif

#ifdef CONFIG_Titania3
#define REG_PMMCU_BASE                          0xBF002000

#define REG_SW_RESET_CPU_AEON                   0x43
#define AEON_SW_RESET                           BIT0
#endif

#define REG_CKG_AEONTS0                         0x101E26
#define AEON_SPI_ADDR0                          0x100FFE
#define AEON_CLK_ENABLE                         0x00
#define AEON_CLK_DISABLE                        0x40


#define AEON_REG_CTRL                           0x100FF0

//---------------------------------------------
// definition for AEON_REG_CTRL   //reg[0x0FF0]
#define AEON_CTRL_EN                            BIT0
#define AEON_CTRL_RST                           BIT1
#define AEON_DWB_SWAP                           BIT3


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

/// System output pad switch type
typedef enum
{
	E_SYS_PAD_MSD5010_SM2_IIC2,                                         ///< 5010 SM2, IIC2
	E_SYS_PAD_MSD5011_SM2_IIC2,                                         ///< 5011 SM2, IIC2
	E_SYS_PAD_MSD5015_GPIO,                                             ///< 5015 GPIO
	E_SYS_PAD_MSD5018_SM2,                                              ///< 5018 SM2
} SYS_PadType;

/// System information
typedef struct
{
	/// Software information
	struct
	{
		U8                          Board[SYS_BOARD_NAME_MAX];          ///< Board name
		U8                          Platform[SYS_PLATFORM_NAME_MAX];    ///< Platform name
	} SWLib;
} SYS_Info;

#if 0
/// Memory mapping type
typedef enum
{
	E_SYS_MMAP_LINUX_BASE,
	E_SYS_MMAP_VD_3DCOMB,
	E_SYS_MMAP_MAD_BASE,
	E_SYS_MMAP_MVD_FB,
	E_SYS_MMAP_MVD_BS,
	E_SYS_MMAP_EMAC,
	E_SYS_MMAP_VE,
	E_SYS_MMAP_SCALER_MENU_LOAD,
	E_SYS_MMAP_SCALER_DNR_BUF,
	E_SYS_MMAP_RLD_BUF,
	E_SYS_MMAP_TTX_BUF,
	E_SYS_MMAP_MPOOL,
	E_SYS_MMAP_LINUX_2ND_MEM,
	E_SYS_MMAP_SVD,
	E_SYS_MMAP_MVD_SW,
	E_SYS_MMAP_SVD_ALL,
	E_SYS_MMAP_POSD0_MEM,
	E_SYS_MMAP_POSD1_MEM,
	E_SYS_MMAP_TSP, // samuel, 20081107
	E_SYS_MMAP_AUDIO_CLIP_MEM, // samuel, 20081107
	E_SYS_MMAP_BIN_MEM,
	E_SYS_MMAP_JPD_MEM, // Samuel, 20090108
	E_SYS_MMAP_BT_POOL,	// dreamer@lge.com, 20090112
	E_SYS_MMAP_M4VE,
#if defined(CONFIG_Titania3)
	E_SYS_MMAP_JPG_OSD,
#endif
	//#if defined(CONFIG_MSTAR_TITANIA_BD_T3_FPGA)
	E_SYS_MMAP_MIU1_BASE,
	E_SYS_MMAP_FPGA_POOL_BASE,
	//#endif
    E_SYS_MMAP_PVR_DOWNLOAD_MEM, // StevenL, 20090311
	E_SYS_MMAP_PVR_UPLOAD_MEM, // StevenL, 20090311
	E_SYS_MMAP_PVR_THUMBNAIL_DECODE_MEM, // StevenL, 20090311
	E_SYS_MMAP_NUMBER,
} SYS_Memory_Mapping;
#endif


//mail box crash protection 2009-11-06
typedef enum VDMCU_DSP_TYPE_e
{
    VDMCU_DSP_UNKNOWN = 0,
    VDMCU_DSP_DVBT,
    VDMCU_DSP_DVBC,
    VDMCU_DSP_ATSC,
    VDMCU_DSP_VIF,
} VDMCU_DSP_TYPE_t;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
BOOL MDrv_System_Init(void);
//BOOL MDrv_System_SwitchPad(SYS_PadType ePadType);
void MDrv_System_WDTEnable(BOOL bEnable);
void MDrv_System_WDTClear(void);
BOOL MDrv_System_WDTLastStatus(void);
void MDrv_System_WDTSetTime(U32 u32Ms);
void MDrv_System_ResetChip(void);
void MDrv_System_ResetCPU(void);


U32 MDrv_SYS_SetPanelInfo(U32 arg);
void MDrv_SYS_GetPanelRes(U32 arg);
PMST_PANEL_INFO_t MDrv_SYS_GetPanelInfo(void);

void MDrv_SYS_ReadGeneralRegister(U32 arg);
void MDrv_SYS_WriteGeneralRegister(U32 arg);
void MDrv_SYS_LoadAeon(U32 arg);
void MDrv_SYS_ResetAeon(U32 arg);
void MDrv_SYS_EnableAeon(void);
void MDrv_SYS_DumpAeonMessage(void);
void MDrv_SYS_DisableAeon(void);
void MDrv_SYS_SwitchUart(U32 arg);
U32 MDrv_SYS_IsAeonEnable(U32 arg);

U32 MDrv_SYS_GetRawUART(U32 arg);
void MDrv_SYS_ReloadAeon( U32 arg ) ;
U32 MDrv_SYS_Timer(U32 arg) ;
U32 MDrv_SYS_RegOP(U32 arg);
void MDrv_SYS_GetPanelRes(U32 arg);
extern void MDrv_SYS_MMAP_Dump( void ) ;
U32 MDrv_SYS_HotelMode(U32 arg) ;
U32 MDrv_SYS_HotelModePrintf(U32 arg) ;
U32 MDrv_SYS_SetUartMode(U32 arg);

U32 MDrv_SYS_SendRawUART(U32 arg);
U32 MDrv_SYS_GetRawUART(U32 arg);


void MDrv_SYS_EnableSVDCPU( int enable ) ; // samuel, 20081107
void MDrv_SYS_Enable3DCOM( int enable ) ; // samuel, 20081107

// return reserved memory address & size for specified type
extern int MDrv_SYS_GetMMAP(int type, unsigned int *addr, unsigned int *len);

void MDrv_SYS_ChangeUart( U32 arg );

void MDrv_SYS_CPU_Sync(void);

void MDrv_SYS_Flush_Memory(void);

void MDrv_SYS_Read_Memory(void);

unsigned long MDrv_SYS_GetSyetemTime(void);

void* MDrv_SYS_PA2NonCacheSeg( void* pAddrPA );

U32 MDrv_SYS_SetSeqFile(U32 arg);

U32 MDrv_SYS_GetSPI(U32 arg); //20090724 Terry, URSA ISP Load Code

BOOL MDrv_SYS_MIU_Protect(U32 arg); //20091028 Terry, MIU protect

U32 MDrv_SYS_GetRev(U32 arg);

void MDrv_SYS_LoadInitBWTable(void);                                                // 20091002 daniel.huang: add for bandwidth adjustment
void MDrv_SYS_LoadBWTable(U16 u16Input_HSize, U16 u16Input_VSize, BOOL bInterlace); // 20091002 daniel.huang: add for bandwidth adjustment

//mail box crash protection 2009-11-06
void MDrv_SYS_VDmcuSetType(VDMCU_DSP_TYPE_t type);
VDMCU_DSP_TYPE_t MDrv_SYS_VDmcuGetType(void);

#endif // _DRV_SYSTEM_H_

