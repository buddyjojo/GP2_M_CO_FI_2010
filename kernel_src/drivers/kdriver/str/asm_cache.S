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
#include "asm_str.inc"

.text
.set     noreorder

.global asm_sets_per_way
.global asm_line_size
.global asm_associativity
.global asm_dump_icache_tag
.global asm_strflash_entry
.global asm_invalid_cache
.global asm_fill_icache
.global asm_lock_fetch_icache

.global asm_flush_d_cache
.global asm_invalid_d_cache

# -===========================================-
# function      :   asm_invalid_cache
# parameters    :   none
# return        :   none
# -===========================================-
asm_invalid_cache:
    # Clear TagLo & TagHi
    mtc0    $0, $28
    mtc0    $0, $29

    # Config1
    mfc0    $8, $16, 1

    ### clear I-CACHE ###
    # Line Size $10
    li      $9, M_Config1IL
    and     $9, $8, $9
    srl     $9, $9, S_Config1IL # right shift
    beqz    $9, Icache_done     # no Icache
    nop
    li      $10, 2
    sll     $10, $10, $9        # left shift

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

    li      $15, 0x1000
Icache_loop:
    # Invalidate each tag entry
    cache   Index_Store_Tag_I, 0($14)
    add     $14, $14, 1
    sub     $15, 1
    bnez    $15, Icache_loop    # loop until Set = 0
    nop


Icache_done:

    jr      $31
    nop


# -===========================================-
# function      :   asm_flush_d_cache
# parameters    :   none
# return        :   none
# -===========================================-
asm_flush_d_cache:

    # Config1
    mfc0    $8, $16, 1

    ### Get D-CACHE Info ###

    # Line Size $10
    li      $9, M_Config1DL
    and     $9, $8, $9
    srl     $9, $9, S_Config1DL
    beqz    $9, Dcache_done    # no Dcache
    nop
    add     $9, $9, 1
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
    li      $12, 0
    add     $12, $9, 1

    li      $14, A_KSEG0BASE
    #li      $14, 0
    #move    $15, $10

    li       $15, 0x1000

Dcache_loop:
    # Invalidate each tag entry
    cache   Index_Writeback_Inv_D, 0($14)
    add     $14, $14, 1
    sub     $15, 1
    bnez    $15, Dcache_loop
    nop

Dcache_done:

    # Enable cache
    mfc0    $8, $16
    and     $8, (~M_ConfigK0)
    li      $8, (K_CacheAttrCN)
    mtc0    $8, $16


    jr      $31
    nop


# -===========================================-
# function      :   asm_invalid_d_cache
# parameters    :   none
# return        :   none
# -===========================================-
asm_invalid_d_cache:

    # Clear TagLo & TagHi
    mtc0    $0, $28
    mtc0    $0, $29

    # Config1
    mfc0    $8, $16, 1

    ### Get D-CACHE Info ###

    # Line Size $10
    li      $9, M_Config1DL
    and     $9, $8, $9
    srl     $9, $9, S_Config1DL
    beqz    $9, Dcache_done    # no Dcache
    nop
    add     $9, $9, 1
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
    li      $12, 0
    add     $12, $9, 1

    li      $14, A_KSEG0BASE
    move    $15, $10

    li      $15, 0x1000

Dcache_loop_inv:
    # Invalidate each tag entry
    cache   Hit_Invalidate_D, 0($14)
    add     $14, $14, 1
    sub     $15, 1
    bnez    $15, Dcache_loop_inv    # loop until Set = 0
    nop


Dcache_done_inv:

    # Enable cache
    mfc0    $8, $16
    and     $8, (~M_ConfigK0)
    li      $8, (K_CacheAttrCN)
    mtc0    $8, $16


    jr      $31
    nop



# -===========================================-
# function      :   asm_line_size
# parameters    :   none
# return        :   $2:cache line size
# -===========================================-
asm_line_size:
    mfc0    $8, $16, 1

    # Line Size $10
    li      $9, M_Config1IL
    and     $9, $8, $9
    srl     $9, $9, S_Config1IL # right shift

    # return
    jr      $31
    move    $2, $9

# -===========================================-
# function      :   asm_sets_per_way
# parameters    :   none
# return        :   $2:sets per way
# -===========================================-
asm_sets_per_way:
    mfc0    $8, $16, 1

    # Sets per way $11
    li      $9, M_Config1IS
    and     $9, $8, $9
    srl     $9, $9, S_Config1IS

	# return
    jr      $31
    move    $2, $9

# -===========================================-
# function      :   asm_associativity
# parameters    :   none
# return        :   $2:total cache lines
# -===========================================-
asm_associativity:
    mfc0    $8, $16, 1

    # Associativity $12
    li      $9, M_Config1IA
    and     $9, $8, $9
    srl     $9, $9, S_Config1IA

	# return
    jr      $31
    move    $2, $9


# -===========================================-
# function      :   asm_fill_icache
# parameters    :   $4:start address
# parameters    :   none
# return        :   none
# -===========================================-
asm_fill_icache:
    cache   Address_Fill_I, 0($4)
    jr      $31
    nop

# -===========================================-
# function      :   asm_dump_icache_tag
# parameters    :   $4:address [0:3] index bytes, [4:9] index, [10:11] ways.
# return        :   $2:cache line tag
# variables     :   $8:TagLo
# -===========================================-
asm_dump_icache_tag:
    cache   Index_Load_Tag_I, 0($4)
    mfc0    $8, $28
    jr      $31
    nop


# -===========================================-
# function      :   asm_lock_fetch_icache
# parameters    :   $4:start address
# parameters    :   none
# return        :   none
# -===========================================-
asm_lock_fetch_icache:
    cache   Address_Fetch_lock_I, 0($4)
    jr      $31
    nop


#if 0
/*******************************************************************************
*
* flush_cache_all - Writeback and invalidate all caches
*
* PARAMS : void
* RETURNS: N/A
*
*/
    .globl  flush_cache_all
    .ent    flush_cache_all
flush_cache_all:
    li      a0,     K0_MEM_BASE
    li      a1,     MIPS_DCACHE_SIZE
    li      a2,     MIPS_CACHE_LINE_SIZE

    /* make max address */
    addu    a1,     a0

    /* address space alignment */
    _align(a0, a1, a2)

    .set    noreorder
1:  cache   Index_Writeback_Inv_D,  0(a0)
    nop
    cache   Index_Writeback_Inv_D,  1(a0)
    nop
    cache   Index_Writeback_Inv_D,  2(a0)
    nop
    cache   Index_Writeback_Inv_D,  3(a0)
    nop

    bne     a0,     a1,     1b
    add     a0,     a2
    .set    reorder

    li      a0,     K0_MEM_BASE
    li      a1,     MIPS_ICACHE_SIZE
    li      a2,     MIPS_CACHE_LINE_SIZE

    /* make max address */
    addu    a1,     a0

    /* address space alignment */
    _align(a0, a1, a2)

    .set    noreorder
2:  cache   Index_Invalidate_I,     0(a0)
    nop
    cache   Index_Invalidate_I,     1(a0)
    nop
    cache   Index_Invalidate_I,     2(a0)
    nop
    cache   Index_Invalidate_I,     3(a0)
    nop

    bne     a0,     a1,     2b
    add     a0,     a2
    .set    reorder

    jr      ra

    .end    flush_cache_all
#endif
