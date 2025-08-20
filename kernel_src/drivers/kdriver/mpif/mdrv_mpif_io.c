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
/// file    mdrv_mpif.c
/// @brief  MPIF Driver
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


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
#include <linux/time.h>  //added
#include <linux/cdev.h>
#include <asm/io.h>
#include "mdrv_mpif_io.h"
#include "mdrv_mpif.h"
#include "mst_devid.h"

#include "mdrv_probe.h"

#define MPIF_WARNING(fmt, args...)       printk(KERN_WARNING "[MPIF][%06d]     " fmt, __LINE__, ## args)
#define MPIF_PRINT(fmt, args...)         printk("[MPIF][%06d]     " fmt, __LINE__, ## args)

#define OPT_IO_DGB 0
#if OPT_IO_DGB
#define MPIF_IO_DEBUG(fmt, args...)           printk("\033[47;34m[IO][%05d] " fmt "\033[0m", __LINE__, ## args)
#else
#define MPIF_IO_DEBUG(fmt, args...)
#endif
#define MOD_MPIF_DEVICE_COUNT                 1
#define MOD_MPIF_NAME                         "ModMPIF"


//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32MPIFMajor;
    int                         s32MPIFMinor;
    struct cdev                 cDevice;
    struct file_operations      MPIFFop;
} MPIF_Dev_t;


//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
static int __init   MDrv_MPIF_ModuleInit(void);
static void __exit  MDrv_MPIF_ModuleExit(void);

static int          MDrv_MPIF_open(struct inode *inode, struct file *filp);
static int          MDrv_MPIF_release(struct inode *inode, struct file *filp);
static ssize_t      MDrv_MPIF_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t      MDrv_MPIF_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static int          MDrv_MPIF_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);


//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
static MPIF_Dev_t _mpif_dev =
{
    .s32MPIFMajor          =           MDRV_MAJOR_MPIF,
    .s32MPIFMinor          =           MDRV_MINOR_MPIF,
    .cDevice              =
    {
        .kobj             =              {.name= MOD_MPIF_NAME, },
        .owner            =              THIS_MODULE,
    },
    .MPIFFop               =
    {
        .open             =              MDrv_MPIF_open,
        .release          =              MDrv_MPIF_release,
        .read             =              MDrv_MPIF_read,
        .write            =              MDrv_MPIF_write,
        .ioctl            =              MDrv_MPIF_Ioctl,
    },
};





//--------------------------------------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------------------------------------
static int __init MDrv_MPIF_ModuleInit(void)
{
    int     s32Ret;
    dev_t   dev;

    MPIF_PRINT("MDrv_MPIF_ModuleInit\n");

    if (_mpif_dev.s32MPIFMajor)
    {
        dev = MKDEV(_mpif_dev.s32MPIFMajor, _mpif_dev.s32MPIFMinor);
        s32Ret = register_chrdev_region(dev, MOD_MPIF_DEVICE_COUNT, MOD_MPIF_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, _mpif_dev.s32MPIFMinor, MOD_MPIF_DEVICE_COUNT, MOD_MPIF_NAME);
        _mpif_dev.s32MPIFMajor = MAJOR(dev);
    }

    if (0 > s32Ret)
    {
        MPIF_PRINT("Unable to get major %d\n", _mpif_dev.s32MPIFMajor);
        return s32Ret;
    }

    cdev_init(&_mpif_dev.cDevice, &_mpif_dev.MPIFFop);
    if (0 != (s32Ret = cdev_add(&_mpif_dev.cDevice, dev, MOD_MPIF_DEVICE_COUNT)))
    {
        MPIF_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_MPIF_DEVICE_COUNT);
        return s32Ret;
    }

    return 0;
}

static void __exit MDrv_MPIF_ModuleExit(void)
{
    cdev_del(&_mpif_dev.cDevice);
    unregister_chrdev_region(MKDEV(_mpif_dev.s32MPIFMajor, _mpif_dev.s32MPIFMinor), MOD_MPIF_DEVICE_COUNT);
}

static int MDrv_MPIF_open(struct inode *inode, struct file *filp)
{
    MPIF_Dev_t* pdev;

    pdev = container_of(inode->i_cdev, MPIF_Dev_t, cDevice);
    filp->private_data = pdev;
    MPIF_PRINT("MDrv_MPIF_open\n");

    return 0;
}

static int MDrv_MPIF_release(struct inode *inode, struct file *filp)
{
    //MPIF_Dev_t* pdev = filp->private_data;
    return 0;
}

static ssize_t MDrv_MPIF_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

static ssize_t MDrv_MPIF_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

