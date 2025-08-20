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

#ifndef _BOARD_H
#define _BOARD_H

//#include "type.h"
//#include "BinInfo.h"

//------------------------------SATURN_PLATFORM---------------------------------
#define PLATFORM_CMODEL             0
#define PLATFORM_FPGA               1
#define PLATFORM_FPGA_SCALER        2

//------------------------------BONDING_TYPE------------------------------------

#define MF_NONE                 0x00
#define MF_FN0                  0x01
#define MF_FN1                  0x02
#define MF_FN2                  0x04
#define MF_FN3                  0x08
#define MF_FN4                  0x10

//------------------------------MEMORY_MAP--------------------------------------
#define MMAP_24MB               0
#define MMAP_16MB               1
#define MMAP_8MB                2
#define MMAP_4MB                3
#define MMAP_4MB_FLASH          4
#define MMAP_32MB               5
#define MMAP_64MB               6

//------------------------------MEMORY_MAP_FUNCTION-----------------------------
#define MMAP_NORMAL             0x00    // PURE TV APP
#define MMAP_MHEG5              0x01
#define MMAP_MPLAYER            0x02

//------------------------------MS_DVB_SDRAM_SIZE-------------------------------
#define MS_DVBORG               1       // Build code using "DVB_All.Uv2"
#define MS_DVB2M                2       // Build code using "DVB_4M.Uv2"
#define MS_DVB4M                3       // Build code using "DVB_4M.Uv2"

//------------------------------MS_FONT_SIZE_TYPE-------------------------------
#define MS_FONT_SMALL           0
#define MS_FONT_LARGE           1

//-------------------------------------------------------
// Frame buffer setting
//-------------------------------------------------------
#define MAIN_FRAME_BUFFER               0
#define MAIN_FRAME_BUFLESS              1
#define MAIN_FRAME_PC444                2
#define MAIN_FRAME_UXGA                 4
#define MAIN_FRAME_BUF_SEL_NO_CHANGE    7

//------------------------------VIDEO_FIRMWARE_CODE_SEL-------------------------
#define VIDEO_FIRMWARE_CODE_SD  0x00
#define VIDEO_FIRMWARE_CODE_HD  0x10
//------------------------------MS_MENU_RESOLUTION------------------------------
#define DTV_640X480             0
#define DTV_1366X768            1

//------------------------------FLASH SETTING-----------------------------------
#define FLASH_TYPE_SERIAL           0
#define FLASH_TYPE_PARALLEL         1

//------------------------------------------------------------------------------
// Serial flash IC
#define FLASH_IC_SST25VF080B    0x0100      // 1M    SST
#define FLASH_IC_SST25VF016B    0x0101      // 2M
#define FLASH_IC_Pm25LV080      0x0200      // 1M    PMC
#define FLASH_IC_Pm25LV016      0x0201      // 2M
#define FLASH_IC_ST25P80_A      0x0300      // 1M    ST
#define FLASH_IC_ST25P16_A      0x0301      // 2M
#define FLASH_IC_AT26DF081A     0x0400      // 1M    ATMEL
#define FLASH_IC_AT26DF161      0x0401      // 2M
#define FLASH_IC_MX25L8005      0x0500      // 1M    MXIC
#define FLASH_IC_MX25L1605A     0x0501      // 2M
#define FLASH_IC_MX25L3205      0x0502      // 4M
#define FLASH_IC_MX25L6405      0x0503      // 8M
#define FLASH_IC_NX25P80        0x0600      // 1M    NX
#define FLASH_IC_NX25P16        0x0601      // 2M
#define FLASH_IC_W25X80         0x0700      // 1M    WINB
#define FLASH_IC_W25X16         0x0701      // 2M
#define FLASH_IC_W25X32         0x0702      // 4M
#define FLASH_IC_W25X64         0x0703      // 8M
#define FLASH_IC_S25FL008A      0x0800      // 1M    SPANSION
#define FLASH_IC_S25FL016A      0x0801      // 2M
#define FLASH_IC_EN25B20T       0x0900      // 2M    EON
#define FLASH_IC_EN25B20B       0x0901      // 2M
#define FLASH_IC_EN25B10T       0x0902      // 1M
#define FLASH_IC_EN25B10B       0x0903      // 1M
#define FLASH_IC_ESMT_F25L016A  0x0A00      // 2M

