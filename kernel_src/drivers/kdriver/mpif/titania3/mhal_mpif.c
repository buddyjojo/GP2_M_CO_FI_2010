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
/// file    mhal_mpif.c
/// @brief  MPIF HAL layer Driver
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include "mdrv_types.h"
#include "mdrv_mpif_st.h"
#include "mhal_mpif.h"


//-------------------------------------------------------------------------------------------------
//  Definition
//-------------------------------------------------------------------------------------------------
#define WAIT_TIMEOUT		  (MASTER_CLOCK_HZ>>12)//0xFFFFFFFF	

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//-------------------------------------------------------------------------
//Please set clock, trun around cycle, commend data with,
//slave data width first to meet SPIF status
//--------------------------------------------------------------------------
BOOL MHal_MPIF_Init(U8 u8clk, U8 u8trc, U8 u8wc)
{
    REG_WM(REG_CHIP_ALLPAD_IN, 0, BIT15);       //Disable All Pad in
    REG_WM(REG_CKGEN0_CKG_MPIF, u8clk, 0x3F);   //set clock XTAL

    REG_WR(MPIF_INTERRUPT_EVENT_MASK, 0xFFFF); //enable all interrupt
    REG_WR(MPIF_INTERRUPT_EVENT_STATUS, 0xFFFF); //clear interrupt status

    MHal_MPIF_SetTrcWc(u8trc, u8wc);

    REG_WM(MPIF_MISC1,    0, BIT0); //sw reset
    REG_WM(MPIF_MISC1, BIT0, BIT0);

	REG_WM(MPIF_MIU_WPROTECT_CONTROL, 0x0C, 0x0C); //enable dummy write

    printk("MPIF init done!\n");

    return TRUE;
}

