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
//
//  File name: fwTSP.c
//  Description: Transport Stream Processer firmware
//
////////////////////////////////////////////////////////////////////////////////

#ifdef TSP_MEM_IC
    #include "board_ic.h"
#elif defined TSP_MEM_QMEM
    #include "board_qmem.h"
#else
    #error "Choose a TSP memory layout"
#endif

#if defined(_CMODEL)

#include <windows.h>
#include <process.h>
#define FWTSP_CMODEL_DELAY()        Sleep(50);

#else

typedef unsigned long               BOOL;
#define FALSE                       0
#define TRUE                        1

#endif // _CMODEL

typedef unsigned long               U32; // 4 bytes
typedef unsigned short              U16; // 2 bytes
typedef unsigned char               U8;
typedef signed long                 S32;

#define TS_PACKET_SIZE              188
#define TTX_PACKET_SIZE             46

#define TSP_CPU_REGMAP
#include "reg_tsp.h"
#undef TSP_CPU_REGMAP

#include "uart.h"


//--------------------------------------------------------------------------------------------------
//  Compiler Option
//--------------------------------------------------------------------------------------------------
#define REQ_NOTIFY_SUPPORT          0

#define SEC_128_ALIGN_SUPPORT       0

//[Note] Jerry
// If we want to support REQ in section packet handler
// We have to set MEET to force write address to be updated
#define SEC_REQ_NOTIFY_SUPPORT      0
//[Note]
// Can only protect the wrong packet has wrong continuity counter.
//#define SEC_CC_CHECK_SUPPORT        1                                   //[OBSOLETE] always do

#define SEC_CRC_CHECK_SUPPORT       1

#define SEC_HDR_BREAK_SUPPORT       1                                   // Support section header cross packets
#define SEC_PTN_BREAK_SUPPORT       1                                   // Support match pattern cross packets

//[Note]
// PES can not roll back to correct write address if discontinuity
#define PES_CC_CHECK_SUPPORT        0

#define TTX_PACKET_SUPPORT          0

#define PKT_MATCH_SUPPORT           1

#define PCR_STC_SYNC_ENABLE         0
#define PCR_STC_SYNC2_ENABLE        1
#if ((PCR_STC_SYNC_ENABLE == 1) && (PCR_STC_SYNC2_ENABLE == 1))
#error "PCR_STC_SYNC_ENABLE and PCR_STC_SYNC2_ENABLE cannot be enabled at the same time"
#endif

#define OVERFLOW_BLOCK_ENABLE       1
#define OVERRUN_CHECK_ENABLE        1

#define VPES_ENABLE                 0

//[NOTE] Jerry
// If firmware wants to support one-shot section filter, the upper
// layer (eg. driver) should not check PIDFLT enable regsiter, becuase
// it will disable by firmware. Upper layer also need to know it will
// be disable when ISR happens and update it's software local state.
// Now we keep doing section DMA and disable filter by upper layer.

#define SEC_ONESHOT_SUPPORT         1

#define FWTSP_INFO_ENABLE           0
#define FWTSP_DEBUG_ENABLE          0
#define DBG_SWITCH_DIFF_SYNTH       0
#define DBG_SWITCH                  0

#if PCR_STC_SYNC_ENABLE
#define Diff(x1, x2) ((x1>x2)?(x1-x2):(x2-x1))
#define STD_TICK                    90000   //90K clock
#define RESET_THD                   5000    //Rest STC after the difference between PCR & STC is bigger than RESET_THD.
//#define PCR_INTERVAL                8000    //Interval between the different PCR.
#define PCR_INTERVAL                4000    //Interval between the different PCR.
#define MAX_ADJUST_TICK             500     //Frequency up bound: STD_TICK + (PCR_INTERVAL*STD_TICK)/PCR_INTERVAL => 95625 => 28.687 MHz
                                            //Frequency low bound: STD_TICK - (PCR_INTERVAL*STD_TICK)/PCR_INTERVAL => 84375 => 25.3 MHz
#define SYNC_START                  2       //After the 5th PCR, start sync process
#define SYNC_THD                    2       //Sync threshold, Tick difference between two PCR.
#define USE_STC1_FOR_COMPARE        0       //Use STC1 for the compare process.
#define SYNC_DEBUG                  0
#endif

#define WAIT_INT_CLEAR_TIMEOUT      10000    // for one PID set to more than one section filter, wait for host to clear interrupt
                                            // before issuing the second interrupt

#define SEC_SIZE_MAX                4096

#define VPES_SIZE_MAX               0xFFFFFFF // 2009.09.16 - Support Video PES filtering

#define DEFER_FIRE_INT              1

//--------------------------------------------------------------------------------------------------
//  Some useful macros
//--------------------------------------------------------------------------------------------------
#define REG_W(reg, value)          ((reg)= (value))
#define REG_R(reg)                 (reg)


//--------------------------------------------------------------------------------------------------
//  Firmware and CModel Interface
//--------------------------------------------------------------------------------------------------
#if defined(_CMODEL)
U32                                 _rFwTspCtrl_Reg = 0;
U32                                 _rFwTspSec_Reg = 0;
U32                                 _uFwTsp_Run = 0;
extern void                         HwTSP_Interrupt(void);
#endif


//--------------------------------------------------------------------------------------------------
//  Firmware Debug Interface
//--------------------------------------------------------------------------------------------------
#if FWTSP_DEBUG_ENABLE

#define DEBUG_FLAG_NULL             0x00
#define DEBUG_FLAG_INIT             0x01
#define DEBUG_FLAG_PROC             0x02
#define DEBUG_FLAG_SEC              0x03
#define DEBUG_FLAG_PES              0x04
#define DEBUG_FLAG_PKT              0x05
#define DEBUG_FLAG_AF               0x06
#define DEBUG_FLAG_TTX              0x07
#define DEBUG_FLAG_ERROR            0xE0

#if defined(_CMODEL)

//--------------------------------------------------------------------------------------------------
// CModel debug environment
//--------------------------------------------------------------------------------------------------
static volatile U32                 dbg_fw;
static volatile U16                 dbg_flag;
static volatile U32                 dbg_data;
#define DBG_FWTSP_FLAG(_major, _minor)                                  \
    _Dbg_SetFlag((_major<<8)|(_minor));
#define DBG_FWTSP_MSG(_msg)                                             \
    OutputDebugString(_msg);

#else // no _CMODEL

//--------------------------------------------------------------------------------------------------
// Normal debug environment
//--------------------------------------------------------------------------------------------------
extern volatile U32                 dbg_fw;
extern volatile U32                 dbg_flag;
extern volatile U32                 dbg_data;
//static volatile U32                 dbg_flag_addr = (U32)&dbg_flag;
#define DBG_FWTSP_FLAG(_major, _minor)                                  \
    _Dbg_SetFlag((_major<<8)|(_minor));                                 \
//    asm("l.nop %0" : : "i" ((_major<<8)|(_minor)));
#if UART_ENABLE
#define DBG_FWTSP_MSG(_msg)                                             \
    uart_print_str(_msg);
#else
#define DBG_FWTSP_MSG(_msg)  ;
#endif

#endif // _CMODEL

#else // no FWTSP_DEBUG_ENABLE

//--------------------------------------------------------------------------------------------------
// Disable firmware debug
//--------------------------------------------------------------------------------------------------
extern volatile U32                 dbg_fw;
extern volatile U32                 dbg_flag;
extern volatile U32                 dbg_data;

// shared mem is initialized by linker script
extern volatile U32                 shm_sdr_base;
extern volatile U32                 shm_cmd;

// transport_scrambling_control detection
extern volatile U32                 shm_tsc_ready;
extern volatile U32                 shm_tsc_non_zero;

// each filter uses 1-bit
extern volatile U32                 shm_sec_overflow;

// section info
typedef struct {
    U16 w_count;
    U16 r_count;
    U32 r_ptr;
    U32 buf_size;
    U32 buf_end;
} sec_info_t;
extern volatile sec_info_t          shm_sec_info[TSP_ENGINE_NUM][TSP_SECFLT_NUM];

#define DBG_FWTSP_FLAG(_major, _minor)  ;
#define DBG_FWTSP_MSG(_msg)             ;

#endif // FWTSP_DEBUG_ENABLE

// each filter uses 1-bit
static U32  _not_ver_enable;

// pcr offset
static S32 _pcr_ofs = 0;

// each version number uses 8-bit
// version[0] = [0][1][2][3]
// version[1] = [4][5][6][7]
// ...
// version[7] = [28][29][30][31]
static U8   _not_ver_version[TSP_SECFLT_NUM];

#define NotVersionEnabled(fid)      ((_not_ver_enable >> (fid)) & 1)
#define NotVersionNum(fid)          _not_ver_version[fid]

#define SEC_VER_MASK                0x1F

// DRAM address conversion
#define MEM32(abs_adr) (*((volatile U32*)(((abs_adr)&~3) - shm_sdr_base)))

//------------------------------------------------------------------------------
//  Firmware Global Variable & Definition
//------------------------------------------------------------------------------

#define PES_OPT_NONE                0x00000000
#define PES_OPT_TTX                 0x00000001

#define DEF_STC_SYNTH               0x14000000                          // 216/2.5  = 86.4, 2.5(dec) = 00010.1(bin) = 0x14000000(u5.27)
#define MAX_STC_SYNTH               0x15000000                          // 216/2.75 = 78.55
#define MIN_STC_SYNTH               0x13000000                          // 216/2.25 = 96

#if PCR_STC_SYNC2_ENABLE

#define STC_SYNTH_DEF               0x0fe8d800
#define STC_DIFF_CHECK              0x4000
#define STC_DIFF_LO                 0x2
#define STC_DIFF_HI                 0x1000
#define STC_ADJ_A                   8
#define STC_ADJ_B                   64
#define STC_RESET_TH                2

static U8 u8ResetPCR = (STC_RESET_TH + 1);
static U32 u32PrevSTCBase;
static U32 u32PrevSynth;

#endif

// TS packet
static U32                          _u32TS_PktSyncFlag;
static U32                          _u32TS_PktEIFlag;                   // TSH: error indicator
static U32                          _u32TS_PktPUSIFlag;                 // TSH: payload unit start indicator
static U32                          _u32TS_PktPIDFlag;
static U32                          _u32TS_PktAFFlag;                   // TSH: adaptation field
static U32                          _u32TS_PktCCFlag;                   // TSH: cont. count

// TSP unit
static U32                          _u32EngId;

static U32                          _u32PktContiCnt[TSP_ENGINE_NUM][TSP_SECFLT_NUM];
static U32                          _u32FltDataCnt[TSP_ENGINE_NUM][TSP_SECFLT_NUM];

static U32                          _u32BufPtr[TSP_ENGINE_NUM][TSP_SECFLT_NUM];

// 2009.09.16 - Support Video PES filtering
//static U16                          _u16DataSize[TSP_ENGINE_NUM][TSP_SECFLT_NUM];
static U32                          _u32DataSize[TSP_ENGINE_NUM][TSP_SECFLT_NUM];

static U8                           _u8mapsec2pid[TSP_ENGINE_NUM][TSP_SECFLT_NUM];

#if SYNC_LOCK_SUPPORT
/*
static U32                          _u32TSP_SyncWait[TSP_ENGINE_NUM];
static BOOL                         _bTSP_SyncLock[TSP_ENGINE_NUM];
*/
#endif // SYNC_LOCK_SUPPORT


#if defined(_CMODEL)
static REG_Ctrl                    *_rCtrl;
static REG_Sec                     *_rSec;
#else
#if 0
static REG_Ctrl                    *_rCtrl = ((REG_Ctrl*)REG_CTRL_BASE); // 2 TSP array
static REG_Sec                     *_rSec = ((REG_Sec*)REG_SECFLT_BASE);
#else // optimization
#define _rCtrl                      ((REG_Ctrl*)REG_CTRL_BASE)
#define _rPid                       ((REG_Pid*)REG_PIDFLT_BASE)
#define _rSec                       ((REG_Sec*)REG_SECFLT_BASE)
#endif // 0
#endif


static REG_Ctrl                    *_prCtrl;
static REG_SecFlt                  *_prSecFlt;
static REG_SecFlt                  *_prSecBuf;
static U32                          _u32SfId;

static BOOL                         _bTail;
static U32                          _u32DmaSize;
static U32                          _u32DmaCtrl;

#if SEC_PTN_BREAK_SUPPORT
static U32                          _u32PtnByte[TSP_ENGINE_NUM][TSP_SECFLT_NUM];
static U32                          _u32PtnMask[(TSP_FILTER_DEPTH<< 1)/sizeof(U32)];
static U32                          _u32PtnMaskShft[TSP_FILTER_DEPTH/sizeof(U32)];
static U32                          _u32Ptn[(TSP_FILTER_DEPTH<< 1)/sizeof(U32)];
static U32                          _u32PtnShft[TSP_FILTER_DEPTH/sizeof(U32)];
#endif

#if SEC_HDR_BREAK_SUPPORT
static U32                          _u32SecByte[TSP_ENGINE_NUM][TSP_SECFLT_NUM];
static U32                          _u32SecLenData[TSP_ENGINE_NUM][TSP_SECFLT_NUM];
#endif

