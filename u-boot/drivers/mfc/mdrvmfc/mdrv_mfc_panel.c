#define _PANEL_C_
	#include "mdrv_mfc_platform.h"

#include "mdrv_mfc_fb.h"
#include "mdrv_mfc_mcu.h"
extern void	udelay	      (unsigned long);

#if(CODESIZE_SEL == CODESIZE_ALL)

extern code U8 tInitializeColorMatrix[];
extern U8 tInitializeColorMatrix_count;
extern code MST_MFC_RegUnitType_t tInitializeTcon23b[];
extern code  MST_MFC_RegUnitType_t tInitializeScTop2[];
extern code U8 tOverDrive[];

//_MINI_LVDS
code MST_MFC_RegUnitType_t tInitializeTcon22b_Comm[]=
{
	{0x22A0, 0x01}, //DEOUT delay
	{0x22A1, 0x00},
	{0x22A2, 0x00}, //DEOUT_AHEAD delay
	{0x22A3, 0x11},
{_END_OF_TBL_, _END_OF_TBL_},
};

//miniLVDS
code MST_MFC_RegUnitType_t tInitializeTcon23b_Comm[]=
 {
    //common
	//{0x2301, 0x00},
	{0x2302, 0x7c},  // {8'h0, skew_reg, ivmd_on, bist_hw_set, swap_fs, fsmode, dskew} = { - 01111, 3'h4};
	{0x2303, 0x00},
	{0x2304, 0x50},  // {8'h0, f2line, fcol, f1a2line, age_8bit, 4'h0} = {- 0001 - }
	{0x2305, 0x00},
	{0x230A, 0x2d},
	/*{0x2310, 0xc0},  // {5'h0, hres} = {- 11'h556 }
	{0x2311, 0x03},
	{0x2314, 0x38},  // {5'h0, vres } = {- 11'h300 }
	{0x2315, 0x04},
	{0x2318, 0x98},  // {4'h0, htot} = {- 12'h698 }
	{0x2319, 0x08},*/
	{0x231A, 0x20},  // {10'h0, cout_type, inmod_pad_sel, 4'h0} =  {- 00 -}
	{0x231B, 0x00},
	///{0x231C, 0x65},  // {5'h0, vtot } = {- 12'h326 }
	///{0x231D, 0x04},
	{0x231E, 0xc0},  // {8'h0, tp_drv, sth_drv, 1'b0, pol_newtype, 4'h0} = {- 11 - 0 -};
	{0x231F, 0x00},
///////Tcon setting by panel start/////////
	{0x23E4, 0x00}, //[15]en_minilvds [14:13]bit_flag [12:9]mini_channel_max
	{0x23E5, 0xac},
    {0x23E8, 0x20},//2374
	{0x23E9, 0x5b},
	{0x23F4, 0x00},
    {0x23F5, 0xFC}, //GOE Mask  //andy.jeong_20091203  0x40 -> 0xFC
	{0x23F6, 0x08},
	{0x23F7, 0x00},
{_END_OF_TBL_, _END_OF_TBL_},
};

code MST_MFC_RegUnitType_t tInitializeTcon23b_55[]=
{
	{0x233C, 0xe0},  // {5'h0, bank1_len[10:0]}
	{0x233D, 0x01},
	{0x2327, 0x01},//only [15:12]
	{0x2321, 0x03},//only [15:12]
	{0x2323, 0x00},//only [15:12]
	{0x2333, 0x03},//only [15:12]
	{0x2335, 0x00},//only [15:12]
	{0x232B, 0x03},//only [15:12]
	{0x232F, 0x02},//only [15:12]
	{0x2339, 0x03},//only [15:12]
    {0x23ED, 0x01},//only [15:12]
    {0x23E1, 0x03},//only [15:12]
    {_END_OF_TBL_, _END_OF_TBL_},
};

code MST_MFC_TCON_TIMING_t tInitNonGipTiming[]=
{
    //TCON_DEFAULT_120
    {
    // GSP(VST)
    0x1D0,//U16  GSP_Start;    // BK23_13[11:0]    //0x2326
    // SOE
    0x3FF,//U16  SOE_Start;    // BK23_10[11:0]    //0x2320
    0x046,//U16  SOE_Period;   // BK23_11[11:0]    //0x2322
    // GOE
    0x3A6,//U16  GOE_Start;    // BK23_19[11:0]    //0x2332
    0x052,//U16  GOE_Period;   // BK23_1A[11:0]    //0x2334
    // GSC
    0x3D9,//U16  GSC_Start;    // BK23_15[11:0]    //0x232A
    0x273,//U16  GSC_Period;   // BK23_17[11:0]    //0x232E
    // FLK(H)
    0x3A6,//U16  FLK_Start;    // BK23_70[11:0]    //0x23E0
    0x1FF,//U16  FLK_Period;   // BK23_76[11:0]    //0x23EC
    // POL
    0x3C4,//U16  POL_Start;    // BK23_1C[11:0]    //0x2338
    },
    //TCON_LAMP_120, TCON_LAMP_T240
    {
    // GSP(VST)
    0x1D0,//U16  GSP_Start;    // BK23_13[11:0]    //0x2326
    // SOE
    0x3FF,//U16  SOE_Start;    // BK23_10[11:0]    //0x2320
    0x046,//U16  SOE_Period;   // BK23_11[11:0]    //0x2322
    // GOE
    0x3A6,//U16  GOE_Start;    // BK23_19[11:0]    //0x2332
    0x052,//U16  GOE_Period;   // BK23_1A[11:0]    //0x2334
    // GSC
    0x3D9,//U16  GSC_Start;    // BK23_15[11:0]    //0x232A
    0x273,//U16  GSC_Period;   // BK23_17[11:0]    //0x232E
    // FLK(H)
    0x3A6,//U16  FLK_Start;    // BK23_70[11:0]    //0x23E0
    0x1FF,//U16  FLK_Period;   // BK23_76[11:0]    //0x23EC
    // POL
    0x3C4,//U16  POL_Start;    // BK23_1C[11:0]    //0x2338
    },
    //TCON_EDGE_120, TCON_IOP_T240
    {
    // GSP(VST)
    0x1D0,//U16  GSP_Start;    // BK23_13[11:0]    //0x2326
    // SOE
    0x3FF,//U16  SOE_Start;    // BK23_10[11:0]    //0x2320
    0x046,//U16  SOE_Period;   // BK23_11[11:0]    //0x2322
    // GOE
    0x3A6,//U16  GOE_Start;    // BK23_19[11:0]    //0x2332
    0x052,//U16  GOE_Period;   // BK23_1A[11:0]    //0x2334
    // GSC
    0x3D9,//U16  GSC_Start;    // BK23_15[11:0]    //0x232A
    0x273,//U16  GSC_Period;   // BK23_17[11:0]    //0x232E
    // FLK(H)
    0x3A6,//U16  FLK_Start;    // BK23_70[11:0]    //0x23E0
    0x1FF,//U16  FLK_Period;   // BK23_76[11:0]    //0x23EC
    // POL
    0x3C4,//U16  POL_Start;    // BK23_1C[11:0]    //0x2338
    },
};
void MDrv_MFC_LoadTconTiming(TCON_PANEL_TYPE idx)
{
    idx&=0x7F;
    // GSP(VST)
    MDrv_MFC_Write2BytesMask(0x2326, tInitNonGipTiming[idx].GSP_Start   , 0x0FFF);
    // SOE
    MDrv_MFC_Write2BytesMask(0x2320, tInitNonGipTiming[idx].SOE_Start   , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2322, tInitNonGipTiming[idx].SOE_Period  , 0x0FFF);
    // GOE
    MDrv_MFC_Write2BytesMask(0x2332, tInitNonGipTiming[idx].GOE_Start   , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2334, tInitNonGipTiming[idx].GOE_Period  , 0x0FFF);
    // GSC
    MDrv_MFC_Write2BytesMask(0x232A, tInitNonGipTiming[idx].GSC_Start   , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x232E, tInitNonGipTiming[idx].GSC_Period  , 0x0FFF);
    // FLK(H)
    MDrv_MFC_Write2BytesMask(0x23E0, tInitNonGipTiming[idx].FLK_Start   , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23EC, tInitNonGipTiming[idx].FLK_Period  , 0x0FFF);
    // POL
    MDrv_MFC_Write2BytesMask(0x2338, tInitNonGipTiming[idx].POL_Start   , 0x0FFF);
}

//_MINI_LVDS_GIP
code MST_MFC_RegUnitType_t tInitializeTcon22c[]=
{
	{0x22A0, 0x01}, //DEOUT delay
	{0x22A1, 0x00},
	{0x22A2, 0x00}, //DEOUT_AHEAD delay
	{0x22A3, 0x11},
{_END_OF_TBL_, _END_OF_TBL_},
};

