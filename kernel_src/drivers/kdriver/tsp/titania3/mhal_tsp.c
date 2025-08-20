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
// file    mhal_tsp.c
// @brief  Transport Stream Processer (TSP) Driver
// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/io.h>
#include "mdrv_types.h"

#include "Board.h"
#include "mhal_chiptop_reg.h"
#include "mhal_tsp.h"

#define TSP_VA2PA(addr)      (U32)virt_to_phys((void*)addr);

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned char _tsp_dat[] __attribute__((section(".tspdat"))) =
{
    #include "fw_tsp.dat"
};

static U16                      u16LastAddr0;
static U16                      u16LastAddr1;
static U16                      u16LastWrite0;
static U16                      u16LastWrite1;
static U16                      u16LastRead0;
static U16                      u16LastRead1;

typedef struct {
    U16 r_count;
    U16 w_count;
    U32 r_ptr;
    U32 buf_size;
    U32 buf_end;
} sec_info_t;

//shared memory area is defined by firmware linker script
#define SHARED_MEM_START 0x4800
typedef struct {
    U32 sdr_base_pa;
    U32 cmd;
    U32 tsc_ready;
    U32 tsc_non_zero;
    U32 sec_overflow;
    sec_info_t sec_info[TSP_ENGINE_NUM][TSP_SECFLT_NUM];
} shared_mem_t;
static volatile shared_mem_t* shm = NULL;

#define TSP_CMD_TIMEOUT_MS      100    // timeout in ms

/////////////////////////////////////////////////////////////////////////////////////////////////////
// External functions
/////////////////////////////////////////////////////////////////////////////////////////////////////

U32 REG32_IndR(REG32 *reg)
{

    // set address
    REG16_T(ADDR_INDR_ADDR0)=   ((((U32)reg)>> 1) & 0xFFFF);
    REG16_T(ADDR_INDR_ADDR1)=   ((((U32)reg)>> 17) & 0xFFFF);

    mb();

    // set command
    REG16_T(ADDR_INDR_CTRL)=    (TSP_IDR_MCUWAIT | TSP_IDR_READ | TSP_IDR_START);

    mb();

    // get read value
    return ((U32)(REG16_T(ADDR_INDR_READ0))| ((U32)(REG16_T(ADDR_INDR_READ1)<< 16)));
}

void REG32_IndW(REG32 *reg, U32 value)
{
    // set address
    REG16_T(ADDR_INDR_ADDR0)=   ((((U32)reg)>> 1) & 0xFFFF);
    REG16_T(ADDR_INDR_ADDR1)=   ((((U32)reg)>> 17) & 0xFFFF);

    // set write value
    REG16_T(ADDR_INDR_WRITE0)=  (value & 0xFFFF);
    REG16_T(ADDR_INDR_WRITE1)=  ((value >> 16) & 0xFFFF);

    mb();

    // set command
    REG16_T(ADDR_INDR_CTRL)=    (TSP_IDR_MCUWAIT | TSP_IDR_WRITE | TSP_IDR_START);

    mb();
}

void
MHal_TSP_SetTSIFExtSync(U32 u32EngId, U32 u32Tsin, B16 bExtSync)
{
    REG32* u32RegAddr[] = {&_TspCtrl[u32EngId].Hw_Config0, &_TspCtrl[u32EngId].Hw_Config2};
    U32 u32RegValue[] = {REG32_R(u32RegAddr[0]), REG32_R(u32RegAddr[1])};
    U32 u32RegMask[] = {TSP_HW_CFG0_TSIF0_EXTSYNC, TSP_HW_CFG2_TSIF1_EXTSYNC};

    if (bExtSync)
    {
        SET_FLAG(u32RegValue[u32Tsin], u32RegMask[u32Tsin]);
    }
    else
    {
        RESET_FLAG(u32RegValue[u32Tsin], u32RegMask[u32Tsin]);
    }

    REG32_W(u32RegAddr[u32Tsin], u32RegValue[u32Tsin]);
}

