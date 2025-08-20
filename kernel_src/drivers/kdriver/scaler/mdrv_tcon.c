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
/// file    mdrv_tcon.c
/// @brief  TCON Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DRV_TCON_C__
#define __DRV_TCON_C__

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "mdrv_types.h"
#include "mdrv_scaler_st.h"
#include "mhal_mod_reg.h"
#include "mhal_scaler_reg.h"
#include "mhal_utility.h"
#include "mhal_tcon.h"
#include "mdrv_tcon.h"
#include "mdrv_tcon_tbl.c"
#include "mdrv_tcon_tbl.h"
#include <linux/jiffies.h>


//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures Start
//-------------------------------------------------------------------------------------------------
extern unsigned long irq_flags_swbk;
extern spinlock_t switch_bk_lock;
extern U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
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
#if 0
static void MDrv_TCONMAP_DumpScalerRegTable(U8 *pTCONTable)
{
    U32 u32tabIdx = 0;
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;
    U8 u8CurBank;
    U8 u8LastBank = 0xff;
    BOOL bHiByte;

    TCONDBL(printk("tab: sc\n"));
    if (pTCONTable == NULL)
    {
        TCONDBL(printk("[TCON]TCONTable error: Scaler Reg Table\n"));
        return;
    }
    SC_BK_STORE;
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
            SC_BK_SWICH(u8CurBank);
            u8LastBank = u8CurBank;
        }

        TCONDBL(printk("[addr=%04x, msk=%02x, val=%02x]\n",
            REG_SC_BANK((u16Addr & 0x00FF))|bHiByte, u8Mask, u8Value));

        if (bHiByte)
        {
            REG_WM(REG_SC_BANK((u16Addr & 0x00FF)), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
        }
        else
        {
            REG_WM(REG_SC_BANK((u16Addr & 0x00FF)), (U16)u8Value, (U16)u8Mask);
        }
        u32tabIdx = u32tabIdx + 4;
    }
    SC_BK_RESTORE;
    if (u16timeout==0)
        assert(0);
}
#endif
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

    TCONDBL(printk("tab: mod\n"));
    if (pTCONTable == NULL)
    {
        TCONDBL(printk("[TCON]TCONTable error: Mod Reg Table\n"));
        return;
    }
    MOD_BK_STORE;
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
    MOD_BK_RESTORE;
    if (u16timeout==0)
        assert(0);
}

static void MDrv_TCONMAP_DumpTable(E_TCON_PANEL_INDEX etconIdx)
{
    TCON_TAB_INFO Tcontab;
    if ((etconIdx > TCON_PANEL_NUMS) || (etconIdx < 0))
        return;
    Tcontab = TConMAP_Main[etconIdx];
    MDrv_TCONMAP_DumpGeneralRegTable(Tcontab.pTConInitTab);
    //MDrv_TCONMAP_DumpGeneralRegTable(Tcontab.pTConInit_GPIOTab);	//balup_090921_DPM HCONV는 app에서 gpio45, gpio44로 ctrl하므로, 여기서는 막아야함.
    //MDrv_TCONMAP_DumpScalerRegTable(Tcontab.pTConInit_SCTab);
    MDrv_TCONMAP_DumpMODRegTable(Tcontab.pTConInit_MODTab);
}

void MDrv_SC_Set_TCONMap(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_TCON_MAP_t tconMap;
    if (copy_from_user(&tconMap, (void __user *)arg, sizeof(SC_TCON_MAP_t)))
    {
        return;
    }
    TCONDBL(printk("[TCON_LoadSettings]\n"));
    MDrv_TCONMAP_DumpTable(tconMap.u16tconpanelIdx);
    TCONDBL(printk("...done\n"));
}

static void MDrv_TCONMAP_GetTableSize(U8 *pTconTab, U16 u16tabtype, U32 *pu32TconTabsize)
{
    U8 u8Colslen = 0;
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    if (pTconTab == NULL)
    {
        TCONDBL(printk("[TCON]TCONTable error\n"));
        return;
    }
    switch(u16tabtype )
    {
    case TCON_TABTYPE_GENERAL:
    case TCON_TABTYPE_GPIO:
    case TCON_TABTYPE_SCALER:
    case TCON_TABTYPE_MOD:
    case TCON_TABTYPE_GAMMA:
        u8Colslen = 4;
        break;
    case TCON_TABTYPE_POWER_SEQUENCE_ON:
    case TCON_TABTYPE_POWER_SEQUENCE_OFF:
        u8Colslen = 7;
        break;
    default:
        TCONDBL(printk("[TCON]GetTable Size :unknown Tab Size\n"));
        return ;
    }
    while (--u16timeout)
    {
        u16Addr = (pTconTab[*pu32TconTabsize]<<8) + pTconTab[(*pu32TconTabsize +1)];
        if (u16Addr == REG_TABLE_END) // check end of table
            break;
        *pu32TconTabsize = *pu32TconTabsize + u8Colslen;
    }
    if (u16timeout==0)
        assert(0);
    TCONDBL(printk("<<*pu32TconTabsize=%d>>\n",  *pu32TconTabsize));
}

