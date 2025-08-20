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
/// file    mdrv_mace.c
/// @brief  Advance Color Engine Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/errno.h>
#include <linux/string.h>
#include <asm/uaccess.h>

#include "mdrv_types.h"
#include "mdrv_scaler_st.h"
#include "mdrv_scaler.h"
#include "mhal_scaler.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define CONFIG_MACE_DBG 0

#if CONFIG_MACE_DBG
#define MACE_PRINT(fmt, args...)  printk("[MACE][%06d]     " fmt, __LINE__, ## args)
#else
#define MACE_PRINT(fmt, args...)
#endif

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------

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
//  Utility
//--------------------------------------------------------------------------------------------------
U32 MACE_sine_data[512] = {
    0,   100,   201,   301,   402,   502,   603,   703,   804,   904,  1005,  1105,  1206,  1306,  1407,  1507,
 1607,  1708,  1808,  1909,  2009,  2109,  2210,  2310,  2410,  2510,  2611,  2711,  2811,  2911,  3011,  3111,
 3211,  3311,  3411,  3511,  3611,  3711,  3811,  3911,  4011,  4110,  4210,  4310,  4409,  4509,  4609,  4708,
 4808,  4907,  5006,  5106,  5205,  5304,  5403,  5503,  5602,  5701,  5800,  5898,  5997,  6096,  6195,  6294,
 6392,  6491,  6589,  6688,  6786,  6884,  6983,  7081,  7179,  7277,  7375,  7473,  7571,  7669,  7766,  7864,
 7961,  8059,  8156,  8254,  8351,  8448,  8545,  8642,  8739,  8836,  8933,  9029,  9126,  9223,  9319,  9415,
 9512,  9608,  9704,  9800,  9896,  9991, 10087, 10183, 10278, 10374, 10469, 10564, 10659, 10754, 10849, 10944,
11039, 11133, 11228, 11322, 11416, 11511, 11605, 11699, 11793, 11886, 11980, 12073, 12167, 12260, 12353, 12446,
12539, 12632, 12725, 12817, 12910, 13002, 13094, 13186, 13278, 13370, 13462, 13554, 13645, 13736, 13828, 13919,
14010, 14100, 14191, 14282, 14372, 14462, 14552, 14642, 14732, 14822, 14912, 15001, 15090, 15180, 15269, 15357,
15446, 15535, 15623, 15712, 15800, 15888, 15976, 16063, 16151, 16238, 16325, 16413, 16499, 16586, 16673, 16759,
16846, 16932, 17018, 17104, 17189, 17275, 17360, 17445, 17530, 17615, 17700, 17784, 17869, 17953, 18037, 18121,
18204, 18288, 18371, 18454, 18537, 18620, 18703, 18785, 18868, 18950, 19032, 19113, 19195, 19276, 19358, 19439,
19519, 19600, 19681, 19761, 19841, 19921, 20001, 20080, 20159, 20239, 20318, 20396, 20475, 20553, 20631, 20709,
20787, 20865, 20942, 21020, 21097, 21173, 21250, 21326, 21403, 21479, 21555, 21630, 21706, 21781, 21856, 21931,
22005, 22080, 22154, 22228, 22301, 22375, 22448, 22521, 22594, 22667, 22740, 22812, 22884, 22956, 23027, 23099,
23170, 23241, 23312, 23382, 23453, 23523, 23593, 23662, 23732, 23801, 23870, 23939, 24007, 24075, 24144, 24211,
24279, 24346, 24414, 24480, 24547, 24614, 24680, 24746, 24812, 24877, 24943, 25008, 25073, 25137, 25201, 25266,
25330, 25393, 25457, 25520, 25583, 25645, 25708, 25770, 25832, 25894, 25955, 26016, 26077, 26138, 26199, 26259,
26319, 26379, 26438, 26498, 26557, 26615, 26674, 26732, 26790, 26848, 26905, 26963, 27020, 27076, 27133, 27189,
27245, 27301, 27356, 27411, 27466, 27521, 27576, 27630, 27684, 27737, 27791, 27844, 27897, 27949, 28002, 28054,
28106, 28157, 28208, 28259, 28310, 28361, 28411, 28461, 28511, 28560, 28609, 28658, 28707, 28755, 28803, 28851,
28898, 28946, 28993, 29039, 29086, 29132, 29178, 29223, 29269, 29314, 29359, 29403, 29447, 29491, 29535, 29578,
29621, 29664, 29707, 29749, 29791, 29833, 29874, 29915, 29956, 29997, 30037, 30077, 30117, 30156, 30196, 30235,
30273, 30312, 30350, 30387, 30425, 30462, 30499, 30535, 30572, 30608, 30644, 30679, 30714, 30749, 30784, 30818,
30852, 30886, 30919, 30952, 30985, 31018, 31050, 31082, 31114, 31145, 31176, 31207, 31237, 31268, 31298, 31327,
31357, 31386, 31414, 31443, 31471, 31499, 31526, 31554, 31581, 31607, 31634, 31660, 31685, 31711, 31736, 31761,
31785, 31810, 31834, 31857, 31881, 31904, 31927, 31949, 31971, 31993, 32015, 32036, 32057, 32078, 32098, 32118,
32138, 32157, 32176, 32195, 32214, 32232, 32250, 32268, 32285, 32302, 32319, 32335, 32351, 32367, 32383, 32398,
32413, 32427, 32442, 32456, 32469, 32483, 32496, 32509, 32521, 32533, 32545, 32557, 32568, 32579, 32589, 32600,
32610, 32619, 32629, 32638, 32647, 32655, 32663, 32671, 32679, 32686, 32693, 32700, 32706, 32712, 32718, 32723,
32728, 32733, 32737, 32741, 32745, 32749, 32752, 32755, 32758, 32760, 32762, 32764, 32765, 32766, 32767, 32768 };

