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
/// file    mdrv_mpool.c
/// @brief  Memory Pool Control Interface
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
#include <linux/io.h> //added
#include <asm/io.h>
#include <asm-mips/types.h>

#include "mst_devid.h"
#include "mdrv_mpool.h"
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_system.h"


#include "mdrv_probe.h"
//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
//#define MPOOL_DPRINTK(fmt, args...) printk(KERN_WARNING"%s:%d " fmt,__FUNCTION__,__LINE__,## args)
#define MPOOL_DPRINTK(fmt, args...)

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_MPOOL_DEVICE_COUNT     1
#define MOD_MPOOL_NAME             "ModMPOOL"

// Define MPOOL Device
U32 mpool_base;
U32 mpool_size;
extern unsigned int mpool_userspace_base; // base address for userspace mmap

#define MPOOL_BASE                 mpool_base
#define MPOOL_SIZE                 mpool_size

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------;

typedef struct
{
    int                         s32MPoolMajor;
    int                         s32MPoolMinor;
    void*                       dmaBuf;
    struct cdev                 cDevice;
    struct file_operations      MPoolFop;
} MPoolModHandle;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _MDrv_MPOOL_Open (struct inode *inode, struct file *filp);
static int                      _MDrv_MPOOL_Release(struct inode *inode, struct file *filp);
static int                      _MDrv_MPOOL_MMap(struct file *filp, struct vm_area_struct *vma);
static int                      _MDrv_MPOOL_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------
static MPoolModHandle MPoolDev=
{
    .s32MPoolMajor=               MDRV_MAJOR_MPOOL,
    .s32MPoolMinor=               MDRV_MINOR_MPOOL,
    .cDevice=
    {
        .kobj=                  {.name= MOD_MPOOL_NAME, },
        .owner  =               THIS_MODULE,
    },
    .MPoolFop=
    {
        .open=                  _MDrv_MPOOL_Open,
        .release=               _MDrv_MPOOL_Release,
        .mmap=                  _MDrv_MPOOL_MMap,
        .ioctl=                 _MDrv_MPOOL_Ioctl,
    },
};

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

static int _MDrv_MPOOL_Open (struct inode *inode, struct file *filp)
{
    MPoolModHandle *dev;

    MPOOL_DPRINTK("LennyD\n");

    dev = container_of(inode->i_cdev, MPoolModHandle, cDevice);
    // dev->dmaBuf=ioremap_nocache(MPOOL_BASE, MPOOL_SIZE);
    filp->private_data = dev;

    return 0;
}

static int _MDrv_MPOOL_Release(struct inode *inode, struct file *filp)
{
    // MPoolModHandle *dev = filp->private_data ;;

    MPOOL_DPRINTK("LennyD\n");
    // iounmap(dev->dmaBuf) ;
    return 0;
}

static int _MDrv_MPOOL_MMap(struct file *filp, struct vm_area_struct *vma)
{
    vma->vm_pgoff = MPOOL_BASE >> PAGE_SHIFT;

    /* set page to no cache */
    pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
    pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;


#if defined(CONFIG_Titania3)
    // MIU0 start from 0x00000000 (physical)
    if (io_remap_pfn_range(vma, vma->vm_start,
                            MPOOL_BASE >> PAGE_SHIFT, MPOOL_SIZE>>1,
                             vma->vm_page_prot))
    {
        return -EAGAIN;
    }
    // MIU1 start from 0x40000000 (physical)
    if (io_remap_pfn_range(vma, vma->vm_start+(MPOOL_SIZE>>1),
                             0x40000000 >> PAGE_SHIFT, MPOOL_SIZE>>1,
                             vma->vm_page_prot))
    {
        return -EAGAIN;
    }

    // log the userspace base address to let kernel mode to access MIU1 directly
    if( 0xFFFFFFFF!=mpool_userspace_base )
        printk( "multi-proc are not supported!!\n" ) ;
    mpool_userspace_base = vma->vm_start ;
#else
    if (io_remap_pfn_range(vma, vma->vm_start,
                            MPOOL_BASE >> PAGE_SHIFT, MPOOL_SIZE,
                             vma->vm_page_prot))
    {
        return -EAGAIN;
    }
#endif

    return 0;
}


