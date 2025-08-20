////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mhal_mpif.c
/// @brief  MPIF HAL layer Driver
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
//#include <linux/timer.h>
//#include <linux/jiffies.h>
#include "mdrv_types.h"
#include "mdrv_mpif_st.h"
#include "mhal_mpif.h"
#include "mdrv_mpif_io_uboot.h"

extern void	printf(const char *fmt, ...);
#define printk printf


//-------------------------------------------------------------------------------------------------
//  Definition
//-------------------------------------------------------------------------------------------------

#define MAKE_16BIT_ADDR(x)    ((x)>>1)

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
    REG_WR(MPIF_INTERRUPT_EVENT_STATUS, 0x3FFF); //clear interrupt status

    MHal_MPIF_SetTrcWc(u8trc, u8wc);

    REG_WM(MPIF_MISC1,    0, BIT0); //sw reset
    REG_WM(MPIF_MISC1, BIT0, BIT0);

    //printk("MPIF init done!\n");

    return TRUE;
}

// Wait the channel is free via checking the channel busy status
static BOOL MHal_MPIF_BusyWait_ChannelFree(U8 event_bit, U32 timeout)
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
    index = MAKE_16BIT_ADDR(index);

    // Waiting for the channel being free
    if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC1ATX, MASTER_CLOCK_HZ>>12))
    {
        MPIF_DEBUGINFO(printk("1A Write Waiting for channel timed out.\n"));
        return FALSE;
    }

    // Set LC1A settings
    if(u8bWite > 0)
    {
        REG_WR(MPIF_LC1A_SETTINGS,
              ((*pu8data << 8)        |
              ((index  & 0x07) << 4)  |
              ((slaveid & 0x03) << 2) |
              ((u8bWite & 0x01) << 1) |
              0x01)); //fired;
    }
    else
    {
        REG_WR(MPIF_LC1A_SETTINGS,
              (((index  & 0x07) << 4) |
              ((slaveid & 0x03) << 2) |
              ((u8bWite & 0x01) << 1) |
              0x01)); //fired;
    }

    // Wait for LC1A write done
    if(!MHal_MPIF_BusyWait_InterruptEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC1ATX, MASTER_CLOCK_HZ>>12))
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
BOOL MHal_MPIF_LC2A_RW(U8 u8bWite, U8 slaveid, U8 checksum, U8 rtx, U16 addr, U16 *pu16data)
{
    U8 event_id = MPIF_EVENT_NULL;
    addr = MAKE_16BIT_ADDR(addr);

    // Waiting for the channel being free
    if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC2ATX, MASTER_CLOCK_HZ>>12))
    {
        MPIF_DEBUGINFO(printk("2A Write Waiting for channel timed out.\n"));
        return FALSE;
    }

    // Set the register address
    REG_WR(MPIF_LC2A_REG_ADDRESS, addr);

    // Put the data into the sending buffer
    if(u8bWite > 0)
        REG_WR(MPIF_LC2A_REG_DATA, *pu16data);

    // Fire the read command, write control register
    REG_WM(MPIF_LC2A_REG_CONTROL,
          (((rtx & 0x03) << 6) |
          ((checksum & 0x01)<< 4) |
          ((slaveid & 0x03) << 2) |
          ((u8bWite & 0x01) << 1) |
          0x01),
          0xFF); //fired

    // Wait for LC2A write done
    event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2ATX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2ATXER, MASTER_CLOCK_HZ>>12);
    if (event_id != MPIF_EVENT_TX_DONE)
        return FALSE;

    // Retrieve the returned data
    if(u8bWite == 0)
        *pu16data = REG_RR(MPIF_LC2A_REG_DATA);

    return TRUE;
}

BOOL MHal_MPIF_LC2B_RW(U8 u8bWite, U8 slaveid, U8 checksum, U8 rtx, U16 addr, U16 *pu16data)
{
    U8 event_id = MPIF_EVENT_NULL;
    addr = MAKE_16BIT_ADDR(addr);

    // Waiting for the channel being free
    if(!MHal_MPIF_BusyWait_ChannelFree(MPIF_CHANNEL_BUSY_STATUS_BIT_LC2BTX, MASTER_CLOCK_HZ>>12))
    {
        MPIF_DEBUGINFO(printk("2b read Waiting for channel timed out.\n"));
        return FALSE;
    }

    // Set the register address
    REG_WR(MPIF_LC2B_REG_ADDRESS, addr);
    if(u8bWite > 0)
        REG_WR(MPIF_LC2B_REG_DATA, *pu16data);

    REG_WM(MPIF_LC2B_REG_CONTROL,
          (((rtx & 0x03) << 6) |
          ((checksum & 0x01) << 4) |
          ((slaveid & 0x03) << 2) |
          ((u8bWite & 0x01) << 1) |
          0x01),
          0xFF); //fired

    // Wait for LC2B read done
    event_id = MHal_MPIF_BusyWait_InterruptErrorEvent(MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2BTX, MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2BTXER, MASTER_CLOCK_HZ>>12);
    if (event_id != MPIF_EVENT_TX_DONE)
        return FALSE;

    // Retrieve the returned data
    if(u8bWite == 0)
        *pu16data = REG_RR(MPIF_LC2B_REG_DATA);

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