void
MHal_TSP_SetTSIFType(U32 u32EngId, U32 u32Tsin, U32 u32TSIFType)
{
    REG32* u32RegAddr[] = {&_TspCtrl[u32EngId].Hw_Config0, &_TspCtrl[u32EngId].Hw_Config2};
    U32 u32RegValue[] = {REG32_R(u32RegAddr[0]), REG32_R(u32RegAddr[1])};
    U32 u32RegMask[] = {TSP_HW_CFG0_TSIF0_PARL, TSP_HW_CFG2_TSIF1_PARL};

    switch(u32TSIFType)
    {
        case E_HALTSP_TSIF_SERIAL:
            RESET_FLAG(u32RegValue[u32Tsin], u32RegMask[u32Tsin]);
            break;
        case E_HALTSP_TSIF_PARALLEL:
            SET_FLAG(u32RegValue[u32Tsin], u32RegMask[u32Tsin]);
            break;
    }

    REG32_W(u32RegAddr[u32Tsin], u32RegValue[u32Tsin]);
    MHal_TSP_OneBitSync_Enable();
    MHal_TSP_VarClk_Enable();
}

void MHal_TSP_isr_save_all(void)
{
    mb();

    // save address
    u16LastAddr0=               (U16)REG16_T(ADDR_INDR_ADDR0);
    u16LastAddr1=               (U16)REG16_T(ADDR_INDR_ADDR1);

    // save write
    u16LastWrite0=              (U16)REG16_T(ADDR_INDR_WRITE0);
    u16LastWrite1=              (U16)REG16_T(ADDR_INDR_WRITE1);

    // save read
    u16LastRead0=               (U16)REG16_T(ADDR_INDR_READ0);
    u16LastRead1=               (U16)REG16_T(ADDR_INDR_READ1);

    mb();
}

void MHal_TSP_isr_restore_all(void)
{
    // restore read
    REG16_T(ADDR_INDR_READ0)=   u16LastRead0;
    REG16_T(ADDR_INDR_READ1)=   u16LastRead1;

    // restore write
    REG16_T(ADDR_INDR_WRITE0)=  u16LastWrite0;
    REG16_T(ADDR_INDR_WRITE1)=   u16LastWrite1;

    // restore addr
    REG16_T(ADDR_INDR_ADDR0)=   u16LastAddr0;
    REG16_T(ADDR_INDR_ADDR1)=   u16LastAddr1;

    mb();
}

void MHal_TSP_SecFlt_SetMatchMask(REG_SecFlt *pSecFilter, U8 *pu8Match, U8 *pu8Mask)
{
    int i;

    for (i = 0; i < (TSP_FILTER_DEPTH/sizeof(U32)); i++)
    {
        REG32_IndW(&pSecFilter->Match[i], *(U32*)(&pu8Match[i<< 2]));
        REG32_IndW(&pSecFilter->Mask[i], ~*(U32*)(&pu8Mask[i<< 2])); // 1 is masked (don't care for hardware)
    }

#if 0  // T2 removed
    if (bNotMatch){
        for (i = 0; i < (TSP_FILTER_DEPTH/sizeof(U32)); i++)
        {
            REG32_IndW(&pSecFilter->NMatch[i], *((U32*)(&pu8Mask[i<<2])));
        }
    }
    else{
        for (i = 0; i < (TSP_FILTER_DEPTH/sizeof(U32)); i++)
        {
            REG32_IndW(&pSecFilter->NMatch[i], 0);
        }
    }
#else
     for (i = 0; i < (TSP_FILTER_DEPTH/sizeof(U32)); i++)
    {
        REG32_IndW(&pSecFilter->NMatch[i], 0);
    }
#endif // T2 removed

}

