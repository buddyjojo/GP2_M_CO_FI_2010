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
/// @file   drvMicom.c
/// @brief  Micom Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <common.h>
#include <command.h>

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define MICOM_REG(addr)                     (*(volatile U32 *)(addr))
#define MICOM_REG_8(addr)                   (*(volatile U8 *)(addr))
#define mdelay(n)							({unsigned long msec=(n); while (msec--) udelay(1000);})
#define msleep(a)							udelay(a * 1000)
//for I2C
#define IIC_REG(addr) 						(*(volatile U32*)(REG_IIC_BASE + ((addr)<<2)))
#define CHIP_REG(addr)						(*(volatile U32*)(REG_CHIP_BASE + ((addr)<<2)))

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
//for test
typedef struct
{
	U8 Preamble    : 2;
	U8 Opcode    : 6;
} PM_CMD_t;

static PM_CMD_t gPmCMD;

//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------
#define REG_BASE_ADDR_MIPS                  0xBF800000
#define MMAP_MICOM_FW_SIZE                  0x2000             //8K isize

//-------------------------------------------------------------------------------------------------
// Register define
//-------------------------------------------------------------------------------------------------
#define BIT0 								0x00000001
#define BIT1 								0x00000002
#define BIT2 								0x00000004
#define BIT3 								0x00000008
#define BIT4 								0x00000010
#define BIT5 								0x00000020
#define BIT6 								0x00000040
#define BIT7 								0x00000080
#define BIT8 								0x00000100
#define BIT9 								0x00000200
#define BIT10								0x00000400
#define BIT11								0x00000800
#define BIT12								0x00001000
#define BIT13								0x00002000
#define BIT14								0x00004000
#define BIT15								0x00008000

#define REG_XIU_MBOX0						0x0e0c*2
#define REG_XIU_MBOX1						0x0e10*2
#define REG_XIU_MBOX2						0x0e14*2
#define REG_XIU_MBOX3						0x0e18*2
#define REG_RIU_MBOX0						0x0e20*2
#define REG_RIU_MBOX1						0x0e24*2
#define REG_RIU_MBOX2						0x0e28*2
#define REG_RIU_MBOX3						0x0e2c*2
#define REG_INT_STATUS_TO_CHIP_TOP			0x0e04*2

#define REG_HK_TO_MICOM_MB_0			(REG_BASE_ADDR_MIPS+REG_XIU_MBOX0)
#define REG_HK_TO_MICOM_MB_1			(REG_BASE_ADDR_MIPS+REG_XIU_MBOX1)
#define REG_HK_TO_MICOM_MB_2			(REG_BASE_ADDR_MIPS+REG_XIU_MBOX2)
#define REG_HK_TO_MICOM_MB_3			(REG_BASE_ADDR_MIPS+REG_XIU_MBOX3)
#define REG_MICOM_TO_HK_MB_0			(REG_BASE_ADDR_MIPS+REG_RIU_MBOX0)
#define REG_MICOM_TO_HK_MB_1			(REG_BASE_ADDR_MIPS+REG_RIU_MBOX1)
#define REG_MICOM_TO_HK_MB_2			(REG_BASE_ADDR_MIPS+REG_RIU_MBOX2)
#define REG_MICOM_TO_HK_MB_3			(REG_BASE_ADDR_MIPS+REG_RIU_MBOX3)
#define REG_MICOM_TO_HK_MB_STATUS		(REG_BASE_ADDR_MIPS+REG_INT_STATUS_TO_CHIP_TOP)

//for I2C
#define REG_IIC_BASE     				0xBF806840    // 0xBF800000 + (0x1A10*4)
#define REG_IIC_CTRL     				0x00
#define REG_IIC_CLK_SEL  				0x01
#define REG_IIC_WDATA    				0x02
#define REG_IIC_RDATA    				0x03
#define REG_IIC_STATUS   				0x04         // reset, clear and status
#define REG_CHIP_BASE    				0xBF803C00
#define REG_IIC_MODE     				0x50
#define REG_DDCR_GPIO_SEL				0x70

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define TRUE 							1
#define FALSE							0

//PM_CMD opcode
#define WRITE_CMD     					0
#define READ_CMD      					1
#define SLEEP_CMD     					2
#define CTRL_WRITE_CMD					3
#define CTRL_READ_CMD 					4

