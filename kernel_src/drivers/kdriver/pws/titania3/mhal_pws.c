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
/// file    mhal_pws.c
/// @brief  power save mode
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
// Header Files
//------------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/delay.h>
#include "mhal_demod_reg.h"
#include "mdrv_types.h"
#include "mhal_pws.h"
//#include "regPWS.h"
//------------------------------------------------------------------------------
// Define & data type
//------------------------------------------------------------------------------
#define MHAL_DEBUG_INFO(y)    //y

#define DemodCmdWrReg     0x01    // write register.
#define DemodCmdRdReg     0x02    // read register.

#define DemodCmd          0x00000000L//0x110500L
#define DemodAdrL         0x00000001L//0x110501L
#define DemodAdrH         0x00000004L//0x110502L
#define DemodData         0x00000005L//0x110503L

//#define MBRegBase         0xBF220A00//0x110500

#define BIN_FOR_ATV       0001
#define BIN_FOR_DVBT      0002
#define BIN_FOR_DVBC      0003
#define BIN_FOR_ATSC      0004


//#define _MHal_W1B( u32Reg, u08Val )   (X1BYTE(u32Reg) = u08Val)
//#define _MHal_W2B( u32Reg, u16Val )   (X2BYTE(u32Reg) = u16Val)


//------------------------------------------------------------------------------
// Local variable
//------------------------------------------------------------------------------
U32 _gPWS_MapBase;

//------------------------------------------------------------------------------
// Local Function
//------------------------------------------------------------------------------
void MHal_PWS_WriteReg(U32 u32Addr, U8 u8Value)
{
    _MHal_W1B(BYTE2REAL(u32Addr & 0x000FFFFF), u8Value);
}

U32 MHal_PWS_ReadReg(U32 u32Addr)
{
    return _MHal_R1B(BYTE2REAL(u32Addr & 0x000FFFFF));
}

U8 MHal_PWS_WriteRegBit(U32 u32RegAddr, U8 bEnable, U8 u8BitPos)
{
    U8 u8Val = MHal_PWS_ReadReg(u32RegAddr);
    if (!u32RegAddr)
    {
        return FALSE;
    }

    u8Val = MHal_PWS_ReadReg(u32RegAddr);
    u8Val = (bEnable) ? (u8Val | u8BitPos) : (u8Val & ~u8BitPos);
    MHal_PWS_WriteReg(u32RegAddr, u8Val);
    return TRUE;
}

void MHal_PWS_WriteRegMask(U32 u32RegAddr, U8 u8Val, U8 u8Mask)
{
    MHal_PWS_WriteReg(u32RegAddr, (U8)(MHal_PWS_ReadReg(u32RegAddr) & (~u8Mask)) | (u8Val & u8Mask));
}

// Access VIF -> Sync with APP Mailbox API
// ATV
U32 MHal_ATV_ReadReg(U32 u32Reg)
{
    U32 u32Timeout=5000;
    while (MHal_PWS_ReadReg(DemodCmd) && u32Timeout ) u32Timeout--; // wait VDMCU ready
    MHal_PWS_WriteReg(DemodAdrL, u32Reg&0xFF);
    MHal_PWS_WriteReg(DemodAdrH, (u32Reg>>8)&0xFF);
    MHal_PWS_WriteReg(DemodCmd, DemodCmdRdReg);

    u32Timeout=5000;
    while (MHal_PWS_ReadReg(DemodCmd) && u32Timeout ) u32Timeout--; // wait VDMCU ready
    return MHal_PWS_ReadReg(DemodData);
}

void MHal_ATV_WriteReg( U32 u32Reg, U8 u8Val)
{
    U32 u32Timeout=5000;
    while (MHal_PWS_ReadReg(DemodCmd) && u32Timeout ) u32Timeout--; // wait VDMCU ready
    MHal_PWS_WriteReg(DemodAdrL, u32Reg&0xFF);
    MHal_PWS_WriteReg(DemodAdrH, (u32Reg>>8)&0xFF);
    MHal_PWS_WriteReg(DemodData, u8Val);
    MHal_PWS_WriteReg(DemodCmd, DemodCmdWrReg);
}

