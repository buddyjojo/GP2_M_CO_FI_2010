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


//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <linux/delay.h>
#include "mdrv_types.h"
#include "mdrv_adctbl.h"
#include "mhal_scaler.h"
#include "mhal_adc_reg.h"
#include "mhal_auto.h"		// added to remove warning(dreamer@lge.com)
#include "mhal_adc.h"
#include "mhal_scaler_reg.h"
#include "mhal_utility.h"

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures Start
//-------------------------------------------------------------------------------------------------
extern unsigned long irq_flags_swbk;
extern spinlock_t switch_bk_lock;
extern U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------
#define ADC_ENABLE_CHECK        0

#define ADC_FUNC_LOAD_REG       0
#define ADC_FUNC_CHK_REG        1

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef unsigned long long U64;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Constant
//-------------------------------------------------------------------------------------------------
// cc.chen - T.B.D.
#define MST_XTAL_CLOCK_KHZ 12000

#define  ADC_POWERINIT_MSK_04   (BIT8 | BIT7 | BIT6 | BIT0)
#define  ADC_POWERINIT_MSK_05   (BIT6 | BIT5 | BIT2)
#define  ADC_POWERINIT_MSK_06   (BIT12 | BIT10 | BIT0)
#define  ADC_POWERINIT_MSK_08   (BIT4)
#define  ADC_POWERINIT_MSK_60   (BIT6 | BIT4)
#define  ADC_POWERINIT_MSK_69   (BIT6 | BIT4)

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
#define REG_ADC_MAP(x)          (0x100000+(x))
#define REG_ADC_DTOPB_MAP(x)    (0x110000+(x))

static BOOL _bEnDbg = FALSE;
#define ADC_DBG(x) \
do{                \
    if (_bEnDbg){  \
        x;         \
    }              \
}while(0)


#if (ADC_ENABLE_CHECK == 0)
#define ADC_REG_FUNC( u32Reg, u8Value, u8Mask )    MDrv_WriteByteMask( u32Reg, u8Value, u8Mask )

#else // #if(ADC_ENABLE_CHECK == 1)

static U8 _u8ADCRegFunc = ADC_FUNC_LOAD_REG;

static void _MDrv_ADC_SetRegFunction(U8 u8Func)
{
    _u8ADCRegFunc = u8Func;
}

#define ADC_REG_FUNC( u16Reg, u8Value, u8Mask ) \
    do{ \
        if (_u8ADCRegFunc == ADC_FUNC_LOAD_REG){ \
            MDrv_WriteByteMask( u16Reg, u8Value, u8Mask ); \
        }else{ \
            if ((MDrv_ReadByte(u16Reg) & u8Mask) != (u8Value & u8Mask)){ \
                printf("[RegDiff]addr=%04x, mask=%02x, val=%02x[%02x]\n", \
                    u16Reg, (U16)u8Mask, (U16)MDrv_ReadByte(u16Reg), (U16)u8Value); \
            } \
        } \
    }while(0)
#endif  //#if (ADC_ENABLE_CHECK)

