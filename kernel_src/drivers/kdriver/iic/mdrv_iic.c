////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_iic.c
/// @brief  IIC Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mst_devid.h"

#include "mdrv_iic.h"
#include "mhal_iic_reg.h"
#include "mhal_iic.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define SWI2C_LOCK      0//alex_tung@@@
#if SWI2C_LOCK
spinlock_t _iic_spinlock;
#define MDrv_SW_IIC_InitLock()      spin_lock_init(&_iic_spinlock)
#define MDrv_SW_IIC_Lock()          spin_lock_irq(&_iic_spinlock)
#define MDrv_SW_IIC_UnLock()        spin_unlock_irq(&_iic_spinlock)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define IIC_DBG_ENABLE           0

#if IIC_DBG_ENABLE
#define IIC_DBG(_f)             (_f)
#else
#define IIC_DBG(_f)
#endif

#if 0
#define LINE_DBG()             printf("IIC %d\n", __LINE__)
#else
#define LINE_DBG()
#endif

#define IIC_PRINT(fmt, args...)    //printk("[IIC][%05d] " fmt, __LINE__, ## args)

// for SW I2C
#define _INPUT                  1
#define _OUTPUT                 0
#define _HIGH                   1
#define _LOW                    0
#define SWIIC_READ              0
#define SWIIC_WRITE             1
#define I2C_CHECK_PIN_DUMMY     3200/*6000*//*3200*/ // 091021_louis, to ensure the data holding time. 8000//hjkoh_090909 S6가 3200인것을 감안하여 Counter 재설정함.
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
//  Global Variables
//-------------------------------------------------------------------------------------------------
IIC_Bus_t _I2CBus[] =
{
/*CH2*/{REG_DDCR_CK_OEN, BIT4, REG_DDCR_CK_OUT, BIT4, REG_DDCR_CK_IN, BIT4, \
     REG_DDCR_DA_OEN, BIT3, REG_DDCR_DA_OUT, BIT3, REG_DDCR_DA_IN, BIT3, 2},


/*CH3*/{REG_I2S_OUT_SD3_OEN, BIT3, REG_I2S_OUT_SD3_OUT, BIT3, REG_I2S_OUT_SD3_IN, BIT3, \
     REG_I2S_OUT_SD2_OEN, BIT2, REG_I2S_OUT_SD2_OUT, BIT2, REG_I2S_OUT_SD2_IN, BIT2, 2},

/*CH4*/{REG_I2S_OUT_SD1_OEN, BIT1, REG_I2S_OUT_SD1_OUT, BIT1, REG_I2S_OUT_SD1_IN, BIT1, \
     REG_SPDIF_IN_OEN, BIT3, REG_SPDIF_IN_OUT, BIT3, REG_SPDIF_IN_IN, BIT3, 2},


/*CH5*/{REG_TGPIO0_OEN, BIT5, REG_TGPIO0_OUT, BIT5, REG_TGPIO0_IN, BIT5, \
    REG_TGPIO1_OEN, BIT6, REG_TGPIO1_OUT, BIT6, REG_TGPIO1_IN, BIT6, 2},

/*CH6*/{REG_TGPIO2_OEN, BIT7, REG_TGPIO2_OUT, BIT7, REG_TGPIO2_IN, BIT7, \
    REG_TGPIO3_OEN, BIT0, REG_TGPIO3_OUT, BIT0, REG_TGPIO3_IN, BIT0, 2},


/*CH7*/{REG_I2S_IN_WS_OEN, BIT0, REG_I2S_IN_WS_OUT, BIT0, REG_I2S_IN_WS_IN, BIT0, \
     REG_I2S_IN_BCK_OEN, BIT1, REG_I2S_IN_BCK_OUT, BIT1, REG_I2S_IN_BCK_IN, BIT1, 2},

/*CH8 : RGB - EDID */
    {REG_DDCA_CK_OEN, BIT1, REG_DDCA_CK_OUT, BIT2, REG_DDCA_CK_IN, BIT0, \
     REG_DDCA_DA_OEN, BIT5, REG_DDCA_DA_OUT, BIT6, REG_DDCA_DA_IN, BIT4, 2},

/*CH9 : HDMI port A - EDID */
    {REG_DDCDA_CK_OEN, BIT1, REG_DDCDA_CK_OUT, BIT2, REG_DDCDA_CK_IN, BIT0, \
     REG_DDCDA_DA_OEN, BIT5, REG_DDCDA_DA_OUT, BIT6, REG_DDCDA_DA_IN, BIT4, 2},
/*CH10 : HDMI port B - EDID */
    {REG_DDCDB_CK_OEN, BIT1, REG_DDCDB_CK_OUT, BIT2, REG_DDCDB_CK_IN, BIT0, \
     REG_DDCDB_DA_OEN, BIT5, REG_DDCDB_DA_OUT, BIT6, REG_DDCDB_DA_IN, BIT4, 2},
/*CH11 : HDMI port C - EDID */
    {REG_DDCDC_CK_OEN, BIT1, REG_DDCDC_CK_OUT, BIT2, REG_DDCDC_CK_IN, BIT0, \
     REG_DDCDC_DA_OEN, BIT5, REG_DDCDC_DA_OUT, BIT6, REG_DDCDC_DA_IN, BIT4, 2},

#if 0 //2 Duplicated setting
/*CH12 : HDMI port C - EDID */
    {REG_DDCDD_CK_OEN, BIT1, REG_DDCDD_CK_OUT, BIT2, REG_DDCDD_CK_IN, BIT0, \
     REG_DDCDD_DA_OEN, BIT5, REG_DDCDD_DA_OUT, BIT6, REG_DDCDD_DA_IN, BIT4, 5},
#endif

/*CH12 : HDMI port D - EDID */
    {REG_DDCDD_CK_OEN, BIT1, REG_DDCDD_CK_OUT, BIT2, REG_DDCDD_CK_IN, BIT0, \
     REG_DDCDD_DA_OEN, BIT5, REG_DDCDD_DA_OUT, BIT6, REG_DDCDD_DA_IN, BIT4, 2},


#if 0  //2 wrong connection to Micom
/*CH13 : NEC */
    {REG_GPIO24_OEN, BIT0, REG_GPIO24_OUT, BIT0, REG_GPIO24_IN, BIT0, \
     REG_GPIO25_OEN, BIT1, REG_GPIO25_OUT, BIT1, REG_GPIO25_IN, BIT1, 5},
#endif

};




