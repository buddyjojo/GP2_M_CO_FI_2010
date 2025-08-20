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

#include <linux/interrupt.h>
#include <linux/spinlock.h>

#include "mhal_msmailbox_reg.h"
#include "mhal_msmailbox.h"
#include "mdrv_msmailbox_st.h"

#define     _DEBUG
#ifdef      _DEBUG
#include <linux/kernel.h>

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT"BUG at %s:%d assert(%s)\n",\
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

_ERR_CODE_ hal_opFire(msmailbox_t *ptMail, U16 CPU_HostID);
_ERR_CODE_ hal_opReceive(msmailbox_t *ptMail, U16 CPU_HostID);
_ERR_CODE_ hal_opFireLoopback(msmailbox_t *ptMail, U16 CPU_SlaveID);
_ERR_CODE_ hal_opInit(void);
void       hal_opOverflow(U16 CPU_HostID, U8 Flag);
hal_msmailbox_t             hal_msmailbox=
{
    .REG_MAIL=              (volatile ms_reg_t*) (MSMAILBOX(0x00)+0xBF200000),
    .ops.Fire=              hal_opFire,
    .ops.Receive=           hal_opReceive,
    .ops.FireLoopback=      hal_opFireLoopback,
    .ops.Init=              hal_opInit,
    .ops.Overflow=          hal_opOverflow,
};
// PM, AEON, H2, H3
U16 MAIL_GROUP[CPUID_MAX_NO] = { 16, 8, 32, 0 };


DEFINE_SPINLOCK(_RIU_LOCK);

void hal_opOverflow(U16 CPU_HostID, U8 Flag)
{
    volatile ms_reg_t   *ptREG;
    if(CPU_HostID >= CPUID_MAX_NO)
    {
        MSMAILBOX_ASSERT(FALSE);
    }

    ptREG = hal_msmailbox.REG_MAIL;
    ptREG = ptREG + MAIL_GROUP[CPU_HostID];

    if(Flag == TRUE)
    {
        _OVERFLOW_S(ptREG);
    }
    else
    {
        _OVERFLOW_C(ptREG);
    }
}


_ERR_CODE_ hal_opInit(void)
{
    int                 s32MailIdx;
    volatile ms_reg_t   *ptREG;

    ptREG = hal_msmailbox.REG_MAIL;

    for(s32MailIdx=0; s32MailIdx<8; s32MailIdx++)
    {
        ptREG[s32MailIdx + MAIL_GROUP[CPUID_PM] ].W[0] = 0x00;
        ptREG[s32MailIdx + MAIL_GROUP[CPUID_H3] ].W[0] = 0x00;
        ptREG[s32MailIdx + MAIL_GROUP[CPUID_AEON] ].W[0] = 0x00;
        mb();
    }
    return E_OK;
}

_ERR_CODE_ hal_opFireLoopback(msmailbox_t *ptMail, U16 CPU_SlaveID)
{
    int                 s32MailIdx;
    unsigned short *    pu16MailBuf;
    BOOL                bIPState;
    volatile ms_reg_t   *ptREG;
    pu16MailBuf      = (unsigned short*)ptMail;

    if(CPU_SlaveID >= CPUID_MAX_NO)
    {
        MSMAILBOX_ASSERT(FALSE);
    }
    ptREG = hal_msmailbox.REG_MAIL;
    ptREG = ptREG + MAIL_GROUP[CPU_SlaveID];

    spin_lock_bh(&_RIU_LOCK);

    /* check send bit in CONTROL register. */
    mb();
    bIPState = _FIRE(ptREG);
    //if(bIPState == ~FIRE_FLAG)
    if(bIPState > 0)
    {
        MSMAILBOX_KDBG("[TIME OUT] : co-processor is not ready\n");
        spin_unlock_bh(&_RIU_LOCK);
        return  E_HAL_COPROCESSOR_NOT_READY;
    }

    mb();
    //bIPState = _BUSY(AEON);
    bIPState = _BUSY(ptREG);
    //if(bIPState == ~BUSY_FLAG)
    if(bIPState > 0)
    {
        MSMAILBOX_KDBG("[TIME OUT] : co-processor is busy\n");
        spin_unlock_bh(&_RIU_LOCK);
        return  E_HAL_COPROCESSOR_BUSY;
    }

    #if 1
    mb();
    //bIPState = _OVERFLOW(AEON);
    bIPState = _OVERFLOW(ptREG);
    //if(bIPState == ~OVERFLOW_FLAG)
    if(bIPState > 0)
    {
        MSMAILBOX_KDBG("[TIME OUT] : co-processor is overflow\n");
        spin_unlock_bh(&_RIU_LOCK);
        return  E_HAL_COPROCESSOR_OVERFLOW;
    }
    #endif
    /* clear status1 register. */
    _S1_C(ptREG);

    /* fill mail box register. */
    for(s32MailIdx=0; s32MailIdx<8; s32MailIdx++)
    {
        //MSMAILBOX_KDBG("Internel Mail[%x] : 0x%x\n", s32MailIdx, (U16)*(pu16MailBuf+s32MailIdx));

        _MAIL(ptREG,s32MailIdx) = (U16)*(pu16MailBuf+s32MailIdx);
        mb();
    }
    spin_unlock_bh(&_RIU_LOCK);
    _FIRE_S(ptREG);


    return E_OK;
}


