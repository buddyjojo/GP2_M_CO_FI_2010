#include <linux/delay.h>

#include "mdrv_types.h"     	// added to remove warning(dreamer@lge.com)
#include "hal_vd_settings.h"	// added to remove warning(dreamer@lge.com)
#include "hal_vd_hwreg.h"
#include "hal_vd_types.h"
#include "hal_vd_adc.h"


/******************************************************************************/
///This function power on ADC
///@param enADCPowerType \b IN: type of power mode
/******************************************************************************/
void MHal_VD_ADC_PowerOn(MS_ADC_POWER_ON_TYPE enADCPowerType)
{
    U8 u8ADC_Power_04L = ADC_POWERINIT_MSK_04_L;
    U8 u8ADC_Power_04H = ADC_POWERINIT_MSK_04_H;
    U8 u8ADC_Power_05L = ADC_POWERINIT_MSK_05_L;
    U8 u8ADC_Power_06L = ADC_POWERINIT_MSK_06_L;
    U8 u8ADC_Power_06H = ADC_POWERINIT_MSK_06_H;
    U8 u8ADC_Power_60L = ADC_POWERINIT_MSK_60_L;
    U8 u8ADC_Power_60H = ADC_POWERINIT_MSK_60_H;
    U8 u8ADC_Power_69L = ADC_POWERINIT_MSK_69_L;
    U8 u8ADC_Power_69H = ADC_POWERINIT_MSK_69_H;
    U8 u8ADC_Power_40L_6 = 1;

    switch (enADCPowerType)
    {
    case MS_ADC_A_POWER_ON:
        u8ADC_Power_04L |= (_BIT5|_BIT4|_BIT3|_BIT2|_BIT1);
        u8ADC_Power_06L |= (_BIT3|_BIT2|_BIT1);
        break;

    case MS_VDA_CVBS_POWER_ON:
        u8ADC_Power_04H |= (_BIT7|_BIT4|_BIT1);
        u8ADC_Power_05L |= (_BIT4|_BIT3|_BIT1);
        u8ADC_Power_06L |= (_BIT7|_BIT6);
        break;

    case MS_VDA_SV_POWER_ON:
        u8ADC_Power_04L |= (_BIT5);
        u8ADC_Power_04H |= (_BIT7|_BIT6|_BIT4|_BIT1);
        u8ADC_Power_05L |= (_BIT4|_BIT3|_BIT1|_BIT0);
        u8ADC_Power_06L |= (_BIT7|_BIT6);
        break;

    case MS_VDA_FBLANK_POWER_ON:
        u8ADC_Power_04L |= (_BIT5|_BIT4|_BIT3|_BIT2|_BIT1);
        u8ADC_Power_04H |= (_BIT7|_BIT4|_BIT1);
        u8ADC_Power_05L |= (_BIT4|_BIT3|_BIT1);
        u8ADC_Power_06L |= (_BIT7|_BIT6|_BIT3|_BIT2|_BIT1);
        u8ADC_Power_06H |= (_BIT6);
        u8ADC_Power_40L_6 = 0;
        break;

    case MS_DVI_POWER_ON:
        u8ADC_Power_06H |= (_BIT7|_BIT5|_BIT3|_BIT1|_BIT0);
        u8ADC_Power_60L |= (_BIT7|_BIT5|_BIT3|_BIT2|_BIT1|_BIT0);
        u8ADC_Power_60H |= (_BIT6|_BIT5|_BIT4|_BIT3|_BIT2|_BIT1|_BIT0);
        u8ADC_Power_69L |= (_BIT7|_BIT5|_BIT3|_BIT2|_BIT1|_BIT0);
        u8ADC_Power_69H |= (_BIT6|_BIT5|_BIT4|_BIT3|_BIT2|_BIT1|_BIT0);
        break;

    case MS_ADC_VD_BLEND_POWER_ON:
        u8ADC_Power_04L |= (_BIT5|_BIT4|_BIT3|_BIT2|_BIT1);
        u8ADC_Power_04H |= (_BIT7|_BIT4|_BIT1);
        u8ADC_Power_05L |= (_BIT4|_BIT3|_BIT1);
        u8ADC_Power_06L |= (_BIT7|_BIT6|_BIT3|_BIT2|_BIT1);
        u8ADC_Power_06H |= (_BIT6);
        u8ADC_Power_40L_6 = 0;
        break;

     default:
        break;
    }

    _MHal_VD_W1B(L_BK_ADC_ATOP(0x04), (0xFF & ~(u8ADC_Power_04L)));
    _MHal_VD_W1B(H_BK_ADC_ATOP(0x04), (0xFF & ~(u8ADC_Power_04H)));
    _MHal_VD_W1B(L_BK_ADC_ATOP(0x05), (0xFF & ~(u8ADC_Power_05L)));
    _MHal_VD_W1B(L_BK_ADC_ATOP(0x06), (0xFF & ~(u8ADC_Power_06L)));
    _MHal_VD_W1B(H_BK_ADC_ATOP(0x06), (0xFF & ~(u8ADC_Power_06H)));
    _MHal_VD_W1B(L_BK_ADC_ATOP(0x60), (0xFF & ~(u8ADC_Power_60L)));
    _MHal_VD_W1B(H_BK_ADC_ATOP(0x60), (0xFF & ~(u8ADC_Power_60H)));
    _MHal_VD_W1B(L_BK_ADC_ATOP(0x69), (0xFF & ~(u8ADC_Power_69L)));
    _MHal_VD_W1B(H_BK_ADC_ATOP(0x69), (0xFF & ~(u8ADC_Power_69H)));
    _MHal_VD_W1BM(L_BK_ADC_ATOP(0x40), u8ADC_Power_40L_6, _BIT6);
}


