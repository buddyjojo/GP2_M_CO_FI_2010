////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///
/// @file    madp_mpif.h
/// @brief  MPIF Control Interface
/// @author MStar Semiconductor Inc.
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Include standard library
//------------------------------------------------------------------------------
//#include <unistd.h>
//#include <fcntl.h>
//#include <signal.h>
//#include <stropts.h>
//#include <sys/ioctl.h>
//#include <string.h>
//#include <stdio.h>
//------------------------------------------------------------------------------
// Include Files
//------------------------------------------------------------------------------
#include "mdrv_types.h"
#include "mdrv_mpif_st.h"
#include "mdrv_mpif_io_uboot.h"

//-------------------------------------------------------------------------------------------------
// Macro
//-------------------------------------------------------------------------------------------------
#define OPT_MPIF_MADP_DBG    0

#if (OPT_MPIF_MADP_DBG)
#define MPIF_PRINT(fmt, args...)      printf("[MAdp_MPIF][%05d] " fmt, __LINE__, ## args)
#else
#define MPIF_PRINT(fmt, args...)
#endif

//------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
int MPIF_fd = 0;

//------------------------------------------------------------------------------
// Function
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Init MPIF
/// @return None
//------------------------------------------------------------------------------
void MAdp_MPIF_Open(void)
{
/*
    if (MPIF_fd==0)
    {
        MPIF_fd = open(MPIF_MODULE_KERNAL_NAME, O_RDWR);
        MPIF_PRINT("open MPIF Kernal finish\n");
    }
    else
    {
        MPIF_PRINT("Fail to open MPIF Kernal Module\n");
    }*/
}

void MAdp_MPIF_Init(U8 u8clk, U8 u8trc, U8 u8wc)
{
    MPIF_INIT_PARAM param;
    param.u8clk = u8clk;
    param.u8trc = u8trc;
    param.u8wc = u8wc;
    
    ioctl(MPIF_fd, IOCTL_MPIF_INIT, &param);
}

void MAdp_MPIF_InitSPIF(U8 u8slaveid)
{
    ioctl(MPIF_fd, IOCTL_MPIF_INIT_SPIF, &u8slaveid);
}

void MAdp_MPIF_SetCmdDataWidth(U8 u8slaveid, U8 u8cmdwidth, U8 u8datawidth)
{
    MPIF_BUS_PARAM param;
    param.u8slaveid = u8slaveid;
    param.u8cmdwidth = u8cmdwidth;
    param.u8datawidth = u8datawidth;
    ioctl(MPIF_fd, IOCTL_MPIF_SET_CMDDATA_WIDTH, &param);
}

BOOL MAdp_MPIF_1A(U8 u8bWite, U8 slaveid, U8 index, U8* pu8data)
{
    MPIF_PARAM param;
    param.u8bWite = u8bWite;
    param.slaveid = slaveid;
    param.addr = index;
    param.data = *pu8data;
    ioctl(MPIF_fd, IOCTL_MPIF_1A, &param);
    *pu8data = param.data;
    return param.bRet;
}

BOOL MAdp_MPIF_2A(U8 u8bWite, U8 slaveid, U16 addr, U16 *pu16data)
{
    MPIF_PARAM param;
    param.u8bWite = u8bWite;
    param.slaveid = slaveid;
    param.addr = addr;
    param.data = *pu16data;
    ioctl(MPIF_fd, IOCTL_MPIF_2A, &param);
    *pu16data = param.data;
    return param.bRet;
}

//--------- general register read/write function ------------

U8 MAdp_MPIF_ReadByte(U8 u8slaveid, U16 u16Addr)
{
    U16 u16data;
    MAdp_MPIF_2A(0, u8slaveid, (u16Addr & ~0x1), &u16data);
    return (u16Addr & 0x1) ? ((U8)(u16data>>8)) : ((U8)u16data);
}

U16 MAdp_MPIF_Read2Byte(U8 u8slaveid, U16 u16Addr)
{
    U16 u16data;
    U16 u16temp;
    if (u16Addr & 0x1)
    {
        // read high byte
        MAdp_MPIF_2A(0, u8slaveid, (u16Addr+1), &u16temp);
        u16data = u16temp << 8;
        // read low byte
        MAdp_MPIF_2A(0, u8slaveid, (u16Addr & ~0x1), &u16temp);
        u16data = u16data | (u16temp>>8);
    }
    else
    {
        MAdp_MPIF_2A(0, u8slaveid, u16Addr, &u16data);
    }
    return u16data;
}

U32 MAdp_MPIF_Read3Byte(U8 u8slaveid, U16 u16Addr)
{
    U32 u32data;
    U16 u16temp;
    if (u16Addr & 0x1)
    {
        // read high word
        MAdp_MPIF_2A(0, u8slaveid, (u16Addr+1), &u16temp);
        u32data = ((U32)u16temp) << 8;
        // read low byte
        MAdp_MPIF_2A(0, u8slaveid, (u16Addr & ~0x1), &u16temp);
        u32data = u32data | (u16temp>>8);
    }
    else
    {
        // read high byte
        MAdp_MPIF_2A(0, u8slaveid, (u16Addr+2), &u16temp);
        u32data = ((U32)(u16temp & 0xFF))<<16;
        // read low word
        MAdp_MPIF_2A(0, u8slaveid, u16Addr, &u16temp);
        u32data = u32data | u16temp;
    }
    return u32data;
}

BOOL MAdp_MPIF_WriteByte(U8 u8slaveid, U16 u16Addr, U8 u8Val)
{
    U16 u16data;
    BOOL bRet = 1;
    bRet &= MAdp_MPIF_2A(0, u8slaveid, (u16Addr & ~0x1), &u16data);
    if (u16Addr & 0x1)
        u16data = (u16data & 0x00FF) | (((U16)u8Val) << 8);
    else
        u16data = (u16data & 0xFF00) | (U16)u8Val;

    bRet &= MAdp_MPIF_2A(1, u8slaveid, (u16Addr & ~0x1), &u16data);
    return bRet;
}

