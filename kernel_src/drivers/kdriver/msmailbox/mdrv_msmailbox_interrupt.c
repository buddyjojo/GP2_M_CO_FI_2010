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

//#include "mdrv_msmailbox.h"
#include "mdrv_msmailbox_interrupt.h"
//#include "mhal_msmailbox.h"

//-------------------------------------------------------------------------------------------------
// DEBUG Macro
//-------------------------------------------------------------------------------------------------
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

#define     INTERRUPT_ASSERT(arg)               assert((arg))
#define     INTERRUPT_KDBG(fmt, args...)        printk(KERN_WARNING fmt, ## args)
#else
#define     INTERRUPT_ASSERT(arg)
#define     INTERRUPT_KDBG(fmt, args...)
#endif


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void    opInit(void *pDev, cb_irq_t);
void    opRelease(void* pDev);
B16     opFlag(U8 u8CPUID);
void    opBypass(U8 u8CPUID);
void    opHandle(U8 u8CPUID);
void    opEnable(U8 u8CPUID);
void    opDisable(U8 u8CPUID);
void    opClear(U8 u8CPUID);

extern hal_int_t                        hal_msmailbox_int;

int_mgr_t                               msmailbox_int_mgr =
{
    .bValid                             = FALSE,
    .ops.Init                           = opInit,
    .ops.Release                        = opRelease,
    .ops.Flag                           = opFlag,
    .ops.Bypass                         = opBypass,
    .ops.Handle                         = opHandle,
    .ops.Disable                        = opDisable,
    .ops.Enable                         = opEnable,
    .ops.IRCB                           = NULL,
    .ops.Clear                          = opClear,
};

#define     INT_MGR                     msmailbox_int_mgr


extern U8                               CPU_IRQNO[CPUID_MAX_NO];
//extern hal_msmailbox_t                  hal_msmailbox;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void _MDrv_MSMailBox_MIPS_Receive_BH(unsigned long unused)
{
    if( msmailbox_int_mgr.ops.IRCB == NULL)
        return;
    (*msmailbox_int_mgr.ops.IRCB)((void*)CPUID_AEON);
}

void _PM2MIPS_BH(unsigned long unused)
{
#if 0
    msmailbox_t tMail;
    U8          tPayoad;

    hal_msmailbox.ops.Receive(&tMail, CPUID_PM);

    INTERRUPT_KDBG("Class 0x%2x\n", tMail.u8Class);
    INTERRUPT_KDBG("Index 0x%2x\n", tMail.u8Index);
    INTERRUPT_KDBG("Count 0x%2x\n", tMail.u8ParameterCount);
    for(tPayoad=0; tPayoad<tMail.u8ParameterCount; tPayoad++)
    {
        INTERRUPT_KDBG("[%2d]=0x%2x \n", tPayoad, tMail.u8Parameters[tPayoad]);
    }
#endif

    if( msmailbox_int_mgr.ops.IRCB == NULL)
        return;
    (*msmailbox_int_mgr.ops.IRCB)((void*)CPUID_PM);

}

DECLARE_TASKLET(MSMailBoxTasklet, _MDrv_MSMailBox_MIPS_Receive_BH, 0);
DECLARE_TASKLET(PM2MIPS, _PM2MIPS_BH, 0);

irqreturn_t MDrv_MSMailBox_Interrupt(int irq, void *dev_id)
{
    switch(irq)
    {
    case E_FIQ_INT_8051_TO_MIPS:
        if(INT_MGR.ops.Flag(CPUID_PM) == START_INTERRUPT_FLAG)
        {
            tasklet_schedule(&PM2MIPS);
        }
        INT_MGR.ops.Clear(CPUID_PM);
        break;
    case E_FIQ_INT_AEON_TO_MIPS:
        if(INT_MGR.ops.Flag(CPUID_AEON) == START_INTERRUPT_FLAG)
        {
            tasklet_schedule(&MSMailBoxTasklet);
        }
        INT_MGR.ops.Clear(CPUID_AEON);
        break;
    default:
        break;
    }
    return IRQ_HANDLED;
}

static unsigned int u32AEON_TO_MIPSIntRegister;
static unsigned int u328051_TO_MIPSIntRegister;

