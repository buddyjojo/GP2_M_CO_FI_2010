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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    madp_gop.h
/// @author MStar Semiconductor Inc.
/// @brief  GOP Device Driver
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _MDEV_GOP_H_
#define _MDEV_GOP_H_


#ifndef MDRV_GOP_NR_DEVS
#define MDRV_GOP_NR_DEVS 1
#endif

#ifdef __KERNEL__
//#include "MDevTypes.h"
#else
#if 0
typedef unsigned char               U8;

typedef unsigned short              U16;

typedef unsigned long               U32;

typedef unsigned char               U16;

#define	NULL	                    0
#define FALSE                       0
#define TRUE                        1
#endif
#endif

//#include "madp_gop.h"
//#include "mhal_gop.h"

// These type define can be removed after the coding style is confirmed.
#include <asm-mips/types.h>

//------------------------------------------------------------------------------
// macro
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// enum
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// structure
//------------------------------------------------------------------------------

//////////////////////// The above type is moved from madp_gop.h //////////////////////////

typedef struct {
    GOP_HW_Type eGOP_Type;
    U16 blEnable;
} MS_GOP_GWIN_FORCEWRITE, *PMS_GOP_GWIN_FORCEWRITE;

typedef struct {
    GOP_HW_Type eGOP_Type;
    GOPDstType eDstType;
} MS_GOP_SET_DSTPLANE, *PMS_GOP_SET_DSTPLANE;

typedef struct {
    GOP_HW_Type eGOP_Type;
    GopPaletteEntry *pPalArray;
    U32 u32PalStart;
    U32 u32PalEnd;
    GopPalType enPalType;
} MS_GOP_SET_PALETTE, *PMS_GOP_SET_PALETTE;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U16 blEnable;
} MS_GOP_ENABLE_GOP_FUNC, *PMS_GOP_ENABLE_GOP_FUNC;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    GOPColorType eColorType;
    U32 u32SrcX;
    U32 u32SrcY;
    U32 u32DispX;
    U32 u32DispY;
    U32 u32Width;
    U32 u32Height;
    U32 u32DRAMRBlkStart;
    U32 u32DRAMRBlkHSize;
    U32 u32DRAMRBlkVSize;
} MS_GOP_CREATE_GWIN, *PMS_GOP_CREATE_GWIN;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U32 u32Rate;
} MS_GOP_SET_BLINK_RATE, *PMS_GOP_SET_BLINK_RATE;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U32 u32Rate;
} MS_GOP_SET_SCROLL_RATE, *PMS_GOP_SET_SCROLL_RATE;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
} MS_GOP_ALLOC_FREE_GWIN, *PMS_GOP_ALLOC_FREE_GWIN;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    U16 blEnable;
} MS_GOP_ENABLE_GWIN, PMS_GOP_ENABLE_GWIN;

typedef struct {
    GOP_HW_Type eGOP_Type;
    GOPTRSColor TRSColor;
} MS_GOP_SET_TRANSCRL, *PMS_GOP_SET_TRANSCRL;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    GopScrollType eScrollType;
    U8 u8ScrollStep;
    U16 blEnable;
} MS_GOP_ENABLE_SCROLL, *PMS_GOP_ENABLE_SCROLL;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    U16 blEnable;
    U32 u32Alpha;
} MS_GOP_SET_BLENDING, *PMS_GOP_SET_BLENDING;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    U32 u32DispX;
    U32 u32DispY;
} MS_GOP_MOVE_GWIN_DISPPOS, *PMS_GOP_MOVE_GWIN_DISPPOS;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    GopScrollAutoStopType eScrollStopType;
    U16 blEnable;
} MS_GOP_ENABLE_SCROLL_AUTOSTOP, *PMS_GOP_ENABLE_SCROLL_AUTOSTOP;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    U32 u32ScrollAutoStopOffset;
} MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET, *PMS_GOP_SET_SCROLL_AUTOSTOP_OFFSET;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U16 *pbIntStatus;
} MS_GOP_GET_INT_STATUS, *PMS_GOP_GET_INT_STATUS;

