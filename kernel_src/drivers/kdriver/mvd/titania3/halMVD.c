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


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#ifdef REDLION_LINUX_KERNEL_ENVI
#include "mdrv_types.h"
#include "mdrv_system.h"

#include "drvMVD_Common.h"
#include "drvMVD.h"

#include "regMVD.h"
#include "halMVD.h"

#include "halVPU.h"

static MS_U8 u8MVD_FW_Binary[] =
{
    #include "fwMVD.dat"
};

#define MVD_ENABLE_BDMA_FW_FLASH_2_SDRAM     0

#else
#include "MsCommon.h"
#include "drvMVD.h"
#include "drvMMIO.h"

// Internal Definition
#include "regMVD.h"
#include "halMVD.h"
#include "halVPU.h"
#include "osalMVD.h"
#include "mvd4_interface.h" //firmware header
#include "asmCPU.h"

#if defined(FW_EXTERNAL_BIN)
    #include "drvSERFLASH.h"

    #define MVD_ENABLE_BDMA_FW_FLASH_2_SDRAM     1
#else   // defined(FW_EMBEDDED_ASC)
    #define MVD_ENABLE_BDMA_FW_FLASH_2_SDRAM     0
#endif
#if (!MVD_ENABLE_BDMA_FW_FLASH_2_SDRAM)
static MS_U8 u8MVD_FW_Binary[] =
{
    #include "fwMVD.dat"
};
#endif
#endif

extern MVD_MEMCfg stMemCfg;


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MIU1_BASEADDR       stMemCfg.u32Miu1BaseAddr
#define _PA2Offset(x)       (((x)>=MIU1_BASEADDR)?(x-MIU1_BASEADDR):(x))

#ifndef MS_ASSERT
#define MS_ASSERT(expr)     do {                                                        \
                                if(!(expr))                                             \
                                printf("MVD assert fail %s %d!\n", __FILE__, __LINE__); \
                            } while(0)
#endif

//constant
#define MVD_PollingTimes      0x40000UL

#define MVD_DEBUG_FWCMD(x)    if (_u8HalDbgLevel>2)  { (x); }
#define MVD_DEBUGINFO(x)      if (_u8HalDbgLevel>1)  { (x); }
#define MVD_DEBUGERROR(x)     if (_u8HalDbgLevel>0)  { (x); }
#define MVD_ERROR(x)          x
#if 0
#define SETUP_CMDARG(x)     \
            do {            \
                x.Arg0 = 0; \
                x.Arg1 = 0; \
                x.Arg2 = 0; \
                x.Arg3 = 0; \
                x.Arg4 = 0; \
                x.Arg5 = 0; \
               } while(0)
#endif
#define RIU     ((unsigned short volatile *) u32RiuBaseAdd)
#define RIU8    ((unsigned char  volatile *) u32RiuBaseAdd)

#define MVDCPU_ON_MIU1     ((HAL_MVD_RegReadByte(MIU0_SEL0_L) & BIT7) == BIT7)
#define MVDHW_ON_MIU1     ((HAL_MVD_RegReadByte(MIU0_SEL3_L) & BIT4) == BIT4)


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
static MS_U32 u32fmAddr, u32fmLen;
static MS_U32 u32bsAddr, u32bsLen;
static MS_U32 u32fbAddr, u32fbLen;
static MS_U32 u32DrvProcAddr, u32DrvProcLen;
static MS_U32 u32FirmwareSrcID;
static MS_BOOL _bMvdInit = FALSE;
static MS_U32 u32RiuBaseAdd;
static MS_U8 _u8HalDbgLevel = 0;


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void HAL_MVD_SetDbgLevel(MS_U8 level)
{
    _u8HalDbgLevel = level;
    return;
}


MS_U8 HAL_MVD_GetCaps(void)
{
    MS_U8 caps = 0;

    caps |= (MVD_SUPPORT_MPEG2|MVD_SUPPORT_MPEG4|MVD_SUPPORT_VC1);
    return caps;
}


void HAL_MVD_MemSetMap(MS_U8 u8type, MS_U32 u32addr, MS_U32 u32len)
{
    //printf("miuBase=0x%lx\n", stMemCfg.u32Miu1BaseAddr);
    switch (u8type)
    {
        case E_MVD_MMAP_ALL:
        case E_MVD_MMAP_FW:
            u32fmAddr = _PA2Offset(u32addr);
            u32fmLen = u32len;
            break;
        case E_MVD_MMAP_BS:
            u32bsAddr = _PA2Offset(u32addr);
            u32bsLen = u32len;
            break;
        case E_MVD_MMAP_FB:
            u32fbAddr = _PA2Offset(u32addr);
            u32fbLen = u32len;
            break;

        case E_MVD_MMAP_DRV:
            u32DrvProcAddr = _PA2Offset(u32addr);
            u32DrvProcLen = u32len;
            break;

        default:
            break;
    }

    MVD_DEBUGINFO(printf("HAL_MVD_MemSetMap[%d] add=0x%lx offset=0x%lx len=0x%lx\n",u8type,u32addr,_PA2Offset(u32addr),u32len));
    return;
}

void HAL_MVD_MemGetMap(MS_U8 u8type, MS_U32* pu32addr, MS_U32* pu32len)
{
    switch (u8type)
    {
        case E_MVD_MMAP_ALL:
        case E_MVD_MMAP_FW:
            *pu32addr = u32fmAddr;
            *pu32len = u32fmLen;
            break;
        case E_MVD_MMAP_BS:
            *pu32addr = u32bsAddr;
            *pu32len = u32bsLen;
            break;
        case E_MVD_MMAP_FB:
            *pu32addr = u32fbAddr;
            *pu32len = u32fbLen;
            break;
        case E_MVD_MMAP_DRV:
            *pu32addr = u32DrvProcAddr;
            *pu32len = u32DrvProcLen;
            break;
        default:
            break;
    }

    MVD_DEBUGINFO(printf("HAL_MVD_MemGetMap[%d] add=0x%lx len=0x%lx\n",u8type,*pu32addr,*pu32len));
    return;
}


void HAL_MVD_RegSetBase(MS_U32 u32Base)
{
    u32RiuBaseAdd = u32Base;
    HAL_VPU_InitRegBase(u32Base);
}

void HAL_MVD_RegWriteByte(MS_U32 u32Reg, MS_U8 u8Val)
{
    if ( __builtin_constant_p( u32Reg ) )
    {
        RIU8[((u32Reg) * 2) - ((u32Reg) & 1)] = u8Val;
    }
    else
    {
        RIU8[(u32Reg << 1) - (u32Reg & 1)] = u8Val;
    }
}