void MHal_ATV_WriteRegBit(U32 u32Reg, U8 bEnable, U8 u8Mask )
{
    U32 u32Value;
    u32Value = MHal_ATV_ReadReg(u32Reg);
    MHal_ATV_WriteReg(u32Reg, (bEnable) ? (u32Value | (u8Mask)) : (u32Value & ~(u8Mask)));
}

// DVBT & DVBC
U32 MHal_DVB_ReadReg(U32 u32Addr)
{
//    U16 u16CheckCount;
    U32 u32Value;
    U16 u16WaitCnt=0;
    U32 u32Timeout=5000;

    while (MHal_PWS_ReadReg(MBRegBase + 0x00) && u32Timeout ) u32Timeout--; // wait VDMCU ready

    MHal_PWS_WriteReg(MBRegBase + 0x02, (U8)(u32Addr >> 8));
    MHal_PWS_WriteReg(MBRegBase + 0x01, (U8)u32Addr);
    MHal_PWS_WriteReg(MBRegBase + 0x00, 0x01);
    MHal_PWS_WriteReg(0x103463, 0x02);
    MHal_PWS_WriteReg(0x103463, 0x00);

    while(MHal_PWS_ReadReg(MBRegBase + 0x00) != 0xFF)           // wait MB_CNTL set done
    {
        if (u16WaitCnt++ >= 0xFF)
        {
            MHAL_DEBUG_INFO(printf(">> DVBT ReadReg Fail!"));
        }
    }

    u32Value = MHal_PWS_ReadReg(MBRegBase + 0x03);             // REG_DATA get
    MHal_PWS_WriteReg(MBRegBase + 0x00, 0x00);                 // MB_CNTL clear

    return u32Value;
}

void MHal_DVB_WriteReg(U32 u32Addr, U8 u8Data)
{
//    U16 u16CheckCount;
    U16 u16WaitCnt=0;

    MHal_PWS_WriteReg(MBRegBase + 0x02, (U8)(u32Addr >> 8));   // ADDR_H
    MHal_PWS_WriteReg(MBRegBase + 0x01, (U8)u32Addr);          // ADDR_L
    MHal_PWS_WriteReg(MBRegBase + 0x03, u8Data);               // REG_DATA
    MHal_PWS_WriteReg(MBRegBase + 0x00, 0x02);                 // MB_CNTL set write mode

    MHal_PWS_WriteReg(0x103463, 0x02);                         // assert interrupt to VD MCU51
    MHal_PWS_WriteReg(0x103463, 0x00);                         // de-assert interrupt to VD MCU51

    while(MHal_PWS_ReadReg(MBRegBase + 0x00) != 0xFF)          // wait done flag
    {
        if (u16WaitCnt++ >= 0xFF)
        {
            MHAL_DEBUG_INFO(printf(">> DVBT WriteReg Fail!"));
        }
    }

    MHal_PWS_WriteReg(MBRegBase + 0x00, 0x00);                 // MB_CNTL clear
}

void MHal_DVB_WriteRegBit( U16 u16Reg, U8 bEnable, U8 u8Mask )
{
    U8 u8Value;
    u8Value = MHal_DVB_ReadReg(u16Reg);
    MHal_DVB_WriteReg(u16Reg, (bEnable) ? (u8Value | (u8Mask)) : (u8Value & ~(u8Mask)));
}

// ATSC
void MHal_ATSC_WriteReg(U32 u32Addr, U8 u8Data)
{
    U16 u16CheckCount;
    U8 u8Value;

    MHal_PWS_WriteReg(0x110500, (u32Addr&0xff));
    MHal_PWS_WriteReg(0x110501, (u32Addr>>8));
    MHal_PWS_WriteReg(0x110510, u8Data);
    MHal_PWS_WriteReg(0x11051E, 0x01);

    for ( u16CheckCount=0; u16CheckCount < 200 ; u16CheckCount++ )//total 200ms
    {
      u8Value = MHal_PWS_ReadReg(0x11051E);
      if ((u8Value&0x01)==0)
        break;
      msleep(1);
    }
}

