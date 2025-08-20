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


.global save_cp0
.global save_tlb_entries
.global set_tlb_boundary
.global restore_cp0
.global restore_tlb_entries


# -===========================================-
# function      :   set_tlb_boundary
# parameters    :   $4:tlb boundary number
# parameters    :   none
# return        :   none
# -===========================================-
set_tlb_boundary:
    mtc0    $4, $6, 0
    jr      $31
    nop

# -===========================================-
# function      :   save_tlb_entries
# parameters    :   $4:tlb index, $5:save address
# parameters    :   none
# return        :   none
# -===========================================-
save_tlb_entries:

#The EntryHi, EntryLo0, EntryLo1, and PageMask registers are loaded with the contents of the TLB entry pointed to
#by the Index register.

    mtc0    $4, $0, 0
    ehb
    tlbr
    ehb

    # EntryHi
    mfc0    $8, $10, 0
    sw      $8,0($5)

    #EntryLo0
    mfc0    $8, $2, 0
    sw      $8,4($5)

    #EntryLo1
    mfc0    $8, $3, 0
    sw      $8,8($5)

    #PageMask
    mfc0    $8, $5, 0
    sw      $8,12($5)


    jr      $31
    nop

# -===========================================-
# function      :   restore_tlb_entries
# parameters    :   $4:tlb index, $5:save address
# parameters    :   none
# return        :   none
# -===========================================-
restore_tlb_entries:

#The EntryHi, EntryLo0, EntryLo1, and PageMask registers are loaded with the contents of the TLB entry pointed to
#by the Index register.

    # set index
    mtc0    $4, $0, 0

    # set EntryHi
    lw      $8,0($5)
    mtc0    $8, $10, 0

    #set EntryLo0
    lw      $8,4($5)
    mtc0    $8, $2, 0

    #set EntryLo1
    lw      $8,8($5)
    mtc0    $8, $3, 0

    #set PageMask
    lw      $8,12($5)
    mtc0    $8, $5, 0

    ehb
    tlbwi
    ehb

    jr      $31
    nop



# -===========================================-
# function      :   save_cp0
# parameters    :   $4:save address
# parameters    :   none
# return        :   none
# -===========================================-
save_cp0:
    mfc0    $8, $0, 1
    sw      $8,0($4)
    mfc0    $8, $1, 1
    sw      $8,4($4)
    mfc0    $8, $1, 2
    sw      $8,8($4)
    mfc0    $8, $1, 3
    sw      $8,12($4)
    mfc0    $8, $1, 4
    sw      $8,16($4)
    mfc0    $8, $1, 7
    sw      $8,20($4)

    mfc0    $8, $2, 1
    sw      $8,24($4)
    mfc0    $8, $2, 2
    sw      $8,28($4)
    mfc0    $8, $2, 5
    sw      $8,32($4)
    mfc0    $8, $2, 6
    sw      $8,36($4)
    mfc0    $8, $2, 7
    sw      $8,40($4)

    mfc0    $8, $4, 0
    sw      $8,44($4)
    mfc0    $8, $4, 2
    sw      $8,48($4)

    mfc0    $8, $5, 0
    sw      $8,52($4)

    mfc0    $8, $6, 0
    sw      $8,56($4)
    mfc0    $8, $6, 1
    sw      $8,60($4)

    mfc0    $8, $7, 0
    sw      $8,64($4)

	# don't change below two CP0 register sequence
    mfc0    $8, $11, 0
    sw      $8,68($4)
    mfc0    $8, $9, 0
    sw      $8,72($4)

    mfc0    $8, $12, 0
    sw      $8,76($4)
    mfc0    $8, $12, 1
    sw      $8,80($4)
    mfc0    $8, $12, 2
    sw      $8,84($4)
    mfc0    $8, $12, 3
    sw      $8,88($4)

    mfc0    $8, $13, 0
    sw      $8,92($4)

    mfc0    $8, $14, 0
    sw      $8,96($4)

    mfc0    $8, $15, 1
    sw      $8,100($4)

    mfc0    $8, $16, 0
    sw      $8,104($4)
    mfc0    $8, $16, 1
    sw      $8,108($4)
    mfc0    $8, $16, 2
    sw      $8,112($4)
    mfc0    $8, $16, 7
    sw      $8,116($4)

    mfc0    $8, $25, 0
    sw      $8,120($4)
    mfc0    $8, $25, 1
    sw      $8,124($4)
    mfc0    $8, $25, 2
    sw      $8,128($4)
    mfc0    $8, $25, 3
    sw      $8,132($4)

    mfc0    $8, $26, 0
    sw      $8,136($4)

    mfc0    $8, $30, 0
    sw      $8,140($4)

    jr      $31
    nop

# -===========================================-
# function      :   restore_cp0
# parameters    :   $4:save address
# parameters    :   none
# return        :   none
# -===========================================-
restore_cp0:
    lw      $8,0($4)
    mtc0    $8, $0, 1
    lw      $8,4($4)
    mtc0    $8, $1, 1
    lw      $8,8($4)
    mtc0    $8, $1, 2
    lw      $8,12($4)
    mtc0    $8, $1, 3
    lw      $8,16($4)
    mtc0    $8, $1, 4
    lw      $8,20($4)
    mtc0    $8, $1, 7

    lw      $8,24($4)
    mtc0    $8, $2, 1
    lw      $8,28($4)
    mtc0    $8, $2, 2
    lw      $8,32($4)
    mtc0    $8, $2, 5
    lw      $8,36($4)
    mtc0    $8, $2, 6
    lw      $8,40($4)
    mtc0    $8, $2, 7

    lw      $8,44($4)
    mtc0    $8, $4, 0
    lw      $8,48($4)
    mtc0    $8, $4, 2

    lw      $8,52($4)
    mtc0    $8, $5, 0

    lw      $8,56($4)
    mtc0    $8, $6, 0
    lw      $8,60($4)
    mtc0    $8, $6, 1

    lw      $8,64($4)
    mtc0    $8, $7, 0

	# don't change below two CP0 register sequence
    lw      $8,68($4)
    mtc0    $8, $11, 0
    lw      $8,72($4)
    mtc0    $8, $9, 0

    lw      $8,76($4)
    mtc0    $8, $12, 0
    lw      $8,80($4)
    mtc0    $8, $12, 1
    lw      $8,84($4)
    mtc0    $8, $12, 2
    lw      $8,88($4)
    mtc0    $8, $12, 3

    lw      $8,92($4)
    mtc0    $8, $13, 0

    lw      $8,96($4)
    mtc0    $8, $14, 0

    lw      $8,100($4)
    mtc0    $8, $15, 1

    lw      $8,104($4)
    mtc0    $8, $16, 0
    lw      $8,108($4)
    mtc0    $8, $16, 1
    lw      $8,112($4)
    mtc0    $8, $16, 2
    lw      $8,116($4)
    mtc0    $8, $16, 7

    lw      $8,120($4)
    mtc0    $8, $25, 0
    lw      $8,124($4)
    mtc0    $8, $25, 1
    lw      $8,128($4)
    mtc0    $8, $25, 2
    lw      $8,132($4)
    mtc0    $8, $25, 3

    lw      $8,136($4)
    mtc0    $8, $26, 0

    lw      $8,140($4)
    mtc0    $8, $30, 0

    jr      $31
    nop

