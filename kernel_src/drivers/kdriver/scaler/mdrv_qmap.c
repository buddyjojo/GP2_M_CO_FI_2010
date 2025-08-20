
#include "mdrv_types.h"
#include "mdrv_scaler_st.h"
#include "mdrv_scaler.h"
#include "mhal_scaler_reg.h"
#include "mdrv_qmap.h"
#include "mhal_qmap.h"
#include "QualityMode.h"
#include "QualityMode.c"
#include "mhal_utility.h"

#include "Titania3_Main.c"
#include "Titania3_Main_1920.c"
#include "Titania3_Main_1366.c"
#include "Titania3_Main_HSDRule.c"
#include "Titania3_Main_VSDRule.c"
#include "Titania3_Main_HSPRule.c"
#include "Titania3_Main_VSPRule.c"
#include "Titania3_Main_CSCRule.c"

//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures Start
//-------------------------------------------------------------------------------------------------
extern unsigned long irq_flags_swbk;
extern spinlock_t switch_bk_lock;
extern U8 u8switch_bk_lock;
//-------------------------------------------------------------------------------------------------
//  SPIN Lock Structures End
//-------------------------------------------------------------------------------------------------

#define PQTAB_DBG(x) //x
#define SRULE_DBG(x) //x

#define _END_OF_TBL_                0xFFFF//-1 // end of register table

static PQTABLE_INFO PQTableInfo[MAX_WINDOW];
static U16 _u16PQSrcType[MAX_WINDOW];
static U8 _u8PQTabIdx[PQ_IP_NUM_Main];    // store all TabIdx of all IPs
#if PQ_ENABLE_DEBUG
BOOL bEnPQDbg = FALSE;
#endif

#define assert(p)   do {\
                        if (!(p)) {\
                            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                                    __FILE__, __LINE__, #p);\
                        }\
                    } while (0)


// 20090915 daniel.huang: fix sram filter not be initialized after STR power on
// to save loading SRAM table time, SRAM are only downloaded
// when current SRAM table is different to previous SRAM table
static U8 _u8SRAM1Table=0xFF;
static U8 _u8SRAM2Table=0xFF;
static U8 _u8SRAM3Table=0xFF;
static U8 _u8SRAM4Table=0xFF;
static U8 _u8C_SRAM1Table=0xFF;
static U8 _u8C_SRAM2Table=0xFF;
static U8 _u8C_SRAM3Table=0xFF;
static U8 _u8C_SRAM4Table=0xFF;
static U8 _u8Color_SRAMTable=0xFF;
static U8 _u8Color_SNR_SRAMTable=0xFF;
static U8 _u8Color_DNR_SRAMTable=0xFF;

