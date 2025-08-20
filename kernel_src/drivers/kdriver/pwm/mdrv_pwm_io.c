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
/// file    mdrv_pwm_io.c
/// @brief  PWM Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
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
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mst_devid.h"

#include "mdrv_pwm_io.h"
#include "mhal_pwm_reg.h"
#include "mhal_pwm.h"
#include "mdrv_pwm.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define PWM_DBG_ENABLE              0

#if PWM_DBG_ENABLE		// changed to remove warning(dreamer@lge.com)
#define PWM_DBG(_f)                 (_f)
#else
#define PWM_DBG(_f)
#endif

#if 0
#define LINE_DBG()                  printf("PWM %d\n", __LINE__)
#else
#define LINE_DBG()
#endif

#define PWM_PRINT(fmt, args...)        //printk("[PWM][%05d] " fmt, __LINE__, ## args)

typedef struct
{
    S32                          s32MajorPWM;
    S32                          s32MinorPWM;
    struct cdev                 cDevice;
    struct file_operations      PWMFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} PWM_ModHandle_t;


#define MOD_PWM_DEVICE_COUNT         1
#define MOD_PWM_NAME                 "ModPWM"


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _MDrv_PWM_Open (struct inode *inode, struct file *filp);
static int                      _MDrv_PWM_Release(struct inode *inode, struct file *filp);
static ssize_t                  _MDrv_PWM_Read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _MDrv_PWM_Write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _MDrv_PWM_Poll(struct file *filp, poll_table *wait);
static int                      _MDrv_PWM_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int                      _MDrv_PWM_Fasync(int fd, struct file *filp, int mode);

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

static PWM_ModHandle_t PWMDev=
{
    .s32MajorPWM = MDRV_MAJOR_PWM,
    .s32MinorPWM = MDRV_MINOR_PWM,
    .cDevice =
    {
        .kobj = {.name= MOD_PWM_NAME, },
        .owner = THIS_MODULE,
    },
    .PWMFop =
    {
        .open =     _MDrv_PWM_Open,
        .release =  _MDrv_PWM_Release,
        .read =     _MDrv_PWM_Read,
        .write =    _MDrv_PWM_Write,
        .poll =     _MDrv_PWM_Poll,
        .ioctl =    _MDrv_PWM_Ioctl,
        .fasync =   _MDrv_PWM_Fasync,
    },
};

static PWM_Param_t PWM_Param;
static B16  bClkEn;
#if 0 //for T2 U05
static U8   u8CntMode;
static U16  u16Temp;
#endif
//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

static int _MDrv_PWM_Open (struct inode *inode, struct file *filp)
{
    PWM_ModHandle_t *dev;

    PWM_PRINT("%s is invoked\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, PWM_ModHandle_t, cDevice);
    filp->private_data = dev;

    return 0;
}

static int _MDrv_PWM_Release(struct inode *inode, struct file *filp)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _MDrv_PWM_Read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _MDrv_PWM_Write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _MDrv_PWM_Poll(struct file *filp, poll_table *wait)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}


static int _MDrv_PWM_Fasync(int fd, struct file *filp, int mode)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);

    return 0;
}

