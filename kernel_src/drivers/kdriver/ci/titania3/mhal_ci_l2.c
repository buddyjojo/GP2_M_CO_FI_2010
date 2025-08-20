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
/// file    mhal_ci_l2.c
/// @brief  CI Link Layer Device Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
/*                       Header Files                                        */
/*****************************************************************************/
#include "mdrv_ci.h"
#include "mhal_ci_pcmcia.h"

/*****************************************************************************/
/*                       Define                                              */
/*****************************************************************************/
//#define CI_DUMP_RX
//#define CI_DUMP_TX
//#define CI_DEBUG_ENABLE

/*****************************************************************************/
/*                       Macro                                               */
/*****************************************************************************/

/*****************************************************************************/
/*                       Global Variables                                    */
/*****************************************************************************/

/*****************************************************************************/
/*                       Local Variables                                     */
/*****************************************************************************/

/*****************************************************************************/
/*                       Local Functions                                     */
/*****************************************************************************/
static void _MDrv_CI_L2_WriteBufferSize( PCMCIA_HANDLE hCI, U16 wBufferSize );

static void _MDrv_CI_L2_WriteBufferSize( PCMCIA_HANDLE hCI, U16 wBufferSize )
{
    U16 u16TryLoop = 0;
    
    gu8CI_Command |= CICOMMAND_SIZEWRITE;                                           // UP SW
    MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
    gu8CI_Command |= CICOMMAND_HOSTCONTROL;                                         // UP HC
    MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
    while ( !( MDrv_CI_L2_WaitForCIBit( hCI, CISTATUS_FREE ) ) && ( MDrv_CI_PCMCIA_IsModuleStillPlugged() ) && ( u16TryLoop < 10 ) )
    {
        gu8CI_Command &= ( ~CICOMMAND_HOSTCONTROL );                                // DOWN HC
        MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
        msleep(1);
        gu8CI_Command |= CICOMMAND_HOSTCONTROL;                                     // UP HC
        MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
        u16TryLoop++;
    }
    
    if ( 10 == u16TryLoop )
    {
        DRV_CI_DEBUG("CI _MDrv_CI_L2_WriteBufferSize: \"Fail\"\r\n");
    }

    MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_SIZELOW, 0x02 );
    MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_SIZEHIGH, 0x00 );
    MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_DATA, ( U8 )( wBufferSize >> 8 ) );
    MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_DATA, ( U8 )( wBufferSize ) );

    msleep(50);

    gu8CI_Command &= ( ~( CICOMMAND_SIZEWRITE | CICOMMAND_HOSTCONTROL ) );          // DOWN SW & HC
    MDrv_CI_PCMCIA_WriteIOMem( hCI, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );

    msleep(10);
}

// ----------------------------------------------
void MDrv_CI_L2_WriteData(CI_TPDUBUFHANDLE_t hBuf, U16 wBufferSize, const U8 *pData)
{
    int i;
    if ( wBufferSize > hBuf->wBufferSize )
    {
        DRV_CI_DEBUG("ERROR: MDrv_CI_L2_WriteData Packet too big %08X %08X\r\n", wBufferSize, hBuf->wBufferSize);
        return;
    }
    for ( i = 0; i < CI_T_SENDQSIZE; i++ )
    {
        if ( !hBuf->CI_T_SENDQ_Size[i] )
        {
            memcpy( hBuf->CI_T_SENDQ[i], pData, wBufferSize );
            hBuf->CI_T_SENDQ_Size[i] = ( U16 )wBufferSize & 0xFFFF;
            return;
        }
    }
    DRV_CI_DEBUG("ERROR: MDrv_CI_L2_WriteData, no free tx slot\r\n");
}

