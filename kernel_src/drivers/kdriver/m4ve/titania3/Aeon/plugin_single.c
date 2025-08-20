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

#include "drvM4VE.h"
#ifdef _WIN32
#include <stdio.h>
#elif defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
#include <linux/string.h>
#include <linux/memory.h>
#elif defined(_M4VE_BIG2_)&&defined(_MIPS_PLATFORM_)
#include "stdarg.h"
#include "string.h"
#include "shellcfg.h"
#endif

//#include <limits.h>
// #include <momusys.h>

#include "xvid.h"
// #include "vm_enc_defs.h"
// #include "../image/image.h"
// #include "global.h"

// #define I_VOP		0		/* vop coding modes */
// #define P_VOP		1
// #define B_VOP		2

/*
extern int UTL_printf(const char *pFmt, ...);
#define printf UTL_printf
*/


#define DEFAULT_INITIAL_QUANTIZER 4

#define DEFAULT_BITRATE 700000	/* 700kbps */
#define DEFAULT_DELAY_FACTOR 16
#define DEFAULT_AVERAGING_PERIOD 100
#define DEFAULT_BUFFER 100

#if (_FIXEDPTR_ ==1)
	#define FIXPTR_SHIFT 10
	#define SFT_2ND 0
	#define SFT_3RD 2
	#define SFT_1ST	(FIXPTR_SHIFT-SFT_2ND-SFT_3RD)
	#define EXSFT 6
	#define FIXPTR_INT32 (0.06452*(1<<FIXPTR_SHIFT))
#elif (_FIXEDPTR_ ==2)
	#define FIXPTR_SFT64 28
	#define SFT64_TIME 14
	#define SFT64_2ND 4
	#define SFT64_3RD 2
	#define SFT64_1ST	(SFT64_TIME-SFT64_2ND-SFT64_3RD)
	#define EXSFT64 6
	#define FIXPTR_INT64 (0.06452*(1<<FIXPTR_SFT64))
#endif

#define _OUTPUTQUANT_ 0//write out QUANT value
#if (_OUTPUTQUANT_ ==1)
	#if (_FIXEDPTR_ ==1)
		FILE *fquant32;
	#elif (_FIXEDPTR_ ==2)
		FILE *fquant64;
	#elif (_FIXEDPTR_ ==0)
		FILE *fquant;
	#endif
#endif


#if 0
inline void dump(rc_single_t * rc)
{
/*
	printf("time32:%d", rc->time32);
	printf("reaction_delay_factor32:%d", rc->reaction_delay_factor32);
	printf("target_framesize32:%d", rc->target_framesize32);
	printf("sequence_quality32:%d", rc->sequence_quality32);
	printf("avg_framesize32:%d", rc->avg_framesize32);
	printf("\n");
*/
/*
	printf("rcrcrcrccccccccccccccccccccccccccccccccccccccccccc\n");
	fpPrintf(rc->time,10);
	fpPrintf(rc->reaction_delay_factor,10);
	printf("byte_per_sec: %d\n",rc->bytes_per_sec);
	fpPrintf(rc->target_framesize,10);
	fpPrintf(rc->sequence_quality,10);
	fpPrintf(rc->avg_framesize,10);
	printf("rtn_quant:%d\n",rc->rtn_quant);
	printf("buff:%d\n",rc->buffer);
	printf("rcrcrcrccccccccccccccccccccccccccccccccccccccccend\n");
*/
}
#endif

int get_initial_quant(unsigned int bitrate)
{

#if defined(DEFAULT_INITIAL_QUANTIZER)
	return (DEFAULT_INITIAL_QUANTIZER);
#else
	int i;

	const unsigned int bitrate_quant[31] = {
		UINT_MAX
	};

	for (i = 30; i >= 0; i--) {
		if (bitrate > bitrate_quant[i])
			continue;
	}

	return (i + 1);
#endif
}

#ifdef _IPB_FRAMEQP_
static double gop_size;
static int scene_change=0;
int B_VOP_INSERT=0;
#endif

