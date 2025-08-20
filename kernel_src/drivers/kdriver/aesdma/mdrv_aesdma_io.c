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
/// file    mdrv_aesdma_io.c
/// @brief  aesdma Control Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>

//#include <linux/wait.h>
#include <linux/cdev.h>
#include <asm/io.h>

#include "Board.h"
#include "mst_devid.h"
#include "mdrv_types.h"
#include "chip_int.h"
#include "mdrv_aesdma.h"
#include "mhal_aesdma.h"
#include "mdrv_aesdma_io.h"
#include "mdrv_aesdma_st.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define AESDMA_PRINT(fmt, args...)   //   printk("[AESDMA][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_AESDMA_DEVICE_COUNT     1


#define AESDMA_DEBUG
#ifdef AESDMA_DEBUG
#define DEBUG_AESDMA(x) (x)
#else
#define DEBUG_AESDMA(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _mod_aesdma_open (struct inode *inode, struct file *filp);
static int                      _mod_aesdma_release(struct inode *inode, struct file *filp);
static ssize_t                  _mod_aesdma_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _mod_aesdma_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _mod_aesdma_poll(struct file *filp, poll_table *wait);
static int                      _mod_aesdma_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _mod_aesdma_fasync(int fd, struct file *filp, int mode);

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32AesdmaMajor;
    int                         s32AesdmaMinor;
    struct cdev                 cDevice;
    struct file_operations      AesdmaFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */

} AesdmaModHandle;
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

AesdmaModHandle AesdmaDev=
{
    .s32AesdmaMajor=               MDRV_MAJOR_AESDMA,
    .s32AesdmaMinor=               MDRV_MINOR_AESDMA,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_AESDMA, },
        .owner  =               THIS_MODULE,
    },
    .AesdmaFop=
    {
        .open=                  _mod_aesdma_open,
        .release=               _mod_aesdma_release,
        .read=                  _mod_aesdma_read,
        .write=                 _mod_aesdma_write,
        .poll=                  _mod_aesdma_poll,
        .ioctl=                 _mod_aesdma_ioctl,
        .fasync =	            _mod_aesdma_fasync,
    },
};