//dhjung LGE
#if 0
static U8 u8BusSel;
static U8 _u8SwIicDly2;
static U8 _u8SwIicDly3;
static U8 _u8SwIicDly4;
static U8 _u8SwIicDly5;
static U8 _u8SwIicDly6;
static U8 _u8SwIicDly7;
#endif

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
#define NEW_DELAY_FORMULA   1//alex_tung@@@
#define EXTRA_DELAY_CYCLE   1//alex_tung@@@
#if (NEW_DELAY_FORMULA)
#define FACTOR_DLY    69000UL
#define FACTOR_ADJ    11040UL
static U32 u32CpuSpeedMHz;
static U32 u32AdjParam;
static U16 _u16SwIicDly[IIC_NUM_OF_MAX];
//#define DELAY_CYCLES(SpeedKHz)  ((FACTOR_DLY/(SpeedKHz))-((110+u32AdjParam)-((SpeedKHz)/u32AdjParam)))
#define DELAY_CYCLES(SpeedKHz)  ((FACTOR_DLY/(SpeedKHz))-((130+u32AdjParam)-((SpeedKHz)/u32AdjParam))+((1<<((440-SpeedKHz)/40))))
#endif

void MDrv_SW_IIC_SetSpeed(U8 u8ChIIC, U8 u8Speed)
{
// added for removing _u8SwIicDly2 by LGE(dreamer@lge.com)
    if( (u8ChIIC > IIC_NUM_OF_HW) && (u8ChIIC <= IIC_NUM_OF_MAX) )
    {
        SWII_DELAY(u8ChIIC) = u8Speed;
    }
    #if (NEW_DELAY_FORMULA)
    switch(SWII_DELAY(u8ChIIC))
    {
        case 2:
            _u16SwIicDly[u8ChIIC]=400;//KHz
            break;
        case 3:
        case 4:
        case 5:
            _u16SwIicDly[u8ChIIC]=100;//KHz
            break;
        case 6:
            _u16SwIicDly[u8ChIIC]=50;//KHz
            break;
        default:
            _u16SwIicDly[u8ChIIC]=100;//KHz
            break;
    }
    #endif
}
#if (EXTRA_DELAY_CYCLE)
void MDrv_SW_IIC_DelayEx(U8 u8ChIIC,U8 u8Divisor)
{
    __asm__ __volatile__ (
        ".set noreorder\n"
        "1:"
        "\tbgtz %0,1b\n"
        "\taddiu %0,%0,-1\n"
        : : "r" (DELAY_CYCLES(_u16SwIicDly[u8ChIIC])/u8Divisor));
}
#endif

