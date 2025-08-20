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
//
//      File name: drvGE.c
//      Description:  GR driver implementation.
//                    1. PE part : piexl engine
//                    2. Blt part : fast blt engine
//
///////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------------

#ifdef RED_LION
#include <linux/kernel.h>   /* printk() */
#include <linux/spinlock.h>
#include <linux/string.h>
#else
#include "MsCommon.h"
#endif
#include "chip_setup.h"
#include "mhal_ge_reg.h"
#include "mhal_ge.h"
#include "mhal_ge_tp.h"
#include "mhal_ge_fp.h"
// @FIXME: Richard: temporarily remove this
// #include "drvPower.h"

//----------------------------------------------------------------------------
// Compile options
//----------------------------------------------------------------------------
// #define GE_DEBUG
#define GE_ADDR_RELAX

//----------------------------------------------------------------------------
// Local Defines
//----------------------------------------------------------------------------
#define WAIT_CMD_FIFO   1
#if WAIT_CMD_FIFO
#define PATCH_RD_CMD    0
#else
#define PATCH_RD_CMD    0
#endif
#define GE_AUTO_ALIGN_SRC_BUFFER        0
#define GE_AUTO_ALIGN_FONT              0

// addr >256MB => MIU 1, else => MIU 0
__inline__ __attribute__((always_inline)) U32 Phy_to_HAL_Addr(U32 u32Addr)
{
	if ( u32Addr & 0xF0000000 )
	{
		u32Addr &= 0x0FFFFFFF;
		u32Addr |= 0x80000000;
	}
	
	return u32Addr;
}
// MIU 1 => >256MB, else => MIU 0 <256MB
#define HAL_TO_PHY_ADDR(u32Addr)	{ if ( (u32Addr) & 0x80000000 ) { (u32Addr) &= 0x0FFFFFFF; (u32Addr) |= 0x10000000; } }

//----------------------------------------------------------------------------
// Debug Macros
//----------------------------------------------------------------------------
#define GE_DEBUGINFO(x)

#ifdef GE_DEBUG
#define GE_ASSERT(_bool, _f)                 if (!(_bool)) { (_f); while (1) printk("G0"); }
#else
#define GE_ASSERT(_bool, _f)                 while (0)
#endif // #ifdef SCL_DEBUG

#ifdef GE_DEBUG

#define _GE_CHECK_BUFFER_ALIGN0(addr, color_fmt)                                                \
    switch ((color_fmt)){                                                                       \
    case GE_FMT_I1:                                                                             \
    case GE_FMT_I2:                                                                             \
    case GE_FMT_I4:                                                                             \
    case GE_FMT_I8:                                                                             \
        break;                                                                                  \
    case GE_FMT_YUV422:                                                                         \
    case GE_FMT_1ABFGBG12355:                                                                   \
    case GE_FMT_RGB565 :                                                                        \
    case GE_FMT_ARGB1555 :                                                                      \
    case GE_FMT_ARGB4444 :                                                                      \
        GE_ASSERT(!(0x1 & (addr)), printk("[GE DRV][%06d] Bad buffer address (0x%08x, %d)\n", (addr), (color_fmt)));    \
        break;                                                                                  \
    case GE_FMT_ARGB8888 :                                                                      \
        GE_ASSERT(!(0x3 & (addr)), printk("[GE DRV][%06d] Bad buffer address (0x%08x, %d)\n", (addr), (color_fmt)));    \
        break;                                                                                  \
    default:                                                                                    \
        GE_ASSERT(0, printk("[GE DRV][%06d] Invalid color format\n"));                          \
        break;                                                                                  \
    }


#define _GE_CHECK_BUFFER_ALIGN1(addr, width, height, pitch, color_fmt)                          \
    switch ((color_fmt)){                                                                       \
    case GE_FMT_I1:                                                                             \
    case GE_FMT_I2:                                                                             \
    case GE_FMT_I4:                                                                             \
    case GE_FMT_I8:                                                                             \
        break;                                                                                  \
    case GE_FMT_YUV422:                                                                         \
    case GE_FMT_1ABFGBG12355:                                                                   \
    case GE_FMT_RGB565 :                                                                        \
    case GE_FMT_ARGB1555 :                                                                      \
    case GE_FMT_ARGB4444 :                                                                      \
        GE_ASSERT(!(0x1 & (addr)), printk("[GE DRV][%06d] Bad buffer address (0x%08x, %d)\n", (addr), (color_fmt)));            \
        GE_ASSERT(!(0x1 & (pitch)), printk("[GE DRV][%06d] Bad buffer pitch (%d, %d)\n", (pitch), (color_fmt)));                \
        GE_ASSERT(((pitch)>>1)== (width), printk("[GE DRV][%06d] Bad buffer pitch/width (%d, %d)\n", (pitch), (width)));        \
        break;                                                                                  \
    case GE_FMT_ARGB8888 :                                                                      \
        GE_ASSERT(!(0x3 & (addr)), printk("[GE DRV][%06d] Bad buffer address (0x%08x, %d)\n", (addr), (color_fmt)));            \
        GE_ASSERT(!(0x3 & (pitch)), printk("[GE DRV][%06d] Bad buffer pitch (%d, %d)\n", (pitch), (color_fmt)));                \
        GE_ASSERT(((pitch)>>2)== (width), printk("[GE DRV][%06d] Bad buffer pitch/width (%d, %d)\n", (pitch), (width)));        \
        break;                                                                                  \
    default:                                                                                    \
        GE_ASSERT(0, printk("[GE DRV][%06d] Invalid color format\n"));                          \
        break;                                                                                  \
    }




#define _GE_SIMPLE_BB_CHECK()                                                                   \
    GE_WaitAvailableCMDQueue(32);                                                               \
    if (!(GE_VAL_EN_STRETCH_BITBLT & GE_ReadReg(GE_REG_FMT_BLT)))                               \
    {                                                                                           \
        if (GE_ReadReg(GE_REG_STBB_INIT_DX) || GE_ReadReg(GE_REG_STBB_INIT_DY))                 \
        {                                                                                       \
            while (1)printk("G1");                                                                          \
        }                                                                                       \
        if (GE_ReadReg(GE_REG_STBB_WIDTH)!= (GE_ReadReg(GE_REG_PRI_V1_X)- GE_ReadReg(GE_REG_PRI_V0_X)+ 1))  \
        {                                                                                       \
            while (1)printk("G2");                                                                          \
        }                                                                                       \
        if (GE_ReadReg(GE_REG_STBB_HEIGHT)!= (GE_ReadReg(GE_REG_PRI_V1_Y)- GE_ReadReg(GE_REG_PRI_V0_Y)+ 1)) \
        {                                                                                       \
            while (1)printk("G3");                                                                          \
        }                                                                                       \
    }

#else // #ifdef GE_DEBUG

#define _GE_CHECK_BUFFER_ALIGN0(addr, color_fmt)                                while (0);
#define _GE_CHECK_BUFFER_ALIGN1(addr, width, height, pitch, color_fmt)          while (0);
#define _GE_SIMPLE_BB_CHECK()                                                   while (0);

#endif // #ifdef GE_DEBUG

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
//#define CHECK_IDLE() while(BLT_Reg(BLT_REG_STATUS)&BLT_VAL_BUSY) { MsOS_DelayTask(1);}
// #define CHECK_IDLE() while(BLT_Reg(BLT_REG_STATUS)&BLT_VAL_BUSY) { MsOS_YieldTask();}
#define delay(ms)
//#define delay(ms)    MsOS_DelayTask(ms)

#if 1
//#define GE_WaitCmdEmpty     {while(((GE_Reg(GE_REG_STATUS)) & GE_MSK_CMQ_FIFO_STATUS) != 0x80);}
//#define GE_WaitCmdNotFull   {while(((GE_Reg(GE_REG_STATUS)) & GE_MSK_CMQ_FIFO_STATUS) <= 0x04);}
#define GE_WaitCmdEmpty()       while(((GE_Reg(GE_REG_STATUS)) & GE_MSK_CMQ_FIFO_STATUS) != 0x80)
#define GE_WaitCmdNotFull()     while ((GE_Reg(GE_REG_STATUS) & GE_MSK_CMQ_FIFO_STATUS)== 0x00)
#else
#define GE_WaitCmdEmpty
#define GE_WaitCmdNotFull
#endif


#ifndef DYNAMIC_POWER_ON_OFF    // added to remove warning(dreamer@lge.com)
#define DYNAMIC_POWER_ON_OFF    0
#endif
#ifndef MOVE_TO_USER_SPACE      // added to remove warning(dreamer@lge.com)
#define MOVE_TO_USER_SPACE      0
#endif

//----------------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------------
//static U32 _u32FrameWidth, _u32FrameHeight, _u32FramePitch;
#ifdef RED_LION
//spinlock_t GE_SpinLock; // LGE. jjab@lge.com
#else
static S32 _s32GeMutex = -1;
#endif

static GE_FONT_INFO _FontTable[MAX_FONT];
static GE_BITMAP_INFO _BitmapTable[MAX_BITMAP];

#ifdef DBGLOG
//debug use only
static U16 _bOutFileLog = false;
static U16  *_pu16OutLogAddr = NULL;
static U16  _u16LogCount=0;
#endif

static U32 _u32RegBackup[0x80];
static U32 _u32Reg60hFlag = 0;
#if PATCH_RD_CMD
static U32 _u32Reg0hValue = 0;
#endif
static U16 _u16PeClipX1 = 0;
static U16 _u16PeClipX2 = 0;
static U16 _u16PeClipY1 = 0;
static U16 _u16PeClipY2 = 0;


static U8  _u8TypeGE[32] = "MstarTypeGE";

#if GE_AUTO_ALIGN_SRC_BUFFER
static U32 _u32SrcAddrPE;
static U32 _u32SrcPitchPE;
static U32 _u32SrcFmtPE;
static U32 _u32OffsetSrc;
#endif

extern int g_emp_movie_in_play ; // Samuel, 090109

//----------------------------------------------------------------------------
// Local Function Prototypes
//----------------------------------------------------------------------------
void GE_ConvertRGB2DBFmt(GE_Buffer_Format Fmt, U32 *color, U16* low, U16* high);
// void BLT_ConvertRGB2DBFmt(GE_Buffer_Format Fmt, U32 *colorinfo, U16* low, U16* high);

extern void msleep(unsigned int msec);

static void GE_WaitAvailableCMDQueue(U8 u8CMDCount)
{
#if WAIT_CMD_FIFO
    U32                 wait_count= 1;
    unsigned int tt ;
    u8CMDCount += 2 ; // Samuel, 090109
    u8CMDCount = (u8CMDCount > 32)? 32 : u8CMDCount;

//Note: In Titania, 1 empty count can save 2 commands.
//      And if the empty item only saves 1 commands, it is treated as empty;
//      therefore 15 cmmands will need (15+1)/2 empty items.
    u8CMDCount = ((u8CMDCount + 1)>>1);


    // check GE busy status to avoid command queue bug
    tt = 0 ;
    while (((GE_Reg(GE_REG_STATUS)&0x01))){
        if( g_emp_movie_in_play ) msleep(2) ; // Samuel, 090109
        tt++ ;
        if( (tt&0xFFFFFF)==0 )
            printk("G5 ") ;
    }
    //return;

//    while (((GE_Reg(GE_REG_STATUS)&0xfc)>>2) < u8CMDCount)
    while (((GE_Reg(GE_REG_STATUS)&GE_MSK_W_CMQ_FIFO_STATUS)>>11) < u8CMDCount)
    {
        if( g_emp_movie_in_play ) msleep(2) ; // Samuel, 090109
        if (0x0000== (0xFFF & wait_count))
        {
#ifdef RED_LION
            printk("[GE INFO] GE wait command queue\n");
#else
            printk("[GE INFO] GE wait command queue\n");
#endif
        }
        wait_count++;
#ifndef RED_LION
        MsOS_YieldTask();
#endif
    }
#endif
}

//-------------------------------------------------------------------------------------------------
static U32 GE_ReadReg(U32 addr)
{
	U16 u16NoFIFOMask;

    if(0x80 <= addr)
    {
        GE_WaitAvailableCMDQueue(32);
        return GE_Reg(addr);
    }
	
//    GE_WaitCmdEmpty();

    switch (addr)
    {//for registers which do not go through command queue
	case GE_REG_EN:
		u16NoFIFOMask = GE_VAL_EN_PE;
		break;
	case GE_REG_FMT_BLT:
		u16NoFIFOMask = ~(GE_VAL_EN_STRETCH_BITBLT|GE_VAL_EN_CLIPING_CHECKING|GE_VAL_EN_ITALIC_FONT|GE_VAL_EN_SRC_TILE|GE_VAL_EN_DST_TILE);
		break;
	case GE_REG_DBGMSG:
	case GE_REG_STBB_TH:
	case GE_REG_BIST_STAT:
	case GE_REG_STATUS:
	case GE_REG_VCMDQ_STAT:
	case GE_REG_MIU_PROT_LTH_L(0):
	case GE_REG_MIU_PROT_LTH_H(0):
	case GE_REG_MIU_PROT_HTH_L(0):
	case GE_REG_MIU_PROT_HTH_H(0):
	case GE_REG_MIU_PROT_LTH_L(1):
	case GE_REG_MIU_PROT_LTH_H(1):
	case GE_REG_MIU_PROT_HTH_L(1):
	case GE_REG_MIU_PROT_HTH_H(1):
	case PE_REG_PT_TAG:
	case GE_REG_VCMDQ_BASE0:
	case GE_REG_VCMDQ_BASE1:
		u16NoFIFOMask = 0xffff;
		break;
	case GE_REG_VCMDQ_SIZE:
		u16NoFIFOMask = 0x07;
		break;
	default:
		  u16NoFIFOMask = 0;
		  break;
    }

    if(0 == u16NoFIFOMask)
        return _u32RegBackup[addr];
    return (GE_Reg(addr)&u16NoFIFOMask)|(_u32RegBackup[addr]&~u16NoFIFOMask);
}

//-------------------------------------------------------------------------------------------------

GESTATUS MHal_GE_WaitGEFinish(U8 u8CMDCount)
{
#if WAIT_CMD_FIFO
    U32                 wait_count= 1;
    unsigned int tt ;
    u8CMDCount += 2 ; // Samuel, 090109
    u8CMDCount = (u8CMDCount > 32)? 32 : u8CMDCount;

//Note: In Titania, 1 empty count can save 2 commands.
//      And if the empty item only saves 1 commands, it is treated as empty;
//      therefore 15 cmmands will need (15+1)/2 empty items.
    u8CMDCount = ((u8CMDCount + 1)>>1);


    // check GE busy status to avoid command queue bug
    tt = 0 ;
    while (((GE_Reg(GE_REG_STATUS)&0x01))){
        if( g_emp_movie_in_play ) msleep(2) ; // Samuel, 090109
        tt++ ;
        if( (tt&0xFFFFFF)==0 )
            return GESTATUS_FAIL ;
    }

    while (((GE_Reg(GE_REG_STATUS)&GE_MSK_W_CMQ_FIFO_STATUS)>>11) < u8CMDCount)
    {
        if( g_emp_movie_in_play ) msleep(2) ; // Samuel, 090109
        if (0x0000== (0xFFF & wait_count))
        {
            printk("[GE INFO] GE wait command queue\n");
        }
        wait_count++;
#ifndef RED_LION
        MsOS_YieldTask();
#endif
    }
#endif

    return GESTATUS_SUCCESS;
}

void outGE_WaitAvailableCMDQueue(void){
    GE_WaitAvailableCMDQueue(1) ; // Samuel, 090109
}

static void GE_WriteReg(U32 u32addr, U32 u32val)
{
#if WAIT_CMD_FIFO
    // Wait GE command queue empty before fire to avoid GE hang
    if (GE_REG_CMD== u32addr)
    {
        GE_WaitAvailableCMDQueue(32);
    }

    if(0x80 > u32addr)
         _u32RegBackup[u32addr] = u32val;
		 
    GE_Reg(u32addr)=u32val;
#else
//    GE_WaitCmdNotFull();
    GE_Reg(u32addr)=u32val;
    if ((u32addr == GE_REG_CMD)&&(GE_Reg(GE_REG_EN)& GE_VAL_EN_PE))
    {
        while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
        {
            MsOS_YieldTask();
        }
    }
#endif
}

#if 0 // disable BLT

//-------------------------------------------------------------------------------------------------
static U32 BLT_ReadReg(U32 u32addr)
{
//    GE_WaitCmdEmpty();
    return BLT_Reg(u32addr);
}


//-------------------------------------------------------------------------------------------------
static void BLT_WriteReg(U32 u32addr, U32 u32val)
{
#if WAIT_CMD_FIFO
    BLT_Reg(u32addr)=u32val;
#else
    if ( (BLT_ReadReg(BLT_REG_EN)& BLT_VAL_EN_GE) )
    {
        while ( BLT_ReadReg(BLT_REG_STATUS)& BLT_VAL_FIFO_FULL)
        {
            //MsOS_DelayTask(0);
            MsOS_YieldTask();
        }
    }

//    GE_WaitCmdNotFull();
    BLT_Reg(u32addr)=u32val;
#endif

#ifdef DBGLOG
    // write to dram 0xa1600000
    if (_bOutFileLog == true)
    {
        *_pu16OutLogAddr = u32addr & 0xffff;
        _pu16OutLogAddr++;
        *_pu16OutLogAddr = u32val & 0xffff;
        _pu16OutLogAddr++;
        _u16LogCount += 2;
    }
#endif
}
#endif // #if 0 // disable BLT

