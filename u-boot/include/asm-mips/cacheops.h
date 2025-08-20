/*
 * Cache operations for the cache instruction.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * (C) Copyright 1996, 1997 by Ralf Baechle
 */
#ifndef	__ASM_MIPS_CACHEOPS_H
#define	__ASM_MIPS_CACHEOPS_H

/*
 * Cache Operations
 */
#define Index_Invalidate_I    		0x00
#define Index_Writeback_Inv_D 		0x01
#define Index_Invalidate_SI   		0x02
#define Index_Writeback_Inv_SD		0x03
#define Index_Load_Tag_I      		0x04
#define Index_Load_Tag_D      		0x05
#define Index_Load_Tag_SI     		0x06
#define Index_Load_Tag_SD     		0x07
#define Index_Store_Tag_I     		0x08
#define Index_Store_Tag_D     		0x09
#define Index_Store_Tag_SI    		0x0A
#define Index_Store_Tag_SD    		0x0B
#define Index_CacheFlush_I    		0x0C
#define Index_CacheFlush_D    		0x0D
#define Create_Dirty_Excl_D   		0x0D
#define Index_CacheFlush_SI   		0x0E
#define Index_CacheFlush_SD   		0x0F
#define Create_Dirty_Excl_SD  		0x0F
#define Hit_Invalidate_I      		0x10
#define Hit_Invalidate_D      		0x11
#define Hit_Invalidate_SI     		0x12
#define Hit_Invalidate_SD     		0x13
#define Fill                  		0x14
#define Hit_Writeback_Inv_D   		0x15
/* 0x16 is unused */
#define Hit_Writeback_Inv_SD  		0x17
#define Hit_Writeback_I       		0x18
#define Hit_Writeback_D       		0x19
/* 0x1A is unused */
#define Hit_Writeback_SD      		0x1B
/* 0x1C is unused */
#define Fill_D                		0x1D
#define Hit_Set_Virtual_SI    		0x1E
#define Hit_Set_Virtual_SD    		0x1F

/*
 * mips memory segment definitions
 */
#define K0_MEM_BASE					0x80000000
#define K1_MEM_BASE					0xA0000000
#define KS_MEM_BASE					0xC0000000
#define K3_MEM_BASE					0xE0000000

/*
 * Options for cacheflush system call
 */
#define	ICACHE						(1<<0)			/* flush instruction cache        */
#define	DCACHE						(1<<1)			/* writeback and flush data cache */
#define	BCACHE						(ICACHE|DCACHE)	/* flush both caches              */

/*
 * Caching modes for the cachectl(2) call
 *
 * cachectl(2) is currently not supported and returns ENOSYS.
 */
#define CACHEABLE					0	/* make pages cacheable */
#define UNCACHEABLE					1	/* make pages uncacheable */

/*
 * MIPS cache operation
 */
#define MIPS_CACHE_LINE_SIZE		32
#define MIPS_ICACHE_SIZE			(32*1024)
#define MIPS_DCACHE_SIZE			(32*1024)

#define MIPS_MAX_CACHE_SIZE			(0x8000)
#define CACHE_LOCK_SIZE				(MIPS_DCACHE_SIZE)

#define _align(minaddr, maxaddr, linesize)	\
   .set noat ; 								\
   subu  AT,linesize,1 ;					\
   not   AT ;								\
   and   minaddr,AT ;						\
   addu  maxaddr,-1 ;						\
   and   maxaddr,AT ;						\
   .set at

#endif	/* __ASM_MIPS_CACHEOPS_H */