//GIP
code MST_MFC_RegUnitType_t tInitializeTcon23c[]=
{
    {0x2321, 0x03}, //only [15:12]
    {0x2323, 0x00}, //only [15:12]
    //common
	{0x2302, 0x7c},  // {8'h0, skew_reg, ivmd_on, bist_hw_set, swap_fs, fsmode, dskew} = { - 01111, 3'h4};
	{0x2303, 0x00},
	{0x2304, 0x50},  // {8'h0, f2line, fcol, f1a2line, age_8bit, 4'h0} = {- 0001 - }
	{0x2305, 0x00},
	{0x230A, 0x2d},
	{0x2310, 0xc0},  // {5'h0, hres} = {- 11'h556 }
	{0x2311, 0x03},
	{0x2314, 0x38},  // {5'h0, vres } = {- 11'h300 }
	{0x2315, 0x04},
	{0x2318, 0x98},  // {4'h0, htot} = {- 12'h698 }
	{0x2319, 0x08},
	{0x231A, 0x20},  // {10'h0, cout_type, inmod_pad_sel, 4'h0} =  {- 00 -}
	{0x231B, 0x00},
	{0x231C, 0x65},  // {5'h0, vtot } = {- 12'h326 }
	{0x231D, 0x04},
	{0x231E, 0xc0},  // {8'h0, tp_drv, sth_drv, 1'b0, pol_newtype, 4'h0} = {- 11 - 0 -};
	{0x231F, 0x00},
	{0x2326, 0xd0},  // {4'h0, stvst} = {- 12'h1c0 }
	{0x2327, 0x01},
	{0x232A, 0xa9},  // {4'h0, cpvst} = {- 12'h2e2 }
	{0x232B, 0x03},
	{0x232E, 0x73},  // {4'h0, cpvpw} = {- 12'h1a5 }
	{0x232F, 0x02},
	{0x2332, 0x52},  // {4'h0, oest} = {- 12'h28b }
	{0x2333, 0x03},
	{0x2334, 0xd2},  // {6'h0, oepw} = {- 10'h95 }
	{0x2335, 0x00},
	{0x2339, 0x03},//0x04,
	{0x233C, 0xe0},  // {5'h0, bank1_len[10:0]} = {- 11'h2ab}
	{0x233D, 0x01},
    {0x23E0, 0xa6},
    {0x23E1, 0x03},
	{0x23E4, 0x00}, //[15]en_minilvds [14:13]bit_flag [12:9]mini_channel_max
	{0x23E5, 0xac},
	{0x23E8, 0x20}, //HEMAN
	{0x23E9, 0x5B},
    {0x23EC, 0xff},
    {0x23ED, 0x01},
	{0x23F6, 0x08},
	{0x23F7, 0x00},
    //gip
	{0x234F, 0x00},
    {0x2349, 0x40}, //only [15:12]
    {0x234B, 0x04}, //only [15:12]
    {0x2341, 0x04}, //only [15:12]
    {0x2343, 0x04}, //only [15:12]
    {0x2345, 0x23}, //only [15:12]
    {0x2347, 0x03}, //only [15:12]
    {0x23A9, 0x04}, //only [15:12]
	{0x23AB, 0x03},
	{0x235F, 0x00},
    {0x2359, 0x40}, //only [15:12]
    {0x235B, 0x04}, //only [15:12]
    {0x2351, 0x04}, //only [15:12]
    {0x2353, 0x04}, //only [15:12]
    {0x2355, 0x23}, //only [15:12]
    {0x2357, 0x03}, //only [15:12]
    {0x23AD, 0x04}, //only [15:12]
    {0x23AF, 0x03}, //only [15:12]
	{0x236F, 0x00},
    {0x2369, 0x40}, //only [15:12]
    {0x236B, 0x04}, //only [15:12]
    {0x2361, 0x04}, //only [15:12]
    {0x2363, 0x04}, //only [15:12]
    {0x2365, 0x23}, //only [15:12]
    {0x2367, 0x03}, //only [15:12]
    {0x23B1, 0x04}, //only [15:12]
    {0x23B3, 0x03}, //only [15:12]
	{0x237F, 0x00},
    {0x2379, 0x40}, //only [15:12]
    {0x237B, 0x04}, //only [15:12]
    {0x2371, 0x04}, //only [15:12]
    {0x2373, 0x04}, //only [15:12]
    {0x2375, 0x23}, //only [15:12]
    {0x2377, 0x03}, //only [15:12]
    {0x23B5, 0x04}, //only [15:12]
    {0x23B7, 0x03}, //only [15:12]
	{0x238D, 0x00},
	{0x238F, 0x00},
    {0x2389, 0x40}, //only [15:12]
    {0x238B, 0x04}, //only [15:12]
    {0x2381, 0x04}, //only [15:12]
    {0x2383, 0x04}, //only [15:12]
    {0x2385, 0x23}, //only [15:12]
    {0x2387, 0x03}, //only [15:12]
    {0x23B9, 0x04}, //only [15:12]
    {0x23BB, 0x03}, //only [15:12]
	//gpo5 //GCLK6
    {0x2393, 0x40}, //only [15:12]
    {0x23BD, 0x04}, //only [15:12]
    {0x23BF, 0x03}, //only [15:12]
    {0x23D5, 0x24}, //only [15:12]
    {0x23D7, 0x03}, //only [15:12]
    {0x23D9, 0x04}, //only [15:12]
    {0x23DB, 0x03}, //only [15:12]
	//gpo6 //VST
    {0x2395, 0x40}, //only [15:12]
    {0x23A5, 0x40}, //only [15:12]
    {0x23DF, 0x03}, //only [15:12]
    {0x23C9, 0x00}, //only [15:12]
    {0x23C5, 0x80}, //only [15:12]
    {0x23C7, 0x00}, //only [15:12]
	{0x23D3, 0x00},
	{0x23CE, 0x00}, //[15]frame_tog_md,[12]wave_polarity,[11:0]vst
	{0x23CF, 0x90},
    {0x23D1, 0x00}, //only [15:12]
    {0x2312, 0xC0}, // 0xC0
    {0x2339, 0x03}, //only [15:12]

    // GIP power on sequence
    {0x23F5, 0xFC}, //0x23F5[3]:SOE OEN [2]:POL

    {0x23C8, 0x08},//[4]:VDD_EVEN [3:0] delay
    {0x23D2, 0x08},//[4]:VDD_ODD  [3:0] delay
    {0x23A5, 0xC0},// [7:4] VST delay

    {0x23A3, 0xF4},  // [7:4] GCLK6 delay
    {0x238E, 0x0F},  // [3:0] GCLK5 delay
    {0x237E, 0x0F},  // [5] GCLK4 EN [3:0] delay
    {0x236E, 0x0F},  // [5]:GCLK3 EN [3:0] delay
    {0x235E, 0x0F},  // [5]:GCLK2 EN [3:0] delay
    {0x234E, 0x0F},  // [5]:GCLK1 EN [3:0] delay
    {0x238C, 0x01},  // [2]: VST EN [6]:GCLK5 EN [4]:GCLK6 EN

{_END_OF_TBL_, _END_OF_TBL_},
};

code MST_MFC_GIP_TIMING_t tInitGipTiming[]=
{
    // GIP_DEFAULT_120
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        0, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x8080, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_32_LAMP_60
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_32_LAMP_120, GIP_32_LAMP_T240, GIP_32_EDGE_120
    {
    // SOE
    0x417, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x063, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x184, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00E, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x00F, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x11F, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x1B3, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x1, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x26F, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x06D, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x25B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x053, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x254, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x053, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x27F, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x07E, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x27A, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x053, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x254, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x053, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x201, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x000, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x1EE, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x002, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x254, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x053, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x240, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x04D, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x254, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x053, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x254, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x053, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x448, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x208, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x0AF, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x206, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x098, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x1FA, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x089, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x265, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x062, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x254, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x053, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x254, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x053, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xFD, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x005, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x0E4, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xFD, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_37_LAMP_60
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_37_LED_120
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_37_LAMP_T240
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_42_LAMP_60
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_42_LED_120, GIP_42_LAMP_120, GIP_42_LAMP_T240, GIP_42_IOP_T240
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_47_LAMP_60
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_47_LAMP_120, GIP_47_LAMP_T240
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },
    // GIP_47_EDGE_120, GIP_47_IOP_T240
    {
    // SOE
    0x3DD, // U16 SOE_Start;           //BK23_10[11:0] 0x2320
    0x078, // U16 SOE_End;             //BK23_11[11:0] 0x2322
    // POL
    0x3C4, // U16 POL_Start;           //BK23_1C[11:0] 0x2338
    // VST
    0x00F, // U16 VST_VerticalStart;   //BK23_4A[11:0] 0x2394
    0x010, // U16 VST_VerticalEnd;     //BK23_52[11:0] 0x23A4
    0x000, // U16 VST_HorizontalStart; //BK23_6E[11:0] 0x23DC
    0x37E, // U16 VST_HorizontalEnd;   //BK23_6F[11:0] 0x23DE
      0x0, // U8  VST_HorizontalWidth; //BK23_6E[15:12] 0x23DD
    // GCLK1
    0x44B, // U16 GCLK1_1stLn_Start;   //BK23_21[11:0] 0x2342
    0x31F, // U16 GCLK1_1stLn_End;     //BK23_22[11:0] 0x2344
    0x44B, // U16 GCLK1_OthLn_Start;   //BK23_20[11:0] 0x2340
    0x31F, // U16 GCLK1_OthLn_End;     //BK23_23[11:0] 0x2346
    0x44B, // U16 GCLK1_EndLn_Start;   //BK23_54[11:0] 0x23A8
    0x31F, // U16 GCLK1_EndLn_End;     //BK23_55[11:0] 0x23AA
    0x013, // U16 GCLK1_SeqVertical_Start; //BK23_24[11:0] 0x2348
    0x44B, // U16 GCLK1_SeqVertical_End;   //BK23_25[11:0] 0x234A
    // GCLK2
    0x44B, // U16 GCLK2_1stLn_Start;   //BK23_29[11:0] 0x2352
    0x31F, // U16 GCLK2_1stLn_End;     //BK23_2A[11:0] 0x2354
    0x44B, // U16 GCLK2_OthLn_Start;   //BK23_28[11:0] 0x2350
    0x31F, // U16 GCLK2_OthLn_End;     //BK23_2B[11:0] 0x2356
    0x44B, // U16 GCLK2_EndLn_Start;   //BK23_56[11:0] 0x23AC
    0x31F, // U16 GCLK2_EndLn_End;     //BK23_57[11:0] 0x23AE
    0x014, // U16 GCLK2_SeqVertical_Start; //BK23_2C[11:0] 0x2358
    0x44C, // U16 GCLK2_SeqVertical_End;   //BK23_2D[11:0] 0x235A
    // GCLK3
    0x44B, // U16 GCLK3_1stLn_Start;   //BK23_31[11:0] 0x2362
    0x31F, // U16 GCLK3_1stLn_End;     //BK23_32[11:0] 0x2364
    0x44B, // U16 GCLK3_OthLn_Start;   //BK23_30[11:0] 0x2360
    0x31F, // U16 GCLK3_OthLn_End;     //BK23_33[11:0] 0x2366
    0x44B, // U16 GCLK3_EndLn_Start;   //BK23_58[11:0] 0x23B0
    0x31F, // U16 GCLK3_EndLn_End;     //BK23_59[11:0] 0x23B2
    0x015, // U16 GCLK3_SeqVertical_Start; //BK23_34[11:0] 0x2368
    0x44D, // U16 GCLK3_SeqVertical_End;   //BK23_35[11:0] 0x236A
    // GCLK4
    0x44B, // U16 GCLK4_1stLn_Start;   //BK23_39[11:0] 0x2372
    0x31F, // U16 GCLK4_1stLn_End;     //BK23_3A[11:0] 0x2374
    0x44B, // U16 GCLK4_OthLn_Start;   //BK23_38[11:0] 0x2370
    0x31F, // U16 GCLK4_OthLn_End;     //BK23_3B[11:0] 0x2376
    0x44B, // U16 GCLK4_EndLn_Start;   //BK23_5A[11:0] 0x23B4
    0x31F, // U16 GCLK4_EndLn_End;     //BK23_5B[11:0] 0x23B6
    0x010, // U16 GCLK4_SeqVertical_Start; //BK23_3C[11:0] 0x2378
    0x44E, // U16 GCLK4_SeqVertical_End;   //BK23_3D[11:0] 0x237A
    // GCLK5
    0x44B, // U16 GCLK5_1stLn_Start;   //BK23_41[11:0] 0x2382
    0x31F, // U16 GCLK5_1stLn_End;     //BK23_42[11:0] 0x2384
    0x44B, // U16 GCLK5_OthLn_Start;   //BK23_40[11:0] 0x2380
    0x31F, // U16 GCLK5_OthLn_End;     //BK23_43[11:0] 0x2386
    0x44B, // U16 GCLK5_EndLn_Start;   //BK23_5C[11:0] 0x23B8
    0x31F, // U16 GCLK5_EndLn_End;     //BK23_5D[11:0] 0x23BA
    0x011, // U16 GCLK5_SeqVertical_Start; //BK23_44[11:0] 0x2388
    0x449, // U16 GCLK5_SeqVertical_End;   //BK23_45[11:0] 0x238A
    // GCLK6
    0x44B, // U16 GCLK6_1stLn_Start;   //BK23_5E[11:0] 0x23BC
    0x31F, // U16 GCLK6_1stLn_End;     //BK23_5F[11:0] 0x23BE
    0x44B, // U16 GCLK6_OthLn_Start;   //BK23_6A[11:0] 0x23D4
    0x31F, // U16 GCLK6_OthLn_End;     //BK23_6B[11:0] 0x23D6
    0x44B, // U16 GCLK6_EndLn_Start;   //BK23_6C[11:0] 0x23D8
    0x31F, // U16 GCLK6_EndLn_End;     //BK23_6D[11:0] 0x23DA
    0x012, // U16 GCLK6_SeqVertical_Start; //BK23_49[11:0] 0x2392
    0x44A, // U16 GCLK6_SeqVertical_End;   //BK23_51[11:0] 0x23A2
    // VDD_ODD
    0x000, // U16 VDD_ODD_Vertical_Start;   //BK23_67[11:0] 0x23CE
    0x000, // U16 VDD_ODD_Vertical_End;     //BK23_68[11:0] 0x23D0
    0x064, // U16 VDD_ODD_Horizontal_Start; //BK23_65[11:0] 0x23CA
    0x080, // U16 VDD_ODD_Horizontal_End;   //BK23_66[11:0] 0x23CC
     0xD8, // U8  VDD_ODD_nFrameTog;        //L:BK23_60[15:12] H:BK23_61[15:12] 0x23C1, 0x23C3
    // VDD_EVEN
    0x000, // U16 VDD_EVEN_Vertical_Start;  //BK23_62[11:0] 0x23C4
    0x000, // U16 VDD_EVEN_Vertical_End;    //BK23_63[11:0] 0x23C6
    0x0C8, // U16 VDD_EVEN_Horizontal_Start;//BK23_60[11:0] 0x23C0
    0x01E, // U16 VDD_EVEN_Horizontal_End;  //BK23_61[11:0] 0x23C2
     0xD8, // U8  VDD_EVEN_nFrameTog;       //L:BK23_65[15:12] H:BK23_66[15:12] 0x23CB, 0x23CD
        1, // U8  REG_2312_BIT5;            //BK23_09[5]
   0x02C0, // U16 OUT_MUX;                  //BK31_10
    },

};

