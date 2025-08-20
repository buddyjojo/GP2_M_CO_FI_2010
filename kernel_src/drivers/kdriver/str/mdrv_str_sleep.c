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

#include <asm/cacheflush.h>
#include <asm-mips/tlbflush.h>


extern void asm_invalid_cache(void);
extern void deep_sleep(unsigned int stack_address_backup_addr);
extern void save_cp0(unsigned int);
extern void save_tlb_entries(unsigned int, unsigned int);
extern void set_tlb_boundary(unsigned int);
extern void restore_cp0(unsigned int);
extern void restore_tlb_entries(unsigned int, unsigned int);
extern void ddr_enter_selfrefresh(void);

#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)


typedef struct {
	unsigned int		  CP0_R0_S1;	  //MVPControl
	unsigned int		  CP0_R1_S1;	  //VPEControl
	unsigned int		  CP0_R1_S2;	  //VPEConf0
	unsigned int		  CP0_R1_S3;	  //VPEConf1
	unsigned int		  CP0_R1_S4;	  //YQMASK
	unsigned int		  CP0_R1_S7;	  //VPEOpt

	unsigned int		  CP0_R2_S1;	  //TCStatus
	unsigned int		  CP0_R2_S2;	  //TCBind
	unsigned int		  CP0_R2_S5;	  //TCContext
	unsigned int		  CP0_R2_S6;	  //TCSchedule
	unsigned int		  CP0_R2_S7;	  //TCScheFBack

	unsigned int		  CP0_R4_S0;	  //Context
	unsigned int		  CP0_R4_S2;	  //UserLocal

	unsigned int		  CP0_R5_S0;	  //PageMask

	unsigned int		  CP0_R6_S0;	  //Wired
	unsigned int		  CP0_R6_S1;	  //SRSConf

	unsigned int		  CP0_R7_S0;	  //HWREna

	// Don't change below two register sequence
	unsigned int		  CP0_R11_S0;	  //Compare
	unsigned int		  CP0_R9_S0;	  //Count
	
	unsigned int		  CP0_R12_S0;	  //Status
	unsigned int		  CP0_R12_S1;	  //IntCtl
	unsigned int		  CP0_R12_S2;	  //SRSCtl
	unsigned int		  CP0_R12_S3;	  //SRSMap

	unsigned int		  CP0_R13_S0;	  //Cause

	unsigned int		  CP0_R14_S0;	  //Exception

	unsigned int		  CP0_R15_S1;	  //EBase

	unsigned int		  CP0_R16_S0;	  //Config
	unsigned int		  CP0_R16_S1;	  //Config1
	unsigned int		  CP0_R16_S2;	  //Config2
	unsigned int		  CP0_R16_S7;	  //Config7

	unsigned int		  CP0_R25_S0;	  //Performance
	unsigned int		  CP0_R25_S1;	  //Performance
	unsigned int		  CP0_R25_S2;	  //Performance
	unsigned int		  CP0_R25_S3;	  //Performance

	unsigned int		  CP0_R26_S0;	  //ErrCtl

	unsigned int		  CP0_R30_S0;	  //ErrEPC
}__MIPS_CP0__;

typedef struct {
    unsigned int          EntryHi;
    unsigned int          EntryLo0;
    unsigned int          EntryLo1;
    unsigned int          PageMask;
}__MIPS_TLB_ENTRY__;


#define SIZE_OF_CP0_SAVE_AREA		sizeof(__MIPS_CP0__)
#define SIZE_OF_TLB_LINE			(16)
//#define DEBUG_TLB

// If you want change below address, you must change boot_1st's asm_str.inc also.
#define DEEP_SLEEP_SP_BACKUP_ADDR	0xa0C00000

