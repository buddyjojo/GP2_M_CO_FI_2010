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
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include "mdrv_types.h"

#include "mst_platform.h"
#include "mdrv_system.h"

#include "mst_utility.h"
#include "mhal_scaler_reg.h"
#include "mhal_scaler.h"
#include "mhal_utility.h"
#include "mhal_adc_reg.h"
#include "mhal_lpll.h"
#include "mhal_lpll_reg.h"
#include "mdrv_scaler.h"
#include "mdrv_qmap.h"
#include "mhal_qmap.h"

#define RIU_BASE    0xBF000000

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures Start
//-------------------------------------------------------------------------------------------------
extern unsigned long irq_flags_menuload;
extern spinlock_t menuload_lock;
extern unsigned long irq_flags_swbk;
extern spinlock_t switch_bk_lock;
extern U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
#define H_PreScalingDownRatio(Input, Output)        ((((U32)Output) * 1048576ul) / (Input) + 1)     //victor 20081016, scaling ratio
#define H_PreScalingDownRatioAdv(Input, Output)     ((((U32)(Input) -1) * 1048576ul) / (Output-1))  //victor 20081016, scaling ratio
#define V_PreScalingDownRatio(Input, Output)        ((((U32)Output) * 1048576ul) / (Input) + 1)     //victor 20081016, scaling ratio

#define H_PostScalingRatio(Input, Output)           (((U32)Input) * 1048576ul / (Output))           //victor 20081016, scaling ratio
#define V_PostScalingRatio(Input, Output)           (((U32)(Input)-1) * 1048576ul / (Output-1) + 1) //victor 20081016, scaling ratio

static BOOL bIsInputSourceEnable = FALSE;
BOOL  bIsFastFrameLock = FALSE;

static BOOL busemenuload_sc = FALSE;

//[091223_Leo]
static BOOL bIsLowAvgLuma = FALSE;
#define AVG_LUMA_TH     100
//--------------------------------------------------------------------------------------------------
//  Macro definition
//--------------------------------------------------------------------------------------------------
//by hwangbos 20080630
#define OPT_SC_MHAL_DBG    0

#if (OPT_SC_MHAL_DBG)
#define SC_PRINT(fmt, args...)      printk("[Mhal_SC][%05d] " fmt, __LINE__, ## args)
#else
#define SC_PRINT(fmt, args...)
#endif

#define assert(p)   do {\
	if (!(p)) {\
		printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
				__FILE__, __LINE__, #p);\
	}\
} while (0)

//-------------------------------------------------------------------------------------------------
//  Definition
//-------------------------------------------------------------------------------------------------
#define SC_H_PERIOD_MASK    0x3FFF //ykkim5 091205 invalid issue
#define SC_V_TOTAL_MASK     0x07FF




//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

// Free Run / No Signal Color (Table)
U8 SC_FreeRunColor_Table[][3] =
{
	// R, G, B
	{ 0x00, 0x00, 0x00 },   // FREE_RUN_COLOR_BLACK
	{ 0xFF, 0xFF, 0xFF },   // FREE_RUN_COLOR_WHITE
	{ 0x00, 0x00, 0xFF },   // FREE_RUN_COLOR_BLUE
	{ 0xFF, 0x00, 0x00 },   // FREE_RUN_COLOR_RED
	{ 0x00, 0xFF, 0x00 },   // FREE_RUN_COLOR_GREEN
	{ 0x7F, 0x7F, 0x7F },   // FREE_RUN_COLOR_GREY
};

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  IP Mux
//------------------------------------------------------------------------------
void MHal_SC_IPMuxSet(U8 u8DataMux, U8 u8ClkMux)
{
    // 20091026 Daniel.Huang: switch idclk2 clock to XTAL to clear ip status

	REG_WL(REG_IPMUX(0x01), u8DataMux << 4);

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
    REG_WL(REG_SC_GOPINT(0x02), SC_RST_IP_F2);
    SC_BK_RESTORE;

    OS_Delayms(1);
    REG_WL(REG_CKGEN0(0x55), 0xF << 2);         // idclk2: set to XTAL
    REG_WL(REG_CKGEN0(0x55), u8ClkMux << 2);    // idclk2: set to input src clock
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
    REG_WL(REG_SC_GOPINT(0x02), 0x00);
    SC_BK_RESTORE;
}

//------------------------------------------------------------------------------
//  Init
//------------------------------------------------------------------------------
void MHal_SC_Reset(HAL_SC_REST_t reset)
{
	SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
	REG_WL(REG_SC_GOPINT(0x02), reset);
	SC_BK_RESTORE;

	OS_Delayms(1);

	SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
	REG_WL(REG_SC_GOPINT(0x02), 0x00);
	SC_BK_RESTORE;
}

void MHal_SC_RegInit(U32 u32MemAddr, U32 u32MemSize)
{


    SC_BK_STORE;
	// DNR memory base address
	SC_BK_SWICH(REG_SC_BK_SCMI);
	REG_W3(REG_SC_SCMI(0x08), (u32MemAddr / BYTE_PER_WORD));
	REG_W3(REG_SC_SCMI(0x0A), ((u32MemAddr+u32MemSize/2) / BYTE_PER_WORD));     //2009/09/22 fix dnr base1 error
	REG_WR(REG_SC_SCMI(0x0E), 0x0400);
	REG_WR(REG_SC_SCMI(0x0F), 0x02D0);
	SC_BK_RESTORE;

	/* 이하 내용이 없을 경우엔 MFC I/F 안됨 */
	REG_WR(0x3200, 0x0); // 080808 LGE drmyung wih cc.chen

	/*
	// OPM memory base address
	// undocumented registers (removed)
	// No sub-window now (no any _f1 registers)
	SC_BK_SWICH(REG_SC_BK_OPM);
	REG_WR(REG_SC_OPM(0x12), u32MemAddr);
	REG_WR(REG_SC_OPM(0x13), u32MemAddr >> 16);
	REG_WR(REG_SC_OPM(0x14), (u32MemAddr + u32MemSize) / 2);
	REG_WR(REG_SC_OPM(0x15), ((u32MemAddr + u32MemSize) / 2) >> 16);
	REG_WR(REG_SC_OPM(0x16), 0x0400);
	REG_WR(REG_SC_OPM(0x17), 0x02D0);
	*/
	// cc.chen - T.B.D.
	// color matrix
	// .....

	// cc.chen - T.B.D. - Work around for MLink issue: field detection error
	//SC_BK_SWICH(REG_SC_BK_IP1F2);
	//REG_WM(REG_SC_IP1F2(0x21), 0x0001, 0x0003);


}

//------------------------------------------------------------------------------
//  GOPINIT
//------------------------------------------------------------------------------
void MHal_SC_GOPINT_SetGOPEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_GOPINT);
	REG_WM(REG_SC_GOPINT(0x06), 0xE000, 0xE000);
	SC_BK_RESTORE;
}
void MHal_SC_SetGOPSEL(u8 u8IPSelGOP)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_GOPINT);

	switch(u8IPSelGOP)
	{
		case MS_IP0_SEL_GOP0:
			REG_WL(REG_SC_GOPINT(0x05), 0x90);
			REG_WH(REG_SC_GOPINT(0x05), 0x10); //temp solution
			break;
		case MS_IP1_SEL_GOP1:
			REG_WH(REG_SC_GOPINT(0x05), 0xA0);
			break;
		case MS_IP0_SEL_GOP1:
			REG_WL(REG_SC_GOPINT(0x05), 0xA0);
			REG_WH(REG_SC_GOPINT(0x05), 0x20); //temp solution
			break;
		case MS_NIP_SEL_GOP0:
			REG_WL(REG_SC_GOPINT(0x05), 0x00);
			break;
		case MS_NIP_SEL_GOP1:
			REG_WH(REG_SC_GOPINT(0x05), 0x00);
			break;
	}
	SC_BK_RESTORE;
}

//20091020 daniel.huang: fix gop test pattern cannot cover all video problem
static U16 u16SC_BK00_05, u16SC_BK00_06, u16SC_BK10_46;
void MHal_SC_SaveGOPSetting(void)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
    u16SC_BK00_05 = REG_RR(REG_SC_GOPINT(0x05));
    u16SC_BK00_06 = REG_RR(REG_SC_GOPINT(0x06));

    SC_BK_SWICH(REG_SC_BK_VOP);
    u16SC_BK10_46 = REG_RR(REG_SC_VOP(0x46)); //Save VOP "New Blending"
    SC_BK_RESTORE;
}

void MHal_SC_RestoreGOPSetting(void)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
    REG_WR(REG_SC_GOPINT(0x05), u16SC_BK00_05);
    REG_WR(REG_SC_GOPINT(0x06), u16SC_BK00_06);

    SC_BK_SWICH(REG_SC_BK_VOP);
    REG_WR(REG_SC_VOP(0x46), u16SC_BK10_46); //Restore VOP "New Blending"
    SC_BK_RESTORE;
}

void MHal_SC_GOPINT_SetGOP_TO_IP(u8 gop, u8 channel, BOOL enable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_GOPINT);

	if(enable == FALSE)
	{
		REG_WI(REG_SC_GOPINT(0x05), FALSE, BIT5);
	}
	else
	{
		REG_WI(REG_SC_GOPINT(0x05), TRUE, BIT5);

		REG_WM(REG_SC_GOPINT(0x05), gop << 12, 0x3000);
		REG_WM(REG_SC_GOPINT(0x05), channel << 6, 0xC0);
	}

	SC_BK_RESTORE;
}

void MHal_SC_GOPINT_SetGOP_TO_VOP(u8 gop, BOOL enable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_GOPINT);

    switch(gop)
    {
    case 0:
        REG_WI(REG_SC_GOPINT(0x06), enable, BIT14);
        break;
    case 1:
        REG_WI(REG_SC_GOPINT(0x06), enable, BIT15);
        break;
    case 2:
        REG_WI(REG_SC_GOPINT(0x06), enable, BIT13);
        break;
    case 4:
        if(enable)
        {
            REG_WM(REG_SC_GOPINT(0x06), (BIT15 | BIT14 | BIT13), (BIT15 | BIT14 | BIT13));
        }
        else
        {
            REG_WM(REG_SC_GOPINT(0x06), 0x00, (BIT15 | BIT14 | BIT13));
        }
        break;
    default:

        break;
    }
	SC_BK_RESTORE;
}
//FitchHsu 20081125 JPEG issue for rotate
void MHal_SC_EnableInterrupt(HAL_SC_INT_e u8IntSrc, BOOL bEnable)
{
	U16 u16ByteMask;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_GOPINT);
	u16ByteMask = (1 << (u8IntSrc));
	// clear interrupt
	REG_WI(REG_SC_GOPINT(0x12), bEnable, u16ByteMask);
	SC_BK_RESTORE;
}

//thchen 20080820
U32 MHal_SC_GetInterruptSts(void)
{
	U32 u32Sts;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_GOPINT);
	u32Sts = REG_RR(REG_SC_GOPINT(0x10)) | ((U32)REG_RR(REG_SC_GOPINT(0x11)) << 16);
	SC_BK_RESTORE;
	return u32Sts;
}

//[090601_Leo]
void MHal_SC_SetColorAdaptiveRange(U8 u8CbUpValue, U8 u8CbDownValue, U8 u8CrUpValue, U8 u8CrDownValue)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);

	REG_WL(REG_SC_DLC(0x70), u8CbUpValue);
	REG_WL(REG_SC_DLC(0x71), u8CbDownValue);
	REG_WH(REG_SC_DLC(0x70), u8CrUpValue);
	REG_WH(REG_SC_DLC(0x71), u8CrDownValue);

	SC_BK_RESTORE;
}

//[090921_Leo]
void MHal_SC_SetAdaptiveCGainEnable(U8 u8OnOff)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_ACE2);
    REG_WM(REG_SC_ACE2(0x40), u8OnOff, BIT0); //c gain adap luma(y) en
    SC_BK_RESTORE;
}

//[090814_Leo]
void MHal_SC_SetAdaptiveCGain(U8 *pCGainParam)
{
    SC_BK_STORE;
  #if 0//LG request to remove, [090921_Leo]
    SC_BK_SWICH(REG_SC_BK_DLC);
    REG_WH(REG_SC_DLC(0x14), *pCGainParam & 0xFF); //main c gain
    SC_BK_SWICH(REG_SC_BK_ACE2);
    REG_WM(REG_SC_ACE2(0x40), *(pCGainParam + 1), BIT0); //c gain adap luma(y) en
  #else
    SC_BK_SWICH(REG_SC_BK_ACE2);
  #endif
    REG_WL(REG_SC_ACE2(0x41), *(pCGainParam)); //c gain adap y low threshold
    REG_WM(REG_SC_ACE2(0x40), (U8)(*(pCGainParam + 1)<<4), 0x30); //c gain adap y step
    REG_WL(REG_SC_ACE2(0x42), *(pCGainParam + 2)); //c gain in LUT0
    REG_WH(REG_SC_ACE2(0x42), *(pCGainParam + 3)); //...LUT1
    REG_WL(REG_SC_ACE2(0x43), *(pCGainParam + 4)); //...LUT2
    REG_WH(REG_SC_ACE2(0x43), *(pCGainParam + 5)); //...LUT3
    REG_WL(REG_SC_ACE2(0x44), *(pCGainParam + 6)); //...LUT4
    REG_WH(REG_SC_ACE2(0x44), *(pCGainParam + 7)); //...LUT5
    REG_WL(REG_SC_ACE2(0x45), *(pCGainParam + 8)); //...LUT6
    REG_WH(REG_SC_ACE2(0x45), *(pCGainParam + 9)); //...LUT7

    //SC_BK_SWICH(REG_SC_BK_DLC);
    SC_BK_RESTORE;
}

//[090825_Leo]
void MHal_SC_SetPieceWiseEnable(U8 u8OnOff, U8 *pPieceWiseXPosition)
{
    U8 i;
    U8 u8Addr=0x20;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_OP1PIPEXT);
    REG_WM(REG_SC_OP1PIPEXT(0x10), u8OnOff, BIT0);
    for(i=0;i<16;i++)
    {
        if((i%2) == 0)
        {
            REG_WL(REG_SC_OP1PIPEXT(u8Addr), *(pPieceWiseXPosition + i));
        }
        else
        {
            REG_WH(REG_SC_OP1PIPEXT(u8Addr), *(pPieceWiseXPosition + i));
            u8Addr++;
        }
    }

    //for T3 curve fit n0/16 initial setting, [090909_Leo]
    SC_BK_SWICH(REG_SC_BK_DLC);
    if(u8OnOff)
    {
        REG_WM(REG_SC_DLC(0x76), 0x0, 0x1FF);
        REG_WM(REG_SC_DLC(0x77), 0xFF/*0x100*/, 0x1FF);//change to 0xFF first, need more check, [090924_Leo]
    }
    else
    {
        REG_WM(REG_SC_DLC(0x76), 0x108, 0x1FF);
        REG_WM(REG_SC_DLC(0x77), 0x108, 0x1FF);
    }
    SC_BK_RESTORE;
}

//-------------------------------------------------------------------------------------------------
//  IP1F2
//-------------------------------------------------------------------------------------------------
void MHal_SC_SetSourceMux(HAL_SC_SRCMUX_e mux)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WM(REG_SC_IP1F2(0x02), mux, 0x07);
	SC_BK_RESTORE;
}

void MHal_SC_SetInputSyncType(HAL_SC_ISYNC_e isync)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	if (isync == SC_ISYNC_SOG)
	{
		isync |= BIT4;
	}
	else
	{
		isync &= ~(BIT4);
	}
	REG_WM(REG_SC_IP1F2(0x02), isync, 0x70);
	SC_BK_RESTORE;
}

void MHal_SC_SetVideoPortSelect(HAL_SC_VPSELECT_e vpselect)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WM(REG_SC_IP1F2(0x02), vpselect << 8, 0x2300);
	SC_BK_RESTORE;
}

void MHal_SC_InitForVGA(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WH(REG_SC_IP1F2(0x02), 0x00);
	REG_WL(REG_SC_IP1F2(0x03), (BIT6|BIT3));
	REG_WL(REG_SC_IP1F2(0x24), 0x01); // enable ADC coast
	REG_WH(REG_SC_IP1F2(0x25), 0x00); // reserved
	REG_WL(REG_SC_IP1F2(0x26), 0x00); // disable DE glitch removal function & more tolerance for DE
	REG_WH(REG_SC_IP1F2(0x26), 0x00); // reserved
	SC_BK_RESTORE;
}

void MHal_SC_InitForYPbPr(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WH(REG_SC_IP1F2(0x02), BIT2);
	REG_WL(REG_SC_IP1F2(0x03), (BIT7|BIT6|BIT5|BIT3));
	REG_WL(REG_SC_IP1F2(0x24), 0x01); // enable ADC coast
	REG_WL(REG_SC_IP1F2(0x26), 0x94);   //victor 20081016, component 480i problem 0x00 => 0x94
	SC_BK_RESTORE;
}

void MHal_SC_InitForHDMI(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WH(REG_SC_IP1F2(0x02), 0x00);
	REG_WL(REG_SC_IP1F2(0x03), (BIT7|BIT6|BIT3|BIT0));
	REG_WL(REG_SC_IP1F2(0x24), 0x00); // enable ADC coast
	REG_WL(REG_SC_IP1F2(0x26), 0xF0); // disable DE glitch removal function & more tolerance for DE
	REG_WL(REG_SC_IP1F2(0x27), 0x20);
	SC_BK_RESTORE;
}

void MHal_SC_InitForVD(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WL(REG_SC_IP1F2(0x03), 0x9B);
	REG_WL(REG_SC_IP1F2(0x26), 0x00);//victor 20090108, add emp video input source
	SC_BK_RESTORE;
}

void MHal_SC_InitForDC(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WL(REG_SC_IP1F2(0x03), 0x8B);
	REG_WI(REG_SC_IP1F2(0x08), FALSE, BIT7);
	REG_WL(REG_SC_IP1F2(0x26), 0x00);//victor 20090108, add emp video input source
	SC_BK_RESTORE;
}

void MHal_SC_BrightnessInit()
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	// FitchHs 20081111 WI register error
	//REG_WI(REG_SC_VOP(0x16), 0x0100, 0x0100);
	REG_WI(REG_SC_VOP(0x16), 0x01, 0x0100);
	SC_BK_RESTORE;
}

void MHal_SC_SetFieldDetect(U8 u8IP1F2_1D, U16 u8IP1F2_21, U8 u8IP1F2_23)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WH(REG_SC_IP1F2(0x1D), u8IP1F2_1D);
	REG_WR(REG_SC_IP1F2(0x21), u8IP1F2_21);
	REG_WH(REG_SC_IP1F2(0x23), u8IP1F2_23);
	SC_BK_RESTORE;
}

void MHal_SC_SetRegeneratedDE(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);

	//REG_WI(REG_SC_IP1F2(0x21), u8IP1F2_21);
	REG_WI(REG_SC_IP1F2(0x21),  bEnable, BIT12);

	SC_BK_RESTORE;
}


void MHal_SC_SetCapWin(U16 u16HStart, U16 u16HSize, U16 u16VStart, U16 u16VSize)
{
    if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x01);
		MHal_SC_ML_WriteData(REG_SC_IP1F2(0x04), u16VStart, 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_IP1F2(0x05), u16HStart, 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_IP1F2(0x06), u16VSize,  0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_IP1F2(0x07), u16HSize,  0xFFFF);
	}
	else
	{
		SC_BK_STORE;
    	SC_BK_SWICH(REG_SC_BK_IP1F2);
    	REG_WR(REG_SC_IP1F2(0x04), u16VStart);
    	REG_WR(REG_SC_IP1F2(0x05), u16HStart);
    	REG_WR(REG_SC_IP1F2(0x06), u16VSize);
    	REG_WR(REG_SC_IP1F2(0x07), u16HSize);
    	SC_BK_RESTORE;
	}
}

//20091020 daniel.huang: fix gop test pattern cannot cover all video problem
void MHal_SC_GetCapWin(U16 *u16HStart, U16 *u16HSize, U16 *u16VStart, U16 *u16VSize)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    *u16VStart = REG_RR(REG_SC_IP1F2(0x04));
    *u16HStart = REG_RR(REG_SC_IP1F2(0x05));
    *u16VSize  = REG_RR(REG_SC_IP1F2(0x06));
    *u16HSize  = REG_RR(REG_SC_IP1F2(0x07));
    SC_BK_RESTORE;
}

#define SC_DEBUG(msg) //msg // LGE drmyung 081024

//thchen 20081001  // LGE drmyung 081024
void MHal_SC_IP1_SetInputSourceEnable(BOOL bEnable, BOOL bDelayOn)
{
	U32 timer_out;
	SC_BK_STORE;
	//	SC_BK_SWICH(REG_SC_BK_S_VOP);

	timer_out = 0x15000;//65535;// LGE drmyung 081024 : ATV 채널 변경 시 과도를 막기 위하여 delay 늘임.
	if(bEnable && bDelayOn) // LGE drmyung 081024 : Video Mute를 푸는 시점에서만 수행되도록 하기 위함...
	{
		SC_BK_SWICH(REG_SC_BK_S_VOP);
		SC_DEBUG(printk("Time\n"));
		while (1)
		{
			if (REG_RI(REG_SC_S_VOP(0x56), BIT15))
			{
				break;
			}
			if(timer_out == 0)
			{
				break;
			}
			timer_out--;
		}
		SC_DEBUG(printk(" 0x%x\n", 0x15000-timer_out));
	}

	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WI(REG_SC_IP1F2(0x02), !bEnable, BIT7);
	SC_BK_RESTORE;
}

void MHal_SC_IP1_CtrlHistoDataReport(BOOL bEnable)
{
	bIsInputSourceEnable = bEnable;
}

U16 MHal_SC_IP1_GetHPeriod(void)
{
	U16 u16HPeriod;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u16HPeriod = (REG_RR(REG_SC_IP1F2(0x20)) & SC_H_PERIOD_MASK);
	SC_BK_RESTORE;

	if (u16HPeriod == SC_H_PERIOD_MASK)
	{
		u16HPeriod = 0;
	}

	return u16HPeriod;
}

U16 MHal_SC_IP1_GetVTotal(void)
{
	U16 u16VTotal;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u16VTotal = (REG_RR(REG_SC_IP1F2(0x1f)) & SC_V_TOTAL_MASK);
	SC_BK_RESTORE;

	if (u16VTotal == SC_V_TOTAL_MASK)
	{
		u16VTotal = 0;
	}

	return u16VTotal;
}

void MHal_SC_IP1_ResetSyncDetect(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WL(REG_SC_IP1F2(0x21), 0x00);
	SC_BK_RESTORE;
}

U16 MHal_SC_IP1_GetDetectSyncStatus(void)
{
	U16 u16SyncStatus;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u16SyncStatus = REG_RR(REG_SC_IP1F2(0x1E));
	SC_BK_RESTORE;
	return u16SyncStatus;
}

void MHal_SC_IP1_SetCoastWin(U8 u8Start, U8 u8End)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WH(REG_SC_IP1F2(0x24), u8Start);    // coast start
	REG_WL(REG_SC_IP1F2(0x25), u8End);      // coast end
	SC_BK_RESTORE;
}

U16 MHal_SC_IP1_GetHorizontalDE(void)
{
	U16 u16HorizontalDE;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u16HorizontalDE = REG_RR(REG_SC_IP1F2(0x15)) -
		REG_RR(REG_SC_IP1F2(0x13)) + 1;
	SC_BK_RESTORE;

	return u16HorizontalDE;
}

U16 MHal_SC_IP1_GetVerticalDE(void)
{
	U16 u16VerticalDE;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u16VerticalDE = REG_RR( REG_SC_IP1F2(0x14 )) -
		REG_RR( REG_SC_IP1F2(0x12 )) + 1;
	SC_BK_RESTORE;
	return u16VerticalDE;
}

