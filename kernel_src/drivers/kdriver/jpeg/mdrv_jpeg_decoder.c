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

#define _MDRV_JPEG_DECODER_C

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include "mdrv_jpeg_decoder.h"
#include "mhal_jpd.h"
#include "mhal_jpd_reg.h"
#include "mdrv_jpeg_memory.h"

#define JPEG_PRINT(fmt, args...)      // default OFF for MP3 with Photo, spmarine 080910 ::  printk("[JPEG][%06d]     " fmt, __LINE__, ## args)


int MAX_JPEG_WIDTH_HD = 3832;
int MAX_JPEG_HEIGHT_HD = 2152;

int MAX_JPEG_PROGRESSIVE_WIDTH  = 1920;
int MAX_JPEG_PROGRESSIVE_HEIGHT = 1440;

int MAX_JPEG_BASELINE_WIDTH  = 3832;
int MAX_JPEG_BASELINE_HEIGHT = 2152;

#define DEFAULT_DECODE_TIMEOUT  100

// PS. CMODEL always supports progressive mode decode
#define SUPPORT_PROGRESSIVE_MODE    1
#define SUPPORT_PROGRESSIVE_SCLAEDOWN_MODE    0


#define JPEG_DEBUG(fmt, args...)  //printk("[JPEG][%06d]     " fmt, __LINE__, ## args)

//-----------------------------------------------------------------------------
/// @brief \b Enum \b Name: JPEG_BuffLoadType
/// @brief \b Enum \b Description: JPEG buffer loading mode
//-----------------------------------------------------------------------------
//allen.chang 2009/12/30 patch
typedef enum
{
    E_JPEG_BUFFER_NONE  = 0
  , E_JPEG_BUFFER_HIGH  = 1
  , E_JPEG_BUFFER_LOW   = 2
} JPEG_BuffLoadType;

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
//------------------------------------------------------------------------------
#define IFAST_SCALE_BITS 2; /* fractional bits in scale factors */
//------------------------------------------------------------------------------
// Data members
//------------------------------------------------------------------------------
//TODO: add static for all ???

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

static BLOCK_TYPE _s16dc_pred[3];

// The width/height may be the thumbnail or original image size, it based on decoding mode
static U16 _u16Image_x_size;
static U16 _u16Image_y_size;

// The original size of this JPEG file
static U16 _u16OriginalImage_x_size;
static U16 _u16OriginalImage_y_size;

// The original thumbnail size of this JPEG file
U16 _u16OriginalThumbnail_x_size;
U16 _u16OriginalThumbnail_y_size;

//static PJPEG_DECODER_FILE_SYSTEM _pStream;

static U8 _u8DecodeType = JPEG_TYPE_MAIN;
static BOOL _bProgressive_flag;
static U8 _u8DownScaleRatio;
static BOOL _bFirstRLE;

/******* Thumbnail related *******/
static BOOL _bThumbnailFound;
static BOOL _bThumbnailAccessMode;
static U32 _u32ThumbnailOffset;
static U16 _u16ThumbnailSize;
static BOOL _bTiffBigEndian;

static U32 _u32ThumbnailBufferOffset;   // keep track of thumb buffer access address
static U32 _u16ThumbnailBufferSize;   // keep track of thumb buffer size
/*****************************/

static HUFF_INFO _Huff_info[JPEG_MAXHUFFTABLES];

static QUANT_TABLES _QuantTables[JPEG_MAXQUANTTABLES]; /* pointer to quantization tables */

static U8 _u8Precision;
static U8 _u8Comps_in_frame;                      /* # of components in frame */
static U8 _u8Comp_h_samp[JPEG_MAXCOMPONENTS];     /* component's horizontal sampling factor */
static U8 _u8Comp_v_samp[JPEG_MAXCOMPONENTS];     /* component's vertical sampling factor */
static U8 _u8Comp_quant[JPEG_MAXCOMPONENTS];      /* component's quantization table selector */
static U8 _u8Comp_ident[JPEG_MAXCOMPONENTS];      /* component's ID */

// The Luma and Chroma (YU) component ID, default is 1 & 2
static U8 _u8LumaCi = 1;
static U8 _u8ChromaCi = 2;

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

U8 gu8Max_mcu_x_size;                 /* MCU's max. X size in pixels */
U8 gu8Max_mcu_y_size;                 /* MCU's max. Y size in pixels */

static U8 _u8Blocks_per_mcu;
static U32 _u32Max_blocks_per_row;
static U16 _u16Mcus_per_row, _u16Mcus_per_col;

static U8 _u8Mcu_org[JPEG_MAXBLOCKSPERMCU];


#if SUPPORT_PROGRESSIVE_MODE
static U16 _u16Total_lines_left;               /* total # lines left in image */
U16 gu16Mcu_lines_left;                 /* total # lines left in this MCU */
static U32 _u32Block_y_mcu[JPEG_MAXCOMPONENTS];
static HUFF_TABLES _Huff_tbls[JPEG_MAXHUFFTABLES];
static PCOEFF_BUF _DC_Coeffs[JPEG_MAXCOMPONENTS];
static PCOEFF_BUF _AC_Coeffs[JPEG_MAXCOMPONENTS];
static U32 _u32Last_dc_val[JPEG_MAXCOMPONENTS];

static U32 _u32EOB_run;
#endif

U16 gu16Max_mcus_per_row;
static U16 _u16Max_blocks_per_mcu;
static U16 _u16Max_mcus_per_col;

U8 gu8Scan_type;                      /* Grey, Yh1v1, Yh1v2, Yh2v1, Yh2v2,
                                       CMYK111, CMYK4114 */
static U8 *_pu8In_buf_ofs;
static U32 _u32In_buf_left;
static U8 _u8Tem_flag;
static BOOL _bEOF_flag;

static U8 *_pu8In_buf;

static S16 _s16Bits_left;
static U32 _u32Bit_buf;

static U16 _u16Restart_interval;
static U16 _u16Restarts_left;
static U16 _u16Next_restart_num;

static void *_pBlocks[JPEG_MAXBLOCKS];         /* list of all dynamically allocated blocks */

static S16 _s16Error_code;
static BOOL _bReady_flag;

//static jmp_buf _jmp_state;
S32  _Total_Decoded_Size = 0;
static U32 _u32Total_bytes_read;

static U8 *_pu32ExifHeaderAddr;
#if 0
static U8 *_pu8CameraMake;
static U8 *_pu8CameraMode;
static U8 *_pu8CreationDate;
#endif

/* wait queue */
static wait_queue_head_t wq_ttx;
static atomic_t       _aWaitQueueGuard = ATOMIC_INIT(0);
static U8 _u8FileReadEnd_flag = FALSE;

static BOOL _Progressive_ROI_flag = FALSE;
static U16 ROI_width;

static JPEG_BuffLoadType u8PreloadLHFlag = E_JPEG_BUFFER_NONE;
extern void Chip_Flush_Memory(void);
//------------------------------------------------------------------------------
// end of Data members
//------------------------------------------------------------------------------
static void MDrv_JPEG_terminate( U16 status );
static BOOL MDrv_JPEG_process_restart( void );
//static void MDrv_JPEG_decode_init( PJPEG_DECODER_FILE_SYSTEM Pstream);
//------------------------------------------------------------------------------

// Refill the input buffer.
// This method will sit in a loop until (A) the buffer is full or (B)
// the stream's read() method reports an end of file condition.
static void MDrv_JPEG_fill_read_buffer(void)
{
    _u32In_buf_left = 0;
    _pu8In_buf_ofs = _pu8In_buf;

    if ( _bEOF_flag )
    {
        return;
    }

    if(_bThumbnailAccessMode)
    {
        if(_u16ThumbnailBufferSize>=MRC_BUFFER_SIZE)
        {
            memcpy((U8 *) (_pu8In_buf + _u32In_buf_left), (U8 *) (INTERNAL_BUFFER_ADDR + _u32ThumbnailBufferOffset), MRC_BUFFER_SIZE);
            _u32In_buf_left = MRC_BUFFER_SIZE;
            _u32ThumbnailBufferOffset += MRC_BUFFER_SIZE;
            _u16ThumbnailBufferSize -= MRC_BUFFER_SIZE;
        }
        else
        {
            memcpy((U8 *) (_pu8In_buf + _u32In_buf_left), (U8 *) (INTERNAL_BUFFER_ADDR + _u32ThumbnailBufferOffset), _u16ThumbnailBufferSize);
            _bEOF_flag = TRUE;
            _u32In_buf_left = _u16ThumbnailBufferSize;
            _u32ThumbnailBufferOffset += _u16ThumbnailBufferSize;
            _u16ThumbnailBufferSize = 0;
        }

        _u32Total_bytes_read += _u32In_buf_left;


    }
    else
    {
        volatile  U8* tmp;
        tmp = (volatile  U8*)(MRC_BUFFER_ADDR);
        tmp[0]=0x55;
        tmp[1]=0xAA;
        tmp[2]=0xAA;
        tmp[3]=0x55;
        MDrv_JPEG_Wait();
    }

#if 0 //need to check   sharon
    else
    {
        do
        {

            S32 bytes_read = _pStream->read( _pu8In_buf + _u32In_buf_left, MRC_BUFFER_SIZE - _u32In_buf_left, &_bEOF_flag, _pStream, 0 );
            if ( bytes_read == -1 )
            {
                MDrv_JPEG_terminate( JPEG_STREAM_READ );
            }
            _u32In_buf_left += bytes_read;
            _Total_Decoded_Size += bytes_read;
        } while ( ( _u32In_buf_left < MRC_BUFFER_SIZE ) && ( !_bEOF_flag ) );
    }
#endif


}



