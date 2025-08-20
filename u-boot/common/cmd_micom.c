////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   cmd_micom.c
/// @brief  Micom command
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <common.h>
#include <command.h>

#define MICOM_LOAD_FW_INPUT_LEN       1
#define MICOM_REG_WRITE_INPUT_LEN     3
#define MICOM_REG_READ_INPUT_LEN      2
#define MICOM_PM_SLEEP_LEN             3
#define MICOM_CTRL_WRITE_INPUT_LEN    4
#define MICOM_CTRL_READ_INPUT_LEN     2
//for I2C
#define DISP_LINE_LEN	32
//for control
#define CTRL_LEN 2
//select sleep mode
#define SLEEP_MODE  0
#define DEEP_SLEEP_MODE  1
#define POWERKEY_SLEEP_MODE  2
//enable wakeup source
#define IR_WAKEUP       (1<<0)
#define CEC_WAKEUP      (1<<1)
#define GPIO5_WAKEUP    (1<<2)
#define GPIO6_WAKEUP    (1<<3)
#define KEYPAD_WAKEUP   (1<<4)
#define EASYSYNC_WAKEUP (1<<5)  //only deep-sleep mode, test by using VGA port
#define SYNC_WAKEUP     (1<<6)   //only sleep mode, test by using VGA port
#define RTC_WAKEUP      (1<<7)
#define DVI_DET_WAKEUP      (1<<10) //only sleep mode, for HDMI_A and HDMI_B, test by using DVD player
#define DVI_DET2_WAKEUP      (1<<11) //only sleep mode, for HDMI_C, test by using DVD player

extern void drvMicom_load_fw (void);
extern B16 drvMICOM_RegWrite( U16 u16Addr, U8 u8Data );
extern B16 drvMICOM_PM_Sleep(U8 u8Data, U16 u16Data );
extern U8 drvMICOM_RegRead( U16 u16Addr );
extern B16 drvMICOM_CtrlWrite(U8 u8CmdType, U8 u8DataNum, U8 *pu8Data );
extern B16 drvMICOM_CtrlRead(U8 u8CmdType, U8 u8DataNum, U8 *pu8Data );
extern void drvIIC_Init(void);
extern S32 drvIIC_Write(U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC);
extern S32 drvIIC_Read(U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC);

#if (CONFIG_COMMANDS & CFG_CMD_MICOM)
int do_MICOM_Load_Fw ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    drvMicom_load_fw();
	printf("load fw done\n");
	return 0;
}