U16 MHal_SC_IP1_GetHorizontalDEStart(void)
{
	U16 u16HDE_Start;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u16HDE_Start = REG_RR(REG_SC_IP1F2(0x13));
	SC_BK_RESTORE;

	return u16HDE_Start;
}

U16 MHal_SC_IP1_GetVerticalDEStart(void)
{
	U16 u16VDE_Start;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u16VDE_Start = REG_RR(REG_SC_IP1F2(0x12));
	SC_BK_RESTORE;
	return u16VDE_Start;
}

void MHal_SC_IP1_SetForceInputClkDiv(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WI(REG_SC_IP1F2(0x02), bEnable, BIT15);
	SC_BK_RESTORE;
}

void MHal_SC_IP2_SetCSCDither(BOOL bEnable)//thchen 20080719
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP2F2);
	REG_WI(REG_SC_IP2F2(0x01), bEnable, BIT2);
	SC_BK_RESTORE;
}

void MHal_SC_IP1_EnableAutoGain(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WL(REG_SC_IP1F2(0x0E), 0x11);
	SC_BK_RESTORE;
}

void MHal_SC_IP1_DisableAutoGain(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WL(REG_SC_IP1F2(0x0E), 0x00);
	SC_BK_RESTORE;
}

void MHal_SC_IP1_SetSampleHStart(U16 u16SHS)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WR(REG_SC_IP1F2(0x05), u16SHS);
	SC_BK_RESTORE;
}

void MHal_SC_IP1_SetSampleVStart(U16 u16SVS)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WR(REG_SC_IP1F2(0x04), u16SVS);
	SC_BK_RESTORE;
}

void MHal_SC_IP1_SetYLock(U8 u8YLock)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WM(REG_SC_IP1F2(0x09), u8YLock, 0x0F);
	SC_BK_RESTORE;
}

void MHal_SC_IP1_SetFramLock(BOOL bFramelockEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	REG_WI(REG_SC_IP1F2(0x03), bFramelockEnable, BIT15);
	SC_BK_RESTORE;
}

//-------------------------------------------------------------------------------------------------
//  IP2
//-------------------------------------------------------------------------------------------------
/******************************************************************************/
/// Set IP2 & ACE color space convert enable/disable
/// @param -bEnable \b IN: enable/disable
/******************************************************************************/
void MHal_SC_IP2_SetCSC(BOOL bEnable, BOOL bUseVIPCSC) // 20090828 daniel.huang: add VIP CSC for PCRGB & DVI
{
	SC_BK_STORE;
	if (!bUseVIPCSC)
	{
		SC_BK_SWICH(REG_SC_BK_IP2F2);
		REG_WI(REG_SC_IP1F2(0x40), bEnable, BIT3);

    		SC_BK_SWICH(REG_SC_BK_ACE);
        	REG_WI(REG_SC_ACE(0x6E), FALSE, BIT0);
	}
	else
	{
        	SC_BK_SWICH(REG_SC_BK_IP2F2);
        	REG_WI(REG_SC_IP1F2(0x40), FALSE, BIT3);

        	SC_BK_SWICH(REG_SC_BK_ACE);
        	REG_WI(REG_SC_ACE(0x6E), bEnable, BIT0);
	}
	SC_BK_RESTORE;
}

/******************************************************************************/
/// Get IP2 color space convert enable/disable
/// @return: enable/disable
/******************************************************************************/
BOOL MHal_SC_IP2_GetCSC(void)   // 20091021 daniel.huang: add ipmux test pattern for inner test pattern
{
    BOOL bEnable;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP2F2);
    bEnable = (REG_RR(REG_SC_IP1F2(0x40)) & BIT3) ? TRUE: FALSE;
    SC_BK_RESTORE;
    return bEnable;
}

/******************************************************************************/
/// Get ACE color space convert enable/disable
/// @return: enable/disable
/******************************************************************************/
BOOL MHal_SC_VIP_GetCSC(void)   // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
{
    BOOL bEnable;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_ACE);
    bEnable = (REG_RR(REG_SC_ACE(0x6E)) & BIT0) ? TRUE: FALSE;
    SC_BK_RESTORE;
    return bEnable;
}

/******************************************************************************/
/// Calculate horizontal scaling down ratio
/// @param -u16Source \b IN: input video horizontal size
/// @param -u16Target \b IN: ouput panel horizontal size
/// @return horizontal scaling down ratio
/******************************************************************************/
U32 MHal_SC_IP2_CalHSD(U16 u16Source, U16 u16Target, BOOL bAdvMode)
{
	U32 u32Ret;

	if (u16Source > u16Target)
	{
		if (bAdvMode)
		{
			u32Ret = H_PreScalingDownRatioAdv(u16Source, u16Target);
		}
		else
		{
			u32Ret = H_PreScalingDownRatio(u16Source, u16Target);
		}

		if (u32Ret != 0)
		{
			u32Ret |= 0x80000000;   // H Scaliing Down enable
		}
	}
	else
	{
		u32Ret = 0x00100000;
	}
	if (bAdvMode)
	{
		// Adv mode enable, 6TapY/4TapC filter mode, fac = IN/OUT(format [3.20])
		u32Ret |= 0x40000000;
	}

	return u32Ret;
}

/******************************************************************************/
/// Calculate vertical scaling down ratio
/// @param -u16Source \b IN: input video vertical size
/// @param -u16Target \b IN: ouput panel vertical size
/// @return vertical scaling down ratio
/******************************************************************************/
U32 MHal_SC_IP2_CalVSD(U16 u16Source, U16 u16Target)
{
	U32 u32Ratio;


	if (u16Source > u16Target)
	{
		u32Ratio = V_PreScalingDownRatio(u16Source, u16Target);
		u32Ratio &= 0xFFFFFL;// 20 bits //victor 20081016, scaling ratio

		if (u32Ratio != 0)
		{
			u32Ratio |= 0x80000000; // V Scaling Down enable
		}
	}
	else
	{
		u32Ratio = 0;
	}
	return u32Ratio;
}

/******************************************************************************/
/// Set horizontal scaling down ratio
/// @param -u32Ratio \b IN: Horizontal scaling down ratio
/******************************************************************************/
void MHal_SC_IP2_SetHSDRatio(U32 u32Ratio)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x02);
		MHal_SC_ML_WriteData(REG_SC_IP1F2(0x04), (U16)(u32Ratio), 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_IP1F2(0x05), (U16)(u32Ratio >> 16), 0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_IP2F2);
		REG_W4(REG_SC_IP2F2(0x04), u32Ratio);
		SC_BK_RESTORE;
	}
}

/******************************************************************************/
/// Set vertical scaling down ratio
/// @param -u32Ratio \b IN: Vertical scaling down ratio
/******************************************************************************/
void MHal_SC_IP2_SetVSDRatio(U32 u32Ratio)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x02);
		MHal_SC_ML_WriteData(REG_SC_IP1F2(0x08), (U16)(u32Ratio), 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_IP1F2(0x09), (U16)(u32Ratio >> 16), 0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_IP2F2);
		REG_W4(REG_SC_IP2F2(0x08), u32Ratio);
		SC_BK_RESTORE;
	}
}

//-------------------------------------------------------------------------------------------------
//  Chip Top - cc.chen - T.B.D.
//-------------------------------------------------------------------------------------------------
void MHal_SC_SetClk(void)
{
	REG_WM(REG_CKGEN0(0x52), 0x0C00, 0x3C00);  // fclk  <- select 216MHz daniel.huang //170 MHz (MPLL_DIV_BUF), 20090619 daniel.huang
	REG_WL(REG_CKGEN0(0x53), 0x1C);            // odclk <- XTAL
}

BOOL MHal_SC_GetSplashWindow(void)
{
    BOOL bsplash;
    U8 u8fclk, u8odclk;

    u8fclk = REG_RH(REG_CKGEN0(0x52)) & 0x3C;                   // fclk  <- select 216MHz daniel.huang //170 MHz (MPLL_DIV_BUF), 20090619 daniel.huang
	u8odclk = REG_RL(REG_CKGEN0(0x53)) & 0x3F;                  // odclk <- XTAL

    if((u8fclk == 0x0C) && (u8odclk == 0x1C))
    {
        bsplash = TRUE;
    }
    else
    {
        bsplash = FALSE;
    }
    return bsplash;
}

void MHal_TOP_SetFickF2_ClkSrc(BOOL bPreVdown)
{
	if (bPreVdown)
		REG_WM(REG_CKGEN0(0x51), 0x0000, 0x0C00); // ficlk <- clk_idclk2
	else
		REG_WM(REG_CKGEN0(0x51), 0x0400, 0x0C00); // ficlk <- clk_fclk
}

void MHal_SC_FilmEnable(BOOL bEnable, U8 u8Filmtype)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_FILM);
	// u8Filmtype = 1 is VFreq = 60Hz
	// u8Filmtype = 0 is VFreq = 50Hz
	// LGE [vivakjh]  2008/12/11 	LG에서 Film Mode Setting 하면 15, 14bit 둘 다 세팅됨. ==> 검토 필요.
	// PDP 경우 Fast Frame Lock을 못하므로 Frame Lock시점까지 Film Mode를 Off 했다가 Frame Lock 되면 On해줌.
	if (u8Filmtype==2)
	{
		REG_WI(REG_SC_FILM(0x10), bEnable, BIT15);
		REG_WI(REG_SC_FILM(0x10), bEnable, BIT14);
	}
	else if (u8Filmtype==1)
	{
		REG_WI(REG_SC_FILM(0x10), bEnable, BIT15);
		REG_WI(REG_SC_FILM(0x10), 0, BIT14);
	}
	else
	{
		REG_WI(REG_SC_FILM(0x10), 0, BIT15);
		REG_WI(REG_SC_FILM(0x10), bEnable, BIT14);
	}

	// Set enable bit for Film mode, Victor
	// Victor 081106 : recommend to delete
	//SC_BK_SWICH(REG_SC_BK_SCMI);
	//REG_WI(REG_SC_SCMI(0x07), bEnable, BIT14);
	SC_BK_RESTORE;
}

// LGE [vivakjh]  2008/12/11 	PDP 경우 Fast Frame Lock을 못하므로 Frame Lock시점까지 Film Mode를 Off 했다가 Frame Lock 되면 On해줌.
// This function can read the Film Mode status => 0 : OFF,  1 : 50Hz, 2 : 60Hz, 3 : Both
U8 MHal_SC_GetFilmMode(void)
{
	U16 u16FilmMode = 0;
	U8 u8RetFilmMode = 0;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_FILM);

	u16FilmMode = REG_RI(REG_SC_FILM(0x10), BIT15);
	u16FilmMode |= REG_RI(REG_SC_FILM(0x10), BIT14);
	u8RetFilmMode = (U8)(u16FilmMode >> 14);

	SC_BK_RESTORE;
	return u8RetFilmMode;
}


void MHal_SC_SetDEOnlyMode(BOOL bEnable)
{
	REG_WI(REG_CKGEN0(0x50), bEnable, BIT2);

	{    //	added to remove warning: ISO C90 forbids mixed declarations and code (dreamer@lge.com)
		SC_BK_STORE; // LGE drmyung 081103
		SC_BK_SWICH(REG_SC_BK_IP1F2); // LGE drmyung 081103

		/*
		   if (bEnable)
		   {
		   REG_WL(REG_SC_IP1F2(0x27), 0x20);   // VSync glitch removal with line less than 2(DE only)
		   }
		   else
		   {
		   REG_WL(REG_SC_IP1F2(0x27), 0x00);   // CHANNEL_ATTRIBUTE function control
		   }
		   */
		if (bEnable)

		{
			//FitchHsu 20081213 HDMI YCbCr422 problem
			REG_WL(REG_SC_IP1F2(0x27), 0x24);   // VSync glitch removal with line less than 2(DE only)
		}
		else
		{
			//FitchHsu 20081213 HDMI YCbCr422 problem
			REG_WL(REG_SC_IP1F2(0x27), 0x04);   // CHANNEL_ATTRIBUTE function control
		}

		SC_BK_RESTORE; // LGE drmyung 081103
	}
}

//-------------------------------------------------------------------------------------------------
//  IPM
//-------------------------------------------------------------------------------------------------

void MHal_SC_IPM_SetFBL(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_SCMI);
	REG_WI(REG_SC_SCMI(0x01), bEnable, BIT7);
	if(bEnable)
	{
    	SC_BK_SWICH(REG_SC_BK_DNR);
    	REG_WI(REG_SC_DNR(0x21), 0, BIT0);
    }
	SC_BK_RESTORE;
}

void MHal_SC_IPM_SetMemBaseAddr(U32 u32Base0, U32 u32Base1, U32 u32Base2)
{
    if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x12);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x08), (U16)((u32Base0 / BYTE_PER_WORD) ),       0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x09), (U16)((u32Base0 / BYTE_PER_WORD) >> 16 ), 0x00FF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x0A), (U16)((u32Base1 / BYTE_PER_WORD) ),       0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x0B), (U16)((u32Base1 / BYTE_PER_WORD) >> 16 ), 0x00FF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x0C), (U16)((u32Base2 / BYTE_PER_WORD) ),       0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x0D), (U16)((u32Base2 / BYTE_PER_WORD) >> 16 ), 0x00FF);
	}
	else
	{
		SC_BK_STORE;
    	SC_BK_SWICH(REG_SC_BK_SCMI);
    	REG_W3(REG_SC_SCMI(0x08), (u32Base0 / BYTE_PER_WORD));
    	REG_W3(REG_SC_SCMI(0x0A), (u32Base1 / BYTE_PER_WORD));
    	REG_W3(REG_SC_SCMI(0x0C), (u32Base2 / BYTE_PER_WORD));
    	SC_BK_RESTORE;
	}
}

void MHal_SC_IPM_SetMemFetchOffset(U16 u16Fetch, U16 u16Offset)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x12);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x0E), u16Offset, 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x0F), u16Fetch, 0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_SCMI);
		REG_WR(REG_SC_SCMI(0x0E), u16Offset);
		REG_WR(REG_SC_SCMI(0x0F), u16Fetch);
		SC_BK_RESTORE;
	}
}

void MHal_SC_IPM_SetVLengthWriteLimit(U16 u16VCapSize, BOOL bInterlaced)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_SCMI);

	// set write limit
	if (bInterlaced)
	{
		REG_WR(REG_SC_SCMI(0x18), u16VCapSize/2);
	}
	else
	{
		REG_WR(REG_SC_SCMI(0x18), u16VCapSize);
	}

	// enable write limit
	REG_WI(REG_SC_SCMI(0x18), TRUE, BIT12);

	SC_BK_RESTORE;
}

void MHal_SC_IPM_SetFreezeImg(BOOL bFreeze)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_SCMI);

	if (bFreeze)
	{
		REG_WM(REG_SC_SCMI(0x01), 0x0800, 0x0800);   // enable Freeze
	}
	else
	{
		REG_WM(REG_SC_SCMI(0x01), 0x0000, 0x0800);   // disable Freeze
	}

	SC_BK_RESTORE;
}

/******************************************************************************/
/// Set the DNR write memory limit to avoid writing illigal memory area
/// @param -u32MemAddr \b IN: DNR memory start address
/// @param -u32MemSize \b IN: DNR memory length
/******************************************************************************/
void MHal_SC_IPM_SetDNRWriteLimit(U32 u32WritelimitAddrBase)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_SCMI);
    REG_W4(REG_SC_SCMI(0x1A), u32WritelimitAddrBase);
    SC_BK_RESTORE;
}

//-------------------------------------------------------------------------------------------------
//  OPM
//-------------------------------------------------------------------------------------------------
void MHal_SC_OPM_SetMemBaseAddr(U32 u32Base0, U32 u32Base1, U32 u32Base2, U16 u16CropLBoffset)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x12);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x10), (U16)((u32Base0 / BYTE_PER_WORD) ),       0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x11), (U16)((u32Base0 / BYTE_PER_WORD) >> 16 ), 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x12), (U16)((u32Base1 / BYTE_PER_WORD) ),       0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x13), (U16)((u32Base1 / BYTE_PER_WORD) >> 16 ), 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x14), (U16)((u32Base2 / BYTE_PER_WORD) ),       0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x15), (U16)((u32Base2 / BYTE_PER_WORD) >> 16 ), 0xFFFF);

		MHal_SC_ML_ChangeBank(0x20);
		MHal_SC_ML_WriteData(REG_SC_PIP(0x1D), (U16)u16CropLBoffset, 0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_SCMI);
		REG_W3(REG_SC_SCMI(0x10), (u32Base0 / BYTE_PER_WORD));
		REG_W3(REG_SC_SCMI(0x12), (u32Base1 / BYTE_PER_WORD));
		REG_W3(REG_SC_SCMI(0x14), (u32Base2 / BYTE_PER_WORD));

		SC_BK_SWICH(REG_SC_BK_PIP);
		REG_WM(REG_SC_PIP(0x1D), u16CropLBoffset, 0x0FF);
		SC_BK_RESTORE;
	}
}

void MHal_SC_OPM_SetMemFetchOffset(U16 u16Fetch, U16 u16Offset)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x12);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x16), u16Offset, 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_SCMI(0x17), u16Fetch, 0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_SCMI);
		REG_WR(REG_SC_SCMI(0x16), u16Offset);
		REG_WR(REG_SC_SCMI(0x17), u16Fetch);
		SC_BK_RESTORE;
	}
}
//-------------------------------------------------------------------------------------------------
//  OP1
//-------------------------------------------------------------------------------------------------

U32 MHal_SC_OP1_CalVSP(U16 u16Source, U16 u16Target)
{
	U32 u32Ratio;

	u32Ratio  = V_PostScalingRatio(u16Source, u16Target);

	u32Ratio &= 0xFFFFFF;

	if (u32Ratio != 0)
	{
		u32Ratio |= 0x1000000;
	}

	return u32Ratio;
}

U32 MHal_SC_OP1_CalHSP(U16 u16Source, U16 u16Target)
{
	U32 u32Ratio;

	u32Ratio  = H_PostScalingRatio(u16Source, u16Target);

	u32Ratio &= 0xFFFFFF;

	if (u32Ratio != 0)
	{
		u32Ratio |= 0x1000000;
	}

	return u32Ratio;
}

void MHal_SC_OP1_SetVSP(U32 u32Ratio)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x23);
		MHal_SC_ML_WriteData(REG_SC_HVSP(0x09), (U16)(u32Ratio), 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_HVSP(0x0A), (U16)(u32Ratio >> 16), 0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_HVSP);
		REG_W4(REG_SC_HVSP(0x09), u32Ratio);
		SC_BK_RESTORE;
	}
}

void MHal_SC_OP1_SetHSP(U32 u32Ratio)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x23);
		MHal_SC_ML_WriteData(REG_SC_HVSP(0x07), (U16)(u32Ratio), 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_HVSP(0x08), (U16)(u32Ratio >> 16), 0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_HVSP);
		REG_W4(REG_SC_HVSP(0x07), u32Ratio);
		SC_BK_RESTORE;
	}
}

#if 1// 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
U32 MHal_SC_OP1_GetHSP(void)
{
    U32 u32Ratio;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_HVSP);
    u32Ratio = (U32)REG_RR(REG_SC_HVSP(0x07)) | ((U32)REG_RR(REG_SC_HVSP(0x08)) << 16);
    SC_BK_RESTORE;
    return u32Ratio;
}

U32 MHal_SC_OP1_GetVSP(void)
{
    U32 u32Ratio;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_HVSP);
    u32Ratio = (U32)REG_RR(REG_SC_HVSP(0x09)) | ((U32)REG_RR(REG_SC_HVSP(0x0A)) << 16);
    SC_BK_RESTORE;
    return u32Ratio;
}

//*************************************************************************
//Function name: MHal_SC_OP1_GetHCropCoordiante
//Passing parameter: u16Disp - position or length in display coordinate
//                   bPositionOrLength - TRUE: position, FALSE: length
//Return parameter:
//  U32: point position in crop coordinate
//Description: Get auto phase value.
//*************************************************************************

U16 MHal_SC_OP1_GetHCropCoordiante(U16 u16Disp, BOOL bPositionOrLength)
{
    U32 u32Ratio = MHal_SC_OP1_GetHSP();
    U16 u16Crop;
    BOOL bScalingEnable;

    bScalingEnable = (u32Ratio & 0x1000000) ? 1:0;
    u32Ratio &= 0xFFFFFF;

    if (bScalingEnable == 0 || u16Disp == 0)
    {
        u16Crop = u16Disp;
    }
    else
    {
        // ratio = (((U32)Input) * 1048576ul / (Output))
        if (bPositionOrLength == TRUE)
            u16Crop = (U16)((u32Ratio * u16Disp) / 1048576ul); // floor
        else
            u16Crop = (U16)((u32Ratio * u16Disp + (1048576ul/2)) / 1048576ul); // ceiling

    }
    return u16Crop;
}


//*************************************************************************
//Function name: MHal_SC_OP1_GetVCropCoordiante
//Passing parameter: u16Disp - position or length in display coordinate
//                   bPositionOrLength - TRUE: position, FALSE: length
//Return parameter:
//  U32: point position in crop coordinate
//Description: Get auto phase value.
//*************************************************************************

U16 MHal_SC_OP1_GetVCropCoordiante(U16 u16Disp, BOOL bPositionOrLength)
{
    U32 u32Ratio = MHal_SC_OP1_GetVSP();
    U16 u16Crop;
    BOOL bScalingEnable;

    bScalingEnable = (u32Ratio & 0x1000000) ? 1:0;
    u32Ratio &= 0xFFFFFF;

    if (bScalingEnable == 0 || u16Disp == 0)
    {
        u16Crop = u16Disp;
    }
    else
    {
        //  ratio = (((U32)(Input)-1) * 1048576ul / (Output-1) + 1)
        if (bPositionOrLength)
            u16Crop = (U16)(((u32Ratio-1) * (u16Disp-1)) / 1048576ul + 1);  // floor
        else
            u16Crop = (U16)(((u32Ratio-1) * (u16Disp) + (1048576ul/2)) / 1048576ul);  // ceiling & cancel "u16Disp-1" & "+1" for "height"
    }
    return u16Crop;
}

#endif


void MHal_SC_OP1_SetVLength(U16 u16VLength)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x20);
		MHal_SC_ML_WriteData(REG_SC_PIP(0x15), u16VLength, 0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_PIP);
		REG_WR(REG_SC_PIP(0x15), u16VLength);
		SC_BK_RESTORE;
	}
}

void MHal_SC_EODI_SetUpSampling(U16 u16Sequence, BOOL bEnable420CUP)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x21);
		MHal_SC_ML_WriteData(REG_SC_EODI(0x76), u16Sequence, 0xFFFF);
		if(bEnable420CUP)//420cup controlled by driver, [100222_Leo]
		{
		    MHal_SC_ML_WriteData(REG_SC_EODI(0x77), 0x01, 0x01);
		}
		else
		{
            MHal_SC_ML_WriteData(REG_SC_EODI(0x77), 0x00, 0x01);
		}
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_EODI);
		REG_WR(REG_SC_EODI(0x76), u16Sequence);
		if(bEnable420CUP)//420cup controlled by driver, [100222_Leo]
		{
		    REG_WM(REG_SC_EODI(0x77), 0x01, 0x01);
		}
		else
		{
            REG_WM(REG_SC_EODI(0x77), 0x00, 0x01);
		}
		SC_BK_RESTORE;
	}
}

//-------------------------------------------------------------------------------------------------
//  OP2
//-------------------------------------------------------------------------------------------------
void MHal_SC_OP2_SetOSDBlending(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	if (bEnable)
	{
		REG_WM(REG_SC_VOP(0x23), 0x00A0, 0x00A0);
	}
	else
	{
		REG_WM(REG_SC_VOP(0x23), 0x0000, 0x00A0);
	}
	SC_BK_RESTORE;
}

