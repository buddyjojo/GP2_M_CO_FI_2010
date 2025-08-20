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
// #include <linux/config.h>
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

#include "chip_int.h"
#include "hwreg_S5.h"
#ifdef CONFIG_VT
#include <linux/console.h>
#endif

#include "board/Board.h"

#if defined (CONFIG_MSTAR_TITANIA_BD_S5_LG_DEMO_BOARD_1)
#define USE_LGE_DEMO_BOARD
#endif

const char display_string[] = "        LINUX ON Titania 2       ";

/******************************************************************************/
/* Macro for bitwise                                                          */
/******************************************************************************/
#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

//------------------------------------------------------------------------------
// MIU assignment   0: MIU0, 1: MIU1
//------------------------------------------------------------------------------
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
// 3DCOM switch to miu1, samuel, 20081105
#define MIUSEL_FLAG_COMB_R          1
#define MIUSEL_FLAG_COMB_W          1
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
//------------------------------------------------------------------------------

extern void mips_reboot_setup(void);
extern void mips_time_init(void);
extern void mips_timer_setup(struct irqaction *irq);
extern unsigned long mips_rtc_get_time(void);

static void __init serial_init(void);
static void __init chiptop_init(void);
static void __init pad_config(void);
static void __init miu_priority_config(void);
static void __init miu_assignment(void);

#if 0
struct resource standard_io_resources[] = {
	{ "dma1", 0x00, 0x1f, IORESOURCE_BUSY },
	{ "timer", 0x40, 0x5f, IORESOURCE_BUSY },
	{ "keyboard", 0x60, 0x6f, IORESOURCE_BUSY },
	{ "dma page reg", 0x80, 0x8f, IORESOURCE_BUSY },
	{ "dma2", 0xc0, 0xdf, IORESOURCE_BUSY },
};
#endif

#if 0//def CONFIG_MTD
static struct mtd_partition malta_mtd_partitions[] = {
	{
		.name =		"YAMON",
		.offset =	0x0,
		.size =		0x100000,
		.mask_flags =	MTD_WRITEABLE
	},
	{
		.name =		"User FS",
		.offset = 	0x100000,
		.size =		0x2e0000
	},
	{
		.name =		"Board Config",
		.offset =	0x3e0000,
		.size =		0x020000,
		.mask_flags =	MTD_WRITEABLE
	}
};

#define number_partitions	(sizeof(malta_mtd_partitions)/sizeof(struct mtd_partition))
#endif

const char *get_system_type(void)
{
	return "MStar Titania";
}

extern int MDrv_SYS_GetMMAP(int type, unsigned int *addr, unsigned int *len) ;

static int __init Titania_setup(void)
{
//	int i;

#if 0 //samuel comment
	*((char*)0xbf804b80) = 0x00;  // Dean
	*((char*)0xbf807558) = *((char*)0xbf807558) | 0x04;  // Dean
	*((char*)0xbf80750c) = *((char*)0xbf80750c) | 0x04;  // Dean
	*((char*)0xbf807500) = 0x00;  // Dean
	*((char*)0xbf807501) = 0x00;  // Dean
	*((char*)0xbf800e00) = *((char*)0xbf800e00) | 0x28;  // Dean
#endif

	printk(KERN_DEBUG "Begin Titania_setup\n");

#if defined(CONFIG_MSTAR_TITANIA2)

#if defined(CONFIG_MSTAR_TITANIA_MMAP_32MB_32MB) || \
	defined(CONFIG_MSTAR_TITANIA_MMAP_64MB_32MB) || \
	defined(CONFIG_MSTAR_TITANIA_MMAP_64MB_64MB) || \
	defined(CONFIG_MSTAR_TITANIA_MMAP_128MB_128MB)

	// for MIU assignment
	miu_assignment();
#endif

#endif

	// **************************************************
	// disable XIU no-ack timeout issue, for using MSTV tool
	*((char*)0xBF801D80) = *((char*)0xBF801D80) | 0x0001;
	*((unsigned short*)0xBF801DA8) = 0xFFFF;
	// **************************************************

#if 0
	// setting DRAM size
#if(CONFIG_MSTAR_TITANIA_MMAP_32MB)
	*((volatile unsigned short *)0xbf802404) = 0x0345; //32M
#elif(CONFIG_MSTAR_TITANIA_MMAP_64MB)
	*((volatile unsigned short *)0xbf802404) = 0x0545; //64M
#endif
#endif
#if 0 //dhjung LGE
	// show memory mapping information
	printk("\n=====================================");
	printk("\nmemory mapping\n");
	printk("=====================================\n");
	for(i=0; i < 12; ++i)
	{
		int addr, len;
		if(MDrv_SYS_GetMMAP(i, &addr, &len))
			printk("[%2i] start: %x   end: %x   len: %d\n", i, addr, addr+len-1, len);
		else
			printk("[%i] not defined in memory mapping table\n", i);
	}
#endif

#if 1
	chiptop_init();
#endif

#if 1
	pad_config();

#ifdef USE_LGE_DEMO_BOARD
	//for BD_LGE_S5_DEMO, UART_TX2 pull low for LG power board
	volatile unsigned int *reg;
	// set UART_TX2
	reg = (volatile unsigned int *)(0xbf803c08);
	*reg = ((*reg) & 0xf3ff);
	reg = (volatile unsigned int *)(0xbf803c0c);
	*reg = ((*reg) & 0xf1ff);
	// UART_TX2 eon
	reg = (volatile unsigned int *)(0xbf803cc0);
	*reg = ((*reg) & 0xfffd);
	// UART_TX2 pull low
	reg = (volatile unsigned int *)(0xbf803cbc);
	*reg = ((*reg) & 0xfffd);
#endif

#endif

	miu_priority_config();

#ifdef USE_LGE_DEMO_BOARD
	// set scaler buffer into MIU1
	*((char*)0xBF8025E4) = 0xC1;
#endif

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

	Uart16550IntrruptEnable();
	//while(1) Uart16550Put('*');
	s.type = PORT_16550;
	s.iobase = 0xBF801300;
	s.irq = E_IRQ_UART;

#ifdef CONFIG_Titania2

	// for FPGA
	//s.uartclk = 14318180;
	//s.uartclk = 123000000;
	//s.uartclk = 144000000;
	//s.uartclk = 160000000;
	s.uartclk = 172800000;
	//s.uartclk = 12000000;
	//s.uartclk = 108000000;

#else

#ifdef CONFIG_MSTAR_TITANIA_BD_FPGA
	s.uartclk = FPGA_XTAL_CLOCK;    //12000000;///14318180;   //43200000;
#else
	s.uartclk = 123000000;
#endif

#endif
	s.iotype = 0;
	s.regshift = 0;
	s.fifosize = 16 ; // use the 8 byte depth FIFO well

	if (early_serial_setup(&s) != 0) {
		printk(KERN_ERR "Serial setup failed!\n");
	}
#endif
}

static void __init _RV1(unsigned int addr, unsigned short value)
{
	if(addr & 1)
		(*(volatile unsigned char *)(0xbf800000+((addr-1)<<1)+1))=(unsigned char)(value & 0xFF);
	else
		(*(volatile unsigned char *)(0xbf800000+(addr<<1)))=(unsigned char)(value & 0xFF);
}

static void __init _RV2(unsigned int addr, unsigned short value)
{
	(*(volatile unsigned short *)(0xbf800000+(addr<<1)))=(unsigned short)(value & 0xFFFF);
}

static void __init _RV3(unsigned int addr, unsigned int value)
{
	_RV1(addr, value & 0xFF);
	_RV1(addr+1, (value>>8) & 0xFF);
	_RV1(addr+2, (value>>16) & 0xFF);
}

static void __init _RV4(unsigned int addr, unsigned int value)
{
	(*(volatile unsigned int *)(0xbf800000+(addr<<1)))=(unsigned int)value;
}

static void __init MDrv_WriteByteMask(unsigned int addr, unsigned char value, unsigned char u8Mask)
{
	volatile unsigned char *reg;

	if(addr & 1)
		reg = (volatile unsigned char *)(0xbf800000+((addr-1)<<1)+1);
	else
		reg = (volatile unsigned char *)(0xbf800000+(addr<<1));

	*reg = ((*reg) & (~u8Mask)) | (value & u8Mask);
}