// Wait the channel is free via checking the channel busy status
BOOL MHal_MPIF_BusyWait_ChannelFree(U8 event_bit, U32 timeout)
{
    BOOL bRet = FALSE;
    U32 i;

    // Use a limit value to prevent infinite loop
    for (i=0; i < timeout; i++)
    {
        if ((REG_RR(MPIF_CHANNEL_BUSY_STATUS) & event_bit) == 0x00)
        {
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}

// Wait the interrupt event
static BOOL MHal_MPIF_BusyWait_InterruptEvent(U16 event_bit, U32 timeout)
{
    BOOL bRet = FALSE;
    U32 i;

    // Use a limit value to prevent infinite loop
    for (i=0; i < timeout; i++)
    {
        if ((REG_RR(MPIF_INTERRUPT_EVENT_STATUS) & event_bit ) == event_bit)
        {
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}

#if 0   // UNUSED
// Wait the interrupt event to be cleared
static BOOL MHal_MPIF_BusyWait_InterruptClear(U16 event_bit, U32 timeout)
{
    BOOL bRet = FALSE;
    U32 i;

    // Use a limit value to prevent infinite loop
    for (i=0; i < timeout; i++)
    {
        if ( ( REG_RR(MPIF_INTERRUPT_EVENT_STATUS) & event_bit ) == 0 )
        {
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}
#endif

// Wait the interrupt or error event
// return MPIF_EVENT_TX_DONE if transmission done, or MPIF_EVENT_TX_ERROR if transmission error
// otherwise, return MPIF_EVENT_NULL
static U8 MHal_MPIF_BusyWait_InterruptErrorEvent(U16 event_bit, U16 error_bit, U32 timeout)
{
    U8 u8Ret = MPIF_EVENT_NULL;
    U32 i = 0;
    U16 u16temp;

    // Use a limit value to prevent infinite loop
    do
    {
        u16temp = REG_RR(MPIF_INTERRUPT_EVENT_STATUS);

        // Check if TX done or TX error
        if((u16temp & error_bit ) == error_bit )
        {
            u8Ret = MPIF_EVENT_TX_ERROR;
            break;
        }
        if((u16temp & MPIF_INTERRUPT_EVENT_STATUS_BIT_BUSYTIMEOUT ) == MPIF_INTERRUPT_EVENT_STATUS_BIT_BUSYTIMEOUT)
        {
            u8Ret = MPIF_EVENT_BUSYTIMEOUT;
            break;
        }
        if((u16temp & event_bit)== event_bit)
        {
            u8Ret = MPIF_EVENT_TX_DONE;
            break;
        }
        i++;
    } while(i < timeout);

    if(u8Ret != MPIF_EVENT_TX_DONE)
        MPIF_DEBUGINFO(printk("interrupt_status=0x%x\n", u16temp));

    REG_WR(MPIF_INTERRUPT_EVENT_STATUS, u16temp); //clear status

    return u8Ret;
}

// Write data to the specific SPIF register via LC1A
BOOL MHal_MPIF_LC1A_RW(U8 u8bWite, U8 slaveid, U8 index, U8 *pu8data)
{
	U16 u16data;   	

    // Waiting for the channel being free
    if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC1ATX, WAIT_TIMEOUT))
    {
        MPIF_DEBUGINFO(printk("1A Write Waiting for channel timed out.\n"));
        return FALSE;
    }

	u16data = ((U16)(index  & 0x07) << 4) | ((U16)(slaveid & 0x03) << 2) | ((U16)(u8bWite & 0x01) << 1) | 0x01; //fired          

    // Set LC1A settings
    if(u8bWite > 0)
    {
   		u16data |= ((U16)(*pu8data & 0xFF) << 8);
    }

	REG_WR(MPIF_LC1A_SETTINGS,u16data);	   

    // Wait for LC1A write done
    if(!MHal_MPIF_BusyWait_InterruptEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC1ATX, WAIT_TIMEOUT))
    {
        MPIF_DEBUGINFO( printk("1A Write Waiting for TX interrupt timed out.\n"));
        return FALSE;
    }

    // Retrieve the returned data
    if(u8bWite == 0)
        *pu8data = (U8)(REG_RR(MPIF_LC1A_SETTINGS) >> 8);

    // Clear the interrupt
    REG_WR(MPIF_INTERRUPT_EVENT_STATUS, REG_RR(MPIF_INTERRUPT_EVENT_STATUS));

    return TRUE;
}

// Write data to the specific SPIF register via LC2A
BOOL MHal_MPIF_LC2A_RW(U8 u8bWite, U8 slaveid, U8 checksum, U8 rtx, U16 wordaddr, U16 *pu16data)
{
	U16 u16data;
    U8 event_id = MPIF_EVENT_NULL;

    // Waiting for the channel being free
    if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC2ATX, WAIT_TIMEOUT))
    {
        MPIF_DEBUGINFO(printk("2A Write Waiting for channel timed out.\n"));
        return FALSE;
    }

    // Set the register address
    REG_WR(MPIF_LC2A_REG_ADDRESS, wordaddr);

    // Put the data into the sending buffer
    if(u8bWite > 0)
        REG_WR(MPIF_LC2A_REG_DATA, *pu16data);

    // Fire the read command, write control register
    u16data = ((U16)(rtx & 0x03) << 6) | ((U16)(checksum & 0x01)<< 4) | ((U16)(slaveid & 0x03) << 2) |
    			((U16)(u8bWite & 0x01) << 1) | 0x01; //fired 
	REG_WM(MPIF_LC2A_REG_CONTROL,u16data, 0xFF);	

    // Wait for LC2A write done
    event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2ATX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2ATXER, WAIT_TIMEOUT);
    if (event_id != MPIF_EVENT_TX_DONE)
        return FALSE;

    // Retrieve the returned data
    if(u8bWite == 0)
        *pu16data = REG_RR(MPIF_LC2A_REG_DATA);

    return TRUE;
}

BOOL MHal_MPIF_LC2B_RW(U8 u8bWite, U8 slaveid, U8 checksum, U8 rtx, U16 wordaddr, U16 *pu16data)
{
	U16 u16data;
    U8 event_id = MPIF_EVENT_NULL;

    // Waiting for the channel being free
    if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC2BTX, WAIT_TIMEOUT))
    {
        MPIF_DEBUGINFO(printk("2b read Waiting for channel timed out.\n"));
        return FALSE;
    }

    // Set the register address
    REG_WR(MPIF_LC2B_REG_ADDRESS, wordaddr);
	
    if(u8bWite > 0)
        REG_WR(MPIF_LC2B_REG_DATA, *pu16data);

	u16data = ((U16)(rtx & 0x03) << 6) | ((U16)(checksum & 0x01)<< 4) | ((U16)(slaveid & 0x03) << 2) |
    			((U16)(u8bWite & 0x01) << 1) | 0x01; //fired   
	REG_WM(MPIF_LC2B_REG_CONTROL,u16data, 0xFF);

    // Wait for LC2B read done
    event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2BTX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2BTXER, WAIT_TIMEOUT);
    if (event_id != MPIF_EVENT_TX_DONE)
        return FALSE;

    // Retrieve the returned data
    if(u8bWite == 0)
        *pu16data = REG_RR(MPIF_LC2B_REG_DATA);

    return TRUE;
}

