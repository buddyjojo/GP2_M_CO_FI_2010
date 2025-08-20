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
///
/// file    mdrv_ci.c
/// @brief  CI Device Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_CI_C_
#define _DRV_CI_C_

//-----------------------------------------------------------------------------
// Header files
//-----------------------------------------------------------------------------
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#include "mdrv_ci_io.h"

#include "mst_devid.h"
#include "mdrv_ci.h"
#include "chip_int.h"

#include "mdrv_probe.h"

// ----------------------------------------------------------------------------
// Local functions
// ----------------------------------------------------------------------------
static int MDrv_CI_Open(struct inode *inode, struct file *filp);
static int MDrv_CI_Release(struct inode *inode, struct file *filp);
static int MDrv_CI_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct MS_CI_DEV_s {
    struct cdev cdev;      /* Char device structure */
};

struct file_operations _ci_fops = {
    .owner      =    THIS_MODULE,
    .ioctl      =    MDrv_CI_Ioctl,
    .open       =    MDrv_CI_Open,
    .release    =    MDrv_CI_Release,
};

// ----------------------------------------------------------------------------
// Enums
// ----------------------------------------------------------------------------
typedef enum {
    E_CI_INIT_STATE_INIT        = 0x00,
    E_CI_INIT_STATE_CHECKCIS    = 0x01,
    E_CI_INIT_STATE_WRITECOR    = 0x02,
    E_CI_INIT_STATE_RESET       = 0x04,
    E_CI_INIT_STATE_BUFNEGO     = 0x08
} CI_INIT_STATE;

typedef enum
{
    CIPLUS_DATARATE_72 = 0,
    CIPLUS_DATARATE_96,
} CIPLUS_DATARATE_T;

// ----------------------------------------------------------------------------
// Local variables
// ----------------------------------------------------------------------------
static S32                  _s32MajorNumCI  = MDRV_MAJOR_CI;
static S32                  _s32MinorNumCI  = MDRV_MINOR_CI;
static S32                  _s32DevNumCI    = MDRV_CI_NR_DEVS;
static struct MS_CI_DEV_s   *_pstDeviceCI;  /* allocated in scull_init_module */
PCMCIA_INFO_t        _stInfoPCMCIA;
static U8                   u8CIS[MAX_CIS_SIZE];
static U8                   u8CIInitState;

//-----------------------------------------------------------------------------
//                      Function
//-----------------------------------------------------------------------------
#if PCMCIA_IRQ_ENABLE
static irqreturn_t _MDrv_CI_ISR(int irq, void *dev_id)
{
    /* Disable PCMCIA IRQ */
    MDrv_PCMCIA_Enable_Interrupt(DISABLE);

    /* Interupter Handling */


    MDrv_PCMCIA_Clear_Interrupt();

    if (MDrv_CI_PCMCIA_IsModuleStillPlugged())
    {
        /* Enable PCMCIA IRQ */
    MDrv_PCMCIA_Enable_Interrupt(ENABLE);
    }

    return IRQ_HANDLED;
}
#endif

// ----------------------------------------------------------------------------
// Device Methods
// ----------------------------------------------------------------------------

/*
 * Set up the char_dev structure for this device.
 */