wait_queue_head_t	aesdma_key_wait_q;

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static int _mod_aesdma_open (struct inode *inode, struct file *filp)
{
	AesdmaModHandle *dev;

    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, AesdmaModHandle, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _mod_aesdma_release(struct inode *inode, struct file *filp)
{
    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_aesdma_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_aesdma_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_aesdma_poll(struct file *filp, poll_table *wait)
{
    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _mod_aesdma_fasync(int fd, struct file *filp, int mode)
{
    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);

	return fasync_helper(fd, filp, mode, &AesdmaDev.async_queue);
}

static int _mod_aesdma_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err = 0;
    AESDMA_INIT_t          AesdmaInit;
    AESDMA_START_t         AesdmaStart;
    AESDMA_SELENG_t        AesdmaSeleng;
    AESDMA_SETPS_t         AesdmaSetps;
    AESDMA_SETFILEINOUT_t  AesdmaSetfileinout;
    AESDMA_NOTIFY_t        Aesdma_Notify;
    AESDMA_RANDOM_t        Aesdma_Random;
    AESDMA_STATUS_t        Aesdma_Status;

    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((AESDMA_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> AESDMA_IOC_MAXNR))
    {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        s32Err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        s32Err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (s32Err)
    {
        return -EFAULT;
    }

    PROBE_IO_ENTRY(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));

    switch(cmd)
    {
        case MDRV_AESDMA_INIT:
            AESDMA_PRINT("ioctl: MDRV_AESDMA_INIT\n");
            if (copy_from_user(&AesdmaInit, (AESDMA_INIT_t __user *) arg, sizeof(AESDMA_INIT_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            AesdmaInit.u32Result = MDrv_AESDMA_Init(&AesdmaInit);
            if (copy_to_user((AESDMA_INIT_t __user *) arg, &AesdmaInit, sizeof(AESDMA_INIT_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_AESDMA_START:
            AESDMA_PRINT("ioctl: MDRV_AESDMA_START\n");
            if (copy_from_user(&AesdmaStart, (AESDMA_START_t __user *) arg, sizeof(AESDMA_START_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }


            MDrv_AESDMA_Start(AesdmaStart.bStart);
            if (copy_to_user((AESDMA_START_t __user *) arg, &AesdmaStart, sizeof(AESDMA_START_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_AESDMA_SEL_ENG:
            AESDMA_PRINT("ioctl: MDRV_AESDMA_SEL_EN\n");
            if (copy_from_user(&AesdmaSeleng, (AESDMA_SELENG_t __user *) arg, sizeof(AESDMA_SELENG_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            AesdmaSeleng.u32Result = MDrv_AESDMA_SelEng(&AesdmaSeleng);
            if (copy_to_user((AESDMA_SELENG_t __user *) arg, &AesdmaSeleng, sizeof(AESDMA_SELENG_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_AESDMA_SET_PS:

            AESDMA_PRINT("ioctl: MDRV_AESDMA_SET_PS\n");
            if (copy_from_user(&AesdmaSetps, (AESDMA_SETPS_t __user *) arg, sizeof(AESDMA_SETPS_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            AesdmaSetps.u32Result = MDrv_AESDMA_SetPS(&AesdmaSetps);
            if (copy_to_user((AESDMA_SETPS_t __user *) arg, &AesdmaSetps, sizeof(AESDMA_SETPS_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_AESDMA_SET_FILE_INOUT:
            AESDMA_PRINT("ioctl: MDRV_AESDMA_SET_FILE_INPUT\n");
            if (copy_from_user(&AesdmaSetfileinout, (AESDMA_SETFILEINOUT_t __user *) arg, sizeof(AESDMA_SETFILEINOUT_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            AesdmaSetfileinout.u32Result = MDrv_AESDMA_SetFileInOut(&AesdmaSetfileinout);
            if (copy_to_user((AESDMA_SETFILEINOUT_t __user *) arg, &AesdmaSetfileinout, sizeof(AESDMA_SETFILEINOUT_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_AESDMA_NOTIFY:
            AESDMA_PRINT("ioctl: MDRV_AESDMA_NOTIFY\n");
            if (copy_from_user(&Aesdma_Notify, (AESDMA_NOTIFY_t __user *) arg, sizeof(AESDMA_NOTIFY_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            Aesdma_Notify.u32Result = MDrv_AESDMA_Notify(&Aesdma_Notify);
            if (copy_to_user((AESDMA_NOTIFY_t __user *) arg, &Aesdma_Notify, sizeof(AESDMA_NOTIFY_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_AESDMA_RANDOM:

            AESDMA_PRINT("ioctl: MDRV_AESDMA_RANDOM\n");
            Aesdma_Random.u32Result = MDrv_AESDMA_Random(&Aesdma_Random);

            if (copy_to_user((AESDMA_RANDOM_t  __user *) arg, &Aesdma_Random, sizeof(AESDMA_RANDOM_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_AESDMA_STATUS:
            AESDMA_PRINT("ioctl: MDRV_AESDMA_STATUS\n");
            Aesdma_Status.Res_Status = (DRVAESDMA_RESULT)MDrv_AESDMA_GetStatus();

            if (copy_to_user((AESDMA_STATUS_t  __user *) arg, &Aesdma_Status, sizeof(AESDMA_STATUS_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_AESDMA_RESET:
            AESDMA_PRINT("ioctl: MDRV_AESDMA_RESET\n");
            MDrv_AESDMA_Reset();
            break;

        default:
            AESDMA_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
            return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_AESDMA, _IOC_NR(cmd));
    return 0;
}

static int __init mod_aesdma_init(void)
{
    S32         s32Ret;
    dev_t       dev;
	//U8			aesdma_dn_flag;

    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);

    if (AesdmaDev.s32AesdmaMajor)
    {
        dev = MKDEV(AesdmaDev.s32AesdmaMajor, AesdmaDev.s32AesdmaMinor);
        s32Ret = register_chrdev_region(dev, MOD_AESDMA_DEVICE_COUNT, MDRV_NAME_AESDMA);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, AesdmaDev.s32AesdmaMinor, MOD_AESDMA_DEVICE_COUNT, MDRV_NAME_AESDMA);
        AesdmaDev.s32AesdmaMajor = MAJOR(dev);
    }

	init_waitqueue_head(&aesdma_key_wait_q);

    if ( 0 > s32Ret)
    {
        AESDMA_PRINT("Unable to get major %d\n", AesdmaDev.s32AesdmaMajor);
        return s32Ret;
    }

    cdev_init(&AesdmaDev.cDevice, &AesdmaDev.AesdmaFop);
    if (0!= (s32Ret= cdev_add(&AesdmaDev.cDevice, dev, MOD_AESDMA_DEVICE_COUNT)))
    {
        AESDMA_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_AESDMA_DEVICE_COUNT);
        return s32Ret;
    }

    //MDrv_AESDMA_Init();

    return 0;
}

static void __exit mod_aesdma_exit(void)
{
    AESDMA_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&AesdmaDev.cDevice);
    unregister_chrdev_region(MKDEV(AesdmaDev.s32AesdmaMajor, AesdmaDev.s32AesdmaMinor), MOD_AESDMA_DEVICE_COUNT);
}

module_init(mod_aesdma_init);
module_exit(mod_aesdma_exit);

//dhjung LGE
MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("AESDMA driver");
MODULE_LICENSE("MSTAR");

