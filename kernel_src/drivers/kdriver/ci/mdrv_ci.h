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
/// file    mdrv_ci.h
/// @brief  CI Device Driver Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_CI_H_
#define _DRV_CI_H_

#include <linux/module.h>
#include <linux/delay.h>

#include "mhal_ci_pcmcia.h"

/******************************************************************************/
/*                                 Macro                                      */
/******************************************************************************/
#ifdef RED_LION
#define ASSERT(EX) // 0

#define REG(addr) (*(volatile unsigned int *)(addr))
#endif

#define DRV_CI_DEBUG_ENABLE         0

#if DRV_CI_DEBUG_ENABLE
#define DRV_CI_DEBUG(fmt, args...)  printk("" fmt, ## args)
#else
#define DRV_CI_DEBUG(fmt, args...)
#endif

#ifndef MDRV_CI_NR_DEVS
#define MDRV_CI_NR_DEVS             1
#endif

#define MAX_CI_USE_COUNTER          1

///////////////////////////////////////////////////////////////////
// Internals / CI Protocol Stack Part

#define MAX_CI_SESSIONS             (15)    //!< Configuration - Maximum number of CI sessions per slot
#define MAX_CASYS_IDS               (100)    //!< Configuration - Maximum number of CA system IDs
#define MAX_CI_MODULENAME           (50)    //!< Configuration - Maximum length of a CI module name
#define MAX_LEN_CA_PMT              (1024)    //!< MPEG2 defined maximum size of a PMT section

#define MAX_CI_SLOTS                (1)        //(2)//!< Configuration - This is the maximum number of CI slots
#define MAX_CI_RESOURCES            (20)    //!< Configuration - This is the maximum number of simultaneously registered resources

#define CI_T_SENDQSIZE              10
#define CI_T_SENDQBUFSIZE           4096
#define MAX_CI_TPDU                 4096

// the following codes are used as second byte of a CI_MSG_CI_INFO message
#define    CI_SLOT_EMPTY            (0)
#define    CI_SLOT_MODULE_INSERTED  (1)
#define    CI_SLOT_MODULE_NAME_OK   (2)
#define    CI_SLOT_MODULE_CA_OK     (3)

#define MAX_CI_MMI_DATA_BUFFER      4

/////////////////////////////////////////////////////////////////
// Transport Tag Values (EN 50221:P70)
#define CI_T_SB                     0x80 // Host <-  Module
#define CI_T_RCV                    0x81 // Host  -> Module
#define CI_T_CREATE                 0x82 // Host  -> Module
#define CI_T_CREATE_REPLY           0x83 // Host <-  Module
#define CI_T_DELETE_TC              0x84 // Host <-> Module
#define CI_T_DELETE_REPLY           0x85 // Host <-> Module
#define CI_T_REQUEST_TC             0x86 // Host <-  Module
#define CI_T_NEW_TC                 0x87 // Host  -> Module
#define CI_T_TC_ERROR               0x88 // Host  -> Module
#define CI_T_DATALAST               0xA0 // Host <-> Module
#define CI_T_DATAMORE               0xA1 // Host <-> Module
// Session Tag Values (EN 50221:P23)
#define CI_SESSION_OPEN_REQUEST     0x91
#define CI_SESSION_OPEN_RESPONSE    0x92
#define CI_SESSION_CREATE           0x93
#define CI_SESSION_CREATE_RESPONSE  0x94
#define CI_SESSION_CLOSE_REQUEST    0x95
#define CI_SESSION_CLOSE_RESPONSE   0x96
#define CI_SESSION_DATA             0x90

#define MSTOTICKS(a)                a

#ifdef _DRV_CI_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                 Enum                                       */
/******************************************************************************/

/*****************************************************************************/
/*                              Structures                                   */
/*****************************************************************************/
//! This structure holds the data for an established connection to a CI resource.
typedef struct CI_SESSION_s
{
    B16 fSlotInUse;         //!< flag if this session slot is in-use
    U8 bTransportConnID;    //!< ID of the lowlevel transport connection
    U16 wSessionID;         //!< session id
    struct RESOURCE_INFO_s *pResource;//struct RESOURCE_INFO_t const *pResource;
} CI_SESSION_t;


typedef struct CI_RXTXTPDUBUFFER_s {
    U16 wBufferSize;
    U8 CI_T_SENDQ[CI_T_SENDQSIZE][CI_T_SENDQBUFSIZE];
    U16 CI_T_SENDQ_Size[CI_T_SENDQSIZE];

} CI_RXTXTPDUBUFFER_t, *CI_TPDUBUFHANDLE_t;

typedef U16 CISESSIONHANDLE;