//------------------------------------------------------------------------------
// Flash Control
#define DEFAULT_CONTROL         0x00
#define INVALID_FLAG            0xFF
// CS#
#define SF_CS_GPIO1             0x01
#define SF_CS_GPIO2             0x02
#define SF_CS_GPIO3             0x03
#define SF_CS_FLH_WE_N          0x04
#define SF_CS_FLH_A21           0x05
#define SF_CS_LDE               0x06
#define SF_CS_LHSYNC            0x07
#define SF_CS_LVSYNC            0x08
#define SF_CS_LHSYNC2           0x09
#define SF_CS_LVSYNC2           0x0A
#define SF_CS_I2S_IN_WS         0x0B
#define SF_CS_I2S_IN_BCK        0x0C
// WP#
#define SF_WP_PWM0              0x01
#define SF_WP_PWM3              0x02
#define SF_WP_PCM2_WAIT_N       0x03
#define SF_WP_CF_D10            0x04
#define SF_WP_SAR3              0x05

#define IS_GPIO_PIN             0x00
#define IS_PWM_PIN              0x01 


//PWM0~3 mode
#define NORMAL_MODE             0
#define ENHANCE_MODE            1
//------------------------------STANDBY_MODE------------------------------------
#define POWERMODE_S0            0   // Working
#define POWERMODE_S1            1   // Sleep, MCU run in DRAM; MIU low speed; Disable un-use power
#define POWERMODE_S2            2   // Sleep, Reboot MCU and run in FLASH; MIU low speed; Disable un-use power
#define POWERMODE_S3            3   // Sleep, Reboot MCU and run in FLASH; DRAM stop, Disable most extra power
#define POWERMODE_S4            4   // Hibernate, Reboot MCU and stop, All clock stop, Disable most power
#define POWERMODE_S5            5   // Off, The system is completely off

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

//------------------------------BOARD_TYPE_SEL----------------------------------
#define BD_SHINCHU_CRT          0
#define BD_SHINCHU_LCD          1
#define BD_NONE                 255

//------------------------------VD_FSC_SEL--------------------------------------
#define VD_4FSC                 0
#define VD_8FSC                 1
//#define VD_16FSC              2 => Not support

//------------------------------------------------------------------------------
#define FREQ_12MHZ              (12000000UL)
#define FREQ_14P318MHZ          (14318180UL)
#define FREQ_24MHZ              (24000000UL)
#define FREQ_67P5MHZ            (67500000UL)
#define FREQ_83MHZ              (83000000UL)
#define FREQ_90MHZ              (90000000UL)
#define FREQ_108MHZ             (108000000UL)
#define FREQ_120MHZ             (120000000UL)
#define FREQ_123MHZ             (123428000UL)
#define FREQ_125MHZ             (125000000UL)
#define FREQ_130MHZ             (130000000UL)
#define FREQ_135MHZ             (135000000UL)
#define FREQ_141MHZ             (141000000UL)
#define FREQ_144MHZ             (144000000UL)
#define FREQ_160MHZ             (160000000UL)
#define FREQ_166MHZ             (166000000UL)
#define FREQ_170MHZ             (170000000UL)
#define FREQ_172P8MHZ           (172800000UL)
#define FREQ_200MHZ             (200000000UL)
#define FREQ_200MHZ_LG          (200000001UL)
#define FREQ_210MHZ             (210000000UL)
#define FREQ_220MHZ             (220000000UL)
#define FREQ_230MHZ             (230000000UL)

//------------------------------DRAM type---------------------------------------
#define SDR_32                  0
#define DDR_32                  1
#define DDR_16                  2

//------------------------------DRAM Chip---------------------------------------
#define DRAM_ETRON              0
#define DRAM_NANYA              1