static void __init MDrv_WriteRegBit(unsigned int addr, int bBit, unsigned char u8BitPos)
{
	volatile unsigned char *reg;

	if(addr & 1)
		reg = (volatile unsigned char *)(0xbf800000+((addr-1)<<1)+1);
	else
		reg = (volatile unsigned char *)(0xbf800000+(addr<<1));

	*reg = (bBit) ? ( *reg | u8BitPos) : (*reg | ~u8BitPos);
}
#define CONFIG_IIC_DSP
#ifdef CONFIG_IIC_DSP
void Chip_Flush_Memory(void)
{
    static unsigned char u8_4Lines[64];
    unsigned char *pu8;

    // Transfer the memory to noncache memory
    pu8 = ((unsigned char *)(((unsigned int)u8_4Lines) | 0xa0000000));

    // Flush the data from pipe and buffer in MIU
    pu8[0] = pu8[16] = pu8[32] = pu8[48] = 1;

    // Flush the data in the EC bridge buffer
    mb();
}
EXPORT_SYMBOL(Chip_Flush_Memory);

//
// Loading off-loading DSP. Samuel.Huang 20090605
//================================================================================================
#include "_aeon_bin.h"

#define REG_ADDR(addr)              (*((volatile unsigned short*)(0xBF800000 + (addr << 1))))
#define REG_CKG_AEONTS0                         0x1E26
#define AEON_CLK_DISABLE                        0x40
    //---------------------------------------------
    // definition for AEON_REG_CTRL   //reg[0x0FF0]
    #define AEON_CTRL_EN                            BIT0
    #define AEON_CTRL_RST                           BIT1
    #define AEON_DWB_SWAP                           BIT3
#define REG_SW_RESET_CPU_AEON                   0x1086
    //---------------------------------------------
    // definition for REG_SW_RESET_CPU_AEON   //reg[0x1086]
    #define AEON_SW_RESET                           BIT0

void load_i2c_aeon( void ){

    int i, cnt ;
    unsigned int* p = (unsigned int*)0xA0000000 ;
    unsigned int* s = (unsigned int*)_aeon_i2c_bin ;

    // load code.
    cnt = (sizeof(_aeon_i2c_bin)>>2)+1 ;
    for( i=0; i<cnt; i++ ){
        *p = *s ;
        p++ ;
        s++ ;
    }

    // enable AEON
    REG_ADDR(0xF80) = 0x2; // Let aeon command ¢®¡×in order¢®¡§
    REG_ADDR(REG_CKG_AEONTS0) &= ~AEON_CLK_DISABLE;

    //Mask Aeon FIQ/IRQ
    REG_ADDR(0x2B40) = 0xFFFF;
    REG_ADDR(0x2B42) = 0xFFFF;
    REG_ADDR(0x2B58) = 0xFFFF;
    REG_ADDR(0x2B5A) = 0xFFFF;

    REG_ADDR(0x0FF0) |= (AEON_CTRL_EN + AEON_CTRL_RST);
    REG_ADDR(REG_SW_RESET_CPU_AEON) |= AEON_SW_RESET;
    //==============================================
    // bit0 = 1/0 : Aeon stop/start
    // bit1 = 1 for patch1
    // bit2 = 1 for patch2
    REG_ADDR(0xFE6) = 0x02;
}
//================================================================================================
//

#endif

static void __init chiptop_init(void)
{
//	unsigned int tmp ;
//	int i ;

#if 0
	// The initials are ported from /kingwork/kernel/s4m by Dean Tseng

	// The following 6 setup were done in bootloader

	//_RV4(0x2508, 0x0),      // clear power down
	//_RV2(0x250C, 0x0),      // clear power down clocks

	// MPLL
	//_RV2(0x2514, 0x24),
	//_RV2(0x2510, 0x0506),
	//_RV2(0x2516, 0x0),      // MPLL div clocks enable
	//_RV2(0x25C0, 0x0),

	_RV1(0x1E24, 0x0);                      // USB30

	// DDR MIU was set in bootloader
	//_RV1(0x1E25, CKG_DDR_MEMPLL |           // DDR, MIU
	//             CKG_MIU_MEMPLL),
	_RV1(0x1E26, CKG_TS0_TS0_CLK);          // TS0, TCK, AEON
	_RV1(0x1E27, CKG_TSP_144MHZ |CKG_STC0_STC0_SYNTH);  // TSP, STC0, MAD_STC

	_RV1(0x1E28, CKG_MAD_STC_STC0_SYNTH);   // MAD_STC
	_RV1(0x1E29, CKG_MVD_BOOT_CLK_MIU | CKG_MVD_144MHZ);     // MVD_BOOT, MVD
	_RV1(0x1E2A, CKG_M4V_144MHZ | CKG_DC0_SYNCHRONOUS);      // M4V, DC0

	_RV1(0x1E2C, CKG_GOPG0_ODCLK | CKG_GOPG1_ODCLK);         // GOPG0, GOPG1
	_RV1(0x1E2D, CKG_VD_CLK_VD);            // VD
	_RV1(0x1E2E, CKG_VDMCU_108MHZ | CKG_VD200_216MHZ);        // VDMCU, VD200
	_RV1(0x1E2F, CKG_DHC_86MHZ);            // DHC
	_RV1(0x1E30, CKG_FICLK_F2_CLK_IDCLK2);  // FICLK_F2
	_RV1(0x1E31, CKG_GOPG2_CLK_ODCLK | CKG_PCM_27MHZ);     // GOPG2, PCM
	_RV1(0x1E32, 0);                        // reserved
	_RV1(0x1E33, CKG_VE_27MHZ | CKG_VEDAC_27MHZ);            // VE, VEDEC
	_RV1(0x1E34, CKG_DAC_CLK_ODCLK);        // DAC
	_RV1(0x1E35, CKG_FCLK_MPLL);
	//_RV1(0x1E36, 0x00);                     // FIXME, disable CPU_M later
	_RV1(0x1E37, CKG_ODCLK_XTAL_);          // ODCLK (suggested by janick in bringup)
	_RV1(0x1E38, CKG_VE_IN_CLK_MPEG0);      // VE_IN
	_RV1(0x1E39, CKG_NFIE_62MHZ);           // NFIE
	_RV1(0x1E3A, CKG_TS2_TS2_CLK | CKG_TSOUT_27MHZ);         // TS2, TSOUT
	//UART use XTAL
	_RV1(0x1E3D, CKG_DC1_SYNCHRONOUS);      // DC1    _RV1(0x1E3E, CKG_IDCLK1_XTAL),          // IDCLK1
	_RV1(0x1E3F, CKG_IDCLK2_XTAL);          // IDCLK2
	_RV1(0x1E44, CKG_STRLD_144MHZ);         // STRLD
	//_RV1(0x1E45, CKG_MCU_144MHZ);           // MCU		//RobertYang
	_RV1(0x1E9A, CKG_JPD_123MHZ | CKG_SDIO_XTAL);          // JPD, SDIO
#endif
	//090304_dc power off/on latch mstar pathch apply in h.264 ch
	*((volatile unsigned int*)(0xBF800000 + (0x06F4<<1))) = 0x0400; // MIU0 group2 timeout value
	*((volatile unsigned int*)(0xBF800000 + (0x0626<<1))) = 0x0400; // MIUI group2 timeout value
	*((volatile unsigned int*)(0xBF800000 + (0x06F0<<1))) = 0x8010; // MIU0 group2 timeout enable
	*((volatile unsigned int*)(0xBF800000 + (0x0622<<1))) = 0x8010; // MIU1 group2 timeout enable
	*((volatile unsigned int*)(0xBF800000 + (0x0640<<1))) |= BIT4; // MIU1 group0 timeout enable
	*((volatile unsigned int*)(0xBF800000 + (0x0660<<1))) |= BIT4; // MIU1 group1 timeout enable


	_RV1(0x1E2B, CKG_DHC_SBM_12MHZ | CKG_GE_144MHZ);       // DHC_SBM, GE
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

#ifdef CONFIG_IIC_DSP
	 load_i2c_aeon() ;
#endif
}

// The following macros are defined just for easy porting from 8051 code
#define MDrv_WriteByte(addr, value)     _RV1(addr, value)
#define MDrv_Write2Byte(addr, value)    _RV2(addr, value)
#define MDrv_Write3Byte(addr, value)    _RV3(addr, value)
#define MDrv_Write4Byte(addr, value)    _RV4(addr, value)

static void __init pad_config(void)
{
	// TODO: must be clarify --- Dean
	//Mst_PwmPortInit();

	//-----------------------------------------------------------------
	// JTAG overwrite
	//-----------------------------------------------------------------
	MDrv_WriteByte(0x1E01, 0x07);

	//-----------------------------------------------------------------
	// MOD pads controlled by PAD_TOP
	//-----------------------------------------------------------------

#define FORCE_MODPAD    (0) // no ATCON

	//-----------------------------------------------------------------
	// BT656_IN
	//-----------------------------------------------------------------

#define BT656_IN_LOC    (0) // no BT656 in?

	//-----------------------------------------------------------------
	// TS0
	//-----------------------------------------------------------------
#define TS0_MODE        (PIN_TS0 == PADS_TS0 ? BIT6 : 0)

	MDrv_WriteByteMask(0x1E02, TS0_MODE | BT656_IN_LOC | FORCE_MODPAD, BITMASK(6:0));

	//-----------------------------------------------------------------
	// PWM2 PWM3
	//-----------------------------------------------------------------

	// 0x1EA0[1:0]
#define PWM23_IN_GPIO   ((PIN_PWM2 == PAD_GPIO14 ? BIT0 : 0) | \
		(PIN_PWM3 == PAD_GPIO15 ? BIT1 : 0))

	MDrv_WriteByteMask(0x1EA0, PWM23_IN_GPIO, BITMASK(1:0));

	//-----------------------------------------------------------------
	// I2S / SPDIF
	//-----------------------------------------------------------------
	// 0x1E03[0]
