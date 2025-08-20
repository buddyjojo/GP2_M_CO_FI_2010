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
#include "mhal_ve_reg.h"

#ifndef _MHAL_VE_TBL_H
#define _MHAL_VE_TBL_H
///////////////////////////////////////////////////////////////////////////////
/// @file   mhal_ve_tbl.h
/// This file contains the Mstar driver for TVEncoder
/// @author MStar Semiconductor Inc.
/// @brief  TVEncoder module
///////////////////////////////////////////////////////////////////////////////


#define _END_OF_TBL_                -1


//------------------------------------------------------------------------------
RegUnitType tVE_ENCODER_NTSC_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},
    {  _BK_VE_ENC(0x0000), 0x7F01}, // hsync end & hsync st
    {  _BK_VE_ENC(0x0001), 0xD794}, // burst end & burst st
    {  _BK_VE_ENC(0x0002), 0x0028},
    {  _BK_VE_ENC(0x0003), 0x0000},
    {  _BK_VE_ENC(0x0004), 0x4800}, // luma brightness offset
    {  _BK_VE_ENC(0x0006), 0x0000}, // pal switch en
    {  _BK_VE_ENC(0x0009), 0x06B4}, // H total

    {  _BK_VE_ENC(0x000A), 0x8000}, // bright
    {  _BK_VE_ENC(0x000B), 0x7c1F}, // burst phase step
    {  _BK_VE_ENC(0x000C), 0x21F0}, // burst phase step
    {  _BK_VE_ENC(0x000D), 0x00d0}, // lower stage fraction
    {  _BK_VE_ENC(0x000E), 0x0000}, // 625 stage fraction

    {  _BK_VE_ENC(0x0025), 0x0100}, // av st
    {  _BK_VE_ENC(0x0026), 0x068D}, // av end
    {  _BK_VE_ENC(0x0027), 0x102A}, // sync tip level & pad level
    {  _BK_VE_ENC(0x0028), 0x38F0}, // sync step & blank level
    {  _BK_VE_ENC(0x0029), 0x7013}, // burst amp & burst step
    {  _BK_VE_ENC(0x002A), 0x5B41}, // u,v sat. gain
    {  _BK_VE_ENC(0x002E), 0x0000},

    {  _BK_VE_ENC(0x78)|BK_LOW_BIT, 0x00},// disable MV


   // {  _BK_VE_ENC(0x003E), 0x00F9}, // start pixel number
   // {  _BK_VE_ENC(0x003F), 0x0417}, // do sync with TVE,start line number
   // {_REG_BASE_TBL, REG_BASE_VE_SOURCE},
    {  _BK_VE_SRC(0x0042), 0x01E0}, // Frame line number
    {  _BK_VE_SRC(0x0045), 0x5460}, // Field Size

    {_END_OF_TBL_, 0},
};


RegUnitType tVE_ENCODER_NTSC_443_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},
    {  _BK_VE_ENC(0x0000), 0x7F01}, // hsync end & hsync st
    {  _BK_VE_ENC(0x0001), 0xD18E}, // burst end & burst st
    {  _BK_VE_ENC(0x0002), 0x0000},
    {  _BK_VE_ENC(0x0003), 0x0000},
    {  _BK_VE_ENC(0x0004), 0x4C00}, // luma brightness offset
    //{  _BK_VE_ENC(0x0006), 0x0000}, // pal switch en
    {  _BK_VE_ENC(0x0009), 0x06B4}, // H total

    {  _BK_VE_ENC(0x000A), 0x8000}, // bright
    {  _BK_VE_ENC(0x000B), 0x7C1F}, // burst phase step
    {  _BK_VE_ENC(0x000C), 0x12F0}, // burst phase step
    {  _BK_VE_ENC(0x000D), 0x00D0}, // lower stage fraction
    {  _BK_VE_ENC(0x000E), 0x0000}, // 625 stage fraction

    {  _BK_VE_ENC(0x0025), 0x000F}, // av st
    {  _BK_VE_ENC(0x0026), 0x068D}, // av end
    {  _BK_VE_ENC(0x0027), 0x102A}, // sync tip level & pad level
    {  _BK_VE_ENC(0x0028), 0x38F0}, // sync step & blank level
    {  _BK_VE_ENC(0x0029), 0x700E}, // burst amp & burst step
    //{  _BK_VE_ENC(0x002A), 0x5B41}, // U,V sat. gain

    //{  _BK_VE_ENC(0x003E), 0x00f9}, // start pixel number
    //{  _BK_VE_ENC(0x003F), 0x0417}, // do sync with TVE,start line number

    //{_REG_BASE_TBL, REG_BASE_VE_SOURCE},
    {  _BK_VE_SRC(0x0042), 0x01E0}, // Frame line number
    {  _BK_VE_SRC(0x0045), 0x5460}, // Field Size

    {_END_OF_TBL_, 0},
};


