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
/// file    mdrv_gpio.c
/// @brief  GPIO Driver Interface
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

#include "mdrv_gpio.h"
#include "mdrv_gpio_io.h"
#include "mhal_gpio_reg.h"
#include "mhal_gpio.h"
#if use_IO_Expand
#include "mdrv_ext_gpio.c"
#endif

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

/*
	test가 충분히 안된 상태임.
	_INCLUDE_PM_USE_STATE
*/
//#define _INCLUDE_PM_USE_STATE

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#ifdef	_INCLUDE_PM_USE_STATE

static	struct semaphore	pm_gpio_mutex ;
#define	_MUTEX_INIT()   	init_MUTEX( &pm_gpio_mutex )
#define	_MUTEX_LOCK()   	down( &pm_gpio_mutex )
#define	_MUTEX_UNLOCK()		up( &pm_gpio_mutex )

static	U16	_PMOdn = (U16) -1;
static	U16	_PMOut = (U16) -1;

#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
extern void MDrv_MICOM_Init(void);
extern U8 MDrv_MICOM_RegRead( U16 u16Addr );
extern B16 MDrv_MICOM_RegWrite(U16 u16Addr, U8 u8Data);

//-------------------------------------------------------------------------------------------------
/// GPIO chiptop initialization
/// @return None
/// @note   Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Init(eBOOT_TYPE type)
{
//dhjung LGE
	static is_emac_reset = 0;

#ifdef	_INCLUDE_PM_USE_STATE
	if(type == eCOLD_BOOT) {
	_MUTEX_INIT();
	}
#endif

	MHal_GPIO_Init();

#if 0 // LGE dreamer 080902
	MDrv_MICOM_Init();
	msleep(150);
#if 1
	/* 	drmyung LGE 080527 : for HDMI
		HDMI 두 채널을 위해서는 "2"로 설정되야함.
	*/
	MDrv_MICOM_RegWrite(0x106A, 2);// 1);
#endif
#endif

//dhjung LGE : phy reset have to be moved to chip_setup.c or
//gpio init have to be moved to first of drivers' init
//for booting time
	if (!is_emac_reset)
	{
		//Reset Emac PHY
		MHal_GPIO_REG(0x101ea1) &= ~BIT7;
		MHal_GPIO_REG(0x101e24) &= ~(BIT6|BIT5|BIT4);
		MHal_GPIO_REG(0x1422) &= ~(BIT4|BIT3|BIT2|BIT1|BIT0);
		MHal_GPIO_REG(0x101e07) &= ~(BIT2|BIT1|BIT0);

		MHal_GPIO_REG(0x1423) |= BIT3;
		//set out
		MHal_GPIO_REG(0x1423)  &= ~BIT3;
		//set output low
		MHal_GPIO_REG(0x1424) &= ~BIT3;
		//mdelay(20);
		mdelay(1);
		//set output high
		MHal_GPIO_REG(0x1424) |= BIT3;

		//dhjung LGE : Should be retained to '1' for somewhile
		//jjab   LGE : Remove delay because emac init was moved to user_initcall section.
		//           : It will guarantee sufficient delay.
		//mdelay(200);

		is_emac_reset = 1;
	}
}

EXPORT_SYMBOL(MDrv_GPIO_Init);

#ifdef	_INCLUDE_BOOTSTRAP

static int lcdmodel = 0;
static int __init check_lcdmodel(char *str)
{
	lcdmodel = 1;
}

early_param("lcdmodels", check_lcdmodel);

//	added for BOOTSTRAP(DISPLAY & MODULE TYPE) by dreamer@lge.com
//-------------------------------------------------------------------------------------------------
/// get BOOT STRAP status( DISPLAY & MODULE type )
/// @return U8	( 0x1? )
/// @note  	LG용으로 추가됨
//-------------------------------------------------------------------------------------------------
U8	MDrv_GPIO_GetBootStrap( void )
{
#ifdef _INCLUDE_NEWBOOTSTRAP

static	U8	_gBootStrap = 0xFF;

#define _MAX_NUM_OF_READ	3

U8	regVal1[_MAX_NUM_OF_READ];
U8	regVal2[_MAX_NUM_OF_READ];
U8	regVal3[_MAX_NUM_OF_READ];
U8	regVal4[_MAX_NUM_OF_READ];
U8	regVal5[_MAX_NUM_OF_READ];
U8	regVal6[_MAX_NUM_OF_READ];
U8	regVal7[_MAX_NUM_OF_READ];


U32 i;
U32 index;

/*reg. address					option index			    gpio		pin name			bit mask[chip]        bit mask[read]			hw low/high
			###################### LCD ######################
#define REG_GPIO6_IN			MODEL_OPT0 (0x101e50)  GPIO42    PAD_GPIO6	         (>>6: BIT6 - 0x40) - 0x40 (FRC OPT0)
#define REG_GPIO_PM5_IN 		MODEL_OPT1 (0x0e22)     GPIO11    PAD_GPIO_PM5       (>>5: BIT5 - 0x20) - 0x20 (Mini LVDS/ LVDS) 		high/low
#define REG_GPIO_PM8_IN 		MODEL_OPT2 (0x0e23)     GPIO14    PAD_GPIO_PM8       ( 	BIT0 - 0x01)  - 0x10 (DDR 512/ DDR256) 		high/low
#define REG_SAR1_IN 			MODEL_OPT3 (0x1425)     GPIO32    PAD_GPIO_SAR1      (>>1: BIT1 - 0x02) - 0x04 (FHD/ HD) 				high/low
#define REG_I2S_OUT_SD2_IN	MODEL_OPT4 (0x101e6b)  GPIO184  PAD_I2S_OUT_SD2   (>>2: BIT2 - 0x04) - 0x02 (FRC OPT1)
#define REG_I2S_OUT_SD3_IN	MODEL_OPT5 (0x101e6b)  GPIO185  PAD_I2S_OUT_SD3   (>>3: BIT3 - 0x08) - 0x01 (GIP/ Non-GIP panel) 	high/low
#define REG_GPIO_PM4_IN		MODEL_OPT6 (0x0e22)     GPIO10    PAD_GPIO_PM4        (>>4: BIT4 - 0x10) - 0x80 (OLED/LCD) 			high/low

	X X X X X X X X
	|  | | | |  | | |
	|  | | | |  | | [MODEL_OPT5] _NonGIP/GIP
	|  | | | |  | [MODEL_OPT4] _FRC_OPT1
	|  | | | |  [MODEL_OPT3] _HD/FHD
	|  | | | LCD/PDP
	|  | | [MODEL_OPT2] _DDR256/512
	|  | [MODEL_OPT1] _LVDS/MiniLVDS
	|  [MODEL_OPT0] _FRC_OPT0
	[MPDEL_OPT6] _LCD/OLED
	
			###################### PDP ######################
#define REG_GPIO6_IN			MODEL_OPT0 (0x101e50)  GPIO42    PAD_GPIO6		(>>6: BIT6 - 0x40) - 0x40 (LCD/ PDP)			high/low			=> LOW
#define REG_GPIO_PM5_IN 		MODEL_OPT1 (0x0e22)     GPIO11    PAD_GPIO_PM5		(>>5: BIT5 - 0x20) - 0x20 (XGA/ WXGA) 			high/low
#define REG_GPIO_PM8_IN 		MODEL_OPT2 (0x0e23)     GPIO14    PAD_GPIO_PM8		(         BIT0 - 0x01)  - 0x10 (LED_NORMAL/ LED_MOVING) high/low	=> HIGH
#define REG_SAR1_IN 			MODEL_OPT3 (0x1425)     GPIO32    PAD_GPIO_SAR1	(>>1: BIT1 - 0x02) - 0x04 (FHD/ HD) 			high/low			=> LOW

	X X X X X X X X
	|  | | | |  | | |
	|  | | | |  | | NONE
	|  | | | |  | NONE
	|  | | | |  [MODEL_OPT3] _HD/FHD
	|  | | | LCD/PDP
	|  | | [MODEL_OPT2] LED_NORMAL/LED_MOVING
	|  | [MODEL_OPT1] XGA/WXGA
	|  [MODEL_OPT0] LCD/PDP
	NONE
*/

if( _gBootStrap == 0xFF )
{
	index = 0;

	if(lcdmodel)
	{
		for( i = 0; i < (3 * _MAX_NUM_OF_READ); i++ )
		{
		   //for model option 0
		   MHal_GPIO_GPIO6_Set();
		   MHal_GPIO_GPIO6_Odn();
		   regVal1[index] =  MHal_GPIO_REG(REG_GPIO6_IN); //(MHal_GPIO_GPIO6_In()) ? 0x10 : 0x00;

		   //for model option 1
		   MHal_GPIO_GPIO_PM5_Odn();
		   regVal2[index] = MHal_GPIO_REG(REG_GPIO_PM5_IN); //(MHal_GPIO_GPIO_PM5_In()) ? 0x04 : 0x00;

		   //for model option 2
		   MHal_GPIO_GPIO_PM8_Odn();
		   regVal3[index] = MHal_GPIO_REG(REG_GPIO_PM8_IN); //(MHal_GPIO_GPIO_PM8_In()) ? 0x02 : 0x00;

		   //for model option 3
		   MHal_GPIO_SAR1_Set();
		   MHal_GPIO_SAR1_Odn();

		   regVal4[index] = MHal_GPIO_REG(REG_SAR1_IN); //(MHal_GPIO_SAR1_In()) ? 0x01 : 0x00;

		   //for model option 4
		   MHal_GPIO_I2S_OUT_SD2_Set();
		   MHal_GPIO_I2S_OUT_SD2_Odn();
		   regVal5[index] = MHal_GPIO_REG(REG_I2S_OUT_SD2_IN) ; //MHal_GPIO_I2S_OUT_SD2_In();

		   //for model option 5
		   MHal_GPIO_I2S_OUT_SD3_Set();
		   MHal_GPIO_I2S_OUT_SD3_Odn();
		   regVal6[index] = MHal_GPIO_REG(REG_I2S_OUT_SD3_IN) ; //MHal_GPIO_I2S_OUT_SD3_In();

		   //for model option 6
		   MHal_GPIO_GPIO_PM4_Set();
		   MHal_GPIO_GPIO_PM4_Odn();
		   regVal7[index] = MHal_GPIO_REG(REG_GPIO_PM4_IN) ;

			if( index >= _MAX_NUM_OF_READ - 1 ){	break; }
			else
			{
				if	(	(index > 0)
					&&	((regVal1[index] != regVal1[index-1]) ||
						 (regVal2[index] != regVal2[index-1]) ||
						 (regVal3[index] != regVal3[index-1]) ||
						 (regVal4[index] != regVal4[index-1]) ||
						 (regVal5[index] != regVal5[index-1]) ||
						 (regVal6[index] != regVal6[index-1]) ||
						 (regVal7[index] != regVal7[index-1])))
				{
					index = 0;
				}
				else
				{
					index ++;
				}
			}
		}

		_gBootStrap  = (regVal1[index] & 0x40) ?  0x40	: 0x00; //	GPIO42	[MODEL_OPT0] _FRC_OPT0
		_gBootStrap |= (regVal2[index] & 0x20) ? 0x20	: 0x00; //	GPIO11	[MODEL_OPT1] _LVDS/MiniLVDS
		_gBootStrap |= (regVal3[index] & 0x01) ? 0x10	: 0x00; //	GPIO14	[MODEL_OPT2] _DDR256/512
		_gBootStrap |= (regVal4[index] & 0x02) ? 0x04	: 0x00; //	GPIO32	[MODEL_OPT3] _HD/FHD
		_gBootStrap |= (regVal5[index] & 0x04) ? 0x02	: 0x00; //	GPIO184	[MODEL_OPT4] _FRC_OPT1
		_gBootStrap |= (regVal6[index] & 0x08) ? 0x01	: 0x00; //	GPIO185	[MODEL_OPT5] _NonGIP/GIP
		_gBootStrap |= (regVal7[index] & 0x10) ? 0x80   : 0x00; //      GPIO10 [MPDEL_OPT6] _LCD/OLED
		_gBootStrap |=  lcdmodel			   ? 0x08   : 0x00; //    LCD/PDP

		//chage gpio to output port
		MHal_GPIO_GPIO6_Oen();
		MHal_GPIO_GPIO_PM5_Oen();
		MHal_GPIO_GPIO_PM8_Oen();
		MHal_GPIO_SAR1_Oen();
		MHal_GPIO_I2S_OUT_SD2_Oen();
		MHal_GPIO_I2S_OUT_SD3_Oen();
		MHal_GPIO_GPIO_PM4_Oen();

		MHal_GPIO_REG(REG_I2S_OUT_SD2_SET) &= ~(BIT6); //I2S_OUT_SD2 port gpio set disable
	}
	else	// PDP
	{
		for( i = 0; i < (3 * _MAX_NUM_OF_READ); i++ )
		{
		   //for model option 0
		   MHal_GPIO_GPIO6_Set();
		   MHal_GPIO_GPIO6_Odn();
		   regVal1[index] =  MHal_GPIO_REG(REG_GPIO6_IN); //(MHal_GPIO_GPIO6_In()) ? 0x10 : 0x00;

		   //for model option 1
		   MHal_GPIO_GPIO_PM5_Odn();
		   regVal2[index] = MHal_GPIO_REG(REG_GPIO_PM5_IN); //(MHal_GPIO_GPIO_PM5_In()) ? 0x04 : 0x00;

		   //for model option 2
		   MHal_GPIO_GPIO_PM8_Odn();
		   regVal3[index] = MHal_GPIO_REG(REG_GPIO_PM8_IN); //(MHal_GPIO_GPIO_PM8_In()) ? 0x02 : 0x00;

		   //for model option 3
		   MHal_GPIO_SAR1_Set();
		   MHal_GPIO_SAR1_Odn();

		   regVal4[index] = MHal_GPIO_REG(REG_SAR1_IN); //(MHal_GPIO_SAR1_In()) ? 0x01 : 0x00;

			if( index >= _MAX_NUM_OF_READ - 1 ){	break; }
			else
			{
				if	(	(index > 0)
					&&	((regVal1[index] != regVal1[index-1]) ||
						 (regVal2[index] != regVal2[index-1]) ||
						 (regVal3[index] != regVal3[index-1]) ||
						 (regVal4[index] != regVal4[index-1])))
				{
					index = 0;
				}
				else
				{
					index ++;
				}
			}
		}

		_gBootStrap  = (regVal1[index] & 0x40) ?  0x40	: 0x00; //	GPIO42	[MODEL_OPT0] LCD/PDP
		_gBootStrap |= (regVal2[index] & 0x20) ? 0x20	: 0x00; //	GPIO11	[MODEL_OPT1] XGA/WXGA
		_gBootStrap |= (regVal3[index] & 0x01) ? 0x10	: 0x00; //	GPIO14	[MODEL_OPT2] LED_NORMAL/LED_MOVING
		_gBootStrap |= (regVal4[index] & 0x02) ? 0x04	: 0x00; //	GPIO32	[MODEL_OPT3] FHD/HD
		_gBootStrap |=  lcdmodel			   ? 0x08   : 0x00; //    LCD/PDP

		//chage gpio to output port
		MHal_GPIO_GPIO6_Oen();
		MHal_GPIO_GPIO_PM5_Oen();
		MHal_GPIO_GPIO_PM8_Oen();
		MHal_GPIO_SAR1_Oen();
	}

	printk( "BOOTSTRAP %02x(%d)\n", _gBootStrap, i );
}

return _gBootStrap;


#else

	static	U8	_gBootStrap = 0xFF;

	#define	_MAX_NUM_OF_READ	3

	U8	regVal1[_MAX_NUM_OF_READ];
	U8	regVal2[_MAX_NUM_OF_READ];
	U8  regVal3[_MAX_NUM_OF_READ];
	U8	regVal4[_MAX_NUM_OF_READ];

	U32	i;
	U32 index;

	/*	 GP2 model option
	#define REG_GPIO6_IN	(0x101e50) // GPIO42 PAD_GPIO6    (>>6: BIT6 - 0x40) - 0x10 (FRC /Not)
	#define REG_GPIO_PM5_IN (0x0e22)   // GPIO11 PAD_GPIO_PM5 (>>5: BIT5 - 0x20) - 0x04 (Mini LVDS/ LVDS)
	#define REG_GPIO_PM8_IN (0x0e23)   // GPIO14 PAD_GPIO_PM8 (     BIT0 - 0x01) - 0x02 (DDR 512/ DDR256)
	#define REG_SAR1_IN     (0x1425)   // GPIO32 PAD_GPIO_SAR1(>>1: BIT1 - 0x02) - 0x01 (FHD/ HD)
	*/

	if( _gBootStrap == 0xFF )
	{
		index = 0;

		for( i = 0; i < (3 * _MAX_NUM_OF_READ); i++ )
		{
		   //for model option 0
		   MHal_GPIO_GPIO6_Set();
		   MHal_GPIO_GPIO6_Odn();
		   regVal1[index] =  MHal_GPIO_REG(REG_GPIO6_IN); //(MHal_GPIO_GPIO6_In()) ? 0x10 : 0x00;

		   //for model option 1
		   MHal_GPIO_GPIO_PM5_Odn();
		   regVal2[index] = MHal_GPIO_REG(REG_GPIO_PM5_IN); //(MHal_GPIO_GPIO_PM5_In()) ? 0x04 : 0x00;

		   //for model option 2
		   MHal_GPIO_GPIO_PM8_Odn();
		   regVal3[index] = MHal_GPIO_REG(REG_GPIO_PM8_IN); //(MHal_GPIO_GPIO_PM8_In()) ? 0x02 : 0x00;

		   //for model option 3
		   MHal_GPIO_SAR1_Set();
		   MHal_GPIO_SAR1_Odn();

		   regVal4[index] = MHal_GPIO_REG(REG_SAR1_IN); //(MHal_GPIO_SAR1_In()) ? 0x01 : 0x00;

			if( index >= _MAX_NUM_OF_READ - 1 )
			{
				break;
			}
			else
			{

				if	(	(index > 0)
					&&	((regVal1[index] != regVal1[index-1]) ||
					     (regVal2[index] != regVal2[index-1]) ||
					     (regVal3[index] != regVal3[index-1]) ||
					     (regVal4[index] != regVal4[index-1])))
				{
					index = 0;
				}
				else
				{
					index ++;
				}
			}
		}

		_gBootStrap  = (regVal1[index] & 0x40) ? 0x10		: 0x00;	//	GPIO42 FRC
		_gBootStrap |= (regVal2[index] & 0x20) ? 0x04	: 0x00;	//	GPIO11 mini LVDS
		_gBootStrap |= (regVal3[index] & 0x01) ? 0x02	: 0x00;	//	GPIO14 DDR 256/512
		_gBootStrap |= (regVal4[index] & 0x02) ? 0x01	: 0x00; 	//	GPIO32 HD/FHD
		_gBootStrap |=  lcdmodel			   ? 0x08   : 0x00; //    LCD/PDP

		//chage gpio to output port
		MHal_GPIO_GPIO6_Oen();
		MHal_GPIO_GPIO_PM5_Oen();
		MHal_GPIO_GPIO_PM8_Oen();
		MHal_GPIO_SAR1_Oen();

		printk( "BOOTSTRAP %02x(%d)\n", _gBootStrap, i );
	}

	return _gBootStrap;
#endif
}
EXPORT_SYMBOL(MDrv_GPIO_GetBootStrap);
#endif	/* #ifdef	_INCLUDE_BOOTSTRAP */

//-------------------------------------------------------------------------------------------------
/// select one pad to set
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO)
{
//    U8  u8RegTemp;

    switch(u8IndexGPIO)
    {
        case PAD_PM_SPI_CZ:
            MHal_GPIO_PM_SPI_CZ_Set();
        break;
        case PAD_PM_SPI_CK:
            MHal_GPIO_PM_SPI_CK_Set();
        break;
        case PAD_PM_SPI_DI:
            MHal_GPIO_PM_SPI_DI_Set();
        break;
        case PAD_PM_SPI_DO:
            MHal_GPIO_PM_SPI_DO_Set();
        break;
        case PAD_IRIN:
            MHal_GPIO_IRIN_Set();
        break;
        case PAD_CEC:
            MHal_GPIO_CEC_Set();
        break;
        case PAD_GPIO_PM0:
            MHal_GPIO_GPIO_PM0_Set();
        break;
        case PAD_GPIO_PM1:
            MHal_GPIO_GPIO_PM1_Set();
        break;
        case PAD_GPIO_PM2:
            MHal_GPIO_GPIO_PM2_Set();
        break;
        case PAD_GPIO_PM3:
            MHal_GPIO_GPIO_PM3_Set();
        break;
        case PAD_GPIO_PM4:
            MHal_GPIO_GPIO_PM4_Set();
        break;
        case PAD_GPIO_PM5:
            MHal_GPIO_GPIO_PM5_Set();
        break;
        case PAD_GPIO_PM6:
            MHal_GPIO_GPIO_PM6_Set();
        break;
        case PAD_GPIO_PM7:
            MHal_GPIO_GPIO_PM7_Set();
        break;
        case PAD_GPIO_PM8:
            MHal_GPIO_GPIO_PM8_Set();
        break;
        case PAD_GPIO_PM9:
            MHal_GPIO_GPIO_PM9_Set();
        break;
        case PAD_GPIO_PM10:
            MHal_GPIO_GPIO_PM10_Set();
        break;
        case PAD_GPIO_PM11:
            MHal_GPIO_GPIO_PM11_Set();
        break;
        case PAD_GPIO_PM12:
            MHal_GPIO_GPIO_PM12_Set();
        break;
        case PAD_HOTPLUGA:
            MHal_GPIO_HOTPLUGA_Set();
        break;
        case PAD_HOTPLUGB:
            MHal_GPIO_HOTPLUGB_Set();
        break;
        case PAD_HOTPLUGC:
            MHal_GPIO_HOTPLUGC_Set();
        break;
        case PAD_HOTPLUGD:
            MHal_GPIO_HOTPLUGD_Set();
        break;
        case PAD_DDCDA_CK:
            MHal_GPIO_DDCDA_CK_Set();
        break;
        case PAD_DDCDA_DA:
            MHal_GPIO_DDCDA_DA_Set();
        break;
        case PAD_DDCDB_CK:
            MHal_GPIO_DDCDB_CK_Set();
        break;
        case PAD_DDCDB_DA:
            MHal_GPIO_DDCDB_DA_Set();
        break;
        case PAD_DDCDC_CK:
            MHal_GPIO_DDCDC_CK_Set();
        break;
        case PAD_DDCDC_DA:
            MHal_GPIO_DDCDC_DA_Set();
        break;
        case PAD_DDCDD_CK:
            MHal_GPIO_DDCDD_CK_Set();
        break;
        case PAD_DDCDD_DA:
            MHal_GPIO_DDCDD_DA_Set();
        break;
        case PAD_SAR0:
            MHal_GPIO_SAR0_Set();
        break;
        case PAD_SAR1:
            MHal_GPIO_SAR1_Set();
        break;
        case PAD_SAR2:
            MHal_GPIO_SAR2_Set();
        break;
        case PAD_SAR3:
            MHal_GPIO_SAR3_Set();
        break;
        case PAD_SAR4:
            MHal_GPIO_SAR4_Set();
        break;
        case PAD_GPIO0:
            MHal_GPIO_GPIO0_Set();
        break;
        case PAD_GPIO1:
            MHal_GPIO_GPIO1_Set();
        break;
        case PAD_GPIO2:
            MHal_GPIO_GPIO2_Set();
        break;
        case PAD_GPIO3:
            MHal_GPIO_GPIO3_Set();
        break;
        case PAD_GPIO4:
            MHal_GPIO_GPIO4_Set();
        break;
        case PAD_GPIO5:
            MHal_GPIO_GPIO5_Set();
        break;
        case PAD_GPIO6:
            MHal_GPIO_GPIO6_Set();
        break;
        case PAD_GPIO7:
            MHal_GPIO_GPIO7_Set();
        break;
        case PAD_GPIO8:
            MHal_GPIO_GPIO8_Set();
        break;
        case PAD_GPIO9:
            MHal_GPIO_GPIO9_Set();
        break;
        case PAD_GPIO10:
            MHal_GPIO_GPIO10_Set();
        break;
        case PAD_GPIO11:
            MHal_GPIO_GPIO11_Set();
        break;
        case PAD_GPIO12:
            MHal_GPIO_GPIO12_Set();
        break;
        case PAD_GPIO13:
            MHal_GPIO_GPIO13_Set();
        break;
        case PAD_GPIO14:
            MHal_GPIO_GPIO14_Set();
        break;
        case PAD_GPIO15:
            MHal_GPIO_GPIO15_Set();
        break;
        case PAD_GPIO16:
            MHal_GPIO_GPIO16_Set();
        break;
        case PAD_GPIO17:
            MHal_GPIO_GPIO17_Set();
        break;
        case PAD_GPIO18:
            MHal_GPIO_GPIO18_Set();
        break;
        case PAD_GPIO19:
            MHal_GPIO_GPIO19_Set();
        break;
        case PAD_GPIO20:
            MHal_GPIO_GPIO20_Set();
        break;
        case PAD_GPIO21:
            MHal_GPIO_GPIO21_Set();
        break;
        case PAD_GPIO22:
            MHal_GPIO_GPIO22_Set();
        break;
        case PAD_GPIO23:
            MHal_GPIO_GPIO23_Set();
        break;
        case PAD_GPIO24:
            MHal_GPIO_GPIO24_Set();
        break;
        case PAD_GPIO25:
            MHal_GPIO_GPIO25_Set();
        break;
        case PAD_GPIO26:
            MHal_GPIO_GPIO26_Set();
        break;
        case PAD_GPIO27:
            MHal_GPIO_GPIO27_Set();
        break;
        case PAD_UART_RX2:
            MHal_GPIO_UART_RX2_Set();
        break;
        case PAD_UART_TX2:
            MHal_GPIO_UART_TX2_Set();
        break;
        case PAD_PWM0:
            MHal_GPIO_PWM0_Set();
        break;
        case PAD_PWM1:
            MHal_GPIO_PWM1_Set();
        break;
        case PAD_PWM2:
            MHal_GPIO_PWM2_Set();
        break;
        case PAD_PWM3:
            MHal_GPIO_PWM3_Set();
        break;
        case PAD_PWM4:
            MHal_GPIO_PWM4_Set();
        break;
        case PAD_DDCR_DA:
            MHal_GPIO_DDCR_DA_Set();
        break;
        case PAD_DDCR_CK:
            MHal_GPIO_DDCR_CK_Set();
        break;
        case PAD_TGPIO0:
            MHal_GPIO_TGPIO0_Set();
        break;
        case PAD_TGPIO1:
            MHal_GPIO_TGPIO1_Set();
        break;
        case PAD_TGPIO2:
            MHal_GPIO_TGPIO2_Set();
        break;
        case PAD_TGPIO3:
            MHal_GPIO_TGPIO3_Set();
        break;
        case PAD_TS0_D0:
            MHal_GPIO_TS0_D0_Set();
        break;
        case PAD_TS0_D1:
            MHal_GPIO_TS0_D1_Set();
        break;
        case PAD_TS0_D2:
            MHal_GPIO_TS0_D2_Set();
        break;
        case PAD_TS0_D3:
            MHal_GPIO_TS0_D3_Set();
        break;
        case PAD_TS0_D4:
            MHal_GPIO_TS0_D4_Set();
        break;

        case PAD_TS0_D5:
            MHal_GPIO_TS0_D5_Set();
        break;
        case PAD_TS0_D6:
            MHal_GPIO_TS0_D6_Set();
        break;
        case PAD_TS0_D7:
            MHal_GPIO_TS0_D7_Set();
        break;
        case PAD_TS0_VLD:
            MHal_GPIO_TS0_VLD_Set();
        break;
        case PAD_TS0_SYNC:
            MHal_GPIO_TS0_SYNC_Set();
        break;
        case PAD_TS0_CLK:
            MHal_GPIO_TS0_CLK_Set();
        break;
        case PAD_TS1_D0:
            MHal_GPIO_TS1_D0_Set();
        break;
        case PAD_TS1_D1:
            MHal_GPIO_TS1_D1_Set();
        break;
        case PAD_TS1_D2:
            MHal_GPIO_TS1_D2_Set();
        break;
        case PAD_TS1_D3:
            MHal_GPIO_TS1_D3_Set();
        break;
        case PAD_TS1_D4:
            MHal_GPIO_TS1_D4_Set();
        break;
        case PAD_TS1_D5:
            MHal_GPIO_TS1_D5_Set();
        break;
        case PAD_TS1_D6:
            MHal_GPIO_TS1_D6_Set();
        break;
        case PAD_TS1_D7:
            MHal_GPIO_TS1_D7_Set();
        break;
        case PAD_TS1_VLD:
            MHal_GPIO_TS1_VLD_Set();
        break;
        case PAD_TS1_SYNC:
            MHal_GPIO_TS1_SYNC_Set();
        break;
        case PAD_TS1_CLK:
            MHal_GPIO_TS1_CLK_Set();
        break;
        case PAD_PCM_A4:
            MHal_GPIO_PCM_A4_Set();
        break;
        case PAD_PCM_WAIT_N:
            MHal_GPIO_PCM_WAIT_N_Set();
        break;
        case PAD_PCM_A5:
            MHal_GPIO_PCM_A5_Set();
        break;
        case PAD_PCM_A6:
            MHal_GPIO_PCM_A6_Set();
        break;
        case PAD_PCM_A7:
            MHal_GPIO_PCM_A7_Set();
        break;
        case PAD_PCM_A12:
            MHal_GPIO_PCM_A12_Set();
        break;
        case PAD_PCM_IRQA_N:
            MHal_GPIO_PCM_IRQA_N_Set();
        break;
        case PAD_PCM_A14:
            MHal_GPIO_PCM_A14_Set();
        break;
        case PAD_PCM_A13:
            MHal_GPIO_PCM_A13_Set();
        break;
        case PAD_PCM_A8:
            MHal_GPIO_PCM_A8_Set();
        break;
        case PAD_PCM_IOWR_N:
            MHal_GPIO_PCM_IOWR_N_Set();
        break;
        case PAD_PCM_A9:
            MHal_GPIO_PCM_A9_Set();
        break;
        case PAD_PCM_IORD_N:
            MHal_GPIO_PCM_IORD_N_Set();
        break;
        case PAD_PCM_A11:
            MHal_GPIO_PCM_A11_Set();
        break;
        case PAD_PCM_OE_N:
            MHal_GPIO_PCM_OE_N_Set();
        break;
        case PAD_PCM_A10:
            MHal_GPIO_PCM_A10_Set();
        break;
        case PAD_PCM_CE_N:
            MHal_GPIO_PCM_CE_N_Set();
        break;
        case PAD_PCM_D7:
            MHal_GPIO_PCM_D7_Set();
        break;
        case PAD_PCM_D6:
            MHal_GPIO_PCM_D6_Set();
        break;
        case PAD_PCM_D5:
            MHal_GPIO_PCM_D5_Set();
        break;
        case PAD_PCM_D4:
            MHal_GPIO_PCM_D4_Set();
        break;
        case PAD_PCM_D3:
            MHal_GPIO_PCM_D3_Set();
        break;
        case PAD_PCM_A3:
            MHal_GPIO_PCM_A3_Set();
        break;
        case PAD_PCM_A2:
            MHal_GPIO_PCM_A2_Set();
        break;
        case PAD_PCM_REG_N:
            MHal_GPIO_PCM_REG_N_Set();
        break;
        case PAD_PCM_A1:
            MHal_GPIO_PCM_A1_Set();
        break;
        case PAD_PCM_A0:
            MHal_GPIO_PCM_A0_Set();
        break;
        case PAD_PCM_D0:
            MHal_GPIO_PCM_D0_Set();
        break;
        case PAD_PCM_D1:
            MHal_GPIO_PCM_D1_Set();
        break;
        case PAD_PCM_D2:
            MHal_GPIO_PCM_D2_Set();
        break;
        case PAD_PCM_RESET:
            MHal_GPIO_PCM_RESET_Set();
        break;
        case PAD_PCM_CD_N:
            MHal_GPIO_PCM_CD_N_Set();
        break;
        case PAD_PCM2_CE_N:
            MHal_GPIO_PCM2_CE_N_Set();
        break;
        case PAD_PCM2_IRQA_N:
            MHal_GPIO_PCM2_IRQA_N_Set();
        break;
        case PAD_PCM2_WAIT_N:
            MHal_GPIO_PCM2_WAIT_N_Set();
        break;
        case PAD_PCM2_RESET:
            MHal_GPIO_PCM2_RESET_Set();
        break;
        case PAD_PCM2_CD_N:
            MHal_GPIO_PCM2_CD_N_Set();
        break;
        case PAD_PF_AD15:
            MHal_GPIO_PF_AD15_Set();
        break;
        case PAD_PF_CE0Z:
            MHal_GPIO_PF_CE0Z_Set();
        break;
        case PAD_PF_CE1Z:
            MHal_GPIO_PF_CE1Z_Set();
        break;
        case PAD_PF_OEZ:
            MHal_GPIO_PF_OEZ_Set();
        break;
        case PAD_PF_WEZ:
            MHal_GPIO_PF_WEZ_Set();
        break;
        case PAD_PF_ALE:
            MHal_GPIO_PF_ALE_Set();
        break;
        case PAD_F_RBZ:
            MHal_GPIO_F_RBZ_Set();
        break;
        case PAD_TCON0:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif
				{
					MHal_GPIO_GPIO48_Set();
				}
				else
				{
					MHal_GPIO_TCON0_Set();
				}
			}
			else	// PDP
				MHal_GPIO_TCON0_Set();
		}
