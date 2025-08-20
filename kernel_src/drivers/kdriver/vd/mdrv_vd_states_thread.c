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

///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_vd_states_thread.c
/// This file contains the Mstar video decoder driver (this thread update driver's states)
/// @author MStar Semiconductor Inc.
/// @brief  VD
///////////////////////////////////////////////////////////////////////////////

#include <asm/atomic.h>
#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/delay.h>

#include "common_vd_singal_def.h"
#include "hal_vd_types.h"
#include "hal_vd_settings.h"
#include "mdrv_vd_states_thread.h"
#include "hal_vd_adc.h"


#define VD_REG_DEBUG
#ifdef  VD_REG_DEBUG

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                            BUG();\
                        }\
                    } while (0)

#define VD_ASSERT(arg)                  assert((arg))
#define VD_KDBG(x1, args...)            printk(KERN_WARNING x1, ## args)

#else
#define VD_KDBG(x1)
#define VD_ASSERT(arg)
#endif

/*
ASPECT_RATIO_TYPE       m_eAspectRatioCode;
*/

U32          _u32PreviousScart1IDLevelForAutoAV;
U32          _u32PreviousScart2IDLevelForAutoAV;

/* current video standard. */
atomic_t    _aVideoStandard = ATOMIC_INIT(0);

/* current video source */
atomic_t    _aVideoSource = ATOMIC_INIT(0);

/* current scart type */
atomic_t    _aScart1SrcType = ATOMIC_INIT(0);

/* sync counter */
U32         _u32NoSyncCounter;

/* _aStatesVD is a flag for VD states. */
atomic_t    _aStatesVD;

/* _aEventsVD is a flag for VD events. */
atomic_t    _aEventsVD;

/* _aSetsVD is a settings for VD. */
atomic_t    _aSettingsVD;

/* _aInCSVD is true to stop timer interrupt. */
atomic_t    _aInCSVD;

/* _aInCSVD is true to stop timer interrupt. */
atomic_t    _aInTUNING;	//channel skip test 090113

/* _aReleaseThreadVD tells system to halt the VD thread. */
atomic_t    _aReleaseThreadVD;

/* _aExitThreadVD means thread exit is sucess */
atomic_t    _aExitThreadVD;

/* timer counter, unit is 15ms. */
U32         _u32TimerCounterVD;

/* wait queue */
static wait_queue_head_t wq;
/* wait queue guard */
atomic_t    _aWaitQueueGuardVD = ATOMIC_INIT(0);

void MDrv_VD_Init(void)
{
    _u32NoSyncCounter = 10;
    atomic_set(&_aVideoStandard,E_VIDEOSTANDARD_NOTSTANDARD);
    atomic_set(&_aVideoSource, E_VIDEOSOURCE_INVALID);
    atomic_set(&_aScart1SrcType, E_SCART_SRC_TYPE_UNKNOWN);

    /* thread states initial */
    init_waitqueue_head(&wq);

    TIMER_INTERRUPT_PASS;

    atomic_set(&_aReleaseThreadVD,0x01);
    atomic_set(&_aExitThreadVD, 0x00);
    _u32TimerCounterVD = 0;
    atomic_set(&_aWaitQueueGuardVD, 0);

    /* VD states initial */
    _SetVDStates(0);

    /* VD events initial */
    _SetVDEvents(0);


    /* VD settings initial */
    _SetVDSettings(0);

    _bAGCEnabled(set);
    _bPermanentVideoMute(set);
    //VD_KDBG("VD settings = 0x%x\n", _ReadVDSettings);

}


