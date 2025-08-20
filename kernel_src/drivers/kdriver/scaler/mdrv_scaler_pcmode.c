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
//
////////////////////////////////////////////////////////////////////////////////


/******************************************************************************/
/*                    Header Files                        */
/* ****************************************************************************/
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "mst_utility.h"
#include "mdrv_types.h"
#include "mdrv_scaler_st.h"
#include "mdrv_scaler_pcmode.h"
#include "mdrv_scaler.h"
#include "mdrv_auto.h"
#include "mdrv_adc.h"
#include "mhal_adc.h"



/******************************************************************************/
/*                     Local                    */
/* ****************************************************************************/
SC_PCMODE_RESOLUTION astStandardModeResolution[RES_MAXIMUM] =
{
    { 640,  350}, // 00: RES_640X350
    { 640,  400}, // 01: RES_640X400
    { 720,  400}, // 02: RES_720X400
    { 640,  480}, // 03: RES_640X480
    { 800,  600}, // 04: RES_800X600
    { 832,  624}, // 05: RES_832X624
    {1024,  768}, // 06: RES_1024X768
    {1280, 1024}, // 07: RES_1280X1024
    {1600, 1200}, // 08: RES_1600X1200
    {1152,  864}, // 09: RES_1152X864
    {1152,  870}, // 10: RES_1152X870
    {1280,  768}, // 11: RES_1280x768
    {1280,  960}, // 12: RES_1280X960
    { 720,  480}, // 13: RES_720X480
    {1920, 1080}, // 14: RES_1920X1080

    {1280,  720}, // 15: RES_1280X720
    { 720,  576}, // 16: RES_720X576

    {1920, 1200}, // 17: RES_1920X1200

    {1400, 1050}, // 18: RES_1400X1050
    {1440,  900}, // 19: RES_1440X900
    {1680, 1050}, // 20: RES_1680X1050

    {1280,  800}, // 21: RES_1280X800
    {1600, 1024}, // 22: RES_1600X1024
    {1600,  900}, // 23: RES_1600X900
    {1360,  768}, // 24: RES_1360X768
    { 848,  480}, // 25: RES_848X480
    {1920, 1080}, // 26: RES_1920X1080P

    {1366,  768}, // 27: RES_1366X768,
    { 864,  648}, // 28: RES_864X648,
};

//SC_PCADC_MODESETTING_TYPE g_PcadcModeSetting;
extern SC_PCADC_MODETABLE_TYPE astStandardModeTable[MD_STD_MODE_MAX_INDEX];

#define MODE_TABLE_MAXIMUM_NUM  (sizeof(astStandardModeTable) / sizeof(SC_PCADC_MODETABLE_TYPE))

#define MIN_PC_PHASE            0
#define MAX_PC_PHASE            63

//#define MIN_PC_H_START          (g_PcadcModeSetting.u16DefaultHStart-(g_PcadcModeSetting.u16DefaultHStart/2))
//#define MAX_PC_H_START          (g_PcadcModeSetting.u16DefaultHStart+(g_PcadcModeSetting.u16DefaultHStart/2))

//#define MIN_PC_V_START          (1)
//#define MAX_PC_V_START          (g_PcadcModeSetting.u16DefaultVStart*2 - MIN_PC_V_START)

#define OPT_PCMODE_DGB 0
#if OPT_PCMODE_DGB // drmyung LGE 080626
#define SC_PCMODE_DEBUG(fmt, args...)   printk("\033[47;34m[PCMODE][%05d] " fmt "\033[0m", __LINE__, ## args)
#else
#define SC_PCMODE_DEBUG(fmt, args...)
#endif




//--------------------------------------------------------------------------------------------------
//  PC Mode
//--------------------------------------------------------------------------------------------------
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <asm/semaphore.h>

static SC_PCMODE_MODETABLE_t* g_ModeTablePtr = NULL;
static U16 g_u16ModeTableSize = 0;
U16	SysValPCResolution;


/******************************************************************************/
/*                   Functions                      */
/******************************************************************************/

//*************************************************************************
//Function name:    MDrv_SC_PCMode_GetStdModeResH
//Passing parameter:    U8 u8ModeIndex: current mode index
//Return parameter: U16: H resolution
//Description:          get standard H resolution
//*************************************************************************
U16 MDrv_SC_PCMode_GetStdModeResH(U8 u8ModeIndex)
{
    return astStandardModeResolution[g_ModeTablePtr[u8ModeIndex].u8ResIdx].u16DisplayWidth;
}
//FitchHsu 20081127 PCmode Auto config issue
U16 MDrv_SC_PCMode_GetStdModeHTotal(U8 u8ModeIndex)
{
    return g_ModeTablePtr[u8ModeIndex].u16HTotal;
}
//LGE [vivakjh]  2008/12/17	Component 특정 256Grey Pattern에서 세로줄 생김 현상 개선.
U16 MDrv_SC_PCMode_GetStdModeResV(U8 u8ModeIndex)
{
    return astStandardModeResolution[g_ModeTablePtr[u8ModeIndex].u8ResIdx].u16DisplayHeight;
}

/*	2008,10,13:	외부입력 전환 Aging Test시 System pending되는 문제(dreamer@lge.com)

	-> Aging 후, COMPONENT/RGB/HDMI에서 "NO SIGNAL" 표시하는 문제 수정
	(PC MODE의 KERNEL TASK에서 Wait Queue 처리가 잘못되어 있음)
*/
//static wait_queue_head_t  sc_pcmode_wq;

// function decalaraction


void MDrv_SC_PCMode_SetModeTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_MODETABLE_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_MODETABLE_t)))
    {
        return;
    }

    if (pDrvCtx->ModeTablePtr != NULL)
    {
        kfree(pDrvCtx->ModeTablePtr);
    }

    pDrvCtx->ModeTablePtr = kmalloc(param.u32TableSize, GFP_KERNEL);
    if (pDrvCtx->ModeTablePtr != NULL)
    {
        memcpy(pDrvCtx->ModeTablePtr, param.pModeTable, param.u32TableSize);
        g_ModeTablePtr = (SC_PCMODE_MODETABLE_t*)pDrvCtx->ModeTablePtr;
        g_u16ModeTableSize = param.u8TableCount;
    }
}
void MDrv_SC_PCMode_GetInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_PCMODE_INFO_t  param;
    PSC_SOURCE_INFO_t pSrcInfo;
    U16 u16HorizontalTotal;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_PCMODE_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    // FitchHsu 20090605 patch Sherwood USA DVD player timing invalid format
    #if SC_DOUBLE_SAMPLING_ENABLE // Disable double sampling requested by LG
    if (
    #if SC_YPBPR_720x480_60_SW_PATCH
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x480_60I_P ||
    #endif
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x480_60I   ||
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x480_60P   ||
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x576_50I   ||
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x576_50P)
    {
        u16HorizontalTotal = pSrcInfo->Modesetting.u16HorizontalTotal << 1;  // for better quality
    }
    else
	#endif
    {
        u16HorizontalTotal = pSrcInfo->Modesetting.u16HorizontalTotal;
    }

#if 0
    printk("u16H_CapStart = %d\n", pSrcInfo->u16H_CapStart);
    printk("u16V_CapStart = %d\n", pSrcInfo->u16V_CapStart);
    printk("u8Colck = %d\n", MDrv_SC_CalculatePixClk(u16HorizontalTotal));
    printk("u8Phase = %d\n", pSrcInfo->Modesetting.u8Phase);
#endif