#else
			MHal_GPIO_TCON0_Set();
#endif
        break;
        case PAD_TCON1:
            MHal_GPIO_TCON1_Set();
        break;
        case PAD_TCON2:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif
				{
					MHal_GPIO_GPIO49_Set();
				}
				else
				{
					MHal_GPIO_TCON2_Set();
				}
			}
			else	// PDP
				MHal_GPIO_TCON2_Set();
		}
#else
			MHal_GPIO_TCON2_Set();
#endif
        break;
        case PAD_TCON3:
            MHal_GPIO_TCON3_Set();
        break;
        case PAD_TCON4:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO53_Set();
				}
				else
				{
					 MHal_GPIO_TCON4_Set();
				}
			}
			else	//PDP
				MHal_GPIO_TCON4_Set();
		}
#else
			MHal_GPIO_TCON4_Set();
#endif
        break;
        case PAD_TCON5:
            MHal_GPIO_TCON5_Set();
        break;
        case PAD_TCON6:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO52_Set();
				}
				else
				{
					 MHal_GPIO_TCON6_Set();
				}
			}
			else
				MHal_GPIO_TCON6_Set();
		}
#else
			MHal_GPIO_TCON6_Set();
#endif
        break;
        case PAD_TCON7:
            MHal_GPIO_TCON7_Set();
        break;
        case PAD_TCON8:
							//balup_090907
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO47_Set();
				}
				else
				{
					MHal_GPIO_TCON8_Set();
				}
			}
			else	// PDP
				MHal_GPIO_TCON8_Set();
		}
#else
			MHal_GPIO_TCON8_Set();
#endif
        break;
        case PAD_TCON9:
            MHal_GPIO_TCON9_Set();
        break;
        case PAD_TCON10:
            MHal_GPIO_TCON10_Set();
        break;
        case PAD_TCON11:
            MHal_GPIO_TCON11_Set();
        break;
        case PAD_TCON12:
            MHal_GPIO_TCON12_Set();
        break;
        case PAD_TCON13:
            MHal_GPIO_TCON13_Set();
        break;
        case PAD_TCON14:
            MHal_GPIO_TCON14_Set();
        break;
        case PAD_TCON15:
            MHal_GPIO_TCON15_Set();
        break;
        case PAD_TCON16:
            MHal_GPIO_TCON16_Set();
        break;
        case PAD_TCON17:
            MHal_GPIO_TCON17_Set();
        break;
        case PAD_TCON18:
            MHal_GPIO_TCON18_Set();
        break;
        case PAD_TCON19:
            MHal_GPIO_TCON19_Set();
        break;
        case PAD_TCON20:
            MHal_GPIO_TCON20_Set();
        break;
        case PAD_TCON21:
            MHal_GPIO_TCON21_Set();
        break;
        case PAD_ET_COL:
            MHal_GPIO_ET_COL_Set();
        break;
        case PAD_ET_TXD1:
            MHal_GPIO_ET_TXD1_Set();
        break;
        case PAD_ET_TXD0:
            MHal_GPIO_ET_TXD0_Set();
        break;
        case PAD_ET_TX_EN:
            MHal_GPIO_ET_TX_EN_Set();
        break;
        case PAD_ET_TX_CLK:
            MHal_GPIO_ET_TX_CLK_Set();
        break;
        case PAD_ET_RXD0:
            MHal_GPIO_ET_RXD0_Set();
        break;
        case PAD_ET_RXD1:
            MHal_GPIO_ET_RXD1_Set();
        break;
        case PAD_ET_MDC:
            MHal_GPIO_ET_MDC_Set();
        break;
        case PAD_ET_EDIO:
            MHal_GPIO_ET_EDIO_Set();
        break;
        case PAD_I2S_IN_WS:
            MHal_GPIO_I2S_IN_WS_Set();
        break;
        case PAD_I2S_IN_BCK:
            MHal_GPIO_I2S_IN_BCK_Set();
        break;
        case PAD_I2S_IN_SD:
            MHal_GPIO_I2S_IN_SD_Set();
        break;
        case PAD_SPDIF_IN:
            MHal_GPIO_SPDIF_IN_Set();
        break;
        case PAD_SPDIF_OUT:
            MHal_GPIO_SPDIF_OUT_Set();
        break;
        case PAD_I2S_OUT_MCK:
            MHal_GPIO_I2S_OUT_MCK_Set();
        break;
        case PAD_I2S_OUT_WS:
            MHal_GPIO_I2S_OUT_WS_Set();
        break;
        case PAD_I2S_OUT_BCK:
            MHal_GPIO_I2S_OUT_BCK_Set();
        break;
        case PAD_I2S_OUT_SD:
            MHal_GPIO_I2S_OUT_SD_Set();
        break;
        case PAD_I2S_OUT_SD1:
            MHal_GPIO_I2S_OUT_SD1_Set();
        break;
		//balup_090922
        case PAD_I2S_OUT_SD2:	// Model Option 4
        {
        	U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			
			if((bootstrap & 0x08) != 0x08)	// PDP
            	MHal_GPIO_I2S_OUT_SD2_Set();
        }
        break;
        case PAD_I2S_OUT_SD3:	// Model Option 5
        {
        	U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) != 0x08)	// PDP
            	MHal_GPIO_I2S_OUT_SD3_Set();
        }
        break;
        case PAD_B_ODD_0:
            MHal_GPIO_B_ODD_0_Set();
        break;
        case PAD_B_ODD_1:
            MHal_GPIO_B_ODD_1_Set();
        break;
        case PAD_B_ODD_2:
            MHal_GPIO_B_ODD_2_Set();
        break;
        case PAD_B_ODD_3:
            MHal_GPIO_B_ODD_3_Set();
        break;
        case PAD_B_ODD_4:
            MHal_GPIO_B_ODD_4_Set();
        break;
        case PAD_B_ODD_5:
            MHal_GPIO_B_ODD_5_Set();
        break;
        case PAD_B_ODD_6:
            MHal_GPIO_B_ODD_6_Set();
        break;
        case PAD_B_ODD_7:
            MHal_GPIO_B_ODD_7_Set();
        break;
        case PAD_G_ODD_0:
            MHal_GPIO_G_ODD_0_Set();
        break;
        case PAD_G_ODD_1:
            MHal_GPIO_G_ODD_1_Set();
        break;
        case PAD_G_ODD_2:
            MHal_GPIO_G_ODD_2_Set();
        break;
        case PAD_G_ODD_3:
            MHal_GPIO_G_ODD_3_Set();
        break;
        case PAD_G_ODD_4:
            MHal_GPIO_G_ODD_4_Set();
        break;
        case PAD_G_ODD_5:
            MHal_GPIO_G_ODD_5_Set();
        break;
        case PAD_G_ODD_6:
            MHal_GPIO_G_ODD_6_Set();
        break;
        case PAD_G_ODD_7:
            MHal_GPIO_G_ODD_7_Set();
        break;
        case PAD_R_ODD_0:
            MHal_GPIO_R_ODD_0_Set();
        break;
        case PAD_R_ODD_1:
            MHal_GPIO_R_ODD_1_Set();
        break;
        case PAD_R_ODD_2:
            MHal_GPIO_R_ODD_2_Set();
        break;
        case PAD_R_ODD_3:
            MHal_GPIO_R_ODD_3_Set();
        break;
        case PAD_R_ODD_4:
            MHal_GPIO_R_ODD_4_Set();
        break;
        case PAD_R_ODD_5:
            MHal_GPIO_R_ODD_5_Set();
        break;
        case PAD_R_ODD_6:
            MHal_GPIO_R_ODD_6_Set();
        break;
        case PAD_R_ODD_7:
            MHal_GPIO_R_ODD_7_Set();
        break;
        case PAD_mini_LVDS_0:
            MHal_GPIO_mini_LVDS_0_Set();
        break;
        case PAD_mini_LVDS_1:
            MHal_GPIO_mini_LVDS_1_Set();
        break;
        case PAD_mini_LVDS_2:
            MHal_GPIO_mini_LVDS_2_Set();
        break;
        case PAD_mini_LVDS_3:
            MHal_GPIO_mini_LVDS_3_Set();
        break;
        case PAD_mini_LVDS_4:
            MHal_GPIO_mini_LVDS_4_Set();
        break;
        case PAD_mini_LVDS_5:
            MHal_GPIO_mini_LVDS_5_Set();
        break;
        case PAD_mini_LVDS_6:
            MHal_GPIO_mini_LVDS_6_Set();
        break;
        case PAD_mini_LVDS_7:
            MHal_GPIO_mini_LVDS_7_Set();
        break;
        case PAD_LCK:
            MHal_GPIO_LCK_Set();
        break;
        case PAD_LDE:
            MHal_GPIO_LDE_Set();
        break;
        case PAD_LHSYNC:
            MHal_GPIO_LHSYNC_Set();
        break;
        case PAD_LVSYNC:
            MHal_GPIO_LVSYNC_Set();
        break;
        case PAD_PCM_WE_N:
            MHal_GPIO_PCM_WE_N_Set();
        break;



        default:
            break;
    }
}
EXPORT_SYMBOL(MDrv_GPIO_Pad_Set);

//-------------------------------------------------------------------------------------------------
/// enable output for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Oen(U8 u8IndexGPIO)
{
    //U8  u8RegTemp;

    switch(u8IndexGPIO)
    {
        case PAD_PM_SPI_CZ:
            MHal_GPIO_PM_SPI_CZ_Oen();
        break;
        case PAD_PM_SPI_CK:
            MHal_GPIO_PM_SPI_CK_Oen();
        break;
        case PAD_PM_SPI_DI:
            MHal_GPIO_PM_SPI_DI_Oen();
        break;
        case PAD_PM_SPI_DO:
            MHal_GPIO_PM_SPI_DO_Oen();
        break;
        case PAD_IRIN:
            MHal_GPIO_IRIN_Oen();
        break;
        case PAD_CEC:
            MHal_GPIO_CEC_Oen();
        break;
        case PAD_GPIO_PM0:
            MHal_GPIO_GPIO_PM0_Oen();
        break;
        case PAD_GPIO_PM1:
            MHal_GPIO_GPIO_PM1_Oen();
        break;
        case PAD_GPIO_PM2:
            MHal_GPIO_GPIO_PM2_Oen();
        break;
        case PAD_GPIO_PM3:
            MHal_GPIO_GPIO_PM3_Oen();
        break;
        case PAD_GPIO_PM4:
            MHal_GPIO_GPIO_PM4_Oen();
        break;
        case PAD_GPIO_PM5:
            MHal_GPIO_GPIO_PM5_Oen();
        break;
        case PAD_GPIO_PM6:
            MHal_GPIO_GPIO_PM6_Oen();
        break;
        case PAD_GPIO_PM7:
            MHal_GPIO_GPIO_PM7_Oen();
        break;
        case PAD_GPIO_PM8:
            MHal_GPIO_GPIO_PM8_Oen();
        break;
        case PAD_GPIO_PM9:
            MHal_GPIO_GPIO_PM9_Oen();
        break;
        case PAD_GPIO_PM10:
            MHal_GPIO_GPIO_PM10_Oen();
        break;
        case PAD_GPIO_PM11:
            MHal_GPIO_GPIO_PM11_Oen();
        break;
        case PAD_GPIO_PM12:
            MHal_GPIO_GPIO_PM12_Oen();
        break;
        case PAD_HOTPLUGA:
            MHal_GPIO_HOTPLUGA_Oen();
        break;
        case PAD_HOTPLUGB:
            MHal_GPIO_HOTPLUGB_Oen();
        break;
        case PAD_HOTPLUGC:
            MHal_GPIO_HOTPLUGC_Oen();
        break;
        case PAD_HOTPLUGD:
            MHal_GPIO_HOTPLUGD_Oen();
        break;
        case PAD_DDCDA_CK:
            MHal_GPIO_DDCDA_CK_Oen();
        break;
        case PAD_DDCDA_DA:
            MHal_GPIO_DDCDA_DA_Oen();
        break;
        case PAD_DDCDB_CK:
            MHal_GPIO_DDCDB_CK_Oen();
        break;
        case PAD_DDCDB_DA:
            MHal_GPIO_DDCDB_DA_Oen();
        break;
        case PAD_DDCDC_CK:
            MHal_GPIO_DDCDC_CK_Oen();
        break;
        case PAD_DDCDC_DA:
            MHal_GPIO_DDCDC_DA_Oen();
        break;
        case PAD_DDCDD_CK:
            MHal_GPIO_DDCDD_CK_Oen();
        break;
        case PAD_DDCDD_DA:
            MHal_GPIO_DDCDD_DA_Oen();
        break;
        case PAD_SAR0:
            MHal_GPIO_SAR0_Oen();
        break;
        case PAD_SAR1:
            MHal_GPIO_SAR1_Oen();
        break;
        case PAD_SAR2:
            MHal_GPIO_SAR2_Oen();
        break;
        case PAD_SAR3:
            MHal_GPIO_SAR3_Oen();
        break;
        case PAD_SAR4:
            MHal_GPIO_SAR4_Oen();
        break;
        case PAD_GPIO0:
            MHal_GPIO_GPIO0_Oen();
        break;
        case PAD_GPIO1:
            MHal_GPIO_GPIO1_Oen();
        break;
        case PAD_GPIO2:
            MHal_GPIO_GPIO2_Oen();
        break;
        case PAD_GPIO3:
            MHal_GPIO_GPIO3_Oen();
        break;
        case PAD_GPIO4:
            MHal_GPIO_GPIO4_Oen();
        break;
        case PAD_GPIO5:
            MHal_GPIO_GPIO5_Oen();
        break;
        case PAD_GPIO6:
            MHal_GPIO_GPIO6_Oen();
        break;
        case PAD_GPIO7:
            MHal_GPIO_GPIO7_Oen();
        break;
        case PAD_GPIO8:
            MHal_GPIO_GPIO8_Oen();
        break;
        case PAD_GPIO9:
            MHal_GPIO_GPIO9_Oen();
        break;
        case PAD_GPIO10:
            MHal_GPIO_GPIO10_Oen();
        break;
        case PAD_GPIO11:
            MHal_GPIO_GPIO11_Oen();
        break;
        case PAD_GPIO12:
            MHal_GPIO_GPIO12_Oen();
        break;
        case PAD_GPIO13:
            MHal_GPIO_GPIO13_Oen();
        break;
        case PAD_GPIO14:
            MHal_GPIO_GPIO14_Oen();
        break;
        case PAD_GPIO15:
            MHal_GPIO_GPIO15_Oen();
        break;
        case PAD_GPIO16:
            MHal_GPIO_GPIO16_Oen();
        break;
        case PAD_GPIO17:
            MHal_GPIO_GPIO17_Oen();
        break;
        case PAD_GPIO18:
            MHal_GPIO_GPIO18_Oen();
        break;
        case PAD_GPIO19:
            MHal_GPIO_GPIO19_Oen();
        break;
        case PAD_GPIO20:
            MHal_GPIO_GPIO20_Oen();
        break;
        case PAD_GPIO21:
            MHal_GPIO_GPIO21_Oen();
        break;
        case PAD_GPIO22:
            MHal_GPIO_GPIO22_Oen();
        break;
        case PAD_GPIO23:
            MHal_GPIO_GPIO23_Oen();
        break;
        case PAD_GPIO24:
            MHal_GPIO_GPIO24_Oen();
        break;
        case PAD_GPIO25:
            MHal_GPIO_GPIO25_Oen();
        break;
        case PAD_GPIO26:
            MHal_GPIO_GPIO26_Oen();
        break;
        case PAD_GPIO27:
            MHal_GPIO_GPIO27_Oen();
        break;
        case PAD_UART_RX2:
            MHal_GPIO_UART_RX2_Oen();
        break;
        case PAD_UART_TX2:
            MHal_GPIO_UART_TX2_Oen();
        break;
        case PAD_PWM0:
            MHal_GPIO_PWM0_Oen();
        break;
        case PAD_PWM1:
            MHal_GPIO_PWM1_Oen();
        break;
        case PAD_PWM2:
            MHal_GPIO_PWM2_Oen();
        break;
        case PAD_PWM3:
            MHal_GPIO_PWM3_Oen();
        break;
        case PAD_PWM4:
            MHal_GPIO_PWM4_Oen();
        break;
        case PAD_DDCR_DA:
            MHal_GPIO_DDCR_DA_Oen();
        break;
        case PAD_DDCR_CK:
            MHal_GPIO_DDCR_CK_Oen();
        break;
        case PAD_TGPIO0:
            MHal_GPIO_TGPIO0_Oen();
        break;
        case PAD_TGPIO1:
            MHal_GPIO_TGPIO1_Oen();
        break;
        case PAD_TGPIO2:
            MHal_GPIO_TGPIO2_Oen();
        break;
        case PAD_TGPIO3:
            MHal_GPIO_TGPIO3_Oen();
        break;
        case PAD_TS0_D0:
            MHal_GPIO_TS0_D0_Oen();
        break;
        case PAD_TS0_D1:
            MHal_GPIO_TS0_D1_Oen();
        break;
        case PAD_TS0_D2:
            MHal_GPIO_TS0_D2_Oen();
        break;
        case PAD_TS0_D3:
            MHal_GPIO_TS0_D3_Oen();
        break;
        case PAD_TS0_D4:
            MHal_GPIO_TS0_D4_Oen();
        break;

        case PAD_TS0_D5:
            MHal_GPIO_TS0_D5_Oen();
        break;
        case PAD_TS0_D6:
            MHal_GPIO_TS0_D6_Oen();
        break;
        case PAD_TS0_D7:
            MHal_GPIO_TS0_D7_Oen();
        break;
        case PAD_TS0_VLD:
            MHal_GPIO_TS0_VLD_Oen();
        break;
        case PAD_TS0_SYNC:
            MHal_GPIO_TS0_SYNC_Oen();
        break;
        case PAD_TS0_CLK:
            MHal_GPIO_TS0_CLK_Oen();
        break;
        case PAD_TS1_D0:
            MHal_GPIO_TS1_D0_Oen();
        break;
        case PAD_TS1_D1:
            MHal_GPIO_TS1_D1_Oen();
        break;
        case PAD_TS1_D2:
            MHal_GPIO_TS1_D2_Oen();
        break;
        case PAD_TS1_D3:
            MHal_GPIO_TS1_D3_Oen();
        break;
        case PAD_TS1_D4:
            MHal_GPIO_TS1_D4_Oen();
        break;
        case PAD_TS1_D5:
            MHal_GPIO_TS1_D5_Oen();
        break;
        case PAD_TS1_D6:
            MHal_GPIO_TS1_D6_Oen();
        break;
        case PAD_TS1_D7:
            MHal_GPIO_TS1_D7_Oen();
        break;
        case PAD_TS1_VLD:
            MHal_GPIO_TS1_VLD_Oen();
        break;
        case PAD_TS1_SYNC:
            MHal_GPIO_TS1_SYNC_Oen();
        break;
        case PAD_TS1_CLK:
            MHal_GPIO_TS1_CLK_Oen();
        break;
        case PAD_PCM_A4:
            MHal_GPIO_PCM_A4_Oen();
        break;
        case PAD_PCM_WAIT_N:
            MHal_GPIO_PCM_WAIT_N_Oen();
        break;
        case PAD_PCM_A5:
            MHal_GPIO_PCM_A5_Oen();
        break;
        case PAD_PCM_A6:
            MHal_GPIO_PCM_A6_Oen();
        break;
        case PAD_PCM_A7:
            MHal_GPIO_PCM_A7_Oen();
        break;
        case PAD_PCM_A12:
            MHal_GPIO_PCM_A12_Oen();
        break;
        case PAD_PCM_IRQA_N:
            MHal_GPIO_PCM_IRQA_N_Oen();
        break;
        case PAD_PCM_A14:
            MHal_GPIO_PCM_A14_Oen();
        break;
        case PAD_PCM_A13:
            MHal_GPIO_PCM_A13_Oen();
        break;
        case PAD_PCM_A8:
            MHal_GPIO_PCM_A8_Oen();
        break;
        case PAD_PCM_IOWR_N:
            MHal_GPIO_PCM_IOWR_N_Oen();
        break;
        case PAD_PCM_A9:
            MHal_GPIO_PCM_A9_Oen();
        break;
        case PAD_PCM_IORD_N:
            MHal_GPIO_PCM_IORD_N_Oen();
        break;
        case PAD_PCM_A11:
            MHal_GPIO_PCM_A11_Oen();
        break;
        case PAD_PCM_OE_N:
            MHal_GPIO_PCM_OE_N_Oen();
        break;
        case PAD_PCM_A10:
            MHal_GPIO_PCM_A10_Oen();
        break;
        case PAD_PCM_CE_N:
            MHal_GPIO_PCM_CE_N_Oen();
        break;
        case PAD_PCM_D7:
            MHal_GPIO_PCM_D7_Oen();
        break;
        case PAD_PCM_D6:
            MHal_GPIO_PCM_D6_Oen();
        break;
        case PAD_PCM_D5:
            MHal_GPIO_PCM_D5_Oen();
        break;
        case PAD_PCM_D4:
            MHal_GPIO_PCM_D4_Oen();
        break;
        case PAD_PCM_D3:
            MHal_GPIO_PCM_D3_Oen();
        break;
        case PAD_PCM_A3:
            MHal_GPIO_PCM_A3_Oen();
        break;
        case PAD_PCM_A2:
            MHal_GPIO_PCM_A2_Oen();
        break;
        case PAD_PCM_REG_N:
            MHal_GPIO_PCM_REG_N_Oen();
        break;
        case PAD_PCM_A1:
            MHal_GPIO_PCM_A1_Oen();
        break;
        case PAD_PCM_A0:
            MHal_GPIO_PCM_A0_Oen();
        break;
        case PAD_PCM_D0:
            MHal_GPIO_PCM_D0_Oen();
        break;
        case PAD_PCM_D1:
            MHal_GPIO_PCM_D1_Oen();
        break;
        case PAD_PCM_D2:
            MHal_GPIO_PCM_D2_Oen();
        break;
        case PAD_PCM_RESET:
            MHal_GPIO_PCM_RESET_Oen();
        break;
        case PAD_PCM_CD_N:
            MHal_GPIO_PCM_CD_N_Oen();
        break;
        case PAD_PCM2_CE_N:
            MHal_GPIO_PCM2_CE_N_Oen();
        break;
        case PAD_PCM2_IRQA_N:
            MHal_GPIO_PCM2_IRQA_N_Oen();
        break;
        case PAD_PCM2_WAIT_N:
            MHal_GPIO_PCM2_WAIT_N_Oen();
        break;
        case PAD_PCM2_RESET:
            MHal_GPIO_PCM2_RESET_Oen();
        break;
        case PAD_PCM2_CD_N:
            MHal_GPIO_PCM2_CD_N_Oen();
        break;
        case PAD_PF_AD15:
            MHal_GPIO_PF_AD15_Oen();
        break;
        case PAD_PF_CE0Z:
            MHal_GPIO_PF_CE0Z_Oen();
        break;
        case PAD_PF_CE1Z:
            MHal_GPIO_PF_CE1Z_Oen();
        break;
        case PAD_PF_OEZ:
            MHal_GPIO_PF_OEZ_Oen();
        break;
        case PAD_PF_WEZ:
            MHal_GPIO_PF_WEZ_Oen();
        break;
        case PAD_PF_ALE:
            MHal_GPIO_PF_ALE_Oen();
        break;
        case PAD_F_RBZ:
            MHal_GPIO_F_RBZ_Oen();
        break;
        case PAD_TCON0:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif
				{
					 MHal_GPIO_GPIO48_Oen();
				}
				else
				{
					 MHal_GPIO_TCON0_Oen();
				}
			}
			else	// PDP
				MHal_GPIO_TCON0_Oen();
		}
#else
			MHal_GPIO_TCON0_Oen();
#endif
        break;
        case PAD_TCON1:
            MHal_GPIO_TCON1_Oen();
        break;
        case PAD_TCON2:

 //balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					 MHal_GPIO_GPIO49_Oen();
				}
				else
				{
					 MHal_GPIO_TCON2_Oen();
				}
			}
			else	// PDP
				MHal_GPIO_TCON2_Oen();
		}
#else
			MHal_GPIO_TCON2_Oen();
#endif
        break;
        case PAD_TCON3:
            MHal_GPIO_TCON3_Oen();
        break;
        case PAD_TCON4:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					 MHal_GPIO_GPIO53_Oen();
				}
				else
				{
					 MHal_GPIO_TCON4_Oen();
				}
			}
			else	// PDP
				MHal_GPIO_TCON4_Oen();
		}
#else
			MHal_GPIO_TCON4_Oen();
#endif
        break;
        case PAD_TCON5:
            MHal_GPIO_TCON5_Oen();
        break;
        case PAD_TCON6:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					 MHal_GPIO_GPIO52_Oen();
				}
				else
				{
					 MHal_GPIO_TCON6_Oen();
				}
			}
			else	// PDP
				MHal_GPIO_TCON6_Oen();
		}
#else
			MHal_GPIO_TCON6_Oen();
#endif
        break;
        case PAD_TCON7:
            MHal_GPIO_TCON7_Oen();
        break;
        case PAD_TCON8:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					 MHal_GPIO_GPIO47_Oen();
				}
				else
				{
					 MHal_GPIO_TCON8_Oen();
				}
			}
			else	// PDP
				MHal_GPIO_TCON8_Oen();
		}
#else
			MHal_GPIO_TCON8_Oen();
#endif
        break;
        case PAD_TCON9:
            MHal_GPIO_TCON9_Oen();
        break;
        case PAD_TCON10:
            MHal_GPIO_TCON10_Oen();
        break;
        case PAD_TCON11:
            MHal_GPIO_TCON11_Oen();
        break;
        case PAD_TCON12:
            MHal_GPIO_TCON12_Oen();
        break;
        case PAD_TCON13:
            MHal_GPIO_TCON13_Oen();
        break;
        case PAD_TCON14:
            MHal_GPIO_TCON14_Oen();
        break;
        case PAD_TCON15:
            MHal_GPIO_TCON15_Oen();
        break;
        case PAD_TCON16:
            MHal_GPIO_TCON16_Oen();
        break;
        case PAD_TCON17:
            MHal_GPIO_TCON17_Oen();
        break;
        case PAD_TCON18:
            MHal_GPIO_TCON18_Oen();
        break;
        case PAD_TCON19:
            MHal_GPIO_TCON19_Oen();
        break;
        case PAD_TCON20:
            MHal_GPIO_TCON20_Oen();
        break;
        case PAD_TCON21:
            MHal_GPIO_TCON21_Oen();
        break;
        case PAD_ET_COL:
            MHal_GPIO_ET_COL_Oen();
        break;
        case PAD_ET_TXD1:
            MHal_GPIO_ET_TXD1_Oen();
        break;
        case PAD_ET_TXD0:
            MHal_GPIO_ET_TXD0_Oen();
        break;
        case PAD_ET_TX_EN:
            MHal_GPIO_ET_TX_EN_Oen();
        break;
        case PAD_ET_TX_CLK:
            MHal_GPIO_ET_TX_CLK_Oen();
        break;
        case PAD_ET_RXD0:
            MHal_GPIO_ET_RXD0_Oen();
        break;
        case PAD_ET_RXD1:
            MHal_GPIO_ET_RXD1_Oen();
        break;
        case PAD_ET_MDC:
            MHal_GPIO_ET_MDC_Oen();
        break;
        case PAD_ET_EDIO:
            MHal_GPIO_ET_EDIO_Oen();
        break;
        case PAD_I2S_IN_WS:
            MHal_GPIO_I2S_IN_WS_Oen();
        break;
        case PAD_I2S_IN_BCK:
            MHal_GPIO_I2S_IN_BCK_Oen();
        break;
        case PAD_I2S_IN_SD:
            MHal_GPIO_I2S_IN_SD_Oen();
        break;
        case PAD_SPDIF_IN:
            MHal_GPIO_SPDIF_IN_Oen();
        break;
        case PAD_SPDIF_OUT:
            MHal_GPIO_SPDIF_OUT_Oen();
        break;
        case PAD_I2S_OUT_MCK:
            MHal_GPIO_I2S_OUT_MCK_Oen();
        break;
        case PAD_I2S_OUT_WS:
            MHal_GPIO_I2S_OUT_WS_Oen();
        break;
        case PAD_I2S_OUT_BCK:
            MHal_GPIO_I2S_OUT_BCK_Oen();
        break;
        case PAD_I2S_OUT_SD:
            MHal_GPIO_I2S_OUT_SD_Oen();
        break;
        case PAD_I2S_OUT_SD1:
            MHal_GPIO_I2S_OUT_SD1_Oen();
        break;
        case PAD_I2S_OUT_SD2:
            MHal_GPIO_I2S_OUT_SD2_Oen();
        break;
        case PAD_I2S_OUT_SD3:
            MHal_GPIO_I2S_OUT_SD3_Oen();
        break;
        case PAD_B_ODD_0:
            MHal_GPIO_B_ODD_0_Oen();
        break;
        case PAD_B_ODD_1:
            MHal_GPIO_B_ODD_1_Oen();
        break;
        case PAD_B_ODD_2:
            MHal_GPIO_B_ODD_2_Oen();
        break;
        case PAD_B_ODD_3:
            MHal_GPIO_B_ODD_3_Oen();
        break;
        case PAD_B_ODD_4:
            MHal_GPIO_B_ODD_4_Oen();
        break;
        case PAD_B_ODD_5:
            MHal_GPIO_B_ODD_5_Oen();
        break;
        case PAD_B_ODD_6:
            MHal_GPIO_B_ODD_6_Oen();
        break;
        case PAD_B_ODD_7:
            MHal_GPIO_B_ODD_7_Oen();
        break;
        case PAD_G_ODD_0:
            MHal_GPIO_G_ODD_0_Oen();
        break;
        case PAD_G_ODD_1:
            MHal_GPIO_G_ODD_1_Oen();
        break;
        case PAD_G_ODD_2:
            MHal_GPIO_G_ODD_2_Oen();
        break;
        case PAD_G_ODD_3:
            MHal_GPIO_G_ODD_3_Oen();
        break;
        case PAD_G_ODD_4:
            MHal_GPIO_G_ODD_4_Oen();
        break;
        case PAD_G_ODD_5:
            MHal_GPIO_G_ODD_5_Oen();
        break;
        case PAD_G_ODD_6:
            MHal_GPIO_G_ODD_6_Oen();
        break;
        case PAD_G_ODD_7:
            MHal_GPIO_G_ODD_7_Oen();
        break;
        case PAD_R_ODD_0:
            MHal_GPIO_R_ODD_0_Oen();
        break;
        case PAD_R_ODD_1:
            MHal_GPIO_R_ODD_1_Oen();
        break;
        case PAD_R_ODD_2:
            MHal_GPIO_R_ODD_2_Oen();
        break;
        case PAD_R_ODD_3:
            MHal_GPIO_R_ODD_3_Oen();
        break;
        case PAD_R_ODD_4:
            MHal_GPIO_R_ODD_4_Oen();
        break;
        case PAD_R_ODD_5:
            MHal_GPIO_R_ODD_5_Oen();
        break;
        case PAD_R_ODD_6:
            MHal_GPIO_R_ODD_6_Oen();
        break;
        case PAD_R_ODD_7:
            MHal_GPIO_R_ODD_7_Oen();
        break;
        case PAD_mini_LVDS_0:
            MHal_GPIO_mini_LVDS_0_Oen();
        break;
        case PAD_mini_LVDS_1:
            MHal_GPIO_mini_LVDS_1_Oen();
        break;
        case PAD_mini_LVDS_2:
            MHal_GPIO_mini_LVDS_2_Oen();
        break;
        case PAD_mini_LVDS_3:
            MHal_GPIO_mini_LVDS_3_Oen();
        break;
        case PAD_mini_LVDS_4:
            MHal_GPIO_mini_LVDS_4_Oen();
        break;
        case PAD_mini_LVDS_5:
            MHal_GPIO_mini_LVDS_5_Oen();
        break;
        case PAD_mini_LVDS_6:
            MHal_GPIO_mini_LVDS_6_Oen();
        break;
        case PAD_mini_LVDS_7:
            MHal_GPIO_mini_LVDS_7_Oen();
        break;
        case PAD_LCK:
            MHal_GPIO_LCK_Oen();
        break;
        case PAD_LDE:
            MHal_GPIO_LDE_Oen();
        break;
        case PAD_LHSYNC:
            MHal_GPIO_LHSYNC_Oen();
        break;
        case PAD_LVSYNC:
            MHal_GPIO_LVSYNC_Oen();
        break;
        case PAD_PCM_WE_N:
            MHal_GPIO_PCM_WE_N_Oen();
        break;



        default:
            break;
    }
}
EXPORT_SYMBOL(MDrv_GPIO_Pad_Oen);