//------------------------------------------------------------------------------
// Logical rotate left operation.
static U32 MDrv_JPEG_rol( U32 i, U8 j )
{
    return ( ( i << j ) | ( i >> ( 32 - j ) ) );
}
//------------------------------------------------------------------------------
// Retrieve one character from the input stream.
static U8 MDrv_JPEG_get_char( void )
{
    U8 c;


    // Any bytes remaining in buffer?
    if ( !_u32In_buf_left )
    {
        // Try to get more bytes.
        MDrv_JPEG_fill_read_buffer();
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
//------------------------------------------------------------------------------
// Same as previus method, except can indicate if the character is
// a "padd" character or not.
static U8 MDrv_JPEG_get_charP( BOOL *Ppadding_flag )
{
    U8 t;
    U8 c;

    if ( !_u32In_buf_left )
    {
        MDrv_JPEG_fill_read_buffer();
        if ( !_u32In_buf_left )
        {
            *Ppadding_flag = TRUE;
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

    *Ppadding_flag = FALSE;

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
//------------------------------------------------------------------------------
// Retrieves one character from the input stream, but does
// not read past markers. Will continue to return 0xFF when a
// marker is encountered.
// FIXME: Bad name?
static U8 MDrv_JPEG_get_octet( void )
{
    BOOL padding_flag;
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

//------------------------------------------------------------------------------
// Retrieves a variable number of bits from the input stream.
// Markers will not be read into the input bit buffer. Instead,
// an infinite number of all 1's will be returned when a marker
// is encountered.
// FIXME: Is it better to return all 0's instead, like the older implementation?
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


//------------------------------------------------------------------------------
// Decodes a Huffman encoded symbol.
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
const S32 extend_test[16] =   /* entry n is 2**(n-1) */
{
    0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040,
    0x0080, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000
};

const S32 extend_offset[16] = /* entry n is (-1 << n) + 1 */
{
    0, ( ( -1 ) << 1 ) + 1, ( ( -1 ) << 2 ) + 1, ( ( -1 ) << 3 ) + 1, ( ( -1 ) << 4 ) + 1,
    ( ( -1 ) << 5 ) + 1, ( ( -1 ) << 6 ) + 1, ( ( -1 ) << 7 ) + 1, ( ( -1 ) << 8 ) + 1,
    ( ( -1 ) << 9 ) + 1, ( ( -1 ) << 10 ) + 1, ( ( -1 ) << 11 ) + 1, ( ( -1 ) << 12 ) + 1,
    ( ( -1 ) << 13 ) + 1, ( ( -1 ) << 14 ) + 1, ( ( -1 ) << 15 ) + 1
};

// used by huff_extend()
const S32 extend_mask[] =
{
    0, ( 1 << 0 ), ( 1 << 1 ), ( 1 << 2 ), ( 1 << 3 ),
    ( 1 << 4 ), ( 1 << 5 ), ( 1 << 6 ), ( 1 << 7 ),
    ( 1 << 8 ), ( 1 << 9 ), ( 1 << 10 ), ( 1 << 11 ),
    ( 1 << 12 ), ( 1 << 13 ), ( 1 << 14 ), ( 1 << 15 ), ( 1 << 16 ),
};

#define HUFF_EXTEND_TBL(x,s) ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

#define HUFF_EXTEND(x,s) HUFF_EXTEND_TBL(x,s)
#define HUFF_EXTEND_P(x,s) HUFF_EXTEND_TBL(x,s)

//------------------------------------------------------------------------------
// Unconditionally frees all allocated blocks.
static void MDrv_JPEG_free_all_blocks( void )
{
    U8 i;

//    if ( _pStream )
//    {
//        _pStream = NULL;
//    }

    for ( i = 0; i < JPEG_MAXBLOCKS; i++ )
    {
        if( _pBlocks[i]!=NULL )
            MDrv_JPEG_free( _pBlocks[i] );  //need to check    sharon

        _pBlocks[i] = NULL;
    }
}


//------------------------------------------------------------------------------
// This method handles all errors.
// It could easily be changed to use C++ exceptions.
static void MDrv_JPEG_terminate( U16 status )
{
    _s16Error_code = status;

    MDrv_JPEG_free_all_blocks();

    JPEG_PRINT("MDrv_JPEG_terminate  ERROR= %d\n", _s16Error_code);
    //longjmp( _jmp_state, status );

}
//------------------------------------------------------------------------------
// Allocate a block of memory-- store block's address in list for
// later deallocation by MDrv_JPEG_free_all_blocks().
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
        MDrv_JPEG_terminate( JPEG_TOO_MANY_BLOCKS );
        return NULL;
    }

    //FixMe: eCos does not support aligned allocate ???
    q = MDrv_JPEG_malloc( n + 8 );
    //q = MsOS_AllocateAlignedMemory(n+8, 8, gs32CachedPoolID);
    //q = MsOS_AllocateMemory(n+8, gs32CachedPoolID);

    if ( q == NULL )
    {
        MDrv_JPEG_terminate( JPEG_NOTENOUGHMEM );
        return NULL;
    }

    memset( q, 0, n + 8 );

    _pBlocks[i] = q;

    JPEG_DEBUG("JPEG %d bytes allocated\n", n);

    return ( ( void* )q );
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

#if 0
#define JPEG_TIFF_JPEG_IMG_CAMERA_MAKE 0x010F
#define JPEG_TIFF_JPEG_IMG_CAMERA_MODE 0x0110
#define JPEG_TIFF_JPEG_IMG_DATE_TIME 0x0132
#endif
//------------------------------------------------------------------------------
static U32 MDrv_JPEG_Tiff_EndianChangeL(U32 u32Val)
{
    if (_bTiffBigEndian)
        return EndianChangeL(u32Val);
    else
        return u32Val;
}

static U16 MDrv_JPEG_Tiff_EndianChangeS(U16 u16Val)
{
    if (_bTiffBigEndian)
        return EndianChangeS(u16Val);
    else
        return u16Val;
}

static U16 MDrv_JPEG_GetU16(U8 *data)
{
    S8 i;
    U16 val = 0;

    for(i = 1; i>=0; i--)
        val = (val << 8) + (U8) *(data + i);


    return val;
}

static U32 MDrv_JPEG_GetU32(U8 *data)
{
    S8 i;
    U32 val = 0;

    for(i = 3; i>=0; i--)
        val = (val << 8) + (U8) *(data + i);

    return val;
}
//------------------------------------------------------------------------------
//***************************************************
//Parse EXIF header
//***************************************************
static BOOL MDrv_JPEG_DecodeExifInfo(U8 *data, U32 data_length)
{
    U8 *pJpegBuff = data;
    U32  u32tmp, u32Len;
    U16  u16Marker, u16Len, u16NumOfEntry, i;
    U8   *pTiffHdrPtr, *pNextIfd;

    u32tmp = MDrv_JPEG_GetU32(pJpegBuff);
    if (u32tmp != JPEG_TAG_EXIF)
    {
        return FALSE;
    }

    // Exif starts here
    pJpegBuff += 6;
    u16Marker = MDrv_JPEG_GetU16(pJpegBuff);
    if (u16Marker == JPEG_TIFF_BIG_ENDIAN)
        _bTiffBigEndian = TRUE;
    else if (u16Marker == JPEG_TIFF_LITTLE_ENDIAN)
        _bTiffBigEndian = FALSE;
    else
        return FALSE;

    _u32ThumbnailOffset += 6;
    pTiffHdrPtr = pJpegBuff;

    pJpegBuff += 2;
    u16Marker = MDrv_JPEG_Tiff_EndianChangeS(MDrv_JPEG_GetU16(pJpegBuff));
    if (u16Marker != 0x002A)
        return FALSE;

    pJpegBuff += 2;
    u16Len = (U16)MDrv_JPEG_Tiff_EndianChangeL(MDrv_JPEG_GetU32(pJpegBuff));
    pJpegBuff += (u16Len - 4);

    // 0th IFD start here
    u16NumOfEntry = MDrv_JPEG_Tiff_EndianChangeS(MDrv_JPEG_GetU16(pJpegBuff));
    pJpegBuff += 2;

    // Boundary check, prevent from buffer over-run
    if((((U32) pJpegBuff) - INTERNAL_BUFFER_ADDR + u16NumOfEntry*12)>=data_length)
    {
        return FALSE;
    }
#if 0
 for (i = 0; i < u16NumOfEntry; i++)
    {
        u16Marker = MDrv_JPEG_Tiff_EndianChangeS(MDrv_JPEG_GetU16(pJpegBuff));
        u32Len = MDrv_JPEG_Tiff_EndianChangeL(MDrv_JPEG_GetU32(pJpegBuff + 4));
        u32tmp = MDrv_JPEG_Tiff_EndianChangeL(MDrv_JPEG_GetU32(pJpegBuff + 8));

        switch (u16Marker)
        {
            case JPEG_TIFF_JPEG_IMG_CAMERA_MAKE:
                _pu8CameraMake = &pTiffHdrPtr[u32tmp];
                JPEG_PRINT("\nCamera Make is %s\n",_pu8CameraMake);
                break;
            case JPEG_TIFF_JPEG_IMG_CAMERA_MODE:
                _pu8CameraMode= &pTiffHdrPtr[u32tmp];
                JPEG_PRINT("Camera Mode is %s\n",_pu8CameraMode);
                break;
            case JPEG_TIFF_JPEG_IMG_DATE_TIME:
                _pu8CreationDate= &pTiffHdrPtr[u32tmp];
                JPEG_PRINT("DateTime is %s\n\n",_pu8CreationDate);
                break;
        }

        pJpegBuff += 12;
    }
#else
   pJpegBuff += 12*u16NumOfEntry;
#endif
    // 1th IFD
    u32tmp = MDrv_JPEG_Tiff_EndianChangeL(MDrv_JPEG_GetU32(pJpegBuff));
    if (u32tmp == 0)
        return FALSE;
    else
        pNextIfd = &pTiffHdrPtr[u32tmp];

    u16NumOfEntry = MDrv_JPEG_Tiff_EndianChangeS(MDrv_JPEG_GetU16(pNextIfd));
    pNextIfd += 2;

    // Boundary check, prevent from buffer over-run
    if((((U32) pNextIfd) - INTERNAL_BUFFER_ADDR + u16NumOfEntry*12)>=data_length)
    {
        return FALSE;
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
                _bThumbnailFound= TRUE;
                _u32ThumbnailOffset += u32tmp;
                break;
        }

        pNextIfd += 12;
    }

    // Boundary check, prevent from buffer over-run
    if(_bThumbnailFound)
    {
        if((_u32ThumbnailOffset + _u16ThumbnailSize) > data_length)
            _bThumbnailFound = FALSE;

        // means it only contains SOI header..
        if(_u16ThumbnailSize<=2)
            _bThumbnailFound = FALSE;
    }

    return _bThumbnailFound;
}


//------------------------------------------------------------------------------
// Read exif info
static BOOL MDrv_JPEG_read_app1_marker( void )
{
    U16 length;
    U8 *exif_buffer = (U8 *) INTERNAL_BUFFER_ADDR;
    U16 i = 0;

    JPEG_DEBUG("APP1\n");

    length = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( length < 2 )
    {
        MDrv_JPEG_terminate( JPEG_BAD_APP1_MARKER );
        return FALSE;
    }

    length -= 2;

    while((length - i)!=0)
    {
        exif_buffer[i] = (U8) MDrv_JPEG_get_char();
        i++;
    }

    if(MDrv_JPEG_DecodeExifInfo(exif_buffer, length)==TRUE)
    {
        JPEG_PRINT("FOUND THUMBNAIL!\n");
        _u32ThumbnailBufferOffset = _u32ThumbnailOffset;
        _u16ThumbnailBufferSize = _u16ThumbnailSize;
    }
    else
    {
        JPEG_PRINT("NO THUMBNAIL!\n");
    }
    return TRUE;
}
/* END OF EXIF PARSING SECTION */
//------------------------------------------------------------------------------
// Read a Huffman code table.
static BOOL MDrv_JPEG_read_dht_marker( void )
{
    U16 i, index, count;
    U32 left;
    U8 u8Huff_num[17];
    U8 u8Huff_val[256];

    JPEG_DEBUG("DHT\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( left < 2 )
    {
        MDrv_JPEG_terminate( JPEG_BAD_DHT_MARKER );
        return FALSE;
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
            MDrv_JPEG_terminate( JPEG_BAD_DHT_COUNTS );
            return FALSE;
        }

        for ( i = 0; i < count; i++ )
        {
            u8Huff_val[i] = MDrv_JPEG_get_char();
        }

        i = 1 + 16 + count;

        if ( left < ( U32 )i )
        {
            MDrv_JPEG_terminate( JPEG_BAD_DHT_MARKER );
            return FALSE;
        }

        left -= i;

        if ( ( index & 0x10 ) > 0x10 ) //no need ???
        {
            MDrv_JPEG_terminate( JPEG_BAD_DHT_INDEX );
            return FALSE;
        }

        index = ( index & 0x0F ) + ( ( index & 0x10 ) >> 4 ) * ( JPEG_MAXHUFFTABLES >> 1 ); //???

        if ( index >= JPEG_MAXHUFFTABLES )
        {
            MDrv_JPEG_terminate( JPEG_BAD_DHT_INDEX );
            return FALSE;
        }

        if(_Huff_info[index].bValid==FALSE)
            _Huff_info[index].bValid = TRUE;

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
    return TRUE;
}


//------------------------------------------------------------------------------
// Read a quantization table.
static BOOL MDrv_JPEG_read_dqt_marker( void )
{
    U16 n, i, prec;
    U32 left;
    U32 temp;

    //JPEG_PRINT("DQT\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( left < 2 )
    {
        MDrv_JPEG_terminate( JPEG_BAD_DQT_MARKER );
        return FALSE;
    }

    left -= 2;

    while ( left )
    {
        n = MDrv_JPEG_get_char();
        prec = n >> 4;
        n &= 0x0F;

        if ( n >= JPEG_MAXQUANTTABLES )
        {
            MDrv_JPEG_terminate( JPEG_BAD_DQT_TABLE );
            return FALSE;
        }

        if(_QuantTables[n].bValid==FALSE)
            _QuantTables[n].bValid = TRUE;

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
            MDrv_JPEG_terminate( JPEG_BAD_DQT_LENGTH );
            return FALSE;
        }

        left -= i;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
// Read the start of frame (SOF) marker.
static BOOL MDrv_JPEG_read_sof_marker( void )
{
    U8 i,tmp;
    U32 left;

    JPEG_DEBUG("SOF\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());
    _u8Precision = MDrv_JPEG_get_char();

    if ( _u8Precision != 8 )   /* precision: sorry, only 8-bit precision is supported right now */
    {
        MDrv_JPEG_terminate( JPEG_BAD_PRECISION );
        return FALSE;
    }

    _u16Image_y_size = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( ( _u16Image_y_size < JPEG_MIN_HEIGHT ) || ( _u16Image_y_size > JPEG_MAX_HEIGHT ) )
    {
        MDrv_JPEG_terminate( JPEG_BAD_HEIGHT );
        return FALSE;
    }

    _u16Image_x_size = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());


    if ( ( _u16Image_x_size < JPEG_MIN_WIDTH) || ( _u16Image_x_size > JPEG_MAX_WIDTH ) )
    {
        MDrv_JPEG_terminate( JPEG_BAD_WIDTH );
        return FALSE;
    }

    //allen.chang 2009/09/30 keep original thumbanil size

     _u16OriginalThumbnail_x_size = _u16Image_x_size;
     _u16OriginalThumbnail_y_size = _u16Image_y_size;

    //----------

    _u8Comps_in_frame = MDrv_JPEG_get_char();

    if ( _u8Comps_in_frame > JPEG_MAXCOMPONENTS )
    {
        MDrv_JPEG_terminate( JPEG_TOO_MANY_COMPONENTS );
        return FALSE;
    }

    if ( left != ( U32 )( _u8Comps_in_frame * 3 + 8 ) )
    {
        MDrv_JPEG_terminate( JPEG_BAD_SOF_LENGTH );
        return FALSE;
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
    return TRUE;
}


//------------------------------------------------------------------------------
// Used to skip unrecognized markers.
static void MDrv_JPEG_skip_bytes(U32 count)
{
    while(count!=0)
    {
        // Any bytes remaining in buffer?
        if ( !_u32In_buf_left )
        {
            // Try to get more bytes.
            MDrv_JPEG_fill_read_buffer();
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

//------------------------------------------------------------------------------
// Used to skip unrecognized markers.
static BOOL MDrv_JPEG_skip_variable_marker( void )
{
    U32 left;

    JPEG_DEBUG("SKIP markers\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    if ( left < 2 )
    {
        MDrv_JPEG_terminate( JPEG_BAD_VARIABLE_MARKER );
        return FALSE;
    }

    left -= 2;

    MDrv_JPEG_skip_bytes(left);
    return TRUE;
}


//------------------------------------------------------------------------------
// Read a define restart interval (DRI) marker.
static BOOL MDrv_JPEG_read_dri_marker( void )
{
    JPEG_DEBUG("DRI\n");

    if ( (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char()) != 4 )
    {
        MDrv_JPEG_terminate( JPEG_BAD_DRI_LENGTH );
        return FALSE;
    }

    _u16Restart_interval = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());
    return TRUE;
}


//------------------------------------------------------------------------------
// Read a start of scan (SOS) marker.
static BOOL MDrv_JPEG_read_sos_marker( void )
{
    U32 left;
    U16 i, ci, n, c, cc;
    U8 tmp;

    JPEG_DEBUG("SOS\n");

    left = (U16)((MDrv_JPEG_get_char()<<8)+MDrv_JPEG_get_char());

    n = MDrv_JPEG_get_char();

    _u8Comps_in_scan = n;

    left -= 3;

    if ( ( left != ( U32 )( n * 2 + 3 ) ) || ( n < 1 ) || ( n > JPEG_MAXCOMPSINSCAN ) )
    {
        MDrv_JPEG_terminate( JPEG_BAD_SOS_LENGTH );
        return FALSE;
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
            MDrv_JPEG_terminate( JPEG_BAD_SOS_COMP_ID );
            return FALSE;
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

    return TRUE;
}


//------------------------------------------------------------------------------
// Finds the next marker.
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


//------------------------------------------------------------------------------
// Process markers. Returns when an SOFx, SOI, EOI, or SOS marker is
// encountered.
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
                if( (_u8DecodeType==JPEG_TYPE_THUMBNAIL) && (_bThumbnailFound==FALSE) )
                {
                    if( !MDrv_JPEG_read_app1_marker() )
                        return FALSE;
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
                        return FALSE;
                    break;
                }
                // Sorry, no arithmitic support at this time. Dumb patents!
            case M_DAC:
                {
                    MDrv_JPEG_terminate( JPEG_NO_ARITHMETIC_SUPPORT );
                        return FALSE;
                    break;
                }
            case M_DQT:
                {
                    if( !MDrv_JPEG_read_dqt_marker() )
                        return FALSE;
                    break;
                }
            case M_DRI:
                {
                    if( !MDrv_JPEG_read_dri_marker() )
                        return FALSE;
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
                    MDrv_JPEG_terminate( JPEG_UNEXPECTED_MARKER );
                    return FALSE;
                }
            default:
                /* must be DNL, DHP, EXP, APPn, JPGn, COM, or RESn or APP0 */
                {
                    MDrv_JPEG_skip_variable_marker();
                    break;
                }
        }
    }
    return TRUE;
}


//------------------------------------------------------------------------------
// Finds the start of image (SOI) marker.
// This code is rather defensive: it only checks the first 512 bytes to avoid
// false positives.
static BOOL MDrv_JPEG_locate_soi_marker( void )
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
        return TRUE;
    }

    bytesleft = 640; //Fix this number from 512 -> 640 for some cases

    for ( ; ; )
    {
        if ( --bytesleft == 0 )
        {
            MDrv_JPEG_terminate( JPEG_NOT_JPEG );
            return FALSE;
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
        MDrv_JPEG_terminate( JPEG_NOT_JPEG );
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
// Find a start of frame (SOF) marker.
static BOOL MDrv_JPEG_locate_sof_marker( void )
{
    U32 c;

    if( !MDrv_JPEG_locate_soi_marker() )
        return FALSE;

    c = MDrv_JPEG_process_markers();

    if( c == FALSE )
        return FALSE;

    switch ( c )
    {
        case M_SOF2:
            JPEG_DEBUG("Progressive\n");
            _bProgressive_flag = TRUE;
        case M_SOF0:
            /* baseline DCT */
        case M_SOF1:
            /* extended sequential DCT */
            {
                if(c==M_SOF0||c==M_SOF1)
                {
                    JPEG_DEBUG("Baseline\n");
                }
                if( !MDrv_JPEG_read_sof_marker() )
                    return FALSE;
                break;
            }
        case M_SOF9:
            /* Arithmetic coding */
            {
                MDrv_JPEG_terminate( JPEG_NO_ARITHMETIC_SUPPORT );
                return FALSE;
            }

        default:
            {
                MDrv_JPEG_terminate( JPEG_UNSUPPORTED_MARKER );
                return FALSE;
            }
    }
    return TRUE;
}


//------------------------------------------------------------------------------
// Find a start of scan (SOS) marker.
static BOOL MDrv_JPEG_locate_sos_marker( void )
{
    U32 c;

    c = MDrv_JPEG_process_markers();

    if( c == FALSE )
        return FALSE;

    if ( c == M_EOI )
    {
        return FALSE;
    }
    else if ( c != M_SOS )
    {
        MDrv_JPEG_terminate( JPEG_UNEXPECTED_MARKER );
        return FALSE;
    }

    if( !MDrv_JPEG_read_sos_marker())
        return FALSE;

    return TRUE;
}


//------------------------------------------------------------------------------
// Reset thumbnail parameters
static void MDrv_JPEG_init_thumbnail(void)
{
    _bThumbnailFound = FALSE;
    _u32ThumbnailOffset = 0;
    _u16ThumbnailSize = 0;
    _bTiffBigEndian = FALSE;

    _u32ThumbnailBufferOffset = 0;
    _u16ThumbnailSize = 0;
    _bThumbnailAccessMode = FALSE;
}


//------------------------------------------------------------------------------
// Reset everything to default/uninitialized state.
//static void MDrv_JPEG_init( PJPEG_DECODER_FILE_SYSTEM _Pstream )
static void MDrv_JPEG_init( void )
{
    U16 i;

    _u8LumaCi = 1;
    _u8ChromaCi = 2;

    _s16Error_code = 0;

    _bReady_flag = FALSE;

    _u16Image_x_size = _u16Image_y_size = 0;
    _u16OriginalImage_x_size = _u16OriginalImage_y_size = 0;

    //_pStream = _Pstream;

    _bProgressive_flag = FALSE;
    _u8DownScaleRatio = JPD_DOWNSCALE_ORG;
    _u32RLEOffset = 0;
    _bFirstRLE = FALSE;

    _s16dc_pred[0] = _s16dc_pred[1] = _s16dc_pred[2] = 0;

    u8PreloadLHFlag = E_JPEG_BUFFER_NONE;

    for(i = 0; i<JPEG_MAXHUFFTABLES; i++)
    {
        _Huff_info[i].bValid = FALSE;
        memset(_Huff_info[i].u8Huff_num, 0, 17);
        memset(_Huff_info[i].u8Huff_val, 0, 256);
        memset(_Huff_info[i].u8Symbol, 0, 17);
        memset(_Huff_info[i].u16Code, 0, 17);
    }

    for(i = 0; i<JPEG_MAXQUANTTABLES; i++)
    {
        _QuantTables[i].bValid = FALSE;
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
    _bEOF_flag = FALSE;
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
        MDrv_JPEG_fill_read_buffer();
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

    #ifdef CMODEL
    for ( i = 0; i < JPEG_MAXBLOCKSPERROW; i++ )
    {
        _u8Block_max_zag_set[i] = 64;
    }
    #endif

    init_waitqueue_head(&wq_ttx);
}


//------------------------------------------------------------------------------
// The coeff_buf series of methods originally stored the coefficients
// into a "virtual" file which was located in EMS, XMS, or a disk file. A cache
// was used to make this process more efficient. Now, we can store the entire
// thing in RAM.
static PCOEFF_BUF MDrv_JPEG_coeff_buf_open( U16 block_num_x, U16 block_num_y, U8 block_len_x, U8 block_len_y )
{
    PCOEFF_BUF cb = ( PCOEFF_BUF )alloc( sizeof( COEFF_BUF ) );
    if(cb == NULL)
    {
        MDrv_JPEG_terminate( JPEG_NOTENOUGHMEM );
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
        MDrv_JPEG_terminate( JPEG_NOTENOUGHMEM );
    }

    return cb;
}


//------------------------------------------------------------------------------
static BLOCK_TYPE * MDrv_JPEG_coeff_buf_getp( PCOEFF_BUF cb, U16 block_x, U16 block_y )
{
    if ( block_x >= cb->u16Block_num_x )
    {
        MDrv_JPEG_terminate( JPEG_ASSERTION_ERROR );
        return NULL;
    }

    if ( block_y >= cb->u16Block_num_y )
    {
        MDrv_JPEG_terminate( JPEG_ASSERTION_ERROR );
        return NULL;
    }

    return ( BLOCK_TYPE * )( cb->pu8Data + block_x * cb->u16Block_size + block_y * ( cb->u16Block_size * cb->u16Block_num_x ) );
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Creates the tables needed for efficient Huffman decoding.
static BOOL MDrv_JPEG_make_huff_table( U8 index)
{
    U16 p, i, l, si;
    U8 huffsize[257];
    U16 huffcode[257];
    U16 code;
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
                MDrv_JPEG_terminate( JPEG_UNDEFINED_HUFF_TABLE );
                return FALSE;
            }
        }
    }

    huffsize[p] = 0;

    lastp = p;

    code = 0;
    si = huffsize[0];
    p = 0;

    while ( huffsize[p] )
    {
        while ( huffsize[p] == si )
        {
            huffcode[p++] = code;
            code++;

            //kevinhuang, add protection
            if ( p >= 257 )
            {
                MDrv_JPEG_terminate( JPEG_UNDEFINED_HUFF_TABLE );
                return FALSE;
            }
        }

        code <<= 1;
        si++;
    }

    // Calculate the min code
    for(i = 1; i<=16; i++)
        _Huff_info[index].u16Code[i] = huffcode[_Huff_info[index].u8Symbol[i]] << (15 - (i - 1));


    #if SUPPORT_PROGRESSIVE_MODE

    // In JPD mode, SW doesn't need huff table when baseline decoding
    if(_bProgressive_flag == FALSE)
        return TRUE;

    memset( hs->s16Look_up, 0, sizeof( hs->s16Look_up ) );
    memset( hs->s16Tree, 0, sizeof( hs->s16Tree ) );
    memset( hs->u8Code_size, 0, sizeof( hs->u8Code_size ) );

    nextfreeentry = -1;

    p = 0;

    while ( p < lastp )
    {
        i = _Huff_info[index].u8Huff_val[p];
        code = huffcode[p];
        code_size = huffsize[p];

        hs->u8Code_size[i] = code_size;

        if ( code_size <= 8 )
        {
            code <<= ( 8 - code_size );

            for ( l = 1 << ( 8 - code_size ); l > 0; l-- )
            {
                hs->s16Look_up[code] = i;
                code++;
            }
        }
        else
        {
            subtree = ( code >> ( code_size - 8 ) ) & 0xFF;

            currententry = hs->s16Look_up[subtree];

            if ( currententry == 0 )
            {
                hs->s16Look_up[subtree] = currententry = nextfreeentry;

                nextfreeentry -= 2;
            }

            code <<= ( 16 - ( code_size - 8 ) );

            for ( l = code_size; l > 9; l-- )
            {
                if ( ( code & 0x8000 ) == 0 )
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

                code <<= 1;
            }

            if ( ( code & 0x8000 ) == 0 )
            {
                currententry--;
            }

            hs->s16Tree[-currententry - 1] = i;
        }

        p++;
    }
    #endif  //SUPPORT_PROGRESSIVE_MODE
    return TRUE;
}


//------------------------------------------------------------------------------
// Verifies the quantization tables needed for this scan are available.
static BOOL MDrv_JPEG_check_quant_tables( void ) //ok
{
    U8 i;

    for ( i = 0; i < _u8Comps_in_scan; i++ )
    {
        if ( _QuantTables[_u8Comp_quant[_u8Comp_list[i]]].bValid==FALSE )
        {
            MDrv_JPEG_terminate( JPEG_UNDEFINED_QUANT_TABLE );
            return FALSE;
        }
    }
    return TRUE;
}


//------------------------------------------------------------------------------
// Verifies that all the Huffman tables needed for this scan are available.
static BOOL MDrv_JPEG_check_huff_tables( void )
{
    U8 i;

    for ( i = 0; i < _u8Comps_in_scan; i++ )
    {
        if ( ( _u8Spectral_start == 0 ) && ( _Huff_info[_u8Comp_dc_tab[_u8Comp_list[i]]].bValid== FALSE ) )
        {
            MDrv_JPEG_terminate( JPEG_UNDEFINED_HUFF_TABLE );
            return FALSE;
        }

        if ( ( _u8Spectral_end > 0 ) && ( _Huff_info[_u8Comp_ac_tab[_u8Comp_list[i]]].bValid== FALSE ) )
        {
            MDrv_JPEG_terminate( JPEG_UNDEFINED_HUFF_TABLE );
            return FALSE;
        }
    }

    for ( i = 0; i < JPEG_MAXHUFFTABLES; i++ )
    {
        if ( _Huff_info[i].bValid )
        {
            if( !MDrv_JPEG_make_huff_table(i))
                return FALSE;
        }
    }
    return TRUE;
}


//------------------------------------------------------------------------------
// Determines the component order inside each MCU.
// Also calcs how many MCU's are on each row, etc.
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


//------------------------------------------------------------------------------
/* Get current access byte address in MRC buffer relative to MRC start address */
U32 MDrv_JPEG_GetECS(void)
{
    //JPEG_PRINT("_u32Total_bytes_read = %08X\n",_u32Total_bytes_read);
    //JPEG_PRINT("_u32In_buf_left = %08X\n",_u32In_buf_left);
    return (_u32Total_bytes_read - _u32In_buf_left);
}


//------------------------------------------------------------------------------
//*************************************************
//write symbol table
//*************************************************
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


//------------------------------------------------------------------------------
//***************************************************
//write quantization table
//***************************************************
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


//------------------------------------------------------------------------------
//*************************************************
//write group information
//*************************************************
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


//------------------------------------------------------------------------------
// Write RLE result
BOOL MDrv_JPEG_write_RLE(PSVLD pVld, BOOL bDecodeNow)
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
    if(_u32RLEOffset==MRC_BUFFER_SIZE || bDecodeNow==TRUE)
    {
        JPEG_DEBUG("Do RLE, LENG %d\n", _u32RLEOffset);

        Chip_Flush_Memory(); //2009/12/31 patch

        if(_bFirstRLE==TRUE)
        {
            // clear MRC low/high portion read complete event
            MHal_JPD_ClearJPDStatus(JPD_EVENT_MRBH_DONE | JPD_EVENT_MRBL_DONE);
            // mark low/high buffer valid
            reg_value = (JPD_REG(REG_JPD_MCONFIG) & ~0x0003) | (JPD_H_VLD |JPD_L_VLD);
            JPD_REG(REG_JPD_MCONFIG) = reg_value;
            //MDrv_Write2Byte( REG_JPD_MCONFIG, (MDrv_Read2Byte(REG_JPD_MCONFIG) & ~0x0003) | (JPD_H_VLD |JPD_L_VLD));
        }

        // Trigger JPD decoding
        if(_bFirstRLE==FALSE)
        {
            MDrv_JPEG_StartDecode();
            _bFirstRLE = TRUE;
        }

        //sharon start_time = MsOS_GetSystemTime();
        cur_vIdx = JPD_REG(REG_JPD_CUR_VIDX);

        JPEG_DEBUG("cur_vIdx = %x\n",cur_vIdx);

        if( bDecodeNow )
        {
            return TRUE;  //wait done in main loop
        }

        while( u16Count > 0 )
        {
            status = MHal_JPD_ReadJPDStatus();

            if(status & JPD_EVENT_DEC_DONE)
            {
                JPEG_DEBUG("P deocde done\n");
                break;
            }

            if((status & JPD_EVENT_ECS_ERROR) || (status & JPD_EVENT_IS_ERROR) || (status & JPD_EVENT_RST_ERROR))
            {
                 // temp patch for protect JPD from writing to illegal memory
                MHal_JPD_SW_Pause_Reset();
                MDrv_JPEG_terminate( JPEG_SVLD_DECODE_ERROR );
                return FALSE;
            }

            if((status & JPD_EVENT_MRBH_DONE) && (status & JPD_EVENT_MRBL_DONE))
            {
                JPEG_DEBUG("Partial SVLD decode done\n");
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
                    JPEG_DEBUG("ERROR: SVLD deocde time out, VIdx %d\n", cur_vIdx);
                    break;
                }
            }
            msleep(1);
            if((u16Count--) == 0)
            {
                JPEG_PRINT("MDrv_JPEG_write_RLE Timeout\n");
                return TRUE;
            }
        }

        _u32RLEOffset = 0;
    }
    return TRUE;
}


#define ABS(x)    ((x>=0)?(x):(-x))
//------------------------------------------------------------------------------
// Do run length encode of coefficient buffer
void MDrv_JPEG_do_RLE(BLOCK_TYPE *p, BOOL eop, U8 comp_id,BOOL BlockInRange)
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
    if( BlockInRange )//Current block is within display range.
        my_vld.run = 8;
    else
        my_vld.run = 0;

    value = (p[0] - predictor);
    my_vld.sign = (value<0)?1:0;
    my_vld.amp = ABS(value);
    my_vld.sym_type = RLE_DC;
    MDrv_JPEG_write_RLE(&my_vld, FALSE);

   if( BlockInRange == FALSE )//Current block is within display range.
        return;

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
                MDrv_JPEG_write_RLE(&my_vld, FALSE);

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
                MDrv_JPEG_write_RLE(&my_vld, TRUE);

                _s16dc_pred[cur_blk - 1] = p[0];//update predictor
                return;
            }
            else
                MDrv_JPEG_write_RLE(&my_vld, FALSE);

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
            MDrv_JPEG_write_RLE(&my_vld, TRUE);
        }
        else
            MDrv_JPEG_write_RLE(&my_vld, FALSE);
    }

    _s16dc_pred[cur_blk - 1] = p[0];//update predictor
}


//------------------------------------------------------------------------------
#if 0  //buffer will be loaded in user space
/******************************************************************************/
///Preload buffer from JPEG content
///@param type \b IN indicate which parts of buffer wants to load
///The calling order must be HIGH -> LOW -> HIGH -> LOW.... or LOW -> HIGH -> LOW -> HIGH
///Because the file offset is maintained outside, we assume that calling won't be out of order
/******************************************************************************/
BOOL MDrv_JPEG_PreLoadBuffer(U8 type)
{
    S32 bytes_read;
    U8 *pBuffer_addr, Position;
    U32 buf_left = 0;
    U32 reg_value;

    if ( _bEOF_flag )
    {
        JPEG_PRINT("end of file!!\n");
        return FALSE;
    }

//    if ( setjmp( _jmp_state ) )
//    {
//        return ( FALSE );
//    }


    if(type==JPEG_BUFFER_HIGH)
    {
        pBuffer_addr = _pu8In_buf + (MRC_BUFFER_SIZE/2);
        Position = 1;
    }
    else
    {
        pBuffer_addr = _pu8In_buf;
        Position = 0;
    }

    do
    {
        //need to check  sharon
        //bytes_read = JPEG_DECODER_FILE_SYSTEM_read( pBuffer_addr + buf_left, MRC_BUFFER_SIZE/2 - buf_left, &_bEOF_flag, _pStream, Position );

        if ( bytes_read == -1 )
        {
            MDrv_JPEG_terminate( JPEG_STREAM_READ );
        }

        buf_left += bytes_read;
        _Total_Decoded_Size += bytes_read;
    } while(( buf_left < MRC_BUFFER_SIZE/2 ) && ( !_bEOF_flag ));


    if(type==JPEG_BUFFER_HIGH)
    {
        // clear MRC high portion read complete event
        MHal_JPD_ClearJPDStatus(JPD_EVENT_MRBH_DONE);
        // mark high portion valid

        reg_value = (JPD_REG(REG_JPD_MCONFIG) & ~0x0003) | JPD_H_VLD;
        JPD_REG(REG_JPD_MCONFIG) = reg_value;
        //MDrv_Write2Byte( REG_JPD_MCONFIG, (MDrv_Read2Byte(REG_JPD_MCONFIG) & ~0x0003) | JPD_H_VLD);
    }
    else
    {
        // clear MRC low portion read complete event
        MHal_JPD_ClearJPDStatus(JPD_EVENT_MRBL_DONE);
        // mark low portion valid
        reg_value = (JPD_REG(REG_JPD_MCONFIG) & ~0x0003) | JPD_L_VLD;
        JPD_REG(REG_JPD_MCONFIG) = reg_value;
        //MDrv_Write2Byte( REG_JPD_MCONFIG, (MDrv_Read2Byte(REG_JPD_MCONFIG) & ~0x0003) | JPD_L_VLD);
    }

    return TRUE;
}
#endif

//------------------------------------------------------------------------------
/******************************************************************************/
///Start JPEG decoding
/******************************************************************************/
void MDrv_JPEG_StartDecode(void)
{
    U8 mcu_width, mcu_height;
    U16 pic_width = _u16Image_x_size;
    U16 pic_height = _u16Image_y_size;
    U8 Y_VSF = _u8Comp_v_samp[0];
    U8 Y_HSF = _u8Comp_h_samp[0];
    BOOL bUV_en;
    U32 reg_value;
    U8 i,RatioVal, Factor = 1;
    U8 com_num = 0;
    U8 comp[JPEG_MAXCOMPONENTS];

    JPEG_DEBUG("Start JPD decode\n");

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
        bUV_en = TRUE;
    else
        bUV_en = FALSE;

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
        MHal_JPD_SetMRCStartAddr(INTERNAL_BUFFER_ADDR + _u32ThumbnailOffset + MDrv_JPEG_GetECS());
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

    //Don't care progressive JPEG here,Progressive must be down scale 1/1
    MAX_JPEG_WIDTH_HD = MAX_JPEG_BASELINE_WIDTH;
    MAX_JPEG_HEIGHT_HD = MAX_JPEG_BASELINE_HEIGHT;

#if SUPPORT_PROGRESSIVE_SCLAEDOWN_MODE
    if(_bProgressive_flag)
    {
        MAX_JPEG_WIDTH_HD = MAX_JPEG_PROGRESSIVE_WIDTH;
        MAX_JPEG_HEIGHT_HD = MAX_JPEG_PROGRESSIVE_HEIGHT;
    }
#endif

    JPEG_PRINT("Width = %d and Height = %d\n",pic_width,pic_height);
    JPEG_PRINT("Max Width = %d and Max Height = %d\n", MAX_JPEG_WIDTH_HD,MAX_JPEG_HEIGHT_HD);

    //Set the Scale down variable
    if(pic_width > MAX_JPEG_WIDTH_HD*4 || pic_height>MAX_JPEG_HEIGHT_HD*4)
    {
        RatioVal = RATIO_EIGHTH;
        _u8DownScaleRatio = JPD_DOWNSCALE_EIGHTH;
        Factor = 8;
        _u16Image_x_size = pic_width/Factor;
        _u16Image_y_size = pic_height/Factor;
        JPEG_PRINT("down scale 1/8!!\n");
    }
    else if(pic_width > MAX_JPEG_WIDTH_HD*2 || pic_height>MAX_JPEG_HEIGHT_HD*2)
    {
        RatioVal = RATIO_FOURTH;
        _u8DownScaleRatio = JPD_DOWNSCALE_FOURTH;
        Factor = 4;
        _u16Image_x_size = pic_width/Factor;
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
    if(_bProgressive_flag==FALSE)
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

    if( ((U32)_pu8In_buf_ofs)  > (MRC_BUFFER_ADDR +MRC_BUFFER_SIZE/2) && _u8FileReadEnd_flag==FALSE )
    {
        volatile  U8* tmp;
        tmp = (volatile  U8*)(MRC_BUFFER_ADDR);
        tmp[0]=0x55;
        tmp[1]=0xAA;
        tmp[2]=0xAA;
        tmp[3]=0x54;
        MDrv_JPEG_Wait();
        u8PreloadLHFlag = E_JPEG_BUFFER_LOW;
    }

    // mark low/high buffer valid
    JPD_REG(REG_JPD_MCONFIG) = JPD_H_VLD |JPD_L_VLD;
    //MDrv_Write2Byte( REG_JPD_MCONFIG, JPD_H_VLD |JPD_L_VLD);

    // enable JPD decoding
    JPD_REG(REG_JPD_SCONFIG) = reg_value | JPD_DEC_EN | JPD_SWRST_S4L;
    //MDrv_Write2Byte( REG_JPD_SCONFIG, reg_value | JPD_DEC_EN | JPD_SWRST_S4L);

    //JPEG_DEBUG("After SCONFIG START\n");
}


//------------------------------------------------------------------------------
// Starts a new scan.
static BOOL MDrv_JPEG_init_scan( void )
{

    if ( !MDrv_JPEG_locate_sos_marker() )
    {
        return FALSE;
    }

    MDrv_JPEG_calc_mcu_block_order();

    if( !MDrv_JPEG_check_huff_tables() )
        return FALSE;

    if( !MDrv_JPEG_check_quant_tables() )
        return FALSE;

    #if SUPPORT_PROGRESSIVE_MODE
    memset( _u32Last_dc_val, 0, _u8Comps_in_frame * sizeof( U32 ) );

    _u32EOB_run = 0;
    #endif

    if ( _u16Restart_interval )
    {
        _u16Restarts_left = _u16Restart_interval;
        _u16Next_restart_num = 0;
    }

    _Total_Decoded_Size = (S32)MDrv_JPEG_GetECS();

    JPEG_DEBUG("ECS %08x\n", MDrv_JPEG_GetECS());

    if(_bProgressive_flag)
    {
        // pre-fill bit buffer for later decoding
        _s16Bits_left = 16;
        MDrv_JPEG_get_bits_2( 16 );
        MDrv_JPEG_get_bits_2( 16 );
    }

    return TRUE;
}


//------------------------------------------------------------------------------
// Starts a frame. Determines if the number of components or sampling factors
// are supported.
static BOOL MDrv_JPEG_init_frame( void ) //ok
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
            MDrv_JPEG_terminate( JPEG_UNSUPPORTED_SAMP_FACTORS );
            return FALSE;
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
            MDrv_JPEG_terminate( JPEG_UNSUPPORTED_SAMP_FACTORS );
            return FALSE;
        }
    }
    else
    {
        MDrv_JPEG_terminate( JPEG_UNSUPPORTED_COLORSPACE );
        return FALSE;
    }

    gu16Max_mcus_per_row = ( _u16Image_x_size + ( gu8Max_mcu_x_size - 1 ) ) / gu8Max_mcu_x_size;
    _u16Max_mcus_per_col = ( _u16Image_y_size + ( gu8Max_mcu_y_size - 1 ) ) / gu8Max_mcu_y_size;

    _u32Max_blocks_per_row = gu16Max_mcus_per_row * _u16Max_blocks_per_mcu;

    // Should never happen
    if ( _u32Max_blocks_per_row > JPEG_MAXBLOCKSPERROW )
    {
        MDrv_JPEG_terminate( JPEG_ASSERTION_ERROR );
        return FALSE;
    }

    #if SUPPORT_PROGRESSIVE_MODE
    _u16Total_lines_left = _u16Max_mcus_per_col * gu8Max_mcu_y_size;
    gu16Mcu_lines_left = 0;
    #endif
    return TRUE;
}
//------------------------------------------------------------------------------


