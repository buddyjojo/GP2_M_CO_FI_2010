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
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/jiffies.h>
#include "mdrv_types.h"
#include "mhal_lpll_reg.h"
#include "mhal_lpll.h"
#include "mhal_scaler_reg.h"
#include "mhal_utility.h"

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
extern U8 bIsphase_df_ok;
extern U8 bIslock_time;
extern U8 bIs_locked;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
#define LPLL_DBG(msg) //msg // LGE drmyung 081106

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void MHal_LPLL_Init(HAL_LPLL_INIT_t* pInit)
{
    U16 u16Reg_03 = 0x0003;

    REG_WR(REG_LPLL(0x00), pInit->u16InputDiv);
    REG_WR(REG_LPLL(0x01), pInit->u16LoopDiv);
    REG_WR(REG_LPLL(0x02), pInit->u16OutputDiv);

    if (pInit->u8Type == 1)
    {
        // RSDS type
        u16Reg_03 |= BIT6;
    }

    if (pInit->u8Mode == 1)
    {
        // dual mode
        u16Reg_03 |= BIT7;
    }

    REG_WR(REG_LPLL(0x03), u16Reg_03);

    // register init //thchen 20080829 for GP LCD settings first
    //REG_WR(REG_LPLL(0x06), 0x8000); // limit_d5d6d7//thchen 20080829
    REG_WR(REG_LPLL(0x06), 0x0000); // Fitch 20081024 Improving frame lock speed // confirm by drmyung
    REG_WR(REG_LPLL(0x07), 0x0001);//thchen 20080925
    REG_WR(REG_LPLL(0x08), 0x0000); // limit_d5d6d7_RK
    REG_WR(REG_LPLL(0x09), 0x0001);
    //REG_WR(REG_LPLL(0x0A), 0x5000); // limit_lpll_offset//thchen 20080925
    REG_WR(REG_LPLL(0x0A), 0xFFF0);//victor 20081210, 3
    //REG_WR(REG_LPLL(0x0D), 0x0701);
    REG_WM(REG_LPLL(0x0D), 0x0701, 0xF7FF);	//lachesis_091017 ssc enable err

    REG_WR(REG_LPLL(0x0B), 0x7600);
    REG_WR(REG_LPLL(0x1D), 0x000B);
    REG_WM(REG_LPLL(0x1C), 0x0000, 0xFF00);
    REG_WM(REG_LPLL(0x1E), 0x0040, 0x00FF);
}

//LGE [vivakjh] 2008/11/12 	Add for DVB PDP Panel
void MHal_LPLL_Init4PDP(HAL_LPLL_INIT_t* pInit)
{
    U16 u16Reg_03 = 0x0003;

    REG_WR(REG_LPLL(0x00), pInit->u16InputDiv);
    REG_WR(REG_LPLL(0x01), pInit->u16LoopDiv);
    REG_WR(REG_LPLL(0x02), pInit->u16OutputDiv);

    if (pInit->u8Type == 1)
    {
        // RSDS type
        u16Reg_03 |= BIT6;
    }

    if (pInit->u8Mode == 1)
    {
        // dual mode
        u16Reg_03 |= BIT7;
    }

    REG_WR(REG_LPLL(0x03), u16Reg_03);

    // register init //thchen 20080829 for GP LCD settings first
    REG_WR(REG_LPLL(0x06), 0x4000); //LGE [vivakjh]  Apply to suitable value for PDP // limit_d5d6d7//thchen 20080829
    REG_WR(REG_LPLL(0x07), 0x0000); //LGE [vivakjh]  Apply to suitable value for PDP //thchen 20080925
    REG_WR(REG_LPLL(0x08), 0x4000); //LGE [vivakjh]  Apply to suitable value for PDP // limit_d5d6d7_RK
    REG_WR(REG_LPLL(0x09), 0x0000); //LGE [vivakjh]  Apply to suitable value for PDP
    REG_WR(REG_LPLL(0x0A), 0x1000); // Fitch 20081024 Improving frame lock speed // confirm by drmyung
    //REG_WR(REG_LPLL(0x0D), 0x0701);
    REG_WM(REG_LPLL(0x0D), 0x0701, 0xF7FF);	//lachesis_091017 ssc enable err

    REG_WR(REG_LPLL(0x0B), 0x0B00); //LGE [vivakjh]  Apply to Startrek value
    REG_WR(REG_LPLL(0x1D), 0x000A); //LGE [vivakjh]  Apply to Startrek value
    REG_WM(REG_LPLL(0x1C), 0x0000, 0xFF00);
    REG_WM(REG_LPLL(0x1E), 0x0040, 0x00FF);
}

