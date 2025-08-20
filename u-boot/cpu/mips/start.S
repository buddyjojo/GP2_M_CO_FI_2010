/*
 *  Startup Code for MIPS32 CPU-core
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
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
 * ----------------------------------------------
 */

/** @file start.S
 *
 *  @author     dhjung77@lge.com
 *  @version    0.1
 *  @date       2009.06.13
 *  @note
 *  @see
 */

#include <config.h>
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>

//#define TEST
//#define USE_KSEG1
//#define USE_CACHE_LOCK

#ifdef TEST
#define RVECENT(f,n)		\
	b 		f; 				\
	nop
#define XVECENT(f,bev)		\
	b 		f;				\
	li		k0,		bev
#endif

.macro SSSW reg, value
	lui     k0, \reg >> 16
	sw      \value, (\reg & 0xFFFF)(k0)
.endm

.macro PUTC char
	lui     k1, 0
	ori	    k1, k1, \char
	SSSW    0xBF201300, k1
.endm

	.set	noreorder

	.globl	_start
	.text
_start:
#include "magic.S"

#ifdef TEST
	RVECENT(reset,0)			/* U-boot entry point 	*/
	RVECENT(reset,1)			/* software reboot 		*/
	RVECENT(romReserved,2)
	RVECENT(romReserved,3)
	RVECENT(romReserved,4)
	RVECENT(romReserved,5)
	RVECENT(romReserved,6)
	RVECENT(romReserved,7)
	RVECENT(romReserved,8)
	RVECENT(romReserved,9)
	RVECENT(romReserved,10)
	RVECENT(romReserved,11)
	RVECENT(romReserved,12)
	RVECENT(romReserved,13)
	RVECENT(romReserved,14)
	RVECENT(romReserved,15)
	RVECENT(romReserved,16)
	RVECENT(romReserved,17)
	RVECENT(romReserved,18)
	RVECENT(romReserved,19)
	RVECENT(romReserved,20)
	RVECENT(romReserved,21)
	RVECENT(romReserved,22)
	RVECENT(romReserved,23)
	RVECENT(romReserved,24)
	RVECENT(romReserved,25)
	RVECENT(romReserved,26)
	RVECENT(romReserved,27)
	RVECENT(romReserved,28)
	RVECENT(romReserved,29)
	RVECENT(romReserved,30)
	RVECENT(romReserved,31)
	RVECENT(romReserved,32)
	RVECENT(romReserved,33)
	RVECENT(romReserved,34)
	RVECENT(romReserved,35)
	RVECENT(romReserved,36)
	RVECENT(romReserved,37)
	RVECENT(romReserved,38)
	RVECENT(romReserved,39)
	RVECENT(romReserved,40)
	RVECENT(romReserved,41)
	RVECENT(romReserved,42)
	RVECENT(romReserved,43)
	RVECENT(romReserved,44)
	RVECENT(romReserved,45)
	RVECENT(romReserved,46)
	RVECENT(romReserved,47)
	RVECENT(romReserved,48)
	RVECENT(romReserved,49)
	RVECENT(romReserved,50)
	RVECENT(romReserved,51)
	RVECENT(romReserved,52)
	RVECENT(romReserved,53)
	RVECENT(romReserved,54)
	RVECENT(romReserved,55)
	RVECENT(romReserved,56)
	RVECENT(romReserved,57)
	RVECENT(romReserved,58)
	RVECENT(romReserved,59)
	RVECENT(romReserved,60)
	RVECENT(romReserved,61)
	RVECENT(romReserved,62)
	RVECENT(romReserved,63)
	XVECENT(romExcHandle,0x200)	/* bfc00200: R4000 tlbmiss vector */
	RVECENT(romReserved,65)
	RVECENT(romReserved,66)
	RVECENT(romReserved,67)
	RVECENT(romReserved,68)
	RVECENT(romReserved,69)
	RVECENT(romReserved,70)
	RVECENT(romReserved,71)
	RVECENT(romReserved,72)
	RVECENT(romReserved,73)
	RVECENT(romReserved,74)
	RVECENT(romReserved,75)
	RVECENT(romReserved,76)
	RVECENT(romReserved,77)
	RVECENT(romReserved,78)
	RVECENT(romReserved,79)
	XVECENT(romExcHandle,0x280)	/* bfc00280: R4000 xtlbmiss vector */
	RVECENT(romReserved,81)
	RVECENT(romReserved,82)
	RVECENT(romReserved,83)
	RVECENT(romReserved,84)
	RVECENT(romReserved,85)
	RVECENT(romReserved,86)
	RVECENT(romReserved,87)
	RVECENT(romReserved,88)
	RVECENT(romReserved,89)
	RVECENT(romReserved,90)
	RVECENT(romReserved,91)
	RVECENT(romReserved,92)
	RVECENT(romReserved,93)
	RVECENT(romReserved,94)
	RVECENT(romReserved,95)
	XVECENT(romExcHandle,0x300)	/* bfc00300: R4000 cache vector */
	RVECENT(romReserved,97)
	RVECENT(romReserved,98)
	RVECENT(romReserved,99)
	RVECENT(romReserved,100)
	RVECENT(romReserved,101)
	RVECENT(romReserved,102)
	RVECENT(romReserved,103)
	RVECENT(romReserved,104)
	RVECENT(romReserved,105)
	RVECENT(romReserved,106)
	RVECENT(romReserved,107)
	RVECENT(romReserved,108)
	RVECENT(romReserved,109)
	RVECENT(romReserved,110)
	RVECENT(romReserved,111)
	XVECENT(romExcHandle,0x380)	/* bfc00380: R4000 general vector */
	RVECENT(romReserved,113)
	RVECENT(romReserved,114)
	RVECENT(romReserved,115)
	RVECENT(romReserved,116)
	RVECENT(romReserved,116)
	RVECENT(romReserved,118)
	RVECENT(romReserved,119)
	RVECENT(romReserved,120)
	RVECENT(romReserved,121)
	RVECENT(romReserved,122)
	RVECENT(romReserved,123)
	RVECENT(romReserved,124)
	RVECENT(romReserved,125)
	RVECENT(romReserved,126)
	RVECENT(romReserved,127)

	/* We hope there are no more reserved vectors!
	 * 128 * 8 == 1024 == 0x400
	 * so this is address R_VEC+0x400 == 0xbfc00400
	 */
	.align 4
