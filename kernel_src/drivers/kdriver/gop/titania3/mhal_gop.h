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
/// @file drvGOP.h
/// @brief Graphics Output Path (GOP) Driver Interface
/// @author MStar Semiconductor Inc.
///
/// GOP is used to window the RAW data into DRAM for GE, and readout the GE result in DRAM . GOP include two channel to MIU (1R1W). GOP supports to read a random frame window and read 4 random windows (GWIN) .
///
// \n\n\n\n\n
// @image html GOP_COP_DC_TVE.JPG "GOP, COP, DC and GE diagram"
///
/// Features:
/// - Support color formats:
///     - RGB based formats: RGB555, RGB565, ARGB4444, 8-bit Palette, ARGB8888
///     - YUV based formats: VU7Y8, VU8Y8
///     - Blink format: Blink 12355 format
/// - Transparent color support (color key)
/// - Transparent Window support
/// - Alpha support:
///     - Constant alpha
///     - Per-pixel alpha for ARGB8888 & ARGB4444.
/// - Duplicate support: horizontal and/or vertical
/// - Scroll modes: horizontal and/or vertical.
/// - Output modes: Interlace or progressive
/// \n\n
///
/// Lmitations:
/// - Gwins buffer <  32MB.
/// - Gwins in different color depths can't be overlapped.
/// @image html GOP_Limit0.JPG "Limitation of GOP"
///
/// - Cannot display GWIN beyond display plane.
/// @image html GOP_Limit1.JPG "Limitation of GOP"
///
/// GOP example codes:
/// - //Inital GOP0 Features
/// - MHal_GOP_Init(E_GOP4G_0);
/// - //Allocate GOP Gwin Resource
/// -    MHal_GOP_GWIN_Alloc(E_GOP4G_0, 0); // allocate gop0, gwin0 resource
/// - MHal_GOP_DstPlane_Set( E_GOP4G_0, E_GOP_DST_OP);  // set GOP destination display plane
/// - MHal_GOP_Output_Progressive_Enable (E_GOP4G_0, TRUE); //set progressive mode
/// - MHal_GOP_Output_HDup_Enable (E_GOP4G_0, FALSE); //disable horizontal duplicate
/// - MHal_GOP_Output_VDup_Enable (E_GOP4G_0, FALSE); //disable vertical duplicate
/// - MHal_GOP_Palette_Set ( E_GOP4G_0, pPalArray, 0, 31, E_GOP_PAL_RGB888); //set RGB888 palette (0~31 entries) from palette array (pPalArray)
/// - MHal_GOP_GWIN_Create( E_GOP4G_0,
///                                 - 0,                                      //create Gwin0
///                                 - E_GOP_COLOR_BLINK,            //eColorType = BLINK
///                                 - 0,                                      // u32SrcX = 0
///                                 - 0,                                      // u32SrcY = 0
///                                 - 48,                                     //u32DispX = 48
///                                 - 50,                                     //u32DispY = 50
///                                 - 200,                                    //u32Width = 200 pixel,
///                                 - 200,                                     //u32Height = 200 pixel,
///                                 - u32DRAMRBlkStart,               //u32DRAMRBlkStart (must define by user)
///                                 - 400,                                     //u32DRAMRBlkHSize = 400 pixel
///                                 - 400,                                    //u32DRAMRBlkVSize = 400 pixel
///                                 -);
/// - MHal_GOP_GWIN_Enable(E_GOP4G_0, 0, TRUE);         //Enable Gwin0 (show in display plane)
///     - //Then User can do some Gwin function such as
///         - Scroll Funs:  MHal_GOP_GWIN_Scroll_Enable, MHal_GOP_Scroll_SetRate, ...
///         - Alpha Blending Funs: MHal_GOP_GWIN_Blending_Set
///         - Transparent Color Funs: MHal_GOP_TransClr_Set, MHal_GOP_TransClr_Enable
///         - BLINK Funs: MHal_GOP_Blink_SetRate, MHal_GOP_Blink_Enable
///         - Move Gwin: MHal_GOP_GWIN_DispPos_Move
/// - MHal_GOP_GWIN_Free(E_GOP4G_0, 0);                        //Free Gwin0

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DRVGOP_H__
#define __DRVGOP_H__

