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

#ifndef _DEV_MVD_H_
#define _DEV_MVD_H_

#include "mvd4_interface.h" //firmware header
#include "halVPU.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//#define MVD_ENABLE_ISR    //unmark this to enable MVD ISR

#ifndef __MDRV_TYPES_H__
#define BIT0    BIT(0)
#define BIT1    BIT(1)
#define BIT2    BIT(2)
#define BIT3    BIT(3)
#define BIT4    BIT(4)
#define BIT5    BIT(5)
#define BIT6    BIT(6)
#define BIT7    BIT(7)
#endif

#define H_DWORD(x)            (MS_U8)(((x)>>24)&0xff)
#define L_DWORD(x)            (MS_U8)(((x)>>16)&0xff)
#define H_WORD(x)             (MS_U8)(((x)>>8 )&0xff)
#define L_WORD(x)             (MS_U8)((x)&0xff)
#define COMBM16(hi,lo)        ((((MS_U16)(hi))<<8) | ((MS_U16)(lo)))
#define COMBU32(hh,hl,lh,ll)  ((((MS_U32)(hh))<<24) | (((MS_U32)(hl))<<16) | (((MS_U32)(lh))<<8) | ((MS_U32)(ll)))
#define MemAlign(n, unit)     ( (((n)+(unit)-1)/(unit)) * (unit) )

//Init command arguments
#define SETUP_CMDARG(x)     \
            do {            \
                x.Arg0 = 0; \
                x.Arg1 = 0; \
                x.Arg2 = 0; \
                x.Arg3 = 0; \
                x.Arg4 = 0; \
                x.Arg5 = 0; \
               } while(0)

//Set command arguments
#define SET_CMDARG(cmd, u32val)             \
            do {                            \
                cmd.Arg0 = L_WORD(u32val);  \
                cmd.Arg1 = H_WORD(u32val);  \
                cmd.Arg2 = L_DWORD(u32val); \
                cmd.Arg3 = H_DWORD(u32val); \
                cmd.Arg4 = 0;               \
                cmd.Arg5 = 0;               \
               } while(0)

void HAL_MVD_RegSetBase(MS_U32 u32Base);
MS_U8 HAL_MVD_RegReadByte(MS_U32 u32Reg);
void HAL_MVD_RegWriteByte(MS_U32 u32Reg, MS_U8 u8Val);
void HAL_MVD_RegWriteBit(MS_U32 u32Reg, MS_BOOL bEnable, MS_U8 u8Mask);
void HAL_MVD_RegWriteByteMask(MS_U32 u32Reg, MS_U8 u8Val, MS_U8 u8Msk);
void HAL_MVD_RegWrite4Byte(MS_U32 u32Reg, MS_U32 u32Val);

// Mutex function for HI interface
#define HAL_MVD_LockHiIfMutex()
#define HAL_MVD_UnlockHiIfMutex()

#define MVD_SUPPORT_MPEG2 0x01
#define MVD_SUPPORT_MPEG4 0x02
#define MVD_SUPPORT_VC1   0x04

#define MVD_FW_VERSION   FW_VERSION
#define MVD_FW_CODE_LEN                    (0x40000UL)  //256K
#define MVD_SYNC_DONE    1

#if defined(CHIP_T3)
#define MVD_MIU1_BASE_ADDRESS   0x08000000UL//0x10000000UL
#else
#define MVD_MIU1_BASE_ADDRESS   0x08000000UL
#endif

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MVD_MMAP_FW  = 0, //firmware
    E_MVD_MMAP_BS  = 1, //bitstream buffer
    E_MVD_MMAP_FB  = 2, //framebuffer
    E_MVD_MMAP_ALL = 3,
    E_MVD_MMAP_DRV = 4  //driver processing buffer
} MVD_MMAP_Type;