static int MDrv_MPIF_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{

    int err = 0;
    //printk("[MDrv_MPIF_Ioctl]\n");
    //MPIF_Dev_t* pdev = filp->private_data;

    // extract the type and number bitfields, and don't decode
    // wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
    if (_IOC_TYPE(cmd) != MPIF_IOCTL_MAGIC)   return -ENOTTY;
    if (_IOC_NR(cmd) > IOCTL_MPIF_MAXNR)      return -ENOTTY;

    // the direction is a bitmask, and VERIFY_WRITE catches R/W
    // transfers. ¡¥Type¡¦ is user oriented, while
    // access_ok is kernel oriented, so the concept of "read" and
    // "write" is reversed
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;


    PROBE_IO_ENTRY(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
    switch (cmd)
    {
    	case IOCTL_MPIF_INIT:
    	{
    	    MPIF_INIT_PARAM param;
            U32 err;
            err = __copy_from_user(&param,(MPIF_INIT_PARAM*)arg,sizeof(MPIF_INIT_PARAM));
            if (err != 0)
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

	        MPIF_IO_DEBUG("IOCTL_MPIF_INIT[CK:%u, TR:%u, WC:%u\n", param.u8clk, param.u8trc, param.u8wc);
	        MDrv_MPIF_Init(param.u8clk, param.u8trc, param.u8wc);
	        break;
        }
        case IOCTL_MPIF_INIT_SPIF:
        {
            U8 u8slaveid;
            if (copy_from_user(&u8slaveid,(MPIF_INIT_PARAM*)arg,sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            MPIF_IO_DEBUG("IOCTL_MPIF_INIT_SPIF[ID:%u]\n", u8slaveid);
            MDrv_MPIF_InitSPIF(u8slaveid);
            break;
        }
        case IOCTL_MPIF_1A:
        {
            MPIF_PARAM param;
            U8 u8data;

            if (copy_from_user(&param,(MPIF_PARAM*)arg,sizeof(MPIF_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            u8data = (U8)param.data;
            param.bRet = MDrv_MPIF_LC1A_RW(param.u8bWite, param.slaveid, param.addr, &u8data);
            param.data = (U32)u8data;
            MPIF_IO_DEBUG("IOCTL_MPIF_1A[W:%u, ID:%u A:0x%x D:0x%x]\n",
                param.u8bWite, param.slaveid, param.addr, u8data);

            if (copy_to_user((MPIF_PARAM*)arg, (MPIF_PARAM*)&param, sizeof(MPIF_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            break;
        }
        case IOCTL_MPIF_2A:
        {
            MPIF_PARAM param;
            U16 u16data;

            if (copy_from_user(&param,(MPIF_PARAM*)arg,sizeof(MPIF_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            u16data = (U16)param.data;
            param.bRet = MDrv_MPIF_LC2A_RW(param.u8bWite, param.slaveid, param.addr, &u16data);
            param.data = (U16)u16data;
            MPIF_IO_DEBUG("IOCTL_MPIF_1A[W:%u, ID:%u A:0x%x D:0x%x]\n",
                param.u8bWite, param.slaveid, param.addr, param.data);

            if (copy_to_user((MPIF_PARAM*)arg, (MPIF_PARAM*)&param, sizeof(MPIF_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            break;
        }
        case IOCTL_MPIF_2B:
        {
            MPIF_PARAM param;
            U16 u16data;

            if (copy_from_user(&param,(MPIF_PARAM*)arg,sizeof(MPIF_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            u16data = (U16)param.data;
            param.bRet = MDrv_MPIF_LC2B_RW(param.u8bWite, param.slaveid, param.addr, &u16data);
            param.data = (U16)u16data;
            MPIF_IO_DEBUG("MDrv_MPIF_LC2B_RW[W:%u, ID:%u A:0x%x D:0x%x]\n",
                param.u8bWite, param.slaveid, param.addr, param.data);

            if (copy_to_user((MPIF_PARAM*)arg, (MPIF_PARAM*)&param, sizeof(MPIF_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            break;
        }
		case IOCTL_MPIF_3A_RIU:
		{
            MPIF_LC3XRIU_PARAM param;

            if (copy_from_user(&param,(MPIF_LC3XRIU_PARAM*)arg,sizeof(MPIF_LC3XRIU_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            param.bRet = MDrv_MPIF_LC3A_RIU_RW(&param);
            MPIF_IO_DEBUG("IOCTL_MPIF_3ARIU[W:%u, ID:%u]\n", param.u8bWite, param.slaveid);

            if (copy_to_user((MPIF_LC3XRIU_PARAM*)arg, (MPIF_LC3XRIU_PARAM*)&param, sizeof(MPIF_LC3XRIU_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            break;
        }
		case IOCTL_MPIF_3A_MIU:
		{
            MPIF_LC3XMIU_PARAM param;

            if (copy_from_user(&param,(MPIF_LC3XMIU_PARAM*)arg,sizeof(MPIF_LC3XMIU_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }
            param.bRet = MDrv_MPIF_LC3A_MIU_RW(&param);
            MPIF_IO_DEBUG("IOCTL_MPIF_3AMIU[W:%u, ID:%u]\n", param.u8bWite, param.slaveid);

            if (copy_to_user((MPIF_LC3XMIU_PARAM*)arg, (MPIF_LC3XMIU_PARAM*)&param, sizeof(MPIF_LC3XMIU_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            break;
        }
		case IOCTL_MPIF_3B_RIU:
		{
            MPIF_LC3XRIU_PARAM param;

            if (copy_from_user(&param,(MPIF_LC3XRIU_PARAM*)arg,sizeof(MPIF_LC3XRIU_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }
            param.bRet = MDrv_MPIF_LC3B_RIU_RW(&param);
            MPIF_IO_DEBUG("IOCTL_MPIF_3BRIU[W:%u, ID:%u]\n", param.u8bWite, param.slaveid);

            if (copy_to_user((MPIF_LC3XRIU_PARAM*)arg, (MPIF_LC3XRIU_PARAM*)&param, sizeof(MPIF_LC3XRIU_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            break;
        }
		case IOCTL_MPIF_3B_MIU:
		{
            MPIF_LC3XMIU_PARAM param;

            if (copy_from_user(&param,(MPIF_LC3XMIU_PARAM*)arg,sizeof(MPIF_LC3XMIU_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }
            param.bRet = MDrv_MPIF_LC3B_MIU_RW(&param);
            MPIF_IO_DEBUG("IOCTL_MPIF_3BMIU[W:%u, ID:%u]\n", param.u8bWite, param.slaveid);

            if (copy_to_user((MPIF_LC3XMIU_PARAM*)arg, (MPIF_LC3XMIU_PARAM*)&param, sizeof(MPIF_LC3XMIU_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            break;
        }
		case IOCTL_MPIF_4A:
		{
            MPIF_LC4A_PARAM param;

            if (copy_from_user(&param,(MPIF_LC4A_PARAM*)arg,sizeof(MPIF_LC4A_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }
            param.bRet = MDrv_MPIF_LC4A_MIU_RW(&param);
            MPIF_IO_DEBUG("IOCTL_MPIF_4AMIU[W:%u, ID:%u]\n", param.u8bWite, param.slaveid);

            if (copy_to_user((MPIF_LC4A_PARAM*)arg, (MPIF_LC4A_PARAM*)&param, sizeof(MPIF_LC4A_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            break;
        }
        case IOCTL_MPIF_SET_CMDDATA_WIDTH:
        {
            MPIF_BUS_PARAM param;
            BOOL ret;
            MPIF_IO_DEBUG("IOCTL_MPIF_SET_CMDDATA_WIDTH\n");
            if (copy_from_user(&param,(MPIF_BUS_PARAM*)arg,sizeof(MPIF_BUS_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            ret = MDrv_MPIF_SetCmdDataWidth(param.u8slaveid, param.u8cmdwidth, param.u8datawidth);

            if (copy_to_user((BOOL*)arg, &ret, sizeof(BOOL)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;
        }
        case IOCTL_MPIF_SET_SPIF_CLKINVDELAY:
		{
            MPIF_SPIF_CLKINVDELAY_PARAM param;
            BOOL ret;
            MPIF_IO_DEBUG("IOCTL_MPIF_SET_SPIF_CLKINVDELAY\n");
            if (copy_from_user(&param,(MPIF_SPIF_CLKINVDELAY_PARAM*)arg,sizeof(MPIF_SPIF_CLKINVDELAY_PARAM)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }

            ret = MDrv_MPIF_SetSlave_ClkInv_Delay(param.u8slaveid, param.u8DlyBufNum);

            if (copy_to_user((BOOL*)arg, &ret, sizeof(BOOL)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;
        }
        default:
	        MPIF_PRINT("mpif ioctl: unknown command, <%d>\n", cmd);
            PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
	        return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_MPIF, _IOC_NR(cmd));
    return 0;

}


module_init(MDrv_MPIF_ModuleInit);
module_exit(MDrv_MPIF_ModuleExit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MPIF driver");
MODULE_LICENSE("MSTAR");