#define MACE_sine(a)        ( a > 511 ? MACE_sine_data[511] : MACE_sine_data[a]     )
#define MACE_cosine(a)      ( a > 511 ? MACE_sine_data[0]   : MACE_sine_data[511-a] )

extern SC_HISTOGRAM_INFO_t g_HistogramData; //thchen 20080820


//------------------------------------------------------------------------------
//  Declaration
//------------------------------------------------------------------------------
void MDrv_SC_MACE_InitCtx(PMACE_INFO_t pMACE_Info);
void MDrv_SC_MACE_SetVideoContrastMatrix(PMACE_INFO_t pMACE_Info);
void MDrv_SC_MACE_SetVideoColorMatrix(PMACE_INFO_t pMACE_Info);
void MDrv_SC_MACE_SetVideoSatHueMatrix(PMACE_INFO_t pMACE_Info);

//------------------------------------------------------------------------------
//  Implementation
//------------------------------------------------------------------------------
void MDrv_SC_MACE_Init(PSC_DRIVER_CONTEXT_t pDrvCtx)
{
    PMACE_INFO_t pMACE_Info;
    pMACE_Info = &pDrvCtx->SrcInfo[SC_MAIN_WINDOW].MACE_Info;
    MDrv_SC_MACE_InitCtx(pMACE_Info);
}

void MDrv_SC_MACE_HistogramData_Init(PSC_DRIVER_CONTEXT_t pDrvCtx)
{
    u8 i;
    SC_HISTOGRAM_INFO_t HistogramData;
    HistogramData = pDrvCtx->SrcInfo[SC_MAIN_WINDOW].HistogramData;
    for (i=0 ;i<=31 ;i++)
    {
        HistogramData.u16Histogram32[i] = 0;
    }
    HistogramData.bDataReady = FALSE ;
    HistogramData.u8AvgPixelValue = 0;
    HistogramData.u8MaxPixelValue = 0;
    HistogramData.u8MinPixelValue = 0;

    HistogramData.u16TotalColorCount = 0;//[090601_Leo]

}
void MDrv_SC_MACE_SetYUV2RGBMatrix(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_YUV2RGB_MTX_t param;
    PMACE_INFO_t pMACE_Info;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_YUV2RGB_MTX_t)))
    {
        return;
    }

    pMACE_Info = &pDrvCtx->SrcInfo[param.srcIdx].MACE_Info;
    if (copy_from_user(&pMACE_Info->YUV2RGB_Matrix, (void __user*)param.pYUV2RGB_Matrix, sizeof(pMACE_Info->YUV2RGB_Matrix)))
    {
        return;
    }

	MDrv_SC_MACE_SetVideoContrastMatrix(pMACE_Info);
	MDrv_SC_MACE_SetVideoColorMatrix(pMACE_Info);
}

