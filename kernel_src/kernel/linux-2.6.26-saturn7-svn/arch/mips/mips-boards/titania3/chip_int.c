/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000, 2001, 2004 MIPS Technologies, Inc.
 * Copyright (C) 2001 Ralf Baechle
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Routines for generic manipulation of the interrupts found on the MIPS
 * Malta board.
 * The interrupt controller is located in the South Bridge a PIIX4 device
 * with two internal 82C95 interrupt controllers.
 */
//#include <linux/config.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/random.h>

//#include <asm/i8259.h>
#include <asm/io.h>

#include <asm/mips-boards/malta.h>
#include <asm/mips-boards/maltaint.h>
#include <asm/mips-boards/piix4.h>
#include <asm/gt64120.h>
#include <asm/mips-boards/generic.h>
#include <asm/mips-boards/msc01_pci.h>
#include "chip_int.h"

#define REG(addr) (*(volatile unsigned int *)(addr))

//static raw_spinlock_t mips_irq_lock = RAW_SPIN_LOCK_UNLOCKED;


static int Titania_EnableInterrupt (InterruptNum eIntNum)
{
    int bRet = 0;

    if (eIntNum == E_IRQ_FIQ_ALL)
    {
            REG(REG_IRQ_MASK_L) &= ~IRQL_ALL;
            REG(REG_IRQ_MASK_H) &= ~IRQH_ALL;
            REG(REG_FIQ_MASK_L) &= ~FIQL_ALL;
            REG(REG_FIQ_MASK_H) &= ~FIQH_ALL;

            REG(REG_IRQ_EXP_MASK_L) &= ~IRQL_EXP_ALL;
            REG(REG_IRQ_EXP_MASK_H) &= ~IRQH_EXP_ALL;
            REG(REG_FIQ_EXP_MASK_L) &= ~FIQL_EXP_ALL;
            REG(REG_FIQ_EXP_MASK_H) &= ~FIQH_EXP_ALL;
    }
    else if ( (eIntNum >= E_IRQL_START) && (eIntNum <= E_IRQL_END) )
    {
            REG(REG_IRQ_MASK_L) &= ~(0x1 << (eIntNum-E_IRQL_START) );
    }
    else if ( (eIntNum >= E_IRQH_START) && (eIntNum <= E_IRQH_END) )
    {
            REG(REG_IRQ_MASK_H) &= ~(0x1 << (eIntNum-E_IRQH_START) );
    }
    else if ( (eIntNum >= E_FIQL_START) && (eIntNum <= E_FIQL_END) )
    {
            REG(REG_FIQ_MASK_L) &= ~(0x1 << (eIntNum-E_FIQL_START) );
    }
    else if ( (eIntNum >= E_FIQH_START) && (eIntNum <= E_FIQH_END) )
    {
            REG(REG_FIQ_MASK_H) &= ~(0x1 << (eIntNum-E_FIQH_START) );
    }
    else if ( (eIntNum >= E_IRQL_EXP_START) && (eIntNum <= E_IRQL_EXP_END) )
    {
            REG(REG_IRQ_EXP_MASK_L) &= ~(0x1 << (eIntNum-E_IRQL_EXP_START) );
    }
    else if ( (eIntNum >= E_IRQH_EXP_START) && (eIntNum <= E_IRQH_EXP_END) )
    {
            REG(REG_IRQ_EXP_MASK_H) &= ~(0x1 << (eIntNum-E_IRQH_EXP_START) );
    }
    else if ( (eIntNum >= E_FIQL_EXP_START) && (eIntNum <= E_FIQL_EXP_END) )
    {
            REG(REG_FIQ_EXP_MASK_L) &= ~(0x1 << (eIntNum-E_FIQL_EXP_START) );
    }
    else if ( (eIntNum >= E_FIQH_EXP_START) && (eIntNum <= E_FIQH_EXP_END) )
    {
            REG(REG_FIQ_EXP_MASK_H) &= ~(0x1 << (eIntNum-E_FIQH_EXP_START) );
    }
    return bRet;
}


