K_CacheAttrCN       = (0x3  << 0) #Cacheable, noncoherent, write-back, write allocate.
K_UnCache 	    = (0x2  << 0) #UnCache.

M_ConfigK0          = (0x7  << 0)
S_Config1IS         = 22
M_Config1IS         = (0x7  << S_Config1IS)
S_Config1IL         = 19
M_Config1IL         = (0x7  << S_Config1IL)
S_Config1IA         = 16
M_Config1IA         = (0x7  << S_Config1IA)
S_Config1DS         = 13
M_Config1DS         = (0x7  << S_Config1DS)
S_Config1DL         = 10
M_Config1DL         = (0x7  << S_Config1DL)
S_Config1DA         = 7
M_Config1DA         = (0x7  << S_Config1DA)

A_KSEG0BASE         = 0x80000000


#Index_Invalidate_I  	0x00 (000 00)
#Index_Writeback_Inv_D   0x01 (000 01)
#Index_Writeback_Inv_SD  0x03 (000 11)
#Index_Load_Tag_I	0x04 (001 00)
#Index_Load_Tag_D 	0x05 (001 01)
#Index_Store_Tag_I	0x08 (010 00)
#Index_Store_Tag_D 	0x09 (010 01)
#Hit_Invalidate_I	0x10 (100 00)
#Hit_Invalidate_D 	0x11 (100 01)
#Hit_Invalidate_SD 	0x13 (100 11)
#Hit_Writeback_Inv_D 	0x15 (101 00)
#Hit_Writeback_Inv_SD    0x17 (101 11)


Index_Load_Tag_I    	= 0x4
Index_Load_Tag_D    	= 0x5
Index_Store_Tag_I   	= 0x8
Index_Store_Tag_D   	= 0x9
Address_Fill_I      	= 0x14
Address_Fetch_lock_I 	= 0x1C
Index_Writeback_Inv_D   = 0x01
Hit_Invalidate_D	= 0x11


CPU_CLOCK_FREQ          = 12000000


BIT0 = 0x0001
BIT1 = 0x0002
BIT2 = 0x0004
BIT3 = 0x0008
BIT4 = 0x0010
BIT5 = 0x0020
BIT6 = 0x0040
BIT7 = 0x0080
BIT8 = 0x0100
BIT9 = 0x0200
BIT10= 0x0400
BIT11= 0x0800
BIT12= 0x1000
BIT13= 0x2000
BIT14= 0x4000
BIT15= 0x8000

# register base
MIU0 		= 0x1200
MIU1 		= 0x0600
MIU0_LOST 	= 0x0600
MIU1_LOST 	= 0x0600
PM 		= 0x0E00



#MIPS_WAKEUP_ADR = 0xA2800000
#MIPS_RET_ADR    = 0xA2800004
#MIPS_STACK_PNT  = 0xA2800008
#MIPS_GPS        = 0xA2900000
#MIPS_SRAM_KEEP  = 0xA2A00000
#(0x80000000 + 0x04000000 + 0x1000)
#MIPS_SRAM_ADR   = 0xA4001000

#MIPS_DATA       = 0xA2B00000

#MICOM_ADDR      = 0x00001FFC
#-------------------------------------------------------------------------------------------------
# Macros
#-------------------------------------------------------------------------------------------------
# --------------------------------------------------------------
# MACRO NAME 	: 	R2B_REG
# PARAMETERS 	:	base, RIU register base address
# PARAMETERS 	:	index, RIU register index
# PARAMETERS 	:	rd, register name
# INTERNAL REGS	: $8
# RETURN     	: rd
# --------------------------------------------------------------
.macro R2B_REG base, index, rd
li      $8, (((\index+\base) >> 1) << 2) + ((\index+\base)&0x01) + 0xBF200000
lw      \rd, 0($8)
.endm

# --------------------------------------------------------------
# MACRO NAME 	: 	R1B_REG
# PARAMETERS 	:	base, RIU register base address
# PARAMETERS 	:	index, RIU register index
# PARAMETERS 	:	rd, register name
# INTERNAL REGS	: $8
# RETURN     	: rd
# --------------------------------------------------------------
.macro R1B_REG base, index, rd
li      $8, (((\index+\base) >> 1) << 2) + ((\index+\base)&0x01) + 0xBF200000
lw      \rd, 0($8)
.endm

