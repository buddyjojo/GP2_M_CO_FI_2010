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

///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_vd.c
/// This file contains the Mstar video decoder driver
/// @author MStar Semiconductor Inc.
/// @brief  VD
///////////////////////////////////////////////////////////////////////////////




#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/wait.h>
#include <asm/page.h>
#include <asm/atomic.h>
#include <asm/bitops.h>

#include "mdrv_vd.h"
#include "common_vd_singal_def.h"
#include "hal_vd_types.h"
#include "hal_vd_settings.h"
#include "mdrv_vd_states_thread.h"


#include "mst_devid.h"
#include "mdrv_probe.h"

#define VD_REG_DEBUG
#ifdef  VD_REG_DEBUG

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                            BUG();\
                        }\
                    } while (0)

#define VD_ASSERT(arg)                  assert((arg))
#define VD_KDBG(x1, args...)            printk(KERN_WARNING x1, ## args)
#else
#define VD_KDBG(x1)
#define VD_ASSERT(arg)
#endif

// -----------------------------------------------
//
// extern functions define
//


// -----------------------------------------------
//
// Local function prototypes.
//
static int _MDrv_VD_Open(struct inode *inode, struct file *filp);
static int _MDrv_VD_Release(struct inode *inode, struct file *filp);
static int _MDrv_VD_IOCtl(struct inode *inode, struct file *filp, U32 u32CmdVD, unsigned long u32ArgVD);

// -----------------------------------------------
//
// golbal variable.
//
#ifdef LOCK3DSPEEDUP
extern B16 _bComb10Bit3Flag;
extern B16 _bStartCheckCombSpeedup;
#endif

/* PLL Tracking Speed up */
#ifdef PLL_TRACKING_SPEEDUP
extern B16  _bPLLTrigerTrack;
#endif

// -----------------------------------------------
//
// vd device variable.
//
/* Our various sub-devices. */
static struct file_operations vd_ops = {
	.owner      = THIS_MODULE,
	.open       = _MDrv_VD_Open,
	.release    = _MDrv_VD_Release,
	.ioctl      = _MDrv_VD_IOCtl,
};

/* device id */
static U32  _u32majorVD    = 160;
module_param(_u32majorVD, int, 0);

/* VD device cdev */
static struct cdev _stVDDevsVD[1];

// -----------------------------------------------
//
// device functions
//
/* Open vd device */
static int _MDrv_VD_Open(struct inode *inode, struct file *filp)
{
    //MHal_VD_Init();//DSP branch
    //MHal_VD_McuReset();
	return 0;
}