void MDrv_MFC_LoadGipTiming(TCON_PANEL_TYPE idx)
{
    // SOE
    MDrv_MFC_Write2BytesMask(0x2320, tInitGipTiming[idx].SOE_Start              , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2322, tInitGipTiming[idx].SOE_End                , 0x0FFF);
    // POL
    MDrv_MFC_Write2BytesMask(0x2338, tInitGipTiming[idx].POL_Start              , 0x0FFF);
    // VST
    MDrv_MFC_Write2BytesMask(0x2394, tInitGipTiming[idx].VST_VerticalStart      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23A4, tInitGipTiming[idx].VST_VerticalEnd        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23DC, tInitGipTiming[idx].VST_HorizontalStart    , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23DE, tInitGipTiming[idx].VST_HorizontalEnd      , 0x0FFF);
    MDrv_MFC_WriteByteMask(0x23DD, ((tInitGipTiming[idx].VST_HorizontalWidth)<<4) , 0xF0);
    // GCLK1
    MDrv_MFC_Write2BytesMask(0x2342, tInitGipTiming[idx].GCLK1_1stLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2344, tInitGipTiming[idx].GCLK1_1stLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2340, tInitGipTiming[idx].GCLK1_OthLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2346, tInitGipTiming[idx].GCLK1_OthLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23A8, tInitGipTiming[idx].GCLK1_EndLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23AA, tInitGipTiming[idx].GCLK1_EndLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2348, tInitGipTiming[idx].GCLK1_SeqVertical_Start, 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x234A, tInitGipTiming[idx].GCLK1_SeqVertical_End  , 0x0FFF);
    // GCLK2
    MDrv_MFC_Write2BytesMask(0x2352, tInitGipTiming[idx].GCLK2_1stLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2354, tInitGipTiming[idx].GCLK2_1stLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2350, tInitGipTiming[idx].GCLK2_OthLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2356, tInitGipTiming[idx].GCLK2_OthLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23AC, tInitGipTiming[idx].GCLK2_EndLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23AE, tInitGipTiming[idx].GCLK2_EndLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2358, tInitGipTiming[idx].GCLK2_SeqVertical_Start, 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x235A, tInitGipTiming[idx].GCLK2_SeqVertical_End  , 0x0FFF);
    // GCLK3
    MDrv_MFC_Write2BytesMask(0x2362, tInitGipTiming[idx].GCLK3_1stLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2364, tInitGipTiming[idx].GCLK3_1stLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2360, tInitGipTiming[idx].GCLK3_OthLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2366, tInitGipTiming[idx].GCLK3_OthLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23B0, tInitGipTiming[idx].GCLK3_EndLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23B2, tInitGipTiming[idx].GCLK3_EndLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2368, tInitGipTiming[idx].GCLK3_SeqVertical_Start, 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x236A, tInitGipTiming[idx].GCLK3_SeqVertical_End  , 0x0FFF);
    // GCLK4
    MDrv_MFC_Write2BytesMask(0x2372, tInitGipTiming[idx].GCLK4_1stLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2374, tInitGipTiming[idx].GCLK4_1stLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2370, tInitGipTiming[idx].GCLK4_OthLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2376, tInitGipTiming[idx].GCLK4_OthLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23B4, tInitGipTiming[idx].GCLK4_EndLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23B6, tInitGipTiming[idx].GCLK4_EndLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2378, tInitGipTiming[idx].GCLK4_SeqVertical_Start, 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x237A, tInitGipTiming[idx].GCLK4_SeqVertical_End  , 0x0FFF);
    // GCLK5
    MDrv_MFC_Write2BytesMask(0x2382, tInitGipTiming[idx].GCLK5_1stLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2384, tInitGipTiming[idx].GCLK5_1stLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2380, tInitGipTiming[idx].GCLK5_OthLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2386, tInitGipTiming[idx].GCLK5_OthLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23B8, tInitGipTiming[idx].GCLK5_EndLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23BA, tInitGipTiming[idx].GCLK5_EndLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2388, tInitGipTiming[idx].GCLK5_SeqVertical_Start, 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x238A, tInitGipTiming[idx].GCLK5_SeqVertical_End  , 0x0FFF);
    // GCLK6
    MDrv_MFC_Write2BytesMask(0x23BC, tInitGipTiming[idx].GCLK6_1stLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23BE, tInitGipTiming[idx].GCLK6_1stLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23D4, tInitGipTiming[idx].GCLK6_OthLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23D6, tInitGipTiming[idx].GCLK6_OthLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23D8, tInitGipTiming[idx].GCLK6_EndLn_Start      , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23DA, tInitGipTiming[idx].GCLK6_EndLn_End        , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x2392, tInitGipTiming[idx].GCLK6_SeqVertical_Start, 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23A2, tInitGipTiming[idx].GCLK6_SeqVertical_End  , 0x0FFF);
    // VDD_ODD
    MDrv_MFC_Write2BytesMask(0x23CE, tInitGipTiming[idx].VDD_ODD_Vertical_Start , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23D0, tInitGipTiming[idx].VDD_ODD_Vertical_End , 0x0FFF);
    MDrv_MFC_Write2Bytes(0x23CA, ((0x000F&tInitGipTiming[idx].VDD_EVEN_nFrameTog)<<12)|(tInitGipTiming[idx].VDD_ODD_Horizontal_Start));
    MDrv_MFC_Write2Bytes(0x23CC, ((0x00F0&tInitGipTiming[idx].VDD_EVEN_nFrameTog)<<8)|(tInitGipTiming[idx].VDD_ODD_Horizontal_End));
    // VDD_EVEN
    MDrv_MFC_Write2BytesMask(0x23C4, tInitGipTiming[idx].VDD_EVEN_Vertical_Start , 0x0FFF);
    MDrv_MFC_Write2BytesMask(0x23C6, tInitGipTiming[idx].VDD_EVEN_Vertical_End , 0x0FFF);
    MDrv_MFC_Write2Bytes(0x23C0, ((0x000F&tInitGipTiming[idx].VDD_ODD_nFrameTog)<<12)|(tInitGipTiming[idx].VDD_EVEN_Horizontal_Start));
    MDrv_MFC_Write2Bytes(0x23C2, ((0x00F0&tInitGipTiming[idx].VDD_ODD_nFrameTog)<<8)|(tInitGipTiming[idx].VDD_EVEN_Horizontal_End));
    // Others
    MDrv_MFC_WriteBit(0x2312, tInitGipTiming[idx].REG_2312_BIT5, _BIT5);
    MDrv_MFC_Write2Bytes(0x3120, tInitGipTiming[idx].OUT_MUX);
}