int do_MICOM_RegWrite ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint uaddr,udata;

    if ((argc < MICOM_REG_WRITE_INPUT_LEN) || (argc > MICOM_REG_WRITE_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    uaddr = simple_strtoul(argv[1], NULL, 16);
    udata = simple_strtoul(argv[2], NULL, 16);
    printf ("Micom register write addr:%4x and data:%4x\n",(ushort)uaddr,(uchar)udata);
    drvMICOM_RegWrite((ushort)uaddr,(uchar)udata);
	printf("Write Register Done!\n");
	return 0;
}

int do_MICOM_PmSleep ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint umode,usource;

    if ((argc < MICOM_PM_SLEEP_LEN) || (argc > MICOM_PM_SLEEP_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    umode = simple_strtoul(argv[1], NULL, 16);
    usource = simple_strtoul(argv[2], NULL, 16);
    printf ("Micom Pm Sleep Mode:%4x and WakeUp Source:%4x\n",(uchar)umode,(ushort)usource);
    drvMICOM_PM_Sleep((uchar)umode,(ushort)usource);
	return 0;
}

int do_MICOM_RegRead ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint uaddr;
	uchar udata;
    if ((argc < MICOM_REG_READ_INPUT_LEN) || (argc > MICOM_REG_READ_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }
    uaddr = simple_strtoul(argv[1], NULL, 16);
    udata = drvMICOM_RegRead((ushort)uaddr);
	printf("Read Register value is %4x\n",udata);
	return 0;
}

int do_MICOM_CtrlWrite ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint ucmd,unum, udata;
    uchar uCtrl[CTRL_LEN];
    if ((argc < MICOM_CTRL_WRITE_INPUT_LEN) || (argc > MICOM_CTRL_WRITE_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    ucmd  = simple_strtoul(argv[1], NULL, 16);
    unum  = simple_strtoul(argv[2], NULL, 16);
    if ((unum < 0) || (unum > 1))
    {
        printf ("Please input number: 0 or 1 \n");
        return 1;
    }

    udata = simple_strtoul(argv[3], NULL, 16);
	if(unum==1)
	    uCtrl[0]=(uchar)udata;

    printf ("Micom control write command:%4x number:%4x, and data:%4x\n",(uchar)ucmd,(uchar)unum,(uchar)udata);
    drvMICOM_CtrlWrite((uchar)ucmd,(uchar)unum,uCtrl);
	printf("Control Write Done!\n");
	return 0;
}


int do_MICOM_CtrlRead ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uint ucmd, unum;
    uchar udata;
    uint uCtrl[CTRL_LEN];

    if ((argc < MICOM_CTRL_READ_INPUT_LEN) || (argc > MICOM_CTRL_READ_INPUT_LEN))
    {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    ucmd  = simple_strtoul(argv[1], NULL, 16);

    drvMICOM_CtrlRead((uchar)ucmd, unum, (U8 *)&uCtrl);

	udata=uCtrl[0];
    printf("Control Read value is %4x\n",udata);

	return 0;
}

//add hardware I2C function
int do_IIC_Init ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    drvIIC_Init();
	return 0;
}

int do_IIC_Read ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uchar slave_id;
    uchar addr[2];
    uchar alen;
    uchar data[DISP_LINE_LEN];
    uint len;
    uint i;

    if ((argc < 6) || (argc > 6)) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	slave_id = simple_strtoul(argv[1], NULL, 16);
    //for EEPROM(AT24C512), if addr is one byte, this function must be modified.
	addr[0] = simple_strtoul(argv[2], NULL, 16);
    addr[1] = simple_strtoul(argv[3], NULL, 16);
    alen = simple_strtoul(argv[4], NULL, 16);;

    len = simple_strtoul(argv[5], NULL, 16);

    if (len > 32) {
		printf ("I2C read: length is out of range\n");
		return 1;
	}

    drvIIC_Read(slave_id, addr, alen, data, len);

    for(i=0;i<len;i++)
        printf("data[%d]=%4x\n",i,data[i]);

    return 0;
}

int do_IIC_Read_Byte ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uchar devaddr;
    uchar regoffset[2];
    uchar value;

    if ((argc < 3) || (argc > 3)) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	devaddr = simple_strtoul(argv[1], NULL, 16);
	//for EEPROM(AT24C512), if addr is one byte, this function must be modified.
	regoffset[0] = 0;
	regoffset[1] = simple_strtoul(argv[2], NULL, 16);

    drvIIC_Read(devaddr, regoffset, 2, &value, 1);
    printf("value=%4x\n", value);
    return 0;
}

int do_IIC_Write_Byte ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uchar devaddr;
    uchar regoffset[2];
    uchar value;

    if ((argc < 4) || (argc > 4)) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	devaddr = simple_strtoul(argv[1], NULL, 16);
	//for EEPROM(AT24C512), if addr is one byte, this function must be modified.
	regoffset[0] = 0;
	regoffset[1] = simple_strtoul(argv[2], NULL, 16);
	value = simple_strtoul(argv[3], NULL, 16);

    drvIIC_Write(devaddr, regoffset, 2, &value, 1);
	return 0;
}