//------------------------------DRAM Size---------------------------------------
#define DRAM_NONE               0
#define DRAM_32M                1
#define DRAM_64M                2

//------------------------------RM_EEPROM_TYPE----------------------------------
#define RM_TYPE_24C01           0
#define RM_TYPE_24C02           1
#define RM_TYPE_24C04           2
#define RM_TYPE_24C08           3
#define RM_TYPE_24C16           4
#define RM_TYPE_24C32           5
#define RM_TYPE_24C512          6

#define EEPROM_CLK_100KHZ       0
#define EEPROM_CLK_200KHZ       1
#define EEPROM_CLK_400KHZ       2

//------------------------------PANEL_TYPE_SEL----------------------------------
#define PNL_DAC_720X480I        0
#define PNL_DAC_720X480P        1
#define PNL_DAC_720X576I        2
#define PNL_DAC_720X576P        3
#define PNL_DAC_720X833I        4
#define PNL_DAC_1024X768P       5
#define PNL_DAC_1920X1080       6
#define PNL_AU17_EN05_SXGA      7
#define PNL_AU20_VGA            8
#define PNL_AU20_SVGA           9
#define PNL_AU20_UXGA           10
#define PNL_CMO19_SXGA          11
#define PNL_CMO20_VGA           12
#define PNL_CMO27_WXGA          13
#define PNL_CMO29_WXGA          14
#define PNL_CMO32_WXGA          15
#define PNL_FU17_FLC43_WXGA     16
#define PNL_HAN23_HSD230WX01A   17
#define PNL_LG17_SXGA           18
#define PNL_LG19_SXGA           19
#define PNL_LG17_LC171_WXGA     20
#define PNL_LG20_UXGA           21
#define PNL_LG23_LC230_WXGA     22
#define PNL_LG26_WXGA           23
#define PNL_LG30_LC300_WXGA     24
#define PNL_QDI17_EL07_SXGA     25
#define PNL_SEC_LTM170W1_L01    26
#define PNL_SH19_LQ197_VGA      27
#define PNL_SH20_B7B_UXGA       28
#define PNL_SAMSUNG24_WUXGA     29
#define PNL_CMO37_WUXGA         30
#define PNL_AU26_T260XW01_WXGA  31
#define PNL_AU20_M201EW01_WSXGA 32
#define PNL_LG32_WXGA           33
#define PNL_SAMSUNG21_WSXGA     34
#define PNL_LG20_VGA            35
#define PNL_HSD260WX11_A        36
#define PNL_AU10_2_DT           37
#define PNL_CPT20_VGA           38
#define PNL_PVI10_VGA           39
#define PNL_CMO20_WXGA          40
#define PNL_CPT15_XGA           41
#define PNL_PANASONIC22_WVGA    42
#define PNL_TMD133_WXGA         43
#define PNL_HANNS96_SVGA        44
#define PNL_HANNS12_VGA         45
#define PNL_LG15_XGA            46
#define PNL_SHARP08_D           47
#define PNL_TMD12_SVGA          48
#define PNL_AU17_WXGA           49
#define PNL_AU17_EG01_SXGA      50
#define PNL_CMO19_M190A1_WXGA   51
#define PNL_CPT15_XG09_XGA      52
#define PNL_AU26_T260XW02_WXGA  53
#define PNL_AU19PW01_WXGA       54
#define PNL_SAMSUNG19M2_WXGAL01 55
#define PNL_AU07_AT             56
#define PNL_AU20_T200XW02_WXGA  57
#define PNL_CMO26_V260B1_WXGA   58
#define PNL_AU85_AT             59
#define PNL_CMO22_WSXGA         60
#define PNL_AU23_T230XW01_WXGA  61
#define PNL_CMO42_WUXGA         62
#define PNL_LPL22_LM220WE1_WSXGA 63
#define PNL_LPL19_LM190WX1_WXGA 64
#define PNL_SAMSUNG23_LTA230W2L1_WXGA 65
#define PNL_LPL42_WXGA          66
#define PNL_LG_PDP32F1_WVGA		67
#define PNL_AU37_T370HW01_HD    68

