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

#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include "Board.h"

#include "mdrv_msmailbox_mbmgr.h"
//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define     _DEBUG
#ifdef      _DEBUG
#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                            BUG();\
                        }\
                    } while (0)

#define     MSMAILBOX_ASSERT(arg)               assert((arg))
#define     MSMAILBOX_KDBG(fmt, args...)        printk(KERN_WARNING fmt, ## args)
#else
#define     MSMAILBOX_ASSERT(arg)
#define     MSMAILBOX_KDBG(fmt, args...)
#endif

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
_ERR_CODE_ opMBInit         (U32            u32PageNum);
_ERR_CODE_ opMBFill         (msmailbox_t*   tMail);
_ERR_CODE_ opMBInstant      (msmailbox_t*   tMail);
_ERR_CODE_ opMBRelease      (void);
_ERR_CODE_ opMBPutAddr      (msmailbox_t**  ptMail);


msmailbox_queue_mgr_t             msmailbox_queue_mgr=
{
    .bValid=                    FALSE,
    .pInfo=                     NULL,
    .pBuf=                      NULL,
    .ops.Init=                  opMBInit,
    .ops.Fill=                  opMBFill,
    .ops.Instant=               opMBInstant,
    .ops.Release=               opMBRelease,
    .ops.PutAddr=               opMBPutAddr,
};

_ERR_CODE_ opMBInit(U32 u32PageNum)
{
    //struct page             *pPage;
    //U32                     u32PageIdx;
    U32                     u32MaxMailUnit;

    MSMAILBOX_ASSERT(u32PageNum>0);

    /* if mail box buffer is valid, driver need to be initialed */
    if(msmailbox_queue_mgr.bValid == FALSE)
    {
        msmailbox_queue_mgr.u32PageNum    = u32PageNum;
        //msmailbox_queue_mgr.pBuf          = (U8*)__get_free_pages(GFP_KERNEL, get_order(u32PageNum*(0x01<<PAGE_SHIFT)) );
        msmailbox_queue_mgr.pBuf = (U8*)phys_to_virt((U32)MAILBOX_ADR);

        MSMAILBOX_ASSERT(msmailbox_queue_mgr.pBuf!=NULL);

        if(msmailbox_queue_mgr.pBuf == NULL)
        {
            MSMAILBOX_KDBG("cannot alloc %d pages\n", u32PageNum);
            return E_MEMALLOC_FAIL;
        }
        msmailbox_queue_mgr.bValid        = TRUE;

        /* set pages to reserved */
        #if 0
        for(u32PageIdx=0; u32PageIdx<u32PageNum; u32PageIdx++)
        {
            pPage = virt_to_page(msmailbox_queue_mgr.pBuf+u32PageIdx*(0x01<<PAGE_SHIFT));
            SetPageReserved(pPage);
        }
        MSMAILBOX_KDBG("page order = %d\n", get_order(u32PageNum*(0x01<<PAGE_SHIFT)));
        #endif

        u32MaxMailUnit                          = (u32PageNum*(0x01<<PAGE_SHIFT)/MSMAILBOX_SIZE - 4);
        msmailbox_queue_mgr.pInfo                        = (msmailbox_info_t *)((U32)msmailbox_queue_mgr.pBuf + MSMAILBOX_SIZE*(u32MaxMailUnit));
        msmailbox_queue_mgr.pInfo->u32PutAdr             = 0;
        msmailbox_queue_mgr.pInfo->u32GetAdr             = 0;
        msmailbox_queue_mgr.pInfo->u32MaxMUnit           = u32MaxMailUnit-1;
        msmailbox_queue_mgr.pInfo->u32Overflow           = FALSE;

        MSMAILBOX_KDBG("mail box buffer addr 0x%x\n",(U32)msmailbox_queue_mgr.pBuf);
        MSMAILBOX_KDBG("mail box buffer info addr 0x%x\n",(U32)msmailbox_queue_mgr.pInfo);
        MSMAILBOX_KDBG("mail box buffer phy addr addr 0x%x\n",MAILBOX_ADR);
        return E_OK;
    }
    return E_MAILBOX_INVALID;
}