// Read data from the SPIF to the RIU via LC3A
BOOL MHal_MPIF_LC3A_RIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U8 *pu8data, U16 n)
{
    U16 u16data;
    U8 event_id = MPIF_EVENT_NULL;
	U8 u8i;	
	
    // Set data length
    REG_WR(MPIF_LC3A_PACKET_LENGTH, 1); //unit is 16 bytes

	// Put the data into the sending buffer    
	if(u8bWite > 0)
	{
    	for(u8i=0; u8i < (U8)n; u8i+=2)    	
    	{
    		u16data = (U16)(pu8data[u8i] & 0xFF);			
			u16data += ((U16)(pu8data[u8i+1] & 0xFF) << 8);			
    		REG_WR((MPIF_LC3A_PACKET_DATA + u8i), u16data);
    		
    	}     		
	} 
    
    u16data = ((U16)(u8bWite & 0x01) << 1) | ((U16)(slaveid & 0x03) << 2) | ((U16)(checksum & 0x01) << 4) |
			((U16)(rtx_idc & 0x01) << 5) | ((U16)(rtx & 0x03) << 6) | ((U16)(fastmode & 0x01) << 10) |
            ((U16)(wcnt & 0x0F) << 12) | 0x01; //fired
	REG_WR(MPIF_LC3A_PACKET_CONTROL, u16data);	 
	
    // Wait for LC3A read done
    event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3ATX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3ATXER, WAIT_TIMEOUT);
    if (event_id != MPIF_EVENT_TX_DONE)
    {
    	printk("3A RIU set spif control failed\n");
        return FALSE;
    }

    // Put the data into the receiving buffer
    if(u8bWite > 0)
        return TRUE;

    for (u8i=0; u8i<(U8)n; u8i+=2)
    {
        u16data = REG_RR(MPIF_LC3A_PACKET_DATA + u8i);
		pu8data[u8i] = (U8)(u16data & 0xFF);
		pu8data[u8i+1] = (U8)((u16data >> 8) & 0xFF);
    }

    return TRUE;

}// Read data from the SPIF to the RIU via LC3A

// Read data from the SPIF to the RIU via LC3B
BOOL MHal_MPIF_LC3B_RIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U8 *pu8data, U16 n)
{
    U16 u16data;
    U8 event_id = MPIF_EVENT_NULL;
	U8 u8i;    		

    // Set data length
    REG_WR(MPIF_LC3B_PACKET_LENGTH, 1); //unit is 16 bytes

	// Put the data into the sending buffer    
	if(u8bWite > 0)
	{
    	for(u8i=0; u8i < (U8)n; u8i+=2)    	
    	{
    		u16data = (U16)(pu8data[u8i] & 0xFF);			
			u16data += ((U16)(pu8data[u8i+1] & 0xFF) << 8);			
    		REG_WR((MPIF_LC3B_PACKET_DATA + u8i), u16data);    		
    	}      		
	} 
    
    u16data = ((U16)(u8bWite & 0x01) << 1) | ((U16)(slaveid & 0x03) << 2) | ((U16)(checksum & 0x01) << 4) |
			((U16)(rtx_idc & 0x01) << 5) | ((U16)(rtx & 0x03) << 6) | ((U16)(fastmode & 0x01) << 10) |
            ((U16)(wcnt & 0x0F) << 12) | 0x01; //fired
	REG_WR(MPIF_LC3B_PACKET_CONTROL, u16data);	 
	
    // Wait for LC3A read done
    event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3BTX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3BTXER, WAIT_TIMEOUT);
    if (event_id != MPIF_EVENT_TX_DONE)
        return FALSE;

    // Put the data into the receiving buffer
    if(u8bWite > 0)
        return TRUE;

    for (u8i=0; u8i<(U8)n; u8i+=2)
    {
        u16data = REG_RR(MPIF_LC3B_PACKET_DATA + u8i);
		pu8data[u8i] = (U8)(u16data & 0xFF);
		pu8data[u8i+1] = (U8)((u16data >> 8) & 0xFF);
    }

    return TRUE;

}// Read data from the SPIF to the RIU via LC3B

