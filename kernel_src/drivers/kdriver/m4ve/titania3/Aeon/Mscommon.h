#ifndef _COMMON_TYPE_H_
#define _COMMON_TYPE_H_

#include "M4VE_chip.h"
//#define _BCB_PLATFORM_
//#define _WRITE_UDMA_
#ifdef _WIN32
#include <windows.h>
#endif
#if defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_)
#include <linux/kernel.h>
#else
#include <stdio.h>
#include <assert.h>
#ifdef _M4VE_OLYMPUS_
#include <typedef.h>
#else
#include <time.h>
#endif
#endif

#ifndef MAX
#define  MAX(a,b)              (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define  MIN(a,b)              (((a) < (b)) ? (a) : (b))
#endif

#define MUX(a, b, c)    ((a) ? (b) : (c))
#ifndef _WIN32
#define LOWORD(l)        ((unsigned short)(l))
#define HIWORD(l)        ((unsigned short)(((unsigned int)(l) >> 16) & 0xFFFF))
#define MAKELONG(a, b)    ((unsigned int)(((unsigned short)(a)) | ((unsigned int)((unsigned short)(b))) << 16))
#endif
#define dump(x) //for rate control debug

#if (!defined(_M4VE_OLYMPUS_)&&!defined(_M4VE_T3_)) || defined(_WIN32)
typedef unsigned char u8;
typedef unsigned long u32;
typedef long i32;

#if (!defined(_AEON_PLATFORM_)&&!defined(_MIPS_PLATFORM_)) || defined(_WIN32)
typedef unsigned char     U8;
typedef unsigned short     U16;
typedef unsigned long     U32;
typedef long              S32;
#endif
#endif //_M4VE_OLYMPUS_

#if !defined(_AEON_PLATFORM_) && !defined(_M4VE_OLYMPUS_) && !defined(_MIPS_PLATFORM_)
#define MsOS_GetSystemTick() clock()
#elif !defined(_M4VE_TRITON_)
#define MsOS_GetSystemTick()
#endif

#if defined(ASSERT)
#undef ASSERT
#endif
#if defined(_M4VE_OLYMPUS_) || (defined(_M4VE_T3_)&&defined(_MIPS_PLATFORM_))
#define ASSERT(x)
#else
#define ASSERT(x) assert(x)
#endif

#ifdef _WIN32//_BCB_PLATFORM_

#include <memory.h>

#ifndef INT64
#if __STDC_VERSION__ >= 199901L
typedef long long INT64;
#else
typedef __int64 INT64;
#endif
#endif

#else //WIN32

//#define _NO_FLOATING_
//#define FLOAT   int
//#define DOUBLE  int//double
#ifndef INT64
typedef long long INT64;
#endif
#if defined(_M4VE_T3_) && defined(_AEON_PLATFORM_)
#else
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
#endif
#if !defined(_AEON_PLATFORM_) && !defined(_MIPS_PLATFORM_)
typedef long            BOOL;
#endif

#if !defined(FALSE)
#define FALSE   0
#endif
#if !defined(TRUE)
#define TRUE    1
#endif
#define Sleep(x) \
{ int sleep_i=0;\
    do { \
        sleep_i++; \
    } while(sleep_i<x); \
}

#endif //WIN32

#endif