void msInitializeColorMatrix(void)
{
	U8 i;

	MDrv_MFC_WriteBit(0x20C0,  gmfcSysInfo.u8PanelCSC, _BIT0); // [0]:CSC [1]:dither [2]:round
	MDrv_MFC_WriteByte(0x3074, 0x00); // disable dither 6bit enable
	MDrv_MFC_WriteByte(0x3075, 0x2d);
	for (i=0; i<tInitializeColorMatrix_count/*sizeof(tInitializeColorMatrix)*/; i++) // 0x3002 ~ 0x301d
	{
	    if((0x3002+i)==0x3017)
        	MDrv_MFC_WriteByteMask(0x3002+i, tInitializeColorMatrix[i], 0x7F);
	    else
        	MDrv_MFC_WriteByte(0x3002+i, tInitializeColorMatrix[i]);
    }

    MDrv_MFC_WriteBit(0x3002,  gmfcSysInfo.u8PanelCSC, _BIT3); // [3]:Cm En
    MDrv_MFC_WriteBit(0x3016,  gmfcSysInfo.u8PanelCSC, _BIT1); // [1]:Bri En


    //printf("\r\nmsInitializeColorMatrix()");
}

void msInitializeTcon(void)
{
#if((CODESIZE_SEL == CODESIZE_ALL) && (S7M_CHIP==0))
    U16 waitCnt=5000;
#endif
    //if(gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP)
    //    MDrv_MFC_WriteByte(0x1E0F, 0x10); //HEMAN

	//MDrv_MFC_WriteBit(0x2330, 1, _BIT4);//GOE polarity swap-----I-Chang 0829
	//MDrv_MFC_WriteByte(0x23F0, 0xC0);//reg_ctrl_low_sel and reg_de_delay-----I-Chang 0909
	MDrv_MFC_WriteBit(0x23F0, 1, _BIT3);//reg_ctrl_low_sel-----I-Chang 0909
	MDrv_MFC_WriteBit(0x23F0, 1, _BIT2);//reg_de_delay-----I-Chang 0909
	MDrv_MFC_WriteBit(0x23F0, 0, _BIT1);//reg_de_delay-----I-Chang 0909
	MDrv_MFC_WriteBit(0x23F0, 0, _BIT0);//reg_de_delay-----I-Chang 0909
	MDrv_MFC_WriteBit(0x2313, 1, _BIT7);//reg_tp_md_sel-----I-Chang 0909
	MDrv_MFC_WriteBit(0x230F, 1, _BIT3);//Request by Bryan, control load-----I-Chang 0901

 //   if(gmfcSysInfo.u8PanelType != _MINI_LVDS_GIP && gmfcSysInfo.u8PanelType != _MINI_LVDS_GIP_V5)
//        MDrv_MFC_WriteBit(0x238C, 0, _BIT0); //Output Enable FLK //j081031    //andy.jeong_20091203

	MDrv_MFC_WriteByte(0x2398, 0x00);
	MDrv_MFC_WriteByte(0x2399, 0x00);
	MDrv_MFC_WriteByte(0x23A6, 0xFF);
	MDrv_MFC_WriteByte(0x23A7, 0x00);
	MDrv_MFC_WriteByte(0x23E0, 0xFF);
	MDrv_MFC_WriteByte(0x23E1, 0x00);
	MDrv_MFC_WriteByte(0x23EC, 0xFF);
	MDrv_MFC_WriteByte(0x23ED, 0x00);
	//msWriteBit(0x2330, 0, _BIT4);//GOE polarity swap-----I-Chang 0829
	//printf("\r\nmsInitializeTcon()");
	MDrv_MFC_WriteBit(0x2300, gmfcSysInfo.u8PanelBlankCPVC, _BIT6);
	MDrv_MFC_WriteBit(0x2300, gmfcSysInfo.u8PanelBlankOEC, _BIT5);
	MDrv_MFC_WriteBit(0x2300, gmfcSysInfo.u8PanelBlankTPC, _BIT4);
	MDrv_MFC_WriteBit(0x2300, gmfcSysInfo.u8PanelBlankSTHC, _BIT3);

	if(gmfcSysInfo.u8PanelType==_MINI_LVDS)
	{
		MDrv_MFC_WriteRegsTbl(0x2200, tInitializeTcon22b_Comm); // initialize all of bank
		MDrv_MFC_WriteRegsTbl(0x2300, tInitializeTcon23b_Comm); // initialize all of bank

		MDrv_MFC_Write2Bytes(0x2310, gmfcSysInfo.u16Width/2);
		MDrv_MFC_Write2Bytes(0x2314, gmfcSysInfo.u16Height);
		MDrv_MFC_Write2Bytes(0x2318, gmfcSysInfo.u16HTotal);
		MDrv_MFC_Write2Bytes(0x231C, gmfcSysInfo.u16VTotal);
            MDrv_MFC_WriteRegsTbl(0x2300, tInitializeTcon23b_55);

#if(S7M_CHIP)
        //temporary release
        #if 0//!MFC_USE_IN_BOOLLOAD
            MDrv_MFC_LoadTconTiming(gmfcSysInfo.eTconType);
        #else
            MDrv_MFC_LoadTconTiming(TCON_DEFAULT_120);//temporary release
        #endif
#else
        /*//temporary release
        while(waitCnt--)
        {
            if(_bit3_(MDrv_MFC_ReadByte(0x2C45)))
            {
                MDrv_MFC_LoadTconTiming(MDrv_MFC_ReadByte(0x2C4F));
                break;
            }
            mfcSleepMsNop(1);
        }
        if(waitCnt==0)
        */
        MDrv_MFC_LoadTconTiming(TCON_DEFAULT_120);
#endif
	}
	else if(gmfcSysInfo.u8PanelType==_MINI_LVDS_GIP ||gmfcSysInfo.u8PanelType==_MINI_LVDS_GIP_V5)
	{
		MDrv_MFC_WriteRegsTbl(0x2200, tInitializeTcon22c); // initialize all of bank
		MDrv_MFC_WriteRegsTbl(0x2300, tInitializeTcon23c); // initialize all of bank

#if(S7M_CHIP)
        //temporary release
        #if 0//!MFC_USE_IN_BOOLLOAD
            MDrv_MFC_LoadGipTiming(gmfcSysInfo.eTconType);
        #else
            MDrv_MFC_LoadGipTiming(GIP_DEFAULT_120);//temporary release
        #endif
#else
        /*//temporary release
        while(waitCnt--)
        {
            if(_bit3_(MDrv_MFC_ReadByte(0x2C45)))
            {
                MDrv_MFC_LoadTconTiming(MDrv_MFC_ReadByte(0x2C4F));
                break;
            }
            mfcSleepMsNop(1);
        }
        if(waitCnt==0)
        */
        MDrv_MFC_LoadGipTiming(GIP_DEFAULT_120);
#endif

		{
            // Fitch T cont V5 setting
            // 20090810
            MDrv_MFC_WriteBit(0x3240,  1, _BIT7);

            MDrv_MFC_WriteByte(0x1E3F, 0x60);

            #if (ENABLE_Mst_func3_PWM_Freq)
            //updatePwmFreq=120;
            MDrv_MFC_WriteByte(0x2C4E, 100);
            //appPWMHandler();
            #endif
		}
        //MDrv_MFC_WriteByte(0x23F5, 0x00); //POL
	}

    //MDrv_MFC_WriteByte(0x23F9, 0x20);
	MDrv_MFC_WriteBit(0x23F9, 1, _BIT5); //Tc_cnt_en       //j091217
    MDrv_MFC_WriteBit(0x23F9, 1, _BIT6); // GPO_CLK Gating(clk off)
    MDrv_MFC_WriteBit(0x1E0F, 0, _BIT4);
}
#define OD_DEBUG 0
#if (OD_MODE_SEL != OD_MODE_OFF )
BOOL _waitWriteODReady(U16 reg)
{
    U16 waitCnt;
	for(waitCnt=0;waitCnt<10000;waitCnt++)
	{        
        if(_bit7_(MDrv_MFC_ReadByte(reg))==0)
        {
            waitCnt=0xffff;
            break;
        }
        mfcSleepMs(1);
    }
    if(waitCnt!=0xffff)
    {
        //printf("\n\r ^R^ ---------------  ODC table write fail!!!! --------------- ");
        return FALSE;
    }
    return TRUE;
}

#if OD_DEBUG
BOOL _waitReadODReady(U16 reg)
{
    U16 waitCnt;
    for(waitCnt=0;waitCnt<10000;waitCnt++)
    {
        if(_bit6_(MDrv_MFC_ReadByte(reg))==0)
        {
            waitCnt=0xffff;
            break;
        }
        mfcSleepMs(10);
    }
    if(waitCnt!=0xffff)
    {
        //printf("\n\r ^R^ ---------------  ODC table read fail!!!! --------------- ");
        return FALSE;
    }
    return TRUE;
}
#endif