void MDrv_SW_IIC_Delay(U8 u8ChIIC)
{
#if (NEW_DELAY_FORMULA)
    __asm__ __volatile__ (
        ".set noreorder\n"
        "1:"
        "\tbgtz %0,1b\n"
        "\taddiu %0,%0,-1\n"
        : : "r" (DELAY_CYCLES(_u16SwIicDly[u8ChIIC])));
#else
    U16 u16I;

    U8 u8Loop = 5;

// added for removing _u8SwIicDly2 by LGE(dreamer@lge.com)
    if( (u8ChIIC > IIC_NUM_OF_HW) && (u8ChIIC <= IIC_NUM_OF_MAX) )
    {
        u8Loop = SWII_DELAY(u8ChIIC);
    }


    //for fast mode  about 380khz
    if(u8Loop == 2)
    {
		if(u8ChIIC == 6) /* temporary code to avoid tuner(ch=6) i2c data error! by jjab. LGE. */
			udelay(3);
		else
			for (u16I=0;u16I<35/*40*/;u16I++)
				asm volatile ("nop;");
	}
    // for slow mode (about 50Khz)
    else if(u8Loop == 6)
    {
         udelay(10);
    }
    else
    //for normal and slow mode
    {
         udelay(4);
    }
#endif
}

//dhjung LGE
#if 0
void MDrv_SW_IIC_SelectBus(U8 u8BusChn)
{
    u8BusSel = u8BusChn;
}
#endif

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
    u16 u16Dummy;       // loop dummy

    if (bSet == _HIGH)  // if set pin high
    {
        SWIIC_SCL_PIN(u8ChIIC, _INPUT);
        u16Dummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
        //< 090903 leehc > IIC 통신 오류대응. - hjkoh_090909 udelay제거.
        while ((GET_SWIIC_SCL(u8ChIIC) == _LOW) && (u16Dummy--));
    }
    else
    {
        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);    // set SCL pin
        SWIIC_SCL_PIN(u8ChIIC, _OUTPUT);
    }
}

void MDrv_SW_IIC_SDA_Chk(U8 u8ChIIC, B16 bSet)
{
    U16 u16Dummy;       // loop dummy

    if (bSet == _HIGH)  // if set pin high
    {
        SWIIC_SDA_PIN(u8ChIIC, _INPUT);
        u16Dummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
        while ((GET_SWIIC_SDA(u8ChIIC) == _LOW) && (u16Dummy--));// check SDA pull high// - hjkoh_090909 udelay제거.
    }
    else
    {
        MDrv_SW_IIC_SDA(u8ChIIC, _LOW);    // set SDA pin
        SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);
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
//    _SCL_HIGH(u8ChIIC);
	MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);	// <20091212 takerest> fix SCL pin error
    MDrv_SW_IIC_Delay(u8ChIIC);
//    _SDA_HIGH(u8ChIIC);
	MDrv_SW_IIC_SDA_Chk(u8ChIIC, _HIGH);	// <20091212 takerest> fix SDA pin error
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
    	if(SWII_DELAY(u8ChIIC) == 2)
    	{
    		MDrv_SW_IIC_DelayEx(u8ChIIC, 8);	// 20091103_louis added delay to fix fast mode issue which is over 400KHz.
    	}
        MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH); // clock
        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);
        u8Mask >>= 1; // next
    }

    // recieve acknowledge
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
    MDrv_SW_IIC_Delay(u8ChIIC);	// 091021_louis, LOW period of SCL Clock is not enough before getting ACK.
    MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);

    bAck = GET_SWIIC_SDA(u8ChIIC); // recieve acknowlege
