/*
 * nand_bbm.c
 *
 * Nand Bad Block Management by junorion
 *
 * Copyright (C)
 */

#include <common.h>
#ifdef CONFIG_MTD_NAND_BBM
#include <command.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>

#include <linux/mtd/nand.h>
#include <exports.h>

#include <nand.h>
#include <linux/mtd/nand_bbm.h>
#include <mtdinfo.h>

#define _UBOOT_
#ifdef _UBOOT_
#define GFP_KERNEL	0
#define ENOMEM		1
#define printk		printf

#define mutex_lock(mtx)
#define mutex_unlock(mtx)

#else
DEFINE_MUTEX(bbminfo_mutex);
#endif

struct nand_bad_block_mng * nand_bbm = NULL;
struct bbm_blkmap nand_bbm_blkmap;
unsigned long * nand_bbm_bitmap = NULL;
unsigned int nand_bbm_check = 0;
unsigned long assign_index;
unsigned long bb_count_data = 0;
unsigned long bb_count_redirect = 0;
unsigned long redirect_size;
unsigned long redirect_start;

int bbm_dirty_flag = 0;
int nand_bbm_debug;

int nand_bbm_init(struct mtd_info * nandmtd);
int nand_bbm_update(struct mtd_info * mtd);

static unsigned long bbm_pos = 0;
//static unsigned long bbm_backup_pos = 0;

static unsigned long
nand_bbm_find_pos(struct mtd_info * mtd)
{
#if (BBM_INFO_DUP==0)
	bbm_pos = BBM_INFO_POS;
#else
	unsigned long pos = 0;

	bbm_debug("nand_bbm_check = %d\n", nand_bbm_check);

	bbm_pos = bbm_backup_pos = 0;

	for (pos = BBM_INFO_START; pos < BBM_INFO_END; pos += mtd->erasesize) {
		if (mtd->block_isbad(mtd, pos) == 0) {
			bbm_pos = pos;
			break;
		}
	}

	if(pos == BBM_INFO_END) {
		bbm_print("Find bbm info failed... All block is bad block in BBM info region\n");
		return 0;
	}

	for (pos = (BBM_INFO_END-mtd->erasesize); pos > bbm_pos; pos -= mtd->erasesize) {
		if (mtd->block_isbad(mtd, pos) == 0) {
			bbm_backup_pos = pos;
			break;
		}
	}

	if(pos == bbm_pos) {
		bbm_print("Find bbm info failed... All block is bad block in BBM info region\n");
		return 0;
	}
#endif
	return 0;
}

#ifndef _UBOOT_
int nand_bbm_copy_block(struct mtd_info *mtd, unsigned long badpos, unsigned long redirpos)
{
	struct nand_chip * chip = mtd->priv;
	u_char *databuf, *buf;
	u_char *oobbuf = NULL;
	int ret = 0, datalen=0;
	uint32_t page;

	buf = (char *)kmalloc(mtd->erasesize, GFP_KERNEL);
	if(buf == NULL) {
		bbm_print("malloc failed\n");
		return -1;
	}

	//1. read block
	databuf = buf;
	datalen = 0;
	page = badpos >> chip->page_shift;
	while(datalen < mtd->erasesize) {
		ret = chip->read_page(mtd, databuf, oobbuf, page);
		if (ret) {
			printk(KERN_ERR "%s: chip->read_page at %08x failed ret=%d\n",
				__FUNCTION__, (unsigned int) page, ret);
			return ret;
		}
		page++;
		datalen += mtd->oobblock;
		databuf = &buf[datalen];
	}

	//2. write block
	databuf = buf;
	datalen = 0;
	page = redirpos >> chip->page_shift;
	while (datalen < mtd->erasesize) {
		ret = chip->write_page(mtd, databuf, oobbuf, page);
		if (ret) {
			printk(KERN_ERR "%s: chip->write_page at %08x failed ret=%d\n",
				__FUNCTION__, (unsigned int) page, ret);
			return ret;
		}
		page++;
		datalen += mtd->oobblock;
		databuf = &buf[datalen];
	}

	return 0;
}
#endif

