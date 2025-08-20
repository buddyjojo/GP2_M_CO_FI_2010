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
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "mst_utility.h"
#include "mdrv_types.h"
#include "mdrv_auto.h"
#include "mdrv_adc.h"
#include "mdrv_scaler_pcmode.h"
#include "mdrv_scaler.h"
#include "mhal_scaler.h"
#include "mhal_auto.h"
#include "mhal_adc.h"
#include "mhal_scaler_reg.h"
#include "mhal_utility.h"
#include "mhal_adc_reg.h"

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures Start
//-------------------------------------------------------------------------------------------------
extern unsigned long irq_flags_swbk;
extern spinlock_t switch_bk_lock;
extern U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------


#define ADC_MEMAVG_WITHOUT_MAXMIN 0
#define ADC_READ_PATTERN_FROM_EXTERNAL_SIGNAL 0

#define CAL_WINDOW_SIZE 32
enum {
    DOWN_TOP= 1,
    TOP_DOWN= 2,
};

//victor 20080826
#define AUTOMSG(x)  //x
/******************************************************************************/
/*                     Local                    */
/* ****************************************************************************/
// 20091012 daniel.huang: for finetune internal calibration
BOOL MDrv_Auto_Geometry(PSC_DRIVER_CONTEXT_t pDrvCtx, PSC_SOURCE_INFO_t psrc, SC_AUTO_TUNE_TYPE_e AutoTuneType, 
                        U16 TargetForRGain, U16 TargetForGGain, U16 TargetForBGain)
{
    U8   u8VSyncTime;   // VSync time
    BOOL bAutoResult = TRUE;

	AUTOMSG( printk("-> MDrv_Auto_Geometry\n"));

    u8VSyncTime = MDrv_SC_CalculateVSyncTime(psrc->u16Input_VTotal, psrc->u16Input_HPeriod);
    AUTOMSG(printk("u8VSyncTime = %d\n", u8VSyncTime));

	MHal_Auto_SetValidThreadhold(0x40); // 20091107 daniel.huang: refine code (ykkim5 091112 autoconfig)

    // auto horizontal total
    if (AutoTuneType & AUTO_TUNE_FREQ)
    {
        if (MDrv_Auto_TuneHTotal(psrc, u8VSyncTime) == FALSE)
        {
            AUTOMSG(printk("Htotal no need\n"));
            bAutoResult = FALSE;
        }
    }

    // auto phase
    if (AutoTuneType & AUTO_TUNE_PHASE)
    {
        if (MDrv_Auto_TunePhase(psrc, u8VSyncTime) == FALSE)
        {
            AUTOMSG(printk("AUTO_TUNE_PHASE no need\n"));
            bAutoResult = FALSE;
        }
    }

    // auto position
    if (AutoTuneType & AUTO_TUNE_POSITION)
    {
        if (MDrv_Auto_TunePosition(psrc, u8VSyncTime) == FALSE)
        {
            AUTOMSG(printk("AUTO_TUNE_POSITION no need\n"));
            bAutoResult = FALSE;
        }
    }

    if(AutoTuneType & AUTO_TUNE_GAIN)
    {
        // 20091012 daniel.huang: for finetune internal calibration
        U16 GainTarget[3];
        GainTarget[0] = TargetForRGain;
        GainTarget[1] = TargetForGGain;
        GainTarget[2] = TargetForBGain;
        AUTOMSG(printk("AutoTuneType=0x%x, Target R=%u, G=%u, B=%u\n", AutoTuneType, TargetForRGain, TargetForGGain, TargetForBGain));

        if(AutoTuneType & AUTO_TUNE_INT_ADC)
        {
            AUTO_SRC_INFO_t SrcBackup;
            AUTO_TIMING_INFO_t TimingBackup;
            BOOL result;
            AUTO_CAL_WINDOW_t CalWin;

            MHal_Auto_Save_Source(&SrcBackup);
            MHal_Auto_Save_Timing(&TimingBackup);

            MDrv_ADC_SetMode(Use_VGA_Source(psrc->SrcType) ? TRUE:FALSE, 5); // Set ADC input LPF to low bandwidth (5MHz)
            MHal_Auto_ADC_Setting();
            MHal_Auto_ADC_720p_Timing();
            MDrv_ADC_ADCCal(FALSE); // do mismatch calibration
            MHal_Auto_IPMUX_720p_Timing();
            MHal_Auto_SC_720p_Setting(pDrvCtx->u32MemAddr, pDrvCtx->u32MemSize);    // 2009/09/23 daniel.huang: for calibration with mirror

            /*  // 20091012 daniel.huang: for finetune internal calibration
            if (AutoTuneType & AUTO_TUNE_YUV)
            {
                AUTOMSG(printk("YUV\n"));
                GainTarget[0] = AUTO_GAIN_RANGE_CBCR;
                GainTarget[1] = AUTO_GAIN_RANGE_Y;
                GainTarget[2] = AUTO_GAIN_RANGE_CBCR;
            }
            else
            {
                AUTOMSG(printk("RGB\n"));
                GainTarget[0] = AUTO_GAIN_RANGE_RGB;
                GainTarget[1] = AUTO_GAIN_RANGE_RGB;
                GainTarget[2] = AUTO_GAIN_RANGE_RGB;
            }*/

            // CalWin: 0, 0, 0x280, 0x1c0
            CalWin.x=(0x280-0x20)/2;
            CalWin.y=(0x1c0-0x20)/2;
            CalWin.width = 0x20;
            CalWin.height= 0x20;

            // 20090928 daniel.huang: refine ADC calibration and update calibration algorithm
            result = MDrv_ADC_CalGain(AutoTuneType, psrc,
                                      GainTarget,
                                      &CalWin,    // for internal calibration, use pCalWinG_Y only
                                      &CalWin,
                                      &CalWin);

            if(result == FALSE)
            {
                AUTOMSG(printk("Calibration failed!\n"));
                bAutoResult = FALSE;
            }

            MHal_ADC_InternalDc( INTERNAL_NONE );
            MHal_Auto_Restore_Source(&SrcBackup);
            MHal_Auto_Restore_Timing(&TimingBackup);
            MHal_SC_Reset(SC_RST_IP_F2);

        }
        else    // external calibration
        {
            AUTO_SRC_INFO_t SrcBackup;
            AUTO_TIMING_INFO_t TimingBackup;
            BOOL result;
            // Calibration Pattern
            // Vertical Color Bar: white/yellow/cyan/green/pink/red/blue/black (Progressive)
            AUTO_CAL_WINDOW_t CalWinG_Y, CalWinB_Cb, CalWinR_Cr;

            MHal_Auto_Save_Source(&SrcBackup);
            MHal_Auto_Save_Timing(&TimingBackup);

            //20090917 daniel.huang: fix component 1080p external calibration fail
            ////MDrv_ADC_SetMode(Use_VGA_Source(psrc->SrcType) ? TRUE:FALSE, 5); // Set ADC input LPF to low bandwidth (5MHz)
            MDrv_ADC_ADCCal(FALSE); // do mismatch calibration

            if (!(AutoTuneType & AUTO_TUNE_YUV))
            {
                MHal_Auto_SetVIP_CSC_Enable();
                AUTOMSG(printk("Set VIP CSC\n"));
            }

            MHal_Auto_SC_Ext_Setting(pDrvCtx->u32MemAddr, pDrvCtx->u32MemSize,  // 2009/09/23 daniel.huang: for calibration with mirror
                                    (MHal_SC_IP1_GetDetectSyncStatus() & BIT11) ? TRUE:FALSE);

            if (AutoTuneType & AUTO_TUNE_YUV)
            {
                AUTOMSG(printk("YUV\n"));
                //GainTarget[0] = AUTO_GAIN_TARGET_CBCR;// 20091012 daniel.huang: for finetune internal calibration
                //GainTarget[1] = AUTO_GAIN_TARGET_Y;   // 20091012 daniel.huang: for finetune internal calibration
                //GainTarget[2] = AUTO_GAIN_TARGET_CBCR;// 20091012 daniel.huang: for finetune internal calibration
                // use white as pattern (Y: target 940)
                CalWinG_Y.x = 0 * psrc->u16H_CapSize/8 + psrc->u16H_CapSize/16 - 8;
                CalWinG_Y.y = psrc->u16V_CapSize/4  - 8;
                CalWinG_Y.width  = 16;
                CalWinG_Y.height = 16;
                // use red as pattern   (Cr: target 960)
                CalWinR_Cr.x = 5 * psrc->u16H_CapSize/8 + psrc->u16H_CapSize/16 - 8;
                CalWinR_Cr.y = psrc->u16V_CapSize/4  - 8;
                CalWinR_Cr.width  = 16;
                CalWinR_Cr.height = 16;
                // use blue as pattern  (Cb: target 960)
                CalWinB_Cb.x = 6 * psrc->u16H_CapSize/8 + psrc->u16H_CapSize/16 - 8;
                CalWinB_Cb.y = psrc->u16V_CapSize/4  - 8;
                CalWinB_Cb.width  = 16;
                CalWinB_Cb.height = 16;
            }
            else
            {
                AUTOMSG(printk("RGB\n"));
                //GainTarget[0] = AUTO_GAIN_TARGET_RGB;// 20091012 daniel.huang: for finetune internal calibration
                //GainTarget[1] = AUTO_GAIN_TARGET_RGB;// 20091012 daniel.huang: for finetune internal calibration
                //GainTarget[2] = AUTO_GAIN_TARGET_RGB;// 20091012 daniel.huang: for finetune internal calibration
                // use white as pattern
                CalWinG_Y.x = 0 * psrc->u16H_CapSize/8 + psrc->u16H_CapSize/16 - 8;
                CalWinG_Y.y = psrc->u16V_CapSize/4  - 8;
                CalWinG_Y.width  = 16;
                CalWinG_Y.height = 16;
                // use white as pattern
                CalWinR_Cr.x = 0 * psrc->u16H_CapSize/8 + psrc->u16H_CapSize/16 - 8;
                CalWinR_Cr.y = psrc->u16V_CapSize/4  - 8;
                CalWinR_Cr.width  = 16;
                CalWinR_Cr.height = 16;
                // use white as pattern
                CalWinB_Cb.x = 0 * psrc->u16H_CapSize/8 + psrc->u16H_CapSize/16 - 8;
                CalWinB_Cb.y = psrc->u16V_CapSize/4  - 8;
                CalWinB_Cb.width  = 16;
                CalWinB_Cb.height = 16;
            }

            // 20090928 daniel.huang: refine ADC calibration and update calibration algorithm
            result = MDrv_ADC_CalGain(AutoTuneType, psrc,
                                      GainTarget,
                                      &CalWinG_Y,
                                      &CalWinB_Cb,
                                      &CalWinR_Cr);
            if(result == FALSE)
            {
                AUTOMSG(printk("Calibration failed!\n"));
                bAutoResult = FALSE;
            }

            MHal_Auto_Restore_Source(&SrcBackup);
            MHal_Auto_Restore_Timing(&TimingBackup);
            MHal_SC_Reset(SC_RST_IP_F2);
        }
    }

    return bAutoResult;
}

