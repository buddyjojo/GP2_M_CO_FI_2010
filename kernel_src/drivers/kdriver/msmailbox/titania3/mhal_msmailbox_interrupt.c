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

#include <asm-mips/system.h>

#include "mhal_msmailbox_reg.h"
#include "mhal_msmailbox_interrupt.h"

/* --
SYSTEM DEBUG
-- */
#define     _DEBUG
#ifdef      _DEBUG
#include <linux/kernel.h>
#include <asm-mips/delay.h>

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT"BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                            BUG();\
                        }\
                    } while (0)

#define     INTERRUPT_ASSERT(arg)               assert((arg))
#define     INTERRUPT_KDBG(fmt, args...)        printk(KERN_WARNING"%s:%d " fmt,__FUNCTION__,__LINE__,## args)
#else
#define     INTERRUPT_ASSERT(arg)
#define     INTERRUPT_KDBG(fmt, args...)
#endif
/*-------------------------------------------------------------------*/

U32         INTERRUPT_MAP[64] =
{
    BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6, BIT7, BIT8, BIT9, BIT10, BIT11, BIT12, BIT13, BIT14, BIT15, /* 0 - 15 */
    BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6, BIT7, BIT8, BIT9, BIT10, BIT11, BIT12, BIT13, BIT14, BIT15, /* 16 - 31 */
    BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6, BIT7, BIT8, BIT9, BIT10, BIT11, BIT12, BIT13, BIT14, BIT15, /* 32 - 47 */
    BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6, BIT7, BIT8, BIT9, BIT10, BIT11, BIT12, BIT13, BIT14, BIT15  /* 48 - 63 */
};


U8  CPUINT_MAP[4][4] =
{
    {0, BIT0, BIT1, BIT2},
    {BIT0, 0, BIT1, BIT2},
    {BIT0, BIT1, 0, BIT2},
    {BIT0, BIT1, BIT2, 0}
};

// PM, AEON, H2, H3 to H3
U8  CPU_IRQNO[CPUID_MAX_NO] =
{
    36, 40, 44, 0xFF
};


void            EnableFiq_VPE0  (U32);
void            DisableFiq_VPE0 (U32);
void            EnableIrq_VPE0  (U32);
void            DisableIrq_VPE0 (U32);
U32             StatusFiq_VPE0  (U32);
U32             StatusIrq_VPE0  (U32);
void            ClearFiq_VPE0   (U32);

void            IAEON           (void);
void            IMIPS           (void);
void            ICO_PROC        (U32);
void            IMIPS_VPE0      (U32);
void            ICO_PROC2       (U32, U32);
void            IPM2MIPS        (void);

hal_int_t       hal_msmailbox_int=
{
    .REG=                   (volatile ms_reg_t*)(INT(0)+0xBF200000),
    .REG_CPU_INT=           (volatile ms_reg_t*)(INTCPU(0)+0xBF200000),
    .ops.IEnable=           EnableIrq_VPE0,
    .ops.IDisable=          DisableIrq_VPE0,
    .ops.IStatus=           StatusIrq_VPE0,
    .ops.FEnable=           EnableFiq_VPE0,
    .ops.FDisable=          DisableFiq_VPE0,
    .ops.FStatus=           StatusFiq_VPE0,
    .ops.FClear=            ClearFiq_VPE0,
    .ops.IAEON=             IAEON,
    .ops.IMIPS=             IMIPS,
    .ops.ICO_PROC=          ICO_PROC,
    .ops.IMIPS_VPE0=        IMIPS_VPE0,
    .ops.IPROC=             ICO_PROC2,
};

void ICO_PROC2(U32 u32SrcCPUID, U32 u32DstCPUID)
{
    U8 DstCPUBIT;

    // HOST 0   8051
    // HOST 1   AEON
    // HOST 2   MIPS VPE1
    // HOST 3   MIPS VPE0
    if(u32SrcCPUID >= CPUID_MAX_NO || u32DstCPUID >= CPUID_MAX_NO)
    {
       INTERRUPT_ASSERT(0);
    }

    if(u32SrcCPUID == u32DstCPUID)
    {
       INTERRUPT_ASSERT(0);
    }

    DstCPUBIT = CPUINT_MAP[u32SrcCPUID][u32DstCPUID];

    u32SrcCPUID = (u32SrcCPUID <<1);
    mb();
    hal_msmailbox_int.REG_CPU_INT[u32SrcCPUID].W[0] |= DstCPUBIT;
    mb();
    //udelay(10);
    hal_msmailbox_int.REG_CPU_INT[u32SrcCPUID].W[0] &= ~(DstCPUBIT);
    mb();
}

/* fire interrupt, PM to MIPS */
void IPM2MIPS(void)
{
    // PM to MIPS
    IMIPS_VPE0(CPUID_PM);
}

/* fire interrupt, MIPS to PM */
void IMIPS2PM(void)
{
    // MIPS to PM
    ICO_PROC(CPUID_PM);
}


void IMIPS_VPE0(U32 u32FormHost)
{
    // HOST 0   8051
    // HOST 1   AEON
    // HOST 2   MIPS VPE1
    // HOST 3   MIPS VPE0 (Current)
    if(u32FormHost > 3)
    {
       INTERRUPT_ASSERT(0);
    }
    u32FormHost = (u32FormHost <<1);
    mb();
    hal_msmailbox_int.REG_CPU_INT[u32FormHost].W[0] |= BIT2;
    mb();
    //udelay(10);
    hal_msmailbox_int.REG_CPU_INT[u32FormHost].W[0] &= ~(BIT2);
    mb();
}

