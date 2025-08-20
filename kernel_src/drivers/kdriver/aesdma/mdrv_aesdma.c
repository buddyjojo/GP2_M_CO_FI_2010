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
//
// file    drvAESDMA.c
// @brief  AESDMA Driver
// @author MStar Semiconductor,Inc.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//#define MSOS_TYPE_LINUX
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "chip_int.h"
#include "mhal_chiptop_reg.h"
#include "mdrv_aesdma.h"
#include "mhal_aesdma.h"
#include "Board.h"

//--------------------------------------------------------------------------------------------------
//  Driver Compiler Option
//--------------------------------------------------------------------------------------------------
#define AESDMAIntEnable()                       enable_irq(E_IRQ_AESDMA)
#define AESDMAIntDisable()                      disable_irq(E_IRQ_AESDMA)
#define AESDMAPA2KSEG1(addr) 	                ((void *)(((U32)addr) | 0xa0000000)) //physical -> unchched

//--------------------------------------------------------------------------------------------------
//  AESDMA HAL function
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Local Defines
//--------------------------------------------------------------------------------------------------
#define AESDMA_TASK_STACK_SIZE 4096

//--------------------------------------------------------------------------------------------------
//  Global Variable
//--------------------------------------------------------------------------------------------------
static DrvAESDMA_Event                  _AESDMAEvtNotify;
static P_DrvAESDMA_EvtCallback          _AESDMACallback;
static U32                              _u32MIU0_Addr, _u32MIU1_Addr , _u32MIU_num;
static U32                              u32AESDMAStatus;

//--------------------------------------------------------------------------------------------------
//  Internal Variable
//--------------------------------------------------------------------------------------------------
///////////AESDMA Driver//////////////

DRVAESDMA_RESULT MDrv_AESDMA_Random(AESDMA_RANDOM_t *aesdma_random)
{
    aesdma_random->u16RandomVar = MHal_AESDMA_Random();
    return aesdma_random->u32Result;
}

DRVAESDMA_RESULT MDrv_AESDMA_GetStatus(void)
{
    return MHal_AESDMA_Get_AESDMA_Status();
}