reset:

	/* Force KSEG1 in case control is passed here from KSEG0
	 */
#ifdef USE_KSEG1
	bal		1f
	li		t0,		K1_MEM_BASE
1:	or		t0,		ra
	addu	t0,		16
	jr		t0
	nop
#endif

	/* Init Watch.
	 */
	mtc0	zero,	CP0_WATCHLO
	mtc0	zero,	CP0_WATCHHI

	/* STATUS register.
	 */
	mfc0	k0, 	CP0_STATUS
	li		k1, 	~(ST0_IE | ST0_KSU)
	and		k0, 	k1
	or		k0, 	KSU_KERNEL
	mtc0	k0, 	CP0_STATUS

	/* CAUSE register.
	 */
	mtc0	zero, 	CP0_CAUSE

	/* Init Timer.
	 */
	mtc0	zero, 	CP0_COUNT
	mtc0	zero, 	CP0_COMPARE
#endif

	/* Chip Configurations.
	 */
	bal		1f
	nop
1:	move	t0, 	ra
	li		t1, 	0xfff00000
	and		t0, 	t1
	bne		t0,  	RESET_VECTOR, 	determine_src
	nop

	bal	chip_conf
	nop

	/* Determine Source Address.
	 */
determine_src:
	bal		1f
	nop
1:	move	t0, 	ra
	li		t1, 	0xfff00000
	and		t0, 	t1				/* Source Address in ROM or RAM 			*/

	/* Determine GOT pointer and Destination address and Copy size.
	*/
reloc_start:
	bal     1f
	nop
	.word   _GLOBAL_OFFSET_TABLE_
	.word	TEXT_BASE
	.word	uboot_end_data
1:	lw      t2, 	0(ra)
	move	gp, 	t2
	lw		t1, 	4(ra)			/* Destination address  	  				*/
#ifdef USE_KSEG1
	or		t1, 	K1_MEM_BASE		/* Destination address  	  				*/