U32 MHal_ATSC_ReadReg(U32 u32Addr)
{
    U16 u16CheckCount;
    U8 u8Value;

    MHal_PWS_WriteReg(0x110500, (u32Addr&0xff));
    MHal_PWS_WriteReg(0x110501, (u32Addr>>8));
    MHal_PWS_WriteReg(0x11051E, 0x02);

    for ( u16CheckCount=0; u16CheckCount < 30000 ; u16CheckCount++ )//andy new driver update 090720
    {
      u8Value = MHal_PWS_ReadReg(0x11051E);
      if ((u8Value&0x02)==0)
      {
        return MHal_PWS_ReadReg(0x110510);
      }
      msleep(1);
    }
    return 0;
}

void MHal_ATSC_WriteRegBit( U32 u32Reg, U8 bEnable, U8 u8Mask )
{
    U8 u8Value;
    u8Value = MHal_ATSC_ReadReg(u32Reg);
    MHal_ATSC_WriteReg(u32Reg, (bEnable) ? (u8Value | (u8Mask)) : (u8Value & ~(u8Mask)));
}

//------------------------------------------------------------------------------
// Global Function
//------------------------------------------------------------------------------
U32 MHal_PWS_GetChipId(void)
{
    U32 u32Device_ID;
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));
    u32Device_ID = MHal_PWS_ReadReg(0x001ECC);
    if(u32Device_ID == 0x000F)   // T3
        return u32Device_ID;
    else
        return 0xFFFF;
}

void MHal_PWS_SetIOMapBase(U32 u32Base)
{
    MHAL_DEBUG_INFO(printk("%s\n",__FUNCTION__));

    _gPWS_MapBase = u32Base;
    MHAL_DEBUG_INFO(printk("PWS_MAP_BASE=%lx\n",_gPWS_MapBase));
}

U32 MHal_PWS_GetIOMapBase(void)
{
    MHAL_DEBUG_INFO(printk("%s\n",__FUNCTION__));
    return _gPWS_MapBase;
}

void MHal_PWS_Init(void)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));
    // default power save register setting (keep ON or OFF)
#if 0
    // VIDEO_ATOP
    MHal_PWS_WriteReg(0x102508, (MHal_PWS_ReadReg(0x102508) & ~(BIT4|BIT3|BIT2|BIT1|BIT0)));
    MHal_PWS_WriteReg(0x102509, (MHal_PWS_ReadReg(0x102509) | (BIT7|BIT5)));
    MHal_PWS_WriteReg(0x10250A, (MHal_PWS_ReadReg(0x10250A) | (BIT0)));
    MHal_PWS_WriteReg(0x10250B, (MHal_PWS_ReadReg(0x10250B) & ~(BIT7|BIT6|BIT5|BIT4|BIT3)));
    MHal_PWS_WriteReg(0x10250C, (MHal_PWS_ReadReg(0x10250C) & ~(BIT0)));
    MHal_PWS_WriteReg(0x10250D, (MHal_PWS_ReadReg(0x10250D) | (BIT7|BIT5|BIT3|BIT1|BIT0)));
    MHal_PWS_WriteReg(0x10250D, (MHal_PWS_ReadReg(0x10250D) & ~(BIT2)));

    // USB
    // Port0
    // Enable override mode
    MHal_PWS_WriteReg(0x103A80, (MHal_PWS_ReadReg(0x103A80) | (BIT7|BIT6|BIT1|BIT0)));
    // Port1
    // Enable override mode
    MHal_PWS_WriteReg(0x103A00, (MHal_PWS_ReadReg(0x103A00) | (BIT7|BIT6|BIT1|BIT0)));

    MHal_PWS_SetUsbPllLdo(0);
    MHal_PWS_SetUsbLdo(0);
    MHal_PWS_SetUsbRefBiasCir(0);

    // Port0
    // Disable override mode
    MHal_PWS_WriteReg(0x103A80, (MHal_PWS_ReadReg(0x103A80) & ~(BIT7|BIT6|BIT1|BIT0)));
    // Port1
    // Disable override mode
    MHal_PWS_WriteReg(0x103A00, (MHal_PWS_ReadReg(0x103A00) & ~(BIT7|BIT6|BIT1|BIT0)));
