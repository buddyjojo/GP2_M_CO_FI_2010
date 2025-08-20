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


#ifdef _MSVC6_
#include <windows.h> //for sleep
#endif
#include "drvM4VE.h"
#ifdef _M4VE_OLYMPUS_
#include "math.h"
#endif
//#include "m4vereg.h"

//extern FILE *fp_script;
//extern M4VE_REG m4ve_reg;
typedef struct _sgc_putbit
{
	short data;
	short nbits;
} SGC_PUTBIT;

static SGC_PUTBIT sgc_buf[128];
static int sgc_buf_count = 0;
U32 u32HeaderBitLen = 0;
extern U8 u8PBlength;
extern U32 VopTimeIncResolution ;
extern U32 VopTimeInc ;
extern short DOUBLE_FRAME;
extern int modulo_time_base[2];
static long VopTime = -1;

#ifdef _NO_FILESYSTEM_ //defined(_AEON_PLATFORM_) || defined(_M4VE_OLYMPUS_)
extern int fp_script;
#else
extern FILE *fp_script;
#endif

extern void MDrv_M4VE_PutHeaderSeq(U16 bits, U8 len);

#if defined(_M4VE_T3_) && defined(_MIPS_PLATFORM_)
int log2ceil(int arg)
{
    int j=0;
    int i=1;
    while (arg>i) {
        i=i+i;
        j++;
    }
    return j;
}
#endif

U32 BitsCalculate(U32 value)
{
	U32 i = 0;
	do {
		value >>= 1 ;
		i++;
	} while( value > 0 ) ;
	return i;
}

void init_HeaderSeq(unsigned int f_count)
{
    sgc_buf_count = 0;
    u32HeaderBitLen = 0;
    if (f_count==0)
    VopTime = -1;
}
static void Output_Header(int data, int nbits)
{

		int nbytes = 2;
		int formatbits;
		unsigned int output;
		unsigned int tmp_buf;
		unsigned int mask[5]={0,
							  0xff000000,
							  0xffff0000,
							  0xffffff00,
							  0xffffffff
							};

		formatbits=(nbytes*8);
		output = data;
		output = output<<(32-nbits);
		do
		{
			tmp_buf = output;
			tmp_buf&=mask[nbytes];
			tmp_buf=tmp_buf>>(32-formatbits);
			sgc_buf[sgc_buf_count].data = tmp_buf;
//fprintf(fp_script, "ddd 0x%04X 0x%04X %d\n", 0xa80+0, m4ve_reg.reg00, sgc_buf_count);
            output<<=formatbits;
            ASSERT(sgc_buf_count<(sizeof(sgc_buf)/sizeof(SGC_PUTBIT)));
            sgc_buf[sgc_buf_count].nbits = (nbits<formatbits)?nbits:formatbits;

			MDrv_M4VE_PutHeaderSeq(tmp_buf,(nbits<formatbits)?nbits:formatbits);

			nbits-=formatbits;
			sgc_buf_count++;


		}while(nbits>0);

}

void NextStartCode(void/*U32 bitLen*/)
{

  U32 i = 0;
  U32 count = ( u32HeaderBitLen % 8 );


  count = 8 - count;

//  printf("bitlen:%d bit stuffing: %d bits\n",u32HeaderBitLen,count);

  u32HeaderBitLen += count;

  count --;
  Output_Header(0,1);

  while (i < count)
  {
      Output_Header(1,1);
      i++;
  }
}


