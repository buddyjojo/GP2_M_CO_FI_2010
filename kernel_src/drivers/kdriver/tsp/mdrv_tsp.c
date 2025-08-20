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

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include "chip_int.h"
#include "mst_devid.h"
#include "mdrv_types.h"

#include "mhal_chiptop_reg.h"

#include "mhal_tsp.h"

// Note: include the following header files shared between kernel driver and user adaptation driver after including mhal_tsp_hardware.h
#include "mdrv_tsp_st.h"
#include "mdrv_tsp_io.h"

#include "mdrv_tsp.h"


#include "mdrv_probe.h"

//--------------------------------------------------------------------------------------------------
// Local variables
//--------------------------------------------------------------------------------------------------
static unsigned int u32TSPIntRegister = 0 ; // LGE added
static wait_queue_head_t        _wait_queue;
static spinlock_t               _spinlock;
static U32                      _u32TspEvent =      0;
static U32                      _u32TspSecRdy =     0;
static U32                      _u32TspSecOvf =     0;

// TSP DDI states
static DrvTSP_State             _TspState[TSP_ENGINE_NUM];
static U32                      _TspEnable[TSP_ENGINE_NUM];
static B16                      _bPvrBuf0 = TRUE;
static U32                      _u32LastErr;
static B16                      _b192Mode = FALSE;

#define RESERVED_PIDFLT_NUM     6
static const U8                 _u8IdxStream[TSP_ENGINE_NUM][RESERVED_PIDFLT_NUM]=
    {{ TSP_PIDFLT_NUM - 1, TSP_PIDFLT_NUM - 2, TSP_PIDFLT_NUM - 3, TSP_PIDFLT_NUM - 4, TSP_PIDFLT_NUM - 5, TSP_PIDFLT_NUM - 6}};

//--------------------------------------------------------------------------------------------------
// OS interface
//--------------------------------------------------------------------------------------------------
static int              _mod_tsp_open (struct inode *inode, struct file *filp);
static int              _mod_tsp_release(struct inode *inode, struct file *filp);
static ssize_t          _mod_tsp_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t          _mod_tsp_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int     _mod_tsp_poll(struct file *filp, poll_table *wait);
static int              _mod_tsp_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static irqreturn_t      MDrv_TSP_Isr(int irq, void *dev_id);

//--------------------------------------------------------------------------------------------------
// Local variable
//--------------------------------------------------------------------------------------------------
static TspModHandle TspDev=
{
    .s32TspMajor=               MDRV_MAJOR_TSP,
    .s32TspMinor=               MDRV_MINOR_TSP,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_TSP, },
        .owner  =               THIS_MODULE,
    },
    .TspFop=
    {
        .open=                  _mod_tsp_open,
        .release=               _mod_tsp_release,
        .read=                  _mod_tsp_read,
        .write=                 _mod_tsp_write,
        .poll=                  _mod_tsp_poll,
        .ioctl=                 _mod_tsp_ioctl,
    },
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Local functions
/////////////////////////////////////////////////////////////////////////////////////////////////////

//// Helper functions

static void     _MDrv_TSP_PidFlt_Disable(U32 u32EngId, U32 u32PidFltId);
static void     _MDrv_TSP_SecFlt_Init(U32 u32EngId, U32 u32SecFltId);
static B16      _MDrv_TSP_SecFlt_Alloc(U32 u32EngId, DrvTSP_FltType eFilterType, U32 *pu32SecFltId);
static B16      _MDrv_TSP_SecFlt_Free(U32 u32EngId, U32 u32SecFltId);
static B16      _MDrv_TSP_SecFlt_StateGet(U32 u32EngId, U32 u32SecFltId, DrvTSP_FltState *peState);
static void     _MDrv_TSP_CmdQ_Init(void);
static void     _MDrv_TSP_PidFlt_Init(U32 u32EngId, U32 u32PidFltId);
static DRVTSP_RESULT _MDrv_TSP_PidFlt_Free(U32 u32EngId, U32 u32PidFltId);
static B16      _MDrv_TSP_PidFlt_StateGet(U32 u32EngId, U32 u32PidFltId, DrvTSP_FltState *peState);
static B16      _MDrv_TSP_PidFlt_Alloc(U32 u32EngId, DrvTSP_FltType eFilterType, U32 *pu32PidFltId);

static void     _MDrv_TSP_Close(void);
static void     _MDrv_TSP_SelPad(U32 u32EngId, DrvTSP_PadIn ePad);
static U8       _MDrv_TSP_SecFlt_Type_ID(U32 u32DrvType);

//// Primary functions which connect to OS interface via MDRv_TSP_IO functions.

static DRVTSP_RESULT MDrv_TSP_PidFlt_Alloc(U32 u32EngId, DrvTSP_FltType eFilterType, U32 *pu32PidFltId);
static DRVTSP_RESULT MDrv_TSP_PidFlt_Free(U32 u32EngId, U32 u32PidFltId);
static DRVTSP_RESULT MDrv_TSP_PidFlt_SetSource(U32 u32EngId, U32 u32PidFltId, U32 u32Src);
static DRVTSP_RESULT MDrv_TSP_PidFlt_GetSource(U32 u32EngId, U32 u32PidFltId, U32* u32Src);
static DRVTSP_RESULT MDrv_TSP_PidFlt_SetPid(U32 u32EngId, U32 u32PidFltId, U32 u32PID);
static DRVTSP_RESULT MDrv_TSP_PidFlt_GetPid(U32 u32EngId, U32 u32PidFltId, U32 *pu32PID);
static DRVTSP_RESULT MDrv_TSP_PidFlt_SelSecFlt(U32 u32EngId, U32 u32PidFltId, U32 u32SecFltId);
static DRVTSP_RESULT MDrv_TSP_PidFlt_Enable(U32 u32EngId, U32 u32PidFltId, B16 bEnable);
static DRVTSP_RESULT MDrv_TSP_PidFlt_GetState(U32 u32EngId, U32 u32PidFltId, DrvTSP_FltState *peState);
static DRVTSP_RESULT MDrv_TSP_PidFlt_ScmbStatus(U32 u32EngId, U32 u32PidFltId, DrvTSP_Scmb_Level* pScmbLevel);

// Richard: @FIXME:
// SEC_FLT_CTRL->FilterType has not been set at the current stage.
// The default value is always section.
// The possible value could be section/PES/TS_packet/AF_only
static DRVTSP_RESULT MDrv_TSP_SecFlt_Alloc(U32 u32EngId, U32 *pu32SecFltId);
static DRVTSP_RESULT MDrv_TSP_SecFlt_Free(U32 u32EngId, U32 u32SecFltId);
static DRVTSP_RESULT MDrv_TSP_SecFlt_SetMode(U32 u32EngId, U32 u32SecFltId, DrvTSP_FltMode eSecFltMode);
static DRVTSP_RESULT MDrv_TSP_SecFlt_SetPattern(U32 u32EngId, U32 u32SecFltId, U8 *pu8Match, U8 *pu8Mask);
static DRVTSP_RESULT MDrv_TSP_SecFlt_ResetBuffer(U32 u32EngId, U32 u32SecFltId);
static DRVTSP_RESULT MDrv_TSP_SecFlt_SetBuffer(U32 u32EngId, U32 u32SecFltId, U32 u32StartAddr, U32 u32BufSize);
static DRVTSP_RESULT MDrv_TSP_SecFlt_SetReqCount(U32 u32EngId, U32 u32SecFltId, U32 u32ReqCount);
static DRVTSP_RESULT MDrv_TSP_SecFlt_GetBufStart(U32 u32EngId, U32 u32SecFltId, U32 *pu32BufStart);
static DRVTSP_RESULT MDrv_TSP_SecFlt_GetBufSize(U32 u32EngId, U32 u32SecFltId, U32 *pu32BufSize);
static DRVTSP_RESULT MDrv_TSP_SecFlt_GetReadAddr(U32 u32EngId, U32 u32SecFltId, U32 *pu32ReadAddr);
static DRVTSP_RESULT MDrv_TSP_SecFlt_GetWriteAddr(U32 u32EngId, U32 u32SecFltId, U32 *pu32WriteAddr);
static DRVTSP_RESULT MDrv_TSP_SecFlt_SetReadAddr(U32 u32EngId, U32 u32SecFltId, U32 u32ReadAddr);
static DRVTSP_RESULT MDrv_TSP_SecFlt_GetCRC32(U32 u32EngId, U32 u32SecFltId, U32 *pu32CRC32);
static DRVTSP_RESULT MDrv_TSP_SecFlt_GetState(U32 u32EngId, U32 u32SecFltId, DrvTSP_FltState *peState);
static DRVTSP_RESULT MDrv_TSP_SecFlt_Notify(U32 u32EngId, U32 u32SecFltId, DrvTSP_Event eEvents, P_DrvTSP_EvtCallback pfCallback);
static DRVTSP_RESULT MDrv_TSP_SecFlt_GetCtrl(U32 u32EngId, U32 u32SecFltId, U32 *pu32Ctrl);

static DRVTSP_RESULT MDrv_TSP_PVR_SetBuffer(U32 u32BufStart0, U32 u32BufStart1, U32 u32BufSize);
static DRVTSP_RESULT MDrv_TSP_PVR_Start(DrvTSP_RecMode eRecMode, B16 bStart);
static DRVTSP_RESULT MDrv_TSP_PVR_GetWriteAddr(U32 *pu32WriteAddr);
static DRVTSP_RESULT MDrv_TSP_PVR_Notify(DrvTSP_Event eEvents, P_DrvTSP_EvtCallback pfCallback);
static DRVTSP_RESULT MDrv_TSP_PVR_TimeStampSetRecordStamp(U32 u32Stamp);
static DRVTSP_RESULT MDrv_TSP_PVR_TimeStampGetRecordStamp(U32* u32Stamp);
static DRVTSP_RESULT MDrv_TSP_PVR_TimeStampSetPlaybackStamp(U32 u32Stamp);
static DRVTSP_RESULT MDrv_TSP_PVR_TimeStampGetPlaybackStamp(U32* u32Stamp);
static DRVTSP_RESULT MDrv_TSP_PVR_TimeStampFileinEnable(B16 bEnable);
static DRVTSP_RESULT MDrv_TSP_PVR_TimeStampGetFileinStamp(U32* u32Stamp);
static DRVTSP_RESULT MDrv_TSP_PVR_TimeStampRecord(B16 bEnable);

static DRVTSP_RESULT MDrv_TSP_MLink_Start(U32 bStart);

static DRVTSP_RESULT MDrv_TSP_M2T_SetAddr(U32 u32StreamAddr);
static DRVTSP_RESULT MDrv_TSP_M2T_SetSize(U32 u32StreamSize);
static DRVTSP_RESULT MDrv_TSP_M2T_SetSTC(U32 u32EngId, U32 u32STC_32, U32 u32STC);
static DRVTSP_RESULT MDrv_TSP_M2T_Start(DrvTSP_M2tMode eM2tMode);
static DRVTSP_RESULT MDrv_TSP_M2T_Pause(void);
static DRVTSP_RESULT MDrv_TSP_M2T_Resume(void);
static DRVTSP_RESULT MDrv_TSP_M2T_GetReadPtr(U32* u32ReadPtr);
static DRVTSP_RESULT MDrv_TSP_GetM2tState(DrvTSP_M2tState *peM2tState);
static DRVTSP_RESULT MDrv_TSP_SetM2tRate(U32 u32Div2);
static DRVTSP_RESULT MDrv_TSP_GetM2tSlot(U32 *pu32EmptySlot);
static DRVTSP_RESULT MDrv_TSP_GetM2tWriteLevel(U32 *pu32Level);
static DRVTSP_RESULT MDrv_TSP_M2T_Reset(void);
static DRVTSP_RESULT MDrv_TSP_GetSTC(U32 u32EngId, U32 *pu32STC_32, U32 *pu32STC);
static DRVTSP_RESULT MDrv_TSP_SetMode(U32 u32EngId, DrvTSP_CtrlMode eCtrlMode);
static DRVTSP_RESULT MDrv_TSP_SelPad(U32 u32EngId, DrvTSP_PadIn ePad);
static DRVTSP_RESULT MDrv_TSP_SetTSIFType(U32 u32EngId, U32 u32TSIFType);
static DRVTSP_RESULT MDrv_TSP_File_SetPacketMode(DrvTSP_PacketMode PKT_Mode);

static DRVTSP_RESULT MDrv_TSP_GetLastErr(void);
//[OBSOLETE] DRVTSP_RESULT MDrv_TSP_PowerOn(void);
//[OBSOLETE] DRVTSP_RESULT MDrv_TSP_PowerOff(void);
static DRVTSP_RESULT MDrv_TSP_Init(void);
static DRVTSP_RESULT MDrv_TSP_PowerCtrl(B16 bOn);
//[OBSOLETE] DRVTSP_RESULT MDrv_TSP_Close(void);

static DRVTSP_RESULT MDrv_TSP_Scmb_Status(U32 u32EngId, DrvTSP_Scmb_Level* pScmbLevel);

static DRVTSP_RESULT MDrv_TSP_GetCFG(U32 u32CfgId, U32* pu32Status, U32* pu32Value);

static DRVTSP_RESULT MDrv_TSP_CSA_SetScmbPath(U32 u32EngId, DrvTSP_CSA_DataPath eInPath, DrvTSP_CSA_DataPath eOutPath, B16 bEnable);
static DRVTSP_RESULT MDrv_TSP_CSA_SetEsaMode(U32 u32EngId, DrvTSP_CSA_Protocol eProtocol);
static DRVTSP_RESULT MDrv_TSP_CSA_CW_SetKey(U32 u32EngId, DrvTSP_CSA_Protocol eProtocol, DrvTSP_CSA_Key eKey, U32 u32FID, U32* pau32CipherKeys);

static DRVTSP_RESULT MDrv_TSP_Fifo_Flush(DrvTSP_FltType eFilterType, B16 bFlush);
static DRVTSP_RESULT MDrv_TSP_Fifo_Fetch(DrvTSP_FltType eFilterType, U8* bByte);

static DRVTSP_RESULT MDrv_TSP_GetPcr(U32 u32EngId, U32 *pu32Pcr_32, U32 *pu32PCr);

/////////////////////////////////////////////////////////////////////////////////////////////////////
// IOCTL argument intepretation and function dispatch
/////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int MDrv_TSP_IO_Signal(unsigned long arg)
{
    DrvTSP_Signal_t*            pTspSignal= (DrvTSP_Signal_t*)(arg);

    pTspSignal->u32EngId=       0;
    pTspSignal->u32Event=           _u32TspEvent;
    pTspSignal->u32EventSecRdy=     _u32TspSecRdy;
    pTspSignal->u32EventSecOvf=     _u32TspSecOvf;

    disable_irq(E_IRQ_TSP);
    _u32TspEvent=               0;
    _u32TspSecRdy=              0;
    _u32TspSecOvf=              0;
    enable_irq(E_IRQ_TSP);

    return 0;
}

static inline int MDrv_TSP_IO_PidFltAlloc(unsigned long arg)
{
    DrvTSP_PidFlt_Alloc_t*      pPidFltAlloc= (DrvTSP_PidFlt_Alloc_t*)(arg);

    return MDrv_TSP_PidFlt_Alloc((U32)pPidFltAlloc->u32EngId,
                                 (DrvTSP_FltType)pPidFltAlloc->eFilterType,
                                 (U32*)&(pPidFltAlloc->u32PidFltId));
}

static inline int MDrv_TSP_IO_PidFlt_Free(unsigned long arg)
{
    DrvTSP_PidFlt_Free_t*       pPidFltFree= (DrvTSP_PidFlt_Free_t*)(arg);

    return MDrv_TSP_PidFlt_Free((U32)pPidFltFree->u32EngId,
                                 (U32)pPidFltFree->u32PidFltId);
}

static inline int MDrv_TSP_IO_PidFlt_SetSource(unsigned long arg)
{
    DrvTSP_PidFlt_Set_Src_t*  pPidFltSetSrc= (DrvTSP_PidFlt_Set_Src_t*)(arg);

    return MDrv_TSP_PidFlt_SetSource((U32)pPidFltSetSrc->u32EngId,
                                  (U32)pPidFltSetSrc->u32PidFltId,
                                  (U32)pPidFltSetSrc->u32Src);

}

static inline int MDrv_TSP_IO_PidFlt_GetSource(unsigned long arg)
{
    DrvTSP_PidFlt_Set_Src_t*  pPidFltSetSrc= (DrvTSP_PidFlt_Set_Src_t*)(arg);
    return MDrv_TSP_PidFlt_GetSource((U32)pPidFltSetSrc->u32EngId,
                                  (U32)pPidFltSetSrc->u32PidFltId,
                                  (U32*)&(pPidFltSetSrc->u32Src));
}

static inline int MDrv_TSP_IO_PidFlt_SetPid(unsigned long arg)
{
    DrvTSP_PidFlt_Pid_t*        pPidFltSet= (DrvTSP_PidFlt_Pid_t*)(arg);

    return MDrv_TSP_PidFlt_SetPid((U32)pPidFltSet->u32EngId,
                                  (U32)pPidFltSet->u32PidFltId,
                                  (U32)pPidFltSet->u32Pid);
}

static inline int MDrv_TSP_IO_PidFlt_GetPid(unsigned long arg)
{
    DrvTSP_PidFlt_Pid_t*        pPidFltGet= (DrvTSP_PidFlt_Pid_t*)(arg);

    return MDrv_TSP_PidFlt_GetPid((U32)pPidFltGet->u32EngId,
                                  (U32)pPidFltGet->u32PidFltId,
                                  (U32*)&(pPidFltGet->u32Pid));
}

static inline int MDrv_TSP_IO_PidFlt_SelSecFlt(unsigned long arg)
{
    DrvTSP_PidFlt_SelSecFlt_t*  pPidFltSelSecFlt= (DrvTSP_PidFlt_SelSecFlt_t*)(arg);

    return MDrv_TSP_PidFlt_SelSecFlt((U32)pPidFltSelSecFlt->u32EngId,
                                     (U32)pPidFltSelSecFlt->u32PidFltId,
                                     (U32)pPidFltSelSecFlt->u32SecFltId);
}

static inline int MDrv_TSP_IO_PidFlt_Enable(unsigned long arg)
{
    DrvTSP_PidFlt_Enable_t*     pPidFltEnable= (DrvTSP_PidFlt_Enable_t*)(arg);

    return MDrv_TSP_PidFlt_Enable((U32)pPidFltEnable->u32EngId,
                                  (U32)pPidFltEnable->u32PidFltId,
                                  (B16)pPidFltEnable->bEnable);
}

static inline int MDrv_TSP_IO_PidFlt_GetState(unsigned long arg)
{
    DrvTSP_PidFlt_State_t*      pPidFltState= (DrvTSP_PidFlt_State_t*)(arg);

    return MDrv_TSP_PidFlt_GetState((U32)pPidFltState->u32EngId,
                                    (U32)pPidFltState->u32PidFltId,
                                    (DrvTSP_FltState*)&(pPidFltState->eFltState));
}

