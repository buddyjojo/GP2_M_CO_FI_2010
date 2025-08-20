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


///////////////////////////////////////////////////////////////////////////////
/// @file   hal_vd.c
/// @author MStar Semiconductor Inc.
/// @brief  Video decoder hal level
///////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include <linux/init.h>

#include <linux/delay.h>
#include <linux/jiffies.h>

#include <linux/seqlock.h>
#include <linux/spinlock.h>

#include <linux/wait.h>


#include "hal_vd_types.h"
#include "hal_vd_settings.h"
#include "common_vd_singal_def.h"
#include "hal_vd_hwreg.h"
#include "hal_vd_adc.h"


#include "mst_platform.h"
#include "mdrv_system.h"


#include "vd_dsp.h"

#define PLATFORM_MODEL PLATFORM_MSD5018

#define VD_REG_DEBUG
#ifdef  VD_REG_DEBUG

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                            BUG();\
                        }\
                    } while (0)

#define VD_ASSERT(arg)                  assert((arg))
#define VD_KDBG(x1, args...)            printk(KERN_WARNING x1, ## args)
#else
#define VD_KDBG(x1, args...)
#define VD_ASSERT(arg)
#endif


//------------------------------------------------------------------------------
//
//  VD Global Variables.
//
//------------------------------------------------------------------------------
/* LOCK 3D SPEED UP is used to improve 3D comb performance at channel is changed. */
#ifdef LOCK3DSPEEDUP
/* _bComb10Bit3Flag = 1 means driver enable 3D speed up.
_bComb10Bit3Flag = 0 means driver disable 3D speed up.	*/
B16 _bComb10Bit3Flag = 0;
/* _u32VideoSystemTimer is used to record jiffies at enable 3D speed up.
driver use it to disable 3D speed up at system goes 1.5 seconds. */
U32 _u32VideoSystemTimer;
//VD Update 1119 add more
/* this flag is used to indicate driver enables 3D comb speed up. */
B16 _bStartCheckCombSpeedup = 0;

B16	_bUnstableChecking = 0;
#endif

#ifdef PLL_TRACKING_SPEEDUP
/* PLL Tracking Speed up */
B16 _bPLLTrigerTrack = 0;
B16 _bPLLTracking = 0;
U32 _u32PLLTrackingTime = 0;
U8 u8HPllstatus = 0;	// shjang_100322
#endif

//lachesis_091021
B16 _bCheckSwingThreshold = FALSE;
U32 _u32SwingThresholdStartTime = 0;

/* this variable is used in the system layer and indicates 3D comb is on or off. */
B16 _b3DCombState = 1;

/* 3D Comb buffer */
U32     _u32Comb3DBufStartAdrVD = 0;
U32     _u32Comb3DBufLenVD = 0;

/* only one function use these variables */
U8  _u8PreNoiseMagVD;
U8  _u8AbnormalCounterVD; // <- UNUSED
U16 _u16DataHVD[3];
U8  _u8HtotalDebounceVD;
U8  _u8BackupNoiseMagVD;
U8  _u8OneshotFlag = 0;
/* below variables are used in many functions */
U16 _u16LatchHVD;

/* system forces specific video standard.  */
atomic_t _aForceVideoStandardVD;
atomic_t _aVideoSystemVD;

/* Time counter */
atomic_t _aMCUResetCounterVD;

INPUT_PORT_TYPE     _etInputPortTypeVD;
//VIDEOSTANDARD_TYPE  _etVideoSystemVD;
VIDEOSTANDARD_TYPE  _etVideoLastStandardVD = E_VIDEOSTANDARD_NOTSTANDARD;

/* HAL VD States bit repestation. */
U32         _u32HalDrvStatesVD;
#define     HAL_VD_STATES                   ((void*)&_u32HalDrvStatesVD)

#define     _bVideoSignalStable_HAL(x)      x##_bit(0, HAL_VD_STATES)
#define     _bAutoDetMode_HAL(x)            x##_bit(1, HAL_VD_STATES)
#define     _bMCUStabl_HAL(x)               x##_bit(2, HAL_VD_STATES)

/* DSP Version : 0 for DVB, 1 for NTSC
    LGE skystone 20090109
*/
VDSYSTEM		_eDSPVersion = VD_SYSTEM_DVB;// Set Analog Color System - 090309.

#ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
static MS_U32 u32VDPatchFlagStatus=0;
#endif

// shjang_091020
U8	u8VDScanFlagStatus = 0;

//Add SIF ADC reset after VDDSP loaded 20090803 Ethan
//----------------------------------------------------------------------------------------
#define DemodCmdWrReg    0x01    // write register.
#define DemodCmdRdReg    0x02    // read register.
#define DemodCmd         0x110500L
#define DemodAdrL        0x110501L
#define DemodAdrH        0x110502L
#define DemodData        0x110503L

VIDEO_CAPTUREWINTABLE_TYPE _tVideoCaptureWinTblVD[] =
{
    {E_VIDEOSTANDARD_PAL_BGHI,	MSVD_HSTART_PAL,      15/*14*/, MSVD_HACTIVE_PAL,      576}, // PAL
    {E_VIDEOSTANDARD_NTSC_M,	MSVD_HSTART_NTSC,      8/*6*/, MSVD_HACTIVE_NTSC,     480}, // NSTC
    {E_VIDEOSTANDARD_SECAM,		MSVD_HSTART_SECAM,    14, MSVD_HACTIVE_SECAM,    576}, // SECAM
    {E_VIDEOSTANDARD_NTSC_44,	MSVD_HSTART_NTSC_433,  8/*6*/, MSVD_HACTIVE_NTSC_443, 480}, // NTSC-443/PAL-60
    {E_VIDEOSTANDARD_PAL_M,		MSVD_HSTART_PAL_M,     8/*6*/, MSVD_HACTIVE_PAL_M,    480}, // PAL-M
    {E_VIDEOSTANDARD_PAL_N,		MSVD_HSTART_PAL_NC,   15/*14*/, MSVD_HACTIVE_PAL_NC,   576}, // PAL-Nc
    {E_VIDEOSTANDARD_PAL_60,	MSVD_HSTART_PAL_60,    8/*6*/, MSVD_HACTIVE_PAL_60,   480}, // PAL-60
    {E_VIDEOSTANDARD_NOTSTANDARD, 0,                     0, 0x0,                   0x0}  // Nonstandard
/*
    {MSVD_HSTART_PAL,      0x10, MSVD_HACTIVE_PAL,      576}, // PAL
    {MSVD_HSTART_NTSC,     0x09, MSVD_HACTIVE_NTSC,     480}, // NSTC
    {MSVD_HSTART_SECAM,    0x12, MSVD_HACTIVE_SECAM,    576}, // SECAM
    {MSVD_HSTART_NTSC_433, 0x09, MSVD_HACTIVE_NTSC_443, 480}, // NTSC-443/PAL-60
    {MSVD_HSTART_PAL_M,    0x09, MSVD_HACTIVE_PAL_M,    480}, // PAL-M
    {MSVD_HSTART_PAL_NC,   0x12, MSVD_HACTIVE_PAL_NC,   576}, // PAL-Nc
    {MSVD_HSTART_PAL_60,   0x09, MSVD_HACTIVE_PAL_60,   480}, // PAL-60
    {0,                    0x00, 0x0,                   0x0}  // Nonstandard

*/
};

VIDEO_CAPTUREWINTABLE_TYPE _tVideoCaptureWinTblVD_SVIDEO[] =
{
    {E_VIDEOSTANDARD_PAL_BGHI,	MSVD_HSTART_PAL_SVIDEO,      14/*16*/, MSVD_HACTIVE_PAL,      576}, // PAL
    {E_VIDEOSTANDARD_NTSC_M,	MSVD_HSTART_NTSC_SVIDEO,      6/*8*/, MSVD_HACTIVE_NTSC,     480}, // NSTC
    {E_VIDEOSTANDARD_SECAM,		MSVD_HSTART_SECAM_SVIDEO,    14/*15*/, MSVD_HACTIVE_SECAM,    576}, // SECAM
    {E_VIDEOSTANDARD_NTSC_44,	MSVD_HSTART_NTSC_433_SVIDEO,  6/*8*/, MSVD_HACTIVE_NTSC_443, 480}, // NTSC-443/PAL-60
    {E_VIDEOSTANDARD_PAL_M,		MSVD_HSTART_PAL_M_SVIDEO,     6/*8*/, MSVD_HACTIVE_PAL_M,    480}, // PAL-M
    {E_VIDEOSTANDARD_PAL_N,		MSVD_HSTART_PAL_NC_SVIDEO,   14/*17*/, MSVD_HACTIVE_PAL_NC,   576}, // PAL-Nc
    {E_VIDEOSTANDARD_PAL_60,	MSVD_HSTART_PAL_60_SVIDEO,    6/*8*/, MSVD_HACTIVE_PAL_60,   480}, // PAL-60
    {E_VIDEOSTANDARD_NOTSTANDARD, 0,                     0, 0x0,                   0x0}  // Nonstandard
/*
    {MSVD_HSTART_PAL,      0x10, MSVD_HACTIVE_PAL,      576}, // PAL
    {MSVD_HSTART_NTSC,     0x09, MSVD_HACTIVE_NTSC,     480}, // NSTC
    {MSVD_HSTART_SECAM,    0x12, MSVD_HACTIVE_SECAM,    576}, // SECAM
    {MSVD_HSTART_NTSC_433, 0x09, MSVD_HACTIVE_NTSC_443, 480}, // NTSC-443/PAL-60
    {MSVD_HSTART_PAL_M,    0x09, MSVD_HACTIVE_PAL_M,    480}, // PAL-M
    {MSVD_HSTART_PAL_NC,   0x12, MSVD_HACTIVE_PAL_NC,   576}, // PAL-Nc
    {MSVD_HSTART_PAL_60,   0x09, MSVD_HACTIVE_PAL_60,   480}, // PAL-60
    {0,                    0x00, 0x0,                   0x0}  // Nonstandard

*/
};

const VD_REG_TABLE_t gVdRegInit[] =
{
    //initial AFEC Setting
    {BK_AFEC_E3, 0x20}, // fixed color stripe issue
    {BK_AFEC_69, 0x80}, // 3569[6]=0 ,HK mcu is fast VD
    //{BK_AFEC_1E, 0x88}, // REG_351E[3]=1 Disable RF Compensation Filter
    {BK_AFEC_1E, 0xC8}, // BY 20090628 enable ADC 4x  //20090630
    {BK_AFEC_21, 0x19}, // REG_3521[0]=1 Disable Digial Clamp S/W Reset
    {BK_AFEC_2F, 0x84},
    {BK_AFEC_38, 0x13},
    {BK_AFEC_B4, 0x7C}, // fixed color stripe issue
//#if( !ENABLE_DYNAMIC_VD_HTOTAL )
    {BK_AFEC_7B, 0xB6},
    {BK_AFEC_8F, 0x19}, // frequency synthesizer output
    {BK_AFEC_9D, 0xD4},
    {BK_AFEC_9E, 0xC0},
//#endif
    {BK_AFEC_D8, 0x20},
    {BK_AFEC_6B, 0xA2},
    {BK_AFEC_5A, 0xAF}, // for improve the H glich

    //add to  enable 2-Line-delay and current bottom for improve the H-Jitter issue that has released for Laser
    { BK_AFEC_A2, 0x40 },
    { BK_AFEC_E0, 0x05 },
    { BK_AFEC_E6, 0x05 },
    { BK_AFEC_EA, 0x4B },
    // initial comb filter
//  #if (COMB_FLTR_TYPE == COMB_FLTR_3D) // 3D comb
//    {BK_COMB_10, 0x17}, // 2 = 2D comb, 03 = 3D comb.
//  #else // 2D/1D comb
    {BK_COMB_10, 0x12}, // 2 = 2D comb, 03 = 3D comb.
//  #endif
	//lachesis_090116 by victor
    //{BK_COMB_12, 0x12}, // Reg3612[3:0] < 8 from AFC , Reg3612[3:0]= 8 from comb
    {BK_COMB_12, 0x1A}, // Reg3612[3:0] < 8 from AFC , Reg3612[3:0]= 8 from comb
	{BK_COMB_22, 0x83},

    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_13, 0x82},
    {BK_COMB_14, 0x00},
    #endif

#if(TV_SYSTEM==TV_PAL)
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_17, 0xBF}, // update for ACC issue, 20080410, James.Lu
    {BK_COMB_20, 0x75},
    {BK_COMB_49, 0xC0},
    #endif

    {BK_COMB_15, 0x0A},

    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_90, 0x33},
    {BK_COMB_91, 0x12},
    {BK_COMB_92, 0xD1},
    {BK_COMB_95, 0x55},
    {BK_COMB_97, 0x00},
    {BK_COMB_9A, 0x03},
    {BK_COMB_D0, 0x91},
    #endif
#elif(TV_SYSTEM==TV_NTSC)
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_17, 0xC0},
    {BK_COMB_20, 0x27},
    #endif
    {BK_COMB_7F, 0x1F},
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_90, 0x31},
    {BK_COMB_91, 0x32},
    {BK_COMB_92, 0x00},
    {BK_COMB_97, 0x80},
    {BK_COMB_9A, 0x44},
    {BK_COMB_D0, 0xB0},
    #endif
#endif
    {BK_COMB_18, 0x01},
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_1B, 0x8B},
    {BK_COMB_1C, 0xFC}, // Reg361C[6]= 1 from AFC , Reg361C[6]= 0 from comb

    {BK_COMB_21, 0x00},
    {BK_COMB_22, 0x83},
    {BK_COMB_23, 0x2D},
    {BK_COMB_24, 0x13}, // for still image, Rober.chan 20080410, ,Mick 070904
    {BK_COMB_25, 0xA0},
    {BK_COMB_2A, 0x84}, // Reg362A[7]= 1 from AFC , Reg362A[7]= 0 from comb
    {BK_COMB_2B, 0x20},
    {BK_COMB_2C, 0x04},
    {BK_COMB_2D, 0xC0},
    {BK_COMB_2E, 0x22},
    {BK_COMB_2F, 0x48},

    {BK_COMB_30, 0xA2},
    {BK_COMB_31, 0x25},
    {BK_COMB_33, 0x30},
    {BK_COMB_35, 0x50}, //Robert Comb
    {BK_COMB_36, 0x50},
    {BK_COMB_37, 0x88}, //modify 3637 0x88->0x8A by Robert Chen
    {BK_COMB_38, 0x38}, // for Eris-ATT, James.Lu, 20080327
    {BK_COMB_39, 0x00},
    {BK_COMB_3A, 0x25}, // modify 363A[7:0]: motion detection Max filter for LG
    {BK_COMB_3C, 0xF8},
    #endif
    /*
    #ifdef COMB_3D
    { BK_COMB_3F, (COMB_3D_BUF_START_ADR >>  3) & 0xFF }, // <-<<< set 3D COMB BUF START ADDR---L
    { BK_COMB_3E, (COMB_3D_BUF_START_ADR >> 11) & 0xFF }, // <-<<< set 3D COMB BUF START ADDR---M
    { BK_COMB_3D, (COMB_3D_BUF_START_ADR >> 19) & 0xFF }, // <-<<< set 3D COMB BUF START ADDR---H
    { BK_COMB_88, (COMB_3D_BUF_START_ADR+COMB_3D_BUF_LEN)>>19}, // COMB memory protect address - H
    { BK_COMB_89, (COMB_3D_BUF_START_ADR+COMB_3D_BUF_LEN)>>11}, // COMB memory protect address - M
    { BK_COMB_8A, (COMB_3D_BUF_START_ADR+COMB_3D_BUF_LEN)>>3},  // COMB memory protect address - L
    { BK_COMB_4C, 0x80},                                // COMB memory protect enable
    #endif
    */

//#if( MST_TV_VER == MST_9xxxG_A )
    {BK_COMB_40, 0x98},
//#else
//    {BK_COMB_40, 0x88},
//#endif
#if(PQ_IN_VD_ENABLE)
    {BK_COMB_41, 0x06}, // requested by James.Lu for ATT, 20080327
#endif
    {BK_COMB_45, 0xB1},
    {BK_COMB_4D, 0x90},
    {BK_COMB_4E, 0x2C},

    {BK_COMB_52, 0x6F},
    {BK_COMB_53, 0x04},
    {BK_COMB_57, 0x50}, //Mike 070904
    {BK_COMB_58, 0x20}, //Mike 070904
    {BK_COMB_5A, 0x00},
    {BK_COMB_5B, 0x10},
    {BK_COMB_5C, 0x08},
    {BK_COMB_5D, 0xFF},
    {BK_COMB_5E, 0x14},
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_5F, 0x00},
    {BK_COMB_62, 0x32},
    {BK_COMB_63, 0xFE},
    {BK_COMB_64, 0x78}, // 0x79}, Bruce Lin 2007.0824
    {BK_COMB_65, 0xFD},
    {BK_COMB_66, 0x88},
    {BK_COMB_67, 0x07},
    {BK_COMB_68, 0x04},
    {BK_COMB_69, 0xFD},
    #endif
    {BK_COMB_6A, 0xF0},
    {BK_COMB_6D, 0x02},
    {BK_COMB_6B, 0x3B},
    {BK_COMB_6D, 0x61},

    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_6E, 0x18},
    #endif

    {BK_COMB_70, 0xD8}, // BK6_70[3] = 1 ; burst height 0xD0->0xD8 by Robert chen , (ACC mode is auto)
    {BK_COMB_71, 0x87},
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_78, 0xB0},
    {BK_COMB_7A, 0x0F},
    {BK_COMB_7B, 0x80},
    {BK_COMB_7C, 0x36},
    {BK_COMB_7E, 0xE0}, // for ACC issue, 20080410, James.Lu
    #endif

#if (PATCH_VD_VIF_BRI)
    {BK_COMB_80, 0xDC},
#endif
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_82, 0x6A},
    {BK_COMB_86, 0x00},
    {BK_COMB_87, 0x00},
    {BK_COMB_8B, 0x0D},
    {BK_COMB_8C, 0x06},
    {BK_COMB_8D, 0x80},
    {BK_COMB_8E, 0x80},
    {BK_COMB_8F, 0xC3},

    {BK_COMB_93, 0x00},
    {BK_COMB_94, 0x71},
    {BK_COMB_96, 0x00},
    {BK_COMB_98, 0x2A},
    {BK_COMB_99, 0x00},
    {BK_COMB_9B, 0x00},
    {BK_COMB_9C, 0x03},
    {BK_COMB_9D, 0x00},
    {BK_COMB_9E, 0x01},
    #endif
    {BK_COMB_9F, 0xC6},
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_A0, 0xC8},
    {BK_COMB_AD, 0x60},
    {BK_COMB_C0, 0xA0},
    {BK_COMB_C1, 0x11},
    {BK_COMB_C3, 0x1D},
    {BK_COMB_C4, 0x01},
    {BK_COMB_D1, 0x20},
    {BK_COMB_D2, 0x20},
    {BK_COMB_D3, 0x15},
    {BK_COMB_D6, 0x0D},
    {BK_COMB_DB, 0x32},
    {BK_COMB_DD, 0x60},
    {BK_COMB_DE, 0x00},
    #endif
    {BK_COMB_ED, 0x80},
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_F1, 0xA0},
    #endif
    {BK_COMB_F2, 0x47},
    #if(PQ_IN_VD_ENABLE)
    {BK_COMB_F3, 0x0B},
    {BK_COMB_F4, 0x17},
    {BK_COMB_F8, 0x40},
    {BK_COMB_FE, 0x03},
    #endif
    // Fine tune SECAM detect threshold
    {BK_SECAM_02, 0x92},
    {BK_SECAM_03, 0xA2},
    {BK_SECAM_04, 0x2B},
    {BK_SECAM_05, 0x64},
    {BK_SECAM_0A, 0x20}, // modify by CJ 20070402 0xC0},
    {BK_SECAM_0B, 0x42},
    {BK_SECAM_0D, 0x05}, // modify by CJ 20070402 0x04},
    {BK_SECAM_0E, 0x65}, // modify by CJ 20070402 0x64},

    {BK_SECAM_11, 0xC0}, // modify by CJ 20070402 0x00},
    {BK_SECAM_12, 0x01}, // modify by CJ 20070402 0x02},
    {BK_SECAM_13, 0x00},
    {BK_SECAM_14, 0xC0},
    {BK_SECAM_15, 0xBF}, // modify by CJ 20070402 0xBF},

    {BK_SECAM_01, 0x50}, // CJ updated 0x00 --> 0x40 ([6]: Enable mixing option)2007.02.26
    {BK_SECAM_17, 0x07}, // CJ add sett 2007.02.26

    // Fine WSS detect by CJ
    {BK_VBI_45, 0x60},
    {BK_VBI_4A, 0x03},
    {BK_VBI_4F, 0x20},
    {BK_VBI_50, 0xF2},     //BK8_50
    {BK_VBI_77, 0x11},     //BK8_77
    {BK_VBI_7C, 0x04},     //BK8_7C
    {BK_VBI_7D, 0x36},     //BK8_7D
    {BK_VBI_7E, 0x84},     //BK8_7E
    {BK_VBI_7F, 0xF6},     //BK8_7F
    {BK_VBI_81, 0x52},     //BK8_81
    {BK_VBI_86, 0x96},     //BK8_86
    {BK_VBI_89, 0xC2},     //BK8_89
    {BK_VBI_8B, 0x84},     //BK8_8B
    {BK_VBI_8D, 0x95},     //BK8_8D
    {BK_VBI_99, 0x8C},//for PAL
    {BK_VBI_9A, 0x01},//for PAL
//    {BK_VBI_99, 0x6D},//for SECAM
//    {BK_VBI_9A, 0x9A},//for SECAM
    {BK_VBI_BB, 0x26},
    {BK_VBI_CA, 0x21},
    {BK_VBI_CB, 0xC4},     //BK8_CC

    { _END_OF_TBL_, 0x00 }
};

//	modified to change the initial value of color kill bound (dreamer@lge.com)
static U8	_gColorKillBound[2] = { COLOR_KILL_HIGH_BOUND, COLOR_KILL_LOW_BOUND };


/* Write/Read method invalid */
inline U8 _MHal_VD_R1B( U32 u32Reg )
{
    U8  u8Val;
    u8Val = X1BYTE(u32Reg);
    mb();//channel skip test 090113
    return u8Val;
}

inline void _MHal_VD_W1B( U32 u32Reg, U8 u08Val)
{
    X1BYTE(u32Reg) = u08Val;
    wmb();
}

inline void _MHal_VD_W1BM( U32 u32Reg, U8 u08Val, U8 u08Mask)
{
    U8  u8RegVal;
    rmb();
    u8RegVal = X1BYTE(u32Reg);
    u8RegVal = ((u8RegVal & ~(u08Mask)) | ((u08Val) & (u08Mask)));
    X1BYTE(u32Reg) = u8RegVal;
    wmb();
}


inline void _MHal_VD_W1Rb( U32 u32Reg, B16 bBit, U8 u08BitPos )
{
    U8  u8RegVal;
    rmb();
    u8RegVal = X1BYTE(u32Reg);
    if(bBit)
    {
        u8RegVal = (u8RegVal | u08BitPos);
    }
    else
    {
        u8RegVal = u8RegVal & ~(u08BitPos);
    }

    X1BYTE(u32Reg) = u8RegVal;
    wmb();
}


inline U8  _MHal_VD_R1Rb( U32 u32Reg, U8 u08BitPos )
{
    rmb();
    return (X1BYTE(u32Reg) & (u08BitPos));
}

inline U16 _MHal_VD_R2B( U32 u32Reg )
{
    U16  u16Val;
    u16Val = X2BYTE(u32Reg);
    mb();
    return u16Val;//channel skip test 090113
}


inline void _MHal_VD_W2B( U32 u32Reg, U16 u16Val )
{
    (X2BYTE(u32Reg) = (u16Val));
    wmb();
}

inline void _MHal_VD_W2BM( U32 u32Reg, U16 u16Val, U16 u16Mask )
{
    U16  u16RegVal;
    rmb();
    u16RegVal = X2BYTE(u32Reg);
    u16RegVal = ((u16RegVal & ~(u16Mask)) | ((u16Val) & (u16Mask)));
    X2BYTE(u32Reg) = u16RegVal;
    wmb();
}

