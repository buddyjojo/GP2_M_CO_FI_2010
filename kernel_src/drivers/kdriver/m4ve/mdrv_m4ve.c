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
#include "M4VE_chip.h"

#if defined(_MIPS_PLATFORM_)&&defined(_M4VE_T3_)
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
#elif defined(_M4VE_BIG2_)&&defined(_MIPS_PLATFORM_)
#include "cyg/hal/drv_api.h"
#include <string.h>
#endif

#ifdef CONFIG_MSTAR_TITANIA
#else
#if defined(_MIPS_PLATFORM_)&&defined(_M4VE_T3_)
#include "mst_devid.h"
#include "./Aeon/drvM4VE.h"
#else
#include "drvM4VE.h"
#endif
#include "memmap.h"
#include "mdrv_m4ve.h"

#include "mdrv_probe.h"


int wbits_index=0;
int rbits_index=0;
int enc_frame_count=0;
static U32 _u32M4veEvent = 0;
BITSFRAME_INFO bitsframe[MAX_BITS_FRAME];
U8 m4ve_vol[20];
int VOL_take=0;
extern volatile M4VE_STAT encode_state;

#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
struct M4VE_Dev  g_M4VE_Dev;
static wait_queue_head_t        _m4ve_wait_queue;
static spinlock_t               _spinlock;

S32 MDrv_M4VE_Ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg);
S32 MDrv_M4VE_Open(struct inode *inode, struct file *filp);
S32 MDrv_M4VE_Release(struct inode *inode, struct file *filp);
S32 MDrv_M4VE__mmap(struct file *filp, struct vm_area_struct *vma);
static unsigned int MDrv_M4VE_poll(struct file *filp, poll_table *wait);

struct file_operations M4VE_Fops = {
	.owner =    THIS_MODULE,
	.ioctl =    MDrv_M4VE_Ioctl,
	.open =     MDrv_M4VE_Open,
	.release =  MDrv_M4VE_Release,
	.mmap =	    MDrv_M4VE__mmap,
	.poll =     MDrv_M4VE_poll,
};
#else
sem_t        _m4ve_wait_queue;
pthread_mutex_t _spinlock;
#define wake_up(a) sem_post(a)
#if defined(_M4VE_BIG2_)&&defined(_MIPS_PLATFORM_)
#define spin_lock(a) cyg_drv_dsr_lock() //pthread_mutex_lock
#define spin_unlock(a) cyg_drv_dsr_unlock() //pthread_mutex_unlock
#else
#define spin_lock pthread_mutex_lock
#define spin_unlock pthread_mutex_unlock
#endif
#define spin_lock_init pthread_mutex_init
#endif //#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)

#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
S32 MDrv_M4VE_Open(struct inode *inode, struct file *filp)
{
	return 0;

}

S32 MDrv_M4VE_Release(struct inode *inode, struct file *filp)
{
//    free_irq(E_IRQ_M4VE, NULL)

	return 0;
}

S32 MDrv_M4VE__mmap(struct file *filp, struct vm_area_struct *vma)
{
	return 0;
}

static unsigned int MDrv_M4VE_poll(struct file *filp, poll_table *wait)
{
    unsigned int ret;
//    printf("MDrv_M4VE_poll is invoked 0x%x\n", &_m4ve_wait_queue);
    poll_wait(filp, &_m4ve_wait_queue,  wait);
  //  printf("MDrv_M4VE_poll is waken up %d\n", _u32M4veEvent);
    spin_lock(&_spinlock);
    ret = _u32M4veEvent ? POLLIN : 0;
    M4VE_DEBUG("leave MDrv_M4VE_poll, %d %d\n", ret, _u32M4veEvent);
    spin_unlock(&_spinlock);
    return  ret;//  | POLLPRI;
}

static int __init MDrv_M4VE_ModuleInit(void)
{
    dev_t       devno;
    int         result;
    M4VE_DEBUG("in MDrv_M4VE_ModuleInit\n");
    devno = MKDEV(MDRV_MAJOR_M4VE, MDRV_MINOR_M4VE);
    result = register_chrdev_region(devno, 1, MDRV_NAME_M4VE);
    if ( result < 0)
   	{
   		printf(KERN_WARNING "M4VE: can't get major %d\n", MDRV_MAJOR_M4VE);
		return result;
   	}
	g_M4VE_Dev.cdev.owner = THIS_MODULE;
 	g_M4VE_Dev.cdev.kobj.name = MDRV_NAME_M4VE;
	g_M4VE_Dev.cdev.ops = &M4VE_Fops;
   	cdev_init(&(g_M4VE_Dev.cdev), &M4VE_Fops);
    result = cdev_add (&(g_M4VE_Dev.cdev), devno, 1);
	/* Fail gracefully if need be */
    if (result) {
		printf(KERN_NOTICE "Error add M4VE device");
    }
    spin_lock_init(&_spinlock);
    init_waitqueue_head(&_m4ve_wait_queue);
    return 0;
}

