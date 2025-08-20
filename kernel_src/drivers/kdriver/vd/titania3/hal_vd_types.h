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

#include "mdrv_types.h"
#include <asm/atomic.h>
#include <asm/bitops.h>
#include <linux/seqlock.h>
#include <linux/interrupt.h>
#include <linux/timer.h>


#include "common_vd_singal_def.h"
#include "chip_setup.h"
#ifndef _HAL_VD_TYPES_H_
#define _HAL_VD_TYPES_H_



/* c control, id is a symbolc means loop identifier for human reads easily */
#define _FOREVER(id)          while(1)
#define _BEGIN(id)            {
#define _END(id)              }


/* global variable locker */
/* this version cannot be used by hardware interrupter handler*/
#define INIT_VAR_LOCK(locker) seqlock_t locker##_lock = SEQLOCK_UNLOCKED

#define R_VAR_LOCKED(code, locker) \
{\
    U32 lock_serial;\
    do {\
        lock_serial = read_seqbegin(&locker##_lock);\
        (code);\
    } while(read_seqretry(&locker##_lock, lock_serial));\
}


#define W_VAR_LOCKED(code, locker) \
{\
    write_seqlock_bh(&locker##_lock);\
    (code);\
    write_sequnlock_bh(&locker##_lock);\
}



/* time unit */

/* data type use by VD */
#define _BIT0                       0x00000001
#define _BIT1                       0x00000002
#define _BIT2                       0x00000004
#define _BIT3                       0x00000008
#define _BIT4                       0x00000010
#define _BIT5                       0x00000020
#define _BIT6                       0x00000040
#define _BIT7                       0x00000080
#define _BIT8                       0x00000100
#define _BIT9                       0x00000200
#define _BIT10                      0x00000400
#define _BIT11                      0x00000800
#define _BIT12                      0x00001000
#define _BIT13                      0x00002000
#define _BIT14                      0x00004000
#define _BIT15                      0x00008000
#define _BIT16                      0x00010000
#define _BIT17                      0x00020000
#define _BIT18                      0x00040000
#define _BIT19                      0x00080000
#define _BIT20                      0x00100000
#define _BIT21                      0x00200000
#define _BIT22                      0x00400000
#define _BIT23                      0x00800000
#define _BIT24                      0x01000000
#define _BIT25                      0x02000000
#define _BIT26                      0x04000000
#define _BIT27                      0x08000000
#define _BIT28                      0x10000000
#define _BIT29                      0x20000000
#define _BIT30                      0x40000000
#define _BIT31                      0x80000000

#define _END_OF_TBL_                -1 // end of register table

#define WORD                        U16
#define BYTE                        U8

/*
RIU Byte address to Titania IO address
RIU address in titania is 4 byte alignment and high word address is reserved.
*/
#define BYTE2REAL(B)                (((B)>>1<<2)+((B)&0x01))
#define WORD2REAL(W)                ((W)<<2)


/* BITMASK used by VD */
#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)
#define BITFLAG(loc) (1U << (loc))
#define IS_BITS_SET(val, bits)      (((val)&(bits)) == (bits))

#define MAKEWORD(value1, value2)    (((U16)(value1)) * 0x100) + (value2)


#define XBYTE(addr)             X1BYTE(addr)
#define X1BYTE(addr)            *(volatile U8*)(REG_MIPS_BASE + (addr))
#define X2BYTE(addr)            *(volatile U16*)(REG_MIPS_BASE + (addr))

/* Write/Read method invalid */
U8      _MHal_VD_R1B    ( U32 u32Reg );
void    _MHal_VD_W1B    ( U32 u32Reg, U8 u08Val);
void    _MHal_VD_W1BM   ( U32 u32Reg, U8 u08Val, U8 u08Mask);
void    _MHal_VD_W1Rb   ( U32 u32Reg, B16 bBit, U8 u08BitPos );
U8      _MHal_VD_R1Rb   ( U32 u32Reg, U8 u08BitPos );
U16     _MHal_VD_R2B    ( U32 u32Reg );
void    _MHal_VD_W2B    ( U32 u32Reg, U16 u16Val );
void    _MHal_VD_W2BM   ( U32 u32Reg, U16 u16Val, U16 u16Mask );
void    _MHal_VD_W2Bb   ( U32 u32Reg, B16 bBit, U16 u16BitPos );
U16     _MHal_VD_R2Bb   ( U32 u32Reg, U16 u16BitPos );


typedef struct {
//lachesis_081120
VIDEOSTANDARD_TYPE eVideoStandard;

U16 u16SRHStart;    // H capture start
U16 u16SRVStart;    // V capture start
U16 u16HRange;      // H capture range
U16 u16VRange;      // V capture range

} VIDEO_CAPTUREWINTABLE_TYPE;

// Set Analog Color System - Start 090309.
typedef enum
{
    VD_SYSTEM_DVB  = 0,
    VD_SYSTEM_NTSC = 1,
} VDSYSTEM;

typedef enum
{
    VD_COLOR_MULTI,
    VD_COLOR_NTSC_M,
    VD_COLOR_PAL_M,
    VD_COLOR_PAL_NC,
    VD_COLOR_PAL_N,
    VD_COLOR_NUM
} VD_ANALOG_COLOR_SYSTEM;
// Set Analog Color System - End   090309.

extern VIDEO_CAPTUREWINTABLE_TYPE _tVideoCaptureWinTblVD[];
extern VIDEO_CAPTUREWINTABLE_TYPE _tVideoCaptureWinTblVD_SVIDEO[];

extern void
MDrv_VD_SetRegFromDSP(VIDEOSOURCE_TYPE etSourceVD);

extern B16
MHal_VD_IsSyncDetected(void);

extern VIDEOSTANDARD_TYPE
MHal_VD_GetStandardDetection(void);

extern VIDEOSTANDARD_TYPE
MHal_VD_GetLastDetectedStandard(void);

extern VIDEOSTANDARD_TYPE
MHal_VD_GetStandardDetection(void);

extern void MHal_VD_Init(U32);//DSP branch 10

extern void MHal_VD_SetInput(VIDEOSOURCE_TYPE etSourceTypeVD);
extern void MHal_VD_SetVideoStandard(VIDEOSOURCE_TYPE etSourceVD, VIDEOSTANDARD_TYPE etStandardVD, B16 bIsTurningVD);

extern void MHal_VD_McuReset(void);

extern void MHal_VD_McuReset_Stop(void);

extern U16 _MHal_VD_GetOutputVT(void);
extern U16 _MHal_VD_GetOutputHT(void);

extern void MHal_VD_SetHsyncDetectionForTuning(B16 bEnableVD);

extern U16 MHal_VD_CheckStatusLoop(void);
extern U16 MHal_VD_GetStatus(void);
extern S8 MHal_VD_LoadDsp(void);// Set Analog Color System - 090309.

//LGE boae20081025
// VER1 1025
extern void MHal_VD_ResetAGC(void);

//lachesis_100118 for indonesia
extern void MHal_VD_SetAdaptiveHsyncTracking(B16 bEnable);
extern void MHal_VD_SetAdaptiveHsyncSlice(B16 bEnable);

// shjang_091020
extern void MHal_VD_SetSigSWingThreshold(U8 Threshold);

// VER2_1025
/* this function add a parameter to enable or disable 3D comb speed up. */
extern void MHal_VD_3DCombSpeedup(B16);
extern void MHal_VD_Set3dComb(B16);
extern U16 MHal_VD_GetHsyncCountAndVdStatus2ndCheck(void);
//	addeded to change the initial value of color kill bound (dreamer@lge.com)
extern void MHal_VD_InitColorKillBound( U32 type );

// shjang_100322 manual H pll speed control
extern void MHal_VD_ManualHPllTrackingControl( U8 PLLTrackingSpeed);
extern void MHal_VD_SetHsyncWidthDetection(B16 bEnable);

/* DSP Version : 0 for DVB, 1 : NTSC
  LGE skystone 20090109
*/
// Set Analog Color System - Start 090309.
extern void MHal_VD_SetDSPVer(VDSYSTEM   eDSPVer);
extern VDSYSTEM MHal_VD_GetDSPVer(void);
extern void MHal_VD_SetColorSystem(VD_ANALOG_COLOR_SYSTEM eColorSystem);
// Set Analog Color System - End   090309.

#ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
extern void MHal_VD_COMB_Set3dCombMid(MS_BOOL bEnable);
#endif

extern void MHal_VD_SetClock(BOOL bEnable);
extern void MHal_VD_VDMCU_SoftStop(void);  //20090826EL

#endif
