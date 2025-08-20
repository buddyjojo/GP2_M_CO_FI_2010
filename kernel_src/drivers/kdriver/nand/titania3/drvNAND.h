/*********************************************************************
 *   HEADER NAME:
 *       drvNAND.h
 *
 *   Written by Derek.Horng@MSTAR Inc.
 *---------------------------------------------------------------------
 *
 * Initial revision.
 *********************************************************************/
#ifndef DRVNAND_H
#define DRVNAND_H

#define MCU_8BIT    				1
#define MCU_32BIT   				2
#define NAND_PLATFORM 				MCU_32BIT
#define ENABLE_NAND 				1

#ifndef bool
#define bool 						int
#endif

#ifndef BOOL
#define BOOL 						int
#endif

#ifndef false
#define false 						0
#define true 						(!false)
#endif

/*--------------------------------------------------------------------
  LITERAL CONSTANTS
  --------------------------------------------------------------------*/
//cached/unchched segment
#define KSEG0_BASE					((void *)0x80000000)
#define KSEG1_BASE					((void *)0xa0000000)
#define KSEG0_SIZE					0x20000000
#define KSEG1_SIZE					0x20000000

//cached <-> unchched
#define KSEG02KSEG1(addr)			((void *)((U32)(addr)|0x20000000))
#define KSEG12KSEG0(addr)			((void *)((U32)(addr)&~0x20000000))

//virtual <-> physical
#define VA2PA(addr)   				((void *)(((U32)addr) & 0x1fffffff))
#define PA2KSEG0(addr)				((void *)(((U32)addr) | 0x80000000))
#define PA2KSEG1(addr)				((void *)(((U32)addr) | 0xa0000000))

//U32<->U16
#define LOU16(u32Val)				( (U16)(u32Val) )
#define HIU16(u32Val)				( (U16)((u32Val) >> 16) )
#define CATU32(u16HiVal, u16LoVal)  ( (U32)(u16HiVal)<<16 | (U32)(u16LoVal) )

#define DBG_NAND(x)					//(x)
#define diag_printf 				printf

#define ENABLE_LINUX_MTD            1
#define ENABLE_LINUX_2K_NAND        1
#define MULTI_CE            		0   //1 original is 1 and set to 0 for test on S5
#define MAX_CE_NO           		1

#define GetUnCachedAddr(x)			((U8*)((U32)(x) + 0x20000000))   // Access data in DRAM not Cache
#define PAGE_SECTOR_SIZE  			2048
#define PAGE_RDD_SIZE     			64
#define NFIE_PAGE_SIZE				(PAGE_SECTOR_SIZE+PAGE_RDD_SIZE)

//===================================================================
// fcie register constant
//===================================================================
#define NFIE_OffShift				2
#define REG(addr)					(*(volatile U32 *)(addr))

#if defined(CONFIG_Titania3)
#define SYS_BASE     				0xBF200000
#define CHIPTOP_BASE 				(SYS_BASE+(0xF00<<NFIE_OffShift))
#define NANDIE_BASE  				(SYS_BASE+(0x8100<<NFIE_OffShift))  //0xBF220400
#define CLKGEN0_BASE 				(SYS_BASE+(0x580<<NFIE_OffShift))
#define reg_clk_nfie 				(CLKGEN0_BASE+(0x64<<NFIE_OffShift))
#define reg_chiptop_A           	(CHIPTOP_BASE+(0xA<<NFIE_OffShift))

#else
#define SYS_BASE    				0xBF800000
#define CHIPTOP_BASE				(SYS_BASE+(0xF00<<NFIE_OffShift))
#define NANDIE_BASE 				(SYS_BASE+(0x180<<NFIE_OffShift))
#define reg_clk_nfie				(CHIPTOP_BASE+(0x1C<<NFIE_OffShift))
#define reg_chiptop_A				(CHIPTOP_BASE+(0xA<<NFIE_OffShift))
#endif

// system register constant
#define reg_chiptop_8				(CHIPTOP_BASE+(0x8<<NFIE_OffShift))
#define reg_nfie_en  				(0x80)
#define reg_alt_nd   				(0x20)
#define reg_gpioc_oen				((U16)0x46<<1)

#define reg_clk_fcie        		((U16)0x43<<1)
#define REG_CHIPTOP_ALT_TRAP		(0xA<<1)
#define reg_alt_nd_cez      		(0x20)
#define reg_alt_nd_dl       		(0x40)