B16 _MDrv_VD_Wait_Event(void)
{
    DEFINE_WAIT(waitentry);

    /* if systems stops timer interrupt, we don't need to wait event */
    if(atomic_read(&_aReleaseThreadVD) == 0x01)
        return FALSE;

    if(atomic_read(&_aWaitQueueGuardVD) == 0x00)
    {
        prepare_to_wait(&wq, &waitentry, TASK_INTERRUPTIBLE);

        atomic_set(&_aWaitQueueGuardVD, 0x01);
        if (_ReadVDEvents == 0x0)
        {
            //VD_KDBG("Sleep\n");
            schedule();
        }
        else
        {
            /* if we don't wait event, we should set _aWaitQueueGuardVD to zero */
            atomic_set(&_aWaitQueueGuardVD, 0x00);
        }
        finish_wait(&wq, &waitentry);
    }

    if(atomic_read(&_aReleaseThreadVD) == 0x01)
        return FALSE;
    /* if thread is active, system returns TRUE. */
    return TRUE;
}


void _MDrv_VD_Wakeup(void)
{
    if(atomic_read(&_aWaitQueueGuardVD) == 0x01)
    {
        if(_ReadVDEvents != 0x0 || atomic_read(&_aReleaseThreadVD)==0x01 )
        {
            //VD_KDBG("Wakeup\n");
            wake_up(&wq);
            atomic_set(&_aWaitQueueGuardVD, 0x00);
        }
    }
}

U32 _MDrv_VD_GetEvent(void)
{
    U32         etEvent;

    /* stop timer interrupt */
    //atomic_set(&_aInCSVD, 0x01);
    TIMER_INTERRUPT_STOP;

    etEvent = _ReadVDEvents;
    _SetVDEvents(0);


    //atomic_set(&_aInCSVD, 0x00);
    TIMER_INTERRUPT_PASS;
    return etEvent;
}

U32 _MDrv_VD_GetStates(void)
{
    U32         etStates;

    /* stop timer interrupt */
    //atomic_set(&_aInCSVD, 0x01);
    //TIMER_INTERRUPT_STOP;		//channel skip test 090113

    etStates = MHal_VD_GetStatus();


    etStates = etStates | (_ReadVDStates & 0xFFFF0000);
    //_SetVDStates(etStates);

    //atomic_set(&_aInCSVD, 0x00);

    //TIMER_INTERRUPT_PASS;		//channel skip test 090113
    return etStates;
}

void MDrv_VD_IsSyncDetected(void)
{
    if( MHal_VD_IsSyncDetected() == TRUE )
    {
        if( _u32NoSyncCounter > SYNC_CHECK_INIT )
        {
            _u32NoSyncCounter = SYNC_CHECK_INIT;
        }
        else if( _u32NoSyncCounter > 1 )
        {
            _u32NoSyncCounter--;
        }

        if( _u32NoSyncCounter == SYNC_LOCK )
        {
            _u32NoSyncCounter = 0;
            //VD_KDBG("Sync is changed (locked)\n");
            _bSyncChanged(set);
            _bSyncLock(set);
            //_SetVDStates(MHal_VD_GetStatus());
        }
    }
    else
    {
        /* sync stable after (SYNC_LOST-SYNC_CHECK_INIT) * 15ms */
        if( _u32NoSyncCounter < SYNC_CHECK_INIT )
        {
            _u32NoSyncCounter = SYNC_CHECK_INIT;
        }
        else if( _u32NoSyncCounter < SYNC_LIMIT )
        {
            _u32NoSyncCounter++;
        }

        if( _u32NoSyncCounter == SYNC_LOST )
        {
            _u32NoSyncCounter = SYNC_LIMIT;
            //VD_KDBG("Sync is changed (lost)\n");
            _bSyncChanged(set);
            _bSyncLock(clear);
            //_SetVDStates(MHal_VD_GetStatus());
        }
    }
}