void MDrv_SCMAP_DumpGeneralRegTable(EN_IP_Info* pIP_Info)
{
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;

    PQ_DBG(printk("tab: general\n"));
    if (pIP_Info->u8TabIdx >= pIP_Info->u8TabNums){
        PQ_DBG(printk("[PQ]IP_Info error: General Reg Table\n"));
        return;
    }

    while (--u16timeout)
    {
         u16Addr = (pIP_Info->pIPTable[0]<<8) + pIP_Info->pIPTable[1];
         u8Mask  = pIP_Info->pIPTable[2];
         u8Value = pIP_Info->pIPTable[REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabIdx];

         if (u16Addr == _END_OF_TBL_) // check end of table
             break;

         PQ_DBG(printk("[addr=%04x, msk=%02x, val=%02x]\n", u16Addr, u8Mask, u8Value));

         if (u16Addr & 0x1)
         {
             u16Addr--;
             REG_WM( REG_GENERAL_BANK(u16Addr), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
         }
         else
         {
             REG_WM( REG_GENERAL_BANK(u16Addr), ((U16)u8Value), ((U16)u8Mask));
         }
         pIP_Info->pIPTable+=(REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabNums); // next
    }
    if (u16timeout==0)
        assert(0);
}

void MDrv_SCMAP_DumpCombRegTable(EN_IP_Info* pIP_Info)
{
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;
    U8 u8CurBank;
    U8 u8LastBank = 0xff;
    BOOL bHiByte;


    PQ_DBG(printk("tab: comb\n"));
    if (pIP_Info->u8TabIdx >= pIP_Info->u8TabNums){
        PQ_DBG(printk("[PQ]IP_Info error: Comb Reg Table\n"));
        return;
    }
    COMB_BK_STORE;
    while (--u16timeout)
    {
        u16Addr = (pIP_Info->pIPTable[0]<<8) + pIP_Info->pIPTable[1];
        u8Mask  = pIP_Info->pIPTable[2];
        u8Value = pIP_Info->pIPTable[REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabIdx];

        if (u16Addr == _END_OF_TBL_) // check end of table
            break;

        u8CurBank = ((u16Addr & 0x7FFF) >> 8);
        bHiByte = (u16Addr & 0x8000) ? TRUE : FALSE;

        if (u8CurBank != u8LastBank)
        {
            PQ_DBG(printk("<<bankswitch=%02x>>\n", u8CurBank));
            COMB_BK_SWICH(u8CurBank);
            u8LastBank = u8CurBank;
        }

        PQ_DBG(printk("[addr=%04x, msk=%02x, val=%02x]\n",
            REG_COMB_BANK((u16Addr & 0x00FF))|bHiByte, u8Mask, u8Value));

        if (bHiByte)
        {
            REG_WM(REG_COMB_BANK((u16Addr & 0x00FF)), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
        }
        else
        {
            REG_WM(REG_COMB_BANK((u16Addr & 0x00FF)), (U16)u8Value, (U16)u8Mask);
        }

         pIP_Info->pIPTable+=(REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabNums); // next
    }
    COMB_BK_RESTORE;
    if (u16timeout==0)
        assert(0);
}

void MDrv_SCMAP_DumpScalerRegTable(EN_IP_Info* pIP_Info)
{
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;
    U8 u8CurBank;
    U8 u8LastBank = 0xff;
    BOOL bHiByte;


    PQ_DBG(printk("tab: sc\n"));
    if (pIP_Info->u8TabIdx >= pIP_Info->u8TabNums){
        PQ_DBG(printk("[PQ]IP_Info error: Scaler Reg Table\n"));
        return;
    }
    SC_BK_STORE;
    while (--u16timeout)
    {
        u16Addr = (pIP_Info->pIPTable[0]<<8) + pIP_Info->pIPTable[1];
        u8Mask  = pIP_Info->pIPTable[2];
        u8Value = pIP_Info->pIPTable[REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabIdx];

        if (u16Addr == _END_OF_TBL_) // check end of table
            break;

        u8CurBank = ((u16Addr & 0x7FFF) >> 8);
        bHiByte = (u16Addr & 0x8000) ? TRUE : FALSE;
        if (u8CurBank != u8LastBank)
        {
            PQ_DBG(printk("<<bankswitch=%02x>>\n", u8CurBank));
            SC_BK_SWICH(u8CurBank);
            u8LastBank = u8CurBank;
        }

        PQ_DBG(printk("[addr=%04x, msk=%02x, val=%02x]\n",
            REG_SC_BANK((u16Addr & 0x00FF))|bHiByte, u8Mask, u8Value));

        if (bHiByte)
        {
            REG_WM(REG_SC_BANK((u16Addr & 0x00FF)), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
        }
        else
        {
            REG_WM(REG_SC_BANK((u16Addr & 0x00FF)), (U16)u8Value, (U16)u8Mask);
        }

        pIP_Info->pIPTable+=(REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabNums); // next
    }
    SC_BK_RESTORE;
    if (u16timeout==0)
        assert(0);

}

void MDrv_SCMAP_DumpFilterTable(EN_IP_Info* pIP_Info)
{
    U16 i, j, x, winidx, base;
    U8 u8Ramcode[8];
    U16 u16Gain_DNR, u16Gain_SNR;


    PQ_DBG(printk("tab: sram\n"));
    if (pIP_Info->u8TabIdx >= pIP_Info->u8TabNums){
        PQ_DBG(printk("[PQ]IP_Info error: SRAM Table\n"));
        return;
    }
    SC_BK_STORE;
    SC_BK_SWICH(REG_SC_BK_GOPINT);
    REG_WM(REG_SC_GOPINT(0x03), 0x0000, 0x1000);
    //printk("u8TabIdx=%02x\n",pIP_Info->u8TabIdx );

    j = 0;
    if ((pIP_Info->u8TabType == PQ_TABTYPE_SRAM1)
    ||  (pIP_Info->u8TabType == PQ_TABTYPE_SRAM2)
    ||  (pIP_Info->u8TabType == PQ_TABTYPE_SRAM3)
    ||  (pIP_Info->u8TabType == PQ_TABTYPE_SRAM4))
    {
        // SRAM1/2    (128x60bit)
        if (pIP_Info->u8TabType == PQ_TABTYPE_SRAM1)
        {
            winidx = 0;
            base = 0x00;
        }
        else if (pIP_Info->u8TabType == PQ_TABTYPE_SRAM2)
        {
            winidx = 0;
            base = 0x80;
        }
        else if (pIP_Info->u8TabType == PQ_TABTYPE_SRAM3)
        {
            winidx = 1;
            base = 0x00;
        }
        else if (pIP_Info->u8TabType == PQ_TABTYPE_SRAM4)
        {
            winidx = 1;
            base = 0x80;
        }
        else
        {
            SC_BK_RESTORE;
            assert(0);
            return;
        }

        SC_BK_SWICH(REG_SC_BK_HVSP);
        REG_WL(REG_SC_HVSP(0x41), BIT0);

        for ( i=0; i<128; i++)
        {
            while (REG_RH(REG_SC_HVSP(0x41)) & BIT0);

            // address
            REG_WM(REG_SC_HVSP(0x42), ((winidx<<8)|(base+i)), 0x1FF );

            for ( x=0;x<8;x++ )
            {
                u8Ramcode[x] = pIP_Info->pIPTable[pIP_Info->u8TabIdx * PQ_IP_SRAM2_SIZE_Main + j];
                PQ_DBG(printk(" %02x ", u8Ramcode[x] ));
                j++;
            }
            PQ_DBG(printk("\n"));

            REG_WL(REG_SC_HVSP(0x43), u8Ramcode[0]);
            REG_WH(REG_SC_HVSP(0x43), u8Ramcode[1]);
            REG_WL(REG_SC_HVSP(0x44), u8Ramcode[2]);
            REG_WH(REG_SC_HVSP(0x44), u8Ramcode[3]);
            REG_WL(REG_SC_HVSP(0x45), u8Ramcode[4]);
            REG_WH(REG_SC_HVSP(0x45), u8Ramcode[5]);
            REG_WL(REG_SC_HVSP(0x49), u8Ramcode[6]);
            REG_WH(REG_SC_HVSP(0x49), u8Ramcode[7]);
            // enable write
            REG_WM(REG_SC_HVSP(0x41), BIT8, BIT8);
        }
        REG_WL(REG_SC_HVSP(0x41), 0x00);
    }
    else if ((pIP_Info->u8TabType == PQ_TABTYPE_C_SRAM1)
            ||  (pIP_Info->u8TabType == PQ_TABTYPE_C_SRAM2)
            ||  (pIP_Info->u8TabType == PQ_TABTYPE_C_SRAM3)
            ||  (pIP_Info->u8TabType == PQ_TABTYPE_C_SRAM4))
    {
        // C_SRAM1/C_SRAM2/C_SRAM3/C_SRAM4    (64x40bit)
        if (pIP_Info->u8TabType == PQ_TABTYPE_C_SRAM1)
        {
            winidx = 0;
            base = 0x00;
        }
        else if (pIP_Info->u8TabType == PQ_TABTYPE_C_SRAM2)
        {
            winidx = 0;
            base = 0x40;
        }
        else if (pIP_Info->u8TabType == PQ_TABTYPE_C_SRAM3)
        {
            winidx = 1;
            base = 0x80;
        }
        else if (pIP_Info->u8TabType == PQ_TABTYPE_C_SRAM4)
        {
            winidx = 1;
            base = 0xC0;
        }
        else
        {
            SC_BK_RESTORE;
            assert(0);
            return;
        }

        SC_BK_SWICH(REG_SC_BK_HVSP);
        REG_WL(REG_SC_HVSP(0x41), BIT1);

        for ( i=0; i<64; i++)
        {
            while (REG_RH(REG_SC_HVSP(0x41)) & BIT0);

            // address
            REG_WM(REG_SC_HVSP(0x42), ((winidx<<8)|(base+i)), 0x1FF );

            for ( x=0;x<5;x++ )
            {
                u8Ramcode[x] = pIP_Info->pIPTable[pIP_Info->u8TabIdx * PQ_IP_C_SRAM1_SIZE_Main + j];
                PQ_DBG(printk(" %02x ", u8Ramcode[x] ));
                j++;
            }
            PQ_DBG(printk("\n"));

            REG_WL(REG_SC_HVSP(0x43), u8Ramcode[0]);
            REG_WH(REG_SC_HVSP(0x43), u8Ramcode[1]);
            REG_WL(REG_SC_HVSP(0x44), u8Ramcode[2]);
            REG_WH(REG_SC_HVSP(0x44), u8Ramcode[3]);
            REG_WL(REG_SC_HVSP(0x45), u8Ramcode[4]);

            // enable write
            REG_WM(REG_SC_HVSP(0x41), BIT8, BIT8);
        }

    }
    else if (pIP_Info->u8TabType == PQ_TABTYPE_SRAM_COLOR_INDEX)
    {
        // SRAM COLOR INDEX
        SC_BK_SWICH(REG_SC_BK_DNR);

        for ( i=0; i<256; i++)
        {
            while (REG_RH(REG_SC_DNR(0x61)) & BIT0);

            // address
            REG_WM(REG_SC_DNR(0x63), i, 0x00FF );

            u8Ramcode[0] = pIP_Info->pIPTable[pIP_Info->u8TabIdx * PQ_IP_C_SRAM1_SIZE_Main + j];
            PQ_DBG(printk(" %02x ", u8Ramcode[0] ));
            j++;
            PQ_DBG(printk("\n"));

            REG_WM(REG_SC_DNR(0x62), u8Ramcode[0], 0x0007);
            // enable write
            REG_WM(REG_SC_DNR(0x61), BIT8, BIT8);
        }
    }
    else if (pIP_Info->u8TabType == PQ_TABTYPE_SRAM_COLOR_GAIN_SNR)
    {
        // SRAM COLOR GAIN SNR
        SC_BK_SWICH(REG_SC_BK_DNR);

        for ( i=0; i<8; i++)
        {
            while (REG_RH(REG_SC_DNR(0x61)) & BIT1);

            // address
            REG_WM(REG_SC_DNR(0x66), i, 0x00FF );
            u16Gain_SNR = (REG_RL(REG_SC_DNR(0x65)) & 0x001F);
            u8Ramcode[0] = pIP_Info->pIPTable[pIP_Info->u8TabIdx * PQ_IP_C_SRAM1_SIZE_Main + j];
            u8Ramcode[0] = (u8Ramcode[0] <<8) | u16Gain_SNR;
            PQ_DBG(printk(" %02x ", u8Ramcode[0] ));
            j++;
            PQ_DBG(printk("\n"));

            REG_WM(REG_SC_DNR(0x64), u8Ramcode[0], 0x1F1F);
            // enable write
            REG_WM(REG_SC_DNR(0x61), BIT9, BIT9);
        }
    }
    else if (pIP_Info->u8TabType == PQ_TABTYPE_SRAM_COLOR_GAIN_DNR)
    {
        // SRAM COLOR GAIN DNR
        SC_BK_SWICH(REG_SC_BK_DNR);

        for ( i=0; i<8; i++)
        {
            while (REG_RH(REG_SC_DNR(0x61)) & BIT1);

            // address
            REG_WM(REG_SC_DNR(0x66), i, 0x00FF );
            u16Gain_DNR = REG_RR(REG_SC_DNR(0x65)) & 0x1F00;
            u8Ramcode[0] = pIP_Info->pIPTable[pIP_Info->u8TabIdx * PQ_IP_C_SRAM1_SIZE_Main + j];
            u8Ramcode[0] = u8Ramcode[0] | u16Gain_DNR;
            PQ_DBG(printk(" %02x ", u8Ramcode[0] ));
            j++;
            PQ_DBG(printk("\n"));

            REG_WM(REG_SC_DNR(0x64), u8Ramcode[0], 0x1F1F);
            // enable write
            REG_WM(REG_SC_DNR(0x61), BIT9, BIT9);
        }
    }
    SC_BK_SWICH(REG_SC_BK_GOPINT);
    REG_WM(REG_SC_GOPINT(0x03), 0x1000, 0x1000);
    SC_BK_RESTORE;
}


void MDrv_SCMAP_DumpScalerRegTable_AP(EN_IP_Info_AP* pIP_Info)
{
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;
    U8 u8CurBank;
    U8 u8LastBank = 0xff;
    BOOL bHiByte;
    U16 u16tabidx =0;


    PQ_DBG(printk("tab: sc\n"));
    if (pIP_Info->u8TabIdx >= pIP_Info->u8TabNums){
        PQ_DBG(printk("[PQ]IP_Info error: Scaler Reg Table\n"));
        return;
    }
    SC_BK_STORE;
    while (--u16timeout)
    {
        u16Addr = (pIP_Info->pIPTable[u16tabidx]<<8) + pIP_Info->pIPTable[u16tabidx +1];
        u8Mask  = pIP_Info->pIPTable[u16tabidx +REG_ADDR_SIZE];
        u8Value = pIP_Info->pIPTable[u16tabidx +REG_ADDR_SIZE +REG_MASK_SIZE +pIP_Info->u8TabIdx];

        if (u16Addr == _END_OF_TBL_) // check end of table
            break;

        u8CurBank = ((u16Addr & 0x7FFF) >> 8);
        bHiByte = (u16Addr & 0x8000) ? TRUE : FALSE;
        if (u8CurBank != u8LastBank)
        {
            PQ_DBG(printk("<<bankswitch=%02x>>\n", u8CurBank));
            SC_BK_SWICH(u8CurBank);
            u8LastBank = u8CurBank;
        }

        PQ_DBG(printk("[addr=%04x, msk=%02x, val=%02x]\n",
            REG_SC_BANK((u16Addr & 0x00FF))|bHiByte, u8Mask, u8Value));

        if (bHiByte)
        {
            REG_WM(REG_SC_BANK((u16Addr & 0x00FF)), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
        }
        else
        {
            REG_WM(REG_SC_BANK((u16Addr & 0x00FF)), (U16)u8Value, (U16)u8Mask);
        }
        u16tabidx = u16tabidx + REG_ADDR_SIZE+REG_MASK_SIZE + pIP_Info->u8TabNums;
    }
    SC_BK_RESTORE;
    if (u16timeout==0)
        assert(0);

}

void MDrv_SCMAP_DumpCombRegTable_AP(EN_IP_Info_AP* pIP_Info)
{
    U16 u16timeout = 0xffff;
    U16 u16Addr;
    U8 u8Mask;
    U8 u8Value;
    U8 u8CurBank;
    U8 u8LastBank = 0xff;
    BOOL bHiByte;
    U16 u16tabidx =0;

    PQ_DBG(printk("tab: comb\n"));
    if (pIP_Info->u8TabIdx >= pIP_Info->u8TabNums){
        PQ_DBG(printk("[PQ]IP_Info error: Comb Reg Table\n"));
        return;
    }
    COMB_BK_STORE;
    while (--u16timeout)
    {
        u16Addr = (pIP_Info->pIPTable[u16tabidx]<<8) + pIP_Info->pIPTable[u16tabidx +1];
        u8Mask  = pIP_Info->pIPTable[u16tabidx +REG_ADDR_SIZE];
        u8Value = pIP_Info->pIPTable[u16tabidx +REG_ADDR_SIZE +REG_MASK_SIZE +pIP_Info->u8TabIdx];

        if (u16Addr == _END_OF_TBL_) // check end of table
            break;

        u8CurBank = ((u16Addr & 0x7FFF) >> 8);
        bHiByte = (u16Addr & 0x8000) ? TRUE : FALSE;

        if (u8CurBank != u8LastBank)
        {
            PQ_DBG(printk("<<bankswitch=%02x>>\n", u8CurBank));
            COMB_BK_SWICH(u8CurBank);
            u8LastBank = u8CurBank;
        }

        PQ_DBG(printk("[addr=%04x, msk=%02x, val=%02x]\n",
            REG_COMB_BANK((u16Addr & 0x00FF))|bHiByte, u8Mask, u8Value));

        if (bHiByte)
        {
            REG_WM(REG_COMB_BANK((u16Addr & 0x00FF)), ((U16)u8Value) << 8, ((U16)u8Mask) << 8);
        }
        else
        {
            REG_WM(REG_COMB_BANK((u16Addr & 0x00FF)), (U16)u8Value, (U16)u8Mask);
        }
        u16tabidx = u16tabidx + REG_ADDR_SIZE+REG_MASK_SIZE + pIP_Info->u8TabNums; // next
    }
    COMB_BK_RESTORE;
    if (u16timeout==0)
        assert(0);
}

void MDrv_SCMAP_DumpTable(EN_IP_Info* pIP_Info)
{
/*
    // 20090915 daniel.huang: fix sram filter not be initialized after STR power on
    // to save loading SRAM table time, SRAM are only downloaded
    // when current SRAM table is different to previous SRAM table
    static U8 _u8SRAM1Table=0xFF;
    static U8 _u8SRAM2Table=0xFF;
    static U8 _u8SRAM3Table=0xFF;
    static U8 _u8SRAM4Table=0xFF;
    static U8 _u8C_SRAM1Table=0xFF;
    static U8 _u8C_SRAM2Table=0xFF;
    static U8 _u8C_SRAM3Table=0xFF;
    static U8 _u8C_SRAM4Table=0xFF;
    static U8 _u8Color_SRAMTable=0xFF;
    static U8 _u8Color_SNR_SRAMTable=0xFF;
    static U8 _u8Color_DNR_SRAMTable=0xFF; */

    if (pIP_Info->pIPTable == NULL)
        return;

    switch(pIP_Info->u8TabType )
    {
    case PQ_TABTYPE_SCALER:
        MDrv_SCMAP_DumpScalerRegTable(pIP_Info);
        break;
    case PQ_TABTYPE_COMB:
        MDrv_SCMAP_DumpCombRegTable(pIP_Info);
        break;
    case PQ_TABTYPE_GENERAL:
        MDrv_SCMAP_DumpGeneralRegTable(pIP_Info);
        break;
    case PQ_TABTYPE_SRAM1:
        if (_u8SRAM1Table != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old sram1: %u, new sram1: %u\n",
                (U16)_u8SRAM1Table, (U16)pIP_Info->u8TabIdx));
            _u8SRAM1Table = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same sram1: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;
    case PQ_TABTYPE_SRAM2:
        if (_u8SRAM2Table != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old sram2: %u, new sram2: %u\n",
                (U16)_u8SRAM2Table, (U16)pIP_Info->u8TabIdx));
            _u8SRAM2Table = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same sram2: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;

     case PQ_TABTYPE_SRAM3:
        if (_u8SRAM3Table != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old sram3: %u, new sram3: %u\n",
                (U16)_u8SRAM3Table, (U16)pIP_Info->u8TabIdx));
            _u8SRAM3Table = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same sram3: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;

    case PQ_TABTYPE_SRAM4:
        if (_u8SRAM4Table != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old sram4: %u, new sram4: %u\n",
                (U16)_u8SRAM4Table, (U16)pIP_Info->u8TabIdx));
            _u8SRAM4Table = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same sram4: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;

   case PQ_TABTYPE_C_SRAM1:
        if (_u8C_SRAM1Table != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old c_sram1: %u, new c_sram1: %u\n",
                (U16)_u8C_SRAM1Table, (U16)pIP_Info->u8TabIdx));
            _u8C_SRAM1Table = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same c_sram1: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;
    case PQ_TABTYPE_C_SRAM2:
        if (_u8C_SRAM2Table != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old c_sram2: %u, new c_sram2: %u\n",
                (U16)_u8C_SRAM2Table, (U16)pIP_Info->u8TabIdx));
            _u8C_SRAM2Table = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same c_sram2: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;
    case PQ_TABTYPE_C_SRAM3:
        if (_u8C_SRAM3Table != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old c_sram3: %u, new c_sram3: %u\n",
                (U16)_u8C_SRAM3Table, (U16)pIP_Info->u8TabIdx));
            _u8C_SRAM3Table = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same c_sram3: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;
    case PQ_TABTYPE_C_SRAM4:
        if (_u8C_SRAM4Table != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old c_sram4: %u, new c_sram4: %u\n",
                (U16)_u8C_SRAM4Table, (U16)pIP_Info->u8TabIdx));
            _u8C_SRAM4Table = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same c_sram4: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;
    case PQ_TABTYPE_SRAM_COLOR_INDEX:
        if (_u8Color_SRAMTable != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old color sram: %u, new color sram: %u\n",
                (U16)_u8Color_SRAMTable, (U16)pIP_Info->u8TabIdx));
            _u8Color_SRAMTable = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same color sram: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;
    case PQ_TABTYPE_SRAM_COLOR_GAIN_SNR:
        if (_u8Color_SNR_SRAMTable != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old color snr sram: %u, new color snr sram: %u\n",
                (U16)_u8Color_SNR_SRAMTable, (U16)pIP_Info->u8TabIdx));
            _u8Color_SNR_SRAMTable = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same color snr sram: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;
    case PQ_TABTYPE_SRAM_COLOR_GAIN_DNR:
        if (_u8Color_DNR_SRAMTable != pIP_Info->u8TabIdx){
            PQ_DBG(printk("old color dnr sram: %u, new color dnr sram: %u\n",
                (U16)_u8Color_DNR_SRAMTable, (U16)pIP_Info->u8TabIdx));
            _u8Color_DNR_SRAMTable = pIP_Info->u8TabIdx;
            MDrv_SCMAP_DumpFilterTable(pIP_Info);
        }
        else{
            PQ_DBG(printk("use the same color dnr sram: %u\n", (U16)pIP_Info->u8TabIdx));
        }
        break;
    default:
        PQ_DBG(printk("[PQ]DumpTable:unknown type\n"));
        break;
    }
}