void MDrv_SC_MACE_SetContrast(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_CONTRAST_t param;
    PMACE_INFO_t pMACE_Info;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CONTRAST_t)))
    {
        return;
    }

    pMACE_Info = &pDrvCtx->SrcInfo[param.srcIdx].MACE_Info;
    pMACE_Info->u8Contrast = param.u8Contrast;

    MDrv_SC_MACE_SetVideoContrastMatrix(pMACE_Info);
    MDrv_SC_MACE_SetVideoColorMatrix(pMACE_Info);
}

void MDrv_SC_MACE_SetSaturation(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_SATURATION_t param;
    PMACE_INFO_t pMACE_Info;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_SATURATION_t)))
    {
        return;
    }

    pMACE_Info = &pDrvCtx->SrcInfo[param.srcIdx].MACE_Info;
    pMACE_Info->u8Saturation = param.u8Saturation;

    MDrv_SC_MACE_SetVideoSatHueMatrix(pMACE_Info);
    MDrv_SC_MACE_SetVideoColorMatrix(pMACE_Info);
}

void MDrv_SC_MACE_SetHue(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_HUE_t param;
    PMACE_INFO_t pMACE_Info;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_HUE_t)))
    {
        return;
    }

    pMACE_Info = &pDrvCtx->SrcInfo[param.srcIdx].MACE_Info;
    pMACE_Info->u8Hue = param.u8Hue;

    MDrv_SC_MACE_SetVideoSatHueMatrix(pMACE_Info);
    MDrv_SC_MACE_SetVideoColorMatrix(pMACE_Info);
}
void MDrv_SC_MACE_SetRGBEx(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_RGB_EX_t param;
    PMACE_INFO_t pMACE_Info;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_RGB_EX_t)))
    {
        return;
    }

    pMACE_Info = &pDrvCtx->SrcInfo[param.srcIdx].MACE_Info;
    pMACE_Info->u16RConEx = param.u16Red;
    pMACE_Info->u16GConEx = param.u16Green;
    pMACE_Info->u16BConEx = param.u16Blue;

    MACE_PRINT("SetRGBEx Red=0x%x, Green=0x%x, Blue=0x%x\n", pMACE_Info->u16RConEx, pMACE_Info->u16GConEx, pMACE_Info->u16BConEx);

    MDrv_SC_MACE_SetVideoContrastMatrix(pMACE_Info);
    MDrv_SC_MACE_SetVideoColorMatrix(pMACE_Info);
}
#if 1	//thchen 20080820
void MDrv_SC_MACE_GetHistogramInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_HISTOGRAM_INFO_t info;
    U8 i;
    if (copy_from_user(&info, (void __user *)arg, sizeof(SC_GET_HISTOGRAM_INFO_t)))
    {
        return;
    }

#if 1//def CONFIG_Titania2 //Histogram data should be updated from vsync ISR, [090915_Leo]
    for (i=0 ; i<=31; i++)
    {
        info.u16Histogram32[i] = g_HistogramData.u16Histogram32[i];
    }
    info.u8MaxPixelValue = g_HistogramData.u8MaxPixelValue;
    info.u8MinPixelValue = g_HistogramData.u8MinPixelValue;;
    info.u8AvgPixelValue = g_HistogramData.u8AvgPixelValue;;
    info.u16TotalColorCount = g_HistogramData.u16TotalColorCount;//[090601_Leo]
    info.bDataReady = 1;
#else
    info.bDataReady = MHal_SC_MACE_WaitHistogramDataReady();
    HistogramData = pDrvCtx->SrcInfo[info.srcIdx].HistogramData;
    if (info.bDataReady == TRUE)
    {
        MHal_SC_MACE_GetHistogram32(&info.u16Histogram32[0]);
        info.u8MaxPixelValue = MHal_SC_MACE_GetMaxPixelValue();
        info.u8MinPixelValue = MHal_SC_MACE_GetMinPixelValue();
        info.u8AvgPixelValue = MHal_SC_MACE_GetAvgPixelValue();

        info.u16TotalColorCount = MHal_SC_MACE_GetTotalColorCount();//[090601_Leo]
	MHal_SC_MACE_RequestHistogramData();	//thchen 20080717
        for (i=0 ; i<=31; i++)
        {
            HistogramData.u16Histogram32[i] = info.u16Histogram32[i];
			pDrvCtx->SrcInfo[info.srcIdx].HistogramData.u16Histogram32[i] = info.u16Histogram32[i];	//KWON_0721 modified by Cho04
        }
        HistogramData.u8MaxPixelValue = info.u8MaxPixelValue;
        HistogramData.u8MinPixelValue = info.u8MinPixelValue;
        HistogramData.u8AvgPixelValue = info.u8AvgPixelValue;

        HistogramData.u16TotalColorCount = info.u16TotalColorCount;//[090601_Leo]
    }
    else
    {
        for (i=0 ; i<=31; i++)
        {
            info.u16Histogram32[i] = HistogramData.u16Histogram32[i];
        }
        info.u8MaxPixelValue = HistogramData.u8MaxPixelValue;
        info.u8MinPixelValue = HistogramData.u8MinPixelValue;;
        info.u8AvgPixelValue = HistogramData.u8AvgPixelValue;;
        info.u16TotalColorCount = HistogramData.u16TotalColorCount;//[090601_Leo]
    }
