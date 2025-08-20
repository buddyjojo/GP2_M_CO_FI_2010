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

#ifndef __MDRV_MSMAILBOX_DEV_H__
#define __MDRV_MSMAILBOX_DEV_H__

/* linux header files */
// 2.6.26
//#include <linux/config.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/mm.h>

#include <asm/atomic.h>
#include <linux/interrupt.h>

/* driver board common header files */
#include "mst_devid.h"
#include "mdrv_types.h"

/* driver hal header files */
#include "mhal_msmailbox.h"

/* driver header files */
#include "mdrv_msmailbox_st.h"
#include "mdrv_msmailbox_io.h"

extern BOOL _receive_mail_fromPM(msmailbox_t *pMail);
extern BOOL _send_mail_toPM(msmailbox_t *pMail);
//-------------------------------------------------------------------------------------------------
//  struct
//-------------------------------------------------------------------------------------------------;
typedef struct
{
    int                         s32Major;
    int                         s32Minor;
    U32                         u32MM;
    BOOL                        bOpen;
    struct cdev                 cDevice;
    struct file_operations      Fop;
    struct vm_operations_struct MMops;
} devicehandle_t;


typedef _ERR_CODE_ (*ioctl_cmd_t)(U32 arg);
typedef struct
{
    void                        (*Register)(U32, ioctl_cmd_t);
    void                        (*Init)(void);
} ioctl_ops_t;

typedef struct
{
    ioctl_cmd_t                 cmd[IOCTL_MSMAILBOX_END_NUM];
    ioctl_ops_t                 ops;
} ioctrl_mgr_t;


typedef B16 (*cb_check_t)(void);
typedef struct
{
    _ERR_CODE_                  (*Wait)(void);
    _ERR_CODE_                  (*Wakeup)(void);
    _ERR_CODE_                  (*Init)(cb_check_t);
    void                        (*Release)(void);
    void                        (*Set)(void);
    void                        (*Clr)(void);
    U32                         (*Num)(void);
} ms_wq_ops_t;

typedef struct
{
    B16                         (*Check)(void);
} ms_wq_callback_ops_t;

typedef struct
{
    B16                         bValid;
    wait_queue_head_t           tWQ;
    atomic_t                    aNum;

    ms_wq_ops_t                 ops;
    ms_wq_callback_ops_t        cb_ops;
} ms_wq_mgr_t;
#endif