MS_U8 HAL_MVD_RegReadByte(MS_U32 u32Reg)
{
    return (__builtin_constant_p( u32Reg ) ?
            (((u32Reg) & 0x01) ? RIU8[(u32Reg) * 2 - 1] : RIU8[(u32Reg) * 2]) :
            (RIU8[(u32Reg << 1) - (u32Reg & 1)]));
}

void HAL_MVD_RegWriteBit(MS_U32 u32Reg, MS_BOOL bEnable, MS_U8 u8Mask)
{
    MS_U32 u32Reg8 = ((u32Reg) * 2) - ((u32Reg) & 1);
    RIU8[u32Reg8] = (bEnable) ? (RIU8[u32Reg8] |  (u8Mask)) :
                                (RIU8[u32Reg8] & ~(u8Mask));
}

void HAL_MVD_RegWriteByteMask(MS_U32 u32Reg, MS_U8 u8Val, MS_U8 u8Msk)
{
    MS_U32 u32Reg8 = ((u32Reg) * 2) - ((u32Reg) & 1);
    RIU8[u32Reg8] = (RIU8[u32Reg8] & ~(u8Msk)) | ((u8Val) & (u8Msk));
}

void HAL_MVD_RegWrite4Byte(MS_U32 u32Reg, MS_U32 u32Val)
{
    if ( __builtin_constant_p( u32Reg ) && !((u32Reg) & 0x01) )
    {
        RIU[u32Reg] = (MS_U16)(u32Val);
        RIU[(u32Reg) + 2] = (MS_U16)((u32Val) >> 16);
    }
    else
    {
        if (u32Reg & 0x01)
        {
            RIU8[(u32Reg << 1) - 1] = u32Val;
            RIU[u32Reg + 1] = (u32Val >> 8);
            RIU8[((u32Reg + 3) << 1)] = (u32Val >> 24);
        }
        else
        {
            RIU[u32Reg] = u32Val;
            RIU[u32Reg + 2] = (u32Val >> 16);
        }
    }
}


MS_U32 HAL_MVD_MemRead4Byte(MS_U32 u32Address)
{
    volatile MS_U32 u32Val;
    MS_ASSERT(!(u32Address & 0x03UL));

    HAL_MVD_CPU_Sync();
    HAL_MVD_ReadMemory();

    if (stMemCfg.bFWMiuSel == MIU_SEL_1)
    {
        u32Address += MIU1_BASEADDR;
    }

    u32Val = *(volatile MS_U32*) HAL_MVD_PA2NonCacheSeg(u32Address);

    //printf("mvd rd 0x%lx = 0x%lx\n", u32Address, u32Val);
    return u32Val;
}

MS_U16 HAL_MVD_MemRead2Byte(MS_U32 u32Address)
{
    MS_U32 u32ReadAddr;
    MS_U32 u32ReadValue;
    MS_U16 u16Value;
    MS_U8 u8Shift;
    u32ReadAddr = (u32Address >> 2) << 2;
    u8Shift = (MS_U8)((u32Address & 0x03) * 8);
    u32ReadValue = HAL_MVD_MemRead4Byte(u32ReadAddr);

    u16Value = (MS_U16)(u32ReadValue >> u8Shift);
    if(u8Shift == 24)
    {
        u32ReadValue = HAL_MVD_MemRead4Byte(u32ReadAddr+4);
        u16Value = u16Value << 8 || (MS_U16)(u32ReadValue & 0xFF);
    }
    return u16Value;
}

MS_U8 HAL_MVD_MemReadByte(MS_U32 u32Address)
{
    MS_U32 u32ReadAddr;
    MS_U32 u32ReadValue;
    MS_U8 u8Value;
    MS_U8 u8Shift;
    u32ReadAddr = (u32Address >> 2) << 2;
    u8Shift = (MS_U8)((u32Address & 0x03) * 8);
    u32ReadValue = HAL_MVD_MemRead4Byte(u32ReadAddr);
    u8Value = (MS_U8)(u32ReadValue >> u8Shift);

    return u8Value;
}

MS_BOOL HAL_MVD_MemWrite4Byte(MS_U32 u32Address, MS_U32 u32Value)
{
    if (stMemCfg.bFWMiuSel == MIU_SEL_1)
    {
        u32Address += MIU1_BASEADDR;
    }

    *(volatile MS_U32 *) HAL_MVD_PA2NonCacheSeg(u32Address) = u32Value;

    HAL_MVD_CPU_Sync();
    HAL_MVD_FlushMemory();

    return TRUE;
}

MS_BOOL HAL_MVD_MemWriteByte(MS_U32 u32Address, MS_U8 u8Value)
{
    MS_U32 u32ReadAddr;
    MS_U32 u32ReadValue;
    MS_U8 u8Shift;

    u32ReadAddr = (u32Address >> 2) << 2;
    u8Shift = (MS_U8)((u32Address & 0x03UL) * 8);
    u32ReadValue = HAL_MVD_MemRead4Byte(u32ReadAddr);
    u32ReadValue &= ~(0xFFUL << u8Shift);
    u32ReadValue |= ((MS_U32)u8Value << u8Shift);
    HAL_MVD_MemWrite4Byte(u32ReadAddr, u32ReadValue);
    return TRUE;
}

MS_BOOL HAL_MVD_MemWrite2Byte(MS_U32 u32Address, MS_U16 u16Value)
{
    MS_U32 u32ReadAddr;
    MS_U32 u32ReadValue;
    MS_U8 u8Shift;

    u8Shift = (MS_U8)((u32Address & 0x03UL) * 8) ;
    if(u8Shift < 24)
    {
        u32ReadAddr = (u32Address >> 2) << 2;
        u32ReadValue = HAL_MVD_MemRead4Byte(u32ReadAddr);
        u32ReadValue &= ~(0xFFFF << u8Shift);
        u32ReadValue |= ((MS_U32)u16Value << u8Shift);
        HAL_MVD_MemWrite4Byte(u32ReadAddr, u32ReadValue);
    }
    else
    {
        HAL_MVD_MemWriteByte(u32Address, (MS_U8)(u16Value));
        HAL_MVD_MemWriteByte(u32Address+1, (MS_U8)(u16Value >> 8));
    }
    return TRUE;
}


void HAL_MVD_SetReqMask(MS_BOOL bEnMask)
{
    HAL_VPU_MIU_RW_Protect(bEnMask);

    if (MVDHW_ON_MIU1)
    {
        HAL_MVD_RegWriteBit(MIU1_RQ3_MASK_L, bEnMask, BIT4);  //MVD R/W
        HAL_MVD_RegWriteBit(MIU1_RQ3_MASK_L, bEnMask, BIT5);  //MVD bbu R/W
    }
    else
    {
        HAL_MVD_RegWriteBit(MIU0_RQ3_MASK_L, bEnMask, BIT4);  //MVD R/W
        HAL_MVD_RegWriteBit(MIU0_RQ3_MASK_H, bEnMask, BIT5);  //MVD bbu R/W
    }
    HAL_MVD_Delayms(1);

    return;
}