//-------------------------------------------------------------------------------------------------
static U16 GE_CheckInClipWindow(U16 u16X1, U16 u16Y1, U16 u16X2, U16 u16Y2)
{
    if (((u16X1 < _u16PeClipX1) && (u16X2 < _u16PeClipX1)) ||
        ((u16X1 > _u16PeClipX2) && (u16X2 > _u16PeClipX2)) ||
        ((u16Y1 < _u16PeClipY1) && (u16Y2 < _u16PeClipY1)) ||
        ((u16Y1 > _u16PeClipY2) && (u16Y2 > _u16PeClipY2)))
    {
        return FALSE;
    }

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Description: for debug output register log use
// Arguments:   enable  -  true or false
//
// Return:      NONE
//
// Notes:       if any
//-------------------------------------------------------------------------------------------------
void MHal_GE_Debug_SetOutputLog(U16 enable)
{
#ifdef DBGLOG
     _bOutFileLog = enable;
     if (enable)
     {
        _pu16OutLogAddr = (U16 *)GE_LOG_ADR;
        memset(_pu16OutLogAddr, 0, 0x100000);
        //write header
        *_pu16OutLogAddr = 0xffff;
        _pu16OutLogAddr++;
        *_pu16OutLogAddr = 0x55AA;
        _pu16OutLogAddr++;
        _u16LogCount =2;
     }
     else
     {
        *_pu16OutLogAddr = 0xffff;
        _pu16OutLogAddr++;
        *_pu16OutLogAddr = _u16LogCount;
        _pu16OutLogAddr++;
        _u16LogCount = 0;
     }
#endif
}

static void GE_Enable_CmdQ(void)
{
    GE_WriteReg(GE_REG_FMT_BLT, (GE_VAL_EN_CMDQ | GE_VAL_EN_DYNAMIC_CLOCK));
    //GE_Reg(GE_REG_FMT_BLT) = GE_VAL_EN_CMDQ;
}


static void GE_Register_Reset(void)
{
/*
#if 1
    GE_WaitAvailableCMDQueue(32);
#else
    while (((GE_Reg(GE_REG_STATUS) & GE_VAL_BUSY))){
        MsOS_YieldTask();
    }
    while (((GE_Reg(GE_REG_STATUS) & 0xfc)>>2) < 32)
    {
        MsOS_YieldTask();
    }
#endif
*/

    // GE_WriteReg(GE_REG_EN, 0);
    GE_WriteReg(GE_REG_EN, GE_VAL_EN_PE);
#if PATCH_RD_CMD
    _u32Reg0hValue = GE_VAL_EN_PE;
#endif

    GE_WriteReg(GE_REG_STBB_TH, 0x08);

    //Set Line pattern to default.
    //GE_WriteReg(GE_REG_LPT, (GE_VAL_LINE_LAST | GE_VAL_LPT_RESET | (0x3F)));
    //GE_WriteReg(GE_REG_LPT, (GE_VAL_LINE_LAST | (0x3F)));
    _u32Reg60hFlag = 0;

}

//-------------------------------------------------------------------------------------------------
/// Begin PE Engine drawing, this function should be called before all PE drawing function,
/// and it will lock PE engine resource, reset all PE register and static variable.
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_BeginDraw(void)
{
    // U32 u32Value, u32Value1;
    Chip_Flush_Memory() ;

    GE_DEBUGINFO(printk( "MHal_GE_BeginDraw\n"));

#ifdef RED_LION
    //spin_lock(&GE_SpinLock); // LGE. jjab@lge.com
#else
    if (_s32GeMutex >= 0)
    {
        MsOS_ObtainMutex(_s32GeMutex, MSOS_WAIT_FOREVER);
    }
    else
    {
        GE_DEBUGINFO(printk( "MHal_GE_EndDraw: No availible Mutex Index.\n"));
        return GESTATUS_FAIL;
    }
#endif

#if DYNAMIC_POWER_ON_OFF
    MDrv_Power_ClockTurnOn(E_POWER_MODULE_GE);
#endif

    /*
    u32Value = BLT_ReadReg(BLT_REG_EN);
    if (u32Value & BLT_VAL_EN_GE)
    {
        u32Value1 = GE_ReadReg(GE_REG_STATUS);
        while (u32Value1 & GE_BLT_BUSY)
        {
            u32Value1 = GE_ReadReg(GE_REG_STATUS);
        }
        u32Value &= ~BLT_VAL_EN_GE;
        BLT_WriteReg(BLT_REG_EN, u32Value);
    }
    */

    GE_Register_Reset();

    return GESTATUS_SUCCESS;
}
//-------------------------------------------------------------------------------------------------
/// Initial PE engine
/// @return  None
//-------------------------------------------------------------------------------------------------
void MHal_GE_Init(void)
{
    U32 u32RegVal;

    GE_DEBUGINFO(printk( "MHal_GE_Init\n"));

#ifdef RED_LION
     //spin_lock_init(&GE_SpinLock); // LGE. jjab@lge.com
#else
    if (_s32GeMutex < 0)
    {
        _s32GeMutex = MsOS_CreateMutex(E_MSOS_FIFO, "GE_Mutex");
    }
#endif

#if MOVE_TO_USER_SPACE
#if DYNAMIC_POWER_ON_OFF
    MDrv_Power_ClockTurnOn(E_POWER_MODULE_GE);
#endif
#endif

    GE_Enable_CmdQ();

    GE_WriteReg(GE_REG_SCK_LTH0, *(U32*)&_u8TypeGE[0]);
    GE_WriteReg(GE_REG_SCK_LTH1, *(U32*)&_u8TypeGE[4]);
    GE_WriteReg(GE_REG_SCK_HTH0, *(U32*)&_u8TypeGE[8]);
    GE_WriteReg(GE_REG_SCK_HTH1, *(U32*)&_u8TypeGE[12]);

    /*mstar mingchia 2009.12.24
    For 1080P@60Hz component bandwidth issue, adjust ge access miu data length*/
    u32RegVal = GE_ReadReg(GE_REG_FMT_BLT);
    u32RegVal |= GE_VAL_EN_LENGTH_LIMIT;
    GE_WriteReg(GE_REG_FMT_BLT, u32RegVal);

    //printk("\n!!!!!!!!MHal_GE_Init adr2:0x%x", GE_ReadReg(GE_REG_DBGMSG));
    u32RegVal = GE_LENGTH_TH(GE_ReadReg(GE_REG_DBGMSG), 7);
    //printk("\n&&&&&&&&&&&&&&MHal_GE_Init adr2:0x%x", u32RegVal);
    GE_WriteReg(GE_REG_DBGMSG, u32RegVal);

#if DYNAMIC_POWER_ON_OFF
    while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
    {
#ifndef RED_LION
        MsOS_YieldTask();
#endif
    }
#if MOVE_TO_USER_SPACE
    MDrv_Power_ClockTurnOff(E_POWER_MODULE_GE);
#endif
#endif

#if GE_AUTO_ALIGN_SRC_BUFFER
    _u32SrcAddrPE = 0;
    _u32SrcPitchPE = 0;
    _u32SrcFmtPE = 0xFF;
    _u32OffsetSrc = 0;
#endif

    //_u32FrameWidth  = width;
    //_u32FrameHeight = height;
    //_u32FramePitch  = pitch;
    //MHal_GE_SetFrameBufferInfo(_u32FrameWidth, _u32FrameHeight, _u32FramePitch, fbFmt, GE_FRAMEBUFFER_ADR);
}


//-------------------------------------------------------------------------------------------------
/// End PE engine drawing (pair with MHal_GE_BeginDraw), this function should be called after
/// all PE drawing function. And it will release PE engine resource.
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_EndDraw(void)
{
#if DYNAMIC_POWER_ON_OFF
    GE_WaitAvailableCMDQueue(32);
    while (GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
    {
        MsOS_YieldTask();
    }
    MDrv_Power_ClockTurnOff(E_POWER_MODULE_GE);
#endif

    //not sure what to do!!!
#ifdef RED_LION
    //spin_unlock(&GE_SpinLock); // LGE. jjab@lge.com
#else
    if (_s32GeMutex >= 0)
    {
        MsOS_ReleaseMutex(_s32GeMutex);
    }
    else
    {
        GE_DEBUGINFO(printk( "MHal_GE_EndDraw: No availible Mutex Index.\n"));
        return GESTATUS_FAIL;
    }
#endif

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE Engine dither
/// @param  enable \b IN: true/false
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetDither(U16 enable)
{
    U32 u32Value;

    GE_DEBUGINFO(printk("MDrv_SetDither\n"));
    GE_WaitAvailableCMDQueue(4);

#if PATCH_RD_CMD
    u32Value = _u32Reg0hValue;
#else
    u32Value = GE_ReadReg(GE_REG_EN);
#endif

    if(enable)
    {
        u32Value |= GE_VAL_EN_DITHER;
    }
    else
    {
        u32Value &= ~GE_VAL_EN_DITHER;
    }

    GE_WriteReg(GE_REG_EN, u32Value);
#if PATCH_RD_CMD
    _u32Reg0hValue = u32Value;
#endif

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE source color key
/// @param enable   \b IN: true/false\n
///                     When enable is FALSE, do not care the other parameters.\n
/// @param opMode   \b IN: source color key mode
///                      The alpha channel does not matter\n
/// @param fmt      \b IN: source color key format
/// @param ps_color \b IN: pointer of source color key start (GE_RGB_COLOR or GE_BLINK_DATA depend on color format).\n
///                        For all RGB color, the color set as the ARGB8888 format.\n
///                        Each color component need to shift to high bit.\n
///                        Use ARGB1555 as the example, the source color key as the following:\n
///                        ARGB1555  --> ARRRRRGGGGGBBBBB                   (every character represents one bit)\n
///                        *ps_color --> A0000000RRRRR000GGGGG000BBBBB000   (every character represents one bit)\n\n
///                        For GE_FMT_I8 format, the index set to b component (ps_color->b = b).\n
///                        For GE_FMT_1BAAFGBG123433 foramt, the foramt set as the GE_BLINK_DATA.\n
/// @param pe_color \b IN: pointer of source color key end (GE_RGB_COLOR or GE_BLINK_DATA depend on color format).\n
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetSrcColorKey(U16 enable,
                                   GE_COLOR_KEY_MODE opMode,
                                   GE_Buffer_Format fmt,
                                   void *ps_color,
                                   void *pe_color)
{
    U32 u32Value, u32Value2;
    U16 u16Color0, u16Color1;

    GE_DEBUGINFO(printk("MHal_GE_SetSrcColorKey\n"));
    GE_WaitAvailableCMDQueue(8);

#if PATCH_RD_CMD
    u32Value = _u32Reg0hValue;
#else
    u32Value = GE_ReadReg(GE_REG_EN);
#endif

    if(enable)
    {
        u32Value |= GE_VAL_EN_SCK;

        // Color key threshold
        GE_ConvertRGB2DBFmt(fmt, (U32 *)ps_color, &u16Color0, &u16Color1);
        GE_WriteReg(GE_REG_SCK_LTH0, u16Color0);
        GE_WriteReg(GE_REG_SCK_LTH1, u16Color1);

        GE_ConvertRGB2DBFmt(fmt, (U32 *)pe_color, &u16Color0, &u16Color1);
        GE_WriteReg(GE_REG_SCK_HTH0, u16Color0);
        GE_WriteReg(GE_REG_SCK_HTH1, u16Color1);

        // Color op
        u32Value2 = GE_ReadReg(GE_REG_KEY_OP);
        u32Value2 = (u32Value2 & ~GE_VAL_SCK_OP_TRUE) | opMode;
        GE_WriteReg(GE_REG_KEY_OP, u32Value2);
    }
    else
    {
        u32Value &= ~(GE_VAL_EN_SCK);
    }

    GE_WriteReg(GE_REG_EN, u32Value);
#if PATCH_RD_CMD
    _u32Reg0hValue = u32Value;
#endif


    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE destination color key
/// @param enable   \b IN: true/false\n
///                     When enable is FALSE, do not care the other parameters.\n
/// @param opMode   \b IN: destination color key mode\n
///                      The alpha channel does not matter\n
/// @param fmt      \b IN: destination color key format
/// @param ps_color \b IN: pointer of destination color key start (GE_RGB_COLOR or GE_BLINK_DATA depend on color format).\n
///                        For all RGB color, the color set as the ARGB8888 format.\n
///                        Each color component need to shift to high bit.\n
///                        Use ARGB1555 as the example, the source color key as the following:\n
///                        ARGB1555  --> ARRRRRGGGGGBBBBB                   (every character represents one bit)\n
///                        *ps_color --> A0000000RRRRR000GGGGG000BBBBBB000  (every character represents one bit)\n\n
///                        For GE_FMT_I8 format, the index set to b component (ps_color->b = b).\n
///                        For GE_FMT_1BAAFGBG123433 foramt, the foramt set as the GE_BLINK_DATA.\n
/// @param pe_color \b IN: pointer of destination color key end (GE_RGB_COLOR or GE_BLINK_DATA depend on color format).\n
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetDstColorKey(U16 enable,
                                   GE_COLOR_KEY_MODE opMode,
                                   GE_Buffer_Format fmt,
                                   void *ps_color,
                                   void *pe_color)
{
    U32 u32Value, u32Value2;
    U16 u16Color0, u16Color1;

    GE_DEBUGINFO(printk("MHal_GE_SetDstColorKey\n"));
    GE_WaitAvailableCMDQueue(8);
#if PATCH_RD_CMD
    u32Value = _u32Reg0hValue;
#else
    u32Value = GE_ReadReg(GE_REG_EN);
#endif

    if(enable)
    {
        u32Value |= GE_VAL_EN_DCK;

        // Color key threshold
        GE_ConvertRGB2DBFmt(fmt, (U32 *)ps_color, &u16Color0, &u16Color1);
        GE_WriteReg(GE_REG_DCK_LTH0, u16Color0);
        GE_WriteReg(GE_REG_DCK_LTH1, u16Color1);

        GE_ConvertRGB2DBFmt(fmt, (U32 *)pe_color, &u16Color0, &u16Color1);
        GE_WriteReg(GE_REG_DCK_HTH0, u16Color0);
        GE_WriteReg(GE_REG_DCK_HTH1, u16Color1);

        // Color op
        u32Value2 = GE_ReadReg(GE_REG_KEY_OP);
        u32Value2 = (u32Value2 & ~GE_VAL_DCK_OP_TRUE) | (opMode<<1);
        GE_WriteReg(GE_REG_KEY_OP, u32Value2);
    }
    else
    {
        u32Value &= ~GE_VAL_EN_DCK;
    }
    GE_WriteReg(GE_REG_EN, u32Value);
#if PATCH_RD_CMD
    _u32Reg0hValue = u32Value;
#endif

    return GESTATUS_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
/// Set PE intensity : total 16 color Palette in PE
/// @param id \b IN: id of intensity
/// @param fmt \b IN: intensity color format (GE_FMT_ARGB8888 , GE_FMT_1BAAFGBG123433 or GE_FMT_I8)
/// @param pColor \b IN: pointer of intensity (GE_RGB_COLOR or GE_BLINK_DATA depend on color format)
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
/// @return GESTATUS_INVALID_INTENSITY_ID - Inavlid index (id >= 16)
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetIntensity(U32 id, GE_Buffer_Format fmt, U32 *pColor)
{
    U16 u16Color0, u16Color1;
    GE_DEBUGINFO(printk("MHal_GE_SetIntensity\n"));
    GE_WaitAvailableCMDQueue(5);

    if (id < GE_INTENSITY_NUM)
    {
        GE_ConvertRGB2DBFmt(fmt, (U32 *)pColor, &u16Color0, &u16Color1);
        GE_WriteReg(GE_REG_I0_C0 + (2*id), u16Color0);
        GE_WriteReg(GE_REG_I0_C1 + (2*id), u16Color1);
    }
    else
    {
        return GESTATUS_INVALID_INTENSITY_ID;
    }
    return GESTATUS_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
/// Set PE raster operation
/// @param enable \b IN: true/false
/// @param eRopMode \b IN: raster operation
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetROP2(U16 enable, GE_ROP2_OP eRopMode)
{
    U32 u32Value, u32Value2;

	GE_DEBUGINFO(printk("MHal_GE_SetROP2\n"));
    GE_WaitAvailableCMDQueue(5);

#if PATCH_RD_CMD
    u32Value = _u32Reg0hValue;
#else
    u32Value = GE_ReadReg(GE_REG_EN);
#endif

    if(enable)
    {
        u32Value |= GE_VAL_EN_ROP;

        u32Value2 = GE_ReadReg(GE_REG_ROP2);
        u32Value2 = (u32Value2 & ~GE_MSK_ROP2) | eRopMode;
        GE_WriteReg(GE_REG_ROP2, u32Value2);
    }
    else
    {
        u32Value &= ~GE_VAL_EN_ROP;
    }
    GE_WriteReg(GE_REG_EN, u32Value);
#if PATCH_RD_CMD
    _u32Reg0hValue = u32Value;
#endif

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE raster operation8
/// @param enable \b IN: true/false
/// @param eRopMode \b IN: raster operation
/// @param u8ConstAlpha \b IN: constant alpha
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetROP8(U16 enable, GE_ROP8_OP_t eRopMode, U8 u8ConstAlpha)
{
    U32 u32Value;
    U32 u32AblCoef = COEF_ONE;
    U32 u32DbAbl = ABL_FROM_ASRC;

#if PATCH_RD_CMD
    u32Value = _u32Reg0hValue;
#else
    u32Value = GE_ReadReg(GE_REG_EN);
#endif

    if(enable)
    {

      MHal_GE_SetROP2(FALSE, ROP2_OP_ZORE);

      switch(eRopMode){
        case ROP8_COPY:
        case ROP8_DVB_SRC:
            u32AblCoef = COEF_ONE;
            u32DbAbl = ABL_ASRCxCONST;
            break;

        case ROP8_XOR:
            MHal_GE_SetROP2(enable, ROP2_OP_PS_XOR_PD);
            u32DbAbl = ABL_FROM_ADST;
            MHal_GE_SetAlphaSrcFrom((GE_ALPHA_SRC_FROM)u32DbAbl);  // reg12
            GE_WriteReg(GE_REG_EN, u32Value);
            return GESTATUS_SUCCESS;
            break;

        case ROP8_ALPHA:
            u32AblCoef = COEF_ASRCxConst_DIV_2;
            u32DbAbl = ABL_FROM_ADST;
            break;

        case ROP8_TRANSPARENT:
        /*
            GE_WriteReg(GE_REG_DCK_HTH0, 0xFF); // dest color key high threshold
            GE_WriteReg(GE_REG_DCK_HTH1, 0xFF);
            GE_WriteReg(GE_REG_DCK_LTH0, 0x00); // dest color key low threshold
            GE_WriteReg(GE_REG_DCK_LTH1, 0x00);
            GE_WriteReg(GE_REG_KEY_OP, 0x00); // dest color key low threshold
            GE_WriteReg(GE_REG_EN, u32Value);
        */
            return GESTATUS_SUCCESS;
            break;

        case ROP8_DVB_CLEAR:
            u8ConstAlpha = 0x00;
            u32AblCoef = COEF_ONE;  // don't care here
            u32DbAbl = ABL_ASRCxCONST;
            break;
        case ROP8_DVB_SRC_OVER:
            u32AblCoef = COEF_SRC_OVER;
            u32DbAbl = ABL_ASRCxCONST_MINUS_ADSTxASRCxCONST_PLUS_ADST;
            break;

        case ROP8_DVB_DEST_OVER:
            u32AblCoef = COEF_DST_OVER;
            u32DbAbl = ABL_ASRCxCONST_MINUS_ADSTxASRCxCONST_PLUS_ADST;
            break;

        case ROP8_DVB_SRC_IN:
            u32AblCoef = COEF_ONE;
            u32DbAbl = ABL_ASRCxCONSTxADST;
            break;

        case ROP8_DVB_DEST_IN:
            u32AblCoef = COEF_ZERO;
            u32DbAbl = ABL_ASRCxCONSTxADST;
            break;
        case ROP8_DVB_SRC_OUT:
            u32AblCoef = COEF_ONE;
            u32DbAbl = ABL_ASRCxCONST_MINUS_ADSTxASRCxCONST;
            break;

        case ROP8_DVB_DEST_OUT:
            u32AblCoef = COEF_ZERO;
            u32DbAbl = ABL_ADST_MINUS_ADSTxASRCxCONST;
            break;

        default:
            printk("Unsupported ROP8 mode\n");
            break;

        }

        MHal_GE_SetAlphaBlending(u32AblCoef,u8ConstAlpha);  // reg11
        MHal_GE_SetAlphaSrcFrom((GE_ALPHA_SRC_FROM) u32DbAbl);  // reg12
    }
    else
    {
        u32Value &= ~GE_VAL_EN_ROP;
    }
    GE_WriteReg(GE_REG_EN, u32Value);
#if PATCH_RD_CMD
    _u32Reg0hValue = u32Value;
#endif

    return GESTATUS_SUCCESS;
}




GESTATUS MHal_GE_SetFramePtr(U32 gopidx, U32 gwinidx, U32 new_addr )
{
    U32 low_addr ;
    U32 high_addr ;
    U32 control, backup ;
    U32 fp_addr ;

	GE_DEBUGINFO(printk("MHal_GE_SetFramePtr\n"));	
    GE_WaitAvailableCMDQueue(32) ;

    low_addr = new_addr&0xFFFF ;
    high_addr = new_addr>>16 ;

    // stop update immediatly bit
    backup = control = (*(volatile unsigned int *)(REG_MIPS_BASE+(0x1FFE<<1))) ;

    // set bank
    control &= 0xFFFFFFF8 ;
    if( 0==gopidx ) // 4GGOP
        control |= 0x001 ;
    else if( 1==gopidx ) // 2GGOP
        control |= 0x003 ;
    else
        printk( "GOP not support\n" ) ;

    control &= (~0x100) ; // low bit 8 (update with VSync)

    (*(volatile unsigned int *)(REG_MIPS_BASE+(0x1FFE<<1))) = control ; // apply

    // update new front buffer address

    // select gwin
    fp_addr = 0x1F02 + gwinidx*0x40 ;
    (*(volatile unsigned int *)(REG_MIPS_BASE+(fp_addr<<1))) = low_addr ;
    (*(volatile unsigned int *)(REG_MIPS_BASE+((fp_addr+2)<<1))) = high_addr ;

    control |= 0x100 ; // high bit 8 (update with VSync)
    (*(volatile unsigned int *)(REG_MIPS_BASE+(0x1FFE<<1))) = control ;

    control &= (~0x100) ; // low bit 8 (update with VSync) again
    (*(volatile unsigned int *)(REG_MIPS_BASE+(0x1FFE<<1))) = control ; // point of update

    // resume the original value
    (*(volatile unsigned int *)(REG_MIPS_BASE+(0x1FFE<<1))) = backup ;

    return GESTATUS_SUCCESS;
}



 // Alan.Chen This buffer shoud be created by users themself.
#ifndef RED_LION
//-------------------------------------------------------------------------------------------------
/// Set PE Buffer info
/// @param addr \b IN: address : should be 128 bit aligned
/// @param width \b IN: width  : should be 8 bit aligned
/// @param height \b IN: height
/// @param fbFmt \b IN: color format\n
/// @return  buffer handle (if create buffer is failed then return NULL)
//-------------------------------------------------------------------------------------------------
PGE_BUFFER_INFO MHal_GE_CreateBuffer(U32 addr, U32 width, U32 height, GE_Buffer_Format fbFmt)
{
    PGE_BUFFER_INFO bufInfo = NULL;

    bufInfo = (GE_BUFFER_INFO *)MsOS_AllocateMemory(sizeof(GE_BUFFER_INFO),gs32CachedPoolID);

    if (bufInfo == NULL)
    {
        return NULL;
    }

    _GE_CHECK_BUFFER_ALIGN0(addr, fbFmt);

    bufInfo->u32Addr   = addr;
    bufInfo->u32Width  = width;
    bufInfo->u32Height = height;

    bufInfo->u32ColorFmt      = fbFmt;

    switch ( fbFmt )
    {
        case GE_FMT_I1 :
            bufInfo->u32Pitch = (width) >> 3;
            break;
        case GE_FMT_I2 :
            bufInfo->u32Pitch = (width<<1) >> 3;
            break;
        case GE_FMT_I4 :
            bufInfo->u32Pitch = (width<<2) >> 3;
            break;
        case GE_FMT_I8 :
            // @FIXME: Uranus pitch for pallette 8 is 1 byte alignment (Double check with Francis Tai
            bufInfo->u32Pitch= width;
            break;
        case GE_FMT_YUV422:
        case GE_FMT_1ABFGBG12355:
        case GE_FMT_RGB565 :
        case GE_FMT_ARGB1555 :
        case GE_FMT_ARGB4444 :
        // case GE_FMT_1BAAFGBG123433 :
#if GE_AUTO_ALIGN_SRC_BUFFER
            bufInfo->u32Pitch = width << 1;
#else
            bufInfo->u32Pitch = ((((width << 1)+15)>>4)<<4);
#endif
            break;
        case GE_FMT_ARGB8888 :
#if GE_AUTO_ALIGN_SRC_BUFFER
            bufInfo->u32Pitch = width << 2;
#else
            bufInfo->u32Pitch = ((((width << 2)+15)>>4)<<4);
#endif
            break;
        default :
            GE_ASSERT(0, printk("[GE DRV][%06d] Bad color format\n", __LINE__));
            break;
    }
    return bufInfo;
}

//-------------------------------------------------------------------------------------------------
/// delete PE Buffer info
/// @param bufinfo \b IN: buffer handle
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_INVALID_BUFF_INFO - Inavlid buffer info address
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_DeleteBuffer(PGE_BUFFER_INFO bufinfo)
{
    if (bufinfo != NULL)
    {
        MsOS_FreeMemory(bufinfo, gs32CachedPoolID);
        return GESTATUS_SUCCESS;
    }
    else
    {
        return GESTATUS_INVALID_BUFF_INFO;
    }
}
#endif

//-------------------------------------------------------------------------------------------------
/// Set PE Buffer info
/// @param addr \b IN: address : should be 128 bit aligned
/// @param width \b IN: width  : should be 8 bit aligned
/// @param height \b IN: height
/// @param fbFmt \b IN: color format\n
/// @return  buffer handle (if create buffer is failed then return NULL)
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_InitBufferInfo(PGE_BUFFER_INFO pBufInfo, U32 addr, U32 width, U32 height, GE_Buffer_Format fbFmt)
{
    if (pBufInfo == NULL)
    {
        return GESTATUS_FAIL;
    }

    _GE_CHECK_BUFFER_ALIGN0(addr, fbFmt);

    pBufInfo->u32Addr   = addr;
    pBufInfo->u32Width  = width;
    pBufInfo->u32Height = height;

    pBufInfo->u32ColorFmt      = fbFmt;

    switch ( fbFmt )
    {
        case GE_FMT_I1 :
            pBufInfo->u32Pitch = (width) >> 3;
            break;
        case GE_FMT_I2 :
            pBufInfo->u32Pitch = (width<<1) >> 3;
            break;
        case GE_FMT_I4 :
            pBufInfo->u32Pitch = (width<<2) >> 3;
            break;
        case GE_FMT_I8 :
            // @FIXME: Uranus pitch for pallette 8 is 1 byte alignment (Double check with Francis Tai
            pBufInfo->u32Pitch= width;
            break;
        case GE_FMT_YUV422:
        case GE_FMT_1ABFGBG12355:
        case GE_FMT_RGB565 :
        case GE_FMT_ARGB1555 :
        case GE_FMT_ARGB4444 :
        // case GE_FMT_1BAAFGBG123433 :
#if GE_AUTO_ALIGN_SRC_BUFFER
            pBufInfo->u32Pitch = width << 1;
#else
            pBufInfo->u32Pitch = ((((width << 1)+15)>>4)<<4);
#endif
            break;
        case GE_FMT_ARGB8888 :
#if GE_AUTO_ALIGN_SRC_BUFFER
            pBufInfo->u32Pitch = width << 2;
#else
            pBufInfo->u32Pitch = ((((width << 2)+15)>>4)<<4);
#endif
            break;
        default :
            GE_ASSERT(0, printk("[GE DRV][%06d] Bad color format\n", __LINE__));
            break;
    }

    return GESTATUS_SUCCESS;

}


//-------------------------------------------------------------------------------------------------
/// load PE font info
/// @param addr   \b IN: address
/// @param len    \b IN: total size of font
/// @param width  \b IN: width
/// @param height \b IN: height
/// @param pBBox  \b IN: pointer to bounding box
/// @image html BoundingBox.jpg "in_line & ini_dis"
/// @param fmt    \b IN: color format (only 3 font mode)
///                      GE_FMT_I1 :  I1 mode\n
///                      GE_FMT_I2 :  I2 mode\n
///                      GE_FMT_I4 :  I4 mode\n
/// @return  font handle (  fail to create handle will return ERR_HANDLE )
//-------------------------------------------------------------------------------------------------
FONTHANDLE MHal_GE_LoadFont(U32 addr,
                               U32 len,
                               U32 width,
                               U32 height,
                               GE_GLYPH_BBOX* pBBox,
                               GE_Buffer_Format fmt)
{
    U8 s8count;

    GE_DEBUGINFO( printk( "MHal_GE_LoadFont\n" ) );

#if !GE_AUTO_ALIGN_FONT
    if ( addr & 0xf )       // 128 bits aligned check
    {
        return ERR_HANDLE;
    }
#endif
    for (s8count = 0; s8count<MAX_FONT; s8count++)
    {
        if (_FontTable[s8count].inUsed == false)
            break;
    }

    if ( s8count == MAX_FONT)
    {
        return ERR_HANDLE;
    }


    if (fmt == GE_FMT_I4)
    {
        _FontTable[s8count].pitch    = ((width+1)*4) >> 3;
        _FontTable[s8count].offset   = _FontTable[s8count].pitch * height;
    }
    else if (fmt == GE_FMT_I2)
    {
        _FontTable[s8count].pitch    = ((width+3)*2) >> 3;
        _FontTable[s8count].offset   = _FontTable[s8count].pitch * height;

    }
    else if (fmt == GE_FMT_I1)
    {
        _FontTable[s8count].pitch    = (width+7) >> 3;
        _FontTable[s8count].offset   = _FontTable[s8count].pitch * height;
    }
    else
    {
        return ERR_HANDLE;
    }
#if !GE_AUTO_ALIGN_FONT
    _FontTable[s8count].offset = (((_FontTable[s8count].offset + 15)>>4)<<4);
#endif

    _FontTable[s8count].addr     = addr;
    _FontTable[s8count].len      = len;
    _FontTable[s8count].width    = width;
    _FontTable[s8count].height   = height;
    _FontTable[s8count].pBBox    = pBBox;
    _FontTable[s8count].fmt      = fmt;
    _FontTable[s8count].inUsed   = true;

    return s8count;
}

//-------------------------------------------------------------------------------------------------
/// free PE font info
/// @param handle \b IN: font handle
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_INVALID_FONT_HANDLE - Invalid font handle
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_FreeFont(FONTHANDLE handle)
{
    if ((handle < MAX_FONT) && (handle >= 0))
    {
        _FontTable[handle].inUsed = false;
        _FontTable[handle].pBBox = NULL;
        return GESTATUS_SUCCESS;
    }
    return GESTATUS_INVALID_FONT_HANDLE;
}

//-------------------------------------------------------------------------------------------------
/// Get PE font info
/// @param handle \b IN: font handle
/// @param pinfo \b OUT: font information.
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_INVALID_FONT_HANDLE - Invalid font handle
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_GetFontInfo(FONTHANDLE handle, GE_FONT_INFO* pinfo)
{
    if ((handle < MAX_FONT) && (handle >= 0))
    {
        pinfo->addr     = _FontTable[handle].addr;
        pinfo->height   = _FontTable[handle].height;
        pinfo->len      = _FontTable[handle].len;
        pinfo->offset   = _FontTable[handle].offset;
        pinfo->width    = _FontTable[handle].width;
        pinfo->pitch    = _FontTable[handle].pitch;
        pinfo->fmt      = _FontTable[handle].fmt;
        pinfo->pBBox    = _FontTable[handle].pBBox;

        return GESTATUS_SUCCESS;
    }
    return GESTATUS_INVALID_FONT_HANDLE;
}

//-------------------------------------------------------------------------------------------------
/// Get PE bitmap info
/// @param handle \b IN: bitmap handle
/// @param pinfo \b OUT: bitmap information.
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_INVALID_BMP_HANDLE - Invalid Bitmap handle
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_GetBitmapInfo(BMPHANDLE handle, GE_BITMAP_INFO* pinfo)
{
    if ((handle < MAX_BITMAP) && (handle >= 0))
    {
        pinfo->addr    = _BitmapTable[handle].addr;
        pinfo->len     = _BitmapTable[handle].len;
        pinfo->inUsed  = _BitmapTable[handle].inUsed;
        pinfo->width   = _BitmapTable[handle].width;
        pinfo->height  = _BitmapTable[handle].height;
        pinfo->pitch   = _BitmapTable[handle].pitch;
        pinfo->fmt     = _BitmapTable[handle].fmt;

        return GESTATUS_SUCCESS;
    }
    return GESTATUS_INVALID_BMP_HANDLE;
}


static void GE_TextOut(FONTHANDLE fhandle,
                       U8 *pindex,
                       U32 strwidth,
                       GE_POINT_t *ppoint,
                       U32 width,
                       S32 dis)
{
    U32 i, u32Addr;
    U16 *pu16TmpIndex;
    U8  *pu8TmpIndex;
    U32 u32Offset;
    U32 u32Spoint = 0;
    U32 u32FlatAddr;
#if GE_AUTO_ALIGN_FONT
    U8 *pTempBuf=NULL;
#endif

	GE_DEBUGINFO(printk("MHal_GE_TextOut\n"));
    u32Offset   = _FontTable[fhandle].offset;
    u32FlatAddr = _FontTable[fhandle].addr;

    i = 0;
    if (strwidth == 2)
    {
        pu16TmpIndex =(U16 *)pindex;
        while (*pu16TmpIndex != '\0')
        {

            if( ( *pu16TmpIndex ) == 0 )
            {
                break;
            }

            GE_WaitAvailableCMDQueue(8);
            if(i==0)
            {
                u32Spoint = ppoint->x;
            }
            else
            {
                u32Spoint += (width - dis);
            }
            GE_WriteReg(GE_REG_PRI_V0_X, u32Spoint);
            GE_WriteReg(GE_REG_PRI_V1_X, u32Spoint + width - 1);

            u32Addr = (u32FlatAddr + ( *pu16TmpIndex ) * u32Offset);    // sb addr
#if GE_AUTO_ALIGN_FONT
            if(u32Addr & 0x0F)      // Need 128-bit alignment.
            {
                // If the character address is not 16-byte alignment, allocate a 16-byte memory to do it.
                if(!pTempBuf)
                {
                    pTempBuf = MsOS_AllocateMemory(_FontTable[fhandle].pitch *
                                                       _FontTable[fhandle].height,
                                                   gs32NonCachedPoolID);
                    if(!pTempBuf)
                    {
                        return;     // error
                    }
                }
                memcpy(pTempBuf,
                       (void*)u32Addr,
                       _FontTable[fhandle].pitch*_FontTable[fhandle].height);
                GE_WriteReg(GE_REG_SB_BASE0, (U32)pTempBuf & 0xffff);
                GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr((U32)pTempBuf) >> 16);
            }
            else
            {
                GE_WriteReg(GE_REG_SB_BASE0, u32Addr & 0xffff);
                GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr(u32Addr) >> 16);
            }
#else
            GE_WriteReg(GE_REG_SB_BASE0, u32Addr & 0xffff);
            GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr(u32Addr) >> 16);
#endif

    // @FIXME
    // check GE_REG_SB_DB_MODE

            _GE_SIMPLE_BB_CHECK();
            GE_WriteReg(GE_REG_CMD, (GE_VAL_PRIM_BITBLT|_u32Reg60hFlag));
            _GE_SIMPLE_BB_CHECK();

            i++;
            pu16TmpIndex++;
#if GE_AUTO_ALIGN_FONT
            if(u32Addr & 0x0F)
            {
                GE_WaitCmdEmpty();
                while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
                {
                    MsOS_YieldTask();
                }
            }
#endif
        }
    }
    else
    {
        pu8TmpIndex =pindex;
        while ((*pu8TmpIndex != '\0'))
        {

            if( ( *pu8TmpIndex ) == 0 )
            {
                break;
            }

            GE_WaitAvailableCMDQueue(8);
            if(i==0)
            {
                u32Spoint = ppoint->x;
            }
            else
            {
                u32Spoint += (width - dis);
            }
            GE_WriteReg(GE_REG_PRI_V0_X, u32Spoint);
            GE_WriteReg(GE_REG_PRI_V1_X, u32Spoint + width - 1);

            u32Addr = (u32FlatAddr + ( *pu8TmpIndex ) * u32Offset);     // sb addr

#if GE_AUTO_ALIGN_FONT
            if(u32Addr & 0x0F)      // Need 128-bit alignment.
            {
                // If the character address is not 16-byte alignment, allocate a 16-byte memory to do it.
                if(!pTempBuf)
                {
                    pTempBuf = MsOS_AllocateMemory(_FontTable[fhandle].pitch *
                                                       _FontTable[fhandle].height,
                                                   gs32NonCachedPoolID);
                    if(!pTempBuf)
                    {
                        return;     // error
                    }
                }
                memcpy(pTempBuf,
                       (void*)u32Addr,
                       _FontTable[fhandle].pitch*_FontTable[fhandle].height);
                GE_WriteReg(GE_REG_SB_BASE0, (U32)pTempBuf & 0xffff);
                GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr((U32)pTempBuf) >> 16);
            }
            else
            {
                GE_WriteReg(GE_REG_SB_BASE0, u32Addr & 0xffff);
                GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr(u32Addr) >> 16);
            }
#else
            GE_WriteReg(GE_REG_SB_BASE0, u32Addr & 0xffff);
            GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr(u32Addr) >> 16);
#endif

    // @FIXME
    // check GE_REG_SB_DB_MODE

            _GE_SIMPLE_BB_CHECK();
            GE_WriteReg(GE_REG_CMD, (GE_VAL_PRIM_BITBLT|_u32Reg60hFlag));
            _GE_SIMPLE_BB_CHECK();

            i++;
            pu8TmpIndex++;
#if GE_AUTO_ALIGN_FONT
            if(u32Addr & 0x0F)
            {
                GE_WaitCmdEmpty();
                while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
                {
                    MsOS_YieldTask();
                }
            }
#endif
        }
    }

#if GE_AUTO_ALIGN_FONT
    if(pTempBuf)
    {
        GE_WaitCmdEmpty();
        while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
        {
            MsOS_YieldTask();
        }

        MsOS_FreeMemory(pTempBuf, gs32NonCachedPoolID);
    }
#endif
}

void dummyRegWrite( void ){
    GE_WriteReg(GE_REG_STBB_WIDTH, GE_ReadReg(GE_REG_STBB_WIDTH));
}

static GESTATUS GE_TextOutEx(FONTHANDLE fhandle,
                             U8 *pindex,
                             U32 strwidth,
                             GE_TEXT_OUT_INFO *pfmt)
{
    U32 u32Width, u32Height;
    S32 dis;
    U32 u32Value, u32Value2;
    GE_POINT_t point;

    GE_DEBUGINFO(printk( "MHal_GE_TextOut\n" ));
    GE_WaitAvailableCMDQueue(20);

    if (!_FontTable[fhandle].inUsed)
    {
        return GESTATUS_INVALID_FONT_HANDLE;
    }

    if (pindex == NULL)
    {
        return GESTATUS_INVALID_PARAMETERS;
    }

    if (!GE_CheckInClipWindow(pfmt->dstblk.x, pfmt->dstblk.y,
                              pfmt->dstblk.x+pfmt->dstblk.width-1, pfmt->dstblk.y+pfmt->dstblk.height-1))
    {
        return GESTATUS_FAIL;
    }

    u32Width  = _FontTable[fhandle].width;
    u32Height = _FontTable[fhandle].height;
    GE_WriteReg(GE_REG_STBB_WIDTH, u32Width);
    GE_WriteReg(GE_REG_STBB_HEIGHT, u32Height);


    //---------------------------------------------------------------------------
    // Set source font format
    //---------------------------------------------------------------------------
    u32Value = GE_ReadReg(GE_REG_SB_DB_MODE);
    u32Value = (u32Value & ~GE_MSK_SB_FMT) | (_FontTable[fhandle].fmt); // one-bit format
    GE_WriteReg(GE_REG_SB_DB_MODE, u32Value);


    //---------------------------------------------------------------------------
    // Font data pitch
    //---------------------------------------------------------------------------
    GE_WriteReg(GE_REG_SB_PIT, (_FontTable[fhandle].pitch));

    //---------------------------------------------------------------------------
    // BLT scale to 1
    //---------------------------------------------------------------------------
    u32Value2 = GE_ReadReg(GE_REG_FMT_BLT);

    if(pfmt->flag & GEFONT_FLAG_SCALE)
    {
		S32 s32Temp1, s32Temp2;
		U32 u32Value3, u32ValueFix = u32Width;;

		if ( pfmt->dstblk.width < (u32Width >> 1) )
			u32ValueFix = u32Width - 1;
			
        u32Value = (U16)Divide2Fixed((U16)u32ValueFix, pfmt->dstblk.width, 5, 12) ; //sc
        GE_WriteReg(GE_REG_STBB_DX, u32Value & 0xffff);
		s32Temp1 = u32ValueFix - pfmt->dstblk.width;
		s32Temp2 = 2* pfmt->dstblk.width;
		s32Temp1 = s32Temp1 % s32Temp2;
        u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
		if ( u32Value & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DX_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DX_MSB;
		}
		GE_WriteReg(GE_REG_STBB_INIT_DX, u32Value3);

		if ( pfmt->dstblk.height < (u32Height >> 1) )
			u32ValueFix = u32Height - 1;
		else
			u32ValueFix = u32Height;
			
        u32Value = (U16)Divide2Fixed((U16)u32ValueFix, pfmt->dstblk.height, 5, 12) ; //sc
        GE_WriteReg(GE_REG_STBB_DY, u32Value & 0xffff);
		s32Temp1 = u32ValueFix - pfmt->dstblk.height;
		s32Temp2 = 2* pfmt->dstblk.height;
		s32Temp1 = s32Temp1 % s32Temp2;
        u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
		if ( u32Value & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DY_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DY_MSB;
		}
		GE_WriteReg(GE_REG_STBB_INIT_DY, u32Value3);

        //scale = (U16)((float)(u32Width-1) * ((float)pbmpfmt->width / (float)u32Width));//TODO
        //u32Scale = (((U32)pfmt->dstblk.width << 5) / u32Width * (u32Width-1)) >> 5; //sc
        //u32Width = u32Scale;  //pbmpfmt->width;
        u32Width = pfmt->dstblk.width;

        //scale = (U16)((float)(u32Height-1) * ((float)pbmpfmt->height / (float)u32Height));//TODO
        //u32Scale = (((U32)pfmt->dstblk.height << 5) / u32Height * (u32Height-1)) >> 5; //sc
        //u32Height = u32Scale;  //pbmpfmt->height;
        u32Height= pfmt->dstblk.height;

        u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
        _u32Reg60hFlag |= GE_VAL_STBB_NEAREST;

    }
    else
    {
        GE_WriteReg(GE_REG_STBB_DX, 0x1000);
        GE_WriteReg(GE_REG_STBB_DY, 0x1000);
        GE_WriteReg(GE_REG_STBB_INIT_DX, 0);
        GE_WriteReg(GE_REG_STBB_INIT_DY, 0);
        u32Value2 &= ~GE_VAL_EN_STRETCH_BITBLT;
        _u32Reg60hFlag &= ~GE_VAL_STBB_NEAREST;
    }
    GE_WriteReg(GE_REG_FMT_BLT, u32Value2);

    //---------------------------------------------------------------------------
    // Coordinate
    //---------------------------------------------------------------------------
    GE_WriteReg(GE_REG_PRI_V0_Y, pfmt->dstblk.y);
    GE_WriteReg(GE_REG_PRI_V1_Y, pfmt->dstblk.y + u32Height - 1);

    GE_WriteReg(GE_REG_PRI_V2_X, 0);
    GE_WriteReg(GE_REG_PRI_V2_Y, 0);

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_X_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_X, _FontTable[fhandle].width - 1);
    }

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_Y_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_Y, _FontTable[fhandle].height - 1);
    }

    //---------------------------------------------------------------------------
    // Compact
    //---------------------------------------------------------------------------
    if(pfmt->flag & GEFONT_FLAG_COMPACT)
    {
        dis = pfmt->dis;
    }
    else if (pfmt->flag & GEFONT_FLAG_GAP)
    {
        dis = (0-pfmt->dis);
    }
    else
    {
        dis = 0;
    }

    //---------------------------------------------------------------------------
    // Real drawing
    //---------------------------------------------------------------------------

    point.x = pfmt->dstblk.x;
    point.y = pfmt->dstblk.y;
    GE_TextOut( fhandle, pindex, strwidth, &point, u32Width, dis );

    return GESTATUS_SUCCESS;
}




