
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _BOARD_H_
#define _BOARD_H_

#include <linux/autoconf.h>
//#include <linux/undefconf.h>


// alan updates start

//------------------------------BOOTLOADER--------------------------------------
#ifndef BOOTLOADER_SYSTEM
// defined in KeilC UV2
#define BOOTLOADER_SYSTEM           0
#define BOOTLOADER_SYSTEM_LITE      1
#else
#undef BOOTLOADER_SYSTEM
#define BOOTLOADER_SYSTEM           1

#ifndef BOOTLOADER_SYSTEM_LITE
#define BOOTLOADER_SYSTEM_LITE      0
#else
#undef BOOTLOADER_SYSTEM_LITE
#define BOOTLOADER_SYSTEM_LITE      1
#endif

#endif

//------------------------------PUMODE_WORK-------------------------------------
#define PUMODE_WORK             0
#define PUMODE_STANDBY          1

//------------------------------TV_SYSTEM---------------------------------------
#define TV_NTSC                 0
#define TV_PAL                  1
#define TV_CHINA                2

//------------------------------AUDIO_SYSTEM_SEL--------------------------------
#define AUDIO_SYSTEM_BTSC       0
#define AUDIO_SYSTEM_A2         1
#define AUDIO_SYSTEM_EIAJ       2

//Eddie added at 20071231
#define HDCPKEY_USE_CODE    0
#define HDCPKEY_USE_24C04    1
#define HDCPKEY_USE_24C08   2
#define HDCPKEY_USE_24C16   3
#define HDCPKEY_USE_24C32   4
//End of Eddie

//------------------------------DRAM type---------------------------------------
#define SDRAM_32                0
#define DDRAM_32                1
#define DDRAM_16                2

// new style for S4...
#define SDR                     0x10
#define DDR_I                   0x11
#define DDR_II                  0x12

//------------------------------DRAM Bus---------------------------------------
#define DRAM_BUS_16             0
#define DRAM_BUS_32             1

//------------------------------DDRPLL Freq--------------------------------------
#define DDRLLL_FREQ_166         166
#define DDRLLL_FREQ_200         200
#define DDRLLL_FREQ_333         333
#define DDRLLL_FREQ_380         380
#define DDRLLL_FREQ_400         400

//------------------------------DRAM Chip---------------------------------------
#define DRAM_ETRON              0
#define DRAM_NANYA              1

//------------------------------DRAM Size---------------------------------------
#define DDRAM16_32M             0
#define DDRAM16_64M             1

#define EEPROM_CLK_100KHZ       0
#define EEPROM_CLK_200KHZ       1
#define EEPROM_CLK_400KHZ       2

#define EEPROM_BLK_CPY_SIZE     MSAPI_DMX_SECTION_4K

//------------------------------POWER_KEY_SEL------------
#define POWER_KEY_PAD_WAKEUP    0   // power key is connected to PAD_WAKEUP
#define POWER_KEY_PAD_INT       1   // power key is connected to PAD_INT

//------------------------------VIDEO_FIRMWARE_CODE_SEL-------------------------
#define VIDEO_FIRMWARE_CODE_SD  0x00
#define VIDEO_FIRMWARE_CODE_HD  0x10

//------------------------------IO_DEVICE_SEL-----------------------------------
#define IO_DEV_NULL             -1
#define IO_DEV_UART0            0
#define IO_DEV_UART1            1
#define IO_DEV_PIU_UART0        2

//------------------------------EXT_SUBMCU_TYPE---------------------------------
#define EXT_SUBMCU_NONE         0
#define EXT_SUBMCU_MSTAR_UA01   1   // UART, chip A, protocol 01

//------------------------------MEMORY_MAP--------------------------------------
#define MMAP_24MB               0
#define MMAP_16MB               1
#define MMAP_8MB                2
#define MMAP_4MB                3
#define MMAP_4MB_FLASH          4
#define MMAP_32MB               5
#define MMAP_64MB               6
#define MMAP_64_32MB            7
#define MMAP_64_64MB            8
#define MMAP_64MB_SD            9
#define MMAP_128MB              10