BOOL MHal_MPIF_LC3A_MIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U32 miu_addr, U32 spif_mdr, U32 n)	
{
	U16 u16data;
	U8 event_id = MPIF_EVENT_NULL;	
	
	// Set the MIU address as the source address
	u16data = (U16)((miu_addr >> 3) & 0xFFFF);
	REG_WR(MPIF_LC3A_PACKET_MIU_BASE_ADDRESS_L, u16data);
	u16data = (U16)((miu_addr >> 19) & 0x1FF);
	REG_WR(MPIF_LC3A_PACKET_MIU_BASE_ADDRESS_H, u16data);

	// Set data length
	REG_WR(MPIF_LC3A_PACKET_LENGTH, (U16)(n >> 4));

	u16data = ((U16)(slaveid & 0x03) << 2) | ((U16)(checksum & 0x01) << 4) | ((U16)(rtx_idc & 0x01) << 5) | ((U16)(rtx & 0x03) << 6) |
				((U16)(u8bWite & 0x01) << 1) | ((U16)(fastmode & 0x01) << 10) |
                ((U16)(wcnt & 0x0F) << 12) | ((U16)(MPIF_MIUID_0) << 9) | 0x100 | 0x01; //fired

	REG_WR(MPIF_LC3A_PACKET_CONTROL, u16data);	 
	
	// Wait for LC3B write done	
	event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3ATX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3ATXER,
				0xFFFFFFFF/*WAIT_TIMEOUT*/);
	if (event_id != MPIF_EVENT_TX_DONE)
		return FALSE;

	return TRUE;
}// Write data from the MIU to the SPIF via LC3B

BOOL MHal_MPIF_LC3B_MIURW(U8 u8bWite, U8 slaveid, U8 checksum, U8 fastmode, U8 rtx_idc, U8 rtx, U8 wcnt, U32 miu_addr, U32 spif_mdr, U32 n)	
{
	U16 u16data;
	U8 event_id = MPIF_EVENT_NULL;	
	
	// Set the MIU address as the source address
	u16data = (U16)((miu_addr >> 3) & 0xFFFF);
	REG_WR(MPIF_LC3B_PACKET_MIU_BASE_ADDRESS_L, u16data);
	u16data = (U16)((miu_addr >> 19) & 0x1FF);
	REG_WR(MPIF_LC3B_PACKET_MIU_BASE_ADDRESS_H, u16data);

	// Set data length
	REG_WR(MPIF_LC3B_PACKET_LENGTH, (U16)(n >> 4));

	u16data = ((U16)(slaveid & 0x03) << 2) | ((U16)(checksum & 0x01) << 4) | ((U16)(rtx_idc & 0x01) << 5) | ((U16)(rtx & 0x03) << 6) |
				((U16)(u8bWite & 0x01) << 1) | ((U16)(fastmode & 0x01) << 10) |
                ((U16)(wcnt & 0x0F) << 12) | ((U16)(MPIF_MIUID_0) << 9) | 0x100 | 0x01; //fired

	REG_WR(MPIF_LC3B_PACKET_CONTROL, u16data);	 

	// Wait for LC3B write done
	event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3BTX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3BTXER,
				0xFFFFFFFF/*WAIT_TIMEOUT*/);
	if (event_id != MPIF_EVENT_TX_DONE)
		return FALSE;

	return TRUE;
}// Write data from the MIU to the SPIF via LC3B

