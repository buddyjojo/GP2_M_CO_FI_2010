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

#include "board.h"


#include "hwreg.h"
//#include "mreg51.h"

#include "msKeypad.h"
#include "msIR.h"
//#include "msAPI_Timer.h"

void msKeypad_Init(void)
{
	XBYTE[0x1408] |= KEYPAD_CH1_UB;  //channel 1
	XBYTE[0x140A] |= KEYPAD_CH1_LB;

	XBYTE[0x140C] |= KEYPAD_CH2_UB;  //channel 2
	XBYTE[0x140E] |= KEYPAD_CH2_LB;  //0x21

	XBYTE[0x1410] |= 0x3f;  //channel 3
	XBYTE[0x1412] |= 0x00;

	XBYTE[0x1414] |= 0x3f;  //channel 4
	XBYTE[0x1416] |= 0x00;

	XBYTE[REG_SAR_CTRL0] = 0x24;
 	XBYTE[REG_SAR_CTRL1] = 0x00;
	XBYTE[REG_SAR_CTRL2] = 0x05;
	XBYTE[REG_SAR_CTRL3] = 0x00;
	XBYTE[REG_SAR_CTRL21] |= 0x80;

	XBYTE[0x1426] = 0x00;
	XBYTE[0x1426] |= BIT1;
	XBYTE[0x1426] &= ~BIT1;

}

#if 0 /*(KEYPAD_TYPE_SEL != KEYPAD_TYPE_NONE) && \
    (KEYPAD_TYPE_SEL != KEYPAD_TYPE_BOARDDEP) */

#define _MSKEYPAD_C_

#include "hwreg.h"
//#include "mreg51.h"

#include "msKeypad.h"
#include "msIR.h"
//#include "msAPI_Timer.h"

#if KEYPAD_USE_ISR
//static U32 KeypadSuccessTime; //100 ms based
//BOOLEAN KeypadRepeatStart;
static U8 PreviousCMD;
#else
static U8 KeyPadCheckCount, PreviousCMD, PressKey;
static U32 KeyPadTimePeriod;
U32 volatile g100msTimeCount;
#endif

unsigned char tADCKeyLevel[] =
{
    ADC_KEY_L0,
    ADC_KEY_L1,
    ADC_KEY_L2,
    ADC_KEY_L3,
};


unsigned char code tADCKey1Flag[] =
{
    ADC_KEY_1_L0_FLAG,
    ADC_KEY_1_L1_FLAG,
    ADC_KEY_1_L2_FLAG,
    ADC_KEY_1_L3_FLAG,
};

unsigned char code tADCKey2Flag[] =
{
    ADC_KEY_2_L0_FLAG,
    ADC_KEY_2_L1_FLAG,
    ADC_KEY_2_L2_FLAG,
    ADC_KEY_2_L3_FLAG,
};

void msKeypad_Init(void)
{
    #if KEYPAD_USE_ISR
    int i=0;
    PreviousCMD = 0;
    //KeyPadCheckCount = 0;
    //PressKey = FALSE;
    for(i=0;i<sizeof(KEYPAD_LV_CHANNEL);i++){
        KEYPAD_LV_CHANNEL[i]=0;
    }

    for(i=0;i<sizeof(KEYPAD_PREVIOUS_LV_CHANNEL);i++){
        KEYPAD_PREVIOUS_LV_CHANNEL[i]=0;
    }

     for(i=0;i<sizeof(KEYPAD_LV_COUNT_CHANNEL);i++){
        KEYPAD_LV_COUNT_CHANNEL[i]=0;
    }
    #else
    PreviousCMD = 0;
    KeyPadCheckCount = 0;
    PressKey = FALSE;
    #endif

#if( POWER_KEY_PAD_BY_INTERRUPT )
    //u8PwrKeypadIntFlag = FALSE;
#endif

	XBYTE[0x1408] |= 0x3f;  //channel 1
	XBYTE[0x140A] |= 0x21;

	XBYTE[0x140C] |= 0x3f;  //channel 2
	XBYTE[0x140E] |= 0x21;

	XBYTE[0x1410] |= 0x3f;  //channel 3
	XBYTE[0x1412] |= 0x00;

	XBYTE[0x1414] |= 0x3f;  //channel 4
	XBYTE[0x1416] |= 0x00;

	XBYTE[REG_SAR_CTRL0] = 0x24;
 	XBYTE[REG_SAR_CTRL1] = 0x00;
	XBYTE[REG_SAR_CTRL2] = 0x05;
	XBYTE[REG_SAR_CTRL3] = 0x00;
	XBYTE[REG_SAR_CTRL21] |= 0x80;

	XBYTE[0x1426] = 0x00;
	XBYTE[0x1426] |= BIT1;
	XBYTE[0x1426] &= ~BIT1;


}