//all uboot process
int do_MICOM_Run ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    uchar addr[2];
    uchar udata;
    uchar detect;
    uchar umode; //Hotel Mode
    uchar ustatus; //Last Power Status
    ushort uWkSrc = 0;
    int i;

    //set wake-up source
    uWkSrc |= (IR_WAKEUP | RTC_WAKEUP );

    //initialize I2C
    drvIIC_Init();

    addr[0] = 0;
    addr[1] = 0x20;
    //read data from eeprom
    drvIIC_Read(0xa0, addr, 2, &umode, 1);
    printf("Hotel Mode Value=%4x\n", umode);

    addr[1] = 0x10;
    //read data from eeprom
    drvIIC_Read(0xa0, addr, 2, &ustatus, 1);
    printf("Last Power Status Value=%4x\n", ustatus);

    //detect if micom is alive
    detect = drvMICOM_RegRead(0x1055);
    printf("detect value=%4x\n", detect);

    for(i=0;i<10;i++)
    {
        udata = drvMICOM_RegRead(0x1055);
        printf("udata value=%4x\n", udata);
        if( detect != udata)
        {
            printf("Micom is alive\n");
            break;
        }
        if(i==9)
        {
            //load micom firmware
            drvMicom_load_fw();
            printf("Load Micom firmware\n");
        }
    }

    udata = drvMICOM_RegRead(0x1055);
    printf("udata value=%4x\n", udata);
    udata = drvMICOM_RegRead(0x1055);
    printf("udata value=%4x\n", udata);
    udata = drvMICOM_RegRead(0x1055);
    printf("udata value=%4x\n", udata);

    switch(umode)
    {
        case 1:
            if((udata & 0x40) == 0x40)
            {
                //power on
                printf("power on\n");
                asm volatile (
                "li      $31, 0x80898000;"
                "j       $31;");
            }
            else
            {
                //stand-by
                addr[1] = 0x10;
                ustatus = 0;
                //write data to eeprom
                drvIIC_Write(0xa0, addr, 2, &ustatus, 1);
                //pm_sleep
                //drvMICOM_PM_Sleep(DEEP_SLEEP_MODE, uWkSrc);
                drvMICOM_PM_Sleep(SLEEP_MODE, uWkSrc);
            }
            break;
        case 2:
            //power on
            asm volatile (
            "li      $31, 0x80898000;"
            "j       $31;");
            break;
    }

	return 0;
}
/***************************************************/

U_BOOT_CMD(
	m_loadfw,	MICOM_LOAD_FW_INPUT_LEN,	1,	do_MICOM_Load_Fw,
	"m_loadfw- InDirect Download Firmware\n",
	"command: m_loadfw \n"
);

U_BOOT_CMD(
	m_wreg,	    MICOM_REG_WRITE_INPUT_LEN,	1,	do_MICOM_RegWrite,
	"m_wreg  - Write MICOM register\n",
	"command: m_wreg addr(hex) data(hex)\n"
);

U_BOOT_CMD(
	m_rreg,	    MICOM_REG_READ_INPUT_LEN,	1,	do_MICOM_RegRead,
	"m_rreg  - Read MICOM register\n",
	"command: m_rreg addr(hex)\n"
);

U_BOOT_CMD(
	m_wctrl,	    MICOM_CTRL_WRITE_INPUT_LEN,	1,	do_MICOM_CtrlWrite,
	"m_wctrl - Sequence Write MICOM register Nick11\n",
	"command: m_wctrl cmd(hex) num(hex) data(hex)\n"
);

U_BOOT_CMD(
	m_rctrl,	    MICOM_CTRL_READ_INPUT_LEN,	1,	do_MICOM_CtrlRead,
	"m_rctrl - Sequence Read MICOM register\n",
	"command: m_rctrl cmd(hex)\n"
);

U_BOOT_CMD(
	m_sleep,	    MICOM_PM_SLEEP_LEN,	1,	do_MICOM_PmSleep,
	"m_sleep - MICOM Pm Sleep\n",
	"command: m_sleep mode(hex) wksource(hex)\n"
);
//add hardware I2C
U_BOOT_CMD(
	m_i2ci,	1,	1,	do_IIC_Init,
	"m_i2ci  - i2c initialization\n",
	"\n    -i2c initialization!!\n"
);

U_BOOT_CMD(
	m_i2cr,	6,	1,	do_IIC_Read,
	"m_i2cr  - i2c read \n",
	"\n command: m_i2cr slaveID(hex) addr[0](hex) addr[1](hex) alen(hex) len(hex)\n"
);

U_BOOT_CMD(
	m_i2crb,	3,	1,	do_IIC_Read_Byte,
	"m_i2crb - i2c read for one byte\n",
	"\n command: m_i2crb slaveID(hex) offset(hex)\n"
);

U_BOOT_CMD(
	m_i2cwb,	4,	1,	do_IIC_Write_Byte,
	"m_i2cwb - i2c write for one byte\n",
	"\n command: m_i2cwb slaveID(hex) offset(hex) value(hex)\n"
);
//all uboot process
U_BOOT_CMD(
	m_run,	1,	1,	do_MICOM_Run,
	"m_run   - run uboot booting process\n",
	"\n    -run uboot booting process!!\n"
);

#endif /* CFG_CMD_MICOM */

