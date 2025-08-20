/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * SPI Read/Write Utilities
 */
#include <common.h>
#include <command.h>
#include <spi.h>

#include "../drivers/drvSerFlash.h"

/*-----------------------------------------------------------------------
 * Definitions
 */

#define KSEG02KSEG1(addr)   ((void *)((U32)(addr)|0x80000000))

#ifndef MAX_SPI_BYTES
#define MAX_SPI_BYTES 0x1000    /* Maximum number of bytes we can handle */
#endif
#define READ_ID_LENGTH 3

//input total char
#define BASE_CMD_INPUT_LEN             5
#define INIT_INPUT_LEN                 1
#define READ_ID_INPUT_LEN              1
#define READ_INPUT_LEN                 3
#define WRITE_INPUT_LEN                3
#define ERASE_CHIP_INPUT_LEN           1
#define ERASE_SEC_INPUT_LEN            3
#define WRITE_PRO_INPUT_LEN            2
#define WRITE_BUFF_INPUT_LEN           4
#define READ_BUFF_INPUT_LEN            3
#define GET_CHIP_REV_INPUT_LEN         1
#define GET_FLASH_INFO_INPUT_LEN       1
#define READ_STATUS_INPUT_LEN          1
#define DMA_INPUT_LEN                  4
#define READ_CODE_INPUT_LEN            4
#define WRITE_CODE_INPUT_LEN           4

#define SERFLASH_BLOCK_SIZE (64 * 1024) // <-@@@ please sync with SERFLASH_BLOCK_SIZE @ spiflash.c

/*
 * External table of chip select functions (see the appropriate board
 * support for the actual definition of the table).
 */
// spi_chipsel_type spi_chipsel[]; // UNUSED
int spi_chipsel_cnt;
uchar ubuffer[MAX_SPI_BYTES];//uint *wbuffer;//uchar *ubuffer;//
/*
 * Values from last command.
 */
static int   device;
static int   bitlen;
static uchar dout[MAX_SPI_BYTES];
//static uchar din[MAX_SPI_BYTES];

/*
 * SPI init
 * Syntax
 * spi_init"
 */

int do_spi_init ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    MDrv_SerFlash_Init();
    printf("initialization done!\n");
    return 0;
}

int do_spi_get_Chip_Rev ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uchar uGetChipRev;
    uGetChipRev=MDrv_SerFlash_GetChipRev();
    printf("Get Chip Rev:\t0x%02x\n", uGetChipRev);
    return 0;
}

int do_spi_get_flash_info ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    SerFlashInfo FlashInfo;
    MDrv_SerFlash_GetInfo(&FlashInfo);
    printf(" Block Num:\t0x%04x\n",      FlashInfo.u32BlockNum);
    printf(" Block Size:\t0x%08x\n",     FlashInfo.u32BlockSize);
    printf(" Total Size:\t0x%08x\n",     FlashInfo.u32TotalSize);
    printf(" Access Width:\t0x%02x\n",   FlashInfo.u32AccessWidth);
    return 0;
}

int do_spi_read_status ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uchar u8StatusReg;
    MDrv_SerFlash_ReadStatusReg(&u8StatusReg);
    printf(" u8StatusReg:\t0x%02X\n", u8StatusReg);
    return 0;
}

int do_spi_dma ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint uflhaddr,udmaaddr;
    uint len;

#ifdef DMA_TEST
    uint idx;
    uchar buffer[MAX_SPI_BYTES];