#define I2S_IN_IN_PCMPADS       (PIN_I2S_IN == PADS_PCM ? BIT0 : 0)

	// 0x1EE3[4]
#define I2S_OUT_IN_PCMPADS      (PIN_I2S_OUT == PADS_PCM ? BIT4 : 0)
	// 0x1EE3[5]
#define SPDIF_IN_IN_PCMPADS     (PIN_SPDIF_IN == PAD_PCM_CE_N ? BIT5 : 0)
	// 0x1EE3[6]
#define SPDIF_OUT_IN_PCMPADS    (PIN_SPDIF_OUT == PAD_PCM_A10 ? BIT6 : 0)

	MDrv_WriteByteMask(0x1E03, I2S_IN_IN_PCMPADS, BIT0);
	MDrv_WriteByteMask(0x1EE3, I2S_OUT_IN_PCMPADS |
			SPDIF_IN_IN_PCMPADS |
			SPDIF_OUT_IN_PCMPADS, BITMASK(6:4));

	//-----------------------------------------------------------------
	// UART mux / I2S_OUT_MUTE
	//-----------------------------------------------------------------

	// 0x1E05[1:0]
#define UART1_MODE      ((PIN_UART1 == PADS_UART1_MODE1) ? (BIT0) : \
		(PIN_UART1 == PADS_UART1_MODE2) ? (BIT1) : \
		(PIN_UART1 == PADS_UART1_MODE3) ? (BIT1 | BIT0) : 0)

	// 0x1E05[3:2]
#define UART2_MODE      ((PIN_UART2 == PADS_UART2_MODE1) ? (BIT2) : \
		(PIN_UART2 == PADS_UART2_MODE2) ? (BIT3) : \
		(PIN_UART2 == PADS_UART2_MODE3) ? (BIT3 | BIT2) : 0)

	// 0x1E05[7:6]
#define I2S_OUT_MUTE    ((PIN_I2S_OUT_MUTE == PAD_LHSYNC2) ? BIT6 : \
		(PIN_I2S_OUT_MUTE == PAD_PCM_A14) ? BIT7 : 0)

	MDrv_WriteByteMask(0x1E05, I2S_OUT_MUTE | UART1_MODE | UART2_MODE,
			BITMASK(7:6) | BITMASK(3:0));

	//-----------------------------------------------------------------
	// UART Source, Inv
	//-----------------------------------------------------------------
#if 0   // TODO: Should be verify -- Dean
#define UART_INV    ((UART0_INV ? BIT4 : 0) | \
		(UART1_INV ? BIT5 : 0) | \
		(UART2_INV ? BIT6 : 0))

	MDrv_WriteByteMask(0x1EAA, (UART0_SRC_SEL << 2) | (UART1_SRC_SEL << 5), BITMASK(7:2));
	MDrv_WriteByteMask(0x1EAB, UART_INV | (UART2_SRC_SEL << 0), BITMASK(6:4) | BITMASK(2:0));
#endif

	//-----------------------------------------------------------------
	// MIIC (TODO: don't enable IIC master mode here)
	//-----------------------------------------------------------------

	// 0x1EA1[1:0]      - Enable IIC Master Mode
#define MIIC_MODE   (PIN_MIIC ? BIT1 : 0)

	//MDrv_WriteRegBit(0x1EE0, (PIN_MIIC == PADS_DDCR), BIT7);          // now 0x1EE0[7] is reg_ddcr_gpio_sel
	MDrv_WriteByteMask(0x1EA1, MIIC_MODE, BITMASK(1:0));
	MDrv_WriteByte(0x3422, 0x03);   // default I2C speed is XTAL / 4 / 16 = 187.5kHz

	//-----------------------------------------------------------------
	// GPIO
	//-----------------------------------------------------------------

#define GPIO_P1_ENABLE  (PAD_UART_RX2_IS_GPIO  == GPIO_51PORT_LOW  || \
		PAD_UART_RX2_IS_GPIO  == GPIO_51PORT_HIGH || \
		PAD_UART_TX2_IS_GPIO  == GPIO_51PORT_LOW  || \
		PAD_UART_TX2_IS_GPIO  == GPIO_51PORT_HIGH || \
		PAD_PCM_D5_IS_GPIO    == GPIO_51PORT_LOW  || \
		PAD_PCM_D5_IS_GPIO    == GPIO_51PORT_HIGH || \
		PAD_PCM_D6_IS_GPIO    == GPIO_51PORT_LOW  || \
		PAD_PCM_D6_IS_GPIO    == GPIO_51PORT_HIGH || \
		PAD_LHSYNC2_IS_GPIO   == GPIO_51PORT_LOW  || \
		PAD_LHSYNC2_IS_GPIO   == GPIO_51PORT_HIGH || \
		PAD_LVSYNC2_IS_GPIO   == GPIO_51PORT_LOW  || \
		PAD_LVSYNC2_IS_GPIO   == GPIO_51PORT_HIGH || \
		PAD_GPIO14_IS_GPIO    == GPIO_51PORT_LOW  || \
		PAD_GPIO14_IS_GPIO    == GPIO_51PORT_HIGH || \
		PAD_GPIO15_IS_GPIO    == GPIO_51PORT_LOW  || \
		PAD_GPIO15_IS_GPIO    == GPIO_51PORT_HIGH)

#define GPIO_P1_VAL     ((PAD_UART_RX2_IS_GPIO  == GPIO_51PORT_HIGH ? BIT0 : 0) | \
		(PAD_UART_TX2_IS_GPIO  == GPIO_51PORT_HIGH ? BIT1 : 0) | \
		(PAD_PCM_D5_IS_GPIO    == GPIO_51PORT_HIGH ? BIT2 : 0) | \
		(PAD_PCM_D6_IS_GPIO    == GPIO_51PORT_HIGH ? BIT3 : 0) | \
		(PAD_LHSYNC2_IS_GPIO   == GPIO_51PORT_HIGH ? BIT4 : 0) | \
		(PAD_LVSYNC2_IS_GPIO   == GPIO_51PORT_HIGH ? BIT5 : 0) | \
		(PAD_GPIO14_IS_GPIO    == GPIO_51PORT_HIGH ? BIT6 : 0) | \
		(PAD_GPIO15_IS_GPIO    == GPIO_51PORT_HIGH ? BIT7 : 0))

	// 0x1E62
#define GPIO_OUT        ((PAD_UART_RX2_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_UART_TX2_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_PWM0_IS_GPIO        == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_PWM1_IS_GPIO        == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_PWM2_IS_GPIO        == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_PWM3_IS_GPIO        == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_DDCR_DAT_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_DDCR_CLK_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
		(PAD_DDCR_DA2_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_DDCR_CK2_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_LHSYNC2_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_LVSYNC2_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_GPIO12_IS_GPIO      == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_GPIO13_IS_GPIO      == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_GPIO14_IS_GPIO      == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_GPIO15_IS_GPIO      == GPIO_OUT_HIGH ? (1UL << 15) : 0))

	// 0x1E64, default is input
#define GPIO_OEN        ((PAD_UART_RX2_IS_GPIO    <= GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_UART_TX2_IS_GPIO    <= GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_PWM0_IS_GPIO        <= GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_PWM1_IS_GPIO        <= GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_PWM2_IS_GPIO        <= GPIO_IN ? (1UL <<  4) : 0) | \
		(PAD_PWM3_IS_GPIO        <= GPIO_IN ? (1UL <<  5) : 0) | \
		(PAD_DDCR_DAT_IS_GPIO    <= GPIO_IN ? (1UL <<  6) : 0) | \
		(PAD_DDCR_CLK_IS_GPIO    <= GPIO_IN ? (1UL <<  7) : 0) | \
		(PAD_DDCR_DA2_IS_GPIO    <= GPIO_IN ? (1UL <<  8) : 0) | \
		(PAD_DDCR_CK2_IS_GPIO    <= GPIO_IN ? (1UL <<  9) : 0) | \
		(PAD_LHSYNC2_IS_GPIO     <= GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_LVSYNC2_IS_GPIO     <= GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_GPIO12_IS_GPIO      <= GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_GPIO13_IS_GPIO      <= GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_GPIO14_IS_GPIO      <= GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_GPIO15_IS_GPIO      <= GPIO_IN ? (1UL << 15) : 0))

	// 0x1EA2[3:0], GPIO[5:2]
