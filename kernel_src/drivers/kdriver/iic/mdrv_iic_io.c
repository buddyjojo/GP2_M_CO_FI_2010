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
/// file    mdrv_iic.c
/// @brief  IIC Driver Interface
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
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mst_devid.h"

#include "mdrv_iic_io.h"
#include "mhal_iic_reg.h"
#include "mdrv_iic.h"
#include "mhal_iic.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_VENUS_BOARD_DVB_1 1
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1 1
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_DVB_1 1
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERVA_BOARD_ATSC_1 1
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_PDP_BOARD_DVB_1 1

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define IIC_MUTEX_WAIT_TIME         3000

#define IIC_DBG_ENABLE              0

#if IIC_DBG_ENABLE
#define IIC_DBG(_f)                 (_f)
#else
#define IIC_DBG(_f)
#endif

#if 0
#define LINE_DBG()                  printf("IIC %d\n", __LINE__)
#else
#define LINE_DBG()
#endif

#define IIC_PRINT(fmt, args...)		//printk("[IIC][%05d] " fmt, __LINE__, ## args)

typedef struct
{
    S32                         s32MajorIIC;
    S32                         s32MinorIIC;
    struct cdev                 cDevice;
    struct file_operations      IICFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} IIC_ModHandle_t;


#define MOD_IIC_DEVICE_COUNT         1
#define MOD_IIC_NAME                 "ModIIC"


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _MDrv_IIC_Open (struct inode *inode, struct file *filp);
static int                      _MDrv_IIC_Release(struct inode *inode, struct file *filp);
static ssize_t                  _MDrv_IIC_Read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _MDrv_IIC_Write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _MDrv_IIC_Poll(struct file *filp, poll_table *wait);
static int                      _MDrv_IIC_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _MDrv_IIC_Fasync(int fd, struct file *filp, int mode);

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

static IIC_ModHandle_t IICDev=
{
    .s32MajorIIC = MDRV_MAJOR_IIC,
    .s32MinorIIC = MDRV_MINOR_IIC,
    .cDevice =
    {
        .kobj = {.name= MOD_IIC_NAME, },
        .owner = THIS_MODULE,
    },
    .IICFop =
    {
        .open =     _MDrv_IIC_Open,
        .release =  _MDrv_IIC_Release,
        .read =     _MDrv_IIC_Read,
        .write =    _MDrv_IIC_Write,
        .poll =     _MDrv_IIC_Poll,
        .ioctl =    _MDrv_IIC_Ioctl,
        .fasync =   _MDrv_IIC_Fasync,
    },
};

//dhjung LGE
#if 0
static IIC_Param_t IIC_param;
static U8 u8BufIIC[IIC_RW_BUF_SIZE];
#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

static int _MDrv_IIC_Open (struct inode *inode, struct file *filp)
{
	IIC_ModHandle_t *dev;

    IIC_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, IIC_ModHandle_t, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _MDrv_IIC_Release(struct inode *inode, struct file *filp)
{
    IIC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _MDrv_IIC_Read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    U32     u32RetCountIIC = 0;
//dhjung LGE
#if 0
    U32     u32AddrIIC = 0;
    U8      u8I;
    U8      u8OffsetIIC[4];

    IIC_PRINT("%s is invoked\n", __FUNCTION__);

	for (u8I=0; u8I< IIC_param.u8AddrSizeIIC; u8I++)
	{
		u32AddrIIC = (u32AddrIIC << 8) |IIC_param.u8AddrIIC[u8I];
		u8OffsetIIC[u8I] =IIC_param.u8AddrIIC[u8I];
	}

	switch(IIC_param.u8IdIIC)
	{
		case 1:
			u32RetCountIIC = MDrv_HW_IIC_Read(IIC_param.u8SlaveIdIIC,IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count,u8BufIIC);
			break;
		case 2:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_param.u8SlaveIdIIC,IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count,u8BufIIC);
			break;
		case 3:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_param.u8SlaveIdIIC,IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count,u8BufIIC);
			break;
		case 4:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_param.u8SlaveIdIIC,IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count,u8BufIIC);
			break;
		case 5:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_param.u8SlaveIdIIC,IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count,u8BufIIC);
			break;
		case 6:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_param.u8SlaveIdIIC,IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count,u8BufIIC);
			break;
		default:
			printk("_MDrv_IIC_Read():ERROR: invalid IIC ID\n");
			return(-1);
	}

	IIC_DBG(printk("_MDrv_IIC_Read() -> u32RetCountIIC=%d\n", u32RetCountIIC));

	if (u32RetCountIIC)
	{
		if( copy_to_user(buf, u8BufIIC, u32RetCountIIC) )
			return(-1);
	}
