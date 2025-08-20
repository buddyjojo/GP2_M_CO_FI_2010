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
/// file    mdrv_scaler.c
/// @brief  Scaler Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/errno.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h> //thchen 20080820
#include <asm/div64.h> // LGE drmyung 081113
#include <linux/timer.h>
#include <linux/jiffies.h>
#include "Board.h"
#include "chip_int.h"		// LGE drmyung 081013
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mst_utility.h"
#include "mdrv_scaler_io.h"
#include "mdrv_scaler_st.h"
#include "mdrv_scaler.h"
#include "mdrv_adc.h"
#include "mdrv_hdmi.h"
#include "mdrv_mace.h"
#include "mdrv_scaler_pcmode.h"
#include "mdrv_system.h"
#include "mdrv_tcon.h"
#include "mhal_scaler_reg.h"
#include "mhal_scaler.h"
#include "mhal_lpll.h"
#include "mhal_mod.h"
#include "mhal_hdmi.h"
#include "mhal_adc.h"
#include "mhal_tcon.h"
#include "mhal_od.h"    // 20091012 Michu: add OD
#include "mhal_utility.h"
#include "QualityMode.h"
#include "mdrv_qmap.h"
#include "mhal_qmap.h"

#define ENABLE_OD       0    // 20091012 Michu: add OD

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures
//-------------------------------------------------------------------------------------------------
spinlock_t menuload_lock    = SPIN_LOCK_UNLOCKED;
unsigned long irq_flags_menuload;
spinlock_t switch_bk_lock   = SPIN_LOCK_UNLOCKED;
unsigned long irq_flags_swbk;
U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------
#define FREERUN_VFREQ   600//lachesis_100105 ursa 초기값 60Hz
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define ISR_INPUT_VSYNC_CNT		5//7 lachesis_091006 for speed (test)
#define OUTPUT_VSYNC_TIMEOUT    250 //LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue
#define SC_MAX_LINE_BUF			1920
//#define IRQ_DISP        10 // Kernel 2.6.10 thchen 20080820 please refer to chip_int.h  "E_IRQ_DISP = E_IRQL_START+10,"
//#define E_IRQ_DEB       18 // Kernel 2.6.26 LGE drmyung 081013

//#define FL_DEBUG_PRINT // LGE drmyung 080922
//#define SC_DEBUG_PRINT // LGE drmyung 080922
//#define SC_PQ_DEBUG_PRINT // LGE drmyung 080922
#define SC_FPLL_PRINT

//#define MEASURE_TIME
#ifdef MEASURE_TIME
////////////////////////////////////////////
// 8bit 0x00302x PIU timer0
// h00  [0] reg_timer_en
//      [1] enable from 0 to max
// h0003:h0002 reg_timer_max
// h0005:h0004 reg_timer_cap (unit: 1/12MHz)
#define REG_PIU_TIMER0(_x_)              (0x003020 | (_x_ << 1))
#define REG_ADDR(addr)              (*((volatile U16*)(0xBF000000 + ((addr) << 1))))
#define REG_WR(_reg_, _val_)        do{ REG_ADDR(_reg_) = (_val_);  }while(0)
#define REG_RR(_reg_)               ({ REG_ADDR(_reg_);})
#endif

#define GAMMA_LOAD_SIZE 192
//--------------------------------------------------------------------------------------------------
//  Macro definition
//--------------------------------------------------------------------------------------------------
#ifdef FL_DEBUG_PRINT
#define FL_DBG(fmt, args...)    printk("\033[47;31m" fmt "\033[0m", ## args)
#else
#define FL_DBG(fmt, args...)
#endif
#ifdef SC_DEBUG_PRINT
#define SC_DBG(fmt, args...)    printk("\033[47;31m" fmt "\033[0m", ## args)
#else
#define SC_DBG(fmt, args...)
#endif
#ifdef SC_PQ_DEBUG_PRINT
#define SC_PQ_DBG(fmt, args...)    printk("\033[47;31m" fmt "\033[0m", ## args)
#else
#define SC_PQ_DBG(fmt, args...)
#endif
#ifdef SC_FPLL_PRINT
#define SC_FPLL_DBG(fmt, args...)    printk("\033[47;31m" fmt "\033[0m", ## args)
#else
#define SC_FPLL_DBG(fmt, args...)
#endif

#define SC_SPEED_DBG(fmt, args...)    printk("\033[47;31m" fmt "\033[0m", ## args)
#ifdef SC_MIRROR_DEBUG_PRINT// Michu 20090903
#define SC_MIRROR_DBG(fmt, args...)    printk("\033[47;31m" fmt "\033[0m", ## args)
#else
#define SC_MIRROR_DBG(fmt, args...)
#endif

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                        }\
                    } while (0)

#define SC_MMIO_DBG             0
#if (SC_MMIO_DBG)
#define SC_MMIO_PRINT(fmt, args...)      printk("[Mdrv_SC][%05d] " fmt, __LINE__, ## args)
#else
#define SC_MMIO_PRINT(fmt, args...)
#endif

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef unsigned long long U64; // LGE drmyung 081113
typedef enum
    {
        E_SC_MMIO_OK = 1,
        E_SC_MMIO_WRONG_END_CHAR = 2,
        E_SC_MMIO_WRONG_MMAPADDR = 3,

    }E_SC_MMIO;
//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
U16 defaultColorMatrix [9] =
{
    0x0660, 0x04A6, 0x0000, 0x133F, 0x04A6, 0x118F, 0x0000, 0x04A6, 0x0810
};

#ifdef	SC_USE_ONLY_ONE_THREAD	// (dreamer@lge.com)
long scaler_pid = -1;
#endif
#ifdef	SC_USE_SCART_THREAD   	// (dreamer@lge.com)
static SC_SCART_AR_e _gScartARs[2] = {SCART_AR_INVALID, SCART_AR_INVALID};
#endif

//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------

SC_INPUT_SOURCE_e g_SrcType; // for SC ISR handle only

MST_PANEL_INFO_t  g_PanelInfo;
SC_DRIVER_CONTEXT_t g_DrvCtx;
SC_HISTOGRAM_INFO_t g_HistogramData; //thchen 20080820

SC_PCADC_MODETABLE_TYPE astStandardModeTable[MD_STD_MODE_MAX_INDEX];
static BOOL	bIsForceFreeRun = FALSE; // LGE drmyung 081106
static U8 gControlAutoNR = 0; //[090804_Leo]

static U32 isr_inputCnt = 0; //FitchHsu 20081125 JPEG issue for rotate
BOOL gIsVideoMute = FALSE; // LGE drmyung 081129 : for ATV Frame Lock
static U16 _u16PrevOutputVFreq = 0;		//LGE [vivakjh] 2008/11/12 	Add for DVB PDP Panel

static SC_DISPLAY_WIN_t _gDispwin = {0,};

static BOOL _gIsPdpPanel	= FALSE;	//LGE[vivakjh] 2008/12/07  pDrvCtx를 인자로 사용하지 않는 함수에서 PDP/LCD를 구분하기 위한 static 변수. mdrv_scaler.c 내에서만 사용함.
// LGE [vivakjh]  2008/12/11 	PDP 경우 Fast Frame Lock을 못하므로 Frame Lock시점까지 Film Mode를 Off 했다가 Frame Lock 되면 On해줌.
static U8 	_gFilmMode = 0;		// _gFilmMode  => 0 : OFF,  1 : 50Hz, 2 : 60Hz, 3 : Both
// LGE [vivakjh]  2008/12/11	PDP 만  Freeze 기능 대응됨.  Film On 시에 Freeze 시 화면 떨림 수정함.
static U8 	_gFreezeFilmMode = 0;		// _gFreezeFilmMode  => 0 : OFF,  1 : 50Hz, 2 : 60Hz, 3 : Both
static BOOL	_bIsFilmInFreeze = FALSE;
static BOOL _gFrameTo48Hz = FALSE;
static BOOL _gbIsPDPMarginOver = FALSE;	// LGE [vivakjh] 2009/01/21	Modified the PDP module flicker problem after playing some perticular movie files that are over the PDP module margin.
BOOL  bIsphase_df_ok   = TRUE;
BOOL  bIslock_time     = FALSE;
BOOL  bIs_locked       = FALSE;
static struct timer_list vcnt_freeze_rst_timer;

//--------------------------------------------------------------------------------------------------
//  Exernal
//--------------------------------------------------------------------------------------------------
extern void MDrv_GOP_VSyncHandle(void);

//--------------------------------------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------------------------------------

//FitchHsu 20081113 EMP when PAUSE, little shaking
void MDrv_SC_PQ_FastPlayback(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_PQ_FASTPLAYBACK_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_PQ_FASTPLAYBACK_t)))
    {
        return;
    }
    MHal_SC_PQ_FastPlayback(param.bEnable);
}

//FitchHsu 20081119 EMP preview setting for 24P and 30P
// FitchHsu20090119 EMP preview setting error
void MDrv_SC_EMP_Preview(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_PQ_EMPPREVIEW_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_PQ_EMPPREVIEW_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
	pSrcInfo->bEmppreview = param.bEnable;
}

//victor 20090108, add emp video input source
void MDrv_SC_EMP_SetPlayingEMPVideo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_PQ_EMPPlayingVideo_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_PQ_EMPPlayingVideo_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    pSrcInfo->bEmpPlayingVideo = param.bEnable;
}

void MDrv_SC_EMP_SetEMPJPEG(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_PQ_EMPJPEG_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_PQ_EMPPlayingVideo_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    pSrcInfo->bMediaPhoto = param.bEnable;
}

void MDrv_SC_InitDrvCtx(PSC_DRIVER_CONTEXT_t pDrvCtx)
{
    pDrvCtx->pPanelInfo = (PMST_PANEL_INFO_t)MDrv_SYS_GetPanelInfo();

    pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u32SrcState = 0;
    pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u32ActiveFlag = 0;
    pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u32SigInfo  = 0;

    pDrvCtx->SrcInfo[SC_MAIN_WINDOW].bCropWin_Enable = FALSE;
    pDrvCtx->SrcInfo[SC_MAIN_WINDOW].bEmpPlayingVideo = FALSE;//victor 20090108, add emp video input source

	#ifdef FAST_THREAD_CONTROL
    init_waitqueue_head(&pDrvCtx->SrcInfo[SC_MAIN_WINDOW].ThreadData.thread_wq);
	#else
    init_waitqueue_head(&pDrvCtx->SrcInfo[SC_MAIN_WINDOW].ThreadData.wait_event_wq);
	#endif

    atomic_set(&pDrvCtx->SrcInfo[SC_MAIN_WINDOW].ThreadData.wait_event_sts, SC_EVENT_WAITING);

    pDrvCtx->ModeTablePtr = NULL;
}


void MDrv_SC_PQDumpRegTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    EN_IP_Info_AP param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(EN_IP_Info_AP)))
    {
        return;
    }
    switch(param.u8TabType)
    {
    case PQ_TABTYPE_SCALER:
        MDrv_SCMAP_DumpScalerRegTable_AP(&param);
        break;
    case PQ_TABTYPE_COMB:
        MDrv_SCMAP_DumpCombRegTable_AP(&param);
        break;
    default:
        SC_DBG("[PQ]DumpTable:Table is not scaler or COMB\n");
        break;
    }
}


void MDrv_SC_Cleanup(PSC_DRIVER_CONTEXT_t pDrvCtx)
{
    PSC_SOURCE_INFO_t pSrcInfo;

    MHal_SC_SetInterruptMask(SC_INT_VSINT, FALSE);
    MHal_SC_SetInterruptMask(SC_INT_F2_IPVS_SB, FALSE);

    pSrcInfo = &pDrvCtx->SrcInfo[SC_MAIN_WINDOW];

    if (SC_GET_SRC_STATE(pSrcInfo) & SC_SRC_STATE_PCMODE)
    {
        MDrv_SC_WakeupEvent(pSrcInfo, SC_EVENT_STOP);   // prevent infinite waiting IOCTL_SC_WAIT_EVENT in ap
        MDrv_SC_PCMode_Stop(pSrcInfo);
        SC_CLR_SRC_STATE(pSrcInfo, SC_SRC_STATE_PCMODE);
    }
    else if (SC_GET_SRC_STATE(pSrcInfo) & SC_SRC_STATE_HDMI)
    {
        MDrv_SC_WakeupEvent(pSrcInfo, SC_EVENT_STOP);   // prevent infinite waiting IOCTL_SC_WAIT_EVENT in ap
        MDrv_HDMI_Stop(pSrcInfo);
        SC_CLR_SRC_STATE(pSrcInfo, SC_SRC_STATE_HDMI);
    }
#if 0 //080912 LGE drmyung
    else if (SC_GET_SRC_STATE(pSrcInfo) & SC_SRC_STATE_LPLL) //thchen 20080904
    {
        MDrv_SC_WakeupEvent(pSrcInfo, SC_EVENT_STOP);   // prevent infinite waiting IOCTL_SC_WAIT_EVENT in ap
        MDrv_SC_LPLL_Stop(pSrcInfo);
        SC_CLR_SRC_STATE(pSrcInfo, SC_SRC_STATE_LPLL);
    }
#endif
    if (pDrvCtx->ModeTablePtr != NULL)
    {
        kfree(pDrvCtx->ModeTablePtr);
        pDrvCtx->ModeTablePtr = NULL;
    }
}

extern BOOL bTCONpwsq;

// cc.chen - SC ISR //thchen 20080820 LGE drmyung
static irqreturn_t MDrv_SC_ISR(int irq, void *dev_id )	// changed to remove warning(dreamer@lge.com)
{
    U32 u32IntSts;
 	//U32 u32MotionValue;	//victor 20080922
    //U32 timer1, timer2;

    u32IntSts = MHal_SC_GetInterruptSts();
    if (u32IntSts)
    {
        if(u32IntSts & (1 << (SC_INT_VSINT)))
        {
#if (USE_MENULOAD_SCALER || USE_MENULOAD_PQ)
            // menuload is trigger at vsync falling edge and must update register as soon as possible
            if (MHal_SC_ML_IsMenuloadDone())
            {
                MHal_SC_ML_UpdateReg();
            }
#endif

#if 1 //Moved here. Histogram data should be got as early as possible after vsync int. [090915_Leo]
            g_HistogramData.bDataReady = MHal_SC_MACE_WaitHistogramDataReady();
            if (g_HistogramData.bDataReady == TRUE)
            {
                MHal_SC_MACE_GetHistogram32(&g_HistogramData.u16Histogram32[0]);
                g_HistogramData.u8MaxPixelValue = MHal_SC_MACE_GetMaxPixelValue();
                g_HistogramData.u8MinPixelValue = MHal_SC_MACE_GetMinPixelValue();
                g_HistogramData.u8AvgPixelValue = MHal_SC_MACE_GetAvgPixelValue();
                g_HistogramData.u16TotalColorCount = MHal_SC_MACE_GetTotalColorCount();
            }
            MHal_SC_MACE_RequestHistogramData();
#endif
            if(bTCONpwsq)
            {
                MDrv_TCONMAP_DumpPowerOnSequenceReg();
            }
            if(gControlAutoNR) //enable temporarily. may need finetune flow later. [090804_Leo]
            {
                MHal_SC_DynamicNRInATV();
            }

			if(g_SrcType == INPUT_SOURCE_DTV)
				MHal_Scaler_AdaptiveTuning(TRUE);	//[091021_Leo]
			else
				MHal_Scaler_AdaptiveTuning(FALSE);

#if 0 // mark for T3 bringup
            // FitchHsu 20090604 SpeedUp frame lock for TW model in YPbPr and HDMI
            if(bIsphase_df_ok == FALSE)
            {
                FL_DBG("Phase DF is not OK\n");
				if(MHal_LPLL_GetPhaseDiff() < 0x1000)
                {
                    FL_DBG("Phase DF is OK\n");
                    MHal_LPLL_DisableIGainForLockPhase(FALSE);
                    bIsphase_df_ok = TRUE;
                    bIs_locked = TRUE;
                }
            }

			if(isrCnt < 10)	isrCnt++; // LGE drmyung 081106 : VSync Wait Count

            //timer1=Get_Timer();
            // handler....

            //marked because it's not been verified and decided to use yet, [090915_Leo]
            MHal_Scaler_AdaptiveTuning();	// fitch 20081222

			#if 0	//KWON_1126_FILM_PATCH
			MHal_Scaler_FilmPatch(u32MotionValue);			//victor
			#endif
			MDrv_GOP_VSyncHandle(); // LGE drmyung 080902

			// LGE [vivakjh]  2008/12/11	PDP 경우 Fast Frame Lock을 못하므로 Frame Lock시점까지 Film Mode를 Off 했다가 Frame Lock 되면 On해줌.
			if ((_gFilmMode > 0) && MHal_LPLL_GetFrameLock_Status())
			{
				MHal_SC_FilmEnable(TRUE, _gFilmMode-1);
				_gFilmMode = 0;
			}

#endif // mark for T3 bringup
            MDrv_GOP_VSyncHandle(); // LGE drmyung 080902

            MHal_SC_EnableInterrupt(SC_INT_VSINT, TRUE);
            MHal_SC_EnableInterrupt(SC_INT_VSINT, FALSE);
            //timer2=Get_Timer();
        }
        //FitchHsu 20081125 JPEG issue for rotate
        if(u32IntSts & (1 << (SC_INT_F2_IPVS_SB)))
        {

#if 1 //lachesis_090831
            if(isr_inputCnt < ISR_INPUT_VSYNC_CNT)
			{
				//printk("isr_inputCnt = %d\n", isr_inputCnt);
				isr_inputCnt++; // Input VSync Wait Count
			}
#endif // mark for T3 bringup
            MHal_SC_EnableInterrupt(SC_INT_F2_IPVS_SB, TRUE);
            MHal_SC_EnableInterrupt(SC_INT_F2_IPVS_SB, FALSE);
        }
    }
    //printk("R=%d\n",g_HistogramData.bDataReady);
    //printk("t1=%d t2=%d diff=%d R=%d\n",timer1,timer2,((timer2-timer1)/12),g_HistogramData.bDataReady);
    //Restart_Timer();

   return IRQ_HANDLED;
}

//balup_1231
U32 MDrv_SetPanelData(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    HAL_LPLL_INIT_t lpllInit;
    U16				u16TempVBackPorch = 0;	//[vivakjh] 2008/12/23	Modified for adjusting the MRE in PDP S6
    U16				u16TempHTotal, u16TempVTotal;		// shjang_090904
    HAL_SC_OUTPUTSYNC_MODE_e eTempOutSyncMode = SC_OUTPUTSYNC_MODE_1;	//[vivakjh] 2009/01/29	Modified the 1 line garbage problem at PDP.

    // LPLL init
    lpllInit.u16InputDiv  = pDrvCtx->pPanelInfo->u16LPLL_InputDiv;
    lpllInit.u16LoopDiv   = pDrvCtx->pPanelInfo->u16LPLL_LoopDiv;
    lpllInit.u16OutputDiv = pDrvCtx->pPanelInfo->u16LPLL_OutputDiv;
    lpllInit.u8Type       = pDrvCtx->pPanelInfo->u8LPLL_Type;
    lpllInit.u8Mode       = pDrvCtx->pPanelInfo->u8LPLL_Mode;
	lpllInit.u32LimitD5d6d7= pDrvCtx->pPanelInfo->u32LimitD5d6d7;
	lpllInit.u16LimitOffset= pDrvCtx->pPanelInfo->u16LimitOffset;

	// shjang_090904
	u16TempVTotal = pDrvCtx->pPanelInfo->u16VTotal;
	u16TempHTotal = pDrvCtx->pPanelInfo->u16HTotal; // 20091029 Daniel.Huang: for fixing TCON panel dclk

	//LGE [vivakjh] 2008/11/12 	Add for PDP Panel
	if(pDrvCtx->pPanelInfo->u8LCDorPDP == 1)	// PDP Only
	{
		MHal_LPLL_Init4PDP(&lpllInit);
		_gIsPdpPanel = TRUE;	//LGE[vivakjh] 2008/12/07  pDrvCtx를 인자로 사용하지 않는 함수에서 PDP/LCD를 구분하기 위한 static 변수. mdrv_scaler.c 내에서만 사용함.
		u16TempVBackPorch = pDrvCtx->pPanelInfo->u16VBackPorch50Hz;	//[vivakjh] 2008/12/23	Modified for adjusting the MRE in PDP S6
		eTempOutSyncMode = SC_OUTPUTSYNC_MODE_3;	//[vivakjh] 2009/01/29	Modified the 1 line garbage problem at PDP.
	}
	else if(pDrvCtx->pPanelInfo->u8LCDorPDP == 2)
	{
		MHal_LPLL_Init4NonFRC(&lpllInit);
		_gIsPdpPanel = FALSE;
		u16TempVBackPorch = 8;
		eTempOutSyncMode = SC_OUTPUTSYNC_MODE_1;	//[vivakjh] 2009/01/29	Modified the 1 line garbage problem at PDP.
	}
    // shjang_090904
    else if(pDrvCtx->pPanelInfo->u8LCDorPDP == 3)
    {
        MHal_SC_VOP_SetAutoVSyncCtrl(FALSE);
        MHal_SC_VOP_OD_DataPath(pDrvCtx->pPanelInfo->bOD_DataPath);
        MHal_LPLL_Init4NonFRC(&lpllInit);
        _gIsPdpPanel = FALSE;
        u16TempVBackPorch = pDrvCtx->pPanelInfo->u16VBackPorch60Hz;
        eTempOutSyncMode = SC_OUTPUTSYNC_MODE_3;    //[vivakjh] 2009/01/29  Modified the 1 line garbage problem at PDP.
    }
	else
	{
    	MHal_LPLL_Init(&lpllInit);
		_gIsPdpPanel = FALSE;	//LGE[vivakjh] 2008/12/07  FALSE ==> LCD
		u16TempVBackPorch = 8;	//[vivakjh] 2008/12/23	Modified for adjusting the MRE in PDP S6
		eTempOutSyncMode = SC_OUTPUTSYNC_MODE_1;	//[vivakjh] 2009/01/29	Modified the 1 line garbage problem at PDP.
	}

    // output timing

	// shjang_090906
    MHal_SC_VOP_HVTotalSet(u16TempHTotal, u16TempVTotal);
    MHal_SC_VOP_HSyncWidthSet(pDrvCtx->pPanelInfo->u8HSyncWidth);

    // DE window
    if(pDrvCtx->pPanelInfo->u8LCDorPDP == 3)
    {
        MHal_SC_VOP_SetDEWin(
            pDrvCtx->pPanelInfo->u16HStart,
            pDrvCtx->pPanelInfo->u16HStart + pDrvCtx->pPanelInfo->u16Width - 1,
            pDrvCtx->pPanelInfo->u16DE_VStart -2,  // [daniel.huang] 2009/4/2, use application to set zero, not driver
            pDrvCtx->pPanelInfo->u16DE_VStart + pDrvCtx->pPanelInfo->u16Height);
    }
    else
    {
        MHal_SC_VOP_SetDEWin(
            pDrvCtx->pPanelInfo->u16HStart,
            pDrvCtx->pPanelInfo->u16HStart + pDrvCtx->pPanelInfo->u16Width - 1,
            pDrvCtx->pPanelInfo->u16DE_VStart,  // [daniel.huang] 2009/4/2, use application to set zero, not driver
            pDrvCtx->pPanelInfo->u16DE_VStart + pDrvCtx->pPanelInfo->u16Height - 1);
    }

    // display window
    MHal_SC_VOP_SetDispWin(
        pDrvCtx->pPanelInfo->u16HStart,
        pDrvCtx->pPanelInfo->u16HStart + pDrvCtx->pPanelInfo->u16Width - 1,
        pDrvCtx->pPanelInfo->u16DE_VStart,
        pDrvCtx->pPanelInfo->u16DE_VStart + pDrvCtx->pPanelInfo->u16Height - 1);

    // output control
    MHal_SC_VOP_OutputCtrl(
        pDrvCtx->pPanelInfo->u16OCTRL,
        pDrvCtx->pPanelInfo->u16OSTRL,
        pDrvCtx->pPanelInfo->u16ODRV,
        pDrvCtx->pPanelInfo->u16DITHCTRL);

	//[vivakjh] 2008/12/23	Modified for adjusting the MRE in PDP S6
	// lemonic LGE 081206 init시 1번 적용하는 것이 정상이나 081206현재 VOP 0x10이 0x00으로 setting되는 현상 있어 MDrv_SC_SetDisplayWin()부분에서 적용부 추가함.
	MHal_SC_VOP_SetOutputSycCtrl(eTempOutSyncMode,
	    pDrvCtx->pPanelInfo->u16VTotal - pDrvCtx->pPanelInfo->u16VStart,
	    pDrvCtx->pPanelInfo->u16VTotal - u16TempVBackPorch); // cc.chen - T.B.D. - panel dependent

        // shjang_090904
	if((pDrvCtx->pPanelInfo->u8LCDorPDP == 1) || (pDrvCtx->pPanelInfo->u8LCDorPDP == 2)  || (pDrvCtx->pPanelInfo->u8LCDorPDP == 3))
	{
		_u16PrevOutputVFreq =0;
	}

    // daniel.huang: refine frame lock
    SC_FPLL_DBG("##########MDrv_SC_SetPanelTiming( 1 )\n");

	//lachesis_100105 최초 power on 시 output dclk 설정 되지 않는 문제 개선.
    MDrv_SC_SetPanelTiming(pDrvCtx, TRUE, TRUE);

	MHal_MOD_Init(pDrvCtx->pPanelInfo);
	////MHal_SC_SetDithering(pDrvCtx->pPanelInfo->u8MOD_CTRLB);//[100118_Leo]
	if(pDrvCtx->pPanelInfo->u16Height < 770)
	{
	    MHal_SC_OSD_Reference(pDrvCtx->pPanelInfo->u16Height);
	}
    return 0;
}


static unsigned int u32DEBIntRegister;
//--------------------------------------------------------------------------------------------------
//  IOCTL Entry
//--------------------------------------------------------------------------------------------------
U32 MDrv_SC_Init(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    int             result;		// added to remove warning(dreamer@lge.com)
    U32 u32MenuloadMemAddr, u32MenuloadMemSize;
    U32 u32MIU1_MemAddr, u32MIU1_MemSize;
    U8  u8MIU_num = 0;
    BOOL bsplash;
#if ENABLE_OD// 20091012 Michu: add OD
    U32 u32ODmstMemAddr, u32ODmstMemSize;
    U32 u32ODlstMemAddr, u32ODlstMemSize;
    U32 OD_MSB_START_ADDR, OD_MSB_LIMIT_ADDR, OD_LSB_START_ADDR, OD_LSB_LIMIT_ADDR;
#endif

    bsplash = MHal_SC_GetSplashWindow();

    MDrv_SYS_GetMMAP(E_SYS_MMAP_SCALER_DNR_BUF, &pDrvCtx->u32MemAddr, &pDrvCtx->u32MemSize);
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MIU1_BASE, &u32MIU1_MemAddr, &u32MIU1_MemSize);
    if(pDrvCtx->u32MemAddr >= u32MIU1_MemAddr)
    {
        pDrvCtx->u32MemAddr -= u32MIU1_MemAddr;
        u8MIU_num = 1;
    }

    u32MenuloadMemAddr = pDrvCtx->u32MemAddr;
    u32MenuloadMemSize = 0x4000;    // 16KB
    pDrvCtx->u32MemAddr += 0x4000;  // FB addr = original addr + 16KB
    pDrvCtx->u32MemSize -= 0x4000;  // FB size = original size - 16KB

    //MDrv_SYS_GetMMAP(E_SYS_MMAP_SCALER_MENU_LOAD, &u32MenuloadMemAddr, &u32MenuloadMemSize);
    //FL_DBG("(memuload)==> 0x%x, 0x%x\n", u32MenuloadMemAddr, u32MenuloadMemSize);
    if(bsplash == FALSE)
    {
        MHal_SC_Reset(SC_RST_ALL);
    }
    MHal_SC_RegInit(pDrvCtx->u32MemAddr, pDrvCtx->u32MemSize);
    MHal_SC_ML_Init(u32MenuloadMemAddr, u32MenuloadMemSize, u8MIU_num);
    if(bsplash == FALSE)
    {
        MHal_SC_SetClk();
    }

    // F2 DNR write limit
    // Michu 20090925
    MHal_SC_IPM_SetDNRWriteLimit((((pDrvCtx->u32MemAddr + pDrvCtx->u32MemSize) / BYTE_PER_WORD) - 1) | 0x02000000);

    // Init Video Mirror
    // Michu 20090903
    pDrvCtx->SrcInfo[0].bHMirror = FALSE;
    pDrvCtx->SrcInfo[0].bVMirror = FALSE;

    //(20100115 ykkim5) 20100113 daniel.huang: for solve sog sync unstable problem for some dvd-player with 480i timing
    MHal_SC_SetMacroVisionFilter(TRUE);

    // Init ADC
    MDrv_ADC_Init();

    // cc.chen - T.B.D.
    MHal_SC_OP2_SetColorMatrix(defaultColorMatrix);

    // HDMI
    //MDrv_HDMI_Init(); // LGE wlgnsl99

    // MACE
    MDrv_SC_MACE_Init(pDrvCtx);

    // GOP
    if(bsplash == FALSE)
    {
        MHal_SC_GOPINT_SetGOPEnable(TRUE);
        MHal_SC_OP2_SetOSDBlending(TRUE);
    }

    //DBK //[091201_Leo]
    MHal_SC_DBKInit();
    //HistogramData
    MDrv_SC_MACE_HistogramData_Init(pDrvCtx);
    //IHCHueInit
    MHal_SC_MACE_IHCHueInit();
    //BrightnessInit
    MHal_SC_BrightnessInit();
    // Init Scart SAR
    MHal_SC_ScartIDInit();

    init_timer(&vcnt_freeze_rst_timer);

    // cc.chen - SC ISR  //thchen 20080820
    //if (request_irq(DISP_IRQ_NUM, MDrv_SC_Interrupt, SA_INTERRUPT, "SC_IRQ", &_scaler_dev) != 0)
	if(0 == u32DEBIntRegister) {
    result = request_irq(E_IRQ_DEB, MDrv_SC_ISR, SA_INTERRUPT, "SC_IRQ", NULL);//LGE drmyung 081013
		if (result) {
			return -EBUSY;
		}
		u32DEBIntRegister= 1;
	} else {
		disable_irq(E_IRQ_DEB);
		enable_irq(E_IRQ_DEB);
	}

    //LGE [vivakjh] 2008/12/27		Request chaning the histogram read point(0x1a High(0x04)) to all 0 from PQ team
	if(pDrvCtx->pPanelInfo->u8LCDorPDP == 1)	// PDP Only
		MHal_SC_MACE_HistogramInit4PDP();
	else
	    MHal_SC_MACE_HistogramInit();//thchen 20080820


    MHal_SC_SetInterruptMask(SC_INT_VSINT, TRUE);//thchen 20080903
	MHal_SC_SetInterruptMask(SC_INT_F2_IPVS_SB, TRUE);//FitchHsu 20081125 JPEG issue for rotate

    MDrv_SCMAP_Init(0);
    MHal_SC_2D_Peaking_LBS();
