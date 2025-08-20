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

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <devices.h>
#include <version.h>
#include <net.h>
#include <environment.h>
#include <flash.h>
#include <nand.h>
#include <nvm.h>

DECLARE_GLOBAL_DATA_PTR;

#if ( ((CFG_ENV_ADDR+CFG_ENV_SIZE) < CFG_MONITOR_BASE) || \
      (CFG_ENV_ADDR >= (CFG_MONITOR_BASE + CFG_MONITOR_LEN)) ) || \
    defined(CFG_ENV_IS_IN_NVRAM)
#define	TOTAL_MALLOC_LEN	(CFG_MALLOC_LEN + CFG_ENV_SIZE)
#else
#define	TOTAL_MALLOC_LEN	CFG_MALLOC_LEN
#endif

#undef DEBUG
#define mdelay(n)			udelay((n)*1000)

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
void nand_init (void);
#endif

extern void dcache_disable (void);
extern void BootSplash (void);

extern int timer_init(void);
extern int incaip_set_cpuclk(void);
extern void malloc_bin_reloc (void);
extern void set_releaseEnv(void);
extern ulong uboot_end_data;
extern ulong _start;
extern nand_info_t nand_info[CFG_MAX_NAND_DEVICE];

extern void print_dqsvalue(void);

ulong monitor_flash_len;
ulong malloc_base;
ulong global_data_base;
ulong stack_ptr;

const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")";

static char *failed = "*** failed ***\n";

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;

/***********************************************
 부팅 최적화 및 시간 측정함수
 ***********************************************/
#if (BOOTTIME_LOG == 0)
#define BOOTLOG(x)				//printf(x"\n")
#define SHOWLOG(x)
#else
#define MAX_LOG_SIZE            32
#define MAX_DESC_STRING_LENGTH  28

typedef struct
{
	ulong   tick;
	char    desc[MAX_DESC_STRING_LENGTH];
} TIMELOG;

TIMELOG     timelog[MAX_LOG_SIZE];
uint        tcnt = 0;

void BOOTLOG(const char * str)
{
	timelog[tcnt].tick = get_ticks();
	strncpy(timelog[tcnt].desc, str, MAX_DESC_STRING_LENGTH);
	timelog[tcnt].desc[MAX_DESC_STRING_LENGTH-1] = '\0';
	tcnt++;
}

void SHOWLOG(void)
{
	int     i;
	uint    msec;

	printf("\n- Booting time log -\n");
	printf("\n%2s: %28s  %4s  %6s\n", "#", "desc", "msec", "tick");

	for (i=0 ; i < tcnt ; i++)
	{
		msec = timelog[i].tick / (CFG_HZ / 1000);
		printf("%2d: %28s  %4d  %6d\n", i, timelog[i].desc, msec, timelog[i].tick);
	}
	printf("\n");
}
#endif

/*
 * The Malloc area is immediately below the monitor copy in DRAM
 */
static void mem_malloc_init (void)
{
	mem_malloc_start	= malloc_base;
	mem_malloc_end		= mem_malloc_start + CFG_MALLOC_LEN - 4;
	mem_malloc_brk		= mem_malloc_start;

	memset ((void *) mem_malloc_start,
			0,
			mem_malloc_end - mem_malloc_start);
	#if 0
	printf	("base(0x%08x),st(0x%08x),end(0x%08x)\n",
			malloc_base,
			mem_malloc_start,
			mem_malloc_end
			);
	#endif
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;
	return ((void *) old);
}

static int init_func_ram (void)
{
#ifdef	CONFIG_BOARD_TYPES
	int board_type = gd->board_type;
#else
	int board_type = 0;	/* use dummy arg */
#endif

#if (OPTIMIZE_BOOT == 0)
	puts ("DRAM\t: ");
#endif

	if ((gd->ram_size = initdram (board_type)) > 0) {
#if (OPTIMIZE_BOOT == 0)
	printf ("%dMB\n", gd->ram_size*2);
#endif
		return (0);
	}
	puts (failed);
	return (1);
}

static int display_banner(void)
{
	extern ulong read_cp0conf(void);
	ulong  cp0_config = read_cp0conf();

	printf ("\n\n%s\n\n", version_string);
	printf ("[CP0config] 0x%08x\n", cp0_config);
	printf ("[ Baudrate] %d\n", gd->baudrate);

	return (0);
}

#ifndef CFG_NO_FLASH
static void display_flash_config(ulong size)
{
	puts ("Flash\t: ");
	print_size (size, "\n");
}
#endif


static int init_baudrate (void)
{
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_r ("baudrate", tmp, sizeof (tmp));

	gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;
	return (0);
}

/*
 * Breath some life into the board...
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	timer_init,
	env_init,			/* initialize environment */
#ifdef CONFIG_INCA_IP
	incaip_set_cpuclk,	/* set cpu clock according to environment variable */
#endif
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,
	display_banner,		/* say that we are here */
	checkboard,
	init_func_ram,
	NULL,
};