#define PWM_IS_GPIO     ((PAD_PWM0_IS_GPIO ? BIT0 : 0) | \
		(PAD_PWM1_IS_GPIO ? BIT1 : 0) | \
		(PAD_PWM2_IS_GPIO ? BIT2 : 0) | \
		(PAD_PWM3_IS_GPIO ? BIT3 : 0))

	// 0x1EE0[7], GPIO[7:6]
#define DDCR_IS_GPIO    ((PAD_DDCR_DAT_IS_GPIO | PAD_DDCR_CLK_IS_GPIO) ? BIT7 : 0)

#if 0   // TODO: should be clarify -- Dean
	if (GPIO_P1_ENABLE)
	{
		P1 = GPIO_P1_VAL;
	}
#endif

	MDrv_WriteByte(0x1FA4, GPIO_P1_ENABLE);
	MDrv_Write2Byte(0x1E5E, GPIO_OUT);
	MDrv_Write2Byte(0x1E60, GPIO_OEN);

	// GPIO[1:0] is default gpio if PAD_UART_RX2/PAD_UART_TX2 is not for UART2/DSPJTAG/P1
	// GPIO[5:2]
	MDrv_WriteByteMask(0x1EA2, PWM_IS_GPIO, BITMASK(3:0));
	// GPIO[7:6]
	MDrv_WriteByteMask(0x1EE0, DDCR_IS_GPIO, BIT7);
	// GPIO[9:8] is default gpio if PAD_DDCR_DA2/PAD_DDCR_CK2 is not for DDCR/IIC
	// GPIO[15:10] is default gpio if the pads is not for other functions

	//-----------------------------------------------------------------
	// TS0 TS1 GPIO
	//-----------------------------------------------------------------
	// 0x1EA3[2]
#define TS0_ISGPIO      ((PAD_TS0_D0_IS_GPIO   || \
			PAD_TS0_D1_IS_GPIO   || \
			PAD_TS0_D2_IS_GPIO   || \
			PAD_TS0_D3_IS_GPIO   || \
			PAD_TS0_D4_IS_GPIO   || \
			PAD_TS0_D5_IS_GPIO   || \
			PAD_TS0_D6_IS_GPIO   || \
			PAD_TS0_D7_IS_GPIO   || \
			PAD_TS0_VLD_IS_GPIO  || \
			PAD_TS0_SYNC_IS_GPIO || \
			PAD_TS0_CLK_IS_GPIO) ? BIT2 : 0)

	// 0x1EA3[3]
#define TS1_ISGPIO      ((PAD_TS1_D0_IS_GPIO   || \
			PAD_TS1_VLD_IS_GPIO  || \
			PAD_TS1_SYNC_IS_GPIO || \
			PAD_TS1_CLK_IS_GPIO) ? BIT3 : 0)

	// 0x1E64[10:0]
#define TS0_GPIO_OUT    ((PAD_TS0_D0_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
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

	// 0x1E66[15:12] (0x1E67[7:4])  NOTE: address not same as TS0_GPIO_OUT
#define TS1_GPIO_OUT    ((PAD_TS1_D0_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_TS1_VLD_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_TS1_SYNC_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_TS1_CLK_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  7) : 0))
	// default is input
#define TS0_GPIO_OEN    ((PAD_TS0_D0_IS_GPIO   <= GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_TS0_D1_IS_GPIO   <= GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_TS0_D2_IS_GPIO   <= GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_TS0_D3_IS_GPIO   <= GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_TS0_D4_IS_GPIO   <= GPIO_IN ? (1UL <<  4) : 0) | \
		(PAD_TS0_D5_IS_GPIO   <= GPIO_IN ? (1UL <<  5) : 0) | \
		(PAD_TS0_D6_IS_GPIO   <= GPIO_IN ? (1UL <<  6) : 0) | \
		(PAD_TS0_D7_IS_GPIO   <= GPIO_IN ? (1UL <<  7) : 0) | \
		(PAD_TS0_VLD_IS_GPIO  <= GPIO_IN ? (1UL <<  8) : 0) | \
		(PAD_TS0_SYNC_IS_GPIO <= GPIO_IN ? (1UL <<  9) : 0) | \
		(PAD_TS0_CLK_IS_GPIO  <= GPIO_IN ? (1UL << 10) : 0))

	// default is input
#define TS1_GPIO_OEN    ((PAD_TS1_D0_IS_GPIO   <= GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_TS1_VLD_IS_GPIO  <= GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_TS1_SYNC_IS_GPIO <= GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_TS1_CLK_IS_GPIO  <= GPIO_IN ? (1UL << 15) : 0))

	if (TS0_ISGPIO || TS1_ISGPIO)
	{
		MDrv_Write2Byte(0x1E64, TS0_GPIO_OUT);
		MDrv_WriteByteMask(0x1E67, TS1_GPIO_OUT, BITMASK(7:4));
		MDrv_Write2Byte(0x1E68, TS1_GPIO_OEN | TS0_GPIO_OEN);
	}
	MDrv_WriteRegBit(0x1EA3, TS1_IS_GPIO | TS0_ISGPIO, BITMASK(3:2));

	//-----------------------------------------------------------------
	// I2S SPI GPIO
	//-----------------------------------------------------------------

	// 0x1EA2[4]
#define I2S_N_ISGPIO    ((PAD_I2S_IN_WS_IS_GPIO || \
			PAD_I2S_IN_BCK_IS_GPIO || \
			PAD_I2S_IN_SD_IS_GPIO) ? BIT4 : 0)
	// 0x1EA2[5]
#define I2S_N_ISGPIO2   ((PAD_I2S_OUT_WS_IS_GPIO || \
			PAD_I2S_OUT_MCK_IS_GPIO || \
			PAD_I2S_OUT_BCK_IS_GPIO || \
			PAD_I2S_OUT_SD_IS_GPIO) ? BIT5 : 0)
	// 0x1EE1[7:6]
#define I2S_N_ISGPIO34  ((PAD_SPDIF_IN_IS_GPIO  ? BIT6 : 0) | \
		(PAD_SPDIF_OUT_IS_GPIO ? BIT7 : 0))
	// 0x1EDF[7]
#define SPI_IS_GPIO     ((PAD_SPI_CK_IS_GPIO || \
			PAD_SPI_DI_IS_GPIO || \
			PAD_SPI_CZ_IS_GPIO || \
			PAD_SPI_DO_IS_GPIO) ? BIT7 : 0)

	// 0x1E6C[8:0], default is input
#define I2S_GPIO_OEN    ((PAD_I2S_IN_WS_IS_GPIO   <= GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_I2S_IN_BCK_IS_GPIO  <= GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_I2S_IN_SD_IS_GPIO   <= GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_SPDIF_IN_IS_GPIO    <= GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_SPDIF_OUT_IS_GPIO   <= GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_I2S_OUT_WS_IS_GPIO  <= GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_I2S_OUT_MCK_IS_GPIO <= GPIO_IN ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_BCK_IS_GPIO <= GPIO_IN ? (1UL << 7) : 0) | \
		(PAD_I2S_OUT_SD_IS_GPIO  <= GPIO_IN ? (1UL << 8) : 0))

	// 0x1E6C[15:12], default is output
#define SPI_GPIO_OEN    ((PAD_SPI_CK_IS_GPIO == GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_SPI_DI_IS_GPIO == GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_SPI_CZ_IS_GPIO == GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_SPI_DO_IS_GPIO == GPIO_IN ? (1UL << 15) : 0))

	// 0x1E6E[8:0]
#define I2S_GPIO_OUT    ((PAD_I2S_IN_WS_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_I2S_IN_BCK_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 1) : 0) | \
		(PAD_I2S_IN_SD_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 2) : 0) | \
		(PAD_SPDIF_IN_IS_GPIO    == GPIO_OUT_HIGH ? (1UL << 3) : 0) | \
		(PAD_SPDIF_OUT_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_I2S_OUT_WS_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_I2S_OUT_MCK_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 6) : 0) | \
		(PAD_I2S_OUT_BCK_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 7) : 0) | \
		(PAD_I2S_OUT_SD_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 8) : 0))

	// 0x1E6E[15:12]
