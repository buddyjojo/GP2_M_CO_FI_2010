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
/// @file   mdrv_DIP.c
/// This file contains the Mstar DIP driver
/// @author MStar Semiconductor Inc.
/// @brief  DIP
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


#include "Board.h"
#include "mst_devid.h"
#include "mdrv_types.h"
#include "mdrv_dip_io.h"
#include "mhal_dip.h"
#include "mdrv_probe.h"


#define DIP_DEBUG
#ifdef  DIP_DEBUG

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

static int _MDrv_DIP_Open(struct inode *inode, struct file *filp);
static int _MDrv_DIP_IOCtl(struct inode *inode, struct file *filp, U32 u32Cmd, unsigned long u32Arg);
static int _MDrv_DIP_Release(struct inode *inode, struct file *filp);
static int _MDrv_DIP_MM_MMAP(struct file *filp, struct vm_area_struct *vma);

static void _MDrv_DIP_MM_Open(struct vm_area_struct *vma);
static void _MDrv_DIP_MM_Close(struct vm_area_struct *vma);
int         _MDrv_DIP_MM_NoPage(struct vm_area_struct *vma, struct vm_fault *vmf);

typedef struct
{
    dip_DIP_t                   *pDIPInfo;
    U32                         u32MM;
    int                         s32Major;
    int                         s32Minor;
    struct cdev                 cDevice;
    struct file_operations      Fop;
    struct vm_operations_struct MMops;
} ModHandle;


ModHandle DIPDev=
{
    .pDIPInfo=                  (dip_DIP_t*)NULL,
    .u32MM=                     0,
    .s32Major=                  MDRV_MAJOR_DIP,
    .s32Minor=                  MDRV_MINOR_DIP,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_DIP, },
        .owner  =               THIS_MODULE,
    },
    .Fop=
    {
        .open=                  _MDrv_DIP_Open,
        .release=               _MDrv_DIP_Release,
        .ioctl=                 _MDrv_DIP_IOCtl,
        .mmap=                  _MDrv_DIP_MM_MMAP,
    },
    .MMops =
    {
        .open=                  _MDrv_DIP_MM_Open,
        .close=                 _MDrv_DIP_MM_Close,
        .fault=                 _MDrv_DIP_MM_NoPage
    }
};


U32 pDIPInfo = NULL;


// -----------------------------------------------
//
// device functions
//
static int _MDrv_DIP_Open(struct inode *inode, struct file *filp)
{
    DIPDev.pDIPInfo = (dip_DIP_t*)phys_to_virt((U32)DIP_ADR);
    pDIPInfo = (U32)DIPDev.pDIPInfo;
	return 0;
}

