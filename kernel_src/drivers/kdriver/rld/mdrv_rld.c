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
/// file    devRLD.c
/// @brief  RLD Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm-mips/types.h>

#include "mdrv_types.h"
#include "mst_devid.h"
#include "mst_platform.h"
#include "mdrv_system.h"

#include "mhal_rld.h"
#include "mdrv_rld_st.h"
#include "mdrv_rld_io.h"

#include "mdrv_probe.h"
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_RLD_DEVICE_COUNT    1

#define RLD_WARNING(fmt, args...)       printk(KERN_WARNING "[RLDMOD][%06d] " fmt, __LINE__, ## args)
#define RLD_PRINT(fmt, args...)         printk("[RLDMOD][%06d]     " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32RldMajor;
    int                         s32RldMinor;
    struct cdev                 cDevice;
    struct file_operations      RldFop;
} RldModHandle;


//-------------------------------------------------------------------------------------------------
//  Forward declaration
//-------------------------------------------------------------------------------------------------
static int _mod_rld_open (struct inode *inode, struct file *filp);
static int _mod_rld_release(struct inode *inode, struct file *filp);
static int _mod_rld_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int _mod_rld_mmap(struct file *filp, struct vm_area_struct *vma);


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static RldModHandle RldDev=
{
    .s32RldMajor=               MDRV_MAJOR_RLD,
    .s32RldMinor=               MDRV_MINOR_RLD,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_RLD, },
        .owner  =               THIS_MODULE,
    },
    .RldFop=
    {
        .open=                  _mod_rld_open,
        .release=               _mod_rld_release,
        .ioctl=                 _mod_rld_ioctl,
        .mmap=                  _mod_rld_mmap
    },
};

static U32 _u32RLDBaseAddr=0;
static U32 _u32RLDBaseLength=0;
//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
BOOL _MDrv_RLD_SetTopFieldAddress(unsigned long arg) {
    U32 u32Addr;
    if (__get_user(u32Addr, (U32  __user *)arg))
        return EFAULT;
    MHal_RLD_SetTopFieldAddress(u32Addr); // convert to physical address
    return 0;
}

BOOL _MDrv_RLD_SetBottomFieldAddress(unsigned long arg) {
    U32 u32Addr;
    if (__get_user(u32Addr, (U32  __user *)arg))
        return EFAULT;
    MHal_RLD_SetBottomFieldAddress(u32Addr);  // convert to physical address
    return 0;
}

BOOL _MDrv_RLD_SetOutputAddress(unsigned long arg) {
    U32 u32Addr;
    if (__get_user(u32Addr, (U32  __user *)arg))
        return EFAULT;
    MHal_RLD_SetOutputAddress(u32Addr);   // convert to physical address
    return 0;
}

BOOL _MDrv_RLD_GetMMAPInfo(unsigned long arg){
    if ( __put_user(_u32RLDBaseAddr, &(((RLD_MMAPInfo_t __user *)arg)->u32Address)))
        return EFAULT;
    if (__put_user(_u32RLDBaseLength, &(((RLD_MMAPInfo_t __user *)arg)->u32Length)))
        return EFAULT;
    return 0;
}

