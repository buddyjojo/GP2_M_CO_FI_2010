/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995 Waldorf Electronics
 * Written by Ralf Baechle and Andreas Busse
 * Copyright (C) 1994 - 99, 2003, 06 Ralf Baechle
 * Copyright (C) 1996 Paul M. Antoine
 * Modified for DECStation and hence R3000 support by Paul M. Antoine
 * Further modifications by David S. Miller and Harald Koerfgen
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Kevin Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.  All rights reserved.
 */
#include <linux/init.h>
#include <linux/threads.h>

#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/irqflags.h>
#include <asm/regdef.h>
#include <asm/page.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>

#include <kernel-entry-init.h>

	/*
	 * inputs are the text nasid in t1, data nasid in t2.
	 */
	.macro MAPPED_KERNEL_SETUP_TLB
#ifdef CONFIG_MAPPED_KERNEL
	/*
	 * This needs to read the nasid - assume 0 for now.
	 * Drop in 0xffffffffc0000000 in tlbhi, 0+VG in tlblo_0,
	 * 0+DVG in tlblo_1.
	 */
	dli	t0, 0xffffffffc0000000
	dmtc0	t0, CP0_ENTRYHI
	li	t0, 0x1c000		# Offset of text into node memory
	dsll	t1, NASID_SHFT		# Shift text nasid into place
	dsll	t2, NASID_SHFT		# Same for data nasid
	or	t1, t1, t0		# Physical load address of kernel text
	or	t2, t2, t0		# Physical load address of kernel data
	dsrl	t1, 12			# 4K pfn
	dsrl	t2, 12			# 4K pfn
	dsll	t1, 6			# Get pfn into place
	dsll	t2, 6			# Get pfn into place
	li	t0, ((_PAGE_GLOBAL|_PAGE_VALID| _CACHE_CACHABLE_COW) >> 6)
	or	t0, t0, t1
	mtc0	t0, CP0_ENTRYLO0	# physaddr, VG, cach exlwr
	li	t0, ((_PAGE_GLOBAL|_PAGE_VALID| _PAGE_DIRTY|_CACHE_CACHABLE_COW) >> 6)
	or	t0, t0, t2
	mtc0	t0, CP0_ENTRYLO1	# physaddr, DVG, cach exlwr
	li	t0, 0x1ffe000		# MAPPED_KERN_TLBMASK, TLBPGMASK_16M
	mtc0	t0, CP0_PAGEMASK
	li	t0, 0			# KMAP_INX
	mtc0	t0, CP0_INDEX
	li	t0, 1
	mtc0	t0, CP0_WIRED
	tlbwi
#else
	mtc0	zero, CP0_WIRED
#endif
	.endm

	/*
	 * For the moment disable interrupts, mark the kernel mode and
	 * set ST0_KX so that the CPU does not spit fire when using
	 * 64-bit addresses.  A full initialization of the CPU's status
	 * register is done later in per_cpu_trap_init().
	 */
	.macro	setup_c0_status set clr
	.set	push
#ifdef CONFIG_MIPS_MT_SMTC
	/*
	 * For SMTC, we need to set privilege and disable interrupts only for
	 * the current TC, using the TCStatus register.
	 */
	mfc0	t0, CP0_TCSTATUS
	/* Fortunately CU 0 is in the same place in both registers */
	/* Set TCU0, TMX, TKSU (for later inversion) and IXMT */
	li	t1, ST0_CU0 | 0x08001c00
	or	t0, t1
	/* Clear TKSU, leave IXMT */
	xori	t0, 0x00001800
	mtc0	t0, CP0_TCSTATUS
	_ehb
	/* We need to leave the global IE bit set, but clear EXL...*/
	mfc0	t0, CP0_STATUS
	or	t0, ST0_CU0 | ST0_EXL | ST0_ERL | \set | \clr
	xor	t0, ST0_EXL | ST0_ERL | \clr
	mtc0	t0, CP0_STATUS