//*************************************************************************
//Function name: MDrv_Auto_GetPosition
//Passing parameter:
//  U8 u8RegIndex: Register index
//  U8 u8VSyncTime: VSync time
//Return parameter:
//  U16: Position value.
//Description: Get position by register select
//*************************************************************************
U16 MDrv_Auto_GetPosition(PSC_SOURCE_INFO_t psrc, U8 u8RegIndex, U8 u8VSyncTime)
{
    U16 u16ComparePos = 0, u16AutoPos = 0; // position buffer, Add the initial value
    U8  u8Dummy = 20;   // loop dummy
    U8  u8Count = 0;    // counter of compare alike

    while (u8Dummy--)
    {
        MHal_Auto_WaitStatusReady(0x10, BIT1); // auto position result ready

        //u16AutoPos = REG_RR(u8RegIndex) & 0x0FFF; // get auto position
        u16AutoPos = MHal_Auto_GetAutoPosition(u8RegIndex);
        if (u16AutoPos == u16ComparePos) // match
        {
            u8Count++;
        }
        else // different
        {
            u8Count = 0; // reset counter
            u16ComparePos = u16AutoPos; // reset position
        }

        if (u8Count == 3) // match counter ok
        {
            break;
        }

        if (MDrv_Auto_CheckSyncLoss(psrc)) // check no signal
        {
            return -1; // return fail
        }

        OS_Delayms(u8VSyncTime); // wait next frame
    } // while

    return u16AutoPos;
}

//*************************************************************************
//Function name: MDrv_Auto_CheckSyncLoss
//Passing parameter: NO
//Return parameter:
//  BOOL: If PC mode change, return TRUE.
//Description: Check PC mode change when auto-tune executive.
//*************************************************************************
BOOL MDrv_Auto_CheckSyncLoss(PSC_SOURCE_INFO_t psrc)
{
    U16  u16SyncCounter;
    BOOL bResult = FALSE;

    if ((MDrv_SC_IP1_GetInputSyncStatus() & SC_SYNCSTS_SYNC_LOSS) != 0)
    {
        bResult = TRUE;
    }

    // check H/VSync change
    // HSync
    u16SyncCounter = MHal_SC_IP1_GetHPeriod();

    if (MDrv_Calculate_ABS(u16SyncCounter, psrc->u16Input_HPeriod) > MD_HPERIOD_TORLANCE)
    {
        bResult = TRUE;
    }

    // VSync
    u16SyncCounter = MHal_SC_IP1_GetVTotal();

    if (MDrv_Calculate_ABS(u16SyncCounter, psrc->u16Input_VTotal) > MD_VTOTAL_TORLANCE)
    {
        bResult = TRUE;
    }

    return bResult;
}