//-------------------------------------------------------------------------------------------------
/// disable output for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Odn(U8 u8IndexGPIO)
{
//    U8  u8RegTemp;

    switch(u8IndexGPIO)
    {
        case PAD_PM_SPI_CZ:
            MHal_GPIO_PM_SPI_CZ_Odn();
        break;
        case PAD_PM_SPI_CK:
            MHal_GPIO_PM_SPI_CK_Odn();
        break;
        case PAD_PM_SPI_DI:
            MHal_GPIO_PM_SPI_DI_Odn();
        break;
        case PAD_PM_SPI_DO:
            MHal_GPIO_PM_SPI_DO_Odn();
        break;
        case PAD_IRIN:
            MHal_GPIO_IRIN_Odn();
        break;
        case PAD_CEC:
            MHal_GPIO_CEC_Odn();
        break;
        case PAD_GPIO_PM0:
            MHal_GPIO_GPIO_PM0_Odn();
        break;
        case PAD_GPIO_PM1:
            MHal_GPIO_GPIO_PM1_Odn();
        break;
        case PAD_GPIO_PM2:
            MHal_GPIO_GPIO_PM2_Odn();
        break;
        case PAD_GPIO_PM3:
            MHal_GPIO_GPIO_PM3_Odn();
        break;
        case PAD_GPIO_PM4:	// Model Option 6
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			
			if((bootstrap & 0x08) == 0x08)	// LCD
			{
				// in LCD Model
				//	added for BOOTSTRAP(OLED/LCD) by balupzillot@lge.com
				//	to make "Pad_Odn" not work
			}
			else	// PDP
				MHal_GPIO_GPIO_PM4_Odn();	
		}
#else
		 MHal_GPIO_GPIO_PM4_Odn();
#endif
        break;
        case PAD_GPIO_PM5:
#ifdef _INCLUDE_S7BOOTSTRAP
//	added for BOOTSTRAP(Mini LVDS/LVDS) by balupzillot@lge.com
//	to make "Pad_Odn" not work
#else
            MHal_GPIO_GPIO_PM5_Odn();
#endif
        break;
        case PAD_GPIO_PM6:
            MHal_GPIO_GPIO_PM6_Odn();
        break;
        case PAD_GPIO_PM7:
            MHal_GPIO_GPIO_PM7_Odn();
        break;
        case PAD_GPIO_PM8:
#ifdef _INCLUDE_S7BOOTSTRAP
//	added for BOOTSTRAP(DDR 512/ DDR 256) by balupzillot@lge.com
//	to make "Pad_Odn" not work
#else
            MHal_GPIO_GPIO_PM8_Odn();
#endif
        break;
        case PAD_GPIO_PM9:
            MHal_GPIO_GPIO_PM9_Odn();
        break;
        case PAD_GPIO_PM10:
            MHal_GPIO_GPIO_PM10_Odn();
        break;
        case PAD_GPIO_PM11:
            MHal_GPIO_GPIO_PM11_Odn();
        break;
        case PAD_GPIO_PM12:
            MHal_GPIO_GPIO_PM12_Odn();
        break;
        case PAD_HOTPLUGA:
            MHal_GPIO_HOTPLUGA_Odn();
        break;
        case PAD_HOTPLUGB:
            MHal_GPIO_HOTPLUGB_Odn();
        break;
        case PAD_HOTPLUGC:
            MHal_GPIO_HOTPLUGC_Odn();
        break;
        case PAD_HOTPLUGD:
            MHal_GPIO_HOTPLUGD_Odn();
        break;
        case PAD_DDCDA_CK:
            MHal_GPIO_DDCDA_CK_Odn();
        break;
        case PAD_DDCDA_DA:
            MHal_GPIO_DDCDA_DA_Odn();
        break;
        case PAD_DDCDB_CK:
            MHal_GPIO_DDCDB_CK_Odn();
        break;
        case PAD_DDCDB_DA:
            MHal_GPIO_DDCDB_DA_Odn();
        break;
        case PAD_DDCDC_CK:
            MHal_GPIO_DDCDC_CK_Odn();
        break;
        case PAD_DDCDC_DA:
            MHal_GPIO_DDCDC_DA_Odn();
        break;
        case PAD_DDCDD_CK:
            MHal_GPIO_DDCDD_CK_Odn();
        break;
        case PAD_DDCDD_DA:
            MHal_GPIO_DDCDD_DA_Odn();
        break;
        case PAD_SAR0:
            MHal_GPIO_SAR0_Odn();
        break;
        case PAD_SAR1:
#ifdef _INCLUDE_S7BOOTSTRAP
//	added for BOOTSTRAP(FHD/HD) by balupzillot@lge.com
//	to make "Pad_Odn" not work
#else
            MHal_GPIO_SAR1_Odn();
#endif
        break;
        case PAD_SAR2:
            MHal_GPIO_SAR2_Odn();
        break;
        case PAD_SAR3:
            MHal_GPIO_SAR3_Odn();
        break;
        case PAD_SAR4:
            MHal_GPIO_SAR4_Odn();
        break;
        case PAD_GPIO0:
            MHal_GPIO_GPIO0_Odn();
        break;
        case PAD_GPIO1:
            MHal_GPIO_GPIO1_Odn();
        break;
        case PAD_GPIO2:
            MHal_GPIO_GPIO2_Odn();
        break;
        case PAD_GPIO3:
            MHal_GPIO_GPIO3_Odn();
        break;
        case PAD_GPIO4:
            MHal_GPIO_GPIO4_Odn();
        break;
        case PAD_GPIO5:
            MHal_GPIO_GPIO5_Odn();
        break;
        case PAD_GPIO6:
#ifdef _INCLUDE_S7BOOTSTRAP
//	added for BOOTSTRAP(FRC/not FRC) by balupzillot@lge.com
//	to make "Pad_Odn" not work
#else
            MHal_GPIO_GPIO6_Odn();
#endif
        break;
        case PAD_GPIO7:
            MHal_GPIO_GPIO7_Odn();
        break;
        case PAD_GPIO8:
            MHal_GPIO_GPIO8_Odn();
        break;
        case PAD_GPIO9:
            MHal_GPIO_GPIO9_Odn();
        break;
        case PAD_GPIO10:
            MHal_GPIO_GPIO10_Odn();
        break;
        case PAD_GPIO11:
            MHal_GPIO_GPIO11_Odn();
        break;
        case PAD_GPIO12:
            MHal_GPIO_GPIO12_Odn();
        break;
        case PAD_GPIO13:
            MHal_GPIO_GPIO13_Odn();
        break;
        case PAD_GPIO14:
            MHal_GPIO_GPIO14_Odn();
        break;
        case PAD_GPIO15:
            MHal_GPIO_GPIO15_Odn();
        break;
        case PAD_GPIO16:
            MHal_GPIO_GPIO16_Odn();
        break;
        case PAD_GPIO17:
            MHal_GPIO_GPIO17_Odn();
        break;
        case PAD_GPIO18:
            MHal_GPIO_GPIO18_Odn();
        break;
        case PAD_GPIO19:
            MHal_GPIO_GPIO19_Odn();
        break;
        case PAD_GPIO20:
            MHal_GPIO_GPIO20_Odn();
        break;
        case PAD_GPIO21:
            MHal_GPIO_GPIO21_Odn();
        break;
        case PAD_GPIO22:
            MHal_GPIO_GPIO22_Odn();
        break;
        case PAD_GPIO23:
            MHal_GPIO_GPIO23_Odn();
        break;
        case PAD_GPIO24:
            MHal_GPIO_GPIO24_Odn();
        break;
        case PAD_GPIO25:
            MHal_GPIO_GPIO25_Odn();
        break;
        case PAD_GPIO26:
            MHal_GPIO_GPIO26_Odn();
        break;
        case PAD_GPIO27:
            MHal_GPIO_GPIO27_Odn();
        break;
        case PAD_UART_RX2:
            MHal_GPIO_UART_RX2_Odn();
        break;
        case PAD_UART_TX2:
            MHal_GPIO_UART_TX2_Odn();
        break;
        case PAD_PWM0:
            MHal_GPIO_PWM0_Odn();
        break;
        case PAD_PWM1:
            MHal_GPIO_PWM1_Odn();
        break;
        case PAD_PWM2:
            MHal_GPIO_PWM2_Odn();
        break;
        case PAD_PWM3:
            MHal_GPIO_PWM3_Odn();
        break;
        case PAD_PWM4:
            MHal_GPIO_PWM4_Odn();
        break;
        case PAD_DDCR_DA:
            MHal_GPIO_DDCR_DA_Odn();
        break;
        case PAD_DDCR_CK:
            MHal_GPIO_DDCR_CK_Odn();
        break;
        case PAD_TGPIO0:
            MHal_GPIO_TGPIO0_Odn();
        break;
        case PAD_TGPIO1:
            MHal_GPIO_TGPIO1_Odn();
        break;
        case PAD_TGPIO2:
            MHal_GPIO_TGPIO2_Odn();
        break;
        case PAD_TGPIO3:
            MHal_GPIO_TGPIO3_Odn();
        break;
        case PAD_TS0_D0:
            MHal_GPIO_TS0_D0_Odn();
        break;
        case PAD_TS0_D1:
            MHal_GPIO_TS0_D1_Odn();
        break;
        case PAD_TS0_D2:
            MHal_GPIO_TS0_D2_Odn();
        break;
        case PAD_TS0_D3:
            MHal_GPIO_TS0_D3_Odn();
        break;
        case PAD_TS0_D4:
            MHal_GPIO_TS0_D4_Odn();
        break;

        case PAD_TS0_D5:
            MHal_GPIO_TS0_D5_Odn();
        break;
        case PAD_TS0_D6:
            MHal_GPIO_TS0_D6_Odn();
        break;
        case PAD_TS0_D7:
            MHal_GPIO_TS0_D7_Odn();
        break;
        case PAD_TS0_VLD:
            MHal_GPIO_TS0_VLD_Odn();
        break;
        case PAD_TS0_SYNC:
            MHal_GPIO_TS0_SYNC_Odn();
        break;
        case PAD_TS0_CLK:
            MHal_GPIO_TS0_CLK_Odn();
        break;
        case PAD_TS1_D0:
            MHal_GPIO_TS1_D0_Odn();
        break;
        case PAD_TS1_D1:
            MHal_GPIO_TS1_D1_Odn();
        break;
        case PAD_TS1_D2:
            MHal_GPIO_TS1_D2_Odn();
        break;
        case PAD_TS1_D3:
            MHal_GPIO_TS1_D3_Odn();
        break;
        case PAD_TS1_D4:
            MHal_GPIO_TS1_D4_Odn();
        break;
        case PAD_TS1_D5:
            MHal_GPIO_TS1_D5_Odn();
        break;
        case PAD_TS1_D6:
            MHal_GPIO_TS1_D6_Odn();
        break;
        case PAD_TS1_D7:
            MHal_GPIO_TS1_D7_Odn();
        break;
        case PAD_TS1_VLD:
            MHal_GPIO_TS1_VLD_Odn();
        break;
        case PAD_TS1_SYNC:
            MHal_GPIO_TS1_SYNC_Odn();
        break;
        case PAD_TS1_CLK:
            MHal_GPIO_TS1_CLK_Odn();
        break;
        case PAD_PCM_A4:
            MHal_GPIO_PCM_A4_Odn();
        break;
        case PAD_PCM_WAIT_N:
            MHal_GPIO_PCM_WAIT_N_Odn();
        break;
        case PAD_PCM_A5:
            MHal_GPIO_PCM_A5_Odn();
        break;
        case PAD_PCM_A6:
            MHal_GPIO_PCM_A6_Odn();
        break;
        case PAD_PCM_A7:
            MHal_GPIO_PCM_A7_Odn();
        break;
        case PAD_PCM_A12:
            MHal_GPIO_PCM_A12_Odn();
        break;
        case PAD_PCM_IRQA_N:
            MHal_GPIO_PCM_IRQA_N_Odn();
        break;
        case PAD_PCM_A14:
            MHal_GPIO_PCM_A14_Odn();
        break;
        case PAD_PCM_A13:
            MHal_GPIO_PCM_A13_Odn();
        break;
        case PAD_PCM_A8:
            MHal_GPIO_PCM_A8_Odn();
        break;
        case PAD_PCM_IOWR_N:
            MHal_GPIO_PCM_IOWR_N_Odn();
        break;
        case PAD_PCM_A9:
            MHal_GPIO_PCM_A9_Odn();
        break;
        case PAD_PCM_IORD_N:
            MHal_GPIO_PCM_IORD_N_Odn();
        break;
        case PAD_PCM_A11:
            MHal_GPIO_PCM_A11_Odn();
        break;
        case PAD_PCM_OE_N:
            MHal_GPIO_PCM_OE_N_Odn();
        break;
        case PAD_PCM_A10:
            MHal_GPIO_PCM_A10_Odn();
        break;
        case PAD_PCM_CE_N:
            MHal_GPIO_PCM_CE_N_Odn();
        break;
        case PAD_PCM_D7:
            MHal_GPIO_PCM_D7_Odn();
        break;
        case PAD_PCM_D6:
            MHal_GPIO_PCM_D6_Odn();
        break;
        case PAD_PCM_D5:
            MHal_GPIO_PCM_D5_Odn();
        break;
        case PAD_PCM_D4:
            MHal_GPIO_PCM_D4_Odn();
        break;
        case PAD_PCM_D3:
            MHal_GPIO_PCM_D3_Odn();
        break;
        case PAD_PCM_A3:
            MHal_GPIO_PCM_A3_Odn();
        break;
        case PAD_PCM_A2:
            MHal_GPIO_PCM_A2_Odn();
        break;
        case PAD_PCM_REG_N:
            MHal_GPIO_PCM_REG_N_Odn();
        break;
        case PAD_PCM_A1:
            MHal_GPIO_PCM_A1_Odn();
        break;
        case PAD_PCM_A0:
            MHal_GPIO_PCM_A0_Odn();
        break;
        case PAD_PCM_D0:
            MHal_GPIO_PCM_D0_Odn();
        break;
        case PAD_PCM_D1:
            MHal_GPIO_PCM_D1_Odn();
        break;
        case PAD_PCM_D2:
            MHal_GPIO_PCM_D2_Odn();
        break;
        case PAD_PCM_RESET:
            MHal_GPIO_PCM_RESET_Odn();
        break;
        case PAD_PCM_CD_N:
            MHal_GPIO_PCM_CD_N_Odn();
        break;
        case PAD_PCM2_CE_N:
            MHal_GPIO_PCM2_CE_N_Odn();
        break;
        case PAD_PCM2_IRQA_N:
            MHal_GPIO_PCM2_IRQA_N_Odn();
        break;
        case PAD_PCM2_WAIT_N:
            MHal_GPIO_PCM2_WAIT_N_Odn();
        break;
        case PAD_PCM2_RESET:
            MHal_GPIO_PCM2_RESET_Odn();
        break;
        case PAD_PCM2_CD_N:
            MHal_GPIO_PCM2_CD_N_Odn();
        break;
        case PAD_PF_AD15:
            MHal_GPIO_PF_AD15_Odn();
        break;
        case PAD_PF_CE0Z:
            MHal_GPIO_PF_CE0Z_Odn();
        break;
        case PAD_PF_CE1Z:
            MHal_GPIO_PF_CE1Z_Odn();
        break;
        case PAD_PF_OEZ:
            MHal_GPIO_PF_OEZ_Odn();
        break;
        case PAD_PF_WEZ:
            MHal_GPIO_PF_WEZ_Odn();
        break;
        case PAD_PF_ALE:
            MHal_GPIO_PF_ALE_Odn();
        break;
        case PAD_F_RBZ:
            MHal_GPIO_F_RBZ_Odn();
        break;
        case PAD_TCON0:
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO48_Odn();
				}
				else
				{
					MHal_GPIO_TCON0_Odn();
				}
			}
			else	// PDP
				MHal_GPIO_TCON0_Odn();
		}
#else
			MHal_GPIO_TCON0_Odn();
#endif
        break;
        case PAD_TCON1:
            MHal_GPIO_TCON1_Odn();
        break;
        case PAD_TCON2:
 			//balup_090907
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO49_Odn();
				}
				else
				{
					MHal_GPIO_TCON2_Odn();
				}
			}
			else	// PDP
				MHal_GPIO_TCON2_Odn();
		}
#else
			MHal_GPIO_TCON2_Odn();
#endif


        break;
        case PAD_TCON3:
            MHal_GPIO_TCON3_Odn();
        break;
        case PAD_TCON4:
 			//balup_090907
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO53_Odn();
				}
				else
				{
					  MHal_GPIO_TCON4_Odn();
				}
			}
			else	// PDP
				MHal_GPIO_TCON4_Odn();
		}
#else
			MHal_GPIO_TCON4_Odn();
#endif

        break;
        case PAD_TCON5:
            MHal_GPIO_TCON5_Odn();
        break;
        case PAD_TCON6:
 			//balup_090907
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO52_Odn();
				}
				else
				{
					MHal_GPIO_TCON6_Odn();
				}
			}
			else	// PDP
				MHal_GPIO_TCON6_Odn();
		}
#else
			MHal_GPIO_TCON6_Odn();
#endif

        break;
        case PAD_TCON7:
            MHal_GPIO_TCON7_Odn();
        break;
        case PAD_TCON8:
 			//balup_090907
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO47_Odn();
				}
				else
				{
					MHal_GPIO_TCON8_Odn();
				}
			}
			else	// PDP
				MHal_GPIO_TCON8_Odn();
		}
#else
			MHal_GPIO_TCON8_Odn();
#endif

        break;
        case PAD_TCON9:
            MHal_GPIO_TCON9_Odn();
        break;
        case PAD_TCON10:
            MHal_GPIO_TCON10_Odn();
        break;
        case PAD_TCON11:
            MHal_GPIO_TCON11_Odn();
        break;
        case PAD_TCON12:
            MHal_GPIO_TCON12_Odn();
        break;
        case PAD_TCON13:
            MHal_GPIO_TCON13_Odn();
        break;
        case PAD_TCON14:
            MHal_GPIO_TCON14_Odn();
        break;
        case PAD_TCON15:
            MHal_GPIO_TCON15_Odn();
        break;
        case PAD_TCON16:
            MHal_GPIO_TCON16_Odn();
        break;
        case PAD_TCON17:
            MHal_GPIO_TCON17_Odn();
        break;
        case PAD_TCON18:
            MHal_GPIO_TCON18_Odn();
        break;
        case PAD_TCON19:
            MHal_GPIO_TCON19_Odn();
        break;
        case PAD_TCON20:
            MHal_GPIO_TCON20_Odn();
        break;
        case PAD_TCON21:
            MHal_GPIO_TCON21_Odn();
        break;
        case PAD_ET_COL:
            MHal_GPIO_ET_COL_Odn();
        break;
        case PAD_ET_TXD1:
            MHal_GPIO_ET_TXD1_Odn();
        break;
        case PAD_ET_TXD0:
            MHal_GPIO_ET_TXD0_Odn();
        break;
        case PAD_ET_TX_EN:
            MHal_GPIO_ET_TX_EN_Odn();
        break;
        case PAD_ET_TX_CLK:
            MHal_GPIO_ET_TX_CLK_Odn();
        break;
        case PAD_ET_RXD0:
            MHal_GPIO_ET_RXD0_Odn();
        break;
        case PAD_ET_RXD1:
            MHal_GPIO_ET_RXD1_Odn();
        break;
        case PAD_ET_MDC:
            MHal_GPIO_ET_MDC_Odn();
        break;
        case PAD_ET_EDIO:
            MHal_GPIO_ET_EDIO_Odn();
        break;
        case PAD_I2S_IN_WS:
            MHal_GPIO_I2S_IN_WS_Odn();
        break;
        case PAD_I2S_IN_BCK:
            MHal_GPIO_I2S_IN_BCK_Odn();
        break;
        case PAD_I2S_IN_SD:
            MHal_GPIO_I2S_IN_SD_Odn();
        break;
        case PAD_SPDIF_IN:
            MHal_GPIO_SPDIF_IN_Odn();
        break;
        case PAD_SPDIF_OUT:
            MHal_GPIO_SPDIF_OUT_Odn();
        break;
        case PAD_I2S_OUT_MCK:
            MHal_GPIO_I2S_OUT_MCK_Odn();
        break;
        case PAD_I2S_OUT_WS:
            MHal_GPIO_I2S_OUT_WS_Odn();
        break;
        case PAD_I2S_OUT_BCK:
            MHal_GPIO_I2S_OUT_BCK_Odn();
        break;
        case PAD_I2S_OUT_SD:
            MHal_GPIO_I2S_OUT_SD_Odn();
        break;
        case PAD_I2S_OUT_SD1:
            MHal_GPIO_I2S_OUT_SD1_Odn();
        break;
        case PAD_I2S_OUT_SD2:	// Model Option 4
#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			
			if((bootstrap & 0x08) == 0x08)	// LCD
			{
				//	in LCD Model
				//	added for BOOTSTRAP(FRC_OPT1) by balupzillot@lge.com
				//	to make "Pad_Odn" not work
			}
			else	// PDP
				MHal_GPIO_I2S_OUT_SD2_Odn();
		}
#else
            MHal_GPIO_I2S_OUT_SD2_Odn();
#endif
        break;
        case PAD_I2S_OUT_SD3:	// Model Option 5
#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			
			if((bootstrap & 0x08) == 0x08)	// LCD
			{
				//	in LCD Model
				//	added for BOOTSTRAP(FRC_OPT1) by balupzillot@lge.com
				//	to make "Pad_Odn" not work
			}
			else	// PDP
				MHal_GPIO_I2S_OUT_SD3_Odn();
		}
#else
            MHal_GPIO_I2S_OUT_SD3_Odn();
#endif
        break;
        case PAD_B_ODD_0:
            MHal_GPIO_B_ODD_0_Odn();
        break;
        case PAD_B_ODD_1:
            MHal_GPIO_B_ODD_1_Odn();
        break;
        case PAD_B_ODD_2:
            MHal_GPIO_B_ODD_2_Odn();
        break;
        case PAD_B_ODD_3:
            MHal_GPIO_B_ODD_3_Odn();
        break;
        case PAD_B_ODD_4:
            MHal_GPIO_B_ODD_4_Odn();
        break;
        case PAD_B_ODD_5:
            MHal_GPIO_B_ODD_5_Odn();
        break;
        case PAD_B_ODD_6:
            MHal_GPIO_B_ODD_6_Odn();
        break;
        case PAD_B_ODD_7:
            MHal_GPIO_B_ODD_7_Odn();
        break;
        case PAD_G_ODD_0:
            MHal_GPIO_G_ODD_0_Odn();
        break;
        case PAD_G_ODD_1:
            MHal_GPIO_G_ODD_1_Odn();
        break;
        case PAD_G_ODD_2:
            MHal_GPIO_G_ODD_2_Odn();
        break;
        case PAD_G_ODD_3:
            MHal_GPIO_G_ODD_3_Odn();
        break;
        case PAD_G_ODD_4:
            MHal_GPIO_G_ODD_4_Odn();
        break;
        case PAD_G_ODD_5:
            MHal_GPIO_G_ODD_5_Odn();
        break;
        case PAD_G_ODD_6:
            MHal_GPIO_G_ODD_6_Odn();
        break;
        case PAD_G_ODD_7:
            MHal_GPIO_G_ODD_7_Odn();
        break;
        case PAD_R_ODD_0:
            MHal_GPIO_R_ODD_0_Odn();
        break;
        case PAD_R_ODD_1:
            MHal_GPIO_R_ODD_1_Odn();
        break;
        case PAD_R_ODD_2:
            MHal_GPIO_R_ODD_2_Odn();
        break;
        case PAD_R_ODD_3:
            MHal_GPIO_R_ODD_3_Odn();
        break;
        case PAD_R_ODD_4:
            MHal_GPIO_R_ODD_4_Odn();
        break;
        case PAD_R_ODD_5:
            MHal_GPIO_R_ODD_5_Odn();
        break;
        case PAD_R_ODD_6:
            MHal_GPIO_R_ODD_6_Odn();
        break;
        case PAD_R_ODD_7:
            MHal_GPIO_R_ODD_7_Odn();
        break;
        case PAD_mini_LVDS_0:
            MHal_GPIO_mini_LVDS_0_Odn();
        break;
        case PAD_mini_LVDS_1:
            MHal_GPIO_mini_LVDS_1_Odn();
        break;
        case PAD_mini_LVDS_2:
            MHal_GPIO_mini_LVDS_2_Odn();
        break;
        case PAD_mini_LVDS_3:
            MHal_GPIO_mini_LVDS_3_Odn();
        break;
        case PAD_mini_LVDS_4:
            MHal_GPIO_mini_LVDS_4_Odn();
        break;
        case PAD_mini_LVDS_5:
            MHal_GPIO_mini_LVDS_5_Odn();
        break;
        case PAD_mini_LVDS_6:
            MHal_GPIO_mini_LVDS_6_Odn();
        break;
        case PAD_mini_LVDS_7:
            MHal_GPIO_mini_LVDS_7_Odn();
        break;
        case PAD_LCK:
            MHal_GPIO_LCK_Odn();
        break;
        case PAD_LDE:
            MHal_GPIO_LDE_Odn();
        break;
        case PAD_LHSYNC:
            MHal_GPIO_LHSYNC_Odn();
        break;
        case PAD_LVSYNC:
            MHal_GPIO_LVSYNC_Odn();
        break;
        case PAD_PCM_WE_N:
            MHal_GPIO_PCM_WE_N_Odn();
        break;



        default:
            break;
    }
}
EXPORT_SYMBOL(MDrv_GPIO_Pad_Odn);

//-------------------------------------------------------------------------------------------------
/// read data from selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
U8 MDrv_GPIO_Pad_Read(U8 u8IndexGPIO)
{
//    U8  u8RegTemp;

    switch(u8IndexGPIO)
    {
        case PAD_PM_SPI_CZ:
            return MHal_GPIO_PM_SPI_CZ_In();
        break;
        case PAD_PM_SPI_CK:
            return MHal_GPIO_PM_SPI_CK_In();
        break;
        case PAD_PM_SPI_DI:
            return MHal_GPIO_PM_SPI_DI_In();
        break;
        case PAD_PM_SPI_DO:
            return MHal_GPIO_PM_SPI_DO_In();
        break;
        case PAD_IRIN:
            return MHal_GPIO_IRIN_In();
        break;
        case PAD_CEC:
            return MHal_GPIO_CEC_In();
        break;
        case PAD_GPIO_PM0:
            return MHal_GPIO_GPIO_PM0_In();
        break;
        case PAD_GPIO_PM1:
            return MHal_GPIO_GPIO_PM1_In();
        break;
        case PAD_GPIO_PM2:
            return MHal_GPIO_GPIO_PM2_In();
        break;
        case PAD_GPIO_PM3:
            return MHal_GPIO_GPIO_PM3_In();
        break;
        case PAD_GPIO_PM4:	// Model Option 6
#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
				return ((bootstrap & 0x80) ? 0x1 : 0x0);
			else	// PDP
				return MHal_GPIO_GPIO_PM4_In();
		}
#else
            return MHal_GPIO_GPIO_PM4_In();
#endif
        break;
        case PAD_GPIO_PM5:	// Model Option 1

#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			return ((bootstrap & 0x20) ? 0x1 : 0x0);
		}

#else
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			return ((bootstrap & 0x04) ? 0x1 : 0x0);
		}
#else
            return MHal_GPIO_GPIO_PM5_In();
#endif
#endif
        break;
        case PAD_GPIO_PM6:
            return MHal_GPIO_GPIO_PM6_In();
        break;
        case PAD_GPIO_PM7:
            return MHal_GPIO_GPIO_PM7_In();
        break;
        case PAD_GPIO_PM8:	// Model Option 2
#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			return ((bootstrap & 0x10) ? 0x1 : 0x0);
		}

#else
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			return ((bootstrap & 0x02) ? 0x1 : 0x0);
		}
#else

            return MHal_GPIO_GPIO_PM8_In();
#endif
#endif
        break;
        case PAD_GPIO_PM9:
            return MHal_GPIO_GPIO_PM9_In();
        break;
        case PAD_GPIO_PM10:
            return MHal_GPIO_GPIO_PM10_In();
        break;
        case PAD_GPIO_PM11:
            return MHal_GPIO_GPIO_PM11_In();
        break;
        case PAD_GPIO_PM12:
            return MHal_GPIO_GPIO_PM12_In();
        break;
        case PAD_HOTPLUGA:
            return MHal_GPIO_HOTPLUGA_In();
        break;
        case PAD_HOTPLUGB:
            return MHal_GPIO_HOTPLUGB_In();
        break;
        case PAD_HOTPLUGC:
            return MHal_GPIO_HOTPLUGC_In();
        break;
        case PAD_HOTPLUGD:
            return MHal_GPIO_HOTPLUGD_In();
        break;
        case PAD_DDCDA_CK:
            return MHal_GPIO_DDCDA_CK_In();
        break;
        case PAD_DDCDA_DA:
            return MHal_GPIO_DDCDA_DA_In();
        break;
        case PAD_DDCDB_CK:
            return MHal_GPIO_DDCDB_CK_In();
        break;
        case PAD_DDCDB_DA:
            return MHal_GPIO_DDCDB_DA_In();
        break;
        case PAD_DDCDC_CK:
            return MHal_GPIO_DDCDC_CK_In();
        break;
        case PAD_DDCDC_DA:
            return MHal_GPIO_DDCDC_DA_In();
        break;
        case PAD_DDCDD_CK:
            return MHal_GPIO_DDCDD_CK_In();
        break;
        case PAD_DDCDD_DA:
            return MHal_GPIO_DDCDD_DA_In();
        break;
        case PAD_SAR0:
            return MHal_GPIO_SAR0_In();
        break;
        case PAD_SAR1:	// Model Option 3
#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			return ((bootstrap & 0x04) ? 0x1 : 0x0);
		}

#else
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			return ((bootstrap & 0x01) ? 0x1 : 0x0);
		}
#else
            return MHal_GPIO_SAR1_In();
#endif
#endif
        break;
        case PAD_SAR2:
            return MHal_GPIO_SAR2_In();
        break;
        case PAD_SAR3:
            return MHal_GPIO_SAR3_In();
        break;
        case PAD_SAR4:
            return MHal_GPIO_SAR4_In();
        break;
        case PAD_GPIO0:
            return MHal_GPIO_GPIO0_In();
        break;
        case PAD_GPIO1:
            return MHal_GPIO_GPIO1_In();
        break;
        case PAD_GPIO2:
            return MHal_GPIO_GPIO2_In();
        break;
        case PAD_GPIO3:
            return MHal_GPIO_GPIO3_In();
        break;
        case PAD_GPIO4:
            return MHal_GPIO_GPIO4_In();
        break;
        case PAD_GPIO5:
            return MHal_GPIO_GPIO5_In();
        break;
        case PAD_GPIO6:	// Model Option 0
