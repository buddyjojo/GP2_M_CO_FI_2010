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

#define MDRV_M4VE_C

//#define MEM_OFFSET 0x0
#include "M4VE_chip.h"
#if defined(_MIPS_PLATFORM_)&&defined(_M4VE_T3_)
#include <linux/kernel.h>
#include <linux/string.h>
#include "chip_int.h"
#include <linux/interrupt.h>
#else
#include <string.h>
#endif
#include "yuvgen.h"
#include "drvM4VE.h"
#include "../globalinc/m4vereg.h"
#include "memmap.h"
#include "stdarg.h"
#include "udma_share.h"

#ifdef _AEON_PLATFORM_
#ifdef _M4VE_TRITON_
#include "NoOS.h"
#include "usb_fs.h"
#endif
#ifdef _M4VE_T3_
#include "mdrv_irq.h"
#include "drvISR.h"
#endif
#else
//#include <malloc.h>
#ifdef _WIN32
#include <windows.h>
#include "pthread.h"
#endif
#endif //_AEON_PLATFORM_

#ifdef _EN_CRC_
#include "CheckSum.h"
unsigned int drvM4VE_adler;
#endif

#ifdef _MIPS_PLATFORM_
#ifdef _M4VE_BIG2_
#include "utility.h"
#include "shellcfg.h"
#ifdef _ENABLE_ISR_
#include "cyg/hal/hal_intr.h"
//#define CYGNUM_HAL_INTERRUPT_M4VE   8+16+32+4
#endif //_ENABLE_ISR_

#else
//_M4VE_T3_

#endif //M4VE_BIG2
#endif //_MIPS_PLATFORM_

#ifdef _FPGA_//_WRITE_UDMA_
#include "FPGAapi.h"
#endif
#ifdef _ENABLE_AVIMUX_
#include "avi_muxer.h"
#endif
#if defined(_TRACE32_CMM_)||defined(_DUMP_SCRIPT_)
FILE *dump_script;
#endif
// X_RATE_CONTROL
extern void rc_init(U16 width, U16 height, /*U16 framerate, */ U32 bitrate);
extern U8 rc_vop_before(U16 vop_type);
extern void rc_vop_after(U32 num_bytes);
// CVBR_RATE_CONTROL
extern void cvbr_rc_init(U16 width, U16 height, /*U16 framerate, */ U32 bitrate);
extern U8 cvbr_rc_vop_before(U16 vop_type);
extern void cvbr_rc_vop_after(U32 num_bytes);


extern U32 u32HeaderBitLen ;

extern void MDrv_M4VE_BitstreamPutVOPHeader(U8 voptype, U8 quant, U8 interlace, U32 frameNum,U8 SearchRange
#ifdef _M4VE_BIG2_
                                            , int g_IsShortHeader, int g_nSourceFormat, int width, int height
#endif
                                            );
extern void MDrv_M4VE_BitstreamPutVOLHeader(U16 width, U16 height, U8 interlace, U8 qtype, U32 frame_count);
extern int MDrv_M4VE_GenSkipVOPHeader(U8 voptype, U8 *pBuf);

struct _vop_time_inc time_inc[] = {
    {30.0, 30, 1},
    {29.97, 30000, 1001 },
    {25.0, 25, 1},
    {24.0, 24, 1},
    {23.976, 24000, 1001 }
    // {23.97, 30000, 1251}  divx used.
};

//////////////////////////////////////// Eric org ////////////////////////////////////////////////
static U32 u32FrameCount = 0 ;
static U32 src_inc = 0;
#ifdef _DIP_INPUT_
#define MAX_SRC_INFRAME_NUM     4
typedef struct _INFRAME_STRUCT {
    U32 src_inframe_addr;
    U32 src_inframe_ind;
} INFRAME_STRUCT;
static INFRAME_STRUCT src_inframe_buf[MAX_SRC_INFRAME_NUM];
static int total_inframe_ind=0;
static int inframe_encind = 0;
//U8 FSwrite_ready; //is USB write finished
#endif

extern U32 u32HeaderBitLen ;
U32 yuvBuffer = 0x11820;//0x00400000 ; //yuv: first frame is source, second and third are rec and ref

int modulo_time_base[2];
U32 VopTimeIncResolution = 30;
U32 VopTimeInc = 1;
short fixed_vop_time_increment=1;
/////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_AEON_PLATFORM_) && !defined(_FPGA_) && !defined(_ARM_PLATFORM_) && !defined(_MIPS_PLATFORM_)
//unsigned short RIU_BASE = 0 ;
unsigned short REG_BANK_M4VE[0x100] = {0};
#endif

volatile M4VE_STAT encode_state;

#ifdef _NO_FILESYSTEM_ //defined(_AEON_PLATFORM_) || defined(_M4VE_OLYMPUS_)
int fp_script;
#else
FILE *fp_script;
#endif

M4VE_REG m4ve_reg;
#ifdef _WIN32 //!defined(_AEON_PLATFORM_) && !defined(_MIPS_PLATFORM_)
U8 global_buf[GBufSize];
#endif
#ifndef _NO_BVOP_
MEMMAP_t bak_frame;
#endif
MEMMAP_t cur_frame;
MEMMAP_t for_frame;
MEMMAP_t rec_frame;
MEMMAP_t bitstream_buf[OutBsBfNum];
MEMMAP_t bitstreamM4VE;
//int mybitrate[4]={1500000, 2500000, 4000000, 8000000};
#ifdef _WIN32
char Script_Buffer[2048];
#endif
//#ifdef _MUL_OUTBITS_BUF_
OutBitSBUF outbitsbuf[OutBsBfNum];
int bitsbuf_used[OutBsBfNum];
#ifndef _MUL_OUTBITS_BUF_
static U32 prev_read_baddr=0;
#endif
unsigned long bitsbuf_size = IBufSize; //1024*1;
//move initialize to m4ve_init
int bitsbuf_ind = 0;
short DOUBLE_FRAME = 1; //top or bottom
short fak_start=0;
short fak_end= 4;//30*/;
short MB_in_height_top=4;
short top_data=1;
short large_diamond_limit=245;
short small_diamond_limit=245;
short gL1 = 1;
short gL2 = 2;
#ifdef _M4VE_BIG2_
short isUMV = 0;
#else
short isUMV = 1;
#endif
short is_b_rec = 1;
short interlaced = 0;
short is_sr16 = 1;
int x_rate_control_mb_enable = 0;// //
int switch_br_bnr = 0;
int me_dynamic_enable = 0;
int Width = 176;//352;//64;
int Height = 144;//288;//64;
int is_back_ref =0;
U8 u8IPlength = 0;//10;
U8 u8PBlength = 0;//1;
U8 IPcount = 0;
U8 PBcount = 0;
static U8 VopType = 0;
static U8 m4ve_quant = 0;
static U32 preLen = 0;
static U32 M4VE_Bitrate=1500000;
static U8 is_fixedQP = 0;
#ifdef _MIPS_PLATFORM_
    int framerate = 30; //30;
#else
    double framerate = 30.0;//29.97;
#endif
//
MEMMAP_t m4ve_frame_ptr_tmp;
#if defined(_M4VE_T3_) || defined(_M4VE_BIG2_)
short   hw_crc_mode=0;
extern unsigned int mpool_userspace_base;
#endif
#ifdef _M4VE_BIG2_
int g_IsShortHeader = 0;
int g_H263Resync = 0;
int g_nSourceFormat = 0;
int g_nGobHeightInMb = 0;
int g_nGobunit = 0;
#endif
ME_REG_SET me_reg = {{1, 1, 0, 1, 0, 0}};//{{1,1,1,1,1,0}};

char slash = '/';
//assign bitstream pointer and size to MDrv_M4VE
extern int M4VE_getbits_callback(U32 miuaddr, U32 miupointer, long size
                , int is_frame_done, U8 voptype, U32 Y_start);

int msAPI_M4VE_EncodeVOP_End(MEMMAP_t *tmp_m4ve_frame_ptr
#if defined(_DIP_INPUT_) && defined(_PARA_FILEOUT_)
                             , U32 inframe_ind, U32 *encframe_ind
                             , U32 *bitsAddr, U32 *bitsSize
#endif
                             );

void WriteRegM4VE(U32 u32Address, U16 val)
{
    //U16 temp;
//	printf("wriu 0x%04X 0x%04X\n", REG_BANK_M4VE+u32Address, val);
#if defined(_AEON_PLATFORM_) || defined(_ARM_PLATFORM_) || defined(_WRITE_UDMA_)
    //printf("wriu 0x%04X 0x%04X\n", REG_BANK_M4VE+u32Address, val);
    //temp = __M4VE_REG(u32Address);
    //printf("after write reg 0x%04X 0x%04X\n", REG_BANK_M4VE+u32Address, temp);
    //while(1);
    //M4VE_SCRIPT(fprintf(fp_script, "wriu 0x%04X 0x%04X\n",
    //            REG_BANK_M4VE+u32Address, val));
#else
#ifdef _TRACE32_CMM_
    M4VE_SCRIPT(fprintf(fp_script, "B::D.S D:0x%08X %%LE %%WORD 0x%04X\n",
        0xBF834000+u32Address, val));
#else
    M4VE_SCRIPT(fprintf(fp_script, "wriu 0x%04X 0x%04X\n",
				0xa80+u32Address, val));
#endif
#endif
#if defined(_FPGA_) || defined(_UDMA_) || defined(_HIF_)
    FPGA_RIUWrite16(REG_BANK_M4VE+u32Address, val);
    //UDMA_RIUWrite16(REG_BANK_M4VE+u32Address, val);
#else
    //temp = __M4VE_REG(u32Address);
    //printf("before write reg 0x%04X 0x%04X\n", REG_BANK_M4VE+u32Address, temp);
    __M4VE_REG(u32Address) = val;
#endif
}