RegUnitType tVE_ENCODER_NTSC_J_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},
    {  _BK_VE_ENC(0x0000), 0x7F01}, // hsync end & hsync st
    {  _BK_VE_ENC(0x0001), 0xD18E}, // burst end & burst st
    {  _BK_VE_ENC(0x0002), 0x0000},
    {  _BK_VE_ENC(0x0003), 0x0000},
    {  _BK_VE_ENC(0x0004), 0x8040}, // luma brightness offset
    //{  _BK_VE_ENC(0x0006), 0x0000}, // pal switch en
    {  _BK_VE_ENC(0x0009), 0x06B4}, // H total

    {  _BK_VE_ENC(0x000A), 0x8000}, // bright
    {  _BK_VE_ENC(0x000B), 0x7C1F}, // burst phase step
    {  _BK_VE_ENC(0x000C), 0x12F0}, // burst phase step
    {  _BK_VE_ENC(0x000D), 0x00D0}, // lower stage fraction
    {  _BK_VE_ENC(0x000E), 0x0000}, // 625 stage fraction

    {  _BK_VE_ENC(0x0025), 0x00FF}, // av st
    {  _BK_VE_ENC(0x0026), 0x068D}, // av end
    {  _BK_VE_ENC(0x0027), 0x1000}, // sync tip level & pad level
    {  _BK_VE_ENC(0x0028), 0x38F0}, // sync step & blank level
    {  _BK_VE_ENC(0x0029), 0x700E}, // burst amp & burst step
    //{  _BK_VE_ENC(0x002A), 0x5B41}, // U,V sat. gain

    //{  _BK_VE_ENC(0x003E), 0x00f9}, // start pixel number
    //{  _BK_VE_ENC(0x003F), 0x0417}, // do sync with TVE, start line number

    //{_REG_BASE_TBL, REG_BASE_VE_SOURCE},
    {  _BK_VE_SRC(0x0042), 0x01E0}, // Frame line number
    {  _BK_VE_SRC(0x0045), 0x5460}, // Field Size

    {_END_OF_TBL_, 0},
};


