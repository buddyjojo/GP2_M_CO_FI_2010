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

#include <linux/module.h>
#include <linux/sched.h>
#include "mdrv_types.h"

#define BW_DBG(x)               //x
#define REG_ADDR_SIZE           2
#define REG_BANK_SIZE           1
#define REG_MASK_SIZE           1
#define REG_TABLE_END           0xFFFF
#define REG(reg)                ((reg>>8)&0xFF), (reg&0xFF)
#define MAKE_ADDR(_x_)          (0x100000 | _x_)
#define code
#define REG_SCALER_BANK         0x102F00
#define REG_GOP_BANK            0x101FFE

typedef struct
{
    U8 *pIPTable;
    U8 u8TabNums;
    U8 u8TabIdx;
} TAB_Info;

#include "Titania3_Bandwidth_RegTable.h"
#include "Titania3_Bandwidth_RegTable.c"

#define assert(p)\
        do{\
            if (!(p)) {\
                printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                        __FILE__, __LINE__, #p);\
          }\
        } while (0)

#define REG_ADDR(addr)      (*((volatile U16*)(0xBF000000 + ((addr) << 1))))
#define REG_WM(_reg_, _val_, _msk_)\
        do{\
          REG_ADDR(_reg_) = (REG_ADDR(_reg_) & ~(_msk_)) | ((_val_) & (_msk_)); }while(0)

#define USE_SC_WRITEREG    1
extern void MHal_SC_WriteRegMask(U8 u8Bank, U32 u32Addr, U16 u16Value, U16 u16Mask);

static void _MDrv_SYS_DumpBWTable(TAB_Info* ptab_Info)
{
    U16 u16Addr;
    U8 u8Bank;
    U8 u8Mask;
    U8 u8Value;
    U8 u8BankSC_Bak;
    U8 u8BankGOP_Bak;

    if (ptab_Info->pIPTable == NULL){
        return;
    }

    if (ptab_Info->u8TabIdx >= ptab_Info->u8TabNums){
        assert(0);
        return;
    }

    u8BankSC_Bak = REG_ADDR(REG_SCALER_BANK) & 0xFF;
    u8BankGOP_Bak = REG_ADDR(REG_GOP_BANK)   & 0x0F;

    while (1)
    {
        u16Addr = (ptab_Info->pIPTable[0]<<8) + ptab_Info->pIPTable[1];
        u8Bank  = ptab_Info->pIPTable[2];
        u8Mask  = ptab_Info->pIPTable[3];
        u8Value = ptab_Info->pIPTable[REG_ADDR_SIZE+REG_BANK_SIZE+REG_MASK_SIZE+ptab_Info->u8TabIdx];

        if (u16Addr == REG_TABLE_END) // check end of table
            break;

        if ((u16Addr >> 8) == 0x2F)        // Scaler
        {
#if (USE_SC_WRITEREG==0)
            if ((REG_ADDR(REG_SCALER_BANK) & 0xFF) != u8Bank)
            {
                REG_ADDR(REG_SCALER_BANK) = u8Bank;
            }
#endif
        }
        else if ((u16Addr >> 8 ) == 0x1F)   // GOP
        {
            if ((REG_ADDR(REG_GOP_BANK) & 0x0F) != u8Bank)
            {
                REG_ADDR(REG_GOP_BANK) = u8Bank;
            }
        }

#if (USE_SC_WRITEREG==1)
        if ((u16Addr >> 8) == 0x2F)        // Scaler
        {
            if (u16Addr & 0x1)
            {
                u16Addr--;
                //REG_WM( MAKE_ADDR(u16Addr), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
                MHal_SC_WriteRegMask(u8Bank, MAKE_ADDR(u16Addr), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
                BW_DBG(printk("[addr=%04x, bank=%02x, msk=%04x, val=%04x]\n", u16Addr, u8Bank, ((U16)u8Mask) << 8, ((U16)u8Value) << 8));

            }
            else
            {
                //REG_WM( MAKE_ADDR(u16Addr), ((U16)u8Value), ((U16)u8Mask));
                MHal_SC_WriteRegMask(u8Bank, MAKE_ADDR(u16Addr), ((U16)u8Value), ((U16)u8Mask));
                BW_DBG(printk("[addr=%04x, bank=%02x, msk=%04x, val=%04x]\n", u16Addr, u8Bank, u8Mask, u8Value));
            }
        }
        else
#endif
        {
            if (u16Addr & 0x1)
            {
                u16Addr--;
                REG_WM( MAKE_ADDR(u16Addr), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
                BW_DBG(printk("[addr=%04x, bank=%02x, msk=%04x, val=%04x]\n", u16Addr, u8Bank, ((U16)u8Mask) << 8, ((U16)u8Value) << 8));

            }
            else
            {
                REG_WM( MAKE_ADDR(u16Addr), ((U16)u8Value), ((U16)u8Mask));
                BW_DBG(printk("[addr=%04x, bank=%02x, msk=%04x, val=%04x]\n", u16Addr, u8Bank, u8Mask, u8Value));
            }
        }

        ptab_Info->pIPTable+=(REG_ADDR_SIZE+REG_BANK_SIZE+REG_MASK_SIZE+ptab_Info->u8TabNums); // next
    }

    REG_ADDR(REG_SCALER_BANK) = u8BankSC_Bak;
    REG_ADDR(REG_GOP_BANK)    = u8BankGOP_Bak;

}

static void _MDrv_SYS_LoadBWTable(U8 u8TabIdx)
{
    TAB_Info tab_Info;
    tab_Info.pIPTable  = (void*)BWTABLE;
    tab_Info.u8TabNums = BWTABLE_NUMS;
    tab_Info.u8TabIdx = u8TabIdx;

    BW_DBG(printk("[BW]LoadTable\n"));
    _MDrv_SYS_DumpBWTable(&tab_Info);
}

// 20091002 daniel.huang: add bandwidth table & solve video flash on 1080p/i 60Hz in PICTURE menu
void MDrv_SYS_LoadInitBWTable(void)
{
    TAB_Info tab_Info;
    tab_Info.pIPTable = (void*)BWTABLE_COM;
    tab_Info.u8TabNums = 1;
    tab_Info.u8TabIdx = 0;

    BW_DBG(printk("MDrv_SYS_LoadInitBWTable()\n"));
    _MDrv_SYS_DumpBWTable(&tab_Info);
}

// 20091002 daniel.huang: add bandwidth table & solve video flash on 1080p/i 60Hz in PICTURE menu
void MDrv_SYS_LoadBWTable(U16 u16Input_HSize, U16 u16Input_VSize, BOOL bInterlace)
{

    U8 u8TabIdx;

    if (u16Input_HSize>=1440 && u16Input_VSize >= 900 && !bInterlace)
    {
        u8TabIdx = BWTABLE_1080p_mode;
    }else
    {
        u8TabIdx = BWTABLE_Normal_mode;
    }
    _MDrv_SYS_LoadBWTable(u8TabIdx);

}

