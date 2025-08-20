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
.global ddr_enter_selfrefresh
.global deep_sleep


# -===========================================-
# function      :   ddr_enter_selfrefresh
# parameters    :   none
# return        :   none
# -===========================================-
ddr_enter_selfrefresh:

    # Mask request
    W1B_REG MIU0, 0x46, 0xFE
    W1B_REG MIU0, 0x47, 0xFF
    W1B_REG MIU1, 0x46, 0xFE
    W1B_REG MIU1, 0x47, 0xFF

    W1B_REG MIU0, 0x66, 0xFF
    W1B_REG MIU0, 0x67, 0xFF
    W1B_REG MIU1, 0x66, 0xFF
    W1B_REG MIU1, 0x67, 0xFF

    W1B_REG MIU0, 0x86, 0xFF
    W1B_REG MIU0, 0x87, 0xFF
    W1B_REG MIU1, 0x86, 0xFF
    W1B_REG MIU1, 0x87, 0xFF

    W1B_REG MIU0, 0xA6, 0xFF
    W1B_REG MIU0, 0xA7, 0xFF
    W1B_REG MIU1, 0xA6, 0xFF
    W1B_REG MIU1, 0xA7, 0xFF
    # Wait 1 us
    DELAYUS 1000
    W1B_REG MIU0, 0x1E, 0x00


    # Turn off ODT & auto refresh
    W1BC_REG MIU0, 0x00, BIT4
    W1BC_REG MIU1, 0x00, BIT4
    W1BS_REG MIU0, 0x00, BIT5
    W1BS_REG MIU1, 0x00, BIT5
    # Enter idle state
    W1B_REG MIU0, 0x18, 0x00
    W1B_REG MIU1, 0x18, 0x00
    W1B_REG MIU0, 0x19, 0x05
    W1B_REG MIU1, 0x19, 0x05

    W1B_REG MIU0, 0x00, 0x2E
    W1B_REG MIU1, 0x00, 0x2E

    W1B_REG MIU0, 0x01, 0x05
    W1B_REG MIU1, 0x01, 0x05

    W1B_REG MIU0, 0x00, 0x2E
    W1B_REG MIU1, 0x00, 0x2E

    W1B_REG MIU0, 0x01, 0x00
    W1B_REG MIU1, 0x01, 0x00
    #
    W1B_REG MIU0, 0x18, 0x00
    W1B_REG MIU1, 0x18, 0x00

    W1B_REG MIU0, 0x19, 0x05
    W1B_REG MIU1, 0x19, 0x05

    W1B_REG MIU0, 0x00, 0x2E
    W1B_REG MIU1, 0x00, 0x2E

    W1B_REG MIU0, 0x01, 0x03
    W1B_REG MIU1, 0x01, 0x03

    W1B_REG MIU0, 0x00, 0x2E
    W1B_REG MIU1, 0x00, 0x2E

    W1B_REG MIU0, 0x01, 0x00
    W1B_REG MIU1, 0x01, 0x00

    W1BS_REG MIU0, 0x07, BIT4
    W1BS_REG MIU1, 0x07, BIT4
    # Wait 1 us
    DELAYUS 1000
    # Enter self refresh mode
    W1B_REG MIU0, 0x46, 0xFF
    W1B_REG MIU1, 0x46, 0xFF

    W1BS_REG MIU0, 0x01, BIT5
    W1BS_REG MIU1, 0x01, BIT5

    # tell PM sleep

	#set_gpio_174_as_output_low:
    # set all_PAD_IN  0x101ea1  #bit7 =0
    RREG    $11,         0xbf203D40
    and     $11, $11,    0x7FFF
    sw      $11,         0xbf203D40

    # set gpio174 as gpio; reg[101e24] = 0
    RREG    $11,         0xbf203C48
    and     $11, $11,    0xffcf
    sw      $11,         0xbf203C48

    # set gpio174 as gpio; reg[101eA2]#5 = 1
    RREG    $11,         0xbf203D44
    ori     $11, $11,    0x0020
    sw      $11,         0xbf203D44

	#configure gpio 174 101e04
	RREG 	$11, 0xbf203c08
	and		$11,	$11,	0x0000
	sw		$11, 0xbf203c08


	# configure [101e6e]#0 0 set out
	RREG	$11,		 0xbf203cdc
	and		$11, $11,	 0xffff
	sw 		$11,		 0xbf203cdc

	# configure	[101e6c]#1 0 set low
	RREG	$11,		 0xbf203cd8
	and		$11, $11,	 0xffff
	sw 		$11,		 0xbf203cd8

	#set_gpio_175_as_output_low:

    # set all_PAD_IN  0x101ea1  #bit7 =0
    RREG    $11,         0xbf203D40
    and     $11, $11,    0x7FFF
    sw      $11,         0xbf203D40

    # set gpio175 as gpio; reg[101e24] = 0
    RREG    $11,         0xbf203C48
    and     $11, $11,    0x0000
    sw      $11,         0xbf203C48

    # set gpio175 as gpio; reg[101eA2]#5 = 1
    RREG    $11,         0xbf203D44
    ori     $11, $11,    0x0020
    sw      $11,         0xbf203D44

	# configure [101e6e]#0 0 set out
	RREG	$11,		 0xbf203cdc
	and		$11, $11,	 0xffff
	sw 		$11,		 0xbf203cdc

	# configure	[101e6c]#1 0 set low
	RREG	$11,		 0xbf203cd8
	and		$11, $11,	 0xffff
	sw 		$11,		 0xbf203cd8

    PUTC	'K'
