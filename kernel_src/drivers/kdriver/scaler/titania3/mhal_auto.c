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
//
////////////////////////////////////////////////////////////////////////////////

#include <asm/io.h>
#include "mst_utility.h"
#include "mdrv_types.h"
#include "mdrv_auto.h"
#include "mhal_scaler_reg.h"
#include "mhal_scaler.h"
#include "mhal_utility.h"
#include "mhal_auto.h"
#include "mhal_adc_reg.h"       //victor 20080827
#include "mhal_adc.h"

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures Start
//-------------------------------------------------------------------------------------------------
extern unsigned long irq_flags_swbk;
extern spinlock_t switch_bk_lock;
extern U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------

#define AUTOMSG(x)  //x

void MHal_Auto_SetValidThreadhold(U8 u8Value) // 20091107 daniel.huang: refine code
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    REG_WM(REG_SC_IP1F2(0x10), (((U16)u8Value) << 8), 0xF000);
    SC_BK_RESTORE;
}

//*************************************************************************
//Function name: MHal_Auto_WaitStatusReady
//Passing parameter:
//  U8 u8RegIndex: Register index
//  U8 u8RegMask: Status mask
//Return parameter: NO
//Description: Wait for status ready.
//*************************************************************************
void MHal_Auto_WaitStatusReady(U8 u8RegIndex, U8 u8RegMask)
{
    U16 u16Dummy = 1000; // loop dummy

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    //while(!(MDrv_ReadByte(u16RegIndex) & u8RegMask) && (u16Dummy--)) ;
    while (!(REG_RR(REG_SC_IP1F2(u8RegIndex)) & u8RegMask) && (u16Dummy--)) ;
    SC_BK_RESTORE;
}

U16 MHal_Auto_GetAutoPosition(U8 u8RegIndex)
{
    U16 AutoPos;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    AutoPos = REG_RR(REG_SC_IP1F2(u8RegIndex)) & 0x0FFF;
    SC_BK_RESTORE;
    return AutoPos;
}

//*************************************************************************
//Function name: MHal_Auto_GetPhaseVal
//Passing parameter: NO
//Return parameter:
//  U32: Return full image sun of difference value between two pixles
//Description: Get auto phase value.
//*************************************************************************
U32 MHal_Auto_GetPhaseVal(void)
{
    U32 PhaseValBff = 0; // double word buffer

    MHal_Auto_WaitStatusReady(0x19, BIT1);

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    PhaseValBff  |= REG_RR(REG_SC_IP1F2(0x1B));
    PhaseValBff <<= 16;
    PhaseValBff  |= REG_RR(REG_SC_IP1F2(0x1A));
    SC_BK_RESTORE;

    return PhaseValBff;
}

void MHal_Auto_IPMUX_480p_Timing( void )
{
    REG_WR (REG_IPMUX(0x12), 0x2d0) ;  //HDE
    REG_WL (REG_IPMUX(0x13), 0x10) ;   //H front porch
    REG_WL (REG_IPMUX(0x14), 0x3e) ;   //H sync pulse width
    REG_WR (REG_IPMUX(0x15), 0x35a) ;  //Htotal
    REG_WR (REG_IPMUX(0x16), 0x1e0) ;  //VDE
    REG_WL (REG_IPMUX(0x17), 0x06) ;    //V front porch
    REG_WL (REG_IPMUX(0x18), 0x09) ;    //V sync pulse width
    REG_WR (REG_IPMUX(0x19), 0x20d) ;  //Vtotal

    REG_WI (REG_IPMUX(0x10), 0x1, BIT15 );
    REG_WI (REG_IPMUX(0x10), 0x1, BIT0 );//output timing enable
    // move from MHal_SC_IP_Setting
    REG_WM( REG_IPMUX(0x01) , 0xF0, 0xF0 );  //select pattern generator source
}

void MHal_Auto_IPMUX_720p_Timing()
{
    REG_WR (REG_IPMUX(0x12), 0x500) ;  //HDE
    REG_WL (REG_IPMUX(0x13), 0x6E) ;   //H front porch
    REG_WL (REG_IPMUX(0x14), 0x28) ;   //H sync pulse width
    REG_WR (REG_IPMUX(0x15), 0x690) ;  //Htotal
    REG_WR (REG_IPMUX(0x16), 0x2D0) ;  //VDE
    REG_WL (REG_IPMUX(0x17), 0x05) ;    //V front porch
    REG_WL (REG_IPMUX(0x18), 0x05) ;    //V sync pulse width
    REG_WR (REG_IPMUX(0x19), 0x2EE) ;  //Vtotal

    REG_WI (REG_IPMUX(0x10), 0x1, BIT15 );
    REG_WI (REG_IPMUX(0x10), 0x1, BIT0 );//output timing enable
    // move from MHal_SC_IP_Setting
    REG_WM( REG_IPMUX(0x01) , 0xF0, 0xF0 );  //select pattern generator source
}