_ERR_CODE_ opMBRelease(void)
{
    //struct page         *pPage;
    //U32                 u32PageIdx;

    if(msmailbox_queue_mgr.bValid)
    {
        /*
        for(u32PageIdx=0; u32PageIdx<msmailbox_queue_mgr.u32PageNum; u32PageIdx++)
        {
            pPage                       = virt_to_page(msmailbox_queue_mgr.pBuf+u32PageIdx*(0x01<<PAGE_SHIFT) + 0x01);
            ClearPageReserved(pPage);
        }
        free_pages((U32)msmailbox_queue_mgr.pBuf, get_order(msmailbox_queue_mgr.u32PageNum*(0x01<<PAGE_SHIFT)) );
        */
        msmailbox_queue_mgr.pBuf                = NULL;
        msmailbox_queue_mgr.pInfo               = NULL;
        msmailbox_queue_mgr.bValid              = FALSE;
        return E_OK;
    }
    return E_MAILBOX_INVALID;
}

_ERR_CODE_ opMBInstant(msmailbox_t*  tMail)
{
    U32                 u32PutAdr;
    U32                 u32Ind;
    volatile U8         *pBuf;
    volatile U8         *pMailBuf;
    if(msmailbox_queue_mgr.bValid)
    {
        u32PutAdr                       = msmailbox_queue_mgr.pInfo->u32MaxMUnit+1;
        pBuf                            = msmailbox_queue_mgr.pBuf;
        pBuf                            = (volatile U8*)((U32)pBuf + u32PutAdr*MSMAILBOX_SIZE);
        pMailBuf                        = (volatile U8*)tMail;
        for(u32Ind=0; u32Ind<MSMAILBOX_SIZE; u32Ind++)
        {
            *(pBuf+u32Ind) = *(pMailBuf+u32Ind);
        }
        return E_OK;
    }
    return E_MAILBOX_INVALID;
}

_ERR_CODE_ opMBFill(msmailbox_t*  tMail)
{
    U32                 u32PutAdr;
    U32                 u32Ind;
    volatile U8         *pBuf;
    volatile U8         *pMailBuf;
    if(msmailbox_queue_mgr.bValid)
    {
        u32PutAdr                       = msmailbox_queue_mgr.pInfo->u32PutAdr;
        pBuf                            = msmailbox_queue_mgr.pBuf;
        pBuf                            = (volatile U8*)((U32)pBuf + u32PutAdr*MSMAILBOX_SIZE);
        pMailBuf                        = (volatile U8*)tMail;
        for(u32Ind=0; u32Ind<MSMAILBOX_SIZE; u32Ind++)
        {
            *(pBuf+u32Ind) = *(pMailBuf+u32Ind);
            mb();
        }
        u32PutAdr                       = u32PutAdr + 1;
        if(u32PutAdr >= msmailbox_queue_mgr.pInfo->u32MaxMUnit)
            u32PutAdr = 0;

        msmailbox_queue_mgr.pInfo->u32PutAdr     = u32PutAdr;
        return E_OK;
    }
    return E_MAILBOX_INVALID;
}

_ERR_CODE_ opMBPutAddr(msmailbox_t**  ptMail)
{
    U32                 u32PutAdr;
    U8                  *pBuf;
    if(msmailbox_queue_mgr.bValid)
    {
        u32PutAdr                       = msmailbox_queue_mgr.pInfo->u32PutAdr;
        pBuf                            = (U8*)msmailbox_queue_mgr.pBuf;
        *ptMail                         = (msmailbox_t*)((U32)pBuf + u32PutAdr*MSMAILBOX_SIZE);
        u32PutAdr                       = u32PutAdr + 1;
        if(u32PutAdr >= msmailbox_queue_mgr.pInfo->u32MaxMUnit)
            u32PutAdr = 0;

        msmailbox_queue_mgr.pInfo->u32PutAdr     = u32PutAdr;
        return E_OK;
    }
    else
        return E_MAILBOX_INVALID;
}




