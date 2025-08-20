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
/// file    mdrv_h264.c
/// @brief  H.264 Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
//#include "chip_int.h"


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

#include "mdrv_probe.h"

#ifdef CONFIG_MSTAR_TITANIA
#else
#include "mst_devid.h"
#include "mdrv_h264.h"
#include "mhal_h264.h"

#if defined(UTOPIA_HVD_DRIVER)
#include "drvHVD_Common.h"
#include "drvHVD.h"

extern void MHal_H264_AllocBuf(void);

#endif
//--------------------------------------------------------------------------------------------------
// Constant definition
//--------------------------------------------------------------------------------------------------
#define MVD_WARNING(fmt, args...)       printk(KERN_WARNING "[HVDMOD][%06d]     " fmt, __LINE__, ## args)
#define MVD_PRINT(fmt, args...)         printk("[HVDMOD][%06d]     " fmt, __LINE__, ## args)

#if HVD_ENABLE_ISR_POLL
static unsigned int MDrv_H264_poll(struct file *filp, poll_table *wait);

wait_queue_head_t        gHVD_wait_queue;
EXPORT_SYMBOL(gHVD_wait_queue);
#endif

struct MVD_Dev  g_H264_Dev;

struct file_operations H264_Fops = {
	.owner =    THIS_MODULE,
	.ioctl =    MDrv_H264_Ioctl,
	.open =     MDrv_H264_Open,
	.release =  MDrv_H264_Release,
    #if HVD_ENABLE_ISR_POLL
        .poll=      MDrv_H264_poll,
    #endif
	.mmap =	    MDrv_H264__mmap,
};

typedef enum _TYPE_State_Local_Play_Mode{
	TSLPM_UNINIT = 0,
	TSLPM_INIT,
	TSLPM_SET_MODE,
	TSLPM_DOWNLOAD,
	TSLPM_DOWNLOAD_END,
	TSLPM_PLAY
}TYPE_State_File_Play_Mode;


//static wait_queue_head_t wq_mvd;
//atomic_t    _aWaitQueueGuard = ATOMIC_INIT(0);

U8 *pH264DataMem = NULL;
U32 *pH264DataMemSerNumb = NULL;

#define SIZE_OF_STREAM_DOWNLOAD_BUFFER  1048576         //1024*1024=1MB
#define LOCAL_PLAY_MODE_PAGE_NUM 256
#define LOCAL_BIT_STREAM_PAGE_NUM 512        //2MB
static U8* pTMPDataBitStreamBuff = NULL;

#if defined(UTOPIA_HVD_DRIVER)
U32 u32MIU1BaseAddr=0;
U32 u32CodeVAddr=0;
U32 u32CodeAddr=0;
U32 u32CodeSize=0;
U32 u32FrmVAddr=0;
U32 u32FrmAddr=0;
U32 u32FrmSize=0;
U32 u32ESVAddr=0;
U32 u32ESAddr=0;
U32 u32ESSize=0;
U32 u32DrvProcVAddr=0;
U32 u32DrvProcAddr=0;
U32 u32DrvProcSize=0;
#endif

extern void* MDrv_SYS_PA2NonCacheSeg( void* pAddrPA );

U32 MDrv_H264_RegisterInterrupt(void)
{
    return MHal_H264_RegisterInterrupt();
}

U32 MDrv_H264_DeRegisterInterrupt(void)
{
    return MHal_H264_DeRegisterInterrupt();
}