void MHal_Auto_Save_Source(AUTO_SRC_INFO_t *pinfo)
{
    pinfo->u16ATOP_00 = REG_RR(REG_ADC_ATOP(0x00));
    pinfo->u16ATOP_01 = REG_RR(REG_ADC_ATOP(0x01));
    pinfo->u16ATOP_04 = REG_RR(REG_ADC_ATOP(0x04));
    pinfo->u16ATOP_05 = REG_RR(REG_ADC_ATOP(0x05));
    pinfo->u16ATOP_06 = REG_RR(REG_ADC_ATOP(0x06));
    pinfo->u16ATOP_09 = REG_RR(REG_ADC_ATOP(0x09));
    pinfo->u16ATOP_0A = REG_RR(REG_ADC_ATOP(0x0A));
    pinfo->u16ATOP_0C = REG_RR(REG_ADC_ATOP(0x0C));
    pinfo->u16ATOP_1C = REG_RR(REG_ADC_ATOP(0x1C));
    pinfo->u16ATOP_1F = REG_RR(REG_ADC_ATOP(0x1F));
    pinfo->u16ATOP_34 = REG_RR(REG_ADC_ATOP(0x34));
    pinfo->u16ATOP_35 = REG_RR(REG_ADC_ATOP(0x35));

    pinfo->u16DTOP_01 = REG_RR(REG_ADC_DTOP(0x01));
    pinfo->u16DTOP_02 = REG_RR(REG_ADC_DTOP(0x02));
    pinfo->u16DTOP_06 = REG_RR(REG_ADC_DTOP(0x06));
    pinfo->u16DTOP_19 = REG_RR(REG_ADC_ATOP(0x19));


    pinfo->u16DTOP_52= REG_RR(REG_ADC_DTOP(0x52));    //offset_r
    pinfo->u16DTOP_57= REG_RR(REG_ADC_DTOP(0x57));    //offset_g
    pinfo->u16DTOP_5C= REG_RR(REG_ADC_DTOP(0x5C));    //offset_b

    pinfo->u16DTOP_50= REG_RR(REG_ADC_DTOP(0x50));    //blacklevel_r
    pinfo->u16DTOP_55= REG_RR(REG_ADC_DTOP(0x55));    //blacklevel_g
    pinfo->u16DTOP_5A= REG_RR(REG_ADC_DTOP(0x5A));    //blacklevel_b



    pinfo->u16CHIPTOP_1F = REG_RR(REG_CKGEN0(0x1F));

    pinfo->u16IPMUX_12 = REG_RR(REG_IPMUX(0x12));
    pinfo->u16IPMUX_13 = REG_RR(REG_IPMUX(0x13));
    pinfo->u16IPMUX_14 = REG_RR(REG_IPMUX(0x14));
    pinfo->u16IPMUX_15 = REG_RR(REG_IPMUX(0x15));
    pinfo->u16IPMUX_16 = REG_RR(REG_IPMUX(0x16));
    pinfo->u16IPMUX_17 = REG_RR(REG_IPMUX(0x17));
    pinfo->u16IPMUX_18 = REG_RR(REG_IPMUX(0x18));
    pinfo->u16IPMUX_19 = REG_RR(REG_IPMUX(0x19));
    pinfo->u16IPMUX_10 = REG_RR(REG_IPMUX(0x10));
    pinfo->u16IPMUX_01 = REG_RR(REG_IPMUX(0x01));
}