#ifdef __KERNEL__
//#include "MDevTypes.h"
#include "mdrv_types.h"
#else
// This include can be reomved later
#include "MsCommon.h"
#endif

//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
#define EN_DBG_GOP              1
//#define MAX_GWIN_SUPPORT        4       ///< Specify the number of GWIN per GOP
#define MAX_GOP0_GWIN_SUPPORT        4       ///< Specify the number of GWIN per GOP
#define MAX_GOP1_GWIN_SUPPORT        2       ///< Specify the number of GWIN per GOP
#define MAX_GOP2_GWIN_SUPPORT        1       ///< Specify the number of GWIN per GOP
#define MAX_GOP3_GWIN_SUPPORT        1       ///< Specify the number of GWIN per GOP
#define MAX_GOP_SUPPORT         4       ///< Specify the number of GOP (2(GOP4G) + 1(GOP1G))
#define TOTAL_GWIN_SUPPORT      (MAX_GOP0_GWIN_SUPPORT + MAX_GOP1_GWIN_SUPPORT + MAX_GOP2_GWIN_SUPPORT + MAX_GOP3_GWIN_SUPPORT) //9       ///< Specify the total GWIN (2x4(GOP4G)+1(GOP1G))
#define GOP_PALETTE_ENTRY_NUM   256
#define BLINK_PALETTE_ENTRY_NUM 32

#define GOP_IP_PD          10    ///Chip characteristic
#define GOP_DST_TYGE_VALIDBITS   2

#define GOP_VSYNC_INTERRUPT_FLIP_ENABLE 0
//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
//dhjung LGE BIT_MASK->_BIT_MASK
#define _BIT_MASK(bits)                  ((1<<bits)-1)

//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------

/// Define the GOP MUX.
/// Priority TCON_OSD_2 > TCON_OSD_1 > TCON_OSD_0
typedef enum
{
    /// TCOM OSD 0, E_GOP4G_0 default Path
    E_TCON_OSD_0 = 0,
    /// TCOM OSD 1, E_GOP4G_1 default Path,
    E_TCON_OSD_1 = 1,
    /// TCOM OSD 2 , GOP1G default Path
    E_TCON_OSD_2 = 2,
    /// TCOM OSD 3 , GOP1GX default Path
    E_TCON_OSD_3 = 3,
}GOP_MUX;

/// Define GOP HW Type
typedef enum
{
    /// GOP 0 (4 Gwin Support)
    E_GOP4G_0 = 0,
    /// GOP 1 (2 Gwin Support)
    E_GOP2G_1 = 1,
    /// GOP SIGNAL (1 Gwin Support)
    E_GOP1G = 2,
    /// GOP SIGNAL (1 Gwin Support which can be used for cursor)
    E_GOP1GX = 3,
}GOP_HW_Type;

// GOP DWIN (1 Dwin Support)
#define E_GOP_DWIN 4


/// Define the entry of palette.
typedef union
{
#if 0
    /// RGB888
    struct
    {
        /// B8
        U8 u8B;
        /// G8
        U8 u8G;
        /// R8
        U8 u8R;
        /// Reserved
        U8 reserved;
    } RGB;
    /// ARGB6666
    struct
    {
        /// B6
        U32 u6B:        6;
        /// G6
        U32 u6G:        6;
        /// R6
        U32 u6R:        6;
        /// A6
        U32 u6A:        6;
        /// Reserved
        U32 reserved:   8;
    } ARGB;
#endif
    /// ARGB8888
    struct
    {
        /// B8
        U32 u8B:        8;
        /// G8
        U32 u8G:        8;
        /// R8
        U32 u8R:        8;
        /// A8
        U32 u8A:        8;
    } ARGB;
    // 32-bit direct access.
    U32 u32Data;
} GopPaletteEntry;

/// Define palette color format.
typedef enum
{
#if 0
    /// Palette color format is RGB888.
    E_GOP_PAL_RGB888    = 0,
    /// Palette color format is ARGB6666 (Only I8 format support).
    E_GOP_PAL_ARGB6666  = 1,
    /// Invalid palette color format.
    E_GOP_PAL_INVALID
#endif
    /// Palette color is for I8.
    E_GOP_PAL_I8            = 0,
    /// Palette color is for Blink12355.
    E_GOP_PAL_BLINK12355    = 1,
    /// Palette color is for Blink2266.
    E_GOP_PAL_BLINK2266     = 2,
    /// Invalid palette color format.
    E_GOP_PAL_INVALID

} GopPalType;

