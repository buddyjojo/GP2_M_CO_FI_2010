#ifndef _NAND_BBM_H_
#define _NAND_BBM_H_

#include <config.h>
#ifdef CONFIG_MTD_NAND_BBM

#define NAND_BBM_DEBUG	(0)

#define bbm_debug(fmt, args...) 	\
	if (nand_bbm_debug)				printk("%s: " fmt, __FUNCTION__, ## args)

#define bbm_print(fmt, args...)		\
	if (nand_bbm_debug)				printk(fmt, ## args)

#define NAND_BBM_BAD_CHECK			(0)

#define BLOCK_INDEX(chip,pos)		(unsigned long)(pos>>chip->phys_erase_shift)
#define BLOCK_POS(chip,index)		(unsigned long)(index<<chip->phys_erase_shift)

#define PAGE_INDEX(chip,pos)		(unsigned long)(pos>>chip->page_shift)
#define PAGE_POS(chip,index)		(unsigned long)(index<<chip->page_shift)

#define BLK2PG_INDEX(chip,idx)		(unsigned long)(idx<<(chip->phys_erase_shift-chip->page_shift))
#define PG2BLK_INDEX(chip,idx)		(unsigned long)(idx>>(chip->phys_erase_shift-chip->page_shift))

#define BBM_INFO_DUP				(0)

#define BBM_INFO_POS				DEFAULT_BBMINFO_BASE
#define BBM_INFO_SIZE				DEFAULT_BBMINFO_SIZE

#define BBM_INFO_START				BBM_INFO_POS
#define BBM_INFO_END				(BBM_INFO_START+BBM_INFO_SIZE)

#define BBM_REDIR_POS				redirect_start
#define BBM_REDIR_SIZE				redirect_size
#define BBM_REDIR_START				(BBM_REDIR_POS)
#define BBM_REDIR_END				(BBM_REDIR_START+BBM_REDIR_SIZE)
#define BBM_REDIR_RATIO				3

#define BBM_DATA_POS				(BBM_INFO_END)
#define BBM_DATA_SIZE				(BBM_REDIR_START-BBM_DATA_POS)

#define BBM_MAGIC					(0x8192a5a6)
#define BBM_BB_MASK 				(0x80000000)
#define BBM_REDIR_MASK 				(0x40000000)
#define BBM_INFO_MASK 				(0x20000000)
#define BBM_IDX_MASK				(0x0fffffff)

enum {
	BBM_BLK_DATA = 0,
	BBM_BLK_INFO,
	BBM_BLK_REDIR
};

typedef struct bbm_blkinfo {
//	unsigned int type;
	unsigned long offset;
	unsigned long length;
} bbm_blkinfo_t;

struct bbm_blkmap {
	bbm_blkinfo_t blkinfo[3];
};

struct bbm_mapinfo {
	unsigned long bb_index;
	unsigned long reloc_index;
};

struct nand_bad_block_mng {
	unsigned long magic;
	unsigned long bb_count;
	unsigned long assign_index;
	struct bbm_mapinfo maps[];
};

extern struct bbm_blkmap nand_bbm_blkmap;

extern unsigned long * nand_bbm_bitmap;

extern unsigned int nand_bbm_check;

extern int nand_bbm_debug;

extern unsigned long assign_index;

extern unsigned long bb_count_data;

extern unsigned long bb_count_redirect;

extern int bbm_dirty_flag;

extern int nand_bbm_init(struct mtd_info * mtd);

extern int nand_bbm_update(struct mtd_info * mtd);

extern unsigned long nand_bbm_assign_block(struct mtd_info * mtd, unsigned long badpos, int blkcopy);

#define BBM_BLK_MAP(x)		&(nand_bbm_blkmap.blkinfo[x])

#define NAND_BBM_ADJUST(mtd,chip,ofs)													\
do {																			\
	if(nand_bbm_check) {														\
		if(nand_bbm_bitmap[ofs>>chip->phys_erase_shift] & BBM_BB_MASK) {		\
			bbm_debug("NAND_BBM_ADJUST(0x%08x)", (unsigned int)ofs);		\
			ofs = ((nand_bbm_bitmap[ofs>>chip->phys_erase_shift] & BBM_IDX_MASK)<<chip->phys_erase_shift) | (ofs&(mtd->erasesize-1));		\
			bbm_debug(" -> 0x%08x\n", (unsigned int)ofs);						\
		}																		\
	}																			\
} while (0)

#define NAND_BBM_ADJUST_CHECK(mtd,chip,ofs) 											\
do {																			\
	if(nand_bbm_check) {														\
		if(nand_bbm_bitmap[ofs>>chip->phys_erase_shift] & BBM_BB_MASK) {		\
			bbm_debug("NAND_BBM_ADJUST_CHECK(0x%08x)", (unsigned int)ofs);		\
			ofs = ((nand_bbm_bitmap[ofs>>chip->phys_erase_shift] & BBM_IDX_MASK)<<chip->phys_erase_shift) | (ofs&(mtd->erasesize-1));	\
			bbm_debug(" -> 0x%08x\n", (unsigned int)ofs);						\
		}																		\
		if(chip->block_bad(mtd, ofs, 0) != 0) { 						\
			bbm_debug("\t0x%08x is bad block => ", (unsigned int)ofs);			\
			ofs = (loff_t)nand_bbm_assign_block(mtd, (unsigned long)ofs, 0);		\
			bbm_debug("0x%08x\n", (unsigned int)ofs);							\
		}																		\
	}																			\
} while (0)

#define NAND_BBM_UPDATE(mtd)		nand_bbm_update(mtd);

#else

#define NAND_BBM_ADJUST(mtd,chip,ofs)

#define NAND_BBM_ADJUST_CHECK(mtd,chip,ofs)

#define NAND_BBM_UPDATE(mtd)

#endif /* CONFIG_MTD_NAND_BBM */

#endif /* _NAND_BBM_H_ */

