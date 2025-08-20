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

#include "mst_devid.h"
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mhal_scaler.h"
#include "mdrv_scaler_io.h"
#include "mdrv_scaler_st.h"
#include "mdrv_scaler.h"
#include "mdrv_scaler_pcmode.h"
#include "mdrv_hdmi.h"
#include "mdrv_mace.h"
#include "mdrv_qmap.h"//victor 20081210, QMap
#include "mdrv_adc.h" // daniel.huang 20090615
#include "mdrv_tcon.h"
#include "Board.h"


#include "mdrv_probe.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define SCALER_WARNING(fmt, args...)       printk(KERN_WARNING "[SCALER][%06d]     " fmt, __LINE__, ## args)
#define SCALER_PRINT(fmt, args...)         printk("[SCALER][%06d]     " fmt, __LINE__, ## args)

#define OPT_IO_DGB 0
#if OPT_IO_DGB
#define SC_IO_DEBUG(fmt, args...)           printk("\033[47;34m[IO][%05d] " fmt "\033[0m", __LINE__, ## args)
#else
#define SC_IO_DEBUG(fmt, args...)
#endif


#define SC_MMIO_DBG             0
#if (SC_MMIO_DBG)
#define SC_MMIO_PRINT(fmt, args...)      printk("[MDrv_SC_IO][%05d] " fmt, __LINE__, ## args)
#else
#define SC_MMIO_PRINT(fmt, args...)
#endif

#define MOD_SCALER_DEVICE_COUNT    1
#define MOD_SCALER_NAME            "ModScaler"

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32ScalerMajor;
    int                         s32ScalerMinor;
    struct cdev                 cDevice;
    struct file_operations      ScalerFop;
    struct SC_DRIVER_CONTEXT_s  DrvCtx;

#if ENABLE_SC_MMIO
  	int vmas;                 /* active mappings counter*///[20090920 Shawn] MM I/O
#endif

} Scaler_Dev_t;

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
static int __init   _mod_scaler_init(void);
static void __exit  _mod_scaler_exit(void);

static int          _mod_scaler_open (struct inode *inode, struct file *filp);
static int          _mod_scaler_release(struct inode *inode, struct file *filp);
static ssize_t      _mod_scaler_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t      _mod_scaler_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static int          _mod_scaler_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

#if ENABLE_SC_MMIO //[20090920 Shawn] MM I/O
#include <linux/mm.h>
static int MDrv_SC_MMAP(struct file *filp, struct vm_area_struct *vma);
static U8           *_pu8SCBuf = NULL;
#endif

//--------------------------------------------------------------------------------------------------
//  Extern functions
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
static Scaler_Dev_t _scaler_dev =
{
    .s32ScalerMajor =           MDRV_MAJOR_SCALER,
    .s32ScalerMinor =           MDRV_MINOR_SCALER,
    .cDevice     =
    {
        .kobj    =              {.name= MOD_SCALER_NAME, },
        .owner   =              THIS_MODULE,
    },
    .ScalerFop   =
    {
        .open    =              _mod_scaler_open,
        .release =              _mod_scaler_release,
        .read    =              _mod_scaler_read,
        .write   =              _mod_scaler_write,
        .ioctl   =              _mod_scaler_ioctl,
#if ENABLE_SC_MMIO
	.mmap =	    MDrv_SC_MMAP,//[20090920 Shawn] MM I/O
#endif
    },
};

//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------