inline void _MHal_VD_W2Bb( U32 u32Reg, B16 bBit, U16 u16BitPos )
{
    U16  u16RegVal;
    rmb();
    u16RegVal = X2BYTE(u32Reg);
    if(bBit)
    {
        u16RegVal = (u16RegVal | (u16BitPos));
    }
    else
    {
        u16RegVal = (u16RegVal & ~(u16BitPos));
    }
    X2BYTE(u32Reg) = u16RegVal;
    wmb();

}

inline U16 _MHal_VD_R2Bb( U32 u32Reg, U16 u16BitPos )
{
    U16  u16Val;
    u16Val = X2BYTE(u32Reg);
    mb();

    return (u16Val & (u16BitPos));//channel skip test 090113
}


//------------------------------------------------------------------------------
//
//  VD Local Function Prototypes
//
//------------------------------------------------------------------------------
VIDEOSTANDARD_TYPE      MHal_VD_GetStandardDetection(void);
static void             _SetHTotal(U32 u32HTotal);
static void             _EnableCVBSLPF(B16 bEnable);
void                    MHal_VD_McuReset(void);


#ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
void MHal_VD_COMB_Set3dCombMid(MS_BOOL bEnable);
#endif

void MHal_VD_SIF_ADC_Reset(void);  //Add SIF ADC reset after VDDSP loaded 20090803 Ethan

//------------------------------------------------------------------------------
//
//  Macro for VD driver
//
//------------------------------------------------------------------------------

///-----------------------------------------------------------------------------
/// enable auto detect FSC mode
/// @param  B16
///     - FSC_AUTO_DET_ENABLE  : enable auto detection mode.
///     - FSC_AUTO_DET_DISABLE : doesn't effect.
/// @return void
/// @note   <private function>
/// @type
///-----------------------------------------------------------------------------
void inline  _MHal_VD_SetAutoDetetion(B16 bMode)
{
#if (TEST_VD_DSP)
    if(bMode == FSC_AUTO_DET_ENABLE)
    {
        _bAutoDetMode_HAL(set);
        _MHal_VD_W1BM( BK_AFEC_CE, 0, _BIT0 );
    }
    else
    {

        /* this feature doesn't be included in origin of the Kingwork (project name). */
    }
#else
    if(bMode == FSC_AUTO_DET_ENABLE)
    {
        _bAutoDetMode_HAL(set);
        _MHal_VD_W1BM( BK_AFEC_CE, 0, _BIT0 );
    }
    else
    {
        /* this feature doesn't be included in origin of the Kingwork (project name). */
    }
#endif
}
/*
#if (TEST_VD_DSP)
#define SET_FSC_AUTO_DET_MODE(x)    { _u8AutoDetModeVD = (x); _MHal_VD_W1BM( BK_AFEC_CE, 0, _BIT0 ); } // { _u8AutoDetModeVD = (x); _MHal_VD_W1BM( BK_AFEC_CE, _u8AutoDetModeVD, _BIT0 ); } // { _u8AutoDetModeVD = (x); _MHal_VD_W1BM( BK_AFEC_CE, _u8AutoDetModeVD, _BIT0 ); printf ("SET %s\n", (_u8AutoDetModeVD == FSC_AUTO_DET_ENABLE) ? "AUTO" : "FORCE"); }
#else
#define SET_FSC_AUTO_DET_MODE(x)    { _u8AutoDetModeVD = (x); _MHal_VD_W1BM( BK_AFEC_CE, 0, _BIT0 ); } // { _u8AutoDetModeVD = (x); _MHal_VD_W1BM( BK_AFEC_CE, 0, _BIT0 ); printf ("SET %s\n", (_u8AutoDetModeVD == FSC_AUTO_DET_ENABLE) ? "AUTO" : "FORCE"); }
#endif
*/


///-----------------------------------------------------------------------------
/// force VD runs in specific mode
/// @param  U32
///     - FSC_MODE_PAL, FSC_MODE_HTSC, FSC_MODE_SECAM, ...
/// @return void
/// @note   <private function>
/// @type
///-----------------------------------------------------------------------------
void inline _MHal_VD_SetFSCType(U32 u32FSCType)
{
#if (TEST_VD_DSP)
    _MHal_VD_W1BM( BK_AFEC_CE, (u32FSCType) << 1, BITMASK(3:1) );
#else

#endif

#ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
if ((u32FSCType != FSC_MODE_PAL) && (u32FSCType != FSC_MODE_NTSC))
{
    MHal_VD_COMB_Set3dCombMid(DISABLE);
}
#endif

}
/*
#if (TEST_VD_DSP)
#define SET_FSC_MODE(x)             _MHal_VD_W1BM( BK_AFEC_CE, (x) << 1, BITMASK(3:1) )
#else
#define SET_FSC_MODE(x)
#endif
*/

///-----------------------------------------------------------------------------
/// Read data from VD status register
/// @param  void
/// @return U16:
/// - VD status register
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
U16 MHal_VD_GetStatus(void)
{
    /* AFEC_CC register doesn't need lock due to it's value is updated by VDMCU. */
    //return _MHal_VD_R2B( BK_AFEC_CC );
	/* access AFEC CC and CD register in 8 bits address. */
    U16     u16VDCC_CD;
    u16VDCC_CD = _MHal_VD_R1B( BK_AFEC_CD );
    u16VDCC_CD = (u16VDCC_CD<<8) | (_MHal_VD_R1B( BK_AFEC_CC ));
    return u16VDCC_CD;
}


#if 1
///-----------------------------------------------------------------------------
/// Initial VD clock (TEST USAGE)
/// @param  void
/// @return void
/// @note   <private function>
/// @type
///-----------------------------------------------------------------------------
void _MHal_VD_InitClock(void)
{
    /*
    _MHal_VD_W1B(BYTE2REAL(0x2508), 0x00);

	_MHal_VD_W1B(BYTE2REAL(0x2509), 0x00);
	_MHal_VD_W1B(BYTE2REAL(0x250A), 0x00);
	_MHal_VD_W1B(BYTE2REAL(0x250B), 0x00);

	_MHal_VD_W1B(BYTE2REAL(0x250C), 0x00);
	_MHal_VD_W1B(BYTE2REAL(0x250D), 0x00);

    _MHal_VD_W1B(BYTE2REAL(0x2514), 0x24);
    _MHal_VD_W1B(BYTE2REAL(0x2515), 0x0);

    _MHal_VD_W1B(BYTE2REAL(0x2510), 0x06);
    _MHal_VD_W1B(BYTE2REAL(0x2511), 0x05);

    _MHal_VD_W1B(BYTE2REAL(0x2516), 0x0);
    _MHal_VD_W1B(BYTE2REAL(0x2517), 0x0);

    _MHal_VD_W1B(BYTE2REAL(0x25C0), 0x0);
    _MHal_VD_W1B(BYTE2REAL(0x25C1), 0x0);
    */

    // MIU, DDR
    //_MHal_VD_W1B(BYTE2REAL(0x1E25), 0x0);

    // CKG_VD, VDMCU, VD200, VD clock enable
    #if 0
    _MHal_VD_W1B(BYTE2REAL(0x1E2D), 0x0);
    _MHal_VD_W1B(BYTE2REAL(0x1E2E), 0x0);
    #endif

    // reserved
    //_MHal_VD_W1B(BYTE2REAL(0x1E32), 0x0);

    // turn on VD mcu and VD200 clock pll
    /*
    XBYTE(CHIP_TOP_CKG_VD) &= ~(BIT4);
    XBYTE(CHIP_TOP_CKG_VDMCU) &= ~(BIT0);
    XBYTE(CHIP_TOP_CKG_VD200) &= ~(BIT4);
    XBYTE(CHIP_TOP_CKG_VD) &= ~(BIT4);
    */
    // CKG_VD
    #if 0
    _MHal_VD_W1Rb(CHIP_TOP_VD_SEL, 0, _BIT1);
    #endif
    //XBYTE(CHIP_TOP_VD_SEL) &= ~(BIT1);


    /* reset VD MCU */
    if (0 == Chip_Query_Rev())
    {
        _MHal_VD_W1B(L_BK_CLKGEN0(0x21), 0x10);  // 20090628 BY put VD200 clock setting here. TODO, add another function??
        _MHal_VD_W1B(H_BK_CLKGEN0(0x21), 0x00);
    }
    else
    {
        _MHal_VD_W1B(L_BK_CLKGEN0(0x21), 0x0D);  // 20090628 BY put VD200 clock setting here. TODO, add another function??
        //_MHal_VD_W1B(H_BK_CLKGEN0(0x21), 0x04);  //20090825EL
        _MHal_VD_W1B(H_BK_CLKGEN0(0x21), 0x00);
    }


    _MHal_VD_W1B(L_BK_CLKGEN0(0x22), 0x00);  // 20090628 BY put mailbo0 clock setting here. TODO, add another function??
    _MHal_VD_W1B(H_BK_CLKGEN0(0x22), 0x00);  // 20090628 BY put mailbo1 clock setting here. TODO, add another function??

    //AVD_VDMCU_CLOCK_86Mhz
    //_MHal_VD_W1BM(L_BK_CLKGEN0(0x21), BIT(4), 0x1C);  //20090611EL


    //AVD_VDMCU_CLOCK_INV
    //_MHal_VD_W1BM(L_BK_CLKGEN0(0x21), BIT(1), BIT(1));  //20090611EL

    msleep(7);
}

///-----------------------------------------------------------------------------
/// Dump memory
/// @param  void
/// @return void
/// @note   <private function>
/// @type
///-----------------------------------------------------------------------------
void _MHal_VD_DumpMemory(U32 pu8MemAddr, U32 u32Len)
{
    volatile U8  *pu8BufDumpAddr;
    U32 u32Index = 0;

    //VD_KDBG("Dump buffer 0x%x\n", (U32)pu8MemAddr);

    pu8BufDumpAddr = (volatile U8*)((U32)pu8MemAddr);
    for(u32Index=0; u32Index<u32Len; u32Index++)
    {
        //VD_KDBG("0x%02x, 0x%02x, 0x%02x, 0x%02x", (U8)*pu8BufDumpAddr, (U8)*(pu8BufDumpAddr+1), (U8)*(pu8BufDumpAddr+2), (U8)*(pu8BufDumpAddr+3));
        //VD_KDBG("   0x%02x, 0x%02x, 0x%02x, 0x%02x\n", (U8)*(pu8BufDumpAddr+4), (U8)*(pu8BufDumpAddr+5), (U8)*(pu8BufDumpAddr+6), (U8)*(pu8BufDumpAddr+7));
        pu8BufDumpAddr+=8;
    }
}
#endif

// Set Analog Color System - Start 090309.
///-----------------------------------------------------------------------------
/// Set DSP Version
/// @param  VDSYSTEM   eDSPVer
/// @return void
/// @note
/// @type
///-----------------------------------------------------------------------------
void MHal_VD_SetDSPVer(VDSYSTEM   eDSPVer)
{
	_eDSPVersion= eDSPVer;
}
///-----------------------------------------------------------------------------
/// Get DSP Version
/// @param  void
/// @return VDDSP_VERSION
/// @note
/// @type
///-----------------------------------------------------------------------------
VDSYSTEM MHal_VD_GetDSPVer(void)
{
	return _eDSPVersion;
}

///-----------------------------------------------------------------------------
/// Load DSP code
/// @param  void
/// @return U16
/// @note
/// @type
///-----------------------------------------------------------------------------
#ifdef PWS_STOP_VDMCU
extern void MHal_PWS_Stop_VDMCU();
#endif
S8 MHal_VD_LoadDsp( void )
{
    S8 s8Return = 0;
#ifdef DSPBIN_LOAD_FROM_SERIAL_FLASH
    U32 u32Len;
    U32 u32Adr;
    volatile U32 *pu32ChunkHeaderAdr;
#endif

#ifdef DSPBIN_LOAD_FROM_MEM
    U8              *pu8VD_DSP = NULL;
    U32             u32VD_DSP_len;
//    U32             u32Timeout;
    U32             i;
#endif
    //printk("MHal_VD_LoadDsp\r\n");

#ifdef PWS_STOP_VDMCU
    MHal_PWS_Stop_VDMCU(); // sonic 20091120
#else
    MHal_VD_VDMCU_SoftStop();
#endif

    _MHal_VD_W1B(BYTE2REAL(DemodCmd&0x0FFFFF),0); // 20091118 BY, clear VIF mailbox command
#ifdef DSPBIN_LOAD_FROM_SERIAL_FLASH

    //========MDrv_VD_LoadDSPCode=======

    //=======MDrv_VDMCU_Init=========
    //halt VD MCU first
    _MHal_VD_W1Rb(VD_MCU_RESET, 0x01, _BIT0);
    //disable Read/write
    _MHal_VD_W1B(VD_MCU_READ_WRITE, 0x00);
    //get demux binary information
    _MHal_VD_W1B(VD_MCU_READ_WRITE, 0x00);



    //--------------------------------------------------------
    // Load code
    //--------------------------------------------------------

    /*
        Copy code from spi space to VD MCU (not SDRAM)
        PIU dma cannot do data translation between DRAM to DRAM.
    */
    // vd_dsp.bin is fix at SPI serial flash address 0x240000 in current version.
    //#define VDADDR 0x240000             // SPI VD_DSP.bin's address in SPI serial flash.

    //_MHal_VD_DumpMemory(0xBFC00000+0x1C00, 16);

    pu32ChunkHeaderAdr = (volatile U32*)(0xBFC00000+0x1C00);
    u32Adr = (U32) *pu32ChunkHeaderAdr;
    u32Len = (U32) *(pu32ChunkHeaderAdr+1);

    //VD_KDBG("ADR=%x\n", u32Adr);
    //VD_KDBG("LEN=%x\n", u32Len);

    _MHal_VD_W1B(DMA_SRC_ADDR_0, (U8)(((U32)u32Adr+0x00000000)&0x000000FF));
    _MHal_VD_W1B(DMA_SRC_ADDR_1, (U8)((((U32)u32Adr+0x00000000)&0x0000FF00)>>8));
    _MHal_VD_W1B(DMA_SRC_ADDR_2, (U8)((((U32)u32Adr+0x00000000)&0x00FF0000)>>16));
    _MHal_VD_W1B(DMA_SRC_ADDR_3, (U8)((((U32)u32Adr+0x00000000)&0xFF000000)>>24));


    /* System doesn't care destination address if it is VD MCU */
    _MHal_VD_W1B(DMA_DST_ADDR_0, 0);
    _MHal_VD_W1B(DMA_DST_ADDR_1, 0);
    _MHal_VD_W1B(DMA_DST_ADDR_2, 0);
    _MHal_VD_W1B(DMA_DST_ADDR_3, 0);


    /* If your destination is DRAM, the DMA length need 8 bytes alignment */
    //u32len = (((_u32VDDSPCodeLen>>3)+1)<<3);
    // the length of vd_dsp.bin is 11986 in current version.
    //u32len = 11986;
    //u32len = 12004;
    _MHal_VD_W1B(DMA_SIZE_0, (U8)(((U32)u32Len)&0x000000FF));
    _MHal_VD_W1B(DMA_SIZE_1, (U8)((((U32)u32Len)&0x0000FF00)>>8));
    _MHal_VD_W1B(DMA_SIZE_2, (U8)((((U32)u32Len)&0x00FF0000)>>16));
    _MHal_VD_W1B(DMA_SIZE_3, (U8)((((U32)u32Len)&0xFF000000)>>24));

    // 3c20   crc_dum#3=1;
    _MHal_VD_W1Rb(DMA_FLAGS, 0x01, _BIT3);

    // 3cec   addr_inc=1,riu_mode=1,dst_sle=1,trig=1;
    _MHal_VD_W1B(DMA_CTRL, 0xC9);

    /* wait DMA be done */
    // BIT2: dma_busy  BIT1: dma_done
    while ((XBYTE(DMA_FLAGS) & (_BIT2 | _BIT1)) != _BIT1){
        msleep(2);
    }

    // crc_dum#3=0, or the bus will be hold!!
    _MHal_VD_W1Rb(DMA_FLAGS, 0x00, _BIT3);

    // release VD_MCU and run code on SDRAM
    // clear MCU2MIU_RST and enable MIU_FETCH_EN
    //XBYTE(L_BK_CHIPTOP(0x17)) = (XBYTE(L_BK_CHIPTOP(0x17)) & ~(0x0E)) | 0x0C; // <-<<< inverse VD MCU clock
    _MHal_VD_W1BM(L_BK_CHIPTOP(0x17), 0x04/*0x0C*/, 0x0E);	//dcsong 080826

    /* reset VD mcu */
    _MHal_VD_W1Rb(VD_MCU_RESET, 0, _BIT0);

    /* dump SPI address memory */
    //_MHal_VD_DumpMemory(0xBFC00000+0x240000, 20);
#endif


#ifdef DSPBIN_LOAD_FROM_MEM

    // release VD_MCU and run code on SDRAM
    // clear MCU2MIU_RST and enable MIU_FETCH_EN
    //XBYTE(L_BK_CHIPTOP(0x17)) = (XBYTE(L_BK_CHIPTOP(0x17)) & ~(0x0E)) | 0x0C; // <-<<< inverse VD MCU clock

    //_MHal_VD_W1BM(L_BK_CHIPTOP(0x17), 0x04, 0x0E);


    /* reset VD MCU */
    //_MHal_VD_W2Bb(VD_MCU_REG(0x06), 0x01, _BIT0);
    _MHal_VD_W1Rb(VD_MCU_RESET, 0x01, _BIT0);

    /* disable sram */
    _MHal_VD_W1Rb(VD_MCU_SRAM_EN, DISABLE, _BIT0);


    /* enable down load code */
    //_MHal_VD_W2Bb(VD_MCU_REG(0x05), 0x01, _BIT0);
    _MHal_VD_W1BM(VD_MCU_KEY, 0x50, 0xF0);

    /* enable address auto increment */
    //_MHal_VD_W2Bb(VD_MCU_REG(0x04), 0x01, _BIT3);
    _MHal_VD_W1Rb(VD_MCU_ADDR_AUTO_INC, ENABLE, _BIT0);

    /* reset sram address to 0 */
    _MHal_VD_W1B(VD_MCU_ADDR_L, 0x00);
    _MHal_VD_W1B(VD_MCU_ADDR_H, 0x00);
    //HAL_AVD_VDMCU_SetClock(AVD_VDMCU_CLOCK_86Mhz, AVD_VDMCU_CLOCK_INV);


	//DSP branch
	if(MHal_VD_GetDSPVer() == VD_SYSTEM_DVB)
	{
		// DVB
		VD_KDBG("Load DVB\n");
	    pu8VD_DSP = (U8*)&_u8VDDspImage[0];
	    u32VD_DSP_len = _u32VDDSPCodeLen;
	}
	else
	{
		// NTSC only
		VD_KDBG("Load ATSC\n");
	    pu8VD_DSP = (U8*)&_u8VDDspImageForNTSC[0];
		u32VD_DSP_len = _u32VDDSPCodeLenForNTSC;
	}

    // Prevent over the max size of VD DSP code.
    //for(i=0;i<u32VD_DSP_len; i++)
    #if 0
    if(u32VD_DSP_len > 12288)
        u32VD_DSP_len = 12288;
    #endif

    for(i=0;i<u32VD_DSP_len; i++)
    {
        /*
        this is very import point.
        access VD_MCU_WDATA and VD_MCU_WDATA_CTRL must use BYTE access
        */
        #if 0
        _MHal_VD_W1B(VD_MCU_WDATA, pu8VD_DSP[i]);
        _MHal_VD_W1BM(VD_MCU_WDATA_CTRL, _BIT0, _BIT0);
        #endif
        _MHal_VD_W1B(VD_MCU_SRAM_WD, pu8VD_DSP[i]);

//??????????????????????????????????? Unknown Part 20090630
#if 0
        u32Timeout = 100;
        while(_MHal_VD_R2Bb(VD_MCU_REG(0x02), _BIT8) != 0x00)
        {
            u32Timeout--;
            if(u32Timeout == 0)
            {
                //VD_KDBG(" %d VD Firmware download (BIT8) [Time Out]\n", i);
                VD_ASSERT(FALSE);
                s8Return = -1;
                goto DWN_FAL;
            }
            udelay(10);
        }

        u32Timeout = 100;
        while(_MHal_VD_R2Bb(VD_MCU_REG(0x02), _BIT9) != _BIT9)
        {
            u32Timeout--;
            if(u32Timeout == 0)
            {
                //VD_KDBG(" %d  VD Firmware download (BIT9) [Time Out]\n", i);
                VD_ASSERT(FALSE);
                s8Return = -1;
                goto DWN_FAL;
            }
            udelay(10);
        }
#endif
//??????????????????????????????????? Unknown Part 20090630

    }

    if(i==u32VD_DSP_len)
    {
        VD_KDBG("\n^@^ DSP code loaded successfully\n");
    }


//DWN_FAL:

    /* disable down load code */
    //_MHal_VD_W2Bb(VD_MCU_REG(0x05), 0x00, _BIT0);
    _MHal_VD_W1BM(VD_MCU_KEY, 0x00, 0xF0);

    /* disable address auto increment */
    //_MHal_VD_W2Bb(VD_MCU_REG(0x04), 0x00, _BIT3);
    _MHal_VD_W1Rb(VD_MCU_ADDR_AUTO_INC, DISABLE, BIT(0));

    /* enable sram */
    _MHal_VD_W1Rb(VD_MCU_SRAM_EN, ENABLE, BIT(0));

    // release VD_MCU and run code on SDRAM
    // clear MCU2MIU_RST and enable MIU_FETCH_EN
    //XBYTE(L_BK_CHIPTOP(0x17)) = (XBYTE(L_BK_CHIPTOP(0x17)) & ~(0x0E)) | 0x0C; // <-<<< inverse VD MCU clock
    #if 0
    _MHal_VD_W1BM(L_BK_CHIPTOP(0x17), 0x04/*0x0C*/, 0x0E);	//dcsong 080826
    #endif

    /* release VD mcu */
    _MHal_VD_W1Rb(VD_MCU_RESET, 0, _BIT0);

    _MHal_VD_W1Rb(BK_AFEC_D4, 1, _BIT7);

	/* after DSP firmwae is downloaded complete. Driver set DSP firmware initial vertical line to 525.
	  This is for ATSC's power on/off issue.
	  LGE skystone 20090109
	 */
	if(MHal_VD_GetDSPVer() == VD_SYSTEM_NTSC) /* ATSC */
		_MHal_VD_W1BM (BK_AFEC_CF, (BIT0|BIT1), (BIT0|BIT1));
	else
		_MHal_VD_W1BM (BK_AFEC_CF, (0x00), (BIT0|BIT1));

	//mail box crash protection 2009-11-06
	MDrv_SYS_VDmcuSetType(VDMCU_DSP_VIF);

    return s8Return;
#endif
}
// Set Analog Color System - End   090309.