#if 1    // swwoo LGE 080626 : match to LG DDI
    param.u8ModeIdx = pSrcInfo->u8ModeIdx;

    param.s16PC_H_POS = pSrcInfo->s16PC_H_POS;
    param.s16PC_V_POS = pSrcInfo->s16PC_V_POS;
    param.s16PC_H_TOTAL = pSrcInfo->s16PC_H_TOTAL;
    param.u8Phase = pSrcInfo->Modesetting.u8Phase;

	//param.s8Clock = MDrv_SC_CalculatePixClk(u16HorizontalTotal);//why??
    //param.u16PC_H_SIZE = 0;	// 20080714 swwoo LGE

#else
    param.u16PC_H_POS  = pSrcInfo->u16H_CapStart;
    param.u16PC_H_SIZE = pSrcInfo->u16H_CapSize;
    param.u16PC_V_POS  = pSrcInfo->u16V_CapStart;
    param.u16PC_V_SIZE = pSrcInfo->u16V_CapSize;
    param.u8Clock = MDrv_SC_CalculatePixClk(u16HorizontalTotal);
    param.u8Phase = pSrcInfo->Modesetting.u8Phase;
    param.u16PC_H_TOTAL = u16HorizontalTotal;
#endif

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_PCMODE_INFO_t)))
    {
        return;
    }
}

// 20080806 swwoo LGE
void MDrv_SC_PCMode_GetInfo_AutoTune(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_PCMODE_INFO_t  param;
    PSC_SOURCE_INFO_t pSrcInfo;
    U16 u16HorizontalTotal;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_PCMODE_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    // FitchHsu 20090605 patch Sherwood USA DVD player timing invalid format
    #if SC_DOUBLE_SAMPLING_ENABLE // Disable double sampling requested by LG
    if (
    #if SC_YPBPR_720x480_60_SW_PATCH
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x480_60I_P ||
    #endif
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x480_60I   ||
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x480_60P   ||
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x576_50I   ||
        pSrcInfo->Modesetting.u8ModeIndex == MD_720x576_50P)
    {
        u16HorizontalTotal = pSrcInfo->Modesetting.u16HorizontalTotal << 1;  // for better quality
        SC_PCMODE_DEBUG("MDrv_SC_PCMode_GetInfo_AutoTune [%d]\n", u16HorizontalTotal);
    }
    else
	#endif
    {
        u16HorizontalTotal = pSrcInfo->Modesetting.u16HorizontalTotal;
    }

    // swwoo LGE 080626 : match to LG DDI
    param.u8ModeIdx = pSrcInfo->u8ModeIdx;
    param.s16PC_H_POS = pSrcInfo->Modesetting.u16HorizontalStart ;
    param.s16PC_V_POS = pSrcInfo->Modesetting.u16VerticalStart;
    param.s16PC_H_TOTAL = pSrcInfo->Modesetting.u16HorizontalTotal;
    param.u8Phase = pSrcInfo->Modesetting.u8Phase;

    //param.s8Clock = MDrv_SC_CalculatePixClk(u16HorizontalTotal);//why??
    //param.u16PC_H_SIZE = 0;	// 20080714 swwoo LGE

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_PCMODE_INFO_t)))
    {
        return;
    }
}

void MDrv_SC_PCMode_SetInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_PCMODE_INFO_t  param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_PCMODE_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

#if 1 // swwoo LGE 080626
    if (param.s16PC_H_POS != 0x7fff)
    	pSrcInfo->s16PC_H_POS= param.s16PC_H_POS;
    if (param.s16PC_V_POS != 0x7fff)
    	pSrcInfo->s16PC_V_POS= param.s16PC_V_POS;

    if(param.s16PC_H_TOTAL > 3 && param.s16PC_H_TOTAL <3500 && pSrcInfo->u16Input_HFreq >0 && pSrcInfo->bIsSupportMode)
    {
        // Jericho - T.B.D
        if (param.s16PC_H_TOTAL != 0x7fff)
        {
            MDrv_ADC_SetMode(Use_VGA_Source(pSrcInfo->SrcType) ? TRUE:FALSE,
                (U16)(((U32)param.s16PC_H_TOTAL * pSrcInfo->u16Input_HFreq)/10000));// unit in MHz
    	    pSrcInfo->s16PC_H_TOTAL = param.s16PC_H_TOTAL;
    	    MHal_ADC_SetADCClk(param.s16PC_H_TOTAL);
        }
    }
        if (param.u8Phase != PHASE_MAX) //ykkim5 091112 autoconfig
        {
    	    MHal_ADC_SetADCPhase(param.u8Phase);  // setting ADC phase
        }    
#else
    pSrcInfo->u16H_CapStart = param.u16PC_H_POS;
    pSrcInfo->u16V_CapStart = param.u16PC_V_POS;
    pSrcInfo->Modesetting.u8Phase = param.u8Phase;

    //MHal_ADC_SetSogFilter(param.u8Clock); // no need
    //MHal_ADC_SetADCPLL(param.u8Clock);    // no need
    MHal_ADC_SetADCClk(param.u16PC_H_TOTAL);
    MHal_ADC_SetADCPhase(pSrcInfo->Modesetting.u8Phase);  // setting ADC phase
#endif
}

void MDrv_SC_AutoAdjust(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_ADC_CAL_INFO_t param;
    PSC_SOURCE_INFO_t   pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_ADC_CAL_INFO_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[0];

    // 20091012 daniel.huang: for finetune internal calibration
    param.bAutoResult = MDrv_Auto_Geometry(pDrvCtx, pSrcInfo, param.u8CalType,
                        param.u16TargetForRGain,
                        param.u16TargetForGGain,
                        param.u16TargetForBGain);

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_ADC_CAL_INFO_t)))
    {
        return;
    }
}

void MDrv_SC_Phaseadjust(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg) // swwoo LGE 080626
{
    U32 u32WinIdx;

    if (copy_from_user(&u32WinIdx, (void __user *)arg, sizeof(U32)))
    {
        return;
    }

    MHal_ADC_SetADCPhase((U8)u32WinIdx);  // setting ADC phase
}

SC_PCMODE_STATE_e MDrv_SC_PCMode_InitHandler(void)
{
    MHal_SC_Reset(SC_RST_IP_F2);
    MHal_SC_IP1_ResetSyncDetect();
    //(20100115 ykkim5) 20100113 daniel.huang: for solve sog sync unstable problem for some dvd-player with 480i timing
    MHal_SC_IP1_SetCoastWin(0x0C, 0x0C);
    return SC_PCMODE_DEBOUNCE_STATE;
}

SC_PCMODE_STATE_e MDrv_SC_PCMode_Debounce(void)
{
    static U8 u8Debounce      = 0;
    static U8 u8SyncStatus    = 0;
    static U8 u8PreSyncStatus = 0;

    u8SyncStatus = MDrv_SC_IP1_GetInputSyncStatus();

    SC_PCMODE_DEBUG("u8Debounce = %x, u8SyncStatus = %x, u8PreSyncStatus = %x\n",
        u8Debounce, u8SyncStatus, u8PreSyncStatus);

    if (u8SyncStatus == u8PreSyncStatus)
    {
        u8Debounce++;
    }
    else
    {
        u8Debounce = 0;
    }

    if (u8Debounce > SC_PCMODE_DEBOUNCE_THRESHOLD)
    {
        u8Debounce = 0;		// LGE 20080925 swwoo
        return SC_PCMODE_SYNC_DETECT_STATE;
    }

    u8PreSyncStatus = u8SyncStatus;
    return SC_PCMODE_DEBOUNCE_STATE;
}

