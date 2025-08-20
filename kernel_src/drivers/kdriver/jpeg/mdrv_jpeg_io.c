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
/// file    drvScaler.c
/// @brief  Scaler Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>

#include "mst_devid.h"
#include "mdrv_types.h"
#include "mhal_jpd.h"
#include "mdrv_jpeg_io.h"
#include "mdrv_jpeg_st.h"
#include "mdrv_jpeg_decoder.h"


#include "mdrv_probe.h"

#define JPEG_PRINT(fmt, args...)        // default OFF for MP3 with Photo, spmarine 080910 ::  printk("[JPEG][%06d]     " fmt, __LINE__, ## args)

struct JPEG_Dev
{
	struct cdev cdev;	  /* Char device structure		*/
};


S32 MDrv_JPEG_Ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg);
S32 MDrv_JPEG_Open(struct inode *inode, struct file *filp);
S32 MDrv_JPEG_Release(struct inode *inode, struct file *filp);

struct JPEG_Dev  g_JPEG_Dev;

struct file_operations JPEG_Fops = {
	.owner =    THIS_MODULE,
	.ioctl =    MDrv_JPEG_Ioctl,
	.open =     MDrv_JPEG_Open,
	.release =  MDrv_JPEG_Release,
};

unsigned int v2p_offset = 0 ;
extern U16 _u16OriginalThumbnail_x_size;
extern U16 _u16OriginalThumbnail_y_size;
void JPEGWaitDone(JPEG_PIC_Info *pInfo)
{
    U16 u16Count = 1000;

    while( u16Count > 0 )
    {
        pInfo->enJPDStatus = MDrv_JPEG_WaitDone();
        switch( pInfo->enJPDStatus )
        {
        case EN_JPD_DECODE_DONE:
            pInfo->u32Width = MDrv_JPEG_get_width();
            pInfo->u32Height = MDrv_JPEG_get_height();
            pInfo->u32OriginalWidth= MDrv_JPEG_get_original_width();
            pInfo->u32OriginalHeight = MDrv_JPEG_get_original_height();
            JPEG_PRINT("EN_JPD_DECODE_DONE\n");
            return;

        case EN_JPD_MRBFL_DONE:
        case EN_JPD_MRBFH_DONE:
        case EN_JPD_DECODE_ERROR:
            pInfo->u32Width = 0;
            pInfo->u32Height = 0;
            JPEG_PRINT("enJPDStatus [%d] \n",pInfo->enJPDStatus);
            return;

        case EN_JPD_DECODING:
        default:
            break;
        }

            msleep(1);
            u16Count --;
            if(u16Count == 0)
            {
                JPEG_PRINT("JPEGWaitDone Timeout\n");
                if  ( MDrv_JPEG_IsProgressive () )
                {
                             MHal_JPD_SW_Pause_Reset();
                             MDrv_JPEG_Finalize();
                             pInfo->u32Width = MDrv_JPEG_get_width();
                             pInfo->u32Height = MDrv_JPEG_get_height();
                             pInfo->u32OriginalWidth= MDrv_JPEG_get_original_width();
                             pInfo->u32OriginalHeight = MDrv_JPEG_get_original_height();
                             pInfo->enJPDStatus = EN_JPD_DECODE_DONE;
                }
                 return;
            }
    }
}
// samuel,20081107
extern void MDrv_SYS_EnableSVDCPU( int enable ) ;
extern void MDrv_SYS_Enable3DCOM( int enable ) ;
S32 MDrv_JPEG_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 err = 0;
    JPEG_Buffer_Info param;
    JPEG_PIC_Info stPicInfo;
    U32 Dataread;
    memset(&stPicInfo,0,sizeof(stPicInfo)); //allen.chang

    if ((JPEG_IOCTL_MAGIC != _IOC_TYPE(cmd)))
    {
        JPEG_PRINT("ioctl: Err Mg= %d  TYPE= %d\n", JPEG_IOCTL_MAGIC, _IOC_TYPE(cmd));
        //return -ENOTTY;
    }

    if ( _IOC_NR(cmd) > JPEG_IOCTL_MAXNR )
    {
        JPEG_PRINT("ioctl: Err NR\n");
        return -ENOTTY;
    }

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
        JPEG_PRINT("ioctl: Err\n");
        return -EFAULT;
    }

    PROBE_IO_ENTRY(MDRV_MAJOR_JPEG, _IOC_NR(cmd));


    switch(cmd)
    {
        case IOCTL_JPEG_INIT:
            #if 0 // Check if we need this in T3!!!
            // samuel,20081107
            MDrv_SYS_EnableSVDCPU(0) ;
            MDrv_SYS_Enable3DCOM(0) ;
            #endif
            err = __copy_from_user( &param, (JPEG_Buffer_Info*)arg, sizeof(JPEG_Buffer_Info));
            JPEG_PRINT("\nioctl Init err=%d\n", err);
        	if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
        		return -EFAULT;
            }
            v2p_offset = param.v2p_offset;
            if( !MDrv_JPEG_SetInitParameter( param.u32ReadBufferAddr,
                                         param.u32ReadBufferSize,
                                         param.u32WriteBufferAddr,
                                         param.u32InterBufferAddr,
                                         param.u32InterBufferSize,
                                         param.u8FileReadEnd) )
            {
                JPEG_PRINT("MDrv_JPEG_SetInitParameter Error.\n");
                PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
                return -1;
            }
            MHal_JPD_SetClock(1);
            PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
            return 0;
            break;

        case IOCTL_JPEG_SETBUFFER:
            err = __copy_from_user( &param, (JPEG_Buffer_Info*)arg, sizeof(JPEG_Buffer_Info));
        	if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        	{
            	PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
        		return -EFAULT;
            }
    		v2p_offset = param.v2p_offset;
            if( !MDrv_JPEG_SetInitParameter( param.u32ReadBufferAddr,
                                         param.u32ReadBufferSize,
                                         param.u32WriteBufferAddr,
                                         param.u32InterBufferAddr,
                                         param.u32InterBufferSize,
                                         param.u8FileReadEnd) )
            {
                JPEG_PRINT("MDrv_JPEG_SetInitParameter Error.\n");
            }
            JPEG_PRINT("ioctl Set Buffer err=%d\n", err);
            break;

        case IOCTL_JPEG_PLAY:

           if( !MDrv_JPEG_constructor(JPEG_TYPE_MAIN) )
            {
               JPEG_PRINT("MDrv_JPEG_constructor Error.\n");
               PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
                return -1;
            }
            if(  !MDrv_JPEG_DecodeHeader() )
            {
                JPEG_PRINT("MDrv_JPEG_DecodeHeader Error.\n");
                PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
                return -1;
            }
            JPEG_PRINT("%dx%d\n", MDrv_JPEG_get_original_width(), MDrv_JPEG_get_original_height());

            if(MDrv_JPEG_IsProgressive())
            {
                JPEG_PRINT("Progressive\n");
                MDrv_JPEG_Progressive_Decode();
            }
            else
            {
                JPEG_PRINT("Baseline\n");
                MDrv_JPEG_StartDecode();
            }

            JPEGWaitDone( &stPicInfo );
            err = copy_to_user((JPEG_PIC_Info*)arg, &stPicInfo, sizeof(JPEG_PIC_Info));
            JPEG_PRINT("Image: W= %d  H= %d err= %d\n",stPicInfo.u32Width, stPicInfo.u32Height, err);
    	    if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
    	    {
    	        PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
    		    return -EFAULT;
            }
            break;

        case IOCTL_JPEG_FEEDDATA_L:
            MHal_JPD_ClearJPDStatus(JPD_EVENT_MRBL_DONE);
            JPEGWaitDone( &stPicInfo );

            err = copy_to_user((JPEG_PIC_Info*)arg, &stPicInfo, sizeof(JPEG_PIC_Info));
            JPEG_PRINT("FeedData L: JPDStatus= %d  err= %d\n", stPicInfo.enJPDStatus, err);
    	    if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
    	    {
    	        PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
    		    return -EFAULT;
            }
            break;

        case IOCTL_JPEG_FEEDDATA_H:
            MHal_JPD_ClearJPDStatus(JPD_EVENT_MRBH_DONE);
            JPEGWaitDone( &stPicInfo );

            err = copy_to_user((JPEG_PIC_Info*)arg, &stPicInfo, sizeof(JPEG_PIC_Info));
            JPEG_PRINT("FeedData H: JPDStatus= %d  err= %d\n", stPicInfo.enJPDStatus, err);
    	    if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
    	    {
    	        PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
    		    return -EFAULT;
            }
            break;

        case IOCTL_JPEG_THUMBNAIL:
            if( MDrv_JPEG_constructor(JPEG_TYPE_THUMBNAIL) )
            {
                U16 u16Count = 3000;
                if(  !MDrv_JPEG_DecodeHeader() )
                {
                    JPEG_PRINT("MDrv_JPEG_DecodeHeader Error.\n");
                    PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
                    return -1;
                }
                MDrv_JPEG_StartDecode();

                while( u16Count > 0 )
                {
                    if( MDrv_JPEG_WaitDone() )
                    {
                        JPEG_PRINT("Done\n");
                        break;
                    }
                    msleep(1);
                    if((u16Count--) == 0)
                    {
                        JPEG_PRINT("IOCTL_JPEG_THUMBNAIL Timeout\n");
                        break;
                    }
                }

                stPicInfo.u32Width = MDrv_JPEG_get_width();
                stPicInfo.u32Height = MDrv_JPEG_get_height();

                //allen.chang 2009/09/30
                stPicInfo.u32OriginalWidth =  _u16OriginalThumbnail_x_size;
                stPicInfo.u32OriginalHeight = _u16OriginalThumbnail_y_size;

                err = copy_to_user((JPEG_PIC_Info*)arg, &stPicInfo, sizeof(JPEG_PIC_Info));

                JPEG_PRINT("Thumbnail: W= %d  H= %d err= %d\n",stPicInfo.u32Width, stPicInfo.u32Height, err);


                //allen.chang 2009/09/30
                JPEG_PRINT(" Thumbnail: Original W=[%d] H=[%d] err= %d\n",stPicInfo.u32OriginalWidth, stPicInfo.u32OriginalHeight, err);

        	    if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        	    {
        	        PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
        		    return -EFAULT;
                }
            }
            else
            {
                stPicInfo.u32Width = 0;
                stPicInfo.u32Height = 0;
                err = copy_to_user((JPEG_PIC_Info*)arg, &stPicInfo, sizeof(JPEG_PIC_Info));
                JPEG_PRINT("Thumbnail: W= %d  H= %d err= %d\n",stPicInfo.u32Width, stPicInfo.u32Height, err);
        	    if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        	    {
        	        PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
        		    return -EFAULT;
                }
            }
            break;

        case IOCTL_JPEG_WAKEUP:
            err = __copy_from_user( &Dataread, (U32*)arg, sizeof(U32));
        	if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
        		return -EFAULT;
            }
            JPEG_PRINT("IOCTL_JPEG_WAKEUP %x\n",Dataread);
            MDrv_JPEG_Wakeup(Dataread);
            break;

        case IOCTL_JPEG_SET_MAX_SIZE:
        {
            JPEG_SIZE size;
            err = __copy_from_user( &size, (U32*)arg, sizeof(JPEG_SIZE));
        	if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
        		return -EFAULT;
            }
            JPEG_PRINT("IOCTL_JPEG_SET_MAX_SIZE\n");
            MDrv_JPEG_SetMaxSize(size.progressive_width, size.progressive_height, size.baseline_width, size.baseline_height);
        }
            break;

