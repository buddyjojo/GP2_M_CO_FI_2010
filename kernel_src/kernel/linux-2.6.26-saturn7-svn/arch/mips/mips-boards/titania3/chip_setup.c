/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.  All rights reserved.
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
 */
#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/tty.h>

#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>


#ifdef CONFIG_MTD
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#endif

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mips-boards/generic.h>
#include <asm/mips-boards/prom.h>
#include <asm/dma.h>
#include <asm/time.h>
#include <asm/traps.h>
#include <linux/poll.h>
#include <linux/wait.h>

#include "chip_int.h"
#include "chip_setup.h"
#include "mhal_chiptop_reg.h"
#include "mhal_pmsleep_reg.h"

#ifdef CONFIG_VT
#include <linux/console.h>
#endif

#include "board/Board.h"
#include "board/chip/bond.h"

#if defined(CONFIG_MSTAR_TITANIA_BD_S5_LG_DEMO_BOARD_1)
#define USE_LGE_DEMO_BOARD
#endif

#define MIU_SETTING_FOR_BRINGUP			0

// start: the following code is ported from Yoga\Chakra2\core\driver\sys\s7\srvPadConf.c
#define GPIO_NONE    					0       // Not GPIO pin (default)
#define GPIO_IN      					1       // GPI
#define GPIO_OUT_LOW 					2       // GPO output low
#define GPIO_OUT_HIGH					3       // GPO output high

#define ENABLE_PCM_GPIO					1
#define ENABLE_NOR_GPIO					0
#define ENABLE_PF_GPIO 					1
// end:

const char display_string[] = "        LINUX ON Titania 3       ";

/******************************************************************************/
/* MEMORY SETTING                                                             */
/******************************************************************************/
//dhjung LGE
//const unsigned long LINUX_MEMORY_ADDRESS=LINUX_MEM2_BASE_ADR;
//const unsigned long LINUX_MEMORY_LENGTH=LINUX_MEM2_LEN;

/******************************************************************************/
/* Macro for bitwise                                                          */
/******************************************************************************/
#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)

#define BIT0							0x01
#define BIT1							0x02
#define BIT2							0x04
#define BIT3							0x08
#define BIT4							0x10
#define BIT5							0x20
#define BIT6							0x40
#define BIT7							0x80

#if 0
//------------------------------------------------------------------------------
// PM_SLEEP_XIU assignment
//------------------------------------------------------------------------------
#define REG_PMSLEEP_BASE 					0xBF001C00
#define PMSLEEP_REG(addr)					(*((volatile u32*)(REG_PMSLEEP_BASE + ((addr)<<2))))

#define REG_CKG_MCU                          0x0020
#define PMSLEEP_CKG_MCU_MASK                 BMASK(1:0)
#define PMSLEEP_CKG_MCU_DIS                  _BIT(0)
#define PMSLEEP_CKG_MCU_INV                  _BIT(1)
#define PMSLEEP_CKG_MCU_SRC_MASK             BMASK(5:2)
#define PMSLEEP_CKG_MCU_SRC_170              BITS(5:2, 0)
#define PMSLEEP_CKG_MCU_SRC_160              BITS(5:2, 1)
#define PMSLEEP_CKG_MCU_SRC_144              BITS(5:2, 2)
#define PMSLEEP_CKG_MCU_SRC_123              BITS(5:2, 3)
#define PMSLEEP_CKG_MCU_SRC_108              BITS(5:2, 4)
#define PMSLEEP_CKG_MCU_SRC_MEM_CLK          BITS(5:2, 5)
#define PMSLEEP_CKG_MCU_SRC_MEM_CLK_DIV2     BITS(5:2, 6)
#define PMSLEEP_CKG_MCU_SRC_INT_XTALI        BITS(5:2, 7)
#define PMSLEEP_CKG_MCU_SRC_EXT_XTALI_DIV8   BITS(5:2, 8)
#define PMSLEEP_CKG_MCU_SRC_24               BITS(5:2, 9)
#define PMSLEEP_CKG_MCU_SRC_INT_XTALI_DIV_1M BITS(5:2, 10)
#define PMSLEEP_CKG_MCU_SRC_EXT_XTALI_DIV16  BITS(5:2, 11)
#define PMSLEEP_CKG_MCU_SRC_EXT_XTALI_DIV2   BITS(5:2, 12)
#define PMSLEEP_CKG_MCU_SRC_EXT_XTALI_DIV4   BITS(5:2, 13)
#define PMSLEEP_CKG_MCU_SRC_240              BITS(5:2, 14)
#define PMSLEEP_CKG_MCU_SRC_192              BITS(5:2, 15)
#define PMSLEEP_CKG_SW_MCU_CLK               _BIT(7)
#endif

//------------------------------------------------------------------------------
// MIU assignment   0: MIU0, 1: MIU1
//------------------------------------------------------------------------------
#if defined(CONFIG_MSTAR_TITANIA2)
#define MIUSEL_FLAG_FSEICH_R        0
#define MIUSEL_FLAG_SVD_DB_R        1
#define MIUSEL_FLAG_SVD_DB_W        1
#define MIUSEL_FLAG_FDECICH_R       0
#define MIUSEL_FLAG_OD_LSB_R        0
#define MIUSEL_FLAG_OD_LSB_W        0
#define MIUSEL_FLAG_GOP2_R          0

#define MIUSEL_FLAG_OPM_R           0
#define MIUSEL_FLAG_MVOP_R          1
#define MIUSEL_FLAG_GOP0_W          1
#define MIUSEL_FLAG_GOP0_R          1
#define MIUSEL_FLAG_GOP1_R          0
#define MIUSEL_FLAG_DNRB_W          0
#define MIUSEL_FLAG_DNRB_R          0
#define MIUSEL_FLAG_COMB_R          0
#define MIUSEL_FLAG_COMB_W          0
#define MIUSEL_FLAG_VE_R            0
#define MIUSEL_FLAG_VE_W            0
#define MIUSEL_FLAG_OD_R            0
#define MIUSEL_FLAG_OD_W            0
#define MIUSEL_FLAG_FSEDMA2_RW      0
#define MIUSEL_FLAG_MAD_RW          0

#define MIUSEL_FLAG_SVD_EN_R        1
#define MIUSEL_FLAG_MVD_R           1   // shared by RVD_RW, SVDINTP_R, MVD_R
#define MIUSEL_FLAG_AESDMA_W        0
#define MIUSEL_FLAG_AESDMA_R        0
#define MIUSEL_FLAG_MVD_W           1
#define MIUSEL_FLAG_JPD_R           0
#define MIUSEL_FLAG_JPD_W           0
#endif


//#if defined(CONFIG_MSTAR_TITANIA3)

#if 1
// Group 0
#define MIUSEL_FLAG_VIVALDI_DECODER_R   0      //#define MIUSEL_FLAG_FSEICH_R        0
#define MIUSEL_FLAG_VIVALDI_SE_R        0      //#define MIUSEL_FLAG_SVD_DB_R        1
#define MIUSEL_FLAG_MIPS_W              0      //#define MIUSEL_FLAG_SVD_DB_W        1
#define MIUSEL_FLAG_MIPS_R              0      //#define MIUSEL_FLAG_FDECICH_R       0
#define MIUSEL_FLAG_PIU_MAU0_W          0      //#define MIUSEL_FLAG_OD_LSB_R        0
#define MIUSEL_FLAG_PIU_MAU1_R          0      //#define MIUSEL_FLAG_OD_LSB_W        0
#define MIUSEL_FLAG_VD_MHEG5_DCACHE_RW  0      //#define MIUSEL_FLAG_GOP2_R          0
#define MIUSEL_FLAG_VD_MHEG5_QDMA_RW    0
#define MIUSEL_FLAG_GE_RW               1      // GE will decide by itself
#define MIUSEL_FLAG_HVD_RW              1
#define MIUSEL_FLAG_HVD_BBU_R           1
#define MIUSEL_FLAG_USB_UHC0_RW         0
#define MIUSEL_FLAG_USB_UHC1_RW         0
#define MIUSEL_FLAG_PIU_BDMA_RW         0
// Group 1
#define MIUSEL_FLAG_VIVALDI_MAD_RW      0     //#define MIUSEL_FLAG_OPM_R           0
#define MIUSEL_FLAG_VIVALDI_DMA_RW      0     //#define MIUSEL_FLAG_MVOP_R          1
#define MIUSEL_FLAG_VIVALDI_AUDMA_RW    0     //#define MIUSEL_FLAG_GOP0_W          1
#define MIUSEL_FLAG_MVOP_R              1     //#define MIUSEL_FLAG_GOP0_R          1
#define MIUSEL_FLAG_GOP_W               1     //#define MIUSEL_FLAG_GOP1_R          0
#define MIUSEL_FLAG_GOP0_R              0     //#define MIUSEL_FLAG_DNRB_W          0
#define MIUSEL_FLAG_GOP1_R              0     //#define MIUSEL_FLAG_DNRB_R          0
#define MIUSEL_FLAG_GOP2_R              1     //#define MIUSEL_FLAG_COMB_R          0
#define MIUSEL_FLAG_GOP3_R              0     //#define MIUSEL_FLAG_COMB_W          0

#define MIUSEL_FLAG_SC_OP_R             0     //#define MIUSEL_FLAG_VE_R            0
#define MIUSEL_FLAG_SC_IPMAIN_W         0     //#define MIUSEL_FLAG_VE_W            0
#define MIUSEL_FLAG_SC_IPMAIN_R         0     //#define MIUSEL_FLAG_OD_R            0

#define MIUSEL_FLAG_VD_COMB_W           1     //#define MIUSEL_FLAG_OD_W            0
#define MIUSEL_FLAG_VD_COMB_R           1     //#define MIUSEL_FLAG_FSEDMA2_RW      0
#define MIUSEL_FLAG_VE_W                0     //#define MIUSEL_FLAG_MAD_RW          0
#define MIUSEL_FLAG_VE_R                0
//GROUP 2
#define MIUSEL_FLAG_OD_W                1     //#define MIUSEL_FLAG_SVD_EN_R        1
#define MIUSEL_FLAG_OD_R                1     //#define MIUSEL_FLAG_MVD_R           1   // shared by RVD_RW, SVDINTP_R, MVD_R
#define MIUSEL_FLAG_OD_LSB_W            1     //#define MIUSEL_FLAG_AESDMA_W        0
#define MIUSEL_FLAG_OD_LSB_R            1     //#define MIUSEL_FLAG_AESDMA_R        0
#define MIUSEL_FLAG_SC_IPSUB_W          1     //#define MIUSEL_FLAG_MVD_W           1
#define MIUSEL_FLAG_SC_IPSUB_R          1     //#define MIUSEL_FLAG_JPD_R           0
#define MIUSEL_FLAG_DISP_IPATH_NR_RW    1     //#define MIUSEL_FLAG_JPD_W           0
#define MIUSEL_FLAG_DISP_IPATH_MR_RW    1
#define MIUSEL_FLAG_DISP_IPATH_DI_W     1
#define MIUSEL_FLAG_VD_MHEG5_ICACHE_R   0
#define MIUSEL_FLAG_TSP_W               0
#define MIUSEL_FLAG_TSP_R               0
#define MIUSEL_FLAG_VD_TTX_RW           0
#define MIUSEL_FLAG_VD_TTXSL_W          0
#define MIUSEL_FLAG_TSP_ORZ_W           0
#define MIUSEL_FLAG_TSP_ORZ_R           0
//Group 3
#define MIUSEL_FLAG_MVOP_W              1
#define MIUSEL_FLAG_M4VE_0_RW           1
#define MIUSEL_FLAG_M4VE_ME_R           1
#define MIUSEL_FLAG_M4VE_2_RW           1
#define MIUSEL_FLAG_MVD_RW              1
#define MIUSEL_FLAG_MVD_BBU_RW          1
#define MIUSEL_FLAG_RVD_RW              1
#define MIUSEL_FLAG_RVD_BBU_R           1
#define MIUSEL_FLAG_JPD_RW              1
#define MIUSEL_FLAG_DSCRMB_RW           0
#define MIUSEL_FLAG_STRLD_RW            0
#define MIUSEL_FLAG_PIU_HISPEED_UART_RW 0
#define MIUSEL_FLAG_EMAC_RW             0
#define MIUSEL_FLAG_NAND_RW             0
#define MIUSEL_FLAG_MPIF_RW             0
#define MIUSEL_FLAG_G3D_RW              0
#endif

//------------------------------------------------------------------------------

unsigned int RV_REG_BASE_ADDR = 0;

extern void mips_reboot_setup(void);
extern void mips_time_init(void);
extern void mips_timer_setup(struct irqaction *irq);
extern unsigned long mips_rtc_get_time(void);

//#define RAWUART_MAX_SIZE 4800//204800
unsigned char  uart_base[RAWUART_MAX_SIZE];
unsigned int   uart_len=RAWUART_MAX_SIZE;
unsigned int   uart_wptr=0;
unsigned int   uart_rptr=0;
unsigned int   uart_c=0;
wait_queue_head_t        uart_wait_queue;

DECLARE_MUTEX(UartSem);
EXPORT_SYMBOL(uart_base);
EXPORT_SYMBOL(uart_len);
EXPORT_SYMBOL(uart_wptr);
EXPORT_SYMBOL(uart_rptr);
EXPORT_SYMBOL(uart_c);
EXPORT_SYMBOL(uart_wait_queue);
EXPORT_SYMBOL(UartSem);


static void __init serial_init(void);
static void chiptop_init(void);
static void __init pad_config(void);
static void __init miu_priority_config(void);
static void __init miu_assignment(void);

//dhjung LGE
extern unsigned int sys_memsize;
const char *get_system_type(void)
{
	return "MStar Titania";
}