//*************************************************************************
//Function name: MDrv_Auto_TuneHTotal
//Passing parameter:
//  U8 u8VSyncTime: VSync time
//  MS_PCADC_MODESETTING_TYPE *pstModesetting: Current PC mode setting
//Return parameter:
//  BOOL: Success status. (If faile, return FALSE.)
//Description: auto-tune horizontal total.
//*************************************************************************
//FitchHsu 20081127 PCmode Auto config issue
BOOL MDrv_Auto_TuneHTotal(PSC_SOURCE_INFO_t psrc, U8 u8VSyncTime)
{
    U16	wTempSrcHTotal;
	U16	semWidth;
	U16	wStdModeHtotal;
	U16	wStdModeWidth;
	U32	curDiffSum = 0, maxDiffSum = 0;
	U16	bestClock = 0, i, j;
	U8	tryCount = 3;
//	U16	u16OriSrcHTotal; // FitchHsu 20081224 VGA 800x600 auto config fail

	if(MDrv_Auto_CheckSyncLoss(psrc))
	{
		return FALSE;
	}

    wTempSrcHTotal = psrc->Modesetting.u16HorizontalTotal; // intialize horizontal total buffer
//    u16OriSrcHTotal = wTempSrcHTotal; // FitchHsu 20081224 VGA 800x600 auto config fail

    while(tryCount)
	{
		semWidth = MDrv_Auto_GetActualWidth(psrc, u8VSyncTime);
		wStdModeWidth = MDrv_SC_PCMode_GetStdModeResH(psrc->Modesetting.u8ModeIndex);
		wStdModeHtotal = MDrv_SC_PCMode_GetStdModeHTotal(psrc->Modesetting.u8ModeIndex);

		AUTOMSG(printk("-> step1 semWidth=%d, wStdModeWidth=%d wStdModeHtotal=%d\n",
						semWidth, wStdModeWidth, wStdModeHtotal));


/*		lachesis modetable檜 醱碟ж雖 彊戲嘎煎 橾欽 瞳辨 爾盟.
		if( (semWidth > (wStdModeWidth + MaxMeasuredWidthDelta)) ||
			(semWidth < (wStdModeWidth - MaxMeasuredWidthDelta)) )
		{
			return AUTO_RESULT_CONTINUE;	//set to default.
		}
		else
*/
		{
			if(semWidth != wStdModeWidth)
			{
				wTempSrcHTotal = ((U32) psrc->Modesetting.u16HorizontalTotal * wStdModeWidth) / semWidth;
				wTempSrcHTotal = wTempSrcHTotal & 0xFFFE;
				AUTOMSG(printk("-> step2 semWidth and stdwidth diffrent new htotal=%d\n", wTempSrcHTotal));
			}
			/*
			if(wTempSrcHTotal > HTOTAL_ADJUST_MAX || wTempSrcHTotal < HTOTAL_ADJUST_MIN)
			{
				wTempSrcHTotal = wStdModeHtotal;
			}
			*/

			psrc->Modesetting.u16HorizontalTotal = wTempSrcHTotal;
			MHal_ADC_SetADCClk(psrc->Modesetting.u16HorizontalTotal);
			OS_Delayms(40); //lkw 0812 10-> 40
		}

		if(MDrv_Auto_CheckSyncLoss(psrc))
		{
			return FALSE;
		}

		semWidth = MDrv_Auto_GetActualWidth(psrc, u8VSyncTime);

		if( (semWidth >= (wStdModeWidth - MIN_SRC_WIDTH_DELTA)) &&
			(semWidth <= (wStdModeWidth + MAX_SRC_WIDTH_DELTA)) )
		{
			AUTOMSG(printk("-> step3 range in go to finetune\n" ));
			break;
		}
		else
		{
			AUTOMSG(printk("-> step3 range over retry\n" ));
		}

		tryCount--; // 080301_kimhw_01
	}

	if (tryCount == 0)
	{
		psrc->Modesetting.u16HorizontalTotal = wStdModeHtotal;
		MHal_ADC_SetADCClk(psrc->Modesetting.u16HorizontalTotal);
		AUTOMSG(printk("-> find htotal error !! set to default %d\n",  psrc->Modesetting.u16HorizontalTotal));
		return FALSE;
	}

	if( (psrc->Modesetting.u16HorizontalTotal != wStdModeHtotal) ||(semWidth != wStdModeWidth) )
	{
		maxDiffSum = 0;
		wTempSrcHTotal = psrc->Modesetting.u16HorizontalTotal - MIN_SRC_WIDTH_DELTA;

		for(i = 0; i < TOT_SRC_WIDTH_DELTA; i++)
		{
			MHal_ADC_SetADCClk(wTempSrcHTotal);
			OS_Delayms(10);

			for(j = PHASE_MIN; j <= PHASE_MAX; j += PHASE_STEP)
			{
				if(MDrv_Auto_CheckSyncLoss(psrc))
					return FALSE;

				MHal_ADC_SetADCPhase(j);

				OS_Delayms(20);//lkw 0812 30->40

				curDiffSum = MHal_Auto_GetPhaseVal();


				if(curDiffSum > maxDiffSum)
				{
					maxDiffSum = curDiffSum;
					bestClock = i;
					//AUTOMSG(printk("-> best clock=%d\n", bestClock));
				}
			}
			wTempSrcHTotal++;
		}

		psrc->Modesetting.u16HorizontalTotal = psrc->Modesetting.u16HorizontalTotal - MIN_SRC_WIDTH_DELTA + bestClock;

#if 0	//side effect嫦儅ж罹 歜衛煎 虞擠
	// FitchHsu 20081224 VGA 800x600 auto config fail
	if (psrc->Modesetting.u16HorizontalTotal != u16OriSrcHTotal)
    	{
    		psrc->Modesetting.u16HorizontalTotal = u16OriSrcHTotal;
    		MHal_ADC_SetADCClk(psrc->Modesetting.u16HorizontalTotal);
    		AUTOMSG(printk("->Fitch find htotal error !! set to default %d\n",  psrc->Modesetting.u16HorizontalTotal);)
    		return FALSE;
    	}
#endif
		AUTOMSG(printk("-> final clock=%d\n", psrc->Modesetting.u16HorizontalTotal));

		MHal_ADC_SetADCClk(psrc->Modesetting.u16HorizontalTotal);
	}

    return TRUE;
}

//*************************************************************************
//Function name: MDrv_Auto_TunePhase
//Passing parameter:
//  U8 u8VSyncTime: VSync time
//  MS_PCADC_MODESETTING_TYPE *pstModesetting: Current PC mode setting
//Return parameter:
//  BOOL: Success status. (If faile, return FALSE.)
//Description: auto-tune phase.
//*************************************************************************
//(ykkim5 091112 autoconfig) #define AUTO_PHASE_STEP 4

