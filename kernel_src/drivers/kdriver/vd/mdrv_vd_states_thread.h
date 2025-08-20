#ifndef __MDRV_VD_STATES_THREAD_H__
#define __MDRV_VD_STATES_THREAD_H__


#include "hal_vd_types.h"
#include "common_vd_singal_def.h"
#include <linux/wait.h>
#include <asm/atomic.h>
#include <asm/bitops.h>

typedef enum
{
    E_VIDEO_FORMAT_CHANGED =    _BIT0,
    E_VIDEO_INPUT_CHANGED =     _BIT1,
    E_ASPECT_RATION_CHANGED =   _BIT2,
    E_SYNC_CHANGED =            _BIT3,
    E_SCART1_SOURCE_CHANGED =   _BIT4,
    E_THREAD_STOP =             _BIT31,
    E_NO_EVENT =                0x0
} EVENT_VD;

typedef enum
{
    E_DATA_VALID                =_BIT0,
    E_BURST                     =_BIT1,
    E_MACRO_VISION              =_BIT2,
    E_VCR_PASUE                 =_BIT3,
    
    E_VCR_MODE                  =_BIT4,
    
    E_FSM_READY                 =_BIT8,
    E_PAL                       =_BIT9,
    E_COLOR_LOCK                =_BIT10,
    E_RESET_ON                  =_BIT11,
    
    E_VSYNC_TYPE                =_BIT12,
    E_INTERLACE                 =_BIT13,
    E_HSYNC_LOCK                =_BIT14,
    E_VSYNC_LOCK                =_BIT15,
    
    E_AUTO_AV1_ACTIVE           =_BIT16,
    E_AUTO_AV2_ACTIVE           =_BIT17,
    E_ASPECT_RATION             =_BIT18,
    E_SYNC_LOCK                 =_BIT19,
    
    E_THREAD_STATES             =_BIT31,
    E_NO_STATES =               0x0
} STATES_VD;

#define     VD_STATES   ((void*)&_aStatesVD)
#define _ReadVDStates                       atomic_read(&_aStatesVD)
#define _SetVDStates(x)                     atomic_set(&_aStatesVD, x)
#define _bValid(x)                          x##_bit(0, VD_STATES)
#define _bBurst(x)                          x##_bit(1, VD_STATES)
#define _bMacroVision(x)                    x##_bit(2, VD_STATES)
#define _bVCRPause(x)                       x##_bit(3, VD_STATES)
#define _bVCRMode(x)                        x##_bit(4, VD_STATES)
// 5:7 is FSC Type
#define _bFSMReady(x)                       x##_bit(8, VD_STATES)
#define _bPAL(x)                            x##_bit(9, VD_STATES)
#define _bColorLock(x)                      x##_bit(10, VD_STATES)
#define _ResetOn(x)                         x##_bit(11, VD_STATES)
#define _bVSyncType(x)                      x##_bit(12, VD_STATES)
#define _bInterlaced(x)                     x##_bit(13, VD_STATES)
#define _bHSyncLock(x)                      x##_bit(14, VD_STATES)
#define _bVSyncLock(x)                      x##_bit(15, VD_STATES)

#define _bAutoAV1Active(x)                  x##_bit(16, VD_STATES)
#define _bAutoAV2Active(x)                  x##_bit(17, VD_STATES)
#define _bAspectRation(x)                   x##_bit(18, VD_STATES)
#define _bSyncLock(x)                       x##_bit(19, VD_STATES)

#define _bThreadStates(x)                   x##_bit(31, VD_STATES)

#define     VD_EVENTS   ((void*)&_aEventsVD)
#define _ReadVDEvents                       atomic_read(&_aEventsVD)
#define _SetVDEvents(x)                     atomic_set(&_aEventsVD, x)