void opInit(void *pDev, cb_irq_t func)
{
    int     s32Err;

    INTERRUPT_ASSERT(pDev!=NULL);
    if(pDev == NULL)
        return;

    if(INT_MGR.bValid == FALSE)
    {
        INT_MGR.bFiq                = TRUE;
        INT_MGR.ops.IRCB            = func;
//2.6.26
//        s32Err = request_irq(INT_MGR.u32IrqNum+FIQ_BASE_NUM, MDrv_MSMailBox_Interrupt, SA_INTERRUPT, "MSMAILBOX_FIQ", pDev);
        //s32Err = request_irq(INT_MGR.u32IrqNum+E_FIQL_START, MDrv_MSMailBox_Interrupt, SA_INTERRUPT, "MSMAILBOX_FIQ", pDev);
        //s32Err = request_irq(E_FIQL_EXP_INT_AEON_TO_MIPS, MDrv_MSMailBox_Interrupt, SA_INTERRUPT, "MSMAILBOX_FIQ", pDev);
		if(0 == u32AEON_TO_MIPSIntRegister) {
        s32Err = request_irq(E_FIQ_INT_AEON_TO_MIPS, MDrv_MSMailBox_Interrupt, SA_INTERRUPT, "MSMAILBOX_FIQ", pDev);
			if( s32Err ){
				return -EBUSY;
			}
			u32AEON_TO_MIPSIntRegister= 1;
		} else {
			disable_irq(E_FIQ_INT_AEON_TO_MIPS);
			enable_irq(E_FIQ_INT_AEON_TO_MIPS);
		}

        //INTERRUPT_ASSERT(s32Err==0);
		if(0 == u328051_TO_MIPSIntRegister) {
        s32Err = request_irq(E_FIQ_INT_8051_TO_MIPS, MDrv_MSMailBox_Interrupt, SA_INTERRUPT, "MSMAILBOX_FIQ", pDev);
			if( s32Err ){
				return -EBUSY;
			}
			u328051_TO_MIPSIntRegister= 1;
		} else {
			disable_irq(E_FIQ_INT_8051_TO_MIPS);
			enable_irq(E_FIQ_INT_8051_TO_MIPS);
		}

        //INTERRUPT_ASSERT(s32Err==0);

        if(s32Err != 0)
        {
            INTERRUPT_KDBG("Mail Box IRQ register failure\n");
            INT_MGR.bValid          = FALSE;
        }
        else
        {
            INTERRUPT_KDBG("Mail Box IRQ register\n");
            INT_MGR.bValid          = TRUE;
        }
        INT_MGR.ops.Disable(CPUID_AEON);
        INT_MGR.ops.Disable(CPUID_PM);
    }
}

void opRelease(void* pDev)
{
    if(INT_MGR.bValid == TRUE)
    {
        INT_MGR.ops.Disable(CPUID_AEON);
        INT_MGR.ops.Disable(CPUID_PM);
        //free_irq(E_FIQL_EXP_INT_AEON_TO_MIPS, pDev);
		u32AEON_TO_MIPSIntRegister= 0;
		u328051_TO_MIPSIntRegister= 0;
        free_irq(E_FIQ_INT_AEON_TO_MIPS, pDev);
        free_irq(E_FIQ_INT_8051_TO_MIPS, pDev);
        INT_MGR.bValid              = FALSE;
    }
}

B16 opFlag(U8 u8CPUID)
{
    INTERRUPT_ASSERT(u8CPUID < CPUID_MAX_NO);

    if(INT_MGR.bValid)
        return atomic_read(&INT_MGR.aFlag[u8CPUID]);
    else
        return STOP_INTERRUPT_FLAG;
}

void opBypass(U8 u8CPUID)
{
    INTERRUPT_ASSERT(u8CPUID < CPUID_MAX_NO);

    if(INT_MGR.bValid)
        atomic_set(&INT_MGR.aFlag[u8CPUID], STOP_INTERRUPT_FLAG);
}

void opHandle(U8 u8CPUID)
{
    INTERRUPT_ASSERT(u8CPUID < CPUID_MAX_NO);

    if(INT_MGR.bValid)
        atomic_set(&INT_MGR.aFlag[u8CPUID], START_INTERRUPT_FLAG);
}

void opEnable(U8 u8CPUID)
{
    INTERRUPT_ASSERT(u8CPUID < CPUID_MAX_NO);

    if(INT_MGR.bValid)
    {
        atomic_set(&INT_MGR.aFlag[u8CPUID], START_INTERRUPT_FLAG);

        if(INT_MGR.bFiq)
            hal_msmailbox_int.ops.FEnable(CPU_IRQNO[u8CPUID]);
        else
            hal_msmailbox_int.ops.IEnable(CPU_IRQNO[u8CPUID]);
    }
}

void opClear(U8 u8CPUID)
{
    INTERRUPT_ASSERT(u8CPUID < CPUID_MAX_NO);

    if(INT_MGR.bValid)
    {
        if(INT_MGR.bFiq)
            hal_msmailbox_int.ops.FClear(CPU_IRQNO[u8CPUID]);
        else
            hal_msmailbox_int.ops.FClear(CPU_IRQNO[u8CPUID]);
    }
}

void opDisable(U8 u8CPUID)
{
    INTERRUPT_ASSERT(u8CPUID < CPUID_MAX_NO);

    if(INT_MGR.bValid)
    {
        atomic_set(&INT_MGR.aFlag[u8CPUID], STOP_INTERRUPT_FLAG);

        if(INT_MGR.bFiq)
            hal_msmailbox_int.ops.FDisable(CPU_IRQNO[u8CPUID]);
        else
            hal_msmailbox_int.ops.IDisable(CPU_IRQNO[u8CPUID]);

    }
}



