/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 1999,2000 MIPS Technologies, Inc.  All rights reserved.
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
 * PROM library functions for acquiring/using memory descriptors given to
 * us from the YAMON.
 */
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/pfn.h>
#include <linux/string.h>

#include <asm/bootinfo.h>
#include <asm/page.h>
#include <asm/sections.h>

#include <asm/mips-boards/prom.h>
#if defined(CONFIG_Titania3)
#include "../titania3/board/Board.h"
#include "../titania3/chip_setup.h"
#endif

//dhjung LGE
unsigned long baudrate;
unsigned int sys_memsize;

extern char *prom_get_arg(char *cmd);
extern unsigned long prom_get_argvalue(char *arg);

//dhjung LGE
extern void MDrv_SYS_MMAP_Dump(void);

unsigned long linux_miu1_start_offset;
void __init prom_meminit(void)
{
	//dhjung LGE
	unsigned int	memsize = 0;
	unsigned int	xipfs_size = 0;
	unsigned int	isbt = 0, isdvr = 0;
	unsigned int	miu1_dummy_addr = 0, miu1_dummy_len = 0;
	unsigned int	linux_2nd_mem_addr = 0, linux_2nd_mem_len = 0;
	unsigned int	linux_aux_mem_addr = 0, linux_aux_mem_len = 0;
	unsigned int	linux_high_mem_start = 0, linux_high_mem_len = 0;

	xipfs_size		= prom_get_argvalue("xipfs=");
	memsize			= prom_get_argvalue("memsize=");
	isbt			= prom_get_argvalue("bt=");
	isdvr			= prom_get_argvalue("dvr=");
	baudrate		= prom_get_argvalue("console=ttyS0,");

	linux_2nd_mem_addr	= isdvr ? LINUX_2ND_MEM_ADDR : PVR_DOWNLOAD_BUFFER_ADR;

	if (!isbt && !isdvr) { 			//bt(X), dvr(X)
		miu1_dummy_addr		= BT_POOL_ADR;
	} else if (isbt && !isdvr) { 	//bt(O), dvr(X)
		miu1_dummy_addr		= APVR_BUF_ADR;
	} else if (!isbt && isdvr) { 	//bt(X), dvr(O)
		miu1_dummy_addr		= MIU1_DUMMY_ADR;
		linux_aux_mem_addr 	= BT_POOL_ADR;
		linux_aux_mem_len 	= BT_POOL_LEN;
	} else {						//bt(O), dvr(O)
		miu1_dummy_addr		= MIU1_DUMMY_ADR;
	}

	linux_2nd_mem_len 		= (memsize<<20) - linux_2nd_mem_addr;
	miu1_dummy_len			= MIU1_MEM_BASE_ADR + (memsize<<20) - miu1_dummy_addr;

	if (linux_aux_mem_len)
		linux_miu1_start_offset	= linux_aux_mem_addr - MIU1_MEM_BASE_ADR;
	else
		linux_miu1_start_offset	= miu1_dummy_addr - MIU1_MEM_BASE_ADR;

	linux_high_mem_start	= miu1_dummy_addr - MIU1_MEM_BASE_ADR;
	linux_high_mem_len     	= miu1_dummy_len;

	if (memsize == 128) {
		printk(KERN_DEBUG "DDR Size is 256MB\n");
		sys_memsize = E_SYS_MMAP_256MB;
	} else if (memsize == 256) {
		printk(KERN_DEBUG "DDR Size is 512MB\n");
		sys_memsize = E_SYS_MMAP_512MB;
	}

	//dhjung LGE
	add_memory_region(0, CPHYSADDR(PFN_ALIGN(&_end)), BOOT_MEM_RESERVED);
	add_memory_region(CPHYSADDR(PFN_ALIGN(&_end)), LINUX_MEM_LEN-CPHYSADDR(PFN_ALIGN(&_end)), BOOT_MEM_RAM);
	#if 0 //dhjung LGE
	add_memory_region(linux_2nd_mem_addr, linux_2nd_mem_len-(xipfs_size<<20), BOOT_MEM_RAM);
#else
	add_memory_region(linux_2nd_mem_addr, linux_2nd_mem_len-(xipfs_size<<20), BOOT_MEM_RAM);
	if (linux_aux_mem_len)
		add_memory_region(0x40000000 + (linux_aux_mem_addr - MIU1_MEM_BASE_ADR), linux_aux_mem_len, BOOT_MEM_RAM);
	add_memory_region(0x40000000 + linux_high_mem_start, linux_high_mem_len, BOOT_MEM_RAM);
#endif

//	MDrv_SYS_MMAP_Dump();
}

void __init prom_free_prom_memory(void)
{
	unsigned long addr;
	int i;

	for (i = 0; i < boot_mem_map.nr_map; i++) {
		if (boot_mem_map.map[i].type != BOOT_MEM_ROM_DATA)
			continue;

		addr = boot_mem_map.map[i].addr;
		free_init_pages("prom memory",
				addr, addr + boot_mem_map.map[i].size);
	}
}
