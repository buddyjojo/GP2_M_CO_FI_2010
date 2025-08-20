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
#define _MDRV_VE_C
///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_ve.c
/// This file contains the Mstar driver for TVEncoder
/// @author MStar Semiconductor Inc.
/// @brief  TVEncoder module
///////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>    // for MKDEV()

#include "mdrv_types.h"
#include "mst_devid.h"
#include "mdrv_ve.h"
#include "mdrv_ve_io.h"
#include "mdrv_ve_st.h"
#include "mhal_ve.h"
#include "mdrv_scaler_st.h"

#include "mdrv_probe.h"

#define MOD_VE_DEVICE_COUNT    1

//#define VE_WARNING(fmt, args...)       printk(KERN_WARNING "[VEMOD][%06d] " fmt, __LINE__, ## args)
//#define VE_PRINT(fmt, args...)         printk("[VEMOD][%06d]     " fmt, __LINE__, ## args)
// -----------------------------------------------------------------------------
// Global variable
// -----------------------------------------------------------------------------
//MS_TVENCODER_INFO   g_VEInfo;

SC_INPUT_MUX_t		g_SCInputMuxInfo = {0,};

typedef struct
{
    int                         s32VEMajor;
    int                         s32VEMinor;
    struct cdev                 cDevice;
    struct file_operations      VEFop;
} VEModHandle;

static VEModHandle VEDev=
{
    .s32VEMajor=               MDRV_MAJOR_TVENCODER,
    .s32VEMinor=               MDRV_MINOR_TVENCODER,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_TVENCODER, },
        .owner  =               THIS_MODULE,
    },
    .VEFop=
    {
    	.owner =    THIS_MODULE,
    	.ioctl =    MDrv_VE_IOCtl,
    	.open =     MDrv_VE_Open,
    	.release =  MDrv_VE_Release,
    },
};



// -----------------------------------------------------------------------------
// Local function
// -----------------------------------------------------------------------------


//------------------------------------------------------------------------------
/// set the output destination of video encoder
///
///              None SCART CVBS SVIDEO YPbPr
///       None    O     O     O     O     O
///       SCART   O     -     X     X     X
///        CVBS   O     X     -     O     O
///     S_VIDEO   O     X     O     -     X
///       YPbPr   O     X     O     X     -
///
/// @param  -pSwitchInfo \b IN/OUT: the information of switching output destination of TV encodeer
/// @return None
//------------------------------------------------------------------------------
BOOL _MDrv_VE_SwitchOuputDest(unsigned long arg)
{
    MS_SWITCH_VE_DEST_INFO stDestInfo;

    if (copy_from_user(&stDestInfo, (MS_SWITCH_VE_DEST_INFO __user *)arg, sizeof(MS_SWITCH_VE_DEST_INFO)))
        return EFAULT;

    MHal_VE_SwitchOuputDest(&stDestInfo);
    return 0;
}


BOOL _MDrv_VE_SwitchInputSrc(unsigned long arg)
{
    MS_SWITCH_VE_SRC_INFO stSrcInfo;

    if (copy_from_user(&stSrcInfo, (MS_SWITCH_VE_SRC_INFO __user *)arg, sizeof(MS_SWITCH_VE_SRC_INFO)))
        return EFAULT;
    VE_DRV_DBG("_MDrv_VE_SwitchInputSrc %d\n", stSrcInfo.InputSrcType);
    MHal_VE_SwitchInputSrc(&stSrcInfo);
    return 0;
}

#if 0  // daniel.huang 20090615
BOOL _MDrv_VE_TransferInputMux(unsigned long arg)
{
    SC_INPUT_MUX_t param;

    if (copy_from_user(&param, (void __user *)arg, sizeof(SC_INPUT_MUX_t)))
        return EFAULT;
    printk("_MDrv_VE_TransferInputMux \n");

	g_SCInputMuxInfo.u8AtvYMux     = param.u8AtvYMux;
	g_SCInputMuxInfo.u8AtvCMux     = param.u8AtvCMux;
	g_SCInputMuxInfo.u8Cvbs1YMux   = param.u8Cvbs1YMux;
	g_SCInputMuxInfo.u8Cvbs1CMux   = param.u8Cvbs1CMux;
	g_SCInputMuxInfo.u8Cvbs2YMux   = param.u8Cvbs2YMux;
	g_SCInputMuxInfo.u8Cvbs2CMux   = param.u8Cvbs2CMux;
	g_SCInputMuxInfo.u8Cvbs3YMux   = param.u8Cvbs3YMux;
	g_SCInputMuxInfo.u8Cvbs3CMux   = param.u8Cvbs3CMux;
	g_SCInputMuxInfo.u8SVideo1YMux = param.u8SVideo1YMux;
	g_SCInputMuxInfo.u8SVideo1CMux = param.u8SVideo1CMux;
	g_SCInputMuxInfo.u8SVideo2YMux = param.u8SVideo2YMux;
	g_SCInputMuxInfo.u8SVideo2CMux = param.u8SVideo2CMux;
	g_SCInputMuxInfo.u8Scart1YMux  = param.u8Scart1YMux;
	g_SCInputMuxInfo.u8Scart1CMux  = param.u8Scart1CMux;
	g_SCInputMuxInfo.u8Scart2YMux  = param.u8Scart2YMux;
	g_SCInputMuxInfo.u8Scart2CMux  = param.u8Scart2CMux;

    return 0;
}
#endif

