
#include <common.h>

//#ifdef CONFIG_TITANIA
//#ifdef CONFIG_TITANIA_FPGA
//2008/07/29 Nick add start
#if defined(CONFIG_TITANIA3) ||defined(CONFIG_EUCLID) || defined(CONFIG_TITANIA)|| defined(CONFIG_TITANIA2)||defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_GP2_DEMO1) ||defined(CONFIG_EUCLID)
//2008/07/29 Nick add end
#include "titania_hw_i2c.h"

#define	IIC_NUM_OF_MAX				(14)
#define	IIC_NUM_OF_HW				(1)

#define IIC_NUM_OF_NEC_MICOM		(7) //(3)
#define IIC_NUM_OF_AUDIO_AMP		(4)



#define IIC_NUM_OF_RGB_EDID			(8)
#define IIC_NUM_OF_HDMI_A_EDID		(9)
#define IIC_NUM_OF_HDMI_B_EDID		(10)
#define IIC_NUM_OF_HDMI_C_EDID		(11)
#define IIC_NUM_OF_HDMI_D_EDID		(12)

// for SW I2C
#define _INPUT  1
#define _OUTPUT 0
#define _HIGH   1
#define _LOW    0
#define SWIIC_READ      0
#define SWIIC_WRITE     1
#define I2C_CHECK_PIN_DUMMY     0xffff //200//100
#define I2C_ACKNOWLEDGE         _LOW
#define I2C_NON_ACKNOWLEDGE     _HIGH
#define I2C_ACCESS_DUMMY_TIME   5//3

#define SWIIC_SCL_PIN(chNum, x) \
        ( MHal_GPIO_REG(_I2CBus[chNum-2].SclOenReg) = (x == _INPUT) ? \
         (MHal_GPIO_REG(_I2CBus[chNum-2].SclOenReg) |  _I2CBus[chNum-2].SclOenBit) : \
         (MHal_GPIO_REG(_I2CBus[chNum-2].SclOenReg) & ~_I2CBus[chNum-2].SclOenBit) )

#define SWIIC_SDA_PIN(chNum, x) \
        ( MHal_GPIO_REG(_I2CBus[chNum-2].SdaOenReg) = (x == _INPUT) ? \
         (MHal_GPIO_REG(_I2CBus[chNum-2].SdaOenReg) |  _I2CBus[chNum-2].SdaOenBit) : \
         (MHal_GPIO_REG(_I2CBus[chNum-2].SdaOenReg) & ~_I2CBus[chNum-2].SdaOenBit) )

#define SWIIC_SCL_OUT(chNum, x) \
        ( MHal_GPIO_REG(_I2CBus[chNum-2].SclOutReg) = (x == _HIGH) ? \
         (MHal_GPIO_REG(_I2CBus[chNum-2].SclOutReg) |  _I2CBus[chNum-2].SclOutBit) : \
         (MHal_GPIO_REG(_I2CBus[chNum-2].SclOutReg) & ~_I2CBus[chNum-2].SclOutBit) )

#define SWIIC_SDA_OUT(chNum, x) \
        ( MHal_GPIO_REG(_I2CBus[chNum-2].SdaOutReg) = (x == _HIGH) ? \
         (MHal_GPIO_REG(_I2CBus[chNum-2].SdaOutReg) |  _I2CBus[chNum-2].SdaOutBit) : \
         (MHal_GPIO_REG(_I2CBus[chNum-2].SdaOutReg) & ~_I2CBus[chNum-2].SdaOutBit) )

#define GET_SWIIC_SCL(chNum) \
        ( (MHal_GPIO_REG(_I2CBus[chNum-2].SclInReg) & _I2CBus[chNum-2].SclInBit) ? _HIGH : _LOW )

#define GET_SWIIC_SDA(chNum) \
        ( (MHal_GPIO_REG(_I2CBus[chNum-2].SdaInReg) & _I2CBus[chNum-2].SdaInBit) ? _HIGH : _LOW )

#define SWII_DELAY(chNum)    (_I2CBus[chNum-2].DefDelay)// added for removing _u8SwIicDlyX by LGE(dreamer@lge.com)


#define _SDA_HIGH(chNum)     SWIIC_SDA_PIN(chNum, _INPUT)
#define _SDA_LOW(chNum)     do { SWIIC_SDA_OUT(chNum, _LOW); SWIIC_SDA_PIN(chNum, _OUTPUT); } while(0)

#define _SCL_HIGH(chNum)     SWIIC_SCL_PIN(chNum, _INPUT)
#define _SCL_LOW(chNum)     do { SWIIC_SCL_OUT(chNum, _LOW); SWIIC_SCL_PIN(chNum, _OUTPUT); } while(0)






//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define IIC_MUTEX_WAIT_TIME         3000

#define IIC_DBG_ENABLE              1

#if IIC_DBG_ENABLE
#define IIC_DBG(_f)                 (_f)
#else
#define IIC_DBG(_f)
#endif

#define IIC_MODULE_KERNAL_NAME       "/dev/iic"
#define MOD_IIC_DEVICE_COUNT         1
#define MOD_IIC_NAME                 "ModIIC"

#define IIC_UNIT_NUM               2

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static S32 s32FdIIC = 0;
static U8 u8InitIIC = FALSE;
static U8 u8ForceChIIC =  0;


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
//unsigned int u32CpuCount;
//unsigned int u32SwIICdelayCycle;


