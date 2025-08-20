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
/// @file   mdrv_probe.c
/// This file contains the Mstar PROBE driver
/// @author MStar Semiconductor Inc.
/// @brief  PROBE (Driver probe system)
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

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/marker.h>
#include <asm/atomic.h>



#include "mdrv_probe.h"

#include "chip_setup.h"
#include "mst_devid.h"
#include "mdrv_types.h"


//extern void local_flush_tlb_all(void);

#define PROBE_DEBUG
#ifdef  PROBE_DEBUG

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

#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)


static int _MDrv_PROBE_Open(struct inode *inode, struct file *filp);
static int _MDrv_PROBE_IOCtl(struct inode *inode, struct file *filp, U32 u32Cmd, unsigned long u32Arg);
static int _MDrv_PROBE_Release(struct inode *inode, struct file *filp);

typedef struct
{
    int                         s32Major;
    int                         s32Minor;
    struct cdev                 cDevice;
    struct file_operations      Fop;
} PROBEModHandle;

static PROBEModHandle PROBEDev=
{
    .s32Major=               MDRV_MAJOR_PROBE,
    .s32Minor=               MDRV_MINOR_PROBE,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_PROBE, },
        .owner  =               THIS_MODULE,
    },
    .Fop=
    {
        .open=                  _MDrv_PROBE_Open,
        .release=               _MDrv_PROBE_Release,
        .ioctl=                 _MDrv_PROBE_IOCtl,
    },
};


struct IO_Entry
{
    unsigned int    Jiffies;
    unsigned short  DrvID;
    unsigned short  IONum;
};

struct INT_Entry
{
    unsigned int        Jiffies;
    unsigned short      INTNum;
};

struct Var_Entry
{
    unsigned int        value;
};

#define IO_ENTRY_NUM    4000
#define INT_ENTRY_NUM   10000

struct DebugLog
{
    // IO Log
    unsigned int        CT_IO;
    unsigned int        CT_INT;
    unsigned int        Jiffies;

    struct Var_Entry    VAR;
    struct IO_Entry     LOG_IO[IO_ENTRY_NUM];  //32 K bytes
    struct INT_Entry    LOG_INT[INT_ENTRY_NUM]; //60 K bytes
};

struct DebugLog         *Driver_LOG;

/* rordd */
static int enable_probe;
static int __init probe_enable(char *str)
{	
	if (str != NULL && strcmp(str,"1") == 0)
		enable_probe=1; 
    return 1;
}
__setup("enable_probe=", probe_enable);


// -----------------------------------------------
//
// probe functions
//
void probe_IO_entry(void *probe_data, void *call_data,
	const char *format, va_list *args)
{
	/* Declare args */
	unsigned int IONum;
	unsigned int DrvID;
    int    CT;
    if(Driver_LOG != 0)
    {
    	/* Assign args */
    	IONum = va_arg(*args, unsigned int);
    	DrvID = va_arg(*args, unsigned int);
        CT = Driver_LOG->CT_IO;

        Driver_LOG->LOG_IO[CT].Jiffies= jiffies;
        Driver_LOG->LOG_IO[CT].DrvID = DrvID;
        Driver_LOG->LOG_IO[CT].IONum = IONum;
        if(CT < IO_ENTRY_NUM)
            CT++;
        else
            CT = 0;

        Driver_LOG->CT_IO = CT;
        Chip_Flush_Memory();
    }
}

void probe_IO_exit(void *probe_data, void *call_data,
	const char *format, va_list *args)
{
	/* Declare args */
	unsigned int IONum;
	unsigned int DrvID;
	int         CT;

    if(Driver_LOG != 0)
    {
    	/* Assign args */
    	IONum = va_arg(*args, unsigned int);
    	DrvID = va_arg(*args, unsigned int);
        CT = Driver_LOG->CT_IO;

        Driver_LOG->LOG_IO[CT].Jiffies= jiffies;
        Driver_LOG->LOG_IO[CT].DrvID = DrvID;
        Driver_LOG->LOG_IO[CT].IONum = IONum | 0x1000;

        if(CT < IO_ENTRY_NUM)
            CT++;
        else
            CT = 0;

        Driver_LOG->CT_IO = CT;
        Chip_Flush_Memory();
    }
}