void MHal_TSP_SecFlt_SetType(REG_SecFlt *pSecFilter, U32 u32Type)
{
    REG32_IndW(&pSecFilter->Ctrl, (REG32_IndR(&pSecFilter->Ctrl) & ~TSP_SECFLT_TYPE_MASK) | (u32Type << TSP_SECFLT_TYPE_SHFT));
}

U32 MHal_TSP_SecFlt_GetType(REG_SecFlt *pSecFilter)
{
    return (HAS_FLAG(REG32_IndR(&pSecFilter->Ctrl), TSP_SECFLT_TYPE_MASK) >> TSP_SECFLT_TYPE_SHFT);
}

void MHal_TSP_PVR_SetBuffer(U32 u32BufStart0, U32 u32BufStart1, U32 u32BufSize)
{
    U32                     u32BufEnd;
#if 0 //double buffer
    // for buffer 0
    u32BufEnd=              u32BufStart0+ u32BufSize;

    REG32_W(&_TspCtrl[0].TsRec_Head, (u32BufStart0>>TSP_MIU_BASE_OFFSET));                // 16 bytes unit
    REG32_W(&_TspCtrl[0].TsRec_Tail, (u32BufEnd>>TSP_MIU_BASE_OFFSET));                   // 16 bytes unit

    // for buffer 1
    u32BufEnd=              u32BufStart1+ u32BufSize;
    REG16_T(ADDR_PVR_HEAD20)=   (u32BufStart1>>TSP_MIU_BASE_OFFSET) & 0xFFFF;
    REG16_T(ADDR_PVR_HEAD21)=   (u32BufStart1>>(16+TSP_MIU_BASE_OFFSET)) & 0x00FF;
    REG16_T(ADDR_PVR_TAIL20)=   (u32BufEnd>>TSP_MIU_BASE_OFFSET) & 0xFFFF;
    REG16_T(ADDR_PVR_TAIL21)=   (u32BufEnd>>(16+TSP_MIU_BASE_OFFSET)) & 0x00FF;

    mb();

    // enable PVR ping pong buffer
    REG32_W(&_TspCtrl[0].reg15b4,
            SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), TSP_PVR_PINGPONG));
#else //single buffer
    u32BufEnd=              u32BufStart0+ (u32BufSize<<1);

    REG32_W(&_TspCtrl[0].TsRec_Head, (u32BufStart0>>TSP_MIU_BASE_OFFSET));
    REG32_W(&_TspCtrl[0].TsRec_Mid,  ((u32BufStart0+u32BufSize)>>TSP_MIU_BASE_OFFSET));
    REG32_W(&_TspCtrl[0].TsRec_Tail, (u32BufEnd>>TSP_MIU_BASE_OFFSET));

    mb();
#endif
    // flush PVR buffer
    MHal_TSP_PVR_WaitFlush();
}

void MHal_TSP_PVR_SetMode(HalTSP_RecMode eRecMode)
{
    if (HAS_FLAG(eRecMode, 0x02))
    {
        // bypass mode
        REG32_W(&_TspCtrl[0].Hw_Config4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config4), TSP_HW_CFG4_PVR_PIDFLT_SEC));
        REG32_W(&_TspCtrl[0].reg15b4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), TSP_PVR_PID));
    }
    else
    {
        REG32_W(&_TspCtrl[0].Hw_Config4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config4), TSP_HW_CFG4_PVR_PIDFLT_SEC));
        REG32_W(&_TspCtrl[0].reg15b4,
                SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), TSP_PVR_PID));
    }
}

U32 MHal_TSP_PVR_GetCtrlMode(void)
{
    if (HAS_FLAG(REG32_R(&_TspCtrl[0].reg15b4), TSP_PVR_PID)){
        return E_HALTSP_REC_MODE_ENG1_FLTTYPE;
    }
    return E_HALTSP_REC_MODE_ENG1_BYPASS;
}

