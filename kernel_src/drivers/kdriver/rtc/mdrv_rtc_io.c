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
/// file    mdrv_rtc_io.c
/// @brief  RTC Control Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
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
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mst_devid.h"

#include "mdrv_rtc_io.h"
#include "mhal_rtc_reg.h"
#include "mdrv_rtc.h"
#include "mhal_rtc.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define RTC_PRINT(fmt, args...)      //printk("[RTC][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_RTC_DEVICE_COUNT     1
#define MOD_RTC_NAME                 "ModRTC"


#define RTC_DEBUG
#ifdef RTC_DEBUG
#define DEBUG_RTC(x) (x)
#else
#define DEBUG_RTC(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _mod_rtc_open (struct inode *inode, struct file *filp);
static int                      _mod_rtc_release(struct inode *inode, struct file *filp);
static ssize_t                  _mod_rtc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _mod_rtc_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _mod_rtc_poll(struct file *filp, poll_table *wait);
static int                      _mod_rtc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _mod_rtc_fasync(int fd, struct file *filp, int mode);


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

static RTC_ModHandle_t RTCDev=
{
    .s32RTCMajor=               MDRV_MAJOR_RTC,
    .s32RTCMinor=               MDRV_MINOR_RTC,
    .cDevice=
    {
        .kobj=                  {.name= MOD_RTC_NAME, },
        .owner  =               THIS_MODULE,
    },
    .RTCFop=
    {
        .open=                  _mod_rtc_open,
        .release=               _mod_rtc_release,
        .read=                  _mod_rtc_read,
        .write=                 _mod_rtc_write,
        .poll=                  _mod_rtc_poll,
        .ioctl=                 _mod_rtc_ioctl,
        .fasync =	            _mod_rtc_fasync,
    },
};

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static int _mod_rtc_open (struct inode *inode, struct file *filp)
{
	RTC_ModHandle_t *dev;

    RTC_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, RTC_ModHandle_t, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _mod_rtc_release(struct inode *inode, struct file *filp)
{
    RTC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_rtc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    RTC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_rtc_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    RTC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_rtc_poll(struct file *filp, poll_table *wait)
{
    RTC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _mod_rtc_fasync(int fd, struct file *filp, int mode)
{
    RTC_PRINT("%s is invoked\n", __FUNCTION__);

	return fasync_helper(fd, filp, mode, &RTCDev.async_queue);
}

static int _mod_rtc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err = 0;
    U32 u32Counter;

    RTC_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((RTC_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> RTC_IOC_MAXNR))
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

    PROBE_IO_ENTRY(MDRV_MAJOR_RTC, _IOC_NR(cmd));

    switch(cmd)
    {
        case MDRV_RTC_INIT:
            RTC_PRINT("ioctl: MDRV_RTC_INIT\n");
            MDrv_RTC_Init();
            break;

        case MDRV_RTC_SET_COUNTER:
            RTC_PRINT("ioctl: MDRV_RTC_SET_COUNTER\n");
            if (copy_from_user(&u32Counter, (U32 __user *) arg, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RTC, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            RTC_PRINT("u32Counter=%04x\n",u32Counter);
            MDrv_RTC_SetCounter(u32Counter);
            break;

        case MDRV_RTC_GET_COUNTER:
            RTC_PRINT("ioctl: MDRV_RTC_GET_COUNTER\n");
            u32Counter = MDrv_RTC_GetCounter();
            if (copy_to_user((U32 __user *) arg, &u32Counter, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RTC, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            RTC_PRINT("u32Counter=%04x\n",u32Counter);
            break;

        case MDRV_RTC_SET_MATCH_COUNTER:
            RTC_PRINT("ioctl: MDRV_RTC_SET_MATCH_COUNTER\n");
            if (copy_from_user(&u32Counter, (U32 __user *) arg, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RTC, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            RTC_PRINT("u32Counter=%04x\n",u32Counter);
            MDrv_RTC_SetMatchCounter(u32Counter);
            break;

        case MDRV_RTC_GET_MATCH_COUNTER:
            RTC_PRINT("ioctl: MDRV_RTC_GET_MATCH_COUNTER\n");
            u32Counter = MDrv_RTC_GetMatchCounter();
            if (copy_to_user((U32 __user *) arg, &u32Counter, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RTC, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            RTC_PRINT("u32Counter=%04x\n",u32Counter);
            break;

        case MDRV_RTC_CLEAR_INT:
            RTC_PRINT("ioctl: MDRV_RTC_CLEAR_INT\n");
            MDrv_RTC_ClearInterrupt();
            break;

        case MDRV_RTC_TEST:
            RTC_PRINT("ioctl: MDRV_RTC_TEST\n");
            MDrv_RTC_Test();
            break;

        default:
            RTC_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_RTC, _IOC_NR(cmd));
            return -ENOTTY;
    }


    PROBE_IO_EXIT(MDRV_MAJOR_RTC, _IOC_NR(cmd));
    return 0;
}

static int __init mod_rtc_init(void)
{
    S32         s32Ret;
    dev_t       dev;

    RTC_PRINT("%s is invoked\n", __FUNCTION__);

    if (RTCDev.s32RTCMajor)
    {
        dev = MKDEV(RTCDev.s32RTCMajor, RTCDev.s32RTCMinor);
        s32Ret = register_chrdev_region(dev, MOD_RTC_DEVICE_COUNT, MDRV_NAME_RTC);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, RTCDev.s32RTCMinor, MOD_RTC_DEVICE_COUNT, MDRV_NAME_RTC);
        RTCDev.s32RTCMajor = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        RTC_PRINT("Unable to get major %d\n", RTCDev.s32RTCMajor);
        return s32Ret;
    }

    cdev_init(&RTCDev.cDevice, &RTCDev.RTCFop);
    if (0!= (s32Ret= cdev_add(&RTCDev.cDevice, dev, MOD_RTC_DEVICE_COUNT)))
    {
        RTC_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_RTC_DEVICE_COUNT);
        return s32Ret;
    }

    //MDrv_RTC_Init();

    return 0;
}

static void __exit mod_rtc_exit(void)
{
    RTC_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&RTCDev.cDevice);
    unregister_chrdev_region(MKDEV(RTCDev.s32RTCMajor, RTCDev.s32RTCMinor), MOD_RTC_DEVICE_COUNT);
}

module_init(mod_rtc_init);
module_exit(mod_rtc_exit);