#define NC_MIE_EVENT   				(NANDIE_BASE+(0x00<<NFIE_OffShift))
#define NC_MIE_INT_EN  				(NANDIE_BASE+(0x01<<NFIE_OffShift))
#define NC_MMA_PRI_REG 				(NANDIE_BASE+(0x02<<NFIE_OffShift))
#define NC_MIU_DMA1    				(NANDIE_BASE+(0x03<<NFIE_OffShift))
#define NC_MIU_DMA0    				(NANDIE_BASE+(0x04<<NFIE_OffShift))
#define NC_CARD_EVENT  				(NANDIE_BASE+(0x05<<NFIE_OffShift))
#define NC_CARD_INT_EN 				(NANDIE_BASE+(0x06<<NFIE_OffShift))
#define NC_CARD_DET    				(NANDIE_BASE+(0x07<<NFIE_OffShift))
#define NC_CARD_PWR    				(NANDIE_BASE+(0x08<<NFIE_OffShift))
#define NC_INTR_TEST   				(NANDIE_BASE+(0x09<<NFIE_OffShift))
#define NC_MIE_PATH_CTL				(NANDIE_BASE+(0x0A<<NFIE_OffShift))
#define NC_JOB_BL_CNT  				(NANDIE_BASE+(0x0B<<NFIE_OffShift))
#define NC_TR_BK_CNT   				(NANDIE_BASE+(0x0C<<NFIE_OffShift))
#if defined(CONFIG_Titania3)
#define RIU2NAND_STAT				(NANDIE_BASE+(0x1F<<NFIE_OffShift))
#define RIU2NAND_CTL 				(NANDIE_BASE+(0x20<<NFIE_OffShift))
#define RIU2NAND_R   				(NANDIE_BASE+(0x21<<NFIE_OffShift))
#define RIU2NAND_W   				(NANDIE_BASE+(0x22<<NFIE_OffShift))
#endif
#define NC_CIF_FIFO_CTL 			(NANDIE_BASE+(0x25<<NFIE_OffShift))
#define NC_TEST_MODE    			(NANDIE_BASE+(0x30<<NFIE_OffShift))
#define NC_MISC         (NANDIE_BASE+(0x31<<NFIE_OffShift))
#if defined(CONFIG_Titania3)
#define NC_FCIE2_VERSION			(NANDIE_BASE+(0x3F<<NFIE_OffShift))
#endif
// NAND Flash controler register map
#define NC_CFG0      				(NANDIE_BASE+(0x40<<NFIE_OffShift))
#define NC_CFG1      				(NANDIE_BASE+(0x41<<NFIE_OffShift))
#define NC_STAT      				(NANDIE_BASE+(0x42<<NFIE_OffShift))
#define NC_AUXR_ADR  				(NANDIE_BASE+(0x43<<NFIE_OffShift))
#define NC_AUXR_DAT  				(NANDIE_BASE+(0x44<<NFIE_OffShift))
#define NC_CTRL      				(NANDIE_BASE+(0x45<<NFIE_OffShift))
#define NC_ST_READ   				(NANDIE_BASE+(0x46<<NFIE_OffShift))
#define NC_LUT_ACCESS				(NANDIE_BASE+(0x47<<NFIE_OffShift))
#define NC_LUT_DATA  				(NANDIE_BASE+(0x48<<NFIE_OffShift))
#define NC_LUT_CTRL  				(NANDIE_BASE+(0x49<<NFIE_OffShift))
#define NC_ADDEND    				(NANDIE_BASE+(0x4A<<NFIE_OffShift))

#if defined(CONFIG_Titania3)
#define CIFIFO_CMD_ADDR				(NANDIE_BASE+(0xC0<<NFIE_OffShift))
#else
#define CIFIFO_CMD_ADDR				(NANDIE_BASE+(0x60<<NFIE_OffShift))
#endif

// fcie register constant
//============================================
//NC_MIE_EVENT:offset 0x0
//============================================
#if defined(CONFIG_Titania3)
#define R_NC_JOB_END     			0x0200
#else
#define R_NC_JOB_END     			0x0010
#endif
#define R_NC_MMA_DATA_END			0x0001

//============================================
//MIE_INT_EN:offset 0x1
//============================================
#if defined(CONFIG_Titania3)
#define R_NC_JOB_ENDE        		0x0200
#else
#define R_NC_JOB_ENDE   			0x0010
#endif
#define R_NC_MMA_DATA_ENDE 			0x0001

//============================================
//NC_MMA_PRI_REG:offset 0x2
//============================================
#define R_FIFO_CLKRDY				0x0020
#define R_NC_JOB_RW_DIR        		0x0004
#define R_MMA_W_PRIORITY			0x0002
#define R_MMA_R_PRIORITY			0x0001

//============================================
//NC_MIU_DMA1:offset 0x3
//============================================
#define R_MIU1_SELECT				0x0200

//============================================
//NC_MIU_DMA0:offset 0x4
//============================================

//============================================
//NC_CARD_EVENT:offset 0x5
//============================================

//============================================
//NC_CARD_INT_EN:offset 0x6
//============================================

//============================================
//NC_CARD_DET:offset 0x7
//============================================

//============================================
//NC_CARD_PWR:offset 0x8
//============================================

//============================================
//NC_INTR_TEST:offset 0x9
//============================================
#define R_FORCE_NC_JOB_END  		0x0040
#define R_FORCE_MMA_DATA_END		0x0001

//============================================
//NC_MIE_PATH_CTL:offset 0xA
//============================================
#if defined(CONFIG_Titania3)
#define   R_NC_EN					0x20
#else
#define R_NC_EN     				0x0008
#endif
#define R_MMA_ENABLE				0x0001