void MHal_ADC_LoadTable(TAB_Info* pTab_info)
{
    U32 u32Addr;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;
    U8 u8Flag;
    U8 u8HiByte;
    U16 i;

    if (pTab_info->pTable == NULL)
        return;

    if (REG_ADDR_SIZE+REG_MASK_SIZE+pTab_info->u8TabIdx*REG_DATA_SIZE >= pTab_info->u8TabCols){
        ADC_DBG(printk("Tab_info error: tabIdx:%u, TabCols:%u\n", pTab_info->u8TabIdx, pTab_info->u8TabCols));
        return;
    }

    for (i=0; i<pTab_info->u8TabRows-1; i++) // 20090618 daniel.huang: add -1 to skip TABLE_END
    {
        u16Addr = (pTab_info->pTable[0]<<8) + pTab_info->pTable[1];
        u8Mask  = pTab_info->pTable[2];
        u8Value = pTab_info->pTable[REG_ADDR_SIZE+REG_MASK_SIZE+pTab_info->u8TabIdx*REG_DATA_SIZE+1];
        u8Flag = pTab_info->pTable[REG_ADDR_SIZE+REG_MASK_SIZE+pTab_info->u8TabIdx*REG_DATA_SIZE];
#if 1
        ADC_DBG(printk("[addr=%04x, msk=%02x, val=%02x %s]\n", u16Addr, u8Mask, u8Value, u8Flag ? "skip":""));
#else
        ADC_DBG(printk("wriu 0x10%04x 0x%02x\n", u16Addr, u8Value));
#endif
        pTab_info->pTable+=pTab_info->u8TabCols; // next

        if (u8Flag & BIT0) // skip
            continue;

        u8HiByte = u16Addr & 0x1;
        u16Addr = u16Addr & ~0x1;

        if (u16Addr == REG_ADC_DTOPB_FE_L)
        {  // do delay
            msleep(u8Value);
            continue;
        }
        else if ((u16Addr >> 8) == 0x12)
        {
            u32Addr = REG_ADC_DTOPB_MAP(u16Addr);
        }
        else
        {
            u32Addr = REG_ADC_MAP(u16Addr);
        }

        if (u8HiByte)
            REG_WM(u32Addr, ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
        else
            REG_WM(u32Addr, (U16)u8Value, (U16)u8Mask);

    }
}

/******************************************************************************/
///This function sets input HSync polarity
///@param u8Value \b IN:
/******************************************************************************/
void MHal_ADC_SetInputHsyncPolarity(BOOL bHightActive)
{
    REG_WI(REG_ADC_DTOP(0x07), bHightActive, BIT7);
}

/******************************************************************************/
///This function will set ADC offset
///@param *pstADCSetting \b IN: pointer to ADC settings
/******************************************************************************/
// 20090903 daniel.huang: add/fix set/get gain/offset function
void MHal_ADC_SetOffset(U8 u8Color, U16 u16Value)
{
    U8 u8Addr;
    switch(u8Color)
    {
        case ADC_COLOR_RED:
            u8Addr = 0x52;
            break;
        case ADC_COLOR_GREEN:
            u8Addr = 0x57;
            break;
        case ADC_COLOR_BLUE:
        default:
            u8Addr = 0x5C;
            break;
    }
    REG_WM( REG_ADC_DTOP(u8Addr), u16Value, 0x1FFF);    // [12:0]
}
/******************************************************************************/
///This function will get ADC offset
/******************************************************************************/
// 20090903 daniel.huang: add/fix set/get gain/offset function
void MHal_ADC_GetOffset(U8 u8Color, U16 *u16Value)
{
    U8 u8Addr;
    switch(u8Color)
    {
        case ADC_COLOR_RED:
            u8Addr = 0x52;
            break;
        case ADC_COLOR_GREEN:
            u8Addr = 0x57;
            break;
        case ADC_COLOR_BLUE:
        default:
            u8Addr = 0x5C;
            break;
    }
    *u16Value = REG_RR( REG_ADC_DTOP(u8Addr)) & 0x1FFF;    // [12:0]
}

/******************************************************************************/
///This function sets ADC gain
/******************************************************************************/

void MHal_ADC_SetGain(U8 u8Color, U16 u16Value)
{
    U8 u8Addr;
    switch(u8Color)
    {
        case ADC_COLOR_RED:
            u8Addr = 0x51;
            break;
        case ADC_COLOR_GREEN:
            u8Addr = 0x56;
            break;
        case ADC_COLOR_BLUE:
        default:
            u8Addr = 0x5B;
            break;
    }
    // 20090814 daniel.huang: T3 gain larger, value larger
    REG_WM(REG_ADC_DTOP(u8Addr), u16Value, 0x3FFF);     // [13:0]
}

/******************************************************************************/
///This function get ADC gain
/******************************************************************************/
// 20090903 daniel.huang: add/fix set/get gain/offset function
void MHal_ADC_GetGain(U8 u8Color, U16 *u16Value)
{
    U8 u8Addr;
    switch(u8Color)
    {
        case ADC_COLOR_RED:
            u8Addr = 0x51;
            break;
        case ADC_COLOR_GREEN:
            u8Addr = 0x56;
            break;
        case ADC_COLOR_BLUE:
        default:
            u8Addr = 0x5B;
            break;
    }
    *u16Value = REG_RR(REG_ADC_DTOP(u8Addr)) & 0x3FFF;     // [13:0]
}


void MHal_ADC_SetSOGThreshold(U8 u8Threshold)   //20090924 daniel.huang
{
#if 1   // T3
    REG_WL(REG_ADC_ATOP(0x3B), u8Threshold);
#else   // T2
    REG_WM(REG_ADC_ATOP(0x1C), u8Threshold, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0));
#endif
    }