#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			return ((bootstrap & 0x40) ? 0x1 : 0x0);
		}

#else
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			return ((bootstrap & 0x10) ? 0x1 : 0x0);
		}
#else
            return MHal_GPIO_GPIO6_In();
#endif
#endif
        break;
        case PAD_GPIO7:
            return MHal_GPIO_GPIO7_In();
        break;
        case PAD_GPIO8:
            return MHal_GPIO_GPIO8_In();
        break;
        case PAD_GPIO9:
            return MHal_GPIO_GPIO9_In();
        break;
        case PAD_GPIO10:
            return MHal_GPIO_GPIO10_In();
        break;
        case PAD_GPIO11:
            return MHal_GPIO_GPIO11_In();
        break;
        case PAD_GPIO12:
            return MHal_GPIO_GPIO12_In();
        break;
        case PAD_GPIO13:
            return MHal_GPIO_GPIO13_In();
        break;
        case PAD_GPIO14:
            return MHal_GPIO_GPIO14_In();
        break;
        case PAD_GPIO15:
            return MHal_GPIO_GPIO15_In();
        break;
        case PAD_GPIO16:
            return MHal_GPIO_GPIO16_In();
        break;
        case PAD_GPIO17:
            return MHal_GPIO_GPIO17_In();
        break;
        case PAD_GPIO18:
            return MHal_GPIO_GPIO18_In();
        break;
        case PAD_GPIO19:
            return MHal_GPIO_GPIO19_In();
        break;
        case PAD_GPIO20:
            return MHal_GPIO_GPIO20_In();
        break;
        case PAD_GPIO21:
            return MHal_GPIO_GPIO21_In();
        break;
        case PAD_GPIO22:
            return MHal_GPIO_GPIO22_In();
        break;
        case PAD_GPIO23:
            return MHal_GPIO_GPIO23_In();
        break;
        case PAD_GPIO24:
            return MHal_GPIO_GPIO24_In();
        break;
        case PAD_GPIO25:
            return MHal_GPIO_GPIO25_In();
        break;
        case PAD_GPIO26:
            return MHal_GPIO_GPIO26_In();
        break;
        case PAD_GPIO27:
            return MHal_GPIO_GPIO27_In();
        break;
        case PAD_UART_RX2:
            return MHal_GPIO_UART_RX2_In();
        break;
        case PAD_UART_TX2:
            return MHal_GPIO_UART_TX2_In();
        break;
        case PAD_PWM0:
            return MHal_GPIO_PWM0_In();
        break;
        case PAD_PWM1:
            return MHal_GPIO_PWM1_In();
        break;
        case PAD_PWM2:
            return MHal_GPIO_PWM2_In();
        break;
        case PAD_PWM3:
            return MHal_GPIO_PWM3_In();
        break;
        case PAD_PWM4:
            return MHal_GPIO_PWM4_In();
        break;
        case PAD_DDCR_DA:
            return MHal_GPIO_DDCR_DA_In();
        break;
        case PAD_DDCR_CK:
            return MHal_GPIO_DDCR_CK_In();
        break;
        case PAD_TGPIO0:
            return MHal_GPIO_TGPIO0_In();
        break;
        case PAD_TGPIO1:
            return MHal_GPIO_TGPIO1_In();
        break;
        case PAD_TGPIO2:
            return MHal_GPIO_TGPIO2_In();
        break;
        case PAD_TGPIO3:
            return MHal_GPIO_TGPIO3_In();
        break;
        case PAD_TS0_D0:
            return MHal_GPIO_TS0_D0_In();
        break;
        case PAD_TS0_D1:
            return MHal_GPIO_TS0_D1_In();
        break;
        case PAD_TS0_D2:
            return MHal_GPIO_TS0_D2_In();
        break;
        case PAD_TS0_D3:
            return MHal_GPIO_TS0_D3_In();
        break;
        case PAD_TS0_D4:
            return MHal_GPIO_TS0_D4_In();
        break;

        case PAD_TS0_D5:
            return MHal_GPIO_TS0_D5_In();
        break;
        case PAD_TS0_D6:
            return MHal_GPIO_TS0_D6_In();
        break;
        case PAD_TS0_D7:
            return MHal_GPIO_TS0_D7_In();
        break;
        case PAD_TS0_VLD:
            return MHal_GPIO_TS0_VLD_In();
        break;
        case PAD_TS0_SYNC:
            return MHal_GPIO_TS0_SYNC_In();
        break;
        case PAD_TS0_CLK:
            return MHal_GPIO_TS0_CLK_In();
        break;
        case PAD_TS1_D0:
            return MHal_GPIO_TS1_D0_In();
        break;
        case PAD_TS1_D1:
            return MHal_GPIO_TS1_D1_In();
        break;
        case PAD_TS1_D2:
            return MHal_GPIO_TS1_D2_In();
        break;
        case PAD_TS1_D3:
            return MHal_GPIO_TS1_D3_In();
        break;
        case PAD_TS1_D4:
            return MHal_GPIO_TS1_D4_In();
        break;
        case PAD_TS1_D5:
            return MHal_GPIO_TS1_D5_In();
        break;
        case PAD_TS1_D6:
            return MHal_GPIO_TS1_D6_In();
        break;
        case PAD_TS1_D7:
            return MHal_GPIO_TS1_D7_In();
        break;
        case PAD_TS1_VLD:
            return MHal_GPIO_TS1_VLD_In();
        break;
        case PAD_TS1_SYNC:
            return MHal_GPIO_TS1_SYNC_In();
        break;
        case PAD_TS1_CLK:
            return MHal_GPIO_TS1_CLK_In();
        break;
        case PAD_PCM_A4:
            return MHal_GPIO_PCM_A4_In();
        break;
        case PAD_PCM_WAIT_N:
            return MHal_GPIO_PCM_WAIT_N_In();
        break;
        case PAD_PCM_A5:
            return MHal_GPIO_PCM_A5_In();
        break;
        case PAD_PCM_A6:
            return MHal_GPIO_PCM_A6_In();
        break;
        case PAD_PCM_A7:
            return MHal_GPIO_PCM_A7_In();
        break;
        case PAD_PCM_A12:
            return MHal_GPIO_PCM_A12_In();
        break;
        case PAD_PCM_IRQA_N:
            return MHal_GPIO_PCM_IRQA_N_In();
        break;
        case PAD_PCM_A14:
            return MHal_GPIO_PCM_A14_In();
        break;
        case PAD_PCM_A13:
            return MHal_GPIO_PCM_A13_In();
        break;
        case PAD_PCM_A8:
            return MHal_GPIO_PCM_A8_In();
        break;
        case PAD_PCM_IOWR_N:
            return MHal_GPIO_PCM_IOWR_N_In();
        break;
        case PAD_PCM_A9:
            return MHal_GPIO_PCM_A9_In();
        break;
        case PAD_PCM_IORD_N:
            return MHal_GPIO_PCM_IORD_N_In();
        break;
        case PAD_PCM_A11:
            return MHal_GPIO_PCM_A11_In();
        break;
        case PAD_PCM_OE_N:
            return MHal_GPIO_PCM_OE_N_In();
        break;
        case PAD_PCM_A10:
            return MHal_GPIO_PCM_A10_In();
        break;
        case PAD_PCM_CE_N:
            return MHal_GPIO_PCM_CE_N_In();
        break;
        case PAD_PCM_D7:
            return MHal_GPIO_PCM_D7_In();
        break;
        case PAD_PCM_D6:
            return MHal_GPIO_PCM_D6_In();
        break;
        case PAD_PCM_D5:
            return MHal_GPIO_PCM_D5_In();
        break;
        case PAD_PCM_D4:
            return MHal_GPIO_PCM_D4_In();
        break;
        case PAD_PCM_D3:
            return MHal_GPIO_PCM_D3_In();
        break;
        case PAD_PCM_A3:
            return MHal_GPIO_PCM_A3_In();
        break;
        case PAD_PCM_A2:
            return MHal_GPIO_PCM_A2_In();
        break;
        case PAD_PCM_REG_N:
            return MHal_GPIO_PCM_REG_N_In();
        break;
        case PAD_PCM_A1:
            return MHal_GPIO_PCM_A1_In();
        break;
        case PAD_PCM_A0:
            return MHal_GPIO_PCM_A0_In();
        break;
        case PAD_PCM_D0:
            return MHal_GPIO_PCM_D0_In();
        break;
        case PAD_PCM_D1:
            return MHal_GPIO_PCM_D1_In();
        break;
        case PAD_PCM_D2:
            return MHal_GPIO_PCM_D2_In();
        break;
        case PAD_PCM_RESET:
            return MHal_GPIO_PCM_RESET_In();
        break;
        case PAD_PCM_CD_N:
            return MHal_GPIO_PCM_CD_N_In();
        break;
        case PAD_PCM2_CE_N:
            return MHal_GPIO_PCM2_CE_N_In();
        break;
        case PAD_PCM2_IRQA_N:
            return MHal_GPIO_PCM2_IRQA_N_In();
        break;
        case PAD_PCM2_WAIT_N:
            return MHal_GPIO_PCM2_WAIT_N_In();
        break;
        case PAD_PCM2_RESET:
            return MHal_GPIO_PCM2_RESET_In();
        break;
        case PAD_PCM2_CD_N:
            return MHal_GPIO_PCM2_CD_N_In();
        break;
        case PAD_PF_AD15:
            return MHal_GPIO_PF_AD15_In();
        break;
        case PAD_PF_CE0Z:
            return MHal_GPIO_PF_CE0Z_In();
        break;
        case PAD_PF_CE1Z:
            return MHal_GPIO_PF_CE1Z_In();
        break;
        case PAD_PF_OEZ:
            return MHal_GPIO_PF_OEZ_In();
        break;
        case PAD_PF_WEZ:
            return MHal_GPIO_PF_WEZ_In();
        break;
        case PAD_PF_ALE:
            return MHal_GPIO_PF_ALE_In();
        break;
        case PAD_F_RBZ:
            return MHal_GPIO_F_RBZ_In();
        break;
        case PAD_TCON0:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					return MHal_GPIO_GPIO48_In();
				}
				else
				{
					return MHal_GPIO_TCON0_In();
				}
			}
			else	// PDP
				return MHal_GPIO_TCON0_In();
		}
#else
			return MHal_GPIO_TCON0_In();
#endif

        break;
        case PAD_TCON1:
            return MHal_GPIO_TCON1_In();
        break;
        case PAD_TCON2:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					return MHal_GPIO_GPIO49_In();
				}
				else
				{
					return MHal_GPIO_TCON2_In();
				}
			}
			else
				return MHal_GPIO_TCON2_In();
		}
#else
			return MHal_GPIO_TCON2_In();
#endif

        break;
        case PAD_TCON3:
            return MHal_GPIO_TCON3_In();
        break;
        case PAD_TCON4:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					return MHal_GPIO_GPIO53_In();
				}
				else
				{
					return MHal_GPIO_TCON4_In();
				}
			}
			else	// PDP
				return MHal_GPIO_TCON4_In();
		}
#else
			return MHal_GPIO_TCON4_In();
#endif

        break;
        case PAD_TCON5:
            return MHal_GPIO_TCON5_In();
        break;
        case PAD_TCON6:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif
				{
					return MHal_GPIO_GPIO52_In();
				}
				else
				{
					return MHal_GPIO_TCON6_In();
				}
			}
			else	// PDP
				return MHal_GPIO_TCON6_In();
		}
#else
			return MHal_GPIO_TCON6_In();
#endif

        break;
        case PAD_TCON7:
            return MHal_GPIO_TCON7_In();
        break;

		case PAD_TCON8:

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					return MHal_GPIO_GPIO47_In();
				}
				else
				{
					return MHal_GPIO_TCON8_In();
				}
			}
			else	// PDP
				return MHal_GPIO_TCON8_In();
		}
#else
			return MHal_GPIO_TCON8_In();
#endif
        break;
        case PAD_TCON9:
            return MHal_GPIO_TCON9_In();
        break;
        case PAD_TCON10:
            return MHal_GPIO_TCON10_In();
        break;
        case PAD_TCON11:
            return MHal_GPIO_TCON11_In();
        break;
        case PAD_TCON12:
            return MHal_GPIO_TCON12_In();
        break;
        case PAD_TCON13:
            return MHal_GPIO_TCON13_In();
        break;
        case PAD_TCON14:
            return MHal_GPIO_TCON14_In();
        break;
        case PAD_TCON15:
            return MHal_GPIO_TCON15_In();
        break;
        case PAD_TCON16:
            return MHal_GPIO_TCON16_In();
        break;
        case PAD_TCON17:
            return MHal_GPIO_TCON17_In();
        break;
        case PAD_TCON18:
            return MHal_GPIO_TCON18_In();
        break;
        case PAD_TCON19:
            return MHal_GPIO_TCON19_In();
        break;
        case PAD_TCON20:
            return MHal_GPIO_TCON20_In();
        break;
        case PAD_TCON21:
            return MHal_GPIO_TCON21_In();
        break;
        case PAD_ET_COL:
            return MHal_GPIO_ET_COL_In();
        break;
        case PAD_ET_TXD1:
            return MHal_GPIO_ET_TXD1_In();
        break;
        case PAD_ET_TXD0:
            return MHal_GPIO_ET_TXD0_In();
        break;
        case PAD_ET_TX_EN:
            return MHal_GPIO_ET_TX_EN_In();
        break;
        case PAD_ET_TX_CLK:
            return MHal_GPIO_ET_TX_CLK_In();
        break;
        case PAD_ET_RXD0:
            return MHal_GPIO_ET_RXD0_In();
        break;
        case PAD_ET_RXD1:
            return MHal_GPIO_ET_RXD1_In();
        break;
        case PAD_ET_MDC:
            return MHal_GPIO_ET_MDC_In();
        break;
        case PAD_ET_EDIO:
            return MHal_GPIO_ET_EDIO_In();
        break;
        case PAD_I2S_IN_WS:
            return MHal_GPIO_I2S_IN_WS_In();
        break;
        case PAD_I2S_IN_BCK:
            return MHal_GPIO_I2S_IN_BCK_In();
        break;
        case PAD_I2S_IN_SD:
            return MHal_GPIO_I2S_IN_SD_In();
        break;
        case PAD_SPDIF_IN:
            return MHal_GPIO_SPDIF_IN_In();
        break;
        case PAD_SPDIF_OUT:
            return MHal_GPIO_SPDIF_OUT_In();
        break;
        case PAD_I2S_OUT_MCK:
            return MHal_GPIO_I2S_OUT_MCK_In();
        break;
        case PAD_I2S_OUT_WS:
            return MHal_GPIO_I2S_OUT_WS_In();
        break;
        case PAD_I2S_OUT_BCK:
            return MHal_GPIO_I2S_OUT_BCK_In();
        break;
        case PAD_I2S_OUT_SD:
            return MHal_GPIO_I2S_OUT_SD_In();
        break;
        case PAD_I2S_OUT_SD1:
            return MHal_GPIO_I2S_OUT_SD1_In();
        break;
        case PAD_I2S_OUT_SD2:	// Model Option 4
#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
				return ((bootstrap & 0x02) ? 0x1 : 0x0);
			else	// PDP
				return MHal_GPIO_I2S_OUT_SD2_In();
		}
#else
            return MHal_GPIO_I2S_OUT_SD2_In();
#endif
        break;
        case PAD_I2S_OUT_SD3:	// Model Option 5
#ifdef _INCLUDE_NEWBOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
				return ((bootstrap & 0x01) ? 0x1 : 0x0);
			else	// PDP
				return MHal_GPIO_I2S_OUT_SD3_In();
		}
#else
            return MHal_GPIO_I2S_OUT_SD3_In();
#endif
        break;
        case PAD_B_ODD_0:
            return MHal_GPIO_B_ODD_0_In();
        break;
        case PAD_B_ODD_1:
            return MHal_GPIO_B_ODD_1_In();
        break;
        case PAD_B_ODD_2:
            return MHal_GPIO_B_ODD_2_In();
        break;
        case PAD_B_ODD_3:
            return MHal_GPIO_B_ODD_3_In();
        break;
        case PAD_B_ODD_4:
            return MHal_GPIO_B_ODD_4_In();
        break;
        case PAD_B_ODD_5:
            return MHal_GPIO_B_ODD_5_In();
        break;
        case PAD_B_ODD_6:
            return MHal_GPIO_B_ODD_6_In();
        break;
        case PAD_B_ODD_7:
            return MHal_GPIO_B_ODD_7_In();
        break;
        case PAD_G_ODD_0:
            return MHal_GPIO_G_ODD_0_In();
        break;
        case PAD_G_ODD_1:
            return MHal_GPIO_G_ODD_1_In();
        break;
        case PAD_G_ODD_2:
            return MHal_GPIO_G_ODD_2_In();
        break;
        case PAD_G_ODD_3:
            return MHal_GPIO_G_ODD_3_In();
        break;
        case PAD_G_ODD_4:
            return MHal_GPIO_G_ODD_4_In();
        break;
        case PAD_G_ODD_5:
            return MHal_GPIO_G_ODD_5_In();
        break;
        case PAD_G_ODD_6:
            return MHal_GPIO_G_ODD_6_In();
        break;
        case PAD_G_ODD_7:
            return MHal_GPIO_G_ODD_7_In();
        break;
        case PAD_R_ODD_0:
            return MHal_GPIO_R_ODD_0_In();
        break;
        case PAD_R_ODD_1:
            return MHal_GPIO_R_ODD_1_In();
        break;
        case PAD_R_ODD_2:
            return MHal_GPIO_R_ODD_2_In();
        break;
        case PAD_R_ODD_3:
            return MHal_GPIO_R_ODD_3_In();
        break;
        case PAD_R_ODD_4:
            return MHal_GPIO_R_ODD_4_In();
        break;
        case PAD_R_ODD_5:
            return MHal_GPIO_R_ODD_5_In();
        break;
        case PAD_R_ODD_6:
            return MHal_GPIO_R_ODD_6_In();
        break;
        case PAD_R_ODD_7:
            return MHal_GPIO_R_ODD_7_In();
        break;
        case PAD_mini_LVDS_0:
            return MHal_GPIO_mini_LVDS_0_In();
        break;
        case PAD_mini_LVDS_1:
            return MHal_GPIO_mini_LVDS_1_In();
        break;
        case PAD_mini_LVDS_2:
            return MHal_GPIO_mini_LVDS_2_In();
        break;
        case PAD_mini_LVDS_3:
            return MHal_GPIO_mini_LVDS_3_In();
        break;
        case PAD_mini_LVDS_4:
            return MHal_GPIO_mini_LVDS_4_In();
        break;
        case PAD_mini_LVDS_5:
            return MHal_GPIO_mini_LVDS_5_In();
        break;
        case PAD_mini_LVDS_6:
            return MHal_GPIO_mini_LVDS_6_In();
        break;
        case PAD_mini_LVDS_7:
            return MHal_GPIO_mini_LVDS_7_In();
        break;
        case PAD_LCK:
            return MHal_GPIO_LCK_In();
        break;
        case PAD_LDE:
            return MHal_GPIO_LDE_In();
        break;
        case PAD_LHSYNC:
            return MHal_GPIO_LHSYNC_In();
        break;
        case PAD_LVSYNC:
            return MHal_GPIO_LVSYNC_In();
        break;
        case PAD_PCM_WE_N:
            return MHal_GPIO_PCM_WE_N_In();
        break;



        default:
            return 255;
    }
}
EXPORT_SYMBOL(MDrv_GPIO_Pad_Read);

//-------------------------------------------------------------------------------------------------
/// output pull high for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pull_High(U8 u8IndexGPIO)
{
//    U8  u8RegTemp;

    switch(u8IndexGPIO)
    {
        case PAD_PM_SPI_CZ:
            MHal_GPIO_PM_SPI_CZ_High();
        break;
        case PAD_PM_SPI_CK:
            MHal_GPIO_PM_SPI_CK_High();
        break;
        case PAD_PM_SPI_DI:
            MHal_GPIO_PM_SPI_DI_High();
        break;
        case PAD_PM_SPI_DO:
            MHal_GPIO_PM_SPI_DO_High();
        break;
        case PAD_IRIN:
            MHal_GPIO_IRIN_High();
        break;
        case PAD_CEC:
            MHal_GPIO_CEC_High();
        break;
        case PAD_GPIO_PM0:
            MHal_GPIO_GPIO_PM0_High();
        break;
        case PAD_GPIO_PM1:
            MHal_GPIO_GPIO_PM1_High();
        break;
        case PAD_GPIO_PM2:
            MHal_GPIO_GPIO_PM2_High();
        break;
        case PAD_GPIO_PM3:
            MHal_GPIO_GPIO_PM3_High();
        break;
        case PAD_GPIO_PM4:
            MHal_GPIO_GPIO_PM4_High();
        break;
        case PAD_GPIO_PM5:
            MHal_GPIO_GPIO_PM5_High();
        break;
        case PAD_GPIO_PM6:
            MHal_GPIO_GPIO_PM6_High();
        break;
        case PAD_GPIO_PM7:
            MHal_GPIO_GPIO_PM7_High();
        break;
        case PAD_GPIO_PM8:
            MHal_GPIO_GPIO_PM8_High();
        break;
        case PAD_GPIO_PM9:
            MHal_GPIO_GPIO_PM9_High();
        break;
        case PAD_GPIO_PM10:
            MHal_GPIO_GPIO_PM10_High();
        break;
        case PAD_GPIO_PM11:
            MHal_GPIO_GPIO_PM11_High();
        break;
        case PAD_GPIO_PM12:
            MHal_GPIO_GPIO_PM12_High();
        break;
        case PAD_HOTPLUGA:
            MHal_GPIO_HOTPLUGA_High();
        break;
        case PAD_HOTPLUGB:
            MHal_GPIO_HOTPLUGB_High();
        break;
        case PAD_HOTPLUGC:
            MHal_GPIO_HOTPLUGC_High();
        break;
        case PAD_HOTPLUGD:
            MHal_GPIO_HOTPLUGD_High();
        break;
        case PAD_DDCDA_CK:
            MHal_GPIO_DDCDA_CK_High();
        break;
        case PAD_DDCDA_DA:
            MHal_GPIO_DDCDA_DA_High();
        break;
        case PAD_DDCDB_CK:
            MHal_GPIO_DDCDB_CK_High();
        break;
        case PAD_DDCDB_DA:
            MHal_GPIO_DDCDB_DA_High();
        break;
        case PAD_DDCDC_CK:
            MHal_GPIO_DDCDC_CK_High();
        break;
        case PAD_DDCDC_DA:
            MHal_GPIO_DDCDC_DA_High();
        break;
        case PAD_DDCDD_CK:
            MHal_GPIO_DDCDD_CK_High();
        break;
        case PAD_DDCDD_DA:
            MHal_GPIO_DDCDD_DA_High();
        break;
        case PAD_SAR0:
            MHal_GPIO_SAR0_High();
        break;
        case PAD_SAR1:
            MHal_GPIO_SAR1_High();
        break;
        case PAD_SAR2:
            MHal_GPIO_SAR2_High();
        break;
        case PAD_SAR3:
            MHal_GPIO_SAR3_High();
        break;
        case PAD_SAR4:
            MHal_GPIO_SAR4_High();
        break;
        case PAD_GPIO0:
            MHal_GPIO_GPIO0_High();
        break;
        case PAD_GPIO1:
            MHal_GPIO_GPIO1_High();
        break;
        case PAD_GPIO2:
            MHal_GPIO_GPIO2_High();
        break;
        case PAD_GPIO3:
            MHal_GPIO_GPIO3_High();
        break;
        case PAD_GPIO4:
            MHal_GPIO_GPIO4_High();
        break;
        case PAD_GPIO5:
            MHal_GPIO_GPIO5_High();
        break;
        case PAD_GPIO6:
            MHal_GPIO_GPIO6_High();
        break;
        case PAD_GPIO7:
            MHal_GPIO_GPIO7_High();
        break;
        case PAD_GPIO8:
            MHal_GPIO_GPIO8_High();
        break;
        case PAD_GPIO9:
            MHal_GPIO_GPIO9_High();
        break;
        case PAD_GPIO10:
            MHal_GPIO_GPIO10_High();
        break;
        case PAD_GPIO11:
            MHal_GPIO_GPIO11_High();
        break;
        case PAD_GPIO12:
            MHal_GPIO_GPIO12_High();
        break;
        case PAD_GPIO13:
            MHal_GPIO_GPIO13_High();
        break;
        case PAD_GPIO14:
            MHal_GPIO_GPIO14_High();
        break;
        case PAD_GPIO15:
            MHal_GPIO_GPIO15_High();
        break;
        case PAD_GPIO16:
            MHal_GPIO_GPIO16_High();
        break;
        case PAD_GPIO17:
            MHal_GPIO_GPIO17_High();
        break;
        case PAD_GPIO18:
            MHal_GPIO_GPIO18_High();
        break;
        case PAD_GPIO19:
            MHal_GPIO_GPIO19_High();
        break;
        case PAD_GPIO20:
            MHal_GPIO_GPIO20_High();
        break;
        case PAD_GPIO21:
            MHal_GPIO_GPIO21_High();
        break;
        case PAD_GPIO22:
            MHal_GPIO_GPIO22_High();
        break;
        case PAD_GPIO23:
            MHal_GPIO_GPIO23_High();
        break;
        case PAD_GPIO24:
            MHal_GPIO_GPIO24_High();
        break;
        case PAD_GPIO25:
            MHal_GPIO_GPIO25_High();
        break;
        case PAD_GPIO26:
            MHal_GPIO_GPIO26_High();
        break;
        case PAD_GPIO27:
            MHal_GPIO_GPIO27_High();
        break;
        case PAD_UART_RX2:
            MHal_GPIO_UART_RX2_High();
        break;
        case PAD_UART_TX2:
            MHal_GPIO_UART_TX2_High();
        break;
        case PAD_PWM0:
            MHal_GPIO_PWM0_High();
        break;
        case PAD_PWM1:
            MHal_GPIO_PWM1_High();
        break;
        case PAD_PWM2:
            MHal_GPIO_PWM2_High();
        break;
        case PAD_PWM3:
            MHal_GPIO_PWM3_High();
        break;
        case PAD_PWM4:
            MHal_GPIO_PWM4_High();
        break;
        case PAD_DDCR_DA:
            MHal_GPIO_DDCR_DA_High();
        break;
        case PAD_DDCR_CK:
            MHal_GPIO_DDCR_CK_High();
        break;
        case PAD_TGPIO0:
            MHal_GPIO_TGPIO0_High();
        break;
        case PAD_TGPIO1:
            MHal_GPIO_TGPIO1_High();
        break;
        case PAD_TGPIO2:
            MHal_GPIO_TGPIO2_High();
        break;
        case PAD_TGPIO3:
            MHal_GPIO_TGPIO3_High();
        break;
        case PAD_TS0_D0:
            MHal_GPIO_TS0_D0_High();
        break;
        case PAD_TS0_D1:
            MHal_GPIO_TS0_D1_High();
        break;
        case PAD_TS0_D2:
            MHal_GPIO_TS0_D2_High();
        break;
        case PAD_TS0_D3:
            MHal_GPIO_TS0_D3_High();
        break;
        case PAD_TS0_D4:
            MHal_GPIO_TS0_D4_High();
        break;

        case PAD_TS0_D5:
            MHal_GPIO_TS0_D5_High();
        break;
        case PAD_TS0_D6:
            MHal_GPIO_TS0_D6_High();
        break;
        case PAD_TS0_D7:
            MHal_GPIO_TS0_D7_High();
        break;
        case PAD_TS0_VLD:
            MHal_GPIO_TS0_VLD_High();
        break;
        case PAD_TS0_SYNC:
            MHal_GPIO_TS0_SYNC_High();
        break;
        case PAD_TS0_CLK:
            MHal_GPIO_TS0_CLK_High();
        break;
        case PAD_TS1_D0:
            MHal_GPIO_TS1_D0_High();
        break;
        case PAD_TS1_D1:
            MHal_GPIO_TS1_D1_High();
        break;
        case PAD_TS1_D2:
            MHal_GPIO_TS1_D2_High();
        break;
        case PAD_TS1_D3:
            MHal_GPIO_TS1_D3_High();
        break;
        case PAD_TS1_D4:
            MHal_GPIO_TS1_D4_High();
        break;
        case PAD_TS1_D5:
            MHal_GPIO_TS1_D5_High();
        break;
        case PAD_TS1_D6:
            MHal_GPIO_TS1_D6_High();
        break;
        case PAD_TS1_D7:
            MHal_GPIO_TS1_D7_High();
        break;
        case PAD_TS1_VLD:
            MHal_GPIO_TS1_VLD_High();
        break;
        case PAD_TS1_SYNC:
            MHal_GPIO_TS1_SYNC_High();
        break;
        case PAD_TS1_CLK:
            MHal_GPIO_TS1_CLK_High();
        break;
        case PAD_PCM_A4:
            MHal_GPIO_PCM_A4_High();
        break;
        case PAD_PCM_WAIT_N:
            MHal_GPIO_PCM_WAIT_N_High();
        break;
        case PAD_PCM_A5:
            MHal_GPIO_PCM_A5_High();
        break;
        case PAD_PCM_A6:
            MHal_GPIO_PCM_A6_High();
        break;
        case PAD_PCM_A7:
            MHal_GPIO_PCM_A7_High();
        break;
        case PAD_PCM_A12:
            MHal_GPIO_PCM_A12_High();
        break;
        case PAD_PCM_IRQA_N:
            MHal_GPIO_PCM_IRQA_N_High();
        break;
        case PAD_PCM_A14:
            MHal_GPIO_PCM_A14_High();
        break;
        case PAD_PCM_A13:
            MHal_GPIO_PCM_A13_High();
        break;
        case PAD_PCM_A8:
            MHal_GPIO_PCM_A8_High();
        break;
        case PAD_PCM_IOWR_N:
            MHal_GPIO_PCM_IOWR_N_High();
        break;
        case PAD_PCM_A9:
            MHal_GPIO_PCM_A9_High();
        break;
        case PAD_PCM_IORD_N:
            MHal_GPIO_PCM_IORD_N_High();
        break;
        case PAD_PCM_A11:
            MHal_GPIO_PCM_A11_High();
        break;
        case PAD_PCM_OE_N:
            MHal_GPIO_PCM_OE_N_High();
        break;
        case PAD_PCM_A10:
            MHal_GPIO_PCM_A10_High();
        break;
        case PAD_PCM_CE_N:
            MHal_GPIO_PCM_CE_N_High();
        break;
        case PAD_PCM_D7:
            MHal_GPIO_PCM_D7_High();
        break;
        case PAD_PCM_D6:
            MHal_GPIO_PCM_D6_High();
        break;
        case PAD_PCM_D5:
            MHal_GPIO_PCM_D5_High();
        break;
        case PAD_PCM_D4:
            MHal_GPIO_PCM_D4_High();
        break;
        case PAD_PCM_D3:
            MHal_GPIO_PCM_D3_High();
        break;
        case PAD_PCM_A3:
            MHal_GPIO_PCM_A3_High();
        break;
        case PAD_PCM_A2:
            MHal_GPIO_PCM_A2_High();
        break;
        case PAD_PCM_REG_N:
            MHal_GPIO_PCM_REG_N_High();
        break;
        case PAD_PCM_A1:
            MHal_GPIO_PCM_A1_High();
        break;
        case PAD_PCM_A0:
            MHal_GPIO_PCM_A0_High();
        break;
        case PAD_PCM_D0:
            MHal_GPIO_PCM_D0_High();
        break;
        case PAD_PCM_D1:
            MHal_GPIO_PCM_D1_High();
        break;
        case PAD_PCM_D2:
            MHal_GPIO_PCM_D2_High();
        break;
        case PAD_PCM_RESET:
            MHal_GPIO_PCM_RESET_High();
        break;
        case PAD_PCM_CD_N:
            MHal_GPIO_PCM_CD_N_High();
        break;
        case PAD_PCM2_CE_N:
            MHal_GPIO_PCM2_CE_N_High();
        break;
        case PAD_PCM2_IRQA_N:
            MHal_GPIO_PCM2_IRQA_N_High();
        break;
        case PAD_PCM2_WAIT_N:
            MHal_GPIO_PCM2_WAIT_N_High();
        break;
        case PAD_PCM2_RESET:
            MHal_GPIO_PCM2_RESET_High();
        break;
        case PAD_PCM2_CD_N:
            MHal_GPIO_PCM2_CD_N_High();
        break;
        case PAD_PF_AD15:
            MHal_GPIO_PF_AD15_High();
        break;
        case PAD_PF_CE0Z:
            MHal_GPIO_PF_CE0Z_High();
        break;
        case PAD_PF_CE1Z:
            MHal_GPIO_PF_CE1Z_High();
        break;
        case PAD_PF_OEZ:
            MHal_GPIO_PF_OEZ_High();
        break;
        case PAD_PF_WEZ:
            MHal_GPIO_PF_WEZ_High();
        break;
        case PAD_PF_ALE:
            MHal_GPIO_PF_ALE_High();
        break;
        case PAD_F_RBZ:
            MHal_GPIO_F_RBZ_High();
        break;
        case PAD_TCON0:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO48_High();
				}
				else
				{
					MHal_GPIO_TCON0_High();
				}
			}
			else	// PDP
				MHal_GPIO_TCON0_High();
		}