#define MAILBOX_TIMEOUT  				100 //ms
#define MICOM_DELAY_TIME0				 10  //ms

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
void drvMicom_load_fw (void)
{
	unsigned int u32I;
	unsigned char memdata[MMAP_MICOM_FW_SIZE];

	//memory copy
    memcpy(memdata, (void *)0x80c00000, 0x2000);

	MICOM_REG(REG_BASE_ADDR_MIPS+(0x0E00*2)) |= 0x02;

	MICOM_REG(REG_BASE_ADDR_MIPS+(0x0E3C*2)) |= 0x02;

	MICOM_REG(REG_BASE_ADDR_MIPS+(0x0E00*2)) |= 0x04;

	MICOM_REG(REG_BASE_ADDR_MIPS+(0X0E30*2)) = 0;

#if 1
	for (u32I=0; (u32I< MMAP_MICOM_FW_SIZE)&&(MMAP_MICOM_FW_SIZE<=0x2000); u32I++)
	{
		MICOM_REG(REG_BASE_ADDR_MIPS+(0x0E34*2)) =
			(U8)(MICOM_REG_8(memdata + u32I));
	}
#endif

	MICOM_REG(REG_BASE_ADDR_MIPS+(0x0E00*2)) &= ~0x04;

	MICOM_REG(REG_BASE_ADDR_MIPS+(0x0E00*2)) |= 0x01;

	MICOM_REG(REG_BASE_ADDR_MIPS+(0x0E3C*2)) &= ~0x02;

	MICOM_REG(REG_BASE_ADDR_MIPS+(0x0E60*2)) |= 0x80;
}

/******************************************************************************/
///Write 1 byte data to M4 Lite register
///@param u16Addr \b IN  register address
///@param u8Data \b IN  8 bits (1 byte) value to write
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
/******************************************************************************/
B16 drvMICOM_RegWrite( U16 u16Addr, U8 u8Data )
{
	U16 u16Count;
	PM_CMD_t   send_write_cmd, recv_write_cmd;
	U16 M4_status = 0;
	U16 u16Temp;

	gPmCMD.Preamble++;

	send_write_cmd.Opcode = WRITE_CMD;

	//Clear m4 (Read than Write for timing issue)
	M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
	//Clear IRQ
	MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;


	//Mail Box 1
	MICOM_REG(REG_HK_TO_MICOM_MB_1) = u16Addr;

	//Mail Box 0
	u16Temp = ((U16)u8Data)<<8;
	u16Temp |= (gPmCMD.Preamble<<6) | (send_write_cmd.Opcode);
	MICOM_REG(REG_HK_TO_MICOM_MB_0) = u16Temp;

	u16Count = MAILBOX_TIMEOUT;
	//polling interrput status
	while( !(  MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) & BIT9   ) )
	{
		msleep(1);
		if((u16Count--) == 0)
		{
			printf("u16Count == 0 [RegWrite] timeout\n");
			break;
		}
	}
	recv_write_cmd.Preamble = (U8)((MICOM_REG(REG_MICOM_TO_HK_MB_0) & 0x00C0)>>6 );
	recv_write_cmd.Opcode = (U8)MICOM_REG(REG_MICOM_TO_HK_MB_0);

	//Clear m4 (Read than Write for timing issue)
	M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
	//Clear IRQ
	MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;

	if( gPmCMD.Preamble == recv_write_cmd.Preamble \
			&& 0x3F == recv_write_cmd.Opcode )
	{
		return TRUE;
	}
	else
	{
		printf("drvMICOM_RegWrite FAIL\n");
		return FALSE;
	}
}

/******************************************************************************/
///Read 1 byte data from M4 Lite register
///@param u16Addr \b IN  register address
///@return
///- 8 bits (1 byte) value for read
///- FALSE: Fail
/******************************************************************************/

