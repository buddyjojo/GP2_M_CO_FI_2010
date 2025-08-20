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
/// file    mdrv_gpio_io.c
/// @brief  GPIO Driver Interface
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
#include <asm/semaphore.h>

#include "mst_devid.h"

#include "mdrv_gpio_io.h"
#include "mhal_gpio_reg.h"
#include "mhal_gpio.h"
#include "mdrv_gpio.h"
#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_VENUS_BOARD_DVB_1 1
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_VENUS_BOARD_ATSC_1 1
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_DVB_1 1
//#define CONFIG_MSTAR_TITANIA_BD_T2_LG_PDP_BOARD_DVB_1 1

#if defined(CONFIG_MSTAR_TITANIA_BD_S5_LG_DEMO_BOARD_1)
#define LGE_S5_BOARD_GPIO
#elif	(	defined(CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1)	||	\
			defined(CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERVA_BOARD_ATSC_1)	)
#define LGE_S5_ATSC_BOARD_GPIO
#elif defined(CONFIG_MSTAR_TITANIA_BD_S6_LG_DEMO_BOARD_DVB_1)
#define LGE_S6_DVB_BOARD_GPIO
#endif

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define GPIO_DBG_ENABLE              0

#if GPIO_DBG_ENABLE
#define GPIO_DBG(_f)                 (_f)
#else
#define GPIO_DBG(_f)
#endif

#if 0
#define LINE_DBG()                  printf("GPIO %d\n", __LINE__)
#else
#define LINE_DBG()
#endif

#define GPIO_PRINT(fmt, args...)        //printk("[GPIO][%05d] " fmt, __LINE__, ## args)

typedef struct
{
    S32                          s32MajorGPIO;
    S32                          s32MinorGPIO;
    struct cdev                 cDevice;
    struct file_operations      GPIOFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} GPIO_ModHandle_t;


#define MOD_GPIO_DEVICE_COUNT         1
#define MOD_GPIO_NAME                 "ModGPIO"


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
DECLARE_MUTEX(PfModeSem);


//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _MDrv_GPIO_Open (struct inode *inode, struct file *filp);
static int                      _MDrv_GPIO_Release(struct inode *inode, struct file *filp);
static ssize_t                  _MDrv_GPIO_Read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _MDrv_GPIO_Write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _MDrv_GPIO_Poll(struct file *filp, poll_table *wait);
static int                      _MDrv_GPIO_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _MDrv_GPIO_Fasync(int fd, struct file *filp, int mode);

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

static GPIO_ModHandle_t GPIODev=
{
    .s32MajorGPIO = MDRV_MAJOR_GPIO,
    .s32MinorGPIO = MDRV_MINOR_GPIO,
    .cDevice =
    {
        .kobj = {.name= MOD_GPIO_NAME, },
        .owner = THIS_MODULE,
    },
    .GPIOFop =
    {
        .open =     _MDrv_GPIO_Open,
        .release =  _MDrv_GPIO_Release,
        .read =     _MDrv_GPIO_Read,
        .write =    _MDrv_GPIO_Write,
        .poll =     _MDrv_GPIO_Poll,
        .ioctl =    _MDrv_GPIO_Ioctl,
        .fasync =   _MDrv_GPIO_Fasync,
    },
};

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

