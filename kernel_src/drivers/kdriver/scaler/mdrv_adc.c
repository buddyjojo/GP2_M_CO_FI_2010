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
/// file    drv_adc.c
/// @brief  ADC Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include "mdrv_types.h"
#include "mdrv_scaler_io.h"
#include "mdrv_scaler_st.h"
#include "mdrv_scaler.h"
#include "mdrv_adc.h"

#include "mhal_scaler.h"
#include "mhal_adc.h"
#include "mhal_adc_reg.h"
#include "mdrv_adctbl.c"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                        }\
                    } while (0)

#define ADCDBL(x)   //x
//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Exernal
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Init
//--------------------------------------------------------------------------------------------------
void MDrv_ADC_Init(void)
{
    TAB_Info Tab_info;
    Tab_info.pTable = (void*)MST_ADCINIT_TBL;
    Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_INIT_NUMS*REG_DATA_SIZE;
    Tab_info.u8TabRows = sizeof(MST_ADCINIT_TBL)/Tab_info.u8TabCols;
    Tab_info.u8TabIdx = 0;

    ADCDBL(printk("MDrv_ADC_Init\n"));
    MHal_ADC_LoadTable(&Tab_info);
}

void MDrv_ADC_SetSource(ADC_SOURCE_TYPE inputsrc_type) // daniel.huang 20090615
{
    TAB_Info Tab_info;
    Tab_info.pTable = (void*)MST_ADCSOURCE_TBL;
    Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_SOURCE_NUMS*REG_DATA_SIZE;
    Tab_info.u8TabRows = sizeof(MST_ADCSOURCE_TBL)/Tab_info.u8TabCols;
    Tab_info.u8TabIdx = inputsrc_type;

    ADCDBL(printk("MDrv_ADC_SetSource\n"));
    MHal_ADC_LoadTable(&Tab_info);
}

void MDrv_ADC_SetMux(ADC_MUX_TYPE ipmux_type)
{
    TAB_Info Tab_info;
    Tab_info.pTable = (void*)MST_ADCMUX_TBL;
    Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_MUX_NUMS*REG_DATA_SIZE;
    Tab_info.u8TabRows = sizeof(MST_ADCMUX_TBL)/Tab_info.u8TabCols;
    Tab_info.u8TabIdx = ipmux_type;

    ADCDBL(printk("MDrv_ADC_SetMux\n"));
    MHal_ADC_LoadTable(&Tab_info);
}

void MDrv_ADC_SetCVBSO(U8 u8PortNum, ADC_CVBSO_TYPE cvbso_type)
{
    TAB_Info Tab_info;
    if (u8PortNum == 1)
    {
        Tab_info.pTable = (void*)MST_ADCCVBSO_TBL1;
        Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_CVBSO_NUMS*REG_DATA_SIZE;
        Tab_info.u8TabRows = sizeof(MST_ADCCVBSO_TBL1)/Tab_info.u8TabCols;
        Tab_info.u8TabIdx = cvbso_type;

        ADCDBL(printk("MDrv_ADC_SetCVBSO\n"));
        MHal_ADC_LoadTable(&Tab_info);
    }
    else if (u8PortNum == 2)
    {
        Tab_info.pTable = (void*)MST_ADCCVBSO_TBL2;
        Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_CVBSO_NUMS*REG_DATA_SIZE;
        Tab_info.u8TabRows = sizeof(MST_ADCCVBSO_TBL2)/Tab_info.u8TabCols;
        Tab_info.u8TabIdx = cvbso_type;

        ADCDBL(printk("MDrv_ADC_SetCVBSO\n"));
        MHal_ADC_LoadTable(&Tab_info);
    }
    else
    {
        assert(0);
    }
}

void MDrv_ADC_SetCVBSO_MUX(U8 u8PortNum, ADC_CVBSO_MUX_TYPE cvbsomux_type)
{
    TAB_Info Tab_info;
    if (u8PortNum == 1)
    {
        Tab_info.pTable = (void*)MST_ADCCVBSO_MUX_TBL1;
        Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_CVBSO_MUX_NUMS*REG_DATA_SIZE;
        Tab_info.u8TabRows = sizeof(MST_ADCCVBSO_MUX_TBL1)/Tab_info.u8TabCols;
        Tab_info.u8TabIdx = cvbsomux_type;

        ADCDBL(printk("MDrv_ADC_SetCVBSO_MUX\n"));
        MHal_ADC_LoadTable(&Tab_info);
    }
    else if (u8PortNum == 2)
    {
        Tab_info.pTable = (void*)MST_ADCCVBSO_MUX_TBL2;
        Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_CVBSO_MUX_NUMS*REG_DATA_SIZE;
        Tab_info.u8TabRows = sizeof(MST_ADCCVBSO_MUX_TBL2)/Tab_info.u8TabCols;
        Tab_info.u8TabIdx = cvbsomux_type;

        ADCDBL(printk("MDrv_ADC_SetCVBSO_MUX\n"));
        MHal_ADC_LoadTable(&Tab_info);
    }
    else
    {
        assert(0);
    }
}