#define SPI_GPIO_OUT    ((PAD_SPI_CK_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_SPI_DI_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_SPI_CZ_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_SPI_DO_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 15) : 0))

	if (I2S_N_ISGPIO || I2S_N_ISGPIO2 || I2S_N_ISGPIO34 || SPI_IS_GPIO)
	{
		MDrv_Write2Byte(0x1E6E, SPI_GPIO_OUT | I2S_GPIO_OUT);
		MDrv_Write2Byte(0x1E6C, SPI_GPIO_OEN | I2S_GPIO_OEN);
	}
	MDrv_WriteByteMask(0x1EA2, I2S_N_ISGPIO2 | I2S_N_ISGPIO, BITMASK(5:4));
	MDrv_WriteByteMask(0x1EE1, I2S_N_ISGPIO34, BITMASK(7:6));
	MDrv_WriteByteMask(0x1EDF, SPI_IS_GPIO, BIT7);

	//-----------------------------------------------------------------
	// PCM PCM2 GPIO
	//-----------------------------------------------------------------

	// PCM_GPIO[32:31] and PCM2_GPIO see below PCMCONFIG

	// 0x1EE0[5:0] reg_pcmisgpio
#define PCM_ISGPIO      (((PAD_PCM_A4_IS_GPIO     || \
				PAD_PCM_WAIT_N_IS_GPIO) ? BIT0 : 0) | \
		((PAD_PCM_A5_IS_GPIO     || \
		  PAD_PCM_A6_IS_GPIO     || \
		  PAD_PCM_A7_IS_GPIO     || \
		  PAD_PCM_A12_IS_GPIO    || \
		  PAD_PCM_IRQA_N_IS_GPIO) ? BIT1 : 0) | \
		((PAD_PCM_A14_IS_GPIO    || \
		  PAD_PCM_A13_IS_GPIO    || \
		  PAD_PCM_A8_IS_GPIO     || \
		  PAD_PCM_IOWR_N_IS_GPIO || \
		  PAD_PCM_A9_IS_GPIO     || \
		  PAD_PCM_IORD_N_IS_GPIO || \
		  PAD_PCM_A11_IS_GPIO    || \
		  PAD_PCM_OE_N_IS_GPIO   || \
		  PAD_PCM_A10_IS_GPIO    || \
		  PAD_PCM_CE_N_IS_GPIO   || \
		  PAD_PCM_D7_IS_GPIO     || \
		  PAD_PCM_D6_IS_GPIO) ?     BIT5 : 0) | \
		((PAD_PCM_D5_IS_GPIO) ?     BIT2 : 0) | \
		((PAD_PCM_D4_IS_GPIO     || \
		  PAD_PCM_D3_IS_GPIO) ?     BIT4 : 0) | \
	((PAD_PCM_A3_IS_GPIO     || \
	  PAD_PCM_A2_IS_GPIO     || \
	  PAD_PCM_REG_N_IS_GPIO  || \
	  PAD_PCM_A1_IS_GPIO     || \
	  PAD_PCM_A0_IS_GPIO     || \
	  PAD_PCM_D0_IS_GPIO     || \
	  PAD_PCM_D1_IS_GPIO     || \
	  PAD_PCM_D2_IS_GPIO) ?     BIT3 : 0))

	// 0x1E7C, default is input
#define PCM_GPIO_OEN_0  ((PAD_PCM_A4_IS_GPIO     <= GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_PCM_WAIT_N_IS_GPIO <= GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_PCM_A5_IS_GPIO     <= GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_PCM_A6_IS_GPIO     <= GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_PCM_A7_IS_GPIO     <= GPIO_IN ? (1UL <<  4) : 0) | \
		(PAD_PCM_A12_IS_GPIO    <= GPIO_IN ? (1UL <<  5) : 0) | \
		(PAD_PCM_IRQA_N_IS_GPIO <= GPIO_IN ? (1UL <<  6) : 0) | \
		(PAD_PCM_A14_IS_GPIO    <= GPIO_IN ? (1UL <<  7) : 0) | \
		(PAD_PCM_A13_IS_GPIO    <= GPIO_IN ? (1UL <<  8) : 0) | \
		(PAD_PCM_A8_IS_GPIO     <= GPIO_IN ? (1UL <<  9) : 0) | \
		(PAD_PCM_IOWR_N_IS_GPIO <= GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_PCM_A9_IS_GPIO     <= GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_PCM_IORD_N_IS_GPIO <= GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_PCM_A11_IS_GPIO    <= GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_PCM_OE_N_IS_GPIO   <= GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_PCM_A10_IS_GPIO    <= GPIO_IN ? (1UL << 15) : 0) | \
		(PAD_PCM_CE_N_IS_GPIO   <= GPIO_IN ? (1UL << 16) : 0) | \
		(PAD_PCM_D7_IS_GPIO     <= GPIO_IN ? (1UL << 17) : 0) | \
		(PAD_PCM_D6_IS_GPIO     <= GPIO_IN ? (1UL << 18) : 0) | \
		(PAD_PCM_D6_IS_GPIO     <= GPIO_IN ? (1UL << 19) : 0) | \
		(PAD_PCM_D5_IS_GPIO     <= GPIO_IN ? (1UL << 20) : 0) | \
	(PAD_PCM_D4_IS_GPIO     <= GPIO_IN ? (1UL << 21) : 0) | \
	(PAD_PCM_D3_IS_GPIO     <= GPIO_IN ? (1UL << 22) : 0) | \
	(PAD_PCM_A3_IS_GPIO     <= GPIO_IN ? (1UL << 23) : 0) | \
	(PAD_PCM_A2_IS_GPIO     <= GPIO_IN ? (1UL << 24) : 0) | \
	(PAD_PCM_REG_N_IS_GPIO  <= GPIO_IN ? (1UL << 25) : 0) | \
	(PAD_PCM_A1_IS_GPIO     <= GPIO_IN ? (1UL << 26) : 0) | \
	(PAD_PCM_A0_IS_GPIO     <= GPIO_IN ? (1UL << 27) : 0) | \
	(PAD_PCM_D0_IS_GPIO     <= GPIO_IN ? (1UL << 28) : 0) | \
	(PAD_PCM_D1_IS_GPIO     <= GPIO_IN ? (1UL << 29) : 0) | \
	(PAD_PCM_D2_IS_GPIO     <= GPIO_IN ? (1UL << 30) : 0) | \
	(PAD_PCM_RESET_IS_GPIO  <= GPIO_IN ? (1UL << 31) : 0))

	// 0x1E80
#define PCM_GPIO_OEN_1  ((PAD_PCM_CD_N_IS_GPIO   <= GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_PCM2_CE_N_IS_GPIO  <= GPIO_IN ? (1UL <<  8) : 0) | \
		(PAD_PCM2_IRQA_N_IS_GPIO<= GPIO_IN ? (1UL <<  9) : 0) | \
		(PAD_PCM2_WAIT_N_IS_GPIO<= GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_PCM2_RESET_IS_GPIO <= GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_PCM2_CD_N_IS_GPIO  <= GPIO_IN ? (1UL << 12) : 0))

	// 0x1E76
#define PCM_GPIO_DAT_0  ((PAD_PCM_A4_IS_GPIO     == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
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
		(PAD_PCM_D6_IS_GPIO     == GPIO_OUT_HIGH ? (1UL << 18) : 0) | \
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

	// 0x1E7A
#define PCM_GPIO_DAT_1  ((PAD_PCM_CD_N_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_PCM2_CE_N_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_PCM2_IRQA_N_IS_GPIO== GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_PCM2_WAIT_N_IS_GPIO== GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_PCM2_RESET_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_PCM2_CD_N_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 12) : 0))

	// output data
	MDrv_Write4Byte(0x1E7C, PCM_GPIO_DAT_0);
	MDrv_Write2Byte(0x1E80, PCM_GPIO_DAT_1);

	// direction
	MDrv_Write4Byte(0x1E76, PCM_GPIO_OEN_0);
	MDrv_Write2Byte(0x1E7A, PCM_GPIO_OEN_1);

	// reg_pciisgpio
	MDrv_WriteByteMask(0x1EE0, PCM_ISGPIO, BITMASK(5:0));

	//-----------------------------------------------------------------
	// PCMCONFIG
	//-----------------------------------------------------------------

	// 0x1EDC
#define PCM_CONFIG      (PIN_PCMCIA == PADS_PCM ? (1 << 0) : \
		PIN_CIMAX  == PADS_PCM ? (2 << 0) : 0)
#define PCM_CONFIG1     ((PAD_PCM_RESET_IS_GPIO || \
			PAD_PCM_CD_N_IS_GPIO) ? (3 << 2) : \
		(PIN_PCMCIA == PADS_PCM) ? (1 << 2) : \
		(PIN_CIMAX  == PADS_PCM) ? (2 << 2) : 0)