void MHal_SC_OP2_SetNoSignalColor(U8 u8Color)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WL(REG_SC_VOP(0x24), u8Color);
	SC_BK_RESTORE;
}

void MHal_SC_OP2_SetCMC(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	if (bEnable)
	{
		REG_WL(REG_SC_VOP(0x2F), 0x35);
	}
	else
	{
		REG_WL(REG_SC_VOP(0x2F), 0x00);
	}
	SC_BK_RESTORE;
}

void MHal_SC_OP2_SetColorMatrix(U16* pMatrix)
{
	U32 u32Addr;
	U32 u320x26, u320x2F;
	S16 sTmp;
#if USE_MENULOAD_PQ
	MHal_SC_ML_Start();
	MENULOAD_LOCK;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
    u320x26 = REG_SC_VOP(0x26);
	u320x2F = REG_SC_VOP(0x2F);
	SC_BK_RESTORE;
	MHal_SC_ML_ChangeBank(0x10);
	for (u32Addr = u320x26; u32Addr < u320x2F;  u32Addr += 2)
	{
		sTmp = *pMatrix++;
		if (sTmp >= 0)
		{
			if (sTmp > 0xfff)
			{
				sTmp = 0xfff;
			}
		}
		else
		{
			sTmp = -sTmp;
			if (sTmp > 0xfff)
			{
				sTmp = 0xfff;
			}
			sTmp |= 0x1000;
		}

		MHal_SC_ML_WriteData((U16)u32Addr, sTmp, 0xFFFF);
	}
	MHal_SC_ML_End();
	MENULOAD_UNLOCK;
#else
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
    u320x26 = REG_SC_VOP(0x26);
	u320x2F = REG_SC_VOP(0x2F);
	for (u32Addr = u320x26; u32Addr < u320x2F;  u32Addr += 2)
	{
		sTmp = *pMatrix++;
		if (sTmp >= 0)
		{
			if (sTmp > 0xfff)
			{
				sTmp = 0xfff;
			}
		}
		else
		{
			sTmp = -sTmp;
			if (sTmp > 0xfff)
			{
				sTmp = 0xfff;
			}
			sTmp |= 0x1000;
		}
		REG_WR(/*(U16)*/u32Addr, sTmp);//correct addr setting, [090527_Leo]
	}
	SC_BK_RESTORE;
#endif
}
void MHal_SC_OP2_SetBrightness(U16 u16Brightness)//U8 change to U16, [090921_Leo]
{
#if USE_MENULOAD_PQ
	//victor 20081024, menuload
	//MHal_SC_ML_PreventBufOverflow();
        //printk("@@@@@@  MHal_SC_OP2_SetBrightness\n");
	MHal_SC_ML_Start();
	MENULOAD_LOCK;

	MHal_SC_ML_ChangeBank(0x0F);
	MHal_SC_ML_WriteData(REG_SC_S_VOP(0x36), u16Brightness, 0xFFFF);
	MHal_SC_ML_WriteData(REG_SC_S_VOP(0x37), u16Brightness, 0xFFFF);
	MHal_SC_ML_WriteData(REG_SC_S_VOP(0x38), u16Brightness, 0xFFFF);
	MHal_SC_ML_End();
	MENULOAD_UNLOCK;
#else
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	// Victor TBD.
	REG_WR(REG_SC_S_VOP(0x37), u16Brightness);
	REG_WR(REG_SC_S_VOP(0x36), u16Brightness);
	REG_WR(REG_SC_S_VOP(0x38), u16Brightness);

	SC_BK_RESTORE;
#endif
}

void MHal_SC_OP2_SetBlackScreen(BOOL bEnable)
{
	SC_BK_STORE;

	if(bEnable)
	{
		SC_BK_SWICH(REG_SC_BK_VOP);
		REG_WI(REG_SC_VOP(0x19), bEnable, BIT1);
		//SC_BK_SWICH(REG_SC_BK_MADI); //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
		//REG_WH(REG_SC_MADI(0x08), 0x00); //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
	}
	else
	{
		//SC_BK_SWICH(REG_SC_BK_MADI); //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
		//REG_WH(REG_SC_MADI(0x08), 0x08); //LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
		SC_BK_SWICH(REG_SC_BK_VOP);
		REG_WI(REG_SC_VOP(0x19), bEnable, BIT1);
	}

	SC_BK_RESTORE;
}


void MHal_SC_OP2_SetGammaEnable(BOOL bEnable)
{
	SC_BK_STORE;

	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WI(REG_SC_VOP(0x50), bEnable, BIT0);
	SC_BK_RESTORE;
}

void MHal_SC_OP2_SetGammaMappingMode(U8 u8Mapping)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_VOP);
	if (u8Mapping) // OneToOne mapping
	{
		REG_WM(REG_SC_VOP(0x74), BIT15, BIT15);
	}
	else // 256 level linear interpolation
	{
		REG_WM(REG_SC_VOP(0x74), 0, BIT15);
	}
    SC_BK_RESTORE;
}

// 20090824 daniel.huang: including from and not including To, eg: From=0, To=1024
// and assumem u16From and u16To to be even
void MHal_SC_OP2_SetRGBGammaTable(U8* pu8GammaTableR, U8* pu8GammaTableG,
                                  U8* pu8GammaTableB, U16 u16From, U16 u16To)
{
    U8  u8Delay;
    U16 u16Cnt;// = 0; //[090825_Leo]
    U16 i;
    U16 u16GammaValueR = 0;
    U16 u16GammaValueG = 0;
    U16 u16GammaValueB = 0;



    u16Cnt = u16From; //[090825_Leo]
    for (i = u16From/2*3; u16Cnt < u16To; i += 3)
    {
        // gamma x of R
        u16GammaValueR  = pu8GammaTableR[i] & 0x0F;
        u16GammaValueR |= pu8GammaTableR[i+1] << 4;

        // gamma x of G
        u16GammaValueG  = pu8GammaTableG[i] & 0x0F;
        u16GammaValueG |= pu8GammaTableG[i+1] << 4;

        // gamma x of B
        u16GammaValueB  = pu8GammaTableB[i] & 0x0F;
        u16GammaValueB |= pu8GammaTableB[i+1] << 4;

        // write gamma value
        u8Delay = 0xFF;
        SC_BK_STORE;
        SC_BK_SWICH(REG_SC_BK_VOP);
        while ((REG_RR(REG_SC_VOP(0x6D)) & 0x00E0) && (u8Delay--)); // check write channel ready
        REG_WR(REG_SC_VOP(0x6C), u16Cnt);            // set address port
        REG_WR(REG_SC_VOP(0x6E), u16GammaValueR);    // Set channel R data
        REG_WR(REG_SC_VOP(0x6F), u16GammaValueG);    // Set channel G data
        REG_WR(REG_SC_VOP(0x70), u16GammaValueB);    // Set channel B data
        REG_WL(REG_SC_VOP(0x6D), (BIT5|BIT6|BIT7));  // kick off
        SC_BK_RESTORE;
        u16Cnt++;

        // gamma x+1 of R
        u16GammaValueR  = (pu8GammaTableR[i] & 0xF0) >> 4;
        u16GammaValueR |= pu8GammaTableR[i+2] << 4;

        // gamma x+1 of G
        u16GammaValueG  = (pu8GammaTableG[i] & 0xF0) >> 4;
        u16GammaValueG |= pu8GammaTableG[i+2] << 4;

        // gamma x+1 of B
        u16GammaValueB  = (pu8GammaTableB[i] & 0xF0) >> 4;
        u16GammaValueB |= pu8GammaTableB[i+2] << 4;

        // write gamma value
        u8Delay = 0xFF;
        SC_BK_STORE;
        SC_BK_SWICH(REG_SC_BK_VOP);
        while ((REG_RR(REG_SC_VOP(0x6D)) & 0x00E0) && (u8Delay--)); // check write channel ready
        REG_WR(REG_SC_VOP(0x6C), u16Cnt);            // set address port
        REG_WR(REG_SC_VOP(0x6E), u16GammaValueR);    // Set channel R data
        REG_WR(REG_SC_VOP(0x6F), u16GammaValueG);    // Set channel G data
        REG_WR(REG_SC_VOP(0x70), u16GammaValueB);    // Set channel B data
        REG_WL(REG_SC_VOP(0x6D), (BIT5|BIT6|BIT7));  // kick off
        SC_BK_RESTORE;
        u16Cnt++;
    }

}

// 20091030 daniel.huang: fix 256/1024 gamma incorrect
//get & set gamma max value, [090918_Leo]
void MHal_SC_OP2_SetRGBGammaMaxValue(U8* pu8GammaTableR, U8* pu8GammaTableG,
                                  U8* pu8GammaTableB, U16 u16MaxCnt)
{
    U16 u16MaxGammaValueR = 0;
    U16 u16MaxGammaValueG = 0;
    U16 u16MaxGammaValueB = 0;
    U16 u16TableSize = u16MaxCnt*3/2;
    u16MaxGammaValueR  = (pu8GammaTableR[u16TableSize-3] & 0xF0) >> 4;
    u16MaxGammaValueR |= pu8GammaTableR[u16TableSize-1] << 4;
    u16MaxGammaValueG  = (pu8GammaTableG[u16TableSize-3] & 0xF0) >> 4;
    u16MaxGammaValueG |= pu8GammaTableG[u16TableSize-1] << 4;
    u16MaxGammaValueB  = (pu8GammaTableB[u16TableSize-3] & 0xF0) >> 4;
    u16MaxGammaValueB |= pu8GammaTableB[u16TableSize-1] << 4;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_VOP);
    REG_WR(REG_SC_VOP(0x7A), u16MaxGammaValueR);    // max. base 0 of R
    REG_WR(REG_SC_VOP(0x7B), u16MaxGammaValueR);    // max. base 1 of R
    REG_WR(REG_SC_VOP(0x7C), u16MaxGammaValueG);    // max. base 0 of G
    REG_WR(REG_SC_VOP(0x7D), u16MaxGammaValueG);    // max. base 1 of G
    REG_WR(REG_SC_VOP(0x7E), u16MaxGammaValueB);    // max. base 0 of B
    REG_WR(REG_SC_VOP(0x7F), u16MaxGammaValueB);    // max. base 1 of B

    SC_BK_RESTORE;
}


void MHal_SC_OP2_SetFrameColor(U8 u8FrameColorR, U8 u8FrameColorG,U8 u8FrameColorB)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WI(REG_SC_VOP(0x19), TRUE, BIT0);
	REG_WM(REG_SC_VOP(0x19), ((U16)u8FrameColorR)<<8, 0xFF00);
	REG_WM(REG_SC_VOP(0x1A), ((U16)u8FrameColorG), 0x00FF);
	REG_WM(REG_SC_VOP(0x1A), ((U16)u8FrameColorB)<<8, 0xFF00);
	SC_BK_RESTORE;
}


//-------------------------------------------------------------------------------------------------
//  VOP
//-------------------------------------------------------------------------------------------------
void MHal_SC_VOP_HVTotalSet(U16 u16HTotal, U16 u16VTotal)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WR(REG_SC_VOP(0x0C), u16HTotal & 0xffff);
	REG_WR(REG_SC_VOP(0x0D), u16VTotal & 0xffff);
	SC_BK_RESTORE;
}

void MHal_SC_VOP_HSyncWidthSet(U8 u8HSyncWidth)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WL(REG_SC_VOP(0x01), u8HSyncWidth & 0xfffe);
	SC_BK_RESTORE;
}

void MHal_SC_VOP_SetDEWin(U16 u16HStart, U16 u16HEnd, U16 u16VStart, U16 u16VEnd)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WR(REG_SC_VOP(0x04), u16HStart);
	REG_WR(REG_SC_VOP(0x05), u16HEnd);
	//    REG_WR(REG_SC_VOP(0x06), u16VStart +2);  //thchen 20081001
	//    REG_WR(REG_SC_VOP(0x07), u16VEnd +2);  //thchen 20081001
	REG_WR(REG_SC_VOP(0x06), u16VStart); // LGE drmyung 20081008
	REG_WR(REG_SC_VOP(0x07), u16VEnd); // LGE drmyung 20081008
	SC_BK_RESTORE;
}

void MHal_SC_VOP_SetDispWin(U16 u16HStart, U16 u16HEnd, U16 u16VStart, U16 u16VEnd)
{
	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x10);
		MHal_SC_ML_WriteData(REG_SC_VOP(0x08), u16HStart, 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_VOP(0x09), u16HEnd,   0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_VOP(0x0A), u16VStart, 0xFFFF);
		MHal_SC_ML_WriteData(REG_SC_VOP(0x0B), u16VEnd,   0xFFFF);
	}
	else
	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_VOP);
		REG_WR(REG_SC_VOP(0x08), u16HStart);
		REG_WR(REG_SC_VOP(0x09), u16HEnd);
		REG_WR(REG_SC_VOP(0x0A), u16VStart);
		REG_WR(REG_SC_VOP(0x0B), u16VEnd);
		SC_BK_RESTORE;
	}
}

void MHal_SC_VOP_OutputCtrl(U16 u16OCTRL, U16 u16OSTRL, U16 u16ODRV, U16 u16DITHCTRL)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WR(REG_SC_VOP(0x21), u16OCTRL);
	REG_WR(REG_SC_VOP(0x46), u16OSTRL);
	REG_WL(REG_SC_VOP(0x47), u16ODRV);
	REG_WR(REG_SC_VOP(0x1B), u16DITHCTRL);
	SC_BK_RESTORE;
}

void MHal_SC_VOP_EnableClock(HAL_SC_ENCLK_e enclk)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WM(REG_SC_VOP(0x22), enclk, 0x1F);
	SC_BK_RESTORE;
}

void MHal_SC_VOP_SetOutputSycCtrl(HAL_SC_OUTPUTSYNC_MODE_e mode, U16 u16VSyncStart, U16 u16VSyncEnd)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);

	if (mode == SC_OUTPUTSYNC_MODE_0)
	{
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT14);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT10);
	}
	else if (mode == SC_OUTPUTSYNC_MODE_1)
	{
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT14);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT10);
	}
	else if (mode == SC_OUTPUTSYNC_MODE_2)
	{
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT14);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT10);
	}
	// [vivakjh] 2009/01/07	PDP Sync Ctrl 을 위해 추가함.  ==> PC : SC_OUTPUTSYNC_MODE_4, Others : SC_OUTPUTSYNC_MODE_3
	else if (mode == SC_OUTPUTSYNC_MODE_3)
	{
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT14);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT13);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT12);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT11);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT10);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT9);
	}
	else if (mode == SC_OUTPUTSYNC_MODE_4)
	{
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT14);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT13);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT12);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT11);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT10);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT9);
	}
	else if (mode == SC_OUTPUTSYNC_MODE_5)	// PDP Booting 조건 추가함.
	{
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT15);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT14);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT13);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT12);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT11);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT10);
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT9);
	}
	else
	{
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT14);
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT10);
	}

	REG_WR(REG_SC_VOP(0x02), u16VSyncStart); // vsync start position
	REG_WR(REG_SC_VOP(0x03), u16VSyncEnd);   // vsync end position
	SC_BK_RESTORE;
}

// [vivakjh] 2009/01/07	PDP FHD MRE(FMC) 대응.
// The output V Sync can be set automatically or manually using this function.  => TRUE : VSync Out Auto, FALSE : VSync Out Manual.
void MHal_SC_VOP_SetAutoVSyncCtrl(BOOL bIsAutoVSync)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);

	if (bIsAutoVSync)
	{
		REG_WI(REG_SC_VOP(0x10), FALSE, BIT15);
	}
	else
	{
		REG_WI(REG_SC_VOP(0x10),  TRUE, BIT15);
	}

	SC_BK_RESTORE;
}


void MHal_SC_VOP_SetFreeRunColorEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WI(REG_SC_VOP(0x19), bEnable, BIT0);
	SC_BK_RESTORE;
}

void MHal_SC_VOP_SetFreeRunColor(SC_FREERUN_COLOR_e color)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WH(REG_SC_VOP(0x19), SC_FreeRunColor_Table[color][0]);
	REG_WL(REG_SC_VOP(0x1A), SC_FreeRunColor_Table[color][1]);
	REG_WH(REG_SC_VOP(0x1A), SC_FreeRunColor_Table[color][2]);
	SC_BK_RESTORE;
}

//------------------------------------------------------------------------------
//  MACE
//------------------------------------------------------------------------------
void MHal_SC_MACE_RequestHistogramData(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	// Clear histogram status
	//REG_WM(REG_SC_DLC(0x04), 0x0000, 0xFFF0);
	REG_WL(REG_SC_DLC(0x04), REG_RL(REG_SC_DLC(0x04)) & 0xF0);

	// Enable main window histogram, and request histogram
	//REG_WM(REG_SC_DLC(0x04), BIT4|BIT2|BIT1, 0x001F);
	REG_WL(REG_SC_DLC(0x04), REG_RL(REG_SC_DLC(0x04)) | BIT4 | BIT2 | BIT1);
	SC_BK_RESTORE;
}//thchen 20080717

void MHal_SC_MACE_DLCInit(U16 u16Histogram_Vstart, U16 u16Histogram_Vend) //thchen 20080708
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	REG_WL(REG_SC_DLC(0x01), (u16Histogram_Vstart/8));
	REG_WH(REG_SC_DLC(0x01), (u16Histogram_Vend/8));
	SC_BK_RESTORE;
}

BOOL MHal_SC_MACE_WaitHistogramDataReady(void)
{
	BOOL bRet;
	/*
	   lachesis_090123 inputsource disable하게 되면. histogram data의 apl값이 0으로 읽힘.
	   이 때문에 채널 전환 ARC가변시에 PWM 값이 변하게 됨.
	   inputsource가 disable되면 HistogramData가 not Ready로 간주함.
	   */
	if(!bIsInputSourceEnable)
	{
		return FALSE;
	}

	{
		SC_BK_STORE;
		SC_BK_SWICH(REG_SC_BK_DLC);

		if (REG_RR(REG_SC_DLC(0x04)) & BIT3)
		{
			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
		}

		SC_BK_RESTORE;
	}
	return bRet;
}

void MHal_SC_MACE_GetHistogram32(U16* pu16Histogram)
{
	U8 i;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	for (i=0; i<32; i++)
	{
		*(pu16Histogram + i) = REG_RR((REG_SC_DLC(0x40) + (i * 2)));
	}
	SC_BK_RESTORE;
}

U8 MHal_SC_MACE_GetMaxPixelValue(void)
{
	U8 u8Value;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	u8Value = REG_RL(REG_SC_DLC(0x0B));
	SC_BK_RESTORE;
	return u8Value;
}

U8 MHal_SC_MACE_GetMinPixelValue(void)
{
	U8 u8Value;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	u8Value = REG_RH(REG_SC_DLC(0x0B));
	SC_BK_RESTORE;
	return u8Value;
}

U8 MHal_SC_MACE_GetAvgPixelValue(void)
{
	U16 u16SumLuma;
	U16 u16TotalCount;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	// Get Sum of Luma
	u16SumLuma = REG_RR(REG_SC_DLC(0x06));
	// Get Total Count
	u16TotalCount = REG_RR(REG_SC_DLC(0x07));
	SC_BK_RESTORE;
	// calculate
	if (u16TotalCount != 0)
	{
		u16SumLuma = ((U32)u16SumLuma * 256 + (u16TotalCount/2)) / u16TotalCount;
	}
	else
	{
		u16SumLuma = 0;
	}

	bIsLowAvgLuma = (u16SumLuma < AVG_LUMA_TH)?TRUE:FALSE; //[091223_Leo]
	
	return (U8)u16SumLuma;
}

//[090601_Leo]
U16 MHal_SC_MACE_GetTotalColorCount(void)
{
	U16 u16TotalColorCount;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	u16TotalColorCount = REG_RR(REG_SC_DLC(0x6E));
	SC_BK_RESTORE;
	return u16TotalColorCount;
}

void MHal_SC_MACE_SetLumaCurve(U16* pLumaCurve)
{
	U8 count;
	U16 u16LumaCurve;
	U8 u8LumaCurveLsb;
	U8 u8LsbReg = 0x78;
	U16 RegPos = 2; //[3:2]

#if USE_MENULOAD_PQ //use menuload, [090917_Leo]
	MHal_SC_ML_Start();
	MENULOAD_LOCK;
	MHal_SC_ML_ChangeBank(0x1A);

	for(count=0; count<8; count++)
	{
		u16LumaCurve = pLumaCurve[count * 2 + 1] >> 2;
		u16LumaCurve = ((U16)pLumaCurve[count * 2] >> 2) | ((u16LumaCurve)<< 8);
		MHal_SC_ML_WriteData( REG_SC_DLC((0x30 + count)), u16LumaCurve, 0xFFFF);
		SC_PRINT("REG=0x%x, val=0x%x\n",(0x30 + count), u16LumaCurve);
	}

	for(count=0; count<16; count++)
	{
		u8LumaCurveLsb = (U8)(pLumaCurve[count] & 0x3);
		MHal_SC_ML_WriteData( REG_SC_DLC(u8LsbReg), u8LumaCurveLsb<<RegPos, 0x3<<RegPos);
		SC_PRINT("REG=0x%x, val=0x%x, pos=0x%x\n", u8LsbReg, u8LumaCurveLsb<<RegPos, 0x3<<RegPos);
		RegPos += 2;
		if(RegPos >= 16) //overflow to next 2 byte
		{
			RegPos = 0;
			u8LsbReg++;
		}
	}
	MHal_SC_ML_End();
	MENULOAD_UNLOCK;
#else



	for(count=0; count<8; count++)
	{
        //fix bug, [090825_Leo]
		//u16LumaCurve = pLumaCurve[count * 2 + 1] & 0xFF;
        u16LumaCurve = pLumaCurve[count * 2 + 1] >> 2;
		//u16LumaCurve = ((U16)pLumaCurve[count * 2] & 0xFF) | ((u16LumaCurve)<< 8);
		u16LumaCurve = ((U16)pLumaCurve[count * 2] >> 2) | ((u16LumaCurve)<< 8);
    	SC_BK_STORE;
	    SC_BK_SWICH(REG_SC_BK_DLC);
		REG_WR( REG_SC_DLC((0x30 + count)), u16LumaCurve);
		SC_BK_RESTORE;
		SC_PRINT("REG=0x%x, val=0x%x\n",(0x30 + count), u16LumaCurve);
	}

	for(count=0; count<16; count++)
	{
        //fix bug, [090825_Leo]
		//u8LumaCurveLsb = (U8)(pLumaCurve[count]>>8);
		u8LumaCurveLsb = (U8)(pLumaCurve[count] & 0x3);
		SC_BK_STORE;
	    SC_BK_SWICH(REG_SC_BK_DLC);
		REG_WM( REG_SC_DLC(u8LsbReg), u8LumaCurveLsb<<RegPos, 0x3<<RegPos);
		SC_BK_RESTORE;
		SC_PRINT("REG=0x%x, val=0x%x, pos=0x%x\n", u8LsbReg, u8LumaCurveLsb<<RegPos, 0x3<<RegPos);
		RegPos += 2;
		if(RegPos >= 16) //overflow to next 2 byte
		{
			RegPos = 0;
			u8LsbReg++;
		}
	}

#endif
}

void MHal_SC_MACE_SetLumaCurveEnable(BOOL bEnable)
{
#if USE_MENULOAD_PQ		//use menuload, [090917_Leo]
	MHal_SC_ML_Start();
	MENULOAD_LOCK;
	MHal_SC_ML_ChangeBank(0x1A);
	MHal_SC_ML_WriteData(REG_SC_DLC(0x04), (bEnable)<<7, BIT7);
	MHal_SC_ML_End();
	MENULOAD_UNLOCK;
#else
	//    U16 u16LumaCurve[8], count;
	SC_BK_STORE;
	/*	//KWON_0923 TEST
		u16LumaCurve[0] = 0x1808;
		u16LumaCurve[1] = 0x3828;
		u16LumaCurve[2] = 0x5848;
		u16LumaCurve[3] = 0x7868;
		u16LumaCurve[4] = 0x9888;
		u16LumaCurve[5] = 0xB8A8;
		u16LumaCurve[6] = 0xD8C8;
		u16LumaCurve[7] = 0xF8E8;
		*/
	SC_BK_SWICH(REG_SC_BK_DLC);
	REG_WI(REG_SC_DLC(0x04), (bEnable), BIT7); // LGE drmyung 081013
	/*
	   for (count=0; count<8; count++)
	   {
	   REG_WR( REG_SC_DLC((0x30 + count)), u16LumaCurve[count]);
	   }
	   */
	SC_BK_RESTORE;
#endif
}

