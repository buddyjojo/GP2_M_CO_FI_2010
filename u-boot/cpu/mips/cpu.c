/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/inca-ip.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>

#if defined(CONFIG_TITANIA2)
#define REG_WDT_BASE	0xbf8078c0
#elif defined(CONFIG_TITANIA3_FPGA) ||  defined(CONFIG_GP2_DEMO1)
#define REG_WDT_BASE	0xbf006000
#endif

#define cache_op(op,addr)						\
	__asm__ __volatile__(						\
			"	.set	push				\n"	\
			"	.set	noreorder			\n"	\
			"	.set	mips3\n\t			\n"	\
			"	cache	%0, %1				\n"	\
			"	.set	pop					\n"	\
			:									\
			: "i" (op), "R" (*(unsigned char *)(addr)))


extern void flush_cache_all (void);
extern void flush_dcache (void);
extern void flush_icache (void);

extern void clean_cache_all (UINT32 addr, UINT32 size);
extern void clean_dcache (UINT32 addr, UINT32 size);
extern void clean_icache (UINT32 addr, UINT32 size);

extern void clean_dcache_nowrite (UINT32 addr, UINT32 size);
extern void clean_icache_nowrite (UINT32 addr, UINT32 size);

extern void clean_dcache_indexed (UINT32 addr, UINT32 size);
extern void clean_icache_indexed (UINT32 addr, UINT32 size);

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if defined(CONFIG_TITANIA2)
	*(volatile unsigned long*)(REG_WDT_BASE)		= 0x5678;	//WDT_KEY
	*(volatile unsigned long*)(REG_WDT_BASE + 4)	= 0xFFFE;	//WDT_SEL
	*(volatile unsigned long*)(REG_WDT_BASE + 12)	= 0x6;		//WDT_CTRL
	*(volatile unsigned long*)(REG_WDT_BASE + 8)	= 0x10;		//WDT_INT_SEL

	while(1);
#elif defined(CONFIG_TITANIA3_FPGA) ||  defined(CONFIG_GP2_DEMO1)
	unsigned int nSecond = 1;
	unsigned int nCount = nSecond * 12000000;

	*(volatile unsigned long*)(REG_WDT_BASE + 0x10)   = nCount & 0xFFFF;           // WDT_PERIOD_L
	*(volatile unsigned long*)(REG_WDT_BASE + 0x14)   = (nCount >> 16) & 0xFFFF;   // WDT_PERIOD_H

	while(1);
#else
	void (*f)(void) = (void *)(0xbfc00000);
	f();

	fprintf(stderr, "*** reset failed ***\n");
#endif

	return 0;
}

void Chip_Flush_Memory(void)
{
	static unsigned char u8_4Lines[64];
	volatile unsigned char *pu8;
	volatile unsigned char tmp;

	// Transfer the memory to noncache memory
	pu8 = ((unsigned char *)(((unsigned int)u8_4Lines) | 0xa0000000));

	// Flush the data from pipe and buffer in MIU
	pu8[0] = pu8[16] = pu8[32] = pu8[48] = 1;

	// Flush the data in the EC bridge buffer
	asm volatile (
			"sync;"
	);

	// Final read back
	tmp = pu8[48];
}

void Chip_Read_Memory(void)
{
	volatile unsigned int *pu8;
	volatile unsigned int t ;

	// Transfer the memory to noncache memory
	pu8 = ((volatile unsigned int *)0xA0380000);
	t = pu8[0] ;
	t = pu8[64] ;
}

void flush_cache (ulong start_addr, ulong size)
{
#if 1 //dhjung LGE
	#if 1 //Hit is OK, Index is NOT_OK;
	clean_cache_all(start_addr, size);
	#else
	clean_dcache_indexed(start_addr, size);
	clean_icache_indexed(start_addr, size);
	#endif
#else
	unsigned long lsize = CFG_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (start_addr + size - 1) & ~(lsize - 1);

	while (1) {
		cache_op(Hit_Writeback_Inv_D, addr);
		cache_op(Hit_Invalidate_I, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
#endif
}

void write_one_tlb( int index, u32 pagemask, u32 hi, u32 low0, u32 low1 )
{
	write_32bit_cp0_register(CP0_ENTRYLO0, low0);
	write_32bit_cp0_register(CP0_PAGEMASK, pagemask);
	write_32bit_cp0_register(CP0_ENTRYLO1, low1);
	write_32bit_cp0_register(CP0_ENTRYHI, hi);
	write_32bit_cp0_register(CP0_INDEX, index);
	tlb_write_indexed();
}