static int _MDrv_MPOOL_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int         err= 0;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (MPOOL_IOC_MAGIC!= _IOC_TYPE(cmd))
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

    PROBE_IO_ENTRY(MDRV_MAJOR_MPOOL, _IOC_NR(cmd));

    // @FIXME: Use a array of function pointer for program readable and code size later
    switch(cmd)
    {
    //------------------------------------------------------------------------------
    // Signal
    //------------------------------------------------------------------------------
    case MPOOL_IOC_INFO:
        {
            DrvMPool_Info_t i ;
            i.u32Addr = MPOOL_BASE ;
            i.u32Size = MPOOL_SIZE ;
            // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
            if( copy_to_user( (void *)arg, &i, sizeof(i) ) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPOOL, _IOC_NR(cmd));
                return -EFAULT;
            }
            //((DrvMPool_Info_t*)(arg))->u32Addr= MPOOL_BASE;
            //((DrvMPool_Info_t*)(arg))->u32Size= MPOOL_SIZE;
        }
        break;
	case MPOOL_IOC_FLUSHDCACHE:
		{
#if defined(CONFIG_Titania3)
/*
            U8* p ;

            printk( "MPOOL_IOC_FLUSHDCACHE is dropped!!\n" ) ;
            p = (U8*)ioremap_nocache(0x40000000, 256*0x100000) ;
            printk( "p=0x%02X\n", *p ) ;
            (*p) += 23 ;
            iounmap(p) ;

            p = (U8*)mpool_userspace_base ;
            printk( "MIU0 address=0x%08X, value=0x%02X\n", (U32)p, *p ) ;
            (*p) += 13 ;
            printk( "             changed value=0x%02X\n", *p ) ;
            p += 0x10000000+4 ;
            printk( "MIU1 address=0x%08X, value=0x%02X\n", (U32)p, *p ) ;
            (*p) += 7 ;
            printk( "             changed value=0x%02X\n", *p ) ;
*/
            U8* p ;
            p = (U8*)mpool_userspace_base ;
            printk( "K:0x%02X, 0x%02X\n", p[0], p[0x10000000] ) ;
            p[0] = 0xA0 ;
            p[0x10000000] = 0xA1 ;
            printk( "K:0x%02X, 0x%02X\n", p[0], p[0x10000000] ) ;
#else
			U32 addr,size;
			addr =(U32) __va(((DrvMPool_Info_t*)(arg))->u32Addr);
			size = (U32)((DrvMPool_Info_t*)(arg))->u32Size;
			_dma_cache_wback_inv(addr,size);
#endif
		}
		break;
    case MPOOL_IOC_GET_BLOCK_OFFSET:
        {
            DrvMPool_Info_t i ;
            // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
            if( copy_from_user( &i, (DrvMPool_Info_t*)arg, sizeof(i) ) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPOOL, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_SYS_GetMMAP(i.u32Addr, &(i.u32Addr), &(i.u32Size)) ;
            // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
            if( copy_to_user( (void *)arg, &i, sizeof(i) ) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MPOOL, _IOC_NR(cmd));
                return -EFAULT;
            }
        }
        break ;
    default:
        printk("Unknown ioctl command %d\n", cmd);
        PROBE_IO_EXIT(MDRV_MAJOR_MPOOL, _IOC_NR(cmd));
        return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_MPOOL, _IOC_NR(cmd));
    return 0;
}


#ifdef CONFIG_MSTAR_TITANIA3
extern void add_wired_entry(unsigned long entrylo0, unsigned long entrylo1,
	unsigned long entryhi, unsigned long pagemask);