#else
			MHal_GPIO_TCON0_High();
#endif

        break;
        case PAD_TCON1:
            MHal_GPIO_TCON1_High();
        break;
        case PAD_TCON2:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO49_High();
				}
				else
				{
					MHal_GPIO_TCON2_High();
				}
			}
			else	// PDP
				MHal_GPIO_TCON2_High();
		}
#else
			MHal_GPIO_TCON2_High();
#endif
        break;
        case PAD_TCON3:
            MHal_GPIO_TCON3_High();
        break;
        case PAD_TCON4:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO53_High();
				}
				else
				{
					MHal_GPIO_TCON4_High();
				}
			}
			else	// PDP
				MHal_GPIO_TCON4_High();
		}
#else
			MHal_GPIO_TCON4_High();
#endif

        break;
        case PAD_TCON5:
            MHal_GPIO_TCON5_High();
        break;
        case PAD_TCON6:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO52_High();
				}
				else
				{
					MHal_GPIO_TCON6_High();
				}
			}
			else	// PDP
				MHal_GPIO_TCON6_High();
		}
#else
			MHal_GPIO_TCON6_High();
#endif
        break;
        case PAD_TCON7:
            MHal_GPIO_TCON7_High();
        break;
        case PAD_TCON8:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO47_High();
				}
				else
				{
					MHal_GPIO_TCON8_High();
				}
			}
			else	// PDP
				MHal_GPIO_TCON8_High();
		}
#else
			MHal_GPIO_TCON8_High();
#endif
        break;
        case PAD_TCON9:
            MHal_GPIO_TCON9_High();
        break;
        case PAD_TCON10:
            MHal_GPIO_TCON10_High();
        break;
        case PAD_TCON11:
            MHal_GPIO_TCON11_High();
        break;
        case PAD_TCON12:
            MHal_GPIO_TCON12_High();
        break;
        case PAD_TCON13:
            MHal_GPIO_TCON13_High();
        break;
        case PAD_TCON14:
            MHal_GPIO_TCON14_High();
        break;
        case PAD_TCON15:
            MHal_GPIO_TCON15_High();
        break;
        case PAD_TCON16:
            MHal_GPIO_TCON16_High();
        break;
        case PAD_TCON17:
            MHal_GPIO_TCON17_High();
        break;
        case PAD_TCON18:
            MHal_GPIO_TCON18_High();
        break;
        case PAD_TCON19:
            MHal_GPIO_TCON19_High();
        break;
        case PAD_TCON20:
            MHal_GPIO_TCON20_High();
        break;
        case PAD_TCON21:
            MHal_GPIO_TCON21_High();
        break;
        case PAD_ET_COL:
            MHal_GPIO_ET_COL_High();
        break;
        case PAD_ET_TXD1:
            MHal_GPIO_ET_TXD1_High();
        break;
        case PAD_ET_TXD0:
            MHal_GPIO_ET_TXD0_High();
        break;
        case PAD_ET_TX_EN:
            MHal_GPIO_ET_TX_EN_High();
        break;
        case PAD_ET_TX_CLK:
            MHal_GPIO_ET_TX_CLK_High();
        break;
        case PAD_ET_RXD0:
            MHal_GPIO_ET_RXD0_High();
        break;
        case PAD_ET_RXD1:
            MHal_GPIO_ET_RXD1_High();
        break;
        case PAD_ET_MDC:
            MHal_GPIO_ET_MDC_High();
        break;
        case PAD_ET_EDIO:
            MHal_GPIO_ET_EDIO_High();
        break;
        case PAD_I2S_IN_WS:
            MHal_GPIO_I2S_IN_WS_High();
        break;
        case PAD_I2S_IN_BCK:
            MHal_GPIO_I2S_IN_BCK_High();
        break;
        case PAD_I2S_IN_SD:
            MHal_GPIO_I2S_IN_SD_High();
        break;
        case PAD_SPDIF_IN:
            MHal_GPIO_SPDIF_IN_High();
        break;
        case PAD_SPDIF_OUT:
            MHal_GPIO_SPDIF_OUT_High();
        break;
        case PAD_I2S_OUT_MCK:
            MHal_GPIO_I2S_OUT_MCK_High();
        break;
        case PAD_I2S_OUT_WS:
            MHal_GPIO_I2S_OUT_WS_High();
        break;
        case PAD_I2S_OUT_BCK:
            MHal_GPIO_I2S_OUT_BCK_High();
        break;
        case PAD_I2S_OUT_SD:
            MHal_GPIO_I2S_OUT_SD_High();
        break;
        case PAD_I2S_OUT_SD1:
            MHal_GPIO_I2S_OUT_SD1_High();
        break;
        case PAD_I2S_OUT_SD2:
            MHal_GPIO_I2S_OUT_SD2_High();
        break;
        case PAD_I2S_OUT_SD3:
            MHal_GPIO_I2S_OUT_SD3_High();
        break;
        case PAD_B_ODD_0:
            MHal_GPIO_B_ODD_0_High();
        break;
        case PAD_B_ODD_1:
            MHal_GPIO_B_ODD_1_High();
        break;
        case PAD_B_ODD_2:
            MHal_GPIO_B_ODD_2_High();
        break;
        case PAD_B_ODD_3:
            MHal_GPIO_B_ODD_3_High();
        break;
        case PAD_B_ODD_4:
            MHal_GPIO_B_ODD_4_High();
        break;
        case PAD_B_ODD_5:
            MHal_GPIO_B_ODD_5_High();
        break;
        case PAD_B_ODD_6:
            MHal_GPIO_B_ODD_6_High();
        break;
        case PAD_B_ODD_7:
            MHal_GPIO_B_ODD_7_High();
        break;
        case PAD_G_ODD_0:
            MHal_GPIO_G_ODD_0_High();
        break;
        case PAD_G_ODD_1:
            MHal_GPIO_G_ODD_1_High();
        break;
        case PAD_G_ODD_2:
            MHal_GPIO_G_ODD_2_High();
        break;
        case PAD_G_ODD_3:
            MHal_GPIO_G_ODD_3_High();
        break;
        case PAD_G_ODD_4:
            MHal_GPIO_G_ODD_4_High();
        break;
        case PAD_G_ODD_5:
            MHal_GPIO_G_ODD_5_High();
        break;
        case PAD_G_ODD_6:
            MHal_GPIO_G_ODD_6_High();
        break;
        case PAD_G_ODD_7:
            MHal_GPIO_G_ODD_7_High();
        break;
        case PAD_R_ODD_0:
            MHal_GPIO_R_ODD_0_High();
        break;
        case PAD_R_ODD_1:
            MHal_GPIO_R_ODD_1_High();
        break;
        case PAD_R_ODD_2:
            MHal_GPIO_R_ODD_2_High();
        break;
        case PAD_R_ODD_3:
            MHal_GPIO_R_ODD_3_High();
        break;
        case PAD_R_ODD_4:
            MHal_GPIO_R_ODD_4_High();
        break;
        case PAD_R_ODD_5:
            MHal_GPIO_R_ODD_5_High();
        break;
        case PAD_R_ODD_6:
            MHal_GPIO_R_ODD_6_High();
        break;
        case PAD_R_ODD_7:
            MHal_GPIO_R_ODD_7_High();
        break;
        case PAD_mini_LVDS_0:
            MHal_GPIO_mini_LVDS_0_High();
        break;
        case PAD_mini_LVDS_1:
            MHal_GPIO_mini_LVDS_1_High();
        break;
        case PAD_mini_LVDS_2:
            MHal_GPIO_mini_LVDS_2_High();
        break;
        case PAD_mini_LVDS_3:
            MHal_GPIO_mini_LVDS_3_High();
        break;
        case PAD_mini_LVDS_4:
            MHal_GPIO_mini_LVDS_4_High();
        break;
        case PAD_mini_LVDS_5:
            MHal_GPIO_mini_LVDS_5_High();
        break;
        case PAD_mini_LVDS_6:
            MHal_GPIO_mini_LVDS_6_High();
        break;
        case PAD_mini_LVDS_7:
            MHal_GPIO_mini_LVDS_7_High();
        break;

        case PAD_LCK:
            MHal_GPIO_LCK_High();
        break;
        case PAD_LDE:
            MHal_GPIO_LDE_High();
        break;
        case PAD_LHSYNC:
            MHal_GPIO_LHSYNC_High();
        break;
        case PAD_LVSYNC:
            MHal_GPIO_LVSYNC_High();
        break;
        case PAD_PCM_WE_N:
            MHal_GPIO_PCM_WE_N_High();
        break;



        default:
            break;
    }
}
EXPORT_SYMBOL(MDrv_GPIO_Pull_High);

//-------------------------------------------------------------------------------------------------
/// output pull low for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pull_Low(U8 u8IndexGPIO)
{
//    U8  u8RegTemp;

    switch(u8IndexGPIO)
    {
        case PAD_PM_SPI_CZ:
            MHal_GPIO_PM_SPI_CZ_Low();
        break;
        case PAD_PM_SPI_CK:
            MHal_GPIO_PM_SPI_CK_Low();
        break;
        case PAD_PM_SPI_DI:
            MHal_GPIO_PM_SPI_DI_Low();
        break;
        case PAD_PM_SPI_DO:
            MHal_GPIO_PM_SPI_DO_Low();
        break;
        case PAD_IRIN:
            MHal_GPIO_IRIN_Low();
        break;
        case PAD_CEC:
            MHal_GPIO_CEC_Low();
        break;
        case PAD_GPIO_PM0:
            MHal_GPIO_GPIO_PM0_Low();
        break;
        case PAD_GPIO_PM1:
            MHal_GPIO_GPIO_PM1_Low();
        break;
        case PAD_GPIO_PM2:
            MHal_GPIO_GPIO_PM2_Low();
        break;
        case PAD_GPIO_PM3:
            MHal_GPIO_GPIO_PM3_Low();
        break;
        case PAD_GPIO_PM4:
            MHal_GPIO_GPIO_PM4_Low();
        break;
        case PAD_GPIO_PM5:
            MHal_GPIO_GPIO_PM5_Low();
        break;
        case PAD_GPIO_PM6:
            MHal_GPIO_GPIO_PM6_Low();
        break;
        case PAD_GPIO_PM7:
            MHal_GPIO_GPIO_PM7_Low();
        break;
        case PAD_GPIO_PM8:
            MHal_GPIO_GPIO_PM8_Low();
        break;
        case PAD_GPIO_PM9:
            MHal_GPIO_GPIO_PM9_Low();
        break;
        case PAD_GPIO_PM10:
            MHal_GPIO_GPIO_PM10_Low();
        break;
        case PAD_GPIO_PM11:
            MHal_GPIO_GPIO_PM11_Low();
        break;
        case PAD_GPIO_PM12:
            MHal_GPIO_GPIO_PM12_Low();
        break;
        case PAD_HOTPLUGA:
            MHal_GPIO_HOTPLUGA_Low();
        break;
        case PAD_HOTPLUGB:
            MHal_GPIO_HOTPLUGB_Low();
        break;
        case PAD_HOTPLUGC:
            MHal_GPIO_HOTPLUGC_Low();
        break;
        case PAD_HOTPLUGD:
            MHal_GPIO_HOTPLUGD_Low();
        break;
        case PAD_DDCDA_CK:
            MHal_GPIO_DDCDA_CK_Low();
        break;
        case PAD_DDCDA_DA:
            MHal_GPIO_DDCDA_DA_Low();
        break;
        case PAD_DDCDB_CK:
            MHal_GPIO_DDCDB_CK_Low();
        break;
        case PAD_DDCDB_DA:
            MHal_GPIO_DDCDB_DA_Low();
        break;
        case PAD_DDCDC_CK:
            MHal_GPIO_DDCDC_CK_Low();
        break;
        case PAD_DDCDC_DA:
            MHal_GPIO_DDCDC_DA_Low();
        break;
        case PAD_DDCDD_CK:
            MHal_GPIO_DDCDD_CK_Low();
        break;
        case PAD_DDCDD_DA:
            MHal_GPIO_DDCDD_DA_Low();
        break;
        case PAD_SAR0:
            MHal_GPIO_SAR0_Low();
        break;
        case PAD_SAR1:
            MHal_GPIO_SAR1_Low();
        break;
        case PAD_SAR2:
            MHal_GPIO_SAR2_Low();
        break;
        case PAD_SAR3:
            MHal_GPIO_SAR3_Low();
        break;
        case PAD_SAR4:
            MHal_GPIO_SAR4_Low();
        break;
        case PAD_GPIO0:
            MHal_GPIO_GPIO0_Low();
        break;
        case PAD_GPIO1:
            MHal_GPIO_GPIO1_Low();
        break;
        case PAD_GPIO2:
            MHal_GPIO_GPIO2_Low();
        break;
        case PAD_GPIO3:
            MHal_GPIO_GPIO3_Low();
        break;
        case PAD_GPIO4:
            MHal_GPIO_GPIO4_Low();
        break;
        case PAD_GPIO5:
            MHal_GPIO_GPIO5_Low();
        break;
        case PAD_GPIO6:
            MHal_GPIO_GPIO6_Low();
        break;
        case PAD_GPIO7:
            MHal_GPIO_GPIO7_Low();
        break;
        case PAD_GPIO8:
            MHal_GPIO_GPIO8_Low();
        break;
        case PAD_GPIO9:
            MHal_GPIO_GPIO9_Low();
        break;
        case PAD_GPIO10:
            MHal_GPIO_GPIO10_Low();
        break;
        case PAD_GPIO11:
            MHal_GPIO_GPIO11_Low();
        break;
        case PAD_GPIO12:
            MHal_GPIO_GPIO12_Low();
        break;
        case PAD_GPIO13:
            MHal_GPIO_GPIO13_Low();
        break;
        case PAD_GPIO14:
            MHal_GPIO_GPIO14_Low();
        break;
        case PAD_GPIO15:
            MHal_GPIO_GPIO15_Low();
        break;
        case PAD_GPIO16:
            MHal_GPIO_GPIO16_Low();
        break;
        case PAD_GPIO17:
            MHal_GPIO_GPIO17_Low();
        break;
        case PAD_GPIO18:
            MHal_GPIO_GPIO18_Low();
        break;
        case PAD_GPIO19:
            MHal_GPIO_GPIO19_Low();
        break;
        case PAD_GPIO20:
            MHal_GPIO_GPIO20_Low();
        break;
        case PAD_GPIO21:
            MHal_GPIO_GPIO21_Low();
        break;
        case PAD_GPIO22:
            MHal_GPIO_GPIO22_Low();
        break;
        case PAD_GPIO23:
            MHal_GPIO_GPIO23_Low();
        break;
        case PAD_GPIO24:
            MHal_GPIO_GPIO24_Low();
        break;
        case PAD_GPIO25:
            MHal_GPIO_GPIO25_Low();
        break;
        case PAD_GPIO26:
            MHal_GPIO_GPIO26_Low();
        break;
        case PAD_GPIO27:
            MHal_GPIO_GPIO27_Low();
        break;
        case PAD_UART_RX2:
            MHal_GPIO_UART_RX2_Low();
        break;
        case PAD_UART_TX2:
            MHal_GPIO_UART_TX2_Low();
        break;
        case PAD_PWM0:
            MHal_GPIO_PWM0_Low();
        break;
        case PAD_PWM1:
            MHal_GPIO_PWM1_Low();
        break;
        case PAD_PWM2:
            MHal_GPIO_PWM2_Low();
        break;
        case PAD_PWM3:
            MHal_GPIO_PWM3_Low();
        break;
        case PAD_PWM4:
            MHal_GPIO_PWM4_Low();
        break;
        case PAD_DDCR_DA:
            MHal_GPIO_DDCR_DA_Low();
        break;
        case PAD_DDCR_CK:
            MHal_GPIO_DDCR_CK_Low();
        break;
        case PAD_TGPIO0:
            MHal_GPIO_TGPIO0_Low();
        break;
        case PAD_TGPIO1:
            MHal_GPIO_TGPIO1_Low();
        break;
        case PAD_TGPIO2:
            MHal_GPIO_TGPIO2_Low();
        break;
        case PAD_TGPIO3:
            MHal_GPIO_TGPIO3_Low();
        break;
        case PAD_TS0_D0:
            MHal_GPIO_TS0_D0_Low();
        break;
        case PAD_TS0_D1:
            MHal_GPIO_TS0_D1_Low();
        break;
        case PAD_TS0_D2:
            MHal_GPIO_TS0_D2_Low();
        break;
        case PAD_TS0_D3:
            MHal_GPIO_TS0_D3_Low();
        break;
        case PAD_TS0_D4:
            MHal_GPIO_TS0_D4_Low();
        break;

        case PAD_TS0_D5:
            MHal_GPIO_TS0_D5_Low();
        break;
        case PAD_TS0_D6:
            MHal_GPIO_TS0_D6_Low();
        break;
        case PAD_TS0_D7:
            MHal_GPIO_TS0_D7_Low();
        break;
        case PAD_TS0_VLD:
            MHal_GPIO_TS0_VLD_Low();
        break;
        case PAD_TS0_SYNC:
            MHal_GPIO_TS0_SYNC_Low();
        break;
        case PAD_TS0_CLK:
            MHal_GPIO_TS0_CLK_Low();
        break;
        case PAD_TS1_D0:
            MHal_GPIO_TS1_D0_Low();
        break;
        case PAD_TS1_D1:
            MHal_GPIO_TS1_D1_Low();
        break;
        case PAD_TS1_D2:
            MHal_GPIO_TS1_D2_Low();
        break;
        case PAD_TS1_D3:
            MHal_GPIO_TS1_D3_Low();
        break;
        case PAD_TS1_D4:
            MHal_GPIO_TS1_D4_Low();
        break;
        case PAD_TS1_D5:
            MHal_GPIO_TS1_D5_Low();
        break;
        case PAD_TS1_D6:
            MHal_GPIO_TS1_D6_Low();
        break;
        case PAD_TS1_D7:
            MHal_GPIO_TS1_D7_Low();
        break;
        case PAD_TS1_VLD:
            MHal_GPIO_TS1_VLD_Low();
        break;
        case PAD_TS1_SYNC:
            MHal_GPIO_TS1_SYNC_Low();
        break;
        case PAD_TS1_CLK:
            MHal_GPIO_TS1_CLK_Low();
        break;
        case PAD_PCM_A4:
            MHal_GPIO_PCM_A4_Low();
        break;
        case PAD_PCM_WAIT_N:
            MHal_GPIO_PCM_WAIT_N_Low();
        break;
        case PAD_PCM_A5:
            MHal_GPIO_PCM_A5_Low();
        break;
        case PAD_PCM_A6:
            MHal_GPIO_PCM_A6_Low();
        break;
        case PAD_PCM_A7:
            MHal_GPIO_PCM_A7_Low();
        break;
        case PAD_PCM_A12:
            MHal_GPIO_PCM_A12_Low();
        break;
        case PAD_PCM_IRQA_N:
            MHal_GPIO_PCM_IRQA_N_Low();
        break;
        case PAD_PCM_A14:
            MHal_GPIO_PCM_A14_Low();
        break;
        case PAD_PCM_A13:
            MHal_GPIO_PCM_A13_Low();
        break;
        case PAD_PCM_A8:
            MHal_GPIO_PCM_A8_Low();
        break;
        case PAD_PCM_IOWR_N:
            MHal_GPIO_PCM_IOWR_N_Low();
        break;
        case PAD_PCM_A9:
            MHal_GPIO_PCM_A9_Low();
        break;
        case PAD_PCM_IORD_N:
            MHal_GPIO_PCM_IORD_N_Low();
        break;
        case PAD_PCM_A11:
            MHal_GPIO_PCM_A11_Low();
        break;
        case PAD_PCM_OE_N:
            MHal_GPIO_PCM_OE_N_Low();
        break;
        case PAD_PCM_A10:
            MHal_GPIO_PCM_A10_Low();
        break;
        case PAD_PCM_CE_N:
            MHal_GPIO_PCM_CE_N_Low();
        break;
        case PAD_PCM_D7:
            MHal_GPIO_PCM_D7_Low();
        break;
        case PAD_PCM_D6:
            MHal_GPIO_PCM_D6_Low();
        break;
        case PAD_PCM_D5:
            MHal_GPIO_PCM_D5_Low();
        break;
        case PAD_PCM_D4:
            MHal_GPIO_PCM_D4_Low();
        break;
        case PAD_PCM_D3:
            MHal_GPIO_PCM_D3_Low();
        break;
        case PAD_PCM_A3:
            MHal_GPIO_PCM_A3_Low();
        break;
        case PAD_PCM_A2:
            MHal_GPIO_PCM_A2_Low();
        break;
        case PAD_PCM_REG_N:
            MHal_GPIO_PCM_REG_N_Low();
        break;
        case PAD_PCM_A1:
            MHal_GPIO_PCM_A1_Low();
        break;
        case PAD_PCM_A0:
            MHal_GPIO_PCM_A0_Low();
        break;
        case PAD_PCM_D0:
            MHal_GPIO_PCM_D0_Low();
        break;
        case PAD_PCM_D1:
            MHal_GPIO_PCM_D1_Low();
        break;
        case PAD_PCM_D2:
            MHal_GPIO_PCM_D2_Low();
        break;
        case PAD_PCM_RESET:
            MHal_GPIO_PCM_RESET_Low();
        break;
        case PAD_PCM_CD_N:
            MHal_GPIO_PCM_CD_N_Low();
        break;
        case PAD_PCM2_CE_N:
            MHal_GPIO_PCM2_CE_N_Low();
        break;
        case PAD_PCM2_IRQA_N:
            MHal_GPIO_PCM2_IRQA_N_Low();
        break;
        case PAD_PCM2_WAIT_N:
            MHal_GPIO_PCM2_WAIT_N_Low();
        break;
        case PAD_PCM2_RESET:
            MHal_GPIO_PCM2_RESET_Low();
        break;
        case PAD_PCM2_CD_N:
            MHal_GPIO_PCM2_CD_N_Low();
        break;
        case PAD_PF_AD15:
            MHal_GPIO_PF_AD15_Low();
        break;
        case PAD_PF_CE0Z:
            MHal_GPIO_PF_CE0Z_Low();
        break;
        case PAD_PF_CE1Z:
            MHal_GPIO_PF_CE1Z_Low();
        break;
        case PAD_PF_OEZ:
            MHal_GPIO_PF_OEZ_Low();
        break;
        case PAD_PF_WEZ:
            MHal_GPIO_PF_WEZ_Low();
        break;
        case PAD_PF_ALE:
            MHal_GPIO_PF_ALE_Low();
        break;
        case PAD_F_RBZ:
            MHal_GPIO_F_RBZ_Low();
        break;
        case PAD_TCON0:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO48_Low();
				}
				else
				{
					MHal_GPIO_TCON0_Low();
				}
			}
			else	// PDP
				MHal_GPIO_TCON0_Low();
		}
#else
			MHal_GPIO_TCON0_Low();
#endif

        break;
        case PAD_TCON1:
            MHal_GPIO_TCON1_Low();
        break;
        case PAD_TCON2:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();
			
			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO49_Low();
				}
				else
				{
					MHal_GPIO_TCON2_Low();
				}
			}
			else	// PDP
				MHal_GPIO_TCON2_Low();
		}
#else
			MHal_GPIO_TCON2_Low();
#endif
        break;
        case PAD_TCON3:
            MHal_GPIO_TCON3_Low();
        break;
        case PAD_TCON4:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO53_Low();
				}
				else
				{
					MHal_GPIO_TCON4_Low();
				}
			}
			else	// PDP
				MHal_GPIO_TCON4_Low();
		}
#else
			MHal_GPIO_TCON4_Low();
#endif
        break;
        case PAD_TCON5:
            MHal_GPIO_TCON5_Low();
        break;
        case PAD_TCON6:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO52_Low();
				}
				else
				{
					MHal_GPIO_TCON6_Low();
				}
			}
			else	// PDP
				MHal_GPIO_TCON6_Low();
		}
#else
			MHal_GPIO_TCON6_Low();
#endif
        break;
        case PAD_TCON7:
            MHal_GPIO_TCON7_Low();
        break;
        case PAD_TCON8:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO47_Low();
				}
				else
				{
					MHal_GPIO_TCON8_Low();
				}
			}
			else	// PDP
				MHal_GPIO_TCON8_Low();
		}
#else
			MHal_GPIO_TCON8_Low();
#endif
        break;
        case PAD_TCON9:
            MHal_GPIO_TCON9_Low();
        break;
        case PAD_TCON10:
            MHal_GPIO_TCON10_Low();
        break;
        case PAD_TCON11:
            MHal_GPIO_TCON11_Low();
        break;
        case PAD_TCON12:
            MHal_GPIO_TCON12_Low();
        break;
        case PAD_TCON13:
            MHal_GPIO_TCON13_Low();
        break;
        case PAD_TCON14:
            MHal_GPIO_TCON14_Low();
        break;
        case PAD_TCON15:
            MHal_GPIO_TCON15_Low();
        break;
        case PAD_TCON16:
            MHal_GPIO_TCON16_Low();
        break;
        case PAD_TCON17:
            MHal_GPIO_TCON17_Low();
        break;
        case PAD_TCON18:
            MHal_GPIO_TCON18_Low();
        break;
        case PAD_TCON19:
            MHal_GPIO_TCON19_Low();
        break;
        case PAD_TCON20:
            MHal_GPIO_TCON20_Low();
        break;
        case PAD_TCON21:
            MHal_GPIO_TCON21_Low();
        break;
        case PAD_ET_COL:
            MHal_GPIO_ET_COL_Low();
        break;
        case PAD_ET_TXD1:
            MHal_GPIO_ET_TXD1_Low();
        break;
        case PAD_ET_TXD0:
            MHal_GPIO_ET_TXD0_Low();
        break;
        case PAD_ET_TX_EN:
            MHal_GPIO_ET_TX_EN_Low();
        break;
        case PAD_ET_TX_CLK:
            MHal_GPIO_ET_TX_CLK_Low();
        break;
        case PAD_ET_RXD0:
            MHal_GPIO_ET_RXD0_Low();
        break;
        case PAD_ET_RXD1:
            MHal_GPIO_ET_RXD1_Low();
        break;
        case PAD_ET_MDC:
            MHal_GPIO_ET_MDC_Low();
        break;
        case PAD_ET_EDIO:
            MHal_GPIO_ET_EDIO_Low();
        break;
        case PAD_I2S_IN_WS:
            MHal_GPIO_I2S_IN_WS_Low();
        break;
        case PAD_I2S_IN_BCK:
            MHal_GPIO_I2S_IN_BCK_Low();
        break;
        case PAD_I2S_IN_SD:
            MHal_GPIO_I2S_IN_SD_Low();
        break;
        case PAD_SPDIF_IN:
            MHal_GPIO_SPDIF_IN_Low();
        break;
        case PAD_SPDIF_OUT:
            MHal_GPIO_SPDIF_OUT_Low();
        break;
        case PAD_I2S_OUT_MCK:
            MHal_GPIO_I2S_OUT_MCK_Low();
        break;
        case PAD_I2S_OUT_WS:
            MHal_GPIO_I2S_OUT_WS_Low();
        break;
        case PAD_I2S_OUT_BCK:
            MHal_GPIO_I2S_OUT_BCK_Low();
        break;
        case PAD_I2S_OUT_SD:
            MHal_GPIO_I2S_OUT_SD_Low();
        break;
        case PAD_I2S_OUT_SD1:
            MHal_GPIO_I2S_OUT_SD1_Low();
        break;
        case PAD_I2S_OUT_SD2:
            MHal_GPIO_I2S_OUT_SD2_Low();
        break;
        case PAD_I2S_OUT_SD3:
            MHal_GPIO_I2S_OUT_SD3_Low();
        break;
        case PAD_B_ODD_0:
            MHal_GPIO_B_ODD_0_Low();
        break;
        case PAD_B_ODD_1:
            MHal_GPIO_B_ODD_1_Low();
        break;
        case PAD_B_ODD_2:
            MHal_GPIO_B_ODD_2_Low();
        break;
        case PAD_B_ODD_3:
            MHal_GPIO_B_ODD_3_Low();
        break;
        case PAD_B_ODD_4:
            MHal_GPIO_B_ODD_4_Low();
        break;
        case PAD_B_ODD_5:
            MHal_GPIO_B_ODD_5_Low();
        break;
        case PAD_B_ODD_6:
            MHal_GPIO_B_ODD_6_Low();
        break;
        case PAD_B_ODD_7:
            MHal_GPIO_B_ODD_7_Low();
        break;
        case PAD_G_ODD_0:
            MHal_GPIO_G_ODD_0_Low();
        break;
        case PAD_G_ODD_1:
            MHal_GPIO_G_ODD_1_Low();
        break;
        case PAD_G_ODD_2:
            MHal_GPIO_G_ODD_2_Low();
        break;
        case PAD_G_ODD_3:
            MHal_GPIO_G_ODD_3_Low();
        break;
        case PAD_G_ODD_4:
            MHal_GPIO_G_ODD_4_Low();
        break;
        case PAD_G_ODD_5:
            MHal_GPIO_G_ODD_5_Low();
        break;
        case PAD_G_ODD_6:
            MHal_GPIO_G_ODD_6_Low();
        break;
        case PAD_G_ODD_7:
            MHal_GPIO_G_ODD_7_Low();
        break;
        case PAD_R_ODD_0:
            MHal_GPIO_R_ODD_0_Low();
        break;
        case PAD_R_ODD_1:
            MHal_GPIO_R_ODD_1_Low();
        break;
        case PAD_R_ODD_2:
            MHal_GPIO_R_ODD_2_Low();
        break;
        case PAD_R_ODD_3:
            MHal_GPIO_R_ODD_3_Low();
        break;
        case PAD_R_ODD_4:
            MHal_GPIO_R_ODD_4_Low();
        break;
        case PAD_R_ODD_5:
            MHal_GPIO_R_ODD_5_Low();
        break;
        case PAD_R_ODD_6:
            MHal_GPIO_R_ODD_6_Low();
        break;
        case PAD_R_ODD_7:
            MHal_GPIO_R_ODD_7_Low();
        break;
        case PAD_mini_LVDS_0:
            MHal_GPIO_mini_LVDS_0_Low();
        break;
        case PAD_mini_LVDS_1:
            MHal_GPIO_mini_LVDS_1_Low();
        break;
        case PAD_mini_LVDS_2:
            MHal_GPIO_mini_LVDS_2_Low();
        break;
        case PAD_mini_LVDS_3:
            MHal_GPIO_mini_LVDS_3_Low();
        break;
        case PAD_mini_LVDS_4:
            MHal_GPIO_mini_LVDS_4_Low();
        break;
        case PAD_mini_LVDS_5:
            MHal_GPIO_mini_LVDS_5_Low();
        break;
        case PAD_mini_LVDS_6:
            MHal_GPIO_mini_LVDS_6_Low();
        break;
        case PAD_mini_LVDS_7:
            MHal_GPIO_mini_LVDS_7_Low();
        break;

        case PAD_LCK:
            MHal_GPIO_LCK_Low();
        break;
        case PAD_LDE:
            MHal_GPIO_LDE_Low();
        break;
        case PAD_LHSYNC:
            MHal_GPIO_LHSYNC_Low();
        break;
        case PAD_LVSYNC:
            MHal_GPIO_LVSYNC_Low();
        break;
        case PAD_PCM_WE_N:
            MHal_GPIO_PCM_WE_N_Low();
        break;



        default:
            break;
    }
}
EXPORT_SYMBOL(MDrv_GPIO_Pull_Low);