static int Titania_DisableInterrupt (InterruptNum eIntNum)
{

    if (eIntNum == E_IRQ_FIQ_ALL)
    {
        REG(REG_IRQ_MASK_L) |= IRQL_ALL;
        REG(REG_IRQ_MASK_H) |= IRQH_ALL;
        REG(REG_FIQ_MASK_L) |= FIQL_ALL;
        REG(REG_FIQ_MASK_H) |= FIQH_ALL;

        REG(REG_IRQ_EXP_MASK_L) |= IRQL_EXP_ALL;
        REG(REG_IRQ_EXP_MASK_H) |= IRQH_EXP_ALL;
        REG(REG_FIQ_EXP_MASK_L) |= FIQL_EXP_ALL;
        REG(REG_FIQ_EXP_MASK_H) |= FIQH_EXP_ALL;
    }
    else if ( (eIntNum >= E_IRQL_START) && (eIntNum <= E_IRQL_END) )
    {
        REG(REG_IRQ_MASK_L) |= (0x1 << (eIntNum-E_IRQL_START) );
    }
    else if ( (eIntNum >= E_IRQH_START) && (eIntNum <= E_IRQH_END) )
    {
        REG(REG_IRQ_MASK_H) |= (0x1 << (eIntNum-E_IRQH_START) );
    }
    else if ( (eIntNum >= E_FIQL_START) && (eIntNum <= E_FIQL_END) )
    {
        REG(REG_FIQ_MASK_L) |= (0x1 << (eIntNum-E_FIQL_START) );
        //REG(REG_FIQ_CLEAR_L) |= (0x1 << (eIntNum-E_FIQL_START) );
        //REG(REG_FIQ_CLEAR_L) &= ~(0x1 << (eIntNum-E_FIQL_START) );
        REG(REG_FIQ_CLEAR_L) = (0x1 << (eIntNum-E_FIQL_START) );
        //REG(REG_FIQ_CLEAR_L) &= ~(0x1 << (eIntNum-E_FIQL_START) );

    }
    else if ( (eIntNum >= E_FIQH_START) && (eIntNum <= E_FIQH_END) )
    {
        REG(REG_FIQ_MASK_H) |= (0x1 << (eIntNum-E_FIQH_START) );
        //REG(REG_FIQ_CLEAR_H) |= (0x1 << (eIntNum-E_FIQH_START) );
        //REG(REG_FIQ_CLEAR_H) &= ~(0x1 << (eIntNum-E_FIQH_START) );
        REG(REG_FIQ_CLEAR_H) = (0x1 << (eIntNum-E_FIQH_START) );
    }
    else if ( (eIntNum >= E_IRQL_EXP_START) && (eIntNum <= E_IRQL_EXP_END) )
    {
        REG(REG_IRQ_EXP_MASK_L) |= (0x1 << (eIntNum-E_IRQL_EXP_START) );
    }
    else if ( (eIntNum >= E_IRQH_EXP_START) && (eIntNum <= E_IRQH_EXP_END) )
    {
        REG(REG_IRQ_EXP_MASK_H) |= (0x1 << (eIntNum-E_IRQH_EXP_START) );
    }
    else if ( (eIntNum >= E_FIQL_EXP_START) && (eIntNum <= E_FIQL_EXP_END) )
    {
        REG(REG_FIQ_EXP_MASK_L) |= (0x1 << (eIntNum-E_FIQL_EXP_START) );
        REG(REG_FIQ_EXP_CLEAR_L) = (0x1 << (eIntNum-E_FIQL_EXP_START) );
        //REG(REG_FIQ_EXP_CLEAR_L) |= (0x1 << (eIntNum-E_FIQL_EXP_START) );
        //REG(REG_FIQ_EXP_CLEAR_L) &= ~(0x1 << (eIntNum-E_FIQL_EXP_START) );
    }
    else if ( (eIntNum >= E_FIQH_EXP_START) && (eIntNum <= E_FIQH_EXP_END) )
    {
        REG(REG_FIQ_EXP_MASK_H) |= (0x1 << (eIntNum-E_FIQH_EXP_START) );
        REG(REG_FIQ_EXP_CLEAR_H) = (0x1 << (eIntNum-E_FIQH_EXP_START) );
        //REG(REG_FIQ_EXP_CLEAR_H) |= (0x1 << (eIntNum-E_FIQH_EXP_START) );
        //REG(REG_FIQ_EXP_CLEAR_H) &= ~(0x1 << (eIntNum-E_FIQH_EXP_START) );
    }
    return 0;
}