U8 drvMICOM_RegRead( U16 u16Addr )
{
	U16 u16Count;
	PM_CMD_t send_read_cmd, recv_read_cmd;
	U16 M4_status = 0;
	U8 u8Data;

	gPmCMD.Preamble++;
	send_read_cmd.Opcode = READ_CMD;

	//Clear m4 (Read than Write for timing issue)
	M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
	//Clear IRQ
	MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;

	//Mail Box 1
	MICOM_REG(REG_HK_TO_MICOM_MB_1) = u16Addr;

	//Mail Box 0
	MICOM_REG(REG_HK_TO_MICOM_MB_0) = ((gPmCMD.Preamble<<6) | (send_read_cmd.Opcode));

	u16Count = MAILBOX_TIMEOUT;
	//polling interrput status
	while( !(  MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) & BIT9   ) )
	{
		msleep(1);
		if((u16Count--) == 0)
		{
			printf("u16Count == 0 [RegRead] timeout\n");
			break;
		}
	}
	recv_read_cmd.Preamble = (U8)((MICOM_REG(REG_MICOM_TO_HK_MB_0) & 0x00C0)>>6 );
	recv_read_cmd.Opcode = (U8)MICOM_REG(REG_MICOM_TO_HK_MB_0);
	u8Data = (U8)((MICOM_REG(REG_MICOM_TO_HK_MB_0) & 0xFF00)>>8);

	//Clear m4 (Read than Write for timing issue)
	M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
	//Clear IRQ
	MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;

	if( gPmCMD.Preamble == recv_read_cmd.Preamble \
			&& 0x3F == recv_read_cmd.Opcode )
	{
		return u8Data;
	}
	else
	{
		printf("gPmCMD.Preamble = %d\n", gPmCMD.Preamble);
		printf("recv_read_cmd.Preamble = %d\n", recv_read_cmd.Preamble);
		printf("recv_read_cmd.Opcode = %x\n", recv_read_cmd.Opcode);

		printf("drvMICOM_RegRead FAIL\n");
		return FALSE;
	}
}

B16 drvMICOM_PM_Sleep(U8 u8Data, U16 u16Data )
{
	U16 u16Count;
	PM_CMD_t   send_write_cmd, recv_write_cmd;
	U16 M4_status = 0;
	U16 u16Temp;

	gPmCMD.Preamble++;

	send_write_cmd.Opcode = (SLEEP_CMD & 0x3F);

	//Clear m4 (Read than Write for timing issue)
	M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
	//Clear IRQ
	MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;

	//Mail Box 1
	printf("Write u16Data=%04x\n",u16Data);
	MICOM_REG(REG_HK_TO_MICOM_MB_1) = u16Data;

	//Mail Box 0
	u16Temp = (u8Data<<8);
	u16Temp |= ((gPmCMD.Preamble<<6) | (send_write_cmd.Opcode));
	printf("Write u16Temp=%04x\n",u16Temp);
	MICOM_REG(REG_HK_TO_MICOM_MB_0) = u16Temp;


	u16Count = MAILBOX_TIMEOUT;
	//polling interrput status
	while( !(  MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) & BIT9   ) )
	{
		msleep(1);
		if((u16Count--) == 0)
		{
			printf("u16Count == 0 [PM_Sleep] timeout\n");
			break;
		}
	}
	recv_write_cmd.Preamble = (U8)((MICOM_REG(REG_MICOM_TO_HK_MB_0) & 0x00C0)>>6 );
	recv_write_cmd.Opcode = (U8)MICOM_REG(REG_MICOM_TO_HK_MB_0);

	//Clear m4 (Read than Write for timing issue)
	M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
	//Clear IRQ
	MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;

	if( gPmCMD.Preamble == recv_write_cmd.Preamble \
			&& 0x3F == recv_write_cmd.Opcode )
	{
		return TRUE;
	}
	else
	{
		printf("drvMICOM_PM_Sleep FAIL\n");
		return FALSE;
	}
}