typedef struct {
    GOP_HW_Type eGOP_Type;
    GOPDstType eDstType;
    U16 u16XOffset;
    U16 u16YOffset;
} MS_GOP_SET_OFFSET, *PMS_GOP_SET_OFFSET;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    GopGwinInfo *pGWinInfo;
} MS_GWIN_INFO, *PMS_GWIN_INFO;


typedef struct {
    GOP_HW_Type eGOP_Type;
    GopInfo *pGOPInfo;
} MS_GOP_GET_INFO, *PMS_GOP_GET_INFO;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8WinId;
    GopPaletteEntry *pPalArray;
    U32 u32PalNum;
    GopPalType ePalType;
} MS_GOP_SET_BLINK_PALETTE, *PMS_GOP_SET_BLINK_PALETTE;

typedef struct {
    GOP_HW_Type eGOP_Type;
    GopTwinInfo* pTwinInfo;
} MS_GOP_CREATE_TWIN, *PMS_GOP_CREATE_TWIN;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U16 blEnable;
} MS_GOP_ENABLE_TWIN, *PMS_GOP_ENABLE_TWIN;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    U8 u8Rate;
    GOP_FADE_Type eFADE_Type;
} MS_GOP_INIT_FADE, *PMS_GOP_INIT_FADE;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
} MS_GOP_TRIGGER_FADE, *PMS_GOP_TRIGGER_FADE;


typedef struct {
    GOPDWINIntType eIntType;
    U16 blEnable;
} MS_GOP_DWIN_ENABLE_INT, *PMS_GOP_DWIN_ENABLE_INT;

typedef struct {
    U32 u32Addr;
    U32 u32UpBond;
} MS_GOP_DWIN_SET_PINPON, *PMS_GOP_DWIN_SET_PINPON;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U32 u32DispX;
    U32 u32DispY;
    U32 u32Width;
    U32 u32Height;
} MS_GOP_CREATE_STRETCH_WIN, *PMS_GOP_CREATE_STRETCH_WIN;

typedef struct {
    GOP_HW_Type eGOP_Type;
    U32 u32H_Ratio;
    U32 u32V_Ratio;
} MS_GOP_STRETCH_WIN_SET_RATIO, *PMS_GOP_STRETCH_WIN_SET_RATIO;


typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 u8Wid;
    U16 blEnable;
} MS_GOP_ENABLE_GWIN_ALPHA, PMS_GOP_ENABLE_GWIN_ALPHA;

#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
typedef struct
{
    U32 u32GopIdx;  //in
    U32 u32GwinIdx; //in
    U32 u32Addr;    //in
    U32 u32TagId;   //in
    U32 u32QEntry;  //InOut
    U32 u32Result;  //Out
}MS_GOP_FLIP_INFO, *PMS_GOP_FLIP_INFO;
#endif

typedef struct {
    GOP_HW_Type eGOP_Type;
    U8 blEnable;
} MS_GOP_ENABLE_MIRROR, *PMS_GOP_ENABLE_MIRROR;

// KimTH_091026
typedef struct 
{
    GOP_HW_Type u8GOP_Type;
    U16 u16HSPD;
} MS_GOP_ADJUST_HSPD, *PMS_GOP_ADJUST_HSPD;

//------------------------------------------------------------------------------
// external function
//------------------------------------------------------------------------------

/*
int MDrv_GOP_Open(struct inode *inode, struct file *filp);
int MDrv_GOP_Release(struct inode *inode, struct file *filp);
int MDrv_GOP_IOCtl(struct inode *inode, struct file *filp, U16 cmd, unsigned long arg);
*/

#define MDRV_GOP_IOC_MAGIC  '2'

