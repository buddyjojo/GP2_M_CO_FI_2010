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

///////////////////////////////////////////////////////////////////////////////
///
/// file    mhal_ci_pcmcia.c
/// @brief  PCMCIA Device Driver Interface of CI Physical Layer
///         This file contains the API of the PCMCIA CIS parser and the declaration.
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef _DRVCI_PCMCIA_H_
#define _DRVCI_PCMCIA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mdrv_types.h"
#include "chip_setup.h"

/******************************************************************************/
/*                                 Macro                                      */
/******************************************************************************/
#define PCMCIA_IRQ_ENABLE           0
//#define STORE_CISVERS1_INFO 0 //!< Set to 1 if you want to store the information.
#define STORE_CISVERS1_INFO         1 //!< Set to 1 if you want to store the information.

//wgkwak in LGE
#define    PCMCIA_RESETDELAY_ZERO   0
#define    PCMCIA_RESETDELAY_MAX    2000    //Max reset delay for Conax CXV CAM

//hardware spec
#define MDRV_PCMCIA_REG_BASE        (REG_MIPS_BASE + 0x3440 * 2)
#define CI_REG(addr)                (MDRV_PCMCIA_REG_BASE + (addr))

#define MAX_CIS_SIZE                256     //(0x150)//!< The maximum size of a CIS, that is understood by this driver
#define MAX_PCMCIA_CONFIGS          (10)    //!< The maximum number of configurations supported by a PCMCIA card
#define MAX_PCMCIA_STRLEN           (30)    //!< The maximum name of vendor/manufacturer/info strings
#define PHY_TIMEOUT                 100

#define REG_PCM_MEM_IO_CMD          0x00
#define REG_ADDR0                   0x02
#define REG_ADDR1                   0x03
#define REG_WRITE_DATA              0x04
#define REG_FIRE_READ_DATA_CLEAR    0x06
#define REG_READ_DATA               0x08
#define REG_READ_DATA_DONE_BUS_IDLE 0x09
#define REG_INT_MASK_CLEAR          0x0A
#define REG_INT_MASK_CLEAR1         0x0B
#define REG_STAT_INT_RAW_INT        0x0E
#define REG_STAT_INT_RAW_INT1       0x0F
#define REG_MODULE_VCC_OOB          0x10

#define ATTRIBMEMORY_READ           0x0003
#define ATTRIBMEMORY_WRITE          0x0004
#define IO_READ                     0x0005
#define IO_WRITE                    0x0006

// Command interface hardware register
#define CI_PHYS_REG_DATA            (0)
#define CI_PHYS_REG_COMMANDSTATUS   (1)
#define CI_PHYS_REG_SIZELOW         (2)
#define CI_PHYS_REG_SIZEHIGH        (3)

// Status register bits
#define CISTATUS_DATAAVAILABLE      (0x80)  //!< CI Status register bit - The module wants to send data
#define CISTATUS_FREE               (0x40)  //!< CI Status register bit - The module can accept data
#define CISTATUS_IIR                (0x10)  //!< CI Status register bit - The module can accept data
#define CISTATUS_RESERVEDBITS       (0x2C)  //!< CI Status register bits - reserved
#define CISTATUS_WRITEERROR         (0x02)  //!< CI Status register bit - Write error
#define CISTATUS_READERROR          (0x01)  //!< CI Status register bit - Read error

#define CICOMMAND_DAIE              (0x80)    //!< CI Command register bit - DAIE
                                              // when this bit is set, the module asserts IREQ# each time it has data to send
#define CICOMMAND_FRIE              (0x40)    //!< CI Command register bit - FRIE
                                              // when this bit is set, the module asserts IREQ# each time it is free to receive data
#define CICOMMAND_RESERVEDBITS      (0x30)    //!< CI Command register bits - reserved
#define CICOMMAND_RESET             (0x08)  //!< CI Command register bit - Reset
#define CICOMMAND_SIZEREAD          (0x04)  //!< CI Command register bit - Size read
#define CICOMMAND_SIZEWRITE         (0x02)  //!< CI Command register bit - Size Write
#define CICOMMAND_HOSTCONTROL       (0x01)  //!< CI Command register bit - Host control