U8 MDrv_SCMAP_GetTableIndex(U16 u16PQSrcType, U8 u8PQIPIdx)
{
    if (u16PQSrcType >=PQTableInfo[SC_MAIN_WINDOW].u8PQ_InputType_Num){
        PQ_DBG(printk("[PQ]invalid input type\n"));
        return PQ_IP_NULL;
    }
    if (u8PQIPIdx >= PQTableInfo[SC_MAIN_WINDOW].u8PQ_IP_Num){
        PQ_DBG(printk("[PQ]invalid ip type\n"));
        return PQ_IP_NULL;
    }

    return PQTableInfo[SC_MAIN_WINDOW].pQuality_Map_Aray[u16PQSrcType * PQTableInfo[SC_MAIN_WINDOW].u8PQ_IP_Num + u8PQIPIdx];
}

static void _MDrv_SCMAP_GetTable(U8 u8TabIdx, U8 u8PQIPIdx,EN_IP_Info *pip_Info)
{
    _u8PQTabIdx[u8PQIPIdx] = u8TabIdx;
    if (u8TabIdx != PQ_IP_NULL && u8TabIdx != PQ_IP_COMM) {
        pip_Info->pIPTable  = PQTableInfo[SC_MAIN_WINDOW].pIPTAB_Info[u8PQIPIdx].pIPTable;
        pip_Info->u8TabNums = PQTableInfo[SC_MAIN_WINDOW].pIPTAB_Info[u8PQIPIdx].u8TabNums;
        pip_Info->u8TabType = PQTableInfo[SC_MAIN_WINDOW].pIPTAB_Info[u8PQIPIdx].u8TabType;
        pip_Info->u8TabIdx = u8TabIdx;
    }
    else if (u8TabIdx == PQ_IP_COMM) {
        pip_Info->pIPTable = PQTableInfo[SC_MAIN_WINDOW].pIPTAB_Info[u8PQIPIdx].pIPCommTable;
        pip_Info->u8TabNums = 1;
        pip_Info->u8TabType = PQTableInfo[SC_MAIN_WINDOW].pIPTAB_Info[u8PQIPIdx].u8TabType;
        pip_Info->u8TabIdx = 0;
    }
    else {
        pip_Info->pIPTable  = 0;
        pip_Info->u8TabNums = 0;
        pip_Info->u8TabType = 0;
        pip_Info->u8TabIdx = 0;
    }
}

