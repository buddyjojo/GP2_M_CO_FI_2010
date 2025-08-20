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
    #define mfcSleepMs(x) Delay1ms_Nop(x) //Delay1ms(x)
    #define mfcSleepMsNop(x) Delay1ms_Nop(x)
    #define mfcSleep10usNop(x) Delay10us_Nop(x)

    // PWM setting
#define PWM_CRYSTAL_CLOCK           12000000ul
#define PERIOD_MASK     0x03FFFF
#define LETTERBOX_BY_MIPS 0
    #if(CODESIZE_SEL == CODESIZE_ALL)
        #define S7M_CHIP 0
    #else
        #define S7M_CHIP 1
    #endif
#define NANYA_DDR_NEW_SETTING 0


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
        #define mfcSleep10usNop(x) udelay(10*x)
    #include "../mdrv_mfc.h"
    #else
        #include <unistd.h>
        #include <sys/time.h>
        #define mfcSleepMs(x) usleep(1000*x)
        #define mfcSleepMsNop(x)  usleep(1000*x)
        #define mfcSleep10usNop(x) usleep(10*x)
    #include "mdrv_mfc.h"
    #endif
	#define code
    #define XDATA
    #include "pnl_lceall.h"
    #define MFC_IIC_CHANNEL_ID 0x02
    #define MFC_IIC_SLAVE_ADDR 0xB4
    #define LETTERBOX_BY_MIPS 1

    #define MDrv_MFC_Read2BytesINT(a) MDrv_MFC_Read2Bytes(a)
    #define MDrv_MFC_Write3BytesINT(a,b) MDrv_MFC_Write3Bytes(a,b)
    #define MDrv_MFC_Write2BytesINT(a,b) MDrv_MFC_Write2Bytes(a,b)
    #define MDrv_MFC_WriteBitINT(a,b,c) MDrv_MFC_WriteBit(a,b,c)
    #define MDrv_MFC_WriteByteMaskINT(a,b,c) MDrv_MFC_WriteByteMask(a,b,c)

    #define S7M_CHIP 1
    #define NANYA_DDR_NEW_SETTING 1

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
    #define ENABLE_USER_TOTAL 0
    #define XDATA
    #include "pnl_lceall.h"
    #define MFC_IIC_CHANNEL_ID 0x01
    #define MFC_IIC_SLAVE_ADDR 0xB4

    #define MDrv_MFC_Write2BytesINT(a,b) MDrv_MFC_Write2Bytes(a,b)
    #define MDrv_MFC_WriteBitINT(a,b,c) MDrv_MFC_WriteBit(a,b,c)

#endif
extern XDATA BOOL S7M;
#define SW_FILM_STATE_MACHINE 0  // bootloader can not use this.
#define BUF_NUM 5
#if (BUF_NUM==7)
#define REG_290E_DEFAULT 0x2F
#define REG_2910_DEFAULT 0xF7
#elif (BUF_NUM==6)
#define REG_290E_DEFAULT 0x6F
#define REG_2910_DEFAULT 0xEE
#elif (BUF_NUM==5)
#define REG_290E_DEFAULT 0x6F
#define REG_2910_DEFAULT 0xE5
#endif

//////////////////////////////////////////////////
// Chip
//////////////////////////////////////////////////
#define MST_3xxx_U01        			0x21 //Ursa3 U01
#define MST_3xxx_U02        			0x22 //Ursa3 U02
#define MST_3xxx_U03       			0x23 //Ursa3 U03

#define IS_CHIP_3xxx_U01(ucVer)  	        (ucVer==MST_3xxx_U01)
#define IS_CHIP_3xxx_U02_AND_AFTER(ucVer)  	(ucVer>=MST_3xxx_U02)// ( (ucVer>=MST_3xxx_U02)?1:0 )
#define IS_CHIP_3xxx_U03_AND_AFTER(ucVer)  	(ucVer>=MST_3xxx_U03)

#endif