/******************************************************************************/
///This function sets PLL clock divider ratio
///@param u16Value \b IN: PLL clock divider ratio
// 091109 ykkim5  pc의 redish한 문제 대책.
/******************************************************************************/
void MHal_ADC_SetADCClk(U16 u16Value)
{
    U16 u16timeout = 300;
    if(u16Value >3 && u16Value <3500) //20100217 comp problem.
    {
        u16Value -= 3; // actual - 3
        REG_WR(REG_ADC_DTOP(0x00), u16Value);
        while(u16timeout)
        {
            if(REG_RI(REG_ADC_DTOP(0x05), BIT7))
            {
                ADC_DBG(printk("ADC PLL clock stable\n"));
                break;
            }
            else
            {
                udelay(10);
            }
            u16timeout--;
        }
    }
    ADC_DBG(printk("ADC PLL u16timeout = %d\n",u16timeout));
}

/******************************************************************************/
///This function sets PLL phase
///@param u8Value \b IN: PLL phase divider ratio
/******************************************************************************/
void MHal_ADC_SetADCPhase(U8 u8Value)
{
//  REG_WL(REG_ADC_DTOP(0x03), u8Value); // phase [5:0] in T2
    REG_WL(REG_ADC_ATOP(0x15), u8Value); // rphase [6:0], chg in T3
    REG_WL(REG_ADC_ATOP(0x16), u8Value); // gphase [6:0], chg in T3
    REG_WL(REG_ADC_ATOP(0x17), u8Value); // bphase [6:0], chg in T3
    REG_WL(REG_ADC_ATOP(0x18), u8Value); // yphase [6:0], chg in T3
}

//
// T3 change to 1.05V & 0.55V
//
void MHal_ADC_InternalDc( U8 InternalVoltage )
{//20090814 Michu //LDO
    U16 u16regvalue;
     switch ( InternalVoltage )
     {
        case INTERNAL_0_55V: //!! Mapping to T3 0.55V
             u16regvalue = 0x2;
            break;

        case INTERNAL_1_05V: //!! Mapping to T3 1.05V
             u16regvalue = 0x3;
            break;

        case INTERNAL_NONE:
        default:
            REG_WM(REG_ADC_ATOP(0x5A), 0x00 , BIT0);
            // LDO VCal disable
            REG_WM(REG_ADC_DTOPB(0x05), 0x00 , (BIT0|BIT1|BIT2|BIT3));

            REG_WM(REG_ADC_DTOPB(0x04), 0x00 , (BIT10|BIT11|BIT12));

            REG_WM(REG_ADC_DTOPB(0x05), 0x00 , (BIT8|BIT9|BIT10|BIT11|BIT12));
            return;
            break;
     }

    REG_WM(REG_ADC_DTOPB(0x04), (BIT10|BIT11|BIT12) , (BIT10|BIT11|BIT12));

    REG_WM(REG_ADC_DTOPB(0x05), (BIT8 | BIT12) , (BIT8 | BIT12));

    REG_WM(REG_ADC_ATOP(0x5A), BIT0 , BIT0);

    // LDO VCal enable
    REG_WM(REG_ADC_DTOPB(0x05), (BIT2|BIT3) , (BIT2|BIT3));

    //Ref Voltage - ADCA VCAL force value
    REG_WM(REG_ADC_DTOPB(0x05), u16regvalue , (BIT0|BIT1));
}

//victor 20080830
U16 MHal_ADC_GetAutoGainValue(U8 channel)
{
    U16 u16GainValue;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);

    REG_L_WM(REG_SC_IP1F2(0x0f) , channel , 0x0F);
    u16GainValue = REG_RR(REG_SC_IP1F2(0x11) );
    SC_BK_RESTORE;

    return u16GainValue;
}