/// Define palette control mode.
typedef enum
{
    /// Palette mode is GOP 4G access (RIU).
    E_GOP_PAL_4G_RIU    = 0,
    /// Palette mode is GOP 4G access (REGDMA mode).
    E_GOP_PAL_4G_REGDMA = 1,
    /// Palette mode is GOP 2G access (RIU).
    E_GOP_PAL_2G_RIU    = 2,
    /// Palette mode is invalid.
    E_GOP_PAL_INVALID_MODE = 3
} GopPalCtrlMode;

/// Define GWIN color format.
typedef enum
{
    /// Color format RGB555 and Blink.
    E_GOP_COLOR_RGB555_BLINK    =0,
    /// Color format RGB565.
    E_GOP_COLOR_RGB565          =1,
    /// Color format ARGB4444.
    E_GOP_COLOR_ARGB4444        =2,
    /// Color format 555.
    E_GOP_COLOR_RGB555          =3,
    /// Color format I8 (256-entry palette).
    E_GOP_COLOR_I8              =4,
    /// Color format ARGB8888.
    E_GOP_COLOR_ARGB8888        =5,
    /// Color format ARGB1555.
    E_GOP_COLOR_ARGB1555        =6,
    /// Color format I2.
    E_GOP_COLOR_I2              =7,
    /// Color format YUV422_VU7Y8, Packet format: V7Y8,U7Y8,......
    E_GOP_COLOR_YUV422_VU7Y8    =8,
    /// Color format YUV422_VU8Y8. Packet format: V8Y8,U8Y8,......
    E_GOP_COLOR_YUV422_VU8Y8    =9,
    /// Color format I1
    E_GOP_COLOR_I1              =10,
    /// Invalid color format.
    E_GOP_COLOR_INVALID
} GOPColorType;

/// Define GOP destination displayplane type
// In GOP registers, only can select dst is OP or IP,
typedef enum
{
    /// (GOP combine with IP (before scalar), sub-picture blending should use this path)
    E_GOP_DST_IP        = 0,
    E_GOP_DST_IP_SUB    = 1,
    /// (GOP combine with OP (after scalar), Normal OSD should use this path)
    E_GOP_DST_OP        = 2,
    E_GOP_DST_MVOP      = 3,
}GOPDstType;

typedef enum
{
    E_Set_HPos,
    E_Set_VPos,
}HMirror_Opt;

///FADE Type
typedef enum
{
    /// FADE WEAKER TYPE
    E_WEAKER   = 0,
    /// FADE STRONGER TYPE
    E_STRONGER = 1,
}GOP_FADE_Type;

/// Declare the Gwin information.
typedef struct
{
    /// Usage of GWIN
    U16 bUsed;
    /// Display horizontal start position in pixel
    U32 u32DispHStart;
    /// Display vertical start position in pixel
    U32 u32DispVStart;
    /// Display width in pixel
    U32 u32DispWidth;
    /// Display height in pixel
    U32 u32DispHeight;
    /// DRAM block start address in byte
    U32 u32DRAMRBlkStart;
    /// DRAM block horizontal size in pixel
    U32 u32DRAMRBlkHSize;
    /// DRAM block vertical size in pixel
    U32 u32DRAMRBlkVSize;
    /// Source horizontal start position in pixel
    U32 u32SrcHStart;
    /// Source vertical start position in pixel
    U32 u32SrcVStart;
    /// color depth in bytes
    U32 u32ColorDepth;
    /// Color Format
    GOPColorType eColorType;
    /// Specify whether Gwin is enabled.
    U32 u32Enable;
} GopGwinInfo, PGopGwinInfo;

typedef struct
{
    U32 u32NonMirrorFBAdr;
    U16 u16NonMirrorHStr;
    U16 u16NonMirrorVStr;
    U16 u16NonMirrorGOP0WinX;
    U16 u16NonMirrorGOP0WinY;
} GOP_GwinMirror_Info;