#if DEFER_FIRE_INT
static U8                           _u8DeferIntRead[TSP_ENGINE_NUM];
static U8                           _u8DeferIntWrite[TSP_ENGINE_NUM];
//static U8                           _u8DeferIntHK[TSP_ENGINE_NUM][TSP_ENGINE_NUM];
static U32                          _u32DeferIntStatus[TSP_ENGINE_NUM];
static U32                          _u32DeferInt[TSP_ENGINE_NUM][TSP_SECFLT_NUM];
#endif // #if DEFER_FIRE_INT

#if defined(_CMODEL)
static U8                           _FwDate[] = __DATE__ ;
#else
//static U8                           _FwDate[] __attribute__((section(".fwinfo"))) = __DATE__ ;
#endif

#if FWTSP_INFO_ENABLE
static U32                          _u32RdyCnt;
#endif

#if PCR_STC_SYNC_ENABLE
static U32                          _u32NewPCR[TSP_ENGINE_NUM];
static BOOL                         _bNewPCR[TSP_ENGINE_NUM];
static U32                          _u32PCRCount[TSP_ENGINE_NUM];
static U32                          _u32PCR[TSP_ENGINE_NUM][2];
static U32                          _u32PCRId[TSP_ENGINE_NUM];
static BOOL                         _bCheckSynthesizer[TSP_ENGINE_NUM];
static U32                          _u32PCR_STC_Diff[TSP_ENGINE_NUM][2];
static BOOL                         _bPositiveDiff[TSP_ENGINE_NUM][2];
static U32                          _u32Synthesizer[TSP_ENGINE_NUM];
static U32                          _u32PreTick[TSP_ENGINE_NUM];
#endif // #if PCR_STC_SYNC_ENABLE

static U32                         u32DbgCnt = 0;
static BOOL                        bDbgHang = FALSE;
static U32                         u32DbgDummy = 0;

// Simulate Write Trigger Register
#if defined(_CMODEL)
__inline void _CM_Reg_Write(U32 reg_addr)
{
    extern REG32        _hwTSP_DMA_IdxUpd[TSP_ENGINE_NUM];

    reg_addr = reg_addr - (U32)_rCtrl;
    switch (reg_addr)
    {
    case (U32)&(((REG_Ctrl*)0)->Pkt_CacheIdx):
        _hwTSP_DMA_IdxUpd[0] = 1;
        break;
    case ((U32)&(((REG_Ctrl*)0)->Pkt_CacheIdx) + sizeof(REG_Ctrl)):
        _hwTSP_DMA_IdxUpd[1] = 1;
        break;
    }
}
#else
#define _CM_Reg_Write(reg_addr)
#endif

#if FWTSP_DEBUG_ENABLE
void _Dbg_SetFlag(U32 flag)
{
    #if defined(_CMODEL)
    dbg_flag = flag;
    #else
    if (!(dbg_flag & (DEBUG_FLAG_ERROR<<8)))
    {
        dbg_flag = flag; // it has fewer code than using a point variable
    //*(volatile U32*)dbg_flag_addr = flag;
    }
    #endif
}
#endif

void _WaitInterruptClear()
{
    volatile U32 u32WaitCount = WAIT_INT_CLEAR_TIMEOUT;

    // wait for interrupt clear
    while(_rCtrl[0].SwInt_Stat & TSP_SWINT_CTRL_FIRE &&
         u32WaitCount)
    {
         --u32WaitCount;
    }
 }

int _FireInterrupt(U32 status, U32 info)
{
#if (DEFER_FIRE_INT==0)
    _WaitInterruptClear();
#endif

    if (_rCtrl[0].SwInt_Stat & TSP_SWINT_CTRL_FIRE)
    {
        DBG_FWTSP_MSG("SwInt not clear\r\n");
#if DEFER_FIRE_INT
        if (TSP_SWINT_STATUS_PKT_OVRUN == status)
        {
            return 0;
        }

        U32 u32TspId = (info & TSP_SWINT_INFO_ENG_MASK) >> TSP_SWINT_INFO_ENG_SHFT;
        U32 u32SfId = (info & TSP_SWINT_INFO_SEC_MASK) >> TSP_SWINT_INFO_SEC_SHFT;

        if (_u32DeferIntStatus[u32TspId] & (1<< u32SfId))
        {
            // @FIXME. Should it be put into queue?
            return 0;
        }
        _u32DeferIntStatus[u32TspId] |= (1<< u32SfId);
        _u32DeferInt[u32TspId][_u8DeferIntWrite[u32TspId]] = (status << TSP_SWINT_STATUS_CMD_SHFT) | info;

        //_u8DeferIntHK[u32TspId][_u8DeferIntWrite[u32TspId]] = (U8)u32HkId;
        _u8DeferIntWrite[u32TspId]++;
        _u8DeferIntWrite[u32TspId] &= 0x1F; // 32 filter
#endif // #if DEFER_FIRE_INT        
    }
    else
    {
        _rCtrl[0].SwInt_Stat = TSP_SWINT_CTRL_FIRE | (status << TSP_SWINT_STATUS_CMD_SHFT) | info;
        //++shm_sec_info[_u32EngId][_u32SfId].w_count;
    }

    #if TSP_DBG_SAFE_MODE_ENABLE
    while (_rCtrl[0].SwInt_Stat)
    {
        #if defined(_CMODEL)
        FWTSP_CMODEL_DELAY(); // Thread delay for CModel performance
        #endif
    }
    #endif
    return 1;
}

void _UpdateWritePtr()
{
    //_WaitInterruptClear();
    _prSecBuf->BufWrite = _prSecBuf->BufCur;
}


void RevertFlt(void)
{
    _prSecFlt->RmnReqCnt = _prSecFlt->RmnReqCnt & ~TSP_SECFLT_RMNCNT_MASK;
    _prSecBuf->BufCur = _prSecBuf->BufWrite = _u32BufPtr[_u32EngId][_u32SfId] + 4;

    MEM32(_u32BufPtr[_u32EngId][_u32SfId]) = 0x0;
    _u32DataSize[_u32EngId][_u32SfId] = 0;
}

void FinishFlt(void)
{
    U32 u32Size = MEM32(_u32BufPtr[_u32EngId][_u32SfId]);
    U32 u32Tmp;

    if (u32Size > 0)
    {
#if VPES_ENABLE    
        if (u32Size != _u32DataSize[_u32EngId][_u32SfId])
        {
            //------------------------------------------------//
            // 2009.09.16 - Support Video PES filtering
            if(_u32DataSize[_u32EngId][_u32SfId] != VPES_SIZE_MAX )
            //------------------------------------------------//
            {
                RevertFlt();
                return;
            }
        }
#endif
        u32Tmp  = _u32BufPtr[_u32EngId][_u32SfId] + 4 + u32Size;
        if (u32Tmp >= _prSecBuf->BufEnd)
        {
            u32Tmp -= (_prSecBuf->BufEnd - _prSecBuf->BufStart);
        }
        _prSecBuf->BufWrite = u32Tmp;

        if ((_prSecBuf->BufWrite < _prSecBuf->BufRead) &&
           ((_prSecBuf->BufWrite + 8) > _prSecBuf->BufRead))
        {
            RevertFlt();
            return;
        }

        _u32BufPtr[_u32EngId][_u32SfId] = (_prSecBuf->BufWrite + 3) & ~3;

        if (_u32BufPtr[_u32EngId][_u32SfId] >= _prSecBuf->BufEnd)
        {
            _u32BufPtr[_u32EngId][_u32SfId] = _prSecBuf->BufStart;
        }

        if ((_u32BufPtr[_u32EngId][_u32SfId] + 4) >= _prSecBuf->BufEnd)
        {

            _prSecBuf->BufCur = _prSecBuf->BufStart;
        }
        else
        {
            _prSecBuf->BufCur = _u32BufPtr[_u32EngId][_u32SfId] + 4;
        }

        MEM32(_u32BufPtr[_u32EngId][_u32SfId]) = 0;

        _u32DataSize[_u32EngId][_u32SfId] = 0;

        ++shm_sec_info[_u32EngId][_u32SfId].w_count;

        _FireInterrupt(TSP_SWINT_STATUS_SEC_RDY, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
    }
    else
    {
        RevertFlt();
    }

}

static void _DMA_WaitCopyReady(void)
{
    while (!(_prCtrl->Pkt_Stat & TSP_PKT_STAT_DMA_RDY))
    {
        //check alive
        if (_prCtrl->MCU_Data1 == 0xAC)
        {
            _prCtrl->MCU_Data1 = 0xCF;
            bDbgHang = TRUE;
        }
        if (_prCtrl->MCU_Data1 == 0xAD)
        {
            _prCtrl->MCU_Data1 = 0xCE;
            bDbgHang = FALSE;
        }        
        if (bDbgHang == TRUE)
        {
            *(volatile  U32*)0x4010 = ++u32DbgCnt;
            u32DbgDummy = *(volatile U32*)0x4000;
            *(volatile  U32*)0x4014 = u32DbgDummy;
        }
    }

    #if !defined(_CMODEL)
    // for hardware update section filter SRAM register latency
/*
    asm("l.nop %0" : : "i" (0));
    asm("l.nop %0" : : "i" (0));
    asm("l.nop %0" : : "i" (0));
    asm("l.nop %0" : : "i" (0));
    asm("l.nop %0" : : "i" (0));
    asm("l.nop %0" : : "i" (0));
    asm("l.nop %0" : : "i" (0));
    asm("l.nop %0" : : "i" (0));
*/
    #endif

    _prCtrl->Pkt_Stat &= ~TSP_PKT_STAT_DMA_RDY;                            // We are responsible to clear the DMA status after.
}


static void _DMA_WaitCacheReady(void)
{
    while (!(_prCtrl->Pkt_Stat & TSP_PKT_STAT_CACHE_RDY))
    {
    }

    _prCtrl->Pkt_Stat &= ~TSP_PKT_STAT_CACHE_RDY;                      // We are responsible to clear the DMA status after.
}

#define _DMA_RECACHE(_n)            _prCtrl->Pkt_CacheIdx = (_n);                                   \
                                    _CM_Reg_Write((U32)&_prCtrl->Pkt_CacheIdx);                     \
                                    _DMA_WaitCacheReady()

static BOOL _Dma_PreCheck(void)
{
    //---------------------------------------------------------------------------------------------
    // Set DMA size
    _prCtrl->Pkt_DmaSize = _u32DmaSize;
    //---------------------------------------------------------------------------------------------

    return TRUE;
}


static BOOL _Dma_PostCheck(void)
{
    //---------------------------------------------------------------------------------------------
    // Fire DMA
    _prCtrl->Pkt_DmaCtrl = _u32DmaCtrl;
    _DMA_WaitCopyReady();
    //---------------------------------------------------------------------------------------------

    if (_prCtrl->Pkt_Stat & TSP_PKT_STAT_DMA_OVFLOW)
    {
        _prCtrl->Pkt_Stat &= ~TSP_PKT_STAT_DMA_OVFLOW;
        _prSecFlt->Ctrl = _prSecFlt->Ctrl | (TSP_SECFLT_STATE_OVERFLOW<<TSP_SECFLT_STATE_SHFT);
        _prSecFlt->RmnReqCnt = _prSecFlt->RmnReqCnt & ~TSP_SECFLT_RMNCNT_MASK; // clear remain

        shm_sec_overflow |= (1<<_u32SfId);

        _FireInterrupt(TSP_SWINT_STATUS_BUF_OVFLOW, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));

        return FALSE; // exit
    }
    
    if ((_prCtrl->Pkt_Stat & TSP_PKT_STAT_OVRUN) || 
        (_prCtrl->Pkt_Stat & TSP_PKT_STAT_OVRUN2))
    {
        _prCtrl->MCU_Data1 = 0xE3;
        return FALSE; // exit
    }     

    return TRUE;
}


static BOOL _Proc_PreCheck(void)
{
    #if SEC_ONESHOT_SUPPORT
    if (_prSecFlt->Ctrl & (TSP_SECFLT_STATE_DISABLE<<TSP_SECFLT_STATE_SHFT))
    {
        return FALSE;
    }
    else
    #endif // SEC_ONESHOT_SUPPORT

    #if OVERFLOW_BLOCK_ENABLE
    if (_prSecFlt->Ctrl & (TSP_SECFLT_STATE_OVERFLOW<<TSP_SECFLT_STATE_SHFT))
    {
        // _FireInterrupt(TSP_SWINT_STATUS_BUF_OVFLOW, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
        return FALSE;
    }
    else
    #endif // OVERFLOW_BLOCK_ENABLE
    {
        return TRUE;
    }
}