#else
	mfc0	t0, CP0_STATUS
	or	t0, ST0_CU0|\set|0x1f|\clr
	xor	t0, 0x1f|\clr
	mtc0	t0, CP0_STATUS
	.set	noreorder
	sll	zero,3				# ehb
#endif
	.set	pop
	.endm

	.macro	setup_c0_status_pri
#ifdef CONFIG_64BIT
	setup_c0_status ST0_KX 0
#else
	setup_c0_status 0 0
#endif
	.endm

	.macro	setup_c0_status_sec
#ifdef CONFIG_64BIT
	setup_c0_status ST0_KX ST0_BEV
#else
	setup_c0_status 0 ST0_BEV
#endif
	.endm

#=================================================================================================
M_DebugCOUNTDM			= (0x1  << 25)

M_StatusCU    			= (0xf  << 28)
M_StatusRP    			= (0x1  << 27)
M_StatusRE    			= (0x1  << 25)
M_StatusBEV   			= (0x1  << 22)
M_StatusTS    			= (0x1  << 21)
M_StatusSR    			= (0x1  << 20)
#Samuel,redefined in YAMON code.# M_StatusNMI         = (0x1  << 19)
M_StatusIM7				= (0x1  << 15)
M_StatusIM2				= (0x1  << 10)
M_StatusIM 				= (0xff << 8)
M_StatusUM 				= (0x1  << 4)
M_StatusERL				= (0x1  << 2)
M_StatusEXL				= (0x1  << 1)
#Samuel,redefined in YAMON code.#M_StatusIE          = (0x1  << 0)

#Samuel,redefined in YAMON code.#M_CauseIV           = (0x1  << 23)
M_CauseWP  				= (0x1  << 22)
M_CauseIP1 				= (0x1  << 9)
M_CauseIP0 				= (0x1  << 8)

M_ConfigK23				= (0x7  << 28)
M_ConfigKU 				= (0x7  << 25)
M_ConfigMT 				= (0x7  << 7)
M_ConfigK0 				= (0x7  << 0)
#Samuel,redefined in YAMON code. #K_CacheAttrCN       = (0x3  << 0) #Cacheable, noncoherent, write-back, write allocate.
S_Config1IS				= 22
M_Config1IS				= (0x7  << S_Config1IS)
S_Config1IL				= 19
M_Config1IL				= (0x7  << S_Config1IL)
S_Config1IA				= 16
M_Config1IA				= (0x7  << S_Config1IA)
S_Config1DS				= 13
M_Config1DS				= (0x7  << S_Config1DS)
S_Config1DL				= 10
M_Config1DL				= (0x7  << S_Config1DL)
S_Config1DA				= 7
M_Config1DA				= (0x7  << S_Config1DA)

INITIAL_STATUS      	= 0x10400000
INITIAL_CONFIG0    		= 0x00000002 #Uncached

Index_Store_Tag_I		= 0x8
Index_Store_Tag_D		= 0x9

A_DSPRAM         		= 0x04000000        # 0x84000000
A_KSEG0BASE      		= 0x80000000

# alan.chen adds start

zero					= $0

S_StatusNMI				= 19
M_StatusNMI				= (0x1 << S_StatusNMI)	/* Denote NMI (R/W) */

S_StatusIE				= 0			/* Enables interrupts (R/W) */
M_StatusIE				= (0x1 << S_StatusIE)

S_StatusKSU				= 3			/* Two-bit current mode (R/W) */
M_StatusKSU				= (0x3 << S_StatusKSU)

S_CauseIV				= 23
M_CauseIV				= (0x1 << S_CauseIV)


CONFIG0_MIPS32_64_MSK	= 0x8000ffff
STATUS_MIPS32_64_MSK 	= 0xfffcffff

S_ConfigMM				= 18     /* 24K specific, merging enable/disable */
M_ConfigMM				= (0x1<<S_ConfigMM)