#endif
	move	t9, 	t1				/* Backup Destination address			 	*/
	lw		t2, 	8(ra)			/* Destination end address					*/
#ifdef USE_KSEG1
	or		t2, 	K1_MEM_BASE		/* Destination end address					*/
#endif
	sub		t2, 	t1				/* Copy Size 	  	        				*/

	/* Backup Source address, Destination address, Copy size
	 * for verify code.
	 */
	move	t6, 	t0
	move	t7, 	t1
	move	t8, 	t2

	beq		t0, 	t1, 	in_ram
	nop

	/* Source to Destination Copy Loop.
	 */
copy_loop:
	lw		t3, 	0(t0)
	sw		t3, 	0(t1)
	addi	t0, 	4
	addi	t1, 	4
	sub		t2, 	4
	bnez	t2, 	copy_loop
	nop

#ifdef TEST
	/* Copy Verify Loop.
	 */
verify_loop:
	lw		t4, 	0(t6)
	lw		t5, 	0(t7)
	bne		t4,		t5,		verify_loop
	sw		t4,		0(t7)

	bne		t4, 	t5, 	romReserved
	nop

	addi	t6, 	4
	addi	t7, 	4
	sub		t8, 	4
	bnez	t8, 	verify_loop
	nop
#endif

	/* Start running on ram.
	 */
	addi	t9, 	in_ram - _start
	jr		t9						/* Should address jump 						*/
	nop

	/* BSS Initialization.
	 */
in_ram:
	bal     1f
	nop
	.word   uboot_end_data
	.word   uboot_end
1:	lw      t0, 	0(ra)
#ifdef USE_KSEG1
	or		t0, 	K1_MEM_BASE
#endif
	lw      t1, 	4(ra)
#ifdef USE_KSEG1
	or		t1, 	K1_MEM_BASE
#endif
#if 0
	sub		t1, 	t0				/* BSS Size                  				*/

clear_loop:
	sw		zero, 	0(t0)
	addi	t0, 	4
	sub		t1, 	4
	bnez	t1, 	clear_loop
	nop
#else
	sub		t0,		4
1:	addi	t0,		4
	bltl	t0,		t1,		1b
	sw		zero,	0(t0)
#endif

#ifdef TEST
	/* Initialize caches.
	 */
	bal		mips_cache_init
	nop

	/* CONFIG0 register.
	 */
	li		t0, 	CONF_CM_UNCACHED
	mtc0	t0, 	CP0_CONFIG
#else
	/* Initialize caches.
	 */
//	bal		mips_cache_init
//	nop

	/* ... and enable them.
	 */
//	li		t0, 	CONF_CM_CACHABLE_NONCOHERENT
//	mtc0	t0, 	CP0_CONFIG
#endif

	/* Stack Setup using real linker address.
	 */
	bal     1f
	nop
	.word   TEXT_BASE
	.word   CFG_MALLOC_LEN
	.word   CFG_GBL_DATA_SIZE
	.word   CFG_SDRAM_BASE
1:	lw      t0, 	0(ra)
	lw		t1, 	4(ra)
	lw		t2, 	8(ra)
	lw		t3, 	12(ra)
	sub		t0, 	t1
	sub 	t0, 	t2				/* Real Stack Pointer					*/
#ifdef USE_CACHE_LOCK
	sub 	t4, 	t0,		t3		/* Stack Pointer Offset 				*/

	/* Cache lock for stack
	 */
	move	a0,		t4
	la      t9,		mips_cache_lock
	jalr    t9
	nop
#endif

	/* Set up stack.
	 */
	move	sp, 	t0

	/* Jump To C Level Code having real linker address.
	 */
	bal     1f
	nop
	.word   board_init
1:	lw      t9, 	0(ra)
	jr		t9
	nop

romReserved:
	b		romReserved
	nop

romExcHandle:
	b		romExcHandle
	nop


/***************************************************************************************
 *
 * Chip configuration
 */
	.set noreorder
chip_conf:
	nop
	nop

	jr	ra
	nop

/*******************************************************************************
*
* mips_cache_init - mips cache initialize
*
* RETURNS: N/A
*
*/
	.set	reorder