BOOL MDrv_Auto_TunePhase(PSC_SOURCE_INFO_t psrc, U8 u8VSyncTime)
{
    U8  u8Index; // loop index
    U32 u32AutoPhaseVal; // auto phase value result

    //AUTOMSG(printk("-> MDrv_Auto_TunePhase\n"));

    #if AUTO_PHASE_METHOD
    {
        U32 u32MiniPhaseVal = -1; // minimum phase value
        U8 u8WorstPhase1,u8WorstPhase2;

        u8WorstPhase1 = 0x00; // initizlize
        for (u8Index = u8WorstPhase1; u8Index <= PHASE_MAX; u8Index += AUTO_PHASE_STEP)   // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7) (ykkim5 091112 autoconfig)
        {
            MHal_ADC_SetADCPhase(u8Index);
            OS_Delayms(u8VSyncTime); // delay 1 frame
            u32AutoPhaseVal = MHal_Auto_GetPhaseVal();

            if (u32AutoPhaseVal < u32MiniPhaseVal) // check minimum
            {
                u8WorstPhase1 = u8Index; // refresh best phase
                u32MiniPhaseVal = u32AutoPhaseVal; // refresh minimum value
            }

            if (MDrv_Auto_CheckSyncLoss(psrc)) // check no signal
            {
                return FALSE;
            }
        }

        // initizlize
        u8WorstPhase2 = (u8WorstPhase1 - AUTO_PHASE_STEP + 1) & PHASE_MAX;    // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112)
        u8WorstPhase1 = (u8WorstPhase1 + AUTO_PHASE_STEP) & PHASE_MAX;        // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112)
        u32MiniPhaseVal = -1;
        for (u8Index = u8WorstPhase2; u8Index != u8WorstPhase1; u8Index = ((u8Index + 1) & PHASE_MAX))    // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112)
        {
            MHal_ADC_SetADCPhase(u8Index);
            OS_Delayms(u8VSyncTime); // delay 1 frame
            u32AutoPhaseVal = MHal_Auto_GetPhaseVal();

            if (u32AutoPhaseVal < u32MiniPhaseVal) // check minimum
            {
                u8WorstPhase2 = u8Index; // refresh best phase
                u32MiniPhaseVal = u32AutoPhaseVal; // refresh minimum value
            }

            if (MDrv_Auto_CheckSyncLoss(psrc)) // check no signal
                return FALSE;
        }

        psrc->Modesetting.u8Phase = (u8WorstPhase2 + (PHASE_MAX / 2)) & PHASE_MAX;  // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112)
    }
    #else
    {
        U32 u32MaxPhaseVal = 0; // maximum phase value
        U8  u8BestPhase1, u8BestPhase2;

        u8BestPhase1 = 0x00; // initizlize
        for (u8Index = u8BestPhase1; u8Index <= PHASE_MAX; u8Index += AUTO_PHASE_STEP)    // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112)
        {
            MHal_ADC_SetADCPhase(u8Index);
            OS_Delayms(u8VSyncTime); // delay 1 frame
            u32AutoPhaseVal = MHal_Auto_GetPhaseVal();

            if (u32AutoPhaseVal > u32MaxPhaseVal) // check maximum
            {
                u8BestPhase1   = u8Index; // refresh best phase
                u32MaxPhaseVal = u32AutoPhaseVal; // refresh maximum value
            }

            if (MDrv_Auto_CheckSyncLoss(psrc)) // check no signal
            {
				AUTOMSG(printk("-> MDrv_Auto_TunePhase MDrv_Auto_CheckSyncLoss\n"));
                return FALSE;
            }
        }
		AUTOMSG(printk("-> MDrv_Auto_TunePhase first best phase = %d\n", u8BestPhase1));
        // initizlize
        u8BestPhase2 = (u8BestPhase1 - AUTO_PHASE_STEP + 1) & PHASE_MAX;  // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112)
        u8BestPhase1 = (u8BestPhase1 + AUTO_PHASE_STEP) & PHASE_MAX;      // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112)
        u32MaxPhaseVal = 0;

		AUTOMSG(printk("-> MDrv_Auto_TunePhase min%d to max%d\n", u8BestPhase2, u8BestPhase1));


        for (u8Index = u8BestPhase2; u8Index != u8BestPhase1; u8Index = ((u8Index + 1) & PHASE_MAX))  // 20091107 dnaiel.huang: fix max phase range from 0x3F(S6) to 0x7F(S7)(ykkim5 091112)
        {
			//AUTOMSG(printk("-> MDrv_Auto_TunePhase final phase index= %d\n", u8Index);)
            MHal_ADC_SetADCPhase(u8Index);
            OS_Delayms(u8VSyncTime); // delay 1 frame
            u32AutoPhaseVal = MHal_Auto_GetPhaseVal();

            if (u32AutoPhaseVal > u32MaxPhaseVal) // check maximum
            {
                u8BestPhase2 = u8Index; // refresh best phase
                u32MaxPhaseVal = u32AutoPhaseVal; // refresh maximum value
            }

            if (MDrv_Auto_CheckSyncLoss(psrc)) // check no signal
            {
				AUTOMSG(printk("-> MDrv_Auto_TunePhase 2 MDrv_Auto_CheckSyncLoss\n"));
                return FALSE;
            }
        }

        psrc->Modesetting.u8Phase = u8BestPhase2;
		AUTOMSG(printk("-> MDrv_Auto_TunePhase final Modesetting.u8Phase !!!= %d\n", psrc->Modesetting.u8Phase));
    }
    #endif

    MHal_ADC_SetADCPhase(psrc->Modesetting.u8Phase);

    return TRUE;
}


//*************************************************************************
//Function name: MDrv_Auto_TunePosition
//Passing parameter:
//  U8 u8VSyncTime : VSync time
//  MS_PCADC_MODESETTING_TYPE *pstModesetting: Current PC mode setting
//Return parameter:
//  BOOL: Success status. (If faile, return FALSE.)
//Description: Auto-tune vertical and horizontal position for PC mode
//*************************************************************************
BOOL MDrv_Auto_TunePosition(PSC_SOURCE_INFO_t psrc, U8 u8VSyncTime)
{
    U16  u16PosBff; // position buffer
    BOOL bResult = TRUE;

    AUTOMSG(printk("-> MDrv_Auto_TunePosition\n"));

    // horizotal position
    u16PosBff = MDrv_Auto_GetPosition(psrc, 0x13, u8VSyncTime); // auto horizontal start position detected result

    if (u16PosBff > MAX_PC_AUTO_H_START || u16PosBff < MIN_PC_AUTO_H_START)
    {
        u16PosBff = psrc->Modesetting.u16HorizontalStart;
        bResult = FALSE;
    }

    psrc->s16PC_H_POS= 0;	// 20080710 swwoo LGE
	psrc->Modesetting.u16HorizontalStart = u16PosBff;	// 20080710 swwoo LGE

    //MDrv_Write2Byte(L_BK_IP1F2(0x05), u16PosBff);
    MHal_SC_IP1_SetSampleHStart(u16PosBff);

    // vertical positoin
    u16PosBff = MDrv_Auto_GetPosition(psrc, 0x12, u8VSyncTime); // auto vertical start position detected result

    if (u16PosBff > MAX_PC_AUTO_V_START || u16PosBff < MIN_PC_AUTO_V_START)
    {
        u16PosBff = psrc->Modesetting.u16VerticalStart;
        bResult = FALSE;
    }
    psrc->s16PC_V_POS= 0;	// 20080710 swwoo LGE
	psrc->Modesetting.u16VerticalStart = u16PosBff;	// 20080710 swwoo LGE

    MHal_SC_IP1_SetSampleVStart(u16PosBff);

    return bResult;
}

//*************************************************************************
//Function name: MDrv_Auto_GetActualWidth
//Passing parameter:
//  U8 u8VSyncTime : VSync time
//Return parameter:
//  U16: return actual image width
//Description: Get actual image width.
//*************************************************************************
U16 MDrv_Auto_GetActualWidth(PSC_SOURCE_INFO_t psrc, U8 u8VSyncTime)
{
    U16 u16HStart; // actual horizontal start
    U16 u16Width;

    MHal_ADC_SetADCPhase(0x00); // initialize phase value
    u16HStart = MDrv_Auto_GetPosition(psrc, 0x13, u8VSyncTime); // horizontal start position
    //MDrv_Auto_GetTransPos(u8VSyncTime); // seek critical phase

    u16Width = ((MDrv_Auto_GetPosition(psrc, 0x15, u8VSyncTime) - u16HStart) + 1); // actual image width

    return u16Width;
}


/*
1. RGB mode, full scale還是維?10bit 1023這?code, 所以gain 還是??到讀到0.5/0.7*1023.
    Offset?是??到 0這?code, RGB三?channel 都?樣.
2. YCbCr mode, 請將Y的full scale改成10bit code 是(235-16)*4, Cb/Cr full scale 改成10bit (240-16)*4.
    所以Y gain?要調到使 digital code 看到 0.5/0.7*(235-16)*4, Cb/Cr則是0.5/0.7*(240-16)*4.
   Offset 則是將 Y 較準到64 (10bit), Cb/Cr 到512 (10bit).
*/