static int _MDrv_VD_IOCtl(struct inode *inode, struct file *filp, U32 u32CmdVD, unsigned long u32ArgVD)
{
    U32     etStandard = E_VIDEOSTANDARD_NOTSTANDARD;
    U32     etSource = E_VIDEOSOURCE_INVALID;
    VIDEO_CAPTUREWINTABLE_TYPE tVideoWindow;

    U32     etEvent = E_NO_EVENT;
    U32     etStates = E_NO_STATES;
    U32     u32HVTotal;
    U32     u32VD_CC_CD;
    U32     u32VDSystem;
    U32     u32Tmp;

    PROBE_IO_ENTRY(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));

    switch(u32CmdVD)
    {
        case IOCTL_VD_GET_ANALOG_STD_SYSTEM:
            u32VDSystem = (U32)MHal_VD_GetDSPVer();
            //printk("Get VD system [%s]\n",(u32VDSystem == VD_SYSTEM_DVB)?"DVB":"NTSC");
            if( copy_to_user((U8*)u32ArgVD, (const U8*)&u32VDSystem, sizeof(U32)) )
            {
                PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                return -EFAULT;
            }
            break;
        case IOCTL_VD_SET_ANALOG_COLOR_SYSTEM:
            //printk("IOCTL_VD_SET_ANALOG_SYSTEM, u32ArgVD %d\n",u32ArgVD);
            MHal_VD_SetColorSystem((VD_ANALOG_COLOR_SYSTEM)u32ArgVD);
            break;
        case IOCTL_VD_LOADDSP://DSP branch
        	MHal_VD_Init(u32ArgVD);
			TIMER_INTERRUPT_PASS1;
			break;

		case IOCTL_VD_EXTERNAL_LOADDSP:	// 20091130
			if(MHal_VD_LoadDsp()>=0)
			{
				VD_KDBG("Load DSP code OK for external source \r\n");
			}
			else
			{
				VD_KDBG("Load DSP code Fail for external source \r\n");
			}				
			break;

        case IOCTL_VD_INIT:
            MDrv_VD_Init();
            break;

        case IOCTL_VD_INIT_COLOR_KILL_BOUND:
            MHal_VD_InitColorKillBound(u32ArgVD);
            break;

        case IOCTL_VD_3DSPEEDUP:
            /*
            I have modified this ioctrl to enable or disable 3D comb speed up issue.
            */
            TIMER_INTERRUPT_STOP;
            if(u32ArgVD==TRUE)
            {
                _bComb10Bit3Flag = TRUE; // Enable 3D comb speed up.
                _bStartCheckCombSpeedup = FALSE; // Release time monitor for 3D comb speed up.
                /* PLL Tracking Speed up */
                _bPLLTrigerTrack = TRUE; // Enable PLL tracking speed up.
            }
            else
            {
                MHal_VD_3DCombSpeedup(FALSE); // Disable 3D comb speed up.
                _bComb10Bit3Flag = FALSE;
                _bStartCheckCombSpeedup = FALSE; // Release time monitor for 3D comb speed up.
            }
            TIMER_INTERRUPT_PASS;
            break;

        case IOCTL_VD_GET_WINDOWS:
            etStandard = atomic_read(&_aVideoStandard);
            if(etStandard<=E_VIDEOSTANDARD_NOTSTANDARD)
            {
				tVideoWindow.eVideoStandard = etStandard;
                tVideoWindow.u16SRHStart = _tVideoCaptureWinTblVD[etStandard].u16SRHStart;
                tVideoWindow.u16SRVStart = _tVideoCaptureWinTblVD[etStandard].u16SRVStart;
                tVideoWindow.u16HRange = _tVideoCaptureWinTblVD[etStandard].u16HRange;
                tVideoWindow.u16VRange = _tVideoCaptureWinTblVD[etStandard].u16VRange;

                if( copy_to_user((U8*)u32ArgVD, (const U8*)&tVideoWindow, sizeof(VIDEO_CAPTUREWINTABLE_TYPE)) )
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                    return -EFAULT;
                }
            }

            break;

        case IOCTL_VD_GET_WINDOWS_SVIDEO:
            etStandard = atomic_read(&_aVideoStandard);
            if(etStandard<=E_VIDEOSTANDARD_NOTSTANDARD)
            {
				tVideoWindow.eVideoStandard = etStandard;
                tVideoWindow.u16SRHStart = _tVideoCaptureWinTblVD_SVIDEO[etStandard].u16SRHStart;
                tVideoWindow.u16SRVStart = _tVideoCaptureWinTblVD_SVIDEO[etStandard].u16SRVStart;
                tVideoWindow.u16HRange = _tVideoCaptureWinTblVD_SVIDEO[etStandard].u16HRange;
                tVideoWindow.u16VRange = _tVideoCaptureWinTblVD_SVIDEO[etStandard].u16VRange;

                if( copy_to_user((U8*)u32ArgVD, (const U8*)&tVideoWindow, sizeof(VIDEO_CAPTUREWINTABLE_TYPE)) )
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                    return -EFAULT;
                }
            }

            break;

        case IOCTL_VD_GET_CURRENT_VIDEO_STANDARD:
            etStandard = atomic_read(&_aVideoStandard);
            if(copy_to_user((U32*)u32ArgVD, (const U32*)&etStandard, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                return -EFAULT;
            }
            break;

        case IOCTL_VD_GET_CURRENT_VIDEO_SOURCE:
            etSource = atomic_read(&_aVideoSource);
            if(copy_to_user((U32*)u32ArgVD, (const U32*)&etSource, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                return -EFAULT;
            }
            break;

        case IOCTL_VD_SET_VIDEO_SOURCE:
            MDrv_VD_SetVideoSource(u32ArgVD);
            break;

        case IOCTL_VD_THREAD_HALT:
            atomic_set(&_aExitThreadVD, DISABLE);
            atomic_set(&_aReleaseThreadVD, ENABLE);
            _MDrv_VD_DeleteTimer();
            _bThreadStates(clear);
            break;

        case IOCTL_VD_THREAD_START:
            atomic_set(&_aExitThreadVD, DISABLE);
            atomic_set(&_aReleaseThreadVD, DISABLE);
            _MDrv_VD_StartTimer();
            _bThreadStates(set);
            break;

        case IOCTL_VD_WAIT_EVENT:
            etEvent = E_NO_EVENT;
            if(_MDrv_VD_Wait_Event() == TRUE)
                etEvent = _MDrv_VD_GetEvent();
            else
                etEvent = E_THREAD_STOP;

            if(copy_to_user((U32*)u32ArgVD, (const U32*)&etEvent, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                return -EFAULT;
            }
            break;

        case IOCTL_VD_MCU_RUN:
            if(u32ArgVD == FALSE)
                MHal_VD_McuReset_Stop();
            else
                MHal_VD_McuReset();
            break;

        case IOCTL_VD_GET_STATES:
            etStates = _MDrv_VD_GetStates();
            if(copy_to_user((U32*)u32ArgVD, (const U32*)&etStates, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                return -EFAULT;
            }
            break;

        case IOCTL_VD_GET_HV_TOTAL:
            etStandard = atomic_read(&_aVideoStandard);
            if(etStandard!=E_VIDEOSTANDARD_NOTSTANDARD)
                u32HVTotal = (_MHal_VD_GetOutputHT() << 16) | _MHal_VD_GetOutputVT();
            else
                u32HVTotal = 0;
            if(copy_to_user((U32*)u32ArgVD, (const U32*)&u32HVTotal, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                return -EFAULT;
            }
            break;

        case IOCTL_VD_HSYN_DETECTION:
            /* stop timer interrupt */
            if(u32ArgVD)
            {
                TIMER_INTERRUPT_STOP1;
                VD_KDBG("STOP VD THREAD\n");
            }
            else
            {
                VD_KDBG("PASS VD THREAD\n");
                TIMER_INTERRUPT_PASS1;
            }
            MHal_VD_SetHsyncDetectionForTuning(u32ArgVD);
            break;

        case IOCTL_VD_CHECKSTATES:
            u32VD_CC_CD = MHal_VD_CheckStatusLoop();
            if(copy_to_user((U32*)u32ArgVD, (const U32*)&u32VD_CC_CD, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                return -EFAULT;
            }
            break;

        case IOCTL_VD_3DCOMB:
            {
                if(copy_from_user((U32*)&u32Tmp, (U32*)u32ArgVD, sizeof(U32)))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
                    return -EFAULT;
                }
                TIMER_INTERRUPT_STOP;
                MHal_VD_Set3dComb(u32Tmp);
                TIMER_INTERRUPT_PASS;
            }
            break;

		case IOCTL_VD_RESET_AGC:
			TIMER_INTERRUPT_STOP;
			MHal_VD_ResetAGC();
			TIMER_INTERRUPT_PASS;
			break;

		// shjang_091020
		case IOCTL_VD_SWING_THRESHOLD:
			TIMER_INTERRUPT_STOP;
			MHal_VD_SetSigSWingThreshold(u32ArgVD);
			TIMER_INTERRUPT_PASS;
			break;			

		//lachesis_100118
		case IOCTL_VD_SET_ADAPTIVE_SLICE:
			TIMER_INTERRUPT_STOP;
			MHal_VD_SetAdaptiveHsyncSlice(u32ArgVD);
			TIMER_INTERRUPT_PASS;
			break;

		case IOCTL_VD_SET_ADAPTIVE_TRACKING:
			TIMER_INTERRUPT_STOP;
			MHal_VD_SetAdaptiveHsyncTracking(u32ArgVD);
			TIMER_INTERRUPT_PASS;
			break;
			
        case IOCTL_VD_EXIT:  //20090826EL
            MDrv_VD_Exit();
            break;

        case IOCTL_VD_POWER_ON:  //20090826EL
            //printk("VD_POWER_ON\r\n");
            MHal_VD_SetClock(TRUE);
            break;

        case IOCTL_VD_POWER_OFF:  //20090826EL
            //printk("VD_POWER_OFF\r\n");
            MHal_VD_SetClock(FALSE);
            break;

		// shjang_100322 manual H pll speed control
		case IOCTL_VD_HPLL_TRACKING:
			MHal_VD_ManualHPllTrackingControl(u32ArgVD);
			break;
			
		case IOCTL_VD_SET_HSYNC_WIDTH_DET:
			TIMER_INTERRUPT_STOP;
			MHal_VD_SetHsyncWidthDetection(u32ArgVD);
			TIMER_INTERRUPT_PASS;
			break;

        default:
            PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
            return -ENOTTY;

    }
    PROBE_IO_EXIT(MDRV_MAJOR_VD, _IOC_NR(u32CmdVD));
	return 0;
}

/* Release vd device */
static int _MDrv_VD_Release(struct inode *inode, struct file *filp)
{
    VD_KDBG("release %d\n", _u32majorVD);
	return 0;
}

/* Set up the cdev structure for vd device.  */
static void _MDrv_VD_SetupCDev(struct cdev *dev, int minor,
		struct file_operations *fops)
{
	int err, devno = MKDEV(_u32majorVD, minor);

	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add (dev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		VD_KDBG("Error %d adding simple%d", err, minor);
}

/* Module housekeeping. */
static int _MDrv_VD_Init(void)
{
	int result;
	dev_t dev = MKDEV(_u32majorVD, 0);

	/* Figure out our device number. */
	if (_u32majorVD)
		result = register_chrdev_region(dev, 1, "VD");
	else {
		result = alloc_chrdev_region(&dev, 0, 1, "VD");
		_u32majorVD = MAJOR(dev);
	}
	if (result < 0) {
		VD_KDBG("unable to get major %d\n", _u32majorVD);
		return result;
	}
	if (_u32majorVD == 0)
		_u32majorVD = result;

	/* Now set up two cdevs. */
	_MDrv_VD_SetupCDev(_stVDDevsVD, 0, &vd_ops);

    VD_KDBG("init %d\n", _u32majorVD);
	return 0;
}

static void _MDrv_VD_Cleanup(void)
{
	cdev_del(_stVDDevsVD);
	unregister_chrdev_region(MKDEV(_u32majorVD, 0), 2);
}

// -----------------------------------------------
//
// vd device variable.
//
module_init(_MDrv_VD_Init);
module_exit(_MDrv_VD_Cleanup);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("VD driver");
MODULE_LICENSE("MSTAR");

