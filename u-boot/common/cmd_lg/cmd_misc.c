//by dhjung LGE
#include <common.h>
#include <command.h>
#include <net.h>
#include <mtdinfo.h>
#include <zmodem.h>
#include <environment.h>

extern block_dev_desc_t *get_dev (char* ifname, int dev);
extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);
extern void set_uart_baudrate(int baudrate);
extern long file_fat_read(const char *filename, void *buffer, unsigned long maxsize);

#ifdef MTDINFO_IN_FLASH
extern int write_flash_data(uchar *src, ulong addr, ulong cnt);
#else
/* references to names in cmd_nand.c */
#include <nand.h>
extern nand_info_t nand_info[];
int write_flash_data(nand_info_t *nand, ulong ofs, ulong cnt, u_char *src);
#endif

#ifdef CONFIG_MTD_NAND_BBM
extern int nand_bbm_update(struct mtd_info * mtd);
#endif
extern u8 SWU_MICOM_BinUpdate(char* pImg_start, u32 size);
extern u8 SWU_SPI_Update(unsigned char* pImg_start, u32 size);

extern uchar default_environment[];
extern int default_environment_size;
extern env_t *env_ptr;

DECLARE_GLOBAL_DATA_PTR;

char boot1stVer[10];
ulong default_offset = CFG_LOAD_ADDR;

int write_flash_data(nand_info_t *nand, ulong ofs, ulong cnt, u_char *src)
{
	ulong length;
	ulong start=0, offset;
	ulong erasesize = nand->erasesize;
	int ret = 0;

	length  = cnt;
	length += (length % erasesize) ? (erasesize - (length % erasesize)) : 0;

	/* 1. erase */
	printf ("Erasing Nand (0x%08x+0x%08x) ---> ", (unsigned int)ofs, (unsigned int)length);
	for(start = 0; start < length; start += erasesize) {
		offset = ofs + start;
		// erase block
		if (nand_erase(nand, offset, erasesize)) {
			printf ("Failed\n");
			return 1;
		}
	}
	printf ("OK\n");

#ifdef CONFIG_MTD_NAND_BBM
	/* 2. bbm update */
	nand_bbm_update(nand);
#endif

	/* 3. write */
	printf ("Writing Nand (0x%08x+0x%08x) ---> ", (unsigned int)ofs, (unsigned int)length);
	ret = nand_write(nand, ofs, &length, src);
	if(ret) {
		printf ("Failed[%d]\n", ret);
		return 1;
	}
	printf ("OK\n");

	return 0;
}

#ifdef CONFIG_LOAD_TFTP
int tftp_get (char *filename)
{
	int  size;
	char fullpath[128];
#ifdef CONFIG_TFTP_PATH_IN_TFTPSTART
	char *prefix;

	prefix = getenv("userpath");
	if (prefix != NULL) {
		sprintf(fullpath, "%s%s", prefix, filename);
	}
	else
		sprintf(fullpath, "%s", filename);
#else
	sprintf(fullpath, "%s", filename);
#endif

	load_addr = default_offset;
	copy_filename (BootFile, fullpath, strlen(fullpath)+1);

	if ((size = NetLoop(TFTP)) < 0)
		return 1;

	/* done if no file was loaded (no errors though) */
	if (size == 0)
		return 0;

	return size;
}
#endif

#ifdef CONFIG_LOAD_FAT32
int fat_fsload(char *filename)
{
	long size;
	block_dev_desc_t *dev_desc = NULL;
	int dev = 0, part = 1;

	dev_desc = get_dev("usb", dev);
	if (dev_desc == NULL) {
		puts ("Invalid boot device\n");
		return 1;
	}

	if (fat_register_device(dev_desc, part) != 0) {
		printf ("Unable to use usb %d:%d for fatload\n", dev, part);
		return 1;
	}

	size = file_fat_read(filename, (unsigned char *) default_offset, 0);
	if(size == -1) {
		printf("Unable to read \"%s\" from usb %d:%d\n", filename, dev, part);
		return 1;
	}

	return size;
}
#endif