#if SUPPORT_PROGRESSIVE_MODE
/******************************************************************************/
///Start Progressive JPEG decode for JPD
/******************************************************************************/
S16 MDrv_JPEG_Progressive_Decode(void)
{
    if( _bProgressive_flag==FALSE )
        return (JPEG_FAILED);

    if( _u16Total_lines_left == 0 )
    {
        return ( JPEG_DONE );
    }

//    if ( setjmp( _jmp_state ) )
//    {
//        return ( JPEG_FAILED );
//    }

#if SUPPORT_PROGRESSIVE_SCLAEDOWN_MODE
    JPEG_PRINT("_u16Mcus_per_row is %d!\n",_u16Mcus_per_row);
    JPEG_PRINT("_u16Mcus_per_col is %d!\n",_u16Mcus_per_col);
    JPEG_PRINT("_u8Blocks_per_mcu is %d!\n",_u8Blocks_per_mcu);
    JPEG_PRINT("gu8Max_mcu_x_size is %d!\n",gu8Max_mcu_x_size);
    JPEG_PRINT("gu8Max_mcu_y_size is %d!\n",gu8Max_mcu_y_size);
#endif
   while( _u16Total_lines_left > 0 )
    {
        MDrv_JPEG_load_next_row();
        _u16Total_lines_left -= gu8Max_mcu_y_size;
    }

    return (JPEG_OKAY);
}


