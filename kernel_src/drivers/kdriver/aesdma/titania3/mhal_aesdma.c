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

////////////////////////////////////////////////////////////////////////////////////////////////////
// file   mhal_aesdma.c
// @brief  AESDMA HAL
// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mhal_aesdma.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/atomic.h>

//--------------------------------------------------------------------------------------------------
//  Driver Compiler Option
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Hardware Abstraction Layer
//--------------------------------------------------------------------------------------------------
static REG_AESDMACtrl                *_AESDMACtrl = (REG_AESDMACtrl*)REG_AESDMACTRL_BASE;

#define REG32_W(reg, value)         (reg)->L = ((value) & 0x0000FFFF);                          \
                                    (reg)->H = ((value) >> 16);


//--------------------------------------------------------------------------------------------------
//  Macro of bit operations
//--------------------------------------------------------------------------------------------------
U32 MHal_AESDMA_Reg32_r(REG32 *reg)
{
    U32     value;

    value = (reg)->H << 16;
    value |= (reg)->L;

    return value;
}

//--------------------------------------------------------------------------------------------------
//  Inline Function
//--------------------------------------------------------------------------------------------------

void MHal_AESDMA_Reset(void)
{
    U32 Reg_AESDMA;
    int i = 0;
    Reg_AESDMA = (U32)(&_AESDMACtrl[0].Dma_Ctrl);
    REG32_W((&_AESDMACtrl[0].Dma_Ctrl) , 0x0080); // sw rst

    // reset all AESdma register
    for (i = 0 ; i < 19 ; i++)
    {
        REG32_W((REG32 *)(Reg_AESDMA+(i*8)), 0x00000000);
    }
}

void MHal_AESDMA_Set_CipherKey(U32 *cipherkey)
{
    REG32_W((&_AESDMACtrl[0].Dma_CipherKey_L.Key_L) , cipherkey[0]);
    REG32_W((&_AESDMACtrl[0].Dma_CipherKey_L.Key_H) , cipherkey[1]);
    REG32_W((&_AESDMACtrl[0].Dma_CipherKey_H.Key_L) , cipherkey[2]);
    REG32_W((&_AESDMACtrl[0].Dma_CipherKey_H.Key_H) , cipherkey[3]);
}

void MHal_AESDMA_Sel_Key(BOOL keysel)
{
    U32 u32keysel;
    u32keysel = (keysel<<29)&AESDMA_ENG_KEY_SEL;
    REG32_W((&_AESDMACtrl[0].Dma_Ctrl), MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl)|u32keysel);
}

void MHal_AESDMA_Set_FileinDesc(U32 FileinAddr , U32 FileinNum)
{
    REG32_W((&_AESDMACtrl[0].Dma_Filein_Addr), FileinAddr);
    REG32_W((&_AESDMACtrl[0].Dma_Filein_Num), FileinNum);
}

void MHal_AESDMA_Set_FileoutDesc(U32 FileoutSAddr , U32 FileoutEAddr)
{
    REG32_W((&_AESDMACtrl[0].Dma_Fileout_SAddr), FileoutSAddr);
    REG32_W((&_AESDMACtrl[0].Dma_Fileout_EAddr), FileoutEAddr);
}

void MHal_AESDMA_Start(BOOL AESDMAStart)
{
    if (AESDMAStart)
    {
        REG32_W((&_AESDMACtrl[0].Dma_Ctrl), (MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl) &~ AESDMA_CTRL_FILEOUT_START &~ AESDMA_CTRL_FILEIN_START));
        REG32_W((&_AESDMACtrl[0].Dma_Ctrl), (MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl) | AESDMA_CTRL_FILEOUT_START | AESDMA_CTRL_FILEIN_START));
    }
    else
    {
        REG32_W((&_AESDMACtrl[0].Dma_Ctrl) , MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl)&~(AESDMA_CTRL_FILEOUT_START|AESDMA_CTRL_FILEIN_START));
    }
}

void MHal_AESDMA_Set_PS_PTN(U32 MatchPTN)
{
    REG32_W((&_AESDMACtrl[0].Dma_PS_Pattern) , MatchPTN);
}

void MHal_AESDMA_Set_PS_Mask(U32 MatchMask)
{
    REG32_W((&_AESDMACtrl[0].Dma_PS_Pattern_Mask) , MatchMask);
}

void MHal_AESDMA_Set_PS_ENG(BOOL PSin_en, BOOL PSout_en)
{
    U32 u32PSin, u32PSout;

    u32PSin  = AESDMA_ENG_PS_IN_EN & (PSin_en<<20);
    u32PSout = AESDMA_ENG_PS_OUT_EN & (PSout_en<<21);

    REG32_W((&_AESDMACtrl[0].Dma_Ctrl) , MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl)|(u32PSin|u32PSout));
}

U32 MHal_AESDMA_Get_PS_MatchedBytecnt(void)
{
    return MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Matched_Btyecnt);
}

U32 MHal_AESDMA_Get_PS_MatchedPTN(void)
{
    return MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Matched_Pat);
}

U32 MHal_AESDMA_Get_AESDMA_Status(void)
{
    return MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_PVR_Status);
}

void MHal_AESDMA_Set_PVR_ENG(U32 Eng, BOOL Descrypt, BOOL Mode)
{
    U32 u32Eng, u32Descrypt, u32Mode;

    u32Eng = Eng << 24;

    u32Descrypt = AESDMA_ENG_DESCRYPT & (Descrypt<<25);

    u32Mode = Mode<<28;

    REG32_W((&_AESDMACtrl[0].Dma_Ctrl), MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl)|(u32Eng|u32Descrypt|u32Mode));
}

void MHal_AESDMA_Set_MIU_Path(BOOL MIU_R, BOOL MIU_W)
{
    REG32_W((&_AESDMACtrl[0].Dma_Ctrl), AESDMA_RESET_FLAG1(MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl), AESDMA_CTRL_BANK_R));
    REG32_W((&_AESDMACtrl[0].Dma_Ctrl), AESDMA_RESET_FLAG1(MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl), AESDMA_CTRL_BANK_W));

    REG32_W((&_AESDMACtrl[0].Dma_Ctrl), AESDMA_SET_FLAG1(MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl), (AESDMA_CTRL_BANK_R & ((U32)MIU_R << 5))));
    REG32_W((&_AESDMACtrl[0].Dma_Ctrl), AESDMA_SET_FLAG1(MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl), (AESDMA_CTRL_BANK_W & ((U32)MIU_W << 11))));
}

void MHal_AESDMA_Enable_Int(void)
{
    REG32_W((&_AESDMACtrl[0].Dma_Ctrl) , MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl)|AESDMA_INT_EN);
}

void MHal_AESDMA_Disable_Int(void)
{
    REG32_W((&_AESDMACtrl[0].Dma_Ctrl) , MHal_AESDMA_Reg32_r(&_AESDMACtrl[0].Dma_Ctrl)&~AESDMA_INT_EN);
}

U16 MHal_AESDMA_Random(void)
{
    return *(volatile unsigned int*)(0xbf203a38);
}