//------------------------------------------------------------------------------
/// control the output of video encoder
/// @param  -pOutputCtrl \b IN: the control information of VE
/// @return MS_MTSTATUS
//------------------------------------------------------------------------------
BOOL _MDrv_VE_Set_OutputCtrl(unsigned long arg) // purpose to do FRC
{
    MS_VE_OUTPUT_CTRL stOutputCtrl;

    if (copy_from_user(&stOutputCtrl, (MS_VE_OUTPUT_CTRL __user *)arg, sizeof(MS_VE_OUTPUT_CTRL)))
        return EFAULT;

    MHal_VE_Set_OutputCtrl(&stOutputCtrl);
    return 0;
}

//------------------------------------------------------------------------------
/// set the output video standard of video encoder
/// @param  -VideoSystem \b IN: the video standard
/// @return TRUE: supported and success,  FALSE: unsupported or unsuccess
//------------------------------------------------------------------------------
BOOL _MDrv_VE_Set_Output_VideoStd(unsigned long arg)
{
    MS_VE_VIDEOSYS enVideoSystem;

    if (__get_user(enVideoSystem, (MS_VE_VIDEOSYS __user *)arg))
        return EFAULT;

    MHal_VE_Set_Output_VideoStd(enVideoSystem);
    return 0;
}

void _MDrv_VE_Set_VPSData(unsigned long arg)
{
    stVE_VPSData stVPSData;
    if (copy_from_user(&stVPSData, (stVE_VPSData __user *)arg, sizeof(stVE_VPSData)))
        return ;

    MHal_VE_Set_VPSData(stVPSData.bEn, stVPSData.pu8VPSData);
}

void _MDrv_VE_Set_WSSData(unsigned long arg) // 071204
{
    stVE_WSSData stWSSData;
    if (copy_from_user(&stWSSData, (stVE_WSSData __user *)arg, sizeof(stVE_WSSData)))
        return ;

    MHal_VE_Set_WSSData(stWSSData.bEn, stWSSData.u16WSSData);
}

BOOL _MDrv_VE_Set_CCData(unsigned long arg) // 071204
{
    stVE_CCData stCCData;
    if (copy_from_user(&stCCData, (stVE_CCData __user *)arg, sizeof(stVE_CCData)))
        return FALSE;

    return MHal_VE_Set_CCData(stCCData.bEn, stCCData.u16CCData0, stCCData.u16CCData1);
    /*if (MHal_VE_Set_CCData(stCCData.bEn, stCCData.u16CCData0, stCCData.u16CCData1)==FALSE){
        stCCData.u8Success = FALSE;
    }
    else
        stCCData.u8Success = TRUE;*/
}

BOOL _MDrv_VE_Set_TTXData(unsigned long arg)
{
    stVE_TTXData stTTXData;
    if (copy_from_user(&stTTXData, (stVE_TTXData __user *)arg, sizeof(stVE_TTXData)))
        return EFAULT;

    return MHal_VE_SaveTTX(stTTXData.u32LineFlag,
                        stTTXData.u32Size, stTTXData.u32PacketAddr);
}

BOOL _MDrv_VE_Init(unsigned long arg)
{
    return MHal_VE_Init();
}

