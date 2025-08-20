/*
 * nand_bbm.c
 *
 * Nand Bad Block Management by junorion
 *
 * Copyright (C)
 */

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/proc_fs.h>

#include <linux/mtd/nand_bbm.h>

DEFINE_MUTEX(bbminfo_mutex);

struct nand_bad_block_mng * nand_bbm = NULL;
unsigned long * nand_bbm_bitmap = NULL;
unsigned int nand_bbm_check = 0;
unsigned long assign_index;
unsigned long bb_count_data = 0;
unsigned long bb_count_redirect = 0;
unsigned long redirect_size;
unsigned long redirect_start;

int bbm_dirty_flag = 0;

int nand_bbm_init(struct mtd_info * nandmtd);
int nand_bbm_update(struct mtd_info * mtd);

static unsigned long bbm_pos = 0;
//static unsigned long bbm_backup_pos = 0;

static unsigned long
nand_bbm_find_pos(struct mtd_info * mtd)
{
#if (BBM_INFO_DUP == 0)
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

static uint8_t *nand_bbm_fill_oob(struct mtd_info *mtd, struct nand_chip *chip, uint8_t *oob)
{	
	struct nand_oobfree *free = chip->ecc.layout->oobfree;
	uint8_t *oob_poi = chip->oob_poi;
	int i, totlen;

	for(i=0, totlen = 0; totlen < chip->ecc.layout->oobavail; i++) {
		memcpy(&oob_poi[free->offset], &oob[free->offset], free->length);
		totlen += free->length;
		free++;
	}

	return NULL;
}

static int nand_bbm_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	int status = 0;
	const uint8_t *buf = chip->oob_poi;
	int length = mtd->oobsize;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);
	chip->write_buf(mtd, buf, length);
	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

static int nand_bbm_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			   const uint8_t *buf, int page, int cached, int raw)
{
	int status;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);
	chip->ecc.write_page(mtd, chip, buf);
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);
	/*
	 * See if operation failed and additional status checks are
	 * available
	 */
	if ((status & NAND_STATUS_FAIL) && (chip->errstat))
		status = chip->errstat(mtd, chip, FL_WRITING, status,
				       page);

	if (status & NAND_STATUS_FAIL)
		return -EIO;

	return 0;
}

static int nand_bbm_check_empty(uint8_t *buf, int len)
{
	unsigned int *tbuf = (unsigned int*)buf;
	int i, length = len / sizeof(unsigned int);
	for(i=0; i<length; i++)
		if(tbuf[i] != 0xffffffff)
			return 0;
//	bbm_debug("%s : Empty (%p)\n", __func__, (void*)buf);
	return 1;
}

int nand_bbm_erase_block(struct mtd_info *mtd, unsigned long pos)
{
	struct nand_chip * chip = mtd->priv;
	int page, status;
/**
	// 1. check pos block
	bbm_print("Check pos block (pos = %p)\n", (void*)pos);
	if(chip->block_bad(mtd, pos, 0)) {
		return NAND_STATUS_FAIL;
	}
**/
	// 2. erase pos block
	bbm_print("Erase pos block (pos = %p)\n", (void*)pos);
	page = (int)(pos >> chip->page_shift);

	chip->erase_cmd(mtd, page & chip->pagemask);
	status = chip->waitfunc(mtd, chip);
	if ((status & NAND_STATUS_FAIL) && (chip->errstat))
		status = chip->errstat(mtd, chip, FL_ERASING, status, page);

	/* See if block erase succeeded */
	if (status & NAND_STATUS_FAIL) {
		bbm_print("nand_erase: Failed erase, page 0x%08x\n", page);
		return NAND_STATUS_FAIL;
	}
	
	return 0;
}