void MHal_Auto_Restore_Source(AUTO_SRC_INFO_t *pinfo)
{
    REG_WR(REG_ADC_ATOP(0x00), pinfo->u16ATOP_00);
    REG_WR(REG_ADC_ATOP(0x01), pinfo->u16ATOP_01);
    REG_WR(REG_ADC_ATOP(0x04), pinfo->u16ATOP_04);
    REG_WR(REG_ADC_ATOP(0x05), pinfo->u16ATOP_05);
    REG_WR(REG_ADC_ATOP(0x06), pinfo->u16ATOP_06);
    REG_WR(REG_ADC_ATOP(0x09), pinfo->u16ATOP_09);
    REG_WR(REG_ADC_ATOP(0x0A), pinfo->u16ATOP_0A);
    REG_WR(REG_ADC_ATOP(0x0C), pinfo->u16ATOP_0C);
    REG_WR(REG_ADC_ATOP(0x1C), pinfo->u16ATOP_1C);
    REG_WR(REG_ADC_ATOP(0x1F), pinfo->u16ATOP_1F);
    REG_WR(REG_ADC_ATOP(0x34), pinfo->u16ATOP_34);
    REG_WR(REG_ADC_ATOP(0x35), pinfo->u16ATOP_35);

    REG_WR(REG_ADC_DTOP(0x01), pinfo->u16DTOP_01);
    REG_WR(REG_ADC_DTOP(0x02), pinfo->u16DTOP_02);
    REG_WR(REG_ADC_DTOP(0x06), pinfo->u16DTOP_06);
    REG_WR(REG_ADC_ATOP(0x19), pinfo->u16DTOP_19);


    REG_WR(REG_ADC_DTOP(0x52), pinfo->u16DTOP_52);    //offset_r
    REG_WR(REG_ADC_DTOP(0x57), pinfo->u16DTOP_57);    //offset_g
    REG_WR(REG_ADC_DTOP(0x5C), pinfo->u16DTOP_5C);    //offset_b

    REG_WR(REG_ADC_DTOP(0x50), pinfo->u16DTOP_50);    //blacklevel_r
    REG_WR(REG_ADC_DTOP(0x55), pinfo->u16DTOP_55);    //blacklevel_g
    REG_WR(REG_ADC_DTOP(0x5A), pinfo->u16DTOP_5A);    //blacklevel_b


    REG_WR(REG_CKGEN0(0x1F), pinfo->u16CHIPTOP_1F);

    REG_WR(REG_IPMUX(0x12), pinfo->u16IPMUX_12);
    REG_WR(REG_IPMUX(0x13), pinfo->u16IPMUX_13);
    REG_WR(REG_IPMUX(0x14), pinfo->u16IPMUX_14);
    REG_WR(REG_IPMUX(0x15), pinfo->u16IPMUX_15);
    REG_WR(REG_IPMUX(0x16), pinfo->u16IPMUX_16);
    REG_WR(REG_IPMUX(0x17), pinfo->u16IPMUX_17);
    REG_WR(REG_IPMUX(0x18), pinfo->u16IPMUX_18);
    REG_WR(REG_IPMUX(0x19), pinfo->u16IPMUX_19);
    REG_WR(REG_IPMUX(0x10), pinfo->u16IPMUX_10);
    REG_WR(REG_IPMUX(0x01), pinfo->u16IPMUX_01);
}

void MHal_Auto_Save_Timing(AUTO_TIMING_INFO_t *pinfo)
{
    if (!pinfo)
    {
        return;
    }
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    pinfo->u16SC_BK01_02 = REG_RR(REG_SC_IP1F2(0x02));
    pinfo->u16SC_BK01_03 = REG_RR(REG_SC_IP1F2(0x03));
    pinfo->u16SC_BK01_04 = REG_RR(REG_SC_IP1F2(0x04));
    pinfo->u16SC_BK01_05 = REG_RR(REG_SC_IP1F2(0x05));
    pinfo->u16SC_BK01_06 = REG_RR(REG_SC_IP1F2(0x06));
    pinfo->u16SC_BK01_07 = REG_RR(REG_SC_IP1F2(0x07));
    pinfo->u16SC_BK01_21 = REG_RR(REG_SC_IP1F2(0x21));

    SC_BK_SWICH(REG_SC_BK_IP2F2);
    pinfo->u32SC_BK02_04 = (U32)REG_RR(REG_SC_IP2F2(0x04)) | ((U32)REG_RR(REG_SC_IP2F2(0x05)) << 16);
    pinfo->u32SC_BK02_08 = (U32)REG_RR(REG_SC_IP2F2(0x08)) | ((U32)REG_RR(REG_SC_IP2F2(0x09)) << 16);
    pinfo->u16SC_BK02_40 = REG_RR(REG_SC_IP2F2(0x40));

    SC_BK_SWICH(REG_SC_BK_DNR);
    pinfo->u16SC_BK06_21 = REG_RR(REG_SC_DNR(0x21));

    SC_BK_SWICH(REG_SC_BK_VOP);
    pinfo->u16SC_BK10_2F      = REG_RR(REG_SC_VOP(0x2F));

    SC_BK_SWICH(REG_SC_BK_ACE);
    pinfo->u16SC_BK18_6E  = REG_RR(REG_SC_ACE(0x6E));

    SC_BK_SWICH(REG_SC_BK_PIP);
    pinfo->u16SC_BK20_15 = REG_RR(REG_SC_PIP(0x15));
    pinfo->u16SC_BK20_1D = REG_RR(REG_SC_PIP(0x1D));

    SC_BK_SWICH(REG_SC_BK_HVSP);
    pinfo->u16SC_BK23_0C  = REG_RR(REG_SC_BANK(0x0C));
    pinfo->u32SC_BK23_07 = (U32)REG_RR(REG_SC_HVSP(0x07)) | ((U32)REG_RR(REG_SC_HVSP(0x08)) << 16);
    pinfo->u32SC_BK23_09 = (U32)REG_RR(REG_SC_HVSP(0x09)) | ((U32)REG_RR(REG_SC_HVSP(0x0A)) << 16);

    SC_BK_SWICH(REG_SC_BK_SCMI);
    pinfo->u32SC_BK12_08 = (U32)REG_RR(REG_SC_SCMI(0x08)) | ((U32)REG_RR(REG_SC_SCMI(0x09)) << 16);  // IPMBase0
    pinfo->u32SC_BK12_0A = (U32)REG_RR(REG_SC_SCMI(0x0A)) | ((U32)REG_RR(REG_SC_SCMI(0x0B)) << 16);  // IPMBase1
    pinfo->u32SC_BK12_0C = (U32)REG_RR(REG_SC_SCMI(0x0C)) | ((U32)REG_RR(REG_SC_SCMI(0x0D)) << 16);  // IPMBase2
    pinfo->u16SC_BK12_0E = REG_RR(REG_SC_SCMI(0x0E));
    pinfo->u16SC_BK12_0F = REG_RR(REG_SC_SCMI(0x0F));
    pinfo->u32SC_BK12_10 = (U32)REG_RR(REG_SC_SCMI(0x10)) | ((U32)REG_RR(REG_SC_SCMI(0x11)) << 16);
    pinfo->u32SC_BK12_12 = (U32)REG_RR(REG_SC_SCMI(0x12)) | ((U32)REG_RR(REG_SC_SCMI(0x13)) << 16);
    pinfo->u32SC_BK12_14 = (U32)REG_RR(REG_SC_SCMI(0x14)) | ((U32)REG_RR(REG_SC_SCMI(0x15)) << 16);
    pinfo->u16SC_BK12_16 = REG_RR(REG_SC_SCMI(0x16));
    pinfo->u16SC_BK12_17 = REG_RR(REG_SC_SCMI(0x17));
    pinfo->u16SC_BK12_18 = REG_RR(REG_SC_SCMI(0x18));
    pinfo->u16SC_BK12_01 = REG_RR(REG_SC_SCMI(0x01));
    pinfo->u16SC_BK12_02 = REG_RR(REG_SC_SCMI(0x02));
    pinfo->u16SC_BK12_03 = REG_RR(REG_SC_SCMI(0x03));
    pinfo->u16SC_BK12_04 = REG_RR(REG_SC_SCMI(0x04));  // 3 frame mode [7]  // 2009/09/23 daniel.huang: for calibration with mirror
    pinfo->u16SC_BK12_07 = REG_RR(REG_SC_SCMI(0x07));  // 4 frame mode [13] // 2009/09/23 daniel.huang: for calibration with mirror
    pinfo->u32SC_BK12_1A = (U32)REG_RR(REG_SC_SCMI(0x1A)) | ((U32)REG_RR(REG_SC_SCMI(0x1B)) << 16); // 2009/09/23 daniel.huang: for calibration with mirror

    SC_BK_RESTORE;
}