static BOOL _PatternMatchHead(U32 data_start)
{
#if SEC_PTN_BREAK_SUPPORT
    _u32PtnByte[_u32EngId][_u32SfId] = 0;
#endif

    if (data_start < (TS_PACKET_SIZE - 15))
    {
        _DMA_RECACHE(data_start);
    }
#if SEC_PTN_BREAK_SUPPORT
    else
    {
        int     i, byte;

        byte = TS_PACKET_SIZE - data_start;

        // Mask not compare (byte/sizeof(U32)) ~ (TSP_FILTER_DEPTH/sizeof(U32)-1)
        _u32PtnMask[byte/sizeof(U32)] = REG_R(_prSecFlt->Mask[byte/sizeof(U32)]);
        REG_W(_prSecFlt->Mask[byte/sizeof(U32)],
              _u32PtnMask[byte/sizeof(U32)] | (0xFFFFFFFF<<((byte%sizeof(U32))<<3)));
        for (i = (byte/sizeof(U32))+1; i < TSP_FILTER_DEPTH/sizeof(U32); i++)
        {
            _u32PtnMask[i] = REG_R(_prSecFlt->Mask[i]);
            REG_W(_prSecFlt->Mask[i], 0xFFFFFFFF);
        }
        // Compare
        _DMA_RECACHE(data_start);

        // Restore changed mask (byte/sizeof[U32]) ~ (TSP_FILTER_DEPTH/sizeof(U32)-1)
        for (i = (byte/sizeof(U32)); i < TSP_FILTER_DEPTH/sizeof(U32); i++)
        {
            REG_W(_prSecFlt->Mask[i], _u32PtnMask[i]);
        }
        // Mark the number of last compared bytes.
        if (_prCtrl->Pkt_Stat & TSP_PKT_STAT_CMP_MATCH)
        {
            _u32PtnByte[_u32EngId][_u32SfId] = (U32)byte;
        }
    }

    return _prCtrl->Pkt_Stat & TSP_PKT_STAT_CMP_MATCH;

#else
    else
    {
        return TRUE;
    }
#endif

}

static BOOL _PatternMatchTail(U32 data_start)
{
#if SEC_PTN_BREAK_SUPPORT
    U32     u32ShftRight, u32ShftLeft;
    U32     u32ShftBase;

    int         i, byte = (int)_u32PtnByte[_u32EngId][_u32SfId];

    if (0 == byte)
    {
        return TRUE;
    }

    for (i = 0; i < TSP_FILTER_DEPTH>> 2; i++){
        _u32PtnMask[i]=     REG_R(_prSecFlt->Mask[i]);
        _u32Ptn[i]=         REG_R(_prSecFlt->Match[i]);
    }

    u32ShftRight=               (0x03 & byte)<< 3;
    u32ShftLeft=                (sizeof(U32)<<3)- u32ShftRight;
    u32ShftBase=                byte/sizeof(U32);
    for (i= 0; i< TSP_FILTER_DEPTH>> 2; i++){
        _u32PtnMaskShft[i]=     (_u32PtnMask[u32ShftBase+ i]>> u32ShftRight) |
                                (_u32PtnMask[u32ShftBase+ i+ 1]<< u32ShftLeft);
        _u32PtnShft[i]=         (_u32Ptn[u32ShftBase+ i]>> u32ShftRight) |
                                (_u32Ptn[u32ShftBase+ i+ 1]<< u32ShftLeft);
    }

    for (i = 0; i < TSP_FILTER_DEPTH>> 2; i++){
        REG_W(_prSecFlt->Mask[i], ((U32*)_u32PtnMaskShft)[i]);
        REG_W(_prSecFlt->Match[i], ((U32*)_u32PtnShft)[i]);
    }

    // Compare
    _DMA_RECACHE(data_start);

    // Restore changed mask 0 ~ (byte/sizeof(U32))
    for (i = 0; i < TSP_FILTER_DEPTH>> 2; i++){
        REG_W(_prSecFlt->Mask[i], _u32PtnMask[i]);
        REG_W(_prSecFlt->Match[i], _u32Ptn[i]);
    }
    _u32PtnByte[_u32EngId][_u32SfId] = 0;

    return _prCtrl->Pkt_Stat & TSP_PKT_STAT_CMP_MATCH;

#else
    return TRUE;
#endif
}

static BOOL _GetSecFollowPackets(void)
{
    U32     u32SecOffset, u32SecRmnLen;

    u32SecRmnLen = (_prSecFlt->RmnReqCnt & TSP_SECFLT_RMNCNT_MASK) >> TSP_SECFLT_RMNCNT_SHFT;

    if ((_u32TS_PktAFFlag & 0x1) && u32SecRmnLen)
    {
        DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x11);

        _u32PktContiCnt[_u32EngId][_u32SfId]++;
        if (_u32TS_PktCCFlag != (_u32PktContiCnt[_u32EngId][_u32SfId] & 0xF))
        {
            _u32PktContiCnt[_u32EngId][_u32SfId] = _u32TS_PktCCFlag;

            RevertFlt();
            return TRUE;
        }

        //[PERF] Can remove for performance
        _DMA_RECACHE(0);

        u32SecOffset = 4;                                               // Skip transport stream header
        if (_u32TS_PktAFFlag & 0x2)
        {
            u32SecOffset += 1 + (_prCtrl->Pkt_CacheW1 & 0xFF);          // Skip adaptation field
        }

        if (_u32TS_PktPUSIFlag)
        {
            //[PROTECT] No following section protection.
            //[OBSOLETE] obsolete if never happen
            _DMA_RECACHE(u32SecOffset);
            if ((u32SecRmnLen < SEC_SIZE_MAX) && (_prCtrl->Pkt_CacheW0 & 0xFF) < u32SecRmnLen)
            {
                DBG_FWTSP_FLAG(DEBUG_FLAG_ERROR, 3);

                RevertFlt();
                return TRUE;
            }
            //[PROTECT]

            u32SecOffset += 1;
        }

        #if SEC_HDR_BREAK_SUPPORT
        if (_u32SecByte[_u32EngId][_u32SfId] == 2)
        {
            _DMA_RECACHE(u32SecOffset);
            u32SecRmnLen = 3 - 2 + ( _u32SecLenData[_u32EngId][_u32SfId] | (_prCtrl->Pkt_CacheW0 & 0xFF) );
            _u32DataSize[_u32EngId][_u32SfId] = u32SecRmnLen + 2;
        }
        if (_u32SecByte[_u32EngId][_u32SfId] == 1)
        {
            _DMA_RECACHE(u32SecOffset);
            u32SecRmnLen = 3 - 1 + ( ((_prCtrl->Pkt_CacheW0 & 0x0000000F) << 8) |
                                     ((_prCtrl->Pkt_CacheW0 & 0x0000FF00) >> 8) );
            _u32DataSize[_u32EngId][_u32SfId] = u32SecRmnLen + 1;
        }

        REG_W(_prSecFlt->RmnReqCnt, (REG_R(_prSecFlt->RmnReqCnt) & ~TSP_SECFLT_RMNCNT_MASK) |
                                    (u32SecRmnLen << TSP_SECFLT_RMNCNT_SHFT) );
        _u32SecByte[_u32EngId][_u32SfId] = 0;
        _u32SecLenData[_u32EngId][_u32SfId] = 0;
        #endif

        #if SEC_PTN_BREAK_SUPPORT
        if (_u32PtnByte[_u32EngId][_u32SfId])
        {
            U32 byte = _u32PtnByte[_u32EngId][_u32SfId];
            BOOL bMatch = _PatternMatchTail(u32SecOffset);

            if ( !bMatch )        // Leave condition
            {
                RevertFlt();
                return TRUE;
            }

            if (NotVersionEnabled(_u32SfId) &&  (byte < 6))
            {
                U32 drop = 0;

                if (byte == 1)
                {
                    if (((_prCtrl->Pkt_CacheW1 >> 25)&SEC_VER_MASK) == NotVersionNum(_u32SfId))
                    {
                        drop = 1;
                    }
                }
                else
                {
                    if (((_prCtrl->Pkt_CacheW0 >> ((5 - byte)*8 + 1))&SEC_VER_MASK) == NotVersionNum(_u32SfId))
                    {
                        drop = 1;
                    }
                }

                if (drop)
                {
                    RevertFlt();

                    // version number matched, drop section and clear ptn break match data
                    // _prSecBuf->BufCur = _prSecBuf->BufWrite;
                    // _prSecFlt->RmnReqCnt = (_prSecFlt->RmnReqCnt & ~TSP_SECFLT_RMNCNT_MASK);
                    return TRUE;
                }
            }
        }
        #endif

        // DMA copy section data
        _bTail = FALSE;
        _prCtrl->Pkt_DmaAddr = u32SecOffset;
        if ((u32SecOffset + u32SecRmnLen) <= TS_PACKET_SIZE)
        {
            _u32DmaSize = u32SecRmnLen;                                 // DMA size
            _u32DmaCtrl = ( TSP_PKT_DMACTRL_CRC32 |                     // DMA command
                            #if SEC_128_ALIGN_SUPPORT
                            TSP_PKT_DMACTRL_TAIL  |
                            #endif
                            TSP_PKT_DMACTRL_START  );

            _bTail = TRUE;
        }
        else
        {
            _u32DmaSize = TS_PACKET_SIZE - u32SecOffset;                // DMA size
            _u32DmaCtrl = ( TSP_PKT_DMACTRL_CRC32 |                     // DMA command
                            TSP_PKT_DMACTRL_START  );

        }

        DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x12);

        _Dma_PreCheck();
        if (_Dma_PostCheck() == FALSE)
        {
            DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x13);

            RevertFlt();
            return FALSE;
        }

        MEM32(_u32BufPtr[_u32EngId][_u32SfId]) += _u32DmaSize;

        #if !SEC_CRC_CHECK_SUPPORT
        if (_bTail)
        {
            _UpdateWritePtr();
        }
        #endif

        #if SEC_REQ_NOTIFY_SUPPORT
        _UpdateWritePtr();
        #endif

        #if SEC_REQ_NOTIFY_SUPPORT
        u32ReqCnt = (_prSecFlt->RmnReqCnt & TSP_SECFLT_REQCNT_MASK) >> TSP_SECFLT_REQCNT_SHFT;
        if (u32ReqCnt)
        {
            _u32FltDataCnt[_u32EngId][_u32SfId] += _u32DmaSize;

            if (_u32FltDataCnt[_u32EngId][_u32SfId] >= u32ReqCnt)
            {
                _u32FltDataCnt[_u32EngId][_u32SfId] -= u32ReqCnt;
                _FireInterrupt(TSP_SWINT_STATUS_REQ_RDY, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
            }
        }
        #endif // SEC_REQ_NOTIFY_SUPPORT

        _prSecFlt->RmnReqCnt -= (_u32DmaSize << TSP_SECFLT_RMNCNT_SHFT);

        if (_bTail)
        {
            #if FWTSP_INFO_ENABLE
            _u32RdyCnt++;
            #endif

            #if SEC_ONESHOT_SUPPORT
            if (_prSecFlt->Ctrl & (TSP_SECFLT_MODE_ONESHOT<<TSP_SECFLT_MODE_SHFT))
            {
                _prSecFlt->Ctrl = _prSecFlt->Ctrl | (TSP_SECFLT_STATE_DISABLE<<TSP_SECFLT_STATE_SHFT);
            }
            #endif // SEC_ONESHOT_SUPPORT

            if (_prSecFlt->CRC32 == 0)
            {
                #if SEC_CRC_CHECK_SUPPORT
                _UpdateWritePtr();
                #endif

                FinishFlt();
            }
            else
            {
                #if SEC_CRC_CHECK_SUPPORT
                if (_prSecFlt->Ctrl & (TSP_SECFLT_MODE_CRCCHK<<TSP_SECFLT_MODE_SHFT))
                {
                    RevertFlt();
                }
                else
                {
                    // CRC check disabled, send section ready
                    _UpdateWritePtr();
                    FinishFlt();
                }
                #else
                _FireInterrupt(TSP_SWINT_STATUS_SEC_CRCERR, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
                #endif
            }
        }

        DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x14);
    }

    return TRUE;
}