int nand_bbm_copy_block(struct mtd_info *mtd, unsigned long badpos, unsigned long redirpos)
{
	struct nand_chip * chip = mtd->priv;
	u_char *buf, *datbuf, *oobbuf;
	u_char *data, *oob;
	int i, page, pages_per_block, ret=0;

	bbm_debug("badpos = %p, redirpos = %p\n", (void*)badpos, (void*)redirpos);

	pages_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);

	buf = (char *)kmalloc(pages_per_block * (mtd->writesize+mtd->oobsize), GFP_KERNEL);
	if(buf == NULL) {
		bbm_print("malloc failed\n");
		return -1;
	}

	memset(buf, 0x00, pages_per_block * (mtd->writesize+mtd->oobsize));
	
	datbuf = &buf[0];
	oobbuf = &buf[mtd->erasesize];
	printk("buf : datbuf = %p, oobbuf = %p\n", datbuf, oobbuf);
	
	// 1. dump badpos block (data + oob)
	bbm_print("Dump badpos block (badpos = %p)\n", (void*)badpos);
	page = (int)(badpos >> chip->page_shift);
	#if 1
	for(i=0; i<pages_per_block; i++) {
		// read data
		data = &datbuf[mtd->writesize * i];
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page+i);
		chip->read_buf(mtd, data, mtd->writesize);
		// read oob
		oob = &oobbuf[mtd->oobsize * i];
		chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page+i);
		chip->read_buf(mtd, oob, mtd->oobsize);
		bbm_print("#");
	}
	#else
	for(i=0; i<pages_per_block; i++) {
		data = &datbuf[mtd->writesize * i];
		// read data
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page+i);
		chip->read_buf(mtd, data, mtd->writesize);
		printk("[%x] data = %p\n", page+i, data);
	}
	for(i=0; i<pages_per_block; i++) {	
		oob = &oobbuf[mtd->oobsize * i];
		chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page+i);
		chip->read_buf(mtd, oob, mtd->oobsize);
		printk("[%x] oob = %p\n", page+i, oob);
	}	
	#endif
	// 2. Copy block from badpos to redirpos
	bbm_print("\nCopy block from badpos(%p) to redirpos (%p)\n", (void*)badpos, (void*)redirpos);
	#if 1
	page = (int)(redirpos >> chip->page_shift);
	for(i=0; i<pages_per_block; i++) {
		// write oob
		oob = &oobbuf[mtd->oobsize*i];
		if(!nand_bbm_check_empty(oob, mtd->oobsize)) {
			chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
			memset(chip->oob_poi, 0xff, mtd->oobsize);
			nand_bbm_fill_oob(mtd, chip, oob);
			nand_bbm_write_oob(mtd, chip, page+i);
		}
		
		// write data
		data = &datbuf[mtd->writesize*i];
		if(!nand_bbm_check_empty(data, mtd->writesize)) {
			memset(chip->oob_poi, 0xff, mtd->oobsize);
			ret = nand_bbm_write_page(mtd, chip, data, page+i, 0, 0);
			if (ret) {
				printk(KERN_ERR "%s: chip->write_page at %08x failed ret=%d\n",
					__FUNCTION__, (unsigned int) page, ret);
				goto out;
			}
		}	
		bbm_print("#");
	}
	#else
	// 3.1 write page oob
	page = (int)(redirpos >> chip->page_shift);
	bbm_print("write oob  : \n");
	for(i=0; i<pages_per_block; i++) {
//		chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
		oob = &oobbuf[mtd->oobsize*i];
		printk("[%x] oob = %p\n", page+i, oob);
		if(!nand_bbm_check_empty(oob, mtd->oobsize)) {
			memset(chip->oob_poi, 0xff, mtd->oobsize);
			nand_bbm_fill_oob(mtd, chip, oob);
			nand_bbm_write_oob(mtd, chip, page+i);
		}			
		//bbm_print("#");
	}
	
	// 3.2 write pdage data
	bbm_print("\nwrite data : ");	
	for(i=0; i<pages_per_block; i++) {
		data = &datbuf[mtd->writesize*i];
		printk("[%x] data = %p\n", page+i, data);
		if(!nand_bbm_check_empty(data, mtd->writesize)) {
			memset(chip->oob_poi, 0xff, mtd->oobsize);
			ret = nand_bbm_write_page(mtd, chip, data, page+i, 0, 0);
			if (ret) {
				printk(KERN_ERR "%s: chip->write_page at %08x failed ret=%d\n",
					__FUNCTION__, (unsigned int) page+i, ret);
				goto out;
			}
		}
		//bbm_print("#");
	}
	#endif
	bbm_print("\n");