void MHal_Auto_Restore_Timing(AUTO_TIMING_INFO_t *pinfo)
{
    if (!pinfo)
    {
        return;
    }
    SC_BK_STORE;
    // 20091030: restore input source disable status setting
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    REG_WI(REG_SC_IP1F2(0x02), TRUE, BIT7);     // input source disable

    MHal_SC_Reset(SC_RST_IP_F2);
    REG_WR(REG_SC_IP1F2(0x03), pinfo->u16SC_BK01_03);
    REG_WR(REG_SC_IP1F2(0x04), pinfo->u16SC_BK01_04);
    REG_WR(REG_SC_IP1F2(0x05), pinfo->u16SC_BK01_05);
    REG_WR(REG_SC_IP1F2(0x06), pinfo->u16SC_BK01_06);
    REG_WR(REG_SC_IP1F2(0x07), pinfo->u16SC_BK01_07);
    REG_WM(REG_SC_IP1F2(0x21), pinfo->u16SC_BK01_21, 0x03);

    SC_BK_SWICH(REG_SC_BK_IP2F2);
    REG_W4(REG_SC_IP2F2(0x04), pinfo->u32SC_BK02_04);
    REG_W4(REG_SC_IP2F2(0x08), pinfo->u32SC_BK02_08);
    REG_WM(REG_SC_IP2F2(0x40), pinfo->u16SC_BK02_40, BIT3);

    SC_BK_SWICH(REG_SC_BK_DNR);
    REG_WM(REG_SC_DNR(0x21), pinfo->u16SC_BK06_21, BIT0);

    SC_BK_SWICH(REG_SC_BK_VOP);
    REG_WM(REG_SC_VOP(0x2F), pinfo->u16SC_BK10_2F, 0xFF);

    SC_BK_SWICH(REG_SC_BK_ACE);
    REG_WM(REG_SC_ACE(0x6E), pinfo->u16SC_BK18_6E, BIT0);

    SC_BK_SWICH(REG_SC_BK_PIP);
    REG_WR(REG_SC_PIP(0x15), pinfo->u16SC_BK20_15);
    REG_WR(REG_SC_PIP(0x1D), pinfo->u16SC_BK20_1D);

    SC_BK_SWICH(REG_SC_BK_HVSP);
    REG_WM(REG_SC_HVSP(0x0C), pinfo->u16SC_BK23_0C, BIT7);
    REG_W4(REG_SC_HVSP(0x07), pinfo->u32SC_BK23_07);
    REG_W4(REG_SC_HVSP(0x09), pinfo->u32SC_BK23_09);

    SC_BK_SWICH(REG_SC_BK_SCMI);
    REG_W4(REG_SC_SCMI(0x08), pinfo->u32SC_BK12_08);  // IPMBase0
    REG_W4(REG_SC_SCMI(0x0A), pinfo->u32SC_BK12_0A);  // IPMBase1
    REG_W4(REG_SC_SCMI(0x0C), pinfo->u32SC_BK12_0C);  // IPMBase2
    REG_WR(REG_SC_SCMI(0x0E), pinfo->u16SC_BK12_0E);
    REG_WR(REG_SC_SCMI(0x0F), pinfo->u16SC_BK12_0F);
    REG_W4(REG_SC_SCMI(0x10), pinfo->u32SC_BK12_10);
    REG_W4(REG_SC_SCMI(0x12), pinfo->u32SC_BK12_12);
    REG_W4(REG_SC_SCMI(0x14), pinfo->u32SC_BK12_14);
    REG_WR(REG_SC_SCMI(0x16), pinfo->u16SC_BK12_16);
    REG_WR(REG_SC_SCMI(0x17), pinfo->u16SC_BK12_17);
    REG_WR(REG_SC_SCMI(0x18), pinfo->u16SC_BK12_18);
    REG_WR(REG_SC_SCMI(0x01), pinfo->u16SC_BK12_01);

    REG_WM(REG_SC_SCMI(0x02), pinfo->u16SC_BK12_02, BITMASK(10:0));
    REG_WM(REG_SC_SCMI(0x03), pinfo->u16SC_BK12_03, (BIT13|BIT12|BIT5|BIT4));   // 2009/09/23 daniel.huang: for calibration with mirror
    REG_WR(REG_SC_SCMI(0x04), pinfo->u16SC_BK12_04);  // 3 frame mode [7]       // 2009/09/23 daniel.huang: for calibration with mirror
    REG_WR(REG_SC_SCMI(0x07), pinfo->u16SC_BK12_07);  // 4 frame mode [13]      // 2009/09/23 daniel.huang: for calibration with mirror
    REG_W4(REG_SC_SCMI(0x1A), pinfo->u32SC_BK12_1A);                            // 2009/09/23 daniel.huang: for calibration with mirror

    // 20091030: restore input source disable status setting
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    REG_WR(REG_SC_IP1F2(0x02), pinfo->u16SC_BK01_02);

    SC_BK_RESTORE;
}