static int _MDev_CI_Setup_CDev(struct MS_CI_DEV_s *dev, int index)
{
    S32 err, devno = MKDEV(_s32MajorNumCI, _s32MinorNumCI);

    cdev_init(&dev->cdev, &_ci_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &_ci_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    /* Fail gracefully if need be */
    if (err)
        printk(KERN_NOTICE "Error %d adding devCI %d", err, index);
    return err;
}

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
static void _MDev_CI_CleanUp_Module(void)
{
    U32 i;
    dev_t devno = MKDEV(_s32MajorNumCI, _s32MinorNumCI);

    /* Get rid of our char dev entries */
    if (_pstDeviceCI) {
        for (i = 0; i < _s32DevNumCI; i++) {
            cdev_del(&_pstDeviceCI[i].cdev);
        }
        kfree(_pstDeviceCI);
    }

    /* cleanup_module is never called if registering failed */
    unregister_chrdev_region(devno, _s32DevNumCI);
}

/*
 * Open and close
 */
int MDrv_CI_Open(struct inode *inode, struct file *filp)
{
    struct MS_CI_DEV_s *dev; /* device information */

    dev = container_of(inode->i_cdev, struct MS_CI_DEV_s, cdev);
    filp->private_data = dev; /* for other methods */

    /* When Card is always inserted,
        MDrv_CI_PCMCIA_HwRstPCMCIA() is needed to repeat to call
        MDev_CI_Open(), ICOTL(), and MDev_CI_Release.
    */
    //wgkwak in LGE
    //When CI driver is opened, It's not used in LG S/W.
    //MDrv_CI_PCMCIA_HwRstPCMCIA(PCMCIA_RESETDELAY_ZERO);

    memset(gCiSlot, 0x00, sizeof(gCiSlot));
    memset((void *)&_stInfoPCMCIA, 0x00, sizeof(PCMCIA_INFO_t));

    u8CIInitState = E_CI_INIT_STATE_INIT;

    return 0;    /* success */
}

int MDrv_CI_Release(struct inode *inode, struct file *filp)
{
    return 0;
}

int MDrv_CI_Ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg)
{
    S32                err = 0;
    PCMCIA_HANDLE    hPCMCIA = (PCMCIA_HANDLE)&_stInfoPCMCIA;
    CI_HANDLE        hCI = &gCiSlot[0];
    U8                bDataAvailable;
    U8                bIIR;
    S8                *ps8WriteBuf;
    U16                u16DataSize, i;
    U8               u8OnOff_flag;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (_IOC_TYPE(cmd) != MDRV_CI_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > MDRV_CI_IOC_MAXNR) return -ENOTTY;

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

    PROBE_IO_ENTRY(MDRV_MAJOR_CI, _IOC_NR(cmd));

    switch (cmd)
    {
        case IOCTL_CI_DETECT_CARD:
            if (__put_user(MDrv_CI_PCMCIA_IsModuleStillPlugged(), (B16 __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case IOCTL_CI_RESET:
            u8CIInitState &= (~E_CI_INIT_STATE_BUFNEGO);
            if (!(u8CIInitState & E_CI_INIT_STATE_WRITECOR))
            {
                printk("[IOCTL_CI_RESET]CI INIT Warning! You should make IOCTL_CI_CHECK_CIS and then IOCTL_CI_WRITE_COR first!\n");
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }

            if (MDrv_CI_PCMCIA_IsModuleStillPlugged())
            {
#if 0
                if (!MDrv_CI_PCMCIA_ResetInterface())
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
                else
#else
                if (!MDrv_CI_PCMCIA_ResetInterface())
                {
                    printk("[IOCTL_CI_RESET] MDrv_CI_PCMCIA_ResetInterface() NG!\n");
                }
#endif
                    u8CIInitState |= E_CI_INIT_STATE_RESET;
            }
            else
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case IOCTL_CI_CHECK_CIS:
            u8CIInitState = E_CI_INIT_STATE_INIT;

            if (MDrv_CI_PCMCIA_IsModuleStillPlugged())
            {
                //wgkwak in LGE
                MDrv_CI_PCMCIA_HwRstPCMCIA(PCMCIA_RESETDELAY_MAX);    // MUST for re-intialization.
                MDrv_CI_PCMCIA_ReadAttribMem(0, u8CIS);
#if 0
                if (!MDrv_CI_PCMCIA_ParseAttribMem(u8CIS, MAX_CIS_SIZE, &_stInfoPCMCIA))
                    return -EFAULT;
#else
                MDrv_CI_PCMCIA_ParseAttribMem(u8CIS, MAX_CIS_SIZE, &_stInfoPCMCIA);
                //wgkwak in LGE
                if((!_stInfoPCMCIA.bCAMIsInserted) || (_stInfoPCMCIA.wPCMCIAStdRev!=0x0500)) return -EFAULT;
#endif
                u8CIInitState |= E_CI_INIT_STATE_CHECKCIS;
            }
            else
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case IOCTL_CI_WRITE_COR:
            if (!(u8CIInitState & E_CI_INIT_STATE_CHECKCIS))
            {
                printk("[IOCTL_CI_WRITE_COR]CI INIT Warning! You should make IOCTL_CI_CHECK_CIS first!\n");
                return -EFAULT;
            }

            if (MDrv_CI_PCMCIA_IsModuleStillPlugged())
            {
                MDrv_CI_L2_WriteCOR(hPCMCIA, &_stInfoPCMCIA, 0);
                u8CIInitState |= E_CI_INIT_STATE_WRITECOR;
            }
            else
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case IOCTL_CI_SWITCH_BYPASS_MODE:
            MDrv_CI_PCMCIA_SwitchBypassMode(arg);
            break;

        case IOCTL_CI_READ_DATA:
            if (!(u8CIInitState & E_CI_INIT_STATE_BUFNEGO))
            {
                printk("[IOCTL_CI_READ_DATA]Warning! You should initialize PCMCIA Card first!\n");
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }

            {
                U8 u8TempBuf[MAX_CI_TPDU];

                /* Because PCMCIA will have a residual value in CI_PHYS_REG_SIZE, u16DataSize should be zero when no data is available. */
                bDataAvailable = MDrv_CI_PCMCIA_ReadIOMem(hPCMCIA, CI_PHYS_REG_COMMANDSTATUS);
                if (!(bDataAvailable & CISTATUS_DATAAVAILABLE))
                    u16DataSize = 0;
                else
                    u16DataSize = (U16)MDrv_CI_PCMCIA_ReadIOMem(hPCMCIA, CI_PHYS_REG_SIZEHIGH) << 8 | (U16)MDrv_CI_PCMCIA_ReadIOMem(hPCMCIA, CI_PHYS_REG_SIZELOW);
                
                if (u16DataSize > hCI->IOBuffer_Host2CI.wBufferSize)
                {
                    printk("[MDev_CI_Ioctl]Oops! IOCTL_CI_READ_DATA: Data Size is too big!\n");
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }

                for (i = 0; i < u16DataSize; i++)
                {
                    u8TempBuf[i] = MDrv_CI_PCMCIA_ReadIOMem(hPCMCIA, CI_PHYS_REG_DATA);
                }

                if (__put_user(u16DataSize, &(((CI_DATA_INFO_t __user *)arg)->u16DataSize)))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
                if (copy_to_user((((CI_DATA_INFO_t __user *)arg)->pu8Data), u8TempBuf, u16DataSize))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
#if DRV_CI_DEBUG_ENABLE
                else
                {
                    U32 i;
                    
                    printk("RX: ");
                    for ( i = 0; i < u16DataSize; i++ )
                    {
                        printk("%02X ", (U8)u8TempBuf[i]);
                    }
                    printk("\n");
                }
#endif
            }
            break;

        case IOCTL_CI_WRITE_DATA:
            if (!(u8CIInitState & E_CI_INIT_STATE_BUFNEGO))
            {
                printk("[IOCTL_CI_WRITE_DATA]Warning! You should initialize PCMCIA Card first!\n");
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }

            if (__get_user(ps8WriteBuf, &(((CI_DATA_INFO_t __user *)arg)->pu8Data)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }

            if (__get_user(u16DataSize, &(((CI_DATA_INFO_t __user *)arg)->u16DataSize)))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }

            if (TRUE != MDrv_CI_L2_ReallyWriteData(hCI, u16DataSize, ps8WriteBuf))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }
#if DRV_CI_DEBUG_ENABLE
            else
            {
                U32 i;
                
                printk("TX: ");
                for ( i = 0; i < u16DataSize; i++ )
                {
                    printk("%02X ", (U8)ps8WriteBuf[i]);
                }
                printk("\n");
            }
#endif
            break;

        case IOCTL_CI_NEGOTIATE_BUF_SIZE:
            if (!(u8CIInitState & E_CI_INIT_STATE_RESET))
            {
                printk("[IOCTL_CI_NEGOTIATE_BUF_SIZE]CI INIT Warning! You should make IOCTL_CI_RESET first!\n");
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }

            if (MDrv_CI_PCMCIA_IsModuleStillPlugged())
            {
                if (MDrv_CI_L2_CheckConfig(hPCMCIA, &_stInfoPCMCIA, 0))
                {
                    CI_HANDLE hCI = &gCiSlot[0];

                    u8CIInitState |= E_CI_INIT_STATE_BUFNEGO;
                    DRV_CI_DEBUG("PCMCIA Config ok\n");
                    if (__put_user(hCI->IOBuffer_Host2CI.wBufferSize, (U16 __user *)arg))
                    {
                        PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                        return -EFAULT;
                    }
                }
                else
                {
                    DRV_CI_DEBUG("PCMCIA Config not ok\n");
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
            }
            else
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;

        case IOCTL_CI_READ_DA_STATUS:
            if (!(u8CIInitState & E_CI_INIT_STATE_BUFNEGO))
            {
                if (__put_user(FALSE, (B16 __user *)arg))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
            }

            bDataAvailable = MDrv_CI_PCMCIA_ReadIOMem(hPCMCIA, CI_PHYS_REG_COMMANDSTATUS);
            if (bDataAvailable & CISTATUS_DATAAVAILABLE)
            {
                if (__put_user(TRUE, (B16 __user *)arg))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
            }
            else
            {
                if (__put_user(FALSE, (B16 __user *)arg))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
            }
            break;

        case IOCTL_CI_PLUS_SET_PHY_RESET:
            MDrv_CI_PCMCIA_HwRstPCMCIA(PCMCIA_RESETDELAY_ZERO);
            u8CIInitState = E_CI_INIT_STATE_INIT;
            break;

        case IOCTL_CI_PLUS_READ_IIR_STATUS:
            bIIR = MDrv_CI_PCMCIA_ReadIOMem(hPCMCIA, CI_PHYS_REG_COMMANDSTATUS);
            if (bIIR & CISTATUS_IIR)
            {
                if (__put_user(TRUE, (B16 __user *)arg))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
            }
            else
            {
                if (__put_user(FALSE, (B16 __user *)arg))
                {
                    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                    return -EFAULT;
                }
            }
            break;

        case IOCTL_CI_PLUS_GET_DATA_RATE:
            if (__put_user(CIPLUS_DATARATE_72, (U8 __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }
            break;
			
        case IOCTL_CI_POWER_ON_OFF:
            if (copy_from_user(&u8OnOff_flag, (U8 __user *) arg, sizeof(U8)))
			{
				PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
 	            return -EFAULT;
			}
            if (0 == u8OnOff_flag)
                MDrv_CI_PCMCIA_SwitchPower(1);
            else
                MDrv_CI_PCMCIA_SwitchPower(0);
            break;

        case IOCTL_CI_CHECK_CIPLUS_CAPABILITY:
            if (__put_user(_stInfoPCMCIA.bCI_PLUS, (U8 __user *)arg))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }            
            break;

        case IOCTL_CI_GET_MANUFACTURER_INFO:
            if (copy_to_user((((CI_MANUFACTURER_INFO_t __user *)arg)->pu8Manufacturer), _stInfoPCMCIA.pszManufacturerName, 
                                        min(MAX_PCMCIA_STRLEN, (((CI_MANUFACTURER_INFO_t __user *)arg)->u8ManufacturerSize))))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }           

            if (copy_to_user((((CI_MANUFACTURER_INFO_t __user *)arg)->pu8Product), _stInfoPCMCIA.pszProductName, 
                                        min(MAX_PCMCIA_STRLEN, (((CI_MANUFACTURER_INFO_t __user *)arg)->u8ProductSize))))
            {
                PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
                return -EFAULT;
            }                      
            break;
            
        default:
            return -ENOTTY;
    }


    PROBE_IO_EXIT(MDRV_MAJOR_CI, _IOC_NR(cmd));
    return 0;
}

#if PCMCIA_IRQ_ENABLE
static unsigned int u32PCM2MCUIntRegister;
#endif

static int __init MDev_CI_Module_Init(void)
{
    //U32 result = 0, i;
    int result = 0, i;        /* STATIC ANALYSIS(NO_EFFECT) ¼öÁ¤ (dreamer@lge.com)*/
    dev_t dev_Number = 0;

    printk("CI driver inits\n");
    MDrv_PCMCIA_Init(&_stInfoPCMCIA);

   /*
    * Get a range of minor numbers to work with, asking for a dynamic
    * major unless directed otherwise at load time.
    */
    if (_s32MajorNumCI) {
        dev_Number = MKDEV(_s32MajorNumCI, _s32MinorNumCI);
        result = register_chrdev_region(dev_Number, _s32DevNumCI, "devCI");
    } else {
        result = alloc_chrdev_region(&dev_Number, _s32MinorNumCI, _s32DevNumCI, "devCI");
        _s32MajorNumCI = MAJOR(dev_Number);
    }
    if (result < 0) {
        printk(KERN_INFO "devCI: can't get major %d\n", _s32MajorNumCI);
        return result;
    }

    /*
     * allocate the devices -- we can't have them static, as the number
     * can be specified at load time
     */
    _pstDeviceCI = kmalloc(_s32DevNumCI * sizeof(struct MS_CI_DEV_s), GFP_KERNEL);
    if (!_pstDeviceCI) {
        result = -ENOMEM;
        goto fail;  /* Make this more graceful */
    }
    memset(_pstDeviceCI, 0, _s32DevNumCI * sizeof(struct MS_CI_DEV_s));

    /* Initialize each device. */
    for (i = 0; i < _s32DevNumCI; i++) {
        if (_MDev_CI_Setup_CDev(&_pstDeviceCI[i], i))
            goto fail;
    }

#if PCMCIA_IRQ_ENABLE
    if(0 == u32PCM2MCUIntRegister) {
    result = request_irq(E_IRQ_PCM2MCU, (void *)_MDrv_CI_ISR, SA_INTERRUPT, "CI", NULL);
	    if (result) {
        printk("CI can't get assigned irq !!\n");
			return -EBUSY;
	    }

	    u32PCM2MCUIntRegister = 1;
    } else {
		disable_irq(E_IRQ_PCM2MCU);
		enable_irq(E_IRQ_PCM2MCU);
	}
#endif

    return 0;

fail:
    _MDev_CI_CleanUp_Module();

    return result;
}

static void __exit MDev_CI_Module_Exit(void)
{
    dev_t dev_Number = MKDEV(_s32MajorNumCI, _s32MinorNumCI);

    /* cleanup_module is never called if registering failed */
    unregister_chrdev_region(dev_Number, _s32DevNumCI);

    printk("CI driver exits\n");
}

module_init(MDev_CI_Module_Init);
module_exit(MDev_CI_Module_Exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("CI driver");
MODULE_LICENSE("MSTAR");

#endif // #ifndef _DRV_CI_C_