BOOL MAdp_MPIF_Write2Byte(U8 u8slaveid, U16 u16Addr, U16 u16Val)
{
    U16 u16temp;
    BOOL bRet = 1;
    if (u16Addr & 0x1)
    {
        // read & write high byte
        bRet &= MAdp_MPIF_2A(0, u8slaveid, (u16Addr+1), &u16temp);
        u16temp = (u16temp & 0xFF00) | (u16Val >> 8);
        bRet &= MAdp_MPIF_2A(1, u8slaveid, (u16Addr+1), &u16temp);

        // read & write low byte
        bRet &= MAdp_MPIF_2A(0, u8slaveid, (u16Addr & ~0x1), &u16temp);
        u16temp = (u16temp & 0x00FF) | (u16Val << 8);
        bRet &= MAdp_MPIF_2A(1, u8slaveid, (u16Addr & ~0x1), &u16temp);
    }
    else
    {
        bRet &= MAdp_MPIF_2A(1, u8slaveid, u16Addr, &u16Val);
    }
    return bRet;
}

BOOL MAdp_MPIF_Write3Byte(U8 u8slaveid, U16 u16Addr, U32 u32Val)
{
    U16 u16temp;
    BOOL bRet = 1;
    u32Val = u32Val & 0xFFFFFF;
    if (u16Addr & 0x1) // odd address
    {
        // write high word
        u16temp = (U16)(u32Val >> 8);
        bRet &= MAdp_MPIF_2A(1, u8slaveid, (u16Addr+1), &u16temp);

        // read & write low byte
        bRet &= MAdp_MPIF_2A(0, u8slaveid, (u16Addr & ~0x1), &u16temp);
        u16temp = (u16temp & 0x00FF) | (U16)(u32Val << 8);
        bRet &= MAdp_MPIF_2A(1, u8slaveid, (u16Addr & ~0x1), &u16temp);
    }
    else // even address
    {
        // write high byte
        bRet &= MAdp_MPIF_2A(0, u8slaveid, (u16Addr+2), &u16temp);
        u16temp = (u16temp & 0xFF00) | (U16)(u32Val >> 16);
        bRet &= MAdp_MPIF_2A(1, u8slaveid, (u16Addr+2), &u16temp);

        // write low word
        u16temp = (U16)u32Val;
        bRet &= MAdp_MPIF_2A(1, u8slaveid, u16Addr, &u16temp);
    }
    return bRet;
}

#define SPIF_REG_BASE                               (0x3700)
BOOL MAdp_MPIF_Test(U8 u8slaveid, U8 u8TestLevel)
{
    U8 j=0;
    U8 u8data;
    U16 u16data;

    //MAdp_MPIF_Init(0x3C, 1, 1);
    //MAdp_MPIF_InitSPIF(u8slaveid);
    //MAdp_MPIF_SetCmdDataWidth(u8slaveid, MPIF_DATAWIDTH_1, MPIF_DATAWIDTH_2);

    //-- 1A Test ---------------------------
    if (u8TestLevel >= 1)
    {
        U8 kd;
        U8 _u8data[8] = {0x00, 0x01, 0x37, 0x73, 0x55, 0xAA, 0x5A, 0xA5};
        
        for (kd=2; kd<8; kd++)
        {
            u8data = _u8data[kd]+j;
            if(!MAdp_MPIF_1A(1, u8slaveid, kd*2, &u8data))
            {
                MPIF_PRINT("LC1A index %u write NG\n", kd);
                return FALSE;
            }

            if(!MAdp_MPIF_1A(0, u8slaveid, kd*2, &u8data))
            {
                MPIF_PRINT("LC1A index %u read NG\n", kd);
                return FALSE;
            }
            if(u8data != _u8data[kd]+j)
            {
                MPIF_PRINT("LC1A index %u NG, read data=0x%x, golden=0x%x\n", kd, u8data, _u8data[kd]+j);
                return FALSE;
            }
            MPIF_PRINT("LC1A index %u OK\n", kd);
        }
    }

    //-- 2A Test ---------------------------
    if (u8TestLevel >= 2)
    {
        u16data = 0x55AA+j;
        if(!MAdp_MPIF_2A(1, u8slaveid, SPIF_REG_BASE+0x22, &u16data))
        {
            MPIF_PRINT("LC2A no checksum write NG\n");
            return FALSE;
        }
        if(!MAdp_MPIF_2A(0, u8slaveid, SPIF_REG_BASE+0x22, &u16data))
        {
            MPIF_PRINT("LC2A no checksum read NG\n");
            return FALSE;
        }
        if(u16data != 0x55AA+j)
        {
            MPIF_PRINT("LC2A no checksum NG\n");
            return FALSE;
        }
        MPIF_PRINT("LC2A no checksum OK\n");
        u16data = 0x4747+j;
        if(!MAdp_MPIF_2A(1, u8slaveid, SPIF_REG_BASE+0x22, &u16data))
        {
            MPIF_PRINT("LC2A checksum write NG\n");
            return FALSE;
        }
        if(!MAdp_MPIF_2A(0, u8slaveid, SPIF_REG_BASE+0x22, &u16data))
        {
            MPIF_PRINT("LC2A checksum read NG\n");
            return FALSE;
        }
        if(u16data != 0x4747+j)
        {
            MPIF_PRINT("LC2A checksum NG\n");
            return FALSE;
        }
        MPIF_PRINT("LC2A checksum OK\n");
    }

    return TRUE;
}



