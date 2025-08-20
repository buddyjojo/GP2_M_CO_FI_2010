///////////////////////////////////////////////////////////////////////////////
///@file Datatype.h
///@brief Datatype defination
///@author MStarSemi Inc.
///
/// Define datatype in this file.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef __DATATYPE_H
#define __DATATYPE_H

#include <stdio.h>

///////////////////////////////////////////////////////////////////
//     Char definition    : 2 bytes font talbe
///////////////////////////////////////////////////////////////////
#define CHAR_RETURNICON       0X0019
#define CHAR_ENTERICON        0X001A
#define CHAR_MOVEICON         0X001B
#define CHAR_UP              0x001C
#define CHAR_DOWN            0X001D
#define CHAR_LEFT            0X001E
#define CHAR_RIGHT           0X001F
#define CHAR_SPACE           0x0020
#define CHAR_PERCENT         0x0025
#define CHAR_APOSTROPHE     0x0027
#define CHAR_LEFT_QUOTE     0x0028
#define CHAR_RIGHT_QUOTE     0x0029
#define CHAR_STAR             0x002A
#define CHAR_PLUS            0x002B
#define CHAR_COMMA          0x002C
#define CHAR_MINUS           0x002D
#define CHAR_DOT             0x002E
#define CHAR_SLASH           0x002F
#define CHAR_0               0x0030
#define CHAR_1               0x0031
#define CHAR_2               0x0032
#define CHAR_3               0x0033
#define CHAR_4              0x0034
#define CHAR_5               0x0035
#define CHAR_6               0x0036
#define CHAR_7               0x0037
#define CHAR_8               0x0038
#define CHAR_9               0x0039
#define CHAR_COLON           0x003A
#define CHAR_GT              0x003E
#define CHAR_QM              0x003F
#define CHAR_A               0x0041
#define CHAR_B               0x0042
#define CHAR_C               0x0043
#define CHAR_D               0x0044
#define CHAR_E               0x0045
#define CHAR_F               0x0046
#define CHAR_G               0x0047
#define CHAR_H               0x0048
#define CHAR_I               0x0049
#define CHAR_J               0x004A
#define CHAR_K               0x004B
#define CHAR_L               0x004C
#define CHAR_M               0x004D
#define CHAR_N               0x004E
#define CHAR_O               0x004F
#define CHAR_P               0x0050
#define CHAR_Q               0x0051
#define CHAR_R               0x0052
#define CHAR_S               0x0053
#define CHAR_T               0x0054
#define CHAR_U               0x0055
#define CHAR_V               0x0056
#define CHAR_W               0x0057
#define CHAR_X               0x0058
#define CHAR_Y               0x0059
#define CHAR_Z               0x005A
#define CHAR_US              0x005f
#define CHAR_a               0x0061
#define CHAR_b               0x0062
#define CHAR_c               0x0063
#define CHAR_d               0x0064
#define CHAR_e               0x0065
#define CHAR_f               0x0066
#define CHAR_g               0x0067
#define CHAR_h               0x0068
#define CHAR_i               0x0069
#define CHAR_j               0x006A
#define CHAR_k               0x006B
#define CHAR_l               0x006C
#define CHAR_m               0x006D
#define CHAR_n               0x006E
#define CHAR_o               0x006F
#define CHAR_p              0x0070
#define CHAR_q               0x0071
#define CHAR_r               0x0072
#define CHAR_s               0x0073
#define CHAR_t               0x0074
#define CHAR_u               0x0075
#define CHAR_v               0x0076
#define CHAR_w               0x0077
#define CHAR_x               0x0078
#define CHAR_y               0x0079
#define CHAR_z               0x007A
#define CHAR_CC               0x010D
#define CHAR_SIGNAL        0x0121
//extended ASCII
#define CHAR_EXT_C         0x00C7
#define CHAR_EXT_c         0x00E7
#define CHAR_EXT_e         0x00E9
#define CHAR_EXT_e2        0x00E8
#define CHAR_EXT_e3       0x00EA
#define CHAR_EXT_e4       0x00EB
#define CHAR_EXT_E2        0x00C8
#define CHAR_EXT_E3        0x00C9
#define CHAR_EXT_O         0x00D3
#define CHAR_EXT_a         0x00E1
#define CHAR_EXT_a1       0x00E0
#define CHAR_EXT_a2       0x00E2
#define CHAR_EXT_A         0x00C1
#define CHAR_EXT_u         0x00FA
#define CHAR_EXT_U         0x00DA
#define CHAR_EXT_n         0x00F1
#define CHAR_EXT_N         0x00D1
#define CHAR_EXT_I         0x00CD
#define CHAR_EXT_i         0x00ED
#define CHAR_EXT_i2        0x00EE
#define CHAR_EXT_o         0x00F3