void MHal_TSP_PVR_Enable(B16 bEnable)
{
    //TO DO: Refine it for T3
    //20090723 - T2 LGE PVR performance tunning
    REG32_W(&_TspCtrl[0].reg15b4,
                SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4) & ~TSP_PVR_BURST_LEN_MASK, TSP_PVR_BURST_LEN_ONE<<TSP_PVR_BURST_LEN_SHIFT));

    if (bEnable){
        REG32_W(&_TspCtrl[0].Hw_Config4,
                SET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config4), TSP_HW_CFG4_PVR_ENABLE));
    }
    else{
        REG32_W(&_TspCtrl[0].Hw_Config4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config4), TSP_HW_CFG4_PVR_ENABLE));
    }
}

void MHal_TSP_filein_enable(B16 b_enable)
{
    // Richard: enable/disable file in timer as well
    //          file in could only walk through pid filter set 0.
    if (b_enable){
        REG32_W(&_TspCtrl[0].Hw_Config0,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config0), TSP_HW_CFG0_DATA_PORT_EN));
        //REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
        //        SET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_PVR_CMD_QUEUE_ENABLE| TSP_CTRL1_FILEIN_TIMER_ENABLE));
        REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
                SET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_PVR_CMD_QUEUE_ENABLE));
        REG32_W(&_TspCtrl[0].TSP_Ctrl,
                SET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_TSFILE_EN));
    }
    else{
        REG32_W(&_TspCtrl[0].Hw_Config0,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config0), TSP_HW_CFG0_DATA_PORT_EN));
        //REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
        //        RESET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_PVR_CMD_QUEUE_ENABLE| TSP_CTRL1_FILEIN_TIMER_ENABLE));
        REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_PVR_CMD_QUEUE_ENABLE));
        REG32_W(&_TspCtrl[0].TSP_Ctrl,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_TSFILE_EN));
    }
    //2009.07
    REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
            RESET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_FILE_CHECK_WP));
}

void
MHal_TSP_tsif_enable(U32 u32TSIF, B16 bEnable)
{
    static const U32 sTSIFBIT[] = { TSP_HW_CFG4_TSIF0_ENABLE, TSP_HW_CFG4_TSIF1_ENABLE };

    switch(u32TSIF)
    {
        case 0:
        case 1:
            if (bEnable)
            {
                REG32_W(&_TspCtrl[0].Hw_Config4,
                        SET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config4), sTSIFBIT[u32TSIF]));
            }
            else
            {
                REG32_W(&_TspCtrl[0].Hw_Config4,
                        RESET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config4), sTSIFBIT[u32TSIF]));
            }
            break;

        default:
            break;
    }
}

void MHal_TSP_SetCtrlMode(U32 u32EngId, U32 u32Mode, U32 u32TsIfId)
{
    // Control bits:
    // TSP_CTRL_CPU_EN
    // TSP_CTRL_SW_RST
    // TSP_CTRL_MEM_DMA_EN

    // for file in related setting
    REG32_W(&_TspCtrl[u32EngId].TSP_Ctrl,
            (REG32_R(&_TspCtrl[u32EngId].TSP_Ctrl) & ~(TSP_CTRL_CPU_EN   |
                                                       TSP_CTRL_SW_RST   |
                                                       TSP_CTRL_TSFILE_EN |
//[URANUS]                                                       TSP_CTRL_CLK_GATING_DISABLE |
// @FIXME: Richard ignore this at this stage
                                                       0                  )) | u32Mode);
    MHal_TSP_filein_enable(HAS_FLAG(u32Mode, TSP_CTRL_TSFILE_EN));
    if (2> u32TsIfId)
    {
        MHal_TSP_tsif_enable(u32TsIfId, true);
    }
}

void MHal_TSP_WbDmaEnable(B16 bEnable)
{
    if (bEnable)
    {
        REG32_W(&_TspCtrl[0].Hw_Config0,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config0), TSP_HW_CFG0_WB_DMA_RESET));
    }
    else
    {
        REG32_W(&_TspCtrl[0].Hw_Config0,
                SET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config0), TSP_HW_CFG0_WB_DMA_RESET));
    }
}