//------------------------------------------------------------------------------
// Loads and dequantizes the next row of (already decoded) coefficients.
// Progressive images only.
void MDrv_JPEG_load_next_row( void )
{
    BLOCK_TYPE p[64];

    U16 mcu_row, mcu_block;
    U8 component_num, component_id;
    U16 block_x_mcu[JPEG_MAXCOMPONENTS];
    BOOL EOF_Flag = FALSE;

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
                JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
                continue;
            }
            pDC = MDrv_JPEG_coeff_buf_getp( _DC_Coeffs[component_id], block_x_mcu[component_id] + block_x_mcu_ofs, _u32Block_y_mcu[component_id] + block_y_mcu_ofs );
            if(pDC == NULL)
            {
                JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
                continue;
            }
            p[0] = pDC[0];
            memcpy( &p[1], &pAC[1], 63 * sizeof( BLOCK_TYPE ) );

            if( _Progressive_ROI_flag == FALSE )
            {
                if( mcu_block==(_u8Blocks_per_mcu -1)&&mcu_row==(_u16Mcus_per_row - 1)&&(_u16Total_lines_left - gu8Max_mcu_y_size)==0)
                    MDrv_JPEG_do_RLE(p, TRUE, component_id,TRUE);   // means it is end of picture
                else
                    MDrv_JPEG_do_RLE(p, FALSE, component_id,TRUE);
            }
            else
            {
                if( _u16Total_lines_left == gu8Max_mcu_y_size )//Last Line
                {
                    JPEG_PRINT("_u16Total_lines_left ==%d,%d,%d \n",gu8Max_mcu_y_size,mcu_block,mcu_row);

                    if( (mcu_block == (_u8Blocks_per_mcu -1) )&& ((mcu_row+2)*gu8Max_mcu_x_size >  ROI_width) )//Last line last block within rang
                        {
                            if( EOF_Flag == FALSE )
                            {
                            EOF_Flag = TRUE;
                            JPEG_PRINT("Run EOF Here \n");
                            MDrv_JPEG_do_RLE(p, TRUE, component_id,TRUE);   // means it is end of picture
                                }
                        }
                    else
                        {
                         MDrv_JPEG_do_RLE(p, FALSE, component_id,TRUE);
                        }
                }
                else
                {
                   if( (mcu_row+1)*gu8Max_mcu_x_size >  ROI_width)//ever line out rang block
                    {
                    //MDrv_JPEG_do_RLE(p, FALSE, component_id,FALSE);
                    }
                    else
                        MDrv_JPEG_do_RLE(p, FALSE, component_id,TRUE);
                 }
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


//------------------------------------------------------------------------------
// Restart interval processing.
static BOOL MDrv_JPEG_process_restart( void )
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
        MDrv_JPEG_terminate( JPEG_BAD_RESTART_MARKER );
        return FALSE;
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
        MDrv_JPEG_terminate( JPEG_BAD_RESTART_MARKER );
        return FALSE;
    }

    // Is it the expected marker? If not, something bad happened.
    if ( c != ( _u16Next_restart_num + M_RST0 ) )
    {
        MDrv_JPEG_terminate( JPEG_BAD_RESTART_MARKER );
        return FALSE;
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
    return TRUE;
}