void MHal_SC_MACE_SetHistogramReqEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	REG_WI(REG_SC_DLC(0x04), (bEnable), BIT14); // LGE drmyung 081013
	SC_BK_RESTORE;
}

//thchen 20080820
void MHal_SC_MACE_HistogramInit(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT1);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT4);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT5);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT7);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT9);
	REG_WI(REG_SC_DLC(0x08), TRUE, BIT0);
	SC_BK_RESTORE;
}

//LGE [vivakjh] 2008/12/27		Request chaning the histogram read point(0x1a High(0x04)) to all 0 from PQ team
void MHal_SC_MACE_HistogramInit4PDP(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DLC);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT1);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT4);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT5);
	REG_WI(REG_SC_DLC(0x04), TRUE, BIT7);
	REG_WI(REG_SC_DLC(0x04), FALSE, BIT9);
	REG_WI(REG_SC_DLC(0x08), TRUE, BIT0);
	SC_BK_RESTORE;
}


//FitchHsu 20081125 JPEG issue for rotate
void MHal_SC_SetInterruptMask(HAL_SC_INT_e u8IntSrc, BOOL bEnable)
{
	U16 u16ByteMask;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_GOPINT);
	u16ByteMask = (1 << (u8IntSrc));
	REG_WI(REG_SC_GOPINT(0x14), !bEnable, u16ByteMask);    // disable mask
	SC_BK_RESTORE;

	REG_WI(0x2B18, !bEnable, BIT10);
}

void MHal_SC_MACE_SetICCSaturationAdj(U8 u8ColorType, S8 s8SatAdj)
{
	U32  u32Reg;
	U8   u8SigBitNum;
	BOOL bHiByte;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_ACE);

	switch (u8ColorType)
	{
		case 0: // red
			u32Reg = REG_SC_ACE(0x31);
			bHiByte = FALSE;
			u8SigBitNum = BIT1;
			break;

		case 1: // green
			u32Reg = REG_SC_ACE(0x31);
			bHiByte = TRUE;
			u8SigBitNum = BIT2;
			break;

		case 2: // blue
			u32Reg = REG_SC_ACE(0x32);
			bHiByte = FALSE;
			u8SigBitNum = BIT3;
			break;

		case 3: // cyan
			u32Reg = REG_SC_ACE(0x32);
			bHiByte = TRUE;
			u8SigBitNum = BIT4;
			break;

		case 4: // magenta
			u32Reg = REG_SC_ACE(0x33);
			bHiByte = FALSE;
			u8SigBitNum = BIT5;
			break;

		case 5: // yellow
			u32Reg = REG_SC_ACE(0x33);
			bHiByte = TRUE;
			u8SigBitNum = BIT6;
			break;

		case 6: // fresh
			u32Reg = REG_SC_ACE(0x34);
			bHiByte = FALSE;
			u8SigBitNum = BIT7;
			break;

		default: // unknow
			u32Reg = REG_SC_ACE(0x00);
			bHiByte = FALSE;
			u8SigBitNum = BIT0;
			break;
	}

	if (bHiByte)
	{
		REG_WM(u32Reg, ((U16)abs(s8SatAdj)) << 8, 0x0F00);
	}
	else
	{
		REG_WM(u32Reg, abs(s8SatAdj), 0x000F);
	}
	REG_WI(REG_SC_ACE(0x35), (s8SatAdj<0) ? ENABLE : DISABLE, u8SigBitNum);
	REG_WM(REG_SC_ACE(0x36), 0x000F, 0x001F); // constant gain
	SC_BK_RESTORE;
}

void MHal_SC_MACE_SetIBCYAdj(U8 u8ColorType, U8 u8YAdj)
{
	U32 u32Reg;
	BOOL bHiByte;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_ACE);

	switch (u8ColorType)
	{
		case 0: // red
			u32Reg = REG_SC_ACE(0x41);
			bHiByte = FALSE;
			break;

		case 1: // green
			u32Reg = REG_SC_ACE(0x41);
			bHiByte = TRUE;
			break;

		case 2: // blue
			u32Reg = REG_SC_ACE(0x42);
			bHiByte = FALSE;
			break;

		case 3: // cyan
			u32Reg = REG_SC_ACE(0x42);
			bHiByte = TRUE;
			break;

		case 4: // magenta
			u32Reg = REG_SC_ACE(0x43);
			bHiByte = FALSE;
			break;

		case 5: // yellow
			u32Reg = REG_SC_ACE(0x43);
			bHiByte = TRUE;
			break;

		case 6: // fresh
			u32Reg = REG_SC_ACE(0x44);
			bHiByte = FALSE;
			break;

		default:
			u32Reg = REG_SC_ACE(0xFF);
			bHiByte = FALSE;
			break;
	}

	if (bHiByte)
	{
		U16 u16Tmp = u8YAdj;
		u16Tmp = (u16Tmp << 8);
		REG_WM(u32Reg, u16Tmp, 0x3F00);
	}
	else
	{
		REG_WM(u32Reg, u8YAdj, 0x003F);
	}
	SC_BK_RESTORE;
}

void MHal_SC_MACE_SetIHCHueDiffColorYAdj(U8 u8ColorType, S8 s8HueAdj, U8 u8YIndex, U8 u8YLevel)
{
	U32 u32HueReg, u32YLevelReg;
	BOOL bYLevelHiByte, bHueHiByte;
	U8 u8YRegAdd, u8HueRegYAdd;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_ACE2);

	switch (u8YIndex)
	{
		case 0:
			bYLevelHiByte = FALSE;
			u8YRegAdd = 0;
			u8HueRegYAdd = 0;
			break;
		case 1:
			bYLevelHiByte = TRUE;
			u8YRegAdd = 0;
			u8HueRegYAdd = 4;
			break;
		case 2:
			bYLevelHiByte = FALSE;
			u8YRegAdd = 1;
			u8HueRegYAdd = 8;
			break;
		case 3:
		default:
			bYLevelHiByte = TRUE;
			u8YRegAdd = 1;
			u8HueRegYAdd = 0;
			break;
	}

	switch (u8ColorType)
	{
		case 0: //red
			u32YLevelReg = REG_SC_ACE2((0x50 + u8YRegAdd));
			u32HueReg = REG_SC_ACE2((0x63 + u8HueRegYAdd));
			bHueHiByte = FALSE;
			break;
		case 1: //green
			u32YLevelReg = REG_SC_ACE2((0x52 + u8YRegAdd));
			u32HueReg = REG_SC_ACE2((0x63 + u8HueRegYAdd));
			bHueHiByte = TRUE;
			break;
		case 2: //blue
			u32YLevelReg = REG_SC_ACE2((0x54 + u8YRegAdd));
			u32HueReg = REG_SC_ACE2((0x64 + u8HueRegYAdd));
			bHueHiByte = FALSE;
			break;
		case 3: //cyan
			u32YLevelReg = REG_SC_ACE2((0x56 + u8YRegAdd));
			u32HueReg = REG_SC_ACE2((0x64 + u8HueRegYAdd));
			bHueHiByte = TRUE;
			break;
		case 4: //magenta
			u32YLevelReg = REG_SC_ACE2((0x58 + u8YRegAdd));
			u32HueReg = REG_SC_ACE2((0x65 + u8HueRegYAdd));
			bHueHiByte = FALSE;
			break;
		case 5: //yellow
			u32YLevelReg = REG_SC_ACE2((0x5A + u8YRegAdd));
			u32HueReg = REG_SC_ACE2((0x65 + u8HueRegYAdd));
			bHueHiByte = TRUE;
			break;
		case 6: //fresh
			u32YLevelReg = REG_SC_ACE2((0x5C + u8YRegAdd));
			u32HueReg = REG_SC_ACE2((0x66 + u8HueRegYAdd));
			bHueHiByte = FALSE;
			break;
		default:
			u32YLevelReg = REG_SC_ACE2((0x5E + u8YRegAdd));
			u32HueReg = REG_SC_ACE2((0x66 + u8HueRegYAdd));
			bHueHiByte = FALSE;
			break;
	}

	if(bYLevelHiByte)
		REG_WH(u32YLevelReg, u8YLevel);
	else
		REG_WL(u32YLevelReg, u8YLevel);
    SC_BK_RESTORE;
	if(u8YIndex == 3)//original hue setting
	{
		MHal_SC_MACE_SetIHCHueAdj(u8ColorType, s8HueAdj);
	}
	else
	{
	    SC_BK_STORE;
	    SC_BK_SWICH(REG_SC_BK_ACE2);
		if(bHueHiByte)
		{
			U16 u16Tmp = abs(s8HueAdj);
			u16Tmp = (u16Tmp << 8);
			REG_WM(u32HueReg, u16Tmp, 0x3F00);
			REG_WI(u32HueReg, (s8HueAdj<0) ? ENABLE : DISABLE, BIT14);
		}
		else
		{
			REG_WM(u32HueReg, abs(s8HueAdj), 0x003F);
			REG_WI(u32HueReg, (s8HueAdj<0) ? ENABLE : DISABLE, BIT6);
		}
		SC_BK_RESTORE;
	}

}

void MHal_SC_MACE_SetIHCHueAdj(U8 u8ColorType, S8 s8HueAdj)
{
	U32 u32Reg;
	BOOL bHiByte;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_ACE);

	switch (u8ColorType)
	{
		case 0: // red
			u32Reg = REG_SC_ACE(0x61);
			bHiByte = FALSE;
			break;

		case 1: // green
			u32Reg = REG_SC_ACE(0x61);
			bHiByte = TRUE;
			break;

		case 2: // blue
			u32Reg = REG_SC_ACE(0x62);
			bHiByte = FALSE;
			break;

		case 3: // cyan
			u32Reg = REG_SC_ACE(0x62);
			bHiByte = TRUE;
			break;

		case 4: // magenta
			u32Reg = REG_SC_ACE(0x63);
			bHiByte = FALSE;
			break;

		case 5: // yellow
			u32Reg = REG_SC_ACE(0x63);
			bHiByte = TRUE;
			break;

		case 6: // fresh
			u32Reg = REG_SC_ACE(0x64);
			bHiByte = FALSE;
			break;

		default:
			u32Reg = REG_SC_ACE(0x64);
			bHiByte = FALSE;
			break;
	}

	if (bHiByte)
	{
		U16 u16Tmp = abs(s8HueAdj);
		u16Tmp = (u16Tmp << 8);
		REG_WM(u32Reg, u16Tmp, 0x3F00);
		REG_WI(u32Reg, (s8HueAdj<0) ? ENABLE : DISABLE, BIT14);
	}
	else
	{
		REG_WM(u32Reg, abs(s8HueAdj), 0x003F);
		REG_WI(u32Reg, (s8HueAdj<0) ? ENABLE : DISABLE, BIT6);
	}

	//REG_WM(u16Reg, abs(s8HueAdj), 0x3F);
	//REG_WI(u16Reg, (s8HueAdj<0) ? ENABLE : DISABLE, BIT6);
	SC_BK_RESTORE;
}

void MHal_SC_MACE_IHCHueInit()
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_ACE);
	REG_WI(REG_SC_ACE(0x30), 0x0080, 0x0080);
	SC_BK_RESTORE;
}

void MHal_SC_MACE_SetICCSaturationEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetICCSaturationEnable bEnable=%x \n ",bEnable);
	SC_BK_SWICH(REG_SC_BK_ACE);
	//should be enable from init.
	REG_WI(REG_SC_ACE(0x30), 1, BIT7);
	REG_WI(REG_SC_ACE(0x30), bEnable, BIT6);
	SC_BK_RESTORE;
}//thchen 20080718

void MHal_SC_MACE_SetIBCYEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetIBCYEnable bEnable=%x \n ",bEnable);
	SC_BK_SWICH(REG_SC_BK_ACE);
	REG_WI(REG_SC_ACE(0x40), bEnable, BIT7);
	SC_BK_RESTORE;
}//thchen 20080718

void MHal_SC_MACE_SetIHCHueEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetIHCHueEnable bEnable=%x \n ",bEnable);
	SC_BK_SWICH(REG_SC_BK_ACE);
	REG_WI(REG_SC_ACE(0x60), bEnable, BIT7);
	SC_BK_RESTORE;
}//thchen 20080718

void MHal_SC_MACE_SetBlueStretchEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetBlueStretchEnable bEnable=%x \n ",bEnable);
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	REG_WI(REG_SC_ACE(0x70), bEnable, BIT0);
	SC_BK_RESTORE;
}//victor 20080830

void MHal_SC_MACE_SetIHCYModeDiffColorEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetIHCYModeDiffColorEnable bEnable=%x \n ",bEnable);
	SC_BK_SWICH(REG_SC_BK_ACE2);
	REG_WI(REG_SC_ACE(0x60), bEnable, BIT1);
	SC_BK_RESTORE;
}//[090623_Leo]

void MHal_SC_MACE_SetIHCYModelEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetIHCYModelEnable bEnable=%x \n ",bEnable);
	SC_BK_SWICH(REG_SC_BK_ACE2);
	REG_WI(REG_SC_ACE(0x60), bEnable, BIT7);
	SC_BK_RESTORE;
}//victor 20080818

void MHal_SC_MACE_SetICCYModelEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetICCYModelEnable bEnable=%x \n ",bEnable);
	SC_BK_SWICH(REG_SC_BK_ACE2);
	REG_WI(REG_SC_ACE(0x30), bEnable, BIT7);
	SC_BK_RESTORE;

}//victor 20080818

void MHal_SC_MACE_SetICCRegionTable(U8 *data)   //victor 20080814
{
	int i;	/* To avoid compilation warning msg. by LGE. jjab. */
	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetICCRegionTable\n");
	SC_BK_SWICH(REG_SC_BK_ACE);

	REG_WL(REG_SC_ACE(0x78), 0x1);  // icc_sram_io_en

	for(i=0;i<256;i++)
	{
		REG_WL(REG_SC_ACE(0x79), i);        // Address
		REG_WL(REG_SC_ACE(0x7A), data[i]);  // Data
		REG_WH(REG_SC_ACE(0x7B), 0x0);      // Hidden Register ??
		REG_WH(REG_SC_ACE(0x7A), 0x1);      // write enable
		SC_BK_SWICH(REG_SC_BK_ACE);         // Just for delay
	}

	REG_WL(REG_SC_ACE(0x78), 0x0);  // icc_sram_io_en
	REG_WI(REG_SC_ACE(0x30), 1, BIT8);
	SC_BK_RESTORE;
}

void MHal_SC_MACE_SetIHCRegionTable(U8 *data)   //victor 20080901
{
	int i;	/* To avoid compilation warning msg. by LGE. jjab. */
	int j;
	int addr = 0;
	U8 *pData;

	SC_BK_STORE;
	SC_PRINT("MHal_SC_MACE_SetIHCRegionTable\n");
	SC_BK_SWICH(REG_SC_BK_ACE);

	REG_WL(REG_SC_ACE(0x7C), 0x1);  // ihc_sram_io_en
	addr = 0;
	SC_PRINT("//ihc_sram_1\n");
	for(i=0;i<9;i++)                // SRAM 0
	{
		pData = &data[i*2*17];       // Even Row
		for(j=0;j<8;j++, addr++)
		{
			SC_PRINT("wriu 0x2ffa 0x%02x\n", addr);
			SC_PRINT("wriu 0x2ffc 0x%02x\n", pData[j*2]);
			SC_PRINT("wriu 0x2ffd 0x01\n");
			SC_PRINT("wriu 0x2f00 0x18  //delay\n\n");
			REG_WL(REG_SC_ACE(0x7D), addr);     // Address
			REG_WL(REG_SC_ACE(0x7E), pData[j*2]);  // Data, Even Column
			REG_WH(REG_SC_ACE(0x7E), 0x1);      // write enable
			SC_BK_SWICH(REG_SC_BK_ACE);         // Just for delay
		}
	}
	for(i=0;i<9;i++, addr++)
	{
		SC_PRINT("wriu 0x2ffa 0x%02x\n", addr);
		SC_PRINT("wriu 0x2ffc 0x%02x\n", data[i*2*17 + 16]);
		SC_PRINT("wriu 0x2ffd 0x01\n");
		SC_PRINT("wriu 0x2f00 0x18  //delay\n\n");
		REG_WL(REG_SC_ACE(0x7D), addr);     // Address
		REG_WL(REG_SC_ACE(0x7E), data[i*2*17 + 16]);  // Data, Even Column of Last Line, from Bottom to Top
		REG_WH(REG_SC_ACE(0x7E), 0x1);      // write enable
		SC_BK_SWICH(REG_SC_BK_ACE);         // Just for delay
	}


	REG_WL(REG_SC_ACE(0x7C), 0x3);  // ihc_sram_io_en
	addr = 0;
	SC_PRINT("//ihc_sram_2\n");
	for(i=0;i<9;i++)                // SRAM 1
	{
		pData = &data[i*2*17];       // Even Row
		for(j=0;j<8;j++, addr++)
		{
			SC_PRINT("wriu 0x2ffa 0x%02x\n", addr);
			SC_PRINT("wriu 0x2ffc 0x%02x\n", pData[j*2+1]);
			SC_PRINT("wriu 0x2ffd 0x01\n");
			SC_PRINT("wriu 0x2f00 0x18  //delay\n\n");
			REG_WL(REG_SC_ACE(0x7D), addr);     // Address
			REG_WL(REG_SC_ACE(0x7E), pData[j*2+1]);  // Data, Odd Column
			REG_WH(REG_SC_ACE(0x7E), 0x1);      // write enable
			SC_BK_SWICH(REG_SC_BK_ACE);         // Just for delay
		}
	}

	REG_WL(REG_SC_ACE(0x7C), 0x5);  // ihc_sram_io_en
	addr = 0;
	SC_PRINT("//ihc_sram_3\n");
	for(i=0;i<8;i++)                // SRAM 2
	{
		pData = &data[(i*2+1)*17];   // Odd Row
		for(j=0;j<8;j++, addr++)
		{
			SC_PRINT("wriu 0x2ffa 0x%02x\n", addr);
			SC_PRINT("wriu 0x2ffc 0x%02x\n", pData[j*2]);
			SC_PRINT("wriu 0x2ffd 0x01\n");
			SC_PRINT("wriu 0x2f00 0x18  //delay\n\n");
			REG_WL(REG_SC_ACE(0x7D), addr);     // Address
			REG_WL(REG_SC_ACE(0x7E), pData[j*2]);  // Data, Even Column
			REG_WH(REG_SC_ACE(0x7E), 0x1);      // write enable
			SC_BK_SWICH(REG_SC_BK_ACE);         // Just for delay
		}
	}
	for(i=0;i<8;i++, addr++)
	{
		SC_PRINT("wriu 0x2ffa 0x%02x\n", addr);
		SC_PRINT("wriu 0x2ffc 0x%02x\n", data[(i*2+1)*17 + 16]);
		SC_PRINT("wriu 0x2ffd 0x01\n");
		SC_PRINT("wriu 0x2f00 0x18  //delay\n\n");
		REG_WL(REG_SC_ACE(0x7D), addr);     // Address
		REG_WL(REG_SC_ACE(0x7E), data[(i*2+1)*17 + 16]);  // Data, Odd Column of Last Line, from Bottom to Top
		REG_WH(REG_SC_ACE(0x7E), 0x1);      // write enable
		SC_BK_SWICH(REG_SC_BK_ACE);         // Just for delay
	}

	REG_WL(REG_SC_ACE(0x7C), 0x7);  // ihc_sram_io_en
	addr = 0;
	SC_PRINT("//ihc_sram_4\n");
	for(i=0;i<8;i++)                // SRAM 3
	{
		pData = &data[(i*2+1)*17];   // Odd Row

		for(j=0;j<8;j++, addr++)
		{
			SC_PRINT("wriu 0x2ffa 0x%02x\n", addr);
			SC_PRINT("wriu 0x2ffc 0x%02x\n", pData[j*2+1]);
			SC_PRINT("wriu 0x2ffd 0x01\n");
			SC_PRINT("wriu 0x2f00 0x18  //delay\n\n");
			REG_WL(REG_SC_ACE(0x7D), addr);     // Address
			REG_WL(REG_SC_ACE(0x7E), pData[j*2+1]);  // Data, Even Column
			REG_WH(REG_SC_ACE(0x7E), 0x1);      // write enable
			SC_BK_SWICH(REG_SC_BK_ACE);         // Just for delay
		}
	}

	SC_BK_SWICH(REG_SC_BK_ACE);
	REG_WL(REG_SC_ACE(0x7C), 0x0);  // ihc_sram_io_en
	SC_BK_SWICH(REG_SC_BK_ACE);
	REG_WI(REG_SC_ACE(0x60), 1, BIT8);  // Use SRAM Table
	SC_BK_RESTORE;
}

void MHal_SC_SelectCSC(U8 u8selection)
{
    // 20090925 daniel.huang: set equation to VIP CSC
    // Both equation selection rules are
    // 00: SDTV(601) R  G  B  : 16-235
    // 01: SDTV(601) R  G  B  : 0-255
    // 10: HDTV(709) R  G  B  : 16-235
    // 11: HDTV(709) R  G  B  : 0-255
    // 20090925 daniel.huang: set both IP2F2 & VIP CSC equation because only one CSC is enabled at the same time
    //                        together with CSC equation

  #if USE_MENULOAD_PQ
    MHal_SC_ML_Start();
    MENULOAD_LOCK;
    MHal_SC_ML_ChangeBank(0x18);
    MHal_SC_ML_WriteData(REG_SC_ACE(0x6F), (U16)u8selection, 0x0003);

    MHal_SC_ML_ChangeBank(0x02);
    MHal_SC_ML_WriteData(REG_SC_IP2F2(0x40), (U16)u8selection, 0x0003);
    MHal_SC_ML_End();
    MENULOAD_UNLOCK;
  #else//use menuload, [091021_Leo]
	SC_BK_STORE;
	SC_PRINT("MHal_SC_SelectCSC %d\n", u8selection);
	SC_BK_SWICH(REG_SC_BK_IP2F2);
	REG_WM(REG_SC_IP2F2(0x40), u8selection, 0x3);   //victor 20080830

    SC_BK_SWICH(REG_SC_BK_ACE);
    REG_WM(REG_SC_ACE(0x6F), u8selection, 0x03);    // 20090925 daniel.huang: set equation to VIP CSC
	SC_BK_RESTORE;
  #endif
}

void MHal_SC_SetCSCOffset(BOOL isMinus16)
{
	SC_BK_STORE;

	SC_PRINT("%s\n", __FUNCTION__);
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	REG_WI(REG_SC_S_VOP(0x57), isMinus16, BIT6);   // - 16 offset for CSC
	SC_BK_RESTORE;
}//victor 20080830

//-----------------------------------------------------------------------------
// OP1 control
//-----------------------------------------------------------------------------

/******************************************************************************/
/// Get the horizontal period of auto postion
/// @return the horizontal period of auto postion
/******************************************************************************/
U16 MHal_SC_GetHorizontalDEStart(void)
{
	U16 u16HorizontalDE;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);

	u16HorizontalDE = REG_RR(REG_SC_IP1F2(0x13));

	SC_BK_RESTORE;

	return u16HorizontalDE;
}

