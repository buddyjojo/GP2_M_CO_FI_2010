#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/mtd_info.h>

#include <asm/io.h>
#include <asm/semaphore.h>

#ifdef CONFIG_MTD_NAND_BBM
#include <linux/mtd/nand_bbm.h>
#endif

#include "drvNAND.h"

#ifndef CFG_NANDDRV_DEBUG
#define CFG_NANDDRV_DEBUG			0 //default disable debug msg.
#endif

#if (CFG_NANDDRV_DEBUG)
#define NAND_DEBUG(fmt, args...)	printk(fmt, ##args)
#else
#define NAND_DEBUG(fmt, args...)
#endif

struct mtd_info *nand_mtd = NULL;
EXPORT_SYMBOL(nand_mtd);

#ifdef CONFIG_MTD_NAND_MTDINFO
struct mtd_partition mstar_flash_parts[MTD_PARTITION_MAX];
static int nb_parts = 0;

unsigned int get_mtd_partitions(struct mtd_info *mtd)
{
	struct mstar_mtd_info *mstar_mtdinfo;
	struct mtd_partition_info	*mstar_ptt;
	int i=0, ret=0, retlen=0;

	mstar_mtdinfo = (struct mstar_mtd_info *)kmalloc (sizeof(struct mstar_mtd_info), GFP_KERNEL);
	if(mstar_mtdinfo == NULL) {
		printk("%s : kmalloc error\n", __func__);
		return 1;
	}

	ret = mtd->read(mtd, NAND_MTDINFO_OFFSET, sizeof(struct mstar_mtd_info),
			&retlen, (u_char *)(mstar_mtdinfo));

	if(ret) {
		printk("%s : mtdinfo read failed from nand flash\n", __func__);
		return ret;
	}

	nb_parts = mstar_mtdinfo->npartition;

	printk("mtd info : # of partition = %d\n", nb_parts);
	for(i=0; i<nb_parts; i++) {
		mstar_ptt = (struct mtd_partition_info*)&(mstar_mtdinfo->partition[i]);
		mstar_flash_parts[i].name = mstar_ptt->name;
		mstar_flash_parts[i].offset	= mstar_ptt->offset;
		mstar_flash_parts[i].size = mstar_ptt->size;
		if(mstar_flash_parts[i].mask_flags == MTD_WRITEABLE)
			mstar_flash_parts[i].mask_flags = mstar_ptt->mask_flags;
#ifdef FLASH_DEBUG
		printk("{\n");
		printk("  .name =		\"%s\"\n", mstar_flash_parts[i].name);
		printk("  .offset = 0x%x\n", mstar_flash_parts[i].offset);
		printk("  .size =		0x%x\n", mstar_flash_parts[i].size);
		if(mstar_flash_parts[i].mask_flags == MTD_WRITEABLE)
			printk(".mask_flags = %s\n", "MTD_WRITEABLE");
		printk("}\n");
#endif
	}

	add_mtd_partitions(mtd, mstar_flash_parts, nb_parts);

	//kfree(mstar_mtdinfo);
	return 0;
}
#endif

/* These really don't belong here, as they are specific to the NAND Model */
static uint8_t scan_ff_pattern[] = { 0xff };

/* struct nand_bbt_descr - bad block table descriptor */
static struct nand_bbt_descr _titania_nand_bbt_descr = {
	.options = 	NAND_BBT_2BIT	| NAND_BBT_LASTBLOCK | NAND_BBT_VERSION |
		NAND_BBT_CREATE	| NAND_BBT_WRITE,
	.offs = 5,
	.len = 1,
	.pattern = scan_ff_pattern
};

static struct nand_ecclayout titania_nand_oob_16 = {
	.eccbytes 	= 10,
	.eccpos 	= {6,7,8,9,10,11,12,13,14,15},
	.oobfree 	= {
		{.offset = 0,
			.length = 5}
	}
};