int rc_single_create(xvid_plg_create_t * create, rc_single_t *rc)
{
	int i;
	int default_factor;

	/*
	 * single needs to caclculate the average frame size. In order to do that,
	 * we really need valid fps
	 */
    M4VE_DEBUG("rc_single_create %d\n", create->fincr);
	if (create->fincr == 0) {
		return -1;
	}
	/* Constants */
	if (create->bitrate > DEFAULT_BITRATE)
		default_factor=(create->bitrate/DEFAULT_BITRATE);
	else
		default_factor=1;
	dump(rc);
	M4VE_DEBUG("bitrate:%d\n",create->bitrate);
	rc->bytes_per_sec =	(create->bitrate > 0) ? create->bitrate / 8 : DEFAULT_BITRATE / 8;
	dump(rc);
	rc->averaging_period = (create->averaging_period > 0) ? create->averaging_period : DEFAULT_AVERAGING_PERIOD*default_factor;
	rc->buffer = (create->buffer > 0) ? create->buffer : DEFAULT_BUFFER*default_factor;

	rc->total_size = 0;

#if (_FIXEDPTR_ ==1)
	rc->time32 = 0;

	rc->target_framesize32 = rc->bytes_per_sec / (create->fbase / create->fincr);
	rc->reaction_delay_factor32 =(create->reaction_delay_factor > 0) ? create->reaction_delay_factor : DEFAULT_DELAY_FACTOR*default_factor;

	rc->rtn_quant= get_initial_quant(create->bitrate);

	for (i = 0; i < 31; i++)
		rc->quant_error32[i] = 0;

	rc->sequence_quality32 =((DEFAULT_INITIAL_QUANTIZER>>1)<<FIXPTR_SHIFT) / (rc->rtn_quant);
	rc->avg_framesize32 = rc->target_framesize32;
    M4VE_DEBUG("avg_framesize32 %d %d %d %d\n", rc->avg_framesize32, rc->bytes_per_sec, create->fbase, create->fincr);
#elif (_FIXEDPTR_ ==2)
	rc->time64 = 0;

	rc->target_framesize64 = rc->bytes_per_sec / (create->fbase / create->fincr);
	rc->reaction_delay_factor64 =(create->reaction_delay_factor > 0) ? create->reaction_delay_factor : DEFAULT_DELAY_FACTOR*default_factor;

	rc->rtn_quant= get_initial_quant(create->bitrate);

	for (i = 0; i < 31; i++)
		rc->quant_error64[i] = 0;

	rc->sequence_quality64 =((DEFAULT_INITIAL_QUANTIZER>>1)<<FIXPTR_SFT64) / (rc->rtn_quant);
	rc->avg_framesize64 = rc->target_framesize64;
#elif (_FIXEDPTR_ ==0)
	rc->time = 0;

	dump(rc);
	rc->target_framesize =(double) rc->bytes_per_sec / ((double) create->fbase / create->fincr);
	dump(rc);
	rc->reaction_delay_factor =	(create->reaction_delay_factor > 0) ? create->reaction_delay_factor : DEFAULT_DELAY_FACTOR*default_factor;

	rc->rtn_quant = get_initial_quant(create->bitrate);

	/* Reset quant error accumulators */
	for (i = 0; i < 31; i++)
		rc->quant_error[i] = 0.0;

	/* Last bunch of variables */
	rc->sequence_quality = ((double)(DEFAULT_INITIAL_QUANTIZER>>1)) / (double) rc->rtn_quant;
	rc->avg_framesize = rc->target_framesize;
#ifdef _IPB_FRAMEQP_
	gop_size=(double) rc->bytes_per_sec*(double)gBufMgr.intra_period / ((double) create->fbase / create->fincr);
#endif

	/* A bit of debug info */
/*	fprintf(stderr, "bytes_per_sec: %i\n", rc->bytes_per_sec);
	fprintf(stderr, "frame rate   : %f\n", (double) create->fbase / create->fincr);
	fprintf(stderr, "target_framesize: %f\n", rc->target_framesize);
*/
#endif


#if (_OUTPUTQUANT_ ==1)
	#if (_FIXEDPTR_ ==1)
		fquant32=my_fopen("quant32.dat","wb");
	#elif (_FIXEDPTR_ ==2)
		fquant64=my_fopen("quant64.dat","wb");
	#elif (_FIXEDPTR_ ==0)
		fquant=my_fopen("quant.dat","wb");
	#endif
#endif
	dump(rc);
	return (0);
}