/******************************************************************************/
/// Get the vertical period of auto postion
/// @return the vertical period of auto postion
/******************************************************************************/
U16 MHal_SC_GetVerticalDEStart(void)
{
	U16 u16VerticalDE;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);

	u16VerticalDE = REG_RR(REG_SC_IP1F2(0x12));

	SC_BK_RESTORE;

	return u16VerticalDE;
}

/******************************************************************************/
///This function will return mode detect status
///@return
///- U8 mode status
/******************************************************************************/
U8 MHal_SC_GetModeStatus(void)
{
	U8 u8DetectStatus;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u8DetectStatus = REG_RH(REG_SC_IP1F2(0x1E));
	SC_BK_RESTORE;

	return u8DetectStatus;
}

/******************************************************************************/
/// Get the vertical period of auto postion
/// @return the vertical period of auto postion
/******************************************************************************/
U16 MDrv_Scaler_GetVerticalDE(void)
{
	U16 u16VerticalDE;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);

	u16VerticalDE = REG_RR(REG_SC_IP1F2(0x14)) -
		REG_RR(REG_SC_IP1F2(0x12)) + 1;

	SC_BK_RESTORE;

	return u16VerticalDE;
}
SC_SCART_MODE_e MHal_SC_GetScartMode(void)
{
	// RGB: 1-3V, CVBS: 0-0.4V

	//TODO: check selection is 0/1/2 according to layout
	// SC1_FB : HSYNC1
	U8 u8fblank_sel = 2;
	//U8 u8Val, u8RGBThreshold = 85;

	REG_WM(REG_ADC_ATOP(0x40), (u8fblank_sel<<4), (BIT5|BIT4));
	if ((REG_RH(REG_ADC_ATOP(0x45)) & 0x0F) > 5)    //bit[11:8]
	{
		return SCART_MODE_RGB;
	}
	else
	{
		return SCART_MODE_CVBS;
	}
	return SCART_MODE_SVIDEO;
}

/******************************************************************************/
/// Set SCART overlay mode(Mixed mode: RGB overlay mode on the CVBS video) on or off.
/// @param -bOverlay \b IN: SCART overlay mode on/off
/******************************************************************************/
void MHal_SC_SetScartOverlay(BOOL bOverlay)
{
	if (bOverlay)   // Show SCART-CVBS & SCART-RGB simutaneously
	{
		REG_WM(REG_ADC_ATOP(0x42), 0x3 << 6, (BIT7|BIT6));  // 0x2584[7:6], select fast blanking mixing mode
		REG_WM(REG_ADC_ATOP(0x43), 0x1F, 0x3F);             // 0x2586[5:0], select FB input pipe delay
	}
	else
	{
		REG_WM(REG_ADC_ATOP(0x42), 0x1 << 6, (BIT7|BIT6));  // 0x2584[7:6], select fast blanking mixing mode
		REG_WM(REG_ADC_ATOP(0x43), 0x00, 0x3F);             // 0x2586[5:0], select FB input pipe delay
	}
}

void MHal_SC_ScartIDInit(void)
{
	// SAR init
	REG_WR( REG_PM_SAR(0x00), 0xAA4);
	REG_WL( REG_PM_SAR(0x01), 0x5);
	REG_WR( REG_PM_SAR(0x0C), 0x80FF);
}

void MHal_SC_GetScart1IDLevel(U8* pIDLevel)
{
	*pIDLevel = 0;
	REG_WM(REG_ADC_ATOP(0x30), BIT5, BIT5); //20100202 daniel.huang
	// make sure connection of scart1/scart2
	REG_WM(REG_ADC_ATOP(0x32), 0x2000, 0xF000);   // Channel 1
	msleep(1);
	//if (REG_RI(REG_PM_SAR(0x14), BIT15)) // sar ready signal
	{
		*pIDLevel = REG_RH(REG_PM_SAR(0x10));  // Change to SAR 3
	}
}
//FitchHsu 20080929
void MHal_SC_GetScart2IDLevel(U8* pIDLevel)
{
	*pIDLevel = 0;
	REG_WM(REG_ADC_ATOP(0x30), BIT5, BIT5); //20100202 daniel.huang
	//make sure connection of scart1/scart2
	REG_WM(REG_ADC_ATOP(0x32), 0x4000, 0xF000);             // Channel 2

	msleep(1);
	//if (REG_RI(REG_PM_SAR(0x14), BIT15))
	{
		*pIDLevel = REG_RH(REG_PM_SAR(0x10)); // Change to SAR 4
	}
}

//20100202 daniel.huang
BOOL MHal_SC_GetScartIDLevelSelect(void)
{
    return (REG_RR(REG_ADC_ATOP(0x66)) & BIT15) ? TRUE: FALSE;
}

//thchen 20081001
void MHal_SC_SetLockPoint(U16 u16LockPoint)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	REG_WM(REG_SC_S_VOP(0x55), u16LockPoint, 0x7FF);
	SC_BK_RESTORE;
}
//thchen 20081001
void MHal_SC_SetFreezePoint(U16 u16FreezePoint)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	REG_WM(REG_SC_S_VOP(0x54), u16FreezePoint, 0x7FF);
	SC_BK_RESTORE;
}
//thchen 20081001
void MHal_SC_FreezeVCN(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	// Use new Lock point
	REG_WI(REG_SC_S_VOP(0x56), TRUE, BIT1);
	// Freeze VCN
	REG_WI(REG_SC_S_VOP(0x56), FALSE, BIT0);
	REG_WI(REG_SC_S_VOP(0x56), TRUE, BIT0);
	SC_BK_RESTORE;
	MHal_SC_SetFastFrameModeStatus(TRUE);
}

// 20091202 daniel.huang: use timer for vcount freeze reset
void MHal_SC_ResetFreezeVCN(unsigned long arg)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_S_VOP);

    REG_WI(REG_SC_S_VOP(0x56), TRUE,  BIT3);
    REG_WI(REG_SC_S_VOP(0x56), FALSE, BIT3);
	//printk("\n===========MHal_SC_ResetFreezeVCN=============\n");
    SC_BK_RESTORE;
}

//LGE [vivakjh] 2008/11/12 with cc.chen	Add for DVB PDP Panel

void MHal_SC_FreezeVCN4PDP(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	// Use new Lock point
	REG_WI(REG_SC_S_VOP(0x56), TRUE, BIT1);
	SC_BK_RESTORE;
	// Freeze VCN
	//REG_WI(REG_SC_S_VOP(0x56), FALSE, BIT0);
	//REG_WI(REG_SC_S_VOP(0x56), TRUE, BIT0);
#if 0
	// For Test
	while (1)
	{
		if (REG_RI(REG_SC_S_VOP(0x56), BIT15))
		{
			break;
		}
	}
#endif
	MHal_SC_SetFastFrameModeStatus(FALSE);

}

void MHal_SC_SetOutputFreeRun(BOOL bEnFreerunOutput)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WH(REG_SC_VOP(0x1C), bEnFreerunOutput);
	SC_BK_RESTORE;
}

void MHal_SC_SetOutputVTotal(U16 u16OutputVtotal)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);
	REG_WR(REG_SC_VOP(0x0D), u16OutputVtotal);
	SC_BK_RESTORE;
}
//victor 20080922, T.B.D, Add new feature first, refine the code later.
/******************************************************************************/
/*                          Macro                                             */
/******************************************************************************/
#define DRVSCA_DBG(x)       //x
#define DBG_DEFEATHERING    0
#define DBG_DEFLICKERING    0
#define DBG_DEBOUNCING      0
#define DBG_DYNAMIC_SNR     0
#define DBG_DYNAMIC_DNR     0

/******************************************************************************/
/*                           Constant                                         */
/******************************************************************************/
// DeFeathering
#define MDP_CNT     2
#define ENABLE_MDP  0   //Motion decrase progressively enable

#if 0//[091201_Leo]
#define DEFETHERING_LV1_TH              30000  //0
#define DEFETHERING_LV2_TH              5000   //0
#define DEFETHERING_LV3_TH              2000  //0
#define DEFETHERING_LV1_CNT             25
#define DEFETHERING_LV2_CNT             20
#define DEFETHERING_LV3_CNT             5
#else
#define DEFETHERING_LV1_TH              230000  //0
#define DEFETHERING_LV2_TH              140000   //0
#define DEFETHERING_LV3_TH              5000  //0
#define DEFETHERING_LV4_TH              2000  //0
#define DEFETHERING_LOWLUMA_LV1_TH      30000 //[091223_Leo]
#define DEFETHERING_LV1_CNT             5
#define DEFETHERING_LV2_CNT             3
#define DEFETHERING_LV3_CNT             1
#define DEFETHERING_LV4_CNT             5
#define DEFETHERING_LOWLUMA_LV1_CNT     25 //[091223_Leo]
#endif
U8 SST_STATIC_CORE_TH_LV1_VALUE;	//  =  0x0F;//victor 20080923
U8 SST_STATIC_CORE_TH_LV2_VALUE;	//  =  0x08;//victor 20080923
U8 SST_STATIC_CORE_TH_LV3_VALUE;	//  =  0x04;//victor 20080923
U8 SST_STATIC_CORE_TH_LV4_VALUE;	//  =  0x00;//victor 20080923
U8 SST_STATIC_CORE_TH_LV5_VALUE;	//  =  0x00;//[091201_Leo]

// DeFlickering
#define DEFLICKERING_TH                 0x2FFFF//52000  //0 //modify the threshold, [091023_Leo]
#define DEFLICKERING_CNT                150

// DeBouncing
#define DEBOUNCING_TH                   35000  //0
#define DEBOUNCING_CNT                  10

// Dynamic SNR
#define DYNAMIC_SNR_TH                  2000
#define DYNAMIC_SNR_CNT                 30

// Dynamic DNR
//#define DYNAMIC_DNR_TH                  6000

#define DNR_TABLEY_0L_Zero_VALUE        0xDD
#define DNR_TABLEY_0H_Zero_VALUE        0xBD
#define DNR_TABLEY_1L_Zero_VALUE        0x79
#define DNR_TABLEY_1H_Zero_VALUE        0x35
#define DNR_TABLEY_2L_Zero_VALUE        0x11
#define DNR_TABLEY_2H_Zero_VALUE        0x00
#define DNR_TABLEY_3L_Zero_VALUE        0x00
#define DNR_TABLEY_3H_Zero_VALUE        0x00

#define DNR_TABLEY_0L_LV2_VALUE         0xCC
#define DNR_TABLEY_0H_LV2_VALUE         0xAC
#define DNR_TABLEY_1L_LV2_VALUE         0x68
#define DNR_TABLEY_1H_LV2_VALUE         0x24
#define DNR_TABLEY_2L_LV2_VALUE         0x00
#define DNR_TABLEY_2H_LV2_VALUE         0x00
#define DNR_TABLEY_3L_LV2_VALUE         0x00
#define DNR_TABLEY_3H_LV2_VALUE         0x00

#define DNR_TABLEY_0L_LV3_VALUE         0x67
#define DNR_TABLEY_0H_LV3_VALUE         0x45
#define DNR_TABLEY_1L_LV3_VALUE         0x33
#define DNR_TABLEY_1H_LV3_VALUE         0x22
#define DNR_TABLEY_2L_LV3_VALUE         0x11
#define DNR_TABLEY_2H_LV3_VALUE         0x00
#define DNR_TABLEY_3L_LV3_VALUE         0x00
#define DNR_TABLEY_3H_LV3_VALUE         0x00

// Dynamic Film 22
#define DYNAMIC_FILM22_TH               520000

//fitch 20081222
/******************************************************************************/
///Set control register for adaptive tuning function
/******************************************************************************/
void MHal_Scaler_SetAdaptiveCtrl(U8 u8Ctrl)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);;
	REG_WL(REG_SC_MADI(0x7C), u8Ctrl);
	SC_BK_RESTORE;
}

//victor 20080922
/******************************************************************************/
///Get control register for adaptive tuning function
///@return U8: Control status
/******************************************************************************/
U8 MHal_Scaler_GetAdaptiveCtrl(void)
{
	U8 u8Ctrl;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);;
	u8Ctrl = REG_RL(REG_SC_MADI(0x7C));
	SC_BK_RESTORE;
	return u8Ctrl;
}

//victor 20080922
/******************************************************************************/
///Read motion value (F2 motion status)
///@return U8: Motion value
/******************************************************************************/
U32 MHal_Scaler_ReadMotionValue1(void)
{
	U32 u32MotionValue;
	U32 u32RegMadi_1C, u32RegMadi_1B, u32RegMadi_1A;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);;
	u32RegMadi_1C = (U32)REG_RL(REG_SC_MADI(0x0e)) & 0x3F;
	u32RegMadi_1B = (U32)REG_RH(REG_SC_MADI(0x0d));
	u32RegMadi_1A = (U32)REG_RL(REG_SC_MADI(0x0d));
	SC_BK_RESTORE;

	u32RegMadi_1C = u32RegMadi_1C * 0x10000UL;
	u32RegMadi_1B = u32RegMadi_1B * 0x100UL;

	u32MotionValue = ( u32RegMadi_1C + u32RegMadi_1B + u32RegMadi_1A)  ;

	DRVSCA_DBG(printf("MotionValue = 0x%lx\n", u32MotionValue));


	return u32MotionValue;
}

//victor 20080922
/******************************************************************************/
///DeFeathering
///@param u32MotionValue \b IN: Motion value
/******************************************************************************/
//modified motion condition, [091201_Leo]
void MHal_Scaler_DeFeathering(U32 u32MotionValue, BOOL IsDTV)
{
	static U32 u32DeFeatherCntLv1 = 0;
	static U32 u32DeFeatherCntLv2 = 0;
	static U32 u32DeFeatherCntLv3 = 0;
    static U32 u32DeFeatherCntLv4 = 0;
    static U32 u32DeFeatherCntLowLumaLv1 = 0; //[091223_Leo]
    
	U8 u8SST_Static_Core_TH;
    U8 u8SST_Static_Tgain_TH;

	//int reg_defethering_lv1_cnt;
	//int reg_defethering_lv2_cnt;
	int reg_mdp_cnt;

	//reg_defethering_lv1_cnt = 0x08;
	//reg_defethering_lv2_cnt = 0x01;
	reg_mdp_cnt = 0x01;

	//divided by avg luma condition, [091223_Leo]
	if (bIsLowAvgLuma)    
	{        
		if (u32MotionValue >= DEFETHERING_LOWLUMA_LV1_TH)        
		{            
			if (u32DeFeatherCntLowLumaLv1 < DEFETHERING_LOWLUMA_LV1_CNT)                
				u32DeFeatherCntLowLumaLv1++;        
		}        
		else        
		{            
			if (u32DeFeatherCntLowLumaLv1 >= reg_mdp_cnt)                
				u32DeFeatherCntLowLumaLv1 = u32DeFeatherCntLowLumaLv1 - reg_mdp_cnt;            
			if(reg_mdp_cnt == 0xFF)                
				u32DeFeatherCntLowLumaLv1 = 0;        
		}        
		//Lv1 in normal condition		
		if (u32DeFeatherCntLv1 >= reg_mdp_cnt)			
			u32DeFeatherCntLv1 = u32DeFeatherCntLv1 - reg_mdp_cnt;		
		if(reg_mdp_cnt == 0xFF)			
			u32DeFeatherCntLv1 = 0;        
		//Lv2 in normal condition		
		if (u32DeFeatherCntLv2 >= reg_mdp_cnt)			
			u32DeFeatherCntLv2 = u32DeFeatherCntLv2 - reg_mdp_cnt;		
		if(reg_mdp_cnt == 0xFF)			
			u32DeFeatherCntLv2 = 0;    
	}    
	else    
	{
		// motion level count
		if (u32MotionValue >= DEFETHERING_LV1_TH)
		{
			if (u32DeFeatherCntLv1 < DEFETHERING_LV1_CNT)
				u32DeFeatherCntLv1++;
		}
		else
		{
			if (u32DeFeatherCntLv1 >= reg_mdp_cnt)
				u32DeFeatherCntLv1 = u32DeFeatherCntLv1 - reg_mdp_cnt;
			if(reg_mdp_cnt == 0xFF)
				u32DeFeatherCntLv1 = 0;
		}

		if (u32MotionValue >= DEFETHERING_LV2_TH)
		{
			if (u32DeFeatherCntLv2 < DEFETHERING_LV2_CNT)
				u32DeFeatherCntLv2++;
		}
		else
		{
			if (u32DeFeatherCntLv2 >= reg_mdp_cnt)
				u32DeFeatherCntLv2 = u32DeFeatherCntLv2 - reg_mdp_cnt;
			if(reg_mdp_cnt == 0xFF)
				u32DeFeatherCntLv2 = 0;
		}

    	//Lv1 in low luma condition        
    	if (u32DeFeatherCntLowLumaLv1 >= reg_mdp_cnt)            
			u32DeFeatherCntLowLumaLv1 = u32DeFeatherCntLowLumaLv1 - reg_mdp_cnt;        

		if(reg_mdp_cnt == 0xFF)            
			u32DeFeatherCntLowLumaLv1 = 0;    
	}
        
	if (u32MotionValue >= DEFETHERING_LV3_TH)
	{
		if (u32DeFeatherCntLv3 < DEFETHERING_LV3_CNT)
			u32DeFeatherCntLv3++;
	}
	else
	{
		if (u32DeFeatherCntLv3 >= reg_mdp_cnt)
			u32DeFeatherCntLv3 = u32DeFeatherCntLv3 - reg_mdp_cnt;
		if(reg_mdp_cnt == 0xFF)
			u32DeFeatherCntLv3 = 0;
	}

	if (u32MotionValue >= DEFETHERING_LV4_TH)
	{
		if (u32DeFeatherCntLv4 < DEFETHERING_LV4_CNT)
			u32DeFeatherCntLv4++;
	}
	else
	{
		if (u32DeFeatherCntLv4 >= reg_mdp_cnt)
			u32DeFeatherCntLv4 = u32DeFeatherCntLv4 - reg_mdp_cnt;
		if(reg_mdp_cnt == 0xFF)
			u32DeFeatherCntLv4 = 0;
	}

	//DeFeathering begin
        //divided by avg luma condition, [091223_Leo]	
       if (bIsLowAvgLuma)	
	{        
		if (u32DeFeatherCntLowLumaLv1 >= DEFETHERING_LOWLUMA_LV1_CNT)        
		{            
			u8SST_Static_Core_TH    = 0x14;//0x1A;        
		}    	
		else if (u32DeFeatherCntLv3 >= DEFETHERING_LV3_CNT)    	
		{    		
			u8SST_Static_Core_TH    = 0x0C;//0x10;//SST_STATIC_CORE_TH_LV3_VALUE;//0x08    	
		}    	
		else if (u32DeFeatherCntLv4 >= DEFETHERING_LV4_CNT)    	
		{    		
			u8SST_Static_Core_TH    = 0x8;//SST_STATIC_CORE_TH_LV4_VALUE;//0x04    	
		}        
		else        
		{            
			u8SST_Static_Core_TH    = 0x04;//SST_STATIC_CORE_TH_LV5_VALUE;//0x0        
		}        
		u8SST_Static_Tgain_TH       = 0x05;	
	}    
	else    
	{	
		if (u32DeFeatherCntLv1 >= DEFETHERING_LV1_CNT)
		{
			if( IsDTV == TRUE)
			{
				u8SST_Static_Core_TH    = 0x14;
				u8SST_Static_Tgain_TH   = 0x05;
			}
			else
			{
				u8SST_Static_Core_TH    = SST_STATIC_CORE_TH_LV1_VALUE;//0x1A
				u8SST_Static_Tgain_TH   = 0x02;
			}
		}
		else if (u32DeFeatherCntLv2 >= DEFETHERING_LV2_CNT)
		{
			u8SST_Static_Core_TH    = SST_STATIC_CORE_TH_LV2_VALUE;//0x14
	        	u8SST_Static_Tgain_TH   = 0x05;
		}
		else if (u32DeFeatherCntLv3 >= DEFETHERING_LV3_CNT)
		{
			u8SST_Static_Core_TH    = SST_STATIC_CORE_TH_LV3_VALUE;//0x08
	        	u8SST_Static_Tgain_TH   = 0x05;
		}
		else if (u32DeFeatherCntLv4 >= DEFETHERING_LV4_CNT)
		{
			u8SST_Static_Core_TH    = SST_STATIC_CORE_TH_LV4_VALUE;//0x04
	        	u8SST_Static_Tgain_TH   = 0x05;
		}
	    	else
	    	{
	        	u8SST_Static_Core_TH    = SST_STATIC_CORE_TH_LV5_VALUE;//0x0
	        	u8SST_Static_Tgain_TH   = 0x05;
	    	}
	}
	
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);

    //REG_WL(REG_SC_MADI(0x1A), u8SST_Static_Core_TH & 0x3F);
    //REG_WH(REG_SC_MADI(0x1A), u8SST_Static_Tgain_TH & 0xF);

	REG_WM(REG_SC_MADI(0x1A), u8SST_Static_Core_TH, 0x3F);
    REG_WM(REG_SC_MADI(0x1A), u8SST_Static_Tgain_TH<<8, 0xF00);

	SC_BK_RESTORE;
}

//victor 20080922
/******************************************************************************/
///DeFlickering
///@param u32MotionValue \b IN: Motion value
/******************************************************************************/
void MHal_Scaler_DeFlickering(U32 u32MotionValue)
{
	static S32 s32DeFlickerCnt = 0;

	U32 reg_m_feat_smooth_hle_th, reg_m_feat_smooth_shrink;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);

	reg_m_feat_smooth_hle_th = (U32)(REG_RH(REG_SC_MADI(0x1E)) & 0xF0); // Feat Smooth HLE TH
	reg_m_feat_smooth_shrink = (U32)(REG_RL(REG_SC_MADI(0x1E)) & 0x8F);
	SC_BK_RESTORE;

	if (u32MotionValue >= DEFLICKERING_TH)
	{
		if (s32DeFlickerCnt < 65535)
			s32DeFlickerCnt++;
	}
	else
	{
		s32DeFlickerCnt = 0;
	}

	if (s32DeFlickerCnt >= DEFLICKERING_CNT)
	{
		reg_m_feat_smooth_hle_th += 0x03;
		reg_m_feat_smooth_shrink += 0x10;
	}
	else
	{
		reg_m_feat_smooth_hle_th += 0x07;
		reg_m_feat_smooth_shrink += 0x30;
	}
    SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);
	REG_WL(REG_SC_MADI(0x1E), reg_m_feat_smooth_shrink);
	REG_WH(REG_SC_MADI(0x1E), reg_m_feat_smooth_hle_th);

	SC_BK_RESTORE;
}

//victor 20080922
/******************************************************************************/
///DeBouncing
///@param u32MotionValue \b IN: Motion value
/******************************************************************************/
#define DEBOUNCING_GAIN 1000 //0
void MHal_Scaler_DeBouncing(U32 u32MotionValue)
{
	static S32 s32DeBouncingCnt = 0;

	U32 reg_his_wt_f2;

	U32 reg_debouncing_th;
	int reg_debouncing_cnt;


	reg_debouncing_th = 0x0A;
	reg_debouncing_cnt = 0x03;
    SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);
	reg_his_wt_f2 = (U32)(REG_RL(REG_SC_MADI(0x0A)) & 0xF8); // history ratio weighting
	SC_BK_RESTORE;

	if (u32MotionValue <= reg_debouncing_th * DEBOUNCING_GAIN)
	{
		s32DeBouncingCnt = 0;
	}
	else
	{
		if (s32DeBouncingCnt < 65535)
			s32DeBouncingCnt++;
	}

	if (s32DeBouncingCnt >= reg_debouncing_cnt)
	{
		reg_his_wt_f2 += 0x06; // history = 6 moving
	}
	else
	{
		reg_his_wt_f2 += 0x00; // history = 3 still
	}
    SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);
	REG_WL(REG_SC_MADI(0x0A), reg_his_wt_f2 );  //u32RegELA_3E);
	SC_BK_RESTORE;
}