#ifdef CONFIG_LOAD_KERMIT
ulong kermit_get(ulong offset)
{
	int size, i;

	set_kerm_bin_mode ((ulong *) offset);
	size = k_recv ();

	/*
	 * Gather any trailing characters (for instance, the ^D which
	 * is sent by 'cu' after sending a file), and give the
	 * box some time (100 * 1 ms)
	 */
	for (i=0; i<100; ++i) {
		if (tstc()) {
			(void) getc();
		}
		udelay(1000);
	}

	flush_cache (offset, size);

	printf("## Total Size      = 0x%08x = %d Bytes\n", size, size);

	return size;
}

int do_rkermit (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong size=0;
	ulong address=0;

	address = simple_strtoul(argv[1], NULL, 16);
	size = kermit_get(address);

	if(!size) {
		printf("rkermit failed...(size:%d)\n", size);
		return -1;
	}

	return 0;
}

U_BOOT_CMD(
	rk,	2,	0,	do_rkermit,
	"rk\t- downlaod image file,though zmodem\n",
	"[address]\n"
);
#endif

#ifdef CONFIG_LOAD_SERIAL
ulong get_serial_data(ulong offset)
{
	extern int serial_tstc (void);
	extern int serial_getc (void);

	unsigned char  *buf = NULL;
	ulong  ch;
	ulong len=0;
	ulong sTimer, cTimer = 0, dsTimer = 0, tmdiff = 0;
	ulong timeout=60, dataflush;
	printf("Download from serial... send image file\n");

	len = 0;
	buf = (unsigned char*)(default_offset);

	do {
		sTimer = get_timer(0);
_getc:
		if ( serial_tstc() ) {
			ch = serial_getc();
			buf[len] = (unsigned char)ch;
			if(len==0)
				dsTimer = get_timer(0);
			len++;
			timeout = 1;
		} else {
			cTimer = get_timer(0);
			tmdiff = (cTimer - sTimer) / CFG_HZ;
			if(tmdiff > timeout)	{
				goto _end_of_down;
			}
			goto _getc;
		}
	} while(1);

_end_of_down:
	printf("Download done size : %d bytes in %d.%03dms\n", len, (cTimer-dsTimer)/1000, (cTimer-dsTimer)%1000);

	while (serial_tstc())
		dataflush = serial_getc();

	return len;
}

int do_rserial(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong size=0;
	ulong address=0;

	address = simple_strtoul(argv[1], NULL, 16);
	size = get_serial_data(address);

	if(!size) {
		printf("rserial failed...(size:%d)\n", size);
		return -1;
	}

	return 0;
}

U_BOOT_CMD(
	rs,	2,	0,	do_rserial,
	"rs\t- downlaod image file,though zmodem\n",
	"[address]\n"
);
#endif

