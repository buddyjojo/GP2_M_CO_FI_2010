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
/// file    drvGOP.c
/// @author MStar Semiconductor Inc.
/// @brief  GOP Device Driver
///////////////////////////////////////////////////////////////////////////////////////////////////



#include <linux/module.h>
#include <linux/fs.h>    // for MKDEV()
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>

#include "mst_devid.h"
#include "mdrv_types.h"
#include "mdrv_system.h"
#include "mhal_gop.h"
#include "mdrv_gop.h"
#include "mhal_gop_interrupt.h"


#include "mdrv_probe.h"

#define TEST_VMA 1

#if TEST_VMA
#include <linux/mm.h>

static int MDrv_GOP_MMap(struct file *filp, struct vm_area_struct *vma);
#endif

static int MDrv_GOP_Open(struct inode *inode, struct file *filp);
static int MDrv_GOP_Release(struct inode *inode, struct file *filp);
static int MDrv_GOP_IOCtl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

static int MDrv_GOP_TrueMotionDemo(unsigned long arg);	// 20080805 LGE Added for TRUE Motion Demo.

//------------------------------------------------------------------------------
// structure
//------------------------------------------------------------------------------
struct MS_GOP_DEV {
	struct cdev cdev;	  /* Char device structure		*/
#if TEST_VMA
  	int vmas;                 /* active mappings */
#endif
};

struct file_operations _gop_fops = {
	.owner =    THIS_MODULE,
	.ioctl =    MDrv_GOP_IOCtl,
	.open =     MDrv_GOP_Open,
	.release =  MDrv_GOP_Release,
#if TEST_VMA
	.mmap =	    MDrv_GOP_MMap,
#endif
};

// -----------------------------------------------------------------------------
// Global variable
// -----------------------------------------------------------------------------
int _GOP_Major =    MDRV_MAJOR_GOP; //MDEV_MAJOR_GOP;
int _GOP_Minor =    MDRV_MINOR_GOP; //MDEV_MINOR_GOP;
int _GOP_Num_Devs = MDRV_GOP_NR_DEVS;	/* number of bare devGOP devices */

static U32 u32InterruptEnable;
static BOOL bIsVMirror=FALSE, bIsHMirror=FALSE;

struct MS_GOP_DEV *_pGOP_Devices;	/* allocated in scull_init_module */

static MS_GOP_CREATE_GWIN   gGWINInfo[3] ;

#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
extern FPTR_CHECKTAGID fptrCheckTagId;
#endif
// -----------------------------------------------------------------------------
// Local function
// -----------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                     GOP Function
//------------------------------------------------------------------------------
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
U32 MDrv_GOP_RegisterInterrupt(void)
{
    if(0 == u32InterruptEnable)//need register
    {
        if(0 != (request_irq(E_IRQ_GOP,gop_interrupt,SA_INTERRUPT,"gop",NULL)))
        {
            printk("[GOP] Fail to request IRQ %d\n", E_IRQ_GOP);
            return EFAULT;
        }

        printk("[GOP] request IRQ %d\n", (U32)E_IRQ_GOP);
        u32InterruptEnable = 1;
    }
    else
    {
        printk("[GOP] already to request IRQ %d!!!\n", E_IRQ_GOP);
    }
    return 0;
}