infinite_loop:
    # wait for power down
	j 	infinite_loop

end_of_ddr_enter_selfrefresh:


# -===========================================-
# function      :   deep_sleep
# parameters    :   $4 : stack backup address
# return        :   none
# -===========================================-
deep_sleep:
    sync

    sub		$29, $29, 4*100

    # save current stack pointer to memory
    sw      $29, 0($4)

    # save $16 - $23
    sw      $16,0($29)
    sw      $17,4($29)
    sw      $18,8($29)
    sw      $19,12($29)
    sw      $20,16($29)
    sw      $21,20($29)
    sw      $22,24($29)
    sw      $23,28($29)

    # save $24 - $27
    sw      $24,32($29)
    sw      $25,36($29)
    sw      $26,40($29)
    sw      $27,44($29)

    # save $28
    sw      $28,48($29)


    # save $30 - $31
    sw      $30, 52($29)
    sw      $31, 56($29)


    # save $2 - $15
    sw      $2,60($29)
    sw      $3,64($29)
    sw      $4,68($29)
    sw      $5,72($29)
    sw      $6,76($29)
    sw      $7,80($29)

    sw      $9,88($29)
    sw      $10,92($29)
    sw      $11,96($29)
    sw      $12,100($29)
    sw      $13,104($29)
    sw      $14,108($29)
    sw      $15,112($29)

    la      $8, local_r4k___flush_cache_all
    jalr    $8
    nop

    la      $8, Chip_Flush_Memory 
    jalr    $8
    nop

	sync

    ehb

    PUTC	'\r'
    PUTC	'\n'
    PUTC	'D'
    PUTC	'A'
    PUTC	'\r'
    PUTC	'\n'

    la      $12, ddr_enter_selfrefresh
    la      $13, end_of_ddr_enter_selfrefresh
cache_lock_loop:
    cache   Address_Fetch_lock_I, 0($12)
    add     $12, $12, 16
    bleu    $12, $13, cache_lock_loop
    nop

    # ddr enter self refresh
    la      $8, ddr_enter_selfrefresh
    jalr    $8
    nop

	# never reach here
never_reach_here:
	j 		never_reach_here




.global print_reg
print_reg:

    move    $11, $4
    rol     $11, $11, 0x4
    la      $12, char0
    move    $10, $0
    li      $10, 0x8

print_reg_loop:
    and     $9, $11, 0xf

    addu    $9, $12, $9

    lbu     $13, 0($9)

    PUTR    $13

    rol     $11, $11, 0x4

    subu    $10, $10, 0x1
    bnez    $10,  print_reg_loop
    nop

    jr      $31
    nop

.global print_addr
print_addr:
    lw      $11, 0($4)
    rol     $11, $11, 0x4
    la      $12, char0
    li      $10, 0x8
print_loop:
    sync

    and     $9, $11, 0xf

    addu    $9, $12, $9

    lbu     $13, 0($9)

    PUTR    $13

    sync

    rol     $11, $11, 0x4

    subu    $10, $10, 0x1
    bnez    $10, print_loop
    nop

    sync
    jr      $31
    nop

.align 4
char0:  .byte   0x30
char1:  .byte   0x31
char2:  .byte   0x32
char3:  .byte   0x33
char4:  .byte   0x34
char5:  .byte   0x35
char6:  .byte   0x36
char7:  .byte   0x37
char8:  .byte   0x38
char9:  .byte   0x39
chara:  .byte   0x61
charb:  .byte   0x62
charc:  .byte   0x63
chard:  .byte   0x64
chare:  .byte   0x65
charf:  .byte   0x66