void start_deep_sleep(void)
{
    unsigned int    num_of_tlb_entries;
    unsigned int    i;
	unsigned int	backup;
	__MIPS_CP0__	*pCP0_DATA;
	__MIPS_TLB_ENTRY__ *pTLB;

	pCP0_DATA = kzalloc( SIZE_OF_CP0_SAVE_AREA, GFP_KERNEL );

    // save CP0
    save_cp0( (unsigned int)pCP0_DATA );

    num_of_tlb_entries = ( ( pCP0_DATA->CP0_R16_S1 >> 25 ) &( BITMASK( 5:0 ) ) ) + 1;
		
    set_tlb_boundary( num_of_tlb_entries - 1 );

	pTLB = kzalloc( SIZE_OF_TLB_LINE * num_of_tlb_entries, GFP_KERNEL );
	
    i = num_of_tlb_entries;
	
    while(i--)
    {
        save_tlb_entries(i, ( unsigned int )( ( unsigned char * )pTLB + SIZE_OF_TLB_LINE * i));
    }

	#ifdef DEBUG_TLB
		__MIPS_TLB_ENTRY__ *pTLB_debug;
		printk("%s %d\n", __func__, __LINE__);
		printk("CP0 R4 S0 = \t%08x\n", pCP0_DATA->CP0_R4_S0);
		printk("CP0 R6 S0 = \t%08x\n", pCP0_DATA->CP0_R6_S0);
		printk("CP0 R7 S0 = \t%08x\n", pCP0_DATA->CP0_R7_S0);
		printk("CP0 R9 S0 = \t%08x\n", pCP0_DATA->CP0_R9_S0);
		printk("CP0 R11 S0 = \t%08x\n", pCP0_DATA->CP0_R11_S0);
		printk("CP0 R12 S0 = \t%08x\n", pCP0_DATA->CP0_R12_S0);
		printk("CP0 R12 S3 = \t%08x\n", pCP0_DATA->CP0_R12_S3);
		printk("CP0 R13 S0 = \t%08x\n", pCP0_DATA->CP0_R13_S0);
		printk("CP0 R14 S0 = \t%08x\n", pCP0_DATA->CP0_R14_S0);
		printk("CP0 R15 S1 = \t%08x\n", pCP0_DATA->CP0_R15_S1);
		printk("CP0 R16 S0 = \t%08x\n", pCP0_DATA->CP0_R16_S0);
		printk("CP0 R16 S1 = \t%08x\n", pCP0_DATA->CP0_R16_S1);
		printk("CP0 R16 S2 = \t%08x\n", pCP0_DATA->CP0_R16_S2);
		printk("CP0 R12 S2 = \t%08x\n", pCP0_DATA->CP0_R12_S2);

		i = ((pCP0_DATA->CP0_R16_S1>>25)&(BITMASK(5:0)))+1;
		printk("MMU TLB Entries %d\n", i );
		printk("MMU TLB Boundary %d\n", pCP0_DATA->CP0_R6_S0);
		while(i--)
		{
			pTLB_debug = (__MIPS_TLB_ENTRY__*)((unsigned int)pTLB+sizeof(unsigned int)*100+i*sizeof(__MIPS_TLB_ENTRY__));
			printk("TLB Entry = \t%04d\n", i);
			printk("EntryHi = \t0x%08x\n", pTLB_debug->EntryHi);
			printk("EntryLo0 = \t0x%08x\n", pTLB_debug->EntryLo0);
			printk("EntryLo1 = \t0x%08x\n", pTLB_debug->EntryLo1);
			printk("PageMask = \t0x%08x\n", (pTLB_debug->PageMask>>11));
		}
	#endif

	backup = *(unsigned int *)DEEP_SLEEP_SP_BACKUP_ADDR;
	
	// real power off, we will restart after this function when power on.
    deep_sleep(DEEP_SLEEP_SP_BACKUP_ADDR);

	*(unsigned int *)DEEP_SLEEP_SP_BACKUP_ADDR = backup;

    // restore tlb entries
    i = ((pCP0_DATA->CP0_R16_S1>>25)&(BITMASK(5:0)))+1;
    while(i--)
    {
        restore_tlb_entries( i, ( unsigned int )( ( unsigned char * )pTLB + SIZE_OF_TLB_LINE * i) );
    }

    restore_cp0( (unsigned int)pCP0_DATA );

	kfree(pCP0_DATA);
	kfree(pTLB);
	
    asm_invalid_cache();
	
    return;
}
