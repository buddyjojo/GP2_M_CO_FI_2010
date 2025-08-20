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
////////////////////////////////////////////////////////////////////////////////
/// file drvPWS.c
/// @author MStar Semiconductor Inc.
/// @brief power saving driver
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// Header Files
//------------------------------------------------------------------------------
#include "mdrv_types.h"
#include "mdrv_pws.h"
#include "mhal_pws.h"

//------------------------------------------------------------------------------
// Local defines & local structures
//------------------------------------------------------------------------------
#define DEBUG_INFO(x)  // x
#define MBRegBase	   0x110500

//------------------------------------------------------------------------------
// External funciton
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local & Global Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Local Function
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Global Function
//------------------------------------------------------------------------------
U8 MDrv_PWS_SetIOMapBase(void)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetIOMapBase(MBRegBase);
    return 1;
}

U32 MDrv_PWS_GetIOMapBase(void)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    return MHal_PWS_GetIOMapBase();
}

U32 MDrv_PWS_GetChipId(void)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    return MHal_PWS_GetChipId();
}

void MDrv_PWS_Init(void)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_Init();
}

// T3
U8 MDrv_PWS_SetDviDmDemux(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviDmDemux(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviPreamp(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviPreamp(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviBist(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviBist(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviDmRxckBist(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviDmRxckBist(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviDmPreclk(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviDmPreclk(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviDmPreclkOffl(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviDmPreclkOffl(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviDmRbias(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviDmRbias(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviDmEnvdet(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviDmEnvdet(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviPllCore(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviPllCore(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviPllRegm(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviPllRegm(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetDviPllPhdac(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetDviPllPhdac(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetSingalPadRArray(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetSingalPadRArray(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifBandgapIbiasVref(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifBandgapIbiasVref(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifCalibrationBuffer(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifCalibrationBuffer(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifClampBuffer(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifClampBuffer(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifAdcI(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifAdcI(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifAdcQ(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifAdcQ(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifPga1(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifPga1(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifPga2(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifPga2(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifMpllReg(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifMpllReg(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifAdcOutClkPd(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifAdcOutClkPd(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifMpll_div2_pd(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifMpll_div2_pd(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifMpll_div3_pd(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifMpll_div3_pd(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifMpll_div4_pd(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifMpll_div4_pd(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifMpll_div8_pd(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifMpll_div8_pd(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifMpll_div10_pd(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetVifMpll_div10_pd(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetVifTagc(U8 OnOff_flag)
{
    DEBUG_INFO(printf("%s\n",__FUNCTION__));
    MHal_PWS_SetVifTagc(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetUsbHsTx(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetUsbHsTx(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetUsbHsRx(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetUsbHsRx(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetUsbFlXcvr(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetUsbFlXcvr(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetUsbSerdes(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetUsbSerdes(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetUsbPllLdo(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetUsbPllLdo(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetUsbLdo(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetUsbLdo(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetUsbRefBiasCir(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetUsbRefBiasCir(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcR(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcR(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcG(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcG(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcB(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcB(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPhdig(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPhdig(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPllA(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPllA(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcIclampRgb(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcIclampRgb(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcIclampVdy(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcIclampVdy(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcIclampVdc(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcIclampVdc(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcY(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcY(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPllB(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPllB(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcSog(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcSog(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcSogOff(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcSogOff(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcSogUnused(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcSogUnused(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcHsync0(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcHsync0(OnOff_flag);
    return 1;
}
U8 MDrv_PWS_SetAdcHSync1(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcHSync1(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcHsync2(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcHsync2(OnOff_flag);
    return 1;
}


U8 MDrv_PWS_SetAdcPdClk200(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClk200(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPdClk400(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClk400(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPdClkPll(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClkPll(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPdClkR(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClkR(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPdClkG(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClkG(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPdClkB(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClkB(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPdClkY(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClkY(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPdClkVd(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClkVd(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcPdClk200Fb(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcPdClk200Fb(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcSogMux(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcSogMux(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcFbAdc(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcFbAdc(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcCvbsLpfY(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcCvbsLpfY(OnOff_flag);
    return 1;
}

U8 MDrv_PWS_SetAdcCvbsLpfC(U8 OnOff_flag)
{
    DEBUG_INFO(printk("%s\n",__FUNCTION__));
    MHal_PWS_SetAdcCvbsLpfC(OnOff_flag);
    return 1;
}

void MDrv_PWS_Set_DVDD(void)
{
    MHal_PWS_Set_DVDD();
}