DRVAESDMA_RESULT MDrv_AESDMA_Start(BOOL bStart)
{
    MHal_AESDMA_Start(bStart);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_Reset(void)
{
    MHal_AESDMA_Reset();
    AESDMAIntDisable();
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_GetPSMatchedByteCNT(void)
{
    return MHal_AESDMA_Get_PS_MatchedBytecnt();
}

DRVAESDMA_RESULT MDrv_AESDMA_GetPSMatchedPTN(void)
{
    return MHal_AESDMA_Get_PS_MatchedPTN();
}

static BOOL MDrv_AESDMA_Chk_MIU(U32 u32InAddr, U32 u32InLen, U32 u32OutSAddr, U32 u32OutEAddr)
{
    u32InAddr = (U32)AESDMAPA2KSEG1(u32InAddr);
    u32OutSAddr = (U32)AESDMAPA2KSEG1(u32OutSAddr);
    u32OutEAddr = (U32)AESDMAPA2KSEG1(u32OutEAddr);

    if (_u32MIU_num == 2)
    {
        if ((_u32MIU1_Addr > u32InAddr) & (_u32MIU1_Addr > u32InAddr+u32InLen) & (_u32MIU1_Addr > u32OutSAddr) & (_u32MIU1_Addr > u32OutEAddr))
        {
            MHal_AESDMA_Set_MIU_Path(FALSE,FALSE); // miu0->miu0
             return TRUE;
        }

        if ((_u32MIU1_Addr < u32InAddr) & (_u32MIU1_Addr < u32InAddr+u32InLen) & (_u32MIU1_Addr < u32OutSAddr) & (_u32MIU1_Addr < u32OutEAddr))
        {
            MHal_AESDMA_Set_MIU_Path(TRUE,TRUE); // miu1->miu1
            return TRUE;
        }

        if ((_u32MIU1_Addr > u32InAddr) & (_u32MIU1_Addr > u32InAddr+u32InLen) & (_u32MIU1_Addr <= u32OutSAddr) & (_u32MIU1_Addr <= u32OutEAddr))
        {
            MHal_AESDMA_Set_MIU_Path(FALSE,TRUE); // miu0->miu1
            return TRUE;
        }

        if ((_u32MIU1_Addr <= u32InAddr) & (_u32MIU1_Addr <= u32InAddr+u32InLen) & (_u32MIU1_Addr > u32OutSAddr) & (_u32MIU1_Addr > u32OutEAddr))
        {
            MHal_AESDMA_Set_MIU_Path(TRUE,FALSE); // miu1->miu0
            return TRUE;
        }
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void MDrv_AESDMA_Isr_Proc(unsigned long unused)
{
    U32              u32Events;

    u32Events = u32AESDMAStatus;

    if (u32Events & AESDMA_STATES_GROUP)
    {
        if (AESDMA_HAS_FLAG(u32Events, AESDMA_STATES_GROUP) && _AESDMAEvtNotify && _AESDMACallback)
        {
            switch (u32Events & AESDMA_STATES_GROUP)
            {
                case AESDMA_PS_DONE:
                     if (AESDMA_HAS_FLAG(_AESDMAEvtNotify, E_DRVAESDMA_EVENT_PS_DONE))
                     {
                         _AESDMACallback(E_DRVAESDMA_EVENT_PS_DONE);
                     }
                     break;

                case AESDMA_PS_STOP:
                     if (AESDMA_HAS_FLAG(_AESDMAEvtNotify, E_DRVAESDMA_EVENT_PS_STOP))
                     {
                         _AESDMACallback(E_DRVAESDMA_EVENT_PS_STOP);
                     }
                     break;

                case AESDMA_DMA_DONE:
                     if (AESDMA_HAS_FLAG(_AESDMAEvtNotify, E_DRVAESDMA_EVENT_DMA_DONE))
                     {
                         _AESDMACallback(E_DRVAESDMA_EVENT_DMA_DONE);
                     }
                     break;

                case AESDMA_DMA_PAUSE:
                     if (AESDMA_HAS_FLAG(_AESDMAEvtNotify, E_DRVAESDMA_EVENT_DMA_PAUSE))
                     {
                         _AESDMACallback(E_DRVAESDMA_EVENT_DMA_PAUSE);
                     }
                     break;
            }
        }
    }
}

DECLARE_TASKLET(aesdma_isr_tasklet, MDrv_AESDMA_Isr_Proc, 0);

irqreturn_t MDrv_AESDMA_Isr(int irq, void *dev_id)
{
    u32AESDMAStatus = MDrv_AESDMA_GetStatus();
    tasklet_schedule(&aesdma_isr_tasklet);
    return IRQ_HANDLED;
}

//--------------------------------------------------------------------------------------------------

// Global Function

//--------------------------------------------------------------------------------------------------
static unsigned int u32AESDMAIntRegister;

DRVAESDMA_RESULT MDrv_AESDMA_Init(AESDMA_INIT_t *AesdmaInit)
{
    _u32MIU_num = AesdmaInit->u32miunum;
    _u32MIU0_Addr = (U32)AESDMAPA2KSEG1(AesdmaInit->u32miu0addr);
    if (_u32MIU_num == 2)
    {
        _u32MIU1_Addr = (U32)AESDMAPA2KSEG1(AesdmaInit->u32miu1addr);
    }

    MHal_AESDMA_Reset();

    _AESDMAEvtNotify = E_DRVAESDMA_EVENT_DATA_INIT;
    _AESDMACallback = NULL;

	
    if(0 == u32AESDMAIntRegister) {

    if ( request_irq(E_IRQ_AESDMA, MDrv_AESDMA_Isr, SA_INTERRUPT, "AESDMAInt", NULL))
    {
        return -EBUSY;
    }

    	u32AESDMAIntRegister = 1;
	} else {
		disable_irq(E_IRQ_AESDMA);
    	enable_irq(E_IRQ_AESDMA);
    }

    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_SelEng(AESDMA_SELENG_t *aesdma_seleng)
{
    if (aesdma_seleng->cipherkey != NULL)
    {
        MHal_AESDMA_Set_CipherKey(aesdma_seleng->cipherkey);
        MHal_AESDMA_Sel_Key(FALSE); // key from cipherkey
    }
    else
    {
        MHal_AESDMA_Sel_Key(TRUE); // key from KL
    }

    MHal_AESDMA_Set_PVR_ENG(aesdma_seleng->u32Eng, aesdma_seleng->bDescrypt, aesdma_seleng->bMode);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_SetPS(AESDMA_SETPS_t *aesdma_setps)
{
    MHal_AESDMA_Set_PS_PTN(aesdma_setps->u32PTN);
    MHal_AESDMA_Set_PS_Mask(aesdma_setps->u32Mask);
    MHal_AESDMA_Set_PS_ENG(aesdma_setps->bPSin_Enable, aesdma_setps->bPSout_Enable);
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_SetFileInOut(AESDMA_SETFILEINOUT_t *aesdma_SetFileInout)
{
    // check miu addr
    if (!MDrv_AESDMA_Chk_MIU(aesdma_SetFileInout->u32FileinAddr, aesdma_SetFileInout->u32FileInNum, aesdma_SetFileInout->u32FileOutSAddr, aesdma_SetFileInout->u32FileOutEAddr))
    {
        return DRVAESDMA_MIU_ADDR_ERROR;
    }

    if ((aesdma_SetFileInout->u32FileOutSAddr == 0) | (aesdma_SetFileInout->u32FileOutEAddr == 0))
    {
        MHal_AESDMA_Set_FileinDesc(aesdma_SetFileInout->u32FileinAddr , aesdma_SetFileInout->u32FileInNum);
    }
    else
    {
        MHal_AESDMA_Set_FileinDesc(aesdma_SetFileInout->u32FileinAddr, aesdma_SetFileInout->u32FileInNum);
        MHal_AESDMA_Set_FileoutDesc(aesdma_SetFileInout->u32FileOutSAddr, aesdma_SetFileInout->u32FileOutEAddr);
    }
    return DRVAESDMA_OK;
}

DRVAESDMA_RESULT MDrv_AESDMA_Notify(AESDMA_NOTIFY_t *aesdma_notify)
{
    if (aesdma_notify->pfCallback)
    {
        MHal_AESDMA_Enable_Int();
    }
    else
    {
        MHal_AESDMA_Disable_Int();
    }

    _AESDMAEvtNotify = aesdma_notify->eEvents;
    _AESDMACallback = aesdma_notify->pfCallback;
    return DRVAESDMA_OK;
}