// init Adaptive tuning
//Fitch 20081222
	MHal_Scaler_SetAdaptiveCtrl(ENABLE_SCALER_DEFEATHERING|ENABLE_SCALER_DEFLICKERING|ENABLE_SCALER_DEBOUNCING);

    SC_FPLL_DBG("##########MDrv_SC_SetPanelTiming( 2 )\n");
    if(bsplash == FALSE)
    {
        MDrv_SC_SetPanelTiming(pDrvCtx, FALSE, TRUE);
    }

#if ENABLE_OD// 20091012 Michu: add OD
    MDrv_SYS_GetMMAP(E_SYS_MMAP_OD_MSB_BUFFER, &u32ODmstMemAddr, &u32ODmstMemSize);
    MDrv_SYS_GetMMAP(E_SYS_MMAP_OD_LSB_BUFFER, &u32ODlstMemAddr, &u32ODlstMemSize);

    OD_MSB_START_ADDR = ((U32)u32ODmstMemAddr / 16 & 0xFFFFFF);
    //OD_MSB_LIMIT_ADDR = ((((((U32)u32ODmstMemAddr / 16 & 0xFFFFFF) + (U32)1920 * 1080 * 2)+4096)/8)+20);
    OD_MSB_LIMIT_ADDR = OD_MSB_START_ADDR + (U32)0x47310 + (U32)0x14;//+ (U32)((1920 * 1080 * 9) / (8 *8)) + 20;
    OD_LSB_START_ADDR = ((U32)u32ODlstMemAddr / 16 & 0xFFFFFF);
    //OD_LSB_LIMIT_ADDR = ((((((U32)u32ODlstMemAddr / 16 & 0xFFFFFF) + (U32)1920 * 1080 )+4096)/8)+20);
    OD_LSB_LIMIT_ADDR = OD_LSB_START_ADDR + (U32)0x2F760 + (U32)0x14;//(U32)((1920 * 1080 * 6) / (8 *8)) + 20;

    MHal_SC_OD_Init(OD_MSB_START_ADDR, OD_MSB_LIMIT_ADDR, OD_LSB_START_ADDR, OD_LSB_LIMIT_ADDR);
#endif

    MDrv_SYS_LoadInitBWTable();
    return 0;
}

void MDrv_SC_SetDuplicate(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    BOOL param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(BOOL)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[SC_MAIN_WINDOW];
    pSrcInfo->bHDuplicate = param;
    SC_DBG("pSrcInfo->bHDuplicate:%u\n", pSrcInfo->bHDuplicate);
}

void MDrv_SC_HDMI_Init(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg) // LGE wlgnsl99
{
    // HDMI
    MDrv_HDMI_Init();
}

void MDrv_SC_SetPanelOutput(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bEnable)
{
    if (bEnable)
    {
        if (pDrvCtx->pPanelInfo->u8LPLL_Type == LPLL_TYPE_LVDS)
        {
            MHal_SC_VOP_EnableClock(SC_ENCLK_SIGNAL_LVDS);
        }
        else if (pDrvCtx->pPanelInfo->u8LPLL_Type == LPLL_TYPE_TTL)
        {
            MHal_SC_VOP_EnableClock(SC_ENCLK_TTL);
        }
		// shjang_090904
		else if (pDrvCtx->pPanelInfo->u8LPLL_Type == LPLL_TYPE_MINILVDS)
		{
			// ???
		}
    }
    else
    {
        #if 0 // cc.chen - T.B.D.
        if (devPanel_IsTTL())
            MDrv_WriteByte(L_BK_VOP(0x46), 0x00);
        else
            MDrv_WriteByte(L_BK_VOP(0x46), 0xFF);
        #endif
    }

    MDrv_SC_SetMODPower(pDrvCtx, bEnable);
}

void MDrv_SC_SetNoSignalColor(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_NO_SIGNAL_COLOR_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_NO_SIGNAL_COLOR_t)))
    {
        return;
    }

    MHal_SC_OP2_SetNoSignalColor(param.u8Color);
}
//--------------------------------------------------------------------------------------------------
// Input source
//--------------------------------------------------------------------------------------------------

U32 MDrv_SC_SetInputSource(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_INPUTSOURCE_t input;
    PSC_SOURCE_INFO_t pSrcInfo;
    HAL_SC_SRCMUX_e srcmux = SC_SRCMUX_VIDEO;
    HAL_SC_ISYNC_e  isync  = SC_ISYNC_AUTO;
    HAL_SC_VPSELECT_e vpselect = SC_VPSELECT_CCIR656_A;

    if (copy_from_user(&input, (void __user *)arg, sizeof(SC_SET_INPUTSOURCE_t)))
    {
        return -EFAULT;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[input.srcIdx];

    // store source info into driver context
    pSrcInfo->SrcType = input.inputSrc;

    //==============================
    // Stop Thread
    //==============================
    if (SC_GET_SRC_STATE(pSrcInfo) & SC_SRC_STATE_PCMODE)
    {
        MDrv_SC_WakeupEvent(pSrcInfo, SC_EVENT_STOP);   // prevent infinite waiting IOCTL_SC_WAIT_EVENT in ap
        MDrv_SC_PCMode_Stop(pSrcInfo);                  // stop pcmode thread
        SC_CLR_SRC_STATE(pSrcInfo, SC_SRC_STATE_PCMODE);
    }
    else if (SC_GET_SRC_STATE(pSrcInfo) & SC_SRC_STATE_HDMI)
    {
        MDrv_SC_WakeupEvent(pSrcInfo, SC_EVENT_STOP);   // prevent infinite waiting IOCTL_SC_WAIT_EVENT in ap
        MDrv_HDMI_Stop(pSrcInfo);                       // stop hdmi thread
        SC_CLR_SRC_STATE(pSrcInfo, SC_SRC_STATE_HDMI);
    }
#if 0 //080912 LGE drmyung
    else if (SC_GET_SRC_STATE(pSrcInfo) & SC_SRC_STATE_LPLL) //thchen 20080904 //080912 LGE drmyung
    {
        MDrv_SC_WakeupEvent(pSrcInfo, SC_EVENT_STOP);   // prevent infinite waiting IOCTL_SC_WAIT_EVENT in ap
        MDrv_SC_LPLL_Stop(pSrcInfo);                       // stop hdmi thread
        SC_CLR_SRC_STATE(pSrcInfo, SC_SRC_STATE_LPLL);
    }
#endif

    //==============================
    // Special Setting for each input.
    //==============================
    g_SrcType = input.inputSrc;
    switch (input.inputSrc)
    {
    case INPUT_SOURCE_VGA:
        SC_DBG("VGA\n");
        srcmux = SC_SRCMUX_ANALOG_1;
        isync  = SC_ISYNC_AUTO;
        pSrcInfo->u16Input_SigStatus &= ~SC_INPUT_SIG_YUV_DOMAIN;
		pSrcInfo->u8Input_SyncStatus &= ~SC_SYNCSTS_INTERLACE_BIT;//091106 ykkim5, pc입력전환시 progress모드로 set.ATV<->PC(1280x1024)문제.
        break;

    case INPUT_SOURCE_YPBPR_1:
        SC_DBG("YPbPr\n");
        srcmux = SC_SRCMUX_ANALOG_1;
        isync  = SC_ISYNC_SOG;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        break;
    case INPUT_SOURCE_YPBPR_2:
        SC_DBG("YPbPr\n");
        srcmux = SC_SRCMUX_ANALOG_1;
        isync  = SC_ISYNC_SOG;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        break;

    case INPUT_SOURCE_HDMI_A: // LGE drmyung 080922
		//MDrv_MICOM_RegWrite(0x106A, 1);  //thchen 20090117
        SC_DBG("HDMI_A\n");
        srcmux = SC_SRCMUX_HDMI;
        isync  = SC_ISYNC_AUTO;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        MHal_HDMI_Reset();
        break;
    case INPUT_SOURCE_HDMI_B: // LGE drmyung 080922
		//MDrv_MICOM_RegWrite(0x106A, 2);  //thchen 20090117
        SC_DBG("HDMI_B\n");
        srcmux = SC_SRCMUX_HDMI;
        isync  = SC_ISYNC_AUTO;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        MHal_HDMI_Reset();
        break;
    case INPUT_SOURCE_HDMI_C: // LGE drmyung 080922
		//MDrv_MICOM_RegWrite(0x106A, 1);  //thchen 20090117
        SC_DBG("HDMI_C\n");
        srcmux = SC_SRCMUX_HDMI;
        isync  = SC_ISYNC_AUTO;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        MHal_HDMI_Reset();
        break;

    case INPUT_SOURCE_HDMI_D:
        SC_DBG("HDMI_D\n");
        srcmux = SC_SRCMUX_HDMI;
        isync  = SC_ISYNC_AUTO;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        MHal_HDMI_Reset();
        break;

    case INPUT_SOURCE_ATV:
        SC_DBG("ATV\n");
        srcmux = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
        break;

    case INPUT_SOURCE_CVBS_1: //main AV
        SC_DBG("CVBS1\n");
        srcmux   = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
        break;

    case INPUT_SOURCE_CVBS_2: //Side AV
        SC_DBG("CVBS2\n");
        srcmux   = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
        break;
    case INPUT_SOURCE_CVBS_3:
        SC_DBG("CVBS3\n");
        srcmux   = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
        break;

    case INPUT_SOURCE_SVIDEO_1:
        SC_DBG("SV1\n");
        srcmux = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
        break;

    case INPUT_SOURCE_SVIDEO_2:
        SC_DBG("SV2\n");
        srcmux = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
        break;

    case INPUT_SOURCE_SCART_1:
        SC_DBG("SCART 1\n");
        srcmux = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
        break;

    case INPUT_SOURCE_SCART_2:
        SC_DBG("SCART 2\n");
        srcmux   = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
        break;

    case INPUT_SOURCE_DTV:
        SC_DBG("DTV\n");
        srcmux   = SC_SRCMUX_VIDEO;
        vpselect = SC_VPSELECT_VD_A;
        pSrcInfo->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
        break;

    // unknow source
    default:
        SC_DBG("Unknow\n");
        break;
    }

    //20091218 Daniel&Michu
    //==============================
    // Prevent garbage occurred from high resolution to another high resolution
    //==============================
    MHal_SC_IPM_SetMemFetchOffset(0, 0);
    MHal_SC_OPM_SetMemFetchOffset(0, 0);

    //==============================
    // IP MUX
    //==============================
    MDrv_SC_SetIPMux(input.inputSrc);

    //==============================
    // IP source mux
    //==============================
    MHal_SC_SetSourceMux(srcmux);
    if (Use_Analog_Source(input.inputSrc) || Use_HDMI_Source(input.inputSrc))
    {
        MHal_SC_SetInputSyncType(isync);
    }
    else
    {
        MHal_SC_SetVideoPortSelect(vpselect);
    }

    //==============================
    // IP settings
    //==============================
    MHal_SC_SetDEOnlyMode(FALSE);
    if (Use_VGA_Source(input.inputSrc))
    {
        MHal_SC_InitForVGA();
        MHal_SC_IP1_SetCoastWin(0x00, 0x00);
    }
    else if (Use_YPbPr_Source(input.inputSrc))
    {
        MHal_SC_InitForYPbPr();
        MHal_SC_IP1_SetCoastWin(0x00, 0x00);
		//MHal_ADC_SetSOGThreshold(_stSogSyncLevel.u8720);
    }
    else if (Use_VD_Source((input.inputSrc)))
    {
        MHal_SC_InitForVD();
		pSrcInfo->u8VideoSystem = VD_NOTSTANDARD;// LGE drmyung 081008 : init for frame lock
    }
    else if (Use_HDMI_Source(input.inputSrc))
    {
        MHal_SC_InitForHDMI();
        MHal_SC_SetDEOnlyMode(TRUE);
    }
    else
    {
        MHal_SC_InitForDC();
    }

    //==============================
    // Filed detect
    //==============================
    MDrv_SC_IP1_SetFieldDetect(&pDrvCtx->SrcInfo[input.srcIdx]);

    OS_Delayms(2);

    //==============================
    // Start Thread
    //==============================
    if (Use_VGA_Source(input.inputSrc) || Use_YPbPr_Source(input.inputSrc))
    {
        MDrv_SC_PCMode_Start(pSrcInfo);
        SC_SET_SRC_STATE(pSrcInfo, SC_SRC_STATE_PCMODE);
    }
    else if (Use_HDMI_Source(input.inputSrc))
    {
        MDrv_HDMI_Start(pSrcInfo);
        SC_SET_SRC_STATE(pSrcInfo, SC_SRC_STATE_HDMI);
    }

    SC_SET_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_ALL);
    SC_SET_SRC_STATE(pSrcInfo, SC_SRC_STATE_INPUT_SELECT);


    return 0;
}

void MDrv_SC_SetMVDSigInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_MVD_SIG_INFO_t mvdinfo;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&mvdinfo, (void __user *)arg, sizeof(SC_SET_MVD_SIG_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[mvdinfo.srcIdx];

    if (mvdinfo.bInterlace)
    {
        pSrcInfo->u8Input_SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
    }
    else
    {
        pSrcInfo->u8Input_SyncStatus &= ~SC_SYNCSTS_INTERLACE_BIT;
    }

    pSrcInfo->u16Input_VFreq = mvdinfo.u16VFreq;
    pSrcInfo->u16Input_HActive = mvdinfo.u16HActive;
    pSrcInfo->u16Input_VActive = mvdinfo.u16VActive;
    pSrcInfo->u16OriInput_VFreq = pSrcInfo->u16Input_VFreq; //FitchHsu 20090121 EMP preview tearing after normal play

	FL_DBG("MDrv_SC_SetMVDSigInfo : HFreq[%d] VFreq[%d] HActive[%d] VActive[%d]\n",
		pSrcInfo->u16Input_HFreq,
		pSrcInfo->u16Input_VFreq,
		pSrcInfo->u16Input_HActive,
		pSrcInfo->u16Input_VActive);


    SC_SET_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_ALL);
}

void MDrv_SC_SetVDSigInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_VD_SIG_INFO_t vdinfo;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&vdinfo, (void __user *)arg, sizeof(SC_SET_VD_SIG_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[vdinfo.srcIdx];

    pSrcInfo->u16Input_HFreq = vdinfo.u16HFreq; // LGE drmyung 081008
    pSrcInfo->u16Input_VFreq = vdinfo.u16VFreq; // LGE drmyung 081008
    pSrcInfo->u16Input_HPeriod = vdinfo.u16HTotal;
    pSrcInfo->u16Input_VTotal  = vdinfo.u16VTotal;

	pSrcInfo->u8VideoSystem = vdinfo.u8VideoSystem;

	FL_DBG("MDrv_SC_SetVDSigInfo : VideoSystem[%d] HFreq[%d] VFreq[%d] HTotal[%d] VTotal[%d]\n",
		pSrcInfo->u8VideoSystem,
		pSrcInfo->u16Input_HFreq,
		pSrcInfo->u16Input_VFreq,
		pSrcInfo->u16Input_HPeriod,
		pSrcInfo->u16Input_VTotal);

    SC_SET_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_ALL);
}

void MDrv_SC_SetSCSigInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg) // LGE drmyung 081022
{
    SC_SET_SC_SIG_INFO_t scinfo;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&scinfo, (void __user *)arg, sizeof(SC_SET_SC_SIG_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[scinfo.srcIdx];

	// LGE drmyung 081106 : for the frame lock, input vfreq have to update in thread(case of hdmi and pcmode)
	//pSrcInfo->u16Input_VFreq = scinfo.u16VFreq;
	FL_DBG("MDrv_SC_SetSCSigInfo : HFreq[%d] VFreq[%d] HTotal[%d] VTotal[%d]\n",
		scinfo.u16HFreq,
		scinfo.u16VFreq,
		scinfo.u16HTotal,
		scinfo.u16VTotal);
	/*
	lachesis_090114 format change, input change시 sync가 unstable한 상태에서 MDrv_SC_SetSCSigInfo가 call되어
	output V total 설정이 변경되어 flicker가 발생함.
	debounce check후 sync가 안정화 되면, SearchModeHandler에서 bIsSupportMode로 판된되면 정보를 넘겨주도록 한다.
	*/

}

void MDrv_SC_SetCaptureWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_CAPTURE_WIN_t capwin;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&capwin, (void __user *)arg, sizeof(SC_CAPTURE_WIN_t)))
    {
        return;
    }

    pDrvCtx->SrcInfo[capwin.srcIdx].u16H_CapStart = capwin.u16HStart;
    pDrvCtx->SrcInfo[capwin.srcIdx].u16H_CapSize  = capwin.u16HSize;
    pDrvCtx->SrcInfo[capwin.srcIdx].u16H_CapStart_Backup = capwin.u16HStart;
    pDrvCtx->SrcInfo[capwin.srcIdx].u16H_CapSize_Backup = capwin.u16HSize;
    pDrvCtx->SrcInfo[capwin.srcIdx].u16V_CapStart = capwin.u16VStart;
    pDrvCtx->SrcInfo[capwin.srcIdx].u16V_CapSize  = capwin.u16VSize;
    pDrvCtx->SrcInfo[capwin.srcIdx].u16CropLBoffset = 0;

#if 0
	FL_DBG("CAP_WIN %dx%d %dx%d\n", pDrvCtx->SrcInfo[capwin.srcIdx].u16H_CapStart,
									pDrvCtx->SrcInfo[capwin.srcIdx].u16V_CapStart,
									pDrvCtx->SrcInfo[capwin.srcIdx].u16H_CapSize,
									pDrvCtx->SrcInfo[capwin.srcIdx].u16V_CapSize);
#endif

    pSrcInfo = &pDrvCtx->SrcInfo[capwin.srcIdx];
#ifdef TQM_MODE
	//LGE gbtogether(081015) by FitchHsu
    //if((pSrcInfo->SrcType == INPUT_SOURCE_DTV) && (capwin.u16HSize < 704))
    if(pSrcInfo->bHDuplicate == TRUE )
    {
        pSrcInfo->u16H_CapSize *= 2;
    }

#endif

    SC_ADD_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_CAP_WIN);
    SC_SET_SRC_STATE(&pDrvCtx->SrcInfo[capwin.srcIdx], SC_SRC_STATE_CAPTURE_WIN);

}

void MDrv_SC_GetCaptureWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_CAPTURE_WIN_t capWin;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&capWin, (void __user *)arg, sizeof(SC_CAPTURE_WIN_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[capWin.srcIdx];
    capWin.u16HStart = pSrcInfo->u16H_CapStart;
    capWin.u16HSize  = pSrcInfo->u16H_CapSize;
    capWin.u16VStart = pSrcInfo->u16V_CapStart;
    capWin.u16VSize  = pSrcInfo->u16V_CapSize;
    if (Use_YPbPr_Source(pSrcInfo->SrcType))
    {
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
            capWin.u16HStart = pSrcInfo->u16H_CapStart_Backup;
            capWin.u16HSize  = pSrcInfo->u16H_CapSize_Backup;
        }
        #endif
    }

#ifdef TQM_MODE
    //LGE gbtogether(081015) by FitchHsu
   // if((pSrcInfo->SrcType == INPUT_SOURCE_DTV) && (pSrcInfo->bHDuplicate == TRUE))
	if(pSrcInfo->bHDuplicate == TRUE )
	{
        capWin.u16HSize /= 2;
    }
#endif

    if (copy_to_user((U32*)arg, (U32*)&capWin, sizeof(SC_DISPLAY_WIN_t)))
    {
        return;
    }
}

//20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MDrv_SC_GetRealCaptureWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_CAPTURE_WIN_t capWin;

    if (copy_from_user(&capWin, (void __user *)arg, sizeof(SC_CAPTURE_WIN_t)))
    {
        return;
    }

    MHal_SC_GetCapWin(&capWin.u16HStart, &capWin.u16HSize, &capWin.u16VStart, &capWin.u16VSize);

    if (copy_to_user((U32*)arg, (U32*)&capWin, sizeof(SC_DISPLAY_WIN_t)))
    {
        return;
    }
}

void MDrv_SC_SetDisplayWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_DISPLAY_WIN_t dispwin;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&dispwin, (void __user *)arg, sizeof(SC_DISPLAY_WIN_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[dispwin.srcIdx];

    // 20091118 daniel.huang: fix MHEG5 window position incorrect under mirror mode
    if (pSrcInfo->bHMirror) {
        dispwin.u16HStart = pDrvCtx->pPanelInfo->u16Width - (dispwin.u16HStart+dispwin.u16HSize);
    }
    if (pSrcInfo->bVMirror) {
        dispwin.u16VStart = pDrvCtx->pPanelInfo->u16Height - (dispwin.u16VStart+dispwin.u16VSize);
    }

    pDrvCtx->SrcInfo[dispwin.srcIdx].u16H_DispSize  = dispwin.u16HSize;
    pDrvCtx->SrcInfo[dispwin.srcIdx].u16V_DispSize  = dispwin.u16VSize;
    pDrvCtx->SrcInfo[dispwin.srcIdx].u16H_DispStart = dispwin.u16HStart;
    pDrvCtx->SrcInfo[dispwin.srcIdx].u16V_DispStart = dispwin.u16VStart;


#if	1	// lemonic LGE 081120
	_gDispwin.u16HStart= pDrvCtx->pPanelInfo->u16HStart + dispwin.u16HStart;
	_gDispwin.u16HSize = pDrvCtx->pPanelInfo->u16HStart + dispwin.u16HStart + dispwin.u16HSize - 1;
	_gDispwin.u16VStart= pDrvCtx->pPanelInfo->u16DE_VStart + dispwin.u16VStart;
	_gDispwin.u16VSize = pDrvCtx->pPanelInfo->u16DE_VStart + dispwin.u16VStart + dispwin.u16VSize - 1;
#else
    MHal_SC_VOP_SetDispWin(
        pDrvCtx->pPanelInfo->u16HStart + dispwin.u16HStart,
        pDrvCtx->pPanelInfo->u16HStart + dispwin.u16HStart + dispwin.u16HSize - 1,
        pDrvCtx->pPanelInfo->u16DE_VStart + dispwin.u16VStart,
        pDrvCtx->pPanelInfo->u16DE_VStart + dispwin.u16VStart + dispwin.u16VSize - 1);
#endif

    SC_ADD_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_DISP_WIN);
    SC_SET_SRC_STATE(&pDrvCtx->SrcInfo[dispwin.srcIdx], SC_SRC_STATE_DISPLAY_WIN);
}

void MDrv_SC_GetDisplayWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_DISPLAY_WIN_t dispwin;

    if (copy_from_user(&dispwin, (void __user *)arg, sizeof(SC_DISPLAY_WIN_t)))
    {
        return;
    }

    dispwin.u16HStart = pDrvCtx->SrcInfo[dispwin.srcIdx].u16H_DispStart;
    dispwin.u16HSize  = pDrvCtx->SrcInfo[dispwin.srcIdx].u16H_DispSize;
    dispwin.u16VStart = pDrvCtx->SrcInfo[dispwin.srcIdx].u16V_DispStart;
    dispwin.u16VSize  = pDrvCtx->SrcInfo[dispwin.srcIdx].u16V_DispSize;

    if (copy_to_user((U32*)arg, (U32*)&dispwin, sizeof(SC_DISPLAY_WIN_t)))
    {
        return;
    }
}

void MDrv_SC_Active(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_ACTIVE_t param;
    PSC_SOURCE_INFO_t pSrcInfo;
    BOOL bEnCSC, bUseVIPCSC;
#ifdef  MEASURE_TIME
    U32 u32Timer;
#endif

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_ACTIVE_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

    //----------------------------------
    // Pre-Setting
    //----------------------------------
    MDrv_SCMAP_DesideSrcType(SC_MAIN_WINDOW, pSrcInfo);

#if 1
    // 20091211 daniel.huang: turn on 3 frame buffer mode under progressive input
    if(!(pSrcInfo->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT))
        pSrcInfo->u8NumOfFB = MAX_FRAMEBUFFER_NUM;
    else
        pSrcInfo->u8NumOfFB = 2;
#else
    if(pDrvCtx->pPanelInfo->u8LCDorPDP == 1)    // PDP Only
    {
        if(pSrcInfo->bEmppreview && !(pSrcInfo->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT))
            pSrcInfo->u8NumOfFB = MAX_FRAMEBUFFER_NUM;
        else
            pSrcInfo->u8NumOfFB = 2;
    }
    else    // LCD
    {
        //FitchHsu 20090121 EMP preview tearing after normal play
        if((pSrcInfo->bEmppreview) && (!(pSrcInfo->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)) && (pSrcInfo->u16OriInput_VFreq < 450) && (pSrcInfo->u16OriInput_VFreq > 200))
            pSrcInfo->u8NumOfFB = MAX_FRAMEBUFFER_NUM;
        else
            pSrcInfo->u8NumOfFB = 2;
    }
#endif
    //20091225 daniel.huang: for 3 frame mode crop setting incorrect
    if (pSrcInfo->u8NumOfFB == 3)
        pSrcInfo->bLinearAddrMode = TRUE;
    else
        pSrcInfo->bLinearAddrMode = FALSE;

    //----------------------------------
    // Config
    //----------------------------------
    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_FMT_CHG))
    {
        MDrv_SC_SetMode(pSrcInfo);
        MDrv_SCMAP_SetMemFormat(param.srcIdx, &pSrcInfo->u8BitPerPixel, &pSrcInfo->bMemFormat422);

        MDrv_SYS_LoadBWTable(QM_GetInputHSize(pSrcInfo), QM_GetInputVSize(pSrcInfo), QM_IsInterlaced(pSrcInfo));
    }

    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_CROP_WIN))
    {
        MDrv_SC_SetCrop(pSrcInfo);
    }

    //----------------------------------
    // IP1
    //----------------------------------
    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_FMT_CHG))
    {
        MDrv_SC_IP1_SetFieldDetect(pSrcInfo);

    //----------------------------------
    // IP2
    //----------------------------------

        // 20090828 daniel.huang: add VIP CSC for PCRGB & DVI
        bEnCSC = !(pSrcInfo->u16Input_SigStatus & SC_INPUT_SIG_YUV_DOMAIN);
        if((Use_VGA_Source(pSrcInfo->SrcType)) ||
          ((Use_HDMI_Source(pSrcInfo->SrcType) && (pSrcInfo->bHDMIMode == FALSE) && (!(pSrcInfo->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)))))
        {
            bUseVIPCSC = TRUE;
        }
        else
        {
            bUseVIPCSC = FALSE;
        }
        MHal_SC_IP2_SetCSC(bEnCSC, bUseVIPCSC);
    }
    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_FMT_CHG) ||
        SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_DISP_WIN))
    {
        MDrv_SC_IP2_HScaling(pSrcInfo);
        MDrv_SC_IP2_VScaling(pSrcInfo);
    }
    //----------------------------------
    // IPM
    //----------------------------------
    MDrv_SC_IPM_CalMemSetting(pSrcInfo, pDrvCtx->u32MemAddr, pDrvCtx->u32MemSize);

    //----------------------------------
    // OP1
    //----------------------------------

    //----------------------------------
    // OPM
    //----------------------------------
    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_CROP_WIN))
    {
        // Michu 20090903
        MDrv_SC_OPM_CalMemSetting(pSrcInfo);
        MDrv_SC_OP1_CalVScaling(pSrcInfo);
        MDrv_SC_OP1_CalHScaling(pSrcInfo);
    }

    //----------------------------------
    // OP2
    //----------------------------------
    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_FMT_CHG))
    {
        MDrv_SC_OP2_SetCSC(pSrcInfo);
    }


#ifdef MEASURE_TIME
    REG_WR(REG_PIU_TIMER0(0x00), 0x0);
    REG_WR(REG_PIU_TIMER0(0x00), 0x1);
#endif
    MDrv_Set_QMap(SC_MAIN_WINDOW);
#ifdef  MEASURE_TIME
    u32Timer = ((U32)REG_RR(REG_PIU_TIMER0(0x04))) |
               ((U32)REG_RR(REG_PIU_TIMER0(0x05)) << 16); // h0005:h0004
    printk("#### qmap time=%u us\n", u32Timer/12);
#endif



#if 0   // for T3 bringup
    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_FMT_CHG))
    {
        if( pSrcInfo->u16H_CapSize!=0 && pSrcInfo->u16V_CapSize!=0 &&
            pSrcInfo->u16H_CropSize!=0 && pSrcInfo->u16V_CropSize!=0 )
        {
            MHal_SC_PQ_Protect_FastPlayback(); //FitchHsu 20081113 EMP when PAUSE, little shaking
        }
    }
#endif

    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_FMT_CHG))
    {
        if(pSrcInfo->bFBL)
        {
            MHal_SC_IPM_SetFBL(pSrcInfo->bFBL);
        }
    }

    MDrv_SC_TriggerByVSync(pSrcInfo);

    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_CAP_WIN))//for improve speed
    {
        //if(bisr_input_ready)
        {
            isr_inputCnt = 0;
			SC_SPEED_DBG("\n isr_inputCnt reset for SC_ACTIVE_FLG_FMT_CHG\n");
        }
    }

	// FitchHsu 20090605 patch Sherwood USA DVD player timing invalid format
#if SC_YPBPR_720x480_60_SW_PATCH
    if (SC_CHK_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_FMT_CHG))
    {
        if (Use_YPbPr_Source(pSrcInfo->SrcType) &&(pSrcInfo->Modesetting.u8ModeIndex == MD_720x480_60I_P))
        {
            pSrcInfo->u8Input_SyncStatus = pSrcInfo->u8Input_SyncStatus & (~(SC_SYNCSTS_INTERLACE_BIT));
        }
    }
#endif

    SC_CLR_ACTIVE_FLAG(pSrcInfo);
    SC_SET_SRC_STATE(pSrcInfo, SC_SRC_STATE_ACTIVE);
}