//------------------------------------------------------------------------------
/// Initialize MVD
/// @return -result of resetting MVD hardware
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_RstHW(void)
{
#ifndef REDLION_LINUX_KERNEL_ENVI
    MS_U32 u32Time = 0;
    OSAL_MVD_LockHwMutex();
#endif

    HAL_MVD_SetReqMask(ENABLE);

    HAL_MVD_RegWriteBit(MVD_CTRL, 1, MVD_CTRL_DISCONNECT_MIU);//disconnect MIU
    HAL_MVD_RegWriteBit(MVD_CTRL, 0, MVD_CTRL_DISCONNECT_MIU);//release reset

    HAL_MVD_RegWriteBit(MVD_CTRL, 1, MVD_CTRL_RST);//reset MVD
    HAL_MVD_RegWriteBit(MVD_CTRL, 0, MVD_CTRL_RST);//release reset

#ifdef REDLION_LINUX_KERNEL_ENVI
    while ((HAL_MVD_RegReadByte(MVD_STATUS) & MVD_STATUS_READY) == 0) ;
#else
    u32Time = HAL_MVD_GetTime();
    while ( ((HAL_MVD_RegReadByte(MVD_STATUS) & MVD_STATUS_READY) == 0)
            && ((HAL_MVD_GetTime() - u32Time) < 200) );
#endif

    HAL_MVD_SetReqMask(DISABLE);

#ifndef REDLION_LINUX_KERNEL_ENVI
    OSAL_MVD_UnlockHwMutex();
#endif

    return TRUE;
}


//------------------------------------------------------------------------------
/// Release CPU
/// @return -release CPU successfully or not
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_ReleaseFW(void)
{
    HAL_VPU_SwRstRelse();
    return TRUE;
}

void HAL_MVD_SetFWBinID(MS_U32 u32SrcID)
{
    u32FirmwareSrcID = u32SrcID;
    return;
}


//------------------------------------------------------------------------------
/// config VD_AEON CPU
/// @param u32StAddr \b IN: CPU binary code base address in DRAM.
/// @param u8dlend_en \b IN: endian
///     - 1, little endian
///     - 0, big endian
//------------------------------------------------------------------------------
static void HAL_MVD_CPUSetting(MS_U32 u32StAddr)
{
    if (!HAL_VPU_CPUSetting(u32StAddr))
    {
        MVD_ERROR(printf("HAL_MVD_CPUSetting fail!\n"));
    }
    return;
}

//------------------------------------------------------------------------------
/// H.264 SW/CPU reset
/// @return TRUE or FALSE
///     - TRUE, Success
///     - FALSE, Failed
//------------------------------------------------------------------------------
static MS_BOOL _HAL_MVD_SwCPURst(void)
{
    HAL_VPU_SwRst();
#if defined(HAL_MVD_FPAG)
    // set request mask
    HAL_MVD_SetReqMask(ENABLE);

    HAL_MVD_RegWriteBit(REG_VD_MHEG5_RESET, 1, REG_VD_MHEG_RESET_CPURST);
    HAL_MVD_Delayms(100);

    HAL_MVD_SetReqMask(DISABLE);
#endif
    return TRUE;
}

MS_PHYADDR HAL_MVD_PA2NonCacheSeg(MS_PHYADDR u32PhyAddr)
{
#ifdef REDLION_LINUX_KERNEL_ENVI
    return (MS_PHYADDR) MDrv_SYS_PA2NonCacheSeg((void *) u32PhyAddr);
#else
    return MS_PA2KSEG1(u32PhyAddr);
#endif
}

MS_U32 HAL_MVD_GetTime(void)
{
#ifdef REDLION_LINUX_KERNEL_ENVI
    return MDrv_SYS_GetSyetemTime();
#else
    return MsOS_GetSystemTime();
#endif
}

void HAL_MVD_Delayms(MS_U32 u32msecs)
{
#ifdef REDLION_LINUX_KERNEL_ENVI
    msleep(u32msecs);
#else
    MsOS_DelayTask(u32msecs);
#endif
}

void HAL_MVD_CPU_Sync(void)
{
#ifdef REDLION_LINUX_KERNEL_ENVI
    MDrv_SYS_CPU_Sync();
#else
    MAsm_CPU_Sync();
#endif
}

void HAL_MVD_FlushMemory(void)
{
#ifdef REDLION_LINUX_KERNEL_ENVI
    MDrv_SYS_Flush_Memory();
#else
    MsOS_FlushMemory();
#endif
}

void HAL_MVD_ReadMemory(void)
{
#ifdef REDLION_LINUX_KERNEL_ENVI
    MDrv_SYS_Read_Memory();
#else
    MsOS_ReadMemory();
#endif
}

