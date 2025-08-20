#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#include <nand.h>
#include "../../drivers/drvNAND.h"
#ifndef CFG_NANDDRV_DEBUG
#define CFG_NANDDRV_DEBUG			0 //default disable debug msg.
#endif

#if (CFG_NANDDRV_DEBUG)
#define NAND_DEBUG(fmt, args...)	printf(fmt, ##args)
#else
#define NAND_DEBUG(fmt, args...)
#endif

/* These really don't belong here, as they are specific to the NAND Model */
static uint8_t scan_ff_pattern[] = { 0xff };

/* struct nand_bbt_descr - bad block table descriptor */
static struct nand_bbt_descr _titania_nand_bbt_descr = {
	.options = NAND_BBT_2BIT | NAND_BBT_LASTBLOCK | NAND_BBT_VERSION |
		NAND_BBT_CREATE | NAND_BBT_WRITE,
	.offs = 5,
	.len = 1,
	.pattern = scan_ff_pattern
};

static struct nand_oobinfo _titania_nand_oob = {
	.useecc = MTD_NANDECC_AUTOPLACE,//MTD_NANDECC_AUTOPLACE,MTD_NANDECC_AUTOPL_USR /* MTD_NANDECC_PLACEONLY, */
	.eccbytes = 10,
	.eccpos = {6,7,8,9,10,11,12,13,14,15},
	.oobfree = { {0, 5} }
};

static struct nand_oobinfo _titania_nand_oob_64 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 10,
	.eccpos = { 0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
		0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
		0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
		0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,},
	.oobfree = {
		{1,5},
		{0x10,6},
		{0x20,6},
		{0x30,6}}
};

static void _titania_nand_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	//NAND_DEBUG("NANDDrv_HWCONTROL()\n");

	switch(cmd)
	{
		case NAND_CTL_SETNCE:
			NAND_DEBUG("NAND_CTL_SETNCE\n");
			MDrv_NAND_Set_Chip_Select(0x0);
			break;
		case NAND_CTL_CLRNCE:
			NAND_DEBUG("NAND_CTL_CLRNCE\n");
			MDrv_NAND_Set_Chip_Select(0x1);
			break;
		case NAND_CTL_SETCLE:
			NAND_DEBUG("NAND_CTL_SETCLE\n");
			MDrv_NAND_Set_Command_Latch(0x1);
			break;
		case NAND_CTL_CLRCLE:
			NAND_DEBUG("NAND_CTL_CLRCLE\n");
			MDrv_NAND_Set_Command_Latch(0x0);
			break;
		case NAND_CTL_SETALE:
			NAND_DEBUG("NAND_CTL_SETALE\n");
			MDrv_NAND_Set_Address_Latch(0x1);
			break;
		case NAND_CTL_CLRALE:
			NAND_DEBUG("NAND_CTL_CLRALE\n");
			MDrv_NAND_Set_Address_Latch(0x0);
			break;
		case NAND_CTL_SETWP:
			NAND_DEBUG("NAND_CTL_SETWP\n");
			MDrv_NAND_Set_Write_Protect(0x1);
			break;
		case NAND_CTL_CLRWP:
			NAND_DEBUG("NAND_CTL_CLRWP\n");
			MDrv_NAND_Set_Write_Protect(0x0);
			break;
		default:
			NAND_DEBUG("UNKNOWN CMD\n");
			break;
	}

	return;
}

static int _titania_nand_device_ready(struct mtd_info *mtdinfo)
{
	NAND_DEBUG("NANDDrv_DEVICE_READY()\n");
	return 1;
}

static void _titania_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	NAND_DEBUG("NANDDrv_WRITE_BUF()\n");
	MDrv_NAND_WriteBuf((U8 *const)buf, len);
	return;
}

static void _titania_nand_write_word(struct mtd_info *mtd, u16 word)
{
	NAND_DEBUG("NANDDrv_WRITE_WORD()\n");
	return;
}