void __Titania_setup(void)
{
	printk(KERN_DEBUG "Begin Titania3_setup\n");

	// for MIU assignment
	miu_assignment();

#if defined(CONFIG_MSTAR_TITANIA3)
	// **************************************************
	// Enable MSTV tool to access MIU (for DDR3). Refer to BOOT_EnableAccessMIU.cmm in T32 script cmm file.
	*((volatile unsigned int *)0xBF005788) = 0x0004;
	*((volatile unsigned int *)0xBF00578C) = 0x3C12;
	*((volatile unsigned int *)0xBF005790) = 0x0010;
	*((volatile unsigned int *)0xBF005794) = 0x403C;
	*((volatile unsigned int *)0xBF005798) = 0x0101;
	// **************************************************

#if 0//dhjung LGE
	// Set the MIU size in MAU for AEON reference.
#if defined(CONFIG_MSTAR_TITANIA_MMAP_256MB_256MB)
	*((volatile unsigned int *)0xBF203084) |= 0x0100;
	*((volatile unsigned int *)0xBF2030C4) |= 0x0100;
#elif defined(CONFIG_MSTAR_TITANIA_MMAP_128MB_128MB)
	*((volatile unsigned int *)0xBF203084) &= ~(0x0100);
	*((volatile unsigned int *)0xBF2030C4) &= ~(0x0100);
#endif

#else
	// Set the MIU size in MAU for AEON reference.
	if (sys_memsize == E_SYS_MMAP_512MB) {
		printk(KERN_DEBUG "Set the MIU size in the 512MB DDR\n");
		*((volatile unsigned int *)0xBF203084) |= 0x0100;
		*((volatile unsigned int *)0xBF2030C4) |= 0x0100;

	} else if (sys_memsize == E_SYS_MMAP_256MB) {
		printk(KERN_DEBUG "Set the MIU size in the 256MB DDR\n");
#if 0
		*((volatile unsigned int *)0xBF203084) &= ~(0x0100);
		*((volatile unsigned int *)0xBF2030C4) &= ~(0x0100);
#else
		*((volatile unsigned int *)0xBF203084) |= 0x0100;
		*((volatile unsigned int *)0xBF2030C4) |= 0x0100;
#endif
	}
#endif
#endif

	chiptop_init();

	pad_config();

#if ! defined(UART0) //for UART2
	*(volatile unsigned int*)(0xbf203d4c)  = 0x0400;
	*(volatile unsigned int*)(0xbf203d50)  = 0x0000;
	*(volatile unsigned int*)(0xbf203c08) |= 0x0800;
	*(volatile unsigned int*)(0xbf203c08) &= 0xFBFF;
#endif

	miu_priority_config();

}


static int __init Titania_setup(void)
{

    (*(volatile unsigned char *)(0xBF2025C0))=(unsigned char)(0x0);

	__Titania_setup();
	
	serial_init();

	mips_reboot_setup();

	//board_time_init = mips_time_init;
	//board_timer_setup = mips_timer_setup;
#if 0
	rtc_get_time = mips_rtc_get_time;
#endif
	return 0;
}

// early_initcall(Titania_setup);
void __init plat_mem_setup(void)
{
	Titania_setup();
}

extern void Uart16550IntrruptEnable(void);
extern void Uart16550Put(unsigned char);
static void __init serial_init(void)
{
#ifdef CONFIG_SERIAL_8250
	struct uart_port s;

	memset(&s, 0, sizeof(s));

	s.type = PORT_16550;

#if defined(CONFIG_Titania3)
	s.iobase = 0xBF201300;
	s.uartclk = 123000000;
#endif

#if defined(CONFIG_Titania2)
	s.iobase = 0xBF801300;
#endif

	s.irq = E_IRQ_UART;


#ifdef CONFIG_MSTAR_TITANIA_BD_T3_FPGA
	s.uartclk = 12000000;
#endif

#ifdef CONFIG_MSTAR_TITANIA_BD_GP2_DEMO1
	s.uartclk = 123000000;
#endif

	s.iotype = 0;
	s.regshift = 0;

#if defined(CONFIG_Titania2)
	s.fifosize = 16 ; // use the 8 byte depth FIFO well
#endif

#if defined(CONFIG_Titania3)
	s.fifosize = 16 ; // use the 8 byte depth FIFO well
#endif

	if (early_serial_setup(&s) != 0) {
		printk(KERN_ERR "Serial setup failed!\n");
	}
#endif
}

#if 0
static void __init _RV1(unsigned int addr, unsigned short value)
{
#if defined(CONFIG_Titania3)
	if(addr & 1)
		(*(volatile unsigned char *)(0xbf200000+((addr-1)<<1)+1))=(unsigned char)(value & 0xFF);
	else
		(*(volatile unsigned char *)(0xbf200000+(addr<<1)))=(unsigned char)(value & 0xFF);
#endif

#if defined(CONFIG_Titania2)
	if(addr & 1)
		(*(volatile unsigned char *)(0xbf800000+((addr-1)<<1)+1))=(unsigned char)(value & 0xFF);
	else
		(*(volatile unsigned char *)(0xbf800000+(addr<<1)))=(unsigned char)(value & 0xFF);
#endif
}

static void __init _RV2(unsigned int addr, unsigned short value)
{
#if defined(CONFIG_Titania3)
	(*(volatile unsigned short *)(0xbf200000+(addr<<1)))=(unsigned short)(value & 0xFFFF);
#endif
#if defined(CONFIG_Titania2)
	(*(volatile unsigned short *)(0xbf800000+(addr<<1)))=(unsigned short)(value & 0xFFFF);
#endif
}

static void __init _RV3(unsigned int addr, unsigned int value)
{
#if defined(CONFIG_Titania3)
	_RV2(addr, value & 0xFFFF);
	_RV1(addr+2, (value>>16) & 0xFF);
#endif
#if defined(CONFIG_Titania2)
	_RV1(addr, value & 0xFF);
	_RV1(addr+1, (value>>8) & 0xFF);
	_RV1(addr+2, (value>>16) & 0xFF);
#endif
}

static void __init _RV4(unsigned int addr, unsigned int value)
{
#if defined(CONFIG_Titania3)
	_RV2(addr, value & 0xFFFF);
	_RV2(addr + 2, (value>>16) & 0xFFFF);
#endif
#if defined(CONFIG_Titania2)
	(*(volatile unsigned int *)(0xbf800000+(addr<<1)))=(unsigned int)value;
#endif

}

static void __init _RV1_ByteMask(unsigned int addr, unsigned char value, unsigned char u8Mask)
{
	volatile unsigned char *reg;
#if defined(CONFIG_Titania2)
	if(addr & 1)
		reg = (volatile unsigned char *)(0xbf800000+((addr-1)<<1)+1);
	else
		reg = (volatile unsigned char *)(0xbf800000+(addr<<1));
#endif

#if defined(CONFIG_Titania3)
	if(addr & 1)
		reg = (volatile unsigned char *)(0xbf200000+((addr-1)<<1)+1);
	else
		reg = (volatile unsigned char *)(0xbf200000+(addr<<1));
#endif
	*reg = ((*reg) & (~u8Mask)) | (value & u8Mask);
}

static void __init _RV1_WriteRegBit(unsigned int addr, int bBit, unsigned char u8BitPos)
{
	volatile unsigned char *reg;

#if defined(CONFIG_Titania2)
	if(addr & 1)
		reg = (volatile unsigned char *)(0xbf800000+((addr-1)<<1)+1);
	else
		reg = (volatile unsigned char *)(0xbf800000+(addr<<1));
#endif

#if defined(CONFIG_Titania3)
	if(addr & 1)
		reg = (volatile unsigned char *)(0xbf200000+((addr-1)<<1)+1);
	else
		reg = (volatile unsigned char *)(0xbf200000+(addr<<1));
#endif

	*reg = (bBit) ? ( *reg | u8BitPos) : (*reg | ~u8BitPos);
}
#endif

#if (MS_BOARD_TYPE_SEL == BD_GP2_DEMO1)
static void _RVM1(unsigned int addr, unsigned char value, unsigned char u8Mask)
{
	volatile unsigned char *reg;

	if(addr & 1)
	{
		reg = (volatile unsigned char *)(RV_REG_BASE_ADDR+((addr-1)<<1)+1);
	}
	else
	{
		reg = (volatile unsigned char *)(RV_REG_BASE_ADDR+(addr<<1));
	}
	*reg = ((*reg) & (~u8Mask)) | (value & u8Mask);
}

static void _RVM2(unsigned int addr, unsigned short int value, unsigned short int u16Mask)
{
	volatile unsigned char *reg;

	if(addr & 1)
	{
		_RVM1(addr, (value & 0xFF), (u16Mask & 0xFF));

		value = value >> 8;
		u16Mask = u16Mask >> 8;
		_RVM1(addr+1, value, u16Mask);
	}
	else
	{
		reg = (volatile unsigned char *)(RV_REG_BASE_ADDR+(addr<<1));
		*reg = ((*reg) & (~u16Mask)) | (value & u16Mask);
	}
}

static void _RVM3(unsigned int addr, unsigned int value, unsigned int u24Mask)
{
	if(addr & 1)
	{
		_RVM1(addr, (value & 0xFF), (u24Mask & 0xFF));

		value = value >> 8;
		u24Mask = u24Mask >> 8;
		_RVM2(addr + 1, value, u24Mask);
	}
	else
	{
		_RVM2(addr, (value & 0xFFFF), (u24Mask & 0xFFFF));

		value = value >> 16;
		u24Mask = u24Mask >> 16;
		_RVM1(addr + 2, value, u24Mask);
	}
}

static void _RVM4(unsigned int addr, unsigned int value, unsigned int u32Mask)
{
	if(addr & 1)
	{
		_RVM1(addr, (value & 0xFF), (u32Mask & 0xFF));

		value = value >> 8;
		u32Mask = u32Mask >> 8;
		_RVM2(addr+1, (value  & 0xFFFF), (u32Mask & 0xFFFF));

		value = value >> 16;
		u32Mask = u32Mask >> 16;
		_RVM1((addr+3), value, u32Mask);
	}
	else
	{
		_RVM2(addr, (value & 0xFFFF), (u32Mask & 0xFFFF));

		value = value >> 16;
		u32Mask = u32Mask >> 16;
		_RVM2(addr+2, value, u32Mask);
	}
}
#endif // #if (MS_BOARD_TYPE_SEL == BD_GP2_DEMO1)

static void chiptop_init(void)
{
#if defined(CONFIG_MSTAR_TITANIA2)
	_RV1(REG_CKG_DHC_SBM, CKG_DHC_SBM_12MHZ | CKG_GE_144MHZ);       // DHC_SBM, GE
	{
		volatile unsigned int a ;
		volatile unsigned int b ;

		//RobertYang, currently, Aeon can't run on 170Mhz.
		//Aeon switch to 170M

		*(volatile unsigned int*)(0xbf800000+(0x3c88)) &= ~(0x1F00) ;
		*(volatile unsigned int*)(0xbf800000+(0x4A20)) |= 0x0080 ; // 170MHZ

		// set mcu (8051) clock to enable TX empty interrupt
		*(volatile unsigned int*)(0xbf800000+(0x3c44)) |= 0x0001 ;

		// wait for a while until mcu clock stable
		for( a=0; a<0xFF; a++ )
			b = a ;
	}
#endif

#if defined(CONFIG_MSTAR_TITANIA3)

	CKLGEN0_REG(REG_CLKGEN0_GE) = CLKGEN0_CKG_GE_144MHZ;

	{
		volatile unsigned int a ;
		volatile unsigned int b ;

		//Aeon switch to 144M
		CKLGEN0_REG(REG_CLKGEN0_AEON) &= ~CLKGEN0_CKG_SW_AEON_CLK;
		CKLGEN0_REG(REG_CLKGEN0_AEON) |= CLKGEN0_CKG_AEON_SRC_170;
		CKLGEN0_REG(REG_CLKGEN0_AEON) |= CLKGEN0_CKG_SW_AEON_CLK;

		// TODO: Need to refine the following setting. It is T2.
		// set mcu (8051) clock to enable TX empty interrupt

		PMSLEEP_REG(REG_CKG_MCU) &= ~PMSLEEP_CKG_SW_MCU_CLK;
		PMSLEEP_REG(REG_CKG_MCU) |= PMSLEEP_CKG_MCU_SRC_170;
		PMSLEEP_REG(REG_CKG_MCU) |= PMSLEEP_CKG_SW_MCU_CLK;

		// wait for a while until mcu clock stable
		for( a=0; a<0xFF; a++ )
			b = a ;
	}
#endif
}


#define  _MEMMAP_PM_       { RV_REG_BASE_ADDR = 0xBF000000; }
#define  _MEMMAP_nonPM_    { RV_REG_BASE_ADDR = 0xBF200000; }
#define  REG_CSZ_SPI_FLASH 0x003c1c
#define _END_OF_TBL2_      // 0xFF, 0xFF




static void pad_config(void)
{
	// TODO: must be clarify --- Dean
	//Mst_PwmPortInit();

	//-----------------------------------------------------------------
	// JTAG overwrite
	//-----------------------------------------------------------------
	//_RV1(0x1E01, 0x07);


#if (MS_BOARD_TYPE_SEL == BD_GP2_DEMO1)

#if 0
	0x39, 0xB6, 0x5B, 0x53,     // magic code for ISP_Tool

		// programable device number
		// spi flash count
		1 + (PIN_SPI_CZ1 != 0) + (PIN_SPI_CZ2 != 0) + (PIN_SPI_CZ3 != 0),
		0x00,                       // nor
		0x00,                       // nand
		0x00,                       // reserved
		0x00,                       // reserved
		0x00,                       // reserved
#endif

		//---------------------------------------------------------------------
		// GPIO Configuartion
		//---------------------------------------------------------------------
		_MEMMAP_PM_;

	// SPI BUS
#define SPI_CZ_IS_GPIO  (PAD_PM_SPI_CZ_IS_GPIO ? BIT1:0)

#define SPI_BUS_IS_GPIO ((PAD_PM_SPI_CK_IS_GPIO | \
			PAD_PM_SPI_DI_IS_GPIO | \
			PAD_PM_SPI_DO_IS_GPIO) ? BIT0 :0)

#define SPI_BUS_OEN     ((PAD_PM_SPI_CZ_IS_GPIO == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_PM_SPI_CK_IS_GPIO == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_PM_SPI_DI_IS_GPIO == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_PM_SPI_DO_IS_GPIO == GPIO_IN ? (1UL << 7) : 0))

#define SPI_CZ_OUT      (PAD_PM_SPI_CZ_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 4) : 0)

#define SPI_BUS_OUT     ((PAD_PM_SPI_CK_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_PM_SPI_DI_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_PM_SPI_DO_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 7) : 0))

	_RVM1(0x0E3B, SPI_CZ_OUT, BIT4);
	_RVM1(0x0E3A, SPI_BUS_OUT, BITMASK(7:5));
	_RVM1(0x0E3C, SPI_BUS_OEN, BITMASK(7:4));
	_RVM1(0x0E6A, SPI_CZ_IS_GPIO, BIT1);
	_RVM1(0x0E6A, SPI_BUS_IS_GPIO, BIT0);

	// IRIN