U32 MDrv_H264_CleanBitStreamBuff(void)
{
    // it is very dangerous to clear ES directly. It may cause HW hang up for a long time.
    // Use flush in stead of clear could have the same effect.
#if 1
    BOOL bPlayback=FALSE;
    U32 timer=300;
    if( MDrv_HVD_GetPlayState() ==  E_HVD_GSTATE_PLAY )
    {
        bPlayback=TRUE;
    }
    if( MDrv_HVD_Flush(FALSE) == E_HVD_OK)
    {
        while( timer )
        {
            if( MDrv_HVD_IsAllBufferEmpty() )
            {
                MVD_PRINT("HVD flush ES buffer success\n");
                break;
            }
            msleep(1);
            timer--;
        }
        if( timer ==0)
        {
            MVD_PRINT("HVD flush ES buffer failed\n");
        }
    }
    if( (MDrv_HVD_GetPlayState() !=  E_HVD_GSTATE_PLAY) && bPlayback )
    {
        MDrv_HVD_Play();
    }
#else
    U32 u32BufferStart = 0;
    U32 u32BufferSize = 0;
    U8* pData = NULL;
    MVD_PRINT("[H264] Clean Bit-Stream Buff\n");
    MHal_H264_GetBitStreamBuffer(&u32BufferStart, &u32BufferSize);

    u32BufferStart = (U32)pTMPDataBitStreamBuff;

    if(( 0 >= u32BufferSize) || (0 == u32BufferStart))
    {
        MVD_PRINT("[H264] Clean Bit-Stream Buff fail 0x%x, 0x%x\n", u32BufferStart, u32BufferSize);
        return -1;
    }
	#if 0
    pData = (U8*)(u32BufferStart | 0xA0000000);
	#else
	pData = (U8*)pTMPDataBitStreamBuff;
	#endif
    //MVD_PRINT("[MVD] Clean Bit-Stream Buff 0x%x, 0x%x 0x%x\n", u32BufferStart, pData, u32BufferSize);
    memset(pData, 0, u32BufferSize);
#endif
    return 0;
}


