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
/// file    drvIR.c
/// @brief  IR Control Interface
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
#include <linux/time.h>  //added
#include <linux/timer.h> //added
#include <asm/io.h>

#include "mst_devid.h"
#include "mdrv_ir_io.h"
#include "mdrv_ir.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define REG(addr)                   (*(volatile u32 *)(addr))
#define IR_PRINT(fmt, args...)      //printk("[IR][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_IR_DEVICE_COUNT     1

//#define IR_DEBUG
#ifdef IR_DEBUG
#define DEBUG_IR(x) (x)
#else
#define DEBUG_IR(x)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _mod_ir_open (struct inode *inode, struct file *filp);
static int                      _mod_ir_release(struct inode *inode, struct file *filp);
static ssize_t                  _mod_ir_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _mod_ir_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _mod_ir_poll(struct file *filp, poll_table *wait);
static int                      _mod_ir_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _mod_ir_fasync(int fd, struct file *filp, int mode);


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
IRModHandle IRDev=
{
    .s32IRMajor=               MDRV_MAJOR_IR,
    .s32IRMinor=               MDRV_MINOR_IR,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_IR, },
        .owner  =               THIS_MODULE,
    },
    .IRFop=
    {
        .open=                  _mod_ir_open,
        .release=               _mod_ir_release,
        .read=                  _mod_ir_read,
        .write=                 _mod_ir_write,
        .poll=                  _mod_ir_poll,
        .ioctl=                 _mod_ir_ioctl,
        .fasync =	            _mod_ir_fasync,
    },
};

extern wait_queue_head_t	key_wait_q;

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
extern B16 MDrv_MICOM_ReceiveMailFromMicom(U8 *u8MBIndex, U16 *u16MBData); /* To avoid compilation warning msg. by LGE. jjab. */

//-------------------------------------------------------------------------------------------------
/// Initialize IR timing and enable IR.
/// @return None
//-------------------------------------------------------------------------------------------------

