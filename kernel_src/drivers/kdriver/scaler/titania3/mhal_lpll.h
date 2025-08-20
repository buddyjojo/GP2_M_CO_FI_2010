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

#ifndef _HAL_LPLL_H_
#define _HAL_LPLL_H_


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//
// register operation
//

// write 2 bytes
#define LPLL_WR(_x_, _y_) (_x_ = _y_)

// read 2 bytes
#define LPLL_RR(_x_)  _x_

// write low byte
#define LPLL_WL(_x_, _y_)     \
        _x_ & 0xFF00; \
        _x_ |= (_y_ & 0xFF00)

// write high byte
#define LPLL_WH(_x_, _y_)     \
        _x_ & 0x00FF  \
        _x_ |= (_y_ & 0x00FF)

#if 0
// write 2 bytes with mask
#define LPLL_WM(_x_, _y_, _z_)  \
        _x_ & ~(_z_)            \
        _x_ = (_y_ & _z_)
#endif

// write bit
#define LPLL_WI(_reg_, _bit_, _pos_)    \
        _reg_ = (_bit_) ? (_reg_ | _pos_) : (_reg_ & ~(_pos_))

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------
/// LPLL init
typedef struct
{
    U16 u16InputDiv;
    U16 u16LoopDiv;
    U16 u16OutputDiv;
    U8  u8Type;         // 0: LVDS type, 1: RSDS type, 2, TCON	// shjang_090904
    U8  u8Mode;         // 0: single mode, 1: dual mode
    U32 u32LimitD5d6d7; //thchen 20081216
	U16 u16LimitOffset; //thchen 20081216
} HAL_LPLL_INIT_t;

// MHal_LPLL_SetFrameDiv
typedef enum
{
    LPLL_FRC_1_1,
    LPLL_FRC_1_2,
    LPLL_FRC_1_4,
    LPLL_FRC_2_5,
    LPLL_FRC_2_10,
    LPLL_FRC_5_12,
    LPLL_FRC_5_24,
} HAL_LPLL_FRC_e;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MHal_LPLL_Init(HAL_LPLL_INIT_t* pInit);
void MHal_LPLL_Init4PDP(HAL_LPLL_INIT_t* pInit);	//LGE [vivakjh] 2008/11/12 	Add for DVB PDP Panel
void MHal_LPLL_Init4NonFRC(HAL_LPLL_INIT_t* pInit);
void MHal_LPLL_LPLLSET(U32 u32LPLLSET);
void MHal_LPLL_LimitD5d6d7(U32 u32LimitD5d6d7); //thchen 20090118 temp solution for ATSC frame lock

#if 1 //thchen 20080829 merged by LGE drmyung 080910
void MHal_LPLL_SetFrameSync(U8 u8FrcIn, U8 u8FrcOut); //thchen 20080829
void MHal_LPLL_GetFrameSync(U8 *u8FrcIn, U8 *u8FrcOut); // 20091006 daniel.huang: fix dclk setting incorrect under mini-LVDS, and refine i/p gain
void MHal_LPLL_SetIPGain(U8 u8IGain, U8 u8PGain); //thchen 20080829
#else
void MHal_LPLL_SetFrameDiv(HAL_LPLL_FRC_e frc);
void MHal_LPLL_SetIPGain(HAL_LPLL_FRC_e frc, BOOL bPnlDblVSync);
#endif
void MHal_LPLL_EnableFPLL(BOOL bEnable);
BOOL MHal_LPLL_IsFRC(void);                             //victor 20081210
U16 MHal_LPLL_GetPhaseDiff(void);                       //victor 20081210
void MHal_LPLL_DisableIGainForLockPhase(BOOL bEnable);  //victor 20081210
BOOL MHal_LPLL_IsPhaseLockDone(void);                   //victor 20081210
void MHal_LPLL_MODSET(U8 u8LPLL_Type, U8 u8LPLL_Mode);
BOOL MHal_LPLL_GetFrameLock_Status(void); // LGE [vivakjh]  2008/12/11 Merge!!  FitchHsu 20081209 implement frame lock status report
void MHal_LPLL_SetFrameLockSpeedToZero(BOOL bIsSpeedZero);	// LGE [vivakjh] 2009/01/21	Modified the PDP module flicker problem after playing some perticular movie files that are over the PDP module margin.
U32 MHal_LPLL_GetIvsPrd(void); // FitchHsu 20090604 SpeedUp frame lock for TW model in YPbPr and HDMI
U32 MHal_LPLL_GetOvsPrd(void); // FitchHsu 20090604 SpeedUp frame lock for TW model in YPbPr and HDMI
void MHal_LPLL_Measure_FrameLockSpeed(void); // for debug

#endif // _HAL_LPLL_H_
