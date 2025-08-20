#ifndef _MTD_INFO_H_
#define _MTD_INFO_H_

#ifndef NO
#define NO							0x00
#define YES							0x01
#endif

#undef BIG_ENDIAN
#ifdef BIG_ENDIAN
#define SWAP32(x)					((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8) | \
									((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24)
#else
#define SWAP32(x)					x
#endif

/* magic number */
#define CONFIG_MAGIC				0x20090729

/*-----------------------------------------------------------------------------
 * map info
 */
#define FLASH_BASE					0x00000000
#define FLASH_SIZE					(256*1024*1024)
#define FLASH_BANKWIDTH 			8

/*-----------------------------------------------------------------------------
 * mtd partition info
 */
#define MTD_FLG_FIXED				1
#define MTD_FLG_MASTER				2
#define MTD_FLG_IDKEY				4
#define MTD_FLG_CACHE				8

#define MTD_STR_LEN					32
#define MTD_MAP_MAX					4
#define MTD_PARTITION_MAX			32

typedef enum PART_INFO {
	PART_INFO_IDX	= 0,
	PART_INFO_OFFSET,
	PART_INFO_SIZE,
	PART_INFO_FILESIZE
} PART_INFO_T;

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
	char name[MTD_STR_LEN];				/* identifier string */
	unsigned int offset;				/* offset within the master MTD space */
	unsigned int size;					/* partition size */
	char filename[MTD_STR_LEN];			/* file name */
	unsigned int filesize;				/* file size */
	unsigned int sw_ver;				/* software version */
	unsigned char used;					/* Is this partition is used? */
	unsigned char valid;				/* Is this partition is valid? */
	unsigned char mask_flags;			/* master MTD flags to mask out for this partition */
};

struct boot_mtd_info {
	unsigned int					magic;
	unsigned int					cur_epk_ver;
	unsigned int					old_epk_ver;
	unsigned char					nmap;
	unsigned char 					npartition;
	struct mtd_map_info				map[MTD_MAP_MAX];
	struct mtd_partition_info		partition[MTD_PARTITION_MAX];
};

extern struct boot_mtd_info			mtdinfo;

#define GET_MTD_INFO()				((struct boot_mtd_info *)&(mtdinfo))
#define GET_MAP_INFO(x)				((struct mtd_map_info *)&(mtdinfo.map[x]))
#define GET_PARTITION_INFO(x)		((struct mtd_partition_info *)&(mtdinfo.partition[x]))
#define GET_PARTITION_NUM()			(mtdinfo.npartition)

//by dhjung
#define DEFAULT_BBMINFO_BASE		(FLASH_BASE)
#define DEFAULT_BBMINFO_SIZE		(0x20000)

#define DEFAULT_BOOT1___BASE		(DEFAULT_BBMINFO_BASE + DEFAULT_BBMINFO_SIZE)
#define DEFAULT_BOOT1___SIZE		(0x80000)

#define DEFAULT_MTDINFO_BASE		(DEFAULT_BOOT1___BASE + DEFAULT_BOOT1___SIZE + 0x20000) //0xa0000 ~ 0xc0000 : env area
#define DEFAULT_MTDINFO_SIZE		(0x20000)

#define DEFAULT_ROOTFS__BASE		(DEFAULT_MTDINFO_BASE + DEFAULT_MTDINFO_SIZE)
#define DEFAULT_ROOTFS__SIZE		(0x700000)

#define DEFAULT_LGINIT__BASE		(DEFAULT_ROOTFS__BASE + DEFAULT_ROOTFS__SIZE)
#define DEFAULT_LGINIT__SIZE		(0x60000)

#define DEFAULT_BOOT2___BASE		(DEFAULT_LGINIT__BASE + DEFAULT_LGINIT__SIZE)
#define DEFAULT_BOOT2___SIZE		(0x80000)



#define DEFAULT_MAP_0				{														\
	name		: "mstar_map0",																\
	size		: SWAP32(FLASH_SIZE),			phys		: SWAP32(FLASH_BASE),			\
	virt		: 0x0, 							cached		: 0x0,							\
	bankwidth	: SWAP32(FLASH_BANKWIDTH),		used		: SWAP32(YES)					\
}

#define DEFAULT_EMPTY_MAP			{														\
	name		: " ",																		\
	size		: 0x0,							phys		: 0x0,							\
	virt		: 0x0, 							cached		: 0x0,							\
	bankwidth	: 0x0,							used		: SWAP32(NO)					\
}

