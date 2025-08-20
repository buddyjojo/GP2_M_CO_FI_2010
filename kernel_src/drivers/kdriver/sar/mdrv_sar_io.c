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
/// file    mdrv_sar_io.c
/// @brief  SAR Control Interface
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

#include "mdrv_sar_io.h"
#include "mhal_sar_reg.h"
#include "mdrv_sar.h"
#include "mhal_sar.h"


#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define SAR_PRINT(fmt, args...)      //printk("[RTC][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_SAR_DEVICE_COUNT     1
#define MOD_SAR_NAME                 "ModSAR"


#define SAR_DEBUG
#ifdef SAR_DEBUG
#define DEBUG_SAR(x) (x)
#else
#define DEBUG_SAR(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _mod_sar_open (struct inode *inode, struct file *filp);
static int                      _mod_sar_release(struct inode *inode, struct file *filp);
static ssize_t                  _mod_sar_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _mod_sar_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _mod_sar_poll(struct file *filp, poll_table *wait);
static int                      _mod_sar_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _mod_sar_fasync(int fd, struct file *filp, int mode);


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

static SAR_ModHandle_t SARDev=
{
    .s32SARMajor=               MDRV_MAJOR_SAR,
    .s32SARMinor=               MDRV_MINOR_SAR,
    .cDevice=
    {
        .kobj=                  {.name= MOD_SAR_NAME, },
        .owner  =               THIS_MODULE,
    },
    .SARFop=
    {
        .open=                  _mod_sar_open,
        .release=               _mod_sar_release,
        .read=                  _mod_sar_read,
        .write=                 _mod_sar_write,
        .poll=                  _mod_sar_poll,
        .ioctl=                 _mod_sar_ioctl,
        .fasync =	            _mod_sar_fasync,
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
static int _mod_sar_open (struct inode *inode, struct file *filp)
{
	SAR_ModHandle_t *dev;

    SAR_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, SAR_ModHandle_t, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _mod_sar_release(struct inode *inode, struct file *filp)
{
    SAR_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_sar_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    SAR_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_sar_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    SAR_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_sar_poll(struct file *filp, poll_table *wait)
{
    SAR_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _mod_sar_fasync(int fd, struct file *filp, int mode)
{
    SAR_PRINT("%s is invoked\n", __FUNCTION__);

	return fasync_helper(fd, filp, mode, &SARDev.async_queue);
}

static int _mod_sar_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err = 0;
    SAR_CFG_t  SAR_Info;
    SAR_Key_t  Key_Info;

    SAR_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((SAR_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> SAR_IOC_MAXNR))
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

    PROBE_IO_ENTRY(MDRV_MAJOR_SAR, _IOC_NR(cmd));

    switch(cmd)
    {
        case MDRV_SAR_INIT:
            SAR_PRINT("ioctl: MDRV_SAR_INIT\n");
            MDrv_SAR_Init();
            break;

        case MDRV_SAR_CH_INFO:
            SAR_PRINT("ioctl: MDRV_SAR_CH_INFO\n");
            if (copy_from_user(&SAR_Info, (SAR_CFG_t __user *) arg, sizeof(SAR_CFG_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_SAR, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            SAR_Info.u8RetVal = MDrv_SAR_SetChInfo(&SAR_Info);
            if (copy_to_user((SAR_CFG_t __user *) arg, &SAR_Info, sizeof(SAR_CFG_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_SAR, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_SAR_CH_GET_KEY:
            SAR_PRINT("ioctl: MDRV_SAR_CH_GET_KEY\n");
            if (copy_from_user(&Key_Info, (SAR_Key_t __user *) arg, sizeof(SAR_Key_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_SAR, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            Key_Info.u8RetVal = MDrv_SAR_CHGetKey(Key_Info.u8Index, Key_Info.u8Index, &Key_Info.u8Key, &Key_Info.u8Rep);
            if (copy_to_user((SAR_Key_t __user *) arg, &Key_Info, sizeof(SAR_Key_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_SAR, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_SAR_GET_KEY_CODE:
            SAR_PRINT("ioctl: MDRV_SAR_GET_KEY_CODE\n");
            if (copy_from_user(&Key_Info, (SAR_Key_t __user *) arg, sizeof(SAR_Key_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_SAR, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            Key_Info.u8RetVal = MDrv_SAR_GetKeyCode(&Key_Info.u8Key, &Key_Info.u8Rep);
            if (copy_to_user((SAR_Key_t __user *) arg, &Key_Info, sizeof(SAR_Key_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_SAR, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        default:
            SAR_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_SAR, _IOC_NR(cmd));

            return -ENOTTY;
    }


    PROBE_IO_EXIT(MDRV_MAJOR_SAR, _IOC_NR(cmd));
    return 0;
}

static int __init mod_sar_init(void)
{
    S32         s32Ret;
    dev_t       dev;

    SAR_PRINT("%s is invoked\n", __FUNCTION__);

    if (SARDev.s32SARMajor)
    {
        dev = MKDEV(SARDev.s32SARMajor, SARDev.s32SARMinor);
        s32Ret = register_chrdev_region(dev, MOD_SAR_DEVICE_COUNT, MDRV_NAME_SAR);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, SARDev.s32SARMinor, MOD_SAR_DEVICE_COUNT, MDRV_NAME_SAR);
        SARDev.s32SARMajor = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        SAR_PRINT("Unable to get major %d\n", SARDev.s32SARMajor);
        return s32Ret;
    }

    cdev_init(&SARDev.cDevice, &SARDev.SARFop);
    if (0!= (s32Ret= cdev_add(&SARDev.cDevice, dev, MOD_SAR_DEVICE_COUNT)))
    {
        SAR_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_SAR_DEVICE_COUNT);
        return s32Ret;
    }

    //MDrv_SAR_Init();

    return 0;
}

static void __exit mod_sar_exit(void)
{
    SAR_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&SARDev.cDevice);
    unregister_chrdev_region(MKDEV(SARDev.s32SARMajor, SARDev.s32SARMinor), MOD_SAR_DEVICE_COUNT);
}

module_init(mod_sar_init);
module_exit(mod_sar_exit);
