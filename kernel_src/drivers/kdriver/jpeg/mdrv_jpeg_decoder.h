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

#ifndef MDRV_JPEG_DECODER_H
#define MDRV_JPEG_DECODER_H
//-----------------------------------------------------------------------------

#include "mdrv_types.h"
#include "mdrv_jpeg_st.h"
#include "mdrv_jpeg_io.h"

#ifdef _MDRV_JPEG_DECODER_C
#define JPEG_INTERRFACE
#else
#define JPEG_INTERRFACE extern
#endif

typedef enum
{
    RATIO_ORIGINAL = 0,
    RATIO_HALF = 1,
    RATIO_FOURTH = 2,
    RATIO_EIGHTH = 3
} EN_JPEG_DOWNSCALE_RATIO;
//------------------------------------------------------------------------------
#define RST0 0xD0
//------------------------------------------------------------------------------
typedef struct
{
    BOOL bValid;
    U8 u8Huff_num[17]; /* number of Huffman codes per bit size */
    U8 u8Huff_val[256]; /* Huffman codes per bit size */

    U8 u8Symbol[17]; /* u8Huff_num in reverse order.. */
    U16 u16Code[17]; /* Minimun code word */
} HUFF_INFO;
//------------------------------------------------------------------------------
typedef struct tagHUFF_TABLES
{
    S16 s16Look_up[256];
    U8 u8Code_size[256];
    // FIXME: Is 512 tree entries really enough to handle _all_ possible
    // code sets? I think so but not 100% positive.
    S16 s16Tree[512];
} HUFF_TABLES, *PHUFF_TABLES;
//------------------------------------------------------------------------------
typedef struct tagCOEFF_BUF
{
    U8 *pu8Data;

    U16 u16Block_num_x, u16Block_num_y;
    U8 u8Block_len_x, u8Block_len_y;

    U16 u16Block_size;
} COEFF_BUF, *PCOEFF_BUF;
//------------------------------------------------------------------------------
typedef struct tagQUANT_TABLES
{
    BOOL bValid;
    QUANT_TYPE s16Value[64];
} QUANT_TABLES, *PQUANT_TABLES;
//------------------------------------------------------------------------------
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

typedef enum
{
	RLE_DC = 0,
	RLE_AC,
	RLE_ZRL,
	RLE_EOB,
} EN_RLE_SYMBOL;


//------------------------------------------------------------------------------
typedef BOOL ( *Pdecode_block_func )( U8, U16, U16 );

//------------------------------------------------------------------------------
JPEG_INTERRFACE BOOL MDrv_JPEG_SetInitParameter(U32 u32RBufAddr, U32 u32RBufSize, U32 u32WBufAddr, U32 u32IBufAddr, U32 u32IBufSize, U8 u8FileEnd);
//JPEG_INTERRFACE BOOL msDrv_MJPEG_SetInitParameter(U32 u32RBufAddr, U32 u32RBufSize, U32 u32WBufAddr, U32 u32IBufAddr, U32 u32IBufSize);
//JPEG_INTERRFACE BOOL msDrv_MJPEG_ReSetMWC(U32 u32RBufAddr, U32 u32RBufSize, U32 u32WBufAddr, U32 u32IBufAddr, U32 u32IBufSize);
JPEG_INTERRFACE BOOL MDrv_JPEG_DecodeHeader( void );

JPEG_INTERRFACE BOOL MDrv_JPEG_constructor( U8 decode_type );
//JPEG_INTERRFACE S16 MDrv_JPEGbegin( void );

JPEG_INTERRFACE void MDrv_JPEG_StartDecode(void);
//JPEG_INTERRFACE void MDrv_JPEGGetOutputDimension(U16 *width, U16 *height);

JPEG_INTERRFACE S16 MDrv_JPEG_Progressive_Decode(void);
JPEG_INTERRFACE void MDrv_JPEG_Finalize( void );
JPEG_INTERRFACE S16 MDrv_JPEG_get_error_code( void );

// Get the width and height of JPEG picture based on specified decoding mode
JPEG_INTERRFACE U16 MDrv_JPEG_get_width( void );
JPEG_INTERRFACE U16 MDrv_JPEG_get_height( void );
// Get the original width and height of JPEG picture
JPEG_INTERRFACE U16 MDrv_JPEG_get_original_width( void );
JPEG_INTERRFACE U16 MDrv_JPEG_get_original_height( void );
JPEG_INTERRFACE U8 MDrv_JPEG_get_precision( void );
//JPEG_INTERRFACE U8* MDrv_JPEGget_CameraMake( void );
//JPEG_INTERRFACE U8* MDrv_JPEGget_CameraMode( void );
//JPEG_INTERRFACE U8* MDrv_JPEGget_CreationDate( void );
void MDrv_JPEG_load_next_row( void );
JPEG_INTERRFACE U8 MDrv_JPEG_get_num_components( void );
JPEG_INTERRFACE U32 MDrv_JPEG_get_total_bytes_read( void );
JPEG_INTERRFACE void MDrv_JPEG_Wakeup(U32 bytes_read);
//JPEG_INTERRFACE BOOL MDrv_JPEGPreLoadBuffer(U8 type);
//JPEG_INTERRFACE void MDrv_JPEGPrint_Qtbl(void);
BOOL MDrv_JPEG_IsProgressive(void);
//void MDrv_JPEGload_next_row( void );
EN_JPD_STATUS MDrv_JPEG_WaitDone(void);
B16 MDrv_JPEG_Wait(void);
// For debug
void MDrv_JPEG_DumpTables(void);
//void MDrv_JPEGSetDownScaleRatio(EN_JPEG_DOWNSCALE_RATIO ratio);
void MDrv_JPEG_SetMaxSize(int progressive_width, int progressive_height, int baseline_width, int baseline_height);
//void MDrv_JPEG_SetMIU(int miu_no);
void MDrv_JPEG_SetPowerOnOff(U8 on);


//------------------------------------------------------------------------------
// Extern Data members
//------------------------------------------------------------------------------

//JPEG_INTERRFACE PJPEG_DECODER_FILE_SYSTEM Pstream;

//------------------------------------------------------------------------------

/*
Framework :
(baseline)
1. MDrv_JPEGSetInitParameter( )
2. Set I/O callback functions for input stream
3. MDrv_JPEGconstructor( )
4. MDrv_JPEGbegin( )
5. MDrv_JPEGStartDecode( ) <-- do preload buffer and polling JPD status
6. MDrv_JPEGFinalize( )

(progressive)
1. MDrv_JPEGSetInitParameter( )
2. Set I/O callback functions for input stream
3. MDrv_JPEGconstructor( )
4. MDrv_JPEGbegin( )
5. MDrv_JPEGStart_Progressive_Decode( ) <-- do RLE and polling JPD status
6. MDrv_JPEGFinalize( )
*/

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------