void MHal_LPLL_Init4NonFRC(HAL_LPLL_INIT_t* pInit)
{
    U16 u16Reg_03 = 0x0003;

    REG_WR(REG_LPLL(0x00), pInit->u16InputDiv);
    REG_WR(REG_LPLL(0x01), pInit->u16LoopDiv);
    REG_WR(REG_LPLL(0x02), pInit->u16OutputDiv);

    if (pInit->u8Type == 1)
    {
        // RSDS type
        u16Reg_03 |= BIT6;
    }

    if (pInit->u8Mode == 1)
    {
        // dual mode
        u16Reg_03 |= BIT7;
    }

    REG_WR(REG_LPLL(0x03), u16Reg_03);
    REG_W4(REG_LPLL(0x06), pInit->u32LimitD5d6d7); //thchen 20081216
    REG_WR(REG_LPLL(0x08), 0x0000); // limit_d5d6d7_RK
    REG_WR(REG_LPLL(0x09), 0x0001);
    REG_WR(REG_LPLL(0x0A), pInit->u16LimitOffset);  //thchen 20081216
    //REG_WR(REG_LPLL(0x0D), 0x0701);
    REG_WM(REG_LPLL(0x0D), 0x0701, 0xF7FF);	//lachesis_091017 ssc enable err
    REG_WR(REG_LPLL(0x0B), 0x7600);
    REG_WR(REG_LPLL(0x1D), 0x000B);
    REG_WM(REG_LPLL(0x1C), 0x0000, 0xFF00);
    REG_WM(REG_LPLL(0x1E), 0x0040, 0x00FF);
}

void MHal_LPLL_LPLLSET(U32 u32LPLLSET)
{
    //printk("\n MHal_LPLL_LPLLSET ==> %d\n", u32LPLLSET);
    REG_WR(REG_LPLL(0x0F), (U16)(u32LPLLSET & 0xFFFF));
    REG_WR(REG_LPLL(0x10), (U16)(u32LPLLSET>>16));
}

//thchen 20090118 temp solution for ATSC frame lock
void MHal_LPLL_LimitD5d6d7(U32 u32LimitD5d6d7)
{
    //printk("\n MHal_LPLL_LPLLSET ==> %d\n", u32LPLLSET);
    REG_W4(REG_LPLL(0x06), u32LimitD5d6d7); //thchen 20081216
}

//victor 20081210
BOOL MHal_LPLL_IsFRC(void)
{
    if(REG_RR(REG_LPLL(0x0C)) & 0xF000)
        return TRUE;
    return FALSE;
}

//victor 20081210
U16 MHal_LPLL_GetPhaseDiff(void)
{
    return REG_RR(REG_LPLL(0x11));
}

//victor 20081210
void MHal_LPLL_DisableIGainForLockPhase(BOOL bEnable)
{
    REG_WI(REG_LPLL(0x0C), bEnable, BIT6);
}

//victor 20081210
BOOL MHal_LPLL_IsPhaseLockDone(void)
{
    if(REG_RI(REG_LPLL(0x2A), BIT8) > 0)
        return TRUE;
    return FALSE;
}

//victor 20081210
void MHal_LPLL_EnableFPLL(BOOL bEnable)
{
    LPLL_DBG(printk("\n FRAME is %s\n\n", bEnable?"LOCKED":"UN-LOCKED"));
    REG_WI(REG_LPLL(0x0C), bEnable, BIT3);
}

void MHal_LPLL_SetFrameSync(U8 u8FrcIn, U8 u8FrcOut)
{
    REG_WH(REG_LPLL(0x0C), ((u8FrcOut-1) << 4) | (u8FrcIn-1));
}

