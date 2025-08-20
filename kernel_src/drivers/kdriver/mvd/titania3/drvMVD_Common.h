#ifndef _DRVMVD_COMMON_H_
#define _DRVMVD_COMMON_H_

#ifndef __MDRV_TYPES_H__
/// data type unsigned char, data length 1 byte
typedef unsigned char               MS_U8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              MS_U16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned long               MS_U32;                             // 4 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;                              // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;                             // 2 bytes
/// data type signed int, data length 4 byte
typedef signed long                 MS_S32;                             // 4 bytes
/// data type float, data length 4 byte
typedef float                       MS_FLOAT;                           // 4 bytes

/// definition for MS_BOOL
typedef unsigned char               MS_BOOL;
/// definition for VOID
typedef void                        VOID;
/// definition for FILEID
typedef MS_S32                      FILEID;
#endif

typedef unsigned long MS_PHYADDR;

typedef union _MSIF_Version
{
    struct _DDI
    {
        MS_U8                       tag[4];
        MS_U8                       class[2];
        MS_U16                      customer;
        MS_U16                      model;
        MS_U16                      chip;
        MS_U8                       cpu;
        MS_U8                       name[4];
        MS_U8                       version[2];
        MS_U8                       build[2];
        MS_U8                       change[8];
        MS_U8                       os;
    } DDI;

    struct _MW
    {
    } MW;

    struct _APP
    {
    } APP;

} MSIF_Version;


#include <linux/module.h>
#define printf 						printk


#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif

#ifndef BITS
    #ifdef _BIT
        #define BITS(bits,value)            ((_BIT(((1)?bits)+1)-_BIT(((0)?bits))) & (value<<((0)?bits)))
    #elif defined(BIT)
        #define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
    #endif
#endif

#define MSIF_TAG                    {'M','S','I','F'}                   // MSIF
#define MSIF_CLASS                  {'0','0'}                           // DRV/API (DDI)
#define MSIF_CUS                    0x0000                              // MStar Common library
#define MSIF_MOD                    0x0000                              // MStar Common library
#define MSIF_CHIP                   0x000B
#define MSIF_CPU                    '0'
#define MSIF_OS                     '2'

#define DISABLE                     0
#define ENABLE                      1

#if !defined(TRUE) && !defined(FALSE)
#define TRUE                        1
#define FALSE                       0
#endif


#ifdef NULL
#undef NULL
#endif
#define NULL                        0


#ifndef MS_ASSERT
#define MS_ASSERT(expr)     do {                                                            \
                                if (!(expr))                                                \
                                {                                                           \
                                    printk("MVD assert fail %s %d!\n", __FILE__, __LINE__); \
                                }                                                           \
                            } while(0)
#endif

void msleep(unsigned int msecs);

#endif