#define PCM2_CONFIG     ((PIN_PCMCIA == PADS_PCM && CHIP_USE_PCM_CDN_IN_PCM2) ? (1 << 4) : \
		(PAD_PCM2_CE_N_IS_GPIO || \
		 PAD_PCM2_IRQA_N_IS_GPIO || \
		 PAD_PCM2_WAIT_N_IS_GPIO) ? (3 << 4) : \
		(PIN_CIMAX == PADS_PCM) ? (2 << 4) : 0)
#define PCM2_CONFIG1    ((PAD_PCM2_RESET_IS_GPIO || PAD_PCM2_CD_N_IS_GPIO) ? (2 << 6) : (1 << 6))

	MDrv_WriteByte(0x1EDC, PCM2_CONFIG1 | PCM2_CONFIG | PCM_CONFIG1 | PCM_CONFIG);


//yongs_logo----------------------------------
	/*
#define PAD_ET_TXD3_IS_GPIO GPIO_OUT_LOW
#define PAD_GPIO_PM0_IS_GPIO    GPIO_OUT_LOW
#define PAD_GPIO_PM2_IS_GPIO    GPIO_OUT_HIGH
#define PAD_GPIO_PM3_IS_GPIO    GPIO_OUT_LOW
#define PAD_GPIO_PM4_IS_GPIO    GPIO_OUT_LOW
	*/
		//-----------------------------------------------------------------
		// PM GPIO (PM_SLEEP)
		//-----------------------------------------------------------------
		// 0x1002[10:0], default is input
	#define PM_GPIO_OEN     ((PAD_GPIO_PM0_IS_GPIO  <= GPIO_IN ? (1UL <<  0) : 0) | \
			(PAD_GPIO_PM1_IS_GPIO  <= GPIO_IN ? (1UL <<  1) : 0) | \
			(PAD_GPIO_PM2_IS_GPIO  <= GPIO_IN ? (1UL <<  2) : 0) | \
			(PAD_GPIO_PM3_IS_GPIO  <= GPIO_IN ? (1UL <<  3) : 0) | \
			(PAD_GPIO_PM4_IS_GPIO  <= GPIO_IN ? (1UL <<  4) : 0) | \
			(PAD_GPIO_PM5_IS_GPIO  <= GPIO_IN ? (1UL <<  5) : 0) | \
			(PAD_GPIO_PM6_IS_GPIO  <= GPIO_IN ? (1UL <<  6) : 0) | \
			(PAD_GPIO_PM7_IS_GPIO  <= GPIO_IN ? (1UL <<  7) : 0) | \
			(PAD_GPIO_PM8_IS_GPIO  <= GPIO_IN ? (1UL <<  8) : 0) | \
			(PAD_GPIO_PM9_IS_GPIO  <= GPIO_IN ? (1UL <<  9) : 0) | \
			(PAD_GPIO_PM10_IS_GPIO <= GPIO_IN ? (1UL << 10) : 0))
			// 0x1004[10:0]
	#define PM_GPIO_DAT     ((PAD_GPIO_PM0_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
			(PAD_GPIO_PM1_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
			(PAD_GPIO_PM2_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
			(PAD_GPIO_PM3_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
			(PAD_GPIO_PM4_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
			(PAD_GPIO_PM5_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
			(PAD_GPIO_PM6_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
			(PAD_GPIO_PM7_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
			(PAD_GPIO_PM8_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
			(PAD_GPIO_PM9_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
			(PAD_GPIO_PM10_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 10) : 0))

			MDrv_WriteByte(0x1004, (u8)(PM_GPIO_DAT) );
			MDrv_WriteByte(0x1005, (u8)(PM_GPIO_DAT >> 8));
			MDrv_WriteByte(0x1002, (u8)(PM_GPIO_OEN));
			MDrv_WriteByte(0x1003, (u8)(PM_GPIO_OEN >> 8));
	//-------------------------------------------


	//-----------------------------------------------------------------
	// ET GPIO
	//-----------------------------------------------------------------
	// 0x1EDF[3:2]
#define ET_IS_GPIO (((PAD_ET_CRS_IS_GPIO    || \
				PAD_ET_TXD3_IS_GPIO   || \
				PAD_ET_TXD2_IS_GPIO   || \
				PAD_ET_TXER_IS_GPIO   || \
				PAD_ET_RXER_IS_GPIO   || \
				PAD_ET_RX_CLK_IS_GPIO || \
				PAD_ET_RX_DV_IS_GPIO  || \
				PAD_ET_RXD2_IS_GPIO   || \
				PAD_ET_RXD3_IS_GPIO) ? BIT2 : 0) | \
		((PAD_ET_COL_IS_GPIO    || \
		  PAD_ET_TXD1_IS_GPIO   || \
		  PAD_ET_TXD0_IS_GPIO   || \
		  PAD_ET_TX_EN_IS_GPIO  || \
		  PAD_ET_TX_CLK_IS_GPIO || \
		  PAD_ET_RXD0_IS_GPIO   || \
		  PAD_ET_RXD1_IS_GPIO   || \
		  PAD_ET_MDC_IS_GPIO    || \
		  PAD_ET_MDIO_IS_GPIO) ? BIT3 : 0))

	// 0x1E89, default is input
#define ET_GPIO_OEN ((PAD_ET_CRS_IS_GPIO    <= GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_ET_TXD3_IS_GPIO   <= GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_ET_TXD2_IS_GPIO   <= GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_ET_TXER_IS_GPIO   <= GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_ET_RXER_IS_GPIO   <= GPIO_IN ? (1UL <<  4) : 0) | \
		(PAD_ET_RX_CLK_IS_GPIO <= GPIO_IN ? (1UL <<  5) : 0) | \
		(PAD_ET_RX_DV_IS_GPIO  <= GPIO_IN ? (1UL <<  6) : 0) | \
		(PAD_ET_RXD2_IS_GPIO   <= GPIO_IN ? (1UL <<  7) : 0) | \
		(PAD_ET_RXD3_IS_GPIO   <= GPIO_IN ? (1UL <<  8) : 0) | \
		(PAD_ET_COL_IS_GPIO    <= GPIO_IN ? (1UL <<  9) : 0) | \
		(PAD_ET_TXD1_IS_GPIO   <= GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_ET_TXD0_IS_GPIO   <= GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_ET_TX_EN_IS_GPIO  <= GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_ET_TX_CLK_IS_GPIO <= GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_ET_RXD0_IS_GPIO   <= GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_ET_RXD1_IS_GPIO   <= GPIO_IN ? (1UL << 15) : 0) | \
		(PAD_ET_MDC_IS_GPIO    <= GPIO_IN ? (1UL << 16) : 0) | \
		(PAD_ET_MDIO_IS_GPIO   <= GPIO_IN ? (1UL << 17) : 0))

	// 0x1E86
#define ET_GPIO_DAT ((PAD_ET_CRS_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_ET_TXD3_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_ET_TXD2_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_ET_TXER_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_ET_RXER_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_ET_RX_CLK_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_ET_RX_DV_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_ET_RXD2_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
		(PAD_ET_RXD3_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_ET_COL_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_ET_TXD1_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_ET_TXD0_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_ET_TX_EN_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_ET_TX_CLK_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_ET_RXD0_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_ET_RXD1_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 15) : 0) | \
		(PAD_ET_MDC_IS_GPIO    == GPIO_OUT_HIGH ? (1UL << 16) : 0) | \
		(PAD_ET_MDIO_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 17) : 0))

	//yongs_logo --------------------------------------------------
/*
#define ET_GPIO_DAT_INV ((PAD_ET_CRS_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_ET_TXD3_IS_GPIO   == GPIO_OUT_HIGH ? 0 : (1UL <<  1) ) | \
		(PAD_ET_TXD2_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_ET_TXER_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_ET_RXER_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_ET_RX_CLK_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_ET_RX_DV_IS_GPIO  == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_ET_RXD2_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
		(PAD_ET_RXD3_IS_GPIO   == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_ET_COL_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_ET_TXD1_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_ET_TXD0_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_ET_TX_EN_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_ET_TX_CLK_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_ET_RXD0_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_ET_RXD1_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 15) : 0) | \
		(PAD_ET_MDC_IS_GPIO    == GPIO_OUT_HIGH ? (1UL << 16) : 0) | \
		(PAD_ET_MDIO_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 17) : 0))
*/
	//-------------------------------------------------------------


//	MDrv_Write3Byte(0x1E86, ET_GPIO_DAT);
//	MDrv_Write3Byte(0x1E89, ET_GPIO_OEN);
//	MDrv_WriteByteMask(0x1EDF, ET_IS_GPIO, BITMASK(3:2));

	//yongs_logo-----------------------------------
/*
	MDrv_WriteByteMask(0x1E86, ET_GPIO_DAT_INV, 0x3FFFF);
	MDrv_WriteByteMask(0x1E86, ET_GPIO_DAT, 0x3FFFF);
*/
	//----------------------------------------------

	//-----------------------------------------------------------------
	// PF GPIO
	//-----------------------------------------------------------------
	// 0x1EDF[5:4]