// CI message Type
#define CIMSG_NONE                  0x00
#define CIMSG_DETECTED              0x01
#define CIMSG_REMOVED               0x02
#define CIMSG_INVALID               0x03
#define CIMSG_DISPLAY               0x04
#define CIMSG_LOADINFO              0x05
#define CIMSG_LOAD_NONE             0x06
#define CIMSG_NOMODULE              0x07
#define CIMSG_NOMODULE_DISPLAY      0x08
#define CIMSG_TRYAGAIN              0x09

#define PCMCIAINFO_MANID_VALID      (0x00000001)
#define PCMCIAINFO_VERS1_VALID      (0x00000002)
#define PCMCIAINFO_FUNCID_VALID     (0x00000004)

#ifdef _DRVCI_PCMCIA_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

/******************************************************************************/
/*                                 Enum                                       */
/******************************************************************************/
//! This enum contains the card types, that can be encoded in CISTPL_FUNCID (0x21)
typedef enum {
    E_PCMCIA_FUNC_CUSTOM        = 0,
    E_PCMCIA_FUNC_MEMORY        = 1,
    E_PCMCIA_FUNC_SERIAL        = 2,
    E_PCMCIA_FUNC_PARALLEL      = 3,
    E_PCMCIA_FUNC_DRIVE         = 4,
    E_PCMCIA_FUNC_VIDEO         = 5,
    E_PCMCIA_FUNC_NETWORK       = 6,
    E_PCMCIA_FUNC_AIMS          = 7,
    E_PCMCIA_FUNC_SCSI          = 8,
    E_PCMCIA_FUNC_CARDBUS       = 9,
    E_PCMCIA_FUNC_MANUFACTURER  = 0xFF      // manifacturer designed purpose
} PCMCIA_FUNCTYPE;

/*****************************************************************************/
/*                              Structures                                   */
/*****************************************************************************/
//! This structure contains information about exactly one possible PCMCIA card configuration.
typedef struct PCMCIA_CONFIG_s
{
    U8  bConfigIndex;       //!< cor //!< The value of bConfigIndex has to be written into the card configuration register to activate this configuration.
    U8  bIRQDesc1;          //!< Interrupt descriptor byte
    U16 wIRQData;           //!< supported IRQ number mask
    U32 dwEAAddr;           //!< EA-address
    U32 dwEALen;            //!< size of the EA area (zero for none)
    U8  fCITagsPresent;     //!< Bitfield which is 0x03 if both required CI tags are present. 0x01 for DVB_HOST, 0x02 for DVB_CI_MODULE
} PCMCIA_CONFIG_t;

//! This structure provides simple access to the PCMCIA card information
//! after decoding of the Card Information Structure. This decoding is performed by MDrv_CI_PCMCIA_ReadAttribMem()
typedef struct PCMCIA_INFO_s
{
    U32 ConfigOffset;                               //!< Offset of the Configuration byte in the Attribute Memory
    U32 dwValidFlags;                               //!< Bitmask that defines which of the other fields are valid
    U16 wManufacturerId;                            //!< 16Bit Manufacturer ID (PCMCIAINFO_MANID_VALID)
    U16 wCardID;                                    //!< 16Bit Card ID (PCMCIAINFO_MANID_VALID)
#if STORE_CISVERS1_INFO
    U16 wPCMCIAStdRev;                              //!< PCMCIA Standard version supported by the card (PCMCIAINFO_VERS1_VALID)
    char pszManufacturerName[MAX_PCMCIA_STRLEN];    //!< Name of the card manufacturer (PCMCIAINFO_VERS1_VALID)
    char pszProductName[MAX_PCMCIA_STRLEN];         //!< Product name (PCMCIAINFO_VERS1_VALID)
    char pszProductInfo1[MAX_PCMCIA_STRLEN];        //!< (PCMCIAINFO_VERS1_VALID)
    char pszProductInfo2[MAX_PCMCIA_STRLEN];        //!< (PCMCIAINFO_VERS1_VALID)
#endif
    U8 bCI_PLUS;                                    //!< CI plus CAM or not
    PCMCIA_FUNCTYPE FuncType;                       //!< Card function type (PCMCIAINFO_FUNCID_VALID)
    U8 bFuncIDSysInfo;                              //!< SysInitByte from the FuncID block (PCMCIAINFO_FUNCID_VALID)
    //wgkwak in LGE
    U8 bCAMIsInserted;
    U8 bINT;                                        //!< PCMCIA card Support interrupt or not
    U8 bNumConfigs;                                 //!< The number of configurations supported by the card. Exactly bNumConfigs entries are valid in the Config array.
    PCMCIA_CONFIG_t Config[MAX_PCMCIA_CONFIGS];     //!< The array of possible card configurations
} PCMCIA_INFO_t;