IIC_Bus_t _I2CBus[] =
{
/*CH2*/{REG_DDCR_CK_OEN, BIT4, REG_DDCR_CK_OUT, BIT4, REG_DDCR_CK_IN, BIT4, \
     REG_DDCR_DA_OEN, BIT3, REG_DDCR_DA_OUT, BIT3, REG_DDCR_DA_IN, BIT3, 5},


/*CH3*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
     REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 5},
/*CH4*/{REG_TGPIO0_OEN, BIT5, REG_TGPIO0_OUT, BIT5, REG_TGPIO0_IN, BIT5, \
    REG_TGPIO1_OEN, BIT6, REG_TGPIO1_OUT, BIT6, REG_TGPIO1_IN, BIT6, 5},

/*CH5*/{REG_TGPIO0_OEN, BIT5, REG_TGPIO0_OUT, BIT5, REG_TGPIO0_IN, BIT5, \
    REG_TGPIO1_OEN, BIT6, REG_TGPIO1_OUT, BIT6, REG_TGPIO1_IN, BIT6, 5},

/*CH6*/{REG_TGPIO2_OEN, BIT7, REG_TGPIO2_OUT, BIT7, REG_TGPIO2_IN, BIT7, \
    REG_TGPIO3_OEN, BIT0, REG_TGPIO3_OUT, BIT0, REG_TGPIO3_IN, BIT0, 5},


/*CH7*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
     REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 5},

/*CH8 : RGB - EDID */
    {REG_DDCA_CK_OEN, BIT1, REG_DDCA_CK_OUT, BIT2, REG_DDCA_CK_IN, BIT0, \
     REG_DDCA_DA_OEN, BIT5, REG_DDCA_DA_OUT, BIT6, REG_DDCA_DA_IN, BIT4, 5},

/*CH9 : HDMI port A - EDID */
    {REG_DDCDA_CK_OEN, BIT1, REG_DDCDA_CK_OUT, BIT2, REG_DDCDA_CK_IN, BIT0, \
     REG_DDCDA_DA_OEN, BIT5, REG_DDCDA_DA_OUT, BIT6, REG_DDCDA_DA_IN, BIT4, 5},
/*CH10 : HDMI port B - EDID */
    {REG_DDCDB_CK_OEN, BIT1, REG_DDCDB_CK_OUT, BIT2, REG_DDCDB_CK_IN, BIT0, \
     REG_DDCDB_DA_OEN, BIT5, REG_DDCDB_DA_OUT, BIT6, REG_DDCDB_DA_IN, BIT4, 5},
/*CH11 : HDMI port C - EDID */
    {REG_DDCDC_CK_OEN, BIT1, REG_DDCDC_CK_OUT, BIT2, REG_DDCDC_CK_IN, BIT0, \
     REG_DDCDC_DA_OEN, BIT5, REG_DDCDC_DA_OUT, BIT6, REG_DDCDC_DA_IN, BIT4, 5},


/*CH12 : HDMI port D - EDID */
    {REG_DDCDD_CK_OEN, BIT1, REG_DDCDD_CK_OUT, BIT2, REG_DDCDD_CK_IN, BIT0, \
     REG_DDCDD_DA_OEN, BIT5, REG_DDCDD_DA_OUT, BIT6, REG_DDCDD_DA_IN, BIT4, 5},



};

// for software IIC
void MDrv_SW_IIC_SetSpeed(U8 u8ChIIC, U8 u8Speed);
void MDrv_SW_IIC_Delay(U8 u8ChIIC);
void MDrv_SW_IIC_SCL(U8 u8ChIIC, U8 u8Data);
void MDrv_SW_IIC_SDA(U8 u8ChIIC, U8 u8Data);
void MDrv_SW_IIC_SCL_Chk(U8 u8ChIIC, B16 bSet);
void MDrv_SW_IIC_SDA_Chk(U8 u8ChIIC, B16 bSet);
B16 MDrv_SW_IIC_Start(U8 u8ChIIC);
void MDrv_SW_IIC_Stop(U8 u8ChIIC);
B16 MDrv_IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack);
B16 MDrv_SW_IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack);
B16 MDrv_SW_IIC_AccessStart(U8 u8ChIIC, U8 u8SlaveAdr, U8 u8Trans);
U8 MDrv_SW_IIC_GetByte (U8 u8ChIIC, U16 u16Ack);

void MDrv_SW_IIC_Enable( U8 u8ChIIC, U8 bEnable );	//	added for RGB EDID by LGE(dreamer@lge.com)
//static U8 u8BusSel;
//static U8 _u8SwIicDly2;
//static U8 _u8SwIicDly3;
//static U8 _u8SwIicDly4;
//static U8 _u8SwIicDly5;
//static U8 _u8SwIicDly6;
//extern void MDrv_IIC_Init(void);
//extern S32 MDrv_HW_IIC_Write(U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC);
//extern S32 MDrv_HW_IIC_Read(U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC);
void MDrv_IIC_Init(void);

void i2c_init(void)
{
    MDrv_IIC_Init();
}

#if 0
static int i2c_read_byte (u8 devaddr, u8 regoffset, u8 * value)
{

}

static int i2c_write_byte (u8 devaddr, u8 regoffset, u8 value)
{

}
#endif

int i2c_probe (uchar chip)
{
   return 0;
}

//int i2c_read (uchar chip, uint * addr, int alen, uchar * buffer, int len)
void i2c_read (uchar channel,uchar chip, uint * addr, uchar alen, uchar * buffer, uint len)
{
    if(channel==1)
    {
        MDrv_HW_IIC_Read(chip, alen, (U8*)addr, len, buffer);

    }
    else
    {
        MDrv_SW_IIC_Read(channel, chip, alen, (U8*)addr,len,buffer);
    }
}

