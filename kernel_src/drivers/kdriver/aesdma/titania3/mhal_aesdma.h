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
//
//  File name: mhal_aesdma.h
//  Description: Personal video record DMA Register Definition
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _AESDMA_REG_MCU_H_
#define _AESDMA_REG_MCU_H_
#include "mdrv_types.h"
//--------------------------------------------------------------------------------------------------
//  Abbreviation
//--------------------------------------------------------------------------------------------------
// Addr                             Address
// Buf                              Buffer
// Clr                              Clear
// CmdQ                             Command queue
// Cnt                              Count
// Ctrl                             Control
// Flt                              Filter
// Hw                               Hardware
// Int                              Interrupt
// Len                              Length
// Ovfw                             Overflow
// Pkt                              Packet
// Rec                              Record
// Recv                             Receive
// Rmn                              Remain
// Reg                              Register
// Req                              Request
// Rst                              Reset
// Scmb                             Scramble
// Sec                              Section
// Stat                             Status
// Sw                               Software
// Ts                               Transport Stream


//--------------------------------------------------------------------------------------------------
//  Global Definition
//--------------------------------------------------------------------------------------------------
#define AESDMA_HAS_FLAG(flag, bit)        ((flag)&  (bit))
#define AESDMA_SET_FLAG(flag, bit)        ((flag)|= (bit))
#define AESDMA_RESET_FLAG(flag, bit)      ((flag)&= (~(bit)))
#define AESDMA_SET_FLAG1(flag, bit)       ((flag)|  (bit))
#define AESDMA_RESET_FLAG1(flag, bit)     ((flag)&  (~(bit)))

#define AESDMA_NUM  2
#define AESDMA_MIU0 0
#define AESDMA_MIU1 0x800000
//--------------------------------------------------------------------------------------------------
//  Compliation Option
//--------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Harware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
#define REG_AESDMACTRL_BASE         0xbf201940//0xbf201800

typedef struct _REG32
{
    volatile U32                L;
    volatile U32                H;
} REG32;

typedef struct _REG_CipherKey
{
    REG32                           Key_L;
    REG32                           Key_H;
} REG_CipherKey;

typedef struct _REG_AESDMACtrl
{
    REG32               Dma_Ctrl;                                   //0x50
    #define AESDMA_CTRL_FILEIN_START            0x00000100
    #define AESDMA_CTRL_FILEOUT_START           0x00000001
    #define AESDMA_CTRL_BANK_R                  0x00000020
    #define AESDMA_CTRL_BANK_W                  0x00000800

    #define AESDMA_INT_EN                       0x00400000
    #define AESDMA_ENG_PS_IN_EN                 0x00100000
    #define AESDMA_ENG_PS_OUT_EN                0x00200000
    #define AESDMA_ENG_AES_EN                   0x01000000
    #define AESDMA_ENG_DESCRYPT                 0x02000000          // 0:encrypt, 1:decrypt
    #define AESDMA_ENG_TDES_EN                  0x10000000          // If (aes_en) AES DMA Else if (tdes_en) TDES DMA Else DMA only
    #define AESDMA_ENG_KEY_SEL                  0x20000000          // 0: reg_cipher_key, 1: key from KL

    REG32               Dma_Filein_Addr;                            //0x52
    REG32               Dma_Filein_Num;                             //0x54
    REG32               Dma_Fileout_SAddr;                          //0x56
    REG32               Dma_Fileout_EAddr;                          //0x58
    REG32               Dma_PS_Pattern;                             //0x5a
    REG32               Dma_PS_Pattern_Mask;                        //0x5c
    REG32               Dma_Ctrl2;                                  //0x5e
    REG_CipherKey       Dma_CipherKey_L;                            //0x60
    REG_CipherKey       Dma_CipherKey_H;                            //0x64
    REG32               Dma_Aes64_L[2];                             //0x68~0x6b T3 new for AES lower 64IV
    REG32               Dma_Aes64_H[2];                             //0x6c~0x6f T3 new for AES higher 64IV
    REG32               Dma_Matched_Btyecnt;                        //0x70
    REG32               Dma_Matched_Pat;                            //0x72
    REG32               Dma_Err_Wadr;                               //0x74
    REG32               Dma_Null_Reg[4];                            //0x76~0x7c
    REG32               Dma_PVR_Status;                             //0x7e
    #define AESDMA_PS_DONE                      0x00000001
    #define AESDMA_PS_STOP                      0x00000002
    #define AESDMA_DMA_DONE                     0x00010000
    #define AESDMA_DMA_PAUSE                    0x00020000
    #define AESDMA_STATES_GROUP                 (AESDMA_PS_DONE     | \
                                                 AESDMA_PS_STOP     | \
                                                 AESDMA_DMA_DONE    | \
                                                 AESDMA_DMA_PAUSE  )
}REG_AESDMACtrl;


extern U32 MHal_AESDMA_Reg32_r(REG32 *reg);
extern void MHal_AESDMA_Reset(void);
extern void MHal_AESDMA_Set_CipherKey(U32 *cipherkey);
extern void MHal_AESDMA_Sel_Key(BOOL keysel);
extern void MHal_AESDMA_Set_FileinDesc(U32 FileinAddr , U32 FileinNum);
extern void MHal_AESDMA_Set_FileoutDesc(U32 FileoutSAddr , U32 FileoutEAddr);
extern void MHal_AESDMA_Start(BOOL AESDMAStart);
extern void MHal_AESDMA_Set_PS_PTN(U32 MatchPTN);
extern void MHal_AESDMA_Set_PS_Mask(U32 MatchMask);
extern void MHal_AESDMA_Set_PS_ENG(BOOL PSin_en, BOOL PSout_en);
extern U32 MHal_AESDMA_Get_PS_MatchedBytecnt(void);
extern U32 MHal_AESDMA_Get_PS_MatchedPTN(void);
extern U32 MHal_AESDMA_Get_AESDMA_Status(void);
extern void MHal_AESDMA_Set_PVR_ENG(U32 Eng, BOOL Descrypt, BOOL Mode);
extern void MHal_AESDMA_Set_MIU_Path(BOOL MIU_R, BOOL MIU_W);
extern void MHal_AESDMA_Enable_Int(void);
extern void MHal_AESDMA_Disable_Int(void);
extern U16 MHal_AESDMA_Random(void);
#endif // #ifndef _AESDMA_REG_MCU_H_