#define PF_IS_GPIO (((PAD_PF_AD15_IS_GPIO || \
				PAD_PF_CE0Z_IS_GPIO || \
				PAD_PF_CE1Z_IS_GPIO || \
				PAD_PF_OEZ_IS_GPIO  || \
				PAD_PF_WEZ_IS_GPIO  || \
				PAD_PF_ALE_IS_GPIO) ? BIT4 : 0) | \
		((PAD_F_RBZ_IS_GPIO) ? BIT5 : 0))

	// 0x1E94, default is input
#define PF_GPIO_OEN ((PAD_PF_AD15_IS_GPIO <= GPIO_IN ? (1UL << 0) : 0) | \
		(PAD_PF_CE0Z_IS_GPIO <= GPIO_IN ? (1UL << 1) : 0) | \
		(PAD_PF_CE1Z_IS_GPIO <= GPIO_IN ? (1UL << 2) : 0) | \
		(PAD_PF_OEZ_IS_GPIO  <= GPIO_IN ? (1UL << 3) : 0) | \
		(PAD_PF_WEZ_IS_GPIO  <= GPIO_IN ? (1UL << 4) : 0) | \
		(PAD_PF_ALE_IS_GPIO  <= GPIO_IN ? (1UL << 5) : 0) | \
		(PAD_F_RBZ_IS_GPIO   <= GPIO_IN ? (1UL << 6) : 0))

	// 0x1E90
#define PF_GPIO_DAT ((PAD_PF_AD15_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 0) : 0) | \
		(PAD_PF_CE0Z_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 1) : 0) | \
		(PAD_PF_CE1Z_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 2) : 0) | \
		(PAD_PF_OEZ_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 3) : 0) | \
		(PAD_PF_WEZ_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 4) : 0) | \
		(PAD_PF_ALE_IS_GPIO  == GPIO_OUT_HIGH ? (1UL << 5) : 0) | \
		(PAD_F_RBZ_IS_GPIO   == GPIO_OUT_HIGH ? (1UL << 6) : 0))

	MDrv_WriteByte(0x1E90, PF_GPIO_DAT);
	MDrv_WriteByte(0x1E94, PF_GPIO_OEN);
	MDrv_WriteByteMask(0x1EDF, PF_IS_GPIO, BITMASK(5:4));

	//-----------------------------------------------------------------
	// MOD GPIO
	//-----------------------------------------------------------------

#define MOD_SELTTL (((PAD_R_ODD7_IS_GPIO | PAD_R_ODD6_IS_GPIO) ? (1UL <<  0) : 0) | \
		((PAD_R_ODD5_IS_GPIO | PAD_R_ODD4_IS_GPIO) ? (1UL <<  1) : 0) | \
		((PAD_R_ODD3_IS_GPIO | PAD_R_ODD2_IS_GPIO) ? (1UL <<  2) : 0) | \
		((PAD_R_ODD1_IS_GPIO | PAD_R_ODD0_IS_GPIO) ? (1UL <<  3) : 0) | \
		((PAD_G_ODD7_IS_GPIO | PAD_G_ODD6_IS_GPIO) ? (1UL <<  4) : 0) | \
		((PAD_G_ODD5_IS_GPIO | PAD_G_ODD4_IS_GPIO) ? (1UL <<  5) : 0) | \
		((PAD_G_ODD3_IS_GPIO | PAD_G_ODD2_IS_GPIO) ? (1UL <<  6) : 0) | \
		((PAD_G_ODD1_IS_GPIO | PAD_G_ODD0_IS_GPIO) ? (1UL <<  7) : 0) | \
		((PAD_B_ODD7_IS_GPIO | PAD_B_ODD6_IS_GPIO) ? (1UL <<  8) : 0) | \
		((PAD_B_ODD5_IS_GPIO | PAD_B_ODD4_IS_GPIO) ? (1UL <<  9) : 0) | \
		((PAD_B_ODD3_IS_GPIO | PAD_B_ODD2_IS_GPIO) ? (1UL << 10) : 0) | \
		((PAD_B_ODD1_IS_GPIO | PAD_B_ODD0_IS_GPIO) ? (1UL << 11) : 0))

#define MOD_GPO_SEL ((PAD_LVSYNC_IS_GPIO ? (1UL <<  0) : 0) | \
		(PAD_LHSYNC_IS_GPIO ? (1UL <<  1) : 0) | \
		(PAD_LDE_IS_GPIO    ? (1UL <<  2) : 0) | \
		(PAD_LCK_IS_GPIO    ? (1UL <<  3) : 0) | \
		(PAD_R_ODD7_IS_GPIO ? (1UL <<  5) : 0) | \
		(PAD_R_ODD6_IS_GPIO ? (1UL <<  4) : 0) | \
		(PAD_R_ODD5_IS_GPIO ? (1UL <<  7) : 0) | \
		(PAD_R_ODD4_IS_GPIO ? (1UL <<  6) : 0) | \
		(PAD_R_ODD3_IS_GPIO ? (1UL <<  9) : 0) | \
		(PAD_R_ODD2_IS_GPIO ? (1UL <<  8) : 0) | \
		(PAD_R_ODD1_IS_GPIO ? (1UL << 11) : 0) | \
		(PAD_R_ODD0_IS_GPIO ? (1UL << 10) : 0) | \
		(PAD_G_ODD7_IS_GPIO ? (1UL << 13) : 0) | \
		(PAD_G_ODD6_IS_GPIO ? (1UL << 12) : 0) | \
		(PAD_G_ODD5_IS_GPIO ? (1UL << 15) : 0) | \
		(PAD_G_ODD4_IS_GPIO ? (1UL << 14) : 0) | \
		(PAD_G_ODD3_IS_GPIO ? (1UL << 17) : 0) | \
		(PAD_G_ODD2_IS_GPIO ? (1UL << 16) : 0) | \
		(PAD_G_ODD1_IS_GPIO ? (1UL << 19) : 0) | \
		(PAD_G_ODD0_IS_GPIO ? (1UL << 18) : 0) | \
		(PAD_B_ODD7_IS_GPIO ? (1UL << 21) : 0) | \
	(PAD_B_ODD6_IS_GPIO ? (1UL << 20) : 0) | \
	(PAD_B_ODD5_IS_GPIO ? (1UL << 23) : 0) | \
	(PAD_B_ODD4_IS_GPIO ? (1UL << 22) : 0) | \
	(PAD_B_ODD3_IS_GPIO ? (1UL << 25) : 0) | \
	(PAD_B_ODD2_IS_GPIO ? (1UL << 24) : 0) | \
	(PAD_B_ODD1_IS_GPIO ? (1UL << 27) : 0) | \
	(PAD_B_ODD0_IS_GPIO ? (1UL << 26) : 0))

	// default is output
#define MOD_GPO_OEZ ((PAD_LVSYNC_IS_GPIO == GPIO_IN ? (1UL <<  0) : 0) | \
		(PAD_LHSYNC_IS_GPIO == GPIO_IN ? (1UL <<  1) : 0) | \
		(PAD_LDE_IS_GPIO    == GPIO_IN ? (1UL <<  2) : 0) | \
		(PAD_LCK_IS_GPIO    == GPIO_IN ? (1UL <<  3) : 0) | \
		(PAD_R_ODD7_IS_GPIO == GPIO_IN ? (1UL <<  5) : 0) | \
		(PAD_R_ODD6_IS_GPIO == GPIO_IN ? (1UL <<  4) : 0) | \
		(PAD_R_ODD5_IS_GPIO == GPIO_IN ? (1UL <<  7) : 0) | \
		(PAD_R_ODD4_IS_GPIO == GPIO_IN ? (1UL <<  6) : 0) | \
		(PAD_R_ODD3_IS_GPIO == GPIO_IN ? (1UL <<  9) : 0) | \
		(PAD_R_ODD2_IS_GPIO == GPIO_IN ? (1UL <<  8) : 0) | \
		(PAD_R_ODD1_IS_GPIO == GPIO_IN ? (1UL << 11) : 0) | \
		(PAD_R_ODD0_IS_GPIO == GPIO_IN ? (1UL << 10) : 0) | \
		(PAD_G_ODD7_IS_GPIO == GPIO_IN ? (1UL << 13) : 0) | \
		(PAD_G_ODD6_IS_GPIO == GPIO_IN ? (1UL << 12) : 0) | \
		(PAD_G_ODD5_IS_GPIO == GPIO_IN ? (1UL << 15) : 0) | \
		(PAD_G_ODD4_IS_GPIO == GPIO_IN ? (1UL << 14) : 0) | \
		(PAD_G_ODD3_IS_GPIO == GPIO_IN ? (1UL << 17) : 0) | \
		(PAD_G_ODD2_IS_GPIO == GPIO_IN ? (1UL << 16) : 0) | \
		(PAD_G_ODD1_IS_GPIO == GPIO_IN ? (1UL << 19) : 0) | \
		(PAD_G_ODD0_IS_GPIO == GPIO_IN ? (1UL << 18) : 0) | \
		(PAD_B_ODD7_IS_GPIO == GPIO_IN ? (1UL << 21) : 0) | \
	(PAD_B_ODD6_IS_GPIO == GPIO_IN ? (1UL << 20) : 0) | \
	(PAD_B_ODD5_IS_GPIO == GPIO_IN ? (1UL << 23) : 0) | \
	(PAD_B_ODD4_IS_GPIO == GPIO_IN ? (1UL << 22) : 0) | \
	(PAD_B_ODD3_IS_GPIO == GPIO_IN ? (1UL << 25) : 0) | \
	(PAD_B_ODD2_IS_GPIO == GPIO_IN ? (1UL << 24) : 0) | \
	(PAD_B_ODD1_IS_GPIO == GPIO_IN ? (1UL << 27) : 0) | \
	(PAD_B_ODD0_IS_GPIO == GPIO_IN ? (1UL << 26) : 0))