#define MDRV_GOP_IOC_INIT             _IOW(MDRV_GOP_IOC_MAGIC,  0, GOP_HW_Type)
#define MDRV_GOP_IOC_GWIN_FORCEWRITE  _IOW(MDRV_GOP_IOC_MAGIC,  1, MS_GOP_GWIN_FORCEWRITE)
#define MDRV_GOP_IOC_SET_DSTPLANE     _IOW(MDRV_GOP_IOC_MAGIC,  2, MS_GOP_SET_DSTPLANE)
#define MDRV_GOP_IOC_SET_PALETTE      _IOW(MDRV_GOP_IOC_MAGIC,  3, MS_GOP_SET_PALETTE)
#define MDRV_GOP_IOC_ENABLE_OUTPUT_PROGRESSIVE _IOW(MDRV_GOP_IOC_MAGIC,  4, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_ENABLE_OUTPUT_HDUP  _IOW(MDRV_GOP_IOC_MAGIC,  5, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_ENABLE_OUTPUT_VDUP  _IOW(MDRV_GOP_IOC_MAGIC,  6, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_ENABLE_YUV_OUT   _IOW(MDRV_GOP_IOC_MAGIC,  7, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_CREATE_GWIN      _IOW(MDRV_GOP_IOC_MAGIC,  8, MS_GOP_CREATE_GWIN)
#define MDRV_GOP_IOC_SET_BLINK_RATE   _IOW(MDRV_GOP_IOC_MAGIC,  9, MS_GOP_SET_BLINK_RATE)
#define MDRV_GOP_IOC_ENABLE_BLINK     _IOW(MDRV_GOP_IOC_MAGIC,  10, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_SET_SCROLL_RATE  _IOW(MDRV_GOP_IOC_MAGIC,  11, MS_GOP_SET_SCROLL_RATE)
#define MDRV_GOP_IOC_ALLOC_GWIN       _IOW(MDRV_GOP_IOC_MAGIC,  12, MS_GOP_ALLOC_FREE_GWIN)
#define MDRV_GOP_IOC_FREE_GWIN        _IOW(MDRV_GOP_IOC_MAGIC,  13, MS_GOP_ALLOC_FREE_GWIN)
#define MDRV_GOP_IOC_ENABLE_GWIN      _IOW(MDRV_GOP_IOC_MAGIC,  14, MS_GOP_ENABLE_GWIN)
#define MDRV_GOP_IOC_SET_TRANSCLR     _IOW(MDRV_GOP_IOC_MAGIC,  15, MS_GOP_SET_TRANSCRL)
#define MDRV_GOP_IOC_ENABLE_TRANSCLR  _IOW(MDRV_GOP_IOC_MAGIC,  16, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_ENABLE_SCROLL    _IOW(MDRV_GOP_IOC_MAGIC,  17, MS_GOP_ENABLE_SCROLL)
#define MDRV_GOP_IOC_SET_BLENDING     _IOW(MDRV_GOP_IOC_MAGIC,  18, MS_GOP_SET_BLENDING)
#define MDRV_GOP_IOC_MOVE_GWIN_DISPPOS _IOW(MDRV_GOP_IOC_MAGIC,  19, MS_GOP_MOVE_GWIN_DISPPOS)
#define MDRV_GOP_IOC_ENABLE_SCROLL_AUTOSTOP _IOW(MDRV_GOP_IOC_MAGIC,  20, MS_GOP_ENABLE_SCROLL_AUTOSTOP)
#define MDRV_GOP_IOC_SET_SCROLL_AUTOSTOP_H  _IOW(MDRV_GOP_IOC_MAGIC,  21, MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET)
#define MDRV_GOP_IOC_SET_SCROLL_AUTOSTOP_V  _IOW(MDRV_GOP_IOC_MAGIC,  22, MS_GOP_SET_SCROLL_AUTOSTOP_OFFSET)
#define MDRV_GOP_IOC_SET_ENABLE_INT   _IOW(MDRV_GOP_IOC_MAGIC,  23, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_GET_INT_STATUS   _IOR(MDRV_GOP_IOC_MAGIC,  24, MS_GOP_GET_INT_STATUS)
#define MDRV_GOP_IOC_SET_OFFSET       _IOW(MDRV_GOP_IOC_MAGIC,  25, MS_GOP_SET_OFFSET)
#define MDRV_GOP_IOC_GWIN_GET_INFO        _IOR(MDRV_GOP_IOC_MAGIC,  26, MS_GWIN_INFO)
#define MDRV_GOP_IOC_GET_INFO         _IOR(MDRV_GOP_IOC_MAGIC,  27, MS_GOP_GET_INFO)
#define MDRV_GOP_IOC_SET_BLINK_PALETTE _IOW(MDRV_GOP_IOC_MAGIC,  28, MS_GOP_SET_BLINK_PALETTE)