void _MDrv_SCMAP_LoadTable(U8 u8TabIdx, U8 u8PQIPIdx)
{
    EN_IP_Info ip_Info;
    _MDrv_SCMAP_GetTable(u8TabIdx, u8PQIPIdx, &ip_Info);
    MDrv_SCMAP_DumpTable(&ip_Info);
}

void _MDrv_SCMAP_LoadCommTable(void)
{
    U8 i;
    EN_IP_Info ip_Info;

    PQ_DBG(printk("[PQ]LoadCommTable\n"));
    for (i=0; i<PQTableInfo[SC_MAIN_WINDOW].u8PQ_IP_Num; i++){
        //#if (PQTBL_REGTYPE == PQTBL_NORMAL)
        if (i == PQ_IP_SRAM1_Main   || i == PQ_IP_SRAM2_Main ||
            i == PQ_IP_SRAM3_Main   || i == PQ_IP_SRAM4_Main ||
            i == PQ_IP_C_SRAM1_Main || i == PQ_IP_C_SRAM2_Main ||
            i == PQ_IP_C_SRAM3_Main || i == PQ_IP_C_SRAM4_Main)
            continue;
        //#endif

        PQ_DBG(printk("  IP:%u\n", (U16)i));
        _MDrv_SCMAP_GetTable(PQ_IP_COMM, i, &ip_Info);
        MDrv_SCMAP_DumpTable(&ip_Info);
    }
}