#endif
    if ((argc < DMA_INPUT_LEN) || (argc > DMA_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    uflhaddr = simple_strtoul(argv[1], NULL, 16);
    udmaaddr = simple_strtoul(argv[2], NULL, 16);
    len      = simple_strtoul(argv[3], NULL, 16);

    printf ("SPI flash start addr:\t0x%4x\n",uflhaddr);
    printf ("SPI DMA start addr:\t0x%4x\n",udmaaddr);
    printf ("SPI DMA length:\t0x%4x\n",len);

    if (len > MAX_SPI_BYTES)
    {
        printf ("SPI DMA: length is out of range\n");
        return 1;
    }
#ifdef DMA_TEST
        //////DMA Test start
        for( idx=0; idx<len; idx++)
        {
            buffer[idx] = (uchar)idx;
        }
        MDrv_SerFlash_Write(uflhaddr, buffer, len);

        flush_cache(udmaaddr,len);
        MDrv_SerFlash_DMA (uflhaddr, udmaaddr, len);

        for( idx=0; idx<len; idx++)
        {
            if( *(volatile uchar *)( (uint)KSEG02KSEG1(udmaaddr)+idx ) != (uchar)idx )
            {
                printf ("SPI DMA fail!\n");
            }
        }
        printf ("SPI DMA compare pass!\n");
        //////DMA Test end
#else
        MDrv_SerFlash_DMA (uflhaddr, udmaaddr, len);
        printf ("SPI DMA Done!\n");
#endif

    return 0;
}

int do_spi_readID( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uchar buffer[READ_ID_LENGTH];
    uint len;

    if ((argc < READ_ID_INPUT_LEN) || (argc > READ_ID_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    len = READ_ID_LENGTH;
    MDrv_SerFlash_ReadID (buffer, len);

    printf("Manufacturer ID:\t0x%02X\n", buffer[0]);
    printf("Device ID:\t0x%02X 0x%02X\n", buffer[1], buffer[2]);

    return 0;
}

/*
 * SPI read
 * Syntax
 * spi_r ID(hex) addr(hex) len(hex)\n"
 */
int do_spi_read ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint addr;
    uint len;
    uint idx;

    if ((argc < READ_INPUT_LEN) || (argc > READ_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    addr = simple_strtoul(argv[1], NULL, 16);
    len  = simple_strtoul(argv[2], NULL, 16);

    printf ("SPI read addr:0x%08x and len:0x%08x\n",addr,len);

    if (len > MAX_SPI_BYTES)
    {
        printf ("SPI read: length is out of range\n");
        return 1;
    }

    MDrv_SerFlash_Read ((uint)addr, ubuffer, len);//uID

    for(idx=0;idx<len;idx++)
    {
        if((idx%16)==0)
        {
            printf("[0x%08x~0x%08x]: ", (addr+idx), (addr+idx+15));
        }

        printf("%02X ", ubuffer[idx]);

        if(((idx+1)%16)==0)
        {
            printf("\n");
        }
    }

    printf("Read SPI flash done!\n");

    return 0;
}

int do_spi_read_buff ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint addr;
    uint len;
    uint idx;

    if ((argc < READ_BUFF_INPUT_LEN) || (argc > READ_BUFF_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    addr = simple_strtoul(argv[1], NULL, 16);
    len  = simple_strtoul(argv[2], NULL, 16);

    for(idx=0;idx<len;idx++)
    {
        if((idx%16)==0)
        {
            printf("[0x%08x~0x%08x]: ", (addr+idx), (addr+idx+15));
        }

        printf("%02X ",ubuffer[idx]);

        if(((idx+1)%16)==0)
        {
            printf("\n");
        }
    }
    printf("Read buffer done!\n");

    return 0;
}

int do_spi_write_protect ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uchar uEnable;
    if ((argc < WRITE_PRO_INPUT_LEN) || (argc > WRITE_PRO_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    uEnable = simple_strtoul(argv[1], NULL, 16);

    if(MDrv_SerFlash_WriteProtect(uEnable)==0)
    {
        printf ("SPI write protect fail\n");
    }
    else
    {
        printf ("SPI write protect pass\n");
    }

    return 0;
}

int do_spi_write_buff ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint addr;
    uint val;
    uint len;
    uint i;

    if ((argc < WRITE_BUFF_INPUT_LEN) || (argc > WRITE_BUFF_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    addr    = simple_strtoul(argv[1], NULL, 16);
    val     = simple_strtoul(argv[2], NULL, 16);
    len     = simple_strtoul(argv[3], NULL, 16);

    if (len > MAX_SPI_BYTES)
    {
        printf ("SPI write: length is out of range\n");
        return 1;
    }

    for(i=0; i<len; i++)
    {
        ubuffer[i]=val;
        //printf("buffer %x is %08X\n",i,val);
    }
    printf("Write buffer done!\n");

    return 0;
}

/*
 * SPI write
 */
int do_spi_write ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint addr;
    uint len;
    uint i;

    if ((argc < WRITE_INPUT_LEN) || (argc > WRITE_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    addr    = simple_strtoul(argv[1], NULL, 16);
    len     = simple_strtoul(argv[2], NULL, 16);

    if (len > MAX_SPI_BYTES)
    {
        printf ("SPI write: length is out of range\n");
        return 1;
    }

#ifdef SPI_WRITE_TEST //for testing
    for(i=0;i<len;i++)
    {
        ubuffer[i]=i;
        printf("buffer %x is %08X\n",i,ubuffer[i]);
    }
#endif

    MDrv_SerFlash_Write ((uint)addr, ubuffer, len);
    printf("write SPI flash done!\n");

    return 0;
}

int do_spi_erase_chip ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    MDrv_SerFlash_EraseChip ();

    return 0;
}

int do_spi_erase_block ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint ustartsec, uendsec;
    uint ustartAddr, uendAddr;

    if ((argc < ERASE_SEC_INPUT_LEN) || (argc > ERASE_SEC_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    ustartAddr=simple_strtoul(argv[1], NULL, 16);
    uendAddr =simple_strtoul(argv[2], NULL, 16);
    if(ustartAddr>uendAddr)
    {
        printf("Start Addr > End Addr");
        return 0;
    }
    ustartsec=ustartAddr/0x1000;
    uendsec =uendAddr/0x1000;
    //MDrv_SerFlash_EraseBlock (ustartsec,uendsec);
    MDrv_SerFlash_EraseSec(ustartsec,uendsec);
    return 0;
}

/*
 * SPI read/write
 *
 * Syntax:
 *   spi {dev} {num_bits} {dout}
 *     {dev} is the device number for controlling chip select (see TBD)
 *     {num_bits} is the number of bits to send & receive (base 10)
 *     {dout} is a hexadecimal string of data to send
 * The command prints out the hexadecimal string received via SPI.
 */

int do_spi (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    char  *cp = 0;
    uchar tmp;
    int   j;
    int   rcode = 0;

    /*
     * We use the last specified parameters, unless new ones are
     * entered.
     */

    if ((flag & CMD_FLAG_REPEAT) == 0)
    {
        if (argc >= 2)
        {
            device = simple_strtoul(argv[1], NULL, 10);
        }

        if (argc >= 3)
        {
            bitlen = simple_strtoul(argv[2], NULL, 10);
        }

        if (argc >= 4)
        {
            cp = argv[3];
            for(j = 0; *cp; j++, cp++)
            {
                tmp = *cp - '0';
                if(tmp > 9)
                {
                    tmp -= ('A' - '0') - 10;
                }

                if(tmp > 15)
                {
                    tmp -= ('a' - 'A');
                }

                if(tmp > 15)
                {
                    printf("Hex conversion error on %c, giving up.\n", *cp);
                    return 1;
                }

                if((j % 2) == 0)
                {
                    dout[j / 2] = (tmp << 4);
                }
                else
                {
                    dout[j / 2] |= tmp;
                }
            }
        }
    }

    if ((device < 0) || (device >=  spi_chipsel_cnt))
    {
        printf("Invalid device %d, giving up.\n", device);
        return 1;
    }

    if ((bitlen < 0) || (bitlen >  (MAX_SPI_BYTES * 8)))
    {
        printf("Invalid bitlen %d, giving up.\n", bitlen);
        return 1;
    }

#if 0 //Test
    if(spi_xfer(spi_chipsel[device], bitlen, dout, din) != 0) {
        printf("Error with the SPI transaction.\n");
        rcode = 1;
    } else {
        cp = (char *)din;
        for(j = 0; j < ((bitlen + 7) / 8); j++) {
            printf("%02X", *cp++);
        }
        printf("\n");
    }
#endif
    return rcode;
}

int do_spi_rdc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int flash_addr;
    unsigned int dram_addr;
    unsigned int len;

    /* check argc */
    if (argc != READ_CODE_INPUT_LEN)
    {
        printf("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    flash_addr = simple_strtoul(argv[1], NULL, 16);
    dram_addr = simple_strtoul(argv[2], NULL, 16);
    len = simple_strtoul(argv[3], NULL, 16);

    /* check alignment and show warning*/
    if (flash_addr % SERFLASH_BLOCK_SIZE)
    {
        printf("WARNING: it is better to set flash start addr aligned to %d !!!\n", SERFLASH_BLOCK_SIZE);
    }

    if (dram_addr % SERFLASH_BLOCK_SIZE)
    {
        printf("WARNING: it is better to set dram start addr aligned to %d !!!\n", SERFLASH_BLOCK_SIZE);
    }

    if (len % SERFLASH_BLOCK_SIZE)
    {
        printf("WARNING: it is better to set total length aligned to %d !!!\n", SERFLASH_BLOCK_SIZE);
    }

    /* init SPI flash first */
    MDrv_SerFlash_Init();

    /* read from flash to dram */
    if (0 == MDrv_SerFlash_Read(flash_addr, (MS_U8 *)dram_addr, len))
    {
        printf("ERROR: SPI DMA fail !!!\n");
        return 1;
    }
    else
    {
        return 0;
    }
}

int do_spi_wrc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int dram_addr;
    unsigned int flash_addr;
    unsigned int len;
    unsigned int dram_addr_for_verify;

    /* check argc */
    if (argc != WRITE_CODE_INPUT_LEN)
    {
        printf("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    dram_addr = simple_strtoul(argv[1], NULL, 16);
    flash_addr = simple_strtoul(argv[2], NULL, 16);
    len = simple_strtoul(argv[3], NULL, 16);

    /* check alignment and show warning*/
    if (dram_addr % SERFLASH_BLOCK_SIZE)
    {
        printf("WARNING: it is better to set dram start addr aligned to %d !!!\n", SERFLASH_BLOCK_SIZE);
    }

    if (flash_addr % SERFLASH_BLOCK_SIZE)
    {
        printf("WARNING: it is better to set flash start addr aligned to %d !!!\n", SERFLASH_BLOCK_SIZE);
    }

    if (len % SERFLASH_BLOCK_SIZE)
    {
        printf("WARNING: it is better to set total length aligned to %d !!!\n", SERFLASH_BLOCK_SIZE);
    }

    /* init SPI flash first */
    MDrv_SerFlash_Init();
    /* SPI erase */
    printf("Erasing...\n");
  //  MDrv_SerFlash_WriteProtect(0);
    //MDrv_SerFlash_EraseBlock(flash_addr/SERFLASH_BLOCK_SIZE, (flash_addr+len)/SERFLASH_BLOCK_SIZE);
    MDrv_SerFlash_EraseSec(flash_addr/SERFLASH_BLOCK_SIZE, (flash_addr+len)/SERFLASH_BLOCK_SIZE);
    /* SPI write */
    printf("Writing...\n");
    MDrv_SerFlash_Write(flash_addr, (MS_U8 *)dram_addr, len);
  //  MDrv_SerFlash_WriteProtect(1);
#if 0
    /* SPI verify */
    printf("Verifying...");
    dram_addr_for_verify = ((dram_addr+len+SERFLASH_BLOCK_SIZE-1) / SERFLASH_BLOCK_SIZE) * SERFLASH_BLOCK_SIZE;
    if (0 != MDrv_SerFlash_Read(flash_addr, (MS_U8 *)dram_addr_for_verify, len))
    {
        int i;
        for (i = 0; i < len; i++)
        {
            if (((unsigned char *)dram_addr)[i] != ((unsigned char *)dram_addr_for_verify)[i])
            {
                break;
            }
        }

        if (i == len)
        {
            printf("OK !!!\n");
            return 0;
        }
    }

    printf("FAIL !!!\n");
#endif
    return 1;
}

/***************************************************/

U_BOOT_CMD(
    sspi,       BASE_CMD_INPUT_LEN,           1,    do_spi,
    "sspi    - SPI utility commands\n",
    "<device> <bit_len> <dout> - Send <bit_len> bits from <dout> out the SPI\n"
    "<device>  - Identifies the chip select of the device\n"
    "<bit_len> - Number of bits to send (base 10)\n"
    "<dout>    - Hexadecimal string that gets sent\n"
);

U_BOOT_CMD(
    spi_in,     INIT_INPUT_LEN,               1,    do_spi_init,
    "spi_in  - SPI initialization\n",
    "command: spi_in\n"
);

U_BOOT_CMD(
    spi_id,     READ_ID_INPUT_LEN,           1,    do_spi_readID,
    "spi_id  - SPI read ID\n",
    "command: spi_id\n"
);

U_BOOT_CMD(
    spi_r,      READ_INPUT_LEN,               1,    do_spi_read,
    "spi_r   - SPI read commands\n",
    "command: spi_r addr(hex) len(hex)\n"
);

U_BOOT_CMD(
    spi_w,      WRITE_INPUT_LEN,           1,    do_spi_write,
    "spi_w   - SPI write commands\n",
    "command: spi_w addr(hex) len(hex) \n"
);

U_BOOT_CMD(
    spi_ea,     ERASE_CHIP_INPUT_LEN,       1,    do_spi_erase_chip,
    "spi_ea  - SPI erase all\n",
    "command: spi_ea\n"
);

U_BOOT_CMD(
    spi_eb,     ERASE_SEC_INPUT_LEN,       1,    do_spi_erase_block,
    "spi_eb  - SPI erase block\n",
    "command: spi_es start addr(hex) end addr(hex)\n"
);

U_BOOT_CMD(
    spi_wp,     WRITE_PRO_INPUT_LEN,       1,    do_spi_write_protect,
    "spi_wp  - SPI write protect \n",
    "command: spi_wp (enable(1)or disable(0))\n"
);

U_BOOT_CMD(
    spi_gr,     GET_CHIP_REV_INPUT_LEN,       1,    do_spi_get_Chip_Rev,
    "spi_gr  - SPI get Chip Rev\n",
    "command: spi_gr \n"
);

U_BOOT_CMD(
    spi_gfo,    GET_FLASH_INFO_INPUT_LEN,  1,    do_spi_get_flash_info,
    "spi_gfo - SPI get flash info\n",
    "command:spi_gfo\n"
);

U_BOOT_CMD(
    spi_rs,     READ_STATUS_INPUT_LEN,       1,    do_spi_read_status,
    "spi_rs  - SPI read status\n",
    "command:spi_rs\n"
);

U_BOOT_CMD(
    spi_dma,    DMA_INPUT_LEN,           1,    do_spi_dma,
    "spi_dma - SPI copy data from flash to DRAM by PIU DMA\n",
    "command: spi_dma flash start addr(hex) DRAM start addr(hex) len(hex)\n"
);

U_BOOT_CMD(
    spi_wb,     WRITE_BUFF_INPUT_LEN,       1,    do_spi_write_buff,
    "spi_wb  - SPI write buffer\n",
    "command: spi_ewb edit addr(hex) value(hex) len(hex)\n"
);

U_BOOT_CMD(
    spi_rb,     READ_BUFF_INPUT_LEN,        1,    do_spi_read_buff,
    "spi_rb  - SPI read buffer\n",
    "command: spi_rb addr(hex) len(hex)\n"
);

U_BOOT_CMD(
    spi_rdc,    READ_CODE_INPUT_LEN,    1,  do_spi_rdc,
    "spi_rdc - SPI read code from SPI flash to DRAM\n",
    "from_flash_addr(hex) to_dram_addr(hex) len(hex)\n"
    "    - from_flash_addr: flash start address (hex, flash erase size aligned)\n"
    "    - to_dram_addr: dram start address (hex, flash erase size aligned)\n"
    "    - len: total lenght to move data (hex, flash erase size aligned)\n"
);

U_BOOT_CMD(
    spi_wrc,    WRITE_CODE_INPUT_LEN,   1,  do_spi_wrc,
    "spi_wrc - SPI write code from DRAM to SPI flash\n",
    "from_dram_addr(hex) to_flash_addr(hex) len(hex)\n"
    "    - from_dram_addr: dram start address (hex, flash erase size aligned)\n"
    "    - to_flash_addr: flash start address (hex, flash erase size aligned)\n"
    "    - len: total lenght to move data (hex, flash erase size aligned)\n"
);