unsigned long nand_bbm_assign_block(struct mtd_info * mtd, unsigned long badpos, int blkcopy)
{
	struct nand_chip * chip = mtd->priv;
	unsigned long pos = 0;
	int pagesize, blocksize;

	pagesize = mtd->oobblock;
	blocksize = mtd->erasesize;

	bbm_debug("badpos = 0x%08x, assign_index = %x\n", (u32)badpos, (u32)assign_index);
	bbm_debug("assign_index = %x, BLOCK_INDEX(BBM_REDIR_END) = %x\n", (u32)assign_index, (u32)BLOCK_INDEX(chip, BBM_REDIR_END));

	nand_bbm_check = 0;

	do {
		if(assign_index <= BLOCK_INDEX(chip, BBM_REDIR_START)) {
			bbm_print("There is no reserved block : plz check bbm info\n");
			return badpos;
		}

		bbm_debug("check bad block (index:%d, pos:%08x)\n", (u32)assign_index, (u32)BLOCK_POS(chip, assign_index));

		if(chip->block_bad(mtd, BLOCK_POS(chip, assign_index), 0) == 0) {
			nand_bbm_bitmap[BLOCK_INDEX(chip, badpos)] = (BBM_BB_MASK | assign_index) ;
			nand_bbm_bitmap[assign_index] |= BBM_REDIR_MASK ;
			pos = BLOCK_POS(chip, assign_index);
			assign_index--;
			bb_count_data++;
			bbm_dirty_flag++;
			break;
		}
		nand_bbm_bitmap[assign_index] |= BBM_BB_MASK ;
		bb_count_redirect++;
		assign_index--;
	} while(1);

	#if 0	/* in kernel */
	if(blkcopy) {
		// copy badpos -> redirectpos
		nand_bbm_copy_block(mtd, badpos, pos);
	}
	#endif

	nand_bbm_check = 1;

	bbm_debug("return pos(0x%08x)\n", (u32)pos);
	return pos;
}

static void nand_bbm_info_init(struct mtd_info * mtd)
{
//	struct nand_chip *chip = mtd->priv;
	unsigned long pos = 0;

	bbm_debug("Init nand bad block information...\n");

	if(!nand_bbm) {
		nand_bbm = (struct nand_bad_block_mng *)kmalloc(mtd->erasesize, GFP_KERNEL);
		if(!nand_bbm) {
			printf("nand bad block mng structure malloc failed..--;;\n");
			return ;
		}
	}

	mutex_lock(&bbminfo_mutex);

	assign_index = (BBM_REDIR_END/mtd->erasesize) - 1;
	bb_count_data = bb_count_redirect = 0;

	nand_bbm->magic = BBM_MAGIC;
	nand_bbm->bb_count = 0;
	nand_bbm->assign_index = assign_index;

	nand_bbm_bitmap = (unsigned long*)kmalloc(sizeof(unsigned long)*(mtd->size/mtd->erasesize), GFP_KERNEL);

	for(pos = 0; pos < mtd->size; pos += mtd->erasesize) {
		nand_bbm_bitmap[pos/mtd->erasesize] = pos/mtd->erasesize;
	}

	mutex_unlock(&bbminfo_mutex);
}