static void _titania_nand_write_byte(struct mtd_info *mtd, u_char byte)
{
	NAND_DEBUG("NANDDrv_WRITE_BYTE(),%X\n",byte);
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
	NAND_DEBUG("NANDDrv_READ_BYTE()\n");
	return (MDrv_NAND_ReadByte());
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
static int _titania_nand_wait(struct mtd_info *mtd, struct nand_chip *this, int state)
{
	NAND_DEBUG("NANDDrv_WAIT()\n");
	MDrv_NAND_WaitReady();
	return 0;
}

static void _titania_nand_cmdfunc(struct mtd_info *mtd, unsigned command,
		int column, int page_addr)
{
	switch (command) {
		case NAND_CMD_READ0:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_READ0, page_addr: 0x%x, column: 0x%x.\n", page_addr, (column>>1));
			//MDrv_NAND_SendCmdAdr(NAND_CMD_READ0,page_addr,column);
			MDrv_NAND_SendCmd(NAND_CMD_READ0);
			MDrv_NAND_SendAdr(page_addr,column);
			//MDrv_NAND_ReadPage(page_addr,column);
			if(mtd->oobsize > 16)
				MDrv_NAND_SendCmd(NAND_CMD_READSTART);
			break;

		case NAND_CMD_READ1:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_READ1.\n");
			//MDrv_NAND_SendCmdAdr(NAND_CMD_READ1,page_addr,column);
			MDrv_NAND_SendCmd(NAND_CMD_READ1);
			MDrv_NAND_SendAdr(page_addr,column);
			break;

		case NAND_CMD_READOOB:
			NAND_DEBUG("_titania_nand_cmdfunc: NAND_CMD_READOOB.\n");
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
			printf("_titania_nand_cmdfunc: error, unsupported command.\n");
			break;
	}

	return;
}

static void _titania_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	NAND_DEBUG("enable_hwecc\r\n");
	// default enable
}

static int _titania_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	NAND_DEBUG("calculate_ecc\r\n");

	// HW auto check.
	U8 u8Tmp;
	if(mtd->oobsize > 0x10)
	{
		for (u8Tmp = 0; u8Tmp < 40; u8Tmp++)
			ecc_code[u8Tmp] = 0xFF;
	}
	else
	{
		for (u8Tmp = 0; u8Tmp < 10; u8Tmp++)
			ecc_code[u8Tmp] = 0xFF;
	}

	return 0;
}

static int _titania_nand_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
	NAND_DEBUG("correct_data\r\n");
	return(MDrv_NAND_CheckECC());
}

/*
 * Board-specific NAND initialization.
 * - hwcontrol: hardwarespecific function for accesing control-lines
 * - dev_ready: hardwarespecific function for  accesing device ready/busy line
 * - eccmode: mode of ecc, see defines
 */
void board_nand_init(struct nand_chip *nand)
{
	NAND_DEBUG("NANDDrv_BOARD_NAND_INIT().\n");

	drvNAND_FLASH_INIT();

	/* please refer to include/linux/nand.h for more info. */
	nand->hwcontrol		 = _titania_nand_hwcontrol;
	nand->dev_ready		 = _titania_nand_device_ready;
	if(_fsinfo.eFlashConfig & FLASH_2KPAGE)
	{
		nand->eccmode    = NAND_ECC_HW40_2048;
		nand->options	 = NAND_NO_AUTOINCR;
		_titania_nand_bbt_descr.offs = 0;
	}
	else
	{
		nand->eccmode    = NAND_ECC_HW10_512;
		nand->options    = 0;
	}
#ifdef CONFIG_MTD_NAND_BBM
	nand->options		|= NAND_SKIP_BBTSCAN;       //by junorion
#endif
	nand->waitfunc  	 = _titania_nand_wait;
	nand->read_byte 	 = _titania_nand_read_byte;
	nand->write_byte	 = _titania_nand_write_byte;
	nand->read_word 	 = _titania_nand_read_word;
	nand->write_word	 = _titania_nand_write_word;
	nand->read_buf  	 = _titania_nand_read_buf;
	nand->write_buf 	 = _titania_nand_write_buf;
	nand->chip_delay	 = 1; //@FIXME: unite: us, please refer to nand_base.c 20us is default.

	nand->enable_hwecc	 = _titania_nand_enable_hwecc;
	nand->calculate_ecc	 = _titania_nand_calculate_ecc;
	nand->correct_data	 = _titania_nand_correct_data;

	nand->cmdfunc		 = _titania_nand_cmdfunc;
	if(_fsinfo.eFlashConfig & FLASH_2KPAGE)
		nand->autooob    = &_titania_nand_oob_64;
	else
		nand->autooob	 = &_titania_nand_oob; //using default oob layout.
	nand->badblock_pattern = &_titania_nand_bbt_descr; //using default badblock pattern.
}

#endif //(CONFIG_COMMANDS & CFG_CMD_NAND)
