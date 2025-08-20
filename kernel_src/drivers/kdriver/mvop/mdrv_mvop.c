
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
#include "mdrv_mvop.h"
#include "mhal_mvop.h"

#include "mdrv_probe.h"

#define MVOP_PRINT(fmt, args...)//         printk("[MVOPMOD][%06d]     " fmt, __LINE__, ## args)

// add extern function to avoid compilation warning msg. by LGE. jjab
extern void MHal_MVOP_SetH264HardwireMode(void);
extern void MHal_MVOP_SetRMHardwireMode(void);

struct MVOP_Dev  g_MVOP_Dev;

struct file_operations MVOP_Fops = {
	.owner =    THIS_MODULE,
	.ioctl =    MDrv_MVOP_Ioctl,
	.open =     MDrv_MVOP_Open,
	.release =  MDrv_MVOP_Release,
};

S32 MDrv_MVOP_Ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
{
    S32         err= 0;
    B16 bMVOPModeHardwire = 0;
    //U32         retval=0;
    //int         intval;
    //U16         u16FrameCnt = 0;
    //MVD_PictureData picData = {0, 0, 0, 0, 0};
    //MVD_IFRAME_ADDR iFrameAddr;

    //MVD_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((MVOP_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> MVOP_IOC_MAXNR))
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


    PROBE_IO_ENTRY(MDRV_MAJOR_MVOP, _IOC_NR(cmd));

    switch(cmd)
    {
    case MVOP_IOC_INIT:
        MHal_MVOP_Init();
        break;
    case MVOP_IOC_HARDWAREMODE:
        //MVOP_PRINT("ioctl: MVD play mode: %d\n", (u8)arg);
        MHal_MVOP_Input_Mode(MVOPINPUT_HARDWIRE,NULL);
        //return MHal_MVD_Play((U8)arg);
        break;
    case MVOP_IOC_HARDWIREMODE:
	    bMVOPModeHardwire = arg;
		MVOP_PRINT("ioctl: MVOP set Hardwire mode: %d\n", (u8)bMVOPModeHardwire);
		if(0==bMVOPModeHardwire)
		{
            MHal_MVOP_Input_Mode(MVOPINPUT_HARDWIRE,NULL);
        }
        else if(1==bMVOPModeHardwire)
        {
            //H.264
            MHal_MVOP_Input_Mode(MVOPINPUT_HARDWIRE,NULL);
            MHal_MVOP_SetH264HardwireMode();
        }
        else if(2==bMVOPModeHardwire)
        {
            //RM
            MHal_MVOP_Input_Mode(MVOPINPUT_HARDWIRE,NULL);
            MHal_MVOP_SetRMHardwireMode();
        }
        else if(3==bMVOPModeHardwire)
        {
            //JPEG
            MHal_MVOP_Input_Mode(MVOPINPUT_HARDWIRE,NULL);
            MHAL_MVOP_SetJpegHardwireMode();
        }
        else
        {
            //wrong mode
            MVOP_PRINT("ioctl: Wrong to set MVOP Hardwire mode: %d\n", (u8)bMVOPModeHardwire);
        }
        break;
    case MVOP_IOC_HARDWIRECLIPMODE:
    	{
        	MVOPINPUTPARAM param;
        	U32 err;
        	err = __copy_from_user(&param,(MVOPINPUTPARAM*)arg,sizeof(MVOPINPUTPARAM));
        	if (err != 0)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
        		return -EFAULT;
            }
        	MHal_MVOP_Input_Mode(MVOPINPUT_HARDWIRECLIP,&param);
    	}
        break;
    case MVOP_IOC_HARDWARECLIPMODE:
    	{
        	MVOPINPUTPARAM param;
        	U32 err;
        	err = __copy_from_user(&param,(MVOPINPUTPARAM*)arg,sizeof(MVOPINPUTPARAM));
        	if (err != 0)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
        		return -EFAULT;
            }
        	MHal_MVOP_Input_Mode(MVOPINPUT_HARDWIRECLIP,&param);
    	}
        break;
    case MVOP_IOC_MCUMODE:
        {
        	MVOPINPUTPARAM param;
        	U32 err;
        	err = __copy_from_user(&param,(MVOPINPUTPARAM*)arg,sizeof(MVOPINPUTPARAM));
        	if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
        		return -EFAULT;
            }
        	MHal_MVOP_Input_Mode(MVOPINPUT_MCUCTRL,&param);
    	}
        break;
    case MVOP_IOC_SETYUVADDR:
        {
        	MVOPINPUTPARAM param;
        	U32 err;
        	err = __copy_from_user(&param,(MVOPINPUTPARAM*)arg,sizeof(MVOPINPUTPARAM));
        	if (err != 0)    // check the return value of copy_to_user/copy_from_user (dreamer@lge.com)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
        		return -EFAULT;
            }
        	MHal_MVOP_Set_YUV_Address(MVOPINPUT_MCUCTRL,&param);
    	}
        break;
    case MVOP_IOC_SETTIMING:
		{
			MS_MVOP_TIMING timing;
			U32 err;
			err = __copy_from_user(&timing,(MS_MVOP_TIMING*)arg,sizeof(MS_MVOP_TIMING));
        	if (err != 0)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
        		return -EFAULT;
            }
        	MHal_MVOP_SetOutputTiming ( &timing );
    	}
        break;
	case MVOP_IOC_SETMVOPSYNCCLK:
		{
			MS_MVOP_TIMING timing;
			U32 err;
			err = __copy_from_user(&timing,(MS_MVOP_TIMING*)arg,sizeof(MS_MVOP_TIMING));
        	if (err != 0)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
        		return -EFAULT;
            }
        	MHal_MVOP_SetMVOPSynClk( &timing );
    	}
        break;
  	case MVOP_IOC_ENABLEBLACKBG:
	{

		MHal_MVOP_EnableBlackBg();

  	}
		break;
  	case MVOP_IOC_ENABLEINTERLACE:
    	{

    		B16 interlace;
			interlace = arg;
    		MHal_MVOP_Output_EnableInterlace(interlace);
    	}
    	break;
    case MVOP_IOC_SETMLINKMODE:
        {
            B16 bMlinkMode;
			bMlinkMode = arg;
			MVOP_PRINT("ioctl: MVOP set Mlink mode: %d\n", (u8)bMlinkMode);
            MHal_MVOP_SetMlinkMode(bMlinkMode);
        }
        break;
    case MVOP_IOC_SETMLINKBYPASS:
        {
            B16 bMlinkBypassMode;
			bMlinkBypassMode = arg;
			MVOP_PRINT("ioctl: MVOP set Mlink bypass mode: %d\n", (u8)bMlinkBypassMode);
            MHal_MVOP_SetMlinkByPassMode(bMlinkBypassMode);
        }
        break;
    case MVOP_IOC_SETMVOPOPERATION:
        {
            B16 bMVOPOperation;
			bMVOPOperation = arg;
			MVOP_PRINT("ioctl: MVOP set mvop enable/disable: %d\n", (u8)bMVOPOperation);
            MHal_MVOP_Enable(bMVOPOperation);
        }
        break;
	case MVOP_IOC_SET_TEST_PATTERN:		// lemonic LGE 080908
		{
			MVOP_SET_TEST_PATTERN_t param;
			U32 err;
			err = __copy_from_user(&param,(MVOP_SET_TEST_PATTERN_t*)arg,sizeof(MVOP_SET_TEST_PATTERN_t));
        	if (err != 0)
        	{
        	    PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
        		return -EFAULT;
            }
			MHal_MVOP_SetTestPattern(param.u8PatternType, param.u8Pattern);
		}
		break;
	//LGE gbtogether(081217) ->to fix ChromaArtifact when 420to422 by Junyou.Lin
    case MVOP_IOC_SETUVSHIT:
        {
            MHal_MVOP_SetUVShift((B16)arg);
        }
        break;

    //junyou add for H264 thumbnail<20091001>
    case MVOP_IOC_SETH264_ORDER:
        {
            MHal_MVOP_SetH264_Order((B16)arg);
        }
        break;

    default:
        //MVD_WARNING("ioctl: unknown command\n");
        PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
        return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_MVOP, _IOC_NR(cmd));
    return 0;
}

}
S32 MDrv_MVOP_Open(struct inode *inode, struct file *filp)
{
	return 0;

}
S32 MDrv_MVOP_Release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int __init MDrv_MVOP_ModuleInit(void)
{
    //int         s32Ret;
    dev_t       devno;
    //U32         result;
    int         result;		/* STATIC ANALYSIS(NO_EFFECT) ¼öÁ¤ (dreamer@lge.com)*/

    devno = MKDEV(MDRV_MAJOR_MVOP, MDRV_MINOR_MVOP);
    result = register_chrdev_region(devno, 1, "drvMVOP");
    if ( result < 0)
   	{
   		printk(KERN_WARNING "MVOP: can't get major %d\n", MDRV_MAJOR_MVOP);
		return result;


   	}

   	cdev_init(&(g_MVOP_Dev.cdev), &MVOP_Fops);
	g_MVOP_Dev.cdev.owner = THIS_MODULE;
	g_MVOP_Dev.cdev.ops = &MVOP_Fops;
	result = cdev_add (&(g_MVOP_Dev.cdev), devno, 1);
	/* Fail gracefully if need be */
	if (result)
		printk(KERN_NOTICE "Error add MVOP device");


    MHal_MVOP_Init();

    return 0;
}

static void __exit MDrv_MVOP_ModuleExit(void)
{
    dev_t devno = MKDEV(MDRV_MAJOR_MVOP, MDRV_MINOR_MVOP);
    unregister_chrdev_region(devno, 1);
}


module_init(MDrv_MVOP_ModuleInit);
module_exit(MDrv_MVOP_ModuleExit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MVOP driver");
MODULE_LICENSE("MSTAR");