static GESTATUS GE_CharacterOut(GE_CHAR_INFO*  pChar, GE_TEXT_OUT_INFO *pfmt)
{
    U32 u32Width, u32Height;
    //U32 dis;
    U32 u32Value, u32Value2;
    //GE_POINT_t point;
#if GE_AUTO_ALIGN_FONT
    U8 *pTempBuf=NULL;
#endif

    GE_DEBUGINFO(printk( "MHal_GE_TextOutEx\n" ));
    GE_WaitAvailableCMDQueue(22);

    u32Width  = pChar->width;       //pChar->u32Width;
    u32Height = pChar->height;      //pChar->u32Height;

    if (!GE_CheckInClipWindow(pfmt->dstblk.x, pfmt->dstblk.y, (pfmt->dstblk.x + u32Width - 1), pfmt->dstblk.y + u32Height - 1))
    {
        return GESTATUS_FAIL;
    }

    GE_WriteReg(GE_REG_STBB_WIDTH, u32Width);
    GE_WriteReg(GE_REG_STBB_HEIGHT, u32Height);

    //---------------------------------------------------------------------------
    // Set source font format
    //---------------------------------------------------------------------------
    u32Value = GE_ReadReg(GE_REG_SB_DB_MODE);
    u32Value = (u32Value & ~GE_MSK_SB_FMT) | (pChar->fmt); // one-bit format
    GE_WriteReg(GE_REG_SB_DB_MODE, u32Value);


    //---------------------------------------------------------------------------
    // Font data start address & pitch
    //---------------------------------------------------------------------------
#if GE_AUTO_ALIGN_FONT
    if(pChar->addr % 16)   // Need 128-bit alignment.
    {
        // If the bitmap address is not 16-byte alignment, allocate a 16-byte memory to do it.
        pTempBuf = MsOS_AllocateMemory(pChar->pitch * pChar->height, gs32NonCachedPoolID);
        if(!pTempBuf)
        {
            return GESTATUS_NON_ALIGN_PITCH;
        }
        memcpy(pTempBuf, (void*)pChar->addr, pChar->pitch*u32Height);
        GE_WriteReg(GE_REG_SB_BASE0, (U32)pTempBuf & 0xffff);
        GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr((U32)pTempBuf) >> 16);
        GE_WriteReg(GE_REG_SB_PIT, (pChar->pitch));
    }
    else
    {
        GE_WriteReg(GE_REG_SB_BASE0, pChar->addr & 0xffff);
        GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr(pChar->addr) >> 16);
        GE_WriteReg(GE_REG_SB_PIT, (pChar->pitch));
    }
#else
    GE_WriteReg(GE_REG_SB_BASE0, pChar->addr & 0xffff);
    GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr(pChar->addr) >> 16);
    GE_WriteReg(GE_REG_SB_PIT, (pChar->pitch));
#endif

    //---------------------------------------------------------------------------
    // BLT scale to 1
    //---------------------------------------------------------------------------
    u32Value2 = GE_ReadReg(GE_REG_FMT_BLT);

    if(pfmt->flag & GEFONT_FLAG_SCALE)
    {
		S32 s32Temp1, s32Temp2;
		U32 u32Value3, u32ValueFix = u32Width;

		if ( pfmt->dstblk.width < (u32Width >> 1) )
			u32ValueFix = u32Width - 1;
			
        u32Value = (U16)Divide2Fixed((U16)u32ValueFix, pfmt->dstblk.width, 5, 12) ; //sc
        GE_WriteReg(GE_REG_STBB_DX, u32Value & 0xffff);
		s32Temp1 = u32ValueFix - pfmt->dstblk.width;
		s32Temp2 = 2* pfmt->dstblk.width;
		s32Temp1 = s32Temp1 % s32Temp2;
		u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
		if ( u32Value & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DX_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DX_MSB;
		}
		GE_WriteReg(GE_REG_STBB_INIT_DX, u32Value3);

		if ( pfmt->dstblk.height < (u32Height >> 1) )
			u32ValueFix = u32Height - 1;
		else
			u32ValueFix = u32Height;
			
        u32Value = (U16)Divide2Fixed((U16)u32ValueFix, pfmt->dstblk.height, 5, 12) ; //sc
        GE_WriteReg(GE_REG_STBB_DY, u32Value & 0xffff);
		s32Temp1 = u32ValueFix - pfmt->dstblk.height;
		s32Temp2 = 2* pfmt->dstblk.height;
		s32Temp1 = s32Temp1 % s32Temp2;
		u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
		if ( u32Value & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DY_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DY_MSB;
		}
		GE_WriteReg(GE_REG_STBB_INIT_DY, u32Value3);

        u32Width = pfmt->dstblk.width;
        u32Height= pfmt->dstblk.height;

        u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
        _u32Reg60hFlag |= GE_VAL_STBB_NEAREST;
    }
    else
    {
        GE_WriteReg(GE_REG_STBB_DX, 0x1000);
        GE_WriteReg(GE_REG_STBB_DY, 0x1000);
        GE_WriteReg(GE_REG_STBB_INIT_DX, 0);
        GE_WriteReg(GE_REG_STBB_INIT_DY, 0);
        u32Value2 &= ~GE_VAL_EN_STRETCH_BITBLT;
        _u32Reg60hFlag &= ~GE_VAL_STBB_NEAREST;
    }
    GE_WriteReg(GE_REG_FMT_BLT, u32Value2);

    //---------------------------------------------------------------------------
    // Coordinate
    //---------------------------------------------------------------------------
    GE_WriteReg(GE_REG_PRI_V0_X, pfmt->dstblk.x);
    GE_WriteReg(GE_REG_PRI_V0_Y, pfmt->dstblk.y);
    GE_WriteReg(GE_REG_PRI_V1_X, pfmt->dstblk.x + u32Width - 1);
    GE_WriteReg(GE_REG_PRI_V1_Y, pfmt->dstblk.y + u32Height - 1);

    //GE_WriteReg(GE_REG_PRI_V2_X, 0);
    GE_WriteReg(GE_REG_PRI_V2_X, pChar->Hoffset);
    GE_WriteReg(GE_REG_PRI_V2_Y, pChar->Voffset);

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_X_NEG)
    {
        //GE_WriteReg(GE_REG_PRI_V2_X, pChar->u32Width - 1);
        GE_WriteReg(GE_REG_PRI_V2_X, pChar->width - 1);
    }

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_Y_NEG)
    {
        //GE_WriteReg(GE_REG_PRI_V2_Y, pChar->u32Height - 1);
        GE_WriteReg(GE_REG_PRI_V2_Y, pChar->height - 1);
    }
    //---------------------------------------------------------------------------
    // Real drawing
    //---------------------------------------------------------------------------

    // @FIXME
    // check GE_REG_SB_DB_MODE

    _GE_SIMPLE_BB_CHECK();
    GE_WriteReg(GE_REG_CMD, (GE_VAL_PRIM_BITBLT|_u32Reg60hFlag));
    _GE_SIMPLE_BB_CHECK();

#if GE_AUTO_ALIGN_FONT
    if(pTempBuf)
    {
        GE_WaitCmdEmpty();
        while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
        {
            MsOS_YieldTask();
        }

        MsOS_FreeMemory(pTempBuf, gs32NonCachedPoolID);
    }
#endif

    return GESTATUS_SUCCESS;
}


