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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    Mdrv_mvd.c
/// @brief  MVD Driver
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


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
#include <linux/time.h>  //added
#include <linux/timer.h> //added
#include <asm/io.h>

#include "mst_devid.h"
#include "mdrv_mvd.h"
#include "mhal_mvd.h"
#include "mhal_mvd_interrupt.h"
//dhjung LGE
#include "Board.h"

#include "mdrv_system.h"

#include "mdrv_probe.h"

//--------------------------------------------------------------------------------------------------
// Constant definition
//--------------------------------------------------------------------------------------------------
#define MVD_WARNING(fmt, args...)       printk(KERN_WARNING "[MVDMOD][%06d]     " fmt, __LINE__, ## args)
//#define MVD_DEBUG_PRINT // LGE drmyung 080922
#ifdef MVD_DEBUG_PRINT
#define MVD_PRINT(fmt, args...)         printk("[MVDMOD][%06d]     " fmt, __LINE__, ## args)
#else
#define MVD_PRINT(fmt, args...)
#endif

//#define MOD_MVD_DEVICE_COUNT    1
//#define MOD_MVD_NAME            "ModMvd"

// buffer to load firmware
#if defined(CONFIG_Titania)
U8 *g_mvd_fw_buf                = NULL;
#define RISC_BOOT_CODE_SIZE     0x1800
#endif

struct MVD_Dev g_MVD_Dev;
MVD_BUFFER_INFO __stMVDBuffInfo;

struct file_operations MVD_Fops = {.owner = THIS_MODULE,
                                   .ioctl = MDrv_MVD_Ioctl,
                                   .open = MDrv_MVD_Open,
                                   .release = MDrv_MVD_Release,
//                                   .mmap = MDrv_MVD__mmap, //Removed by Tonio Liu 20090903
};

typedef enum _TYPE_State_Local_Play_Mode
{
    TSLPM_UNINIT = 0,
    TSLPM_INIT,
    TSLPM_SET_MODE,
    TSLPM_DOWNLOAD,
    TSLPM_DOWNLOAD_END,
    TSLPM_PLAY
}TYPE_State_File_Play_Mode;

U32 u32InterruptEnable;
static wait_queue_head_t wq_mvd;
atomic_t _aWaitQueueGuard                       = ATOMIC_INIT(0);

//U8 *pDataMMAP                                   = NULL; // Tonio Liu 20090903
U8 *pDataMem                                    = NULL;
U32 *pMVDDataMemSerNumb = NULL;

#define SIZE_OF_STREAM_DOWNLOAD_BUFFER  1048576         //1024*1024=1MB
#define LOCAL_PLAY_MODE_PAGE_NUM 256
#define LOCAL_BIT_STREAM_PAGE_NUM 512        //2MB
U8 *pTMPDataBitStreamBuff                       = NULL;
U8 *pTMPDataLocalPlayMode                       = NULL;
TYPE_State_File_Play_Mode StateOfLocalPlayMode  = TSLPM_UNINIT;

U32 gu32SizeOffSet                              = 0;

//U8 u8FilePlayMode = 0;
U32 u32MMAP_COPY_SIZE                           = 0;

extern  U32 u32NumOfData;

U32 MDrv_MVD_RegisterInterrupt(void)
{
    u32NumOfData = 0;

    if (0 == u32InterruptEnable)//need register
    {
        if (request_irq(E_IRQ_MVD, mvd_interrupt, SA_INTERRUPT, "mvd", NULL))
        {
            MVD_PRINT("[MVD] Fail to request IRQ %d\n", E_IRQ_MVD);
            return MVDSTATUS_FAIL;
        }

        MVD_PRINT("[MVD] request IRQ %d\n", (U32) E_IRQ_MVD);
        u32InterruptEnable = 1;
    }
    else
    {
		disable_irq(E_IRQ_MVD);
		enable_irq(E_IRQ_MVD);
    }

    return 0;
}

U32 MDrv_MVD_DeRegisterInterrupt(void)
{
    //if(1 == u32InterruptEnable){

    /* Problem : UserData Missed After EMP & DTV Switch - 090219_andy */
    /* Initialize serial number of DataBuffer */
    u32NumOfData = 0;

    MVD_PRINT("[MVD] dismiss IRQ %d\n", E_IRQ_MVD);
    free_irq(E_IRQ_MVD, NULL);
    u32InterruptEnable = 0;
    //}
    return 0;
}

U32 MDrv_MVD_CleanBitStreamBuff(void)
{
    U32 u32BufferStart  = 0;
    U32 u32BufferSize   = 0;
    U8 *pData           = NULL;
    MVD_PRINT("[MVD] Clean Bit-Stream Buff\n");
    MHal_MVD_GetBitStreamBuffer(&u32BufferStart, &u32BufferSize);

    u32BufferStart = (U32) pTMPDataBitStreamBuff;

    if ((0 >= u32BufferSize) || (0 == u32BufferStart))
    {
        MVD_PRINT("[MVD] Clean Bit-Stream Buff fail 0x%x, 0x%x\n",
                  u32BufferStart,
                  u32BufferSize);
        return -1;
    }
#if 0
   pData = (U8*)(u32BufferStart | 0xA0000000);
#else
pData = (U8*)pTMPDataBitStreamBuff;
#endif
    //MVD_PRINT("[MVD] Clean Bit-Stream Buff 0x%x, 0x%x 0x%x\n", u32BufferStart, pData, u32BufferSize);
    memset(pData, 0, u32BufferSize);

    return 0;
}


