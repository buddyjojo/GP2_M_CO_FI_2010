////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("; MStar; Confidential; Information;") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_tcon.c
/// @brief  TCON Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DRV_TCON_C__
#define __DRV_TCON_C__

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "stddef.h"
#include <common.h>
#include <linux/types.h>//#include "mdrv_types.h"
#include "mdrv_scaler.h"
#include "mdrv_tcon.h"
#include "mdrv_tcon_tbl.h"




//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define printk    printf

#define assert(p)   do {\
                        if (!(p)) {\
                            printk("BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                        }\
                    } while (0)

#define TCONDBL(x)   //x
//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Exernal
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------------------------------------

static void MDrv_TCONMAP_DumpGeneralRegTable(U8 *pTCONTable)
{
    U32 u32tabIdx = 0;
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;

    TCONDBL(printk("tab: general\n"));
    if (pTCONTable == NULL)
    {
        TCONDBL(printk("[TCON]TCONTable error: General Reg Table\n"));
        return;
    }

    while (--u16timeout)
    {
         u16Addr = (pTCONTable[u32tabIdx]<<8) + pTCONTable[(u32tabIdx +1)];
         u8Mask  = pTCONTable[(u32tabIdx +2)];
         u8Value = pTCONTable[(u32tabIdx +3)];

         if (u16Addr == REG_TABLE_END) // check end of table
             break;

         TCONDBL(printk("[addr=%04x, msk=%02x, val=%02x]\n", u16Addr, u8Mask, u8Value));

         if (u16Addr & 0x1)
         {
             u16Addr --;
             REG_WM( REG_GENERAL_BANK(u16Addr), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
         }
         else
         {
             REG_WM( REG_GENERAL_BANK(u16Addr), ((U16)u8Value), ((U16)u8Mask));
         }
         u32tabIdx = u32tabIdx + 4;
    }
    if (u16timeout==0)
        assert(0);
}

static void MDrv_TCONMAP_DumpMODRegTable(U8 *pTCONTable)
{
    U32 u32tabIdx = 0;
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;
    U8 u8CurBank;
    U8 u8LastBank = 0xff;
    BOOL bHiByte;
    MOD_BK_STORE;

    TCONDBL(printk("tab: mod\n"));
    if (pTCONTable == NULL)
    {
        TCONDBL(printk("[TCON]TCONTable error: Mod Reg Table\n"));
        return;
    }

    while (--u16timeout)
    {
        u16Addr = (pTCONTable[u32tabIdx]<<8) + pTCONTable[(u32tabIdx +1)];
        u8Mask  = pTCONTable[(u32tabIdx +2)];
        u8Value = pTCONTable[(u32tabIdx +3)];

        if (u16Addr == REG_TABLE_END) // check end of table
            break;

        u8CurBank = ((u16Addr & 0x7FFF) >> 8);
        bHiByte = (u16Addr & 0x8000) ? TRUE : FALSE;
        if (u8CurBank != u8LastBank)
        {
            TCONDBL(printk("<<bankswitch=%02x>>\n", u8CurBank));
            MOD_BK_SWICH(u8CurBank);
            u8LastBank = u8CurBank;
        }

        TCONDBL(printk("[addr=%04x, msk=%02x, val=%02x]\n",
            MOD_REG((u16Addr & 0x00FF))|bHiByte, u8Mask, u8Value));

        if (bHiByte)
        {
            REG_WM(MOD_REG((u16Addr & 0x00FF)), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
        }
        else
        {
            REG_WM(MOD_REG((u16Addr & 0x00FF)), (U16)u8Value, (U16)u8Mask);
        }
        u32tabIdx = u32tabIdx + 4;
    }
    if (u16timeout==0)
        assert(0);

    MOD_BK_RESTORE;
}

static void MDrv_TCONMAP_DumpTable(E_TCON_PANEL_INDEX etconIdx)
{
    TCON_TAB_INFO Tcontab;
    if ((etconIdx > TCON_PANEL_NUMS) || (etconIdx < 0))
        return;
    Tcontab = TConMAP_Main[etconIdx];
    MDrv_TCONMAP_DumpGeneralRegTable(Tcontab.pTConInitTab);
    //MDrv_TCONMAP_DumpGeneralRegTable(Tcontab.pTConInit_GPIOTab);
    //MDrv_TCONMAP_DumpScalerRegTable(Tcontab.pTConInit_SCTab);
    MDrv_TCONMAP_DumpMODRegTable(Tcontab.pTConInit_MODTab);
}

void MDrv_SC_Set_TCONMap(SC_TCON_MAP_t *pTconMap)//(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    TCONDBL(printk("[TCON_LoadSettings]\n"));
    MDrv_TCONMAP_DumpTable(pTconMap->u16tconpanelIdx);
    TCONDBL(printk("...done\n"));
}

BOOL bTCONpwsq =FALSE;
static U8 u8TCONPWSQ[32];
static U8 u8TCONPWSQcnt = 0;

void MDrv_TCONMAP_DumpPowerOnSequenceReg(void)
{
    U32 u32tabIdx = 0;
    U16 u16timeout = 0xFF;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;

    TCONDBL(printk("tab: PowerOnSequenceReg\n"));
    if (u8TCONPWSQ == NULL)
    {
        TCONDBL(printk("[TCON]TCONTable error: General Reg Table\n"));
        return;
    }

    while (--u16timeout)
    {
         u16Addr = (u8TCONPWSQ[u32tabIdx]<<8) + u8TCONPWSQ[(u32tabIdx +1)];
         u8Mask  = u8TCONPWSQ[(u32tabIdx +2)];
         u8Value = u8TCONPWSQ[(u32tabIdx +3)];

         if (u16Addr == REG_TABLE_END) // check end of table
         {
             bTCONpwsq = FALSE;
             u8TCONPWSQcnt = 0;
             break;
         }

         TCONDBL(printk("[addr=%04x, msk=%02x, val=%02x]\n", u16Addr, u8Mask, u8Value));
         if (u16Addr & 0x1)
         {
             u16Addr --;
             REG_WM( REG_GENERAL_BANK(u16Addr), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
         }
         else
         {
             REG_WM( REG_GENERAL_BANK(u16Addr), ((U16)u8Value), ((U16)u8Mask));
         }
         u32tabIdx = u32tabIdx + 4;
    }
    if (u16timeout==0)
        assert(0);
}

static void MDrv_TCONMAP_DumpPowerSequenceRegTable(U8 *pTCONTable, U8 u8Tcontype, BOOL benable)
{
    U32 u32tabIdx = 0;
    U16 u16timeout = 0x3FF;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;
    U8 u8signal_type = 0;
    BOOL bTcondone = 0;

    TCONDBL(printk("tab: PowerSequenceReg\n"));
    if (pTCONTable == NULL)
    {
        TCONDBL(printk("[TCON]TCONTable error: General Reg Table\n"));
        return;
    }

    while (--u16timeout)
    {
         u16Addr = (pTCONTable[u32tabIdx]<<8) + pTCONTable[(u32tabIdx +1)];
         u8Mask  = pTCONTable[(u32tabIdx +2)];
         u8Value = pTCONTable[(u32tabIdx +3)];
         u8signal_type = pTCONTable[(u32tabIdx +6)];

         if (u16Addr == REG_TABLE_END) // check end of table
         {
             if(benable)
             {
                 u8TCONPWSQ[u8TCONPWSQcnt] =  0xFF;
                 u8TCONPWSQ[u8TCONPWSQcnt +1] = 0xFF;
                 u8TCONPWSQ[u8TCONPWSQcnt +2] = 0xFF;
                 u8TCONPWSQ[u8TCONPWSQcnt +3] = 0xFF;
                 u8TCONPWSQcnt = u8TCONPWSQcnt + 4;
             }
             break;
         }
         if(benable)
         {
             bTcondone = FALSE;
         }
         if(u8Tcontype == u8signal_type)
         {
             TCONDBL(printk("[addr=%04x, msk=%02x, val=%02x]\n", u16Addr, u8Mask, u8Value));
             if(benable)
             {
                 u8TCONPWSQ[u8TCONPWSQcnt] = (U8)((u16Addr & 0xFF00) >> 8);
                 u8TCONPWSQ[u8TCONPWSQcnt +1] = (U8)(u16Addr & 0xFF);
                 u8TCONPWSQ[u8TCONPWSQcnt +2] = u8Mask;
                 u8TCONPWSQ[u8TCONPWSQcnt +3] = u8Value;
                 bTCONpwsq = TRUE;
                 bTcondone = TRUE;
             }
             else
             {
                 if (u16Addr & 0x1)
                 {
                     u16Addr --;
                     REG_WM( REG_GENERAL_BANK(u16Addr), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
                 }
                 else
                 {
                     REG_WM( REG_GENERAL_BANK(u16Addr), ((U16)u8Value), ((U16)u8Mask));
                 }
             }

         }
         u32tabIdx = u32tabIdx + 7;
         if(bTcondone)
         {
             u8TCONPWSQcnt = u8TCONPWSQcnt + 4;
         }
    }
    if(benable)
    {        
        u16timeout = 2;
        while(bTCONpwsq)
        {
            u16timeout--;
            if(u16timeout == 0)
            {
                bTCONpwsq = FALSE;
            }
            SC_BK_STORE;
            SC_BK_SWICH(REG_SC_BK_GOPINT);
            REG_WM(REG_SC_GOPINT(0x12), 0x20, 0x20);
            REG_WM(REG_SC_GOPINT(0x12), 0x00, 0x20);
            SC_BK_RESTORE;
            MDrv_SC_TCON_PWS_VSINT();
        }
    }
    if (u16timeout==0)
        assert(0);
}


void MDrv_SC_Set_TCONPower_Sequence(SC_TCON_POW_SEQ_INFO_t tconpower_sequence, U8 u8Tcontype)
{
    U8  *pTconTab =NULL;

    if(tconpower_sequence.benable)
    {
        TCONDBL(printk("[MDrv_SC_Set_TCONPower_Sequence_On]\n"));
        pTconTab = TConMAP_Main[tconpower_sequence.u16tconpanelIdx].pTConPower_Sequence_OnTab;
        MDrv_TCONMAP_DumpPowerSequenceRegTable(pTconTab, u8Tcontype, tconpower_sequence.benable);
    }
    else
    {
        TCONDBL(printk("[MDrv_SC_Set_TCONPower_Sequence_Off]\n"));
        pTconTab = TConMAP_Main[tconpower_sequence.u16tconpanelIdx].pTConPower_Sequence_OffTab;
        MDrv_TCONMAP_DumpPowerSequenceRegTable(pTconTab, u8Tcontype, tconpower_sequence.benable);
    }
}

#endif//__DRV_ADC_H__