#define IRIN_IS_GPIO    (PAD_IRIN_IS_GPIO ? BIT4: 0)
#define IRIN_OEN        (PAD_IRIN_IS_GPIO == GPIO_IN ? (1UL << 0) : 0)
#define IRIN_OUT        (PAD_IRIN_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0)

	_RVM1(0x0E3A, IRIN_OUT, BIT0);
	_RVM1(0x0E3C, IRIN_OEN, BIT0);
	_RVM1(0x0E38, IRIN_IS_GPIO, BIT4);

	// CEC
#define CEC_IS_GPIO     (PAD_CEC_IS_GPIO ? BIT6: 0)
#define CEC_OEN         (PAD_CEC_IS_GPIO == GPIO_IN ? (1UL << 2) : 0)
#define CEC_OUT         (PAD_CEC_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0)

	_RVM1(0x0E3C, CEC_OUT, BIT0);
	_RVM1(0x0E3C, CEC_OEN, BIT2);
	_RVM1(0x0E38, CEC_IS_GPIO, BIT6);

	// GPIO_PM0~GPIO_PM12
#define GPIO_PM6_IS_GPIO    (PAD_GPIO_PM6_IS_GPIO ? BIT2: 0)
#define GPIO_PM10_IS_GPIO   (PAD_GPIO_PM10_IS_GPIO ? 0: BIT3)

#define GPIO_PM_OEN     ((PAD_GPIO_PM0_IS_GPIO == GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_GPIO_PM1_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_GPIO_PM2_IS_GPIO == GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_GPIO_PM3_IS_GPIO == GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_GPIO_PM4_IS_GPIO == GPIO_IN ? (1UL <<  4) : 0) | \
		(PAD_GPIO_PM5_IS_GPIO == GPIO_IN ? (1UL <<  5) : 0) | \
		(PAD_GPIO_PM6_IS_GPIO == GPIO_IN ? (1UL <<  6) : 0) | \
		(PAD_GPIO_PM7_IS_GPIO == GPIO_IN ? (1UL <<  7) : 0) | \
		(PAD_GPIO_PM8_IS_GPIO == GPIO_IN ? (1UL <<  8) : 0) | \
		(PAD_GPIO_PM9_IS_GPIO == GPIO_IN ? (1UL <<  9) : 0) | \
		(PAD_GPIO_PM10_IS_GPIO == GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_GPIO_PM11_IS_GPIO == GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_GPIO_PM12_IS_GPIO == GPIO_IN ? (1UL << 12) : 0))

#define GPIO_PM_OUT     ((PAD_GPIO_PM0_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_GPIO_PM1_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_GPIO_PM2_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_GPIO_PM3_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_GPIO_PM4_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_GPIO_PM5_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_GPIO_PM6_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_GPIO_PM7_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
		(PAD_GPIO_PM8_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_GPIO_PM9_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_GPIO_PM10_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_GPIO_PM11_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_GPIO_PM12_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 12) : 0))

	_RVM2(0x0E20, GPIO_PM_OUT, 0x1FFF);
	_RVM2(0x0E1E, GPIO_PM_OEN, 0x1FFF);
	_RVM1(0x0E6A, GPIO_PM6_IS_GPIO|GPIO_PM10_IS_GPIO, BITMASK(3:2));

	// HOTPLUG A~D
#define HOT_PLUG_OEN    ((PAD_HOTPLUGA_IS_GPIO == GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_HOTPLUGB_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_HOTPLUGC_IS_GPIO == GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_HOTPLUGD_IS_GPIO == GPIO_IN ? (1UL <<  3) : 0))

#define HOT_PLUG_OUT    ((PAD_HOTPLUGA_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_HOTPLUGB_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_HOTPLUGC_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_HOTPLUGD_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  7) : 0))

	_RVM2(0x0E4E, HOT_PLUG_OUT, BITMASK(7:4));
	_RVM2(0x0E4E, HOT_PLUG_OEN, BITMASK(3:0));

	// DDCDA~DDCDD
#define DDCDA_IS_GPIO   ((PAD_DDCDA_CK_IS_GPIO  | \
			PAD_DDCDA_DA_IS_GPIO ) ? BIT7 :0)

#define DDCDA_OEN       ((PAD_DDCDA_CK_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_DDCDA_DA_IS_GPIO == GPIO_IN ? (1UL <<  5) : 0))

#define DDCDA_OUT       ((PAD_DDCDA_CK_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_DDCDA_DA_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0))

	_RVM1(0x0496, DDCDA_OUT, BIT4|BIT0);
	_RVM1(0x0496, DDCDA_OEN, BIT5|BIT1);
	_RVM1(0x0496, DDCDA_IS_GPIO, BIT7);

#define DDCDB_IS_GPIO   ((PAD_DDCDB_CK_IS_GPIO  | \
			PAD_DDCDB_DA_IS_GPIO ) ? BIT7 :0)

#define DDCDB_OEN       ((PAD_DDCDB_CK_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_DDCDB_DA_IS_GPIO == GPIO_IN ? (1UL <<  5) : 0))

#define DDCDB_OUT       ((PAD_DDCDB_CK_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_DDCDB_DA_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0))

	_RVM1(0x0497, DDCDB_OUT, BIT4|BIT0);
	_RVM1(0x0497, DDCDB_OEN, BIT5|BIT1);
	_RVM1(0x0497, DDCDB_IS_GPIO, BIT7);

#define DDCDC_IS_GPIO   ((PAD_DDCDC_CK_IS_GPIO  | \
			PAD_DDCDC_DA_IS_GPIO ) ? BIT7 :0)

#define DDCDC_OEN       ((PAD_DDCDC_CK_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_DDCDC_DA_IS_GPIO == GPIO_IN ? (1UL <<  5) : 0))

#define DDCDC_OUT       ((PAD_DDCDC_CK_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_DDCDC_DA_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0))

	_RVM1(0x0498, DDCDC_OUT, BIT4|BIT0);
	_RVM1(0x0498, DDCDC_OEN, BIT5|BIT1);
	_RVM1(0x0498, DDCDC_IS_GPIO, BIT7);

#define DDCDD_IS_GPIO   ((PAD_DDCDD_CK_IS_GPIO  | \
			PAD_DDCDD_DA_IS_GPIO ) ? BIT7 :0)

#define DDCDD_OEN       ((PAD_DDCDD_CK_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_DDCDD_DA_IS_GPIO == GPIO_IN ? (1UL <<  5) : 0))

#define DDCDD_OUT       ((PAD_DDCDD_CK_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_DDCDD_DA_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0))

	_RVM1(0x0499, DDCDD_OUT, BIT4|BIT0);
	_RVM1(0x0499, DDCDD_OEN, BIT5|BIT1);
	_RVM1(0x0499, DDCDD_IS_GPIO, BIT7);

	// SAR
#define SAR_OEN         ((PAD_SAR0_IS_GPIO == GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_SAR1_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_SAR2_IS_GPIO == GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_SAR3_IS_GPIO == GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_SAR4_IS_GPIO == GPIO_IN ? (1UL <<  4) : 0))

#define SAR_OUT         ((PAD_SAR0_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_SAR1_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_SAR2_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_SAR3_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_SAR4_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0))

	_RVM1(0x1425, SAR_OUT, BITMASK(4:0));
	_RVM1(0x1423, SAR_OEN, BITMASK(4:0));

	_MEMMAP_nonPM_;

	// GPIO
#define GPIO_OEN        ((PAD_GPIO0_IS_GPIO == GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_GPIO1_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_GPIO2_IS_GPIO == GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_GPIO3_IS_GPIO == GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_GPIO4_IS_GPIO == GPIO_IN ? (1UL <<  4) : 0) | \
		(PAD_GPIO5_IS_GPIO == GPIO_IN ? (1UL <<  5) : 0) | \
		(PAD_GPIO6_IS_GPIO == GPIO_IN ? (1UL <<  6) : 0) | \
		(PAD_GPIO7_IS_GPIO == GPIO_IN ? (1UL <<  7) : 0) | \
		(PAD_GPIO8_IS_GPIO == GPIO_IN ? (1UL <<  8) : 0) | \
		(PAD_GPIO9_IS_GPIO == GPIO_IN ? (1UL <<  9) : 0) | \
		(PAD_GPIO10_IS_GPIO == GPIO_IN ? (1UL <<  10) : 0) | \
		(PAD_GPIO11_IS_GPIO == GPIO_IN ? (1UL <<  11) : 0) | \
		(PAD_GPIO12_IS_GPIO == GPIO_IN ? (1UL <<  12) : 0) | \
		(PAD_GPIO13_IS_GPIO == GPIO_IN ? (1UL <<  13) : 0) | \
		(PAD_GPIO14_IS_GPIO == GPIO_IN ? (1UL <<  14) : 0) | \
		(PAD_GPIO15_IS_GPIO == GPIO_IN ? (1UL <<  15) : 0) | \
		(PAD_GPIO16_IS_GPIO == GPIO_IN ? (1UL <<  16) : 0) | \
		(PAD_GPIO17_IS_GPIO == GPIO_IN ? (1UL <<  17) : 0) | \
		(PAD_GPIO18_IS_GPIO == GPIO_IN ? (1UL <<  18) : 0) | \
		(PAD_GPIO19_IS_GPIO == GPIO_IN ? (1UL <<  19) : 0) | \
		(PAD_GPIO20_IS_GPIO == GPIO_IN ? (1UL <<  20) : 0) | \
	(PAD_GPIO21_IS_GPIO == GPIO_IN ? (1UL <<  21) : 0) | \
	(PAD_GPIO22_IS_GPIO == GPIO_IN ? (1UL <<  22) : 0) | \
	(PAD_GPIO23_IS_GPIO == GPIO_IN ? (1UL <<  23) : 0) | \
	(PAD_GPIO24_IS_GPIO == GPIO_IN ? (1UL <<  24) : 0) | \
	(PAD_GPIO25_IS_GPIO == GPIO_IN ? (1UL <<  25) : 0) | \
	(PAD_GPIO26_IS_GPIO == GPIO_IN ? (1UL <<  26) : 0) | \
	(PAD_GPIO27_IS_GPIO == GPIO_IN ? (1UL <<  27) : 0))

#define GPIO_OUT        ((PAD_GPIO0_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_GPIO1_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_GPIO2_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_GPIO3_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_GPIO4_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_GPIO5_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_GPIO6_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_GPIO7_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
		(PAD_GPIO8_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_GPIO9_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_GPIO10_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  10) : 0) | \
		(PAD_GPIO11_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  11) : 0) | \
		(PAD_GPIO12_IS_GPIO      == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_GPIO13_IS_GPIO      == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_GPIO14_IS_GPIO      == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_GPIO15_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  15) : 0) | \
		(PAD_GPIO16_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  16) : 0) | \
		(PAD_GPIO17_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  17) : 0) | \
		(PAD_GPIO18_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  18) : 0) | \
		(PAD_GPIO19_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  19) : 0) | \
		(PAD_GPIO20_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  20) : 0) | \
	(PAD_GPIO21_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  21) : 0) | \
	(PAD_GPIO22_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  22) : 0) | \
	(PAD_GPIO23_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  23) : 0) | \
	(PAD_GPIO24_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  24) : 0) | \
	(PAD_GPIO25_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  25) : 0) | \
	(PAD_GPIO26_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  26) : 0) | \
	(PAD_GPIO27_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  27) : 0))

	_RVM4(0x1E56, GPIO_OUT, 0x0FFFFFFF);
	_RVM4(0x1E5C, GPIO_OEN, 0x0FFFFFFF);

	// UART_RX2 and UART_TX2
#define UART2_OEN      ((PAD_UART_RX2_IS_GPIO == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_UART_TX2_IS_GPIO == GPIO_IN ? (1UL << 5) : 0))

#define UART2_OUT      ((PAD_UART_RX2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_UART_TX2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 5) : 0))

	_RVM1(0x1E59, UART2_OUT, BITMASK(5:4));
	_RVM1(0x1E5F, UART2_OEN, BITMASK(5:4));

	// PWM
#define PWM_IS_GPIO    ((PAD_PWM0_IS_GPIO ? (1UL << 0) : 0) | \
		(PAD_PWM1_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_PWM2_IS_GPIO ? (1UL << 2) : 0) | \
		(PAD_PWM3_IS_GPIO ? (1UL << 3) : 0) | \
		(PAD_PWM4_IS_GPIO ? (1UL << 4) : 0))

#define PWM_OEN        ((PAD_PWM0_IS_GPIO == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_PWM1_IS_GPIO == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_PWM2_IS_GPIO == GPIO_IN ? (1UL << 8) : 0) | \
		(PAD_PWM3_IS_GPIO == GPIO_IN ? (1UL << 9) : 0) | \
		(PAD_PWM4_IS_GPIO == GPIO_IN ? (1UL << 10) : 0))

#define PWM_OUT        ((PAD_PWM0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_PWM1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 7) : 0) | \
		(PAD_PWM2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 8) : 0) | \
		(PAD_PWM3_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 9) : 0) | \
		(PAD_PWM4_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 10) : 0))

	_RVM2(0x1E59, PWM_OUT, BITMASK(10:6));
	_RVM2(0x1E5F, PWM_OEN, BITMASK(10:6));
	_RVM1(0x1EA2, PWM_IS_GPIO, BITMASK(4:0));

	// DDCR
#define DDCR_IS_GPIO   ((PAD_DDCR_DA_IS_GPIO  | \
			PAD_DDCR_CK_IS_GPIO) ? BIT7 : 0)

#define DDCR_OEN       ((PAD_DDCR_DA_IS_GPIO == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_DDCR_CK_IS_GPIO == GPIO_IN ? (1UL << 4) : 0))

#define DDCR_OUT       ((PAD_DDCR_DA_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 3) : 0) | \
		(PAD_DDCR_CK_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 4) : 0))

	_RVM1(0x1E5A, DDCR_OUT, BITMASK(4:3));
	_RVM1(0x1E60, DDCR_OEN, BITMASK(4:3));
	_RVM1(0x1EE0, DDCR_IS_GPIO, BIT7);

	// TGPIO
#define TGPIO_OEN      ((PAD_TGPIO0_IS_GPIO == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_TGPIO1_IS_GPIO == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_TGPIO2_IS_GPIO == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_TGPIO3_IS_GPIO == GPIO_IN ? (1UL << 8) : 0))

#define TGPIO_OUT      ((PAD_TGPIO0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_TGPIO1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_TGPIO2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 7) : 0) | \
		(PAD_TGPIO3_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 8) : 0))

	_RVM2(0x1E5A, TGPIO_OUT, BITMASK(8:5));
	_RVM2(0x1E60, TGPIO_OEN, BITMASK(8:5));

	// TS0