U32 MDrv_GOP_DeRegisterInterrupt(void)
{
    //if(1 == u32InterruptEnable){
        printk("[GOP] dismiss IRQ %d\n", E_IRQ_GOP);
        free_irq(E_IRQ_GOP,NULL);
        u32InterruptEnable = 0;
    //}
    return 0;
}
#endif
int MDrv_GOP_IOC_Init(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;

    // clear gwin back info
    memset( gGWINInfo, 0, sizeof(gGWINInfo) ) ;

    if (__get_user(eGOP_Type, (GOP_HW_Type __user *)arg))
    {
        return EFAULT;
    }

    if (MHal_GOP_Init(eGOP_Type))
    {
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
        return MDrv_GOP_RegisterInterrupt();
#else
        return 0;
#endif
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_GWIN_ForceWrite(unsigned long arg)
{
    MS_GOP_GWIN_FORCEWRITE stGWIN_FroceWrite;

    if (copy_from_user(&stGWIN_FroceWrite, (MS_GOP_GWIN_FORCEWRITE __user *)arg, sizeof(MS_GOP_GWIN_FORCEWRITE)))
        return EFAULT;

    if (MHal_GOP_GWIN_ForceWrite(stGWIN_FroceWrite.eGOP_Type, stGWIN_FroceWrite.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_DstPlane_Set(unsigned long arg)
{
    MS_GOP_SET_DSTPLANE stSetDstPlane;

    if (copy_from_user(&stSetDstPlane, (MS_GOP_SET_DSTPLANE __user *)arg, sizeof(MS_GOP_SET_DSTPLANE)))
        return EFAULT;

    if (MHal_GOP_DstPlane_Set(stSetDstPlane.eGOP_Type, stSetDstPlane.eDstType))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_Palette_Set(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;
    GopPaletteEntry *pGopPalArray;
    U32 u32PalStart;
    U32 u32PalEnd;
    U32 u32PalSize;
    GopPalType ePalType;
    int iRet = 0;

    if (__get_user(eGOP_Type, &((MS_GOP_SET_PALETTE __user *)arg)->eGOP_Type))
        iRet = EFAULT;
    if (__get_user(u32PalStart, &((MS_GOP_SET_PALETTE __user *)arg)->u32PalStart))
        iRet = EFAULT;
    if (__get_user(u32PalEnd, &((MS_GOP_SET_PALETTE __user *)arg)->u32PalEnd))
        iRet = EFAULT;
    if (__get_user(ePalType, &((MS_GOP_SET_PALETTE __user *)arg)->enPalType))
        iRet = EFAULT;

    u32PalSize = u32PalEnd - u32PalStart + 1;

    pGopPalArray = kmalloc(sizeof(GopPaletteEntry) * u32PalSize, GFP_KERNEL);

    if (pGopPalArray)
    {
        if (copy_from_user(pGopPalArray, ((MS_GOP_SET_PALETTE __user *)arg)->pPalArray, sizeof(GopPaletteEntry)*u32PalSize))
        {
            //iRet = EFAULT;
            kfree(pGopPalArray);    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
            return EFAULT;
        }

        if (MHal_GOP_Palette_Set(eGOP_Type, pGopPalArray, u32PalStart, u32PalEnd))
        {
            iRet = EFAULT;
        }

        kfree(pGopPalArray);
    }
    else
    {
        iRet = EFAULT;
    }

    return iRet;

}

int MDrv_GOP_IOC_Palette_Read(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;
    GopPaletteEntry *pGopPalArray;
    U32 u32PalStart;
    U32 u32PalEnd;
    U32 u32PalSize;
    GopPalType ePalType;
    int iRet = 0;

    if (__get_user(eGOP_Type, &((MS_GOP_SET_PALETTE __user *)arg)->eGOP_Type))
        iRet = EFAULT;
    if (__get_user(u32PalStart, &((MS_GOP_SET_PALETTE __user *)arg)->u32PalStart))
        iRet = EFAULT;
    if (__get_user(u32PalEnd, &((MS_GOP_SET_PALETTE __user *)arg)->u32PalEnd))
        iRet = EFAULT;
    if (__get_user(ePalType, &((MS_GOP_SET_PALETTE __user *)arg)->enPalType))
        iRet = EFAULT;

    u32PalSize = u32PalEnd - u32PalStart + 1;

    pGopPalArray = kmalloc(sizeof(GopPaletteEntry) * u32PalSize, GFP_KERNEL);

    if (pGopPalArray)
    {
        if (MHal_GOP_Palette_Read(eGOP_Type, pGopPalArray, u32PalStart, u32PalEnd))
        {
            iRet = EFAULT;
        }
        copy_to_user(((MS_GOP_SET_PALETTE __user *)arg)->pPalArray, pGopPalArray, sizeof(GopPaletteEntry)*u32PalSize);


        kfree(pGopPalArray);
    }
    else
    {
        iRet = EFAULT;
    }

    return iRet;

}

int MDrv_GOP_IOC_Output_Progressive_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stEnableOutputProgressive;

    if (copy_from_user(&stEnableOutputProgressive, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(MS_GOP_ENABLE_GOP_FUNC)))
        return EFAULT;

    if (MHal_GOP_Output_Progressive_Enable(stEnableOutputProgressive.eGOP_Type, stEnableOutputProgressive.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

#if 0
int MDrv_GOP_IOC_Output_HDup_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stEnableOutputDump;

    if (copy_from_user(&stEnableOutputDump, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(MS_GOP_ENABLE_GOP_FUNC)))
        return EFAULT;

    if (MHal_GOP_Output_HDup_Enable(stEnableOutputDump.eGOP_Type, stEnableOutputDump.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_Output_VDup_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stEnableOutputDump;

    if (copy_from_user(&stEnableOutputDump, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(MS_GOP_ENABLE_GOP_FUNC)))
        return EFAULT;

    if (MHal_GOP_Output_VDup_Enable(stEnableOutputDump.eGOP_Type, stEnableOutputDump.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}
#endif

U16 MDrv_GOP_IOC_YUVOut_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stEnableYUV;

    if (copy_from_user(&stEnableYUV, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(stEnableYUV)))
        return EFAULT;

    if (MHal_GOP_YUVOut_Enable(stEnableYUV.eGOP_Type, stEnableYUV.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_Create(unsigned long arg)
{
    MS_GOP_CREATE_GWIN stCreateGWin;

    if (copy_from_user(&stCreateGWin, (MS_GOP_CREATE_GWIN __user *)arg, sizeof(MS_GOP_CREATE_GWIN)))
        return EFAULT;

    // backup the latest setting info
    memcpy( &(gGWINInfo[stCreateGWin.eGOP_Type]), &stCreateGWin, sizeof(stCreateGWin) ) ;

    if (MHal_GOP_GWIN_Create(stCreateGWin.eGOP_Type, stCreateGWin.u8Wid, stCreateGWin.eColorType,
                            stCreateGWin.u32SrcX, stCreateGWin.u32SrcY, stCreateGWin.u32DispX, stCreateGWin.u32DispY,
                            stCreateGWin.u32Width, stCreateGWin.u32Height, stCreateGWin.u32DRAMRBlkStart, stCreateGWin.u32DRAMRBlkHSize,
                            stCreateGWin.u32DRAMRBlkVSize))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_INFO(unsigned long arg)
{
    MS_GOP_CREATE_GWIN* p;
    int i, pitch ;

    for( i=0; i<4; i++ ){
        pitch = 0 ;
        p = gGWINInfo+i ;
        printk( "\nGOP[%d] info-----------------------------------\n", i ) ;
        printk( "p->eGOP_Type=%d\n", p->eGOP_Type ) ;
        printk( "p->u8Wid=%d\n", p->u8Wid ) ;
        printk( "p->u32SrcX=%d\n", p->u32SrcX ) ;
        printk( "p->u32SrcY=%d\n", p->u32SrcY ) ;
        printk( "p->u32DispX=%d\n", p->u32DispX ) ;
        printk( "p->u32DispY=%d\n", p->u32DispY ) ;
        printk( "p->u32Width=%d\n", p->u32Width ) ;
        printk( "p->u32Height=%d\n", p->u32Height ) ;

        printk( "p->u32DRAMRBlkStart=0x%08X\n", p->u32DRAMRBlkStart ) ;
        printk( "p->u32DRAMRBlkHSize=%d\n", p->u32DRAMRBlkHSize ) ;
        printk( "p->u32DRAMRBlkVSize=%d\n", p->u32DRAMRBlkVSize ) ;

        printk( "p->eColorTyped=" ) ;
        switch(p->eColorType){
        case E_GOP_COLOR_RGB555_BLINK:
            printk( "E_GOP_COLOR_RGB555_BLINK\n" ) ;
            break ;
        case E_GOP_COLOR_RGB565:
            printk( "E_GOP_COLOR_RGB565\n" ) ;
            break ;
        case E_GOP_COLOR_ARGB4444:
            printk( "E_GOP_COLOR_ARGB4444\n" ) ;
            pitch = (((((p->u32DRAMRBlkHSize) << 1)+15)>>4)<<4) ;
            break ;
        case E_GOP_COLOR_I8:
            printk( "E_GOP_COLOR_I8\n" ) ;
            pitch = p->u32DRAMRBlkHSize ;
            break ;
        case E_GOP_COLOR_ARGB8888:
            printk( "E_GOP_COLOR_ARGB8888\n" ) ;
            pitch = (((((p->u32DRAMRBlkHSize) << 2)+15)>>4)<<4) ;
            break ;
        default:
            printk( "unknown %d\n", p->eColorType ) ;
            break ;
        }

        printk( "p->pitch=%d\n", pitch ) ;
        printk( "p->mem size=0x%08X\n", (pitch*(p->u32DRAMRBlkVSize)) ) ;

    }

    return 0;

}




int MDrv_GOP_IOC_Blink_SetRate(unsigned long arg)
{
    MS_GOP_SET_BLINK_RATE stSetBlinkRate;

    if (copy_from_user(&stSetBlinkRate, (MS_GOP_SET_BLINK_RATE __user *)arg, sizeof(MS_GOP_SET_BLINK_RATE)))
        return EFAULT;

    if (MHal_GOP_Blink_SetRate(stSetBlinkRate.eGOP_Type, stSetBlinkRate.u32Rate))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_Blink_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stEnableBlink;

    if (copy_from_user(&stEnableBlink, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(MS_GOP_ENABLE_GOP_FUNC)))
        return EFAULT;

    if (MHal_GOP_Blink_Enable(stEnableBlink.eGOP_Type, stEnableBlink.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_Scroll_SetRate(unsigned long arg)
{
    MS_GOP_SET_SCROLL_RATE stSetScrollRate;

    if (copy_from_user(&stSetScrollRate, (MS_GOP_SET_SCROLL_RATE __user *)arg, sizeof(MS_GOP_SET_SCROLL_RATE)))
        return EFAULT;

    if (MHal_GOP_Scroll_SetRate(stSetScrollRate.eGOP_Type, stSetScrollRate.u32Rate))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_Alloc(unsigned long arg)
{
    MS_GOP_ALLOC_FREE_GWIN stAllocGWin;

    if (copy_from_user(&stAllocGWin, (MS_GOP_ALLOC_FREE_GWIN __user *)arg, sizeof(MS_GOP_ALLOC_FREE_GWIN)))
        return EFAULT;

    if (MHal_GOP_GWIN_Alloc(stAllocGWin.eGOP_Type, stAllocGWin.u8Wid))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_Free(unsigned long arg)
{
    MS_GOP_ALLOC_FREE_GWIN stAllocGWin;

    if (copy_from_user(&stAllocGWin, (MS_GOP_ALLOC_FREE_GWIN __user *)arg, sizeof(MS_GOP_ALLOC_FREE_GWIN)))
        return EFAULT;

    if (MHal_GOP_GWIN_Free(stAllocGWin.eGOP_Type, stAllocGWin.u8Wid))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GWIN stEnableGWin;

    if (copy_from_user(&stEnableGWin, (MS_GOP_ENABLE_GWIN __user *)arg, sizeof(MS_GOP_ENABLE_GWIN)))
        return EFAULT;

    if (MHal_GOP_GWIN_Enable(stEnableGWin.eGOP_Type, stEnableGWin.u8Wid, stEnableGWin.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_TransClr_Set(unsigned long arg)
{
    MS_GOP_SET_TRANSCRL stSetTransClr;

    if (copy_from_user(&stSetTransClr, (MS_GOP_SET_TRANSCRL __user *)arg, sizeof(MS_GOP_SET_TRANSCRL)))
        return EFAULT;

    if (MHal_GOP_TransClr_Set(stSetTransClr.eGOP_Type, stSetTransClr.TRSColor))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_TransClr_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stEnableTransClr;

    if (copy_from_user(&stEnableTransClr, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(MS_GOP_ENABLE_GOP_FUNC)))
        return EFAULT;

    if (MHal_GOP_TransClr_Enable(stEnableTransClr.eGOP_Type, stEnableTransClr.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_GWIN_Scroll_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_SCROLL stEnableScroll;

    if (copy_from_user(&stEnableScroll, (MS_GOP_ENABLE_SCROLL __user *)arg, sizeof(MS_GOP_ENABLE_SCROLL)))
        return EFAULT;

    if (MHal_GOP_GWIN_Scroll_Enable(stEnableScroll.eGOP_Type, stEnableScroll.u8Wid,
        stEnableScroll.eScrollType, stEnableScroll.u8ScrollStep, stEnableScroll.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_Blending_Set(unsigned long arg)
{
    MS_GOP_SET_BLENDING stSetBlending;

    if (copy_from_user(&stSetBlending, (MS_GOP_SET_BLENDING __user *)arg, sizeof(MS_GOP_SET_BLENDING)))
        return EFAULT;

    if (MHal_GOP_GWIN_Blending_Set(stSetBlending.eGOP_Type, stSetBlending.u8Wid,
        stSetBlending.blEnable, stSetBlending.u32Alpha))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_GWIN_DispPos_Move(unsigned long arg)
{
    MS_GOP_MOVE_GWIN_DISPPOS stMoveGWinDispPos;

    if (copy_from_user(&stMoveGWinDispPos, (MS_GOP_MOVE_GWIN_DISPPOS __user *)arg, sizeof(MS_GOP_MOVE_GWIN_DISPPOS)))
        return EFAULT;

    if (MHal_GOP_GWIN_DispPos_Move(stMoveGWinDispPos.eGOP_Type, stMoveGWinDispPos.u8Wid,
        stMoveGWinDispPos.u32DispX, stMoveGWinDispPos.u32DispY))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_Scroll_AutoStop_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_SCROLL_AUTOSTOP stEnableScrollAutoStop;

    if (copy_from_user(&stEnableScrollAutoStop, (MS_GOP_ENABLE_SCROLL_AUTOSTOP __user *)arg, sizeof(MS_GOP_ENABLE_SCROLL_AUTOSTOP)))
        return EFAULT;

    if (MHal_GOP_GWIN_Scroll_AutoStop_Enable(stEnableScrollAutoStop.eGOP_Type, stEnableScrollAutoStop.u8Wid,
        stEnableScrollAutoStop.eScrollStopType, stEnableScrollAutoStop.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_Scroll_AutoStop_HSet(unsigned long arg)
{
    MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET stSetAutoStopOffset;

    if (copy_from_user(&stSetAutoStopOffset, (MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET __user *)arg, sizeof(MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET)))
        return EFAULT;

    if (MHal_GOP_GWIN_Scroll_AutoStop_HSet(stSetAutoStopOffset.eGOP_Type,
        stSetAutoStopOffset.u8Wid, stSetAutoStopOffset.u32ScrollAutoStopOffset))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_GWIN_Scroll_AutoStop_VSet(unsigned long arg)
{
    MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET stSetAutoStopOffset;

    if (copy_from_user(&stSetAutoStopOffset, (MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET __user *)arg, sizeof(MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET)))
        return EFAULT;

    if (MHal_GOP_GWIN_Scroll_AutoStop_VSet(stSetAutoStopOffset.eGOP_Type,
        stSetAutoStopOffset.u8Wid, stSetAutoStopOffset.u32ScrollAutoStopOffset))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
int MDrv_GOP_IOC_SetFlipInfo(unsigned long arg)
{
    MS_GOP_FLIP_INFO stFlipInfo;

    if (copy_from_user(&stFlipInfo, (MS_GOP_FLIP_INFO __user *)arg, sizeof(MS_GOP_FLIP_INFO)))
        return EFAULT;

    if (MHal_GOP_IOC_SetFlipInfo(stFlipInfo.u32GopIdx, stFlipInfo.u32GwinIdx, stFlipInfo.u32Addr,stFlipInfo.u32TagId,&stFlipInfo.u32QEntry,&stFlipInfo.u32Result))
    {
        __put_user(stFlipInfo.u32QEntry, &(((MS_GOP_FLIP_INFO __user *)arg)->u32QEntry));
        __put_user(stFlipInfo.u32Result, &(((MS_GOP_FLIP_INFO __user *)arg)->u32Result));
	return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}
#endif

int MDrv_GOP_IOC_Int_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stEnableGOPFunc;

    if (copy_from_user(&stEnableGOPFunc, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(MS_GOP_ENABLE_GOP_FUNC)))
        return EFAULT;

    if (MHal_GOP_Int_Enable(stEnableGOPFunc.eGOP_Type, stEnableGOPFunc.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_Int_GetStatus(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;
    U16 bIntStatus;

    if (__get_user(eGOP_Type, &(((MS_GOP_GET_INT_STATUS __user *)arg)->eGOP_Type)))
        return EFAULT;

    if (MHal_GOP_Int_GetStatus(eGOP_Type, &bIntStatus))
    {
        return __put_user(bIntStatus, ((MS_GOP_GET_INT_STATUS __user *)arg)->pbIntStatus);
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

#if 0
int MDrv_GOP_IOC_Set_Offset(unsigned long arg)
{
    MS_GOP_SET_OFFSET stSetOffset;

    if (copy_from_user(&stSetOffset, (MS_GOP_SET_OFFSET __user *)arg, sizeof(MS_GOP_SET_OFFSET)))
        return EFAULT;

    if (MHal_GOP_Set_Offset(stSetOffset.eGOP_Type, stSetOffset.eDstType, stSetOffset.u16XOffset, stSetOffset.u16YOffset))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}
#endif

int MDrv_GOP_IOC_GWIN_Info_Get(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    GopGwinInfo gopGWinInfo;

    if (__get_user(eGOP_Type, &((MS_GWIN_INFO __user *)arg)->eGOP_Type))
        return EFAULT;
    if (__get_user(u8Wid, &((MS_GWIN_INFO __user *)arg)->u8Wid))
        return EFAULT;

    if (MHal_GOP_GWIN_Info_Get(eGOP_Type, u8Wid, &gopGWinInfo))
    {
        return copy_to_user((( MS_GWIN_INFO __user *)arg)->pGWinInfo, &gopGWinInfo, sizeof(GopGwinInfo));
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_GWIN_Info_Set(unsigned long arg)
{

    GOP_HW_Type eGOP_Type;
    U8 u8Wid = 0 ;
    GopGwinInfo gopGWinInfo;

    if (__get_user(eGOP_Type, &((MS_GWIN_INFO __user *)arg)->eGOP_Type))
        return EFAULT;

    if (__get_user(u8Wid, &((MS_GWIN_INFO __user *)arg)->u8Wid))
        return EFAULT;

    if (copy_from_user(&gopGWinInfo, ((MS_GWIN_INFO __user *)arg)->pGWinInfo, sizeof(GopGwinInfo)))
        return EFAULT;

    if (MHal_GOP_GWIN_Info_Set(eGOP_Type, u8Wid, &gopGWinInfo))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }

}

int MDrv_GOP_IOC_Info_Get(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;
    GopInfo gopInformation;

    if (__get_user(eGOP_Type, &((MS_GOP_GET_INFO __user *)arg)->eGOP_Type))
        return EFAULT;

    if (MHal_GOP_Info_Get(eGOP_Type, &gopInformation))
    {
        return copy_to_user((( MS_GOP_GET_INFO __user *)arg)->pGOPInfo, &gopInformation, sizeof(GopInfo));
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_GWIN_Blink_Palette_Set(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;
    U8 u8WinId;
    GopPaletteEntry *pGopPalArray;
    U32 u32PalNum;
    GopPalType ePalType;
    int iRet = 0;

    if (__get_user(eGOP_Type, &((MS_GOP_SET_BLINK_PALETTE __user *)arg)->eGOP_Type))
        iRet = EFAULT;
    if (__get_user(u8WinId, &((MS_GOP_SET_BLINK_PALETTE __user *)arg)->u8WinId))
        iRet = EFAULT;

    if (__get_user(u32PalNum, &((MS_GOP_SET_BLINK_PALETTE __user *)arg)->u32PalNum))
        iRet = EFAULT;

    if (__get_user(ePalType, &((MS_GOP_SET_BLINK_PALETTE __user *)arg)->ePalType))
        iRet = EFAULT;
//printk("eGOP_Type=%x,u8WinId=%x u32PalNum=%lx\n",eGOP_Type,u8WinId,u32PalNum );
    pGopPalArray = kmalloc(sizeof(GopPaletteEntry) * u32PalNum, GFP_KERNEL);

    if (pGopPalArray)
    {
        if (copy_from_user(pGopPalArray, ((MS_GOP_SET_BLINK_PALETTE __user *)arg)->pPalArray, sizeof(GopPaletteEntry)*u32PalNum))
        {
            iRet = EFAULT;
        }

        if (FALSE == MHal_GOP_GWIN_Blink_Palette_Set(eGOP_Type, u8WinId, pGopPalArray, u32PalNum, ePalType))
        //if (FALSE == MHal_GOP_GWIN_Blink_Palette_Set(eGOP_Type, u8WinId, pGopPalArray, u32PalNum))
        {
            iRet = EFAULT;
        }

        kfree(pGopPalArray);
    }
    else
    {
        iRet = EFAULT;
    }

    return iRet;
}


#if 0
int MDrv_GOP_IOC_TWIN_Create(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;
    GopTwinInfo gopTwinInfo;

    if (__get_user(eGOP_Type, &((MS_GOP_CREATE_TWIN __user *)arg)->eGOP_Type))
        return EFAULT;

    if (copy_from_user(&gopTwinInfo, ((MS_GOP_CREATE_TWIN __user *)arg)->pTwinInfo, sizeof(GopPaletteEntry)))
        return EFAULT;


    if (MHal_GOP_TWIN_Create(eGOP_Type, &gopTwinInfo))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_TWIN_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_TWIN stEnableTWin;

    if (copy_from_user(&stEnableTWin, (MS_GOP_ENABLE_TWIN __user *)arg, sizeof(MS_GOP_ENABLE_TWIN)))
        return EFAULT;


    if (MHal_GOP_TWIN_Enable(stEnableTWin.eGOP_Type, stEnableTWin.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}
#endif

int MDrv_GOP_IOC_GWIN_FADE_Init(unsigned long arg)
{
    MS_GOP_INIT_FADE stInitFade;

    if (copy_from_user(&stInitFade, (MS_GOP_INIT_FADE __user *)arg, sizeof(MS_GOP_INIT_FADE)))
        return EFAULT;


    if (MHal_GOP_GWIN_FADE_Init(stInitFade.eGOP_Type, stInitFade.u8Wid, stInitFade.u8Rate, stInitFade.eFADE_Type))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_GWIN_FADE_TRIGGER(unsigned long arg)
{
    MS_GOP_TRIGGER_FADE stTriggerFade;

    if (copy_from_user(&stTriggerFade, (MS_GOP_TRIGGER_FADE __user *)arg, sizeof(MS_GOP_TRIGGER_FADE)))
        return EFAULT;


    if (MHal_GOP_GWIN_FADE_TRIGGER(stTriggerFade.eGOP_Type, stTriggerFade.u8Wid))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_DWIN_Init(unsigned long arg)
{
    if (MHal_GOP_DWIN_Init())
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_DWIN_Alloc(unsigned long arg)
{
    if (MHal_GOP_DWIN_Alloc())
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_DWIN_Free(unsigned long arg)
{
    if (MHal_GOP_DWIN_Free())
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_DWIN_CaptureStream_Enable(unsigned long arg)
{
    U16 blEnable;
    if (__get_user(blEnable, (U16 __user *)arg))
        return EFAULT;

    if (MHal_GOP_DWIN_CaptureStream_Enable(blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_DWIN_CaptureMode_Set(unsigned long arg)
{
    GopCaptureMode eCaptureMode;

    if (__get_user(eCaptureMode, (GopCaptureMode __user *)arg))
        return EFAULT;

    if (MHal_GOP_DWIN_CaptureMode_Set(eCaptureMode))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

//Arki >>
int MDrv_GOP_IOC_DWIN_InputSourceMode_Set(unsigned long arg)
{
    //GopCaptureMode eCaptureMode;
    EN_GOP_DWIN_DATA_SRC eDwinDataSrcMode;
    if (__get_user(eDwinDataSrcMode, (EN_GOP_DWIN_DATA_SRC __user *)arg))
        return EFAULT;

    if (MHal_GOP_DWIN_InputSrcMode_Set(eDwinDataSrcMode))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_DWIN_CLKGen_Set(unsigned long arg)
{
    //GopCaptureMode eCaptureMode;
    EN_GOP_DWIN_DATA_SRC eDwinDataSrcMode;
    if (__get_user(eDwinDataSrcMode, (EN_GOP_DWIN_DATA_SRC __user *)arg))
        return EFAULT;

    if (MHal_GOP_DWIN_CLKGen_Set(eDwinDataSrcMode))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

//------------------------------------------------------------------------------
/// MPEG Video Decoder(MVD) power on.
/// @return MVDSTATUS_SUCCESSS: Process success.
//------------------------------------------------------------------------------
int MDrv_GOP_PowerOn(void)
{
    _MHal_GOP_Clk_Enable(E_GOP4G_0,  TRUE);
    _MHal_GOP_Clk_Enable(E_GOP2G_1,  TRUE);
    _MHal_GOP_Clk_Enable(E_GOP1G,    TRUE);
    _MHal_GOP_Clk_Enable(E_GOP_DWIN, TRUE);
    _MHal_GOP_Clk_Enable(E_GOP1GX,   TRUE);
    return TRUE;
}


//------------------------------------------------------------------------------
/// MPEG Video Decoder(MVD) power off.
/// @return MVDSTATUS_SUCCESSS: Process success.
//------------------------------------------------------------------------------
int MDrv_GOP_PowerOff(void)
{
    _MHal_GOP_Clk_Enable(E_GOP4G_0,  FALSE);
    _MHal_GOP_Clk_Enable(E_GOP2G_1,  FALSE);
    _MHal_GOP_Clk_Enable(E_GOP1G,    FALSE);
    _MHal_GOP_Clk_Enable(E_GOP_DWIN, FALSE);
    _MHal_GOP_Clk_Enable(E_GOP1GX,   FALSE);
    return TRUE;
}
//Arki <<

int MDrv_GOP_IOC_DWIN_SetScanType(unsigned long arg)
{
    U16 blProgressive;

    if (__get_user(blProgressive, (U16 __user *)arg))
        return EFAULT;

    if (MHal_GOP_DWIN_SetScanType(blProgressive))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_DWIN_WinInfo_Set(unsigned long arg)
{
    GOP_DWIN_INFO gopDWIN_Info;

    if (copy_from_user(&gopDWIN_Info, (GOP_DWIN_INFO __user *)arg, sizeof(GOP_DWIN_INFO)))
        return EFAULT;

    if (MHal_GOP_DWIN_WinInfo_Set(&gopDWIN_Info))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_DWIN_Int_Enable(unsigned long arg)
{
    MS_GOP_DWIN_ENABLE_INT stDWin_Enable_Int;

    //if (copy_from_user(&stDWin_Enable_Int, (MS_GOP_DWIN_ENABLE_INT __user *)arg, sizeof(GOP_DWIN_INFO)))
    //    return EFAULT;

    if (copy_from_user(&stDWin_Enable_Int, (MS_GOP_DWIN_ENABLE_INT __user *)arg, sizeof(MS_GOP_DWIN_ENABLE_INT)))
         return EFAULT;

    if (MHal_GOP_DWIN_Int_Enable(stDWin_Enable_Int.eIntType, stDWin_Enable_Int.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_DWIN_GetIntrStatus(unsigned long arg)
{
    U16 u16IntStatus;

    if (MHal_GOP_DWIN_GetIntrStatus(&u16IntStatus))
    {
        return __put_user(u16IntStatus, (U16 __user *)arg);
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_DWIN_Set_PINPON(unsigned long arg)
{
    MS_GOP_DWIN_SET_PINPON stSetPinpon;

    if (copy_from_user(&stSetPinpon, (MS_GOP_DWIN_SET_PINPON __user *)arg, sizeof(MS_GOP_DWIN_SET_PINPON)))
        return EFAULT;

    if (MHal_GOP_DWIN_Set_PINPON(stSetPinpon.u32Addr, stSetPinpon.u32UpBond))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_UpdateReg(unsigned long arg)
{
    GOP_HW_Type eGOP_Type;

    if (__get_user(eGOP_Type, (GOP_HW_Type __user *)arg))
        return EFAULT;


    _MHal_GOP_UpdateReg((U8)eGOP_Type);
    return 0;
}

int MDrv_GOP_IOC_Clk_Set2(unsigned long arg)
{
    MS_GOP_SET_DSTPLANE stSetClock;

    if (copy_from_user(&stSetClock, (MS_GOP_SET_DSTPLANE __user *)arg, sizeof(MS_GOP_SET_DSTPLANE)))
        return EFAULT;

    if (_MHal_GOP_Clk_Set(stSetClock.eGOP_Type, stSetClock.eDstType))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_StretchWin_Create(unsigned long arg)
{
    MS_GOP_CREATE_STRETCH_WIN stStretchWinCreate;

    if (copy_from_user(&stStretchWinCreate, (MS_GOP_CREATE_STRETCH_WIN __user *)arg, sizeof(MS_GOP_CREATE_STRETCH_WIN)))
        return EFAULT;

    if (MHal_GOP_StretchWin_Create(stStretchWinCreate.eGOP_Type, stStretchWinCreate.u32DispX,
        stStretchWinCreate.u32DispY, stStretchWinCreate.u32Width, stStretchWinCreate.u32Height))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}


int MDrv_GOP_IOC_StretchWin_SetRatio(unsigned long arg)
{
    MS_GOP_STRETCH_WIN_SET_RATIO stStretchWinSetRatio;

    if (copy_from_user(&stStretchWinSetRatio, (MS_GOP_STRETCH_WIN_SET_RATIO __user *)arg, sizeof(MS_GOP_STRETCH_WIN_SET_RATIO)))
        return EFAULT;

    if (MHal_GOP_StretchWin_SetRatio(stStretchWinSetRatio.eGOP_Type,
            stStretchWinSetRatio.u32H_Ratio, stStretchWinSetRatio.u32V_Ratio))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_GWIN_Alpha_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_GWIN_ALPHA stEnableGWinAlpha;

    if (copy_from_user(&stEnableGWinAlpha, (MS_GOP_ENABLE_GWIN __user *)arg, sizeof(MS_GOP_ENABLE_GWIN_ALPHA)))
        return EFAULT;

    if (MHal_GOP_GWIN_Alpha_Enable(stEnableGWinAlpha.eGOP_Type, stEnableGWinAlpha.u8Wid, stEnableGWinAlpha.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_Scaler_Set_GOPSel(unsigned long arg)
{
    MS_IPSEL_GOP ipSelGop;

    if (copy_from_user(&ipSelGop, (MS_IPSEL_GOP __user *)arg, sizeof(MS_IPSEL_GOP)))
        return EFAULT;

    if (MHal_GOP_Scaler_Set_GOPSel(ipSelGop))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}



int MDrv_GOP_IOC_SetFieldInver(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stFieldInverse;

    if (copy_from_user(&stFieldInverse, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(MS_GOP_ENABLE_GOP_FUNC)))
        return EFAULT;

    if (MHal_GOP_SetFieldInver(stFieldInverse.eGOP_Type, stFieldInverse.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}



U32 MDrv_GOP_RegisterInterrupt(unsigned long arg)
{
	GOP_HW_Type eGOP_Type;

    if (__get_user(eGOP_Type, (GOP_HW_Type __user *)arg))
    {
        return EFAULT;
    }

    if(0 == u32InterruptEnable)//need register
    {
        if(0 != (request_irq(E_IRQ_GOP,gop_interrupt,SA_INTERRUPT,"mvd",NULL)))
        {
            printk("[GOP] Fail to request IRQ %d\n", E_IRQ_GOP);
            return EFAULT;
        }

        printk("[GOP] request IRQ %d\n", (U32)E_IRQ_GOP);
        u32InterruptEnable = 1;
    }
    else
    {
        printk("[GOP] already to request IRQ %d!!!\n", E_IRQ_GOP);
    }
    return 0;
}

U32 MDrv_GOP_DeRegisterInterrupt(unsigned long arg)
{
	GOP_HW_Type eGOP_Type;

    if (__get_user(eGOP_Type, (GOP_HW_Type __user *)arg))
    {
        return EFAULT;
    }

    //if(1 == u32InterruptEnable){
        printk("[GOP] dismiss IRQ %d\n", E_IRQ_GOP);
        free_irq(E_IRQ_GOP,NULL);
        u32InterruptEnable = 0;
    //}
    return 0;
}

U32 MDrv_GOP_InfoBackup(unsigned long arg)
{
	GOP_HW_Type eGOP_Type;

    if (__get_user(eGOP_Type, (GOP_HW_Type __user *)arg))
    {
        return EFAULT;
    }

    if (MHal_GOP_InfoBackup(eGOP_Type))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

U32 MDrv_GOP_InfoRestore(unsigned long arg)
{
	GOP_HW_Type eGOP_Type;

    if (__get_user(eGOP_Type, (GOP_HW_Type __user *)arg))
    {
        return EFAULT;
    }

    if (MHal_GOP_InfoRestore(eGOP_Type))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_SetPaletteControl(unsigned long arg)
{
    MS_GOP_ENABLE_GOP_FUNC stPaletteControl;

    if (copy_from_user(&stPaletteControl, (MS_GOP_ENABLE_GOP_FUNC __user *)arg, sizeof(MS_GOP_ENABLE_GOP_FUNC)))
        return EFAULT;

    if (MHal_GOP_SetPaletteControl(stPaletteControl.eGOP_Type, (GopPalCtrlMode) stPaletteControl.blEnable & 0x3))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

int MDrv_GOP_IOC_Test_Pattern(unsigned long arg)
{
    SetPattern TestPattern;

    if (copy_from_user(&TestPattern, (SetPattern __user *)arg, sizeof(SetPattern)))
        return EFAULT;

    //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
    MHal_GOP_Test_Pattern(TestPattern.u8Pattern, TestPattern.u8Param, TestPattern.u16HsPD,
                          TestPattern.u16HStart, TestPattern.u16VStart, TestPattern.u16HSize, TestPattern.u16VSize);
        return 0;
}

// KimTH_091026: HSPD 조정 코드 대응 by Mingchia
int MDrv_GOP_IOC_AdjustHSPD(unsigned long arg)
{
    MS_GOP_ADJUST_HSPD  sGop_HSPD;
    
    if (copy_from_user(&sGop_HSPD, (MS_GOP_ADJUST_HSPD __user *)arg, sizeof(MS_GOP_ADJUST_HSPD)))
        return EFAULT;
    
    MHal_GOP_AdjustHSPD(sGop_HSPD.u8GOP_Type, sGop_HSPD.u16HSPD);
    return 0;
}

U16 MDrv_GOP_IOC_VMirror_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_MIRROR stEnableMirror;

    if (copy_from_user(&stEnableMirror, (MS_GOP_ENABLE_MIRROR __user *)arg, sizeof(stEnableMirror)))
        return EFAULT;

    bIsVMirror = stEnableMirror.blEnable;
    if (MHal_GOP_VMirror_Enable(stEnableMirror.eGOP_Type, stEnableMirror.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

U16 MDrv_GOP_IOC_HMirror_Enable(unsigned long arg)
{
    MS_GOP_ENABLE_MIRROR stEnableMirror;

    if (copy_from_user(&stEnableMirror, (MS_GOP_ENABLE_MIRROR __user *)arg, sizeof(stEnableMirror)))
        return EFAULT;

    bIsHMirror = stEnableMirror.blEnable;
    if (MHal_GOP_HMirror_Enable(stEnableMirror.eGOP_Type, stEnableMirror.blEnable))
    {
        return 0;
    }
    else
    {
        // The return value is not so appropriate.
        return EFAULT;
    }
}

// -----------------------------------------------------------------------------
// Device Methods
// -----------------------------------------------------------------------------

/*
 * Set up the char_dev structure for this device.
 */
static void MDrv_GOP_Setup_CDev(struct MS_GOP_DEV *dev, int index)
{
	int err, devno = MKDEV(_GOP_Major, _GOP_Minor);

	cdev_init(&dev->cdev, &_gop_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &_gop_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding drvGOP %d", err, index);
}

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void MDrv_GOP_CleanUp_Module(void)
{
	int i;
	dev_t devno = MKDEV(_GOP_Major, _GOP_Minor);

	/* Get rid of our char dev entries */
	if (_pGOP_Devices) {
		for (i = 0; i < _GOP_Num_Devs; i++) {
			cdev_del(&_pGOP_Devices[i].cdev);
		}
		kfree(_pGOP_Devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, _GOP_Num_Devs);

}

/*
 * Open and close
 */

int MDrv_GOP_Open(struct inode *inode, struct file *filp)
{
	struct MS_GOP_DEV *dev; /* device information */

	dev = container_of(inode->i_cdev, struct MS_GOP_DEV, cdev);
	filp->private_data = dev; /* for other methods */

    printk("GOP opens successfully\n");

	return 0;          /* success */
}

int MDrv_GOP_Release(struct inode *inode, struct file *filp)
{
    printk("GOP closes successfully\n");

	return 0;
}


int MDrv_GOP_IOCtl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int retval = 0;

	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != MDRV_GOP_IOC_MAGIC)
	{
		printk("IOCtl Type Error!!! (Cmd=%x)\n",cmd);
	 	return -ENOTTY;
	}
	if (_IOC_NR(cmd) > MDRV_GOP_IOC_MAXNR)
	{
		printk("IOCtl NR Error!!! (Cmd=%x)\n",cmd);
	 	return -ENOTTY;
	}

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) {
				printk("IOCtl Error!!! (cmd=%x)\n",cmd);
				return -EFAULT;
	}

    PROBE_IO_ENTRY(MDRV_MAJOR_GOP, _IOC_NR(cmd));

//	printk("MDrv_GOP_IOCtl IOCTL %x\n",cmd);
	switch(cmd) {

	  case MDRV_GOP_IOC_INIT:
        MDrv_GOP_IOC_Init(arg);
		break;

	  case MDRV_GOP_IOC_GWIN_FORCEWRITE:
        MDrv_GOP_IOC_GWIN_ForceWrite(arg);
		break;

	  case MDRV_GOP_IOC_SET_DSTPLANE:
        MDrv_GOP_IOC_DstPlane_Set(arg);
		break;

	  case MDRV_GOP_IOC_SET_PALETTE:
        MDrv_GOP_IOC_Palette_Set(arg);
		break;

	  case MDRV_GOP_IOC_READ_PALETTE:
        MDrv_GOP_IOC_Palette_Read(arg);
		break;

	  case MDRV_GOP_IOC_ENABLE_OUTPUT_PROGRESSIVE:
        MDrv_GOP_IOC_Output_Progressive_Enable(arg);
		break;

#if 0
	  case MDRV_GOP_IOC_ENABLE_OUTPUT_HDUP:
        MDrv_GOP_IOC_Output_HDup_Enable(arg);
		break;

	  case MDRV_GOP_IOC_ENABLE_OUTPUT_VDUP:
        MDrv_GOP_IOC_Output_VDup_Enable(arg);
		break;
#endif

	  case MDRV_GOP_IOC_ENABLE_YUV_OUT:
        MDrv_GOP_IOC_YUVOut_Enable(arg);
		break;

	  case MDRV_GOP_IOC_CREATE_GWIN:
        MDrv_GOP_IOC_GWIN_Create(arg);
		break;

	  case MDRV_GOP_IOC_GWIN_INFO:
        MDrv_GOP_IOC_GWIN_INFO(arg);
		break;

	  case MDRV_GOP_IOC_SET_BLINK_RATE:
        MDrv_GOP_IOC_Blink_SetRate(arg);
		break;

	  case MDRV_GOP_IOC_ENABLE_BLINK:
        MDrv_GOP_IOC_Blink_Enable(arg);
		break;

	  case MDRV_GOP_IOC_SET_SCROLL_RATE:
        MDrv_GOP_IOC_Scroll_SetRate(arg);
		break;

	  case MDRV_GOP_IOC_ALLOC_GWIN:
        MDrv_GOP_IOC_GWIN_Alloc(arg);
		break;

	  case MDRV_GOP_IOC_FREE_GWIN:
        MDrv_GOP_IOC_GWIN_Free(arg);
		break;

	  case MDRV_GOP_IOC_ENABLE_GWIN: //0x8008320E: // fixed me //MDRV_GOP_IOC_ENABLE_GWIN:
	  	//printk("IOCTL %lx real got %lx\n",MDRV_GOP_IOC_ENABLE_GWIN,0x8008320E);
        MDrv_GOP_IOC_GWIN_Enable(arg);
		break;

	  case MDRV_GOP_IOC_SET_TRANSCLR: //0x8008320f:
	  	//printk("IOCTL %x real got %x\n",MDRV_GOP_IOC_SET_TRANSCLR,0x8008320f);
	  	MDrv_GOP_IOC_TransClr_Set(arg);
	  break;

	  case MDRV_GOP_IOC_ENABLE_TRANSCLR:
        MDrv_GOP_IOC_TransClr_Enable(arg);
		break;

	  case MDRV_GOP_IOC_ENABLE_SCROLL:// 0x80103211://MDRV_GOP_IOC_ENABLE_SCROLL:
	  	//printk("IOCTL %lx real got %lx\n",MDRV_GOP_IOC_ENABLE_SCROLL,0x80103211);
        MDrv_GOP_IOC_GWIN_Scroll_Enable(arg);
		break;

	  case MDRV_GOP_IOC_SET_BLENDING: //0x800C3212:
	  	//printk("IOCTL %x real got %x\n",MDRV_GOP_IOC_SET_BLENDING,0x800C3212);
        MDrv_GOP_IOC_GWIN_Blending_Set(arg);
		break;

	  case MDRV_GOP_IOC_MOVE_GWIN_DISPPOS:
        MDrv_GOP_IOC_GWIN_DispPos_Move(arg);
		break;

	  case MDRV_GOP_IOC_ENABLE_SCROLL_AUTOSTOP:
        MDrv_GOP_IOC_GWIN_Scroll_AutoStop_Enable(arg);
		break;

	  case MDRV_GOP_IOC_SET_SCROLL_AUTOSTOP_H:
        MDrv_GOP_IOC_GWIN_Scroll_AutoStop_HSet(arg);
		break;

	  case MDRV_GOP_IOC_SET_SCROLL_AUTOSTOP_V:
        MDrv_GOP_IOC_GWIN_Scroll_AutoStop_VSet(arg);
		break;

	  case MDRV_GOP_IOC_SET_ENABLE_INT:
        MDrv_GOP_IOC_Int_Enable(arg);
		break;

	  case MDRV_GOP_IOC_GET_INT_STATUS:
        MDrv_GOP_IOC_Int_GetStatus(arg);
		break;

#if 0
	  case MDRV_GOP_IOC_SET_OFFSET:
        MDrv_GOP_IOC_Set_Offset(arg);
		break;
#endif

	  case MDRV_GOP_IOC_GWIN_GET_INFO:
        MDrv_GOP_IOC_GWIN_Info_Get(arg);
		break;

	  case MDRV_GOP_IOC_GWIN_SET_INFO:
	  	MDrv_GOP_IOC_GWIN_Info_Set(arg);
	  	break;

	  case MDRV_GOP_IOC_GET_INFO:
        MDrv_GOP_IOC_Info_Get(arg);
		break;

	  case MDRV_GOP_IOC_SET_BLINK_PALETTE:
        MDrv_GOP_IOC_GWIN_Blink_Palette_Set(arg);
		break;

#if 0
	  case MDRV_GOP_IOC_CREATE_TWIN:
        MDrv_GOP_IOC_TWIN_Create(arg);
		break;

	  case MDRV_GOP_IOC_Enable_TWIN:
        MDrv_GOP_IOC_TWIN_Enable(arg);
		break;
#endif

	  case MDRV_GOP_IOC_INIT_FADE:
        MDrv_GOP_IOC_GWIN_FADE_Init(arg);
		break;

	  case MDRV_GOP_IOC_TRIGGER_FADE:
        MDrv_GOP_IOC_GWIN_FADE_TRIGGER(arg);
		break;

	  case MDRV_GOP_IOC_INIT_DWIN:
        MDrv_GOP_IOC_DWIN_Init(arg);
		break;

	  case MDRV_GOP_IOC_ALLOC_DWIN:
        MDrv_GOP_IOC_DWIN_Alloc(arg);
		break;

	  case MDRV_GOP_IOC_FREE_DWIN:
        MDrv_GOP_IOC_DWIN_Free(arg);
		break;

	  case MDRV_GOP_IOC_ENABLE_CAPTURE_STREAM:
        MDrv_GOP_IOC_DWIN_CaptureStream_Enable(arg);
		break;

	  case MDRV_GOP_IOC_SET_CAPTURE_MODE:
        MDrv_GOP_IOC_DWIN_CaptureMode_Set(arg);
		break;

      //Arki>>
      case MDRV_GOP_IOC_SET_INPUTSOURCE_MODE:
        MDrv_GOP_IOC_DWIN_InputSourceMode_Set(arg);
		break;

      case MDRV_GOP_IOC_SET_CLKGEN_MODE:
        MDrv_GOP_IOC_DWIN_CLKGen_Set(arg);
        break;

      case MDRV_GOP_IOC_SET_PWR_ON:
        //printk("ioctl: GOP power on\n");
        return MDrv_GOP_PowerOn();
        break;

      case MDRV_GOP_IOC_SET_PWR_OFF:
        //printk("ioctl: GOP power off\n");

        PROBE_IO_EXIT(MDRV_MAJOR_GOP, _IOC_NR(cmd));
        return MDrv_GOP_PowerOff();
        break;
      //Arki <<

	  case MDRV_GOP_IOC_DWIN_SET_SCAN_TYPE:
        MDrv_GOP_IOC_DWIN_SetScanType(arg);
		break;

	  case MDRV_GOP_IOC_DWIN_SET_WININFO:
        MDrv_GOP_IOC_DWIN_WinInfo_Set(arg);
		break;

	  case MDRV_GOP_IOC_DWIN_ENABLE_INT:
        MDrv_GOP_IOC_DWIN_Int_Enable(arg);
		break;

	  case MDRV_GOP_IOC_DWIN_GET_INTSTATUS:
        MDrv_GOP_IOC_DWIN_GetIntrStatus(arg);
		break;

	  case MDRV_GOP_IOC_DWIN_SET_PINPON:
        MDrv_GOP_IOC_DWIN_Set_PINPON(arg);
		break;

      case MDRV_GOP_IOC_UPDATE_REG:
        MDrv_GOP_IOC_UpdateReg(arg);
        break;

      case MDRV_GOP_IOC_SET_CLOCK:
        MDrv_GOP_IOC_Clk_Set2(arg);
        break;

      case MDRV_GOP_IOC_CREATE_STRETCH_WIN:
        MDrv_GOP_IOC_StretchWin_Create(arg);
        break;

      case MDRV_GOP_IOC_STRETCH_WIN_SET_RATIO:
        MDrv_GOP_IOC_StretchWin_SetRatio(arg);
        break;

	  case MDRV_GOP_IOC_ENABLE_GWIN_ALPHA:
        MDrv_GOP_IOC_GWIN_Alpha_Enable(arg);
		break;
	  case MDRV_GOP_IOC_SCALER_SET_GOPSEL:
        MDrv_GOP_IOC_Scaler_Set_GOPSel(arg);
		break;
	  case MDRV_GOP_IOC_SET_FIELD_INVERSE:
        MDrv_GOP_IOC_SetFieldInver(arg);
		break;
	  case MDRV_GOP_IOC_SET_PALETTE_CONTROL:
        MDrv_GOP_IOC_SetPaletteControl(arg);
		break;
	  case MDRV_GOP_IOC_REGISTER_INT:
	  	MDrv_GOP_RegisterInterrupt(arg);
		break;
	  case MDRV_GOP_IOC_DEREGISTER_INT:
	  	MDrv_GOP_DeRegisterInterrupt(arg);
		break;

      case MDRV_GOP_IOC_INFOBACKUP:
	  	MDrv_GOP_InfoBackup(arg);
		break;
	  case MDRV_GOP_IOC_INFORESTORE:
	  	MDrv_GOP_InfoRestore(arg);
		break;

///////////////////////////////////////////////////////
// 20080805 LGE Added for TRUE Motion Demo.
	  case MDRV_GOP_IOC_TRUE_MOTION_DEMO :
		MDrv_GOP_TrueMotionDemo(arg);
		break;
///////////////////////////////////////////////////////

	  ///////////////////////////////////////////////////////
	  // 20080805 LGE Added for TRUE Motion Demo.
	  case MDRV_GOP_IOC_VCOM_PATTERN:
	    {
        MS_GOP_CREATE_GWIN t ;
	    if( copy_from_user( &t, (MS_GOP_CREATE_GWIN __user *)arg, sizeof(MS_GOP_CREATE_GWIN)) )
        {    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
            PROBE_IO_EXIT(MDRV_MAJOR_GOP, _IOC_NR(cmd));
		    return -EFAULT;
        }
        MHal_GOP_VCOM_Pattern( t.u8Wid, t.u32SrcX, t.u32SrcY,
            t.u32DispX, t.u32DispY,
            t.u32Width, t.u32Height ) ;
	  	}
		break;
	  ///////////////////////////////////////////////////////

     case MDRV_GOP_IOC_TEST_PATTERN:
        MDrv_GOP_IOC_Test_Pattern(arg);
        break;
		
	  case MDRV_GOP_IOC_ENABLE_VMIRROR:
        MDrv_GOP_IOC_VMirror_Enable(arg);
		break;

        case MDRV_GOP_IOC_ADJUST_HSPD: // KimTH_091026: OSD H-Position 설정 변수 추가
            MDrv_GOP_IOC_AdjustHSPD(arg);
            break;

	  case MDRV_GOP_IOC_ENABLE_HMIRROR:
        MDrv_GOP_IOC_HMirror_Enable(arg);
		break;
		
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
     case MDRV_GOP_IOC_SET_FLIP_INFO:
        MDrv_GOP_IOC_SetFlipInfo(arg);
		break;
#endif
	  default:  /* redundant, as cmd was checked against MAXNR */
		printk(" ERROR IOCtl number %x\n ",cmd);
        PROBE_IO_EXIT(MDRV_MAJOR_GOP, _IOC_NR(cmd));
		return -ENOTTY;
	}


    PROBE_IO_EXIT(MDRV_MAJOR_GOP, _IOC_NR(cmd));
	return retval;

}




static int __init MDrv_GOP_Module_Init(void)
{
	int result, i;
	dev_t dev_Number = 0;

    printk("GOP driver inits\n");

    MHal_GOP_ResetGOP();
   /*
    * Get a range of minor numbers to work with, asking for a dynamic
    * major unless directed otherwise at load time.
    */
	if (_GOP_Major) {
		dev_Number = MKDEV(_GOP_Major, _GOP_Minor);
		result = register_chrdev_region(dev_Number, _GOP_Num_Devs, "drvGOP");
	} else {
		result = alloc_chrdev_region(&dev_Number, _GOP_Minor, _GOP_Num_Devs, "drvGOP");
		_GOP_Major = MAJOR(dev_Number);
	}
	if (result < 0) {
		printk("drvGOP: can't get major %d\n", _GOP_Major);
		return result;
	}

    /*
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	_pGOP_Devices = kmalloc(_GOP_Num_Devs * sizeof(struct MS_GOP_DEV), GFP_KERNEL);
	if (!_pGOP_Devices) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(_pGOP_Devices, 0, _GOP_Num_Devs * sizeof(struct MS_GOP_DEV));

    /* Initialize each device. */
	for (i = 0; i < _GOP_Num_Devs; i++) {
		MDrv_GOP_Setup_CDev(&_pGOP_Devices[i], i);
	}

    // Maybe need to move to MDrv_GOP_Module_Init
    //MHal_GOP_Init();
    //printk("GOP driver inits ok.\n");

    return 0;

  fail:
	MDrv_GOP_CleanUp_Module();

    return result;

}

static void __exit MDrv_GOP_Module_Exit(void)
{
	dev_t dev_Number = MKDEV(_GOP_Major, _GOP_Minor);

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(dev_Number, _GOP_Num_Devs);

#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
        MDrv_GOP_DeRegisterInterrupt();
        if(fptrCheckTagId != NULL)
        {
          symbol_put(fptrCheckTagId);
        }
#endif

    printk("GOP driver exits\n");
}


#if TEST_VMA

/*
 * open and close: just keep track of how many times the device is
 * mapped, to avoid releasing it.
 */

void MDrv_GOP_vma_open(struct vm_area_struct *vma)
{

	struct MS_GOP_DEV *dev = vma->vm_private_data;
	dev->vmas++;
}

void MDrv_GOP_vma_close(struct vm_area_struct *vma)
{
	struct MS_GOP_DEV *dev = vma->vm_private_data;
	dev->vmas--;
}


struct vm_operations_struct MDrv_GOP_vm_ops = {
	.open =     MDrv_GOP_vma_open,
	.close =    MDrv_GOP_vma_close,
};


int MDrv_GOP_MMap(struct file *filp, struct vm_area_struct *vma)
{
    int ret;
    vma->vm_pgoff = 0x1000000 >> PAGE_SHIFT;

    ret = remap_pfn_range(vma, vma->vm_start,
                          vma->vm_pgoff,
                          vma->vm_end - vma->vm_start,
                          vma->vm_page_prot) ? -EAGAIN : 0;

	if (-EAGAIN != ret)
	{
	vma->vm_ops = &MDrv_GOP_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = filp->private_data;

	MDrv_GOP_vma_open(vma);
	}

	return ret;
}

#endif

////////////////////////////////////////////////////// start
// 20080805 LGE Added for Interrupt Handling (TRUE Motion Demo)
static struct {
	unsigned char *pFrame;			/**< frame ptr */
	int frameStride;				/**< frame stride */
	int bytePerPixel;				/**< bytes per Pixel */
	unsigned char *pImage;			/**< image(max 1366*60) */
	int imgWidth;					/**< image width */
	int imgHeight;					/**< image height */
	int count;						/**< count */
	int step;						/**< step */
	int y;							/**< y */
} scrollImageInfo = {
	.pFrame			= NULL,
	.frameStride	= 0,
	.bytePerPixel	= 4,
	.pImage			= NULL,
	.imgWidth		= 0,
	.imgHeight		= 0,
	.count			= 0,
	.step			= 0,
	.y				= 0,
};
static unsigned char scrollImage[1366*80*4];
static int fScrollEnable = 0;
static int u8TMGwinId = 0;

static int MDrv_GOP_TrueMotionDemo(unsigned long arg)
{    
    if (copy_from_user((void *)&scrollImageInfo, (void *)arg, sizeof(scrollImageInfo)))
        return EFAULT;

    if (scrollImageInfo.step!=0 && fScrollEnable)
        return 0;
    
    printk("%s : scrollImageInfo\n", __FUNCTION__);
    //scrollImageInfo.pFrame = MDrv_SYS_PA2NonCacheSeg( scrollImageInfo.pFrame );
    printk(".pFrame = %p\n", scrollImageInfo.pFrame);
    printk(".frameStride = %d\n", scrollImageInfo.frameStride);
    printk(".bytePerPixel = %d\n", scrollImageInfo.bytePerPixel);
    printk(".pImage = %p\n", scrollImageInfo.pImage);
    printk(".imgWidth = %d\n", scrollImageInfo.imgWidth);
    printk(".imgHeight = %d\n", scrollImageInfo.imgHeight);
    printk(".count = %d\n", scrollImageInfo.count);
    printk(".step = %d\n", scrollImageInfo.step);
    printk(".y = %d\n", scrollImageInfo.y);

    if (copy_from_user(scrollImage, scrollImageInfo.pImage, scrollImageInfo.imgWidth*scrollImageInfo.imgHeight*scrollImageInfo.bytePerPixel))
        return EFAULT;

    if (scrollImageInfo.step==0)
    {
        MHal_GOP_TrueMotionDemo(FALSE);
        MHal_GOP_GWIN_Free(E_GOP4G_0, u8TMGwinId);
        fScrollEnable = FALSE;
    }
    else
    {
        scrollImageInfo.step = 2; 
        MHal_GOP_GWIN_Alloc(E_GOP4G_0, u8TMGwinId);
        if (bIsVMirror)
        {
            MHal_GOP_GWIN_Create(E_GOP4G_0, u8TMGwinId, E_GOP_COLOR_ARGB8888, 0, 0, 0, 0, scrollImageInfo.frameStride/scrollImageInfo.bytePerPixel, scrollImageInfo.imgHeight, scrollImageInfo.pFrame+scrollImageInfo.y*scrollImageInfo.frameStride, scrollImageInfo.frameStride/scrollImageInfo.bytePerPixel, scrollImageInfo.imgHeight);
        }
        else 
        {
            MHal_GOP_GWIN_Create(E_GOP4G_0, u8TMGwinId, E_GOP_COLOR_ARGB8888, 0, 0, 0, scrollImageInfo.y, scrollImageInfo.frameStride/scrollImageInfo.bytePerPixel, scrollImageInfo.imgHeight, scrollImageInfo.pFrame+scrollImageInfo.y*scrollImageInfo.frameStride, scrollImageInfo.frameStride/scrollImageInfo.bytePerPixel, scrollImageInfo.imgHeight);
        }
        if (bIsHMirror)
        {
            MHal_GOP_GWIN_Scroll_Enable(E_GOP4G_0, u8TMGwinId, GOP_SCROLL_RIGHT, scrollImageInfo.step, TRUE);
        }
        else
        {
            MHal_GOP_GWIN_Scroll_Enable(E_GOP4G_0, u8TMGwinId, GOP_SCROLL_LEFT, scrollImageInfo.step, TRUE);
        }
        
        MHal_GOP_Scroll_SetRate(E_GOP4G_0, 0);
        MHal_GOP_TrueMotionDemo(TRUE);
        MHal_GOP_GWIN_Enable(E_GOP4G_0, u8TMGwinId, TRUE);
        fScrollEnable = TRUE;
    }
    return 0;
}


void MDrv_GOP_VSyncHandle(void)
{
#if 0
	unsigned char *pImgFore, *pImgBack;
	unsigned char *pf;
	int wf, wb, wi, h;

	if (!fScrollEnable)
		return;

	pImgFore = scrollImage + scrollImageInfo.count * scrollImageInfo.bytePerPixel;
	wf = (scrollImageInfo.imgWidth - scrollImageInfo.count) * scrollImageInfo.bytePerPixel;
	pImgBack = scrollImage;
	wb = scrollImageInfo.count * scrollImageInfo.bytePerPixel;
	wi = scrollImageInfo.imgWidth * scrollImageInfo.bytePerPixel;

	for (h=scrollImageInfo.imgHeight, pf=(scrollImageInfo.pFrame+scrollImageInfo.y*scrollImageInfo.frameStride); h>0;
	  h--, pf+=scrollImageInfo.frameStride) {
		memcpy(pf, pImgFore, wf);
		if (wb)
			memcpy(pf+wf, pImgBack, wb);
		pImgFore += wi, pImgBack += wi;
	}
	scrollImageInfo.count = (scrollImageInfo.count+scrollImageInfo.step)%scrollImageInfo.imgWidth;
#endif
}

EXPORT_SYMBOL(MDrv_GOP_VSyncHandle);
////////////////////////////////////////////////////// end

module_init(MDrv_GOP_Module_Init);
module_exit(MDrv_GOP_Module_Exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("GOP driver");
MODULE_LICENSE("MSTAR");