#endif
}

void MHal_PWS_SetDviDmDemux(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x1109C0, (MHal_PWS_ReadReg(0x1109C0) | (BIT2|BIT1|BIT0)));
        MHal_PWS_WriteReg(0x1109D2, (MHal_PWS_ReadReg(0x1109D2) | (BIT2|BIT1|BIT0)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x1109C0, (MHal_PWS_ReadReg(0x1109C0) & ~(BIT2|BIT1|BIT0)));
        MHal_PWS_WriteReg(0x1109D2, (MHal_PWS_ReadReg(0x1109D2) & ~(BIT2|BIT1|BIT0)));
    }
}

void MHal_PWS_SetDviPreamp(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x1109C0, (MHal_PWS_ReadReg(0x1109C0) | (BIT2|BIT1|BIT0)));
        MHal_PWS_WriteReg(0x1109D2, (MHal_PWS_ReadReg(0x1109D2) | (BIT2|BIT1|BIT0)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x1109C0, (MHal_PWS_ReadReg(0x1109C0) & ~(BIT2|BIT1|BIT0)));
        MHal_PWS_WriteReg(0x1109D2, (MHal_PWS_ReadReg(0x1109D2) & ~(BIT2|BIT1|BIT0)));
    }

}

void MHal_PWS_SetDviBist(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x1109C6, (MHal_PWS_ReadReg(0x1109C6) | (BIT5|BIT4|BIT3)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x1109C6, (MHal_PWS_ReadReg(0x1109C6) & ~(BIT5|BIT4|BIT3)));
    }

}

void MHal_PWS_SetDviDmRxckBist(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x1109C6, (MHal_PWS_ReadReg(0x1109C6) | (BIT2)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x1109C6, (MHal_PWS_ReadReg(0x1109C6) & ~(BIT2)));
    }

}

void MHal_PWS_SetDviDmPreclk(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x000E97, (MHal_PWS_ReadReg(0x000E97) | (BIT0|BIT2)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x000E97, (MHal_PWS_ReadReg(0x000E97) & ~(BIT0|BIT2)));
    }

}

void MHal_PWS_SetDviDmPreclkOffl(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x000E97, (MHal_PWS_ReadReg(0x000E97) | (BIT1)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x000E97, (MHal_PWS_ReadReg(0x000E97) & ~(BIT1)));
    }

}

void MHal_PWS_SetDviDmRbias(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x000E98, (MHal_PWS_ReadReg(0x000E98) | (BIT0|BIT1)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x000E98, (MHal_PWS_ReadReg(0x000E98) & ~(BIT0|BIT1)));
    }

}

void MHal_PWS_SetDviDmEnvdet(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x1109BE, (MHal_PWS_ReadReg(0x1109BE) | (BIT0)));
        MHal_PWS_WriteReg(0x1109D8, (MHal_PWS_ReadReg(0x1109D8) | (BIT0)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x1109BE, (MHal_PWS_ReadReg(0x1109BE) & ~(BIT0)));
        MHal_PWS_WriteReg(0x1109D8, (MHal_PWS_ReadReg(0x1109D8) & ~(BIT0)));
    }

}

void MHal_PWS_SetDviPllCore(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x1109C0, (MHal_PWS_ReadReg(0x1109C0) | (BIT7)));
        MHal_PWS_WriteReg(0x1109D2, (MHal_PWS_ReadReg(0x1109D2) | (BIT7)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x1109C0, (MHal_PWS_ReadReg(0x1109C0) & ~(BIT7)));
        MHal_PWS_WriteReg(0x1109D2, (MHal_PWS_ReadReg(0x1109D2) & ~(BIT7)));
    }

}