static int nand_bbm_info_load(struct mtd_info *mtd)
{
	struct nand_chip *chip = (struct nand_chip*)mtd->priv;
	unsigned long pos = 0, bb_index, reloc_index;
	unsigned long rindex;
	unsigned long pagesize, blocksize;
	size_t total;
	int ret = 0, i;
	#if BBM_INFO_DUP
	int check = 0;
	#endif

	pagesize = mtd->oobblock;
	blocksize = mtd->erasesize;

	if(!nand_bbm) {
		nand_bbm = (struct nand_bad_block_mng *)kmalloc(blocksize, GFP_KERNEL);
		if(nand_bbm == NULL) {
			bbm_print("malloc failed\n");
			return 0;
		}
	}

	memset(nand_bbm, 0, blocksize);

	nand_bbm_find_pos(mtd);

	if(bbm_pos)
		pos = bbm_pos;

#if BBM_INFO_DUP
load_bbm_info:
#endif
	/* load bad block info */
	bbm_debug ("Loading nand bad block info(%p+%p)...\n", (void*)pos, (void*)blocksize);

	ret = mtd->read(mtd, pos, blocksize, &total, (u_char *)nand_bbm);
	if(ret) {
		bbm_print("nand read failed..--;;\n");
		return ret;
	}

	mutex_lock(&bbminfo_mutex);

	bbm_debug("bbm->magic = %x\n", nand_bbm->magic);
	if(nand_bbm->magic == BBM_MAGIC) {

		if(!nand_bbm_bitmap)
			nand_bbm_bitmap = (unsigned long *)kmalloc(sizeof(unsigned long)*(mtd->size/blocksize), GFP_KERNEL);

		for(pos = 0; pos < mtd->size; pos += blocksize) {
			nand_bbm_bitmap[pos/blocksize] = pos/blocksize;
		}

		bb_count_data = bb_count_redirect;
		rindex = (BBM_REDIR_END/blocksize) - 1;
		i = 0;
		for(pos = BBM_REDIR_START; pos < BBM_REDIR_END; pos += blocksize) {

			bbm_debug("%s] bb_count (%d, %d)\n", __func__, bb_count_data, bb_count_redirect);
			if((bb_count_data+bb_count_redirect) == nand_bbm->bb_count)
				break;

			bbm_debug("%s] [%d] maps(%x, %x)\n", __func__, i,
					(u32)nand_bbm->maps[i].bb_index, (u32)nand_bbm->maps[i].reloc_index);

			bb_index = nand_bbm->maps[i].bb_index;
			reloc_index = nand_bbm->maps[i].reloc_index;

			while(reloc_index != rindex) {		// for gang
				nand_bbm_bitmap[rindex] = BBM_BB_MASK | rindex;
				
				bbm_debug("bb_count_redirect : BLOCK_POS(rindex) = %x\n", BLOCK_POS(chip, rindex));
				bb_count_redirect++;
				rindex--;
			}

			nand_bbm_bitmap[bb_index] = BBM_BB_MASK | reloc_index;
			nand_bbm_bitmap[reloc_index] |= BBM_REDIR_MASK;

			//rindex = min(rindex, nand_bbm->maps[i].reloc_index);

			if(BLOCK_POS(chip, bb_index) != BLOCK_POS(chip, reloc_index)) {
				bbm_debug("bb_count_data : BLOCK_POS(chip, bb_index) = %x, BLOCK_POS(chip, reloc_index) = %x\n", BLOCK_POS(chip, bb_index), BLOCK_POS(chip, reloc_index));
				bb_count_data++;
			}else{
				bbm_debug("bb_count_redirect : BLOCK_POS(chip, bb_index) = %x, BLOCK_POS(chip, reloc_index) = %x\n", BLOCK_POS(chip, bb_index), BLOCK_POS(chip, reloc_index));
				bb_count_redirect++;
			}

			rindex--;
			i++;

			bbm_debug("\t\t rindex = %x\n", (u32)rindex);
		}

		assign_index = (rindex);
		bbm_debug("rindex = %x, assign_index = %x\n", rindex, assign_index);
		if(assign_index != nand_bbm->assign_index) {
			bbm_print("Somthing wrong ...!!! assign_index(%x) is unmatched\n", nand_bbm->assign_index);
			goto not_ok;
		}

		goto load_ok;
	}
	else
	{
		printk("ERROR :: %s: Invalid bbm magic %lx should be %x\n", __func__, nand_bbm->magic, BBM_MAGIC);
	}

	#if BBM_INFO_DUP
	if(check++)
		goto not_ok;

	if(bbm_backup_pos) {
		pos = bbm_backup_pos;
		goto load_bbm_info;
	}
	#endif
not_ok:
	mutex_unlock(&bbminfo_mutex);
	bbm_debug ("%s: Load backup bbm info failed...\n", __func__);
	return (1);

load_ok:
	mutex_unlock(&bbminfo_mutex);
	bbm_debug ("%s: Loading nand bad block info...ok\n", __func__);
	return 0;
}