U32 MDrv_H264_Stream_Download_Init(U32 u32SizeOfBuffer)
{
    //FIXME
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

U32 MDrv_H264_Stream_downloading(void)
{
//FIXME

    return 0;
}

U32 MDrv_H264_Stream_DownloadEnd(U32 u32SizeOfEndBuffer)
{
    //FIXME

    return 0;
}

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

U32 MDrv_H264_Stream_Download_Close(void)
{
    //FIXME
    return 0;
}



U32 MDrv_H264_WaitIntEvent(void)
{
//FIXME
    return 0;
}

void MDrv_H264_Wakeup(void)
{
//FIXME
}

S32 MDrv_H264_Ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
    int         err= 0;
    u32         retval=0;
    u32         u32FrameCnt = 0;
    AVCH264_FRAMEINFO picData ;
    VUI_DISP_INFO vuiInfo ;
    H264_Decoder_Status decoderstatus;
    H264_BUFFER_INFO mvdBuf;
    ///MVD_IFRAME_ADDR iFrameAddr;
    H264_PLAY_MODE_T H264_PLAY_MODE_Tis;
    H264_PVR_MODE_T H264_PVR_MODE_Tis;


    //MVD_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((H264_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> H264_IOC_MAXNR))
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

    PROBE_IO_ENTRY(MDRV_MAJOR_H264, _IOC_NR(cmd));

    switch(cmd)
    {
    case H264_IOC_INIT:
    	MVD_PRINT("ioctl: H264 init stream type: %d\n", ((H264_INIT_PARAM*)arg)->eCodecType );
        if( H264STATUS_SUCCESS==MHal_H264_Init((H264_INIT_PARAM*)arg))
        {
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return retval ;
        }
        else
        {
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return -1 ;
        }
        break;
    case H264_IOC_PLAY:
    	{
    		MVD_PRINT("ioctl: H264 play mode: %d\n", (U8)arg);
    		PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return MHal_H264_Play((U8)arg);
        }
        break;
    case H264_IOC_PAUSE:
        MVD_PRINT("ioctl: H264 Pause\n");
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return MHal_H264_Pause();
        break;
    case H264_IOC_STOP:
        MVD_PRINT("ioctl: H264 Stop\n");
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return MHal_H264_Stop();
        break;
    case H264_IOC_DECODE_IFRAME:
        MVD_PRINT("ioctl: H264 decode I frame\n");
        if( __get_user(mvdBuf.u32BufferAddr, &(((H264_BUFFER_INFO __user *)arg)->u32BufferAddr)) )
        {
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
        }
        if( __get_user(mvdBuf.u32BufferSize, &(((H264_BUFFER_INFO __user *)arg)->u32BufferSize)) )
        {
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
        }
        retval = MHal_H264_Decode_IFrame(mvdBuf.u32BufferAddr, mvdBuf.u32BufferAddr+mvdBuf.u32BufferSize);
        MVD_PRINT("ioctl: H264 decode I frame done %d (%x %x)\n",retval  ,  mvdBuf.u32BufferAddr , mvdBuf.u32BufferSize  );
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_GET_FRAME_CNT:
//jmkim        MVD_PRINT("ioctl: MVD get frame cnt\n");
        retval = MHal_H264_GetDecdeFrameCount(&u32FrameCnt);
        if( __put_user(u32FrameCnt, (int __user*)arg) )
        {
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
        }

        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_GET_VUI_INFO:
//jmkim        MVD_PRINT("ioctl: SVD get VUI info\n");
        retval = MHal_H264_GetVUIInfo(&vuiInfo);
        if (__copy_to_user((VUI_DISP_INFO __user *)arg, &vuiInfo, sizeof(VUI_DISP_INFO)))
            MVD_PRINT("copy VUI info failed\n");
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_GET_PIC_DATA:
//jmkim        MVD_PRINT("ioctl: MVD get pic data\n");
        retval = MHal_H264_GetPictureData(&picData);
        if (__copy_to_user((AVCH264_FRAMEINFO __user *)arg, &picData, sizeof(AVCH264_FRAMEINFO)))
            MVD_PRINT("copy picture data failed\n");
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_RESET:
        MVD_PRINT("ioctl: H264 reset\n");
        MHal_H264_Reload((H264_INIT_PARAM*)arg);
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_GET_FIRST_FRAME:
//jmkim        MVD_PRINT("ioctl: MVD get first frame\n");
        retval = MHal_H264_GetFirstFrame();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_GET_DISP_RDY:
//jmkim        MVD_PRINT("ioctl: MVD get disp rdy\n");

        retval = MHal_H264_GetDispRdy();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_IS_SEQ_CHG:
        retval = MHal_H264_IsSeqChg();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_GET_SYNC_STATUS:
//jmkim        MVD_PRINT("ioctl: MVD get sync status\n");
        retval = MHal_H264_GetSyncStatus();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_I_PIC_FOUND:
//jmkim        MVD_PRINT("ioctl: MVD I pic found\n");
        retval = MHal_H264_IPicFound();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_SET_VOP_DONE:
        MVD_PRINT("ioctl: H264 set vop done\n");

        retval = MHal_H264_SetVOPDone();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_GET_PROG_INTL:
        //MVD_PRINT("ioctl: MVD get prog/intl\n");

        retval = MHal_H264_GetProgInt();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;

    case H264_IOC_SET_PWR_ON:
        MVD_PRINT("ioctl: H264 power on\n");

        retval = MHal_H264_PowerOn();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;

        break;
    case H264_IOC_SET_PWR_OFF:
        MVD_PRINT("ioctl: H264 power off\n");

        retval = MHal_H264_PowerOff();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;

        break;
    case H264_IOC_GET_ACTIVEFORMAT:
        {
            U8 u8active_format = 0;
    	    //MVD_PRINT("ioctl: H264 get active format\n");
    	    MHal_H264_GetActiveFormat(&u8active_format);
            if( __put_user(u8active_format, (int __user*)arg) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
                return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
            }

            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return 0;
        }
    	break;
	case H264_IOC_GET_FAST_FRAME_INFO:
        {
            AVCH264_FRAMEINFO tmp;
            MHal_H264_FastGetFrameInfo(&tmp);
            if (__copy_to_user((AVCH264_FRAMEINFO __user *)arg, &tmp, sizeof(AVCH264_FRAMEINFO)))
                MVD_PRINT("fast copy picture data failed\n");
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return 0;
        }
    	break;
	case H264_IOC_WAITEVENT:
		//MVD_PRINT("ioctl : MVD wait Event\n");
		//return MHal_MVD_WaitEvent((MVD_FRAMEINFO*)arg);
		PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
		return -1;
		break;
	case H264_IOC_CLOSEWAIT:
		//MVD_PRINT("ioctl: close wait \n");
		//return MHal_MVD_CloseWait();
		PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
		return -1;
		break;
	case H264_IOC_TOGGLE_AV_SYNC:
    	{
    		U8 u8Flag = (U8)arg;
        	MVD_PRINT("ioctl: H264 toggle AV sync: %d : 1(E) 0(D)\n", u8Flag);
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return MHal_H264_ToggleAVSync((U8)u8Flag);
        }
        break;
    case H264_IOC_REG_INT:
    	{
        	MVD_PRINT("ioctl: H264 Register interrupt\n");
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return MDrv_H264_RegisterInterrupt();
        }
        break;
    case H264_IOC_DEREG_INT:
    	{
        	MVD_PRINT("ioctl: H264 DeRegister interrupt\n");
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return MDrv_H264_DeRegisterInterrupt();
        }
        break;
    case H264_IOC_WAIT_INT_EVENT:
        {
        	//MVD_PRINT("ioctl: MVD Wait interrupt event\n");

        	retval = MDrv_H264_WaitIntEvent();
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return retval;
        }
        break;
    case H264_IOC_GET_AV_SYNC_STATUS:
        {
            U32 u32AVSyncStatus = 0;
        	MVD_PRINT("ioctl: H264 get av sync status\n");
        	retval = MHal_H264_GetAvSyncStatus(&u32AVSyncStatus);
            if( __put_user(u32AVSyncStatus, (int __user*)arg) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
                return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
            }
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return retval;
        }
        break;
    case H264_IOC_SET_INT_FLAG:
        {
            MVD_PRINT("ioctl: H264 set Interrupt flag %x\n", (U32)arg);

            retval = Mhal_H264_SetIntSubscribe((U32)arg);
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return retval;
        }
        break;
	case H264_IOC_SET_FILE_PLAY:
        {
            MVD_PRINT("ioctl: H264 Set file play\n");
            //MVD_PLAY_MODE_Tis;
            if( __get_user(mvdBuf.u32BufferAddr, &(((H264_BUFFER_INFO __user *)arg)->u32BufferAddr)) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
                return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
            }
            if( __get_user(mvdBuf.u32BufferSize, &(((H264_BUFFER_INFO __user *)arg)->u32BufferSize)) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
                return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
            }
            retval = MHal_H264_SetFilePlay(mvdBuf.u32BufferAddr, mvdBuf.u32BufferAddr+mvdBuf.u32BufferSize);
            MVD_PRINT("ioctl: H264 Set file play done\n");
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return retval;
        }
        break;
    case H264_IOC_SET_FILE_CLOSE:
        {
            MVD_PRINT("ioctl: H264 Set file close\n");
            //MVD_PLAY_MODE_Tis;
            retval = MHal_H264_SetFileClose();
            MVD_PRINT("ioctl: H264 Set file close done\n");
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return retval;
        }
        break;
	case H264_IOC_SET_FILE_PLAY2:
        {
            MVD_PRINT("ioctl: H264 Set file play2\n");
            //MVD_PLAY_MODE_Tis;
            if( __get_user(H264_PLAY_MODE_Tis.u8PlayFrameRate, &(((H264_PLAY_MODE_T __user *)arg)->u8PlayFrameRate)) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
                return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
            }
            if( __get_user(H264_PLAY_MODE_Tis.u8Mode, &(((H264_PLAY_MODE_T __user *)arg)->u8Mode)) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
                return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
            }
            //retval = MHal_H264_SetFilePlay2(MVD_PLAY_MODE_Tis.u8PlayFrameRate, MVD_PLAY_MODE_Tis.u8Mode);
            MVD_PRINT("ioctl: H264 Set file play2 %d %d done\n", H264_PLAY_MODE_Tis.u8PlayFrameRate, H264_PLAY_MODE_Tis.u8Mode);
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return retval;
        }
        break;
    case H264_IOC_SET_PVR_MODE:
        {
            MVD_PRINT("ioctl: H264 set PVR mode\n");
            if( __get_user(H264_PVR_MODE_Tis.PlayMode, &(((H264_PVR_MODE_T __user *)arg)->PlayMode)) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
                return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
            }
            if( __get_user(H264_PVR_MODE_Tis.u8FrameRateUnit, &(((H264_PVR_MODE_T __user *)arg)->u8FrameRateUnit)) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
                return -EFAULT;	// check the return value of __put_user/__get_user (dreamer@lge.com)
            }
            retval = MHal_H264_PVRPlayMode(H264_PVR_MODE_Tis.PlayMode, H264_PVR_MODE_Tis.u8FrameRateUnit);//H.264 update 090812
            MVD_PRINT("ioctl: H264 set PVR mode %d %d done\n", H264_PVR_MODE_Tis.PlayMode, H264_PVR_MODE_Tis.u8FrameRateUnit);
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return retval;
        }
        break;
    case H264_IOC_LOCAL_PLAY_INIT:
        {
            U32 u32BitStreamSize = (U32)arg;
        	MVD_PRINT("ioctl: H264 Init local play %d\n", u32BitStreamSize);
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return 0;//MDrv_H264_Stream_Download_Init(u32BitStreamSize);
        }
        break;
    case H264_IOC_LOCAL_PLAY_CLOSE:
        {
        	MVD_PRINT("ioctl: H264 STOP local play\n");
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return 0;//MDrv_H264_Stream_Download_Close();
        }
        break;
    case H264_IOC_LOCAL_PLAY_DOWNLOADING:
        {
        	MVD_PRINT("ioctl: H264 download bit-stream for local play\n");
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return 0;//MDrv_H264_Stream_downloading();
        }
        break;
    case H264_IOC_LOCAL_PLAY_DOWNLOADEND:
        {
            U32 u32RestBitStreamSize = (U32)arg;
        	MVD_PRINT("ioctl: H264 download end of bit-stream for local play [%d]\n",u32RestBitStreamSize);
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return 0;//MDrv_H264_Stream_DownloadEnd(u32RestBitStreamSize);
        }
        break;
    case H264_IOC_CLEAN_BIT_STREAM_BUFF:
        {
            MVD_PRINT("ioctl: H264 clean bit-stream buffer to zero\n");
            retval = MDrv_H264_CleanBitStreamBuff();
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));

        	return retval;
        }
        break;
    case H264_IOC_GET_VIDEO_REPEAT:
//jmkim        MVD_PRINT("ioctl: MVD get video repeat\n");
        retval = MHal_H264_GetVideoRepeat();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;

        break;
    case H264_IOC_GET_VIDEO_SKIP:
//jmkim        MVD_PRINT("ioctl: MVD get video skip\n");

        retval=MHal_H264_GetVideoSkip();
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;


        break;
    case H264_IOC_GET_PTS:
	{
		H264_PTS pts;
		U32 pu32PTSLow = 0;
		U32	pu32PTSHigh = 0;
//jmkim        MVD_PRINT("ioctl: MVD get PTS\n");
        retval = MHal_H264_GetPTS(&pu32PTSLow, &pu32PTSHigh);
		pts.u32High = pu32PTSHigh;
		pts.u32Low  = pu32PTSLow;
        __copy_to_user((AVCH264_FRAMEINFO __user *)arg, &pts, sizeof(H264_PTS));
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    }
    case H264_IOC_GET_FRAMEBUF:
        MVD_PRINT("ioctl: H264 get framebuf\n");
        retval = MHal_H264_GetFrameBuffer(&(mvdBuf.u32BufferAddr), &(mvdBuf.u32BufferSize));
        mvdBuf.u32BufferAddr += DefSizeOfPictureData;
        __copy_to_user((H264_BUFFER_INFO __user *)arg, &mvdBuf, sizeof(H264_BUFFER_INFO));
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
	case H264_IOC_SET_DATA_MEM:
    	{
            pH264DataMemSerNumb = (U32*)HVD_ALIGN_4(ioremap(arg, HVD_ISR_SHAREDATA_ENTRY_SIZE * 1024));
            pH264DataMem = (U8*)((U32)pH264DataMemSerNumb +20);
            MVD_PRINT("ioctl: H264 set data memory : Paddr:0x%x Vaddr:0x%x\n", (U32)arg , (U32)pH264DataMem );
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return H264STATUS_SUCCESS;
        }
        break;
    case H264_IOC_SET_DELAY:
    	{
    		U32 u32Delay = (U32)arg;
        	MVD_PRINT("ioctl: H264 set delay time(ms): %d\n", u32Delay);
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return MHal_H264_SetDelay(u32Delay);
        }
        break;