//------------------------------FLASH_SIZE--------------------------------------
#define FLASH_SIZE_1MB          0x100000
#define FLASH_SIZE_2MB          0x200000
#define FLASH_SIZE_4MB          0x400000
#define FLASH_SIZE_8MB          0x800000
#define FLASH_SIZE_16MB         0x1000000

//------------------------------MS_FONT_SIZE_TYPE-------------------------------
#define MS_FONT_SMALL           0
#define MS_FONT_LARGE           1
#define MS_FONT_NORMAL          2

//------------------------------UNUSED MACROS-----------------------------------
#define _FUNC_NOT_USED()        do {} while ( 0 )

//S7M,...
#define BD_T3_FPGA_001          0x0701
#define BD_MST087A_D01A_S       0x0702   // 256p QFP
#define BD_MST087B_S7M_D01A_S   0x0703   // S7M - support URSA3
#define BD_MST087B_S7_D01A_S    0x0704   // S7
#define BD_MST087C_D02A_S       0x0705   // SEC X1
#define BD_MST087D_D01A_S       0x0706   // S7

// TODO:

#if (BOOTLOADER_SYSTEM)
    // should always place in the end of Board.h; no move!
    #include "Bootloader.h"
    #define XMODEM_DWNLD_ENABLE      0
    #define XMODEM_1K                0
#else
    #define XMODEM_DWNLD_ENABLE      0
    #define XMODEM_1K                0
#endif

//------PCI_PORT_FUNCTION---------------------------------------------------------
#define NO_PCI_PORT                 0x00
#define PCI_PORT_IS_GPIO            0x01
#define PCI_PORT_IS_CARDREADER      0x02
#define ATCON_PORT                  0x03

#define IS_GPIO_PIN                 0x00
#define IS_PWM_PIN                  0x01

//PWM0~3 mode
#define NORMAL_MODE                 0
#define ENHANCE_MODE                1

//------DMA TYPE------------------------------------------------------------------
#define DRAM_GE_DMA                 0
#define DRAM_W1_DMA                 1
#define DRAM_BYTEDMA                2
#define DRAM_DMATYPE                DRAM_BYTEDMA//DRAM_W1_DMA

//------Sub Micro-----------------------------------------------------------------
#ifndef EXT_SUBMCU_TYPE
#define EXT_SUBMCU_TYPE             EXT_SUBMCU_NONE
#endif

//------------------------------ Board Default -----------------------------------
#ifndef I2C_IO_EXPANDER_TYPE
#define I2C_IO_EXPANDER_TYPE        I2C_IO_EXPANDER_NONE
#endif

// alan updates end


//------------------------------IR_TYPE_SEL--------------
#define IR_TYPE_OLD                 0
#define IR_TYPE_NEW                 1
#define IR_TYPE_MSTAR_DTV           2
#define IR_TYPE_MSTAR_RAW		    3
#define IR_TYPE_RC_V16              4
#define IR_TYPE_CUS03_DTV           5
#define IR_TYPE_MSTAR_FANTASY       6

//------------------------------MEMORY_MAP for 8051------
#define MMAP_8051_CFG_1             0
#define MMAP_8051_CFG_2             1
#define MMAP_8051_CFG_3             2
#define MMAP_8051_CFG_4             3
#define MMAP_8051_CFG_5             4

//------------------------------MEMORY_MAP for 8051 Lite-
#define MMAP_8051_LITE_CFG_1        0
#define MMAP_8051_LITE_CFG_2        1
#define MMAP_8051_LITE_CFG_3        2
#define MMAP_8051_LITE_CFG_4        3
#define MMAP_8051_LITE_CFG_5        4