//! This structure represents the state of one individual CI slot
typedef struct CI_HANDLESTRUCT_s
{
    ////////////////////////////////
    // PCMCIA related
    PCMCIA_HANDLE hSlotHandle;  //!< handle to the PCMCIA Driver
    U8 bSlotNum;                //!< internal (0-based) slot number
    B16 fHighPrio;              // hint to the PCMCIA Driver (for P Modules)

    ////////////////////////////////
    // Buffer related
    CI_RXTXTPDUBUFFER_t IOBuffer_Host2CI;   //!< data buffer, contains data to be sent during the next poll
    U8 CI_T_RX[MAX_CI_TPDU];                //!< buffer for the re-assembly of incoming TPDUs
    U8 TPDU[MAX_CI_TPDU];                   //!< buffer for the re-assembly of incoming TPDUs
    U32 TPDULen;                            //!< current length of an incoming TPDU
    B16 fRXPending;                         //!< flag that is set to TRUE if an multipart TPDU is waiting to be completed
    B16 fGotPollAck;                        //!< flag that is set to TRUE in response to a T_SB (enforce 1 T_SB per poll)
    U8 bUnknown_TPDU;                       //!< unknown TPDU counter, used to trigger a reset for crashed modules

    ////////////////////////////////
    // Misc
    U8 ModulReset; // internal flag... causes a call to PCMCIA Reset later

    ////////////////////////////////
    // Session related
    CI_SESSION_t CiSessions[MAX_CI_SESSIONS];   //!< array for open sessions on this slot

    U16 wAppInfoSession;                        //!< current session handle for the application info resource (0 if none)
    char pszModuleName[MAX_CI_MODULENAME];      //!< Name of the module, according to the application info resource
    U8 bModuleNameLength;                       //!< length of the module name, according to the application info resource

    B16 fIsPremiereModule;
    B16 fModuleRequiresCAIDFiltering;

    CISESSIONHANDLE dwCCSession;

    U32 dwCASupportSession;                     //!< current session handle for the CA support resource (0 if none)
    U16 wCASystemIDs[MAX_CASYS_IDS];            //!< array of DVB CA System IDs supported by the module
    U16 wNumCASystems;                          //!< number of valid entries in the ca_system_ids array

    U32 dwDateTimeSession;                      //!< current session handle for the datetime resource (0 if none)
    U32 dwDateTimeIntervallTicks;               //!< requested interval for date/time notifications in system ticks (0 for none)
    U32 dwLastDateTimeSendTicks;                //!< System tick counter value at which the last date/time packet has been sent or 0 if no packet has been sent yet

    CISESSIONHANDLE dwMMISession;               //!< current session handle for the mmi resource (0 if none)

    CISESSIONHANDLE dwAuthSession;              //!< current session handle for the authentication resource (0 if none or unsupported)

    CISESSIONHANDLE dwMacrovisionSession;       // private

    CISESSIONHANDLE dwPinMgmtSession;           // private

    CISESSIONHANDLE dwResMgrSession;            //!< current session handle for the resource manager resource (0 if none)

    CISESSIONHANDLE dwPowerMgmtSession;         //!< current session handle for the power manager resource (0 if none)

    // Lowspeed Communication
    CISESSIONHANDLE dwLSCSession;               //!< current session handle for the low speed communication resource (0 if none)
    U8 bLSCBufferSize;
    U8 bLSCTimeout;
    B16 fLSCConnecting;
    B16 fLSCConnected;
    U8 bLSCSendPhase;
    U8 bLSCReceivePhase;
} CI_HANDLESTRUCT_t, *CI_HANDLE;

/******************************************************************************/
/*                       Global Variable Declarations                         */
/******************************************************************************/
//! This is the global SLOT status array
INTERFACE CI_HANDLESTRUCT_t gCiSlot[MAX_CI_SLOTS];

/******************************************************************************/
/*                       Global Function Prototypes                           */
/******************************************************************************/
// PCMCIA Bus Driver(s) to CI stack
//! This function boots a common interface module, after the CIS has been parsed.
//! In the context of this function, the buffer size negotiation and the TS-enable happen.
//! @param hCI Private handle of the PCMCIA slot, used for the PCMCIA API calls.
//! @param pInfo PCMCIA structure, as generated by the CIS parser
//! @param bSlotNum zero-based index of the CI slot.
INTERFACE B16 MDrv_CI_L2_CheckConfig(PCMCIA_HANDLE hCI, PCMCIA_INFO_t *pInfo, U8 bSlotNum);

INTERFACE B16 MDrv_CI_L2_WaitForCIBit(PCMCIA_HANDLE hCI, U8 BitMask);

INTERFACE void MDrv_CI_L2_WriteCOR(PCMCIA_HANDLE hPCMCIA, PCMCIA_INFO_t *pInfo, U8 bSlotNum);

// CI Stack -> Hardware layer
INTERFACE B16 MDrv_CI_L2_ReallyWriteData( CI_HANDLE hCI, U16 wBufferSize, const U8 *pData );

#ifdef __cplusplus
}
#endif


#undef INTERFACE
#endif // _DRV_CI_H_