int rc_single_destroy(rc_single_t * rc)
{
#if (_OUTPUTQUANT_ ==1)
	#if (_FIXEDPTR_ ==1)
		if(fquant32)
			fclose(fquant32);
	#elif (_FIXEDPTR_ ==2)
		if(fquant64)
			fclose(fquant64);
	#elif (_FIXEDPTR_ ==0)
		if(fquant)
			fclose(fquant);
	#endif
#endif
//	free(rc);
	return (0);
}

int rc_single_before(rc_single_t * rc, xvid_plg_data_t * data)
{
    M4VE_DEBUG("in rc_single_before %d\n", data->quant);
	if (data->quant <= 0)
	{
		int q = rc->rtn_quant;
		/* limit to min/max range
		   we don't know frame type of the next frame, so we just use
		   P-VOP's range... */
		if (q > data->max_quant[XVID_TYPE_PVOP-1])
		{
			q = data->max_quant[XVID_TYPE_PVOP-1];
		}
		else if (q < data->min_quant[XVID_TYPE_PVOP-1])
		{
			q = data->min_quant[XVID_TYPE_PVOP-1];
		}
		data->quant = q;
	}

#if !defined(_TUNE_IVOP_) && defined(_IPB_FRAMEQP_)
	if (data->type == P_VOP)
	{
		scene_change=0;
		if(B_VOP_INSERT)
		{
			for (i = 1; i <= gBufMgr.b_vop_insert; i++)
			{
				for (j = 0; j < gBufMgr.buf_num; j++)
				{
					if(gBufMgr.Buf[j].order==(gBufMgr.Buf[ gBufMgr.curr_enc].order-i))
					{
						if ((gBufMgr.Buf[j].sad/(data->mb_width*data->mb_height))>=25)
							scene_change=2;
						else if ((gBufMgr.Buf[j].sad/(data->mb_width*data->mb_height))>17 && scene_change==0)
							scene_change=1;
						break;
					}
				}

			}
		}
		else
		{
			if ((gBufMgr.Buf[ gBufMgr.curr_enc].sad/(data->mb_width*data->mb_height))>=25)
				scene_change=2;
			else if ((gBufMgr.Buf[ gBufMgr.curr_enc].sad/(data->mb_width*data->mb_height))>17 && scene_change==0)
				scene_change=1;
		}

		rc->target_framesize= gop_size*P_VOP_BITS_WEIGHT
			/( I_VOP_BITS_WEIGHT+P_VOP_BITS_WEIGHT*(gBufMgr.intra_period/(B_VOP_INSERT+1)-1)+B_VOP_INSERT*B_VOP_BITS_WEIGHT*(gBufMgr.intra_period/(B_VOP_INSERT+1)) );

	}
	if (data->type == B_VOP)
	{
		rc->target_framesize= gop_size*B_VOP_BITS_WEIGHT
			/( I_VOP_BITS_WEIGHT+P_VOP_BITS_WEIGHT*(gBufMgr.intra_period/(B_VOP_INSERT+1)-1)+B_VOP_INSERT*B_VOP_BITS_WEIGHT*(gBufMgr.intra_period/(B_VOP_INSERT+1)) );
	}
	else if (data->type == I_VOP)
	{
		rc->target_framesize= gop_size*I_VOP_BITS_WEIGHT
			/( I_VOP_BITS_WEIGHT+P_VOP_BITS_WEIGHT*(gBufMgr.intra_period/(B_VOP_INSERT+1)-1)+B_VOP_INSERT*B_VOP_BITS_WEIGHT*(gBufMgr.intra_period/(B_VOP_INSERT+1)) );
	}

#endif
	//if (data->type==0)
	//	data->quant=DEFAULT_INITIAL_QUANTIZER;
#if !defined(_TUNE_IVOP_) && defined(_IPB_FRAMEQP_)
	if(data->type != I_VOP)
	{
		if(scene_change==2)
		{
			//data->quant=data->quant-1;
			rc->target_framesize=rc->target_framesize*(P_VOP_BITS_WEIGHT+2)/P_VOP_BITS_WEIGHT;
			DPRINTF(DEBUG_MSG,"\nscene_change=22222222222222222222");
		}
		else if(scene_change==1)
		{
			//data->quant=data->quant-1;
			rc->target_framesize=rc->target_framesize*(P_VOP_BITS_WEIGHT+1)/P_VOP_BITS_WEIGHT;
			DPRINTF(DEBUG_MSG,"\nscene_change=11111111111111111111");
		}
	} else if (data->type == I_VOP)
	{
		if ((gBufMgr.Buf[ gBufMgr.curr_enc].sad/(data->mb_width*data->mb_height))<3)
		{
			//data->quant=data->quant-1;
			rc->target_framesize= gop_size*P_VOP_BITS_WEIGHT
				/( I_VOP_BITS_WEIGHT+P_VOP_BITS_WEIGHT*(gBufMgr.intra_period/(B_VOP_INSERT+1)-1)+B_VOP_INSERT*B_VOP_BITS_WEIGHT*(gBufMgr.intra_period/(B_VOP_INSERT+1)) );
		}
	}
	DPRINTF(DEBUG_MSG,"\nVOP=%d	SAD=%d",data->type,(Int)(gBufMgr.Buf[ gBufMgr.curr_enc].sad/(data->mb_width*data->mb_height)));
	DPRINTF(DEBUG_MSG,"\nrc->target_framesize=%f  intraperiod=%d  gop_size=%f  P=%d	B=%d",rc->target_framesize,(Int)gBufMgr.intra_period,gop_size,(Int)(gBufMgr.intra_period/(B_VOP_INSERT+1)-1),(Int)(gBufMgr.intra_period/(B_VOP_INSERT+1)));
#else
/*  //unmark these codes if we want to do rate-control like CModel
	if ((gBufMgr.Buf[ gBufMgr.curr_enc].sad/(data->mb_width*data->mb_height))>=25)
		data->quant=data->quant-2;
	else if ((gBufMgr.Buf[ gBufMgr.curr_enc].sad/(data->mb_width*data->mb_height))>17)
		data->quant=data->quant-1;
*/
#endif
	if (data->quant<data->min_quant[XVID_TYPE_PVOP-1])
		data->quant=data->min_quant[XVID_TYPE_PVOP-1];

	dump(rc);
	return data->quant;
}


