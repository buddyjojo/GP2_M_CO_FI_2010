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

///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_str.c
/// This file contains the Mstar STR driver
/// @author MStar Semiconductor Inc.
/// @brief  STR (Suspend To RAM)
///////////////////////////////////////////////////////////////////////////////

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/wait.h>
#include <linux/spinlock.h>

#include <asm/cacheflush.h>
#include <asm-mips/tlbflush.h>

#include <linux/delay.h>
#include <linux/jiffies.h>

#include "chip_int.h"

#include "mst_devid.h"
#include "mdrv_types.h"
#include "mdrv_str_io.h"

#include "mdrv_system.h"

#include "../gpio/titania3/mhal_gpio.h"
#include "../nand/titania3/drvNAND.h"

#include "mdrv_gpio_io.h"



#define STR_DEBUG
#ifdef  STR_DEBUG

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                            BUG();\
                        }\
                    } while (0)

#define ASSERT(arg)                  assert((arg))
#define KDBG(x1, args...)            printk(KERN_WARNING x1, ## args)
#else
#define KDBG(x1)
#define ASSERT(arg)
#endif


static int _MDrv_STR_Open(struct inode *inode, struct file *filp);
static int _MDrv_STR_IOCtl(struct inode *inode, struct file *filp, unsigned int u32Cmd, unsigned long u32Arg);
static int _MDrv_STR_Release(struct inode *inode, struct file *filp);


extern void Titania_DisableInterrupt_str(void ) ;
extern void start_deep_sleep(void);
extern void __Titania_setup(void);
extern void Titania_EnableInterrupt_str (void);
extern void ehci_hcd_cleanup(void);
extern void second_ehci_hcd_cleanup(void);
extern void usb_stor_exit(void);
extern void uranus_disable_power_down(void);
extern void reset_usercalls_stat(void);

extern void disable_irq(unsigned int irq);
extern void enable_irq(unsigned int irq);
extern void drvNAND_RESET_PAD_VARIABLE(void);


typedef struct
{
    int                         s32Major;
    int                         s32Minor;
    struct cdev                 cDevice;
    struct file_operations      Fop;
} STRModHandle;

static STRModHandle STRDev=
{
    .s32Major=               MDRV_MAJOR_STR,
    .s32Minor=               MDRV_MINOR_STR,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_STR, },
        .owner  =               THIS_MODULE,
    },
    .Fop=
    {
        .open=                  _MDrv_STR_Open,
        .release=               _MDrv_STR_Release,
        .ioctl=                 _MDrv_STR_IOCtl,
    },
};

// -----------------------------------------------
//
// device functions
//
void peripheral_exit_for_deep_sleep_suspend(void)
{
	usb_stor_exit();
	second_ehci_hcd_cleanup();
	ehci_hcd_cleanup();

	disable_irq(E_IRQ_UART);
}

void peripheral_init_for_deep_sleep_resume(void)
{
	Titania_DisableInterrupt_str();

	__Titania_setup();
	
	enable_irq(E_IRQ_UART);

	drvNAND_RESET_PAD_VARIABLE();

	drvNAND_FLASH_INIT();

	MHal_GPIO_Init();

	__mod_gpio_init(eDEEP_SLEEP);
	
	reset_usercalls_stat();
	uranus_disable_power_down();
}

static int _MDrv_STR_Open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int _MDrv_STR_IOCtl(struct inode *inode, struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    spinlock_t  irq_lock = SPIN_LOCK_UNLOCKED;

    switch(u32Cmd)
    {
		case STR_IOC_ENTER:
			peripheral_exit_for_deep_sleep_suspend();
			
            spin_lock_irq(&irq_lock);

			start_deep_sleep();

			// suspend ZZZ.......

            spin_unlock_irq(&irq_lock);
			
			peripheral_init_for_deep_sleep_resume();
            break;

        default:
            return -ENOTTY;

    }

	return 0;
}


static int _MDrv_STR_Release(struct inode *inode, struct file *filp)
{
	return 0;
}


static int __init MOD_STR_Init(void)
{
    int         s32Ret;
    dev_t       dev;

    if (STRDev.s32Major)
    {
        dev=                    MKDEV(STRDev.s32Major, STRDev.s32Minor);
        s32Ret=                 register_chrdev_region(dev, 1, MDRV_NAME_STR);
    }
    else
    {
        s32Ret=                 alloc_chrdev_region(&dev, STRDev.s32Minor, 1, MDRV_NAME_STR);
        STRDev.s32Major=     MAJOR(dev);
    }
    if (0> s32Ret)
    {
        KDBG("Unable to get major %d\n", STRDev.s32Major);
        return s32Ret;
    }

    cdev_init(&STRDev.cDevice, &STRDev.Fop);
    if (0!= (s32Ret= cdev_add(&STRDev.cDevice, dev, 1)))
    {
        KDBG("Unable add a character device\n");
        unregister_chrdev_region(dev, 1);
        return s32Ret;
    }
    return 0;
}


static void __exit MOD_STR_Exit(void)
{
    cdev_del(&STRDev.cDevice);
    unregister_chrdev_region(MKDEV(STRDev.s32Major, STRDev.s32Minor), 1);
}


module_init(MOD_STR_Init);
module_exit(MOD_STR_Exit);


MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("STR driver");
MODULE_LICENSE("MSTAR");