static int _MDrv_GPIO_Open (struct inode *inode, struct file *filp)
{
	GPIO_ModHandle_t *dev;

    GPIO_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, GPIO_ModHandle_t, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _MDrv_GPIO_Release(struct inode *inode, struct file *filp)
{
    GPIO_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _MDrv_GPIO_Read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    GPIO_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _MDrv_GPIO_Write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    GPIO_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _MDrv_GPIO_Poll(struct file *filp, poll_table *wait)
{
    GPIO_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _MDrv_GPIO_Fasync(int fd, struct file *filp, int mode)
{
    GPIO_PRINT("%s is invoked\n", __FUNCTION__);

	return 0;//fasync_helper(fd, filp, mode, &IICDev.async_queue);
}

static int _MDrv_GPIO_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err= 0;
  	U8 u8Temp;

    GPIO_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((GPIO_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> GPIO_IOC_MAXNR))
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


    PROBE_IO_ENTRY(MDRV_MAJOR_GPIO, _IOC_NR(cmd));

    switch(cmd)
    {
        //the cases of this section set to initialize
        case MDRV_GPIO_INIT:
            GPIO_PRINT("ioctl: MDRV_GPIO_INIT\n");
            MDrv_GPIO_Init(eCOLD_BOOT);
            break;
        case MDRV_GPIO_SET:
            GPIO_PRINT("ioctl: MDRV_GPIO_SET\n");
            if (copy_from_user(&u8Temp, (U8 *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MDrv_GPIO_Pad_Set(u8Temp);
            break;
        case MDRV_GPIO_OEN:
            GPIO_PRINT("ioctl: MDRV_GPIO_OEN\n");
            if (copy_from_user(&u8Temp, (U8 *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MDrv_GPIO_Pad_Oen(u8Temp);
            break;
        case MDRV_GPIO_ODN:
            GPIO_PRINT("ioctl: MDRV_GPIO_ODN\n");
            if (copy_from_user(&u8Temp, (U8 *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MDrv_GPIO_Pad_Odn(u8Temp);
            break;
        case MDRV_GPIO_READ:
            GPIO_PRINT("ioctl: MDRV_GPIO_READ\n");
            if (copy_from_user(&u8Temp, (U8 *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Temp=MDrv_GPIO_Pad_Read(u8Temp);
            if (copy_to_user((U8 *) arg, &u8Temp, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;
        case MDRV_GPIO_PULL_HIGH:
            GPIO_PRINT("ioctl: MDRV_GPIO_PULL_HIGH\n");
            if (copy_from_user(&u8Temp, (U8 *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MDrv_GPIO_Pull_High(u8Temp);
            break;
        case MDRV_GPIO_PULL_LOW:
            GPIO_PRINT("ioctl: MDRV_GPIO_PULL_LOW\n");
            if (copy_from_user(&u8Temp, (U8 *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MDrv_GPIO_Pull_Low(u8Temp);
            break;

        default:
            GPIO_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
            return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_GPIO, _IOC_NR(cmd));
    return 0;
}

void __mod_gpio_init(eBOOT_TYPE type)
{
#ifdef	_INCLUDE_BOOTSTRAP
/*	LCD/PDP RUNTIME 분기를 위해 H/W OPTION을 기준으로 처리하도록 수정(dreamer@lge.com) */
	extern U8	MDrv_GPIO_GetBootStrap( void );

	U8	u8TconType; //balup_090907
#endif	/*	#ifdef	_INCLUDE_BOOTSTRAP */

	U8	u8IsLCDType = TRUE;

    GPIO_PRINT("Backlight turns on...\n");
    //GPIO chiptop initialization
	MDrv_GPIO_Init(type);

#ifdef	_INCLUDE_BOOTSTRAP
/*	LCD/PDP RUNTIME 분기를 위해 H/W OPTION을 기준으로 처리하도록 수정(dreamer@lge.com) */

#ifdef _INCLUDE_NEWBOOTSTRAP
	printk("\n Check model option \n");
	u8TconType = (MDrv_GPIO_GetBootStrap() & 0xFF);

	u8IsLCDType = (u8TconType & 0x08);
	printk("Display Type : %s\n", (( u8IsLCDType ) ? "LCD" : "PDP"));

	if (u8IsLCDType)
	{
		if	   (((u8TconType & 0x42) == 0x00)  || ((u8TconType & 0x40) == 0x00)) printk("NO FRC\n");
		else if((u8TconType & 0x42) == 0x40) printk("URSA3 Internal\n");
		else if((u8TconType & 0x42) == 0x42) printk("URSA3 External\n");
		else if((u8TconType & 0x42) == 0x02) printk("PWIZ Panel TCON FRC\n");

		if((u8TconType & 0x20) == 0x20) 	printk("Mini LVDS\n");
		else if((u8TconType & 0x20) == 0x00) printk("Normal LVDS\n");

		if((u8TconType & 0x10) == 0x10) printk("DDR 512\n");
		else if((u8TconType & 0x10) == 0x00) printk("DDR 256\n");

		if((u8TconType & 0x04) == 0x04) printk("FHD Panel\n");
		else if((u8TconType & 0x04) == 0x00) printk("HD Panel\n");

		if((u8TconType & 0x01) == 0x01) printk("GIP Panel\n");
		else if((u8TconType & 0x01) == 0x00) printk("Non-GIP Panel\n");

		if((u8TconType & 0x80) == 0x80) printk("OLED Panel\n");
		else if((u8TconType & 0x80) == 0x00) printk("LCD Panel\n");
	}
	else
	{
		if((u8TconType & 0x40) == 0x0) printk("PDP Display\n");

		if((u8TconType & 0x20) == 0x20) 	printk("XGA\n");
		else if((u8TconType & 0x20) == 0x00) printk("WXGA\n");

		if((u8TconType & 0x10) == 0x10) printk("LED Normal\n");
		else if((u8TconType & 0x10) == 0x00) printk("LED Moving\n");

		if((u8TconType & 0x04) == 0x04) printk("FHD Panel\n");
		else if((u8TconType & 0x04) == 0x00) printk("HD Panel\n");
	}

#else
	#ifdef _INCLUDE_S7BOOTSTRAP
	u8TconType = (MDrv_GPIO_GetBootStrap() & 0x14);

	if(u8TconType == 0x04) 		printk("model option: SATURN7 TCON 50HZ\n");
	else if(u8TconType == 0x14) printk("model option: SATURN7M MiniLVDS 100Hz\n");
	else if(u8TconType == 0x10) printk("model option: SATURN7M LVDS 100Hz\n");
	else						printk("model option: SATURN7 LVDS 50Hz\n");

	#else
		u8IsLCDType = (MDrv_GPIO_GetBootStrap() & 0xF0);
		printk("GPIO Init(%s)\n", (( u8IsLCDType ) ? "LCD" : "PDP"));
	#endif
#endif

#endif	/* #ifdef	_INCLUDE_BOOTSTRAP */


	//BlueTooth On  -- // MODEL_OPT_3 동일 PIN 사용
	MDrv_GPIO_Pad_Set(PAD_SAR1);
	MDrv_GPIO_Pad_Oen(PAD_SAR1);
	MDrv_GPIO_Pull_High(PAD_SAR1);


#if 0  // ieeum  inverter is controlled by Micom
	if( u8IsLCDType )
	{	/* LCD Only:	*/
		MDrv_GPIO_Pad_Set(PAD_GPIO_PM2); // invert control
		MDrv_GPIO_Pad_Oen(PAD_GPIO_PM2);
		MDrv_GPIO_Pull_High(PAD_GPIO_PM2);	//athens: Change INV_CTRL Init Value (Low --> High) 080819
		//printk("PM2(INV) High\n");
	}
#endif

	// CI Enable (PCM_5V_CTL)
    MDrv_GPIO_Pad_Set(PAD_SAR2); // CI enable
    MDrv_GPIO_Pad_Oen(PAD_SAR2);
    MDrv_GPIO_Pull_Low(PAD_SAR2);

	// USB CTL
    MDrv_GPIO_Pad_Set(PAD_GPIO_PM1);
    MDrv_GPIO_Pad_Oen(PAD_GPIO_PM1);
    MDrv_GPIO_Pull_High(PAD_GPIO_PM1);

	// USB OCD (over current detection)
	MDrv_GPIO_Pad_Set(PAD_GPIO_PM0);
	MDrv_GPIO_Pad_Odn(PAD_GPIO_PM0); // set as input pin

     // USB Hub reset
	MDrv_GPIO_Pad_Set(PAD_SAR4);
	MDrv_GPIO_Pad_Oen(PAD_SAR4);
	MDrv_GPIO_Pull_High(PAD_SAR4);
	if(type == eDEEP_SLEEP) {
		mdelay(1);
	} else {
	msleep(1);
	}
	MDrv_GPIO_Pull_Low(PAD_SAR4);
	if(type == eDEEP_SLEEP) {
		mdelay(1);
	} else {
	msleep(1);
	}
	MDrv_GPIO_Pull_High(PAD_SAR4);
	if( !u8IsLCDType )
	{
		// USB2 CTL
		MDrv_GPIO_Pad_Set(PAD_GPIO_PM2);
		MDrv_GPIO_Pad_Oen(PAD_GPIO_PM2);
		MDrv_GPIO_Pull_High(PAD_GPIO_PM2);

		// USB2 OCD (over current detection)
		MDrv_GPIO_Pad_Set(PAD_GPIO_PM3);
		MDrv_GPIO_Pad_Odn(PAD_GPIO_PM3); // set as input pin
	}

        // GPIO_PORT_RD_ERROR_OUT
	MDrv_GPIO_Pad_Set(PAD_GPIO5);
	MDrv_GPIO_Pad_Odn(PAD_GPIO5); // set as input pin



#ifdef _INCLUDE_BOOTSTRAP
	//balup_090907
	#ifdef _INCLUDE_NEWBOOTSTRAP
	if(u8IsLCDType && (((u8TconType & 0x62) == 0x20)  || ((u8TconType & 0x60) == 0x20)))//LCD Saturn7 Tcon [NO FRC + MINI LVDS]
	#else
	if(u8TconType == 0x04)
	#endif
	{
		//Panel Power on - 150ms - DPM on sequence를 위해 여기서는 high로 올리면 안됨.
		#if 0
		//printk("\n- saturn7 tcon gpio init -\n");
		MDrv_GPIO_Pad_Set(PAD_GPIO9);
	    MDrv_GPIO_Pad_Oen(PAD_GPIO9);
	    MDrv_GPIO_Pull_High(PAD_GPIO9);
		#endif

		MDrv_GPIO_Pad_Set(PAD_GPIO8);
	    MDrv_GPIO_Pad_Oen(PAD_GPIO8);
	    MDrv_GPIO_Pull_Low(PAD_GPIO8);
	}
#endif

#if 0
//#if ! (defined(LGE_S5_BOARD_GPIO) || defined(LGE_S6_DVB_BOARD_GPIO) || defined(LGE_S5_ATSC_BOARD_GPIO))
    //for MST55C,PAD_GPIO14 pull low to let backlight turn on
    //MDrv_GPIO_Pad_Set(PAD_GPIO14);
    MDrv_GPIO_Pad_Oen(PAD_GPIO14);
    MDrv_GPIO_Pull_Low(PAD_GPIO14);
    //for MST55C,PAD_GPIO15 pull low to let backlight turn on
    //MDrv_GPIO_Pad_Set(PAD_GPIO15);
    MDrv_GPIO_Pad_Oen(PAD_GPIO15);
    MDrv_GPIO_Pull_Low(PAD_GPIO15);
#endif


}

// ieeum internal Micom 사용시 inverter, OPC등 초기화 필요할지도...
static int __init mod_gpio_init(void)
{
    S32         s32Ret;
    dev_t       dev;

    GPIO_PRINT("%s is invoked\n", __FUNCTION__);

    if (GPIODev.s32MajorGPIO)
    {
        dev = MKDEV(GPIODev.s32MajorGPIO, GPIODev.s32MinorGPIO);
        s32Ret = register_chrdev_region(dev, MOD_GPIO_DEVICE_COUNT, MOD_GPIO_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, GPIODev.s32MinorGPIO, MOD_GPIO_DEVICE_COUNT, MOD_GPIO_NAME);
        GPIODev.s32MajorGPIO = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        GPIO_PRINT("Unable to get major %d\n", GPIODev.s32MajorGPIO);
        return s32Ret;
    }

    cdev_init(&GPIODev.cDevice, &GPIODev.GPIOFop);
    if (0!= (s32Ret= cdev_add(&GPIODev.cDevice, dev, MOD_GPIO_DEVICE_COUNT)))
    {
        GPIO_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_GPIO_DEVICE_COUNT);
        return s32Ret;
    }

	__mod_gpio_init(eCOLD_BOOT);

	return 0;


}


static void __exit mod_gpio_exit(void)
{
    GPIO_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&GPIODev.cDevice);
    unregister_chrdev_region(MKDEV(GPIODev.s32MajorGPIO, GPIODev.s32MinorGPIO), MOD_GPIO_DEVICE_COUNT);
}

module_init(mod_gpio_init);
module_exit(mod_gpio_exit);

EXPORT_SYMBOL(PfModeSem);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("GPIO driver");
MODULE_LICENSE("MSTAR");