#endif

    if (copy_to_user((U32*)arg, (U32*)&info, sizeof(SC_GET_HISTOGRAM_INFO_t)))
    {
        return;
    }
}
#else
void MDrv_SC_MACE_GetHistogramInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_HISTOGRAM_INFO_t info;
    SC_HISTOGRAM_INFO_t HistogramData;
    U8 i;

    if (copy_from_user(&info, (void __user *)arg, sizeof(SC_GET_HISTOGRAM_INFO_t)))
    {
        return;
    }

    info.bDataReady = MHal_SC_MACE_WaitHistogramDataReady();
    HistogramData = pDrvCtx->SrcInfo[info.srcIdx].HistogramData;
    if (info.bDataReady == TRUE)
    {
        MHal_SC_MACE_GetHistogram32(&info.u16Histogram32[0]);
        info.u8MaxPixelValue = MHal_SC_MACE_GetMaxPixelValue();
        info.u8MinPixelValue = MHal_SC_MACE_GetMinPixelValue();
        info.u8AvgPixelValue = MHal_SC_MACE_GetAvgPixelValue();
	MHal_SC_MACE_RequestHistogramData();	//thchen 20080717
        for (i=0 ; i<=31; i++)
        {
            HistogramData.u16Histogram32[i] = info.u16Histogram32[i];
			pDrvCtx->SrcInfo[info.srcIdx].HistogramData.u16Histogram32[i] = info.u16Histogram32[i];	//KWON_0721 modified by Cho04
        }
        HistogramData.u8AvgPixelValue = info.u8MaxPixelValue;
        HistogramData.u8MinPixelValue = info.u8MinPixelValue;
        HistogramData.u8AvgPixelValue = info.u8AvgPixelValue;

    }
    else
    {
        for (i=0 ; i<=31; i++)
        {
            info.u16Histogram32[i] = HistogramData.u16Histogram32[i];
        }
        info.u8MaxPixelValue = HistogramData.u8AvgPixelValue;
        info.u8MinPixelValue = HistogramData.u8MinPixelValue;;
        info.u8AvgPixelValue = HistogramData.u8AvgPixelValue;;
    }

    if (copy_to_user((U32*)arg, (U32*)&info, sizeof(SC_GET_HISTOGRAM_INFO_t)))
    {
        return;
    }
}
#endif

void MDrv_SC_MACE_SetLumaCurve(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_LUMA_CURVE_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_LUMA_CURVE_t)))
    {
        return;
    }

    MHal_SC_MACE_SetLumaCurve(param.u16LumaCurve);
}

void MDrv_SC_MACE_SetLumaCurveEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_LUMA_CURVE_ENABLE_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_LUMA_CURVE_ENABLE_t)))
    {
        return;
    }

    MHal_SC_MACE_SetLumaCurveEnable(param.bEnable);
}

void MDrv_SC_MACE_SetHistogramReqEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_HISTOGRAM_REQ_ENABLE_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_HISTOGRAM_REQ_ENABLE_t)))
    {
        return;
    }
    //MHal_SC_MACE_SetHistogramReqEnable(param.bEnable);
    MHal_SC_MACE_RequestHistogramData();
    // cc.chen - T.B.D. - It is useless for current "Histogram grabing" machanism.
}