# --------------------------------------------------------------
# MACRO NAME 	: 	W2B_REG_R
# PARAMETERS 	:	base, RIU register base address
# PARAMETERS 	:	index, RIU register index
# PARAMETERS 	:	rd, register name
# INTERNAL REGS	: $8
# RETURN     	: rd
# --------------------------------------------------------------
.macro W2B_REG_R base, index, rd
li      $8, (((\index+\base) >> 1) << 2) + ((\index+\base)&0x01) + 0xBF200000
sw      \rd, 0($8)
.endm


# --------------------------------------------------------------
# MACRO NAME 	: 	W2B_REG
# PARAMETERS 	:	base, RIU register base address
# PARAMETERS 	:	index, RIU register index
# PARAMETERS 	:	value
# INTERNAL REGS	: $8, $9
# RETURN     	: none
# --------------------------------------------------------------
.macro W2B_REG base, index, value
li      $8, (((\index+\base) >> 1) << 2) + ((\index+\base)&0x01) + 0xBF200000
li      $9, (\value &0xFFFF)
sw      $9, 0($8)
.endm

# --------------------------------------------------------------
# MACRO NAME 	: 	W1B_REG
# PARAMETERS 	:	base, RIU register base address
# PARAMETERS 	:	index, RIU register index
# PARAMETERS 	:	value, value
# INTERNAL REGS	: $8, $9
# RETURN     	:  none
# --------------------------------------------------------------
.macro W1B_REG base, index, value
# $8 and $9 = BYTE2REAL(B)            (((B)>>1<<2)+((B)&0x01)
li      $8, (((\index+\base) >> 1) << 2) + ((\index+\base)&0x01) + 0xBF200000
li      $9, (\value & 0xFF)
sb      $9, 0($8)
.endm


# --------------------------------------------------------------
# MACRO NAME 	: 	W1BC_REG
# PARAMETERS 	:	base, RIU register base address
# PARAMETERS 	:	index, RIU register index
# PARAMETERS 	:	bit_inx, clear bits
# INTERNAL REGS	: $8, $9
# RETURN     	:  none
# --------------------------------------------------------------
.macro W1BC_REG base, index, bit_inx
li      $8, (((\index+\base) >> 1) << 2) + ((\index+\base)&0x01) + 0xBF200000
lb      $9, 0($8)
andi	$9, $9, (~\bit_inx)&0x00FF
sb      $9, 0($8)
.endm

# --------------------------------------------------------------
# MACRO NAME 	: 	W1BS_REG
# PARAMETERS 	:	base, RIU register base address
# PARAMETERS 	:	index, RIU register index
# PARAMETERS 	:	bit_inx, set bits
# INTERNAL REGS	: $8, $9
# RETURN     	:  none
# --------------------------------------------------------------
.macro W1BS_REG base, index, bit_inx
li      $8, (((\index+\base) >> 1) << 2) + ((\index+\base)&0x01) + 0xBF200000
lb      $9, 0($8)
ori     $9, $9, \bit_inx
sb      $9, 0($8)
.endm

.macro SSSW reg, value
    lui     $8, \reg >> 16
    sw      \value, (\reg & 0xFFFF)($8)
.endm

.macro LSSSW reg, value
    sw      \value, (\reg & 0xFFFF)($8)
.endm

.macro PUTCW char
    W1B_REG     0x980, 0x0, \char
    PUTC_\@:

    li          $8, 0xBF201328
    lw          $8, 0($8)
    andi        $8, $8, 0x60
    li          $9, 0x60
    bne         $8, $9, PUTC_\@

    nop
.endm

.macro PUTC char
    lui     $27, 0
    ori     $27, $27, \char
    SSSW    0xBF201300, $27
.endm


.macro DELAYUS us
    li      $10, \us
    li      $8, 1
DELAYUS_\@:
    subu    $10, $10, $8
    bgtz    $10, DELAYUS_\@
    nop
.endm

.macro PUTR reg
SSSW    0xBF201300, \reg
.endm


#.macro RREG reg_d, reg
#   	lui     $8, \reg >> 16
#	lw      \reg_d, (\reg & 0xFFFF)($8)
#   .endm


.macro RREG reg_d, reg
    lui     $8, \reg >> 16
	lw      \reg_d, 0($8)
.endm