static inline int MDrv_TSP_IO_PidFlt_ScmbStatus(unsigned long arg)
{
    DrvTSP_PidFlt_Scmb_Status_t* pPidFltScmbStatus= (DrvTSP_PidFlt_Scmb_Status_t*)(arg);

    return MDrv_TSP_PidFlt_ScmbStatus((U32)pPidFltScmbStatus->u32EngId,
                                      (U32)pPidFltScmbStatus->u32PidFltId,
                                      (DrvTSP_Scmb_Level*)&(pPidFltScmbStatus->eScmbLevel));
}

static inline int MDrv_TSP_IO_SecFlt_Alloc(unsigned long arg)
{
    DrvTSP_SecFlt_Alloc_t*      pSecFltAlloc= (DrvTSP_SecFlt_Alloc_t*)arg;

    return MDrv_TSP_SecFlt_Alloc((U32)pSecFltAlloc->u32EngId,
                                 (U32*)&(pSecFltAlloc->u32SecFltId));
}

static inline int MDrv_TSP_IO_SecFlt_Free(unsigned long arg)
{
    DrvTSP_SecFlt_Free_t*       pSecFltFree= (DrvTSP_SecFlt_Free_t*)arg;

    return MDrv_TSP_SecFlt_Free((U32)pSecFltFree->u32EngId,
                                (U32)pSecFltFree->u32SecFltId);
}

static inline int MDrv_TSP_IO_SecFlt_SetMode(unsigned long arg)
{
    DrvTSP_SecFlt_Mode_t*       pSecFltMode= (DrvTSP_SecFlt_Mode_t*)(arg);

    return MDrv_TSP_SecFlt_SetMode((U32)pSecFltMode->u32EngId,
                                   (U32)pSecFltMode->u32SecFltId,
                                   (DrvTSP_FltMode)pSecFltMode->eSecFltMode);
}

static inline int MDrv_TSP_IO_SecFlt_SetPattern(unsigned long arg)
{
    U8 u8Match[TSP_FILTER_DEPTH];
    U8 u8Mask[TSP_FILTER_DEPTH];

    DrvTSP_SecFlt_Pattern_t*    pSecFltPattern= (DrvTSP_SecFlt_Pattern_t*)(arg);

    if (TSP_FILTER_DEPTH != pSecFltPattern->u32FltDepth)
    {
        return DRVTSP_FAIL;
    }

    if( copy_from_user(u8Match, pSecFltPattern->pu8Match, TSP_FILTER_DEPTH) )
    {   // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        return DRVTSP_FAIL;
    }

    if( copy_from_user(u8Mask, pSecFltPattern->pu8Mask, TSP_FILTER_DEPTH) )
    {   // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        return DRVTSP_FAIL;
    }

    return MDrv_TSP_SecFlt_SetPattern((U32)pSecFltPattern->u32EngId,
                                      (U32)pSecFltPattern->u32SecFltId,
                                      (U8*)u8Match,
                                      (U8*)u8Mask);
}

static inline int MDrv_TSP_IO_SecFlt_SetReqCount(unsigned long arg)
{
    DrvTSP_SecFlt_ReqCount_t*   pSecFltReqCount= (DrvTSP_SecFlt_ReqCount_t*)(arg);

    return MDrv_TSP_SecFlt_SetReqCount((U32)pSecFltReqCount->u32EngId,
                                       (U32)pSecFltReqCount->u32SecFltId,
                                       (U32)pSecFltReqCount->u32ReqCount);
}

static inline int MDrv_TSP_IO_SecFlt_ResetBuffer(unsigned long arg)
{
    DrvTSP_SecFlt_Buffer_Reset* pSecFltBufReset= (DrvTSP_SecFlt_Buffer_Reset*)(arg);

    return MDrv_TSP_SecFlt_ResetBuffer((U32)pSecFltBufReset->u32EngId,
                                       (U32)pSecFltBufReset->u32SecFltId);
}

static inline int MDrv_TSP_IO_SecFlt_SetBuffer(unsigned long arg)
{
    DrvTSP_SecFlt_Buffer_Set_t* pSecFltBufSet= (DrvTSP_SecFlt_Buffer_Set_t*)(arg);

    return MDrv_TSP_SecFlt_SetBuffer((U32)pSecFltBufSet->u32EngId,
                                     (U32)pSecFltBufSet->u32SecFltId,
                                     (U32)pSecFltBufSet->u32BufAddr,
                                     (U32)pSecFltBufSet->u32BufSize);
}

static inline int MDrv_TSP_IO_SetFlt_GetBufStart(unsigned long arg)
{
    DrvTSP_SecFlt_Buffer_Addr_Get_t*    pSecFltBufAddrGet= (DrvTSP_SecFlt_Buffer_Addr_Get_t*)(arg);

    return MDrv_TSP_SecFlt_GetBufStart((U32)pSecFltBufAddrGet->u32EngId,
                                       (U32)pSecFltBufAddrGet->u32SecFltId,
                                       (U32*)&(pSecFltBufAddrGet->u32BufAddr));
}

static inline int MDrv_TSP_IO_SecFlt_GetBufSize(unsigned long arg)
{
    DrvTSP_SecFlt_Buffer_Size_Get_t*    pSecFltBufSizeGet= (DrvTSP_SecFlt_Buffer_Size_Get_t*)(arg);

    return MDrv_TSP_SecFlt_GetBufSize((U32)pSecFltBufSizeGet->u32EngId,
                                      (U32)pSecFltBufSizeGet->u32SecFltId,
                                      (U32*)&(pSecFltBufSizeGet->u32BufSize));
}

static inline int MDrv_TSP_IO_SecFlt_GetReadAddr(unsigned long arg)
{
    DrvTSP_SecFlt_Buffer_Read_Get_t*    pSecFltBufReadGet= (DrvTSP_SecFlt_Buffer_Read_Get_t*)(arg);

    return MDrv_TSP_SecFlt_GetReadAddr((U32)pSecFltBufReadGet->u32EngId,
                                       (U32)pSecFltBufReadGet->u32SecFltId,
                                       (U32*)&(pSecFltBufReadGet->u32Read));
}

static inline int MDrv_TSP_IO_SecFlt_SetReadAddr(unsigned long arg)
{
    DrvTSP_SecFlt_Buffer_Read_Set_t*    pSecFltBufReadSet= (DrvTSP_SecFlt_Buffer_Read_Set_t*)(arg);

    return MDrv_TSP_SecFlt_SetReadAddr((U32)pSecFltBufReadSet->u32EngId,
                                       (U32)pSecFltBufReadSet->u32SecFltId,
                                       (U32)pSecFltBufReadSet->u32Read);
}

static inline int MDrv_TSP_IO_SecFlt_GetWriteAddr(unsigned long arg)
{
    DrvTSP_SecFlt_Buffer_Write_Get_t*   pSecFltBufWriteGet= (DrvTSP_SecFlt_Buffer_Write_Get_t*)(arg);

    return MDrv_TSP_SecFlt_GetWriteAddr((U32)pSecFltBufWriteGet->u32EngId,
                                        (U32)pSecFltBufWriteGet->u32SecFltId,
                                        (U32*)&(pSecFltBufWriteGet->u32Write));
}

static inline int MDrv_TSP_IO_SecFlt_GetCRC32(unsigned long arg)
{
    DrvTSP_SecFlt_Crc32_t*      pSecFltBufCrc32= (DrvTSP_SecFlt_Crc32_t*)(arg);

    return MDrv_TSP_SecFlt_GetCRC32((U32)pSecFltBufCrc32->u32EngId,
                                    (U32)pSecFltBufCrc32->u32SecFltId,
                                    (U32*)&(pSecFltBufCrc32->u32Crc));
}

static inline int MDrv_TSP_IO_SecFlt_Notify(unsigned long arg)
{
    DrvTSP_SecFlt_Notify_t*     pSecFltNotify= (DrvTSP_SecFlt_Notify_t*)(arg);

    return MDrv_TSP_SecFlt_Notify((U32)pSecFltNotify->u32EngId,
                                  (U32)pSecFltNotify->u32SecFltId,
                                  (DrvTSP_Event)pSecFltNotify->eEvents,
                                  NULL);
}

static inline int MDrv_TSP_IO_SecFlt_StateGet(unsigned long arg)
{
    DrvTSP_SecFlt_State_t*     pSecFltStateGet= (DrvTSP_SecFlt_State_t*)(arg);

    return MDrv_TSP_SecFlt_GetState((U32)pSecFltStateGet->u32EngId,
                                    (U32)pSecFltStateGet->u32SecFltId,
                                    (DrvTSP_FltState*)&(pSecFltStateGet->eState));
}

static inline int MDrv_TSP_IO_SecFlt_GetCtrl(unsigned long arg)
{
    DrvTSP_SecFlt_Crc32_t*      pSecFltCtrl= (DrvTSP_SecFlt_Crc32_t*)(arg);
    return MDrv_TSP_SecFlt_GetCtrl((U32)pSecFltCtrl->u32EngId,
                                   (U32)pSecFltCtrl->u32SecFltId,
                                   (U32*)&(pSecFltCtrl->u32Crc));
}

static inline int MDrv_TSP_IO_PVR_SetBuffer(unsigned long arg)
{
    DrvTSP_Pvr_Buffer_Set_t*    pPvrBufSet= (DrvTSP_Pvr_Buffer_Set_t*)(arg);

    return MDrv_TSP_PVR_SetBuffer((U32)pPvrBufSet->u32BufAddr0,
                                  (U32)pPvrBufSet->u32BufAddr1,
                                  (U32)pPvrBufSet->u32BufSize);
}

static inline int MDrv_TSP_IO_PVR_Start(unsigned long arg)
{
    DrvTSP_Pvr_Start_t*         pPvrStart= (DrvTSP_Pvr_Start_t*)(arg);

    return MDrv_TSP_PVR_Start((DrvTSP_RecMode)pPvrStart->eRecMode,
                              (B16)pPvrStart->bStart);
}

static inline int MDrv_TSP_IO_MLink_Start(unsigned long arg)
{
    return MDrv_TSP_MLink_Start(*((U32*)arg));
}

static inline int MDrv_TSP_IO_PVR_GetWriteAddr(unsigned long arg)
{
    return MDrv_TSP_PVR_GetWriteAddr((U32*)arg);
}

static inline int MDrv_TSP_IO_PVR_Notify(unsigned long arg)
{
    return MDrv_TSP_PVR_Notify(*((DrvTSP_Event*)arg), NULL);
}

static inline int MDrv_TSP_IO_PVR_SetRecStamp(unsigned long arg)
{
    return MDrv_TSP_PVR_TimeStampSetRecordStamp(*(U32*)arg);
}

static inline int MDrv_TSP_IO_PVR_GetRecStamp(unsigned long arg)
{
    return MDrv_TSP_PVR_TimeStampGetRecordStamp((U32*)arg);
}

static inline int MDrv_TSP_IO_PVR_SetPlayStamp(unsigned long arg)
{
    return MDrv_TSP_PVR_TimeStampSetPlaybackStamp(*(U32*)arg);
}

static inline int MDrv_TSP_IO_PVR_GetPlayStamp(unsigned long arg)
{
    return MDrv_TSP_PVR_TimeStampGetPlaybackStamp((U32*)arg);
}

static inline int MDrv_TSP_IO_PVR_EnableFileinTimeStamp(unsigned long arg)
{
    return MDrv_TSP_PVR_TimeStampFileinEnable(*(B16*)arg);
}

static inline int MDrv_TSP_IO_PVR_GetFileinStamp(unsigned long arg)
{
    return MDrv_TSP_PVR_TimeStampGetFileinStamp((U32*)arg);
}

static inline int MDrv_TSP_IO_PVR_TimeStampRecord(unsigned long arg)
{
    return MDrv_TSP_PVR_TimeStampRecord(*(B16*)arg);
}

static inline int MDrv_TSP_IO_M2T_SetAddr(unsigned long arg)
{
    return MDrv_TSP_M2T_SetAddr(*((U32*)arg));
}

static inline int MDrv_TSP_IO_M2T_GetReadPtr(unsigned long arg)
{
    return MDrv_TSP_M2T_GetReadPtr((U32*)arg);
}

static inline int MDrv_TSP_IO_M2T_SetSize(unsigned long arg)
{
    return MDrv_TSP_M2T_SetSize(*((U32*)arg));
}

static inline int MDrv_TSP_IO_M2T_SetSTC(unsigned long arg)
{
    DrvTSP_M2T_Stc_Set_t*       pM2TStcSet= (DrvTSP_M2T_Stc_Set_t*)(arg);

    return MDrv_TSP_M2T_SetSTC((U32)pM2TStcSet->u32EngId,
                               (U32)pM2TStcSet->u32STC_32,
                               (U32)pM2TStcSet->u32STC);
}

static inline int MDrv_TSP_IO_M2T_Start(unsigned long arg)
{
    return MDrv_TSP_M2T_Start(*((DrvTSP_M2tMode*)(arg)));
}

static inline int MDrv_TSP_IO_M2T_Pause(unsigned long arg)
{
    return MDrv_TSP_M2T_Pause();
}

static inline int MDrv_TSP_IO_M2T_Resume(unsigned long arg)
{
    return MDrv_TSP_M2T_Resume();
}

static inline int MDrv_TSP_IO_GetM2tState(unsigned long arg)
{
    return MDrv_TSP_GetM2tState(((DrvTSP_M2tState*)arg));
}

static inline int MDrv_TSP_IO_SetM2tRate(unsigned long arg)
{
    return MDrv_TSP_SetM2tRate(*((U32*)(arg)));
}

static inline int MDrv_TSP_IO_GetM2tSlot(unsigned long arg)
{
    return MDrv_TSP_GetM2tSlot((U32*)(arg));
}

static inline int MDrv_TSP_IO_GetM2tWriteLevel(unsigned long arg)
{
    return MDrv_TSP_GetM2tWriteLevel((U32*)(arg));
}

static inline int MDrv_TSP_IO_M2T_Reset(unsigned long arg)
{
    return MDrv_TSP_M2T_Reset();
}

static inline int MDrv_TSP_IO_GetSTC(unsigned long arg)
{
    DrvTSP_Stc_Get_t*           pStcGet= (DrvTSP_Stc_Get_t*)(arg);

    return MDrv_TSP_GetSTC((U32) pStcGet->u32EngId,
                           (U32*)&(pStcGet->u32STC_32),
                           (U32*)&(pStcGet->u32STC));
}

static inline int MDrv_TSP_IO_SetMode(unsigned long arg)
{
    DrvTSP_Set_Mode_t*          pTspSetMode= (DrvTSP_Set_Mode_t*)(arg);

    return MDrv_TSP_SetMode((U32)pTspSetMode->u32EngId,
                            (DrvTSP_CtrlMode)pTspSetMode->eCtrlMode);
}

static inline int MDrv_TSP_IO_SelPad(unsigned long arg)
{
    DrvTSP_Sel_Pad_t*           pSelPad= (DrvTSP_Sel_Pad_t*)arg;

    return MDrv_TSP_SelPad((U32)pSelPad->u32EngId,
                           (DrvTSP_PadIn)pSelPad->ePad);
}

static inline int MDrv_TSP_IO_Power(unsigned long arg)
{
    return MDrv_TSP_PowerCtrl(*((U32*)(arg)));
    // to allow selct power on off in the future
    //return MDrv_TSP_Close();
}

static inline int MDrv_TSP_IO_LastErr(unsigned long arg)
{
    return MDrv_TSP_GetLastErr();
}

static inline int MDrv_TSP_IO_Init(unsigned long arg)
{
    return MDrv_TSP_Init();
}

static inline int MDrv_TSP_IO_Scmb_Status(unsigned long arg)
{
    DrvTSP_Scmb_Status_t*       pScmbStatus= (DrvTSP_Scmb_Status_t*)(arg);

    return MDrv_TSP_Scmb_Status((U32)pScmbStatus->u32EngId,
                                (DrvTSP_Scmb_Level*)&(pScmbStatus->eScmbLevel));
}

static inline int MDrv_TSP_IO_Set_TSIF_Type(unsigned long arg)
{
    DrvTSP_Set_TSIF_Type_t*   pSetTSIFType = (DrvTSP_Set_TSIF_Type_t*)(arg);
    return MDrv_TSP_SetTSIFType(pSetTSIFType->u32EngId, pSetTSIFType->u32TSIFType);
}

static inline int MDrv_TSP_IO_SetPacketMode(unsigned long arg)
{
    return MDrv_TSP_File_SetPacketMode(*(U32*)arg);
}

static inline int MDrv_TSP_IO_GetCFG(unsigned long arg)
{
    DrvTSP_Cfg_t* pCfg = (DrvTSP_Cfg_t*)(arg);

    return MDrv_TSP_GetCFG(pCfg->u32CfgId,
                             &(pCfg->u32Status),
                             &(pCfg->u32Value));
}

static inline int MDrv_TSP_IO_Set_ScmbPath(unsigned long arg)
{
    DrvTSP_Set_Scmb_Path_t* pScmbPath = (DrvTSP_Set_Scmb_Path_t*)arg;
    return MDrv_TSP_CSA_SetScmbPath(pScmbPath->u32EngId,
                                    pScmbPath->eInPath,
                                    pScmbPath->eOutPath,
                                    pScmbPath->bEnableOutputAV);
    //return MDrv_TSP_CSA_Enable();
}

static inline int MDrv_TSP_IO_Set_EsaMode(unsigned long arg)
{
    DrvTSP_Set_Esa_Mode_t* pEsaMode = (DrvTSP_Set_Esa_Mode_t*)arg;
    return MDrv_TSP_CSA_SetEsaMode(pEsaMode->u32EngId, pEsaMode->eProtocol);
}