void MDrv_SC_MACE_DLCInit(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{

    SC_DLCINIT_t DLCinit;

    if (copy_from_user(&DLCinit, (void __user *)arg, sizeof(SC_DLCINIT_t)))
    {
        return;
    }
    MHal_SC_MACE_DLCInit(DLCinit.u16Histogram_Vstart, DLCinit.u16Histogram_Vend); //thchen 20080708

}

void MDrv_SC_MACE_SetICCSaturationAdj(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_ICC_SATURATION_ADJ_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_ICC_SATURATION_ADJ_t)))
    {
        return;
    }

	MHal_SC_MACE_SetICCSaturationAdj(param.colorType, param.s8SatAdj);
}

void MDrv_SC_MACE_SetIBCYAdj(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_IBC_Y_ADJ_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_IBC_Y_ADJ_t)))
    {
        return;
    }

	MHal_SC_MACE_SetIBCYAdj(param.colorType, param.u8YAdj);
}

void MDrv_SC_MACE_SetIHCHueDiffColorYAdj(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_IHC_HUE_COLOR_DIFF_ADJ_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_IHC_HUE_COLOR_DIFF_ADJ_t)))
    {
        return;
    }

    MHal_SC_MACE_SetIHCHueDiffColorYAdj(param.colorType, param.s8HueAdj, param.u8YIndex, param.u8YLevel);
}

void MDrv_SC_MACE_SetIHCHueAdj(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_IHC_HUE_ADJ_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_IHC_HUE_ADJ_t)))
    {
        return;
    }

    //MDrv_SC_IP1_WaitOutputVSync(1, 50);//victor 20080923
    MHal_SC_MACE_SetIHCHueAdj(param.colorType, param.s8HueAdj);
}

void MDrv_SC_MACE_SetICCSaturationEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_ICC_SATURATION_ENABLE_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_ICC_SATURATION_ENABLE_t)))
    {
        return;
    }

    MHal_SC_MACE_SetICCSaturationEnable(param.bEnable);
}//thchen 20080718

void MDrv_SC_MACE_SetIBCYEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_IBC_Y_ENABLE_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_IBC_Y_ENABLE_t)))
    {
        return;
    }

    MHal_SC_MACE_SetIBCYEnable(param.bEnable);
}//thchen 20080718

void MDrv_SC_MACE_SetIHCHueEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_IHC_HUE_ENABLE_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_IHC_HUE_ENABLE_t)))
    {
        return;
    }

    MHal_SC_MACE_SetIHCHueEnable(param.bEnable);
}//thchen 20080718

void MDrv_SC_MACE_SetICCRegionTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_ICC_REGION_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_ICC_REGION_t)))
    {
        return;
    }

    MHal_SC_MACE_SetICCRegionTable(param.u8Data);
}//victor 20080814

void MDrv_SC_MACE_SetIHCRegionTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_IHC_REGION_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_IHC_REGION_t)))
    {
        return;
    }

    MHal_SC_MACE_SetIHCRegionTable(param.u8Data);
}//victor 20080814

void MDrv_SC_MACE_SetIHCYModeDiffColorEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    U32 param;
    if (copy_from_user(&param, (void __user *)arg, sizeof(U32)))
    {
        return;
    }
    MHal_SC_MACE_SetIHCYModeDiffColorEnable(param);
}

void MDrv_SC_MACE_SetIHCYModelEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    U32 param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(U32)))
    {
        return;
    }

    MHal_SC_MACE_SetIHCYModelEnable(param);
}//victor 20080826

void MDrv_SC_MACE_SetICCYModelEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    U32 param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(U32)))
    {
        return;
    }

    MHal_SC_MACE_SetICCYModelEnable(param);
}//victor 20080826

void MDrv_SC_SelectCSC(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    U8 param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(U8)))
    {
        return;
    }

    MHal_SC_SelectCSC(param);
}//victor 20080821

//------------------------------------------------------------------------------
//  Private
//------------------------------------------------------------------------------
void MDrv_SC_MACE_ArrayMultiply(S16 sFirst[3][3], S16 sSecond[3][3], S16 sResult[3][3])
{
    U8 ucRow, ucCol;

    // go line by line
    for (ucRow=0; ucRow!=3; ucRow++)
    {
        // go column by column
        for (ucCol=0; ucCol!=3; ucCol++)
        {
            sResult[ucRow][ucCol] = (S16)((((S32)sFirst[ucRow][0] * sSecond[0][ucCol]) +
                                     ((S32)sFirst[ucRow][1] * sSecond[1][ucCol]) +
                                     ((S32)sFirst[ucRow][2] * sSecond[2][ucCol])) >> 10);
        }
    }
}