void MDrv_SC_SetBlackScreen(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_BLACK_SCREEN_t param;
    PSC_SOURCE_INFO_t pSrcInfo;
    U8 u8PreMotionGain = 0;
    U32 u32Time; //LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_BLACK_SCREEN_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    pSrcInfo->binputsync = TRUE; //LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue

	if(param.bEnable)
	{
		FL_DBG("\n +++++++++++++MDrv_SC_SetBlackScreen Enabled+++++++++++++\n");
		MDrv_SC_IP1_WaitOutputVSync(1, 40); // LGE drmyung 081027 : mute 시 영상 전체 차단 안됨 개선.

		MDrv_Scaler_GenerateBlackVideo(param.bEnable);//090904 drmyung

        //marked, reset just do once before black screen disable, [091012_Leo]
		//MHal_SC_FilmModeReset(1); //LGE gbtogether(081128) --> SD(Film) to HD(non-Film) issue by FitchHsu

		gIsVideoMute = TRUE;
	}
	else //  black screen disable
	{
		//LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
		if((pSrcInfo->bTimingCHG) && Use_DTV_Source(pSrcInfo->SrcType))
		{
		    u8PreMotionGain = MHal_SC_Pre_Memory_Motion_Gain(0);
		}
		//resolution change시 화면 잔상 남는 문제로 DNR을 refresh함.
		//화면 mute가 풀리기전 다시 enable.
		//godfather99
		MHal_SC_EnableDNR(FALSE);

		gIsVideoMute = FALSE; // LGE drmyung 081129 : Timing 중요함.
		// 아래 delay 후에 gIsVideoMute 설정 시 stable->unstable->stable 상태 발생 시 Free Run에서 Frame Lock 다시 못하는 경우 발생 가능성 있슴.
		u32Time = jiffies; //LGE [vivakjh] 2009/01/18 Merge!!		 FitchHsu 20090116 VGA Lock issue
		while(isr_inputCnt < ISR_INPUT_VSYNC_CNT) // 5 input v sync
		{
			//SC_SPEED_DBG("\n disable mute isr_inputCnt=%d\n", isr_inputCnt);
			msleep(1);

			//LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue
			if ((jiffies - u32Time) > OUTPUT_VSYNC_TIMEOUT)
			{
			    isr_inputCnt = ISR_INPUT_VSYNC_CNT;
			    pDrvCtx->SrcInfo[0].binputsync = FALSE; //LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue
                break;
            }
		}

		// LGE [vivakjh]  2008/12/11	PDP 경우 Fast Frame Lock을 못하므로 Frame Lock시점까지 Film Mode를 Off 했다가 Frame Lock 되면 On해줌.
		if(pDrvCtx->pPanelInfo->u8LCDorPDP == 1)	// PDP Only
		{
			_gFilmMode = MHal_SC_GetFilmMode();
			if (_gFilmMode)
				MHal_SC_FilmEnable(FALSE, _gFilmMode-1);
		}

		if(Use_DTV_Source(pSrcInfo->SrcType))
		{
            //Reset FilmMode Counter, set to 0 and then dump the setting of the coming input, [091012_Leo]
    		MHal_SC_FilmModeReset(); //LGE gbtogether(081128) --> SD(Film) to HD(non-Film) issue by FitchHsu
            MDrv_SCMAP_SetFilmMode(0);
		}

		//godfather99 DNR enable
		MHal_SC_EnableDNR(TRUE);
        if(pDrvCtx->pPanelInfo->u8LPLL_Type == LPLL_TYPE_MINILVDS)
        {
            MHal_TCON_Count_Reset(0);
            MHal_TCON_Count_Reset(1);
        }
		MDrv_SC_IP1_WaitOutputVSync(1, 40); // LGE lemonic 081213 Mute off시 간헐적으로 Mute off 과정 보임 개선.
        if((pSrcInfo->bTimingCHG) && Use_DTV_Source(pSrcInfo->SrcType))
        {
            MHal_SC_Pre_Memory_Motion_Gain(u8PreMotionGain); //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
        }
		MDrv_Scaler_GenerateBlackVideo(param.bEnable);//090904 drmyung

		FL_DBG("\n +++++++++++++MDrv_SC_SetBlackScreen Disabled+++++++++++++\n");
//		gIsVideoMute = FALSE;

	}
#if 0//ndef NEW_FRAME_LOCK//lachesis_090831
    param.u8FrameLock = pSrcInfo->u8FrameLock;
    param.u16VFreq = pSrcInfo->u16OutputVFreq;

    if (copy_to_user((SC_SET_BLACK_SCREEN_t*)arg, (SC_SET_BLACK_SCREEN_t*)&param, sizeof(SC_SET_BLACK_SCREEN_t)))
    {
        return;
    }
#endif
}
//lachesis_090831
void MDrv_SC_SetFreerun(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
	SC_SET_BLACK_SCREEN_t param;
	PSC_SOURCE_INFO_t pSrcInfo;

	if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_BLACK_SCREEN_t)))
	{
		return;
	}
	pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

	if(param.bEnable)
	{
		SC_SPEED_DBG("\n +++++++++++++MDrv_SC_SetFreerun Enabled+++++++++++++\n");
		MHal_SC_IP1_CtrlHistoDataReport(DISABLE);

		param.u16VFreq = MDrv_SC_SetPanelTiming(pDrvCtx, FALSE, FALSE);
	}
	else //  black screen disable
	{
		param.u16VFreq = MDrv_SC_SetPanelTiming(pDrvCtx, TRUE, FALSE);
		MHal_SC_IP1_CtrlHistoDataReport(ENABLE);

		//bisr_input_ready = 1;
		isr_inputCnt = 0;
		//SC_SPEED_DBG("\n reset inputCnt=%d\n", isr_inputCnt);

		SC_SPEED_DBG("\n +++++++++++++MDrv_SC_SetFreerun Disabled+++++++++++++\n");
	}

    param.u8FrameLock = pSrcInfo->u8FrameLock;
    param.u16VFreq = pSrcInfo->u16OutputVFreq;

    if (copy_to_user((SC_SET_BLACK_SCREEN_t*)arg, (SC_SET_BLACK_SCREEN_t*)&param, sizeof(SC_SET_BLACK_SCREEN_t)))
    {
        return;
    }
}

//CDNR Enable, [090615_Leo]
void MDrv_SC_SetCDNREnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_CDNR_ENABLE_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CDNR_ENABLE_t)))
    {
        return;
    }
    MHal_SC_EnableColorFunction(param.bEnable);
}

void MDrv_SC_SetCDNRIndex(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_CDNR_INDEX_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CDNR_INDEX_t)))
    {
        return;
    }
    MHal_SC_SetCDNRIndex(param.pIndex);
}

void MDrv_SC_SetCDNRGain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_CDNR_GAIN_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CDNR_GAIN_t)))
    {
        return;
    }
    MHal_SC_SetCDNRGain(param.pDnrGain, param.pPreSnrGain);
}

void MDrv_SC_SetAutoNREnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_AUTO_NR_ENABLE_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_AUTO_NR_ENABLE_t)))
    {
        return;
    }

#if 0 //[090804_Leo]
    if(param.bOnOff)
    {
        MHal_SC_DynamicNRInATV();
    }
    else
    {
        return;
    }
#else
    gControlAutoNR = param.bOnOff;
#endif
}
void MDrv_SC_SetBrightness(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_BRIGHTNESS_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_BRIGHTNESS_t)))
    {
        return;
    }

    MHal_SC_OP2_SetBrightness(param.u16Brightness);//change U8 to U16, [090921_Leo]
}

void MDrv_SC_VOP_SetFreeRunColorEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bEnable)
{
    MHal_SC_VOP_SetFreeRunColorEnable(bEnable);
}

void MDrv_SC_VOP_SetFreeRunColor(PSC_DRIVER_CONTEXT_t pDrvCtx, SC_FREERUN_COLOR_e color)
{
    MHal_SC_VOP_SetFreeRunColor(color);
}

void MDrv_SC_SetFrameRate(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_FRAMERATE_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_FRAMERATE_t)))
    {
        return;
    }

	if(param.bIsForceFreeRun)
	{
		_gFrameTo48Hz = FALSE;
		pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
		pSrcInfo->u16Input_VFreq = param.u16FrameRate;// * 10; drmyung LGE 080701

        bIsForceFreeRun = TRUE;
		MDrv_SC_SetPanelTiming(pDrvCtx, TRUE, FALSE);//lachesis_090831
	}
	else
	{
		bIsForceFreeRun = FALSE;
	}
	FL_DBG("\n [[[[ FORCE FREE RUN %s ]]]\n\n", bIsForceFreeRun?"START":"END");
}

void MDrv_SC_GetFrameRate(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_FRAMERATE_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_FRAMERATE_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    param.u16FrameRate = pSrcInfo->u16Input_VFreq/10;

// LGE drmyung 080825    printk("pSrcInfo->u16Input_VFreq = %d\n", pSrcInfo->u16Input_VFreq);

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_FRAMERATE_t)))
    {
        return;
    }
}

void MDrv_SC_GetInputTimingInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_INPUT_TIMING_INFO_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_INPUT_TIMING_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];


    param.hFreq		= pSrcInfo->u16Input_SC_HFreq;
    param.vFreq		= pSrcInfo->u16Input_SC_VFreq;

    param.hTotal	= pSrcInfo->u16Input_SC_HPeriod;
	param.vTotal	= pSrcInfo->u16Input_SC_VTotal;

    param.hStart	= pSrcInfo->u16Input_SC_HStart;
    param.vStart	= pSrcInfo->u16Input_SC_VStart;
	param.hActive	= pSrcInfo->u16H_SC_CapSize;
    param.vActive	= pSrcInfo->u16V_SC_CapSize;

    if (pSrcInfo->u8Input_SC_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
    {
        param.scanType = 0;
    }
    else
    {
        param.scanType = 1;
    }
    param.phase = pSrcInfo->u8Input_SC_Phase;//Modesetting.u8Phase;

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_INPUT_TIMING_INFO_t)))
    {
        return;
    }
}

void MDrv_SC_GetHDMIInputTimingInfo(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_INPUT_TIMING_INFO_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_INPUT_TIMING_INFO_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

#if 1 // LGE drmyung 081103
	if(pSrcInfo->u16Input_HDMI_HFreq < 100 || pSrcInfo->u16Input_HDMI_VFreq < 200 )
	{
		param.hFreq   = 0;
		param.vFreq   = 0;
		param.hTotal  = 0;
		param.hStart  = 0;
		param.vStart  = 0;
		param.vTotal  = 0;
		param.hActive = 0;
		param.vActive = 0;
		param.scanType = 0;
		param.phase = 0;
	}
	else
	{
#endif
	param.hFreq   = pSrcInfo->u16Input_HDMI_HFreq;
	param.vFreq   = pSrcInfo->u16Input_HDMI_VFreq;
	param.hTotal  = pSrcInfo->u16Input_HDMI_HPeriod;
	param.hStart  = pSrcInfo->u16Input_HDMI_HDE_Start;
	param.vStart  = pSrcInfo->u16Input_HDMI_VDE_Start;
	param.vTotal  = pSrcInfo->u16Input_HDMI_VTotal;
	param.hActive = pSrcInfo->u16Input_HDMI_HDE;
	param.vActive = pSrcInfo->u16Input_HDMI_VDE;
	if (pSrcInfo->u8Input_HDMI_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
	{
		param.scanType = 0;
	}
	else
	{
		param.scanType = 1;
	}
	param.phase = 0;
#if 1 // LGE drmyung 081103
	}
#endif
    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_INPUT_TIMING_INFO_t)))
    {
        return;
    }
}

#if (ENABLE_SC_MMIO )//[20090920 Shawn] MM I/O
void* MDrv_SC_GetMMAPAddr(void)
{
    if(SC_BUF_ADR == 0)
    {
        return NULL;
    }
    else
    {
        //return MDrv_SYS_PA2NonCacheSeg((void*)SC_BUF_ADR);
        return ioremap(SC_BUF_ADR,SC_BUF_LEN);
    }
}

E_SC_MMIO _MDrv_SC_Copy_From_MMIO(SC_SET_GAMMA_TABLE_t *p_param)
{
    U8 u8EndChar = 0;
    U16 u16TableSize = 0;
    U8 u8SrcIdx = 0;
    volatile unsigned char* paddr = MDrv_SC_GetMMAPAddr();


    if(paddr == NULL)
    {
        /*debug infomation*/ SC_MMIO_PRINT("cannot alloc SC_BUF pages\n");
        return E_SC_MMIO_WRONG_MMAPADDR;
    }
    else
    /*debug infomation*/ SC_MMIO_PRINT("SC_BUF_ADR: 0x%x\n", SC_BUF_ADR);

    memcpy((void*)(&u8SrcIdx), (void*)paddr, sizeof(U8));
    p_param->srcIdx = u8SrcIdx;
    /*debug infomation*/ SC_MMIO_PRINT("paddr=0x%x, param.srcIdx = %d\n", (U32)paddr, p_param->srcIdx);
    paddr += sizeof(U8);

    memcpy((void*)(&p_param->u16NumOfLevel), (void*)paddr, sizeof(U16));
    /*debug infomation*/ SC_MMIO_PRINT("paddr=0x%x, param.u16NumOfLevel = %d\n", (U32)paddr, p_param->u16NumOfLevel);
    paddr += sizeof(U16);

    u16TableSize = (p_param->u16NumOfLevel*12/8);

    p_param->pu8Gamma_R = (U8*)paddr;
    /*debug infomation*/SC_MMIO_PRINT("RTbl paddr=0x%x, u16TableSize = %d\n", (U32)paddr, u16TableSize);
    paddr += u16TableSize;

    p_param->pu8Gamma_G = (U8*)paddr;
    /*debug infomation*/SC_MMIO_PRINT("GTbl paddr=0x%x,  \n", (U32)paddr);
    paddr += u16TableSize;

    p_param->pu8Gamma_B = (U8*)paddr;
    /*debug infomation*/SC_MMIO_PRINT("BTbl paddr=0x%x,  \n", (U32)paddr);
    paddr += u16TableSize;

    /*debug infomation*/SC_MMIO_PRINT("End paddr=0x%x,  \n", (U32)paddr);
    memcpy((void*)(&u8EndChar), (void*)paddr, sizeof(U8));
    paddr += sizeof(U8);

    if( u8EndChar != 0x99 )
    {
        SC_MMIO_PRINT("[%s]( %d ) Copy_Param_From_MMIO fails, 0x%x\n", "Mdrv_SC", __LINE__, u8EndChar);

        return E_SC_MMIO_WRONG_END_CHAR;
    }
    else
    {
        return TRUE;
    }


}
#endif

void MDrv_SC_SetGammaTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
#ifdef  MEASURE_TIME
    U32 u32Timer;
#endif
    SC_SET_GAMMA_TABLE_t param;
    U8* pGammaTableR = NULL;
    U8* pGammaTableG = NULL;
    U8* pGammaTableB = NULL;

    U16 u16TableSize;

#if (ENABLE_SC_MMIO)
    E_SC_MMIO eRet = _MDrv_SC_Copy_From_MMIO(&param);

    if( eRet!=E_SC_MMIO_OK)
    {
        /*debug infomation*/printk("Warning:_MDrv_SC_Copy_From_MMIO fails, %s\n",
            eRet == E_SC_MMIO_WRONG_END_CHAR? "E_SC_MMIO_WRONG_END_CHAR":"E_SC_MMIO_WRONG_MMAPADDR");
        return;
    }

#else //[20090920 Shawn] MM I/O
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_GAMMA_TABLE_t)))
    {
        return;
    }
#endif
    u16TableSize = param.u16NumOfLevel*3/2;
    pGammaTableR = kmalloc(u16TableSize, GFP_KERNEL);
    pGammaTableG = kmalloc(u16TableSize, GFP_KERNEL);
    pGammaTableB = kmalloc(u16TableSize, GFP_KERNEL);

    if (pGammaTableR && pGammaTableG && pGammaTableB)
    {

        if (param.u16NumOfLevel == 1024) //add condition, [090825_Leo]
        {
            U16 i, j;
            U16 u16Cnt = 0;
			U16 gammaLoadStart;
			U16 gammaLoadEnd;

            SC_PQ_DBG("1. load sampling gamma table in interval of 4 to first 256 entries, ");
            for (i = 0, j = 0; u16Cnt < 256; i += 3, j += 12)
            {
                pGammaTableR[i]   = (param.pu8Gamma_R[j] & 0x0F) | ((param.pu8Gamma_R[j+6] & 0x0F) << 4);
                pGammaTableG[i]   = (param.pu8Gamma_G[j] & 0x0F) | ((param.pu8Gamma_G[j+6] & 0x0F) << 4);
                pGammaTableB[i]   = (param.pu8Gamma_B[j] & 0x0F) | ((param.pu8Gamma_B[j+6] & 0x0F) << 4);

                pGammaTableR[i+1] = param.pu8Gamma_R[j+1];
                pGammaTableG[i+1] = param.pu8Gamma_G[j+1];
                pGammaTableB[i+1] = param.pu8Gamma_B[j+1];

                pGammaTableR[i+2] = param.pu8Gamma_R[j+7];
                pGammaTableG[i+2] = param.pu8Gamma_G[j+7];
                pGammaTableB[i+2] = param.pu8Gamma_B[j+7];
                u16Cnt +=2;
            }

            // 20091030 daniel.huang: fix 256/1024 gamma incorrect
            MHal_SC_OP2_SetRGBGammaMaxValue(pGammaTableR, pGammaTableG, pGammaTableB, 256);
            MDrv_SC_IP1_WaitOutputVSync(1, 50);//victor 20080909

#ifdef MEASURE_TIME
            REG_WR(REG_PIU_TIMER0(0x00), 0x0);
            REG_WR(REG_PIU_TIMER0(0x00), 0x1);
#endif
            MHal_SC_OP2_SetGammaEnable(FALSE);
            MHal_SC_OP2_SetGammaMappingMode(0);
            MHal_SC_OP2_SetRGBGammaTable(pGammaTableR, pGammaTableG, pGammaTableB, 0, 256);
            MHal_SC_OP2_SetGammaEnable(TRUE);


            ////////////////////////////////////////////////////////////////////
            #if 1 //should be removed while measuring time
            SC_PQ_DBG("2. load remain (1024-256) entries gamma table, ");
            #endif
            memcpy(pGammaTableR, param.pu8Gamma_R, u16TableSize);
            memcpy(pGammaTableG, param.pu8Gamma_G, u16TableSize);
            memcpy(pGammaTableB, param.pu8Gamma_B, u16TableSize);

            // 20091030 daniel.huang: fix 256/1024 gamma incorrect
            MHal_SC_OP2_SetRGBGammaMaxValue(pGammaTableR, pGammaTableG, pGammaTableB, 1024);
            MDrv_SC_IP1_WaitOutputVSync(1, 50);//victor 20080909

			gammaLoadStart=256;
			gammaLoadEnd = gammaLoadStart+GAMMA_LOAD_SIZE;
			while (gammaLoadEnd < 1024)
			{
	            MHal_SC_OP2_SetRGBGammaTable(pGammaTableR, pGammaTableG, pGammaTableB, gammaLoadStart, gammaLoadEnd);
				MDrv_SC_IP1_WaitOutputVSync(1, 50);
				gammaLoadStart = gammaLoadEnd;
				gammaLoadEnd = gammaLoadStart + GAMMA_LOAD_SIZE;
			}
			MHal_SC_OP2_SetRGBGammaTable(pGammaTableR, pGammaTableG, pGammaTableB, gammaLoadStart, 1024);

            ////////////////////////////////////////////////////////////////////
            #if 1 //should be removed while measuring time
            SC_PQ_DBG("3. load 1st 256 entries gamma table, ");
            #endif
            MDrv_SC_IP1_WaitOutputVSync(1, 50);//victor 20080909
            MHal_SC_OP2_SetRGBGammaTable(pGammaTableR, pGammaTableG, pGammaTableB, 0, 256);
            MHal_SC_OP2_SetGammaMappingMode(1);

#ifdef MEASURE_TIME
            u32Timer = ((U32)REG_RR(REG_PIU_TIMER0(0x04))) |
                       ((U32)REG_RR(REG_PIU_TIMER0(0x05)) << 16); // h0005:h0004
            SC_PQ_DBG("1024 gamma table loading time=%u us\n", u32Timer/12);
#endif
        }
        else if (param.u16NumOfLevel == 256)
        {
            memcpy(pGammaTableR, param.pu8Gamma_R, u16TableSize);
            memcpy(pGammaTableG, param.pu8Gamma_G, u16TableSize);
            memcpy(pGammaTableB, param.pu8Gamma_B, u16TableSize);

            // 20091030 daniel.huang: fix 256/1024 gamma incorrect
            MHal_SC_OP2_SetRGBGammaMaxValue(pGammaTableR, pGammaTableG, pGammaTableB, 256);
        MDrv_SC_IP1_WaitOutputVSync(1, 50);//victor 20080909
    #ifdef MEASURE_TIME
            REG_WR(REG_PIU_TIMER0(0x00), 0x0);
            REG_WR(REG_PIU_TIMER0(0x00), 0x1);
    #endif
            MHal_SC_OP2_SetGammaMappingMode((param.u16NumOfLevel == 1024) ? 1: 0);
            //Modified parameter, [090825_Leo]
            //MHal_SC_OP2_SetRGBGammaTable(pGammaTableR, pGammaTableG, pGammaTableB, param.u16NumOfLevel);
            MHal_SC_OP2_SetRGBGammaTable(pGammaTableR, pGammaTableG, pGammaTableB, 0, 256);
            MHal_SC_OP2_SetGammaEnable(TRUE);

    #ifdef  MEASURE_TIME
            u32Timer = ((U32)REG_RR(REG_PIU_TIMER0(0x04))) |
                       ((U32)REG_RR(REG_PIU_TIMER0(0x05)) << 16); // h0005:h0004
            SC_PQ_DBG("256 gamma table loading time=%u us\n", u32Timer/12);
    #endif

        }
    }
    else {
        assert(0);
    }

    if (pGammaTableR)
        kfree(pGammaTableR);
    if (pGammaTableG)
        kfree(pGammaTableG);
    if (pGammaTableB)
        kfree(pGammaTableB);

}

void MDrv_SC_SetFilmMode(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_FILM_MODE_t param;
    PSC_SOURCE_INFO_t pSrcInfo;
    U8 u8Filmtype;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_FILM_MODE_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    if(pSrcInfo->u16Input_VFreq > 550)
    {
        u8Filmtype = 1;
    }
    else
    {
        u8Filmtype = 0;
    }
    MHal_SC_FilmEnable(param.bEnable, u8Filmtype);
}

void MDrv_SC_SetFrameColor(U32 arg)
{
    SC_SET_FRAMECOLOR_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_FRAMECOLOR_t)))
    {
        return;
    }
    MHal_SC_OP2_SetFrameColor(param.u8FrameColorR, param.u8FrameColorG,param.u8FrameColorB);
}

void MDrv_SC_SetCropWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_CROPWIN_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CROPWIN_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

    pSrcInfo->u16H_CropStart_Backup = param.u16H_CropStart;
    pSrcInfo->u16H_CropSize_Backup  = param.u16H_CropSize;
    pSrcInfo->u16V_CropStart = param.u16V_CropStart;
    pSrcInfo->u16V_CropSize  = param.u16V_CropSize;
    pSrcInfo->u16H_CropStart = param.u16H_CropStart;
    pSrcInfo->u16H_CropSize  = param.u16H_CropSize;
    pSrcInfo->u16CropLBoffset= 0;
    pSrcInfo->bCropWin_Enable = TRUE;
    // FitchHsu 20090605 patch Sherwood USA DVD player timing invalid format
#if SC_YPBPR_720x480_60_SW_PATCH
    if (Use_YPbPr_Source(pSrcInfo->SrcType) &&(pSrcInfo->Modesetting.u8ModeIndex == MD_720x480_60I_P))
    {
        pSrcInfo->u8Input_SyncStatus = pSrcInfo->u8Input_SyncStatus | SC_SYNCSTS_INTERLACE_BIT;
    }
#endif
#if 1
	// FitchHsu 20081111 vertical center position mismatch
    // FitchHsu 20081107 vertical center position mismatch
    if (pSrcInfo->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
	{
		if ((pSrcInfo->u16V_CropStart - pSrcInfo->u16V_CapStart) %2)
		{
			pSrcInfo->u16V_CropStart++;
			pSrcInfo->u16V_CropSize = pSrcInfo->u16V_CropSize - 2;
		}
	}
#endif
#ifdef TQM_MODE

	 //FitchHsu 20081015
	// if((pSrcInfo->SrcType == INPUT_SOURCE_DTV) && (pSrcInfo->bHDuplicate == TRUE))
	if(pSrcInfo->bHDuplicate == TRUE )
	{
		 pSrcInfo->u16H_CropSize *= 2;
		 //LGE gbtogether(081120) --> Position shift in DTV by Fitch
		 pSrcInfo->u16H_CropStart = pSrcInfo->u16H_CropStart + (pSrcInfo->u16H_CropStart - pSrcInfo->u16H_CapStart);
	 }
#endif
    SC_ADD_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_CROP_WIN);
}

void MDrv_SC_GetCropWin(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_CROPWIN_t cropWin;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&cropWin, (void __user *)arg, sizeof(SC_GET_CROPWIN_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[cropWin.srcIdx];
    cropWin.u16H_CropStart = pSrcInfo->u16H_CropStart;
    cropWin.u16H_CropSize  = pSrcInfo->u16H_CropSize;
    cropWin.u16V_CropStart = pSrcInfo->u16V_CropStart;
    cropWin.u16V_CropSize  = pSrcInfo->u16V_CropSize;
    if (Use_YPbPr_Source(pSrcInfo->SrcType))
    {
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
            cropWin.u16H_CropStart = pSrcInfo->u16H_CropStart_Backup;
            cropWin.u16H_CropSize  = pSrcInfo->u16H_CropSize_Backup;
        }
        #endif
    }

#ifdef TQM_MODE
    //LGE gbtogether(081015) by FitchHsu
   // if((pSrcInfo->SrcType == INPUT_SOURCE_DTV) && (pSrcInfo->bHDuplicate == TRUE))
    if(pSrcInfo->bHDuplicate == TRUE )
    {
        cropWin.u16H_CropSize /= 2;
		//LGE gbtogether(081120) --> Position shift in DTV by Fitch
		cropWin.u16H_CropStart = pSrcInfo->u16H_CropStart - ((pSrcInfo->u16H_CropStart - pSrcInfo->u16H_CapStart)/2);
    }
#endif

    if (copy_to_user((U32*)arg, (U32*)&cropWin, sizeof(SC_GET_CROPWIN_t)))
    {
        return;
    }
}
void MDrv_SC_SetGOPSEL(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    U8 u8IPSelGOP;
    if (copy_from_user(&u8IPSelGOP, (void __user *)arg, sizeof(U8)))
    {
        return;
    }

    MHal_SC_SetGOPSEL(u8IPSelGOP);
}

//20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MDrv_SC_SaveGOPSetting(void)
{
    MHal_SC_SaveGOPSetting();
}
void MDrv_SC_RestoreGOPSetting(void)
{
    MHal_SC_RestoreGOPSetting();
}

void MDrv_SC_SetGOP_TO_IP(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{

    SC_SET_GOP_TO_IP_t data;

    if (copy_from_user(&data, (void __user *)arg, sizeof(SC_SET_GOP_TO_IP_t)))
    {
        return;
    }

    MHal_SC_GOPINT_SetGOP_TO_IP(data.gop, data.channel, data.enable);
}

void MDrv_SC_SetGOP_TO_VOP(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{

    SC_SET_GOP_TO_VOP_t data;

    if (copy_from_user(&data, (void __user *)arg, sizeof(SC_SET_GOP_TO_VOP_t)))
    {
        return;
    }

    MHal_SC_GOPINT_SetGOP_TO_VOP(data.gop, data.enable);
}
void MDrv_SC_NotifyChangedFmt(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg) // drmyung LGE 080619
{
    PSC_SOURCE_INFO_t pSrcInfo;
    SC_ACTIVE_t sc;

    if (copy_from_user(&sc, (void __user *)arg, sizeof(SC_ACTIVE_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[sc.srcIdx];
    SC_ADD_ACTIVE_FLAG(pSrcInfo, SC_ACTIVE_FLG_FMT_CHG);
}
void MDrv_SC_SetMVDType(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    PSC_SOURCE_INFO_t pSrcInfo;
    SC_SETMVDTYPE_t param;
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SETMVDTYPE_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    pSrcInfo->u16mvdtype = param.u16mvdtype;
}
void MDrv_Scart_GetFBMode(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SCART_MODE_e param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SCART_MODE_e)))
    {
        return;
    }

    param = MHal_SC_GetScartMode();

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_SCART_MODE_e)))
    {
        return;
    }
}

#ifdef	SC_USE_SCART_THREAD

/******************************************************************************/
/// SCART THREAS: check the Aspect Ratio of SCART 1, 2
/// @param -data \b IN: NOT USED
/******************************************************************************/
int MDrv_Scart_Thread(void *data)
{
	while( TRUE )
	{
		_gScartARs[0] = MDrv_Scart_GetAspectRatio(INPUT_SOURCE_SCART_1);
//printk("SC1: %d\n", _gScartARs[0] );
		msleep(4);
#if 0
		_gScartARs[1] = MDrv_Scart_GetAspectRatio(INPUT_SOURCE_SCART_2);
//printk("SC2: %d\n", _gScartARs[1] );
		msleep(4);
#endif		
	}
	return 0;
}
#endif	//#ifdef	SC_USE_SCART_THREAD

// Aspect Ratio
int MDrv_Scart_GetARMode(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_GET_SCART_AR_t param;

#ifdef	SC_USE_SCART_THREAD
	static long _gPidScart = -1;

	if( _gPidScart < 0 )
	{
    	_gPidScart = kernel_thread( MDrv_Scart_Thread, (void*)NULL, CLONE_KERNEL);
		SC_DBG("SCART Thread: %d\n", (uint) _gPidScart );
	}
#endif	//#ifdef	SC_USE_SCART_THREAD

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_GET_SCART_AR_t)))
    {
        return -EFAULT;
    }

#ifdef	SC_USE_SCART_THREAD
	if( param.scartIdx == INPUT_SOURCE_SCART_1 )
	{
	    param.scartAR = _gScartARs[0];
	}
	else if( param.scartIdx == INPUT_SOURCE_SCART_2 )
	{
	    param.scartAR = _gScartARs[1];
	}
	else
	{
        return -EFAULT;
	}
#else	//#ifdef	SC_USE_SCART_THREAD
    param.scartAR = MDrv_Scart_GetAspectRatio(param.scartIdx);
#endif	//#ifdef	SC_USE_SCART_THREAD

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_GET_SCART_AR_t)))
    {
        return -EFAULT;
    }

    return 0;
}

void MDrv_Scart_SetOverlay(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_SCART_OVERLAY_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_SCART_OVERLAY_t)))
    {
        return;
    }

    MHal_SC_SetScartOverlay(param.bOverlay);
}