static int _MDrv_DIP_IOCtl(struct inode *inode, struct file *filp, U32 u32Cmd, unsigned long u32Arg)
{
    int                 err= 0;
    dip_frameinfo_t     frameinfo;
    dip_buf_mgr_t       bufmgr;
    dip_hist_out_t      *phistout;
    dip_hist_diff_t     *phistdiff;
    dip_DIbufinfo_t     *pdibufinfo;
    U32                 retval;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (DIP_IOC_MAGIC != _IOC_TYPE(u32Cmd))
    {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(u32Cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)u32Arg, _IOC_SIZE(u32Cmd));
    }
    else if (_IOC_DIR(u32Cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)u32Arg, _IOC_SIZE(u32Cmd));
    }
    if (err)
    {
        return -EFAULT;
    }


    PROBE_IO_ENTRY(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));

    //spinlock_t  irq_lock = SPIN_LOCK_UNLOCKED;
    switch(u32Cmd)
    {
        case DIP_IOC_INIT:
            err = MHal_DIP_Init(u32Arg);
            break;
        case DIP_IOC_SET_FRAME_INFO:
            if (copy_from_user(&frameinfo, (dip_frameinfo_t __user *)u32Arg, sizeof(dip_frameinfo_t)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            err = MHal_DIP_SetFrameInfo(frameinfo.u32FrameWidth, frameinfo.u32FrameHeight, frameinfo.bInterLace);
            break;
        case DIP_IOC_INPUT_MODE:
            err = MHal_DIP_SetIntputMode(u32Arg);
            break;

        case DIP_IOC_SET_NRBUF:
            if (copy_from_user(&bufmgr, (dip_buf_mgr_t __user *)u32Arg, sizeof(dip_buf_mgr_t)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            err = MHal_DIP_SetNRBuff(bufmgr.u8BufCnt, bufmgr.u32BufWidth, bufmgr.u32BufHeight, bufmgr.u32BufStart, bufmgr.u32BufEnd);
            break;

        case DIP_IOC_SET_DIBUF:
            if (copy_from_user(&bufmgr, (dip_buf_mgr_t __user *)u32Arg, sizeof(dip_buf_mgr_t)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            err = MHal_DIP_SetDIBuff(bufmgr.u8BufCnt, bufmgr.u32BufWidth, bufmgr.u32BufHeight, bufmgr.u32BufStart, bufmgr.u32BufEnd);
            break;
        case DIP_IOC_GET_HIS:
            phistout = (dip_hist_out_t*) MHal_DIP_GetHistogramOut();
            if(copy_to_user((dip_hist_out_t*)u32Arg, phistout, sizeof(dip_hist_out_t)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            break;

        case DIP_IOC_GET_HIS_DIFF:
            phistdiff = (dip_hist_diff_t*) MHal_DIP_GetHistogramDiff();
            if(copy_to_user((dip_hist_diff_t*)u32Arg, phistdiff, sizeof(dip_hist_diff_t)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            break;
        case DIP_IOC_ENABLE_NRDI:
            err = MHal_DIP_EnableNRandDI((u32Arg&BIT0), (u32Arg&BIT1), (u32Arg&BIT2), (u32Arg&BIT3));
            break;

        case DIP_IOC_GET_DI_BUF_COUNT:
            retval = MHal_DIP_GetDiBuffCount();
            if(copy_to_user((U32*)u32Arg, &retval, sizeof(U32)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            break;
        case DIP_IOC_GET_DI_BUF_INFO:
            pdibufinfo = (dip_DIbufinfo_t*)MHal_DIP_GetDiBuffInfo();
            if(copy_to_user((dip_DIbufinfo_t*)u32Arg, pdibufinfo, sizeof(dip_DIbufinfo_t)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            break;
        case DIP_IOC_CLEAR_DI_BUF_STATUS:
            err = MHal_DIP_ClearDiBuffStatus(u32Arg);
            break;
        case DIP_IOC_GET_DI_BUF_FRAME_COUNT:
            retval = MHal_DIP_GetDiBuffFrameCount();
            if(copy_to_user((U32*)u32Arg, &retval, sizeof(U32)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            break;
        case DIP_IOC_GET_DI_BUF_STATUS:
            retval =MHal_DIP_GetDiBuffStatus();
            if(copy_to_user((U32*)u32Arg, &retval, sizeof(U32)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            break;
        case DIP_IOC_GET_DI_OUTPUT_INFO:
            retval = MHal_DIP_GetDIOutputInfo();
            if(copy_to_user((U32*)u32Arg, &retval, sizeof(U32)))
            {
                assert(0);
                PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
                return -EFAULT;
            }
            break;

        case DIP_IOC_SHOW_FIELD_INFO:
            MHal_DIP_ShowFieldInfo();

            break;
        default:
            PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
            return -ENOTTY;

    }

    PROBE_IO_EXIT(MDRV_MAJOR_DIP, _IOC_NR(u32Cmd));
	return err;
}

static void _MDrv_DIP_MM_Open(struct vm_area_struct *vma)
{
    DIPDev.u32MM++;
}

static void _MDrv_DIP_MM_Close(struct vm_area_struct *vma)
{
    DIPDev.u32MM--;
}


static int _MDrv_DIP_MM_MMAP(struct file *filp, struct vm_area_struct *vma)
{
#if 1
    KDBG("DIP VMSIZE = 0x%x\n", (U32)(vma->vm_end - vma->vm_start));

    /* over size */
    if(vma->vm_end - vma->vm_start > (0x01<<PAGE_SHIFT))
    {
        ASSERT( FALSE );
        return -EFAULT;
    }
    vma->vm_flags |= VM_RESERVED;
    vma->vm_ops = &DIPDev.MMops;
    /* set page to no cache */
    //pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
    //pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;

    remap_pfn_range(vma, vma->vm_start, (virt_to_phys((U8*)DIPDev.pDIPInfo)>>PAGE_SHIFT),
        vma->vm_end - vma->vm_start, vma->vm_page_prot);

    KDBG("VMSTART = 0x%x, VMEND = 0x%x, PFN = 0x%x\n",
        (U32)(vma->vm_start), (U32)(vma->vm_end), (U32)(virt_to_phys((U8*)DIPDev.pDIPInfo)>>PAGE_SHIFT));

    _MDrv_DIP_MM_Open(vma);

    KDBG("MMAP Done.\n");
#endif

    return 0;
}

#if 1
int _MDrv_DIP_MM_NoPage(struct vm_area_struct *vma, struct vm_fault *vmf)
{
    return 0;
}
#endif


static int _MDrv_DIP_Release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int __init MOD_DIP_Init(void)
{
    int         s32Ret;
    dev_t       dev;

    if (DIPDev.s32Major)
    {
        dev=                    MKDEV(DIPDev.s32Major, DIPDev.s32Minor);
        s32Ret=                 register_chrdev_region(dev, 1, MDRV_NAME_DIP);
    }
    else
    {
        s32Ret=                 alloc_chrdev_region(&dev, DIPDev.s32Minor, 1, MDRV_NAME_DIP);
        DIPDev.s32Major=     MAJOR(dev);
    }
    if (0> s32Ret)
    {
        KDBG("Unable to get major %d\n", DIPDev.s32Major);
        return s32Ret;
    }

    cdev_init(&DIPDev.cDevice, &DIPDev.Fop);
    if (0!= (s32Ret= cdev_add(&DIPDev.cDevice, dev, 1)))
    {
        KDBG("Unable add a character device\n");
        unregister_chrdev_region(dev, 1);
        return s32Ret;
    }
    return 0;
}

static void __exit MOD_DIP_Exit(void)
{
    // don't release all driver's mmap
    ASSERT(DIPDev.u32MM!=0);
    cdev_del(&DIPDev.cDevice);
    unregister_chrdev_region(MKDEV(DIPDev.s32Major, DIPDev.s32Minor), 1);
}


module_init(MOD_DIP_Init);
module_exit(MOD_DIP_Exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("DIP driver");
MODULE_LICENSE("MSTAR");