#define PNL_DAC_CRT             99

//------------------------------KEYPAD_TYPE_SEL---------------------------------
#define KEYPAD_TYPE_ORIG        0   // MStar normal keypad
#define KEYPAD_TYPE_DEMO        1   // MStar demoset keypad
#define KEYPAD_TYPE_CUSTMOER    2   // Customer

//------------------------------IR_TYPE_SEL-------------------------------------
#define IR_TYPE_OLD             0
#define IR_TYPE_NEW             1
#define IR_TYPE_MSTAR_DTV       2
#define IR_TYPE_MSTAR_RAW       3
#define IR_TYPE_RC_V16          4
#define IR_TYPE_CUS03_DTV       5
#define IR_TYPE_CUS4            6
#define IR_TYPE_DC_LWB1         7
#define IR_TYPE_DC_BN59         8
#define IR_TYPE_SZ_CH           9
//------------------------------IR_MODE_SEL-------------------------------------
#define IR_FULLDECODE_MODE             1
#define IR_RAWDATA_MODE       2
#define IR_SWDECODE_MODE       3
//------------------------------I2C_TYPE_SEL-------------------------------------
#define I2C_TYPE_SW             0
#define I2C_TYPE_HW             1

//------------------------------POWER_KEY_SEL------------
#define POWER_KEY_PAD_WAKEUP    0   // power key is connected to PAD_WAKEUP
#define POWER_KEY_PAD_INT       1   // power key is connected to PAD_INT

//------------------------------AUDIO_DSP_CODE_SEL------------------------------
#define AUDIO_DSP_CODE_DVB      0
#define AUDIO_DSP_CODE_SIF      1

//------------------------------IO_DEVICE_SEL-----------------------------------
#define IO_DEV_NULL             -1
#define IO_DEV_UART0            0
#define IO_DEV_UART1            1

//------------------------------GPIO_PIN----------------------------------------
#define GPIO_NONE               0       // Not GPIO pin (default)
#define GPIO_IN                 1       // GPI
#define GPIO_OUT_LOW            2       // GPO output low
#define GPIO_OUT_HIGH           3       // GPO output high
#define GPIO_51PORT_LOW         4
#define GPIO_51PORT_HIGH        5

//------------------------------I2C_IO_EXPANDER_TYPE----------------------------
#define I2C_IO_EXPANDER_NONE    0
#define I2C_IO_EXPANDER_PCA9557 1
#define I2C_IO_EXPANDER_PCF8574 2


//------------------------------FRONTEND_TYPE-----------------------------------
#define ZARLINK_ZL10353_DEMODE  0   // DVB COFDM Demodulator
#define MSTAR_MSB1200_DEMOD     1   // DVB COFDM Demodulator

#define PHILIPS_TDA1316_TUNER   0    // DVB RF Tuner
#define THOMSON_FE6640_TUNER    1    // DVB RF Tuner
#define PHILIPS_FQD1216_TUNER   2    // DVB RF Tuner
#define MSTAR_MSR1200_TUNER     3    // Silicon Tuner for ATSC & DVB
#define LG_TDTC_G001D_TUNER     4
#define XUGUANG_DDT8A1C_TUNER   5

#define XUGUANG_T126CWADC       0    // DVB Mixer for Analog IF
#define PHILIPS_TDA9886         0    // DVB Domodulator for Analog AV

//------------------------------MS_BOARD_TYPE_SEL-------------------------------
#define BD_FPGA                 0x00FF

// S2, S3
#define BD_MST037A_D01A         0x0020
#define BD_MST037B_D01A         0x0021
#define BD_MST037C_D01A         0x0022

#define BD_MST037E_D01A         0x0023
#define BD_MST037F_C01A         0x0024
#define BD_MST037G_C01A         0x0025
#define BD_MST037H_D01A         0x0026
#define BD_MST037I_C01A         0x0027
#define BD_MST037J_D01A         0x0028
#define BD_MST037K_D01A         0x0029
#define BD_MST037L_D01A         0x003A
#define BD_MST037N_C01A         0x003B