//============================================
//NC_JOB_BL_CNT:offset 0xB
//============================================
#define R_JOB_BL_CNT7				0x0080
#define R_JOB_BL_CNT6				0x0040
#define R_JOB_BL_CNT5				0x0020
#define R_JOB_BL_CNT4				0x0010
#define R_JOB_BL_CNT3				0x0008
#define R_JOB_BL_CNT2				0x0004
#define R_JOB_BL_CNT1				0x0002
#define R_JOB_BL_CNT0				0x0001

//============================================
//TR_BK_CNT:offset 0xC
//============================================
#define R_TR_BK_CNT7				0x0080
#define R_TR_BK_CNT6				0x0040
#define R_TR_BK_CNT5				0x0020
#define R_TR_BK_CNT4				0x0010
#define R_TR_BK_CNT3				0x0008
#define R_TR_BK_CNT2				0x0004
#define R_TR_BK_CNT1				0x0002
#define R_TR_BK_CNT0				0x0001

#if defined(CONFIG_Titania3)
//============================================
//RIU2NAND_STAT:offset 0x1F
//============================================
#define R_DMA_RD4_NFC				0x0000
#define R_RIU2NAND_DIR				0x0001 // 0: RIU2NAND, 1: NAND2RIU

//============================================
//RIU2NAND_CTL:offset 0x20
//============================================
#define R_RIU2NAND_EN				0x0001
#define R_RIU2NAND_R_STR			0x0002
#define R_RIU2NAND_R_EN				0x0004
#define R_RIU2NAND_R_END			0x0008
#define R_RIU2NAND_W_STR			0x0010
#define R_RIU2NAND_W_EN				0x0020
#define R_RIU2NAND_W_END			0x0040
#define R_RIU2NAND_LB_EN			0x0100
#define R_NFIE_DLY_EN				0x0200
#define R_NFIE_DLY_L0				0x0400
#define R_NFIE_DLY_L1				0x0800
#define R_NFIE_DLY_H0				0x1000
#define R_NFIE_DLY_H1				0x2000

//============================================
//RIU2NAND_R      :offset 0x21
//============================================

//============================================
//RIU2NAND_W      :offset 0x22
//============================================
#endif

//============================================
//NC_CIF_FIFO_CTL:offset 0x25
//============================================
//#define R_CIFD_RD_REQ   0x02
//#define R_CIFC_RD_REQ   0x01

//============================================
//TEST_MODE:offset 0x30
//============================================
#if defined(CONFIG_Titania3)
#define R_NFIE_SOFT_RESETZ  		0x1000
#else
#define R_NFIE_SOFT_RESETZ  		0x0800
#endif
#define R_DEBUG_MODE2				0x0400
#define R_DEBUG_MODE1				0x0200
#define R_DEBUG_MODE0				0x0100
#define R_CIFC_BISTFAIL				0x0004
#define R_DBFB_BISTFAIL				0x0002
#define R_DBFA_BISTFAIL				0x0001
//============================================
//NC_MISC offser 0x31
//============================================
#define reg_nfie_encrypt
#define R_NC_ENCRYPT 				0x0002
#define R_NC_RW8B_EN 				0x0001
//============================================
//NC_CFG0:offset 0x40
//============================================
#if (MULTI_CE)
#define R_NC_CE_SEL1     			0x8000
#define R_NC_CE_SEL0     			0x4000
#endif
#define R_NC_DYNGATED_ON 			0x2000
#define R_NC_ECCERR_NSTOP			0x1000
#define R_NC_WP_AUTO     			0x0800
#define R_NC_WP_EN       			0x0400
#define R_NC_REDU_BYPASS 			0x0200
#define R_NC_CHK_RB_HIGH 			0x0100
#define R_NC_ONECOLADR   			0x0080    // one column address
#define R_NC_CE_AUTO     			0x0008
#define R_NC_CE_EN       			0x0004
#define R_NC_DSIZE_SEL   			0x0002    // 0:512, 1:512+16
#define R_NC_PSIZE_SEL   			0x0001    // 0:512+16, 1:2048+64

//============================================
//NC_CFG1:offset 0x41
//============================================
#define R_NC_DEB_SEL2				0x0400
#define R_NC_DEB_SEL1				0x0200
#define R_NC_DEB_SEL0				0x0100
#define R_NC_RD_LW1  				0x0080
#define R_NC_RD_LW0  				0x0040
#define R_NC_RD_HW1  				0x0020
#define R_NC_RD_HW0  				0x0010
#define R_NC_WR_LW1  				0x0008
#define R_NC_WR_LW0  				0x0004
#define R_NC_WR_HW1  				0x0002
#define R_NC_WR_HW0  				0x0001

//============================================
//NC_STAT:offset 0x42
//============================================
#define R_NC_ECC_SEC_CNT2			0x40
#define R_NC_ECC_SEC_CNT1			0x20
#define R_NC_ECC_SEC_CNT0			0x10
#define R_NC_ECC_FLAG1   			0x08
#define R_NC_ECC_FLAG0   			0x04
#define R_NC_STCK_ERRL8  			0x01