static int _load(int type, char* partname)
{
	ulong size = 0, floffset = 0, flsize = 0, idx = 0;
	struct mtd_partition_info *mpi = NULL, *mtdmpi = NULL;
	struct boot_mtd_info *pmi = NULL;
	char dnfilename[64];

	if(!strncmp(partname, "spiboot", 4))
	{
		sprintf(dnfilename, "u-boot-spi.bin");
		goto loads_start;
	}

	if(!strncmp(partname, "lcdmicom", 4))
	{
		sprintf(dnfilename, "micom_lcd.hex");
		goto loads_start;
	}

	if(!strncmp(partname, "pdpmicom", 4))
	{
		sprintf(dnfilename, "micom_pdp.hex");
		goto loads_start;
	}

	mpi = get_partition(partname);
	if(!mpi) {
		printf("unknown partition : %s\n", partname);
		return 0;
	}

	floffset	= (ulong)(mpi->offset + CFG_FLASH_BASE);
	flsize		= (ulong)mpi->size;
	sprintf(dnfilename, mpi->filename);

loads_start:
	switch(type) {
#ifdef CONFIG_LOAD_TFTP
		case LOAD_TYPE_TFTP:
			size = tftp_get(dnfilename);
			break;
#endif
#ifdef CONFIG_LOAD_FAT32
		case LOAD_TYPE_FAT32:
			size = fat_fsload(dnfilename);
			break;
#endif
#ifdef CONFIG_LOAD_ZMODEM
		case LOAD_TYPE_ZMODEM:
			size = rz("test.txt", default_offset);
			break;
#endif
#ifdef CONFIG_LOAD_HZMODEM
		case LOAD_TYPE_HZMODEM:
			set_uart_baudrate(CONFIG_RZ_BAUDRATE);
			size = rz("test.txt", default_offset);
			break;
#endif
#ifdef CONFIG_LOAD_KERMIT
		case LOAD_TYPE_KERMIT:
			size = kermit_get(default_offset);
			break;
#endif
#ifdef CONFIG_LOAD_SERIAL
		case LOAD_TYPE_SERIAL:
			size = get_serial_data(default_offset);
			break;
#endif
		default:
			size = 0;
			break;
	}

	if(size == 0) {
		printf("size is zero..!!\n");
		goto loads_end;
	}

	if(flsize != 0 && (size > flsize)) {
		printf("out of partition size : %x / %x\n", (unsigned int)size, (unsigned int)flsize);
		goto loads_end;
	}

	if(!strncmp(partname, "spiboot", 4)) {
		SWU_SPI_Update((unsigned char *) default_offset, size);
		goto loads_end;
	}

	if(!strncmp(partname, "lcdmicom", 4) || !strncmp(partname, "pdpmicom", 4)) {
		SWU_MICOM_BinUpdate((char *)default_offset, size);
		goto loads_end;
	}

	if (!strcmp(mpi->name, "mtdinfo")) {
		pmi = (struct boot_mtd_info *)(default_offset);
		do {
			mtdmpi = &(pmi->partition[idx]);
			if(mtdmpi->used && mtdmpi->valid && (strncmp(mpi->name, mtdmpi->name, 4) == 0))
				break;
		}while(++idx < MTD_PARTITION_MAX);

		if (mpi->sw_ver == mtdmpi->sw_ver) {
			printf("mtdinfo version is same case!! 0x%x -> 0x%x\n", mpi->sw_ver, mtdmpi->sw_ver);
			goto loads_end;
		}
		printf("mtdinfo version is different case!! 0x%x -> 0x%x\n", mpi->sw_ver, mtdmpi->sw_ver);
	}

	#ifdef MTDINFO_IN_FLASH
	flash_protect(FLAG_PROTECT_CLEAR, floffset, floffset+size-1, &flash_info[0]);
	write_flash_data((uchar*)default_offset, floffset, size);
	#else
	write_flash_data(&nand_info[0], (ulong)floffset, (ulong)size, (u_char *)default_offset);
	#endif
	mpi->filesize = size;

	/* save mtdinfo */
	if (strcmp(mpi->name, "mtdinfo"))
		save_mtdinfo();

	/* load mtdinfo */
	if (!strcmp(mpi->name, "mtdinfo")) {
		load_mtdinfo();
		erase_cachepart();
	}

loads_end:
#ifdef CONFIG_LOAD_HZMODEM
	if(type == LOAD_TYPE_HZMODEM) {
		printf("\n\n");
		set_uart_baudrate(CONFIG_BAUDRATE);
	}
#endif

	return 0;
}

#ifdef CONFIG_LOAD_TFTP
int do_load (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}
	return _load(LOAD_TYPE_TFTP, argv[1]);
}

U_BOOT_CMD(
	load,	2,	0,	do_load,
	"load\t- downlaod image file using tftp, and write on flash \n",
	" [flashoffset] \n [partition name] - \n"
);
#endif

#ifdef CONFIG_LOAD_FAT32
int do_loadm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}
	return _load(LOAD_TYPE_FAT32, argv[1]);
}

U_BOOT_CMD(
	loadm,	2,	0,	do_loadm,
	"loadm\t- downlaod image file using memory stick, and write on flash \n",
	" [flashoffset] \n [partition name] - \n"
);
#endif

#ifdef CONFIG_LOAD_ZMODEM
int do_loadz (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}
	return _load(LOAD_TYPE_ZMODEM, argv[1]);
}

U_BOOT_CMD(
	loadz,	2,	0,	do_loadz,
	"loadz\t- downlaod image file using zmodem, and write on flash \n",
	" [flashoffset] \n [partition name] - \n"
);
#endif

#ifdef CONFIG_LOAD_HZMODEM
int do_loadhz (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}
	return _load(LOAD_TYPE_HZMODEM, argv[1]);
}

U_BOOT_CMD(
	loadhz,	2,	0,	do_loadhz,
	"loadhz\t- downlaod image file using zmodem(460800), and write on flash \n",
	" [flashoffset] \n [partition name] - \n"
);
#endif

#ifdef CONFIG_LOAD_KERMIT
int do_loadk (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}
	return _load(LOAD_TYPE_KERMIT, argv[1]);
}

