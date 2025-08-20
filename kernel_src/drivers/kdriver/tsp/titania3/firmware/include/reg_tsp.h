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
//  File name: regTSP.h
//  Description: Transport Stream Processor (TSP) Register Definition
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _TSP_REG_AEON_H_
#define _TSP_REG_AEON_H_


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
// #define TS_PACKET_SIZE              188


//--------------------------------------------------------------------------------------------------
//  Compliation Option
//--------------------------------------------------------------------------------------------------

//[CMODEL][FWTSP]
// When enable, interrupt will not lost, CModel will block next packet
// and FwTSP will block until interrupt status is clear by MIPS.
// (For firmware and cmodel only)
#define TSP_DBG_SAFE_MODE_ENABLE    0


//-------------------------------------------------------------------------------------------------
//  Harware Capability
//-------------------------------------------------------------------------------------------------
#define TSP_ENGINE_NUM              1
#define TSP_PIDFLT_NUM              32   // REG_PID_FLT_0~1F           'h00210000 ~ 'h0021007c
#define TSP_PIDFLT1_NUM              8   // REG_PID1_FLT_0~7           'h00210080 ~ 'h0021009c, it's for PVR (while for section filtering, only PID1 0~7 can be used)
#define TSP_PIDFLT_NUM_ALL          (TSP_PIDFLT_NUM+ TSP_PIDFLT1_NUM)
#define TSP_SECFLT_NUM              TSP_PIDFLT_NUM
#define TSP_FILTER_DEPTH            16

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

// Preprocessor warning notification
#if !defined(TSP_CPU_REGMAP)
#error "Please chose a TSP register mapping";
#endif

// Software
#if defined(TSP_CPU_REGMAP)

    #define REG_PIDFLT_BASE         (0x00210000)
    #define REG_SECFLT_BASE         (0x00211000)

    #define REG_CTRL_BASE           (0x00210200)

    #define REG_SYNTH_VAL           (0x0021024c)

    typedef volatile U32            REG32;
    typedef volatile U32            REG16;
#endif


typedef REG32                       REG_PidFlt;

// PID
#define TSP_PIDFLT_PID_MASK         0x00001FFF
#define TSP_PIDFLT_PID_SHFT         0

// Section filter Id
#define TSP_PIDFLT_SECFLT_MASK      0x001F0000                          // [20:16] secflt id
#define TSP_PIDFLT_SECFLT_SHFT      16
#define TSP_PIDFLT_SECFLT_NULL      0x1F                                // software usage

// AF/Sec/Video/Audio/Audio-second
#define TSP_PIDFLT_OUT_MASK         0x01e02000
#define TSP_PIDFLT_OUT_SECFLT_AF    0x00002000
#define TSP_PIDFLT_OUT_NONE         0x00000000
#define TSP_PIDFLT_OUT_SECFLT       0x00200000
#define TSP_PIDFLT_OUT_VFIFO        0x00400000
#define TSP_PIDFLT_OUT_AFIFO        0x00800000
#define TSP_PIDFLT_OUT_AFIFO2       0x01000000

// note, this bit is only useful for PVR pure pid
// use SEC/VIDEO/AUDIO flag is identical to PVR a certain PID
#define TSP_PIDFLT_PVR_ENABLE       0x04000000

// FW COMMAND
#define TSP_CMD_CLEAR            0x00
#define TSP_CMD_RESET_FLT        0xC0
#define TSP_CMD_SET_NOTVER       0xC1
#define TSP_CMD_CLR_NOTVER       0xC2
#define TSP_CMD_MAP_PIDFLT       0xC3
#define TSP_CMD_SET_PCROFS       0xC4