//============================================
//NC_AUXR_ADR:offset 0x43
//============================================
#define R_AUXR_ADR_WD7				0x80
#define R_AUXR_ADR_WD6				0x40
#define R_AUXR_ADR_WD5				0x20
#define R_AUXR_ADR_WD4				0x10
#define R_AUXR_ADR_WD3				0x08
#define R_AUXR_ADR_WD2				0x04
#define R_AUXR_ADR_WD1				0x02
#define R_AUXR_ADR_WD0				0x01

//============================================
//NC_AUXR_DAT:offset 0x44
//============================================

//============================================
//NC_CTRL:offset 0x45
//============================================
#define R_NC_SER_PART_CNT1			0x0200
#define R_NC_SER_PART_CNT0			0x0100
#define R_NC_SER_PART_MODE			0x0080
#define R_NC_NONEFFH_NUM1 			0x0040
#define R_NC_NONEFFH_NUM0 			0x0020
#define R_NC_ECC_BYPASS   			0x0010
#define R_NC_DIR_DSTREAM  			0x0008
#define R_NC_INST_HB_SEL  			0x0004
#define R_NC_CIFC_ACCESS  			0x0002
#define R_NC_JOB_START    			0x0001

//============================================
//NC_ST_READ:offset 0x46
//============================================

//============================================
//NC_LUT_ACCESS:offset 0x47
//============================================

//============================================
//NC_LUT_DATA:offset 0x48
//============================================

//============================================
//NC_LUT_CTRL:offset 0x49
//============================================

//============================================
//NC_ADDEND:offset 0x4A
//============================================
#define R_NC_RPT_ADR3_SEL2			0x0800
#define R_NC_RPT_ADR3_SEL1			0x0400
#define R_NC_RPT_ADR3_SEL0			0x0200
#define R_NC_RPT_ADR2_SEL2			0x0100
#define R_NC_RPT_ADR2_SEL1			0x0080
#define R_NC_RPT_ADR2_SEL0			0x0040
#define R_NC_RPT_ADR1_SEL2			0x0020
#define R_NC_RPT_ADR1_SEL1			0x0010
#define R_NC_RPT_ADR1_SEL0			0x0008
#define R_NC_RPT_ADR0_SEL2			0x0004
#define R_NC_RPT_ADR0_SEL1			0x0002
#define R_NC_RPT_ADR0_SEL0			0x0001

//============================================
//REG0008:offset 0x08
//============================================
#if defined(CONFIG_Titania3)
#define NC_NFIE_CLK_SHIFT   		0x00
#else
#define NC_NFIE_CLK_SHIFT   		0x08
#endif
#if defined(CONFIG_Titania3)
#define NC_CLK_SEL3     			((U16)0x20<<NC_NFIE_CLK_SHIFT)
#endif
#define NC_CLK_SEL2					((U16)0x10<<NC_NFIE_CLK_SHIFT)
#define NC_CLK_SEL1					((U16)0x08<<NC_NFIE_CLK_SHIFT)
#define NC_CLK_SEL0					((U16)0x04<<NC_NFIE_CLK_SHIFT)
#define R_INVERT_CLOCK  			((U16)0x02<<NC_NFIE_CLK_SHIFT)
#define R_DISABLE_CLOCK 			((U16)0x01<<NC_NFIE_CLK_SHIFT)

#if defined(CONFIG_Titania3)
#define NC_CLK_MASK 				((U16)(NC_CLK_SEL3+NC_CLK_SEL2+NC_CLK_SEL1+NC_CLK_SEL0))
#define NC_CLK_300K					(0)
#define NC_CLK_750K					(NC_CLK_SEL0)
#define NC_CLK_5M					(NC_CLK_SEL1)
#define NC_CLK_13M					(NC_CLK_SEL1+NC_CLK_SEL0)
#define NC_CLK_18M					(NC_CLK_SEL2)
#define NC_CLK_22M					(NC_CLK_SEL2+NC_CLK_SEL0)
#define NC_CLK_27M					(NC_CLK_SEL2+NC_CLK_SEL1)
#define NC_CLK_32M					(NC_CLK_SEL2+NC_CLK_SEL1+NC_CLK_SEL0)
#define NC_CLK_43M					(NC_CLK_SEL3)
#define NC_CLK_54M					(NC_CLK_SEL3+NC_CLK_SEL0)
#define NC_CLK_72M					(NC_CLK_SEL3+NC_CLK_SEL1)
#define NC_CLK_SSC					(NC_CLK_SEL3+NC_CLK_SEL1+NC_CLK_SEL0)
#define NC_CLK_86M					(NC_CLK_SEL3+NC_CLK_SEL2)
#else
#define NC_CLK_MASK     			((U16)(NC_CLK_SEL2+NC_CLK_SEL1+NC_CLK_SEL0))
#define NC_CLK_62M					(0)
#define NC_CLK_54M					(NC_CLK_SEL0)
#define NC_CLK_43M					(NC_CLK_SEL1)
#define NC_CLK_36M					(NC_CLK_SEL1+NC_CLK_SEL0)
#define NC_CLK_27M					(NC_CLK_SEL2)
#define NC_CLK_18M					(NC_CLK_SEL2+NC_CLK_SEL0)
#define NC_CLK_13M					(NC_CLK_SEL2+NC_CLK_SEL1)
#endif