U_BOOT_CMD(
	loadk,	2,	0,	do_loadk,
	"loadk\t- downlaod image file using zmodem, and write on flash \n",
	" [flashoffset] \n [partition name] - \n"
);
#endif

#ifdef CONFIG_LOAD_SERIAL
int do_loads (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}
	return _load(LOAD_TYPE_SERIAL, argv[1]);
}

U_BOOT_CMD(
	loads,	2,	0,	do_loads,
	"loads\t- downlaod image file using serial, and write on flash \n",
	" [flashoffset] \n [partition name] - \n"
);
#endif

int do_defaultenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	if (default_environment_size > CFG_ENV_SIZE) {
		puts ("*** Error - default environment is too large\n\n");
		return 1;
	}

	memset (env_ptr, 0, sizeof(env_t));
	memcpy (env_ptr->data, default_environment, default_environment_size);
	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
	gd->env_valid = 1;

	ret = write_flash_data(&nand_info[0], (ulong)CFG_ENV_OFFSET, (ulong)CFG_ENV_SIZE, (u_char *)env_ptr);
	if (ret) {
		printf("env save failed\n");
		return 1;
	}

	return ret;
}

U_BOOT_CMD(
	defaultenv, 1,  0,  do_defaultenv,
	"defaultenv\t- set default env\n",
	"\n"
);

extern cmd_tbl_t __u_boot_cmd_setboot;
extern int do_setboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int saveenv(void)
{
	char *rootfs_str, *argv[2];
	int	ret = 0;

	rootfs_str = getenv("rootfs");

	argv[0] = "setboot";
	argv[1] = rootfs_str;

	ret = do_setboot (&__u_boot_cmd_setboot, 0, 2, argv);
	if (ret) {
		printf("env save failed\n");
		return 1;
	}

	return ret;
}

int do_revert(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong size = 0, floffset = 0, flsize = 0;
	struct mtd_partition_info *mpi = NULL;

	mpi = get_partition("lginit");
	if(!mpi) {
		printf("unknown partition : %s\n", "lginit");
		return -1;
	}

	mpi->filesize = 0;
	mpi->sw_ver = 0;

	nand_erase(&nand_info[0], mpi->offset, mpi->size);

	mpi = get_partition("lgapp");
	if(!mpi) {
		printf("unknown partition : %s\n", "lgapp");
		return -1;
	}

	mpi->filesize = 0;
	mpi->sw_ver = 0;

	nand_erase(&nand_info[0], mpi->offset, mpi->size);

	mpi = get_partition("lgfont");
	if(!mpi) {
		printf("unknown partition : %s\n", "lgfont");
		return -1;
	}

	mpi->filesize = 0;
	mpi->sw_ver = 0;

	nand_erase(&nand_info[0], mpi->offset, mpi->size);

	mpi = get_partition("kernel");
	if(!mpi) {
		printf("unknown partition : %s\n", "kernel");
		return 0;
	}

	floffset	= (ulong)(mpi->offset + CFG_FLASH_BASE);
	flsize		= (ulong)mpi->size;
	sprintf(mpi->filename, "uImage"); //to revert to normal

	size = tftp_get(mpi->filename);
	if(size == 0) {
		printf("size is zero..!!\n");
		return -1;
	}

	if(size > flsize) {
		printf("out of partition size : %x / %x\n", (unsigned int)size, (unsigned int)flsize);
		return -1;
	}

	write_flash_data(&nand_info[0], (ulong)floffset, (ulong)size, (u_char *)default_offset);
	mpi->filesize = size;

	save_mtdinfo();

	return 0;
}

U_BOOT_CMD(
	revert,	1,	0,	do_revert,
	"revert\t- revert to normal mode\n",
	"revert \n"
);

int get_tick(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("current time tick = %ld\n", get_cur_time());
	return 0;
}

U_BOOT_CMD(
	tick,	1,	0,	get_tick,
	"tick\t- show time tick\n",
	"tick \n"
);

#define	CACHE_LINE_WORDS			(8)
int test_cachemissmemcpy(volatile unsigned int * pSrc, volatile unsigned int * pDest, unsigned int count)
{
	volatile unsigned int *p1 = NULL;
	volatile unsigned int *p2 = NULL;
	unsigned int	i, k;
	unsigned int	nTest;
	unsigned int	loop;

	for (nTest = 0; nTest < 10; nTest++)
	{
		p1 = (volatile unsigned int *) pSrc;
		p2 = (volatile unsigned int *) pDest;
		loop = (count/sizeof(unsigned int));

		for (k = 0; k < CACHE_LINE_WORDS ; k++)
			for (i = 0; i < loop; i+=CACHE_LINE_WORDS)
				p2[k+i] = p1[k+i];
	}

	return nTest;
}