void ICO_PROC(U32 u32ToHot)
{
    // HOST 0   8051
    // HOST 1   AEON
    // HOST 2   MIPS VPE1
    // HOST 3   MIPS VPE0 (Current)
    if(u32ToHot > 3)
    {
       INTERRUPT_ASSERT(0);
    }
    u32ToHot = INTERRUPT_MAP[u32ToHot];
    mb();
    hal_msmailbox_int.REG_CPU_INT[0x06].W[0] |= u32ToHot;
    mb();
    //udelay(10);
    hal_msmailbox_int.REG_CPU_INT[0x06].W[0] &= ~(u32ToHot);
    mb();
}

/* fire interrupt to AEON */
void IAEON(void)
{
    // MIPS to AEON
    ICO_PROC(1);
}

/* fire interrupt to MIPS */
void IMIPS(void)
{
    // AEON to MIPS
    IMIPS_VPE0(1);
}

void ClearFiq_VPE0(U32 u32FiqNum)
{
    U32     u32FiqBit;
    U16     u16Val;

    INTERRUPT_ASSERT(u32FiqNum <= 63);
    if(u32FiqNum > 64)
        return;
    u32FiqBit = INTERRUPT_MAP[u32FiqNum];
    /*
    15 ... 000 1111b
    31 ... 001 1111b
    47 ... 010 1111b
    63 ... 011 1111b
    */
    u16Val = 0x6C + (u32FiqNum>>4);
    mb();
    //hal_msmailbox_int.REG[u16Val].W[0] |= u32FiqBit;
    hal_msmailbox_int.REG[u16Val].W[0] = u32FiqBit;
    mb();
    /*
    udelay(10);
    hal_msmailbox_int.REG[u16Val].W[0] &= ~(u32FiqBit);
    mb();
    */
}

void EnableFiq_VPE0(U32 u32FiqNum)
{
    U32     u32FiqBit;
    U16     u16Val;

    INTERRUPT_ASSERT(u32FiqNum <= 63);
    if(u32FiqNum > 63)
        return;

    u32FiqBit = INTERRUPT_MAP[u32FiqNum];
    /*
    15 ... 000 1111b
    31 ... 001 1111b
    47 ... 010 1111b
    63 ... 011 1111b
    */
    u16Val = 0x64 + (u32FiqNum>>4);
    mb();
    hal_msmailbox_int.REG[u16Val].W[0] &= ~(u32FiqBit);
    mb();
}

void DisableFiq_VPE0(U32 u32FiqNum)
{
    U32     u32FiqBit;
    U16     u16Val;

    INTERRUPT_ASSERT(u32FiqNum <= 63);
    if(u32FiqNum > 63)
        return;

    u32FiqBit = INTERRUPT_MAP[u32FiqNum];
    /*
    15 ... 000 1111b
    31 ... 001 1111b
    47 ... 010 1111b
    63 ... 011 1111b
    */
    u16Val = 0x64 + (u32FiqNum>>4);
    mb();
    hal_msmailbox_int.REG[u16Val].W[0] |= (u32FiqBit);
    mb();
}


void EnableIrq_VPE0(U32 u32IrqNum)
{
    U32     u32IrqBit;
    U16     u16Val;

    INTERRUPT_ASSERT(u32IrqNum <= 63);
    if(u32IrqNum > 63)
        return;

    u32IrqBit = INTERRUPT_MAP[u32IrqNum];
    /*
    15 ... 000 1111b
    31 ... 001 1111b
    47 ... 010 1111b
    63 ... 011 1111b
    */
    u16Val = 0x74 + (u32IrqNum>>4);
    mb();
    hal_msmailbox_int.REG[u16Val].W[0] = u16Val & ~(u32IrqBit);
    mb();

}

void DisableIrq_VPE0(U32 u32IrqNum)
{
    U32     u32IrqBit;
    U16     u16Val;

    INTERRUPT_ASSERT(u32IrqNum <= 63);
    if(u32IrqNum > 63)
        return;

    u32IrqBit = INTERRUPT_MAP[u32IrqNum];
    u16Val = 0x74 + (u32IrqNum>>4);
    mb();
    hal_msmailbox_int.REG[u16Val].W[0] |= (u32IrqBit);
    mb();

}

U32 StatusFiq_VPE0(U32 u32FiqNum)
{
    U32     u32FiqBit;
    U32     u32Status = 0;
    U16     u16Val;

    INTERRUPT_ASSERT(u32FiqNum <= 63);
    if(u32FiqNum > 63)
        return FALSE;

    u32FiqBit = INTERRUPT_MAP[u32FiqNum];
    u16Val = 0x6C + (u32FiqNum>>4);
    mb();
    u32Status = (hal_msmailbox_int.REG[u16Val].W[0]&u32FiqBit);
    mb();

    if(u32Status >= 0)
        return FALSE;
    else
        return TRUE;
}


U32 StatusIrq_VPE0(U32 u32IrqNum)
{
    U32     u32IrqBit;
    U32     u32Status = 0;
    U16     u16Val;

    INTERRUPT_ASSERT(u32IrqNum <= 63);
    if(u32IrqNum > 63)
        return FALSE;

    u32IrqBit = INTERRUPT_MAP[u32IrqNum];
    u16Val = 0x7C + (u32IrqNum>>4);
    mb();
    u32Status = (hal_msmailbox_int.REG[u16Val].W[0]&u32IrqBit);
    mb();

    if(u32Status >= 0)
        return FALSE;
    else
        return TRUE;
}

