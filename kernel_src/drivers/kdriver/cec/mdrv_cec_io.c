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
/// file    mdrv_cec_io.c
/// @brief  CEC(Consumer Electronics Control) Control Interface
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

#include "mdrv_cec_io.h"
#include "mhal_cec_reg.h"
#include "mdrv_cec.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define CEC_PRINT(fmt, args...)      //printk("[CEC][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_CEC_DEVICE_COUNT     1
#define MOD_CEC_NAME                 "ModCEC"


#define CEC_DEBUG
#ifdef CEC_DEBUG
#define DEBUG_CEC(x) (x)
#else
#define DEBUG_CEC(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _mod_cec_open (struct inode *inode, struct file *filp);
static int                      _mod_cec_release(struct inode *inode, struct file *filp);
static ssize_t                  _mod_cec_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _mod_cec_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _mod_cec_poll(struct file *filp, poll_table *wait);
static int                      _mod_cec_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _mod_cec_fasync(int fd, struct file *filp, int mode);


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

static CEC_ModHandle_t CECDev=
{
    .s32CECMajor=               MDRV_MAJOR_CEC,
    .s32CECMinor=               MDRV_MINOR_CEC,
    .cDevice=
    {
        .kobj=                  {.name= MOD_CEC_NAME, },
        .owner  =               THIS_MODULE,
    },
    .CECFop=
    {
        .open=                  _mod_cec_open,
        .release=               _mod_cec_release,
        .read=                  _mod_cec_read,
        .write=                 _mod_cec_write,
        .poll=                  _mod_cec_poll,
        .ioctl=                 _mod_cec_ioctl,
        .fasync =	            _mod_cec_fasync,
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
static int _mod_cec_open (struct inode *inode, struct file *filp)
{
	CEC_ModHandle_t *dev;

    CEC_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, CEC_ModHandle_t, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _mod_cec_release(struct inode *inode, struct file *filp)
{
    CEC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_cec_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    CEC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_cec_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    CEC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_cec_poll(struct file *filp, poll_table *wait)
{
    CEC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _mod_cec_fasync(int fd, struct file *filp, int mode)
{
    CEC_PRINT("%s is invoked\n", __FUNCTION__);

	return fasync_helper(fd, filp, mode, &CECDev.async_queue);
}

static int _mod_cec_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err = 0;
    //for CEC
    CEC_INFO_LIST_t CECData;
    CEC_TX_INFO_t   CECTxData;
    U16  u16CecResult;

    CEC_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((CEC_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> CEC_IOC_MAXNR))
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


    PROBE_IO_ENTRY(MDRV_MAJOR_CEC, _IOC_NR(cmd));

    switch(cmd)
    {
        case MDRV_CEC_INIT:
            CEC_PRINT("ioctl: MDRV_CEC_INIT\n");
            MDrv_CEC_Init();
            break;

        case MDRV_CEC_CHKDEVS:
            CEC_PRINT("ioctl: MDRV_MICOM_CEC_CHKDEVS\n");
            MDrv_CEC_ChkDevs(&CECData);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[0]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[1]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[2]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[3]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[4]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[5]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[6]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[7]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[8]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[9]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[10]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[11]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[12]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[13]);
            CEC_PRINT("ioctl:MDRV_CEC_CHKDEVS=>%d\n",CECData.CecDevicesExisted[14]);
            if (copy_to_user((CEC_INFO_LIST_t  __user *) arg, &CECData, sizeof(CEC_INFO_LIST_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CEC, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case MDRV_CEC_RX_API:
            CEC_PRINT("ioctl: MDRV_CEC_RX_API\n");
            MDrv_CEC_RxApi(&CECData);
            if (copy_to_user((CEC_INFO_LIST_t  __user *) arg, &CECData, sizeof(CEC_INFO_LIST_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CEC, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case MDRV_CEC_TX_API:
            CEC_PRINT("ioctl: MDRV_CEC_TX_API\n");
            if (copy_from_user(&CECTxData, (CEC_TX_INFO_t __user *) arg, sizeof(CEC_TX_INFO_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CEC, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MDrv_CEC_TxApi(&CECTxData);
            break;

        case MDRV_CEC_GET_RESULT:
            CEC_PRINT("ioctl: MDRV_CEC_GET_RESULT\n");
            MDrv_CEC_GetResult(&CECTxData);
            if (copy_to_user((CEC_TX_INFO_t  __user *) arg, &CECTxData, sizeof(CEC_TX_INFO_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CEC, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case MDRV_CEC_RESPONSE:
            CEC_PRINT("ioctl: MDRV_CEC_RESPONSE\n");
            u16CecResult = MDrv_CEC_Response();
            if (copy_to_user((U16  __user *) arg, &u16CecResult, sizeof(U16)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CEC, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        default:
            CEC_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_CEC, _IOC_NR(cmd));
            return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_CEC, _IOC_NR(cmd));
    return 0;
}

static int __init mod_cec_init(void)
{
    S32         s32Ret;
    dev_t       dev;

    CEC_PRINT("%s is invoked\n", __FUNCTION__);

    if (CECDev.s32CECMajor)
    {
        dev = MKDEV(CECDev.s32CECMajor, CECDev.s32CECMinor);
        s32Ret = register_chrdev_region(dev, MOD_CEC_DEVICE_COUNT, MDRV_NAME_CEC);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, CECDev.s32CECMinor, MOD_CEC_DEVICE_COUNT, MDRV_NAME_CEC);
        CECDev.s32CECMajor = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        CEC_PRINT("Unable to get major %d\n", CECDev.s32CECMajor);
        return s32Ret;
    }

    cdev_init(&CECDev.cDevice, &CECDev.CECFop);
    if (0!= (s32Ret= cdev_add(&CECDev.cDevice, dev, MOD_CEC_DEVICE_COUNT)))
    {
        CEC_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_CEC_DEVICE_COUNT);
        return s32Ret;
    }

    //MDrv_CEC_Init();

    return 0;
}

static void __exit mod_cec_exit(void)
{
    CEC_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&CECDev.cDevice);
    unregister_chrdev_region(MKDEV(CECDev.s32CECMajor, CECDev.s32CECMinor), MOD_CEC_DEVICE_COUNT);
}

module_init(mod_cec_init);
module_exit(mod_cec_exit);