BOOL _MDrv_RLD_FlushDCache(unsigned long arg){
    U32 u32PhyAddr,u32VirtAddr, u32Size;
    if (__get_user(u32PhyAddr, &(((RLD_MMAPInfo_t __user *)arg)->u32Address)))
        return EFAULT;
    if (__get_user(u32Size, &(((RLD_MMAPInfo_t __user *)arg)->u32Length)))
        return EFAULT;
    if (!u32PhyAddr)
        return EFAULT;
    if (!u32Size)
        return EFAULT;
    u32VirtAddr = (U32)__va((void *)u32PhyAddr);
    //RLD_PRINT("flush dcache %lx size %lx\n", u32PhyAddr,u32Size);
	_dma_cache_wback_inv(u32VirtAddr,u32Size);
    return 0;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
static int _mod_rld_open (struct inode *inode, struct file *filp)
{
    RLD_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static int _mod_rld_release(struct inode *inode, struct file *filp)
{
    RLD_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static int _mod_rld_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int         err= 0;
    U8       u8Value;
    U16      u16Value;
    U32      u32Value1,u32Value2, u32Value3, u32Value4;

    //RLD_PRINT("%s: 0x%02x: is invoked\n", __FUNCTION__, _IOC_NR(cmd));

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (RLD_IOC_MAGIC!= _IOC_TYPE(cmd))
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

    PROBE_IO_ENTRY(MDRV_MAJOR_RLD, _IOC_NR(cmd));

    switch(cmd) {
    	case MDRV_RLD_IOC_RLD_START:
    		MHal_RLD_Start();
    		break;
        case MDRV_RLD_IOC_RLD_RESET:
            MHal_RLD_Reset();
            break;
        case MDRV_RLD_IOC_SET_OUTPUTFORMAT:
            if (__get_user(u8Value, (U8  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            //RLD_PRINT("mdrv: output format %x\n", (U16)u8Value);
            MHal_RLD_SetOutputFormat(u8Value);
            break;
        case MDRV_RLD_IOC_SET_ENLARGE_RATE:
            if (__get_user(u8Value, (U8  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetEnlargeRate(u8Value);
            break;
        case MDRV_RLD_IOC_SET_TRANSPARENT_KEY:
            if (__get_user(u8Value, (U8  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetTransparentKey(u8Value);
            break;
        case MDRV_RLD_IOC_SET_TOPFIELD_LENGTH:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetTopFieldLength(u16Value);
            break;

        case MDRV_RLD_IOC_SET_BOTTOMFIELD_LENGTH:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetBottomFieldLength(u16Value);
            break;
        case MDRV_RLD_IOC_SET_REGION_WIDTH:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetRegionWidth(u16Value);
            break;
        case MDRV_RLD_IOC_SET_REGION_HEIGHT:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetRegionHeight(u16Value);
            break;
        case MDRV_RLD_IOC_SET_REGION_PITCH:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetRegionPitch(u16Value);
            break;
        case MDRV_RLD_IOC_SET_OBJ_XOFFSET:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetObjXOffset(u16Value);
            break;
        case MDRV_RLD_IOC_SET_OBJ_YOFFSET:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetObjYOffset((U16)u16Value);
            break;
        case MDRV_RLD_IOC_SET_REGION_OFFSET:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetRegionOffset(u16Value);
            break;
        case MDRV_RLD_IOC_SET_REGION_DEPTH:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetRegionDepth(u16Value);
            break;
/*
        case MDRV_RLD_IOC_SET_REGION_COLORKEYFG:
            MHal_RLD_SetRegionColorKeyFG((U8)arg);
            break;
        case MDRV_RLD_IOC_SET_REGION_COLORKEYBG:
            MHal_RLD_SetRegionColorKeyBG((U8)arg);
            break;*/
        case MDRV_RLD_IOC_SET_TOPFIELD_ADDRESS:
            _MDrv_RLD_SetTopFieldAddress(arg);
            break;

        case MDRV_RLD_IOC_SET_BOTTOMFIELD_ADDRESS:
            _MDrv_RLD_SetBottomFieldAddress(arg);
            break;
        case MDRV_RLD_IOC_SET_OUTPUT_ADDRESS:
            _MDrv_RLD_SetOutputAddress(arg);
            break;

        case MDRV_RLD_IOC_SET_MAPPINGTABLE_2TO4:
            if (__get_user(u16Value, (U16  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetMappingTable2to4(u16Value);
            break;

        case MDRV_RLD_IOC_SET_MAPPINGTABLE_2TO8:
            if (__get_user(u32Value1, (U32  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetMappingTable2to8(u32Value1);
            break;
        case MDRV_RLD_IOC_SET_MAPPINGTABLE_4TO8:
            if (__get_user(u32Value1, &(((RLD_4to8Mapping_t __user *)arg)->u32Value1)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            if (__get_user(u32Value2, &(((RLD_4to8Mapping_t __user *)arg)->u32Value2)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            if (__get_user(u32Value3, &(((RLD_4to8Mapping_t __user *)arg)->u32Value3)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            if (__get_user(u32Value4, &(((RLD_4to8Mapping_t __user *)arg)->u32Value4)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetMappingTable4to8(u32Value1, u32Value2, u32Value3, u32Value4);
            break;
        case MDRV_RLD_IOC_RLD_GETRETURN:
      	    u8Value = MHal_RLD_GetReturn();
            if (__put_user(u8Value, (int __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
      	    break;
        case MDRV_RLD_IOC_RLD_WAITRETURN:
      	    u8Value = MHal_RLD_WaitReturn();
            if (__put_user(u8Value, (int __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
      	    break;
        case MDRV_RLD_IOC_PATCH_CLRCOLORMAPPING:
            MHal_RLD_ClearColorMappingPatch();
            break;
        case MDRV_RLD_IOC_PATCH_SETCOLORMAPPING:
            MHal_RLD_SetColorMappingPatch();
            break;
        case MDRV_RLD_IOC_PATCH_COLORREDUCTION:
            MHal_RLD_SetColorReductionPatch();
            break;
        case MDRV_RLD_IOC_PATCH_OUTOFWIDTH:
            MHal_RLD_SetOutOfWidthPatch();
            break;
        case MDRV_RLD_IOC_PATCH_ENDLASTDATA:
            MHal_RLD_SetEndLastDataPatch();
            break;
        case MDRV_RLD_IOC_RLD_INIT:
            MHal_RLD_Init();
            break;
        case MDRV_RLD_IOC_RLD_SETCOLORKEYFG:
            if (__get_user(u8Value, (U8  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetRegionColorKeyFG(u8Value);
            break;
        case MDRV_RLD_IOC_RLD_SETCOLORKEYBG:
            if (__get_user(u8Value, (U8  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetRegionColorKeyBG(u8Value);
            break;
        case MDRV_RLD_IOC_PATCH_NONMODIFY:
            MHal_RLD_SetNonModifyingPatch();
            break;
        case MDRV_RLD_IOC_GET_MMAPINFO:
            _MDrv_RLD_GetMMAPInfo(arg);
            break;
        case MDRV_RLD_IOC_FLUSHDCACHE:
            _MDrv_RLD_FlushDCache(arg);
            break;
        case MDRV_RLD_IOC_2BS_RESET:
            MHal_RLD_2BS_Reset();
            break;
        case MDRV_RLD_IOC_2BS_START:
            MHal_RLD_2BS_Start();
            break;
        case MDRV_RLD_IOC_2BS_GETRETURN:
            u8Value = MHal_RLD_2BS_GetReturn();
            if (__put_user(u8Value, (int __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            break;
        case MDRV_RLD_IOC_2BS_SETCTRL:
            if (__get_user(u8Value, (U8  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_2BS_SetCtrl(u8Value);
            break;
        case MDRV_RLD_IOC_SETNONMODIFYCOLOR:
            if (__get_user(u8Value, (U8  __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
                return EFAULT;
            }
            MHal_RLD_SetNonModifying_Color(u8Value);
            break;
        default:
            RLD_WARNING("Unknown ioctl command %d\n", cmd);
            PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
            return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_RLD, _IOC_NR(cmd));
    return 0;
}

static int _mod_rld_mmap(struct file *filp, struct vm_area_struct *vma)
{
    if (!_u32RLDBaseAddr)
        return -EFAULT;
    vma->vm_pgoff = _u32RLDBaseAddr >> PAGE_SHIFT;
    //RLD_PRINT("page shift %d\n", PAGE_SHIFT);
    if (io_remap_pfn_range(vma, vma->vm_start,
                            _u32RLDBaseAddr >> PAGE_SHIFT, _u32RLDBaseLength,
                             vma->vm_page_prot))
    {
        return -EAGAIN;
    }
    return 0;
}

static int __init mod_rld_init(void)
{
    int         s32Ret;
    dev_t       dev;

    RLD_PRINT("%s is invoked\n", __FUNCTION__);

    if (RldDev.s32RldMajor)
    {
        RLD_PRINT("call register_chrdev_region\n");
        dev=                    MKDEV(RldDev.s32RldMajor, RldDev.s32RldMinor);
        s32Ret=                 register_chrdev_region(dev, MOD_RLD_DEVICE_COUNT, MDRV_NAME_RLD);
    }
    else
    {
        RLD_PRINT("call alloc_chrdev_region\n");
        s32Ret=                 alloc_chrdev_region(&dev, RldDev.s32RldMinor, MOD_RLD_DEVICE_COUNT, MDRV_NAME_RLD);
        RldDev.s32RldMajor=     MAJOR(dev);
    }
    if (0> s32Ret)
    {
        RLD_PRINT("Unable to get major %d\n", RldDev.s32RldMajor);
        RLD_WARNING("Unable to get major %d\n", RldDev.s32RldMajor);
        return s32Ret;
    }

    cdev_init(&RldDev.cDevice, &RldDev.RldFop);
    if (0!= (s32Ret= cdev_add(&RldDev.cDevice, dev, MOD_RLD_DEVICE_COUNT)))
    {
        RLD_PRINT("Unable add a character device\n");
        RLD_WARNING("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_RLD_DEVICE_COUNT);
        return s32Ret;
    }

    // request RLD_BASE & RLD_SIZE
    if (MDrv_SYS_GetMMAP(E_SYS_MMAP_RLD_BUF, &_u32RLDBaseAddr, &_u32RLDBaseLength))
        _u32RLDBaseAddr = (_u32RLDBaseAddr +0x1000) & 0xfffff000;
    RLD_PRINT("Init RLD successfully\n");
    return 0;
}

static void __exit mod_rld_exit(void)
{
    RLD_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&RldDev.cDevice);
    unregister_chrdev_region(MKDEV(RldDev.s32RldMajor, RldDev.s32RldMinor), MOD_RLD_DEVICE_COUNT);
}

module_init(mod_rld_init);
module_exit(mod_rld_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("RLD driver");
MODULE_LICENSE("MSTAR");