//int i2c_write (uchar chip, uint * addr, int alen, uchar * buffer, int len)
void i2c_write (uchar channel,uchar chip, uchar * addr, uchar alen, uchar * buffer, uint len)
{	
    if(channel==1)
    {
        //MDrv_HW_IIC_Write(chip, addr, alen, buffer, len);
        MDrv_HW_IIC_Write(chip, alen, addr, len, buffer);

    }
    else
    {

       // MDrv_SW_IIC_Write(channel,chip, addr, alen, buffer, len);
		MDrv_SW_IIC_Write(channel, chip, alen, addr, len, buffer);	  

    }
}
void MDrv_SW_IIC_SetSpeed(U8 u8ChIIC, U8 u8Speed)
{
// added for removing _u8SwIicDly2 by LGE(dreamer@lge.com)
    if( (u8ChIIC > IIC_NUM_OF_HW) && (u8ChIIC <= IIC_NUM_OF_MAX) )
    {
        SWII_DELAY(u8ChIIC) = u8Speed;
    }
}

void MDrv_SW_IIC_Delay(U8 u8ChIIC)
{
    U8 u8I;
    U8 u8Loop = 5;

// added for removing _u8SwIicDly2 by LGE(dreamer@lge.com)
    if( (u8ChIIC > IIC_NUM_OF_HW) && (u8ChIIC <= IIC_NUM_OF_MAX) )
    {
        u8Loop = SWII_DELAY(u8ChIIC);
    }

    //for fast mode
    if(u8Loop == 2)
        for (u8I=0;u8I<150;u8I++)
            ;
    else
    {
		if(u8ChIIC == IIC_NUM_OF_NEC_MICOM)//slow mode for ext micom
		{
			udelay(9);
		}
		else //normal mode
		{       
         	udelay(4);//(6);//19us        
		} 

    }
}


void MDrv_SW_IIC_SCL(U8 u8ChIIC, U8 u8Data)
{
    if ( u8Data == _HIGH )
    {
        SWIIC_SCL_PIN(u8ChIIC, _INPUT);  //set to input
    }
    else
    {
        SWIIC_SCL_OUT(u8ChIIC, _LOW);
        SWIIC_SCL_PIN(u8ChIIC, _OUTPUT);
    }
}

void MDrv_SW_IIC_SDA(U8 u8ChIIC, U8 u8Data)
{
    if ( u8Data == _HIGH )
    {
        SWIIC_SDA_PIN(u8ChIIC, _INPUT);  //set to input
    }
    else
    {
        SWIIC_SDA_OUT(u8ChIIC, _LOW);
        SWIIC_SDA_PIN(u8ChIIC, _OUTPUT); //set to output
    }
}

void MDrv_SW_IIC_SCL_Chk(U8 u8ChIIC, B16 bSet)
{
    U16 u8Dummy;       // loop dummy

    if (bSet == _HIGH)  // if set pin high
    {
        SWIIC_SCL_PIN(u8ChIIC, _INPUT);
        u8Dummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
        while ((GET_SWIIC_SCL(u8ChIIC) == _LOW) && (u8Dummy--))
		{
			udelay(1);
		}; // check SCL pull high
    }
    else
    {
        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);    // set SCL pin
        SWIIC_SCL_PIN(u8ChIIC, _OUTPUT);
    }
}

void MDrv_SW_IIC_SDA_Chk(U8 u8ChIIC, B16 bSet)
{
    U16 u8Dummy;       // loop dummy

    if (bSet == _HIGH)  // if set pin high
    {
        SWIIC_SDA_PIN(u8ChIIC, _INPUT);
        u8Dummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
        while ((GET_SWIIC_SDA(u8ChIIC) == _LOW) && (u8Dummy--))
		{
			udelay(1);
		}; // check SDA pull high		
    }
    else
    {
        udelay(2);
		SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);
        MDrv_SW_IIC_SDA(u8ChIIC, _LOW);    // set SDA pin
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
B16 MDrv_SW_IIC_Start(U8 u8ChIIC)
{
    B16 bStatus = TRUE;    // success status

    MDrv_SW_IIC_SDA_Chk(u8ChIIC, _HIGH);
    MDrv_SW_IIC_Delay(u8ChIIC);

    MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);
    MDrv_SW_IIC_Delay(u8ChIIC);

    // check pin error
    SWIIC_SCL_PIN(u8ChIIC, _INPUT);
     SWIIC_SDA_PIN(u8ChIIC, _INPUT);


    if ((GET_SWIIC_SCL(u8ChIIC) == _LOW) || (GET_SWIIC_SDA(u8ChIIC) == _LOW))
    {
           SWIIC_SCL_PIN(u8ChIIC, _OUTPUT);
           SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);
        bStatus = FALSE;
    }
    else // success
    {
        MDrv_SW_IIC_SDA(u8ChIIC, _LOW);
		udelay(10);
        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);
    }

    return bStatus;     //vain
}

//-------------------------------------------------------------------------------------------------
// SW I2C: stop signal.
// <comment>
//              ____________
//  SCL _______/
//                 _________
//  SDA __________/
//-------------------------------------------------------------------------------------------------
void MDrv_SW_IIC_Stop(U8 u8ChIIC)
{
    _SCL_LOW(u8ChIIC);

    MDrv_SW_IIC_Delay(u8ChIIC);
    _SDA_LOW(u8ChIIC);

    MDrv_SW_IIC_Delay(u8ChIIC);
    _SCL_HIGH(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);
    _SDA_HIGH(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Send 1 bytes data
///@param u8data \b IN: 1 byte data to send
//-------------------------------------------------------------------------------------------------
B16 MDrv_IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack )   // Be used int IIC_SendByte
{
    U8      u8Mask = 0x80;
    B16     bAck; // acknowledge bit

    while ( u8Mask )
    {
        if (u8data & u8Mask)
        {
            MDrv_SW_IIC_SDA_Chk(u8ChIIC, _HIGH);
        }
        else
        {
            MDrv_SW_IIC_SDA_Chk(u8ChIIC, _LOW);
        }

        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH); // clock
        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);

        //MDrv_SW_IIC_Delay(u8ChIIC);

        u8Mask >>= 1; // next
        
    }

    // recieve acknowledge
    udelay(2);
    SWIIC_SDA_PIN(u8ChIIC, _INPUT);
    if( u8Delay4Ack > 0 )
    {
        //printk( "MDrv_IIC_SendByte %d %02x(%d)\n", u8ChIIC, u8data, u8Delay4Ack );
        udelay( u8Delay4Ack );
    }
    else
    {
        MDrv_SW_IIC_Delay(u8ChIIC);
    }
    MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);

    MDrv_SW_IIC_Delay(u8ChIIC);
    bAck = GET_SWIIC_SDA(u8ChIIC); // recieve acknowlege
