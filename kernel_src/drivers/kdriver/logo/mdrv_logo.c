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

///////////////////////////////////////////////////////////////////////////////
/// @file   mdrv_ve.c
/// This file contains the Mstar driver for TVEncoder
/// @author MStar Semiconductor Inc.
/// @brief  TVEncoder module
///////////////////////////////////////////////////////////////////////////////

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_logo.h"
#include "mdrv_logo_scaler.h"
#include "mdrv_mfc_st.h"
#include "mdrv_mfc_platform.h"

//redcloud00@lge.com/////////////
#include <splash_auxinfo_jpg.h>
#include <linux/mtd/nand.h>
//#include <linux/mtd/mtd.h>
#include <linux/mtd/mtd_info.h>
#include <drv_extmicom.h>


///////////////////////////////



//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define TIME_TEST   0
#define PANEL_TYPE  1920
#define USE_MEASURE_TIME      0
#define USE_S7_Board          0
#define USE_S7M_FHD_60HZ      0
#define USE_S7M_WXGA_60HZ     0
#define USE_S7M_FHD_120HZ     1

#define GOP4G_BANK    0           // GOP 0
#define GOP2G_BANK    3           // GOP 1
#define GOP1G_BANK    7           // GOP 2
#define GOP1GS_BANK   10          // GOP 3

#define GOP_BUFFER  0x02600000

#define SYS_REGOP(addr) (*((volatile unsigned int*)(0xBF200000 + (addr))))
#define PM_REGOP(addr)  (*((volatile unsigned int*)(0xBF000000 + (addr))))

#define printf  printk


//redcloud00@lge.com logo//////////////////
void Splash_GetJpegInfo(SPLASH_BMP_TYPE_T bmpType, SPLASH_BMP_INFO_T *pSplashBmpInfo, void *argv );//yongs_logo
int Splash_Cp2Ram (ulong addr);
extern void MDrv_IIC_Init(void);
extern void MDrv_SW_IIC_SetSpeed(U8 u8ChIIC, U8 u8Speed);

/////////////////////////////////////


//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
//extern BOOL S7M;

MST_MFC_SYS_INFO_t LC420WUF=
{
	.u16HStart = 160,
	.u16VStart = 20,
	.u16Width  = 1920,
	.u16Height	= 1080,
	.u16HTotal = 2200,
    .u16VTotal = 1125,

	// set board info
	.u8LVDSChannel =_DUAL,
    .u8LVDSBitNum = _10BITS,//lachesis_090717//_8BITS, danie 5:48
	.u8LVDSTiMode = 1,
	.u8LVDSSwapMsbLsb = 0,
	.u8LVDSSwap_P_N = 0,
	.u8LVDSSwapOddEven = 1,

	// set pannel info
	.u8PanelVfreq = 120,
	.u8PanelChannel = _QUAD,
    .u8PanelLVDSSwapCH =CHANNEL_SWAP(_CH_B, _CH_D, _CH_A, _CH_C),  //0x72
	.u8PanelBitNum =_10BITS,
	// [6]TI mode, [5]=0:pair shift, [3]Polarity swap, [2]pair swap
	.u8PanelLVDSShiftPair = 0,
	.u8PanelLVDSTiMode = 1,
	.u8PanelLVDSSwapPol = 0,
	.u8PanelLVDSSwapPair = 0,

	.u16MFCMemoryClk		= 1000,//400
	.u16MFCMemoryType	= 0x0705,//0x0545,
	.u8PanelIncVtotalFor50Hz = 1,
	.u8PanelType		= 2,		//TTL, Mini_LVDS, LVDS
	.u8PanelDither		= 0,
	.u8PanelBlankCPVC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankOEC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankTPC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankSTHC	= 0,	//Panel Mini LVDS use
	.u8PanelCSC 		= 1,	//LVDS CSC enable/disable
	.u8PanelGAMMA		= 1,
	.u8UseMPIF			= 0,
};

MST_MFC_SYS_INFO_t AU20_WXGA=
{
    .u16HStart = 20,
    .u16VStart = 20,
    .u16Width  = 1366,
    .u16Height  = 768,
    .u16HTotal = 1604,
    .u16VTotal = 840,

    // set board info
    .u8LVDSChannel =_SINGLE,
    .u8LVDSBitNum = _8BITS,
    .u8LVDSTiMode = 1,
    .u8LVDSSwapMsbLsb = 0,
    .u8LVDSSwap_P_N = 0,
    .u8LVDSSwapOddEven = 1,

    // set pannel info
    .u8PanelVfreq = 60,
    .u8PanelChannel = _SINGLE,
    .u8PanelLVDSSwapCH =CHANNEL_SWAP(_CH_A, _CH_B, _CH_A, _CH_B),
    .u8PanelBitNum =_8BITS,
    // [6]TI mode, [5]=0:pair shift, [3]Polarity swap, [2]pair swap
    .u8PanelLVDSShiftPair = 0,
    .u8PanelLVDSTiMode = 1,
    .u8PanelLVDSSwapPol = 0,
    .u8PanelLVDSSwapPair = 0,

    .u16MFCMemoryClk    = 1000,//400
    .u16MFCMemoryType   = 0x0705,//0x0545,
    .u8PanelIncVtotalFor50Hz = 1,
    .u8PanelType        = _LVDS,        //TTL, Mini_LVDS, LVDS
    .u8PanelDither      = 0,
    .u8PanelBlankCPVC   = 0,    //Panel Mini LVDS use
    .u8PanelBlankOEC    = 0,    //Panel Mini LVDS use
    .u8PanelBlankTPC    = 0,    //Panel Mini LVDS use
    .u8PanelBlankSTHC   = 0,    //Panel Mini LVDS use
    .u8PanelCSC         = 1,    //LVDS CSC enable/disable
    .u8PanelGAMMA       = 0,
    .u8UseMPIF          = 0,
};

MST_MFC_SYS_INFO_t AU20_1680_1050=
{
	.u16HStart = 200,
	.u16VStart = 20,
	.u16Width  = 1680,
	.u16Height	= 1050,
	.u16HTotal = 2256,
	.u16VTotal = 1087,

	// set board info
	.u8LVDSChannel =_DUAL,
	.u8LVDSBitNum = _8BITS,
	.u8LVDSTiMode = 1,
	.u8LVDSSwapMsbLsb = 0,
	.u8LVDSSwap_P_N = 0,
	.u8LVDSSwapOddEven = 0,

	// set pannel info
	.u8PanelVfreq = 60,
	.u8PanelChannel = _DUAL,
	.u8PanelLVDSSwapCH =CHANNEL_SWAP(_CH_A, _CH_C, _CH_A, _CH_C),
	.u8PanelBitNum =_8BITS,
	// [6]TI mode, [5]=0:pair shift, [3]Polarity swap, [2]pair swap
	.u8PanelLVDSShiftPair = 0,
	.u8PanelLVDSTiMode = 1,
	.u8PanelLVDSSwapPol = 0,
	.u8PanelLVDSSwapPair = 0,

	.u16MFCMemoryClk		= 1000,//400
	.u16MFCMemoryType	= 0x0705,//0x0545,
	.u8PanelIncVtotalFor50Hz = 1,
	.u8PanelType		= 2,		//TTL, Mini_LVDS, LVDS
	.u8PanelDither		= 0,
	.u8PanelBlankCPVC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankOEC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankTPC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankSTHC	= 0,	//Panel Mini LVDS use
	.u8PanelCSC 		= 1,	//LVDS CSC enable/disable
	.u8PanelGAMMA		= 1,
	.u8UseMPIF			= 1,
};


MST_MFC_SYS_INFO_t AU21=
{
	.u16HStart = 40,
	.u16VStart = 20,
	.u16Width  = 1920,
	.u16Height	= 1200,
	.u16HTotal = 2080,
	.u16VTotal = 1235,

	// set board info
	.u8LVDSChannel =_DUAL,
	.u8LVDSBitNum = _8BITS,
	.u8LVDSTiMode = 1,
	.u8LVDSSwapMsbLsb = 0,
	.u8LVDSSwap_P_N = 0,
	.u8LVDSSwapOddEven = 0,

	// set pannel info
	.u8PanelVfreq = 60,
	.u8PanelChannel = _DUAL,
	.u8PanelLVDSSwapCH =CHANNEL_SWAP(_CH_A, _CH_C, _CH_A, _CH_C),
	.u8PanelBitNum =_8BITS,
	// [6]TI mode, [5]=0:pair shift, [3]Polarity swap, [2]pair swap
	.u8PanelLVDSShiftPair = 0,
	.u8PanelLVDSTiMode = 1,
	.u8PanelLVDSSwapPol = 0,
	.u8PanelLVDSSwapPair = 0,

	.u16MFCMemoryClk		= 1000,//400
	.u16MFCMemoryType	= 0x0705,//0x0545,
	.u8PanelIncVtotalFor50Hz = 1,
	.u8PanelType		= 2,		//TTL, Mini_LVDS, LVDS
	.u8PanelDither		= 0,
	.u8PanelBlankCPVC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankOEC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankTPC	= 0,	//Panel Mini LVDS use
	.u8PanelBlankSTHC	= 0,	//Panel Mini LVDS use
	.u8PanelCSC 		= 1,	//LVDS CSC enable/disable
	.u8PanelGAMMA		= 1,
	.u8UseMPIF			= 1,
};

//-------------------------------------------------------------------------------------------------
//  Export
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
unsigned char u8GOP = GOP1G_BANK;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/// Init MFC
/// @return None
//------------------------------------------------------------------------------
void MAdp_MFC_Init(PMST_MFC_SYS_INFO_t mfcSysInfo, U8 u8IsS7M)
{
    //Init MFC
    MDrv_MFC_SetSerialDebug(TRUE);
    S7M = u8IsS7M;
    if(S7M) //for S7M
    {
        MDrv_MFC_Init(mfcSysInfo);

        // Patch patch patch, eco in ursa u02
        //MDrv_MFC_SoftwareResetScaler();

        #if 0
        // debug
        mfcSleepMs(100);
        MDrv_MFC_WriteByteMask(0x290A, 0x04, 0x07);
        MDrv_MFC_WriteBit(0x2f25, 1, 0x80);
        MDrv_MFC_WriteBit(0x3017, 1, 0x80);  // 0:FPLL 1:FILM
        #endif
        #if 0
        // test
        mfcSleepMs(100);
        //MDrv_MFC_TrueMotionDemo(1);
        MDrv_MFC_DemoBarByRptWin(1, 0, 100, 0xf0);
        #endif
    }
	else//for BCM
	{
		//MAdp_MFC_SpiLoadCode();
		//MAdp_MFC_WaitReady();

	    // addr 0x2F2E
	    // 0x01: URSA will init with default data
	    // 0x07: URSA will set the panel info from Saturn6.
    	//#if 0
    	//	MDrv_MFC_WriteByte(0x2F2E, 0x01);
    	//#else
    	//	MAdp_MFC_PLATFORM_INFO_SET(mfcSysInfo);
    	//	MDrv_MFC_WriteByte(0x2F2E, 0x07);
    	//#endif
    	//for removing OSD garbage
	    //MDrv_MFC_WriteByteMask(0x2052, 0, 0x01);
	}
    MDrv_MFC_SetSerialDebug(FALSE);
}

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
#if 0
static void mstar_msleep( unsigned int ms )
{
    U32 ret_timer ;
    U32 limit ;

    // stop and capture timer value
    PM_REGOP( (0x3C88<<1) ) = 0x0300 ;

    // set max timer0 value
    PM_REGOP( (0x3C80<<1) ) = 0xFFFF ;
    PM_REGOP( (0x3C82<<1) ) = 0xFFFF ;
    // re-start timer
    PM_REGOP( (0x3C88<<1) ) = 0x0003 ;
    PM_REGOP( (0x3C88<<1) ) = 0x0300 ;
    limit = 12000*ms ;
    while(1){
        // capture timer value
        PM_REGOP( (0x3C88<<1) ) = 0x0301 ;
        PM_REGOP( (0x3C80<<1) ) = 0xFFFF ;
        PM_REGOP( (0x3C82<<1) ) = 0xFFFF ;
        PM_REGOP( (0x3C82<<1) ) = 0xFFFF ; // one more right for delay
        ret_timer = PM_REGOP( (0x3C86<<1) ) ;
        ret_timer <<= 16 ;
        ret_timer += PM_REGOP( (0x3C84<<1) ) ;
        if( ret_timer>=limit )
            return ;
        else
        {
            printf("ret_timer = %x, limit = %x\n", ret_timer, limit);
        }
    }
}
#else
static void mstar_msleep( unsigned int ms )
{
    U32 ret_timer = 0xFFFF;

    while (ret_timer)
    {
        ret_timer--;
    }
}

#endif

#if TIME_TEST
static U32 stimer( void )
{
    U32 ret_timer ;

    // stop and capture timer value
    PM_REGOP( (0x3C88<<1) ) = 0x0301 ;
    // set max timer0 value
    PM_REGOP( (0x3C80<<1) ) = 0xFFFF ;
    PM_REGOP( (0x3C82<<1) ) = 0xFFFF ;
    PM_REGOP( (0x3C82<<1) ) = 0xFFFF ; // one more right for delay
    ret_timer = PM_REGOP( (0x3C86<<1) ) ;
    ret_timer <<= 16 ;
    ret_timer += PM_REGOP( (0x3C84<<1) ) ;
    // re-start timer
    PM_REGOP( (0x3C88<<1) ) = 0x0003 ;
    PM_REGOP( (0x3C88<<1) ) = 0x0300 ;
    return ret_timer ;
}
#endif

#if USE_MEASURE_TIME
#define REG_PIU_TIMER0(_x_)              (0x003020 | (_x_ << 1))
static U32 stimer_sc( U8 u8status )
{
    U32 u32Timer =0;
    if(u8status == 0)
    {
        REG_WR(REG_PIU_TIMER0(0x00), 0x0);
        REG_WR(REG_PIU_TIMER0(0x00), 0x1);
    }
    if(u8status)
    {
        u32Timer = ((U32)REG_RR(REG_PIU_TIMER0(0x04))) |
                   ((U32)REG_RR(REG_PIU_TIMER0(0x05)) << 16); // h0005:h0004
    }
    return u32Timer ;
}
#endif


//
// Decode JPEG
//-----------------------------------------------------------------

#define JPEG_MAXNUMBER       16
#define JPEG_MAXBLOCKSPERMCU 10
#define JPEG_MAXHUFFTABLES   8
#define JPEG_MAXQUANTTABLES  4
#define JPEG_MAXCOMPONENTS   4
#define JPEG_MAXCOMPSINSCAN  4

//#define JPEG_PRINT(fmt, args...)         printk("[JPEG][%06d]     " fmt, __LINE__, ## args)
#define JPEG_PRINT(fmt, args...)


#define MAX_JPEG_OUTPUT_WIDTH   1920
#define MAX_JPEG_OUTPUT_HEIGHT  1080

#define MemAlign(n, unit)           ((((n)+(unit)-1)/(unit))*(unit))
#define OUTPUT_BUFF_SIZE MemAlign(MAX_JPEG_OUTPUT_WIDTH*MAX_JPEG_OUTPUT_HEIGHT*2,0x100000)

#define JPEG_TYPE_MAIN          0
#define JPEG_TYPE_THUMBNAIL     1
#define JPEG_TYPE_MJPEG         2

typedef enum
{
    EN_JPD_DECODING,
    EN_JPD_DECODE_DONE,
    EN_JPD_DECODE_ERROR,
    EN_JPD_MRBFH_DONE,
    EN_JPD_MRBFL_DONE
} EN_JPD_STATUS;

typedef struct
{
    U32 u32OriginalWidth;
    U32 u32OriginalHeight;
    U32 u32Width;
    U32 u32Height;
    U8  u8BitPP;
    U8  u8Progressive;
    EN_JPD_STATUS enJPDStatus;
} JPEG_PIC_Info;

typedef struct
{
	U8  u8GwinID;
	U32 u32X;
	U32 u32Y;
	U32 u32Width;
	U32 u32Height;
} GWIN_INFO;


typedef struct
{
	U8 u8GwinID;
	U8 u8R_bg;
	U8 u8G_bg;
	U8 u8B_bg;
} GWIN_BG_INFO;

static U32 jpegWidth[JPEG_MAXNUMBER];
static U32 jpegHeight[JPEG_MAXNUMBER];

#define JPEG_DEFAULT_EXIF_SIZE      (64*1024UL + 128)

#define MRC_BUFFER_ADDR             _u32ReadBufferAddr
#define MRC_BUFFER_SIZE             _u32ReadBufferSize
#define MWC_BUFFER_ADDR             _u32WriteBufferAddr
#define INTERNAL_BUFFER_ADDR        _u32InternalBufferAddr
#define INTERNAL_BUFFER_SIZE        _u32InternalBufferSize

static U32 _u32ReadBufferSize;
static U32 _u32ReadBufferAddr;
static U32 _u32WriteBufferAddr;
static U32 _u32InternalBufferAddr;
static U32 _u32InternalBufferSize;
static U32 _u32RLEOffset;   // offset to record the current RLE access address


static U8 *_pu8In_buf;
static U8 _u8FileReadEnd_flag = 0 ;

static GWIN_INFO    _Gwin_info[4];

#define MIN_READBUFFER_SIZE 128



struct __mem__
{
    struct __mem__  *next;    /* single-linked list */
    U32	  len;      /* length of following block */
};

typedef struct __mem__	 __memt__;
typedef __memt__  *__memp__;

#define	HLEN	(sizeof(__memt__))
#define MIN_BLOCK	(HLEN * 4)

static __memt__  __mem_avail__ [2] =
{
    { NULL, 0 },	/* HEAD for the available block list */
    { NULL, 0 },	/* UNUSED but necessary so free doesn't join HEAD or ROVER with the pool */
};

#define AVAIL	(__mem_avail__[0])

#define MIN_POOL_SIZE	(HLEN * 10)

int G_MDrv_JPEG_init_mempool (void *pool, U32 size)
{
    if (size < MIN_POOL_SIZE)
        return 0; 					/* FAILURE */

    if (pool == NULL)
    {
        pool = (void *)1;
        size--;
    }

    //memset(pool, 0, size);

    AVAIL.next = (struct __mem__  *)pool;
    AVAIL.len  = size;

    (AVAIL.next)->next = NULL;
    (AVAIL.next)->len  = size - HLEN;

    return 1;   					/* SUCCESS */
}


int G_MDrv_JPEG_SetInitParameter(U32 u32RBufAddr, U32 u32RBufSize, U32 u32WBufAddr, U32 u32IBufAddr, U32 u32IBufSize, U8 u8FileEnd)
{
    // the buffer size must be multiple of 4 bytes
    if( u32RBufSize < MIN_READBUFFER_SIZE )
    {
        JPEG_PRINT("ERROR\n");
        return 0 ;
    }
    // the buffer size must be multiple of 8 bytes
    u32RBufSize = u32RBufSize & ~0x7;

    MRC_BUFFER_ADDR = u32RBufAddr;
    MWC_BUFFER_ADDR = u32WBufAddr;
    INTERNAL_BUFFER_ADDR = u32IBufAddr;
    MRC_BUFFER_SIZE = u32RBufSize;
    INTERNAL_BUFFER_SIZE = u32IBufSize;

    _pu8In_buf = (U8 *) u32RBufAddr;
    _u8FileReadEnd_flag = u8FileEnd;

    JPEG_PRINT("MRC= 0x%X\n", MRC_BUFFER_ADDR);
    JPEG_PRINT("MWC= 0x%X\n", MWC_BUFFER_ADDR);
    JPEG_PRINT("INT= 0x%X\n", INTERNAL_BUFFER_ADDR);

    G_MDrv_JPEG_init_mempool(((U8 *) (u32IBufAddr + JPEG_DEFAULT_EXIF_SIZE)), u32IBufSize - JPEG_DEFAULT_EXIF_SIZE);

    return 1 ;
}

static U8 _u8DecodeType = JPEG_TYPE_MAIN;
// The width/height may be the thumbnail or original image size, it based on decoding mode
static U16 _u16Image_x_size;
static U16 _u16Image_y_size;
// The original size of this JPEG file
static U16 _u16OriginalImage_x_size;
static U16 _u16OriginalImage_y_size;

static U32 _bProgressive_flag;
static U8 _u8DownScaleRatio;
static U32 _bFirstRLE;

// The Luma and Chroma (YU) component ID, default is 1 & 2
static U8 _u8LumaCi = 1;
static U8 _u8ChromaCi = 2;

static S16 _s16Error_code;
static BOOL _bReady_flag;

/************* Downscale Ratio *************/
/*** Bellows are 1, 1/2, 1/4 and 1/8, in order ***/
#define G_JPD_DOWNSCALE_ORG       0x00
#define G_JPD_DOWNSCALE_HALF      0x01
#define G_JPD_DOWNSCALE_FOURTH    0x02
#define G_JPD_DOWNSCALE_EIGHTH    0x03
/********************I*********************/

#define QUANT_TYPE S16
#define BLOCK_TYPE S16

static BLOCK_TYPE _s16dc_pred[3];


typedef struct
{
    U32 bValid;
    U8 u8Huff_num[17]; /* number of Huffman codes per bit size */
    U8 u8Huff_val[256]; /* Huffman codes per bit size */

    U8 u8Symbol[17]; /* u8Huff_num in reverse order.. */
    U16 u16Code[17]; /* Minimun code word */
} HUFF_INFO;
static HUFF_INFO _Huff_info[JPEG_MAXHUFFTABLES];


typedef struct tagQUANT_TABLES
{
    U32 bValid;
    QUANT_TYPE s16Value[64];
} QUANT_TABLES, *PQUANT_TABLES;
static QUANT_TABLES _QuantTables[JPEG_MAXQUANTTABLES]; /* pointer to quantization tables */

static U8 gu8Scan_type;                      /* Grey, Yh1v1, Yh1v2, Yh2v1, Yh2v2,
                                       CMYK111, CMYK4114 */
static U8 _u8Comps_in_frame;                      /* # of components in frame */

static U8 _u8Comp_h_samp[JPEG_MAXCOMPONENTS];     /* component's horizontal sampling factor */
static U8 _u8Comp_v_samp[JPEG_MAXCOMPONENTS];     /* component's vertical sampling factor */
static U8 _u8Comp_quant[JPEG_MAXCOMPONENTS];      /* component's quantization table selector */
static U8 _u8Comp_ident[JPEG_MAXCOMPONENTS];      /* component's ID */

static U16 _u16Comp_h_blocks[JPEG_MAXCOMPONENTS];
static U16 _u16Comp_v_blocks[JPEG_MAXCOMPONENTS];

static U8 _u8Comps_in_scan;                  /* # of components in scan */
static U8 _u8Comp_list[JPEG_MAXCOMPSINSCAN];      /* components in this scan */
static U8 _u8Comp_dc_tab[JPEG_MAXCOMPONENTS];     /* component's DC Huffman coding table selector */
static U8 _u8Comp_ac_tab[JPEG_MAXCOMPONENTS];     /* component's AC Huffman coding table selector */

static U8 _u8Spectral_start;                 /* spectral selection start */
static U8 _u8Spectral_end;                   /* spectral selection end   */
static U8 _u8Successive_low;                 /* successive approximation low */
static U8 _u8Successive_high;                /* successive approximation high */

static U8 gu8Max_mcu_x_size;                 /* MCU's max. X size in pixels */
static U8 gu8Max_mcu_y_size;                 /* MCU's max. Y size in pixels */

static U8 _u8Blocks_per_mcu;
static U32 _u32Max_blocks_per_row;
static U16 _u16Mcus_per_row, _u16Mcus_per_col;

static U8 _u8Mcu_org[JPEG_MAXBLOCKSPERMCU];

#define JPEG_MAXBLOCKS    50
static void *_pBlocks[JPEG_MAXBLOCKS];         /* list of all dynamically allocated blocks */

#define SUPPORT_PROGRESSIVE_MODE    1

#if SUPPORT_PROGRESSIVE_MODE

typedef struct tagHUFF_TABLES
{
    S16 s16Look_up[256];
    U8 u8Code_size[256];
    // FIXME: Is 512 tree entries really enough to handle _all_ possible
    // code sets? I think so but not 100% positive.
    S16 s16Tree[512];
} HUFF_TABLES, *PHUFF_TABLES;

typedef struct tagCOEFF_BUF
{
    U8 *pu8Data;

    U16 u16Block_num_x, u16Block_num_y;
    U8 u8Block_len_x, u8Block_len_y;

    U16 u16Block_size;
} COEFF_BUF, *PCOEFF_BUF;