int rc_single_after(rc_single_t * rc, xvid_plg_data_t * data)
{
//	INT64 deviation;
	int deviation;
	int rtn_quant;

#if (_FIXEDPTR_ ==1)
	int overflow32;
//	int averaging_period32;
	int reaction_delay_factor32;
	int quality_scale32;
	int base_quality32;
	int target_quality32;
	int temp_value=FIXPTR_INT32;
//	float comp_temp = 0.1*(1<<FIXPTR_SHIFT);
	int comp_temp = (1<<FIXPTR_SHIFT)/10;
#elif (_FIXEDPTR_ ==2)
	INT64 overflow64;
//	INT64 averaging_period64;
	INT64 reaction_delay_factor64;
	INT64 quality_scale64;
	INT64 base_quality64;
	INT64 target_quality64;
	INT64 temp_value64=FIXPTR_INT64;
	INT64 value64=(DEFAULT_INITIAL_QUANTIZER>>1);
//	double comp_temp64 =(0.1*(1<<FIXPTR_SFT64));
	INT64 comp_temp64 =(0.1*(1<<FIXPTR_SFT64));
#elif (_FIXEDPTR_ ==0)
	double overflow;
	double averaging_period;
	double reaction_delay_factor;
	double quality_scale;
	double base_quality;
	double target_quality;
	averaging_period = (double) rc->averaging_period;
#endif

	rc->total_size += data->length;
	M4VE_DEBUG("rc->total_size:%lld %d %d\n", rc->total_size, data->fincr, data->fbase);
	//M4VE_DEBUG(printf("0x%x\n",rc->total_size));
#if (_FIXEDPTR_ ==1)
	rc->time32 += (data->fincr<<FIXPTR_SHIFT) / data->fbase;
//    printf("rc->time32 %d %d\n", rc->time32, rc->bytes_per_sec);
	deviation =
		((rc->total_size<<SFT_3RD) - ((rc->bytes_per_sec>>SFT_1ST) * (rc->time32>>SFT_2ND)))>>SFT_3RD;
#elif (_FIXEDPTR_ ==2)
	rc->time64 += (data->fincr<<SFT64_TIME) / data->fbase;

	deviation =
		((rc->total_size<<SFT64_3RD) - (((rc->bytes_per_sec>>SFT64_1ST) * (rc->time64>>SFT64_2ND))))>>SFT64_3RD;
#elif (_FIXEDPTR_ ==0)
	/* Update internal values */
	rc->time += (double) data->fincr / data->fbase;

	/* Compute the deviation from expected total size */
	deviation =
		rc->total_size - rc->bytes_per_sec * rc->time;
#endif

	dump(rc);
	/* calculate the sequence quality */
#if (_FIXEDPTR_ ==1)
    M4VE_DEBUG("in rc_single_after 0x%x 0x%x\n", rc->sequence_quality32, rc->averaging_period);
	rc->sequence_quality32 -= rc->sequence_quality32 / rc->averaging_period;
	rc->sequence_quality32 +=
		((DEFAULT_INITIAL_QUANTIZER>>1)<<FIXPTR_SHIFT) / data->quant / rc->averaging_period;
	if (rc->sequence_quality32 < comp_temp)
		rc->sequence_quality32 = comp_temp;
	else if (rc->sequence_quality32 > (1<<FIXPTR_SHIFT))
		rc->sequence_quality32 = (1<<FIXPTR_SHIFT);
    M4VE_DEBUG("in rc_single_after2 0x%x 0x%x\n", rc->sequence_quality32, rc->averaging_period);
#elif (_FIXEDPTR_ ==2)
	rc->sequence_quality64 -= rc->sequence_quality64 / rc->averaging_period;
	rc->sequence_quality64 +=
		((DEFAULT_INITIAL_QUANTIZER>>1)<<FIXPTR_SFT64) / data->quant / rc->averaging_period;
	if (rc->sequence_quality64 < comp_temp64)
		rc->sequence_quality64 = comp_temp64;
	else if (rc->sequence_quality64 > (1<<FIXPTR_SFT64))
		rc->sequence_quality64 = (1<<FIXPTR_SFT64);
#elif (_FIXEDPTR_ ==0)
	rc->sequence_quality -= rc->sequence_quality / averaging_period;
	rc->sequence_quality +=
		((double)(DEFAULT_INITIAL_QUANTIZER>>1)) / (double) data->quant / averaging_period;

	/* clamp the sequence quality to 10% to 100%
	 * to try to avoid using the highest
	 * and lowest quantizers 'too' much */
	if (rc->sequence_quality < 0.1)
		rc->sequence_quality = 0.1;
	else if (rc->sequence_quality > 1.0)
		rc->sequence_quality = 1.0;
#endif
	dump(rc);
	/* factor this frame's size into the average framesize
	 * but skip using ivops as they are usually very large
	 * and as such, usually disrupt quantizer distribution */
	if (data->type != I_VOP)
	{
#if (_FIXEDPTR_ ==1)
//        printf("in rc_single_after 0x%x\n", rc->reaction_delay_factor32);
		reaction_delay_factor32 = rc->reaction_delay_factor32;
		rc->avg_framesize32 -= rc->avg_framesize32 / reaction_delay_factor32;
		rc->avg_framesize32 += data->length / reaction_delay_factor32;
        M4VE_DEBUG("in rc_single_after 0x%x 0x%x 0x%x\n", data->length, reaction_delay_factor32, rc->avg_framesize32);
#elif (_FIXEDPTR_ ==2)
		reaction_delay_factor64 = rc->reaction_delay_factor64;
		rc->avg_framesize64 -= rc->avg_framesize64 / reaction_delay_factor64;
		rc->avg_framesize64 += data->length / reaction_delay_factor64;
#elif (_FIXEDPTR_ ==0)
		reaction_delay_factor = (double) rc->reaction_delay_factor;
		rc->avg_framesize -= rc->avg_framesize / reaction_delay_factor;
		rc->avg_framesize += data->length / reaction_delay_factor;
#endif
	}

	/* don't change the quantizer between pvops */
#if !defined(_IPB_FRAMEQP_)
	if (data->type == XVID_TYPE_BVOP)
		return (0);
#endif
dump(rc);


#if (_FIXEDPTR_ == 1)
    M4VE_DEBUG("in rc_single_after 0x%x\n",  rc->avg_framesize32);
	quality_scale32 =
		(rc->target_framesize32<<(FIXPTR_SHIFT>>1)) / rc->avg_framesize32 *
		(rc->target_framesize32<<(FIXPTR_SHIFT>>1)) / rc->avg_framesize32;

	base_quality32 = rc->sequence_quality32;
	if (quality_scale32 >= (1<<FIXPTR_SHIFT)) {
		base_quality32 = (1<<FIXPTR_SHIFT) - (((1<<FIXPTR_SHIFT) - base_quality32)<<FIXPTR_SHIFT)/ quality_scale32;
	} else {
		base_quality32 = temp_value + (((base_quality32 - temp_value) * quality_scale32)>>FIXPTR_SHIFT);
	}
    M4VE_DEBUG("base_quality32: %d %d %d %d %d\n", base_quality32, deviation, rc->buffer, rc->bytes_per_sec, rc->time32);
	//deviation <<=FIXPTR_SHIFT>>1;
	overflow32 = -( deviation / rc->buffer);
	if (overflow32 > rc->target_framesize32)
		overflow32 = rc->target_framesize32;
	else if (overflow32 < -rc->target_framesize32)
		overflow32 = -rc->target_framesize32;
    M4VE_DEBUG("overflow: %d\n", overflow32);
	target_quality32 =
		base_quality32 + (base_quality32 -
				temp_value) * overflow32 / rc->target_framesize32;
//    printk("target_quality32: %d\n", target_quality32);
	if (target_quality32 > (1<<(FIXPTR_SHIFT+1)))
		target_quality32 = (1<<(FIXPTR_SHIFT+1));
	else if (target_quality32 < temp_value)
		target_quality32 = temp_value;

	rtn_quant = (int) (((DEFAULT_INITIAL_QUANTIZER>>1)<<FIXPTR_SHIFT) / target_quality32);
    M4VE_DEBUG("rtn_quant: %d\n", rtn_quant);
	if (rtn_quant > 0 && rtn_quant < 31)
	{
		rc->quant_error32[rtn_quant - 1] += ((DEFAULT_INITIAL_QUANTIZER>>1)<<(FIXPTR_SHIFT+EXSFT)) / target_quality32 - (rtn_quant<<EXSFT);
		if (rc->quant_error32[rtn_quant - 1] >= (1<<EXSFT))
		{
			rc->quant_error32[rtn_quant - 1] -= (1<<EXSFT);
			rtn_quant++;
			rc->rtn_quant++;
		}
	}
#elif (_FIXEDPTR_ ==2)
	quality_scale64 =
		(rc->target_framesize64<<(FIXPTR_SFT64>>1)) / rc->avg_framesize64 *
		(rc->target_framesize64<<(FIXPTR_SFT64>>1)) / rc->avg_framesize64;

	base_quality64 = rc->sequence_quality64;
	if (quality_scale64 >= (1<<FIXPTR_SFT64)) {
		base_quality64 = (1<<FIXPTR_SFT64) - (((1<<FIXPTR_SFT64) - base_quality64)<<FIXPTR_SFT64)/ quality_scale64;
	} else {
		base_quality64 = temp_value64 + (((base_quality64 - temp_value64) * quality_scale64)>>FIXPTR_SFT64);
	}

	overflow64 = -( deviation / rc->buffer);
	if (overflow64 > rc->target_framesize64)
		overflow64 = rc->target_framesize64;
	else if (overflow64 < -rc->target_framesize64)
		overflow64 = -rc->target_framesize64;

	target_quality64 =
		base_quality64 + (base_quality64 -
				temp_value64) * overflow64 / rc->target_framesize64;
	if (target_quality64 > (1<<(FIXPTR_SFT64+1)))
		target_quality64 = (1<<(FIXPTR_SFT64+1));
	else if (target_quality64 < temp_value64)
		target_quality64 = temp_value64;

	rtn_quant = (int) (((DEFAULT_INITIAL_QUANTIZER>>1)<<FIXPTR_SFT64) / target_quality64);

	if (rtn_quant > 0 && rtn_quant < 31)
	{
		rc->quant_error64[rtn_quant - 1] += (value64<<(FIXPTR_SFT64+EXSFT64)) / target_quality64 - (rtn_quant<<EXSFT64);
		if (rc->quant_error64[rtn_quant - 1] >= (1<<EXSFT64))
		{
			rc->quant_error64[rtn_quant - 1] -= (1<<EXSFT64);
			rtn_quant++;
			rc->rtn_quant++;
		}
	}
#elif (_FIXEDPTR_ ==0)
	/* calculate the quality_scale which will be used
	 * to drag the target quality up or down, depending
	 * on if avg_framesize is >= target_framesize */
	quality_scale =
		rc->target_framesize / rc->avg_framesize *
		rc->target_framesize / rc->avg_framesize;

	/* use the current sequence_quality as the
	 * base_quality which will be dragged around
	 *
	 * 0.06452 = 6.452% quality (quant:31) */
	base_quality = rc->sequence_quality;
	if (quality_scale >= 1.0) {
		base_quality = 1.0 - (1.0 - base_quality) / quality_scale;
	} else {
		base_quality = 0.06452 + (base_quality - 0.06452) * quality_scale;
	}
	overflow = -((double) deviation / (double) rc->buffer);

	/* clamp overflow to 1 buffer unit to avoid very
	 * large bursts of bitrate following still scenes */
	if (overflow > rc->target_framesize)
		overflow = rc->target_framesize;
	else if (overflow < -rc->target_framesize)
		overflow = -rc->target_framesize;

	/* apply overflow / buffer to get the target_quality */
	target_quality =
		base_quality + (base_quality -
						0.06452) * overflow / rc->target_framesize;

	/* clamp the target_quality to quant 1-31
	 * 2.0 = 200% quality (quant:1) */
	if (target_quality > 2.0)
		target_quality = 2.0;
	else if (target_quality < 0.06452)
		target_quality = 0.06452;


	rtn_quant = (int) (((double)(DEFAULT_INITIAL_QUANTIZER>>1)) / target_quality);

	/* accumulate quant <-> quality error and apply if >= 1.0 */
	if (rtn_quant > 0 && rtn_quant < 31)
	{
		rc->quant_error[rtn_quant - 1] += ((double)(DEFAULT_INITIAL_QUANTIZER>>1)) / target_quality - rtn_quant;
		if (rc->quant_error[rtn_quant - 1] >= 1.0) {
			rc->quant_error[rtn_quant - 1] -= 1.0;
			rtn_quant++;
			rc->rtn_quant++;
		}
	}
#endif
	/* prevent rapid quantization change */
#if 0
	{
		FILE *tmp_fp;
		tmp_fp=my_fopen("sad.txt","a+");
		fprintf(tmp_fp,"%d\n",gBufMgr.Buf[ gBufMgr.curr_enc].sad);
		fclose(tmp_fp);
	}
#endif
	dump(rc);

	if (1 /* enable_rc_q_limit */ )
	{
		if (rtn_quant > rc->rtn_quant + 1) {
			if (rtn_quant > rc->rtn_quant + 3)
				if (rtn_quant > rc->rtn_quant + 5)
					rtn_quant = rc->rtn_quant + 3;
				else
					rtn_quant = rc->rtn_quant + 2;
			else
				rtn_quant = rc->rtn_quant + 1;
		}
		else if (rtn_quant < rc->rtn_quant - 1) {
			if (rtn_quant < rc->rtn_quant - 3)
				if (rtn_quant < rc->rtn_quant - 5)
					rtn_quant = rc->rtn_quant - 3;
				else
					rtn_quant = rc->rtn_quant - 2;
			else
				rtn_quant = rc->rtn_quant - 1;
		}
	}

    M4VE_DEBUG("rtn_quant %d\n", rtn_quant);

	//fwrite(&rtn_quant,sizeof(int),1,fquant);
#if (_OUTPUTQUANT_ ==1)
#if (_FIXEDPTR_ ==1)
	fprintf(fquant32, "%d \n",rtn_quant);
#elif (_FIXEDPTR_ ==2)
	fprintf(fquant64, "%d \n",rtn_quant);
#elif (_FIXEDPTR_ ==0)
	fprintf(fquant, "%d \n",rtn_quant);
#endif
#endif
	dump(rc);

	rc->rtn_quant = rtn_quant;
	return (0);
}

