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
/// file    mdrv_micom_io.c
/// @brief  MICOM Control Interface
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
#include <linux/wait.h>
#include <linux/cdev.h>
#include <asm/io.h>

#include "Board.h"
#include "mst_devid.h"
#include "mdrv_types.h"
#include "chip_int.h"
#include "mhal_mailbox_reg.h"
#include "mhal_micom_reg.h"
#include "mhal_micom.h"
#include "mdrv_micom_io.h"
#include "mdrv_micom.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define MICOM_PRINT(fmt, args...)      //printk("[Micom][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_MICOM_DEVICE_COUNT     1


#define MICOM_DEBUG
#ifdef MICOM_DEBUG
#define DEBUG_MICOM(x) (x)
#else
#define DEBUG_MICOM(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _mod_micom_open (struct inode *inode, struct file *filp);
static int                      _mod_micom_release(struct inode *inode, struct file *filp);
static ssize_t                  _mod_micom_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _mod_micom_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _mod_micom_poll(struct file *filp, poll_table *wait);
static int                      _mod_micom_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _mod_micom_fasync(int fd, struct file *filp, int mode);


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

MicomModHandle MicomDev=
{
    .s32MicomMajor=               MDRV_MAJOR_MICOM,
    .s32MicomMinor=               MDRV_MINOR_MICOM,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_MICOM, },
        .owner  =               THIS_MODULE,
    },
    .MicomFop=
    {
        .open=                  _mod_micom_open,
        .release=               _mod_micom_release,
        .read=                  _mod_micom_read,
        .write=                 _mod_micom_write,
        .poll=                  _mod_micom_poll,
        .ioctl=                 _mod_micom_ioctl,
        .fasync =	            _mod_micom_fasync,
    },
};