//============================================
//  AUX Reg Address
//============================================
#define AUXADR_USERCMD      		0x08
#define AUXADR_ADRSET       		0x0B
#define AUXADR_INSTQUE      		0x20
#define AUXADR_RPTCNT       		0x18
#define AUXADR_RAN_CNT      		0x19
#define AUXADR_RAN_OFFSET   		0x1A
#define AUXADR_STATUS_MASK  		0x1B
#define AUXADR_WAIT_IDLE_CNT		0x1C

//============================================
//  OP_CODE
//============================================
#define OP_CODE_CMD 				0x00
#define OP_CODE_ADDR				0x40
#define OP_CODE_ACT 				0x80

//============================================
//  OP_CODE: command
//============================================
#define OP_CMD_WIRE                	0x00
#define OP_CMD_REG                 	0x10

#define OP_CMD00_READ              	0x00
#define OP_CMD30_READ              	0x01
#define OP_CMD35_READ4COPYBACK     	0x02
#define OP_CMD90_READID            	0x03
#define OP_CMDFF_RESET             	0x04
#define OP_CMD80_SERIAL_DATA_INPUT 	0x05
#define OP_CMD10_TRUE_PROGRAM      	0x06
#define OP_CMD15_CACHE_PROGRAM     	0x07
#define OP_CMD85_COPY_BACK_INPUT   	0x08
#define OP_CMD85_RANDOM_DATA_INPUT 	0x08
#define OP_CMD60_BLOCK_ERASE       	0x09
#define OP_CMDD0_BLOCK_ERASE       	0x0A
#define OP_CMD05_RANDOM_DATA_OUTPUT	0x0B
#define OP_CMDE0_RANDOM_DATA_OUTPUT	0x0C
#define OP_CMD70_READ_STATUS       	0x0D
#define OP_CMD50_READ_RDD          	0x0E
#define OP_CMD01_READHALF          	0x0F
//============================================
//  OP_CODE: address
//============================================
// CYCLE
#define OP_ADR_2CYCLE   			0x00
#define OP_ADR_3CYCLE   			0x10
#define OP_ADR_4CYCLE   			0x20
#define OP_ADR_5CYCLE   			0x30
//TYPE
#define OP_ADR_COL_ADDR 			0x00
#define OP_ADR_ROW_ADDR 			0x04
#define OP_ADR_FULL_ADDR			0x08
#define OP_ADR_ADDR_1CYC			0x0C
//SELECT
#define OP_ADR_SET0     			0x00
#define OP_ADR_SET1     			0x01
#define OP_ADR_SET2     			0x02
#define OP_ADR_SET3     			0x03

#define OP_ADR_C5TFS0				(OP_CODE_ADDR+OP_ADR_5CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET0)
#define OP_ADR_C5TFS1				(OP_CODE_ADDR+OP_ADR_5CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET1)
#define OP_ADR_C5TFS2				(OP_CODE_ADDR+OP_ADR_5CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET2)
#define OP_ADR_C5TFS3				(OP_CODE_ADDR+OP_ADR_5CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET3)
#define OP_ADR_C4TFS0				(OP_CODE_ADDR+OP_ADR_4CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET0)
#define OP_ADR_C4TFS1				(OP_CODE_ADDR+OP_ADR_4CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET1)
#define OP_ADR_C4TFS2				(OP_CODE_ADDR+OP_ADR_4CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET2)
#define OP_ADR_C4TFS3				(OP_CODE_ADDR+OP_ADR_4CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET3)
#define OP_ADR_C3TFS0				(OP_CODE_ADDR+OP_ADR_3CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET0)
#define OP_ADR_C3TFS1				(OP_CODE_ADDR+OP_ADR_3CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET1)
#define OP_ADR_C3TFS2				(OP_CODE_ADDR+OP_ADR_3CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET2)
#define OP_ADR_C3TFS3				(OP_CODE_ADDR+OP_ADR_3CYCLE+OP_ADR_FULL_ADDR+OP_ADR_SET3)

#define OP_ADR_C5TRS0				(OP_CODE_ADDR+OP_ADR_5CYCLE+OP_ADR_ROW_ADDR+OP_ADR_SET0)
#define OP_ADR_C5TRS1				(OP_CODE_ADDR+OP_ADR_5CYCLE+OP_ADR_ROW_ADDR+OP_ADR_SET1)
#define OP_ADR_C4TRS0				(OP_CODE_ADDR+OP_ADR_4CYCLE+OP_ADR_ROW_ADDR+OP_ADR_SET0)
#define OP_ADR_C4TRS1				(OP_CODE_ADDR+OP_ADR_4CYCLE+OP_ADR_ROW_ADDR+OP_ADR_SET1)
#define OP_ADR_C3TRS0				(OP_CODE_ADDR+OP_ADR_3CYCLE+OP_ADR_ROW_ADDR+OP_ADR_SET0)
#define OP_ADR_C3TRS1				(OP_CODE_ADDR+OP_ADR_3CYCLE+OP_ADR_ROW_ADDR+OP_ADR_SET1)
#define OP_ADR_C2TRS0				(OP_CODE_ADDR+OP_ADR_2CYCLE+OP_ADR_ROW_ADDR+OP_ADR_SET0)
#define OP_ADR_C2TRS1				(OP_CODE_ADDR+OP_ADR_2CYCLE+OP_ADR_ROW_ADDR+OP_ADR_SET1)

