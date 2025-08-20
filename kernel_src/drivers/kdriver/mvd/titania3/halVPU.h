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

#ifndef _HAL_VPU_H_
#define _HAL_VPU_H_


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#if defined(REDLION_LINUX_KERNEL_ENVI)
#define ENABLE_VPU_MUTEX_PROTECTION     0
#define VPU_DEFAULT_MUTEX_TIMEOUT       0xFFFFFFFFUL
#else
#define ENABLE_VPU_MUTEX_PROTECTION     1
#define VPU_DEFAULT_MUTEX_TIMEOUT       MSOS_WAIT_FOREVER
#endif

#define VPU_CLOCK_160MHZ                BITS(6:2,0)
#define VPU_CLOCK_144MHZ                BITS(6:2,1)
#define VPU_CLOCK_123MHZ                BITS(6:2,2)
#define VPU_CLOCK_108MHZ                BITS(6:2,3)
#define VPU_CLOCK_96MHZ                BITS(6:2,4)
#define VPU_CLOCK_72MHZ                BITS(6:2,5)
#define VPU_CLOCK_NOTSUPPORT                BITS(6:2,8)

#define VPU_HI_MBOX0        0
#define VPU_HI_MBOX1        1
#define VPU_RISC_MBOX0      2
#define VPU_RISC_MBOX1      3


#define VPU_Timer_delay_ms(x)            \
    do {                                                            \
        volatile MS_U32 ticks=0;                                         \
        while( ticks < ( ((MS_U32)(x)) <<13) )      \
        {                                                               \
            ticks++;                                                \
        }                                                               \
    } while(0)
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_VPU_CLOCK_160MHZ=VPU_CLOCK_160MHZ,
    E_VPU_CLOCK_144MHZ=VPU_CLOCK_144MHZ,
    E_VPU_CLOCK_123MHZ=VPU_CLOCK_123MHZ,
    E_VPU_CLOCK_108MHZ=VPU_CLOCK_108MHZ,
    E_VPU_CLOCK_96MHZ=VPU_CLOCK_96MHZ,
    E_VPU_CLOCK_86MHZ=VPU_CLOCK_NOTSUPPORT,
    E_VPU_CLOCK_72MHZ=VPU_CLOCK_72MHZ,
} VPU_Clock_Speed;


typedef struct
{
    VPU_Clock_Speed eClockSpeed;
    MS_BOOL  bClockInv;
    MS_S32  s32VPUMutexID;
    MS_U32  u32VPUMutexTimeout;
} VPU_Init_Params;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void HAL_VPU_InitRegBase(MS_U32 u32RegBase);
MS_BOOL HAL_VPU_Init(VPU_Init_Params* InitParams);
void HAL_VPU_PowerCtrl(MS_BOOL bEnable);
void HAL_VPU_ClockSpeed(MS_U32 u32type);
void HAL_VPU_ClockInv(MS_BOOL bEnable);
void HAL_VPU_MIU_RW_Protect(MS_BOOL bEnable);
MS_BOOL HAL_VPU_CPUSetting(MS_U32 u32StAddr);
MS_BOOL HAL_VPU_SwRst(void);
void HAL_VPU_SwRstRelse(void);
MS_U32 HAL_VPU_MemRead(MS_U32 u32Address);
MS_BOOL HAL_VPU_MemWrite(MS_U32 u32Address, MS_U32 u32Value);
MS_BOOL HAL_VPU_MBoxRdy(MS_U32 u32type);
MS_BOOL HAL_VPU_MBoxRead(MS_U32 u32type, MS_U32 *u32Msg);
void HAL_VPU_MBoxClear(MS_U32 u32type);
MS_BOOL HAL_VPU_MBoxSend(MS_U32 u32type, MS_U32 u32Msg);
MS_U32 HAL_VPU_GetProgCnt(void);

#endif // _HAL_VPU_H_