void probe_INT_entry(void *probe_data, void *call_data,
	const char *format, va_list *args)
{
	/* Declare args */
	unsigned int INTNum;
	int         CT;

    if(Driver_LOG != 0)
    {
    	/* Assign args */
    	INTNum = va_arg(*args, unsigned int);
        CT = Driver_LOG->CT_INT;

        Driver_LOG->LOG_INT[CT].Jiffies= jiffies;
        Driver_LOG->LOG_INT[CT].INTNum = INTNum;

        if(CT < INT_ENTRY_NUM)
            CT++;
        else
            CT = 0;

        Driver_LOG->CT_INT = CT;
        Chip_Flush_Memory();
    }
}

void probe_INT_exit(void *probe_data, void *call_data,
	const char *format, va_list *args)
{
	/* Declare args */
	unsigned int INTNum;
	int         CT;

    if(Driver_LOG != 0)
    {
    	/* Assign args */
    	INTNum = va_arg(*args, unsigned int);
        CT = Driver_LOG->CT_INT;

        Driver_LOG->LOG_INT[CT].Jiffies= jiffies;
        Driver_LOG->LOG_INT[CT].INTNum = INTNum | 0x1000;

        if(CT < INT_ENTRY_NUM)
            CT++;
        else
            CT = 0;

        Driver_LOG->CT_INT = CT;
        Chip_Flush_Memory();
    }
}


void probe_jiffies(void *probe_data, void *call_data,
	const char *format, va_list *args)
{
    Driver_LOG->Jiffies = jiffies;
}

void probe_VAR(void *probe_data, void *call_data,
	const char *format, va_list *args)
{
	/* Declare args */
	unsigned int Var;
    if(Driver_LOG != 0)
    {
    	/* Assign args */
    	Var = va_arg(*args, unsigned int);
        Driver_LOG->VAR.value = Var;
        Chip_Flush_Memory();
    }
}



struct probe_entry {
	const char          *name;
	marker_probe_func   *probe_func;
	const char          *format;
};

#define NUM_PROBES ARRAY_SIZE(probe_list)


static struct probe_entry probe_list[] =
{
	{	.name = "_IO_entry",
		.format = "%d %d",
		.probe_func = probe_IO_entry
	},

	{	.name = "_IO_exit",
		.format = "%d %d",
		.probe_func = probe_IO_exit
	},

	{	.name = "_jiffies",
		.format = MARK_NOARGS,
		.probe_func = probe_jiffies
	},

	{	.name = "_INT_entry",
		.format = "%d",
		.probe_func = probe_INT_entry
	},

	{	.name = "_INT_exit",
		.format = "%d",
		.probe_func = probe_INT_exit
	},
	{	.name = "_var",
		.format = "%d",
		.probe_func = probe_VAR
	},

};

// -----------------------------------------------
//
// device functions
//
static int _MDrv_PROBE_Open(struct inode *inode, struct file *filp)
{
    return 0;
}
static int _MDrv_PROBE_IOCtl(struct inode *inode, struct file *filp, U32 u32Cmd, unsigned long u32Arg)
{
    switch(u32Cmd)
    {
        /*
        case STR_IOC_ENTER:
            break;
        default:
            return -ENOTTY;
        */

    }
    return 0;
}

static int _MDrv_PROBE_Release(struct inode *inode, struct file *filp)
{
    return 0;
}