typedef struct
{
   U16 u16GOP_HasRBLKVMirror_Adr;       //save which GOP0's GWIN DRMA V-Start is set for V-Mirror
   U16 u16GOP_NoRBLKVMirror_Adr;       //save which GOP1's GWIN RBLK adr is set for V-Mirror      //save which GOP1's GWIN RBLK adr is set for V-Mirror
   U16 u16GOP_NoRBLKHMirror_Adr;
   U16 u16GOP_HMirror_HPos;
   U16 u16GOP_VMirror_VPos;
   U8 bVMirror;
   U8 bHMirror;   
   GOP_GwinMirror_Info sMirrorInfo[TOTAL_GWIN_SUPPORT];//GPU_NUM_OF_OSDS];   
} GOP_MirrorInfo;


/// Declare the Twin information.
typedef struct
{
    /// Display horizontal start position in pixel
    U32 u32DispHStart;
    /// Display vertical start position in pixel
    U32 u32DispVStart;
    /// Display width in pixel
    U32 u32DispWidth;
    /// Display height in pixel
    U32 u32DispHeight;
} GopTwinInfo, PGopTwinInfo;

/// Define GOP Info.
typedef struct
{
    /// Destination display plane
    GOPDstType eDstType;
    /// Ture: Progressive Output/ False: Interlace Out
    U16 bProgressiveOut;
    /// OSD Display Mux Info;
    GOP_MUX eGopMux;
    /// Force Write Status;
    U16 bForceWrite;
    /// X Base Position
    U8 u8X_Base;
    /// Y Base Position
    U8 u8Y_Base;
    /// Total number of GWin
    U8 u8NumOfGWin;

} GopInfo, PGopInfo;

/// Define scroll direction.
typedef enum
{
    /// Does not scroll.
    GOP_SCROLL_NONE = 0,
    /// Scroll direction is up.
    GOP_SCROLL_UP,
    /// Scroll direction is down.
    GOP_SCROLL_DOWN,
    /// Scroll direction is left.
    GOP_SCROLL_LEFT,
    /// Scroll direction is right.
    GOP_SCROLL_RIGHT,
} GopScrollType;


///Define Uranus GOP Transprent Color (RGB)
typedef union
{
    ///RGB Based
    struct
    {
        /// B8
        U8 u8B;
        /// G8
        U8 u8G;
        /// R8
        U8 u8R;
        /// Reserved
        U8 u8Reserved;
    }RGB;
    U32 u32Data;
}GOPTRSColor;


/// Define scroll auto stop type.
typedef enum
{
    /// Does not scroll auto stop.
    GOP_SCROLL_AUTOSTOP_NONE    = 0,
    /// Scroll stop automatically on specified V offset.
    GOP_SCROLL_AUTOSTOP_VR      = 1,
    /// Scroll stop automatically on specified H offset.
    GOP_SCROLL_AUTOSTOP_HR      = 2,
#if 0   // NOTE: Titania doesn't allow to scroll in H & V at the same time
    /// Scroll stop automatically on specified V offset or H offset.
    GOP_SCROLL_AUTOSTOP_ALL      = 3
#endif
} GopScrollAutoStopType;

/// Define Dwin Caputre Mode.
typedef enum
{
    E_Cap_EachFrame  = 0,
    E_Cap_OneFrame   = 1,

} GopCaptureMode;

//Arki>>
/// Define Dwin Inputsource Mode.
typedef enum
{
    GOPDWIN_DATA_SRC_SCALAR = 0,        //!< scaler output
    GOPDWIN_DATA_SRC_EXTRGB = 1,        //!< External, RGB, YUV(pass R2Y)
    GOPDWIN_DATA_SRC_VOP    = 3,        //!< DVB VOP (MVOP)
    GOPDWIN_DATA_SRC_MAX    = 4

} EN_GOP_DWIN_DATA_SRC;
//Arki<<

/// Define GOP DWIN color format. (onvert DATA to DRAM)
typedef enum
{
    /// DWIN Color format UV7Y8.
    E_GOP_DWIN_UV7Y8       = 0,
    /// DWIN Color format UV8Y8.
    E_GOP_DWIN_UV8Y8       = 1,
    /// Invalid color format.
    E_GOP_DWIN_INVALID
} GOPDWINColorType;

