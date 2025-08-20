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


#ifndef _MHal_PWS_H_
#define _MHal_PWS_H_

////////////////////////////////////////////////////////////////////////////////
/// @file mhal_pws.h
/// @author MStar Semiconductor Inc.
/// @brief power saving hal
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Header Files
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Define & data type
//------------------------------------------------------------------------------
#define PWS_ABSO_MEM_BASE 0xA0000000
//------------------------------------------------------------------------------
// Extern function
//------------------------------------------------------------------------------
void MHal_PWS_WriteReg(U32 u32Addr, U8 u8Value);
U32 MHal_PWS_ReadReg(U32 u32Addr);
U8 MHal_PWS_WriteRegBit(U32 u32RegAddr, U8 bEnable, U8 u8BitPos);
void MHal_PWS_WriteRegMask(U32 u32RegAddr, U8 u8Val, U8 u8Mask);

U32 MHal_ATV_ReadReg(U32 u32Reg);
void MHal_ATV_WriteReg( U32 u32Reg, U8 u8Val);
void MHal_ATV_WriteRegBit(U32 u32Reg, U8 bEnable, U8 u8Mask );
U32 MHal_DVB_ReadByte(U32 u32Addr);
void MHal_DVB_WriteByte(U32 u32Addr, U8 u8Data);
void MHal_DVB_WriteByteBit( U16 u16Reg, U8 bEnable, U8 u8Mask );
void MHal_ATSC_WriteReg(U32 u32Addr, U8 u8Data);
U32 MHal_ATSC_ReadReg(U32 u32Addr);
void MHal_ATSC_WriteRegBit( U32 u32Reg, U8 bEnable, U8 u8Mask );
U32 MHal_PWS_GetChipId(void);
void MHal_PWS_SetIOMapBase(U32 u32Base);
U32 MHal_PWS_GetIOMapBase(void);
void MHal_PWS_Init(void);
void MHal_PWS_SetDviDmDemux(U8 OnOff_flag);

void MHal_PWS_SetDviPreamp(U8 OnOff_flag);
void MHal_PWS_SetDviBist(U8 OnOff_flag);
void MHal_PWS_SetDviDmRxckBist(U8 OnOff_flag);
void MHal_PWS_SetDviDmPreclk(U8 OnOff_flag);
void MHal_PWS_SetDviDmPreclkOffl(U8 OnOff_flag);
void MHal_PWS_SetDviDmRbias(U8 OnOff_flag);
void MHal_PWS_SetDviDmEnvdet(U8 OnOff_flag);
void MHal_PWS_SetDviPllCore(U8 OnOff_flag);
void MHal_PWS_SetDviPllRegm(U8 OnOff_flag);
void MHal_PWS_SetDviPllPhdac(U8 OnOff_flag);
void MHal_PWS_SetSingalPadRArray(U8 OnOff_flag);
void MHal_PWS_SetVifBandgapIbiasVref(U8 OnOff_flag);
void MHal_PWS_SetVifCalibrationBuffer(U8 OnOff_flag);
void MHal_PWS_SetVifClampBuffer(U8 OnOff_flag);
void MHal_PWS_SetVifAdcI(U8 OnOff_flag);
void MHal_PWS_SetVifAdcQ(U8 OnOff_flag);
void MHal_PWS_SetVifPga1(U8 OnOff_flag);
void MHal_PWS_SetVifPga2(U8 OnOff_flag);
void MHal_PWS_SetVifMpllReg(U8 OnOff_flag);
void MHal_PWS_SetVifAdcOutClkPd(U8 OnOff_flag);
void MHal_PWS_SetVifMpll_div2_pd(U8 OnOff_flag);
void MHal_PWS_SetVifMpll_div3_pd(U8 OnOff_flag);
void MHal_PWS_SetVifMpll_div4_pd(U8 OnOff_flag);
void MHal_PWS_SetVifMpll_div8_pd(U8 OnOff_flag);
void MHal_PWS_SetVifMpll_div10_pd(U8 OnOff_flag);
void MHal_PWS_SetVifTagc(U8 OnOff_flag);
void MHal_PWS_SetUsbHsTx(U8 OnOff_flag);
void MHal_PWS_SetUsbHsRx(U8 OnOff_flag);
void MHal_PWS_SetUsbFlXcvr(U8 OnOff_flag);
void MHal_PWS_SetUsbSerdes(U8 OnOff_flag);
void MHal_PWS_SetUsbPllLdo(U8 OnOff_flag);
void MHal_PWS_SetUsbLdo(U8 OnOff_flag);
void MHal_PWS_SetUsbRefBiasCir(U8 OnOff_flag);
void MHal_PWS_SetAdcR(U8 OnOff_flag);
void MHal_PWS_SetAdcG(U8 OnOff_flag);
void MHal_PWS_SetAdcB(U8 OnOff_flag);
void MHal_PWS_SetAdcPhdig(U8 OnOff_flag);
void MHal_PWS_SetAdcPllA(U8 OnOff_flag);
void MHal_PWS_SetAdcIclampRgb(U8 OnOff_flag);
void MHal_PWS_SetAdcIclampVdy(U8 OnOff_flag);
void MHal_PWS_SetAdcIclampVdc(U8 OnOff_flag);
void MHal_PWS_SetAdcY(U8 OnOff_flag);
void MHal_PWS_SetAdcPllB(U8 OnOff_flag);
void MHal_PWS_SetAdcSog(U8 OnOff_flag);
void MHal_PWS_SetAdcSogOff(U8 OnOff_flag);
void MHal_PWS_SetAdcSogUnused(U8 OnOff_flag);
void MHal_PWS_SetAdcHsync0(U8 OnOff_flag);
void MHal_PWS_SetAdcHSync1(U8 OnOff_flag);
void MHal_PWS_SetAdcHsync2(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClk200(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClk400(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClkPll(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClkR(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClkG(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClkB(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClkY(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClkVd(U8 OnOff_flag);
void MHal_PWS_SetAdcPdClk200Fb(U8 OnOff_flag);
void MHal_PWS_SetAdcSogMux(U8 OnOff_flag);
void MHal_PWS_SetAdcFbAdc(U8 OnOff_flag);
void MHal_PWS_SetAdcCvbsLpfY(U8 OnOff_flag);
void MHal_PWS_SetAdcCvbsLpfC(U8 OnOff_flag);
U32 MHal_PWS_Get_DVDD(void);
void MHal_PWS_Set_DVDD(void);

#endif  //_MHal_PWS_H_


