/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 */

/*
 * This file contains the configuration parameters for the Douglas board.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Miscellaneous definitions
 */
#define OPTIMIZE_BOOT					0

#define BOOTTIME_LOG					0
#define	CFG_NO_FLASH

/* Board configurations
 */
#define CONFIG_MIPS32					1					/* MIPS 4K CPU core	*/
#define CONFIG_GP2_DEMO1				1

#ifndef CPU_CLOCK_RATE
//#define CPU_CLOCK_RATE				24000000			/* 24 MHz clock for the MIPS core */
//#define CPU_CLOCK_RATE				48000000			/* 48 MHz clock for the MIPS core */
//#define CPU_CLOCK_RATE				72000000			/* 72 MHz clock for the MIPS core */
//#define CPU_CLOCK_RATE				96000000			/* 96 MHz clock for the MIPS core */
//#define CPU_CLOCK_RATE				144000000			/* 144 MHz clock for the MIPS core */
//#define CPU_CLOCK_RATE				192000000			/* 192 MHz clock for the MIPS core */
//#define CPU_CLOCK_RATE				504000000			/* 504 MHz clock for the MIPS core */
#define CPU_CLOCK_RATE					552000000			/* 504 MHz clock for the MIPS core */
#endif
#define CPU_TCLOCK_RATE					16588800			/* 16.5888 MHz for TClock */

#define CONFIG_CONS_INDEX				1

//#define CONFIG_ETHADDR				00:02:3e:26:0a:55   /* Ethernet address */
#define CONFIG_LSERVERIP				165.186.175.92
#define CONFIG_TSERVERIP				192.168.0.1
#define CONFIG_TARGETIP					192.168.0.10
#define CONFIG_GATEWAYIP				192.168.0.1
#define CONFIG_NETMASK					255.255.255.0

#define CONFIG_STACKSIZE				(128 * 1024)
#define CFG_INIT_SP_OFFSET  			0x400000
#define CONFIG_BOOTDELAY				3					/* autoboot after 3 seconds	*/

#define CONFIG_BAUDRATE					115200
#define CONFIG_RZ_BAUDRATE				460800
#define CFG_BAUDRATE_TABLE				{9600, 19200, 38400, 57600, 115200,  230400, 256000, 460800, 921600} /* baudrates */

#define CFG_MALLOC_LEN					(512*1024)
#define CFG_BOOTPARAMS_LEN				(128*1024)
#define CFG_GBL_DATA_SIZE				(128+CFG_BOOTPARAMS_LEN)

#define CFG_LOAD_ADDR					0x82000000			/* default load address    		*/
#define CFG_KERLOAD_ADDR				0x84000000			/* default kernel load address	*/
#define	CFG_RAMDISK_ADDR				0x80900000			/* default ramdisk address		*/

#define CFG_MEMTEST_START				0x80000000
#define CFG_MEMTEST_END					0x84000000

#undef	CONFIG_TIMESTAMP									/* Print image info with timestamp */
#undef	CONFIG_BZIP2            							/* Enable bzip2 decomp. */

#define CONFIG_SILENT_CONSOLE
#define CONFIG_AUTO_COMPLETE

/* Boot command configurations
 */
#undef	CONFIG_BOOTARGS
#define	CONFIG_EXTRA_ENV_SETTINGS												\
	"bootargs=root=/dev/mtdblock3 lginit=/dev/mtdblock4 rootfstype=squashfs\0"	\
	"userpath=\0"																\
	"hostname=saturn7\0"											 			\
	"lpj=167424\0"											 					\
	"rootfs=flash\0"												 			\
	"autoboot=n\0"													 			\
	"verify=n\0"													 			\
	"appxip=y\0"													 			\
	"fontxip=y\0"													 			\
	"silent=y\0"													 			\
	"silent_app=y\0"												 			\
	"silent_ker=y\0"												 			\
	"lock_time=0\0"												 				\
	"enable_probe=0\0"												 				\
	""

#define CONFIG_BOOTCOMMAND				"xip lgapp; xip lgfont; cp2ram kernel 0x84000000; bootm 0x84000000"
#define CONFIG_COMMANDS					((CFG_CMD_PING   | CFG_CMD_NET   | CFG_CMD_USB  | CFG_CMD_NAND | 	\
										  CFG_CMD_MEMORY | CFG_CMD_RUN   | CFG_CMD_ENV  | CFG_CMD_CACHE | CFG_CMD_I2C) &	\
				 						~(CFG_CMD_FPGA   | CFG_CMD_LOADS | CFG_CMD_IMLS | CFG_CMD_ITEST | 	\
				  						  CFG_CMD_XIMG   | CFG_CMD_BOOTD | CFG_CMD_SETGETDCR | CFG_CMD_NFS))

