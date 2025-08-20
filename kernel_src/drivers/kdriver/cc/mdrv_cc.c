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
#define _MDRV_CC_C
///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_cc.c
/// This file contains the Mstar driver  for Close Caption
/// @author MStar Semiconductor Inc.
/// @brief  Close Caption module
///////////////////////////////////////////////////////////////////////////////


#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/wait.h>


#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/page-flags.h>
#include <asm-mips/io.h>
#include <asm-mips/string.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>

#include "mst_devid.h"
#include "mhal_cc.h"
#include "mhal_cc_reg.h"
#include "mhal_cc_types.h"

#include "mdrv_types.h"

#include "mdrv_cc.h"

#include "mst_platform.h"
#include "mdrv_system.h"

#include "mdrv_probe.h"

/* device id */
static U32  _u32majorCC    = MDRV_MAJOR_CC;
module_param(_u32majorCC, int, MDRV_MINOR_CC);

U32  _u32RLDBaseAddr;
U32  _u32RLDBaseLength;

/* TTX device cdev */
static struct cdev      _stDevCC[1];
// -----------------------------------------------
//
// device functions
//

static int MDrv_CC_Open(struct inode *inode, struct file *filp)
{
    CC_KDBG("\n");
    CC_KDBG("open %d\n", _u32majorCC);
	//msAPI_VBI_Init();
	printk("MDrv_CC_Open :msAPI_VBI_Init finished\n");

    // change dram access mode
    // 0x371F, BIT7 must be always 1 (otherwise the DMA access related function could be error)
    _MHal_W1BM(BK_VBI_1F, 0x80, 0x80);

	printk("MDrv_CC_Open: Finished \n");
	return 0;
}


/* Release CC device */
static void MDrv_CC_IOCTRL_Release(void)
{
    MHal_CC_EnableInterrupt(0);
    MHal_CC_OnOffVBISlicer(0);

}

/* Release ttx device */
static int MDrv_CC_Release(struct inode *inode, struct file *filp)
{
    CC_KDBG("release %d\n", _u32majorCC);

    MDrv_CC_IOCTRL_Release();

	return 0;
}