static struct nand_ecclayout titania_nand_oob_64 = {
	.eccbytes = 40,
	.eccpos = { 0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
		0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
		0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
		0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,},
	.oobfree = {
		{.offset = 1,
			.length = 5},
		{.offset = 0x10,
			.length = 6},
		{.offset = 0x20,
			.length = 6},
		{.offset = 0x30,
			.length = 6}}
};

static void _titania_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
    //NAND_DEBUG("NANDDrv_HWCONTROL(),ctrl=%X\n",ctrl);

    if(ctrl & NAND_CTRL_CHANGE)
    {
        if(ctrl & NAND_NCE)
        {
            MDrv_NAND_Set_Chip_Select(0x0);
            //NAND_DEBUG("NAND_CTL_SET CE low\n");
        }
        else
        {
            //MDrv_NAND_Set_Chip_Select(0x1);
            //NAND_DEBUG("NAND_CTL_SET CE high\n");
        }

		if(ctrl & NAND_CLE)
		{
			MDrv_NAND_Set_Command_Latch(0x1);
			//NAND_DEBUG("NAND_CTL_SETCLE\n");
		}
		else
		{
			MDrv_NAND_Set_Command_Latch(0x0);
			//NAND_DEBUG("NAND_CTL_CLRCLE\n");
		}

		if(ctrl & NAND_ALE)
		{
			MDrv_NAND_Set_Address_Latch(0x1);
			//NAND_DEBUG("NAND_CTL_SETALE\n");
		}
		else
		{
			MDrv_NAND_Set_Address_Latch(0x0);
			//NAND_DEBUG("NAND_CTL_CLRALE\n");
		}
	}

    if(cmd != NAND_CMD_NONE)
    {
        MDrv_NAND_SendCmd(cmd);
    }

	return;
}

static int _titania_nand_device_ready(struct mtd_info *mtdinfo)
{
	//NAND_DEBUG("NANDDrv_DEVICE_READY()\n");
	MDrv_NAND_WaitReady();
	return 1;
}

static void _titania_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	NAND_DEBUG("NANDDrv_WRITE_BUF() 0x%X\r\n",len);
		MDrv_NAND_WriteBuf((U8 *const)buf, len);
	return;
}

static void _titania_nand_read_buf(struct mtd_info *mtd, u_char* const buf, int len)
{
	NAND_DEBUG("NANDDrv_READ_BUF()0x%X\n",len);

	MDrv_NAND_ReadBuf(buf, len);
	return;
}

static u16 _titania_nand_read_word(struct mtd_info *mtd)
{
	NAND_DEBUG("NANDDrv_READ_WORD()\n");
	return 0;
}

static u_char _titania_nand_read_byte(struct mtd_info *mtd)
{
	U8 u8Ret ;

	//NAND_DEBUG("NANDDrv_READ_BYTE()\n");
	u8Ret = MDrv_NAND_ReadByte() ;

	return (u8Ret);
}

/**
 * nand_wait - [DEFAULT]  wait until the command is done
 * @mtd:	MTD device structure
 * @this:	NAND chip structure
 * @state:	state to select the max. timeout value
 *
 * Wait for command done. This applies to erase and program only
 * Erase can take up to 400ms and program up to 20ms according to
 * general NAND and SmartMedia specs
 *
 */
static int _titania_nand_wait(struct mtd_info *mtd, struct nand_chip *this)
{
	NAND_DEBUG("NANDDrv_WAIT()\n");

	MDrv_NAND_WaitReady();
	return 0;
}