/// Define GOP DWIN Interrupt Type;
typedef enum
{
    /// DWIN Progressive ACK Interrupt.
    E_GOP_PROG_INT     = 0x10,
    /// DWIN Top Field ACK Interrupt.
    E_GOP_TF_INT       = 0x20,
    /// DWIN Bottom Field ACK Interrupt.
    E_GOP_BF_INT       = 0x40,
    /// DWIN VSYNC interrupt.
    E_GOP_VSYNC_INT       = 0x80,
} GOPDWINIntType;


/// Dump Window Information
typedef struct
{
	U16 u16VPixelStart;		  //!< unit: pix
	U16 u16VPixelEnd;		  //!< unit: pix
	U16 u16HPixelStart;		  //!< unit: pix
	U16 u16HPixelEnd;		  //!< unit: pix
	U32 u32DWINBufAddr;		  //!< unit: Byte
	U32 u32DWINHBondAddr;     //!< unit: Byte
	U16 u16DRAMJumpLen;		  //!< unit: Byte
	GOPDWINColorType eDwinFmt; //!< DWIN format
} GOP_DWIN_INFO, *PGOP_DWIN_INFO;

typedef enum
{
    E_GOP_BANK_4G_0,
    E_GOP_BANK_4G_1,
    E_GOP_BANK_4G_ST,
    E_GOP_BANK_2G_0,
    E_GOP_BANK_2G_1,
    E_GOP_BANK_2G_ST,
    E_GOP_BANK_D,
    E_GOP_BANK_1G,
} GOP_BANK;


typedef enum
{
    MS_IP0_SEL_GOP0,
    MS_IP0_SEL_GOP1,
    MS_IP1_SEL_GOP0,
    MS_IP1_SEL_GOP1,
    MS_NIP_SEL_GOP0,
    MS_NIP_SEL_GOP1
}MS_IPSEL_GOP;
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
typedef struct
{
    U32 u32GwinIdx;//in
    U32 u32Addr; //in
    U32 u32TagId; //in
    U32 u32AvailbQEntry; //out
}GOP_FLIP_INFO, *PGOP_FLIP_INFO;

typedef B16 (*FPTR_CHECKTAGID)(U16);
#endif
///Test Pattern Type
typedef struct
{
    /// Test pattern index to be shown
    U8 u8Pattern;
    /// Test pattern on/off
    U8 u8Param;
    /// Stretch window HS PD
    U16 u16HsPD;

    //20091020 daniel.huang: fix gop test pattern cannot cover all video problem
    U16 u16HStart;
    U16 u16VStart;
    U16 u16HSize;
    U16 u16VSize;
} SetPattern;

//-------------------------------------------------------------------------------------------------
// Extern Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// GOP GWIN functions
//-------------------------------------------------------------------------------------------------
extern B16 MHal_GOP_Init(GOP_HW_Type eGOP_Type);
extern B16 MHal_GOP_GWIN_ForceWrite(GOP_HW_Type eGOP_Type, U16 bEnable);
extern B16 MHal_GOP_DstPlane_Set(GOP_HW_Type eGOP_Type, GOPDstType eDstType);
extern B16 MHal_GOP_Palette_Set ( GOP_HW_Type eGOP_Type, GopPaletteEntry *pPalArray, U32 u32PalStart, U32 u32PalEnd);
extern B16 MHal_GOP_Palette_Read ( GOP_HW_Type eGOP_Type, GopPaletteEntry *pPalArray, U32 u32PalStart, U32 u32PalEnd);
extern B16 MHal_GOP_YUVOut_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
extern B16 MHal_GOP_Output_Progressive_Enable (GOP_HW_Type eGOP_Type, U16 bProgress);
extern B16 MHal_GOP_StretchWin_Create(GOP_HW_Type eGOP_Type,
                                             U32 u32DispX, U32 u32DispY,
                                             U32 u32Width, U32 u32Height);
extern B16 MHal_GOP_StretchWin_SetRatio(GOP_HW_Type eGOP_Type, U32 u32H_Ratio, U32 u32V_Ratio);
extern B16 MHal_GOP_GWIN_Create(GOP_HW_Type eGOP_Type,
                                 U8 u8Wid,
                                 GOPColorType eColorType,
                                 U32 u32SrcX,
                                 U32 u32SrcY,
                                 U32 u32DispX,
                                 U32 u32DispY,
                                 U32 u32Width,
                                 U32 u32Height,
                                 U32 u32DRAMRBlkStart,
                                 U32 u32DRAMRBlkHSize,
                                 U32 u32DRAMRBlkVSize);