SC_SCART_AR_e MDrv_Scart_GetAspectRatio(SC_INPUT_SOURCE_e scartIdx)
{
    U8 IDLevel = SCART_ID_LEVEL_0V;

    if (scartIdx == INPUT_SOURCE_SCART_1)
    {
        MHal_SC_GetScart1IDLevel(&IDLevel);
    }
    else if (scartIdx == INPUT_SOURCE_SCART_2)
    {
        //FitchHsu 20080929
        MHal_SC_GetScart2IDLevel(&IDLevel);
    }

    if (MHal_SC_GetScartIDLevelSelect())
    {
        if (IDLevel > 131)      // [132<=Level]     // Level 1B: +9.5V to +12V
        {
            return SCART_AR_4_3;
        }
        else if (IDLevel > 51)  // [52<=Level<=131] // Level 1A: +4.5V to +7V
        {
            return SCART_AR_16_9;
        }
        else                    // [0<=Level<=51]   // Level 0: 0V to 2V
        {
            return SCART_AR_INVALID;
        }
    }
    else
    {
        if (IDLevel > 32)       // [33<=Level]      // Level 1B: +9.5V to +12V
        {
    	    return SCART_AR_4_3;
        }
        else if (IDLevel > 11)  // [12<=Level<=32]  // Level 1A: +4.5V to +7V
        {
            return SCART_AR_16_9;
        }
        else                    // [0<=Level<=11]   // Level 0: 0V to 2V
        {
            return SCART_AR_INVALID;
        }
    }
}

void MDrv_SC_TriggerByVSync(PSC_SOURCE_INFO_t psrc)
{
    U16 u16Sequence;
#if USE_MENULOAD_SCALER
    MHal_SC_ML_Start();
   	MENULOAD_LOCK;
#else
    MDrv_SC_IP1_WaitOutputVSync(1, 50);
#endif
    if (SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_DISP_WIN))
    {
        MHal_SC_VOP_SetDispWin(
            _gDispwin.u16HStart,
            _gDispwin.u16HSize,
            _gDispwin.u16VStart,
            _gDispwin.u16VSize);
    }
    if (SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_FMT_CHG) ||
        SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_CAP_WIN))
    {
    // IP1
        MHal_SC_SetCapWin(
            psrc->u16H_CapStart,
            psrc->u16H_CapSize,
            psrc->u16V_CapStart,
            psrc->u16V_CapSize);
    }
    if (SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_FMT_CHG) ||
        SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_DISP_WIN))
    {
        // IP2
        MHal_SC_IP2_SetHSDRatio(psrc->u32HSDRatio);
        MHal_SC_IP2_SetVSDRatio(psrc->u32VSDRatio);
    }
    if ((psrc->bHMirror || psrc->bVMirror) &&
        (SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_FMT_CHG) ||
         SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_DISP_WIN) ||
         SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_CROP_WIN)))
    {
        // IPM
        MHal_SC_IPM_SetMemBaseAddr(psrc->u32MirrorIPMBase0, psrc->u32MirrorIPMBase1, psrc->u32MirrorIPMBase2);
        MHal_SC_SetVideoMirrorAlignWidth(psrc->u8MirrorAlignWidth);
    }
    else if (SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_FMT_CHG) ||
             SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_DISP_WIN))
    {
        // IPM
        MHal_SC_IPM_SetMemBaseAddr(psrc->u32IPMBase0, psrc->u32IPMBase1, psrc->u32IPMBase2);
    }
    if (SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_FMT_CHG))
    {
        MHal_SC_IPM_SetVLengthWriteLimit(psrc->u16V_CapSize, (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)?TRUE:FALSE);
    }
    if (SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_FMT_CHG) ||
        SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_DISP_WIN))
    {
        MHal_SC_IPM_SetMemFetchOffset(psrc->u16IPMFetch, psrc->u16IPMOffset);
        MHal_SC_SetNumOfFB(psrc->u8NumOfFB);//20091229 Michu, Implement frame buffer mode
    }
    if (SC_CHK_ACTIVE_FLAG(psrc, SC_ACTIVE_FLG_CROP_WIN))
    {
        // OPM
        MHal_SC_OPM_SetMemBaseAddr(psrc->u32OPMBase0, psrc->u32OPMBase1, psrc->u32OPMBase2, psrc->u16CropLBoffset);
        MHal_SC_OPM_SetMemFetchOffset(psrc->u16OPMFetch, psrc->u16OPMOffset);

        // OP1
        MHal_SC_OP1_SetVSP(psrc->u32VSPRatio);
        MHal_SC_OP1_SetHSP(psrc->u32HSPRatio);
        MHal_SC_OP1_SetVLength(psrc->u16V_Length);

        // EODI
        // Only interlace need to set this.
        if((psrc->u16V_CropStart - psrc->u16V_CapStart) % 2 == 0 && psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
        {
            if((psrc->u16V_CropStart - psrc->u16V_CapStart) % 4 == 0)
                u16Sequence = 0x6666;
            else
                u16Sequence = 0x9999;
        }
        else
        {
            u16Sequence = 0x6666;
        }

        //420cup controlled by driver, [100222_Leo]
        if((psrc->SrcType == INPUT_SOURCE_DTV) && (psrc->bMediaPhoto == FALSE) && psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
        {
            MHal_SC_EODI_SetUpSampling(u16Sequence, TRUE);
        }
        else
        {
            MHal_SC_EODI_SetUpSampling(u16Sequence, FALSE);
        }
    }
#if USE_MENULOAD_SCALER
    MHal_SC_ML_End();
   	MENULOAD_UNLOCK;
#endif
}

// 20091202 daniel.huang: use timer for vcount freeze reset
static void MDrv_SC_FreezeVCN(void)
{
    MHal_SC_FreezeVCN();
	//printk("==FreezeVCN===\n");

    // 20091026 daniel.huang: add fast mode patch to fix during toggling vcnt freeze no input signal
#if 0
//    msleep(100);
//    MHal_SC_FreezeVCN_Reset();
#else
    //init_timer(&vcnt_freeze_rst_timer);
    del_timer_sync(&vcnt_freeze_rst_timer);
    vcnt_freeze_rst_timer.function = MHal_SC_ResetFreezeVCN;
    vcnt_freeze_rst_timer.data = 0;
    vcnt_freeze_rst_timer.expires = jiffies + HZ/10;    // 100ms
    add_timer(&vcnt_freeze_rst_timer);
#endif
}

//thchen 20081001
void MDrv_SC_SetFrameLock(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bEnable)
{
    U8   u8LockYLine = 0;
    BOOL bFramelockEnable = DISABLE;
    BOOL bEnFreerunOutput = DISABLE;
    BOOL bSetFPLL = DISABLE;
	//LGE [vivakjh]  2008/12/07	PDP 경우 Format Changed 와 Source Changed 구분하기 위해 추가함.
	PSC_SOURCE_INFO_t pSrcInfo;
	pSrcInfo = &pDrvCtx->SrcInfo[SC_MAIN_WINDOW];

    if (bEnable == FALSE)
    {
        // no signal (freerun mode)
        bEnFreerunOutput = ENABLE;
        MHal_SC_SetFastFrameModeStatus(FALSE);
    }
    else
    {
        // have signal
        bSetFPLL = ENABLE;
        bEnFreerunOutput = ENABLE; // LGE drmyung 081015 : Test 결과 무조건 Enable 되어야 함.
        u8LockYLine = 0x04;  //victor 20080923 merged by totaesun
        bFramelockEnable = ENABLE;
    }

    /* set Y-Line lock */
    MHal_SC_IP1_SetYLock(u8LockYLine);
    /* enable output free-run */
    MHal_SC_SetOutputFreeRun(bEnFreerunOutput);
    /* Enable/Disable FrameLock */
    MHal_SC_IP1_SetFramLock(bFramelockEnable);
	//thchen 20081001 // cc.chen - Fast frame lock
    // FitchHsu 20090524 T3 frame lock fast mode can be used in FRC mdoe
    if (bEnable)//victor 20081210, FRC cannot use fast frame lock
    {
		//20091202 Daniel.Huang: do fast framepll for LCD and LCD TCON
		//lachesis_100115 disable fast fpll S7T cause fliker picture wizard and dvix thumbnail
        if (pDrvCtx->pPanelInfo->u8LCDorPDP == 1 || pDrvCtx->pPanelInfo->u8LCDorPDP == 2 || pDrvCtx->pPanelInfo->u8LCDorPDP == 3)
        {
			// LGE [vivakjh] 2008/12/16 	PDP Only!! PDP Format Change시(PC, HDMI, Component 등)에 화변 껄떡거림 문제 수정.
			if (pDrvCtx->pPanelInfo->u8LCDorPDP == 1)
			{
				// LGE [vivakjh] 2009/01/21	Modified the PDP module flicker problem after playing some perticular movie files that are over the PDP module margin.
				FL_DBG(" [[ vivakjh ]] _gbIsPDPMarginOver = %d \n", _gbIsPDPMarginOver);
				if (_gbIsPDPMarginOver)	 	//if (pSrcInfo->bIsFmtChanged)
					MDrv_SC_FreezeVCN();    // 20091202 daniel.huang: use timer for vcount freeze reset
				else
	        		MHal_SC_FreezeVCN4PDP();
			}
			else
			{
				MHal_SC_FreezeVCN4PDP();
			}
        }
		else
		{
        	MDrv_SC_FreezeVCN();    // 20091202 daniel.huang: use timer for vcount freeze reset
		}
    }

#if 0	//lachesis_090827 need to check again
	//thchen 20090118 temp solution for ATSC frame lock in ATV.
	if((pDrvCtx->pPanelInfo->u8LCDorPDP == 2) && (pSrcInfo->u16Input_VFreq > 550 ) && (pDrvCtx->pPanelInfo->u16Height >=1000))
	{
		if(pSrcInfo->SrcType == INPUT_SOURCE_ATV || pSrcInfo->SrcType == INPUT_SOURCE_DTV)
			MHal_LPLL_LimitD5d6d7(0x20000);
		else
			MHal_LPLL_LimitD5d6d7(pDrvCtx->pPanelInfo->u32LimitD5d6d7);
	}
	// Speed up Frame lock when scaler cant use fast frame lock
	// T3 bring up first disable
#endif

#if 0
	if(bEnable)
	{
        MDrv_SC_SpeedUpFrameLock(pSrcInfo);
    }
#endif
    /* set FPLL */
    MHal_LPLL_EnableFPLL(bSetFPLL);
}

void MDrv_SC_SetMode(PSC_SOURCE_INFO_t psrc)
{
    // cc.chen - FIXME: Review set mode flow


    MDrv_ADC_SetAnalogADC(psrc);

    if (Use_VD_Source(psrc->SrcType))
    {
#if 0 // 20090504 daniel.huang: loaded by scmap
        psrc->MemFormat = SC_MEM_FORMAT_422;
        psrc->MADiMode  = SC_OP1_MADI_3DDI_HISTORY;
#if 0 // LGE drmyung 081008 : move to MDrv_SC_SetVDSigInfo()
        psrc->u16Input_HFreq = MDrv_SC_CalculateHFreqX10(MDrv_SC_IP1_GetHPeriod_VD());
        psrc->u16Input_VTotal = MDrv_SC_IP1_GetVTotal_VD(psrc->u16Input_HFreq);
        psrc->u16Input_VFreq = (MDrv_SC_CalculateVFreqX10(psrc->u16Input_HFreq, psrc->u16Input_VTotal)) * 2;
#endif
#endif
    }
    else if (Use_YPbPr_Source(psrc->SrcType))
    {
        SC_DBG("using ypbpr\n");
        // FitchHsu 20090605 patch Sherwood USA DVD player timing invalid format
#if SC_YPBPR_720x480_60_SW_PATCH
        if (psrc->Modesetting.u8ModeIndex == MD_720x480_60I_P)
        {
            psrc->u8Input_SyncStatus = psrc->u8Input_SyncStatus | SC_SYNCSTS_INTERLACE_BIT;
        }
#endif
        // T3 doesn't need ADC offset calibration
        //MHal_ADC_SetAutoADC(TRUE, TRUE);
#if 0 // 20090504 daniel.huang: loaded by scmap
        psrc->MemFormat = SC_MEM_FORMAT_422;
#endif
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
            SC_DBG("double sampling\n");
            psrc->u16H_CapStart = psrc->u16H_CapStart_Backup << 1;
            psrc->u16H_CapSize = psrc->u16H_CapSize_Backup << 1;
            psrc->u16H_CropStart = psrc->u16H_CropStart_Backup << 1;
            psrc->u16H_CropSize = psrc->u16H_CropSize_Backup << 1;
        }
	#endif

    }
    else if (Use_HDMI_Source(psrc->SrcType))
    {
        if (psrc->bHDMIMode)
        {
            // HDMI
            if (HDMI_COLOR_RGB == MDrv_HDMI_GetPacketColorFormat())
            {
                psrc->u16Input_SigStatus &= ~SC_INPUT_SIG_YUV_DOMAIN;
            }
            else
            {
                psrc->u16Input_SigStatus |= SC_INPUT_SIG_YUV_DOMAIN;
            }
            SC_DBG("========> HDMI\n");
        }
        else
        {
            // DVI
            SC_DBG("========> DVI\n");
            psrc->u16Input_SigStatus &= ~SC_INPUT_SIG_YUV_DOMAIN;
        }
    }
    else
    {
        // cc.chen - T.B.D. - No finish
    }
}

void MDrv_SC_SetCrop(PSC_SOURCE_INFO_t psrc)
{
    U8 u8CropLBoffset = 0;
    U8 u8CropHoffset = 0;

    // leave condition
    if (psrc->bCropWin_Enable == FALSE)
        return;

    if (Use_YPbPr_Source(psrc->SrcType))
    {
        // FitchHsu 20090605 patch Sherwood USA DVD player timing invalid format
        #if SC_DOUBLE_SAMPLING_ENABLE // Disable double sampling requested by LG
        if (
        #if SC_YPBPR_720x480_60_SW_PATCH
            psrc->Modesetting.u8ModeIndex == MD_720x480_60I_P ||
        #endif
            psrc->Modesetting.u8ModeIndex == MD_720x480_60I   ||
            psrc->Modesetting.u8ModeIndex == MD_720x480_60P   ||
            psrc->Modesetting.u8ModeIndex == MD_720x576_50I   ||
            psrc->Modesetting.u8ModeIndex == MD_720x576_50P)
        {
            psrc->u16H_CropStart = psrc->u16H_CropStart_Backup << 1;
            psrc->u16H_CropSize  = psrc->u16H_CropSize_Backup << 1;
        }
        #endif
    }

    psrc->u16H_Cropoffset = psrc->u16H_CropStart - psrc->u16H_CapStart;
    psrc->u16V_Cropoffset = psrc->u16V_CropStart - psrc->u16V_CapStart;
    psrc->u16CropLBoffset = 0;

	// LGE 20081107 lemonic : vertical position is shifted 1 line to top
	// must find another solution.
#if 0
    // FitchHsu 20081026 vertical center position mismatch
    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
    {
        if (psrc->u16V_Cropoffset % 2)
        {
            psrc->u16V_Cropoffset--;
            psrc->u16V_CropSize = psrc->u16V_CropSize + 2;
        }
    }
#endif

    psrc->u16H_CropsizeAfterIP2 = psrc->u16H_CropSize;
    if(psrc->bHDuplicate)
    {
        psrc->u16H_Cropoffset = psrc->u16H_Cropoffset/2 ;
        psrc->u16H_CropsizeAfterIP2 = psrc->u16H_CropsizeAfterIP2/2;
    }
    else
    {
        if (psrc->u16H_CapSize > psrc->u16H_DispSize)
        {
            // Jericho - T.B.D
            psrc->u16H_Cropoffset = (psrc->u16H_Cropoffset * psrc->u16H_DispSize)/psrc->u16H_CapSize;
            // H CropsizeAfterIP2
            psrc->u16H_CropsizeAfterIP2 = (psrc->u16H_CropSize * psrc->u16H_DispSize)/psrc->u16H_CapSize ;
        }
        else
        {
            // H CropsizeAfterIP2
            psrc->u16H_CropsizeAfterIP2 = psrc->u16H_CropSize;
        }
    }


    if (psrc->u16H_Cropoffset %2)
    {
        u8CropHoffset = 1;
    }
    psrc->u16H_CropsizeAfterIP2 = psrc->u16H_CropsizeAfterIP2 - (2*u8CropHoffset);
    psrc->u16H_Cropoffset = ((psrc->u16H_Cropoffset + 1) / 2) * 2; // normalize, must be even

    u8CropLBoffset = psrc->u16H_Cropoffset % OFFSET_PIXEL_ALIGNMENT;
    psrc->u16CropLBoffset = u8CropLBoffset;
    psrc->u16H_Cropoffset = psrc->u16H_Cropoffset / OFFSET_PIXEL_ALIGNMENT;
    psrc->u16CropOPMFetch = psrc->u16H_CropsizeAfterIP2 + u8CropLBoffset;

    // V CropsizeAfterIP2
    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
    {
        psrc->u16V_CropsizeAfterIP2 =(psrc->u16V_CropSize);
        psrc->u16V_Length = psrc->u16V_CropSize;
        psrc->u16V_Cropoffset =psrc->u16V_Cropoffset/2;
    }
    else
    {
        if (psrc->u16V_CapSize > psrc->u16V_DispSize)
        {
            psrc->u16V_Cropoffset = (psrc->u16V_Cropoffset * psrc->u16V_DispSize)/psrc->u16V_CapSize;
            psrc->u16V_CropsizeAfterIP2 = (psrc->u16V_CropSize * psrc->u16V_DispSize)/psrc->u16V_CapSize ;
        }
        else
        {
            psrc->u16V_CropsizeAfterIP2 = psrc->u16V_CropSize;
        }
        psrc->u16V_Length = psrc->u16V_CropsizeAfterIP2;
    }

    psrc->u16H_Cropoffset = psrc->u16H_Cropoffset * OFFSET_PIXEL_ALIGNMENT * psrc->u8BitPerPixel/8; // offset * pixel * byte_per_pixel
    psrc->u16V_Cropoffset = psrc->u16V_Cropoffset * psrc->u8BitPerPixel/8;
    if (!psrc->bLinearAddrMode)   //20091225 daniel.huang: for 3 frame mode crop setting incorrect
    {
        psrc->u16H_Cropoffset *=2;  // data interleave
        psrc->u16V_Cropoffset *=2;  // data interleave
    }

}
void MDrv_Scaler_GenerateBlackVideo(BOOL bEnable)
{
	MHal_SC_OP2_SetBlackScreen(bEnable);    //victor 20080924
}

//------------------------------------------------------------------------------
//  IP1
//------------------------------------------------------------------------------
/******************************************************************************/
///Set scaler field detect for different port
///@param PSC_SOURCE_INFO_t \b IN
///- input source info
/******************************************************************************/
void MDrv_SC_IP1_SetFieldDetect(PSC_SOURCE_INFO_t psrc)
{
    U8  reg_IP1F2_1D, reg_IP1F2_23;
    U16 reg_IP1F2_21;

    if (Use_VD_Source(psrc->SrcType))
    {
        reg_IP1F2_1D = 0xA1;
        reg_IP1F2_21 = 0xC403; //victor 20080923 merged by totaesun
        reg_IP1F2_23 = 0x30;
    }
    else if (Use_DTV_Source(psrc->SrcType))
    {
        if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
        {
            reg_IP1F2_1D = 0xA1;
            reg_IP1F2_21 = 0xC403; //victor 20080923 merged by totaesun
            reg_IP1F2_23 = 0x30;
        }
        else
        {
            reg_IP1F2_1D = 0x21;
            //reg_IP1F2_21 = 0x4400;
//            reg_IP1F2_21 = 0x4401;  // work around for MLINK
            reg_IP1F2_21 = 0xC401;  //victor 20080922, frame tearing and film mode
            reg_IP1F2_23 = 0x00;
        }
    }
    else if (Use_HDMI_Source(psrc->SrcType))
    {
        if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
        {
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0003;  // enable DE -> no field invert, disable DE -> use field invert
            reg_IP1F2_23 = 0x20;
        }
        else
        {
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0000;
            reg_IP1F2_23 = 0x00;
        }
		reg_IP1F2_21 |= BIT12;	//20080903	thchen & LGE jguy : From re-generate DE
    }
    else if (Use_VGA_Source(psrc->SrcType))
    {
        // 20091104 Daniel: remove for fix ATV to VGA 1280x1024 60P sometimes no signal
        if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
        {
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0103;
            reg_IP1F2_23 = 0x20;
        }
        else
        {
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0000;
            reg_IP1F2_23 = 0x00;
        }
    }
    else // YPbPr
    {
        reg_IP1F2_1D = 0xA1;
        reg_IP1F2_21 = 0xC100; //victor 20080923 merged by totaesun
        reg_IP1F2_23 = 0x20;
    }

    MHal_SC_SetFieldDetect(reg_IP1F2_1D, reg_IP1F2_21, reg_IP1F2_23);
}

U8 MDrv_SC_IP1_GetInputSyncStatus(void)
{
    U16 u16HPer, u16VTotal;
    U16 u16DetectStatus;        // mode detect status
    U8  u8SyncStatus = 0x00;    // sync status

    // H Sync
    u16HPer = MHal_SC_IP1_GetHPeriod();
    if (u16HPer < 10)
    {
        u8SyncStatus |= SC_SYNCSTS_HSYNC_LOSS_BIT;
    }

    // V Sync
    u16VTotal = MHal_SC_IP1_GetVTotal();
    if (u16VTotal < 200)
    {
        u8SyncStatus |= SC_SYNCSTS_VSYNC_LOSS_BIT;
    }

    // sync status
    u16DetectStatus = MHal_SC_IP1_GetDetectSyncStatus();
    if (u16DetectStatus & BIT8)
    {
        u8SyncStatus |= SC_SYNCSTS_VSYNC_POL_BIT;
    }
    if (u16DetectStatus & BIT9)
    {
        u8SyncStatus |= SC_SYNCSTS_HSYNC_POL_BIT;
    }
    if (u16DetectStatus & BIT11)
    {
        u8SyncStatus |= SC_SYNCSTS_INTERLACE_BIT;
    }

    return u8SyncStatus;
}

BOOL MDrv_SC_IP1_WaitInputVSync(U8 u8VSyncCnt, U16 u16Timeout)
{
    U8  u8SyncSts;
    U8  u8VSync = !((MHal_SC_IP1_GetDetectSyncStatus() & BIT8) ? 1:0); //polarity
    U32 u32Time;

    u32Time = jiffies;

    while (1)
    {
        u8SyncSts = (MHal_SC_IP1_GetDetectSyncStatus() & BIT2) ? 1:0;

        if (u8SyncSts == u8VSync)
        {
            if (u8VSync && --u8VSyncCnt == 0)
                return TRUE;

            u8VSync = !u8VSync;
        }

        if ((jiffies - u32Time) >= u16Timeout)
            return FALSE;
    }
}

BOOL MDrv_SC_IP1_WaitOutputVSync(U8 u8VSyncCnt, U16 u16Timeout)
{
    U16 u16SyncSts;
    U8  u8VSync = 0;
    U32 u32Time;

    u32Time = jiffies;

    while (1)
    {
        u16SyncSts = MHal_SC_IP1_GetDetectSyncStatus();

        if ((u16SyncSts & BIT0) == u8VSync)
        {
            if (u8VSync && --u8VSyncCnt == 0)
                return TRUE;

            u8VSync = !u8VSync;
        }

        if ((jiffies - u32Time) >= u16Timeout)
            return FALSE;
    }
}

//------------------------------------------------------------------------------
//  IP2
//------------------------------------------------------------------------------


void MDrv_SC_IP2_SetCSCEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_CSC_ENABLE_t param;
    PSC_SOURCE_INFO_t pSrcInfo;
    BOOL bUseVIPCSC;
    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CSC_ENABLE_t)))
    {
        return;
    }
    // 20090828 daniel.huang: add VIP CSC for PCRGB & DVI
    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    if((Use_VGA_Source(pSrcInfo->SrcType)) ||
      ((Use_HDMI_Source(pSrcInfo->SrcType) && (pSrcInfo->bHDMIMode == FALSE) && (!(pSrcInfo->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)))))
    {
        bUseVIPCSC = TRUE;
    }
    else
    {
        bUseVIPCSC = FALSE;
    }
    MHal_SC_IP2_SetCSC(param.bEnable, bUseVIPCSC);
}//victor 20080909

//20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MDrv_SC_VIP_GetCSCEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    BOOL bEnable;
    bEnable = MHal_SC_VIP_GetCSC();
    copy_to_user((U32*)arg, (BOOL*)&bEnable, sizeof(BOOL));
}


void MDrv_SC_SetColorAdaptiveRange(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_COLOR_RANGE_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_COLOR_RANGE_t)))
    {
        return;
    }
    MHal_SC_SetColorAdaptiveRange(param.u8CbUpValue, param.u8CbDownValue, param.u8CrUpValue, param.u8CrDownValue);
}//[090601_Leo]

void MDrv_SC_SetAdaptiveCGainEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_ADAPTIVE_CGAIN_EN_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_ADAPTIVE_CGAIN_EN_t)))
    {
        return;
    }

    MHal_SC_SetAdaptiveCGainEnable(param.bOnOff);

}//[090921_Leo]

void MDrv_SC_SetAdaptiveCGain(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_ADAPTIVE_CGAIN_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_ADAPTIVE_CGAIN_t)))
    {
        return;
    }

    MHal_SC_SetAdaptiveCGain(param.pCGainParam);

}//[090814_Leo]

void MDrv_SC_SetPieceWiseEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_PIECEWISE_ENABLE_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_PIECEWISE_ENABLE_t)))
    {
        return;
    }

    MHal_SC_SetPieceWiseEnable(param.u8OnOff, param.pPieceWiseXPosition);

}//[090825_Leo]

/******************************************************************************/
/// Calculate horizontal scaling down ratio
/// @param -psrc \b IN: input source information
/******************************************************************************/
void MDrv_SC_IP2_HScaling(PSC_SOURCE_INFO_t psrc)
{
    BOOL bAdvMode;
    if(psrc->bHDuplicate)
    {
        psrc->u16H_sizeAfterIP2 = psrc->u16H_CapSize/2;
    }
    else
    {
        if (psrc->u16H_CapSize > psrc->u16H_DispSize)
        {
            psrc->u16H_sizeAfterIP2 = psrc->u16H_DispSize;
        }
        else
        {
            psrc->u16H_sizeAfterIP2 = psrc->u16H_CapSize;
        }
    }
    if (psrc->u16H_sizeAfterIP2 > SC_MAX_LINE_BUF)
    {
        psrc->u16H_sizeAfterIP2 = SC_MAX_LINE_BUF;
    }

    bAdvMode = MDrv_SCMAP_LoadScalingTable(SC_MAIN_WINDOW,
                                        E_XRULE_HSD,
                                        FALSE,
                                       (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)?TRUE:FALSE,
                                       (psrc->u16Input_SigStatus & SC_INPUT_SIG_YUV_DOMAIN) ?TRUE:FALSE,
                                        psrc->u16H_CapSize,
                                        psrc->u16H_sizeAfterIP2);

    psrc->u32HSDRatio = MHal_SC_IP2_CalHSD(psrc->u16H_CapSize, psrc->u16H_sizeAfterIP2, bAdvMode);

}

/******************************************************************************/
/// Calculate vertical scaling down ratio
/// @param -psrc \b IN: input source information
/******************************************************************************/
void MDrv_SC_IP2_VScaling(PSC_SOURCE_INFO_t psrc)
{
    if (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)
    {
        psrc->u32VSDRatio = 0;
        psrc->u16V_sizeAfterIP2 = psrc->u16V_CapSize;
        psrc->u16V_Length = psrc->u16V_CapSize;
    }
    else
    {
        // video larger than screen size
        if (psrc->u16V_CapSize > psrc->u16V_DispSize)
        {
            psrc->u16V_sizeAfterIP2 = psrc->u16V_DispSize;
        }
        else
        {
            psrc->u16V_sizeAfterIP2 = psrc->u16V_CapSize;
        }

        psrc->u32VSDRatio = MHal_SC_IP2_CalVSD(psrc->u16V_CapSize, psrc->u16V_sizeAfterIP2);
        psrc->u16V_Length = psrc->u16V_sizeAfterIP2;
    }

    if (psrc->u16V_CapSize != psrc->u16V_sizeAfterIP2) {
        psrc->bPreV_ScalingDown = TRUE;
    }
    else {
        psrc->bPreV_ScalingDown = FALSE;
    }

    // cc.chen - T.B.D - Chiptop setting
    if (psrc->u32VSDRatio == 0)
    {
        MHal_TOP_SetFickF2_ClkSrc(FALSE);
    }
    else
    {
        MHal_TOP_SetFickF2_ClkSrc(TRUE);
    }

    // 20090618 daniel.huang: add pre-V scaling down filter select
    MDrv_SCMAP_LoadScalingTable(SC_MAIN_WINDOW,
                                E_XRULE_VSD,
                                psrc->bPreV_ScalingDown,
                               (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)?TRUE:FALSE,
                               (psrc->u16Input_SigStatus & SC_INPUT_SIG_YUV_DOMAIN) ?TRUE:FALSE,
                                psrc->u16V_CapSize,
                                psrc->u16V_sizeAfterIP2);

}