/******************************************************************************/
///PM Control Write Function
///@param u8opcode \b IN  control command
///@param u8Data \b IN  8 bits data
///@param u16Data \b IN  16 bits data
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
/******************************************************************************/
B16 drvMICOM_CtrlWrite(U8 u8CmdType, U8 u8DataNum, U8 *pu8Data )
{
	U16 M4_status = 0;
	U16 u16Temp;
	U16 u16Count = 0;
	U8 u8DataIndex = u8DataNum;
	PM_CMD_t   send_write_cmd, recv_write_cmd;
	//printf("Enter drvMICOM_CtrlWrite\n"); //for debug
	send_write_cmd.Opcode = (CTRL_WRITE_CMD & 0x3F);
	while( u8DataIndex-- )
	{
		//printf("before gPmCMD.Preamble = %d\n", gPmCMD.Preamble);

		gPmCMD.Preamble++;
		//printf("After gPmCMD.Preamble = %d\n", gPmCMD.Preamble);

		//Clear m4 (Read than Write for timing issue)
		M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
		//Clear IRQ
		MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;
		//Mail Box 1
		//printf("Write u8DataIndex=%02x\n", u8DataIndex);  //Index of u8Data
		//printf("before write pu8Data[%d]=%02x\n", u8DataIndex, pu8Data[u8DataIndex]);
		u16Temp = ( ( (U16)(pu8Data[u8DataIndex])<<8) | (U16)u8DataIndex);   //H: Data, L: Index
		printf("u16Temp is:%x\n",u16Temp); //for debug
		MICOM_REG(REG_HK_TO_MICOM_MB_1) = (u32)u16Temp;
		//Mail Box 0
		u16Temp = 0;// case CP_NO_CMD = 0
		//if( 0 == u8DataIndex )    //After send full length of data then with a command
		//{
		u16Temp = ((U16)u8CmdType<<8);  //DDI Command Type
		//}
		u16Temp |= ((gPmCMD.Preamble<<6) | (send_write_cmd.Opcode));
		//printf("Write u16Temp=%04x\n",u16Temp);
		MICOM_REG(REG_HK_TO_MICOM_MB_0) = u16Temp;

		u16Count = MAILBOX_TIMEOUT;
		//polling interrput status
		//while( !(XBYTE[0x0e05] & BIT1) )//wait for flag of interrupt status ready
		while( !(  MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) & BIT9   ) )
		{
			msleep(1);
			if((u16Count--) == 0)
			{
				printf("u16Count == 0 [CtrlWrite] timeout\n");
				break;
			}
		}
		recv_write_cmd.Preamble = (U8)((MICOM_REG(REG_MICOM_TO_HK_MB_0) & 0x00C0)>>6 );
		recv_write_cmd.Opcode = (U8)MICOM_REG(REG_MICOM_TO_HK_MB_0);

		//Clear m4 (Read than Write for timing issue)
		M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
		//Clear IRQ
		MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;

		if( gPmCMD.Preamble == recv_write_cmd.Preamble \
				&& 0x3F == recv_write_cmd.Opcode )
		{
			;//do nothing
		}
		else
		{
			printf("MDrv_MICOM_PM_CtrlWrite FAIL\n");
			return FALSE;
		}
	}
	//up(&lite_sem);

	return TRUE;
}