//-------------------------------------------------------------------------------------------------
// GOP TWIN functions
//-------------------------------------------------------------------------------------------------
#define MDRV_GOP_IOC_CREATE_TWIN      _IOW(MDRV_GOP_IOC_MAGIC,  29, MS_GOP_CREATE_TWIN)
#define MDRV_GOP_IOC_Enable_TWIN      _IOW(MDRV_GOP_IOC_MAGIC,  30, MS_GOP_ENABLE_TWIN)
//-------------------------------------------------------------------------------------------------
// GOP FADE functions
//-------------------------------------------------------------------------------------------------
#define MDRV_GOP_IOC_INIT_FADE        _IOW(MDRV_GOP_IOC_MAGIC,  31, MS_GOP_INIT_FADE)
#define MDRV_GOP_IOC_TRIGGER_FADE     _IOW(MDRV_GOP_IOC_MAGIC,  32, MS_GOP_TRIGGER_FADE)
//-------------------------------------------------------------------------------------------------
// GOP DWIN functions
//-------------------------------------------------------------------------------------------------
#define MDRV_GOP_IOC_INIT_DWIN        _IO(MDRV_GOP_IOC_MAGIC,  33)
#define MDRV_GOP_IOC_ALLOC_DWIN       _IO(MDRV_GOP_IOC_MAGIC,  34)
#define MDRV_GOP_IOC_FREE_DWIN        _IO(MDRV_GOP_IOC_MAGIC,  35)
#define MDRV_GOP_IOC_ENABLE_CAPTURE_STREAM _IOW(MDRV_GOP_IOC_MAGIC,  36, U16)
#define MDRV_GOP_IOC_SET_CAPTURE_MODE _IOW(MDRV_GOP_IOC_MAGIC,  37, GopCaptureMode)
#define MDRV_GOP_IOC_SET_INPUTSOURCE_MODE _IOW(MDRV_GOP_IOC_MAGIC,  62, EN_GOP_DWIN_DATA_SRC)//Arki ><
#define MDRV_GOP_IOC_DWIN_SET_SCAN_TYPE _IOW(MDRV_GOP_IOC_MAGIC,  38, U16)
#define MDRV_GOP_IOC_DWIN_SET_WININFO _IOW(MDRV_GOP_IOC_MAGIC,  39, GOP_DWIN_INFO)
#define MDRV_GOP_IOC_DWIN_ENABLE_INT  _IOW(MDRV_GOP_IOC_MAGIC,  40, MS_GOP_DWIN_ENABLE_INT)
#define MDRV_GOP_IOC_DWIN_GET_INTSTATUS _IOR(MDRV_GOP_IOC_MAGIC,  41, U16)
#define MDRV_GOP_IOC_DWIN_SET_PINPON _IOW(MDRV_GOP_IOC_MAGIC,  42, MS_GOP_DWIN_SET_PINPON)

#define MDRV_GOP_IOC_UPDATE_REG      _IOW(MDRV_GOP_IOC_MAGIC,  43, GOP_HW_Type)
#define MDRV_GOP_IOC_SET_CLOCK       _IOW(MDRV_GOP_IOC_MAGIC,  44, MS_GOP_SET_DSTPLANE)
// alex add
#define MDRV_GOP_IOC_CREATE_STRETCH_WIN      _IOW(MDRV_GOP_IOC_MAGIC,  45, MS_GOP_CREATE_STRETCH_WIN)
#define MDRV_GOP_IOC_STRETCH_WIN_SET_RATIO _IOW(MDRV_GOP_IOC_MAGIC,  46, MS_GOP_STRETCH_WIN_SET_RATIO)
#define MDRV_GOP_IOC_ENABLE_GWIN_ALPHA      _IOW(MDRV_GOP_IOC_MAGIC,  47, MS_GOP_ENABLE_GWIN_ALPHA)
#define MDRV_GOP_IOC_SCALER_SET_GOPSEL      _IOW(MDRV_GOP_IOC_MAGIC,  48, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_SET_FIELD_INVERSE      _IOW(MDRV_GOP_IOC_MAGIC,  49, MS_GOP_ENABLE_GOP_FUNC)
#define MDRV_GOP_IOC_SET_PALETTE_CONTROL      _IOW(MDRV_GOP_IOC_MAGIC,  50, MS_GOP_ENABLE_GOP_FUNC)