GESTATUS MHal_GE_TextOut(FONTHANDLE fhandle, U8 *pindex, U32 strwidth, GE_TEXT_OUT_INFO *pfmt)
{
    GE_DEBUGINFO(printk( "MHal_GE_TextOut\n" ));

    if ((pfmt->flag & GEFONT_FLAG_VARWIDTH) &&
        (_FontTable[fhandle].pBBox!= NULL))
    {
        U32 i;
        U16 *pu16TmpIndex;
        U8  *pu8TmpIndex;
        S8 dis;
        GE_CHAR_INFO  Char;
        GE_TEXT_OUT_INFO fmt;
        GE_FONT_INFO* pFont = &_FontTable[fhandle];
        U32 u32PosX = 0;
        U32 u32Width = 0;
        U32 u32Height = 0;

        if (!pFont->inUsed)
        {
            return GESTATUS_INVALID_FONT_HANDLE;
        }
        if (pindex == NULL)
        {
            return GESTATUS_INVALID_PARAMETERS;
        }

        if(pfmt->flag & GEFONT_FLAG_COMPACT)
        {
            dis = pfmt->dis;
        }
        else if (pfmt->flag & GEFONT_FLAG_GAP)
        {
            dis = (0-pfmt->dis);
        }
        else
        {
            dis = 0;
        }
        memcpy((void*)&fmt, pfmt, sizeof(GE_TEXT_OUT_INFO));
        Char.fmt = pFont->fmt;
        Char.Voffset = 0;
        Char.height = pFont->height;
        Char.pitch = pFont->pitch;

        i = 0;
        if (strwidth == 2)
        {
            pu16TmpIndex =(U16 *)pindex;
            while (*pu16TmpIndex != '\0')
            {
                if( ( *pu16TmpIndex ) == 0 )
                {
                    break;
                }

                if (i == 0)
                {
                    u32PosX = pfmt->dstblk.x;
                }
                if (fmt.flag & GEFONT_FLAG_SCALE)
                {
                    u32Width = pfmt->dstblk.width;
                    u32Height = pfmt->dstblk.height;
                }
                else
                {
                    u32Width = pFont->pBBox[*pu16TmpIndex].u8Width;
                    u32Height = pFont->height;
                }
                fmt.dstblk.x = u32PosX;
                fmt.dstblk.width = u32Width;
                fmt.dstblk.height = u32Height;

                Char.addr = pFont->addr + ((*pu16TmpIndex)*pFont->offset);
                Char.Hoffset = pFont->pBBox[*pu16TmpIndex].u8X0;
                Char.width = pFont->pBBox[*pu16TmpIndex].u8Width;

                GE_CharacterOut(&Char, &fmt);

                u32PosX = u32PosX + u32Width - dis;
                i++;
                pu16TmpIndex++;
            }
        }
        else
        {
            pu8TmpIndex =(U8*)pindex;
            while (*pu8TmpIndex != '\0')
            {
                if( ( *pu8TmpIndex ) == 0 )
                {
                    break;
                }

                if (i == 0)
                {
                    u32PosX = pfmt->dstblk.x;
                }
                if (fmt.flag & GEFONT_FLAG_SCALE)
                {
                    u32Width = pfmt->dstblk.width;
                    u32Height = pfmt->dstblk.height;
                }
                else
                {
                    u32Width = pFont->pBBox[*pu8TmpIndex].u8Width;
                    u32Height = pFont->height;
                }
                fmt.dstblk.x = u32PosX;
                fmt.dstblk.width = u32Width;
                fmt.dstblk.height = u32Height;

                Char.addr = pFont->addr + ((*pu8TmpIndex)*pFont->offset);
                Char.Hoffset = pFont->pBBox[*pu8TmpIndex].u8X0;
                Char.width = pFont->pBBox[*pu8TmpIndex].u8Width;

                GE_CharacterOut(&Char, &fmt);

                u32PosX = u32PosX + u32Width - dis;
                i++;
                pu16TmpIndex++;
            }

        }
        return GESTATUS_SUCCESS;
    }
    else
    {
        return GE_TextOutEx(fhandle, pindex, strwidth, pfmt);
    }
}
//-------------------------------------------------------------------------------------------------
/// Query PE text display length
/// @param fhandle \b IN: font handle
/// @param pu8index \b IN: pointer of character index array
/// @param u32strwidth \b IN: character index length(1:ONE_BYTE_CHAR, 2: TWO_BYTE_CHAR)
/// @param pfmt \b IN: pointer to text out info
/// @param pu32DispLength \b OUT: pointer to display length.
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_INVALID_FONT_HANDLE - Invalid font handle
/// @return GESTATUS_INVALID_PARAMETERS - Inavlid input parameters
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_QueryTextDispLength(FONTHANDLE fhandle, U8 *pu8index, U32 u32strwidth,
                                        GE_TEXT_OUT_INFO *pfmt, U32* pu32DispLength)
{
    GE_FONT_INFO* pFont = &_FontTable[fhandle];
    U16* pu16TmpIndex;
    U8* pu8TmpIndex;
    U32 u32Length = 0;
    S8 dis;

    if (!pFont->inUsed)
    {
        return GESTATUS_INVALID_FONT_HANDLE;
    }
    if (pu8index == NULL)
    {
        return GESTATUS_INVALID_PARAMETERS;
    }

    if(pfmt->flag & GEFONT_FLAG_COMPACT)
    {
        dis = pfmt->dis;
    }
    else if (pfmt->flag & GEFONT_FLAG_GAP)
    {
        dis = (0-pfmt->dis);
    }
    else
    {
        dis = 0;
    }


    if (u32strwidth == 1)
    {
        pu8TmpIndex = (U8*)pu8index;
        while(*pu8TmpIndex != 0)
        {
            if (pfmt->flag & GEFONT_FLAG_SCALE)
            {
                u32Length = u32Length + pfmt->dstblk.width - dis;
            }
            else
            {
                if ((pfmt->flag & GEFONT_FLAG_VARWIDTH) &&
                    (pFont->pBBox != NULL))
                {
                    u32Length = u32Length + pFont->pBBox[*pu8TmpIndex].u8Width - dis;
                }
                else
                {
                    u32Length = u32Length + pFont->width - dis;
                }
            }
            pu8TmpIndex++;
        }
        *pu32DispLength = u32Length + dis;
    }
    else if (u32strwidth == 2)
    {
        pu16TmpIndex = (U16*)pu8index;
        while(*pu16TmpIndex != 0)
        {
            if (pfmt->flag & GEFONT_FLAG_SCALE)
            {
                u32Length = u32Length + pfmt->dstblk.width - dis;
            }
            else
            {
                if ((pfmt->flag & GEFONT_FLAG_VARWIDTH) &&
                    (pFont->pBBox != NULL))
                {
                    u32Length = u32Length + pFont->pBBox[*pu16TmpIndex].u8Width - dis;
                }
                else
                {
                    u32Length = u32Length + pFont->width - dis;
                }
            }
            pu16TmpIndex++;
        }
        *pu32DispLength = u32Length + dis;
    }
    else
    {
        *pu32DispLength = u32Length;
        return GESTATUS_INVALID_PARAMETERS;
    }

    return GESTATUS_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
/// Force PE text out one character without load font.
/// @param pChar \b IN: pointer to character information
/// @param pfmt \b IN: pointer to text out info
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_CharacterOut(GE_CHAR_INFO*  pChar, GE_TEXT_OUT_INFO *pfmt)
{
    return GE_CharacterOut(pChar, pfmt);
}



//-------------------------------------------------------------------------------------------------
/// Force PE draw line
/// @param pline \b IN: pointer to line info
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_DrawLine(GE_DRAW_LINE_INFO *pline)
{
    U32  u32Start, u32End;
    U32  u32Width, u32Height, u32Ratio;
    U32  u32Value, u32Value2;
    U32  u32X1, u32X2, u32Y1, u32Y2;
    S32  i;
    S16  s16Dif;
    U16 bYMajor = false;
    U16 bInverse = false;
    U16 u16Color0, u16Color1;
    GE_RGB_COLOR color_s, color_e;
    GE_BLINK_DATA  *blinkData = NULL;

    GE_DEBUGINFO( printk("MHal_GE_DrawLine\n"));
    GE_WaitAvailableCMDQueue(24);

    if ((!GE_CheckInClipWindow(pline->x1, pline->y1, pline->x2, pline->y2)) && (pline->width == 1))
    {
        return GESTATUS_FAIL;
    }

    GE_WriteReg(GE_REG_LPT, (GE_VAL_LINE_LAST | GE_VAL_LPT_RESET | (0x3F)));
    GE_WriteReg(GE_REG_LPT, (GE_VAL_LINE_LAST | (0x3F)));

    u32X1 = pline->x1;
    u32X2 = pline->x2;
    u32Y1 = pline->y1;
    u32Y2 = pline->y2;

    u32Width  = u32X2 - u32X1 ;
    u32Height = u32Y2 - u32Y1 ;
    color_s = pline->colorRange.color_s;
    color_e = pline->colorRange.color_e;

    if ( u32Y1 > u32Y2 )
    {
        u32Height = u32Y1 - u32Y2;
    }

    if ( u32X1 > u32X2 )
    {
        u32Width  = u32X1 - u32X2;
    }


    u32Value2  = GE_VAL_PRIM_LINE;

    if( (u32Height >= u32Width)||(u32Width==0))
    {
        bYMajor = true;
    }

    if (bYMajor)
    {
        if (pline->x1 >= pline->x2)
        {
            if (!GE_CheckInClipWindow(pline->x1+pline->width, pline->y1, pline->x2, pline->y2))
            {
                return GESTATUS_FAIL;
            }
        }
        else
        {
            if (!GE_CheckInClipWindow(pline->x1, pline->y1, pline->x2+pline->width, pline->y2))
            {
                return GESTATUS_FAIL;
            }
        }
    }
    else
    {
        if (pline->y1 >= pline->y2)
        {
            if (!GE_CheckInClipWindow(pline->x1, pline->y1+pline->width, pline->x2, pline->y2))
            {
                return GESTATUS_FAIL;
            }
        }
        else
        {
            if (!GE_CheckInClipWindow(pline->x1, pline->y1, pline->x2, pline->y2+pline->width))
            {
                return GESTATUS_FAIL;
            }
        }
    }
/*
    if ( bYMajor )
    {
        if ( u32X1  >  u32X2)
        {
            u32X1 = pline->x2;
            u32X2 = pline->x1;
            u32Y1 = pline->y2;
            u32Y2 = pline->y1;
            bInverse = true;
        }
        if ( u32Y1  >  u32Y2)
            u32Value2 |= GE_VAL_DRAW_DST_DIR_X_NEG;
    }
    else
    {
        if ( u32Y1  >  u32Y2)
        {
            u32X1 = pline->x2;
            u32X2 = pline->x1;
            u32Y1 = pline->y2;
            u32Y2 = pline->y1;
            bInverse = true;
        }
        if ( u32X1  >  u32X2)
            u32Value2 |= GE_VAL_DRAW_DST_DIR_X_NEG;
    }
*/
    if ( bYMajor )
    {
        if ( u32X1  >  u32X2)
        {
            bInverse = true;
        }
        if ( u32Y1  >  u32Y2)
            u32Value2 |= GE_VAL_DRAW_DST_DIR_X_NEG; // @FIXME: Richard: should be Y NEG???
    }
    else
    {
        if ( u32Y1  >  u32Y2)
        {
            bInverse = true;
        }
        if ( u32X1  >  u32X2)
            u32Value2 |= GE_VAL_DRAW_DST_DIR_X_NEG;
    }

    if ((u32Width==0)||(u32Height==0))
    {
        u32Value = 0;
    }
    else
    {

        if ( bYMajor )
        {
            if (bInverse)
            {
                u32Value = (0x4000 - (U16)Divide2Fixed(u32Width, u32Height, 1, 12)) << 1; //sc
            }
            else
            {
                u32Value = (U16)Divide2Fixed(u32Width, u32Height, 1, 12) << 1; //sc
            }
            bInverse = FALSE;
        }
        else
        {
            if (bInverse)
            {
                u32Value = (0x4000 - (U16)Divide2Fixed(u32Height, u32Width, 1, 12)) << 1; //sc
            }
            else
            {
                u32Value = (U16)Divide2Fixed(u32Height, u32Width, 1, 12) << 1; //sc
            }
            bInverse = FALSE;
        }
    }

    if ( bYMajor )
    {
        GE_WriteReg(GE_REG_LINE_DTA, (u32Value & GE_MSK_LINE_DTA )|GE_VAL_LINE_Y_MAJOR);
    }
    else
    {
        GE_WriteReg(GE_REG_LINE_DTA, (u32Value & GE_MSK_LINE_DTA ));
    }

    // Start color
    // @FIXME: Richard uses GE_FMT_1ABFGBG12355 instead
    if (GE_FMT_1ABFGBG12355!= pline->fmt)
    // if (pline->fmt != GE_FMT_1BAAFGBG123433)
    {
        GE_ConvertRGB2DBFmt(pline->fmt, (U32*)&color_s, &u16Color0, &u16Color1);
    }
    else
    {
#if 0
        // @FIXME: Richard uses GE_FMT_1ABFGBG12355 instead
        //          1 A B Fg Bg
        //          1 2 3  5  5
        //
        //          1 B A A Fg Bg
        //          1 2 3 4  3  3

        blinkData =(GE_BLINK_DATA *)&color_s;
        u16Color0 = ((((blinkData->background&0x7) | ((blinkData->foreground&0x7)<<3))<<2) | ((blinkData->Bits.BlinkAlpha&0xf)<<12));
        u16Color1 = (0xff00 | ((((blinkData->Bits.Blink&0x3)<<3) | (blinkData->Bits.Alpha&0x7))<<3));
#else
        blinkData=      (GE_BLINK_DATA *)&color_s;
        u16Color0=      (0x1F & blinkData->background) |
                        ((0x1F & blinkData->foreground) << 8);
        u16Color1=      (0x7 & blinkData->Bits.Blink) |
                        ((0x3 & blinkData->Bits.Alpha) << 3) |
                        BIT8;
#endif
    }
    GE_WriteReg(GE_REG_PRI_BG_ST, u16Color0);
    GE_WriteReg(GE_REG_PRI_RA_ST, u16Color1);

    if (bYMajor) u32Ratio = u32Height;
    else   u32Ratio = u32Width;

    if(pline->flag & GELINE_FLAG_COLOR_GRADIENT)
    {
        if(bInverse)
        {
            GE_ConvertRGB2DBFmt(pline->fmt, (U32*)&color_e, &u16Color0, &u16Color1);
            GE_WriteReg(GE_REG_PRI_BG_ST, u16Color0);
            GE_WriteReg(GE_REG_PRI_RA_ST, u16Color1);
            //GE_WriteReg(GE_REG_PRI_BG_ST, (pline->color2.b & 0xff) | ((pline->color2.g & 0xff) << 8));
            //GE_WriteReg(GE_REG_PRI_RA_ST, (pline->color2.r & 0xff) | ((pline->color2.a & 0xff) << 8));
        }

        if(bInverse)
        {
            s16Dif = color_s.r - color_e.r;
        }
        else
        {
            s16Dif = color_e.r - color_s.r;
        }
        u32Value = Divide2Fixed(s16Dif, u32Ratio , 7, 12);
        GE_WriteReg(GE_REG_PRI_R_DX0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_R_DX1, u32Value >> 16);

        if(bInverse)
        {
            s16Dif = color_s.g - color_e.g;
        }
        else
        {
            s16Dif = color_e.g - color_s.g;
        }
        u32Value = Divide2Fixed(s16Dif, u32Ratio, 7, 12);
        GE_WriteReg(GE_REG_PRI_G_DX0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_G_DX1, u32Value >> 16);

        if(bInverse)
        {
            s16Dif = color_s.b - color_e.b;
        }
        else
        {
            s16Dif = color_e.b - color_s.b;
        }
        u32Value = Divide2Fixed(s16Dif, u32Ratio, 7, 12);
        GE_WriteReg(GE_REG_PRI_B_DX0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_B_DX1, u32Value >> 16);

        if(bInverse)
        {
            s16Dif = color_s.a - color_e.a;
        }
        else
        {
            s16Dif = color_e.a - color_s.a;
        }
        u32Value = Divide2Fixed(s16Dif, u32Ratio, 4, 11);
        GE_WriteReg(GE_REG_PRI_A_DX, u32Value & 0xffff);

        u32Value2 |= GE_VAL_LINE_GRADIENT;
    }

    GE_ASSERT(0x7FF>= u32X1, printk("[GE DRV][%06d] out of range\n", __LINE__));
    GE_ASSERT(0x7FF>= u32X2, printk("[GE DRV][%06d] out of range\n", __LINE__));
    GE_ASSERT(0x7FF>= u32Y1, printk("[GE DRV][%06d] out of range\n", __LINE__));
    GE_ASSERT(0x7FF>= u32Y2, printk("[GE DRV][%06d] out of range\n", __LINE__));
    GE_WriteReg(GE_REG_PRI_V0_X, u32X1);
    GE_WriteReg(GE_REG_PRI_V1_X, u32X2);
    GE_WriteReg(GE_REG_PRI_V0_Y, u32Y1);
    GE_WriteReg(GE_REG_PRI_V1_Y, u32Y2);

    if(bYMajor)
    {
        u32Start = u32X1;
        u32End = u32X2;
        for(i=0;i<pline->width;i++)
        {
            GE_WaitAvailableCMDQueue(8);
            GE_ASSERT(0x7FF>= u32Start, printk("[GE DRV][%06d] out of range\n", __LINE__));
            GE_ASSERT(0x7FF>= u32End, printk("[GE DRV][%06d] out of range\n", __LINE__));
            GE_WriteReg(GE_REG_LENGTH, u32Height);
            GE_WriteReg(GE_REG_PRI_V0_X, u32Start);
            GE_WriteReg(GE_REG_PRI_V1_X, u32End);
            GE_WriteReg(GE_REG_CMD, u32Value2|_u32Reg60hFlag);
            u32Start++;
            u32End++;
        }
    }
    else
    {
        u32Start = u32Y1;
        u32End = u32Y2;
        for (i=0;i<pline->width;i++)
        {
            GE_WaitAvailableCMDQueue(8);
            GE_ASSERT(0x7FF>= u32Start, printk("[GE DRV][%06d] out of range\n", __LINE__));
            GE_ASSERT(0x7FF>= u32End, printk("[GE DRV][%06d] out of range\n", __LINE__));
            GE_WriteReg(GE_REG_LENGTH, u32Width);
            GE_WriteReg(GE_REG_PRI_V0_Y, u32Start);
            GE_WriteReg(GE_REG_PRI_V1_Y, u32End);
            GE_WriteReg(GE_REG_CMD, u32Value2|_u32Reg60hFlag);
            u32Start++;
            u32End++;
        }
    }

    return GESTATUS_SUCCESS;
}

GESTATUS MHal_GE_DrawOval(GE_OVAL_FILL_INFO* pOval)
{
    S32 x, y, c_x, c_y;
    S32 Xchange, Ychange;
    S32 EllipseError;
    S32 TwoASquare, TwoBSquare;
    S32 StoppingX, StoppingY;
    U32 Xradius, Yradius;
    U32 u32Value2 = 0;
    U16 u16Color0, u16Color1;
    GE_BLINK_DATA  *blinkData = NULL;
	
	GE_DEBUGINFO(printk("MHal_GE_DrawOval\n"));
    if (!GE_CheckInClipWindow(pOval->dstBlock.x, pOval->dstBlock.y, pOval->dstBlock.x+pOval->dstBlock.width-1, pOval->dstBlock.y+pOval->dstBlock.height-1))
    {
        return GESTATUS_FAIL;
    }

    GE_WaitAvailableCMDQueue(8);

    Xradius = (pOval->dstBlock.width - pOval->u32LineWidth*2) / 2;
    Yradius = (pOval->dstBlock.height - pOval->u32LineWidth*2) / 2;

    /* center of ellipse */
    //c_x = pOval->dstBlock.x + Xradius + pOval->u32LineWidth/2;
    //c_y = pOval->dstBlock.y + Yradius + pOval->u32LineWidth/2;
    c_x = pOval->dstBlock.x + Xradius + pOval->u32LineWidth;
    c_y = pOval->dstBlock.y + Yradius + pOval->u32LineWidth;

    TwoASquare = 2*Xradius*Xradius;
    TwoBSquare = 2*Yradius*Yradius;

    /*1st set of points*/
    x = Xradius-1;  /*radius zero == draw nothing*/
    y = 0;

    Xchange = Yradius*Yradius*(1-2*Xradius);
    Ychange = Xradius*Xradius;

    EllipseError = 0;

    StoppingX = TwoBSquare*Xradius;
    StoppingY = 0;

    GE_WriteReg(GE_REG_LPT, (GE_VAL_LINE_LAST | GE_VAL_LPT_RESET | (0x3F)));
    GE_WriteReg(GE_REG_LPT, (GE_VAL_LINE_LAST | (0x3F)));

    u32Value2  = GE_VAL_PRIM_LINE;
    GE_WriteReg(GE_REG_LINE_DTA, 0);
    // @FIXME: Richard uses GE_FMT_1ABFGBG12355 instead
    if (GE_FMT_1ABFGBG12355!= pOval->fmt)
    // if (pOval->fmt != GE_FMT_1BAAFGBG123433)
    {
        GE_ConvertRGB2DBFmt(pOval->fmt, (U32*)&pOval->color, &u16Color0, &u16Color1);
    }
    else
    {
#if 0
        blinkData =(GE_BLINK_DATA *)&pOval->blink_data;
        u16Color0 = ((((blinkData->background&0x7) | ((blinkData->foreground&0x7)<<3))<<2) | ((blinkData->Bits.BlinkAlpha&0xf)<<12));
        u16Color1 = (0xff00 | ((((blinkData->Bits.Blink&0x3)<<3) | (blinkData->Bits.Alpha&0x7))<<3));
#else
        blinkData =(GE_BLINK_DATA *)&pOval->blink_data;
        u16Color0=      (0x1F & blinkData->background) |
                        ((0x1F & blinkData->foreground) << 8);
        u16Color1=      (0x7 & blinkData->Bits.Blink) |
                        ((0x3 & blinkData->Bits.Alpha) << 3) |
                        BIT8;
#endif
    }
    GE_WriteReg(GE_REG_PRI_BG_ST, u16Color0);
    GE_WriteReg(GE_REG_PRI_RA_ST, u16Color1);




    /*Plot 2 ellipse scan lines for iteration*/
    while (StoppingX > StoppingY)
    {
        GE_WaitAvailableCMDQueue(16);

        GE_WriteReg(GE_REG_PRI_V0_X, c_x - x);
        GE_WriteReg(GE_REG_PRI_V1_X, c_x + x);
        GE_WriteReg(GE_REG_PRI_V0_Y, c_y + y);
        GE_WriteReg(GE_REG_PRI_V1_Y, c_y + y);
        GE_WriteReg(GE_REG_LENGTH, 2*x);
        GE_WriteReg(GE_REG_CMD, u32Value2|_u32Reg60hFlag);

        GE_WriteReg(GE_REG_PRI_V0_X, c_x - x);
        GE_WriteReg(GE_REG_PRI_V1_X, c_x + x);
        GE_WriteReg(GE_REG_PRI_V0_Y, c_y - y);
        GE_WriteReg(GE_REG_PRI_V1_Y, c_y - y);
        GE_WriteReg(GE_REG_LENGTH, 2*x);
        GE_WriteReg(GE_REG_CMD, u32Value2|_u32Reg60hFlag);

        ++y;
        StoppingY    += TwoASquare;
        EllipseError += Ychange;
        Ychange      += TwoASquare;
        if (( 2*EllipseError + Xchange) > 0)
        {
            --x;
            StoppingX    -= TwoBSquare;
            EllipseError += Xchange;
            Xchange      += TwoBSquare;
        }
    }

    /*2nd set of points*/
    x = 0;
    y = Yradius-1;  /*radius zero == draw nothing*/
    Xchange = Yradius*Yradius;
    Ychange = Xradius*Xradius*(1-2*Yradius);
    EllipseError = 0;
    StoppingX = 0;
    StoppingY = TwoASquare*Yradius;

    /*Plot 2 ellipse scan lines for iteration*/
    while (StoppingX < StoppingY)
    {
        GE_WaitAvailableCMDQueue(16);

        GE_WriteReg(GE_REG_PRI_V0_X, c_x - x);
        GE_WriteReg(GE_REG_PRI_V1_X, c_x + x);
        GE_WriteReg(GE_REG_PRI_V0_Y, c_y + y);
        GE_WriteReg(GE_REG_PRI_V1_Y, c_y + y);
        GE_WriteReg(GE_REG_LENGTH, 2*x);
        GE_WriteReg(GE_REG_CMD, u32Value2|_u32Reg60hFlag);

        GE_WriteReg(GE_REG_PRI_V0_X, c_x - x);
        GE_WriteReg(GE_REG_PRI_V1_X, c_x + x);
        GE_WriteReg(GE_REG_PRI_V0_Y, c_y - y);
        GE_WriteReg(GE_REG_PRI_V1_Y, c_y - y);
        GE_WriteReg(GE_REG_LENGTH, 2*x);
        GE_WriteReg(GE_REG_CMD, u32Value2|_u32Reg60hFlag);

        ++x;
        StoppingX    += TwoBSquare;
        EllipseError += Xchange;
        Xchange      += TwoBSquare;
        if ((2*EllipseError + Ychange) > 0)
        {
            --y;
            StoppingY    -= TwoASquare;
            EllipseError += Ychange;
            Ychange      += TwoASquare;
        }
    }

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Force PE rectangle fill
/// @param pfillblock \b IN: pointer to block info
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_RectFill(GE_RECT_FILL_INFO *pfillblock)
{
    S16 s16Dif;
    U32 u32Value, u32Value2;
    GE_RGB_COLOR color_s, color_e;
    U16 u16Color0, u16Color1;
    GE_BLINK_DATA  *blinkData = NULL;


    GE_DEBUGINFO( printk("MHal_GE_RectFill\n"));

    if((pfillblock->dstBlock.width == 0) ||(pfillblock->dstBlock.height==0))
    {
        GE_DEBUGINFO(printk("MHal_GE_RectFill error!! width or height equal 0!!\n"));
        return FALSE;
    }

    if (!GE_CheckInClipWindow(pfillblock->dstBlock.x, pfillblock->dstBlock.y,
                              pfillblock->dstBlock.x+pfillblock->dstBlock.width-1,
                              pfillblock->dstBlock.y+pfillblock->dstBlock.height-1))
    {
        return GESTATUS_FAIL;
    }

    GE_WaitAvailableCMDQueue(20);

    //u32Value  = GE_ReadReg(GE_REG_EN);
    //u32Value |= GE_VAL_EN_PE;
    //GE_WriteReg(GE_REG_EN, u32Value);

    GE_WriteReg(GE_REG_PRI_V0_X, pfillblock->dstBlock.x);
    GE_WriteReg(GE_REG_PRI_V0_Y, pfillblock->dstBlock.y);
    GE_WriteReg(GE_REG_PRI_V1_X, pfillblock->dstBlock.x + pfillblock->dstBlock.width - 1);
    GE_WriteReg(GE_REG_PRI_V1_Y, pfillblock->dstBlock.y + pfillblock->dstBlock.height - 1);

    // Start color
    color_s = pfillblock->colorRange.color_s;
    color_e = pfillblock->colorRange.color_e;

    // @FIXME: Richard uses GE_FMT_1ABFGBG12355 instead
    if (GE_FMT_1ABFGBG12355!= pfillblock->fmt)
    // if (pfillblock->fmt != GE_FMT_1BAAFGBG123433)
    {
        GE_ConvertRGB2DBFmt(pfillblock->fmt, (U32*)&color_s, &u16Color0, &u16Color1);
    }
    else
    {
#if 0
        blinkData =(GE_BLINK_DATA *)&color_s;
        u16Color0 = ((((blinkData->background&0x7) | ((blinkData->foreground&0x7)<<3))<<2) | ((blinkData->Bits.BlinkAlpha&0xf)<<12));
        u16Color1 = (0xff00 | ((((blinkData->Bits.Blink&0x3)<<3) | (blinkData->Bits.Alpha&0x7))<<3));
#else
        blinkData=      (GE_BLINK_DATA *)&color_s;
        u16Color0=      (0x1F & blinkData->background) |
                        ((0x1F & blinkData->foreground) << 8);
        u16Color1=      (0x7 & blinkData->Bits.Blink) |
                        ((0x3 & blinkData->Bits.Alpha) << 3) |
                        BIT8;
#endif
    }
    GE_WriteReg(GE_REG_PRI_BG_ST, u16Color0);
    GE_WriteReg(GE_REG_PRI_RA_ST, u16Color1);


    GE_WriteReg(GE_REG_PRI_A_DY, 0);

    // @FIXME
    // check GE_REG_SB_DB_MODE

    u32Value2 = GE_VAL_PRIM_RECTANGLE;

    if((pfillblock->flag & GERECT_FLAG_COLOR_GRADIENT_Y) == GERECT_FLAG_COLOR_GRADIENT_Y)
    {
        s16Dif = color_e.r - color_s.r;
        u32Value = Divide2Fixed(s16Dif, pfillblock->dstBlock.height - 1, 7, 12);
        GE_WriteReg(GE_REG_PRI_R_DY0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_R_DY1, u32Value >> 16);

        s16Dif = color_e.g - color_s.g;
        u32Value = Divide2Fixed(s16Dif, pfillblock->dstBlock.height - 1, 7, 12);
        GE_WriteReg(GE_REG_PRI_G_DY0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_G_DY1, u32Value >> 16);

        s16Dif = color_e.b - color_s.b;
        u32Value = Divide2Fixed(s16Dif, pfillblock->dstBlock.height - 1, 7, 12);
        GE_WriteReg(GE_REG_PRI_B_DY0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_B_DY1, u32Value >> 16);

        s16Dif = color_e.a - color_s.a;
        u32Value = Divide2Fixed(s16Dif, pfillblock->dstBlock.height - 1, 4, 11);
        GE_WriteReg(GE_REG_PRI_A_DY, u32Value & 0xffff);

        u32Value2 |= GE_VAL_RECT_GRADIENT_V;
    }
    if((pfillblock->flag & GERECT_FLAG_COLOR_GRADIENT_X) == GERECT_FLAG_COLOR_GRADIENT_X)
    {
        s16Dif = color_e.r - color_s.r;
        u32Value = Divide2Fixed(s16Dif, pfillblock->dstBlock.width - 1, 7, 12);
        GE_WriteReg(GE_REG_PRI_R_DX0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_R_DX1, u32Value >> 16);

        s16Dif = color_e.g - color_s.g;
        u32Value = Divide2Fixed(s16Dif, pfillblock->dstBlock.width - 1, 7, 12);
        GE_WriteReg(GE_REG_PRI_G_DX0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_G_DX1, u32Value >> 16);

        s16Dif = color_e.b - color_s.b;
        u32Value = Divide2Fixed(s16Dif, pfillblock->dstBlock.width - 1, 7, 12);
        GE_WriteReg(GE_REG_PRI_B_DX0, u32Value & 0xffff);
        GE_WriteReg(GE_REG_PRI_B_DX1, u32Value >> 16);

        s16Dif = color_e.a - color_s.a;
        u32Value = Divide2Fixed(s16Dif, pfillblock->dstBlock.width - 1, 4, 11);
        GE_WriteReg(GE_REG_PRI_A_DX, u32Value & 0xffff);

        u32Value2 |= GE_VAL_RECT_GRADIENT_H;
    }

    // @FIXME
    // check GE_REG_SB_DB_MODE

    GE_WriteReg(GE_REG_CMD, u32Value2|_u32Reg60hFlag);

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Reset PE line pattern
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_Line_Pattern_Reset(void)
{
    U32 u32Value;
	
	GE_DEBUGINFO(printk("MHal_GE_LinePatternReset\n"));	
    GE_WaitAvailableCMDQueue(5);

    u32Value  = GE_ReadReg(GE_REG_LPT) | GE_VAL_LPT_RESET;
    GE_WriteReg(GE_REG_LPT, u32Value);

    u32Value  = GE_ReadReg(GE_REG_LPT) & (~GE_VAL_LPT_RESET);
    GE_WriteReg(GE_REG_LPT, u32Value);

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE line pattern
/// @param enable \b IN: true/false
/// @param linePattern \b IN: p0-0x3F one bit represent draw(1) or not draw(0)
/// @param repeatFactor \b IN: 0 : repeat once, 1 : repeat twice, 2: repeat 3, 3: repeat 4
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_Set_Line_Pattern(U16 enable, U8 linePattern, U8 repeatFactor)
{
    U32 u32Value, u32Value2;
	
	GE_DEBUGINFO(printk("MHal_GE_SetLinePattern\n"));
    GE_WaitAvailableCMDQueue(6);

#if PATCH_RD_CMD
    u32Value  = _u32Reg0hValue;
#else
    u32Value  = GE_ReadReg(GE_REG_EN);
#endif

    if (enable)
    {
        u32Value  |= GE_VAL_EN_LPT;
        u32Value2 = ((linePattern & GE_MSK_LP) | ((repeatFactor << 6) & GE_MSK_LPT_FACTOR) | GE_VAL_LPT_RESET);
        GE_WriteReg(GE_REG_LPT, u32Value2);
    }
    else
    {
        u32Value  &= ~GE_VAL_EN_LPT;
        GE_WriteReg(GE_REG_LPT, (GE_VAL_LINE_LAST | GE_VAL_LPT_RESET | (0x3F)));
        GE_WriteReg(GE_REG_LPT, (GE_VAL_LINE_LAST | (0x3F)));
    }
    GE_WriteReg(GE_REG_EN, u32Value);
#if PATCH_RD_CMD
    _u32Reg0hValue = u32Value;
#endif

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// load PE bitmap info
/// @param addr   \b IN: address
/// @param len    \b IN: total size of bitmap
/// @param width  \b IN: width
/// @param height \b IN: height
/// @param fmt    \b IN: color format (only support 6 color format)
///                     GE_FMT_I8    \n
///                     GE_FMT_RGB565\n
///                     GE_FMT_ARGB1555\n
///                     GE_FMT_ARGB4444\n
///                     GE_FMT_1BAAFGBG123433\n
///                     GE_FMT_ARGB8888\n
/// @return  bitmap handle (  fail to create handle will return ERR_HANDLE )
//-------------------------------------------------------------------------------------------------
BMPHANDLE MHal_GE_LoadBitmap(U32 addr, U32 len, U32 width, U32 height, GE_Buffer_Format fmt)
{
    int count;


    GE_DEBUGINFO( printk( "MHal_GE_LoadBitmap\n" ) );

#if !GE_AUTO_ALIGN_SRC_BUFFER
/* - MaxCC20080215
    if (addr & 0xf)         // 128 bits aligned check
// . MaxCC20080215 */
// + MaxCC20080215
    if (addr & 0x3)         // 64 bits aligned check
// . MaxCC20080215 */
    {
        return ERR_HANDLE;
    }
#endif

#if 0
    if( ( fmt == GE_FMT_I1 ) || ( fmt == GE_FMT_I2 ) || ( fmt == GE_FMT_I4 ) )
    {
        return ERR_HANDLE;
    }
#endif

    for (count = 0; count<MAX_BITMAP; count++ )
    {
        if (_BitmapTable[count].inUsed == false)
            break;
    }

    if ( count == MAX_BITMAP)
    {
        return ERR_HANDLE;
    }


    if( fmt == GE_FMT_ARGB8888 ) //E_GOP_COLOR_ARGB8888 16-bit align
    {
        _BitmapTable[count].pitch = width << 2;
    }
    else if( fmt == GE_FMT_I8 ) //E_GOP_COLOR_I8
    {
        _BitmapTable[count].pitch = width ;
    }
    else if( fmt == GE_FMT_I4 ) //E_GOP_COLOR_I4
    {
        _BitmapTable[count].pitch = width >> 1 ;
    }
    else if( fmt == GE_FMT_I2 ) //E_GOP_COLOR_I2
    {
        _BitmapTable[count].pitch = width >> 2 ;
    }
    else if( fmt == GE_FMT_I1 ) //E_GOP_COLOR_I1
    {
        _BitmapTable[count].pitch = width >> 3 ;
    }
    else // E_GOP_COLOR_RGB555 , E_GOP_COLOR_RGB565, E_GOP_COLOR_ARGB4444, E_GOP_COLOR_BLINK
    {
        _BitmapTable[count].pitch = width << 1;
    }

#if !GE_AUTO_ALIGN_SRC_BUFFER
    if (!((GE_FMT_I1 == fmt) || (GE_FMT_I2 == fmt) || (GE_FMT_I4 == fmt) || (GE_FMT_I8 == fmt)))
    {
    _BitmapTable[count].pitch = (((_BitmapTable[count].pitch + 15)>>4)<<4);
    }
#endif

    _BitmapTable[count].addr = addr;
    _BitmapTable[count].len = len;
    _BitmapTable[count].width = width;
    _BitmapTable[count].height = height;
    _BitmapTable[count].fmt = fmt;
    _BitmapTable[count].inUsed = true;

    return  count;
}

//-------------------------------------------------------------------------------------------------
/// free PE bitmap info
/// @param handle \b IN: bitmap handle
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_INVALID_BMP_HANDLE - Invalid bitmap handle
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_FreeBitmap(BMPHANDLE handle)
{
    if ((handle < MAX_BITMAP) && (handle >= 0))
    {
        _BitmapTable[handle].inUsed = false;
        return GESTATUS_SUCCESS;
    }
    return GESTATUS_INVALID_BMP_HANDLE;
}


//-------------------------------------------------------------------------------------------------
/// Set PE Bit blt
/// @param drawbuf \b IN: pointer to drawbuf info
/// @param drawflag \b IN: draw flag \n
///                  GEDRAW_FLAG_DEFAULT \n
///                  GEDRAW_FLAG_SCALE \n
///                  GEDRAW_FLAG_DUPLICAPE \n
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_BitBlt(GE_DRAW_RECT *drawbuf,U32 drawflag)
{
    U32 u32Width, u32Height;
    U32 u32Value, u32Value2;
#if GE_AUTO_ALIGN_SRC_BUFFER
    U8 *pTempBuf=NULL;
#endif

    //fixed by Samuel Huang(Mstar) and updated by wgkwak(LGE)
    if((!drawbuf->dstblk.width)||(!drawbuf->dstblk.height)||(!drawbuf->srcblk.width)||(!drawbuf->srcblk.height)) {
        GE_DEBUGINFO(printk("GE bilblt: zero condition!!!!!\n"));
        return GESTATUS_FAIL;
    }

    GE_DEBUGINFO( printk( "MHal_GE_BitBlt\n" ) );
    if (!GE_CheckInClipWindow(drawbuf->dstblk.x, drawbuf->dstblk.y,
                              drawbuf->dstblk.x+drawbuf->dstblk.width-1,
                              drawbuf->dstblk.y+drawbuf->dstblk.height-1))
    {
        return GESTATUS_FAIL;
    }
    GE_WaitAvailableCMDQueue(20);

    u32Value2 = GE_ReadReg(GE_REG_FMT_BLT);

    u32Width  = drawbuf->srcblk.width;
    u32Height = drawbuf->srcblk.height;

#if GE_AUTO_ALIGN_SRC_BUFFER
    if((_u32SrcAddrPE % 16) ||
       ((_u32SrcPitchPE % 16) &&
        (_u32SrcFmtPE != GE_VAL_I1) &&
        (_u32SrcFmtPE != GE_VAL_I2) &&
        (_u32SrcFmtPE != GE_VAL_I4)))
    {
        // If the bitmap pitch is not 16-byte alignment, allocate a 16-byte memory to do it.
        U8 *pSrcBuf;
        U32 u32PitchNew;
        U32 i;

        u32PitchNew = ALIGN_16(_u32SrcPitchPE);
        pTempBuf = MsOS_AllocateMemory(u32PitchNew * u32Height, gs32NonCachedPoolID);
        if(!pTempBuf)
        {
            return GESTATUS_NON_ALIGN_PITCH;
        }
        pSrcBuf = (U8*)_u32SrcAddrPE;
        for(i=0;i<u32Height;i++)
        {
            memcpy(&pTempBuf[i*u32PitchNew],
                   &pSrcBuf[i*_u32SrcPitchPE]+_u32OffsetSrc,
                   _u32SrcPitchPE);
        }
        // Set source address
        GE_WriteReg(GE_REG_SB_BASE0, (U32)pTempBuf & 0xffff);
        GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr((U32)pTempBuf) >> 16);

        // Set source pitch
        GE_WriteReg(GE_REG_SB_PIT, u32PitchNew);
    }
#endif

    GE_WriteReg(GE_REG_STBB_WIDTH, u32Width);

    if ((drawflag&GEDRAW_FLAG_SCALE) && (u32Height>5))
        GE_WriteReg(GE_REG_STBB_HEIGHT,u32Height -5 );
    else
        GE_WriteReg(GE_REG_STBB_HEIGHT,u32Height);

    // Set source coordinate
    GE_WriteReg(GE_REG_PRI_V2_X, drawbuf->srcblk.x);
    GE_WriteReg(GE_REG_PRI_V2_Y, drawbuf->srcblk.y);

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_X_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_X, drawbuf->srcblk.x + drawbuf->srcblk.width - 1);
    }

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_Y_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_Y, drawbuf->srcblk.y + drawbuf->srcblk.height - 1);
    }

    if(drawflag&GEDRAW_FLAG_DUPLICAPE)
    {
        _u32Reg60hFlag |= GE_VAL_STBB_PATCH;
    }
    else
    {
        _u32Reg60hFlag &= ~GE_VAL_STBB_PATCH;
    }

    if(drawflag&GEDRAW_FLAG_SCALE)
    {
		S32 s32Temp1, s32Temp2;
		U32 u32Value3, u32ValueFix = u32Width;
		
		if ( drawbuf->dstblk.width < (u32Width >> 1) )
			u32ValueFix = u32Width - 1;

        u32Value = Divide2Fixed(u32ValueFix, drawbuf->dstblk.width, 5, 12);//<< 2 ; //sc
        GE_WriteReg(GE_REG_STBB_DX, u32Value & 0xffff);
		s32Temp1 = u32ValueFix - drawbuf->dstblk.width;
		s32Temp2 = 2* drawbuf->dstblk.width;
		s32Temp1 = s32Temp1 % s32Temp2;
        u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
		if ( u32Value & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DX_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DX_MSB;
		}
		GE_WriteReg(GE_REG_STBB_INIT_DX, u32Value3);
		
		if ( drawbuf->dstblk.height < (u32Height >> 1) )
			u32ValueFix = u32Height - 1;
		else
			u32ValueFix = u32Height;
			
        u32Value = Divide2Fixed(u32ValueFix, drawbuf->dstblk.height, 5, 12);// << 2 ; //sc
        GE_WriteReg(GE_REG_STBB_DY, u32Value & 0xffff);
		s32Temp1 = u32ValueFix - drawbuf->dstblk.height;
		s32Temp2 = 2* drawbuf->dstblk.height;
		s32Temp1 = s32Temp1 % s32Temp2;
        u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
		if ( u32Value & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DY_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DY_MSB;
		}
		GE_WriteReg(GE_REG_STBB_INIT_DY, u32Value3);

        //scale = (U16)((float)(u32Width-1) * ((float)pbmpfmt->width / (float)u32Width));//TODO
//        u32Scale = (((U32)drawbuf->dstblk.width << 5) / u32Width * (u32Width-1)) >> 5; //sc
//        u32Width = u32Scale;  //pbmpfmt->width;
        u32Width = drawbuf->dstblk.width;

        //scale = (U16)((float)(u32Height-1) * ((float)pbmpfmt->height / (float)u32Height));//TODO
//        u32Scale = (((U32)drawbuf->dstblk.height << 5) / u32Height * (u32Height-1)) >> 5; //sc
//        u32Height = u32Scale;  //pbmpfmt->height;
        u32Height = drawbuf->dstblk.height;
        u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
    }
    else
    {
        u32Width  = drawbuf->dstblk.width;
        u32Height = drawbuf->dstblk.height;
        GE_WriteReg(GE_REG_STBB_DX, 0x1000);
        GE_WriteReg(GE_REG_STBB_DY, 0x1000);
        GE_WriteReg(GE_REG_STBB_INIT_DX, 0);
        GE_WriteReg(GE_REG_STBB_INIT_DY, 0);
        if ((drawbuf->dstblk.width != drawbuf->srcblk.width) ||
            (drawbuf->dstblk.height != drawbuf->srcblk.height))
        {
            u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
        }
        else
        {
            u32Value2 &= ~GE_VAL_EN_STRETCH_BITBLT;
        }
    }

    GE_WriteReg(GE_REG_FMT_BLT, u32Value2);

    if (u32Value2 & GE_VAL_EN_STRETCH_BITBLT)
    {
        _u32Reg60hFlag |= GE_VAL_STBB_PATCH;
    }

    //------------------------------------------------------------
    // Destination coordinate
    //------------------------------------------------------------
    GE_WriteReg(GE_REG_PRI_V0_X, drawbuf->dstblk.x);
    GE_WriteReg(GE_REG_PRI_V0_Y, drawbuf->dstblk.y);
    GE_WriteReg(GE_REG_PRI_V1_X, drawbuf->dstblk.x + u32Width - 1);
    GE_WriteReg(GE_REG_PRI_V1_Y, drawbuf->dstblk.y + u32Height - 1);

    // @FIXME
    // check GE_REG_SB_DB_MODE

    GE_WriteReg(GE_REG_CMD, (GE_VAL_PRIM_BITBLT|_u32Reg60hFlag));

#if GE_AUTO_ALIGN_SRC_BUFFER
    // Restore source buffer address & pitch
    if(pTempBuf)
    {
        GE_WaitCmdEmpty();
        while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
        {
            MsOS_YieldTask();
        }

        MsOS_FreeMemory(pTempBuf, gs32NonCachedPoolID);
//        // Set source address
//        GE_WriteReg(GE_REG_SB_BASE0, _u32SrcAddrPE & 0xffff);
//        GE_WriteReg(GE_REG_SB_BASE1, _u32SrcAddrPE >> 16);
//
//        // Set source pitch
//        GE_WriteReg(GE_REG_SB_PIT, _u32SrcPitchPE);
    }
#endif

    return GESTATUS_SUCCESS;
}