wait_queue_head_t	key_wait_q;

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static int _mod_micom_open (struct inode *inode, struct file *filp)
{
	MicomModHandle *dev;

    MICOM_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, MicomModHandle, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _mod_micom_release(struct inode *inode, struct file *filp)
{
    MICOM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_micom_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    MICOM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_micom_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    MICOM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_micom_poll(struct file *filp, poll_table *wait)
{
    MICOM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _mod_micom_fasync(int fd, struct file *filp, int mode)
{
    MICOM_PRINT("%s is invoked\n", __FUNCTION__);

	return fasync_helper(fd, filp, mode, &MicomDev.async_queue);
}

static int _mod_micom_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err = 0;
    PM_WakeCfg_t PmWakeCfg;
    PM_DrvStatus_t PmStatus;
    PM_LibVer_t PmLibVer;
    PM_DrvInfo_t PmDrvInfo;
    PM_DbgLv_t PmDbgLv;
    //RTC Section
    U32 u32Counter;

    //SAR Section
    SAR_CFG_t  SAR_Info;
    SAR_Key_t  Key_Info;

    MICOM_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((MICOM_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> MICOM_IOC_MAXNR))
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

    PROBE_IO_ENTRY(MDRV_MAJOR_MICOM, _IOC_NR(cmd));

    switch(cmd)
    {
        //PM Section
        case MDRV_MICOM_INIT:
            MICOM_PRINT("ioctl: MDRV_MICOM_INIT\n");
            if (copy_from_user(&PmWakeCfg, (PM_WakeCfg_t __user *) arg, sizeof(PM_WakeCfg_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            PmWakeCfg.Result = MDrv_MICOM_Init(&PmWakeCfg);
            if (copy_to_user((PM_WakeCfg_t __user *) arg, &PmWakeCfg, sizeof(PM_WakeCfg_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            break;

        case MDRV_MICOM_GET_STATUS:
            MICOM_PRINT("ioctl: MDRV_MICOM_GET_STATUS\n");
            if (copy_from_user(&PmStatus, (PM_DrvStatus_t __user *) arg, sizeof(PM_DrvStatus_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            PmStatus.Result = MDrv_MICOM_GetStatus(&PmStatus.DrvStatus);
            if (copy_to_user((PM_DrvStatus_t __user *) arg, &PmStatus, sizeof(PM_DrvStatus_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_MICOM_GET_VER:
            MICOM_PRINT("ioctl: MDRV_MICOM_GET_VER\n");
            if (copy_from_user(&PmLibVer, (PM_LibVer_t __user *) arg, sizeof(PM_LibVer_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            PmLibVer.Result = MDrv_MICOM_GetLibVer(&PmLibVer);
            if (copy_to_user((PM_LibVer_t __user *) arg, &PmLibVer, sizeof(PM_LibVer_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_MICOM_GET_INFO:
            MICOM_PRINT("ioctl: MDRV_MICOM_GET_INFO\n");
            if (copy_from_user(&PmDrvInfo, (PM_DrvInfo_t __user *) arg, sizeof(PM_DrvInfo_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            PmDrvInfo.Result = MDrv_MICOM_GetInfo(&PmDrvInfo);
            if (copy_to_user((PM_DrvInfo_t __user *) arg, &PmDrvInfo, sizeof(PM_DrvInfo_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_MICOM_SET_DBG_LEVEL:
            MICOM_PRINT("ioctl: MDRV_MICOM_SET_DBG_LEVEL\n");
            if (copy_from_user(&PmDbgLv, (PM_DbgLv_t __user *) arg, sizeof(PM_DbgLv_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            PmDbgLv.Result = MDrv_MICOM_SetDbgLevel(PmDbgLv.DebugLv);
            if (copy_to_user((PM_DbgLv_t __user *) arg, &PmDbgLv, sizeof(PM_DbgLv_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            break;

        case MDRV_MICOM_POWER_DOWN:
            MICOM_PRINT("ioctl: MDRV_MICOM_POWER_DOWN\n");
            break;

        case MDRV_MICOM_RESET:
            MICOM_PRINT("ioctl: MDRV_MICOM_RESET\n");
            break;

        case MDRV_MICOM_CONTROL:
            MICOM_PRINT("ioctl: MDRV_MICOM_CONTROL\n");
            break;

        case MDRV_MICOM_TX_DATA:
            MICOM_PRINT("ioctl: MDRV_MICOM_TX_DATA\n");
            break;

        case MDRV_MICOM_RX_DATA:
            MICOM_PRINT("ioctl: MDRV_MICOM_RX_DATA\n");
            break;

        //RTC Section
        case MDRV_RTC_INIT:
            MICOM_PRINT("ioctl: MDRV_RTC_INIT\n");
            MDrv_MICOM_Rtc_Init();
            break;

        case MDRV_RTC_SET_COUNTER:
            MICOM_PRINT("ioctl: MDRV_RTC_SET_COUNTER\n");
            if (copy_from_user(&u32Counter, (U32 __user *) arg, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MICOM_PRINT("u32Counter=%04x\n",u32Counter);
            MDrv_MICOM_Rtc_SetCounter(u32Counter);
            break;

        case MDRV_RTC_GET_COUNTER:
            MICOM_PRINT("ioctl: MDRV_RTC_GET_COUNTER\n");
            u32Counter = MDrv_MICOM_Rtc_GetCounter();
            if (copy_to_user((U32 __user *) arg, &u32Counter, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MICOM_PRINT("u32Counter=%04x\n",u32Counter);
            break;

        case MDRV_RTC_SET_MATCH_COUNTER:
            MICOM_PRINT("ioctl: MDRV_RTC_SET_MATCH_COUNTER\n");
            if (copy_from_user(&u32Counter, (U32 __user *) arg, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MICOM_PRINT("u32Counter=%04x\n",u32Counter);
            MDrv_MICOM_Rtc_SetMatchCounter(u32Counter);
            break;

        case MDRV_RTC_GET_MATCH_COUNTER:
            MICOM_PRINT("ioctl: MDRV_RTC_GET_MATCH_COUNTER\n");
            u32Counter = MDrv_MICOM_Rtc_GetMatchCounter();
            if (copy_to_user((U32 __user *) arg, &u32Counter, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MICOM_PRINT("u32Counter=%04x\n",u32Counter);
            break;

        case MDRV_RTC_CLEAR_INT:
            MICOM_PRINT("ioctl: MDRV_RTC_CLEAR_INT\n");
            MDrv_MICOM_Rtc_ClearInterrupt();
            break;

        case MDRV_RTC_TEST:
            MICOM_PRINT("ioctl: MDRV_RTC_TEST\n");
            MDrv_MICOM_Rtc_Test();
            break;


			
        //SAR Section
        case MDRV_SAR_INIT:
            MICOM_PRINT("ioctl: MDRV_SAR_INIT\n");
            MDrv_MICOM_Sar_Init();
            break;

        case MDRV_SAR_CH_INFO:
            MICOM_PRINT("ioctl: MDRV_SAR_CH_INFO\n");
            if (copy_from_user(&SAR_Info, (SAR_CFG_t __user *) arg, sizeof(SAR_CFG_t)))
 	            return -EFAULT;
            SAR_Info.u8RetVal = MDrv_MICOM_Sar_SetChInfo(&SAR_Info);
            if (copy_to_user((SAR_CFG_t __user *) arg, &SAR_Info, sizeof(SAR_CFG_t)))
 	            return -EFAULT;
            break;

        case MDRV_SAR_CH_GET_KEY:
            MICOM_PRINT("ioctl: MDRV_SAR_CH_GET_KEY\n");
            if (copy_from_user(&Key_Info, (SAR_Key_t __user *) arg, sizeof(SAR_Key_t)))
 	            return -EFAULT;
            Key_Info.u8RetVal = MDrv_MICOM_Sar_CHGetKey(Key_Info.u8Index, Key_Info.u8Index, &Key_Info.u8Key, &Key_Info.u8Rep);
            if (copy_to_user((SAR_Key_t __user *) arg, &Key_Info, sizeof(SAR_Key_t)))
 	            return -EFAULT;
            break;

        case MDRV_SAR_GET_KEY_CODE:
            MICOM_PRINT("ioctl: MDRV_SAR_GET_KEY_CODE\n");
            if (copy_from_user(&Key_Info, (SAR_Key_t __user *) arg, sizeof(SAR_Key_t)))
 	            return -EFAULT;
            Key_Info.u8RetVal = MDrv_MICOM_Sar_GetKeyCode(&Key_Info.u8Key, &Key_Info.u8Rep);
            if (copy_to_user((SAR_Key_t __user *) arg, &Key_Info, sizeof(SAR_Key_t)))
 	            return -EFAULT;
            break;

        default:
            MICOM_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
            return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_MICOM, _IOC_NR(cmd));
    return 0;
}

static int __init mod_micom_init(void)
{
    S32         s32Ret;
    dev_t       dev;
	//U8			micom_dn_flag;

    MICOM_PRINT("%s is invoked\n", __FUNCTION__);

    if (MicomDev.s32MicomMajor)
    {
        dev = MKDEV(MicomDev.s32MicomMajor, MicomDev.s32MicomMinor);
        s32Ret = register_chrdev_region(dev, MOD_MICOM_DEVICE_COUNT, MDRV_NAME_MICOM);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, MicomDev.s32MicomMinor, MOD_MICOM_DEVICE_COUNT, MDRV_NAME_MICOM);
        MicomDev.s32MicomMajor = MAJOR(dev);
    }

	init_waitqueue_head(&key_wait_q);

    if ( 0 > s32Ret)
    {
        MICOM_PRINT("Unable to get major %d\n", MicomDev.s32MicomMajor);
        return s32Ret;
    }

    cdev_init(&MicomDev.cDevice, &MicomDev.MicomFop);
    if (0!= (s32Ret= cdev_add(&MicomDev.cDevice, dev, MOD_MICOM_DEVICE_COUNT)))
    {
        MICOM_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_MICOM_DEVICE_COUNT);
        return s32Ret;
    }

    //MDrv_MICOM_Init();

    return 0;
}

static void __exit mod_micom_exit(void)
{
    printk("%s is invoked\n", __FUNCTION__);

    cdev_del(&MicomDev.cDevice);
    unregister_chrdev_region(MKDEV(MicomDev.s32MicomMajor, MicomDev.s32MicomMinor), MOD_MICOM_DEVICE_COUNT);
}

EXPORT_SYMBOL(MicomDev);
EXPORT_SYMBOL(key_wait_q);
#ifdef DEBUG_MICOM_MODULE
EXPORT_SYMBOL(gMicomScartID);
module_init(mod_micom_init);
#else
__define_initcall("5", mod_micom_init, 0);
#endif
module_exit(mod_micom_exit);

//dhjung LGE
MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MICOM driver");
MODULE_LICENSE("MSTAR");