static inline int MDrv_TSP_IO_Set_CipherKey(unsigned long arg)
{
    #define CSA_PIDFLT_IDX_OFFSET 16
    //U32 u32Idx;

    DrvTSP_Set_CipherKey_t* pCipherKey = (DrvTSP_Set_CipherKey_t*)arg;
    DRVTSP_RESULT eRet;
    /*return MDrv_TSP_CSA_CW_SetKey(pCipherKey->u32EngId,
                                  pCipherKey->eProtocol,
                                  pCipherKey->eKey,
                                  pCipherKey->u32FID,
                                  pCipherKey->pu32CipherKey);
    */
    //video
    eRet = MDrv_TSP_CSA_CW_SetKey(pCipherKey->u32EngId,
                                  pCipherKey->eProtocol,
                                  pCipherKey->eKey,
                                  _u8IdxStream[pCipherKey->u32EngId][0] - CSA_PIDFLT_IDX_OFFSET,
                                  pCipherKey->pu32CipherKey);
    if (eRet != DRVTSP_OK)
    {

    }
    //audio
    eRet = MDrv_TSP_CSA_CW_SetKey(pCipherKey->u32EngId,
                                  pCipherKey->eProtocol,
                                  pCipherKey->eKey,
                                  _u8IdxStream[pCipherKey->u32EngId][1] - CSA_PIDFLT_IDX_OFFSET,
                                  pCipherKey->pu32CipherKey);
    if (eRet != DRVTSP_OK)
    {

    }
    //audio2
    eRet = MDrv_TSP_CSA_CW_SetKey(pCipherKey->u32EngId,
                                  pCipherKey->eProtocol,
                                  pCipherKey->eKey,
                                  _u8IdxStream[pCipherKey->u32EngId][2] - CSA_PIDFLT_IDX_OFFSET,
                                  pCipherKey->pu32CipherKey);
    if (eRet != DRVTSP_OK)
    {

    }
    //pcr
    eRet = MDrv_TSP_CSA_CW_SetKey(pCipherKey->u32EngId,
                                  pCipherKey->eProtocol,
                                  pCipherKey->eKey,
                                  _u8IdxStream[pCipherKey->u32EngId][3] - CSA_PIDFLT_IDX_OFFSET,
                                  pCipherKey->pu32CipherKey);
    if (eRet != DRVTSP_OK)
    {

    }
#if 0
    //pes cip - 2009.09.23
    for (u32Idx = CSA_PIDFLT_IDX_OFFSET; u32Idx < (TSP_PIDFLT_NUM - RESERVED_PIDFLT_NUM); ++u32Idx)
    {
        if (//_TspState[pCipherKey->u32EngId].FltState[u32Idx] != E_DRVTSP_FLT_STATE_FREE &&
            _TspState[pCipherKey->u32EngId].FltType[u32Idx] == E_DRVTSP_FLT_TYPE_PES)
        {
            eRet = MDrv_TSP_CSA_CW_SetKey(pCipherKey->u32EngId,
                                          pCipherKey->eProtocol,
                                          pCipherKey->eKey,
                                          u32Idx - CSA_PIDFLT_IDX_OFFSET,
                                          pCipherKey->pu32CipherKey);
        }
        if (eRet != DRVTSP_OK)
        {

        }
    }
#else
    //pes
    eRet = MDrv_TSP_CSA_CW_SetKey(pCipherKey->u32EngId,
                                  pCipherKey->eProtocol,
                                  pCipherKey->eKey,
                                  _u8IdxStream[pCipherKey->u32EngId][4] - CSA_PIDFLT_IDX_OFFSET,
                                  pCipherKey->pu32CipherKey);
    if (eRet != DRVTSP_OK)
    {

    }
    eRet = MDrv_TSP_CSA_CW_SetKey(pCipherKey->u32EngId,
                                  pCipherKey->eProtocol,
                                  pCipherKey->eKey,
                                  _u8IdxStream[pCipherKey->u32EngId][5] - CSA_PIDFLT_IDX_OFFSET,
                                  pCipherKey->pu32CipherKey);
    if (eRet != DRVTSP_OK)
    {

    }
#endif

    return ((int)eRet);
}

static inline int MDrv_TSP_IO_Fifo_Flush(unsigned long arg)
{
    DrvTSP_Fifo_Flush_t*    pstFifoFlush = (DrvTSP_Fifo_Flush_t*)arg;
    return MDrv_TSP_Fifo_Flush(pstFifoFlush->eFilterType, pstFifoFlush->bFlush);
}

static inline int MDrv_TSP_IO_Fifo_Fetch(unsigned long arg)
{
    DrvTSP_Fifo_Fetch_t*    pstFifoFetch = (DrvTSP_Fifo_Fetch_t*)arg;
    return MDrv_TSP_Fifo_Fetch(pstFifoFetch->eFilterType, &pstFifoFetch->u8Byte);
}

static inline int MDrv_TSP_IO_GetPcr(unsigned long arg)
{
    DrvTSP_Stc_Get_t*           pStcGet= (DrvTSP_Stc_Get_t*)(arg);

    return MDrv_TSP_GetPcr((U32) pStcGet->u32EngId,
                           (U32*)&(pStcGet->u32STC_32),
                           (U32*)&(pStcGet->u32STC));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// OS interface
//////////////////////////////////////////////////////////////////////////////////////////////////////

static int _mod_tsp_open (struct inode *inode, struct file *filp)
{
    TSP_PRINT("%s is invoked\n", __FUNCTION__);
#if defined(CONFIG_Titania3) //2009.11.10
    spin_lock_init(&_spinlock);
    init_waitqueue_head(&_wait_queue);
    _u32TspEvent =      0;
    _u32TspSecRdy =     0;
    _u32TspSecOvf =     0;
    //_TspState[TSP_ENGINE_NUM];
    //_TspEnable[TSP_ENGINE_NUM];
    _bPvrBuf0 = TRUE;
    //_u32LastErr;
    _b192Mode = FALSE;
    u32TSPIntRegister = 0 ; //LGE
#else
    // MaxCC 20080320 review when to set this
    *((volatile U32*)0xbf803c4c)= 0x0000;
    // MaxCC 20080320 */
#endif
    return 0;
}

static int _mod_tsp_release(struct inode *inode, struct file *filp)
{
    TSP_PRINT("%s is invoked\n", __FUNCTION__);
    MDrv_TSP_PowerCtrl(FALSE);
    return 0;
}

static ssize_t _mod_tsp_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    TSP_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_tsp_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    TSP_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_tsp_poll(struct file *filp, poll_table *wait)
{
    //TSP_PRINT("%s is invoked\n", __FUNCTION__);
    poll_wait(filp, &_wait_queue,  wait);
    return (!_u32TspEvent)? 0: POLLIN; //  | POLLPRI;
}

static int _mod_tsp_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int         err= 0;

    // TSP_PRINT("%s: 0x%02x: is invoked\n", __FUNCTION__, _IOC_NR(cmd));

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (TSP_IOC_MAGIC!= _IOC_TYPE(cmd))
    {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (err)
    {
        return -EFAULT;
    }

    PROBE_IO_ENTRY(MDRV_MAJOR_TSP, _IOC_NR(cmd));

    // @FIXME: Use a array of function pointer for program readable and code size later
    switch(cmd)
    {
    //------------------------------------------------------------------------------
    // Signal
    //------------------------------------------------------------------------------
    case TSP_IOC_SIGNAL:
        err= MDrv_TSP_IO_Signal(arg);
        break;
    //------------------------------------------------------------------------------
    // PidFlt
    //------------------------------------------------------------------------------
    case TSP_IOC_PIDFLT_ALLOC:
        err= MDrv_TSP_IO_PidFltAlloc(arg);
        break;
    case TSP_IOC_PIDFLT_FREE:
        err= MDrv_TSP_IO_PidFlt_Free(arg);
        break;
    case TSP_IOC_PIDFLT_SET_SRC:
        err=MDrv_TSP_IO_PidFlt_SetSource(arg);
        break;
    case TSP_IOC_PIDFLT_GET_SRC:
        err=MDrv_TSP_IO_PidFlt_GetSource(arg);
        break;
    case TSP_IOC_PIDFLT_SET_PID:
        err= MDrv_TSP_IO_PidFlt_SetPid(arg);
        break;
    case TSP_IOC_PIDFLT_GET_PID:
        err= MDrv_TSP_IO_PidFlt_GetPid(arg);
        break;
    case TSP_IOC_PIDFLT_SEL_SECFLT:
        err= MDrv_TSP_IO_PidFlt_SelSecFlt(arg);
        break;
    case TSP_IOC_PIDFLT_ENABLE:
        err= MDrv_TSP_IO_PidFlt_Enable(arg);
        break;
    case TSP_IOC_PIDFLT_STATE:
        err= MDrv_TSP_IO_PidFlt_GetState(arg);
        break;
    case TSP_IOC_PIDFLT_SCMB_STATUS:
        err= MDrv_TSP_IO_PidFlt_ScmbStatus(arg);
        break;
    //------------------------------------------------------------------------------
    // SecFlt
    //------------------------------------------------------------------------------
    case TSP_IOC_SECFLT_ALLOC:
        err= MDrv_TSP_IO_SecFlt_Alloc(arg);
        break;
    case TSP_IOC_SECFLT_FREE:
        err= MDrv_TSP_IO_SecFlt_Free(arg);
        break;
    case TSP_IOC_SECFLT_MODE:
        err= MDrv_TSP_IO_SecFlt_SetMode(arg);
        break;
    case TSP_IOC_SECFLT_PATTERN:
        err= MDrv_TSP_IO_SecFlt_SetPattern(arg);
        break;
    case TSP_IOC_SECFLT_REQCOUNT:
        err= MDrv_TSP_IO_SecFlt_SetReqCount(arg);
        break;
    case TSP_IOC_SECFLT_BUFFER_RESET:
        err= MDrv_TSP_IO_SecFlt_ResetBuffer(arg);
        break;
    case TSP_IOC_SECFLT_BUFFER_SET:
        err= MDrv_TSP_IO_SecFlt_SetBuffer(arg);
        break;
    case TSP_IOC_SECFLT_BUFFER_ADRR:
        err= MDrv_TSP_IO_SetFlt_GetBufStart(arg);
        break;
    case TSP_IOC_SECFLT_BUFFER_SIZE:
        err= MDrv_TSP_IO_SecFlt_GetBufSize(arg);
        break;
    case TSP_IOC_SECFLT_BUFFER_READ_GET:
        err= MDrv_TSP_IO_SecFlt_GetReadAddr(arg);
        break;
    case TSP_IOC_SECFLT_BUFFER_READ_SET:
        err= MDrv_TSP_IO_SecFlt_SetReadAddr(arg);
        break;
    case TSP_IOC_SECFLT_BUFFER_WRITE_GET:
        err= MDrv_TSP_IO_SecFlt_GetWriteAddr(arg);
        break;
    case TSP_IOC_SECFLT_CRC32:
        err= MDrv_TSP_IO_SecFlt_GetCRC32(arg);
        break;
    case TSP_IOC_SECFLT_NOTIFY:
        err= MDrv_TSP_IO_SecFlt_Notify(arg);
        break;
    case TSP_IOC_SECFLT_STATE_GET:
        err= MDrv_TSP_IO_SecFlt_StateGet(arg);
        break;
    case TSP_IOC_SECFLT_CTRL_GET:
        err= MDrv_TSP_IO_SecFlt_GetCtrl(arg);
        break;

    //------------------------------------------------------------------------------
    // PVR
    //------------------------------------------------------------------------------
    case TSP_IOC_PVR_BUFFER_SET:
        err= MDrv_TSP_IO_PVR_SetBuffer(arg);
        break;
    case TSP_IOC_PVR_START:
        err= MDrv_TSP_IO_PVR_Start(arg);
        break;
    case TSP_IOC_PVR_WRITE_GET:
        err= MDrv_TSP_IO_PVR_GetWriteAddr(arg);
        break;
    case TSP_IOC_PVR_NOTIFY:
        err= MDrv_TSP_IO_PVR_Notify(arg);
        break;
    case TSP_IOC_PVR_SET_REC_STAMP:
        err= MDrv_TSP_IO_PVR_SetRecStamp(arg);
        break;
    case TSP_IOC_PVR_GET_REC_STAMP:
        err= MDrv_TSP_IO_PVR_GetRecStamp(arg);
        break;
    case TSP_IOC_PVR_SET_PLAY_STAMP:
        err= MDrv_TSP_IO_PVR_SetPlayStamp(arg);
        break;
    case TSP_IOC_PVR_GET_PLAY_STAMP:
        err= MDrv_TSP_IO_PVR_GetPlayStamp(arg);
        break;
    case TSP_IOC_PVR_STAMP_ENABLE:
        err= MDrv_TSP_IO_PVR_EnableFileinTimeStamp(arg);
        break;
    case TSP_IOC_PVR_GET_FILEIN_STAMP:
        err= MDrv_TSP_IO_PVR_GetFileinStamp(arg);
        break;
    case TSP_IOC_PVR_RECORD_STAMP_ENABLE:
        err= MDrv_TSP_IO_PVR_TimeStampRecord(arg);
        break;

    //------------------------------------------------------------------------------
    // MLink
    //------------------------------------------------------------------------------
    case TSP_IOC_MLINK_START:
        err= MDrv_TSP_IO_MLink_Start(arg);
        break;

    //------------------------------------------------------------------------------
    // File in
    //------------------------------------------------------------------------------
    case TSP_IOC_M2T_BUFFER_ADDR_SET:
        err= MDrv_TSP_IO_M2T_SetAddr(arg);
        break;
    case TSP_IOC_M2T_BUFFER_SIZE_SET:
        err= MDrv_TSP_IO_M2T_SetSize(arg);
        break;
    case TSP_IOC_M2T_STC_SET:
        err= MDrv_TSP_IO_M2T_SetSTC(arg);
        break;
    case TSP_IOC_M2T_START:
        err= MDrv_TSP_IO_M2T_Start(arg);
        break;
    case TSP_IOC_M2T_PAUSE:
        err= MDrv_TSP_IO_M2T_Pause(arg);
        break;
    case TSP_IOC_M2T_RESUME:
        err= MDrv_TSP_IO_M2T_Resume(arg);
        break;
    case TSP_IOC_M2T_STATE:
        err= MDrv_TSP_IO_GetM2tState(arg);
        break;
    case TSP_IOC_M2T_RATE_SET:
        err= MDrv_TSP_IO_SetM2tRate(arg);
        break;
    case TSP_IOC_M2T_SLOT_GET:
        err= MDrv_TSP_IO_GetM2tSlot(arg);
        break;
    case TSP_IOC_M2T_WRLEVEL_GET:
        err= MDrv_TSP_IO_GetM2tWriteLevel(arg);
        break;
    case TSP_IOC_M2T_BUFFER_RP_GET:
        err= MDrv_TSP_IO_M2T_GetReadPtr(arg);
        break;
    case TSP_IOC_M2T_RESET:
        err= MDrv_TSP_IO_M2T_Reset(arg);
        break;
    case TSP_IOC_CFG:
        err = MDrv_TSP_IO_GetCFG(arg);
        break;
#if 0
    //------------------------------------------------------------------------------
    // Dmx Flt
    //------------------------------------------------------------------------------
    case TSP_IOC_DMXFLT_ALLOC:
        err= MDrv_TSP_IO_Dmx_Flt_Alloc(arg);
        break;
    case TSP_IOC_DMXFLT_FREE:
        err= MDrv_TSP_IO_Dmx_Flt_Free(arg);
        break;
    case TSP_IOC_DMXFLT_SET:
        err= MDrv_TSP_IO_Dmx_Flt_Set(arg);
        break;
    case TSP_IOC_DMXFLT_SET_SECBUF:
        err= MDrv_TSP_IO_Dmx_Flt_SecBufSet(arg);
        break;
    case TSP_IOC_DMXFLT_GET_SECBUF:
        err= MDrv_TSP_IO_Dmx_Flt_SecBufGet(arg);
        break;
    case TSP_IOC_DMXFLT_ENABLE:
        err= MDrv_TSP_IO_Dmx_Flt_Enable(arg);
        break;
    case TSP_IOC_DMXFLT_SETMODE:
        err= MDrv_TSP_IO_Dmx_Flt_SetMode(arg);
        break;
    case TSP_IOC_DMXFLT_SETPATTERN:
        err= MDrv_TSP_IO_Dmx_Flt_SetPattern(arg);
        break;
    case TSP_IOC_DMXFLT_GETSTATE:
        err= MDrv_TSP_IO_Dmx_Flt_GetState(arg);
        break;
    case TSP_IOC_DMXFLT_NOTIFY:
        err= MDrv_TSP_IO_Dmx_Flt_Notify(arg);
        break;
    case TSP_IOC_DMXFLT_REQCOUNT:
        err= MDrv_TSP_IO_Dmx_Flt_ReqCount(arg);
        break;
    case TSP_IOC_DMXFLT_CRC32:
        err= MDrv_TSP_IO_Dmx_Flt_Crc32(arg);
        break;
    case TSP_IOC_DMXFLT_RESET_STATE:
        err= MDrv_TSP_IO_Dmx_Flt_ResetState(arg);
        break;

    //------------------------------------------------------------------------------
    // Dmx Sec Buf
    //------------------------------------------------------------------------------
    case TSP_IOC_DMXSECBUF_ALLOC:
        err= MDrv_TSP_IO_Dmx_SecBuf_Alloc(arg);
        break;
    case TSP_IOC_DMXSECBUF_FREE:
        err= MDrv_TSP_IO_Dmx_SecBuf_Free(arg);
        break;
    case TSP_IOC_DMXSECBUF_BUF_SET:
        err= MDrv_TSP_IO_Dmx_SecBuf_Set(arg);
        break;
    case TSP_IOC_DMXSECBUF_BUF_RESET:
        err= MDrv_TSP_IO_Dmx_SecBuf_Reset(arg);
        break;
    case TSP_IOC_DMXSECBUF_BUF_BUF_ADDR:
        err= MDrv_TSP_IO_Dmx_SecBuf_Addr(arg);
        break;
    case TSP_IOC_DMXSECBUF_BUF_BUF_SIZE:
        err= MDrv_TSP_IO_Dmx_SecBuf_Size(arg);
        break;
    case TSP_IOC_DMXSECBUF_BUF_READ_GET:
        err= MDrv_TSP_IO_Dmx_SecBuf_ReadGet(arg);
        break;
    case TSP_IOC_DMXSECBUF_BUF_READ_SET:
        err= MDrv_TSP_IO_Dmx_SecBuf_ReadSet(arg);
        break;
    case TSP_IOC_DMXSECBUF_BUF_WRITE_GET:
        err= MDrv_TSP_IO_Dmx_SecBuf_WriteGet(arg);
        break;
#endif

    //------------------------------------------------------------------------------
    // Misc
    //------------------------------------------------------------------------------
    case TSP_IOC_GET_STC:
        err= MDrv_TSP_IO_GetSTC(arg);
        break;
    case TSP_IOC_SET_MODE:
        err= MDrv_TSP_IO_SetMode(arg);
        break;
    case TSP_IOC_SEL_PAD:
        err= MDrv_TSP_IO_SelPad(arg);
        break;
    case TSP_IOC_POWER:
        err= MDrv_TSP_IO_Power(arg);
        //err=MDrv_TSP_Close();
        break;
    case TSP_IOC_GET_LAST_ERR:
        err= MDrv_TSP_IO_LastErr(arg);
        break;
    case TSP_IOC_INIT:
        err= MDrv_TSP_IO_Init(arg);
        break;
    case TSP_IOC_SCMB_STATUS:
        err= MDrv_TSP_IO_Scmb_Status(arg);
        break;
    case TSP_IOC_SET_TSIF_TYPE:
        err= MDrv_TSP_IO_Set_TSIF_Type(arg);
        break;
    case TSP_IOC_SET_PACKET_MODE:
        err = MDrv_TSP_IO_SetPacketMode(arg);
        break;
    case TSP_IOC_SET_SCRM_PATH:
        err= MDrv_TSP_IO_Set_ScmbPath(arg);
        break;
    case TSP_IOC_SET_ESA_MODE:
        err= MDrv_TSP_IO_Set_EsaMode(arg);
        break;
    case TSP_IOC_SET_CIPHERKEY:
        err= MDrv_TSP_IO_Set_CipherKey(arg);
        break;
    case TSP_IOC_FIFO_FLUSH:
        err= MDrv_TSP_IO_Fifo_Flush(arg);
        break;
    case TSP_IOC_FIFO_FETCH:
        err= MDrv_TSP_IO_Fifo_Fetch(arg);
        break;
    case TSP_IOC_GET_PCR:
        err= MDrv_TSP_IO_GetPcr(arg);
        break;
    default:
        TSP_WARNING("Unknown ioctl command %d\n", cmd);
        PROBE_IO_EXIT(MDRV_MAJOR_TSP, _IOC_NR(cmd));
        return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_TSP, _IOC_NR(cmd));
    return err;
}

static int __init mod_tsp_init(void)
{
    int         s32Ret;
    dev_t       dev;

    TSP_PRINT("%s is invoked\n", __FUNCTION__);

    if (TspDev.s32TspMajor)
    {
        dev=                    MKDEV(TspDev.s32TspMajor, TspDev.s32TspMinor);
        s32Ret=                 register_chrdev_region(dev, MOD_TSP_DEVICE_COUNT, MDRV_NAME_TSP);
    }
    else
    {
        s32Ret=                 alloc_chrdev_region(&dev, TspDev.s32TspMinor, MOD_TSP_DEVICE_COUNT, MDRV_NAME_TSP);
        TspDev.s32TspMajor=     MAJOR(dev);
    }
    if (0> s32Ret)
    {
        TSP_WARNING("Unable to get major %d\n", TspDev.s32TspMajor);
        return s32Ret;
    }

    cdev_init(&TspDev.cDevice, &TspDev.TspFop);
    if (0!= (s32Ret= cdev_add(&TspDev.cDevice, dev, MOD_TSP_DEVICE_COUNT)))
    {
        TSP_WARNING("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_TSP_DEVICE_COUNT);

        return s32Ret;
    }
#if 0 //2009.11.10
    spin_lock_init(&_spinlock);
    init_waitqueue_head(&_wait_queue);
#endif
    return 0;
}

static void __exit mod_tsp_exit(void)
{
    TSP_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&TspDev.cDevice);
    unregister_chrdev_region(MKDEV(TspDev.s32TspMajor, TspDev.s32TspMinor), MOD_TSP_DEVICE_COUNT);
}

void tsp_isr_task(unsigned long param)
{
    _SetEvent(TSP_TASK_EVENT_SECTION);
}

DECLARE_TASKLET(tsp_isr_tasklet,     tsp_isr_task, 0);

//--------------------------------------------------------------------------------------------------
// Interrupt service routine of TSP
// Arguments:   None
// Return:      None
//--------------------------------------------------------------------------------------------------
irqreturn_t MDrv_TSP_Isr(int irq, void *dev_id)
{
#if 1 //TO DO
    U16                 u16HwInt;
    //U32                 u32SwInt;

    TSP_ISR_SAVE_ALL;

    u16HwInt = REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1);

    if (HAS_FLAG(u16HwInt, TSP_HWINT_TSP_PVR_TAIL0_STATUS))
    {
        _SetEvent(TSP_TASK_EVENT_PVR0_RDY);
    }
    if (HAS_FLAG(u16HwInt, TSP_HWINT_TSP_PVR_MID0_STATUS))
    {
        _SetEvent(TSP_TASK_EVENT_PVR0_MID);
    }
    if (HAS_FLAG(u16HwInt, TSP_HWINT_TSP_SW_INT_STATUS))
    {
        _SetEvent(TSP_TASK_EVENT_SECTION);
    }
#else
    TSP_ISR_SAVE_ALL;

    tasklet_schedule(&tsp_isr_tasklet);
#endif
    MHal_TSP_Int_ClearSw();
    MHal_TSP_Int_ClearHw(TSP_HWINT_ALL);

    TSP_ISR_RESTORE_ALL;

    // Linux seems not to need to enable int in ISR.
    return IRQ_HANDLED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver functions
///////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------
// Macro function to disable PID filter, update driver status, check power control
// Arguments:   u32EngId - index of TSP engine
//              u32PidFltId - index of PID filter
// Return:      None
//--------------------------------------------------------------------------------------------------
static void _MDrv_TSP_PidFlt_Disable(U32 u32EngId, U32 u32PidFltId)
{
    REG_PidFlt *pPidFilter = &(_TspPid[u32EngId].Flt[u32PidFltId]);

    MHal_TSP_PidFlt_SelFltOutput(pPidFilter, TSP_PIDFLT_OUT_NONE);
    MHal_TSP_PidFlt_SelFltSource(pPidFilter, TSP_PIDFLT_IN_LIVE);
    _TspState[u32EngId].FltState[u32PidFltId] &= ~E_DRVTSP_FLT_STATE_ENABLE;
    _TspEnable[u32EngId] &= ~(0x1 << u32PidFltId);
}

//--------------------------------------------------------------------------------------------------
// Macro function to initilize a TSP section filter
// Arguments:   u32EngId - index of TSP engine
//              u32SecltId - index of section filter
// Return:      None
//--------------------------------------------------------------------------------------------------
static void _MDrv_TSP_SecFlt_Init(U32 u32EngId, U32 u32SecFltId)
{
    static U8    u8MatchMask[TSP_FILTER_DEPTH] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    REG_SecFlt      *pSecFilter = &(_TspSec[u32EngId].Flt[u32SecFltId]);
    // TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] SecFltId= %d\n", __LINE__, u32SecFltId));

    MHal_TSP_SecFlt_ClrCtrl(pSecFilter);
    MHal_TSP_SecFlt_SetMatchMask(pSecFilter, u8MatchMask, u8MatchMask);
    MHal_TSP_SecFlt_SetReqCount(pSecFilter, 0);
    MHal_TSP_SecFlt_SetBuffer(pSecFilter, 0, 0);
    MHal_TSP_SecFlt_ResetBuffer(pSecFilter);
    MHal_TSP_SecFlt_ResetState(pSecFilter);
#ifndef MSOS_TYPE_LINUX
    _TspState[0].SecFltEvtNotify[u32SecFltId]= 0;
    _TspState[0].SecFltCallback[u32SecFltId]= NULL;
#endif // #ifndef MSOS_TYPE_LINUX
}

static B16 _MDrv_TSP_SecFlt_Alloc(U32 u32EngId, DrvTSP_FltType eFilterType, U32 *pu32SecFltId)
{
    int     i;

    *pu32SecFltId = 0xFFFFFFFF;
    if (E_DRVTSP_FLT_TYPE_VIDEO== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        i=                          _u8IdxStream[u32EngId][0];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].SecFltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
    }
    else if (E_DRVTSP_FLT_TYPE_AUDIO== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        i=                          _u8IdxStream[u32EngId][1];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].SecFltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
    }
    else if (E_DRVTSP_FLT_TYPE_AUDIO2== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        i=                          _u8IdxStream[u32EngId][2];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].SecFltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
    }
    else if (E_DRVTSP_FLT_TYPE_PCR== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        i=                          _u8IdxStream[u32EngId][3];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].SecFltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
    }
    else
    {
        for (i = 0; i < TSP_SECFLT_NUM; i++)
        {
            if (_TspState[u32EngId].SecFltState[i] == E_DRVTSP_FLT_STATE_FREE)
            {
                break;
            }
        }
    }
    if (i == TSP_SECFLT_NUM)
    {
        return FALSE;
    }

    _TspState[u32EngId].SecFltState[i] = E_DRVTSP_FLT_STATE_ALLOC;
    _MDrv_TSP_SecFlt_Init(u32EngId, i);
    *pu32SecFltId = i;
    return TRUE;
}