#define OP_ADR_C4TCS0				(OP_CODE_ADDR+OP_ADR_4CYCLE+OP_ADR_COL_ADDR+OP_ADR_SET0)
#define OP_ADR_C2TCS0				(OP_CODE_ADDR+OP_ADR_2CYCLE+OP_ADR_COL_ADDR+OP_ADR_SET0)
//Address
#define ADR_C4TCS0					0x60
#define ADR_C2TRS0					0x44
#define ADR_C4TFS0					0x68
#define ADR_C2TIS0					0x4C

//============================================
//  OP_CODE: Action
//============================================
//TYPE
#define OP_ACT_SYSTEM    			0x00
#define OP_ACT_DMA_BUF_IO			0x10
#define OP_ACT_QUEUE_CTRL			0x20
#define OP_ACT_REPEAT    			0x30
//SELECT
#define OP_ACT_WAIT_READY			0x00
#define OP_ACT_CHK_STATUS			0x01
#define OP_ACT_WAIT_IDLE 			0x02
#define OP_ACT_WAIT_MMA  			0x03
#define OP_ACT_BREAK     			0x08

#define OP_ACT_SERIAL_DATA_OUT		0x00
#define OP_ACT_RANDOM_DATA_OUT		0x01
#define OP_ACT_WRITE_RDD      		0x02
#define OP_ACT_SERIAL_DATA_IN 		0x08
#define OP_ACT_RANDOM_DATA_IN 		0x09
#define OP_ACT_READ_RDD       		0x0A

#define OP_ACT_PAGECOPY_US			0x00
#define OP_ACT_PAGECOPY_DS			0x01

#define OP_ACT_INSTRUCT_ADDR		0x00

#define OP_A_WAIT_READY_BUSY		(OP_CODE_ACT+OP_ACT_SYSTEM+OP_ACT_WAIT_READY)
#define OP_A_CHECK_STATUS   		(OP_CODE_ACT+OP_ACT_SYSTEM+OP_ACT_CHK_STATUS)
#define OP_A_WAIT_IDLE      		(OP_CODE_ACT+OP_ACT_SYSTEM+OP_ACT_WAIT_IDLE)
#define OP_A_WAIT_MMA       		(OP_CODE_ACT+OP_ACT_SYSTEM+OP_ACT_WAIT_MMA)
#define OP_A_BREAK          		(OP_CODE_ACT+OP_ACT_SYSTEM+OP_ACT_BREAK)

#define OP_A_SERIAL_DATA_OUT		(OP_CODE_ACT+OP_ACT_DMA_BUF_IO+OP_ACT_SERIAL_DATA_OUT)
#define OP_A_RANDOM_DATA_OUT		(OP_CODE_ACT+OP_ACT_DMA_BUF_IO+OP_ACT_RANDOM_DATA_OUT)
#define OP_A_WRITE_RDD      		(OP_CODE_ACT+OP_ACT_DMA_BUF_IO+OP_ACT_WRITE_RDD)
#define OP_A_SERIAL_DATA_IN 		(OP_CODE_ACT+OP_ACT_DMA_BUF_IO+OP_ACT_SERIAL_DATA_IN)
#define OP_A_RANDOM_DATA_IN 		(OP_CODE_ACT+OP_ACT_DMA_BUF_IO+OP_ACT_RANDOM_DATA_IN)
#define OP_A_READ_RDD       		(OP_CODE_ACT+OP_ACT_DMA_BUF_IO+OP_ACT_READ_RDD)

#define OP_A_PAGECOPY_US    		(OP_CODE_ACT+OP_ACT_QUEUE_CTRL+OP_ACT_PAGECOPY_US)
#define OP_A_PAGECOPY_DS    		(OP_CODE_ACT+OP_ACT_QUEUE_CTRL+OP_ACT_PAGECOPY_DS)

#define OP_A_REPEAT         		(OP_CODE_ACT+OP_ACT_REPEAT)
//============================================
// NAND_FLASH_INFO.eFlashConfig
//============================================

//============================================
// NAND Flash capacity
//============================================
#define NAND_DISK_4M  				0x1F3F
#define NAND_DISK_8M  				0x3E7F
#define NAND_DISK_16M 				0x7CFF
#define NAND_DISK_32M 				0xF9FF
#define NAND_DISK_64M 				0x1F3FF
#define NAND_DISK_128M				0x3E7FF
#define NAND_DISK_256M				0x7CFFF
#define NAND_DISK_512M				0xF9FFF
#define NAND_DISK_1G  				0x1F3FFF
#define NAND_DISK_2G  				0x3E7FFF
//============================================
// LUT ram constant
//============================================
#define LUT_RAM    					0x0000
#define FREELUT_RAM					0x03E8