extern B16 MHal_GOP_Blink_SetRate(GOP_HW_Type eGOP_Type, U32 u32Rate);
extern B16 MHal_GOP_Blink_Enable(GOP_HW_Type eGOP_Type,  U16 bEnable);
extern B16 MHal_GOP_Scroll_SetRate(GOP_HW_Type eGOP_Type,U32 u32Rate);

extern B16 MHal_GOP_GWIN_Alloc(GOP_HW_Type eGOP_Type, U8 u8Wid);
extern B16 MHal_GOP_GWIN_Free(GOP_HW_Type eGOP_Type, U8 u8Wid);

extern B16 MHal_GOP_GWIN_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable);
extern B16 MHal_GOP_TransClr_Set(GOP_HW_Type eGOP_Type, GOPTRSColor TRSColor);
extern B16 MHal_GOP_TransClr_Enable( GOP_HW_Type eGOP_Type, U16 bEnable);
extern B16 MHal_GOP_GWIN_Scroll_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, GopScrollType eScrollType, U8 u16ScrollStep, U16 bEnable);
extern B16 MHal_GOP_GWIN_Blending_Set(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable, U32 u32Alpha);
extern B16 MHal_GOP_GWIN_DispPos_Move(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32DispX, U32 u32DispY);
extern B16 MHal_GOP_GWIN_Scroll_AutoStop_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, GopScrollAutoStopType eScrollStopType, U16 bEnable);
extern B16 MHal_GOP_GWIN_Scroll_AutoStop_HSet(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32ScrollAutoHStop);
extern B16 MHal_GOP_GWIN_Scroll_AutoStop_VSet(GOP_HW_Type eGOP_Type, U8 u8Wid, U32 u32ScrollAutoVStop);
extern B16 MHal_GOP_Int_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
extern B16 MHal_GOP_Int_GetStatus(GOP_HW_Type eGOP_Type, U16* pbIntStatus);
extern B16 MHal_GOP_Set_Offset (GOP_HW_Type eGOP_Type, GOPDstType eDstType, U16 u16XOffset, U16 u16YOffset);
extern B16 MHal_GOP_GWIN_Info_Set(GOP_HW_Type eGOP_Type, U8 u8Wid, GopGwinInfo *pGInfo);
extern B16 MHal_GOP_GWIN_Info_Get(GOP_HW_Type eGOP_Type, U8 u8Wid, GopGwinInfo *pGInfo);
extern B16 MHal_GOP_Info_Set(GOP_HW_Type eGOP_Type, GopInfo *pGInfo);
extern B16 MHal_GOP_Info_Get(GOP_HW_Type eGOP_Type, GopInfo *pGInfo);
extern B16 MHal_GOP_GWIN_Blink_Palette_Set ( GOP_HW_Type eGOP_Type, U8 u8WinId, GopPaletteEntry *pPalArray, U32 u32PalNum, GopPalType ePalType);
//extern U16 MHal_GOP_GWIN_Blink_Palette_Set ( GOP_HW_Type eGOP_Type, U8 u8WinId, GopPaletteEntry *pPalArray, U32 u32PalNum);
extern B16 MHal_GOP_GWIN_Alpha_Enable(GOP_HW_Type eGOP_Type, U8 u8Wid, U16 bEnable);
extern B16 MHal_GOP_Scaler_Set_GOPSel(MS_IPSEL_GOP ipSelGop);
extern B16 MHal_GOP_InfoBackup(GOP_HW_Type eGOP_Type);
extern B16 MHal_GOP_InfoRestore(GOP_HW_Type eGOP_Type);

extern B16 MHal_GOP_VMirror_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
extern B16 MHal_GOP_HMirror_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);
extern B16 MHal_GOP_TrueMotionDemo(B16 bTure);