static void __exit MDrv_M4VE_ModuleExit(void)
{
    cdev_del(&g_M4VE_Dev.cdev);
    unregister_chrdev_region(MKDEV(MDRV_MAJOR_M4VE, MDRV_MINOR_M4VE), 1);
}
#endif //#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
void MDrv_M4VE_ENC_Continue(void)
{
    MHal_M4VE_set_outbitsbuf();
}

void MDrv_M4VE_clear_bitsbuf(U32 clear_num)
{
    spin_lock(&_spinlock);
    while(clear_num) {
        bitsbuf_used[rbits_index] = 0;
        M4VE_DEBUG("clear rbits_index: %d\n", rbits_index);
        rbits_index = MUX((rbits_index+1)==MAX_BITS_FRAME, 0, rbits_index+1);
        clear_num--;
    };
    spin_unlock(&_spinlock);
}

int MDrv_M4VE_getbits(BITSFRAME_INFO *bits_info)
{
    int i=0;
    int index;
#if !(defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_))
    sem_wait(&_m4ve_wait_queue);
#endif
    spin_lock(&_spinlock);
    index = rbits_index;
    while(_u32M4veEvent!=0) {
        bits_info[i].miuAddress = bitsframe[index].miuAddress;
        bits_info[i].miuPointer = bitsframe[index].miuPointer;
        bits_info[i].size = bitsframe[index].size;
        bits_info[i].is_frame_done = bitsframe[index].is_frame_done;
        bits_info[i].voptype = bitsframe[index].voptype;
        bits_info[i].is_more_bits = 1;
        bits_info[i].IVOP_address= bitsframe[index].IVOP_address;
//        printk("in MDrv_M4VE_getbits rbits:%d %d %d 0x%x\n", index
//            , _u32M4veEvent, bits_info[i].is_frame_done, bits_info[i].IVOP_address);
        _u32M4veEvent ^= (1<<index);
        index = MUX((index+1)==MAX_BITS_FRAME, 0, index+1);
        i++;
        if (i>3) {
            printf("MDrv_M4VE_getbits error!! use too many bits buffer\n");
        }
    };
    bits_info[i-1].is_more_bits = 0;
    spin_unlock(&_spinlock);

//    out_frame_count++;
    return 0;
}
int M4VE_getbits_callback(U32 miuaddr, U32 miupointer, long size
            , int is_frame_done, U8 voptype, U32 Y_start)
{

    bitsframe[wbits_index].miuAddress = miuaddr;
    bitsframe[wbits_index].miuPointer = miupointer;
    bitsframe[wbits_index].size = size;
    bitsframe[wbits_index].is_frame_done = is_frame_done;
    bitsframe[wbits_index].voptype = voptype;
    if (bitsframe[wbits_index].size<0) {
        printf("M4VE_getbits_callback %d %d %d\n", (int)size, (int)bitsframe[wbits_index].size, (int)voptype);
    }
#ifdef _WIN32
    //we don't lock on mips, Aeon, ARM because these codes are in ISR in those platforms.
    spin_lock(&_spinlock);
#endif
    _u32M4veEvent |= 1<<wbits_index;
    bitsbuf_used[wbits_index] = 1;
#ifdef _WIN32
    spin_unlock(&_spinlock);
#endif
//    printk("next: 0x%x  prev: 0x%x\n", _m4ve_wait_queue.task_list.next, _m4ve_wait_queue.task_list.prev);
//    enc_state = 1;
    if (voptype==I_VOP) {
        bitsframe[wbits_index].IVOP_address = Y_start;
//        printk("IVOP address 0x%x wbits_index %d\n", Y_start, wbits_index);
    } else {
        bitsframe[wbits_index].IVOP_address = 0;
//        printk("IVOP address = 0\n");
    }
    wbits_index = MUX((wbits_index+1)==MAX_BITS_FRAME, 0, wbits_index+1);
    //printk("next: 0x%x  prev: 0x%x\n", _m4ve_wait_queue.task_list.next, _m4ve_wait_queue.task_list.prev);
    if (is_frame_done || bitsbuf_used[wbits_index]) {
        wake_up(&_m4ve_wait_queue);
    }
    M4VE_DEBUG("_u32M4veEvent: %d %d\n", _u32M4veEvent, wbits_index);
    return 0;
}

int MDrv_M4VE_ENC_OneFrame(U32 YUV_addr)
{
#ifdef _FRAMEPTR_IN_
    U32 enc_frame_ind;
#else
    char realname[100];
    char tmpname[100];
#endif

    msAPI_M4VE_EnOneFrm(enc_frame_count, YUV_addr
#ifdef _FRAMEPTR_IN_
    , &enc_frame_ind
#else
    , realname, tmpname
#endif
    );
    enc_frame_count++;
    return 0;
}