static void _titania_nand_cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
    _fsinfo.u16NC_CFG0 &= ~(R_NC_CE_AUTO+R_NC_WP_AUTO);
    _fsinfo.u16NC_CFG0 |= (R_NC_CE_EN+R_NC_WP_EN);
    HAL_WRITE_UINT16(NC_CFG0,_fsinfo.u16NC_CFG0);
    drvNAND_ClearNFIE_EVENT();

	switch (command) {
		case NAND_CMD_READ0:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_READ0, page_addr: 0x%x, column: 0x%x.\n", page_addr, (column>>1));
			MDrv_NAND_SendCmdAdr(NAND_CMD_READ0, page_addr, column);
			if(mtd->oobsize > 16)
			{
				MDrv_NAND_SendCmd(NAND_CMD_READSTART);
			}
			break;

		case NAND_CMD_READ1:
			//NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_READ1.\n");
			MDrv_NAND_SendCmd(NAND_CMD_READ1);
			MDrv_NAND_SendAdr(page_addr,column);
			break;

		case NAND_CMD_READOOB:
			//NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_READOOB,page_addr: 0x%x, column: 0x%x.\n", page_addr, column);
			MDrv_NAND_ReadOOB(page_addr,column);
			break;

		case NAND_CMD_READID:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_READID.\n");
			drvNAND_READ_ID();
			break;

		case NAND_CMD_PAGEPROG:
			/* sent as a multicommand in NAND_CMD_SEQIN */
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_PAGEPROG.\n");
			MDrv_NAND_Cmd_PageProgram(NAND_CMD_PAGEPROG);
			break;

		case NAND_CMD_ERASE1:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_ERASE1,  page_addr: 0x%x, column: 0x%x.\n", page_addr, column);
			MDrv_NAND_Erase1(page_addr);
			break;

		case NAND_CMD_ERASE2:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_ERASE2.\n");
			MDrv_NAND_Erase2();
			break;

		case NAND_CMD_SEQIN:
			/* send PAGE_PROG command(0x1080) */
			MDrv_NAND_WritePage(page_addr,column);
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_SEQIN/PAGE_PROG,  page_addr: 0x%x, column: 0x%x.\n", page_addr, column);
			break;

		case NAND_CMD_STATUS:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_STATUS.\n");
			MDrv_NAND_Send_Read_Status_CMD(NAND_CMD_STATUS);
			break;

		case NAND_CMD_RESET:
			drvNAND_FLASHRESET();
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_RESET.\n");
			break;

		case NAND_CMD_STATUS_MULTI:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_STATUS_MULTI.\n");
			MDrv_NAND_Send_Read_Status_CMD(NAND_CMD_STATUS_MULTI);
			break;

		case NAND_CMD_READSTART:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_READSTART.\n");
			MDrv_NAND_SendCmd(NAND_CMD_READSTART);
			break;

		case NAND_CMD_CACHEDPROG:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_CACHEDPROG.\n");
			MDrv_NAND_Cmd_PageProgram(NAND_CMD_CACHEDPROG);
			break;

		default:
			printk("_titania_nand_cmdfunc: error, unsupported command.\n");
			break;
	}

	return;
}

static void _titania_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	//NAND_DEBUG("enable_hwecc\r\n");
	// default enable
}

static int _titania_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	U8 u8Tmp;

	NAND_DEBUG("calculate_ecc\r\n");

	if(mtd->oobsize > 0x10)
	{
		for(u8Tmp = 0; u8Tmp < 40; u8Tmp++)
			ecc_code[u8Tmp] = 0xFF;
	}
	else
	{
		for(u8Tmp = 0; u8Tmp < 10; u8Tmp++)
			ecc_code[u8Tmp] = 0xFF;
	}
	return 0;
}

static int _titania_nand_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
	U8 u8Ret;

	//NAND_DEBUG("correct_data\r\n");
	u8Ret = MDrv_NAND_CheckECC();
	return(u8Ret);
}

/*
 * Board-specific NAND initialization.
 * - hwcontrol: hardwarespecific function for accesing control-lines
 * - dev_ready: hardwarespecific function for  accesing device ready/busy line
 * - eccmode: mode of ecc, see defines
 */