///-----------------------------------------------------------------------------
/// Initial video decoder
/// @param  void
/// @return void
/// @note
///     To initialize VDI(Video Signal Processor).
///     When turning power on or waking up form power saving mode,
///     this function should be called before using all funcions of VDI.
/// @type
///-----------------------------------------------------------------------------
void MHal_VD_Init(U32   u32VDSystem)//DSP branch
{

    U16 u16RegIndVD;
//    U16 u16ResetTimeOutVD;


	MHal_VD_SetDSPVer((VDSYSTEM)u32VDSystem);// Set Analog Color System - 090309.

    //VD_KDBG("MHal_VD_Init() %s\n", __TIME__);
    MHal_VD_SetClock(ENABLE); // 20091118


    /* initial 3D comb buffer */
    MDrv_SYS_GetMMAP(E_SYS_MMAP_VD_3DCOMB, &_u32Comb3DBufStartAdrVD, &_u32Comb3DBufLenVD);
    _u32Comb3DBufStartAdrVD = _u32Comb3DBufStartAdrVD;// & 0x1FFFFFF;

    _u8AbnormalCounterVD = 0; // <- UNUSED
    _u16DataHVD[0] = _u16DataHVD[1] = _u16DataHVD[2] = VD_HT_PAL;
    _u8HtotalDebounceVD = 0;
    _u8BackupNoiseMagVD = 0x20;
    //_etVideoSystemVD = E_VIDEOSTANDARD_NOTSTANDARD;
    atomic_set(&_aVideoSystemVD, E_VIDEOSTANDARD_NOTSTANDARD);

    /* Enable auto detection FSC mode. */
    _bAutoDetMode_HAL(set);

    /* initial force mode to invalid standard. */
    atomic_set(&_aForceVideoStandardVD, 0);

    /* initial vdmcu timer counter to 0. */
    atomic_set(&_aMCUResetCounterVD, 0);

    /* clear all VD driver states */
    _u32HalDrvStatesVD = 0;


    _u8PreNoiseMagVD = 0;
	_etInputPortTypeVD = INPUT_PORT_NONE;

#if 0  //20090630 Ethan
    //Reset page address=0x0000
    //XBYTE(VD_MCU_PAGE_ADDR)=0x20;   //reset page =0x00 <-- BIT5(0x20) is a trigger bit for page write
    _MHal_VD_W1B(VD_MCU_PAGE_ADDR, 0x20);

    /* wait trigger bit to clean */
    for ( u16ResetTimeOutVD=0;u16ResetTimeOutVD<0xffff;u16ResetTimeOutVD++ )
    {
        if ( (_MHal_VD_R1B(VD_MCU_PAGE_ADDR) & 0x20 ) == 0x00 )
            break;
    }


    // reset VD mcu address to 0x0000
    //XBYTE(VD_MCU_OFFSET_ADDR_0)=0x00;
    _MHal_VD_W1B(VD_MCU_OFFSET_ADDR_0,0x00);
    //XBYTE(VD_MCU_OFFSET_ADDR_1)=0x80; <-- BIT15(0x80) is a trigger bit for sram write
    _MHal_VD_W1B(VD_MCU_OFFSET_ADDR_1,0x80);

    /* wait trigger bit to clean */
    for ( u16ResetTimeOutVD=0;u16ResetTimeOutVD<0xffff;u16ResetTimeOutVD++ )
    {
        if ( (_MHal_VD_R1B(VD_MCU_OFFSET_ADDR_1) & 0x80 ) == 0x00 )
        {
            break;
        }
    }
#endif

    _MHal_VD_W1B(BK_AFEC_0A, 0x10);
    _MHal_VD_W1B(BK_AFEC_0F, 0x48);
    _MHal_VD_W1Rb(BK_AFEC_14, ENABLE, _BIT7);
    _MHal_VD_W1Rb(BK_AFEC_14, DISABLE, _BIT7);

    #ifdef COMB_3D
    if (0 == Chip_Query_Rev())
    {
        _MHal_VD_W1B(BK_COMB_3F, (_u32Comb3DBufStartAdrVD >>  4) & 0xFF);
        _MHal_VD_W1B(BK_COMB_3E, (_u32Comb3DBufStartAdrVD >> 12) & 0xFF);
        _MHal_VD_W1B(BK_COMB_3D, (_u32Comb3DBufStartAdrVD >> 20) & 0xFF);
        _MHal_VD_W1B(BK_COMB_88, ((_u32Comb3DBufStartAdrVD+_u32Comb3DBufLenVD)>>20) & 0xFF);
        _MHal_VD_W1B(BK_COMB_89, ((_u32Comb3DBufStartAdrVD+_u32Comb3DBufLenVD)>>12) & 0xFF);
        _MHal_VD_W1B(BK_COMB_8A, ((_u32Comb3DBufStartAdrVD+_u32Comb3DBufLenVD)>>4) & 0xFF);
    }
    else
    {
        _MHal_VD_W1B(BK_COMB_3F, (_u32Comb3DBufStartAdrVD >>  3) & 0xFF);
        _MHal_VD_W1B(BK_COMB_3E, (_u32Comb3DBufStartAdrVD >> 11) & 0xFF);
        _MHal_VD_W1B(BK_COMB_3D, (_u32Comb3DBufStartAdrVD >> 19) & 0xFF);
        _MHal_VD_W1B(BK_COMB_88, ((_u32Comb3DBufStartAdrVD+_u32Comb3DBufLenVD)>>19) & 0xFF);
        _MHal_VD_W1B(BK_COMB_89, ((_u32Comb3DBufStartAdrVD+_u32Comb3DBufLenVD)>>11) & 0xFF);
        _MHal_VD_W1B(BK_COMB_8A, ((_u32Comb3DBufStartAdrVD+_u32Comb3DBufLenVD)>>3) & 0xFF);
    }
    _MHal_VD_W1B(BK_COMB_4C, 0x80);
    _MHal_VD_W1B(BK_COMB_44, 0x90);
    #endif

    for(u16RegIndVD=0; u16RegIndVD < (sizeof(gVdRegInit)/sizeof(VD_REG_TABLE_t)); u16RegIndVD++)
    {
        if ( gVdRegInit[u16RegIndVD].u32RegIdx == (U32)_END_OF_TBL_ ) // check end of table
            break;
        _MHal_VD_W1B( gVdRegInit[u16RegIndVD].u32RegIdx, gVdRegInit[u16RegIndVD].u08RegVal ); // write register
    }

#if 1
#if defined(COMB_3D)
    {
        B16 i;
        #define COMB_3D_BUF_SIZE COMB_3D_BUF_LEN//0x69999
        //#define COMB_3D_BUF_SIZE  0x69999

        //XBYTE[BK_COMB_6F] = XBYTE[BK_COMB_3F] + (U8)(COMB_3D_BUF_SIZE & 0xFF);
        _MHal_VD_W1B(BK_COMB_6F, _MHal_VD_R1B(BK_COMB_3F) + (U8)(COMB_3D_BUF_SIZE & 0xFF));

        //i = (XBYTE[BK_COMB_6F] < XBYTE[BK_COMB_3F]) ? 1 : 0; // carry-in
        i = (_MHal_VD_R1B(BK_COMB_6F) < _MHal_VD_R1B(BK_COMB_3F)) ? 1 : 0; // carry-in

        //XBYTE[BK_COMB_6D] = XBYTE[BK_COMB_3E] + (U8)((COMB_3D_BUF_SIZE >> 8) & 0xFF) + i;
        _MHal_VD_W1B(BK_COMB_6D, _MHal_VD_R1B(BK_COMB_3E) + (U8)((COMB_3D_BUF_SIZE >> 8) & 0xFF) + i);

        //i = (XBYTE[BK_COMB_6D] < XBYTE[BK_COMB_3E]) ? 1 : 0; // carru-in
        i = (_MHal_VD_R1B(BK_COMB_6D) < _MHal_VD_R1B(BK_COMB_3E)) ? 1 : 0; // carru-in

        //XBYTE[BK_COMB_6B] = XBYTE[BK_COMB_3D] + (U8)((COMB_3D_BUF_SIZE >> 16) & 0xFF) + i;
        _MHal_VD_W1B(BK_COMB_6B , _MHal_VD_R1B(BK_COMB_3D) + (U8)((COMB_3D_BUF_SIZE >> 16) & 0xFF) + i);
    }
#endif
#endif

    _MHal_VD_W1B( BK_VBI_70, 0x80); // enable VPS/WSS (Video Programming System / Wide Screen Signaling)
    _MHal_VD_W1BM(BK_AFEC_39, 0x03, _BIT1 | _BIT0);

#if 0
#if (MST_VD_PATCH_07112001)
    {
        if (_MHal_VD_R1B(BYTE2REAL(0x1202)) & _BIT4) // DDR 32
        {
            _MHal_VD_W1Rb(BK_COMB_13, DISABLE, _BIT1);
        }
    }
#endif // MST_VD_PATCH_07112001
#endif
    // Set Analog Color System - Start 090309.
#if 0
    if(MHal_VD_LoadDsp()>=0) // BY 20090710 remove me after switch source load code is ok
    {
        printk("Load DSP code ok\n");
    }
    else
    {
        printk("Load DSP code fail\n");
    }
    // Set Analog Color System - End   090309.
#endif
}

///-----------------------------------------------------------------------------
/// Does sync be locked?
/// @param  void
/// @return B16:
/// - TRUE  : Vertical sync is stable.
/// - FALSE : Vertical sync is unstable or not detected.
/// @note
///     To get whether vetical sync is stable or not.
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
B16 MHal_VD_IsSyncLocked(void)
{
    //VD_KDBG("MHal_VD_IsSyncLocked()\n"); // <-<<<

    if(IS_BITS_SET(MHal_VD_GetStatus(), VD_SYNC_LOCKED) )
    {
        return TRUE;
    }
    return FALSE;
}

///-----------------------------------------------------------------------------
/// Does sync be detected?
/// @param  void
/// @return B16:
/// - TRUE  : Horizontal Sync is detected.
/// - FALSE : There is no sync.
/// @note
///     To get whether horizontal sync is detected or not.
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
B16 MHal_VD_IsSyncDetected(void)
{
    if (IS_BITS_SET(MHal_VD_GetStatus(), VD_HSYNC_LOCKED) )
    {
	   /* After DSP found h sync is locked.
		 Driver releases DSP firmware to find singal's vertical line.
		 This is for ATSC's power on/off issue.
		 LGE skystone 20090109
		*/
	   if(MHal_VD_GetDSPVer() == VD_SYSTEM_NTSC) /* ATSC */// Set Analog Color System - 090309.
		  _MHal_VD_W1BM (BK_AFEC_CF, (0), (BIT0|BIT1));

        return TRUE;
    }
    return FALSE;
}

///-----------------------------------------------------------------------------
/// Is signal format interlace
/// @param  void
/// @return B16:
/// - TRUE  : Signal is interlaced.
/// - FALSE : Signal is progressive.
/// @note
///     To get whether signal is interlaced or progressive.
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
B16 MHal_VD_IsSignalInterlaced(void)
{
    if ( IS_BITS_SET(MHal_VD_GetStatus(), VD_INTERLACED | VD_HSYNC_LOCKED | VD_STATUS_RDY) )
    {
        return TRUE;
    }

    return FALSE;
}


///-----------------------------------------------------------------------------
/// Does color be turned on?
/// @param  void
/// @return B16:
/// - TRUE  : Color is correct.
/// - FALSE : Color is disappeared.
/// @note
///     To get whether color is correct or not.
///     Color can be disappeared by color killer or no burst or misjudgment of standard or etc.
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
B16 MHal_VD_IsColorOn(void)
{
    if ( IS_BITS_SET(MHal_VD_GetStatus(), VD_COLOR_LOCKED | VD_HSYNC_LOCKED | VD_STATUS_RDY) )
    {
        return TRUE;
    }
    return FALSE;
}

///-----------------------------------------------------------------------------
/// Is video standard valid?
/// @param  void
/// @return B16:
/// - TRUE  : Standard is valid.
/// - FALSE : Standard is not valid.
/// @note   This function will return TRUE if current video signal format which is forced is valid.
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
#if 1
B16 MHal_VD_IsStandardValid(void)
{
    U32     u32ForceVideoStandard;

    #if (TEST_VD_DSP)
        if( !IS_BITS_SET(MHal_VD_GetStatus(), VD_STANDARD_VALID) )
            return FALSE;
        else
        return TRUE;

    #else
        /* video signal scan automatically */
        if( _bAutoDetMode_HAL(test) )
        {
            if( !IS_BITS_SET(MHal_VD_GetStatus(), VD_STATUS_RDY) )
                return FALSE;
            else
                return TRUE;
        }
        /* video signal scan manually */
        else
        {
            //#define IS_STANDARD_VALID_STABLE_CNT 1
            //static U8 u8StableCnt = 0;
            U32 u32VDStatus = (MHal_VD_GetStatus() & VD_MODE_STANDARD_MASK);
            //R_VAR_LOCKED(u32ForceVideoStandard = _u32ForceVideoStandardVD, _u32ForceVideoStandardVD);
            u32ForceVideoStandard = atomic_read(&_aForceVideoStandardVD);
            if (u32VDStatus == u32ForceVideoStandard)
                return TRUE;
            else
                return FALSE;
            #if 0
            // debounce?
            if (u32VDStatus == u32ForceVideoStandard)
            {
                u8StableCnt = 0;
                return TRUE;
            }
            else
            {
                if (u8StableCnt > IS_STANDARD_VALID_STABLE_CNT)
                {
                    return FALSE;
                }
                else
                {
                    u8StableCnt++;
                    return TRUE;
                }
            }
            #endif
        }

    #endif
}
#endif
///-----------------------------------------------------------------------------
/// Is fast blanking pin high?
/// @param  void
/// @return B16:
/// - TRUE  : Fast blanking pin of scart is high.
/// - FALSE : Fast blanking pin of scart is low.
/// @note   To get whether fast blanking pin of scart is high or low.
///         This function can be used to know whether RGB is coming or not from scart.
///         <dummy function>
///-----------------------------------------------------------------------------
B16 MHal_VD_IsFastBlankingPinHigh(void)
{
    // need touch.
    return FALSE;
}

///-----------------------------------------------------------------------------
/// Get vertical frequency of input signal.
/// @param  void
/// @return VIDEOFREQ:
/// - E_VIDEO_FQ_NOSIGNAL   : There is no signal.
/// - E_VIDEO_FQ_50Hz       : Vertical frequency of input signal is 50Hz.
/// - E_VIDEO_FQ_60Hz       : Vertical frequency of input signal is 60Hz.
/// @note
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
VIDEOFREQ MHal_VD_GetVerticalFreq(void)
{
    U16 u16VDStatusVD;

    u16VDStatusVD = MHal_VD_GetStatus();

    if( IS_BITS_SET(u16VDStatusVD, VD_HSYNC_LOCKED | VD_STATUS_RDY) )
    {
        if( VD_VSYNC_50HZ & u16VDStatusVD )
        {
            return E_VIDEO_FQ_50Hz;
        }
        else
        {
            return E_VIDEO_FQ_60Hz;
        }
    }
    return E_VIDEO_FQ_NOSIGNAL;
}


///-----------------------------------------------------------------------------
/// To set brightness.
/// @param  u8PercentVD:
/// - 0 ~ 100
/// @return void
/// @note
///     <dummy function>
///-----------------------------------------------------------------------------
void MHal_VD_SetBrightness(U8 u8PercentVD)
{
#if 0
    BYTE cValue;

    if( cPercent > 100 )
    {
        return;
    }

    cValue = ((WORD) cPercent * 255+ 100 / 2) / 100;

    if( cValue > 0xFF )
    {
        cValue = 0xFF;
    }

    _MHal_VD_W1B( BK_COMB_74, cValue );
#else
    u8PercentVD = u8PercentVD;
    // _MHal_VD_W1B( BK_COMB_74, cPercent );

#endif
}



///-----------------------------------------------------------------------------
/// To set contrast.
/// @param  u8PercentVD:
/// - 0 ~ 100
/// @return void
/// @note
///     <dummy function>
///-----------------------------------------------------------------------------
void MHal_VD_SetContrast(U8 u8PercentVD)
{
#if 0
    BYTE cValue;

    if( cPercent > 100 )
    {
        return;
    }

    cValue = ((WORD) cPercent * 255+ 100 / 2) / 100;

    if( cValue > 0xFF )
    {
        cValue = 0xFF;
    }

    _MHal_VD_W1B( BK_COMB_73, cValue );
#else
    u8PercentVD = u8PercentVD;
    // _MHal_VD_W1B( BK_COMB_73, cPercent );

#endif
}

///-----------------------------------------------------------------------------
/// To set color (Saturation).
/// @param  u8PercentVD:
/// - 0 ~ 100
/// @return void
/// @note
///     <dummy function>
///-----------------------------------------------------------------------------
void MHal_VD_SetColor(U8 u8PercentVD)
{
#if 0
    BYTE cValue;

    if( cPercent > 100 )
    {
        return;
    }

    cValue = ((WORD) cPercent * 255+ 100 / 2) / 100;

    if( cValue > 0xFF )
    {
        cValue = 0xFF;
    }

    _MHal_VD_W1B( BK_COMB_75, cValue );
#else
    u8PercentVD = u8PercentVD;
    // _MHal_VD_W1B( BK_COMB_75, cPercent );

#endif
}


///-----------------------------------------------------------------------------
/// To set hue.
/// @param  u8PercentVD:
/// - 0 ~ 100 , 50% is center.
/// @return void
/// @note
///     <dummy function>
///-----------------------------------------------------------------------------
void MHal_VD_SetHue(BYTE u8PercentVD)
{
    // need touch.
    u8PercentVD = u8PercentVD; // to avoid warning.
}

///-----------------------------------------------------------------------------
/// To set sharpness.
/// @param  u8PercentVD:
/// - 0 ~ 100
/// @return void
/// @note
///     <dummy function>
///-----------------------------------------------------------------------------
void MHal_VD_SetSharpness(U8 u8PercentVD)
{
    // need touch.
    u8PercentVD = u8PercentVD; // to avoid warning.
}


///-----------------------------------------------------------------------------
/// To set black level
/// @param  u16BlacklevelVD:
/// @return void
/// @note
///     <dummy function>
///-----------------------------------------------------------------------------
void MHal_VD_SetBlacklevel(U16 u16BlacklevelVD)
{
    u16BlacklevelVD=u16BlacklevelVD;
/*//marked, recomment by SC.Wu
    _MHal_VD_W1Rb( BK_AFEC_38, DISABLE, _BIT7 );

    if( HIBYTE(u16Blacklevel) )
        _MHal_VD_W1Rb( BK_AFEC_38, ENABLE, _BIT6 );
    else
        _MHal_VD_W1Rb( BK_AFEC_38, DISABLE, _BIT6 );

    _MHal_VD_W1B( BK_AFEC_3A, LOBYTE(u16Blacklevel) );
*/
}


///-----------------------------------------------------------------------------
/// To set input.
/// @param  etSourceTypeVD:
/// - VIDEOSOURCE_TYPE
/// @return void
/// @note
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
void MHal_VD_SetInput(VIDEOSOURCE_TYPE etSourceTypeVD)
{
#if 1
    U8  u8YMuxVD;
    U8  u8CMuxVD;

    //printk("MHal_VD_SetInput\r\n"); //20091118
    /* INPUT_PORT_TYPE only used in multiplexer ADC */
    INPUT_PORT_TYPE tInputPort;
    MHal_VD_SetClock(ENABLE); // 20091118 BY, add power control

    etSourceTypeVD &= 0x0F;

    switch( etSourceTypeVD )
    {
    case E_VIDEOSOURCE_ATV:
        tInputPort = INPUT_PORT_MS_CVBS0;
        u8YMuxVD = INPUT_TV_YMUX;
        u8CMuxVD = MSVD_CMUX_NONE;
        break;

    case E_VIDEOSOURCE_CVBS1:
        tInputPort = INPUT_PORT_MS_CVBS1;
        u8YMuxVD = INPUT_AV_YMUX;
        u8CMuxVD = MSVD_CMUX_NONE;
        break;

    case E_VIDEOSOURCE_CVBS2:
        tInputPort = INPUT_PORT_MS_CVBS2;
        u8YMuxVD = INPUT_AV2_YMUX;
        u8CMuxVD = MSVD_CMUX_NONE;
        break;

    case E_VIDEOSOURCE_CVBS3:
        tInputPort = INPUT_PORT_MS_CVBS3;
        u8YMuxVD = INPUT_AV3_YMUX;
        u8CMuxVD = MSVD_CMUX_NONE;
        break;

    case E_VIDEOSOURCE_SVIDEO1:
        #ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
        MHal_VD_COMB_Set3dCombMid(DISABLE);
        #endif
        tInputPort = INPUT_PORT_MS_SV0;
        u8YMuxVD = INPUT_SV_YMUX;
        u8CMuxVD = INPUT_SV_CMUX;
        break;

    case E_VIDEOSOURCE_SVIDEO2:
        #ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
        MHal_VD_COMB_Set3dCombMid(DISABLE);
        #endif
        tInputPort = INPUT_PORT_MS_SV1;
        u8YMuxVD = INPUT_SV2_YMUX;
        u8CMuxVD = INPUT_SV2_CMUX;
        break;

    case E_VIDEOSOURCE_SCART1:
        tInputPort = INPUT_PORT_AV_SCART0;
        u8YMuxVD = INPUT_SCART_YMUX;
        u8CMuxVD = INPUT_SCART_CMUX;
        break;

    case E_VIDEOSOURCE_SCART2:
        tInputPort = INPUT_PORT_AV_SCART1;
        u8YMuxVD = INPUT_SCART2_YMUX;
        u8CMuxVD = INPUT_SCART2_CMUX;
        break;

    default:
#if 1
		MHal_VD_McuReset();
#endif
        return;
    }

    _u8OneshotFlag = 0;

	_etVideoLastStandardVD=E_VIDEOSTANDARD_NOTSTANDARD;
    _etInputPortTypeVD=tInputPort;

    /* select Mux and power on */
    //MHal_VD_ADC_PowerOn(MHal_VD_ADC_SetMUX(_etInputPortTypeVD, u8YMuxVD, u8CMuxVD));
    if (IsUseInternalScartPort(_etInputPortTypeVD))
        SwitchRGBToSCART();
    else
        SwitchRGBToDSUB();
    if (etSourceTypeVD == E_VIDEOSOURCE_SCART1)
    {
        /* power on fast blanking ADC, select FB MUX */
     	_MHal_VD_W1BM (L_BK_ADC_ATOP(0x40), INPUT_SCART_FB_MUX<<4, 0x30);
        //_MHal_VD_W1BM (L_BK_ADC_ATOP(0x40), 0, 0x30);
        _MHal_VD_W1B (H_BK_ADC_ATOP(0x43), 0x4C); // i.e. ATOP 0x85
        _MHal_VD_W1B (L_BK_ADC_ATOP(0x42), 0xEC); // i.e. 0x83
    }
    else if (etSourceTypeVD == E_VIDEOSOURCE_SCART2)
    {
        /* power on fast blanking ADC, select FB MUX */
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x40), 0, 0x30);
        _MHal_VD_W1B (L_BK_ADC_ATOP(0x42), 0xEC); // i.e. 0x83
    }
    else
    {
        /* power down fast blanking ADC */
        _MHal_VD_W1Rb (L_BK_ADC_ATOP(0x40), ENABLE, _BIT6); // i.e. 0x80[6]
        _MHal_VD_W1B (L_BK_ADC_ATOP(0x42), 0xEC); // i.e. 0x83
    }