#define TS0_IS_GPIO    ((PAD_TS0_D0_IS_GPIO ? (1UL << 2) : 0) | \
		(PAD_TS0_D1_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TS0_D2_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TS0_D3_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TS0_D4_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TS0_D5_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TS0_D6_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TS0_D7_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TS0_VLD_IS_GPIO ? (1UL << 2) : 0) | \
		(PAD_TS0_SYNC_IS_GPIO ? (1UL << 2) : 0) | \
		(PAD_TS0_CLK_IS_GPIO ? (1UL << 2) : 0))

#define TS0_OEN        ((PAD_TS0_D0_IS_GPIO == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_TS0_D1_IS_GPIO == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_TS0_D2_IS_GPIO == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_TS0_D3_IS_GPIO == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_TS0_D4_IS_GPIO == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_TS0_D5_IS_GPIO == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_TS0_D6_IS_GPIO == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_TS0_D7_IS_GPIO == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_TS0_VLD_IS_GPIO == GPIO_IN ? (1UL << 8) : 0) | \
		(PAD_TS0_SYNC_IS_GPIO == GPIO_IN ? (1UL << 9) : 0) | \
		(PAD_TS0_CLK_IS_GPIO == GPIO_IN ? (1UL << 10) : 0))

#define TS0_OUT        ((PAD_TS0_D0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_TS0_D1_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_TS0_D2_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_TS0_D3_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_TS0_D4_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_TS0_D5_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_TS0_D6_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_TS0_D7_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
		(PAD_TS0_VLD_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_TS0_SYNC_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_TS0_CLK_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 10) : 0))

	_RVM2(0x1E64, TS0_OUT, BITMASK(10:0));
	_RVM2(0x1E68, TS0_OEN, BITMASK(10:0));
	_RVM1(0x1EA3, TS0_IS_GPIO, BITMASK(2:1));

	// TS1
#define TS1_IS_GPIO    ((PAD_TS1_D0_IS_GPIO | \
			PAD_TS1_D1_IS_GPIO | \
			PAD_TS1_D2_IS_GPIO | \
			PAD_TS1_D3_IS_GPIO | \
			PAD_TS1_D4_IS_GPIO | \
			PAD_TS1_D5_IS_GPIO | \
			PAD_TS1_D6_IS_GPIO | \
			PAD_TS1_D7_IS_GPIO | \
			PAD_TS1_VLD_IS_GPIO | \
			PAD_TS1_SYNC_IS_GPIO | \
			PAD_TS1_CLK_IS_GPIO) ? BIT3 : 0)

#define TS1_OEN        ((PAD_TS1_D0_IS_GPIO == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_TS1_D1_IS_GPIO == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_TS1_D2_IS_GPIO == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_TS1_D3_IS_GPIO == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_TS1_D4_IS_GPIO == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_TS1_D5_IS_GPIO == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_TS1_D6_IS_GPIO == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_TS1_D7_IS_GPIO == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_TS1_VLD_IS_GPIO == GPIO_IN ? (1UL << 8) : 0) | \
		(PAD_TS1_SYNC_IS_GPIO == GPIO_IN ? (1UL << 9) : 0) | \
		(PAD_TS1_CLK_IS_GPIO == GPIO_IN ? (1UL << 10) : 0))

#define TS1_OUT        ((PAD_TS1_D0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_TS1_D1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 1) : 0) | \
		(PAD_TS1_D2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 2) : 0) | \
		(PAD_TS1_D3_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 3) : 0) | \
		(PAD_TS1_D4_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_TS1_D5_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_TS1_D6_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_TS1_D7_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 7) : 0) | \
		(PAD_TS1_VLD_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 8) : 0) | \
		(PAD_TS1_SYNC_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 9) : 0) | \
		(PAD_TS1_CLK_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 10) : 0))

	_RVM2(0x1E4C, TS1_OUT, BITMASK(10:0));
	_RVM2(0x1E4E, TS1_OEN, BITMASK(10:0));
	_RVM1(0x1EA3, TS1_IS_GPIO, BIT3);

	// PCM
#if (ENABLE_PCM_GPIO == 1)
#define PCM_IS_GPIO    ((PAD_PCM_A4_IS_GPIO     ? (1UL << 0) : 0) | \
		(PAD_PCM_WAIT_N_IS_GPIO ? (1UL << 0) : 0) | \
		(PAD_PCM_A5_IS_GPIO     ? (1UL << 1) : 0) | \
		(PAD_PCM_A6_IS_GPIO     ? (1UL << 1) : 0) | \
		(PAD_PCM_A7_IS_GPIO     ? (1UL << 1) : 0) | \
		(PAD_PCM_A12_IS_GPIO    ? (1UL << 1) : 0) | \
		(PAD_PCM_IRQA_N_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_PCM_A14_IS_GPIO    ? (1UL << 5) : 0) | \
		(PAD_PCM_A13_IS_GPIO    ? (1UL << 5) : 0) | \
		(PAD_PCM_A8_IS_GPIO     ? (1UL << 5) : 0) | \
		(PAD_PCM_IOWR_N_IS_GPIO ? (1UL << 5) : 0) | \
		(PAD_PCM_A9_IS_GPIO     ? (1UL << 5) : 0) | \
		(PAD_PCM_IORD_N_IS_GPIO ? (1UL << 5) : 0) | \
		(PAD_PCM_A11_IS_GPIO    ? (1UL << 5) : 0) | \
		(PAD_PCM_OE_N_IS_GPIO   ? (1UL << 5) : 0) | \
		(PAD_PCM_A10_IS_GPIO    ? (1UL << 5) : 0) | \
		(PAD_PCM_CE_N_IS_GPIO   ? (1UL << 5) : 0) | \
		(PAD_PCM_D7_IS_GPIO     ? (1UL << 5) : 0) | \
		(PAD_PCM_WE_N_IS_GPIO   ? (1UL << 5) : 0) | \
		(PAD_PCM_D6_IS_GPIO     ? (1UL << 2) : 0) | \
		(PAD_PCM_D5_IS_GPIO     ? (1UL << 2) : 0) | \
	(PAD_PCM_D4_IS_GPIO     ? (1UL << 4) : 0) | \
	(PAD_PCM_D3_IS_GPIO     ? (1UL << 4) : 0) | \
	(PAD_PCM_A3_IS_GPIO     ? (1UL << 3) : 0) | \
	(PAD_PCM_A2_IS_GPIO     ? (1UL << 3) : 0) | \
	(PAD_PCM_REG_N_IS_GPIO  ? (1UL << 3) : 0) | \
	(PAD_PCM_A1_IS_GPIO     ? (1UL << 3) : 0) | \
	(PAD_PCM_A0_IS_GPIO     ? (1UL << 3) : 0) | \
	(PAD_PCM_D0_IS_GPIO     ? (1UL << 3) : 0) | \
	(PAD_PCM_D1_IS_GPIO     ? (1UL << 3) : 0) | \
	(PAD_PCM_D2_IS_GPIO     ? (1UL << 3) : 0))

#define PCM_OEN        ((PAD_PCM_A4_IS_GPIO     == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_PCM_WAIT_N_IS_GPIO == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_PCM_A5_IS_GPIO     == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_PCM_A6_IS_GPIO     == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_PCM_A7_IS_GPIO     == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_PCM_A12_IS_GPIO    == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_PCM_IRQA_N_IS_GPIO == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_PCM_A14_IS_GPIO    == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_PCM_A13_IS_GPIO    == GPIO_IN ? (1UL << 8) : 0) | \
		(PAD_PCM_A8_IS_GPIO     == GPIO_IN ? (1UL << 9) : 0) | \
		(PAD_PCM_IOWR_N_IS_GPIO == GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_PCM_A9_IS_GPIO     == GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_PCM_IORD_N_IS_GPIO == GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_PCM_A11_IS_GPIO    == GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_PCM_OE_N_IS_GPIO   == GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_PCM_A10_IS_GPIO    == GPIO_IN ? (1UL << 15) : 0) | \
		(PAD_PCM_CE_N_IS_GPIO   == GPIO_IN ? (1UL << 16) : 0) | \
		(PAD_PCM_D7_IS_GPIO     == GPIO_IN ? (1UL << 17) : 0) | \
		(PAD_PCM_WE_N_IS_GPIO   == GPIO_IN ? (1UL << 18) : 0) | \
		(PAD_PCM_D6_IS_GPIO     == GPIO_IN ? (1UL << 19) : 0) | \
		(PAD_PCM_D5_IS_GPIO     == GPIO_IN ? (1UL << 20) : 0) | \
	(PAD_PCM_D4_IS_GPIO     == GPIO_IN ? (1UL << 21) : 0) | \
	(PAD_PCM_D3_IS_GPIO     == GPIO_IN ? (1UL << 22) : 0) | \
	(PAD_PCM_A3_IS_GPIO     == GPIO_IN ? (1UL << 23) : 0) | \
	(PAD_PCM_A2_IS_GPIO     == GPIO_IN ? (1UL << 24) : 0) | \
	(PAD_PCM_REG_N_IS_GPIO  == GPIO_IN ? (1UL << 25) : 0) | \
	(PAD_PCM_A1_IS_GPIO     == GPIO_IN ? (1UL << 26) : 0) | \
	(PAD_PCM_A0_IS_GPIO     == GPIO_IN ? (1UL << 27) : 0) | \
	(PAD_PCM_D0_IS_GPIO     == GPIO_IN ? (1UL << 28) : 0) | \
	(PAD_PCM_D1_IS_GPIO     == GPIO_IN ? (1UL << 29) : 0) | \
	(PAD_PCM_D2_IS_GPIO     == GPIO_IN ? (1UL << 30) : 0) | \
	(PAD_PCM_RESET_IS_GPIO  == GPIO_IN ? (1UL << 31) : 0))

#define PCM_OUT        ((PAD_PCM_A4_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_PCM_WAIT_N_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_PCM_A5_IS_GPIO     == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_PCM_A6_IS_GPIO     == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_PCM_A7_IS_GPIO     == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_PCM_A12_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_PCM_IRQA_N_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_PCM_A14_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
		(PAD_PCM_A13_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_PCM_A8_IS_GPIO     == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_PCM_IOWR_N_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_PCM_A9_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_PCM_IORD_N_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_PCM_A11_IS_GPIO    == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_PCM_OE_N_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_PCM_A10_IS_GPIO    == GPIO_OUT_HIGH ? (1UL << 15) : 0) | \
		(PAD_PCM_CE_N_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 16) : 0) | \
		(PAD_PCM_D7_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 17) : 0) | \
		(PAD_PCM_WE_N_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 18) : 0) | \
		(PAD_PCM_D6_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 19) : 0) | \
		(PAD_PCM_D5_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 20) : 0) | \
	(PAD_PCM_D4_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 21) : 0) | \
	(PAD_PCM_D3_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 22) : 0) | \
	(PAD_PCM_A3_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 23) : 0) | \
	(PAD_PCM_A2_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 24) : 0) | \
	(PAD_PCM_REG_N_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 25) : 0) | \
	(PAD_PCM_A1_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 26) : 0) | \
	(PAD_PCM_A0_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 27) : 0) | \
	(PAD_PCM_D0_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 28) : 0) | \
	(PAD_PCM_D1_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 29) : 0) | \
	(PAD_PCM_D2_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 30) : 0) | \
	(PAD_PCM_RESET_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 31) : 0))


#define PCM_RESET_CD_IS_GPIO     ((PAD_PCM_RESET_IS_GPIO | \
			PAD_PCM_CD_N_IS_GPIO) ? (BIT3|BIT2) : 0)

#define PCM_CD_OEN      (PAD_PCM_CD_N_IS_GPIO == GPIO_IN ? (1UL << 0) : 0)
#define PCM_CD_OUT      (PAD_PCM_CD_N_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0)

	_RVM4(0x1E76, PCM_OUT, 0xFFFFFFFF);
	_RVM4(0x1E7C, PCM_OEN, 0xFFFFFFFF);
	_RVM1(0x1E7A, PCM_CD_OUT, BIT0);
	_RVM1(0x1E80, PCM_CD_OEN, BIT0);
	_RVM1(0x1EDC, PCM_RESET_CD_IS_GPIO, BITMASK(3:2));
	_RVM1(0x1EE0, PCM_IS_GPIO, BITMASK(5:0));

#elif (ENABLE_NOR_GPIO == 1)

#else
#error Unknow PCM or NOR GPIO type
#endif

	// PF
#if (ENABLE_PF_GPIO == 1)
#define PF_IS_GPIO      ((PAD_PF_AD15_IS_GPIO ? (1UL << 4) : 0) | \
		(PAD_PF_CE0Z_IS_GPIO ? (1UL << 4) : 0) | \
		(PAD_PF_CE1Z_IS_GPIO ? (1UL << 4) : 0) | \
		(PAD_PF_OEZ_IS_GPIO  ? (1UL << 4) : 0) | \
		(PAD_PF_WEZ_IS_GPIO  ? (1UL << 4) : 0) | \
		(PAD_PF_ALE_IS_GPIO  ? (1UL << 4) : 0) | \
		(PAD_F_RBZ_IS_GPIO   ? (1UL << 5) : 0))

#define PF_OEN          ((PAD_PF_AD15_IS_GPIO == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_PF_CE0Z_IS_GPIO == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_PF_CE1Z_IS_GPIO == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_PF_OEZ_IS_GPIO  == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_PF_WEZ_IS_GPIO  == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_PF_ALE_IS_GPIO  == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_F_RBZ_IS_GPIO   == GPIO_IN ? (1UL << 6) : 0))

#define PF_OUT          ((PAD_PF_AD15_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_PF_CE0Z_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 1) : 0) | \
		(PAD_PF_CE1Z_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 2) : 0) | \
		(PAD_PF_OEZ_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 3) : 0) | \
		(PAD_PF_WEZ_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_PF_ALE_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_F_RBZ_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 6) : 0))

	_RVM1(0x1E8A, PF_OUT, BITMASK(6:0));
	_RVM1(0x1E8C, PF_OEN, BITMASK(6:0));
	_RVM1(0x1EDF, PF_IS_GPIO, BITMASK(5:4));

#elif (ENABLE_NOR_GPIO == 1)

#else
#error Unknow PCM or NOR GPIO type
#endif

	// TCON