#if 0 //MIU is configured in system layer
        case IOCTL_JPEG_SET_MIU:
        {
            S32 miu_no;
            err = __copy_from_user( &miu_no, (U32*)arg, sizeof(S32));
        	if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        		return -EFAULT;
            JPEG_PRINT("IOCTL_JPEG_SET_MIU\n");
            MDrv_JPEG_SetMIU(miu_no);
        }
            break;
#endif

        case IOCTL_JPEG_SET_PWR_ON:
            JPEG_PRINT("IOCTL_JPEG_SET_PWR_ON\n");
            MDrv_JPEG_SetPowerOnOff(1);
            break;
        case IOCTL_JPEG_SET_PWR_OFF:
            JPEG_PRINT("IOCTL_JPEG_SET_PWR_OFF\n");
            MDrv_JPEG_SetPowerOnOff(0);
            break;

        default:
            JPEG_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
            return -ENOTTY;
    }
    PROBE_IO_EXIT(MDRV_MAJOR_JPEG, _IOC_NR(cmd));
    return 0;
}


S32 MDrv_JPEG_Open(struct inode *inode, struct file *filp)
{
	return 0;
}


S32 MDrv_JPEG_Release(struct inode *inode, struct file *filp)
{
	return 0;
}


static int __init MDrv_JPEG_Init(void)
{
    dev_t   dev;
    //U32     result;
    int     result;		/* STATIC ANALYSIS(NO_EFFECT) ¼öÁ¤ (dreamer@lge.com)*/

    JPEG_PRINT("JPEG init\n");

    dev = MKDEV(MDRV_MAJOR_JPEG, MDRV_MINOR_JPEG);
    result = register_chrdev_region(dev, 1, "drvJPEG");

    if ( result < 0)
   	{
   		JPEG_PRINT("JPEG: can't get major %d\n", MDRV_MAJOR_JPEG);
		return result;
   	}

   	cdev_init(&(g_JPEG_Dev.cdev), &JPEG_Fops);
	g_JPEG_Dev.cdev.owner = THIS_MODULE;
	g_JPEG_Dev.cdev.ops = &JPEG_Fops;
	result = cdev_add (&(g_JPEG_Dev.cdev), dev, 1);
	/* Fail gracefully if need be */
	if (result)
		JPEG_PRINT(KERN_NOTICE "Error add JPEG device");

    MHal_JPD_Initialize();

    return 0;
}


static void __exit MDrv_JPEG_Exit(void)
{
    dev_t dev;

    JPEG_PRINT("JPEG exit\n");
    cdev_del(&g_JPEG_Dev.cdev);
    dev = MKDEV(MDRV_MAJOR_JPEG, MDRV_MINOR_JPEG);
    unregister_chrdev_region(dev, 1);
}


module_init(MDrv_JPEG_Init);
module_exit(MDrv_JPEG_Exit);