void Titania_DisableInterrupt_str (void)
{
    Titania_DisableInterrupt(E_IRQ_FIQ_ALL) ;
}


static unsigned int startup_titania_irq(unsigned int irq)
{
    //enable_lasat_irq(irq);
    Titania_EnableInterrupt((InterruptNum)irq);
    return 0; /* never anything pending */
}

static void shutdown_titania_irq(unsigned int irq)
{
    //enable_lasat_irq(irq);
    Titania_DisableInterrupt((InterruptNum)irq);
    //return 0; /* never anything pending */
}

#define enable_titania_irq startup_titania_irq
#define disable_titania_irq shutdown_titania_irq
#define mask_and_ack_titania_irq shutdown_titania_irq
#define end_titania_irq startup_titania_irq

static unsigned int startup_titania_fiq(unsigned int irq)
{
    //enable_lasat_irq(irq);
    Titania_EnableInterrupt((InterruptNum)irq);
    return 0; /* never anything pending */
}

static void shutdown_titania_fiq(unsigned int irq)
{
    //enable_lasat_irq(irq);
    Titania_DisableInterrupt((InterruptNum)irq);
    //return 0; /* never anything pending */
}

#define enable_titania_fiq startup_titania_fiq
#define disable_titania_fiq shutdown_titania_fiq
#define mask_and_ack_titania_fiq shutdown_titania_fiq
#define end_titania_fiq startup_titania_fiq




// Dean modify to match kernel 2.6.26
static struct hw_interrupt_type titania_irq_type = {
    .name = "Titania IRQ",
    .startup = startup_titania_irq,
    .shutdown = shutdown_titania_irq,
    .enable = (void (*)(unsigned int irq))enable_titania_irq,
    .disable = disable_titania_irq,
    .ack = mask_and_ack_titania_irq,
    .mask = disable_titania_irq,
    .mask_ack = disable_titania_irq,
    .unmask = (void (*)(unsigned int irq))enable_titania_irq,
    .eoi = (void (*)(unsigned int irq))enable_titania_irq,
    .end = (void (*)(unsigned int irq))end_titania_irq
};
static struct hw_interrupt_type titania_fiq_type = {
    .name = "Titania IRQ",
    .startup = startup_titania_fiq,
    .shutdown = shutdown_titania_fiq,
    .enable = (void (*)(unsigned int irq))enable_titania_fiq,
    .disable = disable_titania_fiq,
    .ack = mask_and_ack_titania_fiq,
    .mask = disable_titania_fiq,
    .mask_ack = disable_titania_fiq,
    .unmask = (void (*)(unsigned int irq))enable_titania_fiq,
    .eoi = (void (*)(unsigned int irq))enable_titania_fiq,
    .end = (void (*)(unsigned int irq))end_titania_fiq
};
#define ENABLE_PROBE 1
#if ENABLE_PROBE


extern void PROBE_IO_ENTRY(int DrvID, int IONum);
extern void PROBE_IO_EXIT(int DrvID, int IONum);
extern void PROBE_JIFFIES(void);
extern void PROBE_INT_ENTRY(int INTNum);
extern void PROBE_INT_EXIT(int INTNum);
#else

#define PROBE_IO_ENTRY(DrvID, IONum)
#define PROBE_IO_EXIT(DrvID, IONum)
#define PROBE_JIFFIES()
#define PROBE_INT_ENTRY(INTNum)
#define PROBE_INT_EXIT(INTNum)

#endif

// void Titania_hw0_irqdispatch(struct pt_regs *regs)