//    SWIIC_SDA(u8ChIIC, bAck);     //for I2c waveform sharp
//    SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);
    MDrv_SW_IIC_Delay(u8ChIIC);	// 091027_louis, High period of SCL Clock is not enough
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

    //printk("IIC write byte 0x%02x fail!!\n", u8data);
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

        #if (EXTRA_DELAY_CYCLE)
	 if(SWII_DELAY(u8ChIIC) == 2)	 // 091022_louis fine tune Read byre clock speed of fast mode
	{
        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_Delay(u8ChIIC);
	        MDrv_SW_IIC_DelayEx(u8ChIIC, 8);	// 20091103_louis added delay to fix fast mode issue which is over 400KHz.
	}
        #endif
    }
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
    #if SWI2C_LOCK
    MDrv_SW_IIC_Lock();
    #endif
    while (u8Dummy--)
    {
        if (MDrv_SW_IIC_AccessStart(u8ChIIC, u8SlaveID, SWIIC_WRITE) == FALSE)
        {
            s32RetCountIIC = -2;
            goto SW_IIC_Write_End;
        }

        while( u8AddrCnt )
        {
              u8AddrCnt--;
              if ( MDrv_SW_IIC_SendByte(u8ChIIC, *pu8Addr, 0) == FALSE )
              {
                s32RetCountIIC = -3;
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
                s32RetCountIIC = -4;
                goto SW_IIC_Write_End;
            }
            pu8Buf++; // next byte pointer
        }

        break;
    }

SW_IIC_Write_End:
    MDrv_SW_IIC_Stop(u8ChIIC);
    #if SWI2C_LOCK
    MDrv_SW_IIC_UnLock();
    #endif
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
    #if SWI2C_LOCK
    MDrv_SW_IIC_Lock();
    #endif

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

    #if SWI2C_LOCK
    MDrv_SW_IIC_UnLock();
    #endif
    return s32RetCountIIC;
}



//-------------------------------------------------------------------------------------------------
/// IIC master initialization
/// @return None
/// @note   Hardware IIC. Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MDrv_IIC_Init(void)
{
    MHal_IIC_Init();

#if SWI2C_LOCK
    MDrv_SW_IIC_InitLock();
#endif
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

    //MHal_GPIO_REG(REG_I2S_OUT_SD1_SET) |= (BIT6);
    //MHal_GPIO_REG(REG_I2S_OUT_SD2_SET) |= (BIT6);
    //MHal_GPIO_REG(REG_I2S_OUT_SD3_SET) |= (BIT6);

    MHal_GPIO_REG(REG_TGPIO0_SET1) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_TGPIO0_SET2) &= ~(BIT6);;
    MHal_GPIO_REG(REG_TGPIO1_SET) &= ~(BIT7|BIT6);

    MHal_GPIO_REG(REG_TGPIO2_SET2) &= ~(BIT1|BIT0);
    MHal_GPIO_REG(REG_TGPIO3_SET2) &= ~(BIT1|BIT0);

    MHal_GPIO_REG(REG_DDCDA_CK_SET) &= ~BIT7;
    MHal_GPIO_REG(REG_DDCDA_DA_SET) &= ~BIT7;
    MHal_GPIO_REG(REG_DDCDB_DA_SET) &= ~BIT7;
    MHal_GPIO_REG(REG_DDCDB_CK_SET) &= ~BIT7;
    MHal_GPIO_REG(REG_DDCDC_CK_SET) &= ~BIT7;
    MHal_GPIO_REG(REG_DDCDC_DA_SET) &= ~BIT7;
    MHal_GPIO_REG(REG_DDCDD_CK_SET) &= ~BIT7;
    MHal_GPIO_REG(REG_DDCDD_DA_SET) &= ~BIT7;
    MHal_GPIO_REG(REG_LHSYNC2_SET) &= ~(BIT7|BIT6);
    MHal_GPIO_REG(REG_SAR3_OEN) |= BIT3;

#if (NEW_DELAY_FORMULA)
{
    extern unsigned int Chip_Query_MIPS_CLK(void);
    U8 chidx;

    u32CpuSpeedMHz = Chip_Query_MIPS_CLK()/1000000UL;
    u32AdjParam = FACTOR_ADJ/u32CpuSpeedMHz;
    printk("@@@@@@ u32CpuSpeedMHz= %d MHz\n",u32CpuSpeedMHz);
    printk("@@@@@@ u32AdjParam= %d MHz\n",u32AdjParam);
    for(chidx=0;chidx<IIC_NUM_OF_MAX;chidx++)
    {
        _u16SwIicDly[chidx] = 100; //100KHz
    }
}
#endif

}