//------------------------------------------------------------------------------
/// Load MVD firmware code
/// @return -Load firmware success or not
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_LoadCode( void )
{
    MS_BOOL bRst = TRUE;
    MS_U32 u32fwAddr = 0;
    MS_U32 u32fwPA = 0;
    MS_U32 u32fwLen = 0;
    MS_U8* pu8fw;
    MS_U32 i;
    MS_U32 u32Size;

    //set CPU clk
    VPU_Init_Params VPUInitParams = {E_VPU_CLOCK_144MHZ, FALSE, -1, VPU_DEFAULT_MUTEX_TIMEOUT};
    HAL_VPU_Init(&VPUInitParams);

    HAL_MVD_MemGetMap(E_MVD_MMAP_FW, &u32fwAddr, &u32fwLen);

    // reset
    if(!_HAL_MVD_SwCPURst())
    {
        (printf("AVCH264 reset failed...\n"));
        return FALSE;
    }

    // load code
    //MS_ASSERT((MVDCPU_ON_MIU1 == TRUE) && (stMemCfg.bFWMiuSel == MIU_SEL_1));
    //MS_ASSERT((MVDCPU_ON_MIU1 == FALSE) && (stMemCfg.bFWMiuSel == MIU_SEL_0));
    if (stMemCfg.bFWMiuSel == MIU_SEL_1)
    {
        u32fwPA = u32fwAddr + MIU1_BASEADDR;
    }
    else
    {
        u32fwPA = u32fwAddr;
    }
    MVD_DEBUGINFO(printf("HAL_MVD_LoadCode u32fwAddr=%lx, miuSel=%x, miu1Base=%lx\n",u32fwAddr,stMemCfg.bFWMiuSel,MIU1_BASEADDR));

    pu8fw = (MS_U8 *) HAL_MVD_PA2NonCacheSeg(u32fwPA);
    MVD_DEBUGINFO(printf("HAL_MVD_LoadCode u32fwPA = %lx==>%lx\n", u32fwPA, HAL_MVD_PA2NonCacheSeg(u32fwPA)));

#if MVD_ENABLE_BDMA_FW_FLASH_2_SDRAM
    if (stMemCfg.u32FWBinSize != 0)
    {
        if (stMemCfg.eFWSrcType == E_MVD_FW_SOURCE_DRAM)
        {
            u32Size = stMemCfg.u32FWBinSize;
            if ((u32Size>0) && (stMemCfg.u32FWSrcVAddr != NULL))
            {
                printf("%s(%d) size=0x%lx, va=0x%lx\n", __FUNCTION__, __LINE__, u32Size, stMemCfg.u32FWSrcVAddr);
                for (i = 0; i < u32Size; i++)
                {
                    pu8fw[i] = *(MS_U8*)(stMemCfg.u32FWSrcVAddr+i);
                }
            }
            else
            {
                printf("%s err: binSize=0x%lx, SrcVA=0x%lx", __FUNCTION__,
                        stMemCfg.u32FWBinSize, stMemCfg.u32FWSrcVAddr);
            }
        }
        else //assume that either from DRAM or FLASH
        {
            SPIDMA_Dev eDmaDst = E_SPIDMA_DEV_MIU1;
            if (stMemCfg.bFWMiuSel == MIU_SEL_1)
            {
                eDmaDst = E_SPIDMA_DEV_MIU1;
            }
            else
            {
                eDmaDst = E_SPIDMA_DEV_MIU0;
            }

            printf("%s src=0x%lx dst=0x%lx size=0x%lx eDmaDst=0x%x\n", __FUNCTION__,
                    stMemCfg.u32FWBinAddr, u32fwPA, stMemCfg.u32FWBinSize, eDmaDst);

            HAL_MVD_CPU_Sync();

            if (!MDrv_SERFLASH_CopyHnd(stMemCfg.u32FWBinAddr, u32fwPA, stMemCfg.u32FWBinSize, eDmaDst, SPIDMA_OPCFG_DEF))
            {
                printf("%s MDrv_SERFLASH_CopyHnd fail!\n", __FUNCTION__);
                return FALSE;
            }
        }
    }
    else
    {
        printf("%s err: the source size of FW is zero\n", __FUNCTION__);
        return FALSE;
    }
#else
    {
        if (stMemCfg.eFWSrcType == E_MVD_FW_SOURCE_DRAM)
        {
            u32Size = stMemCfg.u32FWBinSize;
            if ((u32Size>0) && (stMemCfg.u32FWSrcVAddr != NULL))
            {
                printf("%s(%d) size=0x%lx, va=0x%lx\n", __FUNCTION__, __LINE__, u32Size, stMemCfg.u32FWSrcVAddr);
                for (i = 0; i < u32Size; i++)
                {
                    pu8fw[i] = *(MS_U8*)(stMemCfg.u32FWSrcVAddr+i);
                }
            }
            else
            {
                printf("%s err: binSize=0x%lx, SrcVA=0x%lx", __FUNCTION__,
                        stMemCfg.u32FWBinSize, stMemCfg.u32FWSrcVAddr);
            }
        }
        else
        {
            u32Size = sizeof(u8MVD_FW_Binary);

            for (i = 0; i < u32Size; i++)
            {
                pu8fw[i] = u8MVD_FW_Binary[i];
            }
        }
    }
#endif
    HAL_MVD_CPU_Sync();
    HAL_MVD_FlushMemory();

    // setting cpu base and endian
    HAL_MVD_CPUSetting(u32fwAddr);

    return bRst;
}


MS_U8 HAL_MVD_GetChipMajorID(void)
{
    return (HAL_MVD_RegReadByte(REG_CHIP_ID_MAJOR));
}


#ifdef MVD_ENABLE_WRITEPROTECT
//------------------------------------------------------------------------------
/// Enable write limitation for MVD3 hardware.
/// Set start/end address of the memory boundary; Enable the write protection.
//------------------------------------------------------------------------------
void HAL_MVD_EnableWriteProtect(void)
{
    return;
}
#endif


void HAL_MVD_PowerCtrl(MS_BOOL bOn)
{
#ifdef REDLION_LINUX_KERNEL_ENVI
    HAL_MVD_RegWriteByteMask(REG_CKG_MVD, CKG_MVD_144MHZ, CKG_MVD_MASK);
    HAL_MVD_RegWriteBit(REG_CKG_MVD, !bOn, CKG_MVD_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD, !bOn, CKG_MVD_INVERT);

    if (1 == Chip_Query_Rev()) // U01
    {
        HAL_MVD_RegWriteByteMask(REG_CKG_MVD2, CKG_MVD2_144MHZ, CKG_MVD2_MASK);
    }
    else // U02 or after
    {
        HAL_MVD_RegWriteByteMask(REG_CKG_MVD2, CKG_MVD2_170MHZ, CKG_MVD2_MASK);
    }
#else
    MS_U32 u32PMBankSize;
    MS_U32 u32PMBankAddr = 0;
    #define __CHIP_VERSION 0x1ecf


    HAL_MVD_RegWriteByteMask(REG_CKG_MVD, CKG_MVD_144MHZ, CKG_MVD_MASK);
    HAL_MVD_RegWriteBit(REG_CKG_MVD, !bOn, CKG_MVD_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD, !bOn, CKG_MVD_INVERT);

    if (!MDrv_MMIO_GetBASE(&u32PMBankAddr, &u32PMBankSize, MS_MODULE_PM))
    {
        printf("HAL_MVD_PowerCtrl: IOMap failure\n");
        return;
    }
    //printf("u32PMBankAddr=0x%lx\n", u32PMBankAddr);

    if (READ_BYTE(u32PMBankAddr+(__CHIP_VERSION<<1)-(__CHIP_VERSION&1)) == 0x00)
    {
        //printf("T3 u01\n");
        HAL_MVD_RegWriteByteMask(REG_CKG_MVD2, CKG_MVD2_144MHZ, CKG_MVD2_MASK);
    }
    else //after t3_u02
    {
        //printf("T3 u02\n");
        HAL_MVD_RegWriteByteMask(REG_CKG_MVD2, CKG_MVD2_170MHZ, CKG_MVD2_MASK);
    }
#endif

    HAL_MVD_RegWriteBit(REG_CKG_MVD2, !bOn, CKG_MVD2_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD2, !bOn, CKG_MVD2_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_CHROMA, !bOn, CKG_MVD_CHROMA_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_CHROMA, !bOn, CKG_MVD_CHROMA_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_LUMA_A, !bOn, CKG_MVD_LUMA_A_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_LUMA_A, !bOn, CKG_MVD_LUMA_A_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_LUMA_B, !bOn, CKG_MVD_LUMA_B_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_LUMA_B, !bOn, CKG_MVD_LUMA_B_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_LUMA_C, !bOn, CKG_MVD_LUMA_C_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_LUMA_C, !bOn, CKG_MVD_LUMA_C_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_RMEM, !bOn, CKG_MVD_RMEM_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_RMEM, !bOn, CKG_MVD_RMEM_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_RMEM1, !bOn, CKG_MVD_RMEM1_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_RMEM1, !bOn, CKG_MVD_RMEM1_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_INTPRAM0, !bOn, CKG_MVD_INTPRAM0_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_INTPRAM0, !bOn, CKG_MVD_INTPRAM0_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_INTPRAM1, !bOn, CKG_MVD_INTPRAM1_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_INTPRAM1, !bOn, CKG_MVD_INTPRAM1_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_RREFDAT, !bOn, CKG_MVD_RREFDAT_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_RREFDAT, !bOn, CKG_MVD_RREFDAT_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_WREFDAT, !bOn, CKG_MVD_WREFDAT_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_WREFDAT, !bOn, CKG_MVD_WREFDAT_INVERT);

    HAL_MVD_RegWriteBit(REG_CKG_MVD_DPFF, !bOn, CKG_MVD_DPFF_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_MVD_DPFF, !bOn, CKG_MVD_DPFF_INVERT);

    //Set MVD all clock sources equal to clk_miu_p   0: enable  1: disable
    HAL_MVD_RegWriteBit(REG_CKG_MVD_SYNC, !bOn, CKG_MVD_SYNC_GATED);

#if 0
    //Set
    HAL_MVD_RegWriteByteMask(REG_CKG_VD_AEON, CKG_VD_AEON_144MHZ, CKG_VD_AEON_MASK);
    HAL_MVD_RegWriteBit(REG_CKG_VD_AEON, !bOn, CKG_VD_AEON_GATED);
    HAL_MVD_RegWriteBit(REG_CKG_VD_AEON, !bOn, CKG_VD_AEON_INVERT);
#endif

    return;
}