//-------------------------------------------------------------------------------------------------
/// output pull high for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Set_High(U8 u8IndexGPIO)
{
    switch(u8IndexGPIO)
    {
        case PAD_PM_SPI_CZ:
            MHal_GPIO_PM_SPI_CZ_Set();
            MHal_GPIO_PM_SPI_CZ_Oen();
            MHal_GPIO_PM_SPI_CZ_High();
        break;
        case PAD_PM_SPI_CK:
            MHal_GPIO_PM_SPI_CK_Set();
            MHal_GPIO_PM_SPI_CK_Oen();
            MHal_GPIO_PM_SPI_CK_High();

        break;
        case PAD_PM_SPI_DI:
            MHal_GPIO_PM_SPI_DI_Set();
            MHal_GPIO_PM_SPI_DI_Oen();
            MHal_GPIO_PM_SPI_DI_High();

        break;
        case PAD_PM_SPI_DO:
            MHal_GPIO_PM_SPI_DO_Set();
            MHal_GPIO_PM_SPI_DO_Oen();
            MHal_GPIO_PM_SPI_DO_High();

        break;
        case PAD_IRIN:
            MHal_GPIO_IRIN_Set();
            MHal_GPIO_IRIN_Oen();
            MHal_GPIO_IRIN_High();
        break;
        case PAD_CEC:
            MHal_GPIO_CEC_Set();
            MHal_GPIO_CEC_Oen();
            MHal_GPIO_CEC_High();
        break;
        case PAD_GPIO_PM0:
            MHal_GPIO_GPIO_PM0_Set();
            MHal_GPIO_GPIO_PM0_Oen();
            MHal_GPIO_GPIO_PM0_High();
        break;
        case PAD_GPIO_PM1:
            MHal_GPIO_GPIO_PM1_Set();
            MHal_GPIO_GPIO_PM1_Oen();
            MHal_GPIO_GPIO_PM1_High();
        break;
        case PAD_GPIO_PM2:
            MHal_GPIO_GPIO_PM2_Set();
            MHal_GPIO_GPIO_PM2_Oen();
            MHal_GPIO_GPIO_PM2_High();
        break;
        case PAD_GPIO_PM3:
            MHal_GPIO_GPIO_PM3_Set();
            MHal_GPIO_GPIO_PM3_Oen();
            MHal_GPIO_GPIO_PM3_High();

        break;
        case PAD_GPIO_PM4:
            MHal_GPIO_GPIO_PM4_Set();
            MHal_GPIO_GPIO_PM4_Oen();
            MHal_GPIO_GPIO_PM4_High();

        break;
        case PAD_GPIO_PM5:
            MHal_GPIO_GPIO_PM5_Set();
            MHal_GPIO_GPIO_PM5_Oen();
            MHal_GPIO_GPIO_PM5_High();

        break;
        case PAD_GPIO_PM6:
            MHal_GPIO_GPIO_PM6_Set();
            MHal_GPIO_GPIO_PM6_Oen();
            MHal_GPIO_GPIO_PM6_High();

        break;
        case PAD_GPIO_PM7:
            MHal_GPIO_GPIO_PM7_Set();
            MHal_GPIO_GPIO_PM7_Oen();
            MHal_GPIO_GPIO_PM7_High();


        break;
        case PAD_GPIO_PM8:
            MHal_GPIO_GPIO_PM8_Set();
            MHal_GPIO_GPIO_PM8_Oen();
            MHal_GPIO_GPIO_PM8_High();

        break;
        case PAD_GPIO_PM9:
            MHal_GPIO_GPIO_PM9_Set();
            MHal_GPIO_GPIO_PM9_Oen();
            MHal_GPIO_GPIO_PM9_High();

        break;
        case PAD_GPIO_PM10:
            MHal_GPIO_GPIO_PM10_Set();
            MHal_GPIO_GPIO_PM10_Oen();
            MHal_GPIO_GPIO_PM10_High();

        break;
        case PAD_GPIO_PM11:
            MHal_GPIO_GPIO_PM11_Set();
            MHal_GPIO_GPIO_PM11_Oen();
            MHal_GPIO_GPIO_PM11_High();

        break;
        case PAD_GPIO_PM12:
            MHal_GPIO_GPIO_PM12_Set();
            MHal_GPIO_GPIO_PM12_Oen();
            MHal_GPIO_GPIO_PM12_High();

        break;
        case PAD_HOTPLUGA:
            MHal_GPIO_HOTPLUGA_Set();
            MHal_GPIO_HOTPLUGA_Oen();
            MHal_GPIO_HOTPLUGA_High();

        break;
        case PAD_HOTPLUGB:
            MHal_GPIO_HOTPLUGB_Set();
            MHal_GPIO_HOTPLUGB_Oen();
            MHal_GPIO_HOTPLUGB_High();

        break;
        case PAD_HOTPLUGC:
            MHal_GPIO_HOTPLUGC_Set();
            MHal_GPIO_HOTPLUGC_Oen();
            MHal_GPIO_HOTPLUGC_High();

        break;
        case PAD_HOTPLUGD:
            MHal_GPIO_HOTPLUGD_Set();
            MHal_GPIO_HOTPLUGD_Oen();
            MHal_GPIO_HOTPLUGD_High();

        break;
        case PAD_DDCDA_CK:
            MHal_GPIO_DDCDA_CK_Set();
            MHal_GPIO_DDCDA_CK_Oen();
            MHal_GPIO_DDCDA_CK_High();

        break;
        case PAD_DDCDA_DA:
            MHal_GPIO_DDCDA_DA_Set();
            MHal_GPIO_DDCDA_DA_Oen();
            MHal_GPIO_DDCDA_DA_High();

        break;
        case PAD_DDCDB_CK:
            MHal_GPIO_DDCDB_CK_Set();
            MHal_GPIO_DDCDB_CK_Oen();
            MHal_GPIO_DDCDB_CK_High();

        break;
        case PAD_DDCDB_DA:
            MHal_GPIO_DDCDB_DA_Set();
            MHal_GPIO_DDCDB_DA_Oen();
            MHal_GPIO_DDCDB_DA_High();

        break;
        case PAD_DDCDC_CK:
            MHal_GPIO_DDCDC_CK_Set();
            MHal_GPIO_DDCDC_CK_Oen();
            MHal_GPIO_DDCDC_CK_High();

        break;
        case PAD_DDCDC_DA:
            MHal_GPIO_DDCDC_DA_Set();
            MHal_GPIO_DDCDC_DA_Oen();
            MHal_GPIO_DDCDC_DA_High();

        break;
        case PAD_DDCDD_CK:
            MHal_GPIO_DDCDD_CK_Set();
            MHal_GPIO_DDCDD_CK_Oen();
            MHal_GPIO_DDCDD_CK_High();

        break;
        case PAD_DDCDD_DA:
            MHal_GPIO_DDCDD_DA_Set();
            MHal_GPIO_DDCDD_DA_Oen();
            MHal_GPIO_DDCDD_DA_High();

        break;
        case PAD_SAR0:
            MHal_GPIO_SAR0_Set();
            MHal_GPIO_SAR0_Oen();
            MHal_GPIO_SAR0_High();

        break;
        case PAD_SAR1:
            MHal_GPIO_SAR1_Set();
            MHal_GPIO_SAR1_Oen();
            MHal_GPIO_SAR1_High();

        break;
        case PAD_SAR2:
            MHal_GPIO_SAR2_Set();
            MHal_GPIO_SAR2_Oen();
            MHal_GPIO_SAR2_High();

        break;
        case PAD_SAR3:
            MHal_GPIO_SAR3_Set();
            MHal_GPIO_SAR3_Oen();
            MHal_GPIO_SAR3_High();

        break;
        case PAD_SAR4:
            MHal_GPIO_SAR4_Set();
            MHal_GPIO_SAR4_Oen();
            MHal_GPIO_SAR4_High();

        break;
        case PAD_GPIO0:
            MHal_GPIO_GPIO0_Set();
            MHal_GPIO_GPIO0_Oen();
            MHal_GPIO_GPIO0_High();

        break;
        case PAD_GPIO1:
            MHal_GPIO_GPIO1_Set();
            MHal_GPIO_GPIO1_Oen();
            MHal_GPIO_GPIO1_High();

        break;
        case PAD_GPIO2:
            MHal_GPIO_GPIO2_Set();
            MHal_GPIO_GPIO2_Oen();
            MHal_GPIO_GPIO2_High();

        break;
        case PAD_GPIO3:
            MHal_GPIO_GPIO3_Set();
            MHal_GPIO_GPIO3_Oen();
            MHal_GPIO_GPIO3_High();

        break;
        case PAD_GPIO4:
            MHal_GPIO_GPIO4_Set();
            MHal_GPIO_GPIO4_Oen();
            MHal_GPIO_GPIO4_High();

        break;
        case PAD_GPIO5:
            MHal_GPIO_GPIO5_Set();
            MHal_GPIO_GPIO5_Oen();
            MHal_GPIO_GPIO5_High();

        break;
        case PAD_GPIO6:
            MHal_GPIO_GPIO6_Set();
            MHal_GPIO_GPIO6_Oen();
            MHal_GPIO_GPIO6_High();

        break;
        case PAD_GPIO7:
            MHal_GPIO_GPIO7_Set();
            MHal_GPIO_GPIO7_Oen();
            MHal_GPIO_GPIO7_High();

        break;
        case PAD_GPIO8:
            MHal_GPIO_GPIO8_Set();
            MHal_GPIO_GPIO8_Oen();
            MHal_GPIO_GPIO8_High();

        break;
        case PAD_GPIO9:
            MHal_GPIO_GPIO9_Set();
            MHal_GPIO_GPIO9_Oen();
            MHal_GPIO_GPIO9_High();

        break;
        case PAD_GPIO10:
            MHal_GPIO_GPIO10_Set();
            MHal_GPIO_GPIO10_Oen();
            MHal_GPIO_GPIO10_High();

        break;
        case PAD_GPIO11:
            MHal_GPIO_GPIO11_Set();
            MHal_GPIO_GPIO11_Oen();
            MHal_GPIO_GPIO11_High();

        break;
        case PAD_GPIO12:
            MHal_GPIO_GPIO12_Set();
            MHal_GPIO_GPIO12_Oen();
            MHal_GPIO_GPIO12_High();

        break;
        case PAD_GPIO13:
            MHal_GPIO_GPIO13_Set();
            MHal_GPIO_GPIO13_Oen();
            MHal_GPIO_GPIO13_High();

        break;
        case PAD_GPIO14:
            MHal_GPIO_GPIO14_Set();
            MHal_GPIO_GPIO14_Oen();
            MHal_GPIO_GPIO14_High();

        break;
        case PAD_GPIO15:
            MHal_GPIO_GPIO15_Set();
            MHal_GPIO_GPIO15_Oen();
            MHal_GPIO_GPIO15_High();

        break;
        case PAD_GPIO16:
            MHal_GPIO_GPIO16_Set();
            MHal_GPIO_GPIO16_Oen();
            MHal_GPIO_GPIO16_High();

        break;
        case PAD_GPIO17:
            MHal_GPIO_GPIO17_Set();
            MHal_GPIO_GPIO17_Oen();
            MHal_GPIO_GPIO17_High();

        break;
        case PAD_GPIO18:
            MHal_GPIO_GPIO18_Set();
            MHal_GPIO_GPIO18_Oen();
            MHal_GPIO_GPIO18_High();

        break;
        case PAD_GPIO19:
            MHal_GPIO_GPIO19_Set();
            MHal_GPIO_GPIO19_Oen();
            MHal_GPIO_GPIO19_High();

        break;
        case PAD_GPIO20:
            MHal_GPIO_GPIO20_Set();
            MHal_GPIO_GPIO20_Oen();
            MHal_GPIO_GPIO20_High();

        break;
        case PAD_GPIO21:
            MHal_GPIO_GPIO21_Set();
            MHal_GPIO_GPIO21_Oen();
            MHal_GPIO_GPIO21_High();

        break;
        case PAD_GPIO22:
            MHal_GPIO_GPIO22_Set();
            MHal_GPIO_GPIO22_Oen();
            MHal_GPIO_GPIO22_High();

        break;
        case PAD_GPIO23:
            MHal_GPIO_GPIO23_Set();
            MHal_GPIO_GPIO23_Oen();
            MHal_GPIO_GPIO23_High();

        break;
        case PAD_GPIO24:
            MHal_GPIO_GPIO24_Set();
            MHal_GPIO_GPIO24_Oen();
            MHal_GPIO_GPIO24_High();

        break;
        case PAD_GPIO25:
            MHal_GPIO_GPIO25_Set();
            MHal_GPIO_GPIO25_Oen();
            MHal_GPIO_GPIO25_High();

        break;
        case PAD_GPIO26:
            MHal_GPIO_GPIO26_Set();
            MHal_GPIO_GPIO26_Oen();
            MHal_GPIO_GPIO26_High();

        break;
        case PAD_GPIO27:
            MHal_GPIO_GPIO27_Set();
            MHal_GPIO_GPIO27_Oen();
            MHal_GPIO_GPIO27_High();

        break;
        case PAD_UART_RX2:
            MHal_GPIO_UART_RX2_Set();
            MHal_GPIO_UART_RX2_Oen();
            MHal_GPIO_UART_RX2_High();

        break;
        case PAD_UART_TX2:
            MHal_GPIO_UART_TX2_Set();
            MHal_GPIO_UART_TX2_Oen();
            MHal_GPIO_UART_TX2_High();

        break;
        case PAD_PWM0:
            MHal_GPIO_PWM0_Set();
            MHal_GPIO_PWM0_Oen();
            MHal_GPIO_PWM0_High();

        break;
        case PAD_PWM1:
            MHal_GPIO_PWM1_Set();
            MHal_GPIO_PWM1_Oen();
            MHal_GPIO_PWM1_High();

        break;
        case PAD_PWM2:
            MHal_GPIO_PWM2_Set();
            MHal_GPIO_PWM2_Oen();
            MHal_GPIO_PWM2_High();

        break;
        case PAD_PWM3:
            MHal_GPIO_PWM3_Set();
            MHal_GPIO_PWM3_Oen();
            MHal_GPIO_PWM3_High();

        break;
        case PAD_PWM4:
            MHal_GPIO_PWM4_Set();
            MHal_GPIO_PWM4_Oen();
            MHal_GPIO_PWM4_High();

        break;
        case PAD_DDCR_DA:
            MHal_GPIO_DDCR_DA_Set();
            MHal_GPIO_DDCR_DA_Oen();
            MHal_GPIO_DDCR_DA_High();

        break;
        case PAD_DDCR_CK:
            MHal_GPIO_DDCR_CK_Set();
            MHal_GPIO_DDCR_CK_Oen();
            MHal_GPIO_DDCR_CK_Set();

        break;
        case PAD_TGPIO0:
            MHal_GPIO_TGPIO0_Set();
            MHal_GPIO_TGPIO0_Oen();
            MHal_GPIO_TGPIO0_High();

        break;
        case PAD_TGPIO1:
            MHal_GPIO_TGPIO1_Set();
            MHal_GPIO_TGPIO1_Oen();
            MHal_GPIO_TGPIO1_High();

        break;
        case PAD_TGPIO2:
            MHal_GPIO_TGPIO2_Set();
            MHal_GPIO_TGPIO2_Oen();
            MHal_GPIO_TGPIO2_High();

        break;
        case PAD_TGPIO3:
            MHal_GPIO_TGPIO3_Set();
            MHal_GPIO_TGPIO3_Oen();
            MHal_GPIO_TGPIO3_High();

        break;
        case PAD_TS0_D0:
            MHal_GPIO_TS0_D0_Set();
            MHal_GPIO_TS0_D0_Oen();
            MHal_GPIO_TS0_D0_High();

        break;
        case PAD_TS0_D1:
            MHal_GPIO_TS0_D1_Set();
            MHal_GPIO_TS0_D1_Oen();
            MHal_GPIO_TS0_D1_High();

        break;
        case PAD_TS0_D2:
            MHal_GPIO_TS0_D2_Set();
            MHal_GPIO_TS0_D2_Oen();
            MHal_GPIO_TS0_D2_High();

        break;
        case PAD_TS0_D3:
            MHal_GPIO_TS0_D3_Set();
            MHal_GPIO_TS0_D3_Oen();
            MHal_GPIO_TS0_D3_High();

        break;
        case PAD_TS0_D4:
            MHal_GPIO_TS0_D4_Set();
            MHal_GPIO_TS0_D4_Oen();
            MHal_GPIO_TS0_D4_High();

        break;

        case PAD_TS0_D5:
            MHal_GPIO_TS0_D5_Set();
            MHal_GPIO_TS0_D5_Oen();
            MHal_GPIO_TS0_D5_High();

        break;
        case PAD_TS0_D6:
            MHal_GPIO_TS0_D6_Set();
            MHal_GPIO_TS0_D6_Oen();
            MHal_GPIO_TS0_D6_High();

        break;
        case PAD_TS0_D7:
            MHal_GPIO_TS0_D7_Set();
            MHal_GPIO_TS0_D7_Oen();
            MHal_GPIO_TS0_D7_High();

        break;
        case PAD_TS0_VLD:
            MHal_GPIO_TS0_VLD_Set();
            MHal_GPIO_TS0_VLD_Oen();
            MHal_GPIO_TS0_VLD_High();

        break;
        case PAD_TS0_SYNC:
            MHal_GPIO_TS0_SYNC_Set();
            MHal_GPIO_TS0_SYNC_Oen();
            MHal_GPIO_TS0_SYNC_High();

        break;
        case PAD_TS0_CLK:
            MHal_GPIO_TS0_CLK_Set();
            MHal_GPIO_TS0_CLK_Oen();
            MHal_GPIO_TS0_CLK_High();

        break;
        case PAD_TS1_D0:
            MHal_GPIO_TS1_D0_Set();
            MHal_GPIO_TS1_D0_Oen();
            MHal_GPIO_TS1_D0_High();

        break;
        case PAD_TS1_D1:
            MHal_GPIO_TS1_D1_Set();
            MHal_GPIO_TS1_D1_Oen();
            MHal_GPIO_TS1_D1_High();

        break;
        case PAD_TS1_D2:
            MHal_GPIO_TS1_D2_Set();
            MHal_GPIO_TS1_D2_Oen();
            MHal_GPIO_TS1_D2_High();

        break;
        case PAD_TS1_D3:
            MHal_GPIO_TS1_D3_Set();
            MHal_GPIO_TS1_D3_Oen();
            MHal_GPIO_TS1_D3_High();

        break;
        case PAD_TS1_D4:
            MHal_GPIO_TS1_D4_Set();
            MHal_GPIO_TS1_D4_Oen();
            MHal_GPIO_TS1_D4_High();

        break;
        case PAD_TS1_D5:
            MHal_GPIO_TS1_D5_Set();
            MHal_GPIO_TS1_D5_Oen();
            MHal_GPIO_TS1_D5_High();

        break;
        case PAD_TS1_D6:
            MHal_GPIO_TS1_D6_Set();
            MHal_GPIO_TS1_D6_Oen();
            MHal_GPIO_TS1_D6_High();

        break;
        case PAD_TS1_D7:
            MHal_GPIO_TS1_D7_Set();
            MHal_GPIO_TS1_D7_Oen();
            MHal_GPIO_TS1_D7_High();

        break;
        case PAD_TS1_VLD:
            MHal_GPIO_TS1_VLD_Set();
            MHal_GPIO_TS1_VLD_Oen();
            MHal_GPIO_TS1_VLD_High();

        break;
        case PAD_TS1_SYNC:
            MHal_GPIO_TS1_SYNC_Set();
            MHal_GPIO_TS1_SYNC_Oen();
            MHal_GPIO_TS1_SYNC_High();

        break;
        case PAD_TS1_CLK:
            MHal_GPIO_TS1_CLK_Set();
            MHal_GPIO_TS1_CLK_Oen();
            MHal_GPIO_TS1_CLK_High();

        break;
        case PAD_PCM_A4:
            MHal_GPIO_PCM_A4_Set();
            MHal_GPIO_PCM_A4_Oen();
            MHal_GPIO_PCM_A4_High();

        break;
        case PAD_PCM_WAIT_N:
            MHal_GPIO_PCM_WAIT_N_Set();
            MHal_GPIO_PCM_WAIT_N_Oen();
            MHal_GPIO_PCM_WAIT_N_High();

        break;
        case PAD_PCM_A5:
            MHal_GPIO_PCM_A5_Set();
            MHal_GPIO_PCM_A5_Oen();
            MHal_GPIO_PCM_A5_High();

        break;
        case PAD_PCM_A6:
            MHal_GPIO_PCM_A6_Set();
            MHal_GPIO_PCM_A6_Oen();
            MHal_GPIO_PCM_A6_High();

        break;
        case PAD_PCM_A7:
            MHal_GPIO_PCM_A7_Set();
            MHal_GPIO_PCM_A7_Oen();
            MHal_GPIO_PCM_A7_High();

        break;
        case PAD_PCM_A12:
            MHal_GPIO_PCM_A12_Set();
            MHal_GPIO_PCM_A12_Oen();
            MHal_GPIO_PCM_A12_High();

        break;
        case PAD_PCM_IRQA_N:
            MHal_GPIO_PCM_IRQA_N_Set();
            MHal_GPIO_PCM_IRQA_N_Oen();
            MHal_GPIO_PCM_IRQA_N_High();

        break;
        case PAD_PCM_A14:
            MHal_GPIO_PCM_A14_Set();
            MHal_GPIO_PCM_A14_Oen();
            MHal_GPIO_PCM_A14_High();

        break;
        case PAD_PCM_A13:
            MHal_GPIO_PCM_A13_Set();
            MHal_GPIO_PCM_A13_Oen();
            MHal_GPIO_PCM_A13_High();

        break;
        case PAD_PCM_A8:
            MHal_GPIO_PCM_A8_Set();
            MHal_GPIO_PCM_A8_Oen();
            MHal_GPIO_PCM_A8_High();

        break;
        case PAD_PCM_IOWR_N:
            MHal_GPIO_PCM_IOWR_N_Set();
            MHal_GPIO_PCM_IOWR_N_Oen();
            MHal_GPIO_PCM_IOWR_N_High();

        break;
        case PAD_PCM_A9:
            MHal_GPIO_PCM_A9_Set();
            MHal_GPIO_PCM_A9_Oen();
            MHal_GPIO_PCM_A9_High();

        break;
        case PAD_PCM_IORD_N:
            MHal_GPIO_PCM_IORD_N_Set();
            MHal_GPIO_PCM_IORD_N_Oen();
            MHal_GPIO_PCM_IORD_N_High();

        break;
        case PAD_PCM_A11:
            MHal_GPIO_PCM_A11_Set();
            MHal_GPIO_PCM_A11_Oen();
            MHal_GPIO_PCM_A11_High();

        break;
        case PAD_PCM_OE_N:
            MHal_GPIO_PCM_OE_N_Set();
            MHal_GPIO_PCM_OE_N_Oen();
            MHal_GPIO_PCM_OE_N_High();

        break;
        case PAD_PCM_A10:
            MHal_GPIO_PCM_A10_Set();
            MHal_GPIO_PCM_A10_Oen();
            MHal_GPIO_PCM_A10_High();

        break;
        case PAD_PCM_CE_N:
            MHal_GPIO_PCM_CE_N_Set();
            MHal_GPIO_PCM_CE_N_Oen();
            MHal_GPIO_PCM_CE_N_High();

        break;
        case PAD_PCM_D7:
            MHal_GPIO_PCM_D7_Set();
            MHal_GPIO_PCM_D7_Oen();
            MHal_GPIO_PCM_D7_High();

        break;
        case PAD_PCM_D6:
            MHal_GPIO_PCM_D6_Set();
            MHal_GPIO_PCM_D6_Oen();
            MHal_GPIO_PCM_D6_High();

        break;
        case PAD_PCM_D5:
            MHal_GPIO_PCM_D5_Set();
            MHal_GPIO_PCM_D5_Oen();
            MHal_GPIO_PCM_D5_High();

        break;
        case PAD_PCM_D4:
            MHal_GPIO_PCM_D4_Set();
            MHal_GPIO_PCM_D4_Oen();
            MHal_GPIO_PCM_D4_High();

        break;
        case PAD_PCM_D3:
            MHal_GPIO_PCM_D3_Set();
            MHal_GPIO_PCM_D3_Oen();
            MHal_GPIO_PCM_D3_High();

        break;
        case PAD_PCM_A3:
            MHal_GPIO_PCM_A3_Set();
            MHal_GPIO_PCM_A3_Oen();
            MHal_GPIO_PCM_A3_High();

        break;
        case PAD_PCM_A2:
            MHal_GPIO_PCM_A2_Set();
            MHal_GPIO_PCM_A2_Oen();
            MHal_GPIO_PCM_A2_High();

        break;
        case PAD_PCM_REG_N:
            MHal_GPIO_PCM_REG_N_Set();
            MHal_GPIO_PCM_REG_N_Oen();
            MHal_GPIO_PCM_REG_N_High();

        break;
        case PAD_PCM_A1:
            MHal_GPIO_PCM_A1_Set();
            MHal_GPIO_PCM_A1_Oen();
            MHal_GPIO_PCM_A1_High();

        break;
        case PAD_PCM_A0:
            MHal_GPIO_PCM_A0_Set();
            MHal_GPIO_PCM_A0_Oen();
            MHal_GPIO_PCM_A0_High();

        break;
        case PAD_PCM_D0:
            MHal_GPIO_PCM_D0_Set();
            MHal_GPIO_PCM_D0_Oen();
            MHal_GPIO_PCM_D0_High();

        break;
        case PAD_PCM_D1:
            MHal_GPIO_PCM_D1_Set();
            MHal_GPIO_PCM_D1_Oen();
            MHal_GPIO_PCM_D1_High();

        break;
        case PAD_PCM_D2:
            MHal_GPIO_PCM_D2_Set();
            MHal_GPIO_PCM_D2_Oen();
            MHal_GPIO_PCM_D2_High();

        break;
        case PAD_PCM_RESET:
            MHal_GPIO_PCM_RESET_Set();
            MHal_GPIO_PCM_RESET_Oen();
            MHal_GPIO_PCM_RESET_High();

        break;
        case PAD_PCM_CD_N:
            MHal_GPIO_PCM_CD_N_Set();
            MHal_GPIO_PCM_CD_N_Oen();
            MHal_GPIO_PCM_CD_N_High();

        break;
        case PAD_PCM2_CE_N:
            MHal_GPIO_PCM2_CE_N_Set();
            MHal_GPIO_PCM2_CE_N_Oen();
            MHal_GPIO_PCM2_CE_N_High();

        break;
        case PAD_PCM2_IRQA_N:
            MHal_GPIO_PCM2_IRQA_N_Set();
            MHal_GPIO_PCM2_IRQA_N_Oen();
            MHal_GPIO_PCM2_IRQA_N_High();

        break;
        case PAD_PCM2_WAIT_N:
            MHal_GPIO_PCM2_WAIT_N_Set();
            MHal_GPIO_PCM2_WAIT_N_Oen();
            MHal_GPIO_PCM2_WAIT_N_High();

        break;
        case PAD_PCM2_RESET:
            MHal_GPIO_PCM2_RESET_Set();
            MHal_GPIO_PCM2_RESET_Oen();
            MHal_GPIO_PCM2_RESET_High();

        break;
        case PAD_PCM2_CD_N:
            MHal_GPIO_PCM2_CD_N_Set();
            MHal_GPIO_PCM2_CD_N_Oen();
            MHal_GPIO_PCM2_CD_N_Set();

        break;
        case PAD_PF_AD15:
            MHal_GPIO_PF_AD15_Set();
            MHal_GPIO_PF_AD15_Oen();
            MHal_GPIO_PF_AD15_High();

        break;
        case PAD_PF_CE0Z:
            MHal_GPIO_PF_CE0Z_Set();
            MHal_GPIO_PF_CE0Z_Oen();
            MHal_GPIO_PF_CE0Z_High();

        break;
        case PAD_PF_CE1Z:
            MHal_GPIO_PF_CE1Z_Set();
            MHal_GPIO_PF_CE1Z_Oen();
            MHal_GPIO_PF_CE1Z_High();

        break;
        case PAD_PF_OEZ:
            MHal_GPIO_PF_OEZ_Set();
            MHal_GPIO_PF_OEZ_Oen();
            MHal_GPIO_PF_OEZ_High();

        break;
        case PAD_PF_WEZ:
            MHal_GPIO_PF_WEZ_Set();
            MHal_GPIO_PF_WEZ_Oen();
            MHal_GPIO_PF_WEZ_High();

        break;
        case PAD_PF_ALE:
            MHal_GPIO_PF_ALE_Set();
            MHal_GPIO_PF_ALE_Oen();
            MHal_GPIO_PF_ALE_High();

        break;
        case PAD_F_RBZ:
            MHal_GPIO_F_RBZ_Set();
            MHal_GPIO_F_RBZ_Oen();
            MHal_GPIO_F_RBZ_High();

        break;
        case PAD_TCON0:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO48_Set();
            		MHal_GPIO_GPIO48_Oen();
            		MHal_GPIO_GPIO48_High();
				}
				else
				{
					MHal_GPIO_TCON0_Set();
            		MHal_GPIO_TCON0_Oen();
            		MHal_GPIO_TCON0_High();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON0_Set();
            	MHal_GPIO_TCON0_Oen();
            	MHal_GPIO_TCON0_High();
			}
		}
#else
            MHal_GPIO_TCON0_Set();
            MHal_GPIO_TCON0_Oen();
            MHal_GPIO_TCON0_High();

#endif

        break;
        case PAD_TCON1:
            MHal_GPIO_TCON1_Set();
            MHal_GPIO_TCON1_Oen();
            MHal_GPIO_TCON1_High();

        break;
        case PAD_TCON2:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO49_Set();
            		MHal_GPIO_GPIO49_Oen();
            		MHal_GPIO_GPIO49_High();
				}
				else
				{
					MHal_GPIO_TCON2_Set();
            		MHal_GPIO_TCON2_Oen();
            		MHal_GPIO_TCON2_High();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON2_Set();
        		MHal_GPIO_TCON2_Oen();
        		MHal_GPIO_TCON2_High();
			}
		}
#else
            MHal_GPIO_TCON2_Set();
            MHal_GPIO_TCON2_Oen();
            MHal_GPIO_TCON2_High();

#endif

        break;
        case PAD_TCON3:
            MHal_GPIO_TCON3_Set();
            MHal_GPIO_TCON3_Oen();
            MHal_GPIO_TCON3_High();

        break;
        case PAD_TCON4:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO53_Set();
            		MHal_GPIO_GPIO53_Oen();
            		MHal_GPIO_GPIO53_High();
				}
				else
				{
					MHal_GPIO_TCON4_Set();
            		MHal_GPIO_TCON4_Oen();
            		MHal_GPIO_TCON4_High();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON4_Set();
        		MHal_GPIO_TCON4_Oen();
        		MHal_GPIO_TCON4_High();
			}
		}
#else
            MHal_GPIO_TCON4_Set();
            MHal_GPIO_TCON4_Oen();
            MHal_GPIO_TCON4_High();

#endif

        break;
        case PAD_TCON5:
            MHal_GPIO_TCON5_Set();
            MHal_GPIO_TCON5_Oen();
            MHal_GPIO_TCON5_High();

        break;
        case PAD_TCON6:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO52_Set();
            		MHal_GPIO_GPIO52_Oen();
            		MHal_GPIO_GPIO52_High();
				}
				else
				{
					MHal_GPIO_TCON6_Set();
            		MHal_GPIO_TCON6_Oen();
            		MHal_GPIO_TCON6_High();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON6_Set();
        		MHal_GPIO_TCON6_Oen();
        		MHal_GPIO_TCON6_High();
			}
		}
#else
            MHal_GPIO_TCON6_Set();
            MHal_GPIO_TCON6_Oen();
            MHal_GPIO_TCON6_High();