void Titania_hw0_irqdispatch(void)
{
    unsigned short u16Reglow,u16Reghigh;

    u16Reglow = (unsigned short)REG(REG_IRQ_PENDING_L);
    u16Reghigh = (unsigned short)REG(REG_IRQ_PENDING_H);

//	u16Reglow += MSTAR_INT_BASE;

    if ( u16Reglow & IRQL_UART )
    {
        PROBE_INT_ENTRY(E_IRQ_UART);
        do_IRQ((unsigned int)E_IRQ_UART);
        PROBE_INT_EXIT(E_IRQ_UART);
    }

    if ( u16Reglow & IRQL_MVD )
    {
        PROBE_INT_ENTRY(E_IRQ_MVD);
        do_IRQ((unsigned int)E_IRQ_MVD);
        PROBE_INT_EXIT(E_IRQ_MVD);
    }

    if ( u16Reglow & IRQL_UHC )
    {
       PROBE_INT_ENTRY(E_IRQ_UHC);
       do_IRQ((unsigned int)E_IRQ_UHC);
       PROBE_INT_EXIT(E_IRQ_UHC);
    }
    if ( u16Reglow & IRQL_DEB )
    {
        PROBE_INT_ENTRY(E_IRQ_DEB);
        do_IRQ((unsigned int)E_IRQ_DEB);
        PROBE_INT_EXIT(E_IRQ_DEB);
    }

#if 0
    if ( u16Reglow & IRQL_UART0 )
    {
        do_IRQ((unsigned int)E_IRQ_UART0);
    }
    if ( u16Reglow & IRQL_UART1 )
    {
        do_IRQ((unsigned int)E_IRQ_UART1);
    }
    if ( u16Reglow & IRQL_UART2 )
    {
         do_IRQ((unsigned int)E_IRQ_UART2);
    }
#endif

    if(u16Reglow & IRQL_EMAC)
    {
        PROBE_INT_ENTRY(E_IRQ_EMAC);
        do_IRQ((unsigned int)E_IRQ_EMAC);
        PROBE_INT_EXIT(E_IRQ_EMAC);
    } // if

#if 0  // TODO: Need to port it to PM interrup
    if(u16Reglow & IRQL_MICOM)
    {
        do_IRQ((unsigned int)E_IRQ_MICOM);
    }
#endif

    if ( u16Reglow & IRQL_COMB )
    {
        PROBE_INT_ENTRY(E_IRQ_COMB);
        do_IRQ((unsigned int)E_IRQ_COMB);
        PROBE_INT_EXIT(E_IRQ_COMB);
    }

    if( u16Reghigh & IRQH_VBI )
    {
        PROBE_INT_ENTRY(E_IRQ_VBI);
        do_IRQ((unsigned int)E_IRQ_VBI);
        PROBE_INT_EXIT(E_IRQ_VBI);
    }

    if ( u16Reghigh & IRQH_TSP )
    {
        PROBE_INT_ENTRY(E_IRQ_TSP);
        do_IRQ((unsigned int)E_IRQ_TSP);
        PROBE_INT_EXIT(E_IRQ_TSP);

    }

#if 0
    if ( u16Reghigh & IRQH_MLINK )
    {
        do_IRQ((unsigned int)E_IRQ_MLINK);
    }
#endif

    if ( u16Reghigh & IRQH_HDMITX )
    {
        PROBE_INT_ENTRY(E_IRQ_HDMITX);
        do_IRQ((unsigned int)E_IRQ_HDMITX);
        PROBE_INT_EXIT(E_IRQ_HDMITX);
    }
    if ( u16Reghigh & IRQH_GOP )
    {
        PROBE_INT_ENTRY(E_IRQ_GOP);
        do_IRQ((unsigned int)E_IRQ_GOP);
        PROBE_INT_EXIT(E_IRQ_GOP);
    }
    if ( u16Reghigh & IRQH_PCM2MCU )
    {
        PROBE_INT_ENTRY(E_IRQ_PCM2MCU);
        do_IRQ((unsigned int)E_IRQ_PCM2MCU);
        PROBE_INT_EXIT(E_IRQ_PCM2MCU);
    }
    if ( u16Reghigh & IRQH_RTC )
    {
        PROBE_INT_ENTRY(E_IRQ_RTC);
        do_IRQ((unsigned int)E_IRQ_RTC);
        PROBE_INT_EXIT(E_IRQ_RTC);
    }

    //2008/10/23 Nick DDC2BI interrupt
    if ( u16Reghigh & IRQH_D2B )
    {
        PROBE_INT_ENTRY(E_IRQ_D2B);
        do_IRQ((unsigned int)E_IRQ_D2B);
        PROBE_INT_EXIT(E_IRQ_D2B);
    }

    u16Reglow = (unsigned short)REG(REG_IRQ_EXP_PENDING_L);
    u16Reghigh = (unsigned short)REG(REG_IRQ_EXP_PENDING_H);

    //u16Reglow += MSTAR_INT_BASE;
    if ( u16Reglow & IRQ_SVD )
    {
        PROBE_INT_ENTRY(E_IRQ_SVD);
        do_IRQ((unsigned int)E_IRQ_SVD);
        PROBE_INT_EXIT(E_IRQ_SVD);
    }
    if ( u16Reglow & IRQ_USB1 )
    {
        PROBE_INT_ENTRY(E_IRQ_USB1);
        do_IRQ((unsigned int)E_IRQ_USB1);
        PROBE_INT_EXIT(E_IRQ_USB1);
    }
    if ( u16Reglow & IRQ_UHC1 )
    {
        PROBE_INT_ENTRY(E_IRQ_UHC1);
        do_IRQ((unsigned int)E_IRQ_UHC1);
        PROBE_INT_EXIT(E_IRQ_UHC1);
    }
    if ( u16Reglow & IRQ_MIU )
    {
        PROBE_INT_ENTRY(E_IRQ_MIU);
        do_IRQ((unsigned int)E_IRQ_MIU);
        PROBE_INT_EXIT(E_IRQ_MIU);
    }
    if ( u16Reglow & IRQ_DIP )
    {
        PROBE_INT_ENTRY(E_IRQ_DIP);
        do_IRQ((unsigned int)E_IRQ_DIP);
        PROBE_INT_EXIT(E_IRQ_DIP);
    }
    if ( u16Reglow & IRQ_M4VE ) {
        PROBE_INT_ENTRY(E_IRQH_EXP_HDMI);
        do_IRQ((unsigned int)E_IRQ_M4VE);
        PROBE_INT_EXIT(E_IRQH_EXP_HDMI);
    }

    //u16RegHigh += MSTAR_INT_BASE;
    if ( u16Reghigh & IRQH_EXP_HDMI ) {
        PROBE_INT_ENTRY(E_IRQH_EXP_HDMI);
        do_IRQ((unsigned int)E_IRQH_EXP_HDMI);
        PROBE_INT_EXIT(E_IRQH_EXP_HDMI);
    }
    //do_IRQ(irq, regs);
}