//    SWIIC_SDA(u8ChIIC, bAck);     //for I2c waveform sharp
//    SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);
    MDrv_SW_IIC_SCL(u8ChIIC, _LOW);

    MDrv_SW_IIC_Delay(u8ChIIC);

    MDrv_SW_IIC_SDA(u8ChIIC, bAck);     //for I2c waveform sharp
    SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);

    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);

    return (bAck);
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Send 1 bytes data, this function will retry 5 times until success.
///@param u8data \b IN: 1 byte data to send
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
B16 MDrv_SW_IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack )
{
//    U8 u8I;

//    for(u8I=0;u8I<5;u8I++)
    {
        if (MDrv_IIC_SendByte(u8ChIIC, u8data, u8Delay4Ack) == I2C_ACKNOWLEDGE)
            return TRUE;
    }

    printf("IIC write byte 0x%02x fail!!\n", u8data);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
// SW I2C: access start.
//
// Arguments: u8SlaveAdr - slave address
//            u8Trans - I2C_TRANS_WRITE/I2C_TRANS_READ
//-------------------------------------------------------------------------------------------------
B16 MDrv_SW_IIC_AccessStart(U8 u8ChIIC, U8 u8SlaveAdr, U8 u8Trans)
{
    U8 u8Dummy; // loop dummy
    U8 u8Delay4Ack = 0;

    if (u8Trans == SWIIC_READ) // check i2c read or write
    {
        u8SlaveAdr = u8SlaveAdr | 0x01; // read
    }
    else
    {
        u8SlaveAdr = u8SlaveAdr & 0xfe; // write
    }

    if( u8ChIIC == 4 )
    {
        u8Delay4Ack = 100;
    }

    u8Dummy = I2C_ACCESS_DUMMY_TIME;
    while (u8Dummy--)
    {
        if ( MDrv_SW_IIC_Start(u8ChIIC) == FALSE)
        {
            //printk("MDrv_SW_IIC_Start=>Failed\n");
            continue;
        }

        if ( MDrv_SW_IIC_SendByte(u8ChIIC, u8SlaveAdr, u8Delay4Ack) == TRUE )   //I2C_ACKNOWLEDGE) // check acknowledge
        {
            return TRUE;
        }

        MDrv_SW_IIC_Stop(u8ChIIC);
    }

    //printk("MDrv_SW_IIC_Start(%c %02x %02x)=>Failed\n", (u8Trans == SWIIC_READ) ? 'r':'w', u8ChIIC, u8SlaveAdr );
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Get 1 bytes data
///@param u16Ack  \b IN: acknowledge
///@return U8:    \b OUT: get data from the device
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
U8 MDrv_SW_IIC_GetByte (U8 u8ChIIC, U16 u16Ack)
{
    U8 u8Receive = 0;
    U8 u8Mask = 0x80;

    SWIIC_SDA_PIN(u8ChIIC, _INPUT);          //SWIIC_SDA_PIN

    while ( u8Mask )
    {
        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);        //SWIIC_SCL_Chk
        MDrv_SW_IIC_Delay(u8ChIIC);

        if (GET_SWIIC_SDA(u8ChIIC) == _HIGH)      //GET_SWIIC_SDA
        {
            u8Receive |= u8Mask;
        }
        u8Mask >>= 1; // next

        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);      //SWIIC_SCL
        //SWIIC_Delay();

    }
	udelay(1);
    if (u16Ack)
    {
        // acknowledge
        MDrv_SW_IIC_SDA_Chk(u8ChIIC, I2C_ACKNOWLEDGE);      //SWIIC_SDA_Chk
    }
    else
    {
        // non-acknowledge
        MDrv_SW_IIC_SDA_Chk(u8ChIIC, I2C_NON_ACKNOWLEDGE);
    }
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);    //SWIIC_SCL_Chk
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_SCL(u8ChIIC, _LOW);         //SWIIC_SCL
    MDrv_SW_IIC_Delay(u8ChIIC);


    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);

    return u8Receive;
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Write bytes, be able to write 1 byte or several bytes to several register offsets in same slave address.
///@param u8SlaveID \b IN: Slave ID (Address)
///@param u8AddrCnt \b IN:  register NO to write, this parameter is the NO of register offsets in pu8addr buffer,
///it should be 0 when *pu8Addr = NULL.
///@param *pu8Addr \b IN: pointer to a buffer containing target register offsets to write
///@param u32BufLen \b IN: Data length (in byte) to write
///@param *pu8Buf \b IN: pointer to the data buffer for write
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
S32 MDrv_SW_IIC_Write(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf)
{
    U8  u8Dummy; // loop dummy
    S32 s32RetCountIIC;

    u8Dummy = 1;  //I2C_ACCESS_DUMMY_TIME;
    s32RetCountIIC = u32BufLen;

    while (u8Dummy--)
    {
        if (MDrv_SW_IIC_AccessStart(u8ChIIC, u8SlaveID, SWIIC_WRITE) == FALSE)
        {
            s32RetCountIIC = -2;
	     printf("err 1 ~~ \n");		
            goto SW_IIC_Write_End;
        }

        while( u8AddrCnt )
        {
              u8AddrCnt--;
              if ( MDrv_SW_IIC_SendByte(u8ChIIC, *pu8Addr, 0) == FALSE )
              {
                s32RetCountIIC = -3;
		  printf("err 2 ~~ \n");		
                goto SW_IIC_Write_End;
              }
              pu8Addr++;
        }
        while (u32BufLen) // loop of writting data
        {
            u32BufLen-- ;
            //IIC_SendByte(*pu8Buf); // send byte
            if ( MDrv_SW_IIC_SendByte(u8ChIIC, *pu8Buf, 0) == FALSE )
            {
            	  printf("err 3 ~~ \n");
                s32RetCountIIC = -4;
                goto SW_IIC_Write_End;
            }
            pu8Buf++; // next byte pointer
        }

        break;
    }

SW_IIC_Write_End:
    MDrv_SW_IIC_Stop(u8ChIIC);
//	printf("s32RetCountIIC = %d~~ \n", s32RetCountIIC);
//	printf("i2c done~~ \n");
    return s32RetCountIIC;
}