void MHal_TSP_NewDmaWriteArbiter(B16 bEnable)
{
    if (bEnable)
    {
        REG32_W(&_TspCtrl[0].reg15b4,
                SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), TSP_NEW_DMA_WARB));
    }
    else
    {
        REG32_W(&_TspCtrl[0].reg15b4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), TSP_NEW_DMA_WARB));
    }
}

void MHal_TSP_SelPad(U32 u32EngId, U32 PadId)
{
    if (PadId)
    {
        REG32_W(&_TspCtrl[u32EngId].TSP_Ctrl,
                SET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_MUX0_PAD1_SEL));
    }
    else
    {
        REG32_W(&_TspCtrl[u32EngId].TSP_Ctrl,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_MUX0_PAD1_SEL));
    }
}

void MHal_TSP_MUX1_SelPad(U32 u32EngId, U32 PadId)
{
    if (PadId)
    {
        REG32_W(&_TspCtrl[u32EngId].TSP_Ctrl,
                SET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_MUX1_PAD1_SEL));
    }
    else
    {
        REG32_W(&_TspCtrl[u32EngId].TSP_Ctrl,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_MUX1_PAD1_SEL));
    }
}

void MHal_TSP_SwReset(void)
{
    //U32     u32TspCtrl = REG32_R(&_TspCtrl[0].TSP_Ctrl);
    //REG32_W(&_TspCtrl[0].TSP_Ctrl, u32TspCtrl |  TSP_CTRL_SW_RST);
    //REG32_W(&_TspCtrl[0].TSP_Ctrl, u32TspCtrl & ~TSP_CTRL_SW_RST);
    REG32_W(&_TspCtrl[0].TSP_Ctrl, 0);
}

void MHal_TSP_CPU_SetBase(void)
{
    // TSP FW running in QMEM
    MHal_TSP_FW_load((U32)_tsp_dat, sizeof(_tsp_dat), TRUE, TRUE, TRUE);
}