void MDrv_MFC_InitializeOD(const U8* pODTbl)
{

	U8 ucVal;
	U32 wCount;
    U8 ucTARGET;
    U8 ucData;//, ucTemp=0;

    // Disable OD
    MDrv_MFC_WriteByteMask(0x2820, 0, _BIT0);

	// od_top clock enable
	MDrv_MFC_WriteByte(0x2802, 0x0e); // sram io enable
	MDrv_MFC_WriteByte(0x2803, 0x00); // sram io enable

    // Uncompressed mode
    //printf("^R^==============Group 2======================================\n\r");
    ucTARGET=*(pODTbl+272+19);// 20th
    for (wCount=0; wCount<272; wCount++)
    {
        ucData = (wCount == 19)?ucTARGET:(ucTARGET ^ *(pODTbl+272+wCount));
        MDrv_MFC_WriteByte(0x280C, ucData);
        MDrv_MFC_Write2Bytes(0x280A, wCount|0x8000);
		_waitWriteODReady(0x280B);
        #if OD_DEBUG
            //Read
            MDrv_MFC_WriteByte(0x2802, 0x08);
            MDrv_MFC_Write2Bytes(0x280A, wCount|0x4000);
            _waitReadODReady(0x280B);

            if(wCount!=0 && (wCount &0x000f)==0)
                printf("\n");
            printf("%d\t", ucData);
            if(ucData != MDrv_MFC_ReadByte(0x280E))
                printf("group 2 OD table error [%d]=[%d]",ucData, MDrv_MFC_ReadByte(0x280E));
        #endif
    }
    //printf("^R^==============Group 3======================================\n\r");
    ucTARGET=*(pODTbl+272*2+29);// 30th
    for (wCount=0; wCount<256; wCount++)
	{
        ucData = (wCount == 29)?ucTARGET:(ucTARGET ^ *(pODTbl+272*2+wCount));
        MDrv_MFC_WriteByte(0x2812, ucData);
        MDrv_MFC_Write2Bytes(0x2810, wCount|0x8000);
	    _waitWriteODReady(0x2811);
        #if OD_DEBUG
            //Read
            MDrv_MFC_WriteByte(0x2802, 0x08);
            MDrv_MFC_Write2Bytes(0x2810, wCount|0x4000);
            _waitReadODReady(0x2811);
            if(wCount!=0 && (wCount &0x000f)==0)
                printf("\n");
            printf("%d\t", ucData);
            if(ucData != MDrv_MFC_ReadByte(0x2814))
                printf("group 3 OD table error [%d]=[%d]",ucData, MDrv_MFC_ReadByte(0x2814));
        #endif
	}
    //printf("^R^==============Group 4======================================\n\r");
    ucTARGET=*(pODTbl+272*2+256+39);// 40th
    for (wCount=0; wCount<256; wCount++)
    {
        ucData = (wCount == 39)?ucTARGET:(ucTARGET ^ *(pODTbl+272*2+256+wCount));
        MDrv_MFC_WriteByte(0x2818,ucData );
        MDrv_MFC_Write2Bytes(0x2816, wCount|0x8000);
	    _waitWriteODReady(0x2817);
        #if OD_DEBUG
            //Read
            MDrv_MFC_WriteByte(0x2802, 0x08);
            MDrv_MFC_Write2Bytes(0x2816, wCount|0x4000);
            _waitReadODReady(0x2817);

            if(wCount!=0 && (wCount &0x000f)==0)
                printf("\n");
            printf("%d\t", ucData);
            if(ucData != MDrv_MFC_ReadByte(0x281A))
                printf("group 4 OD table error [%d]=[%d]",ucData, MDrv_MFC_ReadByte(0x281A));
        #endif
	}

    //printf("^R^==============Group 1======================================\n\r");
    ucTARGET=*(pODTbl+9);// 10th
    for (wCount=0; wCount<272; wCount++)
	{
        ucData = (wCount == 9)?ucTARGET:(ucTARGET ^ *(pODTbl+wCount));
    #if 0 //U02 chip with RX TTL interface 01192010
        if((gmfcSysInfo.u8ChipRevision < MST_3xxx_U03) && S7M_RX_TTL)
        {
            //********************************************************************Group 1
            if(wCount%4==0  && (((wCount%64)>>2)<4) )
            {
                //printf("preData_[%d]=", ucData);
        ucTemp = (ucData%4)>=2 ? 1: 0;
        ucData = (ucData >>2) + ucTemp;
        ucData = (ucData >= 0x3F)?0x3F:ucData;
                //printf("posData_[%d]*****", ucData);
            MDrv_MFC_WriteByte(0x2806, (ucData));
    		MDrv_MFC_Write2Bytes(0x2804, (wCount>>2)|0x8000);
                //printf("num_[%d]=adr_[%d]\n ", (wCount), (wCount>>2));
        }
            else if((wCount%4==0  && (((wCount%64)>>2)==4))||(wCount ==271))
            {
                if(wCount ==271)
        {
            MDrv_MFC_WriteByte(0x2806, 0x3f);
        		    MDrv_MFC_Write2Bytes(0x2804, ((wCount>>2)+1)|0x8000);
                    //printf("preData_[%d]=posData_[%d]*****", ucData, 0x3f);
                    //printf("num_[%d]=adr_[%d] \n", (wCount), ((wCount>>2)+1));
        }
                else
        {
            MDrv_MFC_WriteByte(0x2806, 0x3f);
            		MDrv_MFC_Write2Bytes(0x2804, (wCount>>2)|0x8000);
                    //printf("preData_[%d]=posData_[%d]*****", ucData, 0x3f);
                    //printf("num_[%d]=adr_[%d] \n\n\n\n\n\n\n\n\n", (wCount), (wCount>>2));
                }
        }
        else
        {
            MDrv_MFC_WriteByte(0x2806, ucData);
    		MDrv_MFC_Write2Bytes(0x2804, wCount|0x8000);
        }

        _waitWriteODReady(0x2805);

            //********************************************************************Group 2
            if(wCount%4==2  && (((wCount%64)>>2)<4) )
    {
                //printf("preData_[%d]=", ucData);
        ucTemp = (ucData%4)>=2 ? 1: 0;
        ucData = (ucData >>2) + ucTemp;
        ucData = (ucData >= 0x3F)?0x3F:ucData;
                //printf("posData_[%d]*****", ucData);
                MDrv_MFC_WriteByte(0x280C, (ucData));
    		MDrv_MFC_Write2Bytes(0x280A, (wCount>>2)|0x8000);
                //printf("num_[%d]=adr_[%d]\n ", (wCount), (wCount>>2));
        }
            else if((wCount%4==2  && (((wCount%64)>>2)==4) )||(wCount ==271))
            {
                if(wCount ==271)
                {
                    MDrv_MFC_WriteByte(0x280C, 0x3f);
                    MDrv_MFC_Write2Bytes(0x280A, ((wCount>>2)+1)|0x8000);
                    //printf("preData_[%d]=posData_[%d]*****", ucData, 0x3f);
                    //printf("num_[%d]=adr_[%d] \n", (wCount), ((wCount>>2)+1));
                }
                else
        {
            MDrv_MFC_WriteByte(0x280C, 0x3f);
    		MDrv_MFC_Write2Bytes(0x280A, (wCount>>2)|0x8000);
                //printf("preData_[%d]=posData_[%d]*****", ucData, 0x3f);
                //printf("num_[%d]=adr_[%d] \n\n\n\n\n\n\n\n\n", (wCount), (wCount>>2));
        }
            }
        else
        {
            MDrv_MFC_WriteByte(0x280C, ucData);
            MDrv_MFC_Write2Bytes(0x280A, wCount|0x8000);
        }
		_waitWriteODReady(0x280B);

            //********************************************************************Group 3
            if( wCount%4==0  && ( ((wCount%64)>>2)>=8 && ((wCount%64)>>2)<12 ) )
	{
                //printf("preData_[%d]=", ucData);
        ucTemp = (ucData%4)>=2 ? 1: 0;
        ucData = (ucData >>2) + ucTemp;
        ucData = (ucData >= 0x3F)?0x3F:ucData;
                //printf("posData_[%d]*****", ucData);
            MDrv_MFC_WriteByte(0x2812, (ucData));
        		MDrv_MFC_Write2Bytes(0x2810, ((wCount-32)>>2)|0x8000);
                //printf("num_[%d]=adr_[%d]\n ", (wCount), ((wCount-32)>>2));
        }
            else if(wCount%4==0  && (((wCount%64)>>2)==12) )
        {
            MDrv_MFC_WriteByte(0x2812, 0x3f);
        		MDrv_MFC_Write2Bytes(0x2810, ((wCount-32)>>2)|0x8000);
                //printf("preData_[%d]=posData_[%d]*****", ucData, 0x3f);
                //printf("num_[%d]=adr_[%d] \n\n\n\n\n\n\n\n\n", (wCount), ((wCount-32)>>2));
        }
        else
        {
            MDrv_MFC_WriteByte(0x2812, ucData);
            MDrv_MFC_Write2Bytes(0x2810, wCount|0x8000);
        }
	    _waitWriteODReady(0x2811);


            //********************************************************************Group 4
            if( wCount%4==2  && ( ((wCount%64)>>2)>=8 && ((wCount%64)>>2)<12 ) )
    {
                //printf("preData_[%d]=", ucData);
        ucTemp = (ucData%4)>=2 ? 1: 0;
        ucData = (ucData >>2) + ucTemp;
        ucData = (ucData >= 0x3F)?0x3F:ucData;
                //printf("posData_[%d]*****", ucData);
            MDrv_MFC_WriteByte(0x2818, (ucData));
        		MDrv_MFC_Write2Bytes(0x2816, ((wCount-32)>>2)|0x8000);
                //printf("num_[%d]=adr_[%d]\n ", (wCount), ((wCount-32)>>2));
        }
            else if(wCount%4==2  && (((wCount%64)>>2)==12) )
        {
                MDrv_MFC_WriteByte(0x2818, 0x3f);
        		MDrv_MFC_Write2Bytes(0x2816, ((wCount-32)>>2)|0x8000);
                //printf("preData_[%d]=posData_[%d]*****", ucData, 0x3f);
                //printf("num_[%d]=adr_[%d] \n\n\n\n\n\n\n\n\n", (wCount), ((wCount-32)>>2));
        }
        else
        {
            MDrv_MFC_WriteByte(0x2818,ucData );
            MDrv_MFC_Write2Bytes(0x2816, wCount|0x8000);
        }
	    _waitWriteODReady(0x2817);
        }
        else
    #endif
        {
            MDrv_MFC_WriteByte(0x2806, ucData);
		    MDrv_MFC_Write2Bytes(0x2804, wCount|0x8000);
            _waitWriteODReady(0x2805);
            #if OD_DEBUG
                //Read
                MDrv_MFC_WriteByte(0x2802, 0x08);
                MDrv_MFC_Write2Bytes(0x2804, wCount|0x4000);
                _waitReadODReady(0x2805);

                if(wCount!=0 && (wCount &0x000f)==0)
                    printf("\n");
                printf("%d\t", ucData);
                if(ucData != MDrv_MFC_ReadByte(0x2808))
                    printf("group 1 OD table error [%d]=[%d]",ucData, MDrv_MFC_ReadByte(0x2808));
            #endif
        }
	}
	MDrv_MFC_WriteByte(0x2802, 0x00); // sram io disable
	MDrv_MFC_WriteByte(0x2803, 0x00); // sram io disable
	MDrv_MFC_WriteByte(0x2823, 0x5f); //[3:0] od_user_weight, [7:4] b_weight
    #if 0 //U02 chip with RX TTL interface 01192010
    if((gmfcSysInfo.u8ChipRevision< MST_3xxx_U03) && S7M_RX_TTL)
	    MDrv_MFC_WriteByte(0x2824, 0x03); // [7:0] od active threshold
	else
    #endif
	MDrv_MFC_WriteByte(0x2824, 0x0c); // [7:0] od active threshold
	// [7:0] Even request base address low byte
	MDrv_MFC_WriteByte(0x282A, (U8)(gmfcMiuBaseAddr.u32OdBaseEven>>4));
	// [7:0] Even request base address med byte
	MDrv_MFC_WriteByte(0x282B, (U8)((gmfcMiuBaseAddr.u32OdBaseEven>>4)>>8));
	// [7:0] Even request base address high byte
	MDrv_MFC_WriteByte(0x282C, (U8)((gmfcMiuBaseAddr.u32OdBaseEven>>4)>>16));

	// [7:0] request limit address low byte
	MDrv_MFC_WriteByte(0x282E, (U8)(gmfcMiuBaseAddr.u32OdLimitEven>>4));
	// [7:0] request limit address med byte
	MDrv_MFC_WriteByte(0x282F, (U8)((gmfcMiuBaseAddr.u32OdLimitEven>>4)>>8));
	// [7:0] request limit address high byte
	MDrv_MFC_WriteByte(0x2830, (U8)((gmfcMiuBaseAddr.u32OdLimitEven>>4)>>16));

	// [7:0] reg_od_wadr_max_limit low byte
	MDrv_MFC_WriteByte(0x2872, (U8)((gmfcMiuBaseAddr.u32OdSizehalf+0x7800)>>4));
	// [7:0] reg_od_wadr_max_limit med byte
	MDrv_MFC_WriteByte(0x2873, (U8)(((gmfcMiuBaseAddr.u32OdSizehalf+0x7800)>>4)>>8));
	// [7:0] reg_od_wadr_max_limit high byte
	MDrv_MFC_WriteByte(0x2874, (U8)(((gmfcMiuBaseAddr.u32OdSizehalf+0x7800)>>4)>>16));

	// [7:0] reg_od_radr_max_limit low byte
	MDrv_MFC_WriteByte(0x2876, (U8)((gmfcMiuBaseAddr.u32OdSizehalf+0x7800)>>4));
	// [7:0] reg_od_radr_max_limit med byte
	MDrv_MFC_WriteByte(0x2877, (U8)(((gmfcMiuBaseAddr.u32OdSizehalf+0x7800)>>4)>>8));
	// [7:0] reg_od_radr_max_limit high byte
	MDrv_MFC_WriteByte(0x2878, (U8)(((gmfcMiuBaseAddr.u32OdSizehalf+0x7800)>>4)>>16));

	// [7:0] Odd request base address low byte
	MDrv_MFC_WriteByte(0x288E, (U8)(gmfcMiuBaseAddr.u32OdBaseOdd>>4));
	// [7:0] Odd request base address med byte
	MDrv_MFC_WriteByte(0x288F, (U8)((gmfcMiuBaseAddr.u32OdBaseOdd>>4)>>8));
	// [7:0] Odd request base address high byte
	MDrv_MFC_WriteByte(0x2890, (U8)((gmfcMiuBaseAddr.u32OdBaseOdd>>4)>>16));

	// [7:0] request limit address low byte
	MDrv_MFC_WriteByte(0x2891, (U8)(gmfcMiuBaseAddr.u32OdLimitOdd>>4));
	// [7:0] request limit address med byte
	MDrv_MFC_WriteByte(0x2892, (U8)((gmfcMiuBaseAddr.u32OdLimitOdd>>4)>>8));
	// [7:0] request limit address high byte
	MDrv_MFC_WriteByte(0x2893, (U8)((gmfcMiuBaseAddr.u32OdLimitOdd>>4)>>16));

	MDrv_MFC_WriteByte(0x2832, 0x30); // [7:0] reg_od_r_thrd
	MDrv_MFC_WriteByte(0x2833, 0x7e); // [7:0] reg_od_wff_ack_thrd
	MDrv_MFC_WriteByte(0x2834, 0x20); // [7:0] reg_od_r_thrd2
	MDrv_MFC_WriteByte(0x2835, 0x50); // [7:0] reg_od_r_hpri
	MDrv_MFC_WriteByte(0x2836, 0x30); // [7:0] reg_od_w_thrd
	MDrv_MFC_WriteByte(0x2837, 0x04); // [7:0] reg_od_wlast_fire_thrd
	MDrv_MFC_WriteByte(0x2838, 0x20); // [7:0] reg_od_w_thrd2
	MDrv_MFC_WriteByte(0x2839, 0x50); // [7:0] reg_od_w_hpri
	MDrv_MFC_WriteByte(0x2841, 0x00); // od request space stop cnt
	MDrv_MFC_WriteByte(0x285C, 0x80); // [7:0] reg_patchTh0
	MDrv_MFC_WriteByte(0x285D, 0x00); // [5:0] reg_patchTh1 : bias offset
    // [6] reg_patchTh1 : patch enable
	MDrv_MFC_WriteByte(0x285E, 0x88); // [3:0] reg_patchTh2
    // [7:4] reg_patchTh3
	MDrv_MFC_WriteByte(0x2866, 0x10); // [7:0] reg_min3x3Length
	MDrv_MFC_WriteByte(0x2867, 0x40); // [7:0] reg_max3x3Length

	MDrv_MFC_WriteByte(0x2875, 0x02); // [13:8] reg_od_strength_slop [14] reg_od_strength_bypass

	// [7:0] reg_od_mem_adr_limit low byte
	MDrv_MFC_WriteByte(0x2859, (U8)(gmfcMiuBaseAddr.u32OdSize>>4));
	// [7:0] reg_od_mem_adr_limit med byte
	MDrv_MFC_WriteByte(0x285A, (U8)((gmfcMiuBaseAddr.u32OdSize>>4)>>8));
	// [7:0] reg_od_mem_adr_limit high byte
	MDrv_MFC_WriteByte(0x285B, (U8)((gmfcMiuBaseAddr.u32OdSize>>4)>>16));

	// lsb request base address low byte
	MDrv_MFC_WriteByte(0x289E, (U8)(gmfcMiuBaseAddr.u32OdLsbBase>>4));
	// lsb request base address med byte
	MDrv_MFC_WriteByte(0x289F, (U8)((gmfcMiuBaseAddr.u32OdLsbBase>>4)>>8));
	// lsb request base address high byte
	MDrv_MFC_WriteByte(0x28A0, (U8)((gmfcMiuBaseAddr.u32OdLsbBase>>4)>>16));

	// lsb request limit address low byte
	MDrv_MFC_WriteByte(0x28A1, (U8)(gmfcMiuBaseAddr.u32OdLsbLimit>>4));
	// lsb request limit address med byte
	MDrv_MFC_WriteByte(0x28A2, (U8)((gmfcMiuBaseAddr.u32OdLsbLimit>>4)>>8));
	// lsb request limit address high byte
	MDrv_MFC_WriteByte(0x28A3, (U8)((gmfcMiuBaseAddr.u32OdLsbLimit>>4)>>16));

	MDrv_MFC_WriteByte(0x28A4, 0x20); // [7:0] reg_od_r_thrd_lsb
	MDrv_MFC_WriteByte(0x28A5, 0x30); // [7:0] reg_od_r_thrd2_lsb
	MDrv_MFC_WriteByte(0x28A6, 0x50); // [7:0] reg_od_r_hpri_lsb
	MDrv_MFC_WriteByte(0x28A7, 0x50); // [7:0] reg_od_w_hpri_lsb
	MDrv_MFC_WriteByte(0x28A8, 0x20); // [7:0] reg_od_w_thrd_lsb
	MDrv_MFC_WriteByte(0x28A9, 0x30); // [7:0] reg_od_w_thrd2_lsb
	MDrv_MFC_WriteByte(0x28AB, 0x14); // [3:0] reg_vsync_start_delay
    // [5:4] reg_vsync_width_delay    // [7:6] reg_vfend_delay
	if (gmfcMiuBaseAddr.u8OdMode==OD_MODE_666_COMPRESS)
		MDrv_MFC_WriteByte(0x289B, 0x25);
	else if (gmfcMiuBaseAddr.u8OdMode==OD_MODE_555_COMPRESS)
		MDrv_MFC_WriteByte(0x289B, 0x15);
	else
		MDrv_MFC_WriteByte(0x289B, 0x05);

	// [0]   reg_last_data_ctrl_en
    // [1]   reg_od1_last_dummy_pix_sel
    // [2]   reg_od1_last_rdy_sel
    // [6:4] reg_od_compress_mode
    // [7]   reg_od_lsb_wlast_force_req_disable
    MDrv_MFC_WriteByte(0x286C, 0x00);
#if 0
	MDrv_MFC_WriteByte(0x2824, 0x0c);  // od active threshold
	MDrv_MFC_WriteByte(0x2825, 0x00);  // od active threshold
	MDrv_MFC_WriteByte(0x2841, 0x00);  // od request space stop cnt
	MDrv_MFC_WriteByte(0x28AB, 0x14);  // od self generate vsync
	MDrv_MFC_WriteByte(0x286C, 0x07);  // [7:6] reg_od_status_sel
	// [5]   reg_od_read_over_disable
	// [4]   reg_od_read_over_sel
	// [3]   reg_od_status_rst
	// [2]   reg_od_rq_over_under_mask_en
	// [1]   reg_od_next_frame_en
	// [0]	 reg_od_active_sel

	MDrv_MFC_WriteByte(0x286D, 0x82);
	// [7] reg_od1_write_data_over_sel, compress bypaa mode need to set 0
	// [6:4] reg_od1_overflow_thrd,  compress mode check line buffer overflow threshold sel
	// [3:0] reg_od1_underflow_thrd, compress mode check line buffer underflow threshold

	MDrv_MFC_WriteByte(0x284B, 0x80);
	// [7] reg_od1_read_data_under_sel
	// [6] reg_od1_linebuf_bypass_en

	MDrv_MFC_WriteByte(0x2885, 0x80);  // [7:4] reg_od1_read_under_act_thrd
	MDrv_MFC_WriteByte(0x285F, 0xff);  // [7:0] reg_od1_rbuf_thrd
#endif
	ucVal = OD_MODE_SEL;
	ucVal &= 0x0F;
	MDrv_MFC_WriteByte(0x2820, 0x20|ucVal);
	// [0]   od_en
	// [3:1] od_mode , 000{444}, 001{565}, 010{y-8}, 011{333}, 100{666}, 101{compress}, 110{555}, 111{888}
	// [4]   reserved
	// [5]   reg_od_user_weight_sel
	// [6]   od_h_range_en
	// [7]   od_v_range_en

    //printf("MDrv_MFC_InitializeOD()\n");
}

