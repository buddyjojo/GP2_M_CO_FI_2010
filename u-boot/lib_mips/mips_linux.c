/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <zlib.h>
#include <asm/byteorder.h>
#include <asm/addrspace.h>
//dhjung LGE
#include <mtdinfo.h>
#include <nvm.h>
#include "bgtask_powseq.h"

DECLARE_GLOBAL_DATA_PTR;

#define	LINUX_MAX_ENVS		256
#define	LINUX_MAX_ARGS		1024

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
# define SHOW_BOOT_PROGRESS(arg)	show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

extern image_header_t header;           /* from cmd_bootm.c */

extern uint32_t appxip_len;				/* app xip length */
extern uint32_t fontxip_len;			/* font xip length */
extern char boot2ndVer[];

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
//dhjung LGE
extern void dcache_disable (void);
extern void Chip_Flush_Memory(void);
extern void DDI_NVM_GetToolOpt(void);
extern int DDI_GetModelOption(void);
extern unsigned int get_partition_information(PART_INFO_T partinfo, char *name);
extern char *get_1stbootver(void);

static int	linux_argc;
static char **	linux_argv;

static char **	linux_env;
static char *	linux_env_p;
static int	linux_env_idx;

static void linux_params_init (ulong start);
static void linux_env_set (char * env_name, char * env_val);