#endif

    switch( etSourceTypeVD )
    {
    case E_VIDEOSOURCE_ATV:
        _MHal_VD_W1Rb (BK_COMB_10, DISABLE, _BIT7); // <- SET COMB
        _MHal_VD_W1Rb (BK_COMB_60, ENABLE, _BIT6); // <- SET COMB
        //_MHal_VD_W1Rb (BK_COMB_60, DISABLE, _BIT6); // <- SET COMB

        _MHal_VD_W1B( BK_AFEC_1A, ( _MHal_VD_R1B( BK_AFEC_1A ) ) & ~0xC0 ); // disable SV input
        _MHal_VD_W1Rb( BK_AFEC_1F, DISABLE, _BIT7); // disable clamp C

        _MHal_VD_W1B( BK_AFEC_7F, 0x65); // Switch to Comb Y/U/V 444 input.
        _MHal_VD_W1BM(BK_AFEC_A2, 0x00, _BIT4 | _BIT5);	// 20080820 swwo LGE Position Shift ??제 ??정

        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x03), _BIT1, _BIT1 | _BIT0);  //20090717 Mmichu
        break;

    case E_VIDEOSOURCE_CVBS1:
    case E_VIDEOSOURCE_CVBS2:
    case E_VIDEOSOURCE_CVBS3:
        _MHal_VD_W1Rb (BK_COMB_10, DISABLE, _BIT7); // <- SET COMB
        _MHal_VD_W1Rb (BK_COMB_60, DISABLE, _BIT6); // <- SET COMB

        _MHal_VD_W1B( BK_AFEC_1A, ( _MHal_VD_R1B( BK_AFEC_1A ) ) & ~0xC0 ); // disable SV input
        _MHal_VD_W1Rb( BK_AFEC_1F, DISABLE, _BIT7); // disable clamp C

        _MHal_VD_W1B( BK_AFEC_7F, 0x65); // Switch to Comb Y/U/V 444 input.
        _MHal_VD_W1BM(BK_AFEC_A2, 0x00, _BIT4 | _BIT5);	// 20080820 swwo LGE Position Shift ??제 ??정

        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x03), _BIT1, _BIT1 | _BIT0); // Fitch

        break;

    case E_VIDEOSOURCE_SVIDEO1:
    case E_VIDEOSOURCE_SVIDEO2:
        _MHal_VD_W1Rb (BK_COMB_10, ENABLE, _BIT7); // <- SET COMB
        _MHal_VD_W1Rb (BK_COMB_60, DISABLE, _BIT6); // <- SET COMB

        _MHal_VD_W1B( BK_AFEC_1A, ( _MHal_VD_R1B(BK_AFEC_1A) )| 0xC0); // Enable SV input
        _MHal_VD_W1Rb( BK_AFEC_1F, ENABLE, _BIT7); // Enable clamp C

        _MHal_VD_W1B( BK_AFEC_7F, 0x65); // Switch to Comb Y/U/V 444 input. <- 0x65 used in Saturn
        _MHal_VD_W1BM(BK_AFEC_A2, 0x20, _BIT4 | _BIT5);

        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x03), _BIT1 | _BIT0, _BIT1 | _BIT0); // Fitch
        break;

    case E_VIDEOSOURCE_SCART1:
    case E_VIDEOSOURCE_SCART2:
        _MHal_VD_W1Rb (BK_COMB_10, DISABLE, _BIT7); // <- SET COMB
        _MHal_VD_W1Rb (BK_COMB_60, DISABLE, _BIT6); // <- SET COMB

        _MHal_VD_W1B( BK_AFEC_1A, ( _MHal_VD_R1B( BK_AFEC_1A ) ) & ~0xC0 ); // disable SV input
        _MHal_VD_W1Rb( BK_AFEC_1F, DISABLE, _BIT7); // disable clamp C

        #if 0
        _MHal_VD_W1B (H_BK_ADC_ATOP(0x44), 0xFF); // i.e. ATOP 0x88
        _MHal_VD_W1B (L_BK_ADC_ATOP(0x44), 0x18); // i.e. ATOP 0x87
        _MHal_VD_W1B (L_BK_ADC_ATOP(0x41), 0x18); // i.e. ATOP 0x81
        _MHal_VD_W1B (H_BK_ADC_ATOP(0x41), 0x40); // i.e. ATOP 0x82
        _MHal_VD_W1B (H_BK_ADC_ATOP(0x43), 0x4C); // i.e. ATOP 0x85
        _MHal_VD_W1B (L_BK_ADC_ATOP(0x43), 0x30); // i.e. ATOP 0x84

        _MHal_VD_W1B (L_BK_ADC_ATOP(0x1C), _MHal_VD_R1B(L_BK_ADC_ATOP(0x1C)) & 0xE0); // i.e. ATOP 0x36 set SOG level (BIT4~0) to 0
        _MHal_VD_W1B (L_BK_ADC_ATOP(0x2C), 0x00); // i.e. ATOP 0x58 set VClamp to VCLAMP_RGB_ClampGnd (i.e. 0)
        _MHal_VD_W1B (H_BK_ADC_ATOP(0x1C), 0x40); // i.e. ATOP 0x37
	    _MHal_VD_W1B (L_BK_ADC_ATOP(0x4C), 0xA1); // i.e. ATOP 0x98

        _MHal_VD_W1Rb (H_BK_ADC_ATOP(0x2D), ENABLE, _BIT6); // i.e. ATOP 0x5C disable ADC clamp from VD

        _MHal_VD_W1B (L_BK_ADC_DTOP(0x0B), 0xA0); // i.e. DTOP 0x14 set clamp placement
        _MHal_VD_W1B (H_BK_ADC_DTOP(0x0B), 0x40); // i.e. DTOP 0x15 set clamp duration
        _MHal_VD_W1B (L_BK_ADC_DTOP(0x07), 0x8A); // i.e. DTOP 0x0D
        _MHal_VD_W1B (L_BK_ADC_DTOP(0x10), 0x00); // i.e. DTOP 0x20
        #endif

        if (0 == Chip_Query_Rev())
        {
        _MHal_VD_W1B(BK_AFEC_7F, 0x65); // Switch to Comb Y/U/V 444 input.  //Set BK_AFEC_7F as 0x65 20090731
        }
        else
        {
            _MHal_VD_W1B(BK_AFEC_7F, 0x66); // Switch to Comb Y/U/V 444 input.
        }
        _MHal_VD_W1BM(BK_AFEC_A2, 0x00, _BIT4 | _BIT5);	// 20080820 swwo LGE Position Shift ??제 ??정

        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x03), _BIT1 | _BIT0, _BIT1 | _BIT0); // Fitch
        break;
        default:
        // The input of VD are ATV, CVBS, SVIDEO, SCART.
        break;
    }

    if (etSourceTypeVD == E_VIDEOSOURCE_ATV)
    {
        _EnableCVBSLPF(ENABLE);
        /*
        #if(FRONTEND_IF_DEMODE_TYPE == MSTAR_VIF)
            //printf("-----TV-------\n");
            _MHal_VD_W1Rb(BK_AFEC_CF,ENABLE,_BIT7);
            _MHal_VD_W1Rb(L_BK_CHIPTOP(0x12), ENABLE, _BIT4);
            _MHal_VD_W1Rb(BK_AFEC_19,ENABLE,_BIT2);
            _MHal_VD_W1B(BK_AFEC_8F,0x1D);
            _MHal_VD_W1B(BK_AFEC_20,0xBC);
            _MHal_VD_W1B(BK_AFEC_21,0x1B);
            //_MHal_VD_W1BM(BK_AFEC_CF,0x84,0x84); // Enable [7]VIF and [2]double AGC gain
        #endif
        */
        #if (VD_GAIN_OF_RF_SEL == VD_USE_FIX_GAIN)
            VD_KDBG("RF USE FIX GAIN\n");
            _MHal_VD_W1BM(BK_AFEC_43, (AFE_AGC_DISABLE << 5), BITMASK(6:5));
            _MHal_VD_W1BM(BK_AFEC_40, (VD_RF_COARSE_GAIN_USED << 4),    BITMASK(5:4));
            _MHal_VD_W1B(BK_AFEC_44, VD_RF_FINE_GAIN_H_USED);
            _MHal_VD_W1BM(BK_AFEC_3F, VD_RF_FINE_GAIN_L_USED,         BITMASK(3:0));
        #else // (VD_GAIN_OF_AV_SEL == VD_USE_AUTO_GAIN)
            _MHal_VD_W1BM(BK_AFEC_43, (AFE_AGC_ENABLE << 5), BITMASK(6:5));
        #endif

		_MHal_VD_W1B(BK_COMB_39, 0x02); //lachesis_090117 by victor

    }
    else
    {
        _EnableCVBSLPF(DISABLE);
        /*
        #if(FRONTEND_IF_DEMODE_TYPE == MSTAR_VIF)
            //printf("-----AV-------\n");
            _MHal_VD_W1Rb(BK_AFEC_CF,DISABLE,_BIT7);
            _MHal_VD_W1Rb(L_BK_CHIPTOP(0x12), DISABLE, _BIT4);
            _MHal_VD_W1B(BK_AFEC_8F,0x19);
            _MHal_VD_W1B(BK_AFEC_20,0xB0);
            _MHal_VD_W1B(BK_AFEC_21,0x19);
            //_MHal_VD_W1BM(BK_AFEC_CF,0x00,0x84); // Disable [7]VIF and [2]double AGC gain
        #endif
        */
        #if (VD_GAIN_OF_AV_SEL == VD_USE_FIX_GAIN)
            _MHal_VD_W1BM(BK_AFEC_43, (AGC_DISABLE << 5), BITMASK(6:5));
            _MHal_VD_W1BM( BK_AFEC_40, (VD_AV_COARSE_GAIN_USED << 4),    BITMASK(5:4));
            _MHal_VD_W1B(     BK_AFEC_44, VD_AV_FINE_GAIN_H_USED);
            _MHal_VD_W1BM( BK_AFEC_3F, VD_AV_FINE_GAIN_L_USED,         BITMASK(3:0));
        #else // (VD_GAIN_OF_AV_SEL == VD_USE_AUTO_GAIN)
            _MHal_VD_W1BM(BK_AFEC_43, (AFE_AGC_ENABLE << 5), BITMASK(6:5));
        #endif
		_MHal_VD_W1B(BK_COMB_39, 0x00);	//lachesis_090117 by victor
    }


//    _MHal_VD_InitClock();  //Temp solution. To fix MVD/VD is crashed after inputsource switch 20090714 Ethan

    if(MHal_VD_LoadDsp()>=0)
    {
        VD_KDBG("Load DSP code ok\n");
    }
    else
    {
        VD_KDBG("Load DSP code fail\n");
    }


    // VD MCU Reset
    /* system don't do reset in here. do it outside.*/
    MHal_VD_McuReset();

    MHal_VD_SIF_ADC_Reset();  //Add SIF ADC reset after VDDSP loaded 20090803 Ethan

}



///-----------------------------------------------------------------------------
/// To select standard.
/// @param  etSourceVD:
/// - VIDEOSOURCE_TYPE
/// @param  etStandardVD:
/// - VIDEOSTANDARD_TYPE
/// @param  bAutoTurningVD:
/// - TRUE  : system is in turning
/// - FALSE : system is not in turning
/// @return void
/// @note
/// @type   thread and SW interrupt safe.
///-----------------------------------------------------------------------------
void MHal_VD_SetVideoStandard(VIDEOSOURCE_TYPE etSourceVD, VIDEOSTANDARD_TYPE etStandardVD, B16 bIsTurningVD)
{
    //_etVideoSystemVD = etStandardVD;
    atomic_set(&_aVideoSystemVD, etStandardVD);
    switch (etStandardVD)
    {
    default: // case E_VIDEOSTANDARD_PAL_BGHI:
        // 1135
        _SetHTotal(VD_HT_PAL * (VD_USE_FB + 1));
        _MHal_VD_W1B(BK_AFEC_7B, 0xB6); // 732, 656_HDES
        _u16LatchHVD = VD_HT_PAL;
        atomic_set(&_aForceVideoStandardVD,(VD_VSYNC_50HZ | VD_PAL_SWITCH | VD_FSC_4433 | VD_STATUS_RDY));
        _MHal_VD_SetFSCType(FSC_MODE_PAL);
        //VD_KDBG1("SIG_PAL\n");
        break;

    case E_VIDEOSTANDARD_NTSC_44:
    case E_VIDEOSTANDARD_PAL_60:
        // 1127
        _SetHTotal(VD_HT_NTSC_443 * (VD_USE_FB + 1));
        _MHal_VD_W1B(BK_AFEC_7B, 0xB4); // 724, 656_HDES
        _u16LatchHVD = VD_HT_NTSC_443;

        //_u32ForceVideoStandardVD = (((etStandardVD == E_VIDEOSTANDARD_NTSC_44) ? 0 : VD_PAL_SWITCH) | VD_FSC_4433 | VD_STATUS_RDY);
        atomic_set(&_aForceVideoStandardVD, (((etStandardVD == E_VIDEOSTANDARD_NTSC_44) ? 0 : VD_PAL_SWITCH) | VD_FSC_4433 | VD_STATUS_RDY));

        _MHal_VD_SetFSCType((etStandardVD == E_VIDEOSTANDARD_NTSC_44) ? FSC_MODE_NTSC_443 : FSC_MODE_PAL_60);
        //(etStandardVD == E_VIDEOSTANDARD_NTSC_44) ? VD_KDBG1("SIG_NTSC_443\n") : VD_KDBG1("SIG_PAL_60\n");
        break;

    case E_VIDEOSTANDARD_SECAM:
	    if ( bIsTurningVD )
	    {
        	_SetHTotal(VD_HT_PAL * (VD_USE_FB + 1));
        }
	    else
	    {
        	_SetHTotal(VD_HT_SECAM * (VD_USE_FB + 1));
        }

        _MHal_VD_W1B(BK_AFEC_7B, 0xB2); // 716, 656_HDES
        _u16LatchHVD = VD_HT_SECAM;
        //_u32ForceVideoStandardVD = (VD_VSYNC_50HZ | VD_FSC_4285 | VD_STATUS_RDY);
        atomic_set(&_aForceVideoStandardVD, (VD_VSYNC_50HZ | VD_FSC_4285 | VD_STATUS_RDY));

        _MHal_VD_SetFSCType(FSC_MODE_SECAM);
        //VD_KDBG1("SIG_SECAM\n");
        break;

    case E_VIDEOSTANDARD_PAL_M:
        // 909
        _SetHTotal(VD_HT_PAL_M * (VD_USE_FB + 1));
        _MHal_VD_W1B(BK_AFEC_7B, 0x8E); // 572, 656_HDES
        _u16LatchHVD = VD_HT_PAL_M;
        //_u32ForceVideoStandardVD = (VD_PAL_SWITCH | VD_FSC_3575 | VD_STATUS_RDY);
        atomic_set(&_aForceVideoStandardVD, (VD_PAL_SWITCH | VD_FSC_3575 | VD_STATUS_RDY));
        _MHal_VD_SetFSCType(FSC_MODE_PAL_M);
        //VD_KDBG1("SIG_PAL_M\n");
        break;

    case E_VIDEOSTANDARD_PAL_N:
        // 917
        _SetHTotal(VD_HT_PAL_NC * (VD_USE_FB + 1));
        _MHal_VD_W1B(BK_AFEC_7B, 0x93); // 592, 656_HDES
        _u16LatchHVD = VD_HT_PAL_NC;
        //_u32ForceVideoStandardVD = (VD_VSYNC_50HZ | VD_PAL_SWITCH | VD_FSC_3582 | VD_STATUS_RDY);
        atomic_set(&_aForceVideoStandardVD, (VD_VSYNC_50HZ | VD_PAL_SWITCH | VD_FSC_3582 | VD_STATUS_RDY));
        _MHal_VD_SetFSCType(FSC_MODE_PAL_N);
        //VD_KDBG1("SIG_PAL_NC\n");
        break;

    case E_VIDEOSTANDARD_NTSC_M: // 910
        _SetHTotal(VD_HT_NTSC * (VD_USE_FB + 1));
        _MHal_VD_W1B(BK_AFEC_7B, 0x6D); // 440, 656_HDES
        _u16LatchHVD = VD_HT_NTSC;
        //_u32ForceVideoStandardVD = (VD_FSC_3579 | VD_STATUS_RDY);
        atomic_set(&_aForceVideoStandardVD, (VD_FSC_3579 | VD_STATUS_RDY));
        _MHal_VD_SetFSCType(FSC_MODE_NTSC);
        //VD_KDBG1("SIG_NTSC\n");
        break;
    }

    // _MHal_VD_SetAutoDetetion(FSC_AUTO_DET_DISABLE);

    #if(PQ_IN_VD_ENABLE)
    _MHal_VD_W1BM(BK_COMB_1C, 0xE0, 0xE0);
    #endif

    if(IsSrcTypeSV(etSourceVD))
    {
        if (E_VIDEOSTANDARD_NTSC_M == etStandardVD)
        {
            _MHal_VD_W1BM( BK_COMB_48, 0x20, 0x30 );
        }
        else // SIG_NTSC_443, SIG_PAL, SIG_PAL_M, SIG_PAL_NC, SIG_SECAM
        {
            _MHal_VD_W1BM( BK_COMB_48, 0x30, 0x30 );
        }

        switch (etStandardVD)
        {
        case E_VIDEOSTANDARD_PAL_M:
        case E_VIDEOSTANDARD_PAL_N:
            _MHal_VD_W1BM( BK_COMB_6C, 0x04, 0x0C );
            break;

        case E_VIDEOSTANDARD_NTSC_44:
            _MHal_VD_W1BM( BK_COMB_6C, 0x0C, 0x0C );
            break;

        case E_VIDEOSTANDARD_NTSC_M:
        case E_VIDEOSTANDARD_SECAM:
            _MHal_VD_W1BM( BK_COMB_6C, 0x00, 0x0C );
            break;

        case E_VIDEOSTANDARD_PAL_BGHI:
        default:
            _MHal_VD_W1BM( BK_COMB_6C, 0x08, 0x0C );
            break;
        }
    }
    else
	{
	    #if(PQ_IN_VD_ENABLE)
    	_MHal_VD_W1BM(BK_COMB_D0, 0x00, 0x01); // turn off (PQ)
    	#endif
	    _MHal_VD_W1BM(BK_COMB_48, 0x20, 0x30);
	    _MHal_VD_W1BM(BK_COMB_6C, 0x00, 0x0C);
	}

    if (etStandardVD == E_VIDEOSTANDARD_NTSC_44)
    {
        #if(PQ_IN_VD_ENABLE)
        _MHal_VD_W1BM(BK_COMB_D0, 0x00, 0xB0);
        #endif
        _MHal_VD_W1BM(BK_COMB_50, 0x06, 0x07);
        _MHal_VD_W1B(BK_COMB_52, 0x67);
        _MHal_VD_W1B(BK_COMB_53, 0x04);
    }
    else
    {
        _MHal_VD_W1BM(BK_COMB_50, 0x07, 0x07);
        _MHal_VD_W1B(BK_COMB_52, 0x8D);
        _MHal_VD_W1B(BK_COMB_53, 0x03);
    }

    if (etStandardVD == E_VIDEOSTANDARD_SECAM)
    {
        _MHal_VD_W1B(BK_VBI_99, 0x6D);
        _MHal_VD_W1B(BK_VBI_9A, 0x9A);
		// shjang_091020
		_MHal_VD_W1Rb(BK_COMB_10, ENABLE, _BIT3);
    }
    else
    {
        _MHal_VD_W1B(BK_VBI_99, 0x8C);
        _MHal_VD_W1B(BK_VBI_9A, 0x01);
		// shjang_091020
		_MHal_VD_W1Rb(BK_COMB_10, DISABLE, _BIT3);
    }

    _u8BackupNoiseMagVD = 0x20;

    if(etStandardVD == E_VIDEOSTANDARD_PAL_BGHI || etStandardVD == E_VIDEOSTANDARD_PAL_M || etStandardVD == E_VIDEOSTANDARD_PAL_N)
    {
        #if(PQ_IN_VD_ENABLE)
        _MHal_VD_W1B(BK_COMB_90, 0x33);
        _MHal_VD_W1B(BK_COMB_91, 0x12);
        _MHal_VD_W1B(BK_COMB_92, 0xD1);
        _MHal_VD_W1B(BK_COMB_93, 0x00);
        _MHal_VD_W1B(BK_COMB_94, 0x40);
        _MHal_VD_W1B(BK_COMB_95, 0x55);
        _MHal_VD_W1B(BK_COMB_96, 0x00);
        _MHal_VD_W1B(BK_COMB_97, 0x00);
        _MHal_VD_W1B(BK_COMB_98, 0x2A);
        _MHal_VD_W1B(BK_COMB_99, 0x00);
        _MHal_VD_W1B(BK_COMB_9A, 0x03);
        _MHal_VD_W1B(BK_COMB_9B, 0x10);
        _MHal_VD_W1B(BK_COMB_9C, 0x83);
        _MHal_VD_W1B(BK_COMB_9D, 0x00);
        _MHal_VD_W1B(BK_COMB_9E, 0x01);
        //_MHal_VD_W1B(BK_COMB_9F, 0xC6);

        if(IsATVInUse(etSourceVD)|| IsAVInUse(etSourceVD)|| IsScartInUse(etSourceVD))
        {
            _MHal_VD_W1B(BK_COMB_9B, 0x10);
        }
        else
        {
            _MHal_VD_W1B(BK_COMB_9B, 0x00);
        }
        #endif
    }
    else
    {
        #if(PQ_IN_VD_ENABLE)
        _MHal_VD_W1B(BK_COMB_90, 0x31);
        _MHal_VD_W1B(BK_COMB_91, 0x32);
        _MHal_VD_W1B(BK_COMB_92, 0x00);
        _MHal_VD_W1B(BK_COMB_93, 0x40);
        _MHal_VD_W1B(BK_COMB_94, 0x71);
        _MHal_VD_W1B(BK_COMB_95, 0xCC);
        _MHal_VD_W1B(BK_COMB_96, 0x00);
        _MHal_VD_W1B(BK_COMB_97, 0x80);
        _MHal_VD_W1B(BK_COMB_98, 0x28);
        _MHal_VD_W1B(BK_COMB_99, 0x80);
        _MHal_VD_W1B(BK_COMB_9A, 0x44);
        _MHal_VD_W1B(BK_COMB_9B, 0x00);
        _MHal_VD_W1B(BK_COMB_9C, 0x01);
        _MHal_VD_W1B(BK_COMB_9D, 0x08);
        #endif
    }
	/* enable 3D comb speed up while driver found a standard */
	#ifdef LOCK3DSPEEDUP
	//if IsUseDigitalPort(penMsSysInfo->enInputPortType)
	{

            _bComb10Bit3Flag = 1;  //20090918EL
#if 0
            {  //20090827EL
                if(_MHal_VD_R1B( BK_AFEC_CC ) & _BIT1)  //Burst on
                {
                    MHal_VD_3DCombSpeedup(TRUE);
                    _bUnstableChecking = TRUE;
                    _u32VideoSystemTimer = jiffies;
                    //printk(" ^^^ 20090827 ^^^222  MHal_VD_3DCombSpeedup(TRUE);\n");
                }
            }
#endif
           //_bStartCheckCombSpeedup = 1;  //Remove it to fix color bar shaking with FSC changes and video standard changes 20090918EL

	}
	#endif
	#if (FINE_TUNE_COMB_F2)
	//bug's life, for Eris ATT, James.Lu, 20080327
	if ((IsAVInUse(etSourceVD))&&(etStandardVD == E_VIDEOSTANDARD_NTSC_M))
		_MHal_VD_W1B( BK_COMB_F2, 0x57 );
	else
		_MHal_VD_W1B( BK_COMB_F2, 0x47 );
	#endif
	#if (FINE_TUNE_3D_COMB)
	if (etStandardVD==E_VIDEOSTANDARD_PAL_BGHI)
	{
		_MHal_VD_W1BM(BK_COMB_23, 0x02, _BIT3 | _BIT1);
		_MHal_VD_W1B(BK_COMB_9F, 0xc6);
		_MHal_VD_W1B(BK_COMB_ED, 0x82);
	}
	else
	{
		_MHal_VD_W1BM(BK_COMB_23, 0x08, _BIT3 | _BIT1);
		_MHal_VD_W1B(BK_COMB_9F, 0xc6);
		_MHal_VD_W1B(BK_COMB_ED, 0x80);
	}
	#endif

    /*
        Improve 2D/3D comb stability on noisy signal
        Solution/Reason: Fine-tune the 2D/3D detection threshold
    */
    switch (etStandardVD)
    {
    case E_VIDEOSTANDARD_PAL_BGHI:
        _MHal_VD_W1B(BK_COMB_55, 0x05);
        break;
    case E_VIDEOSTANDARD_PAL_M:
        _MHal_VD_W1B(BK_COMB_55, 0x04);
        break;
    default:
        _MHal_VD_W1B(BK_COMB_55, 0x05);
        break;
    }

    _MHal_VD_W1BM(BK_COMB_EE, 0, BIT7); //2009071:0 from BY's suggest
}


///-----------------------------------------------------------------------------
/// To start automatic detection about input video standard.
/// @param  void
/// @return void
/// @note
/// @type   thread safe.
///-----------------------------------------------------------------------------
void MHal_VD_StartAutoStandardDetection(void)
{
    _MHal_VD_SetAutoDetetion(FSC_AUTO_DET_ENABLE);
}


///-----------------------------------------------------------------------------
/// To stop automatic detection about input video standard.
/// @param  void
/// @return void
/// @note
/// @type   thread safe.
///-----------------------------------------------------------------------------
void MHal_VD_StopAutoStandardDetection(void)
{
    _MHal_VD_SetAutoDetetion(FSC_AUTO_DET_DISABLE);
}



///-----------------------------------------------------------------------------
/// To get result of video standard after automatic detection called by MHal_VD_StartAutoStandardDetection.
/// @param  void
/// @return VIDEOSTANDARD_TYPE:
/// - Video standard
/// @note
///-----------------------------------------------------------------------------
VIDEOSTANDARD_TYPE MHal_VD_GetResultOfAutoStandardDetection(void)
{
    VIDEOSTANDARD_TYPE eVideoStandard;

    eVideoStandard = MHal_VD_GetStandardDetection();

	MHal_VD_StopAutoStandardDetection();

    return eVideoStandard;
}