//-------------------------------------------------------------------------------------------------
///SW I2C: Read bytes, be able to read 1 byte or several bytes from several register offsets in same slave address.
///@param u8SlaveID \b IN: Slave ID (Address)
///@param u8AddrCnt \b IN:  register NO to read, this parameter is the NO of register offsets in pu8addr buffer,
///it should be 0 when *paddr = NULL.
///@param *pu8Addr \b IN: pointer to a buffer containing target register offsets to read
///@param u32BufLen \b IN: Data length (in byte) to read
///@param *pu8Buf \b IN: pointer to retun data buffer.
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
S32 MDrv_SW_IIC_Read(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf)
{
    U8  u8Dummy; // loop dummy
    S32 s32RetCountIIC;

    u8Dummy = I2C_ACCESS_DUMMY_TIME;
    s32RetCountIIC = u32BufLen;

    while (u8Dummy--)
    {
        /*    20080812:    (dreamer@lge.com)
            fixed:    Don't make a I2C's write start condition when u8AddrCnt == 0
        */
        if( u8AddrCnt > 0 )
        {
            if (MDrv_SW_IIC_AccessStart(u8ChIIC, u8SlaveID, SWIIC_WRITE) == FALSE)
            {
                s32RetCountIIC = -2;
                goto SW_IIC_Read_End;
            }

            while( u8AddrCnt )
            {
                u8AddrCnt--;
                if (MDrv_SW_IIC_SendByte(u8ChIIC, *pu8Addr, 0) == FALSE)
                {
                    s32RetCountIIC = -3;
                    goto SW_IIC_Read_End;
                }
                pu8Addr++;
            }
        }

        if (MDrv_SW_IIC_AccessStart(u8ChIIC, u8SlaveID, SWIIC_READ) == FALSE)
        {
            s32RetCountIIC = -4;
            goto SW_IIC_Read_End;
        }

        while (u32BufLen--) // loop to burst read
        {
            *pu8Buf = MDrv_SW_IIC_GetByte(u8ChIIC, u32BufLen); // receive byte

            pu8Buf++; // next byte pointer
        }

        break;
    }

SW_IIC_Read_End:
    MDrv_SW_IIC_Stop(u8ChIIC);

    return s32RetCountIIC;
}



//-------------------------------------------------------------------------------------------------
/// IIC master initialization
/// @return None
/// @note   Hardware IIC. Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MDrv_IIC_Init(void)
{
    //set all pads (except SPI) as input


	MHal_CHIP_REG(REG_IIC_MODE) &= ~BIT15;
	MHal_CHIP_REG(REG_IIC_MODE) |= BIT9;
	MHal_CHIP_REG(REG_IIC_MODE) &= ~BIT8;

	// clock = (Xtal / 4T) / div
	MHal_IIC_REG(REG_IIC_CLK_SEL) = 3; // 28.8M / 4 / 16 = 450KHz
	MHal_IIC_REG(REG_IIC_CTRL) = 0x80; // enable IIC
//extern unsigned int Chip_Query_MIPS_CLK(void);

  //  u32CpuCount = Chip_Query_MIPS_CLK();

    //u32SwIICdelayCycle =(unsigned int) ((1.5)/10000000)*u32CpuCount;
     //printk(">>> init ndeay(1)  speed=%d u32SwIICdelayCycle=%d \n",Chip_Query_MIPS_CLK(),u32SwIICdelayCycle);


    //set all pads(except SPI) as input
    MHal_GPIO_REG(REG_ALL_PAD_IN) &= ~BIT7;
    MHal_GPIO_REG(REG_DDCR_CK_SET1)  &= ~(BIT6|BIT5|BIT4);
    MHal_GPIO_REG(REG_DDCR_CK_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_DDCR_CK_SET3) |= (BIT7);


    MHal_GPIO_REG(REG_TEST_MODE)  &= ~(BIT2|BIT1|BIT0);
    MHal_GPIO_REG(REG_TEST_MODE)  &= ~(BIT6|BIT5|BIT4);

    MHal_GPIO_REG(REG_I2S_IN_WS_SET2) &= ~(BIT5|BIT4);
    MHal_GPIO_REG(REG_I2S_IN_WS_SET3) |= (BIT5);
    MHal_GPIO_REG(REG_I2S_IN_WS_SET3) &=~ (BIT6);

    MHal_GPIO_REG(REG_I2S_IN_BCK_SET2) |= (BIT5);

    MHal_GPIO_REG(REG_SPDIF_IN_SET) |= BIT6;

    MHal_GPIO_REG(REG_TGPIO0_SET1) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_TGPIO0_SET2) &= ~(BIT6);;
    MHal_GPIO_REG(REG_TGPIO1_SET) &= ~(BIT7|BIT6);

    MHal_GPIO_REG(REG_TGPIO2_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_TGPIO3_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_LHSYNC2_SET) &= ~(BIT7|BIT6);

    MHal_GPIO_REG(REG_DDCA_SET) &= ~BIT7;


}