#define _M4VE_DRV_
#ifdef _M4VE_DRV_

#include "drvM4VE.h"

static xvid_plg_create_t x_rc_create;
static rc_single_t x_rc;
static xvid_plg_data_t x_rc_data;

extern U32 VopTimeIncResolution ;
extern U32 VopTimeInc ;

void rc_init(U16 width, U16 height , /*double frate ,*/ U32 bitrate)
{
	int i;
	M4VE_DEBUG("rc init %d\n", bitrate);
/*
	create.fbase=(Int)vo_config_list->layers->frame_rate;
	for (vo_config = vo_config_list; vo_config != NULL; vo_config = vo_config->pnext) {
		for (vol_config = vo_config->layers; vol_config != NULL; vol_config = vol_config->pnext) {
			create.bitrate=vol_config->bit_rate;
		}
	}
*/

	memset(&x_rc_create,0,sizeof(xvid_plg_create_t));
	memset(&x_rc,0,sizeof(rc_single_t));
	memset(&x_rc_data,0,sizeof(xvid_plg_data_t));


	x_rc_create.fincr= VopTimeInc ;
	x_rc_create.fbase = VopTimeIncResolution ;

	x_rc_create.bitrate = bitrate;
	x_rc_create.reaction_delay_factor=0;
	x_rc_create.averaging_period=0;
	x_rc_create.buffer=0;

	x_rc_data.frame_num = -1;
	x_rc_data.width = width;
	x_rc_data.height = height;
	x_rc_data.mb_width = width >> 4 ;
	x_rc_data.mb_height = height >> 4 ;


	rc_single_create(&x_rc_create, &x_rc);

	for (i=0; i<3; i++) {
		x_rc_data.min_quant[i] = 1;
		x_rc_data.max_quant[i] = 31;
	}

	M4VE_DEBUG("rc init done\n");
}