//------------------------------MEMORY_MAP for Aeon------
#define MMAP_AEON_CFG_1             0
#define MMAP_AEON_CFG_2             1
#define MMAP_AEON_CFG_3             2
#define MMAP_AEON_CFG_4             3
#define MMAP_AEON_CFG_5             4

//------------------------------BOARD_TYPE_SEL-----------
// 0x00 ~ 0x1F LCD Demo board made in Taiwan
#define BD_FPGA                     0x00
#define BD_MST055A_D01A_S           0x01
#define BD_MST055B_D01A_S           0x02
#define BD_MST055C_D01A_S			0x03
#define BD_MST056C_D01A_S			0x04
#define BD_MST055E_D01A_S			0x05
#define BD_MST064C_D01A_S			0x06
#define BD_MST064D_D01A_S			0x07
#define BD_GP2_DEMO1                0x08
#define BD_T3_FPGA                  0x09
// 0x20 ~ 0x3F LCD Demo board made in China
#define BD_SHENZHEN_01              0x20

// 0x40 ~ 0x6F LCD Customers' board
#define BD_S5_LG_DEMO_BOARD_1       0x40
#define BD_S6_LG_DEMO_BOARD_ATSC_1  0x41
#define BD_T2_LG_VENUS_BOARD_DVB_1  0x42
#define BD_T2_LG_MECURY_BOARD_ATSC_1 0x43
#define BD_T2_LG_MECURY_BOARD_DVB_1 0x44
#define BD_T2_LG_MINERVA_BOARD_ATSC_1 0x45
// 0x70 ~ 0x8F CRT Demo board made in Taiwan

// 0x90 ~ 0x9F CRT Demo board made in China

// 0xA0 ~ 0xAF CRT Customers' board

// 0xE1 ~ 0xFE Others' board

#define BD_UNKNOWN                  0xFF

#define SVD_CLOCK_250MHZ            0x00
#define SVD_CLOCK_240MHZ            0x01
#define SVD_CLOCK_216MHZ            0x02
#define SVD_CLOCK_SCPLL             0x03
#define SVD_CLOCK_MIU               0x04
#define SVD_CLOCK_144MHZ            0x05
#define SVD_CLOCK_123MHZ            0x06
#define SVD_CLOCK_108MHZ            0x07

#define SVD_CLOCK_ENABLE            TRUE
#define SVD_CLOCK_INVERT            FALSE

#define ENABLE_UART1_DEBUG    0


#ifndef MS_BOARD_TYPE_SEL

#if defined (CONFIG_MSTAR_TITANIA_BD_T3_FPGA)
    #define MS_BOARD_TYPE_SEL       BD_MST055E_D01A_S
#elif defined (CONFIG_MSTAR_TITANIA_BD_GP2_DEMO1)
    #define MS_BOARD_TYPE_SEL       BD_GP2_DEMO1
#elif defined(CONFIG_MSTAR_TITANIA_BD_MST055A_D01A_S)
    #define MS_BOARD_TYPE_SEL       BD_MST055A_D01A_S
#elif defined (CONFIG_MSTAR_TITANIA_BD_MST055B_D01A_S)
    #define MS_BOARD_TYPE_SEL       BD_MST055B_D01A_S
#elif defined (CONFIG_MSTAR_TITANIA_BD_MST055C_D01A_S)
    #define MS_BOARD_TYPE_SEL       BD_MST055C_D01A_S
#elif defined (CONFIG_MSTAR_TITANIA_BD_MST056C_D01A_S)
    #define MS_BOARD_TYPE_SEL       BD_MST056C_D01A_S
#elif defined (CONFIG_MSTAR_TITANIA_BD_MST055E_D01A_S)
    #define MS_BOARD_TYPE_SEL       BD_MST055E_D01A_S
#elif defined (CONFIG_MSTAR_TITANIA_BD_MST064C_D01A_S)
    #define MS_BOARD_TYPE_SEL       BD_MST064C_D01A_S