// ----------------------------------------------
B16 MDrv_CI_L2_ReallyWriteData( CI_HANDLE hCI, U16 wBufferSize, const U8 *pData )
{
    int i, u16TryLoop = 0;
    PCMCIA_HANDLE hPCMCIA;
    hPCMCIA = hCI->hSlotHandle;

    #if 0//def CI_DUMP_TX
    printk("CI TX:\n");
    for ( i = 0; i < wBufferSize; i++ )
    {
        printk("%02bx ", pData[i]);
    }
    printk("\r\n");
    #endif
    if ( wBufferSize > hCI->IOBuffer_Host2CI.wBufferSize )
    {
        DRV_CI_DEBUG("ERROR: CI TX Packet too big\r\n");
    }

    gu8CI_Command |= CICOMMAND_HOSTCONTROL;                                                     // UP HC
    MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
    while ( !( MDrv_CI_L2_WaitForCIBit( hPCMCIA, CISTATUS_FREE ) ) && ( MDrv_CI_PCMCIA_IsModuleStillPlugged() ) && ( u16TryLoop < 10 ) )
    {
        gu8CI_Command &= ( ~CICOMMAND_HOSTCONTROL );                                            // DOWN HC
        MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
        msleep(1);
        gu8CI_Command |= CICOMMAND_HOSTCONTROL;                                                 // UP HC
        MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, CICOMMAND_HOSTCONTROL );
        u16TryLoop++;
    }
    
    if ( 10 == u16TryLoop )
    {
        DRV_CI_DEBUG("CI MDrv_CI_L2_ReallyWriteData: not \"free\"\r\n");
        return FALSE;
    }

    MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_SIZELOW, ( U8 )( wBufferSize ) );
    MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_SIZEHIGH, ( U8 )( wBufferSize >> 8 ) );

    for ( i = 0; i < wBufferSize; i++ )
    {
        MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_DATA, pData[i] );
    }
	
    /* It is reset to '0' by the host after the data transfer. */
    gu8CI_Command &= ( ~CICOMMAND_HOSTCONTROL );                                                // DOWN HC
    MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );

    return TRUE;
}

// ----------------------------------------------
B16 MDrv_CI_L2_WaitForCIBit(PCMCIA_HANDLE hCI, U8 BitMask)
{
    U16 i;

    for (i = 0; i < 100; i++)
    {
        // Do not poll for the presence of a CI module here
        // this causes too high I2C load or bus traffic !
        if (MDrv_CI_PCMCIA_ReadIOMem(hCI, CI_PHYS_REG_COMMANDSTATUS) & BitMask)
        {
            return TRUE;
        }
        msleep(50);
        
        if (!MDrv_CI_PCMCIA_IsModuleStillPlugged())
            return FALSE;
    }

    DRV_CI_DEBUG("ERROR: Timeout waiting for CI Status/Ready Bit\r\n");
    return FALSE;
}

void MDrv_CI_L2_WriteCOR(PCMCIA_HANDLE hPCMCIA, PCMCIA_INFO_t *pInfo, U8 bSlotNum)
{
    U32 i;
    CI_HANDLE hCI = &gCiSlot[bSlotNum];
    
    hCI->hSlotHandle = hPCMCIA;
    for ( i = 0; i< pInfo->bNumConfigs; i++ )
    {
        if ( pInfo->Config[i].fCITagsPresent == 0x03 )
        {
            MDrv_CI_PCMCIA_WriteAttribMem( hPCMCIA, ( U16 )( pInfo->ConfigOffset ), pInfo->Config[i].bConfigIndex );//switch to i/o mode
            memset( hCI->IOBuffer_Host2CI.CI_T_SENDQ_Size, 0x00, sizeof( hCI->IOBuffer_Host2CI.CI_T_SENDQ_Size ) );
		}
	}
	msleep(1000);
}