static B16 _MDrv_TSP_SecFlt_Free(U32 u32EngId, U32 u32SecFltId)
{
    _TspState[u32EngId].SecFltState[u32SecFltId] = E_DRVTSP_FLT_STATE_FREE;
    return TRUE;
}

static B16 _MDrv_TSP_SecFlt_StateGet(U32 u32EngId, U32 u32SecFltId, DrvTSP_FltState *peState)
{
    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(peState, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));

    *peState =  _TspState[u32EngId].SecFltState[u32SecFltId];
    if (MHal_TSP_SecFlt_GetState(&(_TspSec[0].Flt[u32SecFltId])) & TSP_SECFLT_STATE_OVERFLOW)
    {
        *peState |= E_DRVTSP_FLT_STATE_OVERFLOW;
    }
    return TRUE;
}

//--------------------------------------------------------------------------------------------------
// Macro function to initilize a TSP PID filter
// Arguments:   u32EngId - index of TSP engine
//              u32PidFltId - index of PID filter
//              eFilterType - type of filter
// Return:      None
//--------------------------------------------------------------------------------------------------
static void _MDrv_TSP_PidFlt_Init(U32 u32EngId, U32 u32PidFltId)
{
    REG_PidFlt *pPidFilter = &(_TspPid[u32EngId].Flt[u32PidFltId]);

    MHal_TSP_PidFlt_SelFltOutput(pPidFilter, TSP_PIDFLT_OUT_NONE);
    MHal_TSP_PidFlt_SetPid(pPidFilter, DRVTSP_PID_NULL);
    MHal_TSP_PidFlt_SelSecFlt(pPidFilter, TSP_PIDFLT_SECFLT_NULL);
}

static B16 _MDrv_TSP_PidFlt_Alloc(U32 u32EngId, DrvTSP_FltType eFilterType, U32 *pu32PidFltId)
{
    int     i;
    int     pid_idx;

    *pu32PidFltId = 0xFFFFFFFF;

    // input parameter validation
    if ( (u32EngId >= TSP_ENGINE_NUM) ||
         ((eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK) >= E_DRVTSP_FLT_TYPE_LAST_ENUM) )
    {
        TSP_ASSERT(0, printf("[TSP_ERROR][%06d] (EngId, FilterType)= (%d %d)\n", __LINE__, u32EngId, eFilterType));
        return FALSE;
    }

    if (E_DRVTSP_FLT_TYPE_VIDEO== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        i=                          _u8IdxStream[u32EngId][0];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].FltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
    }
    else if (E_DRVTSP_FLT_TYPE_AUDIO== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        i=                          _u8IdxStream[u32EngId][1];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].FltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
    }
    else if (E_DRVTSP_FLT_TYPE_AUDIO2== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        i=                          _u8IdxStream[u32EngId][2];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].FltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
    }
    else if (E_DRVTSP_FLT_TYPE_PCR== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        i=                          _u8IdxStream[u32EngId][3];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].FltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
    }
    //2009.09.21

    //cip - 2009.09.23
    else if (E_DRVTSP_FLT_TYPE_PES== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
#if 1
	 /* TSP_PIDFLT_NUM -1,-2,-4 : reverved for video, audio, pcr */
        for (i = (TSP_PIDFLT_NUM - 5); i > (TSP_PIDFLT_NUM - 1 - RESERVED_PIDFLT_NUM); i--)
        {
            if (_TspState[u32EngId].FltState[i] == E_DRVTSP_FLT_STATE_FREE)
            {
                break;
            }
            else if (i == 0)
            {
                TSP_ASSERT(0, printf("[TSP_ERROR][%06d] No free fitler found 0x%02x\n", __LINE__, i));
                return FALSE;
            }
        }
#else
        i=                          _u8IdxStream[u32EngId][4];
        TSP_ASSERT(E_DRVTSP_FLT_STATE_ALLOC!= _TspState[u32EngId].FltState[i],
                   printf("[TSP_ERROR][%06d] bad fitler state 0x%02x\n", __LINE__, i));
        if (_TspState[u32EngId].FltState[i] & E_DRVTSP_FLT_STATE_ALLOC)
        {
            return FALSE;
        }
#endif
    }
    else
    {
        if (E_DRVTSP_FLT_TYPE_PVR== (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
        {
            pid_idx=                TSP_PIDFLT_NUM;
        }
        else
        {
            pid_idx=                0;
        }

        for (i = pid_idx; i < TSP_PIDFLT_NUM_ALL; i++)
        {
            if (_TspState[u32EngId].FltState[i] == E_DRVTSP_FLT_STATE_FREE)
            {
                break;
            }
        }
        if (i == TSP_PIDFLT_NUM_ALL)
        {
            TSP_ASSERT(0, printf("[TSP_ERROR][%06d] No free fitler found 0x%02x\n", __LINE__, i));
            return FALSE;
        }
    }

    _TspState[u32EngId].FltState[i] = E_DRVTSP_FLT_STATE_ALLOC;
    _TspState[u32EngId].FltType[i] = (eFilterType&(DrvTSP_FltType)(~E_DRVTSP_FLT_SOURCE_TYPE_MASK));

    if ((eFilterType&E_DRVTSP_FLT_SOURCE_TYPE_MASK) == E_DRVTSP_FLT_SOURCE_TYPE_LIVE)
    {
        _TspState[u32EngId].FltSource[i] = E_DRVTSP_FLT_SOURCE_TYPE_LIVE;
    }
    else if ((eFilterType&E_DRVTSP_FLT_SOURCE_TYPE_MASK) == E_DRVTSP_FLT_SOURCE_TYPE_FILE)
    {
        _TspState[u32EngId].FltSource[i] = E_DRVTSP_FLT_SOURCE_TYPE_FILE;
    }

    _MDrv_TSP_PidFlt_Init(u32EngId, i);
    *pu32PidFltId = i;

    return TRUE;
}

DRVTSP_RESULT _MDrv_TSP_PidFlt_Free(U32 u32EngId, U32 u32PidFltId)
{
    if ( (u32EngId    >= TSP_ENGINE_NUM) ||
         (u32PidFltId >= TSP_PIDFLT_NUM_ALL) ||
         (_TspState[u32EngId].FltState[u32PidFltId] == E_DRVTSP_FLT_STATE_FREE) )
    {
        TSP_ASSERT(0, printf("[TSP_ERROR][%06d] (EngId, PidFltId, FilterState)= (%d %d %d)\n", __LINE__, u32EngId, u32PidFltId, _TspState[u32EngId].FltState[u32PidFltId]));
        return FALSE;
    }

    _MDrv_TSP_PidFlt_Disable(u32EngId, u32PidFltId);

    // Release Filter
    _MDrv_TSP_PidFlt_Init(u32EngId, u32PidFltId);
    MHal_TSP_PidFlt_SelSecFlt(&(_TspPid[u32EngId].Flt[u32PidFltId]), TSP_PIDFLT_SECFLT_NULL);
    _TspState[u32EngId].FltState[u32PidFltId] = E_DRVTSP_FLT_STATE_FREE;
    return TRUE;
}

static B16 _MDrv_TSP_PidFlt_StateGet(U32 u32EngId, U32 u32PidFltId, DrvTSP_FltState *peState)
{
    // @FIXME: Richard: Don't plan to have section filter in PVR pid filter at this stage.
    //              Therefore, don't need to check PVR pid filter
    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFlt Id %d\n", __LINE__, u32PidFltId));
    TSP_ASSERT(peState, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));

    *peState=   _TspState[u32EngId].FltState[u32PidFltId];

    if (MHal_TSP_Scmb_Status(TRUE, u32PidFltId) || MHal_TSP_Scmb_Status(FALSE, u32PidFltId))
    {
        *peState |= E_DRVTSP_FLT_STATE_SCRAMBLED;
    }
    return TRUE;
}

//--------------------------------------------------------------------------------------------------
// Macro function to flush command queue
// Arguments:   None
// Return:      None
//--------------------------------------------------------------------------------------------------
static void _MDrv_TSP_CmdQ_Init(void)
{
    while (MHal_TSP_CmdQ_CmdCount()); // wait command finish
}

static void
_MDrv_TSP_SelPad(U32 u32EngId, DrvTSP_PadIn ePad)
{
#if defined(CONFIG_Titania3)
    switch(ePad)
    {
    case E_DRVTSP_PAD_EXT_INPUT0: //T3 : External Demod + CI. PAD_TS1 as output is DISABLED.
        MHal_TSP_SelPad(u32EngId, 1);
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)&~(CLKGEN0_CKG_TS0_SRC_MASK|CLKGEN0_CKG_TS0_MASK))|CLKGEN0_CKG_TS0_SRC_TS0;
        TOP_REG(REG_TOP_TS0_TS1_MUX) =
            (TOP_REG(REG_TOP_TS0_TS1_MUX)&~TOP_TS0MUX_MASK)|TOP_TS0MUX_PAD0;
        MHal_TSP_MUX1_SelPad(u32EngId, 1);
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)&~(CLKGEN0_CKG_TS1_SRC_MASK|CLKGEN0_CKG_TS1_MASK))|CLKGEN0_CKG_TS1_SRC_TS0;
        TOP_REG(REG_TOP_TS0_TS1_MUX) =
            (TOP_REG(REG_TOP_TS0_TS1_MUX)&~TOP_TS1MUX_MASK)|TOP_TS1MUX_PAD0;
        TOP_REG(REG_TOP_TCON_TS1) =
            TOP_REG(REG_TOP_TCON_TS1) & ~TOP_TS1_OUT;
        break;

    case E_DRVTSP_PAD_EXT_INPUT1: //T3 : External Demod. PAD_TS1 as output is DISABLED.
        MHal_TSP_SelPad(u32EngId, 1);
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)&~(CLKGEN0_CKG_TS0_SRC_MASK|CLKGEN0_CKG_TS0_MASK))|CLKGEN0_CKG_TS0_SRC_TS1;
        TOP_REG(REG_TOP_TS0_TS1_MUX) =
            (TOP_REG(REG_TOP_TS0_TS1_MUX)&~TOP_TS0MUX_MASK)|TOP_TS0MUX_PAD1;
        MHal_TSP_MUX1_SelPad(u32EngId, 1);
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)&~(CLKGEN0_CKG_TS1_SRC_MASK|CLKGEN0_CKG_TS1_MASK))|CLKGEN0_CKG_TS1_SRC_TS1;
        TOP_REG(REG_TOP_TS0_TS1_MUX) =
            (TOP_REG(REG_TOP_TS0_TS1_MUX)&~TOP_TS1MUX_MASK)|TOP_TS1MUX_PAD1;
        TOP_REG(REG_TOP_TCON_TS1) =
            TOP_REG(REG_TOP_TCON_TS1) & ~TOP_TS1_OUT;
        break;

    case E_DRVTSP_PAD_CI: //T3 : Internal Demod + CI. PAD_TS1 as output is ENABLED.
        MHal_TSP_SelPad(u32EngId, 1);
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)&~(CLKGEN0_CKG_TS0_SRC_MASK|CLKGEN0_CKG_TS0_MASK))|CLKGEN0_CKG_TS0_SRC_TS0;
        TOP_REG(REG_TOP_TS0_TS1_MUX) =
            (TOP_REG(REG_TOP_TS0_TS1_MUX)&~TOP_TS0MUX_MASK)|TOP_TS0MUX_PAD0;
        MHal_TSP_MUX1_SelPad(u32EngId, 1);
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)&~(CLKGEN0_CKG_TS1_SRC_MASK|CLKGEN0_CKG_TS1_MASK))|CLKGEN0_CKG_TS1_SRC_TS0;
        TOP_REG(REG_TOP_TS0_TS1_MUX) =
            (TOP_REG(REG_TOP_TS0_TS1_MUX)&~TOP_TS1MUX_MASK)|TOP_TS1MUX_PAD0;
        TOP_REG(REG_TOP_TCON_TS1) =
            TOP_REG(REG_TOP_TCON_TS1) | TOP_TS1_OUT;
        break;

    case E_DRVTSP_PAD_DEMOD_INV:
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)|CLKGEN0_CKG_TS0_INV);
    case E_DRVTSP_PAD_DEMOD: //T3 : Internal Demod. PAD_TS1 as output is DISABLED.
    default:
        MHal_TSP_SelPad(u32EngId, 0);
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)&~(CLKGEN0_CKG_TS0_SRC_MASK|CLKGEN0_CKG_TS0_MASK))|CLKGEN0_CKG_TS0_SRC_DMD;
        TOP_REG(REG_TOP_TS0_TS1_MUX) =
            (TOP_REG(REG_TOP_TS0_TS1_MUX)&~TOP_TS0MUX_MASK)|TOP_TS0MUX_DVBC;
        MHal_TSP_MUX1_SelPad(u32EngId, 0);
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            (CKLGEN0_REG(REG_CLKGEN0_TS)&~(CLKGEN0_CKG_TS1_SRC_MASK|CLKGEN0_CKG_TS1_MASK))|CLKGEN0_CKG_TS1_SRC_DMD;
        TOP_REG(REG_TOP_TS0_TS1_MUX) =
            (TOP_REG(REG_TOP_TS0_TS1_MUX)&~TOP_TS1MUX_MASK)|TOP_TS1MUX_DVBC;
        TOP_REG(REG_TOP_TCON_TS1) =
            TOP_REG(REG_TOP_TCON_TS1) & ~TOP_TS1_OUT;
        break;

    }
