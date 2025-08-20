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

#define _MHAI_JPD_C_

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/delay.h>
//#include <asm/io.h>

#include "mst_devid.h"
#include "mhal_chiptop_reg.h"
#include "mhal_jpd.h"
#include "mhal_jpd_reg.h"

static U16 _u16SWRST_Addr = JPD_SWRST_S4L;
//static U16 _u16JPDClock_Addr = JPD_CLOCK_S4L;
#define JPEG_PRINT(fmt, args...)       // default OFF for MP3 with Photo, spmarine 080910 ::  printk("[JPEG][%06d]     " fmt, __LINE__, ## args)

extern unsigned int v2p_offset ;
#define REG_MIU_BASE    (0xBF200000 + 0x900*4)
#define REG_MIU_SEL2    (0x7A*4 + (REG_MIU_BASE))
#define MIU_REG(addr)   (*(volatile U32 *)(addr))

//#include "utl.h"
//////////////////////////////////////////////////////////////////////
//
//   Function Declararion
//
//////////////////////////////////////////////////////////////////////
//void MDrv_Write2Byte ( U16 u16Reg, U16 u16Value);
//U16  MDrv_Read2Byte ( U16 u16Reg);
/******************************************************************************/
///JPD driver initialize -- before calling JPD related driver, this function must be called first
///@param NULL
/******************************************************************************/
void MHal_JPD_Initialize(void)
{
    printk("<%s>\n", __FUNCTION__);

#if 0 // MIU is configured in system layer
#if defined(CONFIG_MSTAR_TITANIA_MMAP_64MB_64MB) || \
    defined(CONFIG_MSTAR_TITANIA_MMAP_128MB_128MB)
// 64MB_64MB, MVD buffer is in MIU1
    #if (CONFIG_MSTAR_TITANIA_BD_T3_FPGA)
    // DO NOT set MIU on FPGA
    MHal_JPD_SetMIU(0);
    #elif (CONFIG_MSTAR_TITANIA_BD_GP2_DEMO1)
    MHal_JPD_SetMIU(0);
    #else
    MHal_JPD_SetMIU(1);
    #endif
#endif
#endif

    return;

#if 0  //need to check   shron
    U16 chip_type;

    chip_type = MDrv_Read2Byte(0x1ECC) & 0x0F;

    if(chip_type==3)    // ERIS
    {
        _u16SWRST_Addr = JPD_SWRST_S4;
        _u16JPDClock_Addr = JPD_CLOCK_S4;
        printf("ERIS\n");
    }
    else if(chip_type==4)   // TITANIA
    {
        _u16SWRST_Addr = JPD_SWRST_S4L;
        _u16JPDClock_Addr = JPD_CLOCK_S4L;
        printf("TITANIA\n");
    }
    else
    {
        printf("Unknown CHIP!!\n");
    }
#endif
}

/******************************************************************************/
///Reset JPD -- before trigger JPD, the reset must be called first
///@param NULL
/******************************************************************************/
void MHal_JPD_Reset(void)
{
    JPD_REG(REG_JPD_SCONFIG) = 0;
    JPD_REG(REG_JPD_SCONFIG) = JPD_SWRST_S4L;
    JPD_REG(REG_JPD_INTEN) = JPD_EVENT_ALL;

#if 0
    // reset : low active
    MDrv_Write2Byte(BK_JPD_SCONFIG, 0);
    MDrv_Write2Byte(BK_JPD_SCONFIG, JPD_SWRST_S4L);  //sharon

    // enable all events report
    MDrv_Write2Byte(BK_JPD_INTEN, JPD_EVENT_ALL);
#endif
}

// This function need to be refined.
U16 MHal_JPD_GetSWResetMask(void)
{
    return _u16SWRST_Addr;
}