int test_membandwidth(volatile unsigned int * pSrc, volatile unsigned int * pDest, unsigned int count)
{
	volatile unsigned int *p1 = NULL;
	volatile unsigned int *p2 = NULL;
	unsigned int	i;
	unsigned int	nTest;
	unsigned int	loop;

	for (nTest = 0; nTest < 10; nTest++)
	{
		p1 = (volatile unsigned int *) pSrc;
		p2 = (volatile unsigned int *) pDest;
		loop = (count/sizeof(unsigned int));

		for (i = 0; i < loop; i++)
			p2[i] = p1[i];
	}

	return nTest;
}

int do_chkmem(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	volatile unsigned int	*src, *dst;
	unsigned int count;
	int		loop;
	int		test_count;
	unsigned long	delay;
	unsigned long	tss, tse;

	if (argc != 2) {
		printf("argc=%d, Usage:\n%s\n", argc, cmdtp->usage);
		return -1;
	}

	count = simple_strtoul(argv[1], NULL, 16);
	if (count == 0 || count & 0x3) {
		printf("Zero length or invalid count\n");
		return -1;
	}

	src = (volatile unsigned int *) 0x82000000;
	dst = src + count;

	loop  = 0;
	printf("<mem read/write test>\n");
	while (loop < 3)
	{
		loop++;
		tss = get_cur_time();
		test_count = test_membandwidth(src, dst, count);
		tse = get_cur_time();

		delay = (tse > tss)? (tse-tss) : (0xFFFFFFFF-tss+tse);
		printf("Read/Write %3d: %d times, %8d bytes, %4ul msec => %6d KB/sec\n",
					loop,
					test_count,
					count,
					delay,
					(((count * test_count) / 1024) * 1000) / delay);
	}
	printf("\n<Cache Miss mem read/write test>\n");
	loop = 0;
	while (loop < 3)
	{
		loop++;

		tss = get_cur_time();
		test_count = test_cachemissmemcpy(src, dst, count);
		tse = get_cur_time();

		delay = (tse > tss)? (tse-tss) : (0xFFFFFFFF-tss+tse);
		printf("Read/Write %3d: %d times, %8d bytes, %4ul msec => %6d KB/sec\n",
					loop,
					test_count,
					count,
					delay,
					(((count * test_count) / 1024) * 1000) / delay);
	}
	return 0;
}

U_BOOT_CMD(
	chkmem,	2,	0,	do_chkmem,
	"chkmem\t- memory copy test\n",
	"chkmem count(bytes) \n"
);

extern void drvNAND_SetSpeed(int index);

int do_nandset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int index;

	if (argc != 2) {
		printf("argc=%d, Usage:\n%s\n", argc, cmdtp->usage);
		return -1;
	}

	index = simple_strtoul(argv[1], NULL, 16);
	drvNAND_SetSpeed(index);

	return 0;
}

U_BOOT_CMD(
	nandset, 2,	0,	do_nandset,
	"nandset\t- nand clock set\n",
	"nandset 1 ~ 4\n"
);

int erase_cachepart(void)
{
	struct mtd_partition_info *mpi = NULL;
	int i=0;

	for(i=0; i<GET_PARTITION_NUM(); i++) {
		mpi = GET_PARTITION_INFO(i);

		if(mpi->mask_flags & MTD_FLG_CACHE) {
			printf("MTD [%02d] %s\t part is cache part\n", i, mpi->name);
			nand_erase(&nand_info[0], (ulong)mpi->offset, (ulong)mpi->size);
			printf("nand erased : ofs=0x%08x ~ 0x%08x (erasesize=0x%x)\n",
					(u32)mpi->offset, (u32)(mpi->offset+mpi->size), nand_info[0].erasesize);
		}
	}

	return 0;
}

char *get_1stbootver(void)
{
	memcpy(boot1stVer, (char *)0xbfc00014, 7);
	boot1stVer[7] = '\0';

	return (char *)&boot1stVer[0];
}
