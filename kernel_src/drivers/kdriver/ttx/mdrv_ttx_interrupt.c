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
#define MDRV_TTX_INTERRUPT_C
///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_ttx_interrupt.c
/// This file contains the Mstar driver interrupt for Teletext
/// @author MStar Semiconductor Inc.
/// @brief  Teletext module
///////////////////////////////////////////////////////////////////////////////


#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/wait.h>

#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

#include "mhal_ttx_types.h"
#include "mhal_ttx_reg.h"
#include "mhal_ttx.h"
#include "mdrv_ttx.h"
#include "mdrv_ttx_st.h"


//#define TTX_PACKET_COUNT_MAX    65535
#define TTX_TIMEOUT    ( HZ/5 )       // set 0.2 sec to TTX timeout

static U32 _u32TimerTTX = 0;
static U32 _u32TimerTTX_last = 0;

U32 _u32PrevTTXPacketCount = (U32)(-1);

U32 gu32DataRateTTX = 0;

void _MDrv_VPS_BH(unsigned long unused)
{
    volatile struct VBI_PARAMETERS *ptVP;
    ptVP = _tpVBIParametersTTX;

    MHal_TTX_GatherVpsData(_pu8VPS_TTX);
    ptVP->_u32Status |= (E_VPS);
}

void _MDrv_WSS_BH(unsigned long unused)
{
    volatile  struct  VBI_PARAMETERS *ptVP;
    ptVP = _tpVBIParametersTTX;

    ptVP->_u32WSS = MHal_TTX_ReadWSS();
    ptVP->_u32Status |= (E_WSS);
}


void MDrv_TTX_BH(unsigned long unused)
{
    volatile struct  VBI_PARAMETERS *ptVP;
    U32             u32CurTTXPacketCount;
    U32             u32PacketComing;
    U32             u32FullDet;

    static U32 u32PacketTotal = 0;
    static U32 u32InterruptCounter = 0;

    ptVP = _tpVBIParametersTTX;

    u32CurTTXPacketCount = MHal_TTX_TXX_Count();

    /* 0 is initial vaule, correct value is from 1 to TT_VBI_BUFFER_UNIT */
    if (u32CurTTXPacketCount == 0)
        return;

    /* obtain coming packet number. */
    if(_u32PrevTTXPacketCount == (U32)(-1))
    {
        _u32PrevTTXPacketCount = u32CurTTXPacketCount;
        u32PacketComing = u32CurTTXPacketCount;
    }
    else
    {
        if (u32CurTTXPacketCount >= _u32PrevTTXPacketCount)
            u32PacketComing = u32CurTTXPacketCount - _u32PrevTTXPacketCount;
        else
            u32PacketComing = u32CurTTXPacketCount + TT_VBI_BUFFER_UNIT - _u32PrevTTXPacketCount;

        _u32PrevTTXPacketCount = u32CurTTXPacketCount;
    }

    /* if coming packet number is equ to zero, system quits interrupt. */
    if(u32PacketComing == 0)
    {
        return;
    }

    // reset put & get index -- start
    if (u32PacketComing > 88)  // 5 field     17+18+17+18+(17 or 18) = 87 or 88 packets
    {
        ptVP->_u32GetAdr = 0;
        ptVP->_u32PutAdr = 0;
        //TTX_DRV_DBG("########## Reset Buffer Index ##########\n");
    }
    // reset put & get index --end

    // update TTX data rate  -- start
    u32PacketTotal += u32PacketComing;
    u32InterruptCounter += 1;

    if (u32InterruptCounter == 50)
    {
        gu32DataRateTTX = u32PacketTotal;
        u32PacketTotal = 0;
        u32InterruptCounter = 0;
    }
    // update TTX data rate  -- end

    /*
        ------------------------------------------
            ^                                ^
            |                                |
            G                                P -->

        if (P>=G)
            P-G+Coming Pcket number >= VBI size --> overflow.

    */

    if(ptVP->_u32PutAdr >= ptVP->_u32GetAdr)
    {
        u32FullDet = (ptVP->_u32PutAdr - ptVP->_u32GetAdr);
        if(u32FullDet + u32PacketComing >= TT_VBI_BUFFER_UNIT)
        {
            ptVP->_u32Overflow = 1;
            //TTX_DRV_DBG("########## TTX BUFFER OVERFLOW ##########\n");
        }
    }
    else
    {

    /*
        ------------------------------------------
            ^                                ^
            |                                |
            P -->                            G

        if (P<G)
            G-P <= Coming Pcket number --> overflow.

    */
        u32FullDet = (ptVP->_u32GetAdr - ptVP->_u32PutAdr);
        if(u32FullDet <= u32PacketComing)
        {
            ptVP->_u32Overflow = 1;
            //TTX_DRV_DBG("########## TTX BUFFER OVERFLOW ##########\n");
        }

    }

    //if(u32CurTTXPacketCount >= ptVP->_u32PacketUnit)
    //{
    //    u32CurTTXPacketCount = ptVP->_u32PacketUnit-1;
    //}
    ptVP->_u32PutAdr = u32CurTTXPacketCount - 1;
    ptVP->_u32Status |= E_TTX;

}