#else
	IIC_Param_t IIC_ReadParam;
	U8        	pRdBuffer[IIC_RD_BUF_SIZE];

	if( copy_from_user(&IIC_ReadParam, (IIC_Param_t *)buf, count) )
	{
		return -1;
	}

	if( IIC_ReadParam.u32DataSizeIIC > IIC_RD_BUF_SIZE )
		IIC_ReadParam.u8pbufIIC = kmalloc(IIC_ReadParam.u32DataSizeIIC, GFP_KERNEL);
	else
		IIC_ReadParam.u8pbufIIC = pRdBuffer;

	if( IIC_ReadParam.u8pbufIIC == NULL )
	{    // check the result of kmalloc (dreamer@lge.com)
		return -1;
	}

#if 1	//	modified by LGE(dreamer@lge.com)
	if( IIC_ReadParam.u8IdIIC - 1 >= IIC_NUM_OF_MAX )
	{
		printk("_MDrv_IIC_Read():ERROR: invalid IIC ID\n");
		if( IIC_ReadParam.u8pbufIIC != pRdBuffer )	kfree(IIC_ReadParam.u8pbufIIC);
		return(-1);
	}
	else if( IIC_ReadParam.u8IdIIC == IIC_NUM_OF_HW )
	{
		u32RetCountIIC = MDrv_HW_IIC_Read(IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
	}
	else
	{
		u32RetCountIIC = MDrv_SW_IIC_Read(IIC_ReadParam.u8IdIIC,IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
	}
#else
	switch(IIC_ReadParam.u8IdIIC)
	{
		case 1:
			u32RetCountIIC = MDrv_HW_IIC_Read(IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
			break;
		case 2:
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_ReadParam.u8IdIIC,IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
			break;
		case 3:
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_ReadParam.u8IdIIC,IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
			break;
		case 4:
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_ReadParam.u8IdIIC,IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
			break;
		case 5:
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_ReadParam.u8IdIIC,IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
			break;
		case 6:
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_ReadParam.u8IdIIC,IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
			break;
		case 7:
			u32RetCountIIC = MDrv_SW_IIC_Read(IIC_ReadParam.u8IdIIC,IIC_ReadParam.u8SlaveIdIIC,IIC_ReadParam.u8AddrSizeIIC, IIC_ReadParam.u8AddrIIC, IIC_ReadParam.u32DataSizeIIC,IIC_ReadParam.u8pbufIIC);
			break;
		default:
			printk("_MDrv_IIC_Read():ERROR: invalid IIC ID\n");
			if( IIC_ReadParam.u8pbufIIC != pRdBuffer )	kfree(IIC_ReadParam.u8pbufIIC);
			return(-1);
	}
#endif

	IIC_DBG(printk("_MDrv_IIC_Read() -> u32RetCountIIC=%d\n", u32RetCountIIC));

	if (u32RetCountIIC)
	{
		if( copy_to_user(((IIC_Param_t *)buf)->u8pbufIIC, IIC_ReadParam.u8pbufIIC, IIC_ReadParam.u32DataSizeIIC) )
		{    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
			if( IIC_ReadParam.u8pbufIIC != pRdBuffer )	kfree(IIC_ReadParam.u8pbufIIC);
			return(-1);
		}
	}

	if( IIC_ReadParam.u8pbufIIC != pRdBuffer )	kfree(IIC_ReadParam.u8pbufIIC);
#endif

    return u32RetCountIIC;
}

static ssize_t _MDrv_IIC_Write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    U32     u32RetCountIIC = 0;
#if 0
    U32     u32AddrIIC = 0;
    U8      u8I;
    U8      u8OffsetIIC[4];

    IIC_PRINT("%s is invoked\n", __FUNCTION__);

	if (count> IIC_RW_BUF_SIZE)
	{    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
		if( copy_from_user(u8BufIIC, buf, IIC_RW_BUF_SIZE) )
		    return -1;
	}
	else
	{    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
		if( copy_from_user(u8BufIIC, buf, count) )
		    return -1;
	}

	for (u8I=0; u8I< IIC_param.u8AddrSizeIIC; u8I++)
	{
		u32AddrIIC = (u32AddrIIC << 8) |IIC_param.u8AddrIIC[u8I];
		u8OffsetIIC[u8I] =IIC_param.u8AddrIIC[u8I];
	}

	switch(IIC_param.u8IdIIC)
	{
		case 1:
			u32RetCountIIC = MDrv_HW_IIC_Write(IIC_param.u8SlaveIdIIC, IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count, u8BufIIC);
			break;
		case 2:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_param.u8SlaveIdIIC, IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count, u8BufIIC);
			break;
		case 3:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_param.u8SlaveIdIIC, IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count, u8BufIIC);
			break;
		case 4:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_param.u8SlaveIdIIC, IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count, u8BufIIC);
			break;
		case 5:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_param.u8SlaveIdIIC, IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count, u8BufIIC);
			break;
		case 6:
			MDrv_SW_IIC_SelectBus(IIC_param.u8IdIIC);
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_param.u8SlaveIdIIC, IIC_param.u8AddrSizeIIC, u8OffsetIIC, (count> IIC_RW_BUF_SIZE)? IIC_RW_BUF_SIZE:count, u8BufIIC);
			break;
		default:
			printk("_MDrv_IIC_Write():ERROR: invalid IIC ID\n");
			return(-1);
	}

    IIC_DBG(printk("_MDrv_IIC_Write() --> u32RetCountIIC=%d\n", u32RetCountIIC));