// 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain
void MHal_LPLL_GetFrameSync(U8 *u8FrcIn, U8 *u8FrcOut)
{
    U8 u8FrcOutIn;
    u8FrcOutIn = REG_RR(REG_LPLL(0x0C)) >> 8;
    *u8FrcIn = (u8FrcOutIn & 0xF)+1;
    *u8FrcOut = (u8FrcOutIn >> 4)+1;
}

void MHal_LPLL_SetIPGain(U8 u8IGain, U8 u8PGain)
{
    REG_WH(REG_LPLL(0x0B),( (u8PGain<<4)|(u8IGain)));
}


void MHal_LPLL_MODSET(U8 u8LPLL_Type, U8 u8LPLL_Mode)
{
// shjang_090904
    //.u8LPLL_Type = 0,    // 0: LVDS, 1: RSDS, 2: TTL , 3 : TCON
    //.u8LPLL_Mode = 1,    // 0: Single mode, 1: Dual mode
    if (u8LPLL_Type == 3)
		REG_WM(REG_LPLL(0x03), (u8LPLL_Mode << 7) , 0x80);
	else
    	REG_WM(REG_LPLL(0x03), ((u8LPLL_Mode << 7) | (u8LPLL_Type << 6)), 0xC0);
}


// LGE [vivakjh]  2008/12/11 Merge!! FitchHsu 20081209 implement frame lock status report
BOOL MHal_LPLL_GetFrameLock_Status(void)
{
    return  (REG_RI(REG_LPLL(0x2A), BIT8) >> 8);
}

// LGE [vivakjh] 2009/01/21	Modified the PDP module flicker problem after playing some perticular movie files that are over the PDP module margin.
void MHal_LPLL_SetFrameLockSpeedToZero(BOOL bIsSpeedZero)
{
    if (bIsSpeedZero)
        REG_WR(REG_LPLL(0x0A), 0x0000);
    else
        REG_WR(REG_LPLL(0x0A), 0x1000);      // PDP Init Value 0x1000
}

// FitchHsu 20090604 SpeedUp frame lock for TW model in YPbPr and HDMI
U32 MHal_LPLL_GetIvsPrd(void)
{
    U16 u16ivs_l, u16ivs_h;
    U32 u32ivs;
    u16ivs_l = REG_RR(REG_LPLL(0x21));
    u16ivs_h = REG_RR(REG_LPLL(0x22));
    u32ivs = (u16ivs_h << 16) | (u16ivs_l);
    return u32ivs;
}
// FitchHsu 20090604 SpeedUp frame lock for TW model in YPbPr and HDMI
U32 MHal_LPLL_GetOvsPrd(void)
{
    U16 u16ovs_l, u16ovs_h;
    U32 u32ovs;
    u16ovs_l = REG_RR(REG_LPLL(0x23));
    u16ovs_h = REG_RR(REG_LPLL(0x24));
    u32ovs = (u16ovs_h << 16) | (u16ovs_l);
    return u32ovs;
}

void MHal_LPLL_Measure_FrameLockSpeed(void)
{
#if 1
    U32 u32Timer;
    U16 u16temp = 0;
    u32Timer = jiffies;
    while( !(REG_RR(REG_LPLL(0x2A)) & BIT8) &&  u16temp<3000 )
    {
        u16temp++;
        mdelay(1);
    }
    //printk("[FPLL Speed]Lock time= %u, timeout=%u\n",
    //      (unsigned int)(jiffies - u32Timer),
    //      (u16temp==3000));
#endif

#if 0
    for(u16temp=0; u16temp<20; u16temp++)
    {
        printk("\n%d\n", u16temp);
        printk("[Freeze]Phase dif= (%u)\n", REG_RR(REG_LPLL(0x11)) );
        printk("[Freeze]Phase up= (%u)\n", REG_R1(REG_LPLL(0x12)) );
        printk("[Freeze]LPLLSET using= (0x%x)\n", REG_RR(REG_LPLL(0x28))|(REG_RR(REG_LPLL(0x29))<<16));
    }
#endif
}