RegUnitType tVE_ENCODER_PAL_M_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},
    {  _BK_VE_ENC(0x0000), 0x7F01}, // hsync end & hsync st
    {  _BK_VE_ENC(0x0001), 0xDE9B}, // burst end & burst st
    {  _BK_VE_ENC(0x0002), 0x0088},
    {  _BK_VE_ENC(0x0003), 0x000A},
    {  _BK_VE_ENC(0x0004), 0x5000}, // luma brightness offset
    //{  _BK_VE_ENC(0x0006), 0x0001}, // pal switch en
    {  _BK_VE_ENC(0x0009), 0x06B4}, // H total

    {  _BK_VE_ENC(0x000A), 0x8000}, // bright
    {  _BK_VE_ENC(0x000B), 0xEFE3}, // burst phase step
    {  _BK_VE_ENC(0x000C), 0x21E6}, // burst phase step
    {  _BK_VE_ENC(0x000D), 0x0990}, // lower stage fraction
    {  _BK_VE_ENC(0x000E), 0x0000}, // 625 stage fraction

    {  _BK_VE_ENC(0x0025), 0x00FA}, // av st
    {  _BK_VE_ENC(0x0026), 0x068C}, // av end
    {  _BK_VE_ENC(0x0027), 0x102A}, // sync tip level & pad level
    {  _BK_VE_ENC(0x0028), 0x38F0}, // sync step & blank level
    {  _BK_VE_ENC(0x0029), 0x750E}, // burst amp & burst step
    //{  _BK_VE_ENC(0x002A), 0x6044}, // U,V sat. gain

    //{  _BK_VE_ENC(0x003E), 0x00f9}, // start pixel number
    //{  _BK_VE_ENC(0x003F), 0x0417}, // do sync with TVE, start line number
    //{_REG_BASE_TBL, REG_BASE_VE_SOURCE},
    {  _BK_VE_SRC(0x0042), 0x0240}, // Frame line number
    {  _BK_VE_SRC(0x0045), 0x6540}, // Field Size

    {_END_OF_TBL_, 0},
};

RegUnitType tVE_ENCODER_PAL_N_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},
    {  _BK_VE_ENC(0x0000), 0x8601}, // hsync end & hsync st
    {  _BK_VE_ENC(0x0001), 0xE197}, // burst end & burst st
    {  _BK_VE_ENC(0x0002), 0x0088},
    {  _BK_VE_ENC(0x0003), 0x000A},
    {  _BK_VE_ENC(0x0004), 0x5000}, // luma brightness offset
    //{  _BK_VE_ENC(0x0006), 0x0001}, // pal switch en
    {  _BK_VE_ENC(0x0009), 0x06C0}, // H total

    {  _BK_VE_ENC(0x000A), 0x8000}, // bright
    {  _BK_VE_ENC(0x000B), 0x8ABC}, // burst phase step
    {  _BK_VE_ENC(0x000C), 0x2A09}, // burst phase step
    {  _BK_VE_ENC(0x000D), 0x052E}, // lower stage fraction
    {  _BK_VE_ENC(0x000E), 0x01B2}, // 625 stage fraction

    {  _BK_VE_ENC(0x0025), 0x00FE}, // av st
    {  _BK_VE_ENC(0x0026), 0x0696}, // av end
    {  _BK_VE_ENC(0x0027), 0x102A}, // sync tip level & pad level
    {  _BK_VE_ENC(0x0028), 0x28F0}, // sync step & blank level
    {  _BK_VE_ENC(0x0029), 0x750F}, // burst amp & burst step
    //{  _BK_VE_ENC(0x002A), 0x6044}, // U,V sat. gain

    {  _BK_VE_ENC(0x003E), 0x0110}, // start pixel number
    {  _BK_VE_ENC(0x003F), 0x0417}, // do sync with TVE, start line number
    //{_REG_BASE_TBL, REG_BASE_VE_SOURCE},
    {  _BK_VE_SRC(0x0042), 0x0240}, // Frame line number
    {  _BK_VE_SRC(0x0045), 0x6540}, // Field Size

    {_END_OF_TBL_, 0},
};