#if 0
//------------------------------------------------------------------------------
/// set the output Type of video encoder
///
///       00 : CVBS,Y,C
///       01 : CVBS,Y,Cb,Cr
///       10 : CVBS,R,G,B
///
/// @return None
//------------------------------------------------------------------------------
BOOL _MDrv_VE_Set_OuputType(unsigned long arg)
{
    MS_VE_VE_OUTPUT_TYPE enOutputType;

    if (__get_user(enOutputType, (MS_VE_VE_OUTPUT_TYPE __user *)arg))
        return EFAULT;

    MHal_VE_Set_OuputType(enOutputType);
    return 0;
}
#endif
//------------------------------------------------------------------------------
/// VE enable/disable
/// @return none
//------------------------------------------------------------------------------
BOOL _MDrv_VE_Enable(unsigned long arg)
{
    BOOL blEnable;

    if (__get_user(blEnable, (BOOL __user *)arg))
        return EFAULT;

    MHal_VE_Enable(blEnable);
    return 0;
}

//------------------------------------------------------------------------------
/// VE reset
/// @return none
//------------------------------------------------------------------------------
BOOL _MDrv_VE_Reset(unsigned long arg)
{
    BOOL blEnable;

    if (__get_user(blEnable, (BOOL __user *)arg))
        return EFAULT;

    MHal_VE_Reset(blEnable);
    return 0;
}


BOOL _MDRV_VE_GenTestPattern(unsigned long arg)
{
    MHal_VE_GenTestPattern();
    return 0;
}

//------------------------------------------------------------------------------
/// VE SetBlackScreen
/// @return none
//------------------------------------------------------------------------------
BOOL _MDrv_VE_SetBlackScreen(unsigned long arg)
{
    BOOL blEnable;

    if (__get_user(blEnable, (BOOL __user *)arg))
        return EFAULT;

    MHal_VE_SetBlackScreen(blEnable);
    return 0;
}
#if 0
//------------------------------------------------------------------------------
/// VE RF Out
/// @return none
//------------------------------------------------------------------------------
int _MDrv_VE_Set_ADC_RFOut(unsigned long arg)
{
    BOOL blEnable;

    if (__get_user(blEnable, (BOOL __user *)arg))
        return EFAULT;

	MHal_VE_Set_ADC_RFOut(blEnable);
	return 0;
}
#endif

// -----------------------------------------------------------------------------
// Device Methods
// -----------------------------------------------------------------------------

/*
 * Open and close
 */

int MDrv_VE_Open(struct inode *inode, struct file *filp)
{
    VE_DRV_DBG("%s is invoked\n", __FUNCTION__);

	return 0;          /* success */
}

int MDrv_VE_Release(struct inode *inode, struct file *filp)
{
    VE_DRV_DBG("%s is invoked\n", __FUNCTION__);

	return 0;
}


