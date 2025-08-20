#ifndef _MDRV_MFC_PLATFORM_H_
#define _MDRV_MFC_PLATFORM_H_

#define CODEBASE_51		 	0x01
#define CODEBASE_LINUX		0x02
#define CODEBASE_UTOPIA		0x03

#define CODESIZE_ALL		0x01
#define CODESIZE_SMALL		0x02


#define CODEBASE_SEL				CODEBASE_LINUX //CODEBASE_51 //

#if(CODEBASE_SEL == CODEBASE_51)
	#include "board.h"
	#include "system.h"
	#include "common.h"
	#include "msreg.h"
    #define mfcSleepMs(x) Delay1ms(x)
    #define mfcSleepMsNop(x) Delay1ms_Nop(x)
#elif(CODEBASE_SEL == CODEBASE_LINUX)
	#include "mdrv_types.h"
	#include "mdrv_mfc_define.h"
    #include "mst_platform.h"
    #ifndef MFC_USE_IN_BOOLLOAD
        #define MFC_USE_IN_BOOLLOAD 0
    #endif

    #if MFC_USE_IN_BOOLLOAD
        #define mfcSleepMs(x) udelay(1000*x)
        #define mfcSleepMsNop(x)  udelay(1000*x)
        #include "../mdrv_mfc.h"
    #else
        #ifdef __KERNEL__
            //#include <unistd.h>
            #include <linux/delay.h>
            #define mfcSleepMs(x) udelay(1000*x)
            #define mfcSleepMsNop(x)  udelay(1000*x)
            #include "mdrv_mfc.h"
        #else
            #include <unistd.h>
            #include <sys/time.h>
            #define mfcSleepMs(x) usleep(1000*x)
            #define mfcSleepMsNop(x)  usleep(1000*x)
            #include "mdrv_mfc.h"
        #endif
    #endif
	#define code
    #define XDATA
    #include "pnl_lceall.h"

#elif(CODEBASE_SEL == CODEBASE_UTOPIA)
	#include "mdrv_mfc_define.h"
	//#include "MsTypes.h"
	//#include "MsOS.h"
	#include "MsCommon.h"

	#define code
	#define U8 MS_U8
	#define U16 MS_U16
	#define U32 MS_U32
	#define S8 MS_S8
	#define S16 MS_S16
	#define S32 MS_S32
	#define BOOL MS_BOOL

	#define _6BITS						0
	#define _8BITS						1
	#define _10BITS						2

	#define _TTL             			0
	#define _MINI_LVDS                  1
	#define _LVDS                       2
	#define _RSDS                       3
	#define _MINI_LVDS_GIP              4
	#define _MINI_LVDS_GIP_V5           5

	#define _SINGLE						0
	#define _DUAL       				1
	#define _QUAD       				2
	#define _QUAD_LR       				3
	#define _V_BY1                      4

	// for Lvds output channel,
	// REG_321F[7:6]=>D, [5:4]=>C, [3:2]=>B, [1:0]=>A
	#define CHANNEL_SWAP(D, C, B, A)	((D<<6)&0xC0)|((C<<4)&0x30)|((B<<2)&0x0C)|(A&0x03)
	#define _CH_A                       0
	#define _CH_B                       1
	#define _CH_C                       2
	#define _CH_D                       3
    #define mfcSleepMs(x) OS_DELAY_TASK(x)
    #define mfcSleepMsNop(x)  OS_DELAY_TASK(x)

    #include "pnl_lceall.h"
#endif
extern BOOL S7M;
#endif