B16 MDrv_CI_L2_CheckConfig(PCMCIA_HANDLE hPCMCIA, PCMCIA_INFO_t *pInfo, U8 bSlotNum)
{
    U32 i, u32DataLen;
    CI_HANDLE hCI = &gCiSlot[bSlotNum];

    gu8CI_Command = 0x00;      // Reset CI Command
    hCI->hSlotHandle = hPCMCIA;
    for ( i = 0; i< pInfo->bNumConfigs; i++ )
    {
        if ( pInfo->Config[i].fCITagsPresent == 0x03 )
        {
            hCI->TPDULen = 0;

            // Note1: Some modules do not like an initial reset:
            // like Cryptoworks and Black Viaccess
            // Note2: Do not fail the initialization because of
            // failed Ready-Bits here ! CONAX DOES NOT SET THESE BITS !

            // write size read
            DRV_CI_DEBUG("Write Size Read\r\n");
            gu8CI_Command |= CICOMMAND_SIZEREAD;                                                // Up SR
            MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
            if ( !MDrv_CI_L2_WaitForCIBit( hPCMCIA, CISTATUS_DATAAVAILABLE ) )//if DA is down
            {
                gu8CI_Command &= ( ~CICOMMAND_SIZEREAD );                                       // DOWN SR
                MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
                // write reset
                DRV_CI_DEBUG("Write Reset\r\n");
                gu8CI_Command |= CICOMMAND_RESET;                                                // Up RS
                MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
                MDrv_CI_L2_WaitForCIBit( hPCMCIA, CISTATUS_FREE );
                gu8CI_Command &= ( ~CICOMMAND_RESET );                                           // DOWN RS
                MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
                // write size read
                DRV_CI_DEBUG("Write Size Read\r\n");
                gu8CI_Command |= CICOMMAND_SIZEREAD;                                             // Up SR
                MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
                if ( !MDrv_CI_L2_WaitForCIBit( hPCMCIA, CISTATUS_DATAAVAILABLE ) )
                {
                    DRV_CI_DEBUG("CI Init hangs\r\n");
                    return FALSE;
                }
            }

            // Dump Buffersize
            // read the size (this is always 0002, byteswapped)
            u32DataLen = ( U16 )MDrv_CI_PCMCIA_ReadIOMem( hPCMCIA, CI_PHYS_REG_SIZEHIGH ) << 8 | ( U16 )MDrv_CI_PCMCIA_ReadIOMem( hPCMCIA, CI_PHYS_REG_SIZELOW );
            DRV_CI_DEBUG("Datalen %04X \r\n", u32DataLen);

            // if the module returned an invalid data size, initiate a reset
            if ( u32DataLen != 0x0002 )
            {
                DRV_CI_DEBUG("CI Module returned an invalid buffer size(1). Initiating a reset.\r\n");
                //wgkwak in LGE
                MDrv_CI_PCMCIA_HwRstPCMCIA(PCMCIA_RESETDELAY_MAX);
                return FALSE;
            }

            hCI->IOBuffer_Host2CI.wBufferSize = ( U16 )MDrv_CI_PCMCIA_ReadIOMem( hPCMCIA, CI_PHYS_REG_DATA ) << 8;
            hCI->IOBuffer_Host2CI.wBufferSize |= ( U16 )MDrv_CI_PCMCIA_ReadIOMem( hPCMCIA, CI_PHYS_REG_DATA );
            DRV_CI_DEBUG("Buflen %02X \r\n", hCI->IOBuffer_Host2CI.wBufferSize);
            
            gu8CI_Command &= ( ~CICOMMAND_SIZEREAD );                                           // DOWN SR
            MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
            
            /* EN 50221:P.60:
                Modules shall support a minimum buffer size of 16 bytes.
            */
            if ( hCI->IOBuffer_Host2CI.wBufferSize < 0x10 )
            {
                // if the module returned an invalid buffer size, initiate a reset
                DRV_CI_DEBUG("CI Module returned an invalid buffer size(2). Initiating a reset.\r\n");
                //wgkwak in LGE
                MDrv_CI_PCMCIA_HwRstPCMCIA(PCMCIA_RESETDELAY_MAX);
                return FALSE;
            }

            // select min(4096, what the module offers)
            if ( hCI->IOBuffer_Host2CI.wBufferSize > CI_T_SENDQBUFSIZE )
            {
                hCI->IOBuffer_Host2CI.wBufferSize = CI_T_SENDQBUFSIZE;
            }

            // Delay for NP4 Chipset
            msleep(10);

            DRV_CI_DEBUG("WriteBuffersize %04X\r\n", hCI->IOBuffer_Host2CI.wBufferSize);
            _MDrv_CI_L2_WriteBufferSize( hPCMCIA, hCI->IOBuffer_Host2CI.wBufferSize );

#if PCMCIA_IRQ_ENABLE
            if ( ENABLE == pInfo->bINT )
            {
                MDrv_PCMCIA_Enable_Interrupt( ENABLE );
                DRV_CI_DEBUG( "Up DAIE!!!\n" );
                gu8CI_Command |= CICOMMAND_DAIE ;                             // Up DAIE
                //gu8CI_Command |= CICOMMAND_FRIE;                              // Up FRIE
                MDrv_CI_PCMCIA_WriteIOMem( hPCMCIA, CI_PHYS_REG_COMMANDSTATUS, gu8CI_Command );
            }
#endif
            
            return TRUE;
        }
    }
    return FALSE;
}