static BOOL _GetSecFirstPacket(void)
{
    U32     u32SecOffset, u32SecLength;
    BOOL    bMatch;

    if (_u32TS_PktPUSIFlag && (_u32TS_PktAFFlag & 0x1)) // payload_unit_start_indicator
    {
        DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x1);

        u32SecOffset = 4;                                               // Skip transport stream header
        if (_u32TS_PktAFFlag & 0x2)
        {
            u32SecOffset += (1 + (_prCtrl->Pkt_CacheW1 & 0xFF));          // Skip  adaptation field
        }

        // Update cache to get payload_unit_start pointer
        //[NOTE] Jerry
        // It stands on an assumption CacheIdx will not be TS_PACKET_SIZE or hardware
        // can cache reasonable data (eg. 0xFF)
        _DMA_RECACHE(u32SecOffset);
        u32SecOffset = u32SecOffset + 1 + (_prCtrl->Pkt_CacheW0 & 0xFF);

        DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x2);

        //-------------------------------
        // Get all section in the packet
        //-------------------------------
        //[NOTE] Jerry
        // It stands on an assumption "section header will not cross two packet"
        #if SEC_HDR_BREAK_SUPPORT
        while (u32SecOffset < TS_PACKET_SIZE)                           // Leave condition
        #else
        while (u32SecOffset <= (TS_PACKET_SIZE-4))                      // Leave condition
        #endif
        {
            _DMA_RECACHE(u32SecOffset);
            if ((_prCtrl->Pkt_CacheW0 & 0xFF) == 0xFF)                  // Leave condition
            {
                break;
            }

            DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x3);

            #if SEC_HDR_BREAK_SUPPORT
            if (u32SecOffset == (TS_PACKET_SIZE-2))
            {
                _u32SecByte[_u32EngId][_u32SfId] = 2;
                _u32SecLenData[_u32EngId][_u32SfId] = (_prCtrl->Pkt_CacheW0 & 0x00000F00);
                u32SecLength = 0xFFFF;
            }
            else if (u32SecOffset == (TS_PACKET_SIZE-1))
            {
                _u32SecByte[_u32EngId][_u32SfId] = 1;
                _u32SecLenData[_u32EngId][_u32SfId] = 0;
                u32SecLength = 0xFFFF;
            }
            else
            #endif
            u32SecLength = 3 + ((_prCtrl->Pkt_CacheW0 & 0x00000F00) |
                                ((_prCtrl->Pkt_CacheW0 & 0x00FF0000) >> 16)); // section header + section data


            _u32DataSize[_u32EngId][_u32SfId] = u32SecLength;

            bMatch = _PatternMatchHead(u32SecOffset);

            _bTail = FALSE;

            if (bMatch)
            {
                if (NotVersionEnabled(_u32SfId))
                {
                    if (u32SecOffset < (TS_PACKET_SIZE - 5))
                    {
                        if (((_prCtrl->Pkt_CacheW1 >> 9) & SEC_VER_MASK) ==
                                NotVersionNum(_u32SfId))
                        {
                            // version number matched, drop section and clear ptn break match data
                            _u32PtnByte[_u32EngId][_u32SfId] = 0;

                            RevertFlt();
                            return TRUE;
                        }
                    }
                }

                // Basic information intilization
                _u32PktContiCnt[_u32EngId][_u32SfId] = _u32TS_PktCCFlag; // keep continuity count

                #if SEC_REQ_NOTIFY_SUPPORT
                _u32FltDataCnt[_u32EngId][_u32SfId] = 0;
                #endif // SEC_REQ_NOTIFY_SUPPORT

                _prSecFlt->RmnReqCnt = (_prSecFlt->RmnReqCnt & ~TSP_SECFLT_RMNCNT_MASK) |
                                       (u32SecLength << TSP_SECFLT_RMNCNT_SHFT);

                // DMA copy section data

                _prCtrl->Pkt_DmaAddr = u32SecOffset;
                if ((u32SecOffset + u32SecLength) <= TS_PACKET_SIZE)
                {
                    _u32DmaSize = u32SecLength;                         // DMA size
                    _u32DmaCtrl = ( TSP_PKT_DMACTRL_CRC32 |             // DMA command
                                    TSP_PKT_DMACTRL_CRCRST |
                                    #if SEC_128_ALIGN_SUPPORT
                                    TSP_PKT_DMACTRL_HEAD   |
                                    TSP_PKT_DMACTRL_TAIL   |
                                    #endif // SEC_128_ALIGN_SUPPORT
                                    TSP_PKT_DMACTRL_START   );

                    _bTail = TRUE;
                }
                else
                {
                    _u32DmaSize = TS_PACKET_SIZE - u32SecOffset;        // DMA size
                    _u32DmaCtrl = ( TSP_PKT_DMACTRL_CRC32  |            // DMA command
                                    TSP_PKT_DMACTRL_CRCRST |
                                    #if SEC_128_ALIGN_SUPPORT
                                    TSP_PKT_DMACTRL_HEAD   |
                                    #endif // SEC_128_ALIGN_SUPPORT
                                    TSP_PKT_DMACTRL_START   );
                }

                DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x4);

                _Dma_PreCheck();
                if (_Dma_PostCheck() == FALSE)
                {
                    DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x5);

                    RevertFlt();
                    return FALSE;
                }

                MEM32(_u32BufPtr[_u32EngId][_u32SfId]) += _u32DmaSize;

                #if !SEC_CRC_CHECK_SUPPORT
                if (_bTail)
                {
                    _UpdateWritePtr();
                }
                #endif

                #if SEC_REQ_NOTIFY_SUPPORT
                _UpdateWritePtr();
                #endif

                #if SEC_REQ_NOTIFY_SUPPORT
                if (_prSecFlt->RmnReqCnt)
                {
                    _u32FltDataCnt[_u32EngId][_u32SfId] += _u32DmaSize;

                    if (_u32FltDataCnt[_u32EngId][_u32SfId] >= _prSecFlt->RmnReqCnt)
                    {
                        _u32FltDataCnt[_u32EngId][_u32SfId] -= _prSecFlt->RmnReqCnt;
                        _FireInterrupt(TSP_SWINT_STATUS_REQ_RDY, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
                    }
                }
                #endif // SEC_REQ_NOTIFY_SUPPORT

                _prSecFlt->RmnReqCnt -= (_u32DmaSize << TSP_SECFLT_RMNCNT_SHFT);

                if (_bTail)
                {
                    #if FWTSP_INFO_ENABLE
                    _u32RdyCnt++;
                    #endif

                    #if SEC_ONESHOT_SUPPORT
                    if (_prSecFlt->Ctrl & (TSP_SECFLT_MODE_ONESHOT<<TSP_SECFLT_MODE_SHFT))
                    {
                        _prSecFlt->Ctrl = _prSecFlt->Ctrl | (TSP_SECFLT_STATE_DISABLE<<TSP_SECFLT_STATE_SHFT);
                    }
                    #endif // SEC_ONESHOT_SUPPORT

                    if (_prSecFlt->CRC32 == 0)
                    {
                        #if SEC_CRC_CHECK_SUPPORT
                        _UpdateWritePtr();
                        #endif

                        FinishFlt();
                    }
                    else
                    {
                        #if SEC_CRC_CHECK_SUPPORT
                        if (_prSecFlt->Ctrl & (TSP_SECFLT_MODE_CRCCHK<<TSP_SECFLT_MODE_SHFT))
                        {
                            // Reject CRC error section
                            RevertFlt();
                        }
                        else
                        {
                            // CRC check disabled, send section ready
                            _UpdateWritePtr();
                            FinishFlt();
                        }
                        #else
                        RevertFlt();
                        _FireInterrupt(TSP_SWINT_STATUS_SEC_CRCERR, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
                        #endif
                    }
                }

            }
            else{
            #if SEC_HDR_BREAK_SUPPORT
                _u32SecByte[_u32EngId][_u32SfId] = 0;
                _u32SecLenData[_u32EngId][_u32SfId] = 0;
            #endif
            }

            // Get next section in the same packet
            u32SecOffset += u32SecLength;
        }

        DBG_FWTSP_FLAG(DEBUG_FLAG_SEC, 0x6);
    }

    return TRUE;
}