/******************************************************************************/
///This function can be only used by Pluto/T1/T2... not include ERIS
///Pause JPD write action and then reset JPD
///@param NULL
/******************************************************************************/
void MHal_JPD_SW_Pause_Reset(void)
{
    U8 count = 0;
    U16 regVal,u16Count;

    JPD_REG(REG_JPD_MCONFIG) = (JPD_REG(REG_JPD_MCONFIG)|JPD_MWC_PAUSE);

    // According to designer told me, we need to get the same value (0) twice for MWC_ACTIVE, then we can reset JPD
    while(count<2)
    {
        regVal = JPD_REG(REG_JPD_MCONFIG);

        u16Count = 1000;

        while( u16Count > 0 )
        {
            if(regVal & JPD_MWC_ACTIVE)
            {
                msleep(1);
                u16Count --;
            }
            else
                break;
        }

        count++;
    }

    MHal_JPD_Reset();
#if 0
    U8 count = 0;
    U32 start_time;

    if(_u8ChipType==ERIS)
        return;

    MDrv_Write2Byte( BK_JPD_MCONFIG, MDrv_Read2Byte(BK_JPD_MCONFIG) | JPD_MWC_PAUSE);

    start_time = MsOS_GetSystemTime();
    while(1)
    {
        U16 regVal;

        // timeout, prevent from forever loop
        if((MsOS_GetSystemTime() - start_time)>=500)
            break;

        regVal = MDrv_Read2Byte(BK_JPD_MCONFIG);
        if(regVal & JPD_MWC_ACTIVE)
            continue;

        count++;

        // According to designer told me, we need to get the same value (0) twice for MWC_ACTIVE, then we can reset JPD
        if(count>=2)
            break;
#endif
}

/******************************************************************************/
///Set RCReadBuffer for JPD
///@param u32BufAddr \b IN Read buffer linear address in memory
///@param u16BufSize \b IN Read buffer size
/******************************************************************************/
void MHal_JPD_SetReadBuffer(U32 u32BufAddr, U32 u32BufSize)
{
    //u32BufAddr = virt_to_phys(u32BufAddr);
    //JPEG_PRINT("MRB v2p= 0x%X\n", u32BufAddr);

	u32BufAddr -= v2p_offset;
    JPEG_PRINT("MRB= 0x%X\n", u32BufAddr);

    JPD_REG(REG_JPD_RBUF_FLOOR_L) = (u32BufAddr >> 3) & 0xffff;
    JPD_REG(REG_JPD_RBUF_FLOOR_H) = (u32BufAddr >> 3) >> 16;

    JPD_REG(REG_JPD_RBUF_CEIL_L) = (((( u32BufAddr + u32BufSize ) >> 3)-1)&0xffff);
    JPD_REG(REG_JPD_RBUF_CEIL_H) = ((( u32BufAddr + u32BufSize ) >> 3)-1) >> 16;

#if 0
    u32BufAddr = u32BufAddr & ~AEON_NON_CACHE_MASK;

    // set start address of read buffer
    MDrv_Write2Byte(BK_JPD_RBUF_FLOOR_L, ( u32BufAddr >> 3 ) & 0xffff);
    MDrv_Write2Byte(BK_JPD_RBUF_FLOOR_H, ( u32BufAddr >> 3 ) >> 16);

    // set end address of read buffer
    MDrv_Write2Byte(BK_JPD_RBUF_CEIL_L, (( ( u32BufAddr + u32BufSize ) >> 3 ) & 0xffff) - 1);
    MDrv_Write2Byte(BK_JPD_RBUF_CEIL_H, ( ( u32BufAddr + u32BufSize ) >> 3 ) >> 16);
#endif

    //printf("BK_JPD_RBUF_FLOOR_L = %08x\n",( u32BufAddr >> 3 ) & 0xffff);
    //printf("BK_JPD_RBUF_FLOOR_H = %08x\n",( u32BufAddr >> 3 ) >> 16);
    //printf("BK_JPD_RBUF_CEIL_L = %08x\n",(( ( u32BufAddr + u32BufSize ) >> 3 ) & 0xffff) - 1);
    //printf("BK_JPD_RBUF_CEIL_H = %08x\n",( ( u32BufAddr + u32BufSize ) >> 3 ) >> 16);
}

/******************************************************************************/
///Set MRC start address for JPD
///@param u32ByteOffset \b IN Start address for JPD reading in MRC buffer
/******************************************************************************/
void MHal_JPD_SetMRCStartAddr(U32 u32ByteOffset)
{
    //u32ByteOffset = virt_to_phys(u32ByteOffset);
    //JPEG_PRINT("MRCStart v2p= 0x%X\n", u32ByteOffset);

    u32ByteOffset -= v2p_offset;
    JPEG_PRINT("MRCStart= 0x%X\n", u32ByteOffset);

    JPD_REG(REG_JPD_RCSMADR_L) = (u32ByteOffset & 0xffff);
    JPD_REG(REG_JPD_RCSMADR_H) = (u32ByteOffset >> 16);

#if 0
    u32ByteOffset = u32ByteOffset & ~AEON_NON_CACHE_MASK;

    MDrv_Write2Byte(BK_JPD_RCSMADR_L, (u32ByteOffset & 0xffff));
    MDrv_Write2Byte(BK_JPD_RCSMADR_H, (u32ByteOffset >> 16));
#endif
}

/******************************************************************************/
///Set output frame buffer address for JPD writing JPEG uncompressed data
///@param u32BufAddr \b IN Start address for JPD reading in MRC buffer
/******************************************************************************/
void MHal_JPD_SetOutputFrameBuffer(U32 u32BufAddr)
{
    //u32BufAddr = virt_to_phys(u32BufAddr);
    //JPEG_PRINT("MWB v2p= 0x%X\n", u32BufAddr);

    u32BufAddr -= v2p_offset;
    JPEG_PRINT("MWB= 0x%X\n", u32BufAddr);

    JPD_REG(REG_JPD_MWBF_SADR_L) = (u32BufAddr >> 3) & 0xffff;
    JPD_REG(REG_JPD_MWBF_SADR_H) = (u32BufAddr >> 3) >> 16;

#if 0
    u32BufAddr = u32BufAddr & ~AEON_NON_CACHE_MASK;

    MDrv_Write2Byte(BK_JPD_MWBF_SADR_L, ( u32BufAddr >> 3 ) & 0xffff);
    MDrv_Write2Byte(BK_JPD_MWBF_SADR_H, ( u32BufAddr >> 3 ) >> 16);
#endif
}

/******************************************************************************/
///Set width and height of picture
///@param u16Width \b IN picture width
///@param u16Height \b IN picture height
/******************************************************************************/
void MHal_JPD_SetPicDimension(U16 u16Width, U16 u16Height)
{
    JPD_REG(REG_JPD_PIC_H) = (u16Width >> 3);
    JPD_REG(REG_JPD_PIC_V) = (u16Height >> 3);

#if 0
    MDrv_Write2Byte( BK_JPD_PIC_H, ( u16Width >> 3 ) );
    MDrv_Write2Byte( BK_JPD_PIC_V, ( u16Height >> 3 ) );
#endif
}

/******************************************************************************/
///Get JPD decode status
///@return status
/******************************************************************************/
extern void Chip_Flush_Memory(void);
U16 MHal_JPD_ReadJPDStatus(void)
{
    Chip_Flush_Memory(); //2009/12/31 patch
    return JPD_REG(REG_JPD_EVENTFLAG);

    //return MDrv_Read2Byte((U16)BK_JPD_EVENTFLAG);
}

/******************************************************************************/
///Clear JPD decode status
///@param status_bit \n IN status
/******************************************************************************/
void MHal_JPD_ClearJPDStatus(U16 status_bit)
{

    JPD_REG(REG_JPD_EVENTFLAG) = status_bit;
    JPEG_PRINT("Clear: Bit= %X \n", status_bit);
    if( status_bit == 0x10 )
    {
        JPD_REG(REG_JPD_MCONFIG) |= 0x01;
    }

    else if( status_bit == 0x20 )
    {
        JPD_REG(REG_JPD_MCONFIG) |= 0x02;
    }
    else if ( status_bit == 0x30)
    {
        JPD_REG(REG_JPD_MCONFIG) |= 0x01;
        JPD_REG(REG_JPD_MCONFIG) |= 0x02;
    }
    // clear by write
    //MDrv_Write2Byte(BK_JPD_EVENTFLAG, status_bit);
}

/******************************************************************************/
///Set ROI region
///@param start_x \n IN start X position
///@param start_y \n IN start Y position
///@param width \n IN width
///@param height \n IN height
/******************************************************************************/
void MHal_JPD_SetROI(U16 start_x, U16 start_y, U16 width, U16 height)
{
    JPD_REG(REG_JPD_ROI_H) = start_x;
    JPD_REG(REG_JPD_ROI_V) = start_y;
    JPD_REG(REG_JPD_ROI_WIDTH) = width;
    JPD_REG(REG_JPD_ROI_HEIGHT) = height;

#if 0
    U16 reg_val;

    reg_val = MDrv_Read2Byte(BK_JPD_SCONFIG);

    MDrv_Write2Byte(BK_JPD_ROI_H, start_x);
    MDrv_Write2Byte(BK_JPD_ROI_V, start_y);
    MDrv_Write2Byte(BK_JPD_ROI_WIDTH, width );
    MDrv_Write2Byte(BK_JPD_ROI_HEIGHT, height );
    //MDrv_Write2Byte(BK_JPD_SCONFIG, reg_val | JPD_ROI_EN);
#endif
}

/******************************************************************************/
///Set ROI region
///@param On \n IN Turn On or Off JPD clock
/******************************************************************************/
// Use REG_TOP_BASE defined in mhal_chiptop_reg.h
//#define REG_TOP_BASE                0xBF203C00
//#define TOP_REG(addr)               (*((volatile u32*)(REG_TOP_BASE + ((addr)<<2))))

void MHal_JPD_SetClock(U8 on)
{

    if (on)
    {
        //TOP_REG(0x0035) = 0x0008;
        JPEG_PRINT("MHal_JPD_SetClock : 0x%08X\n", CKLGEN0_REG(0x0035));
        CKLGEN0_REG(0x0035) = 0x00000008;  // [0]:enable-0, [3:2]:clock-10(123MHz)
        //CKLGEN0_REG(0x0035) = 0x00000004;  // [0]:enable-0, [3:2]:clock-01(104MHz)
        // CKLGEN0_REG(0x0035) = 0x00000000;  // [0]:enable-0, [3:2]:clock-01(72MHz)
        JPEG_PRINT("MHal_JPD_SetClock : 0x%08X\n", CKLGEN0_REG(0x0035));
    }
    else
    {
        JPEG_PRINT("MHal_JPD_SetClock : 0x%08X\n", CKLGEN0_REG(0x0035));
        CKLGEN0_REG(0x0035) = 0x00000009;  // [0]:disable-1, [3:2]:clock-10(123MHz)
        //CKLGEN0_REG(0x0035) = 0x00000005;  // [0]:disable-1, [3:2]:clock-01(104MHz)
         //CKLGEN0_REG(0x0035) = 0x00000004;  // [0]:disable-1, [3:2]:clock-01(72MHz)
        JPEG_PRINT("MHal_JPD_SetClock : 0x%08X\n", CKLGEN0_REG(0x0035));
    }


#if 0
    U16 reg_val;

    if(on)
    {
        reg_val = MDrv_Read2Byte(_u16JPDClock_Addr);
        reg_val = reg_val & 0xF2FF;
        reg_val |= 0x0400;  // set JPD disable [8] : 0, [11:10] : 01
        MDrv_Write2Byte(_u16JPDClock_Addr, reg_val);
    }
    else
    {
        reg_val = MDrv_Read2Byte(_u16JPDClock_Addr);
        reg_val = reg_val & 0xF2FF;
        reg_val |= 0x0500;  // set JPD disable [8] : 1, [11:10] : 01
        MDrv_Write2Byte(_u16JPDClock_Addr, reg_val);
    }
#endif
}

/******************************************************************************/
///Get JPD current MRC address
///@return status
/******************************************************************************/
U32 MHal_JPD_ReadCurrentMRCAddr(void)
{
    U32 u32CurrMRC;

    u32CurrMRC = ( (JPD_REG(REG_JPD_CUR_MADR_H)<<16) | JPD_REG(REG_JPD_CUR_MADR_L) );

//    u32CurrMRC = ( AEON_NON_CACHE_MASK | (MDrv_Read2Byte((U16)BK_JPD_CUR_MADR_H)<<16) | MDrv_Read2Byte((U16)BK_JPD_CUR_MADR_L) );
    //printf("CurrMRC= 0x%X\n", u32CurrMRC);

    return u32CurrMRC;
}

#if 0 //MIU is configured in system layer
/******************************************************************************/
///Set which to use for JPD, miu_no = 0 or 1
///@return status
/******************************************************************************/
void MHal_JPD_SetMIU(int miu_no)
{
    if(miu_no == 0)
    {
        MIU_REG(REG_MIU_SEL2) &= ~0x3000;   // set read/write for JPD in MIU0
    }
    else
    {
        MIU_REG(REG_MIU_SEL2) |= 0x3000;    // set read/write for JPD in MIU1
    }
}
#endif

#undef _MHAL_JPD_C_

