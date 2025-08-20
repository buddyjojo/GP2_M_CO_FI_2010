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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_tsp.h
/// @brief  Transport Stream Processer (TSP) Driver Interface
/// @author MStar Semiconductor,Inc.
/// @attention
/// All TSP DDI are not allowed to use in any interrupt context other than TSP ISR and Callback
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRVTSP_H_
#define _MDRVTSP_H_

//--------------------------------------------------------------------------------------------------
// OS interface
//--------------------------------------------------------------------------------------------------
#define TSP_DBG_ENABLE              0
#define TSP_DEBUG                   0

#if TSP_DEBUG
    #define TSP_ASSERT(_bool, _f)               if (!(_bool)) { (_f); while (1); }
    #define DMX_LOCK()                          spin_lock(&_spinlock)
    #define DMX_UNLOCK()                        spin_unlock(&_spinlock)
    #define DBG_TSP(_f)                         (_f)
    #define DBG_CHECK_POINT()                   DBG_TSP(printf("TSP %d\n", __LINE__))
    static U8                                   _TspBuildDate[] = __DATE__;
    #define TSP_PRINT(fmt, args...)             printk("[TSPMOD][%06d]     " fmt, __LINE__, ## args)
#else
    #define TSP_ASSERT(_bool, _f)             while (0)
    #define DMX_LOCK()                        while (0)
    #define DMX_UNLOCK()                      while (0)
    #define DBG_TSP(_f)
    #define DBG_CHECK_POINT()
    #define TSP_PRINT(fmt, args...)
#endif // #ifdef TSP_DEBUG

#define IntEnable()                         enable_irq(E_IRQ_TSP)
#define IntDisable()                        disable_irq(E_IRQ_TSP)
#define IntAttach()                         request_irq(E_IRQ_TSP, MDrv_TSP_Isr, SA_INTERRUPT, "TSPInt", NULL)
#define IntDetach()                         free_irq(E_IRQ_TSP, NULL)

#define _SetEvent(flag)                     { \
                                                SET_FLAG((_u32TspEvent), (flag)); \
                                                wake_up(&_wait_queue); \
                                            }
#define _SetSecRdyId(eng, flag)             SET_FLAG((_u32TspSecRdy), (flag))
#define _SetSecOvfId(eng, flag)             SET_FLAG((_u32TspSecOvf), (flag))

#define _TSP_ENTRY()                        spin_lock(&_spinlock)
#define _TSP_RETURN(_ret)                   spin_unlock(&_spinlock);                               \
                                    if (_ret != DRVTSP_OK) { _u32LastErr = _ret; }              \
                                    return _ret; // Should be optimized by compiler

// Richard: Event definition should be modified as well
#define TSP_TASK_EVENT_SECTION                      0x00000001
#define TSP_TASK_EVENT_PVR0_RDY                     0x00000002
#define TSP_TASK_EVENT_PVR1_RDY                     0x00000004
#define TSP_TASK_EVENT_PVR0_MID                     0x00000008
#define TSP_TASK_EVENT_PVR1_MID                     0x00000010
#define TSP_TASK_EVENT_GROUP                        (TSP_TASK_EVENT_SECTION  |      \
                                                     TSP_TASK_EVENT_PVR0_RDY |      \
                                                     TSP_TASK_EVENT_PVR1_RDY |      \
                                                     TSP_TASK_EVENT_PVR0_MID |      \
                                                     TSP_TASK_EVENT_PVR1_MID)

#define TSP_TASK_STACK_SIZE         2048
#define TSP_MUTEX_TIMEOUT           MSOS_WAIT_FOREVER

//--------------------------------------------------------------------------------------------------
// Constant definition
//--------------------------------------------------------------------------------------------------
#define printf                          printk
#define TSP_WARNING(fmt, args...)       printk(KERN_WARNING "[TSPMOD][%06d]     " fmt, __LINE__, ## args)

//--------------------------------------------------------------------------------------------------
// Constant definition
//--------------------------------------------------------------------------------------------------
#define MOD_TSP_DEVICE_COUNT    1

//--------------------------------------------------------------------------------------------------
// Data structure
//--------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32TspMajor;
    int                         s32TspMinor;
    struct cdev                 cDevice;
    struct file_operations      TspFop;
} TspModHandle;

typedef struct _DrvTSP_State
{
    DrvTSP_FltState                 FltState[TSP_PIDFLT_NUM_ALL];
    DrvTSP_FltType                  FltType[TSP_PIDFLT_NUM_ALL];
    U32                             FltSource[TSP_PIDFLT_NUM_ALL];

    DrvTSP_FltState                 SecFltState[TSP_SECFLT_NUM];
} DrvTSP_State;

#endif // _MDRVTSP_H_