//------------------------------------------------------------------------------
/// Wait MVD command ready or timeout
/// @return -MVD command ready or timeout
//------------------------------------------------------------------------------
static MS_BOOL _MVD_TimeOut (void)
{
    MS_U32 i;

    for ( i = 0; i < MVD_PollingTimes; i++ )
    {
        ///- wait until MVD command ready or timeout
        if ( ( HAL_MVD_RegReadByte(MVD_STATUS) & MVD_STATUS_READY ) == MVD_STATUS_READY )
        {
            return FALSE;
        }
    }
    MVD_DEBUGERROR( printf("MVD_TimeOut=%lx\n", i) );
    return TRUE;
}

//------------------------------------------------------------------------------
/// Set MVD firmware command
/// @param -u8cmd \b IN : MVD command
/// @param -pstCmdArg \b IN : pointer to command argument
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_MVDCommand ( MS_U8 u8cmd, MVD_CmdArg *pstCmdArg )
{
    if ( _MVD_TimeOut() == TRUE )
        return FALSE;

    if ((pstCmdArg->Arg4!=0)||(pstCmdArg->Arg5!=0))
    {
        MVD_DEBUGERROR(printf("Warning: cmd%x; arg4=%x, arg5=%x\n", u8cmd,
        pstCmdArg->Arg4, pstCmdArg->Arg5));
    }

    HAL_MVD_RegWriteByte(MVD_ARG0, pstCmdArg->Arg0);
    HAL_MVD_RegWriteByte(MVD_ARG1, pstCmdArg->Arg1);
    HAL_MVD_RegWriteByte(MVD_ARG2, pstCmdArg->Arg2);
    HAL_MVD_RegWriteByte(MVD_ARG3, pstCmdArg->Arg3);
    HAL_MVD_RegWriteByte(MVD_ARG4, pstCmdArg->Arg4);
    HAL_MVD_RegWriteByte(MVD_ARG5, pstCmdArg->Arg5);
    HAL_MVD_RegWriteByte(MVD_COMMAND, u8cmd);
//    if ((CMD_GET_AFD != u8cmd) && (CMD_SLQ_UPDATE_TBL_WPTR != u8cmd) &&
//       (CMD_SLQ_GET_TBL_RPTR != u8cmd) && (CMD_RD_PTS != u8cmd))
    {
        MVD_DEBUG_FWCMD(printf("MVD_CMD: %x; %x, %x, %x, %x, %x, %x\n", u8cmd, pstCmdArg->Arg0,
        pstCmdArg->Arg1, pstCmdArg->Arg2, pstCmdArg->Arg3, pstCmdArg->Arg4, pstCmdArg->Arg5));
    }

    if ( _MVD_TimeOut() == TRUE )
        return FALSE;

    pstCmdArg->Arg0 = HAL_MVD_RegReadByte(MVD_ARG0);
    pstCmdArg->Arg1 = HAL_MVD_RegReadByte(MVD_ARG1);
    pstCmdArg->Arg2 = HAL_MVD_RegReadByte(MVD_ARG2);
    pstCmdArg->Arg3 = HAL_MVD_RegReadByte(MVD_ARG3);
    pstCmdArg->Arg4 = HAL_MVD_RegReadByte(MVD_ARG4);
    pstCmdArg->Arg5 = HAL_MVD_RegReadByte(MVD_ARG5);
    return TRUE;
}

void HAL_MVD_SetSyncClk(MS_BOOL bEnable)
{
    MS_ASSERT(0==bEnable);//Notice: Euclid & T3 have no sync_mode; Bit4 must be 0.
    HAL_MVD_RegWriteBit(MVD_CTRL, bEnable, MVD_CTRL_CLK_SYNCMODE);

    return;
}

MS_BOOL HAL_MVD_InitHW(void)
{
    //Set MVD Clock @ reg_CHIPTOP
    HAL_MVD_PowerCtrl(ENABLE);

    //Set MVD Clock aync
    HAL_MVD_SetSyncClk(DISABLE);

    //MVD reset
    if(!HAL_MVD_RstHW())
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_MVDInit:MVD4ResetHW failed\n"));
        return FALSE;
    }
    else
    {
        MVD_DEBUGINFO(printf("MDrv_MVD_MVDInit:MVD4ResetHW success\n"));
    }

#ifdef MVD_ENABLE_WRITEPROTECT
    HAL_MVD_EnableWriteProtect();