U8 _MDrv_SCMAP_GetXRuleTableIndex(U8 u8XRuleType, U8 u8XRuleIdx, U8 u8XRuleIP)
{
    U8 *pArray = PQTableInfo[SC_MAIN_WINDOW].pXRule_Array[u8XRuleType];
    //pXRule_Array[u8XRuleIdx][u8XRuleIP];
    return pArray[((U16)u8XRuleIdx) * PQTableInfo[SC_MAIN_WINDOW].u8PQ_XRule_IP_Num[u8XRuleType] + u8XRuleIP];
}

U8 _MDrv_SCMAP_GetXRuleIPIndex(U8 u8XRuleType, U8 u8XRuleIP)
{
    U8 *pArray;
    SRULE_DBG(printk("XRuleType:%u, XRuleIP:%u\n", u8XRuleType, u8XRuleIP));
    pArray = PQTableInfo[SC_MAIN_WINDOW].pXRule_IP_Index[u8XRuleType];
    //pXRule_IP_Index[u8XRuleIP]
    return pArray[u8XRuleIP];
}

U8 _MDrv_SCMAP_GetXRuleIPNum(U8 u8XRuleType)
{
    return PQTableInfo[SC_MAIN_WINDOW].u8PQ_XRule_IP_Num[u8XRuleType];
}

void _MDrv_SCMAP_LoadTableBySrcType(U16 u16PQSrcType, U8 u8PQIPIdx)
{
    EN_IP_Info ip_Info;
    U8 QMIPtype_size,i;
    U8 u8TabIdx;

    if (u8PQIPIdx==PQ_IP_ALL)
    {
        QMIPtype_size=PQTableInfo[SC_MAIN_WINDOW].u8PQ_IP_Num;
        u8PQIPIdx=0;
    }
    else
    {
        QMIPtype_size=1;
    }

    //for debug
    //msAPI_Scaler_SetBlueScreen(DISABLE, 0x00);
    //msAPI_Scaler_GenerateBlackVideo(FALSE);

    for(i=0; i<QMIPtype_size; i++, u8PQIPIdx++)
    {
        if (PQTableInfo[SC_MAIN_WINDOW].pSkipRuleIP[u8PQIPIdx]) {
            PQ_DBG(printk("skip ip idx:%u\n", u8PQIPIdx));
            continue;
        }

        u8TabIdx = MDrv_SCMAP_GetTableIndex(u16PQSrcType, u8PQIPIdx);
        PQ_DBG(printk("[PQ]SrcType=%u, IPIdx=%u, TabIdx=%u\n",
            (U16)u16PQSrcType, (U16)u8PQIPIdx, (U16)u8TabIdx));

        _MDrv_SCMAP_GetTable(u8TabIdx, u8PQIPIdx, &ip_Info);
        MDrv_SCMAP_DumpTable(&ip_Info);

        //MDrv_Timer_Delayms(1500);
    }
}