// S3+
#define BD_MST042A_D01A         0x0040
#define BD_MST042B_D01A         0x0041
#define BD_MST042C_D01A         0x0042
#define BD_MST042D_D01A         0x0043
#define BD_MST042F_D01A         0x0044
#define BD_MST042H_D01A         0x0045
#define BD_MST042H_D02A         0x0046
#define BD_MST042K_D01A         0x0047

// S4M S4Lite
#define BD_MST055A_D01A_S       0x0080
#define BD_MST055B_D01A_S       0x0081


#define BD_S6_DVB_GP             0x0082
#define BD_S6_ATSC_GP       	0x0083
#define BD_S6_PDP 		      	0x0085

// SZ board
#define BD_SHENZHEN_01          0x0120
#define BD_MSD109BL_10A         0x0121
#define BD_MSD109BL_11A         0x0122
#define BD_MSD109BL_MTC         0x0123
#define BD_MSD109BL_20A         0x0124

#define BD_UNKNOWN              0xFFFF


//------PCI_PORT_FUNCTION---------------------------------------------------------
#define NO_PCI_PORT              0x00
#define PCI_PORT_IS_GPIO         0x01
#define PCI_PORT_IS_CARDREADER   0x02
#define ATCON_PORT               0x03

#define IS_GPIO_PIN              0x00
#define IS_PWM_PIN               0x01

//PWM0~3 mode
#define NORMAL_MODE             0
#define ENHANCE_MODE            1
//----------------------------- Board Selection --------------------------------

//#define MS_BOARD_TYPE_SEL   BD_S6_DVB_GP
//#define MS_BOARD_TYPE_SEL	BD_S6_ATSC_GP
#define MS_BOARD_TYPE_SEL BD_S6_PDP
//#define ENABLE_BLINKING_LED 0

//----------------------------- Board Link -------------------------------------

#include "BD_GP_DEF.h"

#if 0
//----------------------------- Board Link -------------------------------------
#if (MS_BOARD_TYPE_SEL == BD_MST037A_D01A )
    #include "BD_MST037A_D01A.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST037B_D01A )
    #include "bd_mst037b_d01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST037C_D01A )
    #include "bd_mst037c_d01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST037E_D01A )
    #include "bd_mst037e_d01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST037F_C01A )
    #include "bd_mst037f_c01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST037G_C01A )
    #include "bd_mst037G_c01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST037H_D01A )
    #include "bd_mst037h_d01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST037I_C01A )
    #include "bd_mst037I_C01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST037L_D01A )
    #include "bd_mst037L_d01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST042F_D01A )
    #include "bd_mst042F_d01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST042H_D01A )
    #include "bd_mst042H_d01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST042H_D02A )
    #include "bd_mst042H_d02a.h"
#elif (MS_BOARD_TYPE_SEL == BD_MST042K_D01A )
    #include "bd_mst042K_d01a.h"
#elif (MS_BOARD_TYPE_SEL == BD_FPGA)
    #include "bd_fpga.h"
#elif (MS_BOARD_TYPE_SEL == BD_MSD109BL_10A)
    #include "BD_MSD109BL_10A.h"
#elif (MS_BOARD_TYPE_SEL == BD_MSD109BL_11A)
    #include "BD_MSD109BL_11A.h"
#elif (MS_BOARD_TYPE_SEL == BD_MSD109BL_MTC)
    #include "BD_MSD109BL_MTC.h"
#elif (MS_BOARD_TYPE_SEL == BD_MSD109BL_20A)
    #include "BD_MSD109BL_20A.h"
#endif
#endif

#define BIU_CLOCK                   MCU_CLOCK
#define BIU_CLOCK_BOOT              MCU_CLOCK

//------------------------------UNUSED MACROS-----------------------------------
#define _FUNC_NOT_USED()        do {} while ( 0 )

#endif // _BOARD_H