static int nand_bbm_info_save(struct mtd_info * mtd)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	unsigned long pos = 0, len = 0;
	unsigned long pagesize, blocksize;
	struct erase_info instr;
	int ret=0, i;
	int index = 0;
	size_t retlen=0;
	u_char * kbuf, *src;

	pagesize = mtd->oobblock;
	blocksize = mtd->erasesize;

	memset(nand_bbm, 0, blocksize);

	mutex_lock(&bbminfo_mutex);

	nand_bbm->magic = BBM_MAGIC;
	nand_bbm->bb_count = (bb_count_data + bb_count_redirect);
	nand_bbm->assign_index = assign_index;

	index = 0;
#if 0
	for(pos = 0; pos < mtd->size; pos += blocksize) {
		if(nand_bbm_bitmap[BLOCK_INDEX(pos)] & BBM_BB_MASK) {
			nand_bbm->maps[index].bb_index = (unsigned long)(BLOCK_INDEX(pos));
			nand_bbm->maps[index].reloc_index = (unsigned long)(nand_bbm_bitmap[BLOCK_INDEX(pos)] & BBM_IDX_MASK);
			bbm_debug(" %08x -> %08x\n", (u32)nand_bbm->maps[index].bb_index, (u32)nand_bbm->maps[index].reloc_index);
			index++;
		}
	}
#else
	for(i=BBM_BLK_DATA; i<=BBM_BLK_REDIR; i++) {
		bbm_blkinfo_t * blkinfo = BBM_BLK_MAP(i);
		unsigned long start, end;
		start = blkinfo->offset;
		end = start + blkinfo->length;
		for(pos = start; pos < end; pos += blocksize) {
			if(nand_bbm_bitmap[BLOCK_INDEX(chip,pos)] & BBM_BB_MASK) {
				nand_bbm->maps[index].bb_index = (unsigned long)(BLOCK_INDEX(chip,pos));
				nand_bbm->maps[index].reloc_index = (unsigned long)(nand_bbm_bitmap[BLOCK_INDEX(chip,pos)] & BBM_IDX_MASK);
				bbm_debug("%08x -> %08x\n", (u32)nand_bbm->maps[index].bb_index, (u32)nand_bbm->maps[index].reloc_index);
				index++;
			}
		}
	}
#endif

	mutex_unlock(&bbminfo_mutex);

	memset((void*)&instr, 0, sizeof(struct erase_info));
	instr.mtd = mtd;

	kbuf=kmalloc(pagesize, GFP_KERNEL);
	if (!kbuf) {
		bbm_print("malloc is null\n");
		return -ENOMEM;
	}

	if(nand_bbm_find_pos(mtd)) {
		printk("find bbm pos error --;\n");
		return (-1);
	}

	pos = bbm_pos;