typedef struct _MVD_DrvCfg
{
    MS_U32 u32fmVerNum;//firmware version number
    MS_U16 u16fmSrcID; //firmware bin source id
    MS_U8  u8fbMode;   //framebuffer mode: HD or SD
} MVD_DrvCfg;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
MS_U8 HAL_MVD_GetCaps(void);
void HAL_MVD_MemSetMap(MS_U8 u8type, MS_U32 u32addr, MS_U32 u32len);
void HAL_MVD_MemGetMap(MS_U8 u8type, MS_U32* pu32addr, MS_U32* pu32len);
MS_U32 HAL_MVD_MemRead4Byte(MS_U32 u32Address);
MS_U16 HAL_MVD_MemRead2Byte(MS_U32 u32Address);
MS_U8 HAL_MVD_MemReadByte(MS_U32 u32Address);
MS_BOOL HAL_MVD_MemWrite4Byte(MS_U32 u32Address, MS_U32 u32Value);
MS_BOOL HAL_MVD_MemWrite2Byte(MS_U32 u32Address, MS_U16 u16Value);
MS_BOOL HAL_MVD_MemWriteByte(MS_U32 u32Address, MS_U8 u8Value);
void HAL_MVD_SetReqMask(MS_BOOL bEnMask);

MS_PHYADDR  HAL_MVD_PA2NonCacheSeg(MS_PHYADDR u32PhyAddr);
MS_U32      HAL_MVD_GetTime(void);
void        HAL_MVD_Delayms(MS_U32 u32msecs);
void        HAL_MVD_CPU_Sync(void);
void        HAL_MVD_FlushMemory(void);
void        HAL_MVD_ReadMemory(void);

MS_BOOL HAL_MVD_RstHW(void);
MS_BOOL HAL_MVD_ReleaseFW(void);
MS_BOOL HAL_MVD_LoadCode(void);
void HAL_MVD_SetFWBinID(MS_U32 u32SrcID);

MS_U8 HAL_MVD_GetChipMajorID(void);

void HAL_MVD_PowerCtrl(MS_BOOL bOn);
MS_BOOL HAL_MVD_MVDCommand(MS_U8 u8cmd, MVD_CmdArg *pstCmdArg);
MS_BOOL HAL_MVD_InitHW(void);
MS_BOOL HAL_MVD_InitFW(void);
MS_U32 HAL_MVD_GetFWVer(void);
MS_BOOL HAL_MVD_SoftRstHW(void);
void HAL_MVD_ClearIRQ(void);

MS_BOOL HAL_MVD_SetSpeed(MVD_SpeedType eSpeedType, MS_U8 u8Multiple);
MS_BOOL HAL_MVD_EnableForcePlay(void);
void HAL_MVD_SetFrameBuffAddr(MS_U32 u32addr, MS_U8 u8fbMode);
void HAL_MVD_SetHeaderBufferAddr(MS_U32 u32addr);
void HAL_MVD_SetVolInfoBufferAddr(MS_U32 u32addr);
void HAL_MVD_SetFrameInfoBufferAddr(MS_U32 u32addr);
void HAL_MVD_SetIAPBufferAddr(MS_U32 u32addr);
void HAL_MVD_SetDPBufferAddr(MS_U32 u32addr);
void HAL_MVD_SetMVBufferAddr(MS_U32 u32addr);
void HAL_MVD_SetUserDataBuf(MS_U32 u32addr, MS_U32 u32size);
void HAL_MVD_SetSLQTblBufStartEnd(MS_U32 u32start, MS_U32 u32end);
MS_BOOL HAL_MVD_StepDisp(void);
MS_BOOL HAL_MVD_EnableLastFrameShow(MS_BOOL bEnable);
MS_BOOL HAL_MVD_SlqTblRst(void);
MS_BOOL HAL_MVD_SeekToPTS(MS_U32 u32Pts);
MS_BOOL HAL_MVD_SkipToPTS(MS_U32 u32Pts);
MS_BOOL HAL_MVD_SetFileModeAVSync(MVD_TIMESTAMP_TYPE eSyncMode);
MS_BOOL HAL_MVD_SetPtsTblAddr(MS_U32 u32addr);
MS_BOOL HAL_MVD_SetScalerInfoAddr(MS_U32 u32addr);
MS_BOOL HAL_MVD_SetDynamicScaleAddr(MS_U32 u32addr);
MS_BOOL HAL_MVD_SetVirtualBox(MS_U16 u16Width, MS_U16 u16Height);
MS_BOOL HAL_MVD_EnableQDMA(void);
MS_BOOL HAL_MVD_SetBlueScreen(MS_BOOL bEn);
MS_BOOL HAL_MVD_Resume(void);
MS_BOOL HAL_MVD_SetFreezeDisp(MS_BOOL bEn);
void HAL_MVD_SetDecFrmInfoAddr(MS_U32 u32addr);

void HAL_MVD_SetDbgLevel(MS_U8 level);

#endif // _DEV_MVD_H_