//Robert Yang add
#define MDRV_GOP_IOC_GWIN_SET_INFO        _IOW(MDRV_GOP_IOC_MAGIC,  51, MS_GWIN_INFO)

#define MDRV_GOP_IOC_GWIN_INFO      _IO(MDRV_GOP_IOC_MAGIC,  52)
#define MDRV_GOP_IOC_REGISTER_INT       _IOW(MDRV_GOP_IOC_MAGIC,  53, U32)
#define MDRV_GOP_IOC_DEREGISTER_INT     _IOW(MDRV_GOP_IOC_MAGIC,  54, U32)

// 20080805 LGE Added for TRUE Motion Demo.
#define MDRV_GOP_IOC_TRUE_MOTION_DEMO      _IOW(MDRV_GOP_IOC_MAGIC,  55, U32)

#define MDRV_GOP_IOC_VCOM_PATTERN		_IOW(MDRV_GOP_IOC_MAGIC,  56, U32)

#define MDRV_GOP_IOC_TEST_PATTERN     _IOW(MDRV_GOP_IOC_MAGIC,  57, SetPattern)

// Jason Su add
#define MDRV_GOP_IOC_INFOBACKUP       _IOW(MDRV_GOP_IOC_MAGIC,  58, U32)

#define MDRV_GOP_IOC_INFORESTORE     _IOW(MDRV_GOP_IOC_MAGIC,  59, U32)

#define MDRV_GOP_IOC_READ_PALETTE      _IOW(MDRV_GOP_IOC_MAGIC,  60, MS_GOP_SET_PALETTE)

#define MDRV_GOP_IOC_ENABLE_VMIRROR   _IOW(MDRV_GOP_IOC_MAGIC,  61, MS_GOP_ENABLE_MIRROR)
#define MDRV_GOP_IOC_ENABLE_HMIRROR   _IOW(MDRV_GOP_IOC_MAGIC,  62, MS_GOP_ENABLE_MIRROR)

#define MDRV_GOP_IOC_SET_CLKGEN_MODE _IOW(MDRV_GOP_IOC_MAGIC,  63, EN_GOP_DWIN_DATA_SRC)//Arki ><
#define MDRV_GOP_IOC_SET_PWR_ON         _IO(MDRV_GOP_IOC_MAGIC,  64)//Arki ><
#define MDRV_GOP_IOC_SET_PWR_OFF        _IO(MDRV_GOP_IOC_MAGIC,  65)//Arki ><

#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
#define MDRV_GOP_IOC_SET_FLIP_INFO     _IOW(MDRV_GOP_IOC_MAGIC,  66, MS_GOP_FLIP_INFO)
#define MDRV_GOP_IOC_ADJUST_HSPD    _IOW(MDRV_GOP_IOC_MAGIC,  67, MS_GOP_ADJUST_HSPD) // KimTH_091026

#define MDRV_GOP_IOC_MAXNR  68//67     // The number should be as same as the number of IOCTRL command

#else
#define MDRV_GOP_IOC_ADJUST_HSPD    _IOW(MDRV_GOP_IOC_MAGIC,  66, MS_GOP_ADJUST_HSPD) // KimTH_091026
#define MDRV_GOP_IOC_MAXNR  67//66     // The number should be as same as the number of IOCTRL command
#endif

void MDrv_GOP_VSyncHandle(void);

#endif  // #ifndef MDrv_GOP_H_