#ifdef NAND_1KLUT
#define MAX_PHYS_BLK				512
#define MAX_LOGI_BLK				500
#else
#define MAX_PHYS_BLK				1024
#define MAX_LOGI_BLK				1000
#endif
#define MAX_FREE_BLK  				(MAX_PHYS_BLK-MAX_LOGI_BLK)
#define LUT_SIZE      				MAX_PHYS_BLK

//============================================
// Free block constant
//============================================
#define ERASED_BLOCK     0x8000
//============================================

//============================================
// TYPES
//============================================
typedef enum EBOUNDINGTYPE
{
	MSPD1010B = 0,
	MSPD1111B = 2,
	MSPD2112B = 1,
	MSPD3222B = 3
}BOUNDINGTYPE;

#define	U8 							unsigned char
#define	U16							unsigned short int
#define	U32							unsigned long int

#define MIUWIDTH    				8
#define NC_SECTOR_SIZE 				0x200

#if (defined(ENABLE_LINUX_MTD) && (ENABLE_LINUX_MTD==1))
typedef struct{
	U8 sector[PAGE_SECTOR_SIZE];
	U8 Rdd[PAGE_RDD_SIZE];
}NAND_PAGE;

typedef union nand_page_buffer{
	U8 au8data[NFIE_PAGE_SIZE];
	NAND_PAGE page;
}NAND_PAGE_BUFFER;
#endif

typedef enum EFLASHCONFIG
{
	FLASH_UNCONFIG 				= 0x0,
	FLASH_READY 				= 0x01,
	FLASH_WP 					= 0x02,
	FLASH_2KPAGE				= 0x04,
	FLASH_16BIT 				= 0x08,
	FLASH_DUALMODE 				= 0x10,
	FLASH_CACHEPROGRAM 			= 0x20,
	FLASH_COPYBACK 				= 0x40,
	FLASH_MEDIACHANGE 			= 0x80
}FLASHCONFIG;

typedef enum EFLASHTYPE
{
	TYPE_SLC 				= 0,
	TYPE_MLC,
	TYPE_AGAND
}FLASHTYPE;

typedef enum EFLASHPLANE
{
	ONE_PLANE 				= 0,
	TWO_PLANE,
	FOUR_PLANE,
	EIGHT_PLANE,
}FLASHPLANE;

typedef enum ENCRW_STATUS
{
	NCRW_STS_OK 			= 0,
	NCRW_STS_NOMEDIA 		= 0x1,
	NCRW_STS_PROGRAM_FAIL 	= 0x2,
	NCRW_STS_ECC_ERROR 		= 0x4,
	NCRW_STS_WRITE_PROTECT 	= 0x8,
	NCRW_STS_TIMEOUT 		= 0x10,
	NCRW_STS_ADDRESS_ERROR 	= 0x20
}NCRW_STATUS;

typedef enum ENC_NAND_STATUS
{
	NC_STS_READY 			= 0,
	NC_STS_MEDIA_CHANGE,
	NC_STS_NOT_READY
}NC_NAND_STATUS;

typedef enum EMAKER_CODE
{
	SAMSUMG					= 0xEC,
	TOSHIBA					= 0x98,
	RENESAS					= 0x07,
	HYNIX  					= 0xAD,
	ST     					= 0x20
}MAKER_CODE;

typedef enum _RW_DIR
{
	RW_READ = 0,
	RW_WRITE
}RW_DIRECTION;

typedef struct nand_operations_struct
{
	NCRW_STATUS (*AUTOSCAN)(void);
	NCRW_STATUS (*READ_CIS)(void);
#ifdef NAND_1KLUT
	void (*BUILD_LUT)(U8,bool);
#else
	void (*BUILD_LUT)(U8);
#endif
	NCRW_STATUS (*NAND_RW)(RW_DIRECTION ,U32 ,U32,U32);
}nand_operations;

typedef struct _FREEBLOCKQUEUE
{
	U8 Head;
	U8 Tail;
	U8 Count;
}FREEBLOCKQUEUE;

#if (defined(ENABLE_LINUX_MTD) && (ENABLE_LINUX_MTD==1))
typedef enum eHW_CONTROL
{
	HW_CLE = 0x01,
	HW_ALE = 0x02,
}HW_CONTROL;
#endif

