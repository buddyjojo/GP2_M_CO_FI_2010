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
/// file    mdrv_pws_io.c
/// @brief  pws Control Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
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

//#include <linux/wait.h>
#include <linux/cdev.h>
#include <asm/io.h>

#include "Board.h"
#include "mst_devid.h"
#include "mdrv_types.h"
#include "chip_int.h"
#include "mhal_pws.h"
#include "mdrv_pws.h"
#include "mdrv_pws_io.h"
//#include "mdrv_pws_st.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define PWS_PRINT(fmt, args...)   //   printk("[PWS][%05d] " fmt, __LINE__, ## args)

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_PWS_DEVICE_COUNT     1

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _mod_pws_open(struct inode *inode, struct file *filp);
static int                      _mod_pws_release(struct inode *inode, struct file *filp);
static ssize_t                  _mod_pws_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _mod_pws_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _mod_pws_poll(struct file *filp, poll_table *wait);
static int                      _mod_pws_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _mod_pws_fasync(int fd, struct file *filp, int mode);

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32PwsMajor;
    int                         s32PwsMinor;
    struct cdev                 cDevice;
    struct file_operations      PwsFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */

} PwsModHandle;
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

PwsModHandle PwsDev=
{
    .s32PwsMajor=                  MDRV_MAJOR_PWS,
    .s32PwsMinor=                  MDRV_MINOR_PWS,
    .cDevice=
    {
        .kobj=                     {.name= MDRV_NAME_PWS, },
        .owner  =                   THIS_MODULE,
    },
    .PwsFop=
    {
        .open=                  _mod_pws_open,
        .release=               _mod_pws_release,
        .read=                  _mod_pws_read,
        .write=                 _mod_pws_write,
        .poll=                  _mod_pws_poll,
        .ioctl=                 _mod_pws_ioctl,
        .fasync =	            _mod_pws_fasync,
    },
};

wait_queue_head_t	pws_key_wait_q;
//-------------------------------------------------------------------------------------------------
// Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static int _mod_pws_open(struct inode *inode, struct file *filp)
{
	PwsModHandle *dev;

    PWS_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, PwsModHandle, cDevice);
	filp->private_data = dev;

    return 0;
}