SC_PCMODE_STATE_e MDrv_SC_PCMode_TimingChangeHandler(PSC_SOURCE_INFO_t psrc)
{
	//lachesis_081027 format 변경시 과도현상 임시 솔루션.
    //MHal_SC_IP1_SetInputSourceEnable(FALSE, FALSE);
	MDrv_Scaler_GenerateBlackVideo(TRUE); // LGE drmyung 081027
    //MHal_SC_IP1_SetInputSourceEnable(FALSE, FALSE);
    return SC_PCMODE_SYNC_DETECT_STATE;
}

SC_PCMODE_STATE_e MDrv_SC_PCMode_SyncDetectHandler(PSC_SOURCE_INFO_t psrc)
{
    psrc->u8Input_SyncStatus = MDrv_SC_IP1_GetInputSyncStatus();

    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_SYNC_LOSS)
    {
        MDrv_TimingDataInit(psrc);	// 20080722 swwoo LGE
        return SC_PCMODE_SYNC_DETECT_STATE;
    }
    else
    {
        return SC_PCMODE_SEARCH_MODE_STATE;
    }
}

SC_PCMODE_STATE_e MDrv_SC_PCMode_SearchModeHandler(PSC_SOURCE_INFO_t psrc)
{
    U16 u16MinHfreq, u16MaxHfreq;
    U16 u16MinVfreq, u16MaxVfreq;
    U16 u16MinVtotal, u16MaxVtotal;
    SC_PCMODE_MODETABLE_t ModeTable;

    psrc->u16Input_HPeriod = MHal_SC_IP1_GetHPeriod();
    psrc->u16Input_VTotal  = MHal_SC_IP1_GetVTotal();

    // calculate H/V frequency
    psrc->u16Input_HFreq = MDrv_SC_CalculateHFreqX10(psrc->u16Input_HPeriod);
    psrc->u16Input_VFreq = MDrv_SC_CalculateVFreqX10(psrc->u16Input_HFreq, psrc->u16Input_VTotal);

    psrc->u8Input_SyncStatus = MDrv_SC_IP1_GetInputSyncStatus();	// LGE 20080925 swwoo
    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_SYNC_LOSS) //ykkim5 091205 from mstar, Invalid issue
    {
        MDrv_TimingDataInit(psrc);
        return SC_PCMODE_SYNC_DETECT_STATE;
    }
    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
    {
        psrc->u16Input_VFreq *= 2;
    }	
	SC_PCMODE_DEBUG("====>YKK Get from Register  u16Input_HFreq:%d, u16Input_VFreq:%d, u16Input_VTotal:%d<===\n",psrc->u16Input_HFreq,psrc->u16Input_VFreq,psrc->u16Input_VTotal);

	//printk("MDrv_SC_PCMode_SearchModeHandler psrc->u16Input_VFreq=%d\n", psrc->u16Input_VFreq);

    // check sync range limitation
    if (Use_VGA_Source(psrc->SrcType))
    {
        u16MinHfreq    = MIN_HFREQ_OF_RGB;
        u16MaxHfreq    = MAX_HFREQ_OF_RGB;
        u16MinVfreq    = MIN_VFREQ_OF_RGB;
        u16MaxVfreq    = MAX_VFREQ_OF_RGB;
        u16MinVtotal   = MIN_VTOTAL_OF_RGB;
        u16MaxVtotal   = MAX_VTOTAL_OF_RGB;
    }
    else // Use_YPbPr_Source
    {
        u16MinHfreq    = MIN_HFREQ_OF_YPBPR;
        u16MaxHfreq    = MAX_HFREQ_OF_YPBPR;
        u16MinVfreq    = MIN_VFREQ_OF_YPBPR;
        u16MaxVfreq    = MAX_VFREQ_OF_YPBPR;
        u16MinVtotal   = MIN_VTOTAL_OF_YPBPR;
        u16MaxVtotal   = MAX_VTOTAL_OF_YPBPR;
    }

    if ((psrc->u16Input_HFreq > u16MaxHfreq)   || (psrc->u16Input_HFreq < u16MinHfreq) ||
        (psrc->u16Input_VFreq > u16MaxVfreq)   || (psrc->u16Input_VFreq < u16MinVfreq) ||
        (psrc->u16Input_VTotal > u16MaxVtotal) || (psrc->u16Input_VTotal < u16MinVtotal) ||
		(Use_VGA_Source(psrc->SrcType) && (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)))
    {
        SC_PCMODE_DEBUG("====> Unsupported mode 1\n");
        psrc->u8ModeIdx = 0xFF;	// 20080811 swwoo LGE 해상도 변경시 Invalid Format display 안되는 문제.
 		psrc->bIsSupportMode = FALSE;
        return SC_PCMODE_UNSUPPORTED_STATE;
    }

    // find match mode
    ModeTable.u16HFreq  = psrc->u16Input_HFreq;
    ModeTable.u16VFreq  = psrc->u16Input_VFreq;
    ModeTable.u16VTotal	= psrc->u16Input_VTotal;
    ModeTable.u8StatusFlag = psrc->u8Input_SyncStatus;
    psrc->u8ModeIdx = MDrv_SC_PCMode_MatchMode(psrc, &ModeTable, Use_YPbPr_Source(psrc->SrcType));
    if (psrc->u8ModeIdx == 0xFF)
    {
        SC_PCMODE_DEBUG("====> Unsupported mode 2\n");
		psrc->bIsSupportMode = FALSE;
        return SC_PCMODE_UNSUPPORTED_STATE;
    }
    else
    {
		psrc->bIsSupportMode = TRUE;
        return SC_PCMODE_SUPPORTED_STATE;
    }
}


// swwoo LGE 080702
SC_PCMODE_STATE_e MDrv_SC_PCMode_ResolutionChangeHandler(PSC_SOURCE_INFO_t psrc)
{
    SC_PCMODE_MODETABLE_t ModeTable;

    // find match mode
    ModeTable.u16HFreq  = psrc->u16Input_HFreq;
    ModeTable.u16VFreq  = psrc->u16Input_VFreq;
    ModeTable.u16VTotal = psrc->u16Input_VTotal;
    ModeTable.u8StatusFlag = psrc->u8Input_SyncStatus;
    psrc->u8ModeIdx = MDrv_SC_PCMode_MatchMode(psrc, &ModeTable, Use_YPbPr_Source(psrc->SrcType));
    if (psrc->u8ModeIdx == 0xFF)
    {
        SC_PCMODE_DEBUG("====> Unsupported mode 2\n");
        return SC_PCMODE_UNSUPPORTED_STATE;
    }
    else
    {
//        return SC_PCMODE_STABLE_CHECK_STATE;
        return SC_PCMODE_SUPPORTED_STATE;	// LGE 20080923 swwoo
    }
}

