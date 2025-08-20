#ifndef __MTD_MTDINFO_H__
#define __MTD_MTDINFO_H__

#include <linux/types.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>

#define NAND_MTDINFO_OFFSET 	(0xC0000)
#define MTD_STR_LEN 			32
#define MTD_MAP_MAX 			4
#define MTD_PARTITION_MAX		32

struct mtd_map_info {
	char name[MTD_STR_LEN];
	unsigned int size;
	unsigned int phys;
	void *virt;
	void *cached;
	int bankwidth;
	unsigned int used;
};

struct mtd_partition_info {
	char name[MTD_STR_LEN]; 	/* identifier string */
	unsigned int offset;		/* offset within the master MTD space */
	unsigned int size;			/* partition size */
	char filename[MTD_STR_LEN]; /* file name */
	unsigned int filesize;		/* file size */
	unsigned int sw_ver;		/* software version */
	unsigned char used; 		/* Is this partition is used? */
	unsigned char valid;		/* Is this partition is valid? */
	unsigned char mask_flags;	/* master MTD flags to mask out for this partition */
};

struct mstar_mtd_info {
	unsigned int		magic;
	unsigned int		cur_epk_ver;
	unsigned int		old_epk_ver;
	unsigned char		nmap;
	unsigned char		npartition;
	struct mtd_map_info map[MTD_MAP_MAX];
	struct mtd_partition_info	partition[MTD_PARTITION_MAX];
} ;

extern struct mtd_info *nand_mtd;
extern unsigned int get_mtd_partitions(struct mtd_info *mtd); //redcloud00@lge.com mtd don't delete

#endif /* __MTD_MTD_H__ */