void MHal_Auto_SC_480p_Setting(U32 u32DNRMemAddr, U32 u32DNRMemSize)    // 2009/09/23 daniel.huang: for calibration with mirror
{
    AUTO_TIMING_INFO_t info;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_SCMI);

    // REG_SC_BK_IP1F2
    info.u16SC_BK01_02 = 0x104;     // video source sel
    info.u16SC_BK01_03 = 0x890;     // sync/de settings
    info.u16SC_BK01_04 = 0x20;      // V_CapStart
    info.u16SC_BK01_05 = 0x50;      // H_CapStart
    info.u16SC_BK01_06 = 0x1C0;    // V_CapSize
    info.u16SC_BK01_07 = 0x280;    // H_CapSize
    info.u16SC_BK01_21 = 0x1;       // interlace md

    //REG_SC_BK_IP2F2
    info.u32SC_BK02_04 = 0x0;       // HSDRatio
    info.u32SC_BK02_08 = 0x0;       // VSDRatio
    info.u16SC_BK02_40 = 0x0;       // IP2_CSC

    //REG_SC_DNR
    info.u16SC_BK06_21 = 0x0;       // [0]: dnr_en

    //REG_SC_BK_VOP
    info.u16SC_BK10_2F = 0x0;       // [6][4][2][1][0]: color matrix control

    //REG_SC_BK_ACE
    info.u16SC_BK18_6E = 0x0;       // [0]: vip csc en

    //REG_SC_BK_PIP
    info.u16SC_BK20_15 = 0x1C0;     // V_Length
    info.u16SC_BK20_1D = 0x0;       // LBoffset

    //REG_SC_BK_HVSP
    info.u16SC_BK23_0C = 0x0;       // [7]: reg_format_422_f2
    info.u32SC_BK23_07 = 0x0;       // HSPRatio
    info.u32SC_BK23_09 = 0x0;       // VSPRatio

    //REG_SC_BK_SCMI
    info.u32SC_BK12_08 = u32DNRMemAddr/BYTE_PER_WORD;                   // IPMBase0 // 2009/09/23 daniel.huang: for calibration with mirror
    info.u32SC_BK12_0A = (u32DNRMemAddr+u32DNRMemSize/2)/BYTE_PER_WORD; // IPMBase1 // 2009/09/23 daniel.huang: for calibration with mirror
    info.u32SC_BK12_0C = 0;                                             // IPMBase2 // 2009/09/23 daniel.huang: for calibration with mirror
    info.u16SC_BK12_0E = 0x280;     // IPMOffset
    info.u16SC_BK12_0F = 0x280;     // IPMFetch
    info.u32SC_BK12_10 = info.u32SC_BK12_08; // OPMBase0
    info.u32SC_BK12_12 = info.u32SC_BK12_0A; // OPMBase1
    info.u32SC_BK12_14 = info.u32SC_BK12_0C; // OPMBase2
    info.u16SC_BK12_16 = 0x280;     // OPMOffset
    info.u16SC_BK12_17 = 0x280;     // OPMFetch
    info.u16SC_BK12_18 = BIT12|0x1C0;   // VCnt limit
    info.u16SC_BK12_01 = 0x0120;    // MEMFmt: P mode 10bit, 444
    info.u16SC_BK12_02 = 0x0400;    // disable user memory format[9:0]=0; enable YC separate [10]=1
    info.u16SC_BK12_03 = (REG_RR(REG_SC_SCMI(0x03))|BIT5|BIT4) & ~(BIT13|BIT12); // F2 IP/OP linear address enable[5:4], cleare h/v mirror [13:12] // 2009/09/23 daniel.huang: for calibration with mirror
    info.u16SC_BK12_04 = REG_RR(REG_SC_SCMI(0x04)) & ~BIT7;     // clear 3 frame mode [7]       // 2009/09/23 daniel.huang: for calibration with mirror
    info.u16SC_BK12_07 = REG_RR(REG_SC_SCMI(0x07)) & ~BIT13;    // clear 4 frame mode [13]      // 2009/09/23 daniel.huang: for calibration with mirror
    info.u32SC_BK12_1A = (((u32DNRMemAddr+u32DNRMemSize)/BYTE_PER_WORD-1) | 0x02000000);        // 2009/09/23 daniel.huang: for calibration with mirror
    SC_BK_RESTORE;

    MHal_Auto_Restore_Timing(&info);
    //while(1);
}

