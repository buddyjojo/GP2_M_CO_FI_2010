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
/// file    mdrv_msmailbox_dev.c
/// @brief  MS MailBox driver
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


/* linux header files */
// 2.6.26
//#include <linux/config.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/wait.h>

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/kdev_t.h>

#include <asm-mips/io.h>
#include <asm-mips/string.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>

#include <asm-mips/system.h>

/* drver header files */
#include "mdrv_msmailbox_dev.h"
#include "mdrv_msmailbox_interrupt.h"
#include "mdrv_msmailbox_mbmgr.h"

#include "mhal_msmailbox.h"


#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                            BUG();\
                        }\
                    } while (0)

#define     MSMAILBOX_ASSERT(arg)               assert((arg))

//#define     _DEBUG  // kjoh LGE 20080813
#ifdef      _DEBUG
#define     MSMAILBOX_KDBG(fmt, args...)        printk(KERN_WARNING fmt, ## args)
#else
#define     MSMAILBOX_KDBG(fmt, args...)
#endif


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
#define     MDRV_MSMAILBOX_DEVICE_COUNT         1
#define     MDRV_MSMAILBOX_NAME                 "MSMAILBOX"


//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _MDrv_MSMAILBOX_Open (struct inode *inode, struct file *filp);
static int                      _MDrv_MSMAILBOX_Release(struct inode *inode, struct file *filp);
static int                      _MDrv_MSMAILBOX_MM_MMAP(struct file *filp, struct vm_area_struct *vma);
static int                      _MDrv_MSMAILBOX_IOCtl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

static void                     _MDrv_MSMAILBOX_MM_Open(struct vm_area_struct *vma);
static void                     _MDrv_MSMAILBOX_MM_Close(struct vm_area_struct *vma);
//int                             _MDrv_MSMAILBOX_MM_NoPage(struct vm_area_struct *vma, unsigned long address, int *type);
int                             _MDrv_MSMAILBOX_MM_NoPage(struct vm_area_struct *vma, struct vm_fault *vmf);

//-------------------------------------------------------------------------------------------------
// Device handler
//-------------------------------------------------------------------------------------------------
static devicehandle_t           MSMailBoxDev=
{
    .s32Major=                  MDRV_MAJOR_MSMAILBOX,
    .s32Minor=                  MDRV_MINOR_MSMAILBOX,
    .u32MM=                     0,
    .bOpen=                     FALSE,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_MSMAILBOX, },
        .owner  =               THIS_MODULE,
    },
    .Fop=
    {
        .open=                  _MDrv_MSMAILBOX_Open,
        .release=               _MDrv_MSMAILBOX_Release,
        .mmap=                  _MDrv_MSMAILBOX_MM_MMAP,
        .ioctl=                 _MDrv_MSMAILBOX_IOCtl,
    },
    .MMops =
    {
        .open=                  _MDrv_MSMAILBOX_MM_Open,
        .close=                 _MDrv_MSMAILBOX_MM_Close,
        .fault=                _MDrv_MSMAILBOX_MM_NoPage
    }
};





//-------------------------------------------------------------------------------------------------
// Ioctrl handler
//-------------------------------------------------------------------------------------------------
void        opIORegister(U32 u32CmdIdx, ioctl_cmd_t  ioctl_func);
void        opIOInit(void);
ioctrl_mgr_t                    IOCTL=
{
    .ops.Register=              opIORegister,
    .ops.Init=                  opIOInit,
};

void opIORegister(U32 u32CmdIdx, ioctl_cmd_t  ioctl_func)
{
    if(u32CmdIdx < IOCTL_MSMAILBOX_END_NUM )
    {
        IOCTL.cmd[u32CmdIdx] = ioctl_func;
    }
}

void opIOInit(void)
{
    U32     u32CmdIdx;
    for(u32CmdIdx=0; u32CmdIdx<IOCTL_MSMAILBOX_END_NUM; u32CmdIdx++)
        IOCTL.cmd[u32CmdIdx] = (ioctl_cmd_t)NULL;
}