#define TCON_IS_GPIO    ((PAD_TCON0_IS_GPIO ? (1UL << 0) : 0) | \
		(PAD_TCON1_IS_GPIO ? (1UL << 0) : 0) | \
		(PAD_TCON2_IS_GPIO ? (1UL << 0) : 0) | \
		(PAD_TCON3_IS_GPIO  ? (1UL << 0) : 0) | \
		(PAD_TCON4_IS_GPIO  ? (1UL << 1) : 0) | \
		(PAD_TCON5_IS_GPIO  ? (1UL << 1) : 0) | \
		(PAD_TCON6_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TCON7_IS_GPIO ? (1UL << 1) : 0) | \
		(PAD_TCON8_IS_GPIO  ? (1UL << 2) : 0) | \
		(PAD_TCON9_IS_GPIO  ? (1UL << 2) : 0) | \
		(PAD_TCON10_IS_GPIO  ? (1UL << 2) : 0) | \
		(PAD_TCON11_IS_GPIO ? (1UL << 2) : 0) | \
		(PAD_TCON12_IS_GPIO ? (1UL << 2) : 0) | \
		(PAD_TCON13_IS_GPIO  ? (1UL << 2) : 0) | \
		(PAD_TCON14_IS_GPIO  ? (1UL << 2) : 0) | \
		(PAD_TCON15_IS_GPIO  ? (1UL << 3) : 0) | \
		(PAD_TCON16_IS_GPIO  ? (1UL << 3) : 0) | \
		(PAD_TCON17_IS_GPIO  ? (1UL << 3) : 0) | \
		(PAD_TCON18_IS_GPIO  ? (1UL << 3) : 0) | \
		(PAD_TCON19_IS_GPIO  ? (1UL << 3) : 0) | \
		(PAD_TCON20_IS_GPIO  ? (1UL << 3) : 0) | \
	(PAD_TCON21_IS_GPIO  ? (1UL << 3) : 0))

#define TCON_OEN0       ((PAD_TCON0_IS_GPIO == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_TCON1_IS_GPIO == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_TCON2_IS_GPIO == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_TCON3_IS_GPIO  == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_TCON4_IS_GPIO  == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_TCON5_IS_GPIO  == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_TCON6_IS_GPIO  == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_TCON7_IS_GPIO  == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_TCON8_IS_GPIO  == GPIO_IN ? (1UL << 8) : 0) | \
		(PAD_TCON9_IS_GPIO  == GPIO_IN ? (1UL << 9) : 0) | \
		(PAD_TCON10_IS_GPIO  == GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_TCON11_IS_GPIO  == GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_TCON12_IS_GPIO  == GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_TCON13_IS_GPIO  == GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_TCON14_IS_GPIO  == GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_TCON15_IS_GPIO  == GPIO_IN ? (1UL << 15) : 0))

#define TCON_OEN1       ((PAD_TCON16_IS_GPIO == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_TCON17_IS_GPIO == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_TCON18_IS_GPIO == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_TCON19_IS_GPIO  == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_TCON20_IS_GPIO  == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_TCON21_IS_GPIO  == GPIO_IN ? (1UL << 5) : 0))

#define TCON_OUT        ((PAD_TCON0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_TCON1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 1) : 0) | \
		(PAD_TCON2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 2) : 0) | \
		(PAD_TCON3_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 3) : 0) | \
		(PAD_TCON4_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_TCON5_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_TCON6_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_TCON7_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 7) : 0) | \
		(PAD_TCON8_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 8) : 0) | \
		(PAD_TCON9_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 9) : 0) | \
		(PAD_TCON10_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_TCON11_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_TCON12_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_TCON13_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_TCON14_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_TCON15_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 15) : 0) | \
		(PAD_TCON16_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 16) : 0) | \
		(PAD_TCON17_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 17) : 0) | \
		(PAD_TCON18_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 18) : 0) | \
		(PAD_TCON19_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 19) : 0) | \
		(PAD_TCON20_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 20) : 0) | \
	(PAD_TCON21_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 21) : 0))

	_RVM3(0x1E98, TCON_OUT, 0x3FFFFF);
	_RVM2(0x1E9C, TCON_OEN0, 0xFFFF);
	_RVM1(0x1E9B, TCON_OEN1, BITMASK(5:0));
	_RVM1(0x1EA5, TCON_IS_GPIO, BITMASK(3:0));

	// ET
#define ET_IS_GPIO      ((PAD_ET_COL_IS_GPIO | \
			PAD_ET_TXD1_IS_GPIO | \
			PAD_ET_TXD0_IS_GPIO | \
			PAD_ET_TX_EN_IS_GPIO | \
			PAD_ET_TX_CLK_IS_GPIO | \
			PAD_ET_RXD0_IS_GPIO | \
			PAD_ET_RXD1_IS_GPIO | \
			PAD_ET_MDC_IS_GPIO | \
			PAD_ET_MDIO_IS_GPIO) ? BIT2 : 0)

#define ET_OEN          ((PAD_ET_COL_IS_GPIO == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_ET_TXD1_IS_GPIO == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_ET_TXD0_IS_GPIO == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_ET_TX_EN_IS_GPIO  == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_ET_TX_CLK_IS_GPIO  == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_ET_RXD0_IS_GPIO  == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_ET_RXD1_IS_GPIO  == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_ET_MDC_IS_GPIO  == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_ET_MDIO_IS_GPIO  == GPIO_IN ? (1UL << 8) : 0))

#define ET_OUT          ((PAD_ET_COL_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_ET_TXD1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 1) : 0) | \
		(PAD_ET_TXD0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 2) : 0) | \
		(PAD_ET_TX_EN_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 3) : 0) | \
		(PAD_ET_TX_CLK_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_ET_RXD0_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_ET_RXD1_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_ET_MDC_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 7) : 0) | \
		(PAD_ET_MDIO_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 8) : 0))

	_RVM2(0x1E84, ET_OUT, 0x01FF);
	_RVM2(0x1E86, ET_OEN, 0x01FF);
	_RVM1(0x1EDF, ET_IS_GPIO, BIT2);

	// I2S,SPDIF,
#define I2S_IS_GPIO     ((PAD_I2S_IN_WS_IS_GPIO ? (1UL << 5) : 0) | \
		(PAD_I2S_IN_BCK_IS_GPIO ? (1UL << 5) : 0) | \
		(PAD_I2S_IN_SD_IS_GPIO ? (1UL << 5) : 0) | \
		(PAD_I2S_OUT_MCK_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_WS_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_BCK_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_SD_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_SD1_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_SD2_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_SD3_IS_GPIO ? (1UL << 6) : 0))

#define I2S_IS_GPIO_EX  ((PAD_I2S_IN_WS_IS_GPIO ? (1UL << 5) : 0) | \
		(PAD_I2S_IN_BCK_IS_GPIO ? (1UL << 5) : 0) | \
		(PAD_I2S_IN_SD_IS_GPIO ? (1UL << 5) : 0) | \
		(PAD_I2S_OUT_MCK_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_WS_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_BCK_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_SD_IS_GPIO ? (1UL << 6) : 0))

#define I2S_OUT_IS_GPIO_NONE  ((PAD_I2S_OUT_MCK_IS_GPIO | \
		PAD_I2S_OUT_WS_IS_GPIO  | \
		PAD_I2S_OUT_BCK_IS_GPIO | \
		PAD_I2S_OUT_SD_IS_GPIO) ? 0 : 3)

#define I2S_SD_IS_GPIO  (((PAD_I2S_OUT_SD1_IS_GPIO & I2S_OUT_IS_GPIO_NONE) ? (1UL << 2) : 0) | \
		((PAD_I2S_OUT_SD2_IS_GPIO & I2S_OUT_IS_GPIO_NONE) ? (1UL << 2) : 0) | \
		((PAD_I2S_OUT_SD3_IS_GPIO & I2S_OUT_IS_GPIO_NONE) ? (1UL << 2) : 0))

#define SPDIF_IS_GPIO   ((PAD_SPDIF_IN_IS_GPIO ? (1UL << 6) : 0) | \
		(PAD_SPDIF_OUT_IS_GPIO ? (1UL << 7) : 0))

#define I2S_SPDIF_OEN   ((PAD_I2S_IN_WS_IS_GPIO     == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_I2S_IN_BCK_IS_GPIO    == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_I2S_IN_SD_IS_GPIO     == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_SPDIF_IN_IS_GPIO      == GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_SPDIF_OUT_IS_GPIO     == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_I2S_OUT_MCK_IS_GPIO   == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_I2S_OUT_WS_IS_GPIO    == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_BCK_IS_GPIO   == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_I2S_OUT_SD_IS_GPIO    == GPIO_IN ? (1UL << 8) : 0) | \
		(PAD_I2S_OUT_SD1_IS_GPIO   == GPIO_IN ? (1UL << 9) : 0) | \
		(PAD_I2S_OUT_SD2_IS_GPIO   == GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_I2S_OUT_SD3_IS_GPIO   == GPIO_IN ? (1UL << 11) : 0))

#define I2S_SPDIF_OUT   ((PAD_I2S_IN_WS_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_I2S_IN_BCK_IS_GPIO    == GPIO_OUT_HIGH ? (1UL << 1) : 0) | \
		(PAD_I2S_IN_SD_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 2) : 0) | \
		(PAD_SPDIF_IN_IS_GPIO      == GPIO_OUT_HIGH ? (1UL << 3) : 0) | \
		(PAD_SPDIF_OUT_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_I2S_OUT_MCK_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_I2S_OUT_WS_IS_GPIO    == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_BCK_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 7) : 0) | \
		(PAD_I2S_OUT_SD_IS_GPIO    == GPIO_OUT_HIGH ? (1UL << 8) : 0) | \
		(PAD_I2S_OUT_SD1_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 9) : 0) | \
		(PAD_I2S_OUT_SD2_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_I2S_OUT_SD3_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 11) : 0))

	_RVM2(0x1E6C, I2S_SPDIF_OUT, 0x0FFF);
	_RVM2(0x1E6E, I2S_SPDIF_OEN, 0x0FFF);
	_RVM1(0x1EE1, SPDIF_IS_GPIO, BIT7|BIT6);
	if(Chip_Query_Rev()<3)
	{
	    _RVM1(0x1EA2, I2S_IS_GPIO, BIT6|BIT5);
	}
	else
	{
	    _RVM1(0x1EA2, I2S_IS_GPIO_EX, BIT6|BIT5);
	    _RVM1(0x1EA1, I2S_SD_IS_GPIO, BIT2);
	}

	// LVDS,ODD
#define LVDS_ODD_OEN    ((PAD_LVSYNC_IS_GPIO     == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_LHSYNC_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_LDE_IS_GPIO    == GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_LCK_IS_GPIO    == GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_mini_LVDS7_IS_GPIO == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_mini_LVDS6_IS_GPIO == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_mini_LVDS5_IS_GPIO == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_mini_LVDS4_IS_GPIO == GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_mini_LVDS3_IS_GPIO == GPIO_IN ? (1UL << 8) : 0) | \
		(PAD_mini_LVDS2_IS_GPIO == GPIO_IN ? (1UL << 9) : 0) | \
		(PAD_mini_LVDS1_IS_GPIO == GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_mini_LVDS0_IS_GPIO == GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_R_ODD7_IS_GPIO     == GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_R_ODD6_IS_GPIO     == GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_R_ODD5_IS_GPIO     == GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_R_ODD4_IS_GPIO     == GPIO_IN ? (1UL << 15) : 0) | \
		(PAD_R_ODD3_IS_GPIO     == GPIO_IN ? (1UL << 16) : 0) | \
		(PAD_R_ODD2_IS_GPIO     == GPIO_IN ? (1UL << 17) : 0) | \
		(PAD_R_ODD1_IS_GPIO     == GPIO_IN ? (1UL << 18) : 0) | \
		(PAD_R_ODD0_IS_GPIO     == GPIO_IN ? (1UL << 19) : 0) | \
		(PAD_G_ODD7_IS_GPIO     == GPIO_IN ? (1UL << 20) : 0) | \
	(PAD_G_ODD6_IS_GPIO     == GPIO_IN ? (1UL << 21) : 0) | \
	(PAD_G_ODD5_IS_GPIO     == GPIO_IN ? (1UL << 22) : 0) | \
	(PAD_G_ODD4_IS_GPIO     == GPIO_IN ? (1UL << 23) : 0) | \
	(PAD_G_ODD3_IS_GPIO     == GPIO_IN ? (1UL << 24) : 0) | \
	(PAD_G_ODD2_IS_GPIO     == GPIO_IN ? (1UL << 25) : 0) | \
	(PAD_G_ODD1_IS_GPIO     == GPIO_IN ? (1UL << 26) : 0) | \
	(PAD_G_ODD0_IS_GPIO     == GPIO_IN ? (1UL << 27) : 0) | \
	(PAD_B_ODD7_IS_GPIO     == GPIO_IN ? (1UL << 28) : 0) | \
	(PAD_B_ODD6_IS_GPIO     == GPIO_IN ? (1UL << 29) : 0) | \
	(PAD_B_ODD5_IS_GPIO     == GPIO_IN ? (1UL << 30) : 0) | \
	(PAD_B_ODD4_IS_GPIO     == GPIO_IN ? (1UL << 31) : 0))

#define LVDS_ODD_OUT    ((PAD_LVSYNC_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_LHSYNC_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_LDE_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_LCK_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_mini_LVDS7_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_mini_LVDS6_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_mini_LVDS5_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_mini_LVDS4_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 7) : 0) | \
		(PAD_mini_LVDS3_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 8) : 0) | \
		(PAD_mini_LVDS2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 9) : 0) | \
		(PAD_mini_LVDS1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_mini_LVDS0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_R_ODD7_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_R_ODD6_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_R_ODD5_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_R_ODD4_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 15) : 0) | \
		(PAD_R_ODD3_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 16) : 0) | \
		(PAD_R_ODD2_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 17) : 0) | \
		(PAD_R_ODD1_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 18) : 0) | \
		(PAD_R_ODD0_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 19) : 0) | \
		(PAD_G_ODD7_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 20) : 0) | \
	(PAD_G_ODD6_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 21) : 0) | \
	(PAD_G_ODD5_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 22) : 0) | \
	(PAD_G_ODD4_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 23) : 0) | \
	(PAD_G_ODD3_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 24) : 0) | \
	(PAD_G_ODD2_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 25) : 0) | \
	(PAD_G_ODD1_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 26) : 0) | \
	(PAD_G_ODD0_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 27) : 0) | \
	(PAD_B_ODD7_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 28) : 0) | \
	(PAD_B_ODD6_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 29) : 0) | \
	(PAD_B_ODD5_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 30) : 0) | \
	(PAD_B_ODD4_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 31) : 0))