#include <cmd_confdefs.h>

/* External devices' configurations
 */
#undef	CONFIG_DRIVER_CS8900

#define RESET_VECTOR					0xBFC00000
#define CFG_BOOT_BASE					RESET_VECTOR
#ifndef CFG_NO_FLASH
#define CFG_FLASH_BASE					0xBE000000
#else
#define CFG_FLASH_BASE					0x00000000
#endif

/* Miscellaneous configurable options
 */
#undef	CONFIG_MISC_INIT_R

#define CFG_PROMPT						"saturn7 # "							/* Monitor Command Prompt    */
#define	CFG_LONGHELP					1									/* undef to save memory      */
#define	CFG_CBSIZE						256									/* Console I/O Buffer Size   */
#define	CFG_PBSIZE 						(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#if 0
#define CFG_HZ							(CPU_TCLOCK_RATE/4)
#else
#define CFG_HZ							(CPU_CLOCK_RATE/2)
#endif
#define	CFG_MAXARGS						16									/* max number of command args*/

/* Dram Bank Configurations
 */
#define CONFIG_NR_DRAM_BANKS			1
#define PHYS_SDRAM_1					0x80000000
#define PHYS_SDRAM_1_SIZE				0x08000000

#define CFG_SDRAM_BASE					PHYS_SDRAM_1
#define CFG_SDRAM_SIZE					PHYS_SDRAM_1_SIZE
#define CPU_MEM_BASE					CFG_SDRAM_BASE

/* Flash and environment organization
 */
#define PHYS_FLASH_1        			0xBE000000							/* Flash Bank #1 */
#define PHYS_FLASH_2        			0xB0800000 							/* Flash Bank #2 */

#define CFG_MAX_FLASH_BANKS				1									/* max number of memory banks */
#define CFG_MAX_FLASH_SECT				256									/* max number of sectors on one chip. */
																			/* 63 64KB sectors, 8 8kb sectors */
#define CONFIG_FLASH_16BIT				1

#define	CFG_MONITOR_BASE				TEXT_BASE
#define	CFG_MONITOR_LEN					(192 << 10)							/* 196608 */

#define CFG_FLASH_ERASE_TOUT			(30 * CFG_HZ) 						/* Max Timeout for Flash Erase 30 Sec */
#define CFG_FLASH_WRITE_TOUT			(50 * CFG_HZ) 						/* Max Timeout for Flash Write 50 Sec */

#undef	CFG_ENV_IS_IN_FLASH
#define	CFG_ENV_IS_IN_NAND

#undef CFG_DIRECT_FLASH_TFTP

/* Address and size of Primary Environment Sector
 */
#define CFG_ENV_SIZE					(0x20000)
#define CFG_ENV_OFFSET					(0xa0000)
#define CFG_ENV_ADDR					(CFG_BOOT_BASE + CFG_ENV_OFFSET)

/* NAND Flash configurations
 */
#define CFG_NAND_BASE					0x00000000
#define CFG_MAX_NAND_DEVICE				1									/* Max number of NAND devices */
#define CONFIG_JFFS2_NAND				1  									/* jffs2 on nand support 	 */

#undef	CONFIG_MTD_NAND_VERIFY_WRITE 										/* verify all writes!!!      */
#define NAND_MAX_FLOORS					1
#define NAND_MAX_CHIPS					1
#define CFG_NANDDRV_DEBUG				0  /* 0:Disable/1:Enable driver-level debug message */

#define CONFIG_MTD_NAND_BBM				1

/*JFFS2 partitions' configurations
 */
#define CONFIG_JFFS2_CMDLINE			1

/* Cache configuration
 */
#define CFG_DCACHE_SIZE					(32*1024)
#define CFG_ICACHE_SIZE					(32*1024)
#define CFG_CACHELINE_SIZE				32

/* USB configuration
 */
#define CONFIG_USB_EHCI

/* Net define
 */
#define CONFIG_NET_MULTI

/* Tftp path config
 */
#define CONFIG_TFTP_PATH_IN_TFTPSTART
#undef	CONFIG_TFTP_PATH_IN_MTDINFO

/* Load command config
 */
#define CONFIG_LOAD_TFTP
#define CONFIG_LOAD_ZMODEM
#undef	CONFIG_LOAD_FAT32
#undef	CONFIG_LOAD_KERMIT
#undef	CONFIG_LOAD_HZMODEM
#undef	CONFIG_LOAD_SERIAL

/* Emac configuration
 */
#define	CONFIG_EMAC

/* Uart configuration
 */
#undef	CONFIG_UART0

#define	virt_to_dma(addr)				(addr - CFG_SDRAM_BASE)

#ifdef __cplusplus
}
#endif

#endif	/* _CONFIG_H_ */