#if defined(UTOPIA_HVD_DRIVER)
    case H264_IOC_STEP_DISPLAY:
        {
            U32 u32Ret= MHal_H264_StepDisplay();
            MVD_PRINT("ioctl: H264 step display:%d\n", u32Ret);
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            return u32Ret;
        }
    case H264_IOC_GET_DISP_QUEUE_SIZE:
        {
            U32 u32Qsize = MHal_H264_GetDispQueueSize();
        	MVD_PRINT("ioctl: H264 get queue size: %d\n", u32Qsize);
            __copy_to_user((U32 __user *)arg, &u32Qsize, sizeof(U32));
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            break;
        }
    case H264_IOC_GET_ESDATASIZE:
        {
            U32 u32Qsize = MHal_H264_GetESDataSize();
        	//MVD_PRINT("ioctl: H264 get ES data size: %d\n", u32Qsize);
            __copy_to_user((U32 __user *)arg, &u32Qsize, sizeof(U32));
            PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
            break;
        }
    case H264_IOC_SET_SYNCREPEATTH:
    	{
    		U32 RepeatTH = (U32)arg;
        	MVD_PRINT("ioctl: H264 set sync repead threashold: %d\n", RepeatTH);
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return MHal_H264_SetSyncRepeatTH(RepeatTH);
        }
        break;
    case H264_IOC_SET_SYNC_THRESHOLD:
    	{
    		U32 syncTH = (U32)arg;
        	MVD_PRINT("ioctl: H264 set sync threashold: %d\n", syncTH);
        	PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        	return MHal_H264_SetSyncThreshold(syncTH);
        }
        break;
    case H264_IOC_GET_DECODER_STATUS:
//jmkim        MVD_PRINT("ioctl: MVD get sync status\n");
        retval = MHal_H264_GetDecoderStatus(&decoderstatus);
        if (__copy_to_user((H264_Decoder_Status __user *)arg, &decoderstatus, sizeof(H264_Decoder_Status)))
            MVD_PRINT("copy MHal_H264_GetDecoderStatus failed\n");
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return retval;
        break;
    case H264_IOC_SET_RESTART_DECODER:
        MVD_PRINT("ioctl: H264 restart decoder\n");
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return MHal_H264_RestartDecoder();
        break;
#endif
    default:
        //MVD_WARNING("ioctl: unknown command\n");
        PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
        return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_H264, _IOC_NR(cmd));
    return 0;
}

S32 MDrv_H264_Open(struct inode *inode, struct file *filp)
{
#if defined(UTOPIA_HVD_DRIVER)
    //printk("*****  Start  ******\n\n\n\n");
    //printk( "cap %d %d %d\n" , MDrv_HVD_GetCaps(0) , MDrv_HVD_GetCaps(1) , MDrv_HVD_GetCaps(2) );
    //printk("\n\n\n*****  End   ******\n\n\n\n");
    MHal_H264_AllocBuf();
#endif
	return 0;

}

S32 MDrv_H264_Release(struct inode *inode, struct file *filp)
{
	return 0;

}