//-------------------------------------------------------------------------------------------------
// GOP TWIN functions
//-------------------------------------------------------------------------------------------------
extern B16 MHal_GOP_TWIN_Create(GOP_HW_Type eGOP_Type, GopTwinInfo* pTwinInfo);
extern B16 MHal_GOP_TWIN_Enable(GOP_HW_Type eGOP_Type, U16 bEnable);

//-------------------------------------------------------------------------------------------------
// GOP FADE functions
//-------------------------------------------------------------------------------------------------
extern B16 MHal_GOP_GWIN_FADE_Init(GOP_HW_Type eGOP_Type, U8 u8Wid, U8 u8Rate, GOP_FADE_Type eFADE_Type);
extern B16 MHal_GOP_GWIN_FADE_TRIGGER(GOP_HW_Type eGOP_Type, U8 u8Wid);


//-------------------------------------------------------------------------------------------------
// GOP DWIN functions
//-------------------------------------------------------------------------------------------------
extern B16 MHal_GOP_DWIN_Init(void);
extern B16 MHal_GOP_DWIN_Alloc(void );
extern B16 MHal_GOP_DWIN_Free(void);
extern B16 MHal_GOP_DWIN_CaptureStream_Enable(U16 bEnable);
extern B16 MHal_GOP_DWIN_CaptureMode_Set(GopCaptureMode eCaptureMode);
extern B16 MHal_GOP_DWIN_InputSrcMode_Set(EN_GOP_DWIN_DATA_SRC eDwin_Data_Src_Mode);    //Arki ><
extern B16 MHal_GOP_DWIN_CLKGen_Set(EN_GOP_DWIN_DATA_SRC eDwin_Data_Src_Mode);
extern B16 MHal_GOP_DWIN_SetScanType(U16 bProgressive);
extern B16 MHal_GOP_DWIN_WinInfo_Set(PGOP_DWIN_INFO pinfo);
extern B16 MHal_GOP_DWIN_Int_Enable(GOPDWINIntType eIntType, U16 bEnable);
extern B16 MHal_GOP_DWIN_GetIntrStatus(U16 *pu16IntStatus);
extern B16 MHal_GOP_DWIN_Set_PINPON(U32 addr, U32 upbond);

extern B16 MHal_GOP_Disable_HSYNC_MASK(GOP_HW_Type eGOP_Type);

extern void _MHal_GOP_UpdateReg(GOP_HW_Type eGOP_Type);
extern B16 _MHal_GOP_Clk_Set (GOP_HW_Type eGOP_Type, GOPDstType eDstType);
extern B16 _MHal_GOP_Clk_Enable(GOP_HW_Type eGOP_Type, U16 bEnable );
extern B16 MHal_GOP_SetFieldInver(GOP_HW_Type eGOP_Type, B16 bFieldInverse);
extern B16 MHal_GOP_SetPaletteControl(GOP_HW_Type eGOP_Type, GopPalCtrlMode ePaletteCtrl);

extern B16 MHal_GOP_ClearIRQ( void ) ;
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
extern B16 MHal_GOP_ProcessIRQ( void ) ;
#endif
//20091020 daniel.huang: fix gop test pattern cannot cover all video problem
extern void MHal_GOP_Test_Pattern(U8 u8Pattern_Type, U8 u8Pattern_Param, U16 u16HSPD,
                               U16 u16HStart, U16 u16VStart, U16 u16HSize, U16 u16VSize);
extern void MHal_GOP_AdjustHSPD(GOP_HW_Type u8GOP_num, U16 u16HSPD); // KimTH_091026
extern void MHal_GOP_Reg_Store(void);
extern void MHal_GOP_Reg_Restore(void);
extern void MHal_GOP_VCOM_Pattern(U32 active, U32 green_level, U32 buf_addr,
                                U32 h_offset, U32 v_offset,
                                U32 width, U32 height );
#if GOP_VSYNC_INTERRUPT_FLIP_ENABLE
extern B16 MHal_GOP_IOC_SetFlipInfo(U32 u32GopIdx, U32 u32GwinIdx, U32 u32Addr, U32 u32TagId, U32 * u32QEntry, U32 *u32Result);
#endif
extern void MHal_GOP_ResetGOP(void);

#define VERIFY_GOP 1

#if VERIFY_GOP
extern void* MHal_GOP_GetPool(U32 u32PoolSize);
#endif


#endif    // #ifndef __DRVGOP_H__