void
MHal_TSP_FW_load(
    U32                     u32FwAddrVirt,
    U32                     u32FwSize,
    B16                     bFwDMA,
    B16                     bIQmem,
    B16                     bDQmem)
{
    U32                     u32FwAddrPhys;
    unsigned char* bin_mem = NULL;

    // bDQmem is always true
    ASSERT(bDQmem);

    bin_mem = ioremap(BIN_MEM_ADR, BIN_MEM_LEN);
    if (NULL == bin_mem)
    {
        BUG();
        return;
    }

    memset(bin_mem, 0, BIN_MEM_LEN);
    memcpy(bin_mem, _tsp_dat, sizeof(_tsp_dat));

    u32FwAddrPhys=          BIN_MEM_ADR;

    printk("TSP FW Addr=0x%x\n", u32FwAddrPhys);

    // @FIXME: Richard: Only allow TSP FW running in DRAM at this first stage.
    // improve this afterward.
    //TO DO:
    REG32_W(&_TspCtrl[0].Cpu_Base, u32FwAddrPhys>>(TSP_MIU_BASE_OFFSET-1)); // 8 bytes address unit
    //REG32_W(&_TspCtrl[0].Cpu_Base, u32FwAddrPhys>>TSP_MIU_BASE_OFFSET); // 16 bytes address unit

    printk("TSP FW DMA enable=%d\n", bFwDMA);
    if (bFwDMA){
        U32                 u32DnldCtrl= 0;
        u32DnldCtrl=        u32FwAddrPhys>>TSP_DMA_BASE_OFFSET;
        // to make sure that download address not overflow
        ASSERT(0== (0xFFF00000 & u32DnldCtrl));

        u32DnldCtrl &= 0xFFFF;
        u32DnldCtrl |= (_TSP_QMEM_SIZE<< 14);      // (size>>2)<< 16

        REG32_W(&_TspCtrl[0].Dnld_Ctrl, u32DnldCtrl);
        REG32_W(&_TspCtrl[0].reg160C,
                (REG32_R(&_TspCtrl[0].reg160C) & ~TSP_DMA_RADDR_19_22_MASK) |
                   (((u32FwAddrPhys>>(16+TSP_DMA_BASE_OFFSET)) << TSP_DMA_RADDR_19_22_SHFT) & TSP_DMA_RADDR_19_22_MASK));

        REG32_W(&_TspCtrl[0].TSP_Ctrl,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_DNLD_START| TSP_CTRL_DNLD_DONE));
        REG32_W(&_TspCtrl[0].TSP_Ctrl,
                SET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_DNLD_START));

        printk("WB status : WB Reset = %d, %d, New WARB = %d, ECO = %d\n",
                HAS_FLAG(REG32_R(&_TspCtrl[0].Hw_Config0), TSP_HW_CFG0_WB_DMA_RESET),
                HAS_FLAG(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_DMA_RST),
                HAS_FLAG(REG32_R(&_TspCtrl[0].reg15b4), TSP_NEW_DMA_WARB),
                HAS_FLAG(REG32_R(&_TspCtrl[0].Hw_Config4), TSP_HW_CFG4_WB_ECO));

        while (!HAS_FLAG(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_DNLD_DONE));
        REG32_W(&_TspCtrl[0].TSP_Ctrl,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_DNLD_START| TSP_CTRL_DNLD_DONE));
    }

    REG32_W(&_TspCtrl[0].Qmem_Imask,
            SET_FLAG1(REG32_R(&_TspCtrl[0].Qmem_Imask), _TSP_QMEM_I_MASK));
    if (bIQmem){
        REG32_W(&_TspCtrl[0].Qmem_Ibase,
                SET_FLAG1(REG32_R(&_TspCtrl[0].Qmem_Ibase), _TSP_QMEM_I_ADDR_HIT));
    }
    else{
        REG32_W(&_TspCtrl[0].Qmem_Ibase,
                SET_FLAG1(REG32_R(&_TspCtrl[0].Qmem_Ibase), _TSP_QMEM_I_ADDR_MISS));
        REG32_W(&_TspCtrl[0].TSP_Ctrl,
                SET_FLAG1(REG32_R(&_TspCtrl[0].TSP_Ctrl), TSP_CTRL_ICACHE_EN));
    }

    REG32_W(&_TspCtrl[0].Qmem_Dmask,
            SET_FLAG1(REG32_R(&_TspCtrl[0].Qmem_Dmask), _TSP_QMEM_D_MASK));

    REG32_W(&_TspCtrl[0].Qmem_Dbase,
            SET_FLAG1(REG32_R(&_TspCtrl[0].Qmem_Dbase), _TSP_QMEM_D_ADDR_HIT));
#if 0
    if (bDQmem){
        REG32_W(&_TspCtrl[0].Qmem_Dbase,
                SET_FLAG1(REG32_R(&_TspCtrl[0].Qmem_Dbase), _TSP_QMEM_D_ADDR_HIT));
    }
    else{
        REG32_W(&_TspCtrl[0].Qmem_Dbase,
                SET_FLAG1(REG32_R(&_TspCtrl[0].Qmem_Dbase), _TSP_QMEM_D_ADDR_MISS));

        REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
                SET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_DCACHE_EN));
    }
#endif

    // Initialize shared memory
    shm = (shared_mem_t*)&bin_mem[SHARED_MEM_START];
    shm->sdr_base_pa = BIN_MEM_ADR;

}

void
MHal_TSP_Scmb_Detect(B16 bEnable)
{
    if (bEnable)
    {
        REG32_W(&_TspCtrl[0].reg15b4,
                SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), TSP_SCMB_FILE| TSP_SCMB_TSIN));
    }
    else
    {
        REG32_W(&_TspCtrl[0].reg15b4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), TSP_SCMB_FILE| TSP_SCMB_TSIN));
    }
}

