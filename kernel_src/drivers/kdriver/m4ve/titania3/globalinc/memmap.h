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


//                       width  height  MBs
//h.264 level4.1, 2Kx1K	 2048	1024	8192
#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#if defined(_MIPS_PLATFORM_) && defined(_M4VE_BIG2_)
#include <sys/bsdtypes.h>
#elif defined(_MIPS_PLATFORM_) && defined(_M4VE_T3_)
//#include "m"
#endif
#include "Mscommon.h"

//#define _MUL_OUTBITS_BUF_
#ifdef _MUL_OUTBITS_BUF_
#if defined(VIDEO_REC_D1_EN)    //Big2, @elaine
#define IBufSize       1024*32//1024*1//1024*64//1024*2
#else
#define IBufSize       1024*96//1024*1//1024*2
#endif
#define GBufSize       (1280*1024*3/2*4 + IBufSize*3) //ken: 1cur, 1bak, 1for, 1rec and 3 bitstream_buf  //(20*1024*1024)
#else
#define IBufSize       1024*512*1//1024*3//(1350*384)
#define GBufSize       (720*480*3/2*4 + IBufSize) //ken: 1cur, 1bak, 1for, 1rec and 3 bitstream_buf  //(20*1024*1024)
#endif
#define FrameSize      (3145728) //1920x1080*3/2 ??

#define SwGBufSize     (5*1024*1024)

//MIU, memory layout
#define DRAM_BIN_SIZE   0//(1024*1024)
#define DRAM_IBUF_SIZE  (IBufSize<<1)

#if defined(_AEON_PLATFORM_) && defined(_M4VE_T3_)
#define DRAM_BASE       0xae3000//0xae3000 //0xafe000//we use DIP buffer temporally //0
#define MEM_LOGMAPPHY   0x80000000
#elif defined(_MIPS_PLATFORM_) && defined(_M4VE_T3_)
#define DRAM_BASE       0x08100000//0xae3000 //0xafe000//we use DIP buffer temporally //0
#define MEM_LOGMAPPHY   0xA0000000//0xB0000000 A:Mstar board MIU0, B:MIU1//use Memory after 16MB  A:put in cache, 8: no in cache
#elif defined(_AEON_PLATFORM_)
#define DRAM_BASE       0xae3000//0xae3000 //0xafe000//we use DIP buffer temporally //0
#define MEM_LOGMAPPHY   0x80000000
#elif defined(_TRACE32_CMM_)
#define DRAM_BASE       0x80000//0xA0080000//0
#define MEM_LOGMAPPHY   0xA0000000
#elif defined(_M4VE_BIG2_) && defined(_HIF_)
#define DRAM_BASE       0x130000 //0x10080000//0xB0080000//0
                        //this should be start from 0, but i only tried start from 0x20000
#define MEM_LOGMAPPHY   0x10080000
#elif defined(_M4VE_BIG2_) && defined(_MIPS_PLATFORM_)
#ifdef _USE_PSRAM_
//#define DRAM_BASE       0x0000
#define DRAM_BASE       0x130000
#define MEM_LOGMAPPHY   0xA4080000
#define BANK_SEL        1
#else
#define DRAM_BASE       0x130000
#define MEM_LOGMAPPHY   0xB0080000
#define BANK_SEL        4
#endif
#elif defined(_M4VE_OLYMPUS_)
#define DRAM_BASE       0x0100a000 //0x10080000 //0xB0080000 //0
#define MEM_LOGMAPPHY   0x08000000 //0x10080000
#else
#define DRAM_BASE       0x20000//0x10080000//0xB0080000//0
#define MEM_LOGMAPPHY   0//0x10080000
#endif

#define DRAM_IBUF_BASE  DRAM_BASE+DRAM_BIN_SIZE
#define DRAM_GBUF_BASE  DRAM_IBUF_BASE//+DRAM_IBUF_SIZE

//#define AEONMEM_LOGMAPPHY   0x80000000
//RIU
//#define RIU_BASE        0xA0000000

//for current FPGA env
#define REGMAP_BASE     0x00001200

//extern u8 input_buf[IBufSize*2];
#if !defined(_AEON_PLATFORM_) && !defined(_ARM_PLATFORM_)
extern U8 global_buf[GBufSize];
extern U8* global_buf_ptr;
#endif
extern U32 global_buf_phy;
extern U32 global_buf_size;

#ifdef _FPGA_
extern U8 outframe[FrameSize]; //2048x1024x1.5
#endif
extern U32 outframe_size;
extern U32 FILE_EOF;
extern U32 picDecodeNumber; /* decoded picture ID */

#endif