void MHal_IIC_Clock_Select(U8 u8ClockIIC)
{
	//master iic clock select.
	//if 1:clk/4, if 2:clk/8, if 3:clk/16, if 4:clk/32,
	//if 5:clk/64, if 6:clk/128, if 7:clk/256, if 8:clk/512,
	//if 9:clk/1024, otherwise: clk/2
	MHal_IIC_REG(REG_IIC_CLK_SEL) = u8ClockIIC;
}

void MHal_IIC_Start(void)
{
    MHal_CHIP_REG(REG_IIC_MODE) &= ~BIT15;
    //iic master mode is enabled, bit 9~8 set to 2
    MHal_CHIP_REG(REG_IIC_MODE) &= ~BIT9;
    MHal_CHIP_REG(REG_IIC_MODE) &= ~BIT10;
    MHal_CHIP_REG(REG_IIC_MODE) |= BIT8;


	//reset iic circuit
	MHal_IIC_REG(REG_IIC_STATUS) = 0x08;
	MHal_IIC_REG(REG_IIC_STATUS) = 0x00;

	//reset iic circuit
	MHal_IIC_REG(REG_IIC_CTRL) = 0xC0;
}

void MHal_IIC_Stop(void)
{
	MHal_IIC_REG(REG_IIC_CTRL) = 0xA0;
	MHal_IIC_DELAY();

    MHal_CHIP_REG(REG_IIC_MODE) &= ~BIT15;
    MHal_CHIP_REG(REG_IIC_MODE) |= BIT9;
    MHal_CHIP_REG(REG_IIC_MODE) &= ~BIT8;


}


void MHal_IIC_NoAck(void)
{
	MHal_IIC_REG(REG_IIC_CTRL) = 0x90;
}