#if BBM_INFO_DUP
save_bbm_info:
#endif
	/* save bbm info */
	bbm_debug("save bbm info\n");

	// erase
	bbm_debug("save bbm info >> erase (%p)\n", (void*)pos);
	instr.addr = pos;
	instr.len = blocksize;
	if (mtd->erase(mtd, &instr)) {
		bbm_debug("mtd->erase error.. --;;\n");
		//goto bbm_save_backup;
		return (-1);
	}

	// write
	bbm_debug("save bbm info >> write\n");
	len = blocksize;
	src = (u_char*)(nand_bbm);
	while(len) {
		memset(kbuf, 0, pagesize);
		memcpy(kbuf, src, pagesize);
		ret = mtd->write(mtd, pos, pagesize, &retlen, kbuf);
		if(ret) {
			bbm_debug("nand write error.. --;;\n");
			//goto bbm_save_backup;
			return (-1);
		}
		pos += pagesize;
		src += pagesize;
		len -= (len > pagesize) ? pagesize: len;
	}

	#if BBM_INFO_DUP
	if(check++)
		return 0;

	if(bbm_backup_pos) {
		pos = bbm_backup_pos;
		goto save_bbm_info;
	}
	#endif

	return 0;
}

static int nand_bbm_info_setup(struct mtd_info * mtd)
{
	unsigned long ofs;
	unsigned long blocksize = mtd->erasesize;
	int i;

	mutex_lock(&bbminfo_mutex);

	for(i=BBM_BLK_DATA; i<=BBM_BLK_REDIR; i++) {
		bbm_blkinfo_t * blkinfo = BBM_BLK_MAP(i);
		unsigned long start, end;
		start = blkinfo->offset;
		end = start + blkinfo->length;
		for(ofs = start; ofs < end; ofs += blocksize) {
			unsigned long to;
			if(mtd->block_isbad(mtd, ofs)) {
				bbm_print("\t0x%08x is bad block => ", (unsigned int)ofs);
				to = (loff_t)nand_bbm_assign_block(mtd, (unsigned long)ofs, 0);
				bbm_print("0x%08x\n", (unsigned int)to);
			}
		}
	}

	mutex_unlock(&bbminfo_mutex);

	return 0;
}

const char *bbm_blkmap_str[] = {
	"data block ",
	"info block ",
	"redir block"
};

static void nand_bbm_blkmap_print(void)
{
	bbm_blkinfo_t * blkinfo;
	int i;
	bbm_print("block map : \n");
	for(i=BBM_BLK_DATA; i<=BBM_BLK_REDIR; i++) {
		blkinfo = BBM_BLK_MAP(i);
//		blkinfo = nand_bbm_blkmap.blkinfo[i]
		bbm_print("\t %s : ", bbm_blkmap_str[i]);
		bbm_print("0x%08x (+0x%08x)\n", blkinfo->offset, blkinfo->length);
	}
}

static void nand_bbm_print(struct mtd_info * mtd)
{
	unsigned long bb_count, index = 0;
	unsigned int block_count = 0;

	mutex_lock(&bbminfo_mutex);

	block_count = (unsigned int)(mtd->size/mtd->erasesize);
	bb_count = bb_count_data + bb_count_redirect;

	printk("nand bad block information:\n");
	nand_bbm_blkmap_print();
	printk("magic = 0x%08x\n", nand_bbm->magic);
	printk("bad block count = %d(%d+%d)\n", bb_count, bb_count_data, bb_count_redirect);
	printk("assign index = %d\n", assign_index);

	for (index = 0; index < bb_count_data; index++) {
		printk(" %08x -> %08x\n", (u32)nand_bbm->maps[index].bb_index, (u32)nand_bbm->maps[index].reloc_index);
	}
#if 1
	for (index = 0; index < block_count; index++) {
		if((index%32)==0)
			printk("[%04x~%04x] ", index, index+31);

		if(nand_bbm_bitmap[index] & BBM_BB_MASK)
			printk("B ");
		else if(nand_bbm_bitmap[index] & BBM_REDIR_MASK)
			printk("R ");
		else
			printk(". ");

		if(((index+1)/32 > 0) && (((index+1)%32)==0))
			printk("\n");
	}
#endif

	mutex_unlock(&bbminfo_mutex);

}