static U8 _SearchFreqSetTableIndex(U16 u16OriginalPixClk)
{
    U8  ClkIndex;
    ADCDBL(printk("SearchFreqTab: freq=%u\n", u16OriginalPixClk));

    for(ClkIndex=0; ClkIndex<sizeof(MST_ADC_FreqRange_TBL)/sizeof(ADC_FREQ_RANGE); ClkIndex++)
    {
        if((u16OriginalPixClk < MST_ADC_FreqRange_TBL[ClkIndex].FreqHLimit) &&
            (u16OriginalPixClk >= MST_ADC_FreqRange_TBL[ClkIndex].FreqLLimit))
            return ClkIndex;
    }
    return ClkIndex-1;
}

void MDrv_ADC_SetMode(BOOL bRGB, U16 u16PixelClk) // unit in MHz
{
    TAB_Info Tab_info;
    if (bRGB)
    {
        Tab_info.pTable = (void*)MST_ADCSetModeRGB_TBL;
        Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_SetMode_NUMS*REG_DATA_SIZE;
        Tab_info.u8TabRows = sizeof(MST_ADCSetModeRGB_TBL)/Tab_info.u8TabCols;
        Tab_info.u8TabIdx = _SearchFreqSetTableIndex(u16PixelClk);

        ADCDBL(printk("MDrv_ADC_SetMode: FreqIdx=%u\n", Tab_info.u8TabIdx));
        MHal_ADC_LoadTable(&Tab_info);
    }
    else
    {
        Tab_info.pTable = (void*)MST_ADCSetModeYUV_TBL;
        Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_SetMode_NUMS*REG_DATA_SIZE;
        Tab_info.u8TabRows = sizeof(MST_ADCSetModeYUV_TBL)/Tab_info.u8TabCols;
        Tab_info.u8TabIdx = _SearchFreqSetTableIndex(u16PixelClk);

        ADCDBL(printk("MDrv_ADC_SetMode: FreqIdx=%u\n", Tab_info.u8TabIdx));
        MHal_ADC_LoadTable(&Tab_info);
    }
}


void MDrv_ADC_ADCCal(BOOL bIsAVInput)   //20091012 daniel.huang: update adctbl to 0.26 to fix componenet position shift and color more white; and new mismatch cal settings
{
    TAB_Info Tab_info;

    if (!bIsAVInput)
    {
        Tab_info.pTable = (void*)MST_ADCAdcCal_TBL;
        Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_AdcCal_NUMS*REG_DATA_SIZE;
        Tab_info.u8TabRows = sizeof(MST_ADCAdcCal_TBL)/Tab_info.u8TabCols;
        Tab_info.u8TabIdx = ADC_TABLE_AdcCal_ALL;

        MHal_ADC_LoadTable(&Tab_info);
     }
     else
     {
         Tab_info.pTable = (void*)MST_ADCAdcCal_AV_TBL;
         Tab_info.u8TabCols = REG_ADDR_SIZE+REG_MASK_SIZE+ADC_TABLE_AdcCal_AV_NUMS*REG_DATA_SIZE;
         Tab_info.u8TabRows = sizeof(MST_ADCAdcCal_AV_TBL)/Tab_info.u8TabCols;
         Tab_info.u8TabIdx = ADC_TABLE_AdcCal_AV_ALL;
         
         MHal_ADC_LoadTable(&Tab_info);
     }
}