#else //T2
    switch(ePad)
    {
    case E_DRVTSP_PAD_DEMOD_INV:
        TOP_REG(REG_TOP_TS0_TSP_STC0) =
            (TOP_REG(REG_TOP_TS0_TSP_STC0)|TOP_CKG_TS0_INV);
    case E_DRVTSP_PAD_DEMOD:
        MHal_TSP_SelPad(u32EngId, 1);
        TOP_REG(REG_TOP_TS0_TSP_STC0) =
            ((TOP_REG(REG_TOP_TS0_TSP_STC0)&~TOP_CKG_TS0_SRC_MASK)|TOP_CKG_TS0_SRC_TS1);
        break;

    case E_DRVTSP_PAD_EXT_INPUT0:
    case E_DRVTSP_PAD_CI:
    default:
        MHal_TSP_SelPad(u32EngId, 0);
        TOP_REG(REG_TOP_TS0_TSP_STC0) =
            ((TOP_REG(REG_TOP_TS0_TSP_STC0)&~TOP_CKG_TS0_SRC_MASK)|TOP_CKG_TS0_SRC_TS0);
        break;
    }
#endif //#if defined(CONFIG_Titania3)
}

static void _MDrv_TSP_Close(void)
{
    MHal_TSP_Int_Disable(TSP_HWINT_ALL);
    IntDisable();
    IntDetach();
    MHal_TSP_SetCtrlMode(0, 0, 2);
    MHal_TSP_WbDmaEnable(FALSE);
    u32TSPIntRegister = 0; //LGE
}

static  U8 _MDrv_TSP_SecFlt_Type_ID(U32 u32DrvType)
{
    switch(u32DrvType)
    {
        case E_DRVTSP_FLT_TYPE_SECTION:
            return TSP_SECFLT_TYPE_SEC;

        case E_DRVTSP_FLT_TYPE_PES:
            return TSP_SECFLT_TYPE_PES;

        case E_DRVTSP_FLT_TYPE_PACKET:
            return TSP_SECFLT_TYPE_PKT;

        case E_DRVTSP_FLT_TYPE_TELETEXT:
            return TSP_SECFLT_TYPE_TTX;

        case E_DRVTSP_FLT_TYPE_PCR:
            return TSP_SECFLT_TYPE_PCR;

        default:
            return TSP_SECFLT_TYPE_UNK;
    }
}

//--------------------------------------------------------------------------------------------------
/// Allocate a PID filter of a TSP unit
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  eFilterType             \b IN: type of PID filter to be allocated
/// @param  pu32PidFltId            \b OUT: pointer of PID filter id return
/// @return DRVTSP_RESULT
/// @note
/// These filter types have to select a section filter\n
/// @ref E_DRVTSP_FLT_TYPE_SECTION\n
/// @ref E_DRVTSP_FLT_TYPE_PCR\n
/// @ref E_DRVTSP_FLT_TYPE_PES\n
/// @note
/// These filter types also have to setup section buffer for data output\n
/// @ref E_DRVTSP_FLT_TYPE_SECTION\n
/// @ref E_DRVTSP_FLT_TYPE_PES\n
/// @sa MDrv_TSP_PidFlt_SelSecFlt, MDrv_TSP_SecFlt_SetBuffer
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_Alloc(U32 u32EngId, DrvTSP_FltType eFilterType, U32 *pu32PidFltId)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));

    *pu32PidFltId=      0xFFFFFFFF;
    if (!_MDrv_TSP_PidFlt_Alloc(u32EngId, eFilterType, pu32PidFltId))
    {
        TSP_ASSERT(0, printf("[TSP_ERROR][%06d] Allocate Pid Filter fail\n", __LINE__));
        _TSP_RETURN(DRVTSP_FAIL);
    }
    _MDrv_TSP_PidFlt_Init(u32EngId, *pu32PidFltId);

///////////////////////////////////////////////////////////
/*
    if (E_DRVTSP_FLT_TYPE_PCR == (eFilterType&~E_DRVTSP_FLT_SOURCE_TYPE_MASK))
    {
        if (!_TSP_SecFlt_Alloc(u32EngId, eFilterType, &u32SecFltId))
        {
            TSP_ASSERT(0, printf("[TSP_ERROR][%06d] Allocate Section Filter fail\n", __LINE__));
        }
        MHal_TSP_PidFlt_SelSecFlt(&(_TspPid[u32EngId].Flt[*pu32PidFltId]), u32SecFltId);
        MHal_TSP_SecFlt_SetType(&(_TspSec[u32EngId].Flt[u32SecFltId]), _TspState[u32EngId].FltType[*pu32PidFltId]);
    }
*/
///////////////////////////////////////////////////////////
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Free a PID filter of a TSP unit
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32PidFltId             \b IN: index of PID filter to be free
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_Free(U32 u32EngId, U32 u32PidFltId)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM_ALL> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    _MDrv_TSP_PidFlt_Free(u32EngId, u32PidFltId);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set PID to a PID filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32PidFltId             \b IN: index of PID filter to be set
/// @param  u32PID                  \b IN: PID value
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_SetPid(U32 u32EngId, U32 u32PidFltId, U32 u32PID)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM_ALL> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    TSP_ASSERT(E_DRVTSP_FLT_STATE_FREE!= _TspState[u32EngId].FltState[u32PidFltId], printf("[TSP_ERROR][%06d] Bad Flt state\n", __LINE__));
    MHal_TSP_PidFlt_SetPid(&(_TspPid[u32EngId].Flt[u32PidFltId]), u32PID);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get PID from a PID filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32PidFltId             \b IN: index of PID filter to be set
/// @param  pu32PID                 \b OUT: point to PID value
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_GetPid(U32 u32EngId, U32 u32PidFltId, U32 *pu32PID)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM_ALL> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    TSP_ASSERT(E_DRVTSP_FLT_STATE_FREE!= _TspState[u32EngId].FltState[u32PidFltId], printf("[TSP_ERROR][%06d] Bad Flt state\n", __LINE__));
    *pu32PID = MHal_TSP_PidFlt_GetPid(&(_TspPid[u32EngId].Flt[u32PidFltId]));
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Select section filter of PID filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32PidFltId             \b IN: index of PID filter\n
/// @param  u32SecFltId             \b IN: index of section filter
/// @return DRVTSP_RESULT
/// @note
/// The PID filter and section filter pair is one-to-one mapping. User has to
/// allocate other PID filters if user have more than one section filter for same
/// PID packet.\n
/// @sa MDrv_TSP_PidFlt_Alloc
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_SelSecFlt(U32 u32EngId, U32 u32PidFltId, U32 u32SecFltId)
{

    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    TSP_ASSERT(E_DRVTSP_FLT_STATE_FREE!= _TspState[u32EngId].FltState[u32PidFltId], printf("[TSP_ERROR][%06d] Bad Flt state\n", __LINE__));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] SecFltId= %d\n", __LINE__, u32SecFltId));

    MHal_TSP_PidFlt_SelSecFlt(&(_TspPid[u32EngId].Flt[u32PidFltId]), u32SecFltId);
    MHal_TSP_SecFlt_SetType(&(_TspSec[u32EngId].Flt[u32SecFltId]), _MDrv_TSP_SecFlt_Type_ID(_TspState[u32EngId].FltType[u32PidFltId]));
    MHal_TSP_SendFWCmd(TSP_CMD_MAP_PIDFLT, u32EngId, u32SecFltId, u32PidFltId);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set Source of PID filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32PidFltId             \b IN: index of PID filter to be set
/// @param  u32Src                  \b IN: 0 = TS input, 1 = File input
/// @return DRVTSP_RESULT
//  @note
//      If one PID filter selects FILE input, all other PID filters, if enabled, becomes FILE input. It's HW behavior.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_SetSource(U32 u32EngId, U32 u32PidFltId, U32 u32Src)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM_ALL> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    switch (u32Src)
    {
        case E_DRVTSP_SRC_TS:
            MHal_TSP_PidFlt_SelFltSource(&(_TspPid[u32EngId].Flt[u32PidFltId]), TSP_PIDFLT_IN_LIVE);
            break;
        case E_DRVTSP_SRC_FILE:
            MHal_TSP_PidFlt_SelFltSource(&(_TspPid[u32EngId].Flt[u32PidFltId]), TSP_PIDFLT_IN_FILE);
            break;
        default:
            _TSP_RETURN(DRVTSP_FAIL);
    }
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get Source of PID filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32PidFltId             \b IN: index of PID filter to be set
/// @param  u32Src                  \b OUT: 0 = TS input, 1 = File input
/// @return DRVTSP_RESULT
//  @note
//      If one PID filter selects FILE input, all other PID filters, if enabled, becomes FILE input. It's HW behavior.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_GetSource(U32 u32EngId, U32 u32PidFltId, U32* u32Src)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM_ALL> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));

    *u32Src = MHal_TSP_PidFlt_GetFltSource(&(_TspPid[u32EngId].Flt[u32PidFltId]));
    switch (*u32Src)
    {
        case TSP_PIDFLT_IN_LIVE:
            *u32Src = E_DRVTSP_SRC_TS;
            break;
        case TSP_PIDFLT_IN_FILE:
            *u32Src = E_DRVTSP_SRC_FILE;
            break;
        default:
            _TSP_RETURN(DRVTSP_FAIL);
    }
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Enable PID filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32PidFltId             \b IN: index of filter to be enable
/// @param  bEnable                 \b IN: TRUE(enable), FALSE(disable)
/// @return DRVTSP_RESULT
/// @note
/// When PID filter enable, the section buffer pointer will be reset to buffer start address,
/// overflow condition will be resolved if exist.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_Enable(U32 u32EngId, U32 u32PidFltId, B16 bEnable)
{
    REG_PidFlt*         pPidFilter= NULL;

    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM_ALL> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    TSP_ASSERT(E_DRVTSP_FLT_STATE_FREE!= _TspState[u32EngId].FltState[u32PidFltId], printf("[TSP_ERROR][%06d] Bad Flt state\n", __LINE__));

    pPidFilter=         &(_TspPid[u32EngId].Flt[u32PidFltId]);
    if (!bEnable){
        // To disable pid filter
        if (!HAS_FLAG(_TspEnable[u32EngId], (0x1 << u32PidFltId)))
        {
            // printf("[TSP_WARNNING][%06d] disable an inactive PidFlt %d\n", __LINE__, u32PidFltId);
            _TSP_RETURN(DRVTSP_OK);
        }

        _MDrv_TSP_PidFlt_Disable(u32EngId, u32PidFltId);
        _TSP_RETURN(DRVTSP_OK);
    }

    if (HAS_FLAG(_TspEnable[u32EngId], (0x1 << u32PidFltId)))
    {
        // printf("[TSP_WARNNING][%06d] Enable an active PidFlt %d\n", __LINE__, u32PidFltId);
        _TSP_RETURN(DRVTSP_OK);
    }
    if (E_DRVTSP_FLT_TYPE_PVR== _TspState[u32EngId].FltType[u32PidFltId])
    {
        TSP_ASSERT(TSP_PIDFLT_NUM<= u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    }
    else if (E_DRVTSP_FLT_TYPE_VIDEO== _TspState[u32EngId].FltType[u32PidFltId])
    {
        MHal_TSP_PidFlt_SelFltOutput(pPidFilter, TSP_PIDFLT_OUT_VFIFO);
    }
    else if (E_DRVTSP_FLT_TYPE_AUDIO== _TspState[u32EngId].FltType[u32PidFltId])
    {
        MHal_TSP_PidFlt_SelFltOutput(pPidFilter, TSP_PIDFLT_OUT_AFIFO);
    }
    else if (E_DRVTSP_FLT_TYPE_AUDIO2== _TspState[u32EngId].FltType[u32PidFltId])
    {
        MHal_TSP_PidFlt_SelFltOutput(pPidFilter, TSP_PIDFLT_OUT_AFIFO2);
    }
    else
    {
        REG_SecFlt*     pSecFilter= NULL;
        U32          u32SecFltId;

        TSP_ASSERT(TSP_PIDFLT_NUM> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));

        u32SecFltId=    MHal_TSP_PidFlt_GetSecFlt(pPidFilter);
        TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] SecFltId= %d\n", __LINE__, u32SecFltId));
        pSecFilter=     &(_TspSec[u32EngId].Flt[u32SecFltId]);

        switch (_TspState[u32EngId].FltType[u32PidFltId])
        {
        case E_DRVTSP_FLT_TYPE_SECTION:
        case E_DRVTSP_FLT_TYPE_PES:
        case E_DRVTSP_FLT_TYPE_PACKET:
        case E_DRVTSP_FLT_TYPE_TELETEXT:
            MHal_TSP_SecFlt_SetRmnCount(pSecFilter, 0);
            MHal_TSP_SecFlt_ResetState(pSecFilter);
            MHal_TSP_SendFWCmd(TSP_CMD_RESET_FLT, u32EngId, u32SecFltId,
            _MDrv_TSP_SecFlt_Type_ID(_TspState[u32EngId].FltType[u32PidFltId]));
            MHal_TSP_PidFlt_SelFltOutput(pPidFilter, TSP_PIDFLT_OUT_SECFLT);
            break;

        case E_DRVTSP_FLT_TYPE_PCR:
            MHal_TSP_SecFlt_SetParam(pSecFilter, TSP_SECFLT_PARAM_PCRRST);
            MHal_TSP_SendFWCmd(TSP_CMD_RESET_FLT, u32EngId, u32SecFltId, TSP_SECFLT_TYPE_PCR);
            //[HWBUG]
            #if 0 // Wait ECO
            MHal_TSP_PidFlt_SelFltOutput(pPidFilter, TSP_PIDFLT_OUT_SECFLT_AF);
            #else
            MHal_TSP_PidFlt_SelFltOutput(pPidFilter, TSP_PIDFLT_OUT_SECFLT | TSP_PIDFLT_OUT_SECFLT_AF);
            #endif
            break;

        default:
            break;
        }
    }

    if (E_DRVTSP_FLT_SOURCE_TYPE_LIVE == _TspState[u32EngId].FltSource[u32PidFltId])
    {
        MHal_TSP_PidFlt_SelFltSource(pPidFilter, TSP_PIDFLT_IN_LIVE);
    }
    else if (E_DRVTSP_FLT_SOURCE_TYPE_FILE == _TspState[u32EngId].FltSource[u32PidFltId])
    {
        MHal_TSP_PidFlt_SelFltSource(pPidFilter, TSP_PIDFLT_IN_FILE);
    }

    _TspState[u32EngId].FltState[u32PidFltId]|= E_DRVTSP_FLT_STATE_ENABLE;
    _TspEnable[u32EngId]|= (0x1 << u32PidFltId);

#if defined(TSP_DUMP)
    if (32> u32PidFltId)
        _TDump_Flt(0, 0, 12);
#endif
    _TSP_RETURN(DRVTSP_OK);
}


//--------------------------------------------------------------------------------------------------
/// Get current PID filter status
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32PidFltId             \b IN: index of PID filter
/// @param  peState                 \b OUT: current ORed state flag of PID filter\n
///                                         E_DRVTSP_FLT_STATE_ALLOC\n
///                                         E_DRVTSP_FLT_STATE_ENABLE\n
///                                         E_DRVTSP_FLT_STATE_SCRAMBLED ([VENUS]: TS level scramble status)
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PidFlt_GetState(U32 u32EngId, U32 u32PidFltId, DrvTSP_FltState *peState)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();
    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM_ALL> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    if (_MDrv_TSP_PidFlt_StateGet(u32EngId, u32PidFltId, peState))
    {
        _TSP_RETURN(DRVTSP_OK);
    }
    else
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }
}

DRVTSP_RESULT MDrv_TSP_PidFlt_ScmbStatus(U32 u32EngId, U32 u32PidFltId, DrvTSP_Scmb_Level* pScmbLevel)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_PIDFLT_NUM_ALL> u32PidFltId, printf("[TSP_ERROR][%06d] Bad PidFltId %d\n", __LINE__, u32PidFltId));
    TSP_ASSERT(E_DRVTSP_FLT_STATE_FREE!= _TspState[u32EngId].FltState[u32PidFltId], printf("[TSP_ERROR][%06d] Bad Flt state\n", __LINE__));
    TSP_ASSERT(pScmbLevel, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));

    *pScmbLevel=        E_DRVTSP_SCMB_NONE;
    if (MHal_TSP_Scmb_Status(TRUE, u32PidFltId) || MHal_TSP_Scmb_Status(FALSE, u32PidFltId))
        *pScmbLevel=    E_DRVTSP_SCMB_TS;
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Allocate a section filter of a PID filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  pu32SecFltId            \b OUT: pointer of section filter id return
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_Alloc(U32 u32EngId, U32 *pu32SecFltId)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(pu32SecFltId, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));
    if (FALSE== _MDrv_TSP_SecFlt_Alloc(u32EngId, E_DRVTSP_FLT_TYPE_SECTION, pu32SecFltId))
    {
        TSP_ASSERT(0, printf("[TSP_ERROR][%06d] Allocate no section filter\n", __LINE__));
        _TSP_RETURN(DRVTSP_FAIL);
    }
    TSP_ASSERT(TSP_SECFLT_NUM> *pu32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, *pu32SecFltId));
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Free a section filter of a PID filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: section filter of TSP to be free
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_Free(U32 u32EngId, U32 u32SecFltId)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(E_DRVTSP_FLT_STATE_FREE!= _TspState[u32EngId].SecFltState[u32SecFltId],
               printf("[TSP_ERROR][%06d] Bad SecFlt state\n", __LINE__));
    _MDrv_TSP_SecFlt_Free(u32EngId, u32SecFltId);
    _TSP_RETURN(DRVTSP_OK);
}