#if HVD_ENABLE_ISR_POLL
extern inline BOOL MHal_H264_IsUnProcessedEntry( void );
static unsigned int MDrv_H264_poll(struct file *filp, poll_table *wait)
{
    //printk("*");
    poll_wait(filp, &gHVD_wait_queue,  wait);
    if( MHal_H264_IsUnProcessedEntry()  )
    {
        //printk("^");
        return POLLIN ;//| POLLOUT;//POLLIN;
    }
    else
    {
        //printk("@");
        return 0;
    }
    //return (MHal_H264_IsUnProcessedEntry()?POLLIN:0);
}
#endif
// mmap implementation for callback data to user mode adaptation layer

void MDrv_H264_vma_open(struct vm_area_struct *vma)
{
    return;
}

void MDrv_H264_vma_close(struct vm_area_struct *vma)
{
    return;
}
#if 0
struct page *MDrv_H264_vma_nopage(struct vm_area_struct *vma,
                                unsigned long address, int *type)
{
    return 0;
}
#endif
struct vm_operations_struct h264_vm_ops = {
	.open =     MDrv_H264_vma_open,
	.close =    MDrv_H264_vma_close,
//	.nopage =   MDrv_H264_vma_nopage,
};

S32 MDrv_H264__mmap(struct file *filp, struct vm_area_struct *vma)
{
	return 0;
}


static int __init MDrv_H264_ModuleInit(void)
{
    //int         s32Ret;
    dev_t       devno;
    //U32         result;
    int         result;		/* STATIC ANALYSIS(NO_EFFECT) ¼öÁ¤ (dreamer@lge.com)*/

    U32 u32BufferStart = 0;
    U32 u32BufferSize = 0;

    devno = MKDEV(MDRV_MAJOR_H264, MDRV_MINOR_H264);
    result = register_chrdev_region(devno, 1, "drvH264");
    if ( result < 0)
   	{
   		printk(KERN_WARNING "H264: can't get major %d\n", MDRV_MAJOR_H264);
		return result;


   	}



    MHal_H264_GetBitStreamBuffer(&u32BufferStart, &u32BufferSize);

    pTMPDataBitStreamBuff = (U8*)MDrv_SYS_PA2NonCacheSeg((void*)u32BufferStart);//H.264 update 090812

    MVD_PRINT("H264 bit-stream buffer addr 0x%x; size %d\n",(U32)pTMPDataBitStreamBuff, u32BufferSize);

   	cdev_init(&(g_H264_Dev.cdev), &H264_Fops);
	g_H264_Dev.cdev.owner = THIS_MODULE;
	g_H264_Dev.cdev.ops = &H264_Fops;
	result = cdev_add (&(g_H264_Dev.cdev), devno, 1);
	/* Fail gracefully if need be */
	if (result)
		printk(KERN_NOTICE "Error add H264 device");

#if HVD_ENABLE_ISR_POLL
    init_waitqueue_head(&gHVD_wait_queue);
#endif

    return 0;
}

static void __exit MDrv_H264_ModuleExit(void)
{

    //IR_PRINT("%s is invoked\n", __FUNCTION__);
    dev_t devno;
    devno = MKDEV(MDRV_MAJOR_H264, MDRV_MINOR_H264);
    //cdev_del(&IRDev.cDevice);
    unregister_chrdev_region(devno, 1);
}





module_init(MDrv_H264_ModuleInit);
module_exit(MDrv_H264_ModuleExit);

//dhjung LGE
MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("H264 driver");
MODULE_LICENSE("MSTAR");
#endif
///////////////////////////////////////////////////////////////////////