void MHal_PWS_SetDviPllRegm(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x1109C0, (MHal_PWS_ReadReg(0x1109C0) | (BIT5)));
        MHal_PWS_WriteReg(0x1109D2, (MHal_PWS_ReadReg(0x1109D2) | (BIT5)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x1109C0, (MHal_PWS_ReadReg(0x1109C0) & ~(BIT5)));
        MHal_PWS_WriteReg(0x1109D2, (MHal_PWS_ReadReg(0x1109D2) & ~(BIT5)));
    }

}

void MHal_PWS_SetDviPllPhdac(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x1109C1, (MHal_PWS_ReadReg(0x1109C1) | (BIT2|BIT1|BIT0)));
        MHal_PWS_WriteReg(0x1109D3, (MHal_PWS_ReadReg(0x1109D3) | (BIT2|BIT1|BIT0)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x1109C1, (MHal_PWS_ReadReg(0x1109C1) & ~(BIT2|BIT1|BIT0)));
        MHal_PWS_WriteReg(0x1109D3, (MHal_PWS_ReadReg(0x1109D3) & ~(BIT2|BIT1|BIT0)));
    }

}

void MHal_PWS_SetSingalPadRArray(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteReg(0x000E97, (MHal_PWS_ReadReg(0x000E97) | (BIT5|BIT4|BIT3)));
        MHal_PWS_WriteReg(0x1109C1, (MHal_PWS_ReadReg(0x1109C1) | (BIT5|BIT4|BIT3)));
        MHal_PWS_WriteReg(0x000E97, (MHal_PWS_ReadReg(0x000E97) | (BIT6)));
        MHal_PWS_WriteReg(0x1109D3, (MHal_PWS_ReadReg(0x1109D3) | (BIT3)));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteReg(0x000E97, (MHal_PWS_ReadReg(0x000E97) & ~(BIT5|BIT4|BIT3)));
        MHal_PWS_WriteReg(0x1109C1, (MHal_PWS_ReadReg(0x1109C1) & ~(BIT5|BIT4|BIT3)));
        MHal_PWS_WriteReg(0x000E97, (MHal_PWS_ReadReg(0x000E97) & ~(BIT6)));
        MHal_PWS_WriteReg(0x1109D3, (MHal_PWS_ReadReg(0x1109D3) & ~(BIT3)));
    }

}

void MHal_PWS_SetVifBandgapIbiasVref(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103440, 1,  BIT4);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3440, 1,  BIT4);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3440, 1,  BIT4);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103440, 0,  BIT4);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3440, 0,  BIT4);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3440, 0,  BIT4);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifCalibrationBuffer(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x10341E, 1,  BIT7);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x341E, 1,  BIT7);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x341E, 1,  BIT7);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x10341E, 0,  BIT7);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x341E, 0,  BIT7);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x341E, 0,  BIT7);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifClampBuffer(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103418, 1,  BIT2);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3418, 1,  BIT2);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3418, 1,  BIT2);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103418, 0,  BIT2);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3418, 0,  BIT2);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3418, 0,  BIT2);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifAdcI(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103418, 1,  BIT0);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3418, 1,  BIT0);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3418, 1,  BIT0);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103418, 0,  BIT0);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3418, 0,  BIT0);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3418, 0,  BIT0);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifAdcQ(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103418, 1,  BIT1);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3418, 1,  BIT1);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3418, 1,  BIT1);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103418, 0,  BIT1);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3418, 0,  BIT1);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3418, 0,  BIT1);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifPga1(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103440, 1,  BIT6);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3440, 1,  BIT6);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3440, 1,  BIT6);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103440, 0,  BIT6);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3440, 0,  BIT6);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3440, 0,  BIT6);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifPga2(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103440, 1,  BIT5);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3440, 1,  BIT5);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3440, 1,  BIT5);
        else
        {
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
        }
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103440, 0,  BIT5);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3440, 0,  BIT5);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3440, 0,  BIT5);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifMpllReg(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 1,  BIT0);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 1,  BIT0);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 1,  BIT0);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 0,  BIT0);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 0,  BIT0);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 0,  BIT0);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifAdcOutClkPd(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 1,  BIT1);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 1,  BIT1);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 1,  BIT1);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 0,  BIT1);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 0,  BIT1);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 0,  BIT1);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifMpll_div2_pd(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 1,  BIT2);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 1,  BIT2);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 1,  BIT2);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 0,  BIT2);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 0,  BIT2);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 0,  BIT2);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifMpll_div3_pd(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 1,  BIT3);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 1,  BIT3);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 1,  BIT3);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 0,  BIT3);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 0,  BIT3);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 0,  BIT3);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifMpll_div4_pd(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 1,  BIT4);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 1,  BIT4);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 1,  BIT4);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 0,  BIT4);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 0,  BIT4);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 0,  BIT4);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifMpll_div8_pd(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 1,  BIT5);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 1,  BIT5);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 1,  BIT5);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 0,  BIT5);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 0,  BIT5);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 0,  BIT5);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifMpll_div10_pd(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 1,  BIT6);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 1,  BIT6);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 1,  BIT6);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x103460, 0,  BIT6);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x3460, 0,  BIT6);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x3460, 0,  BIT6);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }

}