static int _mod_ir_open (struct inode *inode, struct file *filp)
{
	IRModHandle *dev;

    IR_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, IRModHandle, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _mod_ir_release(struct inode *inode, struct file *filp)
{
    IR_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_ir_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	U32 	eventValue = 0;
	U8  	pU8Key[IR_TV_LINK_DATA_NUM];
	U8  	u8Flag;
	//U8  	u8Index;
	//U16 	u16Data;
	BOOL	bRet;
	long	timeout;

/*	2008,12,03:	(dreamer@lge.com)
	IR_MODE_WB_CODE 모드에서 IR_MODE_NORMAL로 전환하는 과정의 과도기로 인해
	잘못된 REPEAT KEY 전달되는 문제 수정
	(POWER ONLY MODE 해제시 채널 UP 되는 문제 수정)
*/
	static	U16	_gU16IRSingle = (U16) -1;


	IR_READ_PARAM_T	irParam;

	IR_PRINT("%s is invoked\n", __FUNCTION__);

	if( copy_from_user( &irParam, (IR_READ_PARAM_T*)buf, sizeof( IR_READ_PARAM_T ) ))
	{    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
		return -EFAULT;
	}

	timeout = irParam.u32Timeout;
	//printk("IR PARAM: %d(%x)\n",  timeout, irParam.u32Timeout );

	irParam.u32Timeout = interruptible_sleep_on_timeout(&key_wait_q, timeout);
	//printk("IR PARAM: %d(%x) end\n",  timeout, irParam.u32Timeout );

	/* Get IR Key */
#if 0
	bRet = MDrv_IR_GetKey( pU8Key, &u8Flag);
	if (bRet)
	{
		eventValue ++;

		irParam.u8IRType	=	((u8Flag) ? 0x2 : 0x1);
		irParam.u8IRValue	=	pU8Key[0];
	}
#endif

	bRet = MDrv_IR_GetKeyOfTVLink( pU8Key, &u8Flag);
	if (bRet)
	{
		eventValue ++;

		if( u8Flag == (_IR_READ_TYPE_WB_CODE - 1) )	/* WB CODE DATA */
		{
			irParam.u8IRType	=	_IR_READ_TYPE_WB_CODE;
			memcpy( &(irParam.u8TVLinkData[0]), pU8Key, IR_WB_CODE_DATA_NUM );

			_gU16IRSingle = (U16) -1;
		}
		else if( u8Flag == (_IR_READ_TYPE_TV_LINK - 1) )	/* TV LINK DATA */
		{
			irParam.u8IRType	=	_IR_READ_TYPE_TV_LINK;
			memcpy( &(irParam.u8TVLinkData[0]), pU8Key, IR_TV_LINK_DATA_NUM );

			_gU16IRSingle = (U16) -1;
		}
		else if( u8Flag == (_IR_READ_TYPE_REPEAT - 1) )	/* REPEAT KEY */
		{
			if( pU8Key[0] == _gU16IRSingle )
			{
				irParam.u8IRType	=	_IR_READ_TYPE_REPEAT;
				irParam.u8IRValue	=	pU8Key[0];
			}
			else
			{
				printk("[IR] INVALID(%02x %04x)\n", pU8Key[0], _gU16IRSingle );
				eventValue --;
			}
		}
		else	/* SINGLE KEY */
		{
			irParam.u8IRType	=	_IR_READ_TYPE_SINGLE;
			irParam.u8IRValue	=	pU8Key[0];

			_gU16IRSingle = pU8Key[0];
		}
	}

	/* Get Local Key */
/*	
	bRet = MDrv_MICOM_ReceiveMailFromMicom(&u8Index, &u16Data);
	if (bRet)
	{
		if( u8Index == 2 )
		{
			eventValue ++;

			//<080805 Leehc> Locak Key 값 잘못 읽히는 문제 수정함.
			// start 080805
			//eventValue	= (u8Index) << MICOM_EVENT_SHIFT_LOCAL_VALUE;
			//eventValue += (((u16Data) ? 0x2 : 0x1)	<< MICOM_EVENT_SHIFT_LOCAL_TYPE);
			//eventValue += ((U8)u16Data) << MICOM_EVENT_SHIFT_LOCAL_VALUE;
			//eventValue += ( (( (U8)(u16Data >> 8) ) ? 0x2 : 0x1)	<< MICOM_EVENT_SHIFT_LOCAL_TYPE);
			// end 080805

			irParam.u8KeyType	=	(((U8)(u16Data >> 8)) ? _IR_READ_TYPE_REPEAT : _IR_READ_TYPE_SINGLE);
			irParam.u8KeyValue	=	 ((U8) u16Data & 0xFF);

		}
		else
		{
			printk("MAILBOX: %d:%x\n",  u8Index, u16Data );
		}
	}
*/

	if( eventValue > 0 )
	{
		if( copy_to_user( buf, &irParam, sizeof( IR_READ_PARAM_T ) ) == 0)
		{    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
			return 0;
		}
	}

    return -ENOTTY;
}

static ssize_t _mod_ir_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    IR_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_ir_poll(struct file *filp, poll_table *wait)
{
    IR_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _mod_ir_fasync(int fd, struct file *filp, int mode)
{
    IR_PRINT("%s is invoked\n", __FUNCTION__);

	return fasync_helper(fd, filp, mode, &IRDev.async_queue);
}

static int _mod_ir_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err= 0;
    MS_IR_DelayTime delaytime;
#ifdef NOT_USED_4_LGE
    int retval = 0;
    MS_IR_KeyValue keyvalue;
    U8 bEnableIR;
    MS_IR_LastKeyTime keytime;
    MS_IR_KeyInfo keyinfo;
#endif

    IR_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((IR_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> IR_IOC_MAXNR))
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
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (err)
    {
        return -EFAULT;
    }


    PROBE_IO_ENTRY(MDRV_MAJOR_IR, _IOC_NR(cmd));

    switch(cmd)
    {
        #if 0
        case IR_IOC:
            IR_PRINT("ioctl: receive IR_IOC\n");
            break;
        case IR_IOCREAD:
            IR_PRINT("ioctl: receive IR_IOCREAD with user pointer 0x%08x 0x%08x\n", (u32)arg, *((u32*)(arg)));
            ptrData = (PMS_IR_KDATA)arg;
            IR_PRINT("IR_IOCREAD (1) -> data1=%d, data2=%d, data3=%d\n", ptrData->data1, ptrData->data2, ptrData->data3);
            ptrData->data1 = 11;
            ptrData->data2 = 22;
            ptrData->data3 = 33;
            IR_PRINT("IR_IOCREAD (2) -> data1=%d, data2=%d, data3=%d\n", ptrData->data1, ptrData->data2, ptrData->data3);
            break;
        case IR_IOCWRITE:
            IR_PRINT("ioctl: receive IR_IOCWRITE with user pointer 0x%08x 0x%08x\n", (u32)arg, *((u32*)(arg)));
            break;
        case IR_IOCRW:
            IR_PRINT("ioctl: receive IR_IOCRW with user pointer 0x%08x 0x%08x\n", (u32)arg, *((u32*)(arg)));
            ptrData = (PMS_IR_KDATA)arg;
            IR_PRINT("IR_IOCREAD (1) -> data1=%d, data2=%d, data3=%d\n", ptrData->data1, ptrData->data2, ptrData->data3);
            ptrData->data1 = 55;
            ptrData->data2 = 66;
            ptrData->data3 = 77;
            IR_PRINT("IR_IOCREAD (2) -> data1=%d, data2=%d, data3=%d\n", ptrData->data1, ptrData->data2, ptrData->data3);
            break;

            break;
        #endif

        case MDRV_IR_INIT:
            MDrv_IR_Init();
            break;

        case MDRV_IR_SET_DELAYTIME:
            IR_PRINT("ioctl: MDRV_IR_SET_DELAYTIME\n");
#ifdef NOT_USED_4_LGE
    		retval = __get_user(delaytime.u32_1stDelayTimeMs, &(((MS_IR_DelayTime __user *)arg)->u32_1stDelayTimeMs));
    		retval = __get_user(delaytime.u32_2ndDelayTimeMs, &(((MS_IR_DelayTime __user *)arg)->u32_2ndDelayTimeMs));
#else
            if( copy_from_user( &delaytime, (MS_IR_DelayTime*)arg, sizeof( MS_IR_DelayTime ) ))
            {    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
                PROBE_IO_EXIT(MDRV_MAJOR_IR, _IOC_NR(cmd));
                return -EFAULT;
            }
#endif
            MDrv_IR_SetDelayTime(delaytime.u32_1stDelayTimeMs, delaytime.u32_2ndDelayTimeMs);
            break;

#ifdef NOT_USED_4_LGE
        case MDRV_IR_GET_KEY:
            IR_PRINT("ioctl: MDRV_IR_GET_KEY\n");
            //TBD: need mutex here to protect data
            keyinfo.u8Valid = MDrv_IR_GetKey(&keyinfo.u8Key, &keyinfo.u8Flag);
            retval = __put_user(keyinfo.u8Key, &(((MS_IR_KeyInfo __user *)arg)->u8Key));
            retval = __put_user(keyinfo.u8Flag, &(((MS_IR_KeyInfo __user *)arg)->u8Flag));
            retval = __put_user(keyinfo.u8Valid, &(((MS_IR_KeyInfo __user *)arg)->u8Valid));
            break;
        case MDRV_IR_GET_LASTKEYTIME:
            IR_PRINT("ioctl: MDRV_IR_GET_LASTKEYTIME\n");
            //retval = __get_user(keytime.time, &(((MS_IR_LastKeyTime __user *)arg)->time));
            keytime.time = MDrv_IR_GetLastKeyTime();
            retval = __put_user(keytime.time, &(((MS_IR_LastKeyTime __user *)arg)->time));
            break;
        case MDRV_IR_PARSE_KEY:
            IR_PRINT("ioctl: MDRV_IR_PARSE_KEY\n");
            retval = __get_user(keyvalue.u8KeyIn, &(((MS_IR_KeyValue __user *)arg)->u8KeyIn));
            keyvalue.u8KeyOut = MDrv_IR_ParseKey(keyvalue.u8KeyIn);
            retval = __put_user(keyvalue.u8KeyOut, &(((MS_IR_KeyValue __user *)arg)->u8KeyOut));
            break;
        case MDRV_IR_TEST:
            IR_PRINT("ioctl: MDRV_IR_TEST\n");
            break;
        case MDRV_IR_ENABLE_IR:
            IR_PRINT("ioctl: MDRV_IR_ENABLE_IR\n");
            retval = __get_user(bEnableIR, (int __user *)arg);
            MDrv_IR_EnableIR(bEnableIR);
            break;
#endif	/* #ifdef	NOT_USED_4_LGE */

/* added by dreamer@lge.com */
		case MDRV_IR_WAKE_UP:
			wake_up_interruptible(&key_wait_q);
			break;

		case MDRV_IR_CONFIG:
			{
				U32	mode = (U32) arg;
				U16	type;
				U16 value;

				type	= (U16) ((mode >> 16) & 0xFFFF);
				value	= (U16)	( mode        & 0xFFFF);
				IR_PRINT("ioctl: MDRV_IR_CONFIG %08x(%d,%d)\n", mode, type, value );
				MDrv_IR_Config( type, value );
			}
			break;
/* added by dreamer@lge.com -end - */

        default:
            IR_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_IR, _IOC_NR(cmd));
            return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_IR, _IOC_NR(cmd));
    return 0;
}

