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
#define _MDRV_TTX_C
///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_ttx.c
/// This file contains the Mstar driver  for Teletext
/// @author MStar Semiconductor Inc.
/// @brief  Teletext module
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
#include <linux/page-flags.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm-mips/io.h>
#include <asm-mips/string.h>

#include "chip_int.h"
#include "mhal_ttx.h"
#include "mhal_ttx_reg.h"
#include "mhal_ttx_types.h"
#include "mdrv_types.h"
#include "mdrv_ttx.h"
#include "mdrv_ttx_io.h"
#include "mdrv_ttx_st.h"
#include "mdrv_ttx_interrupt.h"
#include "Board.h"

// -----------------------------------------------
//
// golbal variable.
//
extern U32 gu32DataRateTTX;
extern U32 _u32PrevTTXPacketCount;

// -----------------------------------------------
//
// Local function prototypes.
//
void _MDrv_TTXMM_Open(struct vm_area_struct *vma);
void _MDrv_TTXMM_Close(struct vm_area_struct *vma);
int _MDrv_TTXMM_MMAP(struct file *filp, struct vm_area_struct *vma);
static int _MDrv_TTX_Open(struct inode *inode, struct file *filp);
static int _MDrv_TTX_Release(struct inode *inode, struct file *filp);
static int _MDrv_TTX_IOCtl(struct inode *inode, struct file *filp, U32 u32Cmd, unsigned long u32Arg);
int _MDrv_TTXMM_MMAP(struct file *filp, struct vm_area_struct *vma);

// -----------------------------------------------
//
// TTX device variable.
//
/* device id */
static U32  _u32majorTTX    = 170;
module_param(_u32majorTTX, int, 0);

/* TTX device cdev */
static struct cdev  _stDevTTX[1];

/* MMap use counter */
U32                 _u32VMASTTX = 0;

/* TTX parameters */
volatile struct VBI_PARAMETERS   *_tpVBIParametersTTX  = 0;
U8                  *_pu8VPS_TTX = 0;
U8                  *_pu8WSS_TTX = 0;

B16                 _bIRQ_ISR_ReadyTTX = FALSE;

/* VBIBufTTX is a ring buffer for VBI slicer. */
U8                  *_pu8VBIBufTTX = NULL;

/* wait queue */
static wait_queue_head_t wq_ttx;
static atomic_t     _aWaitQueueGuard = ATOMIC_INIT(0);

struct vm_operations_struct MDr_TTX_MMAP_OPS = {
	.open =     _MDrv_TTXMM_Open,
	.close =    _MDrv_TTXMM_Close,
//	.nopage =   _MDrv_TTXMM_NoPage,
};


/* Our various sub-devices. */
static struct file_operations ttx_ops = {
	.owner      = THIS_MODULE,
	.open       = _MDrv_TTX_Open,
	.release    = _MDrv_TTX_Release,
	.ioctl      = _MDrv_TTX_IOCtl,
	.mmap       = _MDrv_TTXMM_MMAP,
};

// -----------------------------------------------
//
// device functions
//
B16 MDrv_Wait(void)
{
    DEFINE_WAIT(waitentry_ttx);

    /* if systems stops TTX interrupt, there is no need to wait TTX */
    if(atomic_read(&_aInterruptStop) == 0x01)
        return FALSE;

    if(_tpVBIParametersTTX == NULL)
        return FALSE;

    /* wait queue only accepts one thread to wait TTX */
    if(atomic_read(&_aWaitQueueGuard) == 0x00)
    {
        prepare_to_wait(&wq_ttx, &waitentry_ttx, TASK_INTERRUPTIBLE);

        atomic_set(&_aWaitQueueGuard, 0x01);
        schedule();
        finish_wait(&wq_ttx, &waitentry_ttx);
    }

    if(atomic_read(&_aInterruptStop) == 0x01)
        return FALSE;

    return TRUE;
}


void MDrv_Wakeup(void)
{
    if(atomic_read(&_aWaitQueueGuard) == 0x01)
    {
        wake_up(&wq_ttx);
        atomic_set(&_aWaitQueueGuard, 0x00);
    }
}


static int _MDrv_TTX_Open(struct inode *inode, struct file *filp)
{
    TTX_DRV_DBG("\n");
    TTX_DRV_DBG("open %d\n", _u32majorTTX);

    // change dram access mode
    // 0x371F, BIT7 must be always 1 (otherwise the DMA access related function could be error)
    _MHal_W1BM(BK_VBI_1F, 0x8E, 0x8E);

    /* initial global variable */
    _tpVBIParametersTTX = NULL;
    _pu8VPS_TTX = NULL;
    _pu8WSS_TTX = NULL;
    _u32VMASTTX = 0;
    _pu8VBIBufTTX = 0;
    _bIRQ_ISR_ReadyTTX = FALSE;

    /* thread states initial */
    init_waitqueue_head(&wq_ttx);

	return 0;
}