static U16 _u16Total_lines_left;               /* total # lines left in image */
static U16 gu16Mcu_lines_left;                 /* total # lines left in this MCU */
static U32 _u32Block_y_mcu[JPEG_MAXCOMPONENTS];
static HUFF_TABLES _Huff_tbls[JPEG_MAXHUFFTABLES];
static PCOEFF_BUF _DC_Coeffs[JPEG_MAXCOMPONENTS];
static PCOEFF_BUF _AC_Coeffs[JPEG_MAXCOMPONENTS];
static U32 _u32Last_dc_val[JPEG_MAXCOMPONENTS];

static U32 _u32EOB_run;
#endif

static U8 *_pu8In_buf_ofs;
static U32 _u32In_buf_left;
static U8 _u8Tem_flag;
static U32 _bEOF_flag;

static U16 _u16Restart_interval;
static U16 _u16Restarts_left;
static U16 _u16Next_restart_num;

static U16 gu16Max_mcus_per_row;
static U16 _u16Max_blocks_per_mcu;
static U16 _u16Max_mcus_per_col;

//static jmp_buf _jmp_state;
static S32  _Total_Decoded_Size = 0;
static U32 _u32Total_bytes_read;

static U32 _bThumbnailAccessMode;

static S16 _s16Bits_left;

static U8 *_pu32ExifHeaderAddr;

static void G_MDrv_JPEG_init( void )
{
    U16 i;

    _u8LumaCi = 1;
    _u8ChromaCi = 2;

    _s16Error_code = 0;

    _bReady_flag = 0 ;

    _u16Image_x_size = _u16Image_y_size = 0;
    _u16OriginalImage_x_size = _u16OriginalImage_y_size = 0;

    //_pStream = _Pstream;

    _bProgressive_flag = 0;
    _u8DownScaleRatio = G_JPD_DOWNSCALE_ORG;
    _u32RLEOffset = 0;
    _bFirstRLE = 0;

    _s16dc_pred[0] = _s16dc_pred[1] = _s16dc_pred[2] = 0;



    for(i = 0; i<JPEG_MAXHUFFTABLES; i++)
    {
        _Huff_info[i].bValid = 0;
        memset(_Huff_info[i].u8Huff_num, 0, 17);
        memset(_Huff_info[i].u8Huff_val, 0, 256);
        memset(_Huff_info[i].u8Symbol, 0, 17);
        memset(_Huff_info[i].u16Code, 0, 17);
    }

    for(i = 0; i<JPEG_MAXQUANTTABLES; i++)
    {
        _QuantTables[i].bValid = 0;
        memset(_QuantTables[i].s16Value, 0, 64);
    }

    gu8Scan_type = 0;

    _u8Comps_in_frame = 0;

    memset( _u8Comp_h_samp, 0, sizeof( _u8Comp_h_samp ) );
    memset( _u8Comp_v_samp, 0, sizeof( _u8Comp_v_samp ) );
    memset( _u8Comp_quant, 0, sizeof( _u8Comp_quant ) );
    memset( _u8Comp_ident, 0, sizeof( _u8Comp_ident ) );
    memset( _u16Comp_h_blocks, 0, sizeof( _u16Comp_h_blocks ) );
    memset( _u16Comp_v_blocks, 0, sizeof( _u16Comp_v_blocks ) );

    _u8Comps_in_scan = 0;
    memset( _u8Comp_list, 0, sizeof( _u8Comp_list ) );
    memset( _u8Comp_dc_tab, 0, sizeof( _u8Comp_dc_tab ) );
    memset( _u8Comp_ac_tab, 0, sizeof( _u8Comp_ac_tab ) );

    _u8Spectral_start = 0;
    _u8Spectral_end = 0;
    _u8Successive_low = 0;
    _u8Successive_high = 0;

    gu8Max_mcu_x_size = 0;
    gu8Max_mcu_y_size = 0;

    _u8Blocks_per_mcu = 0;
    _u32Max_blocks_per_row = 0;
    _u16Mcus_per_row = 0;
    _u16Mcus_per_col = 0;

    memset( _u8Mcu_org, 0, sizeof( _u8Mcu_org ) );

    memset( _pBlocks, 0, sizeof( _pBlocks ) );

    #if SUPPORT_PROGRESSIVE_MODE
    _u16Total_lines_left = 0;
    gu16Mcu_lines_left = 0;
    memset( _u32Block_y_mcu, 0, sizeof( _u32Block_y_mcu ) );
    memset( _Huff_tbls, 0, sizeof( _Huff_tbls ) );
    memset( _DC_Coeffs, 0, sizeof( _DC_Coeffs ) );
    memset( _AC_Coeffs, 0, sizeof( _AC_Coeffs ) );
    memset( _u32Last_dc_val, 0, sizeof( _u32Last_dc_val ) );

    _u32EOB_run = 0;
    #endif

    _pu8In_buf_ofs = _pu8In_buf;
    _u32In_buf_left = 0;
    _bEOF_flag = 0;
    _u8Tem_flag = 0;

    //sharon memset( _pu8In_buf, 0, sizeof(U8)*(MRC_BUFFER_SIZE + 128) );

    _u16Restart_interval = 0;
    _u16Restarts_left = 0;
    _u16Next_restart_num = 0;

    gu16Max_mcus_per_row = 0;
    _u16Max_blocks_per_mcu = 0;
    _u16Max_mcus_per_col = 0;

    _u32Total_bytes_read = 0;

    // Tell the stream we're going to use it.
    //_pStream->attach();

    // Ready the input buffer.
    if(_bThumbnailAccessMode)
    {
        //MDrv_JPEG_fill_read_buffer();
    }
    else
    {
        _u32In_buf_left = MRC_BUFFER_SIZE;  //sharon
        _Total_Decoded_Size = MRC_BUFFER_SIZE;  //sharon
        _u32Total_bytes_read = MRC_BUFFER_SIZE;  //sharon
    }

    // Prime the bit buffer.
    _s16Bits_left = 16;

    _pu32ExifHeaderAddr = 0;

}


typedef enum
{
    M_SOF0 = 0xC0,
    M_SOF1 = 0xC1,
    M_SOF2 = 0xC2,
    M_SOF3 = 0xC3,
    M_SOF5 = 0xC5,
    M_SOF6 = 0xC6,
    M_SOF7 = 0xC7,
    M_JPG = 0xC8,
    M_SOF9 = 0xC9,
    M_SOF10 = 0xCA,
    M_SOF11 = 0xCB,
    M_SOF13 = 0xCD,
    M_SOF14 = 0xCE,
    M_SOF15 = 0xCF,
    M_DHT = 0xC4,
    M_DAC = 0xCC,
    M_RST0 = 0xD0,
    M_RST1 = 0xD1,
    M_RST2 = 0xD2,
    M_RST3 = 0xD3,
    M_RST4 = 0xD4,
    M_RST5 = 0xD5,
    M_RST6 = 0xD6,
    M_RST7 = 0xD7,
    M_SOI = 0xD8,
    M_EOI = 0xD9,
    M_SOS = 0xDA,
    M_DQT = 0xDB,
    M_DNL = 0xDC,
    M_DRI = 0xDD,
    M_DHP = 0xDE,
    M_EXP = 0xDF,
    M_APP0 = 0xE0,
    M_APP1 = 0xE1,
    M_APP15 = 0xEF,
    M_JPG0 = 0xF0,
    M_JPG13 = 0xFD,
    M_COM = 0xFE,
    M_TEM = 0x01,
    M_ERROR = 0x100
} EN_JPEG_MARKER;

static U32 _u32Bit_buf;

static U8 MDrv_JPEG_get_char( void )
{
    U8 c;

    // Any bytes remaining in buffer?
    if ( !_u32In_buf_left )
    {
        // Try to get more bytes.
        //MDrv_JPEG_fill_read_buffer();
        // Still nothing to get?
        if ( !_u32In_buf_left )
        {
            // Padd the end of the stream with 0xFF 0xD9 (EOI marker)
            // FIXME: Is there a better padding pattern to use?
            U8 t = _u8Tem_flag;
            _u8Tem_flag ^= 1;
            if ( t )
            {
                return ( 0xD9 );
            }
            else
            {
                return ( 0xFF );
            }
        }
    }

    c = *_pu8In_buf_ofs++;
    _u32In_buf_left--;

    return ( c );
}

static U32 G_MDrv_JPEG_locate_soi_marker( void )
{
    U32 lastchar, thischar;
    U32 bytesleft;

    lastchar = MDrv_JPEG_get_char();

    thischar = MDrv_JPEG_get_char();

    /* ok if it's a normal JPEG file without a special header */
    //JPEG_PRINT("last=%X  this=%X\n", lastchar, thischar);

    if ( ( lastchar == 0xFF ) && ( thischar == M_SOI ) )
    {
        JPEG_PRINT("SOI\n");
        return 1;
    }

    bytesleft = 640; //Fix this number from 512 -> 640 for some cases

    for ( ; ; )
    {
        if ( --bytesleft == 0 )
        {
            return 0;
        }

        lastchar = thischar;

        thischar = MDrv_JPEG_get_char();

        if ( ( lastchar == 0xFF ) && ( thischar == M_SOI ) )
        {
            JPEG_PRINT("SOI\n");
            break;
        }
    }

    /* Check the next character after marker: if it's not 0xFF, it can't
       be the start of the next marker, so it probably isn't a JPEG */

    thischar = ( _u32Bit_buf >> 8 ) & 0xFF;

    if ( thischar != 0xFF )
    {
        return 0;
    }
    return 1;
}

static U32 _bThumbnailFound;

static U32 MDrv_JPEG_next_marker( void ) //ok
{
    U32 c, bytes;

    bytes = 0;

    do
    {
        do
        {
            bytes++;

            c = MDrv_JPEG_get_char();

        }
        while ( c != 0xFF );

        do
        {
            c = MDrv_JPEG_get_char();
        }
        while ( c == 0xFF );
    }
    while ( c == 0 );

    // If bytes > 0 here, there where extra bytes before the marker (not good).

    return c;
}


static U32 _u32ThumbnailBufferOffset;   // keep track of thumb buffer access address
static U32 _u16ThumbnailBufferSize;   // keep track of thumb buffer size
static U32 _u32ThumbnailOffset;
static U16 _u16ThumbnailSize;

static U32 MDrv_JPEG_GetU32(U8 *data)
{
    S8 i;
    U32 val = 0;

    for(i = 3; i>=0; i--)
        val = (val << 8) + (U8) *(data + i);

    return val;
}

//------------------------------------------------------------------------------
/* EXIF parsing section */
#define EndianChangeL(_x)   \
    (((_x & 0xff) << 24) | ((_x & 0xff00) << 8) | ((_x & 0xff0000) >> 8) | ((_x & 0xff000000) >> 24))

#define EndianChangeS(_x)   \
    (((_x & 0xff) << 8) | ((_x & 0xff00) >> 8))

#define JPEG_TAG_EXIF               EndianChangeL(0x45786966)
#define JPEG_TIFF_SOI_OFFSET        0x0201
#define JPEG_TIFF_JPEG_IMG_BYTES    0x0202

#define JPEG_TIFF_BIG_ENDIAN        0x4D4D
#define JPEG_TIFF_LITTLE_ENDIAN     0x4949

static U16 MDrv_JPEG_GetU16(U8 *data)
{
    S8 i;
    U16 val = 0;

    for(i = 1; i>=0; i--)
        val = (val << 8) + (U8) *(data + i);


    return val;
}

static U32 _bTiffBigEndian;

static U16 MDrv_JPEG_Tiff_EndianChangeS(U16 u16Val)
{
    if (_bTiffBigEndian)
        return EndianChangeS(u16Val);
    else
        return u16Val;
}


static U32 MDrv_JPEG_Tiff_EndianChangeL(U32 u32Val)
{
    if (_bTiffBigEndian)
        return EndianChangeL(u32Val);
    else
        return u32Val;
}

static U32 MDrv_JPEG_DecodeExifInfo(U8 *data, U32 data_length)
{
    U8 *pJpegBuff = data;
    U32  u32tmp, u32Len;
    U16  u16Marker, u16Len, u16NumOfEntry, i;
    U8   *pTiffHdrPtr, *pNextIfd;

    u32tmp = MDrv_JPEG_GetU32(pJpegBuff);
    if (u32tmp != JPEG_TAG_EXIF)
    {
        return 0;
    }

    // Exif starts here
    pJpegBuff += 6;
    u16Marker = MDrv_JPEG_GetU16(pJpegBuff);
    if (u16Marker == JPEG_TIFF_BIG_ENDIAN)
        _bTiffBigEndian = 1;
    else if (u16Marker == JPEG_TIFF_LITTLE_ENDIAN)
        _bTiffBigEndian = 0;
    else
        return 0;

    _u32ThumbnailOffset += 6;
    pTiffHdrPtr = pJpegBuff;

    pJpegBuff += 2;
    u16Marker = MDrv_JPEG_Tiff_EndianChangeS(MDrv_JPEG_GetU16(pJpegBuff));
    if (u16Marker != 0x002A)
        return 0;

    pJpegBuff += 2;
    u16Len = (U16)MDrv_JPEG_Tiff_EndianChangeL(MDrv_JPEG_GetU32(pJpegBuff));
    pJpegBuff += (u16Len - 4);

    // 0th IFD start here
    u16NumOfEntry = MDrv_JPEG_Tiff_EndianChangeS(MDrv_JPEG_GetU16(pJpegBuff));
    pJpegBuff += 2;

    // Boundary check, prevent from buffer over-run
    if((((U32) pJpegBuff) - INTERNAL_BUFFER_ADDR + u16NumOfEntry*12)>=data_length)
    {
        return 0;
    }

    pJpegBuff += 12*u16NumOfEntry;

    // 1th IFD
    u32tmp = MDrv_JPEG_Tiff_EndianChangeL(MDrv_JPEG_GetU32(pJpegBuff));
    if (u32tmp == 0)
        return 0;
    else
        pNextIfd = &pTiffHdrPtr[u32tmp];

    u16NumOfEntry = MDrv_JPEG_Tiff_EndianChangeS(MDrv_JPEG_GetU16(pNextIfd));
    pNextIfd += 2;

    // Boundary check, prevent from buffer over-run
    if((((U32) pNextIfd) - INTERNAL_BUFFER_ADDR + u16NumOfEntry*12)>=data_length)
    {
        return 0;
    }

    for (i = 0; i < u16NumOfEntry; i++)
    {
        u16Marker = MDrv_JPEG_Tiff_EndianChangeS(MDrv_JPEG_GetU16(pNextIfd));
        u32Len = MDrv_JPEG_Tiff_EndianChangeL(MDrv_JPEG_GetU32(pNextIfd + 4));
        u32tmp = MDrv_JPEG_Tiff_EndianChangeL(MDrv_JPEG_GetU32(pNextIfd + 8));

        switch (u16Marker)
        {
            case JPEG_TIFF_JPEG_IMG_BYTES:
                _u16ThumbnailSize = u32tmp;
                break;
            case JPEG_TIFF_SOI_OFFSET:
                _bThumbnailFound= 1;
                _u32ThumbnailOffset += u32tmp;
                break;
        }

        pNextIfd += 12;
    }

    // Boundary check, prevent from buffer over-run
    if(_bThumbnailFound)
    {
        if((_u32ThumbnailOffset + _u16ThumbnailSize) > data_length)
            _bThumbnailFound = 0;

        // means it only contains SOI header..
        if(_u16ThumbnailSize<=2)
            _bThumbnailFound = 0;
    }

    return _bThumbnailFound;
}

static U32 MDrv_JPEG_read_app1_marker( void )
{
    U16 length;
    U8 *exif_buffer = (U8 *) INTERNAL_BUFFER_ADDR;
    U16 i = 0;

    JPEG_PRINT("APP1\n");

    length = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( length < 2 )
    {
        return 0;
    }

    length -= 2;

    while((length - i)!=0)
    {
        exif_buffer[i] = (U8) MDrv_JPEG_get_char();
        i++;
    }

    if(MDrv_JPEG_DecodeExifInfo(exif_buffer, length)==1)
    {
        JPEG_PRINT("FOUND THUMBNAIL!\n");
        _u32ThumbnailBufferOffset = _u32ThumbnailOffset;
        _u16ThumbnailBufferSize = _u16ThumbnailSize;
    }
    else
    {
        JPEG_PRINT("NO THUMBNAIL!\n");
    }
    return 1;
}


static void MDrv_JPEG_skip_bytes(U32 count)
{
    while(count!=0)
    {
        // Any bytes remaining in buffer?
        if ( !_u32In_buf_left )
        {
            // Try to get more bytes.
            //MDrv_JPEG_fill_read_buffer();
            // Still nothing to get?
            if ( !_u32In_buf_left )
            {
                // should not happen
                break;
            }
        }

        if(count<_u32In_buf_left)
        {
            _u32In_buf_left -= count;
            _pu8In_buf_ofs += count;
            count = 0;
        }
        else
        {
            count -= _u32In_buf_left;
            _u32In_buf_left = 0;
        }
    }
}


static U32 MDrv_JPEG_skip_variable_marker( void )
{
    U32 left;

    JPEG_PRINT("SKIP markers\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( left < 2 )
    {
        return 0 ;
    }

    left -= 2;

    MDrv_JPEG_skip_bytes(left);
    return 1 ;
}

static U32 MDrv_JPEG_read_dht_marker( void )
{
    U16 i, index, count;
    U32 left;
    U8 u8Huff_num[17];
    U8 u8Huff_val[256];

    JPEG_PRINT("DHT\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( left < 2 )
    {
        return 0 ;
    }

    left -= 2;

    while ( left )
    {
        // set it to zero, initialize
        memset(u8Huff_num, 0, 17);
        memset(u8Huff_val, 0, 256);

        index = MDrv_JPEG_get_char();

        u8Huff_num[0] = 0;

        count = 0;

        for ( i = 1; i <= 16; i++ )
        {
            u8Huff_num[i] = MDrv_JPEG_get_char();
            count += u8Huff_num[i];
        }

        if ( count > 255 )
        {
            //MDrv_JPEG_terminate( JPEG_BAD_DHT_COUNTS );
            return 0;
        }

        for ( i = 0; i < count; i++ )
        {
            u8Huff_val[i] = MDrv_JPEG_get_char();
        }

        i = 1 + 16 + count;

        if ( left < ( U32 )i )
        {
            //MDrv_JPEG_terminate( JPEG_BAD_DHT_MARKER );
            return 0;
        }

        left -= i;

        if ( ( index & 0x10 ) > 0x10 ) //no need ???
        {
            //MDrv_JPEG_terminate( JPEG_BAD_DHT_INDEX );
            return 0;
        }

        index = ( index & 0x0F ) + ( ( index & 0x10 ) >> 4 ) * ( JPEG_MAXHUFFTABLES >> 1 ); //???

        if ( index >= JPEG_MAXHUFFTABLES )
        {
            //MDrv_JPEG_terminate( JPEG_BAD_DHT_INDEX );
            return 0;
        }

        if(_Huff_info[index].bValid==0)
            _Huff_info[index].bValid = 1;

        memcpy( _Huff_info[index].u8Huff_num, u8Huff_num, 17 );
        memcpy( _Huff_info[index].u8Huff_val, u8Huff_val, 256 );

        // Compute the inverse order of HuffNum. this step is only needed in JPD mode (baseline)
        for(i = 1; i<=16; i++)
        {
            if(u8Huff_num[17 - i]!=0)
            {
                count = count - u8Huff_num[17 - i];
                u8Huff_num[17 - i] = count;
            }
            else
                u8Huff_num[17 - i] = 0xFF;
        }
        memcpy( _Huff_info[index].u8Symbol, u8Huff_num, 17 );

    }
    return 1;
}


static U32 MDrv_JPEG_read_dqt_marker( void )
{
    U16 n, i, prec;
    U32 left;
    U32 temp;

    //JPEG_PRINT("DQT\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( left < 2 )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_DQT_MARKER );
        return 0;
    }

    left -= 2;

    while ( left )
    {
        n = MDrv_JPEG_get_char();
        prec = n >> 4;
        n &= 0x0F;

        if ( n >= JPEG_MAXQUANTTABLES )
        {
            //MDrv_JPEG_terminate( JPEG_BAD_DQT_TABLE );
            return 0;
        }

        if(_QuantTables[n].bValid==0)
            _QuantTables[n].bValid = 1;

        // read quantization entries, in zag order
        for ( i = 0; i < 64; i++ )
        {
            temp = MDrv_JPEG_get_char();

            if ( prec )
            {
                temp = ( temp << 8 ) + MDrv_JPEG_get_char();
            }

            _QuantTables[n].s16Value[i] = temp;

            //JPEG_PRINT("_QuantTables[%d].s16Value[%d]= %X\n", n, i, _QuantTables[n].s16Value[i]);
        }

        i = 64 + 1;

        if ( prec )
        {
            i += 64;
        }

        if ( left < ( U32 )i )
        {
            //MDrv_JPEG_terminate( JPEG_BAD_DQT_LENGTH );
            return 0;
        }

        left -= i;
    }
    return 1;
}


static U32 MDrv_JPEG_read_dri_marker( void )
{
    JPEG_PRINT("DRI\n");

    if ( (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char()) != 4 )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_DRI_LENGTH );
        return 0;
    }

    _u16Restart_interval = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());    return 1;
}

static U32 MDrv_JPEG_process_markers( void )
{
    U32 c;

    for ( ; ; )
    {
        c = MDrv_JPEG_next_marker();

        switch ( c )
        {
            case M_APP1:
                // Prevent from there's thumbnail in thumbnail... & multiple APP1
                // Although it's impossible.. =_=
                if( (_u8DecodeType==JPEG_TYPE_THUMBNAIL) && (_bThumbnailFound==0) )
                {
                    if( !MDrv_JPEG_read_app1_marker() )
                        return 0;
                }
                else
                {
                    MDrv_JPEG_skip_variable_marker();
                   //MDrv_JPEG_read_app1_marker();
                }
                break;
            case M_SOF0:
            case M_SOF1:
            case M_SOF2:
            case M_SOF3:
            case M_SOF5:
            case M_SOF6:
            case M_SOF7:
                //      case M_JPG:
            case M_SOF9:
            case M_SOF10:
            case M_SOF11:
            case M_SOF13:
            case M_SOF14:
            case M_SOF15:
            case M_SOI:
            case M_EOI:
            case M_SOS:
                {
                    return c;
                }
            case M_DHT:
                {
                    if( !MDrv_JPEG_read_dht_marker() )
                        return 0;
                    break;
                }
                // Sorry, no arithmitic support at this time. Dumb patents!
            case M_DAC:
                {
                        return 0;
                    break;
                }
            case M_DQT:
                {
                    if( !MDrv_JPEG_read_dqt_marker() )
                        return 0;
                    break;
                }
            case M_DRI:
                {
                    if( !MDrv_JPEG_read_dri_marker() )
                        return 0;
                    break;
                }
                //case M_APP0:  /* no need to read the JFIF marker */

            case M_JPG:
            case M_RST0:
                /* no parameters */
            case M_RST1:
            case M_RST2:
            case M_RST3:
            case M_RST4:
            case M_RST5:
            case M_RST6:
            case M_RST7:
            case M_TEM:
                {
                    return 0;
                }
            default:
                /* must be DNL, DHP, EXP, APPn, JPGn, COM, or RESn or APP0 */
                {
                    MDrv_JPEG_skip_variable_marker();
                    break;
                }
        }
    }
    return 1;
}

static U8 _u8Precision;

//------------------------------------------------------------------------------
#define JPEG_MAX_HEIGHT 1080*8
#define JPEG_MAX_WIDTH  1920*8

static U32 MDrv_JPEG_read_sof_marker( void )
{
    U8 i,tmp;
    U32 left;

    JPEG_PRINT("SOF\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());
    _u8Precision = MDrv_JPEG_get_char();

    if ( _u8Precision != 8 )   /* precision: sorry, only 8-bit precision is supported right now */
    {
        //MDrv_JPEG_terminate( JPEG_BAD_PRECISION );
        return 0;
    }

    _u16Image_y_size = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( ( _u16Image_y_size < 1 ) || ( _u16Image_y_size > JPEG_MAX_HEIGHT ) )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_HEIGHT );
        return 0;
    }

    _u16Image_x_size = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());


    if ( ( _u16Image_x_size < 1 ) || ( _u16Image_x_size > JPEG_MAX_WIDTH ) )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_WIDTH );
        return 0;
    }

    _u8Comps_in_frame = MDrv_JPEG_get_char();

    if ( _u8Comps_in_frame > JPEG_MAXCOMPONENTS )
    {
        //MDrv_JPEG_terminate( JPEG_TOO_MANY_COMPONENTS );
        return 0;
    }

    if ( left != ( U32 )( _u8Comps_in_frame * 3 + 8 ) )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_SOF_LENGTH );
        return 0;
    }

    for ( i = 0; i < _u8Comps_in_frame; i++ )
    {
        _u8Comp_ident[i] = MDrv_JPEG_get_char();
        if(_u8Comp_ident[i]==0)     // The component ID is start from 0 (0 1 2). The normal case is start from 1 (1 2 3) for YUV
        {
            _u8LumaCi = 0;
            _u8ChromaCi = 1;
        }
         tmp  = MDrv_JPEG_get_char();
        _u8Comp_h_samp[i] = (tmp & 0xF0)>>4;
        _u8Comp_v_samp[i] = (tmp & 0x0F);

        _u8Comp_quant[i] = MDrv_JPEG_get_char();
    }
    return 1;
}