void MDrv_SCMAP_Init(U8 u8QMapType)
{
    //PQTABLE_INFO *pPQTableInfo;
    //pPQTableInfo = &PQTableInfo[SC_MAIN_WINDOW];

    if (u8QMapType == 1)
    {
        PQTableInfo[SC_MAIN_WINDOW].pQuality_Map_Aray = (void*)QMAP_1366_Main;
    }
    else
    {
        PQTableInfo[SC_MAIN_WINDOW].pQuality_Map_Aray = (void*)QMAP_1920_Main;
    }

    // table config parameter
    PQTableInfo[SC_MAIN_WINDOW].u8PQ_InputType_Num = QM_INPUTTYPE_NUM_Main;
    PQTableInfo[SC_MAIN_WINDOW].u8PQ_IP_Num = PQ_IP_NUM_Main;
    PQTableInfo[SC_MAIN_WINDOW].pIPTAB_Info = (void*)PQ_IPTAB_INFO_Main;
    PQTableInfo[SC_MAIN_WINDOW].pSkipRuleIP = (void*)MST_SkipRule_IP_Main;

    PQTableInfo[SC_MAIN_WINDOW].u8PQ_XRule_IP_Num[PQ_HSDRule_ID_Main] = PQ_HSDRule_IP_NUM_Main;
    PQTableInfo[SC_MAIN_WINDOW].u8PQ_XRule_IP_Num[PQ_VSDRule_ID_Main] = PQ_VSDRule_IP_NUM_Main;
    PQTableInfo[SC_MAIN_WINDOW].u8PQ_XRule_IP_Num[PQ_HSPRule_ID_Main] = PQ_HSPRule_IP_NUM_Main;
    PQTableInfo[SC_MAIN_WINDOW].u8PQ_XRule_IP_Num[PQ_VSPRule_ID_Main] = PQ_VSPRule_IP_NUM_Main;
    PQTableInfo[SC_MAIN_WINDOW].u8PQ_XRule_IP_Num[PQ_CSCRule_ID_Main] = PQ_CSCRule_IP_NUM_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_IP_Index[PQ_HSDRule_ID_Main] = (void*)MST_HSDRule_IP_Index_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_IP_Index[PQ_VSDRule_ID_Main] = (void*)MST_VSDRule_IP_Index_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_IP_Index[PQ_HSPRule_ID_Main] = (void*)MST_HSPRule_IP_Index_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_IP_Index[PQ_VSPRule_ID_Main] = (void*)MST_VSPRule_IP_Index_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_IP_Index[PQ_CSCRule_ID_Main] = (void*)MST_CSCRule_IP_Index_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_Array[PQ_HSDRule_ID_Main] = (void*)MST_HSDRule_Array_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_Array[PQ_VSDRule_ID_Main] = (void*)MST_VSDRule_Array_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_Array[PQ_HSPRule_ID_Main] = (void*)MST_HSPRule_Array_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_Array[PQ_VSPRule_ID_Main] = (void*)MST_VSPRule_Array_Main;
    PQTableInfo[SC_MAIN_WINDOW].pXRule_Array[PQ_CSCRule_ID_Main] = (void*)MST_CSCRule_Array_Main;

    _MDrv_SCMAP_LoadCommTable();

    // 20090915 daniel.huang: fix sram filter not be initialized after STR power on
    _u8SRAM1Table=0xFF;
    _u8SRAM2Table=0xFF;
    _u8SRAM3Table=0xFF;
    _u8SRAM4Table=0xFF;
    _u8C_SRAM1Table=0xFF;
    _u8C_SRAM2Table=0xFF;
    _u8C_SRAM3Table=0xFF;
    _u8C_SRAM4Table=0xFF;
    _u8Color_SRAMTable=0xFF;
    _u8Color_SNR_SRAMTable=0xFF;
    _u8Color_DNR_SRAMTable=0xFF;

}

void MDrv_Set_QMap(U8 u8WinIdx)
{
    PQTAB_DBG(printk("[PQ_LoadSettings]"));
    _MDrv_SCMAP_LoadTableBySrcType(_u16PQSrcType[u8WinIdx], PQ_IP_ALL);
    PQTAB_DBG(printk("...done\n"));
}

void MDrv_SCMAP_DesideSrcType(U8 u8WinIdx, PSC_SOURCE_INFO_t pSrcInfo)
{
    _u16PQSrcType[u8WinIdx] = QM_InputSourceToIndex(pSrcInfo);
    pSrcInfo->u16PQSrcType = _u16PQSrcType[u8WinIdx];
    PQTAB_DBG(printk("[PQ_DesideSrcType] window=%u, SrcType=%u\n",
              u8WinIdx, _u16PQSrcType[u8WinIdx]));
}

U16 MDrv_SCMAP_GetSrcTypeIndex(U8 u8WinIdx)
{
    return _u16PQSrcType[u8WinIdx];
}

//for reseting film setting, [091012_Leo]
void MDrv_SCMAP_SetFilmMode(U8 u8WinIdx)
{
    U8 u8TabIdx_Film;

    u8TabIdx_Film = MDrv_SCMAP_GetTableIndex(_u16PQSrcType[u8WinIdx], PQ_IP_Film_Main);

    PQTAB_DBG(printk("u8TabIdx_Film=%u\n", u8TabIdx_Film));
    _MDrv_SCMAP_LoadTable(u8TabIdx_Film, PQ_IP_Film_Main);

}