RegUnitType tVE_ENCODER_PAL_NC_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},
    {  _BK_VE_ENC(0x0000), 0x7F01}, // hsync end & hsync st
    {  _BK_VE_ENC(0x0001), 0xD397}, // burst end & burst st
    {  _BK_VE_ENC(0x0002), 0x0088},
    {  _BK_VE_ENC(0x0003), 0x000A},
    {  _BK_VE_ENC(0x0004), 0x5000}, // luma brightness offset
    {  _BK_VE_ENC(0x0006), 0x0001}, // pal switc en
    {  _BK_VE_ENC(0x0009), 0x06C0}, // H total

    {  _BK_VE_ENC(0x000A), 0x8000}, // bright
    {  _BK_VE_ENC(0x000B), 0x9446}, // burst phase step
    {  _BK_VE_ENC(0x000C), 0x21F6}, // burst phase step
    {  _BK_VE_ENC(0x000D), 0x0C2E}, // lower stage fraction
    {  _BK_VE_ENC(0x000E), 0x01B2}, // 625 stage fraction

    {  _BK_VE_ENC(0x0025), 0x0117}, // av st
    {  _BK_VE_ENC(0x0026), 0x0696}, // av end
    {  _BK_VE_ENC(0x0027), 0x1000}, // sync tip level & pad level
    {  _BK_VE_ENC(0x0028), 0x28FC}, // sync step & blank level
    {  _BK_VE_ENC(0x0029), 0x750F}, // burst amp & burst step
    //{  _BK_VE_ENC(0x002A), 0x6044}, // U,V sat. gain

    {  _BK_VE_ENC(0x003E), 0x0110}, // start line number
    {  _BK_VE_ENC(0x003F), 0x0417}, // do sync with TVE, start line number


    //{_REG_BASE_TBL, REG_BASE_VE_SOURCE},
    {  _BK_VE_SRC(0x0042), 0x0240}, // Frame line number
    {  _BK_VE_SRC(0x0045), 0x6540}, // Field Size

    {_END_OF_TBL_, 0},
};


RegUnitType tVE_ENCODER_PAL_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},
    {  _BK_VE_ENC(0x0000), 0x7F01}, // hsync end & hsync st
    {  _BK_VE_ENC(0x0001), 0xD497}, // burst end & burst st
    //{  _BK_VE_ENC(0x0002), 0x0002}, // andy 080924
    {  _BK_VE_ENC(0x0002), 0x0028}, // change to 0x28 to fix TTX output signal issue // andy 080924
    {  _BK_VE_ENC(0x0003), 0x0008},
    {  _BK_VE_ENC(0x0004), 0x5013}, // luma brightness offset
    {  _BK_VE_ENC(0x0006), 0x0001}, // pal switch en

    {  _BK_VE_ENC(0x0009), 0x06C0}, // H total
    {  _BK_VE_ENC(0x000A), 0x8000}, // bright
    {  _BK_VE_ENC(0x000B), 0x8ACB}, // burst phase step
    {  _BK_VE_ENC(0x000C), 0x2A09}, // burst phase step
    {  _BK_VE_ENC(0x000D), 0x052E}, // lower stage fraction
    {  _BK_VE_ENC(0x000E), 0x01B2}, // 625 stage fraction

    {  _BK_VE_ENC(0x0025), 0x0117}, // av st
    {  _BK_VE_ENC(0x0026), 0x0696}, // av end
    {  _BK_VE_ENC(0x0027), 0x1000}, // sync tip level & pad level
    {  _BK_VE_ENC(0x0028), 0x25FC}, // sync step & blank level
    {  _BK_VE_ENC(0x0029), 0x530F}, // burst amp & burst step
    {  _BK_VE_ENC(0x002A), 0x6145}, // burst amp & burst step
    {  _BK_VE_ENC(0x0043), 0x005A}, // setting for y/pb/pr out
    {  _BK_VE_ENC(0x0046), 0x006C}, // setting for y/pb/pr out

    {  _BK_VE_ENC(0x78)|BK_LOW_BIT, 0x00},// disable MV
    //{  _BK_VE_ENC(0x003E), 0x0110}, // start line number
    //{  _BK_VE_ENC(0x003F), 0x0417}, // do sync with TVE , start lner number

    //{_REG_BASE_TBL, REG_BASE_VE_SOURCE},
    {  _BK_VE_SRC(0x0042), 0x0240}, // Frame line number
    {  _BK_VE_SRC(0x0045), 0x6540}, // Field Size

    {_END_OF_TBL_, 0},
};