mips_cache_init:
	li		t2, 	MIPS_ICACHE_SIZE
	li		t3, 	MIPS_DCACHE_SIZE
	li		t4, 	MIPS_CACHE_LINE_SIZE
	move	t5, 	t4
	li		v0, 	MIPS_MAX_CACHE_SIZE

	li		a0, 	K1_MEM_BASE
	addu	a1, 	a0, 	v0

	/* Now clear that much memory starting from zero.
	 */
1:	sw		zero,  	0(a0)
	sw		zero,  	4(a0)
	sw		zero,  	8(a0)
	sw		zero, 	12(a0)
	sw		zero, 	16(a0)
	sw		zero, 	20(a0)
	sw		zero, 	24(a0)
	sw		zero, 	28(a0)
	addu	a0, 	32
	bltu	a0, 	a1, 	1b

	/* Set invalid tag. - 16KB Cache Size in rukh20
	 */
	mtc0	zero, 	CP0_TAGLO

   /*
    * The caches are probably in an indeterminate state,
    * so we force good parity into them by doing an
    * invalidate, load/fill, invalidate for each line.
    */

	/* Assume bottom of RAM will generate good parity for the cache.
	 */

	/*
	 * I-Cache Initialize
	 */
	li		a0, 	K0_MEM_BASE
	move	a2, 	t2		# icacheSize
	move	a3, 	t4		# icacheLineSize
	move	a1, 	a2

	addu	a1,		a0

	_align(a0, a1, a3)

	.set  noreorder
1:	cache	Index_Store_Tag_I,	0(a0)
	nop
	cache	Index_Store_Tag_I,	1(a0)
	nop
	cache	Index_Store_Tag_I,	2(a0)
	nop
	cache	Index_Store_Tag_I,	3(a0)
	nop

	cache	Fill,	0(a0)
	nop
	cache	Fill,	1(a0)
	nop
	cache	Fill,	2(a0)
	nop
	cache	Fill,	3(a0)
	nop

	cache	Index_Store_Tag_I,	0(a0)
	nop
	cache	Index_Store_Tag_I,	1(a0)
	nop
	cache	Index_Store_Tag_I,	2(a0)
	nop
	cache	Index_Store_Tag_I,	3(a0)
	nop

	bne     a0, 	a1, 	1b
	add     a0, 	a3
	.set  reorder

	/* 1: Initialise D-Cache Tags
	 */
	li		a0, 	K0_MEM_BASE
	move	a2, 	t3		# dcacheSize
	move	a3, 	t5		# dcacheLineSize
	move	a1, 	a2

	addu	a1,		a0

	_align(a0, a1, a3)

   .set  noreorder
1:	cache	Index_Store_Tag_D,	0(a0)
	nop
	cache	Index_Store_Tag_D,	1(a0)
	nop
	cache	Index_Store_Tag_D,	2(a0)
	nop
	cache	Index_Store_Tag_D,	3(a0)
	nop

	bne     a0,		a1,		1b
	add     a0,		a3
   .set  reorder

	/* 2: Fill D-Cache
	 */
	li		a0, 	K0_MEM_BASE
	move	a2, 	t3		# dcacheSize
	move	a3, 	t5		# dcacheLineSize
	move	a1, 	a2

	addu	a1,		a0

	_align(a0, a1, a3)

   .set  noreorder
1:	cache	Fill_D,	0(a0)
	nop
	cache	Fill_D,	1(a0)
	nop
	cache	Fill_D,	2(a0)
	nop
	cache	Fill_D,	3(a0)
	nop

	bne     a0,		a1,		1b
	add     a0,		a3
   .set  reorder

	/* 3: Clear D-Cache Tags
	 */
	li		a0, 	K0_MEM_BASE
	move	a2, 	t3		# dcacheSize
	move	a3, 	t5		# dcacheLineSize
	move	a1, 	a2

	addu	a1,		a0

	_align(a0, a1, a3)

   .set  noreorder
1:	cache	Index_Store_Tag_D,	0(a0)
	nop
	cache	Index_Store_Tag_D,	1(a0)
	nop
	cache	Index_Store_Tag_D,	2(a0)
	nop
	cache	Index_Store_Tag_D,	3(a0)
	nop

	bne     a0,		a1,		1b
	add     a0,		a3
   .set  reorder

	jr		ra
