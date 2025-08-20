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
/// file    drv_spi.c
/// @brief  SPI Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include "mst_devid.h"
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_spi.h"
#include "mdrv_spi_io.h"
#include "mdrv_spi_st.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_SPI_DEVICE_COUNT    1

#define SPI_WARNING(fmt, args...)       printk(KERN_WARNING "[SPIMOD][%06d] " fmt, __LINE__, ## args)
#if 0
#define SPI_PRINT(fmt, args...)         printk("[SPIMOD][%06d]     " fmt, __LINE__, ## args)
#else
#define SPI_PRINT(fmt, args...)
#endif

#define SEC_SIZE_BIT 12    // 4KB / Sector

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32SysMajor;
    int                         s32SysMinor;
    struct cdev                 cDevice;
    struct file_operations      SysFop;
} SysModHandle;


//-------------------------------------------------------------------------------------------------
//  Forward declaration
//-------------------------------------------------------------------------------------------------
static int __init   _mod_spi_init(void);
static void __exit  _mod_spi_exit(void);

static int _mod_spi_open (struct inode *inode, struct file *filp);
static int _mod_spi_release(struct inode *inode, struct file *filp);
static int _mod_spi_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

static U32 MDrv_SerFlash_Init_IO(void);
static U32 MDrv_SerFlash_Read_IO(U32 arg);
static U32 MDrv_SerFlash_Write_IO(U32 arg);
static U32 MDrv_SerFlash_EraseSec_IO(U32 arg);

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static SysModHandle SysDev=
{
    .s32SysMajor=               MDRV_MAJOR_SPI,
    .s32SysMinor=               MDRV_MINOR_SPI,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_SPI, },
        .owner  =               THIS_MODULE,
    },
    .SysFop=
    {
        .open=                  _mod_spi_open,
        .release=               _mod_spi_release,
        .ioctl=                 _mod_spi_ioctl,
    },
};

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
static int __init _mod_spi_init(void)
{
    int         s32Ret;
    dev_t       dev;

    SPI_PRINT("%s is invoked\n", __FUNCTION__);

    if (SysDev.s32SysMajor)
    {
        dev=                    MKDEV(SysDev.s32SysMajor, SysDev.s32SysMinor);
        s32Ret=                 register_chrdev_region(dev, MOD_SPI_DEVICE_COUNT, MDRV_NAME_SPI);
    }
    else
    {
        s32Ret=                 alloc_chrdev_region(&dev, SysDev.s32SysMinor, MOD_SPI_DEVICE_COUNT, MDRV_NAME_SPI);
        SysDev.s32SysMajor=     MAJOR(dev);
    }
    if (0> s32Ret)
    {
        SPI_WARNING("Unable to get major %d\n", SysDev.s32SysMajor);
        return s32Ret;
    }

    cdev_init(&SysDev.cDevice, &SysDev.SysFop);
    if (0!= (s32Ret= cdev_add(&SysDev.cDevice, dev, MOD_SPI_DEVICE_COUNT)))
    {
        SPI_WARNING("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_SPI_DEVICE_COUNT);
        return s32Ret;
    }

    return 0;
}

static void __exit _mod_spi_exit(void)
{
    SPI_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&SysDev.cDevice);
    unregister_chrdev_region(MKDEV(SysDev.s32SysMajor, SysDev.s32SysMinor), MOD_SPI_DEVICE_COUNT);
}