void MDrv_M4VE_BitstreamPutVOLHeader(U16 width, U16 height, U8 interlace, U8 qtype
                                    ,U32 frame_count)
{   //in CModel: BitstreamPutVOLs
    if (DOUBLE_FRAME!=2) {
        if (frame_count==0) {
	    Output_Header(0x8, /*VO_START_CODE*/ 27);
	    Output_Header(0, /*vo_id*/5);
        }
        Output_Header(0x12 /*VOL_START_CODE */ , 28 /*VOL_START_CODE_LENGTH */ );
        Output_Header(0, /*GetVolId(layer)*/ 4);
	    Output_Header( 0, /*GetVolRandomAccessibleVol(layer) */ 1);
        if (u8PBlength || interlace)
            Output_Header(17, /*GetVolVisualObjectTypeIndication(layer) */ 8);  // simple object type
        else
	        Output_Header(1, /*GetVolVisualObjectTypeIndication(layer) */ 8);  // simple object type
        Output_Header(1, /*GetVolIsObjectLayerIdentifier(layer)*/ 1);
        Output_Header(2, /*vol_vol_verid,*/ 4);
        Output_Header(1, /* GetVolVisualObjectLayerPriority(layer),*/ 3);
        Output_Header(1, /*aspect_ratio_info*/4);   // 1 means square
        // Output_Header(GetVolAspectRatioInfoParWidth(layer),8);
        // Output_Header(GetVolAspectRatioInfoParHeight(layer),8);
        Output_Header(0, /*GetVolVolControlParameters(layer),*/ 1);
        // Output_Header(GetVolChromaFormat(layer),2);
        // Output_Header(GetVolLowDelay(layer),1);
        // Output_Header(GetVolVbvParameters(layer),1);

        Output_Header(1,3);  // VOL shape & market bit

        Output_Header(VopTimeIncResolution /*GetVolTimeIncrementResolution(layer)*/,16);
#if 0
        //non fixed vop rate
	    Output_Header(5,3); // market_bit & fixed_vop_rate & market_bit
#else
        //fixed vop rate
        Output_Header(3, 2);
        Output_Header(VopTimeInc, BitsCalculate(VopTimeIncResolution));
        Output_Header(1, 1);
#endif
        Sleep(10); //wait 10
#ifdef _TRACE32_CMM_
        M4VE_SCRIPT(fprintf(fp_script, "B::wait.10ms\n"));
#endif
	}

	//1 should add the fixed_vop_rate ??
	//wait 10 in CModel

    Output_Header((width<<1)|1, 13+1);
    //Output_Header(1,1);

	Output_Header(height, 13);
	Output_Header(1,1); //MARKER_BIT

	Output_Header( interlace ? 1 : 0, 1 );
	Output_Header(1,1); // obmc_disable

	// Output_Header(0,1); // sprite_enable
	Output_Header(0,2); // if verid != 1, two bits.

	//Output_Header(0,/*GetVolNot8Bit(layer)*/1);
	//Output_Header(qtype, 1);  // quant type
    Output_Header(qtype, 2);  // quant type

	if( qtype ) {
		Output_Header(0,2); // load intra & no-intra quant_mat   ?
		//1  customized the Quant_mat is not implemented yet.
	}

	Output_Header(0,1); // quarter_sample

	Output_Header(1,/*GetVolComplexityEstimationDisable(layer),*/1);

	Output_Header(1,/*GetVolErrorResDisable(layer),*/1); // resync_market_disable

	Output_Header(0/*GetVolDataPartEnable(layer)*/,1);

	Output_Header(0,3); //  newpred_enable, reduced_resolution_vop_enable, scalability
	NextStartCode();
    Sleep(10); //wait 10
#ifdef _TRACE32_CMM_
    M4VE_SCRIPT(fprintf(fp_script, "B::wait.10ms\n"));
#endif
    //wait 10 in CModel
}