//--------------------------------------------------------------------------------------------------
//[Reserved]
// Set section filtering mode
// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
// @param  u32SecFltId             \b IN: section filter of TSP to be free
// @param  eSecFltMode             \b IN: continue/one-shot mode of section filter
// @return DRVTSP_RESULT
// @attention
// One-shot filter has the disadvantage of interrupt lost becuase it stops filter, a timeout
// to check filter status is better for usage.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_SetMode(U32 u32EngId, U32 u32SecFltId, DrvTSP_FltMode eSecFltMode)
{
    U32         uMode = 0;

    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));

    if (eSecFltMode & E_DRVTSP_FLT_MODE_ONESHOT)
    {
        uMode |= TSP_SECFLT_MODE_ONESHOT;
    }

    if (eSecFltMode & E_DRVTSP_FLT_MODE_CRCCHK)
    {
        uMode |= TSP_SECFLT_MODE_CRCCHK;
    }

    MHal_TSP_SecFlt_SetMode(&(_TspSec[u32EngId].Flt[u32SecFltId]), uMode);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set Match/Mask filter pattern of section filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of filter to be set pattern
/// @param  pu8Match                \b IN: pointer of filter pattern (in @ref DRVTSP_FILTER_DEPTH bytes)
/// @param  pu8Mask                 \b IN: pointer of pattern bitmask (in @ref DRVTSP_FILTER_DEPTH bytes)
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_SetPattern(U32 u32EngId, U32 u32SecFltId, U8 *pu8Match, U8 *pu8Mask)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));

    MHal_TSP_SecFlt_SetMatchMask(&(_TspSec[u32EngId].Flt[u32SecFltId]), pu8Match, pu8Mask);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set buffer start address and buffer size to section buffer
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer to be set
/// @param  u32StartAddr            \b IN: start address of section buffer
/// @param  u32BufSize              \b IN: size of section buffer
/// @return DRVTSP_RESULT
/// @note
/// Buffer start address and buffer size should be 128-bit (16-byte) aligned.\n
/// @sa MDrv_TSP_PidFlt_Alloc
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_SetBuffer(U32 u32EngId, U32 u32SecFltId, U32 u32StartAddr, U32 u32BufSize)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    MHal_TSP_SecFlt_SetBuffer(&(_TspSec[u32EngId].Flt[u32SecFltId]), u32StartAddr, u32BufSize);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Reset the section buffer read/write pointer to start address and resolve overflow condition
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer to be reset
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_ResetBuffer(U32 u32EngId, U32 u32SecFltId)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();
    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));

    MHal_TSP_SecFlt_ResetBuffer(&(_TspSec[u32EngId].Flt[u32SecFltId]));
    MHal_TSP_SecFlt_ResetState(&(_TspSec[u32EngId].Flt[u32SecFltId]));

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set request data size to trigger interrupt
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer to be set
/// @param  u32ReqCount             \b IN: received data size to trigger interrupt
/// @return DRVTSP_RESULT
/// @note
/// TSP always calls section callback function when a completed section data is
/// ready at section buffer.
/// It can set a request value other than 0 and TSP will also notify user when
/// request size of data is ready at buffer. Only support @ref E_DRVTSP_FLT_TYPE_PES.
/// @sa MDrv_TSP_SecFlt_Notify
/// @attention
/// The maximum request count is 0xFFFF
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_SetReqCount(U32 u32EngId, U32 u32SecFltId, U32 u32ReqCount)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    MHal_TSP_SecFlt_SetReqCount(&(_TspSec[u32EngId].Flt[u32SecFltId]), u32ReqCount);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get buffer start address of setction buffer
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer
/// @param  pu32BufStart            \b OUT:  pointer of buffer start address return
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_GetBufStart(U32 u32EngId, U32 u32SecFltId, U32 *pu32BufStart)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(pu32BufStart, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));
    *pu32BufStart= MHal_TSP_SecFlt_GetBufStart(&(_TspSec[u32EngId].Flt[u32SecFltId]));
    _TSP_RETURN(DRVTSP_OK);
}


//--------------------------------------------------------------------------------------------------
/// Get buffer size of section buffer
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer
/// @param  pu32BufSize             \b OUT: pointer of buffer size return
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_GetBufSize(U32 u32EngId, U32 u32SecFltId, U32 *pu32BufSize)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(pu32BufSize, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));
    *pu32BufSize=  MHal_TSP_SecFlt_GetBufEnd(&(_TspSec[u32EngId].Flt[u32SecFltId])) -
                   MHal_TSP_SecFlt_GetBufStart(&(_TspSec[u32EngId].Flt[u32SecFltId]));
    _TSP_RETURN(DRVTSP_OK);
}


//--------------------------------------------------------------------------------------------------
/// Get current read address of section buffer
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer
/// @param  pu32ReadAddr            \b OUT: pointer of address return
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_GetReadAddr(U32 u32EngId, U32 u32SecFltId, U32 *pu32ReadAddr)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(pu32ReadAddr, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));
    *pu32ReadAddr= MHal_TSP_SecFlt_GetBufRead(&(_TspSec[u32EngId].Flt[u32SecFltId]));
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get current section data write address of section buffer
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer
/// @param  pu32WriteAddr           \b OUT: pointer of address return
/// @return DRVTSP_RESULT
/// @note
/// User can get current write address to know where is the end of section data
/// received in the section buffer.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_GetWriteAddr(U32 u32EngId, U32 u32SecFltId, U32 *pu32WriteAddr)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(pu32WriteAddr, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));
    *pu32WriteAddr= MHal_TSP_SecFlt_GetBufWrite(&(_TspSec[u32EngId].Flt[u32SecFltId]));
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set current read address of section buffer
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer
/// @param  u32ReadAddr             \b IN: address of read pointer
/// @return DRVTSP_RESULT
/// @note
/// User can update the read address to notify TSP where is the end of section
/// data already read back by user.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_SetReadAddr(U32 u32EngId, U32 u32SecFltId, U32 u32ReadAddr)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    MHal_TSP_SecFlt_SetBufRead(&(_TspSec[u32EngId].Flt[u32SecFltId]), u32ReadAddr);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get current section 32-bit CRC value
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer
/// @param  pu32CRC32               \b OUT: pointer of CRC value return
/// @return DRVTSP_RESULT
/// @note
/// It's the CRC32 value of completed section data. Non-zero CRC32 value means the
/// data integrity of a section is incorrect.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_GetCRC32(U32 u32EngId, U32 u32SecFltId, U32 *pu32CRC32)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(pu32CRC32, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));
    *pu32CRC32= MHal_TSP_SecFlt_GetCRC32(&(_TspSec[u32EngId].Flt[u32SecFltId]));
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
//[Reserved]
// Get current section filter status
// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
// @param  u32SecFltId             \b IN: index of section filter
// @param  peState                 \b OUT: current ORed state flag of section filter
//                                         E_DRVTSP_FLT_STATE_ALLOC\n
//                                         E_DRVTSP_FLT_STATE_ENABLE\n
//                                         E_DRVTSP_FLT_STATE_OVERFLOW
// @return DRVTSP_RESULT
// @note
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_GetState(U32 u32EngId, U32 u32SecFltId, DrvTSP_FltState *peState)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(peState, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));
    if (_MDrv_TSP_SecFlt_StateGet(u32EngId, u32SecFltId, peState))
    {
        _TSP_RETURN(DRVTSP_OK);
    }
    else
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }
}

//--------------------------------------------------------------------------------------------------
/// Subscribe event notification callback function for specified section filter
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer
/// @param  eEvents                 \b IN: events need to be subscribed\n
///                                        @ref E_DRVTSP_EVENT_DATA_READY\n
///                                        @ref E_DRVTSP_EVENT_BUF_OVERFLOW
/// @param  pfCallback              \b IN: callback function (NULL to disable)
/// @return DRVTSP_RESULT
/// @note
/// This function register a callback function for a section filter to TSP.
/// TSP calls callback function each time when data is ready in section buffer.\n
/// Data ready of section filter:\n
/// @ref E_DRVTSP_FLT_TYPE_SECTION : a section ready\n
/// @ref E_DRVTSP_FLT_TYPE_PES : PES packet ready or received data over than request size.
/// @sa MDrv_TSP_SecFlt_SetReqCount
/// @attention
/// Callback function resides in OS TSP interrupt context, it recommends
/// that callback function should not take too much time to block the system.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_Notify(U32 u32EngId, U32 u32SecFltId, DrvTSP_Event eEvents, P_DrvTSP_EvtCallback pfCallback)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));

    if ((eEvents & (E_DRVTSP_EVENT_DATA_READY   |
                    E_DRVTSP_EVENT_BUF_OVERFLOW  )) == 0)
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }

#ifndef MSOS_TYPE_LINUX
    _TspState[u32EngId].SecFltEvtNotify[u32SecFltId] = eEvents;
    _TspState[u32EngId].SecFltCallback[u32SecFltId] = pfCallback;
#endif // #ifndef MSOS_TYPE_LINUX

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get current section SW control value. For debug purpose.
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32SecFltId             \b IN: index of section buffer
/// @param  pu32Ctrl                \b OUT: SW Control Value
/// @return DRVTSP_RESULT
/// @note
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SecFlt_GetCtrl(U32 u32EngId, U32 u32SecFltId, U32 *pu32Ctrl)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    TSP_ASSERT(TSP_ENGINE_NUM> u32EngId, printf("[TSP_ERROR][%06d] Bad Engine Id %d\n", __LINE__, u32EngId));
    TSP_ASSERT(TSP_SECFLT_NUM> u32SecFltId, printf("[TSP_ERROR][%06d] Bad SecFltId %d\n", __LINE__, u32SecFltId));
    TSP_ASSERT(pu32Ctrl, printf("[TSP_ERROR][%06d] NULL pointer\n", __LINE__));
    *pu32Ctrl= MHal_TSP_SecFlt_GetCtrl(&(_TspSec[u32EngId].Flt[u32SecFltId]));
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set PVR double record buffer start addresses and buffer size
/// @param  u32BufStart0            \b IN: start address of PVR buffer 0
/// @param  u32BufStart1            \b IN: start address of PVR buffer 1
/// @param  u32BufSize              \b IN: size of section buffer
/// @return DRVTSP_RESULT
/// @note
/// Buffer start address and size should be 128-bit (16-byte) aligned\n
/// The maximum support size is 0xFFFF0 (1048560 bytes).
/// @sa MDrv_TSP_PVR_Notify
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_SetBuffer(U32 u32BufStart0, U32 u32BufStart1, U32 u32BufSize)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();
    MHal_TSP_PVR_SetBuffer(u32BufStart0, u32BufStart1, u32BufSize);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set PVR record mode and START/STOP recording.
/// @param  eRecMode                \b IN: mode of recording
/// @param  bStart                  \b IN: TRUE(START), FALSE(STOP)
/// @return DRVTSP_RESULT
/// @note
/// Parameter eRecMode will be ignored when bStart is FALSE(STOP)\n
/// @note
/// It's a synchronous function. When STOP, it flushs internal record fifo
/// and update current PVR record buffer write address before function return.\n
/// User can call MDrv_TSP_PVR_GetWriteAddr to get the final valid data address
/// after recording.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_Start(DrvTSP_RecMode eRecMode, B16 bStart)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    if (bStart)
    {
        _bPvrBuf0=  TRUE;
        if (_b192Mode)
        {
            MHal_TSP_PVR_TimeStampSetRecordStamp(0); //192 mode
            MHal_TSP_PVR_TimeStampRecord(TRUE); //192 mode
        }
        MHal_TSP_PVR_Enable(TRUE);
        MHal_TSP_PVR_Reset();
        MHal_TSP_PVR_SetMode(eRecMode);
        MHal_TSP_tsif_enable(1, TRUE); //192 mode
    }
    else
    {
        MHal_TSP_tsif_enable(1, FALSE); //192 mode
        MHal_TSP_PVR_Enable(FALSE);
        MHal_TSP_PVR_WaitFlush();
        MHal_TSP_PVR_TimeStampRecord(FALSE); //192 mode
    }

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get final write address of PVR record buffer
/// @param  pu32WriteAddr           \b OUT: pointer of address return
/// @return DRVTSP_RESULT
/// @note
/// User can get current write address to know where is the end of section data
/// received in the section buffer.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_GetWriteAddr(U32 *pu32WriteAddr)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    *pu32WriteAddr = MHal_TSP_PVR_GetBufWrite();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Subscribe event notification callback function for PVR record buffer
/// @param  eEvents                 \b IN: events need to be subscribed\n
///                                        @ref E_DRVTSP_EVENT_PVRBUF_FULL
/// @param  pfCallback              \b IN: callback function (NULL to disable)
/// @return DRVTSP_RESULT
/// @note
/// TSP PVR recording uses double buffer mechanism. This function register a callback
/// function for recording. TSP calls callback function each time when one of
/// double buffer is full and switch to another buffer to record.\n
/// @ref E_DRVTSP_FLT_TYPE_PVR
/// @attention
/// Callback function resides in OS TSP interrupt context, it recommends
/// that callback function should not take too much time to block the system.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_Notify(DrvTSP_Event eEvents, P_DrvTSP_EvtCallback pfCallback)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();

    if ((eEvents != E_DRVTSP_EVENT_PVRBUF_FULL)  &&
        //... Add new event here
        1)
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }
#if 0  // MaxCC 20080320
    if (pfCallback)
    {
        MHal_TSP_Int_Enable(TSP_HWINT_TSP_PVR_TAIL0_STATUS| TSP_HWINT_TSP_PVR_TAIL1_STATUS);
    }
    else
    {
        MHal_TSP_Int_Disable(TSP_HWINT_TSP_PVR_TAIL0_STATUS| TSP_HWINT_TSP_PVR_TAIL1_STATUS);
    }
#endif

    MHal_TSP_Int_Enable(TSP_HWINT_HW_MASK);

#ifndef MSOS_TYPE_LINUX
    _PvrEvtNotify = eEvents;
    _PvrCallback = pfCallback;