// LGE drmyung 080702
void MDrv_SC_UpdateTimingInfo(PSC_SOURCE_INFO_t psrc)
{
	if(psrc->bIsSupportMode)
	{
		psrc->u16Input_SC_HFreq = psrc->u16Input_HFreq;
		psrc->u16Input_SC_VFreq = psrc->u16Input_VFreq;

		psrc->u16Input_SC_HPeriod = psrc->Modesetting.u16HorizontalTotal;
		psrc->u16Input_SC_VTotal = psrc->Modesetting.u16VerticalTotal;

		psrc->u16Input_SC_HStart = psrc->Modesetting.u16HorizontalStart;
		psrc->u16Input_SC_VStart = psrc->Modesetting.u16VerticalStart;

		psrc->u16H_SC_CapSize = astStandardModeResolution[g_ModeTablePtr[psrc->u8ModeIdx].u8ResIdx].u16DisplayWidth;
		psrc->u16V_SC_CapSize = astStandardModeResolution[g_ModeTablePtr[psrc->u8ModeIdx].u8ResIdx].u16DisplayHeight;

		psrc->u8Input_SC_SyncStatus = psrc->u8Input_SyncStatus;
		psrc->u8Input_SC_Phase = psrc->Modesetting.u8Phase;
	}
	else
	{
		psrc->u16Input_SC_HFreq = psrc->u16Input_HFreq;
		psrc->u16Input_SC_VFreq = psrc->u16Input_VFreq;

		psrc->u16Input_SC_HPeriod = 0;
		psrc->u16Input_SC_VTotal = 0;

		psrc->u16Input_SC_HStart = 0;
		psrc->u16Input_SC_VStart = 0;
		psrc->u16H_SC_CapSize = 0xffe;	//lachesis_081108 video_ddi에서 invalid로 인식하도록 하기 위하여.
		psrc->u16V_SC_CapSize = 0xffe;

		psrc->u8Input_SC_SyncStatus = 0;
		psrc->u8Input_SC_Phase = 0;
	}

}

SC_PCMODE_STATE_e MDrv_SC_PCMode_StableSyncCheckHandler(PSC_SOURCE_INFO_t psrc)
{
    U8  u8InputSyncStatus;
    U16 u16HPeriod, u16VTotal;
	U8 bIsInvalidTimingDetected = FALSE;

    u8InputSyncStatus = MDrv_SC_IP1_GetInputSyncStatus();
    u16HPeriod = MHal_SC_IP1_GetHPeriod();
    u16VTotal  = MHal_SC_IP1_GetVTotal();

    if (u8InputSyncStatus != psrc->u8Input_SyncStatus)
    {
        SC_PCMODE_DEBUG("From Reg...SyncStatus change new=0x%x, old=0x%x\n", u8InputSyncStatus, psrc->u8Input_SyncStatus);
        psrc->u8Input_SyncStatus = u8InputSyncStatus;
        //return SC_PCMODE_INIT_STATE;	// LGE swwoo 20080903 COMP 480i -> RGB 1080P 60hz 변경시에 No Signal 로 못하는 문제 수정.
		bIsInvalidTimingDetected = TRUE;
    }
    else if (MDrv_SC_SubtractABS(u16HPeriod, psrc->u16Input_HPeriod) > SC_PCMODE_HPERIOD_TORLANCE)
    {
        SC_PCMODE_DEBUG("From Reg..HPeriod change. Current=%d, Standard=%d\n",u16HPeriod,psrc->u16Input_HPeriod);
        bIsInvalidTimingDetected = TRUE;
        //return SC_PCMODE_TIMING_CHANGE_STATE;
    }
    else if (MDrv_SC_SubtractABS(u16VTotal, psrc->u16Input_VTotal) > SC_PCMODE_VTOTAL_TORLANCE)
    {
        SC_PCMODE_DEBUG("From Reg..VTotal change. Current=%d, Standard=%d\n",u16VTotal,psrc->u16Input_VTotal);
        bIsInvalidTimingDetected = TRUE;
        //return SC_PCMODE_TIMING_CHANGE_STATE;
    }

	if(bIsInvalidTimingDetected)
	{
		MDrv_Scaler_GenerateBlackVideo(TRUE);	//lachesis_081115
	    psrc->u8ModeIdx = 0xFF;
		return SC_PCMODE_TIMING_CHANGE_STATE;
	}
	else
	{
	    MDrv_SC_UpdateTimingInfo(psrc);// LGE drmyung 080702
	    return SC_PCMODE_STABLE_CHECK_STATE;
	}
}