S16 MDrv_SC_MACE_Sine(S16 u16Val)
{
    S8  sine_sin;
    S8  cur_region;
    S16 ph;
    S32 ret;

    u16Val = u16Val * 256 / 45;
    ph = u16Val % 2048;

    if ((ph >= 0) && (ph <= 512))
    {
        sine_sin = 1;
        cur_region = 0;
    }
    else if ((ph > 512) && (ph <= 1024))
    {
        sine_sin = 1;
        cur_region = 1;
    }
    else if ((ph > 1024) && (ph <= 1536))
    {
        sine_sin = -1;
        cur_region = 2;
    }
    else
    {
        sine_sin = -1;
        cur_region = 3;
    }

    while (ph > 512) ph -= 512;

    if ((cur_region == 1) || (cur_region == 3))
    {
        ph = 512 - ph;
    }

    ret = ((S32)MACE_sine(ph) * sine_sin * 1024) >> 15;

    return ret;
}

S16 MDrv_SC_MACE_Cosine(S16 u16Val)
{
    S8  cosine_sin;
    S8  cur_region;
    S16 ph;
    S32 ret;

    u16Val = u16Val * 256 / 45;
    ph = u16Val % 2048;

    if ((ph >= 0) && (ph <= 512))
    {
        cosine_sin = 1;
        cur_region = 0;
    }
    else if ((ph > 512) && (ph <= 1024))
    {
        cosine_sin = -1;
        cur_region = 1;
    }
    else if ((ph > 1024) && (ph <= 1536))
    {
        cosine_sin = -1;
        cur_region = 2;
    }
    else
    {
        cosine_sin = 1;
        cur_region = 3;
    }

    while (ph > 512) ph -= 512;

    if ((cur_region == 1) || (cur_region == 3))
    {
        ph = 512 - ph;
    }

    ret = ((S32)MACE_cosine(ph) * cosine_sin * 1024) >> 15;

    return ret;
}

#if 0
code S16 tSDTVYuv2rgb[3][3] =
{
    {  0x0662, 0x04A8,  0x0000 }, // 1.596,  1.164, 0
    { -0x0341, 0x04A8, -0x0190 }, // -0.813, 1.164, -0.391
    {  0x0000, 0x04A8,  0x0812 }  // 0,      1.164, 2.018
};

code S16 tHDTVYuv2rgb[3][3] =
{
    {  0x072C, 0x04A8,  0x0000 }, // 1.793,  1.164, 0
    { -0x0223, 0x04A8, -0x00DA }, // -0.534, 1.164, -0.213
    {  0x0000, 0x04A8,  0x0876 }  // 0,      1.164, 2.115
};
#endif