BOOL MHal_MPIF_LC4A_MIURW(U8 u8bWite, U8 slaveid, U8 rtx, U8 miu_id, U8 bkpt, U8 wcnt, U32 miu_addr, U32 spif_mdr, U16 n)
{
    U16 u16data;
    U8 event_id = MPIF_EVENT_NULL;    
	U32 u32data;

    // Set the MIU address as the source address
    u16data = (miu_addr >> 3) & 0xFFFF;
    REG_WR(MPIF_LC4A_STREAM_MIU_BASE_ADDRESS_L, u16data);
    u16data = (miu_addr >> 19) & 0x1FF;
    REG_WR(MPIF_LC4A_STREAM_MIU_BASE_ADDRESS_H, u16data);

    // Set data length
    REG_WR(MPIF_LC4A_STREAM_LENGTH, n);

    // Fire the read command, write control register
    u16data = ((slaveid & 0x03) << 2) | ((rtx & 0x03) << 6) | ((u8bWite & 0x01) << 1) | 
    		((miu_id & 0x01) << 9) | ((bkpt & 0x03) << 10) | ((wcnt & 0x0F) << 12) | 0x01; //fired
    REG_WR(MPIF_LC4A_STREAM_CONTROL, u16data);
   
    // Wait for LC4A read done
    for(u32data = 0; u32data < 1000; u32data++)
    {
        event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC4ATX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC4ATXER, WAIT_TIMEOUT);
        if(event_id == MPIF_EVENT_TX_DONE)
            break;
        printk("LC4A wait event %x\n", event_id);
    }
    if (event_id != MPIF_EVENT_TX_DONE)
        return FALSE;
	
    return TRUE;
}

BOOL MHal_MPIF_SetCmdDataMode(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth)
{
    U16 u16data;

    u16data = REG_RR(MPIF_MISC1);
    switch(u8slaveid & 0x03)
    {
        case MPIF_SLAVE_ID_0:
            u16data &= ~0xC0;
            u16data |= ((U16)u8datawidth << 6);
            break;
        case MPIF_SLAVE_ID_1:
            u16data &= ~0x300;
            u16data |= ((U16)u8datawidth << 8);
            break;
        case MPIF_SLAVE_ID_2:
            u16data &= ~0xC00;
            u16data |= ((U16)u8datawidth << 10);
            break;
        case MPIF_SLAVE_ID_3:
            u16data &= ~0x3000;
            u16data |= ((U16)u8datawidth << 12);
            break;
    }
    u16data &= ~0xC000;
    u16data |= ((U16)u8cmdwidth << 14);
    REG_WR(MPIF_MISC1, u16data);

    return TRUE;
}

BOOL MHal_MPIF_SetTrcWc(U8 u8trc, U8 u8wc)
{
    U16 u16misc1, u16misc2;

    u16misc1 = REG_RR(MPIF_MISC1);
    u16misc1 &= ~0x3C;
    u16misc1 |= ((u8trc & 0x03) << 2);
    u16misc1 |= ((u8wc & 0x03) << 4);
    REG_WR(MPIF_MISC1, u16misc1);

    u16misc2 = REG_RR(MPIF_MISC2);
    u16misc2 &= ~0x0F;
    u16misc2 |= (u8trc & 0x07);
    u16misc2 |= (u8trc & 0x04) << 3;
    REG_WR(MPIF_MISC2, u16misc2);

    return TRUE;
}

BOOL MHal_MPIF_GetSPIF_ChBusySts(U8 u8slaveid, U16 u16event_bit, U32 u32timeout)
{
    U16 u16data;
    U32 u32i;
	BOOL bRes = FALSE;

    // Use a limit value to prevent infinite loop
    for (u32i=0; u32i < u32timeout; u32i++)
    {
        if(!MHal_MPIF_LC2A_RW(0, u8slaveid, 1, 0, REG_SPIF_CHANNEL_BUSY_STATUS, &u16data))
            break;
        if((u16data & u16event_bit) == 0x00)        
            return TRUE;        
    }

	MPIF_DEBUGINFO(printk("SPIF channel status busy.\n"));

    return bRes;
}

BOOL MHal_MPIF_SetSlave_ClkInv_Delay(U8 u8slaveid, U8 u8DlyBufNum)
{	
	MS_U16 u16data;

	if(!MHal_MPIF_LC2A_RW(0, u8slaveid, 1, 0, REG_SPIF_MISC2, &u16data))
        return FALSE;

	u16data &= ~0x1F00;	
	u16data |= (((MS_U16)(u8DlyBufNum & 0x0F) << 8) | 0x1000);	
	if(!MHal_MPIF_LC2A_RW(1, u8slaveid, 1, 0, REG_SPIF_MISC2, &u16data))
        return FALSE;

    return TRUE;
}