#define B_ODD_OEN       ((PAD_B_ODD3_IS_GPIO == GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_B_ODD2_IS_GPIO == GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_B_ODD1_IS_GPIO == GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_B_ODD0_IS_GPIO == GPIO_IN ? (1UL << 3) : 0))

#define B_ODD_OUT       ((PAD_B_ODD3_IS_GPIO == GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_B_ODD2_IS_GPIO == GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_B_ODD1_IS_GPIO == GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_B_ODD0_IS_GPIO == GPIO_IN ? (1UL << 7) : 0))

	_RVM1(0x32FC, B_ODD_OUT, BITMASK(7:4));
	_RVM1(0x32FC, B_ODD_OEN, BITMASK(3:0));
	_RVM4(0x329E, LVDS_ODD_OUT, 0xFFFFFFFF);
	_RVM4(0x32A2, LVDS_ODD_OEN, 0xFFFFFFFF);

	//---------------------------------------------------------------------
	// Pad Configuartion
	//---------------------------------------------------------------------

#if defined(UART0)
	_RVM1(0x1E03, BIT2, BIT2);   // RX0_ENABLE

	// UART2 [reg_SecondUARTMode]
#define UART2_MODE  ((PIN_UART2 == PAD_TS0_D2) ? (BIT0) : \
		(PIN_UART2 == PAD_TS0_D4) ? (BIT0) : \
		(PIN_UART2 == PAD_PCM_A3) ? (BIT1) : \
		(PIN_UART2 == PAD_PCM_A2) ? (BIT1) : \
		(PIN_UART2 == PAD_GPIO14) ? (BIT1|BIT0) : \
		(PIN_UART2 == PAD_GPIO15) ? (BIT1|BIT0) : 0)

	// UART3 [reg_ThirdUARTMode]
#define UART3_MODE  ((PIN_UART3 == PAD_TS0_D6) ? (BIT2) : \
		(PIN_UART3 == PAD_TS0_VLD) ? (BIT2) : \
		(PIN_UART3 == PAD_UART_RX2) ? (BIT3) : \
		(PIN_UART3 == PAD_UART_TX2) ? (BIT3) : \
		(PIN_UART3 == PAD_GPIO12) ? (BIT3|BIT2) : \
		(PIN_UART3 == PAD_GPIO13) ? (BIT3|BIT2) : 0)

	_RVM1(0x1E05, UART3_MODE|UART2_MODE, BITMASK(3:0));

	// UART3 in TCON [reg_uart3_in_tcon] ???
#define UART3_IN_TCON ((PIN_UART3_IN_TCON == PAD_TCON4) ? (BIT7) : \
		(PIN_UART3_IN_TCON == PAD_TCON6) ? (BIT7) : 0)

	_RVM1(0x1EA3, UART3_IN_TCON, BIT7);

	// UART4 [reg_FastUARTMode]
#define UART4_MODE  ((PIN_UART4 == PAD_GPIO0) ? (BIT6) : \
		(PIN_UART4 == PAD_GPIO1) ? (BIT6) : \
		(PIN_UART4 == PAD_PCM2_CE_N) ? (BIT7) : \
		(PIN_UART4 == PAD_PCM2_IRQA_N) ? (BIT7) : \
		(PIN_UART4 == PAD_TCON0) ? (BIT7|BIT6) : \
		(PIN_UART4 == PAD_TCON2) ? (BIT7|BIT6) : 0)

	// FAST UART [reg_FastUARTMode]
#define FAST_UART_MODE ((PIN_FAST_UART == PAD_I2S_IN_WS) ? (BIT4) : \
		(PIN_FAST_UART == PAD_I2S_IN_SD) ? (BIT4) : \
		(PIN_FAST_UART == PAD_SPDIF_OUT) ? (BIT5) : \
		(PIN_FAST_UART == PAD_MPIF_D3) ? (BIT5) : \
		(PIN_FAST_UART == PAD_GPIO10) ? (BIT5|BIT4) : \
		(PIN_FAST_UART == PAD_GPIO11) ? (BIT5|BIT4) : 0)

	_RVM1(0x1E04, FAST_UART_MODE|UART4_MODE, BITMASK(7:4));

	// UART_SRC_SEL
#define UART_INV    ((UART0_INV ? BIT0 : 0) | \
		(UART1_INV ? BIT1 : 0) | \
		(UART2_INV ? BIT2 : 0) | \
		(UART3_INV ? BIT2 : 0) | \
		(UART4_INV ? BIT3 : 0))

	_RVM1(0x1EAB, UART_INV, BITMASK(7:0));
	_RVM1(0x1EA6, (UART1_SRC_SEL << 4) | (UART0_SRC_SEL << 0), BITMASK(7:0));
	_RVM1(0x1EA7, (UART3_SRC_SEL << 4) | (UART2_SRC_SEL << 0), BITMASK(7:0));
	_RVM1(0x1EA8, (UART4_SRC_SEL << 0), BITMASK(3:0));

#endif

	// I2S MUTE MODE [reg_I2SMUTEMode]
#define I2S_MUTE    ((PIN_I2S_OUT_MUTE == PAD_GPIO7) ? (BIT6) : \
		(PIN_I2S_OUT_MUTE == PAD_ET_COL) ? (BIT7) : 0)

	_RVM1(0x1EA8, (UART4_SRC_SEL << 0), BITMASK(3:0));

	// TS1 IS OUT[reg_TS1isOut]
#define TS1_IS_OUT  (PIN_TS1_IS_OUT ? (BIT3) : 0)

	_RVM1(0x1EA5, TS1_IS_OUT, BIT3);

	// TS1 IN TCON[reg_TS1inTCON]
#define TS1_IS_TCON (PIN_TS1_IS_TCON ? (BIT7) : 0)

	_RVM1(0x1EA5, TS1_IS_TCON, BIT7);

	// PF MODE[reg_pf_mode]
#define PF_MODE     (PIN_PF ? (BIT4) : 0)

	_RVM1(0x1EDE, PF_MODE, BIT4);

	// ETHERNET[reg_ET_Mode]
#define ET_MODE     ((PIN_ET == PADS_ET_MODE1) ? (BIT0) : \
		(PIN_ET == PADS_ET_MODE2) ? (BIT1) : 0)

	_RVM1(0x1EDF, ET_MODE, BIT1|BIT0);

	// PCMCIA and CIMAX - [reg_PCMConfig][reg_pcmconfig1][reg_pcm2config][reg_pcm2config1]
#define PCM_CONFIG      ((PIN_PCM_CONFIG == PADS_PCMCONFIG_MODE1) ? (BIT0) : \
		(PIN_PCM_CONFIG == PADS_PCMCONFIG_MODE2) ? (BIT1) : 0)

#define PCM_CONFIG1     ((PIN_PCM_CONFIG1 == PADS_PCMCONFIG1_MODE1) ? (BIT2) : \
		(PIN_PCM_CONFIG1 == PADS_PCMCONFIG1_MODE2) ? (BIT3) : \
		(PIN_PCM_CONFIG1 == PADS_PCMCONFIG1_MODE3) ? (BIT3|BIT2) : 0)

#define PCM2_CONFIG     ((PIN_PCM2_CONFIG == PADS_PCM2CONFIG_MODE1) ? (BIT4) : \
		(PIN_PCM2_CONFIG == PADS_PCM2CONFIG_MODE2) ? (BIT5) : \
		(PIN_PCM2_CONFIG == PADS_PCM2CONFIG_MODE3) ? (BIT5|BIT4) : 0)

#define PCM2_CONFIG1    ((PIN_PCM2_CONFIG1 == PADS_PCM2CONFIG1_MODE1) ? (BIT6) : \
		(PIN_PCM2_CONFIG1 == PADS_PCM2CONFIG1_MODE2) ? (BIT7) : 0)

	_RVM1(0x1EDC, PCM_CONFIG|PCM_CONFIG1|PCM2_CONFIG|PCM2_CONFIG1, BITMASK(7:0));

	// I2S_IN_BCK IN TCON[reg_i2s_in_bck_in_tcon]
#define I2S_IN_BCK_IN_TCON  (PIN_I2S_IN_BCK == PAD_TCON0 ? (BIT0) : 0)

	_RVM1(0x1EA3, I2S_IN_BCK_IN_TCON, BIT0);

	// SPDIF_IN IN TCON[reg_spdif_in_in_tcon]
#define SPDIF_IN    (PIN_SPDIF_IN == PAD_TCON2 ? (BIT7) : 0)

	_RVM1(0x1EA2, SPDIF_IN, BIT7);

	// MIIC[reg_iicmmode]
#define MIIC_MODE   ((PIN_MIIC == PAD_DDCR_DA) ? (BIT1) : \
		(PIN_MIIC == PAD_DDCR_CK) ? (BIT1) : 0)

	_RVM1(0x1EA1, MIIC_MODE, BIT1|BIT0);

	_MEMMAP_PM_;

	_RVM1(0x0E13, ~BIT4 , BIT4);   // UART0 -> HK_AEON

#define SEL_CZ_ON   ((PIN_SPI_CZ1 ? BIT5 : 0) | \
		(PIN_SPI_CZ2 ? BIT6 : 0) | \
		(PIN_SPI_CZ3 ? BIT7 : 0))

	_RVM1(0x3C1D, ~(SEL_CZ_ON), BITMASK(7:5));

	_MEMMAP_nonPM_;

	// Clear all pad in
	_RVM1(0x1EA1, 0, BIT7);
	_END_OF_TBL2_;

	//---------------------------------------------------------------------
	// ISP_TOOL Write Protect (Need to check ????)
	//---------------------------------------------------------------------

	_MEMMAP_PM_;

	_RVM2(REG_CSZ_SPI_FLASH, 0xE000, 0xE001);
#if (PIN_FLASH_WP0)
	_RVM2(0x0E21, BIT2, BIT2);
	_RVM2(0x0E1F,    0, BIT2);
#endif
	_END_OF_TBL2_;

#if (PIN_SPI_CZ1)
	_RVM2(REG_CSZ_SPI_FLASH, 0xC001, 0xE001);
#if (PIN_FLASH_WP1)
	_RVM2(0x0E21, BIT1, BIT1);
	_RVM2(0x0E1F,    0, BIT1);
#endif
	_END_OF_TBL2_;
#endif

#if (PIN_SPI_CZ2)
	_RVM2(REG_CSZ_SPI_FLASH, 0xA001, 0xE001);
#if (PIN_FLASH_WP2)
	// will assign coorect register while future usage
	//_RVM2(0x0E21, BIT2, BIT2); //_RVMG(CONCAT(GPIO_NUM(PIN_FLASH_WP2), _OUT), 1);
	//_RVM2(0x0E1F,    0, BIT2);//_RVMG(CONCAT(GPIO_NUM(PIN_FLASH_WP2), _OEN), 0);
#endif
	_END_OF_TBL2_;
#endif

#if (PIN_SPI_CZ3)
	_RVM2(REG_CSZ_SPI_FLASH, 0x6001, 0xE001);
#if (PIN_FLASH_WP3)
	// will assign coorect register while future usage
	//_RVM2(0x0E21, BIT2, BIT2); //_RVMG(CONCAT(GPIO_NUM(PIN_FLASH_WP3), _OUT), 1);
	//_RVM2(0x0E1F,    0, BIT2);//_RVMG(CONCAT(GPIO_NUM(PIN_FLASH_WP3), _OEN), 0);
#endif
	_END_OF_TBL2_;
#endif

#endif   // #if (MS_BOARD_TYPE_SEL == BD_GP2_DEMO1)

}

static void miu_priority_config(void)
{
#if defined(CONFIG_Titania3)
	//samuel 081022 //*((volatile unsigned int *) (0xbf20240c)) |= 0x4000;        // MIU group 1 priority > group 0

	// bit 0 & 1 can't be 1 at the same time
	//*((volatile unsigned int *) (0xbf2024c0)) |= 0x000b;        // Group 1 round robin enable; Group 1 fixed priority enable; Group 1 group limit enable

	*((volatile unsigned int *) (0xbf202500)) |= 0x000E;        // Group 2 seeting
	*((volatile unsigned int *) (0xbf202504)) |= 0x0004;        // Group 2 member max.

	// MIU0
	// Group priority config
	*(volatile unsigned int*)(0xbf200000+(0x96E<<2)) &= 0xFEFF;  // Need to set it default priority before set fixed priority
	*(volatile unsigned int*)(0xbf200000+(0x96E<<2)) &= 0xFF00;  // Clear the priority
	*(volatile unsigned int*)(0xbf200000+(0x96E<<2)) |= 0x00C9;
	*(volatile unsigned int*)(0xbf200000+(0x96E<<2)) |= 0x0100;  // Set to fixed priority

#if 0   // TODO: Turn on it, if want to config priority
	// Group 0 client config
	// Set client to the highest priority. 0xFFFF means none is in the highest priority
	*(volatile unsigned int*)(0xbf200000+(0x924<<2)) = 0xFFFF;

	*(volatile unsigned int*)(0xbf200000+(0x925<<2)) = 0x0000;  // Clear the priority
	*(volatile unsigned int*)(0xbf200000+(0x926<<2)) = 0x0000;  // Clear the priority
	*(volatile unsigned int*)(0xbf200000+(0x927<<2)) = 0x0000;  // Clear the priority
	*(volatile unsigned int*)(0xbf200000+(0x928<<2)) = 0x0000;  // Clear the priority

	*(volatile unsigned int*)(0xbf200000+(0x925<<2)) &= 0x3210;  // Set the priority
	*(volatile unsigned int*)(0xbf200000+(0x926<<2)) &= 0x7654;  // Set the priority
	*(volatile unsigned int*)(0xbf200000+(0x927<<2)) &= 0xBA98;  // Set the priority
	*(volatile unsigned int*)(0xbf200000+(0x928<<2)) &= 0xFEDC;  // Set the priority

	*(volatile unsigned int*)(0xbf200000+(0x920<<2)) |= 0x0002;  // Set to fixed priority
#endif

	// MIPS priority to high, samuel 081022
	*(volatile unsigned int*)(0xbf200000+(0x924<<2)) = 0xFFFC ;
	*(volatile unsigned int*)(0xbf200000+(0x903<<2)) &= ~(1<<14) ;
	*(volatile unsigned int*)(0xbf200000+(0x920<<2)) |= (1<<1) ; // triger priority setting working
	*(volatile unsigned int*)(0xbf200000+(0x920<<2)) &= ~(1<<1) ; // triger priority setting working, samuel 081022

#if S7_TEMP_PATCH_KER
	// MIU1
	// Group priority config
	// MIPS MIU1 group priority for 264
#if 0
	*(volatile unsigned int*)(0xbf200000+(0x36E<<2)) &= 0xFE00 ;
	*(volatile unsigned int*)(0xbf200000+(0x36E<<2)) |= 0x1B ;  // 3>2>1>0
	*(volatile unsigned int*)(0xbf200000+(0x36E<<2)) |= (1<<8) ; // triger priority setting working
#endif

	*(volatile unsigned int*)(0xbf200000+(0x36E<<2)) &= 0xFEFF;  // Need to set it default priority before set fixed priority
	*(volatile unsigned int*)(0xbf200000+(0x36E<<2)) &= 0xFF00;  // Clear the priority
	*(volatile unsigned int*)(0xbf200000+(0x36E<<2)) |= 0x001B;
	*(volatile unsigned int*)(0xbf200000+(0x36E<<2)) |= 0x0100;  // Set to fixed priority
#endif

#endif


#if defined(CONFIG_Titania2)
	//samuel 081022 //*((volatile unsigned int *) (0xbf80240c)) |= 0x4000;        // MIU group 1 priority > group 0
	*((volatile unsigned int *) (0xbf8024c0)) |= 0x000b;        // Group 1 round robin enable; Group 1 fixed priority enable; Group 1 group limit enable
	*((volatile unsigned int *) (0xbf802500)) |= 0x000E;        // Group 2 seeting
	*((volatile unsigned int *) (0xbf802504)) |= 0x0004;        // Group 2 member max.

	// MIPS priority to high, samuel 081022
	*(volatile unsigned int*)(0xbf800000+(0x1248<<1)) = 0xFF9F ;
	*(volatile unsigned int*)(0xbf800000+(0x124A<<1)) = 0x1065 ;
	*(volatile unsigned int*)(0xbf800000+(0x124C<<1)) = 0x7432 ;
	*(volatile unsigned int*)(0xbf800000+(0x1206<<1)) &= ~(1<<14) ;
	*(volatile unsigned int*)(0xbf800000+(0x1240<<1)) |= (1<<1) ; // triger priority setting working
	*(volatile unsigned int*)(0xbf800000+(0x1240<<1)) &= ~(1<<1) ; // triger priority setting working, samuel 081022
#endif
}

