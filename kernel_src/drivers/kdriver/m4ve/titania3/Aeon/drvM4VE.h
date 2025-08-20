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
/// @brief M4VE driver
/// @author MStar Semiconductor Inc.
///
/// M4VE is the Mpeg4 ASP encoder, used for PVR function.
///
/// Features:
/// - Mpeg 4 ASP ( part 2 ) encoder.
///
////////////////////////////////////////////////////////////////////////////////


#ifndef _DRVM4VE_H_
#define _DRVM4VE_H_

#ifdef DRVM4VE_TEST
#define M4VE_DEBUG 1
#endif

#include "M4VE_chip.h"

#ifdef _M4VE_BIG2_
//#define _BATCHRUN_
//#define _COMP_INRUN_ //run batch via HIF
#define _NO_FILESYSTEM_
#define _NO_BVOP_
#define _FIX_H263_GOBNUM_
//#define _FPGA_
//#define _YUVGEN_
#define _ENABLE_ISR_
//#define _EN_CRC_
//#define _EN_HWSGC_CRC_
//#define _TRACE32_CMM_
#endif

#ifdef _M4VE_T3_
#define _NO_FILESYSTEM_
#define _NO_BVOP_
#define BASE_ADR_HIGH_BITS  9
#define BASE_ADR_HIGH_MASK	0x1ff
//#define _YUVGEN_
#define _ENABLE_ISR_
//#define _EN_CRC_
//#define _EN_HWSGC_CRC_
#else
#define BASE_ADR_HIGH_BITS  8
#define BASE_ADR_HIGH_MASK	0x0ff
#endif

#if defined(_AEON_PLATFORM_) || defined(_M4VE_OLYMPUS_)
#define _NO_FILESYSTEM_
#endif

#ifdef _AEON_PLATFORM_
#if defined(_M4VE_T3_)
#include "DataType.h"
#else
#include "MsTypes.h"
#endif
#else //if !defined(_ARM_PLATFORM_)
#include "udma_share.h"
#endif
#if defined(_MIPS_PLATFORM_) && defined(_M4VE_BIG2_)
#include <sys/bsdtypes.h>
#include "shellcfg.h"   //for diag_printf
#endif
#include "Mscommon.h"

#define BITSBUF_SIZE    (1024*1024*2)
#define YUVBUF_SIZE        ((1280*1024*3)>>1)

#ifdef _M4VE_BIG2_
#define MIU_SHIFT     2//3
#define MIU_SIZE     4//8
#else
#define MIU_SHIFT     3
#define MIU_SIZE     8
#endif
//#ifndef _AEON_PLATFORM_
//for multiple output bitstream buffer
#ifdef _MUL_OUTBITS_BUF_
#define OutBsBfNum 3
#else
#define OutBsBfNum 1
#endif

typedef enum {
    WAIT_FRAME_DONE = 0,
    WAIT_INPUT_FRAME = 1,
    WRITE_M4VE_REG = 2,
    WAIT_AVAIL_BITSBUF = 3,
} M4VE_STAT;

typedef struct {
    unsigned long   start_addr;
    unsigned long   end_addr;
    long            used_size;
} OutBitSBUF ;
//#endif

typedef union _ME_REG_
{
    struct {
        unsigned int PMV_init_guess_enable   : 1;
        unsigned int large_dimond_enable     : 1;
        unsigned int search_4block_enable    : 1;
        unsigned int half_pixel_16x16_enable : 1;
        unsigned int half_pixel_8x8_enable   : 1;
        unsigned int search_4block_range     : 1; // 0: -2 ~ 2, 1: -1 ~ 1
    };
    unsigned int reg;
} ME_REG_SET;

#ifdef _WIN32//_BCB_PLATFORM_
#define M4VE_SCRIPT_OUT 1
//#define _IPB_FRAMEQP_
#elif defined(_TRACE32_CMM_)
#define M4VE_SCRIPT_OUT 0
#else
#define M4VE_SCRIPT_OUT 0
#endif

#define SHORT_VIDEO_START_MARKER         0x20
#define SHORT_VIDEO_START_MARKER_LENGTH  22
#define SHORT_VIDEO_END_MARKER           0x3F

#define M4VE_DBG_NONE	(0x0000)
#define M4VE_DBG_REG	(0x0001)
#define M4VE_DBG_CFG 	(M4VE_DBG_REG << 1)
#define M4VE_DBG_SIGNAL (M4VE_DBG_CFG << 1)
#define M4VE_DBG_FB 	(M4VE_DBG_SIGNAL << 1)
#define M4VE_DBG_GWIN 	(M4VE_DBG_FB << 1)
#define M4VE_DBG_FIFO 	(M4VE_DBG_GWIN << 1)
#define M4VE_DBG_INTR 	(M4VE_DBG_FIFO << 1)
#define M4VE_DBG_MISC 	(M4VE_DBG_INTR << 1)
#define M4VE_DBG_MAX 	(M4VE_DBG_MISC<< 1)
#define M4VE_DBG_ALL 	(M4VE_DBG_MAX - 1)

