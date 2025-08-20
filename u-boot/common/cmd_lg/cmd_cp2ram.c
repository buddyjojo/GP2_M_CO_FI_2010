#include <common.h>
#include <command.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <mtdinfo.h>
#include <exports.h>
#include "bgtask_powseq.h"

#define	FS_HDR_SIZE			32
#define CRMFS_HDR           "Compressed ROMFS"

extern int lzo_compare_magic(unsigned char *buf);
extern int lzo_get_destsize(unsigned char * buf);
extern int lzo_do_decomp(unsigned char * pDecomp, unsigned char * pComp, unsigned long * pDecompSize);

#ifdef MTDINFO_IN_FLASH
extern int write_flash_data(uchar *src, ulong addr, ulong cnt);
#else
/* references to names in cmd_nand.c */
#include <nand.h>
extern nand_info_t nand_info[];
extern int write_flash_data(nand_info_t *nand, ulong ofs, ulong cnt, u_char *src);
#endif

ulong appxip_len = 0;	/* app xip length */
ulong fontxip_len = 0;	/* font xip length */

DECLARE_GLOBAL_DATA_PTR;


//by dhjung LGE
int do_xip (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char	*s,	*fshdr, pname[MTD_STR_LEN];
	int		idx = 0, ret = 0;
	ulong	*xiplen = NULL;
	ulong	s_msec, e_msec, src, dst, cp_pos;
	ulong	rd_size = 0;
	long	tot_size = 0, left = 0;
	ulong	imgfilesize = 0, tmpfilesize = 0;
	ulong	decomp_size1 = 0, decomp_size2 = 0;
	struct	mtd_partition_info *mpi = NULL;
	static	ulong xip_size = 0;

	if (argc != 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/* find partition */
	do {
		mpi = GET_PARTITION_INFO(idx);
		if(mpi->used && mpi->valid && (strcmp(argv[1], mpi->name) == 0)) {
			imgfilesize = mpi->filesize;
			break;
		}
	} while (++idx < MTD_PARTITION_MAX);

	if (idx == MTD_PARTITION_MAX) {
		printf("Unknown partition name(%s)..\n", pname);
		return 1;
	}

	if (imgfilesize == 0) {
		printf("Partition Image Size Zero Length Case!! ==> Skipped!!\n");
		return 1;
	}

	if (!strcmp(mpi->name, "lgapp")) {
		s = getenv("appxip");
		if (*s == 'n' || s == NULL) {
			return 1;
		}
		xiplen = &appxip_len;

	} else if (!strcmp(mpi->name, "lgfont")) {
		s = getenv("fontxip");
		if (*s == 'n' || s == NULL) {
			return 1;
		}
		xiplen = &fontxip_len;
	}

	/* flash source address */
	src			= FLASH_BASE + mpi->offset;
	cp_pos		= CFG_LOAD_ADDR;
	tmpfilesize = FS_HDR_SIZE;

	ret		= nand_read(&nand_info[0], src, &tmpfilesize, (u_char *)cp_pos);
	if (ret) {
		printf("nand read failed..\n");
		return 1;
	}

	if (!lzo_compare_magic((unsigned char *)cp_pos))
		goto do_lzo;
	else
		goto do_cramfs;

do_lzo :
	s_msec	= get_cur_time();
	printf("[%4d] Copy \"%s (%ld bytes)\" from 0x%08x to 0x%08x\n", (int)s_msec, mpi->name, imgfilesize, (unsigned int)src, (unsigned int)cp_pos);

	tot_size	= 0;
	left		= imgfilesize;
	while (tot_size < imgfilesize)
	{
		rd_size	= (left < nand_info[0].erasesize)? left : nand_info[0].erasesize;
		ret		= nand_read(&nand_info[0], src+tot_size, &rd_size, (u_char *)(cp_pos+tot_size));
		if (ret) {
			printf("nand_read failed..(tot=%ld, rd_size=%ld)\n", tot_size, rd_size);
			return 1;
		}
		tot_size	+= rd_size;
		left		-= rd_size;

		BG_PowerSeqTask(BGTASK_CONT);
	}

	e_msec	= get_cur_time();
	printf("[%4d] ...done (%ld KB/s)\n", (int)e_msec, imgfilesize / (e_msec-s_msec));

	if (ret) {
		printf("nand read failed..\n");
		return 1;
	}

	decomp_size1 = lzo_get_destsize((unsigned char *)cp_pos);
	if (decomp_size1 == 0) {
		printf("decompressed size is zero at LZO header\n");
		return 1;
	}

	*xiplen		 = (decomp_size1 & 0xFFF00000) + 0x100000;
	xip_size	+= *xiplen;

	dst = CFG_SDRAM_BASE + ((gd->ram_size << 20) - xip_size);

	lzo_do_decomp((u_char *)dst, (u_char *)cp_pos, &decomp_size2);
	if (decomp_size2 != decomp_size1) {
		printf("decompressed size=%d is different with %d\n", (int)decomp_size2, (int)decomp_size1);
		return 1;
	}

	return 0;

do_cramfs :
	fshdr = (char *)cp_pos;

	if (!strncmp(&fshdr[16], CRMFS_HDR, 15)) {
		*xiplen = (imgfilesize & 0xFFF00000) + 0x100000;

		/* set xipfs_len in MB */
		xip_size += *xiplen;

		dst = CFG_SDRAM_BASE + ((gd->ram_size << 20) - xip_size);

		s_msec  = get_cur_time();
		printf("[%4d] Copy \"%s (%ld bytes)\" from 0x%08x to 0x%08x\n", (int)s_msec, mpi->name, imgfilesize, (unsigned int)src, (unsigned int)dst);

		tot_size	= 0;
		left		= imgfilesize;
		while (tot_size < imgfilesize)
		{
			rd_size	= (left < nand_info[0].erasesize)? left : nand_info[0].erasesize;
			ret		= nand_read(&nand_info[0], src+tot_size, &rd_size, (u_char *)(dst+tot_size));
			if (ret) {
				printf("nand_read failed..(tot=%ld, rd_size=%ld)\n", tot_size, rd_size);
				return 1;
			}
			tot_size	+= rd_size;
			left		-= rd_size;

			BG_PowerSeqTask(BGTASK_CONT);
		}

		e_msec  = get_cur_time();
		printf("[%4d] ...done (%ld KB/s)\n", (int)e_msec, imgfilesize / (e_msec-s_msec));

		if (ret) {
			printf("nand read failed..\n");
			return 1;
		}
	} else {
		printf("This is not xip image!\n");
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	xip,	2,	0,	do_xip,
	"xip\t- copy to ram for xip\n",
	"xip [lgapp | lgfont]\n"
);

int do_cp2ram (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char pname[MTD_STR_LEN];
	int	idx = 0, ret = 0;
	ulong src, dst, len = 0, addr = 0;
	ulong rd_size = 0;
	long  tot_size = 0, left = 0;
	ulong s_msec, e_msec;
	struct mtd_partition_info *mpi = NULL;

	if (argc == 3) {
		strcpy(pname, argv[1]);
		addr = simple_strtoul(argv[2], NULL, 16);
	}
	else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/* find partition */
	do {
		mpi = GET_PARTITION_INFO(idx);
		if(mpi->used && mpi->valid && (strcmp(pname, mpi->name) == 0))
		{
			len = mpi->filesize;
			break;
		}
	} while (++idx < MTD_PARTITION_MAX);

	if (idx == MTD_PARTITION_MAX) {
		printf("Unknown partition name(%s)..\n", pname);
		return 1;
	}

	if (len == 0) {
		printf("Partition Image Size Zero Length Case!! ==> Skipped!!\n");
		return 1;
	}

	src = (FLASH_BASE + mpi->offset);
	dst = addr;

	s_msec = get_cur_time();
	printf("[%4d] Copy \"%s (%ld bytes)\" from 0x%08x to 0x%08x\n", (int)s_msec, mpi->name, len, (unsigned int)src, (unsigned int)dst);

#ifdef MTDINFO_IN_FLASH
	memcpy((void *)dst, (void *)src, len);
#else
	tot_size	= 0;
	left		= len;
	while (tot_size < len)
	{
		rd_size	= (left < nand_info[0].erasesize)? left : nand_info[0].erasesize;
		ret		= nand_read(&nand_info[0], mpi->offset+tot_size, &rd_size, (u_char *)(dst+tot_size));
		if (ret) {
			printf("nand_read failed..(tot=%ld, rd_size=%ld)\n", tot_size, rd_size);
			return 1;
		}
		tot_size	+= rd_size;
		left		-= rd_size;

		BG_PowerSeqTask(BGTASK_CONT);
	}
#endif

	e_msec = get_cur_time();
	printf("[%4d] ...done (%ld KB/s)\n", (int)e_msec, len / (e_msec-s_msec));

	return 0;
}

/* ====================================================================== */
U_BOOT_CMD(
	cp2ram,	3,	0,	do_cp2ram,
	"cp2ram\t- copy to ram for partition name\n",
	"cp2ram [partition name] [dest address]\n"
);