#define DIVID_HZ	25  // LGE drmyung 080702
int MDrv_SC_PCMode_Thread(void *data)
{
    SC_PCMODE_STATE_e nextState;
    PSC_SOURCE_INFO_t psrc = (PSC_SOURCE_INFO_t)data;
    PSC_THREAD_DATA_t pThreadData = &(psrc->ThreadData);
    U32 u32Timeout;
    //LGE [vivakjh] 2009/01/18	VGA Lock[Fitch's solution] is  NG !!! 	U16 u16synccount = 0; //LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue
    S32 s32Ret;
    static U16 u16SavedResolutionIndex = 0; // swwoo LGE 080626
/*	2008,10,13:	외부입력 전환 Aging Test시 System pending되는 문제(dreamer@lge.com)

	-> Aging 후, COMPONENT/RGB/HDMI에서 "NO SIGNAL" 표시하는 문제 수정
	(PC MODE의 KERNEL TASK에서 Wait Queue 처리가 잘못되어 있음)
*/

#ifndef	SC_USE_ONLY_ONE_THREAD
    //init_waitqueue_head(&sc_pcmode_wq);
    init_waitqueue_head(&pThreadData->thread_wq);
#endif

//printk(KERN_INFO"+Init\n");
    pThreadData->u8ThreadState = SC_PCMODE_INIT_STATE;
    pThreadData->u16Ctrl = SC_THREAD_CTRL_NONE;
#ifdef FAST_THREAD_CONTROL
	pThreadData->u16Ctrl |= SC_THREAD_CTRL_FAST_STATE_CHANGE;
#endif

    nextState = pThreadData->u8ThreadState;
    u32Timeout = HZ/DIVID_HZ;

    while (1)
    {
//        printk(".");
//printk(KERN_INFO"+T%d+", u32Timeout);
/*	2008,10,13:	외부입력 전환 Aging Test시 System pending되는 문제(dreamer@lge.com)

	-> Aging 후, COMPONENT/RGB/HDMI에서 "NO SIGNAL" 표시하는 문제 수정
	(PC MODE의 KERNEL TASK에서 Wait Queue 처리가 잘못되어 있음)
*/
        //s32Ret = wait_event_interruptible_timeout(sc_pcmode_wq, pThreadData->u16Ctrl != 0, u32Timeout);
        s32Ret = wait_event_interruptible_timeout(pThreadData->thread_wq, pThreadData->u16Ctrl != 0, u32Timeout);
        if (s32Ret == -ERESTARTSYS)
        {
            break;
        }

        if (pThreadData->u16Ctrl != 0)
        {
            // fast state change
            if (pThreadData->u16Ctrl & SC_THREAD_CTRL_FAST_STATE_CHANGE)
            {
                pThreadData->u16Ctrl &= ~SC_THREAD_CTRL_FAST_STATE_CHANGE;
            }

            // terminate
            if (pThreadData->u16Ctrl & SC_THREAD_CTRL_TERMINATE)
            {
                break;
            }
        }

        switch (pThreadData->u8ThreadState)
        {
        case SC_PCMODE_INIT_STATE:
            u32Timeout = HZ/DIVID_HZ;
            MDrv_TimingDataInit(psrc);// LGE swwoo
            nextState  = MDrv_SC_PCMode_InitHandler();
            break;

        case SC_PCMODE_DEBOUNCE_STATE:
            u32Timeout = HZ/DIVID_HZ;
			nextState  = MDrv_SC_PCMode_Debounce();
            break;

        case SC_PCMODE_TIMING_CHANGE_STATE:
            MDrv_TimingDataInit(psrc);	// 20080812 swwoo LGE opened... 화면 과도 현상의 원인.
			MHal_SC_IP1_ResetSyncDetect(); // 20081105 ibellup
            if (u16SavedResolutionIndex != SysValPCResolution) // swwoo LGE 080626
            {
            	nextState = SC_PCMODE_TIMING_CHANGE_STATE;
            	u16SavedResolutionIndex = SysValPCResolution;
            	break;
            }
            u32Timeout = HZ/DIVID_HZ;
            MDrv_SC_WakeupEvent(psrc, SC_EVENT_TIMING_CHANGE);
			SC_SET_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_ALL);
            nextState  = MDrv_SC_PCMode_TimingChangeHandler(psrc);
            break;

        case SC_PCMODE_SYNC_DETECT_STATE:
            u32Timeout = HZ/DIVID_HZ;
            nextState  = MDrv_SC_PCMode_SyncDetectHandler(psrc);

            #if 0	//LGE [vivakjh] 2009/01/18	VGA Lock[Fitch's solution] is  NG !!!
			//LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue
            if((u16synccount > 3) && (psrc->binputsync == FALSE))
            {
               	MDrv_SC_WakeupEvent(psrc, SC_EVENT_PCMODE_UNSUPPORTED);
                psrc->binputsync = TRUE;
                u16synccount = 0;
                msleep(20);
            }
            else
            {
                u16synccount++;
            }
			#endif

			if (Use_VGA_Source(psrc->SrcType))
			{
				msleep(15);	//LGE [vivakjh] 2009/01/18  	특정 Notebook VGA lock 잡히는 눈제 추가 Delay가 필요함.
				if (psrc->u8Input_SyncStatus & SC_SYNCSTS_HSYNC_POL_BIT)	// 20080826 swwoo LGE PC 입력의 경우 오른쪽에 녹색바를 볼 수 있는데, 녹색바가 나오기까지의 마진을 좀더 확보하기 위해 수정.
				{
					//printk("! SC_SYNCSTS_HSYNC_POL_BIT\n");
					//(ykkim5 recover origin value.)MHal_ADC_SetInputHsyncPolarity(FALSE);
					MHal_ADC_SetInputHsyncPolarity(TRUE);
				}
				else
				{
					//printk("SC_SYNCSTS_HSYNC_POL_BIT\n");
					//(ykkim5 recover origin value.)MHal_ADC_SetInputHsyncPolarity(TRUE);
					MHal_ADC_SetInputHsyncPolarity(FALSE);
				}
				msleep(15);	//LGE [vivakjh] 2009/01/18  	특정 Notebook VGA lock 잡히는 눈제 추가 Delay가 필요함.
			}
			else	// Component Source
			{
				if (psrc->u8Input_SyncStatus & SC_SYNCSTS_HSYNC_POL_BIT)	// 20080826 swwoo LGE
				{
					//printk("SC_SYNCSTS_HSYNC_POL_BIT\n");
					MHal_ADC_SetInputHsyncPolarity(TRUE);
				}
				else
				{
					//printk("! SC_SYNCSTS_HSYNC_POL_BIT\n");
					MHal_ADC_SetInputHsyncPolarity(FALSE);
				}
			}
            break;

        case SC_PCMODE_SEARCH_MODE_STATE:
            u32Timeout = HZ/DIVID_HZ;
            nextState  = MDrv_SC_PCMode_SearchModeHandler(psrc);
            break;

        case SC_PCMODE_RESOLUTION_CHANGE_STATE:	// swwoo LGE 080702
            u32Timeout = HZ/500;
            nextState  = MDrv_SC_PCMode_ResolutionChangeHandler(psrc);
            break;

        case SC_PCMODE_SUPPORTED_STATE:
			SC_PCMODE_DEBUG("[PCMODE]u8ModeIndex %d\n", psrc->Modesetting.u8ModeIndex);
			SC_PCMODE_DEBUG("[PCMODE]u16HorizontalStart 0x%x\n", psrc->Modesetting.u16HorizontalStart);
			SC_PCMODE_DEBUG("[PCMODE]u16VerticalStart 0x%x\n", psrc->Modesetting.u16VerticalStart);
			SC_PCMODE_DEBUG("[PCMODE]u16HorizontalTotal %d\n", psrc->Modesetting.u16HorizontalTotal);
			SC_PCMODE_DEBUG("[PCMODE]u16VerticalTotal %d\n", psrc->Modesetting.u16VerticalTotal);
			SC_PCMODE_DEBUG("[PCMODE]u8Phase 0x%x\n", psrc->Modesetting.u8Phase);
			SC_PCMODE_DEBUG("[PCMODE]u8SyncStatus 0x%x\n", psrc->Modesetting.u8SyncStatus);
			SC_PCMODE_DEBUG("[PCMODE]u8AutoSign %d\n", psrc->Modesetting.u8AutoSign);

            if (Use_VGA_Source(psrc->SrcType))
            {
//                MDrv_Auto_Geometry(psrc, AUTO_TUNE_BASIC);//thchen 20080729	// 20080715 swwoo LGE
            }

            MDrv_SC_WakeupEvent(psrc, SC_EVENT_PCMODE_SUPPORTED);
            nextState = SC_PCMODE_STABLE_CHECK_STATE;
            break;

        case SC_PCMODE_UNSUPPORTED_STATE:
            MDrv_SC_WakeupEvent(psrc, SC_EVENT_PCMODE_UNSUPPORTED);
            nextState = SC_PCMODE_STABLE_CHECK_STATE;
            break;

        case SC_PCMODE_STABLE_CHECK_STATE:
            if (u16SavedResolutionIndex != SysValPCResolution) // swwoo LGE 080626
            {
            	nextState = SC_PCMODE_RESOLUTION_CHANGE_STATE;
            	u16SavedResolutionIndex = SysValPCResolution;
				u32Timeout = HZ/500;
            	break;
            }
            else
	            u32Timeout = 15;//HZ/DIVID_HZ;	//lachesis_081115 xbox format change시 과도현상 개선 15ms마다 polling

            nextState = MDrv_SC_PCMode_StableSyncCheckHandler(psrc);
            break;

        default:
            break;
        }

        if (nextState != pThreadData->u8ThreadState)
        {
        	SC_PCMODE_DEBUG("[PCMODE]State change from %d\n", pThreadData->u8ThreadState);
            pThreadData->u8ThreadState = nextState;
            SC_PCMODE_DEBUG("[PCMODE]State change   to %d\n", nextState);
        }
    }

    SC_SET_SRC_STATE(psrc, SC_SRC_STATE_PCMODE_STOP);

//printk(KERN_INFO"+Stop\n");
    return 0;
}


void MDrv_SC_PCMode_Start(PSC_SOURCE_INFO_t psrc)
{
#ifdef	SC_USE_ONLY_ONE_THREAD
	extern long scaler_pid;

	psrc->u8ModeIdx = 0xFF;

	if( scaler_pid < 0 )
	{
		scaler_pid = kernel_thread((int (*)(void*))MDrv_SC_Thread, (void*)psrc, CLONE_KERNEL );
	}

	psrc->u8ThreadMode = 1;
#ifdef FAST_THREAD_CONTROL
	psrc->u8ThreadCtrl = 1;
#endif
	MDrv_SC_WakeupEvent(psrc, SC_EVENT_PCMODE_CHANGE);
	//	printk("+Create(%d)\n", scaler_pid);
#else
	static long PCMode_thread_pid;

    psrc->u8ModeIdx = 0xFF;
	if( PCMode_thread_pid < 0 )
	{
	    PCMode_thread_pid = kernel_thread((int (*)(void*))MDrv_SC_PCMode_Thread, (void*)psrc, CLONE_KERNEL);
	}
#endif	/* #ifdef	SC_USE_ONLY_ONE_THREAD */
}