#define M4VE_DBG_FLAG   (M4VE_DBG_ALL)


#define CHECK_M4VE_REG_SET_VALUE 0
#define E_M4VE_RET_OK	0


#if (M4VE_SCRIPT_OUT == 1)
    #define M4VE_SCRIPT(x)	(x)
#else
    #define M4VE_SCRIPT(x)  {}
#endif

// #define ASSERT(expr) ((expr) || printf("Assertion fail: %s %d\n", __FILE__, (int)__LINE__))
// #define ASSERT(expr) do {if(! (expr)) printf("Assertion fail: %s %d\n", __FILE__, (int)__LINE__) ; } while (0)


#if 0
typedef unsigned char  BIT;
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned long  U32;
typedef signed char    S8;
typedef signed short   S16;
typedef signed long    S32;
#endif

#define I_VOP		0		/* vop coding modes */
#define P_VOP		1
#define B_VOP       2       // assumed B VOP always without reconstructed
#define B_VOP_NREC	2       // B VOP without reconstructed
#define B_VOP_REC   3       // B VOP with reconstructed
#define RESYNC_MARKER 1

#if defined(_AEON_PLATFORM_) && defined(_M4VE_TRITON_)
#define printf      UTL_printf
#endif
#if defined(_MIPS_PLATFORM_) && defined(_M4VE_BIG2_)
#define printf(fmt, args...)      diag_printf(fmt, ##args)
#elif defined(_MIPS_PLATFORM_) && defined(_M4VE_T3_)
#define printf(fmt, args...)      printk(fmt, ##args)
#endif

#define M4VE_DEBUG_ENABLE 0
#if (M4VE_DEBUG_ENABLE == 1)
#define M4VE_DEBUG  printf
#elif (M4VE_DEBUG_ENABLE == 0)
#ifdef __GNUC__
#define M4VE_DEBUG(format, ...)
#else
#define M4VE_DEBUG 1 ? 0 :
#endif
#endif

#ifdef _NO_FILESYSTEM_
#define fprintf(format, ...)
#define FILE int
#endif
/* M4VE core uses 16 bit register. */

//1  #define REG(addr) (*(volatile U32 *)(addr))
#if defined(_AEON_PLATFORM_) && defined(_M4VE_T3_)
#define RIU_BASE 0xA0000000
#define REG_BANK_M4VE    0x111000
#define __M4VE_REG(reg)		(*(volatile U16 *) ( RIU_BASE + (REG_BANK_M4VE + reg*2)*2) )
#elif defined(_AEON_PLATFORM_)
#define RIU_BASE 0xA0000000
#define REG_BANK_M4VE    0x1200
#define __M4VE_REG(reg)		(*(volatile U16 *) ( RIU_BASE + (REG_BANK_M4VE + reg)*4) )
//extern U8 FSwrite_ready;
#elif defined(_M4VE_OLYMPUS_) && defined(_ARM_PLATFORM_)
#define RIU_BASE 0xA0000000
#define REG_BANK_M4VE	0x6e00 //0x6a00
#define __M4VE_REG(reg)        (*(volatile U16 *) ( RIU_BASE + REG_BANK_M4VE + (reg)*4) )
#elif defined(_M4VE_BIG2_) && defined(_MIPS_PLATFORM_)
#define RIU_BASE 0xBF834000
#define REG_BANK_M4VE    0 //0xd000
#define __M4VE_REG(reg)        (*(volatile U16 *) ( RIU_BASE + REG_BANK_M4VE + (reg)*4) )
#elif defined(_M4VE_T3_) && defined(_MIPS_PLATFORM_)
#define RIU_BASE 0xBF200000 //CH4
#define REG_BANK_M4VE    0x8800*4//0 //0xd000
#define __M4VE_REG(reg)		(*(volatile U16 *) ( RIU_BASE + (REG_BANK_M4VE + reg*4)) )
#elif defined(_HIF_) && defined(_M4VE_BIG2_)
#define RIU_BASE        0xA0000000
#define REG_BANK_M4VE    0xd000
#define __M4VE_REG(reg)        FPGA_RIURead16(REG_BANK_M4VE+reg, &val_64)
#elif defined(_FPGA_)/*defined(_WRITE_UDMA_)*/
#define RIU_BASE        0xA0000000
#define REG_BANK_M4VE    0xa80
#define __M4VE_REG(reg)        FPGA_RIURead16(REG_BANK_M4VE+reg, &val_64)