GESTATUS MHal_GE_BitBltEx(GE_DRAW_RECT * drawbuf, U32 drawflag, GE_SCALE_INFO * ScaleInfo)
{
    U32 u32Width, u32Height;
    U32 u32Value2;
#if GE_AUTO_ALIGN_SRC_BUFFER
    U8 *pTempBuf=NULL;
#endif

    GE_DEBUGINFO( printk( "MHal_GE_BitBltEx\n" ) );
    if (!GE_CheckInClipWindow(drawbuf->dstblk.x, drawbuf->dstblk.y,
                              drawbuf->dstblk.x+drawbuf->dstblk.width-1,
                              drawbuf->dstblk.y+drawbuf->dstblk.height-1))
    {
        return GESTATUS_FAIL;
    }
    GE_WaitAvailableCMDQueue(20);

    u32Value2 = GE_ReadReg(GE_REG_FMT_BLT);

    u32Width  = drawbuf->srcblk.width;
    u32Height = drawbuf->srcblk.height;

#if GE_AUTO_ALIGN_SRC_BUFFER
    if((_u32SrcAddrPE % 16) ||
       ((_u32SrcPitchPE % 16) &&
        (_u32SrcFmtPE != GE_VAL_I1) &&
        (_u32SrcFmtPE != GE_VAL_I2) &&
        (_u32SrcFmtPE != GE_VAL_I4)))
    {
        // If the bitmap pitch is not 16-byte alignment, allocate a 16-byte memory to do it.
        U8 *pSrcBuf;
        U32 u32PitchNew;
        U32 i;

        u32PitchNew = ALIGN_16(_u32SrcPitchPE);
        pTempBuf = MsOS_AllocateMemory(u32PitchNew * u32Height, gs32NonCachedPoolID);
        if(!pTempBuf)
        {
            return GESTATUS_NON_ALIGN_PITCH;
        }
        pSrcBuf = (U8*)_u32SrcAddrPE;
        for(i=0;i<u32Height;i++)
        {
            memcpy(&pTempBuf[i*u32PitchNew],
                   &pSrcBuf[i*_u32SrcPitchPE]+_u32OffsetSrc,
                   _u32SrcPitchPE);
        }
        // Set source address
        GE_WriteReg(GE_REG_SB_BASE0, (U32)pTempBuf & 0xffff);
        GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr((U32)pTempBuf) >> 16);

        // Set source pitch
        GE_WriteReg(GE_REG_SB_PIT, u32PitchNew);
    }
#endif

    GE_WriteReg(GE_REG_STBB_WIDTH, u32Width);

    if ((drawflag&GEDRAW_FLAG_SCALE) && (u32Height>5))
        GE_WriteReg(GE_REG_STBB_HEIGHT,u32Height -5 );
    else
        GE_WriteReg(GE_REG_STBB_HEIGHT,u32Height);


    // Set source coordinate
    GE_WriteReg(GE_REG_PRI_V2_X, drawbuf->srcblk.x);
    GE_WriteReg(GE_REG_PRI_V2_Y, drawbuf->srcblk.y);

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_X_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_X, drawbuf->srcblk.x + drawbuf->srcblk.width - 1);
    }

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_Y_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_Y, drawbuf->srcblk.y + drawbuf->srcblk.height - 1);
    }

    if(drawflag&GEDRAW_FLAG_DUPLICAPE)
    {
        _u32Reg60hFlag |= GE_VAL_STBB_PATCH;
    }
    else
    {
        _u32Reg60hFlag &= ~GE_VAL_STBB_PATCH;
    }

    if(drawflag&GEDRAW_FLAG_SCALE)
    {
		U32 u32Value3;
		
        GE_WriteReg(GE_REG_STBB_DX, (ScaleInfo->u32DeltaX & 0xffff));
		u32Value3 = ScaleInfo->u32InitDelatX;
		if ( ScaleInfo->u32DeltaX & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DX_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DX_MSB;
		}
        GE_WriteReg(GE_REG_STBB_INIT_DX, (u32Value3 & 0xffff));
        GE_WriteReg(GE_REG_STBB_DY, (ScaleInfo->u32DeltaY & 0xffff));
		u32Value3 = ScaleInfo->u32InitDelatY;
		if ( ScaleInfo->u32DeltaY & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DY_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DY_MSB;
		}
        GE_WriteReg(GE_REG_STBB_INIT_DY, (u32Value3 & 0xffff));

        u32Width = drawbuf->dstblk.width;
        u32Height = drawbuf->dstblk.height;
        u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
    }
    else
    {
        u32Width  = drawbuf->dstblk.width;
        u32Height = drawbuf->dstblk.height;
        GE_WriteReg(GE_REG_STBB_DX, 0x1000);
        GE_WriteReg(GE_REG_STBB_DY, 0x1000);
        GE_WriteReg(GE_REG_STBB_INIT_DX, 0);
        GE_WriteReg(GE_REG_STBB_INIT_DY, 0);
        if ((drawbuf->dstblk.width != drawbuf->srcblk.width) ||
            (drawbuf->dstblk.height != drawbuf->srcblk.height))
        {
            u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
        }
        else
        {
            u32Value2 &= ~GE_VAL_EN_STRETCH_BITBLT;
        }
    }

    GE_WriteReg(GE_REG_FMT_BLT, u32Value2);

    if (u32Value2 & GE_VAL_EN_STRETCH_BITBLT)
    {
        _u32Reg60hFlag |= GE_VAL_STBB_PATCH;
    }

    //------------------------------------------------------------
    // Destination coordinate
    //------------------------------------------------------------
    GE_WriteReg(GE_REG_PRI_V0_X, drawbuf->dstblk.x);
    GE_WriteReg(GE_REG_PRI_V0_Y, drawbuf->dstblk.y);
    GE_WriteReg(GE_REG_PRI_V1_X, drawbuf->dstblk.x + u32Width - 1);
    GE_WriteReg(GE_REG_PRI_V1_Y, drawbuf->dstblk.y + u32Height - 1);

    // @FIXME
    // check GE_REG_SB_DB_MODE

    GE_WriteReg(GE_REG_CMD, (GE_VAL_PRIM_BITBLT|_u32Reg60hFlag));

#if GE_AUTO_ALIGN_SRC_BUFFER
    // Restore source buffer address & pitch
    if(pTempBuf)
    {
        GE_WaitCmdEmpty();
        while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
        {
            MsOS_YieldTask();
        }

        MsOS_FreeMemory(pTempBuf, gs32NonCachedPoolID);
//        // Set source address
//        GE_WriteReg(GE_REG_SB_BASE0, _u32SrcAddrPE & 0xffff);
//        GE_WriteReg(GE_REG_SB_BASE1, _u32SrcAddrPE >> 16);
//
//        // Set source pitch
//        GE_WriteReg(GE_REG_SB_PIT, _u32SrcPitchPE);
    }
#endif

    return GESTATUS_SUCCESS;
}

static GE_POINT_t _MHal_GE_CalRotateCoordinate(U16 destX, U16 destY, U16 refX, U16 refY, U8 nTrapRotateDir)
{
    GE_POINT_t val;

    val.x = 0;
    val.y = 0;

    switch(nTrapRotateDir)
    {
    case 0:
        //no need to convert
        //(X, Y)
        val.x = destX;
        val.y = destY;
        break;
    case 1:
        //need to use rotate 270 degrees formula
        //(Y + Xref - Yref, -X + Xref + Yref)
        val.x = destY + refX - refY;
        val.y = refX + refY - destX;
        break;
    case 2:
        //need to use rotate 180 degrees formula
        //(-X + 2 * Xref, -Y + 2 * Yref)
        val.x = (refX << 1) - destX;
        val.y = (refY << 1) - destY;
        break;
    case 3:
        //need to use rotate 90 degrees formula
        //(-Y + Xref + Yref, X - Xref + Yref)
        val.x = refX + refY - destY;
        val.y = destX + refY - refX;
        break;
    default:
        break;
    }

    return val;
}

//-------------------------------------------------------------------------------------------------
/// Set PE Trapezoid Bit blt
/// @param drawbuf \b IN: pointer to drawbuf info
/// @param drawflag \b IN: draw flag \n
///                  GEDRAW_FLAG_DEFAULT \n
///                  GEDRAW_FLAG_SCALE \n
///                  GEDRAW_FLAG_DUPLICAPE \n
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_TrapezoidBitBlt(GE_TRAPEZOID_INFO *pGETrapezoidInfo,U32 drawflag)
{
    U32 u32Width, u32Height;
    U32 u32Value, u32Value2, u32Value3, u32RotateBackup;//, u32AlphaBackup = 0;
    S32 s32Temp1, s32Temp2;
//    U8 u8ConstAlphaBackup;
	U32 u32ValueFix;
#if GE_AUTO_ALIGN_SRC_BUFFER
    U8 *pTempBuf=NULL;
#endif

    //fixed by Samuel Huang(Mstar) and updated by wgkwak(LGE)
    if( ((pGETrapezoidInfo->dstTrapEdge0End-pGETrapezoidInfo->dstTrapEdge0St) <= 0) ||
        ((pGETrapezoidInfo->dstTrapEdge1End-pGETrapezoidInfo->dstTrapEdge1St) <= 0) ||
        (!pGETrapezoidInfo->dstTrapDistance) ||
        (!pGETrapezoidInfo->srcRect.width) ||
        (!pGETrapezoidInfo->srcRect.height) )
    {
        GE_DEBUGINFO(printk("GE trapezoid bilblt: Invalid condition!!!!!\n"));
        return GESTATUS_FAIL;
    }

    GE_DEBUGINFO( printk( "MHal_GE_TrapezoidBitBlt\n" ) );

// todo later
#if 0
    if (!GE_CheckInClipWindow(drawbuf->dstblk.x, drawbuf->dstblk.y,
                              drawbuf->dstblk.x+drawbuf->dstblk.width-1,
                              drawbuf->dstblk.y+drawbuf->dstblk.height-1))
    {
        return GESTATUS_FAIL;
    }
#endif

    GE_WaitAvailableCMDQueue(20);

    u32Value2 = GE_ReadReg(GE_REG_FMT_BLT);

    u32Width  = pGETrapezoidInfo->srcRect.width;
    u32Height = pGETrapezoidInfo->srcRect.height;

// todo later
#if 0
#if GE_AUTO_ALIGN_SRC_BUFFER
    if((_u32SrcAddrPE % 16) ||
       ((_u32SrcPitchPE % 16) &&
        (_u32SrcFmtPE != GE_VAL_I1) &&
        (_u32SrcFmtPE != GE_VAL_I2) &&
        (_u32SrcFmtPE != GE_VAL_I4)))
    {
        // If the bitmap pitch is not 16-byte alignment, allocate a 16-byte memory to do it.
        U8 *pSrcBuf;
        U32 u32PitchNew;
        U32 i;

        u32PitchNew = ALIGN_16(_u32SrcPitchPE);
        pTempBuf = MsOS_AllocateMemory(u32PitchNew * u32Height, gs32NonCachedPoolID);
        if(!pTempBuf)
        {
            return GESTATUS_NON_ALIGN_PITCH;
        }
        pSrcBuf = (U8*)_u32SrcAddrPE;
        for(i=0;i<u32Height;i++)
        {
            memcpy(&pTempBuf[i*u32PitchNew],
                   &pSrcBuf[i*_u32SrcPitchPE]+_u32OffsetSrc,
                   _u32SrcPitchPE);
        }
        // Set source address
        GE_WriteReg(GE_REG_SB_BASE0, (U32)pTempBuf & 0xffff);
        GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr((U32)pTempBuf) >> 16);

        // Set source pitch
        GE_WriteReg(GE_REG_SB_PIT, u32PitchNew);
    }