/******************************************************************************/
///PM Control Read Function
///@param u8opcode \b IN  control command
///@param u8Data \b IN  8 bits data
///@param u16Data \b IN  16 bits data
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
/******************************************************************************/
B16 drvMICOM_CtrlRead(U8 u8CmdType, U8 u8DataNum, U8 *pu8Data )
{
	U8 u8Data = 0;
	U16 M4_status = 0;
	U16 u16Temp;
	U16 u16Count = 0;
	//B16 _bCmd_Flag = TRUE; //first time DDI command is allowed to fetch the data from MICOM
	U8 u8DataIndex = u8DataNum;
	PM_CMD_t   send_read_cmd, recv_read_cmd;

	send_read_cmd.Opcode = (CTRL_READ_CMD & 0x3F);

	while( u8DataIndex-- )
	{

		//printf("before gPmCMD.Preamble = %d\n", gPmCMD.Preamble);

		gPmCMD.Preamble++;

		//printf("After gPmCMD.Preamble = %d\n", gPmCMD.Preamble);

		//Clear m4 (Read than Write for timing issue)
		M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
		//Clear IRQ
		MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;

		//Mail Box 1
		//printf("Read u8DataIndex=%02x\n", u8DataIndex);  //Index of u8Data
		//printf("before Read pu8Data[%d]=%02x\n", u8DataIndex, pu8Data[u8DataIndex]);
		u16Temp = 0;    //memset Data byte of mail box
		u16Temp |= ((U16)u8DataIndex);   //H: Data (  ( ((U16)pu8Data[u8DataNum])<<8)   ), L: Index
		MICOM_REG(REG_HK_TO_MICOM_MB_1) = u16Temp;

		//Mail Box 0
		u16Temp = 0;// case CP_NO_CMD = 0
		//if( TRUE == _bCmd_Flag )    //get the first time sended command data
		//{
		u16Temp |= ((U16)u8CmdType<<8);  //DDI Command Type
		//_bCmd_Flag = FALSE;
		//}
		u16Temp |= ((gPmCMD.Preamble<<6) | (send_read_cmd.Opcode));
		//printf("Read u16Temp=%04x\n",u16Temp);
		MICOM_REG(REG_HK_TO_MICOM_MB_0) = u16Temp;

		u16Count = MAILBOX_TIMEOUT;
		//polling interrput status
		while( !(  MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) & BIT9   ) )
		{
			msleep(1);
			if((u16Count--) == 0)
			{
				printf("u16Count == 0 [CtrlRead] timeout\n");
				break;
			}
		}
		recv_read_cmd.Preamble = (U8)((MICOM_REG(REG_MICOM_TO_HK_MB_0) & 0x00C0)>>6 );
		recv_read_cmd.Opcode = (U8)MICOM_REG(REG_MICOM_TO_HK_MB_0);
		u8Data = (U8)((MICOM_REG(REG_MICOM_TO_HK_MB_0) & 0xFF00)>>8);

		//Clear m4 (Read than Write for timing issue)
		M4_status = MICOM_REG(REG_MICOM_TO_HK_MB_STATUS);
		//Clear IRQ
		MICOM_REG(REG_MICOM_TO_HK_MB_STATUS) = BIT9;

		if( gPmCMD.Preamble == recv_read_cmd.Preamble \
				&& 0x3F == recv_read_cmd.Opcode )
		{
			pu8Data[u8DataIndex] = u8Data;
			//printf("pu8Data[%d]=%02x\n", u8DataIndex, pu8Data[u8DataIndex]);
		}
		else
		{
			printf("MDrv_MICOM_PM_CtrlRead FAIL\n");
			return FALSE;
		}

	}

	//up(&lite_sem);

	return TRUE;
}

//for I2C
void drvIIC_Init(void)
{
#if 0
	//set all pads (except SPI) as input
	CHIP_REG(REG_IIC_MODE) &= ~BIT15;
	//iic master mode is enabled, bit 9~8 set to 1
	CHIP_REG(REG_IIC_MODE) &= ~BIT9;
	CHIP_REG(REG_IIC_MODE) |= BIT8;

	// clock = (Xtal / 4T) / div
	IIC_REG(REG_IIC_CLK_SEL) = 3; // 28.8M / 4 / 16 = 450KHz
	IIC_REG(REG_IIC_CTRL) = 0x80; // enable IIC
#endif
	//set all pads (except SPI) as input
	CHIP_REG(REG_IIC_MODE) &= ~BIT15;
	//iic master mode is enabled, bit 9~8 set to 2
	CHIP_REG(REG_IIC_MODE) |= BIT9;
	CHIP_REG(REG_IIC_MODE) &= ~BIT8;

	// clock = (Xtal / 4T) / div
	IIC_REG(REG_IIC_CLK_SEL) = 3; // 28.8M / 4 / 16 = 450KHz
	IIC_REG(REG_IIC_CTRL) = 0x80; // enable IIC
}

void drvIIC_Start(void)
{
	//reset iic circuit
	IIC_REG(REG_IIC_STATUS) = 0x08;
	IIC_REG(REG_IIC_STATUS) = 0x00;

	//reset iic circuit
	IIC_REG(REG_IIC_CTRL) = 0xC0;
}

void drvIIC_Stop(void)
{
	IIC_REG(REG_IIC_CTRL) = 0xA0;
	mdelay(5);
}

void drvIIC_NoAck(void)
{
	IIC_REG(REG_IIC_CTRL) = 0x90;
}