void
MHal_TSP_HwPatch(void)
{
    // To prevent the race condition of accessing section filter registers from HW/CPU.
    // It's a HW bug.
    REG32_W(&_TspCtrl[0].Hw_Config4,
            SET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config4), TSP_HW_CFG4_BYTE_ADDR_DMA));

    REG32_W(&_TspCtrl[0].Hw_Config4,
            SET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config4), TSP_HW_CFG4_WB_ECO));

    //REG32_W(&_TspCtrl[0].reg15b4,
    //        SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), TSP_DUPLICATE_PKT_SKIP));

    // Bad initial value of TSP_CTRL1
    // Suppose Standby mode for TSP should NOT be enabled.
    // Enabling TSP standby mode cause TSP section registers (SRAM in AEON) malfunction.
    // Disable it by SW at this stage.
    REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
            RESET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_STANDBY));

    //2009.11.20
    REG32_W(&_TspCtrl[0].reg160C,
            SET_FLAG1(REG32_R(&_TspCtrl[0].reg160C),TSP_RM_DMA_Glitch));
}

// return 0xFFFFFFFF when firmware has not been loaded yet
U32
MHal_TSP_GetSDRBase(void)
{
    return shm->sdr_base_pa;
}

void
MHal_TSP_SendFWCmd(U8 id, U8 param1, U8 param2, U8 param3)
{
    U32 u32TimeOut = TSP_CMD_TIMEOUT_MS;

    shm->cmd = (id | (param1<<8) | (param2<<16)| (param3<<24));

    // wait for command to be processed
    while(--u32TimeOut && (shm->cmd != TSP_CMD_CLEAR)) {
        mdelay(1);
    }
}

void
MHal_TSP_RemoveErrPacket_Enable(HalTSP_Output_Path path)
{
    U32 u32Value = 0;

    switch(path)
    {
        case E_HALTSP_OUT_PATH_VIDEO:
            u32Value = TSP_HW_CFG4_VPES_ERR_RM_EN;
            break;

        case E_HALTSP_OUT_PATH_AUDIO:
            u32Value = TSP_HW_CFG4_APES_ERR_RM_EN;
            break;

        case E_HALTSP_OUT_PATH_SEC:
            u32Value = TSP_HW_CFG4_APES_ERR_RM_EN;
            break;

        default:
            // unknown type
            return;
    }

    REG32_W(&_TspCtrl[0].Hw_Config4, REG32_R(&_TspCtrl[0].Hw_Config4)|u32Value);

}

void
MHal_TSP_RemoveErrPacket_Disable(HalTSP_Output_Path path)
{
    U32 u32Value = 0;

    switch(path)
    {
        case E_HALTSP_OUT_PATH_VIDEO:
            u32Value = TSP_HW_CFG4_VPES_ERR_RM_EN;
            break;

        case E_HALTSP_OUT_PATH_AUDIO:
            u32Value = TSP_HW_CFG4_APES_ERR_RM_EN;
            break;

        case E_HALTSP_OUT_PATH_SEC:
            u32Value = TSP_HW_CFG4_APES_ERR_RM_EN;
            break;

        default:
            // unknown type
            return;
    }

    REG32_W(&_TspCtrl[0].Hw_Config4, REG32_R(&_TspCtrl[0].Hw_Config4)& ~u32Value);

}

void MHal_TSP_Flush_AV_FIFO(U32 u32StreamId, B16 bFlush)
{
    U32 u32Flag = (0 == u32StreamId)? TSP_RESET_VFIFO : TSP_RESET_AFIFO;
    if (bFlush)
    {
        REG32_W(&_TspCtrl[0].reg160C,
                SET_FLAG1(REG32_R(&_TspCtrl[0].reg160C), u32Flag));
    }
    else
    {
        REG32_W(&_TspCtrl[0].reg160C,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].reg160C), u32Flag));
    }
}