#elif defined (CONFIG_MSTAR_TITANIA_BD_MST064D_D01A_S)
    #define MS_BOARD_TYPE_SEL       BD_MST064D_D01A_S
#elif defined (CONFIG_MSTAR_TITANIA_BD_FPGA)
    #define MS_BOARD_TYPE_SEL       BD_FPGA
#elif defined (CONFIG_MSTAR_TITANIA_BD_S5_LG_DEMO_BOARD_1)
    #define MS_BOARD_TYPE_SEL       BD_S5_LG_DEMO_BOARD_1
#elif defined (CONFIG_MSTAR_TITANIA_BD_S6_LG_DEMO_BOARD_ATSC_1)
    #define MS_BOARD_TYPE_SEL       BD_S6_LG_DEMO_BOARD_ATSC_1
#elif defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_VENUS_BOARD_DVB_1)
    #define MS_BOARD_TYPE_SEL       BD_T2_LG_VENUS_BOARD_DVB_1
#elif defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1)
    #define MS_BOARD_TYPE_SEL       BD_T2_LG_MECURY_BOARD_ATSC_1
#elif defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_DVB_1)
    #define MS_BOARD_TYPE_SEL       BD_T2_LG_MECURY_BOARD_DVB_1
#elif defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERVA_BOARD_ATSC_1)
    #define MS_BOARD_TYPE_SEL       BD_T2_LG_MINERVA_BOARD_ATSC_1
#else
    #error "BOARD define not found"
#endif
#endif
//-------------------------------------------------------


/////////////////////////////////////////////////////////
#if   (MS_BOARD_TYPE_SEL == BD_FPGA)
    #include "BD_FPGA.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST055A_D01A_S)
    #include "BD_MST055A_D01A_S.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST055B_D01A_S)
    #include "BD_MST055B_D01A_S.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST055C_D01A_S)
    #include "BD_MST055C_D01A_S.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST056C_D01A_S)
    #include "BD_MST056C_D01A_S.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST055E_D01A_S)
    #include "BD_MST055E_D01A_S.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST064C_D01A_S)
    #include "BD_MST064C_D01A_S.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST064D_D01A_S)
    #include "BD_MST064D_D01A_S.h"
#elif (MS_BOARD_TYPE_SEL == BD_S5_LG_DEMO_BOARD_1)
    #include "BD_S5_LG_DEMO_BOARD_1.h"
#elif (MS_BOARD_TYPE_SEL == BD_S6_LG_DEMO_BOARD_ATSC_1)
    #include "BD_S6_LG_DEMO_BOARD_ATSC_1.h"
#elif (MS_BOARD_TYPE_SEL == BD_T2_LG_VENUS_BOARD_DVB_1)
    #include "BD_T2_LG_VENUS_BOARD_DVB_1.h"
#elif (MS_BOARD_TYPE_SEL == BD_T2_LG_MECURY_BOARD_ATSC_1)
    #include "BD_T2_LG_MECURY_BOARD_ATSC_1.h"
#elif (MS_BOARD_TYPE_SEL == BD_T2_LG_MECURY_BOARD_DVB_1)
    #include "BD_T2_LG_MECURY_BOARD_DVB_1.h"
#elif (MS_BOARD_TYPE_SEL == BD_T2_LG_MINERVA_BOARD_ATSC_1)
    #include "BD_T2_LG_MINERVA_BOARD_ATSC_1.h"
#elif (MS_BOARD_TYPE_SEL == BD_T3_FPGA)//terry
    #include "BD_T3_FPGA.h"
#elif (MS_BOARD_TYPE_SEL == BD_GP2_DEMO1)//terry
    #include "BD_GP2_DEMO1.h"
#else
    #error "MS_BOARD_TYPE_SEL is not defined"
#endif


//----------------------------------------------------------------------------
// UART_SRC_SEL
//----------------------------------------------------------------------------