int MDrv_M4VE_GenSkipVOPHeader(U8 voptype, U8 *pBuf)
{
    int nBufIdx = 0;
    int nBits = 0;
    int index;
    long display_voptime;

    // vop_start_code
    pBuf[nBufIdx++] = 0x00;
    pBuf[nBufIdx++] = 0x00;
    pBuf[nBufIdx++] = 0x01;
    pBuf[nBufIdx++] = 0xB6;

    // vop_coding_type
    if (voptype==B_VOP_NREC || voptype==B_VOP_REC)
        voptype = B_VOP;
    {
        nBits = 2;
        pBuf[nBufIdx] = voptype<<(8-nBits);
    }

    // modulo_time_base
    index = modulo_time_base[voptype!=B_VOP];
    VopTime += VopTimeInc;
    display_voptime = VopTime;

    if ((voptype!=B_VOP)/*&&(frameNum!=0)*/) display_voptime += u8PBlength;
    else if (voptype==B_VOP_NREC || voptype==B_VOP_REC) display_voptime -= 1;
    display_voptime = display_voptime-index*VopTimeIncResolution;
    if( display_voptime >= VopTimeIncResolution ) {
        //Output_Header(1,1);
        //Output_Header(0,1);
        nBits += 2;
        pBuf[nBufIdx] += (0x2<<(8-nBits));
        display_voptime -= VopTimeIncResolution;
        index++;
    } else {
        //Output_Header(0,1);
        nBits += 1;
    }
    if (voptype!=B_VOP) {
        modulo_time_base[0] = modulo_time_base[1];
        modulo_time_base[1] = index;
    }

    // market_bit
    nBits += 1;
    pBuf[nBufIdx] += 0x1<<(8-nBits);

    // vop_time_increment
    {
        U32 data = display_voptime;
        U32 nDataBits = BitsCalculate(VopTimeIncResolution-1);
        while (nDataBits>0) {
            if (nBits+nDataBits>=8) {
                pBuf[nBufIdx] += data>>(nDataBits-(8-nBits));
                nDataBits -= (8-nBits);
                data = data & ((U32)0xFFFFFFFF >> (32-nDataBits)); //32
                nBufIdx++;
                pBuf[nBufIdx] = 0;
                nBits = 0;
            }
            else {
                nBits += nDataBits;
                pBuf[nBufIdx] += data<<(8-nBits);
                nDataBits = 0;
            }
        }
    }

    // marker_bit
    nBits += 1;
    pBuf[nBufIdx] += 0x1<<(8-nBits);
    if (nBits==8) {
        nBufIdx++;
        pBuf[nBufIdx] = 0;
        nBits=0;
    }

    // vop_coded = 0
    nBits += 1;
    if (nBits==8) {
        nBufIdx++;
        pBuf[nBufIdx] = 0;
        nBits=0;
    }

    // NextStartCode
    // 0
    nBits += 1;
    if (nBits==8) {
        nBufIdx++;
        pBuf[nBufIdx] = 0;
        nBits=0;
    }
    //printf("display_voptime skip  %x %x %x %x\n", pBuf, (int)pBuf[4], (int)pBuf[5], (int)((U8)0xFF>>(nBits)));
    // 1's
    if (nBits>0) {
        pBuf[nBufIdx] += ((U8)0xFF>>(/*8-*/nBits));
        nBufIdx++;
        nBits = 0;
    }
    return nBufIdx;
    //*pnSize = nBufIdx;
}