void MDrv_SC_Get_TCONTab_Info(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_TCON_TAB_INFO_t tcontabinfo;
    U8  *pTconTab =NULL;
    TCON_TAB_INFO Tcontab;
    U32  u32TconTabsize =0;
    if (copy_from_user(&tcontabinfo, (void __user *)arg, sizeof(SC_TCON_TAB_INFO_t)))
    {
        return;
    }
    Tcontab = TConMAP_Main[tcontabinfo.u16tconpanelIdx];

    switch(tcontabinfo.u16tabtype )
    {
    case TCON_TABTYPE_GENERAL:
        pTconTab = Tcontab.pTConInitTab;
        break;

    case TCON_TABTYPE_MOD:
        pTconTab = Tcontab.pTConInit_MODTab;
        break;

    case TCON_TABTYPE_POWER_SEQUENCE_ON:
        pTconTab = Tcontab.pTConPower_Sequence_OnTab;
        break;
    case TCON_TABTYPE_POWER_SEQUENCE_OFF:
        pTconTab = Tcontab.pTConPower_Sequence_OffTab;
        break;
    default:
        TCONDBL(printk("[TCON]GetTable:unknown Tab type\n"));
        break;
    }
    MDrv_TCONMAP_GetTableSize(pTconTab, tcontabinfo.u16tabtype, &u32TconTabsize);
    if(u32TconTabsize > TCON_TAB_MAX_SIZE)
    {
        TCONDBL(printk("[TCON]GetTable:Tab size too large than TCON_TAB_MAX_SIZE\n"));
        assert(0);
    }
    memcpy(tcontabinfo.u8TconTab, pTconTab, u32TconTabsize);
    tcontabinfo.u32Tabsize = u32TconTabsize;
    if (copy_to_user((U32*)arg, (U32*)&tcontabinfo, sizeof(SC_TCON_TAB_INFO_t)))
    {
        return;
    }
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
    U32 u32Time;
    U16 u16Timeout;

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
        u32Time = jiffies;
        u16Timeout = 50;
        while (1)
        {
            if ((bTCONpwsq == FALSE) || ((jiffies - u32Time) >= u16Timeout))
            {
                break;
            }
        }
    }
    if (u16timeout==0)
        assert(0);
}


void MDrv_SC_Set_TCONPower_Sequence(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_TCON_POW_SEQ_INFO_t tconpower_sequence;
    U8  *pTconTab =NULL;
    if (copy_from_user(&tconpower_sequence, (void __user *)arg, sizeof(SC_TCON_POW_SEQ_INFO_t)))
    {
        return;
    }
    if(tconpower_sequence.benable)
    {
        TCONDBL(printk("[MDrv_SC_Set_TCONPower_Sequence_On]\n"));
        pTconTab = TConMAP_Main[tconpower_sequence.u16tconpanelIdx].pTConPower_Sequence_OnTab;
        MDrv_TCONMAP_DumpPowerSequenceRegTable(pTconTab, tconpower_sequence.u8Tcontype, tconpower_sequence.benable);
    }
    else
    {
        TCONDBL(printk("[MDrv_SC_Set_TCONPower_Sequence_Off]\n"));
        pTconTab = TConMAP_Main[tconpower_sequence.u16tconpanelIdx].pTConPower_Sequence_OffTab;
        MDrv_TCONMAP_DumpPowerSequenceRegTable(pTconTab, tconpower_sequence.u8Tcontype, tconpower_sequence.benable);
    }
}

void MDrv_SC_Set_TCON_Count_Reset(PSC_DRIVER_CONTEXT_t pDrvCtx, U32 arg)
{
    SC_TCON_COUNT_RESET_t tcon_count;
    if (copy_from_user(&tcon_count, (void __user *)arg, sizeof(SC_TCON_COUNT_RESET_t)))
    {
        return;
    }
    TCONDBL(printk("MDrv_SC_Set_TCON_Count_Reset\n"));
    MHal_TCON_Count_Reset(tcon_count.bEnable);
}
#endif//__DRV_ADC_H__

