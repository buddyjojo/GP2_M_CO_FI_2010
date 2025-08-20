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
/// file    dev_mad.c
/// @brief  MAD Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/ioport.h>

#include "mdrv_mad_io.h"
#include "mdrv_mad_common.h"
#include "mdrv_mad_dvb.h"
#include "mdrv_mad_dvb2.h"
#include "mdrv_mad_process.h"
#include "mdrv_mad_sif.h"
#include "mhal_mad_reg.h"

#include "mdrv_probe.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define MAD_WARNING(fmt, args...)       printk(KERN_WARNING "[MADMOD][%06d]     " fmt, __LINE__, ## args)
#define MAD_PRINT(fmt, args...)         printk("[MADMOD][%06d]     " fmt, __LINE__, ## args)

#define MOD_MAD_DEVICE_COUNT    1
#define MOD_MAD_NAME            "ModMad"

#define MAD_DEBUG_DDI(msg) //msg //For debug
#define MAD_DEBUG_DDI2(msg) //msg //For debug

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32MadMajor;
    int                         s32MadMinor;
    struct cdev                 cDevice;
    struct file_operations      MadFop;
} MadModHandle;

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
static int                      _mod_mad_open (struct inode *inode, struct file *filp);
static int                      _mod_mad_release(struct inode *inode, struct file *filp);
static ssize_t                  _mod_mad_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t                  _mod_mad_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static unsigned int             _mod_mad_poll(struct file *filp, poll_table *wait);
static int                      _mod_mad_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
static MadModHandle MadDev=
{
    .s32MadMajor=               MDRV_MAJOR_MAD,
    .s32MadMinor=               MDRV_MINOR_MAD,
    .cDevice=
    {
        .kobj=                  {.name= MOD_MAD_NAME, },
        .owner  =               THIS_MODULE,
    },
    .MadFop=
    {
        .open=                  _mod_mad_open,
        .release=               _mod_mad_release,
        .read=                  _mod_mad_read,
        .write=                 _mod_mad_write,
        .poll=                  _mod_mad_poll,
        .ioctl=                 _mod_mad_ioctl,
    },
};

AUDIO_BUFFER_INFO  stAudioBufInfo;

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
wait_queue_head_t               _mad_wait_queue;
static U16                      _u16MadEvent =      0;
spinlock_t                      _mad_spinlock;

#define _Set_MAD_Event(flag)                     { \
                                                SET_MAD_EVENT((_u16MadEvent), (flag)); \
                                                wake_up(&_mad_wait_queue); \
                                            }

//--------------------------------------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------------------------------------
void MDrv_Event_to_AP(U8 event)
{
    _Set_MAD_Event(event);
}