/* Release ttx device */
static void _MDrv_TTX_IOCTRL_Release(void)
{
    MHal_TTX_EnableInterrupt(0);
    MHal_TTX_OnOffVBISlicer(0);

    /* if _u32VMASTTX == 0 means this mmap doesn't be referenced by other appilation and we can free it. */
    if(_u32VMASTTX == 0)
    {
        if(_pu8VBIBufTTX != NULL)
        {
            TTX_DRV_DBG("Free VBI buffer and stop interrupter \n");
            if(_bIRQ_ISR_ReadyTTX == TRUE)
            {
                free_irq(E_IRQ_VBI, &_stDevTTX[0]);
                _bIRQ_ISR_ReadyTTX = FALSE;
            }

            _pu8VBIBufTTX = 0;
            _VBIBufferUnitTTX = 0;
            _VBIBufferBaseAddrTTX = 0;
            _tpVBIParametersTTX = 0;
            _pu8VPS_TTX = 0;
            _pu8WSS_TTX = 0;

        }
    }
}

/* Release ttx device */
static int _MDrv_TTX_Release(struct inode *inode, struct file *filp)
{
    TTX_DRV_DBG("release %d\n", _u32majorTTX);

    _MDrv_TTX_IOCTRL_Release();


	return 0;
}

/* Set up the cdev structure for vd device.  */
static void _MDrv_TTX_SetupCDev(struct cdev *dev, int minor,
		struct file_operations *fops)
{
	int err, devno = MKDEV(_u32majorTTX, minor);

	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add (dev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		TTX_DRV_DBG("Error %d adding simple%d", err, minor);
}

/* Reset ttx buffer & variable */
static void _MDrv_TTX_Reset(void)
{
    // reset variable
    _tpVBIParametersTTX->_u32GetAdr = 0;
    _tpVBIParametersTTX->_u32PutAdr = 0;
    _tpVBIParametersTTX->_u32PacketUnitNo = 0;          // is not used
    _tpVBIParametersTTX->_u32Overflow = 0;
    _tpVBIParametersTTX->_u32PacketUnit = _VBIBufferUnitTTX;
    _tpVBIParametersTTX->_u32Protect = 0x0;             // is not used
    _tpVBIParametersTTX->_u32VPS = 0;
    _tpVBIParametersTTX->_u32WSS = 0;
    // reset variable used in mdrv_ttx_interrupt.c
    gu32DataRateTTX = 0;
    _u32PrevTTXPacketCount = (U32)(-1);
}

/* Reset VPS */
static void _MDrv_TTX_ResetVPS(void)
{
    memset((void*)_pu8VPS_TTX, 0, 15);
}

/* Reset WSS */
static void _MDrv_TTX_ResetWSS(void)
{
    _tpVBIParametersTTX->_u32WSS = 0;
}

static int _MDrv_TTX_IOCtl_TTXInit(void)
{
    /* first step to stop slicer and stop interrupter */
    MHal_TTX_EnableInterrupt(0);
    MHal_TTX_OnOffVBISlicer(0);

    /* if memory buffer is released, driver need to be initialed again */
    if(_pu8VBIBufTTX == NULL)
    {
    	_pu8VBIBufTTX = (u8*)TTX_BUF_ADR;
        if(_pu8VBIBufTTX == NULL)
        {
            TTX_DRV_DBG("cannot alloc TTX_PAGE_NUM pages\n");
            return -ENOMEM;
        }
    }

    /* vbi buffer bottom is at _pu8VBIBufTTX + (TTX_PAGE_NUM*(0x01<<PAGE_SHIFT)/TT_VBI_PACKET_SIZE - 1). */
    /* a dummy space at _pu8VBIBufTTX + (TTX_PAGE_NUM*(0x01<<PAGE_SHIFT)/TT_VBI_PACKET_SIZE) is needed due to
        VBI bug. */
    _VBIBufferUnitTTX = TTX_BUF_LEN/TT_VBI_PACKET_SIZE - 1 - 4;
    //_VBIBufferUnitTTX = BUFFER_PACKET_NUM;

    /* (B-1)        (B+0)       (B+1)           (B+2)   (B+3)   */
    /* VBI END      DUMMY       VBI PARAMETERS  VPS     WSS     */
    _tpVBIParametersTTX = (struct VBI_PARAMETERS *)ioremap((u32)_pu8VBIBufTTX + (_VBIBufferUnitTTX + 1)*TT_VBI_PACKET_SIZE, sizeof(struct VBI_PARAMETERS));
    _pu8VPS_TTX = (U8*)ioremap((u32)_pu8VBIBufTTX + (_VBIBufferUnitTTX + 2)*TT_VBI_PACKET_SIZE, sizeof(struct VBI_PARAMETERS));
    _pu8WSS_TTX = (U8*)ioremap((u32)_pu8VBIBufTTX + (_VBIBufferUnitTTX + 3)*TT_VBI_PACKET_SIZE, sizeof(struct VBI_PARAMETERS));

    TTX_DRV_DBG("VBI buffer addr 0x%x\n",(U32)phys_to_virt((u32)_pu8VBIBufTTX));
    TTX_DRV_DBG("VBI buffer parameters addr 0x%x\n",(U32)_tpVBIParametersTTX);

    /* initial VBI buffer and unit */
    TT_VBI_BUFFER_BASE_ADDR = (U32)phys_to_virt((u32)_pu8VBIBufTTX);
    TT_VBI_BUFFER_UNIT = _VBIBufferUnitTTX;

    /* initial VBI parameters */
    _tpVBIParametersTTX->_u32GetAdr = 0;
    _tpVBIParametersTTX->_u32PutAdr = 0;
    _tpVBIParametersTTX->_u32PacketUnitNo = 0;
    _tpVBIParametersTTX->_u32Overflow = 0;
    _tpVBIParametersTTX->_u32PacketUnit = _VBIBufferUnitTTX;
    _tpVBIParametersTTX->_u32Protect = 0x0;
    _tpVBIParametersTTX->_u32VPS = 0;
    _tpVBIParametersTTX->_u32WSS = 0;

    /* create VBI buffer */
    MHal_TTX_PacketBuffer_Create();
    MHal_TTX_InitVBI();

    if(_bIRQ_ISR_ReadyTTX == FALSE)
    {
        if(request_irq(E_IRQ_VBI, MDrv_TTX_Interrupt, SA_INTERRUPT, "VBI_IRQ", &_stDevTTX[0]) != 0)
        {
            TTX_DRV_DBG("IRQ request failure\n");
            return -EIO;
        }
        _bIRQ_ISR_ReadyTTX = TRUE;
    }
	else {
		disable_irq(E_IRQ_VBI);
		enable_irq(E_IRQ_VBI);
	}
    return 0;
}

static int _MDrv_TTX_IOCtl(struct inode *inode, struct file *filp, U32 u32Cmd, unsigned long u32Arg)
{
    U32 u32ReturnVal;
    int Ret = 0;

    switch(u32Cmd)
    {
        case IOCTL_TTX_INIT:
            if((Ret = _MDrv_TTX_IOCtl_TTXInit()) < 0)
                return Ret;
            break;

        case IOCTL_TTX_RESET_VBI_PARAMETERS:
            break;

        case IOCTL_TTX_GET_PACKET_UNIT:
            if(copy_to_user((U32*)u32Arg, (const U32*)&_VBIBufferUnitTTX, sizeof(U32)))
                return -EFAULT;
            break;

        case IOCTL_TTX_GET_TTX_PARAMETER_OFFSET:

            u32ReturnVal = (_VBIBufferUnitTTX +1)*TT_VBI_PACKET_SIZE;
            if(copy_to_user((U32*)u32Arg, (const U32*)&u32ReturnVal, sizeof(U32)))
                return -EFAULT;
            break;

        case IOCTL_TTX_GET_VPS_OFFSET:
            u32ReturnVal = (_VBIBufferUnitTTX +2)*TT_VBI_PACKET_SIZE;
            if(copy_to_user((U32*)u32Arg, (const U32*)&u32ReturnVal, sizeof(U32)))
                return -EFAULT;
            break;

        case IOCTL_TTX_GET_WSS_OFFSET:
            u32ReturnVal = (_VBIBufferUnitTTX +3)*TT_VBI_PACKET_SIZE;
            if(copy_to_user((U32*)u32Arg, (const U32*)&u32ReturnVal, sizeof(U32)))
                return -EFAULT;
            break;

        case IOCTL_TTX_WAIT_EVENT:
            u32ReturnVal = MDrv_Wait();
            if(copy_to_user((U32*)u32Arg, (const U32*)&u32ReturnVal, sizeof(U32)))
                return -EFAULT;
            break;

        case IOCTL_TTX_RELEASE:
            _MDrv_TTX_IOCTRL_Release();
            break;

        case IOCTL_TTX_ONOFF_SLICER:
            if(_pu8VBIBufTTX != NULL)
                MHal_TTX_OnOffVBISlicer(u32Arg);
            break;

        case IOCTL_TTX_SET_VIDEOSTANDARD:
            MHal_TTX_SetVideoSystem(u32Arg);
            break;

        case IOCTL_TTX_INTERRUPT_ED:
            MHal_TTX_EnableInterrupt((B16)u32Arg);
            break;

        case IOCTL_TTX_UART_OFF:
            _MHal_W1BM(BYTE2REAL(0x1E3C), 0x10, _BIT4);
            break;
        case IOCTL_TTX_GET_TTX_DATARATE:
            u32ReturnVal = gu32DataRateTTX;
            if(copy_to_user((U32*)u32Arg, (const U32*)&u32ReturnVal, sizeof(U32)))
                return -EFAULT;
            break;

        case IOCTL_TTX_BUFFER_RESET:
            memset((void*)TTX_BUF_ADR, 0, TT_VBI_BUFFER_UNIT*TT_VBI_PACKET_SIZE);
            break;

        case IOCTL_TTX_RESET:
            _MDrv_TTX_Reset();
            break;

        case IOCTL_TTX_RESET_VPS:
            _MDrv_TTX_ResetVPS();
            break;

        case IOCTL_TTX_RESET_WSS:
            _MDrv_TTX_ResetWSS();
            break;

        default:
            return -ENOTTY;
    }
	return 0;
}

void _MDrv_TTXMM_Open(struct vm_area_struct *vma)
{
    _u32VMASTTX++;
}

void _MDrv_TTXMM_Close(struct vm_area_struct *vma)
{
    TTX_DRV_DBG("VM CLOSE\n");
	_u32VMASTTX--;
}

int _MDrv_TTXMM_MMAP(struct file *filp, struct vm_area_struct *vma)
{
    TTX_DRV_DBG("VMSIZE = 0x%x\n", (U32)(vma->vm_end - vma->vm_start));
    TTX_DRV_DBG("VBI BUF SIZE = 0x%x\n", (U32)((TT_VBI_BUFFER_UNIT) * TT_VBI_PACKET_SIZE));

    /* no initial */
    if(_pu8VBIBufTTX == 0)
        return -EAGAIN;;


    TTX_DRV_DBG("Initial ...\n");
    /* over size */
    //if(vma->vm_end - vma->vm_start > TTX_PAGE_NUM*(0x01<<PAGE_SHIFT))
    if(vma->vm_end - vma->vm_start > TTX_BUF_LEN)
        return -EAGAIN;

  	vma->vm_flags |= VM_RESERVED;

	vma->vm_ops = &MDr_TTX_MMAP_OPS;

#if 0
    remap_pfn_range(vma, vma->vm_start, (virt_to_phys(_pu8VBIBufTTX)>>PAGE_SHIFT),
        vma->vm_end - vma->vm_start, vma->vm_page_prot);
#else
    remap_pfn_range(vma, vma->vm_start, (u32)_pu8VBIBufTTX>>PAGE_SHIFT,
        vma->vm_end - vma->vm_start, vma->vm_page_prot);
#endif
#if 0
    TTX_DRV_DBG("VMSTART = 0x%x, VMEND = 0x%x, PFN = 0x%x\n",
        (U32)(vma->vm_start), (U32)(vma->vm_end), (U32)(virt_to_phys(_pu8VBIBufTTX)>>PAGE_SHIFT));
#else
	TTX_DRV_DBG("VMSTART = 0x%x, VMEND = 0x%x, PFN = 0x%x\n",
			(U32)(vma->vm_start), (U32)(vma->vm_end), (u32)_pu8VBIBufTTX>>PAGE_SHIFT);
#endif
	_MDrv_TTXMM_Open(vma);

    TTX_DRV_DBG("Done.\n");
	return 0;
}


/* Module housekeeping. */
static int _MDrv_TTX_Init(void)
{
	int result;
	dev_t dev = MKDEV(_u32majorTTX, 0);

	/* Figure out our device number. */
	if (_u32majorTTX)
		result = register_chrdev_region(dev, 1, "TTX");
	else {
		result = alloc_chrdev_region(&dev, 0, 1, "TTX");
		_u32majorTTX = MAJOR(dev);
	}
	if (result < 0) {
		TTX_DRV_DBG("unable to get major %d\n", _u32majorTTX);
		return result;
	}
	if (_u32majorTTX == 0)
		_u32majorTTX = result;

	_MDrv_TTX_SetupCDev(_stDevTTX, 0, &ttx_ops);

    TTX_DRV_DBG("init %d\n", _u32majorTTX);
	return 0;
}


static void _MDrv_TTX_Cleanup(void)
{
	cdev_del(_stDevTTX);
	unregister_chrdev_region(MKDEV(_u32majorTTX, 0), 2);
}


module_init(_MDrv_TTX_Init);
module_exit(_MDrv_TTX_Cleanup);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("TTX driver");
MODULE_LICENSE("MSTAR");