#undef NULL
#define NULL 0

/********************************************************************************/
/* Primitive types                                                              */
/********************************************************************************/

#if defined (__C51__)

    #define WORDS_BIGENDIAN

    #define register  data
    #define XBYTE      ((unsigned char volatile xdata *) 0)

    typedef bit            BIT;

    typedef unsigned char  U8;
    typedef unsigned int   U16;
    typedef unsigned long  U32;
    typedef signed char    S8;
    typedef signed int     S16;
    typedef signed long    S32;

    typedef U8   FAST_U8;
    typedef U16  FAST_U16;
    typedef U32  FAST_U32;
    typedef S8   FAST_S8;
    typedef S16  FAST_S16;
    typedef S32  FAST_S32;

    #ifdef _SW_OS_TEST
        typedef unsigned char  INT8U;                /* Unsigned  8 bit quantity                           */
        typedef unsigned int   INT16U;               /* Unsigned 16 bit quantity                           */
        typedef unsigned long  INT32U;               /* Unsigned 32 bit quantity                           */
        typedef unsigned char  OS_STK;               /* size of unit in stack                              */
    #endif

#elif defined (__OR1K__)

    #define WORDS_BIGENDIAN

    typedef unsigned char  BIT;

    typedef unsigned char  U8;
    typedef unsigned short U16;
    typedef unsigned long  U32;
    typedef signed char    S8;
    typedef signed short   S16;
    typedef signed long    S32;

    typedef U32  FAST_U8;
    typedef U32  FAST_U16;
    typedef U32  FAST_U32;
    typedef S32  FAST_S8;
    typedef S32  FAST_S16;
    typedef S32  FAST_S32;

#else

    #error "primitive types are not defined"

#endif

typedef FAST_U8  BOOLEAN;   ///< BOOLEAN

/********************************************************************************/
/* Macro for endianess                                                          */
/********************************************************************************/

#define ReadU16BE(b)  (((b)[0]<<8)|((b)[1]))
#define ReadU32BE(b)  (((b)[0]<<24)|((b)[1]<<16)|((b)[2]<<8)|((b)[3]))
#define ReadU16LE(b)  (((b)[1]<<8)|((b)[0]))
#define ReadU32LE(b)  (((b)[3]<<24)|((b)[2]<<16)|((b)[1]<<8)|((b)[0]))

