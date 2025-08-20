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
/// file    regScaler.h
/// @brief  Scaler Module Register Definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _REG_SCALER_H_
#define _REG_SCALER_H_


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define REG_GENERAL_BASE            0x100000

// Scaler
#define REG_SCALER_BASE             0x102F00
//#define SC_REG(addr)                (*((volatile U16*)(0xBF800000 + (addr << 2))))

// IP Mux
#define REG_IPMUX_BASE              0x102E00
//#define IPMUX_REG(addr)             (*((volatile U16*)(0xBF800000 + (addr << 2))))

// Ckgen0
#define REG_CKGEN0_BASE             0x100B00

//#define CHIPTOP_REG(addr)           (*((volatile U16*)(0xBF800000 + (addr << 2))))

// AFEC
#define REG_AFEC_BASE               0x103500 // FitchHsu 20081216 AutoNR

// Comb
#define REG_COMB_BASE               0x103600
//#define COMB_REG(addr)                (*((volatile U16*)(0xBF800000 + (addr << 2))))
//victor 20081215, HDMI audio
//Audio
#define REG_VIVALDI_BASE            0x102C00

// SAR
#define REG_PM_SAR_BASE             0x001400

// PM SLEEP
#define REG_PM_SLEEP_BASE           0x000E00


//
// Scaler bank
//
#define REG_SC_BK_GOPINT            0x00
#define REG_SC_BK_IP1F2             0x01
#define REG_SC_BK_IP2F2             0x02
#define REG_SC_BK_IP1F1             0x03
#define REG_SC_BK_IP2F1             0x04
#define REG_SC_BK_OPM               0x05
#define REG_SC_BK_DNR               0x06


#define REG_SC_BK_FILM              0x0A
#define REG_SC_BK_ELA               0x0B
#define REG_SC_BK_SNR               0x0C
#define REG_SC_BK_HD                0x0E
#define REG_SC_BK_S_VOP             0x0F
#define REG_SC_BK_VOP               0x10
#define REG_SC_BK_TCON              0x11
#define REG_SC_BK_SCMI              0x12
#define REG_SC_BK_OD                0x16
#define REG_SC_BK_SRAM              0x17
#define REG_SC_BK_ACE               0x18
#define REG_SC_BK_PEAKING           0x19
#define REG_SC_BK_DLC               0x1A
#define REG_SC_BK_OP1PIPEXT         0x1B
#define REG_SC_BK_MEMC              0x1D
#define REG_SC_BK_PIP               0x20
#define REG_SC_BK_EODI              0x21
#define REG_SC_BK_MADI              0x22
#define REG_SC_BK_HVSP              0x23
#define REG_SC_BK_PAFRC             0x24//[100118_Leo]
#define REG_SC_BK_XVYCC             0x25
#define REG_SC_BK_DMS               0x26
#define REG_SC_BK_ACE2              0x27
#define REG_SC_BK_PRED              0x28
#define REG_SC_BK_VOP2_RP           0x2D

#define REG_CHIPTOP_BASE            0x101E00
#define REG_CHIPTOP(_x_)            (REG_CHIPTOP_BASE | (_x_ << 1))

#define REG_GOP_BANK(_x_)           (0x101F00 | (_x_ << 1))


//
// scaler register definition
//
#define REG_SC_BANK(_x_)            (REG_SCALER_BASE | (_x_ << 1))
#define REG_SC_GOPINT(_x_)          REG_SC_BANK(_x_)
#define REG_SC_IP1F2(_x_)           REG_SC_BANK(_x_)
#define REG_SC_IP2F2(_x_)           REG_SC_BANK(_x_)
#define REG_SC_IP1F1(_x_)           REG_SC_BANK(_x_)
#define REG_SC_IP2F1(_x_)           REG_SC_BANK(_x_)
#define REG_SC_OPM(_x_)             REG_SC_BANK(_x_)
#define REG_SC_DNR(_x_)             REG_SC_BANK(_x_)


#define REG_SC_FILM(_x_)            REG_SC_BANK(_x_)
#define REG_SC_ELA(_x_)             REG_SC_BANK(_x_)
#define REG_SC_SNR(_x_)             REG_SC_BANK(_x_)
#define REG_SC_HD(_x_)              REG_SC_BANK(_x_)
#define REG_SC_S_VOP(_x_)           REG_SC_BANK(_x_)
#define REG_SC_VOP(_x_)             REG_SC_BANK(_x_)
#define REG_SC_TCON(_x_)            REG_SC_BANK(_x_)
#define REG_SC_SCMI(_x_)            REG_SC_BANK(_x_)
#define REG_SC_OD(_x_)              REG_SC_BANK(_x_)
#define REG_SC_SRAM(_x_)            REG_SC_BANK(_x_)
#define REG_SC_ACE(_x_)             REG_SC_BANK(_x_)
#define REG_SC_PEAKING(_x_)         REG_SC_BANK(_x_)
#define REG_SC_DLC(_x_)             REG_SC_BANK(_x_)
#define REG_SC_OP1PIPEXT(_x_)       REG_SC_BANK(_x_)
#define REG_SC_MEMC(_x_)            REG_SC_BANK(_x_)
#define REG_SC_PIP(_x_)             REG_SC_BANK(_x_)
#define REG_SC_EODI(_x_)            REG_SC_BANK(_x_)
#define REG_SC_MADI(_x_)            REG_SC_BANK(_x_)
#define REG_SC_HVSP(_x_)            REG_SC_BANK(_x_)
#define REG_SC_PAFRC(_x_)           REG_SC_BANK(_x_)//[100118_Leo]
#define REG_SC_XVYCC(_x_)           REG_SC_BANK(_x_)
#define REG_SC_DMS(_x_)             REG_SC_BANK(_x_)
#define REG_SC_ACE2(_x_)            REG_SC_BANK(_x_)
#define REG_SC_PRED(_x_)            REG_SC_BANK(_x_)
#define REG_SC_VOP2_RP(_x_)         REG_SC_BANK(_x_)


//victor 20081215, HDMI audio
//
// Audio register definition
//
#define REG_VIVALDI_BANK(_x_)       (REG_VIVALDI_BASE | (_x_ << 1))
//
// comb register definition
//
#define REG_COMB_BANK(_x_)          (REG_COMB_BASE | (_x_ << 1))

// FitchHsu 20081216 AutoNR
//
// AFEC register definition
//

#define REG_AFEC_BANK(_x_)          (REG_AFEC_BASE | (_x_ << 1))

//
// IPMux register definition
//
#define REG_IPMUX(_x_)              (REG_IPMUX_BASE | (_x_ << 1))

//
// Ckgen0 register definition
//
#define REG_CKGEN0(_x_)             (REG_CKGEN0_BASE | (_x_ << 1))

#define REG_GENERAL_BANK(_x_)       (REG_GENERAL_BASE | _x_)

//
// SAR register definition
//
#define REG_PM_SAR(x)               (REG_PM_SAR_BASE   | ((x) << 1))

//
// PM SLEEP register definition
//
#define REG_PM_SLEEP(x)             (REG_PM_SLEEP_BASE | ((x) << 1))


#endif // _REG_SCALER_H_