void MDrv_M4VE_GetVOL(U32 *vol_addr)
{
    *vol_addr = (U32)m4ve_vol;
}

#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
S32 MDrv_M4VE_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
S32 MDrv_M4VE_Ioctl(int m4ve_fd, unsigned int cmd, unsigned long arg)
#endif
{
    PVR_Info *pappPVR_Info;
    BITSFRAME_INFO *bits_info;
//    printf("in MDrv_M4VE_Ioctl 0x%x 0x%x 0x%x\n", cmd, M4VE_IOC_INIT, M4VE_IOC_PLAY);

    PROBE_IO_ENTRY(MDRV_MAJOR_M4VE, _IOC_NR(cmd));

    switch(cmd)
    {
    case M4VE_IOC_INIT:
        {
#if !(defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_))
            sem_init(&_m4ve_wait_queue, 0, 0);
            pthread_mutex_init(&_spinlock, NULL);
#endif
            pappPVR_Info = (PVR_Info *)arg;
            rbits_index = wbits_index = 0;
            enc_frame_count = 0;
//            out_frame_count = 0;
            _u32M4veEvent = 0;
            VOL_take = 1;
            printf("in MDrv_M4VE.c W=%d H=%d\n", pappPVR_Info->width, pappPVR_Info->height);
            msAPI_M4VE_Init(pappPVR_Info->width, pappPVR_Info->height, pappPVR_Info->BitsBuffStart, pappPVR_Info->BitsBuffSize);
        }
        break;
    case M4VE_IOC_PLAY:
    	{
    		M4VE_DEBUG("ioctl: H264 play mode: 0x%x\n", (U32)arg);
            //M4VE_Test_AP(6, input_name, real_name, tmp_name);
        }
        break;
    case M4VE_IOC_ENC_ONEFRAME:
        {
            M4VE_DEBUG("ioctl: MDRV: M4VE_ENC_ONEFRAME: 0x%x\n", (U32)arg);
            MDrv_M4VE_ENC_OneFrame(arg);
        }
        break;
    case M4VE_IOC_ENC_SKIPVOP:
        {
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_ENC_SKIPVOP: 0x%x\n", (U32)arg);
            MHal_M4VE_EncSkipVOP();
        }
        break;
    case M4VE_IOC_GETBITS:
        {
            bits_info = (BITSFRAME_INFO *)arg;
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_GETBITS\n");
            MDrv_M4VE_getbits(bits_info);
        }
        break;

    case M4VE_IOC_CLEAR_BITSBUF:
        {
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_CLEAR_BITSBUF\n");
            MDrv_M4VE_clear_bitsbuf(arg);
        }
        break;

    case M4VE_IOC_ENC_CONTINUE:
        {
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_ENC_CONTINUE\n");
            MDrv_M4VE_ENC_Continue();
        }
        break;
    case M4VE_IOC_SET_FRAMERATE:
        {
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_SET_FRAMERATE: 0x%x\n", (U32)arg);
            MHal_M4VE_SetFramerate((U32)arg);
        }
        break;
    case M4VE_IOC_SET_BITRATE:
        {   //set_codec -> set_QP -> set_Bitrate -> Init
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_ENC_SETBITRATE: 0x%x\n", (U32)arg);
            MHal_M4VE_SetBitrate(arg);
        }
        break;
    case M4VE_IOC_SET_QP:
        {
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_SET_QP: 0x%x\n", (U32)arg);
            MHal_M4VE_SetQscale((U8)(arg>>16), arg&0x0ffff);
        }
        break;
#ifdef _M4VE_BIG2_
    case M4VE_IOC_SET_CODEC: //set codec before calling m4ve_init
        {   //0: mpeg4, 1: h.263
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_SET_CODEC: 0x%x\n", (U32)arg);
            MHal_M4VE_SetCodec(arg);
            if (arg==1) //H.263 and we don't have to take VOL
                VOL_take = 1;
        }
        break;
#endif
    case M4VE_IOC_GET_VOL:
        {
            M4VE_DEBUG("ioctl: MDRV: M4VE_IOC_GET_VOL: 0x%x\n", (U32)arg);
            MDrv_M4VE_GetVOL((U32 *)arg);
        }
        break;
    case M4VE_IOC_FINISH:
        {
#if !(defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_))
            sem_destroy(&_m4ve_wait_queue);
            pthread_mutex_destroy(&_spinlock);
#endif
            msAPI_M4VE_Finish();
        }
        break;
   default:
        M4VE_DEBUG("ioctl: MDRV: M4VE receive non-defined Ioctl\n");
        break;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_M4VE, _IOC_NR(cmd));
    return 0;
}

#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
module_init(MDrv_M4VE_ModuleInit);
module_exit(MDrv_M4VE_ModuleExit);

//ken LGE
MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("M4VE driver");
MODULE_LICENSE("MSTAR");
#endif //defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
#endif //CONFIG_MSTAR_TITANIA