static U32 G_MDrv_JPEG_locate_sof_marker( void )
{
    U32 c;

    if( !G_MDrv_JPEG_locate_soi_marker() )
        return 0;

    c = MDrv_JPEG_process_markers();

    if( c == 0 )
        return 0;

    switch ( c )
    {
        case M_SOF2:
            JPEG_PRINT("Progressive\n");
            _bProgressive_flag = 1;
        case M_SOF0:
            /* baseline DCT */
        case M_SOF1:
            /* extended sequential DCT */
            {
                if(c==M_SOF0||c==M_SOF1)
                {
                    JPEG_PRINT("Baseline\n");
                }
                if( !MDrv_JPEG_read_sof_marker() )
                    return 0;
                break;
            }
        case M_SOF9:
            /* Arithmetic coding */
            {
                return 0;
            }

        default:
            {
                return 0;
            }
    }
    return 1;
}


static int G_MDrv_JPEG_decode_init( void )
{
    U16 u16ImageXSize, u16ImageYSize;

    G_MDrv_JPEG_init();

    if( !G_MDrv_JPEG_locate_sof_marker() )
        return 0;

    // save the original image size
    _u16OriginalImage_x_size = u16ImageXSize = _u16Image_x_size;
    _u16OriginalImage_y_size = u16ImageYSize = _u16Image_y_size;

    if(_bProgressive_flag)
    {
        if( _u16Image_x_size*_u16Image_y_size*3*2 > INTERNAL_BUFFER_SIZE )
        {
            JPEG_PRINT("Progressive image size too big... do not handle it\n");
            return 0;
        }
    }

    return 1;
}



int G_MDrv_JPEG_constructor( U8 decode_type )
{

    _u8DecodeType = decode_type;
    if( !G_MDrv_JPEG_decode_init() )
        return 0;

    return 1;
}

//------------------------------------------------------------------------------
// SCAN TYPE
#define JPEG_GRAYSCALE 0
#define JPEG_YH1V1     1
#define JPEG_YH2V1     2
#define JPEG_YH1V2     3
#define JPEG_YH2V2     4
#define JPEG_YH4V1     5

//------------------------------------------------------------------------------
// Increase this if you increase the max width!
#define JPEG_MAXBLOCKSPERROW 6144

static U32 MDrv_JPEG_init_frame( void ) //ok
{

    if ( _u8Comps_in_frame == 1 )
    {
        gu8Scan_type = JPEG_GRAYSCALE;

        _u16Max_blocks_per_mcu = 1;

        gu8Max_mcu_x_size = 8;
        gu8Max_mcu_y_size = 8;
    }
    else if ( _u8Comps_in_frame == 3 )
    {
        if ( ( ( _u8Comp_h_samp[1] != 1 ) || ( _u8Comp_v_samp[1] != 1 ) ) ||   //support only U_H1V1 & V_H1V1
            ( ( _u8Comp_h_samp[2] != 1 ) || ( _u8Comp_v_samp[2] != 1 ) ) )
        {
            //MDrv_JPEG_terminate( JPEG_UNSUPPORTED_SAMP_FACTORS );
            return 0;
        }

        if ( ( _u8Comp_h_samp[0] == 1 ) && ( _u8Comp_v_samp[0] == 1 ) )
        {
            gu8Scan_type = JPEG_YH1V1; //4:4:4

            _u16Max_blocks_per_mcu = 3;

            gu8Max_mcu_x_size = 8;
            gu8Max_mcu_y_size = 8;
        }
        else if ( ( _u8Comp_h_samp[0] == 2 ) && ( _u8Comp_v_samp[0] == 1 ) )
        {
            gu8Scan_type = JPEG_YH2V1; //4:2:2

            _u16Max_blocks_per_mcu = 4;

            gu8Max_mcu_x_size = 16;
            gu8Max_mcu_y_size = 8;
        }
        else if ( ( _u8Comp_h_samp[0] == 1 ) && ( _u8Comp_v_samp[0] == 2 ) )
        {
            gu8Scan_type = JPEG_YH1V2;

            _u16Max_blocks_per_mcu = 4;

            gu8Max_mcu_x_size = 8;
            gu8Max_mcu_y_size = 16;
        }
        else if ( ( _u8Comp_h_samp[0] == 2 ) && ( _u8Comp_v_samp[0] == 2 ) )
        {
            gu8Scan_type = JPEG_YH2V2; //4:2:0

            _u16Max_blocks_per_mcu = 6;

            gu8Max_mcu_x_size = 16;
            gu8Max_mcu_y_size = 16;
        }
        else if ( ( _u8Comp_h_samp[0] == 4 ) && ( _u8Comp_v_samp[0] == 1 ) )
        {
            // 4:1:1
            gu8Scan_type = JPEG_YH4V1;

            _u16Max_blocks_per_mcu = 6;

            gu8Max_mcu_x_size = 32;
            gu8Max_mcu_y_size = 8;
            //#if CMODEL
            //MDrv_JPEG_terminate( JPEG_UNSUPPORTED_SAMP_FACTORS );
            //#endif
        }
        else
        {
            //MDrv_JPEG_terminate( JPEG_UNSUPPORTED_SAMP_FACTORS );
            return 0;
        }
    }
    else
    {
        //MDrv_JPEG_terminate( JPEG_UNSUPPORTED_COLORSPACE );
        return 0;
    }

    gu16Max_mcus_per_row = ( _u16Image_x_size + ( gu8Max_mcu_x_size - 1 ) ) / gu8Max_mcu_x_size;
    _u16Max_mcus_per_col = ( _u16Image_y_size + ( gu8Max_mcu_y_size - 1 ) ) / gu8Max_mcu_y_size;

    _u32Max_blocks_per_row = gu16Max_mcus_per_row * _u16Max_blocks_per_mcu;

    // Should never happen
    if ( _u32Max_blocks_per_row > JPEG_MAXBLOCKSPERROW )
    {
        //MDrv_JPEG_terminate( JPEG_ASSERTION_ERROR );
        return 0;
    }

    #if SUPPORT_PROGRESSIVE_MODE
    _u16Total_lines_left = _u16Max_mcus_per_col * gu8Max_mcu_y_size;
    gu16Mcu_lines_left = 0;
    #endif
    return 1;
}

static void *G_MDrv_JPEG_malloc (U32 size)
{
    __memp__ q;			/* ptr to free block */
    __memp__ p;			/* q->next */
    U32 k;			/* space remaining in the allocated block */
    q = &AVAIL;

    while (1)
    {
        if ((p = q->next) == NULL)
        {
            return (NULL);				/* FAILURE */
        }

        if (p->len >= size)
            break;

        q = p;
    }

    k = p->len - size;		/* calc. remaining bytes in block */

    if (k < MIN_BLOCK)		/* rem. bytes too small for new block */
    {
        q->next = p->next;
        return (&p[1]);				/* SUCCESS */
    }

    k -= HLEN;
    p->len = k;

    q = (__memp__ ) (((char *) (&p [1])) + k);
    q->len = size;
    q->next = NULL;

    return (&q[1]);					/* SUCCESS */
}

static void * alloc( U32 n )
{
    U8 i;
    void *q;

    // Find a free slot. The number of allocated slots will
    // always be very low, so a linear search is good enough.
    for ( i = 0; i < JPEG_MAXBLOCKS; i++ )
    {
        if ( _pBlocks[i] == NULL )
        {
            break;
        }
    }

    if ( i == JPEG_MAXBLOCKS )
    {
        //MDrv_JPEG_terminate( JPEG_TOO_MANY_BLOCKS );
        return NULL;
    }

    //FixMe: eCos does not support aligned allocate ???
    q = G_MDrv_JPEG_malloc( n + 8 );
    //q = MsOS_AllocateAlignedMemory(n+8, 8, gs32CachedPoolID);
    //q = MsOS_AllocateMemory(n+8, gs32CachedPoolID);

    if ( q == NULL )
    {
        //MDrv_JPEG_terminate( JPEG_NOTENOUGHMEM );
        return NULL;
    }

    memset( q, 0, n + 8 );

    _pBlocks[i] = q;

    JPEG_PRINT("JPEG %d bytes allocated\n", n);

    return ( ( void* )q );
}

static PCOEFF_BUF MDrv_JPEG_coeff_buf_open( U16 block_num_x, U16 block_num_y, U8 block_len_x, U8 block_len_y )
{
    PCOEFF_BUF cb = ( PCOEFF_BUF )alloc( sizeof( COEFF_BUF ) );
    if(cb == NULL)
    {
        //MDrv_JPEG_terminate( JPEG_NOTENOUGHMEM );
        return NULL;
    }
    cb->u16Block_num_x = block_num_x;
    cb->u16Block_num_y = block_num_y;

    cb->u8Block_len_x = block_len_x;
    cb->u8Block_len_y = block_len_y;

    cb->u16Block_size = ( block_len_x * block_len_y ) * sizeof( BLOCK_TYPE );

    cb->pu8Data = ( U8 * )alloc( cb->u16Block_size * block_num_x * block_num_y );
    if(cb->pu8Data == NULL)
    {
        //MDrv_JPEG_terminate( JPEG_NOTENOUGHMEM );
    }

    return cb;
}

static void G_MDrv_JPEG_free (void *memp)
{
    __memp__ q;		/* ptr to free block */
    __memp__ p;		/* q->next */
    __memp__ p0;		/* block to free */

    if ((memp == NULL) || (AVAIL.len == 0))
        return;

    p0 = (__memp__) memp;
    p0 = &p0 [-1];		/* get address of header */

    q = &AVAIL;

    while (1)
    {
        p = q->next;

        if ((p == NULL) || (p > (__memp__) memp))
            break;

        q = p;
    }

    if ((p != NULL) && ((((char *)memp) + p0->len) == (char *)p))
    {
        p0->len += p->len + HLEN;
        p0->next = p->next;
    }
    else
    {
        p0->next = p;
    }

    if ((((char *)q) + q->len + HLEN) == (char *)p0)
    {
        q->len += p0->len + HLEN;
        q->next = p0->next;
    }
    else
    {
        q->next = p0;
    }
}

static void MDrv_JPEG_free_all_blocks( void )
{
    U8 i;

    for ( i = 0; i < JPEG_MAXBLOCKS; i++ )
    {
        if( _pBlocks[i]!=NULL )
            G_MDrv_JPEG_free( _pBlocks[i] );  //need to check    sharon

        _pBlocks[i] = NULL;
    }
}


// Completely destroys the decoder object. May be called at any time.
static void G_MDrv_JPEG_Finalize( void )
{
    MDrv_JPEG_free_all_blocks();
}

typedef unsigned int ( *Pdecode_block_func )( U8, U16, U16 );

static U32 MDrv_JPEG_read_sos_marker( void )
{
    U32 left;
    U16 i, ci, n, c, cc;
    U8 tmp;

    JPEG_PRINT("SOS\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    n = MDrv_JPEG_get_char();

    _u8Comps_in_scan = n;

    left -= 3;

    if ( ( left != ( U32 )( n * 2 + 3 ) ) || ( n < 1 ) || ( n > JPEG_MAXCOMPSINSCAN ) )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_SOS_LENGTH );
        return 0;
    }

    for ( i = 0; i < n; i++ )
    {
        cc = MDrv_JPEG_get_char();
        c = MDrv_JPEG_get_char();
        left -= 2;

        for ( ci = 0; ci < _u8Comps_in_frame; ci++ )
        {
            if ( cc == _u8Comp_ident[ci] )
            {
                break;
            }
        }

        if ( ci >= _u8Comps_in_frame )
        {
            //MDrv_JPEG_terminate( JPEG_BAD_SOS_COMP_ID );
            return 0;
        }

        _u8Comp_list[i] = ci;
        _u8Comp_dc_tab[ci] = ( c >> 4 ) & 15;
        _u8Comp_ac_tab[ci] = ( c & 15 ) + ( JPEG_MAXHUFFTABLES >> 1 );
    }

    _u8Spectral_start = MDrv_JPEG_get_char();
    _u8Spectral_end = MDrv_JPEG_get_char();
    tmp = MDrv_JPEG_get_char();
    _u8Successive_high = (tmp & 0xF0) >> 4;
    _u8Successive_low = (tmp & 0x0F);

    if ( !_bProgressive_flag )
    {
        _u8Spectral_start = 0;
        _u8Spectral_end = 63;
    }

    left -= 3;
    MDrv_JPEG_skip_bytes(left);                 /* read past whatever is left */

    return 1;
}

static U32 MDrv_JPEG_locate_sos_marker( void )
{
    U32 c;

    c = MDrv_JPEG_process_markers();

    if( c == 0 )
        return 0;

    if ( c == M_EOI )
    {
        return 0;
    }
    else if ( c != M_SOS )
    {
        //MDrv_JPEG_terminate( JPEG_UNEXPECTED_MARKER );
        return 0;
    }

    if( !MDrv_JPEG_read_sos_marker())
        return 0;

    return 1;
}

static void MDrv_JPEG_calc_mcu_block_order( void ) //ok
{
    U8 component_num, component_id;
    U8 max_h_samp = 0, max_v_samp = 0;

    for ( component_id = 0; component_id < _u8Comps_in_frame; component_id++ )
    {
        if ( _u8Comp_h_samp[component_id] > max_h_samp )
        {
            max_h_samp = _u8Comp_h_samp[component_id];
        }

        if ( _u8Comp_v_samp[component_id] > max_v_samp )
        {
            max_v_samp = _u8Comp_v_samp[component_id];
        }
    }

    for ( component_id = 0; component_id < _u8Comps_in_frame; component_id++ )
    {
        _u16Comp_h_blocks[component_id] = ( ( ( ( _u16Image_x_size * _u8Comp_h_samp[component_id] ) + ( max_h_samp - 1 ) ) / max_h_samp ) + 7 ) / 8;
        _u16Comp_v_blocks[component_id] = ( ( ( ( _u16Image_y_size * _u8Comp_v_samp[component_id] ) + ( max_v_samp - 1 ) ) / max_v_samp ) + 7 ) / 8;
    }

    if ( _u8Comps_in_scan == 1 )
    {
        _u16Mcus_per_row = _u16Comp_h_blocks[_u8Comp_list[0]];
        _u16Mcus_per_col = _u16Comp_v_blocks[_u8Comp_list[0]];
    }
    else
    {
        _u16Mcus_per_row = ( ( ( _u16Image_x_size + 7 ) / 8 ) + ( max_h_samp - 1 ) ) / max_h_samp;
        _u16Mcus_per_col = ( ( ( _u16Image_y_size + 7 ) / 8 ) + ( max_v_samp - 1 ) ) / max_v_samp;
    }

    if ( _u8Comps_in_scan == 1 )
    {
        _u8Mcu_org[0] = _u8Comp_list[0];

        _u8Blocks_per_mcu = 1;
    }
    else
    {
        _u8Blocks_per_mcu = 0;

        for ( component_num = 0; component_num < _u8Comps_in_scan; component_num++ )
        {
            U8 num_blocks;

            component_id = _u8Comp_list[component_num];

            num_blocks = _u8Comp_h_samp[component_id] * _u8Comp_v_samp[component_id];

            while ( num_blocks-- )
            {
                _u8Mcu_org[_u8Blocks_per_mcu++] = component_id;
            }
        }
    }
}

static U32 MDrv_JPEG_make_huff_table( U8 index)
{
    U16 p, i, l, si;
    U8 huffsize[257];
    U16 huffcode[257];
    U16 test_code;
    U16 subtree;
    U16 code_size;
    U16 lastp;
    S16 nextfreeentry;
    S16 currententry;

    #if SUPPORT_PROGRESSIVE_MODE
    PHUFF_TABLES hs = &_Huff_tbls[index];
    #endif

    memset(huffsize, 0, sizeof(huffsize));
    memset(huffcode, 0, sizeof(huffcode));

    p = 0;

    for ( l = 1; l <= 16; l++ )
    {
        for ( i = 1; i <= _Huff_info[index].u8Huff_num[l]; i++ )
        {
            huffsize[p++] = l;

            //kevinhuang, add protection
            if ( p >= 257 )
            {
                //MDrv_JPEG_terminate( JPEG_UNDEFINED_HUFF_TABLE );
                return 0;
            }
        }
    }

    huffsize[p] = 0;

    lastp = p;

    test_code = 0;
    si = huffsize[0];
    p = 0;

    while ( huffsize[p] )
    {
        while ( huffsize[p] == si )
        {
            huffcode[p++] = test_code;
            test_code++;

            //kevinhuang, add protection
            if ( p >= 257 )
            {
                //MDrv_JPEG_terminate( JPEG_UNDEFINED_HUFF_TABLE );
                return 0;
            }
        }

        test_code <<= 1;
        si++;
    }

    // Calculate the min code
    for(i = 1; i<=16; i++)
        _Huff_info[index].u16Code[i] = huffcode[_Huff_info[index].u8Symbol[i]] << (15 - (i - 1));


    #if SUPPORT_PROGRESSIVE_MODE

    // In JPD mode, SW doesn't need huff table when baseline decoding
    if(_bProgressive_flag == 0)
        return 1;

    memset( hs->s16Look_up, 0, sizeof( hs->s16Look_up ) );
    memset( hs->s16Tree, 0, sizeof( hs->s16Tree ) );
    memset( hs->u8Code_size, 0, sizeof( hs->u8Code_size ) );

    nextfreeentry = -1;

    p = 0;

    while ( p < lastp )
    {
        i = _Huff_info[index].u8Huff_val[p];
        test_code = huffcode[p];
        code_size = huffsize[p];

        hs->u8Code_size[i] = code_size;

        if ( code_size <= 8 )
        {
            test_code <<= ( 8 - code_size );

            for ( l = 1 << ( 8 - code_size ); l > 0; l-- )
            {
                hs->s16Look_up[test_code] = i;
                test_code++;
            }
        }
        else
        {
            subtree = ( test_code >> ( code_size - 8 ) ) & 0xFF;

            currententry = hs->s16Look_up[subtree];

            if ( currententry == 0 )
            {
                hs->s16Look_up[subtree] = currententry = nextfreeentry;

                nextfreeentry -= 2;
            }

            test_code <<= ( 16 - ( code_size - 8 ) );

            for ( l = code_size; l > 9; l-- )
            {
                if ( ( test_code & 0x8000 ) == 0 )
                {
                    currententry--;
                }

                if ( hs->s16Tree[-currententry - 1] == 0 )
                {
                    hs->s16Tree[-currententry - 1] = nextfreeentry;

                    currententry = nextfreeentry;

                    nextfreeentry -= 2;
                }
                else
                {
                    currententry = hs->s16Tree[-currententry - 1];
                }

                test_code <<= 1;
            }

            if ( ( test_code & 0x8000 ) == 0 )
            {
                currententry--;
            }

            hs->s16Tree[-currententry - 1] = i;
        }

        p++;
    }
    #endif  //SUPPORT_PROGRESSIVE_MODE
    return 1;
}

static U32 MDrv_JPEG_check_huff_tables( void )
{
    U8 i;

    for ( i = 0; i < _u8Comps_in_scan; i++ )
    {
        if ( ( _u8Spectral_start == 0 ) && ( _Huff_info[_u8Comp_dc_tab[_u8Comp_list[i]]].bValid== 0 ) )
        {
            //MDrv_JPEG_terminate( JPEG_UNDEFINED_HUFF_TABLE );
            return 0;
        }

        if ( ( _u8Spectral_end > 0 ) && ( _Huff_info[_u8Comp_ac_tab[_u8Comp_list[i]]].bValid== 0 ) )
        {
            //MDrv_JPEG_terminate( JPEG_UNDEFINED_HUFF_TABLE );
            return 0;
        }
    }

    for ( i = 0; i < JPEG_MAXHUFFTABLES; i++ )
    {
        if ( _Huff_info[i].bValid )
        {
            if( !MDrv_JPEG_make_huff_table(i))
                return 0;
        }
    }
    return 1;
}

static U32 MDrv_JPEG_check_quant_tables( void ) //ok
{
    U8 i;

    for ( i = 0; i < _u8Comps_in_scan; i++ )
    {
        if ( _QuantTables[_u8Comp_quant[_u8Comp_list[i]]].bValid==0 )
        {
            //MDrv_JPEG_terminate( JPEG_UNDEFINED_QUANT_TABLE );
            return 0;
        }
    }
    return 1;
}

static U32 G_MDrv_JPEG_GetECS(void)
{
    //JPEG_PRINT("_u32Total_bytes_read = %08X\n",_u32Total_bytes_read);
    //JPEG_PRINT("_u32In_buf_left = %08X\n",_u32In_buf_left);
    return (_u32Total_bytes_read - _u32In_buf_left);
}

//------------------------------------------------------------------------------
// Logical rotate left operation.
static U32 MDrv_JPEG_rol( U32 i, U8 j )
{
    return ( ( i << j ) | ( i >> ( 32 - j ) ) );
}

static U8 MDrv_JPEG_get_charP( U32 *Ppadding_flag )
{
    U8 t;
    U8 c;

    if ( !_u32In_buf_left )
    {
        //MDrv_JPEG_fill_read_buffer();
        if ( !_u32In_buf_left )
        {
            *Ppadding_flag = 1;
            t = _u8Tem_flag;
            _u8Tem_flag ^= 1;
            if ( t )
            {
                return ( 0xD9 );
            }
            else
            {
                return ( 0xFF );
            }
        }
    }

    *Ppadding_flag = 0;

    c = *_pu8In_buf_ofs++;
    _u32In_buf_left--;

    return ( c );
}


//------------------------------------------------------------------------------
// Inserts a previously retrieved character back into the input buffer.
static void MDrv_JPEG_stuff_char( U8 q )
{
    *( --_pu8In_buf_ofs ) = q;
    _u32In_buf_left++;
}

static U8 MDrv_JPEG_get_octet( void )
{
    U32 padding_flag;
    U8 c = MDrv_JPEG_get_charP( &padding_flag );

    if ( c == 0xFF )
    {
        if ( padding_flag )
        {
            return ( 0xFF );
        }

        c = MDrv_JPEG_get_charP( &padding_flag );
        if ( padding_flag )
        {
            MDrv_JPEG_stuff_char( 0xFF );
            return ( 0xFF );
        }

        if ( c == 0x00 )
        {
            return ( 0xFF );
        }
        else
        {
            MDrv_JPEG_stuff_char( c );
            MDrv_JPEG_stuff_char( 0xFF );
            return ( 0xFF );
        }
    }

    return ( c );
}

static U32 MDrv_JPEG_get_bits_2( U8 numbits )
{
    U32 i, c1, c2;

    i = ( _u32Bit_buf >> ( 16 - numbits ) ) & ( ( 1 << numbits ) - 1 );

    if ( ( _s16Bits_left -= numbits ) <= 0 )
    {
        _u32Bit_buf = MDrv_JPEG_rol( _u32Bit_buf, numbits += _s16Bits_left );

        c1 = MDrv_JPEG_get_octet();
        c2 = MDrv_JPEG_get_octet();

        _u32Bit_buf = ( _u32Bit_buf & 0xFFFF ) | ( ( ( U32 )c1 ) << 24 ) | ( ( ( U32 )c2 ) << 16 );

        _u32Bit_buf = MDrv_JPEG_rol( _u32Bit_buf, -_s16Bits_left );

        _s16Bits_left += 16;
    }
    else
    {
        _u32Bit_buf = MDrv_JPEG_rol( _u32Bit_buf, numbits );
    }

    return i;
}