static void __init miu_assignment(void)
{
#if defined(CONFIG_MSTAR_TITANIA2)
	_RV2(0x12F0,
			(MIUSEL_FLAG_FSEICH_R   << 0x09)    |\
			(MIUSEL_FLAG_SVD_DB_R   << 0x0C)    |\
			(MIUSEL_FLAG_SVD_DB_W   << 0x0D)    |\
			(MIUSEL_FLAG_FDECICH_R  << 0x0E));

	_RV2(0x12F2,
			(MIUSEL_FLAG_OPM_R      << 0x00)    |\
			(MIUSEL_FLAG_MVOP_R     << 0x01)    |\
			(MIUSEL_FLAG_GOP0_W     << 0x02)    |\
			(MIUSEL_FLAG_GOP0_R     << 0x03)    |\
			(MIUSEL_FLAG_GOP1_R     << 0x04)    |\
			(MIUSEL_FLAG_DNRB_W     << 0x06)    |\
			(MIUSEL_FLAG_DNRB_R     << 0x07)    |\
			(MIUSEL_FLAG_COMB_R     << 0x08)    |\
			(MIUSEL_FLAG_COMB_W     << 0x09)    |\
			(MIUSEL_FLAG_VE_R       << 0x0A)    |\
			(MIUSEL_FLAG_VE_W       << 0x0B)    |\
			(MIUSEL_FLAG_OD_R       << 0x0C)    |\
			(MIUSEL_FLAG_OD_W       << 0x0D)    |\
			(MIUSEL_FLAG_FSEDMA2_RW << 0x0E)    |\
			(MIUSEL_FLAG_MAD_RW     << 0x0F));

	_RV2(0x12F4,
			(MIUSEL_FLAG_SVD_EN_R   << 0x00)    |\
			(MIUSEL_FLAG_MVD_R      << 0x06)    |\
			(MIUSEL_FLAG_AESDMA_W   << 0x07)    |\
			(MIUSEL_FLAG_MVD_W      << 0x0B)    |\
			(MIUSEL_FLAG_JPD_R      << 0x0C)    |\
			(MIUSEL_FLAG_JPD_W      << 0x0D));
#endif
#if defined(CONFIG_MSTAR_TITANIA3)

#if (MIU_SETTING_FOR_BRINGUP == 1)
	*(volatile unsigned int *)(0xBF200000 + (0x978<<2)) = 0x00;
	*(volatile unsigned int *)(0xBF200000 + (0x979<<2)) = 0x00;
	*(volatile unsigned int *)(0xBF200000 + (0x97A<<2)) = 0x00;
	*(volatile unsigned int *)(0xBF200000 + (0x97B<<2)) = 0x00;

#else

	*(volatile unsigned int *)(0xBF200000 + (0x978<<2)) =
		(MIUSEL_FLAG_VIVALDI_DECODER_R   << 0x01) |
		(MIUSEL_FLAG_VIVALDI_SE_R        << 0x02) |
		(MIUSEL_FLAG_MIPS_W              << 0x03) |
		(MIUSEL_FLAG_MIPS_R              << 0x04) |
		(MIUSEL_FLAG_PIU_MAU0_W          << 0x05) |
		(MIUSEL_FLAG_PIU_MAU1_R          << 0x06) |
		(MIUSEL_FLAG_VD_MHEG5_DCACHE_RW  << 0x07) |
		(MIUSEL_FLAG_VD_MHEG5_QDMA_RW    << 0x08) |
		(MIUSEL_FLAG_GE_RW               << 0x09) |
		(MIUSEL_FLAG_HVD_RW              << 0x0A) |
		(MIUSEL_FLAG_HVD_BBU_R           << 0x0B) |
		(MIUSEL_FLAG_USB_UHC0_RW         << 0x0C) |
		(MIUSEL_FLAG_USB_UHC1_RW         << 0x0D) |
		(MIUSEL_FLAG_PIU_BDMA_RW         << 0x0E) ;

	*(volatile unsigned int *)(0xBF200000 + (0x979<<2)) =
		(MIUSEL_FLAG_VIVALDI_MAD_RW   << 0x00) |
		(MIUSEL_FLAG_VIVALDI_DMA_RW   << 0x01) |
		(MIUSEL_FLAG_VIVALDI_AUDMA_RW << 0x02) |
		(MIUSEL_FLAG_MVOP_R           << 0x03) |
		(MIUSEL_FLAG_GOP_W           << 0x04) |
		(MIUSEL_FLAG_GOP0_R           << 0x05) |
		(MIUSEL_FLAG_GOP1_R           << 0x06) |
		(MIUSEL_FLAG_GOP2_R           << 0x07) |
		(MIUSEL_FLAG_GOP3_R           << 0x08) |
		(MIUSEL_FLAG_SC_OP_R          << 0x09) |
		(MIUSEL_FLAG_SC_IPMAIN_W << 0x0A) |
		(MIUSEL_FLAG_SC_IPMAIN_R      << 0x0B) |
		(MIUSEL_FLAG_VD_COMB_W        << 0x0C) |
		(MIUSEL_FLAG_VD_COMB_R        << 0x0D) |
		(MIUSEL_FLAG_VE_W             << 0x0E) |
		(MIUSEL_FLAG_VE_R             << 0x0F) ;


	*(volatile unsigned int *)(0xBF200000 + (0x97A<<2)) =
		(MIUSEL_FLAG_OD_W              << 0x00) |
		(MIUSEL_FLAG_OD_R << 0x01) |
		(MIUSEL_FLAG_OD_LSB_W << 0x02) |
		(MIUSEL_FLAG_OD_LSB_R << 0x03) |
		(MIUSEL_FLAG_SC_IPSUB_W << 0x04) |
		(MIUSEL_FLAG_SC_IPSUB_R        << 0x05) |
		(MIUSEL_FLAG_DISP_IPATH_NR_RW  << 0x06) |
		(MIUSEL_FLAG_DISP_IPATH_MR_RW  << 0x07) |
		(MIUSEL_FLAG_DISP_IPATH_DI_W   << 0x08) |
		(MIUSEL_FLAG_VD_MHEG5_ICACHE_R << 0x09) |
		(MIUSEL_FLAG_TSP_W             << 0x0A) |
		(MIUSEL_FLAG_TSP_R             << 0x0B) |
		(MIUSEL_FLAG_VD_TTX_RW         << 0x0C) |
		(MIUSEL_FLAG_VD_TTXSL_W        << 0x0D) |
		(MIUSEL_FLAG_TSP_ORZ_W         << 0x0E) |
		(MIUSEL_FLAG_TSP_ORZ_R         << 0x0F) ;


	*(volatile unsigned int *)(0xBF200000 + (0x97B<<2)) =
		(MIUSEL_FLAG_MVOP_W              << 0x00) |
		(MIUSEL_FLAG_M4VE_0_RW           << 0x01) |
		(MIUSEL_FLAG_M4VE_ME_R           << 0x02) |
		(MIUSEL_FLAG_M4VE_2_RW           << 0x03) |
		(MIUSEL_FLAG_MVD_RW              << 0x04) |
		(MIUSEL_FLAG_MVD_BBU_RW          << 0x05) |
		(MIUSEL_FLAG_RVD_RW              << 0x06) |
		(MIUSEL_FLAG_RVD_BBU_R           << 0x07) |
		(MIUSEL_FLAG_JPD_RW              << 0x08) |
		(MIUSEL_FLAG_DSCRMB_RW           << 0x09) |
		(MIUSEL_FLAG_STRLD_RW            << 0x0A) |
		(MIUSEL_FLAG_PIU_HISPEED_UART_RW << 0x0B) |
		(MIUSEL_FLAG_EMAC_RW             << 0x0C) |
		(MIUSEL_FLAG_NAND_RW             << 0x0D) |
		(MIUSEL_FLAG_MPIF_RW             << 0x0E) |
		(MIUSEL_FLAG_G3D_RW              << 0x0F) ;


	// Set the MIU1 to enable IP decided by itself.
	// **************************************************
	// Enable BDMA to access MIU 0 & 1 for using MSTV tool
	*((volatile unsigned int *)0xBF200DE0) |= 0x4000;
	// **************************************************

	//(*(volatile unsigned int *)(0xBF000000 + (0x1006F0<<1))) |= (MIUSEL_FLAG_GE_RW         << 0x09) ;
	(*(volatile unsigned int *)(0xBF200000 + (0x378<<2))) |=
		(MIUSEL_FLAG_GE_RW   << 0x09) ;

	//for GE to access both MIU 0 and MIU 1
	//BF000000 + 1012F0*2 bit 9
	//BF000000 + 1006F0*2 bit 9
#endif
#endif
}

void Chip_Flush_Memory(void)
{
	static unsigned char u8_4Lines[64];
	volatile unsigned char *pu8;
	volatile unsigned char tmp;

	// Transfer the memory to noncache memory
	pu8 = ((volatile unsigned char *)(((unsigned int)u8_4Lines) | 0xa0000000));

	// Flush the data from pipe and buffer in MIU0
	pu8[0] = pu8[16] = pu8[32] = pu8[48] = 1;

	// Flush the data in the EC bridge buffer
	mb();

	// final read
	tmp = pu8[0] ;
	tmp = pu8[16] ;
	tmp = pu8[32] ;
	tmp = pu8[48] ;

	// Transfer the memory to noncache memory
	pu8 =(volatile unsigned char*)(MIPS_MIU1_BASE+(BB_ADR-MIU1_MEM_BASE_ADR));

	// Flush the data from pipe and buffer in MIU1
	pu8[0] = pu8[16] = pu8[32] = pu8[48] = 1;

	// Flush the data in the EC bridge buffer
	mb();

	// final read
	tmp = pu8[0] ;
	tmp = pu8[16] ;
	tmp = pu8[32] ;
	tmp = pu8[48];
}

void Chip_Read_Memory(void)
{
	volatile unsigned int *pu8;
	volatile unsigned int t ;

	// Transfer the memory to noncache memory
	pu8 = ((volatile unsigned int *)0xA0380000);
	t = pu8[0] ;
	t = pu8[64] ;

	pu8 = ((volatile unsigned int *)0xC0380000);
	t = pu8[0] ;
	t = pu8[64] ;
}

unsigned int Chip_Query_MIPS_CLK(void)
{
	unsigned int u32Count = 0;
	unsigned int uSpeed_L = 0, uSpeed_H = 0;

#if 0
	if ((((*(volatile u32*)0xBF221864) & 0xFF33) != 0x0010 ) &&
			(((*(volatile u32*)0xBF221868) & 0xFF00) != 0x0000 ))
	{
		u32Count = 12000000;
		printk("MIPS clock-setting regisers have wrong initial values\n");
	}
	else
	{
		// Get REG_MPLL_LOOP_DIV1
		u32Count =  1 << (((*((volatile u32*)0xBF221864)) & 0x000C) >> 2);
		if (((*((volatile u32*)0xBF221868)) & 0x00FF) != 0)
		{
			// Get REG_MPLL_LOOP_DIV2
			u32Count *= ((*((volatile u32*)0xBF221868)) & 0x00FF);
		}
		// MIP CLK = 24M(Crystal) * REG_MPLL_LOOP_DIV1 * REG_MPLL_LOOP_DIV2 / 4
		u32Count *= (24000000 >> 2);
	}
#else
	uSpeed_L = *(volatile u32*)(0xBF221864);
	uSpeed_H = *(volatile u32*)(0xBF221868);

	if(uSpeed_H==0x1c)
	{
		//printf("(CPU Speed is 12 MHz)\n");
		u32Count = 12000000;
	}
	else if(uSpeed_L==0x14)
	{
		//printf("(CPU Speed %d MHz)\n", (int)(uSpeed_H*24));
		u32Count = uSpeed_H*24*1000000;
	}
	else if(uSpeed_L==0x18)
	{
		//printf("(CPU Speed %d MHz setting)\n", (int)(uSpeed_H*48));
		u32Count = uSpeed_H*48*1000000;
	}
#endif

	return u32Count;
}