//-------------------------------------------------------------------------------------------------
// Wait queue manager
//-------------------------------------------------------------------------------------------------
U32         opWQNum(void);
void        opWQSet(void);
void        opWQClr(void);
_ERR_CODE_  opWQWait(void);
_ERR_CODE_  opWQWakeup(void);
_ERR_CODE_  opWQInit(cb_check_t func);
B16         opWQdefCheck(void);

ms_wq_mgr_t                     msmailbox_wq_mgr=
{
    .bValid=                    FALSE,
    .ops.Init=                  opWQInit
};

_ERR_CODE_  opWQInit(cb_check_t func)
{
    if(msmailbox_wq_mgr.bValid == FALSE)
    {
        atomic_set(&msmailbox_wq_mgr.aNum, 0);
        init_waitqueue_head(&msmailbox_wq_mgr.tWQ);

        msmailbox_wq_mgr.ops.Clr=             opWQClr;
        msmailbox_wq_mgr.ops.Set=             opWQSet;
        msmailbox_wq_mgr.ops.Wait=            opWQWait;
        msmailbox_wq_mgr.ops.Wakeup=          opWQWakeup;
        msmailbox_wq_mgr.ops.Num=             opWQNum;
        if(func == (cb_check_t)NULL)
            msmailbox_wq_mgr.cb_ops.Check=    opWQdefCheck;
        else
            msmailbox_wq_mgr.cb_ops.Check=    func;

        msmailbox_wq_mgr.bValid =             TRUE;
        (*msmailbox_wq_mgr.ops.Clr)();
        return E_OK;
    }
    else
        return E_WAITQUEUE_INVALID;
}


B16         opWQdefCheck(void)
{
    return FALSE;
}

U32         opWQNum(void)
{
    if(msmailbox_wq_mgr.bValid == TRUE)
    {
        return atomic_read(&msmailbox_wq_mgr.aNum);
    }
    else
        return 0;
}

void        opWQSet(void)
{
    if(msmailbox_wq_mgr.bValid == TRUE)
    {
        atomic_set(&msmailbox_wq_mgr.aNum, 1);
    }
}

void        opWQClr(void)
{
    if(msmailbox_wq_mgr.bValid == TRUE)
    {
        atomic_set(&msmailbox_wq_mgr.aNum, 0);
    }
}

_ERR_CODE_  opWQWait(void)
{
    /* wait queue for Mail box */
    DEFINE_WAIT(waitentry_mailbox);
    /* wait queue only accepts one thread to wait MLK Mail */
    if((*msmailbox_wq_mgr.ops.Num)() == 0x00)
    {
        prepare_to_wait(&msmailbox_wq_mgr.tWQ, &waitentry_mailbox, TASK_INTERRUPTIBLE);
        (*msmailbox_wq_mgr.ops.Set)();
        /* we have to check if mail is coming. */
        if( (*msmailbox_wq_mgr.cb_ops.Check)() == FALSE )
			schedule_timeout(500);
            //schedule();
        finish_wait(&msmailbox_wq_mgr.tWQ, &waitentry_mailbox);
        (*msmailbox_wq_mgr.ops.Clr)();
    }
    else
    {
        return E_WAITQUEUE_FULL;
    }
    return E_OK;
}


_ERR_CODE_  opWQWakeup(void)
{
    if((*msmailbox_wq_mgr.ops.Num)() == 0x01)
    {
        wake_up(&msmailbox_wq_mgr.tWQ);
        (*msmailbox_wq_mgr.ops.Clr)();
        return E_OK;
    }
    else
        return E_WAITQUEUE_EMPTY;
}

//-------------------------------------------------------------------------------------------------
// Mail box manager
//-------------------------------------------------------------------------------------------------
extern msmailbox_queue_mgr_t            msmailbox_queue_mgr;

//-------------------------------------------------------------------------------------------------
// Interrupt manager
//-------------------------------------------------------------------------------------------------
extern int_mgr_t                        msmailbox_int_mgr;

//-------------------------------------------------------------------------------------------------
// HAL manager
//-------------------------------------------------------------------------------------------------
extern hal_msmailbox_t                  hal_msmailbox;
extern hal_int_t                        hal_msmailbox_int;


//-------------------------------------------------------------------------------------------------
// PM Mailbox
//-------------------------------------------------------------------------------------------------
msmailbox_t     PM_Mail[20];
U8              PMIndxP = 0;
U8              PMIndxG = 0;