static int _MDrv_MAD_SetBufferInfo(unsigned long arg)
{
    if (copy_from_user(&stAudioBufInfo, (AUDIO_BUFFER_INFO __user *)arg, sizeof(AUDIO_BUFFER_INFO)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_InitializeModule(unsigned long arg)
{
    U8 InitCodeType;

    MAD_DEBUG_DDI(printk("_MDrv_MAD_InitializeModule...\n"));
    if (__get_user(InitCodeType, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_AudioInit(InitCodeType);
    MDrv_MAD_ISRInit();
    return 0;
}

static int _MDrv_MAD_SetInputPath(unsigned long arg)
{
    AUDIO_INPUT_INFO AudioInput;

    if (copy_from_user(&AudioInput, (AUDIO_INPUT_INFO __user *)arg, sizeof(AUDIO_INPUT_INFO)))
        return -EFAULT;
    MAD_DEBUG_DDI(printk(" _MDrv_MAD_SetInputPath 0x%x = 0x%x\n",AudioInput.u8Input,AudioInput.u8Path));

    MDrv_MAD_SetInputPath(AudioInput.u8Input, AudioInput.u8Path);

    return 0;
}

static int _MDrv_MAD_SetInternalPath(unsigned long arg)
{
    AUDIO_INTERNAL_PATH_INFO AudioInternal;

    if (copy_from_user(&AudioInternal, (AUDIO_INTERNAL_PATH_INFO __user *)arg, sizeof(AUDIO_INTERNAL_PATH_INFO)))
        return -EFAULT;
    MAD_DEBUG_DDI(printk(" _MDrv_MAD_SetInternalPath Path = 0x%x, Output = 0x%x\n",AudioInternal.u8Path,AudioInternal.u8Output));

    MDrv_MAD_SetInternalPath(AudioInternal.u8Path, AudioInternal.u8Output);

    return 0;
}

static int _MDrv_MAD_Set_Power_Off(unsigned long arg)
{
    U8 OnOff_flag;

    if (__get_user(OnOff_flag, (U8 __user *)arg))
        return -EFAULT;

    MAD_DEBUG_DDI(printk("_MDrv_MAD_Set_Power_Off(%d)\n",OnOff_flag));

    MDrv_MAD_Set_Power_Off(OnOff_flag);
    return 0;
}

static int _MDrv_MAD_TriggerPIO8(void)
{
    MDrv_MAD_TriggerPIO8();
    return 0;
}

static int _MDrv_MAD2_TriggerPIO8(void)
{
    MDrv_MAD2_TriggerPIO8();
    return 0;
}

static int _MDrv_MAD_AudioSamplingRate(unsigned long arg)
{
    U32 sampRate;

    //MDrv_MAD_AudioSamplingRate(&sampRate);

    if (copy_to_user((void *)arg, &sampRate, sizeof(U32)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_ReadMailBox(unsigned long arg)
{
    ACCESS_MAILBOX mailbox;

    if (copy_from_user(&mailbox, (ACCESS_MAILBOX __user *)arg, sizeof(ACCESS_MAILBOX)))
        return -EFAULT;

    mailbox.value = MDrv_MAD_ReadMailBox(mailbox.bDspType, mailbox.u8ParamNum);

    MAD_DEBUG_DDI(printk(" _MDrv_MAD_ReadMailBox value = 0x%x,bDspType = 0x%x, u8ParamNum = 0x%x\n",mailbox.value, mailbox.bDspType, mailbox.u8ParamNum));

    if (copy_to_user((void *)arg, &mailbox, sizeof(ACCESS_MAILBOX)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_WriteMailBox(unsigned long arg)
{
    ACCESS_MAILBOX mailbox;

    if (copy_from_user(&mailbox, (ACCESS_MAILBOX __user *)arg, sizeof(ACCESS_MAILBOX)))
        return -EFAULT;

    MDrv_MAD_WriteMailBox(mailbox.bDspType,mailbox.u8ParamNum, mailbox.value);

    return 0;
}

static int _MDrv_MAD_ReadREG(unsigned long arg)
{
    ACCESS_AUDIO_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_AUDIO_REG __user *)arg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;

    AudioReg.value = MHal_MAD_ReadReg(AudioReg.addr);

    MAD_DEBUG_DDI(printk(" _MDrv_MAD_ReadREG read 0x%x = 0x%x\n",AudioReg.addr,AudioReg.value));

    if (copy_to_user((void *)arg, &AudioReg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_WriteREG(unsigned long arg)
{
    ACCESS_AUDIO_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_AUDIO_REG __user *)arg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;
    MAD_DEBUG_DDI(printk(" _MDrv_MAD_WriteREG Write 0x%x = 0x%x\n",AudioReg.addr,AudioReg.value));

    MDrv_MAD_WriteREG(AudioReg.addr,AudioReg.value);

    return 0;
}

static int _MDrv_MAD_WriteREGMask(unsigned long arg)
{
    ACCESS_MAD_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_MAD_REG __user *)arg, sizeof(ACCESS_MAD_REG)))
        return -EFAULT;

    MDrv_MAD_WriteREGMask(AudioReg.addr,AudioReg.mask, AudioReg.value);

    return 0;
}

static int _MDrv_MAD_DecReadREG(unsigned long arg)
{
    ACCESS_AUDIO_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_AUDIO_REG __user *)arg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;

    AudioReg.value = MDrv_MAD_DecReadREG(AudioReg.addr);

    MAD_DEBUG_DDI(printk(" _MDrv_MAD_DecReadREG read 0x%x = 0x%x\n",AudioReg.addr,AudioReg.value));

    if (copy_to_user((void *)arg, &AudioReg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_DecReadREGByte(unsigned long arg)
{
    ACCESS_AUDIO_REGBYTE AudioRegByte;

    if (copy_from_user(&AudioRegByte, (ACCESS_AUDIO_REGBYTE __user *)arg, sizeof(ACCESS_AUDIO_REGBYTE)))
        return -EFAULT;

    AudioRegByte.value = MDrv_MAD_DecReadREGByte(AudioRegByte.addr);

    MAD_DEBUG_DDI(printk(" _MDrv_MAD_DecReadREGByte read 0x%x = 0x%x\n",AudioRegByte.addr,AudioRegByte.value));

    if (copy_to_user((void *)arg, &AudioRegByte, sizeof(ACCESS_AUDIO_REGBYTE)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_SeReadREG(unsigned long arg)
{
    ACCESS_AUDIO_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_AUDIO_REG __user *)arg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;

    AudioReg.value = MDrv_MAD_SeReadREG(AudioReg.addr);

    MAD_DEBUG_DDI(printk(" _MDrv_MAD_SeReadREG read 0x%x = 0x%x\n",AudioReg.addr,AudioReg.value));

    if (copy_to_user((void *)arg, &AudioReg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_SeReadREGByte(unsigned long arg)
{
    ACCESS_AUDIO_REGBYTE AudioRegByte;

    if (copy_from_user(&AudioRegByte, (ACCESS_AUDIO_REGBYTE __user *)arg, sizeof(ACCESS_AUDIO_REGBYTE)))
        return -EFAULT;

    AudioRegByte.value = MDrv_MAD_SeReadREGByte(AudioRegByte.addr);

    MAD_DEBUG_DDI(printk(" _MDrv_MAD_SeReadREGByte read 0x%x = 0x%x\n",AudioRegByte.addr,AudioRegByte.value));

    if (copy_to_user((void *)arg, &AudioRegByte, sizeof(ACCESS_AUDIO_REGBYTE)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_DecWriteREG(unsigned long arg)
{
    ACCESS_AUDIO_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_AUDIO_REG __user *)arg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;
    MAD_DEBUG_DDI(printk(" _MDrv_MAD_DecWriteREG Write 0x%x = 0x%x\n",AudioReg.addr,AudioReg.value));

    MDrv_MAD_DecWriteREG(AudioReg.addr,AudioReg.value);

    return 0;
}

static int _MDrv_MAD_DecWriteREGByte(unsigned long arg)
{
    ACCESS_AUDIO_REGBYTE AudioRegByte;

    if (copy_from_user(&AudioRegByte, (ACCESS_AUDIO_REGBYTE __user *)arg, sizeof(ACCESS_AUDIO_REGBYTE)))
        return -EFAULT;
    MAD_DEBUG_DDI(printk(" _MDrv_MAD_DecWriteREGByte Write 0x%x = 0x%x\n",AudioRegByte.addr,AudioRegByte.value));

    MDrv_MAD_DecWriteREGByte(AudioRegByte.addr,AudioRegByte.value);

    return 0;
}

static int _MDrv_MAD_SeWriteREG(unsigned long arg)
{
    ACCESS_AUDIO_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_AUDIO_REG __user *)arg, sizeof(ACCESS_AUDIO_REG)))
        return -EFAULT;
    MAD_DEBUG_DDI(printk(" _MDrv_MAD_SeWriteREG Write 0x%x = 0x%x\n",AudioReg.addr,AudioReg.value));

    MDrv_MAD_SeWriteREG(AudioReg.addr,AudioReg.value);

    return 0;
}

static int _MDrv_MAD_SeWriteREGByte(unsigned long arg)
{
    ACCESS_AUDIO_REGBYTE AudioRegByte;

    if (copy_from_user(&AudioRegByte, (ACCESS_AUDIO_REGBYTE __user *)arg, sizeof(ACCESS_AUDIO_REGBYTE)))
        return -EFAULT;
    MAD_DEBUG_DDI(printk(" _MDrv_MAD_SeWriteREGByte Write 0x%x = 0x%x\n",AudioRegByte.addr,AudioRegByte.value));

    MDrv_MAD_SeWriteREGByte(AudioRegByte.addr,AudioRegByte.value);

    return 0;
}
static int _MDrv_MAD_DecWriteREGMask(unsigned long arg)
{
    ACCESS_MAD_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_MAD_REG __user *)arg, sizeof(ACCESS_MAD_REG)))
        return -EFAULT;

    MDrv_MAD_DecWriteREGMask(AudioReg.addr,AudioReg.mask, AudioReg.value);

    return 0;
}

static int _MDrv_MAD_SeWriteREGMask(unsigned long arg)
{
    ACCESS_MAD_REG AudioReg;

    if (copy_from_user(&AudioReg, (ACCESS_MAD_REG __user *)arg, sizeof(ACCESS_MAD_REG)))
        return -EFAULT;

    MDrv_MAD_SeWriteREGMask(AudioReg.addr,AudioReg.mask, AudioReg.value);

    return 0;
}

static int _MDrv_MAD_DecWriteREGMaskByte(unsigned long arg)
{
    ACCESS_MAD_REGBYTE AudioRegByte;

    if (copy_from_user(&AudioRegByte, (ACCESS_MAD_REGBYTE __user *)arg, sizeof(ACCESS_MAD_REGBYTE)))
        return -EFAULT;

    MDrv_MAD_DecWriteREGMaskByte(AudioRegByte.addr,AudioRegByte.mask, AudioRegByte.value);

    return 0;
}

static int _MDrv_MAD_DecWriteIntMaskByte(unsigned long arg)
{
    ACCESS_MAD_REGBYTE AudioRegByte;

    if (copy_from_user(&AudioRegByte, (ACCESS_MAD_REGBYTE __user *)arg, sizeof(ACCESS_MAD_REGBYTE)))
        return -EFAULT;

    MDrv_MAD_DecWriteIntMaskByte(AudioRegByte.addr,AudioRegByte.mask);

    return 0;
}

static int _MDrv_MAD_SeWriteREGMaskByte(unsigned long arg)
{
    ACCESS_MAD_REGBYTE AudioRegByte;

    if (copy_from_user(&AudioRegByte, (ACCESS_MAD_REGBYTE __user *)arg, sizeof(ACCESS_MAD_REGBYTE)))
        return -EFAULT;

    MDrv_MAD_SeWriteREGMaskByte(AudioRegByte.addr,AudioRegByte.mask, AudioRegByte.value);

    return 0;
}


static int _MDrv_MAD_DvbGetSoundMode(unsigned long arg)
{
    U16 SoundMode;

    SoundMode = MDrv_MAD_DvbGetSoundMode();

    MAD_DEBUG_DDI(printk("_MDrv_MAD_DvbGetSoundMode(%d)\n",SoundMode));
    if (copy_to_user((void *)arg, &SoundMode, sizeof(U16)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_SetSystem(unsigned long arg)
{
    U8 SysSet;

    if (__get_user(SysSet, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_SetSystem(SysSet);

    return 0;
}

static int _MDrv_MAD_SetCommand(unsigned long arg)
{
    En_DVB_decCmdType decCmd;

    if (__get_user(decCmd, (En_DVB_decCmdType __user *)arg))
        return -EFAULT;

    MDrv_MAD_SetCommand(decCmd);

    return 0;
}

static int _MDrv_MAD_Dvb_setDecCmd(unsigned long arg)
{
    U8 decCmd;

    if (__get_user(decCmd, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_Dvb_setDecCmd(decCmd);

    return 0;
}

static int _MDrv_MAD2_SetDecCmd(unsigned long arg)
{
    U8 decCmd2;

    if (__get_user(decCmd2, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD2_SetDecCmd(decCmd2);

    return 0;
}

static int _MDrv_MAD_SetFreeRun(unsigned long arg)
{
    U8 FreeRun;

    if (__get_user(FreeRun, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_SetFreeRun(FreeRun);

    return 0;
}
static int _MDrv_MAD2_SetFreeRun(unsigned long arg)
{
    U16 FreeRun2;

    if (__get_user(FreeRun2, (U16 __user *)arg))
        return -EFAULT;

    MDrv_MAD2_SetFreeRun(FreeRun2);

    return 0;
}

static int _MDrv_MAD_ProcessSetMute(unsigned long arg)
{
    PROCESS_MUTE setMute;

    if (copy_from_user(&setMute, (PROCESS_MUTE __user *)arg, sizeof(PROCESS_MUTE)))
        return -EFAULT;

    MDrv_MAD_ProcessSetMute(setMute.path, setMute.enable);

    return 0;
}

static int _MDrv_MAD_ProcessSetVolume(unsigned long arg)
{
    PROCESS_VOLUME setVolume;

    if (copy_from_user(&setVolume, (PROCESS_VOLUME __user *)arg, sizeof(PROCESS_VOLUME)))
    {
        MAD_DEBUG_DDI(printk("[SetVolume]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_ProcessSetVolume(setVolume.path, setVolume.volume);

    return 0;
}

static int _MDrv_MAD_ProcessAbsoluteVolume(unsigned long arg)
{
    PROCESS_VOLUME setVolume;

#if 1//by LGE YWJung 2008.06.07
	U8 vol1,vol2;
#endif

    if (copy_from_user(&setVolume, (PROCESS_VOLUME __user *)arg, sizeof(PROCESS_VOLUME)))
    {
        MAD_DEBUG_DDI(printk("[SetVolume]:Copy from user space data error\n"));
        return -EFAULT;
    }

#if 1//by LGE YWJung 2008.06.07
	vol1 = (U8) ((setVolume.volume&0xff00)>>8);	// don't remove ()
	vol2 = (U8) (setVolume.volume&0x00ff);
    MDrv_MAD_ProcessAbsoluteVolume(setVolume.path, vol1, vol2);
#else
    MDrv_MAD_ProcessAbsoluteVolume(setVolume.path, setVolume.volume, 0x00);
#endif

    return 0;
}

static int _MDrv_MAD_ProcessSPDIFVolume(unsigned long arg)
{
    U16 vol;

    if (__get_user(vol, (U16 __user *)arg))
        return -EFAULT;

    MDrv_MAD_ProcessSPDIFVolume(vol);

    return 0;
}
static int _MDrv_MAD_ProcessSetBalance(unsigned long arg)
{
    PROCESS_BALANCE setBalance;

    if (copy_from_user(&setBalance, (PROCESS_BALANCE __user *)arg, sizeof(PROCESS_BALANCE)))
        return -EFAULT;

    MDrv_MAD_ProcessSetBalance(setBalance.path, setBalance.balance);

    return 0;
}

static int _MDrv_MAD_ProcessSetBass(unsigned long arg)
{
    U8 u8Bass;

    if (__get_user(u8Bass, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_ProcessSetBass(u8Bass);

    return 0;
}

static int _MDrv_MAD_ProcessSetTreble(unsigned long arg)
{
    U8 u8Treble;

    if (__get_user(u8Treble, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_ProcessSetTreble(u8Treble);

    return 0;
}

static int _MDrv_MAD_ProcessSetBBE(unsigned long arg)
{
    MAD_DEBUG_DDI(printk("[ProcessSetBBE]:Value = 0x%x \n", (B16)arg));
    MDrv_MAD_ProcessSetBBE((B16)arg);

    return 0;
}

static int _MDrv_MAD_ProcessSetSRS(unsigned long arg)
{
    MAD_DEBUG_DDI(printk("[ProcessSetSRS]:Value = 0x%x \n", (B16)arg));
    MDrv_MAD_ProcessSetSRS((B16)arg);

    return 0;
}

static int _MDrv_MAD_ProcessSetEqualizerLevel(unsigned long arg)
{
    PROCESS_GEQ setGeq;

    if (copy_from_user(&setGeq, (PROCESS_GEQ __user *)arg, sizeof(PROCESS_GEQ)))
        return -EFAULT;

   MDrv_MAD_ProcessSetEq(setGeq.band, setGeq.level);  // Remove by MStar

    return 0;
}

static int _MDrv_MAD_GetDecodingType(unsigned long arg)
{
    U8 DecoderType;

    DecoderType = MDrv_MAD_GetDSPCodeType();

    if (copy_to_user((void *)arg, &DecoderType, sizeof(U8)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_StartDvb2_Decoding(unsigned long arg)
{
     U8 AdecSysSet=0;

    if (__get_user(AdecSysSet, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("\n[StartAD_Decoding]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD2_SetSystem(AdecSysSet);

    MAD_DEBUG_DDI(printk("\n[StartAD_Decoding]:CodeType = 0x%x\n", AdecSysSet));

    return 0;
}


static int _MDrv_MAD_StartDecoding(unsigned long arg)
{
     U8 AdecSysSet=0;

    if (__get_user(AdecSysSet, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[StartDecoding]:Get user data error\n"));
        return -EFAULT;
    }

    if(AdecSysSet == ADEC_SRC_TYPE_SIF )
        MDrv_MAD_SIF_ReLoadCode(AU_SIF_PALSUM);
    else if (AdecSysSet == ADEC_SRC_TYPE_SIF_BTSC)
        MDrv_MAD_SIF_ReLoadCode(AU_SIF_BTSC);
    else if ( AdecSysSet == ADEC_SRC_TYPE_SIF_A2 )
        MDrv_MAD_SIF_ReLoadCode(AU_SIF_PALSUM); // for Korea A2
    else
         MDrv_MAD_SetSystem(AdecSysSet);

    MAD_DEBUG_DDI(printk("\n[StartDecoding]:CodeType = 0x%x\n", AdecSysSet));

    return 0;
}
static int _MDrv_MAD_GetEncBufInfo(unsigned long arg)
{
    ENC_BUF_INFO_T ENC_BUF_INFO;
    MDrv_MAD_GetEncBufInfo(&(ENC_BUF_INFO.u32_BufAddr), &(ENC_BUF_INFO.u32_BufSize), &(ENC_BUF_INFO.u32_FrameSize), &(ENC_BUF_INFO.u16_FrameTime));

    if (copy_to_user((void *)arg, &ENC_BUF_INFO, sizeof(ENC_BUF_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[GetFileInfo]:Get user data error\n"));
        return -EFAULT;
    }
    return 0;
}
static int _MDrv_MAD_GetEncFrameInfo(unsigned long arg)
{
    ENC_FRAME_INFO_T ENC_FRAME_INFO;

    MDrv_MAD_GetEncFrameInfo(&(ENC_FRAME_INFO.Line_Addr), &(ENC_FRAME_INFO.Line_Size), &(ENC_FRAME_INFO.Enc_PTS));

    if (copy_to_user((void *)arg, &ENC_FRAME_INFO, sizeof(ENC_FRAME_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[GetFileInfo]:Get user data error\n"));
        return -EFAULT;
    }
    return 0;
}

static int _MDrv_MAD_GetEncodeDoneFlag(unsigned long arg)
{
    B16 Flag_Status;

    Flag_Status = MDrv_MAD_GetEncodeDoneFlag();

    if (copy_to_user((void *)arg, &Flag_Status, sizeof(B16)))
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_StopEncoding(void)
{
    MAD_DEBUG_DDI(printk("[StopEncoding]\n"));
    MDrv_MAD_SetEncCmd(0);
    MDrv_MAD_SetEncodeDoneFlag(0);
    return 0;
}

static int _MDrv_MAD_ClrEncodeDoneFlag(void)
{
    MAD_DEBUG_DDI(printk("[ClrEncodeDoneFlag]\n"));
    MDrv_MAD_SetEncodeDoneFlag(0);
    return 0;
}

static int _MDrv_MAD_StartEncoding(void)
{
    MAD_DEBUG_DDI(printk("[StartEncoding]\n"));
    MDrv_MAD_SetEncCmd(1);
    return 0;
}

static int _MDrv_MAD_StopDecoding(void)
{
    MAD_DEBUG_DDI(printk("[StopDecoding]\n"));
    MDrv_MAD_Dvb_setDecCmd(0);
    return 0;
}

static int _MDrv_MAD_GetCurAnalogMode(unsigned long arg)
{
    U16 AlgMode;
    if (copy_from_user(&AlgMode, (U8 __user *)arg, sizeof(U8)))
        return -EFAULT;

    MDrv_MAD_GetCurAnalogMode(&AlgMode);
    MAD_DEBUG_DDI(printk("_MDrv_MAD_GetCurAnalogMode(%d)\n",AlgMode));
    if (copy_to_user((void *)arg, &AlgMode, sizeof(U16)))
    {
        MAD_DEBUG_DDI(printk("[GetCurAnalogMode]:Get user data error\n"));
        return -EFAULT;
    }
    return 0;
}

static int _MDrv_MAD_SetUserAnalogMode(unsigned long arg)
{
    U16 AlgMode;

    if (__get_user(AlgMode, (U16 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SetUserAnalogMode]:Get user data error\n"));
        return -EFAULT;
    }

    MAD_DEBUG_DDI(printk("_MDrv_MAD_SetUserAnalogMode(%d)\n",AlgMode));
    MDrv_MAD_SetUserAnalogMode(AlgMode);

    return 0;
}

static int _MDrv_MAD_SetAutoVolumeControl(unsigned long arg)
{
    U8 enable;

    if (__get_user(enable, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SetAutoVolumeControl]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SetAutoVolumeControl(enable);

    return 0;
}

static int _MDrv_MAD_GetBtscA2StereoLevel(unsigned long arg)
{
    U16 stereoLevel;

    if (MDrv_MAD_GetBtscA2StereoLevel(&stereoLevel) == FALSE)
        return -EFAULT;
    MAD_DEBUG_DDI(printk("_MDrv_MAD_GetBtscA2StereoLevel(%d)\n", stereoLevel));

    if (copy_to_user((void *)arg, &stereoLevel, sizeof(U16)))
    {
        MAD_DEBUG_DDI(printk("[SIF_GetBtscA2StereoLevel]:Get user data error\n"));
        return -EFAULT;
    }
    return 0;
}

#if 0
static int _MDrv_MAD_SifSetSystem(unsigned long arg)
{
    U8 SystemType;

    if (copy_from_user(&SystemType, (U8 __user *)arg, sizeof(U8)))
        return -EFAULT;

    MDrv_MAD_AuSifSetSystem(SystemType);

    return 0;
}
#endif

static int _MDrv_MAD_SPDIF_SetMute(unsigned long arg)
{
    U8 MuteOn;

    if (__get_user(MuteOn, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SetSPDIFMute]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SPDIF_SetMute(MuteOn);

    return 0;
}

static int _MDrv_MAD_SetSPDIFOutputType(unsigned long arg)
{
    U8 OutputType;

    if (__get_user(OutputType, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SetSPDIFOutputType]:Get user data error\n"));
        return -EFAULT;
    }
   // MDrv_MAD_AuSetSPDIF_Path(OutputType);
    return 0;
}
static int _MDrv_MAD_SetAudioPLLSFreq(unsigned long arg)
{
    U32 samplingfreq;

    if (copy_from_user(&samplingfreq, (U32 __user *)arg, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[SetAudioPLLSFreq]:Copy from user space data error\n"));
        return -EFAULT;
    }
    MDrv_MAD_SetAudioPLLSFreq(samplingfreq);
    return 0;
}

static int _MDrv_MAD_SetPCMDelayTime(unsigned long arg)
{
    U32 delayTime;

    if (copy_from_user(&delayTime, (U32 __user *)arg, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[SetPCMDelayTime]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_ProcessSetCH1AudioDelay(delayTime);

    return 0;
}

static int _MDrv_MAD_SetSPDIFDelayTime(unsigned long arg)
{
    U32 delayTime;

    if (copy_from_user(&delayTime, (U32 __user *)arg, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[SetPCMDelayTime]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SetSPDIFAudioDelay(delayTime);

    return 0;
}

static int _MDrv_MAD_GetHDMIAudioMode(unsigned long arg)
{
    U8 AudioPktRecv;

    AudioPktRecv = MDrv_MAD_GetHDMIAudioReceive();

    MAD_DEBUG_DDI(printk("_MDrv_MAD_GetHDMIAudioMode(%d)\n",AudioPktRecv));
    if (copy_to_user((void *)arg, &AudioPktRecv, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[GetHDMIAudioMode]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_GetHDMISynthFreq(unsigned long arg)
{
    U16 regVal;

    regVal = (MHal_MAD_ReadReg(0x2C2C) & 0x7FFF);

    MAD_DEBUG_DDI(printk("_MDrv_MAD_GetHDMISynthFreq(%d)\n",regVal));
    if (copy_to_user((void *)arg, &regVal, sizeof(U16)))
    {
        MAD_DEBUG_DDI(printk("[GetHDMISynthFreq]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_GetAdecStatus(unsigned long arg)
{
    U8 SystemType;

    if (copy_from_user(&SystemType, (U8 __user *)arg, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[GetAdecStatus]:Copy from user space data error\n"));
        return -EFAULT;
    }

    //To Do MDrv_MAD_GetAdecStatus(SystemType);

    return 0;
}
static int _MDrv_MAD_SetHPOutputType(unsigned long arg)
{
    U8 SrcType;

    if (__get_user(SrcType, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SetHPOutputType]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SetHPOutputType(SrcType);

    return 0;
}

static int _MDrv_MAD_SetBtscA2ThresholdLevel(unsigned long arg)
{
    U16 thresholLevel;

    if (__get_user(thresholLevel, (U16 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SIF_SetBtscA2ThresholdLevel]:Copy from user space data error\n"));
        return -EFAULT;
    }

    if (MDrv_MAD_SetBtscA2ThresholdLevel(thresholLevel) == FALSE)
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_mpeg_GetSoundMode(unsigned long arg)
{
    U8 mpeg_soundmode;

    mpeg_soundmode = MDrv_MAD_mpeg_GetSoundMode() ;

    if (copy_to_user((void *)arg, &mpeg_soundmode, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[mpeg_GetSoundMode]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_mpeg_SetSoundMode(unsigned long arg)
{
    U8 mpeg_soundmode;

    if (__get_user(mpeg_soundmode, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[mpeg_SetSoundMode]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_mpeg_SetSoundMode(mpeg_soundmode);

    return 0;
}

static int _MDrv_MAD_SetSPKOutMode(unsigned long arg)
{
    U8 SPKOutMode;

    if (__get_user(SPKOutMode, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SetSPKOutMode]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MAD_DEBUG_DDI(printk("_MDrv_MAD_SetSPKOutMode(%d)\n",SPKOutMode));
    MDrv_MAD_SetSPKOutMode(SPKOutMode);

    return 0;
}

static int _MDrv_MAD_CheckAudioES(unsigned long arg)
{
    U8 es_exist;

    if(IS_MADGOOD())
        es_exist = 1;
    else
	 es_exist = 0;

    MAD_DEBUG_DDI(printk("_MDrv_MAD_CheckAudioES return %s\n",(es_exist) ? "Audio ES exist" : "None Audio ES"));
    if (copy_to_user((void *)arg, &es_exist, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[CheckAudioES]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

#if 0 // Richard.Ni mark, not use this function, use _MDrv_MAD_CV3_PVC_Monitor
static int _MDrv_MAD_PVC_Monitor(unsigned long arg)
{
    pvc_monitor_T pvc;
    int         s32Ret;

    if ( copy_from_user(&pvc, (pvc_monitor_T *)arg, sizeof(pvc_monitor_T)) )
    {
        MAD_DEBUG_DDI(printk("[PVC_MONITOR]:Copy from user data error\n"));
        return -EFAULT;
    }

    s32Ret = MDrv_MAD_PVC_Monitor(pvc.GapCounter, &(pvc.L_Level), &(pvc.R_Level));

    if ( copy_to_user((pvc_monitor_T *)arg, &pvc, sizeof(pvc_monitor_T)) )
    {
        MAD_DEBUG_DDI(printk("[PVC_MONITOR]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}
#endif

static int _MDrv_MAD_SIF_EnableDACOut(unsigned long arg)
{
    DAC_OUT enableDAC;

    if (copy_from_user(&enableDAC, (DAC_OUT __user *)arg, sizeof(DAC_OUT)))
    {
        MAD_DEBUG_DDI(printk("[EnableDACOut]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SIF_EnableDACOut(enableDAC.path,enableDAC.enable);

    return 0;
}

static int _MDrv_MAD_SIF_GetBandDetect(unsigned long arg)
{
    BAND_DETECT_INFO_T band_detect_info;

    if (copy_from_user(&band_detect_info, (BAND_DETECT_INFO_T __user *)arg, sizeof(BAND_DETECT_INFO_T)))
        return -EFAULT;

    if (MDrv_MAD_SIF_GetBandDetect(band_detect_info.soundSystem, &(band_detect_info.bandStrength)) == FALSE)
        return -EFAULT;
    else {
        if (copy_to_user((BAND_DETECT_INFO_T __user *)arg, &band_detect_info, sizeof(BAND_DETECT_INFO_T)))
            return -EFAULT;
    }
    return 0;
}

static int _MDrv_MAD_SIF_SetBandSetup(unsigned long arg)
{
    ADEC_SIF_SOUNDSYSTEM_T SifBand;

    if (__get_user(SifBand, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SIF_SetBandSetup]:Copy from user space data error\n"));
        return -EFAULT;
    }

    if (MDrv_MAD_SIF_SetBandSetup(SifBand) == FALSE)
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_SIF_SetModeSetup(unsigned long arg)
{
    U8 SystemType;

    if (__get_user(SystemType, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SIF_SetModeSetup]:Copy from user space data error\n"));
        return -EFAULT;
    }

    if (MDrv_MAD_SIF_SetModeSetup(SystemType) == FALSE)
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_SIF_SetHDEVMode(unsigned long arg)
{
    U8 bOnOff;

    if (__get_user(bOnOff, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[SIF_SetHDEVMode]:Copy from user space data error\n"));
        return -EFAULT;
    }

    if (MDrv_MAD_SIF_SetHDEVMode(bOnOff) == FALSE)
        return -EFAULT;

    return 0;
}

static int _MDrv_MAD_SIF_CheckNicamDigital(unsigned long arg)
{
    U8 isNicamDetect;

    if (copy_from_user(&isNicamDetect, (U8 __user *)arg, sizeof(U8)))
        return -EFAULT;

    MDrv_MAD_CheckNicamDigital(&isNicamDetect);
    //MAD_DEBUG_DDI(printk("_MDrv_MAD_CheckNicamDigital(%d)\n", isNicamDetect));
    if (copy_to_user((void *)arg, &isNicamDetect, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[SIF_CheckNicamDigital]:Get user data error\n"));
        return -EFAULT;
    }
    return 0;
}

static int _MDrv_MAD_SIF_CheckAvailableSystem(unsigned long arg)
{
    AVAILABLE_SYSTEM_INFO_T available_system_info;

    if (copy_from_user(&available_system_info, (AVAILABLE_SYSTEM_INFO_T __user *)arg, sizeof(AVAILABLE_SYSTEM_INFO_T)))
        return -EFAULT;

    if (MDrv_MAD_SIF_CheckAvailableSystem(available_system_info.standard, &(available_system_info.exist)) == FALSE)
        return -EFAULT;
    else {
        if (copy_to_user((AVAILABLE_SYSTEM_INFO_T __user *)arg, &available_system_info, sizeof(AVAILABLE_SYSTEM_INFO_T)))
            return -EFAULT;
    }
    return 0;
}

static int _MDrv_MAD_SIF_CheckA2DK(unsigned long arg)
{
    AVAILABLE_SYSTEM_INFO_T available_system_info;

    if (copy_from_user(&available_system_info, (AVAILABLE_SYSTEM_INFO_T __user *)arg, sizeof(AVAILABLE_SYSTEM_INFO_T)))
        return -EFAULT;

    if (MDrv_MAD_SIF_CheckA2DK(available_system_info.standard, &(available_system_info.exist)) == FALSE)
        return -EFAULT;
    else {
        if (copy_to_user((AVAILABLE_SYSTEM_INFO_T __user *)arg, &available_system_info, sizeof(AVAILABLE_SYSTEM_INFO_T)))
            return -EFAULT;
    }
    return 0;
}

// Mstar fast routine to get sif standard..
static int _MDrv_MAD_SIF_GetSoundStandard(unsigned long arg)
{
    U8 SifStandard;
    if (copy_from_user(&SifStandard, (U8 __user *)arg, sizeof(U8)))
        return -EFAULT;

    if (MDrv_MAD_SIF_GetSoundStandard(&SifStandard) == FALSE)
        return -EFAULT;
    MAD_DEBUG_DDI(printk("_MDrv_MAD_SIF_GetSoundStandard(%d)\n",SifStandard));
    if (copy_to_user((void *)arg, &SifStandard, sizeof(U16)))
    {
        MAD_DEBUG_DDI(printk("[SIF_GetSoundStandard]:Get user data error\n"));
        return -EFAULT;
    }
    return 0;
}

// for threshold setting
static int _MDrv_MAD_SIF_AccessThreshold(unsigned long arg)
{
    ACCESS_THRESHOLD_INFO_T access_threshold_info;

    if (copy_from_user(&access_threshold_info, (ACCESS_THRESHOLD_INFO_T __user *)arg, sizeof(ACCESS_THRESHOLD_INFO_T)))
        return -EFAULT;
#if 0//CHECK
    if (MDrv_MAD_SIF_AccessThreshold(access_threshold_info.rw_standard_type, access_threshold_info.Threshold_type, &(access_threshold_info.value)) == FALSE)
        return -EFAULT;
    else {
        if (copy_to_user((ACCESS_THRESHOLD_INFO_T __user *)arg, &access_threshold_info, sizeof(ACCESS_THRESHOLD_INFO_T)))
            return -EFAULT;
    }
#endif
    return 0;
}

static int _MDrv_MAD_SIF_SetPrescale(unsigned long arg)
{
    ADEC_SIF_SET_PRESCALE_T set_prescale_info;

    if (copy_from_user(&set_prescale_info, (ADEC_SIF_SET_PRESCALE_T __user *)arg, sizeof(ADEC_SIF_SET_PRESCALE_T)))
        return -EFAULT;

    if (MDrv_MAD_SIF_SetPrescale(set_prescale_info.type, set_prescale_info.db_value) == FALSE)
        return -EFAULT;
    return 0;
}

static int _MDrv_MAD_LoadAudioClip(unsigned long arg)
{
    AUDIO_CLIP_INFO audioClipInfo;

    if (copy_from_user(&audioClipInfo, (U8 __user *)arg, sizeof(AUDIO_CLIP_INFO)))
    {
        MAD_DEBUG_DDI(printk("[LoadAudioClip]:Copy from user space data error\n"));
        return -EFAULT;
    }

    if((U32)audioClipInfo.pPaAddr>=0x10000000)
        MDrv_MAD_LoadAudioClip(audioClipInfo.size,audioClipInfo.pPaAddr+0xB0000000);
    else
        MDrv_MAD_LoadAudioClip(audioClipInfo.size,audioClipInfo.pPaAddr+0xA0000000);

    return 0;
}

static int _MDrv_MAD_PlayAudioClip(unsigned long arg)
{
    U8 repeat;

    if (__get_user(repeat, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[PlayAudioClip]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_PlayAudioClip(repeat);

    return 0;
}

static int _MDrv_MAD_Get_Event(unsigned long arg)
{
    U16 event_mad=0;

    event_mad = _u16MadEvent ;

    if(_u16MadEvent & MAD_EVENT_POLLED)
        _u16MadEvent = 0 ;

    if (copy_to_user((void *)arg, &event_mad, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[MAD_Get_Event]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

 static int _MDrv_MAD_SPDIF_SetMode(unsigned long arg)
{
    SPDIF_OUT spdifouttype;

    if (copy_from_user(&spdifouttype, (SPDIF_OUT __user *)arg, sizeof(SPDIF_OUT)))
    {
        MAD_DEBUG_DDI(printk("[SPDIF_SetMode]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SPDIF_SetMode(spdifouttype.spdifmode,spdifouttype.input_source);

    return 0;
}

static int _MDrv_MAD_SPDIF_SetSCMS(unsigned long arg)
{
    SPDIF_SCMS spdif_scms;

    if (copy_from_user(&spdif_scms, (SPDIF_SCMS __user *)arg, sizeof(SPDIF_SCMS)))
    {
        MAD_DEBUG_DDI(printk("[SPDIF_SetSCMS]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SPDIF_SetSCMS(spdif_scms.C_bit_en, spdif_scms.L_bit_en);

    return 0;
}

static int _MDrv_MAD_SPDIF_GetSCMS(unsigned long arg)
{
    U8 SCMS_status;

    SCMS_status = MDrv_MAD_SPDIF_GetSCMS();

    if (copy_to_user((void *)arg, &SCMS_status, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[Get_HDMI_Monitor]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

 static int _MDrv_MAD_HDMI_SetNonpcm(unsigned long arg)
{
    U8 nonPCM_en;


    if (__get_user(nonPCM_en, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[HDMI_PCM_NONPCM_Set]:Copy from user space data error\n"));
        return -EFAULT;
    }
    MDrv_MAD_HDMI_SetNonpcm(nonPCM_en);

    return 0;
}

static int _MDrv_MAD_HDMI_AC3_PathCFG(unsigned long arg)
{
    U8 path_ctrl;


    if (__get_user(path_ctrl, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[Dvb2_AD_HDMIAC3_PathCFG]:Copy from user space data error\n"));
        return -EFAULT;
    }
    MDrv_MAD_HDMI_AC3_PathCFG(path_ctrl);

    return 0;
}
static int _MDrv_MAD_GetHDMI_Monitor(unsigned long arg)
{
    U8 HDMIType;

if((MDrv_MAD_HDMI_Monitor() & 0xC0) == 0x80)
    HDMIType = MAD_HDMI_NO_AUDIO;
#if 1	// AC3 detection fail on speicific device by KH
else  if((MDrv_MAD_HDMI_Monitor() & 0xC0) == 0x40 || MDrv_MAD_HDMI_Monitor2() == 0x30)//20091126 mstar modify
#endif
    HDMIType = MAD_HDMI_HDMI_AC3;
else  if((MDrv_MAD_HDMI_Monitor() & 0xC0) == 0x0)
    HDMIType = MAD_HDMI_HDMI_PCM;
else
    HDMIType = MAD_HDMI_DEFAULT;

   if (copy_to_user((void *)arg, &HDMIType, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[Get_HDMI_Monitor]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_GetFileInfo(unsigned long arg)
{
    MEDIA_FILE_INFO_T file_info;

    MDrv_MAD_GetFileInfo(&file_info);
    if (copy_to_user((void *)arg, &file_info, sizeof(MEDIA_FILE_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[GetFileInfo]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_GetFileInfo_SE(unsigned long arg)
{
    MEDIA_FILE_INFO_T file_info;

    MDrv_MAD_GetFileInfo_SE(&file_info);
    if (copy_to_user((void *)arg, &file_info, sizeof(MEDIA_FILE_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[GetFileInfo]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_GetAudioStreamInfo(unsigned long arg)
{
    MEDIA_AUDIOSTREAM_INFO_T audiostream_info;

    if (copy_from_user(&audiostream_info, (MEDIA_AUDIOSTREAM_INFO_T __user *)arg, sizeof(MEDIA_AUDIOSTREAM_INFO_T)))
        return -EFAULT;
    MDrv_MAD_GetAudioStreamInfo(&audiostream_info);
    if (copy_to_user((void *)arg, &audiostream_info, sizeof(MEDIA_AUDIOSTREAM_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[GetAudioStreamInfo]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_FileSetInput(unsigned long arg)
{
    MEDIA_FILE_INPUT_T file_input;

    if (copy_from_user(&file_input, (MEDIA_FILE_INPUT_T __user *)arg, sizeof(MEDIA_FILE_INPUT_T)))
    {
        MAD_DEBUG_DDI(printk("[FileSetInpu]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_FileSetInput(file_input.tTag,file_input.wAddress);

    return 0;
}

static int _MDrv_MAD_FileSetInput_SE(unsigned long arg)
{
    MEDIA_FILE_INPUT_T file_input;

    if (copy_from_user(&file_input, (MEDIA_FILE_INPUT_T __user *)arg, sizeof(MEDIA_FILE_INPUT_T)))
    {
        MAD_DEBUG_DDI(printk("[FileSetInpu]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_FileSetInput_SE(file_input.tTag,file_input.wAddress);

    return 0;
}

static int _MDrv_MAD_SetAudioStreamInput(unsigned long arg)
{
    MEDIA_STREAM_INPUT_T stream_input;

    if (copy_from_user(&stream_input, (MEDIA_STREAM_INPUT_T __user *)arg, sizeof(MEDIA_STREAM_INPUT_T)))
    {
        MAD_DEBUG_DDI(printk("[FileSetInpu]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SetAudioStreamInput(stream_input.ch,stream_input.tTag);

    return 0;
}

static int _MDrv_MAD_FileEndNotification(unsigned long arg)
{
    U32 tmp_tag;

    if (__get_user(tmp_tag, (U32 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[FileEndNotification]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_FileEndNotification(tmp_tag);

    return 0;
}

static int _MDrv_MAD_FileEndNotification_SE(unsigned long arg)
{
    U32 tmp_tag;

    if (__get_user(tmp_tag, (U32 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[FileEndNotification_SE]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_FileEndNotification_SE(tmp_tag);

    return 0;
}

static int _MDrv_MAD_FileCheckPlayDone(unsigned long arg)
{
    U32 data=0;

    data = MDrv_MAD_ReadMailBox(DSP_DEC, 7) ;

    if (copy_to_user((void *)arg, &data, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[FileCheckPlayDone]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_FileCheckPlayDone_SE(unsigned long arg)
{
    U32 data=0;

    data = MDrv_MAD_ReadMailBox(DSP_SE, 7) ;

    if (copy_to_user((void *)arg, &data, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[FileCheckPlayDone_SE]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_FileGetFrameNum(unsigned long arg)
{
    U32 frameNum=0;

    frameNum = MDrv_MAD_MPEG_GetFrameNum() ;

    if (copy_to_user((void *)arg, &frameNum, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[FileGetFrameNum]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Check_Copy_Rqt(unsigned long arg)
{
    U32 check_request=0;

    check_request = MDrv_MAD_Check_Copy_Rqt() ;

    if (copy_to_user((void *)arg, &check_request, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[Check_Copy_Rqt]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Audio_Mpeg_Info(unsigned long arg)
{
    MPEG_ES_INFO_T audio_info;

    audio_info.bitRate = MDrv_MAD_MPEG_GetBitRate();
    audio_info.sampleRate = MDrv_MAD_Mpeg_GetSampleRate();
    audio_info.channelNum = MDrv_MAD_Mpeg_GetChannelNum();
    audio_info.layer = MDrv_MAD_MPEG_GetLayer();

    if (copy_to_user((void *)arg, &audio_info, sizeof(MPEG_ES_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[Audio_Mpeg_Info]:Copy to user space data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Audio_AC3_Info(unsigned long arg)
{
    AC3_ES_INFO_T audio_info;

    audio_info.bitRate = MDrv_MAD_DvbGet_AC3_Bitrate();
    audio_info.sampleRate = MDrv_MAD_DvbGet_AC3_Fscod();
    audio_info.channelNum = MDrv_MAD_DvbGet_AC3_Acmod();
    audio_info.EAC3 = MDrv_MAD_DvbGet_AC3_Version();

    if (copy_to_user((void *)arg, &audio_info, sizeof(AC3_ES_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[Audio_AC3_Info]:Copy to user space data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Get_DDP_AD_Status(unsigned long arg)
{
    U8 ADStatus;

    ADStatus = MDrv_MAD_Get_DDP_AD_Status();

    if (copy_to_user((void *)arg, &ADStatus, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[Get_DDP_AD_Status]:Copy to user space data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Set_DDP_AD_Mode(unsigned long arg)
{
    U8 ADmode;

    if (__get_user(ADmode, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[HDMI_PCM_NONPCM_Set]:Copy from user space data error\n"));
        return -EFAULT;
    }
	
    MDrv_MAD_Set_DDP_AD_Mode(ADmode);
	
    return 0;
}

static int _MDrv_MAD_Get_PTS(unsigned long arg)
{
    long long pts;

    pts = MDrv_MAD_Get_PTS();

    if (copy_to_user((void *)arg, &pts, sizeof(long long)))
    {
        MAD_DEBUG_DDI(printk("[Get_PTS]:Copy to user space data error\n"));
        return -EFAULT;
    }

    return 0;
}
	
static int _MDrv_MAD_Audio_HEAAC_Info(unsigned long arg)
{
    HEAAC_ES_INFO_T audio_info;

    audio_info.sampleRate = MDrv_MAD_HEAAC_GetSampleRate();
    audio_info.channelNum = MDrv_MAD_HEAAC_GetChannelNum();
    audio_info.version = MDrv_MAD_HEAAC_GetVersion();
    audio_info.Transmissionformat = MDrv_MAD_HEAAC_GetTransmissionformat();

    if (copy_to_user((void *)arg, &audio_info, sizeof(HEAAC_ES_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[Audio_HEAAC_Info]:Copy to user space data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_DSP_Alive_Check(unsigned long arg)
{
    U32 checkAlive=0;
    U32 cnt=0,data_b,data_f=0;

    data_b = MHal_MAD_ReadReg(REG_MAD_TIMER_COUNTER) ;
    for(cnt=0;cnt<10000;cnt++)
    {
        data_f = MHal_MAD_ReadReg(REG_MAD_TIMER_COUNTER) ;
        if(data_f != data_b)
        {
            checkAlive++;
            break;
        }
        schedule_timeout(2);
        data_b = data_f ;
        //printk("<0x%x> ",data_f);
    }

    if (copy_to_user((void *)arg, &checkAlive, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[DSP_Alive_Check]:Copy to user space data error\n"));
        return -EFAULT;
    }

    return 0;
}


static int _MDrv_MAD_DSP2_Alive_Check(unsigned long arg)
{
    U32 checkAlive=0;
    U32 cnt=0,data_b=0,data_f=0;

    data_b = MHal_MAD_ReadReg(REG_SOUND_TIMER_COUNTER) ;
    for(cnt=0;cnt<10000;cnt++)
    {
        data_b = MHal_MAD_ReadReg(REG_SOUND_TIMER_COUNTER) ;
        data_f = MDrv_MAD_SeTimer_CNT();
        if(data_f != data_b)
        {
            checkAlive++;
            break;
        }
        schedule_timeout(2);
        data_b = data_f ;
        //printk("<0x%x> ",data_f);
    }

    if (copy_to_user((void *)arg, &checkAlive, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[DSP_Alive_Check]:Copy to user space data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_PCMStartUpload(unsigned long arg)
{
    MDRV_BT_T BT_info;

    if (copy_from_user(&BT_info, (MDRV_BT_T __user *)arg, sizeof(MDRV_BT_T)))
    {
        MAD_DEBUG_DDI(printk("[PCMStartUpload]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_PCMStartUpload(&BT_info);

    return 0;
}

static int _MDrv_MAD_PCMStopUpload(void)
{

    MDrv_MAD_PCMStopUpload();

    return 0;
}

static int _MDrv_MAD_PCMStartDownload(unsigned long arg)
{
    MDRV_BT_T BT_info;

    if (copy_from_user(&BT_info, (MDRV_BT_T __user *)arg, sizeof(MDRV_BT_T)))
    {
        MAD_DEBUG_DDI(printk("[PCMStartDownload]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_PCMStartDownload(&BT_info);

    return 0;
}

static int _MDrv_MAD_PCMStopDownload(void)
{

    MDrv_MAD_PCMStopDownload();

    return 0;
}

static int _MDrv_MAD_ReadTimeStamp(unsigned long arg)
{
    U32 timestamp;

    timestamp = MDrv_MAD_ReadTimeStamp() ;

    if (copy_to_user((void *)arg, &timestamp, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[ReadTimeStamp]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_ReadTimeStamp_SE(unsigned long arg)
{
    U32 timestamp;

    timestamp = MDrv_MAD2_ReadTimeStamp() ;

    if (copy_to_user((void *)arg, &timestamp, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[Read Se TimeStamp]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_HDMI_DolbyMonitor(unsigned long arg)
{
    U8 reg2c40_rdtmp;

    reg2c40_rdtmp = MDrv_MAD_HDMI_DolbyMonitor() ;

    if (copy_to_user((void *)arg, &reg2c40_rdtmp, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[HDMI_DolbyMonitor]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_DTV_HDMI_CFG(unsigned long arg)
{
    U8 ctrl;

    if (__get_user(ctrl, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_DTV_HDMI_CFG(ctrl);

    return 0;
}

static int _MDrv_MAD_AC3Dec_DIS(unsigned long arg)
{
    U8 ac3_dis_en;

    if (__get_user(ac3_dis_en, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_AC3Dec_DIS(ac3_dis_en);

    return 0;
}

static int _MDrv_MAD_WMA_GetSampleRate(unsigned long arg)
{
    U16 wma_SampleRate;

    wma_SampleRate = MDrv_MAD_WMA_GetSampleRate() ;

    if (copy_to_user((void *)arg, &wma_SampleRate, sizeof(U16)))
    {
        MAD_DEBUG_DDI(printk("[wma_SampleRate]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_WMA_GetBitRate(unsigned long arg)
{
    U32 wma_BitRate;

    wma_BitRate = MDrv_MAD_WMA_GetBitRate() ;

    if (copy_to_user((void *)arg, &wma_BitRate, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[wma_BitRate]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_aac_getSampleRate(unsigned long arg)
{
    U32 aac_SampleRate;

    aac_SampleRate = MDrv_MAD_aac_getSampleRate() ;

    if (copy_to_user((void *)arg, &aac_SampleRate, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[aac_SampleRate]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_GetDSPCodeType(unsigned long arg)
{
    U8 DSPCodeType;

    DSPCodeType = MDrv_MAD_GetDSPCodeType() ;

    if (copy_to_user((void *)arg, &DSPCodeType, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[GetDSPCodeType]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_GetDSP2CodeType(unsigned long arg)
{
    U8 DSP2CodeType;

    DSP2CodeType = MDrv_MAD_GetDSP2CodeType() ;

    if (copy_to_user((void *)arg, &DSP2CodeType, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[GetDSP2CodeType]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Dvb_XPCM_setParam(unsigned long arg)
{
    XPCM_INFO_T XPCM_info;
    U8 audioType, channels, bitsPerSample;
    U32 sampleRate, blockSize, samplePerBlock;

    if (copy_from_user(&XPCM_info, (XPCM_INFO_T __user *)arg, sizeof(XPCM_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[XPCM_setParam]:Copy from user space data error\n"));
        return -EFAULT;
    }
    audioType = XPCM_info.audioType;
    channels = XPCM_info.channels;
    sampleRate = XPCM_info.sampleRate;
    bitsPerSample = XPCM_info.bitsPerSample;
    blockSize = XPCM_info.blockSize;
    samplePerBlock = XPCM_info.samplePerBlock;

    MDrv_MAD_Dvb_XPCM_setParam(audioType, channels, sampleRate,
                                                        bitsPerSample, blockSize, samplePerBlock);

    return 0;
}

static int _MDrv_MAD_Dvb2_XPCM_setParam(unsigned long arg)
{
    XPCM_INFO_T XPCM_info;
    U8 audioType, channels, bitsPerSample;
    U32 sampleRate, blockSize, samplePerBlock;

    if (copy_from_user(&XPCM_info, (XPCM_INFO_T __user *)arg, sizeof(XPCM_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[XPCM_setParam]:Copy from user space data error\n"));
        return -EFAULT;
    }
    audioType = XPCM_info.audioType;
    channels = XPCM_info.channels;
    sampleRate = XPCM_info.sampleRate;
    bitsPerSample = XPCM_info.bitsPerSample;
    blockSize = XPCM_info.blockSize;
    samplePerBlock = XPCM_info.samplePerBlock;

    MDrv_MAD_Dvb2_XPCM_setParam(audioType, channels, sampleRate,
                                                        bitsPerSample, blockSize, samplePerBlock);

    return 0;
}

static int _MDrv_MAD_Dvb_RA8_setParam(unsigned long arg)
{
    RA8_INFO_T RA8_info;
    U16 mNumCodecs, mSamples, mSampleRate;
    U16* Channels;
    U16* Regions;
    U16* cplStart;
    U16* cplQbits;
    U16*  FrameSize;

    if (copy_from_user(&RA8_info, (RA8_INFO_T __user *)arg, sizeof(RA8_INFO_T)))
    {
        MAD_DEBUG_DDI(printk("[RA8_setParam]:Copy from user space data error\n"));
        return -EFAULT;
    }
    mNumCodecs = RA8_info.mNumCodecs;
    mSamples = RA8_info.mSamples;
    mSampleRate = RA8_info.mSampleRate;
    Channels = RA8_info.Channels;
    Regions = RA8_info.Regions;
    cplStart = RA8_info.cplStart;
    cplQbits = RA8_info.cplQbits;
    FrameSize = RA8_info.FrameSize;

    MDrv_MAD_Dvb_RA8_setParam(mNumCodecs, mSamples, mSampleRate,
                           Channels, Regions, cplStart, cplQbits, FrameSize);

    return 0;
}

static int _MDrv_MAD_ProcessSetADAbsoluteVolume(unsigned long arg)
{
    PROCESS_ADVOLUME ADVolume;

    if (copy_from_user(&ADVolume, (PROCESS_ADVOLUME __user *)arg, sizeof(PROCESS_ADVOLUME)))
    {
        MAD_DEBUG_DDI(printk("[SetADVolume]:Copy from user space data error\n"));
        return -EFAULT;
    }
    MDrv_MAD_ProcessSetADAbsoluteVolume(ADVolume.integer_vol, ADVolume.fraction_vol);
    return 0;
}

static int _MDrv_MAD_ProcessADSetMute(unsigned long arg)
{
    U8 enable;


    if (__get_user(enable, (U8 __user *)arg))
    {
        MAD_DEBUG_DDI(printk("[ADSetMute]:Get user data error\n"));
        return -EFAULT;
    }
    MDrv_MAD_ProcessADSetMute(enable);

    return 0;
}
static int _MDrv_MAD_SetADOutputMode(unsigned long arg)
{
    U8 out_mode;


    if (__get_user(out_mode, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_SetADOutputMode(out_mode);
    return 0;
}
static int _MDrv_MAD_SetADMixMode(unsigned long arg)
{
     AD_MIX_T AD_mix;

    if (copy_from_user(&AD_mix, (AD_MIX_T __user *)arg, sizeof(AD_MIX_T)))
    {
        MAD_DEBUG_DDI(printk("[Dvb_setADMixMode]:Copy from user space data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_Dvb_setADMixMode(AD_mix.mix_mode, AD_mix.en_mix);

    return 0;
}
static int _MDrv_MAD_GetMADBase(unsigned long arg)
{
    MAD_BASE_INFO madBase;

    MDrv_MAD_GetMADBase(&madBase);
    if (copy_to_user((void *)arg, &madBase, sizeof(MAD_BASE_INFO)))
    {
        MAD_DEBUG_DDI(printk("[GetFileInfo]:Get user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_SetClearVoiceIII(unsigned long arg)
{
    Cv3info_T cv3parem;

    if (copy_from_user(&cv3parem, (PROCESS_VOLUME __user *)arg, sizeof(Cv3info_T)))
    {
        MAD_DEBUG_DDI(printk("[SetClearVoiceIII]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_SetClearVoiceIII(&cv3parem);
    return 0;
}

static int _MDrv_MAD_SetClearVoiceOnOff(unsigned long arg)
{
    U8 CVIII;

    if (__get_user(CVIII, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_SetClearVoiceOnOff(CVIII);
    return 0;
}

static int _MDrv_MAD_CV3_SetAVL2OnOff(unsigned long arg)
{
    U8 avl2;

    if (__get_user(avl2, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_CV3_SetAVL2OnOff(avl2);
    return 0;
}

static int _MDrv_MAD_CV3_SetRurroundOnOff(unsigned long arg)
{
    U8 surround;

    if (__get_user(surround, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_CV3_SetSurroundOnOff(surround);
    return 0;
}

static int _MDrv_MAD_CV3_SetUEQOnOff(unsigned long arg)
{
    U8 ueq;

    if (__get_user(ueq, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_CV3_SetUEQOnOff(ueq);
    return 0;
}

static int _MDrv_MAD_CV3_SetVolume(unsigned long arg)
{
    Cv3VolumeInfo_T VOLparem;

    if (copy_from_user(&VOLparem, (PROCESS_VOLUME __user *)arg, sizeof(Cv3VolumeInfo_T)))
    {
        MAD_DEBUG_DDI(printk("[CV3 SetVolume]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_CV3_SetVolume(&VOLparem);

    return 0;
}

static int _MDrv_MAD_CV3_SetBassEnhance(unsigned long arg)
{
    BassEnhanceInfo_T BassEnhanceParem;

    if (copy_from_user(&BassEnhanceParem, (PROCESS_VOLUME __user *)arg, sizeof(BassEnhanceInfo_T)))
    {
        MAD_DEBUG_DDI(printk("[CV3 SetBassEnhance]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_CV3_SetBassEnhance(&BassEnhanceParem);

    return 0;
}


static int _MDrv_MAD_CV3_SetUEQ(unsigned long arg)
{
    Cv3EQinfo_T EQparem;

    if (copy_from_user(&EQparem, (PROCESS_VOLUME __user *)arg, sizeof(Cv3EQinfo_T)))
    {
        MAD_DEBUG_DDI(printk("[CV3 SetUEQ]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_CV3_SetUEQ(&EQparem);
    return 0;
}

static int _MDrv_MAD_CV3_SetAVL2(unsigned long arg)
{
    Cv3AVL2info_T AVL2param;

    if (copy_from_user(&AVL2param, (PROCESS_VOLUME __user *)arg, sizeof(Cv3AVL2info_T)))
    {
        MAD_DEBUG_DDI(printk("[CV3 SetAVL2]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_CV3_SetAVL2(&AVL2param);
    return 0;
}

static int _MDrv_MAD_CV3_SetCV(unsigned long arg)
{
    Cv3CVinfo_T CVparam;

    if (copy_from_user(&CVparam, (PROCESS_VOLUME __user *)arg, sizeof(Cv3CVinfo_T)))
    {
        MAD_DEBUG_DDI(printk("[CV3 SetCV]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_CV3_SetCV(&CVparam);
    return 0;
}

static int _MDrv_MAD_CV3_SetSurround(unsigned long arg)
{
    Cv3SurroundInfo_T surround_param;

    if (copy_from_user(&surround_param, (PROCESS_VOLUME __user *)arg, sizeof(Cv3SurroundInfo_T)))
    {
        MAD_DEBUG_DDI(printk("[CV3 SetCV]:Get user data error\n"));
        return -EFAULT;
    }

    MDrv_MAD_CV3_SetSurround(&surround_param);
    return 0;
}

static int _MDrv_MAD_CV3_PVC_Monitor(unsigned long arg)
{
    U32 pvc_monitor;

    MDrv_MAD_CV3_PVC_Monitor(&pvc_monitor) ;

    if (copy_to_user((void *)arg, &pvc_monitor, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[mpeg_GetSoundMode]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;	
}


static int _MDrv_MAD_CV3_SetMode(unsigned long arg)
{
    U32 cvMode;

    if (__get_user(cvMode, (U32 __user *)arg))
        return -EFAULT;

    MDrv_MAD_CV3_SetMode(cvMode);
    return 0;
}

static int _MDrv_MAD_MPEG_SetFileSize(unsigned long arg)
{
    U8 bValue;

    if (__get_user(bValue, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_MPEG_SetFileSize(bValue);
    return 0;
}

static int _MDrv_MAD_PowerOn_Melody_Path(unsigned long arg)
{
    U8 bPath;

    if (__get_user(bPath, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_PowerOn_Melody_Path(bPath);
    return 0;
}

static int _MDrv_MAD_ADEC_ES_Check(unsigned long arg)
{
    U8 bData;

    bData = MDrv_MAD_ADEC_ES_Check() ;

    if (copy_to_user((void *)arg, &bData, sizeof(U8)))
    {
        MAD_DEBUG_DDI(printk("[ADEC_ES_Check]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Set_Dolby_DRC_Mode(unsigned long arg)
{
    U8 DRC_mod;

    if (__get_user(DRC_mod, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_Set_Dolby_DRC_Mode(DRC_mod);

    return 0;
}

static int _MDrv_MAD_Set_Dolby_AD_DRC_Mode(unsigned long arg)
{
    U8 DRC_mod;

    if (__get_user(DRC_mod, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_Set_Dolby_AD_DRC_Mode(DRC_mod);

    return 0;
}

static int _MDrv_MAD_Set_Dolby_Downmix_Mode(unsigned long arg)
{
    U8 dmix_mod;

    if (__get_user(dmix_mod, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_Set_Dolby_Downmix_Mode(dmix_mod);

    return 0;
}

static int _MDrv_MAD_Set_SRS_DC(unsigned long arg)
{
    U8 bDC;

    if (__get_user(bDC, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_SRS_DC(bDC);
    return 0;
}

static int _MDrv_MAD_Set_SRS_TruBass(unsigned long arg)
{
    U8 bTruBass;

    if (__get_user(bTruBass, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_SRS_TruBass(bTruBass);
    return 0;
}

static int _MDrv_MAD_SetSRSPara(unsigned long arg)
{
    SRS_PARA setPara;

    if (copy_from_user(&setPara, (SRS_PARA __user *)arg, sizeof(SRS_PARA)))
        return -EFAULT;

    MDrv_MAD_SetSRSPara(setPara.mode, setPara.value);

    return 0;
}

#if (!USE_PARAM_POLLING)
static int _MDrv_MAD_SetSRSPara32(unsigned long arg)
{
    SRS_PARA32 setPara32;

    if (copy_from_user(&setPara32, (SRS_PARA32 __user *)arg, sizeof(SRS_PARA32)))
        return -EFAULT;

    MDrv_MAD_SetSRSPara32(setPara32.mode, setPara32.value);

    return 0;
}
#endif

static int _MDrv_MAD_AuProcessWritePARAMETER(unsigned long arg)
{
    PROCESS_WRITE_PARAM wr_param;

    if (copy_from_user(&wr_param, (PROCESS_WRITE_PARAM __user *)arg, sizeof(PROCESS_WRITE_PARAM)))
        return -EFAULT;

    MDrv_MAD_AuProcessWritePARAMETER(wr_param.dsp_addr, wr_param.value);

    return 0;
}

static int _MDrv_MAD_Audio_Monitor(void)
{
    //MAD_DEBUG_DDI(printk("[Audio_Monitor]\n"));
    MDrv_MAD_Audio_Monitor();
    return 0;
}

static int _MDrv_MAD_GetDecDspReviveCount(unsigned long arg)
{
    U32 ReviveCount=0;
    ReviveCount = MDrv_MAD_GetDecDspReviveCount();

    if (copy_to_user((void *)arg, &ReviveCount, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[GetDecDspReviveCount]:Copy to user data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Dvb2_Play_BT(unsigned long arg)
{
    BOOL BT_play;

    if (__get_user(BT_play, (BOOL __user *)arg))
        return -EFAULT;

    MAD_DEBUG_DDI(printk("_MDrv_MAD_Dvb2_Play_BT(%d)\n",BT_play));

    MDrv_MAD_Dvb2_Play_BT(BT_play);
    return 0;
}

static int _MDrv_MAD_DVB_PcmLevelControl(unsigned long arg)
{
    U8 bsysmod;

    if (__get_user(bsysmod, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_DVB_PcmLevelControl(bsysmod);
    return 0;
}


static int _MDrv_MAD_WMA_SetASFParm(unsigned long arg)
{
    WMA_SET_PARAM wma_param;

    if (copy_from_user(&wma_param, (WMA_SET_PARAM __user *)arg, sizeof(WMA_SET_PARAM)))
        return -EFAULT;

    MDrv_MAD_WMA_SetASFParm(wma_param.param1, wma_param.param2);

    return 0;
}

//Mstar add for Skype, 2009/09/22
static int _MDrv_MAD_Init_Skype(unsigned long arg)
{
    MDRV_SKYPE_T Skype_info;

    if (copy_from_user(&Skype_info, (MDRV_SKYPE_T __user *)arg, sizeof(MDRV_SKYPE_T)))
    {
        MAD_DEBUG_DDI(printk("[Init_Skype]:Copy from user space data error\n"));
        return -EFAULT;
    }

    // PCM down
    if((U32)Skype_info.pPcmInSrcAddr>=0x10000000)
        Skype_info.pPcmInSrcAddr +=0xB0000000;
    else
        Skype_info.pPcmInSrcAddr +=0xA0000000;

    // PCM up
    if((U32)Skype_info.pPcmOutDestAddr>=0x10000000)
        Skype_info.pPcmOutDestAddr +=0xB0000000;
    else
        Skype_info.pPcmOutDestAddr +=0xA0000000;

    // Bit Stream down
    if((U32)Skype_info.pBsInSrcAddr>=0x10000000)
        Skype_info.pBsInSrcAddr +=0xB0000000;
    else
        Skype_info.pBsInSrcAddr +=0xA0000000;

    // Bit Stream up
    if((U32)Skype_info.pBsOutDestAddr>=0x10000000)
        Skype_info.pBsOutDestAddr +=0xB0000000;
    else
        Skype_info.pBsOutDestAddr +=0xA0000000;

    MDrv_MAD_Init_Skype(&Skype_info);

    return 0;
}

static int _MDrv_MAD_Stop_Skype(void)
{
    MDrv_MAD_Stop_Skype();
    return 0;
}

static int _MDrv_MAD_Skype_Dn_PCM(void)
{
    if(MDrv_MAD_Skype_Dn_PCM() == 0)
        return -EFAULT;

    return 0;
}

static int _MDrv_SIF_ADC_Reset(void)
{
    MDrv_SIF_ADC_Reset();
    return 0;	
}

static int _MDrv_MAD_Set_Cut_Boost(unsigned long arg)
{
    U8 scale;

    if (__get_user(scale, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_Set_Cut_Boost(scale);

    return 0;
}

static int _MDrv_MAD_Set_ADC_Threshold(unsigned long arg)
{
    U8 threshold;

    if (__get_user(threshold, (U8 __user *)arg))
        return -EFAULT;

    MDrv_MAD_Set_ADC_Threshold(threshold);

    return 0;
}

static int _MDrv_MAD_Get_PCM_Energy(unsigned long arg)
{
    U32 u32PCMEnergy;

    u32PCMEnergy = MDrv_MAD_Get_PCM_Energy();

    if (copy_to_user((void *)arg, &u32PCMEnergy, sizeof(U32)))
    {
        MAD_DEBUG_DDI(printk("[Get_PCM_Energy]:Copy to user space data error\n"));
        return -EFAULT;
    }

    return 0;
}

static int _MDrv_MAD_Vol_Reg_Restore(void)
{
    MDrv_MAD_Vol_Reg_Restore();
    return 0;
}

static int _mod_mad_open (struct inode *inode, struct file *filp)
{
    MAD_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static int _mod_mad_release(struct inode *inode, struct file *filp)
{
    MAD_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_mad_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    MAD_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static ssize_t _mod_mad_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // remove it if it's not required
    MAD_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

static unsigned int _mod_mad_poll(struct file *filp, poll_table *wait)
{
    //MAD_PRINT("%s is invoked\n", __FUNCTION__);
        poll_wait(filp, &_mad_wait_queue,  wait);
    if(_u16MadEvent !=0 )
        _u16MadEvent|=MAD_EVENT_POLLED;

        return (!_u16MadEvent)? 0: POLLIN; //  | POLLPRI;
}

static int _mod_mad_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int         retval = 0;
    int         err= 0;

    //MAD_PRINT("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((MAD_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> MDrv_MAD_IOC_MAXNR))
    {
        MAD_DEBUG_DDI(printk("IO Command Error "));
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
        MAD_DEBUG_DDI(printk("User space variable access error\n"));
        return -EFAULT;
    }


    PROBE_IO_ENTRY(MDRV_MAJOR_MAD, _IOC_NR(cmd));

    switch(cmd)
    {
    case MDrv_MAD_IOC:
        MAD_PRINT("ioctl: receive MDrv_MAD_IOC\n");
        break;
    case MDrv_MAD_IOCREAD:
        MAD_PRINT("ioctl: receive MDrv_MAD_IOCREAD with user pointer 0x%08x 0x%08x\n", (u32)arg, *((u32*)(arg)));
        break;
/*
    case MDrv_MAD_IOC_EVENT_GET:
        if ((!_u32MadEventId) && (wait_event_interruptible(_mad_wait_queue, 0!= _u32MadEventId))))
            return -ERESTARTSYS;
        disable_irq(E_IRQ_MAD);
        spin_lock(&_mad_spinlock);
        pArg->u32EventMad=      _u32MadEventId;
        pArg->u32SecRdyId=      _u32MadSecRdyId;
        pArg->u32SecOvfId=      _u32SecOvfId;
        spin_unlock(&_mad_spinlock);
        enable_irq(E_IRQ_MAD);
        break;
*/
    case MDrv_MAD_IOCWRITE:
        MAD_PRINT("ioctl: receive MAD_IOCWRITE with user pointer 0x%08x 0x%08x\n", (u32)arg, *((u32*)(arg)));
        break;
    case MDrv_MAD_IOCRW:
        MAD_PRINT("ioctl: receive MAD_IOCRW with user pointer 0x%08x 0x%08x\n", (u32)arg, *((u32*)(arg)));
        break;
    case MDrv_MAD_SET_BUFFER_INFO:
        retval = _MDrv_MAD_SetBufferInfo(arg);
        break;
    case MDrv_MAD_INITIALIZE_MODULE:
        MAD_PRINT("ioctl: receive MDrv_MAD_INITIALIZE_MODULE ....\n");
        _MDrv_MAD_InitializeModule(arg);
        break;
    case MDrv_MAD_SET_INPUT_PATH:
        retval = _MDrv_MAD_SetInputPath(arg);
        break;
    case MDrv_MAD_SET_INTERNAL_PATH:
        retval = _MDrv_MAD_SetInternalPath(arg);
        break;
    case MDrv_MAD_SET_DTV_NORMALPATH:
        MDrv_MAD_SetDTV_NormalPath();
        break;
     case MDrv_MAD_SET_ATV_NORMALPATH:
        MDrv_MAD_SetATV_NormalPath();
        break;
    case MDrv_MAD_SET_POWER_OFF:
        retval = _MDrv_MAD_Set_Power_Off(arg);
        break;
    case MDrv_MAD_RESET_MAD:
        MDrv_MAD_ResetMAD();
        break;
    case MDrv_MAD_PIO8_DEC:
        retval = _MDrv_MAD_TriggerPIO8();
        break;
   case MDrv_MAD_PIO8_SE:
        retval = _MDrv_MAD2_TriggerPIO8();
        break;
    case MDrv_MAD_SAMPLING_RATE:
        retval = _MDrv_MAD_AudioSamplingRate(arg);
        break;
    case MDrv_MAD_RD_MAILBOX:
        retval = _MDrv_MAD_ReadMailBox(arg);
        break;
    case MDrv_MAD_WR_MAILBOX:
        retval = _MDrv_MAD_WriteMailBox(arg);
        break;
    case MDrv_MAD_RD_REG:
        retval = _MDrv_MAD_ReadREG(arg);
        break;
    case MDrv_MAD_WR_REG:
        retval = _MDrv_MAD_WriteREG(arg);
        break;
    case MDrv_MAD_DVB_GETSOUNDMODE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_DVB_GETSOUNDMODE\n"));
        retval = _MDrv_MAD_DvbGetSoundMode(arg);
        break;
    case MDrv_MAD_DVB_SETSYSTEM:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_DVB_SETSYSTEM\n"));
        retval = _MDrv_MAD_SetSystem(arg);
        break;
    case MDrv_MAD_DVB_SETDECCMD:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_DVB_SETDECCMD\n"));
        retval = _MDrv_MAD_Dvb_setDecCmd(arg);
        break;
    case MDrv_MAD_DVB2_SETDECCMD:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD2_SetDecCmd\n"));
        retval = _MDrv_MAD2_SetDecCmd(arg);
        break;
    case MDrv_MAD_DVB_SETFREERUN:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_DVB_SETFREERUN\n"));
        retval = _MDrv_MAD_SetFreeRun(arg);
        break;
    case MDrv_MAD_DVB2_SETFREERUN:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD2_SetFreeRun\n"));
        retval = _MDrv_MAD2_SetFreeRun(arg);
        break;
    case MDrv_MAD_SET_DACOUT_MUTE:
        retval = _MDrv_MAD_ProcessSetMute(arg);
        break;
    case MDrv_MAD_SET_VOLUME:
        retval = _MDrv_MAD_ProcessSetVolume(arg);
        break;
    case MDrv_MAD_SET_DACOUT_VOLUME:
        retval = _MDrv_MAD_ProcessAbsoluteVolume(arg);
        break;
    case MDrv_MAD_SET_SPDIF_VOLUME:
        retval = _MDrv_MAD_ProcessSPDIFVolume(arg);
        break;
    case MDrv_MAD_PROCESS_SETBALANCE:
        retval = _MDrv_MAD_ProcessSetBalance(arg);
        break;
    case MDrv_MAD_PROCESS_SETBASS:
        retval = _MDrv_MAD_ProcessSetBass(arg);
        break;
    case MDrv_MAD_PROCESS_SETTREBLE:
        retval = _MDrv_MAD_ProcessSetTreble(arg);
        break;
    case MDrv_MAD_PROCESS_SETBBE:
        retval = _MDrv_MAD_ProcessSetBBE(arg);
        break;
    case MDrv_MAD_PROCESS_SETSRS:
        retval = _MDrv_MAD_ProcessSetSRS(arg);
        break;
    case MDrv_MAD_PROCESS_SETEQUALIZER_LEVEL:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_PROCESS_SETEQUALIZER_LEVEL\n"));
        retval = _MDrv_MAD_ProcessSetEqualizerLevel(arg);
        break;
    case MDrv_MAD_GET_ENCODE_DONE_FLAG:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_ENCODE_DONE_FLAG\n"));
        retval = _MDrv_MAD_GetEncodeDoneFlag(arg);
        break;
    case MDrv_MAD_CLR_ENCODE_DONE_FLAG:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CLR_ENCODE_DONE_FLAG\n"));
        retval = _MDrv_MAD_ClrEncodeDoneFlag();
        break;
    case MDrv_MAD_START_ENCODING:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_START_ENCODING\n"));
        retval = _MDrv_MAD_StartEncoding();
        break;
    case MDrv_MAD_STOP_ENCODING:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_STOP_ENCODING\n"));
        retval = _MDrv_MAD_StopEncoding();
        break;
    case MDrv_MAD_GET_ENCODE_BUF_INFO:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_ENCODE_BUF_INFO\n"));
        retval = _MDrv_MAD_GetEncBufInfo(arg);
        break;
    case MDrv_MAD_GET_ENCODE_FRAME_INFO:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_ENCODE_FRAME_INFO\n"));
        retval = _MDrv_MAD_GetEncFrameInfo(arg);
        break;
    case MDrv_MAD_GET_DECODING_TYPE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_DECODING_TYPE\n"));
        retval = _MDrv_MAD_GetDecodingType(arg);
        break;
    case MDrv_MAD_START_DECODING:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_START_DECODING\n"));
        retval = _MDrv_MAD_StartDecoding(arg);
        break;
     case MDrv_MAD_START_DVB2_DECODING:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_START_DVB2_DECODING\n"));
        retval = _MDrv_MAD_StartDvb2_Decoding(arg);
        break;
     case MDrv_MAD_STOP_DECODING:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_STOP_DECODING\n"));
        retval = _MDrv_MAD_StopDecoding();
        break;
     case MDrv_MAD_GET_CUR_ANALOG_MODE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_CUR_ANALOG_MODE\n"));
        retval = _MDrv_MAD_GetCurAnalogMode(arg);
        break;
     case MDrv_MAD_SET_USER_ANALOG_MODE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_USER_ANALOG_MODE\n"));
        retval = _MDrv_MAD_SetUserAnalogMode(arg);
        break;
     case MDrv_MAD_SET_AUTO_VOLUME_CONTROL:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_AUTO_VOLUME_CONTROL\n"));
        retval = _MDrv_MAD_SetAutoVolumeControl(arg);
        break;
     case MDrv_MAD_GET_BTSC_A2_STEREO_LEVEL:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_BTSC_A2_STEREO_LEVEL\n"));
        retval = _MDrv_MAD_GetBtscA2StereoLevel(arg);
        break;
#if 0
    case MDrv_MAD_SIF_SETSYSTEM:
        //MAD_PRINT("ioctl: receive MDrv_MAD_SIF_SETSYSTEM...\n");
        retval = _MDrv_MAD_SifSetSystem(arg);
        break;
#endif
    case MDrv_MAD_AMPLIFIER_ON:
        MDrv_MAD_AmplifierOn();
        break;
    case MDrv_MAD_AMPLIFIER_OFF:
        MDrv_MAD_AmplifierOff();
        break;
    case MDrv_MAD_SET_SPDIF_MUTE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_SPDIF_MUTE\n"));
        retval = _MDrv_MAD_SPDIF_SetMute(arg);
        break;
    case MDrv_MAD_SET_SPDIF_OUTPUT_TYPE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_SPDIF_OUTPUT_TYPE\n"));
        retval = _MDrv_MAD_SetSPDIFOutputType(arg);
        break;
    case MDrv_MAD_SET_ADUIO_PLLS_FREQ:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_ADUIO_PLLS_FREQ\n"));
        retval = _MDrv_MAD_SetAudioPLLSFreq(arg);
        break;
    case MDrv_MAD_SET_PCM_DELAY_TIME:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_PCM_DELAY_TIME\n"));
        retval = _MDrv_MAD_SetPCMDelayTime(arg);
        break;
    case MDrv_MAD_SET_SPDIF_DELAY_TIME:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_PCM_DELAY_TIME\n"));
        retval = _MDrv_MAD_SetSPDIFDelayTime(arg);
        break;
    case MDrv_MAD_GET_HDMI_AUDIO_MODE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_HDMI_AUDIO_MODE\n"));
        retval = _MDrv_MAD_GetHDMIAudioMode(arg);
        break;
    case MDrv_MAD_GET_HDMI_SYNTH_FREQ:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_HDMI_SYNTH_FREQ\n"));
        retval = _MDrv_MAD_GetHDMISynthFreq(arg);
        break;
    case MDrv_MAD_GET_ADEC_STATUS:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_ADEC_STATUS\n"));
        retval = _MDrv_MAD_GetAdecStatus(arg);
        break;
    case MDrv_MAD_SET_HP_OUTPUT_TYPE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_HP_OUTPUT_TYPE\n"));
        retval = _MDrv_MAD_SetHPOutputType(arg);
        break;
    case MDrv_MAD_SET_BTSC_A2_THRESHOLD_LEVEL:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_BTSC_A2_THRESHOLD_LEVEL\n"));
        retval = _MDrv_MAD_SetBtscA2ThresholdLevel(arg);
        break;
    case MDrv_MAD_SET_SPKOUT_MODE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_SPKOUT_MODE\n"));
        retval = _MDrv_MAD_SetSPKOutMode(arg);
        break;
    case MDrv_MAD_SIF_ENABLE_DACOUT:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SIF_ENABLE_DACOUT\n"));
        retval = _MDrv_MAD_SIF_EnableDACOut(arg);
        break;
    case MDrv_MAD_SIF_GET_BAND_DETECT:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SIF_GET_BAND_DETECT\n"));
        retval = _MDrv_MAD_SIF_GetBandDetect(arg);
        break;
    case MDrv_MAD_SIF_SET_BAND_SETUP:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SIF_SET_BAND_SETUP\n"));
        retval = _MDrv_MAD_SIF_SetBandSetup(arg);
        break;
    case MDrv_MAD_SIF_SET_MODE_SETUP:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SIF_SET_MODE_SETUP\n"));
        retval = _MDrv_MAD_SIF_SetModeSetup(arg);
        break;
    case MDrv_MAD_SIF_CHECK_NICAM_DIGITAL:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SIF_CHECK_NICAM_DIGITAL\n"));
        retval = _MDrv_MAD_SIF_CheckNicamDigital(arg);
        break;
    case MDrv_MAD_SIF_CHECK_AVAILABLE_SYSTEM:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SIF_CHECK_AVAILABLE_SYSTEM\n"));
        retval = _MDrv_MAD_SIF_CheckAvailableSystem(arg);
        break;
    case MDrv_MAD_SIF_CHECKA2DK:
        retval = _MDrv_MAD_SIF_CheckA2DK(arg);
        MAD_DEBUG_DDI2(printk("[IOCTL]:_MDrv_MAD_SIF_CheckA2DK\n"));
        break;
        case MDrv_MAD_SIF_SET_HDEVMODE:
        retval = _MDrv_MAD_SIF_SetHDEVMode(arg);
        break;
    // Mstar fast routine to get sif standard..
    case MDrv_MAD_SIF_GET_SOUND_STANDARD:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SIF_GET_SOUND_STANDARD\n"));
        retval = _MDrv_MAD_SIF_GetSoundStandard(arg);
        break;
    case MDrv_MAD_GET_DSP_CLK:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_GET_DSP_CLK\n"));
        MDrv_MAD_GetDSPClk();
        break;
    case MDrv_MAD_DELETE_AUDIO_CLIP:
        break;
    case MDrv_MAD_LOAD_AUDIO_CLIP:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_LOAD_AUDIO_CLIP\n"));
        _MDrv_MAD_LoadAudioClip(arg);
        break;
    case MDrv_MAD_PLAY_AUDIO_CLIP:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_PLAY_AUDIO_CLIP\n"));
        _MDrv_MAD_PlayAudioClip(arg);
        break;
        break;
    case MDrv_MAD_PAUSE_AUDIO_CLIP:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_PAUSE_AUDIO_CLIP\n"));
        MDrv_MAD_PauseAudioClip();
        break;
    case MDrv_MAD_RESUME_AUDIO_CLIP:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_RESUME_AUDIO_CLIP\n"));
        MDrv_MAD_ResumeAudioClip();
        break;
    case MDrv_MAD_STOP_AUDIO_CLIP:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_STOP_AUDIO_CLIP\n"));
        MDrv_MAD_StopAudioClip();
        break;
#if 0
    case MDrv_MAD_VER_PCM_OUT_INIT:
        MAD_PRINT("ioctl: receive MDrv_MAD_VER_PCM_OUT_INIT...\n");
        Ver_Pcm_Out_Init();
        break;
//    case MDrv_MAD_VER_PCM_OUT_PLAY:
//        MAD_PRINT("ioctl: receive MDrv_MAD_VER_PCM_OUT_PLAY...\n");
//        Ver_Pcm_Out_Play();
        break;
#endif

    case MDrv_MAD_SIF_ACCESS_THRESHOLD: // for threshold setting.
        retval = _MDrv_MAD_SIF_AccessThreshold(arg);
        break;
    case MDrv_MAD_SIF_SET_PRESCALE: // for threshold setting.
        retval = _MDrv_MAD_SIF_SetPrescale(arg);
        break;
    case MDrv_MAD_GET_EVENT: // for threshold setting.
        retval = _MDrv_MAD_Get_Event(arg);
        break;
    case MDrv_MAD_SPDIF_SETMODE:
        retval = _MDrv_MAD_SPDIF_SetMode(arg);
        break;
    case MDrv_MAD_SPDIF_SETSCMS:
        retval = _MDrv_MAD_SPDIF_SetSCMS(arg);
        break;
    case MDrv_MAD_SPDIF_GETSCMS:
    	 retval = _MDrv_MAD_SPDIF_GetSCMS(arg);
        break;
    case MDrv_MAD_MONITOR_SPDIF_NONPCM_FS_RATE:
        MDrv_MAD_Monitor_DDPlus_SPDIF_Rate();
        break;
    case MDrv_MAD_HDMI_PCM_NONPCM_SET:
        retval = _MDrv_MAD_HDMI_SetNonpcm(arg);
        break;
   case MDrv_MAD_HDMI_AC3_PATHCFG:
        retval = _MDrv_MAD_HDMI_AC3_PathCFG(arg);
        break;
   case MDrv_MAD_GET_HDMI_MONITOR:
        retval = _MDrv_MAD_GetHDMI_Monitor(arg);
        break;
   case MDrv_MAD_WR_REG_MASK:
        retval = _MDrv_MAD_WriteREGMask(arg);
        break;
    case MDrv_MAD_GET_FILE_INFO:
        retval = _MDrv_MAD_GetFileInfo(arg);
        break;
    case MDrv_MAD_GET_FILE_INFO_SE:
        retval = _MDrv_MAD_GetFileInfo_SE(arg);
        break;
    case MDrv_MAD_GET_AUDIOSTREAM_INFO:
        retval = _MDrv_MAD_GetAudioStreamInfo(arg);
        break;
    case MDrv_MAD_FILE_SET_INPUT:
        retval = _MDrv_MAD_FileSetInput(arg);
        break;
    case MDrv_MAD_FILE_SET_INPUT_SE:
        retval = _MDrv_MAD_FileSetInput_SE(arg);
        break;
    case MDrv_MAD_SET_AUDIOSTREAM_INPUT:
        retval = _MDrv_MAD_SetAudioStreamInput(arg);
        break;
    case MDrv_MAD_FILE_END_NOTIFICATION:
        retval = _MDrv_MAD_FileEndNotification(arg);
        break;
    case MDrv_MAD_FILE_END_NOTIFICATION_SE:
        retval = _MDrv_MAD_FileEndNotification_SE(arg);
        break;
    case MDrv_MAD_FILE_CHECK_PLAYDONE:
        retval = _MDrv_MAD_FileCheckPlayDone(arg);
        break;
    case MDrv_MAD_FILE_CHECK_PLAYDONE_SE:
        retval = _MDrv_MAD_FileCheckPlayDone_SE(arg);
        break;
    case MDrv_MAD_FILE_CHECK_COPY_RQT:
        retval = _MDrv_MAD_Check_Copy_Rqt(arg);
        break;
    case MDrv_MAD_AUDIO_MPEG_INFO:
        retval = _MDrv_MAD_Audio_Mpeg_Info(arg);
        break;
    case MDrv_MAD_AUDIO_AC3_INFO:
        retval = _MDrv_MAD_Audio_AC3_Info(arg);
        break;
    case MDrv_MAD_AUDIO_HEAAC_INFO:
        retval = _MDrv_MAD_Audio_HEAAC_Info(arg);
        break;
    case MDrv_MAD_FILE_GET_FRAME_NUM:
        retval = _MDrv_MAD_FileGetFrameNum(arg);
        break;
    case MDrv_MAD_PCM_START_UPLOAD:
        retval = _MDrv_MAD_PCMStartUpload(arg);
        break;
    case MDrv_MAD_PCM_STOP_UPLOAD:
        retval = _MDrv_MAD_PCMStopUpload();
        break;
    case MDrv_MAD_PCM_START_DOWNLOAD:
        retval = _MDrv_MAD_PCMStartDownload(arg);
        break;
    case MDrv_MAD_PCM_STOP_DOWNLOAD:
        retval = _MDrv_MAD_PCMStopDownload();
        break;
    case MDrv_MAD_READTIMESTAMP:
        retval = _MDrv_MAD_ReadTimeStamp(arg);
        break;
    case MDrv_MAD_READTIMESTAMP_SE:
        retval = _MDrv_MAD_ReadTimeStamp_SE(arg);
        break;
    case MDrv_MAD_HDMI_DOLBYMONITOR:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_HDMI_DOLBYMONITOR\n"));
        retval = _MDrv_MAD_HDMI_DolbyMonitor(arg);
        break;
    case MDrv_MAD_DTV_HDMI_CFG_:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_DTV_HDMI_CFG_\n"));
        retval = _MDrv_MAD_DTV_HDMI_CFG(arg);
        break;
    case MDrv_MAD_AC3DEC_DIS:
        retval = _MDrv_MAD_AC3Dec_DIS(arg);
        break;
    case MDrv_MAD_WMA_GETSAMPLERATE:
        retval = _MDrv_MAD_WMA_GetSampleRate(arg);
        break;
    case MDrv_MAD_WMA_GETBITRATE:
        retval = _MDrv_MAD_WMA_GetBitRate(arg);
        break;
    case MDrv_MAD_AAC_GETSAMPLERATE:
        retval = _MDrv_MAD_aac_getSampleRate(arg);
        break;
    case MDrv_MAD_GET_DSP_CODETYPE:
        retval = _MDrv_MAD_GetDSPCodeType(arg);
        break;
    case MDrv_MAD_GET_DSP2_CODETYPE:
        retval = _MDrv_MAD_GetDSP2CodeType(arg);
        break;
    case MDrv_MAD_DVB_XPCM_SETPARAM:
        retval = _MDrv_MAD_Dvb_XPCM_setParam(arg);
        break;
    case MDrv_MAD_DVB2_XPCM_SETPARAM:
        retval = _MDrv_MAD_Dvb2_XPCM_setParam(arg);
        break;
    case MDrv_MAD_DVB_RA8_SETPARAM:
        retval = _MDrv_MAD_Dvb_RA8_setParam(arg);
        break;
 //DEC DSP register
    case MDrv_MAD_RD_DEC_REG:
        retval = _MDrv_MAD_DecReadREG(arg);
        break;
    case MDrv_MAD_RD_DEC_REGBYTE:
   	 retval = _MDrv_MAD_DecReadREGByte(arg);
        break;
    case MDrv_MAD_WR_DEC_REG:
        retval = _MDrv_MAD_DecWriteREG(arg);
        break;
    case MDrv_MAD_WR_DEC_REGBYTE:
	 retval = _MDrv_MAD_DecWriteREGByte(arg);
	 break;
    case MDrv_MAD_WR_DEC_MASK_REG:
	 retval = _MDrv_MAD_DecWriteREGMask(arg);
	 break;
    case MDrv_MAD_WR_DEC_MASK_REGBYTE:
        retval = _MDrv_MAD_DecWriteREGMaskByte(arg);
        break;
    case MDrv_MAD_WR_DEC_MASK_INTBYTE:
        retval = _MDrv_MAD_DecWriteIntMaskByte(arg);
        break;
//SE DSP register
    case MDrv_MAD_RD_SE_REG:
        retval = _MDrv_MAD_SeReadREG(arg);
        break;
    case MDrv_MAD_RD_SE_REGBYTE:
   	 retval = _MDrv_MAD_SeReadREGByte(arg);
	 break;
    case MDrv_MAD_WR_SE_REG:
        retval = _MDrv_MAD_SeWriteREG(arg);
        break;
   case MDrv_MAD_WR_SE_REGBYTE:
        retval = _MDrv_MAD_SeWriteREGByte(arg);
        break;
    case MDrv_MAD_WR_SE_MASK_REG:
        retval = _MDrv_MAD_SeWriteREGMask(arg);
        break;
    case MDrv_MAD_WR_SE_MASK_REGBYTE:
        retval = _MDrv_MAD_SeWriteREGMaskByte(arg);
        break;
    case MDrv_MAD_SET_AD_MIXMODE:
        retval = _MDrv_MAD_SetADMixMode(arg);
        break;
    case MDrv_MAD_SET_AD_OUTPUTMODE:
        retval = _MDrv_MAD_SetADOutputMode(arg);
        break;
    case MDrv_MAD_SET_AD_VOLUME:
        retval = _MDrv_MAD_ProcessSetADAbsoluteVolume(arg);
        break;
    case MDrv_MAD_SET_AD_MUTE:
        retval = _MDrv_MAD_ProcessADSetMute(arg);
        break;
    case MDrv_MAD_GET_MAD_BASE:
        retval = _MDrv_MAD_GetMADBase(arg);
        break;
    case MDrv_MAD_SET_CV3_PARAM:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_SET_CV3_PARAM\n"));
        retval = _MDrv_MAD_SetClearVoiceIII(arg);
        break;
    case MDrv_MAD_CV3_SET_CV_ENABLE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_CV_ENABLE\n"));
        retval = _MDrv_MAD_SetClearVoiceOnOff(arg);
        break;
    case MDrv_MAD_CV3_SET_AVL2_ENABLE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_AVL2_ENABLE\n"));
        retval = _MDrv_MAD_CV3_SetAVL2OnOff(arg);
        break;
    case MDrv_MAD_CV3_SET_SURROUND_ENABLE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_SURROUND_ENABLE\n"));
        retval = _MDrv_MAD_CV3_SetRurroundOnOff(arg);
        break;
    case MDrv_MAD_CV3_SET_UEQ_ENABLE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_UEQ_ENABLE\n"));
        retval = _MDrv_MAD_CV3_SetUEQOnOff(arg);
        break;

    case MDrv_MAD_CV3_SET_VOLUME:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_VOLUME\n"));
        retval = _MDrv_MAD_CV3_SetVolume(arg);
        break;
    case MDrv_MAD_CV3_SET_BASS_ENHANCE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_BASS_ENHANCE\n"));
        retval = _MDrv_MAD_CV3_SetBassEnhance(arg);
        break;		
    case MDrv_MAD_CV3_SET_UEQ:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_UEQ\n"));
        retval = _MDrv_MAD_CV3_SetUEQ(arg);
        break;
    case MDrv_MAD_CV3_SET_AVL2:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_AVL2\n"));
        retval = _MDrv_MAD_CV3_SetAVL2(arg);
        break;
    case MDrv_MAD_CV3_SET_CV:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_CV\n"));
        retval = _MDrv_MAD_CV3_SetCV(arg);
        break;
    case MDrv_MAD_CV3_SET_SURROUND:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_SURROUND\n"));
        retval = _MDrv_MAD_CV3_SetSurround(arg);
        break;
    case MDrv_MAD_CV3_SET_MODE:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_SET_MODE\n"));
        retval = _MDrv_MAD_CV3_SetMode(arg);
        break;
    case MDrv_MAD_CV3_PVC_MONITOR:
        MAD_DEBUG_DDI2(printk("[IOCTL]:MDrv_MAD_CV3_GET_PVC_MONITOR\n"));
        retval = _MDrv_MAD_CV3_PVC_Monitor(arg);
	 break;

    case MDrv_MAD_POWER_ON_MELODY:
        MDrv_MAD_PowerOn_Melody();
        break;
    case MDrv_MAD_MPEG_SET_FILE_SIZE:
        retval = _MDrv_MAD_MPEG_SetFileSize(arg);
        break;
    case MDrv_MAD_POWER_ON_MELODY_PATH:
        retval = _MDrv_MAD_PowerOn_Melody_Path(arg);
        break;
    case MDrv_MAD_DSP_ALIVE_CHECK:
    case MDrv_MAD_ADEC_ALIVE_CHECK:
        retval = _MDrv_MAD_DSP_Alive_Check(arg);
        break;
    case MDrv_MAD_ASND_ALIVE_CHECK:
        retval = _MDrv_MAD_DSP2_Alive_Check(arg);
        break;
    case MDrv_MAD_ADEC_ES_CHECK:
        retval = _MDrv_MAD_ADEC_ES_Check(arg);
        break;
    case MDrv_MAD_SET_DOLBY_DRC_MODE:
        retval = _MDrv_MAD_Set_Dolby_DRC_Mode(arg);
        break;
    case MDrv_MAD_SET_DOLBY_AD_DRC_MODE:
        retval = _MDrv_MAD_Set_Dolby_AD_DRC_Mode(arg);
        break;
    case MDrv_MAD_SET_DOLBY_DOWNMIX_MODE:
        retval = _MDrv_MAD_Set_Dolby_Downmix_Mode(arg);
        break;
    case MDrv_MAD_SET_SRS_DC:
         retval = _MDrv_MAD_Set_SRS_DC(arg);
        break;
    case MDrv_MAD_SET_SRS_TRU_BASS:
       retval =  _MDrv_MAD_Set_SRS_TruBass(arg);
        break;
    case MDrv_MAD_SET_SRS_PARA:
        retval = _MDrv_MAD_SetSRSPara(arg);
        break;
#if (!USE_PARAM_POLLING)
    case MDrv_MAD_SET_SRS_PARA32:
	retval = _MDrv_MAD_SetSRSPara32(arg);
        break;
#endif
    case MDrv_MAD_AUPROCESS_WRITE_PARAMETER:
        retval = _MDrv_MAD_AuProcessWritePARAMETER(arg);
        break;
    case MDrv_MAD_CHECK_AUDIO_ES:
	retval = _MDrv_MAD_CheckAudioES(arg);
	break;
    case MDrv_MAD_AUDIO_MONITOR:
	retval = _MDrv_MAD_Audio_Monitor();
	break;
    case MDrv_MAD_GET_DEC_DSP_REVIVE_COUNT:
	retval = _MDrv_MAD_GetDecDspReviveCount(arg);
	break;
    case MDrv_MAD_MPEG_GET_SOUND_MODE:
        retval = _MDrv_MAD_mpeg_GetSoundMode(arg);
        break;
    case MDrv_MAD_MPEG_SET_SOUND_MODE:
        retval = _MDrv_MAD_mpeg_SetSoundMode(arg);
        break;
    case MDrv_MAD_DVB2_PLAY_BT:
        retval = _MDrv_MAD_Dvb2_Play_BT(arg);
        break;
    case MDrv_MAD_DVB_PCM_LEVEL_CONTROL:
        retval = _MDrv_MAD_DVB_PcmLevelControl(arg);
        break;
    case MDrv_MAD_SET_WMA_PARAM:
        retval = _MDrv_MAD_WMA_SetASFParm(arg);
        break;
    case MDrv_MAD_SET_COMMAND:
        retval = _MDrv_MAD_SetCommand(arg);
        break;
//Mstar add for Skype, 2009/09/22
    case MDrv_MAD_INIT_SKYPE:
        retval = _MDrv_MAD_Init_Skype(arg);
        break;
    case MDrv_MAD_SKYPE_DN_PCM:
        retval = _MDrv_MAD_Skype_Dn_PCM();
        break;
    case MDrv_MAD_STOP_SKYPE:
        retval = _MDrv_MAD_Stop_Skype();
        break;
    case MDrv_MAD_SIF_ADC_RESET:
	 retval = _MDrv_SIF_ADC_Reset();
	 break;
    case MDrv_MAD_SET_CUT_BOOST:
	 retval = _MDrv_MAD_Set_Cut_Boost(arg);
	 break;
    case MDrv_MAD_GET_DDP_AD_STATUS:
        retval = _MDrv_MAD_Get_DDP_AD_Status(arg);
        break;
    case MDrv_MAD_SET_DDP_AD_MODE:
        retval = _MDrv_MAD_Set_DDP_AD_Mode(arg);
        break;
    case MDrv_MAD_GET_PTS:
        retval = _MDrv_MAD_Get_PTS(arg);
        break;			
    case MDrv_MAD_SET_ADC_THRESHOLD:
        retval = _MDrv_MAD_Set_ADC_Threshold(arg);
        break;
    case MDrv_MAD_GET_PCM_ENERGY:
        retval = _MDrv_MAD_Get_PCM_Energy(arg);
        break;	
    case MDrv_MAD_RESTORE_VOL:
        retval = _MDrv_MAD_Vol_Reg_Restore();
        break;	
    default:
        MAD_WARNING("ioctl: unknown command\n");
        PROBE_IO_EXIT(MDRV_MAJOR_MAD, _IOC_NR(cmd));
        return -ENOTTY;
    }

    PROBE_IO_EXIT(MDRV_MAJOR_MAD, _IOC_NR(cmd));
    return retval;

}

static int __init _mod_mad_init(void)
{
    int         s32Ret;
    dev_t       dev;

    MAD_PRINT("%s is invoked\n", __FUNCTION__);

    if (MadDev.s32MadMajor)
    {
        dev=                    MKDEV(MadDev.s32MadMajor, MadDev.s32MadMinor);
        s32Ret=                 register_chrdev_region(dev, MOD_MAD_DEVICE_COUNT, MOD_MAD_NAME);
    }
    else
    {
        s32Ret=                 alloc_chrdev_region(&dev, MadDev.s32MadMinor, MOD_MAD_DEVICE_COUNT, MOD_MAD_NAME);
        MadDev.s32MadMajor=     MAJOR(dev);
    }
    if (0> s32Ret)
    {
        MAD_WARNING("Unable to get major %d\n", MadDev.s32MadMajor);
        return s32Ret;
    }

    cdev_init(&MadDev.cDevice, &MadDev.MadFop);
    if (0!= (s32Ret= cdev_add(&MadDev.cDevice, dev, MOD_MAD_DEVICE_COUNT)))
    {
        MAD_WARNING("Unable add a character device\n");
        unregister_chrdev_region(dev, MOD_MAD_DEVICE_COUNT);
        return s32Ret;
    }
    spin_lock_init(&_mad_spinlock);
    init_waitqueue_head(&_mad_wait_queue);
    return 0;
}

static void __exit _mod_mad_exit(void)
{
    MAD_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&MadDev.cDevice);
    unregister_chrdev_region(MKDEV(MadDev.s32MadMajor, MadDev.s32MadMinor), MOD_MAD_DEVICE_COUNT);
}

module_init(_mod_mad_init);
module_exit(_mod_mad_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MAD driver");
MODULE_LICENSE("MSTAR");