#else
	IIC_Param_t IIC_WriteParam;
	U8        	pWrBuffer[IIC_WR_BUF_SIZE];

	if( copy_from_user(&IIC_WriteParam, (IIC_Param_t *)buf, count) )
	{    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
		return -1;
	}

	if( IIC_WriteParam.u32DataSizeIIC > IIC_WR_BUF_SIZE )
		IIC_WriteParam.u8pbufIIC = kmalloc(IIC_WriteParam.u32DataSizeIIC, GFP_KERNEL);
	else
		IIC_WriteParam.u8pbufIIC = pWrBuffer;

	if( IIC_WriteParam.u8pbufIIC == NULL )
	{    // // check the result of kmalloc (dreamer@lge.com)
		return -1;
	}

	if( copy_from_user(IIC_WriteParam.u8pbufIIC, ((IIC_Param_t *)buf)->u8pbufIIC, IIC_WriteParam.u32DataSizeIIC) )
	{    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
		if( IIC_WriteParam.u8pbufIIC != pWrBuffer )	kfree(IIC_WriteParam.u8pbufIIC);
		return -1;
	}

#if 1	//	modified by LGE(dreamer@lge.com)
	if( IIC_WriteParam.u8IdIIC - 1 >= IIC_NUM_OF_MAX )
	{
		printk("_MDrv_IIC_Write():ERROR: invalid IIC ID\n");
		if( IIC_WriteParam.u8pbufIIC != pWrBuffer )	kfree(IIC_WriteParam.u8pbufIIC);
		return(-1);
	}
	else if( IIC_WriteParam.u8IdIIC == IIC_NUM_OF_HW )
	{
		u32RetCountIIC = MDrv_HW_IIC_Write(IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
	}
	else
	{
		u32RetCountIIC = MDrv_SW_IIC_Write(IIC_WriteParam.u8IdIIC, IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
	}
#else
	switch(IIC_WriteParam.u8IdIIC)
	{
		case 1:
			u32RetCountIIC = MDrv_HW_IIC_Write(IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
			break;
		case 2:
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_WriteParam.u8IdIIC, IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
			break;
		case 3:
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_WriteParam.u8IdIIC, IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
			break;
		case 4:
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_WriteParam.u8IdIIC, IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
			break;
		case 5:
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_WriteParam.u8IdIIC, IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
			break;
		case 6:
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_WriteParam.u8IdIIC, IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
			break;
		case 7:
			u32RetCountIIC = MDrv_SW_IIC_Write(IIC_WriteParam.u8IdIIC, IIC_WriteParam.u8SlaveIdIIC, IIC_WriteParam.u8AddrSizeIIC, IIC_WriteParam.u8AddrIIC, IIC_WriteParam.u32DataSizeIIC, IIC_WriteParam.u8pbufIIC);
			break;
		default:
			printk("_MDrv_IIC_Write():ERROR: invalid IIC ID\n");
			if( IIC_WriteParam.u8pbufIIC != pWrBuffer )	kfree(IIC_WriteParam.u8pbufIIC);
			return(-1);
	}
#endif
	if( IIC_WriteParam.u8pbufIIC != pWrBuffer )	kfree(IIC_WriteParam.u8pbufIIC);
#endif

    return u32RetCountIIC;
}

static unsigned int _MDrv_IIC_Poll(struct file *filp, poll_table *wait)
{
    IIC_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _MDrv_IIC_Fasync(int fd, struct file *filp, int mode)
{
    IIC_PRINT("%s is invoked\n", __FUNCTION__);

	return 0;//fasync_helper(fd, filp, mode, &IICDev.async_queue);
}

static int _MDrv_IIC_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err= 0;
	//dhjung LGE
	IIC_Param_t IIC_param;

    IIC_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((IIC_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> IIC_IOC_MAXNR))
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


    PROBE_IO_ENTRY(MDRV_MAJOR_IIC, _IOC_NR(cmd));

    switch(cmd)
    {
        case MDRV_IIC_INIT:
            IIC_PRINT("ioctl: MDRV_IIC_INIT\n");
            MDrv_IIC_Init();
            break;
        case MDRV_IIC_CLOCK:
            IIC_PRINT("ioctl: MDRV_IIC_CLOCK\n");
            if (copy_from_user(&IIC_param, (IIC_Param_t __user *)arg, sizeof(IIC_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_IIC, _IOC_NR(cmd));
 	            return -EFAULT;
            }

#if 1	//	modified by LGE(dreamer@lge.com)
			if( IIC_param.u8IdIIC - 1 >= IIC_NUM_OF_MAX )
			{
			    PROBE_IO_EXIT(MDRV_MAJOR_IIC, _IOC_NR(cmd));
				return -EFAULT;
			}
			else if( IIC_param.u8IdIIC == IIC_NUM_OF_HW )
			{
				MDrv_HW_IIC_Clock_Select(IIC_param.u8ClockIIC);
			}
			else
			{
				MDrv_SW_IIC_SetSpeed(IIC_param.u8IdIIC, IIC_param.u8ClockIIC);
			}
#else
            switch(IIC_param.u8IdIIC)
            {
                case 1:
                    MDrv_HW_IIC_Clock_Select(IIC_param.u8ClockIIC);
                    break;
                case 2:
                    MDrv_SW_IIC_SetSpeed(IIC_param.u8IdIIC, IIC_param.u8ClockIIC);
                    break;
                case 3:
                    MDrv_SW_IIC_SetSpeed(IIC_param.u8IdIIC, IIC_param.u8ClockIIC);
                    break;
                case 4:
                    MDrv_SW_IIC_SetSpeed(IIC_param.u8IdIIC, IIC_param.u8ClockIIC);
                    break;
                case 5:
                    MDrv_SW_IIC_SetSpeed(IIC_param.u8IdIIC, IIC_param.u8ClockIIC);
                    break;
                case 6:
                    MDrv_SW_IIC_SetSpeed(IIC_param.u8IdIIC, IIC_param.u8ClockIIC);
                    break;
                case 7:
                    MDrv_SW_IIC_SetSpeed(IIC_param.u8IdIIC, IIC_param.u8ClockIIC);
                    break;
            }
#endif
            break;
		//dhjung LGE
		#if 0
        case MDRV_IIC_SET_PARAM:
            IIC_PRINT("ioctl: MDRV_IIC_SET_PARAM\n");
        	if( copy_from_user(&IIC_param, (IIC_Param_t __user *)arg, sizeof(IIC_Param_t)))
				return -EFAULT;

            IIC_PRINT("u8SlaveIdIIC = %x\n", IIC_param.u8SlaveIdIIC);
            IIC_PRINT("u8AddrIIC[0] = %x\n", IIC_param.u8AddrIIC[0]);
            IIC_PRINT("u8AddrIIC[1] = %x\n", IIC_param.u8AddrIIC[1]);
            IIC_PRINT("u8AddrIIC[2] = %x\n", IIC_param.u8AddrIIC[2]);
            IIC_PRINT("u8AddrIIC[3] = %x\n", IIC_param.u8AddrIIC[3]);
            IIC_PRINT("u8AddrSizeIIC = %x\n", IIC_param.u8AddrSizeIIC);
            break;
		#endif

		// added for RGB EDID by LGE(dreamer@lge.com)
		case MDRV_IIC_ENABLE:
			IIC_PRINT("ioctl: MDRV_IIC_ENABLE\n");
			if (copy_from_user(&IIC_param, (IIC_Param_t __user *)arg, sizeof(IIC_Param_t)))
			{
			    PROBE_IO_EXIT(MDRV_MAJOR_IIC, _IOC_NR(cmd));
				return -EFAULT;
            }

			switch(IIC_param.u8IdIIC)
			{

                case IIC_NUM_OF_NEC_MICOM:
                case IIC_NUM_OF_AUDIO_AMP:
				case IIC_NUM_OF_RGB_EDID:
				case IIC_NUM_OF_HDMI_A_EDID:
				case IIC_NUM_OF_HDMI_B_EDID:
				case IIC_NUM_OF_HDMI_C_EDID:
				case IIC_NUM_OF_HDMI_D_EDID:
					MDrv_SW_IIC_Enable( IIC_param.u8IdIIC, ((IIC_param.u8ClockIIC) ? TRUE : FALSE) );
					break;
				default:
					break;
			}
			break;

        default:
            IIC_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_IIC, _IOC_NR(cmd));
            return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_IIC, _IOC_NR(cmd));
    return 0;
}

static int __init mod_iic_init(void)
{
    S32         s32Ret;
    dev_t       dev;

    printk("%s is invoked\n", __FUNCTION__);

    if (IICDev.s32MajorIIC)
    {
        dev = MKDEV(IICDev.s32MajorIIC, IICDev.s32MinorIIC);
        s32Ret = register_chrdev_region(dev, MOD_IIC_DEVICE_COUNT, MOD_IIC_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, IICDev.s32MinorIIC, MOD_IIC_DEVICE_COUNT, MOD_IIC_NAME);
        IICDev.s32MajorIIC = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        IIC_PRINT("Unable to get major %d\n", IICDev.s32MajorIIC);
        return s32Ret;
    }

    cdev_init(&IICDev.cDevice, &IICDev.IICFop);
    if (0!= (s32Ret= cdev_add(&IICDev.cDevice, dev, MOD_IIC_DEVICE_COUNT)))
    {
        IIC_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_IIC_DEVICE_COUNT);
        return s32Ret;
    }

    return 0;
}

static void __exit mod_iic_exit(void)
{
    IIC_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&IICDev.cDevice);
    unregister_chrdev_region(MKDEV(IICDev.s32MajorIIC, IICDev.s32MinorIIC), MOD_IIC_DEVICE_COUNT);
}

module_init(mod_iic_init);
module_exit(mod_iic_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("IIC driver");
MODULE_LICENSE("MSTAR");