#if OD_DEBUG
void Read_OD(void)
{
    U8 ucVal;
    U32 wCount;

    // Disable OD
    MDrv_MFC_WriteByteMask(0x2820, 0, _BIT0);

    // od_top clock enable
    MDrv_MFC_WriteByte(0x2802, 0x0e); // sram io enable
    MDrv_MFC_WriteByte(0x2803, 0x00); // sram io enable
    MDrv_MFC_WriteByte(0x2802, 0x08); // R enable

    printf("sram1:\n");
    for (wCount=0; wCount<272; wCount++)//---------------------------------Sram 1
    {
        //Read
        //MDrv_MFC_WriteByte(0x2802, 0x08); // R enable
        MDrv_MFC_Write2Bytes(0x2804, wCount|0x4000);
        _waitReadODReady(0x2805);

        if(wCount!=0 && (wCount &0x000f)==0)
                printf("\n");
        printf("%d\t", MDrv_MFC_ReadByte(0x2808));
    }
    printf("\nsram2:\n");
    for (wCount=0; wCount<272; wCount++)//---------------------------------Sram 2
    {
        //Read
        //MDrv_MFC_WriteByte(0x2802, 0x08); // R enable
        MDrv_MFC_Write2Bytes(0x280A, wCount|0x4000);
        _waitReadODReady(0x280B);

        if(wCount!=0 && (wCount &0x000f)==0)
                printf("\n");
        printf("%d\t", MDrv_MFC_ReadByte(0x280E));
    }
    printf("\nsram3:\n");
    for (wCount=0; wCount<256; wCount++)//---------------------------------Sram 3
    {
        //Read
        //MDrv_MFC_WriteByte(0x2802, 0x08); // R enable
        MDrv_MFC_Write2Bytes(0x2810, wCount|0x4000);
        _waitReadODReady(0x2811);

        if(wCount!=0 && (wCount &0x000f)==0)
                printf("\n");
        printf("%d\t", MDrv_MFC_ReadByte(0x2814));
    }
    printf("\nsram4:\n");
    for (wCount=0; wCount<256; wCount++)//---------------------------------Sram 4
    {
        //Read
        //MDrv_MFC_WriteByte(0x2802, 0x08); // R enable
        MDrv_MFC_Write2Bytes(0x2816, wCount|0x4000);
        _waitReadODReady(0x2817);

        if(wCount!=0 && (wCount &0x000f)==0)
                printf("\n");
        printf("%d\t", MDrv_MFC_ReadByte(0x281A));
    }
    MDrv_MFC_WriteByte(0x2802, 0x00); // sram io disable
    MDrv_MFC_WriteByte(0x2803, 0x00); // sram io disable

    ucVal = OD_MODE_SEL;
    ucVal &= 0x0F;
    MDrv_MFC_WriteByte(0x2820, 0x20|ucVal);

}
#endif
#endif