#endif
#endif

    GE_WriteReg(GE_REG_STBB_WIDTH, u32Width);
    GE_WriteReg(GE_REG_STBB_HEIGHT,u32Height);

    // Set source coordinate
    GE_WriteReg(GE_REG_PRI_V2_X, pGETrapezoidInfo->srcRect.x);
    GE_WriteReg(GE_REG_PRI_V2_Y, pGETrapezoidInfo->srcRect.y);

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_X_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_X, pGETrapezoidInfo->srcRect.x + pGETrapezoidInfo->srcRect.width - 1);
    }

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_Y_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_Y, pGETrapezoidInfo->srcRect.y + pGETrapezoidInfo->srcRect.height - 1);
    }

    if(drawflag&GEDRAW_FLAG_DUPLICAPE)
    {
        _u32Reg60hFlag |= GE_VAL_STBB_PATCH;
    }
    else
    {
        _u32Reg60hFlag &= ~GE_VAL_STBB_PATCH;
    }


    //////////////////////////////////////////////////////////////////////
    // To coordinate conversion
    // calcluate trap dir here
    if ( ( pGETrapezoidInfo->trapDir ) || ( pGETrapezoidInfo->dstTrapRotateDir ) )
    {
        GE_POINT_t NewXY;
        U16 refX = 0, refY = 0;
        U16 NewEdge0St, NewEdge0End, NewEdge0Pos, NewEdge1St, NewEdge1End;
        U16 Offset = 0;

        // horizontal trapezoid rendering, do software simulation
        // Find reference point first
        refX = pGETrapezoidInfo->dstTrapEdge0Pos+pGETrapezoidInfo->dstTrapDistance-1;

        if ( pGETrapezoidInfo->dstTrapEdge0St <= pGETrapezoidInfo->dstTrapEdge1St )
        {
            //Type A, horizontal face right
            //(Edge1Pos, Edge0st) is reference point
            refY = pGETrapezoidInfo->dstTrapEdge0St;
        }
        else
        {
            //Type B, horizontal face left
            //(Edge1Pos, Edge1st) is reference point
            refY = pGETrapezoidInfo->dstTrapEdge1St;
        }

        NewXY = _MHal_GE_CalRotateCoordinate((pGETrapezoidInfo->dstTrapEdge0Pos+pGETrapezoidInfo->dstTrapDistance-1),
                                             pGETrapezoidInfo->dstTrapEdge1St,
                                             refX,
                                             refY,
                                             pGETrapezoidInfo->dstTrapRotateDir);
        NewEdge0St = NewXY.x;
        NewEdge0Pos = NewXY.y;
        NewXY = _MHal_GE_CalRotateCoordinate((pGETrapezoidInfo->dstTrapEdge0Pos+pGETrapezoidInfo->dstTrapDistance-1),
                                             pGETrapezoidInfo->dstTrapEdge1End,
                                             refX,
                                             refY,
                                             pGETrapezoidInfo->dstTrapRotateDir);
        NewEdge0End = NewXY.x;
        NewXY = _MHal_GE_CalRotateCoordinate(pGETrapezoidInfo->dstTrapEdge0Pos,
                                             pGETrapezoidInfo->dstTrapEdge0St,
                                             refX,
                                             refY,
                                             pGETrapezoidInfo->dstTrapRotateDir);
        NewEdge1St = NewXY.x;
        NewXY = _MHal_GE_CalRotateCoordinate(pGETrapezoidInfo->dstTrapEdge0Pos,
                                             pGETrapezoidInfo->dstTrapEdge0End,
                                             refX,
                                             refY,
                                             pGETrapezoidInfo->dstTrapRotateDir);
        NewEdge1End = NewXY.x;

        if ( NewEdge1St < NewEdge0St )
            Offset = NewEdge0St - NewEdge1St;
        pGETrapezoidInfo->dstTrapEdge0St = NewEdge0St - Offset;
        pGETrapezoidInfo->dstTrapEdge0End = NewEdge0End - Offset;
        pGETrapezoidInfo->dstTrapEdge0Pos = NewEdge0Pos + Offset;
        pGETrapezoidInfo->dstTrapEdge1St = NewEdge1St - Offset;
        pGETrapezoidInfo->dstTrapEdge1End = NewEdge1End - Offset;
    }

    //if(drawflag&GEDRAW_FLAG_SCALE)

    //Trapezoid settings
    //after coordinate conversion

    //////////////////////////////////////////////////////////////////////
    //delta X
    //don't care in vertical trapezoid rendering
    //u32Value = Divide2Fixed(u32Width, u32Width, 5, 12);
    //GE_WriteReg(GE_REG_STBB_DX, u32Value & 0xffff);

    //u32Value3 = Divide2Fixed((u32Width - u32Width), 2* u32Width, 0, 12);
    //if ( u32Value & 0x00010000 )
    //{
        //u32Value3 |= GE_VAL_STBB_DX_MSB;
    //}
    //else
    //{
        //u32Value3 &= ~GE_VAL_STBB_DX_MSB;
    //}
    //GE_WriteReg(GE_REG_STBB_INIT_DX, u32Value3);

    //delta Y
	u32ValueFix = u32Height;
		
	if ( pGETrapezoidInfo->dstTrapDistance < (u32Height >> 1) )
		u32ValueFix = u32Height - 1;
		
    u32Value = Divide2Fixed(u32ValueFix, pGETrapezoidInfo->dstTrapDistance, 5, 12);
    GE_WriteReg(GE_REG_STBB_DY, u32Value & 0xffff);
	
    s32Temp1 = u32ValueFix - pGETrapezoidInfo->dstTrapDistance;
    s32Temp2 = 2* pGETrapezoidInfo->dstTrapDistance;
    s32Temp1 = s32Temp1 % s32Temp2;
    u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);
    if ( u32Value & 0x00010000 )
    {
        u32Value3 |= GE_VAL_STBB_DY_MSB;
    }
    else
    {
        u32Value3 &= ~GE_VAL_STBB_DY_MSB;
    }
    GE_WriteReg(GE_REG_STBB_INIT_DY, u32Value3);

    //////////////////////////////////////////////////////////////////////
    u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;

    GE_WriteReg(GE_REG_FMT_BLT, u32Value2);

    _u32Reg60hFlag |= GE_VAL_STBB_PATCH;
    //////////////////////////////////////////////////////////////////////

    //delta X0
	if ( pGETrapezoidInfo->dstTrapDistance < ((pGETrapezoidInfo->dstTrapEdge1St - pGETrapezoidInfo->dstTrapEdge0St) >> 1) )
		u32ValueFix = (pGETrapezoidInfo->dstTrapEdge1St - pGETrapezoidInfo->dstTrapEdge0St) - 1;
	else
		u32ValueFix = (pGETrapezoidInfo->dstTrapEdge1St - pGETrapezoidInfo->dstTrapEdge0St);
		
    u32Value = Divide2Fixed(u32ValueFix, pGETrapezoidInfo->dstTrapDistance, 5, 12);
    GE_WriteReg(GE_REG_TRAP_DELTA_X0, u32Value & 0xffff);

    u32Value >>= 16;
    u32Value <<= GE_MSK_TRAP_DELTA_X0_POS;

    u32Value3 = GE_ReadReg(GE_REG_ROT_MIRROR);
    u32RotateBackup = u32Value3;
    u32Value3 &= ~GE_MSK_TRAP_DELTA_X0;
    u32Value3 |= u32Value;

    //GE_WriteReg(GE_REG_ROT_MIRROR, u32Value1);

    //delta X1
	if ( pGETrapezoidInfo->dstTrapDistance < ((pGETrapezoidInfo->dstTrapEdge1End - pGETrapezoidInfo->dstTrapEdge0End) >> 1) )
		u32ValueFix = (pGETrapezoidInfo->dstTrapEdge1End - pGETrapezoidInfo->dstTrapEdge0End) - 1;
	else
		u32ValueFix = (pGETrapezoidInfo->dstTrapEdge1End - pGETrapezoidInfo->dstTrapEdge0End);
		
    u32Value = Divide2Fixed(u32ValueFix, pGETrapezoidInfo->dstTrapDistance, 5, 12);
    GE_WriteReg(GE_REG_TRAP_DELTA_X1, u32Value & 0xffff);

    u32Value >>= 16;
    u32Value <<= GE_MSK_TRAP_DELTA_X1_POS;

    //u32Value1 = GE_ReadReg(GE_REG_ROT_MIRROR);
    u32Value3 &= ~GE_MSK_TRAP_DELTA_X1;
    u32Value3 |= u32Value;

    //set Trapezoid rotation degree
    u32Value3 &= ~GE_MSK_ROT;
    u32Value3 |= pGETrapezoidInfo->dstTrapRotateDir;

    GE_WriteReg(GE_REG_ROT_MIRROR, u32Value3);
    //////////////////////////////////////////////////////////////////////

    // Enable/Disable antialiasing
    u32Value = GE_ReadReg(GE_REG_EN);
    if ( pGETrapezoidInfo->bAntiAliasing )
    {
        u32Value |= GE_VAL_EN_GE_TRAP_AA;
    }
    else
    {
        u32Value &= ~GE_VAL_EN_GE_TRAP_AA;
    }
    // Enable subpixel correction
    u32Value |= GE_VAL_EN_GE_TRAP_SUBC;

    if ( pGETrapezoidInfo->pixelFormat > GE_FMT_I8 )
    {
        // Enable alpha blending
        u32Value |= GE_VAL_EN_GY_ABL;
    }
    else
    {
        // Disable alpha blending
        u32Value &= ~GE_VAL_EN_GY_ABL;
    }

    GE_WriteReg(GE_REG_EN, u32Value);
    //////////////////////////////////////////////////////////////////////

    //------------------------------------------------------------
    // Destination Trapezoid coordinate
    //------------------------------------------------------------
    GE_WriteReg(GE_REG_PRI_V0_X, pGETrapezoidInfo->dstTrapEdge0St);
    GE_WriteReg(GE_REG_PRI_V0_Y, pGETrapezoidInfo->dstTrapEdge0Pos);
    GE_WriteReg(GE_REG_PRI_V1_X, pGETrapezoidInfo->dstTrapEdge0End);
    GE_WriteReg(GE_REG_PRI_V1_Y, pGETrapezoidInfo->dstTrapEdge0Pos + pGETrapezoidInfo->dstTrapDistance-1);

    //////////////////////////////////////////////////////////////////////
    // Set const alpha here
    //////////////////////////////////////////////////////////////////////
    // Backup original values
//    u32AlphaBackup = u32Value = GE_ReadReg(GE_REG_ABL_COEF);

//    u32Value &= ~GE_MSK_ABL_COEF;
    // set Csrc * Aconst
//    u32Value |= 7;
//    GE_WriteReg(GE_REG_ABL_COEF, u32Value);

//    u32Value = GE_ReadReg(GE_REG_DB_ABL);
//    u32AlphaBackup |= (u32Value << 16);

    // set Asrc
//    u32Value &= ~GE_MSK_DB_ABL;
//    u32Value |= 0x0100;
//    GE_WriteReg(GE_REG_DB_ABL, u32Value);

    // Backup original const alpha
//    u8ConstAlphaBackup = GE_ReadReg(GE_REG_ABL_CONST);
    // set const alpha value
//    GE_WriteReg(GE_REG_ABL_CONST, pGETrapezoidInfo->alphaConst);
    //////////////////////////////////////////////////////////////////////


    // @FIXME
    // check GE_REG_SB_DB_MODE
    // Fire
    GE_WriteReg(GE_REG_CMD, (GE_VAL_PRIM_TRAPEZOID_BITBLT|_u32Reg60hFlag));

    // Restore rotate degree
    u32Value3 &= ~GE_MSK_ROT;
    u32Value3 |= (u32RotateBackup & GE_MSK_ROT);
    GE_WriteReg(GE_REG_ROT_MIRROR, u32Value3);

    // Restore alpha settings
//    GE_WriteReg(GE_REG_ABL_COEF, (u32AlphaBackup & 0xFFFF));
//    GE_WriteReg(GE_REG_DB_ABL, (u32AlphaBackup >> 16));
//    GE_WriteReg(GE_REG_ABL_CONST, u8ConstAlphaBackup);

// todo later
#if 0
#if GE_AUTO_ALIGN_SRC_BUFFER
    // Restore source buffer address & pitch
    if(pTempBuf)
    {
        GE_WaitCmdEmpty();
        while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
        {
            MsOS_YieldTask();
        }

        MsOS_FreeMemory(pTempBuf, gs32NonCachedPoolID);
//        // Set source address
//        GE_WriteReg(GE_REG_SB_BASE0, _u32SrcAddrPE & 0xffff);
//        GE_WriteReg(GE_REG_SB_BASE1, _u32SrcAddrPE >> 16);
//
//        // Set source pitch
//        GE_WriteReg(GE_REG_SB_PIT, _u32SrcPitchPE);
    }
#endif
#endif

    return GESTATUS_SUCCESS;
}

