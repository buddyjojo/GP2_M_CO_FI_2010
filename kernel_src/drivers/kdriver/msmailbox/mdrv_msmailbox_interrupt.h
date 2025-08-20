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

#ifndef __MDRV_MSMAILBOX_INTERRUPT_H__
#define __MDRV_MSMAILBOX_INTERRUPT_H__

#include <asm/atomic.h>
#include <linux/interrupt.h>

#include "mdrv_types.h"
#include "mhal_msmailbox_interrupt.h"

#include "chip_int.h"

extern irqreturn_t  MDrv_MSMailBox_Interrupt            (int irq, void *dev_id);

//#define MSMAILBOX_FIQ_NUM               0x08

#define STOP_INTERRUPT_FLAG             0x00
#define START_INTERRUPT_FLAG            0x01

typedef void (*cb_irq_t)(void*);
typedef struct
{
    B16                                 (*Flag)(U8);
    void                                (*Bypass)(U8);
    void                                (*Handle)(U8);
    void                                (*Enable)(U8);
    void                                (*Disable)(U8);
    void                                (*Init)(void*, cb_irq_t);
    void                                (*Release)(void*);
    void                                (*Clear)(U8);
    void                                (*IRCB)(void*);
} int_mgr_ops_t;

typedef struct
{
    B16                                 bValid;
    B16                                 bFiq;

    atomic_t                            aFlag[CPUID_MAX_NO];

    int_mgr_ops_t                       ops;
} int_mgr_t;



#endif