typedef struct _REG_SecFlt
{
    REG32                           Ctrl;
    // SW flag
    #define TSP_SECFLT_BUF_ENABLE                   0x00000008


    #define TSP_SECFLT_TYPE_MASK                    0x00000007
    #define TSP_SECFLT_TYPE_SHFT                    0
    #define TSP_SECFLT_TYPE_UNK                     0x0
    #define TSP_SECFLT_TYPE_SEC                     0x1
    #define TSP_SECFLT_TYPE_PES                     0x2
    #define TSP_SECFLT_TYPE_PKT                     0x3
    #define TSP_SECFLT_TYPE_PCR                     0x4
    #define TSP_SECFLT_TYPE_TTX                     0x5
    #define TSP_SECFLT_TYPE_OAD                     0x6


    #define TSP_SECFLT_MODE_MASK                    0x000000F0          // software implementation
    #define TSP_SECFLT_MODE_SHFT                    4
    #define TSP_SECFLT_MODE_CONTI                   0x0
    #define TSP_SECFLT_MODE_ONESHOT                 0x1
    #define TSP_SECFLT_MODE_CRCCHK                  0x2

#if 1 // [Titania remove] ??? still keep it ???
    //[NOTE] update section filter
    // It's not suggestion user update section filter control register
    // when filter is enable. There may be race condition. Be careful.
    #define TSP_SECFLT_STATE_MASK                   0x0F000000          // software implementation
    #define TSP_SECFLT_STATE_SHFT                   24
    #define TSP_SECFLT_STATE_OVERFLOW               0x1
    #define TSP_SECFLT_STATE_DISABLE                0x2
    #define TSP_SECFLT_PARAM_MASK                   0xF0000000
    #define TSP_SECFLT_PARAM_SHFT                   28
    #define TSP_SECFLT_PARAM_PCRRST                 0x1
#endif

    REG32                           Match[TSP_FILTER_DEPTH/sizeof(U32)];
    REG32                           Mask[TSP_FILTER_DEPTH/sizeof(U32)];
    REG32                           BufStart;
    // [Titania remove]
    // #define TSP_SECFLT_BUFSTART_MASK                0x07FFFFF8
    REG32                           BufEnd;
    REG32                           BufRead;
    REG32                           BufWrite;
    REG32                           BufCur;

    REG32                           RmnReqCnt;
    #define TSP_SECFLT_REQCNT_MASK                  0xFFFF0000
    #define TSP_SECFLT_REQCNT_SHFT                  16
    #define TSP_SECFLT_RMNCNT_MASK                  0x0000FFFF
    #define TSP_SECFLT_RMNCNT_SHFT                  0


    REG32                           CRC32;
    REG32                           NMatch[TSP_FILTER_DEPTH/sizeof(U32)];
    REG32                           _x50[12]; // (0x210080-0x210050)/4
} REG_SecFlt;


typedef struct _REG_Stc
{
    REG32                           ML;
    REG16                           H32;
} REG_Stc;


typedef struct _REG_Pid
{                                                                       // Index(word)  CPU(byte)       Default
    REG_PidFlt                      Flt[TSP_PIDFLT_NUM_ALL];
} REG_Pid;


typedef struct _REG_Sec
{                                                                       // Index(word)  CPU(byte)       Default
    REG_SecFlt                      Flt[TSP_SECFLT_NUM];
} REG_Sec;