void MHal_Auto_SC_720p_Setting(U32 u32DNRMemAddr, U32 u32DNRMemSize)    // 2009/09/23 daniel.huang: for calibration with mirror
{
    AUTO_TIMING_INFO_t info;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_SCMI);

    // REG_SC_BK_IP1F2
    info.u16SC_BK01_02 = 0x104;     // video source sel
    info.u16SC_BK01_03 = 0x890;     // sync/de settings
    info.u16SC_BK01_04 = 0x20;      // V_CapStart
    info.u16SC_BK01_05 = 0x50;      // H_CapStart
    info.u16SC_BK01_06 = 0x290;    // V_CapSize
    info.u16SC_BK01_07 = 0x460;    // H_CapSize
    info.u16SC_BK01_21 = 0x1;       // interlace md

    //REG_SC_BK_IP2F2
    info.u32SC_BK02_04 = 0x0;       // HSDRatio
    info.u32SC_BK02_08 = 0x0;       // VSDRatio
    info.u16SC_BK02_40 = 0x0;       // IP2_CSC

    //REG_SC_DNR
    info.u16SC_BK06_21 = 0x0;       // [0]: dnr_en

    //REG_SC_BK_VOP
    info.u16SC_BK10_2F = 0x0;       // [6][4][2][1][0]: color matrix control

    //REG_SC_BK_ACE
    info.u16SC_BK18_6E = 0x0;       // [0]: vip csc en

    //REG_SC_BK_PIP
    info.u16SC_BK20_15 = 0x290;     // V_Length
    info.u16SC_BK20_1D = 0x0;       // LBoffset

    //REG_SC_BK_HVSP
    info.u16SC_BK23_0C = 0x0;       // [7]: reg_format_422_f2
    info.u32SC_BK23_07 = 0x0;       // HSPRatio
    info.u32SC_BK23_09 = 0x0;       // VSPRatio

    //REG_SC_BK_SCMI
    info.u32SC_BK12_08 = u32DNRMemAddr/BYTE_PER_WORD;                   // IPMBase0 // 2009/09/23 daniel.huang: for calibration with mirror
    info.u32SC_BK12_0A = (u32DNRMemAddr+u32DNRMemSize/2)/BYTE_PER_WORD; // IPMBase1 // 2009/09/23 daniel.huang: for calibration with mirror
    info.u32SC_BK12_0C = 0;                                             // IPMBase2 // 2009/09/23 daniel.huang: for calibration with mirror
    info.u16SC_BK12_0E = 0x460;     // IPMOffset
    info.u16SC_BK12_0F = 0x460;     // IPMFetch
    info.u32SC_BK12_10 = info.u32SC_BK12_08; // OPMBase0
    info.u32SC_BK12_12 = info.u32SC_BK12_0A; // OPMBase1
    info.u32SC_BK12_14 = info.u32SC_BK12_0C; // OPMBase2
    info.u16SC_BK12_16 = 0x460;     // OPMOffset
    info.u16SC_BK12_17 = 0x460;     // OPMFetch
    info.u16SC_BK12_18 = BIT12|0x290;   // VCnt limit
    info.u16SC_BK12_01 = 0x0120;    // MEMFmt: P mode 10bit, 444
    info.u16SC_BK12_02 = 0x0400;    // disable user memory format[9:0]=0; enable YC separate [10]=1
    info.u16SC_BK12_03 = (REG_RR(REG_SC_SCMI(0x03))|BIT5|BIT4) & ~(BIT13|BIT12); // F2 IP/OP linear address enable[5:4], cleare h/v mirror [13:12] // 2009/09/23 daniel.huang: for calibration with mirror
    info.u16SC_BK12_04 = REG_RR(REG_SC_SCMI(0x04)) & ~BIT7;     // clear 3 frame mode [7]       // 2009/09/23 daniel.huang: for calibration with mirror
    info.u16SC_BK12_07 = REG_RR(REG_SC_SCMI(0x07)) & ~BIT13;    // clear 4 frame mode [13]      // 2009/09/23 daniel.huang: for calibration with mirror
    info.u32SC_BK12_1A = (((u32DNRMemAddr+u32DNRMemSize)/BYTE_PER_WORD-1) | 0x02000000);        // 2009/09/23 daniel.huang: for calibration with mirror
    SC_BK_RESTORE;

    MHal_Auto_Restore_Timing(&info);
}

