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
/// file    mdrv_nand_io.c
/// @brief  nand Control Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/mtd/mtd_info.h>

#include "mst_devid.h"
#include "mdrv_types.h"
#include "mdrv_nand_io.h"
#include "drvNAND.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define NAND_PRINT(fmt, args...)	//printk("[NAND][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_NAND_DEVICE_COUNT		1

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int			_mod_nand_open (struct inode *inode, struct file *filp);
static int			_mod_nand_release(struct inode *inode, struct file *filp);
static ssize_t		_mod_nand_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t		_mod_nand_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int	_mod_nand_poll(struct file *filp, poll_table *wait);
static int			_mod_nand_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int			_mod_nand_fasync(int fd, struct file *filp, int mode);

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
	int						s32NandMajor;
	int						s32NandMinor;
	struct cdev				cDevice;
	struct file_operations	NandFop;
	struct fasync_struct	*async_queue; /* asynchronous readers */

} NandModHandle;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
NandModHandle NandDev =
{
	.s32NandMajor =	MDRV_MAJOR_NAND,
	.s32NandMinor =	MDRV_MINOR_NAND,

	.cDevice =
	{
		.kobj =
		{
			.name =	MDRV_NAME_NAND,
		},
		.owner =	THIS_MODULE,
	},

	.NandFop=
	{
		.open =		_mod_nand_open,
		.release =	_mod_nand_release,
		.read =		_mod_nand_read,
		.write =	_mod_nand_write,
		.poll =		_mod_nand_poll,
		.ioctl =	_mod_nand_ioctl,
		.fasync =	_mod_nand_fasync,
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
static int _mod_nand_open (struct inode *inode, struct file *filp)
{
	NandModHandle *dev;

	NAND_PRINT("%s is invoked\n", __FUNCTION__);

	dev = container_of(inode->i_cdev, NandModHandle, cDevice);
	filp->private_data = dev;

	return 0;
}

static int _mod_nand_release(struct inode *inode, struct file *filp)
{
	NAND_PRINT("%s is invoked\n", __FUNCTION__);
	return 0;
}

static ssize_t _mod_nand_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	// remove it if it's not required
	NAND_PRINT("%s is invoked\n", __FUNCTION__);
	return 0;
}

static ssize_t _mod_nand_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	// remove it if it's not required
	NAND_PRINT("%s is invoked\n", __FUNCTION__);
	return 0;
}

static unsigned int _mod_nand_poll(struct file *filp, poll_table *wait)
{
	NAND_PRINT("%s is invoked\n", __FUNCTION__);
	return 0;
}

static int _mod_nand_fasync(int fd, struct file *filp, int mode)
{
	NAND_PRINT("%s is invoked\n", __FUNCTION__);

	return fasync_helper(fd, filp, mode, &NandDev.async_queue);
}

static int _mod_nand_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	S32 s32Err = 0;
	U8	OnOff_flag;
	U16	u16NandResult;

	NAND_PRINT("%s is invoked\n", __FUNCTION__);
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if ( (NAND_IOC_MAGIC != _IOC_TYPE(cmd)) || (_IOC_NR(cmd) > NAND_IOC_MAXNR) )
	{
		return -ENOTTY;
	}

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if ( _IOC_DIR(cmd) & _IOC_READ )
	{
		s32Err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if ( _IOC_DIR(cmd) & _IOC_WRITE )
	{
		s32Err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}
	if (s32Err)
	{
		return -EFAULT;
	}

    PROBE_IO_ENTRY(MDRV_MAJOR_NAND, _IOC_NR(cmd));

	switch(cmd)
	{
		case MDRV_NAND_INIT:
			// The nand driver is build-in in kernel.
			NAND_PRINT("ioctl: MDRV_NAND_INIT\n");
			break;

		case MDRV_NAND_POWER_ON_OFF:
			NAND_PRINT("ioctl: MDRV_NAND_POWER_ON_OFF\n");
			if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
			{
			    PROBE_IO_EXIT(MDRV_MAJOR_NAND, _IOC_NR(cmd));
				return -EFAULT;
            }

			u16NandResult = MDrv_NAND_Set_Power_Off(OnOff_flag);

			if (copy_to_user((U8  __user *) arg, &OnOff_flag, sizeof(U8)))
			{
			    PROBE_IO_EXIT(MDRV_MAJOR_NAND, _IOC_NR(cmd));
				return -EFAULT;
            }
			break;

		//dhjung LGE
		case MDRV_NAND_MTD_ADD:
			NAND_PRINT("ioctl: MDRV_NAND_MTD_ADD\n");
			get_mtd_partitions(nand_mtd);
			break;

		default:
			NAND_PRINT("ioctl: unknown command\n");
			PROBE_IO_EXIT(MDRV_MAJOR_NAND, _IOC_NR(cmd));
			return -ENOTTY;
	}


    PROBE_IO_EXIT(MDRV_MAJOR_NAND, _IOC_NR(cmd));
	return 0;
}

static int __init mod_nand_init(void)
{
	S32		s32Ret;
	dev_t	dev;

	printk("%s is invoked\n", __FUNCTION__);

	if (NandDev.s32NandMajor)
	{
		dev = MKDEV(NandDev.s32NandMajor, NandDev.s32NandMinor);
		s32Ret = register_chrdev_region(dev, MOD_NAND_DEVICE_COUNT, MDRV_NAME_NAND);
	}
	else
	{
		s32Ret = alloc_chrdev_region(&dev, NandDev.s32NandMinor, MOD_NAND_DEVICE_COUNT, MDRV_NAME_NAND);
		NandDev.s32NandMajor = MAJOR(dev);
	}

	if ( s32Ret < 0 )
	{
		NAND_PRINT("Unable to get major %d\n", NandDev.s32NandMajor);
		return s32Ret;
	}

	cdev_init(&NandDev.cDevice, &NandDev.NandFop);

	if ( (s32Ret = cdev_add(&NandDev.cDevice, dev, MOD_NAND_DEVICE_COUNT)) != 0 )
	{
		NAND_PRINT("Unable add a character device\n");
		unregister_chrdev_region(dev, MOD_NAND_DEVICE_COUNT);
		return s32Ret;
	}

	return 0;
}

static void __exit mod_nand_exit(void)
{
	NAND_PRINT("%s is invoked\n", __FUNCTION__);

	cdev_del(&NandDev.cDevice);
	unregister_chrdev_region(MKDEV(NandDev.s32NandMajor, NandDev.s32NandMinor), MOD_NAND_DEVICE_COUNT);
}

module_init(mod_nand_init);
module_exit(mod_nand_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("NAND IO driver");
MODULE_LICENSE("MSTAR");