MS_AUTOADC_TYPE _MDrv_SC_GetAverageData(PIXEL_32BIT *sMemBuf, AUTO_CAL_WINDOW_t *pCalWin)
{
    MS_AUTOADC_TYPE ptAdcData;
    U32 u32Rtt, u32Gtt, u32Btt;
    U16 i, x, y, width, height;
    BOOL bRGB;

#if ADC_MEMAVG_WITHOUT_MAXMIN
    U32 tmp;
    U16 u16MaxR,u16MaxG,u16MaxB;
    U16 u16MinR,u16MinG,u16MinB;
    U16 u16NumMaxR,u16NumMaxG,u16NumMaxB;
    U16 u16NumMinR,u16NumMinG,u16NumMinB;
    U16 u16NoMinMaxAvgCb, u16NoMinMaxAvgY, u16NoMinMaxAvgCr;
#endif

    ptAdcData.CH_AVG[0] = ptAdcData.CH_AVG[1] = ptAdcData.CH_AVG[2] = 0;

    i = 0;
    u32Rtt = 0;
    u32Gtt = 0;
    u32Btt = 0;

#if ADC_MEMAVG_WITHOUT_MAXMIN
    u16MaxR = 0;
    u16MaxG = 0;
    u16MaxB = 0;
    u16MinR = 0;
    u16MinG = 0;
    u16MinB = 0;

    u16NumMaxR = 0;
    u16NumMaxG = 0;
    u16NumMaxB = 0;
    u16NumMinR = 0;
    u16NumMinG = 0;
    u16NumMinB = 0;

    u16NoMinMaxAvgCb = 0;
    u16NoMinMaxAvgY = 0;
    u16NoMinMaxAvgCr = 0;
#endif

#if ADC_READ_PATTERN_FROM_EXTERNAL_SIGNAL
    AUTOMSG(printk("Block Positon (%d, %d) size %dx%d\n",
        pCalWin->x,
        pCalWin->y,
        pCalWin->width,
        pCalWin->height));
    // Enable Debug Cross
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_DLC);
    REG_WI(REG_SC_DLC(0x68), ENABLE, BIT0);
    SC_BK_RESTORE;
    for(y=pCalWin->y; y<pCalWin->y+pCalWin->height; y++)
    {
        for(x=pCalWin->x; x<pCalWin->x+pCalWin->width; x++)
        {
            U16 u16Y, u16Cb, u16Cr;
            SC_BK_STORE;
            SC_BK_SWICH(REG_SC_BK_DLC);
            REG_WR(REG_SC_DLC(0x69), x);
            REG_WR(REG_SC_DLC(0x6A), y);
            SC_BK_RESTORE;

            msleep(10);

            u16Y = (REG_RR(REG_SC_DLC(0x6B)) & 0x03FF);
            u16Cb = (REG_RR(REG_SC_DLC(0x6C)) & 0x03FF);
            u16Cr = (REG_RR(REG_SC_DLC(0x6D)) & 0x03FF);

            //printk("Y = 0x%04x, Cb = 0x%04x, Cr = 0x%04x\n", u16Y, u16Cb, u16Cr);
            u32Rtt += u16Cr;
            u32Gtt += u16Y;
            u32Btt += u16Cb;
        }
    }
    // Disable Debug Cross
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_DLC);
    REG_WI(REG_SC_DLC(0x68), ENABLE, BIT0);
    SC_BK_RESTORE;

    ptAdcData.CH_AVG[0] = u32Rtt / (pCalWin->height * pCalWin->width);
    ptAdcData.CH_AVG[1] = u32Gtt / (pCalWin->height * pCalWin->width);
    ptAdcData.CH_AVG[2] = u32Btt / (pCalWin->height * pCalWin->width);

    AUTOMSG(printk("AvgCr AvgY AvgCb\n"));
    AUTOMSG(printk("%04d  %04d %04d\n\n", ptAdcData.CH_AVG[0], ptAdcData.CH_AVG[1], ptAdcData.CH_AVG[2]));


#else


    //AUTOMSG(printk("Mem Start (%d, %d) size %dx%d\n", pCalWin->x, pCalWin->y, pCalWin->width, pCalWin->height));

    x = pCalWin->x;
    y = pCalWin->y;
    width = pCalWin->width;
    height = pCalWin->height;
    bRGB = 0; // not used
    MDrv_SC_GetFrameDataCore(&x, &y, &width, &height, &bRGB,
                             (U8 *)sMemBuf,
                             pCalWin->width,
                             pCalWin->height * pCalWin->width * sizeof(PIXEL_32BIT),
                             sizeof(PIXEL_32BIT),
                             FALSE);

    for(y=pCalWin->y; y<pCalWin->y+pCalWin->height; y++)
    {
        for(x=pCalWin->x; x<pCalWin->x+pCalWin->width; x++)
        {
#if ADC_MEMAVG_WITHOUT_MAXMIN
            if (i == 0)
            {
                //printk("pu8VirPixelAdr 0x%lx\n", (U32)pu8VirPixelAdr);
                u16MaxR = sMemBuf[i].Cr;
                u16MinR = sMemBuf[i].Cr;
                u16MaxG = sMemBuf[i].Y;
                u16MinG = sMemBuf[i].Y;
                u16MaxB = sMemBuf[i].Cb;
                u16MinB = sMemBuf[i].Cb;
                u16NumMaxR = 1;
                u16NumMaxG = 1;
                u16NumMaxB = 1;
                u16NumMinR = 1;
                u16NumMinG = 1;
                u16NumMinB = 1;
                //while (1);
            }
            else
            {
                if (sMemBuf[i].Cr > u16MaxR)
                {
                    u16MaxR = sMemBuf[i].Cr;
                    u16NumMaxR = 1;
                }
                else if (sMemBuf[i].Cr == u16MaxR)
                {
                    u16NumMaxR++;
                }

                if (sMemBuf[i].Cr < u16MinR)
                {
                    u16MinR = sMemBuf[i].Cr;
                    u16NumMinR = 1;
                }
                else if (sMemBuf[i].Cr == u16MinR)
                {
                    u16NumMinR++;
                }


                if (sMemBuf[i].Y > u16MaxG)
                {
                    u16MaxG = sMemBuf[i].Y;
                    u16NumMaxG = 1;
                }
                else if (sMemBuf[i].Y == u16MaxG)
                {
                    u16NumMaxG++;
                }

                if (sMemBuf[i].Y < u16MinG)
                {
                    u16MinG = sMemBuf[i].Y;
                    u16NumMinG = 1;
                }
                else if (sMemBuf[i].Y == u16MinG)
                {
                    u16NumMinG++;
                }

                if (sMemBuf[i].Cb > u16MaxB)
                {
                    u16MaxB = sMemBuf[i].Cb;
                    u16NumMaxB = 1;
                }
                else if (sMemBuf[i].Cb == u16MaxB)
                {
                    u16NumMaxB++;
                }

                if (sMemBuf[i].Cb < u16MinB)
                {
                    u16MinB = sMemBuf[i].Cb;
                    u16NumMinB = 1;
                }
                else if (sMemBuf[i].Cb == u16MinB)
                {
                    u16NumMinB++;
                }
            }
#endif
            u32Rtt += sMemBuf[i].R_Cr;
            u32Gtt += sMemBuf[i].G_Y;
            u32Btt += sMemBuf[i].B_Cb;
            i++;
        }
    }

