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

#ifndef __MHAL_AUTO_H__
#define __MHAL_AUTO_H__

typedef struct
{
    U16 u16ATOP_00;
    U16 u16ATOP_01;
    U16 u16ATOP_04;
    U16 u16ATOP_05;
    U16 u16ATOP_06;
    U16 u16ATOP_09;     // in MST_ADCSetMode_TBL
    U16 u16ATOP_0A;     // in MST_ADCSetMode_TBL
    U16 u16ATOP_0C;     // in MST_ADCSetMode_TBL
    U16 u16ATOP_1C;
    U16 u16ATOP_1F;
    U16 u16ATOP_34;     // in MST_ADCSetMode_TBL
    U16 u16ATOP_35;     // in MST_ADCSetMode_TBL

    U16 u16DTOP_01;
    U16 u16DTOP_02;
    U16 u16DTOP_06;
    U16 u16DTOP_19;     // in MST_ADCSetMode_TBL

    U16 u16DTOP_52;     // in MST_ADCSetMode_TBL, offset_r
    U16 u16DTOP_57;     // in MST_ADCSetMode_TBL, offset_g
    U16 u16DTOP_5C;     // in MST_ADCSetMode_TBL, offset_b

    U16 u16DTOP_50;     // in MST_ADCSetMode_TBL, blacklevel_r
    U16 u16DTOP_55;     // in MST_ADCSetMode_TBL, blacklevel_g
    U16 u16DTOP_5A;     // in MST_ADCSetMode_TBL, blacklevel_b


    U16 u16CHIPTOP_1F;

    U16 u16IPMUX_12;
    U16 u16IPMUX_13;    //H front porch
    U16 u16IPMUX_14;    //H sync pulse width
    U16 u16IPMUX_15;    //Htotal
    U16 u16IPMUX_16;    //VDE
    U16 u16IPMUX_17;    //V front porch
    U16 u16IPMUX_18;    //V sync pulse width
    U16 u16IPMUX_19;    //Vtotal
    U16 u16IPMUX_10;    // output timing enable
    U16 u16IPMUX_01;    //source sel
} AUTO_SRC_INFO_t;


typedef struct
{
    // REG_SC_BK_IP1F2
    U16 u16SC_BK01_02;  // video source sel
    U16 u16SC_BK01_03;  // sync/de settings
    U16 u16SC_BK01_04;  // H_CapStart
    U16 u16SC_BK01_05;  // H_CapSize
    U16 u16SC_BK01_06;  // V_CapStart
    U16 u16SC_BK01_07;  // V_CapSize
    U16 u16SC_BK01_21;  // interlace md

    //REG_SC_BK_IP2F2
    U32 u32SC_BK02_04;  // HSDRatio
    U32 u32SC_BK02_08;  // VSDRatio
    U16 u16SC_BK02_40;  // [3]: IP2_CSC

    //REG_SC_DNR
    U16 u16SC_BK06_21;  // [0]: dnr_en

    //REG_SC_BK_VOP
    U16 u16SC_BK10_2F;  // [6][4][2][1][0]: color matrix control

    //REG_SC_ACE2
    //U16 u16SC_BK27_70;  // [0]: vip all function bypass 1: bypass, 0: normal  // 20090925 daniel.huang: remove it for enabling vip csc

    //REG_SC_BK_VIP
    U16 u16SC_BK18_6E;  // [0]: vip csc en

    //REG_SC_BK_PIP
    U16 u16SC_BK20_15;    // V_Length
    U16 u16SC_BK20_1D;    // LBoffset

    //REG_SC_BK_HVSP
    U16 u16SC_BK23_0C;  // [7]: reg_format_422_f2
    U32 u32SC_BK23_07;  // HSPRatio
    U32 u32SC_BK23_09;  // VSPRatio

    //REG_SC_BK_SCMI
    U32 u32SC_BK12_08;  // IPMBase0
    U32 u32SC_BK12_0A;  // IPMBase1
    U32 u32SC_BK12_0C;  // IPMBase2
    U16 u16SC_BK12_0E;  // IPMOffset
    U16 u16SC_BK12_0F;  // IPMFetch
    U32 u32SC_BK12_10;  // OPMBase0
    U32 u32SC_BK12_12;  // OPMBase1
    U32 u32SC_BK12_14;  // OPMBase2
    U16 u16SC_BK12_16;  // OPMOffset
    U16 u16SC_BK12_17;  // OPMFetch
    U16 u16SC_BK12_18;  // VCnt limit
    U16 u16SC_BK12_01;  // MEMFmt
    U16 u16SC_BK12_02;  // user memory format [9:0], YC seperate [10]
    U16 u16SC_BK12_03;  // linear address enable [5:4]
    U16 u16SC_BK12_04;  // 3 frame mode [7]
    U16 u16SC_BK12_07;  // 4 frame mode [13]
    U32 u32SC_BK12_1A;  // write limit address      // 2009/09/23 daniel.huang: for calibration with mirror
} AUTO_TIMING_INFO_t;


void MHal_Auto_SetValidThreadhold(U8 u8Value); // 20091107 daniel.huang: refine code
void MHal_Auto_WaitStatusReady(U8 u8RegIndex, U8 u8RegMask);
U16  MHal_Auto_GetAutoPosition(U8 u8RegIndex);
U32  MHal_Auto_GetPhaseVal(void);


void MHal_Auto_Save_Source(AUTO_SRC_INFO_t *pinfo);
void MHal_Auto_Restore_Source(AUTO_SRC_INFO_t *pinfo);
void MHal_Auto_Save_Timing(AUTO_TIMING_INFO_t *pinfo);
void MHal_Auto_Restore_Timing(AUTO_TIMING_INFO_t *pinfo);
void MHal_Auto_IPMUX_480p_Timing(void);
void MHal_Auto_IPMUX_720p_Timing(void);
void MHal_Auto_SC_480p_Setting(U32 u32DNRMemAddr, U32 u32DNRMemSize);                   // 2009/09/23 daniel.huang: for calibration with mirror
void MHal_Auto_SC_720p_Setting(U32 u32DNRMemAddr, U32 u32DNRMemSize);                   // 2009/09/23 daniel.huang: for calibration with mirror
void MHal_Auto_SC_Ext_Setting(U32 u32DNRMemAddr, U32 u32DNRMemSize, BOOL bInterlace);   // 2009/09/23 daniel.huang: for calibration with mirror

void MHal_Auto_ADC_Setting(void);
void MHal_Auto_ADC_720p_Timing(void);
void MHal_Auto_SetVIP_CSC_Enable(void);


#endif//__MHAL_AUTO_H__
