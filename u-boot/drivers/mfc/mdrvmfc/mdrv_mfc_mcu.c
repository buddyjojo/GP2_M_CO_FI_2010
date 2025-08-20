/******************************************************************************
 Copyright (c) 2005 MStar Semiconductor, Inc.
 All rights reserved.

 [Module Name]: MsMcu.c
 [Date]:        17-Aug-2005
 [Comment]:
   Mcu control subroutines.
 [Reversion History]:
*******************************************************************************/
#define _MSMCU_C_

#include "mdrv_mfc_platform.h"
#include "mdrv_mfc_mcu.h"
#include "mdrv_mfc_fb.h"

extern void	udelay	      (unsigned long);

#if(CODESIZE_SEL == CODESIZE_ALL)
#if (WATCH_DOG_TIMER)
void MDrv_MFC_McuWatchDogInit(U8 u8Sec)
{
    U16 u16Value;

	u16Value = 65536-(MCU_XTAL_CLK_HZ/65536)*u8Sec;
	MDrv_MFC_Write2Bytes(0x3C62, u16Value);
}

void MDrv_MFC_McuWatchDogClear(void)
{
    MDrv_MFC_WriteBit(0x3C66, 1, _BIT1); // XD_MCU1[0xC0] = 0;
}
#endif

void MDrv_MFC_McuWatchDogCtrl(BOOL bEnable)
{
    if (bEnable)
    {
    	MDrv_MFC_WriteByte(0x3C60, 0xaa);
    	MDrv_MFC_WriteByte(0x3C61, 0x55);
    }
    else // disable watch dog
    {
    	MDrv_MFC_WriteByte(0x3C60, 0x55);
    	MDrv_MFC_WriteByte(0x3C61, 0xaa);
    }
}

void MDrv_MFC_McuICacheCtrl(BOOL bEnable)
{
    if (bEnable)
    {
        MDrv_MFC_WriteBit(0x1018, 1, _BIT3);
        MDrv_MFC_WriteBit(0x2BA0, 0, _BIT0);
    }
    else
    {
        MDrv_MFC_WriteBit(0x1018, 0, _BIT3);
        MDrv_MFC_WriteBit(0x2BA0, 1, _BIT0);
    }
}

#if (ISR_EXT_TIMER0_INT)
void MDrv_MFC_McuExtTimer0_Init(void)
{
    // 1Hz interrupt
    MDrv_MFC_WriteByte(0x3C80, (U8)MCU_XTAL_CLK_HZ);
    MDrv_MFC_WriteByte(0x3C81, (U8)MCU_XTAL_CLK_HZ>>8);
    MDrv_MFC_WriteByte(0x3C82, (U8)MCU_XTAL_CLK_HZ>>16);
    MDrv_MFC_WriteByte(0x3C83, (U8)MCU_XTAL_CLK_HZ>>24);

    MDrv_MFC_WriteByte(0x3C89, 0x03);
	MDrv_MFC_WriteByteMask(0x2B00, 0, _BIT0); // int mask 0.0 ex_timer0
}

#endif

#if (ISR_EXT_TIMER1_INT)
void MDrv_MFC_McuExtTimer1_Init(void)
{
    // 1KHz interrupt
    MDrv_MFC_WriteByte(0x3CA0, (U8)MCU_XTAL_CLK_KHZ);
    MDrv_MFC_WriteByte(0x3CA1, (U8)MCU_XTAL_CLK_KHZ>>8);
    MDrv_MFC_WriteByte(0x3CA2, (U8)MCU_XTAL_CLK_KHZ>>16);
    MDrv_MFC_WriteByte(0x3CA3, (U8)MCU_XTAL_CLK_KHZ>>24);

    MDrv_MFC_WriteByte(0x3CA9, 0x03);
    MDrv_MFC_WriteByteMask(0x2B00, 0, _BIT1); // int mask 0.1 ex_timer1
}
#endif

void MDrv_MFC_McuUsePLL(BOOL bEnable)
{
    MDrv_MFC_WriteBit(0x1E06, bEnable, _BIT5);       // [0]
}
/**
MPLL_CLOCK_X2_KHZ/2.5, // 0, 432/2.5 = 173MHz
MPLL_CLOCK_X2_KHZ/3,   // 1, 432/3   = 144MHz
MPLL_CLOCK_X2_KHZ/3.5, // 2, 432/3.5 = 123MHz
MPLL_CLOCK_X2_KHZ/4,   // 3, 432/4   = 108MHz
MPLL_CLOCK_X2_KHZ/8,   // 4, 432/8   =  54MHz
*/
void MDrv_MFC_SetMcuSpeed(U8 u8Speed)
{
    BOOL bPLLEnable;

    if(u8Speed>4)
        bPLLEnable = 0;
    else
        bPLLEnable = 1;

    MDrv_MFC_McuUsePLL(_DISABLE);                                      // MCU use Crystal clock
    if(bPLLEnable)
    {
        MDrv_MFC_WriteByteMask(0x1E06, u8Speed<<2, (_BIT4|_BIT3|_BIT2));
        MDrv_MFC_McuUsePLL(_ENABLE);                                    // MCU use PLL clock
    }
    else
    {
        MDrv_MFC_WriteByteMask(0x1E06, (_BIT4|_BIT3|_BIT2), (_BIT4|_BIT3|_BIT2));
    }

    MDrv_MFC_WriteByteMask(0x2c48, u8Speed, 0x0f);
}