#if 0 // daniel:for debug only
    {
        U16 u16Y, u16Cb, u16Cr;
        SC_BK_SWICH(REG_SC_BK_DLC);
        REG_WI(REG_SC_DLC(0x68), ENABLE, BIT0);

        SC_BK_SWICH(REG_SC_BK_DLC);
        REG_WR(REG_SC_DLC(0x69), pCalWin->x);
        REG_WR(REG_SC_DLC(0x6A), pCalWin->y);

        msleep(10);

        u16Y = (REG_RR(REG_SC_DLC(0x6B)) & 0x03FF);
        u16Cb = (REG_RR(REG_SC_DLC(0x6C)) & 0x03FF);
        u16Cr = (REG_RR(REG_SC_DLC(0x6D)) & 0x03FF);
        printk("cross:%x,%x,%x\n", u16Y, u16Cb, u16Cr);

        SC_BK_SWICH(REG_SC_BK_DLC);
        REG_WI(REG_SC_DLC(0x68), ENABLE, BIT0);

        printk("mem  :%x,%x,%x\n", sMemBuf[0].G_Y, sMemBuf[0].B_Cb, sMemBuf[0].R_Cr);
    }
    while(1);
#endif


#if 0
    printk("\n");

    printk("CrBlock %dx%d", pCalWin->width, pCalWin->height);
    for (i = 0; i < (pCalWin->height * pCalWin->width); i++)
    {
        if ((i % pCalWin->width) == 0)
        {
            printk("\n");
        }
        printk("%04d ", sMemBuf[i].R_Cr);
    }
    printk("\n");

    printk("YBlock %dx%d", pCalWin->width, pCalWin->height);
    for (i = 0; i < (pCalWin->height * pCalWin->width); i++)
    {
        if ((i % pCalWin->width) == 0)
        {
            printk("\n");
        }
        printk("%04d ", sMemBuf[i].G_Y);
    }
    printk("\n");

    printk("CbBlock %dx%d", pCalWin->width, pCalWin->height);
    for (i = 0; i < (pCalWin->height * pCalWin->width); i++)
    {
        if ((i % pCalWin->width) == 0)
        {
            printk("\n");
        }
        printk("%04d ", sMemBuf[i].B_Cb);
    }
    printk("\n");
#endif

#if ADC_MEMAVG_WITHOUT_MAXMIN
    tmp = u32Rtt - ((U32)u16MaxR * (U32)u16NumMaxR) - ((U32)u16MinR * (U32)u16NumMinR);
    if ( tmp != 0 )
    {
        u16NoMinMaxAvgCr = tmp / ((pCalWin->height * pCalWin->width) - (u16NumMaxR + u16NumMinR));
    }

    tmp = u32Gtt - ((U32)u16MaxG * (U32)u16NumMaxG) - ((U32)u16MinG * (U32)u16NumMinG);
    if ( tmp != 0 )
    {
        u16NoMinMaxAvgY = tmp / ((pCalWin->height * pCalWin->width) - (u16NumMaxG + u16NumMinG));
    }

    tmp = u32Btt - ((U32)u16MaxB * (U32)u16NumMaxB) - ((U32)u16MinB * (U32)u16NumMinB);
    if ( tmp != 0 )
    {
        u16NoMinMaxAvgCb = tmp / ((pCalWin->height * pCalWin->width) - (u16NumMaxB + u16NumMinB));
    }
#endif

    ptAdcData.CH_AVG[0] = u32Rtt / (pCalWin->height * pCalWin->width);
    ptAdcData.CH_AVG[1] = u32Gtt / (pCalWin->height * pCalWin->width);
    ptAdcData.CH_AVG[2] = u32Btt / (pCalWin->height * pCalWin->width);

    AUTOMSG( printk("ptAdcData[RGB][%04d,%04d,%04d]\n",ptAdcData.CH_AVG[0],ptAdcData.CH_AVG[1],ptAdcData.CH_AVG[2]) );

#if ADC_MEMAVG_WITHOUT_MAXMIN
    printk("MaxCr    MinCr    MaxY    MinY    MaxCb    MinCb    AvgCr AvgY AvgCb\n");
    printk("%04d     %04d     %04d    %04d    %04d     %04d     %04d  %04d %04d\n",
        u16MaxR, u16MinR, u16MaxG, u16MinG, u16MaxB, u16MinB, ptAdcData.CH_AVG[0], ptAdcData.CH_AVG[1], ptAdcData.CH_AVG[2]);

    printk("NumMaxCr NumMinCr NumMaxY NumMinY NumMaxCb NumMinCb AvgCr AvgY AvgCr\n");
    printk("%04d     %04d     %04d    %04d    %04d     %04d     %04d  %04d %04d\n\n",
        u16NumMaxR, u16NumMinR, u16NumMaxG, u16NumMinG, u16NumMaxB, u16NumMinB, u16NoMinMaxAvgCr, u16NoMinMaxAvgY, u16NoMinMaxAvgCb);
#endif

#endif

    return ptAdcData;
}


// 20090928 daniel.huang: refine ADC calibration and update calibration algorithm
static void _GetGainRange(SC_AUTO_TUNE_TYPE_e enAutoTuneType,
                        AUTO_CAL_WINDOW_t *pCalWinG_Y,
                        AUTO_CAL_WINDOW_t *pCalWinB_Cb,
                        AUTO_CAL_WINDOW_t *pCalWinR_Cr,
                        U16 u16VBlockSize,
                        U16 u16HBlockSize,
                        MS_AUTOADC_TYPE *pDataRange)
{
    MS_AUTOADC_TYPE AutoAdc_Max, AutoAdc_Min, AutoAdcTemp;

    PIXEL_32BIT *sMemBuf = kmalloc(u16VBlockSize * u16HBlockSize * sizeof(PIXEL_32BIT), GFP_KERNEL);
    if (!sMemBuf)
    {
        printk("calibration buffer insufficient\n");
        return;
    }

    if(enAutoTuneType & AUTO_TUNE_INT_ADC)
    {
        MHal_ADC_InternalDc(INTERNAL_1_05V);
        mdelay(200);    //20090829 daniel.huang: change delay to fix internal calibration read value incorrect
        // freeze memory
        MHal_SC_IPM_SetFreezeImg(TRUE);
        MDrv_SC_IP1_WaitInputVSync(1, 30);
        AUTOMSG(printk("internal 1.05V "));
        AutoAdc_Max = _MDrv_SC_GetAverageData(sMemBuf, pCalWinG_Y);
        // un-freeze memory
        MHal_SC_IPM_SetFreezeImg(FALSE);



        MHal_ADC_InternalDc(INTERNAL_0_55V);
        mdelay(200);    //20090829 daniel.huang: change delay to fix internal calibration read value incorrect
        // freeze memory
        MHal_SC_IPM_SetFreezeImg(TRUE);
        MDrv_SC_IP1_WaitInputVSync(1, 30);
        AUTOMSG(printk("internal 0.55V "));
        AutoAdc_Min = _MDrv_SC_GetAverageData(sMemBuf, pCalWinG_Y);
        // un-freeze memory
        MHal_SC_IPM_SetFreezeImg(FALSE);

        pDataRange->CH_AVG[1] = AutoAdc_Max.CH_AVG[1] - AutoAdc_Min.CH_AVG[1];
        pDataRange->CH_AVG[2] = AutoAdc_Max.CH_AVG[2] - AutoAdc_Min.CH_AVG[2];
        pDataRange->CH_AVG[0] = AutoAdc_Max.CH_AVG[0] - AutoAdc_Min.CH_AVG[0];
    }
    else
    {
        // wait input 2 frames
        MDrv_SC_IP1_WaitInputVSync(3, 100);
        //AUTOMSG(printk("\n\n\nVSync Time %ld ms\n", jiffies - u32VsyncTime));
        // freeze memory
        MHal_SC_IPM_SetFreezeImg(TRUE);
        MDrv_SC_IP1_WaitInputVSync(1, 30);

        AutoAdcTemp = _MDrv_SC_GetAverageData(sMemBuf, pCalWinG_Y);
        pDataRange->CH_AVG[1] = AutoAdcTemp.CH_AVG[1]; //G_Y

        AutoAdcTemp = _MDrv_SC_GetAverageData(sMemBuf, pCalWinB_Cb);
        pDataRange->CH_AVG[2] = AutoAdcTemp.CH_AVG[2]; //B_Cb

        AutoAdcTemp = _MDrv_SC_GetAverageData(sMemBuf, pCalWinR_Cr);
        pDataRange->CH_AVG[0] = AutoAdcTemp.CH_AVG[0]; //R_Cr

        // un-freeze memory
        MHal_SC_IPM_SetFreezeImg(FALSE);
    }

    AUTOMSG(printk("G_Y=%04d, B_Cb=%04d, R_Cr=%04d \n", pDataRange->CH_AVG[1], pDataRange->CH_AVG[2], pDataRange->CH_AVG[0]));

    kfree(sMemBuf);
}