void MDrv_MFC_InitializePanel(void)
{
	msInitializeColorMatrix();

    MDrv_MFC_WriteByteMask(0x2C42, gmfcSysInfo.u8PanelType, 0x1F);
    MDrv_MFC_WriteByteMask(0x1E40, 0, 0x03); //PWM 0/1 Output Enable

	if (gmfcSysInfo.u8PanelType == _MINI_LVDS || gmfcSysInfo.u8PanelType == _RSDS || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP || gmfcSysInfo.u8PanelType == _MINI_LVDS_GIP_V5)
		msInitializeTcon();

    //LG will control the OD function not at initialize. 20091027
	#if ( (CODESIZE_SEL==CODESIZE_ALL) && (CODEBASE_SEL == CODEBASE_51)&&(OD_MODE_SEL != OD_MODE_OFF))
	if (gmfcMiuBaseAddr.u8OdMode != OD_MODE_OFF)
	{
		#if (PANEL_TYPE_SEL == PNL_LCEAll)
            if( MDrv_MFC_ReadByte(0x1E48)>0 && MDrv_MFC_ReadByte(0x1E48)<3 )
			{
                #if 0
                if(MDrv_MFC_ReadByte(0x1E48) == 1)//GIP V5
                    MDrv_MFC_InitializeOD(tODGIP);
                else if(MDrv_MFC_ReadByte(0x1E48) == 2)//non GIP
                    MDrv_MFC_InitializeOD(tOD55);
                #else
                    MDrv_MFC_InitializeOD(tOD_Array[MDrv_MFC_ReadByte(0x2C55)&0x0F]);
                #endif
			}
		#else
				MDrv_MFC_InitializeOD(tOverDrive);
		#endif
	}
	#endif

}

void MDrv_MFC_InitializeScTop2_Bypanel(void)
{
	MDrv_MFC_WriteRegsTbl(0x3200, tInitializeScTop2);
}

#if(CODESIZE_SEL == CODESIZE_ALL)
void MDrv_MFC_InitializeBypass(void)
{
	U8 i;
	//for bypass use
	//IP CSC off: 20C0[0] = 0
	MDrv_MFC_WriteBit(0x20C0,  0, _BIT0); // [0]:CSC [1]:dither [2]:round

	//MFC off: 290E[3:0] = 0.
	MDrv_MFC_WriteByteMask(0x290E, 0x00, 0x0F);

	//Color matrixes disable: 3002[3] = 0, 3003~3015 set to 0.
        MDrv_MFC_WriteBit(0x3002, 0, _BIT3);
        for (i=0; i<0x13; i++) // 0x3003 ~ 0x3015
            MDrv_MFC_WriteByte(0x3003+i, 0);

	//Brightness disable: 3016[1] = 0, 3018~301B set to 0.
	MDrv_MFC_WriteBit(0x3016, 0, _BIT1);
        for (i=0; i<0x4; i++) // 0x3018 ~ 0x301B
            MDrv_MFC_WriteByte(0x3018+i, 0);
}
#endif
#endif