/******************************************************************************/
///This function sets ADC MUX and return ADC power type
/******************************************************************************/
#if 0
MS_ADC_POWER_ON_TYPE MDrv_VD_ADC_SetMUX(INPUT_PORT_TYPE enInputPortType, U8 u8VDYMux, U8 u8VDCMux)
{
    /* always open CVBS ADC */
    MS_ADC_POWER_ON_TYPE enADCPowerType = MS_ADC_POWER_ALL_OFF;

    U8 u8Src_En = 0;

    u8Src_En |= En_VD;
    u8Src_En &= ~En_ADC_AMUX;
    enADCPowerType = MS_VDA_CVBS_POWER_ON;

    u8VDCMux &= 0x0F;
    u8VDYMux &= 0x0F;

    _MHal_VD_W1BM(L_BK_ADC_ATOP(0x02), u8VDYMux, ADC_YMUX_MASK);
    _MHal_VD_W1BM(L_BK_ADC_ATOP(0x02), (u8VDCMux<<4), ADC_CMUX_MASK);
    _MHal_VD_W1B(L_BK_ADC_ATOP(0x00), u8Src_En);

    return enADCPowerType;
}
#endif

MS_ADC_POWER_ON_TYPE MHal_VD_ADC_SetMUX(INPUT_PORT_TYPE enInputPortType, U8 u8VDYMux, U8 u8VDCMux)
{
    MS_ADC_POWER_ON_TYPE enADCPowerType = MS_ADC_POWER_ALL_OFF;

    U8 u8Src_En = 0;

    if(IsUseInternalScartPort(enInputPortType))
    {
        u8Src_En |= (En_VD|En_FB_RGB|EN_ADC_FB);
        enADCPowerType = MS_VDA_FBLANK_POWER_ON;
    }
    else if(IsUseInternalSVPort(enInputPortType))
    {
        u8Src_En |= En_VD|En_VD_YC|En_FB_RGB;
        u8Src_En &= ~En_ADC_AMUX;
        enADCPowerType = MS_VDA_SV_POWER_ON;
    }
    else if(IsUseInternalAVPort(enInputPortType))
    {
        u8Src_En |= En_VD;
        u8Src_En &= ~En_ADC_AMUX;
        enADCPowerType = MS_VDA_CVBS_POWER_ON;
    }
    else
    {
        u8Src_En |= En_VD;
        u8Src_En &= ~En_ADC_AMUX;
        enADCPowerType = MS_VDA_CVBS_POWER_ON;
    }

    u8VDCMux &= 0x0F;
    u8VDYMux &= 0x0F;

    if(IsUseInternalScartPort(enInputPortType))
    {
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x01), u8VDCMux,_BIT0|_BIT1); // select RGB channel
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x02), u8VDYMux, ADC_YMUX_MASK); // ADC_VD_YMUX_MASK  for CVBS
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x02), (u8VDCMux<<4), ADC_CMUX_MASK); // ADC_VD_CMUX_MASK
    }
    else if(IsUseInternalSVPort(enInputPortType))
    {
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x02), u8VDYMux, ADC_YMUX_MASK);
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x02), (u8VDCMux << 4), ADC_CMUX_MASK);
    }
    else if(IsUseInternalAVPort(enInputPortType))
    {
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x02), u8VDYMux, ADC_YMUX_MASK);
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x02), (0x0F << 4), ADC_CMUX_MASK);
    }

    _MHal_VD_W1B(L_BK_ADC_ATOP(0x00), u8Src_En);

    return enADCPowerType;
}



B16 MHal_VD_ADC_GetScart1IDLevel(U32 *pu32Scart1IDLevel)
{
	_MHal_VD_W1B(L_BK_ADC_ATOP(0x4C), 0x64); //Channel 2

    msleep(1);

    if ( _MHal_VD_R1Rb(L_BK_ADC_ATOP(0x4C), BIT7) )    // To check one-shot mode done status
    {
		*pu32Scart1IDLevel = (_MHal_VD_R1B(L_BK_ADC_ATOP(0x4E)) & SAR_ADC_CHANNEL_DATA_MASK);    //return updated value for channel 2
        return 0x00;
    }
    else
    {
        return 0x01;
    }
}


U32 MHal_VD_ADC_GetScart2IDLevel(void)
{
    return _MHal_VD_R1B(BYTE2REAL(0x3A1C)) & 0x3F;
}

