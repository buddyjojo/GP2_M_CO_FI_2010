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
/// file    drv_system_io.c
/// @brief  System Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include "mst_devid.h"
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_system_io.h"
#include "mdrv_system_st.h"
#include "mdrv_system.h"

#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MOD_SYS_DEVICE_COUNT    1

#define SYS_WARNING(fmt, args...)       printk(KERN_WARNING "[SYSMOD][%06d] " fmt, __LINE__, ## args)
#if 0
#define SYS_PRINT(fmt, args...)         printk("[SYSMOD][%06d]     " fmt, __LINE__, ## args)
#else
#define SYS_PRINT(fmt, args...)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32SysMajor;
    int                         s32SysMinor;
    struct cdev                 cDevice;
    struct file_operations      SysFop;
} SysModHandle;


//-------------------------------------------------------------------------------------------------
//  Forward declaration
//-------------------------------------------------------------------------------------------------
static int __init   _mod_sys_init(void);
static void __exit  _mod_sys_exit(void);

static int _mod_sys_open (struct inode *inode, struct file *filp);
static int _mod_sys_release(struct inode *inode, struct file *filp);
static int _mod_sys_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static unsigned int _mod_sys_poll(struct file *filp, poll_table *wait) ;
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static SysModHandle SysDev=
{
    .s32SysMajor=               MDRV_MAJOR_SYSTEM,
    .s32SysMinor=               MDRV_MINOR_SYSTEM,
    .cDevice=
    {
        .kobj=                  {.name= MDRV_NAME_SYSTEM, },
        .owner  =               THIS_MODULE,
    },
    .SysFop=
    {
        .open=                  _mod_sys_open,
        .release=               _mod_sys_release,
        .poll=                  _mod_sys_poll,
        .ioctl=                 _mod_sys_ioctl,
    },
};

extern unsigned char  uart_base[];
extern unsigned int   uart_len;
extern unsigned int   uart_wptr ;
extern unsigned int   uart_rptr;
extern unsigned int   uart_c ;
extern wait_queue_head_t        uart_wait_queue;
extern struct semaphore UartSem;

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
static int __init _mod_sys_init(void)
{
    int         s32Ret;
    dev_t       dev;

    SYS_PRINT("%s is invoked\n", __FUNCTION__);

    if (SysDev.s32SysMajor)
    {
        dev=                    MKDEV(SysDev.s32SysMajor, SysDev.s32SysMinor);
        s32Ret=                 register_chrdev_region(dev, MOD_SYS_DEVICE_COUNT, MDRV_NAME_SYSTEM);
    }
    else
    {
        s32Ret=                 alloc_chrdev_region(&dev, SysDev.s32SysMinor, MOD_SYS_DEVICE_COUNT, MDRV_NAME_SYSTEM);
        SysDev.s32SysMajor=     MAJOR(dev);
    }
    if (0> s32Ret)
    {
        SYS_WARNING("Unable to get major %d\n", SysDev.s32SysMajor);
        return s32Ret;
    }

    cdev_init(&SysDev.cDevice, &SysDev.SysFop);
    if (0!= (s32Ret= cdev_add(&SysDev.cDevice, dev, MOD_SYS_DEVICE_COUNT)))
    {
        SYS_WARNING("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_SYS_DEVICE_COUNT);
        return s32Ret;
    }

    #if 0
    // @FIXME: for GE bug. Slow down MCU clock to 123MHz
    TOP_REG(REG_TOP_MCU) = (TOP_REG(REG_TOP_MCU) & ~(TOP_CKG_MCU_MASK)) | TOP_CKG_MCU_SRC_123;

    MDrv_MIU_Init();
    #endif

    init_waitqueue_head(&uart_wait_queue);

    return 0;
}

static void __exit _mod_sys_exit(void)
{
    SYS_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&SysDev.cDevice);
    unregister_chrdev_region(MKDEV(SysDev.s32SysMajor, SysDev.s32SysMinor), MOD_SYS_DEVICE_COUNT);
}