void MDrv_MFC_SetInterrupt(BOOL bCtrl)
{
    // do not enable ursa interrupt in the bootloader.
    #if !MFC_USE_IN_BOOLLOAD
	if (bCtrl) //Enable
	{
		MDrv_MFC_WriteBit(0x2401, 0, _BIT0); // Vsync change irq
		MDrv_MFC_WriteBit(0x2406, 0, _BIT1); // OP Vsync irq
		MDrv_MFC_WriteBit(0x2406, 0, _BIT3); // end of DE irq
	}
	else
    #endif
	{
		//MDrv_MFC_WriteByte(0x2400, 0xFF); //mask int0.0~7
		//MDrv_MFC_WriteByte(0x2401, 0xFF); //        int0.8~15
		//MDrv_MFC_WriteByte(0x2406, 0xFF); //mask int1.0~7
		//MDrv_MFC_WriteByte(0x2407, 0xFF); //	      int1.8~15
		MDrv_MFC_WriteBit(0x2401, 1, _BIT0); // Vsync change irq
		MDrv_MFC_WriteBit(0x2406, 1, _BIT1); // OP Vsync irq
		MDrv_MFC_WriteBit(0x2406, 1, _BIT3); // end of DE irq
	}

}

/////////////////////////////////////////////////////////////////////////
void MDrv_MFC_McuInitialize(void)
{
    MDrv_MFC_McuUsePLL(0);
    //Reset MPLL
	//Read the register to know is URSA version and save it to dummy bank
	MDrv_MFC_WriteByte( 0x1E24, ((MDrv_MFC_ReadByte(0x1203)&0xC0)?0x02:0x01));
	MDrv_MFC_WriteBit(0x1E80, 1, _BIT7);  // MPLL power down
	//msWriteBit(0x1E81, 1, _BIT0);  // MPLL power on reset
	MDrv_MFC_WriteByteMask(0x1E81, _BIT0|_BIT1, _BIT0|_BIT1);  // MPLL power on reset
	#if(CODEBASE_SEL == CODEBASE_51)
	    MDrv_MFC_Write2Bytes(0x1E86, 0x0902); // Loop divider setting
    #else
        MDrv_MFC_Write2Bytes(0x1E86, 0x0901);
    #endif
	MDrv_MFC_WriteBit(0x1E80, 0, _BIT7);  // MPLL power on
	//MDrv_MFC_WriteBit(0x1E81, 0, _BIT0);  // MPLL power on reset release
	MDrv_MFC_WriteByteMask(0x1E81, 0, _BIT0|_BIT1);  // MPLL power on reset
    mfcSleepMsNop(5);
    MDrv_MFC_WriteByte( 0x1E80, 0x00);
	MDrv_MFC_WriteByte( 0x1E81, 0x00);
    MDrv_MFC_McuICacheCtrl(_ENABLE);

#if (WATCH_DOG_TIMER)
	MDrv_MFC_McuWatchDogInit(WATCH_DOG_TIMER);
    MDrv_MFC_McuWatchDogCtrl(_ENABLE);
	MDrv_MFC_McuWatchDogClear();
#else
    MDrv_MFC_McuWatchDogCtrl(_DISABLE);
#endif

    MDrv_MFC_SetMcuSpeed(1); //3
	MDrv_MFC_WriteBit(0x1E03, _ENABLE, _BIT2); // RX enable

#if (ISR_EXT_TIMER0_INT)
	MDrv_MFC_McuExtTimer0_Init();
#endif
#if (ISR_EXT_TIMER1_INT)
	MDrv_MFC_McuExtTimer1_Init();
#endif
	MDrv_MFC_WriteByte(0x2400, 0xFF); //mask int0.0~7
	MDrv_MFC_WriteByte(0x2401, 0xFF); //        int0.8~15
	MDrv_MFC_WriteByte(0x2404, 0xFF);  //clear
	MDrv_MFC_WriteByte(0x2405, 0xFF);
	MDrv_MFC_WriteByte(0x2404, 0x00);  //clear
	MDrv_MFC_WriteByte(0x2405, 0x00);

	MDrv_MFC_WriteByte(0x2406, 0xFF); //mask int1.0~7
	MDrv_MFC_WriteByte(0x2407, 0xFF); //	      int1.8~15
	MDrv_MFC_WriteByte(0x240A, 0xFF);  //clear
	MDrv_MFC_WriteByte(0x240B, 0xFF);
	MDrv_MFC_WriteByte(0x240A, 0x00);  //clear
	MDrv_MFC_WriteByte(0x240B, 0x00);

	MDrv_MFC_WriteBit(0x2B18, 0, _BIT0); // Scaler irq
	MDrv_MFC_SetInterrupt(_DISABLE);
}
#endif