GESTATUS MHal_GE_GetScaleBltInfo(GE_DRAW_RECT* pdrawbuf, GE_BLOCK* pSrcBlk,  GE_BLOCK* pDstBlk, GE_SCALE_INFO * pScaleInfo)
{
    U32 u32InitDx, u32InitDy;
    U32 u32Temp;
    //U32 u32Temp1;
    U32 u32InverseDeltaX;
    U32 u32InverseDeltaY;
    U16 bXStartMinus_1 = FALSE;
    U16 bYStartMinus_1 = FALSE;
    U32 u32Temp2, u32Temp3;
	S32 s32Temp1, s32Temp2;
	
    if (pdrawbuf->srcblk.width == pdrawbuf->dstblk.width)
    {
        pDstBlk->x  = pSrcBlk->x;
        pDstBlk->width = pSrcBlk->width;
        pScaleInfo->u32DeltaX = 0x1000;
        pScaleInfo->u32InitDelatX = 0;
    }
    else
    {
		U32 u32ValueFix = pdrawbuf->srcblk.width;
		
		if ( pdrawbuf->dstblk.width < (pdrawbuf->srcblk.width >> 1) )
			u32ValueFix = pdrawbuf->srcblk.width - 1;
			
        pScaleInfo->u32DeltaX = Divide2Fixed(u32ValueFix, pdrawbuf->dstblk.width, 5, 12);//<< 2 ; //sc
        u32InverseDeltaX = Divide2Fixed(pdrawbuf->dstblk.width, u32ValueFix, 5, 12);//<< 2 ; //sc
		s32Temp1 = u32ValueFix - pdrawbuf->dstblk.width;
		s32Temp2 = 2* pdrawbuf->dstblk.width;
		s32Temp1 = s32Temp1 % s32Temp2;
        u32InitDx = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc

        if (pdrawbuf->srcblk.width > pdrawbuf->dstblk.width)
        {
            //Horizontal down scaling
            //Get destination x
            if (pSrcBlk->x == 0)
            {
                pDstBlk->x = 0;
            }
            else
            {
                //u32Temp = (((pSrcBlk->x<<12) - (u32InitDx & 0xfff))*pdrawbuf->dstblk.width)/pdrawbuf->srcblk.width;
                u32Temp2 = ((((pSrcBlk->x<<12) - (u32InitDx & 0xfff))<<8)/pScaleInfo->u32DeltaX)>>8;
                u32Temp3 = (((pSrcBlk->x<<12) - (u32Temp2*pScaleInfo->u32DeltaX + u32InitDx))<<12)/pScaleInfo->u32DeltaX;
                u32Temp = ((u32Temp2<<12)|(u32Temp3&0xfff));
                if ((u32Temp & 0xfff) > u32InverseDeltaX)
                {
                    pDstBlk->x  = ((u32Temp & 0xfffff000)>>12)+1;
                    bXStartMinus_1 = FALSE;
                }
                else
                {
                    pDstBlk->x  = ((u32Temp & 0xfffff000)>>12);
                    bXStartMinus_1 = TRUE;
                }
            }

            //Get Destination width
            //u32Temp = ((((pSrcBlk->x+pSrcBlk->width)<<12) - (u32InitDx & 0x1fff))*pdrawbuf->dstblk.width)/pdrawbuf->srcblk.width;
            u32Temp2 = (((((pSrcBlk->x+pSrcBlk->width)<<12) - (u32InitDx & 0xfff))<<8)/pScaleInfo->u32DeltaX)>>8;
            u32Temp3 = ((((pSrcBlk->x+pSrcBlk->width)<<12) - (u32Temp2*pScaleInfo->u32DeltaX + u32InitDx))<<12)/pScaleInfo->u32DeltaX;
            u32Temp = ((u32Temp2<<12)|(u32Temp3&0xfff));
            if ((u32Temp & 0xfff) < u32InverseDeltaX)
            {
                pSrcBlk->width++;
            }
            pDstBlk->width = ((u32Temp & 0xfffff000)>>12) - pDstBlk->x  + 1;


            //Get x initial delta.
            if (bXStartMinus_1)
            {
                pSrcBlk->x--;
                pSrcBlk->width++;
            }
            u32Temp = pDstBlk->x *pScaleInfo->u32DeltaX + u32InitDx;
            //u32Temp = ((pDstBlk->x<<12)*pdrawbuf->srcblk.width)/pdrawbuf->dstblk.width + u32InitDx;
        }
        else
        {
            //Horizontal up scaling
            //Get destination x
            if (pSrcBlk->x == 0)
            {
                pDstBlk->x = 0;
            }
            else
            {
                //u32Temp = (((pSrcBlk->x<<12) - ((~u32InitDx+1)&0xfff))*pdrawbuf->dstblk.width)/pdrawbuf->srcblk.width;
                u32Temp2 = ((((pSrcBlk->x<<12) + ((~u32InitDx+1)&0xfff))<<8)/pScaleInfo->u32DeltaX)>>8;
                u32Temp3 = ((((pSrcBlk->x<<12) - (u32Temp2*pScaleInfo->u32DeltaX - ((~u32InitDx+1)&0xfff))))<<12)/pScaleInfo->u32DeltaX;
                u32Temp = ((u32Temp2<<12)|(u32Temp3&0xfff));
                pDstBlk->x  = (((u32Temp-u32InverseDeltaX)&0xfffff000)>>12)+1;
                bXStartMinus_1 = TRUE;
            }

            //Get Destination width
            //u32Temp = ((((pSrcBlk->x + pSrcBlk->width)<<12) + ((~u32InitDx+1) & 0x1fff))*pdrawbuf->dstblk.width)/pdrawbuf->srcblk.width;
            u32Temp2 = (((((pSrcBlk->x + pSrcBlk->width)<<12) + ((~u32InitDx+1)&0xfff))<<8)/pScaleInfo->u32DeltaX)>>8;
            u32Temp3 = (((((pSrcBlk->x + pSrcBlk->width)<<12) - (u32Temp2*pScaleInfo->u32DeltaX - ((~u32InitDx+1)&0xfff))))<<12)/pScaleInfo->u32DeltaX;
            u32Temp = ((u32Temp2<<12)|(u32Temp3&0xfff));
            pDstBlk->width = ((u32Temp & 0xfffff000)>>12) - pDstBlk->x  + 1;
            pSrcBlk->width++;


            //Get x initial delta.
            if (bXStartMinus_1)
            {
                pSrcBlk->x--;
                pSrcBlk->width++;
            }
            u32Temp = pDstBlk->x *pScaleInfo->u32DeltaX - ((~u32InitDx+1)&0xfff);
            //u32Temp = ((pDstBlk->x<<12)*pdrawbuf->srcblk.width)/pdrawbuf->dstblk.width - ((~u32InitDx+1)&0xfff);
        }

        if (pDstBlk->x == 0)
        {
            pScaleInfo->u32InitDelatX = u32InitDx;
        }
        else
        {
            if (u32Temp > (pSrcBlk->x<<12))
            {
                if ((u32Temp - (pSrcBlk->x<<12)) < 0x1000)
                {
                    pScaleInfo->u32InitDelatX = (u32Temp- (pSrcBlk->x<<12));
                }
                else
                {
                    //Invalid destination X position.
                    //printk("+ ,Invalid destination X position.\n");
                    pScaleInfo->u32InitDelatX  = 0xfff;
                }
            }
            else
            {
                //printk("- ,Invalid destination X direction.\n");
                pScaleInfo->u32InitDelatX  = 0;
            }
        }
    }

    if (pdrawbuf->srcblk.height == pdrawbuf->dstblk.height)
    {
        pDstBlk->y = pSrcBlk->y;
        pDstBlk->height = pSrcBlk->height;
        pScaleInfo->u32DeltaY = 0x1000;
        pScaleInfo->u32InitDelatY = 0;
    }
    else
    {
		U32 u32ValueFix = pdrawbuf->srcblk.height;
		
		if ( pdrawbuf->dstblk.height < (pdrawbuf->srcblk.height >> 1) )
			u32ValueFix = pdrawbuf->srcblk.height - 1;
			
        pScaleInfo->u32DeltaY = Divide2Fixed(u32ValueFix, pdrawbuf->dstblk.height, 5, 12);// << 2 ; //sc
        u32InverseDeltaY = Divide2Fixed(pdrawbuf->dstblk.height, u32ValueFix, 5, 12);// << 2 ; //sc
		s32Temp1 = u32ValueFix - pdrawbuf->dstblk.height;
		s32Temp2 = 2* pdrawbuf->dstblk.height;
		s32Temp1 = s32Temp1 % s32Temp2;
        u32InitDy = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc

        if (pdrawbuf->srcblk.height > pdrawbuf->dstblk.height)
        {
            //Vertical down scaling
            //Get destination y
            if (pSrcBlk->y == 0)
            {
                pDstBlk->y = 0;
            }
            else
            {
                //u32Temp = (((pSrcBlk->y<<12) - (u32InitDy & 0xfff))*pdrawbuf->dstblk.height)/pdrawbuf->srcblk.height;
                u32Temp2 = ((((pSrcBlk->y<<12) - (u32InitDy & 0xfff))<<8)/pScaleInfo->u32DeltaY)>>8;
                u32Temp3 = (((pSrcBlk->y<<12) - (u32Temp2*pScaleInfo->u32DeltaY + u32InitDy))<<12)/pScaleInfo->u32DeltaY;
                u32Temp = ((u32Temp2<<12)|(u32Temp3&0xfff));
                if ((u32Temp & 0xfff) > u32InverseDeltaY)
                {
                    pDstBlk->y = ((u32Temp & 0xfffff000)>>12)+1;
                    bYStartMinus_1 = FALSE;
                }
                else
                {
                    pDstBlk->y = ((u32Temp & 0xfffff000)>>12);
                    bYStartMinus_1 = TRUE;
                }
            }

            //Get Destination height
            //u32Temp = ((((pSrcBlk->y+pSrcBlk->height)<<12) - (u32InitDy & 0x1fff))*pdrawbuf->dstblk.height)/pdrawbuf->srcblk.height;
            u32Temp2 = (((((pSrcBlk->y+pSrcBlk->height)<<12) - (u32InitDy & 0xfff))<<8)/pScaleInfo->u32DeltaY)>>8;
            u32Temp3 = ((((pSrcBlk->y+pSrcBlk->height)<<12) - (u32Temp2*pScaleInfo->u32DeltaY + u32InitDy))<<12)/pScaleInfo->u32DeltaY;
            u32Temp = ((u32Temp2<<12)|(u32Temp3&0xfff));
            if ((u32Temp & 0xfff) < u32InverseDeltaY)
            {
                pSrcBlk->height++;
            }
            pDstBlk->height = ((u32Temp & 0xfffff000)>>12) - pDstBlk->y + 1;

            //Get x initial delta.
            if (bYStartMinus_1)
            {
                pSrcBlk->y--;
                pSrcBlk->height++;
            }
            u32Temp = pDstBlk->y*pScaleInfo->u32DeltaY + u32InitDy;
            //u32Temp = ((pDstBlk->y<<12)*pdrawbuf->srcblk.height)/pdrawbuf->dstblk.height + u32InitDy;
        }
        else
        {
            //Vertical up scaling
            //Get destination y
            if (pSrcBlk->y == 0)
            {
                pDstBlk->y = 0;
            }
            else
            {
                //u32Temp = (((pSrcBlk->y<<12) + ((~u32InitDy+1)&0xfff))*pdrawbuf->dstblk.height)/pdrawbuf->srcblk.height;
                u32Temp2 = ((((pSrcBlk->y<<12) + ((~u32InitDy+1)&0xfff))<<8)/pScaleInfo->u32DeltaY)>>8;
                u32Temp3 = ((((pSrcBlk->y<<12) - (u32Temp2*pScaleInfo->u32DeltaY - ((~u32InitDy+1)&0xfff))))<<12)/pScaleInfo->u32DeltaY;
                u32Temp = ((u32Temp2<<12)|(u32Temp3&0xfff));
                pDstBlk->y = (((u32Temp-u32InverseDeltaY)&0xfffff000)>>12)+1;
                bYStartMinus_1 = TRUE;
            }

            //Get Destination height
            //u32Temp = ((((pSrcBlk->y + pSrcBlk->height)<<12) + ((~u32InitDy+1) & 0x1fff))*pdrawbuf->dstblk.height)/pdrawbuf->srcblk.height;
            u32Temp2 = (((((pSrcBlk->y + pSrcBlk->height)<<12) + ((~u32InitDy+1)&0xfff))<<8)/pScaleInfo->u32DeltaY)>>8;
            u32Temp3 = (((((pSrcBlk->y + pSrcBlk->height)<<12) - (u32Temp2*pScaleInfo->u32DeltaY - ((~u32InitDy+1)&0xfff))))<<12)/pScaleInfo->u32DeltaY;
            u32Temp = ((u32Temp2<<12)|(u32Temp3&0xfff));
            pDstBlk->height = ((u32Temp & 0xfffff000)>>12) - pDstBlk->y  + 1;
            pSrcBlk->height++;

            //Get x initial delta.
            if (bYStartMinus_1)
            {
                pSrcBlk->y--;
                pSrcBlk->height++;
            }
            u32Temp = pDstBlk->y*pScaleInfo->u32DeltaY- ((~u32InitDy+1)&0xfff);
            //u32Temp = ((pDstBlk->y<<12)*pdrawbuf->srcblk.height)/pdrawbuf->dstblk.height - ((~u32InitDy+1)&0xfff);
        }

        if (pDstBlk->y == 0)
        {
            pScaleInfo->u32InitDelatY = u32InitDy;
        }
        else
        {
            if (u32Temp > (pSrcBlk->y<<12))
            {
                if ((u32Temp - (pSrcBlk->y<<12)) < 0x1000)
                {
                    pScaleInfo->u32InitDelatY = (u32Temp- (pSrcBlk->y<<12));
                }
                else
                {
                    //Invalid destination Y position.
                    //printk("+ ,Invalid destination Y position.\n");
                    pScaleInfo->u32InitDelatY = 0xfff;
                }
            }
            else
            {
                //printk("- ,Invalid destination Y direction.\n");
                pScaleInfo->u32InitDelatY = 0;
            }
        }
    }

    return GESTATUS_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
/// Set PE draw bitmap
/// @param handle \b IN: handle of bitmap
/// @param pbmpfmt \b IN: bitmap format
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
/// @return GESTATUS_INVALID_BMP_HANDLE - Invalid bitmap handle
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_DrawBitmap(BMPHANDLE handle, GE_DRAW_BMP_INFO *pbmpfmt)
{
    U32 u32Width, u32Height;
    U32 u32Value, u32Value2, u32Addr, u32Pitch;
#if GE_AUTO_ALIGN_SRC_BUFFER
    U8 *pTempBuf=NULL;
#endif

    GE_DEBUGINFO( printk( "MHal_GE_DrawBitmap\n" ) );
    GE_WaitAvailableCMDQueue(24);

    if (!_BitmapTable[handle].inUsed)
    {
        return GESTATUS_INVALID_BMP_HANDLE;
    }

    if (!GE_CheckInClipWindow(pbmpfmt->x, pbmpfmt->y, pbmpfmt->x+pbmpfmt->width-1, pbmpfmt->y+pbmpfmt->height-1))
    {
        return GESTATUS_FAIL;
    }

    u32Value = GE_ReadReg(GE_REG_SB_DB_MODE);
    u32Value &= ~(GE_MSK_SB_FMT);

#if 0
    switch(_BitmapTable[handle].fmt)
    {
    case 0://E_GOP_COLOR_RGB555:
        u32Value |= GE_VAL_SB_FMT_ORGB1555;
        break;
    case 1://E_GOP_COLOR_RGB565:
        u32Value |= GE_VAL_SB_FMT_RGB565;
        break;
    case 2://E_GOP_COLOR_ARGB4444:
        u32Value |= GE_VAL_SB_FMT_ARGB4444;
        break;
    case 3://E_GOP_COLOR_BLINK:
        u32Value |= GE_VAL_SB_FMT_1BAAFGBG123433;
        break;
    case 4://E_GOP_COLOR_I8:
        u32Value |= GE_VAL_SB_FMT_I8;
        break;
    case 5://E_GOP_COLOR_ARGB8888:
        u32Value |= GE_VAL_SB_FMT_ARGB8888;
        break;
    default:
        u32Value |= GE_VAL_SB_FMT_ORGB1555;
        break;
    }
#endif

    GE_WriteReg(GE_REG_SB_DB_MODE, (u32Value|_BitmapTable[handle].fmt));

    u32Value2 = GE_ReadReg(GE_REG_FMT_BLT);

    u32Addr   = (_BitmapTable[handle].addr ) ;
    u32Width  = _BitmapTable[handle].width;
    u32Height = _BitmapTable[handle].height;
    u32Pitch  = _BitmapTable[handle].pitch;

#if GE_AUTO_ALIGN_SRC_BUFFER
    if((u32Addr % 16) || (u32Pitch % 16))
    {
        // If the bitmap pitch is not 16-byte alignment, allocate a 16-byte memory to do it.
        U8 *pSrcBuf;
        U32 u32PitchNew;
        U32 i;

        u32PitchNew = ALIGN_16(u32Pitch);
        pTempBuf = MsOS_AllocateMemory(u32PitchNew * u32Height, gs32NonCachedPoolID);
        if(!pTempBuf)
        {
            return GESTATUS_NON_ALIGN_PITCH;
        }
        pSrcBuf = (U8*)u32Addr;
        for(i=0;i<u32Height;i++)
        {
            memcpy(&pTempBuf[i*u32PitchNew], &pSrcBuf[i*u32Pitch], u32Pitch);
        }
        u32Addr  = (U32)pTempBuf;
        u32Pitch = u32PitchNew;
    }
#endif
//printk("MHal_GE_DrawBitmap: source addr %lx \n",u32Addr);
    // Set source address
    GE_WriteReg(GE_REG_SB_BASE0, u32Addr & 0xffff);
    GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr(u32Addr) >> 16);

    // Set source pitch
    GE_WriteReg(GE_REG_SB_PIT, u32Pitch);

    GE_WriteReg(GE_REG_STBB_WIDTH, u32Width);
    GE_WriteReg(GE_REG_STBB_HEIGHT, u32Height);

    // Set source coordinate
    GE_WriteReg(GE_REG_PRI_V2_X, 0);
    GE_WriteReg(GE_REG_PRI_V2_Y, 0);

    //------------------------------------------------------------
    // BLT scale delta value
    //------------------------------------------------------------
    if ((u32Width != pbmpfmt->width) || (u32Height != pbmpfmt->height))
    {
        if (pbmpfmt->bScale == true)
        {
			S32 s32Temp1, s32Temp2;
			U32 u32Value3, u32ValueFix = u32Width;
		
			if ( pbmpfmt->width < (u32Width >> 1) )
				u32ValueFix = u32Width - 1;		
			
            u32Value = Divide2Fixed(u32ValueFix, pbmpfmt->width, 5, 12);//<< 2 ; //sc
            GE_WriteReg(GE_REG_STBB_DX, u32Value & 0xffff);
			s32Temp1 = u32ValueFix - pbmpfmt->width;
			s32Temp2 = 2* pbmpfmt->width;
			s32Temp1 = s32Temp1 % s32Temp2;
			u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
			if ( u32Value & 0x00010000 )
			{
				u32Value3 |= GE_VAL_STBB_DX_MSB;
			}
			else
			{
				u32Value3 &= ~GE_VAL_STBB_DX_MSB;
			}
			GE_WriteReg(GE_REG_STBB_INIT_DX, u32Value3);

			if ( pbmpfmt->height < (u32Height >> 1) )
				u32ValueFix = u32Height - 1;
			else
				u32ValueFix = u32Height;
			
            u32Value = Divide2Fixed(u32ValueFix, pbmpfmt->height, 5, 12);// << 2 ; //sc
            GE_WriteReg(GE_REG_STBB_DY, u32Value & 0xffff);
			s32Temp1 = u32ValueFix - pbmpfmt->height;
			s32Temp2 = 2* pbmpfmt->height;
			s32Temp1 = s32Temp1 % s32Temp2;
			u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
			if ( u32Value & 0x00010000 )
			{
				u32Value3 |= GE_VAL_STBB_DY_MSB;
			}
			else
			{
				u32Value3 &= ~GE_VAL_STBB_DY_MSB;
			}
			GE_WriteReg(GE_REG_STBB_INIT_DY, u32Value3);

            u32Width = pbmpfmt->width;
            u32Height = pbmpfmt->height;
            u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
        }
        else
        {
            u32Width = pbmpfmt->width;
            u32Height = pbmpfmt->height;
            GE_WriteReg(GE_REG_STBB_DX, 0x1000);
            GE_WriteReg(GE_REG_STBB_DY, 0x1000);
            GE_WriteReg(GE_REG_STBB_INIT_DX, 0);
            GE_WriteReg(GE_REG_STBB_INIT_DY, 0);
            u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
        }
    }
    else
    {
        GE_WriteReg(GE_REG_STBB_DX, 0x1000);
        GE_WriteReg(GE_REG_STBB_DY, 0x1000);
        u32Value2 &= ~GE_VAL_EN_STRETCH_BITBLT;
    }
/*
    if((pbmpfmt->bScale == true) &&
       (u32Width != pbmpfmt->width ||
        u32Height != pbmpfmt->height))
    {
        u32Value = Divide2Fixed(u32Width, pbmpfmt->width, 1, 12);//<< 2 ; //sc
        GE_WriteReg(GE_REG_STBB_DX, u32Value & 0xffff);
        u32Value = Divide2Fixed((u32Width - pbmpfmt->width), 2* pbmpfmt->width, 1, 12);// << 2; //sc
        //u32Value = Divide2Fixed(u32Width, 2*pbmpfmt->width, 1, 12);
        //u32Value = u32Value - 0x0800;      // - 0.5
        GE_WriteReg(GE_REG_STBB_INIT_DX, u32Value);


        u32Value = Divide2Fixed(u32Height, pbmpfmt->height, 1, 12);// << 2 ; //sc
        GE_WriteReg(GE_REG_STBB_DY, u32Value & 0xffff);
        u32Value = Divide2Fixed((u32Height- pbmpfmt->height), 2* pbmpfmt->height, 1, 12);// << 2; //sc
        //u32Value = Divide2Fixed(u32Height, 2*pbmpfmt->height, 1, 12);
        //u32Value = u32Value - 0x0800;      // - 0.5
        GE_WriteReg(GE_REG_STBB_INIT_DY, u32Value);

        //scale = (U16)((float)(u32Width-1) * ((float)pbmpfmt->width / (float)u32Width));//TODO
//        u32Scale = (((U32)pbmpfmt->width << 5) / u32Width * (u32Width-1)) >> 5; //sc
//        u32Width = u32Scale;  //pbmpfmt->width;
        u32Width = pbmpfmt->width;

        //scale = (U16)((float)(u32Height-1) * ((float)pbmpfmt->height / (float)u32Height));//TODO
//        u32Scale = (((U32)pbmpfmt->height << 5) / u32Height * (u32Height-1)) >> 5; //sc
//        u32Height = u32Scale;  //pbmpfmt->height;
        u32Height = pbmpfmt->height;
        u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;

    }
    else
    {
        GE_WriteReg(GE_REG_STBB_DX, 0x1000);
        GE_WriteReg(GE_REG_STBB_DY, 0x1000);
        u32Value2 &= ~GE_VAL_EN_STRETCH_BITBLT;
    }
*/
    GE_WriteReg(GE_REG_FMT_BLT, u32Value2);

    if (u32Value2 & GE_VAL_EN_STRETCH_BITBLT)
    {
        _u32Reg60hFlag |= GE_VAL_STBB_PATCH;
    }

    //------------------------------------------------------------
    // Destination coordinate
    //------------------------------------------------------------
    GE_WriteReg(GE_REG_PRI_V0_X, pbmpfmt->x);
    GE_WriteReg(GE_REG_PRI_V0_Y, pbmpfmt->y);
    GE_WriteReg(GE_REG_PRI_V1_X, pbmpfmt->x + u32Width - 1);
    GE_WriteReg(GE_REG_PRI_V1_Y, pbmpfmt->y + u32Height - 1);

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_X_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_X, _BitmapTable[handle].width - 1);
    }

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_Y_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_Y, _BitmapTable[handle].height - 1);
    }

    //_u32Reg60hFlag = (_u32Reg60hFlag | GE_VAL_STBB_PATCH);

    // @FIXME
    // check GE_REG_SB_DB_MODE

    GE_WriteReg(GE_REG_CMD, (GE_VAL_PRIM_BITBLT|_u32Reg60hFlag));

#if GE_AUTO_ALIGN_SRC_BUFFER
    if(pTempBuf)
    {
        GE_WaitCmdEmpty();
        while ( GE_Reg(GE_REG_STATUS)& GE_VAL_BUSY)
        {
            MsOS_YieldTask();
        }
        MsOS_FreeMemory(pTempBuf, gs32NonCachedPoolID);
        // Set source address
//        GE_WriteReg(GE_REG_SB_BASE0, _BitmapTable[handle].addr & 0xffff);
//        GE_WriteReg(GE_REG_SB_BASE1, _BitmapTable[handle].addr >> 16);
//
//        // Set source pitch
//        GE_WriteReg(GE_REG_SB_PIT, _BitmapTable[handle].pitch);
    }
#endif
    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE Screen to Screen bitblt
/// @param psrcblk \b IN: pointer of source block
/// @param pdstblk \b IN: pointer of destination block
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_ScreenCopy(GE_BLOCK *psrcblk, GE_BLOCK *pdstblk)
{
    //U32 u32Addr;
    U32 u32Scale, u32Value, u32Value2, u32Value3;
	S32 s32Temp1, s32Temp2;
	
    GE_DEBUGINFO( printk( "MHal_GE_ScreenCopy\n" ) );
    GE_WaitAvailableCMDQueue(25);

    if (!GE_CheckInClipWindow(pdstblk->x, pdstblk->y,
                              pdstblk->x+pdstblk->width-1, pdstblk->y+pdstblk->height-1))
    {
        return GESTATUS_FAIL;
    }

    //u32Value  = GE_ReadReg(GE_REG_EN);
    //u32Value |= GE_VAL_EN_PE;
    //GE_WriteReg(GE_REG_EN, u32Value);

    u32Value2 = GE_ReadReg(GE_REG_FMT_BLT);

    GE_WriteReg(GE_REG_STBB_WIDTH, psrcblk->width);
    GE_WriteReg(GE_REG_STBB_HEIGHT, psrcblk->height);

    // Set stretch delta
    if(psrcblk->width != pdstblk->width)
    {
		U32 u32ValueFix = psrcblk->width;

		if ( pdstblk->width < (psrcblk->width >> 1) )
			u32ValueFix = psrcblk->width - 1;
			
        u32Value = Divide2Fixed(u32ValueFix, pdstblk->width, 5, 12);// << 2; //sc
        u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
		s32Temp1 = u32ValueFix - pdstblk->width;
		s32Temp2 = 2* pdstblk->width;
		s32Temp1 = s32Temp1 % s32Temp2;
        u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
		if ( u32Value & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DX_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DX_MSB;
		}
    }
    else
    {
        u32Value = 0x1000;
        u32Value2 &= ~GE_VAL_EN_STRETCH_BITBLT;
        u32Value3 = 0;
     }
    GE_WriteReg(GE_REG_STBB_DX, u32Value & 0xffff);
    GE_WriteReg(GE_REG_STBB_INIT_DX,u32Value3);

    if(psrcblk->height != pdstblk->height)
    {
		U32 u32ValueFix = psrcblk->height;
		
		if ( pdstblk->height < (psrcblk->height >> 1) )
			u32ValueFix = psrcblk->height - 1;
			
        u32Value = (U16)Divide2Fixed(u32ValueFix, pdstblk->height, 5, 12);//<<2; //sc
        u32Value2 |= GE_VAL_EN_STRETCH_BITBLT;
		s32Temp1 = u32ValueFix - pdstblk->height;
		s32Temp2 = 2* pdstblk->height;
		s32Temp1 = s32Temp1 % s32Temp2;
        u32Value3 = Divide2Fixed(s32Temp1, s32Temp2, 0, 12);// << 2; //sc
		if ( u32Value & 0x00010000 )
		{
			u32Value3 |= GE_VAL_STBB_DY_MSB;
		}
		else
		{
			u32Value3 &= ~GE_VAL_STBB_DY_MSB;
		}
    }
    else
    {
        u32Value = 0x1000;
        u32Value2 &= ~GE_VAL_EN_STRETCH_BITBLT;
    }
    GE_WriteReg(GE_REG_STBB_DY, u32Value & 0xffff);
    GE_WriteReg(GE_REG_STBB_INIT_DY,u32Value3);

    GE_WriteReg(GE_REG_FMT_BLT, u32Value2);

    // Source coordinate
    GE_WriteReg(GE_REG_PRI_V2_X, psrcblk->x);
    GE_WriteReg(GE_REG_PRI_V2_Y, psrcblk->y);

    if ((psrcblk->x <= pdstblk->x) && (pdstblk->x <= (psrcblk->x + psrcblk->width)) &&
        (psrcblk->y <= pdstblk->y) && (pdstblk->y <= (psrcblk->y + psrcblk->height)))
    {
        _u32Reg60hFlag |= (GE_VAL_DRAW_DST_DIR_X_NEG | GE_VAL_DRAW_DST_DIR_Y_NEG | GE_VAL_DRAW_SRC_DIR_X_NEG | GE_VAL_DRAW_SRC_DIR_Y_NEG);
    }
    if ((psrcblk->x <= (pdstblk->x + pdstblk->width)) && ((pdstblk->x + pdstblk->width) <= (psrcblk->x + psrcblk->width)) &&
        (psrcblk->y <= pdstblk->y) && (pdstblk->y <= (psrcblk->y + psrcblk->height)))
    {
        _u32Reg60hFlag |= (GE_VAL_DRAW_DST_DIR_Y_NEG | GE_VAL_DRAW_SRC_DIR_Y_NEG);
    }


    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_X_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_X, psrcblk->x + psrcblk->width - 1);
    }

    if (_u32Reg60hFlag & GE_VAL_DRAW_SRC_DIR_Y_NEG)
    {
        GE_WriteReg(GE_REG_PRI_V2_Y, psrcblk->y + psrcblk->height - 1);
    }


    // Set source pitch buffer information with destination buffer.
    GE_WriteReg(GE_REG_SB_PIT, GE_ReadReg(GE_REG_DB_PIT));
    GE_WriteReg(GE_REG_SB_BASE0, GE_ReadReg(GE_REG_DB_BASE0));
    GE_WriteReg(GE_REG_SB_BASE1, GE_ReadReg(GE_REG_DB_BASE1));
    u32Value = GE_ReadReg(GE_REG_SB_DB_MODE);
    u32Value2 = (u32Value & GE_MSK_DB_FMT) | ((u32Value & GE_MSK_DB_FMT)>>8);
    GE_WriteReg(GE_REG_SB_DB_MODE, u32Value2);


    // Destination coordinate
    if (_u32Reg60hFlag & GE_VAL_DRAW_DST_DIR_X_NEG)
    {
        u32Scale = (((U32)pdstblk->width << 5) / psrcblk->width * (psrcblk->width-1)) >> 5; //sc
        GE_WriteReg(GE_REG_PRI_V0_X, pdstblk->x + u32Scale);
        GE_WriteReg(GE_REG_PRI_V1_X, pdstblk->x);
    }
    else
    {
        GE_WriteReg(GE_REG_PRI_V0_X, pdstblk->x);
        u32Scale = (((U32)pdstblk->width << 5) / psrcblk->width * (psrcblk->width-1)) >> 5; //sc
        GE_WriteReg(GE_REG_PRI_V1_X, pdstblk->x + u32Scale);
    }

    if (_u32Reg60hFlag & GE_VAL_DRAW_DST_DIR_Y_NEG)
    {
        u32Scale = (((U32)pdstblk->height << 5) / psrcblk->height * (psrcblk->height-1)) >> 5; //sc
        GE_WriteReg(GE_REG_PRI_V0_Y, pdstblk->y + u32Scale);
        GE_WriteReg(GE_REG_PRI_V1_Y, pdstblk->y);
    }
    else
    {
        GE_WriteReg(GE_REG_PRI_V0_Y, pdstblk->y);
        u32Scale = (((U32)pdstblk->height << 5) / psrcblk->height * (psrcblk->height-1)) >> 5; //sc
        GE_WriteReg(GE_REG_PRI_V1_Y, pdstblk->y + u32Scale);
    }