void MDrv_ADC_SetAnalogADC(PSC_SOURCE_INFO_t psrc)
{
    U8 u8PixClk = 0; // Add the initial value
    U16 u16HorizontalTotal;

    if (Use_VGA_Source(psrc->SrcType) || Use_YPbPr_Source(psrc->SrcType))
    {
        // Disable double sampling requested by LG
        // FitchHsu 20090605 patch Sherwood USA DVD player timing invalid format
        #if SC_DOUBLE_SAMPLING_ENABLE
		if (
        #if SC_YPBPR_720x480_60_SW_PATCH
		    psrc->Modesetting.u8ModeIndex == MD_720x480_60I_P ||
        #endif
		    psrc->Modesetting.u8ModeIndex == MD_720x480_60I   ||
            psrc->Modesetting.u8ModeIndex == MD_720x480_60P   ||
            psrc->Modesetting.u8ModeIndex == MD_720x576_50I   ||
            psrc->Modesetting.u8ModeIndex == MD_720x576_50P)
        {
            ADCDBL(printk("u16HorizontalTotal << 1\n"));
            u16HorizontalTotal = psrc->Modesetting.u16HorizontalTotal << 1;  // for better quality
        }
        else
        #endif
        {
            // LGE drmyung printk("u16HorizontalTotal\n");
            u16HorizontalTotal = psrc->Modesetting.u16HorizontalTotal;
        }
        if(u16HorizontalTotal > 3 && u16HorizontalTotal < 3500 && psrc->u16Input_HFreq >0 && psrc->bIsSupportMode) //20100217 youngkil.kim comp
        {
        u8PixClk = MDrv_SC_CalculatePixClk(u16HorizontalTotal);

#if 0
        printk("u8PixClk = %x\n", u8PixClk);
        printk("u16HorizontalTotal = %x\n", u16HorizontalTotal);
        printk("u8Phase = %x\n", psrc->Modesetting.u8Phase);
#endif

#if 1 	// swwoo LGE 080626 : 40보다 큰 값을 넘겨서 "Enable SOG input low bandwidth filter" 를 DISABLE 시켜야 선명하다.
        MDrv_ADC_SetMode(Use_VGA_Source(psrc->SrcType) ? TRUE:FALSE,
        (U16)((U32)u16HorizontalTotal*psrc->u16Input_HFreq/10000)); // (ykkim5 091110 480P 문제) unit in MHz
#else
        MHal_ADC_SetSogFilter(u8PixClk);
        MHal_ADC_SetADCPLL(u8PixClk);
#endif
        MHal_ADC_SetADCClk(u16HorizontalTotal);     // ADC horizontal total
        MHal_ADC_FreeRun_Enable(0);//20100217 comp problem
        MHal_ADC_SetADCPhase(psrc->Modesetting.u8Phase);  // setting ADC phase
        msleep(5); // 091109 ykkim5 pc RGB의 adc가 조금씩 틀어져서 color가 조금 redish 한 문제의 대책.

        //20091012 daniel.huang: update adctbl to 0.26 to fix componenet position shift and color more white; and new mismatch cal settings
        if (Use_VGA_Source(psrc->SrcType) || Use_YPbPr_Source(psrc->SrcType) || 
           (Use_SCART_Source(psrc->SrcType) && (MHal_SC_GetScartMode()==SCART_MODE_RGB)) )
        {
            MDrv_ADC_ADCCal(FALSE); // do mismatch calibration
        }
        else
        {
            MDrv_ADC_ADCCal(TRUE); // do mismatch calibration
        }
        }
else //20100217 comp problem
{
    MHal_ADC_FreeRun_Enable(1);
}
    }
}

// 20090903 daniel.huang: add/fix set/get gain/offset function
void MDrv_ADC_SetOffset(U8 u8Color, U16 u16Value)
{
    MHal_ADC_SetOffset(u8Color, u16Value);
}

// 20090903 daniel.huang: add/fix set/get gain/offset function
void MDrv_ADC_GetOffset(U8 u8Color, U16 *u16Value)
{
    MHal_ADC_GetOffset(u8Color, u16Value);
}

// 20090903 daniel.huang: add/fix set/get gain/offset function
void MDrv_ADC_SetGain(U8 u8Color, U16 u16Value)
{
    MHal_ADC_SetGain(u8Color, u16Value);
}

// 20090903 daniel.huang: add/fix set/get gain/offset function
void MDrv_ADC_GetGain(U8 u8Color, U16 *u16Value)
{
    MHal_ADC_GetGain(u8Color, u16Value);
}

#if (ENABLE_ADC_TEST==1)
void MDrv_ADC_Test(void) // for debug only
{
    U8 i;
    char SOURCE_TEXT[7][8]={
    "RGB",
    "YUV",
    "ATV",
    "SVIDEO",
    "SCART",
    "CVBS",
    "DVI",
    };

    char MUX_TEXT[][16]={
    "IPMUX_RGB0",
    "IPMUX_RGB1",
    "IPMUX_RGB2",
    "IPMUX_CVBSY0",
    "IPMUX_CVBSY1",
    "IPMUX_CVBSY2",
    "IPMUX_CVBSY3",
    "IPMUX_CVBSY4",
    "IPMUX_CVBSY5",
    "IPMUX_CVBSY6",
    "IPMUX_CVBSY7",
    "IPMUX_CVBSC0",
    "IPMUX_CVBSC1",
    "IPMUX_CVBSC2",
    "IPMUX_CVBSC3",
    "IPMUX_CVBSC4",
    "IPMUX_CVBSC5",
    "IPMUX_CVBSC6",
    "IPMUX_CVBSC7",
    "IPMUX_DVI0",
    "IPMUX_DVI1",
    "IPMUX_DVI2",
    };

    printk("//Init table\n");
    MDrv_ADC_Init();

    printk("//Source table\n");
    for (i=0; i<7; i++)
    {
        printk("//%s\n", SOURCE_TEXT[i]);
        MDrv_ADC_SetSource(i);
    }

    printk("//Mux table\n");
    for (i=0; i<ADC_TABLE_MUX_NUMS; i++)
    {
        printk("//%s\n", MUX_TEXT[i]);
        MDrv_ADC_SetMux(i);
    }

    printk("//BW table\n");
    MDrv_ADC_SetMode(TRUE, 5);
}
#endif

//20090814 Michu, ADC Calibration Testing
void MDrv_ADC_Calibration_Testing(void)
{
    MHal_ADC_Calibration_Testing();
}