S_ConfigK23				= 28			/* Kseg2/3 coherency algorithm (FM MMU only) (R/W) */
S_ConfigKU				= 25			/* Kuseg coherency algorithm (FM MMU only) (R/W) */

K_CacheAttrCN    		= 3            /*3=CACHABLE_NONCOHERENT, 2=UNCACHED*/

CONFIG0_MIPS34K  		= ((K_CacheAttrCN<<S_ConfigK23)|(K_CacheAttrCN<<S_ConfigKU)|(M_ConfigMM))

STATUS_MIPS34K	    	= 0

# alan.chen adds end

.macro WREG reg, value
	lui     $8, \reg >> 16
	li      $9, \value
	sw      $9, (\reg & 0xFFFF)($8)
.endm

.macro SSSW reg, value
	lui     $8, \reg >> 16
	sw      \value, (\reg & 0xFFFF)($8)
.endm

.macro PUTC char
    lui     $27, 0
    ori	    $27, $27, \char
    SSSW    0xBF201300, $27
.endm
#=================================================================================================


#ifndef CONFIG_NO_EXCEPT_FILL
	/*
	 * Reserved space for exception handlers.
	 * Necessary for machines which link their kernels at KSEG0.
	 */
	.fill	0x400
#endif

EXPORT(_stext)

#ifdef CONFIG_BOOT_RAW
	/*
	 * Give us a fighting chance of running if execution beings at the
	 * kernel load address.  This is needed because this platform does
	 * not have a ELF loader yet.
	 */
FEXPORT(__kernel_entry)
	j	kernel_entry
#endif

	__REF

NESTED(kernel_entry, 16, sp)			# kernel entry point


#=================================================================================================
#if (0)
	# Setup cause register (clear cause code), sync from YAMON, AlanChen
	li		$26, M_CauseIV
	mtc0	$26, $13

	j       ROM_start
	nop

ROM_start:

#if (0)
# Disable I or D Cache
	mfc0	$4, $16
	li      $5, (1<<19)
	or      $4, $4, $5
	mtc0    $4, $16

    mfc0	$4, $16,1
#	li      $5, (0xFFC7E3FF)	#disable I/D cache
#	li      $5, (0xFE3FFFFF)	#set I cache size to 8KByte only
#	li      $5, (0xFE07FFFF)	#disable I cache and set I cache size to 8KByte
#	li      $5, (0xFFC7FFFF)	#disable I cache
#	li      $5, (0xFFFF1FFF)	#set D cache size to 8KByte only
#	li      $5, (0xFFFF03FF)	#disable D cache and set D cache size to 8KByte
    li      $5, (0xFFFFE3FF)	#disable D cache
    and     $4, $4, $5
    mtc0	$4, $16,1

	mfc0	$4, $16
	li      $5, ~(1<<19)
	or      $4, $4, $5
	mtc0    $4, $16

#    # switch to fixed memory mapping
#	mfc0	$4, $16
#	li      $5, 0xFFFFFC7F
#	and     $4, $4, $5
#	ori     $4, $4, 0x180
#	mtc0    $4, $16
#endif


	# Clear CPU reset register
	#WREG    0xbf203ddc, 0x029f
#endif

#if (0)
	# Initialize Regs
	bal     BOOT_InitReg
	nop
#endif


#	DBGREG  0x0001


	la	$8, 1f
	and $8, ~0xE0000000
	or  $8, 0xA0000000
	j	$8
	nop
1:

#if (1)
	# Initialize Cache
	bal     BOOT_InitCache
	nop
#endif


	la	$8, 1f
	and $8, ~0xE0000000
	or  $8, 0x80000000
	j	$8
	nop
1:


#=================================================================================================


	kernel_entry_setup			# cpu specific setup

	setup_c0_status_pri

	/* We might not get launched at the address the kernel is linked to,
	   so we jump there.  */
	PTR_LA	t0, 0f
	jr	t0
0:

#ifdef CONFIG_MIPS_MT_SMTC
	/*
	 * In SMTC kernel, "CLI" is thread-specific, in TCStatus.
	 * We still need to enable interrupts globally in Status,
	 * and clear EXL/ERL.
	 *
	 * TCContext is used to track interrupt levels under
	 * service in SMTC kernel. Clear for boot TC before
	 * allowing any interrupts.
	 */
	mtc0	zero, CP0_TCCONTEXT

	mfc0	t0, CP0_STATUS
	ori	t0, t0, 0xff1f
	xori	t0, t0, 0x001e
	mtc0	t0, CP0_STATUS
#endif /* CONFIG_MIPS_MT_SMTC */

	PTR_LA		t0, __bss_start		# clear .bss
	LONG_S		zero, (t0)
	PTR_LA		t1, __bss_stop - LONGSIZE
1:
	PTR_ADDIU	t0, LONGSIZE
	LONG_S		zero, (t0)
	bne		t0, t1, 1b

	LONG_S		a0, fw_arg0		# firmware arguments
	LONG_S		a1, fw_arg1
	LONG_S		a2, fw_arg2
	LONG_S		a3, fw_arg3

	MTC0		zero, CP0_CONTEXT	# clear context register
	PTR_LA		$28, init_thread_union
	PTR_LI		sp, _THREAD_SIZE - 32
	PTR_ADDU	sp, $28
	set_saved_sp	sp, t0, t1
	PTR_SUBU	sp, 4 * SZREG		# init stack pointer

	j		start_kernel
	END(kernel_entry)

	__CPUINIT

#ifdef CONFIG_SMP
/*
 * SMP slave cpus entry point.  Board specific code for bootstrap calls this
 * function after setting up the stack and gp registers.
 */
NESTED(smp_bootstrap, 16, sp)
#ifdef CONFIG_MIPS_MT_SMTC
	/*
	 * Read-modify-writes of Status must be atomic, and this
	 * is one case where CLI is invoked without EXL being
	 * necessarily set. The CLI and setup_c0_status will
	 * in fact be redundant for all but the first TC of
	 * each VPE being booted.
	 */
	DMT	10	# dmt t2 /* t0, t1 are used by CLI and setup_c0_status() */
	jal	mips_ihb
#endif /* CONFIG_MIPS_MT_SMTC */
	setup_c0_status_sec
	smp_slave_setup
#ifdef CONFIG_MIPS_MT_SMTC
	andi	t2, t2, VPECONTROL_TE
	beqz	t2, 2f
	EMT		# emt
2:
#endif /* CONFIG_MIPS_MT_SMTC */
	j	start_secondary
	END(smp_bootstrap)
#endif /* CONFIG_SMP */

	__FINIT


#=================================================================================================
BOOT_InitReg:
# alan.chen adds start

	# Clear watch registers
	li      $26, 0x07
	mtc0    $26, $18

	nop
	nop

#dhjung LGE : zero is not defined
	#mtc0	$0, $18
	mtc0	$0, $19

	nop
	nop

	# Disable interrupts and KSU field (power up issue)
	mfc0   	$26, $12
	sll		$27, $26, (31-S_StatusNMI)	# Is this really from an NMI?
	bltz	$27, 1f
	and		$27, M_StatusNMI >> S_StatusNMI
	li		$27, ~(M_StatusIE | M_StatusKSU)
	and		$26, $27
	mtc0	$26, $12

	# Don't touch cause if we got here from an NMI
	mfc0   	$26, $12
	sll		$26, (31-S_StatusNMI)
	bltz	$26, 1f

	# Setup cause register (clear cause code)
	li		$26, M_CauseIV
	mtc0	$26, $13

1:
	 nop

# alan.chen adds end

	# INIT DEBUG
# alan.chen removes
#	mfc0    $8, $23
#	and     $8, (~M_DebugCOUNTDM)
#	mtc0    $8, $23

	# CLEAR COUNT