static void _mod_mpool_map_miu1(void)
{

#if 0  // move to start_kernel( ) in init/main.c
    unsigned long flags;

#if 1
    unsigned long entrylo0l = ((0x40000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable for MIU1
    unsigned long entrylo1l = ((0x44000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable for MIU1
    unsigned long entrylo0h = ((0x48000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable for MIU1
    unsigned long entrylo1h = ((0x4C000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable for MIU1
#else
    unsigned long entrylo0 = ((0x00000000>> 12) << 6) | (0<<3) | (1<< 2) | (1<< 1) | (1<< 0); // cacheable
    unsigned long entrylo1 = ((0x00000000>> 12) << 6) | (2<<3) | (1<< 2) | (1<< 1) | (1<< 0); // un-cacheable or 0x7
#endif
    local_irq_save(flags);
    add_wired_entry(entrylo0l, entrylo1l, 0xC0000000, PM_64M);
    add_wired_entry(entrylo0h, entrylo1h, 0xC8000000, PM_64M);
    local_irq_restore(flags);
#endif

}
#endif // #ifdef CONFIG_MSTAR_TITANIA3

static int __init mod_mpool_init(void)
{
    int s32Ret;
    dev_t dev;

#if defined(CONFIG_Titania3)
    mpool_base = 0 ;
    mpool_size = (512*0x100000) ;
    mpool_userspace_base = 0xFFFFFFFF ;
#elif defined(CONFIG_Triton) || defined(CONFIG_Titania2)
    mpool_base = 0 ;
    //mpool_size = (128*0x100000) ;
    mpool_size = (192*0x100000) ;
#else
    MDrv_SYS_GetMMAP(E_SYS_MMAP_LINUX_BASE, &mpool_base, &mpool_size);
    mpool_size = (128*0x100000)-mpool_base ;
#endif

    //printk( "\nbase=0x%08X\n", mpool_base );
    //printk( "\nsize=0x%08X\n", mpool_size );

    if (MPoolDev.s32MPoolMajor)
    {
        dev = MKDEV(MPoolDev.s32MPoolMajor, MPoolDev.s32MPoolMinor);
        s32Ret = register_chrdev_region(dev, MOD_MPOOL_DEVICE_COUNT, MOD_MPOOL_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, MPoolDev.s32MPoolMinor, MOD_MPOOL_DEVICE_COUNT, MOD_MPOOL_NAME);
        MPoolDev.s32MPoolMajor = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        MPOOL_DPRINTK("Unable to get major %d\n", MPoolDev.s32MPoolMajor);
        return s32Ret;
    }

    cdev_init(&MPoolDev.cDevice, &MPoolDev.MPoolFop);
    if (0!= (s32Ret= cdev_add(&MPoolDev.cDevice, dev, MOD_MPOOL_DEVICE_COUNT)))
    {
        MPOOL_DPRINTK("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_MPOOL_DEVICE_COUNT);
        return s32Ret;
    }

#ifdef CONFIG_MSTAR_TITANIA3
    _mod_mpool_map_miu1();
    // only for testing
    // *((unsigned int*) 0xD0000000) = 0x12345678;
    // printk("[%s][%d] 0x%08x\n", __FILE__, __LINE__, *((unsigned int*) 0xD0000000));
    // printk("[%s][%d] 0x%08x\n", __FILE__, __LINE__, *((unsigned int*) 0x80000000));
    // printk("[%s][%d] 0x%08x\n", __FILE__, __LINE__, *((unsigned int*) 0xA0000000));
#endif // #ifdef CONFIG_MSTAR_TITANIA3

    return 0;
}

static void __exit mod_mpool_exit(void)
{
    cdev_del(&MPoolDev.cDevice);
    unregister_chrdev_region(MKDEV(MPoolDev.s32MPoolMajor, MPoolDev.s32MPoolMinor), MOD_MPOOL_DEVICE_COUNT);
}

module_init(mod_mpool_init);
module_exit(mod_mpool_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MPOOL driver");
MODULE_LICENSE("MSTAR");