U32 MDrv_MVD_Stream_Download_Init(U32 u32SizeOfBuffer)
{
    U32 u32BufferSize   = 0;
    U32 i;
    struct page *pageptr;
    MVD_PRINT("Required Stream Buffer Size %d\n", u32SizeOfBuffer);
    u32BufferSize = LOCAL_PLAY_MODE_PAGE_NUM * (0x01 << PAGE_SHIFT); //256*4K = 1M
    //1024*1024 => 1M bytes, limit the size of required file play mode bit-stream size
    if (u32SizeOfBuffer > SIZE_OF_STREAM_DOWNLOAD_BUFFER)
    {
        return -1;
    }

    if (NULL != pTMPDataLocalPlayMode)
    {
        if (TSLPM_UNINIT == StateOfLocalPlayMode)
        {
            MVD_PRINT("Wrong state of Local Play Mode %d\n",
                      StateOfLocalPlayMode);
            return -2;
        }
        //Clean memory
        memset(pTMPDataLocalPlayMode, 0, u32BufferSize);
        StateOfLocalPlayMode = TSLPM_INIT;
        gu32SizeOffSet = 0;
        u32MMAP_COPY_SIZE = (MVD_PAGE_NUM * (0x01 << PAGE_SHIFT)) / 2;//64K -> 32(MVD_PAGE_NUM)*4K/2
        return 0;
    }

    //allocate memory for file mode playback
    pTMPDataLocalPlayMode = (U8 *) __get_free_pages(GFP_KERNEL,
                                                    get_order(u32BufferSize));
    if (NULL == pTMPDataLocalPlayMode)
    {
        MVD_PRINT("Cannot alloc %d pages for local play mode\n",
                  LOCAL_PLAY_MODE_PAGE_NUM);
        return -3;
    }

    // set pages to reserved
#if LOCAL_PLAY_MODE_PAGE_NUM > 0
    for (i = 0; i < LOCAL_PLAY_MODE_PAGE_NUM; i++)
    {
        pageptr = virt_to_page(pTMPDataLocalPlayMode +
                               i * (0x01 << PAGE_SHIFT));
        SetPageReserved(pageptr);
    }
    MVD_PRINT("page order = %d\n", get_order(u32BufferSize));
#endif
    MVD_PRINT("MVD buffer addr 0x%x; size %d\n",
              (U32) pTMPDataLocalPlayMode,
              u32BufferSize);
    //Clean memory
    memset(pTMPDataLocalPlayMode, 0, u32BufferSize);
    StateOfLocalPlayMode = TSLPM_INIT;
    gu32SizeOffSet = 0;
    u32MMAP_COPY_SIZE = (MVD_PAGE_NUM * (0x01 << PAGE_SHIFT)) / 2;
    return 0;
}

/*
U32 MDrv_MVD_File_Play_Mode(U32 u32Mode)
{
    //0: for I frame only, 1: for normal
    //MVD_PRINT("Require file play mode [%d]\n",u32Mode);
    u8FilePlayMode = u32Mode;
    MVD_PRINT("File play mode [%d]\n",u8FilePlayMode);
    StateOfLocalPlayMode = TSFPM_SET_MODE;
    return 0;
}
*/

#if 0 // Tonio Liu 20090903
U32 MDrv_MVD_Stream_downloading(void)
{
    U8 *pBufferWritePoint   = NULL;
    U8 *pBufferSource       = NULL;
    //Copy to buffer
    pBufferSource = pDataMMAP + u32MMAP_COPY_SIZE;
    pBufferWritePoint = pTMPDataLocalPlayMode + gu32SizeOffSet;

    memcpy(pBufferWritePoint, pBufferSource, u32MMAP_COPY_SIZE);

    gu32SizeOffSet = gu32SizeOffSet + u32MMAP_COPY_SIZE;
#if 0
    MVD_PRINT("File Test: Src [0x%x] Dst [0x%x] Offset %d Size %d\n",
    //                            pBufferSource,
                                pBufferSource,
                                pBufferWritePoint,
                                gu32SizeOffSet,
                                u32MMAP_COPY_SIZE);
#endif
    StateOfLocalPlayMode = TSLPM_DOWNLOAD;

    return 0;
}

U32 MDrv_MVD_Stream_DownloadEnd(U32 u32SizeOfEndBuffer)
{
    //Copy to buffer
    U8 *pBufferWritePoint   = NULL;
    U8 *pBufferSource       = NULL;
    //Copy to buffer
    pBufferSource = pDataMMAP + u32MMAP_COPY_SIZE;
    pBufferWritePoint = pTMPDataLocalPlayMode + gu32SizeOffSet;

    memcpy(pBufferWritePoint, pBufferSource, u32SizeOfEndBuffer);

    gu32SizeOffSet = gu32SizeOffSet + u32SizeOfEndBuffer;
#if 0
    MVD_PRINT("File Test: Src [0x%x] Dst [0x%x] Offset %d Size %d\n",
    //                            pBufferSource,
                                pBufferSource,
                                pBufferWritePoint,
                                gu32SizeOffSet,
                                u32SizeOfEndBuffer);
#endif
    StateOfLocalPlayMode = TSLPM_DOWNLOAD_END;

    return 0;
}
#endif // Tonio Liu 20090903