//fitch 20081222
/******************************************************************************/
///MHal_Scaler_AdaptiveTuning, control the DeFeathering, DeFlickering, DeBouncing
/******************************************************************************/
void MHal_Scaler_AdaptiveTuning(BOOL IsDTV)
{
	U32 u32MotionValue;
	U8 u8Ctrl;

	//
	// Get adaptive function control
	//
	u8Ctrl = MHal_Scaler_GetAdaptiveCtrl();

	//
	// Get motion value
	//
	u32MotionValue = MHal_Scaler_ReadMotionValue1();
	//u32MotionValue2 = MHal_Scaler_ReadMotionValue2();
	//
	// Adaptive functions
	//

	if (u8Ctrl & ENABLE_SCALER_DEFEATHERING)
	{
		MHal_Scaler_DeFeathering(u32MotionValue, IsDTV);
	}

	if (u8Ctrl & ENABLE_SCALER_DEFLICKERING)
	{
		MHal_Scaler_DeFlickering(u32MotionValue);
	}

	if (u8Ctrl & ENABLE_SCALER_DEBOUNCING)
	{
		MHal_Scaler_DeBouncing(u32MotionValue);
	}
}

//From victor
void MHal_Scaler_FilmPatch(U32 u32MotionValue)
{
	U32 reg_debouncing_th;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_FILM);

	reg_debouncing_th = 0x0A;

	if (REG_RI(REG_SC_FILM(0x21), BIT3))
	{
		if (u32MotionValue <= reg_debouncing_th * DEBOUNCING_GAIN)
		{	//Still
			REG_L_WM(REG_SC_FILM(0x21), 0x06, 0x06);
		}
		else
		{	//Moving
			REG_L_WM(REG_SC_FILM(0x21), 0x06, 0x00);
		}
	}
	else
	{
		REG_L_WM(REG_SC_FILM(0x21), 0x06, 0x00);
	}

	SC_BK_RESTORE;
}

#if 1 //godfather99 수정 필요



//victor 20081128, only call this function in ATV source

U8 u8NRdebounce = 0;
U8 u8NRpre_status = 0;

void MHal_SC_DynamicNRInATV()
{

	U8 u8Ctl;
	U8 u8NR_status;
	static U8 Count;
	static U8 u8PreNoiseMagn = 100; //thchen 20081211 for threshold

	SC_PRINT("MHal_SC_DynamicNRInATV\n");

	u8Ctl = REG_RL(REG_AFEC_BANK(0x01)); // get VD Noise Magnitude


	//if (abs(u8PreNoiseMagn - u8Ctl) < 2)//thchen 20081211 for threshold
	//{
	//    return;
	//}

	//printk("\n u8Ctl = %d\n", u8Ctl);
	//printk("\n u8PreNoiseMagn = %d\n", u8PreNoiseMagn);

	if (u8PreNoiseMagn ==100)  //thchen 20081211 for threshold
	{
		u8PreNoiseMagn = u8Ctl;
		Count = 5;
	}

	u8Ctl = (u8PreNoiseMagn + u8Ctl)/2;  //thchen 20081211 for threshold

	if (abs(u8PreNoiseMagn - u8Ctl) > 2)
		u8Ctl = (3*u8PreNoiseMagn + u8Ctl)/4;

	/////////////////////////////////////////////////////////////// to chaeck vertical nosie cshwang080605


	if (u8Ctl <= 3  )
	{
		u8NR_status = 1;
	}
	else if (  u8Ctl >3 && u8Ctl <= 6 )
	{
		u8NR_status = 2;
	}
	else if (  u8Ctl > 6 && u8Ctl < 10 )
	{
		u8NR_status = 3;
	}
	else if ( u8Ctl >= 10 && u8Ctl < 14  )
	{
		u8NR_status = 4;
	}
	else
	{
		u8NR_status = 5;
	}
	if(u8NRpre_status == u8NR_status)
	{
		u8NRdebounce++;
		if (u8NRdebounce > 200)
		{
			u8NRdebounce = 20;
		}
	}
	else
	{
		u8NRdebounce = 0;
		u8NRpre_status = u8NR_status;
	}

	//if (IsATVInUse() )
	if (u8NRdebounce > 9)
	{
	    SC_BK_STORE;
		if (u8Ctl <= 3  )
		{
			SC_BK_SWICH(REG_SC_BK_DNR);
			REG_WH(REG_SC_DNR(0x21), 0x0F);
			//REG_WL(REG_SC_DNR(0x27), 0x2C);//0122 SSY 30->2c
			REG_WL(REG_SC_DNR(0x2D), 0x30);   //1119 Alvin request by LGE
			REG_WH(REG_SC_DNR(0x2D), 0x02);   //1119 Alvin request by LGE

			SC_BK_SWICH(REG_SC_BK_SNR);
			//REG_WL(REG_SC_SNR(0x30), 0x00);
			REG_WL(REG_SC_SNR(0x31), 0x48);

			SC_BK_SWICH(REG_SC_BK_PEAKING);
			REG_WL(REG_SC_PEAKING(0x13), 0x32); //arier_0107 0x00=>0x32
			REG_WL(REG_SC_PEAKING(0x12), 0x81);
			SC_BK_SWICH(REG_SC_BK_ACE);
			REG_WL(REG_SC_PEAKING(0x55), 0xC1);
		}
		else if (  u8Ctl >3 && u8Ctl <= 6 )//0122 SSY RF noise 끌리는 문제 적용.
		{
			SC_BK_SWICH(REG_SC_BK_DNR);
			REG_WH(REG_SC_DNR(0x21), 0x0F);//0122 SSY 4->F
			//REG_WL(REG_SC_DNR(0x27), 0x2C);//0122 SSY 30->2c
			REG_WL(REG_SC_DNR(0x2D), 0x50);  //1119 Alvin request by LGE //SSY 100218 30->10->50
			REG_WH(REG_SC_DNR(0x2D), 0x03);  //1119 Alvin request by LGE //SSY 100218 2->1->3

			SC_BK_SWICH(REG_SC_BK_SNR);
			// REG_WL(REG_SC_SNR(0x30), 0x00);
			REG_WL(REG_SC_SNR(0x31), 0x48);

			SC_BK_SWICH(REG_SC_BK_PEAKING);
			REG_WL(REG_SC_PEAKING(0x13), 0x53); //SSY 100218 42->53
			REG_WL(REG_SC_PEAKING(0x12), 0x83);//SSY 100218 82->83
			SC_BK_SWICH(REG_SC_BK_ACE);
			REG_WL(REG_SC_PEAKING(0x55), 0xC2);//SSY 100218 C1->c2
		}
		else if (  u8Ctl > 6 && u8Ctl < 10 )
		{
			SC_BK_SWICH(REG_SC_BK_DNR);
			REG_WH(REG_SC_DNR(0x21), 0x04);
			//REG_WL(REG_SC_DNR(0x27), 0x30);
			REG_WL(REG_SC_DNR(0x2D), 0x30);  //1119 Alvin request by LGE
			REG_WH(REG_SC_DNR(0x2D), 0x03);  //1119 Alvin request by LGE

			SC_BK_SWICH(REG_SC_BK_SNR);
			//REG_WL(REG_SC_SNR(0x30), 0x01);
			REG_WL(REG_SC_SNR(0x31), 0x49);

			SC_BK_SWICH(REG_SC_BK_PEAKING);
			REG_WL(REG_SC_PEAKING(0x13), 0x53);
			REG_WL(REG_SC_PEAKING(0x12), 0x83);
			SC_BK_SWICH(REG_SC_BK_ACE);
			REG_WL(REG_SC_PEAKING(0x55), 0xC2);
		}
		else if ( u8Ctl >= 10 && u8Ctl < 14  )
		{
			SC_BK_SWICH(REG_SC_BK_DNR);
			REG_WH(REG_SC_DNR(0x21), 0x04);
			//REG_WL(REG_SC_DNR(0x27), 0x30);
			REG_WL(REG_SC_DNR(0x2D), 0x30);  //1119 Alvin request by LGE
			REG_WH(REG_SC_DNR(0x2D), 0x03);  //1119 Alvin request by LGE

			SC_BK_SWICH(REG_SC_BK_SNR);
			//REG_WL(REG_SC_SNR(0x30), 0x01);
			REG_WL(REG_SC_SNR(0x31), 0x4A);

			SC_BK_SWICH(REG_SC_BK_PEAKING);
			REG_WL(REG_SC_PEAKING(0x13), 0x64);
			REG_WL(REG_SC_PEAKING(0x12), 0x83);
			SC_BK_SWICH(REG_SC_BK_ACE);
			REG_WL(REG_SC_PEAKING(0x55), 0xC3);
		}
		else
		{

			SC_BK_SWICH(REG_SC_BK_DNR);

			REG_WH(REG_SC_DNR(0x21), 0x04);
			//REG_WL(REG_SC_DNR(0x27), 0x30);
			REG_WL(REG_SC_DNR(0x2D), 0x30);  //1119 Alvin request by LGE
			REG_WH(REG_SC_DNR(0x2D), 0x03);  //1119 Alvin request by LGE

			SC_BK_SWICH(REG_SC_BK_SNR);
			//REG_WL(REG_SC_SNR(0x30), 0x01);
			REG_WL(REG_SC_SNR(0x31), 0x4C);

			SC_BK_SWICH(REG_SC_BK_PEAKING);
			REG_WL(REG_SC_PEAKING(0x13), 0x94);
			REG_WL(REG_SC_PEAKING(0x12), 0x83);
			SC_BK_SWICH(REG_SC_BK_ACE);
			REG_WL(REG_SC_PEAKING(0x55), 0xC3);
		}
        SC_BK_RESTORE;
		if ((abs(u8PreNoiseMagn - u8Ctl)<2) && (Count==1))
		{
			u8PreNoiseMagn = u8Ctl;  //thchen 20081211 for threshold
			Count=5;
			return;
		}

		Count=(Count-1);
		if (Count<=1)
			Count=1;

		if (u8Ctl <= 2  )
		{
			COMB_BK_STORE;
			COMB_BK_SWICH(0x00);
			REG_WL(REG_COMB_BANK(0x1C), 0x3E);
			//   REG_WH(REG_COMB_BANK(0x41), 0xC2); //FitchHsu 20081217

			COMB_BK_RESTORE;

		}
		else if (  u8Ctl >2 && u8Ctl <= 5 )
		{
			COMB_BK_STORE;
			COMB_BK_SWICH(0x00);
			REG_WL(REG_COMB_BANK(0x1C), 0x1E);
			//   REG_WH(REG_COMB_BANK(0x41), 0xC1);//FitchHsu 20081217

			COMB_BK_RESTORE;

		}
		else if (  u8Ctl >5 && u8Ctl <= 9 )
		{
			COMB_BK_STORE;
			COMB_BK_SWICH(0x00);
			REG_WL(REG_COMB_BANK(0x1C), 0x1E);
			//  REG_WH(REG_COMB_BANK(0x41), 0x41);//FitchHsu 20081217

			COMB_BK_RESTORE;
		}
		else if (  u8Ctl >9 && u8Ctl <= 14 )
		{

			COMB_BK_STORE;
			COMB_BK_SWICH(0x00);
			REG_WL(REG_COMB_BANK(0x1C), 0x1E);
			//    REG_WH(REG_COMB_BANK(0x41), 0x01);//FitchHsu 20081217

			COMB_BK_RESTORE;
		}
		else
		{
			COMB_BK_STORE;
			COMB_BK_SWICH(0x00);
			REG_WL(REG_COMB_BANK(0x1C), 0x1E);
			//   REG_WH(REG_COMB_BANK(0x41), 0x00);//FitchHsu 20081217

			COMB_BK_RESTORE;

		}

		u8PreNoiseMagn = u8Ctl;  //thchen 20081211 for threshold

	}

}
#endif
///-----------------------------------------------------------------------------
/// Menuload driver initialization
///-----------------------------------------------------------------------------
#define MENULOAD_DBG(x)             //x
#define MENULOAD_DISABLE            (0x0000)
#define MENULOAD_ENABLE             (0x8000)
#define MENULOAD_REQ_LEN            (0x4)           // range 0x0 ~ 0xf (must to be power of 2)
#define MENULOAD_FIRE_THRESHOLD     192             // when wcnt > this threshold, padding null register & reg_mload_en=0
// menuload engine can write 255 data / triggle
#define MENULOAD_CMDQUEUE_SIZE      32
static U32 u32MenuLoadBufAddr;
static U32 u32MenuLoadBase;
static U16 u16MenuLoadReadCount;
static U16 u16MenuLoadWriteCount;
static U16 u16MenuLoadMaxWriteCount;
static U8  u8MenuLoadCurrentBank;       //20090827 daniel.huang: for menuload mask

typedef struct
{
	U16 u16Data; // little endian
	U8 u8Addr;
	U8 u8Bank;
	U8 u8Padded[12];
} MENULOAD_DATA;


static MENULOAD_CMD menuload_cmdq[MENULOAD_CMDQUEUE_SIZE];
static MENULOAD_CMD menuload_cmd0; // current command
static U16 u16menuload_cmdq_rdptr;
static U16 u16menuload_cmdq_wtptr;

static void MHal_SC_MLCMQ_Init(void)
{
	U16 i;
	u16menuload_cmdq_rdptr = u16menuload_cmdq_wtptr = 0;
	for (i=0; i<MENULOAD_CMDQUEUE_SIZE; i++)
	{
		menuload_cmdq[i].base_idx = menuload_cmdq[i].depth = 0;
	}
}

static void MHal_SC_MLCMQ_Add(MENULOAD_CMD cmd)
{
	menuload_cmdq[u16menuload_cmdq_wtptr] = cmd;
	u16menuload_cmdq_wtptr++;
	if (u16menuload_cmdq_wtptr == MENULOAD_CMDQUEUE_SIZE)
		u16menuload_cmdq_wtptr = 0;
}

static MENULOAD_CMD MHal_SC_MLCMQ_Remove(void)
{
	MENULOAD_CMD cmd = menuload_cmdq[u16menuload_cmdq_rdptr];
	u16menuload_cmdq_rdptr++;
	if (u16menuload_cmdq_rdptr == MENULOAD_CMDQUEUE_SIZE)
		u16menuload_cmdq_rdptr = 0;
	return cmd;
}

static BOOL MHal_SC_MLCMQ_Full(void)
{
	U16 ptr;
	ptr = u16menuload_cmdq_wtptr+1;
	if (ptr == MENULOAD_CMDQUEUE_SIZE) ptr = 0;
	return (ptr == u16menuload_cmdq_rdptr);
}

BOOL MHal_SC_MLCMQ_Empty(void)
{
	//printk("[cmdq wtptr:%u, rdptr:%u]\n", u16menuload_cmdq_wtptr, u16menuload_cmdq_rdptr);
	return (u16menuload_cmdq_wtptr == u16menuload_cmdq_rdptr);
}


static void MHal_SC_ML_InitVariable(void)
{
	u16MenuLoadReadCount = u16MenuLoadWriteCount = 0;
	u8MenuLoadCurrentBank = 0xFF;

	menuload_cmd0.base_idx = 0;
	menuload_cmd0.depth = 0;
	MHal_SC_MLCMQ_Init();
}

void MHal_SC_ML_Init(U32 u32Addr, U32 u32Size, U8 u8MIU_num)
{
    u32MenuLoadBufAddr = u32Addr;
	u32MenuLoadBase = u32Addr / BYTE_PER_WORD;
	u16MenuLoadMaxWriteCount = u32Size / BYTE_PER_WORD;
	if(u8MIU_num == 1)
	{
	    u32MenuLoadBufAddr += MIPS_MIU1_BASE;
	}
	else
	{
	    u32MenuLoadBufAddr += MIPS_MIU0_BASE;
	}
	MENULOAD_DBG(printk("menuload bufaddr=0x%x, maxwritecnt=%u\n", u32Addr, u16MenuLoadMaxWriteCount));
	MHal_SC_ML_InitVariable();

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_PIP);

	// reg_sel_mload[1:0] 10: output VSync falling edge
	REG_WM(REG_SC_PIP(0x19), (BIT13), (BIT13|BIT12));
#if 1   //20091117 Daniel.Huang: fix 1366x768 with VDE=790 top-left garbage line problem
    REG_WR(REG_SC_PIP(0x1A), 0x0D);
    REG_WR(REG_SC_PIP(0x1B), 0x0F);
#else
    REG_WR(REG_SC_PIP(0x1A), 0x10);
    REG_WR(REG_SC_PIP(0x1B), 0x12);
#endif
	//REG_WR(REG_SC_PIP(0x22), 0x09);
	SC_BK_RESTORE;
//ashton_100520
#if (USE_MENULOAD_SCALER || USE_MENULOAD_PQ)
	MHal_SC_SET_ML_Protect(1);
	msleep(1);
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_SCMI);
	REG_WM(REG_SC_SCMI(0x30), BIT12, BIT12); // enable write through rui0 // daniel +
	SC_BK_RESTORE;

	MHal_SC_SET_ML_Protect(0);
	msleep(1);
#endif
}

void MHal_SC_ML_ChangeBank(U8 u8Data)
{
	u8MenuLoadCurrentBank = u8Data;
	//printk("#####    BK=0x%x\n", u8Data);
}

void MHal_SC_ML_WriteData(U32 u32Addr, U16 u16Data, U16 u16Mask)
{
    volatile MENULOAD_DATA* pMenuloadBuf = (volatile MENULOAD_DATA*)
		(u32MenuLoadBufAddr);


    //printk("#####   MLWriteData: 0x%x, 0x%x\n", u32Addr, u16Data);
	if(u16MenuLoadWriteCount < u16MenuLoadMaxWriteCount)
	{
		pMenuloadBuf[u16MenuLoadWriteCount].u8Bank = u8MenuLoadCurrentBank;
		pMenuloadBuf[u16MenuLoadWriteCount].u8Addr = (u32Addr & 0xFF) >>1;
		SC_BK_STORE
        SC_BK_SWICH(u8MenuLoadCurrentBank);
		pMenuloadBuf[u16MenuLoadWriteCount].u16Data = (REG_RR(u32Addr) & ~u16Mask) | (u16Data & u16Mask);
		SC_BK_RESTORE
		u16MenuLoadWriteCount++;
		menuload_cmd0.depth++; // check menuload_cmd0.depth=0
	}

	assert(u16MenuLoadWriteCount < u16MenuLoadMaxWriteCount);

}

void MHal_SC_ML_WriteNull(void)
{
    volatile MENULOAD_DATA* pMenuloadBuf = (volatile MENULOAD_DATA*)
		(u32MenuLoadBufAddr);
	do // patch for padding register for reg_mload_en = 0
	{
		pMenuloadBuf[u16MenuLoadWriteCount].u8Bank = 0xFF;
		pMenuloadBuf[u16MenuLoadWriteCount].u8Addr = 0x01;
		pMenuloadBuf[u16MenuLoadWriteCount].u16Data= 0x0000;
		u16MenuLoadWriteCount++;
		menuload_cmd0.depth++;
	}
	while(u16MenuLoadWriteCount%MENULOAD_REQ_LEN != 0);
	assert(u16MenuLoadWriteCount < u16MenuLoadMaxWriteCount);
}

void MHal_SC_ML_WriteEnd(MENULOAD_CMD *pCmd)
{
    volatile MENULOAD_DATA* pMenuloadBuf = (volatile MENULOAD_DATA*)
		(u32MenuLoadBufAddr);
	U16 u16CmdLastEntryIndx = pCmd->base_idx + pCmd->depth - 1;
#if 1
	pMenuloadBuf[u16CmdLastEntryIndx].u8Bank = 0x12;  // disable menuload
	pMenuloadBuf[u16CmdLastEntryIndx].u8Addr = 0x2D;  // REG_SC_BK12_2D_L
	pMenuloadBuf[u16CmdLastEntryIndx].u16Data= MENULOAD_DISABLE | (MENULOAD_REQ_LEN<<8) | pCmd->depth;
	MDrv_SYS_Flush_Memory();

    //printk("\n\n");
#else
	U16 i;
	for (i=0; i<4; i++) // write end with flush miu 2 level write fifo
	{
		pMenuloadBuf[u16CmdLastEntryIndx].u8Bank = 0x12;  // disable menuload
		pMenuloadBuf[u16CmdLastEntryIndx].u8Addr = 0x2D;  // REG_SC_BK12_2D_L
		pMenuloadBuf[u16CmdLastEntryIndx].u16Data= MENULOAD_DISABLE | (MENULOAD_REQ_LEN<<8) | pCmd->depth;
		pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[0] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[1] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[2] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[3] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[4] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[5] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[6] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[7] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[8] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[9] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[10] =
			pMenuloadBuf[u16CmdLastEntryIndx].u8Padded[11] = 0x00;
		wmb(); // flush mips write fifo
	}
#endif
}

U16 MHal_SC_ML_GetFreeBufSize(void)
{
	return (u16MenuLoadMaxWriteCount - u16MenuLoadWriteCount);
}

BOOL MHal_SC_ML_IsMenuloadDone(void)
{
	BOOL bDone;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_SCMI);
	bDone = (REG_RR(REG_SC_SCMI(0x2D)) & MENULOAD_ENABLE) ? 0 : 1;
	SC_BK_RESTORE;

	if (!bDone) {
		MENULOAD_DBG(printk("jif:%u, done:0\n", (unsigned int)jiffies));
	}
	return bDone;
}

void MHal_SC_ML_Fire(MENULOAD_CMD *pCmd)
{
	//ashton_100520
	SC_BK_STORE;//ashton_100520
	SC_BK_SWICH(REG_SC_BK_SCMI);
	REG_W3(REG_SC_SCMI(0x2E), (u32MenuLoadBase + pCmd->base_idx));

	REG_WR(REG_SC_SCMI(0x2D), ((MENULOAD_REQ_LEN<<8) | pCmd->depth));
	REG_WR(REG_SC_SCMI(0x2D), (MENULOAD_ENABLE | (MENULOAD_REQ_LEN<<8) | pCmd->depth));
	//REG_WM(REG_SC_SCMI(0x30), BIT12, BIT12); // enable write through riu0 // daniel +

    SC_BK_RESTORE;//ashton_100520
#if 0 // for debug
	{
		volatile MENULOAD_DATA* pMenuloadBuf = (volatile MENULOAD_DATA*)
			(u32MenuLoadBufAddr);
		U16 i;
		printk("[ML idx=%u,depth=%u]\n", pCmd->base_idx, pCmd->depth);
		for (i = pCmd->base_idx; i<pCmd->base_idx + pCmd->depth; i++)
		{
			printk("Data: %04x, Addr: %02x, Bank: %02x\n", pMenuloadBuf[i].u16Data, pMenuloadBuf[i].u8Addr,
					pMenuloadBuf[i].u8Bank);
		}
	}
#endif
}

// This function is designed to be called during VDE for changing menuload base address
// and turning off menuload enable bit
void MHal_SC_ML_UpdateReg(void)
{
	MENULOAD_LOCK;
	if(u16MenuLoadWriteCount == 0)
	{
		MENULOAD_UNLOCK;
		return;
	}

	if(u16MenuLoadWriteCount == u16MenuLoadReadCount)
	{
		MHal_SC_ML_InitVariable();
		MENULOAD_DBG(printk("ML:init\n"));
	}
	else
	{
		MENULOAD_CMD cmd, cmdget;
		if (!MHal_SC_MLCMQ_Empty())
		{
			cmdget = MHal_SC_MLCMQ_Remove();
			cmd.base_idx = cmdget.base_idx;
			cmd.depth = cmdget.depth;
			while (!MHal_SC_MLCMQ_Empty())
			{
				cmdget = MHal_SC_MLCMQ_Remove();
				cmd.depth +=cmdget.depth;
				if (cmd.depth >= MENULOAD_FIRE_THRESHOLD)
				{
					break;
				}
			}

			MHal_SC_ML_WriteEnd(&cmd);
			MHal_SC_ML_Fire(&cmd);
			MENULOAD_DBG(printk("base_idx:%u, depth:%u, rcnt=%u, wcnt=%u\n",
						cmd.base_idx,
						cmd.depth,
						u16MenuLoadReadCount,
						u16MenuLoadWriteCount));

			u16MenuLoadReadCount += cmd.depth;
			//printk("wait trigger\n"); while(1);
		}
	}
	MENULOAD_UNLOCK;
}