BOOL MDrv_SCMAP_LoadScalingTable(U8 u8WinIdx,
                              U8 eXRuleType,
                              BOOL bPreV_ScalingDown,
                              BOOL bInterlace,
                              BOOL bColorSpaceYUV,
                              U16 u16InputSize,
                              U16 u16SizeAfterScaling)
{
    BOOL bRet = 1; // default is adv mode
    U8 u8XRuleIP;
    U8 u8IPIdx;
    U8 u8TabIdx;
    U8 u8XRuleIdx;

    U32 u32Ratio;

    if (u16InputSize == 0)
    {
        //assert(0);
        return TRUE;
    }
    u32Ratio = ((U32) u16SizeAfterScaling * 1000) / u16InputSize;

    if (eXRuleType > 3)
        assert(0);

    //bEnPQDbg = TRUE;

    SRULE_DBG(printk("[PQ_LoadScalingTable] HSD/VSD/HSP/VSP:%u\n", (U16)eXRuleType));


    for(u8XRuleIP=0; u8XRuleIP<_MDrv_SCMAP_GetXRuleIPNum((U8)eXRuleType); u8XRuleIP++)
    {
        u8TabIdx = PQ_IP_NULL;
        u8XRuleIdx = 0xff;
        u8IPIdx = _MDrv_SCMAP_GetXRuleIPIndex(eXRuleType, u8XRuleIP);

        SRULE_DBG(printk("SRuleIP=%u, IPIdx=%u, input=%u, output=%u, ratio=%u, ",
                 (U16)u8XRuleIP, (U16)u8IPIdx,
                 u16InputSize, u16SizeAfterScaling, u32Ratio));

        if(bPreV_ScalingDown && bInterlace) {
            u8XRuleIdx = PQ_HSDRule_PreV_ScalingDown_Interlace_Main;
            u8TabIdx = _MDrv_SCMAP_GetXRuleTableIndex(eXRuleType, u8XRuleIdx, u8XRuleIP);
        }
        else if(bPreV_ScalingDown && !bInterlace) {
            u8XRuleIdx = PQ_HSDRule_PreV_ScalingDown_Progressive_Main;
            u8TabIdx = _MDrv_SCMAP_GetXRuleTableIndex(eXRuleType, u8XRuleIdx, u8XRuleIP);
        }
        //SRULE_DBG(printk("[[DEBUG: TabIdx:%u, SRuleIdx:%u, SRuleIP:%u]]\n",
        //    MST_SRULE_Array_Main[u8XRuleIdx][u8SRuleIP], u8XRuleIdx, u8SRuleIP));

        if (u8TabIdx != PQ_IP_NULL) {
            SRULE_DBG(printk("u8XRuleIdx: PreV down, interlace:%u", bInterlace));
            SRULE_DBG(printk("(a)tabidx=%u\n", (U16)u8TabIdx));
        }


        if (u8TabIdx == PQ_IP_NULL)
        {
            if(u32Ratio > 1000)
            {
                u8TabIdx = MDrv_SCMAP_GetTableIndex(_u16PQSrcType[u8WinIdx], u8IPIdx);
                SRULE_DBG(printk("u8XRuleIdx: >x1, "));
                SRULE_DBG(printk("(c)tabidx=%u\n", (U16)u8TabIdx));
            }
            else
            {
                if (bColorSpaceYUV)
                {
                    if(u32Ratio == 1000)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_10x_YUV_Main;
                    else if(u32Ratio >= 900)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_09x_YUV_Main;
                    else if(u32Ratio >= 800)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_08x_YUV_Main;
                    else if(u32Ratio >= 700)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_07x_YUV_Main;
                    else if(u32Ratio >= 600)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_06x_YUV_Main;
                    else if(u32Ratio >= 500)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_05x_YUV_Main;
                    else if(u32Ratio >= 400)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_04x_YUV_Main;
                    else if(u32Ratio >= 300)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_03x_YUV_Main;
                    else if(u32Ratio >= 200)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_02x_YUV_Main;
                    else if(u32Ratio >= 100)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_01x_YUV_Main;
                    else
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_00x_YUV_Main;
                }
                else
                {
                    if(u32Ratio == 1000)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_10x_RGB_Main;
                    else if(u32Ratio >= 900)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_09x_RGB_Main;
                    else if(u32Ratio >= 800)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_08x_RGB_Main;
                    else if(u32Ratio >= 700)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_07x_RGB_Main;
                    else if(u32Ratio >= 600)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_06x_RGB_Main;
                    else if(u32Ratio >= 500)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_05x_RGB_Main;
                    else if(u32Ratio >= 400)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_04x_RGB_Main;
                    else if(u32Ratio >= 300)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_03x_RGB_Main;
                    else if(u32Ratio >= 200)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_02x_RGB_Main;
                    else if(u32Ratio >= 100)
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_01x_RGB_Main;
                    else
                        u8XRuleIdx = PQ_HSDRule_ScalingDown_00x_RGB_Main;
                }

                SRULE_DBG(printk("u8XRuleIdx=%u, ", (U16)u8XRuleIdx));
                if (u8XRuleIdx == 0xFF) {
                    assert(0);
                    return 1;
                }

                u8TabIdx = _MDrv_SCMAP_GetXRuleTableIndex(eXRuleType, u8XRuleIdx, u8XRuleIP);
                if (u8TabIdx == PQ_IP_NULL)
                {
                    u8TabIdx = MDrv_SCMAP_GetTableIndex(_u16PQSrcType[u8WinIdx], u8IPIdx);
                    SRULE_DBG(printk("(d)tabidx=%u\n", (U16)u8TabIdx));
                }
                else
                {
                    SRULE_DBG(printk("(e)tabidx=%u\n", (U16)u8TabIdx));
                }
            }
        }

        _MDrv_SCMAP_LoadTable(u8TabIdx, u8IPIdx);

        bRet = (u8TabIdx != PQ_IP_HSD_Y_CB_Main); // PreHSDMode - 0:Cb, 1:Adv
    }
    //bEnPQDbg = FALSE;

    return bRet;
}