void titania_nand_init(struct nand_chip *nand)
{
	NAND_DEBUG("NANDDrv_BOARD_NAND_INIT().\n");

	drvNAND_FLASH_INIT();

	/* please refer to include/linux/nand.h for more info. */
	nand->dev_ready			= _titania_nand_device_ready;
	nand->cmd_ctrl 			= _titania_nand_hwcontrol;
	nand->ecc.mode			= NAND_ECC_HW;
	if(_fsinfo.eFlashConfig & FLASH_2KPAGE)
	{
		nand->ecc.size		= 2048;
		nand->ecc.bytes		= 40;
		nand->ecc.layout	= &titania_nand_oob_64;
		_titania_nand_bbt_descr.offs = 0;
	}
	else
	{
		nand->ecc.size		= 512;
		nand->ecc.bytes		= 10;
		nand->ecc.layout	= &titania_nand_oob_16;
	}
	nand->ecc.hwctl			= _titania_nand_enable_hwecc;
	nand->ecc.correct  		= _titania_nand_correct_data;
	nand->ecc.calculate		= _titania_nand_calculate_ecc;
	if(_fsinfo.eFlashConfig & FLASH_2KPAGE)
		nand->options		= NAND_NO_AUTOINCR;
	else
		nand->options		= 0;
#ifdef CONFIG_MTD_NAND_BBM
	nand->options			|= NAND_SKIP_BBTSCAN;
#endif
	nand->waitfunc   		= _titania_nand_wait;
	nand->read_byte  		= _titania_nand_read_byte;
	nand->read_word			= _titania_nand_read_word;
	nand->read_buf 			= _titania_nand_read_buf;
	nand->write_buf			= _titania_nand_write_buf;
	nand->chip_delay		= 20;
	nand->cmdfunc			= _titania_nand_cmdfunc;
	nand->badblock_pattern	= &_titania_nand_bbt_descr; //using default badblock pattern.
	MDrv_NAND_Set_Write_Protect(1);
}

/*
 * Main initialization routine
 */
#define CFG_NAND_BASE					0x00000000

int __init mstar_nand_init (void)
{
	struct nand_chip *this;
	int err = 0;

	/* Allocate memory for MTD device structure and private data */
	nand_mtd = kmalloc (sizeof(struct mtd_info), GFP_KERNEL);
	if (!nand_mtd) {
		printk (KERN_WARNING "Unable to allocate NAND MTD device structure.\n");
		err = -ENOMEM;
		goto out;
	}
	this = (struct nand_chip *) kmalloc (sizeof (struct nand_chip),	GFP_KERNEL);
	if (!this) {
		printk (KERN_WARNING "Unable to allocate NAND MTD device structure.\n");
		err = -ENOMEM;
		goto out;
	}

	/* Initialize structures */
	memset((char *) nand_mtd, 0, sizeof(struct mtd_info));
	memset((char *) this, 0, sizeof(struct nand_chip));

	/* Link the private data with the MTD structure */
	nand_mtd->priv = this;

	this->IO_ADDR_R = this->IO_ADDR_W = (void  __iomem *)CFG_NAND_BASE;
	titania_nand_init(this);

	/* Scan to find existance of the device */
	if (nand_scan (nand_mtd, 1)) {
		err = -ENXIO;
		goto out_ior;
	}

#ifdef CONFIG_MTD_NAND_BBM
	if(nand_bbm_init(nand_mtd)) {
		goto out;
	}
#endif

#ifdef CONFIG_MTD_NAND_MTDINFO
	printk(KERN_NOTICE "Mstar nand-flash partition definition\n");
	if(!get_mtd_partitions(nand_mtd)) {
		goto out;
	}
#endif

out_ior:
	kfree(nand_mtd);
out:
	return err;
}

/*
 * Clean up routine
 */
static void __exit mstar_nand_cleanup (void)
{
	/* nand_release frees MTD partitions, MTD structure
	   and nand internal buffers*/
	nand_release (nand_mtd);
	kfree (nand_mtd);
}

module_init(mstar_nand_init);
module_exit(mstar_nand_cleanup);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("NAND driver");
MODULE_LICENSE("MSTAR");