#endif

        break;
        case PAD_TCON7:
            MHal_GPIO_TCON7_Set();
            MHal_GPIO_TCON7_Oen();
            MHal_GPIO_TCON7_High();

        break;
        case PAD_TCON8:
			//balup_090907

#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO47_Set();
            		MHal_GPIO_GPIO47_Oen();
            		MHal_GPIO_GPIO47_High();
				}
				else
				{
					MHal_GPIO_TCON8_Set();
            		MHal_GPIO_TCON8_Oen();
            		MHal_GPIO_TCON8_High();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON8_Set();
        		MHal_GPIO_TCON8_Oen();
        		MHal_GPIO_TCON8_High();
			}
		}
#else
            MHal_GPIO_TCON8_Set();
            MHal_GPIO_TCON8_Oen();
            MHal_GPIO_TCON8_High();

#endif

        break;
        case PAD_TCON9:
            MHal_GPIO_TCON9_Set();
            MHal_GPIO_TCON9_Oen();
            MHal_GPIO_TCON9_High();

        break;
        case PAD_TCON10:
            MHal_GPIO_TCON10_Set();
            MHal_GPIO_TCON10_Oen();
            MHal_GPIO_TCON10_High();

        break;
        case PAD_TCON11:
            MHal_GPIO_TCON11_Set();
            MHal_GPIO_TCON11_Oen();
            MHal_GPIO_TCON11_High();

        break;
        case PAD_TCON12:
            MHal_GPIO_TCON12_Set();
            MHal_GPIO_TCON12_Oen();
            MHal_GPIO_TCON12_High();

        break;
        case PAD_TCON13:
            MHal_GPIO_TCON13_Set();
            MHal_GPIO_TCON13_Oen();
            MHal_GPIO_TCON13_High();

        break;
        case PAD_TCON14:
            MHal_GPIO_TCON14_Set();
             MHal_GPIO_TCON14_Oen();
            MHal_GPIO_TCON14_High();

        break;
        case PAD_TCON15:
            MHal_GPIO_TCON15_Set();
            MHal_GPIO_TCON15_Oen();
            MHal_GPIO_TCON15_High();

        break;
        case PAD_TCON16:
            MHal_GPIO_TCON16_Set();
                MHal_GPIO_TCON16_Oen();
            MHal_GPIO_TCON16_High();

        break;
        case PAD_TCON17:
            MHal_GPIO_TCON17_Set();
            MHal_GPIO_TCON17_Oen();
            MHal_GPIO_TCON17_High();

        break;
        case PAD_TCON18:
            MHal_GPIO_TCON18_Set();
            MHal_GPIO_TCON18_Oen();
            MHal_GPIO_TCON18_High();

        break;
        case PAD_TCON19:
            MHal_GPIO_TCON19_Set();
            MHal_GPIO_TCON19_Oen();
            MHal_GPIO_TCON19_High();

        break;
        case PAD_TCON20:
            MHal_GPIO_TCON20_Set();
            MHal_GPIO_TCON20_Oen();
            MHal_GPIO_TCON20_High();

        break;
        case PAD_TCON21:
            MHal_GPIO_TCON21_Set();
            MHal_GPIO_TCON21_Oen();
            MHal_GPIO_TCON21_High();

        break;
        case PAD_ET_COL:
            MHal_GPIO_ET_COL_Set();
            MHal_GPIO_ET_COL_Oen();
            MHal_GPIO_ET_COL_High();

        break;
        case PAD_ET_TXD1:
            MHal_GPIO_ET_TXD1_Set();
            MHal_GPIO_ET_TXD1_Oen();
            MHal_GPIO_ET_TXD1_High();

        break;
        case PAD_ET_TXD0:
            MHal_GPIO_ET_TXD0_Set();
            MHal_GPIO_ET_TXD0_Oen();
            MHal_GPIO_ET_TXD0_High();

        break;
        case PAD_ET_TX_EN:
            MHal_GPIO_ET_TX_EN_Set();
            MHal_GPIO_ET_TX_EN_Oen();
            MHal_GPIO_ET_TX_EN_High();

        break;
        case PAD_ET_TX_CLK:
            MHal_GPIO_ET_TX_CLK_Set();
            MHal_GPIO_ET_TX_CLK_Oen();
            MHal_GPIO_ET_TX_CLK_High();

        break;
        case PAD_ET_RXD0:
            MHal_GPIO_ET_RXD0_Set();
            MHal_GPIO_ET_RXD0_Oen();
            MHal_GPIO_ET_RXD0_High();

        break;
        case PAD_ET_RXD1:
            MHal_GPIO_ET_RXD1_Set();
            MHal_GPIO_ET_RXD1_Oen();
            MHal_GPIO_ET_RXD1_High();

        break;
        case PAD_ET_MDC:
            MHal_GPIO_ET_MDC_Set();
            MHal_GPIO_ET_MDC_Oen();
            MHal_GPIO_ET_MDC_High();

        break;
        case PAD_ET_EDIO:
            MHal_GPIO_ET_EDIO_Set();
            MHal_GPIO_ET_EDIO_Oen();
            MHal_GPIO_ET_EDIO_High();

        break;
        case PAD_I2S_IN_WS:
            MHal_GPIO_I2S_IN_WS_Set();
            MHal_GPIO_I2S_IN_WS_Oen();
            MHal_GPIO_I2S_IN_WS_High();

        break;
        case PAD_I2S_IN_BCK:
            MHal_GPIO_I2S_IN_BCK_Set();
            MHal_GPIO_I2S_IN_BCK_Oen();
            MHal_GPIO_I2S_IN_BCK_High();

        break;
        case PAD_I2S_IN_SD:
            MHal_GPIO_I2S_IN_SD_Set();
            MHal_GPIO_I2S_IN_SD_Oen();
            MHal_GPIO_I2S_IN_SD_High();

        break;
        case PAD_SPDIF_IN:
            MHal_GPIO_SPDIF_IN_Set();
            MHal_GPIO_SPDIF_IN_Oen();
            MHal_GPIO_SPDIF_IN_High();

        break;
        case PAD_SPDIF_OUT:
            MHal_GPIO_SPDIF_OUT_Set();
            MHal_GPIO_SPDIF_OUT_Oen();
            MHal_GPIO_SPDIF_OUT_High();

        break;
        case PAD_I2S_OUT_MCK:
            MHal_GPIO_I2S_OUT_MCK_Set();
            MHal_GPIO_I2S_OUT_MCK_Oen();
            MHal_GPIO_I2S_OUT_MCK_High();

        break;
        case PAD_I2S_OUT_WS:
            MHal_GPIO_I2S_OUT_WS_Set();
            MHal_GPIO_I2S_OUT_WS_Oen();
            MHal_GPIO_I2S_OUT_WS_High();

        break;
        case PAD_I2S_OUT_BCK:
            MHal_GPIO_I2S_OUT_BCK_Set();
            MHal_GPIO_I2S_OUT_BCK_Oen();
            MHal_GPIO_I2S_OUT_BCK_High();

        break;
        case PAD_I2S_OUT_SD:
            MHal_GPIO_I2S_OUT_SD_Set();
            MHal_GPIO_I2S_OUT_SD_Oen();
            MHal_GPIO_I2S_OUT_SD_High();

        break;
        case PAD_I2S_OUT_SD1:
            MHal_GPIO_I2S_OUT_SD1_Set();
            MHal_GPIO_I2S_OUT_SD1_Oen();
            MHal_GPIO_I2S_OUT_SD1_High();

        break;
        case PAD_I2S_OUT_SD2:
            MHal_GPIO_I2S_OUT_SD2_Set();
            MHal_GPIO_I2S_OUT_SD2_Oen();
            MHal_GPIO_I2S_OUT_SD2_High();

        break;
        case PAD_I2S_OUT_SD3:
            MHal_GPIO_I2S_OUT_SD3_Set();
            MHal_GPIO_I2S_OUT_SD3_Oen();
            MHal_GPIO_I2S_OUT_SD3_High();

        break;
        case PAD_B_ODD_0:
            MHal_GPIO_B_ODD_0_Set();
               MHal_GPIO_B_ODD_0_Oen();
            MHal_GPIO_B_ODD_0_High();

        break;
        case PAD_B_ODD_1:
            MHal_GPIO_B_ODD_1_Set();
            MHal_GPIO_B_ODD_1_Oen();
            MHal_GPIO_B_ODD_1_High();

        break;
        case PAD_B_ODD_2:
            MHal_GPIO_B_ODD_2_Set();
            MHal_GPIO_B_ODD_2_Oen();
            MHal_GPIO_B_ODD_2_High();

        break;
        case PAD_B_ODD_3:
            MHal_GPIO_B_ODD_3_Set();
            MHal_GPIO_B_ODD_3_Oen();
            MHal_GPIO_B_ODD_3_High();

        break;
        case PAD_B_ODD_4:
            MHal_GPIO_B_ODD_4_Set();
            MHal_GPIO_B_ODD_4_Oen();
            MHal_GPIO_B_ODD_4_High();

        break;
        case PAD_B_ODD_5:
            MHal_GPIO_B_ODD_5_Set();
            MHal_GPIO_B_ODD_5_Oen();
            MHal_GPIO_B_ODD_5_High();

        break;
        case PAD_B_ODD_6:
            MHal_GPIO_B_ODD_6_Set();
            MHal_GPIO_B_ODD_6_Oen();
            MHal_GPIO_B_ODD_6_High();

        break;
        case PAD_B_ODD_7:
            MHal_GPIO_B_ODD_7_Set();
            MHal_GPIO_B_ODD_7_Oen();
            MHal_GPIO_B_ODD_7_High();

        break;
        case PAD_G_ODD_0:
            MHal_GPIO_G_ODD_0_Set();
            MHal_GPIO_G_ODD_0_Oen();
            MHal_GPIO_G_ODD_0_High();

        break;
        case PAD_G_ODD_1:
            MHal_GPIO_G_ODD_1_Set();
            MHal_GPIO_G_ODD_1_Oen();
            MHal_GPIO_G_ODD_1_High();

        break;
        case PAD_G_ODD_2:
            MHal_GPIO_G_ODD_2_Set();
            MHal_GPIO_G_ODD_2_Oen();
            MHal_GPIO_G_ODD_2_High();

        break;
        case PAD_G_ODD_3:
            MHal_GPIO_G_ODD_3_Set();
            MHal_GPIO_G_ODD_3_Oen();
            MHal_GPIO_G_ODD_3_High();

        break;
        case PAD_G_ODD_4:
            MHal_GPIO_G_ODD_4_Set();
            MHal_GPIO_G_ODD_4_Oen();
            MHal_GPIO_G_ODD_4_High();

        break;
        case PAD_G_ODD_5:
            MHal_GPIO_G_ODD_5_Set();
            MHal_GPIO_G_ODD_5_Oen();
            MHal_GPIO_G_ODD_5_High();

        break;
        case PAD_G_ODD_6:
            MHal_GPIO_G_ODD_6_Set();
            MHal_GPIO_G_ODD_6_Oen();
            MHal_GPIO_G_ODD_6_High();

        break;
        case PAD_G_ODD_7:
            MHal_GPIO_G_ODD_7_Set();
            MHal_GPIO_G_ODD_7_Oen();
            MHal_GPIO_G_ODD_7_High();

        break;
        case PAD_R_ODD_0:
            MHal_GPIO_R_ODD_0_Set();
            MHal_GPIO_R_ODD_0_Oen();
            MHal_GPIO_R_ODD_0_High();

        break;
        case PAD_R_ODD_1:
            MHal_GPIO_R_ODD_1_Set();
            MHal_GPIO_R_ODD_1_Oen();
            MHal_GPIO_R_ODD_1_High();

        break;
        case PAD_R_ODD_2:
            MHal_GPIO_R_ODD_2_Set();
            MHal_GPIO_R_ODD_2_Oen();
            MHal_GPIO_R_ODD_2_High();

        break;
        case PAD_R_ODD_3:
            MHal_GPIO_R_ODD_3_Set();
            MHal_GPIO_R_ODD_3_Oen();
            MHal_GPIO_R_ODD_3_High();

        break;
        case PAD_R_ODD_4:
            MHal_GPIO_R_ODD_4_Set();
            MHal_GPIO_R_ODD_4_Oen();
            MHal_GPIO_R_ODD_4_High();

        break;
        case PAD_R_ODD_5:
            MHal_GPIO_R_ODD_5_Set();
            MHal_GPIO_R_ODD_5_Oen();
            MHal_GPIO_R_ODD_5_High();

        break;
        case PAD_R_ODD_6:
            MHal_GPIO_R_ODD_6_Set();
            MHal_GPIO_R_ODD_6_Oen();
            MHal_GPIO_R_ODD_6_High();

        break;
        case PAD_R_ODD_7:
            MHal_GPIO_R_ODD_7_Set();
            MHal_GPIO_R_ODD_7_Oen();
            MHal_GPIO_R_ODD_7_High();

        break;
        case PAD_mini_LVDS_0:
            MHal_GPIO_mini_LVDS_0_Set();
            MHal_GPIO_mini_LVDS_0_Oen();
            MHal_GPIO_mini_LVDS_0_High();

        break;
        case PAD_mini_LVDS_1:
            MHal_GPIO_mini_LVDS_1_Set();
            MHal_GPIO_mini_LVDS_1_Oen();
            MHal_GPIO_mini_LVDS_1_High();

        break;
        case PAD_mini_LVDS_2:
            MHal_GPIO_mini_LVDS_2_Set();
            MHal_GPIO_mini_LVDS_2_Oen();
            MHal_GPIO_mini_LVDS_2_High();

        break;
        case PAD_mini_LVDS_3:
            MHal_GPIO_mini_LVDS_3_Set();
            MHal_GPIO_mini_LVDS_3_Oen();
            MHal_GPIO_mini_LVDS_3_High();

        break;
        case PAD_mini_LVDS_4:
            MHal_GPIO_mini_LVDS_4_Set();
            MHal_GPIO_mini_LVDS_4_Oen();
            MHal_GPIO_mini_LVDS_4_High();

        break;
        case PAD_mini_LVDS_5:
            MHal_GPIO_mini_LVDS_5_Set();
            MHal_GPIO_mini_LVDS_5_Oen();
            MHal_GPIO_mini_LVDS_5_High();

        break;
        case PAD_mini_LVDS_6:
            MHal_GPIO_mini_LVDS_6_Set();
            MHal_GPIO_mini_LVDS_6_Oen();
            MHal_GPIO_mini_LVDS_6_High();

        break;
        case PAD_mini_LVDS_7:
            MHal_GPIO_mini_LVDS_7_Set();
            MHal_GPIO_mini_LVDS_7_Oen();
            MHal_GPIO_mini_LVDS_7_High();

        break;
        case PAD_LCK:
            MHal_GPIO_LCK_Set();
            MHal_GPIO_LCK_Oen();
            MHal_GPIO_LCK_High();

        break;
        case PAD_LDE:
            MHal_GPIO_LDE_Set();
            MHal_GPIO_LDE_Oen();
            MHal_GPIO_LDE_High();

        break;
        case PAD_LHSYNC:
            MHal_GPIO_LHSYNC_Set();
            MHal_GPIO_LHSYNC_Oen();
            MHal_GPIO_LHSYNC_High();

        break;
        case PAD_LVSYNC:
            MHal_GPIO_LVSYNC_Set();
            MHal_GPIO_LVSYNC_Oen();
            MHal_GPIO_LVSYNC_High();

        break;
        case PAD_PCM_WE_N:
            MHal_GPIO_PCM_WE_N_Set();
            MHal_GPIO_PCM_WE_N_Oen();
            MHal_GPIO_PCM_WE_N_High();

        break;



        default:
            break;
    }
}
EXPORT_SYMBOL(MDrv_GPIO_Set_High);

//-------------------------------------------------------------------------------------------------
/// output pull low for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Set_Low(U8 u8IndexGPIO)
{
    switch(u8IndexGPIO)
    {
        case PAD_PM_SPI_CZ:
            MHal_GPIO_PM_SPI_CZ_Set();
            MHal_GPIO_PM_SPI_CZ_Oen();
            MHal_GPIO_PM_SPI_CZ_Low();
        break;
        case PAD_PM_SPI_CK:
            MHal_GPIO_PM_SPI_CK_Set();
            MHal_GPIO_PM_SPI_CK_Oen();
            MHal_GPIO_PM_SPI_CK_Low();

        break;
        case PAD_PM_SPI_DI:
            MHal_GPIO_PM_SPI_DI_Set();
            MHal_GPIO_PM_SPI_DI_Oen();
            MHal_GPIO_PM_SPI_DI_Low();

        break;
        case PAD_PM_SPI_DO:
            MHal_GPIO_PM_SPI_DO_Set();
            MHal_GPIO_PM_SPI_DO_Oen();
            MHal_GPIO_PM_SPI_DO_Low();

        break;
        case PAD_IRIN:
            MHal_GPIO_IRIN_Set();
            MHal_GPIO_IRIN_Oen();
            MHal_GPIO_IRIN_Low();
        break;
        case PAD_CEC:
            MHal_GPIO_CEC_Set();
            MHal_GPIO_CEC_Oen();
            MHal_GPIO_CEC_Low();
        break;
        case PAD_GPIO_PM0:
            MHal_GPIO_GPIO_PM0_Set();
            MHal_GPIO_GPIO_PM0_Oen();
            MHal_GPIO_GPIO_PM0_Low();
        break;
        case PAD_GPIO_PM1:
            MHal_GPIO_GPIO_PM1_Set();
            MHal_GPIO_GPIO_PM1_Oen();
            MHal_GPIO_GPIO_PM1_Low();
        break;
        case PAD_GPIO_PM2:
            MHal_GPIO_GPIO_PM2_Set();
            MHal_GPIO_GPIO_PM2_Oen();
            MHal_GPIO_GPIO_PM2_Low();
        break;
        case PAD_GPIO_PM3:
            MHal_GPIO_GPIO_PM3_Set();
            MHal_GPIO_GPIO_PM3_Oen();
            MHal_GPIO_GPIO_PM3_Low();

        break;
        case PAD_GPIO_PM4:
            MHal_GPIO_GPIO_PM4_Set();
            MHal_GPIO_GPIO_PM4_Oen();
            MHal_GPIO_GPIO_PM4_Low();

        break;
        case PAD_GPIO_PM5:
            MHal_GPIO_GPIO_PM5_Set();
            MHal_GPIO_GPIO_PM5_Oen();
            MHal_GPIO_GPIO_PM5_Low();

        break;
        case PAD_GPIO_PM6:
            MHal_GPIO_GPIO_PM6_Set();
            MHal_GPIO_GPIO_PM6_Oen();
            MHal_GPIO_GPIO_PM6_Low();

        break;
        case PAD_GPIO_PM7:
            MHal_GPIO_GPIO_PM7_Set();
            MHal_GPIO_GPIO_PM7_Oen();
            MHal_GPIO_GPIO_PM7_Low();


        break;
        case PAD_GPIO_PM8:
            MHal_GPIO_GPIO_PM8_Set();
            MHal_GPIO_GPIO_PM8_Oen();
            MHal_GPIO_GPIO_PM8_Low();

        break;
        case PAD_GPIO_PM9:
            MHal_GPIO_GPIO_PM9_Set();
            MHal_GPIO_GPIO_PM9_Oen();
            MHal_GPIO_GPIO_PM9_Low();

        break;
        case PAD_GPIO_PM10:
            MHal_GPIO_GPIO_PM10_Set();
            MHal_GPIO_GPIO_PM10_Oen();
            MHal_GPIO_GPIO_PM10_Low();

        break;
        case PAD_GPIO_PM11:
            MHal_GPIO_GPIO_PM11_Set();
            MHal_GPIO_GPIO_PM11_Oen();
            MHal_GPIO_GPIO_PM11_Low();

        break;
        case PAD_GPIO_PM12:
            MHal_GPIO_GPIO_PM12_Set();
            MHal_GPIO_GPIO_PM12_Oen();
            MHal_GPIO_GPIO_PM12_Low();

        break;
        case PAD_HOTPLUGA:
            MHal_GPIO_HOTPLUGA_Set();
            MHal_GPIO_HOTPLUGA_Oen();
            MHal_GPIO_HOTPLUGA_Low();

        break;
        case PAD_HOTPLUGB:
            MHal_GPIO_HOTPLUGB_Set();
            MHal_GPIO_HOTPLUGB_Oen();
            MHal_GPIO_HOTPLUGB_Low();

        break;
        case PAD_HOTPLUGC:
            MHal_GPIO_HOTPLUGC_Set();
            MHal_GPIO_HOTPLUGC_Oen();
            MHal_GPIO_HOTPLUGC_Low();

        break;
        case PAD_HOTPLUGD:
            MHal_GPIO_HOTPLUGD_Set();
            MHal_GPIO_HOTPLUGD_Oen();
            MHal_GPIO_HOTPLUGD_Low();

        break;
        case PAD_DDCDA_CK:
            MHal_GPIO_DDCDA_CK_Set();
            MHal_GPIO_DDCDA_CK_Oen();
            MHal_GPIO_DDCDA_CK_Low();

        break;
        case PAD_DDCDA_DA:
            MHal_GPIO_DDCDA_DA_Set();
            MHal_GPIO_DDCDA_DA_Oen();
            MHal_GPIO_DDCDA_DA_Low();

        break;
        case PAD_DDCDB_CK:
            MHal_GPIO_DDCDB_CK_Set();
            MHal_GPIO_DDCDB_CK_Oen();
            MHal_GPIO_DDCDB_CK_Low();

        break;
        case PAD_DDCDB_DA:
            MHal_GPIO_DDCDB_DA_Set();
            MHal_GPIO_DDCDB_DA_Oen();
            MHal_GPIO_DDCDB_DA_Low();

        break;
        case PAD_DDCDC_CK:
            MHal_GPIO_DDCDC_CK_Set();
            MHal_GPIO_DDCDC_CK_Oen();
            MHal_GPIO_DDCDC_CK_Low();

        break;
        case PAD_DDCDC_DA:
            MHal_GPIO_DDCDC_DA_Set();
            MHal_GPIO_DDCDC_DA_Oen();
            MHal_GPIO_DDCDC_DA_Low();

        break;
        case PAD_DDCDD_CK:
            MHal_GPIO_DDCDD_CK_Set();
            MHal_GPIO_DDCDD_CK_Oen();
            MHal_GPIO_DDCDD_CK_Low();

        break;
        case PAD_DDCDD_DA:
            MHal_GPIO_DDCDD_DA_Set();
            MHal_GPIO_DDCDD_DA_Oen();
            MHal_GPIO_DDCDD_DA_Low();

        break;
        case PAD_SAR0:
            MHal_GPIO_SAR0_Set();
            MHal_GPIO_SAR0_Oen();
            MHal_GPIO_SAR0_Low();

        break;
        case PAD_SAR1:
            MHal_GPIO_SAR1_Set();
            MHal_GPIO_SAR1_Oen();
            MHal_GPIO_SAR1_Low();

        break;
        case PAD_SAR2:
            MHal_GPIO_SAR2_Set();
            MHal_GPIO_SAR2_Oen();
            MHal_GPIO_SAR2_Low();

        break;
        case PAD_SAR3:
            MHal_GPIO_SAR3_Set();
            MHal_GPIO_SAR3_Oen();
            MHal_GPIO_SAR3_Low();

        break;
        case PAD_SAR4:
            MHal_GPIO_SAR4_Set();
            MHal_GPIO_SAR4_Oen();
            MHal_GPIO_SAR4_Low();

        break;
        case PAD_GPIO0:
            MHal_GPIO_GPIO0_Set();
            MHal_GPIO_GPIO0_Oen();
            MHal_GPIO_GPIO0_Low();

        break;
        case PAD_GPIO1:
            MHal_GPIO_GPIO1_Set();
            MHal_GPIO_GPIO1_Oen();
            MHal_GPIO_GPIO1_Low();

        break;
        case PAD_GPIO2:
            MHal_GPIO_GPIO2_Set();
            MHal_GPIO_GPIO2_Oen();
            MHal_GPIO_GPIO2_Low();

        break;
        case PAD_GPIO3:
            MHal_GPIO_GPIO3_Set();
            MHal_GPIO_GPIO3_Oen();
            MHal_GPIO_GPIO3_Low();

        break;
        case PAD_GPIO4:
            MHal_GPIO_GPIO4_Set();
            MHal_GPIO_GPIO4_Oen();
            MHal_GPIO_GPIO4_Low();

        break;
        case PAD_GPIO5:
            MHal_GPIO_GPIO5_Set();
            MHal_GPIO_GPIO5_Oen();
            MHal_GPIO_GPIO5_Low();

        break;
        case PAD_GPIO6:
            MHal_GPIO_GPIO6_Set();
            MHal_GPIO_GPIO6_Oen();
            MHal_GPIO_GPIO6_Low();

        break;
        case PAD_GPIO7:
            MHal_GPIO_GPIO7_Set();
            MHal_GPIO_GPIO7_Oen();
            MHal_GPIO_GPIO7_Low();

        break;
        case PAD_GPIO8:
            MHal_GPIO_GPIO8_Set();
            MHal_GPIO_GPIO8_Oen();
            MHal_GPIO_GPIO8_Low();

        break;
        case PAD_GPIO9:
            MHal_GPIO_GPIO9_Set();
            MHal_GPIO_GPIO9_Oen();
            MHal_GPIO_GPIO9_Low();

        break;
        case PAD_GPIO10:
            MHal_GPIO_GPIO10_Set();
            MHal_GPIO_GPIO10_Oen();
            MHal_GPIO_GPIO10_Low();

        break;
        case PAD_GPIO11:
            MHal_GPIO_GPIO11_Set();
            MHal_GPIO_GPIO11_Oen();
            MHal_GPIO_GPIO11_Low();

        break;
        case PAD_GPIO12:
            MHal_GPIO_GPIO12_Set();
            MHal_GPIO_GPIO12_Oen();
            MHal_GPIO_GPIO12_Low();

        break;
        case PAD_GPIO13:
            MHal_GPIO_GPIO13_Set();
            MHal_GPIO_GPIO13_Oen();
            MHal_GPIO_GPIO13_Low();

        break;
        case PAD_GPIO14:
            MHal_GPIO_GPIO14_Set();
            MHal_GPIO_GPIO14_Oen();
            MHal_GPIO_GPIO14_Low();

        break;
        case PAD_GPIO15:
            MHal_GPIO_GPIO15_Set();
            MHal_GPIO_GPIO15_Oen();
            MHal_GPIO_GPIO15_Low();

        break;
        case PAD_GPIO16:
            MHal_GPIO_GPIO16_Set();
            MHal_GPIO_GPIO16_Oen();
            MHal_GPIO_GPIO16_Low();

        break;
        case PAD_GPIO17:
            MHal_GPIO_GPIO17_Set();
            MHal_GPIO_GPIO17_Oen();
            MHal_GPIO_GPIO17_Low();

        break;
        case PAD_GPIO18:
            MHal_GPIO_GPIO18_Set();
            MHal_GPIO_GPIO18_Oen();
            MHal_GPIO_GPIO18_Low();

        break;
        case PAD_GPIO19:
            MHal_GPIO_GPIO19_Set();
            MHal_GPIO_GPIO19_Oen();
            MHal_GPIO_GPIO19_Low();

        break;
        case PAD_GPIO20:
            MHal_GPIO_GPIO20_Set();
            MHal_GPIO_GPIO20_Oen();
            MHal_GPIO_GPIO20_Low();

        break;
        case PAD_GPIO21:
            MHal_GPIO_GPIO21_Set();
            MHal_GPIO_GPIO21_Oen();
            MHal_GPIO_GPIO21_Low();

        break;
        case PAD_GPIO22:
            MHal_GPIO_GPIO22_Set();
            MHal_GPIO_GPIO22_Oen();
            MHal_GPIO_GPIO22_Low();

        break;
        case PAD_GPIO23:
            MHal_GPIO_GPIO23_Set();
            MHal_GPIO_GPIO23_Oen();
            MHal_GPIO_GPIO23_Low();

        break;
        case PAD_GPIO24:
            MHal_GPIO_GPIO24_Set();
            MHal_GPIO_GPIO24_Oen();
            MHal_GPIO_GPIO24_Low();

        break;
        case PAD_GPIO25:
            MHal_GPIO_GPIO25_Set();
            MHal_GPIO_GPIO25_Oen();
            MHal_GPIO_GPIO25_Low();

        break;
        case PAD_GPIO26:
            MHal_GPIO_GPIO26_Set();
            MHal_GPIO_GPIO26_Oen();
            MHal_GPIO_GPIO26_Low();

        break;
        case PAD_GPIO27:
            MHal_GPIO_GPIO27_Set();
            MHal_GPIO_GPIO27_Oen();
            MHal_GPIO_GPIO27_Low();

        break;
        case PAD_UART_RX2:
            MHal_GPIO_UART_RX2_Set();
            MHal_GPIO_UART_RX2_Oen();
            MHal_GPIO_UART_RX2_Low();

        break;
        case PAD_UART_TX2:
            MHal_GPIO_UART_TX2_Set();
            MHal_GPIO_UART_TX2_Oen();
            MHal_GPIO_UART_TX2_Low();

        break;
        case PAD_PWM0:
            MHal_GPIO_PWM0_Set();
            MHal_GPIO_PWM0_Oen();
            MHal_GPIO_PWM0_Low();

        break;
        case PAD_PWM1:
            MHal_GPIO_PWM1_Set();
            MHal_GPIO_PWM1_Oen();
            MHal_GPIO_PWM1_Low();

        break;
        case PAD_PWM2:
            MHal_GPIO_PWM2_Set();
            MHal_GPIO_PWM2_Oen();
            MHal_GPIO_PWM2_Low();

        break;
        case PAD_PWM3:
            MHal_GPIO_PWM3_Set();
            MHal_GPIO_PWM3_Oen();
            MHal_GPIO_PWM3_Low();

        break;
        case PAD_PWM4:
            MHal_GPIO_PWM4_Set();
            MHal_GPIO_PWM4_Oen();
            MHal_GPIO_PWM4_Low();

        break;
        case PAD_DDCR_DA:
            MHal_GPIO_DDCR_DA_Set();
            MHal_GPIO_DDCR_DA_Oen();
            MHal_GPIO_DDCR_DA_Low();

        break;
        case PAD_DDCR_CK:
            MHal_GPIO_DDCR_CK_Set();
            MHal_GPIO_DDCR_CK_Oen();
            MHal_GPIO_DDCR_CK_Set();

        break;
        case PAD_TGPIO0:
            MHal_GPIO_TGPIO0_Set();
            MHal_GPIO_TGPIO0_Oen();
            MHal_GPIO_TGPIO0_Low();

        break;
        case PAD_TGPIO1:
            MHal_GPIO_TGPIO1_Set();
            MHal_GPIO_TGPIO1_Oen();
            MHal_GPIO_TGPIO1_Low();

        break;
        case PAD_TGPIO2:
            MHal_GPIO_TGPIO2_Set();
            MHal_GPIO_TGPIO2_Oen();
            MHal_GPIO_TGPIO2_Low();

        break;
        case PAD_TGPIO3:
            MHal_GPIO_TGPIO3_Set();
            MHal_GPIO_TGPIO3_Oen();
            MHal_GPIO_TGPIO3_Low();

        break;
        case PAD_TS0_D0:
            MHal_GPIO_TS0_D0_Set();
            MHal_GPIO_TS0_D0_Oen();
            MHal_GPIO_TS0_D0_Low();

        break;
        case PAD_TS0_D1:
            MHal_GPIO_TS0_D1_Set();
            MHal_GPIO_TS0_D1_Oen();
            MHal_GPIO_TS0_D1_Low();

        break;
        case PAD_TS0_D2:
            MHal_GPIO_TS0_D2_Set();
            MHal_GPIO_TS0_D2_Oen();
            MHal_GPIO_TS0_D2_Low();

        break;
        case PAD_TS0_D3:
            MHal_GPIO_TS0_D3_Set();
            MHal_GPIO_TS0_D3_Oen();
            MHal_GPIO_TS0_D3_Low();

        break;
        case PAD_TS0_D4:
            MHal_GPIO_TS0_D4_Set();
            MHal_GPIO_TS0_D4_Oen();
            MHal_GPIO_TS0_D4_Low();

        break;

        case PAD_TS0_D5:
            MHal_GPIO_TS0_D5_Set();
            MHal_GPIO_TS0_D5_Oen();
            MHal_GPIO_TS0_D5_Low();

        break;
        case PAD_TS0_D6:
            MHal_GPIO_TS0_D6_Set();
            MHal_GPIO_TS0_D6_Oen();
            MHal_GPIO_TS0_D6_Low();

        break;
        case PAD_TS0_D7:
            MHal_GPIO_TS0_D7_Set();
            MHal_GPIO_TS0_D7_Oen();
            MHal_GPIO_TS0_D7_Low();

        break;
        case PAD_TS0_VLD:
            MHal_GPIO_TS0_VLD_Set();
            MHal_GPIO_TS0_VLD_Oen();
            MHal_GPIO_TS0_VLD_Low();

        break;
        case PAD_TS0_SYNC:
            MHal_GPIO_TS0_SYNC_Set();
            MHal_GPIO_TS0_SYNC_Oen();
            MHal_GPIO_TS0_SYNC_Low();

        break;
        case PAD_TS0_CLK:
            MHal_GPIO_TS0_CLK_Set();
            MHal_GPIO_TS0_CLK_Oen();
            MHal_GPIO_TS0_CLK_Low();

        break;
        case PAD_TS1_D0:
            MHal_GPIO_TS1_D0_Set();
            MHal_GPIO_TS1_D0_Oen();
            MHal_GPIO_TS1_D0_Low();

        break;
        case PAD_TS1_D1:
            MHal_GPIO_TS1_D1_Set();
            MHal_GPIO_TS1_D1_Oen();
            MHal_GPIO_TS1_D1_Low();

        break;
        case PAD_TS1_D2:
            MHal_GPIO_TS1_D2_Set();
            MHal_GPIO_TS1_D2_Oen();
            MHal_GPIO_TS1_D2_Low();

        break;
        case PAD_TS1_D3:
            MHal_GPIO_TS1_D3_Set();
            MHal_GPIO_TS1_D3_Oen();
            MHal_GPIO_TS1_D3_Low();

        break;
        case PAD_TS1_D4:
            MHal_GPIO_TS1_D4_Set();
            MHal_GPIO_TS1_D4_Oen();
            MHal_GPIO_TS1_D4_Low();

        break;
        case PAD_TS1_D5:
            MHal_GPIO_TS1_D5_Set();
            MHal_GPIO_TS1_D5_Oen();
            MHal_GPIO_TS1_D5_Low();

        break;
        case PAD_TS1_D6:
            MHal_GPIO_TS1_D6_Set();
            MHal_GPIO_TS1_D6_Oen();
            MHal_GPIO_TS1_D6_Low();

        break;
        case PAD_TS1_D7:
            MHal_GPIO_TS1_D7_Set();
            MHal_GPIO_TS1_D7_Oen();
            MHal_GPIO_TS1_D7_Low();

        break;
        case PAD_TS1_VLD:
            MHal_GPIO_TS1_VLD_Set();
            MHal_GPIO_TS1_VLD_Oen();
            MHal_GPIO_TS1_VLD_Low();

        break;
        case PAD_TS1_SYNC:
            MHal_GPIO_TS1_SYNC_Set();
            MHal_GPIO_TS1_SYNC_Oen();
            MHal_GPIO_TS1_SYNC_Low();

        break;
        case PAD_TS1_CLK:
            MHal_GPIO_TS1_CLK_Set();
            MHal_GPIO_TS1_CLK_Oen();
            MHal_GPIO_TS1_CLK_Low();

        break;
        case PAD_PCM_A4:
            MHal_GPIO_PCM_A4_Set();
            MHal_GPIO_PCM_A4_Oen();
            MHal_GPIO_PCM_A4_Low();

        break;
        case PAD_PCM_WAIT_N:
            MHal_GPIO_PCM_WAIT_N_Set();
            MHal_GPIO_PCM_WAIT_N_Oen();
            MHal_GPIO_PCM_WAIT_N_Low();

        break;
        case PAD_PCM_A5:
            MHal_GPIO_PCM_A5_Set();
            MHal_GPIO_PCM_A5_Oen();
            MHal_GPIO_PCM_A5_Low();

        break;
        case PAD_PCM_A6:
            MHal_GPIO_PCM_A6_Set();
            MHal_GPIO_PCM_A6_Oen();
            MHal_GPIO_PCM_A6_Low();

        break;
        case PAD_PCM_A7:
            MHal_GPIO_PCM_A7_Set();
            MHal_GPIO_PCM_A7_Oen();
            MHal_GPIO_PCM_A7_Low();

        break;
        case PAD_PCM_A12:
            MHal_GPIO_PCM_A12_Set();
            MHal_GPIO_PCM_A12_Oen();
            MHal_GPIO_PCM_A12_Low();

        break;
        case PAD_PCM_IRQA_N:
            MHal_GPIO_PCM_IRQA_N_Set();
            MHal_GPIO_PCM_IRQA_N_Oen();
            MHal_GPIO_PCM_IRQA_N_Low();

        break;
        case PAD_PCM_A14:
            MHal_GPIO_PCM_A14_Set();
            MHal_GPIO_PCM_A14_Oen();
            MHal_GPIO_PCM_A14_Low();

        break;
        case PAD_PCM_A13:
            MHal_GPIO_PCM_A13_Set();
            MHal_GPIO_PCM_A13_Oen();
            MHal_GPIO_PCM_A13_Low();

        break;
        case PAD_PCM_A8:
            MHal_GPIO_PCM_A8_Set();
            MHal_GPIO_PCM_A8_Oen();
            MHal_GPIO_PCM_A8_Low();

        break;
        case PAD_PCM_IOWR_N:
            MHal_GPIO_PCM_IOWR_N_Set();
            MHal_GPIO_PCM_IOWR_N_Oen();
            MHal_GPIO_PCM_IOWR_N_Low();

        break;
        case PAD_PCM_A9:
            MHal_GPIO_PCM_A9_Set();
            MHal_GPIO_PCM_A9_Oen();
            MHal_GPIO_PCM_A9_Low();

        break;
        case PAD_PCM_IORD_N:
            MHal_GPIO_PCM_IORD_N_Set();
            MHal_GPIO_PCM_IORD_N_Oen();
            MHal_GPIO_PCM_IORD_N_Low();

        break;
        case PAD_PCM_A11:
            MHal_GPIO_PCM_A11_Set();
            MHal_GPIO_PCM_A11_Oen();
            MHal_GPIO_PCM_A11_Low();

        break;
        case PAD_PCM_OE_N:
            MHal_GPIO_PCM_OE_N_Set();
            MHal_GPIO_PCM_OE_N_Oen();
            MHal_GPIO_PCM_OE_N_Low();

        break;
        case PAD_PCM_A10:
            MHal_GPIO_PCM_A10_Set();
            MHal_GPIO_PCM_A10_Oen();
            MHal_GPIO_PCM_A10_Low();

        break;
        case PAD_PCM_CE_N:
            MHal_GPIO_PCM_CE_N_Set();
            MHal_GPIO_PCM_CE_N_Oen();
            MHal_GPIO_PCM_CE_N_Low();

        break;
        case PAD_PCM_D7:
            MHal_GPIO_PCM_D7_Set();
            MHal_GPIO_PCM_D7_Oen();
            MHal_GPIO_PCM_D7_Low();

        break;
        case PAD_PCM_D6:
            MHal_GPIO_PCM_D6_Set();
            MHal_GPIO_PCM_D6_Oen();
            MHal_GPIO_PCM_D6_Low();

        break;
        case PAD_PCM_D5:
            MHal_GPIO_PCM_D5_Set();
            MHal_GPIO_PCM_D5_Oen();
            MHal_GPIO_PCM_D5_Low();

        break;
        case PAD_PCM_D4:
            MHal_GPIO_PCM_D4_Set();
            MHal_GPIO_PCM_D4_Oen();
            MHal_GPIO_PCM_D4_Low();

        break;
        case PAD_PCM_D3:
            MHal_GPIO_PCM_D3_Set();
            MHal_GPIO_PCM_D3_Oen();
            MHal_GPIO_PCM_D3_Low();

        break;
        case PAD_PCM_A3:
            MHal_GPIO_PCM_A3_Set();
            MHal_GPIO_PCM_A3_Oen();
            MHal_GPIO_PCM_A3_Low();

        break;
        case PAD_PCM_A2:
            MHal_GPIO_PCM_A2_Set();
            MHal_GPIO_PCM_A2_Oen();
            MHal_GPIO_PCM_A2_Low();

        break;
        case PAD_PCM_REG_N:
            MHal_GPIO_PCM_REG_N_Set();
            MHal_GPIO_PCM_REG_N_Oen();
            MHal_GPIO_PCM_REG_N_Low();

        break;
        case PAD_PCM_A1:
            MHal_GPIO_PCM_A1_Set();
            MHal_GPIO_PCM_A1_Oen();
            MHal_GPIO_PCM_A1_Low();

        break;
        case PAD_PCM_A0:
            MHal_GPIO_PCM_A0_Set();
            MHal_GPIO_PCM_A0_Oen();
            MHal_GPIO_PCM_A0_Low();

        break;
        case PAD_PCM_D0:
            MHal_GPIO_PCM_D0_Set();
            MHal_GPIO_PCM_D0_Oen();
            MHal_GPIO_PCM_D0_Low();

        break;
        case PAD_PCM_D1:
            MHal_GPIO_PCM_D1_Set();
            MHal_GPIO_PCM_D1_Oen();
            MHal_GPIO_PCM_D1_Low();

        break;
        case PAD_PCM_D2:
            MHal_GPIO_PCM_D2_Set();
            MHal_GPIO_PCM_D2_Oen();
            MHal_GPIO_PCM_D2_Low();

        break;
        case PAD_PCM_RESET:
            MHal_GPIO_PCM_RESET_Set();
            MHal_GPIO_PCM_RESET_Oen();
            MHal_GPIO_PCM_RESET_Low();

        break;
        case PAD_PCM_CD_N:
            MHal_GPIO_PCM_CD_N_Set();
            MHal_GPIO_PCM_CD_N_Oen();
            MHal_GPIO_PCM_CD_N_Low();

        break;
        case PAD_PCM2_CE_N:
            MHal_GPIO_PCM2_CE_N_Set();
            MHal_GPIO_PCM2_CE_N_Oen();
            MHal_GPIO_PCM2_CE_N_Low();

        break;
        case PAD_PCM2_IRQA_N:
            MHal_GPIO_PCM2_IRQA_N_Set();
            MHal_GPIO_PCM2_IRQA_N_Oen();
            MHal_GPIO_PCM2_IRQA_N_Low();

        break;
        case PAD_PCM2_WAIT_N:
            MHal_GPIO_PCM2_WAIT_N_Set();
            MHal_GPIO_PCM2_WAIT_N_Oen();
            MHal_GPIO_PCM2_WAIT_N_Low();

        break;
        case PAD_PCM2_RESET:
            MHal_GPIO_PCM2_RESET_Set();
            MHal_GPIO_PCM2_RESET_Oen();
            MHal_GPIO_PCM2_RESET_Low();

        break;
        case PAD_PCM2_CD_N:
            MHal_GPIO_PCM2_CD_N_Set();
            MHal_GPIO_PCM2_CD_N_Oen();
            MHal_GPIO_PCM2_CD_N_Set();

        break;
        case PAD_PF_AD15:
            MHal_GPIO_PF_AD15_Set();
            MHal_GPIO_PF_AD15_Oen();
            MHal_GPIO_PF_AD15_Low();

        break;
        case PAD_PF_CE0Z:
            MHal_GPIO_PF_CE0Z_Set();
            MHal_GPIO_PF_CE0Z_Oen();
            MHal_GPIO_PF_CE0Z_Low();

        break;
        case PAD_PF_CE1Z:
            MHal_GPIO_PF_CE1Z_Set();
            MHal_GPIO_PF_CE1Z_Oen();
            MHal_GPIO_PF_CE1Z_Low();

        break;
        case PAD_PF_OEZ:
            MHal_GPIO_PF_OEZ_Set();
            MHal_GPIO_PF_OEZ_Oen();
            MHal_GPIO_PF_OEZ_Low();

        break;
        case PAD_PF_WEZ:
            MHal_GPIO_PF_WEZ_Set();
            MHal_GPIO_PF_WEZ_Oen();
            MHal_GPIO_PF_WEZ_Low();

        break;
        case PAD_PF_ALE:
            MHal_GPIO_PF_ALE_Set();
            MHal_GPIO_PF_ALE_Oen();
            MHal_GPIO_PF_ALE_Low();

        break;
        case PAD_F_RBZ:
            MHal_GPIO_F_RBZ_Set();
            MHal_GPIO_F_RBZ_Oen();
            MHal_GPIO_F_RBZ_Low();

        break;
        case PAD_TCON0:
    //balup_090904
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO48_Set();
            		MHal_GPIO_GPIO48_Oen();
            		MHal_GPIO_GPIO48_Low();
				}
				else
				{
					MHal_GPIO_TCON0_Set();
            		MHal_GPIO_TCON0_Oen();
            		MHal_GPIO_TCON0_Low();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON0_Set();
        		MHal_GPIO_TCON0_Oen();
        		MHal_GPIO_TCON0_Low();
			}
		}