/* Add extern function to avoid compilation warning msg. by LGE. jjab */
extern void MDrv_SC_PCMode_GetInfo_AutoTune(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
extern void MDrv_SC_Phaseadjust(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
extern void MDrv_SC_MACE_SetIHCRegionTable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
extern void MDrv_SC_MACE_SetICCYModelEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);
extern void MDrv_SC_MACE_SetIHCYModelEnable(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg);

//--------------------------------------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------------------------------------
static int __init _mod_scaler_init(void)
{
    int     s32Ret;
    dev_t   dev;

    SCALER_PRINT("_mod_scaler_init\n");
    //kernel_thread(mythread, NULL, CLONE_KERNEL);

    if (_scaler_dev.s32ScalerMajor)
    {
        dev = MKDEV(_scaler_dev.s32ScalerMajor, _scaler_dev.s32ScalerMinor);
        s32Ret = register_chrdev_region(dev, MOD_SCALER_DEVICE_COUNT, MOD_SCALER_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, _scaler_dev.s32ScalerMinor, MOD_SCALER_DEVICE_COUNT, MOD_SCALER_NAME);
        _scaler_dev.s32ScalerMajor = MAJOR(dev);
    }

    if (0 > s32Ret)
    {
        SCALER_WARNING("Unable to get major %d\n", _scaler_dev.s32ScalerMajor);
        return s32Ret;
    }

    cdev_init(&_scaler_dev.cDevice, &_scaler_dev.ScalerFop);
    if (0 != (s32Ret = cdev_add(&_scaler_dev.cDevice, dev, MOD_SCALER_DEVICE_COUNT)))
    {
        SCALER_WARNING("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_SCALER_DEVICE_COUNT);
        return s32Ret;
    }

    return 0;
}

static void __exit _mod_scaler_exit(void)
{
    cdev_del(&_scaler_dev.cDevice);
    unregister_chrdev_region(MKDEV(_scaler_dev.s32ScalerMajor, _scaler_dev.s32ScalerMinor), MOD_SCALER_DEVICE_COUNT);
}

static int _mod_scaler_open(struct inode *inode, struct file *filp)
{
    Scaler_Dev_t* pdev;
    PSC_DRIVER_CONTEXT_t pDrvCtx;

    pdev = container_of(inode->i_cdev, Scaler_Dev_t, cDevice);
    filp->private_data = pdev;
    pDrvCtx = &pdev->DrvCtx;

    MDrv_SC_InitDrvCtx(pDrvCtx);

    SCALER_PRINT("_mod_scaler_open\n");

#if ENABLE_SC_MMIO
    _pu8SCBuf = NULL;//[20090920 Shawn] MM I/O
#endif

    return 0;
}

static int _mod_scaler_release(struct inode *inode, struct file *filp)
{
    Scaler_Dev_t* pdev = filp->private_data;
    PSC_DRIVER_CONTEXT_t pDrvCtx = &pdev->DrvCtx;

    MDrv_SC_Cleanup(pDrvCtx);

#if ENABLE_SC_MMIO  //[20090920 Shawn] MM I/O
    if(pdev->vmas == 0)
    {
        _pu8SCBuf = NULL;
    }
#endif

    return 0;
}

static ssize_t _mod_scaler_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

static ssize_t _mod_scaler_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

static int _mod_scaler_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err = 0;
    Scaler_Dev_t* pdev = filp->private_data;
    PSC_DRIVER_CONTEXT_t pDrvCtx = &pdev->DrvCtx;

    // extract the type and number bitfields, and don't decode
    // wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
    if (_IOC_TYPE(cmd) != SC_IOCTL_MAGIC)   return -ENOTTY;
    if (_IOC_NR(cmd) > IOCTL_SC_MAXNR)      return -ENOTTY;

    // the direction is a bitmask, and VERIFY_WRITE catches R/W
    // transfers. ‥Type… is user oriented, while
    // access_ok is kernel oriented, so the concept of "read" and
    // "write" is reversed
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;

    PROBE_IO_ENTRY(MDRV_MAJOR_SCALER, _IOC_NR(cmd));

    switch (cmd)
    {
    case IOCTL_SC_INIT:
        SC_IO_DEBUG("IOCTL_SC_INIT\n");
        MDrv_SC_Init(pDrvCtx, arg);
        break;

    case IOCTL_SET_PANELDATA:
        SC_IO_DEBUG("IOCTL_SC_SET_PANELDATA\n");
        MDrv_SetPanelData(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_PANELOUTPUT:
        SC_IO_DEBUG("IOCTRL_SC_SET_PANELOUTPUT\n");
        MDrv_SC_SetPanelOutput(pDrvCtx, (BOOL)arg);
        break;

    case IOCTL_SC_SET_NO_SIGNAL_COLOR:
        SC_IO_DEBUG("IOCTL_SC_SET_NO_SIGNAL_COLOR\n");
        MDrv_SC_SetNoSignalColor(pDrvCtx, arg);
        break;
    case IOCTL_SC_SET_INPUTSOURCE:
        SC_IO_DEBUG("IOCTL_SC_SET_INPUTSOURCE\n");
        MDrv_SC_SetInputSource(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_MVD_SIGNAL_INFO:
        SC_IO_DEBUG("IOCTL_SC_SET_MVD_SIGNAL_INFO\n");
        MDrv_SC_SetMVDSigInfo(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_VD_SIGNAL_INFO:
// drmyung LGE        SC_IO_DEBUG("IOCTL_SC_SET_VD_SIGNAL_INFO\n");
        MDrv_SC_SetVDSigInfo(pDrvCtx, arg);
        break;

	case IOCTL_SC_SET_SC_SIGNAL_INFO: // LGE drmyung 081022
// drmyung LGE		  SC_IO_DEBUG("IOCTL_SC_SET_SC_SIGNAL_INFO\n");
		MDrv_SC_SetSCSigInfo(pDrvCtx, arg);
		break;

    case IOCTL_SC_SET_CAPTURE_WIN:
        SC_IO_DEBUG("IOCTL_SC_SET_CAPTURE_WIN\n");
        MDrv_SC_SetCaptureWin(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_CAPTURE_WIN:
        SC_IO_DEBUG("IOCTL_SC_GET_CAPTURE_WIN\n");
        MDrv_SC_GetCaptureWin(pDrvCtx, arg);
        break;

    //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
    case IOCTL_SC_GET_REAL_CAPTURE_WIN:
        SC_IO_DEBUG("IOCTL_SC_GET_REAL_CAPTURE_WIN\n");
        MDrv_SC_GetRealCaptureWin(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_DISPLAY_WIN:
        SC_IO_DEBUG("IOCTL_SC_SET_DISPLAY_WIN\n");
        MDrv_SC_SetDisplayWin(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_DISPLAY_WIN:
        SC_IO_DEBUG("IOCTL_SC_GET_DISPLAY_WIN\n");
        MDrv_SC_GetDisplayWin(pDrvCtx, arg);
        break;

    case IOCTL_SC_ACTIVE:
        SC_IO_DEBUG("IOCTL_SC_ACTIVE_SOURCE\n");
        MDrv_SC_Active(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_BLACK_SCREEN:
        SC_IO_DEBUG("IOCTL_SC_SET_BLACK_SCREEN\n");
        MDrv_SC_SetBlackScreen(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_FREERUN://lachesis_090831
        SC_IO_DEBUG("IOCTL_SC_SET_FREERUN\n");
        MDrv_SC_SetFreerun(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_BRIGHTNESS:
        //KWON_0602	SC_IO_DEBUG("IOCTL_SC_SET_BRIGHTNESS\n");
        MDrv_SC_SetBrightness(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_FREEZEIMG:
        SC_IO_DEBUG("IOCTL_SC_SET_FREEZEIMG\n");
        MDrv_SC_IPM_SetFreezeImg(pDrvCtx, (BOOL)arg);
        break;

    case IOCTL_SC_SET_FREERUNCOLORENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_FREERUNCOLORENABLE\n");
        MDrv_SC_VOP_SetFreeRunColorEnable(pDrvCtx, (BOOL)arg);
        break;

    case IOCTL_SC_SET_FREERUNCOLOR:
        SC_IO_DEBUG("IOCTL_SC_SET_FREERUNCOLOR\n");
        MDrv_SC_VOP_SetFreeRunColor(pDrvCtx, (SC_FREERUN_COLOR_e)arg);
        break;

	case IOCTL_SC_SET_FRAMERATE:
//drmyung LGE		 SC_IO_DEBUG("IOCTL_SC_SET_FRAMERATE\n");
		MDrv_SC_SetFrameRate(pDrvCtx, arg);
		break;

    case IOCTL_SC_GET_FRAMERATE:
//drmyung LGE        SC_IO_DEBUG("IOCTL_SC_GET_FRAMERATE\n");
        MDrv_SC_GetFrameRate(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_INPUTTIMINGINFO:
//drmyung LGE        SC_IO_DEBUG("IOCTL_SC_GET_INPUTTIMINGINFO\n");
        MDrv_SC_GetInputTimingInfo(pDrvCtx, arg);
        break;

	case IOCTL_SC_GET_HDMIINPUTTIMINGINFO:
//drmyung LGE		 SC_IO_DEBUG("IOCTL_SC_GET_HDMIINPUTTIMINGINFO\n");
		MDrv_SC_GetHDMIInputTimingInfo(pDrvCtx, arg);
		break;
    case IOCTL_SC_SET_GAMMA_TABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_GAMMA_TABLE\n");
        MDrv_SC_SetGammaTable(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_FILM_MODE:
        SC_IO_DEBUG("IOCTL_SC_SET_FILM_MODE\n");
        MDrv_SC_SetFilmMode(pDrvCtx, arg);
        break;
    case IOCTL_SC_SET_FRAMECOLOR:
        SC_IO_DEBUG("IOCTL_SC_SET_FRAMECOLOR\n");
        MDrv_SC_SetFrameColor(arg);
        break;

    case IOCTL_SC_SET_CROPWIN:
        SC_IO_DEBUG("IOCTL_SC_SET_CROPWIN\n");
        MDrv_SC_SetCropWin(pDrvCtx,arg);
        break;
   case IOCTL_SC_GET_CROP_WIN:
        SC_IO_DEBUG("IOCTL_SC_GET_CROP_WIN\n");
        MDrv_SC_GetCropWin(pDrvCtx, arg);
        break;
    // daniel.huang 20090615
    case IOCTL_SC_SET_ADC_SOURCE:
    {
        ADC_SOURCE_TYPE inputsrc_type;
        SC_IO_DEBUG("IOCTL_SC_SET_ADC_SOURCE\n");
        if (copy_from_user(&inputsrc_type, (void __user *)arg, sizeof(ADC_SOURCE_TYPE)))
            break;
        MDrv_ADC_SetSource(inputsrc_type);
        break;
    }
    // daniel.huang 20090615
    case IOCTL_SC_SET_ADC_MUX:
    {
        ADC_MUX_TYPE adcmux_type;
        SC_IO_DEBUG("IOCTL_SC_SET_ADC_MUX\n");
        if (copy_from_user(&adcmux_type, (void __user *)arg, sizeof(ADC_MUX_TYPE)))
            break;
        MDrv_ADC_SetMux(adcmux_type);
        break;
    }
    // daniel.huang 20090630
    case IOCTL_SC_SET_ADC_CVBSO:
    {
        SC_CVBS_OUT_INFO_t cvbs_out_info;
        SC_IO_DEBUG("IOCTL_SC_SET_ADC_CVBSO\n");
        if (copy_from_user(&cvbs_out_info, (void __user *)arg, sizeof(SC_CVBS_OUT_INFO_t)))
            break;
        MDrv_ADC_SetCVBSO(cvbs_out_info.u8PortNum,
                          (ADC_CVBSO_TYPE)cvbs_out_info.u8Param);
        break;
    }
    case IOCTL_SC_SET_ADC_CVBSO_MUX:
    {
        SC_CVBS_OUT_INFO_t cvbs_out_info;
        SC_IO_DEBUG("IOCTL_SC_SET_ADC_CVBSO_MUX\n");
        if (copy_from_user(&cvbs_out_info, (void __user *)arg, sizeof(SC_CVBS_OUT_INFO_t)))
            break;
        MDrv_ADC_SetCVBSO_MUX(cvbs_out_info.u8PortNum,
                              (ADC_CVBSO_MUX_TYPE)cvbs_out_info.u8Param);
        break;
    }
    case IOCTL_SC_GOPSEL:
        SC_IO_DEBUG("IOCTL_SC_GOPSEL\n");
        MDrv_SC_SetGOPSEL(pDrvCtx, arg);
        break;

    //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
    case IOCTL_SC_GOP_SAVE_SETTING:
        SC_IO_DEBUG("IOCTL_SC_GOP_SAVE_SETTING\n");
        MDrv_SC_SaveGOPSetting();
        break;
    case IOCTL_SC_GOP_RESTORE_SETTING:
        SC_IO_DEBUG("IOCTL_SC_GOP_RESTORE_SETTING\n");
        MDrv_SC_RestoreGOPSetting();
        break;

    case IOCTL_SC_SET_GOP_TO_IP:
        SC_IO_DEBUG("IOCTL_SC_SET_GOP_TO_IP\n");
        MDrv_SC_SetGOP_TO_IP(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_GOP_TO_VOP:
        SC_IO_DEBUG("IOCTL_SC_SET_GOP_TO_VOP\n");
        MDrv_SC_SetGOP_TO_VOP(pDrvCtx, arg);
        break;

    case IOCTL_SC_NOTIFY_CHANGED_FMT: // drmyung LGE 080626
        SC_IO_DEBUG("IOCTL_SC_NOTIFY_CHANGED_FMT\n");
        MDrv_SC_NotifyChangedFmt(pDrvCtx, arg);
        break;
    case IOCTL_SC_SET_MVDTYPE:
        SC_IO_DEBUG("IOCTL_SC_SET_MVDTYPE\n");
        MDrv_SC_SetMVDType(pDrvCtx, arg);
        break;
    case IOCTL_SC_HDMI_INIT: // LGE wlgnsl99
        SC_IO_DEBUG("IOCTL_SC_HDMI_INIT\n");
        MDrv_SC_HDMI_Init(pDrvCtx, arg);
        break;

    case IOCTL_SC_HDMI_SETMUX:  // daniel.huang 20090625
        SC_IO_DEBUG("IOCTL_SC_HDMI_SETMUX\n");
        MDrv_HDMI_SetMux(pDrvCtx, arg);
        break;
	case IOCTL_SC_SET_DUPLICATE: //gbtogether 081021
		SC_IO_DEBUG("IOCTL_SC_SET_DUPLICATE\n");
		MDrv_SC_SetDuplicate(pDrvCtx, arg);
		break;

    case IOCTL_SC_SET_PRE_CONTRAST_BRIGHTNESS://victor 20081016, ContrastBrightness
        MDrv_SC_SetPreConBri(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_POST_CONTRAST_BRIGHTNESS://victor 20081016, ContrastBrightness
        MDrv_SC_SetPostConBri(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_BLACKLEVEL://victor 20081106
        MDrv_SC_SetBlackLevel(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_CDNRENABLE://[090615_Leo]
        SC_IO_DEBUG("IOCTL_SC_SET_CDNRENABLE\n");
        MDrv_SC_SetCDNREnable(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_CDNR_INDEX://[090616_Leo]
        SC_IO_DEBUG("IOCTL_SC_SET_CDNR_INDEX\n");
        MDrv_SC_SetCDNRIndex(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_CDNR_GAIN://[090617_Leo]
        SC_IO_DEBUG("IOCTL_SC_SET_CDNR_GAIN\n");
        MDrv_SC_SetCDNRGain(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_AUTO_NR_ENABLE:
        MDrv_SC_SetAutoNREnable(pDrvCtx, arg);
        break;

    //------------------------
    //  PC Mode
    //------------------------
    case IOCTL_SC_SET_MODETABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_MODETABLE\n");
        MDrv_SC_PCMode_SetModeTable(pDrvCtx, arg);
        break;
    case IOCTL_SC_SET_PCMODE_RESOLUTION_INDEX: // swwoo LGE 080626
        SC_IO_DEBUG("IOCTL_SC_SET_PCMODE_RESOLUTION_INDEX\n");
        MDrv_SC_PCMode_SetResolutionIndex(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_PCMODEINFO:
        SC_IO_DEBUG("IOCTL_SC_GET_PCMODEINFO\n");
        MDrv_SC_PCMode_GetInfo(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_PCMODEINFO_AUTOTUNE: // LGE swwoo
        SC_IO_DEBUG("IOCTL_SC_GET_PCMODEINFO_AUTOTUNE\n");
        MDrv_SC_PCMode_GetInfo_AutoTune(pDrvCtx, arg);
        break;
	// shjang_091006 20091006 ykkim5
	case IOCTL_SC_SET_COMP_SYNCLEVEL:
		SC_IO_DEBUG("IOCTL_SC_SET_COMP_SYNCLEVEL\n");
		MDrv_SC_SetCompSyncLevel(pDrvCtx, arg);
		break;
    case IOCTL_SC_SET_PCMODEINFO:
        SC_IO_DEBUG("IOCTL_SC_SET_PCMODEINFO\n");
        MDrv_SC_PCMode_SetInfo(pDrvCtx, arg);
        break;

    case IOCTL_SC_AUTOADJUST:
        SC_IO_DEBUG("IOCTL_SC_AUTOADJUST\n");
        MDrv_SC_AutoAdjust(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_GAIN:  //thchen 20080729
        SC_IO_DEBUG("IOCTL_SC_SET_GAIN\n");
        MDrv_SC_SetGain(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_GAIN:  //thchen 20080729
        SC_IO_DEBUG("IOCTL_SC_GET_GAIN\n");
        MDrv_SC_GetGain(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_OFFSET:  //thchen 20080729
        SC_IO_DEBUG("IOCTL_SC_SET_OFFSET\n");
        MDrv_SC_SetOffset(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_OFFSET:  //thchen 20080729
        SC_IO_DEBUG("IOCTL_SC_GET_OFFSET\n");
        MDrv_SC_GetOffset(pDrvCtx, arg);
        break;

    case IOCTL_SC_PHASEADJUST: // swwoo LGE 080626
        SC_IO_DEBUG("IOCTL_SC_PHASEADJUST\n");
        MDrv_SC_Phaseadjust (pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_ALL_DATA: //ykkim5 091122
     	MDrv_SC_PCMode_GetInfo_FromReg (pDrvCtx, arg);
        break;

    //------------------------
    //  HDMI Mode
    //------------------------
    case IOCTL_SC_GET_HDMI_INFO:
//drmyung LGE        SC_IO_DEBUG("IOCTL_SC_GET_HDMI_INFO\n");
        MDrv_HDMI_GetInfo(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_HDMI_ASPECTRATIO:
        SC_IO_DEBUG("IOCTL_SC_GET_HDMI_ASPECTRATIO\n");
        MDrv_HDMI_GetAspectRatio(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_HDMI_XVYCC:
        SC_IO_DEBUG("IOCTL_SC_GET_HDMI_XVYCC\n");
        MDrv_HDMI_GetxvYCC(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_HDMI_HDCP: // LGE wlgnsl99
        SC_IO_DEBUG("IOCTL_SC_SET_HDMI_HDCP\n");
        MDrv_HDMI_SetHDCP(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_HDMI_COLOR_DOMAIN://victor 20080910
        SC_IO_DEBUG("IOCTL_SC_GET_HDMI_COLOR_DOMAIN\n");
        MDrv_HDMI_GetHDMIColorDomain(pDrvCtx, arg);
        break;

    case IOCTL_SC_IS_HDMI: //victor 20080923
        SC_IO_DEBUG("IOCTL_SC_IS_HDMI\n");
        MDrv_IS_HDMI(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_HDMI_EQ: // 081027 wlgnsl99 LGE : set HDMI EQ
        SC_IO_DEBUG("IOCTL_SC_SET_HDMI_EQ\n");
        MDrv_SC_SetHDMIEQ(pDrvCtx, arg);
        break;
    case IOCTL_SC_ENABLE_DDCBUS: //victor 20081215, DDC
        SC_IO_DEBUG("IOCTL_SC_ENABLE_DDCBUS\n");
        {
        U32 param;
        if (copy_from_user(&param, (void __user *)arg, sizeof(U32)))
        {
            break;
        }
        MHal_HDMI_EnableDDCBus(param);
        }
        break;

	case IOCTL_SC_ENABLE_DVI_CLOCK:
        SC_IO_DEBUG("IOCTL_SC_ENABLE_DVI_CLOCK\n");
		MDrv_HDMI_Ctrl_DviClock(pDrvCtx, arg);
		break;
	//lachesis_090723 HPD control
	case IOCTL_SC_SET_HDMI_HPD:
		MDrv_HDMI_SetHPD(pDrvCtx, arg);
		break;

    //------------------------
    //  Scart Mode
    //------------------------
    case IOCTL_SC_GET_SCART_FB_MODE:
// drmyung LGE        SC_IO_DEBUG("IOCTL_SC_GET_SCART_FB_MODE\n");
        MDrv_Scart_GetFBMode(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_SCART_INPUT_AR:
// drmyung LGE        SC_IO_DEBUG("IOCTL_SC_GET_SCART_INPUT_AR\n");
        if( MDrv_Scart_GetARMode(pDrvCtx, arg) )
        {   // check the return value (dreamer@lge.com)
            PROBE_IO_EXIT(MDRV_MAJOR_SCALER, _IOC_NR(cmd));
            return -EFAULT;
        }
        break;

    case IOCTL_SC_SET_SCART_OVERLAY:
        SC_IO_DEBUG("IOCTL_SC_SET_SCART_OVERLAY\n");
        MDrv_Scart_SetOverlay(pDrvCtx, arg);
        break;

    //------------------------
    //  MACE
    //------------------------
    case IOCTL_SC_SET_YUV2RGB_MTX:
        SC_IO_DEBUG("IOCTL_SC_SET_YUV2RGB_MTX\n");
        MDrv_SC_MACE_SetYUV2RGBMatrix(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_CONTRAST:
        //KWON_0602	SC_IO_DEBUG("IOCTL_SC_SET_CONTRAST\n");
        MDrv_SC_MACE_SetContrast(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_SATURATION:
        //KWON_0530_TEST	SC_IO_DEBUG("IOCTL_SC_SET_SATURATION\n");
        MDrv_SC_MACE_SetSaturation(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_HUE:
        //KWON_0602	SC_IO_DEBUG("IOCTL_SC_SET_HUE\n");
        MDrv_SC_MACE_SetHue(pDrvCtx, arg);
        break;
    case IOCTL_SC_SET_RGB_EX:
        SC_IO_DEBUG("IOCTL_SC_SET_RGB_EX\n");
        MDrv_SC_MACE_SetRGBEx(pDrvCtx, arg);
        break;
    case IOCTL_SC_GET_HISTOGRAM_INFO:
        //KWON_0530_TEST	SC_IO_DEBUG("IOCTL_SC_GET_HISTOGRAM_INFO\n");
        MDrv_SC_MACE_GetHistogramInfo(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_LUMA_CURVE:
        //KWON_0530_TEST	SC_IO_DEBUG("IOCTL_SC_SET_LUMA_CURVE\n");
        MDrv_SC_MACE_SetLumaCurve(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_LUMA_CURVE_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_LUMA_CURVE_ENABLE\n");
        MDrv_SC_MACE_SetLumaCurveEnable(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_HISTOGRAM_REQ_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_HITOGRAM_REQ_ENABLE\n");
        MDrv_SC_MACE_SetHistogramReqEnable(pDrvCtx, arg);
        break;

    case IOCTL_SC_DLCINIT:
        SC_IO_DEBUG("IOCTL_SC_DLCINIT\n");
        MDrv_SC_MACE_DLCInit(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_ICC_SATURATION_ADJ:
        SC_IO_DEBUG("IOCTL_SC_SET_ICC_SATURATION_ADJ\n");
        MDrv_SC_MACE_SetICCSaturationAdj(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_IBC_Y_ADJ:
        SC_IO_DEBUG("IOCTL_SC_SET_IBC_Y_ADJ\n");
        MDrv_SC_MACE_SetIBCYAdj(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_IHC_HUE_COLOR_DIFF_ADJ:
        SC_IO_DEBUG("IOCTL_SC_SET_IHC_HUE_COLOR_DIFF_ADJ\n");
        MDrv_SC_MACE_SetIHCHueDiffColorYAdj(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_IHC_HUE_ADJ:
        SC_IO_DEBUG("IOCTL_SC_SET_IHC_HUE_ADJ\n");
        MDrv_SC_MACE_SetIHCHueAdj(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_ICC_SATURATION_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_ICC_SATURATION_ENABLE\n");
        MDrv_SC_MACE_SetICCSaturationEnable(pDrvCtx, arg);
        break;//thchen 20080718

    case IOCTL_SC_SET_IBC_Y_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_IBC_Y_ENABLE\n");
        MDrv_SC_MACE_SetIBCYEnable(pDrvCtx, arg);
        break;//thchen 20080718

    case IOCTL_SC_SET_IHC_HUE_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_IHC_HUE_ENABLE\n");
        MDrv_SC_MACE_SetIHCHueEnable(pDrvCtx, arg);
        break;//thchen 20080718

    case IOCTL_SC_SET_ICC_REGION:
        SC_IO_DEBUG("IOCTL_SC_SET_ICC_REGION\n");
        MDrv_SC_MACE_SetICCRegionTable(pDrvCtx, arg);
        break;//victor 20080814

    case IOCTL_SC_SET_IHC_REGION:
        SC_IO_DEBUG("IOCTL_SC_SET_IHC_REGION\n");
        MDrv_SC_MACE_SetIHCRegionTable(pDrvCtx, arg);
        break;//victor 20080814

    case IOCTL_SC_SET_ICC_YMODEL_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_ICC_YMODEL_ENABLE\n");
        MDrv_SC_MACE_SetICCYModelEnable(pDrvCtx, arg);
        break;//victor 20080818

    case IOCTL_SC_SET_IHC_YMODE_DIFF_COLOR_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_IHC_YMODE_DIFF_COLOR_ENABLE\n");
        MDrv_SC_MACE_SetIHCYModeDiffColorEnable(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_IHC_YMODEL_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_IHC_YMODEL_ENABLE\n");
        MDrv_SC_MACE_SetIHCYModelEnable(pDrvCtx, arg);
        break;//victor 20080818

    case IOCTL_SC_SELECT_CSC:
        SC_IO_DEBUG("IOCTL_SC_SELECT_CSC\n");
        MDrv_SC_SelectCSC(pDrvCtx, arg);
        break;//victor 20080821
    case IOCTL_SC_SET_BLUE_STRETCH_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_BLUE_STRETCH_ENABLE\n");
        MDrv_SC_Set_Blue_Stretch_Enable(pDrvCtx, arg);
        break;//victor 20080830

    case IOCTL_SC_SET_CSC_OFFSET_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_CSC_OFFSET_ENABLE\n");
        MDrv_SC_Set_CSC_Offset_Enable(pDrvCtx, arg);
        break;//victor 20080830

    case IOCTL_SC_SET_CSC_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_CSC_ENABLE\n");
        MDrv_SC_IP2_SetCSCEnable(pDrvCtx, arg);
        break;//victor 20080909

    //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
    case IOCTL_SC_GET_VIP_CSC_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_CSC_ENABLE\n");
        MDrv_SC_VIP_GetCSCEnable(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_COLOR_RANGE:
        SC_IO_DEBUG("IOCTL_SC_SET_COLOR_RANGE\n");
        MDrv_SC_SetColorAdaptiveRange(pDrvCtx, arg);
        break;//[090601_Leo]

    case IOCTL_SC_SET_ADAPTIVE_CGAIN_EN:
        SC_IO_DEBUG("IOCTL_SC_SET_ADAPTIVE_CGAIN_EN\n");
        MDrv_SC_SetAdaptiveCGainEnable(pDrvCtx, arg);
        break;//[090921_Leo]

    case IOCTL_SC_SET_ADAPTIVE_CGAIN:
        SC_IO_DEBUG("IOCTL_SC_SET_ADAPTIVE_CGAIN\n");
        MDrv_SC_SetAdaptiveCGain(pDrvCtx, arg);
        break;//[090814_Leo]

    case IOCTL_SC_SET_PIECEWISE_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_PIECEWISE_ENABLE\n");
        MDrv_SC_SetPieceWiseEnable(pDrvCtx, arg);
        break;//[090825_Leo]

    //------------------------
    //  Utility : PQL IO control
    //------------------------
    case IOCTL_SC_PQDUMP_REGTABLE:
        MDrv_SC_PQDumpRegTable(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_QMAP_TYPE:
    {
        U8 param;

        if (copy_from_user(&param, (void __user *)arg, sizeof(U8)))
        {
            break;
        }
        MDrv_SCMAP_Init(param);
    }
         break;

    //FitchHsu 20081113 EMP when PAUSE, little shaking
    case IOCTL_SC_PQ_FASTPLAYBACK:
        SC_IO_DEBUG("IOCTL_SC_PQ_FASTPLAYBACK\n");
        MDrv_SC_PQ_FastPlayback(pDrvCtx, arg);
        break;

    //FitchHsu 20081119 EMP preview setting for 24P and 30P
    case IOCTL_SC_EMP_PREVIEW:
        SC_IO_DEBUG("IOCTL_SC_PQ_FASTPLAYBACK\n");
        MDrv_SC_EMP_Preview(pDrvCtx, arg);
        break;

    //victor 20090108, add emp video input source
    case IOCTL_SC_EMP_PLAYING_VIDEO:
        SC_IO_DEBUG("IOCTL_SC_EMP_PLAYING_VIDEO\n");
        MDrv_SC_EMP_SetPlayingEMPVideo(pDrvCtx, arg);
        break;

    //[090910_Leo]
    case IOCTL_SC_EMP_JPEG:
        SC_IO_DEBUG("IOCTL_SC_EMP_JPEG\n");
        MDrv_SC_EMP_SetEMPJPEG(pDrvCtx, arg);
    break;

    // CC Chen 20081124 MWE implement
    case IOCTL_SC_MWE_Enable:
        SC_IO_DEBUG("IOCTL_SC_MWE_Enable\n");
        MDrv_SC_MWE_Enable((BOOL)arg);
        break;

    // CC Chen 20081124 MWE implement
    case IOCTL_SC_MWE_SetWinType:
        SC_IO_DEBUG("IOCTL_SC_MWE_SetWinType\n");
        MDrv_SC_MWE_SetWinType(pDrvCtx, (SC_MWE_TYPE_e)arg);
        break;

    case IOCTL_SC_SET_DEFEATHER_TH: //victor 20080923
        SC_IO_DEBUG("IOCTL_SC_SET_DEFEATHER_TH\n");
        MDrv_SC_SetDeFeatheringThreshold(pDrvCtx, arg);
        break;
    //victor 20081112, 3DComb
    case IOCTL_SC_SET_3DCOMB:
        SC_IO_DEBUG("IOCTL_SC_SET_3DCOMB\n");
        MDrv_SC_Set3DComb(pDrvCtx, arg);
        break;

	case IOCTL_SC_SET_LVDS_SSC:
		SC_IO_DEBUG("IOCTL_SC__SET_LVDS_SSC\n");
		MDrv_SC_SetSSC(arg);
		break;

    // FitchHsu 20081209 implement THX mode
	case IOCTL_SC_SET_THX:
		SC_IO_DEBUG("IOCTL_SC_SET_THX\n");
		MDrv_SC_Set_THXMode(arg);
		break;

	// LGE [vivakjh]  2008/12/11 Merge!!  FitchHsu 20081209 implement frame lock status report
	case IOCTL_SC_GET_FRAMELOCK_STATUS:
		SC_IO_DEBUG("IOCTL_SC_GET_FRAMELOCK_STATUS\n");
		MDrv_SC_Get_FrameLock_Status(arg);
		break;

	/******************************************************************************
		LGE IOCTL : 240 ~ 254 (추가 자제할 것)
	*******************************************************************************/
	// LGE [vivakjh] 2008/12/09 	For setting the PDP's Color Wash
    case IOCTL_SC_SET_COLOR_WASH_ENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_COLOR_WASH_ENABLE\n");
        MDrv_SC_SetColorWash4PDP(pDrvCtx, arg);
        break;
    case IOCTL_SC_SET_FRAME_TO_48HZ:    // LGE[totaesun] 2008.12.27 24P 입력일때 48Hz 출력으로 바꾸기 위함.
        SC_IO_DEBUG("IOCTL_SC_SET_FRAME_TO_48HZ\n");
        MDrv_SC_SetFrameTo48Hz(pDrvCtx,arg);
        break;

	case IOCTL_SC_SETTIMINGCHGSTATUS: //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
		SC_IO_DEBUG("IOCTL_SC_SETYMOTIONENABLE\n");
		MDrv_SC_SetTimgCHGstauts(pDrvCtx, arg);
		break;

	case IOCTL_SC_GET_HDMI_VSI:
        SC_IO_DEBUG("IOCTL_SC_GET_HDMI_VSI\n");
        MDrv_HDMI_GetVSI(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_HDMI_AVI:
        SC_IO_DEBUG("IOCTL_SC_GET_HDMI_AVI\n");
        MDrv_HDMI_GetAVI(pDrvCtx, arg);
        break;

    case IOCTL_SC_GET_FRAMEDATA:
        SC_IO_DEBUG("IOCTL_SC_GET_FRAMEDATA\n");
        MDrv_SC_GetFrameData(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_FRAMEDATA:
        SC_IO_DEBUG("IOCTL_SC_SET_FRAMEDATA\n");
        MDrv_SC_SetFrameData(pDrvCtx, arg);
        break;

#if (ENABLE_ADC_TEST==1)
    case IOCTL_SC_ADC_TEST:
        MDrv_ADC_Test();
        break;
#endif

    case IOCTL_SC_SET_FBL:
        SC_IO_DEBUG("IOCTL_SC_SET_FBL\n");
        MDrv_SC_SetFBL(pDrvCtx, arg);
        break;

        case IOCTL_SC_SET_LPLL:  // FitchHsu 20080811 implement LPLL type
        SC_IO_DEBUG("IOCTL_SC_SET_LPLL\n");
        MDrv_SC_Set_LPLL(pDrvCtx, arg);
        break;

    //------------------------
    //  TCON IO Control
    //------------------------
    case IOCTL_SC_GET_TCONTAB_INFO:
        SC_IO_DEBUG("IOCTL_SC_GET_TCONTAB_INFO\n");
        MDrv_SC_Get_TCONTab_Info(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_TCONMAP:
        SC_IO_DEBUG("IOCTL_SC_SET_TCONMAP\n");
        MDrv_SC_Set_TCONMap(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_TCONPOWERSEQUENCE:
        SC_IO_DEBUG("IOCTL_SC_SET_TCONPOWERSEQUENCE\n");
        MDrv_SC_Set_TCONPower_Sequence(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_TCONCOUNT_RESET:
        SC_IO_DEBUG("IOCTL_SC_SET_TCONCOUNT_RESET\n");
        MDrv_SC_Set_TCON_Count_Reset(pDrvCtx, arg);
        break;

    //------------------------------------------------------------------------------
    // VIDEO MIRROR
    // Michu 20090903
    //------------------------------------------------------------------------------
    case IOCTL_SC_SET_VIDEOMIRROR:
        SC_IO_DEBUG("IOCTL_SC_SET_VIDEOMIRROR\n");
        MDrv_SC_Set_VideoMirror(pDrvCtx, arg);
        break;
    //------------------------------------------------------------------------------
    // End of VIDEO MIRROR
    //------------------------------------------------------------------------------
    case IOCTL_SC_IPMUX_PATTERN:
        SC_IO_DEBUG("IOCTL_SC_IPMUX_PATTERN\n");
        MDrv_SC_IPMUX_SetTestPattern(pDrvCtx, arg);
        break;

    // Michu 20091026, OD
    case IOCTL_SC_SET_ODTABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_ODTABLE\n");
        MDrv_SC_Set_ODInitial(pDrvCtx, arg);
        break;

    case IOCTL_SC_SET_ODENABLE:
        SC_IO_DEBUG("IOCTL_SC_SET_ODENABLE\n");
        MDrv_SC_OverDriverSwitch(pDrvCtx, arg);
        break;

    case IOCTL_SC_SETFD_MASK:    
        SC_IO_DEBUG("IOCTL_SC_SETFD_MASK\n");
        MDrv_SC_SetFD_Mask(arg);
        break;    

    case IOCTL_SC_SETDITHERING:    
        SC_IO_DEBUG("MDrv_SC_SetDithering\n");
        MDrv_SC_SetDithering(pDrvCtx, arg);
        break;  

    default:
        SCALER_WARNING("sc ioctl: unknown command, <%d>\n", cmd); // LGE
        PROBE_IO_EXIT(MDRV_MAJOR_SCALER, _IOC_NR(cmd));
        return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_SCALER, _IOC_NR(cmd));
    return 0;
}

#if ENABLE_SC_MMIO //[20090920 Shawn] MM I/O


/*
 * open and close: just keep track of how many times the device is
 * mapped, to avoid releasing it.
 */

void MDrv_SC_vma_open(struct vm_area_struct *vma)
{
	Scaler_Dev_t* pdev = vma->vm_private_data;
	pdev->vmas++;
}

void MDrv_SC_vma_close(struct vm_area_struct *vma)
{
	Scaler_Dev_t* pdev  = vma->vm_private_data;
	pdev->vmas--;
}


struct vm_operations_struct MDrv_SC_vm_ops = {
	.open =     MDrv_SC_vma_open,
	.close =    MDrv_SC_vma_close,
};

int MDrv_SC_MMAP(struct file *filp, struct vm_area_struct *vma)
{
    int ret;

    SC_MMIO_PRINT("enter MDrv_SC_MMAP\n");

#if 0
    if(vma->vm_end - vma->vm_start > SC_BUF_LEN)
    {
       SC_MMIO_PRINT("vma->vm_end(0x%x) - vma->vm_start(0x%x) > 0x%x\n", vma->vm_end, vma->vm_start, SC_BUF_LEN);
       return -EAGAIN;
    }
#endif
    _pu8SCBuf = (u8*)(SC_BUF_ADR);

    vma->vm_pgoff = (U32)_pu8SCBuf >> PAGE_SHIFT;

    pgprot_noncached(vma->vm_page_prot);

    //remap kernel memory to userspace
    ret = remap_pfn_range(vma,                  //user vam to map to
                          vma->vm_start,        //target user address to start at
                          vma->vm_pgoff,        //physical address of kernel memory
                          vma->vm_end - vma->vm_start, // size
                          vma->vm_page_prot) ? -EAGAIN : 0;

	if (-EAGAIN != ret)
	{
    	vma->vm_ops = &MDrv_SC_vm_ops;
    	vma->vm_flags |= VM_RESERVED;
    	vma->vm_private_data = filp->private_data;

    	MDrv_SC_vma_open(vma);
        SC_MMIO_PRINT("vma open ok\n", vma->vm_pgoff);
	}
    else
    {
        SC_MMIO_PRINT("remap_pfn_range fails!!\n");
    }

	return ret;
}
#endif


module_init(_mod_scaler_init);
module_exit(_mod_scaler_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("SCALER driver");
MODULE_LICENSE("MSTAR");