int MDrv_VE_IOCtl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{

	int err = 0;
	int retval = 0;

	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != MDRV_VE_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > MDRV_VE_IOC_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

    PROBE_IO_ENTRY(MDRV_MAJOR_TVENCODER, _IOC_NR(cmd));
	switch(cmd) {

	  case MDRV_VE_SWITCH_OUTPUT_DES:
        // Need to refine this code, it is wrong
        retval = _MDrv_VE_SwitchOuputDest(arg);
		break;

	  case MDRV_VE_SET_OUTPUT_CTL:
        retval = _MDrv_VE_Set_OutputCtrl(arg);
		break;

	  case MDRV_VE_SET_OUTPUT_VIDEO_STD:
        retval = _MDrv_VE_Set_Output_VideoStd(arg);
        break;

      case MDRV_VE_POWER_ON:
        MHal_VE_PowerOn();
        break;

      case MDRV_VE_POWER_OFF:
        MHal_VE_PowerOff();
        break;

      case MDRV_VE_INIT:
        // The related setting is moved to MDrv_VE_Open() temporarily.
        // Maybe need to move to MDrv_VE_Module_Init
        retval = _MDrv_VE_Init(arg);
        break;

/*	  case MDRV_VE_SET_OUTPUT_TYPE:
        retval = _MDrv_VE_Set_OuputType(arg);
		break;*/

	  case MDRV_VE_ENABLE:
		retval = _MDrv_VE_Enable(arg);
        break;

	  case MDRV_VE_RESET:
        retval = _MDrv_VE_Reset(arg);
		break;

      case MDRV_VE_GEN_TEST_PATTERN:
        retval = _MDRV_VE_GenTestPattern(arg);
        break;
      case MDRV_VE_SWITCH_INPUTSRC:
        retval = _MDrv_VE_SwitchInputSrc(arg);
        break;
      case MDRV_VE_SET_VPSDATA:
        _MDrv_VE_Set_VPSData(arg);
        break;
      case MDRV_VE_SET_WSSDATA:
        _MDrv_VE_Set_WSSData(arg);
        break;
      case MDRV_VE_SET_TTXDATA:
        if (_MDrv_VE_Set_TTXData(arg)==FALSE)
            retval = -EFAULT;
        break;

      case MDRV_VE_SetBlackScreen:
		retval = _MDrv_VE_SetBlackScreen(arg);
        break;

#if 0
      case MDRV_VE_DISABLE_ADCBUFOUT:
        MHal_VE_Set_ADC_BuffOut(FALSE, 0);
        break;

      case MDRV_VE_AV2MNTOUT:
        MHal_VE_Set_ADC_BuffOut(TRUE, INPUT_PORT_MS_CVBS1);
        break;

      case MDRV_VE_ATV2MNTOUT:
        MHal_VE_Set_ADC_BuffOut(TRUE, INPUT_PORT_MS_CVBS0);
        break;

      case MDRV_VE_SC1CVBS2MNTOUT:
        MHal_VE_Set_ADC_BuffOut(TRUE, INPUT_PORT_AV_SCART0);
        break;

	  case MDRV_VE_SC2CVBS2MNTOUT:
        MHal_VE_Set_ADC_BuffOut(TRUE, INPUT_PORT_AV_SCART1);
        break;

      case MDRV_VE_DTV2MNTOUT:
        MHal_VE_Set_ADC_BuffOut(TRUE, INPUT_PORT_MS_DTV);
        break;

	  case MDRV_VE_ATV2TVOUT:
		_MDrv_VE_Set_ADC_RFOut(arg);
	    break;

	  case MDRV_VE_SVID2MNTOUT:
		MHal_VE_Set_ADC_BuffOut(TRUE, INPUT_PORT_MS_SV0);
		break;

	  case MDRV_VE_TRANSFER_INPUT_MUX:
		_MDrv_VE_TransferInputMux(arg);
		break;
#endif

      case MDRV_VE_SET_CCDATA:
        if (_MDrv_VE_Set_CCData(arg)==FALSE)
            retval = -EFAULT;
        break;
	  default:  /* redundant, as cmd was checked against MAXNR */
    	PROBE_IO_EXIT(MDRV_MAJOR_TVENCODER, _IOC_NR(cmd));
		return -ENOTTY;
	}
    PROBE_IO_EXIT(MDRV_MAJOR_TVENCODER, _IOC_NR(cmd));
	return retval;

}




static int __init MDrv_VE_Module_Init(void)
{
    int         s32Ret;
    dev_t       dev;

    VE_DRV_DBG("%s is invoked\n", __FUNCTION__);

    if (VEDev.s32VEMajor)
    {
        VE_DRV_DBG("call register_chrdev_region\n");
        dev = MKDEV(VEDev.s32VEMajor, VEDev.s32VEMinor);
        s32Ret = register_chrdev_region(dev, MOD_VE_DEVICE_COUNT, MDRV_NAME_TVENCODER);
    }
    else
    {
        VE_DRV_DBG("call alloc_chrdev_region\n");
        s32Ret = alloc_chrdev_region(&dev, VEDev.s32VEMinor, MOD_VE_DEVICE_COUNT, MDRV_NAME_TVENCODER);
        VEDev.s32VEMajor = MAJOR(dev);
    }
    if (0> s32Ret)
    {
        VE_DRV_DBG("Unable to get major %d\n", VEDev.s32VEMajor);
        VE_DRV_DBG("Unable to get major %d\n", VEDev.s32VEMajor);
        return s32Ret;
    }

    cdev_init(&VEDev.cDevice, &VEDev.VEFop);
    if (0!= (s32Ret= cdev_add(&VEDev.cDevice, dev, MOD_VE_DEVICE_COUNT)))
    {
        VE_DRV_DBG("Unable add a character device\n");
        VE_DRV_DBG("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_VE_DEVICE_COUNT);
        return s32Ret;
    }

    VE_DRV_DBG("Init VE successfully\n");
    return 0;
}

static void __exit MDrv_VE_Module_Exit(void)
{
    VE_DRV_DBG("%s is invoked\n", __FUNCTION__);

    cdev_del(&VEDev.cDevice);
    unregister_chrdev_region(MKDEV(VEDev.s32VEMajor, VEDev.s32VEMinor), MOD_VE_DEVICE_COUNT);
}


module_init(MDrv_VE_Module_Init);
module_exit(MDrv_VE_Module_Exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("VE driver");
MODULE_LICENSE("MSTAR");