//EXPORT_SYMBOL(_receive_mail_fromPM);
//EXPORT_SYMBOL(_send_mail_toPM);

void _reset_mailbox_pm(void)
{
    PMIndxG = 0;
    PMIndxP = 0;
    hal_msmailbox.ops.Overflow(CPUID_PM, FALSE);
}

BOOL _receive_mail_fromPM(msmailbox_t *pMail)
{
    if(PMIndxG == PMIndxP)
        return FALSE;

    memcpy(pMail, &PM_Mail[PMIndxG], sizeof(msmailbox_t));
    PMIndxG = PMIndxG + 1;
    if(PMIndxG == 20)
        PMIndxG = 0;
    hal_msmailbox.ops.Overflow(CPUID_PM, FALSE);
    return TRUE;
}

BOOL _send_mail_toPM(msmailbox_t *pMail)
{
    _ERR_CODE_      err = E_OK;
    err = hal_msmailbox.ops.Fire(pMail, CPUID_H3);
    if(err == E_OK)
    {
        hal_msmailbox_int.ops.IPROC(CPUID_H3, CPUID_PM);
    }
    else
    {
        MSMAILBOX_KDBG("Fire error %d\n", err);
    }

    if(err!= E_OK)
        return FALSE;
    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// IOCTOL command
//-------------------------------------------------------------------------------------------------
_ERR_CODE_  ioctl_INIT(U32      u32Arg)
{
    MSMAILBOX_KDBG("IOCTL INIT\n");
    return E_OK;
}

_ERR_CODE_  ioctl_FIREMAIL(U32      u32Arg)
{
    _ERR_CODE_      err;
    msmailbox_t     tMail;
    //U32             i;

    //MSMAILBOX_KDBG("IOCTL FIREMAIL\n");

    //MSMAILBOX_ASSERT(copy_from_user((void*)&tMail, (void*)u32Arg, sizeof(msmailbox_t)) == 0);
    if( copy_from_user((void*)&tMail, (void*)u32Arg, sizeof(msmailbox_t)) )
        return E_COPYFROMUSER_FAIL;
    /*
    for(i=0; i<sizeof(msmailbox_t); i++)
    {
        MSMAILBOX_KDBG("Mail[%d] = %d\n", i, ((U8*)&tMail)[i]);
    }
    */

    err = (*hal_msmailbox.ops.Fire)((msmailbox_t*)&tMail, CPUID_H3);
    if(err == E_OK)
    {
        (*hal_msmailbox_int.ops.IAEON)();
    }
    else
    {
        MSMAILBOX_KDBG("Fire error %d\n", err);
    }
    return err;
}

_ERR_CODE_  ioctl_FIREMAIL_LOOPBACK(U32      u32Arg)
{
    _ERR_CODE_      err;
    msmailbox_t     tMail;
    //U32             i;

    MSMAILBOX_KDBG("IOCTL FIREMAIL LOOPBACK\n");

    //MSMAILBOX_ASSERT(copy_from_user((void*)&tMail, (void*)u32Arg, sizeof(msmailbox_t)) == 0);
    if( copy_from_user((void*)&tMail, (void*)u32Arg, sizeof(msmailbox_t)))
        return E_COPYFROMUSER_FAIL;
    /*
    for(i=0; i<sizeof(msmailbox_t); i++)
    {
        MSMAILBOX_KDBG("Mail[%d] = %d\n", i, ((U8*)&tMail)[i]);
    }
    */

    err = hal_msmailbox.ops.FireLoopback((msmailbox_t*)&tMail, CPUID_PM);
    //err = (*hal_msmailbox.ops.FireLoopback)((msmailbox_t*)&tMail, CPUID_PM);
    //err = hal_msmailbox.ops.FireLoopback((msmailbox_t*)&tMail, CPUID_PM);
    if(err == E_OK)
    {
        //(*hal_msmailbox_int.ops.IMIPS)();
        hal_msmailbox_int.ops.IPROC(CPUID_PM, CPUID_H3);
    }
    else
    {
        MSMAILBOX_KDBG("Fire error %d\n", err);
    }

    return err;
}

_ERR_CODE_  ioctl_WAIT_SYNCMAIL(U32      u32Arg)
{
    _ERR_CODE_      err;

    MSMAILBOX_KDBG("IOCTL SYNCMAIL\n");
    err = (*msmailbox_wq_mgr.ops.Wait)();
    return err;
}

_ERR_CODE_  ioctl_INTERRUPT(U32     u32Arg)
{
    MSMAILBOX_KDBG("IOCTL INTERRUPT\n");
    if(u32Arg)
    {
        (*msmailbox_int_mgr.ops.Enable)(CPUID_AEON);
        (*msmailbox_int_mgr.ops.Enable)(CPUID_PM);
    }
    else
    {
        (*msmailbox_int_mgr.ops.Disable)(CPUID_AEON);
        (*msmailbox_int_mgr.ops.Enable)(CPUID_PM);
    }
    return E_OK;
}

_ERR_CODE_  ioctl_INFO(U32          u32Arg)
{
    U32 u32InfoOfst;

    MSMAILBOX_KDBG("IOCTL INFO\n");
    u32InfoOfst = ((U32)msmailbox_queue_mgr.pInfo - (U32)msmailbox_queue_mgr.pBuf);
    if(copy_to_user((U32*)u32Arg, (const U32*)&u32InfoOfst, sizeof(U32)))
        return E_COPYTOUSER_FAIL;

    return E_OK;
}

_ERR_CODE_  ioctl_ENABLE(U32        u32Arg)
{
    MSMAILBOX_KDBG("IOCTL ENABLE\n");
    (*msmailbox_int_mgr.ops.Enable)(CPUID_AEON);
    (*msmailbox_int_mgr.ops.Enable)(CPUID_PM);
    return E_OK;
}

_ERR_CODE_  ioctl_DISABLE(U32       u32Arg)
{
    MSMAILBOX_KDBG("IOCTL DISABLE\n");
    (*msmailbox_int_mgr.ops.Disable)(CPUID_AEON);
    (*msmailbox_int_mgr.ops.Enable)(CPUID_PM);
    return E_OK;
}



//-------------------------------------------------------------------------------------------------
// Call back functions
//-------------------------------------------------------------------------------------------------
B16 callback_wq(void)
{
    if(msmailbox_queue_mgr.pInfo->u32GetAdr != msmailbox_queue_mgr.pInfo->u32PutAdr)
    return TRUE;
    else
        return FALSE;
}


void callback_receive_mail_int(void* pVoid)
{
    msmailbox_t *ptMail;
    //U8          tPayoad;

    switch((U32)pVoid)
    {
    case CPUID_AEON:
        if(msmailbox_queue_mgr.ops.PutAddr(&ptMail) == E_OK)
        {
            hal_msmailbox.ops.Receive(ptMail, CPUID_AEON);
            msmailbox_wq_mgr.ops.Wakeup();
        }
        break;
    case CPUID_PM:

        if((PMIndxP+1)%20 == PMIndxG)
        {
            MSMAILBOX_KDBG("FULL PM Mailbox\n");
            hal_msmailbox.ops.Overflow(CPUID_PM, TRUE);
            return;
        }
        ptMail = (msmailbox_t *)&PM_Mail[PMIndxP];

        hal_msmailbox.ops.Receive(ptMail, CPUID_PM);

        PMIndxP = PMIndxP + 1;
        if(PMIndxP==20)
            PMIndxP = 0;
        /*
        MSMAILBOX_KDBG("Class 0x%2x\n", ptMail->u8Class);
        MSMAILBOX_KDBG("Index 0x%2x\n", ptMail->u8Index);
        MSMAILBOX_KDBG("Count 0x%2x\n", ptMail->u8ParameterCount);
        for(tPayoad=0; tPayoad<ptMail->u8ParameterCount; tPayoad++)
        {
            MSMAILBOX_KDBG("[%2d]=0x%2x \n", tPayoad, ptMail->u8Parameters[tPayoad]);
        }
        */
        printk("G=%d P=%d\n", PMIndxG, PMIndxP );

        break;
    }
}

//-------------------------------------------------------------------------------------------------
// Driver functions
//-------------------------------------------------------------------------------------------------
int _MDrv_MSMAILBOX_Open(struct inode *inode, struct file *filp)
{
    MSMAILBOX_KDBG("--------->MSMAILBOX DRIVER OPEN\n");

    if(MSMailBoxDev.bOpen == FALSE)
    {
        MSMailBoxDev.bOpen                      = TRUE;

        /* initial mail queue manager */
        if( (*msmailbox_queue_mgr.ops.Init)(MSMAILBOX_PAGES) != E_OK )
            return -EFAULT;

        /* initial hal mail box */
        (*hal_msmailbox.ops.Init)();

        /* initial IRQ manager */
        (*msmailbox_int_mgr.ops.Init)((void*)&MSMailBoxDev.cDevice, callback_receive_mail_int);
        (*msmailbox_int_mgr.ops.Disable)(CPUID_AEON);

        /* initial Wait queue manager */
        (*msmailbox_wq_mgr.ops.Init)((cb_check_t)callback_wq);


        /* register ioctl handler */
        (*IOCTL.ops.Init)();
        (*IOCTL.ops.Register)(NO_INIT,                      NULL);
        (*IOCTL.ops.Register)(NO_RELEASE,                   NULL);
        (*IOCTL.ops.Register)(NO_FIREMAIL,                  ioctl_FIREMAIL);
        (*IOCTL.ops.Register)(NO_WAIT_SYNCMAIL,             ioctl_WAIT_SYNCMAIL);
        (*IOCTL.ops.Register)(NO_INTERRUPT,                 NULL);
        (*IOCTL.ops.Register)(NO_INFO,                      ioctl_INFO);
        (*IOCTL.ops.Register)(NO_ENABLE,                    ioctl_ENABLE);
        (*IOCTL.ops.Register)(NO_DISABLE,                   ioctl_DISABLE);
        (*IOCTL.ops.Register)(NO_FIREMAIL_LOOPBACK,         ioctl_FIREMAIL_LOOPBACK);
        return 0;
    }
    else
    {
        return  -EFAULT;
    }
}

int _MDrv_MSMAILBOX_Release(struct inode *inode, struct file *filp)
{
    MSMAILBOX_KDBG("<---------MAILBOX DRIVER CLOSE\n");

    if(MSMailBoxDev.bOpen == TRUE)
    {
        /* if MSMailBoxDev.u32MM == 0 means this mmap doesn't be referenced by other appilation and system can free it. */
        if(MSMailBoxDev.u32MM == 0)
        {
            /* stop interrupt */
            (*msmailbox_int_mgr.ops.Disable)(CPUID_AEON);
            (*msmailbox_int_mgr.ops.Disable)(CPUID_PM);
            /* release interrupt */
            (*msmailbox_int_mgr.ops.Release)((void*)&MSMailBoxDev.cDevice);
            /* release mail box buffer */
            (*msmailbox_queue_mgr.ops.Release)();

            MSMailBoxDev.bOpen = FALSE;
            return 0;
        }
        else
        {
            /* MMap dosn't release */
            MSMAILBOX_ASSERT(FALSE);
        }
    }
    return -EFAULT;
}

int _MDrv_MSMAILBOX_IOCtl(struct inode *inode, struct file *filp, U32 u32Cmd, unsigned long u32Arg)
{
    int         err= 0;
    U32         u32CmdNum;

    if(MSMailBoxDev.bOpen != TRUE)
    {
        return -EFAULT;
    }
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (MSMAILBOX_IOCTL_MAGIC!= _IOC_TYPE(u32Cmd))
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

    u32CmdNum = _IOC_NR(u32Cmd);

    if(u32CmdNum >= IOCTL_MSMAILBOX_END_NUM)
        return -ENOTTY;

    if(IOCTL.cmd[u32CmdNum] != (ioctl_cmd_t)(NULL))
    {
        PROBE_IO_ENTRY(MDRV_MAJOR_MSMAILBOX, _IOC_NR(u32CmdNum));

        err =(*IOCTL.cmd[u32CmdNum])(u32Arg);

        PROBE_IO_EXIT(MDRV_MAJOR_MSMAILBOX, _IOC_NR(u32CmdNum));
        return err;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_MSMAILBOX, _IOC_NR(u32CmdNum));
    return -ENOTTY;
}

static void _MDrv_MSMAILBOX_MM_Open(struct vm_area_struct *vma)
{
    if(msmailbox_queue_mgr.bValid == TRUE)
    {
        MSMailBoxDev.u32MM++;
    }
}

static void _MDrv_MSMAILBOX_MM_Close(struct vm_area_struct *vma)
{
    if(msmailbox_queue_mgr.bValid == TRUE && MSMailBoxDev.u32MM > 0)
    {
        MSMailBoxDev.u32MM--;
    }
}


static int _MDrv_MSMAILBOX_MM_MMAP(struct file *filp, struct vm_area_struct *vma)
{
    MSMAILBOX_KDBG("MS MailBox VMSIZE = 0x%x\n", (U32)(vma->vm_end - vma->vm_start));

    /* no initial */
    if(msmailbox_queue_mgr.bValid == FALSE)
        return -EFAULT;

    /* only one memory mapping support */
    if(MSMailBoxDev.u32MM > 1)
        return -EFAULT;

    MSMAILBOX_KDBG("Mail Box MM Initial ...\n");

    /* over size */
    if(vma->vm_end - vma->vm_start > MSMAILBOX_PAGES*(0x01<<PAGE_SHIFT))
    {
        //MSMAILBOX_ASSERT( ((vma->vm_end - vma->vm_start) > MSMAILBOX_PAGES*(0x01<<PAGE_SHIFT)) );
        MSMAILBOX_ASSERT( FALSE );
        return -EFAULT;
    }

    vma->vm_flags |= VM_RESERVED;
    vma->vm_ops = &MSMailBoxDev.MMops;

    remap_pfn_range(vma, vma->vm_start, (virt_to_phys(msmailbox_queue_mgr.pBuf)>>PAGE_SHIFT),
        vma->vm_end - vma->vm_start, vma->vm_page_prot);

    MSMAILBOX_KDBG("VMSTART = 0x%x, VMEND = 0x%x, PFN = 0x%x\n",
        (U32)(vma->vm_start), (U32)(vma->vm_end), (U32)(virt_to_phys(msmailbox_queue_mgr.pBuf)>>PAGE_SHIFT));

    _MDrv_MSMAILBOX_MM_Open(vma);

    MSMAILBOX_KDBG("MMAP Done.\n");
    return 0;
}

#if 1
int _MDrv_MSMAILBOX_MM_NoPage(struct vm_area_struct *vma, struct vm_fault *vmf)
{
    MSMAILBOX_KDBG("PAGE REQUEST in MLINK\n");
    return 0;
}
#endif
//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static int _MOD_MSMAILBOX_Init(void)
{
    int         s32Ret;
    dev_t       dev;

    if (MSMailBoxDev.s32Major)
    {
        dev = MKDEV(MSMailBoxDev.s32Major, MSMailBoxDev.s32Minor);
        s32Ret = register_chrdev_region(dev, MDRV_MSMAILBOX_DEVICE_COUNT, MDRV_NAME_MSMAILBOX);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, MSMailBoxDev.s32Minor, MDRV_MSMAILBOX_DEVICE_COUNT, MDRV_NAME_MSMAILBOX);
        MSMailBoxDev.s32Major = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        MSMAILBOX_KDBG("Unable to get major %d\n", MSMailBoxDev.s32Major);
        return s32Ret;
    }

    cdev_init(&MSMailBoxDev.cDevice, &MSMailBoxDev.Fop);
    if (0 != (s32Ret= cdev_add(&MSMailBoxDev.cDevice, dev, MDRV_MSMAILBOX_DEVICE_COUNT)))
    {
        MSMAILBOX_KDBG("Unable add a character device\n");
        unregister_chrdev_region(dev, MDRV_MSMAILBOX_DEVICE_COUNT);
        return s32Ret;
    }

    return 0;

}


static void _MOD_MSMAILBOX_Cleanup(void)
{
    cdev_del(&MSMailBoxDev.cDevice);
    unregister_chrdev_region(MKDEV(MSMailBoxDev.s32Major, MSMailBoxDev.s32Minor), MDRV_MSMAILBOX_DEVICE_COUNT);
}


module_init(_MOD_MSMAILBOX_Init);
module_exit(_MOD_MSMAILBOX_Cleanup);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MSMAILBOX driver");
MODULE_LICENSE("MSTAR");