#define UART0_SRC_SEL   (UART_SEL_PIU_UART0)



/////////////////////////////////////////////////////////

#if 0  // The following code can be found in bond.h
//----------------------------------------------------------------------------
// PIN_PWM2 / PIN_PWM3
//----------------------------------------------------------------------------

#ifndef PIN_PWM2
    #if (CHIP_USE_PWM2_IN_GPIO14 && !PAD_GPIO14_IS_GPIO)
        #define PIN_PWM2    PAD_GPIO14
    #else
        #define PIN_PWM2    PAD_PWM2
    #endif
#endif

#ifndef PIN_PWM3
    #if (CHIP_USE_PWM3_IN_GPIO15 && !PAD_GPIO15_IS_GPIO)
        #define PIN_PWM3    PAD_GPIO15
    #else
        #define PIN_PWM3    PAD_PWM3
    #endif
#endif

//------------------------------GPIO_PIN----------------------------------------
#define GPIO_NONE               0       // Not GPIO pin (default)
#define GPIO_IN                 1       // GPI
#define GPIO_OUT_LOW            2       // GPO output low
#define GPIO_OUT_HIGH           3       // GPO output high
#define GPIO_51PORT_LOW         4
#define GPIO_51PORT_HIGH        5

//----------------------------------------------------------------------------
// PIN_CIMAX
//----------------------------------------------------------------------------

#ifndef PIN_CIMAX
    #if (!CHIP_HAS_CIMAX)
        #define PIN_CIMAX   0
    #else
        #error "PIN_CIMAX not defined"
    #endif
#elif (PIN_CIMAX != 0 && !CHIP_HAS_CIMAX)
    #error "PIN_CIMAX error"
#endif

//----------------------------------------------------------------------------
// PIN_PCMCIA
//----------------------------------------------------------------------------

#ifndef PIN_PCMCIA
    #if (!CHIP_HAS_PCMCIA)
        #define PIN_PCMCIA   0
    #else
        #error "PIN_PCMCIA not defined"
    #endif
#elif (PIN_PCMCIA != 0 && !CHIP_HAS_PCMCIA)
    #error "PIN_PCMCIA error"
#endif

//----------------------------------------------------------------------------
// PIN_SPI_CZ[3:1]
//----------------------------------------------------------------------------

#ifndef PIN_SPI_CZ1
    #define PIN_SPI_CZ1 0
#endif

#ifndef PIN_SPI_CZ2
    #define PIN_SPI_CZ2 0
#endif

#ifndef PIN_SPI_CZ3
    #define PIN_SPI_CZ3 0
#endif

//----------------------------------------------------------------------------
// UART_SRC_SEL
//----------------------------------------------------------------------------

#ifndef UART0_SRC_SEL
    #define UART0_SRC_SEL   (UART_SEL_PIU)
#endif

#ifndef UART1_SRC_SEL
    #define UART1_SRC_SEL   (UART_SEL_HK51_UART1)
#endif

#ifndef UART2_SRC_SEL
    #define UART2_SRC_SEL   (UART_SEL_AEON)
#endif

//----------------------------------------------------------------------------
// UART_INV
//----------------------------------------------------------------------------

#ifndef UART0_INV
    #define UART0_INV   0
#endif

#ifndef UART1_INV
    #define UART1_INV   0
#endif

#ifndef UART2_INV
    #define UART2_INV   0
#endif

//----------------------------------------------------------------------------
// PIN_USBDRVBUS
//----------------------------------------------------------------------------

#ifndef PIN_USBDRVBUS
    #define PIN_USBDRVBUS   0
#endif

#endif

#ifndef USE_SW_I2C
#define USE_SW_I2C                  1
#endif

#define S7_TEMP_PATCH_KER       1   // GP2 bringup flag for kernel. 1:enable ; 0:disable

#endif /* _BOARD_H_ */

