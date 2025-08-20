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
/// @file   mhal_h264_interrupt.c
/// @brief  h264 HAL layer for handling interrupt
/// @author MStar Semiconductor,Inc.
/// @attention
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/wait.h>
#include <linux/poll.h>

#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include "Board.h"
#include "mhal_h264.h"
//#include "mdrv_h264.h"
#include "drvHVD_Common.h"
#include "drvHVD.h"


#if HVD_ENABLE_ISR_POLL
extern U32 gu32H264NumOfDataLast;
extern wait_queue_head_t       gHVD_wait_queue;
#endif

extern BOOL gbRegHVDIRQ;
extern U32 *pH264DataMemSerNumb;
extern U8* pH264DataMem;
extern U32 gH264InterruptFlag;
extern BOOL gbGetGirstFrame;
extern BOOL gbIsAVSync;
extern BOOL gbIsAVSyncDone;
extern U8 gu8VideoSkip;
extern U8 gu8VideoRepeat;
extern BOOL bgDisplayReady;
extern U32 gu32H264NumOfData;
extern U32 gu32H264FrameCnt;
extern U32 gu32HVDDecErrCnt;
extern U32 gu32HVDDataErrCnt;
extern HVD_Disp_Info LastDispInfo;//H.264 update 090812
extern BOOL _bH264_State_Init ;
extern U32 u32ISR_decFramecnt;
extern U32 u32ISR_SPSReportCnt;
extern U32 LastDecCnt;
extern U32 u32Restart_WaitSyncReach;

#define _HVD_PRINT_ERR(format, args...)  printk( format, ##args)
#define _HVD_PRINT_INFO(format, args...)  //printk( format, ##args)
#define _HVD_PRINT_DBG(format, args...)  //printk( format, ##args)

#define HVD_ENABLE_ISR_MONITOR  0
#define HVD_ENABLE_MEASURE_AV_SYNC  0
#define HVD_ENABLE_ISR_REP_STATUS   1
#define HVD_ENABLE_ISR_REP_SPS   1
#define HVD_ENABLE_ISR_REP_PIC   1
#define HVD_ENABLE_ISR_REP_USER_DATA   0

extern HVD_Result MHal_H264_GetDispInfo( HVD_Disp_Info *pinfo );
extern void HVD_Dump_DispFrmInfo(void);
extern void HAL_HVD_Dump_FW_Status(void);