// 20090814 daniel.huang: for extern calibration scaler settings
void MHal_Auto_SC_Ext_Setting(U32 u32DNRMemAddr, U32 u32DNRMemSize, BOOL bInterlace)    // 2009/09/23 daniel.huang: for calibration with mirror
{
    AUTO_TIMING_INFO_t info;
    U16 u16FrameLineCnt = MHal_SC_IP2_GetFrameLineCnt();  // 20091111 daniel.huang: fix lock twice with MHal_SC_IP2_GetFrameLineCnt() in SMP

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_HVSP);

    AUTOMSG(printk("Ext Settings(interlace=%u)\n", bInterlace));

    info.u16SC_BK06_21 = 0x0;       // [0]: dnr_en

    info.u16SC_BK23_0C = 0x0;       // [7]: reg_format_422_f2

    // VSPRatio
    info.u32SC_BK23_09 = ((U32)REG_RR(REG_SC_HVSP(0x09))) | ((U32)REG_RR(REG_SC_HVSP(0x0A)) << 16);
    if (bInterlace)     // scaling twice in v direction
        info.u32SC_BK23_09 = (info.u32SC_BK23_09 - (1<<24))/2 + (1<<24);

    // set opm base0/1/2 = ipm base0
    SC_BK_SWICH(REG_SC_BK_SCMI);
    if (REG_RR(REG_SC_SCMI(0x03)) & BIT13)  // if v mirror
    {

        if (bInterlace) // interlace input
            u16FrameLineCnt/=2;
        info.u32SC_BK12_08 = (u32DNRMemAddr + (u16FrameLineCnt - 1) * REG_RR(REG_SC_SCMI(0x0E)) * 4) / BYTE_PER_WORD;   // IPMBase0,  (frame_line_cnt -1) * line_offset * (N-bits/pix)/64-bits // 2009/09/23 daniel.huang: for calibration with mirror
        info.u32SC_BK12_0A = info.u32SC_BK12_08;            // IPMBase1
        info.u32SC_BK12_0C = info.u32SC_BK12_08;            // IPMBase2
        info.u32SC_BK12_10 = u32DNRMemAddr/BYTE_PER_WORD;   // OPMBase0 // 2009/09/23 daniel.huang: for calibration with mirror
        info.u32SC_BK12_12 = info.u32SC_BK12_10;            // OPMBase1
        info.u32SC_BK12_14 = info.u32SC_BK12_10;            // OPMBase2
        info.u32SC_BK12_1A = ((u32DNRMemAddr/BYTE_PER_WORD-1) | 0x03000000);        // 2009/09/23 daniel.huang: for calibration with mirror

    }
    else
    {
        info.u32SC_BK12_08 = u32DNRMemAddr/BYTE_PER_WORD;   // IPMBase0 // 2009/09/23 daniel.huang: for calibration with mirror
        info.u32SC_BK12_0A = info.u32SC_BK12_08; // IPMBase1
        info.u32SC_BK12_0C = info.u32SC_BK12_08; // IPMBase2
        info.u32SC_BK12_10 = info.u32SC_BK12_08; // OPMBase0
        info.u32SC_BK12_12 = info.u32SC_BK12_08; // OPMBase1
        info.u32SC_BK12_14 = info.u32SC_BK12_08; // OPMBase2
        info.u32SC_BK12_1A = (((u32DNRMemAddr+u32DNRMemSize)/BYTE_PER_WORD-1) | 0x02000000);        // 2009/09/23 daniel.huang: for calibration with mirror
    }

    info.u16SC_BK12_01 = 0x0120;    // MEMFmt: P mode 10bit, 444
    info.u16SC_BK12_02 = 0x0400;    // disable user memory format[9:0]=0; enable YC separate [10]=1
    info.u16SC_BK12_03 = REG_RR(REG_SC_SCMI(0x03))|BIT5|BIT4; // F2 IP/OP linear address enable[5:4], keep mirror mode  // 2009/09/23 daniel.huang: for calibration with mirror

    SC_BK_SWICH(REG_SC_BK_IP1F2);
    REG_WI(REG_SC_IP1F2(0x02), TRUE, BIT7);     // input source disable

    SC_BK_SWICH(REG_SC_BK_DNR);
    REG_WM(REG_SC_DNR(0x21), info.u16SC_BK06_21, BIT0);

    SC_BK_SWICH(REG_SC_BK_HVSP);
    REG_WM(REG_SC_HVSP(0x0C), info.u16SC_BK23_0C, BIT7);
    REG_W4(REG_SC_HVSP(0x09), info.u32SC_BK23_09);

    SC_BK_SWICH(REG_SC_BK_SCMI);
    REG_W4(REG_SC_SCMI(0x08), info.u32SC_BK12_08);  // IPMBase0
    REG_W4(REG_SC_SCMI(0x0A), info.u32SC_BK12_0A);  // IPMBase1
    REG_W4(REG_SC_SCMI(0x0C), info.u32SC_BK12_0C);  // IPMBase2
    REG_W4(REG_SC_SCMI(0x10), info.u32SC_BK12_10);
    REG_W4(REG_SC_SCMI(0x12), info.u32SC_BK12_12);
    REG_W4(REG_SC_SCMI(0x14), info.u32SC_BK12_14);
    REG_WR(REG_SC_SCMI(0x01), info.u16SC_BK12_01);
    REG_WM(REG_SC_SCMI(0x02), info.u16SC_BK12_02, BITMASK(10:0));
    REG_WM(REG_SC_SCMI(0x03), info.u16SC_BK12_03, (BIT13|BIT12|BIT5|BIT4)); // 2009/09/23 daniel.huang: for calibration with mirror

    SC_BK_SWICH(REG_SC_BK_IP1F2);
    REG_WI(REG_SC_IP1F2(0x02), FALSE, BIT7);     // input source enable

    SC_BK_RESTORE;

}