///-----------------------------------------------------------------------------
/// To get result of video standard last time.
/// @param  void
/// @return VIDEOSTANDARD_TYPE:
/// - Video standard
/// @note
///-----------------------------------------------------------------------------
VIDEOSTANDARD_TYPE MHal_VD_GetLastDetectedStandard(void)
{
	return _etVideoLastStandardVD;
}

///-----------------------------------------------------------------------------
/// Get Result Auto Video Standard Detection
/// @param  void
/// @return VIDEOSTANDARD_TYPE:
/// - Video standard
/// @note   <private function>
///-----------------------------------------------------------------------------
VIDEOSTANDARD_TYPE MHal_VD_GetStandardDetection(void)
{
    U16                 u16VDStatusVD;
    VIDEOSTANDARD_TYPE  etVideoStandardVD;

    //VD_KDBG1("MHal_VD_GetStandardDetection()\n"); // <-<<<

    u16VDStatusVD = MHal_VD_GetStatus();

    //VD_KDBG2("Current Status 0x%04x\n", u16VDStatusVD); // <-<<<

    /*
        If system doesn't use debounce to detect video format.
        Driver has add V sync lock and status ready to optimal detecion of video format.
        This solution is follow S3P and S4.
    */
    // 1. if ( !IS_BITS_SET(u16VDStatusVD, VD_HSYNC_LOCKED) ) // if ( !IS_BITS_SET(wStatus, VD_STATUS_RDY) )
    //==> Some Problem!! : PDP Banner flicker while changing DTV to ATV.
    // 2. Merge!!! =>  junyou lin : If you don’t update the below line, I think your video formation detection may get fail result sometime
	// if ( !IS_BITS_SET(u16VDStatusVD, VD_HSYNC_LOCKED|VD_SYNC_LOCKED|VD_STATUS_RDY) )
	//==> But we can see the transition problem(noise) while changing DTV to ATV
    // 3. PDP DTV => ATV 전환시에 Banner 깜박임 문제 해결하고 절환 과도도 동시에 해결함.
    //==> OK!! PDP flicker & Transition noise.
    if ( !IS_BITS_SET(u16VDStatusVD, VD_HSYNC_LOCKED|VD_SYNC_LOCKED))
    {
        _etVideoLastStandardVD=E_VIDEOSTANDARD_NOTSTANDARD;
        return E_VIDEOSTANDARD_NOTSTANDARD;
    }

    switch (u16VDStatusVD & VD_FSC_TYPE)
    {
    case VD_FSC_4285: // (FSC_MODE_SECAM << 5):
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x37), 0x06, 0x07);
        // if (wStatus & VD_VSYNC_50HZ) // SECAM must 50 Hz
        {
            etVideoStandardVD = E_VIDEOSTANDARD_SECAM;
            //VD_KDBG("[SECAM]\n");
        }
        break;

    case VD_FSC_4433: // (FSC_MODE_PAL << 5):
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x37), 0x06, 0x07);
        #if (!TEST_VD_DSP)
        // 60 Hz
        if ( (u16VDStatusVD & VD_VSYNC_50HZ) == 0x0)
        {
            if (u16VDStatusVD & VD_PAL_SWITCH)
            {
                etVideoStandardVD = E_VIDEOSTANDARD_PAL_60; // or vsdNTSC_44
                //VD_KDBG("[PAL60]\n");
            }
            else
            {
                etVideoStandardVD = E_VIDEOSTANDARD_NTSC_44;
                //VD_KDBG("[NTSC-443]\n");
            }
        }
        else
        #endif
        // 50 Hz
        {
        	etVideoStandardVD = E_VIDEOSTANDARD_PAL_BGHI;
        	//VD_KDBG("[PAL]\n");
		}
        break;

    case VD_FSC_3579: // (FSC_MODE_NTSC << 5):
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x37), 0x07, 0x07);
        etVideoStandardVD = E_VIDEOSTANDARD_NTSC_M;
        //VD_KDBG("[NTSC]\n");
        break;

    case VD_FSC_3575: // (FSC_MODE_PAL_M << 5):
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x37), 0x07, 0x07);
        etVideoStandardVD = E_VIDEOSTANDARD_PAL_M;
        //VD_KDBG("[PAL-M]\n");
        break;

    case VD_FSC_3582: // (FSC_MODE_PAL_N << 5):
        _MHal_VD_W1BM(L_BK_ADC_ATOP(0x37), 0x07, 0x07);
        etVideoStandardVD = E_VIDEOSTANDARD_PAL_N;
        //VD_KDBG("[PAL-NC]\n");
        break;

    #if (TEST_VD_DSP)
    case (FSC_MODE_NTSC_443 << 5):
        etVideoStandardVD = E_VIDEOSTANDARD_NTSC_44;
        //debugVDIPrint("[NTSC-443]", NULL);
        break;

    case (FSC_MODE_PAL_60 << 5):
        etVideoStandardVD = E_VIDEOSTANDARD_PAL_60;
        //debugVDIPrint("[PAL60]", NULL);
        break;
    #endif

    default:
        etVideoStandardVD = E_VIDEOSTANDARD_NOTSTANDARD;
        //VD_KDBG2("Not valid FSC_TYPE:0x%x",u16VDStatusVD);
        break;
    }

    _etVideoLastStandardVD=etVideoStandardVD;

    return etVideoStandardVD;
}



///-----------------------------------------------------------------------------
/// To get video standard of input signal within 200ms.
/// @param  void
/// @return VIDEOSTANDARD_TYPE:
/// - Video standard
/// @note
///-----------------------------------------------------------------------------
VIDEOSTANDARD_TYPE MHal_VD_DetectStandardAutomatically(void)
{
    MHal_VD_StartAutoStandardDetection();

    // TVAVDelay(400); // <-<<< OPTIMIZE

    return MHal_VD_GetResultOfAutoStandardDetection();
}

///-----------------------------------------------------------------------------
/// To enable auto gain control.
/// @param  B16:
/// - TRUE  : enable auto gain control.
/// - FALSE : disable auto gain control.
/// @return void
/// @note
///     <dummy function>
///-----------------------------------------------------------------------------
void MHal_VD_EnableAutoGainControl(B16 bEnableVD)
{
    bEnableVD = bEnableVD;
#if 0

#if (VD_GAIN_TYPE_SEL == VD_USE_FIX_GAIN)
    bEnable = bEnable;
    _MHal_VD_W1BM(BK_AFEC_43, AGC_DISABLE << 5, BITMASK(6:5));
#elif (VD_GAIN_TYPE_SEL == VD_USE_AUTO_GAIN)
    bEnable = bEnable;
    _MHal_VD_W1BM(BK_AFEC_43, AFE_AGC_ENABLE << 5, BITMASK(6:5));
#else
    if (bEnable) // Auto Gain Control
    {
        _MHal_VD_W1BM(BK_AFEC_43, AFE_AGC_ENABLE << 5, BITMASK(6:5));
    }
    else // Fixed Gain
    {
        _MHal_VD_W1BM(BK_AFEC_43, AFE_AGC_DISABLE << 5, BITMASK(6:5));
    }
#endif

#endif
}



/**
 FUNCTION   : void MDrv_VD_ImprovePictureQuality(VIDEOSOURCE_TYPE eSource, VIDEOSTANDARD_TYPE eStandard, TELETEXT_DISPLAY_MODE eDisplayMode)

 USAGE  : To improve picture quality. This function can be used by picture quality engineer.

 INPUT  :   eSource - Category of input source.
            eStandard - Video standard for input signal.
            eDispalyMode -  TTX_DISPLAY_INVALID_MODE : Invalid mode.
                            TTX_DISPLAY_UPDATE_MODE  : Update mode of teletext.
                            TTX_DISPLAY_PICTURE_MODE : Picture mode.
                            TTX_DISPLAY_MIX_MODE     : Mix mode of teletext.
                            TTX_DISPLAY_TEXT_MODE    : Text mode of teletext.
                            TTX_DISPLAY_LOCK_MODE    : Clock mode of teletext.
                            TTX_DISPLAY_SUBTITLE_MODE: Subtitle mode of teletext.

 OUTPUT : None

 GLOBALS: None

*/

/* LJU */
#if 0

void MDrv_VD_ImprovePictureQuality(VIDEOSOURCE_TYPE eSource, VIDEOSTANDARD_TYPE eStandard, TELETEXT_DISPLAY_MODE eDisplayMode)
{
    VIDEOSOURCE_AND_STANDARD eSourceStandard;

    eSourceStandard = MAKEWORD(eSource, eStandard);

    switch(eDisplayMode)
    {
    case TTX_DISPLAY_INVALID_MODE:
        // do nothing. To keep previous picture setting.
        break;

//=================== Screen is dedicated for picture ===================//
    case TTX_DISPLAY_PICTURE_MODE:
    case TTX_DISPLAY_UPDATE_MODE:
    case TTX_DISPLAY_LOCK_MODE:
        switch(eSourceStandard)
        {
//----  ATV Input  ----//
        case SRC_STANDARD_ATV_NTSC_M:
        case SRC_STANDARD_ATV_PAL_BGHI:
        case SRC_STANDARD_ATV_PAL_N:
        case SRC_STANDARD_ATV_NTSC_44:
        case SRC_STANDARD_ATV_PAL_M:
        case SRC_STANDARD_ATV_PAL_60:
        case SRC_STANDARD_ATV_SECAM:
            break;

//----  CVBS or SVIDEO Input  ----//
        case SRC_STANDARD_CVBS1_PAL_BGHI:
        case SRC_STANDARD_CVBS1_NTSC_M:
        case SRC_STANDARD_CVBS1_NTSC_44:
        case SRC_STANDARD_CVBS1_PAL_M:
        case SRC_STANDARD_CVBS1_PAL_N:
        case SRC_STANDARD_CVBS1_PAL_60:
        case SRC_STANDARD_CVBS1_SECAM:
            break;

        case SRC_STANDARD_SVIDEO1_PAL_BGHI:
        case SRC_STANDARD_SVIDEO1_NTSC_M:
        case SRC_STANDARD_SVIDEO1_NTSC_44:
        case SRC_STANDARD_SVIDEO1_PAL_M:
        case SRC_STANDARD_SVIDEO1_PAL_N:
        case SRC_STANDARD_SVIDEO1_PAL_60:
        case SRC_STANDARD_SVIDEO1_SECAM:
            break;

//----  SCART1 Input  ----//
        case SRC_STANDARD_SCART1_PAL_BGHI:
        case SRC_STANDARD_SCART1_NTSC_M:
        case SRC_STANDARD_SCART1_NTSC_44:
        case SRC_STANDARD_SCART1_PAL_M:
        case SRC_STANDARD_SCART1_PAL_N:
        case SRC_STANDARD_SCART1_PAL_60:
        case SRC_STANDARD_SCART1_SECAM:
            break;

//----  SCART2 Input  ----//
        case SRC_STANDARD_SCART2_PAL_BGHI:
        case SRC_STANDARD_SCART2_NTSC_M:
        case SRC_STANDARD_SCART2_NTSC_44:
        case SRC_STANDARD_SCART2_PAL_M:
        case SRC_STANDARD_SCART2_PAL_N:
        case SRC_STANDARD_SCART2_PAL_60:
        case SRC_STANDARD_SCART2_SECAM:
            break;

//----  3D-COMB FILTER ATV Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_60:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_SECAM:
            break;

//----  3D-COMB FILTER CVBS Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_60:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_SECAM:
            break;

//----  3D-COMB FILTER SCART1 Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_60:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_SECAM:
            break;

//----  3D-COMB FILTER SCART2 Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_60:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_SECAM:
            break;

        default:
            debugVDIPrint("Error!!! Who are you?",0);
            break;
        }
        break;

//=================== Teletext and pictue is mixed ===================//
    case TTX_DISPLAY_MIX_MODE:
    case TTX_DISPLAY_SUBTITLE_MODE:
        switch(eSourceStandard)
        {
//----  ATV Input  ----//
        case SRC_STANDARD_ATV_NTSC_M:
        case SRC_STANDARD_ATV_PAL_BGHI:
        case SRC_STANDARD_ATV_PAL_N:
        case SRC_STANDARD_ATV_NTSC_44:
        case SRC_STANDARD_ATV_PAL_M:
        case SRC_STANDARD_ATV_PAL_60:
        case SRC_STANDARD_ATV_SECAM:
            break;

//----  CVBS or SVIDEO Input  ----//
        case SRC_STANDARD_CVBS1_PAL_BGHI:
        case SRC_STANDARD_CVBS1_NTSC_M:
        case SRC_STANDARD_CVBS1_SECAM:
        case SRC_STANDARD_CVBS1_NTSC_44:
        case SRC_STANDARD_CVBS1_PAL_M:
        case SRC_STANDARD_CVBS1_PAL_N:
        case SRC_STANDARD_CVBS1_PAL_60:
            break;

        case SRC_STANDARD_SVIDEO1_PAL_BGHI:
        case SRC_STANDARD_SVIDEO1_NTSC_M:
        case SRC_STANDARD_SVIDEO1_SECAM:
        case SRC_STANDARD_SVIDEO1_NTSC_44:
        case SRC_STANDARD_SVIDEO1_PAL_M:
        case SRC_STANDARD_SVIDEO1_PAL_N:
        case SRC_STANDARD_SVIDEO1_PAL_60:
            break;

//----  SCART1 Input  ----//
        case SRC_STANDARD_SCART1_PAL_BGHI:
        case SRC_STANDARD_SCART1_NTSC_M:
        case SRC_STANDARD_SCART1_SECAM:
        case SRC_STANDARD_SCART1_NTSC_44:
        case SRC_STANDARD_SCART1_PAL_M:
        case SRC_STANDARD_SCART1_PAL_N:
        case SRC_STANDARD_SCART1_PAL_60:
            break;

//----  SCART2 Input  ----//
        case SRC_STANDARD_SCART2_PAL_BGHI:
        case SRC_STANDARD_SCART2_NTSC_M:
        case SRC_STANDARD_SCART2_SECAM:
        case SRC_STANDARD_SCART2_NTSC_44:
        case SRC_STANDARD_SCART2_PAL_M:
        case SRC_STANDARD_SCART2_PAL_N:
        case SRC_STANDARD_SCART2_PAL_60:
            break;

//----  3D-COMB FILTER ATV Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_60:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_SECAM:
            break;

//----  3D-COMB FILTER CVBS Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_SECAM:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_60:
            break;

//----  3D-COMB FILTER SCART1 Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_SECAM:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_60:
            break;

//----  3D-COMB FILTER SCART2 Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_SECAM:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_60:
            break;

        default:
            debugVDIPrint("Error!!! Who are you?",0);
            break;
        }
        break;

//=================== Screen is dedicated for teletext ===================//
    case TTX_DISPLAY_TEXT_MODE:
        switch(eSourceStandard)
        {
//----  ATV Input  ----//
        case SRC_STANDARD_ATV_NTSC_M:
        case SRC_STANDARD_ATV_PAL_BGHI:
        case SRC_STANDARD_ATV_PAL_N:
        case SRC_STANDARD_ATV_NTSC_44:
        case SRC_STANDARD_ATV_PAL_M:
        case SRC_STANDARD_ATV_PAL_60:
        case SRC_STANDARD_ATV_SECAM:
            break;

//----  CVBS or SVIDEO Input  ----//
        case SRC_STANDARD_CVBS1_PAL_BGHI:
        case SRC_STANDARD_CVBS1_NTSC_M:
        case SRC_STANDARD_CVBS1_SECAM:
        case SRC_STANDARD_CVBS1_NTSC_44:
        case SRC_STANDARD_CVBS1_PAL_M:
        case SRC_STANDARD_CVBS1_PAL_N:
        case SRC_STANDARD_CVBS1_PAL_60:
            break;

        case SRC_STANDARD_SVIDEO1_PAL_BGHI:
        case SRC_STANDARD_SVIDEO1_NTSC_M:
        case SRC_STANDARD_SVIDEO1_SECAM:
        case SRC_STANDARD_SVIDEO1_NTSC_44:
        case SRC_STANDARD_SVIDEO1_PAL_M:
        case SRC_STANDARD_SVIDEO1_PAL_N:
        case SRC_STANDARD_SVIDEO1_PAL_60:
            break;

//----  SCART1 Input  ----//
        case SRC_STANDARD_SCART1_PAL_BGHI:
        case SRC_STANDARD_SCART1_NTSC_M:
        case SRC_STANDARD_SCART1_SECAM:
        case SRC_STANDARD_SCART1_NTSC_44:
        case SRC_STANDARD_SCART1_PAL_M:
        case SRC_STANDARD_SCART1_PAL_N:
        case SRC_STANDARD_SCART1_PAL_60:
            break;

//----  SCART2 Input  ----//
        case SRC_STANDARD_SCART2_PAL_BGHI:
        case SRC_STANDARD_SCART2_NTSC_M:
        case SRC_STANDARD_SCART2_SECAM:
        case SRC_STANDARD_SCART2_NTSC_44:
        case SRC_STANDARD_SCART2_PAL_M:
        case SRC_STANDARD_SCART2_PAL_N:
        case SRC_STANDARD_SCART2_PAL_60:
            break;

//----  3D-COMB FILTER ATV Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_PAL_60:
        case SRC_STANDARD_THROUGH_3DCOMB_ATV_SECAM:
            break;

//----  3D-COMB FILTER CVBS Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_SECAM:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_CVBS1_PAL_60:
            break;

//----  3D-COMB FILTER SCART1 Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_SECAM:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART1_PAL_60:
            break;

//----  3D-COMB FILTER SCART2 Input  ----//
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_BGHI:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_NTSC_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_SECAM:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_NTSC_44:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_M:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_N:
        case SRC_STANDARD_THROUGH_3DCOMB_SCART2_PAL_60:
            break;

        default:
            debugVDIPrint("Error!!! Who are you?",0);
            break;
        }
        break;
    }
}

#endif


///-----------------------------------------------------------------------------
/// To set freerun PLL
/// @param  VIDEOFREQ:
/// -   video frequency
/// @return void
/// @note
///-----------------------------------------------------------------------------
void MHal_VD_SetFreerunPLL(VIDEOFREQ etVideoFreqVD)
{
    if( etVideoFreqVD == E_VIDEO_FQ_60Hz )
    {
        _MHal_VD_W1B( BK_COMB_52, 0x8E );
        _MHal_VD_W1B( BK_COMB_53, 0x03 );
    }
    else if( etVideoFreqVD == E_VIDEO_FQ_50Hz )
    {
        _MHal_VD_W1B( BK_COMB_52, 0x6F );
        _MHal_VD_W1B( BK_COMB_53, 0x04 );
    }



    // _MHal_VD_W1BM(BK_COMB_50, 0x06, 0x07); // <-<<< DISABLE FIRST
}

///-----------------------------------------------------------------------------
/// To set freerun frequency
/// @param  FREERUNFREQ:
/// -   video freerun frequency
/// @return void
/// @note
///-----------------------------------------------------------------------------
void MHal_VD_SetFreerunFreq(FREERUNFREQ etFreerunfreqVD)
{
    switch(etFreerunfreqVD)
    {
    case E_FREERUN_FQ_AUTO:
        _MHal_VD_W1BM( BK_AFEC_CF, 0x00 << 0, BITMASK(1:0) );
        break;

    case E_FREERUN_FQ_50Hz:
        _MHal_VD_W1BM( BK_AFEC_CF, 0x01 << 0, BITMASK(1:0) );
        break;

    case E_FREERUN_FQ_60Hz:
        _MHal_VD_W1BM( BK_AFEC_CF, 0x03 << 0, BITMASK(1:0) );
        break;

    default:
        break;
    }


}


#if 1

extern int g_system_3dcom_state; // Samuel, 20081110

