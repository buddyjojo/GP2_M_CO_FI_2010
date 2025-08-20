
#ifndef __MDRV_TYPES_H__
#define __MDRV_TYPES_H__

//#include <linux/types.h>

/// data type unsigned char, data length 1 byte
#ifndef U8
typedef unsigned char                         U8;                                 // 1 byte
#endif
/// data type unsigned short, data length 2 byte
typedef unsigned short                         U16;                                // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int                         U32;                                // 4 bytes
/// data type signed char, data length 1 byte
typedef char                          S8;                                 // 1 byte
/// data type signed short, data length 2 byte
typedef short                        S16;                                // 2 bytes
/// data type signed int, data length 4 byte
typedef int                         S32;                                // 4 bytes

/// definitaion for MStar BOOL
typedef unsigned int            B16;
/// definition for MS_BOOL, this will be removed later
typedef unsigned int	                    BOOL;

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

#define TQM_MODE	// LGE drmyung 081014
#define REFINE_FPLL             1

#define USE_BOARD_DEFINE    0      // for T3 development

#if 0 //daniel temp for blocking print jfi
#define USE_MENULOAD_PQ         0
#define USE_MENULOAD_SCALER     0
#else
#define USE_MENULOAD_PQ         1
#define USE_MENULOAD_SCALER     1
#endif


#define use_IO_Expand           0//1

#if use_IO_Expand
//------------------------------I2C_IO_EXPANDER_TYPE----------------------------
#define I2C_IO_EXPANDER_NONE    0
#define I2C_IO_EXPANDER_PCA9557 1
#define I2C_IO_EXPANDER_PCF8574 2


#define I2C_IO_EXPANDER_TYPE        I2C_IO_EXPANDER_PCA9557
#define I2C_IO_EXPANDER_ADDR        0x30

#define I2C_IO_EXPANDER_P0_IS_GPIO  GPIO_OUT_LOW            // RGB0_SW
#define I2C_IO_EXPANDER_P1_IS_GPIO  GPIO_OUT_HIGH           // PANEL_ON     (HIGH: OFF)
#define I2C_IO_EXPANDER_P2_IS_GPIO  GPIO_OUT_HIGH           // VBLCTRL      (HIGH: OFF)
#define I2C_IO_EXPANDER_P3_IS_GPIO  GPIO_OUT_HIGH           // MUTE_S       (HIGH: MUTE)
#define I2C_IO_EXPANDER_P7_IS_GPIO  GPIO_OUT_HIGH           // GREEN_LED
#endif



#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                        1
/// definition for FALSE
#define FALSE                       0
#endif

#ifndef true
/// definition for true
//#define true                        1
/// definition for false
//#define false                       0
#endif


#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                        1
/// definition for FALSE
#define FALSE                       0
#endif


#if !defined(ENABLE) && !defined(DISABLE)
/// definition for ENABLE
#define ENABLE                      1
/// definition for DISABLE
#define DISABLE                     0
#endif


#if !defined(ON) && !defined(OFF)
/// definition for ON
#define ON                          1
/// definition for OFF
#define OFF                         0
#endif

/// @name BIT#
/// definition of one bit mask
/// @{
#if !defined(BIT0) && !defined(BIT1)
#define BIT0	                    0x00000001
#define BIT1	                    0x00000002
#define BIT2	                    0x00000004
#define BIT3	                    0x00000008
#define BIT4	                    0x00000010
#define BIT5	                    0x00000020
#define BIT6	                    0x00000040
#define BIT7	                    0x00000080
#define BIT8	                    0x00000100
#define BIT9	                    0x00000200
#define BIT10	                    0x00000400
#define BIT11	                    0x00000800
#define BIT12	                    0x00001000
#define BIT13	                    0x00002000
#define BIT14	                    0x00004000
#define BIT15  	                    0x00008000
#define BIT16                       0x00010000
#define BIT17                       0x00020000
#define BIT18                       0x00040000
#define BIT19                       0x00080000
#define BIT20                       0x00100000
#define BIT21                       0x00200000
#define BIT22                       0x00400000
#define BIT23                       0x00800000
#define BIT24                       0x01000000
#define BIT25                       0x02000000
#define BIT26                       0x04000000
#define BIT27                       0x08000000
#define BIT28                       0x10000000
#define BIT29                       0x20000000
#define BIT30                       0x40000000
#define BIT31                       0x80000000
#endif
/// @}

#endif // #ifndef __MDRV_TYPES_H__