void MHal_SC_ML_Start()
{
	U16 u16MenuLoadFreeSize = 0;
	busemenuload_sc = TRUE;

	MENULOAD_LOCK;
	u16MenuLoadFreeSize = MHal_SC_ML_GetFreeBufSize();
	MENULOAD_UNLOCK;

	while(u16MenuLoadFreeSize < 100)
	{
		msleep(20);
		//printk("==> need to increase menuload buffer size\n");

		MENULOAD_LOCK;
		u16MenuLoadFreeSize = MHal_SC_ML_GetFreeBufSize();
		MENULOAD_UNLOCK;
	}
}

void MHal_SC_ML_End()
{
	busemenuload_sc = FALSE;
	MHal_SC_ML_WriteNull();

	while (MHal_SC_MLCMQ_Full())
	{
		MENULOAD_UNLOCK;
		msleep(20);
		//printk("==> need to increase menuload commnad queue size\n");
		MENULOAD_LOCK;
	}

	MHal_SC_MLCMQ_Add(menuload_cmd0);
	menuload_cmd0.base_idx += menuload_cmd0.depth;
	menuload_cmd0.depth = 0;
}

//victor 20081105
//-----------------------------------------------------------------------------
// SSC
//-----------------------------------------------------------------------------
// SPAN value, recommend value is 30KHz ~ 40KHz
// STEP percent value, recommend is under 3%
// recommend value.
// u16Periodx100Hz == 350, u16Percentx100 == 200

#define MST_XTAL_CLOCK_KHZ 12000

void MHal_SC_SetSSC(U16 u16Periodx100Hz, U16 u16Percentx100, BOOL bEnable)
{
	U16 u16Span;
	U16 u16Step;
	U32 u32LPLL_0F= 0;

	u32LPLL_0F = REG_RR(REG_LPLL(0x0F)) | (REG_RL(REG_LPLL(0x10)) << 16);

	// Set SPAN
	if(u16Periodx100Hz < 250 || u16Periodx100Hz > 400) u16Periodx100Hz = 350;

	u16Span = ((U32)MST_XTAL_CLOCK_KHZ/10*15*131072) / (u32LPLL_0F * u16Periodx100Hz / 100);


	// Set STEP
	if(u16Percentx100 > 500)
		u16Percentx100 = 200;

	u16Step = (u32LPLL_0F * u16Percentx100) / ((U32)u16Span * 10000);

	REG_WR(REG_LPLL(0x17), u16Step & 0x03FF);// LPLL_STEP
	REG_WR(REG_LPLL(0x18), u16Span & 0x3FFF);// LPLL_SPAN
	REG_WI(REG_LPLL(0x0D), bEnable, BIT11); // Enable ssc

	SC_PRINT("MHal_SC_SetSSC u16Periodx100Hz : %d, u16Percentx100 : %d, bEnable : %d\n",u16Periodx100Hz,u16Percentx100,bEnable);
}

// FitchHsu 20081121 EMP DVIX issue
static BOOL bEnableOld  = FALSE;
static BOOL bPFastPlayback = FALSE;
static U8 u8BK22_78_L = 0;
static U8 u8BK22_79_L = 0;
static U8 u8BK22_14_L = 0;
static U8 u8BK22_14_H = 0;
static U8 u8BK22_15_L = 0;
static U8 u8BK26_50_L = 0;//victor 20090116, EMP DVIX PQ

// FitchHsu 20081121 EMP DVIX issue
void MHal_SC_PQ_FastPlayback(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);

	if (bEnable == TRUE && bEnableOld == FALSE) {
		bEnableOld = TRUE;
		bPFastPlayback = TRUE;
		u8BK22_78_L = REG_RL(REG_SC_MADI(0x78));
		REG_WL(REG_SC_MADI(0x78), 0x80);   // MADi force weave mode on Y
		u8BK22_79_L = REG_RL(REG_SC_MADI(0x79));
		REG_WL(REG_SC_MADI(0x79), 0x80);   // MADi force weave mode on C
		// apply vertical LPF after MADi
		u8BK22_14_L = REG_RL(REG_SC_MADI(0x14));
		REG_WL(REG_SC_MADI(0x14), 0x80);
		u8BK22_14_H = REG_RH(REG_SC_MADI(0x14));
		REG_WH(REG_SC_MADI(0x14), 0x18);
		u8BK22_15_L = REG_RL(REG_SC_MADI(0x15));
		REG_WL(REG_SC_MADI(0x15), 0x00);

		//victor 20090116, EMP DVIX PQ
		SC_BK_SWICH(REG_SC_BK_DMS);
		u8BK26_50_L = REG_RL(REG_SC_DMS(0x50));
	}

	else if (bEnable == FALSE && bEnableOld == TRUE) {
		bEnableOld = FALSE;
		bPFastPlayback = FALSE;
		REG_WL(REG_SC_MADI(0x78), u8BK22_78_L);
		REG_WL(REG_SC_MADI(0x79), u8BK22_79_L);
		REG_WL(REG_SC_MADI(0x14), u8BK22_14_L);
		REG_WH(REG_SC_MADI(0x14), u8BK22_14_H);
		REG_WL(REG_SC_MADI(0x15), u8BK22_15_L);

		//victor 20090116, EMP DVIX PQ
		SC_BK_SWICH(REG_SC_BK_DMS);
		REG_WL(REG_SC_DMS(0x50), u8BK26_50_L);
	}

	SC_BK_RESTORE;
}

// FitchHsu 20081121 EMP DVIX issue
void MHal_SC_PQ_Protect_FastPlayback()
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);

	if (bPFastPlayback)
	{
		REG_WL(REG_SC_MADI(0x78), 0x80);   // MADi force weave mode on Y
		REG_WL(REG_SC_MADI(0x79), 0x80);   // MADi force weave mode on C
		REG_WL(REG_SC_MADI(0x14), 0x80);
		REG_WH(REG_SC_MADI(0x14), 0x18);
		REG_WL(REG_SC_MADI(0x15), 0x00);

		//victor 20090116, EMP DVIX PQ
		SC_BK_SWICH(REG_SC_BK_DMS);
		REG_WI(REG_SC_DMS(0x50), 0, BIT0);
		REG_WI(REG_SC_DMS(0x50), 0, BIT1);
	}
	SC_BK_RESTORE;
}

//victor 20081113, 3DComb
void MHal_SC_Set3DComb(SC_SET_3DCOMB_t param)
{
	COMB_BK_STORE;
	COMB_BK_SWICH(0x00);
	/*
	   SC_PRINT("u16_24H[0x%x]\n",param.u16_24H);
	   SC_PRINT("u16_2FH[0x%x]\n",param.u16_2FH);
	   SC_PRINT("u16_30L[0x%x]\n",param.u16_30L);
	   SC_PRINT("u16_35H[0x%x]\n",param.u16_35H);
	   SC_PRINT("u16_36H[0x%x]\n",param.u16_36H);
	   SC_PRINT("u16_37L[0x%x]\n",param.u16_37L);
	   SC_PRINT("u16_41H[0x%x]\n",param.u16_41H);
	   SC_PRINT("u16_60H[0x%x]\n",param.u16_60H);
	   SC_PRINT("u16_61L[0x%x]\n",param.u16_61L);
	   SC_PRINT("u16_61H[0x%x]\n",param.u16_61H);
	   SC_PRINT("u16_76H[0x%x]\n",param.u16_76H);
	   */
	REG_WH(REG_COMB_BANK(0x24), param.u16_24H);
	//LGE boae 20090108 REG_WH(REG_COMB_BANK(0x2F), param.u16_2FH);
	REG_WL(REG_COMB_BANK(0x30), param.u16_30L);
	REG_WH(REG_COMB_BANK(0x35), param.u16_35H);
	REG_WH(REG_COMB_BANK(0x36), param.u16_36H);
	REG_WL(REG_COMB_BANK(0x37), param.u16_37L);
	REG_WH(REG_COMB_BANK(0x41), param.u16_41H);
	REG_WH(REG_COMB_BANK(0x60), param.u16_60H);
	REG_WL(REG_COMB_BANK(0x61), param.u16_61L);
	REG_WH(REG_COMB_BANK(0x61), param.u16_61H);
	REG_WH(REG_COMB_BANK(0x76), param.u16_76H);

	COMB_BK_RESTORE;
}

#if 0 // remove this patch because no need in T3
void MHal_SC_DVICLK_UNLOCK(SC_INPUT_SOURCE_e SrcType, BOOL bEnable)
{
	// FitchHsu 20081215 DVICLK UNLOCK issue
	static U8 u8preDVICLK = 0xff;
	U8 u8DVICLK;

	if (bEnable)
	{
		switch(SrcType)
		{
			default:
			case INPUT_SOURCE_HDMI_A:
			case INPUT_SOURCE_HDMI_B:
				u8DVICLK = (REG_RI(REG_ADC_DTOP(0x57), BIT10)) >> 10;
				break;
			case INPUT_SOURCE_HDMI_C:
				u8DVICLK = (REG_RI(REG_ADC_DTOP(0x58), BIT10)) >> 10;
				break;
		}

		if(u8preDVICLK != u8DVICLK)
		{
			u8preDVICLK = u8DVICLK;

			if(u8DVICLK)	//dvi clock is smaller than 70Mhz
			{
				//FitchHsu 20081112 fix DVI clock unlcok issue
				REG_WM(REG_ADC_ATOP(0x64), 0x180, 0x180);
				//printk("\n test 0x180 dvi clock is smaller than 70Mhz %d\n", u8DVICLK);
			}
			else
			{
				//FitchHsu 20081112 fix DVI clock unlcok issue
				REG_WM(REG_ADC_ATOP(0x64), 0x00, 0x180);
				//printk("\n test 0x00 dvi clock is lager than 70Mhz %d\n", u8DVICLK);
			}
			//MHal_SC_Reset(SC_RST_IP_F2);
			//MHal_SC_IP1_ResetSyncDetect();
		}
	}
	else
	{
		//printk("\n Disable 0x00 \n");
		//REG_WM(REG_ADC_ATOP(0x64), 0x00, 0x180);
	}
}
#endif

//20091229 Michu, Implement frame buffer mode
void MHal_SC_SetNumOfFB(U8 u8NumOfFB)
{
    if(busemenuload_sc)
    {
        MHal_SC_ML_ChangeBank(0x12);
        if (u8NumOfFB == 4)
        {
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x03), 0, BIT5|BIT4); // linear address enable
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x04), 0, BIT7);
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x07), BIT13, BIT15|BIT14|BIT13); // 20091229 Michu, read/write bank mapping mode
        }
        else if (u8NumOfFB == 3)
        {
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x03), BIT5|BIT4, BIT5|BIT4);
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x04), BIT7, BIT7);
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x07), BIT14, BIT15|BIT14|BIT13);
        }
        else
        {
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x03), 0, BIT5|BIT4);
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x04), 0, BIT7);
            MHal_SC_ML_WriteData(REG_SC_SCMI(0x07), 0, BIT15|BIT14|BIT13);
        }
    }
    else
    {
        SC_BK_STORE;
        SC_BK_SWICH(REG_SC_BK_SCMI);
        if (u8NumOfFB == 4)
        {
            REG_WM(REG_SC_SCMI(0x03), 0, BIT5|BIT4); // linear address enable
            REG_WI(REG_SC_SCMI(0x04), 0, BIT7);
            REG_WM(REG_SC_SCMI(0x07), BIT13, BIT15|BIT14|BIT13); // 20091229 Michu, read/write bank mapping mode
        }
        else if (u8NumOfFB == 3)
        {
            REG_WM(REG_SC_SCMI(0x03), BIT5|BIT4, BIT5|BIT4);
            REG_WI(REG_SC_SCMI(0x04), BIT7, BIT7);
            REG_WM(REG_SC_SCMI(0x07), BIT14, BIT15|BIT14|BIT13);
        }
        else
        {
            REG_WM(REG_SC_SCMI(0x03), 0, BIT5|BIT4);
            REG_WI(REG_SC_SCMI(0x04), 0, BIT7);
            REG_WM(REG_SC_SCMI(0x07), 0, BIT15|BIT14|BIT13);
        }
        SC_BK_RESTORE;
    }
}

// CC Chen 20081124 MWE implement
//------------------------------------------------------------------------------
//  MWE
//------------------------------------------------------------------------------
void MHal_SC_SubWinEnable(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_PIP);
    REG_WI(REG_SC_PIP(0x10), bEnable, BIT1);// Michu 20090922
	REG_WI(REG_SC_PIP(0x10), bEnable, BIT2);
	SC_BK_RESTORE;
}

void MHal_SC_SetSubDispWin(U16 u16HStart, U16 u16HEnd, U16 u16VStart, U16 u16VEnd)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	REG_WR(REG_SC_S_VOP(0x07), u16HStart);
	REG_WR(REG_SC_S_VOP(0x08), u16HEnd);
	REG_WR(REG_SC_S_VOP(0x09), u16VStart);
	REG_WR(REG_SC_S_VOP(0x0A), u16VEnd);
	SC_BK_RESTORE;
}

void MHal_SC_SetMWEQuality(void)
{
	U8 u8Brightness_R = 0;
	U8 u8Brightness_G = 0;
	U8 u8Brightness_B = 0;

	SC_BK_STORE;

	//////////////////////////////////////
	/// Copy main window setting
	//////////////////////////////////////
	SC_BK_SWICH(REG_SC_BK_PEAKING);

	// Copy H-Peak
	REG_WR(REG_SC_PEAKING(0x14), REG_RR(REG_SC_PEAKING(0x10)));
	REG_WR(REG_SC_PEAKING(0x15), REG_RR(REG_SC_PEAKING(0x11)));
	REG_WR(REG_SC_PEAKING(0x16), REG_RR(REG_SC_PEAKING(0x12)));
	REG_WR(REG_SC_PEAKING(0x17), REG_RR(REG_SC_PEAKING(0x13)));

	REG_WR(REG_SC_PEAKING(0x28), REG_RR(REG_SC_PEAKING(0x18)));
	REG_WR(REG_SC_PEAKING(0x29), REG_RR(REG_SC_PEAKING(0x19)));
	REG_WR(REG_SC_PEAKING(0x2A), REG_RR(REG_SC_PEAKING(0x1A)));
	REG_WR(REG_SC_PEAKING(0x2B), REG_RR(REG_SC_PEAKING(0x1B)));
	REG_WR(REG_SC_PEAKING(0x2C), REG_RR(REG_SC_PEAKING(0x1C)));
	REG_WR(REG_SC_PEAKING(0x2D), REG_RR(REG_SC_PEAKING(0x1D)));
	REG_WR(REG_SC_PEAKING(0x2E), REG_RR(REG_SC_PEAKING(0x1E)));
	REG_WR(REG_SC_PEAKING(0x2F), REG_RR(REG_SC_PEAKING(0x1F)));

	// Copy FCC from main window
	SC_BK_SWICH(REG_SC_BK_ACE);
	REG_WR(REG_SC_ACE(0x11), REG_RR(REG_SC_ACE(0x10)));

	// Copy ICC
	REG_WM(REG_SC_ACE(0x30), (REG_RR(REG_SC_ACE(0x30)) >> 4), (BIT2|BIT3));
	REG_WM(REG_SC_ACE(0x31), (REG_RR(REG_SC_ACE(0x31)) << 4), (BIT4|BIT5|BIT6|BIT7|BIT12|BIT13|BIT14|BIT15));
	REG_WM(REG_SC_ACE(0x32), (REG_RR(REG_SC_ACE(0x32)) << 4), (BIT4|BIT5|BIT6|BIT7|BIT12|BIT13|BIT14|BIT15));
	REG_WM(REG_SC_ACE(0x33), (REG_RR(REG_SC_ACE(0x33)) << 4), (BIT4|BIT5|BIT6|BIT7|BIT12|BIT13|BIT14|BIT15));
	REG_WM(REG_SC_ACE(0x34), (REG_RR(REG_SC_ACE(0x34)) << 4), (BIT4|BIT5|BIT6|BIT7));
	REG_WM(REG_SC_ACE(0x35), (REG_RR(REG_SC_ACE(0x35)) << 8), 0xFF00);

	// IBC
	REG_WR(REG_SC_ACE(0x45), REG_RR(REG_SC_ACE(0x41)));
	REG_WR(REG_SC_ACE(0x46), REG_RR(REG_SC_ACE(0x42)));
	REG_WR(REG_SC_ACE(0x47), REG_RR(REG_SC_ACE(0x43)));
	REG_WM(REG_SC_ACE(0x48), REG_RR(REG_SC_ACE(0x44)), (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5));

	//return;
	// Y/C noise masking
	REG_WM(REG_SC_ACE(0x5D), REG_RR(REG_SC_ACE(0x55)), (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT7|BIT8|BIT9|BIT10|BIT11|BIT12|BIT13|BIT15));
	// IHC
	REG_WM(REG_SC_ACE(0x60), REG_RR(REG_SC_ACE(0x60)) >> 1, BIT6);
	// YC-Coring
	REG_WR(REG_SC_ACE(0x58), REG_RR(REG_SC_ACE(0x50)));
	REG_WR(REG_SC_ACE(0x59), REG_RR(REG_SC_ACE(0x51)));
	REG_WR(REG_SC_ACE(0x5A), REG_RR(REG_SC_ACE(0x52)));
	REG_WR(REG_SC_ACE(0x5B), REG_RR(REG_SC_ACE(0x53)));
	REG_WR(REG_SC_ACE(0x5C), REG_RR(REG_SC_ACE(0x54)));
	// VIP-CSC, [100113_Leo]
	REG_WM(REG_SC_ACE(0x6E), REG_RR(REG_SC_ACE(0x6E)) << 4, BIT4);	
	//return;

	// copy Y-Adjust
	SC_BK_SWICH(REG_SC_BK_DLC);
	REG_WR(REG_SC_DLC(0x0E), REG_RR(REG_SC_DLC(0x0E)) | (REG_RR(REG_SC_DLC(0x0E)) << 8));
	REG_WR(REG_SC_DLC(0x0F), REG_RR(REG_SC_DLC(0x0F)) | (REG_RR(REG_SC_DLC(0x0F)) << 8));
	// copy BLE
	REG_WR(REG_SC_DLC(0x12), REG_RR(REG_SC_DLC(0x10)));
	// copy WLE
	REG_WR(REG_SC_DLC(0x13), REG_RR(REG_SC_DLC(0x11)));
	// copy Y/C gain control -- need to fix
	REG_WR(REG_SC_DLC(0x15), REG_RR(REG_SC_DLC(0x14)));
	// Histogram data
	REG_WR(REG_SC_DLC(0x03), REG_RR(REG_SC_DLC(0x01)));
	// Copy DLC table
	REG_WR(REG_SC_DLC(0x38), REG_RR(REG_SC_DLC(0x30)));
	REG_WR(REG_SC_DLC(0x39), REG_RR(REG_SC_DLC(0x31)));
	REG_WR(REG_SC_DLC(0x3A), REG_RR(REG_SC_DLC(0x32)));
	REG_WR(REG_SC_DLC(0x3B), REG_RR(REG_SC_DLC(0x33)));
	REG_WR(REG_SC_DLC(0x3C), REG_RR(REG_SC_DLC(0x34)));
	REG_WR(REG_SC_DLC(0x3D), REG_RR(REG_SC_DLC(0x35)));
	REG_WR(REG_SC_DLC(0x3E), REG_RR(REG_SC_DLC(0x36)));
	REG_WR(REG_SC_DLC(0x3F), REG_RR(REG_SC_DLC(0x37)));
	// Copy Statistic
	REG_WM(REG_SC_DLC(0x04), REG_RR(REG_SC_DLC(0x04)) >> 1, BIT0);
	// Copy Luma curve
	REG_WM(REG_SC_DLC(0x04), REG_RR(REG_SC_DLC(0x04)) >> 1, BIT6);
	//return;
	// Read Brightness setting from main window
	SC_BK_SWICH(REG_SC_BK_VOP);
	u8Brightness_R = REG_RR(REG_SC_VOP(0x17)) & 0x00FF;
	u8Brightness_G = REG_RR(REG_SC_VOP(0x17)) >> 8;
	u8Brightness_B = REG_RR(REG_SC_VOP(0x18)) & 0x00FF;

	// Gamma
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	//REG_WM(REG_SC_S_VOP(0x18), BIT0, BIT0);
	// Set Brightness
	REG_WM(REG_SC_S_VOP(0x19), u8Brightness_R, 0x00FF);
	REG_WM(REG_SC_S_VOP(0x19), u8Brightness_G << 8, 0xFF00);
	REG_WM(REG_SC_S_VOP(0x1A), u8Brightness_B, 0x00FF);

	// copy Post Scaling filter selection from main to sub
	SC_BK_SWICH(REG_SC_BK_HVSP);
	REG_WR(REG_SC_HVSP(0x2B), REG_RR(REG_SC_HVSP(0x0B)) );

	//return;
	// color matrix
	{
		U16 temp, i;
		for (i=0; i<=9; i++)
		{
			SC_BK_SWICH(REG_SC_BK_VOP);
			temp = REG_RR(REG_SC_VOP((0x26+i)));
			SC_BK_SWICH(REG_SC_BK_S_VOP);
			REG_WR(REG_SC_S_VOP((0x1D+i)), temp);
		}
		REG_WR(REG_SC_S_VOP(0x26), 0x0035);
	}
	//return;

	//////////////////////////////////////
	/// Disable
	//////////////////////////////////////
	SC_BK_SWICH(REG_SC_BK_ACE);
	//disable ICC
	REG_WI(REG_SC_ACE(0x30), 0x0, BIT2);
	//disable IBC
	REG_WI(REG_SC_ACE(0x40), 0x0, BIT6);
	//disable IHC
	REG_WI(REG_SC_ACE(0x60), 0x0, BIT6);
	//disable DLC ( fresh contrast )
	SC_BK_SWICH(REG_SC_BK_DLC);
	REG_WM(REG_SC_DLC(0x04), 0x00, BIT6);
	// main window DLC
	REG_WI(REG_SC_DLC(0x04), 1, BIT7);
	//Set brightness ( Set sub-window brightness )
	//SC_BK_SWICH(REG_SC_S_VOP);
	//REG_WM(REG_SC_VOP(0x19), 0xA0, 0x00FF);   // BCR
	//REG_WM(REG_SC_VOP(0x19), 0xA0, 0x00FF);   // BCG
	//REG_WM(REG_SC_VOP(0x1A), 0xA0, 0x00FF);   // BCB

	SC_BK_RESTORE;
}

void MHal_SC_SetSubWinBorder(BOOL bEnable, U8 u8Color, U8 u8Left, U8 u8Right, U8 u8Top, U8 u8Bottom)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_S_VOP);
	REG_WM(REG_SC_S_VOP(0x17), u8Color, 0x00FF); // Sub Window Border Color
	REG_WM(REG_SC_S_VOP(0x02), u8Left , 0x00FF); // left
	REG_WM(REG_SC_S_VOP(0x03), u8Right, 0x00FF); // right
	REG_WM(REG_SC_S_VOP(0x04), u8Top,   0x00FF); // top
	REG_WM(REG_SC_S_VOP(0x05), u8Bottom,0x00FF); // bottom
	REG_WI(REG_SC_S_VOP(0x01), bEnable, BIT0);
	SC_BK_RESTORE;
}