static U32 MDrv_JPEG_init_scan( void )
{

    if ( !MDrv_JPEG_locate_sos_marker() )
    {
        return 0;
    }

    MDrv_JPEG_calc_mcu_block_order();

    if( !MDrv_JPEG_check_huff_tables() )
        return 0;

    if( !MDrv_JPEG_check_quant_tables() )
        return 0;

    #if SUPPORT_PROGRESSIVE_MODE
    memset( _u32Last_dc_val, 0, _u8Comps_in_frame * sizeof( U32 ) );

    _u32EOB_run = 0;
    #endif

    if ( _u16Restart_interval )
    {
        _u16Restarts_left = _u16Restart_interval;
        _u16Next_restart_num = 0;
    }

    _Total_Decoded_Size = (S32)G_MDrv_JPEG_GetECS();

    //JPEG_DEBUG("ECS %08x\n", MDrv_JPEG_GetECS());

    if(_bProgressive_flag)
    {
        // pre-fill bit buffer for later decoding
        _s16Bits_left = 16;
        MDrv_JPEG_get_bits_2( 16 );
        MDrv_JPEG_get_bits_2( 16 );
    }

    return 1;
}

static BLOCK_TYPE * MDrv_JPEG_coeff_buf_getp( PCOEFF_BUF cb, U16 block_x, U16 block_y )
{
    if ( block_x >= cb->u16Block_num_x )
    {
        //MDrv_JPEG_terminate( JPEG_ASSERTION_ERROR );
        return NULL;
    }

    if ( block_y >= cb->u16Block_num_y )
    {
        //MDrv_JPEG_terminate( JPEG_ASSERTION_ERROR );
        return NULL;
    }

    return ( BLOCK_TYPE * )( cb->pu8Data + block_x * cb->u16Block_size + block_y * ( cb->u16Block_size * cb->u16Block_num_x ) );
}

static U32 MDrv_JPEG_decode_block_dc_refine( //JPEG_DECODER *Pd,
  U8 component_id, U16 block_x, U16 block_y )
{
    if ( MDrv_JPEG_get_bits_2( 1 ) )
    {
        BLOCK_TYPE *p = MDrv_JPEG_coeff_buf_getp( _DC_Coeffs[component_id], block_x, block_y );

        if(p == NULL)
        {
            //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
            return 0;
        }
        p[0] |= ( 1 << _u8Successive_low );
    }
    return 1;
}

static S32 MDrv_JPEG_huff_decode( PHUFF_TABLES Ph )
{
    S32 symbol;

    // Check first 8-bits: do we have a complete symbol?
    if ( ( symbol = Ph->s16Look_up[( _u32Bit_buf >> 8 ) & 0xFF] ) < 0 )
    {
        // Decode more bits, use a tree traversal to find symbol.
        MDrv_JPEG_get_bits_2( 8 );

        do
        {
            symbol = Ph->s16Tree[~symbol + ( 1 - MDrv_JPEG_get_bits_2( 1 ) )];
        }
        while ( symbol < 0 );
    }
    else
    {
        MDrv_JPEG_get_bits_2( Ph->u8Code_size[symbol] );
    }

    return symbol;
}

//------------------------------------------------------------------------------
// Tables and macro used to fully decode the DPCM differences.
// (Note: In x86 asm this can be done without using tables.)
static const S32 extend_test[16] =   /* entry n is 2**(n-1) */
{
    0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040,
    0x0080, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000
};

static const S32 extend_offset[16] = /* entry n is (-1 << n) + 1 */
{
    0, ( ( -1 ) << 1 ) + 1, ( ( -1 ) << 2 ) + 1, ( ( -1 ) << 3 ) + 1, ( ( -1 ) << 4 ) + 1,
    ( ( -1 ) << 5 ) + 1, ( ( -1 ) << 6 ) + 1, ( ( -1 ) << 7 ) + 1, ( ( -1 ) << 8 ) + 1,
    ( ( -1 ) << 9 ) + 1, ( ( -1 ) << 10 ) + 1, ( ( -1 ) << 11 ) + 1, ( ( -1 ) << 12 ) + 1,
    ( ( -1 ) << 13 ) + 1, ( ( -1 ) << 14 ) + 1, ( ( -1 ) << 15 ) + 1
};

// used by huff_extend()
static const S32 extend_mask[] =
{
    0, ( 1 << 0 ), ( 1 << 1 ), ( 1 << 2 ), ( 1 << 3 ),
    ( 1 << 4 ), ( 1 << 5 ), ( 1 << 6 ), ( 1 << 7 ),
    ( 1 << 8 ), ( 1 << 9 ), ( 1 << 10 ), ( 1 << 11 ),
    ( 1 << 12 ), ( 1 << 13 ), ( 1 << 14 ), ( 1 << 15 ), ( 1 << 16 ),
};

#define HUFF_EXTEND_TBL(x,s) ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

#define HUFF_EXTEND(x,s) HUFF_EXTEND_TBL(x,s)
#define HUFF_EXTEND_P(x,s) HUFF_EXTEND_TBL(x,s)

static U32 MDrv_JPEG_decode_block_dc_first( //JPEG_DECODER *Pd,
  U8 component_id, U16 block_x, U16 block_y )
{
    S32 s, r;
    BLOCK_TYPE *p = MDrv_JPEG_coeff_buf_getp( _DC_Coeffs[component_id], block_x, block_y );

    if(p == NULL)
    {
        //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
        return 0;
    }
    if ( ( s = MDrv_JPEG_huff_decode( &_Huff_tbls[_u8Comp_dc_tab[component_id]] ) ) != 0 )
    {
        r = MDrv_JPEG_get_bits_2( s );
        s = HUFF_EXTEND_P( r, s );
    }

    // In JPD mode, the DC coefficient is the difference of nearest DC
    _u32Last_dc_val[component_id] = ( s += _u32Last_dc_val[component_id] );

    p[0] = s << _u8Successive_low;
    return 1;
}


//------------------------------------------------------------------------------
///void decode_block_ac_refine(
static U32 MDrv_JPEG_decode_block_ac_refine( //JPEG_DECODER *Pd,
  U8 component_id, U16 block_x, U16 block_y )
{
    S32 s, k, r;
    S32 p1 = 1 << _u8Successive_low;
    S32 m1 = ( -1 ) << _u8Successive_low;
    BLOCK_TYPE *p = MDrv_JPEG_coeff_buf_getp( _AC_Coeffs[component_id], block_x, block_y );

    if(p == NULL)
    {
        //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
        return 0;
    }
    k = _u8Spectral_start;

    if ( _u32EOB_run == 0 )
    {
        for ( ; k <= _u8Spectral_end; k++ )
        {
            s = MDrv_JPEG_huff_decode( &_Huff_tbls[_u8Comp_ac_tab[component_id]] );

            r = s >> 4;
            s &= 15;

            if ( s )
            {
                if ( s != 1 )
                {
                    //MDrv_JPEG_terminate( JPEG_DECODE_ERROR );
                    return 0;
                }

                if ( MDrv_JPEG_get_bits_2( 1 ) )
                {
                    s = p1;
                }
                else
                {
                    s = m1;
                }
            }
            else
            {
                if ( r != 15 )
                {
                    _u32EOB_run = 1 << r;

                    if ( r )
                    {
                        _u32EOB_run += MDrv_JPEG_get_bits_2( r );
                    }

                    break;
                }
            }

            do
            {
                // No need to do ZAG order in JPD mode
                BLOCK_TYPE *this_coef = p + k;

                if ( *this_coef != 0 )
                {
                    if ( MDrv_JPEG_get_bits_2( 1 ) )
                    {
                        if ( ( *this_coef & p1 ) == 0 )
                        {
                            if ( *this_coef >= 0 )
                            {
                                *this_coef += p1;
                            }
                            else
                            {
                                *this_coef += m1;
                            }
                        }
                    }
                }
                else
                {
                    if ( --r < 0 )
                    {
                        break;
                    }
                }

                k++;
            }
            while ( k <= _u8Spectral_end );

            if ( ( s ) && ( k < 64 ) )
            {
                // No need to do ZAG order in JPD mode
                p[k] = s;
            }
        }
    }

    if ( _u32EOB_run > 0 )
    {
        for ( ; k <= _u8Spectral_end; k++ )
        {
            // No need to do ZAG order in JPD mode
            BLOCK_TYPE *this_coef = p + k;

            if ( *this_coef != 0 )
            {
                if ( MDrv_JPEG_get_bits_2( 1 ) )
                {
                    if ( ( *this_coef & p1 ) == 0 )
                    {
                        if ( *this_coef >= 0 )
                        {
                            *this_coef += p1;
                        }
                        else
                        {
                            *this_coef += m1;
                        }
                    }
                }
            }
        }

        _u32EOB_run--;
    }
    return 1;
}

static U32 MDrv_JPEG_decode_block_ac_first( //JPEG_DECODER *Pd,
  U8 component_id, U16 block_x, U16 block_y )
{
    BLOCK_TYPE *p;
    S32 k, s, r;

    if ( _u32EOB_run )
    {
        _u32EOB_run--;
        return 1;
    }

    p = MDrv_JPEG_coeff_buf_getp( _AC_Coeffs[component_id], block_x, block_y );

    if(p == NULL)
    {
        //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
        return 0;
    }
    for ( k = _u8Spectral_start; k <= _u8Spectral_end; k++ )
    {
        s = MDrv_JPEG_huff_decode( &_Huff_tbls[_u8Comp_ac_tab[component_id]] );

        r = s >> 4;
        s &= 15;

        if ( s )
        {
            if ( ( k += r ) > 63 )
            {
                //MDrv_JPEG_terminate( JPEG_DECODE_ERROR );
                return 0;
            }

            r = MDrv_JPEG_get_bits_2( s );
            s = HUFF_EXTEND_P( r, s );

            // No need to do ZAG order in JPD mode
            p[k] = s << _u8Successive_low;
        }
        else
        {
            if ( r == 15 )
            {
                if ( ( k += 15 ) > 63 )
                {
                    //MDrv_JPEG_terminate( JPEG_DECODE_ERROR );
                    return 0;
                }
            }
            else
            {
                _u32EOB_run = 1 << r;

                if ( r )
                {
                    _u32EOB_run += MDrv_JPEG_get_bits_2( r );
                }

                _u32EOB_run--;

                break;
            }
        }
    }
    return 1;
}

static U32 MDrv_JPEG_process_restart( void )
{
    U16 i, c = 0;

    // Let's scan a little bit to find the marker, but not _too_ far.
    // 1536 is a "fudge factor" that determines how much to scan.
    for ( i = 1536; i > 0; i-- )
    {
        if ( MDrv_JPEG_get_char() == 0xFF )
        {
            break;
        }
    }

    if ( i == 0 )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_RESTART_MARKER );
        return 0;
    }

    for ( ; i > 0; i-- )
    {
        if ( ( c = MDrv_JPEG_get_char() ) != 0xFF )
        {
            break;
        }
    }

    if ( i == 0 )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_RESTART_MARKER );
        return 0;
    }

    // Is it the expected marker? If not, something bad happened.
    if ( c != ( _u16Next_restart_num + M_RST0 ) )
    {
        //MDrv_JPEG_terminate( JPEG_BAD_RESTART_MARKER );
        return 0;
    }

    // Reset each component's DC prediction values.
    memset( &_u32Last_dc_val, 0, _u8Comps_in_frame * sizeof( U32 ) );

    _u32EOB_run = 0;

    _u16Restarts_left = _u16Restart_interval;

    _u16Next_restart_num = ( _u16Next_restart_num + 1 ) & 7;

    // Get the bit buffer going again...
    {
        _s16Bits_left = 16;

        MDrv_JPEG_get_bits_2( 16 );
        MDrv_JPEG_get_bits_2( 16 );
    }
    return 1;
}


static U32 MDrv_JPEG_decode_scan( Pdecode_block_func decode_block_func )
{
    U16 mcu_row, mcu_col, mcu_block;
    U32 block_x_mcu[JPEG_MAXCOMPONENTS], block_y_mcu[JPEG_MAXCOMPONENTS];

    memset( block_y_mcu, 0, sizeof( block_y_mcu ) );

    for ( mcu_col = 0; mcu_col < _u16Mcus_per_col; mcu_col++ )
    {
        int component_num, component_id;

        memset( block_x_mcu, 0, sizeof( block_x_mcu ) );

        for ( mcu_row = 0; mcu_row < _u16Mcus_per_row; mcu_row++ )
        {
            U8 block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;

            if ( ( _u16Restart_interval ) && ( _u16Restarts_left == 0 ) )
            {
                if(!MDrv_JPEG_process_restart())
                    return 0;
            }

            for ( mcu_block = 0; mcu_block < _u8Blocks_per_mcu; mcu_block++ )
            {
                component_id = _u8Mcu_org[mcu_block];

                if(!decode_block_func( component_id, block_x_mcu[component_id] + block_x_mcu_ofs, block_y_mcu[component_id] + block_y_mcu_ofs ))
                {
                    //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
                    return 0;
                }

                if ( _u8Comps_in_scan == 1 )
                {
                    block_x_mcu[component_id]++;
                }
                else
                {
                    if ( ++block_x_mcu_ofs == _u8Comp_h_samp[component_id] )
                    {
                        block_x_mcu_ofs = 0;

                        if ( ++block_y_mcu_ofs == _u8Comp_v_samp[component_id] )
                        {
                            block_y_mcu_ofs = 0;

                            block_x_mcu[component_id] += _u8Comp_h_samp[component_id];
                        }
                    }
                }
            }

            _u16Restarts_left--;
        }

        if ( _u8Comps_in_scan == 1 )
        {
            block_y_mcu[_u8Comp_list[0]]++;
        }
        else
        {
            for ( component_num = 0; component_num < _u8Comps_in_scan; component_num++ )
            {
                component_id = _u8Comp_list[component_num];

                block_y_mcu[component_id] += _u8Comp_v_samp[component_id];
            }
        }
    }
    return 1;
}

static U32 MDrv_JPEG_init_progressive( void )
{
    U8 i;

    if ( _u8Comps_in_frame == 4 )
    {
        //MDrv_JPEG_terminate( JPEG_UNSUPPORTED_COLORSPACE );
        return 0;
    }

    // Allocate the coefficient buffers.
    for ( i = 0; i < _u8Comps_in_frame; i++ )
    {
        _DC_Coeffs[i] = MDrv_JPEG_coeff_buf_open( ((gu16Max_mcus_per_row+0x1)& ~0x1) * _u8Comp_h_samp[i], ((_u16Max_mcus_per_col+0x1)& ~0x1) * _u8Comp_v_samp[i], 1, 1 );
        _AC_Coeffs[i] = MDrv_JPEG_coeff_buf_open( ((gu16Max_mcus_per_row+0x1)& ~0x1) * _u8Comp_h_samp[i], ((_u16Max_mcus_per_col+0x1)& ~0x1) * _u8Comp_v_samp[i], 8, 8 );
        if(_DC_Coeffs[i]== NULL || _AC_Coeffs[i] == NULL)
        {
            //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
            G_MDrv_JPEG_Finalize();
            return 0;
        }
    }

    for ( ; ; )
    {
        U32 dc_only_scan, refinement_scan;
        Pdecode_block_func decode_block_func;

        if ( !MDrv_JPEG_init_scan() )
        {
            break;
        }

        dc_only_scan = ( _u8Spectral_start == 0 );
        refinement_scan = ( _u8Successive_high != 0 );

        if ( ( _u8Spectral_start > _u8Spectral_end ) || ( _u8Spectral_end > 63 ) )
        {
            //MDrv_JPEG_terminate( JPEG_BAD_SOS_SPECTRAL );
            return 0;
        }

        if ( dc_only_scan )
        {
            if ( _u8Spectral_end )
            {
                //MDrv_JPEG_terminate( JPEG_BAD_SOS_SPECTRAL );
                return 0;
            }
        }
        else if ( _u8Comps_in_scan != 1 )  /* AC scans can only contain one component */
        {
            //MDrv_JPEG_terminate( JPEG_BAD_SOS_SPECTRAL );
            return 0;
        }

        if ( ( refinement_scan ) && ( _u8Successive_low != _u8Successive_high - 1 ) )
        {
            //MDrv_JPEG_terminate( JPEG_BAD_SOS_SUCCESSIVE );
            return 0;
        }

        if ( dc_only_scan )
        {
            if ( refinement_scan )
            {
                decode_block_func = MDrv_JPEG_decode_block_dc_refine;
            }
            else
            {
                decode_block_func = MDrv_JPEG_decode_block_dc_first;
            }
        }
        else
        {
            if ( refinement_scan )
            {
                decode_block_func = MDrv_JPEG_decode_block_ac_refine;
            }
            else
            {
                decode_block_func = MDrv_JPEG_decode_block_ac_first;
            }
        }

        if( !MDrv_JPEG_decode_scan( decode_block_func ) )
            return 0;

    }

    _u8Comps_in_scan = _u8Comps_in_frame;

    for ( i = 0; i < _u8Comps_in_frame; i++ )
    {
        _u8Comp_list[i] = i;
    }

    MDrv_JPEG_calc_mcu_block_order();
    return 1;
}

static U32 MDrv_JPEG_init_sequential( void )
{
    if ( !MDrv_JPEG_init_scan() )
    {
        JPEG_PRINT("MDrv_JPEG_init_scan - JPEG_UNEXPECTED_MARKER\n");
        //MDrv_JPEG_terminate( JPEG_UNEXPECTED_MARKER );
        return 0;
    }
    return 1;
}