// void Titania_hw0_fiqdispatch(struct pt_regs *regs)
void Titania_hw0_fiqdispatch(void)
{
    unsigned short u16Reg;

    u16Reg = REG(REG_FIQ_PENDING_H);

    //u16Reg += MSTAR_INT_BASE;

#if 0
    if ( u16Reg & FIQH_VSYN_GOP1 )
    {
        do_IRQ((unsigned int)E_FIQ_VSYN_GOP1);
    }
    if ( u16Reg & FIQH_VSYN_GOP0 )
    {
        do_IRQ((unsigned int)E_FIQ_VSYN_GOP0);
    }
#endif
    if ( u16Reg & FIQH_DSP2UP )
    {
        do_IRQ((unsigned int)E_FIQ_DSP2UP);
    }
    if ( u16Reg & FIQH_IR )
    {
        do_IRQ((unsigned int)E_FIQ_IR);
    }
    if ( u16Reg & FIQH_DSP2MIPS )
    {
        do_IRQ((unsigned int)E_FIQ_DSP2MIPS);
    }
    if ( u16Reg & FIQH_VSYNC_VE4VBI )
    {
        do_IRQ((unsigned int)E_FIQ_VSYNC_VE4VBI);
    }
    if ( u16Reg & FIQH_FIELD_VE4VBI )
    {
        do_IRQ((unsigned int)E_FIQ_FIELD_VE4VBI);
    }

    u16Reg = REG(REG_FIQ_PENDING_L);

    //u16Reg += MSTAR_INT_BASE;

    if( u16Reg & FIQL_DSP2_TO_MIPS)
    {
        do_IRQ((unsigned int)E_FIQ_DSP2_TO_MIPS);
    }

    u16Reg = REG(REG_FIQ_EXP_PENDING_L);

    if ( u16Reg & FIQL_EXP_AEON_TO_8051 )
    {
        do_IRQ((unsigned int)E_FIQL_EXP_INT_AEON_TO_8051);
    }
    if ( u16Reg & FIQL_EXP_8051_TO_AEON )
    {
        do_IRQ((unsigned int)E_FIQL_EXP_INT_8051_TO_AEON);
    }
    if ( u16Reg & FIQL_EXP_8051_TO_MIPS )
    {
        do_IRQ((unsigned int)E_FIQL_EXP_INT_8051_TO_MIPS);
    }
    if ( u16Reg & FIQL_EXP_AEON_TO_MIPS )
    {
        do_IRQ((unsigned int)E_FIQL_EXP_INT_AEON_TO_MIPS);
    }

    u16Reg = REG(REG_FIQ_EXP_PENDING_H);

    if ( u16Reg & FIQH_EXP_MIPS_TO_8051 )
    {
        do_IRQ((unsigned int)E_FIQH_EXP_INT_MIPS_TO_8051);
    }
    if ( u16Reg & FIQH_EXP_MIPS_TO_AEON )
    {
        do_IRQ((unsigned int)E_FIQH_EXP_INT_MIPS_TO_AEON);
    }


}