void MDrv_SC_PCMode_Stop(PSC_SOURCE_INFO_t psrc)
{
#if 1
/*	2008,10,07:	외부입력 전환 Aging Test시 System pending되는 문제(dreamer@lge.com)

	MDrv_HDMI_Stop()/MDrv_SC_PCMode_Stop() 함수에서 Return 안되서 생기는 문제
	-> 특정시간 동안만 확인하는 방식으로 수정함.
*/
	U32	index;
#define	_MAX_CHECK_TERM 		1000	// 1 sec
#define _CHECK_SLEEP_TERM		10  	// 10 ms
	//lachesis_081023 입력 전환 시 이전 정보를 읽어가서 mute 풀리는 문제.
	psrc->u16Input_SC_HFreq = 0;
	psrc->u16Input_SC_VFreq = 0;
	psrc->u16Input_SC_HPeriod  = 0;
	psrc->u16Input_SC_HStart = 0;
	psrc->u16Input_SC_VStart = 0;
	psrc->u16Input_SC_VTotal   = 0;
	psrc->u16H_SC_CapSize	   = 0;
	psrc->u16V_SC_CapSize	   = 0;
	psrc->u8Input_SC_SyncStatus = 0;
	psrc->u8Input_SC_Phase = 0;


#ifdef	SC_USE_ONLY_ONE_THREAD
	psrc->u8ThreadMode = 0;
#ifdef FAST_THREAD_CONTROL
	psrc->u8ThreadCtrl = 0;
#endif
#endif

	psrc->ThreadData.u16Ctrl = SC_THREAD_CTRL_TERMINATE;
	for( index = 0; index  < (_MAX_CHECK_TERM / _CHECK_SLEEP_TERM); index ++ )
	{
		if( SC_GET_SRC_STATE(psrc) & SC_SRC_STATE_PCMODE_STOP )
		{
			break;
		}

		msleep( _CHECK_SLEEP_TERM );
	}

	if( index >= (_MAX_CHECK_TERM / _CHECK_SLEEP_TERM))
	{
		SC_PCMODE_DEBUG("ERR: MDrv_SC_PCMode_Stop\n");
	}
#else
    psrc->ThreadData.u16Ctrl = SC_THREAD_CTRL_TERMINATE;
    while (!(SC_GET_SRC_STATE(psrc) & SC_SRC_STATE_PCMODE_STOP))
    {
        schedule();
    }
#endif
    SC_CLR_SRC_STATE(psrc, SC_SRC_STATE_PCMODE_STOP);
}