#else
            MHal_GPIO_TCON0_Set();
            MHal_GPIO_TCON0_Oen();
            MHal_GPIO_TCON0_Low();

#endif

        break;
        case PAD_TCON1:
            MHal_GPIO_TCON1_Set();
            MHal_GPIO_TCON1_Oen();
            MHal_GPIO_TCON1_Low();

        break;
        case PAD_TCON2:
     //balup_090904
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO49_Set();
            		MHal_GPIO_GPIO49_Oen();
            		MHal_GPIO_GPIO49_Low();
				}
				else
				{
					MHal_GPIO_TCON2_Set();
            		MHal_GPIO_TCON2_Oen();
            		MHal_GPIO_TCON2_Low();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON2_Set();
        		MHal_GPIO_TCON2_Oen();
        		MHal_GPIO_TCON2_Low();
			}
		}
#else
            MHal_GPIO_TCON2_Set();
            MHal_GPIO_TCON2_Oen();
            MHal_GPIO_TCON2_Low();

#endif

        break;
        case PAD_TCON3:
            MHal_GPIO_TCON3_Set();
            MHal_GPIO_TCON3_Oen();
            MHal_GPIO_TCON3_Low();

        break;
        case PAD_TCON4:
     //balup_090904
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO53_Set();
            		MHal_GPIO_GPIO53_Oen();
            		MHal_GPIO_GPIO53_Low();
				}
				else
				{
					MHal_GPIO_TCON4_Set();
            		MHal_GPIO_TCON4_Oen();
            		MHal_GPIO_TCON4_Low();
				}
			}
			else
			{
				MHal_GPIO_TCON4_Set();
        		MHal_GPIO_TCON4_Oen();
        		MHal_GPIO_TCON4_Low();
			}
		}
#else
            MHal_GPIO_TCON4_Set();
            MHal_GPIO_TCON4_Oen();
            MHal_GPIO_TCON4_Low();

#endif

        break;
        case PAD_TCON5:
            MHal_GPIO_TCON5_Set();
            MHal_GPIO_TCON5_Oen();
            MHal_GPIO_TCON5_Low();

        break;
        case PAD_TCON6:
     //balup_090904
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO52_Set();
            		MHal_GPIO_GPIO52_Oen();
            		MHal_GPIO_GPIO52_Low();
				}
				else
				{
					MHal_GPIO_TCON6_Set();
            		MHal_GPIO_TCON6_Oen();
            		MHal_GPIO_TCON6_Low();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON6_Set();
        		MHal_GPIO_TCON6_Oen();
        		MHal_GPIO_TCON6_Low();
			}
		}
#else
            MHal_GPIO_TCON6_Set();
            MHal_GPIO_TCON6_Oen();
            MHal_GPIO_TCON6_Low();
#endif

        break;
        case PAD_TCON7:
            MHal_GPIO_TCON7_Set();
            MHal_GPIO_TCON7_Oen();
            MHal_GPIO_TCON7_Low();

        break;
        case PAD_TCON8:
     //balup_090904
#ifdef _INCLUDE_S7BOOTSTRAP
		{
			U8	bootstrap	= MDrv_GPIO_GetBootStrap();

			if((bootstrap & 0x08) == 0x08)	// LCD
			{
			#ifdef _INCLUDE_NEWBOOTSTRAP
				if(((bootstrap & 0x62) == 0x20) || ((bootstrap & 0x60) == 0x20)) //Saturn7 Tcon [NO FRC + MINI LVDS]
			#else
				if((bootstrap & 0x14) == 0x04) //Saturn7 TCon (NO FRC+ Mini LVDS)
			#endif

				{
					MHal_GPIO_GPIO47_Set();
            		MHal_GPIO_GPIO47_Oen();
            		MHal_GPIO_GPIO47_Low();
				}
				else
				{
					MHal_GPIO_TCON8_Set();
            		MHal_GPIO_TCON8_Oen();
            		MHal_GPIO_TCON8_Low();
				}
			}
			else	// PDP
			{
				MHal_GPIO_TCON8_Set();
        		MHal_GPIO_TCON8_Oen();
        		MHal_GPIO_TCON8_Low();
			}
		}
#else
            MHal_GPIO_TCON8_Set();
            MHal_GPIO_TCON8_Oen();
            MHal_GPIO_TCON8_Low();

#endif

        break;
        case PAD_TCON9:
            MHal_GPIO_TCON9_Set();
            MHal_GPIO_TCON9_Oen();
            MHal_GPIO_TCON9_Low();

        break;
        case PAD_TCON10:
            MHal_GPIO_TCON10_Set();
            MHal_GPIO_TCON10_Oen();
            MHal_GPIO_TCON10_Low();

        break;
        case PAD_TCON11:
            MHal_GPIO_TCON11_Set();
            MHal_GPIO_TCON11_Oen();
            MHal_GPIO_TCON11_Low();

        break;
        case PAD_TCON12:
            MHal_GPIO_TCON12_Set();
            MHal_GPIO_TCON12_Oen();
            MHal_GPIO_TCON12_Low();

        break;
        case PAD_TCON13:
            MHal_GPIO_TCON13_Set();
            MHal_GPIO_TCON13_Oen();
            MHal_GPIO_TCON13_Low();

        break;
        case PAD_TCON14:
            MHal_GPIO_TCON14_Set();
             MHal_GPIO_TCON14_Oen();
            MHal_GPIO_TCON14_Low();

        break;
        case PAD_TCON15:
            MHal_GPIO_TCON15_Set();
            MHal_GPIO_TCON15_Oen();
            MHal_GPIO_TCON15_Low();

        break;
        case PAD_TCON16:
            MHal_GPIO_TCON16_Set();
                MHal_GPIO_TCON16_Oen();
            MHal_GPIO_TCON16_Low();

        break;
        case PAD_TCON17:
            MHal_GPIO_TCON17_Set();
            MHal_GPIO_TCON17_Oen();
            MHal_GPIO_TCON17_Low();

        break;
        case PAD_TCON18:
            MHal_GPIO_TCON18_Set();
            MHal_GPIO_TCON18_Oen();
            MHal_GPIO_TCON18_Low();

        break;
        case PAD_TCON19:
            MHal_GPIO_TCON19_Set();
            MHal_GPIO_TCON19_Oen();
            MHal_GPIO_TCON19_Low();

        break;
        case PAD_TCON20:
            MHal_GPIO_TCON20_Set();
            MHal_GPIO_TCON20_Oen();
            MHal_GPIO_TCON20_Low();

        break;
        case PAD_TCON21:
            MHal_GPIO_TCON21_Set();
            MHal_GPIO_TCON21_Oen();
            MHal_GPIO_TCON21_Low();

        break;
        case PAD_ET_COL:
            MHal_GPIO_ET_COL_Set();
            MHal_GPIO_ET_COL_Oen();
            MHal_GPIO_ET_COL_Low();

        break;
        case PAD_ET_TXD1:
            MHal_GPIO_ET_TXD1_Set();
            MHal_GPIO_ET_TXD1_Oen();
            MHal_GPIO_ET_TXD1_Low();

        break;
        case PAD_ET_TXD0:
            MHal_GPIO_ET_TXD0_Set();
            MHal_GPIO_ET_TXD0_Oen();
            MHal_GPIO_ET_TXD0_Low();

        break;
        case PAD_ET_TX_EN:
            MHal_GPIO_ET_TX_EN_Set();
            MHal_GPIO_ET_TX_EN_Oen();
            MHal_GPIO_ET_TX_EN_Low();

        break;
        case PAD_ET_TX_CLK:
            MHal_GPIO_ET_TX_CLK_Set();
            MHal_GPIO_ET_TX_CLK_Oen();
            MHal_GPIO_ET_TX_CLK_Low();

        break;
        case PAD_ET_RXD0:
            MHal_GPIO_ET_RXD0_Set();
            MHal_GPIO_ET_RXD0_Oen();
            MHal_GPIO_ET_RXD0_Low();

        break;
        case PAD_ET_RXD1:
            MHal_GPIO_ET_RXD1_Set();
            MHal_GPIO_ET_RXD1_Oen();
            MHal_GPIO_ET_RXD1_Low();

        break;
        case PAD_ET_MDC:
            MHal_GPIO_ET_MDC_Set();
            MHal_GPIO_ET_MDC_Oen();
            MHal_GPIO_ET_MDC_Low();

        break;
        case PAD_ET_EDIO:
            MHal_GPIO_ET_EDIO_Set();
            MHal_GPIO_ET_EDIO_Oen();
            MHal_GPIO_ET_EDIO_Low();

        break;
        case PAD_I2S_IN_WS:
            MHal_GPIO_I2S_IN_WS_Set();
            MHal_GPIO_I2S_IN_WS_Oen();
            MHal_GPIO_I2S_IN_WS_Low();

        break;
        case PAD_I2S_IN_BCK:
            MHal_GPIO_I2S_IN_BCK_Set();
            MHal_GPIO_I2S_IN_BCK_Oen();
            MHal_GPIO_I2S_IN_BCK_Low();

        break;
        case PAD_I2S_IN_SD:
            MHal_GPIO_I2S_IN_SD_Set();
            MHal_GPIO_I2S_IN_SD_Oen();
            MHal_GPIO_I2S_IN_SD_Low();

        break;
        case PAD_SPDIF_IN:
            MHal_GPIO_SPDIF_IN_Set();
            MHal_GPIO_SPDIF_IN_Oen();
            MHal_GPIO_SPDIF_IN_Low();

        break;
        case PAD_SPDIF_OUT:
            MHal_GPIO_SPDIF_OUT_Set();
            MHal_GPIO_SPDIF_OUT_Oen();
            MHal_GPIO_SPDIF_OUT_Low();

        break;
        case PAD_I2S_OUT_MCK:
            MHal_GPIO_I2S_OUT_MCK_Set();
            MHal_GPIO_I2S_OUT_MCK_Oen();
            MHal_GPIO_I2S_OUT_MCK_Low();

        break;
        case PAD_I2S_OUT_WS:
            MHal_GPIO_I2S_OUT_WS_Set();
            MHal_GPIO_I2S_OUT_WS_Oen();
            MHal_GPIO_I2S_OUT_WS_Low();

        break;
        case PAD_I2S_OUT_BCK:
            MHal_GPIO_I2S_OUT_BCK_Set();
            MHal_GPIO_I2S_OUT_BCK_Oen();
            MHal_GPIO_I2S_OUT_BCK_Low();

        break;
        case PAD_I2S_OUT_SD:
            MHal_GPIO_I2S_OUT_SD_Set();
            MHal_GPIO_I2S_OUT_SD_Oen();
            MHal_GPIO_I2S_OUT_SD_Low();

        break;
        case PAD_I2S_OUT_SD1:
            MHal_GPIO_I2S_OUT_SD1_Set();
            MHal_GPIO_I2S_OUT_SD1_Oen();
            MHal_GPIO_I2S_OUT_SD1_Low();

        break;
        case PAD_I2S_OUT_SD2:
            MHal_GPIO_I2S_OUT_SD2_Set();
            MHal_GPIO_I2S_OUT_SD2_Oen();
            MHal_GPIO_I2S_OUT_SD2_Low();

        break;
        case PAD_I2S_OUT_SD3:
            MHal_GPIO_I2S_OUT_SD3_Set();
            MHal_GPIO_I2S_OUT_SD3_Oen();
            MHal_GPIO_I2S_OUT_SD3_Low();

        break;
        case PAD_B_ODD_0:
            MHal_GPIO_B_ODD_0_Set();
               MHal_GPIO_B_ODD_0_Oen();
            MHal_GPIO_B_ODD_0_Low();

        break;
        case PAD_B_ODD_1:
            MHal_GPIO_B_ODD_1_Set();
            MHal_GPIO_B_ODD_1_Oen();
            MHal_GPIO_B_ODD_1_Low();

        break;
        case PAD_B_ODD_2:
            MHal_GPIO_B_ODD_2_Set();
            MHal_GPIO_B_ODD_2_Oen();
            MHal_GPIO_B_ODD_2_Low();

        break;
        case PAD_B_ODD_3:
            MHal_GPIO_B_ODD_3_Set();
            MHal_GPIO_B_ODD_3_Oen();
            MHal_GPIO_B_ODD_3_Low();

        break;
        case PAD_B_ODD_4:
            MHal_GPIO_B_ODD_4_Set();
            MHal_GPIO_B_ODD_4_Oen();
            MHal_GPIO_B_ODD_4_Low();

        break;
        case PAD_B_ODD_5:
            MHal_GPIO_B_ODD_5_Set();
            MHal_GPIO_B_ODD_5_Oen();
            MHal_GPIO_B_ODD_5_Low();

        break;
        case PAD_B_ODD_6:
            MHal_GPIO_B_ODD_6_Set();
            MHal_GPIO_B_ODD_6_Oen();
            MHal_GPIO_B_ODD_6_Low();

        break;
        case PAD_B_ODD_7:
            MHal_GPIO_B_ODD_7_Set();
            MHal_GPIO_B_ODD_7_Oen();
            MHal_GPIO_B_ODD_7_Low();

        break;
        case PAD_G_ODD_0:
            MHal_GPIO_G_ODD_0_Set();
            MHal_GPIO_G_ODD_0_Oen();
            MHal_GPIO_G_ODD_0_Low();

        break;
        case PAD_G_ODD_1:
            MHal_GPIO_G_ODD_1_Set();
            MHal_GPIO_G_ODD_1_Oen();
            MHal_GPIO_G_ODD_1_Low();

        break;
        case PAD_G_ODD_2:
            MHal_GPIO_G_ODD_2_Set();
            MHal_GPIO_G_ODD_2_Oen();
            MHal_GPIO_G_ODD_2_Low();

        break;
        case PAD_G_ODD_3:
            MHal_GPIO_G_ODD_3_Set();
            MHal_GPIO_G_ODD_3_Oen();
            MHal_GPIO_G_ODD_3_Low();

        break;
        case PAD_G_ODD_4:
            MHal_GPIO_G_ODD_4_Set();
            MHal_GPIO_G_ODD_4_Oen();
            MHal_GPIO_G_ODD_4_Low();

        break;
        case PAD_G_ODD_5:
            MHal_GPIO_G_ODD_5_Set();
            MHal_GPIO_G_ODD_5_Oen();
            MHal_GPIO_G_ODD_5_Low();

        break;
        case PAD_G_ODD_6:
            MHal_GPIO_G_ODD_6_Set();
            MHal_GPIO_G_ODD_6_Oen();
            MHal_GPIO_G_ODD_6_Low();

        break;
        case PAD_G_ODD_7:
            MHal_GPIO_G_ODD_7_Set();
            MHal_GPIO_G_ODD_7_Oen();
            MHal_GPIO_G_ODD_7_Low();

        break;
        case PAD_R_ODD_0:
            MHal_GPIO_R_ODD_0_Set();
            MHal_GPIO_R_ODD_0_Oen();
            MHal_GPIO_R_ODD_0_Low();

        break;
        case PAD_R_ODD_1:
            MHal_GPIO_R_ODD_1_Set();
            MHal_GPIO_R_ODD_1_Oen();
            MHal_GPIO_R_ODD_1_Low();

        break;
        case PAD_R_ODD_2:
            MHal_GPIO_R_ODD_2_Set();
            MHal_GPIO_R_ODD_2_Oen();
            MHal_GPIO_R_ODD_2_Low();

        break;
        case PAD_R_ODD_3:
            MHal_GPIO_R_ODD_3_Set();
            MHal_GPIO_R_ODD_3_Oen();
            MHal_GPIO_R_ODD_3_Low();

        break;
        case PAD_R_ODD_4:
            MHal_GPIO_R_ODD_4_Set();
            MHal_GPIO_R_ODD_4_Oen();
            MHal_GPIO_R_ODD_4_Low();

        break;
        case PAD_R_ODD_5:
            MHal_GPIO_R_ODD_5_Set();
            MHal_GPIO_R_ODD_5_Oen();
            MHal_GPIO_R_ODD_5_Low();

        break;
        case PAD_R_ODD_6:
            MHal_GPIO_R_ODD_6_Set();
            MHal_GPIO_R_ODD_6_Oen();
            MHal_GPIO_R_ODD_6_Low();

        break;
        case PAD_R_ODD_7:
            MHal_GPIO_R_ODD_7_Set();
            MHal_GPIO_R_ODD_7_Oen();
            MHal_GPIO_R_ODD_7_Low();

        break;
        case PAD_mini_LVDS_0:
            MHal_GPIO_mini_LVDS_0_Set();
            MHal_GPIO_mini_LVDS_0_Oen();
            MHal_GPIO_mini_LVDS_0_Low();

        break;
        case PAD_mini_LVDS_1:
            MHal_GPIO_mini_LVDS_1_Set();
            MHal_GPIO_mini_LVDS_1_Oen();
            MHal_GPIO_mini_LVDS_1_Low();

        break;
        case PAD_mini_LVDS_2:
            MHal_GPIO_mini_LVDS_2_Set();
            MHal_GPIO_mini_LVDS_2_Oen();
            MHal_GPIO_mini_LVDS_2_Low();

        break;
        case PAD_mini_LVDS_3:
            MHal_GPIO_mini_LVDS_3_Set();
            MHal_GPIO_mini_LVDS_3_Oen();
            MHal_GPIO_mini_LVDS_3_Low();

        break;
        case PAD_mini_LVDS_4:
            MHal_GPIO_mini_LVDS_4_Set();
            MHal_GPIO_mini_LVDS_4_Oen();
            MHal_GPIO_mini_LVDS_4_Low();

        break;
        case PAD_mini_LVDS_5:
            MHal_GPIO_mini_LVDS_5_Set();
            MHal_GPIO_mini_LVDS_5_Oen();
            MHal_GPIO_mini_LVDS_5_Low();

        break;
        case PAD_mini_LVDS_6:
            MHal_GPIO_mini_LVDS_6_Set();
            MHal_GPIO_mini_LVDS_6_Oen();
            MHal_GPIO_mini_LVDS_6_Low();

        break;
        case PAD_mini_LVDS_7:
            MHal_GPIO_mini_LVDS_7_Set();
            MHal_GPIO_mini_LVDS_7_Oen();
            MHal_GPIO_mini_LVDS_7_Low();

        break;
        case PAD_LCK:
            MHal_GPIO_LCK_Set();
            MHal_GPIO_LCK_Oen();
            MHal_GPIO_LCK_Low();

        break;
        case PAD_LDE:
            MHal_GPIO_LDE_Set();
            MHal_GPIO_LDE_Oen();
            MHal_GPIO_LDE_Low();

        break;
        case PAD_LHSYNC:
            MHal_GPIO_LHSYNC_Set();
            MHal_GPIO_LHSYNC_Oen();
            MHal_GPIO_LHSYNC_Low();

        break;
        case PAD_LVSYNC:
            MHal_GPIO_LVSYNC_Set();
            MHal_GPIO_LVSYNC_Oen();
            MHal_GPIO_LVSYNC_Low();

        break;
        case PAD_PCM_WE_N:
            MHal_GPIO_PCM_WE_N_Set();
            MHal_GPIO_PCM_WE_N_Oen();
            MHal_GPIO_PCM_WE_N_Low();

        break;



        default:
            break;
    }
}
EXPORT_SYMBOL(MDrv_GPIO_Set_Low);