#if KEYPAD_USE_ISR
//this function will not check if key is pressed
static BOOLEAN msKeypad_IsKeyRepeat(U8 channelIdx)
{
    return (KEYPAD_LV_COUNT_CHANNEL[channelIdx]>KEYPAD_LV_STABLE_COUNT);
}

//modified by using ISR
BOOLEAN msKeypad_CH_GetKey(U8 *pkey, U8* pflag)
{
    U8  *Keymapping;
    U8 Channel;
    *pkey = 0xFF;
    *pflag = 0;



    //check if key pressed
    for (Channel=0; Channel<ADC_KEY_CHANNEL_NUM; Channel++)
    {
         if(KEYPAD_LV_CHANNEL[Channel]>0){
             break;
         }
    }


    if(Channel<ADC_KEY_CHANNEL_NUM)//Key pressed
    {
        //decide key map
        switch(Channel)
        {
            case KEYPAD_ADC_CHANNEL_1:
                Keymapping = &tADCKey1Flag;
                break;
            case KEYPAD_ADC_CHANNEL_2:
                Keymapping = &tADCKey2Flag;
                break;
            default:
                break;
        }

        //process key
        *pkey = *(Keymapping+(KEYPAD_LV_CHANNEL[Channel]-1));
        *pflag=msKeypad_IsKeyRepeat(Channel);
        KEYPAD_LV_CHANNEL[Channel]=0;//reset the buffer

        /*
        if(*pflag&&(g100msTimeCount-KeypadSuccessTime)<=KeypadRepeatTimerCount)
        {
            return FALSE;
        }

        KeypadSuccessTime=g100msTimeCount;
        */

        return TRUE;

    }
    else
    {
        return FALSE;
    }

}
#else