U8 MDrv_SC_PCMode_MatchMode(PSC_SOURCE_INFO_t psrc, SC_PCMODE_MODETABLE_t* pModeTable, BOOL bYPbPrSource)
{
    SC_PCMODE_MODETABLE_t* pStdModeTbl;

    U8  u8Index;
    U8  u8MatchIndex;
    U8  u8Flag;
    U16 u16MatchDelta;
    U16 u16VTotalMatchDelta;

    U16 u16HFreqDelta;
    U16 u16VFreqDelta;
    U16 u16HVFreqDalta;
    U16 u16VTotalDelta;
	U8 newFormat;
	U8 modPolarity;

    //-------------------
    // First run match
    //-------------------
    u8MatchIndex = 0xFF;
    u16MatchDelta = SC_PCMODE_HFREQ_TORLANCE + SC_PCMODE_VFREQ_TORLANCE;
    u16VTotalMatchDelta = 0xFFFF;

    pStdModeTbl = g_ModeTablePtr;

    for (u8Index = 0; u8Index < g_u16ModeTableSize; u8Index++, pStdModeTbl++)
    {
		newFormat=0;
        // only search corresponded table
        if (Use_YPbPr_Source(psrc->SrcType))// || Use_YCbCr_Source(psrc->SrcType))
        {
            if (!(pStdModeTbl->u8StatusFlag & SC_PCMODE_FLAG_YPBPR_BIT))
            {
                continue;
            }
        }
        else    // Mode detect for PC
        {
            if (pStdModeTbl->u8StatusFlag & SC_PCMODE_FLAG_YPBPR_BIT)
            {
                continue;
            }
        }

        // match H/V frequency
        u16HFreqDelta  = MDrv_SC_SubtractABS(pModeTable->u16HFreq, pStdModeTbl->u16HFreq);
        u16VFreqDelta  = MDrv_SC_SubtractABS(pModeTable->u16VFreq, pStdModeTbl->u16VFreq);
        u16VTotalDelta = MDrv_SC_SubtractABS(pModeTable->u16VTotal, pStdModeTbl->u16VTotal);

        if ((u16HFreqDelta < SC_PCMODE_HFREQ_TORLANCE) &&
            (u16VFreqDelta < SC_PCMODE_VFREQ_TORLANCE) &&
            (u16VTotalDelta < pStdModeTbl->u8VTotalTolerance))
        {
            // calculate frequency difference
            u16HVFreqDalta = u16HFreqDelta + u16VFreqDelta;

            if (((u16MatchDelta > u16HVFreqDalta) && (u16VTotalMatchDelta > u16VTotalDelta)) ||
                ((u16HVFreqDalta <= 10) && (u16VTotalDelta < u16VTotalMatchDelta)))
            {
				newFormat = 1;
            }
			//640x350@70이 720x400@70으로 인식됨.
			// H,V freq, Vtotal이 match값과  같을경우 Polarity 체크하여 변경.
			else if ((pStdModeTbl->u8ResIdx ==  RES_640X350) && (u16MatchDelta == u16HVFreqDalta) && (u16VTotalMatchDelta == u16VTotalDelta))
			{
				modPolarity = pModeTable->u8StatusFlag & 0x03;

				if(modPolarity==2)	// POR_HPVN
					newFormat = 1;
				else
					newFormat = 0;
				SC_PCMODE_DEBUG(" #####YKK  modPolarity of 640x350 & 720x400 = 0x%x ##### \n\n",modPolarity);				
			}
			//
			// 1280x768 이 1366x768로 인식됨. 1366x768은 polarity 체크하도록 함.
			if ((1== newFormat) && (pStdModeTbl->u8ResIdx ==  RES_1366X768))
			{
				modPolarity = pModeTable->u8StatusFlag & 0x03;

				if(modPolarity!=0)	// POR_HNVN
					newFormat = 0;
				SC_PCMODE_DEBUG(" #####YKK  modPolarity of 1280x768 & 1366x768 = 0x%x ##### \n\n",modPolarity);				
			}
			// 091026 ykkim5...
			//1280x768@75 가 1024x768@75(18번)로 인식됨.  1024x768 75Hz (26번)은 polarity 체크하도록 함.
			if ((1== newFormat) && (pStdModeTbl->u8ResIdx ==  RES_1024X768)&& 
				(MDrv_SC_SubtractABS(pStdModeTbl->u16VFreq,750)<SC_PCMODE_VFREQ_TORLANCE) )
			{
				modPolarity = pModeTable->u8StatusFlag & (SC_SYNCSTS_HSYNC_POL_BIT|SC_SYNCSTS_VSYNC_POL_BIT);// H/V의 Polarity check.

				if(modPolarity == 0x01)//HP && VN..
					newFormat = 0;	//skip it.
				else
					newFormat = 1;	
				SC_PCMODE_DEBUG(" #####YKK  modPolarity of 1280x768@75 & 1024x768@75Hz = 0x%x ##### \n\n",modPolarity);
			}
			//
			//----------------------------------------------------------------------------------
			if (1== newFormat)
			{
                u16MatchDelta       = u16HVFreqDalta;
                u8MatchIndex        = u8Index;
                u16VTotalMatchDelta = u16VTotalDelta;

				//081206 ibellup: Be carefull!!! if mode tabel is changed, must check this part!!!
				// swwoo LGE 080626 : 1024/1280/1360/1366 x768 60hz의 RGB 입력일 경우에만 적용되는 코드임.
            	if (MDrv_SC_SubtractABS(pStdModeTbl->u16VFreq,600)<SC_PCMODE_VFREQ_TORLANCE)
            	{
					if(pStdModeTbl->u8ResIdx == RES_1024X768 || pStdModeTbl->u8ResIdx == RES_1280X768
					|| pStdModeTbl->u8ResIdx == RES_1360X768 || pStdModeTbl->u8ResIdx == RES_1366X768)
					{
						if (SysValPCResolution == 0)	// 1024x768
						{
							if(pStdModeTbl->u8ResIdx != RES_1024X768)
							{
								// 1024x768 60Hz (VESA)         --> 15
								// 925FS_59
								psrc->Modesetting.u8ModeIndex			= 15;
								psrc->Modesetting.u16HorizontalStart	= 0xfc;
								psrc->Modesetting.u16VerticalStart		= 0x15;
								psrc->Modesetting.u16HorizontalTotal	= 1328;
								psrc->Modesetting.u16VerticalTotal		= 798;
								psrc->Modesetting.u8Phase				= 0x39;

								psrc->Modesetting.u8SyncStatus			= SC_PCMODE_FLAG_POR_HNVN;
								psrc->Modesetting.u8AutoSign			= 0;

								continue;
							}
						}
						else if (SysValPCResolution == 1)	// 1280x768
						{
							if(pStdModeTbl->u8ResIdx != RES_1280X768)
							{
								// 1280x768 60Hz (VESA-GTF)     --> 25
								// 925FS_85 86 87 88
								psrc->Modesetting.u8ModeIndex			= 25;
								psrc->Modesetting.u16HorizontalStart	= 0x13c;
								psrc->Modesetting.u16VerticalStart		= 0x12;
/* side effect발생하여 임시로 막음
								// FitchHsu 20081224 VGA 800x600 auto config fail
								if (pModeTable->u16VTotal == 795)
								{
								    psrc->Modesetting.u16HorizontalTotal	= 1680;
								    psrc->Modesetting.u16VerticalTotal		= 795;
								}
								else
								{*/
								psrc->Modesetting.u16HorizontalTotal		= 1664;
								psrc->Modesetting.u16VerticalTotal		= 798;
//								}
								psrc->Modesetting.u8Phase				= 0x2a;

								psrc->Modesetting.u8SyncStatus			= SC_PCMODE_FLAG_POR_HNVP;
								psrc->Modesetting.u8AutoSign			= 0;

								continue;
							}
						}
						else if (SysValPCResolution == 2)	// 1360x768
						{
							if(pStdModeTbl->u8ResIdx != RES_1360X768)
							{
								// 1360x768 60Hz (VESA)        --> 35
								// 925FS_107 108
								psrc->Modesetting.u8ModeIndex			= 35;
								psrc->Modesetting.u16HorizontalStart	= 0x154;
								psrc->Modesetting.u16VerticalStart		= 0x14;
								psrc->Modesetting.u16HorizontalTotal	= 1776;
								psrc->Modesetting.u16VerticalTotal		= 798;
								psrc->Modesetting.u8Phase				= 0x1d;

								psrc->Modesetting.u8SyncStatus			= SC_PCMODE_FLAG_POR_HPVP;
								psrc->Modesetting.u8AutoSign			= 0;

								continue;
							}
						}
						else if (SysValPCResolution == 3)	// 1366x768
						{
							if(pStdModeTbl->u8ResIdx != RES_1366X768)
							{
								// 1366x768 60Hz (VESA)        --> 37
								// 925FS_106
								psrc->Modesetting.u8ModeIndex			= 37;
								psrc->Modesetting.u16HorizontalStart	= 0x6e;
								psrc->Modesetting.u16VerticalStart		= 0xd;
								psrc->Modesetting.u16HorizontalTotal	= 1528;
								psrc->Modesetting.u16VerticalTotal		= 790;
								psrc->Modesetting.u8Phase				= 0x28;

								psrc->Modesetting.u8SyncStatus			= SC_PCMODE_FLAG_POR_HNVN;
								psrc->Modesetting.u8AutoSign			= 0;

								continue;
							}
						}
					}
				}
				//
				// determined the signal and copy to variable.
                psrc->Modesetting.u8ModeIndex			= u8MatchIndex;
                psrc->Modesetting.u16HorizontalStart	= pStdModeTbl->u16HStart;
                psrc->Modesetting.u16VerticalStart		= pStdModeTbl->u16VStart;
                psrc->Modesetting.u16HorizontalTotal	= pStdModeTbl->u16HTotal;
                psrc->Modesetting.u16VerticalTotal		= pStdModeTbl->u16VTotal;
                psrc->Modesetting.u8Phase				= pStdModeTbl->u8AdcPhase;

                psrc->Modesetting.u8SyncStatus			= pStdModeTbl->u8StatusFlag;
                psrc->Modesetting.u8AutoSign			= 0;
#if 0
                printk("****** ModeIdx=%u, HStart=%u, VStart=%u, HTotal=%u, VTotal=%u, SyncStatus=%u, AutoSign=%u\n",
                        psrc->Modesetting.u8ModeIndex,
                        psrc->Modesetting.u16HorizontalStart,
                        psrc->Modesetting.u16VerticalStart,
                        psrc->Modesetting.u16HorizontalTotal,
                        psrc->Modesetting.u16VerticalTotal,
                        psrc->Modesetting.u8SyncStatus,
                        psrc->Modesetting.u8AutoSign
                );
#endif				
			}
        }
    }

    if (u8MatchIndex != 0xFF)
    {
        return psrc->Modesetting.u8ModeIndex;
    }
    else
    {
        //-------------------
        // Second run match
        //-------------------
        /*lachesis_081202
		second match의 목적은
		standard mode table에서 match되는 mode가 없을 경우, 가장 유사한 mode를 찾도록 한다.
             */
        SC_PCMODE_DEBUG(" #####YKK Second Run Match ##### \n\n");
        u8MatchIndex = 0xFF;
        u16MatchDelta = SC_PCMODE_FREQ_DELTA;
        u16VTotalMatchDelta = 0xFFFF;

		pStdModeTbl = g_ModeTablePtr;	//lachesis_081202 initialize!!!

        for (u8Index = 0; u8Index < g_u16ModeTableSize; u8Index++, pStdModeTbl++)
        {
            u8Flag = pStdModeTbl->u8StatusFlag;

            if (bYPbPrSource)
            {
                if (!(u8Flag & SC_PCMODE_FLAG_YPBPR_BIT))
                {
                    continue;
                }
            }

            if ((u8Flag & SC_PCMODE_FLAG_INTERLACE) != (pModeTable->u8StatusFlag & SC_PCMODE_FLAG_INTERLACE))
            {
                continue;
            }

            // match H/V frequency
            u16HFreqDelta  = MDrv_SC_SubtractABS(pModeTable->u16HFreq, pStdModeTbl->u16HFreq);
            u16VFreqDelta  = MDrv_SC_SubtractABS(pModeTable->u16VFreq, pStdModeTbl->u16VFreq);
			u16HVFreqDalta = u16HFreqDelta + u16VFreqDelta;
            u16VTotalDelta = MDrv_SC_SubtractABS(pModeTable->u16VTotal, pStdModeTbl->u16VTotal);

			//printk("hfreq=%d, vfreq=%d, vtotal=%d\n", pStdModeTbl->u16HFreq, pStdModeTbl->u16VFreq, pStdModeTbl->u16VTotal);

			if ( pModeTable->u16VTotal > astStandardModeResolution[pStdModeTbl->u8ResIdx].u16DisplayHeight)
            {
                // calculate frequency difference
                if (((u16MatchDelta > u16HVFreqDalta) && (u16VTotalMatchDelta > u16VTotalDelta)) ||
                    ((u16HVFreqDalta <= 10) && (u16VTotalDelta < u16VTotalMatchDelta)))
                {
                    u16MatchDelta       = u16HVFreqDalta;
                    u16VTotalMatchDelta = u16VTotalDelta;
                    u8MatchIndex        = u8Index;

					psrc->Modesetting.u8ModeIndex			= u8MatchIndex;
					psrc->Modesetting.u16HorizontalStart	= pStdModeTbl->u16HStart;
					psrc->Modesetting.u16VerticalStart		= pStdModeTbl->u16VStart;
					psrc->Modesetting.u16HorizontalTotal	= pStdModeTbl->u16HTotal;
					psrc->Modesetting.u16VerticalTotal		= pStdModeTbl->u16VTotal;
					psrc->Modesetting.u8Phase				= pStdModeTbl->u8AdcPhase;

					psrc->Modesetting.u8SyncStatus			= pStdModeTbl->u8StatusFlag;
					psrc->Modesetting.u8AutoSign			= 0;
                }
            }
        }
    }

    return u8MatchIndex;
}