/* this code run in interrupt */
//void MDrv_VD_SetRegFromDSP(INPUT_SOURCE_TYPE etISTypeVD )
void MDrv_VD_SetRegFromDSP(VIDEOSOURCE_TYPE etSourceVD )
{
    U8  u8Ctl, u8Mask, u8Adr, u8Value;
    U16 u16Htotal, u16CurrentHStart;  // // <-<<< UNUSED
    U32 u32Temp;
    U8  u8AbnormalSignal; // <-<<< UNUSED
    U8  u8update; // <-<<< UNUSED
	U8	u8VDSync;

	/* This patch sloves power on/off issue of ATSC.
	    LGE skystone 20090109
	*/
	if(MHal_VD_GetDSPVer() == VD_SYSTEM_NTSC) /* ATSC */// Set Analog Color System - 090309.
	{
	   u8VDSync = _MHal_VD_R1B( BK_AFEC_CD );

	   if( (u8VDSync & (BIT6|BIT7)) == 0x0)
	   {
	 		/* If driver lost h and v sync, it sets DSP firmware vertical line to 525 */
		   _MHal_VD_W1BM (BK_AFEC_CF, (BIT0|BIT1), (BIT0|BIT1));
	   }

	   if( (u8VDSync & BIT6) == BIT6)
	   {
			/* If driver find h sync, it release DSP firmware to find vertical line. */
		   _MHal_VD_W1BM (BK_AFEC_CF, 0, (BIT0|BIT1));
	   }
	}

    /* debug register 0x35DC */
    u8Ctl = _MHal_VD_R1B( BK_AFEC_DC );

    if( ( u8Ctl & MSK_UD7_STATE ) == VAL_UD7_WRITE ) // write enable
    {
        u8Mask  = _MHal_VD_R1B( BK_AFEC_DD );
        u8Adr   = _MHal_VD_R1B( BK_AFEC_DE );
        u8Value = _MHal_VD_R1B( BK_AFEC_DF );

        if( ( u8Ctl & MSK_UD7_BANK ) == VAL_UD7_BANK6 ) // bank Comb
        {
            u32Temp = COMB_REG_BASE; // set base address to COMB_REG_BASE
        }
        else
        {
            u32Temp = AFEC_REG_BASE; // set base address to AFEC_REG_BASE
        }
        _MHal_VD_W1BM( BYTE2REAL(u32Temp + u8Adr), u8Value, u8Mask );
        _MHal_VD_W1BM( BK_AFEC_DC, VAL_UD7_FREE, MSK_UD7_STATE);
    }
    else if( ( u8Ctl & MSK_UD7_STATE ) == VAL_UD7_READ )
    {
        u8Mask = _MHal_VD_R1B( BK_AFEC_DD );
        u8Adr  = _MHal_VD_R1B( BK_AFEC_DE );
        if( ( u8Ctl & MSK_UD7_BANK) == VAL_UD7_BANK6 ) // bank Comb
        {
            u32Temp = COMB_REG_BASE; // set base address to COMB_REG_BASE
        }
        else
        {
            u32Temp = AFEC_REG_BASE; // set base address to AFEC_REG_BASE
        }

        u8Value = _MHal_VD_R1B(BYTE2REAL(u32Temp + u8Adr)) & u8Mask;

        _MHal_VD_W1BM( BK_AFEC_DF, u8Value, 0xFF );
        _MHal_VD_W1BM( BK_AFEC_DC, VAL_UD7_READ_END, MSK_UD7_STATE );
    }

    if( 1 )
    {
#if 1 // <- UNUSED
        //u16Htotal = _MHal_VD_R2B(BK_AFEC_C7); // SPL_NSPL, H total
        u16Htotal = _MHal_VD_R1B(BK_AFEC_C7) | (_MHal_VD_R1B(BK_AFEC_C8)<<8) ; // SPL_NSPL, H total

        switch( atomic_read(&_aVideoSystemVD) ) // 2006.06.17 Michael, need to check SRC1 if we use MST6xxx
        {
        case E_VIDEOSTANDARD_NTSC_M:     // SIG_NTSC: 910
             u16CurrentHStart = 910;
             break;

        case E_VIDEOSTANDARD_PAL_BGHI:   // SIG_PAL: 1135
             u16CurrentHStart = 1135;
             break;

        case E_VIDEOSTANDARD_NTSC_44:    // SIG_NTSC_443: 1127
        case E_VIDEOSTANDARD_PAL_60:
             u16CurrentHStart = 1127;
             break;

        case E_VIDEOSTANDARD_PAL_M:      // SIG_PAL_M: 909
             u16CurrentHStart = 909;
             break;

        case E_VIDEOSTANDARD_PAL_N:      // SIG_PAL_NC: 917
             u16CurrentHStart = 917;
             break;

        case E_VIDEOSTANDARD_SECAM:      // SIG_SECAM: 1097
             u16CurrentHStart = 1097;
             break;

        default:
             // ASSERT
             u16CurrentHStart = 1135;
             break;
        }

        /* H start is instable and we should wait it stable*/
        if( u16Htotal != u16CurrentHStart )
        {
            if( _u8AbnormalCounterVD < 5)	// 20080718 swwoo LGE 15 )
            {
                _u8AbnormalCounterVD++;
            }
        }
        else
        {
            if( _u8AbnormalCounterVD > 1 )
            {
                _u8AbnormalCounterVD -= 2;
            }
            else
            {
                _u8AbnormalCounterVD = 0;
            }
        }
        /* signal is abnormal if it is never stable */
        if( _u8AbnormalCounterVD > 4)	// 20080718 swwoo LGE 10 )
        {
            u8AbnormalSignal = 1;
        }
        else if( _u8AbnormalCounterVD == 0 )
        {
            u8AbnormalSignal = 0;
        }
        // For Bug 267 <--

        _u16DataHVD[2] = _u16DataHVD[1];
        _u16DataHVD[1] = _u16DataHVD[0];
        _u16DataHVD[0] = u16Htotal;

        if( (_u16DataHVD[2] == _u16DataHVD[1]) && (_u16DataHVD[1] == _u16DataHVD[0]) )
        {
            if( _u8HtotalDebounceVD > 3 )
            {
                _u16LatchHVD = _u16DataHVD[0];
            }
            else
            {
                _u8HtotalDebounceVD++;
            }
        }
        else
        {
            /* trigger debounce */
            _u8HtotalDebounceVD = 0;
        }

        u16Htotal = _u16LatchHVD;

        switch( atomic_read(&_aVideoSystemVD) )
        {
        case E_VIDEOSTANDARD_PAL_BGHI:  // SIG_PAL
        case E_VIDEOSTANDARD_NTSC_44:   // SIG_NTSC_443
                u8Value = 3;
                break;

        case E_VIDEOSTANDARD_PAL_M:     // SIG_PAL_M
        case E_VIDEOSTANDARD_PAL_N:     // SIG_PAL_NC
                u8Value = 1;
                break;

        default:                        // NTSC
                u8Value = 2;
        }

        u8Ctl = ( u16Htotal - u8Value ) % 4;
        u8update =  1;
        if( u8Ctl == 3 )
        {
            u16Htotal = u16Htotal + 1;
        }
        else if( u8Ctl == 2 )
        {
            u8update = 0;
        }
        else
        {
            u16Htotal = u16Htotal - u8Ctl;
        }

        if( u8update )
        {
            // MDrv_Write2Byte( BK_COMB_52, u16Htotal );
            #if 0
            //#ifdef ENABLE_NTSC_50
            if ( ((MDrv_VD_GetStatus() & (VD_FSC_TYPE|VD_VSYNC_50HZ|VD_COLOR_LOCKED)) == (VD_FSC_3579|VD_VSYNC_50HZ|VD_COLOR_LOCKED)) ) // not NTSC-50
            {
                if (u16Htotal > 1300)
                    MDrv_WriteByteMask( BK_COMB_50, 0x07, 0x07 );
                else if (u16Htotal > 1135)
                {
                    MDrv_Write2Byte( BK_COMB_52, 0x396 );
                    MDrv_WriteByteMask( BK_COMB_50, 0x06, 0x07 );
                }
                else
                {
                    MDrv_Write2Byte( BK_COMB_52, u16Htotal );
                    MDrv_WriteByteMask( BK_COMB_50, 0x06, 0x07 );
                }
            }
            else
            {
                MDrv_Write2Byte( BK_COMB_52, u16CurrentHStart );
            }
            #else
            _MHal_VD_W2B( BK_COMB_52, u16CurrentHStart );
            #endif

        }
#endif

    // Fix debounce problem to prevent setting wrong for fine tune FH_DOT.
    #if (FINE_TUNE_FH_DOT)
        {
            #define FINE_TUNE_FH_DOT_MAX 10
            static U8 u8FhDotDebouncer=0;
            //MDrv_WriteByte(BK_AFEC_04, 0x04);
            _MHal_VD_W1B(BK_AFEC_04, 0x04);
            //u8Ctl = MDrv_ReadByte(BK_AFEC_02); // get VD noise magnitude
            u8Ctl = _MHal_VD_R1B(BK_AFEC_02); // get VD noise magnitude

            if( (IsAVInUse(etSourceVD) || IsScartInUse(etSourceVD))&&
                (   (E_VIDEOSTANDARD_NTSC_M==atomic_read(&_aVideoSystemVD)) ||
                    (E_VIDEOSTANDARD_PAL_BGHI==atomic_read(&_aVideoSystemVD)) ) )
            {
                if((abs(_u16LatchHVD-u16CurrentHStart)>=2)&&(u8Ctl<=0x01))
                {
                    if (u8FhDotDebouncer<FINE_TUNE_FH_DOT_MAX)
                        u8FhDotDebouncer++;
                }
                else if (u8FhDotDebouncer)
                    u8FhDotDebouncer--;
            }
            else
                u8FhDotDebouncer = 0;

            if (u8FhDotDebouncer>=FINE_TUNE_FH_DOT_MAX)
            {
                #ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
                u32VDPatchFlagStatus |= FINE_TUNE_FH_DOT;
                MHal_VD_COMB_Set3dCombMid(DISABLE);
                #endif

                //MDrv_WriteRegBit(BK_COMB_18, DISABLE, _BIT0);
                _MHal_VD_W1Rb(BK_COMB_18, DISABLE, _BIT0);
                //MDrv_WriteRegBit(BK_COMB_C0, DISABLE, _BIT5);
                _MHal_VD_W1Rb(BK_COMB_C0, DISABLE, _BIT5);
            }
            else if (!u8FhDotDebouncer)
            {
                #ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
                u32VDPatchFlagStatus &= ~FINE_TUNE_FH_DOT;
                #endif

                //MDrv_WriteRegBit(BK_COMB_18, ENABLE, _BIT0);
                _MHal_VD_W1Rb(BK_COMB_18, ENABLE, _BIT0);
                //MDrv_WriteRegBit(BK_COMB_C0, ENABLE, _BIT5);
                _MHal_VD_W1Rb(BK_COMB_C0, ENABLE, _BIT5);
            }
            //printf("u16LatchH,u16CurrentHStart,u8FhDotDebouncer= %d %d %bd\n",u16LatchH,u16CurrentHStart,u8FhDotDebouncer);
        }
    #endif


    // Add a feature to fine-tune for Fsc shift cause color bar edge blur, James.Lu, 20080919
    #if (FINE_TUNE_FSC_SHIFT)
        {
			#define FINE_TUNE_FSC_SHIFT_CNT_STEP 3
			#define FINE_TUNE_FSC_SHIFT_CNT_MAX FINE_TUNE_FSC_SHIFT_CNT_STEP*7
			#define FINE_TUNE_FSC_SHIFT_CNT_THR FINE_TUNE_FSC_SHIFT_CNT_STEP*3

			static U8 u8FscShiftDebounceCnt=0;

			// Add a nosie reduce feature in the VD thread.
			static U8   u8PreNoiseMagn = 100;
			U8          u8Comb_C0;

			_MHal_VD_W1B(BK_AFEC_04, 0x04);
			u8Ctl = _MHal_VD_R1B(BK_AFEC_02); // get VD noise magnitude

			if (u8PreNoiseMagn ==100)  //thchen 20081211 for threshold
			{
				u8PreNoiseMagn = u8Ctl;
			}

			u8Ctl = (u8PreNoiseMagn + u8Ctl)/2;  //thchen 20081211 for threshold
			u8PreNoiseMagn = u8Ctl;  //thchen 20081211 for threshold

			if (u8Ctl<= 3)
			{
				u8Comb_C0 = 0xA0;
			}
			else if (  u8Ctl >3 && u8Ctl <= 6 )
			{
				u8Comb_C0 = 0xA0;
			}
			else
				u8Comb_C0 = 0x2C;

			// ------>
			if ((E_VIDEOSTANDARD_NTSC_M==atomic_read(&_aVideoSystemVD)) ||
				(E_VIDEOSTANDARD_PAL_BGHI==atomic_read(&_aVideoSystemVD)) ||
				(E_VIDEOSTANDARD_PAL_M==atomic_read(&_aVideoSystemVD)) ||
				(E_VIDEOSTANDARD_PAL_N==atomic_read(&_aVideoSystemVD)) )
			{
				u8Ctl = _MHal_VD_R1B(BK_COMB_E0);
				_MHal_VD_W1B(BK_COMB_E0,0xff);

				if (!(u8Ctl&_BIT6)) // got comb 3D unlocked
				{
					if (u8FscShiftDebounceCnt<FINE_TUNE_FSC_SHIFT_CNT_MAX)
						u8FscShiftDebounceCnt+=FINE_TUNE_FSC_SHIFT_CNT_STEP;
				}
				else
				{
					if (u8FscShiftDebounceCnt)
						u8FscShiftDebounceCnt--;
				}
			}
			else
				u8FscShiftDebounceCnt = 0;

			if (u8FscShiftDebounceCnt>=FINE_TUNE_FSC_SHIFT_CNT_THR)
			{

				_MHal_VD_W1Rb(BK_COMB_15, DISABLE, _BIT6);
				_MHal_VD_W1Rb(BK_COMB_D0, DISABLE, _BIT0);
				_MHal_VD_W1B(BK_COMB_C0,0x6C);
			}
			else if (!u8FscShiftDebounceCnt)
			{
				if (E_VIDEOSTANDARD_PAL_BGHI==atomic_read(&_aVideoSystemVD))
					_MHal_VD_W1Rb(BK_COMB_15, ENABLE, _BIT6);
				else
				    _MHal_VD_W1Rb(BK_COMB_15, DISABLE, _BIT6);

				_MHal_VD_W1Rb(BK_COMB_D0, ENABLE, _BIT0);
				_MHal_VD_W1B(BK_COMB_C0,u8Comb_C0);
			}

        }
    #endif

    //fine-tune for adaptive Hsync tracking, 20081107
    #if (FINE_TUNE_AFEC_72)
        {
            #define FINE_TUNE_HSYNC_STEP_MAX 20
            #define FINE_TUNE_HSYNC_STEP_THR_L (FINE_TUNE_HSYNC_STEP_MAX/4)
            #define FINE_TUNE_HSYNC_STEP_THR_H (FINE_TUNE_HSYNC_STEP_THR_L*3)
            static U8   u8HsyncStepDebounceCnt=0;
            U16         u16Temp;

            //MDrv_WriteByte(BK_AFEC_04, 0x04);
            _MHal_VD_W1B(BK_AFEC_04, 0x04);

            //u8Ctl = MDrv_ReadByte(BK_AFEC_02); // get VD noise magnitude
            //u16Temp=MDrv_Read2Byte(BK_AFEC_CC);
            u8Ctl = _MHal_VD_R1B(BK_AFEC_02);
            u16Temp= _MHal_VD_R2B(BK_AFEC_CC);

            if (IS_BITS_SET(u16Temp, VD_HSYNC_LOCKED|VD_SYNC_LOCKED|VD_STATUS_RDY))
            {
                if (u8Ctl<=4)
                {
                    if (u8HsyncStepDebounceCnt<FINE_TUNE_HSYNC_STEP_MAX)
                        u8HsyncStepDebounceCnt++;
                }
                else if (u8HsyncStepDebounceCnt)
                {
                    u8HsyncStepDebounceCnt--;
                }
            }
            else
            {
                u8HsyncStepDebounceCnt=0;
            }
            if (u8HsyncStepDebounceCnt>=FINE_TUNE_HSYNC_STEP_THR_H)
            {
                //MDrv_WriteByteMask(BK_AFEC_72, 0x00, (_BIT7|_BIT6));
                _MHal_VD_W1BM(BK_AFEC_72, 0x00, (_BIT7|_BIT6));
            }
            else if (u8HsyncStepDebounceCnt<=FINE_TUNE_HSYNC_STEP_THR_L)
            {
                //MDrv_WriteByteMask(BK_AFEC_72, 0x40, (_BIT7|_BIT6));
                _MHal_VD_W1BM(BK_AFEC_72, 0x40, (_BIT7|_BIT6));
            }
            //printf("u8HsyncStepDebounceCnt=%bd\n",u8HsyncStepDebounceCnt);
            //printf("AFEC_72=%bx\n",MDrv_ReadByte(BK_AFEC_72));
        }
    #endif

		// Patch for non-standard V freq issue (3D COMB)
        {
            U16 u16VTotal;

			if (MHal_VD_GetStatus() & VD_VSYNC_50HZ)
            {
                u16VTotal = 625;
            }
            else
            {
                u16VTotal = 525;
            }

			//if (abs(u16VTotal - _MHal_VD_R2B(BK_AFEC_C5)) >= 2)
			//LGE boae20081116
			// VER8_1112 <-----
            // samuel, 20081110: only when system 3DCOM status allowing to switch status
			if (FALSE == g_system_3dcom_state ||
			 (abs(u16VTotal - (_MHal_VD_R1B(BK_AFEC_C5)|(_MHal_VD_R1B(BK_AFEC_C6)<<8)) ) >= 2) )
			// VER8_1112 ----->
            {
                _MHal_VD_W1BM(BK_COMB_10, 0x02, 0x07);
            }
            else
            {
                _MHal_VD_W1BM(BK_COMB_10, 0x07, 0x07);

                //Merge from utopia CL:135135 20090721 Ethan
                #ifdef AVD_COMB_3D_MID // BY 20090717 enable MID mode after enable 3D comb, if the sequence is wrong, there will be garbage
                if (!IsSVInUse(etSourceVD) && !IsATVInUse(etSourceVD))
                {
                    if ((E_VIDEOSTANDARD_PAL_BGHI==atomic_read(&_aVideoSystemVD) ) || (E_VIDEOSTANDARD_NTSC_M==atomic_read(&_aVideoSystemVD) ))
                    {
                        if (!(u32VDPatchFlagStatus & FINE_TUNE_FH_DOT))
                        {
                            MHal_VD_COMB_Set3dCombMid(ENABLE);
                        }
                    }
                }
                #endif

            }
        }
#if 0
        //Patch for SECAM
        if(IsATVInUse(etSourceVD)||IsAVInUse(etSourceVD)||IsScartInUse(etSourceVD))
        {
            _MHal_VD_W1BM(BK_AFEC_A2, 0x00,(_BIT4|_BIT5)); // 2-line-delay ctrl by VD-DSP
        }
        else
        {
            _MHal_VD_W1BM(BK_AFEC_A2, 0x20,(_BIT4|_BIT5)); // Disable 2-line-delay
        }
#endif
#if 0
        // Fix VD have large noise and use different Comb setting
        {
            _MHal_VD_W1B( BK_AFEC_04, 0x04 );

            u8Ctl = _MHal_VD_R1B( BK_AFEC_02 ); // get VD Noise Magnitude
            if( u8Ctl >= 0x20 )
            {
                u8Ctl = 0x20 ;
            }
            if( u8Ctl != _u8BackupNoiseMagVD )
            {
                _u8BackupNoiseMagVD = ( u8Ctl + _u8BackupNoiseMagVD + 1 ) >> 1;
                //ucValue = tVDtoCombTbl[g_ucBackupNoiseMag][0];
                //msWriteByte(BK6_31, ucValue);
                //ucValue = tVDtoCombTbl[g_ucBackupNoiseMag][1];

                /* LJU */
                //u8Value = tVDtoCombTbl[_u8BackupNoiseMagVD];
                u8Value = gu08CombTbl[_u8BackupNoiseMagVD];
                u8Ctl = _MHal_VD_R1B( BK_COMB_14 );
                _MHal_VD_W1B( BK_COMB_14, (u8Ctl & 0xE0) | u8Value );
            }
        }
#else
        //suggest by james lu, modified by clamp
        {
            #if(PQ_IN_VD_ENABLE)
            _MHal_VD_W1B(BK_COMB_14, 0x00);
            #endif
        }


#endif
        // Fix Comb bug
        {
            u8Value = _MHal_VD_R1B( BK_AFEC_CC );

            if( !( u8Value & _BIT1 ) )
            {
                _MHal_VD_W1B( BK_COMB_5A, 0x20 ); // No Burst (for hsync tolerance)
            }
            else
            {
                _MHal_VD_W1B( BK_COMB_5A, 0x00); // Burst On
            }
        }
    }


#if 0
    if( _MHal_VD_R1B(BK_AFEC_D0) >= 0x47 )
    {
#if (VD_DSP_VER > 329)
        if( ( _MHal_VD_R1B(BK_AFEC_CA) & _BIT2 ) == _BIT2 )
#else
        if( ( _MHal_VD_R1B(BK_AFEC_CE) & _BIT6 ) == _BIT6 )
#endif
        {
            _MHal_VD_W1BM( BK_VBI_50, 0x13, BITMASK(4:0) );
            _MHal_VD_W1BM( BK_VBI_51, 0x13, BITMASK(4:0) );
        }
        else
        {
            _MHal_VD_W1BM( BK_VBI_50, 0x12, BITMASK(4:0) );
            _MHal_VD_W1BM( BK_VBI_51, 0x12, BITMASK(4:0) );
        }
    }
#endif

#if 1
    // for Color Hysteresis
    {
        if ( (MHal_VD_GetStatus() & (VD_HSYNC_LOCKED | VD_STATUS_RDY | VD_COLOR_LOCKED)) == (VD_HSYNC_LOCKED | VD_STATUS_RDY) )
        {
            //	modified to change the initial value of color kill bound (dreamer@lge.com)
            _MHal_VD_W1B( BK_AFEC_D7, _gColorKillBound[0] );
            //printk("COLOR KILL %s: %02x\n", "HIGH", _gColorKillBound[0] );
            //_MHal_VD_W1B( BK_AFEC_D7, COLOR_KILL_HIGH_BOUND );
        }
        else
        {
            //	modified to change the initial value of color kill bound (dreamer@lge.com)
            _MHal_VD_W1B( BK_AFEC_D7, _gColorKillBound[1] );
            //printk("COLOR KILL %s: %02x\n", "LOW", _gColorKillBound[1] );
            //_MHal_VD_W1B( BK_AFEC_D7, COLOR_KILL_LOW_BOUND );
        }
    }
#endif

#if ((PATCH_COMB_ZONEPLATE == 1)) // disable PATCH_COMB_ZONEPLATE temprarily
#if 0
    {
        U8 u8MotionA, u8MotionB, u8Bank;
        static U8 u8PrevLuma;

        // calculate threshold for contrast & brightness within 0x70~0x90
        #define LUMA_7070 0x4E//0x50 //0x4E
        #define LUMA_7090 0x70//0x78 //0x70
        #define LUMA_9070 0x6B//0x66 //0x6B
        #define LUMA_9090 0x86//0x95 //0x86

        if (_MHal_VD_R1B(BK_COMB_6A) & _BIT7) // get threshold from register
        {
            u8Ctl=(_MHal_VD_R1B(BK_COMB_6A) & ~_BIT7)+0x40;
        }
        else
        {
            U8 u8Comb73, u8Comb74;
            u8Comb73=_MHal_VD_R1B(BK_COMB_73);
            if (_MHal_VD_R1B(BK_COMB_73) <=0x70)  u8Comb73 = 0x70;
            if (_MHal_VD_R1B(BK_COMB_73) >=0x90)  u8Comb73 = 0x90;
            u8Comb74=_MHal_VD_R1B(BK_COMB_74);
            if (_MHal_VD_R1B(BK_COMB_74) <=0x70)  u8Comb74 = 0x70;
            if (_MHal_VD_R1B(BK_COMB_74) >=0x90)  u8Comb74 = 0x90;
            {
                U8 u8Brightness70=(LUMA_7070+(WORD)(u8Comb73-0x70)*(LUMA_9070-LUMA_7070)/(0x90-0x70));
                U8 u8Brightness90=(0x78+(WORD)(u8Comb73-0x70)*(LUMA_9090-LUMA_7090)/(0x90-0x70));
                u8Ctl=u8Brightness70+((WORD)(u8Comb74-0x70)*(WORD)(u8Brightness90-u8Brightness70))/(0x90-0x70);
            }
            _MHal_VD_W1BM(BK_COMB_6A, u8Ctl-0x40, ~_BIT7);
        }
        #undef LUMA_9090
        #undef LUMA_9070
        #undef LUMA_7090
        #undef LUMA_7070

        //_MHal_VD_W1B(BK_AFEC_D0, u8Ctl);
        u8Value=(WORD)((WORD)u8PrevLuma*7+((WORD)stDlcInfo.u8AvgHistogram* 236)/(_MHal_VD_R1B(BK_AFEC_DA)))>>3;
        u8PrevLuma=u8Value;
        //_MHal_VD_W1B(BK_AFEC_D1, u8Value);
        u8Bank = _MHal_VD_R1B(BK_SELECT_00);
        _MHal_VD_W1B(BK_SELECT_00, REG_BANK_OP1ZZ);
        u8MotionA = _MHal_VD_R1B(L_BK_OP1ZZ(0x1F)) & 0x7F;
        u8MotionB = ((_MHal_VD_R1B(L_BK_OP1ZZ(0x1B)) & 0x07) << 4) |
                    ((_MHal_VD_R1B(L_BK_OP1ZZ(0x1A)) & 0xF0) >> 4) ;
        _MHal_VD_W1B(BK_SELECT_00, u8Bank);

        if(IsATVInUse(etSourceVD) && u8Value > 2)
        {
        //#if (defined(SATURN_IV))
        #if (PLATFORM_MODEL >= PLATFORM_MSD5018)
            _MHal_VD_W1BM(BK_COMB_22, _BIT1|_BIT0, _BIT1|_BIT0);
        #else
            _MHal_VD_W1BM(BK_COMB_48, _BIT3|_BIT2, _BIT3|_BIT2);
        #endif
        }
        else if(IsATVInUse(etSourceVD))
        {
            if(u8MotionA < 10 && (u8MotionB == 0x7F || u8MotionB < 0x10))
            {
            //#if (defined(SATURN_IV))
            #if (PLATFORM_MODEL >= PLATFORM_MSD5018)
                if(u8Value < (u8Ctl-0x20))
                {
                    _MHal_VD_W1BM(BK_COMB_22, _BIT1|_BIT0, _BIT1|_BIT0);
                    _MHal_VD_W1BM(BK_COMB_9F, 0x06, _BIT4|_BIT3|_BIT2|_BIT1|_BIT0);
                }
                else if(u8Value > (u8Ctl-0x10))
                {
                    _MHal_VD_W1BM(BK_COMB_22, 0x00, _BIT1|_BIT0);
                    _MHal_VD_W1BM(BK_COMB_9F, 0x07, _BIT4|_BIT3|_BIT2|_BIT1|_BIT0);
                }
                else
                {
                    _MHal_VD_W1BM(BK_COMB_22, _BIT0, _BIT1|_BIT0);
                    _MHal_VD_W1BM(BK_COMB_9F, 0x06, _BIT4|_BIT3|_BIT2|_BIT1|_BIT0);
                }
            #else
                if(u8Value < (u8Ctl-0x20))
                    _MHal_VD_W1BM(BK_COMB_48, _BIT3|_BIT2, _BIT3|_BIT2);
                else if(u8Value > (u8Ctl-0x10))
                    _MHal_VD_W1BM(BK_COMB_48, 0x00, _BIT3|_BIT2);
                else
                    _MHal_VD_W1BM(BK_COMB_48, _BIT2, _BIT3|_BIT2);
            #endif
            }
            else
            {
            //#if (defined(SATURN_IV))
            #if (PLATFORM_MODEL >= PLATFORM_MSD5018)
                _MHal_VD_W1BM(BK_COMB_22, _BIT1|_BIT0, _BIT1|_BIT0);
            #else
                _MHal_VD_W1BM(BK_COMB_48, _BIT3|_BIT2, _BIT3|_BIT2);
            #endif
            }

        }
        else if(IsAVInUse(etSourceVD) || IsScartInUse(etSourceVD) )
        {
            if(((atomic_read(&_aVideoSystemVD) == E_VIDEOSTANDARD_PAL_BGHI ||
                atomic_read(&_aVideoSystemVD) == E_VIDEOSTANDARD_PAL_M    ||
                atomic_read(&_aVideoSystemVD) == E_VIDEOSTANDARD_PAL_N) && u8MotionA < 10) || //PAL
               (atomic_read(&_aVideoSystemVD) == E_VIDEOSTANDARD_NTSC_M && u8MotionA < 10 && u8MotionB == 0x7F)) // NTSC
            {
            //#if (defined(SATURN_IV))
            #if (PLATFORM_MODEL >= PLATFORM_MSD5018)
                #ifdef ZONEPLATE_LG
                    static U16 u16SaturationSum = 0x00; // for LG demo ZONE play
                    u16SaturationSum =  (u16SaturationSum  +  MDrv_DLC_GetSaturationSum()) /2;
                #endif
                if(u8Value < (u8Ctl-0x20))
                {
                    _MHal_VD_W1BM(BK_COMB_22, _BIT1|_BIT0, _BIT1|_BIT0);
                    _MHal_VD_W1BM(BK_COMB_9F, 0x06, _BIT4|_BIT3|_BIT2|_BIT1|_BIT0);
                    #ifdef ZONEPLATE_LG
                        _MHal_VD_W1B(BK_COMB_71, 0xFF); // for LG demo ZONE play
                    #endif
                }
                else if(u8Value > (u8Ctl-0x10))
                {
                    _MHal_VD_W1BM(BK_COMB_22, 0x00, _BIT1|_BIT0);
                    _MHal_VD_W1BM(BK_COMB_9F, 0x07, _BIT4|_BIT3|_BIT2|_BIT1|_BIT0);

                    #ifdef ZONEPLATE_LG
                        if(u16SaturationSum < 0x80)
                            _MHal_VD_W1B(BK_COMB_71, 0xF7); // for LG demo ZONE play
                        else
                            _MHal_VD_W1B(BK_COMB_71, 0xFF); // for LG demo ZONE play
                    #endif
                }
                else
                {
                    _MHal_VD_W1BM(BK_COMB_22, _BIT0, _BIT1|_BIT0);
                    _MHal_VD_W1BM(BK_COMB_9F, 0x06, _BIT4|_BIT3|_BIT2|_BIT1|_BIT0);
                    #ifdef ZONEPLATE_LG
                        if(u16SaturationSum < 0x80)
                            _MHal_VD_W1B(BK_COMB_71, 0xFB); // for LG demo ZONE play
                        else
                            _MHal_VD_W1B(BK_COMB_71, 0xFF); // for LG demo ZONE play
                        #endif
                }
            #else
                if(u8Value < (u8Ctl-0x20))
                    _MHal_VD_W1BM(BK_COMB_48, _BIT3|_BIT2, _BIT3|_BIT2);
                else if(u8Value > (u8Ctl-0x10))
                    _MHal_VD_W1BM(BK_COMB_48, 0x00, _BIT3|_BIT2);
                else
                    _MHal_VD_W1BM(BK_COMB_48, _BIT2, _BIT3|_BIT2);
            #endif
            }
            else
            {
            //#if (defined(SATURN_IV))
            #if (PLATFORM_MODEL >= PLATFORM_MSD5018)
                _MHal_VD_W1BM(BK_COMB_22, _BIT1|_BIT0, _BIT1|_BIT0);
            #else
                _MHal_VD_W1BM(BK_COMB_48, _BIT3|_BIT2, _BIT3|_BIT2);
            #endif
            }
        }

        // comb filter for low signal compesation
        if(IsATVInUse(etSourceVD))
        {
            _MHal_VD_W1B(BK_AFEC_04, 0x04);
            u8Ctl = _MHal_VD_R1B(BK_AFEC_02); // get VD noise magnitude
            u8Value = (1*_u8PreNoiseMagVD + 1*u8Ctl)/2;
            _u8PreNoiseMagVD = u8Value;

            u8Ctl = _MHal_VD_R1B(BK_COMB_38);
            if(u8Value <= 0x02)
            {
                _MHal_VD_W1B(BK_COMB_37, 0x8A);
                _MHal_VD_W1B(BK_COMB_38, (u8Ctl & 0xF8));
            }
            else
            {
                _MHal_VD_W1B(BK_COMB_37, 0x88);
                _MHal_VD_W1B(BK_COMB_38, (u8Ctl & 0xF8) | 0x06);
            }

            if(u8Value >= 0x06)
            {
                _MHal_VD_W1Rb(BK_COMB_37, 0, _BIT7);
            }

        }
        else if(IsAVInUse(etSourceVD))
        {
            _MHal_VD_W1B(BK_COMB_37, 0x8A); // for LG demo
        }

        //#if (defined(SATURN_IV))
        #if (PLATFORM_MODEL >= PLATFORM_MSD5018)
            if(atomic_read(&_aVideoSystemVD) == E_VIDEOSTANDARD_PAL_BGHI ||
            atomic_read(&_aVideoSystemVD) == E_VIDEOSTANDARD_PAL_M ||
            atomic_read(&_aVideoSystemVD) == E_VIDEOSTANDARD_PAL_N)
                _MHal_VD_W1Rb(BK_COMB_37, 0, _BIT5);
        #endif
    }
#endif
#elif(PATCH_COMB_STILLIMAGE==1)
    {
        #if 1
        // comb filter for low signal compesation
        if(IsATVInUse(etSourceVD))
        {
            _MHal_VD_W1B(BK_AFEC_04, 0x04);
            u8Ctl = _MHal_VD_R1B(BK_AFEC_02); // get VD noise magnitude
            u8Value = (1*_u8PreNoiseMagVD + 1*u8Ctl)/2;
            _u8PreNoiseMagVD = u8Value;

            u8Ctl = _MHal_VD_R1B(BK_COMB_38);
            if(u8Value <= 0x02)
            {
                _MHal_VD_W1B(BK_COMB_37, 0x8A);
                _MHal_VD_W1B(BK_COMB_38, (u8Ctl & 0xF8));
            }
            else
            {
                _MHal_VD_W1B(BK_COMB_37, 0x88);
                _MHal_VD_W1B(BK_COMB_38, ((u8Ctl & 0xF8) | 0x06));
            }

            if(u8Value >= 0x06)
            {
                _MHal_VD_W1Rb(BK_COMB_37, 0, _BIT7);
            }
        }
        #endif
      }
#endif

    // co-channel
    {
        if (_MHal_VD_R1Rb(BK_AFEC_CA, _BIT3))
        {
            _MHal_VD_W1Rb(BK_VBI_8D, ENABLE, _BIT6);
        }
        else
        {
            _MHal_VD_W1Rb(BK_VBI_8D, DISABLE, _BIT6);
        }
    }

	// shjang_091020
	if (u8VDScanFlagStatus == FALSE)
	{
		// For maintain the Hsync reliability and seneitivity
		{
			static U8 u8DebounceCnt=0;

			if (!(_MHal_VD_R1B(BK_AFEC_CD) & (_BIT7|_BIT6)))
			{
				if (u8DebounceCnt < 15)
				{
					u8DebounceCnt++;
				}
			}
        	else
        	{
            	if (u8DebounceCnt > 0)
            	{
                	u8DebounceCnt--;
            	}
        	}

        	if (u8DebounceCnt >= 10)
        	{
            	_MHal_VD_W1BM(BK_AFEC_39, 0x00, _BIT1 | _BIT0);    // more sensitivity
        	}
        	else if (u8DebounceCnt <= 5)
        	{
            	_MHal_VD_W1BM(BK_AFEC_39, 0x03, _BIT1 | _BIT0);    // more reliability
        	}
		}
	}

	/* PLL Tracking Speed up */
    #ifdef PLL_TRACKING_SPEEDUP
    if ( _bPLLTrigerTrack == TRUE )
    {
        _u32PLLTrackingTime = jiffies;
        _MHal_VD_W1B(BK_AFEC_A0, 0xBC);
        _MHal_VD_W1B(BK_AFEC_A1, 0x6A);
        _bPLLTracking = TRUE;
        _bPLLTrigerTrack = FALSE;
    }

    if( _bPLLTracking)
    {
        if(jiffies - _u32PLLTrackingTime > (HZ>>2))
        {
            _bPLLTracking = FALSE;
            _bPLLTrigerTrack = FALSE;

			// shjang_100322 H Pll status
			if (u8HPllstatus > 0) // Manual
				MHal_VD_ManualHPllTrackingControl(u8HPllstatus);
			else	// Auto
            	_MHal_VD_W1Rb(BK_AFEC_A0, 0, _BIT7);
        }
    }
    #endif

	/* Driver uses timer to check when it has to disable 3D comb speed up 	*/
	/* Driver uses timer to check when it has to enable 3D comb speed up. */
    #ifdef LOCK3DSPEEDUP
    {
#if 0
        {  //20090827EL
            MS_BOOL bBurstOn;
            static MS_BOOL bPrevBurstOn=FALSE;

            bBurstOn = _MHal_VD_R1B( BK_AFEC_CC ) & _BIT1;
            if ( bBurstOn  )
            {
                if ( !bPrevBurstOn )
                {
                    MHal_VD_3DCombSpeedup(TRUE);
                    _bUnstableChecking = TRUE;
                    _u32VideoSystemTimer = jiffies;
                    //printk(" ^^^ 20090827 ^^^111  MHal_VD_3DCombSpeedup(TRUE);\n");
                }
            }
            bPrevBurstOn=bBurstOn;
        }
#endif

        /* if driver wants to enable 3D comb speed up. */
        if ( _bComb10Bit3Flag )
        {
            /* if brust is on, we enable 3D comb speed up*/
            //if (MHal_VD_GetStatus() & _BIT1)
            {
                /*
                3D comb speed up condition :
                1. driver want to enable 3D comb speed up.
                2. video signal's brust is on.
                3. 3D comb speed up is not enabled.
                */
                if(_bStartCheckCombSpeedup == FALSE)
                {
                    MHal_VD_3DCombSpeedup(TRUE);
                    _bComb10Bit3Flag=TRUE;
                    /* start to check time for 3D comb. */
                    _bStartCheckCombSpeedup = TRUE;
                    _u32VideoSystemTimer = jiffies;
                    VD_KDBG("3D SPEED UP OPEN\n");
                }
            }
        }
    }

    if(_bStartCheckCombSpeedup == FALSE)
    {
        /* updates time stamp if driver doesn't need to check time for 3D comb. */
        _u32VideoSystemTimer = jiffies;
    }
    #endif

#if 0  //Remove this patch to pass FSC tolerance 400 HZ test by Howard's suggestion 20090722
	{
		static U8 u8Preunlockstate=0xff;
		U8 u8Temp;
		u8Temp = _MHal_VD_R1B(BK_COMB_E0);
		_MHal_VD_W1B(BK_COMB_E0, 0xFF);
		if ((!u8Preunlockstate)&&((u8Temp&_BIT2))&&(!_bComb10Bit3Flag)) // got comb 3D unlocked
		{
			MHal_VD_3DCombSpeedup(TRUE);
			_bUnstableChecking = TRUE;
			_u32VideoSystemTimer = jiffies;
		}
		u8Preunlockstate=(u8Temp&_BIT2);
	}
#endif  //Remove this patch to pass FSC tolerance 400 HZ test by Howard's suggestion 20090722

	/* Driver uses timer to check when it has to disable 3D comb speed up 	*/
    #ifdef LOCK3DSPEEDUP
    {
		if ( _bComb10Bit3Flag || _bUnstableChecking)
        {
            /* if time stamp is return to zero. we just ingore it and check it next time. */
            if(_u32VideoSystemTimer < jiffies)
            {
                /* use time stamp to check when driver has to disable 3D comb speed up */
                if (jiffies - _u32VideoSystemTimer > (HZ/66)*100 )
                {
                    MHal_VD_3DCombSpeedup(FALSE);
                    _bComb10Bit3Flag=FALSE;
                    _bUnstableChecking = FALSE;

                    /* disable Comb check. */
                    _bStartCheckCombSpeedup = FALSE;
                    VD_KDBG("3D SPEED UP CLOSE\n");
                }
            }
            else
            {
                _u32VideoSystemTimer = jiffies;
            }
        }
    }
    #endif

    {//lachesis_091021
		if(_bCheckSwingThreshold)
		{
			if(jiffies - _u32SwingThresholdStartTime > (HZ*3))
			{
				MHal_VD_SetSigSWingThreshold(2);
				VD_KDBG("\n=====MHal_VD_SetSigSWingThreshold disable\n");
			}
		}
    }

#if 0
	{
		U8 u8buffer[16];
		_MHal_VD_W1B( BK_AFEC_04, 0x01 );
		u8buffer[0] = _MHal_VD_R1B( BK_AFEC_01 );
		u8buffer[1] = _MHal_VD_R1B( BK_AFEC_02 );
		u8buffer[2] = _MHal_VD_R1B( BK_AFEC_03 );

		_MHal_VD_W1B( BK_AFEC_04, 0x04 );
		u8buffer[3] = _MHal_VD_R1B( BK_AFEC_03 );

		_MHal_VD_W1B( BK_AFEC_04, 0x09 );
		u8buffer[4] = _MHal_VD_R1B( BK_AFEC_02 );

		u8buffer[5] = _MHal_VD_R1B( BK_AFEC_D0 );
		u8buffer[6] = _MHal_VD_R1B( BK_AFEC_D1 );
		u8buffer[7] = _MHal_VD_R1B( BK_AFEC_D2 );
		u8buffer[8] = _MHal_VD_R1B( BK_AFEC_D3 );

		printk("VD:%x %x %x %x %x %x %x %x %x\n", u8buffer[0], u8buffer[1], u8buffer[2], u8buffer[3], u8buffer[4],u8buffer[5], u8buffer[6], u8buffer[7], u8buffer[8]);
	}

#endif
}
#endif