void MDrv_SC_MACE_InitCtx(PMACE_INFO_t pMACE_Info)
{
    // YUV to RGB matrix
    pMACE_Info->YUV2RGB_Matrix[0][0] =  0x0662;
    pMACE_Info->YUV2RGB_Matrix[0][1] =  0x04A8;
    pMACE_Info->YUV2RGB_Matrix[0][2] =  0x0000;
    pMACE_Info->YUV2RGB_Matrix[1][0] = -0x0341;
    pMACE_Info->YUV2RGB_Matrix[1][1] =  0x04A8;
    pMACE_Info->YUV2RGB_Matrix[1][2] = -0x0190;
    pMACE_Info->YUV2RGB_Matrix[2][0] =  0x0000;
    pMACE_Info->YUV2RGB_Matrix[2][1] =  0x04A8;
    pMACE_Info->YUV2RGB_Matrix[2][2] =  0x0812;

    // Color correction matrix
    pMACE_Info->CC_Matrix[0][0] = 0x0400;	//0x03FF;	//MKB_Alvin_091226 : csc corection
    pMACE_Info->CC_Matrix[0][1] = 0x0000;
    pMACE_Info->CC_Matrix[0][2] = 0x0000;
    pMACE_Info->CC_Matrix[1][0] = 0x0000;
    pMACE_Info->CC_Matrix[1][1] = 0x0400;	//0x03FF;	//MKB_Alvin_091226 : csc corection
    pMACE_Info->CC_Matrix[1][2] = 0x0000;
    pMACE_Info->CC_Matrix[2][0] = 0x0000;
    pMACE_Info->CC_Matrix[2][1] = 0x0000;
    pMACE_Info->CC_Matrix[2][2] = 0x0400;	//0x03FF;	//MKB_Alvin_091226 : csc corection

    pMACE_Info->u8RCon = 0x80;
    pMACE_Info->u8GCon = 0x80;
    pMACE_Info->u8BCon = 0x80;

    //++ ENABLE_RGB_RANGE_EXTENSION
    pMACE_Info->u16RConEx = 512;
    pMACE_Info->u16GConEx = 512;
    pMACE_Info->u16BConEx = 512;
    //-- ENABLE_RGB_RANGE_EXTENSION

    pMACE_Info->u8Contrast = 0x80;
    pMACE_Info->u8Saturation = 0x80;
    pMACE_Info->u8Hue = 50;

    pMACE_Info->sContrastRGBMatrix[0][0] =
    pMACE_Info->sContrastRGBMatrix[1][1] =
    pMACE_Info->sContrastRGBMatrix[2][2] = 1024;
    pMACE_Info->sContrastRGBMatrix[0][1] =
    pMACE_Info->sContrastRGBMatrix[1][0] =
    pMACE_Info->sContrastRGBMatrix[2][0] =
    pMACE_Info->sContrastRGBMatrix[0][2] =
    pMACE_Info->sContrastRGBMatrix[1][2] =
    pMACE_Info->sContrastRGBMatrix[2][1] = 0;

    pMACE_Info->sVideoSatHueMatrix[0][0] =
    pMACE_Info->sVideoSatHueMatrix[1][1] =
    pMACE_Info->sVideoSatHueMatrix[2][2] = 1024;
    pMACE_Info->sVideoSatHueMatrix[0][1] =
    pMACE_Info->sVideoSatHueMatrix[1][0] =
    pMACE_Info->sVideoSatHueMatrix[2][0] =
    pMACE_Info->sVideoSatHueMatrix[0][2] =
    pMACE_Info->sVideoSatHueMatrix[1][2] =
    pMACE_Info->sVideoSatHueMatrix[2][1] = 0;
}

void MDrv_SC_MACE_SetVideoContrastMatrix(PMACE_INFO_t pMACE_Info)
{
    MACE_PRINT("VideoContrastMatrix A: con00=0x%x, con11=0x%x, con22=0x%x\n",
                pMACE_Info->sContrastRGBMatrix[0][0],
                pMACE_Info->sContrastRGBMatrix[1][1],
                pMACE_Info->sContrastRGBMatrix[2][2]);

    pMACE_Info->sContrastRGBMatrix[0][0] = ((U16)pMACE_Info->u8RCon * (pMACE_Info->u8Contrast)) >> 4;
    pMACE_Info->sContrastRGBMatrix[1][1] = ((U16)pMACE_Info->u8GCon * (pMACE_Info->u8Contrast)) >> 4;
    pMACE_Info->sContrastRGBMatrix[2][2] = ((U16)pMACE_Info->u8BCon * (pMACE_Info->u8Contrast)) >> 4;

    //++ ENABLE_RGB_RANGE_EXTENSION
    pMACE_Info->sContrastRGBMatrix[0][0] = ((U32)pMACE_Info->u16RConEx * (pMACE_Info->sContrastRGBMatrix[0][0])) >> 9;
    pMACE_Info->sContrastRGBMatrix[1][1] = ((U32)pMACE_Info->u16GConEx * (pMACE_Info->sContrastRGBMatrix[1][1])) >> 9;
    pMACE_Info->sContrastRGBMatrix[2][2] = ((U32)pMACE_Info->u16BConEx * (pMACE_Info->sContrastRGBMatrix[2][2])) >> 9;
    //-- ENABLE_RGB_RANGE_EXTENSION

    MACE_PRINT("VideoContrastMatrix A': con00=0x%x, con11=0x%x, con22=0x%x\n",
                pMACE_Info->sContrastRGBMatrix[0][0],
                pMACE_Info->sContrastRGBMatrix[1][1],
                pMACE_Info->sContrastRGBMatrix[2][2]);
}

