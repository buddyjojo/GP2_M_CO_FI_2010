#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <mtdinfo.h>
#include <exports.h>
#include <nand.h>

#define KCMD_RAMDISK 	"root=/dev/ram0 rootfstype=squashfs"
#define KCMD_FLASH	 	"root=/dev/mtdblock%d lginit=/dev/mtdblock%d rootfstype=squashfs"
#define KCMD_NFS	 	"root=/dev/nfs rw nfsroot=156.147.69.181:/nfsroot,nolock ip=192.168.0.10::192.168.0.1:255.255.255.0:localhost:eth0:off"

#define CMD_NULL		""
#define CMD_SEMICOLON	"; "
#define	CMD_APPXIP		"xip lgapp"
#define	CMD_FONTXIP		"xip lgfont"

#define ENV_APPXIP		"appxip"
#define ENV_FONTXIP		"fontxip"

char *environ_list[]	= {
	ENV_APPXIP,
	ENV_FONTXIP
};

char *command_list[]	= {
	CMD_NULL,
	CMD_APPXIP	CMD_SEMICOLON,
	CMD_FONTXIP CMD_SEMICOLON,
	CMD_APPXIP	CMD_SEMICOLON	CMD_FONTXIP CMD_SEMICOLON,
};

extern unsigned int get_partition_information(PART_INFO_T partinfo, char *name);
extern void setenv (char *varname, char *varvalue);
extern char *getenv (char *name);
extern char *env_name_spec;
extern env_t *env_ptr;

//by dhjung LGE
int do_setboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *s;
	char bootargs[]	= "bootargs";
	char bootcmd[] 	= "bootcmd";
	char rootenv[] 	= "rootfs";
	char kcmd[512], bcmd[512], rootstr[32];
	int  addcmd = 0, i = 0,	ret = 0;

	if(argc != 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		printf ("%s\n", cmdtp->help);
		return 1;
	}

	for (i = 0; i < 2; i++) {
		s = getenv(environ_list[i]);
		addcmd |= ((s && (*s == 'n')) ? 0 : 1) << i;
	}

	/* ramdisk | flash | nfs */
	memset(kcmd, 0, 512);
	memset(bcmd, 0, 512);
	memset(rootstr, 0, 32);

	/* case ramdisk */
	if(!strcmp(argv[1], "ramdisk")) {
		sprintf(kcmd, KCMD_RAMDISK);

		/* set bcmd */
		sprintf(bcmd, "%scp2ram kernel 0x%x; cp2ram rootfs 0x%x; bootm 0x%x", command_list[addcmd], CFG_KERLOAD_ADDR, CFG_RAMDISK_ADDR, CFG_KERLOAD_ADDR);
	}
	/* case flash */
	else if(!strcmp(argv[1], "flash")) {
		sprintf(kcmd, KCMD_FLASH, get_partition_information(PART_INFO_IDX, "rootfs"), get_partition_information(PART_INFO_IDX, "lginit"));

		/* set bcmd */
		sprintf(bcmd, "%scp2ram kernel 0x%x; bootm 0x%x", command_list[addcmd], CFG_KERLOAD_ADDR, CFG_KERLOAD_ADDR);
	}
	/* case nfsroot */
	else if(!strcmp(argv[1], "nfs")) {
		/* set kcmd */
		sprintf(kcmd, KCMD_NFS);

		/* set bcmd */
		sprintf(bcmd, "%scp2ram kernel 0x%x; bootm 0x%x", command_list[addcmd], CFG_KERLOAD_ADDR, CFG_KERLOAD_ADDR);
	}
	else {
		printf("\n");
		printf("Wrong Type : %s\n", argv[1]);
		printf ("Usage:\n%s\n", cmdtp->usage);
		printf ("%s\n", cmdtp->help);
		return 1;
	}

	/* set rootstr */
	sprintf(rootstr, argv[1]);

	printf("%-9s = %s\n", bootargs, kcmd);
	printf("%-9s = %s\n", bootcmd,  bcmd);
	printf("%-9s = %s\n", rootenv,  rootstr);

	setenv(bootargs, kcmd);
	setenv(bootcmd,  bcmd);
	setenv(rootenv,  rootstr);

	printf("\n");
//	printf ("Saving Environment to %s...\n", env_name_spec);

	ret = write_flash_data(&nand_info[0], (ulong)CFG_ENV_OFFSET, (ulong)CFG_ENV_SIZE, (u_char *)env_ptr);
	if (ret) {
		printf("env save failed\n");
		return 1;
	}

	save_mtdinfo();

	return 0;
}

U_BOOT_CMD(
	setboot,	  2,	  0,	  do_setboot,
	"setboot\t- set boot type(root filesystem)\n",
	"setboot [ramdisk | flash | nfs]\n"
);