void MDrv_M4VE_BitstreamPutVOPHeader(U8 voptype, U8 quant, U8 interlace, U32 frameNum,U8 SearchRange
#ifdef _M4VE_BIG2_
                                     , int g_IsShortHeader, int g_nSourceFormat, int width, int height
#endif
                                     )
{
    long display_voptime;
    int index;//, time_modulo;
#ifdef _M4VE_BIG2_
//    U16 temp16;
    if (g_IsShortHeader) {
        Output_Header((SHORT_VIDEO_START_MARKER>>16),SHORT_VIDEO_START_MARKER_LENGTH-16);
        Output_Header((SHORT_VIDEO_START_MARKER&0xffff),16);
        Output_Header((frameNum&0xff),8);
        // marker_bit                   = 1
        // zero_bit                     = 0
        // split_screen_indicator       = 0
        // document_camera_indicator    = 0
        // full_picture_freeze_release  = 0
        Output_Header(0x10,5);

        Output_Header(g_nSourceFormat,3);
        //ReadRegM4VE(0x60, &temp16);
        Sleep(10);
#ifdef _TRACE32_CMM_
        M4VE_SCRIPT(fprintf(fp_script, "B::wait.10ms\n"));
#endif

        if (g_nSourceFormat<=5) {
        Output_Header(voptype,1);
        // four_reserved_zero_bits
        Output_Header(0,4);
        } else {
            int UFEP, OPPTYPE, MPPTYPE, CPFMT;

			// Update Full Extended PTYPE (UFEP) (3 bits)
			if (voptype == I_VOP)
				UFEP = 1;	// 001
			else
                UFEP = 0;	// 000

            Output_Header(UFEP, 3);
            if (UFEP==1) {
				OPPTYPE = (6<<15)	// Source Format: "110" custom source format
					| (0 <<14)	// Optional Custom PCF
					| (0 <<13)	// Optional Unrestricted Motion Vector (UMV) mode (see Annex D)
					| (0 <<12)	// Optional Syntax-based Arithmetic Coding (SAC) mode (see Annex E)
					| (0 <<11)	// Optional Advanced Prediction (AP) mode (see Annex F)
					| (0 <<10)	// Optional Advanced INTRA Coding (AIC) mode (see Annex I)
					| (0 << 9)	// Optional Deblocking Filter (DF) mode (see Annex J)
					| (0 << 8)	// Optional Slice Structured (SS) mode (see Annex K)
					| (0 << 7)	// Optional Reference Picture Selection (RPS) mode (see Annex N)
					| (0 << 6)	// Optional Independent Segment Decoding (ISD) mode (see Annex R)
					| (0 << 5)	// Optional Alternative INTER VLC (AIV) mode (see Annex S)
					| (0 << 4)	// Optional Modified Quantization (MQ) mode (see Annex T)
					| (1 << 3);	// Equal to "1" to prevent start code emulation
    		    Output_Header(OPPTYPE,18);
            }
            // The mandatory part of PLUSPTYPE when PLUSPTYPE present (MPPTYPE) (9 bits)
            MPPTYPE = ((voptype==I_VOP?0:1)<<6)
				| (0 << 5)		// Optional Reference Picture Resampling (RPR) mode (see Annex P)
				| (0 << 4)		// Optional Reduced-Resolution Update (RRU) mode (see Annex Q)
				| (0 << 3)		// Rounding Type (RTYPE)
				| (1     );		// Equal to "1" to prevent start code emulation
            Output_Header(MPPTYPE,9);
            // CPM=0, PSBI=N/A
            Output_Header(0,1);
            if (UFEP==1) {
				// Custom Picture Format (CPFMT) (23 bits)
				CPFMT = (2 << 19)	// Pixel Aspect Ratio Code. 2: 12:11 (CIF for 4:3 picture)
					| ((width/4-1) << 10)	// Picture Width Indication: Range [0, ... , 511]; Number of pixels per line = (PWI + 1) * 4
					| (1 << 9)							// Equal to "1" to prevent start code emulation
					| (height/4);			// Picture Height Indication: Range [1, ... , 288]; Number of lines = PHI * 4
				Output_Header(CPFMT,23);
			}
        }
        /*
        if (voptype == I_VOP) {
            Output_Header(GetVopIntraQuantizer(vop),5);
        } else {
            Output_Header(GetVopQuantizer(vop),5);
        }*/
        //ReadRegM4VE(0x60, &temp16);
        Output_Header(quant,5);
        if (g_nSourceFormat<=5) {
        // zero_bit = 0
        // pei      = 0
        Output_Header(0,2);
        } else {
            Output_Header(0,1);
        }
        Output_Header(0,0);
        Sleep(10);
#ifdef _TRACE32_CMM_
        M4VE_SCRIPT(fprintf(fp_script, "B::wait.10ms\n"));
#endif
        sgc_buf_count = 0; // clear buffer
        return;
    }
#endif
	Output_Header(0x0,16);
	Output_Header((0x01B6 /*VOP_START_CODE*/ & 0xffff),16);
    if (voptype==B_VOP_NREC || voptype==B_VOP_REC)
        voptype = B_VOP;
	Output_Header(voptype,2);

    index = modulo_time_base[voptype!=B_VOP];
	VopTime += VopTimeInc;
    display_voptime = VopTime;
    //printf("display_voptime %d\n", display_voptime);
    if ((voptype!=B_VOP)&&(frameNum!=0)) display_voptime += u8PBlength;
    else if (voptype==B_VOP_NREC || voptype==B_VOP_REC) display_voptime -= 1;
    display_voptime = display_voptime-index*VopTimeIncResolution;
    Sleep(1000);
	if( display_voptime >= VopTimeIncResolution ) {
		Output_Header(1,1);
		Output_Header(0,1);
		display_voptime -= VopTimeIncResolution;
        index++;
	} else {
		Output_Header(0,1);
	}
    if (voptype!=B_VOP) {
        modulo_time_base[0] = modulo_time_base[1];
        modulo_time_base[1] = index;
    }
//fprintf(fp_script, "eee 0x%04X 0x%04X %x\n", 0xa80+0, m4ve_reg.reg00, &m4ve_reg.reg00);
	Output_Header(1,1); // market_bit
//fprintf(fp_script, "fff 0x%04X 0x%04X\n", 0xa80+0, m4ve_reg.reg00);
	Output_Header(/*VopTime*/display_voptime, BitsCalculate(VopTimeIncResolution-1 ) );
	Output_Header(1,1); // market_bit


	Output_Header(1, /*vop_coded*/ 1);

	if( voptype == P_VOP ) {
		Output_Header(1,/*GetVopRoundingType(vop),*/1);
	}

    Output_Header(0, /*GetVopIntraDCVlcThr(vop),*/3);
    if( interlace ) {
        Output_Header( interlace /* top_field_first*/ , 1 );  // if interlace == 1, top_field_first, interlace == 2, bottom_field_first
        Output_Header(0,1); // alternate_vertical_scan_flag
    }
    Sleep(1000);
	Output_Header(quant, 5 /* quant precision */ );   // spec page 151 related with "not_8_bit" .

	if( voptype != I_VOP )
		Output_Header(SearchRange == 32 ? 2 : 1 /*vop_fcode_forward*/, 3 );
	if(voptype==B_VOP_NREC || voptype==B_VOP_REC)
		Output_Header(SearchRange == 32 ? 2 : 1 /*vop_fcode_backward*/, 3);

    //sw finish put header
    Output_Header(0,0);

    Sleep(200);
#ifdef _TRACE32_CMM_
    M4VE_SCRIPT(fprintf(fp_script, "B::wait.20ms\n"));
#endif
    sgc_buf_count = 0; // clear buffer
    // wait 10
    // wait 20
}
#ifdef _TWO_CORES_
void MDrv_M4VE_BitstreamPutResyncHeader(short MB_in_height_top, short MB_width, short MB_height, U8 quant,
                                        int fcode_for, U8 voptype)
{
    int resync_marker_length=16+fcode_for;
    int vop_quant_precision=5;
#ifdef _M4VE_OLYMPUS_
    int	MB_in_VOP_length = (int) (log(MB_width*MB_height)*1.442695/*/log(2)*/+1.0); /*1.0 for ceiling*/
#else
    int MB_in_VOP_length = (int) log2ceil(MB_width*MB_height); /*1.0 for ceiling*/
#endif
    if (voptype==B_VOP)
        if (resync_marker_length<18) resync_marker_length=18;
    Output_Header(RESYNC_MARKER, resync_marker_length);
    Output_Header(MB_in_height_top*MB_width, MB_in_VOP_length);
    Output_Header(quant, vop_quant_precision);
    Output_Header(0, 1);
    Output_Header(0, 0); //simulate need this symbol at the end of frame in header.txt
}
#endif