out:
	kfree(buf);
	return ret;
}

unsigned long nand_bbm_assign_block(struct mtd_info * mtd, unsigned long badpos, int blkcopy)
{
	struct nand_chip * chip = mtd->priv;
	unsigned long pos = 0;
	int pagesize, blocksize;

	pagesize = mtd->writesize;
	blocksize = mtd->erasesize;
	
	bbm_debug("badpos = 0x%08x, assign_index = %x\n", (u32)badpos, (u32)assign_index);
	bbm_debug("assign_index = %x, BLOCK_INDEX(BBM_REDIR_END) = %x\n", (u32)assign_index, (u32)BLOCK_INDEX(chip,BBM_REDIR_END));

	nand_bbm_check = 0;

	do {
		if(assign_index <= BLOCK_INDEX(chip, BBM_REDIR_START)) {
			bbm_print("There is no reserved block : plz check bbm info\n");
			return badpos;
		}

		bbm_debug("check bad block (index:%d, pos:%08x)\n", (u32)assign_index, (u32)BLOCK_POS(chip, assign_index));

		if(chip->block_bad(mtd, BLOCK_POS(chip, assign_index), 0) == 0) {
//		if(nand_bbm_erase_block(mtd, BLOCK_POS(chip, assign_index)) == 0) {
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

	if(blkcopy) {
		// copy badpos -> redirectpos
		badpos = BLOCK_POS(chip, BLOCK_INDEX(chip, badpos));
		nand_bbm_erase_block(mtd, pos);
		nand_bbm_copy_block(mtd, badpos, pos);
	}
	
	nand_bbm_check = 1;

	bbm_debug("return pos(0x%08x)\n", (u32)pos);
	return pos;
}

static void nand_bbm_info_init(struct mtd_info * mtd)
{
//	struct nand_chip *this = mtd->priv;
	unsigned long pos = 0;

	bbm_print("Init nand bad block information...\n");

	if(!nand_bbm) {
		nand_bbm = (struct nand_bad_block_mng *)kmalloc(mtd->erasesize, GFP_KERNEL);
		if(!nand_bbm) {
			printk("nand bad block mng structure malloc failed..--;;\n");
			return ;
		}
	}

	mutex_lock(&bbminfo_mutex);

	assign_index = (BBM_REDIR_END / mtd->erasesize) - 1;
	bb_count_data = bb_count_redirect = 0;

	nand_bbm->magic = BBM_MAGIC;
	nand_bbm->bb_count = 0;
	nand_bbm->assign_index = assign_index;

	if(!nand_bbm_bitmap) {
		nand_bbm_bitmap = (unsigned long*)kmalloc(sizeof(unsigned long)*(mtd->size/mtd->erasesize), GFP_KERNEL);
		if(!nand_bbm_bitmap) {
			printk("nand bad block mng structure malloc failed..--;;\n");
			return ;
		}
	}

	for(pos = 0; pos < mtd->size; pos += mtd->erasesize) {
		nand_bbm_bitmap[pos/mtd->erasesize] = pos/mtd->erasesize;
	}

	mutex_unlock(&bbminfo_mutex);
}

static int nand_bbm_info_load(struct mtd_info *mtd)
{
	struct nand_chip *chip = (struct nand_chip*)mtd->priv;
	unsigned long pos = 0, bb_index, reloc_index;
	unsigned long rindex;	//, first_rindex;
	unsigned long pagesize, blocksize;
	size_t total;
	int ret = 0, i;
	#if BBM_INFO_DUP
	int check = 0;
	#endif

	pagesize = mtd->writesize;
	blocksize = mtd->erasesize;

	if(!nand_bbm) {
		nand_bbm = (struct nand_bad_block_mng *)kmalloc(blocksize, GFP_KERNEL);
		if(nand_bbm == NULL) {
			bbm_print("malloc failed\n");
			return 0;
		}
	}

	memset(nand_bbm, 0, blocksize);

	if(nand_bbm_find_pos(mtd)) {
		printk("find bbm pos error --;\n");
		return (-1);
	}

	pos = bbm_pos;

#if BBM_INFO_DUP
load_bbm_info:
#endif
	/* load bad block info */
	bbm_print ("Loading nand bad block info(%p+%p)...\n", (void*)pos, (void*)blocksize);

	ret = mtd->read(mtd, pos, blocksize, &total, (u_char *)nand_bbm);
	if(ret) {
		bbm_print("nand read failed..--;;\n");
		return ret;
	}

	mutex_lock(&bbminfo_mutex);

	bbm_debug("bbm->magic = %lx\n", nand_bbm->magic);
	if(nand_bbm->magic == BBM_MAGIC) {

		if(!nand_bbm_bitmap)
			nand_bbm_bitmap = (unsigned long *)kmalloc(sizeof(unsigned long)*(mtd->size/blocksize), GFP_KERNEL);

		for(pos = 0; pos < mtd->size; pos += blocksize) {
			nand_bbm_bitmap[pos/blocksize] = pos/blocksize;
		}

		bb_count_data = bb_count_redirect;
		rindex = (BBM_REDIR_END/blocksize) -1;
		i = 0;
		for(pos = BBM_REDIR_START; pos < BBM_REDIR_END; pos += blocksize) {

			bbm_debug("%s] bb_count (%ld, %ld)\n", __func__, bb_count_data, bb_count_redirect);
			if( (bb_count_data+bb_count_redirect) == nand_bbm->bb_count)
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
			nand_bbm_bitmap[reloc_index] |= BBM_REDIR_MASK ;

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
//		printk("rindex = %x, assign_index = %x\n", rindex, assign_index);
		if(assign_index != nand_bbm->assign_index) {
			bbm_print("Somthing wrong ...!!! assign_index(%lx) is unmatched\n", nand_bbm->assign_index);
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
	int ret=0;
	int index = 0;
	size_t retlen=0;
	u_char * kbuf, *src;

	pagesize = mtd->writesize;
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
	/* data block area */
	for(pos = BBM_DATA_START; pos < BBM_DATA_END; pos += blocksize) {
		if(nand_bbm_bitmap[BLOCK_INDEX(chip,pos)] & BBM_BB_MASK) {
			nand_bbm->maps[index].bb_index = (unsigned long)(BLOCK_INDEX(chip,pos));
			nand_bbm->maps[index].reloc_index = (unsigned long)(nand_bbm_bitmap[BLOCK_INDEX(chip,pos)] & BBM_IDX_MASK);
			bbm_debug("%08x -> %08x\n", (u32)nand_bbm->maps[index].bb_index, (u32)nand_bbm->maps[index].reloc_index);
			index++;
		}
	}

	/* bbm info area */
	for(pos = BBM_INFO_START; pos < BBM_INFO_END; pos += blocksize) {
		if(nand_bbm_bitmap[BLOCK_INDEX(chip,pos)] & BBM_BB_MASK) {
			nand_bbm->maps[index].bb_index = (unsigned long)(BLOCK_INDEX(chip,pos));
			nand_bbm->maps[index].reloc_index = (unsigned long)(nand_bbm_bitmap[BLOCK_INDEX(chip,pos)] & BBM_IDX_MASK);
			bbm_debug("%08x -> %08x\n", (u32)nand_bbm->maps[index].bb_index, (u32)nand_bbm->maps[index].reloc_index);
			index++;
		}
	}

	/* redirect block area */
	for(pos = BBM_REDIR_START; pos < BBM_REDIR_END; pos += blocksize) {
		if(nand_bbm_bitmap[BLOCK_INDEX(chip,pos)] & BBM_BB_MASK) {
			nand_bbm->maps[index].bb_index = (unsigned long)(BLOCK_INDEX(chip,pos));
			nand_bbm->maps[index].reloc_index = (unsigned long)(nand_bbm_bitmap[BLOCK_INDEX(chip,pos)] & BBM_IDX_MASK);
			bbm_debug("%08x -> %08x\n", (u32)nand_bbm->maps[index].bb_index, (u32)nand_bbm->maps[index].reloc_index);
			index++;
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
	bbm_print("Saving nand bad block info(%p+%p)...\n", (void*)pos, (void*)blocksize);

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
	unsigned long block_size = mtd->erasesize;

	mutex_lock(&bbminfo_mutex);

	/* data block area */
	for(ofs = BBM_DATA_START; ofs < BBM_DATA_END; ofs += block_size) {
		if(mtd->block_isbad(mtd, ofs)) {
			bbm_print("\t0x%08x is bad block => ", (unsigned int)ofs);
			ofs = (loff_t)nand_bbm_assign_block(mtd, (unsigned long)ofs, 0);
			bbm_print("0x%08x\n", (unsigned int)ofs);
		}
	}

	/* bbm info area */
	for(ofs = BBM_INFO_START; ofs < BBM_INFO_END; ofs += block_size) {
		if(mtd->block_isbad(mtd, ofs)) {
			bbm_print("\t0x%08x is bad block => ", (unsigned int)ofs);
			ofs = (loff_t)nand_bbm_assign_block(mtd, (unsigned long)ofs, 0);
			bbm_print("0x%08x\n", (unsigned int)ofs);
		}
	}

	/* redirect block area */
	for(ofs = BBM_REDIR_START; ofs < BBM_REDIR_END; ofs += block_size) {
		if(mtd->block_isbad(mtd, ofs)) {
			bbm_print("\t0x%08x is bad block => ", (unsigned int)ofs);
			ofs = (loff_t)nand_bbm_assign_block(mtd, (unsigned long)ofs, 0);
			bbm_print("0x%08x\n", (unsigned int)ofs);
		}
	}

	mutex_unlock(&bbminfo_mutex);

	return 0;
}

static void nand_bbm_print(struct mtd_info * mtd)
{
	unsigned long bb_count, index = 0;
	unsigned int block_count = 0;

	mutex_lock(&bbminfo_mutex);

	block_count = (unsigned int)(mtd->size/mtd->erasesize);
	bb_count = bb_count_data + bb_count_redirect;

	printk("nand bad block information:\n");
	printk("magic = 0x%08lx\n", nand_bbm->magic);
	printk("bad block count = %ld(%ld+%ld)\n", bb_count, bb_count_data, bb_count_redirect);
	printk("assign index = %ld\n", assign_index);

	for (index = 0; index < bb_count_data; index++) {
		printk(" %08x -> %08x\n", (u32)nand_bbm->maps[index].bb_index, (u32)nand_bbm->maps[index].reloc_index);
	}
#if 1
	for (index = 0; index < block_count; index++) {
		if((index%32)==0)
			printk("[%04lx~%04lx] ", index, index+31);

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

	if(c=='t') {
		nand_bbm_check = (nand_bbm_check) ? 0 : 1 ;
		printk("nand_bbm_check = %d\n", nand_bbm_check);
		return count;
	}

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
		break;
	case 'p' :
		nand_bbm_print(_proc_mtd);
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
#if 0
static void proc_bbminfo_exit(void)
{
	if (proc_bbminfo)
	remove_proc_entry("bbminfo", NULL);
}
#endif
#endif /* CONFIG_PROC_FS */

int nand_bbm_init(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd->priv;

	bbm_debug("Init nand bad block information...\n");

	redirect_size	= (((this->chipsize * BBM_REDIR_RATIO) / 100) & 0xFFF00000) + 0x100000;
	redirect_start	= this->chipsize - redirect_size;
	bbm_print("redirect_size = %lx, redirect_start = %lx\n", redirect_size, redirect_start);

	assign_index = (BBM_REDIR_END / mtd->erasesize) -1;

	nand_bbm_check = 0;

	nand_bbm_bad_check(mtd);

	if(nand_bbm_info_load(mtd)) {
		#ifdef __KERNEL__
		printk(KERN_ERR "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		printk(KERN_ERR "ERROR :: Failed to load bbminfo. Check bbminfo structure!.\n");
		printk(KERN_ERR "         Here initialize bbminfo but don't save it.\n");
		printk(KERN_ERR "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

		nand_bbm_info_init(mtd);

		#else
		nand_bbm_info_save(mtd);
		#endif
	}

#ifdef NAND_BBM_DEBUG
	nand_bbm_print(mtd);
#endif

	nand_bbm_check = 1;

#ifdef CONFIG_PROC_FS
	proc_bbminfo_init(mtd);
#endif
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