#if ENABLE_Mst_func3_PWM_Freq
//==========================================================
//Description:
//ucGroupIndex(PWM0 ~PWM1)      : 0~1
//ucFrequency	 : PWM frequency
//bVsyncEn		 : PWM synchronize with Vsync
//Period			 : 18bit
//ucDuty			 : Duty= period/100*ucDuty  0~100%
//==========================================================
//#include "Uart.h"
//XDATA BOOL bPrePolarity;
#define PWM_POLLING_DELAY_10us 30
U16 reg_2540_PWM_Period_MSBExt = 0, reg_2542_PWM_Duty_MSBExt = 0;
void _setPWM(U8 u8GroupIndex , U8 u8Frequency, BOOL bVsyncEn, U32  u32Duty, U32 u32Shift, BOOL bShiftEn, BOOL bPolarity, BOOL enableOutput)
{
	U8 u8Divider=0/*, u8Temp=0*/;
	U16 u16Ext=0;
	U32 u32Period=0;
	U16 newShift, dutyIn360;
    //U8 u8Reg2401;

	MDrv_MFC_Write2BytesINT(0x1E3E , 0x6000); //PWM_SEL(only  pwm0/pwm1, others need check)
	MDrv_MFC_WriteByteMaskINT(0x2509+(u8GroupIndex*6) , _BIT2|_BIT1, _BIT2|_BIT1);  //[2,1]: Vsync reset En,double buffer En

    //printf("\n\r before ## u32Duty=%d", u32Duty);
    //printf(" u32Shift=%d", u32Shift);
    //printf(" bPolarity=%d", bPolarity);

#if 1
        dutyIn360=(u32Duty*36/10);
        if((dutyIn360+u32Shift)>360)
        {
            newShift = (dutyIn360+u32Shift)-360;
            u32Duty  = (u32Shift-newShift)*10/36;
            u32Shift = newShift;
            if(bPolarity)
                bPolarity=FALSE;
            else
                bPolarity=TRUE;
        }
    //printf("\n\r after ## u32Duty=%d", u32Duty);
    //printf(" u32Shift=%d", u32Shift);
    //printf(" bPolarity=%d", bPolarity);
#endif
	//Div
	u8Divider =1;
	MDrv_MFC_WriteByte(0x2508 + (u8GroupIndex*6), u8Divider-1); // [7:0] Divider

	//Check input V Frequecy
	if(bVsyncEn)
        u8Frequency = MDrv_MFC_ReadByte(0x2C49)<<1;//msCalculateDecimal(inputVFreq,1000)*10*2;
    else
	    u8Frequency = (u8Frequency == 0)? 1: u8Frequency;

    if(u8Frequency==0)
    {
        u8Frequency= 120;//120*1125/(MDrv_MFC_Read2Bytes(0x2F02)+1);
    }
    //printf(" u8Frequency=%d", u8Frequency);
    #if 0
    while (_bit1_(MDrv_MFC_ReadByte(0x2414)))
    {
        mfcSleep10usNop(PWM_POLLING_DELAY_10us);
        //mfcSleepMsNop(1);
    }
    mfcSleepMsNop(1);

    u8Reg2401=MDrv_MFC_ReadByte(0x2401);
    MDrv_MFC_SetInterrupt (_DISABLE);
    #endif

	//Period
	u32Period = msCalculateDecimal( (U32)PWM_CRYSTAL_CLOCK, (U32)u8Frequency*u8Divider);
    //printf(" u32Period=%d", u32Period);
	MDrv_MFC_Write2BytesINT(0x2504 + (u8GroupIndex*6), (u32Period & PERIOD_MASK)-1); 	//Period //Actual Value= Reg value +1
	u16Ext = (u32Period>>16) & 0x03;
    reg_2540_PWM_Period_MSBExt = (reg_2540_PWM_Period_MSBExt & (~(3 << (u8GroupIndex*2)))) | (u16Ext << (u8GroupIndex*2));
    //MDrv_MFC_WriteByteINT(0x2540 , (U8)u16Ext); //set if >2 pwm use
    //Duty
    u32Duty = msCalculateDecimal( u32Period*u32Duty, 100);
    //printf("  u32Duty = [%x]", u32Duty);
    #if 0
    while (!_bit1_(MDrv_MFC_ReadByte(0x2414)))
    {
        mfcSleep10usNop(PWM_POLLING_DELAY_10us);
        //mfcSleepMsNop(1);
    }
    #endif

    if(bPolarity)
    {
        //u8Temp |= 0x01;
        //printf(" bPolarity = [%X] \n\r", bPolarity);
        MDrv_MFC_WriteByteMaskINT(0x2509+(u8GroupIndex*6) , _BIT0, _BIT0);
    }
    else
    {
        //u8Temp &= ~0x01;
        //printf(" bPolarity = [%X] \n\r", bPolarity);
        MDrv_MFC_WriteByteMaskINT(0x2509+(u8GroupIndex*6) , 0, _BIT0);
    }

	//Shift
	if(bShiftEn)
	{
		//u32Shift = u32Period * ( u32Shift/360) ;
		u32Shift = msCalculateDecimal( (U32)u32Period*u32Shift, 360);
        //printf("  u32Shift = [%x]", u32Shift);
		MDrv_MFC_Write3BytesINT(0x2550 + (u8GroupIndex*4), u32Shift); //Set shift

		u32Duty =  u32Shift + u32Duty ;
	}
    else
        MDrv_MFC_Write3BytesINT(0x2550 + (u8GroupIndex*4),0);

    u16Ext = (u32Duty>>16) & 0x03;
    reg_2542_PWM_Duty_MSBExt = (reg_2542_PWM_Duty_MSBExt & (~(3 << (u8GroupIndex*2))))| (u16Ext << (u8GroupIndex*2));
	//u16Ext = (MDrv_MFC_Read2BytesINT(0x2542) & (~(3 << (u8GroupIndex*2))))| (u16Ext << (u8GroupIndex*2));
    //printf(" \n\r Pre u16Ext = [%X]", u16Ext);
    //printf(" u32Duty = [%X] \r", u32Duty-1);
	//MDrv_MFC_Write2BytesINT(0x2542 , u16Ext); //set duty extend
	MDrv_MFC_Write2BytesINT(0x2506 + (u8GroupIndex*6), u32Duty-1); //set duty
	//MDrv_MFC_Write2Bytes(0x2542 , u16Ext); //set duty extend
	//MDrv_MFC_Write2Bytes(0x2506 + (u8GroupIndex*6), u32Duty-1); //set duty
	//mfcSleepMsNop(16);
	//printf(" \n\n\n\nRead u16Ext = [%X] ", MDrv_MFC_ReadByte(0x2542));
	//printf(" u32Duty = [%X]\n\n\n\n ", MDrv_MFC_Read2Bytes(0x2506+ (u8GroupIndex*6)));


	//if(bVsyncEn)
        //u8Temp = 0x06;
		//MDrv_MFC_WriteByteMask(0x2509+(u8GroupIndex*6) , _BIT2|_BIT1, _BIT2|_BIT1);  //[2,1]: Vsync reset En,double buffer En



    //MDrv_MFC_WriteByte(0x2509+(u8GroupIndex*6) ,u8Temp);

    if(enableOutput)
    	MDrv_MFC_Write2BytesINT(0x1E40 , MDrv_MFC_Read2BytesINT(0x1E40) & (~(0x01 << u8GroupIndex))); //PWM_OEN	//set if >2 pwm use
    //bPrePolarity = bPolarity;

    #if 0
    if((u8Reg2401&_BIT0)==0) MDrv_MFC_SetInterrupt (_ENABLE);
    #endif
}
//#include "UART.h"
void MDrv_MFC_SetPWMFreq(U8 u8GroupIndex ,U8 u8Duty, U16 u16Shift, BOOL bEnable)
{
    if(u8GroupIndex<2)
    {
        // these value seems not correct, need test more.
        if(u8GroupIndex==0)
        {
            if(u8Duty==22) u8Duty=21;
        }
        else if(u8GroupIndex==1)
        {
            if(u8Duty==75) u8Duty=74;
        }

        u16Shift=((u16Shift+360-(360*u8Duty/100))%360);  // stand on falling edge.
//        printf("\n\r================ u8GroupIndex = %d", u8GroupIndex);
       //printf("   u8Duty = 0x%x", u8Duty);
//        printf("   u16Shift = 0x%x", u16Shift);
        if (u8Duty==0)
        {
            if ( _bit0_(MDrv_MFC_ReadByte(0x2509+(u8GroupIndex*6)))) //Neg POL ->0
            {
                //Shift = 0, Period = 0
                MDrv_MFC_Write3BytesINT(0x2550 + (u8GroupIndex*4), 0);
                MDrv_MFC_Write2BytesINT(0x2504 + (u8GroupIndex*6), 0);
                reg_2540_PWM_Period_MSBExt = reg_2540_PWM_Period_MSBExt & (~(3 << (u8GroupIndex*2)));
                //mfcSleepMsNop(5);   //0x2540[0] sometimes not disabled
            }
            else
            {
                if (( 0 == MDrv_MFC_Read2BytesINT(0x2504 + (u8GroupIndex*6)))
                  &&( 0 == MDrv_MFC_Read2BytesINT(0x2550 + (u8GroupIndex*4)))) //Pos100->0
                    //Polarity swap
                    MDrv_MFC_WriteByteMaskINT(0x2509+(u8GroupIndex*6) , 1, _BIT0);
                else                                                 //Pos POL->0
                {
                    //Period = 0
                    MDrv_MFC_Write2BytesINT(0x2504 + (u8GroupIndex*6), 0);
                    reg_2540_PWM_Period_MSBExt = reg_2540_PWM_Period_MSBExt & (~(3 << (u8GroupIndex*2)));
                    //mfcSleepMsNop(5);   //0x2540[0] sometimes not disabled
                }
            }
        }
        else if (u8Duty>=100)
        {
            if ( 0 == _bit0_(MDrv_MFC_ReadByte(0x2509+(u8GroupIndex*6)))) //Pos POL->100
            {
                //Shift=0,Period = 0
                MDrv_MFC_Write3BytesINT(0x2550 + (u8GroupIndex*4), 0);
                MDrv_MFC_Write2BytesINT(0x2504 + (u8GroupIndex*6), 0);
                reg_2540_PWM_Period_MSBExt = reg_2540_PWM_Period_MSBExt & (~(3 << (u8GroupIndex*2)));
                //mfcSleepMsNop(5);   //0x2540[0] sometimes not disabled
            }
            else
            {
                if (( 0 == MDrv_MFC_Read2BytesINT(0x2504 + (u8GroupIndex*6)))
                  &&( 0 == MDrv_MFC_Read2BytesINT(0x2550 + (u8GroupIndex*4))))//Neg0->100
                {
                    //Polarity swap
                    MDrv_MFC_WriteByteMaskINT(0x2509+(u8GroupIndex*6) , 0, _BIT0);
                }
                else                                                       //Neg POL->100
                {
                    //Period = 0
                    MDrv_MFC_Write2BytesINT(0x2504 + (u8GroupIndex*6), 0);
                    reg_2540_PWM_Period_MSBExt = reg_2540_PWM_Period_MSBExt & (~(3 << (u8GroupIndex*2)));
                    //mfcSleepMsNop(5);   //0x2540[0] sometimes not disabled
                }
            }
        }
        else
        {
        _setPWM(u8GroupIndex, 120, 1, (U32) u8Duty, (U32)u16Shift, 1, 0, bEnable);
    }
}
}

XDATA U8 updatePwmFreq= 0;
XDATA U8 prePwmFreq=0;
XDATA U8 prePwmDuty=0;
XDATA U8 prePwmSwap=0;
void appPWMHandler(void)
{
    U8 pwmDuty, pwmSwap;
//    U8 u8Reg2401;
    updatePwmFreq = MDrv_MFC_ReadByte(0x2C49)<<1;
    pwmDuty = MDrv_MFC_ReadByte(0x2C4E);
    pwmSwap = MDrv_MFC_ReadByte(0x2C45)&_BIT7;
    //MDrv_MFC_Write2Bytes(0x290C, pwmDuty|0xC000);    //Issue 161 Backlight Tuning Debug
    reg_2540_PWM_Period_MSBExt = MDrv_MFC_Read2Bytes(0x2540);
    reg_2542_PWM_Duty_MSBExt = MDrv_MFC_Read2Bytes(0x2542);

    if((updatePwmFreq!=prePwmFreq)
        || pwmDuty!=prePwmDuty
        || pwmSwap!=prePwmSwap)
    {
        //printf("\n\r %dHz", updatePwmFreq);
        //printf("duty=%d", prePwmDuty);
        //MDrv_MFC_Write2Bytes(0x290C, 0xC000|pwmDuty);
        #if 0
        if(pwmDuty>=100||pwmDuty==0)
        {
        u8Reg2401=MDrv_MFC_ReadByte(0x2401);
        MDrv_MFC_SetInterrupt (_DISABLE);
        }
        #endif
        if(updatePwmFreq==120)
        {
            //MDrv_MFC_SetPWMFreq(0, pwmDuty, 319, TRUE); //PWM0
            //MDrv_MFC_SetPWMFreq(1, pwmDuty, 143, TRUE); //PWM1

            MDrv_MFC_SetPWMFreq(((pwmSwap)?1:0), pwmDuty, 141, TRUE); //PWM0
            MDrv_MFC_SetPWMFreq(((pwmSwap)?0:1), pwmDuty, 326, TRUE); //PWM1
        }
        else
        {
            MDrv_MFC_SetPWMFreq(((pwmSwap)?1:0), pwmDuty, 120, TRUE); //PWM0
            MDrv_MFC_SetPWMFreq(((pwmSwap)?0:1), pwmDuty, 270, TRUE); //PWM1
        }
        MDrv_MFC_Write2BytesINT(0x2540, reg_2540_PWM_Period_MSBExt);
        MDrv_MFC_Write2BytesINT(0x2542, reg_2542_PWM_Duty_MSBExt); //set duty extend
        #if 0
        if(pwmDuty>=100||pwmDuty==0)
        {
        if((u8Reg2401&_BIT0)==0) MDrv_MFC_SetInterrupt (_ENABLE);
        }
        #endif
        prePwmFreq=updatePwmFreq;
        prePwmDuty=pwmDuty;
        prePwmSwap=pwmSwap;
    }
}
#endif