//LGE gbtogether(081128) --> SD(Film) to HD(non-Film) issue by FitchHsu
void MHal_SC_FilmModeReset(void) //modified, [091012_Leo]
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_FILM);
	REG_WR(REG_SC_FILM(0x0C), 0x00);
	SC_BK_RESTORE;
}

//victor 20081203, DNR
void MHal_SC_EnableDNR(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DNR);
	REG_WI(REG_SC_BANK(0x21), bEnable, BIT1);
	SC_BK_RESTORE;
}

//CDNR Enable, [090615_Leo]
void MHal_SC_EnableColorFunction(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_DNR);
	REG_WI(REG_SC_DNR(0x60), bEnable, BIT0);
	SC_BK_RESTORE;
}

void MHal_SC_SetCDNRIndex(U8 *pIndex)
{
	U8 uCb, uCr;
	SC_BK_STORE;
	SC_PRINT("MHal_SC_SetCDNRIndex\n ");
	SC_BK_SWICH(REG_SC_BK_DNR);
	for(uCr = 0; uCr < 16; uCr ++)
	{
		for(uCb = 0; uCb < 16; uCb ++)
		{
			REG_WL(REG_SC_DNR(0x63), (uCr << 4)|uCb); //set lut0 addr
			REG_WM(REG_SC_DNR(0x62), *(pIndex + uCr*16 + uCb), 0x07); //set lut0 dnr wdata
			REG_WI(REG_SC_DNR(0x61), 1, BIT8); //lut0 write pulse enable
			REG_WI(REG_SC_DNR(0x61), 0, BIT8); //lut0 write pulse clear

			//read data to confirm write successful
			SC_PRINT("addr: 0x%x, data: 0x%x, Cr:%u, Cb:%u\n ", ((uCb<<4)|uCr), REG_RH(REG_SC_DNR(0x62))&0x07, uCr, uCb);
		}
	}
	SC_BK_RESTORE;
}

void MHal_SC_SetCDNRGain(U8 *pDnrGain, U8 *pPreSnrGain)
{
	U8 nIndexNum;
	SC_BK_STORE;
	SC_PRINT("MHal_SC_SetCDNRGain\n ");
	SC_BK_SWICH(REG_SC_BK_DNR);
	for(nIndexNum = 0; nIndexNum < 8; nIndexNum ++)
	{
		REG_WM(REG_SC_DNR(0x66), nIndexNum, 0x07); //set lut1 addr with 'nIndexNum' value
		REG_WM(REG_SC_DNR(0x64), *(pDnrGain + nIndexNum), 0x1F); //set lut1 dnr with '*pDnrGain' value
		REG_WM(REG_SC_DNR(0x64), *(pPreSnrGain + nIndexNum)<<8, 0x1F00); //set lut1 snr with 'pPreSnrGain' value
		REG_WI(REG_SC_DNR(0x61), 1, BIT9); //lut1 write pulse enable
		REG_WI(REG_SC_DNR(0x61), 0, BIT9); //lut1 write pulse clear

		//read data to confirm write successful
		SC_PRINT("addr: 0x%x\n", nIndexNum);
		SC_PRINT("DnrGain = 0x%x, PreSnrGain = 0x%x\n", REG_RL(REG_SC_DNR(0x65))&0x1F, REG_RH(REG_SC_DNR(0x65))&0x1F);
	}
	SC_BK_RESTORE;
}

// FitchHsu 20081218 modify THX mode setting
static U8 u8BK21_1C_L_THX = 0;
static U8 u8BK22_01_H_THX = 0;
static U8 u8BK22_0A_L_THX = 0; //fitch 20081222
static U8 u8BK22_1A_H_THX = 0;
static U8 u8THXTime = 0; //fitch 20081222
void MHal_SC_Set_THXMode(BOOL bIsTHXMode)
{
	static BOOL bAdaptiveDebouncingOn = FALSE;   // fitch 20081222


	if(bIsTHXMode)
	{
		u8THXTime ++;
		if(u8THXTime == 1)
		{
			// backup bAdaptiveDebouncingOn, fitch 20081222
			bAdaptiveDebouncingOn = MHal_Scaler_GetAdaptiveCtrl() & ENABLE_SCALER_DEBOUNCING;

			// disable debouncing, fitch 20081222
			MHal_Scaler_SetAdaptiveCtrl(MHal_Scaler_GetAdaptiveCtrl() & ~ENABLE_SCALER_DEBOUNCING);

            SC_BK_STORE;
	        SC_BK_SWICH(REG_SC_BK_EODI);
			u8BK21_1C_L_THX = REG_RL(REG_SC_EODI(0x1C));
			REG_WL(REG_SC_EODI(0x1C), 0x14); //daten 20081230
			SC_BK_SWICH(REG_SC_BK_MADI);
			u8BK22_01_H_THX = REG_RH(REG_SC_MADI(0x01));
			u8BK22_0A_L_THX = REG_RL(REG_SC_MADI(0x0A));
			u8BK22_1A_H_THX = REG_RH(REG_SC_MADI(0x1A));
			REG_WH(REG_SC_MADI(0x01), 0x0C); //fitch 20081222
			REG_WL(REG_SC_MADI(0x0A), 0x83); //fitch 20081222
			REG_WH(REG_SC_MADI(0x1A), 0x23);
			SC_BK_RESTORE;
		}
		/*	    printk("u8BK21_1C_L_THX = %x\n",u8BK21_1C_L_THX);
				printk("u8BK22_01_H_THX = %x\n",u8BK22_01_H_THX);
				printk("u8BK22_0A_L_THX = %x\n",u8BK22_0A_L_THX); //fitch 20081222
				printk("u8BK22_1A_H_THX = %x\n",u8BK22_1A_H_THX);*/
	}
	else
	{
		if(u8THXTime > 0)
		{
			u8THXTime = 0;
			SC_BK_STORE;
        	SC_BK_SWICH(REG_SC_BK_EODI);
			REG_WL(REG_SC_EODI(0x1C), u8BK21_1C_L_THX);
			SC_BK_SWICH(REG_SC_BK_MADI);
			REG_WH(REG_SC_MADI(0x01), u8BK22_01_H_THX);
			REG_WL(REG_SC_MADI(0x0A), u8BK22_0A_L_THX); //fitch 20081222
			REG_WH(REG_SC_MADI(0x1A), u8BK22_1A_H_THX);
            SC_BK_RESTORE;
			// re-enable deboucing, fitch 20081222
			MHal_Scaler_SetAdaptiveCtrl(MHal_Scaler_GetAdaptiveCtrl() | bAdaptiveDebouncingOn);
			bAdaptiveDebouncingOn = FALSE;
		}
	}

}

//FitchHsu 20081024 Audio Error
BOOL MHal_SC_IP1_GetInputSourceEnable(void)
{
	U8 u8Inputsource;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_IP1F2);
	u8Inputsource = (REG_RL(REG_SC_IP1F2(0x02)));
	SC_BK_RESTORE;

	return (u8Inputsource >> 7);
}


//LGE lemonic 20090121merge //Fitch 20090112 fix aspect Ratio tearing in DTV interlace mode
U8  MHal_SC_Pre_Memory_Motion_Gain(U8 u8MotionGain)
{
    U8 u8PreMotionGain;
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_MADI);
    u8PreMotionGain = REG_RH(REG_SC_MADI(0x08));
    REG_WH(REG_SC_MADI(0x08), u8MotionGain);
	SC_BK_RESTORE;
	return u8MotionGain;
}

//[091201_Leo]
void MHal_SC_DBKInit(void)
{
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_SNR);
    REG_WI(REG_SC_SNR(0x71), TRUE, BIT0);
    SC_BK_RESTORE;
}

//[100118_Leo]
void MHal_SC_SetDithering(U8 u8BitMode, BOOL bUrsapatch)
{
        //--------------------------------------------------------------
        //Depend On Bitmode to set Dither
        //--------------------------------------------------------------
        if(bUrsapatch)
    	{
            SC_BK_STORE;
            SC_BK_SWICH(REG_SC_BK_PAFRC);
            REG_WM(REG_SC_PAFRC(0x3F), 0x05, 0x1D);
    	}
        else
	{

            SC_BK_STORE;
            SC_BK_SWICH(REG_SC_BK_PAFRC);
            
            REG_WM(REG_SC_PAFRC(0x3F), 0x00, 0x08);//bit3, T3 set default 0 to enable
            REG_WM(REG_SC_PAFRC(0x3F), 0x00, 0x10);//bit4, always 0

            switch(u8BitMode & 0x03)//[1:0]ti_bitmode b'10:8bit,  b'11:6bit  b'00:10bit
            {
                case HAL_TI_6BIT_MODE:
                    //printk("ti 6bit mode\n");
                    REG_WM(REG_SC_PAFRC(0x3F), 0x01, 0x01);//bit0, dither enable
                    REG_WM(REG_SC_PAFRC(0x3F), 0x04, 0x04);//bit2, dither bit = 4-bits
                    break;

                case HAL_TI_8BIT_MODE:
                    //printk("ti 8bit mode\n");
                    REG_WM(REG_SC_PAFRC(0x3F), 0x01, 0x01);//bit0, dither enable
                    REG_WM(REG_SC_PAFRC(0x3F), 0x00, 0x04);//bit2, dither bit = 2-bits
                    break;

                case HAL_TI_10BIT_MODE:
                default:
                    //printk("ti 10bit mode\n");
                    REG_WM(REG_SC_PAFRC(0x3F), 0x00, 0x01);//bit0, dither off
                    REG_WM(REG_SC_PAFRC(0x3F), 0x00, 0x04);//bit2, hw default setting, 10bit mode will not refer to this register
                    break;
            }
      }

    SC_BK_RESTORE;
}

void MHal_SC_GetFrameBufInfo(SC_FRAMEBUF_INFO_t *pFrameBufInfo)
{
	U8 u8Reg;
	if (!pFrameBufInfo) return;

	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_SCMI);
	// 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    pFrameBufInfo->u32IPMBase0 = ((U32)REG_RR(REG_SC_SCMI(0x08)) | ((U32)REG_RR(REG_SC_SCMI(0x09)) << 16)) * BYTE_PER_WORD;  // IPMBase0
    pFrameBufInfo->u32IPMBase1 = ((U32)REG_RR(REG_SC_SCMI(0x0A)) | ((U32)REG_RR(REG_SC_SCMI(0x0B)) << 16)) * BYTE_PER_WORD;  // IPMBase1
    pFrameBufInfo->u32IPMBase2 = ((U32)REG_RR(REG_SC_SCMI(0x0C)) | ((U32)REG_RR(REG_SC_SCMI(0x0D)) << 16)) * BYTE_PER_WORD;  // IPMBase2

    pFrameBufInfo->u32OPMBase0 = ((U32)REG_RR(REG_SC_SCMI(0x10)) | ((U32)REG_RR(REG_SC_SCMI(0x11)) << 16)) * BYTE_PER_WORD;  // OPMBase0
    pFrameBufInfo->u32OPMBase1 = ((U32)REG_RR(REG_SC_SCMI(0x12)) | ((U32)REG_RR(REG_SC_SCMI(0x13)) << 16)) * BYTE_PER_WORD;  // OPMBase1
    pFrameBufInfo->u32OPMBase2 = ((U32)REG_RR(REG_SC_SCMI(0x14)) | ((U32)REG_RR(REG_SC_SCMI(0x15)) << 16)) * BYTE_PER_WORD;  // OPMBase2

	pFrameBufInfo->u16IPMOffset = REG_RR(REG_SC_SCMI(0x0E));
	pFrameBufInfo->u16IPMFetch = REG_RR(REG_SC_SCMI(0x0F));                         // 20090922 daniel.huang: for memory protection
	pFrameBufInfo->bLinearAddrMode = (REG_RR(REG_SC_SCMI(0x03)) & BIT4) ?TRUE:FALSE;//20090813 michu
	pFrameBufInfo->bYCSeperate     = (REG_RR(REG_SC_SCMI(0x02)) & BIT10)?TRUE:FALSE;//20090813 michu

    pFrameBufInfo->bh_mirror = (REG_RR(REG_SC_SCMI(0x03)) & BIT12) ? TRUE:FALSE;    // 20090921 daniel.huang: for mirror mode
    pFrameBufInfo->bv_mirror = (REG_RR(REG_SC_SCMI(0x03)) & BIT13) ? TRUE:FALSE;    // 20090921 daniel.huang: for mirror mode

	u8Reg = REG_RL(REG_SC_SCMI(0x01));
	SC_BK_RESTORE;
	if (u8Reg & 0x30) {//444 //20090813 michu
		pFrameBufInfo->bMemFormat422 = FALSE;
		if (u8Reg & 0x10) { // 444 8BIT
			pFrameBufInfo->u8BitPerPixel = 24;
			pFrameBufInfo->bInterlace = FALSE;  // 2009/09/23 daniel.huang: for calibration with mirror
		}
		else { // 444 10BIT
			pFrameBufInfo->u8BitPerPixel = 32;
			pFrameBufInfo->bInterlace = FALSE;  // 2009/09/23 daniel.huang: for calibration with mirror
		}
	}
	else {//422
		pFrameBufInfo->bMemFormat422 = TRUE;
		SC_BK_STORE;
	    SC_BK_SWICH(REG_SC_BK_SCMI);
		u8Reg = REG_RH(REG_SC_SCMI(0x01));
		SC_BK_RESTORE;
		switch(u8Reg & 0x77)
		{
			case 0x25:
				pFrameBufInfo->u8BitPerPixel = 24;
				pFrameBufInfo->bInterlace = TRUE;   // 2009/09/23 daniel.huang: for calibration with mirror
				break;
			case 0x27:
				pFrameBufInfo->u8BitPerPixel = 16;
				pFrameBufInfo->bInterlace = TRUE;   // 2009/09/23 daniel.huang: for calibration with mirror
				break;
			case 0x21:
				pFrameBufInfo->u8BitPerPixel = 24;
				pFrameBufInfo->bInterlace = FALSE;  // 2009/09/23 daniel.huang: for calibration with mirror
				break;
			case 0x00:
				pFrameBufInfo->u8BitPerPixel = 16;
				pFrameBufInfo->bInterlace = FALSE;  // 2009/09/23 daniel.huang: for calibration with mirror
				break;
			default:
				assert(0);
				if (MHal_SC_IP1_GetDetectSyncStatus() & BIT11) // interlace input
					pFrameBufInfo->bInterlace = TRUE;
				else
					pFrameBufInfo->bInterlace = FALSE;
				break;
		}
	}

#if 0   // 2009/09/23 daniel.huang: for calibration with mirror
    if (MHal_SC_IP1_GetDetectSyncStatus() & BIT11) // interlace input
        pFrameBufInfo->bInterlace = TRUE;
	else
        pFrameBufInfo->bInterlace = FALSE;
#endif
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_PIP);
    pFrameBufInfo->u16LBOffset =  REG_RR(REG_SC_PIP(0x1D)) & 0xFF;  // 20090905 daniel.huang: for DDI_VIDEO_GrabPixels
    SC_BK_RESTORE;    // 20091111 daniel.huang: fix lock twice with caling MHal_SC_IP2_GetFrameLineCnt() in SMP

    pFrameBufInfo->u16FrameLineCnt = MHal_SC_IP2_GetFrameLineCnt();

	SC_PRINT("base0=0x%x, base1=0x%x, offset=%u, frame_line_cnt=%u, 422=%u, i=%u, bpp=%u, linear_md=%u, yc_sep=%u, h_mirror=%u, v_mirror=%u\n",
			(unsigned int)pFrameBufInfo->u32IPMBase0,
			(unsigned int)pFrameBufInfo->u32IPMBase1,
			pFrameBufInfo->u16IPMOffset,
			pFrameBufInfo->u16FrameLineCnt,
			pFrameBufInfo->bMemFormat422,
			pFrameBufInfo->bInterlace,
			pFrameBufInfo->u8BitPerPixel,
			pFrameBufInfo->bLinearAddrMode,
			pFrameBufInfo->bYCSeperate,
			pFrameBufInfo->bh_mirror,   // 20090921 daniel.huang: for mirror mode
			pFrameBufInfo->bv_mirror);  // 20090921 daniel.huang: for mirror mode

}

void MHal_SC_SetFastFrameModeStatus(BOOL bEnable)
{
	bIsFastFrameLock = bEnable;
}

BOOL MHal_SC_GetFastFrameModeStatus(void)
{
	return bIsFastFrameLock;
}

//#if REFINE_FPLL
BOOL MHal_SC_IPM_GetFBL(void)
{
    BOOL bEnable;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_SCMI);
    bEnable = REG_RR(REG_SC_SCMI(0x01)) & BIT7;
    SC_BK_RESTORE;
    return bEnable;
}

U16 MHal_SC_IP1_GetVDC(void)
{
    U16 u16VSize;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    u16VSize = REG_RR(REG_SC_IP1F2(0x06));
    SC_BK_RESTORE;
    return u16VSize;
}

U16 MHal_SC_VOP_GetDispVStart(void)
{
    U16 u16VSt;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_VOP);
    u16VSt = REG_RR(REG_SC_VOP(0x0A)) & 0xFFF;
    SC_BK_RESTORE;
    return u16VSt;
}

U16 MHal_SC_VOP_GetDispVEnd(void)
{
    U16 u16VEnd;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_VOP);
    u16VEnd = REG_RR(REG_SC_VOP(0x0B)) & 0xFFF;
    SC_BK_RESTORE;
    return u16VEnd;
}

// 20090921 daniel.huang: for mirror mode
U16 MHal_SC_VOP_GetDispHStart(void)
{
    U16 u16HSt;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_VOP);
    u16HSt = REG_RR(REG_SC_VOP(0x08)) & 0xFFF;
    SC_BK_RESTORE;
    return u16HSt;
}

// 20090921 daniel.huang: for mirror mode
U16 MHal_SC_VOP_GetDispHEnd(void)
{
    U16 u16HEnd;
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_VOP);
    u16HEnd = REG_RR(REG_SC_VOP(0x09)) & 0xFFF;
    SC_BK_RESTORE;
    return u16HEnd;
}

// 20090921 daniel.huang: for mirror mode
// get size after pre-v scaling down
U16 MHal_SC_IP2_GetFrameLineCnt(void)
{
    U32 u32Ratio;
    U16 u16VCapSize;
    U16 u16FrameLineCnt;
    BOOL bScalingEnable;

    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    u16VCapSize = REG_RR(REG_SC_IP1F2(0x06));

    SC_BK_SWICH(REG_SC_BK_IP2F2);
    u32Ratio = (U32)REG_RR(REG_SC_IP2F2(0x08)) | ((U32)REG_RR(REG_SC_IP2F2(0x09)) << 16);
    SC_BK_RESTORE;

    bScalingEnable = (u32Ratio & 0x80000000L) ? 1:0;
    u32Ratio = u32Ratio & 0xFFFFFL;

    // u32Ratio = ((((U32)Output) * 1048576ul) / (Input) + 1)
    if (u32Ratio != 0)
    {
        u16FrameLineCnt = (u32Ratio - 1) * u16VCapSize / 1048576ul;
    }
    else
    {
        u16FrameLineCnt = u16VCapSize;
    }
    return u16FrameLineCnt;
}


//#endif

//------------------------------------------------------------------------------
// VIDEO MIRROR
// Michu 20090903
//------------------------------------------------------------------------------
void MHal_SC_Set_VideoMirror(BOOL bIsH, BOOL bIsV)
{
    SC_BK_STORE;
    //SC_BK_SWICH(REG_SC_BK_IP1F2);
    //REG_WM(REG_SC_IP1F2(0x02), BIT7, BIT7);

    SC_BK_SWICH(REG_SC_BK_SCMI);
    REG_WI(REG_SC_SCMI(0x03), bIsH, BIT12);
    REG_WI(REG_SC_SCMI(0x03), bIsV, BIT13);
    SC_BK_RESTORE;
}

void MHal_SC_SetVideoMirrorAlignWidth(U8 u8AlignWidth)
{
  	if(busemenuload_sc)
	{
		MHal_SC_ML_ChangeBank(0x02);
		MHal_SC_ML_WriteData(REG_SC_IP2F2(0x2A), (u8AlignWidth << 8), 0xFF00);
	}
	else
	{
		SC_BK_STORE;
        SC_BK_SWICH(REG_SC_BK_IP2F2);
        REG_WH(REG_SC_IP2F2(0x2A), u8AlignWidth);
        SC_BK_RESTORE;
	}
}
//------------------------------------------------------------------------------
// End of VIDEO MIRROR
//------------------------------------------------------------------------------

// 20091021 daniel.huang: add ipmux test pattern for inner test pattern
void MHal_SC_IPMUX_SetTestPattern(BOOL bEnable, U16 u16R_Cr, U16 u16G_Y, U16 u16B_Cb)
{
    if (bEnable)
    {
        REG_WM(REG_IPMUX(0x10), (BIT12|BIT13|BIT14), (BIT12|BIT13|BIT14));   // select r_cr, g_y, b_cb data
        REG_WM(REG_IPMUX(0x1D), u16R_Cr, 0x3FF);   // r_cr data
        REG_WM(REG_IPMUX(0x1E), u16G_Y , 0x3FF);   // g_y  data
        REG_WM(REG_IPMUX(0x1F), u16B_Cb, 0x3FF);   // b_cb data
        REG_WM(REG_IPMUX(0x10), BIT9, BIT9);    // test pattern enable
    }
    else
    {
        REG_WM(REG_IPMUX(0x10),    0, BIT9);    // test pattern disable
    }
}

void MHal_SC_VOP_OD_DataPath(BOOL bEnable)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP);

    REG_WI(REG_SC_VOP(0x5B),  bEnable, BIT10);

	SC_BK_RESTORE;
}


void MHal_SC_WriteRegMask(U8 u8Bank, U32 u32Addr, U16 u16Value, U16 u16Mask)
{
    SC_BK_STORE;
    SC_BK_SWICH(u8Bank);
    REG_WM(u32Addr, u16Value, u16Mask);
    SC_BK_RESTORE;
}

EXPORT_SYMBOL(MHal_SC_WriteRegMask);

void MHal_SC_OSD_Reference(U16 u16Height)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_VOP2_RP);

    REG_WI(REG_SC_VOP2_RP(0x10),  1, BIT15);
    REG_WM(REG_SC_VOP2_RP(0x10), 0x00, 0xFFF);
    REG_WM(REG_SC_VOP2_RP(0x11), 0x01, 0xFFF);
    REG_WM(REG_SC_VOP2_RP(0x12), 0x01, 0xFFF);
    REG_WM(REG_SC_VOP2_RP(0x13), u16Height +2, 0xFFF);

	SC_BK_RESTORE;
}

void MHal_SC_2D_Peaking_LBS(void)
{
	SC_BK_STORE;
	SC_BK_SWICH(REG_SC_BK_PEAKING);

    REG_WI(REG_SC_PEAKING(0x10),  1, BIT7);

	SC_BK_RESTORE;
}

//(20100115 ykkim5) 20100113 daniel.huang: for solve sog sync unstable problem for some dvd-player with 480i timing
void MHal_SC_SetMacroVisionFilter(BOOL bEnable)
{    
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    if (bEnable)
        REG_WM(REG_SC_IP1F2(0x35),  BIT12, (BIT13|BIT12)); // 01: replace input Hsync by re-generated Hsync signal to filter out Macrovison or glitch in the Coast region.
    else
        REG_WM(REG_SC_IP1F2(0x35),      0, (BIT13|BIT12));
    SC_BK_RESTORE;
}

void MHal_SC_SetFD_Mask(BOOL bEnable)
{    
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_IP1F2);
    
    REG_WI(REG_SC_IP1F2(0x2F), bEnable, BIT4);

    SC_BK_RESTORE;
}
//ashton_100520
void MHal_SC_SET_ML_Protect(BOOL bEnable)
{
	if(bEnable)
	{
		REG_WM(0x100104, BIT4, BIT4);
	}
	else
	{
		REG_WM(0x100104, 0x00, BIT4);
	}
}