U32 G_MDrv_JPEG_DecodeHeader(void)
{
    if(!MDrv_JPEG_init_frame())
    {
        //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if ( _bProgressive_flag )
    {
        if(!MDrv_JPEG_init_progressive())
            return 0;
    }
    else
    {
        if(!MDrv_JPEG_init_sequential())
            return 0;
    }
    return 1;
}


U32 G_MDrv_JPEG_IsProgressive(void)
{
    return _bProgressive_flag;
}

//------------------------------------------------------------------------------
// JPEG DECODE RESULT
#define JPEG_FAILED -1
#define JPEG_DONE       1
#define JPEG_OKAY       0

typedef struct tagSVLD
{
    union
    {
        struct
        {
            U32 amp			:11;
            U32 sign		:1;
            U32 run			:4;
            U32 sym_type	:2;
            U32 blk_type	:2;
            U32 EOP			:1;
            U32 trash		:11;
        };

        struct
        {
            U8 byte0;
            U8 byte1;
            U8 byte2;
            U8 byte3;
        };
    };
} SVLD, *PSVLD;

#define ABS(x)    ((x>=0)?(x):(-x))

typedef enum
{
	RLE_DC = 0,
	RLE_AC,
	RLE_ZRL,
	RLE_EOB,
} EN_RLE_SYMBOL;

#define REG_JPD_BASE               0xBF202E00 // Titania3
#define JPD_REG(addr)              (*((volatile U32*)(REG_JPD_BASE + ((addr)<<2))))

//******************************************************************************
//          Structures
//******************************************************************************

#define AEON_NON_CACHE_MASK  0x80000000  //need to check   sharon

//#define JPD_REG_BASE        0x3000

/******* Internal table SRAM address *******/
#define JPD_MEM_SCWGIF_BASE     0x0000
#define JPD_MEM_SYMIDX_BASE     0x0200
#define JPD_MEM_QTBL_BASE       0x0400
/**************************************/
#define JPD_SWRST_S4L   (1<<13)   // Titania
#define JPD_ROI_EN      (1<<11)
#define JPD_SVLD        (1<<10)
#define JPD_SUVQ        (1<<9)
#define JPD_TBL_RDY     (1<<8)
#define JPD_DEC_EN      (1<<7)
#define JPD_RST_EN      (1<<6)
#define JPD_UV          (1<<3)
#define JPD_Y_VSF1      (0<<2)
#define JPD_Y_VSF2      (1<<2)
#define JPD_Y_HSF1      0x01
#define JPD_Y_HSF2      0x02
#define JPD_Y_HSF4      0x03

#define JPD_H_VLD       (1<<1)
#define JPD_L_VLD       (1<<0)

#define OFFSET(x)       (x*2)

#define REG_JPD_SCONFIG          0x00
#define REG_JPD_MCONFIG          0x01
#define REG_JPD_RSTINTV          0x02
#define REG_JPD_PIC_H            0x03
#define REG_JPD_PIC_V            0x04

#define REG_JPD_ROI_H            0x05
#define REG_JPD_ROI_V            0x06
#define REG_JPD_ROI_WIDTH        0x07
#define REG_JPD_ROI_HEIGHT       0x08

#define REG_JPD_INTEN            0x09
#define REG_JPD_EVENTFLAG        0x0A
#define REG_JPD_RCSMADR_L        0x0B
#define REG_JPD_RCSMADR_H        0x0C
#define REG_JPD_RBUF_FLOOR_L     0x0D
#define REG_JPD_RBUF_FLOOR_H     0x0E
#define REG_JPD_RBUF_CEIL_L      0x0F
#define REG_JPD_RBUF_CEIL_H      0x10
#define REG_JPD_MWBF_SADR_L      0x11
#define REG_JPD_MWBF_SADR_H      0x12
// <CK>
#if 1
// only T3, T2 does NOT have this
#define REG_JPD_MWBF_LINE_NUM    0x13

#define REG_JPD_CUR_MADR_L       0x14
#define REG_JPD_CUR_MADR_H       0x15
#define REG_JPD_CUR_ROWP         0x16
#define REG_JPD_CUR_CLNP         0x17
#define REG_JPD_CUR_VIDX         0x18
#define REG_JPD_BIST_FAIL        0x19
#define REG_JPD_DBG_MATCHV       0x1A
#define REG_JPD_DBG_CTRL         0x1B

// only T3, T2 does NOT have this
#define REG_JPD_IP_VERSION       0x28

#else
#define REG_JPD_CUR_MADR_L       0x13
#define REG_JPD_CUR_MADR_H       0x14
#define REG_JPD_CUR_ROWP         0x15
#define REG_JPD_CUR_CLNP         0x16
#define REG_JPD_CUR_VIDX         0x17
#define REG_JPD_BIST_FAIL        0x18
#define REG_JPD_DBG_MATCHV       0x19
#define REG_JPD_DBG_CTRL         0x1A
#endif
// </CK>
#define REG_JPD_TID_ADR          0x40
#define REG_JPD_TID_DAT          0x41

// Chip TOP  //need to check    sharon
#define JPD_CLOCK_S4    (OFFSET(0x0F2D))  // Eris
#define JPD_CLOCK_S4L   (OFFSET(0x0F4D))    // Titania

static void MHal_JPD_ClearJPDStatus(U16 status_bit)
{
    JPD_REG(REG_JPD_EVENTFLAG) = status_bit;
    JPEG_PRINT("Clear: Bit= %X", status_bit);
    if( status_bit == 0x10 )
    {
        JPD_REG(REG_JPD_MCONFIG) |= 0x01;
    }
    else if( status_bit == 0x20 )
    {
        JPD_REG(REG_JPD_MCONFIG) |= 0x02;
    }
    // clear by write
    //MDrv_Write2Byte(BK_JPD_EVENTFLAG, status_bit);
}

// Pluto later
#define JPD_MWC_WPEN        (1<<12)
#define JPD_MWC_ACTIVE        (1<<11)
#define JPD_MRC_ACTIVE        (1<<10)
#define JPD_MWC_PAUSE        (1<<9)
#define JPD_MRC_PAUSE        (1<<8)

/************* Downscale Ratio *************/
/*** Bellows are 1, 1/2, 1/4 and 1/8, in order ***/
#define JPD_DOWNSCALE_ORG       0x00
#define JPD_DOWNSCALE_HALF      0x01
#define JPD_DOWNSCALE_FOURTH    0x02
#define JPD_DOWNSCALE_EIGHTH    0x03
/********************I*********************/

/*** Enable interrupt (event report) ***/
#define JPD_EVENT_DEC_DONE      0x01
#define JPD_EVENT_ECS_ERROR     0x02
#define JPD_EVENT_IS_ERROR      0x04
#define JPD_EVENT_RST_ERROR     0x08
#define JPD_EVENT_MRBL_DONE     0x10
#define JPD_EVENT_MRBH_DONE     0x20
#define JPD_EVENT_ALL           0x3f
#define JPD_EVENT_ERROR_MASK    0x0E
/********************************/

static void MHal_JPD_Reset(void)
{
    JPD_REG(REG_JPD_SCONFIG) = 0;
    JPD_REG(REG_JPD_SCONFIG) = JPD_SWRST_S4L;
    JPD_REG(REG_JPD_INTEN) = JPD_EVENT_ALL;

}

static unsigned int v2p_offset = 0 ;

static void MHal_JPD_SetReadBuffer(U32 u32BufAddr, U32 u32BufSize)
{
    //u32BufAddr = virt_to_phys(u32BufAddr);
    //JPEG_PRINT("MRB v2p= 0x%X\n", u32BufAddr);

	u32BufAddr -= v2p_offset;
    JPEG_PRINT("MRB= 0x%X\n", u32BufAddr);

    JPD_REG(REG_JPD_RBUF_FLOOR_L) = (u32BufAddr >> 3) & 0xffff;
    JPD_REG(REG_JPD_RBUF_FLOOR_H) = (u32BufAddr >> 3) >> 16;

    JPD_REG(REG_JPD_RBUF_CEIL_L) = (((( u32BufAddr + u32BufSize ) >> 3)-1)&0xffff);
    JPD_REG(REG_JPD_RBUF_CEIL_H) = ((( u32BufAddr + u32BufSize ) >> 3)-1) >> 16;

#if 0
    u32BufAddr = u32BufAddr & ~AEON_NON_CACHE_MASK;

    // set start address of read buffer
    MDrv_Write2Byte(BK_JPD_RBUF_FLOOR_L, ( u32BufAddr >> 3 ) & 0xffff);
    MDrv_Write2Byte(BK_JPD_RBUF_FLOOR_H, ( u32BufAddr >> 3 ) >> 16);

    // set end address of read buffer
    MDrv_Write2Byte(BK_JPD_RBUF_CEIL_L, (( ( u32BufAddr + u32BufSize ) >> 3 ) & 0xffff) - 1);
    MDrv_Write2Byte(BK_JPD_RBUF_CEIL_H, ( ( u32BufAddr + u32BufSize ) >> 3 ) >> 16);
#endif

    //printf("BK_JPD_RBUF_FLOOR_L = %08x\n",( u32BufAddr >> 3 ) & 0xffff);
    //printf("BK_JPD_RBUF_FLOOR_H = %08x\n",( u32BufAddr >> 3 ) >> 16);
    //printf("BK_JPD_RBUF_CEIL_L = %08x\n",(( ( u32BufAddr + u32BufSize ) >> 3 ) & 0xffff) - 1);
    //printf("BK_JPD_RBUF_CEIL_H = %08x\n",( ( u32BufAddr + u32BufSize ) >> 3 ) >> 16);
}

static void MHal_JPD_SetMRCStartAddr(U32 u32ByteOffset)
{
    //u32ByteOffset = virt_to_phys(u32ByteOffset);
    //JPEG_PRINT("MRCStart v2p= 0x%X\n", u32ByteOffset);

    u32ByteOffset -= v2p_offset;
    JPEG_PRINT("MRCStart= 0x%X\n", u32ByteOffset);

    JPD_REG(REG_JPD_RCSMADR_L) = (u32ByteOffset & 0xffff);
    JPD_REG(REG_JPD_RCSMADR_H) = (u32ByteOffset >> 16);

#if 0
    u32ByteOffset = u32ByteOffset & ~AEON_NON_CACHE_MASK;

    MDrv_Write2Byte(BK_JPD_RCSMADR_L, (u32ByteOffset & 0xffff));
    MDrv_Write2Byte(BK_JPD_RCSMADR_H, (u32ByteOffset >> 16));
#endif
}


static void MHal_JPD_SetOutputFrameBuffer(U32 u32BufAddr)
{
    //u32BufAddr = virt_to_phys(u32BufAddr);
    //JPEG_PRINT("MWB v2p= 0x%X\n", u32BufAddr);

    u32BufAddr -= v2p_offset;
    JPEG_PRINT("MWB= 0x%X\n", u32BufAddr);

    JPD_REG(REG_JPD_MWBF_SADR_L) = (u32BufAddr >> 3) & 0xffff;
    JPD_REG(REG_JPD_MWBF_SADR_H) = (u32BufAddr >> 3) >> 16;

#if 0
    u32BufAddr = u32BufAddr & ~AEON_NON_CACHE_MASK;

    MDrv_Write2Byte(BK_JPD_MWBF_SADR_L, ( u32BufAddr >> 3 ) & 0xffff);
    MDrv_Write2Byte(BK_JPD_MWBF_SADR_H, ( u32BufAddr >> 3 ) >> 16);
#endif
}

static void MHal_JPD_SetPicDimension(U16 u16Width, U16 u16Height)
{
    JPD_REG(REG_JPD_PIC_H) = (u16Width >> 3);
    JPD_REG(REG_JPD_PIC_V) = (u16Height >> 3);

#if 0
    MDrv_Write2Byte( BK_JPD_PIC_H, ( u16Width >> 3 ) );
    MDrv_Write2Byte( BK_JPD_PIC_V, ( u16Height >> 3 ) );
#endif
}


static int MAX_JPEG_WIDTH_HD = 2048;
static int MAX_JPEG_HEIGHT_HD = 1536;

static int MAX_JPEG_PROGRESSIVE_WIDTH  = 1024;
static int MAX_JPEG_PROGRESSIVE_HEIGHT = 768;

static int MAX_JPEG_BASELINE_WIDTH  = 2048;
static int MAX_JPEG_BASELINE_HEIGHT = 1536;


typedef enum
{
    RATIO_ORIGINAL = 0,
    RATIO_HALF = 1,
    RATIO_FOURTH = 2,
    RATIO_EIGHTH = 3
} EN_JPEG_DOWNSCALE_RATIO;


static void MDrv_JPEG_write_Scwgif(void) //type : luma=>0  chroma=>1
{
    U32 reg_value;
    U16 i, ci, valid, tbl_num_luma, tbl_num_chroma;
    U8 luma_ci = 0, chroma_ci = 0;


    for(ci = 0; ci<_u8Comps_in_frame; ci++)
    {
        if(_u8LumaCi==_u8Comp_ident[ci])
        {
            luma_ci = ci;
            break;
        }
    }

    for(ci = 0; ci<_u8Comps_in_frame; ci++)
    {
        if(_u8ChromaCi==_u8Comp_ident[ci])
        {
            chroma_ci = ci;
            break;
        }
    }

    tbl_num_luma = _u8Comp_dc_tab[luma_ci];
    tbl_num_chroma = _u8Comp_dc_tab[chroma_ci];

    JPD_REG(REG_JPD_TID_ADR) = JPD_MEM_SCWGIF_BASE;

    for ( i = 1; i <= 16; i++ )
    {
        if(_Huff_info[tbl_num_luma].u8Symbol[i] == 0xFF)
            valid = 0;
        else
            valid = 1;

        if ( valid )
        {
            reg_value = ( valid << 24 ) | ( _Huff_info[tbl_num_luma].u16Code[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Symbol[i] << 4 );
        }
        else
        {
            reg_value = 0;
        }

        JPD_REG(REG_JPD_TID_DAT) = reg_value & 0xffff;
        JPD_REG(REG_JPD_TID_DAT) = reg_value >> 16;
        //MDrv_Write2Byte(REG_JPD_TID_DAT, reg_value & 0xffff);
        //MDrv_Write2Byte(REG_JPD_TID_DAT, reg_value >> 16);
    }

    for ( i = 1; i <= 16; i++ )
    {
        if(_Huff_info[tbl_num_chroma].u8Symbol[i] == 0xFF)
            valid = 0;
        else
            valid = 1;

        if ( valid )
        {
            reg_value = ( valid << 24 ) | ( _Huff_info[tbl_num_chroma].u16Code[i] << 8 ) | ( _Huff_info[tbl_num_chroma].u8Symbol[i] << 4 );
        }
        else
        {
            reg_value = 0;
        }

        JPD_REG(REG_JPD_TID_DAT) = reg_value & 0xffff;
        JPD_REG(REG_JPD_TID_DAT) = reg_value >> 16;
        //MDrv_Write2Byte(REG_JPD_TID_DAT, reg_value & 0xffff);
        //MDrv_Write2Byte(REG_JPD_TID_DAT, reg_value >> 16);
    }

    tbl_num_luma = _u8Comp_ac_tab[luma_ci];
    tbl_num_chroma = _u8Comp_ac_tab[chroma_ci];

    for ( i = 1; i <= 16; i++ )
    {
        if(_Huff_info[tbl_num_luma].u8Symbol[i] == 0xFF)
            valid = 0;
        else
            valid = 1;

        if ( valid )
        {
            reg_value = ( valid << 24 ) | ( _Huff_info[tbl_num_luma].u16Code[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Symbol[i] );
        }
        else
        {
            reg_value = 0;
        }

        JPD_REG(REG_JPD_TID_DAT) = reg_value & 0xffff;
        JPD_REG(REG_JPD_TID_DAT) = reg_value >> 16;
        //MDrv_Write2Byte(REG_JPD_TID_DAT, reg_value & 0xffff);
        //MDrv_Write2Byte(REG_JPD_TID_DAT, reg_value >> 16);
    }

    for ( i = 1; i <= 16; i++ )
    {
        if(_Huff_info[tbl_num_chroma].u8Symbol[i] == 0xFF)
            valid = 0;
        else
            valid = 1;

        if ( valid )
        {
            reg_value = ( valid << 24 ) | ( _Huff_info[tbl_num_chroma].u16Code[i] << 8 ) | ( _Huff_info[tbl_num_chroma].u8Symbol[i] );
        }
        else
        {
            reg_value = 0;
        }

        JPD_REG(REG_JPD_TID_DAT) = reg_value & 0xffff;
        JPD_REG(REG_JPD_TID_DAT) = reg_value >> 16;
        //MDrv_Write2Byte(REG_JPD_TID_DAT, reg_value & 0xffff);
        //MDrv_Write2Byte(REG_JPD_TID_DAT, reg_value >> 16);
    }
}


static void MDrv_JPEG_write_symidx(void)
{
    U16 i, tbl_num_luma, tbl_num_chroma;
    U8 ci, luma_ci = 0, chroma_ci = 0;
    U16 regval;

    for(ci = 0; ci<_u8Comps_in_frame; ci++)
    {
        if(_u8LumaCi==_u8Comp_ident[ci])
        {
            luma_ci = ci;
            break;
        }
    }

    for(ci = 0; ci<_u8Comps_in_frame; ci++)
    {
        if(_u8ChromaCi==_u8Comp_ident[ci])
        {
            chroma_ci = ci;
            break;
        }
    }

    tbl_num_luma = _u8Comp_ac_tab[luma_ci];
    tbl_num_chroma = _u8Comp_ac_tab[chroma_ci];

    JPD_REG(REG_JPD_TID_ADR) = JPD_MEM_SYMIDX_BASE;
    //MDrv_Write2Byte(REG_JPD_TID_ADR, JPD_MEM_SYMIDX_BASE);
    for ( i = 0; i < 256; i++ )
    {
        //JPEG_PRINT("_Huff_info[%d].u8Huff_val[%d]= %X\n", tbl_num_chroma, i, _Huff_info[tbl_num_chroma].u8Huff_val[i]);
        //JPEG_PRINT("_Huff_info[%d].u8Huff_val[%d]= %X\n", tbl_num_luma, i, _Huff_info[tbl_num_luma].u8Huff_val[i]);
        //JPEG_PRINT("regval= %X\n", ((U16)( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] )));

        regval = ((U16)( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] ));
        //JPEG_PRINT("regval= %X\n", regval);

        JPD_REG(REG_JPD_TID_DAT) = regval;
        //JPD_REG(REG_JPD_TID_DAT) = ((U16)( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] ));
        //MDrv_Write2Byte( REG_JPD_TID_DAT, ( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] ) );
    }


    tbl_num_luma = _u8Comp_dc_tab[luma_ci];
    tbl_num_chroma = _u8Comp_dc_tab[chroma_ci];

    for ( i = 0; i < 16; i++ )
    {
        //JPEG_PRINT("_Huff_info[%d].u8Huff_val[%d]= %X\n", tbl_num_chroma, i, _Huff_info[tbl_num_chroma].u8Huff_val[i]);
        //JPEG_PRINT("_Huff_info[%d].u8Huff_val[%d]= %X\n", tbl_num_luma, i, _Huff_info[tbl_num_luma].u8Huff_val[i]);
        //JPEG_PRINT("regvalu= %X\n", ((U16)( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] )));

        regval = ((U16)( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] ));
        //JPEG_PRINT("regval= %X\n", regval);

        JPD_REG(REG_JPD_TID_DAT) = regval;
        //MDrv_Write2Byte( REG_JPD_TID_DAT, ( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] ) );
    }
}


// This table is only used in JPD
static const U8 _u8Jpeg_zigzag_order[64] =
{
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

static void MDrv_JPEG_write_Qtbl(void)
{
    U8 i, j;
    U8 com_num = 0;
    U8 comp[JPEG_MAXCOMPONENTS];

    // Calculate how many valid quantization tables
    memset(comp, 0, JPEG_MAXCOMPONENTS);
    for(i = 0; i<_u8Comps_in_frame; i++)
    {
        comp[_u8Comp_quant[i]] = 1;
    }

    for(i = 0; i<JPEG_MAXCOMPONENTS; i++)
    {
        if(comp[i]==1)
            com_num++;
    }

    JPD_REG(REG_JPD_TID_ADR) = JPD_MEM_QTBL_BASE;
    //MDrv_Write2Byte(REG_JPD_TID_ADR, JPD_MEM_QTBL_BASE);
    for ( i = 0; i < com_num; i++ )
    {
        for(j = 0; j<64; j++)
        {
            JPD_REG(REG_JPD_TID_DAT) = _QuantTables[_u8Comp_quant[i]].s16Value[_u8Jpeg_zigzag_order[j]];
            //MDrv_Write2Byte(REG_JPD_TID_DAT, _QuantTables[_u8Comp_quant[i]].s16Value[_u8Jpeg_zigzag_order[j]]);
            //JPEG_PRINT("_QuantTables[%d].s16Value[%d]= %X\n", _u8Comp_quant[i], _u8Jpeg_zigzag_order[j], _QuantTables[_u8Comp_quant[i]].s16Value[_u8Jpeg_zigzag_order[j]]);
        }
    }
    // if all compoents refer to the same Qtable, need to write Qtable twice
    if(com_num==1)
    {
        for ( i = 0; i < com_num; i++ )
        {
            for(j = 0; j<64; j++)
            {
                JPD_REG(REG_JPD_TID_DAT) = _QuantTables[_u8Comp_quant[i]].s16Value[_u8Jpeg_zigzag_order[j]];
                //MDrv_Write2Byte(BK_JPD_TID_DAT, _QuantTables[_u8Comp_quant[i]].s16Value[_u8Jpeg_zigzag_order[j]]);
            }
        }
    }
//    JPD_REG(REG_JPD_TID_ADR) = JPD_MEM_QTBL_BASE;
//    for( j=0; j<64; j++ )
//    {
//        JPEG_PRINT("QT[%d]= %X\n", j, JPD_REG(REG_JPD_TID_DAT));
//    }

}


static void MHal_JPD_SetROI(U16 start_x, U16 start_y, U16 width, U16 height)
{
    JPD_REG(REG_JPD_ROI_H) = start_x;
    JPD_REG(REG_JPD_ROI_V) = start_y;
    JPD_REG(REG_JPD_ROI_WIDTH) = width;
    JPD_REG(REG_JPD_ROI_HEIGHT) = height;

#if 0
    U16 reg_val;

    reg_val = MDrv_Read2Byte(BK_JPD_SCONFIG);

    MDrv_Write2Byte(BK_JPD_ROI_H, start_x);
    MDrv_Write2Byte(BK_JPD_ROI_V, start_y);
    MDrv_Write2Byte(BK_JPD_ROI_WIDTH, width );
    MDrv_Write2Byte(BK_JPD_ROI_HEIGHT, height );
    //MDrv_Write2Byte(BK_JPD_SCONFIG, reg_val | JPD_ROI_EN);
#endif
}


static void G_MDrv_JPEG_StartDecode(void)
{
    U8 mcu_width, mcu_height;
    U16 pic_width = _u16Image_x_size;
    U16 pic_height = _u16Image_y_size;
    U8 Y_VSF = _u8Comp_v_samp[0];
    U8 Y_HSF = _u8Comp_h_samp[0];
    U32 bUV_en;
    U32 reg_value;
    U8 i,RatioVal, Factor = 1;
    U8 com_num = 0;
    U8 comp[JPEG_MAXCOMPONENTS];

    JPEG_PRINT("Start JPD decode\n");

//    if ( setjmp( _jmp_state ) )
//    {
//        return;
//    }

    // reset JPD hardware
    MHal_JPD_Reset();

    // Calculate how many valid quantization tables for components
    memset(comp, 0, JPEG_MAXCOMPONENTS);
    for(i = 0; i<_u8Comps_in_frame; i++)
    {
        comp[_u8Comp_quant[i]] = 1;
    }

    for(i = 0; i<JPEG_MAXCOMPONENTS; i++)
    {
        if(comp[i]==1)
            com_num++;
    }

    if(_u8Comps_in_frame>1)
        bUV_en = 1;
    else
        bUV_en = 0;

    if ( ( mcu_width = pic_width % ( Y_HSF * 8 ) ) )
    {
        pic_width += ( Y_HSF * 8 - mcu_width );
    }

    if ( ( mcu_height = pic_height% ( Y_VSF * 8 ) ) )
    {
        pic_height += ( Y_VSF * 8 - mcu_height );
    }

    if(_bThumbnailAccessMode)
    {
        // Set MRC buffer for JPD
        MHal_JPD_SetReadBuffer(INTERNAL_BUFFER_ADDR, JPEG_DEFAULT_EXIF_SIZE);
        // Set MRC start access byte address
        MHal_JPD_SetMRCStartAddr(INTERNAL_BUFFER_ADDR + _u32ThumbnailOffset + G_MDrv_JPEG_GetECS());
    }
    else
    {
        if(_bProgressive_flag)
        {
            // Set MRC buffer for JPD
            MHal_JPD_SetReadBuffer(MRC_BUFFER_ADDR, MRC_BUFFER_SIZE);
            // Set MRC start access byte address
            MHal_JPD_SetMRCStartAddr(MRC_BUFFER_ADDR);
        }
        else
        {
            // Set MRC buffer for JPD
            MHal_JPD_SetReadBuffer(MRC_BUFFER_ADDR, MRC_BUFFER_SIZE);
            // Set MRC start access byte address
            //MHal_JPD_SetMRCStartAddr(MRC_BUFFER_ADDR + MDrv_JPEG_GetECS());
            JPEG_PRINT("_pu8In_buf_ofs=0x%X\n", (U32)_pu8In_buf_ofs);
            MHal_JPD_SetMRCStartAddr( (U32)_pu8In_buf_ofs );
        }
    }

    // Set MWC buffer for JPD
    MHal_JPD_SetOutputFrameBuffer(MWC_BUFFER_ADDR);
    // Set picture width and height
    MHal_JPD_SetPicDimension(pic_width, pic_height);

    _u16Image_x_size = pic_width;
    _u16Image_y_size = pic_height;

    if(_bProgressive_flag)
    {
        MAX_JPEG_WIDTH_HD = MAX_JPEG_PROGRESSIVE_WIDTH;
        MAX_JPEG_HEIGHT_HD = MAX_JPEG_PROGRESSIVE_HEIGHT;
    }
    else
    {
        MAX_JPEG_WIDTH_HD = MAX_JPEG_BASELINE_WIDTH;
        MAX_JPEG_HEIGHT_HD = MAX_JPEG_BASELINE_HEIGHT;
    }

    JPEG_PRINT("Width = %d and Height = %d\n",pic_width,pic_height);
    JPEG_PRINT("Max Width = %d and Max Height = %d\n", MAX_JPEG_WIDTH_HD,MAX_JPEG_HEIGHT_HD);

    //Set the Scale down variable
    if(pic_width > MAX_JPEG_WIDTH_HD*4 || pic_height>MAX_JPEG_HEIGHT_HD*4)
    {
        RatioVal = RATIO_EIGHTH;
        _u8DownScaleRatio = JPD_DOWNSCALE_EIGHTH;
        Factor = 8;
        _u16Image_y_size = pic_height/Factor;
        JPEG_PRINT("down scale 1/8!!\n");
    }
    else if(pic_width > MAX_JPEG_WIDTH_HD*2 || pic_height>MAX_JPEG_HEIGHT_HD*2)
    {
        RatioVal = RATIO_FOURTH;
        _u8DownScaleRatio = JPD_DOWNSCALE_FOURTH;
        Factor = 4;
        _u16Image_y_size = pic_height/Factor;
        JPEG_PRINT("down scale 1/4!!\n");
    }
    else if (pic_width > MAX_JPEG_WIDTH_HD || pic_height>MAX_JPEG_HEIGHT_HD)
    {
        RatioVal = RATIO_HALF;
        _u8DownScaleRatio = JPD_DOWNSCALE_HALF;
        Factor = 2;
        _u16Image_x_size = pic_width/Factor;
        _u16Image_y_size = pic_height/Factor;
        JPEG_PRINT("down scale 1/2!!\n");
    }
    else
    {
        JPEG_PRINT("down scale 1/1!!\n");
        RatioVal = RATIO_ORIGINAL;
        _u8DownScaleRatio = JPD_DOWNSCALE_ORG;
        Factor = 1;
    }


    // In JPD, software VLD mode, we don't need to write huff & symbol tables
    if(_bProgressive_flag==0)
    {
        MDrv_JPEG_write_Scwgif();
        MDrv_JPEG_write_symidx();
    }


    MDrv_JPEG_write_Qtbl();

    //JPEG_DEBUG("After QTBL\n");

    //MDrv_JPEG_DumpTables();

    Y_VSF -= 1;
    if ( Y_HSF == 4 )
    {
        Y_HSF = 3;
    }

    if(_u16Restart_interval)
    {
        JPD_REG(REG_JPD_RSTINTV) =  _u16Restart_interval - 1;
        //MDrv_Write2Byte(REG_JPD_RSTINTV, _u16Restart_interval - 1);
        reg_value = ( JPD_TBL_RDY | JPD_RST_EN | ((U32) _u8DownScaleRatio) << 4 | ((U32) bUV_en) << 3 | ( Y_VSF << 2 ) | Y_HSF );
    }
    else
    {
        reg_value = ( JPD_TBL_RDY | ((U32) _u8DownScaleRatio) << 4 | ((U32) bUV_en) << 3 | ( Y_VSF << 2 ) | Y_HSF );
    }

    // There're Q tables for U & V, respectively.
    if(com_num>2)
    {
        reg_value = reg_value | JPD_SUVQ;
    }
    /*if(_bProgressive_flag==FALSE)
    {
        // pre-load buffer
        if ( MDrv_JPEG_GetECS()>= ( MRC_BUFFER_SIZE / 2 ) )
        {
            MDrv_JPEG_PreLoadBuffer(JPEG_BUFFER_LOW);
        }
        JPEG_PRINT("Come in Pre-Load Buffer!!\n");
    }*/

    if(_bProgressive_flag)
        reg_value = reg_value | JPD_SVLD;

    // Check if it needs to do ROI
    if(_u8DownScaleRatio==JPD_DOWNSCALE_HALF||_u8DownScaleRatio==JPD_DOWNSCALE_FOURTH||_u8DownScaleRatio==JPD_DOWNSCALE_EIGHTH)
    {
        U16 ROI_width;

        if(_u8DownScaleRatio==JPD_DOWNSCALE_HALF)
        {
            // width must be multiple of 32

            ROI_width = (pic_width/16)*16;


            if(ROI_width!=pic_width)
            {
                //JPEG_PRINT("ROI!!\n");
                MHal_JPD_SetROI(0, 0, (ROI_width>>3), (pic_height>>3));

                reg_value = reg_value | JPD_ROI_EN;
            }

            _u16Image_x_size = ROI_width/Factor;
        }
        else if(_u8DownScaleRatio==JPD_DOWNSCALE_FOURTH)
        {
            // width must be multiple of 32

            ROI_width = (pic_width/32)*32;


            if(ROI_width!=pic_width)
            {
                //JPEG_PRINT("ROI!!\n");
                MHal_JPD_SetROI(0, 0, (ROI_width>>3), (pic_height>>3));

                reg_value = reg_value | JPD_ROI_EN;
            }

            _u16Image_x_size = ROI_width/Factor;
        }
        else if(_u8DownScaleRatio==JPD_DOWNSCALE_EIGHTH)
        {
            // width must be multiple of 64
            ROI_width = (pic_width/64)*64;
           // JPEG_PRINT("down scale 1/8!!\n");
            if(ROI_width!=pic_width)
            {
                //JPEG_PRINT("ROI!!\n");
                MHal_JPD_SetROI(0, 0, ROI_width>>3, pic_height>>3);

                reg_value = reg_value | JPD_ROI_EN;
            }
            _u16Image_x_size = ROI_width/Factor;
        }
    }

    if( ((U32)_pu8In_buf_ofs)  > (MRC_BUFFER_ADDR +MRC_BUFFER_SIZE/2) && _u8FileReadEnd_flag==0 )
    {
        volatile  U8* tmp;
        tmp = (volatile  U8*)(MRC_BUFFER_ADDR);
        tmp[0]=0x55;
        tmp[1]=0xAA;
        tmp[2]=0xAA;
        tmp[3]=0x54;
        //MDrv_JPEG_Wait();
    }

    // <CK>
    // mark low/high buffer valid
    #if 1
    JPD_REG(REG_JPD_MCONFIG) = JPD_H_VLD |JPD_L_VLD | (1<<5); // add UV Swap
    #else
    JPD_REG(REG_JPD_MCONFIG) = JPD_H_VLD |JPD_L_VLD; // no UV Swap
    #endif
    //MDrv_Write2Byte( REG_JPD_MCONFIG, JPD_H_VLD |JPD_L_VLD);

    // enable JPD decoding
    JPD_REG(REG_JPD_SCONFIG) = reg_value | JPD_DEC_EN | JPD_SWRST_S4L;
    //MDrv_Write2Byte( REG_JPD_SCONFIG, reg_value | JPD_DEC_EN | JPD_SWRST_S4L);

    //JPEG_DEBUG("After SCONFIG START\n");
}


static U16 MHal_JPD_ReadJPDStatus(void)
{
    return JPD_REG(REG_JPD_EVENTFLAG);

    //return MDrv_Read2Byte((U16)BK_JPD_EVENTFLAG);
}

static U32 G_MDrv_JPEG_write_RLE(PSVLD pVld, U32 bDecodeNow)
{
    U8 *mrc_buffer = (U8 *) MRC_BUFFER_ADDR;
    U16 status;
    //U32 start_time;
    U16 cur_vIdx;
    U32 reg_value;
    U16 u16Count = 2000;

    memcpy(mrc_buffer + _u32RLEOffset, pVld, 4);
    _u32RLEOffset += 4;

    // Check if buffer full
    if(_u32RLEOffset==MRC_BUFFER_SIZE || bDecodeNow==1)
    {
        //JPEG_DEBUG("Do RLE, LENG %d\n", _u32RLEOffset);

        if(_bFirstRLE==1)
        {
            // clear MRC low/high portion read complete event
            MHal_JPD_ClearJPDStatus(JPD_EVENT_MRBH_DONE | JPD_EVENT_MRBL_DONE);
            // mark low/high buffer valid
            reg_value = (JPD_REG(REG_JPD_MCONFIG) & ~0x0003) | (JPD_H_VLD |JPD_L_VLD);
            JPD_REG(REG_JPD_MCONFIG) = reg_value;
            //MDrv_Write2Byte( REG_JPD_MCONFIG, (MDrv_Read2Byte(REG_JPD_MCONFIG) & ~0x0003) | (JPD_H_VLD |JPD_L_VLD));
        }

        // Trigger JPD decoding
        if(_bFirstRLE==0)
        {
            G_MDrv_JPEG_StartDecode();
            _bFirstRLE = 1;
        }

        //sharon start_time = MsOS_GetSystemTime();
        cur_vIdx = JPD_REG(REG_JPD_CUR_VIDX);

        JPEG_PRINT("cur_vIdx = %x\n",cur_vIdx);

        if( bDecodeNow )
        {
            return 1;  //wait done in main loop
        }

        while( u16Count > 0 )
        {
            status = MHal_JPD_ReadJPDStatus();

            if(status & JPD_EVENT_DEC_DONE)
            {
                JPEG_PRINT("P deocde done\n");
                break;
            }

            if((status & JPD_EVENT_ECS_ERROR) || (status & JPD_EVENT_IS_ERROR) || (status & JPD_EVENT_RST_ERROR))
            {
                 // temp patch for protect JPD from writing to illegal memory
                //MHal_JPD_SW_Pause_Reset();
                //MDrv_JPEG_terminate( JPEG_SVLD_DECODE_ERROR );
                return 0;
            }

            if((status & JPD_EVENT_MRBH_DONE) && (status & JPD_EVENT_MRBL_DONE))
            {
                JPEG_PRINT("Partial SVLD decode done\n");
                break;
            }

            // Check the V index. If it is not changed withing 500ms, it means that the JPD has some problem.
            // We need to break the infinite loop
            if(cur_vIdx != JPD_REG(REG_JPD_CUR_VIDX))
            {
                //sharon  start_time = MsOS_GetSystemTime();
                cur_vIdx = JPD_REG(REG_JPD_CUR_VIDX);
            }
            else
            {
                if(0)  //sharon ((MsOS_GetSystemTime() - start_time)>=DEFAULT_DECODE_TIMEOUT)
                {
                    JPEG_PRINT("ERROR: SVLD deocde time out, VIdx %d\n", cur_vIdx);
                    break;
                }
            }
            mstar_msleep(1);
            if((u16Count--) == 0)
            {
                JPEG_PRINT("MDrv_JPEG_write_RLE Timeout\n");
                return 1;
            }
        }

        _u32RLEOffset = 0;
    }
    return 1;
}

static void G_MDrv_JPEG_do_RLE(BLOCK_TYPE *p, BOOL eop, U8 comp_id)
{
    SVLD my_vld;
    U8 counter;
    S16 value;
    U16 run;
    U8 cur_blk;
    BLOCK_TYPE predictor;

    if(comp_id==0)
        cur_blk = 1;     // Y
    else if(comp_id==1)
        cur_blk = 3;    // U
    else
        cur_blk = 2;    // V

    predictor = _s16dc_pred[cur_blk - 1];

    run = 0;
    my_vld.byte0 = my_vld.byte1 = my_vld.byte2 = my_vld.byte3 = 0;
    my_vld.blk_type = cur_blk;

    //sent DC info
    my_vld.run = 8;
    value = (p[0] - predictor);
    my_vld.sign = (value<0)?1:0;
    my_vld.amp = ABS(value);
    my_vld.sym_type = RLE_DC;
    G_MDrv_JPEG_write_RLE(&my_vld, 0);

    my_vld.byte0 = my_vld.byte1 =my_vld.byte2 = my_vld.byte3= 0;
    my_vld.blk_type = cur_blk;

    for(counter = 1;counter<64; counter++)
    {
        if(p[counter]==0)
        {
            run++;
        }
        else
        {
            while(run>15)
            {
                my_vld.sign = 0;
                my_vld.amp = 0;
                my_vld.sym_type = RLE_ZRL;
                my_vld.run = 15;
                G_MDrv_JPEG_write_RLE(&my_vld, 0);

                my_vld.byte0 = my_vld.byte1 = my_vld.byte2 = my_vld.byte3 = 0;
                my_vld.blk_type = cur_blk;
                run -= 16;
            }

            my_vld.sign = (p[counter]<0)?1:0;
            my_vld.amp = ABS(p[counter]);
            my_vld.sym_type = RLE_AC;
            my_vld.run = run;

            // Check if the last byte is non-zero. If it's non-zero & EOP, add the EOP flag
            if(counter==63&&eop&&p[63]!=0)
            {
                my_vld.EOP = 1;
                G_MDrv_JPEG_write_RLE(&my_vld, 1);

                _s16dc_pred[cur_blk - 1] = p[0];//update predictor
                return;
            }
            else
                G_MDrv_JPEG_write_RLE(&my_vld, 0);

            my_vld.byte0 = my_vld.byte1 = my_vld.byte2 = my_vld.byte3 = 0;
            my_vld.blk_type = cur_blk;
            run = 0;
        }
    }

    counter = 63;

    if(p[counter]==0)
    {
        my_vld.amp = ABS(p[counter]);
        my_vld.sign = p[counter]<0?1:0;
        my_vld.sym_type = RLE_EOB;
        my_vld.run = 0;
        if(eop)
        {
            my_vld.EOP =1;
            G_MDrv_JPEG_write_RLE(&my_vld, 1);
        }
        else
            G_MDrv_JPEG_write_RLE(&my_vld, 0);
    }

    _s16dc_pred[cur_blk - 1] = p[0];//update predictor
}


static void G_MDrv_JPEG_load_next_row( void )
{
    BLOCK_TYPE p[64];

    U16 mcu_row, mcu_block;
    U8 component_num, component_id;
    U16 block_x_mcu[JPEG_MAXCOMPONENTS];

    memset( block_x_mcu, 0, JPEG_MAXCOMPONENTS * sizeof( U16 ) );

    for ( mcu_row = 0; mcu_row < _u16Mcus_per_row; mcu_row++ )
    {
        U16 block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;

        for ( mcu_block = 0; mcu_block < _u8Blocks_per_mcu; mcu_block++ )
        {
            BLOCK_TYPE *pAC;
            BLOCK_TYPE *pDC;

            component_id = _u8Mcu_org[mcu_block];

            pAC = MDrv_JPEG_coeff_buf_getp( _AC_Coeffs[component_id], block_x_mcu[component_id] + block_x_mcu_ofs, _u32Block_y_mcu[component_id] + block_y_mcu_ofs );
            if(pAC == NULL)
            {
                //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
                continue;
            }
            pDC = MDrv_JPEG_coeff_buf_getp( _DC_Coeffs[component_id], block_x_mcu[component_id] + block_x_mcu_ofs, _u32Block_y_mcu[component_id] + block_y_mcu_ofs );
            if(pDC == NULL)
            {
                //JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
                continue;
            }
            p[0] = pDC[0];
            memcpy( &p[1], &pAC[1], 63 * sizeof( BLOCK_TYPE ) );

            if(mcu_block==(_u8Blocks_per_mcu -1)&&mcu_row==(_u16Mcus_per_row - 1)&&(_u16Total_lines_left - gu8Max_mcu_y_size)==0)
            {
                G_MDrv_JPEG_do_RLE(p, 1, component_id);   // means it is end of picture
            }
            else
            {
                G_MDrv_JPEG_do_RLE(p, 0, component_id);
            }

            if ( _u8Comps_in_scan == 1 )
            {
                block_x_mcu[component_id]++;
            }
            else
            {
                if ( ++block_x_mcu_ofs == _u8Comp_h_samp[component_id] )
                {
                    block_x_mcu_ofs = 0;

                    if ( ++block_y_mcu_ofs == _u8Comp_v_samp[component_id] )
                    {
                        block_y_mcu_ofs = 0;

                        block_x_mcu[component_id] += _u8Comp_h_samp[component_id];
                    }
                }
            }
        }
    }

    if ( _u8Comps_in_scan == 1 )
    {
        _u32Block_y_mcu[_u8Comp_list[0]]++;
    }
    else
    {
        for ( component_num = 0; component_num < _u8Comps_in_scan; component_num++ )
        {
            component_id = _u8Comp_list[component_num];

            _u32Block_y_mcu[component_id] += _u8Comp_v_samp[component_id];
        }
    }
}

S16 G_MDrv_JPEG_Progressive_Decode(void)
{
    if( _bProgressive_flag==0 )
        return (JPEG_FAILED);

    if( _u16Total_lines_left == 0 )
    {
        return ( JPEG_DONE );
    }

   while( _u16Total_lines_left > 0 )
    {
        G_MDrv_JPEG_load_next_row();
        _u16Total_lines_left -= gu8Max_mcu_y_size;
    }

    return (JPEG_OKAY);

}


static EN_JPD_STATUS MDrv_JPEG_WaitDone(void)
{
    U16 reg_val;

    reg_val = MHal_JPD_ReadJPDStatus();
    JPEG_PRINT("JPD Status= %X\n", reg_val);

    if(reg_val & JPD_EVENT_DEC_DONE)
    {
        JPEG_PRINT("Decode Done\n");
        G_MDrv_JPEG_Finalize();
        //_Pinput_stream->destructor( _Pinput_stream );
        //g_state = e_DecodeDone;

        return EN_JPD_DECODE_DONE;
    }
    else if(reg_val & JPD_EVENT_ERROR_MASK)
    {
        // temp patch for protect JPD from writing to illegal memory
        //MHal_JPD_SW_Pause_Reset();

        JPEG_PRINT("Baseline decode error: %x\n", reg_val);
        G_MDrv_JPEG_Finalize();
        //_Pinput_stream->destructor( _Pinput_stream );
        //g_state = e_DecodeErr;
        return EN_JPD_DECODE_ERROR;
    }

    else if(reg_val & JPD_EVENT_MRBL_DONE)
    {
        JPEG_PRINT("JPD_EVENT_MRBL_DONE\n");
        // decode of low portion buffer is done, pre-load buffer to low buffer address
        return EN_JPD_MRBFL_DONE;
/*
        if(MDrv_JPEG_PreLoadBuffer(JPEG_BUFFER_LOW)==FALSE)
        {
            MDrv_JPEG_Finalize();
            return true;
            //_Pinput_stream->destructor( _Pinput_stream );

            //if(MDrv_JPEG_get_error_code()==JPEG_STREAM_READ)
            //    g_state = e_Idle;
            //else
            //    g_state = e_DecodeErr;
        }
*/
    }
    else if(reg_val & JPD_EVENT_MRBH_DONE)
    {
        JPEG_PRINT("JPD_EVENT_MRBH_DONE\n");
        // decode of high portion buffer is done, pre-load buffer to high buffer address

        return EN_JPD_MRBFH_DONE;
/*
        if(MDrv_JPEG_PreLoadBuffer(JPEG_BUFFER_HIGH)==FALSE)
        {
            MDrv_JPEG_Finalize();
            return true;
            //_Pinput_stream->destructor( _Pinput_stream );

            //if(MDrv_JPEG_get_error_code()==JPEG_STREAM_READ)
            //    g_state = e_Idle;
            //else
            //    g_state = e_DecodeErr;
        }
*/
    }
    else
    {
        return EN_JPD_DECODING;
    }
}


static U16 MDrv_JPEG_get_width( void )
{
    return ( _u16Image_x_size );
}


static U16 MDrv_JPEG_get_height( void )
{
    return ( _u16Image_y_size );
}


static U16 MDrv_JPEG_get_original_width( void )
{
    return ( _u16OriginalImage_x_size );
}


static U16 MDrv_JPEG_get_original_height( void )
{
    return ( _u16OriginalImage_y_size );
}

static void G_JPEGWaitDone(JPEG_PIC_Info *pInfo)
{
    U32 u32Count = 0xFFFFFFFF;

    while( u32Count > 0 )
    {
        pInfo->enJPDStatus = MDrv_JPEG_WaitDone();
        switch( pInfo->enJPDStatus )
        {
        case EN_JPD_DECODE_DONE:
            pInfo->u32Width = MDrv_JPEG_get_width();
            pInfo->u32Height = MDrv_JPEG_get_height();
            pInfo->u32OriginalWidth= MDrv_JPEG_get_original_width();
            pInfo->u32OriginalHeight = MDrv_JPEG_get_original_height();
            return;

        case EN_JPD_MRBFL_DONE:
        case EN_JPD_MRBFH_DONE:
        case EN_JPD_DECODE_ERROR:
            pInfo->u32Width = 0;
            pInfo->u32Height = 0;
            return;

        case EN_JPD_DECODING:
        default:
            break;
        }
        mstar_msleep(1);
        if((u32Count--) == 0)
        {
            JPEG_PRINT("JPEGWaitDone Timeout\n");
            return;
        }
    }
}

//static int DecodeJPEG( void ){
static int DecodeJPEG(U32 u32InAddr, U32 u32OutAddr, U8 u8JPG_Idx)
{

    JPEG_PIC_Info stPicInfo={0};

    JPEG_PRINT( "\n\n\n-------------------------Start to DecodeJPEG-------------------------\n\n\n" ) ;

#if TIME_TEST
    U32 ttt ;
    stimer() ;
#endif

    G_MDrv_JPEG_SetInitParameter(
        //0xA2000000, //param.u32ReadBufferAddr,
        u32InAddr,
        0x00200000, //param.u32ReadBufferSize,  // 2MB
        //GOP_BUFFER|0xA0000000, //param.u32WriteBufferAddr,
        u32OutAddr,
        u32InAddr+0x00200000, //param.u32InterBufferAddr,
        0x00400000, //param.u32InterBufferSize, // 4MB
        1 //param.u8FileReadEnd
        ) ;

#if TIME_TEST
    ttt = stimer() ;
    printk( "\n\n\nJPEG 0 Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif

#define REG_CLKGEN0_BASE                0xBF201600
#define TOP_REG(addr)               (*((volatile U32*)(REG_CLKGEN0_BASE + ((addr)<<2))))
    //MHal_JPD_SetClock(U8 on)
    TOP_REG(0x0035) = 0x08;


    if( !G_MDrv_JPEG_constructor(JPEG_TYPE_MAIN) )
    {
        JPEG_PRINT("MDrv_JPEG_constructor Error.\n");
        return -1;
    }

#if TIME_TEST
    ttt = stimer() ;
    printk( "\n\n\nJPEG 1 Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif

    if(  !G_MDrv_JPEG_DecodeHeader() )
    {
        JPEG_PRINT("MDrv_JPEG_DecodeHeader Error.\n");
        return -1;
    }
    //JPEG_PRINT("%dx%d\n", MDrv_JPEG_get_original_width(), MDrv_JPEG_get_original_height());

#if TIME_TEST
    ttt = stimer() ;
    printk( "\n\n\nJPEG 2 Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif

    if(G_MDrv_JPEG_IsProgressive())
    {
        JPEG_PRINT("Progressive\n");
        G_MDrv_JPEG_Progressive_Decode();
#if TIME_TEST
        ttt = stimer() ;
        printk( "\n\n\nJPEG 3 Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif
    }
    else
    {
        JPEG_PRINT("Baseline\n");
        G_MDrv_JPEG_StartDecode();
#if TIME_TEST
        ttt = stimer() ;
        printk( "\n\n\nJPEG 4 Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif
    }

    G_JPEGWaitDone( &stPicInfo );
#if TIME_TEST
        ttt = stimer() ;
        printk( "\n\n\nJPEG 5 Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif
    JPEG_PRINT("Image: W= %d  H= %d\n",stPicInfo.u32Width, stPicInfo.u32Height);

    jpegWidth[u8JPG_Idx] = stPicInfo.u32Width ;
    jpegHeight[u8JPG_Idx] = stPicInfo.u32Height ;

    // Copy the JPEG output buffer to MIU1 by the request of Mr. Pyun.
    //memcpy(u32OutAddr, u32OutAddr+0x8000000, sizeof(stPicInfo.u32Width*stPicInfo.u32Height*2));

    JPEG_PRINT( "\n\n\n-------------------------End of DecodeJPEG-------------------------\n\n\n" ) ;
    return 1;
}

//
// GE setting
//-----------------------------------------------------------------
#define GE_ADRESS                       (0x1400*4)
#define REG_GE_BASE                     (GE_ADRESS)

#define REG_GE_EN                       (0x00*4 + (REG_GE_BASE))

#define REG_GE_CFG                      (0x01*4 + (REG_GE_BASE))

#define REG_GE_STATUS                   (0x07*4 + (REG_GE_BASE))

#define REG_GE_ALPHA_CONST              (0x13*4 + (REG_GE_BASE))

#define REG_GE_SRC_BASE_L               (0x20*4 + (REG_GE_BASE))
#define REG_GE_SRC_BASE_H               (0x21*4 + (REG_GE_BASE))
#define REG_GE_DST_BASE_L               (0x26*4 + (REG_GE_BASE))
#define REG_GE_DST_BASE_H               (0x27*4 + (REG_GE_BASE))

#define REG_GE_SRC_PITCH                (0x30*4 + (REG_GE_BASE))
#define REG_GE_DST_PITCH                (0x33*4 + (REG_GE_BASE))
#define REG_GE_FMT                      (0x34*4 + (REG_GE_BASE))

#define REG_GE_CLIP_L                   (0x55*4 + (REG_GE_BASE))
#define REG_GE_CLIP_R                   (0x56*4 + (REG_GE_BASE))
#define REG_GE_CLIP_T                   (0x57*4 + (REG_GE_BASE))
#define REG_GE_CLIP_B                   (0x58*4 + (REG_GE_BASE))

#define REG_GE_ROT                      (0x59*4 + (REG_GE_BASE))
#define REG_GE_BLT_DST_X_OFST           (0x5E*4 + (REG_GE_BASE))
#define REG_GE_BLT_DST_Y_OFST           (0x5F*4 + (REG_GE_BASE))

#define REG_GE_CMD                      (0x60*4 + (REG_GE_BASE))

#define REG_GE_BLT_SRC_DX               (0x64*4 + (REG_GE_BASE))
#define REG_GE_BLT_SRC_DY               (0x65*4 + (REG_GE_BASE))
#define REG_GE_PRIM_V0_X                (0x68*4 + (REG_GE_BASE))
#define REG_GE_PRIM_V0_Y                (0x69*4 + (REG_GE_BASE))
#define REG_GE_PRIM_V1_X                (0x6A*4 + (REG_GE_BASE))
#define REG_GE_PRIM_V1_Y                (0x6B*4 + (REG_GE_BASE))
#define REG_GE_PRIM_V2_X                (0x6C*4 + (REG_GE_BASE))
#define REG_GE_PRIM_V2_Y                (0x6D*4 + (REG_GE_BASE))
#define REG_GE_BLT_SRC_W                (0x6E*4 + (REG_GE_BASE))
#define REG_GE_BLT_SRC_H                (0x6F*4 + (REG_GE_BASE))

#define REG_GE_PRIM_C_L                 (0x70*4 + (REG_GE_BASE))
#define REG_GE_PRIM_C_H                 (0x71*4 + (REG_GE_BASE))

//
// GOP setting
//-----------------------------------------------------------------
#define GOP_ADRESS                      (0xF80*4)
#define REG_GOP_BASE                    (GOP_ADRESS)

#define REG_GOP_SOFT_RESET              (0x00*4 + (REG_GOP_BASE))
#define REG_GOP_DEST                    (0x01*4 + (REG_GOP_BASE))
#define REG_GOP_BLINK                   (0x02*4 + (REG_GOP_BASE))
#define REG_GOP_RDMA_HT                 (0x0E*4 + (REG_GOP_BASE))
#define REG_GOP_HSYNC_PIPE_DELAY        (0x0F*4 + (REG_GOP_BASE))
#define REG_GOP_GWIN_PRI                (0x20*4 + (REG_GOP_BASE))
#define REG_GOP_WR_ACK                  (0x7F*4 + (REG_GOP_BASE))

#define REG_GOP_GWIN0_EN              (0x00*4 + (REG_GOP_BASE))
#define REG_GOP_GWIN0_VSTOP_LO        (0x14*4 + (REG_GOP_BASE))
#define REG_GOP_GWIN0_VSTOP_HI        (0x15*4 + (REG_GOP_BASE))
#define REG_GWIN_SCROLL_LEN           (0x12*4 + (REG_GOP_BASE))
#define REG_GOP_MUX_SEL                (0x7e*4 + (REG_GOP_BASE))

#define REG_GOP_STRCH_HSTART          (0x32*4 + (REG_GOP_BASE))
#define REG_GOP_STRCH_VSTART          (0x34*4 + (REG_GOP_BASE))
#define REG_GOP_STRCH_HSIZE           (0x30*4 + (REG_GOP_BASE))
#define REG_GOP_STRCH_VSIZE           (0x31*4 + (REG_GOP_BASE))
#define REG_GOP_TRS_CLR_LO            (0x24*4 + (REG_GOP_BASE))
#define REG_GOP_TRS_CLR_HI            (0x25*4 + (REG_GOP_BASE))

#define REG_GOP_GWIN0_VSTR              (0x06*4 + (REG_GOP_BASE))
#define REG_GOP_GWIN0_VEND              (0x08*4 + (REG_GOP_BASE))
#define REG_GOP_GWIN0_HSTR              (0x04*4 + (REG_GOP_BASE))
#define REG_GOP_GWIN0_HEND              (0x05*4 + (REG_GOP_BASE))
#define GWIN_SDRAM_NULL     0x30
#define GOP_ALIGNMENT       8  //8 byte(64bit) alignment.
#define GOP_ALIGN_RSHIFT    4  //Right shift 3 byte for HW alignment(64bit).
#define GOP_RESOURCE_CONTROL 1
#define GOP_ACK_COUNTER      15 //93000 // About 200Ms
#define GOP_OP_H_Offset      11//12

#define REG_GOP_DRAM_GWIN0_VSTR_LO      (0x0C*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_GWIN0_VSTR_HI      (0x0D*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_GWIN0_HSTR         (0x0E*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_RBLK0_STR_LO       (0x01*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_RBLK0_STR_HI       (0x02*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_RBLK0_HSIZE        (0x09*4 + (REG_GOP_BASE))
#define GOP_DRAM_RBLK_STR_HI_SHIFT      (0)
#define GOP_DRAM_RBLK_PAGE_SHIFT        (6)

typedef struct
{
    U32 DRAM_RBLK_STR_HI:   6;
    U32 DRAM_RBLK_PAGE:     2;
    U32 reserved1:          8;
    U32 reserved:           16;
} G_GOP_dram_rblk_str_hi;

#if 0
#define REG_GOP_DRAM_RBLK0_SIZE_LO       (0x60*4+0x10*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_RBLK0_SIZE_HI       (0x60*4+0x11*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_RBLK0_HSIZE        (0x60*4+0x09*4 + (REG_GOP_BASE))
#else
#define REG_GOP_DRAM_RBLK0_SIZE_LO       (0x10*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_RBLK0_SIZE_HI       (0x11*4 + (REG_GOP_BASE))
#define REG_GOP_DRAM_RBLK0_HSIZE         (0x09*4 + (REG_GOP_BASE))
#endif

typedef struct
{
    U32 GWIN_EN:            1; // 0:GOP0, 1:GOP1, 2:GOPD, 3:GOPS
    U32 GWIN_RSTOP:         1;
    U32 GWIN_VROLL:         1;
    U32 GWIN_HROLL:         1;
    U32 GWIN_DTYPE:         4;
    U32 GWIN_ALPHA:         6;
    U32 GWIN_ALPHA_EN:      1;
    U32 reserved:           17;
}G_GOP_gwin_set;


#define LO16BIT(x)  (x & 0x0000ffff)
#define HI16BIT(x)  (x >> 16)
#define _GFXOSD_CALC_STRIDE(bpl)		(((bpl)+15)&0xfffffff0)

#if 0
static void BurstInitGOP( void )
{


#if (PANEL_TYPE==1366)

    U32 u32DispX = 162 ;
    U32 u32DispY = 0 ;
    U32 u32Width = 1366 ;
    U32 u32Height =768 ;

#elif (PANEL_TYPE==1920)

    U32 u32DispX = 182 ;
    U32 u32DispY = 0 ;
    U32 u32Width = 1920 ;
    U32 u32Height =1080 ;

#endif

    U32 stride = _GFXOSD_CALC_STRIDE( (u32Width<<2) ) ;
    U32 u32GWinSrcX = 0 ;
    U32 u32GWinSrcY = 0 ;
    U32 u32GWinDispX = 0 ;
    U32 u32GWinDispY = 0 ;
    U32 u32DRAMRBlkStart = GOP_BUFFER ;
    U32 u32DRAMRBlkHSize = stride>>2 ;
    U32 u32DRAMRBlkVSize = u32Height ;
    U32 u32ColorDepth = 4 ;
    U32 u32GWINFormat = 5 ; // ARGB8888=5, YUV422=9

    // init for JPEG
    if( jpegWidth!=0 ){

        u32Width = jpegWidth ;
        u32Height = jpegHeight ;

        stride = _GFXOSD_CALC_STRIDE( (u32Width<<2) ) ;
        u32DRAMRBlkHSize = stride>>2 ;
        u32DRAMRBlkVSize = u32Height ;
        u32ColorDepth = 2 ; // 2 byte for YUV422
        u32GWINFormat = 9 ; // ARGB8888=5, YUV422=9
    }


#if TIME_TEST
    U32 ttt ;
    stimer() ;
#endif

    // _MHal_GOP_Init(eGOP_Type);
    SYS_REGOP( (0xF16<<2) ) &= ~0x1 ; // GOP4G ckg enable
    SYS_REGOP( REG_GOP_WR_ACK ) |= 0x200 ; // force write
    SYS_REGOP( REG_GOP_WR_ACK ) &= ~0xF ; // select GOP4G_0
    SYS_REGOP( REG_GOP_SOFT_RESET ) = 0x4209 ; // into reset state

    SYS_REGOP( REG_GOP_DEST ) &= 0xFFF0 ; // set DMA length and destination:OP
    SYS_REGOP( REG_GOP_DEST ) |= 0x000A ;
    SYS_REGOP(REG_GOP_BLINK)   = 0x1500 ;  //disable blink, set blink rate and scroll rate
    SYS_REGOP(REG_GOP_GWIN_PRI)= 0x3210 ; //GWIN Priority: GWIN0>GWIN1>GWIN2>GWIN3
    SYS_REGOP(REG_GOP_RDMA_HT) = 0x0000 ;

    SYS_REGOP( REG_GOP_WR_ACK ) |= 0x1 ; // select GOP4G_1

    // init GWIN 3
    SYS_REGOP(REG_GOP_GWIN0_EN) = 0x3F00;  // Init alpha value, disable H&V scroll, auto stop
    //initial Scroll auto stop offset
    SYS_REGOP(REG_GOP_GWIN0_VSTOP_LO) =0x0000;
    SYS_REGOP(REG_GOP_GWIN0_VSTOP_HI) =0x0000;
    //initial Scroll Setps
    SYS_REGOP(REG_GWIN_SCROLL_LEN)   =0x0000;

    SYS_REGOP(REG_GOP_MUX_SEL) &= ~0x003F; // set mux
    SYS_REGOP( REG_GOP_WR_ACK ) &= ~0xF ; // select GOP4G_0
    SYS_REGOP( REG_GOP_SOFT_RESET ) = 0x4208 ; // leave reset state

    SYS_REGOP(REG_GOP_STRCH_HSTART) = u32DispX;
    SYS_REGOP(REG_GOP_STRCH_VSTART) = u32DispY;
    SYS_REGOP(REG_GOP_STRCH_HSIZE) = u32Width>>1;
    SYS_REGOP(REG_GOP_STRCH_VSIZE) = u32Height;
    SYS_REGOP(REG_GOP_RDMA_HT) = (u32Width>>1) + 1;

    SYS_REGOP( REG_GOP_WR_ACK ) |= 0x1 ; // select GOP4G_1
    SYS_REGOP(REG_GOP_GWIN0_VSTR) = u32GWinDispY ; //in pixel
    SYS_REGOP(REG_GOP_GWIN0_VEND) = u32GWinDispY + u32Height;//in pixel
    SYS_REGOP(REG_GOP_GWIN0_HSTR) = (u32GWinDispX) * u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment
    SYS_REGOP(REG_GOP_GWIN0_HEND) = (u32GWinDispX + u32Width) * u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment

    //Offset to RBLK
    U32 u32RegVal ;
    u32RegVal = u32GWinSrcY * u32DRAMRBlkHSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(REG_GOP_DRAM_GWIN0_VSTR_LO) = LO16BIT(u32RegVal);
    SYS_REGOP(REG_GOP_DRAM_GWIN0_VSTR_HI) = HI16BIT(u32RegVal);
    u32RegVal = u32GWinSrcX * u32ColorDepth >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(REG_GOP_DRAM_GWIN0_HSTR) = LO16BIT(u32RegVal);

    //RBLK
    U32 u32Tmp ;
    u32Tmp = u32DRAMRBlkStart >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(REG_GOP_DRAM_RBLK0_STR_LO) = LO16BIT(u32Tmp);
    u32RegVal = SYS_REGOP(REG_GOP_DRAM_RBLK0_STR_HI);
    ((G_GOP_dram_rblk_str_hi*)&u32RegVal)->DRAM_RBLK_STR_HI = HI16BIT(u32Tmp) >> GOP_DRAM_RBLK_STR_HI_SHIFT;
    ((G_GOP_dram_rblk_str_hi*)&u32RegVal)->DRAM_RBLK_PAGE = HI16BIT(u32Tmp) >> GOP_DRAM_RBLK_PAGE_SHIFT;
    SYS_REGOP(REG_GOP_DRAM_RBLK0_STR_HI) = u32RegVal;

    u32RegVal = u32DRAMRBlkHSize * u32DRAMRBlkVSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(REG_GOP_DRAM_RBLK0_SIZE_LO) = LO16BIT(u32RegVal);
    SYS_REGOP(REG_GOP_DRAM_RBLK0_SIZE_HI) = HI16BIT(u32RegVal);
    u32RegVal = u32DRAMRBlkHSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(REG_GOP_DRAM_RBLK0_HSIZE) = LO16BIT(u32RegVal);

    //Color type
    u32RegVal = SYS_REGOP(REG_GOP_GWIN0_EN);
    ((G_GOP_gwin_set*)&u32RegVal)->GWIN_DTYPE = u32GWINFormat ; //ARGB8888
    SYS_REGOP(REG_GOP_GWIN0_EN) = u32RegVal|1; // enablue GWIN too

#if TIME_TEST
    ttt = stimer() ;
    printk( "\n\n\nGOP Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif

#if 0
    // fill in the color
    U32 x, y, idx ;
    U8* pu8 ;
    pu8 = 0xA0000000|GOP_BUFFER ;
    idx = 0 ;
    for( y=0; y<u32Height; y++ ){
        for( x=0; x<u32Width; x++ ){
            pu8[idx+(x<<2)] = 0x4C ;
            pu8[idx+(x<<2)+1] = 0xFF ;
            pu8[idx+(x<<2)+2] = 0x4C ;
            pu8[idx+(x<<2)+3] = 0x54 ;
        }
        idx += stride ;
    }
#endif

}
#else
static void BurstInitGOP(void)
{


#if (PANEL_TYPE==1366)

    U32 u32DispX = 162 ;
    U32 u32DispY = 0 ;
    U32 u32Width = 1366 ;
    U32 u32Height =768 ;

#elif (PANEL_TYPE==1920)

    U32 u32DispX = 182 ;
    U32 u32DispY = 0 ;
    U32 u32Width = 1920 ;
    U32 u32Height =1080 ;

#endif


#if TIME_TEST
    U32 ttt ;
    stimer() ;
#endif
    // _MHal_GOP_Init(eGOP_Type);
    SYS_REGOP( (0xF16<<2) ) &= ~0x1 ; // GOP4G ckg enable
    SYS_REGOP( REG_GOP_WR_ACK ) |= 0x200 ; // force write
    SYS_REGOP( REG_GOP_WR_ACK ) &= ~0xF ; // select GOP4G_0
    SYS_REGOP( REG_GOP_WR_ACK ) |= u8GOP ; // select GOP4G_0
    SYS_REGOP( REG_GOP_SOFT_RESET ) = 0x4A09 ; // into reset state

    SYS_REGOP( REG_GOP_DEST ) &= 0xFFF0 ; // set DMA length and destination:OP
    SYS_REGOP( REG_GOP_DEST ) |= 0x000A ;
    SYS_REGOP(REG_GOP_BLINK)   = 0x1500 ;  //disable blink, set blink rate and scroll rate
    SYS_REGOP(REG_GOP_GWIN_PRI)= 0x3210 ; //GWIN Priority: GWIN0>GWIN1>GWIN2>GWIN3
    SYS_REGOP(REG_GOP_RDMA_HT) = 0x0000 ;
    SYS_REGOP(REG_GOP_HSYNC_PIPE_DELAY) = 0x0013;

    SYS_REGOP( REG_GOP_WR_ACK ) &= ~0xF ; // select GOP4G_0
    SYS_REGOP( REG_GOP_WR_ACK ) |= (u8GOP + 1) ; // select GOP4G_0

    //initial Scroll Setps
    SYS_REGOP(REG_GWIN_SCROLL_LEN)   =0x0000;

    SYS_REGOP(REG_GOP_MUX_SEL) &= ~0x00FF; // set mux
    SYS_REGOP(REG_GOP_MUX_SEL) |= 0x00E4; //0x0001; // set mux

    SYS_REGOP( REG_GOP_WR_ACK ) &= ~0xF ; // select GOP4G_0
    SYS_REGOP( REG_GOP_WR_ACK ) |= u8GOP ; // select GOP4G_0

    SYS_REGOP( REG_GOP_SOFT_RESET ) = 0x4A08 ; // leave reset state

    SYS_REGOP(REG_GOP_STRCH_HSTART) = u32DispX;
    SYS_REGOP(REG_GOP_STRCH_VSTART) = u32DispY;
    SYS_REGOP(REG_GOP_STRCH_HSIZE) = u32Width>>1;
    SYS_REGOP(REG_GOP_STRCH_VSIZE) = u32Height;
    SYS_REGOP(REG_GOP_RDMA_HT) = (u32Width>>1) + 1;

    SYS_REGOP( REG_GOP_WR_ACK ) &= ~0xF ; // select GOP4G_0
    SYS_REGOP( REG_GOP_WR_ACK ) |= (u8GOP+1) ; // select GOP4G_0

#if TIME_TEST
    ttt = stimer() ;
    printk( "\n\n\nGOP Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif

}


static void BurstYUV422(U16 u16X, U16 u16Y, U32 u32InAddr, U32 u32OutAddr, U8 u8GWinIdx, U8 u8JPG_Idx)
{
    int tt = 0;

	// cleare the rotate setting
    SYS_REGOP( REG_GE_ROT ) &= ~0x3 ;

	SYS_REGOP( REG_GE_ALPHA_CONST ) = 0xFF;

	// Set color format --> source: YUV422, destination: ARGB8888
	SYS_REGOP( REG_GE_FMT ) = 0x0E0E;

    // Set source address
    SYS_REGOP(REG_GE_SRC_BASE_L) = u32InAddr & 0xFFFF;
    SYS_REGOP(REG_GE_SRC_BASE_H) = ((u32InAddr >> 16) & 0x0FFF);

    // Set source pitch
    SYS_REGOP(REG_GE_SRC_PITCH) =  jpegWidth[u8JPG_Idx] * 2;

    // Set destination address
    SYS_REGOP(REG_GE_DST_BASE_L) = u32OutAddr & 0xFFFF;
    if (0x10000000 <= u32OutAddr)   // Check if in MIU1
    {
        SYS_REGOP(REG_GE_DST_BASE_H) = ((u32OutAddr >> 16) & 0x0FFF) | 0x8000;
    }
    else
    {
        SYS_REGOP(REG_GE_DST_BASE_H) = ((u32OutAddr >> 16) & 0x0FFF);
    }

    // Set destination pitch
    SYS_REGOP(REG_GE_DST_PITCH) =  _Gwin_info[u8GWinIdx].u32Width * 2;

	SYS_REGOP(REG_GE_CLIP_L) = 0;
	SYS_REGOP(REG_GE_CLIP_R) = _Gwin_info[u8GWinIdx].u32Width;
	SYS_REGOP(REG_GE_CLIP_T) = 0;
	SYS_REGOP(REG_GE_CLIP_B) = _Gwin_info[u8GWinIdx].u32Height;

    SYS_REGOP( REG_GE_BLT_SRC_DX ) = 0x1000 ;   // not scaling
    SYS_REGOP( REG_GE_BLT_SRC_DY ) = 0x1000 ;
    SYS_REGOP( REG_GE_BLT_DST_X_OFST ) &= ~(0x1FFF) ;
    SYS_REGOP( REG_GE_BLT_DST_Y_OFST ) &= ~(0x1FFF) ;

	SYS_REGOP( REG_GE_PRIM_V0_X ) = u16X;                // Destination
	SYS_REGOP( REG_GE_PRIM_V0_Y ) = u16Y;
	SYS_REGOP( REG_GE_PRIM_V1_X ) = u16X + jpegWidth[u8JPG_Idx] - 1;
	SYS_REGOP( REG_GE_PRIM_V1_Y ) = u16Y + jpegHeight[u8JPG_Idx] - 1;
	SYS_REGOP( REG_GE_PRIM_V2_X ) = 0;                 // Source
	SYS_REGOP( REG_GE_PRIM_V2_Y ) = 0;
	SYS_REGOP( REG_GE_BLT_SRC_W ) = jpegWidth[u8JPG_Idx];
	SYS_REGOP( REG_GE_BLT_SRC_H ) = jpegHeight[u8JPG_Idx];

    SYS_REGOP( REG_GE_CFG ) = 0x1;

	SYS_REGOP( REG_GE_CMD ) = 0x40;

    while (((SYS_REGOP(REG_GE_STATUS)&0x01)))
    {
        tt++ ;
        asm volatile
        (
            "nop;"
        );
        if( (tt&0xFFFF)==0 )
        {
            printk("[GE INFO] GE is busy\n");
            break ;
        }
    }
/*
    tt = 0;
    while (((SYS_REGOP(REG_GE_STATUS)&0xF800)>>11) != 0xF8)
    {
        tt++;
        if (0 == (0xFFFFFF & tt))
        {
            printk("[GE INFO] GE wait command queue\n");
            break;
        }
    }
*/


}

static void BurstInitGWIN(U32 u32OutAddr, U8 u8GWinIdx)
{
    U32 u32RegVal, u32Tmp ;

    U32 u32Width = _Gwin_info[u8GWinIdx].u32Width;
    U32 u32Height = _Gwin_info[u8GWinIdx].u32Height;

    U32 stride = _GFXOSD_CALC_STRIDE( (u32Width<<2) ) ;
    U32 u32GWinSrcX = 0 ;
    U32 u32GWinSrcY = 0 ;
//    U32 u32GWinDispX_End = u32GWinDispX_Max;
//    U32 u32GWinDispY_End = u32GWinDispY_Max;
    //U32 u32DRAMRBlkStart = GOP_BUFFER ;
    U32 u32DRAMRBlkStart = u32OutAddr;
    U32 u32DRAMRBlkHSize = stride>>2 ;
    U32 u32DRAMRBlkVSize = u32Height ;
    U32 u32ColorDepth = 2 ; // 2 byte for YUV422
    U32 u32GWINFormat = 9 ; // ARGB8888=5, YUV422=9
    U32 u32GWinRegOffset = 0;


#if TIME_TEST
    U32 ttt ;
    stimer() ;
#endif

    if (u8GWinIdx > 3) // We have GWIN 0, 1, 2, 3
        return;

    if (u32OutAddr < 0x10000000)
    {
        switch (u8GOP)
        {
            case GOP4G_BANK:
                SYS_REGOP((0x900+0x79)<<2) &= ~0x0030;
                break;
            case GOP2G_BANK:
                SYS_REGOP((0x900+0x79)<<2) &= ~0x0040;
                break;
            case GOP1G_BANK:
                SYS_REGOP((0x900+0x79)<<2) &= ~0x0080;
                break;
            default:
                SYS_REGOP((0x900+0x79)<<2) &= ~0x0100;
                break;
        }

    }
    else
    {
        switch (u8GOP)
        {
            case GOP4G_BANK:
                SYS_REGOP((0x900+0x79)<<2) |= 0x0030;
                break;
            case GOP2G_BANK:
                SYS_REGOP((0x900+0x79)<<2) |= 0x0040;
                break;
            case GOP1G_BANK:
                SYS_REGOP((0x900+0x79)<<2) |= 0x0080;
                break;
            default:
                SYS_REGOP((0x900+0x79)<<2) |= 0x0100;
                break;
        }
    }

    // Calculate GWIN register base offset
    u32GWinRegOffset = (0x20 * u8GWinIdx) * 4;

    SYS_REGOP( REG_GOP_WR_ACK ) &= ~0xF ; // select GOP4G_0
    SYS_REGOP( REG_GOP_WR_ACK ) |= (u8GOP + 1) ;  // select GOP4G_1

    // init GWIN 3
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_EN) = 0x3F90;  // Init alpha value, disable H&V scroll, auto stop
    //initial Scroll auto stop offset
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_VSTOP_LO) =0x0000;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_VSTOP_HI) =0x0000;

#if 0  // alan comments for test SetGWin
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_VSTR) = u32GWinDispY ; //in pixel
    //SYS_REGOP(REG_GOP_GWIN0_VEND) = u32GWinDispY + u32Height;//in pixel
    if (u32GWinDispY + u32Height <= u32GWinDispY_Max)
        u32GWinDispY_End = u32GWinDispY + u32Height;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_VEND) = u32GWinDispY_End;//in pixel

    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_HSTR) = (u32GWinDispX) * u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment
    //SYS_REGOP(REG_GOP_GWIN0_HEND) = (u32GWinDispX + u32Width) * u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment
    if (u32GWinDispX + u32Width <= u32GWinDispX_Max)
        u32GWinDispX_End = u32GWinDispX + u32Width;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_HEND) = u32GWinDispX_End * u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment
#endif

    //Offset to RBLK
    u32RegVal = u32GWinSrcY * u32DRAMRBlkHSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_GWIN0_VSTR_LO) = LO16BIT(u32RegVal);
    SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_GWIN0_VSTR_HI) = HI16BIT(u32RegVal);
    u32RegVal = u32GWinSrcX * u32ColorDepth >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_GWIN0_HSTR) = LO16BIT(u32RegVal);

    //RBLK
    u32Tmp = u32DRAMRBlkStart >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_RBLK0_STR_LO) = LO16BIT(u32Tmp);