unsigned int Chip_Query_Rev(void)
{
	unsigned int u32Rev;

	u32Rev = *((volatile unsigned int*)(0xBF003D9C));
    u32Rev = u32Rev >> 8;

	return u32Rev;
}
EXPORT_SYMBOL(Chip_Query_Rev);

int MDrv_SYS_GetMMAP(int type, unsigned int *addr, unsigned int *len)
{
	switch(type)
	{
		case E_SYS_MMAP_LINUX_BASE:
#ifdef LINUX_MEM_BASE_ADR
			*addr	= LINUX_MEM_BASE_ADR;
			*len	= LINUX_MEM_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_VD_3DCOMB:
#ifdef VD_3DCOMB_BASE_ADR
			*addr	= VD_3DCOMB_BASE_ADR;
			*len	= VD_3DCOMB_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MAD_BASE:
#ifdef MAD_BASE_BUFFER_ADR
			*addr	= MAD_BASE_BUFFER_ADR;
			*len	= MAD_BASE_BUFFER_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MVD_FB:
#ifdef MVD_FRAMEBUFFER_ADR
			*addr	= MVD_FRAMEBUFFER_ADR;
			*len	= MVD_FRAMEBUFFER_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MVD_BS:
#ifdef MVD_BITSTREAM_ADR
			*addr	= MVD_BITSTREAM_ADR;
			*len	= MVD_BITSTREAM_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_EMAC:
#ifdef EMAC_BUF_ADR
			*addr	= EMAC_BUF_ADR;
			*len	= EMAC_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_VE:
#ifdef VE_FRAMEBUFFER_ADR
			*addr	= VE_FRAMEBUFFER_ADR;
			*len	= VE_FRAMEBUFFER_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_SCALER_DNR_BUF:
#ifdef SCALER_DNR_BUF_ADR
			*addr	= SCALER_DNR_BUF_ADR;
			*len	= SCALER_DNR_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_TTX_BUF:
#ifdef TTX_BUF_ADR
			*addr	= TTX_BUF_ADR;
			*len	= TTX_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MPOOL:
#ifdef MPOOL_ADR
			*addr	= MPOOL_ADR;
			*len	= MPOOL_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_LINUX_2ND_MEM:
#ifdef LINUX_2ND_MEM_ADDR
			*addr	= LINUX_2ND_MEM_ADDR;
			*len	= LINUX_2ND_MEM_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_TSP:  // samuel, 20081107
#ifdef TSP_BUF_AVAILABLE
			*addr	= TSP_BUF_ADR;
			*len	= TSP_BUF_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_SVD:
#ifdef SVD_CPU_AVAILABLE
			*addr	= SVD_CPU_ADR;
			*len	= SVD_CPU_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_MVD_SW:
#ifdef MVD_SW_AVAILABLE
			*addr	= MVD_SW_ADR;
			*len	= MVD_SW_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_SVD_ALL:
#ifdef SVD_CPU_AVAILABLE
			*addr	= SVD_CPU_ADR;
#if 0 // Alan.Chen for JPD ping-pong issue
			*len	= SVD_CPU_LEN + MVD_FRAMEBUFFER_LEN + MVD_BITSTREAM_LEN + JPD_OUTPUT_LEN;
#else
			*len    = SVD_CPU_LEN + MVD_FRAMEBUFFER_LEN + MVD_BITSTREAM_LEN + JPD_OUTPUT_LEN + MVD_SW_LEN;
#endif
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_AUDIO_CLIP_MEM:  // samuel, 20081107
#ifdef AUDIO_CLIP_AVAILABLE
			*addr	= AUDIO_CLIP_ADR;
			*len	= AUDIO_CLIP_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_POSD0_MEM:
#ifdef POSD0_AVAILABLE
			*addr	= POSD0_ADR;
			*len	= POSD0_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_POSD1_MEM:
#ifdef POSD1_AVAILABLE
			*addr	= POSD1_ADR;
			*len	= POSD1_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_BIN_MEM:
#ifdef BIN_MEM_AVAILABLE
			*addr	= BIN_MEM_ADR;
			*len	= BIN_MEM_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_JPD_MEM: // Samuel, 090108
#ifdef JPD_OUTPUT_AVAILABLE
			*addr	= JPD_OUTPUT_ADR;
			*len	= JPD_OUTPUT_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_BT_POOL:
#ifdef BT_POOL_AVAILABLE
			*addr	= BT_POOL_ADR;
			*len	= BT_POOL_LEN;
#else
			return false;
#endif
			break;

#if defined(CONFIG_MSTAR_TITANIA_BD_T3_FPGA)
		case E_SYS_MMAP_FPGA_POOL_BASE:
#ifdef FPGA_POOL_BASE_BUFFER_ADR
			*addr	= FPGA_POOL_BASE_BUFFER_ADR;
			*len	= FPGA_POOL_BASE_BUFFER_LEN;
#else
			return false;
#endif
			break;
#endif

#if defined(CONFIG_MSTAR_TITANIA_BD_GP2_DEMO1)
		case E_SYS_MMAP_FPGA_POOL_BASE:
#ifdef FPGA_POOL_BASE_BUFFER_ADR
			*addr	= FPGA_POOL_BASE_BUFFER_ADR;
			*len	= FPGA_POOL_BASE_BUFFER_LEN;
#else
			return false;
#endif
			break;
#endif

		case E_SYS_MMAP_M4VE:
#ifdef M4VE_BUF_ADR
			*addr   = M4VE_BUF_ADR;
			*len    = M4VE_BUF_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_JPG_OSD:
#ifdef JPG_OSD_ADR
			*addr   = JPG_OSD_ADR;
			*len    = JPG_OSD_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_PVR_DOWNLOAD_MEM: // StevenL, 090311
#ifdef PVR_DOWNLOAD_BUFFER_AVAILABLE
			*addr   = PVR_DOWNLOAD_BUFFER_ADR;
			*len    = PVR_DOWNLOAD_BUFFER_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_PVR_UPLOAD_MEM: // StevenL, 090311
#ifdef PVR_UPLOAD_BUFFER_AVAILABLE
			*addr	= PVR_UPLOAD_BUFFER_ADR;
			*len	= PVR_UPLOAD_BUFFER_LEN;
#else
			return false;
#endif
			break;
		case E_SYS_MMAP_PVR_THUMBNAIL_DECODE_MEM: // StevenL, 090311
#ifdef PVR_THUMBNAIL_DECODE_BUFFER_AVAILABLE
			*addr   = PVR_THUMBNAIL_DECODE_BUFFER_ADR;
			*len    = PVR_THUMBNAIL_DECODE_BUFFER_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_APVR_BASE:
#ifdef APVR_BUF_ADR
			*addr   = APVR_BUF_ADR;
			*len    = APVR_BUF_LEN;
#else
			return false;
#endif
			break;


		case E_SYS_MMAP_MIU1_BASE:
#ifdef MIU1_MEM_BASE_ADR
			*addr   = MIU1_MEM_BASE_ADR;
#if 0 //dhjung LGE
			*len    = MIU1_MEM_BASE_LEN;
#else
			if (sys_memsize == E_SYS_MMAP_512MB) {
				printk(KERN_DEBUG "Get the MIU1 Base Addr from 512M DDR\n");
				*len    = 0x10000000;
			} else if (sys_memsize == E_SYS_MMAP_256MB) {
				printk(KERN_DEBUG "Get the MIU1 Base Addr from 256M DDR\n");
				*len    = 0x8000000;
			}
#endif
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_MIU1_POOL:
#ifdef MIU1_POOL_ADR
			*addr   = MIU1_POOL_ADR;
			*len    = MIU1_POOL_LEN;
#else
			return false;
#endif
			break;


		case E_SYS_MMAP_OD_MSB_BUFFER:
#ifdef OD_MSB_BUFFER_AVAILABLE
			*addr	= OD_MSB_BUFFER_ADR;
			*len	= OD_MSB_BUFFER_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_OD_LSB_BUFFER:
#ifdef OD_LSB_BUFFER_AVAILABLE
			*addr	= OD_LSB_BUFFER_ADR;
			*len	= OD_LSB_BUFFER_LEN;
#else
			return false;
#endif
			break;

		case E_SYS_MMAP_BB:
#ifdef BB_ADR
			*addr   = BB_ADR;
			*len    = BB_LEN;
#else
			return false;
#endif
			break;

        case E_SYS_MMAP_DEBUG:
    #ifdef DEBUG_ADR
                *addr   = DEBUG_ADR;
                *len    = DEBUG_LEN;
    #else
	    		return false;
    #endif
			    break;
			case E_SYS_MMAP_SC_BUF:
#ifdef SC_BUF_ADR
			*addr	= SC_BUF_ADR;
			*len	= SC_BUF_LEN;
#else
			return false;
#endif
				break;

		default:
			return false;
	}
	return true;
}

EXPORT_SYMBOL(MDrv_SYS_GetMMAP);


//dhjung LGE
EXPORT_SYMBOL(Chip_Flush_Memory);
EXPORT_SYMBOL(Chip_Read_Memory);

#include <linux/syscalls.h>
#include <linux/fadvise.h>
int MDrv_fadvise( int fd ){
    int enable ;
    enable = (fd&1) ;
    fd = fd>>1 ;
    if( enable )
	    return sys_fadvise64_64(fd, 0, 0, POSIX_FADV_SEQUENTIAL) ;
	else
        return sys_fadvise64_64(fd, 0, 0, POSIX_FADV_NORMAL) ;	
}
EXPORT_SYMBOL(MDrv_fadvise);

// Samuel, 090108
// BW saving patch for input source that don't need decoder
//==================================================================
DECLARE_MUTEX(q_recover_sem);
static spinlock_t _q_spinlock;
static int _q_spinlock_init = 0 ;
static unsigned int q_recover_jiffies = 0 ;
static unsigned int q_recover_mode = 0 ;
EXPORT_SYMBOL(q_recover_mode);
static int q_tune = 0 ;
static int q_restore = 0 ;
static int q_highsave = 0 ;

// 0=normal quality
// 1=need recover
static int q_st = 0 ;
void Q_RECOVER( void ){
	int cnt ;

	cnt = 0 ;
	while( 0!=down_trylock(&q_recover_sem) ){
		cnt++ ;
		mdelay(1) ;
		if( cnt>=50 )
			return ;
	}

	if( 0==_q_spinlock_init ){
		spin_lock_init(&_q_spinlock);
		_q_spinlock_init = 1 ;
	}

	if( 0==q_st ){
		if( q_recover_jiffies>jiffies ){
			q_st = 1 ;
			q_tune = 1 ;
		}
	}else{
		if( jiffies>=q_recover_jiffies ){
			q_st = 0 ;
			q_restore = 1 ;
		}
	}
	up(&q_recover_sem) ;
}
EXPORT_SYMBOL(Q_RECOVER);


void Q_UPDATE( void ){
	unsigned int flags ;
	unsigned int bank ;

	if( q_tune ){
		//tune
		spin_lock_irqsave(&_q_spinlock, flags);
		*(volatile unsigned int*)(0xbf000000+(0x101268<<1)) = 0x3FFF ;
		bank = *(volatile unsigned int*)(0xbf800000+(0x102F00<<1)) ;
		*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = 0x0012 ;
		*(volatile unsigned int*)(0xbf000000+(0x102F08<<1)) |= (1<<8) ;
		*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = bank ;
		spin_unlock_irqrestore(&_q_spinlock, flags);
		q_tune = 0 ;
	}

	if( q_restore ){
		//restore
		spin_lock_irqsave(&_q_spinlock, flags);
		bank = *(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) ;
		*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = 0x0012 ;
		*(volatile unsigned int*)(0xbf000000+(0x102F08<<1)) &= ~(1<<8) ;
		*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = bank ;
		*(volatile unsigned int*)(0xbf000000+(0x101268<<1)) = 0x7FFF ;
		spin_unlock_irqrestore(&_q_spinlock, flags);
		q_restore = 0 ;
	}
}
EXPORT_SYMBOL(Q_UPDATE);


void Q_SETTIMER( void ){
	return ;
	if( 0==q_recover_mode || 0==q_highsave )
		return ;
	q_recover_jiffies = jiffies+(HZ>>3) ;
	//    Q_RECOVER() ;
	//    Q_UPDATE() ;
}
EXPORT_SYMBOL(Q_SETTIMER);

void Q_SETMODE( int inqmode ){
	unsigned int flags ;
	unsigned int bank ;

	if( 0==_q_spinlock_init ){
		spin_lock_init(&_q_spinlock);
		_q_spinlock_init = 1 ;
	}

	spin_lock_irqsave(&_q_spinlock, flags);
	bank = *(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) ;

	// disbale input source
	//*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = 0x0001 ;
	//*(volatile unsigned int*)(0xbf000000+(0x102F04<<1)) |= (1<<7) ;
	//mdelay(1) ; // stablize MIU

	if( inqmode ){
		// shift Scaler buffer to MIU1
		//*(volatile unsigned int*)(0xbf000000+(0x1012F2<<1)) |= (0x00C1) ;
		// tune YNR only and 8bit motion
		//*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = 0x0012 ;
		//*(volatile unsigned int*)(0xbf000000+(0x102F04<<1)) |= (1<<10) ;

		q_recover_mode = 1 ;
	}else{
		// shift Scaler buffer to MIU0
		//*(volatile unsigned int*)(0xbf000000+(0x1012F2<<1)) &= ~(0x00C1) ;
		// restore
		//*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = 0x0012 ;
		//*(volatile unsigned int*)(0xbf000000+(0x102F04<<1)) &= ~(1<<10) ;

		q_recover_mode = 0 ;
	}

	//enable input source
	//*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = 0x0001 ;
	//*(volatile unsigned int*)(0xbf000000+(0x102F04<<1)) &= ~(1<<7) ;

	*(volatile unsigned int*)(0xbf000000+(0x102F00<<1)) = bank ;
	spin_unlock_irqrestore(&_q_spinlock, flags);
}
EXPORT_SYMBOL(Q_SETMODE);

#if 1
void Q_HIGHSAVE( int mode ){
	q_highsave = mode ;
}
EXPORT_SYMBOL(Q_HIGHSAVE);
#endif

//==================================================================
// End of Samuel, 090108


// EMP MOVIE status, Samuel 090109
int g_emp_movie_in_play = 0 ;
EXPORT_SYMBOL(g_emp_movie_in_play);

unsigned int mpool_userspace_base = 0 ; // base address for userspace mmap
EXPORT_SYMBOL(mpool_userspace_base);