#endif

    //release FW
    if(!HAL_MVD_ReleaseFW())
    {
        MVD_DEBUGERROR(printf("MDrv_MVD_MVDInit:MVD4ReleaseFW failed\n"));
        return FALSE;
    }
    else
    {
        MVD_DEBUGINFO(printf("MDrv_MVD_MVDInit:MVD4ReleaseFW success\n"));
    }

    _bMvdInit = TRUE;
    return TRUE;
}


//------------------------------------------------------------------------------
/// Get MVD firmware version
/// @return -firmware version
//------------------------------------------------------------------------------
MS_U32 HAL_MVD_GetFWVer(void)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_GET_FW_VERSION, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_GET_FW_VERSION ) );
        return 0;
    }

    return COMBU32(mvdcmd.Arg3,mvdcmd.Arg2,mvdcmd.Arg1,mvdcmd.Arg0);
}

//------------------------------------------------------------------------------
/// Check MVD firmware status
/// @return -firmware is ready or not
//------------------------------------------------------------------------------
static MS_BOOL _MVD_Check_FW_Rdy(void)
{
    MS_U32 u32TimeOut = 2000;

    //check firmware version consistent with header file
    while ((FW_VERSION != HAL_MVD_GetFWVer()) && (--u32TimeOut));

    if (u32TimeOut == 0)
    {
        MVD_ERROR(printf("MVD_FW_Version=%lx inconsistent with header file!\n", HAL_MVD_GetFWVer()));
        return FALSE;
    }
    MVD_DEBUGINFO(printf("MVD FW version in header file = %x\n", FW_VERSION));

    return TRUE;
}

MS_BOOL HAL_MVD_InitFW(void)
{
    MS_U32 i=0;
    MS_U32 u32Addr = 0;
    MS_U32 u32Len = 0;
    MVD_CmdArg mvdcmd;

    //check FW ready
    if ( !_MVD_Check_FW_Rdy())
    {
        MS_ASSERT(0);
        return FALSE;
    }

    //set code offset to MVD
    HAL_MVD_MemGetMap(E_MVD_MMAP_FW, &u32Addr, &u32Len);
    SETUP_CMDARG(mvdcmd);
    i = u32Addr >> 3;
    mvdcmd.Arg0 = (i & 0xFF);
    mvdcmd.Arg1 = ((i >> 8) & 0xFF);
    mvdcmd.Arg2 = ((i >> 16) & 0xFF);
    mvdcmd.Arg3 = 0;
    if (HAL_MVD_MVDCommand( CMD_CODE_OFFSET, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_CODE_OFFSET ) );
        return FALSE;
    }

    return TRUE;
}

//utility: get Program Counter
void HAL_MVD_CPUGetPC(void)
{
    MS_U32 u32pc = 0;
    u32pc = HAL_VPU_GetProgCnt();
    printf("VD_MHEG5 32-bit PC = 0x%lx\n", u32pc);

    return;
}

#define _MVD_CMDRDY ((HAL_MVD_RegReadByte(MVD_STATUS) & MVD_STATUS_READY) == MVD_STATUS_READY)
//------------------------------------------------------------------------------
/// Check if MVD command is ready
/// @return TRUE or FALSE
///     - TRUE, Success to process the command
///     - FALSE, Failed due to timeout
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_GetCmdRdy(void)
{
    MS_U32 timeoutTick = 2000;

    while ((!_MVD_CMDRDY) && ((timeoutTick--)!=0));

    if (0 == timeoutTick)
        return FALSE;
    else
        return TRUE;
}

MS_BOOL HAL_MVD_CheckIdle(void)
{
    MS_BOOL bIsIdle = FALSE;
    MVD_CmdArg mvdcmd;

    //issue CheckIdle command
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_MVD_IDLE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_MVD_IDLE ) );
        return FALSE;
    }

    bIsIdle = (mvdcmd.Arg0 == 1);
    if (HAL_MVD_GetCmdRdy())
    {
        return bIsIdle;
    }
    else
    {
        return FALSE;
    }
}


static MS_BOOL _MVD_SoftRstHW(void)
{
#if 0
    MVD_CmdArg mvdcmd;

    //ENG reset
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_SW_RESET, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SW_RESET ) );
        return FALSE;
    }

    //SLQ reset
    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_VC1_HW_SLQ_RESET, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_VC1_HW_SLQ_RESET ) );
        return FALSE;
    }

    return TRUE;
#else
    return FALSE;
#endif
}

//------------------------------------------------------------------------------
/// Soft-reset MVD
/// Ref AP note p.12 HK2MVD Reset Flow
/// @return TRUE or FALSE
///     - TRUE, Success to soft-reset MVD
///     - FALSE, Failed. Need init MVD again.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_SoftRstHW(void)
{
    MS_U32 timetick = 2000;

    if (HAL_MVD_GetCmdRdy())
    {
        //idle check
        while ((!HAL_MVD_CheckIdle()) && ((timetick--)!=0));

        //either MVD is idle or timeout, do ENG/SLQ reset
        return _MVD_SoftRstHW(); //Reset HW engine
    }
    else
    {
        return FALSE; //here means "CPU hanging"
    }
}

//------------------------------------------------------------------------------
/// Clean the IRQ bit (in interrupt handler should call this function while the
/// interrupt has been triggered.
/// @param none
/// @return none
/// @internal
//------------------------------------------------------------------------------
void HAL_MVD_ClearIRQ(void)
{
    HAL_MVD_RegWriteBit(MVD_CTRL, 1, MVD_CTRL_CLR_INT);

    return;
}

//------------------------------------------------------------------------------
/// Set display speed.
///FW use (# of B frames) / (# of decode frames) < Ratio this formula to adjustment.
///Once if the ratio is 1, that means, whenever (#Bframes / #decoded) < 1, then
///FW would drop the B frame.
///In other words, once AP need to back to normal mode, AP have to set the arg0 to 0.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_SetSpeed(MVD_SpeedType eSpeedType, MS_U8 u8Multiple)
{
    MS_BOOL bRst = TRUE;
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);

    if (E_MVD_SPEED_FAST == eSpeedType)
        mvdcmd.Arg0 = 1; //fast forward
    else if (E_MVD_SPEED_SLOW == eSpeedType)
        mvdcmd.Arg0 = 2; //slow motion
    else
        mvdcmd.Arg0 = 0; //normal speed

    if (u8Multiple == 1)
    {
        mvdcmd.Arg0 = 0;
        //The only way to be NORMAL speed.
    }

    mvdcmd.Arg1 = u8Multiple;
    if (HAL_MVD_MVDCommand( CMD_DISP_SPEED_CTRL, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DISP_SPEED_CTRL ) );
        bRst = FALSE;
    }

    if (bRst==TRUE)
    {
        bRst=HAL_MVD_Resume();
    }
    return bRst;
}