void MDrv_TTX_WQ(struct work_struct *unused);
DECLARE_DELAYED_WORK(TTXWorkQueue, MDrv_TTX_WQ);

void MDrv_TTX_WQ(struct work_struct * unused)
{
    //TTX_DRV_DBG("########## MDrv_TTX_WQ is called ##########\n");
    volatile struct VBI_PARAMETERS *ptVP;
    ptVP = _tpVBIParametersTTX;

    if ((jiffies - _u32TimerTTX) > TTX_TIMEOUT)
    {
        //TTX_DRV_DBG("TTX Interrupt no longer happened\n");
        ptVP->_u32Status &= ~(E_TTX);
        gu32DataRateTTX = 0;
        _u32PrevTTXPacketCount = (U32)(-1);
        ptVP->_u32GetAdr = 0;
        ptVP->_u32PutAdr = 0;
        ptVP->_u32Overflow = 0;
        ptVP->_u32Status &= ~(E_VPS);
        ptVP->_u32Status &= ~(E_WSS);
    }
    else
        schedule_delayed_work(&TTXWorkQueue, TTX_TIMEOUT);
}


DECLARE_TASKLET(VPSTasklet, _MDrv_VPS_BH, 0);
DECLARE_TASKLET(WSSTasklet, _MDrv_WSS_BH, 0);
DECLARE_TASKLET(TTXTasklet, MDrv_TTX_BH, 0);

irqreturn_t MDrv_TTX_Interrupt(int irq, void *dev_id)
{
    U32 u32CurVPSCount;
    U32 u32CurWSSCount;
    U32 u32CurTTXCount;

    static U32 _u32VBI_Count = 0;
    static U32 _u32VPS_Count = 0;
    static U32 _u32WSS_Count = 0;
    static U32 _u32TTX_Count = 0;

    static U32  VPSclear_time;
	static U32  WSSclear_time;

    volatile struct VBI_PARAMETERS *ptVP;
    ptVP = _tpVBIParametersTTX;

    _u32TimerTTX_last = _u32TimerTTX;
    _u32TimerTTX = jiffies;

    //ignore garbage
    if ((!MHal_TTX_IsValidPalSignal()) || ((_u32TimerTTX - _u32TimerTTX_last) > HZ/12))
    {
        //TTX_DRV_DBG("not pal or noice\n");
        ptVP->_u32Status &= ~(E_TTX);
        ptVP->_u32Status &= ~(E_VPS);
        ptVP->_u32Status &= ~(E_WSS);
        schedule_delayed_work(&TTXWorkQueue, TTX_TIMEOUT);
        MHal_TTX_ClearIRQ();
        return IRQ_HANDLED;
    }


    u32CurVPSCount = MHal_TTX_VPS_Count();
    if(_u32VPS_Count != u32CurVPSCount)
    {
        _u32VPS_Count = u32CurVPSCount;
        _MDrv_VPS_BH(0);
        VPSclear_time = jiffies;
    }
    else if(VPSclear_time <= jiffies)
    {

        if(jiffies - VPSclear_time > 400) // VPS doesn't happen about tmie of 20 frames.
        {
            ptVP->_u32Status &= ~(E_VPS);
            //TTX_DRV_DBG("NO VPS\n");
        }
    }
    else //VPSclear_time > jiffies
    {
        VPSclear_time = jiffies;
    }

    u32CurWSSCount = MHal_TTX_WSS_Count();
    if(_u32WSS_Count != u32CurWSSCount)
    {
        _u32WSS_Count = u32CurWSSCount;
        _MDrv_WSS_BH(0);
        //tasklet_schedule(&WSSTasklet);
		WSSclear_time = jiffies;
        //TTX_DRV_DBG("NEW WSS\n");
    }
	else if(WSSclear_time <= jiffies)
	{
		if(jiffies - WSSclear_time > 400) //WSS doesn't happen about tmie of 20 frames.
		{
			ptVP->_u32Status &= ~(E_WSS);
			//TTX_DRV_DBG("NO WSS\n");
		}
	}
	else //WSSclear_time > jiffies
	{
		WSSclear_time = jiffies;
	}


    u32CurTTXCount = MHal_TTX_TXX_Count();
    if((_u32TTX_Count != u32CurTTXCount))
    {
        _u32TTX_Count = u32CurTTXCount;
        tasklet_schedule(&TTXTasklet);
        //TTX_DRV_DBG("NEW TTX\n");

        _u32VBI_Count++;

        if(_u32VBI_Count >= 5)
        {
            _u32VBI_Count = 0;
            MDrv_Wakeup();
        }
    }
    else
    {
        ptVP->_u32Status &= ~(E_TTX);
        //TTX_DRV_DBG("NO TTX\n");
    }

    schedule_delayed_work(&TTXWorkQueue, TTX_TIMEOUT);

    MHal_TTX_ClearIRQ();

	{
		static U32 _u32Count = 0;

		//printk("TTX ISR(%d)\n", atomic_read(&_aInterruptStop) );
		if( _u32Count % 4096 == 0 )	// about 60 sec
		{
			//printk("TTX ISR: %10d(%d)\n", _u32Count, atomic_read(&_aInterruptStop) );
		}
		_u32Count ++;
	}

    return IRQ_HANDLED;
}

