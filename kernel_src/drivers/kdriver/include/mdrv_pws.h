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
#ifndef _DRVPWS_H_
#define _DRVPWS_H_

////////////////////////////////////////////////////////////////////////////////
/// @file drvPWS.h
/// @author MStar Semiconductor Inc.
/// @brief power saving driver
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Header Files
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Define & data type
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Extern Function
//------------------------------------------------------------------------------

U32 MDrv_PWS_GetIOMapBase(void);
U32 MDrv_PWS_GetChipId(void);
void MDrv_PWS_Init(void);
U8 MDrv_PWS_SetIOMapBase(void);

// T3
U8 MDrv_PWS_SetDviDmDemux(U8 OnOff_flag);
U8 MDrv_PWS_SetDviPreamp(U8 OnOff_flag);
U8 MDrv_PWS_SetDviBist(U8 OnOff_flag);
U8 MDrv_PWS_SetDviDmRxckBist(U8 OnOff_flag);
U8 MDrv_PWS_SetDviDmPreclk(U8 OnOff_flag);
U8 MDrv_PWS_SetDviDmPreclkOffl(U8 OnOff_flag);
U8 MDrv_PWS_SetDviDmRbias(U8 OnOff_flag);
U8 MDrv_PWS_SetDviDmEnvdet(U8 OnOff_flag);
U8 MDrv_PWS_SetDviPllCore(U8 OnOff_flag);
U8 MDrv_PWS_SetDviPllRegm(U8 OnOff_flag);
U8 MDrv_PWS_SetDviPllPhdac(U8 OnOff_flag);
U8 MDrv_PWS_SetSingalPadRArray(U8 OnOff_flag);
U8 MDrv_PWS_SetVifBandgapIbiasVref(U8 OnOff_flag);
U8 MDrv_PWS_SetVifCalibrationBuffer(U8 OnOff_flag);
U8 MDrv_PWS_SetVifClampBuffer(U8 OnOff_flag);
U8 MDrv_PWS_SetVifAdcI(U8 OnOff_flag);
U8 MDrv_PWS_SetVifAdcQ(U8 OnOff_flag);
U8 MDrv_PWS_SetVifPga1(U8 OnOff_flag);
U8 MDrv_PWS_SetVifPga2(U8 OnOff_flag);
U8 MDrv_PWS_SetVifMpllReg(U8 OnOff_flag);
U8 MDrv_PWS_SetVifAdcOutClkPd(U8 OnOff_flag);
U8 MDrv_PWS_SetVifMpll_div2_pd(U8 OnOff_flag);
U8 MDrv_PWS_SetVifMpll_div3_pd(U8 OnOff_flag);
U8 MDrv_PWS_SetVifMpll_div4_pd(U8 OnOff_flag);
U8 MDrv_PWS_SetVifMpll_div8_pd(U8 OnOff_flag);
U8 MDrv_PWS_SetVifMpll_div10_pd(U8 OnOff_flag);
U8 MDrv_PWS_SetVifTagc(U8 OnOff_flag);
U8 MDrv_PWS_SetUsbHsTx(U8 OnOff_flag);
U8 MDrv_PWS_SetUsbHsRx(U8 OnOff_flag);
U8 MDrv_PWS_SetUsbFlXcvr(U8 OnOff_flag);
U8 MDrv_PWS_SetUsbSerdes(U8 OnOff_flag);
U8 MDrv_PWS_SetUsbPllLdo(U8 OnOff_flag);
U8 MDrv_PWS_SetUsbLdo(U8 OnOff_flag);
U8 MDrv_PWS_SetUsbRefBiasCir(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcR(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcG(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcB(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPhdig(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPllA(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcIclampRgb(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcIclampVdy(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcIclampVdc(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcY(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPllB(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcSog(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcSogOff(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcSogUnused(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcHsync0(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcHSync1(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcHsync2(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClk200(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClk400(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClkPll(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClkR(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClkG(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClkB(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClkY(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClkVd(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcPdClk200Fb(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcSogMux(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcFbAdc(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcCvbsLpfY(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcCvbsLpfC(U8 OnOff_flag);
/*
U8 MDrv_PWS_SetGmcp(U8 OnOff_flag);
U8 MDrv_PWS_SetGmcY(U8 OnOff_flag);
U8 MDrv_PWS_SetGmcC(U8 OnOff_flag);
U8 MDrv_PWS_SetCvbsBufOut(U8 OnOff_flag);
U8 MDrv_PWS_SetDacCvbs(U8 OnOff_flag);
U8 MDrv_PWS_SetFastBlanking(U8 OnOff_flag);
U8 MDrv_PWS_SetAdcRgbBiasCurrentControl(U8 OnOff_flag);
U8 MDrv_PWS_SetAudio(U8 OnOff_flag);
U8 MDrv_PWS_SetVd(U8 OnOff_flag);
U8 MDrv_PWS_SetSvd(U8 OnOff_flag);
U8 MDrv_PWS_SetMvdM4V(U8 OnOff_flag);
U8 MDrv_PWS_SetVe(U8 OnOff_flag);
U8 MDrv_PWS_SetRvd(U8 OnOff_flag);
U8 MDrv_PWS_SetStrld(U8 OnOff_flag);
U8 MDrv_PWS_SetGopg2(U8 OnOff_flag);
*/
void MDrv_PWS_Set_DVDD(void);

#endif