typedef struct _NAND_FLASH_INFO
{
	U16 u16NC_CFG0;
	FLASHCONFIG eFlashConfig;
#if MULTI_CE
	MAKER_CODE eMaker[MAX_CE_NO];
	U8 u8ID[MAX_CE_NO];
#else
	MAKER_CODE eMaker;
	U8 u8ID;
#endif
	U8 u8PagesPerBlock;
	U8 u8SecNumofBlk;
	U8 u8Pageshift;
	U8 u8AddrWidth;
	U8 u8Speed;
	U16 u16BlocksPerCS;
	U32 u32Capacity;
	FLASHTYPE eflashtype;
	FLASHPLANE eflashplane;

	U8 u8CIS_BLOCK;
#if (MULTI_CE)
	U8 u8TotalCE;
	U8 u8CurCE;
#endif
	U8 u8CurZone;
	U8 bCurField;
	U8 WriteBack;
	U32 u32PreSource[2];
	U32 u32PreFree[2];
	U16 u16PreBlock;
	U16 u16PreSector;
	U16 u16ReservedBlocks;  // reserved for image & user setting
	U8 u8WaitEvent;
	struct _FREEBLOCKQUEUE FreeQueue;
	struct nand_operations_struct *nand_op;
	U8 u8BOOT_MODE;
#if (defined(ENABLE_LINUX_MTD) && (ENABLE_LINUX_MTD==1))
	HW_CONTROL ehw_ctl;
	U8 u8AddressCnt;
#endif
}NAND_FLASH_INFO;

typedef union nand_lut
{
	U16 pu16LUT[MAX_PHYS_BLK];
	struct pu16lut_4plan
	{
		U16 pu16LUT0[MAX_PHYS_BLK/4];
		U16 pu16LUT1[MAX_PHYS_BLK/4];
		U16 pu16LUT2[MAX_PHYS_BLK/4];
		U16 pu16LUT3[MAX_PHYS_BLK/4];
	}pu16LUT_4PLAN;
}NAND_LUT;

/*--------------------------------------------------------------------
  MEMORY CONSTANTS
  --------------------------------------------------------------------*/

/*--------------------------------------------------------------------
  VARIABLES
  --------------------------------------------------------------------*/
extern NAND_FLASH_INFO _fsinfo;
extern volatile U32 *_pu32CIF_C;

/*--------------------------------------------------------------------
  MACROS
  --------------------------------------------------------------------*/
#define HAL_READ_UINT16(X,Y)				HAL_READ_UINT16_IMPL(X, &Y)

/*--------------------------------------------------------------------
  PROCEDURES
  --------------------------------------------------------------------*/
#if (ENABLE_NAND)
void drvNAND_ClearNFIE_EVENT(void);
void drvNAND_SoftReset(void);
void drvNAND_FLASHRESET(void);
int drvNAND_FLASH_INIT(void);
void drvNAND_READ_ID(void);
NCRW_STATUS drvNAND_wNCJOBEND(bool wMMA_END);
NCRW_STATUS drvNAND_wNCMMAEND(void);

U8 drvNAND_GetCMD_RSP_BUF(U8 u8addr);
void drvNAND_SetCMD_RSP_BUF(U8 u8addr, U8 u8value);

void drvNAND_ReadRdd(U32 u32Row, U16 u16Column);
void drvNAND_ReadPage(U32 u32Row,U16 u16Col);
void drvNAND_WritePage(U32 u32Row,U16 u16Col);
U8 drvNAND_EraseBlock(U32 u32Row);

void *UTL_memset(void *pDst, signed int u8Ch, unsigned int u32Cnt);
void HAL_WRITE_UINT16(U32 u32REG,U16 u16dat);
U16 HAL_READ_UINT16_IMPL(U32 addr, U16* regval);

#if (defined(ENABLE_LINUX_MTD) && (ENABLE_LINUX_MTD==1))
U8 MDrv_NAND_ReadByte(void);
NCRW_STATUS MDrv_NAND_ReadPage(U32 u32Row,U16 u16Column);
void MDrv_NAND_ReadBuf(U8 * const u8Buf,U16 u16Len);
U8 MDrv_NAND_Send_Read_Status_CMD(U8 u8Cmd);
void MDrv_NAND_Cmd_PageProgram(U8 u8Cmd);
void MDrv_NAND_WriteBuf(U8 * const u8Buf,U16 u16Len);
NCRW_STATUS MDrv_NAND_WritePage(U32 u32Row,U16 u16Column);
void MDrv_NAND_ReadOOB(U32 u32Row,U16 u16Column);
U8 MDrv_NAND_Set_Command_Latch(U8 u8Level);
U8 MDrv_NAND_Set_Address_Latch(U8 u8Level);
U8 MDrv_NAND_Set_Write_Protect(U8 u8Level);
NCRW_STATUS MDrv_NAND_SendCmd(U8 u8Cmd);
NCRW_STATUS MDrv_NAND_SendAdr(U32 u32Row,U16 u16Column);
NCRW_STATUS MDrv_NAND_SendCmdAdr(U8 u8Cmd,U32 u32Row,U16 u16Column);
U8 MDrv_NAND_WaitReady(void);
void MDrv_NAND_Erase1(U32 u32Row);
bool MDrv_NAND_Erase2(void);
U8 MDrv_NAND_Set_Chip_Select(U8 u8Level);
U8 MDrv_NAND_CheckECC(void);
U16 MDrv_NAND_Set_Power_Off(U8 OnOff_flag);
#endif

#endif // #if (ENABLE_NAND)
#endif