void MHal_PWS_SetVifTagc(U8 OnOff_flag)
{
    U8 u8BinType;

    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    u8BinType = MHal_PWS_ReadReg(0x101E3E);
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x10346C, 0,  BIT2);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x346C, 0,  BIT2);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x346C, 0,  BIT2);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        if(u8BinType & BIN_FOR_ATV)
            MHal_ATV_WriteRegBit(0x10346C, 1,  BIT2);
        else if(u8BinType & (BIN_FOR_DVBC|BIN_FOR_DVBT))
            MHal_DVB_WriteRegBit(0x346C, 1,  BIT2);
        else if(u8BinType & BIN_FOR_ATSC)
            MHal_ATSC_WriteRegBit(0x346C, 1,  BIT2);
        else
            MHAL_DEBUG_INFO(printf("No any Bin in VDMCU!!\n"));
    }
}


void MHal_PWS_SetUsbHsTx(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));
    // Port0
    // Enable override mode
    MHal_PWS_WriteReg(0x103A80, (MHal_PWS_ReadReg(0x103A80) | (BIT7|BIT6|BIT1|BIT0)));
    // Port1
    // Enable override mode
    MHal_PWS_WriteReg(0x103A00, (MHal_PWS_ReadReg(0x103A00) | (BIT7|BIT6|BIT1|BIT0)));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 1, 1<<6);    // hs_tx OFF
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<6);    // hs_tx OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<6);    // hs_tx ON
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<6);    // hs_tx OFF
    }

}

void MHal_PWS_SetUsbHsRx(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 1, 1<<2);    // hs_rx OFF
        MHal_PWS_WriteRegBit(0x103A81, 1, 1<<3);    // hs_rx OFF
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<2);    // hs_rx OFF
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<3);    // hs_rx OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<2);    // hs_rx OFF
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<3);    // hs_rx OFF
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<2);    // hs_rx OFF
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<3);    // hs_rx OFF
    }

}

void MHal_PWS_SetUsbFlXcvr(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 1, 1<<4);    // fl_xcvr OFF
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<4);    // fl_xcvr OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<4);    // fl_xcvr OFF
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<4);    // fl_xcvr OFF
    }

}

void MHal_PWS_SetUsbSerdes(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 1, 1<<0);    // serdes OFF
        MHal_PWS_WriteRegBit(0x103A90, 0, 1<<6);    // serdes OFF
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<0);    // serdes OFF
        MHal_PWS_WriteRegBit(0x103A10, 0, 1<<6);    // serdes OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<0);    // serdes ON
        MHal_PWS_WriteRegBit(0x103A90, 1, 1<<6);    // serdes ON
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<0);    // serdes ON
        MHal_PWS_WriteRegBit(0x103A10, 1, 1<<6);    // serdes ON
    }

}