B16 MHal_IIC_SendData(U8 u8DataIIC)
{
    MHal_IIC_REG(REG_IIC_STATUS) = 0x02; /* clear interrupt */
    MHal_IIC_REG(REG_IIC_WDATA) = u8DataIIC;

//redcloud00@lge.com
/*
    {   //tune the dalay
        U8 u8Delay;
        for(u8Delay=0; u8Delay<100; u8Delay++);
        udelay(1000);
    }
*/
    while (1)
    {
        if (MHal_IIC_REG(REG_IIC_STATUS) & 0x01)
        {
			MHal_IIC_REG(REG_IIC_STATUS) = 0x02; /* clear interrupt */
			if (MHal_IIC_REG(REG_IIC_CTRL) & 0x08) /* no ACK */
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

B16 MHal_IIC_SendByte(U8 u8DataIIC)
{
	U8 u8I;

	for(u8I=0;u8I<10;u8I++)
	{
		if(MHal_IIC_SendData(u8DataIIC) == TRUE)
			return TRUE;
	}
	return FALSE;
}

B16 MHal_IIC_GetByte(U8* pu8DataIIC) /* auto generate ACK */
{
	//U32	u32cur_jiffies;

    /* clear interrupt & start byte reading */
    MHal_IIC_REG(REG_IIC_STATUS) = 0x04;

//redcloud00@lge.com
/*
    {   //tune the dalay
        U8 u8Delay;
        for(u8Delay=0; u8Delay<100; u8Delay++);
        udelay(1000);
    }
*/
    while(1)  
    {

		if (MHal_IIC_REG(REG_IIC_STATUS))
		{
			MHal_IIC_REG(REG_IIC_STATUS) = 0x02; /* clear interrupt */
			*pu8DataIIC = MHal_IIC_REG(REG_IIC_RDATA);
            udelay(1);
            return TRUE;
		}
	}

	return FALSE;
}
//-------------------------------------------------------------------------------------------------
/// Write data to an IIC device
/// @param  u8SlaveIdIIC            \b IN:  device slave ID
/// @param  u8AddrSizeIIC           \b IN:  address length in bytes
/// @param  pu8AddrIIC              \b IN:  pointer to the start address of the device
/// @param  u32BufSizeIIC           \b IN:  number of bytes to be written
/// @param  pu8BufIIC               \b IN:  pointer to write data buffer
/// @return TRUE(succeed), FALSE(fail)
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
//S32 MDrv_HW_IIC_Write(U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC);

S32 MDrv_HW_IIC_Write(U8 u8SlaveIdIIC, U8 u8AddrSizeIIC, U8 *pu8AddrIIC, U32 u32BufSizeIIC, U8 *pu8BufIIC)
{
    U32     u32I;
    U8      u8I;
    B16     bReturnIIC = TRUE;
    S32     s32RetCountIIC = -1;

    MHal_IIC_Start();
	//IIC_DBG(printk(">>  MHal_IIC_Start  %s %d  \n ",__FILE__,__LINE__));
    if (FALSE == MHal_IIC_SendByte(u8SlaveIdIIC&0xFE))
    {
    	printf("hw :err1 ~~ \n");
        bReturnIIC = FALSE;
        s32RetCountIIC = -2;
        goto HW_IIC_Write_End;
    }

    for (u8I = 0; u8I < u8AddrSizeIIC; u8I++)
    {
        if (FALSE == MHal_IIC_SendByte(pu8AddrIIC[u8I]))
        {
        	printf("hw :err2 ~~ \n");
            bReturnIIC = FALSE;
            s32RetCountIIC = -3;
            goto HW_IIC_Write_End;
        }
    }

    for (u32I = 0; u32I < u32BufSizeIIC; u32I++)
    {
        if (FALSE == MHal_IIC_SendByte(pu8BufIIC[u32I]))
        {
        	printf("hw :err3 ~~ \n");
            bReturnIIC = FALSE;
            s32RetCountIIC = -4;
            goto HW_IIC_Write_End;
        }
    }

    s32RetCountIIC = u32BufSizeIIC;

HW_IIC_Write_End:
    MHal_IIC_Stop();

//    IIC_DBG(printk("MDrv_IIC_Write() --> s32RetCountIIC=%d \n", s32RetCountIIC));
    return s32RetCountIIC;
}


//-------------------------------------------------------------------------------------------------
/// Read data from an IIC device
/// @param  u8SlaveIdIIC            \b IN:  device slave ID
/// @param  u8AddrSizeIIC           \b IN:  address length in bytes
/// @param  pu8AddrIIC              \b IN:  ptr to the start address inside the device
/// @param  u32BufSizeIIC           \b IN:  number of bytes to be read
/// @param  pu8BufIIC               \b OUT: pointer to read data buffer
/// @return TRUE : succeed
/// @return FALSE : fail
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
S32 MDrv_HW_IIC_Read(U8 u8SlaveIdIIC, U8 u8AddrSizeIIC, U8 *pu8AddrIIC, U32 u32BufSizeIIC, U8 *pu8BufIIC)
{
    U32     u32I;
    U8      u8I;
    B16     bReturnIIC = TRUE;
    S32     s32RetCountIIC = -1;

    MHal_IIC_Start();

    if (FALSE == MHal_IIC_SendByte(u8SlaveIdIIC&0xFE))
    {
        bReturnIIC = FALSE;
        s32RetCountIIC = -2;
        goto HW_IIC_Read_End;
    }

    for (u8I = 0; u8I < u8AddrSizeIIC; u8I++)
    {
        if (FALSE == MHal_IIC_SendByte(pu8AddrIIC[u8I]))
        {
            bReturnIIC = FALSE;
            s32RetCountIIC = -3;
            goto HW_IIC_Read_End;
        }
    }

    MHal_IIC_Start();

    udelay(100);

    if (FALSE == MHal_IIC_SendByte(u8SlaveIdIIC|0x1))
    {
        bReturnIIC = FALSE;
        s32RetCountIIC = -4;
        goto HW_IIC_Read_End;
    }

    for (u32I = 0; u32I < u32BufSizeIIC; u32I++)
    {
        if (u32I == (u32BufSizeIIC-1))
        {
            MHal_IIC_NoAck();
        }

        if (FALSE == MHal_IIC_GetByte(&pu8BufIIC[u32I]))
        {
            bReturnIIC = FALSE;
            s32RetCountIIC = -5;
            goto HW_IIC_Read_End;
        }
    }

    s32RetCountIIC = u32BufSizeIIC;

HW_IIC_Read_End:
    MHal_IIC_Stop();

    //IIC_DBG(printk("MDrv_IIC_Read() --> s32RetCountIIC=%d \n", s32RetCountIIC));
    return s32RetCountIIC;
}


//-------------------------------------------------------------------------------------------------
/// IIC initialization
/// @param  u8ClockIIC            \b IN:  clock selection
/// @return None
/// @note    added by LGE(dreamer@lge.com)
//-------------------------------------------------------------------------------------------------
void MDrv_SW_IIC_Enable( U8 u8ChIIC, U8 bEnable )
{
    switch(u8ChIIC)
    {
        case IIC_NUM_OF_NEC_MICOM:
        case IIC_NUM_OF_AUDIO_AMP:
            if( bEnable )
            {
                //MHal_GPIO_REG(REG_I2S_OUT_SD1_SET) |= BIT6;
            }
            else
            {
                //MHal_GPIO_REG(REG_I2S_OUT_SD1_SET) &= ~(BIT6);
            }
            break;
        case IIC_NUM_OF_RGB_EDID:
            if( bEnable )
            {
                MHal_GPIO_REG(REG_DDCA_SET) |= BIT7;
            }
            else
            {
                MHal_GPIO_REG(REG_DDCA_SET) &= ~(BIT7);
            }
            break;

        case IIC_NUM_OF_HDMI_A_EDID:
            if( bEnable )
            {
                MHal_GPIO_REG(REG_DDCDA_SET) |= BIT7;
            }
            else
            {
                MHal_GPIO_REG(REG_DDCDA_SET) &= ~(BIT7);
            }
            break;
        case IIC_NUM_OF_HDMI_B_EDID:
            if( bEnable )
            {
                MHal_GPIO_REG(REG_DDCDB_SET) |= BIT7;
            }
            else
            {
                MHal_GPIO_REG(REG_DDCDB_SET) &= ~(BIT7);
            }
            break;
        case IIC_NUM_OF_HDMI_C_EDID:
            if( bEnable )
            {
                MHal_GPIO_REG(REG_DDCDC_SET) |= BIT7;
            }
            else
            {
                MHal_GPIO_REG(REG_DDCDC_SET) &= ~(BIT7);
            }
            break;
        default:
            break;
    }
}




//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

void MAdp_IIC_Set_Force_Channel( U8 u8ChIIC ){
	u8ForceChIIC = u8ChIIC ;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/// All IIC master initialization
/// @return None
/// @note   Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MAdp_IIC_Init(void)
{

    i2c_init();
    //MAdp_IIC_Clock_Select(3);

    if (s32FdIIC == 0)   //First time open
    {
//        s32FdIIC = open(IIC_MODULE_KERNAL_NAME, O_RDWR);

        if (s32FdIIC)
        {
            //ioctl(s32FdIIC, MDRV_IIC_INIT);
                i2c_init();
        }
        else
        {
            //(printf("Fail to open IIC Kernal Module\n"));
            return;
        }
    }
    else
    {
        if(u8InitIIC == FALSE)
        {
            u8InitIIC = TRUE;
            //ioctl(s32FdIIC, MDRV_IIC_INIT);
                i2c_init();
        }
    }

}

//-------------------------------------------------------------------------------------------------
/// IIC clock selection
/// @param  u8ChIIC               \b IN:  IIC channel
/// @param  u8ClockIIC            \b IN:  clock selection
/// @return None
/// @note For HW I2C: \n
///       If u8ClockIIC=1 => clk/4,   u8ClockIIC=2 => clk/8,   u8ClockIIC=3 =>clk/16, \n
///       If u8ClockIIC=4 => clk/32,  u8ClockIIC=5 => clk/64,  u8ClockIIC=6 =>clk/128, \n
///       If u8ClockIIC=7 => clk/256, u8ClockIIC=8 => clk/512, u8ClockIIC=9 =>clk/1024, \n
///       Otherwise =>clk/2 \n
//-------------------------------------------------------------------------------------------------
void MAdp_IIC_Clock_Select(U8 u8ChIIC, U8 u8ClockIIC)
{
    IIC_Param_t param;

    param.u8IdIIC = u8ChIIC;
    param.u8ClockIIC = u8ClockIIC;
    if(u8ChIIC == 1)
    {
        MHal_IIC_Clock_Select(u8ClockIIC);
    }
    else
    {
        SWII_DELAY(u8ChIIC) = u8ClockIIC;
    }


    //ioctl(s32FdIIC, MDRV_IIC_CLOCK, &param);
}

//-------------------------------------------------------------------------------------------------
/// Write data to an IIC device
/// @param  u8ChIIC                 \b IN:  IIC channel
/// @param  u8SlaveIdIIC            \b IN:  device slave ID
/// @param  u8AddrSizeIIC           \b IN:  address length in bytes
/// @param  pu8AddrIIC              \b IN:  pointer to the start address of the device
/// @param  u32BufSizeIIC           \b IN:  number of bytes to be written
/// @param  pu8BufIIC               \b IN:  pointer to write data buffer
/// @return TRUE(succeed), FALSE(fail)
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
//B16 MAdp_IIC_Write(U8 u8ChIIC, U8 u8SlaveIdIIC, U8 u8AddrSizeIIC, U8 *pu8AddrIIC, U32 u32BufSizeIIC, U8 *pu8BufIIC)
B16 MAdp_IIC_Write(U8 u8ChIIC, U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC )
{
    IIC_Param_t param;
    S32 s32WCountIIC = 0;

/*
	if( u8ForceChIIC )
	    u8ChIIC = u8ForceChIIC ;
*/

    param.u8IdIIC = u8ChIIC;
    param.u8SlaveIdIIC = u8SlaveIdIIC;
    memcpy(param.u8AddrIIC, pu8AddrIIC, u8AddrSizeIIC);
    param.u8AddrSizeIIC = u8AddrSizeIIC;
	//dhjung LGE


	param.u8pbufIIC      = pu8BufIIC;
	param.u32DataSizeIIC = u32BufSizeIIC;
	
	if( param.u8IdIIC == 1 )
	{
		s32WCountIIC = MDrv_HW_IIC_Write(param.u8SlaveIdIIC, param.u8AddrSizeIIC, param.u8AddrIIC, param.u32DataSizeIIC, param.u8pbufIIC);
	}
	else
	{
		s32WCountIIC = MDrv_SW_IIC_Write(param.u8IdIIC, param.u8SlaveIdIIC, param.u8AddrSizeIIC, param.u8AddrIIC, param.u32DataSizeIIC, param.u8pbufIIC);
	}

	if (s32WCountIIC<0)
	{
		return FALSE;
	}


	return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Read data from an IIC device
/// @param  u8ChIIC                 \b IN:  IIC channel
/// @param  u8SlaveIdIIC            \b IN:  device slave ID
/// @param  u8AddrSizeIIC           \b IN:  address length in bytes
/// @param  pu8AddrIIC              \b IN:  ptr to the start address inside the device
/// @param  u32BufSizeIIC           \b IN:  number of bytes to be read
/// @param  pu8BufIIC               \b OUT: pointer to read data buffer
/// @return TRUE(succeed), FALSE(fail)
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
B16 MAdp_IIC_Read(U8 u8ChIIC, U8 u8SlaveIdIIC, U8 *pu8AddrIIC, U8 u8AddrSizeIIC, U8 *pu8BufIIC, U32 u32BufSizeIIC )
{
    IIC_Param_t param;
    S32 s32RCountIIC = 0;

/*
	if( u8ForceChIIC )
		u8ChIIC = u8ForceChIIC ;
*/
    param.u8IdIIC = u8ChIIC;
    param.u8SlaveIdIIC = u8SlaveIdIIC;
    memcpy(param.u8AddrIIC, pu8AddrIIC, u8AddrSizeIIC);
    param.u8AddrSizeIIC = u8AddrSizeIIC;

	param.u8pbufIIC      = pu8BufIIC;
	param.u32DataSizeIIC = u32BufSizeIIC;


	if( param.u8IdIIC == 1 )
	{
		s32RCountIIC = MDrv_HW_IIC_Read(param.u8SlaveIdIIC,param.u8AddrSizeIIC, param.u8AddrIIC, param.u32DataSizeIIC,param.u8pbufIIC);
	}
	else
	{
		s32RCountIIC = MDrv_SW_IIC_Read(param.u8IdIIC,param.u8SlaveIdIIC,param.u8AddrSizeIIC, param.u8AddrIIC, param.u32DataSizeIIC,param.u8pbufIIC);
	}


	if (s32RCountIIC<0)
	{

		return FALSE;
	}


    return TRUE;
}

#endif //ifdef CONFIG_TITANIA