void MDrv_VD_CheckVideoStandard(void)
{
//lachesis_091010
	#define VD_STD_DET_STABLE_CNT	 0	// reduce channel change time

    VIDEOSTANDARD_TYPE etPreStandard;
    static VIDEOSTANDARD_TYPE etDetectedStandard = E_VIDEOSTANDARD_NOTSTANDARD;
    static U8 u8StdDetStableCnt = 0;


    etPreStandard=MHal_VD_GetLastDetectedStandard();
    etDetectedStandard = MHal_VD_GetStandardDetection();
    //etDetectedStandard = etPreStandard;

    if (etDetectedStandard != etPreStandard)
    {
        /* fire to start video format checking */
        u8StdDetStableCnt = 0;
		VD_KDBG("\n etPreStandard=%d etDetectedStandard=%d\n", etPreStandard, etDetectedStandard);
    }
    else
    {
		if (u8StdDetStableCnt <= VD_STD_DET_STABLE_CNT)
		{
			//VD_KDBG("\n u8StdDetStableCnt=%d\n", u8StdDetStableCnt);

			if (u8StdDetStableCnt == VD_STD_DET_STABLE_CNT)
			{
				if (etDetectedStandard == E_VIDEOSTANDARD_NOTSTANDARD)
				{
					atomic_set(&_aVideoStandard, etDetectedStandard);
					//VD_KDBG("===> video format is changed (lost)\n");
					//_SetVDStates(MHal_VD_GetStatus());
					_bVideoFormatChanged(set);
				}
				else
				{
					atomic_set(&_aVideoStandard, etDetectedStandard);
					MHal_VD_SetVideoStandard(atomic_read(&_aVideoSource), etDetectedStandard, 0);
					//_SetVDStates(MHal_VD_GetStatus());
					_bVideoFormatChanged(set);
					//VD_KDBG("===> video format is changed (found)\n");
				}
			}

			u8StdDetStableCnt++;

		}
    }
}

void MDrv_VD_SetVideoSource(VIDEOSOURCE_TYPE etVideoSource)
{
    if( etVideoSource == atomic_read(&_aVideoSource) )
    {
        return;
    }
    /* stop timer interrupter */
    //atomic_set(&_aInCSVD, 0x01);
    TIMER_INTERRUPT_STOP;

    switch(etVideoSource)
    {
    case E_VIDEOSOURCE_ATV:
        MHal_VD_SetInput(E_VIDEOSOURCE_ATV);
        break;

    case E_VIDEOSOURCE_CVBS1:
        MHal_VD_SetInput(E_VIDEOSOURCE_CVBS1);
        break;

    case E_VIDEOSOURCE_CVBS2:
        MHal_VD_SetInput(E_VIDEOSOURCE_CVBS2);
        break;

    case E_VIDEOSOURCE_CVBS3:
        MHal_VD_SetInput(E_VIDEOSOURCE_CVBS3);
        break;

    case E_VIDEOSOURCE_SVIDEO1:
        MHal_VD_SetInput(E_VIDEOSOURCE_SVIDEO1);
        break;

    case E_VIDEOSOURCE_SVIDEO2:
        MHal_VD_SetInput(E_VIDEOSOURCE_SVIDEO2);
        break;

    case E_VIDEOSOURCE_SCART1:
        MHal_VD_SetInput(E_VIDEOSOURCE_SCART1);
        break;

    case E_VIDEOSOURCE_SCART2:
        MHal_VD_SetInput(E_VIDEOSOURCE_SCART2);
        break;

    default:
        /* enable timer interrupt. */
        //atomic_set(&_aInCSVD, 0x0);
#if 1
		MHal_VD_McuReset_Stop();
        MHal_VD_SetInput(E_VIDEOSOURCE_INVALID);
#endif
       TIMER_INTERRUPT_PASS;
        return;
    }

    //msAPI_VD_PriSetForcedFreeRun(FALSE);
    atomic_set(&_aVideoSource,  etVideoSource);
    atomic_set(&_aVideoStandard, E_VIDEOSTANDARD_NOTSTANDARD);
    _u32NoSyncCounter = 10;
    //_b3DCombfilterUsed(clear);
    atomic_set(&_aScart1SrcType, E_SCART_SRC_TYPE_UNKNOWN);



    /* Reset MCU */
    //MHal_VD_McuReset();

    /* enable timer interrupt. */
    //atomic_set(&_aInCSVD, 0x0);
    TIMER_INTERRUPT_PASS;
}

