/******************************************************************************

	LGE. DTV RESEARCH LABORATORY
	COPYRIGHT(c) LGE CO.,LTD. 2006. SEOUL, KOREA.
	All rights are reserved.

******************************************************************************/

/** @file cache.S
 *
 *  @author     dhjung77@lge.com
 *  @version    0.1
 *  @date       2009.06.13
 *  @note
 *  @see
 */

#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>

	.set	reorder
/*******************************************************************************
*
* dcache_status - get cache status
*
* RETURNS: 0 - cache disabled; 1 - cache enabled
*
*/
	.globl	dcache_status
	.ent	dcache_status
dcache_status:

	mfc0	v0,		CP0_CONFIG
	andi	v0,		v0,		1

	j		ra

	.end  dcache_status

/*******************************************************************************
*
* dcache_disable - disable cache
*
* RETURNS: N/A
*
*/
	.globl	dcache_disable
	.ent	dcache_disable
dcache_disable:

	li		a0, 	K0_MEM_BASE
	li		a1, 	MIPS_DCACHE_SIZE
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* make max address */
	addu	a1,		a0

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Index_Writeback_Inv_D, 0(a0)
	nop
	cache	Index_Writeback_Inv_D, 1(a0)
	nop
	cache	Index_Writeback_Inv_D, 2(a0)
	nop
	cache	Index_Writeback_Inv_D, 3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	mfc0	t0,		CP0_CONFIG
	li		t1,		-8
	and		t0, 	t0,		t1
	ori		t0, 	t0,		CONF_CM_UNCACHED
	mtc0    t0, 	CP0_CONFIG

	j	ra

	.end  dcache_disable

/*******************************************************************************
*
* dcache_enable - enable cache
*
* RETURNS: N/A
*
*/
    .global dcache_enable
    .ent    dcache_enable
dcache_enable:

	mfc0	t0,		CP0_CONFIG
	ori		t0,		CONF_CM_CMASK
	xori	t0,		CONF_CM_CMASK
	ori		t0,		CONF_CM_CACHABLE_NONCOHERENT
	mtc0	t0,		CP0_CONFIG

	jr	ra

    .end dcache_enable

/*******************************************************************************
*
* flush_cache_all - Writeback and invalidate all caches
*
* PARAMS : void
* RETURNS: N/A
*
*/
	.globl	flush_cache_all
	.ent	flush_cache_all
flush_cache_all:
	li		a0, 	K0_MEM_BASE
	li		a1, 	MIPS_DCACHE_SIZE
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* make max address */
	addu	a1,		a0

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Index_Writeback_Inv_D,	0(a0)
	nop
	cache	Index_Writeback_Inv_D,	1(a0)
	nop
	cache	Index_Writeback_Inv_D,	2(a0)
	nop
	cache	Index_Writeback_Inv_D,	3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	li		a0, 	K0_MEM_BASE
	li		a1, 	MIPS_ICACHE_SIZE
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* make max address */
	addu	a1,		a0

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
2:	cache	Index_Invalidate_I,		0(a0)
	nop
	cache	Index_Invalidate_I,		1(a0)
	nop
	cache	Index_Invalidate_I,		2(a0)
	nop
	cache	Index_Invalidate_I,		3(a0)
	nop

	bne		a0, 	a1, 	2b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	flush_cache_all

/*******************************************************************************
*
* flush_dcache - Writeback and invalidate data cache only
*
* PARAMS : void
* RETURNS: N/A
*
*/
	.globl	flush_dcache
	.ent	flush_dcache
flush_dcache:
	li		a0, 	K0_MEM_BASE
	li		a1, 	MIPS_DCACHE_SIZE
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* make max address */
	addu	a1,		a0

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Index_Writeback_Inv_D,	0(a0)
	nop
	cache	Index_Writeback_Inv_D,	1(a0)
	nop
	cache	Index_Writeback_Inv_D,	2(a0)
	nop
	cache	Index_Writeback_Inv_D,	3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	flush_dcache

/*******************************************************************************
*
* flush_icache - Writeback and invalidate instruction cache only
*
* PARAMS : void
* RETURNS: N/A
*
*/
	.globl	flush_icache
	.ent	flush_icache
flush_icache:
	li		a0, 	K0_MEM_BASE
	li		a1, 	MIPS_ICACHE_SIZE
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* make max address */
	addu	a1,		a0

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Index_Invalidate_I,		0(a0)
	nop
	cache	Index_Invalidate_I,		1(a0)
	nop
	cache	Index_Invalidate_I,		2(a0)
	nop
	cache	Index_Invalidate_I,		3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	flush_icache

/*******************************************************************************
*
* clean_cache_all - Writeback and invalidate address range in all caches
*
* PARAMS : a0(address) a1(size)
* RETURNS: N/A
*
*/
	.globl	clean_cache_all
	.ent	clean_cache_all
clean_cache_all:
	/* make max address */
	addu	a1, 	a0
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* address space alignment */
	_align(a0, a1, a2)

	/* address save */
	move    t0,     a0
	move    t1,     a1

	.set	noreorder
1:	cache	Hit_Writeback_Inv_D,	0(a0)
	nop
	cache	Hit_Writeback_Inv_D,	1(a0)
	nop
	cache	Hit_Writeback_Inv_D,	2(a0)
	nop
	cache	Hit_Writeback_Inv_D,	3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	/* address save */
	move    a0,     t0
	move    a1,     t1

	.set	noreorder