void MDrv_SC_PCMode_SetResolutionIndex(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg) // swwoo LGE 080626
{
    U32 param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(U32)))
    {
        return;
    }
    SysValPCResolution = (U16)param;
}
//thchen 20080729
void MDrv_SC_SetOffset(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_OFFSET_VALUE_t param;
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_OFFSET_VALUE_t)))
    {
        return;
    }

    // 20090903 daniel.huang: add/fix set/get gain/offset function
    //printk("MDrv_SC_SetOffset[R = 0x%x, G = 0x%x, B = 0x%x]\n",param.u16ROffsetValue, param.u16GOffsetValue, param.u16BOffsetValue);
    MHal_ADC_SetOffset(ADC_COLOR_RED,   param.u16ROffsetValue);
    MHal_ADC_SetOffset(ADC_COLOR_GREEN, param.u16GOffsetValue);
    MHal_ADC_SetOffset(ADC_COLOR_BLUE,  param.u16BOffsetValue);
}
//thchen 20080729
void MDrv_SC_SetGain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GAIN_VALUE_t param;
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GAIN_VALUE_t)))
    {
        return;
    }

    // 20090903 daniel.huang: add/fix set/get gain/offset function
    //printk("MDrv_SC_SetGain[R = 0x%x, G = 0x%x, B = 0x%x]\n", param.u16RGainValue, param.u16GGainValue, param.u16BGainValue);
    MHal_ADC_SetGain(ADC_COLOR_RED,     param.u16RGainValue);
    MHal_ADC_SetGain(ADC_COLOR_GREEN,   param.u16GGainValue);
    MHal_ADC_SetGain(ADC_COLOR_BLUE,    param.u16BGainValue);
}
//thchen 20080729
void MDrv_SC_GetOffset(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_OFFSET_VALUE_t param;
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_OFFSET_VALUE_t)))
    {
        return;
    }

    // 20090903 daniel.huang: add/fix set/get gain/offset function
    MHal_ADC_GetOffset(ADC_COLOR_RED,   &param.u16ROffsetValue);
    MHal_ADC_GetOffset(ADC_COLOR_GREEN, &param.u16GOffsetValue);
    MHal_ADC_GetOffset(ADC_COLOR_BLUE,  &param.u16BOffsetValue);    
    //printk("MDrv_SC_GetOffset[R = 0x%x, G = 0x%x, B = 0x%x]\n", param.u16ROffsetValue, param.u16GOffsetValue, param.u16BOffsetValue);

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_OFFSET_VALUE_t)))
    {
        return;
    }
}
//thchen 20080729
void MDrv_SC_GetGain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GAIN_VALUE_t param;
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GAIN_VALUE_t)))
    {
        return;
    }
    
    // 20090903 daniel.huang: add/fix set/get gain/offset function
    MHal_ADC_GetGain(ADC_COLOR_RED,     &param.u16RGainValue);
    MHal_ADC_GetGain(ADC_COLOR_GREEN,   &param.u16GGainValue);
    MHal_ADC_GetGain(ADC_COLOR_BLUE,    &param.u16BGainValue);
    //printk("MDrv_SC_GetGain[R = 0x%x, G = 0x%x, B = 0x%x]\n", param.u16RGainValue, param.u16GGainValue, param.u16BGainValue);

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GAIN_VALUE_t)))
    {
        return;
    }
}

// shjang_091006 (20091006 ykkim5)
void MDrv_SC_SetCompSyncLevel(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
	U8 param=0;
		
    if (copy_from_user(&param, (void __user *)arg, sizeof(U8)))
    {
        return;
    }
	//(ykkim5) User value: 0~31,  Register mask: 0~255
	// Available Register setting are 0,8,16....248 / Default Register is 0x90( User value : 18)
	param = param * 8;

	SC_PCMODE_DEBUG("  ### Real Sync Level Value =%d ### \n",param);
	MHal_ADC_SetSOGThreshold(param);
}
//--------------------------------------------------------------------
//ykkim5 091122
//-------------------------------------------------------------------
void MDrv_SC_PCMode_GetInfo_FromReg(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
	U8 u8InputSyncStatus;
	U16 u16HPeriod, u16VTotal;
	U16 u16HFreq, u16VFreq;	
	printk("	*** READ: 1.Hperiod  2.Vtotal  3.SyncStatus, Calculate:H/Vfreq ***\n");
	printk("    ==> DEBOUNCE & SYNC DETECT   [SC_SYNCSTS_HSYNC_POL_BIT] <==\n");
	u8InputSyncStatus = MDrv_SC_IP1_GetInputSyncStatus();
	printk(" InputSyncStatus[BIT0:V, 1:H POL, 2:Hloss, 3:Vloss, 4:INTERLACE] = 0x%x\n",u8InputSyncStatus);
	printk("    ==> SEARCH <==\n");
	u16HPeriod = MHal_SC_IP1_GetHPeriod();
	u16VTotal  = MHal_SC_IP1_GetVTotal();
	// calculate H/V frequency
	u16HFreq = MDrv_SC_CalculateHFreqX10(u16HPeriod);
	u16VFreq = MDrv_SC_CalculateVFreqX10(u16HFreq, u16VTotal);
	printk(" u16Input_HFreq:%d,     u16Input_VFreq:%d, u16Input_VTotal:%d\n",u16HFreq,u16VFreq,u16VTotal);
	printk(" InputSyncStatus = 0x%x,  u16HPeriod:%d\n",u8InputSyncStatus,u16HPeriod);
	printk("    ==> CHECK STABLE <==\n");
	printk(" InputSyncStatus = 0x%x,  u16HPeriod:%d,     u16Input_VTotal:%d\n",u8InputSyncStatus,u16HPeriod,u16VTotal);
	printk("-----------------------------\n");
}