// For LGE S5-1 ATSC Board (no TCON chip)
#if 0
void MDrv_IIC_Init(void)
{
    MHal_IIC_Init();

    //set all pads(except SPI) as input
    MHal_GPIO_REG(REG_ALL_PAD_IN) &= ~BIT7;
    //for channel 2 set
    MHal_GPIO_REG(REG_DDCR_CK_SET) |= BIT7;
    MHal_GPIO_REG(REG_DDCR_DA_SET) |= BIT7;
    _u8SwIicDly2 = _I2CBus[0].DefDelay;
    //for channel 3 set
    MHal_GPIO_REG(REG_PCM2_IRQA_N_SET) |= BIT7;
    MHal_GPIO_REG(REG_PCM2_IRQA_N_SET) &= ~BIT6;
    _u8SwIicDly3 = _I2CBus[1].DefDelay;
    //for channel 4 set
    _u8SwIicDly4 = _I2CBus[2].DefDelay;
    //for channel 5 set
    _u8SwIicDly5 = _I2CBus[3].DefDelay;
    //for channel 6 set
    _u8SwIicDly6 = _I2CBus[4].DefDelay;
}
#endif

//-------------------------------------------------------------------------------------------------
/// IIC clock selection
/// @param  u8ClockIIC            \b IN:  clock selection
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_HW_IIC_Clock_Select(U8 u8ClockIIC)
{
    MHal_IIC_Clock_Select(u8ClockIIC);
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
S32 MDrv_HW_IIC_Write(U8 u8SlaveIdIIC, U8 u8AddrSizeIIC, U8 *pu8AddrIIC, U32 u32BufSizeIIC, U8 *pu8BufIIC)
{
    U32     u32I;
    U8      u8I;
    B16     bReturnIIC = TRUE;
    S32     s32RetCountIIC = -1;

    MHal_IIC_Start();
	IIC_DBG(printk(">>  MHal_IIC_Start  %s %d  \n ",__FILE__,__LINE__));

    if (FALSE == MHal_IIC_SendByte(u8SlaveIdIIC&0xFE))
    {
        bReturnIIC = FALSE;
        s32RetCountIIC = -2;
        goto HW_IIC_Write_End;
    }

    for (u8I = 0; u8I < u8AddrSizeIIC; u8I++)
    {
        if (FALSE == MHal_IIC_SendByte(pu8AddrIIC[u8I]))
        {
            bReturnIIC = FALSE;
            s32RetCountIIC = -3;
            goto HW_IIC_Write_End;
        }
    }

    for (u32I = 0; u32I < u32BufSizeIIC; u32I++)
    {
        if (FALSE == MHal_IIC_SendByte(pu8BufIIC[u32I]))
        {
            bReturnIIC = FALSE;
            s32RetCountIIC = -4;
            goto HW_IIC_Write_End;
        }
    }

    s32RetCountIIC = u32BufSizeIIC;

HW_IIC_Write_End:
    MHal_IIC_Stop();

    IIC_DBG(printk("MDrv_IIC_Write() --> s32RetCountIIC=%d \n", s32RetCountIIC));
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

    IIC_DBG(printk("MDrv_IIC_Read() --> s32RetCountIIC=%d \n", s32RetCountIIC));
    return s32RetCountIIC;
}
EXPORT_SYMBOL(MDrv_HW_IIC_Read);

//-------------------------------------------------------------------------------------------------
/// IIC initialization
/// @param  u8ClockIIC            \b IN:  clock selection
/// @return None
/// @note    added by LGE(dreamer@lge.com)
//-------------------------------------------------------------------------------------------------
void MDrv_HW_IIC_Init( void )
{
	IIC_DBG(printk(">>  MDrv_HW_IIC_Init  %s %d  \n ",__FILE__,__LINE__));

    MHal_IIC_Init();
}
EXPORT_SYMBOL(MDrv_HW_IIC_Init);

//-------------------------------------------------------------------------------------------------
/// IIC initialization
/// @param  u8ClockIIC            \b IN:  clock selection
/// @return None
/// @note    added by LGE(dreamer@lge.com)
//-------------------------------------------------------------------------------------------------
void MDrv_SW_IIC_Enable( U8 u8ChIIC, BOOL bEnable )
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

        case IIC_NUM_OF_HDMI_D_EDID:
            if( bEnable )
            {
                MHal_GPIO_REG(REG_DDCDD_SET) |= BIT7;
            }
            else
            {
                MHal_GPIO_REG(REG_DDCDD_SET) &= ~(BIT7);
            }
            break;

        default:
            break;
    }
}

EXPORT_SYMBOL(MDrv_IIC_Init);
EXPORT_SYMBOL(MDrv_HW_IIC_Write);
EXPORT_SYMBOL(MDrv_SW_IIC_Read);
EXPORT_SYMBOL(MDrv_SW_IIC_Write);
EXPORT_SYMBOL(MDrv_SW_IIC_SetSpeed);