static void nand_bbm_bad_check(struct mtd_info *mtd)
{
#if	(NAND_BBM_BAD_CHECK)
	unsigned long from, badcount = 0;

	for (from = 0; from < mtd->size; from += mtd->erasesize) {
		if(mtd->block_isbad(mtd, from)) {
			badcount++;
			printk("\t %p = bad block (%ld)\n", (void*)from, badcount);
		}
	}
	printk("total bad count = %ld\n", badcount);
#endif
}

#ifdef _UBOOT_
static int nand_bbm_info_bad(struct mtd_info * mtd)
{
	unsigned long from;
	unsigned long badcount = 0;
	size_t readlen, ooblen;
	struct nand_chip *this = mtd->priv;

	readlen	= mtd->oobblock;
	ooblen	= mtd->oobsize;
	printk("oobblock = %x, oobsize = %x, badblockpos = %x\n",
		readlen, ooblen, this->badblockpos);

	for (from = 0; from < mtd->size; from += mtd->erasesize) {
		if (this->block_bad(mtd, from, 1))
			printf("0x%8.8lx is bad (%d)\n", from, ++badcount);
	}
	printf("bad count = %d\n", badcount);
	return 0;
}

int do_nand_bbm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern nand_info_t nand_info[CFG_MAX_NAND_DEVICE];

	if(argc == 1) {
		nand_bbm_print(&nand_info[0]);
		return 0;
	}

	nand_bbm_check = 0;

	if(argc == 2) {

		if(strcmp(argv[1], "init")==0)
			nand_bbm_info_init(&nand_info[0]);
		else if(strcmp(argv[1], "load")==0)
			nand_bbm_info_load(nand_info+0);
		else if(strcmp(argv[1], "save")==0)
			nand_bbm_info_save(&nand_info[0]);
		else if(strcmp(argv[1], "bad")==0)
			nand_bbm_info_bad(&nand_info[0]);
		else if(strcmp(argv[1], "setup")==0) {
			nand_bbm_info_init(&nand_info[0]);
			nand_bbm_info_setup(&nand_info[0]);
			nand_bbm_info_save(&nand_info[0]);
		}
		else
			nand_bbm_print(&nand_info[0]);
		return 0;
	}

	if((argc == 3) && strcmp(argv[1], "debug")==0) {
		nand_bbm_debug = (int)simple_strtoul(argv[2], NULL, 10);
	}

	nand_bbm_check = 1;

	printf("Usage:\n%s\n", cmdtp->usage);
	return 0;
}

/* ====================================================================== */
U_BOOT_CMD(
	bbm, 2,	0,	do_nand_bbm,
	"bbm\t- nand bad block management\n",
	"bbm init/load/save/print \\n"
);

#else	/* in kernel */
#ifdef CONFIG_PROC_FS

static struct proc_dir_entry *proc_bbminfo;
static struct mtd_info *_proc_mtd;

static int bbminfo_read_proc (char *page, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	unsigned int bb_count, index = 0;
	int len = 0;
	unsigned int blk_count = 0;

	mutex_lock(&bbminfo_mutex);

	bb_count = bb_count_data + bb_count_redirect;
	blk_count = (unsigned int)(_proc_mtd->size/_proc_mtd->erasesize);

	len += sprintf(page+len, "nand bad block information:\n");
	len += sprintf(page+len, "bad block count = %d(%ld,%ld)\n", bb_count, bb_count_data, bb_count_redirect);
	len += sprintf(page+len, "assign index = %ld\n", assign_index);

	for (index = 0; index < bb_count_data; index++) {
		len += sprintf(page+len, " %08x -> %08x\n", (u32)nand_bbm->maps[index].bb_index, (u32)nand_bbm->maps[index].reloc_index);
	}

	for (index = 0; index < blk_count; index++) {
		if((index%32)==0)
			len += sprintf(page+len, "[%04x~%04x] ", index, index+31);

		if(nand_bbm_bitmap[index] & BBM_BB_MASK)
			len += sprintf(page+len, "B ");
		else if(nand_bbm_bitmap[index] & BBM_REDIR_MASK)
			len += sprintf(page+len, "R ");
		else
			len += sprintf(page+len, ". ");

		if(((index+1)/32 > 0) && (((index+1)%32)==0))
			len += sprintf(page+len, "\n");
	}

	mutex_unlock(&bbminfo_mutex);

	*eof = 1;

	return len;
}