///-----------------------------------------------------------------------------
/// To set hsync detection for tuning
/// @param  B16:
/// - TRUE  : Auto scan mode
/// - FALSE : Normal mode
/// @return void
/// @note
///-----------------------------------------------------------------------------
void MHal_VD_SetHsyncDetectionForTuning(B16 bEnableVD)
{
    // for Auto Scan Mode
    if (bEnableVD)
    {
        _MHal_VD_W1B (BK_AFEC_99, (HSEN_CHAN_SCAN_DETECT_WIN_AFTER_LOCK & 0x0F) << 4 | (HSEN_CHAN_SCAN_DETECT_WIN_BEFORE_LOCK & 0x0F));

        _MHal_VD_W1BM (BK_AFEC_9A, HSEN_CHAN_SCAN_CNTR_FAIL_BEFORE_LOCK << 4, BITMASK(7:4));
        _MHal_VD_W1BM (BK_AFEC_9B, HSEN_CHAN_SCAN_CNTR_SYNC_BEFORE_LOCK << 0, BITMASK(5:0));

        _MHal_VD_W1BM (BK_AFEC_9C, HSEN_CHAN_SCAN_CNTR_SYNC_AFTER_LOCK << 0, BITMASK(5:0));

        // _MHal_VD_W1BM (BK_AFEC_94, 3 << 3, BITMASK(4:3));
        _MHal_VD_W1BM (BK_AFEC_D8, 0, BITMASK(7:4));

		// shjang_091020
		u8VDScanFlagStatus = TRUE;
		_MHal_VD_W1BM(BK_AFEC_39, 0x00, _BIT1 | _BIT0);
    }
    // for Normal Mode
    else
    {
        _MHal_VD_W1B (BK_AFEC_99, (HSEN_NOAMRL_DETECT_WIN_AFTER_LOCK & 0x0F) << 4 | (HSEN_NORMAL_DETECT_WIN_BEFORE_LOCK & 0x0F));

        _MHal_VD_W1BM (BK_AFEC_9A, HSEN_NORMAL_CNTR_FAIL_BEFORE_LOCK << 4, BITMASK(7:4));
        _MHal_VD_W1BM (BK_AFEC_9B, HSEN_NORMAL_CNTR_SYNC_BEFORE_LOCK << 0, BITMASK(5:0));

        _MHal_VD_W1BM (BK_AFEC_9C, HSEN_NORMAL_CNTR_SYNC_AFTER_LOCK << 0, BITMASK(5:0));

        // _MHal_VD_W1BM (BK_AFEC_94, 2 << 3, BITMASK(4:3));

        _MHal_VD_W1BM (BK_AFEC_D8, SIG_SWING_THRESH << 4, BITMASK(7:4));

		// shjang_091020
		u8VDScanFlagStatus = FALSE;
		_MHal_VD_W1BM(BK_AFEC_39, 0x03, _BIT1 | _BIT0);
    }


}


///-----------------------------------------------------------------------------
/// To enable 3D COMB
/// @param  B16:
/// - TRUE  : Enable 3D Comb.
/// - FALSE : Disable 3D Comb.
/// @return void
/// @note
///-----------------------------------------------------------------------------
void MHal_VD_Set3dComb(B16 bEnableVD)
{
/* 3D comb settings is only BIT[0-2] in the COMB10. */
#if defined(COMB_3D)
    if (bEnableVD)
    {
        //_MHal_VD_W1BM( BK_COMB_10, 0x17, 0x17 );
        _MHal_VD_W1BM(BK_COMB_10, 0x07, 0x07);
        _b3DCombState = TRUE;
    }
    else
    {
        #ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
        MHal_VD_COMB_Set3dCombMid(DISABLE);
        #endif

        //_MHal_VD_W1BM( BK_COMB_10, 0x12, 0x17 );
        _MHal_VD_W1BM(BK_COMB_10, 0x02, 0x07);
        _b3DCombState = FALSE;
    }
#else

    #ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
    MHal_VD_COMB_Set3dCombMid(DISABLE);
    #endif

    //_MHal_VD_W1BM( BK_COMB_10, 0x12, 0x17 );
    _MHal_VD_W1BM(BK_COMB_10, 0x02, 0x07);
    _b3DCombState = FALSE;
#endif
}


void _MHal_VD_TimeDelay(void)
{
    /* Time unit is 15.625 ms */

}

///-----------------------------------------------------------------------------
/// To reset VD mcu address to 0x0000.
/// @param  void
/// @return void
/// @note   <private function>
/// @type
///-----------------------------------------------------------------------------
void MHal_VD_McuReset(void)
{
    VD_KDBG("MHal_VD_McuReset()\n"); // <-<<<
    //printk("MHal_VD_McuReset\r\n");// 20091118
    _MHal_VD_W1Rb(VD_MCU_RESET, 0x01, _BIT0);

    /* You cannot use msleep in any interrupt */
    //msleep(10);
    //atomic_set(&_aMCUResetCounterVD, 1);
    mdelay(10);


    _MHal_VD_W1Rb(VD_MCU_RESET, 0x00, _BIT0);
    /* You cannot use msleep in any interrupt */
    //msleep(200);
#if 0
    mdelay(300);
#else
    msleep(30);	//lachesis_080918 입력전환 시간 개선.
#endif

	/* Driver set DSP firmware vertical line to 525 after MCU reset.
	    LGE skystone 20090109
	*/
	if(MHal_VD_GetDSPVer() == VD_SYSTEM_NTSC) /* ATSC */// Set Analog Color System - 090309.
		_MHal_VD_W1BM (BK_AFEC_CF, (BIT0|BIT1), (BIT0|BIT1));
	else
		_MHal_VD_W1BM (BK_AFEC_CF, (0x00), (BIT0|BIT1));
    // Forced to PAL mode
    // _MHal_VD_SetAutoDetetion(FSC_AUTO_DET_DISABLE);
    // _MHal_VD_SetFSCType(FSC_MODE_PAL);
    //W_VAR_LOCKED(_u32ForceVideoStandardVD = 0, _u32ForceVideoStandardVD);
    atomic_set(&_aForceVideoStandardVD, 0);

	if(!u8VDScanFlagStatus)
	{
    	_MHal_VD_W1BM (BK_AFEC_D8, (SIG_SWING_THRESH << 4), BITMASK(7:4));
	}

#ifdef ENABLE_NTSC_50
    _MHal_VD_W1Rb(BK_AFEC_CE, ENABLE, _BIT5);
#endif

    VD_KDBG("MHal_VD_McuReset() END\n"); // <-<<<
}

///-----------------------------------------------------------------------------
/// To reset VD mcu address to 0x0000 and halt it.
/// @param  void
/// @return void
/// @note   <private function>
/// @type
///-----------------------------------------------------------------------------
void MHal_VD_McuReset_Stop(void)
{
	MHal_VD_McuReset();
}




const U16 SPL_NSPL[5] =
{
    1135,   // PAL
    1097,   // SECAM
    910,    // NTSC, PAL M
    917,    // PAL Nc
    1127,   // NTSC 4.43
};

// VER14_1220
// Check VD states in audio carrie.
U16 MHal_VD_CheckStatusLoop(void)//channel skip test 090113
{
	U16 u16CurrentStatus =0;
	U16 u16Dummy, u16Temp, _u16LatchHVD;
	U8 u8Value;
	U16 u8VdHsyncLockedCount;

	u16Dummy = 800; // just protect program dead lock
	u8VdHsyncLockedCount = 0;
    //msleep(60);
#if 0
	while( u16Dummy-- )
	{
		_MHal_VD_W1B( BK_AFEC_04, 0x00 );
		//udelay(100);
		//msleep(1);
		if( (_MHal_VD_R1B(BK_AFEC_01) & 0xF0) >= 0x60 )
		{
			_MHal_VD_W1B( BK_AFEC_04, 0x21 );
			//udelay(100);
    		//msleep(1);
			u8Value = _MHal_VD_R1B(BK_AFEC_01);
			if( u8Value & _BIT3 ) // check sync found
			{
				u16CurrentStatus = MHal_VD_GetStatus();
				//if(u16CurrentStatus & (_BIT14|_BIT15))
				    u8VdHsyncLockedCount++;
			}
		}
	}
#else
	while( u16Dummy-- )
	{
		u16CurrentStatus = MHal_VD_GetStatus();
		if(u16CurrentStatus & (_BIT14|_BIT15))
			u8VdHsyncLockedCount++;
	}
#endif
	//VD_KDBG("H1 %d\n",u8VdHsyncLockedCount);

	if( u8VdHsyncLockedCount>560)
	{
		u8VdHsyncLockedCount = 0;
		u16Dummy = 800; // just protect program dead lock
		_u16LatchHVD = _MHal_VD_R1B(BK_AFEC_C7)+(_MHal_VD_R1B(BK_AFEC_C8)<<8); // SPL_NSPL, H total

		while( u16Dummy-- )
		{
			//msleep(17);	// don't delete (ch skip prob)
            //udelay(100);
			u16Temp = _MHal_VD_R1B(BK_AFEC_C7)+(_MHal_VD_R1B(BK_AFEC_C8)<<8); // SPL_NSPL, H total
			u16Temp = (u16Temp+_u16LatchHVD)>>1;
			_u16LatchHVD = u16Temp;

			for(u8Value=0; u8Value<=4;u8Value++)
			{
				if( abs(_u16LatchHVD-SPL_NSPL[u8Value]) < 10 )
				{
					u8VdHsyncLockedCount++;
				}
				else
				{
				}
			}
		}

        if(u8VdHsyncLockedCount < 640)
        {
            u16CurrentStatus = 0;
        }

	}
	else
	{
		u16CurrentStatus = 0;
	}
    //VD_KDBG("H2 %d\n",u8VdHsyncLockedCount);

	return u16CurrentStatus;
}

U16 _MHal_VD_GetOutputVT(void)
{
    return _MHal_VD_R1B(BK_AFEC_C5) | (_MHal_VD_R1B(BK_AFEC_C6)<<8);
}

U16 _MHal_VD_GetOutputHT(void)
{
    return ((U16)_MHal_VD_R1B(BK_AFEC_9D)<<3) | (_MHal_VD_R1B(BK_AFEC_9E)>>5);
}

void MHal_VD_ResetAGC(void)
{
	//AFEC_D8[3]: 1 for reset AGC, it will auto clear to 0
	_MHal_VD_W1BM(BK_AFEC_D8, BIT3, BIT3);
}

//lachesis_100118 for indonesia
void MHal_VD_SetAdaptiveHsyncTracking(B16 bEnable)
{
	//AFEC_D8[1]: 1 is enable, 0 is disable
	_MHal_VD_W1BM(BK_AFEC_D8, bEnable, BIT1);
	VD_KDBG("\nMHal_VD_SetAdaptiveHsyncTracking[D8]=%d\n", bEnable);
}

void MHal_VD_SetAdaptiveHsyncSlice(B16 bEnable)
{
	//AFEC_D9[1]: 1 is enable, 0 is disable
	_MHal_VD_W1BM(BK_AFEC_D9, bEnable, BIT1);
	VD_KDBG("\nMHal_VD_SetAdaptiveHsyncSlice[D9]=%d\n", bEnable);
}

