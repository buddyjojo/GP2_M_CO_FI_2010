#include <common.h>

#if defined(CONFIG_TITANIA2) || defined(CONFIG_TITANIA3_FPGA) ||defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
#include "titania_sw_i2c.h"

// for SW I2C
#define _INPUT 			   						1
#define _OUTPUT			   						0
#define _HIGH  			   						1
#define _LOW   			   						0
#define SWIIC_READ 		   						0
#define SWIIC_WRITE		   						1
#define I2C_CHECK_PIN_DUMMY						200//100
#define I2C_ACKNOWLEDGE    						_LOW
#define I2C_NON_ACKNOWLEDGE						_HIGH
#define I2C_ACCESS_DUMMY_TIME					5//3


#define SWIIC_SCL_PIN(x) \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SclOenReg) = (x == _INPUT) ? \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SclOenReg) |  _I2CBus[u8BusSel-2].SclOenBit) : \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SclOenReg) & ~_I2CBus[u8BusSel-2].SclOenBit) )

#define SWIIC_SDA_PIN(x) \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SdaOenReg) = (x == _INPUT) ? \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SdaOenReg) |  _I2CBus[u8BusSel-2].SdaOenBit) : \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SdaOenReg) & ~_I2CBus[u8BusSel-2].SdaOenBit) )

#define SWIIC_SCL_OUT(x) \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SclOutReg) = (x == _HIGH) ? \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SclOutReg) |  _I2CBus[u8BusSel-2].SclOutBit) : \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SclOutReg) & ~_I2CBus[u8BusSel-2].SclOutBit) )

#define SWIIC_SDA_OUT(x) \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SdaOutReg) = (x == _HIGH) ? \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SdaOutReg) |  _I2CBus[u8BusSel-2].SdaOutBit) : \
	(MHal_GPIO_REG(_I2CBus[u8BusSel-2].SdaOutReg) & ~_I2CBus[u8BusSel-2].SdaOutBit) )

#define GET_SWIIC_SCL() \
	( (MHal_GPIO_REG(_I2CBus[u8BusSel-2].SclInReg) & _I2CBus[u8BusSel-2].SclInBit) ? _HIGH : _LOW )

#define GET_SWIIC_SDA() \
	( (MHal_GPIO_REG(_I2CBus[u8BusSel-2].SdaInReg) & _I2CBus[u8BusSel-2].SdaInBit) ? _HIGH : _LOW )

#define _SDA_HIGH()     SWIIC_SDA_PIN( _INPUT )
#define _SDA_LOW()      do { SWIIC_SDA_OUT( _LOW ); SWIIC_SDA_PIN( _OUTPUT ); } while(0)

#define _SCL_HIGH()     SWIIC_SCL_PIN( _INPUT )
#define _SCL_LOW()      do { SWIIC_SCL_OUT( _LOW ); SWIIC_SCL_PIN( _OUTPUT ); } while(0)

//redcloud00@lge.com
#define REG_I2S_IN_WS_OEN            (0x101e6e)
#define REG_I2S_IN_BCK_OEN            (0x101e6e)

#define REG_I2S_IN_WS_IN            (0x101e6a)
#define REG_I2S_IN_BCK_IN            (0x101e6a)
#define REG_I2S_IN_WS_OUT            (0x101e6c)
#define REG_I2S_IN_BCK_OUT            (0x101e6c)