U8 rc_vop_before(U16 vop_type)
{
	U32 quant ;
	//x_rc_data.zone = 0;
	/*
	x_rc_data.width = curr_vop_bb->width;
	x_rc_data.height = curr_vop_bb->height;
	x_rc_data.mb_width = curr_vop_bb->MB_in_width;
	x_rc_data.mb_height = curr_vop_bb->MB_in_height;
	*/
	x_rc_data.fincr = x_rc_create.fincr;
	x_rc_data.fbase = x_rc_create.fbase;
	x_rc_data.frame_num++;
	x_rc_data.quant=0;
	x_rc_data.type=vop_type;

	quant = rc_single_before(&x_rc,&x_rc_data);

	return (U8) quant;

	// PutVopIntraQuantizer(quant, curr_vop_bb);
}

void rc_vop_after(U32 num_bytes)
{
	x_rc_data.length = num_bytes;
	M4VE_DEBUG("x_rc.total_size:%d num_bytes: 0x%x\n", (U32)x_rc.total_size, num_bytes);
	rc_single_after(&x_rc, &x_rc_data);
    //printf("x_rc.rtn_quant: 0x%x\n", x_rc.rtn_quant);
}

void rc_exit(void)
{
	// rc_single_destroy();
	return;
}

#endif /* _M4VE_DRV_ */