//victor 20080830
void MHal_ADC_AutoGainEnable(BOOL bEnable)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    REG_WI(REG_SC_IP1F2(0x0E), bEnable, BIT0 );
    SC_BK_RESTORE;
}

//victor 20080830
BOOL MHal_ADC_AutoGainDataReady(void)
{
    BOOL isReady;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    isReady = REG_RL(REG_SC_IP1F2(0x0E)) & BIT1;
    SC_BK_RESTORE;

    return isReady;
}

//victor 20081024, I refined this function
// Adjust calibration window to prevent to get garbage value
// Fitch 2009 0110 Auto config problem
void MHal_ADC_Adjuect_Calibration_Window(BOOL bEnable)
{
    //set calibrartin window
    U16 Vcut;
    U16 Hcut;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);

    Vcut = REG_RR(REG_SC_IP1F2(0x06)) / 10;
    Hcut = REG_RR(REG_SC_IP1F2(0x07)) / 10;

    REG_WI(REG_SC_IP1F2(0x29), bEnable, BIT8);

    REG_WR(REG_SC_IP1F2(0x2A), (REG_RR(REG_SC_IP1F2(0x04)) + (Vcut/2))  );  //vstart
    REG_WR(REG_SC_IP1F2(0x2B), (REG_RR(REG_SC_IP1F2(0x05)) + (Hcut/2))  );  //hstart
    REG_WR(REG_SC_IP1F2(0x2C), (REG_RR(REG_SC_IP1F2(0x06)) - Vcut)  );  //vsize
    REG_WR(REG_SC_IP1F2(0x2D), (REG_RR(REG_SC_IP1F2(0x07)) - Hcut)  );  //hsize

    SC_BK_RESTORE;
}


//20090814 Michu, ADC Calibration Testing
void MHal_ADC_Calibration_Testing(void)
{
    U32 x;
    SC_BK_STORE;

    SC_BK_SWICH(REG_SC_BK_DLC);
    REG_WI(REG_SC_DLC(0x68), ENABLE, BIT0);
    SC_BK_RESTORE;
    for(x=0x50; x<0x800; x=x+0x100)
    {
        U16 u16Y, u16Cb, u16Cr;

        ADC_DBG(printk("(x, y) = (0x%x, 0x200)\n", x));
        SC_BK_STORE;
        SC_BK_SWICH(REG_SC_BK_DLC);
        REG_WR(REG_SC_DLC(0x69), x);
        REG_WR(REG_SC_DLC(0x6A), 0x200);
        SC_BK_RESTORE;

        OS_Delayms(10);
        SC_BK_STORE;
        SC_BK_SWICH(REG_SC_BK_DLC);
        u16Y = (REG_RR(REG_SC_DLC(0x6B)) & 0x03FF);
        u16Cb = (REG_RR(REG_SC_DLC(0x6C)) & 0x03FF);
        u16Cr = (REG_RR(REG_SC_DLC(0x6D)) & 0x03FF);
        SC_BK_RESTORE;

        ADC_DBG(printk("Y = 0x%04x, Cb = 0x%04x, Cr = 0x%04x\n", u16Y, u16Cb, u16Cr));
    }
    // Disable Debug Cross
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_DLC);
    REG_WI(REG_SC_DLC(0x68), ENABLE, BIT0);
    SC_BK_RESTORE;

}

void MHal_ADC_FreeRun_Enable(BOOL bEnable)
{

    if(bEnable)
    {
        REG_WR(REG_ADC_DTOP(0x01), 0x6656);
        REG_WR(REG_ADC_DTOP(0x02), 0x0066);
        REG_WM(REG_ADC_DTOP(0x06), 0x0080, 0x0080);  // free run enable
        REG_WM(REG_ADC_ATOP(0x09), 0x00, BIT12 | BIT11); // VCO Div 8 mode
        REG_WM(REG_ADC_ATOP(0x0C), BIT0, BIT2 | BIT1 | BIT0); // PLL clock multipler
    }
    else
    {
        msleep(1);
        REG_WM(REG_ADC_DTOP(0x06), 0x00, 0x0080);  // free run Disable
        REG_WR(REG_ADC_DTOP(0x01), 0x0982);
        REG_WR(REG_ADC_DTOP(0x02), 0x0005);
    }
}