static int bbminfo_write_proc(struct file *file, const char __user *buffer,
		    unsigned long count, void *data)
{
	char c;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;

	nand_bbm_check = 0;

	switch (c) {
	case 'i' :
		nand_bbm_info_init(_proc_mtd);
		break;
	case 'l' :
		nand_bbm_info_load(_proc_mtd);
		break;
	case 's' :
		nand_bbm_info_save(_proc_mtd);
		break;
	case 'c' :
		nand_bbm_info_init(_proc_mtd);
		nand_bbm_info_setup(_proc_mtd);
		nand_bbm_info_save(_proc_mtd);
	default:
		break;
	}

	nand_bbm_check = 1;

	return count;
}

static int proc_bbminfo_init(struct mtd_info *mtd)
{
	_proc_mtd = mtd;

	if ((proc_bbminfo = create_proc_entry( "bbminfo", 0, NULL ))) {
		proc_bbminfo->read_proc = bbminfo_read_proc;
		proc_bbminfo->write_proc = bbminfo_write_proc;
	}

	return 0;
}

static void proc_bbminfo_exit(void)
{
	if (proc_bbminfo)
	remove_proc_entry("bbminfo", NULL);
}

#endif /* CONFIG_PROC_FS */

#endif

int nand_bbm_init(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd->priv;

	nand_bbm_debug = NAND_BBM_DEBUG;

	bbm_debug("Init nand bad block information...\n");

	redirect_size	= (((this->chipsize * BBM_REDIR_RATIO) / 100) & 0xFFF00000) + 0x100000;
	redirect_start	= this->chipsize - redirect_size;
	bbm_debug("redirect_size = %x, redirect_start = %x\n", redirect_size, redirect_start);

	// init block map
	nand_bbm_blkmap.blkinfo[0].offset = BBM_DATA_POS;
	nand_bbm_blkmap.blkinfo[0].length = BBM_DATA_SIZE;
	nand_bbm_blkmap.blkinfo[1].offset = BBM_INFO_POS;
	nand_bbm_blkmap.blkinfo[1].length = BBM_INFO_SIZE;
	nand_bbm_blkmap.blkinfo[2].offset = BBM_REDIR_POS;
	nand_bbm_blkmap.blkinfo[2].length = BBM_REDIR_SIZE;
	nand_bbm_blkmap_print();

	assign_index = (BBM_REDIR_END / mtd->erasesize) - 1;

	nand_bbm_check = 0;

	nand_bbm_bad_check(mtd);

	if(nand_bbm_info_load(mtd)) {
		nand_bbm_info_init(mtd);
		#ifndef _UBOOT_
		printk(KERN_ERR "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		printk(KERN_ERR "ERROR :: Failed to load bbminfo. Check bbminfo structure!.\n");
		printk(KERN_ERR "         Here initialize bbminfo but don't save it.\n");
		printk(KERN_ERR "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
		#else
		nand_bbm_info_save(mtd);
		#endif
	}

	nand_bbm_check = 1;

	#ifndef _UBOOT_
	proc_bbminfo_init(mtd);
	#endif

	bbm_debug("End bbm init\n");

	return 0;
}

int nand_bbm_update(struct mtd_info * mtd)
{
	if(bbm_dirty_flag) {
		bbm_debug("bbm dirty : saveinfo\n");

		nand_bbm_check = 0;
		nand_bbm_info_save(mtd);
		nand_bbm_check = 1;

		bbm_dirty_flag = 0;
	}

	return 0;
}
#endif