2:	cache	Hit_Invalidate_I,		0(a0)
	nop
	cache	Hit_Invalidate_I,		1(a0)
	nop
	cache	Hit_Invalidate_I,		2(a0)
	nop
	cache	Hit_Invalidate_I,		3(a0)
	nop

	bne		a0, 	a1, 	2b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	clean_cache_all

/*******************************************************************************
*
* clean_dcache - Writeback and invalidate address range in data cache
*
* PARAMS : a0(address) a1(size)
* RETURNS: N/A
*
*/
	.globl	clean_dcache
	.ent	clean_dcache
clean_dcache:
	/* make max address */
	addu	a1, 	a0
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Hit_Writeback_Inv_D,	0(a0)
	nop
	cache	Hit_Writeback_Inv_D,	1(a0)
	nop
	cache	Hit_Writeback_Inv_D,	2(a0)
	nop
	cache	Hit_Writeback_Inv_D,	3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	clean_dcache

/*******************************************************************************
*
* clean_icache - Writeback and invalidate address range in instruction cache
*
* PARAMS : a0(address) a1(size)
* RETURNS: N/A
*
*/
	.globl	clean_icache
	.ent	clean_icache
clean_icache:
	/* make max address */
	addu	a1, 	a0
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Hit_Invalidate_I,		0(a0)
	nop
	cache	Hit_Invalidate_I,		1(a0)
	nop
	cache	Hit_Invalidate_I,		2(a0)
	nop
	cache	Hit_Invalidate_I,		3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	clean_icache

/*******************************************************************************
*
* clean_dcache_nowrite - Invalidate (but don't writeback) address range in data cache
*
* PARAMS : a0(address) a1(size)
* RETURNS: N/A
*
*/
	.globl	clean_dcache_nowrite
	.ent	clean_dcache_nowrite
clean_dcache_nowrite:
	/* make max address */
	addu	a1, 	a0
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Hit_Invalidate_D,		0(a0)
	nop
	cache	Hit_Invalidate_D,		1(a0)
	nop
	cache	Hit_Invalidate_D,		2(a0)
	nop
	cache	Hit_Invalidate_D,		3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	clean_dcache_nowrite

/*******************************************************************************
*
* clean_icache_nowrite - Invalidate (but don't writeback) address range in instruction cache
*
* PARAMS : a0(address) a1(size)
* RETURNS: N/A
*
*/
	.globl	clean_icache_nowrite
	.ent	clean_icache_nowrite
clean_icache_nowrite:
	/* make max address */
	addu	a1, 	a0
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Hit_Invalidate_I,		0(a0)
	nop
	cache	Hit_Invalidate_I,		1(a0)
	nop
	cache	Hit_Invalidate_I,		2(a0)
	nop
	cache	Hit_Invalidate_I,		3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	clean_icache_nowrite

/*******************************************************************************
*
* clean_dcache_indexed - Writeback and invalidate indexed range in primary data cache
*
* PARAMS : a0(address) a1(size)
* RETURNS: N/A
*
*/
	.globl	clean_dcache_indexed
	.ent	clean_dcache_indexed
clean_dcache_indexed:
	/* make max address */
	addu	a1, 	a0
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Index_Writeback_Inv_D,	0(a0)
	nop
	cache	Index_Writeback_Inv_D,	1(a0)
	nop
	cache	Index_Writeback_Inv_D,	2(a0)
	nop
	cache	Index_Writeback_Inv_D,	3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	clean_dcache_indexed

/*******************************************************************************
*
* clean_icache_indexed - Writeback and invalidate indexed range in primary instruction cache
*
* PARAMS : a0(address) a1(size)
* RETURNS: N/A
*
*/
	.globl	clean_icache_indexed
	.ent	clean_icache_indexed
clean_icache_indexed:
	/* make max address */
	addu	a1, 	a0
	li		a2, 	MIPS_CACHE_LINE_SIZE

	/* address space alignment */
	_align(a0, a1, a2)

	.set	noreorder
1:	cache	Index_Invalidate_I,		0(a0)
	nop
	cache	Index_Invalidate_I,		1(a0)
	nop
	cache	Index_Invalidate_I,		2(a0)
	nop
	cache	Index_Invalidate_I,		3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a2
	.set	reorder

	jr		ra

	.end	clean_icache_indexed

/*******************************************************************************
*
* mips_cache_lock - lock RAM area pointed to by a0 in cache.
*
* RETURNS: N/A
*
*/
	.globl	mips_cache_lock
	.ent	mips_cache_lock
mips_cache_lock:
	li		a1,		K0_MEM_BASE - CACHE_LOCK_SIZE
	addu	a0,		a1
	li		a2,		CACHE_LOCK_SIZE
	li		a3,		MIPS_CACHE_LINE_SIZE
	move	a1,		a2
	addu	a1,		a0

	/* address space alignment */
	_align(a0, a1, a3)

	/* Fill */
	.set	noreorder
1:	cache	Fill_D, 0(a0)
	nop
	cache	Fill_D, 1(a0)
	nop
	cache	Fill_D, 2(a0)
	nop
	cache	Fill_D, 3(a0)
	nop

	bne		a0, 	a1, 	1b
	add		a0, 	a3
	.set reorder

	jr		ra

	.end	mips_cache_lock