static int _mod_pws_release(struct inode *inode, struct file *filp)
{
    PWS_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_pws_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    PWS_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_pws_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    PWS_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_pws_poll(struct file *filp, poll_table *wait)
{
    PWS_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _mod_pws_fasync(int fd, struct file *filp, int mode)
{
    PWS_PRINT("%s is invoked\n", __FUNCTION__);

	return fasync_helper(fd, filp, mode, &PwsDev.async_queue);
}

static int _mod_pws_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err = 0;
    U32 u32Result = 0;
    U8  u8Result = 0;
    U8  OnOff_flag;
//    PWS_INIT_t          PwsInit;
//    PWS_START_t         PwsStart;
//    PWS_SELENG_t        PwsSeleng;
//    PWS_SETPS_t         PwsSetps;
//    PWS_SETFILEINOUT_t  PwsSetfileinout;
//    PWS_NOTIFY_t        Pws_Notify;
//    PWS_RANDOM_t        Pws_Random;
//    PWS_STATUS_t        Pws_Status;
//
    PWS_PRINT("%s is invoked\n", __FUNCTION__);
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((PWS_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> PWS_IOC_MAXNR))
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
        s32Err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        s32Err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (s32Err)
    {
        return -EFAULT;
    }


    PROBE_IO_ENTRY(MDRV_MAJOR_PWS, _IOC_NR(cmd));

    switch(cmd)
    {
        case MDRV_PWS_INIT:
            PWS_PRINT("ioctl: MDRV_PWS_SET_IO_BASE\n");
            MDrv_PWS_Init();
            break;

        case MDRV_PWS_SET_IO_MAP_BASE:
            PWS_PRINT("ioctl: MDRV_PWS_SET_IO_MAP_BASE\n");
            u32Result = MDrv_PWS_SetIOMapBase();

            if (copy_to_user((U32 __user *) arg, &u32Result, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case MDRV_PWS_GET_IO_MAP_BASE:
            PWS_PRINT("ioctl: MDRV_PWS_GET_IO_MAP_BASE\n");
            u32Result = MDrv_PWS_GetIOMapBase();

            if (copy_to_user((U32 __user *) arg, &u32Result, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case MDRV_PWS_GET_CHIP_ID:
            PWS_PRINT("ioctl: MDRV_PWS_GET_IO_MAP_BASE\n");
            u32Result = MDrv_PWS_GetChipId();

            if (copy_to_user((U32 __user *) arg, &u32Result, sizeof(U32)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case MDRV_PWS_SET_DVI_DM_DEMUX:  //00
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_DMDEMUX\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetDviDmDemux(OnOff_flag);
            break;

        case MDRV_PWS_SET_DVI_PREAMP:  //01
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_PREAMP\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetDviPreamp(OnOff_flag);
            break;

        case MDRV_PWS_SET_DVI_BIST:  //02
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_BIST\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetDviBist(OnOff_flag);
            break;

        case MDRV_PWS_SET_DVI_DM_RXCK_BIST: //03
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_DM_RXCK_BIST\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetDviDmRxckBist(OnOff_flag);
            break;


        case MDRV_PWS_SET_DVI_DM_PRECLK: //04
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_DM_PRECLK\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetDviDmPreclk(OnOff_flag);
            break;


        case MDRV_PWS_SET_DVI_DM_PRECLK_OFFL: //05
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_DM_PRECLK_OFFL\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MHal_PWS_SetDviDmPreclkOffl(OnOff_flag);
            break;


        case MDRV_PWS_SET_DVI_DM_RBIAS: //06
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_DM_RBIAS\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MHal_PWS_SetDviDmRbias(OnOff_flag);
            break;


        case MDRV_PWS_SET_DVI_DM_ENVDET: //07
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_DM_PRECLK_OFFL\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            MHal_PWS_SetDviDmEnvdet(OnOff_flag);
            break;

        case MDRV_PWS_SET_DVI_PLL_CORE: //08
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_PLL_CORE\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetDviPllCore(OnOff_flag);
            break;

        case MDRV_PWS_SET_DVI_PLL_REGM: //09
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_PLL_REGM\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetDviPllRegm(OnOff_flag);
            break;

        case MDRV_PWS_SET_DVI_PLL_PHDAC: //10
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_PLL_PHDAC\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetDviPllPhdac(OnOff_flag);
            break;

        case MDRV_PWS_SET_INGAL_PAD_ARRAY: //11
            PWS_PRINT("ioctl: MDRV_PWS_SET_INGAL_PAD_ARRAY\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetSingalPadRArray(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_BANDGAP_IBIAS_VREF: //12
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_BANDGAP_IBIAS_VREF\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifBandgapIbiasVref(OnOff_flag);
            break;

        case MDRV_PWS_SET_Vif_CALIBRATION_BUFFER: //13
            PWS_PRINT("ioctl: MDRV_PWS_SET_Vif_CALIBRATION_BUFFER\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifCalibrationBuffer(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_CLAMP_BUFFER:  //14
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_CLAMP_BUFFER\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifClampBuffer(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_ADC_I:  //15
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_DM_RXCK_BIST\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifAdcI(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_ADCQ:  //16
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_ADCQ\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifAdcQ(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_PGA1:  //17
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_PGA1\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifPga1(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_PGA2: //18
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_PGA2\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifPga2(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_MPLL_REG: //19
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_MPLL_REG\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifMpllReg(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_ADC_OUT_CLK_PD: //20
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_ADC_OUT_CLK_PD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifAdcOutClkPd(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_MPLL_DIV2_PD:  //21
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_MPLL_DIV2_PD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifMpll_div2_pd(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_MPLL_DIV3_PD: //22
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_MPLL_DIV3_PD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifMpll_div3_pd(OnOff_flag);
            break;


        case MDRV_PWS_SET_VIF_MPLL_DIV4_PD:  //23
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_MPLL_DIV4_PD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifMpll_div4_pd(OnOff_flag);
            break;

        case MDRV_PWS_SET_VIF_MPLL_DIV8_PD:  //24
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_MPLL_DIV8_PD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifMpll_div8_pd(OnOff_flag);
            break;


        case MDRV_PWS_SET_VIF_MPLL_DIV10_PD: //25
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_MPLL_DIV10_PD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifMpll_div10_pd(OnOff_flag);
            break;


        case MDRV_PWS_SET_VIF_TAGC:
            PWS_PRINT("ioctl: MDRV_PWS_SET_VIF_TAGC\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetVifTagc(OnOff_flag);
            break;


        case MDRV_PWS_SET_ADC_R:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_R\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetAdcR(OnOff_flag);
            break;


        case MDRV_PWS_SET_ADC_G:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_G\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetAdcG(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_B:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_B\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcB(OnOff_flag);
            break;

        case MDRV_PWS_SET_PHDIG:
            PWS_PRINT("ioctl: MDRV_PWS_SET_PHDIG\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPhdig(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PLLA:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PLLA\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPllA(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_ICLAMP_RGB:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_ICLAMP_RGB\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcIclampRgb(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_ICLAMP_VDY:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_ICLAMP_VDY\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcIclampVdy(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_ICLAMP_VDC:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_ICLAMP_VDC\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetAdcIclampVdc(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_Y:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_Y\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcY(OnOff_flag);
            break;

        case MDRV_PWS_SET_PLL_B:
            PWS_PRINT("ioctl: MDRV_PWS_SET_PLL_B\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPllB(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_SOG:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_SOG\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcSog(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_SOG_OFF:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_SOG_OFF\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcSogOff(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_HSYNC0:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_HSYNC0\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcHsync0(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_HSYNC1:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_HSYNC1\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcHSync1(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_HSYNC2:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_HSYNC2\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }
            u8Result = MDrv_PWS_SetAdcHsync2(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_200:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_200\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClk200(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_400:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_400\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClk400(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_PLL:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_PLL\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClkPll(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_R:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_R\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClkR(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_G:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_G\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClkG(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_B:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_B\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClkB(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_Y:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_Y\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClkY(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_VD:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_VD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClkVd(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_200_FB:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_200_FB\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcPdClk200Fb(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_SOG_MUX:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_SOG_MUX\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcSogMux(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_FB_ADC:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_FB_ADC\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
 	            return -EFAULT;
            }

            u8Result = MDrv_PWS_SetAdcFbAdc(OnOff_flag);
            break;

/*
        case MDRV_PWS_SET_USB_HS_TX:
            PWS_PRINT("ioctl: MDRV_PWS_SET_USB_HS_TX\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetUsbHsTx(OnOff_flag);
            break;

        case MDRV_PWS_SET_USB_HS_RX:
            PWS_PRINT("ioctl: MDRV_PWS_SET_USB_HS_RX\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetUsbHsRx(OnOff_flag);
            break;

        case MDRV_PWS_SET_USB_FlXCVR:
            PWS_PRINT("ioctl: MDRV_PWS_SET_USB_FlXCVR\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetUsbFlXcvr(OnOff_flag);
            break;

        case MDRV_PWS_SET_USB_SERDES:
            PWS_PRINT("ioctl: MDRV_PWS_SET_USB_SERDES\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetUsbSerdes(OnOff_flag);
            break;

        case MDRV_PWS_SET_USB_PLL_LDO:
            PWS_PRINT("ioctl: MDRV_PWS_SET_USB_PLL_LDO\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetUsbPllLdo(OnOff_flag);
            break;

        case MDRV_PWS_SET_USB_LDO:
            PWS_PRINT("ioctl: MDRV_PWS_SET_USB_LDO\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetUsbLdo(OnOff_flag);
            break;

        case MDRV_PWS_SET_USB_REF_BIAS_CIR:
            PWS_PRINT("ioctl: MDRV_PWS_SET_USB_REF_BIAS_CIR\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetUsbRefBiasCir(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADCR:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADCR\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcR(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADCG:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADCG\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcG(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADCB:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADCB\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcB(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PHDIG:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PHDIG\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPhdig(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PLLA:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PLLA\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPllA(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_ICLAMP_RGB:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_ICLAMP_RGB\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcIclampRgb(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_ICLAMP_VDY:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_ICLAMP_VDY\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcIclampVdy(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_ICLAMP_VDC:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_ICLAMP_VDC\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcIclampVdc(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADCY:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADCY\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcY(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PLLB:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PLLB\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPllB(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_SOG:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_SOG\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;

            u8Result = MDrv_PWS_SetAdcSog(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_SOG_OFF:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_SOG_OFF\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcSogOff(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_SOG_UNUSED:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_SOG_UNUSED\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcSogUnused(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_HSYNC0:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_HSYNC0\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcHsync0(OnOff_flag);
            break;


        case MDRV_PWS_SET_ADC_HSYNC1:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_HSYNC1\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcHSync1(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_HSYNC2:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_HSYNC2\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcHsync2(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK200:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK200\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClk200(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK400:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK400\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClk400(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_PLL:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_PLL\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClkPll(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_R:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_R\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClkR(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_G:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_G\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClkG(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_B:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_B\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClkB(OnOff_flag);
            break;


        case MDRV_PWS_SET_ADC_PD_CLK_Y:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_Y\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClkY(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK_VD:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK_VD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClkVd(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_PD_CLK200_FB:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_PD_CLK200_FB\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcPdClk200Fb(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_SOG_MUX:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_SOG_MUX\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcSogMux(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_FB_ADC:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_FB_ADC\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcFbAdc(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_CVBS_LPF_Y:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_CVBS_LPF_Y\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcCvbsLpfY(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_CVBS_LPF_C:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_CVBS_LPF_C\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcCvbsLpfC(OnOff_flag);
            break;
            */
/*
        case MDRV_PWS_SET_GMC_P:
            PWS_PRINT("ioctl: MDRV_PWS_SET_GMC_P\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetGmcp(OnOff_flag);
            break;

        case MDRV_PWS_SET_GMC_Y:
            PWS_PRINT("ioctl: MDRV_PWS_SET_GMC_Y\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetGmcY(OnOff_flag);
            break;

        case MDRV_PWS_SET_GMC_C:
            PWS_PRINT("ioctl: MDRV_PWS_SET_GMC_C\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetGmcC(OnOff_flag);
            break;

        case MDRV_PWS_SET_CVBS_BUF_OUT:
            PWS_PRINT("ioctl: MDRV_PWS_SET_CVBS_BUF_OUT\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetCvbsBufOut(OnOff_flag);
            break;

        case MDRV_PWS_SET_DAC_CVBS:
            PWS_PRINT("ioctl: MDRV_PWS_SET_DAC_CVBS\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetDacCvbs(OnOff_flag);
            break;

        case MDRV_PWS_SET_FAST_BLANKING:
            PWS_PRINT("ioctl: MDRV_PWS_SET_DVI_DM_RXCK_BIST\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetFastBlanking(OnOff_flag);
            break;

        case MDRV_PWS_SET_ADC_RGB_BIAS_CURRENT_CONTROL:
            PWS_PRINT("ioctl: MDRV_PWS_SET_ADC_RGB_BIAS_CURRENT_CONTROL\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAdcRgbBiasCurrentControl(OnOff_flag);
            break;

        case MDRV_PWS_SET_AUDIO:
            PWS_PRINT("ioctl: MDRV_PWS_SET_AUDIO\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetAudio(OnOff_flag);
            break;

        case MDRV_PWS_SET_VD:
            PWS_PRINT("ioctl: MDRV_PWS_SET_VD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetVd(OnOff_flag);
            break;

        case MDRV_PWS_SET_SVD:
            PWS_PRINT("ioctl: MDRV_PWS_SET_SVD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetSvd(OnOff_flag);
            break;

        case MDRV_PWS_SET_MVD_M4V:
            PWS_PRINT("ioctl: MDRV_PWS_SET_MVD_M4V\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetMvdM4V(OnOff_flag);
            break;

        case MDRV_PWS_SET_VE:
            PWS_PRINT("ioctl: MDRV_PWS_SET_VE\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetVe(OnOff_flag);
            break;

        case MDRV_PWS_SET_RVD:
            PWS_PRINT("ioctl: MDRV_PWS_SET_RVD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetRvd(OnOff_flag);
            break;

        case MDRV_PWS_SET_STRLD:
            PWS_PRINT("ioctl: MDRV_PWS_SET_STRLD\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetStrld(OnOff_flag);
            break;

        case MDRV_PWS_SET_GOPG2:
            PWS_PRINT("ioctl: MDRV_PWS_SET_GOPG2\n");
            if (copy_from_user(&OnOff_flag, (U8 __user *) arg, sizeof(U8)))
 	            return -EFAULT;
            u8Result = MDrv_PWS_SetGopg2(OnOff_flag);
            break;
*/
        case MDRV_PWS_SET_DVDD:
            MDrv_PWS_Set_DVDD();
            break;

        default:
            PWS_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
            return -ENOTTY;
    }


    PROBE_IO_EXIT(MDRV_MAJOR_PWS, _IOC_NR(cmd));
    return 0;
}

int __init mod_pws_init(void)
{
    S32         s32Ret;
    dev_t       dev;
	//U8			pws_dn_flag;

    PWS_PRINT("%s is invoked\n", __FUNCTION__);

    if (PwsDev.s32PwsMajor)
    {
        dev = MKDEV(PwsDev.s32PwsMajor, PwsDev.s32PwsMinor);
        s32Ret = register_chrdev_region(dev, MOD_PWS_DEVICE_COUNT, MDRV_NAME_PWS);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, PwsDev.s32PwsMinor, MOD_PWS_DEVICE_COUNT, MDRV_NAME_PWS);
        PwsDev.s32PwsMajor = MAJOR(dev);
    }

	init_waitqueue_head(&pws_key_wait_q);

    if ( 0 > s32Ret)
    {
        PWS_PRINT("Unable to get major %d\n", PwsDev.s32PwsMajor);
        return s32Ret;
    }

    cdev_init(&PwsDev.cDevice, &PwsDev.PwsFop);
    if (0!= (s32Ret= cdev_add(&PwsDev.cDevice, dev, MOD_PWS_DEVICE_COUNT)))
    {
        PWS_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_PWS_DEVICE_COUNT);
        return s32Ret;
    }

    //MDrv_PWS_Init();

    return 0;
}
/*
void __exit mod_pws_exit(void)
{
    PWS_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&PwsDev.cDevice);
    unregister_chrdev_region(MKDEV(PwsDev.s32PwsMajor, PwsDev.s32PwsMinor), MOD_PWS_DEVICE_COUNT);
}*/

module_init(mod_pws_init);
//module_exit(mod_pws_exit);




