// 1. check if it's a following packets
// 2. check and skip adaptation field
// 3. check payload_unit_start
// 4. DMA PES remain data
// 5. Check DMA overflow
// 6. Check REQ notification
static BOOL _GetPESFollowPackets(U32 opt)
{
    U32     u32DmaBitrev;
    U32     u32PesOffset, u32PesRmnLen;
#if REQ_NOTIFY_SUPPORT
    U32     u32ReqCnt;
#endif
    BOOL    bMatch;

    u32PesRmnLen = (_prSecFlt->RmnReqCnt & TSP_SECFLT_RMNCNT_MASK) >> TSP_SECFLT_RMNCNT_SHFT;

    // Do _u32TS_PktPUSIFlag outside
    if ((_u32TS_PktAFFlag & 0x1) && u32PesRmnLen) // Payload
    {
        #if PES_CC_CHECK_SUPPORT
        _u32PktContiCnt[_u32EngId][_u32SfId]++;
        if (_u32TS_PktCCFlag != (_u32PktContiCnt[_u32EngId][_u32SfId] & 0xF))
        {
            _u32PktContiCnt[_u32EngId][_u32SfId] = _u32TS_PktCCFlag;

            RevertFlt();
            return TRUE;
        }
        #endif // PES_CC_CHECK_SUPPORT

        //[PERF] Can remove for performance
        _DMA_RECACHE(0);

        u32DmaBitrev = 0;
        u32PesOffset = 4;                                               // Skip transport stream header

        if (opt == PES_OPT_TTX)
        {
            u32DmaBitrev = TSP_PKT_DMACTRL_BITREV;
        }
        else if (_u32TS_PktAFFlag & 0x2)                                // Teletext only allow 0x1 and 0x2, no 0x3
        {
            u32PesOffset += (1 + (_prCtrl->Pkt_CacheW1 & 0xFF));          // Skip  adaptation field
        }

        // PES header broken
        if (_u32SecByte[_u32EngId][_u32SfId] > 0)
        {
            U32 u32PesLength;

            _DMA_RECACHE(u32PesOffset);

            switch(_u32SecByte[_u32EngId][_u32SfId])
            {
                case 1:
                    u32PesLength = ((_prCtrl->Pkt_CacheW0 >> 16) & 0xFF00) |
                                    ((_prCtrl->Pkt_CacheW1 >> 0) & 0x00FF);
                    break;
                case 2:
                    u32PesLength = ((_prCtrl->Pkt_CacheW0 >> 8) & 0xFF00) |
                                    ((_prCtrl->Pkt_CacheW0 >> 24) & 0x00FF);
                    break;
                case 3:
                    u32PesLength = ((_prCtrl->Pkt_CacheW0 >> 0) & 0xFF00) |
                                    ((_prCtrl->Pkt_CacheW0 >> 16) & 0x00FF);
                    break;
                case 4:
                    u32PesLength = ((_prCtrl->Pkt_CacheW0 << 8) & 0xFF00) |
                                    ((_prCtrl->Pkt_CacheW0 >> 8) & 0x00FF);
                    break;
                case 5:
                    u32PesLength = ((_u32SecLenData[_u32EngId][_u32SfId] << 8) |
                                    (_prCtrl->Pkt_CacheW0 & 0xFF));
                    break;
                default:
                    RevertFlt();
                    return TRUE;
            }

            u32PesRmnLen =  u32PesLength + 6 - _u32SecByte[_u32EngId][_u32SfId];
            if (u32PesLength == 0)
            {
                u32PesLength = VPES_SIZE_MAX;
            }
            else
            {
                u32PesLength += 6;
            }

            _u32DataSize[_u32EngId][_u32SfId] = u32PesLength;

            _prSecFlt->RmnReqCnt = (_prSecFlt->RmnReqCnt & ~TSP_SECFLT_RMNCNT_MASK) |
                                   ((u32PesRmnLen & (TSP_SECFLT_RMNCNT_MASK >> TSP_SECFLT_RMNCNT_SHFT))
                                   << TSP_SECFLT_RMNCNT_SHFT);
            _u32SecByte[_u32EngId][_u32SfId] = 0;
        }

        bMatch = _PatternMatchTail(u32PesOffset);

        // DMA copy section data
        _bTail = FALSE;

        if (bMatch)
        {
            _prCtrl->Pkt_DmaAddr = u32PesOffset;
            if ((u32PesOffset + u32PesRmnLen) <= TS_PACKET_SIZE)
            {
                _u32DmaSize = u32PesRmnLen;                                 // DMA size
                _u32DmaCtrl = (                                             // DMA coommand
                                #if SEC_128_ALIGN_SUPPORT
                                TSP_PKT_DMACTRL_TAIL  |
                                #endif
                                u32DmaBitrev          |
                                TSP_PKT_DMACTRL_START  );

                _bTail = TRUE;
            }
            else
            {
                _u32DmaSize = TS_PACKET_SIZE - u32PesOffset;                // DMA size
                _u32DmaCtrl = (                                             // DMA command
                                u32DmaBitrev          |
                                TSP_PKT_DMACTRL_START  );
            }

            _Dma_PreCheck();
            if (_Dma_PostCheck() == FALSE)
            {
                RevertFlt();
                return FALSE;
            }

            MEM32(_u32BufPtr[_u32EngId][_u32SfId]) += _u32DmaSize;

            if (_bTail)
            {
                _UpdateWritePtr();
            }
            else
            {
                #if REQ_NOTIFY_SUPPORT
                _UpdateWritePtr();
                #endif // REQ_NOTIFY_SUPPORT
            }

            #if REQ_NOTIFY_SUPPORT
            u32ReqCnt = (_prSecFlt->RmnReqCnt & TSP_SECFLT_REQCNT_MASK) >> TSP_SECFLT_REQCNT_SHFT;
            if (u32ReqCnt)
            {
                _u32FltDataCnt[_u32EngId][_u32SfId] += _u32DmaSize;

                if (_u32FltDataCnt[_u32EngId][_u32SfId] >= u32ReqCnt)
                {
                    _u32FltDataCnt[_u32EngId][_u32SfId] -= u32ReqCnt;
                    _FireInterrupt(TSP_SWINT_STATUS_REQ_RDY, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
                }
            }
            #endif // REQ_NOTIFY_SUPPORT

           if (_u32DataSize[_u32EngId][_u32SfId] != VPES_SIZE_MAX) // 2009.09.16 - Support Video PES filtering
            _prSecFlt->RmnReqCnt -= (_u32DmaSize << TSP_SECFLT_RMNCNT_SHFT);

            if (_bTail)
            {
                #if FWTSP_INFO_ENABLE
                _u32RdyCnt++;
                #endif

                FinishFlt();
            }
        }
        else
        {
            RevertFlt();
        }

        return FALSE; // already process, no need to check PES head
    }

    return TRUE;
}


static BOOL _GetPESFirstPacket(U32 opt)
{
    U32     u32DmaBitrev;
    U32     u32PesOffset, u32PesLength;
#if REQ_NOTIFY_SUPPORT
    U32     u32ReqCnt;
#endif
    U32     u32PesRmnLen;
    BOOL    bMatch;

#if VPES_ENABLE
    //------------------------------------------------//
    // 2009.09.16 - Support Video PES filtering
    if(_u32DataSize[_u32EngId][_u32SfId] == VPES_SIZE_MAX)
    {
        #if FWTSP_INFO_ENABLE
        _u32RdyCnt++;
        #endif
        FinishFlt();
        _prSecFlt->RmnReqCnt = _prSecFlt->RmnReqCnt & ~TSP_SECFLT_RMNCNT_MASK; // clear remain
    }
    //------------------------------------------------//
#endif

    // Do _u32TS_PktPUSIFlag outside
    if (_u32TS_PktAFFlag & 0x1) // payload
    {
        u32DmaBitrev = 0;
        u32PesOffset = 4;                                               // Skip transport stream header

        if (opt == PES_OPT_TTX)
        {
            u32DmaBitrev = TSP_PKT_DMACTRL_BITREV;
        }
        else if (_u32TS_PktAFFlag & 0x2)                                // Teletext only allow 0x1 and 0x2, no 0x3
        {
            u32PesOffset += (1 + (_prCtrl->Pkt_CacheW1 & 0xFF));           // Skip  adaptation field
        }

        bMatch = _PatternMatchHead(u32PesOffset);

        // PES Header Broken
        if (u32PesOffset > 182)
        {
            _u32SecByte[_u32EngId][_u32SfId]  = TS_PACKET_SIZE - u32PesOffset;
            if (u32PesOffset == 183)
            {
                _u32SecLenData[_u32EngId][_u32SfId] = (_prCtrl->Pkt_CacheW1 & 0xFF);
            }
            u32PesLength = 0xFFFF;
        }
        else
        {
            _u32SecByte[_u32EngId][_u32SfId]  = 0;

            u32PesLength = ((_prCtrl->Pkt_CacheW1 & 0x00FF) << 8) |
                           ((_prCtrl->Pkt_CacheW1 & 0xFF00) >> 8) ;
            if (u32PesLength == 0)
            {
                u32PesLength = VPES_SIZE_MAX;                               //[NOTE] Choose maximum value to simulate first
            }
            else
            {
                u32PesLength += 6;                                          // PES length + header
            }
        }

        _u32DataSize[_u32EngId][_u32SfId] = u32PesLength;

        if (opt == PES_OPT_TTX)
        {
            u32PesLength -= 46;
            u32PesOffset += 46;

            //[PROTECT] Only output 46*n data for a PES packet to prevent stream data error.
            u32PesLength = (u32PesLength / 46) * 46;
            //[PROTECT]
        }

        //-------------------------------
        // Get PES in the packet
        //-------------------------------
        _bTail = FALSE;

        if (bMatch)
        {
            u32PesRmnLen = (_prSecFlt->RmnReqCnt & TSP_SECFLT_RMNCNT_MASK) >> TSP_SECFLT_RMNCNT_SHFT;

            if (u32PesRmnLen > 0)
            {
                // pes_packet_length is longer than the data received. Need to set the write pointer to the previous write pointer
                RevertFlt();
            }

            #if PES_CC_CHECK_SUPPORT
            _u32PktContiCnt[_u32EngId][_u32SfId] = _u32TS_PktCCFlag; // keep continuity count
            #endif // PES_CC_CHECK_SUPPORT

            #if REQ_NOTIFY_SUPPORT
            _u32FltDataCnt[_u32EngId][_u32SfId] = 0;
            #endif // REQ_NOTIFY_SUPPORT

            _prSecFlt->RmnReqCnt = (_prSecFlt->RmnReqCnt & ~TSP_SECFLT_RMNCNT_MASK) |
                                   ((u32PesLength & (TSP_SECFLT_RMNCNT_MASK >> TSP_SECFLT_RMNCNT_SHFT))
                                   << TSP_SECFLT_RMNCNT_SHFT);
/*
            //[TEST]
            if (opt == PES_OPT_TTX)
            {
                //[NOTE]
                // Only output 46*n data for a PES packet to prevent stream data error.
                u32PesLength = (u32PesLength / 46) * 46;
            }
            //[TEST]
*/
            // DMA copy section data
            _prCtrl->Pkt_DmaAddr = u32PesOffset;
            if ((u32PesOffset + u32PesLength) <= TS_PACKET_SIZE)
            {
                _u32DmaSize = u32PesLength;                             // DMA size
                _u32DmaCtrl = (                                         // DMA command
                                #if SEC_128_ALIGN_SUPPORT
                                TSP_PKT_DMACTRL_HEAD  |
                                TSP_PKT_DMACTRL_TAIL  |
                                #endif // SEC_128_ALIGN_SUPPORT
                                u32DmaBitrev          |
                                TSP_PKT_DMACTRL_START  );

                _bTail = TRUE;
            }
            else
            {
                _u32DmaSize = TS_PACKET_SIZE - u32PesOffset;            // DMA size
                _u32DmaCtrl = (                                         // DMA command
                                #if SEC_128_ALIGN_SUPPORT
                                TSP_PKT_DMACTRL_HEAD  |
                                #endif // SEC_128_ALIGN_SUPPORT
                                u32DmaBitrev          |
                                TSP_PKT_DMACTRL_START  );

            }

            _Dma_PreCheck();
            if (_Dma_PostCheck() == FALSE)
            {
                RevertFlt();
                return FALSE;
            }

            MEM32(_u32BufPtr[_u32EngId][_u32SfId]) += _u32DmaSize;

            if (_bTail)
            {
                _UpdateWritePtr();
            }
            else
            {
                #if REQ_NOTIFY_SUPPORT
                _UpdateWritePtr();
                #endif // REQ_NOTIFY_SUPPORT
            }

            #if REQ_NOTIFY_SUPPORT
            u32ReqCnt = (_prSecFlt->RmnReqCnt & TSP_SECFLT_REQCNT_MASK) >> TSP_SECFLT_REQCNT_SHFT;
            if (u32ReqCnt)
            {
                _u32FltDataCnt[_u32EngId][_u32SfId] += _u32DmaSize;

                if (_u32FltDataCnt[_u32EngId][_u32SfId] >= u32ReqCnt)
                {
                    _u32FltDataCnt[_u32EngId][_u32SfId] -= u32ReqCnt;
                    _FireInterrupt(TSP_SWINT_STATUS_REQ_RDY, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
                }
            }
            #endif // REQ_NOTIFY_SUPPORT

            _prSecFlt->RmnReqCnt -= (_u32DmaSize << TSP_SECFLT_RMNCNT_SHFT);

            if (_bTail)
            {
                #if FWTSP_INFO_ENABLE
                _u32RdyCnt++;
                #endif

                FinishFlt();
            }
        }
        else
        {
            RevertFlt();
        }

        return FALSE; // Already processed
    }

    return TRUE;
}


static BOOL _GetTSPacket(void)
{
#if REQ_NOTIFY_SUPPORT
    U32     u32ReqCnt;
#endif

    DBG_FWTSP_FLAG(DEBUG_FLAG_PKT, 0x1);

#if PKT_MATCH_SUPPORT
    _DMA_RECACHE(0);
    if (_prCtrl->Pkt_Stat & TSP_PKT_STAT_CMP_MATCH)
    {
#endif // PKT_MATCH_SUPPORT

        DBG_FWTSP_FLAG(DEBUG_FLAG_PKT, 0x2);

        // Copy full packet to section buffer
        _u32DmaSize = TS_PACKET_SIZE;                                   // DMA size
        _u32DmaCtrl = (                                                 // DMA command
                        #if SEC_128_ALIGN_SUPPORT
                        TSP_PKT_DMACTRL_HEAD  |
                        TSP_PKT_DMACTRL_TAIL  |
                        #endif // SEC_128_ALIGN_SUPPORT
                        TSP_PKT_DMACTRL_START );

        _prCtrl->Pkt_DmaAddr   = 0;                                     // DMA address

        DBG_FWTSP_FLAG(DEBUG_FLAG_PKT, 0x3);

        _Dma_PreCheck();
        if (_Dma_PostCheck() == FALSE)
        {
            DBG_FWTSP_FLAG(DEBUG_FLAG_PKT, 0x4);
            return FALSE;
        }

        _u32DataSize[_u32EngId][_u32SfId] = _u32DmaSize;
        MEM32(_u32BufPtr[_u32EngId][_u32SfId]) += _u32DmaSize;

        _UpdateWritePtr();

        #if FWTSP_INFO_ENABLE
        _u32RdyCnt++;
        #endif

        #if REQ_NOTIFY_SUPPORT
        u32ReqCnt = (_prSecFlt->RmnReqCnt & TSP_SECFLT_REQCNT_MASK) >> TSP_SECFLT_REQCNT_SHFT;
        if (u32ReqCnt)
        {
            _u32FltDataCnt[_u32EngId][_u32SfId] += _u32DmaSize;

            if (_u32FltDataCnt[_u32EngId][_u32SfId] >= u32ReqCnt)
            {
                _u32FltDataCnt[_u32EngId][_u32SfId] -= u32ReqCnt;
                _FireInterrupt(TSP_SWINT_STATUS_REQ_RDY, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
            }
        }
        #endif // REQ_NOTIFY_SUPPORT

        //_FireInterrupt(TSP_SWINT_STATUS_SEC_RDY, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT)|(_u32SfId<<TSP_SWINT_INFO_SEC_SHFT));
        FinishFlt();

        DBG_FWTSP_FLAG(DEBUG_FLAG_PKT, 0x6);

#if PKT_MATCH_SUPPORT
    }
#endif // PKT_MATCH_SUPPORT

    return TRUE;
}

#if PCR_STC_SYNC_ENABLE
static U32 _GetSynthesizer(U32 u32T, BOOL bIncrease)
{
    static U32 u32SynValue = 0;
    U32 u32Freqency = 0;
    U32 u32Temp, u32Temp1, u32Temp2;

    if (bIncrease)
    {
        u32Freqency = 86400000 + 960*u32T;
    }
    else
    {
        u32Freqency = 86400000 - 960*u32T;
    }

    u32Temp = 216000000/u32Freqency;
    u32Temp1 = (216000000 - u32Temp*u32Freqency)/960;
    u32Temp2 = u32Freqency/960;

    //u32SynValue = ((u32Temp << 27)&0xf8000000) | ((((u32Temp1*0x800)/u32Temp2)<<16)&0x7FFFFFF);
    //u32SynValue = ((u32Temp << 27)&0xf8000000) | ((((u32Temp1*0x8000)/u32Temp2)<<12)&0x7FFFFFF);
    u32SynValue = ((u32Temp << 27)&0xf8000000) | ((((u32Temp1*0x10000)/u32Temp2)<<11)&0x7FFFFFF);
/*
    if (u32SynValue > MAX_STC_SYNTH)
    {
        u32SynValue = MAX_STC_SYNTH;
    }
    if (u32SynValue < MIN_STC_SYNTH)
    {
        u32SynValue = MIN_STC_SYNTH;
    }
*/
/*
    if (bIncrease)
    {
        if (u32SynValue > DEF_STC_SYNTH)
        {
            u32SynValue = DEF_STC_SYNTH;
        }
    }
    else
    {
        if (u32SynValue < DEF_STC_SYNTH)
        {
            u32SynValue = DEF_STC_SYNTH;
        }
    }
*/
    return u32SynValue;
}
#endif // #if PCR_STC_SYNC_ENABLE

static BOOL _GetAdaptField(void)
{
    U32     u32AFLength;
    U32     u32PcrBase, u32PcrExt, u32PcrBit32;


//[TEMP]    DBG_FWTSP_FLAG(DEBUG_FLAG_AF, 0x1);

    // only if Adaptation_Field = 1 that should contain adaptation field
    if (_u32TS_PktAFFlag & 0x2) //[OBSOLETE]
    {
        DBG_FWTSP_FLAG(DEBUG_FLAG_AF, 0x2);

        u32AFLength = (_prCtrl->Pkt_CacheW1 & 0xFF) + 1;                // Get the AF length

        if (u32AFLength <= 1)
        {
            // adaptation_field_length == 0
            return TRUE;
        }

        #define REG_SYNTH()                 (*(volatile unsigned long*)(REG_SYNTH_VAL))

        #define STC_SYNC_INT                12
        #define STC_SYNC_FRAC               20
        #define STC_SYNC_ONE                0x00100000

        #define STC_SYNC_STEP               10
        #define STC_SYNC_1_F_FRAC           8

        #define STC_SYNC_1_F_27M            (U32)(0.4 * STC_SYNC_ONE)           // 0x00066666
        #define STC_SYNC_1_F_10000_S        (U32)(0.044444444 * STC_SYNC_ONE)   // 0xB60B
        #define STC_SYNC_1_F_LVL            (U32)(STC_SYNC_1_F_27M / STC_SYNC_1_F_10000_S - 1)

        #define STC_SYNC_1_F_MAX            (STC_SYNC_1_F_27M + (STC_SYNC_1_F_LVL + STC_SYNC_1_F_10000_S))
        #define STC_SYNC_1_F_MIN            (STC_SYNC_1_F_27M - (STC_SYNC_1_F_LVL + STC_SYNC_1_F_10000_S))

        #define STC_SYNC_1_F_1000_100MS     (U32)(STC_SYNC_1_F_10000_S)         // 0xB60B
        #define STC_SYNC_1_F_100_100MS      (U32)(STC_SYNC_1_F_10000_S / 10)    // 0x1234
        #define STC_SYNC_1_F_10_100MS       (U32)(STC_SYNC_1_F_10000_S / 100)   // 0x01D2
        #define STC_SYNC_1_F_10_1S          (U32)(STC_SYNC_1_F_10000_S / 1000)  // 0x002E


        if (_prCtrl->Pkt_CacheW1 & 0x00001000)                           // Only if PCR_flag = 1
        {
#if PCR_STC_SYNC_ENABLE
            static U32 u32LastStcSynth[TSP_ENGINE_NUM];
#endif

            u32PcrBit32 = (_prCtrl->Pkt_CacheW1 &   0x00800000) >> 23;

            u32PcrBase  = (((_prCtrl->Pkt_CacheW1 & 0x00FF0000) >> 16) << (24+1)) |
                          (((_prCtrl->Pkt_CacheW1 & 0xFF000000) >> 24) << (16+1)) |
                          (((_prCtrl->Pkt_CacheW2 & 0x000000FF) >>  0) << ( 8+1)) |
                          (((_prCtrl->Pkt_CacheW2 & 0x0000FF00) >>  8) << (   1)) |
                          (((_prCtrl->Pkt_CacheW2 & 0x00800000) >> 23));

            u32PcrExt   = ((_prCtrl->Pkt_CacheW2 &  0x00010000) >>  8) |
                          ((_prCtrl->Pkt_CacheW2 &  0xFF000000) >> 24);

            if (_pcr_ofs >= 0)
            {
                u32PcrBase += _pcr_ofs;
            }
            else
            {
                u32PcrBase -= (-_pcr_ofs);
            }
            _rCtrl[0].MCU_MSG = u32PcrBase;
            _rCtrl[0].MCU_MSG2 = u32PcrBit32;

#if !defined(_CMODEL)

#if PCR_STC_SYNC_ENABLE

#if USE_STC1_FOR_COMPARE
            if (_u32EngId == 1)
            {
                if (REG_R(_prSecFlt->Ctrl) & (TSP_SECFLT_PARAM_PCRRST<<TSP_SECFLT_PARAM_SHFT))
                {
                    REG_W(_prSecFlt->Ctrl, REG_R(_prSecFlt->Ctrl) & ~(TSP_SECFLT_PARAM_PCRRST<<TSP_SECFLT_PARAM_SHFT));

                    u32LastStcSynth[_u32EngId] = STC_SYNC_1_F_27M; // 1/f = 0.4
                    // REG_SYNTH(_u32EngId) = DEF_STC_SYNTH;
                    // _FireInterrupt(TSP_SWINT_CMD_STC_UPD, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                    REG_SYNTH()= DEF_STC_SYNTH;
                    REG_SYNTH_ENABLE();
                }
                #if SYNC_DEBUG
                *(U32*)((&dbg_test0)) = REG_SYNTH(_u32EngId);
                *(U32*)((&dbg_test0)+1) = u32PcrBase;
                *(U32*)((&dbg_test0)+2) = 0;
                *(U32*)((&dbg_test0)+3) = 0;
                #endif //SYNC_DEBUG

                // _rCtrl[0].Stc[_u32EngId].ML = u32PcrBase;
                // _rCtrl[0].Stc[_u32EngId].H32 = u32PcrBit32;
                _rCtrl[0].Pcr.ML = u32PcrBase;
                _rCtrl[0].Pcr.H32 = u32PcrBit32;
            }
            else
#endif //USE_STC1_FOR_COMPARE
            {
                if (REG_R(_prSecFlt->Ctrl) & (TSP_SECFLT_PARAM_PCRRST<<TSP_SECFLT_PARAM_SHFT))
                {
                    // First PCR, clear PCR reset
                    REG_W(_prSecFlt->Ctrl, REG_R(_prSecFlt->Ctrl) & ~(TSP_SECFLT_PARAM_PCRRST<<TSP_SECFLT_PARAM_SHFT));

                    u32LastStcSynth[_u32EngId] = STC_SYNC_1_F_27M; // 1/f = 0.4
                    // REG_SYNTH(_u32EngId) = DEF_STC_SYNTH;
                    REG_SYNTH()= DEF_STC_SYNTH;
                    REG_SYNTH_ENABLE();
                    #if SYNC_DEBUG
                    *(U32*)((&dbg_test0)) = DEF_STC_SYNTH;
                    *(U32*)((&dbg_test0)+1) = u32PcrBase;
                    *(U32*)((&dbg_test0)+2) = 0x12345678;
                    *(U32*)((&dbg_test0)+3) = 0x12345678;
                    #endif //SYNC_DEBUG
                    // _FireInterrupt(TSP_SWINT_CMD_STC_UPD, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));

                    // _rCtrl[0].Stc[_u32EngId].ML = u32PcrBase;
                    // _rCtrl[0].Stc[_u32EngId].H32 = u32PcrBit32;
                    _rCtrl[0].Pcr.ML = u32PcrBase;
                    _rCtrl[0].Pcr.H32 = u32PcrBit32;

                    _u32NewPCR[_u32EngId] = 0;
                    _bNewPCR[_u32EngId] = FALSE;
                    _u32PCRCount[_u32EngId] = 0;
                    _u32PCR[_u32EngId][0] = 0;
                    _u32PCR[_u32EngId][1] = 0;
                    _u32PCRId[_u32EngId] = 0;
                    _u32PreTick[_u32EngId] = STD_TICK;
                }
                else
                {
#if 0
                    #if SYNC_DEBUG
                    *(U32*)((&dbg_test0)) = 6;
                    *(U32*)((&dbg_test0)+1) = _rCtrl[0].Stc[_u32EngId].ML;
                    *(U32*)((&dbg_test0)+2) = u32PcrBase;
                    *(U32*)((&dbg_test0)+3) = (_rCtrl[0].Stc[_u32EngId].ML - u32PcrBase);
                    _FireInterrupt(TSP_SWINT_STATUS_DEBUG, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                    #endif //SYNC_DEBUG
#else
                    // U32     u32TmpSTC = _rCtrl[0].Stc[_u32EngId].ML;
                    U32     u32TmpSTC = _rCtrl[0].Pcr.ML;

                    _bCheckSynthesizer[_u32EngId] = FALSE;
                    if (_u32PCRCount[_u32EngId] == 0xFFFFFFFF)
                    {
                        _u32PCRCount[_u32EngId] = 0;
                    }
                    else
                    {
                        _u32PCRCount[_u32EngId]++;
                    }


                    if (_bNewPCR[_u32EngId])
                    {
                        //Discontinuous status is happen, check the second PCR.
                        //Avoid the first discontinuous PCR is the noise jitter.
                        if (_u32NewPCR[_u32EngId] < u32PcrBase)
                        {
                            //The current PCR is bigger than previous diccontinuous PCR.
                            //Discontinuity is true, rest the STC, but don't reset the Synthesizer.

                            u32LastStcSynth[_u32EngId] = STC_SYNC_1_F_27M; // 1/f = 0.4
                            // REG_SYNTH(_u32EngId) = DEF_STC_SYNTH;
                            // _FireInterrupt(TSP_SWINT_CMD_STC_UPD, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                            REG_SYNTH()= DEF_STC_SYNTH;
                            REG_SYNTH_ENABLE();
                            #if SYNC_DEBUG
                            *(U32*)((&dbg_test0)) = DEF_STC_SYNTH;
                            *(U32*)((&dbg_test0)+1) = u32PcrBase;
                            *(U32*)((&dbg_test0)+2) = 0x12345678;
                            *(U32*)((&dbg_test0)+3) = 0x12345678;
                            #endif //SYNC_DEBUG

                            // _rCtrl[0].Stc[_u32EngId].ML = u32PcrBase;
                            // _rCtrl[0].Stc[_u32EngId].H32 = u32PcrBit32;
                            _rCtrl[0].Pcr.ML = u32PcrBase;
                            _rCtrl[0].Pcr.H32 = u32PcrBit32;

                            _u32NewPCR[_u32EngId] = 0;
                            _bNewPCR[_u32EngId] = FALSE;
                            _u32PCRCount[_u32EngId] = 0;
                            _u32PCR[_u32EngId][0] = 0;
                            _u32PCR[_u32EngId][1] = 0;
                            _u32PCRId[_u32EngId] = 0;
                            _u32PreTick[_u32EngId] = STD_TICK;
                        }
                        else
                        {
                            _u32NewPCR[_u32EngId] = 0;
                            _bNewPCR[_u32EngId] = FALSE;
                            _u32PCRCount[_u32EngId] = 0;
                            _u32PCR[_u32EngId][0] = 0;
                            _u32PCR[_u32EngId][1] = 0;
                            _u32PCRId[_u32EngId] = 0;

                            #if SYNC_DEBUG
                            *(U32*)((&dbg_test0)) = 1;
                            *(U32*)((&dbg_test0)+1) = 0;
                            *(U32*)((&dbg_test0)+2) = 0;
                            *(U32*)((&dbg_test0)+3) = 0;
                            _FireInterrupt(TSP_SWINT_STATUS_DEBUG, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                            #endif //SYNC_DEBUG

                        }
                    }
                    else if ((Diff(u32TmpSTC, u32PcrBase) >= RESET_THD))
                    {
                        //1. Difference is bigger than RESET_THD, keep the PCR.
                        //2. discontinuity_indicator is true, keep the PCR.
                        _u32NewPCR[_u32EngId] = u32PcrBase;
                        _bNewPCR[_u32EngId] = TRUE;

                        #if SYNC_DEBUG
                        *(U32*)((&dbg_test0)) = 5;
                        *(U32*)((&dbg_test0)+1) = u32TmpSTC;
                        *(U32*)((&dbg_test0)+2) = u32PcrBase;
                        *(U32*)((&dbg_test0)+3) = 0;
                        _FireInterrupt(TSP_SWINT_STATUS_DEBUG, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                        #endif //SYNC_DEBUG

                    }
                    else if ((_prCtrl->Pkt_CacheW1 & 0x00008000))
                    {
                        //1. Difference is bigger than RESET_THD, keep the PCR.
                        //2. discontinuity_indicator is true, keep the PCR.
                        _u32NewPCR[_u32EngId] = u32PcrBase;
                        _bNewPCR[_u32EngId] = TRUE;

                        #if SYNC_DEBUG
                        *(U32*)((&dbg_test0)) = 7;
                        *(U32*)((&dbg_test0)+1) = u32TmpSTC;
                        *(U32*)((&dbg_test0)+2) = u32PcrBase;
                        *(U32*)((&dbg_test0)+3) = 0;
                        _FireInterrupt(TSP_SWINT_STATUS_DEBUG, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                        #endif //SYNC_DEBUG

                    }
                    else
                    {
                        U32     u32Temp;
                        U32     u32Temp1;
                        BOOL    bIncrease = TRUE;
                        U32     u32CurrPCRId = 1;
                        U32     u32PrevPCRId = 0;


                        //1. Both PCR are 0, keep current PCR.
                        //2. The current PCR is bigger than previous keeping PCR (> PCR_INTERVAL), keep current PCR.
                        if (((_u32PCR[_u32EngId][0] == 0) && (_u32PCR[_u32EngId][1] == 0)) ||
                            ((u32PcrBase - _u32PCR[_u32EngId][_u32PCRId[_u32EngId]])> PCR_INTERVAL))
                        {
                            u32PrevPCRId = _u32PCRId[_u32EngId];
                            u32CurrPCRId = (_u32PCRId[_u32EngId] > 0)?(0):(1);
                            _u32PCRId[_u32EngId] = u32CurrPCRId;

                            _u32PCR[_u32EngId][u32CurrPCRId] = u32PcrBase;
                            if (u32PcrBase >= u32TmpSTC)
                            {
                                _u32PCR_STC_Diff[_u32EngId][u32CurrPCRId] = u32PcrBase - u32TmpSTC;
                                _bPositiveDiff[_u32EngId][u32CurrPCRId] = TRUE;
                            }
                            else
                            {
                                _u32PCR_STC_Diff[_u32EngId][u32CurrPCRId] = u32TmpSTC - u32PcrBase;
                                _bPositiveDiff[_u32EngId][u32CurrPCRId] = FALSE;
                            }

                            if ((_u32PCR[_u32EngId][0] != 0) && (_u32PCR[_u32EngId][1] != 0))
                            {
                                _bCheckSynthesizer[_u32EngId] = TRUE;
                            }
                        }

                        //Ater reset STC and pass 5 PCR, Check the difference between PCR & STC.
                        if ((_u32PCRCount[_u32EngId] > SYNC_START) && (_bCheckSynthesizer[_u32EngId]))
                        {
                            //Get PCR interval
                            u32Temp = Diff(_u32PCR[_u32EngId][1],_u32PCR[_u32EngId][0]);
                            if (u32Temp < PCR_INTERVAL)
                            {
                                #if SYNC_DEBUG
                                *(U32*)((&dbg_test0)) = 2;
                                *(U32*)((&dbg_test0)+1) = u32Temp;
                                *(U32*)((&dbg_test0)+2) = 0;
                                *(U32*)((&dbg_test0)+3) = 0;
                                _FireInterrupt(TSP_SWINT_STATUS_DEBUG, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                                #endif
                                _u32PCR[_u32EngId][0] = 0;
                                _u32PCR[_u32EngId][1] = 0;
                                _u32PCRCount[_u32EngId] = 0;
                                return TRUE;
                            }

                            if (_bPositiveDiff[_u32EngId][0] == _bPositiveDiff[_u32EngId][1])
                            {
                                //Both Diffs are positive, or both Diffs are negative.
                                u32Temp1 = Diff(_u32PCR_STC_Diff[_u32EngId][0], _u32PCR_STC_Diff[_u32EngId][1]);
                                if (u32Temp1 > MAX_ADJUST_TICK)
                                {
                                    #if SYNC_DEBUG
                                    *(U32*)((&dbg_test0)) = 3;
                                    *(U32*)((&dbg_test0)+1) = _u32PCR_STC_Diff[_u32EngId][0];
                                    *(U32*)((&dbg_test0)+2) = _u32PCR_STC_Diff[_u32EngId][1];
                                    *(U32*)((&dbg_test0)+3) = 0;
                                    _FireInterrupt(TSP_SWINT_STATUS_DEBUG, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                                    #endif
                                    _u32PCR[_u32EngId][0] = 0;
                                    _u32PCR[_u32EngId][1] = 0;
                                    _u32PCRCount[_u32EngId] = 0;
                                    return TRUE;
                                }

                                if (u32Temp1 > SYNC_THD)
                                {
                                    if (((_bPositiveDiff[_u32EngId][0]) && (_u32PCR_STC_Diff[_u32EngId][u32CurrPCRId] > _u32PCR_STC_Diff[_u32EngId][u32PrevPCRId])) ||
                                        ((!_bPositiveDiff[_u32EngId][0]) && (_u32PCR_STC_Diff[_u32EngId][u32CurrPCRId] < _u32PCR_STC_Diff[_u32EngId][u32PrevPCRId])))
                                    {
                                        //1. Both positive, curr diff > prev diff.
                                        //2. Both negative, curr diff < prev diff. => Increase 27M
                                        bIncrease = TRUE;

                                    }
                                    else
                                    {
                                        //1. Both positive, curr diff < prev diff.
                                        //2. Both negative, curr diff > prev diff. => Decrease 27M
                                        bIncrease = FALSE;
                                    }
                                    if (_u32PreTick[_u32EngId] == STD_TICK )
                                    {
                                        u32Temp1 = (u32Temp1*STD_TICK)/u32Temp;
                                        _u32PreTick[_u32EngId] = ((bIncrease)?(_u32PreTick[_u32EngId] + u32Temp1):(_u32PreTick[_u32EngId] - u32Temp1));
                                    }
                                    else
                                    {
                                        u32Temp1 = (u32Temp1*_u32PreTick[_u32EngId])/u32Temp;
                                        _u32PreTick[_u32EngId] = ((bIncrease)?(_u32PreTick[_u32EngId] + u32Temp1):(_u32PreTick[_u32EngId] - u32Temp1));
                                        if (_u32PreTick[_u32EngId] >= STD_TICK)
                                        {
                                            u32Temp1 = _u32PreTick[_u32EngId] - STD_TICK;
                                            bIncrease = TRUE;
                                        }
                                        else
                                        {
                                            u32Temp1 = STD_TICK - _u32PreTick[_u32EngId];
                                            bIncrease = FALSE;
                                        }
                                    }
                                    _u32Synthesizer[_u32EngId] = _GetSynthesizer(u32Temp1, bIncrease);
                                    // REG_SYNTH(_u32EngId) = _u32Synthesizer[_u32EngId];
                                    REG_SYNTH()= _u32Synthesizer[_u32EngId];
                                    REG_SYNTH_ENABLE();
                                    #if SYNC_DEBUG
                                    *(U32*)((&dbg_test0)) = REG_SYNTH(0);
                                    if (bIncrease)
                                    {
                                        *(U32*)((&dbg_test0)+1) = 1;
                                    }
                                    else
                                    {
                                        *(U32*)((&dbg_test0)+1) = 2;
                                    }
                                    *(U32*)((&dbg_test0)+2) = 0;
                                    *(U32*)((&dbg_test0)+3) = 0;
                                    #endif
                                    // _FireInterrupt(TSP_SWINT_CMD_STC_UPD, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                                }
                            }
                            else
                            {
                                //One is positive,other one is negative
                                u32Temp1 = _u32PCR_STC_Diff[_u32EngId][0]+_u32PCR_STC_Diff[_u32EngId][1];
                                if (u32Temp1 > MAX_ADJUST_TICK)
                                {
                                    #if SYNC_DEBUG
                                    *(U32*)((&dbg_test0)) = 4;
                                    *(U32*)((&dbg_test0)+1) = _u32PCR_STC_Diff[_u32EngId][0];
                                    *(U32*)((&dbg_test0)+2) = _u32PCR_STC_Diff[_u32EngId][1];
                                    *(U32*)((&dbg_test0)+3) = 0;
                                    _FireInterrupt(TSP_SWINT_STATUS_DEBUG, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                                    #endif

                                    _u32PCR[_u32EngId][0] = 0;
                                    _u32PCR[_u32EngId][1] = 0;
                                    _u32PCRCount[_u32EngId] = 0;
                                    return TRUE;
                                }

                                if (u32Temp1 > SYNC_THD)
                                {
                                    if (_bPositiveDiff[_u32EngId][u32CurrPCRId])
                                    {
                                        //Curr diff is positive, Increase 27M.
                                        bIncrease = TRUE;
                                    }
                                    else
                                    {
                                        //Curr diff is negative, Decrease 27M.
                                        bIncrease = FALSE;
                                    }

                                    if (_u32PreTick[_u32EngId] == STD_TICK )
                                    {
                                        u32Temp1 = (u32Temp1*STD_TICK)/u32Temp;
                                        _u32PreTick[_u32EngId] = ((bIncrease)?(_u32PreTick[_u32EngId] + u32Temp1):(_u32PreTick[_u32EngId] - u32Temp1));
                                    }
                                    else
                                    {
                                        u32Temp1 = (u32Temp1*_u32PreTick[_u32EngId])/u32Temp;
                                        _u32PreTick[_u32EngId] = ((bIncrease)?(_u32PreTick[_u32EngId] + u32Temp1):(_u32PreTick[_u32EngId] - u32Temp1));
                                        if (_u32PreTick[_u32EngId] >= STD_TICK)
                                        {
                                            u32Temp1 = _u32PreTick[_u32EngId] - STD_TICK;
                                            bIncrease = TRUE;
                                        }
                                        else
                                        {
                                            u32Temp1 = STD_TICK - _u32PreTick[_u32EngId];
                                            bIncrease = FALSE;
                                        }
                                    }
                                    _u32Synthesizer[_u32EngId] = _GetSynthesizer(u32Temp1, bIncrease);
                                    // REG_SYNTH(_u32EngId) = _u32Synthesizer[_u32EngId];
                                    REG_SYNTH()= _u32Synthesizer[_u32EngId];
                                    REG_SYNTH_ENABLE();
                                    #if SYNC_DEBUG
                                    *(U32*)((&dbg_test0)) = REG_SYNTH(0);
                                    if (bIncrease)
                                    {
                                        *(U32*)((&dbg_test0)+1) = 3;
                                    }
                                    else
                                    {
                                        *(U32*)((&dbg_test0)+1) = 4;
                                    }
                                    *(U32*)((&dbg_test0)+2) = 0;
                                    *(U32*)((&dbg_test0)+3) = 0;
                                    #endif //SYNC_DEBUG
                                    // _FireInterrupt(TSP_SWINT_CMD_STC_UPD, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));
                                }
                            }
                        }
                    }
                // *(U32*)((&dbg_test0)-4) = REG_SYNTH(0);
                // *(U32*)((&dbg_test0)-3) = u32PcrBase;
                // *(U32*)((&dbg_test0)-2) = 0;
                // *(U32*)((&dbg_test0)-1) = 0;
#endif
                }
            }

#elif PCR_STC_SYNC2_ENABLE

            // check for PCR reset
            if (REG_R(_prSecFlt->Ctrl) & (TSP_SECFLT_PARAM_PCRRST<<TSP_SECFLT_PARAM_SHFT))
            {
                u8ResetPCR = (STC_RESET_TH + 1);
                REG_W(_prSecFlt->Ctrl, REG_R(_prSecFlt->Ctrl) & ~(TSP_SECFLT_PARAM_PCRRST<<TSP_SECFLT_PARAM_SHFT));
            }

            if(u8ResetPCR >= (STC_RESET_TH))
            {
                if(u8ResetPCR == (STC_RESET_TH + 1))
                {
                    REG_SYNTH()= STC_SYNTH_DEF;
                }

                _rCtrl[0].Pcr.ML = u32PcrBase;
                _rCtrl[0].Pcr.H32 = u32PcrBit32;

                u8ResetPCR = 0;
                u32PrevSTCBase = _rCtrl[0].Pcr.ML;
                u32PrevSynth = REG_SYNTH();
            }

            if((_rCtrl[0].Pcr.ML - u32PrevSTCBase) > STC_DIFF_CHECK)
            {
                U32 u32dif;

                u32PrevSTCBase = _rCtrl[0].Pcr.ML;

                if(u32PrevSTCBase > u32PcrBase)
                {
                    u32dif = u32PrevSTCBase - u32PcrBase;

                    if(u32dif <= STC_DIFF_LO)
                    {
                        u8ResetPCR = 0;
                    }
                    else if(u32dif < STC_DIFF_HI)
                    {
                        u32PrevSynth += (u32dif * STC_ADJ_A);
                        REG_SYNTH() = u32PrevSynth + (u32dif * STC_ADJ_B);
                        u8ResetPCR = 0;
                    }
                    else
                    {
                        ++u8ResetPCR;
                    }
                }
                else
                {
                    u32dif = u32PcrBase - u32PrevSTCBase;

                    if(u32dif <= STC_DIFF_LO)
                    {
                        u8ResetPCR = 0;
                    }
                    else if(u32dif < STC_DIFF_HI)
                    {
                        u32PrevSynth -= (u32dif * STC_ADJ_A);
                        REG_SYNTH() = u32PrevSynth - (u32dif * STC_ADJ_B);
                        u8ResetPCR = 0;
                    }
                    else
                    {
                        ++u8ResetPCR;
                    }
                }
            }

#else

            _rCtrl[0].Pcr.ML = u32PcrBase;
            _rCtrl[0].Pcr.H32 = u32PcrBit32;
#endif // PCR_STC_SYNC_ENABLE

#endif // _CMODEL
        }
    }

    return TRUE;
}

void ResetFlt(U32 eng, U32 fid, U32 type)
{

    if ((eng < TSP_ENGINE_NUM) && (fid < TSP_SECFLT_NUM))
    {
        *((U32*)&(shm_sec_info[eng][fid].w_count)) = 0;

        _rSec[eng].Flt[fid].BufRead = _rSec[eng].Flt[fid].BufWrite =
        _rSec[eng].Flt[fid].BufStart;

        _u32BufPtr[eng][fid] = _rSec[eng].Flt[fid].BufStart;
        _rSec[eng].Flt[fid].BufCur = _u32BufPtr[eng][fid] + 4;
        MEM32(_u32BufPtr[eng][fid]) = 0x0;

        shm_sec_info[eng][fid].r_ptr = _rSec[eng].Flt[fid].BufStart;
        shm_sec_info[eng][fid].buf_size = _rSec[eng].Flt[fid].BufEnd - _rSec[eng].Flt[fid].BufStart;
        shm_sec_info[eng][fid].buf_end = _rSec[eng].Flt[fid].BufEnd;

        _u32DataSize[eng][fid] = 0;

        shm_tsc_ready &=  ~(1 << fid);
        //_not_ver_enable &=  ~(1 << fid);
        //_not_ver_version[fid] = 0xFF;

        if (type != 0xFF)
        {
            _rSec[eng].Flt[fid].Ctrl &= ~TSP_SECFLT_TYPE_MASK;
            _rSec[eng].Flt[fid].Ctrl |= ((type & TSP_SECFLT_TYPE_MASK) << TSP_SECFLT_TYPE_SHFT);
        }
    }
}

void SetNotVersion(U32 fid, U32 ver, BOOL bEnable)
{
    _not_ver_version[fid] = ver;
    _not_ver_enable &=   ~(1 << fid);
    _not_ver_enable |=    ((bEnable?1:0) << fid);
}

void SetFltMapping(U32 eng, U32 secfltid, U32 pidfltid)
{
    if ((eng < TSP_ENGINE_NUM) && (secfltid < TSP_SECFLT_NUM))
    {
        _u8mapsec2pid[eng][secfltid] = pidfltid;
    }
}

void SetPCROfs()
{
    _pcr_ofs = (S32)shm_cmd >> 8;
    u8ResetPCR = (STC_RESET_TH + 1);
}

void ProcCmd(void)
{
    U8* cmd = (U8*)&shm_cmd;

    switch (cmd[3])
    {
        case TSP_CMD_RESET_FLT:
            ResetFlt(cmd[2], cmd[1], cmd[0]);
            shm_cmd = TSP_CMD_CLEAR;
            break;

        case TSP_CMD_SET_NOTVER:
        case TSP_CMD_CLR_NOTVER:
            SetNotVersion(cmd[2], cmd[1], cmd[3] == TSP_CMD_SET_NOTVER);
            shm_cmd = TSP_CMD_CLEAR;
            break;

        case TSP_CMD_MAP_PIDFLT:
            SetFltMapping(cmd[2], cmd[1], cmd[0]);
            shm_cmd = TSP_CMD_CLEAR;
            break;

        case TSP_CMD_SET_PCROFS:
            SetPCROfs();
            shm_cmd = TSP_CMD_CLEAR;
            break;

    }

    //check alive
    if (_prCtrl->MCU_Data1 == 0xAB)
    {
        _prCtrl->MCU_Data1 = 0xCD;
    }
}

//--------------------------------------------------------------------------------------------------
// Description: TSP firmware internal packet process function
// Arguments:   NONE
// Return:      NONE
//--------------------------------------------------------------------------------------------------
void FwTSP_Proc(void)
{
    U32 u32TSC;   // transport_scrambling_control

    DBG_FWTSP_FLAG(DEBUG_FLAG_PROC, 0x1);
    DBG_FWTSP_MSG("TSP Firmware Main Process\r\n");

    while (1)
    {
        // wait for host to set firmware start address
        if (shm_sdr_base == 0xFFFFFFFF) { continue; }

        #if defined(_CMODEL)
        if (!_uFwTsp_Run)
        {
            FWTSP_CMODEL_DELAY(); // Thread delay for CModel performance
            continue;
        }
        #endif

        ProcCmd();

        //[TBD]
        // 1. Process all matched pid filter of a TSP in one time
        // or
        // 2. Process one matched pid filter of a TSP in turns.
        for (_u32EngId = 0; _u32EngId < TSP_ENGINE_NUM; _u32EngId++)
        {
            _prCtrl = &_rCtrl[_u32EngId];

            if (_prCtrl->Pkt_Stat & TSP_PKT_STAT_CACHE_RDY)
            {
                DBG_FWTSP_FLAG(DEBUG_FLAG_PROC, 0x2);
                _prCtrl->Pkt_Stat = 0; // reset DMA status
                _u32SfId = _prCtrl->Pkt_SecFltId;

                // Do section filter packet parsing

                //[HW TBD] supposed hardware will cache the first 16 bytes from
                // packet arrival *AUTOMATICALLY*
                //_prTSP->Pkt_CacheIdx = 0;
                //_CM_Reg_Write((U32)&_prTSP->Pkt_CacheIdx);

                _u32TS_PktSyncFlag = ((_prCtrl->Pkt_CacheW0 >> 0 ) & 0xff);
                _u32TS_PktEIFlag   = ((_prCtrl->Pkt_CacheW0 >> 15) & 0x01);  // TSH: error indicator
                _u32TS_PktPUSIFlag = ((_prCtrl->Pkt_CacheW0 >> 14) & 0x01);  // TSH: payload unit start indicator
                _u32TS_PktPIDFlag  = ((_prCtrl->Pkt_CacheW0 & 0x1f00) |
                                     ((_prCtrl->Pkt_CacheW0 >> 16) & 0xff));
                 u32TSC            = ((_prCtrl->Pkt_CacheW0 >> 30) & 0x03);  // TSH: transport_scramble_control
                _u32TS_PktAFFlag   = ((_prCtrl->Pkt_CacheW0 >> 28) & 0x03);  // TSH: adaptation field
                _u32TS_PktCCFlag   = ((_prCtrl->Pkt_CacheW0 >> 24) & 0x0f);  // TSH: cont. count

                // check sync byte
                if (_u32TS_PktSyncFlag != 0x47)
                {
                    goto do_next;
                }

                // check the transport_error_indicator
                if (_u32TS_PktEIFlag)
                {
                    goto do_next;
                }

                // check PID. after channel change, there is data if previous channel left in section ping-pong buffer
                if ((_u8mapsec2pid[_u32EngId][_u32SfId] < TSP_PIDFLT_NUM) &&
                    (_rPid[_u32EngId].Flt[_u8mapsec2pid[_u32EngId][_u32SfId] ] & 0x1FFF) != _u32TS_PktPIDFlag)
                {
                    goto do_next;
                }

                #if FWTSP_DEBUG_ENABLE
                if (_u32SfId >= TSP_SECFLT_NUM)
                {
                    DBG_FWTSP_FLAG(DEBUG_FLAG_ERROR, 1);
                    //[NOTE]
                    // Firmware cannot check TSP_PIDFLT_SECFLT_MASK without PidFltId
                }
                else
                #endif
                {
                    _prSecFlt = &_rSec[_u32EngId].Flt[_u32SfId];
                    _prSecBuf= _prSecFlt;
                    
                    if (_Proc_PreCheck())
                    {
                        // update tsc status to shared memory
                        if ((((shm_tsc_ready >> _u32SfId)&1) == 0) &
                            _u32TS_PktPUSIFlag)
                        {
                            shm_tsc_non_zero &=   ~ (1 << _u32SfId);
                            shm_tsc_non_zero |= ((u32TSC?1:0)  << _u32SfId);
                            shm_tsc_ready |= (1 << _u32SfId);
                        }

                        if ((u32TSC != 0) &&
                            ((_prSecFlt->Ctrl & TSP_SECFLT_TYPE_MASK) !=
                            (TSP_SECFLT_TYPE_PKT<<TSP_SECFLT_TYPE_SHFT)))
                        {
                            goto do_next;
                        }

                        switch (_prSecFlt->Ctrl & TSP_SECFLT_TYPE_MASK)
                        {
                        case (TSP_SECFLT_TYPE_SEC<<TSP_SECFLT_TYPE_SHFT):
                            DBG_FWTSP_FLAG(DEBUG_FLAG_PROC, DEBUG_FLAG_SEC);
                            if (_GetSecFollowPackets())
                            {
                                _GetSecFirstPacket();
                            }
                            break;

                        case (TSP_SECFLT_TYPE_PES<<TSP_SECFLT_TYPE_SHFT):
                            DBG_FWTSP_FLAG(DEBUG_FLAG_PROC, DEBUG_FLAG_PES);
                            if (_u32TS_PktPUSIFlag)
                            {
                                _GetPESFirstPacket(PES_OPT_NONE);
                            }
                            else
                            {
                                _GetPESFollowPackets(PES_OPT_NONE);
                            }
                            break;

                        case (TSP_SECFLT_TYPE_PKT<<TSP_SECFLT_TYPE_SHFT):
                            DBG_FWTSP_FLAG(DEBUG_FLAG_PROC, DEBUG_FLAG_PKT);
                            _GetTSPacket();
                            break;

                        case (TSP_SECFLT_TYPE_PCR<<TSP_SECFLT_TYPE_SHFT):
                            DBG_FWTSP_FLAG(DEBUG_FLAG_PROC, DEBUG_FLAG_AF);
                            _GetAdaptField();
                            break;

                        #if TTX_PACKET_SUPPORT
                        case (TSP_SECFLT_TYPE_TTX<<TSP_SECFLT_TYPE_SHFT):
                            DBG_FWTSP_FLAG(DEBUG_FLAG_PROC, DEBUG_FLAG_TTX);
                            if (_u32TS_PktPUSIFlag)
                            {
                                _GetPESFirstPacket(PES_OPT_TTX);
                            }
                            else
                            {
                                _GetPESFollowPackets(PES_OPT_TTX);
                            }
                            break;
                        #endif // TTX_PACKET_SUPPORT
                        default:
                            // Unknown filter type
                            DBG_FWTSP_FLAG(DEBUG_FLAG_ERROR, 2);
                            break;
                        }
                    }
                }

                #if OVERRUN_CHECK_ENABLE
                //[TODO]
                // Is it possible to tell driver to info *ALL* valid callback that
                // overrun happens? Becuase overrun is different than overflow.
                // Overflow is easy to tell which filter, but overrun is whole
                // TSP status.
                // If overrun is allowed and what's to be recoverable, then driver
                // should disable *ALL* filter and let user to clear this situation.
                if ((_prCtrl->Pkt_Stat & TSP_PKT_STAT_OVRUN) || 
                    (_prCtrl->Pkt_Stat & TSP_PKT_STAT_OVRUN2))
                {
                    //_prCtrl->Pkt_DmaCtrl |= TSP_PKT_DMACTRL_CLEAR;
                    //_prCtrl->Pkt_DmaCtrl = 0;
                    //_FireInterrupt(TSP_SWINT_STATUS_PKT_OVRUN, (_u32EngId<<TSP_SWINT_INFO_ENG_SHFT));                   
                    _prCtrl->MCU_Data1 = 0xE4;
                }
                #endif // OVERRUN_CHECK_ENABLE

do_next:

                //[HW TBD] should re-cache first 16 bytes again when matched section filter
                // is processed to prepare processing next matched section filter?
                //_prTSP->Pkt_CacheIdx = 0;
                //_CM_Reg_Write((U32)&_prTSP->Pkt_CacheIdx);

                _prCtrl->Pkt_Stat = 0; // reset DMA status

                _prCtrl->Pkt_DmaCtrl |= TSP_PKT_DMACTRL_ABORT;
                _prCtrl->Pkt_DmaCtrl &= ~(TSP_PKT_DMACTRL_ABORT);
            }
            #if DEFER_FIRE_INT
            if (_u8DeferIntWrite[_u32EngId] != _u8DeferIntRead[_u32EngId])
            {
                U32 u32Info = _u32DeferInt[_u32EngId][_u8DeferIntRead[_u32EngId]];
                //U32 u32HK = _u8DeferIntHK[_u32EngId][_u8DeferIntRead[_u32EngId]];

                if (_FireInterrupt((u32Info & TSP_SWINT_STATUS_CMD_MASK)>> TSP_SWINT_STATUS_CMD_SHFT, u32Info))
                {
                    U32 u32SfId = (u32Info & TSP_SWINT_INFO_SEC_MASK) >> TSP_SWINT_INFO_SEC_SHFT;
                    _u32DeferInt[_u32EngId][_u8DeferIntRead[_u32EngId]] = 0;
                    _u32DeferIntStatus[_u32EngId] &= ~((1<< u32SfId));
                    _u8DeferIntRead[_u32EngId]++;
                    _u8DeferIntRead[_u32EngId]&= 0x1F; // 32 Filter
                }
            }
            #endif
        }
    }
}


//--------------------------------------------------------------------------------------------------
// Description: TSP firmware intialization function
// Arguments:   NONE
// Return:      NONE
//--------------------------------------------------------------------------------------------------
void FwTSP_Init(void)
{
    int i, j;

    DBG_FWTSP_FLAG(0x1, 0x1);

    // Hardware & Interrupt Initialization
#if UART_ENABLE
    uart_init();
#endif
    DBG_FWTSP_MSG("TSP Firmware Init\r\n");

    // Variable Initialization

    #if FWTSP_INFO_ENABLE
    _u32RdyCnt = 0;
    #endif

    // Hardware Initial
    #if defined(_CMODEL)
    // Let C++ module set _rFwTSP_Reg and _uFwTSP_Run to start firmware.
    _rCtrl = (REG_Ctrl*)_rFwTspCtrl_Reg;
    _rSec = (REG_Sec*)_rFwTspSec_Reg;
    #endif // _CMODEL

/*
    if (_rCtrl[0].MCU_MSG)
    {
        // Firmware parameter
    }
    _rCtrl[0].MCU_MSG = 0;
*/

    // clear pkt cache status after reset

    // Software Initial

    for (i = 0; i < TSP_ENGINE_NUM; i++)
    {
#if PCR_STC_SYNC_ENABLE
        REG_SYNTH()= DEF_STC_SYNTH;
        REG_SYNTH_ENABLE();
#endif // #ifdef PCR_STC_SYNC_ENABLE

        _rCtrl[i].Pkt_Stat = 0;
        _rCtrl[i].Pkt_SecFltId = 0xFF;
        _rCtrl[i].Pkt_DmaCtrl |= TSP_PKT_DMACTRL_ABORT;

        _not_ver_enable = 0;

#if DEFER_FIRE_INT
        _u8DeferIntRead[i] = 0;
        _u8DeferIntWrite[i] = 0;
        _u32DeferIntStatus[i] = 0;
#endif // #ifdef DEFER_FIRE_INT

        for (j = 0; j < TSP_SECFLT_NUM; j++)
        {
#if SEC_PTN_BREAK_SUPPORT
            _u32PtnByte[i][j] = 0;
#endif

#if SEC_HDR_BREAK_SUPPORT
            _u32SecLenData[i][j] = 0;
            _u32SecByte[i][j] = 0;
#endif

            _u32PktContiCnt[i][j] = 0;
            _u32FltDataCnt[i][j] = 0;

            _u32DataSize[i][j] = 0;

            _u8mapsec2pid[i][j] = 0xFF;

#if DEFER_FIRE_INT
            _u32DeferInt[i][j] = 0;
#endif // #if DEFER_FIRE_INT

        }
    }

#if SEC_PTN_BREAK_SUPPORT
    for (i= 0; i< (TSP_FILTER_DEPTH<< 1)/sizeof(U32); i++){
        _u32PtnMask[i]=                 0xffffffff;
    }
#endif
    DBG_FWTSP_FLAG(DEBUG_FLAG_INIT, DEBUG_FLAG_PROC);
    
    shm_cmd = 0;

    bDbgHang = FALSE;

}

// --------------------------------------------------------------------------------------------------
// Description: TSP firmware main function
// Arguments:   NONE
// Return:      NONE
//--------------------------------------------------------------------------------------------------
#if defined(_CMODEL)
unsigned __stdcall FwTSP_Thread(void *param)
#else
int main(void)
#endif
{
/*
    *(volatile unsigned long *)0x2100C8 = ((*(volatile unsigned char*)0x90000002) << 8) |
                                           (*(volatile unsigned char*)0x90000005);
*/
//    uart_init();

    // DBG_FWTSP_FLAG(DEBUG_FLAG_NULL, DEBUG_FLAG_NULL);
    FwTSP_Init();
    FwTSP_Proc();

    return 0;
}