B16 drvIIC_SendData(U8 u8DataIIC)
{
	IIC_REG(REG_IIC_STATUS) = 0x02; /* clear interrupt */
	IIC_REG(REG_IIC_WDATA) = u8DataIIC;

#if 0
	{   //tune the dalay
		U8 u8Delay;
		for(u8Delay=0; u8Delay<100; u8Delay++);
		udelay(1000);
	}
#endif

	while (1)
	{
		if (IIC_REG(REG_IIC_STATUS) & 0x01)
		{
			IIC_REG(REG_IIC_STATUS) = 0x02; /* clear interrupt */
			if (IIC_REG(REG_IIC_CTRL) & 0x08) /* no ACK */
			{
				return FALSE;
			}
			else
			{
				udelay(1);
				return TRUE;
			}
		}
	}

	return FALSE;
}

B16 drvIIC_SendByte(U8 u8DataIIC)
{
	U8 u8I;

	for(u8I=0;u8I<10;u8I++)
	{
		if(drvIIC_SendData(u8DataIIC) == TRUE)
			return TRUE;
	}
	return FALSE;
}

B16 drvIIC_GetByte(U8* pu8DataIIC) /* auto generate ACK */
{
	/* clear interrupt & start byte reading */
	IIC_REG(REG_IIC_STATUS) = 0x04;

#if 0
	{   //tune the dalay
		U8 u8Delay;
		for(u8Delay=0; u8Delay<100; u8Delay++);
		udelay(1000);
	}
#endif

	while(1)
	{
		if (IIC_REG(REG_IIC_STATUS))
		{
			IIC_REG(REG_IIC_STATUS) = 0x02; /* clear interrupt */
			*pu8DataIIC = IIC_REG(REG_IIC_RDATA);
			udelay(1);
			return TRUE;
		}
	}

	return FALSE;
}

S32 drvIIC_Write(U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC)
{
	U32     u32I;
	U8      u8I;
	B16     bReturnIIC = TRUE;
	S32     s32RetCountIIC = -1;

	drvIIC_Start();

	if (FALSE == drvIIC_SendByte(u8SlaveIdIIC&0xFE))
	{
		bReturnIIC = FALSE;
		s32RetCountIIC = -2;
		goto HW_IIC_Write_End;
	}

	for (u8I = 0; u8I < u8AddrSizeIIC; u8I++)
	{
		if (FALSE == drvIIC_SendByte(pu8AddrIIC[u8I]))
		{
			bReturnIIC = FALSE;
			s32RetCountIIC = -3;
			goto HW_IIC_Write_End;
		}
	}

	for (u32I = 0; u32I < u32BufSizeIIC; u32I++)
	{
		if (FALSE == drvIIC_SendByte(pu8BufIIC[u32I]))
		{
			bReturnIIC = FALSE;
			s32RetCountIIC = -4;
			goto HW_IIC_Write_End;
		}
	}

	s32RetCountIIC = u32BufSizeIIC;

HW_IIC_Write_End:
	drvIIC_Stop();
	return s32RetCountIIC;
}

S32 drvIIC_Read(U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC)
{
	U32     u32I;
	U8      u8I;
	B16     bReturnIIC = TRUE;
	S32     s32RetCountIIC = -1;

	drvIIC_Start();

	if (FALSE == drvIIC_SendByte(u8SlaveIdIIC&0xFE))
	{
		bReturnIIC = FALSE;
		s32RetCountIIC = -2;
		goto HW_IIC_Read_End;
	}

	for (u8I = 0; u8I < u8AddrSizeIIC; u8I++)
	{
		if (FALSE == drvIIC_SendByte(pu8AddrIIC[u8I]))
		{
			bReturnIIC = FALSE;
			s32RetCountIIC = -3;
			goto HW_IIC_Read_End;
		}
	}

	drvIIC_Start();

	if (FALSE == drvIIC_SendByte(u8SlaveIdIIC|0x1))
	{
		bReturnIIC = FALSE;
		s32RetCountIIC = -4;
		goto HW_IIC_Read_End;
	}

	for (u32I = 0; u32I < u32BufSizeIIC; u32I++)
	{
		if (u32I == (u32BufSizeIIC-1))
		{
			drvIIC_NoAck();
		}

		if (FALSE == drvIIC_GetByte(&pu8BufIIC[u32I]))
		{
			bReturnIIC = FALSE;
			s32RetCountIIC = -5;
			goto HW_IIC_Read_End;
		}
	}

	s32RetCountIIC = u32BufSizeIIC;

HW_IIC_Read_End:
	drvIIC_Stop();
	return s32RetCountIIC;
}