void MDrv_SC_IPM_CalMemSetting(PSC_SOURCE_INFO_t psrc, U32 u32MemAddr, U32 u32MemLen)
{
    psrc->u32IPMBase0  = u32MemAddr;
    if (psrc->u8NumOfFB == 3)
    {
        psrc->u32IPMBase1  = u32MemAddr + (u32MemLen / 3)/BYTE_PER_WORD*BYTE_PER_WORD;
        psrc->u32IPMBase2  = u32MemAddr + (u32MemLen / 3)/BYTE_PER_WORD*BYTE_PER_WORD*2;
    }
    else
    {
        psrc->u32IPMBase1  = u32MemAddr + (u32MemLen / 2);
        psrc->u32IPMBase2  = 0;
    }
    psrc->u16IPMFetch  = (psrc->u16H_sizeAfterIP2 + 1) & ~1;
    //printk("IPMFetch:%u\n", psrc->u16IPMFetch);

    // 20090410 daniel.huang
    // T3: base address 16 byte alignment; according to designer's suggest,
    // offset to be 32 pixel aignment for all case(444 10bit/8bit, 422 10bit/8bit)
    psrc->u16IPMOffset = (psrc->u16IPMFetch + OFFSET_PIXEL_ALIGNMENT-1)/
                         OFFSET_PIXEL_ALIGNMENT * OFFSET_PIXEL_ALIGNMENT;

    //------------------------------------------------------------------------------
    // VIDEO MIRROR, Align width
    // Michu 20090924
    //------------------------------------------------------------------------------
    if ( psrc->bHMirror )
    {
        psrc->u16IPMFetch = ((psrc->u16IPMFetch + OFFSET_PIXEL_ALIGNMENT-1)/
                                     OFFSET_PIXEL_ALIGNMENT) * OFFSET_PIXEL_ALIGNMENT;
        psrc->u8MirrorAlignWidth = (U8)(psrc->u16IPMFetch - ((psrc->u16H_sizeAfterIP2 + 1) & ~1));

        SC_MIRROR_DBG("==========> psrc->u16IPMOffset = %d\n", psrc->u16IPMOffset);
        SC_MIRROR_DBG("==========> psrc->u16IPMFetch = %d\n", psrc->u16IPMFetch);
        SC_MIRROR_DBG("==========> psrc->u16H_sizeAfterIP2 = %d\n", psrc->u16H_sizeAfterIP2);
        SC_MIRROR_DBG("==========> Align width = %d\n", psrc->u8MirrorAlignWidth);

        if( psrc->u8MirrorAlignWidth )
        {
            psrc->u8MirrorAlignWidth = psrc->u8MirrorAlignWidth | 0x80;
            SC_MIRROR_DBG("==========> psrc->u8MirrorAlignWidth = %d\n", psrc->u8MirrorAlignWidth);
        }
        else
        {
            psrc->u8MirrorAlignWidth = 0;
        }
    }
    // End of align width

    //------------------------------------------------------------------------------
    // VIDEO MIRROR
    // Michu 20090903
    //------------------------------------------------------------------------------
    if ( psrc->bVMirror )
    {
        SC_MIRROR_DBG("====================> Video Mirror\n");
        SC_MIRROR_DBG("**************************************************\n");
        SC_MIRROR_DBG("* MDrv_SC_IPM_CalMemSetting\n");
        SC_MIRROR_DBG("**************************************************\n");
        SC_MIRROR_DBG("==========> psrc->u8NumOfFB = %d\n",(U8)psrc->u8NumOfFB);
        SC_MIRROR_DBG("==========> O_u32MemAddr = 0x%x\n", (U32)u32MemAddr);
        SC_MIRROR_DBG("==========> O_u32MemLen = 0x%x\n", (U32)u32MemLen);
        SC_MIRROR_DBG("==========> O_u32MemAddr+u32MemLen = 0x%x\n", (U32)(u32MemAddr+u32MemLen));
        SC_MIRROR_DBG("==========> psrc->u8BitPerPixel= %d\n", (U8)psrc->u8BitPerPixel);
        SC_MIRROR_DBG("==========> psrc->u8Input_SyncStatus = %d\n",(U8)psrc->u8Input_SyncStatus);
        SC_MIRROR_DBG("==========> O_psrc->u16V_sizeAfterIP2= %d\n", (U16)psrc->u16V_sizeAfterIP2);
        SC_MIRROR_DBG("==========> O_psrc->u16IPMOffset= %d\n", (U16)psrc->u16IPMOffset);

        //base_offset = (frame_line_cnt -2) * line_offset * (N-bits/pix)/128-bits

        if( psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT )
        {
            psrc->u32MirrorBaseOffset = (U32)(psrc->u16V_sizeAfterIP2-2) * (U32)psrc->u16IPMOffset * (U32)psrc->u8BitPerPixel/128;
        }
        else if (psrc->bLinearAddrMode) //20091225 daniel.huang: for 3 frame mode crop setting incorrect
        {
            psrc->u32MirrorBaseOffset = (U32)(psrc->u16V_sizeAfterIP2-1) * (U32)psrc->u16IPMOffset * (U32)psrc->u8BitPerPixel/128;
        }
        else
        {
            psrc->u32MirrorBaseOffset = (U32)(2*psrc->u16V_sizeAfterIP2-2) * (U32)psrc->u16IPMOffset * (U32)psrc->u8BitPerPixel/128;
        }

        SC_MIRROR_DBG("psrc->u32MirrorBaseOffset= 0x%x \n", (U32)psrc->u32MirrorBaseOffset);
        SC_MIRROR_DBG("psrc->u16V_sizeAfterIP2= %d\n", (U16)psrc->u16V_sizeAfterIP2);
        SC_MIRROR_DBG("psrc->u16IPMOffset= %d\n", (U16)psrc->u16IPMOffset);

        SC_MIRROR_DBG("==========> O_psrc->u32IPMBase0= 0x%x\n", (U32)psrc->u32IPMBase0);
        SC_MIRROR_DBG("==========> O_psrc->u32IPMBase1= 0x%x\n", (U32)psrc->u32IPMBase1);

        psrc->u32MirrorBaseOffset = psrc->u32MirrorBaseOffset*BYTE_PER_WORD;
        SC_MIRROR_DBG("psrc->u32MirrorBaseOffset*0x10= 0x%x \n", (U32)psrc->u32MirrorBaseOffset);

        psrc->u32MirrorIPMBase0 = psrc->u32IPMBase0 + psrc->u32MirrorBaseOffset;
        psrc->u32MirrorIPMBase1 = psrc->u32IPMBase1 + psrc->u32MirrorBaseOffset;
        if (psrc->u8NumOfFB == 3)
        {
            psrc->u32MirrorIPMBase2 = psrc->u32IPMBase2 + psrc->u32MirrorBaseOffset;
        }
        SC_MIRROR_DBG("==========> psrc->u32MirrorIPMBase0= 0x%x\n", (U32)psrc->u32MirrorIPMBase0);
        SC_MIRROR_DBG("==========> psrc->u32MirrorIPMBase1= 0x%x\n", (U32)psrc->u32MirrorIPMBase1);
        SC_MIRROR_DBG("==========> psrc->u32MirrorIPMBase2= 0x%x\n", (U32)psrc->u32MirrorIPMBase2);
    }
    //------------------------------------------------------------------------------
    // End of VIDEO MIRROR
    //------------------------------------------------------------------------------

}

// LGE [vivakjh]  2008/12/11	PDP 만  Freeze 기능 대응됨.  Film On 시에 Freeze 시 화면 떨림 수정함.
void MDrv_SC_IPM_SetFreezeImg(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bFreeze)
{
	if (bFreeze)
	{
		_gFreezeFilmMode = MHal_SC_GetFilmMode();
		if (_gFreezeFilmMode > 0)
		{
			 _bIsFilmInFreeze = TRUE;
			 MHal_SC_FilmEnable(FALSE, _gFreezeFilmMode-1);
		}
		MHal_SC_IPM_SetFreezeImg(bFreeze);
	}
	else
	{
		MHal_SC_IPM_SetFreezeImg(bFreeze);
		if (_bIsFilmInFreeze)
		{
			_bIsFilmInFreeze = FALSE;
			MHal_SC_FilmEnable(TRUE, _gFreezeFilmMode-1);
		}
	}
}

//------------------------------------------------------------------------------
//  OPM
//------------------------------------------------------------------------------
void MDrv_SC_OPM_CalMemSetting(PSC_SOURCE_INFO_t psrc)// Michu 20090903
{
    if (psrc->bCropWin_Enable == TRUE)
    {
        psrc->u32OPMBase0 = psrc->u32IPMBase0 + psrc->u16IPMOffset * psrc->u16V_Cropoffset + psrc->u16H_Cropoffset ;
        psrc->u32OPMBase1 = psrc->u32IPMBase1 + psrc->u16IPMOffset * psrc->u16V_Cropoffset + psrc->u16H_Cropoffset ;
        psrc->u32OPMBase2 = psrc->u32IPMBase2 + psrc->u16IPMOffset * psrc->u16V_Cropoffset + psrc->u16H_Cropoffset ;

        if (psrc->bHMirror)
        {
            psrc->u16OPMFetch = (psrc->u16H_sizeAfterIP2+ 1) & ~1;
        }
        else
        {
    		if (psrc->u16H_CropSize > psrc->u16H_sizeAfterIP2)	//thchen 20080816
            {
                psrc->u16OPMFetch = (psrc->u16H_sizeAfterIP2+ 1) & ~1;
            }
            else
            {
    			// for cropping 32 pixel align(T3) + opm fetch 2 pixel align
    			psrc->u16OPMFetch = (psrc->u16H_CropSize + OFFSET_PIXEL_ALIGNMENT) & ~0x1;
            }

            if (psrc->u16OPMFetch > psrc->u16IPMFetch)
                psrc->u16OPMFetch = psrc->u16IPMFetch;
        }
    }
    else
    {
        psrc->u32OPMBase0 = psrc->u32IPMBase0;
        psrc->u32OPMBase1 = psrc->u32IPMBase1;
        psrc->u32OPMBase2 = psrc->u32IPMBase2;
        psrc->u16OPMFetch = psrc->u16IPMFetch;
        if (psrc->bHMirror)
            psrc->u16OPMFetch = (psrc->u16H_sizeAfterIP2+ 1) & ~1;
    }
    psrc->u16OPMOffset = psrc->u16IPMOffset;
}

//------------------------------------------------------------------------------
//  OP1
//------------------------------------------------------------------------------
void MDrv_SC_OP1_CalVScaling(PSC_SOURCE_INFO_t psrc)
{
    U16 u16VSize;
    if (psrc->bCropWin_Enable == TRUE)
    {
        u16VSize = psrc->u16V_CropsizeAfterIP2;
    }
    else
    {
        u16VSize = psrc->u16V_sizeAfterIP2;
    }

    psrc->u32VSPRatio = MHal_SC_OP1_CalVSP(u16VSize, psrc->u16V_DispSize);
    MDrv_SCMAP_LoadScalingTable(SC_MAIN_WINDOW,
                             E_XRULE_VSP,
                             psrc->bPreV_ScalingDown,
                            (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)?TRUE:FALSE,
                            (psrc->u16Input_SigStatus & SC_INPUT_SIG_YUV_DOMAIN) ?TRUE:FALSE,
                             u16VSize,
                             psrc->u16V_DispSize);
}

void MDrv_SC_OP1_CalHScaling(PSC_SOURCE_INFO_t psrc)
{
    U16 u16HSize;
    if (psrc->bCropWin_Enable == TRUE)
    {
        u16HSize = psrc->u16H_CropsizeAfterIP2;
    }
    else
    {
        u16HSize = psrc->u16H_sizeAfterIP2;
    }

    psrc->u32HSPRatio = MHal_SC_OP1_CalHSP(u16HSize, psrc->u16H_DispSize);
    MDrv_SCMAP_LoadScalingTable(SC_MAIN_WINDOW,
                             E_XRULE_HSP,
                             psrc->bPreV_ScalingDown,
                            (psrc->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)?TRUE:FALSE,
                            (psrc->u16Input_SigStatus & SC_INPUT_SIG_YUV_DOMAIN) ?TRUE:FALSE,
                             u16HSize,
                             psrc->u16H_DispSize);
}

//------------------------------------------------------------------------------
//  OP2
//------------------------------------------------------------------------------
void MDrv_SC_OP2_SetCSC(PSC_SOURCE_INFO_t psrc)
{
    // cc.chen - T.B.D. - Consider YUV or RGB form PC
    MHal_SC_OP2_SetCMC(TRUE);
}

//-----------------------------------------------------------------------------
// IP MUX
//-----------------------------------------------------------------------------
void MDrv_SC_SetIPMux(SC_INPUT_SOURCE_e source)
{
    U8 u8Clk_Mux  = SC_IPMUX_ADC_A;
    U8 u8Data_Mux = SC_IPMUX_ADC_A;

    if (Use_Analog_Source(source))
    {
        u8Clk_Mux  = SC_IPMUX_ADC_A;
        u8Data_Mux = SC_IPMUX_ADC_A;
    }
    else if (Use_HDMI_Source(source))
    {
        u8Clk_Mux = u8Data_Mux = SC_IPMUX_HDMI_DVI;
    }
    else if (Use_CVBS_Source(source) || Use_SV_Source(source) || Use_SCART_Source(source))
    {
        u8Clk_Mux  = SC_IPMUX_VD;
        u8Data_Mux = SC_IPMUX_VD;
    }
    else if (Use_DTV_Source(source))
    {
        u8Clk_Mux = SC_IPMUX_MVOP;

        if (source == INPUT_SOURCE_DTV)
        {
            u8Data_Mux = SC_IPMUX_MVOP;
        }
        else if (source == INPUT_SOURCE_DTV_MLINK)
        {
            u8Data_Mux = SC_IPMUX_MVOP_MLINK;
        }
    }
    else
    {
        u8Clk_Mux = u8Data_Mux = SC_IPMUX_EXT_VD;
    }

    // 20091026 Daniel.Huang: switch idclk2 clock to XTAL to clear ip status
    MHal_SC_IPMuxSet(u8Data_Mux, u8Clk_Mux);
    //MHal_SC_Reset(SC_RST_IP_F2);
}

//--------------------------------------------------------------------------------------------------
// MOD
//--------------------------------------------------------------------------------------------------
void MDrv_SC_SetMODPower(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL bEnable)
{
    if (bEnable)
    {
        MHal_LPLL_MODSET(pDrvCtx->pPanelInfo->u8LPLL_Type, pDrvCtx->pPanelInfo->u8LPLL_Mode);
        // FitchHsu 20080811 implement LPLL type
		MHal_MOD_SetPower(TRUE, pDrvCtx->pPanelInfo->u8LPLL_Type); // FitchHsu 20080811 implement LPLL type

        //MHal_MOD_SetOEZ(MOD_OEZ_ALL);
        MHal_MOD_Init(pDrvCtx->pPanelInfo);

    }
    else
    {
        MHal_MOD_SetPower(FALSE, pDrvCtx->pPanelInfo->u8LPLL_Type); // FitchHsu 20080811 implement LPLL type

        // FitchHsu 20080811 implement LPLL type
        //MHal_MOD_SetOEZ(MOD_OEZ_ALL);
        MHal_MOD_SelTTL(0xFFF);
    }
}

//================================================================//
///Calculate Sync & pixel clk, freq
//================================================================//

/***********************************************/
///This function will calculate and return H Frequency x 10
///@param wHPeriod \b IN
///- Horizontal period
///@return
///- U16 H Frequency x 10
/***********************************************/
U16 MDrv_SC_CalculateHFreqX10(U16 wHPeriod)
{
    if (wHPeriod)
        return ( (((U32)MST_XTAL_CLOCK_HZ + (wHPeriod/2)) / wHPeriod ) / 100 ); //KHz
    else
        return 1;   // avoid devide by 0 error
}

/***********************************************/
///This function will calculate and return V Frequency x 10
///@param wHFreq \b IN
///- Horizontal Frequency
///@param wVTotal \b IN
///- Vertical Frequency
///@return
///- U16 V Frequency x 10
/***********************************************/
U16 MDrv_SC_CalculateVFreqX10(U16 wHFreq, U16 wVTotal)
{
    if ( (wHFreq > 0) && (wVTotal > 0))	// 20080718 swwoo LGE
        return ( (((U32)wHFreq * 1000 ) + (wVTotal/2)) / wVTotal ); // Hz
    else
        return 0;
}

/***********************************************/
///This function will calculate and return Vsync time
///@return
///- U8 VSync time
/***********************************************/
U8 MDrv_SC_CalculateVSyncTime(U16 u16VTotal, U16 u16HPeriod)
{
    U16 wHFreqX10; // horizontal frequency

    wHFreqX10 = MDrv_SC_CalculateHFreqX10(u16HPeriod); // calculate HSync frequency
    return (U8)(((u16VTotal*10)+(wHFreqX10/2))/wHFreqX10);
}

/***********************************************/
///Set line buffer clock
///@param u16HTotal \b IN
///- Horizontal total
///@return
///- Output Pixel colok
/***********************************************/
U8 MDrv_SC_CalculatePixClk(U16 u16HTotal)
{
    return (U8)
    (((U32)MDrv_SC_CalculateHFreqX10(MHal_SC_IP1_GetHPeriod())*u16HTotal+5000)/10000);
}

//------------------------------------------------------------------------------
//  Others
//------------------------------------------------------------------------------
void MDrv_SC_WakeupEvent(PSC_SOURCE_INFO_t psrc, U32 u32Event)
{
    atomic_set(&psrc->ThreadData.wait_event_sts, u32Event);

	#ifdef FAST_THREAD_CONTROL
	wake_up(&psrc->ThreadData.thread_wq);
	#else
    wake_up(&psrc->ThreadData.wait_event_wq);
	#endif
}

//------------------------------------------------------------------------------
//  Utility
//------------------------------------------------------------------------------
U16 MDrv_Calculate_ABS(U16 num1, U16 num2)
{
    U16 result;
    if (num1 >= num2)
        result = num1-num2;
    else
        result = num2-num1;

    return result;
}

U16 MDrv_SC_SubtractABS(U16 num1, U16 num2)
{
    U16 result;
    if (num1 >= num2)
        result = num1-num2;
    else
        result = num2-num1;

    return result;
}

void MDrv_TimingDataInit(PSC_SOURCE_INFO_t psrc)
{
	psrc->bIsSupportMode = 0;
    psrc->u16Input_HFreq = 0;
    psrc->u16Input_VFreq = 0;
    psrc->u16Input_HPeriod = 0;
    psrc->u16Input_VTotal = 0;
    psrc->u16Input_HDE = 0;
    psrc->u16Input_VDE = 0;
    psrc->u16Input_HDE_Start = 0;
    psrc->u16Input_VDE_Start = 0;
    psrc->u16Input_HActive = 0;
    psrc->u16Input_VActive = 0;
    psrc->u16H_CapStart = 0;
    psrc->u16V_CapStart = 0;
    psrc->u16H_CapSize = 0;
    psrc->u16V_CapSize = 0;
    psrc->u16H_CapStart_Backup = 0;
    psrc->u16H_CapSize_Backup = 0;
    psrc->u8ModeIdx = 0xFF;	// 20080821 swwoo LGE 신호 제거시 No Signal 표시 안되는 문제 수정.

#if 1 // drmyung LGE 080722
    psrc->u16Input_HDMI_HPeriod  = 0;
    psrc->u16Input_HDMI_VTotal   = 0;
    psrc->u16Input_HDMI_HDE      = 0;
    psrc->u16Input_HDMI_VDE      = 0;
    psrc->u16Input_HDMI_HDE_Start = 0;
    psrc->u16Input_HDMI_VDE_Start = 0;
    psrc->u16Input_HDMI_HFreq = 0;
    psrc->u16Input_HDMI_VFreq = 0;
    psrc->u8Input_HDMI_SyncStatus = 0;

    psrc->u16Input_SC_HFreq = 0;
    psrc->u16Input_SC_VFreq = 0;
    psrc->u16Input_SC_HPeriod  = 0;
    psrc->u16Input_SC_HStart = 0;
    psrc->u16Input_SC_VStart = 0;
    psrc->u16Input_SC_VTotal   = 0;
    psrc->u16H_SC_CapSize      = 0;
    psrc->u16V_SC_CapSize      = 0;
    psrc->u8Input_SC_SyncStatus = 0;
    psrc->u8Input_SC_Phase = 0;
#endif
    // set crop window
    psrc->u16H_CropStart = 0;
    psrc->u16H_CropSize = 0;
    psrc->u16H_CropStart_Backup = 0;
    psrc->u16H_CropSize_Backup = 0;
    psrc->u16H_Cropoffset       = 0;
    psrc->u16H_CropsizeAfterIP2 = 0;
    psrc->u16V_CropStart = 0;
    psrc->u16V_CropSize = 0;
    psrc->u16V_Cropoffset       = 0;
    psrc->u16V_CropsizeAfterIP2 = 0;
    psrc->bCropWin_Enable       = FALSE;
}

#ifdef	SC_USE_ONLY_ONE_THREAD
int MDrv_SC_Thread(void *data)
{
extern int MDrv_SC_PCMode_Thread( void *data );
extern int MDrv_HDMI_Thread(void *data);

    PSC_SOURCE_INFO_t psrc = (PSC_SOURCE_INFO_t)data;
    PSC_THREAD_DATA_t pThreadData = &(psrc->ThreadData);
#ifdef FAST_THREAD_CONTROL
    U32 u32Timeout = HZ/5 ;
#else
    U32 u32Timeout = 1 * HZ ;
#endif
    S32 s32Ret;

/*	2008,10,13:	외부입력 전환 Aging Test시 System pending되는 문제(dreamer@lge.com)

	-> Aging 후, COMPONENT/RGB/HDMI에서 "NO SIGNAL" 표시하는 문제 수정
	(PC MODE의 KERNEL TASK에서 Wait Queue 처리가 잘못되어 있음)
*/

#ifndef FAST_THREAD_CONTROL
    init_waitqueue_head(&pThreadData->thread_wq);
#endif

#ifdef FAST_THREAD_CONTROL
	psrc->u8ThreadCtrl= 1;
#endif

//printk(KERN_INFO"xInit\n");
    while (1)
    {
//        printk(".");

#ifdef FAST_THREAD_CONTROL
	 s32Ret = wait_event_interruptible_timeout(pThreadData->thread_wq, psrc->u8ThreadCtrl, u32Timeout);
#else
        s32Ret = wait_event_interruptible_timeout(pThreadData->thread_wq, 0, u32Timeout);
#endif

        if (s32Ret == -ERESTARTSYS)
        {
            continue;
        }

//printk(KERN_INFO"xT%d(%d %d)\n", u32Timeout, s32Ret, ERESTARTSYS);
		if( psrc->u8ThreadMode == 1 )
		{
//printk(KERN_INFO"x pc x");
			MDrv_SC_PCMode_Thread( psrc );
		}
		else if( psrc->u8ThreadMode == 2  )
		{
//printk(KERN_INFO"x hd x");
			MDrv_HDMI_Thread( psrc );
		}
	}

	return 0;
}
#endif	/*#ifdef	SC_USE_ONLY_ONE_THREAD */

#if 0 //080912 LGE drmyung
void MDrv_LPLL_Thread(void *data)//thchen 20080904 //080912 LGE drmyung
{
    PSC_SOURCE_INFO_t psrc = (PSC_SOURCE_INFO_t)data;

    PSC_THREAD_DATA_t pThreadData = &(psrc->ThreadData);
    U32 u32Timeout;
    S32 s32Ret;
    U8  i = 0;

    init_waitqueue_head(&pThreadData->thread_wq);
    u32Timeout = HZ/60;

    while (1)
    {
        s32Ret = wait_event_interruptible_timeout(pThreadData->thread_wq, pThreadData->u16Ctrl != 0, u32Timeout);

        if (s32Ret == -ERESTARTSYS)
        {
            break;
        }

        //printk("thchen Phase Dif Value=%d \n",MHal_LPLL_GetPhaseDifValue());

        if (MHal_LPLL_GetPhaseDifValue() <2)
        {
            i=i+1;
            if (i>4)
            {
                //if (i ==10)
                //    break;
                printk("\n thchen frame locked time=%d \n",jiffies);
                break;
                //printk("\n %d \n",MHal_LPLL_GetPhaseDifValue());
            }
        }
        else
            i=0;
    }
    SC_SET_SRC_STATE(psrc, SC_SRC_STATE_LPLL_STOP);
}

void MDrv_SC_LPLL_Start(PSC_SOURCE_INFO_t psrc)//thchen 20080904 //080912 LGE drmyung
{
    kernel_thread((int (*)(void*))MDrv_LPLL_Thread, (void*)psrc, CLONE_KERNEL);
}

void MDrv_SC_LPLL_Stop(PSC_SOURCE_INFO_t psrc) //thchen 20080904 //080912 LGE drmyung
{
    psrc->ThreadData.u16Ctrl = SC_THREAD_CTRL_TERMINATE;
    while (!(SC_GET_SRC_STATE(psrc) & SC_SRC_STATE_LPLL_STOP))
    {
        schedule();
    }
    SC_CLR_SRC_STATE(psrc, SC_SRC_STATE_LPLL_STOP);
}
#endif
//victor 20080830
void MDrv_SC_Set_Blue_Stretch_Enable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_BLUE_STRETCH_ENABLE_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_BLUE_STRETCH_ENABLE_t)))
    {
        return;
    }
    MHal_SC_MACE_SetBlueStretchEnable(param.bEnable);
}

//victor 20080830
void MDrv_SC_Set_CSC_Offset_Enable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_CSC_OFFSET_ENABLE_t param;
    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CSC_OFFSET_ENABLE_t)))
    {
        return;
    }
    MHal_SC_SetCSCOffset(param.bEnable);
}

//victor 20080923
void MDrv_IS_HDMI(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_IS_HDMI_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

//    SC_PQ_DBG("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_IS_HDMI_t)))
    {
        return;
    }

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
	if(QM_IsSourceHDMI(pSrcInfo))
    	param.bIsHDMI = TRUE;
	else
		param.bIsHDMI = FALSE;

    if (copy_to_user((U32*)arg, (U32*)&param, sizeof(SC_IS_HDMI_t)))
    {
        return;
    }
}

void MDrv_SC_SetHDMIEQ(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)	// 081027 wlgnsl99 LGE : set HDMI EQ
{
    SC_SET_HDME_EQ_t param;
    //printk("%s\n", __FUNCTION__);
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_HDME_EQ_t)))
    {
        return;
    }
    MHal_HDCP_SetEQ(param.u8EqLevel);
}

extern U8 SST_STATIC_CORE_TH_LV1_VALUE;//victor 20080923
extern U8 SST_STATIC_CORE_TH_LV2_VALUE;//victor 20080923
extern U8 SST_STATIC_CORE_TH_LV3_VALUE;//victor 20080923
extern U8 SST_STATIC_CORE_TH_LV4_VALUE;//victor 20080923
extern U8 SST_STATIC_CORE_TH_LV5_VALUE;//[091201_Leo]

//victor 20080923
void MDrv_SC_SetDeFeatheringThreshold(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_DEFEATHER_TH_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_DEFEATHER_TH_t)))
    {
        return;
    }

    SST_STATIC_CORE_TH_LV1_VALUE = param.u8ThLv1;
    SST_STATIC_CORE_TH_LV2_VALUE = param.u8ThLv2;
    SST_STATIC_CORE_TH_LV3_VALUE = param.u8ThLv3;
    SST_STATIC_CORE_TH_LV4_VALUE = param.u8ThLv4;
    SST_STATIC_CORE_TH_LV5_VALUE = param.u8ThLv5;//[091201_Leo]
}

U8 MST_Pre_ConBri[][4]=
{
    { PQ_MAP_REG(REG_SC_BK0F_53_L), 0x04, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_53_L), 0x02, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_53_L), 0x01, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_47_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_47_H), 0x0F, 0x04 },
    { PQ_MAP_REG(REG_SC_BK0F_48_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_48_H), 0x0F, 0x04 },
    { PQ_MAP_REG(REG_SC_BK0F_49_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_49_H), 0x0F, 0x04 },
    { PQ_MAP_REG(REG_SC_BK0F_4D_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_4D_H), 0x07, 0x04 },
    { PQ_MAP_REG(REG_SC_BK0F_4E_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_4E_H), 0x07, 0x04 },
    { PQ_MAP_REG(REG_SC_BK0F_4F_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK0F_4F_H), 0x07, 0x04 },
    { PQ_MAP_REG(REG_TABLE_END), 0x00, 0x00},
};
//victor 20081016, ContrastBrightness
U8 MST_Post_ConBri[][4]=
{
    { PQ_MAP_REG(REG_SC_BK25_01_L), 0x10, 0x00 },
    { PQ_MAP_REG(REG_SC_BK25_01_L), 0x20, 0x00 },
    { PQ_MAP_REG(REG_SC_BK25_01_L), 0x40, 0x00 },

    { PQ_MAP_REG(REG_SC_BK25_21_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK25_21_H), 0x07, 0x04 },
    { PQ_MAP_REG(REG_SC_BK25_22_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK25_22_H), 0x07, 0x04 },
    { PQ_MAP_REG(REG_SC_BK25_23_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK25_23_H), 0x07, 0x04 },
    { PQ_MAP_REG(REG_SC_BK25_24_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK25_24_H), 0xFF, 0x04 },
    { PQ_MAP_REG(REG_SC_BK25_25_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK25_25_H), 0xFF, 0x04 },
    { PQ_MAP_REG(REG_SC_BK25_26_L), 0xFF, 0x00 },
    { PQ_MAP_REG(REG_SC_BK25_26_H), 0xFF, 0x04 },

    { PQ_MAP_REG(REG_TABLE_END), 0x00, 0x00},
};

//victor 20081016, ContrastBrightness
void MDrv_SC_SetPreConBri(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    //U8 i;			// changed to remove warning(dreamer@lge.com)
    EN_IP_Info SetPreConBri;
    U16 u16R_Gain, u16G_Gain, u16B_Gain;
    U16 u16R_Offset, u16G_Offset, u16B_Offset;
    SC_SET_CONBRI16_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CONBRI16_t)))
    {
        return;
    }
    SC_PQ_DBG("############################%s %d\n", __FUNCTION__, __LINE__);
    SetPreConBri.pIPTable = &MST_Pre_ConBri[0][0];
    SetPreConBri.u8TabNums = 1;
    SetPreConBri.u8TabType = PQ_TABTYPE_SCALER;
    SetPreConBri.u8TabIdx = 0;

	u16R_Gain = param.u16R_Gain;
	u16G_Gain = param.u16G_Gain;
	u16B_Gain = param.u16B_Gain;

	u16R_Offset = param.u16R_Offset;
	u16G_Offset = param.u16G_Offset;
	u16B_Offset = param.u16B_Offset;

    // Noise rounding
    if (param.bNoiseRoundEn)
    {
        SetPreConBri.pIPTable[3] = 0x04;
    }
    else
    {
        SetPreConBri.pIPTable[3] = 0x00;
    }

    if(u16R_Gain + u16G_Gain + u16B_Gain> 0)
    {
        // Brightness En
        if (param.bBrightnessEn)
        {
            SetPreConBri.pIPTable[7] = 0x02;
        }
        else
        {
            SetPreConBri.pIPTable[7] = 0x00;
        }

        // Contrast En
        if (param.bContrastEn)
        {
            SetPreConBri.pIPTable[11] = 0x01;
        }
        else
        {
            SetPreConBri.pIPTable[11] = 0x00;
        }
    }
    // R gain
    SetPreConBri.pIPTable[15] =  u16R_Gain & 0xFF;
    SetPreConBri.pIPTable[19] = (u16R_Gain >> 8) & 0x0F;

    // G gain
    SetPreConBri.pIPTable[23] =  u16G_Gain & 0xFF;
    SetPreConBri.pIPTable[27] = (u16G_Gain >> 8) & 0x0F;

    // B gain
    SetPreConBri.pIPTable[31] =  u16B_Gain & 0xFF;
    SetPreConBri.pIPTable[35] = (u16B_Gain >> 8) & 0x0F;

    // R offset
    SetPreConBri.pIPTable[39] =  u16R_Offset & 0xFF;
    SetPreConBri.pIPTable[43] = (u16R_Offset >> 8) & 0x0F;

    // G offset
    SetPreConBri.pIPTable[47] =  u16G_Offset & 0xFF;
    SetPreConBri.pIPTable[51]= (u16G_Offset >> 8) & 0x0F;

    // B offset
    SetPreConBri.pIPTable[55] = u16B_Offset & 0xFF;
    SetPreConBri.pIPTable[59] = (u16B_Offset >> 8) & 0x0F;