static int _MDrv_PWM_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err= 0;
  //	U16 u16Temp;

    PWM_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((PWM_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> PWM_IOC_MAXNR))
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

    PROBE_IO_ENTRY(MDRV_MAJOR_PWM, _IOC_NR(cmd));

    switch(cmd)
    {
        case MDRV_PWM_INIT:
            PWM_PRINT("ioctl: MDRV_PWM_INIT\n");
            MDrv_PWM_Init();
            break;

        case MDRV_PWM_GRP0_CLK_GATE_EN:
            PWM_PRINT("ioctl: MDRV_PWM_GRP0_CLK_GATE_EN\n");
            if (copy_from_user(&bClkEn, (B16 __user *)arg, sizeof(B16)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Grp0_Clk_Gate_En(bClkEn);
            break;
        case MDRV_PWM_GRP1_CLK_GATE_EN:
            PWM_PRINT("ioctl: MDRV_PWM_GRP1_CLK_GATE_EN\n");
            if (copy_from_user(&bClkEn, (B16 __user *)arg, sizeof(B16)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Grp1_Clk_Gate_En(bClkEn);
            break;
        case MDRV_PWM_GRP2_CLK_GATE_EN:
            PWM_PRINT("ioctl: MDRV_PWM_GRP2_CLK_GATE_EN\n");
            if (copy_from_user(&bClkEn, (B16 __user *)arg, sizeof(B16)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Grp2_Clk_Gate_En(bClkEn);
            break;

        case MDRV_PWM_OEN:
            PWM_PRINT("ioctl: MDRV_PWM_OEN\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Oen(PWM_Param.u8Index, PWM_Param.b16Oen);
            break;
        case MDRV_PWM_PERIOD:
            PWM_PRINT("ioctl: MDRV_PWM_PERIOD\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Period(PWM_Param.u8Index, PWM_Param.u16Period);
            break;
        case MDRV_PWM_DUTY:
            PWM_PRINT("ioctl: MDRV_PWM_DUTY\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_DutyCycle(PWM_Param.u8Index, PWM_Param.u16Duty);
            break;

        case MDRV_PWM_DIV:
            PWM_PRINT("ioctl: MDRV_PWM_DIV\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Div(PWM_Param.u8Index, PWM_Param.u16Div);
            break;
        case MDRV_PWM_POLARITY:
            PWM_PRINT("ioctl: MDRV_PWM_POLARITY\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Polarity(PWM_Param.u8Index, PWM_Param.b16Polarity);
            break;
        case MDRV_PWM_VDBEN:
            PWM_PRINT("ioctl: MDRV_PWM_VDBEN\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Vdben(PWM_Param.u8Index, PWM_Param.b16Vdben);
            break;
        case MDRV_PWM_RESET_EN:
            PWM_PRINT("ioctl: MDRV_PWM_RESET_EN\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Reset_En(PWM_Param.u8Index, PWM_Param.b16ResetEn);
            break;
        case MDRV_PWM_DBEN:
            PWM_PRINT("ioctl: MDRV_PWM_DBEN\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Dben(PWM_Param.u8Index, PWM_Param.b16Dben);
            break;
        case MDRV_PWM_RST_MUX:
            PWM_PRINT("ioctl: MDRV_PWM_RST_MUX\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Rst_Mux(PWM_Param.u8Index, PWM_Param.b16RstMux);
            break;
        case MDRV_PWM_HS_RST_CNT:
            PWM_PRINT("ioctl: MDRV_PWM_HS_RST_CNT\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Rst_Cnt(PWM_Param.u8Index, PWM_Param.u8RstCnt);
            break;
        case MDRV_PWM_PERIOD_EXT:
            PWM_PRINT("ioctl: MDRV_PWM_PERIOD_EXT\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Period_Ext(PWM_Param.u8Index, PWM_Param.u16PeriodExt);
            break;
        case MDRV_PWM_DUTY_EXT:
            PWM_PRINT("ioctl: MDRV_PWM_DUTY_EXT\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Duty_Ext(PWM_Param.u8Index, PWM_Param.u16DutyExt);
            break;
        case MDRV_PWM_DIV_EXT:
            PWM_PRINT("ioctl: MDRV_PWM_DIV_EXT\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Div_Ext(PWM_Param.u8Index, PWM_Param.u16DivExt);
            break;
        case MDRV_PWM_SHIFT:
            PWM_PRINT("ioctl: MDRV_PWM_SHIFT\n");
            if (copy_from_user(&PWM_Param, (PWM_Param_t __user *)arg, sizeof(PWM_Param_t)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
                return -EFAULT;
            }
            MDrv_PWM_Shift(PWM_Param.u8Index, PWM_Param.u32Shift);
            break;

        default:
            PWM_PRINT("ioctl: unknown command\n");
            PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
            return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_PWM, _IOC_NR(cmd));
    return 0;
}

static int __init mod_pwm_init(void)
{
    S32         s32Ret;
    dev_t       dev;

    PWM_PRINT("%s is invoked\n", __FUNCTION__);

    if (PWMDev.s32MajorPWM)
    {
        dev = MKDEV(PWMDev.s32MajorPWM, PWMDev.s32MinorPWM);
        s32Ret = register_chrdev_region(dev, MOD_PWM_DEVICE_COUNT, MOD_PWM_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, PWMDev.s32MinorPWM, MOD_PWM_DEVICE_COUNT, MOD_PWM_NAME);
        PWMDev.s32MajorPWM = MAJOR(dev);
    }

    if ( 0 > s32Ret)
    {
        PWM_PRINT("Unable to get major %d\n", PWMDev.s32MajorPWM);
        return s32Ret;
    }

    cdev_init(&PWMDev.cDevice, &PWMDev.PWMFop);
    if (0!= (s32Ret= cdev_add(&PWMDev.cDevice, dev, MOD_PWM_DEVICE_COUNT)))
    {
        PWM_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_PWM_DEVICE_COUNT);
        return s32Ret;
    }

    return 0;
}

static void __exit mod_pwm_exit(void)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&PWMDev.cDevice);
    unregister_chrdev_region(MKDEV(PWMDev.s32MajorPWM, PWMDev.s32MinorPWM), MOD_PWM_DEVICE_COUNT);
}

module_init(mod_pwm_init);
module_exit(mod_pwm_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("PWM driver");
MODULE_LICENSE("MSTAR");