//! This file typedefs PCMCIA_HANDLE as void*.
//! Actual PCMCIA driver implementations can hide whatever they want inside this handle.
//! Higher level drivers pass
typedef void *PCMCIA_HANDLE;

/******************************************************************************/
/*                       Global Variable Declarations                         */
/******************************************************************************/
INTERFACE U8 gu8CI_Command;

/******************************************************************************/
/*                       Global Function Prototypes                           */
/******************************************************************************/
INTERFACE void MDrv_CI_PCMCIA_ReadAttribMem(U16 Addr, U8 *pDest);

//! This function decodes a raw attribute memory dump into an easily readable PCMCIA_INFO structure.
//! The PCMCIA/PnP task is responsible for reading the raw attribute memory. This function
//! parses the card info structure (CIS) and decodes the relevant parts. Callers should check the
//! dwValidFlags Bitmask before accessing other fields.
INTERFACE B16 MDrv_CI_PCMCIA_ParseAttribMem(const U8 *pAttribMem, U16 dwLen, PCMCIA_INFO_t *pInfo);
//! Initialization function. During system initialization, this function
//! is called once to initialize the driver.
INTERFACE void MDrv_PCMCIA_Init(PCMCIA_INFO_t *_PCMCIA_Info);
//! This function is called by higher level drivers to check if a module is (still) present.
//! Usually, this check is performed by checking the card detect GPIO pins of a PCMCIA slot.
INTERFACE B16 MDrv_CI_PCMCIA_IsModuleStillPlugged(void);
//! This function is called to write the byte bData into the card attribute memory at address wAddr.
INTERFACE void MDrv_CI_PCMCIA_WriteAttribMem(PCMCIA_HANDLE hSlot,U16 wAddr,U8 bData);
//! This function is called to write the byte bData into the card IO memory at address wAddr.
INTERFACE void MDrv_CI_PCMCIA_WriteIOMem(PCMCIA_HANDLE hSlot,U16 wAddr,U8 bData);
//! This function is read one byte of from the card IO memory at address wAddr.
INTERFACE U8 MDrv_CI_PCMCIA_ReadIOMem(PCMCIA_HANDLE hSlot,U16 wAddr);
//! This function is called to enable or disable the TS stream for PCMCIA common interface slots.
//! @param hSlot Handle for the slot for which the TS is to be en-/disabled.
//! @param fEnable TRUE to enable the TS, FALSE otherwise.
INTERFACE void MDrv_CI_PCMCIA_EnableTSRouting(PCMCIA_HANDLE hSlot,B16 fEnable);

//wgkwak in LGE
INTERFACE void MDrv_CI_PCMCIA_HwRstPCMCIA(unsigned int resetDelay);
INTERFACE void MDrv_CI_PCMCIA_ISR(void);

INTERFACE B16 MDrv_CI_PCMCIA_ResetInterface(void);
INTERFACE void MDrv_CI_PCMCIA_SwitchBypassMode(B16 Mode);
INTERFACE void MDrv_CI_PCMCIA_SwitchPower(U8 u8OnOff);

#if PCMCIA_IRQ_ENABLE
void MDrv_PCMCIA_Enable_Interrupt( BOOL bEnable );
void MDrv_PCMCIA_Clear_Interrupt( void );
void MDrv_PCMCIA_Set_InterruptStatus( BOOL Status );
BOOL MDrv_PCMCIA_Get_InterruptStatus( void );
#endif

#ifdef __cplusplus
}
#endif

#undef INTERFACE

#endif // _DRVCI_PCMCIA_H_