void MHal_PWS_SetUsbPllLdo(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));
#if 0
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 1, 1<<1);    // pll+LDO OFF
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<1);    // pll+LDO OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<1);    // pll+LDO ON
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<1);    // pll+LDO ON
    }
#else
// Port 0 : always ON
// Port 1 : USB ON , else OFF
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<1);    // pll+LDO ON
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<1);    // pll+LDO OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<1);    // pll+LDO ON
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<1);    // pll+LDO ON
    }
#endif
}

void MHal_PWS_SetUsbLdo(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));
#if 0
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 1, 1<<7);    // LDO OFF
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<7);    // LDO OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<7);    // LDO ON
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<7);    // LDO ON
    }
#else
// Port 0 : always ON
// Port 1 : USB ON , else OFF
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<7);    // LDO ON
        MHal_PWS_WriteRegBit(0x103A01, 1, 1<<7);    // LDO OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A81, 0, 1<<7);    // LDO ON
        MHal_PWS_WriteRegBit(0x103A01, 0, 1<<7);    // LDO ON
    }
#endif
}

void MHal_PWS_SetUsbRefBiasCir(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));
#if 0
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A80, 1, 1<<2);    // ref & bias OFF
        MHal_PWS_WriteRegBit(0x103A00, 1, 1<<2);    // ref & bias OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A80, 0, 1<<2);    // ref & bias ON
        MHal_PWS_WriteRegBit(0x103A00, 0, 1<<2);    // ref & bias ON
    }
#else
// Port 0 : always ON
// Port 1 : USB ON , else OFF

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x103A80, 0, 1<<2);    // ref & bias ON
        MHal_PWS_WriteRegBit(0x103A00, 1, 1<<2);    // ref & bias OFF
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x103A80, 0, 1<<2);    // ref & bias ON
        MHal_PWS_WriteRegBit(0x103A00, 0, 1<<2);    // ref & bias ON
    }
#endif

    // Port0
    // Disable override mode
    MHal_PWS_WriteReg(0x103A80, (MHal_PWS_ReadReg(0x103A80) & ~(BIT7|BIT6|BIT1|BIT0)));
    // Port1
    // Disable override mode
    MHal_PWS_WriteReg(0x103A00, (MHal_PWS_ReadReg(0x103A00) & ~(BIT7|BIT6|BIT1|BIT0)));
}

void MHal_PWS_SetAdcR(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102508, 1, 1<<5);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102508, 0, 1<<5);
    }

}

void MHal_PWS_SetAdcG(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102508, 1, 1<<6);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102508, 0, 1<<6);
    }

}

void MHal_PWS_SetAdcB(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102508, 1, 1<<7);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102508, 0, 1<<7);
    }

}

void MHal_PWS_SetAdcPhdig(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102509, 1, 1<<0);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102509, 0, 1<<0);
    }

}

void MHal_PWS_SetAdcPllA(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102509, 1, 1<<1);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102509, 0, 1<<1);
    }

}

void MHal_PWS_SetAdcIclampRgb(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102509, 1, 1<<2);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102509, 0, 1<<2);
    }

}

void MHal_PWS_SetAdcIclampVdy(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102509, 1, 1<<3);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102509, 0, 1<<3);
    }

}

void MHal_PWS_SetAdcIclampVdc(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102509, 1, 1<<4);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102509, 0, 1<<4);
    }

}

void MHal_PWS_SetAdcY(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

#if 0
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102509, 1, 1<<6);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102509, 0, 1<<6);
    }
#endif
    MHal_PWS_WriteRegBit(0x102509, 0, 1<<6);
}

void MHal_PWS_SetAdcPllB(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));
#if 0
    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250A, 1, 1<<1);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250A, 0, 1<<1);
    }
#endif
    MHal_PWS_WriteRegBit(0x10250A, 0, 1<<1);
}

void MHal_PWS_SetAdcSog(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250A, 1, 1<<2);
        MHal_PWS_WriteRegBit(0x10250A, 1, 1<<3);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250A, 0, 1<<2);
        MHal_PWS_WriteRegBit(0x10250A, 0, 1<<3);
    }

}