RegUnitType tVE_COEF_NTSC_TBL[] =
{// disable filter
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},

    /*{  _BK_VE_ENC(0x000F), 0x0000}, // lfir_coef1
    {  _BK_VE_ENC(0x0010), 0x0000}, // lfir_coef2
    {  _BK_VE_ENC(0x0011), 0x0000}, // lfir_coef3
    {  _BK_VE_ENC(0x0012), 0x0000}, // lfir_coef4
    {  _BK_VE_ENC(0x0013), 0x0000}, // lfir_coef5
    {  _BK_VE_ENC(0x0014), 0x0200}, // lfir_coef6
    {  _BK_VE_ENC(0x001A), 0x0000}, // cfir_coef1
    {  _BK_VE_ENC(0x001B), 0x0000}, // cfir_coef2
    {  _BK_VE_ENC(0x001C), 0x0000}, // cfir_coef3
    {  _BK_VE_ENC(0x001D), 0x0000}, // cfir_coef4
    {  _BK_VE_ENC(0x001E), 0x0000}, // cfir_coef5
    {  _BK_VE_ENC(0x001F), 0x0200}, // cfir_coef6
    */
    {_END_OF_TBL_, 0},

};

RegUnitType tVE_COEF_PAL_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},

    /*{  _BK_VE_ENC(0x000F), 0x0000}, // lfir_coef1
    {  _BK_VE_ENC(0x0010), 0x0000}, // lfir_coef2
    {  _BK_VE_ENC(0x0011), 0x0000}, // lfir_coef3
    {  _BK_VE_ENC(0x0012), 0x0000}, // lfir_coef4
    {  _BK_VE_ENC(0x0013), 0x0000}, // lfir_coef5
    {  _BK_VE_ENC(0x0014), 0x0200}, // lfir_coef6
    {  _BK_VE_ENC(0x001A), 0x0000}, // cfir_coef1
    {  _BK_VE_ENC(0x001B), 0x0000}, // cfir_coef2
    {  _BK_VE_ENC(0x001C), 0x0000}, // cfir_coef3
    {  _BK_VE_ENC(0x001D), 0x0000}, // cfir_coef4
    {  _BK_VE_ENC(0x001E), 0x0000}, // cfir_coef5
    {  _BK_VE_ENC(0x001F), 0x0200}, // cfir_coef6
    */
    {_END_OF_TBL_, 0},

};


RegUnitType tVE_VBI_NTSC_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},

    {  _BK_VE_ENC(0x002E), 0x0000}, // VBI mode
    {  _BK_VE_ENC(0x004E), 0x0015}, // ccvbi_st1
    {  _BK_VE_ENC(0x004F), 0x0015}, // ccvbi_end1
    {  _BK_VE_ENC(0x0050), 0x011C}, // ccvbi_st2
    {  _BK_VE_ENC(0x0051), 0x011C}, // ccvbi_end2
    {  _BK_VE_ENC(0x0056), 0x0014}, // wssvbi_st1
    {  _BK_VE_ENC(0x0057), 0x0014}, // wssvbi_end1
    {  _BK_VE_ENC(0x006C), 0x011B}, // wssvbi_st2
    {  _BK_VE_ENC(0x006D), 0x011B}, // wssvbi_end2
    {  _BK_VE_ENC(0x005C), 0x1DD6}, // cc_phs_step [15:0]
    {  _BK_VE_ENC(0x005D), 0x04C6}, // cc_phs_step [31:16]
    {  _BK_VE_ENC(0x0060), 0x0F84}, // wws_phs_step [15:0]
    {  _BK_VE_ENC(0x0061), 0x043E}, // wws_phs_step [31:16]
    {  _BK_VE_ENC(0x0064), 0x0110}, // cc_st
    {  _BK_VE_ENC(0x0066), 0x012E}, // wws_st
    {  _BK_VE_ENC(0x0068), 0x0118}, // cc_lvl
    {  _BK_VE_ENC(0x006A), 0x0190}, // wws_lvl

    {_END_OF_TBL_, 0},
};