#define AUTO_GAIN_BONDARY               1   // 20091012 daniel.huang: for finetune internal calibration
#define MAX_CALIBRATION_ITERATION       12
#define EXT_RGB_BINARY_SEARCH_TARGET    1018// 20091012 daniel.huang: for finetune internal calibration
BOOL  MDrv_ADC_CalGain( SC_AUTO_TUNE_TYPE_e enAutoTuneType, PSC_SOURCE_INFO_t psrc,
                        U16 GainTarget[3],
                        AUTO_CAL_WINDOW_t *pCalWinG_Y,
                        AUTO_CAL_WINDOW_t *pCalWinB_Cb,
                        AUTO_CAL_WINDOW_t *pCalWinR_Cr)
{
    MS_AUTOADC_TYPE AutoAdc;
    BOOL bDone[3] = {FALSE, FALSE, FALSE};
    U16 GainTargetLBound[3];
    U16 GainTargetUBound[3];
    U16 Gain[3]      ={0x1000, 0x1000, 0x1000};
    U16 TempGain[3]  ={0x1000, 0x1000, 0x1000};
    U16 delta[3]={0x800, 0x800, 0x800};
    U16 u16UpperDiff[3] = {0xFFFF, 0xFFFF, 0xFFFF};
    U16 u16LowerDiff[3] = {0xFFFF, 0xFFFF, 0xFFFF};
    U16 u16UpperGain[3] = {0, 0, 0};
    U16 u16LowerGain[3] = {0, 0, 0};
    U8 u8Direction[3] = {0, 0, 0};
    U16 ch;
    U16 y = 0;
    U16 u16Diff;
    U16 u16HBlockSize = pCalWinG_Y->width;
    U16 u16VBlockSize = pCalWinG_Y->height;

    for (ch=0; ch<3; ch++)
    {
        // 20091012 daniel.huang: for finetune internal calibration
        if(!(enAutoTuneType & AUTO_TUNE_INT_ADC) && !(enAutoTuneType & AUTO_TUNE_YUV) 
        && (GainTarget[ch] > EXT_RGB_BINARY_SEARCH_TARGET))  // external RGB only
        {
            GainTargetUBound[ch] = EXT_RGB_BINARY_SEARCH_TARGET + AUTO_GAIN_BONDARY;    //Find gain setting from 1018. Than fine tune to 1023
            GainTargetLBound[ch] = EXT_RGB_BINARY_SEARCH_TARGET - AUTO_GAIN_BONDARY;    //Find gain setting from 1018. Than fine tune to 1023
        }
        else
        {
            GainTargetUBound[ch] = GainTarget[ch] + AUTO_GAIN_BONDARY;
            GainTargetLBound[ch] = GainTarget[ch] - AUTO_GAIN_BONDARY;
        }

        AUTOMSG(printk("ch=%u, TargetRangeMax=%u, TargetRangeMin=%u\n",
            ch, GainTargetUBound[ch], GainTargetLBound[ch]));
    }

    for (ch=0; ch<3; ch++)
    {
        MHal_ADC_SetGain(ch, Gain[ch]);
    }

    for ( y = 0; y < MAX_CALIBRATION_ITERATION; y++)
    {
        // 20090928 daniel.huang: refine ADC calibration and update calibration algorithm
        _GetGainRange(enAutoTuneType,
                      pCalWinG_Y, pCalWinB_Cb, pCalWinR_Cr,
                      u16VBlockSize, u16HBlockSize, &AutoAdc);

        //ch = 1; // for debug Y/G channnel only
        for (ch=0; ch<3; ch++)
        {
            if (ch==0) {
                AUTOMSG(printk("============No. %02d=============>\n", (y+1)));
            }
            if (!bDone[ch])
            {
                u16Diff = AutoAdc.CH_AVG[ch];   // 20090928 daniel.huang: refine ADC calibration and update calibration algorithm

                if ( y == (MAX_CALIBRATION_ITERATION-1) )
                {
                    AUTOMSG(printk("Gain[%u]=0x%x, diff=%d\n", ch, TempGain[ch], u16Diff));

                    if (u16Diff >= GainTarget[ch])
                    {
                        if (u16UpperDiff[ch] > (u16Diff - GainTarget[ch]))
                        {
                            u16UpperDiff[ch] = u16Diff - GainTarget[ch];
                            u16UpperGain[ch] = TempGain[ch];
                            //AUTOMSG(printk("Cr Upper Diff %04d, Gain %04d\n", u16UpperDiff_R, u16UpperGain_R));
                        }
                    }

                    if (u16Diff <= GainTarget[ch])
                    {
                        if (u16LowerDiff[ch] > (GainTarget[ch] - u16Diff))
                        {
                            u16LowerDiff[ch] = GainTarget[ch] - u16Diff;
                            u16LowerGain[ch] = TempGain[ch];
                            //AUTOMSG(printk("Cr Lower Diff %04d, Gain %04d\n", u16LowerDiff_R, u16LowerGain_R));
                        }
                    }
                }
                else
                {
                    AUTOMSG(printk("Gain[%u]=0x%x, diff=%d\n", ch, Gain[ch], u16Diff));

                    if ( (u16Diff>=GainTargetLBound[ch] ) && (u16Diff<=GainTargetUBound[ch] ) )
                    {
#if 0                   // 20090928 daniel.huang: refine ADC calibration and update calibration algorithm
                        U16 target,u16Result[5],Step;
                        MS_AUTOADC_TYPE modifiy_diff;
                        U8 index, bRetry = TRUE;
                        U8 Index_Start = 0 ,Index_End = 4;
                        BOOL bFound1 = FALSE, bFound2 = FALSE;

                        bDone[ch] = TRUE;
                        u16UpperDiff[ch] = u16LowerDiff[ch] = 0;

                        u8Direction[ch] = 0;

                        ////////////Find a best solution////////////////////////
                        target = ( GainTargetLBound[ch] + GainTargetUBound[ch] )  / 2 ;
                        AUTOMSG(printk(" target : %d  current : %d\n",target,u16Diff));

                        // Retrieve result array, and make sure it is linear.
                        while (bRetry)
                        {
                            for (index = 0, Step = Gain[ch] - 0x08; Step <= Gain[ch] + 0x08 ; Step+= 0x04,index++)
                            {
                                MHal_ADC_SetGain(ch, Step);
                                _GetGainRange(enAutoTuneType,
                                              pCalWinG_Y, pCalWinB_Cb, pCalWinR_Cr,
                                              u16VBlockSize, u16HBlockSize, &modifiy_diff);

                                u16Result[index] = modifiy_diff.CH_AVG[ch];
                                AUTOMSG(printk("Result index: %d Gain: 0x%4x   %d \n",index,Step,u16Result[index] ));
                            }

                            // check result is linear
                            for ( index = 0 ; index < 5 ; index ++)
                            {
                                if ( index >= 4 )
                                {
                                    bRetry = FALSE;
                                    break;
                                }

                                if ( u16Result[index] > u16Result[index+1] )
                                {
                                    printk(" fail \n");
                                    break;
                                }
                            }
                            if ( bRetry)
                                continue;

                            // Find Best solution
                            while ( Index_End > Index_Start && ( !bFound1 || !bFound2) )
                            {
                                if ( u16Result[Index_Start] != target )
                                    Index_Start++;
                                else
                                    bFound1 = TRUE;

                                if ( u16Result[Index_End] != target)
                                    Index_End--;
                                else
                                    bFound2 = TRUE;
                            }

                            // Founded
                            if ( Index_End >= Index_Start)
                            {
                                Step = Gain[ch] - 0x08;
                                Gain[ch] = Step + ( ( (Index_Start + Index_End ) * 10 ) / 2 ) * 0x04 / 10;
                            }
                            else
                            {
                                printk(" not found \n");
                            }

                        }

                        u16UpperGain[ch] = u16LowerGain[ch] = Gain[ch];

                        AUTOMSG(printk("=====ch:%u--OK--0x%x=======\n", ch, Gain[ch]));

#else
                        bDone[ch] = TRUE;
                        u16UpperDiff[ch] = u16LowerDiff[ch] = 0;
                        u16UpperGain[ch] = u16LowerGain[ch] = Gain[ch];
                        u8Direction[ch] = 0;
                        AUTOMSG( printk("ch:%u--OK--0x%x\n", ch, Gain[ch]));
#endif
                    }
                    else
                    {
                        if (u16Diff >= GainTarget[ch])
                        {
                            if (u16UpperDiff[ch] > (u16Diff - GainTarget[ch]))
                            {
                                u16UpperDiff[ch] = u16Diff - GainTarget[ch];
                                u16UpperGain[ch] = Gain[ch];
                                //AUTOMSG(printk("Upper Diff %04d, Gain %04d\n", u16UpperDiff[ch], u16UpperGain[ch]));
                            }
                        }

                        if (u16Diff <= GainTarget[ch])
                        {
                            if (u16LowerDiff[ch] > (GainTarget[ch] - u16Diff))
                            {
                                u16LowerDiff[ch] = GainTarget[ch] - u16Diff;
                                u16LowerGain[ch] = Gain[ch];
                                //AUTOMSG(printk("Lower Diff %04d, Gain %04d\n", u16LowerDiff[ch], u16LowerGain[ch]));
                            }
                        }

                        if (u16Diff < GainTargetLBound[ch] )
                        {
                            Gain[ch]+=delta[ch];
                            u8Direction[ch] = DOWN_TOP; //DownToUp
                        }
                        else   //(u16Diff > GainTargetUBound[ch] )
                        {
                            Gain[ch]-=delta[ch];
                            u8Direction[ch] = TOP_DOWN; //UpToDown
                        }
                        delta[ch]=delta[ch]>>1;
                    }

                    if ( y == (MAX_CALIBRATION_ITERATION-2) )
                    {
                        if (u8Direction[ch] == DOWN_TOP)
                        {
                            TempGain[ch] = Gain[ch]-0x2;

                        }
                        else if (u8Direction[ch] == TOP_DOWN)
                        {
                            TempGain[ch] = Gain[ch]+0x2;
                        }
                        MHal_ADC_SetGain(ch, TempGain[ch]);
                    }
                    else
                    {
                        //AUTOMSG( printk("Gain[%u] 0x%x\n", ch, Gain[ch]));
                        //AUTOMSG( printk("delta[%u] 0x%x\n", ch, delta[ch]));
                        MHal_ADC_SetGain(ch, Gain[ch]);
                    }
                }
            }
        }

        if ( bDone[0] && bDone[1] && bDone[2] )
        {
            // 20090928 daniel.huang: refine ADC calibration and update calibration algorithm
            if(!(enAutoTuneType & AUTO_TUNE_INT_ADC) && !(enAutoTuneType & AUTO_TUNE_YUV))  // external RGB only
            {
                AUTOMSG(printk(" fine tune gain\n"));
                // 20091012 daniel.huang: for finetune internal calibration
                for (ch=0; ch<3; ch++)
                {
                    if (GainTarget[ch] > EXT_RGB_BINARY_SEARCH_TARGET)
                        bDone[ch] = FALSE;
                    else
                        bDone[ch] = TRUE;
                }
                //bDone[0] = bDone[1] = bDone[2] = FALSE;
                
                while ( !bDone[0] || !bDone[1] || !bDone[2])
                {
                    for (ch=0; ch<3; ch++)
                    {
                        if ( !bDone[ch] )
                        {
                            Gain[ch] += 2 ;
                            MHal_ADC_SetGain(ch, Gain[ch]);
                        }
                    }
                    // 20090928 daniel.huang: refine ADC calibration and update calibration algorithm
                    _GetGainRange(enAutoTuneType,
                                  pCalWinG_Y, pCalWinB_Cb, pCalWinR_Cr,
                                  u16VBlockSize, u16HBlockSize, &AutoAdc);

                    for (ch=0; ch<3; ch++)
                    {
                        // Fine tune gain until
                        if ( bDone[ch] == FALSE && AutoAdc.CH_AVG[ch] == GainTarget[ch] )   // 20091012 daniel.huang: for finetune internal calibration
                        {
                            bDone[ch] = TRUE;
                        }
                    }
                }
            }
            AUTOMSG(printk("done - "));
            break;
        }
    }

    if ( !(bDone[0] && bDone[1] && bDone[2]) )
    {
        for (ch=0; ch<3; ch++)
        {
            if (u16UpperDiff[ch] <= u16LowerDiff[ch])
            {
                Gain[ch] = u16UpperGain[ch];
            }
            else
            {
                Gain[ch] = u16LowerGain[ch];
            }
        }
        AUTOMSG(printk("not done - "));
    }

    for (ch=0; ch<3; ch++)
    {
        MHal_ADC_SetGain(ch, Gain[ch]);
    }

    AUTOMSG(printk("gain check ok  0x%x 0x%x 0x%x\n",Gain[0],Gain[1],Gain[2]));
    return TRUE;

}