//------------------------------------------------------------------------------
// The following methods decode the various types of blocks encountered
// in progressively encoded images.
//void decode_block_dc_first(
static BOOL MDrv_JPEG_decode_block_dc_first( //JPEG_DECODER *Pd,
  U8 component_id, U16 block_x, U16 block_y )
{
    S32 s, r;
    BLOCK_TYPE *p = MDrv_JPEG_coeff_buf_getp( _DC_Coeffs[component_id], block_x, block_y );

    if(p == NULL)
    {
        JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    if ( ( s = MDrv_JPEG_huff_decode( &_Huff_tbls[_u8Comp_dc_tab[component_id]] ) ) != 0 )
    {
        r = MDrv_JPEG_get_bits_2( s );
        s = HUFF_EXTEND_P( r, s );
    }

    // In JPD mode, the DC coefficient is the difference of nearest DC
    _u32Last_dc_val[component_id] = ( s += _u32Last_dc_val[component_id] );

    p[0] = s << _u8Successive_low;
    return TRUE;
}


//------------------------------------------------------------------------------
///void decode_block_dc_refine(
static BOOL MDrv_JPEG_decode_block_dc_refine( //JPEG_DECODER *Pd,
  U8 component_id, U16 block_x, U16 block_y )
{
    if ( MDrv_JPEG_get_bits_2( 1 ) )
    {
        BLOCK_TYPE *p = MDrv_JPEG_coeff_buf_getp( _DC_Coeffs[component_id], block_x, block_y );

        if(p == NULL)
        {
            JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
        p[0] |= ( 1 << _u8Successive_low );
    }
    return TRUE;
}


//------------------------------------------------------------------------------
///void decode_block_ac_first(
static BOOL MDrv_JPEG_decode_block_ac_first( //JPEG_DECODER *Pd,
  U8 component_id, U16 block_x, U16 block_y )
{
    BLOCK_TYPE *p;
    S32 k, s, r;

    if ( _u32EOB_run )
    {
        _u32EOB_run--;
        return TRUE;
    }

    p = MDrv_JPEG_coeff_buf_getp( _AC_Coeffs[component_id], block_x, block_y );

    if(p == NULL)
    {
        JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
        return FALSE;
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
                MDrv_JPEG_terminate( JPEG_DECODE_ERROR );
                return FALSE;
            }

            r = MDrv_JPEG_get_bits_2( s );
            s = HUFF_EXTEND_P( r, s );

            // No need to do ZAG order in JPD mode
            #ifdef CMODEL
            //p[_u8ZAG[k]] = s << _u8Successive_low;
            p[k] = s << _u8Successive_low;
            #else
            p[k] = s << _u8Successive_low;
            #endif
        }
        else
        {
            if ( r == 15 )
            {
                if ( ( k += 15 ) > 63 )
                {
                    MDrv_JPEG_terminate( JPEG_DECODE_ERROR );
                    return FALSE;
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
    return TRUE;
}


//------------------------------------------------------------------------------
///void decode_block_ac_refine(
static BOOL MDrv_JPEG_decode_block_ac_refine( //JPEG_DECODER *Pd,
  U8 component_id, U16 block_x, U16 block_y )
{
    S32 s, k, r;
    S32 p1 = 1 << _u8Successive_low;
    S32 m1 = ( -1 ) << _u8Successive_low;
    BLOCK_TYPE *p = MDrv_JPEG_coeff_buf_getp( _AC_Coeffs[component_id], block_x, block_y );

    if(p == NULL)
    {
        JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
        return FALSE;
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
                    MDrv_JPEG_terminate( JPEG_DECODE_ERROR );
                    return FALSE;
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
                #ifdef CMODEL
                //BLOCK_TYPE *this_coef = p + _u8ZAG[k];
                BLOCK_TYPE *this_coef = p + k;
                #else
                BLOCK_TYPE *this_coef = p + k;
                #endif

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
                #ifdef CMODEL
                //p[_u8ZAG[k]] = s;
                p[k] = s;
                #else
                p[k] = s;
                #endif
            }
        }
    }

    if ( _u32EOB_run > 0 )
    {
        for ( ; k <= _u8Spectral_end; k++ )
        {
            // No need to do ZAG order in JPD mode
            #ifdef CMODEL
            //BLOCK_TYPE *this_coef = p + _u8ZAG[k];
            BLOCK_TYPE *this_coef = p + k;
            #else
            BLOCK_TYPE *this_coef = p + k;
            #endif

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
    return TRUE;
}


//------------------------------------------------------------------------------
// Decode a scan in a progressively encoded image.
static BOOL MDrv_JPEG_decode_scan( Pdecode_block_func decode_block_func )
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
                    return FALSE;
            }

            for ( mcu_block = 0; mcu_block < _u8Blocks_per_mcu; mcu_block++ )
            {
                component_id = _u8Mcu_org[mcu_block];

                if(!decode_block_func( component_id, block_x_mcu[component_id] + block_x_mcu_ofs, block_y_mcu[component_id] + block_y_mcu_ofs ))
                {
                    JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
                    return FALSE;
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
    return TRUE;
}


//------------------------------------------------------------------------------
// Decode a progressively encoded image.
static BOOL MDrv_JPEG_init_progressive( void )
{
    U8 i;

    if ( _u8Comps_in_frame == 4 )
    {
        MDrv_JPEG_terminate( JPEG_UNSUPPORTED_COLORSPACE );
        return FALSE;
    }

    // Allocate the coefficient buffers.
    for ( i = 0; i < _u8Comps_in_frame; i++ )
    {
        _DC_Coeffs[i] = MDrv_JPEG_coeff_buf_open( ((gu16Max_mcus_per_row+0x1)& ~0x1) * _u8Comp_h_samp[i], ((_u16Max_mcus_per_col+0x1)& ~0x1) * _u8Comp_v_samp[i], 1, 1 );
        _AC_Coeffs[i] = MDrv_JPEG_coeff_buf_open( ((gu16Max_mcus_per_row+0x1)& ~0x1) * _u8Comp_h_samp[i], ((_u16Max_mcus_per_col+0x1)& ~0x1) * _u8Comp_v_samp[i], 8, 8 );
        if(_DC_Coeffs[i]== NULL || _AC_Coeffs[i] == NULL)
        {
            JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
            MDrv_JPEG_Finalize();
            return FALSE;
        }
    }

    for ( ; ; )
    {
        BOOL dc_only_scan, refinement_scan;
        Pdecode_block_func decode_block_func;

        if ( !MDrv_JPEG_init_scan() )
        {
            break;
        }

        dc_only_scan = ( _u8Spectral_start == 0 );
        refinement_scan = ( _u8Successive_high != 0 );

        if ( ( _u8Spectral_start > _u8Spectral_end ) || ( _u8Spectral_end > 63 ) )
        {
            MDrv_JPEG_terminate( JPEG_BAD_SOS_SPECTRAL );
            return FALSE;
        }

        if ( dc_only_scan )
        {
            if ( _u8Spectral_end )
            {
                MDrv_JPEG_terminate( JPEG_BAD_SOS_SPECTRAL );
                return FALSE;
            }
        }
        else if ( _u8Comps_in_scan != 1 )  /* AC scans can only contain one component */
        {
            MDrv_JPEG_terminate( JPEG_BAD_SOS_SPECTRAL );
            return FALSE;
        }

        if ( ( refinement_scan ) && ( _u8Successive_low != _u8Successive_high - 1 ) )
        {
            MDrv_JPEG_terminate( JPEG_BAD_SOS_SUCCESSIVE );
            return FALSE;
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
            return FALSE;

    }

    _u8Comps_in_scan = _u8Comps_in_frame;

    for ( i = 0; i < _u8Comps_in_frame; i++ )
    {
        _u8Comp_list[i] = i;
    }

    MDrv_JPEG_calc_mcu_block_order();
    return TRUE;
}
#endif


//------------------------------------------------------------------------------
static BOOL MDrv_JPEG_init_sequential( void )
{
    if ( !MDrv_JPEG_init_scan() )
    {
        JPEG_DEBUG("MDrv_JPEG_init_scan - JPEG_UNEXPECTED_MARKER\n");
        MDrv_JPEG_terminate( JPEG_UNEXPECTED_MARKER );
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
// Find the start of the JPEG file and reads enough data to determine
// its size, number of components, etc.
//static void MDrv_JPEG_decode_init( PJPEG_DECODER_FILE_SYSTEM Pstream)
static BOOL MDrv_JPEG_decode_init( void )
{
    U16 u16ImageXSize, u16ImageYSize;
#if SUPPORT_PROGRESSIVE_SCLAEDOWN_MODE
    U16 mcu_width=0,mcu_height=0;
#endif
    MDrv_JPEG_init();
    //MDrv_JPEG_init(Pstream);
    if( !MDrv_JPEG_locate_sof_marker() )
        return FALSE;

    // save the original image size
    _u16OriginalImage_x_size = u16ImageXSize = _u16Image_x_size;
    _u16OriginalImage_y_size = u16ImageYSize = _u16Image_y_size;

    _Progressive_ROI_flag = FALSE;

    if(_bProgressive_flag)
    {
#if (!SUPPORT_PROGRESSIVE_SCLAEDOWN_MODE)
        if( _u16Image_x_size > MAX_JPEG_PROGRESSIVE_WIDTH )
        {
            JPEG_DEBUG("Progressive Width size too big... do not handle it\n");
            return FALSE;
        }
        if( _u16Image_y_size > MAX_JPEG_PROGRESSIVE_HEIGHT )
        {
            JPEG_DEBUG("Progressive Height size too big... do not handle it\n");
            return FALSE;
        }
#else
    if( _u16Image_x_size > MAX_JPEG_PROGRESSIVE_WIDTH*8 )
    {
        JPEG_DEBUG("Progressive Width size too big... do not handle it\n");
        return FALSE;
    }
    if( _u16Image_y_size > MAX_JPEG_PROGRESSIVE_HEIGHT*8 )
    {
        JPEG_DEBUG("Progressive Height size too big... do not handle it\n");
        return FALSE;
    }
    if (_u16Image_x_size > MAX_JPEG_PROGRESSIVE_WIDTH || _u16Image_y_size>MAX_JPEG_PROGRESSIVE_HEIGHT)
    {
        mcu_width = (_u16Image_x_size % ( _u8Comp_h_samp[0] * 8 ));
        JPEG_DEBUG("mcu_width = %d ,_u8Comp_h_samp[0] = %d\n",mcu_width,_u8Comp_h_samp[0]);

        if( mcu_width != 0)
            mcu_width = ( _u16Image_x_size + ( _u8Comp_h_samp[0] * 8 - (_u16Image_x_size % ( _u8Comp_h_samp[0] * 8 )) ) );
        else
             mcu_width = _u16Image_x_size;
        JPEG_DEBUG("mcu_width = %d \n",mcu_width);

        mcu_height = (_u16Image_y_size % ( _u8Comp_v_samp[0] * 8 ));
        JPEG_DEBUG("mcu_height = %d , _u8Comp_v_samp[0]  = %d\n",mcu_height, _u8Comp_v_samp[0] );

        if( mcu_height != 0)
            mcu_height = ( _u16Image_y_size + ( _u8Comp_v_samp[0] * 8 - (_u16Image_y_size % ( _u8Comp_v_samp[0] * 8 )) ) );
        else
            mcu_height = _u16Image_y_size;
        JPEG_DEBUG("mcu_height = %d \n",mcu_height);

        if(mcu_width > MAX_JPEG_PROGRESSIVE_WIDTH*4 || mcu_height>MAX_JPEG_PROGRESSIVE_WIDTH*4)
        {
            ROI_width= mcu_width/64*64;
        }
        else if(mcu_width > MAX_JPEG_PROGRESSIVE_WIDTH*2 || mcu_height>MAX_JPEG_PROGRESSIVE_WIDTH*2)
        {
            ROI_width = mcu_width/32*32;
        }
        else
             ROI_width = mcu_width/16*16;
        JPEG_DEBUG("ROI_width = %d \n",ROI_width);

        if(   mcu_width != ROI_width )
        {
            _Progressive_ROI_flag = TRUE;
        }
        else
        {
            _Progressive_ROI_flag = FALSE;
        }
    }
    else
    {
        _Progressive_ROI_flag = FALSE;
    }
#endif
        if( _u16Image_x_size*_u16Image_y_size*3*2 > INTERNAL_BUFFER_SIZE )
        {
            JPEG_DEBUG("Progressive image size too big... do not handle it\n");
            return FALSE;
        }
    }

    if(_u8DecodeType == JPEG_TYPE_THUMBNAIL)
    {
        if(_bThumbnailFound)
        {
            _bThumbnailAccessMode = TRUE;
            MDrv_JPEG_Finalize();
            MDrv_JPEG_init();
            //MDrv_JPEG_init( Pstream );

            // save the original image size, because MDrv_JPEG_init will reset all variables to 0
            _u16OriginalImage_x_size = u16ImageXSize;
            _u16OriginalImage_y_size = u16ImageYSize;

            if( !MDrv_JPEG_locate_sof_marker() )
                return FALSE;

            if(_bProgressive_flag)
            {
                JPEG_DEBUG("Progressive image in thumbnail... do not handle it\n");
                MDrv_JPEG_terminate( JPEG_BAD_APP1_MARKER );
                return FALSE;
            }
        }
        else
        {
            //MDrv_JPEG_terminate( JPEG_NO_THUMBNAIL );
            JPEG_PRINT("NO Thumbnail Found\n");
            return TRUE;
        }
    }
    return TRUE;
}


//------------------------------------------------------------------------------
/******************************************************************************/
///Set read/write buffer for JPEG decoding
///@param u32RBufAddr \b IN buffer address for reading
///@param u32RBufSize \b IN read buffer size
///@param u32WBufAddr \b IN buffer address for writing JPEG decompress data (YUV422)
///#param u32IBufAddr \b IN internal buffer address for JPEG decoding system (exif:64K + memory pool)
///#param u32IBufSize \b IN internal buffer size for JPEG decoding system
/******************************************************************************/
#define MIN_READBUFFER_SIZE 128
BOOL MDrv_JPEG_SetInitParameter(U32 u32RBufAddr, U32 u32RBufSize, U32 u32WBufAddr, U32 u32IBufAddr, U32 u32IBufSize, U8 u8FileEnd)
{
    // the buffer size must be multiple of 4 bytes
    if( u32RBufSize < MIN_READBUFFER_SIZE )
    {
        MDrv_JPEG_terminate(JPEG_READBUFFER_TOOSMALL);
        return FALSE;
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

    MDrv_JPEG_init_mempool(((U8 *) (u32IBufAddr + JPEG_DEFAULT_EXIF_SIZE)), u32IBufSize - JPEG_DEFAULT_EXIF_SIZE);

    return TRUE;
}


//------------------------------------------------------------------------------
// Call get_error_code() after constructing to determine if the stream
// was valid or not. You may call the get_width(), get_height(), etc.
// methods after the constructor is called.
// You may then either destruct the object, or begin decoding the image
// by calling begin(), then decode().
BOOL MDrv_JPEG_constructor( U8 decode_type )
{
//    if ( setjmp( _jmp_state ) )
//    {
//        return;
//    }

    _u8DecodeType = decode_type;
    MDrv_JPEG_init_thumbnail();
    if( !MDrv_JPEG_decode_init() )
        return FALSE;

    if( (_u8DecodeType==JPEG_TYPE_THUMBNAIL) &&  !_bThumbnailFound)
    {
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
BOOL MDrv_JPEG_DecodeHeader(void)
{
//    if ( setjmp( _jmp_state ) )
//    {
//        return ( FALSE );
//    }

    if(!MDrv_JPEG_init_frame())
    {
        JPEG_PRINT("%s [%d]\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if ( _bProgressive_flag )
    {
        if(!MDrv_JPEG_init_progressive())
            return FALSE;
    }
    else
    {
        if(!MDrv_JPEG_init_sequential())
            return FALSE;
    }
    return TRUE;
}


// Completely destroys the decoder object. May be called at any time.
void MDrv_JPEG_Finalize( void )
{
    MDrv_JPEG_free_all_blocks();
}


//------------------------------------------------------------------------------
S16 MDrv_JPEG_Check_End_Flag( void )
{
    return ( _bEOF_flag );
}


S16 MDrv_JPEG_get_error_code( void )
{
    return ( _s16Error_code );
}


U16 MDrv_JPEG_get_width( void )
{
    return ( _u16Image_x_size );
}


U16 MDrv_JPEG_get_height( void )
{
    return ( _u16Image_y_size );
}


U16 MDrv_JPEG_get_original_width( void )
{
    return ( _u16OriginalImage_x_size );
}


U16 MDrv_JPEG_get_original_height( void )
{
    return ( _u16OriginalImage_y_size );
}


U8 MDrv_JPEG_get_precision( void )
{
    return ( _u8Precision );
}
#if 0
U8* MDrv_JPEG_get_CameraMake( void )
{
    return ( _pu8CameraMake );
}

U8* MDrv_JPEG_get_CameraMode( void )
{
    return ( _pu8CameraMode );
}

U8* MDrv_JPEG_get_CreationDate( void )
{
    return ( _pu8CreationDate );
}
#endif
U8 MDrv_JPEG_get_num_components( void )
{
    return ( _u8Comps_in_frame );
}


U32 MDrv_JPEG_get_total_bytes_read( void )
{
    return ( _u32Total_bytes_read );
}


BOOL MDrv_JPEG_IsProgressive(void)
{
    return _bProgressive_flag;
}


void MDrv_JPEG_DumpTables(void)
{
//#define JPD_MEM_SCWGIF_BASE     0x0000
//#define JPD_MEM_SYMIDX_BASE     0x0200
//#define JPD_MEM_QTBL_BASE       0x0400

    U16 j;

    JPD_REG(REG_JPD_TID_ADR) = 0x0000;
    for( j=0; j<0x500; j++ )
    {
        JPEG_PRINT("Tab[%X]= %X\n", j, JPD_REG(REG_JPD_TID_DAT));
    }

    return;
}

#if 0
void MDrv_JPEG_DumpTables(void)
{
    // Symbol tables
    {
        U16 i, tbl_num_luma, tbl_num_chroma;
        U8 ci, luma_ci = 0, chroma_ci = 0;

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

        JPEG_DEBUG("Symbol table\n");

        JPEG_DEBUG("%d %d\n", tbl_num_luma, tbl_num_chroma);
        JPEG_DEBUG("Write symidx : AC\n");
        for ( i = 0; i < 256; i++ )
        {
            JPEG_DEBUG("%04x\n", ( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] ));
        }


        tbl_num_luma = _u8Comp_dc_tab[luma_ci];
        tbl_num_chroma = _u8Comp_dc_tab[chroma_ci];

        JPEG_DEBUG("%d %d\n", tbl_num_luma, tbl_num_chroma);
        JPEG_DEBUG("Write symidx : DC\n");
        for ( i = 0; i < 16; i++ )
        {
            JPEG_DEBUG("%04x\n", ( _Huff_info[tbl_num_chroma].u8Huff_val[i] << 8 ) | ( _Huff_info[tbl_num_luma].u8Huff_val[i] ) );
        }
    }

    // Q tables
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

        JPEG_DEBUG("Q Table\n");
        JPEG_DEBUG("Component: %d\n", com_num);
        for ( i = 0; i < com_num; i++ )
        {
            JPEG_DEBUG("====\n");
            for(j = 0; j<64; j++)
            {
                JPEG_DEBUG("%04x\n", _QuantTables[_u8Comp_quant[i]].s16Value[_u8Jpeg_zigzag_order[j]]);
            }
        }
    }

    // SC tables
    {
        U32 reg_value;
        U16 i, ci, valid, tbl_num_luma, tbl_num_chroma;
        U8 luma_ci = 0, chroma_ci = 0;

        JPEG_DEBUG("sc table\n");

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

            JPEG_DEBUG("%08x\n", reg_value);
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

            JPEG_DEBUG("%08x\n", reg_value);
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

            JPEG_DEBUG("%08x\n", reg_value);
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

            JPEG_DEBUG("%08x\n", reg_value);
        }
    }
}
#endif

EN_JPD_STATUS MDrv_JPEG_WaitDone(void)
{
    U16 reg_val;

    reg_val = MHal_JPD_ReadJPDStatus();
    JPEG_PRINT("JPD Status= %X\n", reg_val);
/*
    //For H/W bug, some cases can not exit after decode done. Check Vidx to exit.
    if(PreVIdx != MDrv_ReadByte(BK_JPD_CUR_VIDX))
    {
        PreVIdx = MDrv_ReadByte(BK_JPD_CUR_VIDX);
        ReCheckTime = 0;
    }
    else
    {
        ReCheckTime++;
        if(ReCheckTime >= 1000)
        {
            reg_val = JPD_EVENT_DEC_DONE;
            JPEG_PRINT("Decode timeout!!!!\n");
        }
    }
*/
    if(reg_val & JPD_EVENT_DEC_DONE)
    {
        JPEG_PRINT("Decode Done\n");
        MDrv_JPEG_Finalize();
        //_Pinput_stream->destructor( _Pinput_stream );
        //g_state = e_DecodeDone;

        return EN_JPD_DECODE_DONE;
    }
    else if(reg_val & JPD_EVENT_ERROR_MASK)
    {
        // temp patch for protect JPD from writing to illegal memory
        MHal_JPD_SW_Pause_Reset();

        JPEG_PRINT("Baseline decode error: %x\n", reg_val);
        MDrv_JPEG_Finalize();
        //_Pinput_stream->destructor( _Pinput_stream );
        //g_state = e_DecodeErr;
        return EN_JPD_DECODE_ERROR;
    }


#if 1 //allen.chang 2009/12/30 for baseline ping pong buffer
    if((JPD_EVENT_MRBL_DONE & reg_val)
    && (E_JPEG_BUFFER_LOW != u8PreloadLHFlag))
    {
        u8PreloadLHFlag = E_JPEG_BUFFER_LOW;
        JPEG_PRINT("JPD_EVENT_MRBL_DONE\n");
        return EN_JPD_MRBFL_DONE;
    }
    else if((JPD_EVENT_MRBH_DONE & reg_val)
    && (E_JPEG_BUFFER_HIGH != u8PreloadLHFlag))
    {
        u8PreloadLHFlag = E_JPEG_BUFFER_HIGH;
        JPEG_PRINT("JPD_EVENT_MRBH_DONE\n");
        return EN_JPD_MRBFH_DONE;
    }
#else
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
#endif
    else
    {
        return EN_JPD_DECODING;
    }
}



B16 MDrv_JPEG_Wait(void)
{
    DEFINE_WAIT(waitentry_ttx);
    JPEG_PRINT("MDrv_JPEG_Wait\n");

    /* wait queue only accepts one thread to wait TTX */
    if(atomic_read(&_aWaitQueueGuard) == 0x00)
    {
        prepare_to_wait(&wq_ttx, &waitentry_ttx, TASK_INTERRUPTIBLE);
        atomic_set(&_aWaitQueueGuard, 0x01);
        schedule();
        finish_wait(&wq_ttx, &waitentry_ttx);
    }

    return TRUE;
}


void MDrv_JPEG_Wakeup(U32 bytes_read)
{

    _u32In_buf_left += bytes_read;
    //_Total_Decoded_Size += bytes_read;

    JPEG_PRINT("MDrv_JPEG_Wakeup %x\n",bytes_read);

    if(atomic_read(&_aWaitQueueGuard) == 0x01)
    {
        wake_up(&wq_ttx);
        atomic_set(&_aWaitQueueGuard, 0x00);
    }

}

void MDrv_JPEG_SetMaxSize(int progressive_width, int progressive_height, int baseline_width, int baseline_height)
{
    MAX_JPEG_PROGRESSIVE_WIDTH = progressive_width;
    MAX_JPEG_PROGRESSIVE_HEIGHT = progressive_height;
    MAX_JPEG_BASELINE_WIDTH = baseline_width;
    MAX_JPEG_BASELINE_HEIGHT = baseline_height;
}

#if 0 //MIU is configured in system layer
void MDrv_JPEG_SetMIU(int miu_no)
{
    MHal_JPD_SetMIU(miu_no);
}
#endif

void MDrv_JPEG_SetPowerOnOff(U8 on)
{
    MHal_JPD_SetClock(on);
}


#undef _MDRV_JPEG_DECODER_C