void MDrv_SC_MACE_SetVideoColorMatrix(PMACE_INFO_t pMACE_Info)
{
    S16 sResultTmp1[3][3];
    S16 sResultTmp2[3][3];

    MACE_PRINT("SatHueMatrix=0x%x, 0x%x, 0x%x\n",
                pMACE_Info->sVideoSatHueMatrix[0][0], pMACE_Info->sVideoSatHueMatrix[1][1], pMACE_Info->sVideoSatHueMatrix[2][2]);

    // adjust hue & saturation, and YUV to RGB
    MDrv_SC_MACE_ArrayMultiply(pMACE_Info->YUV2RGB_Matrix, pMACE_Info->sVideoSatHueMatrix, sResultTmp1);

    MACE_PRINT("After adjust hue & saturation, and YUV to RGB\n");
    MACE_PRINT("sResult[0][0]=%d, sResult[0][1]=%d, sResult[0][2]=%d\n",
                sResultTmp1[0][0], sResultTmp1[0][1], sResultTmp1[0][2]);
    MACE_PRINT("sResult[1][0]=%d, sResult[1][1]=%d, sResult[1][2]=%d\n",
                sResultTmp1[1][0], sResultTmp1[1][1], sResultTmp1[1][2]);
    MACE_PRINT("sResult[2][0]=%d, sResult[2][1]=%d, sResult[2][2]=%d\n",
                sResultTmp1[2][0], sResultTmp1[2][1], sResultTmp1[2][2]);

    // adjust contrast-RGB
    MDrv_SC_MACE_ArrayMultiply(pMACE_Info->sContrastRGBMatrix, sResultTmp1, sResultTmp2);

    MACE_PRINT("After adjust contrast-RGB\n");
    MACE_PRINT("sResult[0][0]=%d, sResult[0][1]=%d, sResult[0][2]=%d\n",
                sResultTmp2[0][0], sResultTmp2[0][1], sResultTmp2[0][2]);
    MACE_PRINT("sResult[1][0]=%d, sResult[1][1]=%d, sResult[1][2]=%d\n",
                sResultTmp2[1][0], sResultTmp2[1][1], sResultTmp2[1][2]);
    MACE_PRINT("sResult[2][0]=%d, sResult[2][1]=%d, sResult[2][2]=%d\n",
                sResultTmp2[2][0], sResultTmp2[2][1], sResultTmp2[2][2]);

    // do color correction
    MDrv_SC_MACE_ArrayMultiply(pMACE_Info->CC_Matrix, sResultTmp2, sResultTmp1);

    MACE_PRINT("After do color correction\n");
    MACE_PRINT("sResult[0][0]=%d, sResult[0][1]=%d, sResult[0][2]=%d\n",
                sResultTmp1[0][0], sResultTmp1[0][1], sResultTmp1[0][2]);
    MACE_PRINT("sResult[1][0]=%d, sResult[1][1]=%d, sResult[1][2]=%d\n",
                sResultTmp1[1][0], sResultTmp1[1][1], sResultTmp1[1][2]);
    MACE_PRINT("sResult[2][0]=%d, sResult[2][1]=%d, sResult[2][2]=%d\n",
                sResultTmp1[2][0], sResultTmp1[2][1], sResultTmp1[2][2]);

    MHal_SC_OP2_SetColorMatrix((U16*)sResultTmp1);
}

void MDrv_SC_MACE_SetVideoSatHueMatrix(PMACE_INFO_t pMACE_Info)
{
    S16 s16Tmp;
    U16 u16Hue;

    u16Hue = ((pMACE_Info->u8Hue <= 50) ? (50 - pMACE_Info->u8Hue) : (360-(pMACE_Info->u8Hue-50)));
    s16Tmp = ((S16)pMACE_Info->u8Saturation * 8);

    pMACE_Info->sVideoSatHueMatrix[2][2] = ((( (S32)MDrv_SC_MACE_Cosine(u16Hue) * s16Tmp))>>10);
    pMACE_Info->sVideoSatHueMatrix[0][0] = ((( (S32)MDrv_SC_MACE_Cosine(u16Hue) * s16Tmp))>>10);
    pMACE_Info->sVideoSatHueMatrix[2][0] = ((( (S32)MDrv_SC_MACE_Sine(u16Hue)   * s16Tmp))>>10);
    pMACE_Info->sVideoSatHueMatrix[0][2] = (((-(S32)MDrv_SC_MACE_Sine(u16Hue)   * s16Tmp))>>10);

    pMACE_Info->sVideoSatHueMatrix[1][1] = 1024;

    pMACE_Info->sVideoSatHueMatrix[0][1] = 0;
    pMACE_Info->sVideoSatHueMatrix[1][0] = 0;
    pMACE_Info->sVideoSatHueMatrix[1][2] = 0;
    pMACE_Info->sVideoSatHueMatrix[2][1] = 0;
}