void do_bootm_linux (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[],
		     ulong addr, ulong * len_ptr, int verify)
{
	ulong len = 0, checksum;
	ulong initrd_start, initrd_end;
	ulong data;
	void (*theKernel) (int, char **, char **, int *);
	image_header_t *hdr = &header;
	char env_buf[12];

	theKernel =
		(void (*)(int, char **, char **, int *)) ntohl (hdr->ih_ep);

	/*
	 * Check if there is an initrd image
	 */
	if (argc >= 3) {
		SHOW_BOOT_PROGRESS (9);

		addr = simple_strtoul (argv[2], NULL, 16);

		printf ("[%ld] ## Loading Ramdisk Image at %08lx ...\n", get_cur_time(), addr);

		/* Copy header so we can blank CRC field for re-calculation */
		memcpy (&header, (char *) addr, sizeof (image_header_t));

		if (ntohl (hdr->ih_magic) != IH_MAGIC) {
			printf ("Bad Magic Number\n");
			SHOW_BOOT_PROGRESS (-10);
			do_reset (cmdtp, flag, argc, argv);
		}

		data = (ulong) & header;
		len = sizeof (image_header_t);

		checksum = ntohl (hdr->ih_hcrc);
		hdr->ih_hcrc = 0;

		if (crc32 (0, (uchar *) data, len) != checksum) {
			printf ("Bad Header Checksum\n");
			SHOW_BOOT_PROGRESS (-11);
			do_reset (cmdtp, flag, argc, argv);
		}

		SHOW_BOOT_PROGRESS (10);

		print_image_hdr (hdr);

		data = addr + sizeof (image_header_t);
		len = ntohl (hdr->ih_size);

		if (verify) {
			ulong csum = 0;

			printf ("   Verifying Checksum ... ");
			csum = crc32 (0, (uchar *) data, len);
			if (csum != ntohl (hdr->ih_dcrc)) {
				printf ("Bad Data CRC\n");
				SHOW_BOOT_PROGRESS (-12);
				do_reset (cmdtp, flag, argc, argv);
			}
			printf ("OK\n");
		}

		SHOW_BOOT_PROGRESS (11);

		if ((hdr->ih_os != IH_OS_LINUX) ||
		    (hdr->ih_arch != IH_CPU_MIPS) ||
		    (hdr->ih_type != IH_TYPE_RAMDISK)) {
			printf ("No Linux MIPS Ramdisk Image\n");
			SHOW_BOOT_PROGRESS (-13);
			do_reset (cmdtp, flag, argc, argv);
		}

		/*
		 * Now check if we have a multifile image
		 */
	} else if ((hdr->ih_type == IH_TYPE_MULTI) && (len_ptr[1])) {
		ulong tail = ntohl (len_ptr[0]) % 4;
		int i;

		SHOW_BOOT_PROGRESS (13);

		/* skip kernel length and terminator */
		data = (ulong) (&len_ptr[2]);
		/* skip any additional image length fields */
		for (i = 1; len_ptr[i]; ++i)
			data += 4;
		/* add kernel length, and align */
		data += ntohl (len_ptr[0]);
		if (tail) {
			data += 4 - tail;
		}

		len = ntohl (len_ptr[1]);

	} else {
		/*
		 * no initrd image
		 */
		SHOW_BOOT_PROGRESS (14);

		data = 0;
	}

#ifdef	DEBUG
	if (!data) {
		printf ("No initrd\n");
	}
#endif

	if (data) {
		initrd_start = data;
		initrd_end = initrd_start + len;
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

	SHOW_BOOT_PROGRESS (15);

#ifdef DEBUG
	printf ("[%ld] ## Transferring control to Linux (at address %08lx) ...\n",
		get_cur_time(), (ulong) theKernel);
#endif
	BG_PowerSeqTask(BGTASK_END);

	linux_params_init (UNCACHED_SDRAM (gd->bd->bi_boot_params));

#if (OPTIMIZE_BOOT == 0)
#ifdef CONFIG_MEMSIZE_IN_BYTES
	sprintf (env_buf, "%lu", gd->ram_size);
#ifdef DEBUG
	printf ("## Giving linux memsize in bytes, %lu\n", gd->ram_size);
#endif
#else
	sprintf (env_buf, "%lu", gd->ram_size >> 20);
#ifdef DEBUG
	printf ("## Giving linux memsize in MB, %lu\n", gd->ram_size >> 20);
#endif
#endif /* CONFIG_MEMSIZE_IN_BYTES */

	linux_env_set ("memsize", env_buf);

	sprintf (env_buf, "0x%08X", (uint) UNCACHED_SDRAM (initrd_start));
	linux_env_set ("initrd_start", env_buf);

	sprintf (env_buf, "0x%X", (uint) (initrd_end - initrd_start));
	linux_env_set ("initrd_size", env_buf);

	sprintf (env_buf, "0x%08X", (uint) (gd->bd->bi_flashstart));
	linux_env_set ("flash_start", env_buf);

	sprintf (env_buf, "0x%X", (uint) (gd->bd->bi_flashsize));
	linux_env_set ("flash_size", env_buf);
#endif


	/* we assume that the kernel is in place */
	//gd->flags &= ~(GD_FLG_SILENT);
	printf ("\n[%ld] Starting kernel ...\n\n", get_cur_time());

//dhjung LGE
	dcache_disable();
	Chip_Flush_Memory();
	
	theKernel (linux_argc, linux_argv, linux_env, 0);
}

static void linux_params_init (ulong start)
{
	char	*argp, *s;
	char	*commandline;
	char	*lserverip, *serverip, *ipaddr;
	char	*gatewayip, *netmask;
	char	*hostname, *autoboot;
	char	*silent_app, *silent_ker, *lpj, *bootlogo, *lock_time,*enable_probe;
	int		ramdisk;
	uint32_t initrd_size = 0;
	uint32_t xip_size_mb = 0, appxip_addr = 0, fontxip_addr = 0;
	uchar nDebugStatus;

	nDebugStatus = OSA_MD_GetDbgMode();
	
	linux_argc = 1;
	linux_argv = (char **) start;
	linux_argv[0] = 0;
	argp = (char *) (linux_argv + LINUX_MAX_ARGS);

	/*
	 * ADD bootargs...
	 */
	commandline = getenv ("bootargs");
	sprintf(argp, "%s", commandline);
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	/*
	 * ADD board Configurations
	 * ADD "host=XXX"
	 */
	hostname	= getenv("hostname");
	sprintf(argp, "host=%s", hostname);
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	/*
	 * ADD "initrd=xxxx@xxxx" or "noinitrd"
	 * ADD "ramdisk=XXX"
	 */
	s = getenv("rootfs");
	ramdisk = (s && (!strcmp(s,"ramdisk"))) ? 1 : 0;

	if (ramdisk == 1) {
		initrd_size = get_partition_information(PART_INFO_FILESIZE, "rootfs");
	} else {
		initrd_size = 0;
	}

	if (initrd_size > 0) {
		sprintf(argp, "rd_start=0x%x rd_size=0x%x ramdisk=%d",
				CFG_RAMDISK_ADDR, initrd_size, (initrd_size >> 10) + 32);
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	} else {
		sprintf(argp, "noinitrd ramdisk=0");
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}

	/*
	 * ADD "console=ttySX,xxxxxxn8r
	 */
	sprintf(argp, "console=ttyS0,%dn8r", gd->baudrate);
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	/*
	 * ADD Network Configurations
	 * ip=$(lserverip):$(serverip):$(ipaddr):$(gatewayip):$(netmask)
	 */
	lserverip	= getenv("lserverip");
	serverip	= getenv("serverip");
	ipaddr		= getenv("ipaddr");
	gatewayip	= getenv("gatewayip");
	netmask		= getenv("netmask");

	sprintf(argp, "ip=%s:%s:%s:%s:%s", lserverip, serverip, ipaddr, gatewayip, netmask);
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	/*
	 * ADD "autorun"
	 */
	autoboot	= getenv("autoboot");
	if (autoboot && (!strcmp(autoboot, "y"))) {
		sprintf(argp, "autorun");
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}

	/*
	 * ADD "appxip_addr=XXX fontxip_addr=XXX xipfs=XXX"
	 */
	if (appxip_len > 0) {
		appxip_addr = CFG_SDRAM_BASE + ((gd->ram_size << 20) - appxip_len);
	} else {
		appxip_addr = 0;
	}

	if (fontxip_len > 0) {
		if (appxip_addr > 0)
			fontxip_addr = appxip_addr - fontxip_len;
		else
			fontxip_addr = CFG_SDRAM_BASE + ((gd->ram_size << 20) - fontxip_len);
	} else {
		fontxip_addr = 0;
	}

	xip_size_mb	= (appxip_len ? (appxip_len >> 20) : 0) + (fontxip_len ? (fontxip_len >> 20) : 0);

	sprintf(argp, "appxip_addr=0x%x fontxip_addr=0x%x xipfs=%d", PHYSADDR(appxip_addr), PHYSADDR(fontxip_addr), xip_size_mb);
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	/*
	 * ADD "quiet_app"
	 */
	silent_app	= getenv("silent_app");
	if (silent_app && (!strcmp(silent_app, "y"))) {
		sprintf(argp, "quiet_app");
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}

	/*
	 * ADD "quiet_ker"
	 */
	silent_ker	= getenv("silent_ker");
	if (silent_ker && (!strcmp(silent_ker, "y"))) {
		sprintf(argp, "loglevel=0");
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}
	
	if (nDebugStatus == RELEASE_LEVEL)
	{
		sprintf(argp, "panic=1");
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}
	/*
	 * ADD "lock_time"
	 */
	lock_time	= getenv("lock_time");
	if (lock_time && (strcmp(lock_time, "0"))) {
		sprintf(argp, "lock_time=%s",lock_time);
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}

	/*
	 * ADD "enable_probe"
	 */
	enable_probe	= getenv("enable_probe");
	if (enable_probe && (strcmp(enable_probe, "0"))) {
		sprintf(argp, "enable_probe=%s",enable_probe);
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}

	/*
	*	lcd/pdp model option check
	*/
	if(!DDI_GetModelOption())
	{
		sprintf(argp, "lcdmodels");
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}
	else
	{	
		sprintf(argp, "pdpmodels");
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}

	/*
	 * ADD "memsize=XXX"
	 */
	sprintf(argp, "memsize=%d", gd->ram_size);
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	/*
	 * ADD "boot1stver=XXX boot2ndver=XXX"
	 */
	sprintf(argp, "boot1stver=%s boot2ndver=%s", (char *)get_1stbootver(), boot2ndVer);
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	/*
	 * ADD "nosplash"
	 */
	if ((bootlogo = getenv("bootlogo")) != NULL) {
		if (*bootlogo == 'n')
			sprintf(argp, "nosplash");
		else
			goto next_arg;
	} else {
		DDI_NVM_GetToolOpt();
		if(!gToolOptionDB.nToolOption3.flags.bBootLogo)
			sprintf(argp, "nosplash");
		else
			goto next_arg;
	}
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

next_arg:
	/*
	 * ADD "bt=XXX dvr=XXX"
	 */
	DDI_NVM_GetToolOpt();
	sprintf(argp, "bt=%d dvr=%d", gToolOptionDB.nToolOption3.flags.bBluetooth, gToolOptionDB.nToolOption3.flags.bDVRReady);
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	/*
	 * ADD "lpj=XXX"
	 */
	lpj	= getenv("lpj");
	if (lpj) {
		sprintf(argp, "lpj=%s", lpj);
		linux_argv[linux_argc] = argp;
		argp += strlen(argp) + 1;
		linux_argc++;
	}

	/*
	 * Add "start_kernel=xxxx"
	 */
	sprintf(argp, "start_kernel=%d ", get_cur_time());
	linux_argv[linux_argc] = argp;
	argp += strlen(argp) + 1;
	linux_argc++;

	linux_env = (char **) (((ulong) argp + 15) & ~15);
	linux_env[0] = 0;
	linux_env_p = (char *) (linux_env + LINUX_MAX_ENVS);
	linux_env_idx = 0;
}

static void linux_env_set (char *env_name, char *env_val)
{
	if (linux_env_idx < LINUX_MAX_ENVS - 1) {
		linux_env[linux_env_idx] = linux_env_p;

		strcpy (linux_env_p, env_name);
		linux_env_p += strlen (env_name);

		strcpy (linux_env_p, "=");
		linux_env_p += 1;

		strcpy (linux_env_p, env_val);
		linux_env_p += strlen (env_val);

		linux_env_p++;
		linux_env[++linux_env_idx] = 0;
	}
}