void MDrv_VD_Exit(void)  //20090826EL
{
    MHal_VD_VDMCU_SoftStop();
    //Add CLK off here
}


void _MDrv_VD_Thread(unsigned long arg)
{
    unsigned long u32jiiffes;
    struct timer_list *t_timer;
    //VIDEOSOURCE_TYPE etVideoSource;
    /* timer counter outputs on screen */

    //if(_u32TimerCounterVD%240 ==0)
    //    VD_KDBG("%d\n", (_u32TimerCounterVD/240));

    /* timer returns immediately if timer exit flag is true */
    if(atomic_read(&_aExitThreadVD) == 0x01)
        return;

    /* if _aInCSVD is set, it mean driver is in unstable, and timer interrupt cannot enter. */
    //if(atomic_read(&_aInCSVD) == 0x01)
    if(TIMER_INTERRUPT_FLAG == 0x01)
    {
        /*  we don't need add timer trigger if system wants to release thread. */
        if(atomic_read(&_aReleaseThreadVD) == 0x00)
        {
            t_timer = (struct timer_list *)arg;
            u32jiiffes = jiffies;
            /* timer is triggered at next 10 Ms */
            t_timer->expires =  u32jiiffes + _15MS;
            //t_timer->expires =  u32jiiffes + 5;
            add_timer(t_timer);
        }
        else
        {
            /* set timer exit flag */
            atomic_set(&_aExitThreadVD, 0x01);
            //_MDrv_VD_Wakeup();
        }
        return;
    }

    /* system detectes sync every 135 ms. (15ms * 7 = 109 ms) */
    MDrv_VD_IsSyncDetected();

    /* system monitors VD mcu every 15 ms */
    // FitchHsu 20090714 VD latch up problem
    MDrv_VD_SetRegFromDSP(atomic_read(&_aVideoSource));

    _u32TimerCounterVD++;

	if(MHal_VD_GetDSPVer() == VD_SYSTEM_NTSC)// Set Analog Color System - 090309.
	{	///ntsc only
		if(TIMER_INTERRUPT_FLAG1 != 0x01)	//channel skip test 090113
        	MDrv_VD_CheckVideoStandard();
	}
	else
		MDrv_VD_CheckVideoStandard();

    //_MDrv_VD_Wakeup();


    /*  we don't need add timer trigger if system wants to release thread. */
    if(atomic_read(&_aReleaseThreadVD) == 0x00)
    {
        t_timer = (struct timer_list *)arg;
        u32jiiffes = jiffies;
        /* timer is triggered at next 15 Ms */
        t_timer->expires =  u32jiiffes + _15MS;
        //t_timer->expires =  u32jiiffes + 5;
        add_timer(t_timer);
    }
    else
    {
        /* set timer exit flag */
        atomic_set(&_aExitThreadVD, 0x01);
        //_MDrv_VD_Wakeup();
    }
}


static struct timer_list g_timer;
void _MDrv_VD_StartTimer(void)
{
    unsigned long u32jiiffes = jiffies;
    init_timer(&g_timer);
    g_timer.data = (unsigned long)&g_timer;
    g_timer.function = _MDrv_VD_Thread;
//    if( _10MS == 0 )
	if ( g_timer.expires == 0)
    {
        /* timer unit is small. */
        g_timer.expires = u32jiiffes + (1);
    }
    else
    {
        g_timer.expires = u32jiiffes + _15MS; //(15.625ms)
    }
    add_timer(&g_timer);
}

void _MDrv_VD_DeleteTimer(void)
{
    U32 u32TimeOut = 5;
#if 1
    /* system waits thread exit success or time out after 50*5 ms. */
    while(u32TimeOut--)
    {
        msleep(50);
        if(atomic_read(&_aExitThreadVD) == 0x01)
            break;
    }
    /* delete timer */
    del_timer_sync(&g_timer);
#endif
}