/*
MDrv_MVD_Start_File_Play()
{


    if((StateOfLocalPlayMode == TSLPM_DOWNLOAD) || (StateOfLocalPlayMode == TSLPM_DOWNLOAD_END))
    {
#if 0
        MHal_MVD_CopyBitStream(pTMPDataLocalPlayMode, gu32SizeOffSet);
#endif
        //trigger file mode play
        MHal_MVD_FilePlay(0, 0, pTMPDataLocalPlayMode, gu32SizeOffSet);
    }
    else
    {
        return -1;
    }
}
*/

U32 MDrv_MVD_Stream_Download_Close(void)
{
    U32 i;
    struct page *pageptr;
    U32 u32BufferSize   = 0;
    u32BufferSize = LOCAL_PLAY_MODE_PAGE_NUM * (0x01 << PAGE_SHIFT); //256*4K = 1M

    if (NULL != pTMPDataLocalPlayMode)
    {
        // un-set pages to reserved/
#if LOCAL_PLAY_MODE_PAGE_NUM > 0
        for (i = 0; i < 256; i++)
        {
            pageptr = virt_to_page(pTMPDataLocalPlayMode +
                                   i * (0x01 << PAGE_SHIFT) +
                                   0x01);
            ClearPageReserved(pageptr);
        }
        MVD_PRINT("page order = %d\n", get_order(u32BufferSize));
#endif
        //free MVD mmap buffer
        free_pages((unsigned long) pTMPDataLocalPlayMode,
                   get_order(u32BufferSize));
        pTMPDataLocalPlayMode = NULL;
        MVD_PRINT("Success free local play mode buffer\n");
        pTMPDataLocalPlayMode = NULL;
    }
    StateOfLocalPlayMode = TSLPM_UNINIT;
    return 0;
}



U32 MDrv_MVD_WaitIntEvent(void)
{
    DEFINE_WAIT(waitentry_mvd);
    if (0 == u32InterruptEnable)//driver doesn't register interrupt
    {
        return -1;
    }

    if (atomic_read(&_aWaitQueueGuard) == 0x00)
    {
        prepare_to_wait(&wq_mvd, &waitentry_mvd, TASK_INTERRUPTIBLE);
        atomic_set(&_aWaitQueueGuard, 0x01);
        schedule();
        finish_wait(&wq_mvd, &waitentry_mvd);
    }
    return 0;
}

void MDrv_MVD_Wakeup(void)
{
    if (atomic_read(&_aWaitQueueGuard) == 0x01)
    {
        wake_up(&wq_mvd);
        atomic_set(&_aWaitQueueGuard, 0x00);
    }
}