static int __init MOD_PROBE_Init(void)
{
    int         s32Ret;
    dev_t       dev;
	int         i;


    if (PROBEDev.s32Major)
    {
        dev=                    MKDEV(PROBEDev.s32Major, PROBEDev.s32Minor);
        s32Ret=                 register_chrdev_region(dev, 1, MDRV_NAME_PROBE);
    }
    else
    {
        s32Ret=                 alloc_chrdev_region(&dev, PROBEDev.s32Minor, 1, MDRV_NAME_PROBE);
        PROBEDev.s32Major=     MAJOR(dev);
    }
    if (0> s32Ret)
    {
        KDBG("Unable to get major %d\n", PROBEDev.s32Major);
        return s32Ret;
    }

    cdev_init(&PROBEDev.cDevice, &PROBEDev.Fop);
    if (0!= (s32Ret= cdev_add(&PROBEDev.cDevice, dev, 1)))
    {
        KDBG("Unable add a character device\n");
        unregister_chrdev_region(dev, 1);
        return s32Ret;
    }

	for (i = 0; i < ARRAY_SIZE(probe_list); i++) {
		s32Ret = marker_probe_register(probe_list[i].name,
				probe_list[i].format,
				probe_list[i].probe_func, &probe_list[i]);
		if (s32Ret)
			printk(KERN_INFO "Unable to register probe %s\n",
				probe_list[i].name);
	}

    if(MDrv_SYS_GetMMAP(E_SYS_MMAP_DEBUG, (unsigned int*)&Driver_LOG, &i))
    {
        #if defined(CONFIG_Titania3)
        Driver_LOG = (struct DebugLog *)(((unsigned int)Driver_LOG&~0x10000000) | 0xC0000000);
        Driver_LOG->CT_IO = 0;
        Driver_LOG->CT_INT = 0;
        #endif
    }
    else
    {
        Driver_LOG = (struct DebugLog *)(-1);
    }

	if (enable_probe)
    	printk("[PROBE] buffer %x\n", (unsigned int)Driver_LOG);


    memset(Driver_LOG, 0, 0x20000);

    return 0;
}

static void __exit MOD_PROBE_Exit(void)
{
	int     i;

    cdev_del(&PROBEDev.cDevice);
    unregister_chrdev_region(MKDEV(PROBEDev.s32Major, PROBEDev.s32Minor), 1);

	for (i = 0; i < ARRAY_SIZE(probe_list); i++)
		marker_probe_unregister(probe_list[i].name,
			probe_list[i].probe_func, &probe_list[i]);
}

#if ENABLE_PROBE


void PROBE_IO_ENTRY(int DrvID, int IONum)
{
	if (enable_probe == 0)
		return ;
	
    trace_mark(_IO_entry, "%d %d", IONum, DrvID);
}

void PROBE_IO_EXIT(int DrvID, int IONum)
{
	if (enable_probe == 0)
		return ;
    trace_mark(_IO_exit, "%d %d", IONum, DrvID);
}

void PROBE_JIFFIES(void)
{
	if (enable_probe == 0)
		return ;
    trace_mark(_jiffies, MARK_NOARGS);
}

void PROBE_INT_ENTRY(int INTNum)
{
	if (enable_probe == 0)
		return ;
    trace_mark(_INT_entry, "%d", INTNum);
}

void PROBE_INT_EXIT(int INTNum)
{
	if (enable_probe == 0)
		return ;
    trace_mark(_INT_exit,  "%d", INTNum);
}

void PROBE_VAR(int Var)
{
	if (enable_probe == 0)
		return ;
    //trace_mark(func_exit, "[Exit] (Line %d/Func %s)", Line, pFuncName);
    trace_mark(_var,  "%d", Var);
}


EXPORT_SYMBOL(PROBE_INT_ENTRY);
EXPORT_SYMBOL(PROBE_INT_EXIT);
EXPORT_SYMBOL(PROBE_IO_ENTRY);
EXPORT_SYMBOL(PROBE_IO_EXIT);
EXPORT_SYMBOL(PROBE_JIFFIES);
EXPORT_SYMBOL(PROBE_VAR);


#endif


module_init(MOD_PROBE_Init);
module_exit(MOD_PROBE_Exit);

MODULE_AUTHOR("GPL");
MODULE_DESCRIPTION("Mathieu Desnoyers");
MODULE_LICENSE("SUBSYSTEM Probe");