#define ByteSwap16(x) (((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8)
#define ByteSwap32(x) \
    ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
     (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#ifdef WORDS_BIGENDIAN
    #define BE2ME16(x)  (x)
    #define BE2ME32(x)  (x)
    #define LE2ME16(x)  ByteSwap16(x)
    #define LE2ME32(x)  ByteSwap32(x)

    #define VARBYTE(var, n)    (((U8 *)&(var))[n])
#else
    #define BE2ME16(x)  ByteSwap16(x)
    #define BE2ME32(x)  ByteSwap32(x)
    #define LE2ME16(x)  (x)
    #define LE2ME32(x)  (x)

    #define VARBYTE(var, n)    (((U8 *)&(var))[sizeof(var) - n - 1])
#endif


/********************************************************************************/
/* Macro for TDA1236D                                                             */
/********************************************************************************/
#define Data8    U8
#define Data16    S16
#define Data32    S32
#define bool    BOOLEAN
#define Bool    BOOLEAN
#define true     TRUE
#define false     FALSE

/******************************************************************************/
/* Macro for bitwise                                                          */
/******************************************************************************/
#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)

#define BITFLAG(loc) (1U << (loc))

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define SETBIT(REG, BIT)   ((REG) |=  (0x01 << (BIT)))
#define CLRBIT(REG, BIT)   ((REG) &= ~(0x01 << (BIT)))
#define GETBIT(REG, BIT)   (((REG) >> (BIT)) & 0x01)
#define COMPLEMENT(a)      (~(a))


/// union for long16 (16 bits/ 2 bytes)
typedef union LONG16_BYTE_t
{
    U16 u16Num;     ///< 16bits
    U8 u8Num[2];    ///< 8 bits x 2
    // u8Num[0]  MSB
    // u8Num[1]  LSB
} LONG16_BYTE;

/// union for long32 (32 bits/ 4 bytes)
typedef union LONG32_BYTE_t
{
    U32 u32Num;     ///< 32bits
    U8 u8Num[4];    ///< 8 bits x4
    // u8Num[0]  MSB
    // u8Num[1]
    // u8Num[2]
    // u8Num[3]  LSB
} LONG32_BYTE;

/// union for float (32 bits/ 4 bytes)
typedef union FLOAT_BYTE_T
{
    float fvalue;   ///< 32bits float
    U8 u8Num[4];///< 8 bits x 4
} FLOAT_BYTE;

/// define MSRET U8
typedef U8 MSRET;
/// MS Return Error
#define MSRET_ERROR     0
/// MS Return OK
#define MSRET_OK        1

/// defination for FALSE
#define FALSE           0
/// defination for TRUE
#define TRUE            1

#define DISABLE         0
#define ENABLE          1

/// 0: FAIL
#define FAIL            0
/// 1: PASS
#define PASS            1

/// 0: NO
#define NO              0
/// 1: YES
#define YES             1

#define FE_NOT_LOCK     0
#define FE_LOCK         1
#define FE_AGC_NOT_LOCK 2


#define NONE_HIGHLIGHT  0
#define HIGHLIGHT       1

#define HIU16(u32)    ((U16)(u32>>16))    // SATURN2
#define LOU16(u32)    ((U16)(u32))        // SATURN2


#define LOWBYTE(u16)    ((U8)(u16))
#define HIGHBYTE(u16)   ((U8)((u16) >> 8))
#define HINIBBLE(u8)    ((u8) / 0x10)
#define LONIBBLE(u8)    ((u8) & 0x0F)
#define BCD2Dec(x)      ((((x) >> 4) * 10) + ((x) & 0x0F))


/// Font handle, handle to font table in memory
typedef S8 FONTHANDLE;
/// Bitmap handle, handle to bitmap buffer
typedef S16 BMPHANDLE;
typedef S8 DBHANDLE;

#define INVALID_FONTHANDLE  -1
#define INVALID_BMPHANDLE   -1
#define INVALID_DBHANDLE    -1

// NOTE. these have problem with long integer (32-bit)
#define MAX( a, b )         (((a) > (b)) ? (a) : (b))
#define MIN( a ,b )         (((a) < (b)) ? (a) : (b))

// C pre-processor can not recognize tripple operator.
// We have another version MIN/MAX macro for C pre-precessor.
// use these in C code may generate wrose code
#define _MAX( a, b )        (((a) >= (b)) * (a) + ((b) > (a)) * (b))
#define _MIN( a, b )        (((a) <= (b)) * (a) + ((b) < (a)) * (b))

#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )

#define COUNTOF( array )    (sizeof(array) / sizeof((array)[0]))

#define UNUSED( var )       (void)((var) = (var))

#endif /* __DATATYPE_H */