RegUnitType tVE_VBI_PAL_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_ENCODER},

    {  _BK_VE_ENC(0x002E), 0x0088}, // vbi mode
    {  _BK_VE_ENC(0x0052), 0x0010}, // vpsvib_st1
    {  _BK_VE_ENC(0x0053), 0x0010}, // vpsvib_end1
    {  _BK_VE_ENC(0x0054), 0x011C}, // vpsvib_st2
    {  _BK_VE_ENC(0x0055), 0x011B}, // vpsvib_end2
    {  _BK_VE_ENC(0x0056), 0x0017}, // wssvbi_st1
    {  _BK_VE_ENC(0x0057), 0x0017}, // wssvbi_st2
    {  _BK_VE_ENC(0x006C), 0x011C}, // wssvbi_st2
    {  _BK_VE_ENC(0x006D), 0x011B}, // wssvbi_st2
    {  _BK_VE_ENC(0x0058), 0x0007}, // ttvbi_st1	/* MStar andy 081105 */
    {  _BK_VE_ENC(0x0059), 0x0016}, // ttvbi_end1
    {  _BK_VE_ENC(0x005A), 0x0140}, // ttvbi_st2	/* MStar andy 081105 */
    {  _BK_VE_ENC(0x005B), 0x014F}, // ttvbi_end2
    {  _BK_VE_ENC(0x005E), 0x4BDA}, // vps_phs_step[15:0]
    {  _BK_VE_ENC(0x005F), 0x2F68}, // vps_phs_step[31:16]
    {  _BK_VE_ENC(0x0060), 0x4BDA}, // wws_phs_step[15:0]
    {  _BK_VE_ENC(0x0061), 0x2F68}, // wws_phs_step[31:16]
    {  _BK_VE_ENC(0x0062), 0x1C71}, // tt_phs_step[15:0]
    {  _BK_VE_ENC(0x0063), 0x41C7}, // tt_phs_step[31:16]
    {  _BK_VE_ENC(0x0065), 0x0153}, // vps_st
    {  _BK_VE_ENC(0x0066), 0x0130}, // wws_st
    {  _BK_VE_ENC(0x0067), 0x0115}, // tt_st
    {  _BK_VE_ENC(0x0069), 0x0190}, // vps_lvl
    {  _BK_VE_ENC(0x006A), 0x0190}, // wws_lvl
    {  _BK_VE_ENC(0x006B), 0x0190}, // tt_lvl
    {  _BK_VE_ENC(0x003C), 0x00FF}, // tt_bitmap[15:0]
    {  _BK_VE_ENC(0x003D)|BK_HIGH_BIT, 0x00}, // tt_bitmap[31:16]


    {_END_OF_TBL_, 0},
};

RegUnitType tVE_CCIROUT_NTSC_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_SOURCE},

    { _BK_VE_SRC(0x0047), 0x020D}, // frame line number
    { _BK_VE_SRC(0x0048), 0x0001}, // F0 blank star
    { _BK_VE_SRC(0x0049), 0x0017}, // F0 blank end
    { _BK_VE_SRC(0x004A), 0x0107}, // F1 blank start
    { _BK_VE_SRC(0x004B), 0x011E}, // F1 blank end
    { _BK_VE_SRC(0x004C), 0x0004}, // F0 start
    { _BK_VE_SRC(0x004D), 0x010A}, // F0 end
    { _BK_VE_SRC(0x004E), 0x0001}, // F0 V start
    { _BK_VE_SRC(0x004F), 0x0014}, // F0 V end
    { _BK_VE_SRC(0x0050), 0x0108}, // F1 V start
    { _BK_VE_SRC(0x0051), 0x011B}, // F1 V end

    {_END_OF_TBL_, 0},
};