MS_BOOL HAL_MVD_EnableForcePlay(void)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand(CMD_PLAY_NO_SQE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_PLAY_NO_SQE) );
        return FALSE;
    }

    if (HAL_MVD_Resume() == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: HAL_MVD_Resume fail!!\r\n" ) );
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
/// Set frame buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void HAL_MVD_SetFrameBuffAddr(MS_U32 u32addr, MS_U8 u8fbMode)
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32addr);
    mvdcmd.Arg1 = H_WORD(u32addr);
    mvdcmd.Arg2 = L_DWORD(u32addr);

    //Frame Buffer Mode Setting
    mvdcmd.Arg3 = 0x10; //fixme: set to HD mode
    MVD_DEBUGINFO(printf ("Frame buffer address %lx, FB Mode %d\n", u32addr, mvdcmd.Arg3));

    if (mvdcmd.Arg3 == MVD3_FILE_SD_MODE || mvdcmd.Arg3 == MVD3_SD_MODE)
    {
        u32addr <<= 3;
        if (u32addr+0x30C000 > 0x2000000)
        {
            printf( "MVD3 Frame Buffer for SD mode must be within 32MB, now it's 0x%lX\n", u32addr);
            return;
        }
    }

    if (HAL_MVD_MVDCommand( CMD_FB_BASE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_FB_BASE ) );
        return;
    }

    return;
}

//------------------------------------------------------------------------------
/// Set header buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void HAL_MVD_SetHeaderBufferAddr ( MS_U32 u32addr )
{
    MVD_CmdArg mvdcmd;

    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SET_CMDARG(mvdcmd, u32addr);
    if (HAL_MVD_MVDCommand( CMD_HEADER_INFO_BUF, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_HEADER_INFO_BUF ) );
    }
    return;
}

//------------------------------------------------------------------------------
/// Set vol info buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void HAL_MVD_SetVolInfoBufferAddr ( MS_U32 u32addr )
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SET_CMDARG(mvdcmd, u32addr);
    if (HAL_MVD_MVDCommand( CMD_VOL_INFO_BUF, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_VOL_INFO_BUF ) );
    }
    return;
}

//------------------------------------------------------------------------------
/// Set frame info buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void HAL_MVD_SetFrameInfoBufferAddr ( MS_U32 u32addr )
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SET_CMDARG(mvdcmd, u32addr);
    if (HAL_MVD_MVDCommand( CMD_FRAME_INFO_BUF, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_FRAME_INFO_BUF ) );
    }
    return;
}

//------------------------------------------------------------------------------
/// Set IAP buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void HAL_MVD_SetIAPBufferAddr ( MS_U32 u32addr )
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32addr%8192)==0);
    u32addr >>= 13;

    SET_CMDARG(mvdcmd, u32addr);
    if (HAL_MVD_MVDCommand( CMD_IAP_BUF_START, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_IAP_BUF_START ) );
    }
    return;
}

//------------------------------------------------------------------------------
/// Set Data Partition buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void HAL_MVD_SetDPBufferAddr ( MS_U32 u32addr )
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SET_CMDARG(mvdcmd, u32addr);
    if (HAL_MVD_MVDCommand( CMD_DP_BUF_START, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DP_BUF_START ) );
    }
    return;
}

//------------------------------------------------------------------------------
/// Set Motion Vector buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void HAL_MVD_SetMVBufferAddr ( MS_U32 u32addr )
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32addr%2048)==0);
    u32addr >>= 3;

    SET_CMDARG(mvdcmd, u32addr);
    if (HAL_MVD_MVDCommand( CMD_MV_BUF_START, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_MV_BUF_START ) );
    }
    return;
}

//------------------------------------------------------------------------------
/// Set user data buffer address to MVD
/// @param -u32addr \b IN : start address
//------------------------------------------------------------------------------
void HAL_MVD_SetUserDataBuf(MS_U32 u32addr, MS_U32 u32size)
{
    MVD_CmdArg mvdcmd;

    MS_ASSERT((u32addr%8)==0);
    MS_ASSERT((u32size%8)==0);
    u32addr >>= 3;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32addr);
    mvdcmd.Arg1 = H_WORD(u32addr);
    mvdcmd.Arg2 = L_DWORD(u32addr);
    mvdcmd.Arg4 = H_DWORD(u32addr);
#ifdef REDLION_LINUX_KERNEL_ENVI
    mvdcmd.Arg3 = 2;
#else
    mvdcmd.Arg3 = 0;
#endif

    if (HAL_MVD_MVDCommand( CMD_USER_BUF_START, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_USER_BUF_START ) );
        return;
    }

    u32size >>= 3;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32size);
    mvdcmd.Arg1 = H_WORD(u32size);
    mvdcmd.Arg2 = L_DWORD(u32size);
    mvdcmd.Arg4 = H_DWORD(u32size);
#ifdef REDLION_LINUX_KERNEL_ENVI
    mvdcmd.Arg3 = 2;
#else
    mvdcmd.Arg3 = 0;
#endif

    if (HAL_MVD_MVDCommand( CMD_USER_BUF_SIZE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_USER_BUF_SIZE ) );
    }

    return;
}

void HAL_MVD_SetSLQTblBufStartEnd(MS_U32 u32start, MS_U32 u32end)
{
    MVD_CmdArg mvdcmd;
    MS_U32 u32val = u32end>>3;

    SET_CMDARG(mvdcmd, u32val);
    if (HAL_MVD_MVDCommand( CMD_SLQ_TBL_BUF_END, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SLQ_TBL_BUF_END ) );
        return;
    }

    u32val = (u32start)>>3;
    SET_CMDARG(mvdcmd, u32val);
    if (HAL_MVD_MVDCommand( CMD_SLQ_TBL_BUF_START, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SLQ_TBL_BUF_START ) );
        return;
    }

    MVD_DEBUGINFO(printf("%s st=0x%lx end=0x%lx OK!!!\n", __FUNCTION__, u32start, u32end));
    return;
}

//------------------------------------------------------------------------------
/// Issue StepDisplay command.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_StepDisp(void)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_STEP_DISP_DECODE_ONE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_STEP_DISP_DECODE_ONE ) );
        return FALSE;
    }
    if (HAL_MVD_Resume() == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: HAL_MVD_Resume fail!!\r\n" ) );
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
/// Enable/Disable firmware to show the last frame.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_EnableLastFrameShow(MS_BOOL bEnable)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = bEnable;
    if (HAL_MVD_MVDCommand(CMD_ENABLE_LAST_FRAME_SHOW, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR(printf("Command: 0x%x fail!!\r\n", CMD_ENABLE_LAST_FRAME_SHOW));
        return FALSE;
    }
    return TRUE;
}


MS_BOOL HAL_MVD_SlqTblRst(void)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand( CMD_VC1_HW_SLQ_RESET, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_VC1_HW_SLQ_RESET ) );
        return FALSE;
    }

    return TRUE;
}

