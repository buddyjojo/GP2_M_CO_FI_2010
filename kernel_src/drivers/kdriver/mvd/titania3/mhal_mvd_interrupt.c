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
/// file    Mhal_mvd_interrupt.c
/// @brief  MVD HAL layer Driver for handling interrupt
/// @author MStar Semiconductor Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/wait.h>

#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include "Board.h"
#include "mhal_mvd.h"
#include "mdrv_mvd.h"
#include "mvd4_interface.h"

#define MVD_ISR_DBG     0

#if (MVD_ISR_DBG == 1)
#define MVD_ISR_MSG(...)    printk(__VA_ARGS__)
#else
#define MVD_ISR_MSG(...)
#endif

static void _mvd_interrupt(unsigned long unused);

extern U32 *pMVDDataMemSerNumb;
extern U8 *pDataMem;
extern U32 gInterruptFlag;      //refer declaration in mhal_mvd.c
extern BOOL _bMVD_State_Init;

/* Problem : UserData Missed After EMP & DTV Switch - 090219_andy */
/* for Driver to Initialize serial number of DataBuffer */
U32 u32NumOfData;

static void _mvd_interrupt(unsigned long unused)
{
    U16 u16InterruptState                           = 0;
    U32 uSizeOfSequenceHeader                       = 0;
    U32 uSizeOfSpecificStatus                       = 0;

    //for handling sequence header callback message
    MVD_SEQUENCE_HEADER_T MVD_SEQUENCE_HEADER_Tis;
    MVD_SPECIFIC_STATUS_T MVD_SPECIFIC_STATUS_Tis;
    pMVD_SPECIFIC_STATUS_T pMVD_SPECIFIC_STATUS_Tis = NULL;
    pMVD_Data_Pkt pMVD_Data_PktIs                   = NULL;
    static U32 u32INT5Count;//sequence header
    static U32 u32INT6Count;//picture header
    static U32 u32INT1Count;//picture user data
    static U32 u32AllCount;
    //static U32 u32NumOfData = 0;
    static U8 *pStrLast                             = NULL;

    if( (_bMVD_State_Init == FALSE)  || (pDataMem==NULL) || (pMVDDataMemSerNumb==NULL) )
    {
        return ;
    }

    //read interrupt stat
    u16InterruptState = MHal_MVD_ReadIRQ();
    u32AllCount++;

    if (u32NumOfData % QUEUE_SIZE == 0)
    {
        pStrLast = pDataMem;
        //printk("Init PKT Queue Roll Back!! Num 0x%x\n", u32NumOfData);
    }

    MVD_ISR_MSG("===== mvd_interrupt - u16InterruptState = 0x%x =====\n",
        u16InterruptState);

#if 1
    //for specific status callback
    if ((gInterruptFlag & EN_SPECIFIC_STATUS) &&
        ((0 != (u16InterruptState & INT_SEQ_FOUND)) ||      //sequence header detected
         (0 != (u16InterruptState & INT_VBUF_OVF)) ||       //bit-stream buffer overflow
         (0 != (u16InterruptState & INT_VBUF_UNF)) ||       //bit-stream buffer underflow
         (0 != (u16InterruptState & INT_VES_INVALID)) ||    //Data error
         (0 != (u16InterruptState & INT_DEC_ERR)) ||        //picture decoding error
         (0 != (u16InterruptState & INT_FIRST_FRAME)) ||    //first frame decoded
         (0 != (u16InterruptState & INT_DISP_RDY)) ||       //MVD ready to display
         (0 != (u16InterruptState & INT_SYN_SKIP)) ||       //Skip picture while AV un-sync
         (0 != (u16InterruptState & INT_SYN_REP)))          //repeat picture while AV un-sync
       )
    {
        if ((NULL != pDataMem) && (NULL != pStrLast))
        {
            uSizeOfSpecificStatus = sizeof(MVD_SPECIFIC_STATUS_T);

            memset(&MVD_SPECIFIC_STATUS_Tis, 0, uSizeOfSpecificStatus);

            pMVD_SPECIFIC_STATUS_Tis = &MVD_SPECIFIC_STATUS_Tis;

            if (0 != (u16InterruptState & INT_VES_INVALID))    //Data error
            {
                pMVD_SPECIFIC_STATUS_Tis->bDataError = 1;
            }

            if (0 != (u16InterruptState & INT_DEC_ERR))    //picture decoding error
            {
                pMVD_SPECIFIC_STATUS_Tis->bPictureDecodingError = 1;
            }

            if (0 != (u16InterruptState & INT_VBUF_OVF))    //bit-stream buffer overflow
            {
                pMVD_SPECIFIC_STATUS_Tis->bBitStreamBufferOverflow = 1;
            }

            if (0 != (u16InterruptState & INT_VBUF_UNF))    //bit-stream buffer underflow
            {
                pMVD_SPECIFIC_STATUS_Tis->bBitStreamBufferUnderflow = 1;
            }

            if (0 != (u16InterruptState & INT_FIRST_FRAME))    //first frame decoded
            {
                pMVD_SPECIFIC_STATUS_Tis->bGetFirstFrame = 1;
            }

            if (0 != (u16InterruptState & INT_DISP_RDY))    //MVD ready to display
            {
                pMVD_SPECIFIC_STATUS_Tis->bDisplayReady = 1;
            }

            if (0 != (u16InterruptState & INT_SEQ_FOUND))    //sequence header detected
            {
                pMVD_SPECIFIC_STATUS_Tis->bSequenceHeaderDetected = 1;
            }

            if (0 != (u16InterruptState & INT_SYN_SKIP))    //Skip picture while AV un-sync
            {
                pMVD_SPECIFIC_STATUS_Tis->bVideoSkip = 1;
            }

            if (0 != (u16InterruptState & INT_SYN_REP))    //repeat picture while AV un-sync
            {
                pMVD_SPECIFIC_STATUS_Tis->bVideoRepeat = 1;
            }

            pMVD_Data_PktIs = (pMVD_Data_Pkt) pStrLast;
            pMVD_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pMVD_Data_PktIs->u32TypeOfStruct = 5;               // 5 for specific status
            // copy data into &pMVD_Data_PktIs->pPktData;
            memcpy(&pMVD_Data_PktIs->pPktData,
                   pMVD_SPECIFIC_STATUS_Tis,
                   uSizeOfSpecificStatus);
			
            pMVD_Data_PktIs->u32NumOfDataPkt = u32NumOfData++;
            *pMVDDataMemSerNumb = u32NumOfData;
            if (u32NumOfData % QUEUE_SIZE == 0)
            {
                pStrLast = pDataMem;
                //printk("Type %d PKT Queue Roll Back!! Num 0x%x\n", pMVD_Data_PktIs->u32TypeOfStruct, u32NumOfData);
            }
            else
            {
                pStrLast += PKT_SIZE;
            }
        }
        else
        {
            printk("MVD interrupt Fail to write MMAP [SPECIFIC]\n");
        }
    }
#endif


#if 1
    // interrupt for sequence header  - INT_STAT bit "5" : 0x20 -> 0010 0000
    if ((0 != (u16InterruptState & INT_SEQ_FOUND)) &&
        (gInterruptFlag & EN_SEQ_HDR_DATA))
    {
        u32INT5Count++;

        MVD_ISR_MSG("MVD interrupt: sequence hdr 5:%d 6:%d\n",
            u32INT5Count, u32INT6Count);

        if ((NULL != pDataMem) && (NULL != pStrLast))
        {
            uSizeOfSequenceHeader = sizeof(MVD_SEQUENCE_HEADER_T);
            
            memset(&MVD_SEQUENCE_HEADER_Tis, 0, uSizeOfSequenceHeader);
            
            if (MVDSTATUS_FAIL ==
                MHal_MVD_GetSequenceHeader(&MVD_SEQUENCE_HEADER_Tis))
            {
                printk("MVD Fail to get sequence header [SEQ]\n");
                goto JUMP_OUT_INTERRUPT_0020_HANDLE;
            }

            pMVD_Data_PktIs = (pMVD_Data_Pkt) pStrLast;
            pMVD_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pMVD_Data_PktIs->u32TypeOfStruct = 3;               // 3 for MVD_SEQUENCE_HEADER_T

            memcpy(&pMVD_Data_PktIs->pPktData,
                   &MVD_SEQUENCE_HEADER_Tis,
                   uSizeOfSequenceHeader);
            
            //MVD_ISR_MSG("MVD interrupt @_@ 1 0x%x 0x%x\n",pStrTmp, &pMVD_Data_PktIs->pPktData);

            pMVD_Data_PktIs->u32NumOfDataPkt = u32NumOfData++;
            *pMVDDataMemSerNumb = u32NumOfData;
            if (u32NumOfData % QUEUE_SIZE == 0)
            {
                pStrLast = pDataMem;
                MVD_ISR_MSG("Type %d PKT Queue Roll Back!! Num 0x%x\n", pMVD_Data_PktIs->u32TypeOfStruct, u32NumOfData);
            }
            else
            {
                pStrLast += PKT_SIZE;
            }
            
            MVD_ISR_MSG("MVD interrupt @_@ 3\n");
        }
        else
        {
            printk("MVD interrupt Fail to write MMAP [SEQ]\n");
        }
    }
JUMP_OUT_INTERRUPT_0020_HANDLE:
#endif

#if 1
    if (0 != (u16InterruptState & INT_PIC_FOUND))
    {
    	BOOL bPassPicHeader = FALSE;
		BOOL bPassUserData = FALSE;
		
        if (gInterruptFlag & EN_PIC_HDR_DATA)
        {
            u32INT6Count ++;
			bPassPicHeader = TRUE;
        }

        if ((0 != (u16InterruptState & INT_USER_DATA)) &&
            (gInterruptFlag & EN_PIC_USER_DATA))
        {
            u32INT1Count++;
			bPassUserData = TRUE;
        }

        MVD_ISR_MSG("MVD interrupt %d, Sequence Header->%d; Picture Header->%d\n", u32AllCount,u32INT5Count,u32INT6Count);

		if(bPassUserData)
		{
	        //get data, and write into mmap buffer
	        if ((NULL != pDataMem) && (NULL != pStrLast))
	        {
	            if (MHal_MVD_GetPictureHeaderAndUserData((pMVD_Data_Pkt *) &pStrLast,
	                                                     &u32NumOfData,
	                                                     (gInterruptFlag & EN_PIC_HDR_DATA)) !=
	                MVDSTATUS_SUCCESS)
	            {
	                goto JUMP_OUT_INTERRUPT_0040_HANDLE;
	            }
	        }
	        else
	        {
	            printk("MVD interrupt Fail to write MMAP [PIC]\n");
	        }
		}
		else if(bPassPicHeader)
		{
            MVD_PIC_HEADER_T stMVD_PicHdr;

            MHal_MVD_GetPictureHeader(&stMVD_PicHdr);

            pMVD_Data_PktIs = (pMVD_Data_Pkt) pStrLast;
            pMVD_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pMVD_Data_PktIs->u32TypeOfStruct = 4;
            memcpy(&pMVD_Data_PktIs->pPktData,
                   &stMVD_PicHdr,
                   sizeof(MVD_PIC_HEADER_T));
            pMVD_Data_PktIs->u32NumOfDataPkt = u32NumOfData++;
            *pMVDDataMemSerNumb = u32NumOfData;
            if (u32NumOfData % QUEUE_SIZE == 0)
            {
                pStrLast = pDataMem;
            }
            else
            {
                pStrLast += PKT_SIZE;
            }
		}
	}
JUMP_OUT_INTERRUPT_0040_HANDLE:
#endif

#if 0
    // interrupt for picture header - INT_STAT bit "6" : 0x40 -> 0100 0000
    if (0 != (u16InterruptState & INT_PIC_FOUND))
    {
        if (gInterruptFlag & EN_PIC_HDR_DATA)
        {
            u32INT6Count ++;
        }

        if ((0 != (u16InterruptState & INT_USER_DATA)) &&
            (gInterruptFlag & EN_PIC_USER_DATA))
        {
            u32INT1Count++;
        }

        MVD_ISR_MSG("MVD interrupt %d, Sequence Header->%d; Picture Header->%d\n", u32AllCount,u32INT5Count,u32INT6Count);

        //get data, and write into mmap buffer
        if ((NULL != pDataMem) && (NULL != pStrLast))
        {
            if (MHal_MVD_GetPictureHeaderAndUserData((pMVD_Data_Pkt *) &pStrLast,
                                                     &u32NumOfData,
                                                     (gInterruptFlag & EN_PIC_HDR_DATA)) !=
                MVDSTATUS_SUCCESS)
            {
                goto JUMP_OUT_INTERRUPT_0040_HANDLE;
            }
        }
        else
        {
            printk("MVD interrupt Fail to write MMAP [PIC]\n");
        }
    }

    // if no userdata, then get picture header here
    if (0 == (u16InterruptState & INT_USER_DATA))
    {
        if (gInterruptFlag & EN_PIC_HDR_DATA)
        {
            MVD_PIC_HEADER_T stMVD_PicHdr;

            MHal_MVD_GetPictureHeader(&stMVD_PicHdr);

            pMVD_Data_PktIs = (pMVD_Data_Pkt) pStrLast;
            pMVD_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pMVD_Data_PktIs->u32TypeOfStruct = 4;
            memcpy(&pMVD_Data_PktIs->pPktData,
                   &stMVD_PicHdr,
                   sizeof(MVD_PIC_HEADER_T));
            pMVD_Data_PktIs->u32NumOfDataPkt = u32NumOfData++;
            *pMVDDataMemSerNumb = u32NumOfData;
            if (u32NumOfData % QUEUE_SIZE == 0)
            {
                pStrLast = pDataMem;
            }
            else
            {
                pStrLast += PKT_SIZE;
            }
        }
    }
#endif

    return;
}

DECLARE_TASKLET(MVDTasklet, _mvd_interrupt, 0);


irqreturn_t mvd_interrupt(int irq, void *devid)
{
    tasklet_schedule(&MVDTasklet);

    //clear interrupt
    MHal_MVD_ClearIRQ();
    //printk("received MVD interrupt \n");

    return IRQ_HANDLED;
}