void MHal_Auto_ADC_Setting(void)
{   // daniel.huang: update according to Jerry.Chiu's suggestion in 20090603
    REG_WR(REG_ADC_ATOP(0x00), 0x0001);     // in MST_ADCSOURCE_TBL
    REG_WR(REG_ADC_ATOP(0x04), 0xF800);     // in MST_ADCSOURCE_TBL
    REG_WR(REG_ADC_ATOP(0x05), 0x0003);     // in MST_ADCSOURCE_TBL
    REG_WR(REG_ADC_ATOP(0x06), 0xFB00);     // in MST_ADCSOURCE_TBL

    REG_WM(REG_ADC_ATOP(0x09), 0x00, 0x18); // daniel +, [4]=0 disable CLKPLL output, [3]=0 analog testing signal select lcp
    REG_WR(REG_ADC_ATOP(0x1C), 0xF8);       // daniel +, turn on SOG input low bandwidth filter


    REG_WM(REG_ADC_DTOP(0x52), 0x800, 0x1FFF);    //offset_r
    REG_WM(REG_ADC_DTOP(0x57), 0x800, 0x1FFF);    //offset_g
    REG_WM(REG_ADC_DTOP(0x5C), 0x800, 0x1FFF);    //offset_b

    REG_WM(REG_ADC_DTOP(0x50), 0x800, 0x0FFF);    //blacklevel_r
    REG_WM(REG_ADC_DTOP(0x55), 0x800, 0x0FFF);    //blacklevel_g
    REG_WM(REG_ADC_DTOP(0x5A), 0x800, 0x0FFF);    //blacklevel_b

}

void MHal_Auto_ADC_720p_Timing(void)
{
    // michu
    REG_WM(REG_ADC_ATOP(0x01), 0x0f, 0x0f);
    REG_WM(REG_ADC_ATOP(0x1C), BIT5, BIT5);  /// turn off ADC a SoG comparator
    REG_WM(REG_ADC_ATOP(0x1F), BIT5, BIT5);  /// turn off ADC a SoG comparator

    // daniel.huang: update according to Jerry.Chiu's suggestion in 20090603
    REG_WM(REG_ADC_ATOP(0x0C), 0x1, 0x7);       // in MST_ADCSetMode_TBL

    REG_WR(REG_ADC_DTOP(0x01), 0xB82E);         // frequency control for test
    REG_WR(REG_ADC_DTOP(0x02), 0x0052);         // frequency control for test
    REG_WM(REG_ADC_DTOP(0x06), BIT7, BIT7);     // PLL digital test registers
}

void MHal_Auto_SetVIP_CSC_Enable(void)
{
    // BK02 0x40 L[3] CSC enable: off
    // BK18 0x6E L[0] VIP CSC enable: on

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP2F2);
    REG_WM(REG_SC_IP2F2(0x40), 0x0, BIT3);

    SC_BK_SWICH(0x18);
    REG_WM(REG_SC_BANK(0x6E), 0x1, BIT0);
    SC_BK_RESTORE;
}