MS_BOOL HAL_MVD_SeekToPTS(MS_U32 u32Pts)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32Pts);
    mvdcmd.Arg1 = H_WORD(u32Pts);
    mvdcmd.Arg2 = L_DWORD(u32Pts);
    mvdcmd.Arg3 = H_DWORD(u32Pts);
    if (HAL_MVD_MVDCommand(CMD_STEP_TO_PTS, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_STEP_TO_PTS) );
        return FALSE;
    }

    if (HAL_MVD_Resume() == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: HAL_MVD_Resume fail!!\r\n" ) );
        return FALSE;
    }
    return TRUE;
}

MS_BOOL HAL_MVD_SkipToPTS(MS_U32 u32Pts)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32Pts);
    mvdcmd.Arg1 = H_WORD(u32Pts);
    mvdcmd.Arg2 = L_DWORD(u32Pts);
    mvdcmd.Arg3 = H_DWORD(u32Pts);
    if (HAL_MVD_MVDCommand(CMD_SKIP_TO_PTS, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SKIP_TO_PTS) );
        return FALSE;
    }

    if (HAL_MVD_Resume() == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: HAL_MVD_Resume fail!!\r\n" ) );
        return FALSE;
    }
    return TRUE;
}

MS_BOOL HAL_MVD_SetFileModeAVSync(MVD_TIMESTAMP_TYPE eSyncMode)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    switch (eSyncMode)
    {
        case E_MVD_TIMESTAMP_PTS:
            mvdcmd.Arg0 = FILE_PTS_MODE;
            break;

        case E_MVD_TIMESTAMP_DTS:
            mvdcmd.Arg0 = FILE_DTS_MODE;
            break;

        case E_MVD_TIMESTAMP_FREERUN:
        default:
            mvdcmd.Arg0 = NONE_FILE_MODE; //Freerun
            break;
    }
    if (HAL_MVD_MVDCommand( CMD_ENABLE_FILE_SYNC, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_ENABLE_FILE_SYNC ) );
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
/// Set the start address of PTS table used for SLQ table link mode.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_SetPtsTblAddr(MS_U32 u32addr)
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32addr);
    mvdcmd.Arg1 = H_WORD(u32addr);
    mvdcmd.Arg2 = L_DWORD(u32addr);
    mvdcmd.Arg3 = 0;
    if (HAL_MVD_MVDCommand( CMD_PTS_TBL_START, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_PTS_TBL_START ) );
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
/// Issue Pause command.
/// @return -TRUE for success; FALSE for failure
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_Resume(void)
{
    MS_BOOL bRst = TRUE;
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = DISPLAY_PAUSE_OFF;
    if (HAL_MVD_MVDCommand(CMD_DISPLAY_PAUSE, &mvdcmd)== FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DISPLAY_PAUSE) );
        bRst = FALSE;
    }
    return bRst;
}

//------------------------------------------------------------------------------
/// Set base address for ScalerInfo structure to f/w
/// @param -u32addr \b IN : start address (units in byte)
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_SetScalerInfoAddr(MS_U32 u32addr)
{
    MVD_CmdArg mvdcmd;

    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32addr);
    mvdcmd.Arg1 = H_WORD(u32addr);
    mvdcmd.Arg2 = L_DWORD(u32addr);
    mvdcmd.Arg3 = H_DWORD(u32addr);
    if (HAL_MVD_MVDCommand(CMD_SCALER_INFO_BASE, &mvdcmd) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_SCALER_INFO_BASE ) );
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
/// Set the dynamic scale base address
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_SetDynamicScaleAddr(MS_U32 u32addr)
{
    MVD_CmdArg mvdcmd;
    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32addr);
    mvdcmd.Arg1 = H_WORD(u32addr);
    mvdcmd.Arg2 = L_DWORD(u32addr);
    mvdcmd.Arg3 = 0;
    if (HAL_MVD_MVDCommand(CMD_DYNAMIC_SCALE_BASE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DYNAMIC_SCALE_BASE ) );
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
/// Set virtual box width/height to F/W.
/// F/W will use the same w/h as scaler to calculate scaling factor.
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_SetVirtualBox(MS_U16 u16Width, MS_U16 u16Height)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u16Width);
    mvdcmd.Arg1 = H_WORD(u16Width);
    mvdcmd.Arg2 = L_WORD(u16Height);
    mvdcmd.Arg3 = H_WORD(u16Height);
    if (HAL_MVD_MVDCommand( CMD_DS_VIRTUAL_BOX, &mvdcmd ) == FALSE)   //CMD_DS_VIRTUAL_BOX
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DS_VIRTUAL_BOX ) );
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
/// Enable VD_MHEG5 QDMA
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_EnableQDMA(void)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    if (HAL_MVD_MVDCommand(CMD_ENABLE_DYNAMIC_SCALE, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_ENABLE_DYNAMIC_SCALE ) );
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
/// Set blue screen
/// @return -TRUE for success; FALSE for failure.
//------------------------------------------------------------------------------
MS_BOOL HAL_MVD_SetBlueScreen(MS_BOOL bEn)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = bEn; //1 -> show MVOP frame color.  0 -> normal case.
    if (HAL_MVD_MVDCommand(CMD_FORCE_BLUE_SCREEN, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_FORCE_BLUE_SCREEN ) );
        return FALSE;
    }
    return TRUE;
}


MS_BOOL HAL_MVD_SetFreezeDisp(MS_BOOL bEn)
{
    MVD_CmdArg mvdcmd;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = bEn;
    if (HAL_MVD_MVDCommand(CMD_FREEZE_DISP, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_FREEZE_DISP ) );
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
/// Set base address for DecFrameInfo structure to f/w
/// @param -u32addr \b IN : start address (units in byte)
//------------------------------------------------------------------------------
void HAL_MVD_SetDecFrmInfoAddr ( MS_U32 u32addr )
{
    MVD_CmdArg mvdcmd;

    MS_ASSERT((u32addr%8)==0);
    u32addr >>= 3;

    SETUP_CMDARG(mvdcmd);
    mvdcmd.Arg0 = L_WORD(u32addr);
    mvdcmd.Arg1 = H_WORD(u32addr);
    mvdcmd.Arg2 = L_DWORD(u32addr);
    mvdcmd.Arg3 = H_DWORD(u32addr);
    if (HAL_MVD_MVDCommand( CMD_DEC_FRAME_INFO_BUF, &mvdcmd ) == FALSE)
    {
        MVD_DEBUGERROR( printf( "Command: 0x%x fail!!\r\n", CMD_DEC_FRAME_INFO_BUF ) );
    }
    return;
}