#endif // #ifndef MSOS_TYPE_LINUX

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// PVR time stamp set
/// @param  u32Stamp           \b IN: PVR time stamp
/// @return TSP_Result
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_TimeStampSetRecordStamp(U32 u32Stamp)
{
    _TSP_ENTRY();
    MHal_TSP_PVR_TimeStampSetRecordStamp(u32Stamp);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// PVR time stamp get
/// @param  pu32Stamp           \b OUT: pointer of PVR time stamp
/// @return TSP_Result
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_TimeStampGetRecordStamp(U32* u32Stamp)
{
    _TSP_ENTRY();
    *u32Stamp = MHal_TSP_PVR_TimeStampGetRecordStamp();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Playback time stamp Set
/// @param  pu32Stamp           \b IN: Playback time stamp
/// @return TSP_Result
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_TimeStampSetPlaybackStamp(U32 u32Stamp)
{
    _TSP_ENTRY();
    MHal_TSP_PVR_TimeStampSetPlaybackStamp(u32Stamp);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Playback time stamp get
/// @param  pu32Stamp           \b OUT: pointer of Playback time stamp
/// @return TSP_Result
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_TimeStampGetPlaybackStamp(U32* u32Stamp)
{
    _TSP_ENTRY();
    *u32Stamp = MHal_TSP_PVR_TimeStampGetPlaybackStamp();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// PVR time stamp file-in Enable/Disable
/// @param  bEnable             \b IN: Enable/Disable file-in time stamp
/// @return TSP_Result
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_TimeStampFileinEnable(B16 bEnable)
{
    _TSP_ENTRY();
    MHal_TSP_PVR_TimeStampEnable(bEnable);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Filein time stamp get
/// @param  pu32Stamp           \b OUT: pointer of Filein time stamp
/// @return TSP_Result
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_TimeStampGetFileinStamp(U32* u32Stamp)
{
    _TSP_ENTRY();
    *u32Stamp = MHal_TSP_PVR_TimeStampGetFileinPacket();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Enable/Disable Recording packets with 4 bytes more timestamp
/// @param  bEnable             \b IN: Enable/Disable recording with time stamp
/// @return TSP_Result
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PVR_TimeStampRecord(B16 bEnable)
{
    _TSP_ENTRY();
    if (bEnable)
    {
        _b192Mode = TRUE;
    }
    else
    {
        _b192Mode = FALSE;
    }
    MHal_TSP_PVR_TimeStampRecord(bEnable);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// START/STOP transmitting TS to MLink.
/// @param  bStart                  \b IN: TRUE(START), FALSE(STOP)
/// @return DRVTSP_RESULT
/// @note
///  After start transmitting TS, raw (unfiltered) TS is sent to MLink. \n
//   The input and output path is the same as which of PVR. \n
//   This function affects the settings of PVR.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_MLink_Start(U32 bStart)
{
    DBG_CHECK_POINT();
    _TSP_ENTRY();

    MHal_TSP_tsif_enable(1, bStart);
    if (bStart)
    {
        MHal_TSP_PVR_SetMode(E_DRVTSP_REC_MODE_ENG0_BYPASS);
    }

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Memory to TSP stream control : Stream start address
/// @param  u32StreamAddr           \b IN: pointer of transport stream in memory
/// @return DRVTSP_RESULT
/// @note M2T Command Size: 2
/// @sa MDrv_TSP_GetM2tSlot
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_M2T_SetAddr(U32 u32StreamAddr)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    MHal_TSP_CmdQ_TsDma_SetAddr(u32StreamAddr);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Memory to TSP stream control : File-in read pointer
/// @param  u32ReadPtr           \b OUT: File-in read pointer
/// @return DRVTSP_RESULT
/// @note File-in read pointer is 8-byte aligned.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_M2T_GetReadPtr(U32* u32ReadPtr)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    *u32ReadPtr = (MHal_TSP_CmdQ_TsDma_GetReadPtr());
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Memory to TSP stream control : Stream data size
/// @param  u32StreamSize           \b IN: size of transport stream data to be copied
/// @return DRVTSP_RESULT
/// @note M2T Command Size: 2
/// @sa MDrv_TSP_GetM2tSlot
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_M2T_SetSize(U32 u32StreamSize)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    MHal_TSP_CmdQ_TsDma_SetSize(u32StreamSize);
    _TSP_RETURN(DRVTSP_OK);
}


//--------------------------------------------------------------------------------------------------
/// Memory to TSP stream control : Start stream input
/// @param  eM2tMode                \b IN: input source control of M2T
/// @return DRVTSP_RESULT
/// @note M2T Command Size: 1
/// @sa MDrv_TSP_GetM2tSlot
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_M2T_Start(DrvTSP_M2tMode eM2tMode)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    MHal_TSP_CmdQ_TsDma_Start(eM2tMode);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Suspend TSP driver
/// @return DRVTSP_RESULT
/// @note
/// Save TSP driver states to DRAM
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_M2T_Pause(void)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    MHal_TSP_TsDma_Pause();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Resume TSP driver
/// @return DRVTSP_RESULT
/// @note
/// Restore TSP driver states from DRAM
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_M2T_Resume(void)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    MHal_TSP_TsDma_Resume();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
//[Reserved]
// Memory to TSP stream command : Update Stream STC
// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
// @param  u32STC_32               \b IN: STC[32] for next input stream
// @param  u32STC                  \b IN: STC[31:0] for next input stream
// @return DRVTSP_RESULT
// @note M2T Command Size: 3
// @sa MDrv_TSP_GetM2tSlot
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_M2T_SetSTC(U32 u32EngId, U32 u32STC_32, U32 u32STC)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    if (u32EngId>= TSP_ENGINE_NUM)
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }

    //[HW TODO][HW LIMIT]
    // STC register mapping is different between MCU and CPU.
    MHal_TSP_CmdQ_SetSTC(u32EngId, u32STC);
    MHal_TSP_CmdQ_SetSTC_32(u32EngId, u32STC_32);

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get current M2T state
/// @return DrvTSP_M2tStatus
/// @sa MDrv_TSP_GetM2tSlot
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_GetM2tState(DrvTSP_M2tState *peM2tState)
{
    U32 state, cmdcnt;

    //DBG_CHECK_POINT();

     _TSP_ENTRY();

    state = MHal_TSP_CmdQ_TsDma_GetState();
    cmdcnt = MHal_TSP_CmdQ_CmdCount();

    // @FIXME: Richard: ignore it at this stage
    if (state & TSP_CTRL1_FILEIN_PAUSE)
    {
        *peM2tState = E_DRVTSP_M2T_STATE_PAUSE;
    }
    else if ((state & TSP_TSDMA_CTRL_START) || cmdcnt)
    {
        *peM2tState = E_DRVTSP_M2T_STATE_BUSY;
    }
    else
    {
        *peM2tState = E_DRVTSP_M2T_STATE_IDLE;
    }

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set Memory to TSP Stream Input Rate
/// @param  u32Div2                 \b IN: Divider of M2T stream input rate ([1 .. 31], default 10)
/// @return DRVTSP_RESULT
/// @note
/// <b>input_rate = stream_rate / (u32Div2 * 2)</b>\n
/// @note
/// It's not recommend to change input rate at run-time, because it conflict with
/// the internal stream synchornization mechanism.
/// @sa MDrv_TSP_GetM2tSlot
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SetM2tRate(U32 u32Div2)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();

    //[HW TODO][HW LIMIT]
    // TsDma pause can be access by TSP CPU
    // TsDma pause it's hard to control because read/write in different register
    // When setting TsDma it should be disable interrupt to prevent ISR access
    // but it still can't prevent TSP_cpu access at the same time.
    //[SW PATCH] Add a special firmware command to lock TSP_cpu DMA pause/resume.

    IntDisable();
    MHal_TSP_TsDma_SetDelay(u32Div2);
    IntEnable();

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get the number of empty slot of M2T command
/// @param  pu32EmptySlot           \b OUT: pointer of empty slot count return
/// @return DRVTSP_RESULT
/// @note
/// M2T is a command queue command, it can be queued by TSP when another M2T command
/// is executing by TSP. The queued commands will be executed by order (FIFO)
/// when previous M2T command execution is finished.
/// @note
/// User should call GetM2TSlot to make sure there is enough empty M2T command slot
/// before sending any M2T command. (Each command has different command size)
/// @sa MDrv_TSP_M2T_SetAddr, MDrv_TSP_M2T_SetSize, MDrv_TSP_M2T_Start,

//[Reserved]    MDrv_TSP_M2T_SetSTC
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_GetM2tSlot(U32 *pu32EmptySlot)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    *pu32EmptySlot = MHal_TSP_CmdQ_EmptyCount();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get the write level of M2T command queue
/// @param  pu32EmptySlot           \b OUT: pointer of write level return
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_GetM2tWriteLevel(U32 *pu32Level)
{
    _TSP_ENTRY();
    *pu32Level = MHal_TSP_CmdQ_GetWriteLevel();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Clear command queue
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_M2T_Reset(void)
{
#if defined(CONFIG_Titania3)
    _TSP_ENTRY();
    MHal_TSP_CmdQ_ClearQ();
    _TSP_RETURN(DRVTSP_OK);
#else
    return DRVTSP_FAIL;
#endif
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_Get_VFifoLevel(U32 *pu32Level)
{
#if defined(CONFIG_Titania3)
    _TSP_ENTRY();
    //return (U32)MHal_TSP_Get_AudioLevel();
    _TSP_RETURN(DRVTSP_OK);
#else
    return DRVTSP_FAIL;
#endif
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_Get_AFifoLevel(U32 *pu32Level)
{
#if defined(CONFIG_Titania3)
    _TSP_ENTRY();
    //return (U32)MHal_TSP_Get_AudioLevel();
    _TSP_RETURN(DRVTSP_OK);
#else
    return DRVTSP_FAIL;
#endif
}

//--------------------------------------------------------------------------------------------------
/// Get current system time clock (STC) of TSP
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  pu32STC_32              \b OUT: pointer of STC[32] return
/// @param  pu32STC                 \b OUT: pointer of STC[31:0] return
/// @return DRVTSP_RESULT
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_GetSTC(U32 u32EngId, U32 *pu32STC_32, U32 *pu32STC)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();

    if (u32EngId    >= TSP_ENGINE_NUM)
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }

    *pu32STC = MHal_TSP_GetSTC(u32EngId);
    *pu32STC_32 = MHal_TSP_GetSTC_32(u32EngId);

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set TSP engine operating mode
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  eCtrlMode               \b IN: TSP engine operating mode
/// @return DRVTSP_RESULT
/// @note
/// Only one engine can be set to use memory stream input mode, and all kinds
/// of input are mutually exclusive for each engine.\n
/// Only one engine can output to MAD. The TSP engine which is not selected to
/// MAD can not filter any audio stream, it will block the stream.
/// @sa MDrv_TSP_PidFlt_Alloc, MDrv_TSP_PidFlt_Enable, E_DRVTSP_FLT_TYPE_AUDIO
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SetMode(U32 u32EngId, DrvTSP_CtrlMode eCtrlMode)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();

    if (u32EngId    >= TSP_ENGINE_NUM)
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }

    // if old mode is TSP_CTRL_TSDMA_EN
    if ((MHal_TSP_GetCtrlMode(u32EngId) & TSP_CTRL_TSFILE_EN)||
        (eCtrlMode == E_DRVTSP_CTRL_MODE_MEM_AUD)       ||
        (eCtrlMode == E_DRVTSP_CTRL_MODE_MEM_VID)       ||
        (eCtrlMode == E_DRVTSP_CTRL_MODE_MEM))
    {
        MHal_TSP_CmdQ_TsDma_Reset();
    }

    switch (eCtrlMode)
    {
    case E_DRVTSP_CTRL_MODE_TS0_AUD:
        MHal_TSP_SelAudOut(u32EngId);
    case E_DRVTSP_CTRL_MODE_TS0:
        MHal_TSP_PSAudioDisable(u32EngId);
        MHal_TSP_PSVideoDisable(u32EngId);
        //MHal_TSP_DoubleBuf_Disable(); //TSIN/FILE -> PP
        MHal_TSP_DoubleBuf_En(TRUE);
        MHal_TSP_SetCtrlMode(u32EngId, TSP_CTRL_CPU_EN | TSP_CTRL_SW_RST, 0);//[URANUS] | TSP_CTRL_CLK_GATING_DISABLE);
        //MHal_TSP_SetFwMsg(MHal_TSP_GetFwMsg() & ~(TSP_MSG_FW_STC_NOSYNC << u32EngId));
        break;

    case E_DRVTSP_CTRL_MODE_TS1_AUD:
        MHal_TSP_SelAudOut(u32EngId);
    case E_DRVTSP_CTRL_MODE_TS1:
        MHal_TSP_PSAudioDisable(u32EngId);
        MHal_TSP_PSVideoDisable(u32EngId);
        //MHal_TSP_DoubleBuf_Disable(); //TSIN/FILE -> PP
        MHal_TSP_DoubleBuf_En(TRUE);
        MHal_TSP_SetCtrlMode(u32EngId, TSP_CTRL_CPU_EN | TSP_CTRL_SW_RST, 1);//[URANUS] | TSP_CTRL_CLK_GATING_DISABLE);
        //MHal_TSP_SetFwMsg(MHal_TSP_GetFwMsg() & ~(TSP_MSG_FW_STC_NOSYNC << u32EngId));
        break;

    case E_DRVTSP_CTRL_MODE_MEM_AUD:
    case E_DRVTSP_CTRL_MODE_MEM_VID:
        if (E_DRVTSP_CTRL_MODE_MEM_VID == eCtrlMode) {
            MHal_TSP_PSAudioDisable(u32EngId);
            MHal_TSP_PSVideoEnable(u32EngId);
        }
        else
        {
            MHal_TSP_PSVideoDisable(u32EngId);
            MHal_TSP_PSAudioEnable(u32EngId);
        }
    // disable filein to pvr
    //case E_DRVTSP_CTRL_MODE_MEM_PVR:
        // MHal_TSP_CSA_ENABLE(TRUE);
        // bounding option
        // MHal_TSP_CSA_REC_ENABLE(TRUE);
    case E_DRVTSP_CTRL_MODE_MEM:
        MHal_TSP_DoubleBuf_En(TRUE); //TSIN -> PP, FILE -> Single
        MHal_TSP_SetCtrlMode(u32EngId, TSP_CTRL_CPU_EN | TSP_CTRL_SW_RST| TSP_CTRL_TSFILE_EN, 0);//[URANUS] | TSP_CTRL_CLK_GATING_ENABLE);
        //MHal_TSP_SetFwMsg(MHal_TSP_GetFwMsg() | (TSP_MSG_FW_STC_NOSYNC << u32EngId));
        //MHal_TSP_Reset_AvFifo(u32EngId); // Clear AV fifo
        break;

    // disable tsif0 to pvr
    //case E_DRVTSP_CTRL_MODE_PVR_TS0:
        // MHal_TSP_CSA_ENABLE(TRUE);
        // bounding option
        // MHal_TSP_CSA_REC_ENABLE(TRUE);
        //break;
    case E_DRVTSP_CTRL_MODE_PVR_AND_MEM:
        MHal_TSP_DoubleBuf_En(TRUE); //TSIN -> PP, FILE -> Single
        MHal_TSP_SetCtrlMode(u32EngId, TSP_CTRL_CPU_EN | TSP_CTRL_SW_RST| TSP_CTRL_TSFILE_EN, 0);//[URANUS] | TSP_CTRL_CLK_GATING_ENABLE);
        MHal_TSP_SetCtrlMode(u32EngId, TSP_CTRL_CPU_EN | TSP_CTRL_SW_RST| TSP_CTRL_TSFILE_EN, 1);//[URANUS] | TSP_CTRL_CLK_GATING_DISABLE);
        //MHal_TSP_Reset_AvFifo(u32EngId); // Clear AV fifo
        break;

    default:
        _TSP_RETURN(DRVTSP_FAIL);
        break;
    }
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set TS Input pad
/// @param  u32EngId         \b IN: Index of TSP engine [ 0 .. (@ref _EngNum-1) ]
/// @param  ePad             \b IN: Input pad. \n
///                                        @ref E_DRVTSP_PAD_PLAYCARD   \n
///                                        @ref E_DRVTSP_PAD_CI         \n
///                                        @ref E_DRVTSP_PAD_DEMOD      \n,
///                                        @ref E_DRVTSP_PAD_DEMOD_INV - with TS clock inverted  \n
///                                        @ref E_DRVTSP_PAD_EXT_INPUT0 \n
///                                        @ref E_DRVTSP_PAD_EXT_INPUT1 \n
/// @return DRVTSP_OK - Successfully completed.\n
///         DRVTSP_FAIL - Failed.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SelPad(U32 u32EngId, DrvTSP_PadIn ePad)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    if (u32EngId>= TSP_ENGINE_NUM)
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }
    _MDrv_TSP_SelPad(u32EngId, ePad);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set TS input interface type, either serial or parallel.
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref DRVTSP_ENGINE_NUM-1) ]
/// @param  u32TSIFType             \b IN: TS input interface type {E_DRVTSP_TSIF_PARALLEL, E_DRVTSP_TSIF_SERIAL}
/// @return DRVTSP_RESULT
/// @note
/// Both TS input interfaces (TSIF0, TSIF1) are set.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_SetTSIFType(U32 u32EngId, U32 u32TSIFType)
{
    U32 u32HalTSIFType;

    DBG_CHECK_POINT();

    _TSP_ENTRY();

    if (u32EngId>= TSP_ENGINE_NUM)
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }

    switch(u32TSIFType)
    {
        case E_DRVTSP_TSIF_PARALLEL:
            u32HalTSIFType = E_HALTSP_TSIF_PARALLEL;
            break;
        case E_DRVTSP_TSIF_SERIAL:
            u32HalTSIFType = E_HALTSP_TSIF_SERIAL;
            break;
        default:
            _TSP_RETURN(DRVTSP_FAIL);
    }
    MHal_TSP_SetTSIFType(u32EngId, 0, u32HalTSIFType);
    MHal_TSP_SetTSIFType(u32EngId, 1, u32HalTSIFType);

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Set TSP file-in packet size
/// @param  ePktMode                \b IN: Mode of TSP file packet mode (192, 204, 188)
/// @return TSP_Result
/// @note
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_File_SetPacketMode(DrvTSP_PacketMode ePktMode)
{
    _TSP_ENTRY();

    switch (ePktMode)
    {
        case E_DRVTSP_PKTMODE_188:
            MHal_TSP_SetPktSize(0xBB);
            break;
        case E_DRVTSP_PKTMODE_192:
            MHal_TSP_SetPktSize(0xBF);
            break;
        case E_DRVTSP_PKTMODE_204:
            MHal_TSP_SetPktSize(0xCB);
            break;
        default:
            break;
    }

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Check if any TS packet which is processed by the current PID filters is scrambled at TS level.
/// @param  u32EngId                \b IN: Index of TSP engine [ 0 .. (@ref _EngNum-1) ]
/// @param  pScmbLevel              \b OUT: Scrambling status. \n
///                                        @ref E_DRVTSP_SCMB_NONE - No TS packets are scrambled. \n
///                                        @ref E_DRVTSP_SCMB_TS - At lease one PID filter is processing
///                                                                scrambled packets.
/// @return DRVTSP_OK - Successfully completed.\n
///         DRVTSP_FAIL - Failed.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_Scmb_Status(U32 u32EngId, DrvTSP_Scmb_Level* pScmbLevel)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();
    *pScmbLevel=        E_DRVTSP_SCMB_NONE;

    if (MHal_TSP_Scmb_Status(TRUE, 0xFFFFFFFF) || MHal_TSP_Scmb_Status(FALSE, 0xFFFFFFFF))
        *pScmbLevel=    E_DRVTSP_SCMB_TS;
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get TSP HW information
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_GetCFG(U32 u32CfgId, U32* pu32Status, U32* pu32Value)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();

    *pu32Status = DRVTSP_CFG_STATUS_OK;

    switch(u32CfgId)
    {
        case E_DRVTSP_CFG_ENG_NUM:
            *pu32Value = TSP_ENGINE_NUM;
            break;

        case E_DRVTSP_CFG_PIDFLT0_NUM:
            *pu32Value = TSP_PIDFLT_NUM;
            break;

        case E_DRVTSP_CFG_PIDFLT1_NUM:
            *pu32Value = TSP_PIDFLT1_NUM;
            break;

        case E_DRVTSP_CFG_SECFLT_NUM:
            *pu32Value = TSP_SECFLT_NUM;
            break;

        case E_DRVTSP_CFG_FLT_DEPTH:
            *pu32Value = TSP_FILTER_DEPTH;
            break;

        case E_DRVTSP_CFG_SDR_BASE:
            *pu32Value = MHal_TSP_GetSDRBase();
            break;

        default:
            *pu32Status = DRVTSP_CFG_STATUS_ERR_ID;
            *pu32Value = -1;
            _TSP_RETURN(DRVTSP_FAIL);
    }

    _TSP_RETURN(DRVTSP_OK);

}

//--------------------------------------------------------------------------------------------------
/// Get Last General Error Return Code
/// @return DRVTSP_RESULT
/// @note
/// Last error return code is reset after calling this function.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_GetLastErr(void)
{
    DRVTSP_RESULT ret;
    ret = _u32LastErr;
    _u32LastErr = DRVTSP_OK;
    return ret;
}

//--------------------------------------------------------------------------------------------------
///[OBSOLETE]
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_Reset(void)
{
    U32     TempPidFlt[TSP_ENGINE_NUM][TSP_PIDFLT_NUM_ALL];
    U32     u32EngId, u32FltId;

    DBG_CHECK_POINT();

    //[NOTE] Reset TSP at Run-time
    // REQUIRE:
    // 1. Firmware code integrity is correct. (firmware code)
    // 2. TSP register current is correct configurated. (register not changed)
    // 3. OS and upper task is alive. (mutex, event are available and not changed)
    // 4. Only reset hardware/firmware state machine.
    _TSP_ENTRY();

    // [URANUS] Richard: Refine the interrupt selection later.
    MHal_TSP_Int_Disable(TSP_HWINT_ALL);

    IntDisable();

    // Save PID filter, Disable PID filter, Reset all buffer pointer
    for (u32EngId = 0; u32EngId < TSP_ENGINE_NUM; u32EngId++)
    {
        for (u32FltId = 0; u32FltId < TSP_PIDFLT_NUM_ALL; u32FltId++)
        {
            U32     u32PidFlt;
            u32PidFlt = MHal_TSP_PidFlt_GetFltOutput(&_TspPid[u32EngId].Flt[u32FltId]);
            TempPidFlt[u32EngId][u32FltId] = u32PidFlt;
            MHal_TSP_PidFlt_SelFltOutput(&_TspPid[u32EngId].Flt[u32FltId], TSP_PIDFLT_OUT_NONE); // disable
        }

        for (u32FltId = 0; u32FltId < TSP_SECFLT_NUM; u32FltId++)
        {
            MHal_TSP_SecFlt_ResetBuffer(&_TspSec[u32EngId].Flt[u32FltId]);
            MHal_TSP_SecFlt_ResetState(&_TspSec[u32EngId].Flt[u32FltId]);
        }
    }

    // Do reset
    MHal_TSP_SwReset(); //jyliu.tsp.0330, what will be cleared after reset?
    MHal_TSP_Int_ClearSw();
    MHal_TSP_Int_ClearHw(TSP_HWINT_ALL);

    // Restore PID filter setting.
    for (u32EngId = 0; u32EngId < TSP_ENGINE_NUM; u32EngId++)
    {
        for (u32FltId = 0; u32FltId < TSP_PIDFLT_NUM_ALL; u32FltId++)
        {
            MHal_TSP_PidFlt_SelFltOutput(&_TspPid[u32EngId].Flt[u32FltId], TempPidFlt[u32EngId][u32FltId]); // disable
        }
    }
    IntEnable();
    // [URANUS] Richard: Refine the interrupt selection later.
    MHal_TSP_Int_Enable(TSP_HWINT_ALL);                                     // Enable TSP hardware interrupt
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Initialize TSP driver and TSP engine
/// @return DRVTSP_RESULT
/// @note
/// It should be called before calling any other TSP DDI functions.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_Init(void)
{
    U32     u32EngId, u32FilterId;
    int     result; // added to remove warning(dreamer@lge.com)

    DBG_TSP(printk("[MDRV][Kernel]: Init\n"));

    DBG_CHECK_POINT();

    DBG_TSP(printf("TSP Driver Build %s\n", _TspBuildDate));

    //MDrv_TSP_PowerCtrl(TRUE);
#if defined(CONFIG_Titania3)
    //TS0, TS1
    CKLGEN0_REG(REG_CLKGEN0_TS) =
        CKLGEN0_REG(REG_CLKGEN0_TS) & ~(CLKGEN0_CKG_TS0_MASK | CLKGEN0_CKG_TS1_MASK);
    //TSOUT
    CKLGEN0_REG(REG_CLKGEN0_TSP_TSOUT) =
        CKLGEN0_REG(REG_CLKGEN0_TSP_TSOUT) & ~(CLKGEN0_CKG_TSOUT_MASK);
    //TSP, STC0
    CKLGEN0_REG(REG_CLKGEN0_TSP_STC0) =
        CKLGEN0_REG(REG_CLKGEN0_TSP_STC0) & ~(CLKGEN0_CKG_TSP_MASK | CLKGEN0_CKG_STC0_MASK);

    //switch stc adjustment from chiptop to TSP
    CKLGEN0_REG(REG_CLKGEN0_STC0_DC0) = CKLGEN0_REG(REG_CLKGEN0_STC0_DC0) | CLKGEN0_STC0_CW_SEL;
#endif

    // Diable interrupt
    // [URANUS] Richard: Refine the interrupt selection later.
    MHal_TSP_Int_Disable(TSP_HWINT_ALL);
    IntDisable();

#if defined(CONFIG_Titania3)
    if (FALSE == MHal_TSP_Alive())
    {
        printk("TSP Init : TSP Sw Reset\n");
        MHal_TSP_SwReset();
    }

    do
    {
#endif

    // Disable CPU
    MHal_TSP_SetCtrlMode(0, 0x0, 2);                        // Disable TSP0(Reset CPU), Software_Reset

    // Richard: Enable indirect access
    MHal_TSP_Ind_Enable();

    _MDrv_TSP_CmdQ_Init();


// taburin : 20091128    MHal_TSP_NewDmaWriteArbiter(TRUE);

    MHal_TSP_WbDmaEnable(TRUE);

    //MHal_TSP_Scmb_Detect(TRUE);
    //_TSP_DoubleBuf_En(DOUBLE_BUFFER_SWITCH);              // [OBERON]:Enable double buffer, set filein->pinpon livein->single
    //MHal_TSP_DoubleBuf_Disable(); //TSIN/FILE -> PP
    MHal_TSP_DoubleBuf_En(TRUE);
    //MHal_TSP_CSA_Sel_ScrmEng(TSP_CSA_SCRM_ENG_CSA);       // [OBERON]:Set descrmbler as CSA
    //MHal_TSP_SetDuplicateSkip(); // Skip duplicate packet
    // _TSP_NMatch_Set(FALSE);   // Disable Nmatch

    MHal_TSP_HwPatch();

    for (u32EngId = 0; u32EngId < TSP_ENGINE_NUM; u32EngId++)
    {
        _TspEnable[u32EngId] = 0;
        for (u32FilterId = 0; u32FilterId < TSP_PIDFLT_NUM_ALL; u32FilterId++)
        {
            _TspState[u32EngId].FltState[u32FilterId] = E_DRVTSP_FLT_STATE_FREE;
            _TspState[u32EngId].FltType[u32FilterId] = E_DRVTSP_FLT_TYPE_SECTION; // for section filter
            _MDrv_TSP_PidFlt_Init(u32EngId, u32FilterId);
        }
        for (u32FilterId = 0; u32FilterId < TSP_SECFLT_NUM; u32FilterId++)
        {
            _TspState[u32EngId].SecFltState[u32FilterId] = E_DRVTSP_FLT_STATE_FREE;
            _MDrv_TSP_SecFlt_Init(u32EngId, u32FilterId);
            MHal_TSP_SecFlt_SetBuffer(&(_TspSec[u32EngId].Flt[u32FilterId]), 0, 0);
        }
        // Richard: only use the lowest 7 control word at this first stage
        // It may change later
    }

    MHal_TSP_CPU_SetBase();

    // Synthesizer Tuning
    // For difference : 100 - 4000 STC
    // Step: 10 STC
    //MHal_TSP_SetFwMsg(0x03A4); // Catch up n*10 STC in 1/20 sec

    //2009.12.31
    MHal_Reset_WB();

    MHal_TSP_SetCtrlMode(0, TSP_CTRL_CPU_EN | TSP_CTRL_SW_RST, 0);// | TSP_CTRL_CLK_GATING_DISABLE); // Enable TSP CPU

#if defined(CONFIG_Titania3)
    }
    while(FALSE == MHal_TSP_Alive());
#endif

    MHal_TSP_Int_ClearSw();
    MHal_TSP_Int_ClearHw(TSP_HWINT_ALL);


    MHal_TSP_SetTSIFExtSync(0, 0, TRUE);
    MHal_TSP_SetTSIFType(0, 0, E_DRVTSP_TSIF_PARALLEL);
    MHal_TSP_SetTSIFExtSync(0, 1, TRUE);
    MHal_TSP_SetTSIFType(0, 1, E_DRVTSP_TSIF_PARALLEL);

    MHal_TSP_TsDma_SetDelay(0x0A);                                          // Set TsDma delay, //jyliu.tsp, how come the value?

	if(0 == u32TSPIntRegister) {
    	result = IntAttach();   // changed to remove warning(dreamer@lge.com)
		if (result) {
			return -EBUSY;
		}

		u32TSPIntRegister= 1;
	} else {
		disable_irq(E_IRQ_TSP);
		enable_irq(E_IRQ_TSP);
	}

    MHal_TSP_Int_Enable(TSP_HWINT_ALL);                                     // Enable TSP hardware interrupt
    MHal_TSP_Scmb_Detect(TRUE);

    _u32LastErr = DRVTSP_OK;
    return _u32LastErr;
}

//--------------------------------------------------------------------------------------------------
/// Enable or Disable TSP clock for power-saving control
/// @param  bOn                 \b IN: power-on or power-off
/// @return DRVTSP_RESULT
/// @note
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_PowerCtrl(B16 bOn)
{
    _TSP_ENTRY();
    if (bOn)
    {
#if defined(CONFIG_Titania3)
#if 0
        //TS0, TS1
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            CKLGEN0_REG(REG_CLKGEN0_TS) & ~(CLKGEN0_CKG_TS0_MASK | CLKGEN0_CKG_TS1_MASK);
        //TSOUT
        CKLGEN0_REG(REG_CLKGEN0_TSP_TSOUT) =
            CKLGEN0_REG(REG_CLKGEN0_TSP_TSOUT) & ~(CLKGEN0_CKG_TSOUT_MASK);
        //TSP, STC0
        CKLGEN0_REG(REG_CLKGEN0_TSP_STC0) =
            CKLGEN0_REG(REG_CLKGEN0_TSP_STC0) & ~(CLKGEN0_CKG_TSP_MASK | CLKGEN0_CKG_STC0_MASK);

        //switch stc adjustment from chiptop to TSP
        CKLGEN0_REG(REG_CLKGEN0_STC0_DC0) = CKLGEN0_REG(REG_CLKGEN0_STC0_DC0) | CLKGEN0_STC0_CW_SEL;
#else
        MDrv_TSP_Init();
#endif
#else
        //T2
        //switch stc adjustment from chiptop to TSP
        TOP_REG(REG_TOP_MCU_USB_STC0) = TOP_REG(REG_TOP_MCU_USB_STC0) | TSP_STC0_SEL;
#endif
    }
    else
    {
        _MDrv_TSP_Close();
#if defined(CONFIG_Titania3)
#if 0
        //TS0, TS1
        CKLGEN0_REG(REG_CLKGEN0_TS) =
            CKLGEN0_REG(REG_CLKGEN0_TS) | (CLKGEN0_CKG_TS0_DIS | CLKGEN0_CKG_TS1_DIS);
        //TSOUT
        CKLGEN0_REG(REG_CLKGEN0_TSP_TSOUT) =
            CKLGEN0_REG(REG_CLKGEN0_TSP_TSOUT) | (CLKGEN0_CKG_TSOUT_DIS);
        //TSP, STC0
        CKLGEN0_REG(REG_CLKGEN0_TSP_STC0) =
            CKLGEN0_REG(REG_CLKGEN0_TSP_STC0) | (CLKGEN0_CKG_TSP_DIS | CLKGEN0_CKG_STC0_DIS);
#endif
#else
        //T2
#endif
        // Richard: this is not a bug. Intend to lock to mutex to prevent alive task accessing TSP after power off
        // return DRVTSP_OK;
    }
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
///[OBSOLETE] Apply MDrv_TSP_PowerCtrl() instead
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_Close(void)
{
    _TSP_ENTRY();
    _MDrv_TSP_Close();
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// CI+ : Set the input/output datapath
/// @param  u32EngId                \b IN: Index of TSP engine [ 0 .. (@ref _EngNum-1) ]
/// @param  eInPath                 \b IN: TSIF0_LIVE or TSIF0_FILE or TSIF1
///                                         @ref E_DRVTSP_CSA_DATAPATH_INPUT_LIVE_IN    \n
///                                         @ref E_DRVTSP_CSA_DATAPATH_INPUT_FILE_IN    \n
///                                         @ref E_DRVTSP_CSA_DATAPATH_INPUT_REC_LIVE   \n
/// @param  eOutPath                \b IN: PLAY_LIVE or PLAY_FILE or REC_DESCRMB
///                                         @ref E_DRVTSP_CSA_DATAPATH_OUTPUT_PLAY_LIVE     \n
///                                         @ref E_DRVTSP_CSA_DATAPATH_OUTPUT_PLAY_FILE     \n
///                                         @ref E_DRVTSP_CSA_DATAPATH_OUTPUT_REC_DESCRMB   \n
/// @param  bEnableOutputAV         \b IN: Enable this flag if data will output to AV fifo
/// @return DRVTSP_OK - Successfully completed.\n
///         DRVTSP_FAIL - Failed.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_CSA_SetScmbPath(U32 u32EngId, DrvTSP_CSA_DataPath eInPath, DrvTSP_CSA_DataPath eOutPath, B16 bEnableOutputAV)
{
    U32 u32Path = 0;
    _TSP_ENTRY();
    if (eInPath & E_DRVTSP_CSA_DATAPATH_INPUT_LIVE_IN)
        u32Path |= TSP_CA_INPUT_TSIF0_LIVEIN;
    if (eInPath & E_DRVTSP_CSA_DATAPATH_INPUT_FILE_IN)
        u32Path |= TSP_CA_INPUT_TSIF0_FILEIN;
    if (eInPath & E_DRVTSP_CSA_DATAPATH_INPUT_REC_LIVE)
        u32Path |= TSP_CA_INPUT_TSIF1;

    if (eOutPath & E_DRVTSP_CSA_DATAPATH_OUTPUT_PLAY_LIVE)
        u32Path |= TSP_CA_OUTPUT_PLAY_LIVE;
    if (eOutPath & E_DRVTSP_CSA_DATAPATH_OUTPUT_PLAY_FILE)
        u32Path |= TSP_CA_OUTPUT_PLAY_FILE;
    if (eOutPath & E_DRVTSP_CSA_DATAPATH_OUTPUT_REC_DESCRMB)
        u32Path |= TSP_CA_OUTPUT_REC;

    MHal_TSP_CSA_Set_ScrmPath(u32Path, bEnableOutputAV);
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// CI+ : Set the decryption mode
/// @param  u32EngId                \b IN: Index of TSP engine [ 0 .. (@ref _EngNum-1) ]
/// @param  eProtocol               \b IN: AES or DES
///                                         @ref E_DRVTSP_CSA_PROTOCOL_DES  \n
///                                         @ref E_DRVTSP_CSA_PROTOCOL_AES  \n
/// @return DRVTSP_OK - Successfully completed.\n
///         DRVTSP_FAIL - Failed.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_CSA_SetEsaMode(U32 u32EngId, DrvTSP_CSA_Protocol eProtocol)
{
    _TSP_ENTRY();
    if (eProtocol == E_DRVTSP_CSA_PROTOCOL_DES)
    {
        MHal_TSP_CSA_Set_ESA_Mode(TSP_CSA_ESA_DES_MODE|TSP_CSA_ESA_ECB_MODE|TSP_CSA_ESA_DECRYPT);
    }
    else if (eProtocol == E_DRVTSP_CSA_PROTOCOL_AES)
    {
        MHal_TSP_CSA_Set_ESA_Mode(TSP_CSA_ESA_AES_MODE|TSP_CSA_ESA_CBC_CLR_MODE|TSP_CSA_ESA_DECRYPT);
    }
    else
    {
        //DBG_TSP(printf("Unknown CSA ESA mode %d\n", eProtocol);
        _TSP_RETURN(DRVTSP_FAIL);
    }

    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// CI+ : Set the cipher keys
/// @param  u32EngId                \b IN: Index of TSP engine [ 0 .. (@ref _EngNum-1) ]
/// @param  eProtocol               \b IN: AES or DES
/// @param  eKey                    \b IN: Even key or odd key
/// @param  u8FID                   \b IN: Pid filter number
/// @param  pu32CipherKeys          \b IN: from lower bits to higher bits;
///                                        AES: IVs 128bits + Keys 128bits,
///                                        DES: Keys 64bits.
/// @return DRVTSP_OK - Successfully completed.\n
///         DRVTSP_FAIL - Failed.

/*
   IVs[] = {0xA4,0xA3,0xA2,0xA1, 0xC4,0xC3,0xC2,0xC1, 0xB4,0xB3,0xB2,0xB1, 0xD4,0xD3,0xD2,0xD1};
   Key[] = {0x08,0x07,0x06,0x05, 0x28,0x27,0x26,0x25, 0x18,0x17,0x16,0x15, 0x38,0x37,0x36,0x35};
   pu32CipherKeys[8] = {0xA4A3A2A1, 0xC4C3C2C1, 0xB4B3B2B1, 0xD4D3D2D1,
                        0x08070605, 0x28272625, 0x18171615, 0x38373635};

   [Usage]
   U8 u8DESCCK[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
   MAdp_TSP_CIP_SetCipherKeys (0, u32EsaMode, u32KeyMode, (U32*) u8DESCCK);

   U32 u32DESCCK[2] = { 0x44332211, 0x88776655};
   MAdp_TSP_CIP_SetCipherKeys (0, u32EsaMode, u32KeyMode, u32DESCCK);

*/
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_CSA_CW_SetKey(U32 u32EngId, DrvTSP_CSA_Protocol eProtocol, DrvTSP_CSA_Key eKey, U32 u32FID, U32* pau32CipherKeys)
{
    #define DES_RW_POS_OFFSET        4

    // FID starts from PidFilter 16, which FID is 0
    U8 i;
    U32 u32Tsc; //DrvTSP_CSA_TSC enTsc;

    _TSP_ENTRY();
    u32Tsc = (eKey==E_DRVTSP_CSA_KEY_EVEN) ? TSP_CW_EVEN : TSP_CW_ODD;

    switch (eProtocol)
    {
        case E_DRVTSP_CSA_PROTOCOL_AES:
            for ( i=0 ; i<=7 ; i++ )
            {
                MHal_TSP_CSA_KTE_Set((i/7), u32Tsc, u32FID, i, *(pau32CipherKeys+i) );
            }
            break;

        case E_DRVTSP_CSA_PROTOCOL_DES:
            for ( i=0 ; i<=1 ; i++ )
            {
                MHal_TSP_CSA_KTE_Set((i/1), u32Tsc, u32FID, (i+DES_RW_POS_OFFSET), *(pau32CipherKeys+i) );
            }
            break;

        default:
            //DBG_TSP(printf("CIP_SetCipherKeys>> Error protocol type\n"));
            _TSP_RETURN(DRVTSP_FAIL);
            break;
    }
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Flush audio/video fifo or not
/// @return DRVTSP_RESULT
/// @note
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_Fifo_Flush(DrvTSP_FltType eFilterType, B16 bFlush)
{
    _TSP_ENTRY();
    switch (eFilterType)
    {
    case E_DRVTSP_FLT_TYPE_VIDEO:
        MHal_TSP_Flush_AV_FIFO(0, bFlush);
        break;
    case E_DRVTSP_FLT_TYPE_AUDIO:
        MHal_TSP_Flush_AV_FIFO(1, bFlush);
        break;
    case E_DRVTSP_FLT_TYPE_AUDIO2:
        MHal_TSP_Flush_AV_FIFO(2, bFlush);
        break;
    default:
        _TSP_RETURN(DRVTSP_FAIL);
    }
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get one byte from video or audio fifo. For debug purpose.
/// @return DRVTSP_RESULT
/// @note
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_Fifo_Fetch(DrvTSP_FltType eFilterType, U8* u8Byte)
{
    _TSP_ENTRY();
    switch (eFilterType)
    {
    case E_DRVTSP_FLT_TYPE_VIDEO:
        *u8Byte = MHal_TSP_Fetch_AV_FIFO(0);
        break;
    case E_DRVTSP_FLT_TYPE_AUDIO:
        *u8Byte = MHal_TSP_Fetch_AV_FIFO(1);
        break;
    case E_DRVTSP_FLT_TYPE_AUDIO2:
        *u8Byte = MHal_TSP_Fetch_AV_FIFO(2);
        break;
    default:
        _TSP_RETURN(DRVTSP_FAIL);
    }
    _TSP_RETURN(DRVTSP_OK);
}

//--------------------------------------------------------------------------------------------------
/// Get current PCR
/// @param  u32EngId                \b IN: index of TSP engine [ 0 .. (@ref _EngNum-1) ]
/// @param  pu32Pcr_32              \b OUT: pointer of PCR[32] return
/// @param  pu32Pcr                 \b OUT: pointer of PCR[31:0] return
/// @return DRVTSP_OK - Successfully completed.\n
///         DRVTSP_FAIL - Failed.
//--------------------------------------------------------------------------------------------------
DRVTSP_RESULT MDrv_TSP_GetPcr(U32 u32EngId, U32 *pu32Pcr_32, U32 *pu32Pcr)
{
    DBG_CHECK_POINT();

    _TSP_ENTRY();

    if (u32EngId    >= TSP_ENGINE_NUM)
    {
        _TSP_RETURN(DRVTSP_FAIL);
    }

    *pu32Pcr = MHal_TSP_GetPcr(u32EngId);
    *pu32Pcr_32 = MHal_TSP_GetPcr_32(u32EngId);

    _TSP_RETURN(DRVTSP_OK);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

module_init(mod_tsp_init);
module_exit(mod_tsp_exit)

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("TSP driver");
MODULE_LICENSE("MSTAR");