#define DEFAULT_MAP_INFO			{														\
	DEFAULT_MAP_0,																			\
	DEFAULT_EMPTY_MAP																		\
}

#define DEFAULT_PTT_BBMINFO			{														\
	name		: "bbminfo",																\
	offset		: SWAP32(DEFAULT_BBMINFO_BASE),	size		: SWAP32(DEFAULT_BBMINFO_SIZE), \
	filename	: "",							filesize	: 0x0,							\
	sw_ver		: 0x0,																		\
	used		: NO,							valid		: NO,							\
	mask_flags	: 0 																		\
}

#define DEFAULT_PTT_BOOT1			{														\
	name		: "boot",																	\
	offset		: SWAP32(DEFAULT_BOOT1___BASE),	size		: SWAP32(DEFAULT_BOOT1___SIZE),	\
	filename	: "u-boot-nand.bin",			filesize	: 0x0,							\
	sw_ver		: 0x0,																		\
	used		: YES,							valid		: YES,							\
	mask_flags	: 0 																		\
}

#define DEFAULT_PTT_BOOT2			{														\
	name		: "boot",																	\
	offset		: SWAP32(DEFAULT_BOOT2___BASE),	size		: SWAP32(DEFAULT_BOOT2___SIZE),	\
	filename	: "u-boot-nand.bin",			filesize	: 0x0,							\
	sw_ver		: 0x0,																		\
	used		: NO,							valid		: YES,							\
	mask_flags	: 0 																		\
}


#define DEFAULT_PTT_MTDINFO 		{														\
	name		: "mtdinfo",																\
	offset		: SWAP32(DEFAULT_MTDINFO_BASE),	size		: SWAP32(DEFAULT_MTDINFO_SIZE),	\
	filename	: "MTD.INFO",					filesize	: 0x0,							\
	sw_ver		: 0x0,																		\
	used		: YES,							valid		: YES,							\
	mask_flags	: 0 																		\
}

#define DEFAULT_PTT_ROOTFS			{														\
	name		: "rootfs", 																\
	offset		: SWAP32(DEFAULT_ROOTFS__BASE),	size		: SWAP32(DEFAULT_ROOTFS__SIZE),	\
	filename	: "rootfs.squashfs",			filesize	: 0x0,							\
	sw_ver		: 0x0,																		\
	used		: YES,							valid		: YES,							\
	mask_flags	: 0 															\
}

#define DEFAULT_PTT_LGINIT			{														\
	name		: "lginit", 																\
	offset		: SWAP32(DEFAULT_LGINIT__BASE),	size		: SWAP32(DEFAULT_LGINIT__SIZE),	\
	filename	: "lginit.squashfs",			filesize	: 0x0,							\
	sw_ver		: 0x0,																		\
	used		: YES,							valid		: YES,							\
	mask_flags	: 0															\
}

#define DEFAULT_PTT_NULL			{														\
	name		: "",																		\
	offset		: 0x0000000,					size		: 0x0,							\
	filename	: "",							filesize	: 0x0,							\
	sw_ver		: 0x0,																		\
	used		: NO,							valid		: NO,							\
	mask_flags	: 0 																		\
}

#define DEFAULT_PARTITION			{			\
	DEFAULT_PTT_BBMINFO,						\
	DEFAULT_PTT_BOOT1,							\
	DEFAULT_PTT_MTDINFO,						\
	DEFAULT_PTT_ROOTFS,							\
	DEFAULT_PTT_LGINIT,							\
	DEFAULT_PTT_BOOT2,							\
	DEFAULT_PTT_NULL							\
}

#define DEFAULT_NUM_OF_PARTITION				6

#define DEFAULT_MTD_INFO			{			\
	magic			: SWAP32(CONFIG_MAGIC),		\
	cur_epk_ver		: 0, 						\
	old_epk_ver		: 0, 						\
	nmap			: 1, 						\
	npartition		: DEFAULT_NUM_OF_PARTITION,	\
	map				: DEFAULT_MAP_INFO,			\
	partition 		: DEFAULT_PARTITION			\
}

extern struct mtd_partition_info * get_mtdpartinfo(int idx);
extern struct mtd_partition_info * get_partition(char *name);
extern struct mtd_partition_info * get_unused_partition(char *name);
extern unsigned int swap_partition(struct mtd_partition_info *mpi1, struct mtd_partition_info *mpi2);
extern unsigned int save_mtdinfo(void);
extern unsigned int load_mtdinfo(void);

extern int erase_cachepart(void);

#endif /* MTD_INFO_H_ */