//////////////////////////////////////////////////////////////////
// Load MemoryFormat/MADi table
//
// return:  bit per pixel used in memory
//
void MDrv_SCMAP_SetMemFormat(U8 u8WinIdx, U8 *u8BitsPerPixel, BOOL *bMemFormat422)
{
    U8 u8TabIdx_MemFormat, u8TabIdx_MADi, u8TabIdx_444To422, u8TabIdx_422To444;

    u8TabIdx_MADi = MDrv_SCMAP_GetTableIndex(_u16PQSrcType[u8WinIdx], PQ_IP_MADi_Main);
    u8TabIdx_MemFormat = MDrv_SCMAP_GetTableIndex(_u16PQSrcType[u8WinIdx], PQ_IP_MemFormat_Main);
    u8TabIdx_444To422 = MDrv_SCMAP_GetTableIndex(_u16PQSrcType[u8WinIdx], PQ_IP_444To422_Main);
    u8TabIdx_422To444 = MDrv_SCMAP_GetTableIndex(_u16PQSrcType[u8WinIdx], PQ_IP_422To444_Main);

    PQTAB_DBG(printk("u8TabIdx_MemFormat=%u\n", u8TabIdx_MemFormat));
    _MDrv_SCMAP_LoadTable(u8TabIdx_MemFormat, PQ_IP_MemFormat_Main);

    PQTAB_DBG(printk("u8TabIdx_MADi=%u\n", u8TabIdx_MADi));
    _MDrv_SCMAP_LoadTable(u8TabIdx_MADi, PQ_IP_MADi_Main);

    PQTAB_DBG(printk("u8TabIdx_444To422=%u\n", u8TabIdx_444To422));
    _MDrv_SCMAP_LoadTable(u8TabIdx_444To422, PQ_IP_444To422_Main);

    PQTAB_DBG(printk("u8TabIdx_422To444=%u\n", u8TabIdx_422To444));
    _MDrv_SCMAP_LoadTable(u8TabIdx_422To444, PQ_IP_422To444_Main);

    if (u8TabIdx_MemFormat == PQ_IP_MemFormat_422MF_Main)
    {
        switch(u8TabIdx_MADi)
        {
        case PQ_IP_MADi_25_4R_Main:
        case PQ_IP_MADi_25_2R_Main:
        case PQ_IP_MADi_27_4R_Main:
        case PQ_IP_MADi_27_2R_Main:
        case PQ_IP_MADi_P_MODE_MOT10_Main:
            *u8BitsPerPixel = 24;
            break;
        case PQ_IP_MADi_P_MODE_MOT8_Main:
            *u8BitsPerPixel = 20;
            break;
        case PQ_IP_MADi_P_MODE_Main:
            *u8BitsPerPixel = 16;
            break;
        default:
            *u8BitsPerPixel = 24;
            assert(0);
            break;
        }
        *bMemFormat422 = TRUE;
    }
    else // 444
    {
        if (u8TabIdx_MemFormat == PQ_IP_MemFormat_444_10BIT_Main)
        {
            *u8BitsPerPixel = 32;
        }
        else    //PQ_IP_MemFormat_444_8BIT_Main
        {
            *u8BitsPerPixel = 24;
        }
        *bMemFormat422 = FALSE;

    }
}

#if 0
void MDrv_SCMAP_Set420upsampling(U8 u8WinIdx,
                              BOOL bFBL,
                              BOOL bPreV_ScalingDown,
                              U16 u16V_CropStart)
{
    U8 u8TabIdx;
    U8 u8Bank;

    u8TabIdx = MDrv_SCMAP_GetTableIndex(u8WinIdx, PQ_IP_420CUP_Main);

    PQTAB_DBG(printk("[PQ_Set420upsampling]: SrcType:%u, FBL:%u, PreV down:%u, V_CropStart:%u, u8TabIdx=%u, ",
        _u16PQSrcType[u8WinIdx], bFBL, bPreV_ScalingDown, u16V_CropStart, u8TabIdx));

    if ((u8TabIdx == PQ_IP_420CUP_ON_Main) && !bPreV_ScalingDown && !bFBL)
    {
        PQTAB_DBG(printk("UVShift: on\n"));
        MDrv_VOP_EnableUVShift(ENABLE);
        MDrv_SCMAP_LoadTable(u8WinIdx, PQ_IP_420CUP_ON_Main, PQ_IP_420CUP_Main);
    }
    else    // P mode should not do UV shift
    {
        PQTAB_DBG(printk("UVShift: off\n"));
        MDrv_VOP_EnableUVShift(DISABLE);
        MDrv_SCMAP_LoadTable(u8WinIdx, PQ_IP_420CUP_OFF_Main, PQ_IP_420CUP_Main);
    }

    u8Bank = MDrv_ReadByte(BK_SELECT_00);
    MDrv_WriteByte(BK_SELECT_00, REG_BANK_EODI);

    if ((u16V_CropStart & 0x3) == 0)      // crop lines are multiple of 4
        MDrv_Write2Byte(L_BK_EODI(0x76), 0x6666);
    else if ((u16V_CropStart & 0x1) == 0) // crop lines are multiple of 2
        MDrv_Write2Byte(L_BK_EODI(0x76), 0x9999);
    else
        ASSERT(0);

    MDrv_WriteByte(BK_SELECT_00, u8Bank);
}

void MDrv_SCMAP_SetFilmMode(U8 u8WinIdx, BOOL bEnable)
{
    U8 u8TabIdx;

    PQTAB_DBG(printk("[PQ_SetFilmMode]: PQTabType=%u, enable=%u\n", u8WinIdx, bEnable));

    if (bEnable) u8TabIdx = MDrv_SCMAP_GetTableIndex(u8WinIdx, PQ_IP_Film32_Main);
    else    u8TabIdx = PQ_IP_Film32_OFF_Main;
    MDrv_SCMAP_LoadTable(u8WinIdx, u8TabIdx, PQ_IP_Film32_Main);

    if (bEnable) u8TabIdx = MDrv_SCMAP_GetTableIndex(u8WinIdx, PQ_IP_Film22_Main);
    else    u8TabIdx = PQ_IP_Film22_OFF_Main;
    MDrv_SCMAP_LoadTable(u8WinIdx, u8TabIdx, PQ_IP_Film22_Main);
}


void MDrv_SCMAP_SetNonLinearScaling(U8 u8WinIdx, U8 u8Level, BOOL bEnable)
{
    U8 u8TabIdx;
    PQTAB_DBG(printk("[PQ_SetNonLinearScaling]: Level=%u, enable=%u\n", u8Level, bEnable));

    if (bEnable)
    {
        switch(mvideo_pnl_get_width())
        {
        case 1920:
            switch(u8Level)
            {
            case 2:
                u8TabIdx = PQ_IP_HnonLinear_H_1920_2_Main; break;
            case 1:
                u8TabIdx = PQ_IP_HnonLinear_H_1920_1_Main; break;
            case 0:
            default:
                 u8TabIdx = PQ_IP_HnonLinear_H_1920_0_Main; break;
            }
            break;
        case 1680:
            u8TabIdx = PQ_IP_HnonLinear_H_1680_Main;
            break;
        case 1440:
            u8TabIdx = PQ_IP_HnonLinear_H_1440_Main;
            break;
        case 1366:
            switch(u8Level)
            {
            case 2:
                u8TabIdx = PQ_IP_HnonLinear_H_1366_2_Main; break;
            case 1:
                u8TabIdx = PQ_IP_HnonLinear_H_1366_1_Main; break;
            case 0:
            default:
                 u8TabIdx = PQ_IP_HnonLinear_H_1366_0_Main; break;
            }
            break;
        default:
            u8TabIdx = PQ_IP_HnonLinear_OFF_Main;
            break;
        }
    }
    else
    {
        u8TabIdx = PQ_IP_HnonLinear_OFF_Main;
    }

    MDrv_SCMAP_LoadTable(u8WinIdx, u8TabIdx, PQ_IP_HnonLinear_Main);

}

#endif