/*
    GE_WriteReg(GE_REG_PRI_V0_X, pdstblk->x);
    GE_WriteReg(GE_REG_PRI_V0_Y, pdstblk->y);
    u32Scale = (((U32)pdstblk->width << 5) / psrcblk->width * (psrcblk->width-1)) >> 5; //sc
    GE_WriteReg(GE_REG_PRI_V1_X, pdstblk->x + u32Scale);
    u32Scale = (((U32)pdstblk->height << 5) / psrcblk->height * (psrcblk->height-1)) >> 5; //sc
    GE_WriteReg(GE_REG_PRI_V1_Y, pdstblk->y + u32Scale);
*/
    // @FIXME
    // check GE_REG_SB_DB_MODE

    u32Value = GE_VAL_PRIM_BITBLT;

    GE_WriteReg(GE_REG_CMD, u32Value|_u32Reg60hFlag);

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE clipping window
/// @param v0 \b IN: left-top position
/// @param v1 \b IN: right-down position
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetClip(GE_POINT_t* v0, GE_POINT_t* v1)
{
	GE_DEBUGINFO(printk("MHal_GE_SetClip\n"));
    GE_WaitAvailableCMDQueue(8);
    GE_WriteReg(GE_REG_CLIP_LEFT, v0->x);
    GE_WriteReg(GE_REG_CLIP_TOP, v0->y);
    //GE_WriteReg(GE_REG_CLIP_RIGHT, v1->x - 1);
    //GE_WriteReg(GE_REG_CLIP_BOTTOM, v1->y - 1);
    GE_WriteReg(GE_REG_CLIP_RIGHT, v1->x);
    GE_WriteReg(GE_REG_CLIP_BOTTOM, v1->y);
    _u16PeClipX1 = v0->x;
    _u16PeClipY1 = v0->y;
    _u16PeClipX2 = v1->x;
    _u16PeClipY2 = v1->y;

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE italic
/// @param enable \b IN: true/false
/// @param ini_line \b IN: initial line : default is 0, indicate which line start to get italic effect
/// @param ini_dis \b IN: initial distance : default is 0, indicate which pixel start to get italic effect
/// @param delta \b IN: italic delta 0-0x1f
///                     - D[4]: 0/1  left/right italic
///                     - D[3]-D[0] : delta value of italic
/// @image html Text_Italic.JPG "in_line & ini_dis"
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
/// @note
/// The italic process can't perform with rotate process or mirror process.
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetItalic(U16 enable,U8 ini_line, U8 ini_dis, U8 delta)
{
    U32 u32Value;
	GE_DEBUGINFO(printk("MHal_GE_SetItalic\n"));	
    GE_WaitAvailableCMDQueue(6);

    u32Value = GE_ReadReg(GE_REG_FMT_BLT);
    if (enable)
    {
        u32Value |= GE_VAL_EN_ITALIC_FONT;
        // dis : D[7] - D[0] line : D[15] - D[8]
        GE_WriteReg(GE_REG_ITC_DIS_LINE, ((ini_line<<8)|ini_dis));
        //D[4] - D[0] D[4] is signed bit
        GE_WriteReg(GE_REG_ITC_DELTA, delta&0x1f);
    }
    else
    {
        u32Value &= ~GE_VAL_EN_ITALIC_FONT;
    }

    GE_WriteReg(GE_REG_FMT_BLT, u32Value);

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE rotate
/// @param angle \b IN: rotate angle
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
/// @note
/// The rotate process can't perform with italic process.
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetRotate(GEROTATE_ANGLE angle)
{
    U32 u32Value;
	GE_DEBUGINFO(printk("MHal_GE_SetRotate\n"));	
    GE_WaitAvailableCMDQueue(4);

    u32Value = GE_ReadReg(GE_REG_ROT_MIRROR);
    u32Value &= ~GE_MSK_ROT;
    u32Value |= (U32)angle;
    GE_WriteReg(GE_REG_ROT_MIRROR, u32Value);

    return GESTATUS_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
/// Set PE alpha source
/// @param eMode \b IN: alpha source come from , this indicate alpha channel output source
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetAlphaSrcFrom( GE_ALPHA_SRC_FROM eMode )
{
    U32 u32Value;
	GE_DEBUGINFO(printk("MHal_GE_SetAlphaScrFrom\n"));	
    GE_WaitAvailableCMDQueue(4);

    u32Value = GE_ReadReg(GE_REG_DB_ABL);

    u32Value = ((u32Value & (~GE_MSK_DB_ABL))| (eMode<<8));
    GE_WriteReg(GE_REG_DB_ABL, u32Value);

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
// Description:
// Arguments:   eMode : ABL_FROM_CONST, ABL_FROM_ASRC, ABL_FROM_ADST
//              blendcoef : COEF_ONE,  COEF_CONST,   COEF_ASRC,   COEF_ADST
//                          COEF_ZERO, COEF_1_CONST, COEF_1_ASRC, COEF_1_ADST
//              blendfactor : value : [0,0xff]
// Return:      NONE
//
// Notes:       if any
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/// Set PE alpha blending. Dst = A * Src + (1 - A) Dst
/// @param blendcoef       \b IN: alpha source from
/// @param u8ConstantAlpha \b IN: Contant alpha when blendcoef is equal to COEF_CONST
///                               or COEF_1_CONST.
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetAlphaBlending(GE_BLEND_COEF blendcoef, U8 u8ConstantAlpha)
{
    U32 u32Value;
	GE_DEBUGINFO(printk("MHal_GE_SetAlphaBlending\n"));	
    GE_WaitAvailableCMDQueue(5);

    u32Value = GE_ReadReg(GE_REG_ABL_COEF);
    u32Value &= ~(GE_MSK_ABL_COEF);
    u32Value |= blendcoef ;
    GE_WriteReg(GE_REG_ABL_COEF, u32Value);

    GE_WriteReg(GE_REG_ABL_CONST, (U32)(u8ConstantAlpha&0xff));

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Enable PE alpha blending
/// @param enable \b IN: true/false
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_EnableAlphaBlending(U16 enable)
{
    U32 u32Value;
	GE_DEBUGINFO(printk("MHal_GE_EnableAlphaBlending\n"));	
    GE_WaitAvailableCMDQueue(4);

#if PATCH_RD_CMD
    u32Value = _u32Reg0hValue;
#else
    u32Value = GE_ReadReg(GE_REG_EN);
#endif

    if(enable)
    {
        u32Value |= GE_VAL_EN_GY_ABL;
        GE_WriteReg(GE_REG_EN, u32Value);
    }
    else
    {
        u32Value &= ~GE_VAL_EN_GY_ABL;
        GE_WriteReg(GE_REG_EN, u32Value);
    }

#if PATCH_RD_CMD
    _u32Reg0hValue = u32Value;
#endif

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Enable PE mirror
/// @param isMirrorX \b IN: true/false
/// @param isMirrorY \b IN: true/false
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
/// @note
/// The mirror process can't perform on the source format is GE_FMT_I1, GE_FMT_I2 or GE_FMT_I4.
/// The mirror process can't perform with italic process.
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetMirror(U16 isMirrorX, U16 isMirrorY)
{
    _u32Reg60hFlag  = (_u32Reg60hFlag & ~(GE_VAL_DRAW_SRC_DIR_X_NEG|GE_VAL_DRAW_SRC_DIR_Y_NEG));
    _u32Reg60hFlag |= ( (isMirrorX << 7) | (isMirrorY << 8) );

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Enable PE NearestMode
/// @param enable \b IN: true/false
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetNearestMode(U16 enable)
{
    _u32Reg60hFlag  = (_u32Reg60hFlag & ~(GE_VAL_STBB_NEAREST));
    _u32Reg60hFlag |= (enable << 14);

    return GESTATUS_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
/// Set PE source buffer info
/// @param bufInfo \b IN: buffer handle
/// @param offsetofByte \b IN: start offset (should be 128 bit aligned)
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
/// @return GESTATUS_NON_ALIGN_PITCH - The pitch is not 16 bytes alignment
/// @return GESTATUS_NON_ALIGN_ADDRESS - The address is not 16 bytes alignment
/// @note
/// The buffer start address must be 128 bits alignment.
/// In GE_FMT_I1, GE_FMT_I2 and GE_FMT_I4 format, the pitch must be 8 bits alignment.
/// In other format, the pitch must be 128 bits alignment.
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetSrcBufferInfo(PGE_BUFFER_INFO bufInfo, U32 offsetofByte)
{
    U32 u32Value , u32Addr;

	GE_DEBUGINFO(printk("MHal_GE_SetSrcBufferInfo\n"));	
    GE_WaitAvailableCMDQueue(8);

    _GE_CHECK_BUFFER_ALIGN1(
        bufInfo->u32Addr + offsetofByte,
        bufInfo->u32Width,
        bufInfo->u32Height,
        bufInfo->u32Pitch,
        bufInfo->u32ColorFmt);

#ifndef GE_ADDR_RELAX
#if !GE_AUTO_ALIGN_SRC_BUFFER
    if ((bufInfo->u32ColorFmt >= GE_FMT_I8) && (bufInfo->u32ColorFmt >= GE_FMT_ARGB8888))
    {
        if (bufInfo->u32Pitch%16)
        {
            return GESTATUS_NON_ALIGN_PITCH;
        }
    }
    if ((bufInfo->u32Addr + offsetofByte)%16)
    {
        return GESTATUS_NON_ALIGN_ADDRESS;
    }
#endif
#endif // #ifdef GE_ADDR_RELAX

    u32Value = GE_ReadReg(GE_REG_SB_DB_MODE);
    u32Value = (u32Value & ~GE_MSK_SB_FMT) | (bufInfo->u32ColorFmt); // one-bit format
    GE_WriteReg(GE_REG_SB_DB_MODE, u32Value);


    // Set source address
    u32Addr   = (bufInfo->u32Addr + offsetofByte);
    GE_WriteReg(GE_REG_SB_BASE0, u32Addr & 0xffff);
    GE_WriteReg(GE_REG_SB_BASE1, Phy_to_HAL_Addr(u32Addr) >> 16);

    // Set source pitch
    GE_WriteReg(GE_REG_SB_PIT, (bufInfo->u32Pitch));

#if GE_AUTO_ALIGN_SRC_BUFFER
    _u32SrcAddrPE = bufInfo->u32Addr;
    _u32SrcPitchPE = bufInfo->u32Pitch;
    _u32SrcFmtPE = bufInfo->u32ColorFmt;
    _u32OffsetSrc = offsetofByte;
#endif

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Set PE destination buffer info
/// @param bufInfo \b IN: buffer handle
/// @param offsetofByte \b IN: start offset (should be 128 bit aligned)
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
/// @return GESTATUS_NON_ALIGN_PITCH - The pitch is not 16 bytes alignment
/// @return GESTATUS_NON_ALIGN_ADDRESS - The address is not 16 bytes alignment
/// @note
/// The buffer start address and pitch smust be 128 bits alignment.
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_SetDstBufferInfo(PGE_BUFFER_INFO bufInfo, U32 offsetofByte)
{
    U32 u32Value;
    U32 u32Addr = (bufInfo->u32Addr + offsetofByte );

	GE_DEBUGINFO(printk("MHal_GE_SetDstBuferInfo\n"));	
    GE_WaitAvailableCMDQueue(8);

    _GE_CHECK_BUFFER_ALIGN1(
        bufInfo->u32Addr + offsetofByte,
        bufInfo->u32Width,
        bufInfo->u32Height,
        bufInfo->u32Pitch,
        bufInfo->u32ColorFmt);

#ifndef GE_ADDR_RELAX
    if (bufInfo->u32Pitch%16)
    {
        return GESTATUS_NON_ALIGN_PITCH;
    }
    if (u32Addr%16)
    {
        return GESTATUS_NON_ALIGN_ADDRESS;
    }
#endif // #ifdef GE_ADDR_RELAX

    // Destination Buffer
    GE_WriteReg(GE_REG_DB_PIT, bufInfo->u32Pitch);                      // Pitch
    GE_WriteReg(GE_REG_DB_BASE0, u32Addr & 0xffff);     // Address
    GE_WriteReg(GE_REG_DB_BASE1, Phy_to_HAL_Addr(u32Addr) >> 16);        // Address

    if( GE_FMT_ARGB1555==bufInfo->u32ColorFmt )
        bufInfo->u32ColorFmt = GE_FMT_ARGB1555_DST ;

    // Destination frame buffer format
    u32Value = GE_ReadReg(GE_REG_SB_DB_MODE);
    u32Value = (u32Value & ~GE_MSK_DB_FMT) | (bufInfo->u32ColorFmt<<8); // one-bit format
    GE_WriteReg(GE_REG_SB_DB_MODE, u32Value);

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_GetFrameBufferInfo(U32 *width,
                                       U32 *height,
                                       U32 *pitch,
                                       U32 *fbFmt,
                                       U32 *addr)
{
    // TODO: width & height have not been retrieved
    U32 u32Value;
	GE_DEBUGINFO(printk("MHal_GE_GetFrameBufferInfo\n"));	
    GE_WaitAvailableCMDQueue(31);

    u32Value = GE_ReadReg(GE_REG_DB_PIT);
    *pitch = u32Value;

    u32Value = GE_ReadReg(GE_REG_SB_DB_MODE);
    *fbFmt = ((u32Value & GE_MSK_DB_FMT)>>8);

    u32Value = GE_ReadReg(GE_REG_DB_BASE0);
    *addr = u32Value;
    u32Value = GE_ReadReg(GE_REG_DB_BASE1);
    *addr |= (u32Value << 16);

	HAL_TO_PHY_ADDR(*addr);

	//This needs to fine tune again

    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_YUV_Set(GE_YUV_INFO* pYuvInfo)
{
    U32                         u32YuvInfo= 0;

    GE_ASSERT(pYuvInfo, printk("[GE DRV][%06d] NULL pointer\n", __LINE__));
    // if (bEnable){
    u32YuvInfo|=                (pYuvInfo->rgb2yuv_mode)? GE_VAL_RGB2YUV_255: GE_VAL_RGB2YUV_PC;
    u32YuvInfo|=                (pYuvInfo->yuv_range_out)? GE_VAL_YUV_RANGE_OUT_PC: GE_VAL_YUV_RANGE_OUT_255;
    u32YuvInfo|=                (pYuvInfo->yuv_range_in)? GE_VAL_YUV_RANGE_IN_127: GE_VAL_YUV_RANGE_IN_255;
    u32YuvInfo|=                ((pYuvInfo->yuv_mem_fmt_src)<< GE_SHFT_YUV_MEM_FMT_SRC);
    u32YuvInfo|=                ((pYuvInfo->yuv_mem_fmt_dst)<< GE_SHFT_YUV_MEM_FMT_DST);
    // }
    GE_WriteReg(GE_REG_YUV, u32YuvInfo);
    return GESTATUS_SUCCESS;
}

GESTATUS MHal_GE_YUV_Get(GE_YUV_INFO* pYuvInfo)
{
    U32                         u32YuvInfo;

    GE_ASSERT(pYuvInfo, printk("[GE DRV][%06d] NULL pointer\n", __LINE__));
    u32YuvInfo=                 GE_ReadReg(GE_REG_YUV);
    pYuvInfo->rgb2yuv_mode=     (u32YuvInfo & GE_VAL_RGB2YUV_255)? GE_RGB2YUV_255_MODE: GE_RGB2YUV_PC_MODE;
    pYuvInfo->yuv_range_out=    (u32YuvInfo & GE_VAL_YUV_RANGE_OUT_PC)? GE_YUV_RANGE_OUT_PC: GE_YUV_RANGE_OUT_255;
    pYuvInfo->yuv_range_in=     (u32YuvInfo & GE_VAL_YUV_RANGE_IN_127)? GE_YUV_RANGE_IN_127: GE_YUV_RANGE_IN_255;
    pYuvInfo->yuv_mem_fmt_src=  (u32YuvInfo & GE_MASK_YUV_MEM_FMT_SRC) >> GE_SHFT_YUV_MEM_FMT_SRC;
    pYuvInfo->yuv_mem_fmt_dst=  (u32YuvInfo & GE_MASK_YUV_MEM_FMT_DST) >> GE_SHFT_YUV_MEM_FMT_DST;
    return GESTATUS_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
void GE_ConvertRGB2DBFmt(GE_Buffer_Format Fmt, U32 *colorinfo, U16* low, U16* high)
{
    GE_RGB_COLOR   *color = NULL;
    GE_BLINK_DATA  *blinkData = NULL;
    //U8 a, r, g, b;

    // @FIXME: Richard uses GE_FMT_1ABFGBG12355 instead
    if (GE_FMT_1ABFGBG12355== Fmt)
    // if (Fmt == GE_FMT_1BAAFGBG123433)
    {
        blinkData =(GE_BLINK_DATA *)colorinfo;
    }
    else
    {
        color     =(GE_RGB_COLOR  *)colorinfo;
    }

    switch (Fmt)
    {
        case GE_FMT_RGB565 :
            *low = ((color->b & 0xf8) + (color->b >> 5)) | (((color->g & 0xfc) + (color->g>>6))<<8);
            *high = ((color->r & 0xf8) + (color->r >> 5)) | ((color->a & 0xff) << 8);
            break;
        case GE_FMT_ARGB1555 :
            *low = ((color->b & 0xf8) + (color->b >> 5)) | (((color->g & 0xf8) + (color->g >> 5))<<8);
            if (color->a > 0)
            {
                *high = ((color->r & 0xf8) + (color->r >> 5)) | (0xff << 8);
            }
            else
            {
                *high = ((color->r & 0xf8) + (color->r >> 5));
            }
            break;
        case GE_FMT_ARGB4444 :
            *low = ((color->b & 0xf0) + (color->b >> 4)) | (((color->g & 0xf0) + (color->g >> 4))<<8);
            *high = ((color->r & 0xf0) + (color->r >> 4)) | (((color->a & 0xf0) + (color->a >> 4))<<8);
            break;
        case GE_FMT_ARGB8888 :
            *low  =(color->b & 0xff) | ((color->g & 0xff) << 8);
            *high =(color->r & 0xff) | ((color->a & 0xff) << 8);
            break;
        case GE_FMT_I8 :
            *low  = (color->b & 0xff)|((color->b & 0xff) << 8);
            *high = (color->b & 0xff)|((color->b & 0xff) << 8);
            break;
        // @FIXME: Richard uses GE_FMT_1ABFGBG12355 instead
        //          1 A B Fg Bg
        //          1 2 3  5  5
        case GE_FMT_1ABFGBG12355:
            *low = (0x1f & blinkData->background) |                     // Bg: 4..0
                   ((0x1f & blinkData->foreground)<< 5) |               // Fg: 9..5
                   ((0x1f & blinkData->ctrl_flag)<< 10) |               // [A, B]: [14..13, 12..10]
                   BIT15;                                               // Bit 15
            *high= (0x1f & blinkData->background) |                     // Bg: 4..0
                   ((0x1f & blinkData->foreground)<< 5) |               // Fg: 9..5
                   ((0x1f & blinkData->ctrl_flag)<< 10) |               // [A, B]: [14..13, 12..10]
                   BIT15;                                               // Bit 15
            break;
#if 0
                    1 B A A Fg Bg
                    1 2 3 4  3  3
        case GE_FMT_1BAAFGBG123433 :
            *low = ((blinkData->background & 0x7)|((blinkData->foreground & 0x7) << 3)|((blinkData->ctrl_flag & 0x1ff)<<6)|(0x1 << 15));
            *high = ((blinkData->background & 0x7)|((blinkData->foreground & 0x7) << 3)|((blinkData->ctrl_flag & 0x1ff)<<6)|(0x1 << 15));
            break;
#endif
        case GE_FMT_YUV422:
#ifdef RED_LION
            printk("[GE DRV][%06d] Are you sure to draw in YUV?\n", __LINE__);
#else
            printk("[GE DRV][%06d] Are you sure to draw in YUV?\n", __LINE__);
#endif
        default:
            GE_ASSERT(0, printk("[GE DRV][%06d] Bad color format\n", __LINE__));
            *low  =(color->b & 0xff) | ((color->g & 0xff) << 8);
            *high =(color->r & 0xff) | ((color->a & 0xff) << 8);
            break;
    }

}

void MHal_GE_PowerOff(void)
{
	GE_DEBUGINFO(printk("MHal_GE_PowerOff\n"));		
    GE_WriteReg(GE_REG_FMT_BLT, (GE_VAL_EN_CMDQ | GE_VAL_EN_DYNAMIC_CLOCK));
    GE_WriteReg(GE_REG_EN, GE_VAL_EN_PE);
}

void MHal_GE_Set_Power_Off(U8 flag)
{
	U16 u32Value = 0;
	
	GE_DEBUGINFO(printk("MHal_GE_Set_PowerOff\n"));		
	u32Value = (*((volatile U32 *)(REG_MIPS_BASE + (0x0b00 << 1) + (0x0048 << 2))));
	if ( flag )
	{
		u32Value |= 0x01;
	}
	else
	{
		u32Value &= (~0x01);
	}
	(*((volatile U32 *)(REG_MIPS_BASE + (0x0b00 << 1) + (0x0048 << 2)))) = u32Value;	
}

GESTATUS MHal_GE_EnableVCmdQueue(U16 blEnable)
{
    U32 u32Value;

	GE_DEBUGINFO(printk("MHal_GE_EnableVCmdQueue\n"));		
    GE_WaitAvailableCMDQueue(4);

    u32Value = GE_ReadReg(GE_REG_FMT_BLT);

    if (blEnable)
    {
        u32Value |= GE_VAL_EN_VCMDQ;
    }
    else
    {
        u32Value &= ~GE_VAL_EN_VCMDQ;
    }
    GE_WriteReg(GE_REG_FMT_BLT, u32Value);

    return GESTATUS_SUCCESS;

}

GESTATUS MHal_GE_SetVCmdBuffer(U32 u32Addr, GEVCMD_BUF_SIZE enBufSize)
{

// TODO:    Confirm address

	GE_DEBUGINFO(printk("MHal_GE_SetVCmdBuffer\n"));
    GE_WaitAvailableCMDQueue(5);

    GE_WriteReg(GE_REG_VCMDQ_BASE0, u32Addr & 0xffff);     // Address
    GE_WriteReg(GE_REG_VCMDQ_BASE1, u32Addr >> 16);        // Address

    GE_WriteReg(GE_REG_VCMDQ_SIZE, (enBufSize & 0x07));
    return GESTATUS_SUCCESS;
}

GESTATUS MHal_GE_SetVCmd_W_Thread(U8 u8W_Threshold)
{
    U32 u32Value;

	GE_DEBUGINFO(printk("MHal_GE_SetVCmd_W_Thread\n"));	
    GE_WaitAvailableCMDQueue(4);

    u32Value = GE_ReadReg(GE_REG_STBB_TH);

    u32Value &= ~(GE_MSK_VCMDQ_W_TH);
    u32Value |= u8W_Threshold << 4;

    GE_WriteReg(GE_REG_STBB_TH, u32Value);

    return GESTATUS_SUCCESS;
}

GESTATUS MHal_GE_SetVCmd_R_Thread(U8 u8R_Threshold)
{
    U32 u32Value;

	GE_DEBUGINFO(printk("MHal_GE_SetVCmd_R_Thread\n"));	
    GE_WaitAvailableCMDQueue(4);

    u32Value = GE_ReadReg(GE_REG_STBB_TH);

    u32Value &= ~(GE_MSK_VCMDQ_R_TH);
    u32Value |= u8R_Threshold << 8;

    GE_WriteReg(GE_REG_STBB_TH, u32Value);

    return GESTATUS_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
/// Enable PE destination alpha comparing
/// @param enable \b IN: true/false
/// @return GESTATUS_SUCCESS - Success
/// @return GESTATUS_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_EnableDSTAC(U16 enable)
{
    U32 u32Value;
	
	GE_DEBUGINFO(printk("MHal_GE_EnableDSTAC\n"));	
    GE_WaitAvailableCMDQueue(4);

#if PATCH_RD_CMD
    u32Value = _u32Reg0hValue;
#else
    u32Value = GE_ReadReg(GE_REG_EN);
#endif

    if(enable)
    {
        u32Value |= GE_VAL_EN_DST_ALPHA_COMPARE;
        GE_WriteReg(GE_REG_EN, u32Value);
    }
    else
    {
        u32Value &= ~GE_VAL_EN_DST_ALPHA_COMPARE;
        GE_WriteReg(GE_REG_EN, u32Value);
    }

#if PATCH_RD_CMD
    _u32Reg0hValue = u32Value;
#endif

    return GESTATUS_SUCCESS;
}

GESTATUS MHal_GE_Set_DSTAC_Mode(U16 blOpWithinThreshold)
{
    U32 u32Value;

	GE_DEBUGINFO(printk("MHal_GE_Set_DSTAC_Mode\n"));	
    GE_WaitAvailableCMDQueue(2);

    u32Value = GE_ReadReg(GE_REG_KEY_OP);

    if (blOpWithinThreshold)
    {
        u32Value |= GE_VAL_DST_ALPHA_COMP_MODE;
    }
    else
    {
        u32Value &= ~GE_VAL_DST_ALPHA_COMP_MODE;
    }

    GE_WriteReg(GE_REG_KEY_OP, u32Value);

    return GESTATUS_SUCCESS;
}

GESTATUS MHal_GE_Set_DSTAC_Thread(U8 u8LoThread, U8 u8HiThread)
{
	GE_DEBUGINFO(printk("MHal_GE_Set_DSTAC_Thread\n"));	
    GE_WaitAvailableCMDQueue(2);

    GE_WriteReg(GE_REG_DSTAC_THRESHOLD, (u8LoThread << 8) | u8HiThread);
    return GESTATUS_SUCCESS;
}

#if 0 // LGE Fix 20080814
//-------------------------------------------------------------------------------------------------
// Set a palette entry, I8(8-bit palette) format.
// @param pPalEntry \b IN: ptr to palette data
// @param u32Index   \b IN: Palette Index,
// @return TRUE: sucess / FALSE: fail
// @note: Titania2 GE (support 256 entries: Index: 0-255)
//-------------------------------------------------------------------------------------------------
GESTATUS _MHal_GE_Palette_Set( GePaletteEntry *pPalEntry, U32 u32Index)
{
    U32 u32RegVal;

    // Set palette
    u32RegVal = 0;
        GE_WaitAvailableCMDQueue(8);
        //printk("%d %08x\n", i, pPalEntry[0].u32Data);
    u32RegVal=(U32)pPalEntry;


    GE_WriteReg(PE_REG_PT_GB, (u32RegVal & 0xFFFF));
    GE_WriteReg(PE_REG_PT_AR,((u32RegVal>>16)& 0xFFFF));
    u32RegVal=GE_ReadReg(PE_REG_PT_IDX);
    GE_WriteReg(PE_REG_PT_IDX, ((u32Index&0xFF)|0x100));
    GE_WriteReg(PE_REG_PT_IDX,(u32RegVal &0xFEFF));

    return TRUE;
}




//-------------------------------------------------------------------------------------------------
/// Set GE palette for I8(8-bit palette) format
/// @param pPalArray       \b IN: ptr to palette entry array
/// @param u32PalStart     \b IN: Define palette entry start index for set (range: 0~255)
/// @param u32PalEnd       \b IN: Define palette entry end index for set (range: 0~255)
/// @return TRUE: sucess / FALSE: fail
/// @note 1: GE palette is single port SRAM, You must set palettes before you read palette table.
/// - I8 (valid palette index: 0~255)
/// - Example: MHal_GE_Palette_Set ( pPalArray, 32, 255)
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_Palette_Set(GePaletteEntry * pPalArray, U32 u32PalStart, U32 u32PalEnd)
{
    U32 u32I, u32J;
    GESTATUS bRet = FALSE;


    if((u32PalEnd >= GE_PALETTE_ENTRY_NUM) || (u32PalStart > u32PalEnd))
    {
        return FALSE;
    }

    for (u32I = u32PalStart, u32J=0; u32I<=u32PalEnd; u32I++, u32J++)
    {
        //printk("index %lx pal data=%lx\n",u32J,pPalArray[u32J]);
        bRet = _MHal_GE_Palette_Set(&pPalArray[u32J], u32I);
        if (bRet == FALSE)
            break;
    }

    return bRet;
}
#else // LGE Fix 20080814
//-------------------------------------------------------------------------------------------------
// Set a palette entry, I8(8-bit palette) format.
// @param palEntry \b IN: ptr to palette data
// @param u32Index   \b IN: Palette Index,
// @return TRUE: sucess / FALSE: fail
// @note: Titania2 GE (support 256 entries: Index: 0-255)
//-------------------------------------------------------------------------------------------------
GESTATUS _MHal_GE_Palette_Set( GePaletteEntry palEntry, U32 u32Index)
{
    U32 u32RegVal;

	GE_DEBUGINFO(printk("MHal_GE_Palette_Set\n"));	
    GE_WaitAvailableCMDQueue(8);
    u32RegVal=palEntry.u32Data;

    GE_WriteReg(PE_REG_PT_GB, (u32RegVal & 0xFFFF));
    GE_WriteReg(PE_REG_PT_AR,((u32RegVal>>16)& 0xFFFF));
    u32RegVal=GE_ReadReg(PE_REG_PT_IDX);
    GE_WriteReg(PE_REG_PT_IDX, ((u32Index&0xFF)|0x100));
    GE_WriteReg(PE_REG_PT_IDX,(u32RegVal &0xFEFF));

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/// Set GE palette for I8(8-bit palette) format
/// @param pPalArray       \b IN: ptr to palette entry array
/// @param u32PalStart     \b IN: Define palette entry start index for set (range: 0~255)
/// @param u32PalEnd       \b IN: Define palette entry end index for set (range: 0~255)
/// @return TRUE: sucess / FALSE: fail
/// @note 1: GE palette is single port SRAM, You must set palettes before you read palette table.
/// - I8 (valid palette index: 0~255)
/// - Example: MHal_GE_Palette_Set ( pPalArray, 32, 255)
//-------------------------------------------------------------------------------------------------
GESTATUS MHal_GE_Palette_Set(GePaletteEntry * pPalArray, U32 u32PalStart, U32 u32PalEnd)
{
    U32 u32I;
    GESTATUS bRet = FALSE;


    if((u32PalEnd >= GE_PALETTE_ENTRY_NUM) || (u32PalStart > u32PalEnd))
    {
        return FALSE;
    }

    for (u32I = u32PalStart; u32I<=u32PalEnd; u32I++)
    {
        bRet = _MHal_GE_Palette_Set(pPalArray[u32I], u32I);
        if (bRet == FALSE)
            break;
    }

    return bRet;
}
#endif // LGE Fix 20080814