#define MOD_GPO_DAT ((PAD_LVSYNC_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  0) : 0) | \
		(PAD_LHSYNC_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  1) : 0) | \
		(PAD_LDE_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  2) : 0) | \
		(PAD_LCK_IS_GPIO    == GPIO_OUT_HIGH ? (1UL <<  3) : 0) | \
		(PAD_R_ODD7_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  5) : 0) | \
		(PAD_R_ODD6_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  4) : 0) | \
		(PAD_R_ODD5_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  7) : 0) | \
		(PAD_R_ODD4_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  6) : 0) | \
		(PAD_R_ODD3_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  9) : 0) | \
		(PAD_R_ODD2_IS_GPIO == GPIO_OUT_HIGH ? (1UL <<  8) : 0) | \
		(PAD_R_ODD1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 11) : 0) | \
		(PAD_R_ODD0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 10) : 0) | \
		(PAD_G_ODD7_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 13) : 0) | \
		(PAD_G_ODD6_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 12) : 0) | \
		(PAD_G_ODD5_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 15) : 0) | \
		(PAD_G_ODD4_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 14) : 0) | \
		(PAD_G_ODD3_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 17) : 0) | \
		(PAD_G_ODD2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 16) : 0) | \
		(PAD_G_ODD1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 19) : 0) | \
		(PAD_G_ODD0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 18) : 0) | \
		(PAD_B_ODD7_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 21) : 0) | \
	(PAD_B_ODD6_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 20) : 0) | \
	(PAD_B_ODD5_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 23) : 0) | \
	(PAD_B_ODD4_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 22) : 0) | \
	(PAD_B_ODD3_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 25) : 0) | \
	(PAD_B_ODD2_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 24) : 0) | \
	(PAD_B_ODD1_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 27) : 0) | \
	(PAD_B_ODD0_IS_GPIO == GPIO_OUT_HIGH ? (1UL << 26) : 0))

	if (MOD_GPO_SEL)
	{
		MDrv_Write2Byte(0x3088, MOD_SELTTL);
		MDrv_Write4Byte(0x309E, MOD_GPO_DAT);
		MDrv_Write4Byte(0x30A2, MOD_GPO_OEZ);
		MDrv_Write4Byte(0x308C, MOD_GPO_SEL);   // EXT_PAD_EN
		MDrv_Write4Byte(0x309A, MOD_GPO_SEL);   // GPIO_SEL
	}

	//-----------------------------------------------------------------
	// HDMI HOTPLUG (TODO: set default setting here)
	//-----------------------------------------------------------------

	// default is GPIO

	//-----------------------------------------------------------------
	// SPI_CZ[3:1]
	//-----------------------------------------------------------------

	// 0x1EE5[6:1]
#if 0 // TODO: should be clarify -- Dean
#define SEL_CZ      ((PIN_SPI_CZ1 == PAD_UART_RX2   ? (0UL << 1) : \
			PIN_SPI_CZ1 == PAD_PCM_D5     ? (1UL << 1) : \
			PIN_SPI_CZ1 == PAD_I2S_IN_BCK ? (3UL << 1) : 0) | \
		(PIN_SPI_CZ2 == PAD_UART_TX2   ? (0UL << 3) : \
		 PIN_SPI_CZ2 == PAD_PCM_D6     ? (1UL << 3) : \
		 PIN_SPI_CZ2 == PAD_LHSYNC2    ? (3UL << 3) : 0) | \
		(PIN_SPI_CZ3 == PAD_I2S_IN_WS  ? (0UL << 5) : \
		 PIN_SPI_CZ3 == PAD_LVSYNC2    ? (2UL << 5) : \
		 PIN_SPI_CZ3 == PAD_PCM_A3     ? (3UL << 5) : 0))

	// 0x1EE2[6:4]
#define SEL_CZ_ON   ((PIN_SPI_CZ1 ? BIT4 : 0) | \
		(PIN_SPI_CZ2 ? BIT5 : 0) | \
		(PIN_SPI_CZ3 ? BIT6 : 0))

	if (SEL_CZ)
	{
		MDrv_WriteByteMask(0x1EE5, SEL_CZ, BITMASK(6:1));
		MDrv_WriteByteMask(0x1EE2, SEL_CZ_ON, BITMASK(6:4));
	}
#endif
}

static void __init miu_priority_config(void)
{
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

    // AUDIO  priority to high, samuel 090108
    //*(volatile unsigned int*)(0xbf800000+(0x1268<<1)) = 0x3FFF ;
    *(volatile unsigned int*)(0xbf800000+(0x126A<<1)) = 0x32EF ;
    *(volatile unsigned int*)(0xbf800000+(0x126C<<1)) = 0x7654 ;
    *(volatile unsigned int*)(0xbf800000+(0x126E<<1)) = 0xBA98 ;
    *(volatile unsigned int*)(0xbf800000+(0x1270<<1)) = 0x10DC ;
    *(volatile unsigned int*)(0xbf800000+(0x1260<<1)) |= (1<<1) ; // triger priority setting working
    *(volatile unsigned int*)(0xbf800000+(0x1260<<1)) &= ~(1<<1) ; // triger priority setting working, samuel 081022

}

static void __init miu_assignment(void)
{
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
}


#include <linux/syscalls.h>
#include <linux/fadvise.h>
int MDrv_fadvise( int fd ){
    return sys_fadvise64_64(fd, 0, 0, POSIX_FADV_SEQUENTIAL) ;
}

EXPORT_SYMBOL(MDrv_fadvise) ;

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
        *(volatile unsigned int*)(0xbf800000+(0x1268<<1)) = 0x3FFF ;
        bank = *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) ;
        *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = 0x0012 ;
        *(volatile unsigned int*)(0xbf800000+(0x2F08<<1)) |= (1<<8) ;
        *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = bank ;
        spin_unlock_irqrestore(&_q_spinlock, flags);
        q_tune = 0 ;
    }

    if( q_restore ){
        //restore
        spin_lock_irqsave(&_q_spinlock, flags);
        bank = *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) ;
        *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = 0x0012 ;
        *(volatile unsigned int*)(0xbf800000+(0x2F08<<1)) &= ~(1<<8) ;
        *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = bank ;
        *(volatile unsigned int*)(0xbf800000+(0x1268<<1)) = 0x7FFF ;
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
    bank = *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) ;

    // disbale input source
//    *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = 0x0001 ;
//    *(volatile unsigned int*)(0xbf800000+(0x2F04<<1)) |= (1<<7) ;
//    mdelay(1) ; // stablize MIU

    if( inqmode ){
        // shift Scaler buffer to MIU1
//        *(volatile unsigned int*)(0xbf800000+(0x12F2<<1)) |= (0x00C1) ;
        // tune YNR only and 8bit motion
        //*(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = 0x0012 ;
        //*(volatile unsigned int*)(0xbf800000+(0x2F04<<1)) |= (1<<10) ;

        q_recover_mode = 1 ;
    }else{
        // shift Scaler buffer to MIU0
//        *(volatile unsigned int*)(0xbf800000+(0x12F2<<1)) &= ~(0x00C1) ;
        // restore
        //*(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = 0x0012 ;
        //*(volatile unsigned int*)(0xbf800000+(0x2F04<<1)) &= ~(1<<10) ;

        q_recover_mode = 0 ;
    }

    // enable input source
//    *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = 0x0001 ;
//    *(volatile unsigned int*)(0xbf800000+(0x2F04<<1)) &= ~(1<<7) ;

    *(volatile unsigned int*)(0xbf800000+(0x2F00<<1)) = bank ;
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