#define _bVideoFormatChanged(x)             x##_bit(0, VD_EVENTS)
#define _bVideoInputChanged(x)              x##_bit(1, VD_EVENTS)
#define _bAspectRatioChanged(x)             x##_bit(2, VD_EVENTS)
#define _bSyncChanged(x)                    x##_bit(3, VD_EVENTS)
#define _bScart1SourceTypeChanged(x)        x##_bit(4, VD_EVENTS)
#define _bScart2SourceTypeChanged(x)        x##_bit(5, VD_EVENTS)


#define     VD_SETTINGS   ((void*)&_aSettingsVD)
#define _ReadVDSettings                     atomic_read(&_aSettingsVD)
#define _SetVDSettings(x)                   atomic_set(&_aSettingsVD, x)
#define _b3DCombfilterUsed(x)               x##_bit(0, VD_SETTINGS)
#define _bAGCEnabled(x)                     x##_bit(1, VD_SETTINGS)
#define _bPermanentVideoMute(x)             x##_bit(2, VD_SETTINGS)
#define _bMomentVideoMute(x)                x##_bit(3, VD_SETTINGS)
#define _bByLockVideoMute(x)                x##_bit(4, VD_SETTINGS)
#define _bByParentalVideoMute(x)            x##_bit(5, VD_SETTINGS)
#define _bByDuringLimitedTimeVideoMute(x)   x##_bit(6, VD_SETTINGS)
#define _bWSS(x)                            x##_bit(7, VD_SETTINGS)
#define _bSCART(x)                          x##_bit(8, VD_SETTINGS)

/* time unit */
#define _15MS   (HZ/66)	// x4(ok) x3(NG) x2(NG) x5(ok)
#define _10MS   (HZ/100)	// x4(ok) x3(NG) x2(NG) x5(ok)
#define _7_5MS   (HZ/132)
#define _5MS   (HZ/200)
#define _3_75MS   (HZ/266)
#define _3MS   (HZ/333)
#define _2_5MS   (HZ/400)
#define _2MS   (HZ/500)
#define _1MS   (HZ/1000)

/*
#define _15MS   (HZ/66)
#define _10MS   (HZ/100)

*/
#define SYNC_LOCK         7
#define SYNC_CHECK_INIT   10
#define SYNC_LOST         13
#define SYNC_LIMIT        0xFFFF
/*
#define SYNC_LOCK         5
#define SYNC_CHECK_INIT   50
#define SYNC_LOST         95
#define SYNC_LIMIT        0xFFFF
*/
//#define SYNC_CHECK_INIT   20


extern atomic_t     _aStatesVD;
extern atomic_t     _aEventsVD;
extern atomic_t     _aVideoStandard;
extern atomic_t     _aVideoSource;
extern atomic_t     _aReleaseThreadVD;
extern atomic_t     _aInCSVD;
extern atomic_t     _aExitThreadVD;

extern atomic_t     _aInTUNING;	//channel skip test 090113

#define TIMER_INTERRUPT_STOP                  atomic_set(&_aInCSVD, 0x01)
#define TIMER_INTERRUPT_PASS                  atomic_set(&_aInCSVD, 0x00)
#define TIMER_INTERRUPT_FLAG                  atomic_read(&_aInCSVD)

//channel skip test 090113
#define TIMER_INTERRUPT_STOP1                 atomic_set(&_aInTUNING, 0x01)
#define TIMER_INTERRUPT_PASS1                 atomic_set(&_aInTUNING, 0x00)
#define TIMER_INTERRUPT_FLAG1                 atomic_read(&_aInTUNING)

extern B16 _MDrv_VD_Wait_Event(void);

extern void MDrv_VD_Init(void);
extern void MDrv_VD_CheckVideoStandard(void);
extern void MDrv_VD_SetVideoSource(VIDEOSOURCE_TYPE);

extern U32  _MDrv_VD_GetStates(void);
extern void _MDrv_VD_StartTimer(void);
extern U32  _MDrv_VD_GetEvent(void);
extern void _MDrv_VD_DeleteTimer(void);

extern void MDrv_VD_Exit(void);  //20090826EL

#endif