static inline int clz(unsigned long x)
{
    __asm__(
    "    .set    push                    \n"
    "    .set    mips32                    \n"
    "    clz    %0, %1                    \n"
    "    .set    pop                    \n"
    : "=r" (x)
    : "r" (x));

    return x;
}

/*
 * Version of ffs that only looks at bits 12..15.
 */
static inline unsigned int irq_ffs(unsigned int pending)
{
#if defined(CONFIG_CPU_MIPS32) || defined(CONFIG_CPU_MIPS64)
    return -clz(pending) + 31 - CAUSEB_IP;
#else
    unsigned int a0 = 7;
    unsigned int t0;

    t0 = s0 & 0xf000;
    t0 = t0 < 1;
    t0 = t0 << 2;
    a0 = a0 - t0;
    s0 = s0 << t0;

    t0 = s0 & 0xc000;
    t0 = t0 < 1;
    t0 = t0 << 1;
    a0 = a0 - t0;
    s0 = s0 << t0;

    t0 = s0 & 0x8000;
    t0 = t0 < 1;
    //t0 = t0 << 2;
    a0 = a0 - t0;
    //s0 = s0 << t0;

    return a0;
#endif
}

/*
 * IRQs on the SEAD board look basically are combined together on hardware
 * interrupt 0 (MIPS IRQ 2)) like:
 *
 *    MIPS IRQ    Source
 *      --------        ------
 *             0         Software (ignored)
 *             1        Software (ignored)
 *             2        UART0 (hw0)
 *             3        UART1 (hw1)
 *             4        Hardware (ignored)
 *             5        Hardware (ignored)
 *             6        Hardware (ignored)
 *             7        R4k timer (what we use)
 *
 * We handle the IRQ according to _our_ priority which is:
 *
 * Highest ----     R4k Timer
 * Lowest  ----     Combined hardware interrupt
 *
 * then we just return, if multiple IRQs are pending then we will just take
 * another exception, big deal.
 */
asmlinkage void plat_irq_dispatch(void)
{
    unsigned int pending = read_c0_cause() & read_c0_status() & ST0_IM;
    int irq;

    irq = irq_ffs(pending);
       //printk("\nDean --> [%s] %d irq = %d\n", __FILE__, __LINE__, irq);

    //if(irq != 7)
        //printk("\nirq = %d\n", irq);

    if (irq == 2)
        Titania_hw0_irqdispatch();
    else if (irq == 3)
        Titania_hw0_fiqdispatch();
    else if (irq >= 0)
        do_IRQ(MIPS_CPU_IRQ_BASE + irq);
    else
        spurious_interrupt();

       //printk("\nDean --> [%s] %d irq = %d\n", __FILE__, __LINE__, irq);
}