void ReadRegM4VE(U32 u32Address, U16 *val)
{
#if defined(_FPGA_) || defined(_UDMA_) || defined(_HIF_)
	FPGA_RIURead16(REG_BANK_M4VE+u32Address, val);
	//UDMA_RIURead16(REG_BANK_M4VE+u32Address, val);
#else
    *val = __M4VE_REG(u32Address);
#endif
    //M4VE_DEBUG(fprintf(fp_script, "M4VE: %x : %x\n",u32Address , __M4VE_REG(u32Address)));
}
#ifdef _FPGA_
U16 reg_m4ve_mask[0x80] = {
    (U16)0x3ffe, (U16)0x3f3f, (U16)0xffff, (U16)0xffff, (U16)0x007f, (U16)0x007f, (U16)0xffff, (U16)0x00ff,
    (U16)0xffff, (U16)0x00ff, (U16)0x00ff, (U16)0x0000, (U16)0xffff, (U16)0x00ff, (U16)0xffff, (U16)0x00ff,
    (U16)0xffff, (U16)0x00ff, (U16)0xffff, (U16)0x00ff, (U16)0x0000, (U16)0x0000, (U16)0x7fff, (U16)0x007f,
    (U16)0x007f, (U16)0x0000, (U16)0xffff, (U16)0x00ff, (U16)0xffff, (U16)0x00ff, (U16)0xffff, (U16)0x00ff,
    (U16)0xffff, (U16)0x00ff, (U16)0x0fff, (U16)0x0000, (U16)0x0000, (U16)0x07bf, (U16)0x0000, (U16)0x0000,
    (U16)0x0000, (U16)0x0000, (U16)0x003f, (U16)0x07ff, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000,
    (U16)0x0000, (U16)0x0001, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000,
    (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000,

    (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000,
    (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000,
    (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0002, (U16)0x0000, (U16)0x0000, (U16)0x0000,
    (U16)0x0000, (U16)0x03ff, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000,
    (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x7fff,
    (U16)0x7fff, (U16)0x7fff, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000,
    (U16)0x7fff, (U16)0x03ff, (U16)0x3f3f, (U16)0x03ff, (U16)0x0fff, (U16)0x0fff, (U16)0x03ff, (U16)0x0000,
    (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000, (U16)0x0000
};
#endif

void MDrv_M4VE_stop(void)
{
    U16 val_00;
/*    U16 u16RegVal = 0x0000;
    U16 u16Rst = 0x0000;

    _MDrv_M4VE_RegGet(M4VE_REG_EN, &u16RegVal);
    u16Rst = u16RegVal & 0x0001;
    u16RegVal &= (~0x0001);
    _MDrv_M4VE_RegSet( M4VE_REG_EN, u16RegVal);
*/
#if 0//def _AEON_PLATFORM_
    M4VE_REG_EN &= (~ 0x0001 ) ;
#else
    ReadRegM4VE(0x00, &val_00);
    val_00 &= (~ 0x0001 ) ;
    WriteRegM4VE(0x00, val_00);
#endif

    return;
}

void MDrv_M4VE_start(void)
{
    //U16 val_00;
    //ReadRegM4VE(0x00, &val_00);
    m4ve_reg.update_mc = 1;
    WriteRegM4VE(0x00, m4ve_reg.reg00);
    m4ve_reg.update_mc = 0;
    //WriteRegM4VE(0x00, m4ve_reg.reg00);
    m4ve_reg.frame_start = 1;
    WriteRegM4VE(0x00, m4ve_reg.reg00);

}

void MDrv_M4VE_reset(void)
{
#if 0//def _AEON_PLATFORM_
    M4VE_REG_EN &= (~ 0x0400) ;
    REG_SET(1,0x0);
    M4VE_REG_EN |= 0x0400 ;
    REG_SET(1,0x0);
#else
    //ReadRegM4VE(0x00, &val_00);
    // sw reset, only reset once to gen zero pluse
    m4ve_reg.soft_reset = 0;
    WriteRegM4VE(0x00, m4ve_reg.reg00);
    m4ve_reg.soft_reset = 1;
    WriteRegM4VE(0x00, m4ve_reg.reg00);
#endif
}

void MDrv_M4VE_Reserved(void)
{
#if 0//def _AEON_PLATFORM_
    __M4VE_REG(0x14) = 0;
    REG_SET(1,0x14);
#else
    m4ve_reg.reg14 = 0;
    WriteRegM4VE(0x14, m4ve_reg.reg14);
#endif
}


U8 MDrv_M4VE_GetVopType(void) // reg 0 bit 8-9
{
#if 0//def _AEON_PLATFORM_
    return ( (__M4VE_REG(0x0) >> 8) & 0x3 );
#else
    return m4ve_reg.frame_type;
#endif
}

void MDrv_M4VE_SetVopType(U8 type) // reg 0 bit 8-9
{
#if 0//def _AEON_PLATFORM_
    U16 tmp = __M4VE_REG(0x0) ;
    __M4VE_REG(0x0) = ( (tmp & (~ 0x300) ) | ( (type & 0x3) << 8 ));
    REG_SET(1,0x0);
#else
    m4ve_reg.frame_type = type&3;
    WriteRegM4VE(0x00, m4ve_reg.reg00);
#endif
}



//
// the following functions should be used for each frame encoding
//

// set the source frame address
void MDrv_M4VE_SetSrcAdr(U32 y, U32 c)
{
    ASSERT( y % 8 == 0 );
    y >>= MIU_SHIFT ;

    ASSERT( c % 8 == 0 );
    c >>= MIU_SHIFT ;

    m4ve_reg.cur_y_base_adr_low = LOWORD(y);
    m4ve_reg.cur_y_base_adr_high = HIWORD(y);
    WriteRegM4VE(0x0c, m4ve_reg.reg0c);
    WriteRegM4VE(0x0d, m4ve_reg.reg0d);
    m4ve_reg.cur_cb_cr_base_adr_low = LOWORD(c);
    m4ve_reg.cur_cb_cr_base_adr_high = HIWORD(c);
    WriteRegM4VE(0x0e, m4ve_reg.reg0e);
    WriteRegM4VE(0x0f, m4ve_reg.reg0f);
    return;
}

// set the reference frame address
void MDrv_M4VE_SetRefAdr(U32 y, U32 c)
{
    ASSERT( y % 8 == 0 );
    y >>= MIU_SHIFT ;
    ASSERT( c % 8 == 0 );
    c >>= MIU_SHIFT ;

    m4ve_reg.ref_y_base_adr_low = LOWORD(y);
    m4ve_reg.ref_y_base_adr_high = HIWORD(y);
    WriteRegM4VE(0x06, m4ve_reg.reg06);
    WriteRegM4VE(0x07, m4ve_reg.reg07);
    m4ve_reg.ref_cb_base_adr_low = LOWORD(c);
    m4ve_reg.ref_cb_base_adr_high = HIWORD(c);
    WriteRegM4VE(0x08, m4ve_reg.reg08);
    WriteRegM4VE(0x09, m4ve_reg.reg09);
    return;
}

// set the re-constructed frame address
void MDrv_M4VE_SetRecAdr(U32 y, U32 c)
{
    ASSERT( y % 8 == 0 );
    y >>= MIU_SHIFT ;
    ASSERT( c % 8 == 0 );
    c >>= MIU_SHIFT ;

    m4ve_reg.rec_y_base_adr_low = LOWORD(y);
    m4ve_reg.rec_y_base_adr_high = HIWORD(y);
    WriteRegM4VE(0x10, m4ve_reg.reg10);
    WriteRegM4VE(0x11, m4ve_reg.reg11);
    m4ve_reg.rec_u_base_adr_low = LOWORD(c);
    m4ve_reg.rec_u_base_adr_high = HIWORD(c);
    WriteRegM4VE(0x12, m4ve_reg.reg12);
    WriteRegM4VE(0x13, m4ve_reg.reg13);
    return;
}

void MDrv_M4VE_SetBitstreamBuff(U32 start, U32 end)
{

    ASSERT( start % 8 == 0 );
    start >>= MIU_SHIFT;
//    ASSERT( end % 8 == 0 );
    end >>= MIU_SHIFT;

    m4ve_reg.sgc_bsbfsadr_low = LOWORD(start);
    m4ve_reg.sgc_bsbfsadr_high = HIWORD(start);
    WriteRegM4VE(0x1a, m4ve_reg.reg1a);
    WriteRegM4VE(0x1b, m4ve_reg.reg1b);
    m4ve_reg.sgc_bsbfeadr_low = LOWORD(end);
    m4ve_reg.sgc_bsbfeadr_high = HIWORD(end);
    WriteRegM4VE(0x1c, m4ve_reg.reg1c);
    WriteRegM4VE(0x1d, m4ve_reg.reg1d);
}

void MDrv_M4VE_SetBitstreamLineThr(U8 lineThr)
{
	m4ve_reg.enc_linesth = lineThr&0x3f;
	WriteRegM4VE(0x25, m4ve_reg.reg25);
}

U32 MDrv_M4VE_GetBitstreamEncodedLen(void)
{
	U16 val_26, val_27;
	ReadRegM4VE(0x26, &val_26);
	ReadRegM4VE(0x27, &val_27);
	return ((U32)(val_27&0xff)<<16) | val_26;
}

void MDrv_M4VE_SetBitstreamReadPtr(U32 addr) // 0x1e 0x1f
{
	ASSERT( addr % MIU_SIZE == 0 );
	addr >>= MIU_SHIFT;

	m4ve_reg.sgc_bsbfrptr_low = LOWORD(addr);
	WriteRegM4VE(0x1e, LOWORD(addr));
	m4ve_reg.sgc_bsbfrptr_high = HIWORD(addr) | (1<<BASE_ADR_HIGH_BITS);//0x100;
	WriteRegM4VE(0x1f, HIWORD(addr) | (1<<BASE_ADR_HIGH_BITS/*|0x100*/));
}

void MDrv_M4VE_SetBitstreamWritePtr(U32 addr)
{
	ASSERT( addr % MIU_SIZE == 0 );
	addr >>= MIU_SHIFT;

	m4ve_reg.sgc_swbfwptr_low = LOWORD(addr);
	WriteRegM4VE(0x20, LOWORD(addr));
	m4ve_reg.sgc_swbfwptr_high = HIWORD(addr) | (1<<BASE_ADR_HIGH_BITS);//|0x100;
	WriteRegM4VE(0x21, HIWORD(addr) | (1<<BASE_ADR_HIGH_BITS/*|0x100*/));
}


void MDrv_M4VE_SetIrqMask(U16 mask)
{
#ifdef _M4VE_T3_
    m4ve_reg.reg17 = mask&0xff;
#else
	m4ve_reg.reg17 = mask&0x7f;
#endif
	WriteRegM4VE(0x17, m4ve_reg.reg17);
}


void MDrv_M4VE_SetMiuParam(void)
{
    m4ve_reg.me_rpri_en = 0;
    m4ve_reg.cur_miu_blen = 0;
    m4ve_reg.me_cur_rpri_count = 0x10;
    m4ve_reg.sw_miu_blen = 0;
    m4ve_reg.umvc_sw_rpri_en = 0;
    m4ve_reg.umvc_sw_rpri = 0;
	WriteRegM4VE(0x77, m4ve_reg.reg77);

    m4ve_reg.me_sw_rpri_count = 0x10;
    m4ve_reg.umvc_sw_rpri_count = 0x10;
	WriteRegM4VE(0x78, m4ve_reg.reg78);
}

U8 MDrv_M4VE_GetDctMode(void)
{
    U16 val_76;
    ReadRegM4VE(0x76, &val_76);
    return val_76&0x1;
}
// set the qscale
void MHal_M4VE_SetQscale(U8 quant, U32 fixQP)  // reg 0x2a  bit 1-5
{
    ASSERT(quant > 0);
    ASSERT(quant < 32 );

	m4ve_reg.qscale = m4ve_quant = quant;
	WriteRegM4VE(0x2a, m4ve_reg.reg2a);
    m4ve_reg.mb_rc_en = x_rate_control_mb_enable = !fixQP;
    is_fixedQP = fixQP;
    if (fixQP)
        m4ve_quant = quant;
    WriteRegM4VE(0x0a, m4ve_reg.reg0a);
    M4VE_DEBUG("%d %d\n", quant, fixQP);
}

void MDrv_M4VE_SetQType(U8 qtype)  // reg 0x2a  bit 0
{
    ASSERT( qtype == 0 || qtype == 1 );

	m4ve_reg.quant_type = qtype;
	WriteRegM4VE(0x2a, m4ve_reg.reg2a);
}


U8 MDrv_M4VE_GetQType(void)
{
	U16 val_2a;
	ReadRegM4VE(0x2a, &val_2a);
	return val_2a&1;
}


// put the vop header for each vop.
//1    the bit 10 can be cleared ?
void MDrv_M4VE_PutHeaderSeq(U16 bits, U8 len)
{
    WriteRegM4VE(0x28, bits);
    WriteRegM4VE(0x29, len | 0x100);
    u32HeaderBitLen += len ;
//    if ((u32FrameCount % 10)==0)
//        printf("put Header: 0x%4x len: %d\n", bits ,len);
}

void MDrv_M4VE_PutHeaderEnd(void)
{
    encode_state = WAIT_FRAME_DONE;
	WriteRegM4VE(0x28, 0);
	WriteRegM4VE(0x29, 0x300);
}

//
// Mostly used for init
void MDrv_M4VE_SetMEMode(ME_REG_SET me_reg_set) //0x00
{
    m4ve_reg.initial_guess_en = me_reg_set.PMV_init_guess_enable;
    m4ve_reg.large_diamond_en = me_reg_set.large_dimond_enable;
    m4ve_reg.search_4block_en = me_reg_set.search_4block_enable;
    m4ve_reg.half_pixel_16x16_en = me_reg_set.half_pixel_16x16_enable;
    m4ve_reg.half_pixel_8x8_en = me_reg_set.half_pixel_8x8_enable;
	m4ve_reg.search_4block_range = me_reg_set.search_4block_range;
	m4ve_reg.mode_decision_en = 1;
    m4ve_reg.dynamic_me_en = me_dynamic_enable; //for saving MIU rate
    //printf("first M4VE register SetMEMode\n");
    //Triton will interrupt buffer-full at start if we don't mark this, because soft_reset should be enabled at start.
#if !defined(_AEON_PLATFORM_) && !defined(_M4VE_T3_)
	WriteRegM4VE(0x0, m4ve_reg.reg00);
#endif
}

void MDrv_M4VE_SetVopSize(U16 width, U16 height) // 0x04
{
	// width & height (unit: MBs)
	m4ve_reg.pic_width = width>>4;
	m4ve_reg.pic_height= height>>4;
#if defined(_TWO_CORES_)||defined(_M4VE_T3_)
    m4ve_reg.pic_height&=0x7F;
#else
    m4ve_reg.pic_height&=0x3F;
#endif
	WriteRegM4VE(0x4, m4ve_reg.reg04);

}

U16 MDrv_M4VE_GetVopWidth(void) // reg 0x4 bit 0-6
{
	U16 val_04;
	ReadRegM4VE(0x4, &val_04);
	return (val_04&0x7f)<<4;
}


U16 MDrv_M4VE_GetVopHeight(void) // reg 0x4 bit 8-13
{
	U16 val_04;
	ReadRegM4VE(0x4, &val_04);
	return ((val_04>>8)&0x7f)<<4;
}

U8 MDrv_M4VE_GetSearchRange(void)
{
	U16 val_54;
	ReadRegM4VE(0x54, &val_54);
	return MUX(val_54&1, 16, 32);
}


U32 DMAGetFile(char *tmpname, DWORD start, DWORD size, int type, char *Buffer)
{
#if defined(_UDMA_) || defined(_HIF_) || defined(_FPGA_)
    FPGA_MIUReadFile(tmpname, start, size, type, Buffer);
//#ifdef _UDMA_
    //UDMAOP_DMAGetFile(tmpname, start, size, type, Buffer);
//#elif defined(_HIF_)
    //HIF_DMAGetFile(tmpname, start, size, type, Buffer);
#elif defined(_TRACE32_CMM_)
    fprintf(fp_script, "B::DATA.SAVE.BINARY %s 0x%08X--0x%08X\n", tmpname,
        addr_phy2log(start), addr_phy2log(start+size));
#endif
    return TRUE;
}

U32 DMAPutFile(char *tmpname, DWORD start, int type, char *Buffer)
{
#if defined(_UDMA_) || defined(_HIF_) || defined(_FPGA_)
    FPGA_MIUWriteFile(tmpname, start, type, Buffer);
    //UDMAOP_DMAPutFile(tmpname, start, type, Buffer);
//#elif defined(_HIF_)
    //HIF_DMAPutFile(tmpname, start, type, Buffer);
#elif defined(_TRACE32_CMM_)
    fprintf(fp_script, "B::DATA.LOAD.BINARY %s 0x%08X\n", tmpname, addr_phy2log(start));
#endif
    return TRUE;
}

int DMAPutMem(char *from_ptr, int size /*Width*Height*3>>1*/, U32 to_addr, int type)
{
#if defined(_UDMA_) || defined(_HIF_) || defined(_FPGA_)
    FPGA_MIUWrite(from_ptr, size, to_addr, type);
//#ifdef _UDMA_
  //  UDMAOP_DMAPut(from_ptr, size, to_addr, type);
//#elif defined(_HIF_)
  //  UDMAOP_DMAPut(from_ptr, size, to_addr, type);
#elif defined(_AEON_PLATFORM_)
    memcpy((char *)addr_phy2log(to_addr), from_ptr, size);
#elif defined(_TRACE32_CMM_)
    //????
#else
    memcpy((char *)to_addr, from_ptr, size);
#endif
    return 0;
}

#ifdef _MUL_OUTBITS_BUF_
int MHal_M4VE_set_outbitsbuf()
{
    WORD val_1E, val_1F, val_1A, val_1B, val_1C, val_1D, val_25, val_20, val_21;
//ken: is it necessary?
    ReadRegM4VE(0x25, &val_25);
    val_25 &= 0xfdff;
    WriteRegM4VE(0x25, val_25);
    //sgc_bsbfsadr
    val_1A = LOWORD(outbitsbuf[bitsbuf_ind].start_addr>>MIU_SHIFT);
    val_1B = HIWORD(outbitsbuf[bitsbuf_ind].start_addr>>MIU_SHIFT);
    WriteRegM4VE(0x1A, val_1A);
    WriteRegM4VE(0x1B, val_1B);
#ifdef _M4VE_T3_
    WriteRegM4VE(0xC3, val_1A-1);
    WriteRegM4VE(0xC4, val_1B);
#endif
    //sgc_bsbfeadr
    val_1C = LOWORD(((outbitsbuf[bitsbuf_ind].end_addr)>>MIU_SHIFT)-1);
    val_1D = HIWORD(((outbitsbuf[bitsbuf_ind].end_addr)>>MIU_SHIFT)-1);
    WriteRegM4VE(0x1C, val_1C);
    WriteRegM4VE(0x1D, val_1D);
#ifdef _M4VE_T3_
    WriteRegM4VE(0xC1, val_1C+1);
    WriteRegM4VE(0xC2, val_1D);
#endif
    //sgc_bsbfrptr
    val_1E = LOWORD(outbitsbuf[bitsbuf_ind].start_addr>>MIU_SHIFT);
    val_1F = HIWORD(outbitsbuf[bitsbuf_ind].start_addr>>MIU_SHIFT); //sgc_bsbfrptr_high;
    WriteRegM4VE(0x1E, val_1E);
    WriteRegM4VE(0x1F, val_1F);
    //sgc_bsbfwptr
    val_20 = LOWORD(outbitsbuf[bitsbuf_ind].start_addr>>MIU_SHIFT);
    val_21 = HIWORD(outbitsbuf[bitsbuf_ind].start_addr>>MIU_SHIFT); //sgc_bsbfwptr_high;
    WriteRegM4VE(0x20, val_20);
    WriteRegM4VE(0x21, val_21);

    //sgc_multi_obuf_set
    ReadRegM4VE(0x25, &val_25);
    val_25 |= 0xa00;//0x800;
    WriteRegM4VE(0x25, val_25);
    encode_state = WAIT_FRAME_DONE;
    return 0;
}

int MHal_M4VE_EncSkipVOP(void)
{
    int enc_len;
    enc_len = MDrv_M4VE_GenSkipVOPHeader(P_VOP, (U8 *)(addr_phy2log(outbitsbuf[bitsbuf_ind].start_addr)));
    bitstreamM4VE.miuAddress = outbitsbuf[bitsbuf_ind].start_addr;
    bitstreamM4VE.size = enc_len;
    printf("in MHal_M4VE_EncSkipVOP 0x%x 0x%x %d\n", bitstreamM4VE.miuAddress
        , (U32)addr_phy2log(outbitsbuf[bitsbuf_ind].start_addr), enc_len);
    bitsbuf_ind = (bitsbuf_ind+1)%3;
    MHal_M4VE_set_outbitsbuf();
    M4VE_getbits_callback(bitstreamM4VE.miuAddress, addr_phy2log(bitstreamM4VE.miuAddress)
       , bitstreamM4VE.size, 1, P_VOP, 0);
    return 0;
}
#ifdef _COMP_INRUN_
extern FILE *compare_log;
#endif
U32 copybits2file(char *tmpname, char *realname, WORD val_60
#ifdef _COMP_INRUN_
                  , FILE *fp_golden
#endif
                  )
{
    DWORD start=0, size=0, stuff_size=0;
    WORD val_19, val_64, val_65;
    unsigned short sgc_bsbfwptr_low, sgc_bsbfwptr_high;
    unsigned int write_baddr;
    int frag_num=0;
#ifdef _NO_FILESYSTEM_
    unsigned char *buf;
#else
    char buf[512*1024];
    FILE *fp_temp;
    FILE *fp_real;
    int i, j, ret;
    char str [512];
#ifdef _COMP_INRUN_
    char comp_buf[4*1024];
#endif
#endif
#ifndef _M4VE_TRITON_
    if (val_60&0x10) {
        //judge buffer full first, for sgc bug....
        start = outbitsbuf[bitsbuf_ind].start_addr;
        size =  bitsbuf_size;
        if (val_60&0x8) {
            ReadRegM4VE(0x65, &val_65);
            frag_num = ((val_65 >> BASE_ADR_HIGH_BITS) & 0x07);  //Encoded bitstream last dword unused byte number
		}
    } else if (val_60&0x8) {
        //get sgc write address
        ReadRegM4VE(0x64, &val_64);
        sgc_bsbfwptr_low = (unsigned short) (val_64);
        ReadRegM4VE(0x65, &val_65);
        sgc_bsbfwptr_high = (unsigned short) (val_65 & BASE_ADR_HIGH_MASK);
        write_baddr = (MAKELONG(sgc_bsbfwptr_low, sgc_bsbfwptr_high)+1);
        write_baddr = write_baddr << MIU_SHIFT;
        frag_num = ((val_65 >> BASE_ADR_HIGH_BITS) & 0x07);  //Encoded bitstream last dword unused byte number

        M4VE_DEBUG("h64: 0x%X\n", (int)val_64);
        //printf("%s\n", str);
        M4VE_DEBUG("h65: 0x%X\n", (int)val_65);
        M4VE_DEBUG("start:0x%X  encode:0x%X\n", (U32)outbitsbuf[bitsbuf_ind].start_addr, write_baddr);
        start = outbitsbuf[bitsbuf_ind].start_addr;
        size = write_baddr-start;
#ifndef _HIF_
        if (size & 7) {
            stuff_size = 8-(size&7);
            size += stuff_size;
        }
#endif
    } else if (val_60&0x44) {
        printf("MC frame done IRQ or enlines 0x%X\n", val_60);
        val_19 = val_60&0x7f;//0x10;
        WriteRegM4VE(0x19, val_19);
        return 0;
    } else {
        printf("call copybits2file 0x%x because of M4VE delay IRQ MC\n", val_60);
    }
#ifndef _HIF_
    ASSERT((size&7)==0);
#endif
#ifdef _NO_FILESYSTEM_
    buf = (unsigned char *)addr_phy2log(start);
#elif defined(_FPGA_)
    fp_real = fopen(realname, "r+b");
    fseek(fp_real, 0, SEEK_END);
#ifdef _COMP_INRUN_
    i=0;
    do {

        if (i!=0) {
            if(HIF_DMAGetFile_slow(tmpname, addr_phy2log(start), size, 0, Script_Buffer)==FALSE) {
                printf("> fail, get file through DMA fail 1\n");
                return FALSE;
            }
        } else
#endif
        {
            if(DMAGetFile(tmpname, addr_phy2log(start), size, 0, Script_Buffer)==FALSE) {
                printf("> fail, get file through DMA fail 1\n");
                return FALSE;
            }
        }
        fp_temp = fopen(tmpname, "rb");
        fread(buf, size, 1, fp_temp);
        fclose(fp_temp);

#ifdef _COMP_INRUN_
        fread(comp_buf, sizeof(char), size-frag_num-stuff_size, fp_golden);
        buf[size-frag_num-stuff_size] = '\0';
        comp_buf[size-frag_num-stuff_size] = '\0';
        ret = strcmp(buf, comp_buf);

        if (ret==0)
            break;
        else {
            for (j=0; j<(size-frag_num-stuff_size); j++)
                if (buf[j]!=comp_buf[j])
                    break;
            fprintf(compare_log, "file: %s is different at frame#%d on 0x%x %d\n", realname, u32FrameCount,
                addr_phy2log(start), j);
            if (i!=2)
                fseek(fp_golden, -(size-frag_num-stuff_size), SEEK_CUR);
        }
        i++;
    } while(i<3);
#endif
    fwrite(buf, size-frag_num-stuff_size, 1, fp_real);
    fclose(fp_real);
#endif  //_NO_FILESYSTEM_
#endif //ifndef(_M4VE_TRITON_)
#ifdef _EN_CRC_
    M4VE_DEBUG("before updateChecksum 0x%x 0x%x 0x%x\n", drvM4VE_adler, (U32)buf, size-frag_num-stuff_size);
    UpdateCheckSum(buf, size-frag_num-stuff_size, &drvM4VE_adler);
    M4VE_DEBUG("encoding size: 0x%x   checksum: 0x%x\n", (U32)(size-frag_num-stuff_size), drvM4VE_adler);
#endif
    if ((((int)size)-frag_num-(int)stuff_size) < 0) {
        printf("bitstream size error %d %d %d 0x%x\n", (int)size, frag_num, (int)stuff_size, val_60);
    }
    bitstreamM4VE.size = size-frag_num-stuff_size;
    bitstreamM4VE.miuAddress = start;
    bitstreamM4VE.miuPointer = buf;
#ifdef _EN_HWSGC_CRC_
#ifdef _COMP_INRUN_
    {
        short val_63, val_55, val_66;
        m4ve_reg.crc_select = 1;
        WriteRegM4VE(0xa, m4ve_reg.reg0a);
        ReadRegM4VE(0x63, &val_63);
#ifdef _M4VE_BIG2_
        ReadRegM4VE(0x66, &val_66);
        fprintf(compare_log, "framenum %d, HW CRC code: %x %x\n", u32FrameCount, (int)val_63, (int)val_66);
        fflush(compare_log);
#elif defined(_M4VE_T3_)
        ReadRegM4VE(0x55, &val_55);
        fprintf(compare_log, "framenum %d, HW CRC code: %x %x\n", u32FrameCount, (int)val_55, (int)val_63);
        fflush(compare_log);
#endif
    }
#endif
#endif
//    val_19 = val_60&0x7f;//0x10;
//    WriteRegM4VE(0x19, val_19);
    //clear all set irq, need to set before change bitstream buffer
    //check if we run out of bitsbuf
    bitsbuf_ind = (bitsbuf_ind+1)%3;
    if (bitsbuf_used[bitsbuf_ind] && (val_60&0x8)==0) {
        printf("M4VE encoding bitsbuf full! %d %d\n", bitsbuf_ind, (int)(size-frag_num-stuff_size));
        encode_state = WAIT_AVAIL_BITSBUF;
    } else {
        MHal_M4VE_set_outbitsbuf();
    }
    val_19 = val_60&0x7f;//0x10;
    WriteRegM4VE(0x19, val_19);
    //tell hardware we have got bsbfwptr
    //    UDMA_RIURead16(reg_base+0x24, &val_24);
    //  val_24 |= (0x0100);
    //UDMA_RIUWrite16(reg_base+0x24, val_24);
    //return 0;
    return (size-frag_num-stuff_size);
}
#else
U32 copybits2file(char *tmpname, char *realname)
{
    U32 ret, dwaddr, read_baddr, sgc_start_addr, sgc_end_addr, write_baddr;
    WORD val_64, val_65, val_1A, val_1B, val_1C, val_1D, val_1E, val_1F, val_24;
    unsigned short sgc_bsbfwptr_low, sgc_bsbfwptr_high, sgc_bsbfrptr_low, sgc_bsbfrptr_high;
    DWORD start, size=0, dummy_start=0, dummy_size=0, firstseg_size=0;
    int frag_num=0;
    //char str [512];
#ifndef _NO_FILESYSTEM_ //!defined(_AEON_PLATFORM_) && !defined(_M4VE_OLYMPUS_)
    char buf [1280*512];
    FILE *fp_temp;
    FILE *fp_real;
#else
    char *buf;
#endif
    M4VE_DEBUG(printf("in copybits2file\n"));
    ReadRegM4VE(0x1a, &val_1A);
    ReadRegM4VE(0x1b, &val_1B);
    //printf("val_1A 0x%x, val_1B 0x%x\n", val_1A, val_1B);
    sgc_bsbfwptr_low = (unsigned short) (val_1A);
    sgc_bsbfwptr_high = (unsigned short) (val_1B & BASE_ADR_HIGH_MASK);
    dwaddr = MAKELONG(sgc_bsbfwptr_low, sgc_bsbfwptr_high);
    sgc_start_addr = dwaddr << MIU_SHIFT;
    //printf("sgc start address: %x\n", dwaddr);
    //Get sgc_end_addr
    ReadRegM4VE(0x1c, &val_1C);
    ReadRegM4VE(0x1d, &val_1D);
    //printf("val_1C 0x%x, val_1D 0x%x\n", val_1C, val_1D);
    sgc_bsbfwptr_low = (unsigned short) (val_1C);
    sgc_bsbfwptr_high = (unsigned short) (val_1D & BASE_ADR_HIGH_MASK);
    dwaddr = MAKELONG(sgc_bsbfwptr_low, sgc_bsbfwptr_high);
    sgc_end_addr = dwaddr << MIU_SHIFT;

    //get sgc write address
    ReadRegM4VE(0x64, &val_64);
    ReadRegM4VE(0x65, &val_65);
    sgc_bsbfwptr_low = (unsigned short) (val_64);
    sgc_bsbfwptr_high = (unsigned short) (val_65 & BASE_ADR_HIGH_MASK);
    dwaddr = MAKELONG(sgc_bsbfwptr_low, sgc_bsbfwptr_high);
    write_baddr = dwaddr << MIU_SHIFT;
    //printf("val_64 0x%x, val_65 0x%x\n", val_64, val_65);
    //printf("write address: %x\n", dwaddr);
    ReadRegM4VE(0x1E, &val_1E);
    ReadRegM4VE(0x1F, &val_1F);
    sgc_bsbfrptr_low = (unsigned short) (val_1E);
    sgc_bsbfrptr_high = (unsigned short) (val_1F & BASE_ADR_HIGH_MASK);
    dwaddr = MAKELONG(sgc_bsbfrptr_low, sgc_bsbfrptr_high);
    read_baddr = dwaddr << MIU_SHIFT;
    //printf("val_1E 0x%x, val_1F 0x%x\n", val_1E, val_1F);
    frag_num = ((val_65 >> BASE_ADR_HIGH_BITS) & 0x07);  //Encoded bitstream last dword unused byte number

    if(write_baddr != prev_read_baddr) {
        if(write_baddr > prev_read_baddr) {
            if(prev_read_baddr == 0) {
                start = (DWORD) (sgc_start_addr);
                size = (DWORD) (write_baddr-sgc_start_addr+MIU_SIZE);
#ifdef _UDMA_
#if (MIU_SIZE==4)
                if (start&7) {
                    printf("bitstream start address is not 8bytes aligned 0x%x", start);
                    dummy_start = (start&7);
                }
                if (size&7) {
                    printf("bitstream size is not 8bytes aligned 0x%x\n", size);
                    dummy_size = 8-(size&7);
                    size += dummy_size;
                }
                if (dummy_start) {
                    dummy_size += 8;
                    size += 8;
                }
#endif
                ASSERT((size&7)==0);
#endif
#ifdef _NO_FILESYSTEM_
                buf = (char *)addr_phy2log(start);
#endif
#ifdef _AEON_PLATFORM_
                //printf("write data 1: %x %d\n", start, size-frag_num-dummy_size-dummy_start);
                if (bitstreamM4VE.miuAddress==0) {
                    bitstreamM4VE.miuAddress = start+dummy_start;
                    bitstreamM4VE.miuPointer = buf;
                }
#ifndef _ENABLE_AVIMUX_
                memcpy(((U8 *)bitstreamM4VE.miuPointer)+bitstreamM4VE.size, buf, size-frag_num-dummy_size-dummy_start);
#endif
                bitstreamM4VE.size += size-frag_num-dummy_size-dummy_start;
#else
                if(DMAGetFile(tmpname, addr_phy2log(start), size, 0, Script_Buffer)==FALSE)
                {
                    printf("> fail, get file through DMA fail 1\n");
                    return FALSE;
                }
#endif
            } else {
            //error = write(m4ve_inst.fp_m4ve, m4ve_inst.sgc_start_ptr+ (read_baddr+4 - m4ve_inst.sgc_start_addr), write_baddr-(read_baddr+4)  +4-frag_num);

                start = (DWORD) (prev_read_baddr+MIU_SIZE);
                size = (DWORD) (write_baddr-(read_baddr+MIU_SIZE)+MIU_SIZE);
            //size = (DWORD) (write_baddr-(read_baddr+MIU_SIZE)  +MIU_SIZE-frag_num);

#ifdef _UDMA_
#if (MIU_SIZE==4)
                if (start&7) {
                    printf("bitstream start address is not 8bytes aligned 0x%x\n", start);
                    dummy_start = (start&7);
                }
                if (size&7) {
                    printf("bitstream size is not 8bytes algned 0x%x\n", size);
                    dummy_size = 8-(size&7);
                    size += dummy_size;
                }
                if (dummy_start) {
                    dummy_size += 8;
                    size += 8;
                }
#endif
                ASSERT((size&7)==0);
#endif
                M4VE_DEBUG("write data 2: %x %d\n", start, size-frag_num-dummy_size-dummy_start);
#ifdef _NO_FILESYSTEM_
                buf = (char*)(addr_phy2log(start+dummy_start));
#endif
                //printf("in copybits: %x %x %x %x %x %x\n", u32FrameCount, buf, buf[0], buf[1], buf[2], size-frag_num-dummy_size-dummy_start);
#ifdef _AEON_PLATFORM_
                if (bitstreamM4VE.miuAddress==0) {
                    bitstreamM4VE.miuAddress = start+dummy_start;
                    bitstreamM4VE.miuPointer = buf;
                }
#ifndef _ENABLE_AVIMUX_
                memcpy(((U8 *)bitstreamM4VE.miuPointer)+bitstreamM4VE.size, buf, size-frag_num-dummy_size-dummy_start);
#endif
                bitstreamM4VE.size += size-frag_num-dummy_size-dummy_start;
#else
                if(DMAGetFile(tmpname, addr_phy2log(start), size, 0, Script_Buffer)==FALSE)
                {
                    printf("> fail, get file through DMA fail 2\n");
                    return FALSE;
                }
#endif
            }

        } else {
            if(write_baddr != sgc_end_addr ) {
                start = (DWORD) (prev_read_baddr+MIU_SIZE);
                size = (DWORD) (sgc_end_addr-(prev_read_baddr+MIU_SIZE) +MIU_SIZE);

#ifdef _UDMA_
#if (MIU_SIZE==4)
                if (start&7) {
                    printf("bitstream start address is not 8bytes aligned 0x%x\n", start);
                    dummy_start = (start&7);
                }
                if (size&7) {
                    printf("bitstream size is not 8bytes algned 0x%x\n", size);
                    dummy_size = 8-(size&7);
                    size += dummy_size;
                }
                if (dummy_start) {
                    dummy_size += 8;
                    size += 8;
                }
#endif
#endif
                firstseg_size = size-dummy_size-dummy_start;
                ASSERT((size&7)==0);
                //printf("write data 3: %x %d\n", start, size-dummy_size-dummy_start);
#ifdef _NO_FILESYSTEM_
                buf = (char*)(addr_phy2log(start+dummy_start));
#endif
#ifdef _AEON_PLATFORM_
                if (bitstreamM4VE.miuAddress==0) {
                    bitstreamM4VE.miuAddress = start+dummy_start;
                    bitstreamM4VE.miuPointer = buf;
                }
#ifndef _ENABLE_AVIMUX_
                memcpy(((U8 *)bitstreamM4VE.miuPointer)+bitstreamM4VE.size, buf, size-dummy_size-dummy_start);
#endif

                bitstreamM4VE.size += size-dummy_size-dummy_start;
                //printf("in copybits2: %x %x %x %x %x %x\n", u32FrameCount, buf, buf[0], buf[1], buf[2], bitstreamM4VE.size);
#ifndef _ENABLE_AVIMUX_
                FILE_write(bitstreamM4VE.miuAddress, bitstreamM4VE.size);
                bitstreamM4VE.miuPointer = bitstreamM4VE.miuAddress = 0;
                bitstreamM4VE.size = 0;
#endif
                 //memset(&bitstreamM4VE, 0, sizeof(MEMMAP_t));
                ////FILE_write(start, size);
#elif !defined(_NO_FILESYSTEM_)
                if(DMAGetFile(tmpname, addr_phy2log(start), size, 0, Script_Buffer)==FALSE)
                {
                    printf("> fail, get file through DMA fail 3.1\n");
                    return FALSE;
                }
//#ifndef _AEON_PLATFORM_
                fp_temp = fopen(tmpname, "rb");
#if (MIU_SIZE==4)
                fseek(fp_temp, dummy_start, SEEK_SET);
                if (dummy_start) {
                    size-=dummy_start;
                }
#endif
                fread(buf, size, 1, fp_temp);
                fclose(fp_temp);

                fp_real = fopen(realname, "ab");
                fwrite(buf, size, 1, fp_real);
                fclose(fp_real);
#endif
                dummy_size = dummy_start = 0;
                //error = write(m4ve_inst.fp_m4ve, m4ve_inst.sgc_start_ptr, write_baddr- m4ve_inst.sgc_start_addr+ 4 - frag_num);
                start = (DWORD) (sgc_start_addr);
                //size = (DWORD) (write_baddr- sgc_start_addr+ MIU_SIZE - frag_num);
                size = (DWORD) (write_baddr- sgc_start_addr+ MIU_SIZE);

#ifdef _UDMA_   //#ifndef _HIF_
#if (MIU_SIZE==4)
                if (start&7) {
                    printf("bitstream start address is not 8bytes aligned 0x%x\n", start);
                    dummy_start = (start&7);
                }
                if (size&7) {
                    printf("bitstream size is not 8bytes algned 0x%x\n", size);
                    dummy_size = 8-(size&7);
                    size += dummy_size;
                }
                if (dummy_start) {
                    dummy_size += 8;
                    size += 8;
                }
#endif
                ASSERT((size&7)==0);
#endif
                //printf("write data 4: %x %d\n", start, size-frag_num-dummy_size-dummy_start);
#if !defined(_ENABLE_AVIMUX_) && defined(_NO_FILESYSTEM_)
                buf = (char*)(addr_phy2log(start+dummy_start));
#endif
#ifdef _AEON_PLATFORM_
                M4VE_DEBUG(printf("write data 4: %x %d\n", start, size-frag_num-dummy_size-dummy_start));
                //buf = (char*)((start+dummy_start) | AEONMEM_LOGMAPPHY);
                //bitstreamM4VE_index += size-frag_num-dummy_size-dummy_start;
#ifndef _ENABLE_AVIMUX_
                bitstreamM4VE.miuAddress = start+dummy_start;
                bitstreamM4VE.miuPointer = buf;
                memcpy(((U8 *)bitstreamM4VE.miuPointer)+bitstreamM4VE.size, buf, size-frag_num-dummy_size-dummy_start);
                bitstreamM4VE.size = size-frag_num-dummy_size-dummy_start;
#else
                bitstreamM4VE.size += size-frag_num-dummy_size-dummy_start;
#endif

                //FILE_write(start, size-frag_num);
#else
                if(DMAGetFile(tmpname, addr_phy2log(start), size, 0, Script_Buffer)==FALSE)
                {
                    printf("> fail, get file through DMA fail 3.2\n");
                    return FALSE;
                }
#endif
            }
        }

        //update read ptr
        if(write_baddr != sgc_end_addr )
            prev_read_baddr = write_baddr;
        else
            prev_read_baddr = 0;

        dwaddr = write_baddr >> MIU_SHIFT;
        sgc_bsbfrptr_low = LOWORD(dwaddr);
        sgc_bsbfrptr_high = HIWORD(dwaddr);
        val_1E = sgc_bsbfrptr_low;
        val_1F = sgc_bsbfrptr_high;

        //m4ve_reg.sgc_putrptr = 1; // notify sgc read ptr updated
        val_1F |= (1<<BASE_ADR_HIGH_BITS);//(0x0100);
        WriteRegM4VE(0x1E, val_1E);
        WriteRegM4VE(0x1F, val_1F);
    }

    //m4ve_reg.sgc_readwptr = 1;
    ReadRegM4VE(0x24, &val_24);
    val_24 |= (1<<BASE_ADR_HIGH_BITS);//(0x0100);
    WriteRegM4VE(0x24, val_24);

#ifndef _NO_FILESYSTEM_ //!defined(_AEON_PLATFORM_) && !defined(_M4VE_OLYMPUS_)
    fp_temp = fopen(tmpname, "rb");
#ifdef _UDMA_
#if (MIU_SIZE==4)
    fseek(fp_temp, dummy_start, SEEK_SET);
    if (dummy_start) {
        size-=dummy_start;
        if (size&7) {
            dummy_size-=4;
        }
    }
#endif
#endif
    fread(buf, size-frag_num-dummy_size, 1, fp_temp);
    fclose(fp_temp);

    fp_real = fopen(realname, "ab");
    fwrite(buf, size-frag_num-dummy_size, 1, fp_real);
    fclose(fp_real);
    if( size -frag_num-dummy_size > sizeof(buf) ) {
        printf("fail, buf size is too small, copy will fail");
    }
#endif //_AEON_PLATFORM_ _M4VE_OLYMPUS_
#ifdef _EN_CRC_
    M4VE_DEBUG("buf: 0x%x %d %d %d 0x%x\n", (U32)buf, buf[1], buf[2], buf[3], adler);
    UpdateCheckSum(buf, size-frag_num-dummy_size, &drvM4VE_adler);
    M4VE_DEBUG("checksum: %x  size:%d\n", adler, size-frag_num-dummy_size);

#endif
    //printf("size: %d\n", size-frag_num-dummy_size+firstseg_size);
    return (size-frag_num-dummy_size+firstseg_size);
}// ~is_frame_end = 1
#endif

#if defined(_NO_WAIT_FRAMEDONE_)// || (defined(_MUL_OUTBITS_BUF_)&&defined(_M4VE_T3_))
int MDrv_M4VE_CheckEncodeDone(char *filename, char *realname)
{
    U16 val_60, val_19;//, val_20;
    U16 sgc_frame_done;
    U32 encLen=0;
    static U32 totalLen=0;
    U32 AccuEncLen;
    U8  voptype;
    voptype = MDrv_M4VE_GetVopType();
    AccuEncLen = MDrv_M4VE_GetBitstreamEncodedLen();
    //M4VE_DEBUG(printf("//Enc_Len: 0x%x preLen: 0x%x accu-Len: 0x%x\n",AccuEncLen - preLen, preLen, AccuEncLen));
    ReadRegM4VE(0x60, &val_60);
//    printf("val60: 0x%x\n", (int)val_60);
//    printk("MIPS 0x%x 0x%x\n", *((U16 *)(0xBF203200+0x76*4)), *((U16 *)(0xBF203200+0x75*4)));
    sgc_frame_done = ((val_60 >> 3) & 0x1);
#if defined(_M4VE_TRITON_)&&defined(_AEON_PLATFORM_)
    mc_frame_done = ((val_60 >> 6) & 0x1);
    if (sgc_frame_done == 0 || (voptype != 2 && mc_frame_done ==0)) {
        printf("M4VE have not frame done\n");
        return 0;
    }
#endif
    if (val_60==0) {
        printf("call MDrv_M4VE_CheckEncodeDone %d because M4VE delay IRQ MC\n", val_60);
//        while(1);
        return 1;
    }
    // ~ is_frame_end=0
    encLen = copybits2file(filename, realname, val_60);
    if (encLen==0) {
        //for MC frame done IRQ
        printf("MDrv_M4VE_CheckEncodeDone return\n");
        return 1;
    } else if (((int)encLen)<0) {
        printf("MDrv_M4VE_CheckEncodeDone, encLen<0 %d\n", (int)encLen);
    }
    totalLen += encLen;
    val_19 = val_60;
    {
        U32 rec_addr;
        rec_addr = ((m4ve_reg.rec_y_base_adr_high<<16) | m4ve_reg.rec_y_base_adr_low)<<MIU_SHIFT;
//        printf("!!!CheckEncodeDone 0x%x %d\n", rec_addr , VopType);
        M4VE_getbits_callback(bitstreamM4VE.miuAddress, addr_phy2log(bitstreamM4VE.miuAddress)
            , bitstreamM4VE.size, sgc_frame_done, voptype, rec_addr);
    }
    if (sgc_frame_done) {
#ifdef CVBR_RATE_CONTROL
        if (!is_fixedQP)
            cvbr_rc_vop_after(encLen);
#elif defined(X_RATE_CONTROL)
    if (u32FrameCount==0 && DOUBLE_FRAME!=2
#ifdef _M4VE_BIG2_
        && !g_IsShortHeader
#endif
        )
        totalLen-=19; //don't count VOL bytes, sync to CModel
        if (!is_fixedQP)
            rc_vop_after(totalLen); //rate control when frame end
#endif
        totalLen = 0; //clear to 0 for next frame usage
//#ifndef _NO_WAIT_FRAMEDONE_
        msAPI_M4VE_EncodeVOP_End(&m4ve_frame_ptr_tmp);
//#endif
        M4VE_DEBUG("encode one frame end\n");
    }
#ifndef _MUL_OUTBITS_BUF_
    WriteRegM4VE(0x19, val_19);
#endif
    //ReadRegM4VE(0x60, &val_60);
    //printf("after reset reg0x19, reg0x60=%d\n", val_60);


    preLen += totalLen;
    m4ve_reg.frame_start = 0;
    M4VE_DEBUG("encode_state: %d\n", encode_state);
    return 1;
}
#else
void MDrv_M4VE_CheckEncodeDone(char *filename, char *realname
#ifdef _COMP_INRUN_
                               , FILE *fp_golden
#endif
                               )
{
    U16 val_60, val_19, val_20;
    U16 sgc_frame_done, mc_frame_done;
    U32 cnt = 0;
    U32 encLen=0, totalLen=0;
    U32 AccuEncLen;
    U8 voptype = MDrv_M4VE_GetVopType();
    AccuEncLen = MDrv_M4VE_GetBitstreamEncodedLen();
#ifdef _NO_FILESYSTEM_
    //printf("interrupt start time:%d\n", MsOS_GetSystemTick());
    M4VE_DEBUG("//Enc_Len: 0x%x preLen: 0x%x accu-Len: 0x%x\n",AccuEncLen - preLen, preLen, AccuEncLen);
#else
    fprintf(fp_script, "//Enc_Len: 0x%x accu-Len: 0x%x\n",AccuEncLen - preLen, AccuEncLen);
#endif
#if defined(_WIN32) && !defined(_FPGA_)
#else
    do {
        ReadRegM4VE(0x60, &val_60);
        sgc_frame_done = ((val_60 >> 3) & 0x1);
        mc_frame_done = ((val_60 >> 6) & 0x1);
        if (val_60&0x80) {
            printf("MIU access out of boundary\n");
        }
#ifdef _MUL_OUTBITS_BUF_
        if (val_60 & 0x18) {
            encLen = copybits2file(filename, realname, val_60
#ifdef _COMP_INRUN_
                     , fp_golden
#endif
                     );
            totalLen += encLen;
            M4VE_getbits_callback(bitstreamM4VE.miuAddress, addr_phy2log(bitstreamM4VE.miuAddress)
                , bitstreamM4VE.size, sgc_frame_done);
            val_19 = val_60;
            WriteRegM4VE(0x19, val_19);
        }
#endif
        //if( cnt % 10 == 0 )
        M4VE_DEBUG("val_60 %X ", val_60);
        cnt ++;
        if( (cnt & 0xfff) == 0 ) {
            M4VE_DEBUG("C ");
            if( cnt > /*0xFFFFFF*/0xFFFF ) {
                M4VE_DEBUG("\n> fail, can not get the sgc_frame_done or mc_frame_done\n");
                break;
            }
        }
    } while( sgc_frame_done == 0 || (voptype != 2 && mc_frame_done ==0) );
#endif
#ifndef _MUL_OUTBITS_BUF_
    // ~ is_frame_end=0
    encLen = copybits2file(filename, realname);
    totalLen += encLen;
    val_19 = val_60;
    WriteRegM4VE(0x19, val_19);
#else
    /*  if we define _MUL_OUTBITS_BUF_ buffer-full and frame-done irq are cleared in copybits2file.
        else we clear them here
    */
//    val_19 = val_60&0x40;
#endif

#ifdef CVBR_RATE_CONTROL
    if (!is_fixedQP)
        cvbr_rc_vop_after(encLen);
#elif defined(X_RATE_CONTROL)
    if (u32FrameCount==0 && DOUBLE_FRAME!=2
#ifdef _M4VE_BIG2_
        && !g_IsShortHeader
#endif
        )
        totalLen-=19; //don't count VOL bytes, sync to CModel
    if (!is_fixedQP)
        rc_vop_after(totalLen);
#endif
    preLen += totalLen;
    m4ve_reg.frame_start = 0;
    //m4ve_reg.pre_ip = VopType; //different from CModel: CModel is set by current frame type. We set previous frame type
    encode_state = WAIT_INPUT_FRAME;
    //printf("interrupt end time:%d\n", MsOS_GetSystemTick());
    M4VE_DEBUG("encode_state: %d\n", encode_state);
}
#endif //defined(_NO_WAIT_FRAMEDONE_) || (defined(_MUL_OUTBITS_BUF_)&&defined(_M4VE_T3_))

#ifdef _ENABLE_ISR_
#ifdef _AEON_PLATFORM_
#ifdef _M4VE_T3_
typedef void (*InterruptCb)(void);
BOOL M4VE_ISR_Control(BOOL bEnable, InterruptCb pCallback)
{
    U32 u32OldIntr;
    if(bEnable)
    {
       //MsOS_AttachInterrupt(E_IRQ_M4VE, pCallback);
        mdrv_irq_attach(IRQ_M4VE, pCallback, IRQ_M4VE);
        mdrv_irq_unmask(IRQ_M4VE);
       //MsOS_EnableInterrupt(E_IRQ_M4VE);
       printf("Enable E_IRQ_M4VE:%d\n", IRQ_M4VE);
       //M4VE_DEBUG(fprintf(fp_script, "Enable E_IRQ_M4VE:%d\n", E_IRQ_M4VE));
    }
    else
    {
        mdrv_irq_mask(IRQ_M4VE);
        //MsOS_DisableInterrupt(E_IRQ_M4VE);
    }

    //MsOS_RestoreAllInterrupts(u32OldIntr);
    return TRUE;
}

void M4VE_HandleIsr(void)
{
    char temp[30];
    char real[30];

    MDrv_M4VE_CheckEncodeDone(temp, real);
    mdrv_irq_unmask(IRQ_M4VE);
    //MsOS_EnableInterrupt(IRQ_M4VE);
}
#else
//T1+Triton
BOOL M4VE_ISR_Control(BOOL bEnable, InterruptCb pCallback)
{
    U32 u32OldIntr;
    MsOS_DiableAllInterrupts(u32OldIntr);

    if(bEnable)
    {
       MsOS_AttachInterrupt(E_IRQ_M4VE, pCallback);

       MsOS_EnableInterrupt(E_IRQ_M4VE);
       printf("Enable E_IRQ_M4VE:%d\n", E_IRQ_M4VE);
       //M4VE_DEBUG(fprintf(fp_script, "Enable E_IRQ_M4VE:%d\n", E_IRQ_M4VE));
    }
    else
    {
        MsOS_DisableInterrupt(E_IRQ_M4VE);
    }

    MsOS_RestoreAllInterrupts(u32OldIntr);
    return TRUE;
}

void M4VE_HandleIsr(void)
{
    char temp[30];
    char real[30];
//    U16 irq = __M4VE_REG(0x60) ;
    //M4VE_DEBUG(fprintf(fp_script, "irq:0x%x \n",irq));
    //printf("irq:0x%x \n",irq);
    //__M4VE_REG(0x19) = irq ; // & ~0x48 ; // clear the irq
    MDrv_M4VE_CheckEncodeDone(temp, real);
    MsOS_EnableInterrupt(E_IRQ_M4VE);
}
#endif

void M4VE_ISR_Enable(U8 y)
{
    if( y )
        M4VE_ISR_Control(TRUE,M4VE_HandleIsr);
    else M4VE_ISR_Control(FALSE,NULL);
}
#elif defined(_M4VE_T3_) && defined(_MIPS_PLATFORM_)
static irqreturn_t MDrv_M4VE_ISR(int irq, void *dev_id )
{
    char temp[30];
    char real[30];
    M4VE_DEBUG("get in MDrv_M4VE_ISR\n");

    MDrv_M4VE_CheckEncodeDone(temp, real);
    return IRQ_HANDLED;
//    cyg_interrupt_unmask(vector); //Unmask the interrup that has been masked in _enc_isr()
}

void M4VE_ISR_Enable(U8 y)
{
    int result;
    printf("Enable M4VE ISR\n");
//    enable_irq(E_IRQ_M4VE);
    result = request_irq(E_IRQ_M4VE, MDrv_M4VE_ISR, SA_INTERRUPT, "M4VE_IRQ", NULL);
}
#elif defined(_M4VE_BIG2_) && defined(_MIPS_PLATFORM_)
static cyg_interrupt m4ve_int;
static cyg_handle_t m4ve_handle;
cyg_vector_t m4ve_vector = CYGNUM_HAL_INTERRUPT_M4VE;
cyg_priority_t m4ve_priority = CYGNUM_HAL_PRI_HIGH;

U32 _m4ve_isr( cyg_vector_t vector, cyg_addrword_t data)
{
    M4VE_DEBUG("..\n");
    cyg_interrupt_mask( vector ); //Mask interrupt here and unmask it in _enc_dsr()
    cyg_interrupt_acknowledge( vector );
	return( CYG_ISR_HANDLED | CYG_ISR_CALL_DSR );
}

void fn_m4ve_dsr( cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    char temp[30];
    char real[30];
    MDrv_M4VE_CheckEncodeDone(temp, real);
    cyg_interrupt_unmask(vector); //Unmask the interrup that has been masked in _enc_isr()
}
void M4VE_ISR_Enable(U8 y)
{
//    static cyg_interrupt m4ve_int;
//    static cyg_handle_t m4ve_handle;
//    cyg_vector_t m4ve_vector = CYGNUM_HAL_INTERRUPT_M4VE;
//    cyg_priority_t m4ve_priority = CYGNUM_HAL_PRI_HIGH;

    cyg_interrupt_create(m4ve_vector, m4ve_priority, 0 ,&_m4ve_isr, fn_m4ve_dsr, &m4ve_handle, &m4ve_int);
    cyg_interrupt_attach( m4ve_handle );
    cyg_interrupt_unmask( m4ve_vector );
}
void M4VE_ISR_Disable(void)
{
//    static cyg_interrupt m4ve_int;
//    static cyg_handle_t m4ve_handle;

    cyg_interrupt_delete( m4ve_handle);
    cyg_interrupt_detach( m4ve_handle );
    cyg_interrupt_mask( m4ve_vector );
}
#elif defined(_WIN32)
static int MDrv_M4VE_ISR(int irq, void *dev_id )
{
    char temp[30];
    char real[30];
    M4VE_DEBUG("get in MDrv_M4VE_ISR\n");
    srand(1000);
    while(1) {
        Sleep(300);
        if (encode_state==WAIT_FRAME_DONE) {
            if (rand()%2) //buffer full
                WriteRegM4VE(0x60, 0x10);
            else { //frame done
                unsigned long my_temp;
                my_temp = rand()%bitsbuf_size;
                WriteRegM4VE(0x60, 0x48 | my_temp==(bitsbuf_size-1));
                WriteRegM4VE(0x64, LOWORD((outbitsbuf[bitsbuf_ind].start_addr+my_temp)>>MIU_SHIFT));
                WriteRegM4VE(0x65, HIWORD((outbitsbuf[bitsbuf_ind].start_addr+my_temp)>>MIU_SHIFT));
            }
            MDrv_M4VE_CheckEncodeDone(temp, real);
        }
    }
    return 0;//IRQ_HANDLED;
//    cyg_interrupt_unmask(vector); //Unmask the interrup that has been masked in _enc_isr()
}
void M4VE_ISR_Enable(U8 y)
{
    int irq;
    pthread_t thread_info;
    pthread_attr_t thrattr;
    struct sched_param thrsched;

    pthread_attr_init(&thrattr);
    pthread_attr_setdetachstate(&thrattr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setinheritsched(&thrattr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&thrattr, SCHED_RR);
    pthread_attr_getschedparam(&thrattr, &thrsched);
    thrsched.sched_priority = 80;
    pthread_attr_setschedparam(&thrattr, &thrsched);
    pthread_attr_getschedparam(&thrattr, &thrsched);
    if (0 != pthread_create(&thread_info, &thrattr, MDrv_M4VE_ISR, NULL)) {
        printf("Mhal M4VE ISR create thread error\n");
    }
}
#endif //_AEON_PLATFORM_
#endif //_ENABLE_ISR
/***************************************************************
 *
 *
 *
 *  API Level interface
 *
 *
 *
 **************************************************************/


U8 msAPI_M4VE_VOPMode(void)
{
#if 0
    //this should be formal encoding process
    if (u8PBlength) {
        if (u32FrameCount==0) {
            return I_VOP;
        } else if (u32FrameCount==1)
            return P_VOP;
        else if (PBcount<u8PBlength) {
            PBcount++;
            return B_VOP;
        } else if (IPcount<u8IPlength) {
            IPcount++;
            PBcount=0;
            return P_VOP;
        } else {
            IPcount=0;
            return I_VOP;
        }
    } else
        return (u32FrameCount % u8IPlength == 0) ? I_VOP : P_VOP ;
#else
/*
    if (u32FrameCount==0)
        return I_VOP;
    else
        return P_VOP;
*/
    //this is synced to CModel output
    if (u8PBlength)
        if (((u32FrameCount+1) % u8IPlength == 0) || u32FrameCount==0)
            return I_VOP;
        else if ((u32FrameCount % (u8PBlength+1))==1) //((u32FrameCount % (u8PBlength+1))==0)
            return P_VOP;
        else
            return B_VOP;
    else if (u8IPlength)
        return (u32FrameCount % u8IPlength == 0) ? I_VOP : P_VOP ;
    else
        return I_VOP;
#endif
}

static int attach_ISR=1; //for AEON ISR attach
extern void init_HeaderSeq(unsigned int f_count);

void MHal_M4VE_SetFramerate(U32 FPS)
{
    framerate = FPS;
    VopTimeIncResolution = framerate;
}
void MHal_M4VE_SetBitrate(U32 bitrate)
{
    M4VE_Bitrate = bitrate;
    if (!is_fixedQP)
        rc_init(Width, Height, M4VE_Bitrate);
    /*
//T3 modified start
#ifdef _MIPS_PLATFORM_
        m4ve_reg.reg_m4ve_target_mb_bits = (int) (((int)M4VE_Bitrate/framerate) / ((Width>>4) * (Height>>4)));
#else
        m4ve_reg.reg_m4ve_target_mb_bits = (int) (((double)M4VE_Bitrate/framerate) / ((Width>>4) * (Height>>4)));
#endif
#ifdef _TWO_CORES_
        m4ve_reg.reg_m4ve_target_mb_bits >>= 1;
#endif
	WriteRegM4VE(0x75, m4ve_reg.reg75);
*/
//T3 modified end
    M4VE_DEBUG("calling msAPI_M4VE_SetBitrate %d %d\n", bitrate, is_fixedQP);
}
#ifdef _M4VE_BIG2_
void init_h263_gvar(int width, int height)
{
    int qtype;

    qtype = 0;
    is_sr16 = 1;  // vop_fcode_forward must be 1
    interlaced = 0;
    //para_intra_dc_vlc_thr = 0;
    //B_VOP_INSERT = 0;   // No b-vop
    u8PBlength = 0;

    isUMV = 0;
    // No 4MV
    me_reg.search_4block_enable  = 0;
    me_reg.half_pixel_8x8_enable = 0;
    if (width == 128 && height == 96) {
        g_nSourceFormat = 1;
        g_nGobunit = 0;
        g_nGobHeightInMb = 4;//1;
        g_H263Resync = 3;//1;
        //g_nGobHeightInMb = 1;
        //if (g_H263Resync>0) g_H263Resync = 1;
    } else if (width == 176 && height == 144) {
        g_nSourceFormat = 2;
        g_nGobunit = 0;
        g_nGobHeightInMb = 1;//2;//4;
        g_H263Resync = 2;//3;
        //g_nGobHeightInMb = 1;
        //if (g_H263Resync>0) g_H263Resync = 1;
    } else if (width == 352 && height == 288) {
        g_nSourceFormat = 3;
        g_nGobunit = 0;
        g_nGobHeightInMb = 2;//4;//1;
        g_H263Resync = 2;//3;//1;

        //g_nGobHeightInMb = 1;
        //if (g_H263Resync>0) g_H263Resync = 1;
    } else if (width == 704 && height == 576) {
        g_nSourceFormat = 4;
        g_nGobunit = 1;
        g_nGobHeightInMb = 4;//2;
        g_H263Resync = 3;//2;
        //g_nGobHeightInMb = 2;
        //if (g_H263Resync>0) g_H263Resync = 2;
    } else if (width == 1408 && height == 1152) {
        g_nGobunit = 2;
        g_nSourceFormat = 5;
        g_nGobHeightInMb = 4;
        if (g_H263Resync>0) g_H263Resync = 3;
    } else  {
        //ASSERT(0); // not support now!!
        //return -1;
        //Support H.263 plus header
        g_nSourceFormat = 7;	// 111: Extended PTYPE
        if (height<=400)
			g_nGobunit = 0;
		else if (height<=800)
			g_nGobunit = 1;
		else if (height<=1152)
			g_nGobunit = 2;
        g_nGobHeightInMb = 1<<g_nGobunit;
    }
#if 1 //close resync marker for H.263
    g_H263Resync = 0;
    g_nGobunit = 0;
#endif
    m4ve_reg.reg_m4ve_me_s_16 = is_sr16;
    m4ve_reg.quant_type = qtype;
    m4ve_reg.me_umv_en = isUMV;

    m4ve_reg.reg_m4ve_h263 = g_IsShortHeader;
    m4ve_reg.reg_m4ve_h263_resync = g_H263Resync;
    m4ve_reg.h263_unit = g_nGobunit;
    if (g_IsShortHeader)
        m4ve_reg.reg_m4ve_round_ctl = 0;
    else
        m4ve_reg.reg_m4ve_round_ctl = 1;

    WriteRegM4VE(0x54, m4ve_reg.reg54);
    WriteRegM4VE(0x2a, m4ve_reg.reg2a);
    WriteRegM4VE(0x79, m4ve_reg.reg79);
    WriteRegM4VE(0x00, m4ve_reg.reg00);
    WriteRegM4VE(0x15, m4ve_reg.reg15);
    WriteRegM4VE(0x31, m4ve_reg.reg31);
}

void MHal_M4VE_SetCodec(U32 codec_type)
{
    g_IsShortHeader = codec_type; //0: mpeg4, 1: h.263
    if (g_IsShortHeader)
        init_h263_gvar(Width, Height);
}
#endif
#ifdef _ENABLE_AVIMUX_
//void msAPI_M4VE_Init(int width, int height, unsigned char **outbsbf_start, unsigned char **outbsbf_end)
void msAPI_M4VE_Init(int width, int height, int BitRate, double FrameRate, unsigned char **outbsbf_start, unsigned char **outbsbf_end)
#elif 1//defined(_M4VE_T3_) //&&defined(_MIPS_PLATFORM_)
void msAPI_M4VE_Init(int width, int height, U32 gbuf_start, U32 gbuf_size)
#else
void msAPI_M4VE_Init(int width, int height)
#endif
{
	// . reset the M4VE
	//U8 mbrc = 1;
	//U32 bitrate = 1500000;//mybitrate[mybits];////6144000;
	int i=0;
	int dynamic_clock_control = 0;//0x01f;
	int blur_type = 0, qtype = 0;
    short temp=0;

    //originally assigned value globally
    //M4VE_Bitrate = 6500000;
    bitsbuf_ind = 0;
    top_data=1;
    large_diamond_limit=245;
    small_diamond_limit=245;
    gL1 = 1;
    gL2 = 2;
    Width = width; //1280;
    Height = height;

    DOUBLE_FRAME = 1; //top or bottom
#ifdef _TWO_CORES_
    MB_in_height_top = (height>>4)>>1;
#else
    MB_in_height_top = (height>>4);
#endif
    fak_start = MUX(DOUBLE_FRAME==1, 0, MB_in_height_top);
    fak_end = MUX(DOUBLE_FRAME==1, MB_in_height_top, Height>>4);

    //fak_start = 0;
    //fak_end = height/16;
//    MB_in_height_top = fak_end;
    bitsbuf_size = IBufSize;

#ifndef _FPGA_//ndef _BATCHRUN_
    is_back_ref = 0;
    u8IPlength = 10;
    u8PBlength = 0;//1;
    is_b_rec = 1;
    interlaced = 0;
    is_sr16 = 1;
//    x_rate_control_mb_enable = 1;
    switch_br_bnr = 0;
    me_dynamic_enable = 0;
#ifdef _M4VE_BIG2_
    isUMV = 0;
#else
	isUMV = 1; //0
#endif
#endif

#ifndef _MUL_OUTBITS_BUF_
    prev_read_baddr = 0;
#endif
    IPcount = 0;
    PBcount = 0;
    u32FrameCount = 0;
    src_inc = 0;
    u32HeaderBitLen = 0;
    modulo_time_base[0] = modulo_time_base[1] = 0;
    VopType = 0;
//    m4ve_quant = 0;
//    is_fixedQP = 0;
    preLen = 0;
    bitsbuf_ind = 0;
#ifdef _DIP_INPUT_
    total_inframe_ind = 0;
    inframe_encind = 0;
#endif
    bitstreamM4VE.miuAddress = bitstreamM4VE.size = 0;
    bitstreamM4VE.miuPointer = 0;
    init_HeaderSeq(u32FrameCount);
#ifdef _EN_CRC_
    InitCheckSum(&drvM4VE_adler);
#endif
#ifndef _NO_FILESYSTEM_//_AEON_PLATFORM_
#ifdef _TRACE32_CMM_
    fp_script = fopen("Big2_m4ve.cmm", "w");
#else
    fp_script = fopen("udma.script", "w");
#endif
#endif
#ifdef _M4VE_T3_
//    blur_type = 32;
    interlaced = 1;
#endif
#if defined(_M4VE_OLYMPUS_) && defined(_ARM_PLATFORM_)
    {
        //used to enable M4VE, JPG, GE, USB clock
        unsigned short *m4veclk_addr;
        //USB
        *(unsigned short *)(0xa0003e00+4) = 0x0800;
        //M4VE, JPG, GE
        m4veclk_addr = 0xa0003c00;
        *(m4veclk_addr+0x28*2) = 0x0044;
    }
#elif defined(_M4VE_BIG2_)&&defined(_MIPS_PLATFORM_) && !defined(Enable_CLKMGR)
//    *(short *)(0xBF8000C0) |= (6<<2); //ken: speed up to 156M Hz
#elif defined(_M4VE_T3_)
    //m4ve clock;
    *(unsigned short *)(0xbf200000+(0x1980+0x18)*4) = 1<<2; //2<<2
#endif
#if defined(_M4VE_BIG2_)&&defined(_MIPS_PLATFORM_)
    MApi_UTL_MemBankOffset((void *)gbuf_start, INTF_M4VE_MC_W);
    MApi_UTL_MemBankOffset((void *)gbuf_start, INTF_M4VE_SGC_W);
    MApi_UTL_MemBankOffset((void *)gbuf_start, INTF_M4VE_ME0_R);
    MApi_UTL_MemBankOffset((void *)gbuf_start, INTF_M4VE_ME1_R);
    gbuf_start = addr_log2phy(gbuf_start);
#endif
    MMAPInit(gbuf_start, gbuf_size);

#ifdef _FPGA_
    reg_scan(reg_m4ve_mask, 0x80, WriteRegM4VE, ReadRegM4VE);
#endif
    encode_state = WAIT_INPUT_FRAME; //init encode_state
    //memset(&bitstreamM4VE, 0, sizeof(MEMMAP_t));
    //FILE_OPEN_forWrite(1); move to appAPVR.c
    //FSwrite_ready = 1;

    // preReadPtr = 0 ;
    memset(&m4ve_reg, 0, sizeof(M4VE_REG));
#ifndef _NO_BVOP_
    MMAPMalloc(width*height*3/2, &bak_frame);
#endif
#if defined(FRAME_BUF3) || !defined(_NO_BVOP_)
    MMAPMalloc(width*height*3/2, &cur_frame);
#endif
    MMAPMalloc(width*height*3/2, &for_frame);
    MMAPMalloc(width*height*3/2, &rec_frame);
    MMAPMalloc(IBufSize, &bitstream_buf[0]);

    M4VE_DEBUG("frame buffer pointer: %x %x \n", (U32)&for_frame, (U32)&rec_frame);
    M4VE_DEBUG("MMAPMalloc %x %x %x %x %x %x\n", for_frame.miuAddress, (U32)for_frame.miuPointer,
        rec_frame.miuAddress, (U32)rec_frame.miuPointer, bitstream_buf[0].miuAddress, (U32)bitstream_buf[0].miuPointer);
#ifdef _ENABLE_AVIMUX_
    *outbsbf_start = (unsigned char *)bitstream_buf[0].miuPointer;//bitstream_buf[0].miuAddress;
    *outbsbf_end = (unsigned char *)(bitstream_buf[0].miuPointer+bitstream_buf[0].size);//(bitstream_buf[0].miuAddress+bitstream_buf[0].size);
#endif
#ifdef _MUL_OUTBITS_BUF_
    MMAPMalloc(IBufSize/*width*height*3/2*/, &bitstream_buf[1]);
    MMAPMalloc(IBufSize/*width*height*3/2*/, &bitstream_buf[2]);
#endif

    for( i = 0; i < sizeof(time_inc) / sizeof(time_inc[0]); i ++ ) {
        if( time_inc[i].framerate == framerate ) {
            VopTimeInc = time_inc[i].vop_time_inc;
            VopTimeIncResolution = time_inc[i].vop_time_inc_resolution;
            break;
        }
    }
    M4VE_DEBUG("Width: %x %x,    Height: %x %x,     width %x, height %x\n", (U32)&Width, Width, (U32)&Height, Height
        ,width, height);
    ReadRegM4VE(0x60, &temp);
    M4VE_DEBUG("val60: 0x%x\n", (int)temp);
    if( VopTimeInc == 0 && VopTimeIncResolution == 0 ) {
        VopTimeInc = 1;                    // default framerate 30
        VopTimeIncResolution = framerate ;
    }
    M4VE_DEBUG("frame_rate in m4ve_init %d %d\n", VopTimeInc, VopTimeIncResolution);
#ifdef _MUL_OUTBITS_BUF_
    //outbitsbuf[0].start_addr = bitstream_buf[0]->miuAddress;//0xF00000;
    //outbitsbuf[0].end_addr = outbitsbuf[0].start_addr+bitsbuf_size;
    for (i=0; i<OutBsBfNum; i++) {
        outbitsbuf[i].start_addr = bitstream_buf[i].miuAddress;
        outbitsbuf[i].end_addr = outbitsbuf[i].start_addr+bitsbuf_size;
        outbitsbuf[i].used_size = 0;
    }
#else
    outbitsbuf[0].start_addr= bitstream_buf[0].miuAddress;
    outbitsbuf[0].end_addr = bitstream_buf[0].miuAddress + bitsbuf_size;
    outbitsbuf[0].used_size = 0;
#endif

#ifdef _M4VE_BIG2_
    if (g_IsShortHeader)
        init_h263_gvar(Width, Height);
    m4ve_reg.reg_m4ve_mode_threshold = 2*16*16;//2048;
#else
    m4ve_reg.reg_m4ve_mode_threshold = 2048;
#endif //_M4VE_BIG2_
    //m4ve_reg.search_4block_range = 0;
    m4ve_reg.b_pred_mode = 1; // forward reference
#if defined(_M4VE_T3_) || defined(_M4VE_BIG2_)
#ifdef _EN_HWSGC_CRC_
    m4ve_reg.crc_mode10 = hw_crc_mode&3;
    m4ve_reg.crc_rst = 1;
#endif
    m4ve_reg.reg0a = m4ve_reg.reg0a | (x_rate_control_mb_enable<<7) | dynamic_clock_control;
#else
    m4ve_reg.reg0a = (x_rate_control_mb_enable<<7) | dynamic_clock_control;
#endif

    m4ve_reg.vlc_uidv = 1;
    m4ve_reg.sgc_ptrvld = 0;
    m4ve_reg.rst_frag_num = 1;
    //m4ve_reg.sgc_multi_obuf = 1;
    m4ve_reg.sgc_bsbfth = 4;
    m4ve_reg.reg_m4ve_pskip_off = 0; //p skip function on
    m4ve_reg.reg_m4ve_filter_en = blur_type!=0;

    m4ve_reg.reg_m4ve_me_s_16 = is_sr16;
    m4ve_reg.quant_type = qtype;
    m4ve_reg.me_umv_en = isUMV;
#ifdef _M4VE_BIG2_
    m4ve_reg.reg_m4ve_h263 = g_IsShortHeader;
    m4ve_reg.reg_m4ve_h263_resync = g_H263Resync;
    m4ve_reg.h263_unit = g_nGobunit;
    if (g_IsShortHeader)
        m4ve_reg.reg_m4ve_round_ctl = 0;
    else
        m4ve_reg.reg_m4ve_round_ctl = 1;
#endif

    switch (blur_type)
    {
    case 0:
        m4ve_reg.reg_m4ve_f3x3_mode = 0; // based on blur_type
        break;
    case 8:
        m4ve_reg.reg_m4ve_f3x3_mode = 1;
        break;
    case 16:
        m4ve_reg.reg_m4ve_f3x3_mode = 2;
        break;
    case 24:
        m4ve_reg.reg_m4ve_f3x3_mode = 3;
        break;
    case 32:
        m4ve_reg.reg_m4ve_f3x3_mode = 4;
        break;
    case 72:
        m4ve_reg.reg_m4ve_f3x3_mode = 5;
        break;
    default:
        printf("not a valid blur mode!\n");
        //fprintf(stderr, "not a valid blur mode!\n");
        ASSERT(0); // no hw support
        break;
    }
    m4ve_reg.reg_m4ve_sad_minus_value = 65;
    m4ve_reg.search_pixel_threshold1 = large_diamond_limit;
    m4ve_reg.search_pixel_threshold2 = small_diamond_limit;
/*
    if (me_dynamic_enable) {
        m4ve_reg.tcount_one_mb = 0xa8c*(720*480/(Width*Height));
//        m4ve_reg.tcount_one_mb = 0x7fff;
        m4ve_reg.time_4b = 0x621*(720*480/(Width*Height));
        m4ve_reg.time_hmb = 0xcf*(720*480/(Width*Height));
        m4ve_reg.time_h4b = 0x239*(720*480/(Width*Height));
        m4ve_reg.time_cbcr = 0x1f4*(720*480/(Width*Height));
    } else*/
    //setting for
        /*
        m4ve_reg.tcount_one_mb = 1800;
        m4ve_reg.time_4b = 615;
        m4ve_reg.time_hmb = 143;
        m4ve_reg.time_h4b = 467;
        m4ve_reg.time_cbcr = 1000;
    } else {
        m4ve_reg.tcount_one_mb = 0xa8c;
        m4ve_reg.time_4b = 0x621;
        m4ve_reg.time_hmb = 0xcf;
        m4ve_reg.time_h4b = 0x239;
        m4ve_reg.time_cbcr = 0x1f4;
    }*/
#ifdef _MUL_OUTBITS_BUF_
    m4ve_reg.rst_frag_num = 1;
    //m4ve_reg.sgc_multi_obuf = 1;// //mulit out bitstream buf enable
    m4ve_reg.sgc_bsbfth = 1;//4 // buffer threshold
#else
    m4ve_reg.rst_frag_num = 1;
    //m4ve_reg.sgc_multi_obuf = 0;//1; //mulit out bitstream buf enable
    m4ve_reg.sgc_bsbfth = 128;//4 // buffer threshold
#endif
#ifdef _M4VE_OLYMPUS_
    //reinitial again because Olympus boot code doesn't initialize variable
    me_reg.PMV_init_guess_enable = 1;
    me_reg.large_dimond_enable = 1;
    me_reg.search_4block_enable = 1;
    me_reg.half_pixel_16x16_enable = 1;
    me_reg.half_pixel_8x8_enable = 1;
	me_reg.search_4block_range = 0;
#endif
    WriteRegM4VE(0x25, m4ve_reg.reg25); //clear ptrvld
    MDrv_M4VE_SetMEMode(me_reg);
    //m4ve_reg.soft_reset = 1;
    //WriteRegM4VE(0x0, m4ve_reg.reg00);
    m4ve_reg.soft_reset = 1;
    WriteRegM4VE(0x0, m4ve_reg.reg00);
    WriteRegM4VE(0xa, m4ve_reg.reg0a);

    ReadRegM4VE(0x0, &temp);
    M4VE_DEBUG("debug: reg0x0: %04x\n", (U32)temp);
    ReadRegM4VE(0xa, &temp);
    M4VE_DEBUG("debug: reg0xa: %04x\n", (U32)temp);
#if defined(_M4VE_T3_) || defined(_M4VE_BIG2_)
#ifdef _EN_HWSGC_CRC_
    m4ve_reg.crc_rst = 0;
#endif
#endif
#ifdef _ENABLE_ISR_
    printf("attach ISR %d\n", attach_ISR);
    if (attach_ISR) {
        printf("attach ISR!!!!!!! %d %d\n", width, height);
        M4VE_ISR_Enable(1);
        attach_ISR=0;
    }
#endif
    MDrv_M4VE_Reserved();
    u32FrameCount = 0;
    u32HeaderBitLen = 0;

    m4ve_reg.reg19 = 0x07d; //0x378;
#ifndef _M4VE_OLYMPUS_
//#ifdef _M4VE_BIG2_
    WriteRegM4VE(0x15, m4ve_reg.reg15);
//#endif
    m4ve_reg.mc_wblen_th = 7; //ken new open for enabling mc wfifo to output when data reach 7 fifo
    m4ve_reg.mc_wblen_mode = 0x8; //added by Big2
    WriteRegM4VE(0x16, m4ve_reg.reg16);
#endif
    WriteRegM4VE(0x19, m4ve_reg.reg19); //clear irq
    MDrv_M4VE_SetBitstreamReadPtr(outbitsbuf[0].start_addr);
    MDrv_M4VE_SetBitstreamWritePtr(outbitsbuf[0].start_addr);

    // . set the sgc buffer.
    MDrv_M4VE_SetBitstreamBuff(outbitsbuf[0].start_addr, outbitsbuf[0].end_addr-MIU_SIZE);

#ifdef _M4VE_T3_
    m4ve_reg.sw_qtab_en = 0;
    m4ve_reg.wadr_oob_chk = 1;
    m4ve_reg.wadr_ubound0_low = LOWORD((outbitsbuf[0].end_addr+0x8)>>MIU_SHIFT);
    m4ve_reg.wadr_ubound0_high = HIWORD((outbitsbuf[0].end_addr+0x8)>>MIU_SHIFT);
    m4ve_reg.wadr_lbound0_low = LOWORD((outbitsbuf[0].start_addr-0x8)>>MIU_SHIFT);
    m4ve_reg.wadr_lbound0_high = HIWORD((outbitsbuf[0].start_addr-0x8)>>MIU_SHIFT);
    WriteRegM4VE(0xc0, m4ve_reg.regc0);
    WriteRegM4VE(0xc1, m4ve_reg.regc1);
    WriteRegM4VE(0xc2, m4ve_reg.regc2);
    WriteRegM4VE(0xc3, m4ve_reg.regc3);
    WriteRegM4VE(0xc4, m4ve_reg.regc4);
#endif
    MDrv_M4VE_SetBitstreamLineThr(height>>6);
    WriteRegM4VE(0x22, m4ve_reg.reg22);
    //MDrv_M4VE_SetBitstreamBuffThr(4);
    MDrv_M4VE_SetIrqMask(0x23);

    // . ME mode, MBRC, Filter Mode, Search Range, Threshold, UMV, SAD_Minus,
    m4ve_reg.reg_m4ve_itlc = interlaced;
    if (interlaced)
        m4ve_reg.reg_m4ve_field_dct_threshold=350;

    m4ve_reg.search_pixel_threshold1 = 245; //255
    m4ve_reg.search_pixel_threshold2 = 245; //255
    // .init bitrate controller
    if (!is_fixedQP) {
        printf("before rc_init %d\n", M4VE_Bitrate);
        rc_init(width, height, M4VE_Bitrate) ; //1 bitarte ???
    }
    if( x_rate_control_mb_enable ) {
        m4ve_reg.reg_m4ve_lower_mbqp_diff_f = 3;
        m4ve_reg.reg_m4ve_upper_mbqp_diff_f = 2;
        m4ve_reg.reg_m4ve_lower_mbqp_diff_v = 2;
        m4ve_reg.reg_m4ve_upper_mbqp_diff_v = 2;

        m4ve_reg.reg_m4ve_lower_mbqp_diff_h = 2;
        m4ve_reg.reg_m4ve_upper_mbqp_diff_h = 2;
        m4ve_reg.reg_m4ve_avgact_coeff = 1;
        m4ve_reg.reg_m4ve_var_coeff = 1;

        m4ve_reg.reg_m4ve_mbqp_rounding = 4;
        m4ve_reg.reg_m4ve_nbits_weighting = 4;
        m4ve_reg.reg_m4ve_avgact_weighting = 6;

        /* if the c-model support the _IPB_FRAMEQP_, this target_mb_bits value should be different for different vop type */
        //MDrv_M4VE_SetTargetMbBits( (int) (((double)bitrate/framerate) / ((width>>4) * (height>>4))) );
#ifdef _MIPS_PLATFORM_
        m4ve_reg.reg_m4ve_target_mb_bits = (int) (((/*double*/int)M4VE_Bitrate/framerate) / ((width>>4) * (height>>4)));
#else
        m4ve_reg.reg_m4ve_target_mb_bits = (int) (((double)M4VE_Bitrate/framerate) / ((width>>4) * (height>>4)));
#endif
#ifdef _TWO_CORES_
        m4ve_reg.reg_m4ve_target_mb_bits >>= 1;
#endif
//        REG_SET(5,0x72,0x73,0x74,0x75,0x76);
        m4ve_reg.reg_m4ve_mbr_left_bound = (width >> 4) >> 2;
        m4ve_reg.reg_m4ve_mbr_right_bound = (width>>4) - ((width>>4) >> 2);
        m4ve_reg.reg_m4ve_mbr_top_bound = (height >> 4) >> 2;
        m4ve_reg.reg_m4ve_mbr_bot_bound = (height>>4) - ((height >> 4) >> 2);

        m4ve_reg.reg_m4ve_mbr_target_bits_shift_left = 1;
        m4ve_reg.reg_m4ve_mbr_target_bits_shift_right = 2;
        m4ve_reg.reg_m4ve_mbr_i_vop_dq_inc_value = 1;
        m4ve_reg.reg_m4ve_mbr_i_vop_var_comp = 5;
        m4ve_reg.reg_m4ve_mbr_p_vop_dq_inc_value = 1; //2;

		m4ve_reg.reg_m4ve_mbr_p_vop_var_comp = 5;
		m4ve_reg.reg_m4ve_mbr_b_vop_var_comp = 5;
#if defined(_M4VE_BIG2_) || defined(_TWO_CORES_) || defined(_M4VE_T3_)
		m4ve_reg.reg_m4ve_mbr_var_fixed_value = 2;
#endif
        //REG_SET(4,0x50,0x51,0x52,0x53);
    }
    else {
        //MDrv_M4VE_EnableMBRateControl(0);
        //MDrv_M4VE_SetMbrTuneParam2();
        //MDrv_M4VE_SetMbrTuneParam();
    }
    WriteRegM4VE(0x50, m4ve_reg.reg50);
    WriteRegM4VE(0x51, m4ve_reg.reg51);
    WriteRegM4VE(0x52, m4ve_reg.reg52);
    WriteRegM4VE(0x53, m4ve_reg.reg53);
    //MDrv_M4VE_SetPskip(0); //reg 0x54
    //MDrv_M4VE_SetSearchRange(32);
    WriteRegM4VE(0x54, m4ve_reg.reg54);

    //MDrv_M4VE_SetFilterMode(2); // filter mode 16
    WriteRegM4VE(0x58, m4ve_reg.reg58);
    //MDrv_M4VE_SetSADMinus(65);
    WriteRegM4VE(0x59, m4ve_reg.reg59);

    //dynamic 4mv me
    WriteRegM4VE(0x67, m4ve_reg.reg67);
    WriteRegM4VE(0x68, m4ve_reg.reg68);
    WriteRegM4VE(0x69, m4ve_reg.reg69);
    WriteRegM4VE(0x70, m4ve_reg.reg70);
    WriteRegM4VE(0x71, m4ve_reg.reg71);


    // . set the pic size ( width, height)
	//MDrv_M4VE_SetVopSize(width,height);  //move to msAPI_M4VE_EncodeVOP()
	WriteRegM4VE(0x72, m4ve_reg.reg72);
	WriteRegM4VE(0x73, m4ve_reg.reg73);
	WriteRegM4VE(0x74, m4ve_reg.reg74);
	WriteRegM4VE(0x75, m4ve_reg.reg75);
	WriteRegM4VE(0x76, m4ve_reg.reg76);
	MDrv_M4VE_SetMiuParam();
#if defined(_M4VE_BIG2_)||defined(_TWO_CORES_) || defined(_M4VE_T3_)
	WriteRegM4VE(0x7a, m4ve_reg.reg7a); //could be useless
#if defined(_TWO_CORES_) || defined(_M4VE_T3_)
    if (top_data==0) {
        m4ve_reg.mby_fak_start = fak_end;
        m4ve_reg.mby_fak_end = height>>4;
        m4ve_reg.enc_height = (height>>4)-fak_end;
    } else {
        m4ve_reg.mby_fak_start = fak_start;
        m4ve_reg.mby_fak_end = fak_end;
        m4ve_reg.enc_height = fak_end-fak_start;
    }
#endif
    WriteRegM4VE(0x7b, m4ve_reg.reg7b);
    WriteRegM4VE(0x7c, m4ve_reg.reg7c);
    WriteRegM4VE(0x7d, m4ve_reg.reg7d);
    WriteRegM4VE(0x7e, m4ve_reg.reg7e);
    WriteRegM4VE(0x7f, m4ve_reg.reg7f);
#endif
    Sleep(400);
#ifdef _TRACE32_CMM_
    M4VE_SCRIPT(fprintf(fp_script, "B::wait.1000ms\n"));
#endif
}

#ifdef _TWO_CORES_
void msAPI_M4VE_EncodeVOP(U32 addr, MEMMAP_t *tmp_m4ve_frame_ptr, short MB_in_height_top)
#else
void msAPI_M4VE_EncodeVOP(U32 addr, MEMMAP_t *tmp_m4ve_frame_ptr)
#endif
{
	// . Vop type
	// U8 quant;
	U32 y_rec, u_rec;
	U32 y_ref, u_ref;
    int width = Width; //MDrv_M4VE_GetVopWidth();
    int height = Height; //MDrv_M4VE_GetVopHeight();
#ifdef _M4VE_TRITON_
    M4VE_DEBUG("//start Encode VOP %d\n", MsOS_GetSystemTick());
#endif
#ifndef _NO_BVOP_
    if (u32FrameCount>1 && u8PBlength && VopType!=B_VOP_NREC && VopType!=B_VOP_REC) {
        *tmp_m4ve_frame_ptr = rec_frame;
        rec_frame = for_frame;
        for_frame = bak_frame;
    }
    y_ref = MUX(is_back_ref&&(VopType==B_VOP_NREC || VopType==B_VOP_REC), bak_frame.miuAddress, for_frame.miuAddress);
#else
    y_ref = for_frame.miuAddress;
#endif
    y_rec = rec_frame.miuAddress;


    u_rec = y_rec + width * height ;
    u_ref = y_ref + width * height ;

    if (VopType!=B_VOP_NREC && VopType!=B_VOP_REC)
        m4ve_reg.pre_ip = VopType; //this is the same to CModel, but i think this code should be removed
    if (switch_br_bnr && (VopType==B_VOP))
        // 0-9 B, 10-19 Br, 20-29 B
        m4ve_reg.frame_type += (u32FrameCount/10) & 1;
    else
        m4ve_reg.frame_type = VopType + ((VopType==B_VOP)&&is_b_rec);
    WriteRegM4VE(0x0, m4ve_reg.reg00);
    m4ve_reg.search_range_left = -16;
    m4ve_reg.search_range_right = 15;
    WriteRegM4VE(0x1, m4ve_reg.reg01);
    WriteRegM4VE(0x2, m4ve_reg.reg02);
    m4ve_reg.motion_activity_th_s = gL1;
    m4ve_reg.motion_activity_th_l = gL2;
    WriteRegM4VE(0x3, m4ve_reg.reg03);
    MDrv_M4VE_SetVopSize(width,height);
    WriteRegM4VE(0x5, m4ve_reg.reg05);

    // . get the input buffer, set the re-construct & reference buffer
    MDrv_M4VE_SetRefAdr(y_ref,u_ref);
//    printf("0x%x %d %d %d %d %d %d %d %d\n", for_frame.miuPointer, ((U8 *)for_frame.miuPointer)[0], ((U8 *)for_frame.miuPointer)[1], ((U8 *)for_frame.miuPointer)[2], ((U8 *)for_frame.miuPointer)[3]
//        , ((U8 *)for_frame.miuPointer)[50], ((U8 *)for_frame.miuPointer)[51], ((U8 *)for_frame.miuPointer)[52], ((U8 *)for_frame.miuPointer)[53]);
//    printf("0x%x %d %d %d %d %d %d %d %d\n", cur_frame.miuPointer, ((U8 *)cur_frame.miuPointer)[0], ((U8 *)cur_frame.miuPointer)[1], ((U8 *)cur_frame.miuPointer)[2], ((U8 *)cur_frame.miuPointer)[3]
//        , ((U8 *)cur_frame.miuPointer)[50], ((U8 *)cur_frame.miuPointer)[51], ((U8 *)cur_frame.miuPointer)[52], ((U8 *)cur_frame.miuPointer)[53]);
    WriteRegM4VE(0x0a, m4ve_reg.reg0a);
    MDrv_M4VE_SetSrcAdr(cur_frame.miuAddress, cur_frame.miuAddress+width*height );
    MDrv_M4VE_SetRecAdr(y_rec, u_rec);
#ifdef _M4VE_T3_
    m4ve_reg.wadr_ubound1_low = LOWORD((u_rec+width*height/2+0x10)>>MIU_SHIFT);
    m4ve_reg.wadr_ubound1_high = HIWORD((u_rec+width*height/2+0x10)>>MIU_SHIFT);
    m4ve_reg.wadr_lbound1_low = LOWORD((y_rec-0x10)>>MIU_SHIFT);
    m4ve_reg.wadr_lbound1_high = HIWORD((y_rec-0x10)>>MIU_SHIFT);
    WriteRegM4VE(0xc5, m4ve_reg.regc5);
    WriteRegM4VE(0xc6, m4ve_reg.regc6);
    WriteRegM4VE(0xc7, m4ve_reg.regc7);
    WriteRegM4VE(0xc8, m4ve_reg.regc8);
    m4ve_reg.crc_mode2 = hw_crc_mode>>2;
#endif

    WriteRegM4VE(0x15, m4ve_reg.reg15);
#if defined(_M4VE_BIG2_) || defined(_M4VE_T3_)
    WriteRegM4VE(0x31, m4ve_reg.reg31);
#endif
    WriteRegM4VE(0x25, m4ve_reg.reg25);
    fprintf(fp_script, "//Encode VOP: %d\n", VopType);
    // . Target MB Bits
    // . Qscale
#ifdef CVBR_RATE_CONTROL
    m4ve_quant = cvbr_rc_vop_before(VopType);
#elif defined(X_RATE_CONTROL)
    if (!is_fixedQP)
        m4ve_quant = rc_vop_before(VopType);
#endif
    fprintf(fp_script, "//quant=%d %d\n", m4ve_quant, is_fixedQP);

    MHal_M4VE_SetQscale(m4ve_quant, is_fixedQP);
    WriteRegM4VE(0x75, m4ve_reg.reg75);
    m4ve_reg.me_umv_en = isUMV;
    m4ve_reg.umvc_th = 2;
    m4ve_reg.umvc_blen = 16;
    WriteRegM4VE(0x79, m4ve_reg.reg79);


    // . Encode go....
#if defined(_AEON_PLATFORM_) && !defined(_PARA_FILEOUT_) && defined(_M4VE_TRITON_)
    if (u32FrameCount!=0 && /*bitstreamM4VE_index*/bitstreamM4VE.size==0)
        wait_FILE_WRITE_READY();
        //printf("wait USB write finished before MDrv_M4VE_start()\n");//wait 8051 file write finish
#endif
    MDrv_M4VE_start();
#ifdef _M4VE_T3_
    if(VopType==I_VOP)
#else
    if( u32FrameCount == 0 && DOUBLE_FRAME!=2
#ifdef _M4VE_BIG2_
        && g_IsShortHeader==0
#endif
        )
#endif
    {
        init_HeaderSeq(u32FrameCount);
        MDrv_M4VE_BitstreamPutVOLHeader(
            /*MDrv_M4VE_GetVopWidth()*/width,
            /*MDrv_M4VE_GetVopHeight()*/height,
            MDrv_M4VE_GetDctMode(),
            MDrv_M4VE_GetQType(), u32FrameCount
            );
    }
    // . Put header
    if (DOUBLE_FRAME!=2)
        MDrv_M4VE_BitstreamPutVOPHeader(
            MDrv_M4VE_GetVopType(),
            /*MDrv_M4VE_GetQscale()*/m4ve_quant,
            MDrv_M4VE_GetDctMode(),
            u32FrameCount ,
            /*MDrv_M4VE_GetSearchRange()*/
            MUX(is_sr16, 16, 32)
#ifdef _M4VE_BIG2_
            , g_IsShortHeader, g_nSourceFormat
#endif
            );
#ifdef _TWO_CORES_
    else
        MDrv_M4VE_BitstreamPutResyncHeader(
            MB_in_height_top, width/16, height/16, m4ve_quant, MUX(is_sr16, 1, 2), VopType);
#endif

    MDrv_M4VE_PutHeaderEnd();
#ifdef _MUL_OUTBITS_BUF_
    m4ve_reg.sgc_ptrvld = 1;
    WriteRegM4VE(0x25, m4ve_reg.reg25);
#endif
}
#ifdef _PARA_FILEOUT_
int msAPI_M4VE_Finish(U32 *bitsAddr, U32 *bitsSize)
#else
int msAPI_M4VE_Finish(void)
#endif
{
#if defined(_AEON_PLATFORM_) && defined(_M4VE_TRITON_)
    if (bitstreamM4VE.size) {
        M4VE_DEBUG("Finish Encoding bitstream, write rest bitstream to file %d\n", bitstreamM4VE.size);
#ifdef _ENABLE_AVIMUX_
        *bitsAddr = (U32)bitstreamM4VE.miuPointer;
#else
        *bitsAddr = bitstreamM4VE.miuAddress;
#endif
        *bitsSize = bitstreamM4VE.size;
        //FILE_write(bitstreamM4VE.miuAddress, bitstreamM4VE_index);
        //wait_FILE_WRITE_READY();
    }
#ifndef _PARA_FILEOUT_
#ifndef _DIP_INPUT_
    FILE_CLOSE_forRead();
#endif
    FILE_CLOSE_forWrite();
#endif //_PARA_FILEOUT_
#elif defined(_MIPS_PLATFORM_) && defined(_M4VE_T3_)
//    disable_irq(E_IRQ_M4VE);
    free_irq(E_IRQ_M4VE, NULL);
    attach_ISR=1;
    *(unsigned short *)(0xbf200000+(0x1980+0x18)*4) = 1;//disable M4VE clock
    M4VE_DEBUG("finish encoding in drvM4VE.c\n");
#endif  //_AEON_PLATFORM_ _M4VE_TRITON_
#if defined(_MIPS_PLATFORM_)&&defined(_M4VE_BIG2_)&& !defined(Enable_CLKMGR)
    attach_ISR=1;
    M4VE_ISR_Disable();
    *(short *)(0xBF8000C0) |= (0<<2); // speed down to 15M Hz
#endif
    return 0;
}

int msAPI_M4VE_EncodeVOP_End(MEMMAP_t *tmp_m4ve_frame_ptr
#if defined(_DIP_INPUT_) && defined(_PARA_FILEOUT_)
                             , U32 inframe_ind, U32 *encframe_ind
                             , U32 *bitsAddr, U32 *bitsSize
#endif
                             )
{
    MEMMAP_t temp;
#if defined(_AEON_PLATFORM_) && defined(_M4VE_TRITON_)
#ifdef _PARA_FILEOUT_
#ifndef _ENABLE_AVIMUX_
    if (bitstreamM4VE.size>=256*1024)
#endif
    {
        M4VE_DEBUG("write bitstream addr:0x%x  size:%d\n", bitstreamM4VE.miuAddress, bitstreamM4VE.size);
        //printf("EncodeVOP_End: %x %d %d\n", bitstreamM4VE.miuPointer, bitstreamM4VE.miuPointer[0], bitstreamM4VE.miuPointer[1]);
        //debug_tmp = bitstreamM4VE.miuPointer;
#ifdef _ENABLE_AVIMUX_
        *bitsAddr = (U32)bitstreamM4VE.miuPointer;//bitstreamM4VE.miuAddress;
#else
        *bitsAddr = bitstreamM4VE.miuAddress;
#endif
        //*bitsSize = bitstreamM4VE_index;
        *bitsSize = bitstreamM4VE.size;
        bitstreamM4VE.miuPointer = bitstreamM4VE.miuAddress = 0;
        bitstreamM4VE.size = 0;
//        memset(&bitstreamM4VE, 0, sizeof(MEMMAP_t));
    }
#else
    if ((bitstreamM4VE.size - /*bitstreamM4VE_index*/bitstreamM4VE.size)<128*1024) {
        //printf("write bitstream for_frame: %x %d\n", cur_frame.miuAddress, Width*Height*3/2);
        //FILE_write(cur_frame.miuAddress, Width*Height*3/2);
        M4VE_DEBUG("write bitstream size: %d\n", bitstreamM4VE.size);
        FILE_write(bitstreamM4VE.miuAddress, /*bitstreamM4VE_index*/bitstreamM4VE.size);
        bitstreamM4VE.size = 0;
        //bitstreamM4VE_index = 0;
    }
#endif
#endif
#ifdef _NO_BVOP_
    temp = for_frame;
    for_frame = rec_frame ;
    rec_frame = temp;
#else
    if (u32FrameCount==0) {
        temp = for_frame;
        for_frame = rec_frame;
        rec_frame = bak_frame;
        bak_frame = temp;
    } else if (u32FrameCount==1) {
        if (u8PBlength) {
            temp = rec_frame;
            rec_frame = bak_frame;
            bak_frame = temp;
        } else {
            temp = rec_frame;
            rec_frame = for_frame;
            for_frame = temp;
        }
    } else {
        if (VopType == B_VOP_NREC || VopType == B_VOP_REC) {
            //do nothing;
        } else if (u8PBlength) {
            //int temp = reconstruct_addr;
            //reconstruct_addr = forward_addr;
            //forward_addr = bakward_addr;
            bak_frame = rec_frame;
            rec_frame = *tmp_m4ve_frame_ptr;
        } else {
            temp = rec_frame;
            rec_frame = for_frame;
            for_frame = temp;
        }
    }
#endif
#ifdef _DIP_INPUT_
    //for DIP clear bit
    if (VopType==B_VOP  || VopType==B_VOP_REC)
        *encframe_ind = src_inframe_buf[inframe_encind].src_inframe_ind;
    else
        *encframe_ind = inframe_ind%6; //max DIP buffer
    inframe_encind = MUX(inframe_encind==(MAX_SRC_INFRAME_NUM-1), 0, inframe_encind+1);
    //printf("Finish encoding DIP frame index #%d\n", *encframe_ind);
#endif
    u32FrameCount++ ;
    return 0;
}

#if (defined(_FPGA_) || defined(_TRACE32_CMM_)) && !defined(_YUVGEN_)
BOOL PutFileFunc(char *full_name, DWORD start)
{
    int iLen;//int type,iLen;

    iLen = GetFileLength(full_name);
    if( (iLen % 8 /*MIU_ALIGNMENT*/)!=0) {
        fprintf(fp_script, "> fail, File length not alignment!!! length: %d\n",iLen);
        return FALSE;
    }

    if(DMAPutFile(full_name, start, 0, Script_Buffer)==FALSE) {
        fprintf(fp_script, "> fail, putfile fail\n");
        return FALSE;
    } else {
        fprintf(fp_script, "//> ok, Succeed\n");
    }
    Sleep(400);
#ifdef _TRACE32_CMM_
    fprintf(fp_script, "B::wait.1000ms\n");
#endif
    /*{
        char temp_name[100] = "./test.yuv";
        DMAGetFile(temp_name, start+MEM_LOGMAPPHY, Width*Height*3/2, 0, Script_Buffer);
    }
    Sleep(400);
    */
#ifdef _TRACE32_CMM_
    fprintf(fp_script, "B::wait.1000ms\n");
#endif
    fprintf(fp_script, "//Putfile > ok\n");

    return TRUE;
}
#endif

void GetYCTile(char *yfile_name, char *cfile_name, int width, int height, volatile U8 *yuvbuffer)
{
#if (defined(_FPGA_) || defined(_TRACE32_CMM_)) && !defined(_YUVGEN_)
    FILE *yfile;
    FILE *cfile;
    int iLen;
    PutFileFunc(yfile_name, cur_frame.miuAddress);
    PutFileFunc(cfile_name, cur_frame.miuAddress+width*height);
#elif defined(_AEON_PLATFORM_)
//    int i, j;
    printf("width %d height %d file read to %x\n", width, height, cur_frame.miuAddress);
    FILE_read(cur_frame.miuAddress, width*height*3/2);
    //for (i=0; i<width; i++)
      //  printf("%x ", cur_frame.miuPointer[i]);
#endif
}
#if defined(_NO_WAIT_FRAMEDONE_) && defined(_M4VE_TRITON_)
int M4VE_getframe(U32 inframe_ind, U32 *encframe_ind, U32 *bitsAddr, U32 *bitsSize, U8 *encode_VOP_type)
{
    int is_framedone;
    U16 temp, temp1, temp2, temp3;
    char tmpname[30];
    char realname[30];
#ifdef _ENABLE_ISR_
    while(encode_state == WAIT_FRAME_DONE) {
        //printf("encode_state: %d\n", encode_state);
    };
#else
    is_framedone = MDrv_M4VE_CheckEncodeDone(tmpname, realname);
    //printf("encode_state: %d %d\n", encode_state, is_framedone);
    /*
    if (encode_state == WAIT_INPUT_FRAME) {
        //This means M4VE has output frame bitstream, and wait new frame to encode.
        return 1;
    } else*/ if (is_framedone==0) {
        //This means M4VE encoding a frame, and not frame done
        return 0;
    }
#endif
    msAPI_M4VE_EncodeVOP_End(&m4ve_frame_ptr_tmp, inframe_ind, encframe_ind
#if defined(_DIP_INPUT_) && defined(_PARA_FILEOUT_)
            , bitsAddr, bitsSize
#endif
        );
    *encode_VOP_type = MDrv_M4VE_GetVopType();
    encode_state = WAIT_INPUT_FRAME;
    //frame done, and get bitstream
    //printf("M4VE_getframe End Time: %d  %d\n", MsOS_GetSystemTick(), inframe_ind);
    return 1;
}
#endif
#if defined(_DIP_INPUT_) || defined(_FRAMEPTR_IN_)
#ifdef _PARA_FILEOUT_
int msAPI_M4VE_EnOneFrm(U32 inframe_ind, U32 inputframe_addr, U32 *encframe_ind
#ifndef _NO_WAIT_FRAMEDONE_
                        , U8 *encode_VOP_type
                        , U32 *bitsAddr, U32 *bitsSize
#endif
                        )
#else
int msAPI_M4VE_EnOneFrm(U32 inframe_ind, U32 inputframe_addr, U32 *encframe_ind)
#endif
#else
int msAPI_M4VE_EnOneFrm(U32 inframe_ind, U8 *input_fname, U8 *realname, U8 *tmpname
#ifdef _COMP_INRUN_
                        , FILE* fp_golden
#endif
                        )
#endif //_DIP_INPUT_
{
#ifndef _DIP_INPUT_
    int refidx=0;
#ifndef _NO_FILESYSTEM_
    char yfile_name[100];
    char cfile_name[100];
#endif
#elif !defined(_ENABLE_ISR_)
#ifndef _NO_WAIT_FRAMEDONE_
    char tmpname[30];
    char realname[30];
#endif
#endif

    encode_state = WRITE_M4VE_REG;
    VopType = msAPI_M4VE_VOPMode(); //decide this VOP type
    //printf("u32FrameCount %d  VOPType %d\n", u32FrameCount, VopType);
#ifdef _DIP_INPUT_
#if defined(_PARA_FILEOUT_) && !defined(_NO_WAIT_FRAMEDONE_)
    *encode_VOP_type = VopType;
#endif
    if (VopType==B_VOP) {
        src_inframe_buf[total_inframe_ind].src_inframe_addr = inputframe_addr;
        src_inframe_buf[total_inframe_ind].src_inframe_ind = inframe_ind;
        total_inframe_ind = MUX(total_inframe_ind==(MAX_SRC_INFRAME_NUM-1), 0, total_inframe_ind+1);
        M4VE_DEBUG("input one B frame address: %d %d %x\n", u32FrameCount, total_inframe_ind,
            src_inframe_buf[total_inframe_ind].src_inframe_addr);
        if (u32FrameCount<=u8PBlength) { //don't encode any frame before first few B frames until first P is encoded.
            u32FrameCount++;
            return 1; //not encode
        }
        cur_frame.miuAddress = src_inframe_buf[inframe_encind].src_inframe_addr;
        cur_frame.miuPointer = (U8*)addr_phy2log(src_inframe_buf[inframe_encind].src_inframe_addr);
    } else {
        cur_frame.miuAddress = inputframe_addr;
        cur_frame.miuPointer = (U8*)addr_phy2log(inputframe_addr);
    }
    M4VE_DEBUG("encode one frame address: %x\n", cur_frame.miuAddress);
#else
    if (inframe_ind>0)
        refidx = MUX(VopType==B_VOP, inframe_ind-1, inframe_ind+u8PBlength);
//#ifndef _NO_FILESYSTEM_
//    sprintf(yfile_name, "%s%c%s%d%s", input_fname, slash, "udma_y", refidx, ".dat");
//    sprintf(cfile_name, "%s%c%s%d%s", input_fname, slash, "udma_c", refidx, ".dat");
//#endif
#endif
    M4VE_SCRIPT(fprintf(fp_script, "//start to encode frame #%d\n", inframe_ind));
    M4VE_DEBUG("start to encode frame #%d\n", inframe_ind);
#ifdef _M4VE_BIG2_
    cur_frame.miuAddress = addr_log2phy(inputframe_addr); //@RL
    cur_frame.miuPointer = (U8*)inputframe_addr;
#else
    cur_frame.miuAddress = inputframe_addr;
    cur_frame.miuPointer = (U8*)addr_phy2log(inputframe_addr);
#endif
    M4VE_DEBUG("cur_frame: 0x%x  0x%x\n", cur_frame.miuAddress, cur_frame.miuPointer);
#ifdef _BUF_COSTDOWN_
    rec_frame.miuAddress = cur_frame.miuAddress;
    rec_frame.miuPointer = cur_frame.miuPointer;
#endif
#if defined(_TWO_CORES_)
    msAPI_M4VE_EncodeVOP(yuvBuffer, &m4ve_frame_ptr_tmp, MB_in_height_top);
#else
    msAPI_M4VE_EncodeVOP(yuvBuffer, &m4ve_frame_ptr_tmp);
#endif

#ifndef _NO_WAIT_FRAMEDONE_
#ifdef _ENABLE_ISR_
    while(encode_state == WAIT_FRAME_DONE) {
        short temp;
        ReadRegM4VE(0x60, &temp);
        M4VE_DEBUG("wait frame done in msAPI_M4VE_EnOneFrm(), encode_state: %d %d\n", encode_state, temp);
    };
#else
    MDrv_M4VE_CheckEncodeDone(tmpname, realname
#ifdef _COMP_INRUN_
        , fp_golden
#endif
        );
#endif
    msAPI_M4VE_EncodeVOP_End(&m4ve_frame_ptr_tmp
#if defined(_DIP_INPUT_) && defined(_PARA_FILEOUT_)
        , inframe_ind, encframe_ind
        , bitsAddr, bitsSize
#endif
        );
#endif

    //u32FrameCount++ ;   move to the end of msAPI_M4VE_EncodeVOP_End()
    return 0;
}

extern int batchrun;
#if 0//ndef _DIP_INPUT_
void M4VE_Test_AP(U32 frmNum, char *input_fname, char *realname, char *tmpname
#ifdef _COMP_INRUN_
                  , char *golden_file
#endif
                  )
{
    // U32 frmNum = 2;
    U32 i =0;
    FILE *fp_golden;
#ifdef _FPGA_
	if (batchrun==0)
		FPGA_Connect();
#endif
#ifdef _COMP_INRUN_
    fp_golden = fopen(golden_file, "rb");
#endif
    //printf("Width: %x %x,    Height: %x %x\n", &Width, Width, &Height, Height);
    //printf("m4vereg %x\n", &m4ve_reg);
#ifndef _AEON_PLATFORM_
    if (DOUBLE_FRAME==2)
        strcat(realname, "dfB");
#endif
#ifdef _AEON_PLATFORM_
#ifndef _YUVGEN_
    FILE_OPEN_forRead(1);
#endif
    //FILE_OPEN_forWrite(1);
#elif !defined(_NO_FILESYSTEM_)
    {
        //refresh output file
        FILE *tmpf;
        tmpf = fopen(realname, "wb");
        printf("open realname: %s\n", realname);
        fclose(tmpf);
        printf("close realname: %s\n", realname);
    }
#endif
    printf("start msAPI_M4VE_Init\n");

#if !(defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_))
    msAPI_M4VE_Init(Width, Height);
#endif
	//fprintf(fp_script, "//Begin Tick:%d\n", MsOS_GetSystemTick());
    //MB_in_height_top = MUX(DOUBLE_FRAME==2, fak_start, fak_end-fak_start);

    for(i = 0; i < frmNum; i ++)
    {
        U32 inframe_addr, enc_frame_ind;
        M4VE_DEBUG(printf("start to encode frame #%d %d\n", i, frmNum));
        msAPI_M4VE_EnOneFrm(i
#ifdef _FRAMEPTR_IN_
            , inframe_addr, &enc_frame_ind
#else
            , input_fname, realname, tmpname
#ifdef _COMP_INRUN_
            , fp_golden
#endif
#endif
            );
    }

    fprintf(fp_script, "//%d Frames Encoded.\n",i);
#ifndef _M4VE_OLYMPUS_
    //fprintf(fp_script, "//End Tick:%d\n",MsOS_GetSystemTick());
#endif
#ifdef _AEON_PLATFORM_
    msAPI_M4VE_Finish();
    //FILE_CLOSE_forWrite();
#elif !defined(_NO_FILESYSTEM_)
    fclose(fp_script);
#endif
#ifdef _COMP_INRUN_
    fclose(fp_golden);
#endif
}
#endif


#undef MDRV_M4VE_C