_ERR_CODE_ hal_opFire(msmailbox_t *ptMail, U16 CPU_HostID)
{
    int                 s32MailIdx;
    unsigned short *    pu16MailBuf;
    BOOL                bIPState;
    volatile ms_reg_t   *ptREG;
    pu16MailBuf      = (unsigned short*)ptMail;

    if(CPU_HostID >= CPUID_MAX_NO)
    {
        MSMAILBOX_ASSERT(FALSE);
    }

    ptREG = hal_msmailbox.REG_MAIL;
    ptREG = ptREG + MAIL_GROUP[CPU_HostID];

    spin_lock_bh(&_RIU_LOCK);

    /* check send bit in CONTROL register. */
    mb();
    bIPState = _FIRE(ptREG);
    //if(bIPState == ~FIRE_FLAG)
    if(bIPState > 0)
    {
        MSMAILBOX_KDBG("[TIME OUT] : co-processor is not ready\n");
        spin_unlock_bh(&_RIU_LOCK);
        return  E_HAL_COPROCESSOR_NOT_READY;
    }

    mb();
    bIPState = _BUSY(ptREG);
    //if(bIPState == ~BUSY_FLAG)
    if(bIPState > 0)
    {
        MSMAILBOX_KDBG("[TIME OUT] : co-processor is busy\n");
        spin_unlock_bh(&_RIU_LOCK);
        return  E_HAL_COPROCESSOR_BUSY;
    }

    mb();
    #if 0
    bIPState = _OVERFLOW(ptREG);
    //if(bIPState == ~OVERFLOW_FLAG)
    if(bIPState > 0)
    {
        MSMAILBOX_KDBG("[TIME OUT] : co-processor is overflow\n");
        spin_unlock_bh(&_RIU_LOCK);
        return  E_HAL_COPROCESSOR_OVERFLOW;
    }
    #endif
    /* clear status1 register. */
    _S1_C(ptREG);

    /* fill mail box register. */
    for(s32MailIdx=0; s32MailIdx<8; s32MailIdx++)
    {
        //MSMAILBOX_KDBG("Internel Mail[%x] : 0x%x\n", s32MailIdx, (U16)*(pu16MailBuf+s32MailIdx));

        _MAIL(ptREG,s32MailIdx) = (U16)*(pu16MailBuf+s32MailIdx);
        mb();
    }
    spin_unlock_bh(&_RIU_LOCK);
    _FIRE_S(ptREG);

    /* fire mail interrupt to AEON */
    /* Does check mail be received by AEON? */
    return E_OK;
}

_ERR_CODE_ hal_opReceive(msmailbox_t *ptMail, U16 CPU_HostID)
{
    U32                 u32Ind;
    volatile U16        *pu16DstMail;

    volatile ms_reg_t   *ptREG;

    if(CPU_HostID >= CPUID_MAX_NO)
    {
        MSMAILBOX_ASSERT(FALSE);
    }

    ptREG = hal_msmailbox.REG_MAIL;
    ptREG = ptREG + MAIL_GROUP[CPU_HostID];


    /* set AEON busy bit */
    _BUSY_S(ptREG);

    /* mail from AEON need to instant handle */
    if( _INSTANT(ptREG) )
    {
        MSMAILBOX_ASSERT(FALSE);
        _BUSY_C(ptREG);
        /* clear AEON send bit */
        _FIRE_C(ptREG);
        return E_HAL_INSTANT_MAIL;
    }
    /* mail from AEON need to process immediately*/
    else
    if( _READBACK(ptREG) )
    {
        MSMAILBOX_ASSERT(FALSE);
        _BUSY_C(ptREG);
        /* clear AEON send bit */
        _FIRE_C(ptREG);
        return E_HAL_INSTANT_MAIL;
    }
    /* mail from AEON */
    else
    {
        pu16DstMail = (volatile U16*)ptMail;
        for(u32Ind=0; u32Ind<8; u32Ind++)
        {
            pu16DstMail[u32Ind] = (U16)(_MAIL(ptREG,u32Ind));
            mb();
        }
    }
    _BUSY_C(ptREG);
    /* clear AEON send bit */
    _FIRE_C(ptREG);

    return E_OK;

}