#if 0
    u32RegVal = SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_RBLK0_STR_HI);
    ((G_GOP_dram_rblk_str_hi*)&u32RegVal)->DRAM_RBLK_STR_HI = HI16BIT(u32Tmp) >> GOP_DRAM_RBLK_STR_HI_SHIFT;
    ((G_GOP_dram_rblk_str_hi*)&u32RegVal)->DRAM_RBLK_PAGE = HI16BIT(u32Tmp) >> GOP_DRAM_RBLK_PAGE_SHIFT;
#else
    u32RegVal = HI16BIT(u32Tmp) & 0x1FF;
#endif
    SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_RBLK0_STR_HI) = u32RegVal;

    u32RegVal = u32DRAMRBlkHSize * u32DRAMRBlkVSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_RBLK0_SIZE_LO) = LO16BIT(u32RegVal);
    SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_RBLK0_SIZE_HI) = HI16BIT(u32RegVal);
    u32RegVal = u32DRAMRBlkHSize * u32ColorDepth >> GOP_ALIGN_RSHIFT;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_DRAM_RBLK0_HSIZE) = LO16BIT(u32RegVal);

    //Color type
    u32RegVal = SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_EN);
    ((G_GOP_gwin_set*)&u32RegVal)->GWIN_DTYPE = u32GWINFormat ;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_EN) = u32RegVal|1; // enablue GWIN too