/* Set up the cdev structure for vd device.  */
static void MDrv_CC_SetupCDev(struct cdev *dev, int minor,
		struct file_operations *fops)
{
	int err, devno = MKDEV(_u32majorCC, minor);

	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add (dev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		CC_KDBG("Error %d adding simple%d", err, minor);
}


#define CC_PATCH 0

#if CC_PATCH
#include <linux/timer.h>

static struct timer_list cc_timer;

void cc_copydata(U32 arg)
{
    static u8 packet_cnt = 0;
    static u8 pre_odd_flag = 0;
    static u8 pre_even_flag = 0;

    u8 pBuf[8]= {0,0,0,0,0,0,0,0};

    u8 u8Odd_Even = _MHal_R1B(BK_VBI_56);
    u8 u8CC_Data1 = _MHal_R1B(BK_VBI_57);
    u8 u8CC_Data2 = _MHal_R1B(BK_VBI_58);

    if (u8Odd_Even == 0)
    {
        pre_odd_flag = 0;
        pre_even_flag = 0;
    }

    if ((u8Odd_Even & 0x80) && (pre_odd_flag == 0))
    {
        pBuf[0] = u8CC_Data1;
        pBuf[1] = u8CC_Data2;
        pBuf[2] = (packet_cnt++)%31 + 1;
        pBuf[4] = 0x80;

        memcpy((void *)ioremap(_u32RLDBaseAddr + 8 * (pBuf[2] - 1), 8), (void *)pBuf, 8);
        pre_odd_flag = 1;
        pre_even_flag = 0;
    }

    if ((u8Odd_Even & 0x40) && (pre_even_flag == 0))
    {
        pBuf[0] = u8CC_Data1;
        pBuf[1] = u8CC_Data2;
        pBuf[2] = (packet_cnt++)%31 + 1;
        pBuf[4] = 0xC1;

        memcpy((void *)ioremap(_u32RLDBaseAddr + 8 * (pBuf[2] - 1), 8), (void *)pBuf, 8);
        pre_even_flag = 1;
        pre_odd_flag = 0;
    }

    mod_timer(&cc_timer, jiffies + HZ/100);
}
#endif

static int MDrv_CC_IOCtl(struct inode *inode, struct file *filp, U32 u32Cmd, unsigned long u32Arg)
{
    //U32         u32ReturnVal;
    //struct page *pageptr;
    //U32         i;
    U16         u16Ret;
    U32 u32Value;
    U8 u8Val;

    PROBE_IO_ENTRY(MDRV_MAJOR_CC, _IOC_NR(u32Cmd));

    switch(u32Cmd)
    {
        case IOCTL_CC_INIT:
		printk("IOCTL_CC_INIT\n ");
            if (__get_user(u32Value, (U32  __user *)u32Arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CC, _IOC_NR(u32Cmd));
                return EFAULT;
            }
            /* first step to stop sliver and stop interrupter */
            MHal_CC_EnableInterrupt(FALSE);
            MHal_CC_OnOffVBISlicer(FALSE);

            _u32RLDBaseAddr = u32Value;
            printk("BaseAddr %08x\n", _u32RLDBaseAddr);
	        _MHal_W1B(BK_VBI_5D,(_u32RLDBaseAddr >> 19) & 0xFF );
	        _MHal_W1B(BK_VBI_5E,(_u32RLDBaseAddr >> 11) & 0xFF );
	        _MHal_W1B(BK_VBI_5F,(_u32RLDBaseAddr >> 3) & 0xFF );

	        printk("set buffer len\n");
            /* set cc buffer length */
            //XBYTE[BK_VBI_5C] = CC_BUF_LEN_MAX;
	        _MHal_W1B(BK_VBI_5C,31);

	        printk("set buffer len\n");
            // CJ
            //XBYTE[BK_AFEC_6B] = XBYTE[BK_AFEC_6B] & 0xF7;
            u8Val=_MHal_R1B(BK_AFEC_6B) & 0xF7;
            _MHal_W1B(BK_AFEC_6B,u8Val);
	        printk("set constraint\n");
            // Set a constraint for CC patterns in case of wrong encoder's behavior

            //XBYTE[BK_VBI_4A] = (XBYTE[BK_VBI_4A]&0xf0) | 0x04;
            u8Val=(_MHal_R1B(BK_VBI_4A)&0xf0) | 0x04;
	        printk("test\n");
            _MHal_W1B(BK_VBI_4A,u8Val);
	        printk("test2\n");

	        //_MHal_W1B(BK_VBI_40,0x20);
	        //_MHal_W1B(BK_VBI_41,0x52);
            MHal_CC_InitVBI();

#if CC_PATCH
            {
                _MHal_W1B(BK_VBI_5D,((_u32RLDBaseAddr + 0x200) >> 19) & 0xFF );
                _MHal_W1B(BK_VBI_5E,((_u32RLDBaseAddr + 0x200) >> 11) & 0xFF );
                _MHal_W1B(BK_VBI_5F,((_u32RLDBaseAddr + 0x200) >> 3) & 0xFF );

                init_timer(&cc_timer);
                cc_timer.function = cc_copydata;
                cc_timer.data = 0;
                cc_timer.expires = jiffies + HZ/100;
                add_timer(&cc_timer);
            }
#endif
            break;
        case IOCTL_CC_RELEASE:
            MDrv_CC_IOCTRL_Release();
            break;
        case IOCTL_CC_ONOFF_SLICER:
            //if(_pu8VBIBufTTX != NULL)
                MHal_CC_OnOffVBISlicer(u32Arg);
            break;

        case IOCTL_CC_SET_VIDEOSTANDARD:
            MHal_CC_SetVideoSystem(u32Arg);
            break;

        case IOCTL_CC_INTERRUPT_ED:
            MHal_CC_EnableInterrupt((B16)u32Arg);
            break;

        case IOCTL_CC_UART_OFF:
            _MHal_W1BM(BYTE2REAL(0x1E3C), 0x10, _BIT4);
            break;

        case IOCTL_CC_GET_PACKET_COUNT:
            u16Ret = MHal_CC_GetPacketCount();
            if(copy_to_user((U16*)u32Arg, (const U16*)&u16Ret, sizeof(U16)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CC, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            break;


        case IOCTL_CC_REINIT:
            MHal_CC_InitVBI();
            break;

        case IOCTL_CC_SETSLICINGREGION:
            {
                CC_Slicing_Region_t stSlicingReg;
                if (copy_from_user(&stSlicingReg, (CC_Slicing_Region_t __user *) u32Arg, sizeof(CC_Slicing_Region_t)))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CC, _IOC_NR(u32Cmd));
                    return -EFAULT;
                }
                MHal_CC_SetSlicingRegion(stSlicingReg.u16TopLineStart,
                                         stSlicingReg.u16TopLineEnd,
                                         stSlicingReg.u16BtnLineStart,
                                         stSlicingReg.u16BtnLineEnd,
                                         stSlicingReg.IsNTSC);
            }
            break;
		case IOCTL_CC_SETSLICINGSYS:
			{
				VIDEOSTANDARD_TYPE system_type;
				if (copy_from_user(&system_type, (VIDEOSTANDARD_TYPE __user *) u32Arg, sizeof(VIDEOSTANDARD_TYPE)))
				{
					PROBE_IO_EXIT(MDRV_MAJOR_CC, _IOC_NR(u32Cmd));
					return -EFAULT;
				}
				MHal_CC_SetSlicingSys(system_type);
			}
			break;
        default:
			printk("Unknown IOCTL CMD %x\n",u32Cmd);
			PROBE_IO_EXIT(MDRV_MAJOR_CC, _IOC_NR(u32Cmd));
            return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_CC, _IOC_NR(u32Cmd));
	return 0;
}


// -----------------------------------------------
//
// vd device variable.
//

/* Our various sub-devices. */
static struct file_operations cc_ops = {
	.owner      = THIS_MODULE,
	.open       = MDrv_CC_Open,
	.release    = MDrv_CC_Release,
	.ioctl      = MDrv_CC_IOCtl,
};



/* Module housekeeping. */
static int MDrv_CC_Init(void)
{
	int result;
	dev_t dev = MKDEV(_u32majorCC, 0);
	printk("MDrv_CC_Init\n");
	/* Figure out our device number. */
	if (_u32majorCC)
		result = register_chrdev_region(dev, 1, "CC");
	else {
		result = alloc_chrdev_region(&dev, 0, 1, "CC");
		_u32majorCC = MAJOR(dev);
	}
	if (result < 0) {
		CC_KDBG("unable to get major %d\n", _u32majorCC);
		return result;
	}
	if (_u32majorCC == 0)
		_u32majorCC = result;

	MDrv_CC_SetupCDev(_stDevCC, 0, &cc_ops);

    CC_KDBG("init %d\n", _u32majorCC);
	return 0;
}


static void MDrv_CC_Cleanup(void)
{
	cdev_del(_stDevCC);
	unregister_chrdev_region(MKDEV(_u32majorCC, 0), 2);
}



// -----------------------------------------------
//
// vd device variable.
//
module_init(MDrv_CC_Init);
module_exit(MDrv_CC_Cleanup);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("CC driver");
MODULE_LICENSE("MSTAR");