static int __init mod_ir_init(void)
{
    int         s32Ret;
    dev_t       dev;

    //IR_PRINT("\n\n HELLO --> IR \n\n");
    IR_PRINT("%s is invoked\n", __FUNCTION__);

    if (IRDev.s32IRMajor)
    {
        dev = MKDEV(IRDev.s32IRMajor, IRDev.s32IRMinor);
        s32Ret = register_chrdev_region(dev, MOD_IR_DEVICE_COUNT, MDRV_NAME_IR);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, IRDev.s32IRMinor, MOD_IR_DEVICE_COUNT, MDRV_NAME_IR);
        IRDev.s32IRMajor = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        IR_PRINT("Unable to get major %d\n", IRDev.s32IRMajor);
        return s32Ret;
    }

    cdev_init(&IRDev.cDevice, &IRDev.IRFop);
    if (0!= (s32Ret= cdev_add(&IRDev.cDevice, dev, MOD_IR_DEVICE_COUNT)))
    {
        IR_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_IR_DEVICE_COUNT);
        return s32Ret;
    }

    //MDrv_IR_Init();
    return 0;
}

static void __exit mod_ir_exit(void)
{
    IR_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&IRDev.cDevice);
    unregister_chrdev_region(MKDEV(IRDev.s32IRMajor, IRDev.s32IRMinor), MOD_IR_DEVICE_COUNT);
}

module_init(mod_ir_init);
module_exit(mod_ir_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("IR driver");
MODULE_LICENSE("MSTAR");