#else //if defined(_WIN32)//defined(_BCB_PLATFORM_)
#define __M4VE_REG(reg)		REG_BANK_M4VE[reg]
#endif
#define M4VE_REG_EN						__M4VE_REG(0x00)
#define M4VE_REG_SRC_VOP_Y_ADR_LO		__M4VE_REG(0x0c)
#define M4VE_REG_SRC_VOP_Y_ADR_HI		__M4VE_REG(0x0d)
#define M4VE_REG_SRC_VOP_C_ADR_LO		__M4VE_REG(0x0e)
#define M4VE_REG_SRC_VOP_C_ADR_HI		__M4VE_REG(0x0f)

#define M4VE_REG_REF_VOP_Y_ADR_LO		__M4VE_REG(0x06)
#define M4VE_REG_REF_VOP_Y_ADR_HI		__M4VE_REG(0x07)
#define M4VE_REG_REF_VOP_C_ADR_LO		__M4VE_REG(0x08)
#define M4VE_REG_REF_VOP_C_ADR_HI		__M4VE_REG(0x09)

#define M4VE_REG_REC_VOP_Y_ADR_LO		__M4VE_REG(0x10)
#define M4VE_REG_REC_VOP_Y_ADR_HI		__M4VE_REG(0x11)
#define M4VE_REG_REC_VOP_C_ADR_LO		__M4VE_REG(0x12)
#define M4VE_REG_REC_VOP_C_ADR_HI		__M4VE_REG(0x13)

#define M4VE_REG_QUANT					__M4VE_REG(0x2a)

#define M4VE_REG_SGC_PUT_HEADER_ADR			__M4VE_REG(0x28)
#define M4VE_REG_SGC_CPU					__M4VE_REG(0x29)

struct _vop_time_inc {
#ifdef _MIPS_PLATFORM_
    int framerate;
#else
	double framerate;
#endif
	int vop_time_inc_resolution;
	int vop_time_inc;
};

extern int bitsbuf_used[OutBsBfNum];
extern char slash;
//#define CVBR_RATE_CONTROL  don't define with B-frame
//#ifdef _AEON_PLATFORM_
void MApp_M4VE_TaskEntry(void);
//#else
int MHal_M4VE_set_outbitsbuf(void); //for enc_continue
void MHal_M4VE_SetBitrate(U32 bitrate);
void MHal_M4VE_SetQscale(U8 quant, U32 fixQP);
void MHal_M4VE_SetCodec(U32 codec_type);
void MHal_M4VE_SetFramerate(U32 codec_type);
#ifdef _ENABLE_AVIMUX_
//void msAPI_M4VE_Init(int width, int height, unsigned char **outbsbf_start, unsigned char **outbsbf_end);
void msAPI_M4VE_Init(int width, int height, int BitRate, double FrameRate, unsigned char **outbsbf_start, unsigned char **outbsbf_end);
#elif 1//defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
void msAPI_M4VE_Init(int width, int height, U32 gbuf_start, U32 gbuf_size);
#else
void msAPI_M4VE_Init(int width, int height);
#endif
#ifdef _PARA_FILEOUT_
int msAPI_M4VE_Finish(U32 *bitsAddr, U32 *bitsSize);
#else
int msAPI_M4VE_Finish(void);
#endif

#if defined(_DIP_INPUT_) || defined(_FRAMEPTR_IN_)
#ifdef _PARA_FILEOUT_
int msAPI_M4VE_EnOneFrm(U32 inframe_ind, U32 inputframe_addr, U32 *encframe_ind/*U32 *bitsAddr, U32 *bitsSize,*/
#ifndef _NO_WAIT_FRAMEDONE_
                        , U8 *encode_VOP_type
#endif
                        );
#else
int msAPI_M4VE_EnOneFrm(U32 inframe_ind, U32 inputframe_addr, U32 *encframe_ind);
#endif
#else
int msAPI_M4VE_EnOneFrm(U32 inframe_ind, U8 *input_fname, U8 *realname, U8 *tmpname);
#endif
void M4VE_Test_AP(U32 frmNum, char *input_fname, char *realname, char *tmpname
#ifdef _COMP_INRUN_
                  , FILE *fp_golden
#endif
                  );

#ifdef _NO_WAIT_FRAMEDONE_
int M4VE_getframe(U32 inframe_ind, U32 *encframe_ind, U32 *bitsAddr, U32 *bitsSize, U8 *encode_VOP_type);
#endif

int MHal_M4VE_EncSkipVOP(void);
//int MDrv_M4VE_GenSkipVOPHeader(U8 voptype, U8 *pBuf);
//#endif
#endif