#if defined(CONFIG_MIPS_MT_SMP) /*AWU -- added*/
#define GIC_MIPS_CPU_IPI_RESCHED_IRQ    3
#define GIC_MIPS_CPU_IPI_CALL_IRQ    4

#define MIPS_CPU_IPI_RESCHED_IRQ 0    /* SW int 0 for resched */
#define C_RESCHED C_SW0
#define MIPS_CPU_IPI_CALL_IRQ 1        /* SW int 1 for resched */
#define C_CALL C_SW1
static int cpu_ipi_resched_irq, cpu_ipi_call_irq;

static void ipi_resched_dispatch(void)
{
    do_IRQ(MIPS_CPU_IRQ_BASE + MIPS_CPU_IPI_RESCHED_IRQ);
}

static void ipi_call_dispatch(void)
{
    do_IRQ(MIPS_CPU_IRQ_BASE + MIPS_CPU_IPI_CALL_IRQ);
}

static irqreturn_t ipi_resched_interrupt(int irq, void *dev_id)
{
    return IRQ_HANDLED;
}

static irqreturn_t ipi_call_interrupt(int irq, void *dev_id)
{
    smp_call_function_interrupt();

    return IRQ_HANDLED;
}

static struct irqaction irq_resched = {
    .handler    = ipi_resched_interrupt,
    .flags        = IRQF_DISABLED|IRQF_PERCPU,
    .name        = "IPI_resched"
};

static struct irqaction irq_call = {
    .handler    = ipi_call_interrupt,
    .flags        = IRQF_DISABLED|IRQF_PERCPU,
    .name        = "IPI_call"
};
#endif /* CONFIG_MIPS_MT_SMP */


extern void __init mips_cpu_irq_init(void);
void __init arch_init_irq(void)
{
    int i;

#if defined(CONFIG_MIPS_MT_SMP) /*AWU -- added*/

    // Samuel,20090330: cpu_has_vint is needed in SMP.
    //     For this we have to setup vi hander. And we have to know that the
    //     ebase is not fixed, it is allocated by linux system in trap.c

    mips_cpu_irq_init();

    if (cpu_has_vint) {

        set_vi_handler(2, Titania_hw0_irqdispatch);
        set_vi_handler(3, Titania_hw0_fiqdispatch);

        set_vi_handler (MIPS_CPU_IPI_RESCHED_IRQ, ipi_resched_dispatch);
        set_vi_handler (MIPS_CPU_IPI_CALL_IRQ, ipi_call_dispatch);

    }

    cpu_ipi_resched_irq = MIPS_CPU_IRQ_BASE + MIPS_CPU_IPI_RESCHED_IRQ;
    cpu_ipi_call_irq = MIPS_CPU_IRQ_BASE + MIPS_CPU_IPI_CALL_IRQ;

    setup_irq(cpu_ipi_resched_irq, &irq_resched);
    setup_irq(cpu_ipi_call_irq, &irq_call);

    set_irq_handler(cpu_ipi_resched_irq, handle_percpu_irq);
    set_irq_handler(cpu_ipi_call_irq, handle_percpu_irq);

    for (i = MSTAR_INT_BASE; i <= (TITANIAINT_END+MSTAR_INT_BASE); i++) {
        if ( i <64+MSTAR_INT_BASE)
            set_irq_chip_and_handler(i, &titania_irq_type, handle_level_irq);
        else
            set_irq_chip_and_handler(i, &titania_fiq_type, handle_level_irq);
    }
#else

    for (i = MSTAR_INT_BASE; i <= (TITANIAINT_END+MSTAR_INT_BASE); i++) {
        if ( i <64+MSTAR_INT_BASE)
            set_irq_chip_and_handler(i, &titania_irq_type, handle_level_irq);
        else
            set_irq_chip_and_handler(i, &titania_fiq_type, handle_level_irq);
    }

    set_irq_chip_and_handler(0, &titania_fiq_type, handle_level_irq);
    set_irq_chip_and_handler(1, &titania_fiq_type, handle_level_irq);
    mips_cpu_irq_init();

#endif

    //disable all
    Titania_DisableInterrupt(E_IRQ_FIQ_ALL);
}