typedef struct _REG_Ctrl
{
                                                                        // Index(word)  CPU(byte)     MIPS(0x1500/2+index)*4
    REG32                           Pkt_CacheIdx;                       // 0x00210200
    REG32                           Pkt_CacheW0;                        // 0x00210204
    REG32                           Pkt_CacheW1;                        // 0x00210208
    REG32                           Pkt_CacheW2;                        // 0x0021020c
    REG32                           Pkt_CacheW3;                        // 0x00210210
    REG16                           Pkt_Stat;                           // 0x00210214
    #define TSP_PKT_STAT_DMA_RDY                    0x0001
    #define TSP_PKT_STAT_DMA_OVFLOW                 0x0002
    #define TSP_PKT_STAT_CACHE_RDY                  0x0100
    #define TSP_PKT_STAT_CMP_MATCH                  0x0200
    #define TSP_PKT_STAT_OVRUN                      0x0400
    #define TSP_PKT_STAT_OVRUN2                     0x0800

    REG32                           Pkt_DmaCtrl;                        // 0x00210218
    #define TSP_PKT_DMACTRL_START                   0x0001
    #define TSP_PKT_DMACTRL_HEAD                    0x0002
    #define TSP_PKT_DMACTRL_TAIL                    0x0004
    #define TSP_PKT_DMACTRL_CRC32                   0x0008
    #define TSP_PKT_DMACTRL_ABORT                   0x0010
    #define TSP_PKT_DMACTRL_MEET                    0x0020
    #define TSP_PKT_DMACTRL_BITREV                  0x0040
    #define TSP_PKT_DMACTRL_CRCRST                  0x0080
    #define TSP_PKT_DMACTRL_CLEAR                   0x8000

    REG16                           Pkt_SecFltId;                       // 0x0021021c
    #define TSP_PKT_FLTID_INVALID                   0x00FF

    REG16                           Pkt_DmaAddr;                        // 0x00210220
    REG16                           Pkt_DmaSize;                        // 0x00210224
    REG32                           _x00210228;                         // 0x00210228 dummy
    REG32                           _x0021022c;                         // 0x0021022c dummy
    REG32                           _x00210230;                         // 0x00210230 dummy
    REG32                           _x00210234;                         // 0x00210234 dummy
    REG32                           _x00210238;                         // 0x00210238 dummy
    REG32                           _x0021023c;                         // 0x0021023c dummy
    REG32                           _x00210240;                         // 0x00210240 dummy

    REG_Stc                         Pcr;                                // 0x00210244
                                                                        // 0x00210248
    // STC adjustment // @FIXME: ignore it at this stage
    REG32                           PcrSynth;                           // 0x0021024c

    REG32                           _x00210250_31c[(0x32c- 0x250)>> 2]; // 0x00210250 ~ 0x00210328 dummy

    REG32                           SwInt_Stat;                         // 0x0021032c
    #define TSP_SWINT_INFO_SEC_MASK                 0x000000FF
    #define TSP_SWINT_INFO_SEC_SHFT                 0
    #define TSP_SWINT_INFO_ENG_MASK                 0x0000FF00
    #define TSP_SWINT_INFO_ENG_SHFT                 8
    #define TSP_SWINT_STATUS_CMD_MASK               0x7FFF0000
    #define TSP_SWINT_STATUS_CMD_SHFT               16
    #define TSP_SWINT_STATUS_SEC_RDY                0x0001
    #define TSP_SWINT_STATUS_REQ_RDY                0x0002
    #define TSP_SWINT_STATUS_BUF_OVFLOW             0x0006
    #define TSP_SWINT_STATUS_SEC_CRCERR             0x0007
    #define TSP_SWINT_STATUS_SEC_ERROR              0x0008
    #define TSP_SWINT_STATUS_PES_ERROR              0x0009
    #define TSP_SWINT_STATUS_SYNC_LOST              0x0010
    #define TSP_SWINT_STATUS_PKT_OVRUN              0x0020
    #define TSP_SWINT_STATUS_DEBUG                  0x0030
    #define TSP_SWINT_CMD_DMA_PAUSE                 0x0100
    #define TSP_SWINT_CMD_DMA_RESUME                0x0200

    #define TSP_SWINT_STATUS_SEC_GROUP              0x000F
    #define TSP_SWINT_STATUS_GROUP                  0x00FF
    #define TSP_SWINT_CMD_GROUP                     0x7F00
    #define TSP_SWINT_CMD_STC_UPD                   0x0400

    #define TSP_SWINT_CTRL_FIRE                     0x80000000

    REG32                           MCU_MSG;                            // 0x00210330

    REG32                           _x00210334_338[(0x33C - 0x334) >> 2];

    REG32                           MCU_MSG2;                           // 0x0021033C

    //#define TSP_MCU_DATA_ALIVE                  TSP_MCU_CMD_ALIVE
    REG32                           _x002103c8_340[(0x3c8 - 0x340)>> 2];    // 0x00210340 ~ 0x002103c4 dummy
    REG32                           MCU_Data1;                              // 0x002103c8
    REG32                           _x002130dc_33c[(0x30dc - 0x3cc)>> 2];   // 0x002103cc ~ 0x002130d8 dummy
    REG32                           SwInt_Stat1;                            // 0x002130dc
} REG_Ctrl;

#if 1  // MaxCC
// Firmware status
#define TSP_FW_STATE_MASK           0xFFFF0000
#define TSP_FW_STATE_LOAD           0x00010000
#define TSP_FW_STATE_ENG_OVRUN      0x00020000
#define TSP_FW_STATE_ENG1_OVRUN     0x00040000                          //[reserved]
#define TSP_FW_STATE_IC_ENABLE      0x01000000
#define TSP_FW_STATE_DC_ENABLE      0x02000000
#define TSP_FW_STATE_IS_ENABLE      0x04000000
#define TSP_FW_STATE_DS_ENABLE      0x08000000
#endif

#endif // #ifndef _TSP_REG_AEON_H_