U32 MHal_H264_GetUnProcessedEntryNunb( void )
{
    if(  pH264DataMemSerNumb  )
    {
        U32 u32KerWSN = 0;
        U32 u32AppRSN = 0;
        u32KerWSN = (U32)(*pH264DataMemSerNumb);
        u32AppRSN = (U32)(*(pH264DataMemSerNumb+1));
        //printk( "MHal_H264_GetUnProcessedEntryNunb: %d %d\n"  ,   u32KerWSN   , u32AppRSN   );
        if( _bH264_State_Init && (u32KerWSN > u32AppRSN ||  (  (u32KerWSN != 0xFFFFFFFF) &&  (u32AppRSN != 0xFFFFFFFF) )) )
        {
            return u32KerWSN-u32AppRSN;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

//irqreturn_t MHal_H264_interrupt(int irq,void *devid, struct pt_regs *regs)
//void _hvd_interrupt(void)
static void _hvd_interrupt(unsigned long unused)
{
    U32 u32H264IntStatus;
    U32 uSizeOfSequenceHeader = 0;
    U32 uSizeOfSpecificStatus = 0;
    U32 uSizeOfPicHeader = 0;
    //for handling sequence header callback message
    H264_SEQUENCE_HEADER_T H264_SEQUENCE_HEADER_Tis;
    H264_PIC_HEADER_T	 	H264_PIC_HEADER_Tis;
    HVD_Disp_Info       H264_FRAMEINFO_Tis;
    H264_SPECIFIC_STATUS_T H264_SPECIFIC_STATUS_Tis;
    pH264_SPECIFIC_STATUS_T pH264_SPECIFIC_STATUS_Tis = NULL;
    //pMVD_CallBackMsg pMVD_CallBackMsgIs = NULL;
    pH264_Data_Pkt pH264_Data_PktIs = NULL;
//    pH264_Data_Pkt pMVD_Data_PktFirstIs = NULL;
    static U8 *pStrLast = NULL;
    U8 u8PreHVDSkipCnt=gu8VideoSkip;
    U32 u32PreHVDDecErrCnt=gu32HVDDecErrCnt;
    U32 u32PreHVDDataErrCnt=gu32HVDDataErrCnt;
    BOOL bIsDispInfoChange=FALSE;
    BOOL bIFrmFound=FALSE;
    U8 u8ESLevel=0;
    BOOL bIsRepSPSChange=FALSE;
    U32 framecnt=0;

    #if HVD_ENABLE_ISR_MONITOR
    U32 pictype=0;
    BOOL breportseqchg=FALSE;
    BOOL bDataErr=FALSE;
    BOOL bDecErr=FALSE;
    #endif

    if( (_bH264_State_Init == FALSE) || (pH264DataMem==NULL) || (pH264DataMemSerNumb==NULL) )
    {
        return ;
    }

    MDrv_HVD_GetISRInfo( (HVD_ISR_Event*)(&u32H264IntStatus) );

    if(gu32H264NumOfData % HVD_ISR_SHAREDATA_NUMB == 0)
    {
        pStrLast = pH264DataMem;
    }
    MHal_H264_GetNextFrame();

    _HVD_PRINT_DBG("ISR info: %x %x\n",gH264InterruptFlag,u32H264IntStatus  );
    if( MDrv_HVD_IsDispInfoChg() )
    {
        bIsRepSPSChange=TRUE;
    }
    if(E_HVD_OK != MHal_H264_GetDispInfo(&H264_FRAMEINFO_Tis))//H.264 update 090812
    {
        printk("MVD Fail to get sequence header [SEQ]\n");
        //goto JUMP_OUT_INTERRUPT_0020_HANDLE;
    }
    framecnt=(U32)MDrv_HVD_GetDecodeCnt();

#if HVD_ENABLE_DECODE_TIME_ISR
    if (u8PreHVDSkipCnt != MHal_H264_GetVideoSkip() )
    {
        gu8VideoSkip =MHal_H264_GetVideoSkip();
    }
    if( MDrv_HVD_CheckDispInfoRdy() == E_HVD_OK )
    {
        bgDisplayReady = TRUE;
    }
    if (u32PreHVDDecErrCnt != MDrv_HVD_GetDecErrCnt() )
    {
        gu32HVDDecErrCnt =MDrv_HVD_GetDecErrCnt();
    }
    if (u32PreHVDDataErrCnt != MDrv_HVD_GetDataErrCnt() )
    {
        gu32HVDDataErrCnt =MDrv_HVD_GetDataErrCnt();
    }
    if( (LastDispInfo.u16HorSize != H264_FRAMEINFO_Tis.u16HorSize) ||
         (LastDispInfo.u16VerSize!= H264_FRAMEINFO_Tis.u16VerSize) ||
         (LastDispInfo.u8Interlace!= H264_FRAMEINFO_Tis.u8Interlace) ||
         (LastDispInfo.u8AspectRate!= H264_FRAMEINFO_Tis.u8AspectRate) ||
         (LastDispInfo.u32FrameRate!= H264_FRAMEINFO_Tis.u32FrameRate)  )
    {
        bIsDispInfoChange=TRUE;
    }
    // specially for weak signal. Decoder may get wrong SPS and report fake SPS change.
    if( bIsRepSPSChange &&
        (LastDispInfo.u16HorSize == H264_FRAMEINFO_Tis.u16HorSize) &&
         (LastDispInfo.u16VerSize== H264_FRAMEINFO_Tis.u16VerSize) &&
         (LastDispInfo.u8Interlace== H264_FRAMEINFO_Tis.u8Interlace) &&
         (LastDispInfo.u32FrameRate== H264_FRAMEINFO_Tis.u32FrameRate)  )
    {
        MDrv_HVD_SetMVOPDone();
    }
    if( bIsRepSPSChange  )
    {
        // error handle for weak signal. ignore unworkable SPS showed up.
        //  1. 16K is MVOP suggestion min value.
        //  2. frame rate too low may lead to decoder dead lock.
        //  3. crop size should not over pixel size.
        //
        if(  ((unsigned long long)H264_FRAMEINFO_Tis.u16VerSize * H264_FRAMEINFO_Tis.u16HorSize * H264_FRAMEINFO_Tis.u32FrameRate
              < (unsigned long long)16000000 ) ||
              (H264_FRAMEINFO_Tis.u32FrameRate < 10000)  || (H264_FRAMEINFO_Tis.u32FrameRate > 170000)  ||
               ( H264_FRAMEINFO_Tis.u16CropRight >=H264_FRAMEINFO_Tis.u16HorSize) ||
              ( H264_FRAMEINFO_Tis.u16CropLeft >=H264_FRAMEINFO_Tis.u16HorSize) ||
              ( H264_FRAMEINFO_Tis.u16CropBottom >=H264_FRAMEINFO_Tis.u16VerSize) ||
              ( H264_FRAMEINFO_Tis.u16CropTop >=H264_FRAMEINFO_Tis.u16VerSize)
                 )
        {
            MDrv_HVD_SetMVOPDone();
            bIsDispInfoChange=FALSE;
        }
    }
    if( u32Restart_WaitSyncReach )
    {
        if ( (MDrv_HVD_IsSyncReach() == TRUE ) ||
           ( ( MDrv_HVD_GetPlayMode(E_HVD_GMODE_IS_SYNC_ON) == FALSE ) &&  (MDrv_HVD_Is1stFrmRdy() ) ) ||
           ( (framecnt > 100) &&  (MDrv_HVD_IsSyncStart()  == FALSE) && ( MDrv_HVD_IsSyncReach() == FALSE  )  &&  (MDrv_HVD_Is1stFrmRdy() ) )
        )
        {
            MDrv_HVD_SetMVOPDone();
            u32Restart_WaitSyncReach=FALSE;
        }
    }
    if( MDrv_HVD_IsIFrmFound() )
    {
        bIFrmFound=TRUE;
    }
    // error handler for share memory overflow issue that madp callback function not serviced by OS too long.
    if(  (MHal_H264_GetUnProcessedEntryNunb()  > 80) || ( gu32H264NumOfData  - u32ISR_SPSReportCnt > 20 ) )
    {
        bIsDispInfoChange=TRUE;
        u32ISR_SPSReportCnt = gu32H264NumOfData;
    }
    u8ESLevel = MDrv_HVD_GetData( E_HVD_GDATA_TYPE_ES_LEVEL );
    #if HVD_ENABLE_ISR_REP_STATUS
    if(    (gH264InterruptFlag&EN_SPECIFIC_STATUS) &&(
            (  bIsDispInfoChange == TRUE ) ||
            (gu32HVDDecErrCnt != u32PreHVDDecErrCnt) ||  //picture decode error while
            (gu32HVDDataErrCnt != u32PreHVDDataErrCnt) ||  //picture decode error while
            ( bIFrmFound == TRUE ) ||
            (bgDisplayReady == TRUE )  ||
            ( u8ESLevel != 0 )    )  )
    {
        if((NULL != pH264DataMem) && (NULL != pStrLast))
        {
            uSizeOfSpecificStatus =  sizeof(H264_SPECIFIC_STATUS_T);
            memset(&H264_SPECIFIC_STATUS_Tis, 0, uSizeOfSpecificStatus);
            pH264_SPECIFIC_STATUS_Tis = &H264_SPECIFIC_STATUS_Tis;
            //printk(">>>INT 0x%X\n",u16H264IntStatus);
            //printk("\n");
            if( gu32HVDDataErrCnt != (u32PreHVDDataErrCnt))    //Data error
            {
                pH264_SPECIFIC_STATUS_Tis->bDataError = 1;
                //printk("dataErr ");
                #if HVD_ENABLE_ISR_MONITOR
                bDataErr=TRUE;
                #endif
            }
            if(gu32HVDDecErrCnt != (u32PreHVDDecErrCnt))    //picture decoding error
            {
                pH264_SPECIFIC_STATUS_Tis->bPictureDecodingError = 1;
                //printk("DecErr ");
                #if HVD_ENABLE_ISR_MONITOR
                bDecErr=TRUE;
                #endif
            }

            if( u8ESLevel == E_HVD_ES_LEVEL_UNDER )
            {
                pH264_SPECIFIC_STATUS_Tis->bBitStreamBufferUnderflow = 1;
            }
            else if( u8ESLevel == E_HVD_ES_LEVEL_OVER )
            {
                pH264_SPECIFIC_STATUS_Tis->bBitStreamBufferOverflow = 1;
            }

            if(bIFrmFound)    //first frame decoded
            {
                pH264_SPECIFIC_STATUS_Tis->bGetFirstFrame = 1;
                //gbGetGirstFrame=TRUE;
                //printk("First Frame ");
            }
            if(bgDisplayReady)    //MVD ready to display
            {
                pH264_SPECIFIC_STATUS_Tis->bDisplayReady = 1;
                //printk("Ready ");
            }
            if(bIsDispInfoChange)    //sequence header detected
            {
                pH264_SPECIFIC_STATUS_Tis->bSequenceHeaderDetected = 1;
                //printk("Sequence ");
            }
            //printk("\n");
			pH264_Data_PktIs = (pH264_Data_Pkt)pStrLast;
			pH264_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pH264_Data_PktIs->u32TypeOfStruct  = 5;               // 5 for specific status
            // copy data into &pH264_Data_PktIs->pPktData;
            memcpy(&pH264_Data_PktIs->pPktData, pH264_SPECIFIC_STATUS_Tis, uSizeOfSpecificStatus);
            //printk("HVD irq st: id:%u addr:%x\n",gu32H264NumOfData , (U32)(pStrLast));
            pH264_Data_PktIs->u32NumOfDataPkt  = gu32H264NumOfData++;
            *pH264DataMemSerNumb = gu32H264NumOfData;
            if(gu32H264NumOfData % HVD_ISR_SHAREDATA_NUMB == 0)
            	pStrLast = pH264DataMem;
            else
            	pStrLast += PKT_SIZE;
        }
        else
        {
            printk("MVD interrupt Fail to write MMAP [SPECIFIC]\n");
        }

    }
    #endif

    #if HVD_ENABLE_ISR_REP_SPS
    // interrupt for sequence header  - INT_STAT bit "5" : 0x20 -> 0010 0000
    if(bgDisplayReady && (bIsDispInfoChange) && (gH264InterruptFlag&EN_SEQ_HDR_DATA))
    {
        //printk("MVD interrup: sequence hdr 5:%d 6:%d\n",u32INT5Count,u32INT6Count);
        if((NULL != pH264DataMem) && (NULL != pStrLast))
        {
            uSizeOfSequenceHeader =  sizeof(H264_SEQUENCE_HEADER_T);
            memset(&H264_SEQUENCE_HEADER_Tis, 0, uSizeOfSequenceHeader);
            memset(&H264_FRAMEINFO_Tis, 0, sizeof(H264_FRAMEINFO_Tis));
            if(E_HVD_OK != MHal_H264_GetDispInfo(&H264_FRAMEINFO_Tis))
            {
                printk("MVD Fail to get sequence header [SEQ]\n");
                goto JUMP_OUT_INTERRUPT_0020_HANDLE;
            }
            memcpy( &LastDispInfo  ,  &H264_FRAMEINFO_Tis  ,  sizeof( LastDispInfo ) );//H.264 update 090812
            #if HVD_ENABLE_ISR_MONITOR
            breportseqchg=TRUE;
            #endif
            H264_SEQUENCE_HEADER_Tis.u16HorSize = (U16)H264_FRAMEINFO_Tis.u16HorSize;
            H264_SEQUENCE_HEADER_Tis.u16VerSize = (U16)H264_FRAMEINFO_Tis.u16VerSize;
            H264_SEQUENCE_HEADER_Tis.u16FrameRate = (U16)H264_FRAMEINFO_Tis.u32FrameRate;
            H264_SEQUENCE_HEADER_Tis.u8AspectRatio = (U8)H264_FRAMEINFO_Tis.u8AspectRate;
            H264_SEQUENCE_HEADER_Tis.u8Progressive = (H264_FRAMEINFO_Tis.u8Interlace ? 0 : 1);
            //Next data structure
			pH264_Data_PktIs = (pH264_Data_Pkt)pStrLast;
			pH264_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pH264_Data_PktIs->u32TypeOfStruct = 3;               // 3 for MVD_SEQUENCE_HEADER_T
            memcpy(&pH264_Data_PktIs->pPktData, &H264_SEQUENCE_HEADER_Tis, uSizeOfSequenceHeader);
            //printk("HVD interrupt SEQ  addr:%x id:%u\n", (U32)pH264_Data_PktIs->pPktData ,  gu32H264NumOfData);

            pH264_Data_PktIs->u32NumOfDataPkt = gu32H264NumOfData++;
            *pH264DataMemSerNumb = gu32H264NumOfData;
            if(gu32H264NumOfData % HVD_ISR_SHAREDATA_NUMB == 0)
            	pStrLast = pH264DataMem;
            else
            	pStrLast += PKT_SIZE;
            //printk("MVD interrupt @_@ 3\n");

        }
        else
        {
            printk("H264 interrupt Fail to write MMAP [SEQ]\n");
        }

    }
    #endif

    // interrupt for picture header - INT_STAT bit "6" : 0x40 -> 0100 0000
    #if HVD_ENABLE_ISR_REP_PIC
    if (  (gH264InterruptFlag & EN_PIC_HDR_DATA) &&  (framecnt  !=LastDecCnt) )
    {
        LastDecCnt=framecnt;
        uSizeOfPicHeader =  sizeof(H264_PIC_HEADER_T);
        memset(&H264_PIC_HEADER_Tis, 0, uSizeOfPicHeader);
        H264_PIC_HEADER_Tis.u8PicType = MHal_H264_GetPictType();
        #if HVD_ENABLE_ISR_MONITOR
        pictype = H264_PIC_HEADER_Tis.u8PicType;
        #endif
        pH264_Data_PktIs = (pH264_Data_Pkt)pStrLast;
        pH264_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
        pH264_Data_PktIs->u32TypeOfStruct  = 4;               // 4 for pictuer header
        // copy data into &pH264_Data_PktIs->pPktData;
        memcpy(&pH264_Data_PktIs->pPktData, &H264_PIC_HEADER_Tis, uSizeOfPicHeader);
        pH264_Data_PktIs->u32NumOfDataPkt  = gu32H264NumOfData++;
        *pH264DataMemSerNumb = gu32H264NumOfData;
        if(gu32H264NumOfData % HVD_ISR_SHAREDATA_NUMB == 0)
        {
            pStrLast = pH264DataMem;
        }
        else
        {
            pStrLast += PKT_SIZE;
        }
    }
    #endif

#else //HVD_ENABLE_DECODE_TIME_ISR

    if(0 != (u32H264IntStatus & E_HVD_ISR_DISP_ONE))    //MVD ready to display
    {
        bgDisplayReady = TRUE;
    }
    if(0 != (u32H264IntStatus & E_HVD_ISR_DISP_REPEAT))    //repeat picture while AV un-sync
    {
        gu8VideoRepeat++;
    }
    if (u8PreHVDSkipCnt != MHal_H264_GetVideoSkip() )
    {
        gu8VideoSkip =MHal_H264_GetVideoSkip();
    }
    if (u32PreHVDDecErrCnt != MDrv_HVD_GetDecErrCnt() )
    {
        gu32HVDDecErrCnt =MDrv_HVD_GetDecErrCnt();
    }
    if (u32PreHVDDataErrCnt != MDrv_HVD_GetDataErrCnt() )
    {
        gu32HVDDataErrCnt =MDrv_HVD_GetDataErrCnt();
    }
    if( (LastDispInfo.u16HorSize != H264_FRAMEINFO_Tis.u16HorSize) ||
         (LastDispInfo.u16VerSize!= H264_FRAMEINFO_Tis.u16VerSize) ||
         (LastDispInfo.u8Interlace!= H264_FRAMEINFO_Tis.u8Interlace) ||
         (LastDispInfo.u8AspectRate!= H264_FRAMEINFO_Tis.u8AspectRate) ||
         (LastDispInfo.u32FrameRate!= H264_FRAMEINFO_Tis.u32FrameRate)  )
    {
        bIsDispInfoChange=TRUE;
    }
    // specially for weak signal. Decoder may get wrong SPS and report fake SPS change.
    if( bIsRepSPSChange &&
        (LastDispInfo.u16HorSize == H264_FRAMEINFO_Tis.u16HorSize) &&
         (LastDispInfo.u16VerSize== H264_FRAMEINFO_Tis.u16VerSize) &&
         (LastDispInfo.u8Interlace== H264_FRAMEINFO_Tis.u8Interlace) &&
         (LastDispInfo.u32FrameRate== H264_FRAMEINFO_Tis.u32FrameRate)  )
    {
        MDrv_HVD_SetMVOPDone();
    }
    if( MDrv_HVD_IsIFrmFound() )
    {
        bIFrmFound=TRUE;
    }
    #if HVD_ENABLE_ISR_REP_STATUS
    if(    (gH264InterruptFlag&EN_SPECIFIC_STATUS) &&(
            (  bIsDispInfoChange == TRUE ) ||
            (gu32HVDDecErrCnt != u32PreHVDDecErrCnt) ||  //picture decode error while
            (gu32HVDDataErrCnt != u32PreHVDDataErrCnt) ||  //picture decode error while
            ( bIFrmFound == TRUE ) ||
            (bgDisplayReady == FALSE && 0 != (u32H264IntStatus & E_HVD_ISR_DISP_ONE)) ||  //MVD ready to display
            (gu8VideoSkip != (u8PreHVDSkipCnt)) ||  //Skip picture while AV un-sync
            (0 != (u32H264IntStatus & E_HVD_ISR_DISP_REPEAT))))     //repeat picture while AV un-sync
    {
        if((NULL != pH264DataMem) && (NULL != pStrLast))
        {
            uSizeOfSpecificStatus =  sizeof(H264_SPECIFIC_STATUS_T);
            memset(&H264_SPECIFIC_STATUS_Tis, 0, uSizeOfSpecificStatus);
            pH264_SPECIFIC_STATUS_Tis = &H264_SPECIFIC_STATUS_Tis;

            //printk(">>>INT 0x%X\n",u16H264IntStatus);
            //printk("\n");
            if( gu32HVDDataErrCnt != (u32PreHVDDataErrCnt))    //Data error
            {
                pH264_SPECIFIC_STATUS_Tis->bDataError = 1;
                //printk("dataErr ");
                #if HVD_ENABLE_ISR_MONITOR
                bDataErr=TRUE;
                #endif
            }

            if(gu32HVDDecErrCnt != (u32PreHVDDecErrCnt))    //picture decoding error
            {
                pH264_SPECIFIC_STATUS_Tis->bPictureDecodingError = 1;
                //printk("DecErr ");
                #if HVD_ENABLE_ISR_MONITOR
                bDecErr=TRUE;
                #endif
            }
            if(bIFrmFound)    //first frame decoded
            {
                pH264_SPECIFIC_STATUS_Tis->bGetFirstFrame = 1;
                //gbGetGirstFrame=TRUE;
                //printk("First Frame ");
            }

            if(0 != (u32H264IntStatus & E_HVD_ISR_DISP_ONE))    //MVD ready to display
            {
                pH264_SPECIFIC_STATUS_Tis->bDisplayReady = 1;
                //bgDisplayReady = TRUE;
                //printk("Ready ");
            }

            if(bIsDispInfoChange)    //sequence header detected
            {
                pH264_SPECIFIC_STATUS_Tis->bSequenceHeaderDetected = 1;
                //printk("Sequence ");
            }

            if(gu8VideoSkip != (u8PreHVDSkipCnt))    //Skip picture while AV un-sync
            {
                pH264_SPECIFIC_STATUS_Tis->bVideoSkip = 1;
                //gu8VideoSkip++;
                //printk("skip ");
            }

            if(0 != (u32H264IntStatus & E_HVD_ISR_DISP_REPEAT))    //repeat picture while AV un-sync
            {
                pH264_SPECIFIC_STATUS_Tis->bVideoRepeat = 1;
                //gu8VideoRepeat++;
                //printk("Repeat ");
            }
            //printk("\n");
			pH264_Data_PktIs = (pH264_Data_Pkt)pStrLast;
			pH264_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pH264_Data_PktIs->u32TypeOfStruct  = 5;               // 5 for specific status
            // copy data into &pH264_Data_PktIs->pPktData;
            memcpy(&pH264_Data_PktIs->pPktData, pH264_SPECIFIC_STATUS_Tis, uSizeOfSpecificStatus);
            //printk("HVD irq st: id:%u addr:%x\n",gu32H264NumOfData , (U32)(pStrLast));
            pH264_Data_PktIs->u32NumOfDataPkt  = gu32H264NumOfData++;
            *pH264DataMemSerNumb = gu32H264NumOfData;
            if(gu32H264NumOfData % HVD_ISR_SHAREDATA_NUMB == 0)
            	pStrLast = pH264DataMem;
            else
            	pStrLast += PKT_SIZE;
        }
        else
        {
            printk("MVD interrupt Fail to write MMAP [SPECIFIC]\n");
        }

    }
    #endif

    #if HVD_ENABLE_ISR_REP_SPS
    // interrupt for sequence header  - INT_STAT bit "5" : 0x20 -> 0010 0000
    if(bgDisplayReady && (bIsDispInfoChange) && (gH264InterruptFlag&EN_SEQ_HDR_DATA))
    {
        //printk("MVD interrup: sequence hdr 5:%d 6:%d\n",u32INT5Count,u32INT6Count);
        if((NULL != pH264DataMem) && (NULL != pStrLast))
        {
            uSizeOfSequenceHeader =  sizeof(H264_SEQUENCE_HEADER_T);
            memset(&H264_SEQUENCE_HEADER_Tis, 0, uSizeOfSequenceHeader);
            memset(&H264_FRAMEINFO_Tis, 0, sizeof(H264_FRAMEINFO_Tis));
            if(E_HVD_OK != MHal_H264_GetDispInfo(&H264_FRAMEINFO_Tis))
            {
                printk("MVD Fail to get sequence header [SEQ]\n");
                goto JUMP_OUT_INTERRUPT_0020_HANDLE;
            }
            memcpy( &LastDispInfo  ,  &H264_FRAMEINFO_Tis  ,  sizeof( LastDispInfo ) );//H.264 update 090812
            #if HVD_ENABLE_ISR_MONITOR
            breportseqchg=TRUE;
            #endif
            H264_SEQUENCE_HEADER_Tis.u16HorSize = (U16)H264_FRAMEINFO_Tis.u16HorSize;
            H264_SEQUENCE_HEADER_Tis.u16VerSize = (U16)H264_FRAMEINFO_Tis.u16VerSize;
            H264_SEQUENCE_HEADER_Tis.u16FrameRate = (U16)H264_FRAMEINFO_Tis.u32FrameRate;
            H264_SEQUENCE_HEADER_Tis.u8AspectRatio = (U8)H264_FRAMEINFO_Tis.u8AFD;
            H264_SEQUENCE_HEADER_Tis.u8Progressive = (H264_FRAMEINFO_Tis.u8Interlace ? 0 : 1);
            //Next data structure
			pH264_Data_PktIs = (pH264_Data_Pkt)pStrLast;
			pH264_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
            pH264_Data_PktIs->u32TypeOfStruct = 3;               // 3 for MVD_SEQUENCE_HEADER_T
            memcpy(&pH264_Data_PktIs->pPktData, &H264_SEQUENCE_HEADER_Tis, uSizeOfSequenceHeader);
            //printk("HVD interrupt SEQ  addr:%x id:%u\n", (U32)pH264_Data_PktIs->pPktData ,  gu32H264NumOfData);

            pH264_Data_PktIs->u32NumOfDataPkt = gu32H264NumOfData++;
            *pH264DataMemSerNumb = gu32H264NumOfData;
            if(gu32H264NumOfData % HVD_ISR_SHAREDATA_NUMB == 0)
            	pStrLast = pH264DataMem;
            else
            	pStrLast += PKT_SIZE;
            //printk("MVD interrupt @_@ 3\n");

        }
        else
        {
            printk("H264 interrupt Fail to write MMAP [SEQ]\n");
        }

    }
    #endif

    // interrupt for picture header - INT_STAT bit "6" : 0x40 -> 0100 0000
    #if HVD_ENABLE_ISR_REP_PIC
    if (  (gH264InterruptFlag & EN_PIC_HDR_DATA) &&  (framecnt  !=LastDecCnt) )
    {
        LastDecCnt=framecnt;
        uSizeOfPicHeader =  sizeof(H264_PIC_HEADER_T);
        memset(&H264_PIC_HEADER_Tis, 0, uSizeOfPicHeader);
        H264_PIC_HEADER_Tis.u8PicType = MHal_H264_GetPictType();
        #if HVD_ENABLE_ISR_MONITOR
        pictype = H264_PIC_HEADER_Tis.u8PicType;
        #endif
        pH264_Data_PktIs = (pH264_Data_Pkt)pStrLast;
        pH264_Data_PktIs->u32SizeOfDataPkt = PKT_SIZE;
        pH264_Data_PktIs->u32TypeOfStruct  = 4;               // 4 for pictuer header
        // copy data into &pH264_Data_PktIs->pPktData;
        memcpy(&pH264_Data_PktIs->pPktData, &H264_PIC_HEADER_Tis, uSizeOfPicHeader);
        pH264_Data_PktIs->u32NumOfDataPkt  = gu32H264NumOfData++;
        *pH264DataMemSerNumb = gu32H264NumOfData;
        if(gu32H264NumOfData % HVD_ISR_SHAREDATA_NUMB == 0)
        {
            pStrLast = pH264DataMem;
        }
        else
        {
            pStrLast += PKT_SIZE;
        }
    }
    #endif

#endif //HVD_ENABLE_DECODE_TIME_ISR

    #if HVD_ENABLE_ISR_REP_USER_DATA
    if(gH264InterruptFlag&EN_PIC_USER_DATA)
    {
        if( MHal_H264_CheckUserDataAvailable() )
        {
            if(H264STATUS_FAIL == MHal_H264_GetUserData((pH264_Data_Pkt *)&pStrLast, &gu32H264NumOfData))
            {
                //printk("H264 interrupt Fail to write User Data\n");
                goto JUMP_OUT_INTERRUPT_0020_HANDLE;
            }
        }
    }
    #endif

    #if HVD_ENABLE_MEASURE_AV_SYNC
    {
        U32 u32STC=0, u32PTS =0;
        S16 s16ATS=0,s16ATS2=0 ;
extern U32 MDrv_MAD_Read_DSP_sram(U16 u16Dsp_addr,BOOL dsp_memory_type);

        u32STC=MDrv_HVD_GetData( E_HVD_GDATA_TYPE_DISP_STC );
        u32PTS=MDrv_HVD_GetPTS();

        s16ATS=(S16)(MDrv_MAD_Read_DSP_sram(0x3495, 1)&0x0000FFFF);
        s16ATS2=(S16)(MDrv_MAD_Read_DSP_sram(0x34A8, 1)&0x0000FFFF);

        printk( "HVD: %d SP:%d SA:%d %d\n" , (S32)MDrv_HVD_GetData(E_HVD_GDATA_TYPE_DROP_CNT),  (S32)(u32STC-u32PTS)  ,(S32)s16ATS , (S32)s16ATS2);
    }
    #endif

    #if HVD_ENABLE_ISR_MONITOR
    if( (framecnt > (u32ISR_decFramecnt + 100))   )//|| breportseqchg  || bDataErr || bDecErr  )
    {
        printk("HVD isr: %u %u %x (%d) %d %d(%u %u %u) (%u %u %u)\n" ,gu32H264NumOfData  , framecnt , gH264InterruptFlag , breportseqchg ,
            pictype  , u8ESLevel ,
            (U32)MDrv_HVD_IsIFrmFound(),  (U32)MDrv_HVD_IsSyncStart(), (U32)MDrv_HVD_IsSyncReach() ,
            (U32)MDrv_HVD_GetDataErrCnt()    ,  (U32)MDrv_HVD_GetDecErrCnt()  , (U32)MDrv_HVD_GetData( E_HVD_GDATA_TYPE_SKIP_CNT )  );
        //HVD_Dump_DispFrmInfo();
        //HAL_HVD_Dump_FW_Status();
        u32ISR_decFramecnt=framecnt;
    }
    #endif

JUMP_OUT_INTERRUPT_0020_HANDLE:

#if HVD_ENABLE_ISR_POLL
    if( gu32H264NumOfData != gu32H264NumOfDataLast )
    {
        wake_up(&gHVD_wait_queue);
        gu32H264NumOfDataLast = gu32H264NumOfData;
        //printk(".");
    }
#endif
    return;
}

DECLARE_TASKLET(HVDTasklet, _hvd_interrupt , 0);


void MHal_H264_Isr(void)
{
    tasklet_schedule(&HVDTasklet);
    return;
}

BOOL MHal_H264_IsUnProcessedEntry( void )
{
    if(  pH264DataMemSerNumb  )
    {
        U32 u32KerWSN = 0;
        U32 u32AppRSN = 0;
        u32KerWSN = (U32)(*pH264DataMemSerNumb);
        u32AppRSN = (U32)(*(pH264DataMemSerNumb+1));
        //printk( "MHal_H264_IsUnProcessedEntry: %d %d\n"  ,   u32KerWSN   , u32AppRSN   );
        if( (_bH264_State_Init && (u32KerWSN > u32AppRSN ||  (  (u32KerWSN != 0xFFFFFFFF) &&  (u32AppRSN == 0xFFFFFFFF) )))
            || (gbRegHVDIRQ == FALSE) )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}


