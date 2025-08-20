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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mhal_gop_interrupt.c
/// @brief  GOP HAL layer Driver for handling interrupt
/// @author MStar Semiconductor Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////


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

#include "mhal_gop.h"
#include "mdrv_gop.h"

extern U8 *pDataMem;
extern U32 gInterruptFlag;      //refer declaration in mhal_mvd.c


void _gop_interrupt(unsigned long unused)
{
	printk("_gop_interrupt\n");
    return;
}


DECLARE_TASKLET(GOPTasklet, _gop_interrupt, 0);


irqreturn_t gop_interrupt(int irq, void *devid)
{
	printk("gop_interrupt\n");
    tasklet_schedule(&GOPTasklet);
    //_gop_interrupt();
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
    //process interrupt:
    MHal_GOP_ProcessIRQ();
#endif
    //clear interrupt
    MHal_GOP_ClearIRQ();
    //printk("received GOP interrupt \n");
    return IRQ_HANDLED;
}