#if TIME_TEST
    ttt = stimer() ;
    printk( "\n\n\nGOP Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif

}


void BurstSetGWIN(U32 x, U32 y, U32 u32Width, U32 u32Height, U8 u8GWinIdx)
{
#if (PANEL_TYPE==1366)

		U32 u32GWinDispX_Max = 1366;
		U32 u32GWinDispY_Max = 768;

#elif (PANEL_TYPE==1920)

		U32 u32GWinDispX_Max = 1920 ;
		U32 u32GWinDispY_Max =1080 ;

#endif

//		U32 u32GWinSrcX = 0 ;
//		U32 u32GWinSrcY = 0 ;
		U32 u32GWinDispX = x;
		U32 u32GWinDispY = y;
		U32 u32GWinDispX_End = u32GWinDispX_Max;
		U32 u32GWinDispY_End = u32GWinDispY_Max;
		U32 u32ColorDepth = 2 ; // 2 byte for YUV422
//		U32 u32GWINFormat = 9 ; // ARGB8888=5, YUV422=9
		U32 u32GWinRegOffset = 0;


    if (u8GWinIdx > 3) // We have GWIN 0, 1, 2, 3
        return;

    // Calculate GWIN register base offset
    u32GWinRegOffset = (0x20 * u8GWinIdx) * 4;

    SYS_REGOP( REG_GOP_WR_ACK ) &= ~0x0F ; // select GOP4G_0
    SYS_REGOP( REG_GOP_WR_ACK ) |= (u8GOP + 1) ;  // select GOP4G_1

    // init GWIN 3
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_EN) = 0x3F90;  // Init alpha value, disable H&V scroll, auto stop
    //initial Scroll auto stop offset
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_VSTOP_LO) =0x0000;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_VSTOP_HI) =0x0000;

    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_VSTR) = u32GWinDispY ; //in pixel
    if (u32GWinDispY + u32Height <= u32GWinDispY_Max)
        u32GWinDispY_End = u32GWinDispY + u32Height;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_VEND) = u32GWinDispY_End;//in pixel

    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_HSTR) = (u32GWinDispX) * u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment
    if (u32GWinDispX + u32Width <= u32GWinDispX_Max)
        u32GWinDispX_End = u32GWinDispX + u32Width;
    SYS_REGOP(u32GWinRegOffset+REG_GOP_GWIN0_HEND) = u32GWinDispX_End * u32ColorDepth >> GOP_ALIGN_RSHIFT; //in HW alignment

}

#endif