#if USE_MENULOAD_PQ
    MHal_SC_ML_Start();
    MENULOAD_LOCK;
    MHal_SC_ML_ChangeBank(0x0F);
    MHal_SC_ML_WriteData(REG_SC_S_VOP(0x53), (SetPreConBri.pIPTable[ 3] |
                                              SetPreConBri.pIPTable[ 7] |
                                              SetPreConBri.pIPTable[11]),
                                             // 20090827 daniel.huang: for menuload mask
                                             (SetPreConBri.pIPTable[ 2] |
                                              SetPreConBri.pIPTable[ 6] |
                                              SetPreConBri.pIPTable[10])
                                              );

    MHal_SC_ML_WriteData(REG_SC_S_VOP(0x47), (SetPreConBri.pIPTable[15] | ((U16)
                                              SetPreConBri.pIPTable[19] << 8)),  // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                             // 20090827 daniel.huang: for menuload mask
                                             (SetPreConBri.pIPTable[14] | ((U16)
                                              SetPreConBri.pIPTable[18] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_S_VOP(0x48), (SetPreConBri.pIPTable[23] | ((U16)
                                              SetPreConBri.pIPTable[27] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                             // 20090827 daniel.huang: for menuload mask
                                             (SetPreConBri.pIPTable[22] | ((U16)
                                              SetPreConBri.pIPTable[26] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_S_VOP(0x49), (SetPreConBri.pIPTable[31] | ((U16)
                                              SetPreConBri.pIPTable[35] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                             // 20090827 daniel.huang: for menuload mask
                                             (SetPreConBri.pIPTable[30] | ((U16)
                                              SetPreConBri.pIPTable[34] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_S_VOP(0x4D), (SetPreConBri.pIPTable[39] | ((U16)
                                              SetPreConBri.pIPTable[43] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                             // 20090827 daniel.huang: for menuload mask
                                             (SetPreConBri.pIPTable[38] | ((U16)
                                              SetPreConBri.pIPTable[42] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_S_VOP(0x4E), (SetPreConBri.pIPTable[47] | ((U16)
                                              SetPreConBri.pIPTable[51] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                             // 20090827 daniel.huang: for menuload mask
                                             (SetPreConBri.pIPTable[46] | ((U16)
                                              SetPreConBri.pIPTable[50] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_S_VOP(0x4f), (SetPreConBri.pIPTable[55] | ((U16)
                                              SetPreConBri.pIPTable[59] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                             // 20090827 daniel.huang: for menuload mask
                                             (SetPreConBri.pIPTable[54] | ((U16)
                                              SetPreConBri.pIPTable[58] << 8))
                                              );
    MHal_SC_ML_End();
    MENULOAD_UNLOCK;
#else
    // 20090504 daniel.huang: change for scmap
        MDrv_SC_IP1_WaitOutputVSync(1, 30);
        MDrv_SCMAP_DumpTable(&SetPreConBri);
#endif
}

//victor 20081016, ContrastBrightness
//FitchHsu MENU LOAD
void MDrv_SC_SetPostConBri(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    //U8 i;		// changed to remove warning(dreamer@lge.com)
    U8 u8PostGain, u8Postoffset;
    EN_IP_Info SetPostConBri;
    U16 u16R_Gain, u16G_Gain, u16B_Gain, u16GainCon;
    U16 u16R_Offset, u16G_Offset, u16B_Offset, u16OffsetCon;
    SC_SET_CONBRI_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_CONBRI_t)))
    {
        return;
    }
    SC_PQ_DBG("############################%s %d\n", __FUNCTION__, __LINE__);

    SetPostConBri.pIPTable = &MST_Post_ConBri[0][0];

	SetPostConBri.u8TabNums = 1;
	SetPostConBri.u8TabType = PQ_TABTYPE_SCALER;
	SetPostConBri.u8TabIdx = 0;

    u8PostGain= 192;
    u8Postoffset =64;
    u16GainCon = 0x400;
    u16OffsetCon = 0x400;
    u16R_Gain = ((u16GainCon * param.u8R_Gain)/u8PostGain);
    u16G_Gain = ((u16GainCon * param.u8G_Gain)/u8PostGain);
    u16B_Gain = ((u16GainCon * param.u8B_Gain)/u8PostGain);

    if (u8Postoffset>= param.u8R_Offset)
    {
        u16R_Offset = u16OffsetCon - (2* (u8Postoffset - param.u8R_Offset));
    }
    else
    {
        u16R_Offset = u16OffsetCon + (2* (param.u8R_Offset - u8Postoffset));
    }
    if (u8Postoffset>= param.u8G_Offset)
    {
        u16G_Offset = u16OffsetCon - (2* (u8Postoffset - param.u8G_Offset));
    }
    else
    {
        u16G_Offset = u16OffsetCon + (2* (param.u8G_Offset - u8Postoffset));
    }

    if (u8Postoffset>= param.u8B_Offset)
    {
        u16B_Offset = u16OffsetCon - (2* (u8Postoffset - param.u8B_Offset));
    }
    else
    {
        u16B_Offset = u16OffsetCon + (2* (param.u8B_Offset - u8Postoffset));
    }


    // Brightness En
    if (param.bBrightnessEn)
    {
        SetPostConBri.pIPTable[3] = 0x10;
    }
    else
    {
        SetPostConBri.pIPTable[3] = 0x00;
    }

    // Contrast En
    if (param.bContrastEn)
    {
        SetPostConBri.pIPTable[7] = 0x20;
    }
    else
    {
        SetPostConBri.pIPTable[7] = 0x00;
    }

    // Noise rounding
    if (param.bNoiseRoundEn)
    {
        SetPostConBri.pIPTable[11] = 0x40;
    }
    else
    {
        SetPostConBri.pIPTable[11] = 0x00;
    }

    // R offset
    SetPostConBri.pIPTable[15] =  u16R_Offset & 0xFF;
    SetPostConBri.pIPTable[19] = (u16R_Offset >> 8) & 0x0F;

    // G offset
    SetPostConBri.pIPTable[23] =  u16G_Offset & 0xFF;
    SetPostConBri.pIPTable[27] = (u16G_Offset >> 8) & 0x0F;

    // B offset
    SetPostConBri.pIPTable[31] =  u16B_Offset & 0xFF;
    SetPostConBri.pIPTable[35] = (u16B_Offset >> 8) & 0x0F;

    // R gain
    SetPostConBri.pIPTable[39] =  u16R_Gain & 0xFF;
    SetPostConBri.pIPTable[43] = (u16R_Gain >> 8) & 0x0F;

    // G gain
    SetPostConBri.pIPTable[47] =  u16G_Gain & 0xFF;
    SetPostConBri.pIPTable[51] = (u16G_Gain >> 8) & 0x0F;

    // B gain
    SetPostConBri.pIPTable[55] =  u16B_Gain & 0xFF;
    SetPostConBri.pIPTable[59] = (u16B_Gain >> 8) & 0x0F;
#if USE_MENULOAD_PQ
    MHal_SC_ML_Start();
    MENULOAD_LOCK;
    MHal_SC_ML_ChangeBank(0x25);
    MHal_SC_ML_WriteData(REG_SC_XVYCC(0x01), (SetPostConBri.pIPTable[ 3] |
                                              SetPostConBri.pIPTable[ 7] |
                                              SetPostConBri.pIPTable[11]),
                                              // 20090827 daniel.huang: for menuload mask
                                             (SetPostConBri.pIPTable[ 2] |
                                              SetPostConBri.pIPTable[ 6] |
                                              SetPostConBri.pIPTable[10])
                                              );

    MHal_SC_ML_WriteData(REG_SC_XVYCC(0x21), (SetPostConBri.pIPTable[15] | ((U16)
                                              SetPostConBri.pIPTable[19] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                              // 20090827 daniel.huang: for menuload mask
                                             (SetPostConBri.pIPTable[14] | ((U16)
                                              SetPostConBri.pIPTable[18] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_XVYCC(0x22), (SetPostConBri.pIPTable[23] | ((U16)
                                              SetPostConBri.pIPTable[27] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                              // 20090827 daniel.huang: for menuload mask
                                             (SetPostConBri.pIPTable[22] | ((U16)
                                              SetPostConBri.pIPTable[26] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_XVYCC(0x23), (SetPostConBri.pIPTable[31] | ((U16)
                                              SetPostConBri.pIPTable[35] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                              // 20090827 daniel.huang: for menuload mask
                                             (SetPostConBri.pIPTable[30] | ((U16)
                                              SetPostConBri.pIPTable[34] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_XVYCC(0x24), (SetPostConBri.pIPTable[39] | ((U16)
                                              SetPostConBri.pIPTable[43] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                              // 20090827 daniel.huang: for menuload mask
                                             (SetPostConBri.pIPTable[38] | ((U16)
                                              SetPostConBri.pIPTable[42] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_XVYCC(0x25), (SetPostConBri.pIPTable[47] | ((U16)
                                              SetPostConBri.pIPTable[51] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                              // 20090827 daniel.huang: for menuload mask
                                             (SetPostConBri.pIPTable[46] | ((U16)
                                              SetPostConBri.pIPTable[50] << 8))
                                              );

    MHal_SC_ML_WriteData(REG_SC_XVYCC(0x26), (SetPostConBri.pIPTable[55] | ((U16)
                                              SetPostConBri.pIPTable[59] << 8)), // 20090829 daniel.huang: fix Picture mode "Expert1" video become dark
                                              // 20090827 daniel.huang: for menuload mask
                                             (SetPostConBri.pIPTable[54] | ((U16)
                                              SetPostConBri.pIPTable[58] << 8))
                                              );
    MHal_SC_ML_End();
    MENULOAD_UNLOCK;
#else
    // 20090504 daniel.huang: change for scmap
        MDrv_SC_IP1_WaitOutputVSync(1, 30);
        MDrv_SCMAP_DumpTable(&SetPostConBri);
#endif
}

//victor 20081106
code U8 MST_Blacklevel_Other[][4]=
{
    { PQ_MAP_REG(REG_SC_BK1A_0F_L), 0xFF, 0x00 },// 3 Pre Y_offset
    { PQ_MAP_REG(REG_SC_BK1A_0F_H), 0xFF, 0x00 },//7 xx
    { PQ_MAP_REG(REG_SC_BK1A_14_L), 0xFF, 0x00 },//11   Post Y_Gain
    { PQ_MAP_REG(REG_SC_BK1A_16_L), 0xFF, 0x00 },//15  Pre Y_Gain
    { PQ_MAP_REG(REG_SC_BK1A_18_L), 0xFF, 0x00 },//19  Post Y_offset	//cho04
	{ PQ_MAP_REG(REG_SC_BK1A_14_H), 0xFF, 0x00 },//23  C_gain	//cho04
    { PQ_MAP_REG(REG_TABLE_END), 0x00, 0x00},
};
//victor 20081106
void MDrv_SC_SetBlackLevel(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    //U8 i;
    EN_IP_Info OtherBlacklevel;
    //U32 QMTableSize;
    SC_SET_BLACKLEVEL_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_BLACKLEVEL_t)))
    {
        return;
    }
    SC_PQ_DBG("############################%s %d\n", __FUNCTION__, __LINE__);

    OtherBlacklevel.pIPTable = &MST_Blacklevel_Other[0][0];
    OtherBlacklevel.u8TabNums = 1;
    OtherBlacklevel.u8TabType = PQ_TABTYPE_SCALER;
    OtherBlacklevel.u8TabIdx = 0;

    SC_PQ_DBG("param.u8Index = %u\n", param.u8Index);

	if(MDrv_SC_IsPDPPanel())
	{
		switch(param.u8Index)
		{
			case 0:
				//	bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 1:
				//16 *0. 859 bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x12; //  3 +16 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x37; //15  *0.859 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x38; //23  *0.878 C_gain
				break;
			case 2:
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x00; // bypass Pre Y_offset sjpark21
				OtherBlacklevel.pIPTable[11] = 0x40; // bypass Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; // bypass Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // bypass Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 3:
				//	  -76 *1.08 bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0xed; //  3 -19 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x45; //15  *1.08 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 4: //NTSC RF low
				//	  -76 *1.08 bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0xef; //  3 -19 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x45; //15  *1.08 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 5: //NTSC RF high
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 6: //NTSC AV Low
				//	  -76 *1.08 bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0xef; //  3 -19 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x45; //15  *1.08 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 7: //NTSC AV high
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 8: //Wrdy RGB PC Low
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0xea; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11	Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x4B; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;

			default :
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
		}

	}
	else
	{
		switch(param.u8Index)
		{
			case 0:
				//	-32 bypass 32 bypass
	/*
			OtherBlacklevel.pIPTable[3]  = 0xF8; // -8 Pre Y_offset sjpark21
	            OtherBlacklevel.pIPTable[7]  = 0x00;
	            OtherBlacklevel.pIPTable[11] = 0x40; // bypass Post Y_Gain
	            OtherBlacklevel.pIPTable[15] = 0x40; // bypass Pre Y_Gain
	            OtherBlacklevel.pIPTable[19] = 0x08; // 8 Post Y_offset
	*/
				OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 1:
				//16 *0. 859 bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x12; //  3 +16 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x37; //15  *0.859 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x38; //23  *0.878 C_gain
				break;
			case 2:
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 3:
				//	  -96 *1.079 32 bypass
#if 1//defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) || defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERVA_BOARD_ATSC_1)
	/*
	            OtherBlacklevel.pIPTable[3]  = 0xE5; // -16 Pre Y_offset sjpark21
	            OtherBlacklevel.pIPTable[7]  = 0x00;
	            OtherBlacklevel.pIPTable[11] = 0x40; // bypass Post Y_Gain
	            OtherBlacklevel.pIPTable[15] = 0x46; // *1.079 Pre Y_Gain
	            OtherBlacklevel.pIPTable[19] = 0x08; // 8 Post Y_offset
	*/
				//	  -76 *1.08 bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0xef; //  3 -19 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x45; //15  *1.08 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
#else	// 아래 내용도 적용..
				OtherBlacklevel.pIPTable[3]  = 0xF0; // -16 Pre Y_offset sjpark21
				OtherBlacklevel.pIPTable[11] = 0x40; // bypass Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x45; // *1.079 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x08; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
#endif
				break;
			case 4: //NTSC RF low
				//	  -76 *1.08 bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0xef; //  3 -19 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x45; //15  *1.08 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
			break;
			case 5: //NTSC RF high
				//	  bypass bypass bypass bypass
	 			OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 6: //NTSC AV Low
				//	  -76 *1.08 bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0xef; //  3 -19 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x45; //15  *1.08 Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; // 8 Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 7: //NTSC AV high
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
			case 8: //Wrdy RGB PC Low
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0xea; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11	Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x4B; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;

			default :
				//	  bypass bypass bypass bypass
				OtherBlacklevel.pIPTable[3]  = 0x00; // 3 Pre Y_offset	//cho04 GP2 modify
				OtherBlacklevel.pIPTable[11] = 0x40; //11   Post Y_Gain
				OtherBlacklevel.pIPTable[15] = 0x40; //15  Pre Y_Gain
				OtherBlacklevel.pIPTable[19] = 0x00; //19  Post Y_offset
				OtherBlacklevel.pIPTable[23] = 0x40; //23  C_gain
				break;
		}
	}

#if USE_MENULOAD_PQ
    MHal_SC_ML_Start();
    MENULOAD_LOCK;
    MHal_SC_ML_ChangeBank(0x1A);
    MHal_SC_ML_WriteData(REG_SC_DLC(0x0F), (OtherBlacklevel.pIPTable[ 3] | ((U16)
                                            OtherBlacklevel.pIPTable[ 7] << 8)),
                                            // 20090827 daniel.huang: for menuload mask
                                           (OtherBlacklevel.pIPTable[ 2] | ((U16)
                                            OtherBlacklevel.pIPTable[ 6] << 8))
                                            );

    MHal_SC_ML_WriteData(REG_SC_DLC(0x14), OtherBlacklevel.pIPTable[11],
                                           // 20090827 daniel.huang: for menuload mask (fix mono color bug)
                                           OtherBlacklevel.pIPTable[10]
                                           );

    MHal_SC_ML_WriteData(REG_SC_DLC(0x16), OtherBlacklevel.pIPTable[15],
                                           // 20090827 daniel.huang: for menuload mask (fix mono color bug)
                                           OtherBlacklevel.pIPTable[14]
                                           );

  #if 0//refine menuload, [090923_Leo]
    MHal_SC_ML_WriteData(REG_SC_DLC(0x18), (OtherBlacklevel.pIPTable[19] | ((U16)
                                            OtherBlacklevel.pIPTable[23] << 8)),
                                            // 20090827 daniel.huang: for menuload mask
                                           (OtherBlacklevel.pIPTable[18] | ((U16)
                                            OtherBlacklevel.pIPTable[22] << 8))
                                            );
  #else
    MHal_SC_ML_WriteData(REG_SC_DLC(0x18), OtherBlacklevel.pIPTable[19],
                                           // 20090827 daniel.huang: for menuload mask
                                           OtherBlacklevel.pIPTable[18]
                                           );
    MHal_SC_ML_WriteData(REG_SC_DLC(0x14), (OtherBlacklevel.pIPTable[23] << 8),
                                           // 20090827 daniel.huang: for menuload mask
                                           (OtherBlacklevel.pIPTable[22] << 8)
                                           );
  #endif
    MHal_SC_ML_End();
    MENULOAD_UNLOCK;
#else

    // 20090504 daniel.huang: change for scmap
        MDrv_SC_IP1_WaitOutputVSync(1, 30);
        MDrv_SCMAP_DumpTable(&OtherBlacklevel);
#endif
}

//victor 20081112, 3DComb
void MDrv_SC_Set3DComb(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_3DCOMB_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_3DCOMB_t)))
    {
        return;
    }

    MHal_SC_Set3DComb(param);
}

void MDrv_SC_SetSSC(U32 arg)
{
	SC_SET_LVDS_SSC_t param;

	if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_LVDS_SSC_t)))
	{
	    return;
	}

	MHal_SC_SetSSC(param.u16Periodx100Hz, param.u16Percentx100, param.bEnable);
}

// CC Chen 20081124 MWE implement
//------------------------------------------------------------------------------
//  MWE
//------------------------------------------------------------------------------
void MDrv_SC_MWE_Enable(BOOL bEnable)
{
    if (bEnable)
    {
        MHal_SC_SetMWEQuality();
    }

    MHal_SC_SubWinEnable(bEnable);
}

// CC Chen 20081124 MWE implement
void MDrv_SC_MWE_SetWinType(PSC_DRIVER_CONTEXT_t pDrvCtx, SC_MWE_TYPE_e type)
{
    U16 u16HOffset, u16VOffset, u16HSize, u16VSize;

    if (type == SC_MWE_H_SPLIT)
    {
        u16HOffset = 0;//pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u16H_DispSize / 2;
        u16VOffset = 0;
        u16HSize = pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u16H_DispSize / 2;
        u16VSize = pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u16V_DispSize;
        // set border
        MHal_SC_SetSubWinBorder(FALSE, 0x00, 0x02, 0x00, 0x00, 0x00);
    }
    else if (type == SC_MWE_OFF)
    {
        u16HOffset = 0;
        u16VOffset = 0;
        u16HSize = pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u16H_DispSize;
        u16VSize = pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u16V_DispSize;
        // set border
        MHal_SC_SetSubWinBorder(FALSE, 0x00, 0x00, 0x00, 0x00, 0x00);
    }
    else
    {
        // others
        u16HOffset = 0;
        u16VOffset = 0;
        u16HSize = pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u16H_DispSize;
        u16VSize = pDrvCtx->pPanelInfo->u16Height;
    }
//	printk("fitch pDrvCtx->pPanelInfo->u16HStart=%x\n", pDrvCtx->pPanelInfo->u16HStart);
//	printk("fitch pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u16H_DispSize=%x\n", pDrvCtx->SrcInfo[SC_MAIN_WINDOW].u16H_DispSize);
    MHal_SC_SetSubDispWin(
          pDrvCtx->pPanelInfo->u16HStart+ u16HOffset,
          pDrvCtx->pPanelInfo->u16HStart + u16HOffset + u16HSize - 1,
          pDrvCtx->pPanelInfo->u16DE_VStart + u16VOffset,
          pDrvCtx->pPanelInfo->u16DE_VStart + u16VOffset + u16VSize - 1);
}

// FitchHsu 20081209 implement THX mode
void MDrv_SC_Set_THXMode(U32 arg)
{
	SC_SET_THX_t param;

	if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_THX_t)))
	{
	    return;
	}

	MHal_SC_Set_THXMode(param.bIsTHXMode);
}

// LGE [vivakjh]  2008/12/11 Merge!!  FitchHsu 20081209 implement frame lock status report
void MDrv_SC_Get_FrameLock_Status(U32 arg)
{
    BOOL bframelock;
    bframelock = MHal_LPLL_GetFrameLock_Status();
    if (copy_to_user((BOOL*)arg, (BOOL*)&bframelock, sizeof(BOOL)))
    {
        return;
    }
}


//LGE [vivakjh]  2008/12/07	pDrvCtx를 인자로 사용하지 않는 함수에서 PDP/LCD를 구분하기 위한 함수. mdrv_scaler.c 내에서만 사용함.
BOOL MDrv_SC_IsPDPPanel(void)
{
	return _gIsPdpPanel;
}

/******************************************************************************
	LGE IOCTL : 240 ~ 254 (추가 자제할 것)
*******************************************************************************/
// LGE [vivakjh] 2008/12/09 	For setting the PDP's Color Wash
void MDrv_SC_SetColorWash4PDP(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_COLOR_WASH_ENABLE_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_COLOR_WASH_ENABLE_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

	if ((pSrcInfo->SrcType == INPUT_SOURCE_YPBPR_1) || (pSrcInfo->SrcType == INPUT_SOURCE_YPBPR_2) ||
		(pSrcInfo->SrcType == INPUT_SOURCE_HDMI_A) || (pSrcInfo->SrcType == INPUT_SOURCE_HDMI_B) ||
        (pSrcInfo->SrcType == INPUT_SOURCE_HDMI_C) || (pSrcInfo->SrcType == INPUT_SOURCE_HDMI_D))
	{
		if (param.bEnable)
			*(volatile unsigned int*)(0xbf000000+(0x101266<<1)) &= ~0x0010 ;	// enable OSD 1 R
		else
			*(volatile unsigned int*)(0xbf000000+(0x101266<<1)) |= 0x0010 ;	// disable OSD 1 R
	}
}

// Real cinema on일경우 24P를 48Hz로, off일경우 60Hz로 변경.
void MDrv_SC_SetFrameTo48Hz(PSC_DRIVER_CONTEXT_t pDrvCtx, BOOL arg)
{
    BOOL param;
	PSC_SOURCE_INFO_t pSrcInfo;
    //U32 u32Time; //LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue

    if (copy_from_user(&param, (void __user *)arg, sizeof(BOOL)))
    {
        return;
    }

    pDrvCtx->SrcInfo[0].binputsync = TRUE; //LGE [vivakjh] 2009/01/18 Merge!!		FitchHsu 20090116 VGA Lock issue

	pSrcInfo = &pDrvCtx->SrcInfo[0];
	if((pSrcInfo->u16Input_VFreq < 230 || pSrcInfo->u16Input_VFreq > 250) ||bIsForceFreeRun)
		return;

	// shjang_090904
	if(pDrvCtx->pPanelInfo->u8LCDorPDP == 2 || pDrvCtx->pPanelInfo->u8LCDorPDP == 3)	// 60Hz 모델만 적용됨.
	{
		if(_gFrameTo48Hz==param)
			return;
		else
			_gFrameTo48Hz = param;

		SC_FPLL_DBG("##########MDrv_SC_SetPanelTiming( 7 )\n");
        MDrv_SC_SetPanelTiming(pDrvCtx, TRUE, FALSE);
	}
}

//LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
void MDrv_SC_SetTimgCHGstauts(PSC_DRIVER_CONTEXT_t pDrvCtx,BOOL arg)
{
	BOOL param;
	PSC_SOURCE_INFO_t pSrcInfo;

	if (copy_from_user(&param, (void __user *)arg, sizeof(BOOL)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[0];
    pSrcInfo->bTimingCHG = param;
}


#define FB_DBG(x) //x

// 20090930 daniel.huang: refine and add LSB for GrabPixel
static void _MDrv_SC_ReadPixel(volatile U8* pMEMYC, volatile U8* pMEMLSB,
                         U16 u16X,
                         U16 u16OffsetPixelAlign,
                         BOOL bMemFmt422,
                         U8 *pPoint, U16 u16PointSize,
                         BOOL bR_Cr,
                         BOOL bYCSep)
{
    PIXEL_24BIT *pPoint24 = (PIXEL_24BIT*)pPoint;
    PIXEL_32BIT *pPoint32 = (PIXEL_32BIT*)pPoint;
    U16 u16XOffset = u16X % u16OffsetPixelAlign;
    U16 u16LSB_bytepos, u16LSB_bitpos;

    Chip_Read_Memory(); // 20090828 daniel.huang: need this dummy read for fixing T3 MIPS read buffer

    if (bMemFmt422)
    {
        u16LSB_bitpos = (u16X % (u16OffsetPixelAlign * 2)) * 4;
        u16LSB_bytepos = u16LSB_bitpos / 8;
        u16LSB_bitpos = u16LSB_bitpos % 8;

        if (bYCSep) // YCSep takes effect only in 422 foramt
        {
            pMEMYC += u16XOffset;
            if (u16PointSize == 4)
            {
                pPoint32->G_Y  = pMEMYC[0] << 2;
                if (bR_Cr) {
                    pPoint32->B_Cb = pMEMYC[u16OffsetPixelAlign-1] << 2;   // Y  Y
                    pPoint32->R_Cr = pMEMYC[u16OffsetPixelAlign  ] << 2;   // Cb Cr
                }
                else {
                     pPoint32->B_Cb = pMEMYC[u16OffsetPixelAlign  ] << 2;
                     pPoint32->R_Cr = pMEMYC[u16OffsetPixelAlign+1] << 2;
                }
            }
            else
            {
                pPoint24->G_Y  = pMEMYC[0];
                if (bR_Cr) {
                     pPoint24->B_Cb = pMEMYC[u16OffsetPixelAlign-1];
                     pPoint24->R_Cr = pMEMYC[u16OffsetPixelAlign  ];
                }
                else {
                     pPoint24->B_Cb = pMEMYC[u16OffsetPixelAlign  ];
                     pPoint24->R_Cr = pMEMYC[u16OffsetPixelAlign+1];
                }
            }

        }
        else
        {   // 422 format
            pMEMYC += u16XOffset * 2;
            if (u16PointSize == 4)
            {
                pPoint32->G_Y  = pMEMYC[0] << 2;
                if (bR_Cr) {
                    pPoint32->B_Cb = pMEMYC[-1]<< 2;
                    pPoint32->R_Cr = pMEMYC[1] << 2;
                }
                else {
                    pPoint32->B_Cb = pMEMYC[1] << 2;
                    pPoint32->R_Cr = pMEMYC[3] << 2;
                }
            }
            else
            {
                pPoint24->G_Y  = pMEMYC[0];
                if (bR_Cr) {
                    pPoint24->B_Cb = pMEMYC[-1];
                    pPoint24->R_Cr = pMEMYC[1];
                }
                else {
                     pPoint24->B_Cb = pMEMYC[1];
                     pPoint24->R_Cr = pMEMYC[3];
                }
            }
        }
        if (u16PointSize == 4)
        {
            pPoint32->G_Y |= ((pMEMLSB[u16LSB_bytepos] >> u16LSB_bitpos) & 0x3);

            if (u16LSB_bitpos == 0)
            {
                if (bR_Cr) {
                    pPoint32->B_Cb |= ((pMEMLSB[u16LSB_bytepos-1] >> 6) & 0x3);
                    pPoint32->R_Cr |= ((pMEMLSB[u16LSB_bytepos  ] >> 2) & 0x3);
                }
                else {
                    pPoint32->B_Cb |= ((pMEMLSB[u16LSB_bytepos  ] >> 2) & 0x3);
                    pPoint32->R_Cr |= ((pMEMLSB[u16LSB_bytepos-1] >> 6) & 0x3);
                }
            }
            else    // u16LSB_bitpos == 4
            {
                if (bR_Cr) {
                    pPoint32->B_Cb |= ((pMEMLSB[u16LSB_bytepos] >> 2) & 0x3);
                    pPoint32->R_Cr |= ((pMEMLSB[u16LSB_bytepos] >> 6) & 0x3);
                }
                else {
                    pPoint32->B_Cb |= ((pMEMLSB[u16LSB_bytepos] >> 6) & 0x3);
                    pPoint32->R_Cr |= ((pMEMLSB[u16LSB_bytepos] >> 2) & 0x3);
                }
            }
        }
    }
    else    // 444 format
    {
        pMEMYC += u16XOffset * 4;
        if (u16PointSize == 4)
        {
             pPoint32->B_Cb = (pMEMYC[0])      |
                            ((pMEMYC[1] &0x03)<<8);
             pPoint32->G_Y  = (pMEMYC[1] >> 2) |
                            ((pMEMYC[2] &0x0F)<<6);
             pPoint32->R_Cr = (pMEMYC[2] >> 4) |
                            ((pMEMYC[3] &0x3F)<<4);
        }
        else
        {
             pPoint24->B_Cb = (pMEMYC[0]>>2) |
                             (pMEMYC[1]<<6);
             pPoint24->G_Y  = (pMEMYC[1]>>4) |
                             (pMEMYC[2]<<4);
             pPoint24->R_Cr = (pMEMYC[2]>>6) |
                             (pMEMYC[3]<<2);
        }
    }
}

void MDrv_SC_GetFrameData(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    U8 *pRect;
    U32 u32RectSize;
    SC_FRAMEDATA_t param;
    PSC_SOURCE_INFO_t pSrcInfo;
    SC_FRAMEDATA_t *parg = (SC_FRAMEDATA_t*)arg;

    if (copy_from_user(&param, (void __user *)parg, sizeof(SC_FRAMEDATA_t)))
    {
        return;
    }

	//MHal_SC_IP1_SetInputSourceEnable(TRUE, FALSE);

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

    if (param.u16PointSize != 4 && param.u16PointSize != 3)   // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    {
        SC_DBG("invalid parameter!\n"); return;
    }

	//printk("pSrcInfo->u16H_sizeAfterIP2=%d pSrcInfo->u16V_sizeAfterIP2=%d\n", pSrcInfo->u16H_sizeAfterIP2, pSrcInfo->u16V_sizeAfterIP2);

    u32RectSize = param.u32RectPitch * param.height * param.u16PointSize;   // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    pRect = kmalloc(u32RectSize, GFP_KERNEL);
    if (!pRect)
    {
        return;
    }

	//memset( pRect, 0, u32RectSize );//lachesis_090914

    MDrv_SC_GetFrameDataCore(&param.x0, &param.y0, &param.width, &param.height,
                             &param.bRGB,
                             pRect,
                             param.u32RectPitch,
                             u32RectSize,
                             param.u16PointSize,
                             TRUE);

    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    copy_to_user(parg->pRect, pRect, u32RectSize);
    copy_to_user(&(parg->x0),     &param.x0,     sizeof(param.x0));
    copy_to_user(&(parg->y0),     &param.y0,     sizeof(param.y0));
    copy_to_user(&(parg->width),  &param.width,  sizeof(param.width));
    copy_to_user(&(parg->height), &param.height, sizeof(param.height));
    copy_to_user(&(parg->bRGB),   &param.bRGB,   sizeof(param.bRGB));
    kfree(pRect);

}

// 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
void MDrv_SC_GetFrameDataCore(U16 *pX0, U16 *pY0, U16 *pWidth, U16 *pHeight,
                              BOOL *pRGB,
                              U8 *pRect,
                              U32 u32RectPitch,
                              U32 u32RectSize,
                              U16 u16PointSize,
                              BOOL bUseDispCoordinate)
{
    U16 x0 = *pX0,  y0 = *pY0, w = *pWidth, h = *pHeight;
    S16 s16dx, s16dy;   // 20090921 daniel.huang: for mirror mode
    U16 xPoint, yPoint; // 20090921 daniel.huang: for mirror mode
    U8 *pPoint;
    volatile U8 *pMEM, *pMEMLSB;    // 20090930 daniel.huang: refine and add LSB for GrabPixel
    U8 u8BytePerPixel;
    U8 u8LSB_YM4_Size;
    U16 x, y;
    U16 u16OffsetPixelAlign;
    U32 u32Y_Offset, u32X_Offset, u32X_LSB_Offset;  // 20090930 daniel.huang: refine and add LSB for GrabPixel
    SC_FRAMEBUF_INFO_t FBInfo;

    U32 u32MIPS_MIU_Base;
    U32 u32SC_MemAddr, u32SC_MemSize;
    U32 u32MIU1_MemAddr, u32MIU1_MemSize;
    U32 u32XYOffset;

    MDrv_SYS_GetMMAP(E_SYS_MMAP_SCALER_DNR_BUF, &u32SC_MemAddr, &u32SC_MemSize);
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MIU1_BASE, &u32MIU1_MemAddr, &u32MIU1_MemSize);
    if(u32SC_MemAddr >= u32MIU1_MemAddr)
        u32MIPS_MIU_Base = MIPS_MIU1_BASE;
    else
        u32MIPS_MIU_Base = MIPS_MIU0_BASE;

    MHal_SC_GetFrameBufInfo(&FBInfo);

    if (FBInfo.bMemFormat422 == TRUE)
    {
        if (FBInfo.u8BitPerPixel == 24)
        {
            FB_DBG(printk("422_10BIT\n"));
            u8BytePerPixel = 3;
            u8LSB_YM4_Size = BYTE_PER_WORD;     // 20100116 daniel.huang: fix linear mode address incorrect
            u16OffsetPixelAlign = BYTE_PER_WORD/2*2; // *2 (F0 F0)
        }
        else //if (FBInfo.u8BitPerPixel == 16)
        {
            FB_DBG(printk("422_8BIT\n"));
            u8BytePerPixel = 2;
            u8LSB_YM4_Size = 0;
            u16OffsetPixelAlign = BYTE_PER_WORD/2*2; // *2 (F0 F0)
        }
    }
    else
    {   // using 444 10 bit
        FB_DBG(printk("444_10BIT\n"));
        u8BytePerPixel = 4;
        u8LSB_YM4_Size = 0;
        u16OffsetPixelAlign = BYTE_PER_WORD/4*2; // *2 (F0 F0)
    }
    // 20100116 daniel.huang: fix linear mode address incorrect
    u8LSB_YM4_Size = u8LSB_YM4_Size * (FBInfo.bLinearAddrMode ? 1: 2);

    // 20090921 daniel.huang: for mirror mode
    if (FBInfo.bh_mirror)
    {
        s16dx = -1;
    }
    else
    {
        s16dx = 1;
    }
    if (FBInfo.bv_mirror)
    {
        if (FBInfo.bInterlace)
        {
            FBInfo.u32IPMBase0 -= ((U32)(FBInfo.u16FrameLineCnt-2) * FBInfo.u16IPMOffset * u8BytePerPixel);
            FBInfo.u32IPMBase1 -= ((U32)(FBInfo.u16FrameLineCnt-2) * FBInfo.u16IPMOffset * u8BytePerPixel);
        }
        else
        {
            if (FBInfo.bLinearAddrMode)
            {
                FBInfo.u32IPMBase0 -= ((U32)(FBInfo.u16FrameLineCnt-1) * FBInfo.u16IPMOffset * u8BytePerPixel);
            }
            else
            {
                FBInfo.u32IPMBase0 -= ((U32)(FBInfo.u16FrameLineCnt-1)* 2 * FBInfo.u16IPMOffset * u8BytePerPixel);
            }
        }
        s16dy = -1;
    }
    else
    {
        s16dy = 1;
    }

    if (bUseDispCoordinate)
    {
        // 2009/09/23 daniel.huang: for calibration with mirror
        if (FBInfo.bh_mirror)
        {
            x0 = MHal_SC_VOP_GetDispHEnd() - MHal_SC_VOP_GetDispHStart() - x0;
            FB_DBG(printk("mirror: disp x0=%u\n", x0));
        }
        if (FBInfo.bv_mirror)
        {
            y0 = MHal_SC_VOP_GetDispVEnd() - MHal_SC_VOP_GetDispVStart() - y0;
            FB_DBG(printk("mirror: disp y0=%u\n", y0));
        }
    }
    else
    {
        // 2009/09/23 daniel.huang: for calibration with mirror
        if (FBInfo.bh_mirror)
        {
            x0 = FBInfo.u16IPMFetch - 1 - x0;
            FB_DBG(printk("mirror: crop x0=%u\n", x0));
        }
        if (FBInfo.bv_mirror)
        {
            y0 = FBInfo.u16FrameLineCnt - 1 - y0;
            FB_DBG(printk("mirror: crop y0=%u\n", y0));
        }
    }

    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    // calculate offset between ipmbase & opmbase
    if (bUseDispCoordinate)
    {
        U16 u16tmp;     // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        u32XYOffset = (FBInfo.u32OPMBase0 - FBInfo.u32IPMBase0) / (u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2));
        x = u32XYOffset % FBInfo.u16IPMOffset;
        y = u32XYOffset / FBInfo.u16IPMOffset;
        if (FBInfo.bInterlace) y *= 2;

        FB_DBG(printk("ipmbase0=0x%x, opmbase0=0x%x, bpp=%u, li=%u\n", FBInfo.u32IPMBase0, FBInfo.u32OPMBase0, u8BytePerPixel, FBInfo.bLinearAddrMode));
        FB_DBG(printk("offset between ipmbase & opmbase: x=%u, y=%u\n", x, y));

        // convert disp coordinate to crop coordinate
        FB_DBG(printk("disp coordinate: x0=%u, y0=%u, w=%u, h=%u\n", x0, y0, w, h));
        x0 = MHal_SC_OP1_GetHCropCoordiante(x0, TRUE);
        y0 = MHal_SC_OP1_GetVCropCoordiante(y0, TRUE);
        u16tmp = MHal_SC_OP1_GetHCropCoordiante(w, FALSE);  // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        if (u16tmp < w) w = u16tmp;                         // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        u16tmp = MHal_SC_OP1_GetVCropCoordiante(h, FALSE);  // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        if (u16tmp < h) h = u16tmp;                         // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        FB_DBG(printk("crop coordinate: x0=%u, y0=%u, w=%u, h=%u\n", x0, y0, w, h));

        x0 = x0 + x + FBInfo.u16LBOffset;
        y0 = y0 + y;
        FB_DBG(printk("memory coordinate: x0=%u, y0=%u, w=%u, h=%u\n", x0, y0, w, h));
    }

    // 20090922 daniel.huang: for memory protection
    if (FBInfo.bh_mirror)
        w = ((x0+1) < w) ? (x0+1) : w;
    else
        w = ((x0+w) > FBInfo.u16IPMFetch) ? (FBInfo.u16IPMFetch-x0) : w;
    // 20090922 daniel.huang: for memory protection
    if (FBInfo.bv_mirror)
        h = ((y0+1) < h) ? (y0+1) : h;
    else
        h = ((y0+h) > FBInfo.u16FrameLineCnt) ? (FBInfo.u16FrameLineCnt-y0) : h;

    // 20090921 daniel.huang: for mirror mode
    yPoint = 0;
    for (y = y0; y != (U16)(y0+h*s16dy); y=y+s16dy)
    {
        if (FBInfo.bInterlace)
        {
            u32Y_Offset = (U32)y/2 * FBInfo.u16IPMOffset * u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2);
            if (y & 0x1)
                u32Y_Offset += FBInfo.u32IPMBase1;
            else
                u32Y_Offset += FBInfo.u32IPMBase0;
        }
        else
        {
            u32Y_Offset = (U32)y * FBInfo.u16IPMOffset * u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2);
            u32Y_Offset += FBInfo.u32IPMBase0;
        }
        FB_DBG(printk("***(%u) Yaddress=0x%x, u32Y_Offset:0x%x, u16IPMOffset:%u, y:%u, u8BytePerPixel:%u, linear multiply:%u\n",
            y, u32MIPS_MIU_Base + u32Y_Offset, (int)u32Y_Offset, FBInfo.u16IPMOffset, y, u8BytePerPixel, (FBInfo.bLinearAddrMode ? 1: 2)));

        // 20090921 daniel.huang: for mirror mode
        xPoint = 0;
        for (x = x0; x != (U16)(x0+w*s16dx); x=x+s16dx)
        {
            u32X_Offset = (U32)x / u16OffsetPixelAlign * u16OffsetPixelAlign * u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2);
            // 20090930 daniel.huang: refine and add LSB for GrabPixel
            u32X_LSB_Offset = (U32)x / (u16OffsetPixelAlign*2) * (u16OffsetPixelAlign*2) * u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2)
                            + BYTE_PER_WORD * (FBInfo.bLinearAddrMode ? 2: 4);  // 20100116 daniel.huang: fix linear mode address incorrect

            //FB_DBG(printk("[2]u32X_Offset=0x%x, u16OffsetPixelAlign=%u, x=%u, align(x)=%u\n",
            //    (int)u32X_Offset, u16OffsetPixelAlign, x, x / u16OffsetPixelAlign * u16OffsetPixelAlign));

            // 20090921 daniel.huang: for mirror mode
            pPoint = pRect+ (u32RectPitch * yPoint + xPoint) * u16PointSize;
            if ((x / u16OffsetPixelAlign) & 0x1)
            {
                pMEM = (volatile U8 *)(u32MIPS_MIU_Base + u32Y_Offset + u32X_Offset + u8LSB_YM4_Size);
            }
            else
            {
                pMEM = (volatile U8 *)(u32MIPS_MIU_Base + u32Y_Offset + u32X_Offset);
            }
            pMEMLSB = (volatile U8 *)(u32MIPS_MIU_Base + u32Y_Offset + u32X_LSB_Offset);    // 20090930 daniel.huang: refine and add LSB for GrabPixel

            FB_DBG(printk("***(%u,%u) Y/C address: 0x%x, LSB address: 0x%x\n", x, y, (U32)pMEM, (U32)pMEMLSB));

            // 20090930 daniel.huang: refine and add LSB for GrabPixel
            _MDrv_SC_ReadPixel(pMEM,
                               pMEMLSB,
                               x,
                               u16OffsetPixelAlign,
                               FBInfo.bMemFormat422,
                               pPoint,
                               u16PointSize,
                               (x&0x1),
                               FBInfo.bYCSeperate);

            xPoint++;   // 20090921 daniel.huang: for mirror mode
        }
        yPoint++;       // 20090921 daniel.huang: for mirror mode
    }

    MDrv_SYS_Flush_Memory();

    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    *pX0 = x0;
    *pY0 = y0;
    *pWidth = w;
    *pHeight = h;
    *pRGB = MHal_SC_VIP_GetCSC();
}

// 20090930 daniel.huang: refine and add LSB for GrabPixel
static void _MDrv_SC_WritePixel(volatile U8* pMEMYC, volatile U8* pMEMLSB,
                         U16 u16X,
                         U16 u16OffsetPixelAlign,
                         BOOL bMemFmt422,
                         U8 *pPoint, U16 u16PointSize,
                         BOOL bR_Cr,
                         BOOL bYCSep)
{
    PIXEL_24BIT *pPoint24 = (PIXEL_24BIT*)pPoint;
    PIXEL_32BIT *pPoint32 = (PIXEL_32BIT*)pPoint;
    U16 u16XOffset = u16X % u16OffsetPixelAlign;
    U16 u16LSB_bytepos, u16LSB_bitpos;

    if (bMemFmt422)
    {
        u16LSB_bitpos = (u16X % (u16OffsetPixelAlign * 2)) * 4;
        u16LSB_bytepos = u16LSB_bitpos / 8;
        u16LSB_bitpos = u16LSB_bitpos % 8;

        if (bYCSep) // YCSep takes effect only in 422 foramt
        {
            pMEMYC += u16XOffset;
            if (u16PointSize == 4)
            {
                pMEMYC[0] = pPoint32->G_Y>>2;
                pMEMYC[u16OffsetPixelAlign] = ((bR_Cr) ? pPoint32->R_Cr : pPoint32->B_Cb)>>2;
            }
            else
            {
                pMEMYC[0] = pPoint24->G_Y;
                pMEMYC[u16OffsetPixelAlign] = (bR_Cr) ? pPoint24->R_Cr : pPoint24->B_Cb;
            }
        }
        else
        {   // 422 format
            pMEMYC += u16XOffset * 2;
            if (u16PointSize == 4)
            {
                pMEMYC[0] = pPoint32->G_Y>>2;
                pMEMYC[1] = ((bR_Cr) ? pPoint32->R_Cr : pPoint32->B_Cb)>>2;
            }
            else
            {
                pMEMYC[0] = pPoint24->G_Y;
                pMEMYC[1] = (bR_Cr) ? pPoint24->R_Cr : pPoint24->B_Cb;
            }
        }
        if (u16PointSize == 4)
        {
            pMEMLSB[u16LSB_bytepos] = pMEMLSB[u16LSB_bytepos] & ~(0xF << u16LSB_bitpos);
            pMEMLSB[u16LSB_bytepos] = pMEMLSB[u16LSB_bytepos] |
                                      ((pPoint32->G_Y & 0x3) <<  u16LSB_bitpos) |
                                      ((((bR_Cr) ? pPoint32->R_Cr : pPoint32->B_Cb) & 0x3) << (u16LSB_bitpos+2));
        }
        else
        {
            pMEMLSB[u16LSB_bytepos] = pMEMLSB[u16LSB_bytepos] & ~(0xF << u16LSB_bitpos);
            pMEMLSB[u16LSB_bytepos] = pMEMLSB[u16LSB_bytepos] |
                                      ((pPoint24->G_Y & 0x3) <<  u16LSB_bitpos) |
                                      ((((bR_Cr) ? pPoint24->R_Cr : pPoint24->B_Cb) & 0x3) << (u16LSB_bitpos+2));
        }
    }
    else    // 444 format
    {
        pMEMYC += u16XOffset * 4;
        if (u16PointSize == 4)
        {
            pMEMYC[0] = (U8)(pPoint32->B_Cb);
            pMEMYC[1] = (U8)((pPoint32->G_Y << 2) | (pPoint32->B_Cb>> 8));
            pMEMYC[2] = (U8)((pPoint32->R_Cr<< 4) | (pPoint32->G_Y >> 6));
            pMEMYC[3] = (U8)(pPoint32->R_Cr >> 4);
        }
        else
        {
            pMEMYC[0] = (pPoint24->B_Cb<< 2);
            pMEMYC[1] = (pPoint24->G_Y << 4) | (pPoint24->B_Cb>> 6);
            pMEMYC[2] = (pPoint24->R_Cr<< 6) | (pPoint24->G_Y >> 4);
            pMEMYC[3] = (pPoint24->R_Cr>> 2);
        }
    }
}

void MDrv_SC_SetFrameData(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    U8 *pRect;
    U32 u32RectSize;
    SC_FRAMEDATA_t param;
    PSC_SOURCE_INFO_t pSrcInfo;
    SC_FRAMEDATA_t *parg = (SC_FRAMEDATA_t*)arg;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_FRAMEDATA_t)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];

    u32RectSize = param.u32RectPitch * param.height * param.u16PointSize;
    pRect = kmalloc(u32RectSize, GFP_KERNEL);
    if (!pRect)
    {
        return;
    }
    if (copy_from_user(pRect, (void *)param.pRect, u32RectSize))
    {
        return;
    }

    MDrv_SC_SetFrameDataCore(&param.x0, &param.y0, &param.width, &param.height,
                             &param.bRGB,
                             pRect,
                             param.u32RectPitch,
                             u32RectSize,
                             param.u16PointSize,
                             TRUE);

    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    copy_to_user(parg->pRect, pRect, u32RectSize);
    copy_to_user(&(parg->x0),     &param.x0,     sizeof(param.x0));
    copy_to_user(&(parg->y0),     &param.y0,     sizeof(param.y0));
    copy_to_user(&(parg->width),  &param.width,  sizeof(param.width));
    copy_to_user(&(parg->height), &param.height, sizeof(param.height));
    copy_to_user(&(parg->bRGB),   &param.bRGB,   sizeof(param.bRGB));
    kfree(pRect);

}


void MDrv_SC_SetFrameDataCore(U16 *pX0, U16 *pY0, U16 *pWidth, U16 *pHeight,
                              BOOL *pRGB,
                              U8 *pRect,
                              U32 u32RectPitch,
                              U32 u32RectSize,
                              U16 u16PointSize,
                              BOOL bUseDispCoordinate)
{
    U16 x0 = *pX0,  y0 = *pY0, w = *pWidth, h = *pHeight;
    S16 s16dx, s16dy;   // 20090921 daniel.huang: for mirror mode
    U16 xPoint, yPoint; // 20090921 daniel.huang: for mirror mode
    U8 *pPoint;
    volatile U8 *pMEM, *pMEMLSB;    // 20090930 daniel.huang: refine and add LSB for GrabPixel
    U8 u8BytePerPixel;
    U8 u8LSB_YM4_Size;
    U16 x, y;
    U16 u16OffsetPixelAlign;
    U32 u32Y_Offset, u32X_Offset, u32X_LSB_Offset;  // 20090930 daniel.huang: refine and add LSB for GrabPixel
    SC_FRAMEBUF_INFO_t FBInfo;

    U32 u32MIPS_MIU_Base;
    U32 u32SC_MemAddr, u32SC_MemSize;
    U32 u32MIU1_MemAddr, u32MIU1_MemSize;
    U32 u32XYOffset;

    MDrv_SYS_GetMMAP(E_SYS_MMAP_SCALER_DNR_BUF, &u32SC_MemAddr, &u32SC_MemSize);
    MDrv_SYS_GetMMAP(E_SYS_MMAP_MIU1_BASE, &u32MIU1_MemAddr, &u32MIU1_MemSize);
    if(u32SC_MemAddr >= u32MIU1_MemAddr)
        u32MIPS_MIU_Base = MIPS_MIU1_BASE;
    else
        u32MIPS_MIU_Base = MIPS_MIU0_BASE;

    MHal_SC_GetFrameBufInfo(&FBInfo);

    if (FBInfo.bMemFormat422 == TRUE)
    {
        if (FBInfo.u8BitPerPixel == 24)
        {
            FB_DBG(printk("422_10BIT\n"));
            u8BytePerPixel = 3;
            u8LSB_YM4_Size = BYTE_PER_WORD;     // 20100116 daniel.huang: fix linear mode address incorrect
            u16OffsetPixelAlign = BYTE_PER_WORD/2*2; // *2 (F0 F0)
        }
        else //if (FBInfo.u8BitPerPixel == 16)
        {
            FB_DBG(printk("422_8BIT\n"));
            u8BytePerPixel = 2;
            u8LSB_YM4_Size = 0;
            u16OffsetPixelAlign = BYTE_PER_WORD/2*2; // *2 (F0 F0)
        }
    }
    else
    {   // using 444 10 bit
        FB_DBG(printk("444_10BIT\n"));
        u8BytePerPixel = 4;
        u8LSB_YM4_Size = 0;
        u16OffsetPixelAlign = BYTE_PER_WORD/4*2; // *2 (F0 F0)
    }
    // 20100116 daniel.huang: fix linear mode address incorrect
    u8LSB_YM4_Size = u8LSB_YM4_Size * (FBInfo.bLinearAddrMode ? 1: 2);

    // 20090921 daniel.huang: for mirror mode
    if (FBInfo.bh_mirror)
    {
        s16dx = -1;
    }
    else
    {
        s16dx = 1;
    }
    if (FBInfo.bv_mirror)
    {
        if (FBInfo.bInterlace)
        {
            FBInfo.u32IPMBase0 -= ((U32)(FBInfo.u16FrameLineCnt-2) * FBInfo.u16IPMOffset * u8BytePerPixel);
            FBInfo.u32IPMBase1 -= ((U32)(FBInfo.u16FrameLineCnt-2) * FBInfo.u16IPMOffset * u8BytePerPixel);
        }
        else
        {
            if (FBInfo.bLinearAddrMode)
            {
                FBInfo.u32IPMBase0 -= ((U32)(FBInfo.u16FrameLineCnt-1) * FBInfo.u16IPMOffset * u8BytePerPixel);
            }
            else
            {
                FBInfo.u32IPMBase0 -= ((U32)(FBInfo.u16FrameLineCnt-1)* 2 * FBInfo.u16IPMOffset * u8BytePerPixel);
            }
        }
        s16dy = -1;
    }
    else
    {
        s16dy = 1;
    }

    if (bUseDispCoordinate)
    {
        // 2009/09/23 daniel.huang: for calibration with mirror
        if (FBInfo.bh_mirror)
        {
            x0 = MHal_SC_VOP_GetDispHEnd() - MHal_SC_VOP_GetDispHStart() - x0;
            FB_DBG(printk("mirror: disp x0=%u\n", x0));
        }
        if (FBInfo.bv_mirror)
        {
            y0 = MHal_SC_VOP_GetDispVEnd() - MHal_SC_VOP_GetDispVStart() - y0;
            FB_DBG(printk("mirror: disp y0=%u\n", y0));
        }
    }
    else
    {
        // 2009/09/23 daniel.huang: for calibration with mirror
        if (FBInfo.bh_mirror)
        {
            x0 = FBInfo.u16IPMFetch - 1 - x0;
            FB_DBG(printk("mirror: crop x0=%u\n", x0));
        }
        if (FBInfo.bv_mirror)
        {
            y0 = FBInfo.u16FrameLineCnt - 1 - y0;
            FB_DBG(printk("mirror: crop y0=%u\n", y0));
        }
    }

    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    // calculate offset between ipmbase & opmbase
    if (bUseDispCoordinate)
    {
        U16 u16tmp;     // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        u32XYOffset = (FBInfo.u32OPMBase0 - FBInfo.u32IPMBase0) / (u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2));
        x = u32XYOffset % FBInfo.u16IPMOffset;
        y = u32XYOffset / FBInfo.u16IPMOffset;
        if (FBInfo.bInterlace) y *= 2;

        FB_DBG(printk("ipmbase0=0x%x, opmbase0=0x%x, bpp=%u, li=%u\n", FBInfo.u32IPMBase0, FBInfo.u32OPMBase0, u8BytePerPixel, FBInfo.bLinearAddrMode));
        FB_DBG(printk("offset between ipmbase & opmbase: x=%u, y=%u\n", x, y));

        // convert disp coordinate to crop coordinate
        FB_DBG(printk("disp coordinate: x0=%u, y0=%u, w=%u, h=%u\n", x0, y0, w, h));
        x0 = MHal_SC_OP1_GetHCropCoordiante(x0, TRUE);
        y0 = MHal_SC_OP1_GetVCropCoordiante(y0, TRUE);
        u16tmp = MHal_SC_OP1_GetHCropCoordiante(w, FALSE);  // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        if (u16tmp < w) w = u16tmp;                         // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        u16tmp = MHal_SC_OP1_GetVCropCoordiante(h, FALSE);  // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        if (u16tmp < h) h = u16tmp;                         // 20100118 daniel.huang: fix WXGA grab pixel reponse memory w/h size > display window w/h size
        FB_DBG(printk("crop coordinate: x0=%u, y0=%u, w=%u, h=%u\n", x0, y0, w, h));

        x0 = x0 + x + FBInfo.u16LBOffset;
        y0 = y0 + y;
        FB_DBG(printk("memory coordinate: x0=%u, y0=%u, w=%u, h=%u\n", x0, y0, w, h));
    }

    // 20090922 daniel.huang: for memory protection
    if (FBInfo.bh_mirror)
        w = ((x0+1) < w) ? (x0+1) : w;
    else
        w = ((x0+w) > FBInfo.u16IPMFetch) ? (FBInfo.u16IPMFetch-x0) : w;
    // 20090922 daniel.huang: for memory protection
    if (FBInfo.bv_mirror)
        h = ((y0+1) < h) ? (y0+1) : h;
    else
        h = ((y0+h) > FBInfo.u16FrameLineCnt) ? (FBInfo.u16FrameLineCnt-y0) : h;

    // 20090921 daniel.huang: for mirror mode
    yPoint = 0;
    for (y = y0; y != (U16)(y0+h*s16dy); y=y+s16dy)
    {
        if (FBInfo.bInterlace)
        {
            u32Y_Offset = (U32)y/2 * FBInfo.u16IPMOffset * u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2);
            if (y & 0x1)
                u32Y_Offset += FBInfo.u32IPMBase1;
            else
                u32Y_Offset += FBInfo.u32IPMBase0;
        }
        else
        {
            u32Y_Offset = (U32)y * FBInfo.u16IPMOffset * u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2);
            u32Y_Offset += FBInfo.u32IPMBase0;
        }
        FB_DBG(printk("***(%u) Yaddress=0x%x, u32Y_Offset:0x%x, u16IPMOffset:%u, y:%u, u8BytePerPixel:%u, linear multiply:%u\n",
            y, u32MIPS_MIU_Base + u32Y_Offset, (int)u32Y_Offset, FBInfo.u16IPMOffset, y, u8BytePerPixel, (FBInfo.bLinearAddrMode ? 1: 2)));

        // 20090921 daniel.huang: for mirror mode
        xPoint = 0;
        for (x = x0; x != (U16)(x0+w*s16dx); x=x+s16dx)
        {
            u32X_Offset = (U32)x / u16OffsetPixelAlign * u16OffsetPixelAlign * u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2);
            // 20090930 daniel.huang: refine and add LSB for GrabPixel
            u32X_LSB_Offset = (U32)x / (u16OffsetPixelAlign*2) * (u16OffsetPixelAlign*2) * u8BytePerPixel * (FBInfo.bLinearAddrMode ? 1: 2)
                + BYTE_PER_WORD * (FBInfo.bLinearAddrMode ? 2: 4);  // 20100116 daniel.huang: fix linear mode address incorrect

            //FB_DBG(printk("[2]u32X_Offset=0x%x, u16OffsetPixelAlign=%u, x=%u, align(x)=%u\n",
            //    (int)u32X_Offset, u16OffsetPixelAlign, x, x / u16OffsetPixelAlign * u16OffsetPixelAlign));

            // 20090921 daniel.huang: for mirror mode
            pPoint = pRect+ (u32RectPitch * yPoint + xPoint) * u16PointSize;
            if ((x / u16OffsetPixelAlign) & 0x1)
            {
                pMEM = (volatile U8 *)(u32MIPS_MIU_Base + u32Y_Offset + u32X_Offset + u8LSB_YM4_Size);
            }
            else
            {
                pMEM = (volatile U8 *)(u32MIPS_MIU_Base + u32Y_Offset + u32X_Offset);
            }
            pMEMLSB = (volatile U8 *)(u32MIPS_MIU_Base + u32Y_Offset + u32X_LSB_Offset);    // 20090930 daniel.huang: refine and add LSB for GrabPixel

            FB_DBG(printk("***(%u,%u) Y/C address: 0x%x, LSB address: 0x%x\n", x, y, (U32)pMEM, (U32)pMEMLSB));

            // 20090930 daniel.huang: refine and add LSB for GrabPixel
            _MDrv_SC_WritePixel(pMEM,
                                pMEMLSB,
                                x,
                                u16OffsetPixelAlign,
                                FBInfo.bMemFormat422,
                                pPoint,
                                u16PointSize,
                                (x&0x1),
                                FBInfo.bYCSeperate);
            xPoint++;   // 20090921 daniel.huang: for mirror mode
        }
        yPoint++;       // 20090921 daniel.huang: for mirror mode
    }

    MDrv_SYS_Flush_Memory();

    // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    *pX0 = x0;
    *pY0 = y0;
    *pWidth = w;
    *pHeight = h;
    *pRGB = MHal_SC_VIP_GetCSC();
}

void MDrv_SC_SetFBL(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    BOOL param;
    PSC_SOURCE_INFO_t pSrcInfo;
    if (copy_from_user(&param, (void __user *)arg, sizeof(BOOL)))
    {
        return;
    }
    pSrcInfo = &pDrvCtx->SrcInfo[0];
    pSrcInfo->bFBL = param;
}

/////////////////////////////////////////////////////////////
//
// Decide OutputVFreq and FRC ratio
//
/////////////////////////////////////////////////////////////
static U8 MDrv_SC_LPLL_DecideFRC(U16 u16InputVFreq, U16 *u16OutputVFreq,
                       PSC_SOURCE_INFO_t pSrcInfo)
{
    U8 u8FrameLock;
    U8 u8FrcIn = 1, u8FrcOut = 1;
    S16 u16Delta50 = 100, u16Delta60 = 100;
    U16 u16DiffOf50, u16DiffOf60;
    U16 u16VFreq = 0;
    U8 in, out;
    BOOLEAN bIsDone = FALSE;

    //LGE [vivakjh] 2008/12/07  PDP 800x600@58Hz 에서 영상무 문제 해결을 위해 삽입.
    if (MDrv_SC_IsPDPPanel())
    {
        if (u16InputVFreq > 550 && u16InputVFreq < 590)
        {
            *u16OutputVFreq = 600;  u8FrcIn  = 1;  u8FrcOut = 1;
            u8FrameLock = FRAME_LOCK_60HZ;
            goto SETFRC;
        }
        if (QM_IsSourceHDMI(pSrcInfo) && (u16InputVFreq > 230 && u16InputVFreq < 250))
        {
            *u16OutputVFreq = 480;  u8FrcIn  = 1;  u8FrcOut = 1;
            u8FrameLock = FRAME_LOCK_50HZ;
            goto SETFRC;
        }
    }
    else
    {
        if ((u16InputVFreq > 230 && u16InputVFreq < 250))
        {
            if (_gFrameTo48Hz)
            {
                *u16OutputVFreq = 480;  u8FrcIn  = 1;  u8FrcOut = 1;
                u8FrameLock = FRAME_LOCK_50HZ;
                goto SETFRC;
            }
        }
        else
            _gFrameTo48Hz = FALSE;
    }


    for(in=1; in<=2; in++)
    {
        for(out=1; out<=5; out++)
        {
            u16VFreq = u16InputVFreq * out / in;
            u16DiffOf50 = abs(500 - abs(u16VFreq));
            u16DiffOf60 = abs(600 - abs(u16VFreq));

            if ( u16DiffOf50 <= u16DiffOf60 && u16DiffOf50 < u16Delta50)
            {
                u16Delta50 = u16DiffOf50;
                if(u16Delta50 <= 10)
                {
                    u8FrameLock = FRAME_LOCK_50HZ;
                    u8FrcIn = in; u8FrcOut = out;
                    FL_DBG("FRC 50Hz In:Out[%d:%d]\n", in, out);
                    bIsDone = TRUE;
                    break;
                }
            }
            else if( u16DiffOf60 < u16DiffOf50 && u16DiffOf60 < u16Delta60)
            {
                u16Delta60 = u16DiffOf60;
                if(u16Delta60 <= 10)
                {
                    u8FrameLock = FRAME_LOCK_60HZ;
                    u8FrcIn = in; u8FrcOut = out;
                    FL_DBG("FRC 60Hz In:Out[%d:%d]\n", in, out);
                    bIsDone = TRUE;
                    break;
                }
            }
            else
            {;}
            u8FrameLock = FREE_RUN;
        }
        if(bIsDone) break;
    }

    *u16OutputVFreq = u16InputVFreq * u8FrcOut / u8FrcIn;

    if (!MDrv_SC_IsPDPPanel())
    {
        //lachesis_090108 50Hz 인가후 70Hz 인가 시 flicker
        //60Hz 이상일 때는 60Hz로 설정하도록 함.
        if(u8FrameLock == FREE_RUN)
        {
			/*
			in case of 29Hz, 56Hz output freq will be over panel spec(50 or 60 +-1Hz)
			so search freerun output freq again, and freerun to 50 or 60Hz
			*/
            //lachesis_090114 예외 경우.
            if(*u16OutputVFreq > 610)
            {
                FL_DBG("===> u16OutputVFreq is over 610 force lock to 60Hz======= \n");
                *u16OutputVFreq = 600;
            }
			else
			{
			    u16Delta50 = u16Delta60 = 100;
				for(out=1; out<=2; out++)
				{
					u16VFreq = u16InputVFreq * out;
					u16DiffOf50 = abs(500 - abs(u16VFreq));
					u16DiffOf60 = abs(600 - abs(u16VFreq));

					if ( u16DiffOf50 <= u16DiffOf60 && u16DiffOf50 < u16Delta50)
					{
						u16Delta50 = u16DiffOf50;
						if(u16Delta50 <= 50)
						{
							FL_DBG("FRC 50Hz In:Out[%d:%d]\n", 1, out);

							break;
						}
					}
					else if( u16DiffOf60 < u16DiffOf50 && u16DiffOf60 < u16Delta60)
					{
						u16Delta60 = u16DiffOf60;
						if(u16Delta60 <= 50)
						{
							FL_DBG("FRC 60Hz In:Out[%d:%d]\n", 1, out);

							break;
						}
					}
					else
					{;}
				}

				*u16OutputVFreq = u16InputVFreq * out;

				if(*u16OutputVFreq < 550)
				{
					FL_DBG("===========force lock to 50Hz======= \n");
					*u16OutputVFreq = 500;
				}
				else
				{
					FL_DBG("===========force lock to 60Hz======= \n");
					*u16OutputVFreq = 600;
				}
			}
        }
    }

SETFRC:

    FL_DBG(" OutputVFreq[%d] = InputVFreq[%d] * FrcOut[%d] / FrcIn[%d]\n", *u16OutputVFreq, u16InputVFreq, u8FrcOut, u8FrcIn);
    MHal_LPLL_SetFrameSync(u8FrcIn, u8FrcOut);

    return u8FrameLock;
}


/////////////////////////////////////////////////////////////
//
// Decide H/VTotal & lock/freeze point
//
/////////////////////////////////////////////////////////////
static void MDrv_SC_LPLL_DecideHVTotal(U16 *u16HTotal, U16 *u16VTotal,
                                U16 u16OutputVFreq,
                                PSC_SOURCE_INFO_t pSrcInfo,
                                PMST_PANEL_INFO_t pPanelInfo)
{
    BOOL bFBL, bInterlace;
    U16 u16Vde_in, u16Vde_out, u16Vtt_in, u16Vtt_out;
    U16 u16Offset;
    U16 u16LockPoint;
    U16 u16FreezePoint;
    U16 u16TempVTotal = 0, u16TempVStart = 0, u16TempVBackPorch = 0;    //LGE [vivakjh] 2008/11/12  Add for DVB PDP Panel
    HAL_SC_OUTPUTSYNC_MODE_e eTempOutSyncMode = SC_OUTPUTSYNC_MODE_1;   // [vivakjh] 2009/01/07 PDP FHD MRE(FMC) 대응.

    bFBL = MHal_SC_IPM_GetFBL();
    bInterlace = (pSrcInfo->u8Input_SyncStatus & SC_SYNCSTS_INTERLACE_BIT)?TRUE:FALSE;

    if (bFBL)
    {
        //calculate Vtt_out for FBL
        //Vtt_out = Vtt_in * Vde_out / Vde_in
        u16Vde_in = MHal_SC_IP1_GetVDC();
        u16Vtt_in = MHal_SC_IP1_GetVTotal();

        u16Vde_out = MHal_SC_VOP_GetDispVEnd() - MHal_SC_VOP_GetDispVStart() + 1;
        u16Vtt_out = (U16)(((U32)u16Vtt_in * u16Vde_out) / u16Vde_in);

        FL_DBG("[FPLL]Vde_in= 0x%X (%u)\n", u16Vde_in, u16Vde_in);
        FL_DBG("[FPLL]Vde_out= 0x%X (%u)\n", u16Vde_out, u16Vde_out);
        FL_DBG("[FPLL]Vtt_in= 0x%X (%u)\n", u16Vtt_in, u16Vtt_in);

        MHal_SC_VOP_HVTotalSet(pPanelInfo->u16HTotal, u16Vtt_out);

        // calculate lock point and freeze point
        if(bInterlace)
        {
            u16Vde_in /= 2;
        }
        u16Offset = (U16)(((U32)u16Vde_out*6) / (u16Vde_in));
        u16LockPoint = u16Vtt_out - u16Offset;
        u16FreezePoint = u16LockPoint - 2;

    }
    else
    {
        //LGE [vivakjh] 2008/11/12  Add for DVB PDP Panel
        if(pPanelInfo->u8LCDorPDP == 1)    // PDP Only
        {
            if (u16OutputVFreq != 0)
            {
                if (u16OutputVFreq >= 470 && u16OutputVFreq < 490)
                {
                    u16TempVTotal = pPanelInfo->u16VTotal48Hz;
                    u16TempVStart = pPanelInfo->u16VStart48Hz;         //[vivakjh] 2008/12/23  Modified for adjusting the MRE in PDP S6
                    u16TempVBackPorch = pPanelInfo->u16VBackPorch48Hz; //[vivakjh] 2008/12/23  Modified for adjusting the MRE in PDP S6
                    MHal_SC_VOP_SetAutoVSyncCtrl(FALSE);    // [vivakjh] 2009/01/07 PDP FHD MRE(FMC) 대응.
                }
                else if (u16OutputVFreq >= 490 && u16OutputVFreq <= 520)
                {
                    u16TempVTotal = pPanelInfo->u16VTotal50Hz;//lachesis_090717
                    u16TempVStart = pPanelInfo->u16VStart50Hz;//lachesis_090717
                    u16TempVBackPorch = pPanelInfo->u16VBackPorch50Hz; //[vivakjh] 2008/12/23  Modified for adjusting the MRE in PDP S6
                    MHal_SC_VOP_SetAutoVSyncCtrl(FALSE);    // [vivakjh] 2009/01/07 PDP FHD MRE(FMC) 대응.
                }
                else
                {
                    u16TempVTotal = pPanelInfo->u16VTotal60Hz;
                    u16TempVStart = pPanelInfo->u16VStart60Hz;         //[vivakjh] 2008/12/23  Modified for adjusting the MRE in PDP S6
                    u16TempVBackPorch = pPanelInfo->u16VBackPorch60Hz; //[vivakjh] 2008/12/23  Modified for adjusting the MRE in PDP S6
                    MHal_SC_VOP_SetAutoVSyncCtrl(FALSE);	//[vivakjh] 2009/09/04	PDP Panel Timing Manual로 변경.
                }
                if (u16OutputVFreq != _u16PrevOutputVFreq)
                {
                    MHal_SC_VOP_HVTotalSet(pPanelInfo->u16HTotal, u16TempVTotal);
                    // [vivakjh] 2009/01/07 PDP FHD MRE(FMC) 대응.
                    if (Use_VGA_Source(pSrcInfo->SrcType) || (Use_HDMI_Source(pSrcInfo->SrcType) && (pSrcInfo->bHDMIMode == FALSE)))
                        eTempOutSyncMode = SC_OUTPUTSYNC_MODE_4;
                    else
                        eTempOutSyncMode = SC_OUTPUTSYNC_MODE_3;

                    MHal_SC_VOP_SetOutputSycCtrl(eTempOutSyncMode, u16TempVTotal - u16TempVStart, u16TempVTotal - u16TempVBackPorch);  //[vivakjh] 2008/12/23   Modified for adjusting the MRE in PDP S6
                    _u16PrevOutputVFreq = u16OutputVFreq;
                    FL_DBG("[MDrv_SC_CheckFrameLock] >>>> u16PrevOutputVFreq : %d\n", _u16PrevOutputVFreq);
                }
            }
        }
// shjang_090904
        else if (pPanelInfo->u8LCDorPDP == 3)
        {
            if (u16OutputVFreq >= 470 && u16OutputVFreq < 490)
                u16TempVTotal = pPanelInfo->u16VTotal48Hz;
            else if (u16OutputVFreq >= 490 && u16OutputVFreq <= 520)
                u16TempVTotal = pPanelInfo->u16VTotal50Hz;
            else
                u16TempVTotal = pPanelInfo->u16VTotal60Hz;

            MHal_SC_VOP_HVTotalSet(pPanelInfo->u16HTotal, u16TempVTotal);

            if (Use_VGA_Source(pSrcInfo->SrcType) || (Use_HDMI_Source(pSrcInfo->SrcType) && (pSrcInfo->bHDMIMode == FALSE)))
                eTempOutSyncMode = SC_OUTPUTSYNC_MODE_4;
            else
                eTempOutSyncMode = SC_OUTPUTSYNC_MODE_3;
            MHal_SC_VOP_SetOutputSycCtrl(eTempOutSyncMode,
	            pPanelInfo->u16VTotal - pPanelInfo->u16VStart,
	            pPanelInfo->u16VTotal - pPanelInfo->u16VBackPorch60Hz);

        }
        //lachesis_090109 T2출력은 항상 적용 필요함.
        else //if(pDrvCtx->pPanelInfo->u8LCDorPDP == 2) // for LCD No-FRC models_add balupzillot_081124
        {
            if (u16OutputVFreq != 0)
            {
				//lachesis_090831
                if (u16OutputVFreq >= 470 && u16OutputVFreq < 490)
                {
					u16TempVTotal = pPanelInfo->u16VTotal48Hz;//lachesis_090717
                }
				else if (u16OutputVFreq >= 490 && u16OutputVFreq <= 520)
                {
                    u16TempVTotal = pPanelInfo->u16VTotal50Hz;//lachesis_090717
                }
                else
                {
                    u16TempVTotal = pPanelInfo->u16VTotal60Hz;
                }

                if (u16OutputVFreq != _u16PrevOutputVFreq)
                {
                    MHal_SC_VOP_HVTotalSet(pPanelInfo->u16HTotal, u16TempVTotal);
                    _u16PrevOutputVFreq = u16OutputVFreq;
                    FL_DBG("[MDrv_SC_CheckFrameLock] >>>> u16PrevOutputVFreq : %d\n", _u16PrevOutputVFreq);
                }
            }
        }

        u16FreezePoint = MHal_SC_VOP_GetDispVEnd() - 2 ; //REG_RR(L_BK_VOP(0x0b)) - 2;
        u16LockPoint = u16FreezePoint + 2;
    }

    // set lock point
    MHal_SC_SetLockPoint(u16LockPoint);
    // set freeze point
    MHal_SC_SetFreezePoint(u16FreezePoint);

    *u16HTotal = pPanelInfo->u16HTotal;
    *u16VTotal = u16TempVTotal;

}

/////////////////////////////////////////////////////////////
//
// i/p gain formula
//
/////////////////////////////////////////////////////////////

static U8 _MDrv_SC_LPLL_ConvertGainToReg(U16 u16UsedGain_x32)
{
    if      (u16UsedGain_x32 >=32768) return 15;
    else if (u16UsedGain_x32 >=16384) return 14;
    else if (u16UsedGain_x32 >= 8192) return 13;
    else if (u16UsedGain_x32 >= 4096) return 12;
    else if (u16UsedGain_x32 >= 2048) return 11;
    else if (u16UsedGain_x32 >= 1024) return 10;
    else if (u16UsedGain_x32 >=  512) return 9;
    else if (u16UsedGain_x32 >=  256) return 8;
    else if (u16UsedGain_x32 >=  128) return 7;
    else if (u16UsedGain_x32 >=   64) return 6;
    else if (u16UsedGain_x32 >=   32) return 5;
    else if (u16UsedGain_x32 >=   16) return 4;
    else if (u16UsedGain_x32 >=    8) return 3;
    else if (u16UsedGain_x32 >=    4) return 2;
    else if (u16UsedGain_x32 >=    2) return 1;
    else if (u16UsedGain_x32 >=    1) return 0;
    else {
        assert(0);
        return 0;
    }
}

/////////////////////////////////////////////////////////////
//
// Decide DCLK & IPGain
//
/////////////////////////////////////////////////////////////
static void MDrv_SC_LPLL_DecideDCLK(U16 u16HTotal, U16 u16VTotal, U16 u16OutputVFreq,
                             PMST_PANEL_INFO_t pPanelInfo)
{
    U32 u32OriginalDClk;    // Hz unit
    U64 u64PLLClk;          // Hz unit
    U32 u32DClkFactor;
    U8 u8PGain, u8IGain;
    U32 u32LoopGain;
    U8 u8FrcIn, u8FrcOut;
    U64 u64Div_Factor;      // 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain
    U16 u16UsedGain;        // 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain

    // 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain
    if (pPanelInfo->u8LPLL_Type == LPLL_TYPE_MINILVDS)
    {
        u32DClkFactor = 216*524288*2;
    }
    else
    {
        u32DClkFactor = 216*524288*16;
    }

    FL_DBG(" HTotal[%d] VTotal[%d] OutputVFreq[%d.%d]\n", u16HTotal, u16VTotal, u16OutputVFreq/10, u16OutputVFreq%10);
    FL_DBG(" u32DClkFactor = 0x%x \n", u32DClkFactor);

    if ((u16OutputVFreq < 450 || u16OutputVFreq > 650) || // LGE drmyung 081103 : avoid latch
        (u16HTotal == 0 || u16VTotal == 0) )
    {
        return;
    }


    u32OriginalDClk = ((U32)u16HTotal * u16VTotal * u16OutputVFreq);

    FL_DBG(" u32OriginalDClk = %d.%dMHz\n", u32OriginalDClk/10000000, u32OriginalDClk%10000000);

    // LGE [vivakjh] 2009/01/21 Modified the PDP module flicker problem after playing some perticular movie files that are over the PDP module margin.
    if (pPanelInfo->u8LCDorPDP == 1)   // PDP
    {
        if (u32OriginalDClk > 1494000000)
        {
            FL_DBG("[[ vivakjh ]] >> TRUE : u32OriginalDClk = %d \n", u32OriginalDClk);
            _gbIsPDPMarginOver = TRUE;
            MHal_LPLL_SetFrameLockSpeedToZero(TRUE);
        }
        else
        {
            _gbIsPDPMarginOver = FALSE;
            MHal_LPLL_SetFrameLockSpeedToZero(FALSE);
        }
    }

    // 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain
    u64PLLClk = u32DClkFactor;
    u64PLLClk *= 10000000;
    do_div(u64PLLClk, u32OriginalDClk);
    if ((pPanelInfo->u8LPLL_Type == LPLL_TYPE_LVDS) || (pPanelInfo->u8LPLL_Type == LPLL_TYPE_TTL))
    {
        do_div(u64PLLClk, 7);
        if (pPanelInfo->u8LPLL_Mode == LPLL_MODE_DUAL)
        {
            u64PLLClk *= 2;
        }
    }
    else if (pPanelInfo->u8LPLL_Type == LPLL_TYPE_RSDS)
    {
        do_div(u64PLLClk, 4);
    }
    else if (pPanelInfo->u8LPLL_Type == LPLL_TYPE_MINILVDS)
    {
    }
    else
    {   // unsuported type
        assert(0);
    }
    if(u64PLLClk >= 0xffffff)
    {
        FL_DBG("PLL calculating Error u64PLLClk = 0x%x%x\n", (U32)(u64PLLClk>>32), (U32)u64PLLClk);
    }
    FL_DBG(" u64PLLClk = %d \n", (U32)u64PLLClk);

    if ((pPanelInfo->u8LPLL_Type == LPLL_TYPE_LVDS) || (pPanelInfo->u8LPLL_Type == LPLL_TYPE_TTL))
    {   // pPanelInfo->u16LPLL_LoopDiv value, eg: 0x0203
        switch(pPanelInfo->u16LPLL_LoopDiv & 0x3)
        {
        case 0:     u32LoopGain = 1; break;
        case 1:     u32LoopGain = 2; break;
        case 2:     u32LoopGain = 4; break;
        default:    u32LoopGain = 8; break;
        }
        u32LoopGain *= (pPanelInfo->u16LPLL_LoopDiv >> 8) ? (pPanelInfo->u16LPLL_LoopDiv >> 8) : 1;
    }
    else
    {
        u32LoopGain = 2;
    }
    u64Div_Factor = 216/12*524288*u32LoopGain;
    u64Div_Factor *= 32; // *32 for add 5 bit for computing i/p gain

    MHal_LPLL_GetFrameSync(&u8FrcIn, &u8FrcOut);
    //u64Div_Factor /= (8*u16HTotal * u16VTotal * (u8ovs_div+1));   // u8FrcOut = u8ovs_div+1
    do_div(u64Div_Factor, (8*u16HTotal * u16VTotal * u8FrcOut));
    if ((pPanelInfo->u8LPLL_Type == LPLL_TYPE_LVDS) || (pPanelInfo->u8LPLL_Type == LPLL_TYPE_TTL))
    {
        do_div(u64Div_Factor, 7);
        if (pPanelInfo->u8LPLL_Mode == LPLL_MODE_DUAL)
        {
            u64Div_Factor *= 2;
        }
    }
    u16UsedGain = (U16)u64Div_Factor;
    u8IGain = _MDrv_SC_LPLL_ConvertGainToReg(u16UsedGain);
    u8PGain = _MDrv_SC_LPLL_ConvertGainToReg(u16UsedGain*2);
    FL_DBG("Used IGain[%u], IGain[%u], PGain[%u]\n", u16UsedGain, u8IGain, u8PGain);

    MHal_LPLL_SetIPGain(u8IGain, u8PGain);
    MHal_LPLL_LPLLSET((U32)u64PLLClk);
}


/////////////////////////////////////////////////////////////
//
// MDrv_SC_SetPanelTiming
// input -  pDrvCtx: common driver context
//          bInputSourceEnable: TRUE, input source is enabled
//                             FALSE, input source is disabled
// return - output vfrequency
//
/////////////////////////////////////////////////////////////

U16 MDrv_SC_SetPanelTiming(PSC_DRIVER_CONTEXT_t pDrvCtx, U8 bInputSourceEnable, BOOL bFreeRun)
{
    static U16 u16OutputVFreq = FREERUN_VFREQ;
    U16 u16HTotal, u16VTotal;
    PSC_SOURCE_INFO_t pSrcInfo;
    PMST_PANEL_INFO_t pPanelInfo;
    FRAME_LOCK_e u8FrameLock = FREE_RUN;

    pSrcInfo = pDrvCtx->SrcInfo;
    pPanelInfo = pDrvCtx->pPanelInfo;

	//lachesis_090827
    //MHal_SC_IP1_SetInputSourceEnable(FALSE, FALSE);
    //MHal_LPLL_EnableFPLL(DISABLE);

    // Decide OutputVFreq and FRC
    if (pSrcInfo->u16Input_VFreq > 0)
    {
        //20090812 Daniel Huang: prevent AP not initialize Input_VFreq(=0)
        //1. when sync exsist find FRC in-out ratio and output freq
        u8FrameLock = MDrv_SC_LPLL_DecideFRC(pSrcInfo->u16Input_VFreq, &u16OutputVFreq, pSrcInfo);
    }
    SC_FPLL_DBG("####### inputsrc en=%u, in vfreq=%u, out vfreq=%u\n",
           bInputSourceEnable, pSrcInfo->u16Input_VFreq, u16OutputVFreq);

    // decide HVTotal and ODCLK
    MDrv_SC_LPLL_DecideHVTotal(&u16HTotal, &u16VTotal, u16OutputVFreq,
                               pSrcInfo, pPanelInfo);
    SC_FPLL_DBG("####### u16HTotal =%u, u16VTotal=%u\n",u16HTotal, u16VTotal);

    // decide dclk and I/PGain
    MDrv_SC_LPLL_DecideDCLK(u16HTotal, u16VTotal, u16OutputVFreq, pPanelInfo);

	if(bIsForceFreeRun)	//Force Free Run일 경우 설정하면 안됨. ex) Auto Scan...
	{
		MDrv_SC_SetFrameLock(pDrvCtx, FALSE);
        MHal_SC_IP1_SetInputSourceEnable(TRUE, FALSE);
		u8FrameLock = FREE_RUN;
        SC_FPLL_DBG("force freerun(o)\n");
        FL_DBG("\n Force Free Run ==> FORCE FREE RUN %dHz\n", u16OutputVFreq/10);
	}
	else
	{
		if (bInputSourceEnable == TRUE)
		{
			//20090812 Daniel Huang: prevent AP not initialize Input_VFreq(=0)
			//	  behavior: still set input source enable but freerun
			if (pSrcInfo->u16Input_VFreq == 0)
			{
				assert(0);
				MHal_SC_IP1_SetInputSourceEnable(FALSE, FALSE);
				MDrv_SC_SetFrameLock(pDrvCtx, FALSE);
				FL_DBG("\n input v freq=0 FRAMELOCK FALSE input source disable %dHz\n", u16OutputVFreq/10);
			}
			else
			{
				if(u8FrameLock == FREE_RUN)//sync exsist but over freq (ex input freq is 70hz)
				{
					MDrv_SC_SetFrameLock(pDrvCtx, FALSE);
					MHal_SC_IP1_SetInputSourceEnable(TRUE, FALSE);
					FL_DBG("\n SetInputSourceEnable ==> FREE RUN %dHz\n", u16OutputVFreq/10);
				}
				else
				{
					MHal_SC_IP1_SetInputSourceEnable(TRUE, TRUE); // FitchHsu 20090604 SpeedUp frame lock for TW model in YPbPr and HDMI
					MDrv_SC_SetFrameLock(pDrvCtx, TRUE);
					FL_DBG("\n SetInputSourceEnable ==> FRAME LOCK %dHz\n", u16OutputVFreq/10);
				}
			}
			//MHal_LPLL_Measure_FrameLockSpeed();
		}
		else //(bInputSourceEnable == FALSE)
		{
			MDrv_SC_SetFrameLock(pDrvCtx, FALSE);


			if (Use_ATV_Source(pSrcInfo->SrcType))
			{
				FL_DBG("Use_ATV_Source src\n");
				FL_DBG("\n SetInputSource enable ==> FREE RUN	%dHz\n", u16OutputVFreq/10);
				MHal_SC_IP1_SetInputSourceEnable(TRUE, FALSE);
			}
			else
			{
				MHal_SC_IP1_SetInputSourceEnable(FALSE, FALSE);
				FL_DBG("\n SetInputSource Disable ==> FREE RUN	%dHz\n", u16OutputVFreq/10);
			}

			u8FrameLock = FREE_RUN;
		}
	}

    // 20090822 FitchHsu fix Ursa ATV snow noise frame lock
    pSrcInfo->u8FrameLock = u8FrameLock;
    pSrcInfo->u16OutputVFreq = u16OutputVFreq;
    return u16OutputVFreq;
}

// FitchHsu 20080811 implement LPLL type
void MDrv_SC_Set_LPLL(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
	SC_SET_LPLL_TYPE_t param;

	if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_LPLL_TYPE_t)))
	{
	    return;
	}
    pDrvCtx->pPanelInfo->u8LPLL_Type = param.e_lpll_type;
	MHal_MOD_Set_LPLL(pDrvCtx->pPanelInfo);
}

//------------------------------------------------------------------------------
// VIDEO MIRROR
// Michu 20090903
//------------------------------------------------------------------------------
void MDrv_SC_Set_VideoMirror(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    U32 u32WritelimitBase;
    SC_SET_MIRROR_TYPE_t param;
    PSC_SOURCE_INFO_t pSrcInfo;

	if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_MIRROR_TYPE_t)))
	{
	    return;
	}

    pSrcInfo = &pDrvCtx->SrcInfo[param.srcIdx];
    pSrcInfo->bHMirror = param.bIsH;
    pSrcInfo->bVMirror = param.bIsV;

    if (( param.bIsH && !param.bIsV ) || ( !param.bIsH && !param.bIsV ))
    {
        u32WritelimitBase = (((pDrvCtx->u32MemAddr + pDrvCtx->u32MemSize) / BYTE_PER_WORD) - 1) | 0x02000000;
        //printk("==========> pSrcInfo->bVMirror = %d, WritelimitBase = 0x%x\n", pSrcInfo->bVMirror, (U32)u32WritelimitBase);
    }
    else
    {
        u32WritelimitBase = (pDrvCtx->u32MemAddr / BYTE_PER_WORD) | 0x2000000 | 0x1000000;
        //printk("==========> pSrcInfo->bVMirror = %d, WritelimitBase = 0x%x\n", pSrcInfo->bVMirror, (U32)u32WritelimitBase);
    }

    MHal_SC_IPM_SetDNRWriteLimit( u32WritelimitBase );
    MHal_SC_Set_VideoMirror( param.bIsH, param.bIsV );
}
//------------------------------------------------------------------------------
// End of VIDEO MIRROR
//------------------------------------------------------------------------------

// 20091021 daniel.huang: add ipmux test pattern for inner test pattern
void MDrv_SC_IPMUX_SetTestPattern(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_IPMUX_PATTERN_t param;
    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_IPMUX_PATTERN_t)))
    {
        return;
    }

    //printk("[1]R_Cr=%u, G_Y=%u, B_Cb=%u\n", param.u16R_Cr, param.u16G_Y, param.u16B_Cb);

    if (!MHal_SC_VIP_GetCSC() && !MHal_SC_IP2_GetCSC()) // input is YUV
    {
        //Y  =  0.257R + 0.504G + 0.098B + 16
        //Cb = -0.148R - 0.291G + 0.439B + 128
        //Cr =  0.439R - 0.368G - 0.071B + 128
        S32 s32Y, s32Cb, s32Cr;
        s32Y  = ( (S32)param.u16R_Cr * 257 + (S32)param.u16G_Y * 504 + (S32)param.u16B_Cb *  98) / 1000 + (16 << 2);
        s32Cb = (-(S32)param.u16R_Cr * 148 - (S32)param.u16G_Y * 291 + (S32)param.u16B_Cb * 439) / 1000 + (128<< 2);
        s32Cr = ( (S32)param.u16R_Cr * 439 - (S32)param.u16G_Y * 368 - (S32)param.u16B_Cb *  71) / 1000 + (128<< 2);

        param.u16R_Cr = (U16)s32Cr;
        param.u16G_Y  = (U16)s32Y;
        param.u16B_Cb = (U16)s32Cb;
        //printk("[2]R_Cr=%u, G_Y=%u, B_Cb=%u\n", param.u16R_Cr, param.u16G_Y, param.u16B_Cb);
    }

    MDrv_SC_IP1_WaitOutputVSync(1,50);
    MHal_SC_IPMUX_SetTestPattern(param.bEnable, param.u16R_Cr, param.u16G_Y, param.u16B_Cb);
}

// Michu 20091026, OD
void MDrv_SC_Set_ODInitial(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_OD_TABLE_t param;
    U32 u32ODmstMemAddr, u32ODmstMemSize, u32ODlstMemAddr, u32ODlstMemSize;
    U32 OD_MSB_START_ADDR, OD_MSB_LIMIT_ADDR, OD_LSB_START_ADDR, OD_LSB_LIMIT_ADDR;

	if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_OD_TABLE_t)))
	{
	    return;
	}

    MDrv_SYS_GetMMAP(E_SYS_MMAP_OD_MSB_BUFFER, &u32ODmstMemAddr, &u32ODmstMemSize);
    MDrv_SYS_GetMMAP(E_SYS_MMAP_OD_LSB_BUFFER, &u32ODlstMemAddr, &u32ODlstMemSize);

    OD_MSB_START_ADDR = ((U32)u32ODmstMemAddr / 16 & 0xFFFFFF);
    OD_MSB_LIMIT_ADDR = OD_MSB_START_ADDR + (U32)0x3F4E4;
    OD_LSB_START_ADDR = ((U32)u32ODlstMemAddr / 16 & 0xFFFFFF);
    OD_LSB_LIMIT_ADDR = OD_LSB_START_ADDR + (U32)0x17C14;

    MHal_SC_OD_Init(OD_MSB_START_ADDR, OD_MSB_LIMIT_ADDR, OD_LSB_START_ADDR, OD_LSB_LIMIT_ADDR, param.pODTbl);
}

void MDrv_SC_OverDriverSwitch(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_OD_TABLE_t param;

	if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_OD_TABLE_t)))
	{
	    return;
	}

    MHal_SC_OverDriverSwitch(param.bEnable);
}

void MDrv_SC_SetFD_Mask(U32 arg)
{
    BOOL param;
    if (copy_from_user(&param, (void __user *)arg, sizeof(BOOL)))
    {
        return;
    }
    MHal_SC_SetFD_Mask(param);
}

void MDrv_SC_SetDithering(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_SET_DITHERING_t param;

	if (copy_from_user(&param, (void __user *)arg, sizeof(SC_SET_DITHERING_t)))
	{
	    return;
	}

    MHal_SC_SetDithering(param.u8BitMode, param.bUrsapatch);
}