static int _mod_spi_open (struct inode *inode, struct file *filp)
{
    SPI_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static int _mod_spi_release(struct inode *inode, struct file *filp)
{
    SPI_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static int _mod_spi_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err= 0;

    // extract the type and number bitfields, and don¡¦t decode
    // wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
    if (_IOC_TYPE(cmd) != SPI_IOCTL_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > IOCTL_SPI_MAXNR) return -ENOTTY;

    // the direction is a bitmask, and VERIFY_WRITE catches R/W
    // transfers. ¡¥Type¡¦ is user oriented, while
    // access_ok is kernel oriented, so the concept of "read" and
    // "write" is reversed
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;


    PROBE_IO_ENTRY(MDRV_MAJOR_SPI, _IOC_NR(cmd));
    switch(cmd)
    {
    case IOCTL_SPI_INIT:
        err = MDrv_SerFlash_Init_IO();
        break;

    case IOCTL_SPI_READ:
        err = MDrv_SerFlash_Read_IO(arg);
        break;

    case IOCTL_SPI_WRITE:
        err = MDrv_SerFlash_Write_IO(arg);
        break;

    case IOCTL_SPI_ERASE_SEC:
        err = MDrv_SerFlash_EraseSec_IO(arg);
        break;

    default:
        SPI_WARNING("Unknown ioctl command %d\n", cmd);
        PROBE_IO_EXIT(MDRV_MAJOR_SPI, _IOC_NR(cmd));
        return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_SPI, _IOC_NR(cmd));
    return err;
}


U32 MDrv_SerFlash_Init_IO(void)
{
    MDrv_SerFlash_Init();
    return 0;
}

U32 MDrv_SerFlash_Read_IO(U32 arg)
{
    BOOL bRet = FALSE;
    IO_SPI_RW_t spi_rw;
    U32   u32Buf;

    if (copy_from_user(&spi_rw, (void __user *)arg, sizeof(IO_SPI_RW_t)))
    {
        return -EFAULT;
    }

    // address and size must be 4-byte aligned
    spi_rw.u32Addr &= ~3;
    spi_rw.u32Len &= ~3;

    while (spi_rw.u32Len > 0)
    {
        bRet = MDrv_SerFlash_Read(spi_rw.u32Addr, (U8*)&u32Buf, sizeof(U32));
        if (bRet == FALSE)
        {
            return -EFAULT;
        }
        if (copy_to_user(spi_rw.pu8Buf, &u32Buf, sizeof(U32)))
        {
            return -EFAULT;
        }
        spi_rw.pu8Buf+= sizeof(U32);
        spi_rw.u32Addr+= sizeof(U32);
        spi_rw.u32Len-= sizeof(U32);
    }

    return 0;
}

U32 MDrv_SerFlash_Write_IO(U32 arg)
{
    BOOL bRet = FALSE;
    IO_SPI_RW_t spi_rw;
    U32   u32Buf;

    // address and size must be 4-byte aligned

    if (copy_from_user(&spi_rw, (void __user *)arg, sizeof(IO_SPI_RW_t)))
    {
        return -EFAULT;
    }

    // address and size must be 4-byte aligned
    spi_rw.u32Addr &= ~3;
    spi_rw.u32Len &= ~3;

    while (spi_rw.u32Len > 0)
    {
        if (copy_from_user(&u32Buf, (void __user *)spi_rw.pu8Buf, sizeof(U32)))
        {
            return -EFAULT;
        }

        bRet = MDrv_SerFlash_Write(spi_rw.u32Addr, (U8*)&u32Buf, sizeof(U32));
        if (bRet == FALSE)
        {
            return -EFAULT;
        }
        spi_rw.pu8Buf+= sizeof(U32);
        spi_rw.u32Addr+= sizeof(U32);
        spi_rw.u32Len-= sizeof(U32);
    }

    return 0;

}

U32 MDrv_SerFlash_EraseSec_IO(U32 arg)
{
    BOOL bRet = FALSE;
    IO_SPI_RW_t spi_rw;

    // address and size must be 4-byte aligned

    if (copy_from_user(&spi_rw, (void __user *)arg, sizeof(IO_SPI_RW_t)))
    {
        return -EFAULT;
    }

    // address and size must be 4K-byte aligned
    spi_rw.u32Addr &= ~((1 << SEC_SIZE_BIT) - 1);
    spi_rw.u32Len &= ~((1 << SEC_SIZE_BIT) - 1);

    if (spi_rw.u32Len > 0)
    {
        bRet = MDrv_SerFlash_EraseSec(spi_rw.u32Addr >> SEC_SIZE_BIT, 
                                      (spi_rw.u32Addr + spi_rw.u32Len - 1) >> SEC_SIZE_BIT);
        if (bRet == FALSE)
        {
            return -EFAULT;
        }

    }

    return 0;
}

module_init(_mod_spi_init);
module_exit(_mod_spi_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("SPI driver");
MODULE_LICENSE("MSTAR");