// shjang_091020
void MHal_VD_SetSigSWingThreshold(U8 Threshold)
{
	_MHal_VD_W1BM (BK_AFEC_D8, Threshold << 4, BITMASK(7:4));

	if(Threshold == 0)
	{
		_bCheckSwingThreshold = TRUE;
		_u32SwingThresholdStartTime = jiffies;
		VD_KDBG("==>start  _bCheckSwingThreshold=%d\n", Threshold);
	}
	else
	{
		_bCheckSwingThreshold = FALSE;
	}
}

//------------------------------------------------------------------------------
//
//  Start of Local Function Implementation
//
//------------------------------------------------------------------------------

// <081028 zuel > DC - Off /On 시 래치업 문제 대응 - MSTAR
#if 0
static void _SetHTotal (U32 u32HTotal)
{
    U32 u32Mid;

#if ENABLE_DYNAMIC_VD_HTOTAL
    _MHal_VD_W1Rb(BK_AFEC_A0, ENABLE, _BIT7);
#endif

    u32HTotal = (u32HTotal + 1) & 0xFFFE;
    u32Mid = ((U16)_MHal_VD_R1B(BK_AFEC_9D) << 3) |((U16)_MHal_VD_R1B(BK_AFEC_9E)>>5);

    // this operation doesn't need.
    u32Mid = MAKEWORD(_MHal_VD_R1B(BK_AFEC_9D), _MHal_VD_R1B(BK_AFEC_9E));

    u32Mid = u32Mid>>5;

    /* first adjust pll to (assigned value)/2 */
    u32Mid = (u32Mid+u32HTotal)>>1;
    _MHal_VD_W1B(BK_AFEC_9D, u32Mid >> 3);
    _MHal_VD_W1B(BK_AFEC_9E, (U8)(u32Mid &0x07) << 5);


    //MDrv_Timer_Delayms(75);
    //msleep(75);
    mdelay(75);

    _MHal_VD_W1B(BK_AFEC_9D, u32HTotal >> 3);
    _MHal_VD_W1B(BK_AFEC_9E, (U8)(u32HTotal & 0x07) << 5);


#if ENABLE_DYNAMIC_VD_HTOTAL
    _MHal_VD_W1Rb(BK_AFEC_A0, DISABLE, _BIT7);
#endif

    if (_u8OneshotFlag == 0) // LGE swwoo
    {
        _u8OneshotFlag = 1;
        _MHal_VD_W1Rb(BK_AFEC_14, ENABLE, _BIT7);	// 20080718 swwoo MSTAR th.chen recommned.
        _MHal_VD_W1Rb(BK_AFEC_14, DISABLE, _BIT7);	// 20080718 swwoo MSTAR th.chen recommned.
    }

#if 0 // (PATCH_PAL_VTOTAL624SHAKE)
    _MHal_VD_W2B(BK_COMB_52, u32HTotal);
    _MHal_VD_W1Rb(BK_COMB_50, DISABLE, _BIT0);
#endif
}
#endif

static void _SetHTotal (U32 u32HTotal)
{
    U32 u32Mid;

    // These variables use to save old 3D comb and TTX slicer states.
    // Driver uses these to return the settings after AFEC reset.
    U8  u8IsCombEnable;
    U8  u8IsTTXSlicerEnable;

    // *(volatile unsigned int*)(0xA000000C) = 0xD001 ;
#if ENABLE_DYNAMIC_VD_HTOTAL
    _MHal_VD_W1Rb(BK_AFEC_A0, ENABLE, _BIT7);
#endif
    // *(volatile unsigned int*)(0xA000000C) = 0xD002 ;
    u32HTotal = (u32HTotal + 1) & 0xFFFE;
    // *(volatile unsigned int*)(0xA000000C) = 0xD003 ;
    u32Mid = ((U16)_MHal_VD_R1B(BK_AFEC_9D) << 3) |((U16)_MHal_VD_R1B(BK_AFEC_9E)>>5);
    // *(volatile unsigned int*)(0xA000000C) = 0xD004 ;
    // this operation doesn't need.
    u32Mid = MAKEWORD(_MHal_VD_R1B(BK_AFEC_9D), _MHal_VD_R1B(BK_AFEC_9E));
    // *(volatile unsigned int*)(0xA000000C) = 0xD005 ;
    u32Mid = u32Mid>>5;
    //     *(volatile unsigned int*)(0xA000000C) = 0xD006 ;
    /* first adjust pll to (assigned value)/2 */
    u32Mid = (u32Mid+u32HTotal)>>1;
    //     *(volatile unsigned int*)(0xA000000C) = 0xD007 ;
    _MHal_VD_W1B(BK_AFEC_9D, u32Mid >> 3);
    //     *(volatile unsigned int*)(0xA000000C) = 0xD008 ;
    _MHal_VD_W1B(BK_AFEC_9E, (U8)(u32Mid &0x07) << 5);
    //     *(volatile unsigned int*)(0xA000000C) = 0xD009 ;

    //MDrv_Timer_Delayms(75);
    //msleep(75);
    mdelay(75);
    //     *(volatile unsigned int*)(0xA000000C) = 0xD00A ;

    _MHal_VD_W1B(BK_AFEC_9D, u32HTotal >> 3);
    //     *(volatile unsigned int*)(0xA000000C) = 0xD00B ;
    _MHal_VD_W1B(BK_AFEC_9E, (U8)(u32HTotal & 0x07) << 5);
    //     *(volatile unsigned int*)(0xA000000C) = 0xD00C ;

//    mdelay(15);

#if ENABLE_DYNAMIC_VD_HTOTAL
	if (u8HPllstatus == 0)	// shjang_100322
    	_MHal_VD_W1Rb(BK_AFEC_A0, DISABLE, _BIT7);
    //     *(volatile unsigned int*)(0xA000000C) = 0xD00D ;
#endif

    if (_u8OneshotFlag == 0) // LGE swwoo
    {
    //         *(volatile unsigned int*)(0xA000000C) = 0xD00E ;
        _u8OneshotFlag = 1;
    //         *(volatile unsigned int*)(0xA000000C) = 0xD00F ;

        // Save 3D comb and TTX slicer states and return settings after AFEC reset.
        // use to fix vd AFEC reset latch problem.
        u8IsCombEnable = _MHal_VD_R1B(BK_COMB_10);
        u8IsTTXSlicerEnable = _MHal_VD_R1B(BK_VBI_10);

        _MHal_VD_W1B(BK_VBI_10, 0);	    // disable TTX slicer.
        //MHal_VD_Set3dComb(FALSE);       // disable 3D COMB.
        _MHal_VD_W1BM(BK_COMB_10, 0x02, 0x07);	//lachesis_081117 3dcomb enable 안되는 문제수정. from mstar
        mdelay(3);

        _MHal_VD_W1B(BK_AFEC_C2, 1);
        _MHal_VD_W1B(BK_AFEC_C0, 0xFA);
        {
            U8 u8buffer[2];
            U8 u8timeout = 100;
            do{

                u8buffer[0]=_MHal_VD_R1B(BK_AFEC_D6);
                mdelay(2);
                u8buffer[1]=_MHal_VD_R1B(BK_AFEC_D6);
                u8timeout--;
            } while((u8buffer[0]!=u8buffer[1])&&(u8timeout>0));
        }

        _MHal_VD_W1Rb(BK_AFEC_14, ENABLE, _BIT7);	// 20080718 swwoo MSTAR th.chen recommned.
        mdelay(10);
        _MHal_VD_W1Rb(BK_AFEC_14, DISABLE, _BIT7);	// 20080718 swwoo MSTAR th.chen recommned.
        mdelay(3);
        _MHal_VD_W1B(BK_AFEC_C2, 0);

		// recover old settings
        _MHal_VD_W1B(BK_COMB_10, u8IsCombEnable);
        _MHal_VD_W1B(BK_VBI_10, u8IsTTXSlicerEnable);
        //MHal_VD_Set3dComb(TRUE);                    // enable 3D COMB.
        //_MHal_VD_W1B(BK_VBI_10, 1);	                // enable TTX slicer.

    //         *(volatile unsigned int*)(0xA000000C) = 0xD011 ;
    }

#if 0 // (PATCH_PAL_VTOTAL624SHAKE)
    _MHal_VD_W2B(BK_COMB_52, u32HTotal);
    _MHal_VD_W1Rb(BK_COMB_50, DISABLE, _BIT0);
#endif
    //     *(volatile unsigned int*)(0xA000000C) = 0xD012 ;
}



static void _EnableCVBSLPF(B16 bEnable)
{
    _MHal_VD_W1Rb(BK_AFEC_67, bEnable, _BIT4);
    _MHal_VD_W1Rb(BK_AFEC_CF, bEnable, _BIT2);
}

#ifdef LOCK3DSPEEDUP
/*
    A parameter to enable or disable 3D comb speed up.
    I add a factor (COMB 5F) to speed up 3D comb fastly.
*/
void MHal_VD_3DCombSpeedup(B16 bEnable)
{
    #ifdef COMB_3D
	if(bEnable == TRUE)
	{
		//MDrv_WriteByte( BK_COMB_57, 0x04 );
		//MDrv_WriteByte( BK_COMB_58, 0x01 );
		_MHal_VD_W1B( BK_COMB_57, 0x04 );
		_MHal_VD_W1B( BK_COMB_58, 0x01 );
		_MHal_VD_W1B( BK_COMB_5F, 0x08 );
	}
	else
	{
		if (_bComb10Bit3Flag==1)
		{
			//MDrv_WriteByte( BK_COMB_57, 0x50 );
			//MDrv_WriteByte( BK_COMB_58, 0x20 );
			_MHal_VD_W1B( BK_COMB_57, 0x50 );
			_MHal_VD_W1B( BK_COMB_58, 0x20 );
    		_MHal_VD_W1B( BK_COMB_5F, 0xC8 );
		}
	}
	#endif
}
#endif

/* add for audio fsc checking. */
U16 MHal_VD_GetHsyncCountAndVdStatus2ndCheck(void)
{
       U16 wTemp, wLatchH;
       U8 u8Dummy, u8Value;
       U16 u8VdHsyncLockedCount;
       // printf("\r\n Second Check ");
       u8VdHsyncLockedCount = 0;
       u8Dummy = 10; // just protect program dead lock

       //wLatchH = MDrv_Read2Byte(BK_AFEC_C7); // SPL_NSPL, H total
       wLatchH = (_MHal_VD_R1B(BK_AFEC_C8)<<8)+_MHal_VD_R1B(BK_AFEC_C7); // SPL_NSPL, H total test garbage 1218
       // printf(" Ht=%d", wLatchH);
       while (u8Dummy--)
       {

        //wTemp = MDrv_Read2Byte(BK_AFEC_C7); // SPL_NSPL, H total
        wTemp = (_MHal_VD_R1B(BK_AFEC_C8)<<8)+_MHal_VD_R1B(BK_AFEC_C7);
        wTemp = (wTemp+wLatchH) >> 1;
        wLatchH = wTemp;
        //printf("\r\n Ht = %d", wLatchH);
            for (u8Value = 0; u8Value <= 4; u8Value++)
            {
                if (abs (wLatchH - SPL_NSPL[u8Value]) < 5)//test garbage 1218
                {
                    // putchar('+');
                    u8VdHsyncLockedCount++;
                }
                else
                {
                    // putchar('-');

                }
            }
         // printf("\r\n");
        }
        //    printf ("\r\n u8VdHsyncLockedCount = %u\n", u8VdHsyncLockedCount);
        return u8VdHsyncLockedCount;
}
// VER10_1120 ----->

//	addeded to change the initial value of color kill bound (dreamer@lge.com)
void MHal_VD_InitColorKillBound( U32 type )
{
	if( type == 1 )
	{	// for MINERVA(CAN TUNER)
		_gColorKillBound[0] = COLOR_KILL_HIGH_BOUND_1;
		_gColorKillBound[1] = COLOR_KILL_LOW_BOUND_1;
	}
	else
	{
		_gColorKillBound[0] = COLOR_KILL_HIGH_BOUND;
		_gColorKillBound[1] = COLOR_KILL_LOW_BOUND;
	}

	//printk("COLOR KILL BOUND: %02x %02x\n", _gColorKillBound[0], _gColorKillBound[1] );
}

// Set Analog Color System - Start 090309.
void MHal_VD_SetColorSystem(VD_ANALOG_COLOR_SYSTEM eColorSystem)
{
    U8 u8Value = _MHal_VD_R1B(BK_AFEC_CB);
    //printk("Reg value %02X\n",u8Value);
    switch(eColorSystem)
    {
        case VD_COLOR_NTSC_M:
            //AFEC_CB[0]: FSC_PAL,    1: Disable
            //AFEC_CB[1]: FSC_SECAM,  1: Disable
            //AFEC_CB[2]: FSC_NTSC,   0: Enable
            //AFEC_CB[3]: FSC_NTSC443,1: Disable
            //AFEC_CB[4]: FSC_PAL_M,  1: Disable
            //AFEC_CB[6]: FSC_PAL_Nc, 1: Disable
            u8Value |= BIT0 | BIT1 | BIT3 | BIT4 | BIT6;
            u8Value &= ~BIT2;
            break;
        case VD_COLOR_PAL_M:
            //AFEC_CB[0]: FSC_PAL,    1: Disable
            //AFEC_CB[1]: FSC_SECAM,  1: Disable
            //AFEC_CB[2]: FSC_NTSC,   1: Disable
            //AFEC_CB[3]: FSC_NTSC443,1: Disable
            //AFEC_CB[4]: FSC_PAL_M,  0: Enable
            //AFEC_CB[6]: FSC_PAL_Nc, 1: Disable
            u8Value |= BIT0 | BIT1 | BIT2 | BIT3 | BIT6;
            u8Value &= ~BIT4;
            break;
        case VD_COLOR_PAL_NC:
            //AFEC_CB[0]: FSC_PAL,    1: Disable
            //AFEC_CB[1]: FSC_SECAM,  1: Disable
            //AFEC_CB[2]: FSC_NTSC,   1: Disable
            //AFEC_CB[3]: FSC_NTSC443,1: Disable
            //AFEC_CB[4]: FSC_PAL_M,  1: Disable
            //AFEC_CB[6]: FSC_PAL_Nc, 0: Enable
            u8Value |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4;
            u8Value &= ~BIT6;
            break;
        case VD_COLOR_PAL_N:
            //AFEC_CB[0]: FSC_PAL,    0: Enable
            //AFEC_CB[1]: FSC_SECAM,  1: Disable
            //AFEC_CB[2]: FSC_NTSC,   1: Disable
            //AFEC_CB[3]: FSC_NTSC443,1: Disable
            //AFEC_CB[4]: FSC_PAL_M,  1: Disable
            //AFEC_CB[6]: FSC_PAL_Nc, 1: Disable
            u8Value |= BIT1 | BIT2 | BIT3 | BIT4 | BIT6;
            u8Value &= ~BIT0;
            break;
        case VD_COLOR_MULTI:
        default:
            u8Value &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT6);
            break;
    }

    // Reset dsp when we change color system
    u8Value |= BIT7;

    //printk("color %d, value %02X\n",eColorSystem, u8Value);
    _MHal_VD_W1B(BK_AFEC_CB, u8Value);
    //printk("After AFEC_CB %02X\n",_MHal_VD_R1B(BK_AFEC_CB));

}
// Set Analog Color System - End   090309.

#ifdef AVD_COMB_3D_MID  //Merge from utopia CL:135135 20090721 Ethan
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MHal_VD_COMB_Set3dCombMid()
/// @brief \b Function \b Description: speed up 2D/3D comb switch at OSD menu change
/// @param <IN>        \b bEnable  : TRUE: enable, FALSE: disable.
///
///
/// @param <OUT>       \b None     :
/// @param <RET>       \b None     :
/// @param <GLOBAL>    \b None     :
////////////////////////////////////////////////////////////////////////////////
void MHal_VD_COMB_Set3dCombMid(MS_BOOL bEnable)
{
    if (bEnable)
    {
        _MHal_VD_W1Rb(BK_COMB_EE, ENABLE, _BIT7);
        _MHal_VD_W1Rb(BK_COMB_2D, ENABLE, _BIT4);
    }
    else
    {
        _MHal_VD_W1Rb(BK_COMB_EE, DISABLE, _BIT7);
        _MHal_VD_W1Rb(BK_COMB_2D, DISABLE, _BIT4);
    }
}
#endif


void MHal_VD_VDMCU_SoftStop(void)  //20090826EL
{
    //printk("MHal_VD_VDMCU_SoftStop \r\n");
#ifdef NEW_VD_MCU
    //should halt VDMCU by SW first
    _MHal_VD_W1B(BK_AFEC_C2, 0xA5);
    _MHal_VD_W1B(BK_AFEC_C0, 0xFA);
    msleep(10);
    _MHal_VD_W1B(BK_AFEC_C2, 0x00);
    _MHal_VD_W1B(BK_AFEC_C0, 0x00);
    //MDrv_SYS_VDmcuSetType(VDMCU_DSP_UNKNOWN); // 20091119
    VD_KDBG("><><><><><><><><><><HAL_AVD_VDMCU_SoftStop\r\n");
#endif
}
// 20091118
// 20091020 BY
void MHal_VD_SetClock(BOOL bEnable)
{
    //printk("MHal_VD_SetClock:%u \r\n",bEnable);
    bEnable = (bEnable) ? 0:1; // 0 means enable
    _MHal_VD_W1Rb(H_BK_CLKGEN0(0x20), bEnable, _BIT0);  //reg_ckg_vd(0x0B41) setting off
    if (0 == Chip_Query_Rev())
    {
        _MHal_VD_W1B(L_BK_CLKGEN0(0x21), 0x10);  // 20090628 BY put VD200 clock setting here. TODO, add another function??
    }
    else
    {
        _MHal_VD_W1B(L_BK_CLKGEN0(0x21), 0x0D);  // 20090628 BY put VD200 clock setting here. TODO, add another function??
    }
    _MHal_VD_W1Rb(H_BK_CLKGEN0(0x21), bEnable, _BIT0);  //reg_ckg_vd200(0x0B43) setting off
    _MHal_VD_W1BM (L_BK_CLKGEN0(0x22), 0x00, _BIT0|_BIT1|_BIT2|_BIT3);// CLK_mailbox0
    _MHal_VD_W1BM (H_BK_CLKGEN0(0x22), 0x00, _BIT0|_BIT1|_BIT2|_BIT3);// CLK_mailbox1
    _MHal_VD_W1Rb(L_BK_CLKGEN0(0x23), bEnable, _BIT0);  //reg_ckg_vd2x(0x0B46) setting off
    _MHal_VD_W1Rb(H_BK_CLKGEN0(0x23), bEnable, _BIT0);  //reg_ckg_vd_32fsc(0x0B47) setting off

}

void MHal_VD_T3VIF_WriteByte( U32 u32Reg, U8 u8Val)
{
    U16	    u16WaitCnt = 0;
    //U32 u32Timeout=5000;
    //while (MHal_MAD_ReadByte(DemodCmd) && u32Timeout ) u32Timeout--; // wait VDMCU ready

	//mail box crash protection 2009-11-06
	if (VDMCU_DSP_VIF != MDrv_SYS_VDmcuGetType())
    {
        printk("\n\033[1;31m %s: !!! WARNING VDMCU DSP TYPE IS NOT MATCHED !!! (current type is %d)\033[0m\n\n", __FUNCTION__, MDrv_SYS_VDmcuGetType());
        return;
    }

    if(_MHal_VD_R1B(BYTE2REAL(DemodCmd&0x0FFFFF)))
    {
        VD_KDBG("Mailbox crash (write reg 0x%04x)\n", u32Reg);
        return;
    }

    _MHal_VD_W1B(BYTE2REAL(DemodAdrL&0x0FFFFF),u32Reg&0xFF);
    _MHal_VD_W1B(BYTE2REAL(DemodAdrH&0x0FFFFF),(u32Reg>>8)&0xFF);
    _MHal_VD_W1B(BYTE2REAL(DemodData&0x0FFFFF),u8Val);
    _MHal_VD_W1B(BYTE2REAL(DemodCmd&0x0FFFFF),DemodCmdWrReg);
    //msleep(2);

    while(_MHal_VD_R1B(BYTE2REAL(DemodCmd&0x0FFFFF)) != 0xFF)			// wait done flag
    {
        if (u16WaitCnt++ > 0x7FFF)
        {
            VD_KDBG("VD WriteReg Fail!\n");
            return;
        }
        msleep(1);  //20090819
    }
    _MHal_VD_W1B(BYTE2REAL(DemodCmd&0x0FFFFF), 0x00);					// MB_CNTL clear

}


void MHal_VD_SIF_ADC_Reset(void)//// modify the value for ADC setting 090825
{
    if (0 == Chip_Query_Rev())
    {
    VD_KDBG("=====MHal_VD_SIF_ADC_Reset 0x%x== \r\n",0);

    MHal_VD_T3VIF_WriteByte(0x00346A, 0x04);
    MHal_VD_T3VIF_WriteByte(0x00346B, 0x16);
    MHal_VD_T3VIF_WriteByte(0x00346A, 0x04);
    MHal_VD_T3VIF_WriteByte(0x00346B, 0x06);
    MHal_VD_T3VIF_WriteByte(0x003466, 0x02);
    MHal_VD_T3VIF_WriteByte(0x003467, 0x09);
    MHal_VD_T3VIF_WriteByte(0x003460, 0x00);
    MHal_VD_T3VIF_WriteByte(0x003461, 0x10);
    MHal_VD_T3VIF_WriteByte(0x003402, 0x20);  //
    MHal_VD_T3VIF_WriteByte(0x003403, 0x10);  // not the same with RD side
    MHal_VD_T3VIF_WriteByte(0x00341E, 0x80);
    MHal_VD_T3VIF_WriteByte(0x00341F, 0x00);
    MHal_VD_T3VIF_WriteByte(0x003418, 0x00); // not the same with RD side
    MHal_VD_T3VIF_WriteByte(0x003419, 0x00);
    MHal_VD_T3VIF_WriteByte(0x003440, 0x00);
    MHal_VD_T3VIF_WriteByte(0x003441, 0x00);
    MHal_VD_T3VIF_WriteByte(0x002030, 0x04);
    }
}
//----------------------------------------------------------------------------------------

// shjang_100322 manual H pll speed control
void MHal_VD_ManualHPllTrackingControl( U8 PLLTrackingSpeed)
{
	u8HPllstatus = PLLTrackingSpeed;

	switch(PLLTrackingSpeed)
	{
		case 0x01 :
			_MHal_VD_W1B(BK_AFEC_A0, 0x90);
			_MHal_VD_W1B(BK_AFEC_A1, 0x20);
			break;

		case 0x02 :
			_MHal_VD_W1B(BK_AFEC_A0, 0x9A);
			_MHal_VD_W1B(BK_AFEC_A1, 0x35);
			break;

		case 0x03 :
			_MHal_VD_W1B(BK_AFEC_A0, 0xAE);
			_MHal_VD_W1B(BK_AFEC_A1, 0x5E);
			break;

		case 0x04 :
			_MHal_VD_W1B(BK_AFEC_A0, 0xAE);
			_MHal_VD_W1B(BK_AFEC_A1, 0x6A);
			break;

		case 0x05 :
			_MHal_VD_W1B(BK_AFEC_A0, 0xBC);
			_MHal_VD_W1B(BK_AFEC_A1, 0x6A);
			break;

		case 0x06 :
			_MHal_VD_W1B(BK_AFEC_A0, 0xBF);
			_MHal_VD_W1B(BK_AFEC_A1, 0x7F);
			break;

		default :
			u8HPllstatus = 0;
			_MHal_VD_W1B(BK_AFEC_A0, 0x3C);
			_MHal_VD_W1B(BK_AFEC_A1, 0x6A);
			break;
	}
}

void MHal_VD_SetHsyncWidthDetection(B16 bEnable)
{
	//AFEC_D9[0]: 1 is enable, 0 is disable
	_MHal_VD_W1BM(BK_AFEC_D9, bEnable, BIT0);
	VD_KDBG("\MHal_VD_SetHsyncWidthDetection[D9][0]=%d\n", bEnable);
}