IIC_Bus_t _I2CBus[] =
{
/*CH7*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
	 REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 5},
/*CH7*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
     REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 5},
/*CH7*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
     REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 5},
/*CH7*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
     REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 5},
/*CH7*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
     REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 5},
/*CH7*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
	 REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 5},
};

static U8 u8BusSel;
static U8 _u8SwIicDly2;
static U8 _u8SwIicDly3;
static U8 _u8SwIicDly4;
static U8 _u8SwIicDly5;
static U8 _u8SwIicDly6;

void MDrv_SW_IIC_Init(void);//redcloud00@lge.com
void MDrv_SW_IIC_SCL(U8 u8Data);
void MDrv_SW_IIC_SDA(U8 u8Data);
void MDrv_SW_IIC_SCL_Chk(B16 bSet);
void MDrv_SW_IIC_SDA_Chk(B16 bSet);
B16 MDrv_SW_IIC_Start(void);
void MDrv_SW_IIC_Stop(void);
B16 MDrv_IIC_SendByte(U8 u8data);
B16 MDrv_SW_IIC_SendByte(U8 u8data);
B16 MDrv_SW_IIC_AccessStart(U8 u8SlaveAdr, U8 u8Trans);
U8 MDrv_SW_IIC_GetByte (U16  u16Ack);
S32 MDrv_SW_IIC_Write(U8 u8SlaveID, U8* pu8Addr, U8 u8AddrCnt, U8* pu8Buf, U32 u32BufLen);
S32 MDrv_SW_IIC_Read(U8 u8SlaveID, U8* pu8Addr, U8 u8AddrCnt, U8* pu8Buf, U32 u32BufLen);

void i2c_sw_init (void) //(int speed, int slaveadd) //redcloud00@lge.com
{
	printf("sw i2c_init~~ \n\n");
	MDrv_SW_IIC_Init();//redcloud00@lge.com
}

#if 0 //redcloud00@lge.com no use
static int i2c_read_byte (u8 devaddr, u8 regoffset, u8 * value)
{

}

static int i2c_write_byte (u8 devaddr, u8 regoffset, u8 value)
{

}

int i2c_probe (uchar chip)
{

}
#endif 

int i2c_read (uchar chip, uint * addr, int alen, uchar * buffer, int len)
{
	MDrv_SW_IIC_Read(chip, addr, alen, buffer, len);	
}

int i2c_write (uchar chip, uint * addr, int alen, uchar * buffer, int len)
{
	MDrv_SW_IIC_Write(chip, addr, alen, buffer, len);
}

void MDrv_SW_IIC_Init(void) //redcloud00@lge.com
{
#if 0

#else
	//set all pads(except SPI) as input
	MHal_GPIO_REG(REG_ALL_PAD_IN) &= ~BIT7;
	//for channel 2 set
	MHal_GPIO_REG(REG_DDCR_CK_SET) |= BIT7;
	MHal_GPIO_REG(REG_DDCR_DA_SET) |= BIT7;
	_u8SwIicDly2 = _I2CBus[0].DefDelay;
	//for channel 3 set
	_u8SwIicDly3 = _I2CBus[1].DefDelay;
	//for channel 4 set
	_u8SwIicDly4 = _I2CBus[2].DefDelay;
	//for channel 5 set
	MHal_GPIO_REG(REG_PCM2_IRQA_N_SET) |= BIT5;
	MHal_GPIO_REG(REG_PCM2_IRQA_N_SET) |= BIT4;
	MHal_GPIO_REG(REG_PCM2_WAIT_N_SET) |= BIT5;
	MHal_GPIO_REG(REG_PCM2_WAIT_N_SET) |= BIT4;
	_u8SwIicDly5 = _I2CBus[3].DefDelay;
	//for channel 6 set
	_u8SwIicDly6 = _I2CBus[4].DefDelay;
#endif	
}

void MDrv_SW_IIC_SCL(U8 u8Data)
{
	if ( u8Data == _HIGH )
	{
		SWIIC_SCL_PIN(_INPUT);  //set to input
	}
	else
	{
		SWIIC_SCL_OUT(_LOW);
		SWIIC_SCL_PIN(_OUTPUT);
	}
}

void MDrv_SW_IIC_SDA(U8 u8Data)
{
	if ( u8Data == _HIGH )
	{
		SWIIC_SDA_PIN(_INPUT);  //set to input
	}
	else
	{
		SWIIC_SDA_OUT(_LOW);
		SWIIC_SDA_PIN(_OUTPUT); //set to output
	}
}

void MDrv_SW_IIC_SCL_Chk(B16 bSet)
{
	U8 u8Dummy;       // loop dummy

	if (bSet == _HIGH)  // if set pin high
	{
		SWIIC_SCL_PIN(_INPUT);
		u8Dummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
		while ((GET_SWIIC_SCL() == _LOW) && (u8Dummy--)) ; // check SCL pull high
	}
	else
	{
		MDrv_SW_IIC_SCL(_LOW);    // set SCL pin
		SWIIC_SCL_PIN(_OUTPUT);
	}
}

void MDrv_SW_IIC_SDA_Chk(B16 bSet)
{
	U8 u8Dummy;       // loop dummy

	if (bSet == _HIGH)  // if set pin high
	{
		SWIIC_SDA_PIN(_INPUT);
		u8Dummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
		while ((GET_SWIIC_SDA() == _LOW) && (u8Dummy--)) ; // check SDA pull high
	}
	else
	{
		MDrv_SW_IIC_SDA(_LOW);    // set SDA pin
		SWIIC_SDA_PIN(_OUTPUT);
	}
}

//-------------------------------------------------------------------------------------------------
// SW I2C: start signal.
// <comment>
//  SCL ________
//              \_________
//  SDA _____
//           \____________
//
// Return value: None
//-------------------------------------------------------------------------------------------------
B16 MDrv_SW_IIC_Start(void)
{
	B16 bStatus = TRUE;    // success status

	MDrv_SW_IIC_SDA_Chk(_HIGH);
	//MDrv_SW_IIC_Delay();

	MDrv_SW_IIC_SCL_Chk(_HIGH);
	//MDrv_SW_IIC_Delay();

	// check pin error
	SWIIC_SCL_PIN(_INPUT);
	SWIIC_SDA_PIN(_INPUT);

	if ((GET_SWIIC_SCL() == _LOW) || (GET_SWIIC_SDA() == _LOW))
	{
		SWIIC_SCL_PIN(_OUTPUT);
		SWIIC_SDA_PIN(_OUTPUT);
		bStatus = FALSE;
	}
	else // success
	{
		MDrv_SW_IIC_SDA(_LOW);
		//MDrv_SW_IIC_Delay();
		MDrv_SW_IIC_SCL(_LOW);
	}

	return bStatus;	 //vain
}

//-------------------------------------------------------------------------------------------------
// SW I2C: stop signal.
// <comment>
//              ____________
//  SCL _______/
//                 _________
//  SDA __________/
//-------------------------------------------------------------------------------------------------
void MDrv_SW_IIC_Stop(void)
{
	_SCL_LOW();

	//MDrv_SW_IIC_Delay();
	_SDA_LOW();

	//MDrv_SW_IIC_Delay();
	_SCL_HIGH();
	//MDrv_SW_IIC_Delay();
	_SDA_HIGH();
	//MDrv_SW_IIC_Delay();
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Send 1 bytes data
///@param u8data \b IN: 1 byte data to send
//-------------------------------------------------------------------------------------------------
B16 MDrv_IIC_SendByte(U8 u8data)   // Be used int IIC_SendByte
{
	U8      u8Mask = 0x80;
	B16     bAck; // acknowledge bit

	while ( u8Mask )
	{
		if (u8data & u8Mask)
		{
			MDrv_SW_IIC_SDA_Chk(_HIGH);
		}
		else
		{
			MDrv_SW_IIC_SDA_Chk(_LOW);
		}

		//MDrv_SW_IIC_Delay();
		MDrv_SW_IIC_SCL_Chk(_HIGH); // clock
		//MDrv_SW_IIC_Delay();
		MDrv_SW_IIC_SCL(_LOW);

		//MDrv_SW_IIC_Delay();

		u8Mask >>= 1; // next
	}

	// recieve acknowledge
	SWIIC_SDA_PIN(_INPUT);
	//MDrv_SW_IIC_Delay();
	MDrv_SW_IIC_SCL_Chk(_HIGH);

	//MDrv_SW_IIC_Delay();
	bAck = GET_SWIIC_SDA(); // recieve acknowlege
	//SWIIC_SDA(bAck);     //for I2c waveform sharp
	//SWIIC_SDA_PIN(_OUTPUT);
	MDrv_SW_IIC_SCL(_LOW);

	//MDrv_SW_IIC_Delay();

	MDrv_SW_IIC_SDA(bAck);     //for I2c waveform sharp
	SWIIC_SDA_PIN(_OUTPUT);

	//MDrv_SW_IIC_Delay();
	//MDrv_SW_IIC_Delay();
	//MDrv_SW_IIC_Delay();
	//MDrv_SW_IIC_Delay();

	return (bAck);
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Send 1 bytes data, this function will retry 5 times until success.
///@param u8data \b IN: 1 byte data to send
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
B16 MDrv_SW_IIC_SendByte(U8 u8data)
{
	U8 u8I;

	for(u8I=0;u8I<5;u8I++)
	{
		if (MDrv_IIC_SendByte(u8data) == I2C_ACKNOWLEDGE)
			return TRUE;
	}

	//printk("IIC write byte 0x%02x fail!!\n", u8data);
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
// SW I2C: access start.
//
// Arguments: u8SlaveAdr - slave address
//            u8Trans - I2C_TRANS_WRITE/I2C_TRANS_READ
//-------------------------------------------------------------------------------------------------
B16 MDrv_SW_IIC_AccessStart(U8 u8SlaveAdr, U8 u8Trans)
{
	U8 u8Dummy; // loop dummy

	if (u8Trans == SWIIC_READ) // check i2c read or write
	{
		u8SlaveAdr = u8SlaveAdr | 0x01; // read
	}
	else
	{
		u8SlaveAdr = u8SlaveAdr & 0xfe; // write
	}

	u8Dummy = I2C_ACCESS_DUMMY_TIME;

	while (u8Dummy--)
	{
		if ( MDrv_SW_IIC_Start() == FALSE)
		{
			continue;
		}

		if ( MDrv_SW_IIC_SendByte(u8SlaveAdr) == TRUE )   //I2C_ACKNOWLEDGE) // check acknowledge
		{
			return TRUE;
		}

		MDrv_SW_IIC_Stop();
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Get 1 bytes data
///@param u16Ack  \b IN: acknowledge
///@return U8:    \b OUT: get data from the device
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
U8 MDrv_SW_IIC_GetByte (U16  u16Ack)
{
	U8 u8Receive = 0;
	U8 u8Mask = 0x80;

	SWIIC_SDA_PIN(_INPUT);		  //SWIIC_SDA_PIN

	while ( u8Mask )
	{
		//MDrv_SW_IIC_Delay();
		MDrv_SW_IIC_SCL_Chk(_HIGH);		//SWIIC_SCL_Chk
		//MDrv_SW_IIC_Delay();

		if (GET_SWIIC_SDA() == _HIGH)	  //GET_SWIIC_SDA
		{
			u8Receive |= u8Mask;
		}
		u8Mask >>= 1; // next

		MDrv_SW_IIC_SCL(_LOW);	  //SWIIC_SCL
		//SWIIC_Delay();

	}
	if (u16Ack)
	{
		// acknowledge
		MDrv_SW_IIC_SDA_Chk(I2C_ACKNOWLEDGE);	  //SWIIC_SDA_Chk
	}
	else
	{
		// non-acknowledge
		MDrv_SW_IIC_SDA_Chk(I2C_NON_ACKNOWLEDGE);
	}

	//MDrv_SW_IIC_Delay();
	MDrv_SW_IIC_SCL_Chk(_HIGH);	//SWIIC_SCL_Chk
	//MDrv_SW_IIC_Delay();
	MDrv_SW_IIC_SCL(_LOW);		 //SWIIC_SCL
	//MDrv_SW_IIC_Delay();

	//MDrv_SW_IIC_Delay();
	//MDrv_SW_IIC_Delay();

	return u8Receive;
}

S32 MDrv_SW_IIC_Write(U8 u8SlaveID, U8* pu8Addr, U8 u8AddrCnt, U8* pu8Buf, U32 u32BufLen)
{
	u8BusSel = 7;
	U8  u8Dummy; // loop dummy
	S32 s32RetCountIIC;

	u8Dummy = 1;  //I2C_ACCESS_DUMMY_TIME;
	s32RetCountIIC = u32BufLen;

	while (u8Dummy--)
	{
		if (MDrv_SW_IIC_AccessStart(u8SlaveID, SWIIC_WRITE) == FALSE)
		{
			s32RetCountIIC = -2;
			goto SW_IIC_Write_End;
		}

		while( 	u8AddrCnt )
		{
			u8AddrCnt--;
			if ( MDrv_SW_IIC_SendByte( *pu8Addr ) == FALSE )
			{
				s32RetCountIIC = -3;
				goto SW_IIC_Write_End;
			}
			pu8Addr++;
		}
		while (u32BufLen ) // loop of writting data
		{
			u32BufLen-- ;
			//IIC_SendByte(*pu8Buf); // send byte
			if ( MDrv_SW_IIC_SendByte( *pu8Buf ) == FALSE )
			{
				s32RetCountIIC = -4;
				goto SW_IIC_Write_End;
			}
			pu8Buf++; // next byte pointer
		}

		break;
	}

SW_IIC_Write_End:
	MDrv_SW_IIC_Stop();

	return s32RetCountIIC;
}

S32 MDrv_SW_IIC_Read(U8 u8SlaveID, U8* pu8Addr, U8 u8AddrCnt, U8* pu8Buf, U32 u32BufLen)
{
	u8BusSel = 7;
	U8  u8Dummy; // loop dummy
	S32 s32RetCountIIC;

	u8Dummy = I2C_ACCESS_DUMMY_TIME;
	s32RetCountIIC = u32BufLen;

	while (u8Dummy--)
	{
		if (MDrv_SW_IIC_AccessStart(u8SlaveID, SWIIC_WRITE) == FALSE)
		{
			s32RetCountIIC = -2;
			goto SW_IIC_Read_End;
		}

		while( 	u8AddrCnt )
		{
			u8AddrCnt--;
			if (MDrv_SW_IIC_SendByte( *pu8Addr ) == FALSE)
			{
				s32RetCountIIC = -3;
				goto SW_IIC_Read_End;
			}
			pu8Addr++;
		}

		if (MDrv_SW_IIC_AccessStart(u8SlaveID, SWIIC_READ) == FALSE)
		{
			s32RetCountIIC = -4;
			goto SW_IIC_Read_End;
		}

		while (u32BufLen--) // loop to burst read
		{
			*pu8Buf = MDrv_SW_IIC_GetByte(u32BufLen); // receive byte

			pu8Buf++; // next byte pointer
		}

		break;
	}

SW_IIC_Read_End:
	MDrv_SW_IIC_Stop();

	return s32RetCountIIC;
}

#endif //ifdef CONFIG_TITANIA