# alan.chen removes -> re-enabled by dhjung LGE
	mtc0    $0, $9
	mtc0    $0, $11

	# INIT STATUS
    # TODO:
# alan.chen removes
#	li      $8, INITIAL_STATUS
#	mtc0    $8, $12

    WREG    0xbf206714, 0xF002

	# INIT CAUSE
# alan.chen removes
#	mtc0    $0, $13

	# INIT CONFIG

# alan.chen removes start
#	li	    $8, INITIAL_CONFIG0
#	mtc0	$8, $16
# alan.chen removes end

# alan.chen adds start
	/* Setup MIPS34K specifics (implementation dependent fields) */
	mfc0    $8, $16
	li	    $9, CONFIG0_MIPS32_64_MSK
	and	    $8, $9
	li	    $9, CONFIG0_MIPS34K
	or      $8, $9
	mtc0	$8, $16

	mfc0    $8, $12
	li	    $9, STATUS_MIPS32_64_MSK
	and     $8, $9
	li      $9, STATUS_MIPS34K
	or      $8, $9
	mtc0	$8, $12
# alan.chen adds end

	jr      $31
	nop

#=================================================================================================
BOOT_InitCache:
    # Clear TagLo & TagHi
    mtc0    $0, $28, 0
    ehb
    mtc0    $0, $28, 2
    ehb
    mtc0    $0, $29
    ehb

    # Config1
    mfc0    $8, $16, 1
    ehb
    ### INIT I-CACHE ###

    # Line Size $10
    li      $9, M_Config1IL
    and     $9, $8, $9
    srl     $9, $9, S_Config1IL
    beqz    $9, Icache_done    # no Icache
    nop
    li      $10, 2
    sll     $10, $10, $9

    # Sets per way $11
    li      $9, M_Config1IS
    and     $9, $8, $9
    srl     $9, $9, S_Config1IS
    li      $11, 64
    sll     $11, $11, $9

    # Associativity $12
    li      $9, M_Config1IA
    and     $9, $8, $9
    srl     $9, $9, S_Config1IA
    add     $12, $9, 1

    li      $14, A_KSEG0BASE
    move    $15, $11

Icache_loop:
    # Invalidate each tag entry
    cache   Index_Store_Tag_I, 0($14)
    ehb

    add     $14, $10
    sub     $15, 1
    bnez    $15, Icache_loop
    nop
    move    $15, $11
    sub     $12, 1
    bnez    $12, Icache_loop
    nop

Icache_done:


    ### INIT D-CACHE ###

    # Line Size $10
    li      $9, M_Config1DL
    and     $9, $8, $9
    srl     $9, $9, S_Config1DL
    beqz    $9, Dcache_done    # no Dcache
    nop
    li      $10, 2
    sll     $10, $10, $9

    # Sets per way $11
    li      $9, M_Config1DS
    and     $9, $8, $9
    srl     $9, $9, S_Config1DS
    li      $11, 64
    sll     $11, $11, $9

    # Associativity $12
    li      $9, M_Config1DA
    and     $9, $8, $9
    srl     $9, $9, S_Config1DA
    add     $12, $9, 1

    li      $14, A_KSEG0BASE
    move    $15, $11

Dcache_loop:
    # Invalidate each tag entry
    cache   Index_Store_Tag_D, 0($14)
    ehb

    add     $14, $10
    sub     $15, 1
    bnez    $15, Dcache_loop
    nop
    move    $15, $11
    sub     $12, 1
    bnez    $12, Dcache_loop
    nop

Dcache_done:

    # Enable cache
    mfc0    $8, $16
    ehb

    and     $8, (~M_ConfigK0)
#JunyouLin remove this, got logical bug
#   li      $8, (K_CacheAttrCN)
    addi    $8, $8, (K_CacheAttrCN) #JunyouLin added
    mtc0    $8, $16
    ehb

    jr      $31
    nop
#=================================================================================================