U8 MHal_TSP_Fetch_AV_FIFO(U32 u32StreamId)
{
    U8 u8Byte;

    if (u32StreamId)
    {
        REG32_W(&_TspCtrl[0].reg15b4,
                SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), BIT9));
        REG32_W(&_TspCtrl[0].reg15b4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), BIT10));
    }
    else
    {
        REG32_W(&_TspCtrl[0].reg15b4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), BIT9));
        REG32_W(&_TspCtrl[0].reg15b4,
                RESET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), BIT10));
    }

    REG32_W(&_TspCtrl[0].reg15b4,
            SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), BIT7));
    REG32_W(&_TspCtrl[0].reg15b4,
            SET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), BIT8));
    REG32_W(&_TspCtrl[0].reg15b4,
            RESET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), BIT8));

    u8Byte = REG32_R(&_TspCtrl[0].Pkt_Cnt) & 0xFF;

    REG32_W(&_TspCtrl[0].reg15b4,
            RESET_FLAG1(REG32_R(&_TspCtrl[0].reg15b4), BIT7));

    return u8Byte;
}

B16 MHal_TSP_Alive(void)
{
    U32 i = 0;
    //U32 u32Data;

    REG32_W(&_TspCtrl[0].MCU_Data1, 0xAB);
    //REG32_W(&_TspCtrl[0].MCU_Cmd, TSP_MCU_CMD_ALIVE);
    while (i< 10)
    {
        if (0xCD == REG32_R(&_TspCtrl[0].MCU_Data1))
        {
            //u32Data = REG32_R(&_TspCtrl[0].MCU_Data0);
            REG32_W(&_TspCtrl[0].MCU_Data1, i);
            //return (TSP_MCU_DATA_ALIVE == u32Data)? TRUE: FALSE;
            return TRUE;
        }
        i++;
        mdelay(1);
    }
    //printk("%s: no response!\n", __FUNCTION__);
    REG32_W(&_TspCtrl[0].MCU_Data1, 0);
    return FALSE;
}

void MHal_Reset_WB(void)
{
    REG32_W(&_TspCtrl[0].Hw_Config0,
        SET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config0), TSP_HW_CFG0_WB_DMA_RESET));
    REG32_W(&_TspCtrl[0].Hw_Config0,
        RESET_FLAG1(REG32_R(&_TspCtrl[0].Hw_Config0), TSP_HW_CFG0_WB_DMA_RESET));
    REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
        SET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_DMA_RST));
    REG32_W(&_TspCtrl[0].HwInt_Stat_Ctrl1,
        RESET_FLAG1(REG32_R(&_TspCtrl[0].HwInt_Stat_Ctrl1), TSP_CTRL1_DMA_RST));
}

#ifdef TSP_DUMP
void _TDump_SecFlt(U32 u32EngId, U32 u32SecFltId)
{
    REG_SecFlt*     pSecFilter = &(_TspSec[u32EngId].Flt[u32SecFltId]);

    printf("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x",
        REG32_IndR(&pSecFilter->Ctrl),
        REG32_IndR(&pSecFilter->BufStart),
        REG32_IndR(&pSecFilter->BufEnd),
        REG32_IndR(&pSecFilter->BufWrite),
        REG32_IndR(&pSecFilter->BufCur),
        REG32_IndR(&pSecFilter->Match[0]));
}

void _TDump_Flt(U32 u32EngId, U32 u32IdxStart, U32 u32IdxEnd)
{
    int i;
    printf("==== %s start ====\n", __FUNCTION__);
    for (i= u32IdxStart; i< u32IdxEnd; i++){
        printf("%d: ", i);
        _TDump_PidFlt(u32EngId, i);
        printf("    ");
        _TDump_SecFlt(u32EngId, i);
        printf("\n");
    }
    printf("==== %s end   ====\n", __FUNCTION__);
}

#endif // #if TSP_DUMP