//void BurstImage( void ){
//Setting registers for drawing splash window
void Splash_SetRegister(void)
{
#if USE_MEASURE_TIME
    U32 u32Timer;
#endif
#if TIME_TEST
    U32 ttt ;
    stimer();
#endif

/*** Scaler Init Start ***/
#if USE_MEASURE_TIME
    stimer_sc(0) ;
#endif
#if (PANEL_TYPE==1366)
#if USE_S7_Board
        MDrv_Logo_SC_Init(PANEL_RES_WXGA, LPLL_LVDS);
#else
        MDrv_Logo_SC_Init(PANEL_RES_WXGA, LPLL_TTL);
#endif
#elif (PANEL_TYPE==1920)
#if USE_S7_Board
        MDrv_Logo_SC_Init(PANEL_RES_FULL_HD, LPLL_LVDS);
#else
        MDrv_Logo_SC_Init(PANEL_RES_FULL_HD, LPLL_TTL);
#endif
#endif
#if USE_MEASURE_TIME
    u32Timer = stimer_sc(1) ;
    printk("Scaler Duration loading time=%u us\n", u32Timer/12);
#endif
/*** Scaler End Start ***/

/*** URSA Init Start ***/
#if !USE_S7_Board
#if USE_MEASURE_TIME
    stimer_sc(0) ;
#endif
    #if USE_S7M_FHD_120HZ
    MAdp_MFC_Init(&LC420WUF, 1);
    #elif USE_S7M_FHD_60HZ
    MAdp_MFC_Init(&AU20_1680_1050, 1);
    #elif USE_S7M_WXGA_60HZ
    MAdp_MFC_Init(&AU20_WXGA, 1);
    #endif
#if USE_MEASURE_TIME
    u32Timer = stimer_sc(1) ;
    printk("\n\n Ursa Duration loading time=%u us\n\n", u32Timer/12);
#endif
#endif

/*** URSA Init End  ***/

#if TIME_TEST
    ttt = stimer() ;
    printk( "\n\n\nScaler Duration: %d.%03d sec, %08X\n\n\n", ttt/12000000, (ttt/12000)%1000, ttt ) ;
#endif

    SYS_REGOP(0x378<<2) |= 0x0200; // GE decides which MIU by itself

    SYS_REGOP( REG_GE_EN) = 0x1 ; // Enable GE

    BurstInitGOP() ;

}


void Splash_DecodeJPEG(unsigned int u32InAddr, unsigned int u32OutAddr, unsigned char u8JPG_Idx)
{
    unsigned int u32RegValue;

    //Decoding JPEG

    u32RegValue = *(volatile unsigned int *)(0xBF200000 + (0x97B<<2));

    if (u32InAddr < 0xC0000000)
    {
        u32InAddr |= 0xA0000000;
        u32OutAddr |= 0xA0000000;

        *(volatile unsigned int *)(0xBF200000 + (0x97B<<2)) &= ~0x0100;
    }
    else
    {
        u32InAddr &= ~0xC0000000;
        u32OutAddr &= ~0xC0000000;

        *(volatile unsigned int *)(0xBF200000 + (0x97B<<2)) |= 0x0100;

    }

    DecodeJPEG( u32InAddr, u32OutAddr, u8JPG_Idx);

    *(volatile unsigned int *)(0xBF200000 + (0x97B<<2)) = u32RegValue;
}


void Splash_SetGwin(int x, int y, int width, int height, unsigned char u8GWinIdx)
{
    _Gwin_info[u8GWinIdx].u8GwinID = u8GWinIdx;
	_Gwin_info[u8GWinIdx].u32X = x;
	_Gwin_info[u8GWinIdx].u32Y = y;
	_Gwin_info[u8GWinIdx].u32Width = width;
	_Gwin_info[u8GWinIdx].u32Height = height;

    BurstSetGWIN(x, y, width, height, u8GWinIdx);
}

void Splash_DrawImage(int x, int y, unsigned int u32InAddr, unsigned int u32OutAddr, unsigned char u8GWinIdx, unsigned char u8JPG_Idx)
{
    //Draw images
    if (u32OutAddr >= 0xA0000000)
    {
        u32OutAddr &= (~0xA0000000);
    }
    else if (u32OutAddr >= 0x40000000 && u32OutAddr < 0xA0000000)
    {
        u32OutAddr &= (~0x40000000);
        u32OutAddr += 0x10000000;
    }
    BurstYUV422(x, y, u32InAddr, u32OutAddr, u8GWinIdx, u8JPG_Idx);
    BurstInitGWIN(u32OutAddr, u8GWinIdx);
}


//redcloud00@lge.com logo
#define NAND_MTDINFO_OFFSET 	(0xC0000)
#define MTD_STR_LEN 			32
#define MTD_MAP_MAX 			4
#define MTD_PARTITION_MAX		32
#define FLASH_BASE					0x00000000

#define CFG_LOAD_ADDR					0x80C00000 //0x82000000


#define IMG_LOADED_ADDR					(CFG_LOAD_ADDR | 0x20000000)	//(0xA2000000)
#define GOP_OUT_ADDR					(0x42000000)					//(0xA2C00000) //0x42000000 //0xA8100000
#define _0FFSET							(0x600000)

#define SPLASH_BG_BOOTING_BMP_ADDR		(0x81E80000 | 0x20000000)  // (0x85000000 | 0x20000000)		//0xA4000000 to 0x85000000
#define SPLASH_LG_LOG_BMP_ADDR			(SPLASH_BG_BOOTING_BMP_ADDR + _0FFSET)
#define	SPLASH_BG_GRAPHIC_BMP_ADDR		(SPLASH_LG_LOG_BMP_ADDR + _0FFSET)
#define	SPLASH_CLOCK_ICON_BMP_ADDR		(SPLASH_BG_GRAPHIC_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_COLON_BMP_ADDR		(SPLASH_CLOCK_ICON_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_HH_BMP_ADDR			(SPLASH_TIME_COLON_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_H_BMP_ADDR			(SPLASH_TIME_HH_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_MM_BMP_ADDR			(SPLASH_TIME_H_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_M_BMP_ADDR			(SPLASH_TIME_MM_BMP_ADDR + _0FFSET)

/*
#define IMG_LOADED_ADDR (0xA2000000)
#define GOP_OUT_ADDR	(0x42000000) // (0xA2C00000) //0x42000000 //0xA8100000
#define _0FFSET			(0x600000)

#define SPLASH_BG_BOOTING_BMP_ADDR		(0xA4000000)
#define SPLASH_LG_LOG_BMP_ADDR			(SPLASH_BG_BOOTING_BMP_ADDR + _0FFSET)
#define	SPLASH_BG_GRAPHIC_BMP_ADDR		(SPLASH_LG_LOG_BMP_ADDR + _0FFSET)
#define	SPLASH_CLOCK_ICON_BMP_ADDR		(SPLASH_BG_GRAPHIC_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_COLON_BMP_ADDR		(SPLASH_CLOCK_ICON_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_HH_BMP_ADDR			(SPLASH_TIME_COLON_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_H_BMP_ADDR			(SPLASH_TIME_HH_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_MM_BMP_ADDR			(SPLASH_TIME_H_BMP_ADDR + _0FFSET)
#define	SPLASH_TIME_M_BMP_ADDR			(SPLASH_TIME_MM_BMP_ADDR + _0FFSET)
*/
U8 Splash_MICOM_PanelOn(void)
{
	U8	panel_on;
	U8	retry = 0;

	do{
		MICOM_TurnOnPanelOn();
		mdelay(10);
		MICOM_VerifyPanelOn(&panel_on);
		retry++;
		printf("          ==> panel_on =%d \n",panel_on);
		printf("          ==> panel retry = %d  \n",retry);
	}while( panel_on != 1 &&  retry < 3 );

	if(!panel_on || retry >= 3)
		printf("==> panel on is failed \n");

	return 0;
}

U8 Splash_MICOM_InvOn(void)
{
	U8	inv_on;
	U8	retry = 0;

	do{
		MICOM_TurnOnInv();
		mdelay(10);
		MICOM_VerifyInverterOn(&inv_on);
		retry++;
		printf("          ==> inv_on =%d \n",inv_on);
		printf("          ==>  inv retry = %d  \n",retry);
	}while( inv_on != 1 && retry < 3 );

	if(!inv_on || retry >= 3)
		printf("==> invert on is failed \n");

	return 0;
}

signed int get_partition_info(ulong* len,struct mstar_mtd_info* mstar_mtdinfo)//struct mtd_partition_info* mstar_mtd)//(ulong idx, struct mtd_partition_info* mstar_ptt)//struct mtd_info *mtd, int idx, mtd_partition_info* mstar_ptt)
{

	//struct mstar_mtd_info *mstar_mtdinfo;

	int i=0, ret=0, retlen=0;
	char *pname = "logo";
	ulong idx = 0;

	ret = nand_mtd->read(nand_mtd, NAND_MTDINFO_OFFSET, sizeof(struct mstar_mtd_info),
			&retlen, (u_char *)(mstar_mtdinfo));

	if(ret) {
		printk("%s : mtdinfo read failed from nand flash\n", __func__);
		return ret;
	}

	do {
		if(mstar_mtdinfo->partition[idx].used && mstar_mtdinfo->partition[idx].valid && (strcmp(pname, mstar_mtdinfo->partition[idx].name) == 0)) {
			*len = mstar_mtdinfo->partition[idx].filesize;
			break;
		}

	} while (++idx < MTD_PARTITION_MAX);

	if (idx == MTD_PARTITION_MAX) {
		printf("Unknown partition name(%s)..\n", pname);
		return -1;
	}

	return idx ; //0;
}


int Splash_Cp2Ram (ulong addr)
{
//	char *pname = "logo";
	ulong idx = 0, ret = 0;
	ulong src, dst, len = 0;
//	ulong s_msec, e_msec;
	struct mtd_partition_info *mpi = NULL;
	struct mstar_mtd_info *mstar_mtdinfo = NULL;
	int retlen=0;
	u_char test[32];
	int i;

//	printk("== nand_mtd = 0x%x \n",nand_mtd);
	/* find partition */

	mstar_mtdinfo = (struct mstar_mtd_info *)kmalloc (sizeof(struct mstar_mtd_info), GFP_KERNEL);
	if(mstar_mtdinfo == NULL) {
		printk("%s : kmalloc error\n", __func__);
		return -1;
	}

	idx = get_partition_info(&len,mstar_mtdinfo);//mpi);
	mpi = &(mstar_mtdinfo->partition[idx]);	

	if (len == 0) {
		printf("Partition Image Size Zero Length Case!! ==> Skipped!!\n");
		return 1;
	}

	src = (FLASH_BASE + mpi->offset);
	dst = addr;

	//s_msec = get_cur_time();
	//printf("[%d] Copy \"%s (%ld bytes)\" from 0x%08lx to 0x%08lx\n", s_msec, mpi->name, len, src, dst);
	ret = nand_mtd->read(nand_mtd,src, len, &retlen,(u_char *)dst);

	if(ret) {
		printf("%s : mtdinfo read failed from nand flash\n", __func__);
		return ret;}


	return 0;
}

void Splash_GetJpegInfo(SPLASH_BMP_TYPE_T bmpType, SPLASH_BMP_INFO_T *pSplashBmpInfo, void *argv )
{
	TIME_T *pTimeSt;
	int time;

	pTimeSt = (TIME_T*)argv;
	switch ( bmpType )
	{
		case SPLASH_BG_BOOTING_BMP:
#if 0
			pSplashBmpInfo->offset 	= BOOTING_BG_OFFSET;
			pSplashBmpInfo->size	= BOOTING_BG_SIZE;
			pSplashBmpInfo->xPos	= BOOTING_BG_X_POS;
			pSplashBmpInfo->yPos	= BOOTING_BG_Y_POS;
#else
			pSplashBmpInfo->offset 	= BG_BACKGROUND_OFFSET;
			pSplashBmpInfo->size	= BG_BACKGROUND_SIZE;
			pSplashBmpInfo->xPos	= BG_BACKGROUND_X_POS;
			pSplashBmpInfo->yPos	= BG_BACKGROUND_Y_POS;
#endif
			break;
		case SPLASH_BG_GRAPHIC_BMP:
			pSplashBmpInfo->offset 	= BG_GRAPHIC_OFFSET;
			pSplashBmpInfo->size	= BG_GRAPHIC_SIZE;
			pSplashBmpInfo->xPos	= BG_GRAPHIC_X_POS;
			pSplashBmpInfo->yPos	= BG_GRAPHIC_Y_POS;
			break;
		case SPLASH_LG_LOG_BMP:
			pSplashBmpInfo->offset 	= BG_LOGO_OFFSET;
			pSplashBmpInfo->size	= BG_LOGO_SIZE;
			pSplashBmpInfo->xPos	= BG_LOGO_X_POS;
			pSplashBmpInfo->yPos	= BG_LOGO_Y_POS;
			break;
		case SPLASH_CLOCK_ICON_BMP:
			pSplashBmpInfo->offset	= CLOCK_ICON_OFFSET;
			pSplashBmpInfo->size	= CLOCK_ICON_SIZE;
			pSplashBmpInfo->xPos	= CLOCK_ICON_X_POS;
			pSplashBmpInfo->yPos	= CLOCK_ICON_Y_POS;
			break;
		case SPLASH_TIME_HH_BMP:
			//TODO get time here
			time = pTimeSt->hour/10;
			if( time == 0 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_00_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_00_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_00_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_00_Y_POS;
			}
			else if( time == 1 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_10_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_10_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_10_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_10_Y_POS;
			}
			else if( time == 2 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_20_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_20_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_20_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_20_Y_POS;
			}
			else
			{
				pSplashBmpInfo->offset 	= TIME_HH_00_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_00_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_00_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_00_Y_POS;
			}
			break;
		case SPLASH_TIME_H_BMP:
			time = pTimeSt->hour%12;
			if( time == 0 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_0_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_0_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_0_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_0_Y_POS;
			}
			else if( time == 1 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_1_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_1_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_1_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_1_Y_POS;
			}
			else if( time == 2 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_2_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_2_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_2_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_2_Y_POS;
			}
			else if( time == 3 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_3_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_3_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_3_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_3_Y_POS;
			}
			else if( time == 4 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_4_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_4_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_4_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_4_Y_POS;
			}
			else if( time == 5 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_5_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_5_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_5_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_5_Y_POS;
			}
			else if( time == 6 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_6_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_6_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_6_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_6_Y_POS;
			}
			else if( time == 7 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_7_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_7_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_7_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_7_Y_POS;
			}
			else if( time == 8 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_8_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_8_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_8_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_8_Y_POS;
			}
			else if( time == 9 )
			{
				pSplashBmpInfo->offset 	= TIME_HH_9_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_9_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_9_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_9_Y_POS;
			}
			else
			{
				pSplashBmpInfo->offset 	= TIME_HH_0_OFFSET;
				pSplashBmpInfo->size	= TIME_HH_0_SIZE;
				pSplashBmpInfo->xPos	= TIME_HH_0_X_POS;
				pSplashBmpInfo->yPos	= TIME_HH_0_Y_POS;
			}
			break;
		case SPLASH_TIME_COLON_BMP:
			pSplashBmpInfo->offset	= TIME_COLON_OFFSET;
			pSplashBmpInfo->size	= TIME_COLON_SIZE;
			pSplashBmpInfo->xPos	= TIME_COLON_X_POS;
			pSplashBmpInfo->yPos	= TIME_COLON_Y_POS;
			break;
		case SPLASH_TIME_MM_BMP:
			time = pTimeSt->minute/10;
			if( time == 0 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_00_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_00_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_00_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_00_Y_POS;
			}
			else if( time == 1 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_10_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_10_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_10_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_10_Y_POS;
			}
			else if( time == 2 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_20_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_20_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_20_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_20_Y_POS;
			}
			else if( time == 3 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_30_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_30_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_30_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_30_Y_POS;
			}
			else if( time == 4 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_40_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_40_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_40_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_40_Y_POS;
			}
			else if( time == 5 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_50_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_50_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_50_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_50_Y_POS;
			}
			else
			{
				pSplashBmpInfo->offset 	= TIME_MM_00_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_00_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_00_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_00_Y_POS;
			}
			break;
		case SPLASH_TIME_M_BMP:
			time = pTimeSt->minute%10;
			if( time == 0 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_0_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_0_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_0_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_0_Y_POS;
			}
			else if( time == 1 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_1_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_1_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_1_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_1_Y_POS;
			}
			else if( time == 2 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_2_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_2_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_2_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_2_Y_POS;
			}
			else if( time == 3 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_3_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_3_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_3_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_3_Y_POS;
			}
			else if( time == 4 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_4_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_4_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_4_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_4_Y_POS;
			}
			else if( time == 5 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_5_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_5_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_5_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_5_Y_POS;
			}
			else if( time == 6 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_6_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_6_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_6_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_6_Y_POS;
			}
			else if( time == 7 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_7_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_7_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_7_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_7_Y_POS;
			}
			else if( time == 8 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_8_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_8_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_8_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_8_Y_POS;
			}
			else if( time == 9 )
			{
				pSplashBmpInfo->offset 	= TIME_MM_9_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_9_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_9_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_9_Y_POS;
			}
			else
			{
				pSplashBmpInfo->offset 	= TIME_MM_0_OFFSET;
				pSplashBmpInfo->size	= TIME_MM_0_SIZE;
				pSplashBmpInfo->xPos	= TIME_MM_0_X_POS;
				pSplashBmpInfo->yPos	= TIME_MM_0_Y_POS;
			}
			break;

		default :

			pSplashBmpInfo->offset	= 0;
			pSplashBmpInfo->size	= 0;
			pSplashBmpInfo->xPos	= 0;
			pSplashBmpInfo->yPos	= 0;
	}
}



void BootSplash (void)
{
	SPLASH_BMP_INFO_T splash_bmp_info[9];
	TIME_T rtcTime;
//	U32	nvmData;
//	PANEL_POWER_SEQ_T	pnlpwrseq;
//	PANEL_PWM_T			pnlpwm;
//	TOOL_OPTION3_T toolOpt3;
//	U32 				toolOptvalid;

	
	printk("==BootSplash== \n");


//	DDI_NVM_Read(SYS_DB_BASE + (UINT32)&(gSysNvmDB.validMark) - (UINT32)&gSysNvmDB,			\
//				sizeof(gSysNvmDB.validMark), (UINT8 *)&(nvmData));

//	printf("nvmData = %d  \n", nvmData);
//	printf("gSysNvmDB.validMark = %d  \n", gSysNvmDB.validMark);
/*
	if(nvmData != gSysNvmDB.validMark)
	{
		DDI_NVM_Write(SYS_DB_BASE + (UINT32)&(gSysNvmDB.panelpowerseq) - (UINT32)&gSysNvmDB,			\
				sizeof(gSysNvmDB.panelpowerseq), (UINT8 *)&(gSysNvmDB.panelpowerseq));

		DDI_NVM_Write(SYS_DB_BASE + (UINT32)&(gSysNvmDB.panelpwm) - (UINT32)&gSysNvmDB,			\
				sizeof(gSysNvmDB.panelpwm), (UINT8 *)&(gSysNvmDB.panelpowerseq));
	}


	DDI_NVM_Read(SYS_DB_BASE + (UINT32)&(gSysNvmDB.panelpowerseq) - (UINT32)&gSysNvmDB,			\
				sizeof(gSysNvmDB.panelpowerseq), (UINT8 *)&(pnlpwrseq));

	DDI_NVM_Read(SYS_DB_BASE + (UINT32)&(gSysNvmDB.panelpwm) - (UINT32)&gSysNvmDB,			\
				sizeof(gSysNvmDB.panelpwm), (UINT8 *)&(pnlpwm));



	DDI_NVM_GetToolOpt3(&toolOpt3);
*/
//	printf("==Ram size : %d == \n", gd->ram_size*2);
//	Splash_Print_HWopt();
    MDrv_IIC_Init();
	MDrv_SW_IIC_SetSpeed(7,6);
	// panel power on
	Splash_MICOM_PanelOn();
//	mdelay(pnlpwrseq.powerSeqT2);
//	MICOM_TurnOnPanelOn();
	
	// scaler , mfc init
	Splash_SetRegister();
	
//	if(toolOpt3.flags.eBackLight == BL_NOR_LED || toolOpt3.flags.eBackLight == BL_EDGE_LED)
//		MAdp_MFC_SetReverseControl(3);

//	DDI_PWM_Init(); // PWM Initsh
	MICOM_GetRTCTime(&rtcTime);

//	printf("hour = %d \n",rtcTime.hour);
//	printf("minute = %d \n",rtcTime.minute);

	Splash_SetGwin(0, 0, 1920, 1080,1);


	// Copy all splash image at once.
	Splash_Cp2Ram(IMG_LOADED_ADDR);


	// Read jpeg image from NAND and decde those JPEG images

	Splash_GetJpegInfo(SPLASH_BG_BOOTING_BMP, &splash_bmp_info[0], NULL);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[0].offset, SPLASH_BG_BOOTING_BMP_ADDR, 0);

	Splash_GetJpegInfo(SPLASH_BG_GRAPHIC_BMP, &splash_bmp_info[1], NULL);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[1].offset, SPLASH_BG_GRAPHIC_BMP_ADDR, 1);

	Splash_GetJpegInfo(SPLASH_LG_LOG_BMP, &splash_bmp_info[2], NULL);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[2].offset, SPLASH_LG_LOG_BMP_ADDR, 2);

	Splash_GetJpegInfo(SPLASH_CLOCK_ICON_BMP, &splash_bmp_info[3], NULL);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[3].offset, SPLASH_CLOCK_ICON_BMP_ADDR, 3);

	Splash_GetJpegInfo(SPLASH_TIME_COLON_BMP, &splash_bmp_info[4], NULL);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[4].offset, SPLASH_TIME_COLON_BMP_ADDR, 4);

	Splash_GetJpegInfo(SPLASH_TIME_HH_BMP, &splash_bmp_info[5],&rtcTime);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[5].offset, SPLASH_TIME_HH_BMP_ADDR, 5);

	Splash_GetJpegInfo(SPLASH_TIME_H_BMP, &splash_bmp_info[6], &rtcTime);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[6].offset, SPLASH_TIME_H_BMP_ADDR, 6);

	Splash_GetJpegInfo(SPLASH_TIME_MM_BMP, &splash_bmp_info[7], &rtcTime);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[7].offset, SPLASH_TIME_MM_BMP_ADDR, 7);

	Splash_GetJpegInfo(SPLASH_TIME_M_BMP, &splash_bmp_info[8], &rtcTime);
	Splash_DecodeJPEG(IMG_LOADED_ADDR+splash_bmp_info[8].offset, SPLASH_TIME_M_BMP_ADDR, 8);

	/*
	 * Invert On 전 10ms 이상  VbrA, B 3.0V 이상 요구.
	 * 시간확보를 위해 Jpeg decode 전에 PWM init 실행.
	 */

//	DDI_PQ_PWMInitContol();

	//Draw decoded images
		
	Splash_DrawImage(splash_bmp_info[0].xPos,splash_bmp_info[0].yPos,SPLASH_BG_BOOTING_BMP_ADDR,GOP_OUT_ADDR,1,0);
	Splash_DrawImage(splash_bmp_info[1].xPos,splash_bmp_info[1].yPos,SPLASH_BG_GRAPHIC_BMP_ADDR,GOP_OUT_ADDR,1,1);
	Splash_DrawImage(splash_bmp_info[2].xPos,splash_bmp_info[2].yPos,SPLASH_LG_LOG_BMP_ADDR,GOP_OUT_ADDR,1,2);
	Splash_DrawImage(splash_bmp_info[3].xPos,splash_bmp_info[3].yPos,SPLASH_CLOCK_ICON_BMP_ADDR,GOP_OUT_ADDR,1,3);	
	Splash_DrawImage(splash_bmp_info[4].xPos,splash_bmp_info[4].yPos,SPLASH_TIME_COLON_BMP_ADDR,GOP_OUT_ADDR,1,4);	
	Splash_DrawImage(splash_bmp_info[5].xPos,splash_bmp_info[5].yPos,SPLASH_TIME_HH_BMP_ADDR,GOP_OUT_ADDR,1,5);
	Splash_DrawImage(splash_bmp_info[6].xPos,splash_bmp_info[6].yPos,SPLASH_TIME_H_BMP_ADDR,GOP_OUT_ADDR,1,6);
	Splash_DrawImage(splash_bmp_info[7].xPos,splash_bmp_info[7].yPos,SPLASH_TIME_MM_BMP_ADDR,GOP_OUT_ADDR,1,7);
	Splash_DrawImage(splash_bmp_info[8].xPos,splash_bmp_info[8].yPos,SPLASH_TIME_M_BMP_ADDR,GOP_OUT_ADDR,1,8);

	/* PWM init <-> Inv. On delay 필요하나 JPEG decoding 시간으로 충분한 delay 확보됨. */
	
//	mdelay(pnlpwrseq.powerSeqT3);
	mdelay(200);
 	Splash_MICOM_InvOn();

 	MICOM_TurnOnInv();

}

static int __init mod_logo_init(void)
{

	printk("mod_logo_init \n");
	//BootSplash();
	return 0;

}

module_init(mod_logo_init);