void MHal_PWS_SetAdcSogOff(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250A, 1, 1<<4);
        MHal_PWS_WriteRegBit(0x10250A, 1, 1<<5);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250A, 0, 1<<4);
        MHal_PWS_WriteRegBit(0x10250A, 0, 1<<5);
    }

}

void MHal_PWS_SetAdcSogUnused(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250A, 1, 1<<6);
        MHal_PWS_WriteRegBit(0x10250A, 1, 1<<7);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250A, 0, 1<<6);
        MHal_PWS_WriteRegBit(0x10250A, 0, 1<<7);
    }

}

void MHal_PWS_SetAdcHsync0(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250B, 1, 1<<0);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250B, 0, 1<<0);
    }

}

void MHal_PWS_SetAdcHSync1(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250B, 1, 1<<1);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250B, 0, 1<<1);
    }

}

void MHal_PWS_SetAdcHsync2(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250B, 1, 1<<2);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250B, 0, 1<<2);
    }

}

void MHal_PWS_SetAdcPdClk200(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 1, 1<<1);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 0, 1<<1);
    }

}

void MHal_PWS_SetAdcPdClk400(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 1, 1<<2);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 0, 1<<2);
    }

}

void MHal_PWS_SetAdcPdClkPll(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 1, 1<<3);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 0, 1<<3);
    }

}

void MHal_PWS_SetAdcPdClkR(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 1, 1<<4);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 0, 1<<4);
    }

}

void MHal_PWS_SetAdcPdClkG(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 1, 1<<5);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 0, 1<<5);
    }

}

void MHal_PWS_SetAdcPdClkB(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 1, 1<<6);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 0, 1<<6);
    }

}

void MHal_PWS_SetAdcPdClkY(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 1, 1<<7);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250C, 0, 1<<7);
    }

}

void MHal_PWS_SetAdcPdClkVd(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250D, 1, 1<<4);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250D, 0, 1<<4);
    }

}

void MHal_PWS_SetAdcPdClk200Fb(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10250D, 1, 1<<6);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10250D, 0, 1<<6);
    }

}

void MHal_PWS_SetAdcSogMux(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x10257E, 1, 1<<0);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x10257E, 0, 1<<0);
    }

}

void MHal_PWS_SetAdcFbAdc(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102580, 1, 1<<6);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102580, 0, 1<<6);
    }

}

void MHal_PWS_SetAdcCvbsLpfY(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102570, 0, 1<<0);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102570, 1, 1<<0);
    }

}

void MHal_PWS_SetAdcCvbsLpfC(U8 OnOff_flag)
{
    MHAL_DEBUG_INFO(printf("%s\n",__FUNCTION__));

    if(OnOff_flag)
    {
        MHAL_DEBUG_INFO(printf("_OFF_\n"));
        MHal_PWS_WriteRegBit(0x102570, 0, 1<<1);
    }
    else
    {
        MHAL_DEBUG_INFO(printf("_ON_\n"));
        MHal_PWS_WriteRegBit(0x102570, 1, 1<<1);
    }
}

#define INPUT_DVDD_NUM 0x0003

U32 MHal_PWS_Get_DVDD(void)
{
    U32 u32Dvdd;
    u32Dvdd = (*((volatile unsigned int*)(0xBF000000 + (0x3c00+0x67*4))))>>8;
    return u32Dvdd;
}

void MHal_PWS_Set_DVDD(void)
{
    if(MHal_PWS_Get_DVDD() < INPUT_DVDD_NUM)
    {
        udelay(100);
        *(volatile unsigned int*)(0xBF200000 + (0x4A00 + 0x30*4)) &= ~0x800;
        *(volatile unsigned int*)(0xBF200000 + (0x4A00 + 0x11*4)) |= 0x0020;
        *(volatile unsigned int*)(0xBF200000 + (0x4A00 + 0x30*4)) |= 0x800;
        udelay(100);
        *(volatile unsigned int*)(0xBF200000 + (0x4A00 + 0x30*4)) &= ~0x800;
    }
}