S32 MDrv_MVD_Ioctl(struct inode *inode,
                   struct file *filp,
                   unsigned int cmd,
                   unsigned long arg)
{
    int err                     = 0;
    //int i                       = 0;
    u32 retval;
    //U32 u32BufferSize           = 0;
    U8 *pPicUserData            = NULL;
    //U32         u32SizeOfPicUserData = 0;
    //struct page *pageptr;
    //int         intval;
    u16 u16FrameCnt             = 0;
    MVD_FRAMEINFO picData ;
    MVD_BUFFER_INFO mvdBuf;
    ///MVD_IFRAME_ADDR iFrameAddr;
    MVD_PLAY_MODE_T MVD_PLAY_MODE_Tis;
    MVD_PVR_MODE_T MVD_PVR_MODE_Tis;
    //U32         u32ReturnVal;

    U32 u32BufferStartA         = 0;
    U32 u32BufferSizeA          = 0;

    //MVD_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((MVD_IOC_MAGIC != _IOC_TYPE(cmd)) || (_IOC_NR(cmd) > MVD_IOC_MAXNR))
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
        err = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
    }
    if (err)
    {
        return -EFAULT;
    }


    PROBE_IO_ENTRY(MDRV_MAJOR_MVD, _IOC_NR(cmd));


    switch (cmd)
    {
        case MVD_IOC_INIT:
        {
            //retval = copy_from_user(&__stMVDBuffInfo, (MVD_BUFFER_INFO __user *)arg, sizeof(MVD_BUFFER_INFO));

            u32InterruptEnable = 0;

#if 0 // Tonio Liu 20090903
            //initial pDataMMAP buffer
            if (NULL != pDataMMAP)
            {
                MVD_PRINT("[MVD] Fail to init MMAP buffer!\n");
                return -ENOMEM;
            }
            else
            {
                //allocate 64K memory, 4 = get_order(...), 2^4 * 4K = 64K
                u32BufferSize = MVD_PAGE_NUM * (0x01 << PAGE_SHIFT);
                pDataMMAP = (U8 *)
                            __get_free_pages(GFP_KERNEL,
                                             get_order(u32BufferSize));

                if (pDataMMAP == NULL)
                {
                    MVD_PRINT("Cannot alloc %d pages\n", MVD_PAGE_NUM);
                    return -ENOMEM;
                }

                /* set pages to reserved */
#if MVD_PAGE_NUM > 0
                for (i = 0; i < MVD_PAGE_NUM; i++)
                {
                    pageptr = virt_to_page(pDataMMAP +
                                           i * (0x01 << PAGE_SHIFT));
                    SetPageReserved(pageptr);
                }
                MVD_PRINT("page order = %d\n", get_order(u32BufferSize));
#endif
                //MVD_PRINT("MVD buffer addr 0x%x - 0x%x; size %d\n",(U32)pDataMMAP, __pa(pDataMMAP), u32BufferSize);
                //Clean memory
                memset(pDataMMAP, 0, u32BufferSize);
                //sprintf(pDataMMAP, "Fantasia>> Test %d",(MVD_PAGE_NUM*(0x01<<PAGE_SHIFT)));
                //MVD_PRINT("MVD buffer test [%s]\n",pDataMMAP);
            }
#endif // Tonio Liu 20090903

            if (MVDSTATUS_SUCCESS != MHal_MVD_Init(arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -1;
            }

            MVD_PRINT("ioctl: MVD init done\n");

            //if(MVDSTATUS_SUCCESS == MHal_MVD_Init())
            //{
            //    MVD_PRINT("[MVD] Success to init MVD HAL Layer 11\n");
            //}
            //else{
            //    MVD_PRINT("[MVD] Fail to init MVD HAL Layer! 11\n");
            //}

            //setting picture user data buffer
            //pPicUserData = pDataMMAP;
            //pPicUserData = pPicUserData + (32*1024);//32K (MMAP, 32K-64K for picture user data)
            //u32SizeOfPicUserData = (32*1024); //32K

            //use mvd system allocate memory
#if defined(CONFIG_Titania)
            MHal_MVD_GetFrameBuffer(&u32BufferStartA, &u32BufferSizeA);
            pPicUserData = ioremap(u32BufferStartA, DefSizeOfPictureData);//32KB
            MVD_Debug("MVD picture user data buffer addr 0x%x, size %d\n",
                      (U32) pPicUserData,
                      DefSizeOfPictureData);
            MHal_MVD_SetPictureUserDataBuffer(pPicUserData,
                                              DefSizeOfPictureData);
#else

            MHal_MVD_GetUserDataBuffer(&u32BufferStartA, &u32BufferSizeA);

#if 0
    #if ENABLE_MVD_MIU_256256_SOLUTION
        if( MVD_IsOnMIU1() )
        {
            pPicUserData=(U8*)(mpool_userspace_base + u32BufferStartA + MVD_MIU1_BASE_ADDR);
        }
        else
        {
            pPicUserData = ioremap(u32BufferStartA, u32BufferSizeA);//32KB
        }
    #else
        pPicUserData = ioremap(u32BufferStartA, u32BufferSizeA);//32KB
    #endif
#endif
            pPicUserData = (U8 *)
                           MDrv_SYS_PA2NonCacheSeg((void *)
                                                          u32BufferStartA);

            MVD_Debug("MVD picture user data buffer addr 0x%x, size %d\n",
                      (U32) pPicUserData,
                      u32BufferSizeA);

            MHal_MVD_SetPictureUserDataBuffer((U8 *) u32BufferStartA,
                                              u32BufferSizeA);
#endif

            //disable RS-232
            //(*((volatile U32*)(0xBF800000 +(0xf1e<<2)))) = 0x1d0;
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return 0;
        }
        case MVD_IOC_PLAY:
        {
            U8 frc  = (U8) arg;
            MVD_PRINT("ioctl: MVD play mode: %d\n", (U8) arg);
            retval = MHal_MVD_Play((U8) frc);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_PAUSE:
        {
            MVD_PRINT("ioctl: MVD Pause\n");
            retval = MHal_MVD_Pause();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_STOP:
        {
            MVD_PRINT("ioctl: MVD Stop\n");
            retval = MHal_MVD_Stop();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_DECODE_IFRAME:
        {
            MVD_PRINT("ioctl: MVD decode I frame\n");
            if (__get_user(mvdBuf.u32BufferAddr,
                           &(((MVD_BUFFER_INFO __user *) arg)->u32BufferAddr)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            if (__get_user(mvdBuf.u32BufferSize,
                           &(((MVD_BUFFER_INFO __user *) arg)->u32BufferSize)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            retval = MHal_MVD_Decode_IFrame(mvdBuf.u32BufferAddr,
                                            mvdBuf.u32BufferAddr +
                                            mvdBuf.u32BufferSize);
            MVD_PRINT("ioctl: MVD decode I frame done %d\n", retval);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_FRAME_CNT:
        {
            MVD_PRINT("ioctl: MVD get frame cnt\n");
            retval = MHal_MVD_GetDecdeFrameCount(&u16FrameCnt);

            if (__put_user(u16FrameCnt, (int __user *) arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_PIC_DATA:
        {
            //MVD_PRINT("ioctl: MVD get pic data\n");

            retval = MHal_MVD_GetPictureData(&picData);

            if (__copy_to_user((MVD_FRAMEINFO __user *) arg,
                               &picData,
                               sizeof(MVD_FRAMEINFO)))
            {
                MVD_PRINT("copy picture data failed\n");
            }
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_RESET:
        {
            MVD_PRINT("ioctl: MVD reset\n");
            retval = MHal_MVD_Reset(arg);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_FIRST_FRAME:
        {
            //MVD_PRINT("ioctl: MVD get first frame\n");
            retval = MHal_MVD_GetFirstFrame();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_DISP_RDY:
        {
            MVD_PRINT("ioctl: MVD get disp rdy\n");
            retval = MHal_MVD_GetDispRdy();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_SYNC_STATUS:
        {
            MVD_PRINT("ioctl: MVD get sync status\n");
            retval = MHal_MVD_GetSyncStatus();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_I_PIC_FOUND:
        {
            MVD_PRINT("ioctl: MVD I pic found\n");
            retval = MHal_MVD_IPicFound();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_VOP_DONE:
        {
            //MVD_PRINT("ioctl: MVD set vop done\n");
            retval = MHal_MVD_SetVOPDone();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_PROG_INTL:
        {
            //MVD_PRINT("ioctl: MVD get prog/intl\n");
            retval = MHal_MVD_GetProgInt();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_PWR_ON:
		{
            MVD_PRINT("ioctl: MVD power on\n");
            retval = MHal_MVD_PowerOn();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
		}
        case MVD_IOC_SET_PWR_OFF:
		{
            MVD_PRINT("ioctl: MVD power off\n");
            retval = MHal_MVD_PowerOff();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
    	}
        case MVD_IOC_GET_ACTIVEFORMAT:
        {
#if !defined(CONFIG_Titania)
            U32 u32active_format= 0;
            //MVD_PRINT("ioctl: MVD get active format\n");
            retval = MHal_MVD_GetActiveFormat(&u32active_format);

            if (__put_user(u32active_format, (int __user *) arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            //MVD_PRINT(" AFD=0x%x\n",u32active_format);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
#endif
			break;
        }

        case MVD_IOC_WAITEVENT:
		{
            //MVD_PRINT("ioctl : MVD wait Event\n");
            //return MHal_MVD_WaitEvent((MVD_FRAMEINFO*)arg);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return -1;
    	}
        case MVD_IOC_CLOSEWAIT:
		{
            //MVD_PRINT("ioctl: close wait \n");
            //return MHal_MVD_CloseWait();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return -1;
    	}
        case MVD_IOC_TOGGLE_AV_SYNC:
        {
            U8 u8Flag   = (U8) arg;
            retval = MHal_MVD_ToggleAVSync((U8) u8Flag);
            MVD_PRINT("ioctl: MVD toggle AV sync: %d : 1(E) 0(D)\n",
                      u8Flag);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_REG_INT:
        {
            MVD_PRINT("ioctl: MVD Register interrupt\n");
            retval = MDrv_MVD_RegisterInterrupt();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_DEREG_INT:
        {
            MVD_PRINT("ioctl: MVD DeRegister interrupt\n");
            retval = MDrv_MVD_DeRegisterInterrupt();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_WAIT_INT_EVENT:
        {
            //MVD_PRINT("ioctl: MVD Wait interrupt event\n");
            retval = MDrv_MVD_WaitIntEvent();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_AV_SYNC_STATUS:
        {
            U32 u32AVSyncStatus = 0;

            MVD_PRINT("ioctl: MVD get av sync status\n");

            retval = MHal_MVD_GetAvSyncStatus(&u32AVSyncStatus);

            if (__put_user(u32AVSyncStatus, (int __user *) arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_INT_FLAG:
        {
            MVD_PRINT("ioctl: MVD set Interrupt flag %x\n", (U32) arg);
            retval = Mhal_MVD_SetIntSubscribe((U32) arg);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_FILE_PLAY:
        {
            MVD_PRINT("ioctl: MVD Set file play\n");
            //MVD_PLAY_MODE_Tis;
            if (__get_user(mvdBuf.u32BufferAddr,
                           &(((MVD_BUFFER_INFO __user *) arg)->u32BufferAddr)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            if (__get_user(mvdBuf.u32BufferSize,
                           &(((MVD_BUFFER_INFO __user *) arg)->u32BufferSize)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            retval = MHal_MVD_SetFilePlay(mvdBuf.u32BufferAddr,
                                          mvdBuf.u32BufferAddr +
                                          mvdBuf.u32BufferSize);
            MVD_PRINT("ioctl: MVD Set file play done\n");
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_FILE_CLOSE:
        {
            MVD_PRINT("ioctl: MVD Set file close\n");
            //MVD_PLAY_MODE_Tis;
            retval = MHal_MVD_SetFileClose();
            MVD_PRINT("ioctl: MVD Set file close done\n");
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_FILE_PLAY2:
        {
            MVD_PRINT("ioctl: MVD Set file play2\n");
            //MVD_PLAY_MODE_Tis;
            if (__get_user(MVD_PLAY_MODE_Tis.u8PlayFrameRate,
                           &(((MVD_PLAY_MODE_T __user *) arg)->u8PlayFrameRate)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            if (__get_user(MVD_PLAY_MODE_Tis.u8Mode,
                           &(((MVD_PLAY_MODE_T __user *) arg)->u8Mode)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            retval = MHal_MVD_SetFilePlay2(MVD_PLAY_MODE_Tis.u8PlayFrameRate,
                                           MVD_PLAY_MODE_Tis.u8Mode);
            MVD_PRINT("ioctl: MVD Set file play2 %d %d done\n",
                      MVD_PLAY_MODE_Tis.u8PlayFrameRate,
                      MVD_PLAY_MODE_Tis.u8Mode);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_PVR_MODE:
        {
            MVD_PRINT("ioctl: MVD set PVR mode\n");
            //MVD_PVR_MODE_Tis
            if (__get_user(MVD_PVR_MODE_Tis.PlayMode,
                           &(((MVD_PVR_MODE_T __user *) arg)->PlayMode)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            if (__get_user(MVD_PVR_MODE_Tis.u8FrameRateUnit,
                           &(((MVD_PVR_MODE_T __user *) arg)->u8FrameRateUnit)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }   // check the return value of __put_user/__get_user (dreamer@lge.com)
            retval = MHal_MVD_PVRPlayMode(MVD_PVR_MODE_Tis.PlayMode,
                                          MVD_PVR_MODE_Tis.u8FrameRateUnit);
            MVD_PRINT("ioctl: MVD set PVR mode %d %d done\n",
                      MVD_PVR_MODE_Tis.PlayMode,
                      MVD_PVR_MODE_Tis.u8FrameRateUnit);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_LOCAL_PLAY_INIT:
        {
            U32 u32BitStreamSize= (U32) arg;
            MVD_PRINT("ioctl: MVD Init local play %d\n", u32BitStreamSize);
            retval = MDrv_MVD_Stream_Download_Init(u32BitStreamSize);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_LOCAL_PLAY_CLOSE:
        {
            MVD_PRINT("ioctl: MVD STOP local play\n");
            retval = MDrv_MVD_Stream_Download_Close();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
#if 0 // Tonio Liu 20090903
        case MVD_IOC_LOCAL_PLAY_DOWNLOADING:
        {
            MVD_PRINT("ioctl: MVD download bit-stream for local play\n");
            return MDrv_MVD_Stream_downloading();
        }
        case MVD_IOC_LOCAL_PLAY_DOWNLOADEND:
        {
            U32 u32RestBitStreamSize= (U32) arg;
            MVD_PRINT("ioctl: MVD download end of bit-stream for local play [%d]\n",
                      u32RestBitStreamSize);
            return MDrv_MVD_Stream_DownloadEnd(u32RestBitStreamSize);
        }
#endif // Tonio Liu 20090903
        case MVD_IOC_CLEAN_BIT_STREAM_BUFF:
        {
            MVD_PRINT("ioctl: MVD clean bit-stream buffer to zero\n");
            retval = MDrv_MVD_CleanBitStreamBuff();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_VIDEO_REPEAT:
		{
            //jmkim        MVD_PRINT("ioctl: MVD get video repeat\n");
            retval = MHal_MVD_GetVideoRepeat();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
    	}
        case MVD_IOC_GET_VIDEO_SKIP:
		{
            //jmkim        MVD_PRINT("ioctl: MVD get video skip\n");
            retval = MHal_MVD_GetVideoSkip();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
		}
        case MVD_IOC_GET_PTS:
        {
            MVD_PTS pts;
            U32 pu32PTSLow  = 0;
            U32 pu32PTSHigh = 0;
            //jmkim        MVD_PRINT("ioctl: MVD get PTS\n");
            retval = MHal_MVD_GetPTS(&pu32PTSLow, &pu32PTSHigh);

            pts.u32High = pu32PTSHigh;
            pts.u32Low = pu32PTSLow;

            __copy_to_user((MVD_FRAMEINFO __user *) arg,
                           &pts,
                           sizeof(MVD_PTS));
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_FRAMEBUF:
        {
            MVD_PRINT("ioctl: MVD get framebuf\n");
            retval = MHal_MVD_GetFrameBuffer(&(mvdBuf.u32BufferAddr),
                                             &(mvdBuf.u32BufferSize));
#if defined(CONFIG_Titania)
            mvdBuf.u32BufferAddr += DefSizeOfPictureData;
#endif
            __copy_to_user((MVD_BUFFER_INFO __user *) arg,
                           &mvdBuf,
                           sizeof(MVD_BUFFER_INFO));
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_DATA_MEM:
        {
#define MVD_ALIGN_4(_x) (((U32)(_x) + 3) & ~3)

            pMVDDataMemSerNumb = (U32*)MVD_ALIGN_4(ioremap(arg, 256 * 1024));
            pDataMem = (U8*)((U32)pMVDDataMemSerNumb +4);
            MVD_PRINT("ioctl: mvd set data memory : Paddr:0x%x Vaddr:0x%x\n", (U32)arg , (U32)pDataMem );
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));

            return MVDSTATUS_SUCCESS;
        }
        case MVD_IOC_SET_AVSYNC_THRESHOLD:
        {
            U16 u16Threshold= (U16) arg;
            MVD_PRINT("ioctl: MVD av sync threshold %d\n", u16Threshold);
            retval = MHal_MVD_SetAVSyncThreshold(u16Threshold);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_DELAY:
        {
            U32 u32Delay= (U32) arg;
            MVD_PRINT("ioctl: MVD set delay time(ms): %d\n", u32Delay);
            retval = MHal_MVD_SetDelay(u32Delay);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_SYNC_THRESHOLD:
        {
            U32 u32Threshold= (U32) arg;
            MVD_PRINT("ioctl: MVD set threshold(ms): %d\n", u32Threshold);
            retval = MHal_MVD_SetSyncThreshold(u32Threshold);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_SKIP_REPEAT_MODE:
        {
            U8 u8Mode = (U8) arg;
            MVD_PRINT("ioctl: MVD set skip & repeat mode : %d\n", u8Mode);
            retval = MHal_MVD_SetSkipRepeatMode(u8Mode);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_SET_PLAY_MODE:
        {
            U32 u32Mode = (U32) arg;
            MVD_PRINT("ioctl: MVD set play mode: %d\n", u32Mode);
            retval = MHal_MVD_SetPlayMode(u32Mode);
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_STEP_PLAY:
        {
            MVD_PRINT("ioctl: MVD step play\n");
            retval = MHal_MVD_StepPlay();
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_ESDATASIZE:
        {
            U32 u32DataSize = 0;
            MVD_PRINT("ioctl: MVD get es buffer data size\n");
            retval = MHal_MVD_GetESDataSize(&u32DataSize);
            if (__put_user(u32DataSize, (int __user *) arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_ESRDPTR:
        {
            U32 u32Ptr;
            
            MVD_PRINT("ioctl: MVD get es rd ptr\n");
            
            retval = MHal_MVD_GetESRdPtr(&u32Ptr);
            
            if (__put_user(u32Ptr, (int __user *) arg))
            {
				PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }
			PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        case MVD_IOC_GET_ESWRPTR:
        {
            U32 u32Ptr;
            
            MVD_PRINT("ioctl: MVD get es wr ptr\n");
            
            retval = MHal_MVD_GetESWrPtr(&u32Ptr);
            
            if (__put_user(u32Ptr, (int __user *) arg))
            {
				PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
                return -EFAULT;
            }
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return retval;
        }
        default:
            //MVD_WARNING("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
            return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_MVD, _IOC_NR(cmd));
    return 0;
}

S32 MDrv_MVD_Open(struct inode *inode, struct file *filp)
{
#if 0 // Tonio Liu 20090903
    pDataMMAP = NULL;
#endif // Tonio Liu 20090903

    init_waitqueue_head(&wq_mvd);
    return 0;
}

S32 MDrv_MVD_Release(struct inode *inode, struct file *filp)
{
    //struct page *pageptr;
    //U32 i;
    U32 u32BufferSize       = 0;

    u32BufferSize = MVD_PAGE_NUM * (0x01 << PAGE_SHIFT);
    free_irq(E_IRQ_MVD, NULL);
#if 0 // Tonio Liu 20090903
    if (NULL != pDataMMAP)
    {
        /* un-set pages to reserved */
#if MVD_PAGE_NUM > 0
        for (i = 0; i < MVD_PAGE_NUM; i++)
        {
            pageptr = virt_to_page(pDataMMAP + i * (0x01 << PAGE_SHIFT) + 0x01);
            ClearPageReserved(pageptr);
        }
        MVD_PRINT("page order = %d\n", get_order(u32BufferSize));
#endif
        //free MVD mmap buffer
        free_pages((unsigned long) pDataMMAP, get_order(u32BufferSize));
        pDataMMAP = NULL;
        MVD_PRINT("Success free mmap buffer\n");
    }
#endif // Tonio Liu 20090903
    if (NULL != pTMPDataLocalPlayMode)
    {
        MDrv_MVD_Stream_Download_Close();
    }
    return 0;
}

// mmap implementation for callback data to user mode adaptation layer

void MDrv_MVD_vma_open(struct vm_area_struct *vma)
{
    return;
}

void MDrv_MVD_vma_close(struct vm_area_struct *vma)
{
    return;
}
#if 0
struct page *MDrv_MVD_vma_nopage(struct vm_area_struct *vma,
                                unsigned long address, int *type)
{
    return 0;
}
#endif
struct vm_operations_struct mvd_vm_ops  = {.open = MDrv_MVD_vma_open,
                                           .close = MDrv_MVD_vma_close,
                                           //  .nopage =   MDrv_MVD_vma_nopage,
};

#if 0 // Tonio Liu 20090903
S32 MDrv_MVD__mmap(struct file *filp, struct vm_area_struct *vma)
{
    U32 u32BufferSize           = 0;

    MVD_PRINT("VMSIZE = 0x%x\n", (U32) (vma->vm_end - vma->vm_start));
    u32BufferSize = MVD_PAGE_NUM * (0x01 << PAGE_SHIFT);

    if (NULL == pDataMMAP)
    {
        return -EAGAIN;
    }

    //do not allow larger mappings than the number of pages allocated
    MVD_PRINT("Check length  ... \n");
    if ((vma->vm_end - vma->vm_start) > u32BufferSize)
    {
        return -EAGAIN;
    }

    vma->vm_flags |= VM_RESERVED;

    pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
    pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;

    vma->vm_ops = &mvd_vm_ops;

    remap_pfn_range(vma,
                    vma->vm_start,
                    (virt_to_phys(pDataMMAP) >> PAGE_SHIFT),
                    vma->vm_end - vma->vm_start,
                    vma->vm_page_prot);


    MVD_PRINT("VM-START = 0x%x, VM-END = 0x%x, PFN = 0x%x\n",
              (U32) (vma->vm_start),
              (U32) (vma->vm_end),
              (U32) (virt_to_phys(pDataMMAP) >> PAGE_SHIFT));

    return 0;
}
#endif // Tonio Liu 20090903

static int __init MDrv_MVD_ModuleInit(void)
{
    //int         s32Ret;
    dev_t devno;
    //U32         result;
    int result;     /* STATIC ANALYSIS(NO_EFFECT) ¼öÁ¤ (dreamer@lge.com)*/
#if defined(CONFIG_Titania)
    U32 u32BuffUnit                 = 0x2000;
    u32 u32BootBufferSize           = RISC_BOOT_CODE_SIZE + u32BuffUnit;
#endif
#if 0
    U32 u32BufferSize = 0;
    U32 i;
    struct page *pageptr;
#endif
    U32 u32BufferStart              = 0;
    U32 u32BufferSize               = 0;

    devno = MKDEV(MDRV_MAJOR_MVD, MDRV_MINOR_MVD);
    result = register_chrdev_region(devno, 1, "drvMVD");
    if (result < 0)
    {
        printk(KERN_WARNING "MVD: can't get major %d\n", MDRV_MAJOR_MVD);
        return result;
    }

#if defined(CONFIG_Titania)
#if 1
    g_mvd_fw_buf = kmalloc(u32BootBufferSize, GFP_KERNEL);

#else
    //dhjung LGE
    g_mvd_fw_buf = (BIN_MEM_ADR + 0x20000) + 0x80000000;
#endif

    if (NULL == g_mvd_fw_buf)
    {
        MVD_PRINT("MVD fail to allocate memory for loading MVD firmware\n");
        return MVDSTATUS_OUTOF_MEMORY;
    }
    MVD_PRINT("MVD firmware buffer addr 0x%x [0x%x], size %d\n",
              (U32) g_mvd_fw_buf,
              (U32) (__pa(g_mvd_fw_buf)),
              u32BootBufferSize);
#endif

    MHal_MVD_GetBitStreamBuffer(&u32BufferStart, &u32BufferSize);
#if 0
#if ENABLE_MVD_MIU_256256_SOLUTION
    if (MHal_MVD_IsOnMIU1())
    {
        pTMPDataBitStreamBuff = (U8 *)
                                (mpool_userspace_base +
                                 u32BufferStart +
                                 MVD_MIU1_BASE_ADDR);
    }
    else
    {
        pTMPDataBitStreamBuff = ioremap(u32BufferStart, u32BufferSize);
    }
#else
    pTMPDataBitStreamBuff = ioremap(u32BufferStart, u32BufferSize);
#endif
#endif
    pTMPDataBitStreamBuff = (U8*)MDrv_SYS_PA2NonCacheSeg((void*)u32BufferStart);

    MVD_PRINT("MVD bit-stream buffer addr 0x%x; size %d\n",
              (U32) pTMPDataBitStreamBuff,
              u32BufferSize);


#if 0
    u32BufferSize = LOCAL_BIT_STREAM_PAGE_NUM*(0x01<<PAGE_SHIFT); //256*4K = 1M
    //allocate memory for file mode playback
    pTMPDataBitStreamBuff = (U8*)__get_free_pages(GFP_KERNEL, get_order(u32BufferSize));
    if(NULL == pTMPDataBitStreamBuff)
    {
        MVD_PRINT("Cannot alloc %d pages for local play mode\n",LOCAL_BIT_STREAM_PAGE_NUM);
        return -3;
    }

    // set pages to reserved
    #if LOCAL_BIT_STREAM_PAGE_NUM > 0
        for(i=0; i<LOCAL_BIT_STREAM_PAGE_NUM; i++)
        {
            pageptr = virt_to_page(pTMPDataBitStreamBuff+i*(0x01<<PAGE_SHIFT));
            SetPageReserved(pageptr);
        }
        MVD_PRINT("page order = %d\n", get_order(u32BufferSize));
    #endif
    MVD_PRINT("MVD bit-stream buffer addr 0x%x; size %d\n",(U32)pTMPDataBitStreamBuff, u32BufferSize);
    //Clean memory
    memset(pTMPDataBitStreamBuff, 0, u32BufferSize);
#endif

    cdev_init(&(g_MVD_Dev.cdev), &MVD_Fops);
    g_MVD_Dev.cdev.owner = THIS_MODULE;
    g_MVD_Dev.cdev.ops = &MVD_Fops;
    result = cdev_add(&(g_MVD_Dev.cdev), devno, 1);
    /* Fail gracefully if need be */
    if (result)
    {
        printk(KERN_NOTICE "Error add MVD device");
    }


    //MHal_MVD_Init();

    return 0;
}

static void __exit MDrv_MVD_ModuleExit(void)
{
#if 0
    U32 i;
    struct page *pageptr;
    U32 u32BufferSize = 0;
#endif
    //IR_PRINT("%s is invoked\n", __FUNCTION__);
    dev_t devno;
    devno = MKDEV(MDRV_MAJOR_MVD, MDRV_MINOR_MVD);

#if defined(CONFIG_Titania)
    if (g_mvd_fw_buf)
    {
        kfree((void *) g_mvd_fw_buf);
        g_mvd_fw_buf = NULL;
    }
#endif

#if 0
    u32BufferSize = LOCAL_BIT_STREAM_PAGE_NUM*(0x01<<PAGE_SHIFT); //256*4K = 1M

    if(NULL != pTMPDataBitStreamBuff)
    {
    // un-set pages to reserved/
        #if LOCAL_PLAY_MODE_PAGE_NUM > 0
            for(i=0; i<256; i++)
            {
                pageptr = virt_to_page(pTMPDataBitStreamBuff+i*(0x01<<PAGE_SHIFT) + 0x01);
                ClearPageReserved(pageptr);
            }
            MVD_PRINT("page order = %d\n", get_order(u32BufferSize));
        #endif
        //free MVD mmap buffer
        free_pages((unsigned long)pTMPDataBitStreamBuff, get_order(u32BufferSize));
        pTMPDataBitStreamBuff = NULL;
        MVD_PRINT("Success free local bit-stream buffer\n");
    }
#endif
    //cdev_del(&IRDev.cDevice);
    unregister_chrdev_region(devno, 1);
}





module_init(MDrv_MVD_ModuleInit);
module_exit(MDrv_MVD_ModuleExit);

//dhjung LGE
MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MVD driver");
MODULE_LICENSE("MSTAR");

///////////////////////////////////////////////////////////////////////