BOOLEAN msKeypad_CH_GetKey(U8 Channel, U8 *pkey, U8* pflag)
{
    U8 i, j, KEY_LV[ADC_KEY_LEVEL], Key_Value, *Keymapping;
    U16 RegAddr;

    *pkey = 0xFF;
    *pflag = 0;

#if 0//( POWER_KEY_PAD_BY_INTERRUPT )
    if (u8PwrKeypadIntFlag){
        u8PwrKeypadIntFlag = FALSE;
        *pkey = IRKEY_POWER;
        return TRUE;
    }
#endif


    for(i=0; i<ADC_KEY_LEVEL; i++)
        KEY_LV[i] = 0;



    switch(Channel)
    {
        case KEYPAD_ADC_CHANNEL_1:
            RegAddr = REG_SAR_ADCOUT1;

            Keymapping = &tADCKey1Flag;
            break;

        case KEYPAD_ADC_CHANNEL_2:
            RegAddr = REG_SAR_ADCOUT2;

            Keymapping = &tADCKey2Flag;
            break;
        default:
            break;
    }

    for ( i = 0; i < KEYPAD_STABLE_NUM; i++ )
    {
        Key_Value = XBYTE[RegAddr];

        for (j=0;j<ADC_KEY_LEVEL;j++)
        {
            if (Key_Value < tADCKeyLevel[j])
            {
                KEY_LV[j]++;
                break;
            }
        }
    }

    for(i=0; i<ADC_KEY_LEVEL; i++)
    {
        if(KEY_LV[i] > KEYPAD_STABLE_NUM_MIN)
        {
            PressKey = TRUE;
            *pkey = *(Keymapping+i);
            if (PreviousCMD != *pkey)
            {
                PreviousCMD = *pkey;
                KeyPadCheckCount = 0;
            }
            else
            {
                if (KeyPadCheckCount < KEYPAD_KEY_VALIDATION)
                {
                    KeyPadCheckCount++;
                    return FALSE;
                }
                else if (KeyPadCheckCount == KEYPAD_KEY_VALIDATION)
                {
                    KeyPadCheckCount++;
                    KeyPadTimePeriod = g100msTimeCount;
                    return TRUE;
                }

                if (KeyPadCheckCount == KEYPAD_REPEAT_KEY_CHECK)	//3+2
                {
                    if (g100msTimeCount > KeyPadTimePeriod + KEYPAD_REPEAT_PERIOD/3)
                    {
                        KeyPadTimePeriod = g100msTimeCount;
                        KeyPadCheckCount = KEYPAD_REPEAT_KEY_CHECK_1;
                        *pflag = 0x01;
                    }
                    else
                    {
                    	*pkey = 0xFF;
                    	*pflag = 0x01;
                    }

                        return TRUE;
                    }
                else if (KeyPadCheckCount == KEYPAD_REPEAT_KEY_CHECK_1)	//3+3
                {
                    if (g100msTimeCount > KeyPadTimePeriod)
                    {
                        KeyPadTimePeriod = g100msTimeCount;
                        KeyPadCheckCount = KEYPAD_REPEAT_KEY_CHECK_1;
                        *pflag = 0x01;
                    }
                    else
                    {
                    	*pkey = 0xFF;
                    	*pflag = 0x01;
                    }

                        return TRUE;
                 }

                if (g100msTimeCount > KeyPadTimePeriod + KEYPAD_REPEAT_PERIOD)	//if 700ms
                {
                    KeyPadTimePeriod = g100msTimeCount;
                    KeyPadCheckCount = KEYPAD_REPEAT_KEY_CHECK;	//3+2
                    *pflag = 0x01;
                    return TRUE;
                }
                else
                    return FALSE;

            }
        }
    }

    if(Channel == ADC_KEY_LAST_CHANNEL)
    {
        if (PressKey)
            PressKey = FALSE;

        else
            PreviousCMD = 0xFF;
    }
    return FALSE;
}
#endif


/******************************************************************************/
///Keypad get key value and repeat flag
///@param pkey \b IN return the key value(The same as Irda key value)
///@param pflag \b IN return the repeat flag(1:Repeat)
/******************************************************************************/
BOOLEAN msKeypad_GetKey(U8 *pkey, U8 *pflag)
{
    #if KEYPAD_USE_ISR


    return msKeypad_CH_GetKey(pkey, pflag);

    #else
             U8 Channel;

    // check PAD_WAKEUP / PAD_INT status
    /*if (!(XBYTE[PM_PD] & (POWER_KEY_SEL == POWER_KEY_PAD_INT ? 0x20 : 0x10)))
    {
        *pkey = IRKEY_POWER;
        *pflag = 0;
        return TRUE;
    }*/
    for (Channel=0; Channel<ADC_KEY_CHANNEL_NUM; Channel++)
    {
        if (msKeypad_CH_GetKey(Channel, pkey, pflag))
        {
            return TRUE;
        }
    }
    return FALSE;

    #endif

}


#undef _MSKEYPAD_C_

#else
//static unsigned char code u8Dummy; // keep ?CO?MSKEYPAD
#endif