static int fast_boot(void)
{
	char *s = NULL, key = 0;
	int i;

	/* delay 10 * 1us */
	for (i=0; i<10; ++i) {
		if (serial_tstc()) {	/* we got a key press	*/
			key = serial_getc();
//			printf("key in %02x\n", key);
			break;
		}
		udelay (1);
	}

	if((s = getenv("bootlogo")) != NULL) {
		if (*s == 'y')
			BootSplash();
	} else {
		DDI_NVM_GetToolOpt();
		if(gToolOptionDB.nToolOption3.flags.bBootLogo)
			BootSplash();
	}

	if(!key) {
		s = getenv("bootcmd");
		BOOTLOG("get bootcmd");

		SHOWLOG();

		run_command(s, 0);
	}
	return 0;
}

/************************************************************************
 *
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, the stack size in not  that critical any more,
 * etc.
 *
 ************************************************************************
 */
void board_init(void)
{
#ifndef CFG_NO_FLASH
	ulong size;
#endif
#if (CONFIG_COMMANDS & CFG_CMD_NET)
	char *e;
	int i;
#endif
	char *s;

	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	malloc_base    				= ((ulong)&_start - CFG_MALLOC_LEN);
	global_data_base			= (malloc_base - CFG_GBL_DATA_SIZE);
	stack_ptr      				= global_data_base;

	gd							= (gd_t *)global_data_base;
	memset ((void *)gd, 0, sizeof (gd_t));

	global_data_base			+= sizeof(gd_t);
	gd->bd						= (bd_t *)global_data_base;

	/* initialize malloc() area */
	mem_malloc_init();			BOOTLOG("init malloc");

	if (env_init()) 			{ hang(); } else { BOOTLOG("env init");}		/* initialize environment */


	
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
	nand_init();				BOOTLOG("init nand");
#endif
	/* relocate environment function pointers etc */
	env_relocate(); 			BOOTLOG("env relocate");

	if (OSA_MD_GetDbgMode() == RELEASE_LEVEL)
		set_releaseEnv();

//	if (timer_init())			{ hang(); } else { BOOTLOG("timer init");}		/* Initialize timer */
#ifdef CONFIG_INCA_IP
	if (incaip_set_cpuclk())	{ hang(); } else { BOOTLOG("set cpuclk");}		/* set cpu clock according to environment */
#endif
	if (init_baudrate())		{ hang(); } else { BOOTLOG("init baudrate");}	/* initialze baudrate settings */
	if (serial_init())			{ hang(); } else { BOOTLOG("init serial");}		/* serial communications setup */
	if (console_init_f())		{ hang(); } else { BOOTLOG("init console");}
#if (OPTIMIZE_BOOT == 0)
	if (display_banner())		{ hang(); } else { BOOTLOG("display banner");}	/* say that we are here */
	if (checkboard())			{ hang(); } else { BOOTLOG("check board");}
#endif
	if (init_func_ram())		{ hang(); } else { BOOTLOG("init func ram");}

#if (OPTIMIZE_BOOT == 0)
//dhjung LGE
	printf("NAND\t: ");
	printf("%luMB\n", nand_info[0].size / (1024 * 1024));
	print_dqsvalue();
	printf("TB:0x%08x,MB:0x%08x,GD:0x%08x,SP:0x%08x\n",
			(ulong)&_start,
			malloc_base,
			global_data_base,
			stack_ptr);
#endif

	gd->bd->bi_boot_params		= global_data_base + sizeof(bd_t);
	gd->bd->bi_memstart  		= CPU_MEM_BASE;
	gd->bd->bi_memsize    		= gd->ram_size;
	gd->bd->bi_baudrate  		= gd->baudrate;

	/* configure available FLASH banks */
#ifndef CFG_NO_FLASH
	size	= flash_init();
	BOOTLOG("init flash");

	display_flash_config (size);
	BOOTLOG("disp flash config");

	gd->bd->bi_flashstart		= CFG_FLASH_BASE;
	gd->bd->bi_flashsize		= size;
	gd->bd->bi_flashoffset		= 0;
#endif

	devices_preinit();
	BOOTLOG("device pre init");
	fast_boot();
	BOOTLOG("fast boot");

#if (CONFIG_COMMANDS & CFG_CMD_NET)
	/* board MAC address */
	s = getenv ("ethaddr");
	for (i = 0; i < 6; ++i) {
		gd->bd->bi_enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr("ipaddr");
	BOOTLOG("getenv ip addr");
#endif

#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	pci_init();
#endif

	/*
	 * leave this here (after malloc(), environment and PCI are working)
	 */
	/* Initialize devices */
	devices_init ();
	BOOTLOG("init devices");

	jumptable_init ();
	BOOTLOG("init jump table");

	/* Initialize the console (after the relocation and devices init) */
	console_init_r ();
	BOOTLOG("init console");

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if (CONFIG_COMMANDS & CFG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif	/* CFG_CMD_NET */

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
	BOOTLOG("init misc");
#endif

#if (CONFIG_COMMANDS & CFG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
//	puts ("Net:   ");
#endif
	eth_initialize(gd->bd);
	BOOTLOG("init eth");
#endif

//dhjung LGE
	Chip_Read_Memory();

	SHOWLOG();

	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