RegUnitType tVE_CCIROUT_PAL_TBL[] =
{
    //{_REG_BASE_TBL, REG_BASE_VE_SOURCE},

    { _BK_VE_SRC(0x0047), 0x0271}, // frame line number
    { _BK_VE_SRC(0x0048), 0x0001}, // F0 blank star
    { _BK_VE_SRC(0x0049), 0x0019}, // F0 blank end
    { _BK_VE_SRC(0x004A), 0x0139}, // F1 blank start
    { _BK_VE_SRC(0x004B), 0x0152}, // F1 blank end
    { _BK_VE_SRC(0x004C), 0x0002}, // F0 start
    { _BK_VE_SRC(0x004D), 0x013A}, // F0 end
    { _BK_VE_SRC(0x004E), 0x0107}, // F0 V start
    { _BK_VE_SRC(0x004F), 0x0018}, // F0 V end
    { _BK_VE_SRC(0x0050), 0x0138}, // F1 V start
    { _BK_VE_SRC(0x0051), 0x0151}, // F1 V end

    {_END_OF_TBL_, 0},
};

MS_VE_OUT_VIDEOSYS VE_OUT_VIDEOSTD_TBL[MS_VE_VIDEOSYS_NUM] =
{         //  Reg Tbl                   Coef_TBL         VBI TBL      vtotal_525, bPALSwitch, bPALOut
/*NSTC */    {tVE_ENCODER_NTSC_TBL,     tVE_COEF_NTSC_TBL, tVE_VBI_NTSC_TBL,   0,       0,      0},
/*NSTC_443*/ {tVE_ENCODER_NTSC_443_TBL, tVE_COEF_NTSC_TBL, tVE_VBI_NTSC_TBL,   0,       0,      0},
/*NSTC_J*/   {tVE_ENCODER_NTSC_J_TBL,   tVE_COEF_NTSC_TBL, tVE_VBI_NTSC_TBL,   0,       0,      0},
/*PAL_M*/    {tVE_ENCODER_PAL_M_TBL,    tVE_COEF_PAL_TBL,  tVE_VBI_PAL_TBL,    0,       1,      1},
/*PAL_N*/    {tVE_ENCODER_PAL_N_TBL,    tVE_COEF_PAL_TBL,  tVE_VBI_PAL_TBL,    1,       1,      1},
/*PAL_NC*/   {tVE_ENCODER_PAL_NC_TBL,   tVE_COEF_PAL_TBL,  tVE_VBI_PAL_TBL,    1,       1,      1},
/*PAL_B*/    {tVE_ENCODER_PAL_TBL,      tVE_COEF_PAL_TBL,  tVE_VBI_PAL_TBL,    1,       1,      1},

};

MS_VE_OUT_DEST VE_OUT_MATCH_TBL[MS_VE_DEST_NUM][MS_VE_DEST_NUM] =
{
          //  None                  SCART                 CVBS                    SVIDEO                YPbPr
/*None   */ {{1, VE_OUT_CVBS_YCC},  {1, VE_OUT_CVBS_RGB}, {1, VE_OUT_CVBS_YCC},   {1, VE_OUT_CVBS_YCC}, {1, VE_OUT_CVBS_YCbCr},},
/*SCART  */ {{1, VE_OUT_CVBS_RGB},  {0, VE_OUT_NONE},     {0, VE_OUT_NONE},       {0, VE_OUT_NONE},     {0, VE_OUT_CVBS_YCC},  },
/*CVBS   */ {{1, VE_OUT_CVBS_YCC},  {0, VE_OUT_NONE},     {0, VE_OUT_NONE},       {1, VE_OUT_CVBS_YCC}, {1, VE_OUT_CVBS_YCbCr},},
/*SVIDEO */ {{1, VE_OUT_CVBS_YCC},  {0, VE_OUT_NONE},     {1, VE_OUT_CVBS_YCC},   {0, VE_OUT_NONE},     {0, VE_OUT_CVBS_YCC},  },
/*YPbPr  */ {{1, VE_OUT_CVBS_YCbCr},{0, VE_OUT_NONE},     {1, VE_OUT_CVBS_YCbCr}, {0, VE_OUT_NONE},     {0, VE_OUT_CVBS_YCC},  },
};

#endif