static int _mod_sys_open (struct inode *inode, struct file *filp)
{
    SYS_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static int _mod_sys_release(struct inode *inode, struct file *filp)
{
    SYS_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_sys_poll(struct file *filp, poll_table *wait)
{
    down(&UartSem) ;
    poll_wait(filp, &uart_wait_queue,  wait);
    up(&UartSem);
    return(uart_c?POLLIN:0);
}

static int _mod_sys_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err= 0;

    // extract the type and number bitfields, and don¡¦t decode
    // wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
    if (_IOC_TYPE(cmd) != SYS_IOCTL_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > IOCTL_SYS_MAXNR) return -ENOTTY;

    // the direction is a bitmask, and VERIFY_WRITE catches R/W
    // transfers. ¡¥Type¡¦ is user oriented, while
    // access_ok is kernel oriented, so the concept of "read" and
    // "write" is reversed
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;

    PROBE_IO_ENTRY(MDRV_MAJOR_SYSTEM, _IOC_NR(cmd));

	switch(cmd)
	{
		case IOCTL_SYS_INIT:
			break;

		case IOCTL_SYS_SET_PANEL_INFO:
			err = MDrv_SYS_SetPanelInfo(arg);
			break;

		case IOCTL_SYS_SET_BOARD_INFO:

			break;

		case IOCTL_SYS_GET_PANEL_RES:
			MDrv_SYS_GetPanelRes(arg);
			break;

		case IOCTL_SYS_READ_GEN_REGISTER:
			MDrv_SYS_ReadGeneralRegister(arg);
			break;

		case IOCTL_SYS_WRITE_GEN_REGISTER:
			MDrv_SYS_WriteGeneralRegister(arg);
			break;
			//#if defined(CONFIG_Triton)
		case IOCTL_SYS_LOAD_AEON:
			MDrv_SYS_LoadAeon(arg);
			break;

		case IOCTL_SYS_RESET_AEON:
			MDrv_SYS_ResetAeon(arg);
			break;

		case IOCTL_SYS_ENABLE_AEON:
			MDrv_SYS_EnableAeon();
			break;

		case IOCTL_SYS_DISABLE_AEON:
			MDrv_SYS_DisableAeon();
			break;

		case IOCTL_SYS_SWITCH_UART:
			MDrv_SYS_SwitchUart(arg);
			break;

		case IOCTL_SYS_IS_AEON_ENABLE:
			err = MDrv_SYS_IsAeonEnable(arg);
			break;
			//#endif

		case IOCTL_SYS_DUMP_AEON_MSG:
			MDrv_SYS_DumpAeonMessage();
			break;

#ifdef IO_SYS_REG_OP
		case IOCTL_SYS_REG_OP:
			err = MDrv_SYS_RegOP(arg);
			break ;
#endif

#ifdef IO_SYS_GET_RAW_UART
        case IOCTL_SYS_SEND_RAW_UART:
            err = MDrv_SYS_SendRawUART(arg);
            break ;

		case IOCTL_SYS_GET_RAW_UART:
			err = MDrv_SYS_GetRawUART(arg);
			break ;
#endif

		case IOCTL_SYS_RELOAD_AEON:
			MDrv_SYS_ReloadAeon(arg) ;
			break ;

		case IOCTL_SYS_TIMER:
			err = MDrv_SYS_Timer(arg);
			break ;

		case IOCTL_SYS_HOTEL_MODE:
			err = MDrv_SYS_HotelMode(arg);
			break ;

		case IOCTL_SYS_HOTEL_MODE_PRINTF:
			err = MDrv_SYS_HotelModePrintf(arg);
			break ;

		case IOCTL_SYS_SETUART_MODE:
			err = MDrv_SYS_SetUartMode(arg);
			break ;

#if 0
		case DEV_SYS_IOC_INIT:
			*((MS_BOOL*)arg)= MDrv_System_Init();
			break;
		case DEV_SYS_IOC_SWITCH_PAD:
			((DevSys_Switch_Pad*)arg)->bSuccess= (MS_U32)MDrv_System_SwitchPad((SYS_PadType)((DevSys_Switch_Pad*)arg)->PadType);
			break;
		case DEV_SYS_IOC_WDT_ENABLE:
			MDrv_System_WDTEnable(*((MS_BOOL*)arg));
			break;
		case DEV_SYS_IOC_WDT_CLEAR:
			MDrv_System_WDTClear();
			break;
		case DEV_SYS_IOC_WDT_LASTSTATUS:
			*((MS_U32*)arg)= MDrv_System_WDTLastStatus();
			break;
		case DEV_SYS_IOC_WDT_SETTIME:
			MDrv_System_WDTEnable(*((MS_U32*)arg));
			break;
		case DEV_SYS_IOC_RESET_CHIP:
			MDrv_System_ResetChip();
			break;
		case DEV_SYS_IOC_RESET_CPU:
			MDrv_System_ResetCPU();
			break;
#endif

		case IOCTL_SYS_CHANGE_UART:
			MDrv_SYS_ChangeUart( arg );
			break;

		case IOCTL_SYS_FLUSH_MEMORY:
			MDrv_SYS_Flush_Memory();
            break;

		case IOCTL_SYS_READ_MEMORY:
			MDrv_SYS_Read_Memory();
            break;

		case IOCTL_SYS_SETSEQFILE:
			err = MDrv_SYS_SetSeqFile(arg);
			break;

		case IOCTL_SYS_GETSPI: //20090724 Terry, URSA ISP Load Code
			err = MDrv_SYS_GetSPI(arg);
			break;
		
		case IOCTL_SYS_MIU_PROTECT: //20091028 Terry, MIU protect
			err = MDrv_SYS_MIU_Protect(arg);
			break;

		case IOCTL_SYS_SETVBUS: // Samuel, bad HDD timeout, 090115
			{
				// Remove it because porting. This feature need be added after porting finish.
				// It used to diconnect vbus for USB disk after a period seconds.

				//extern unsigned int usb_vbus_count_down ;
				//usb_vbus_count_down = 1 ;

			}
			break ;

		case IOCTL_SYS_GETREV:
			err = MDrv_SYS_GetRev(arg);
			break;

		default:
			SYS_WARNING("Unknown ioctl command %d\n", cmd);
			err = -ENOTTY;
	}

    PROBE_IO_EXIT(MDRV_MAJOR_SYSTEM, _IOC_NR(cmd));

	return err;
}


module_init(_mod_sys_init);
module_exit(_mod_sys_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("SYSTEM driver");
MODULE_LICENSE("MSTAR");
