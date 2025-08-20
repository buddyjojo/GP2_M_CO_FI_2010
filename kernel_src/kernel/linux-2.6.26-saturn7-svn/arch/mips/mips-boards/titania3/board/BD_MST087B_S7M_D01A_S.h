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

#ifndef _MSBOARD_H_
#define _MSBOARD_H_

#define BOARD_NAME                  "BD_MST087B_S7M_D01A_S"

//------Function to PIN mapping (PAD_MUX) --------------------------------------
#define PIN_SPI_CZ1                 0
#define PIN_SPI_CZ2                 0
#define PIN_SPI_CZ3                 0
#define PIN_FLASH_WP0               BALL_E9
#define PIN_FLASH_WP1               BALL_F7
#define PIN_UART2                   0
#define PIN_UART3                   0
#define PIN_UART3_IN_TCON           0
#define PIN_UART4                   0
#define PIN_FAST_UART               0
#define PIN_I2S_OUT_MUTE            0
#define PIN_TS1_IS_OUT              0
#define PIN_TS1_IS_TCON             0
#define PIN_PF                      0
#define PIN_ET                      PADS_ET_MODE1
#define PIN_PCM_CONFIG              PADS_PCMCONFIG_MODE1
#define PIN_PCM_CONFIG1             PADS_PCMCONFIG1_MODE1
#define PIN_PCM2_CONFIG             0
#define PIN_PCM2_CONFIG1            0
#define PIN_PCMCIA                  PADS_PCM
#define PIN_I2S_IN_BCK              0
#define PIN_SPDIF_IN                0
#define PIN_MIIC                    0
#define PIN_I2S_OUT                 0
#define PIN_SPDIF_OUT               0

//------GPIO setting(default GPIO pin level)------------------------------------
#define BALL_N22_IS_GPIO            GPIO_OUT_HIGH       // PAD_DDCR_CK - I2C-SCL
#define BALL_M22_IS_GPIO            GPIO_OUT_HIGH       // PAD_DDCR_DA - I2C-SDA
#define BALL_G21_IS_GPIO            GPIO_OUT_HIGH       // PAD_PWM4 - MUTE_S-C
#define BALL_E7_IS_GPIO             GPIO_IN             // PAD_GPIO_PM[0] - GPIO_PM0
#define BALL_E11_IS_GPIO            GPIO_OUT_LOW        // PAD_GPIO_PM[2] - AUMUTE_OUT
#define BALL_G9_IS_GPIO             GPIO_OUT_LOW        // PAD_GPIO_PM[3] - Pre-AMP_SD-C
#define BALL_F9_IS_GPIO             GPIO_OUT_HIGH       // PAD_GPIO_PM[4] - POWER_SW_C
#define BALL_E8_IS_GPIO             GPIO_OUT_LOW        // PAD_GPIO_PM[6] - SPI-CS1N
#define BALL_E9_IS_GPIO             GPIO_OUT_HIGH       // PAD_GPIO_PM[7] - FLASH-WPN1
#define BALL_F7_IS_GPIO             GPIO_OUT_HIGH       // PAD_GPIO_PM[8] - FLASH-WPN2
#define BALL_F6_IS_GPIO             GPIO_OUT_HIGH       // PAD_GPIO_PM[9] - I2C-SDA
#define BALL_D8_IS_GPIO             GPIO_OUT_LOW        // PAD_GPIO_PM[10] - SPI-CS2N
#define BALL_G12_IS_GPIO            GPIO_OUT_HIGH       // PAD_GPIO_PM[11] - I2C-SCL
#define BALL_K21_IS_GPIO            GPIO_OUT_LOW        // PAD_GPIO36 - VBLCTRL-P2-C
#define BALL_L23_IS_GPIO            GPIO_OUT_LOW        // PAD_GPIO37 - AMP_STB-C
#define BALL_K20_IS_GPIO            GPIO_OUT_LOW        // PAD_GPIO38 - NAND_CE2-C
#define BALL_L20_IS_GPIO            GPIO_OUT_HIGH       // PAD_GPIO39 - TUNER-RST
#define BALL_M20_IS_GPIO            GPIO_OUT_HIGH       // PAD_GPIO40 - FLH0_WPz
#define BALL_G20_IS_GPIO            GPIO_OUT_HIGH       // PAD_GPIO41 - PCM_PWR_CTL-C
#define BALL_F20_IS_GPIO            GPIO_IN             // PAD_GPIO50 - USB_OCD1-C
#define BALL_F19_IS_GPIO            GPIO_OUT_HIGH       // PAD_GPIO51 - ETH-RST
#define BALL_N21_IS_GPIO            GPIO_OUT_LOW        // PAD_TCON0 - Panel_ON-P1-C
#define BALL_M21_IS_GPIO            GPIO_OUT_LOW        // PAD_TCON2 - MUTE-POP-C
#define BALL_L22_IS_GPIO            GPIO_OUT_HIGH       // PAD_TCON4 - RP_flash
#define BALL_L21_IS_GPIO            GPIO_OUT_HIGH       // PAD_TCON6 - FLH1_WPz
#define BALL_P21_IS_GPIO            GPIO_IN             // PAD_TCON8 - USB_OCD-C
#define BALL_U1_IS_GPIO             GPIO_OUT_HIGH       // PAD_TGPIO0 - SIF_CTL
#define BALL_U2_IS_GPIO             GPIO_OUT_HIGH       // PAD_TGPIO1 - RF_AGC_CTRL
// ?? Support HW IIC will cancel GPIO initialization
#define BALL_R3_IS_GPIO             GPIO_OUT_HIGH       // PAD_TGPIO2 - I2CM_SCL
#define BALL_T3_IS_GPIO             GPIO_OUT_HIGH       // PAD_TGPIO3 - I2CM_SDA
#define BALL_F5_IS_GPIO             GPIO_IN             // PAD_DDCDA_DA - HDMI0-SDA
#define BALL_F4_IS_GPIO             GPIO_IN             // PAD_DDCDA_CK - HDMI0-SCL
#define BALL_D4_IS_GPIO             GPIO_IN             // PAD_DDCDB_DA - HDMI1-SDA
#define BALL_E4_IS_GPIO             GPIO_IN             // PAD_DDCDB_CK - HDMI1-SCL
#define BALL_AB4_IS_GPIO            GPIO_IN             // PAD_DDCDC_DA - HDMI2-SDA
#define BALL_AA4_IS_GPIO            GPIO_IN             // PAD_DDCDC_CK - HDMI2-SCL
//------Chip Type---------------------------------------------------------------
#include "chip/MSD307KIV.h"

//------I2C devices-------------------------------------------------------------
#if USE_SW_I2C
#undef USE_SW_I2C
#define USE_SW_I2C 1
#endif

#define I2C_DEV_DATABASE            ((E_I2C_BUS_SYS << 8) | 0xA4)
#define I2C_DEV_HDCPKEY             ((E_I2C_BUS_SYS << 8) | 0xA8)
#define I2C_DEV_EDID_A0             ((E_I2C_BUS_DDCA0 << 8) | 0xA0)
#define I2C_DEV_EDID_D0             ((E_I2C_BUS_DDCD0 << 8) | 0xA0)
#define I2C_DEV_EDID_D1             ((E_I2C_BUS_DDCD1 << 8) | 0xA0)
#define I2C_DEV_EDID_D2             ((E_I2C_BUS_DDCD2 << 8) | 0xA0)

#define RM_DEVICE_ADR               I2C_DEV_DATABASE
#define RM_HDCP_ADR                 I2C_DEV_HDCPKEY

//------Peripheral Device Setting-----------------------------------------------
#define PANEL_TYPE_SEL                  g_PNL_TypeSel//PNL_AU37_T370HW01_HD //PNL_AU20_T200XW02_WXGA//PNL_LG32_WXGA //PNL_AU19PW01_WXGA//PNL_AU20_T200XW02_WXGA //PNL_LG19_SXGA  //PNL_CMO22_WSXGA  //PNL_AU20_T200XW02_WXGA  // PNL_CMO22_WSXGA  // PNL_AU20_T200XW02_WXGA // PNL_AU17_EN05_SXGA
#define SATURN_FLASH_TYPE               FLASH_TYPE_SERIAL
#define SATURN_FLASH_IC                 FLASH_IC_MX25L6405D

#define RM_EEPROM_TYPE                  RM_TYPE_24C512          // RM_TYPE_24C32
#define INPUT_AV_VIDEO_COUNT            1
#define INPUT_SV_VIDEO_COUNT            1
#define INPUT_YPBPR_VIDEO_COUNT         1
#define INPUT_SCART_VIDEO_COUNT         1
#define INPUT_HDMI_VIDEO_COUNT          3

//------Input Source Mux--------------------------------------------------------
#define INPUT_VGA_MUX               INPUT_PORT_ANALOG0
#define INPUT_YPBPR_MUX             INPUT_PORT_ANALOG1
#define INPUT_TV_YMUX               INPUT_PORT_YMUX_CVBS0
#define INPUT_AV_YMUX               INPUT_PORT_YMUX_CVBS2
#define INPUT_AV2_YMUX              INPUT_PORT_NONE_PORT
#define INPUT_AV3_YMUX              INPUT_PORT_NONE_PORT
#define INPUT_SV_YMUX               INPUT_PORT_YMUX_CVBS4
#define INPUT_SV_CMUX               INPUT_PORT_CMUX_CVBS6
#define INPUT_SV2_YMUX              INPUT_PORT_NONE_PORT
#define INPUT_SV2_CMUX              INPUT_PORT_NONE_PORT
#define INPUT_SCART_YMUX            INPUT_PORT_YMUX_CVBS7
#define INPUT_SCART_RGBMUX          INPUT_PORT_ANALOG2
#define INPUT_SCART_FB_MUX          SCART_FB1
#define INPUT_SCART2_YMUX           INPUT_PORT_NONE_PORT
#define INPUT_SCART2_RGBMUX         INPUT_PORT_NONE_PORT
#define INPUT_SCART2_FB_MUX         SCART_FB_NONE



//============================================
// Use ATSC code base audio path setting
//============================================
// Need to check with Audio team ???
#define AUDIO_SOURCE_DTV            AUDIO_DSP1_DVB_INPUT
#define AUDIO_SOURCE_DTV2           AUDIO_DSP2_DVB_INPUT
#define AUDIO_SOURCE_ATV            AUDIO_DSP3_SIF_INPUT
#define AUDIO_SOURCE_PC             AUDIO_AUIN0_INPUT
#define AUDIO_SOURCE_YPBPR          AUDIO_AUIN1_INPUT
#define AUDIO_SOURCE_YPBPR2         AUDIO_AUIN2_INPUT
#define AUDIO_SOURCE_AV             AUDIO_AUIN4_INPUT//AUDIO_AUIN3_INPUT
#define AUDIO_SOURCE_AV2            AUDIO_AUIN4_INPUT
#define AUDIO_SOURCE_AV3            AUDIO_AUIN3_INPUT
#define AUDIO_SOURCE_SV             AUDIO_AUIN4_INPUT//AUDIO_AUIN3_INPUT
#define AUDIO_SOURCE_SV2            AUDIO_AUIN3_INPUT
#define AUDIO_SOURCE_SCART          AUDIO_AUIN2_INPUT
#define AUDIO_SOURCE_SCART2         AUDIO_AUIN3_INPUT//AUDIO_AUIN0_INPUT
#define AUDIO_SOURCE_HDMI           AUDIO_HDMI_INPUT
#define AUDIO_SOURCE_HDMI2          AUDIO_HDMI_INPUT
#define AUDIO_SOURCE_HDMI3          AUDIO_HDMI_INPUT
#define AUDIO_SOURCE_DVI            AUDIO_AUIN0_INPUT
#define AUDIO_SOURCE_DVI2           AUDIO_AUIN0_INPUT
#define AUDIO_SOURCE_DVI3           AUDIO_AUIN0_INPUT

#define AUDIO_PATH_MAIN_SPEAKER     AUDIO_PATH_MAIN
#define AUDIO_PATH_HP               AUDIO_PATH_1
#define AUDIO_PATH_SCART1           AUDIO_PATH_2//AUDIO_PATH_5   // define SCART1=LineOut
#define AUDIO_PATH_SCART2           AUDIO_PATH_5//AUDIO_PATH_2   // define SCART2=SifOut
#define AUDIO_PATH_LINEOUT          AUDIO_PATH_5
#define AUDIO_PATH_SIFOUT           AUDIO_PATH_2
#define AUDIO_PATH_I2S              AUDIO_PATH_5
#define AUDIO_PATH_SPDIF            AUDIO_PATH_3

#define AUDIO_OUTPUT_MAIN_SPEAKER  AUDIO_AUOUT1_OUTPUT
#define AUDIO_OUTPUT_HP            AUDIO_AUOUT1_OUTPUT
#define AUDIO_OUTPUT_SCART1        AUDIO_AUOUT0_OUTPUT
#define AUDIO_OUTPUT_SCART2        AUDIO_AUOUT2_OUTPUT    // always output SIF
#define AUDIO_OUTPUT_LINEOUT       AUDIO_I2S_OUTPUT       // As SCART1
#define AUDIO_OUTPUT_SIFOUT        AUDIO_AUOUT2_OUTPUT

//-----------------------Add GPIO switch setting -------------------------
#define Switch_PC()        _FUNC_NOT_USED()
#define Switch_YPBPR()     _FUNC_NOT_USED()
#define Switch_YPBPR2()    _FUNC_NOT_USED()
#define Switch_AV()        _FUNC_NOT_USED()
#define Switch_AV2()       _FUNC_NOT_USED()
#define Switch_AV3()       _FUNC_NOT_USED()
#define Switch_SV()        _FUNC_NOT_USED()
#define Switch_SV2()       _FUNC_NOT_USED()
#define Switch_SCART()     _FUNC_NOT_USED()
#define Switch_SCART2()    _FUNC_NOT_USED()
#define Switch_DVI()       MApi_XC_DVI_SwitchSrc(INPUT_PORT_DVI0)
#define Switch_DVI2()      MApi_XC_DVI_SwitchSrc(INPUT_PORT_DVI1)
#define Switch_DVI3()      MApi_XC_DVI_SwitchSrc(INPUT_PORT_DVI2)
#define Switch_DVI4()      _FUNC_NOT_USED()
#define Switch_DEFAULT()   _FUNC_NOT_USED()

//===============================================================
//------Tuner Setting-----------------------------------------------------------
#define TS_CLK_INV                      0
#define TS_PARALLEL_OUTPUT              0
#define FRONTEND_DEMOD_TYPE             ZARLINK_ZL10353_DEMODE
#define FRONTEND_TUNER_TYPE             PHILIPS_FQD1216_TUNER//PHILIPS_TDA1316_TUNER //PHILIPS_FQD1216_TUNER
#define BD_MST072A_D01A_WITH_ONBOARD_TUNER_ATV

#define MSB1210_TS_SERIAL_INVERSION          0
#define MSB1210_TS_PARALLEL_INVERSION      1
#define MSB1210_DTV_DRIVING_LEVEL             1 //0 or 1
#define MSB1210_WEAK_SIGNAL_PICTURE_FREEZE_ENABLE  1
//#if (I2C_IO_EXPANDER_TYPE == I2C_IO_EXPANDER_MSG1016RC)
//#define SECAM_L_PRIME_ON()              MDrv_ExGPIO3_Set(BIT3, 1) //For SECAM-L PRIME
//#define SECAM_L_PRIME_OFF()             MDrv_ExGPIO3_Set(BIT3, 0) //For Not SECAM-L PRIME
//#define EXT_RF_AGC_ON()                 MDrv_ExGPIO3_Set(BIT2, 1) // ATV mode: external RF AGC
//#define EXT_RF_AGC_OFF()                MDrv_ExGPIO3_Set(BIT2, 0) // DTV mode: internal RF AGC

//#else
#define SECAM_L_PRIME_ON()              _FUNC_NOT_USED() //GPIO60_SET(1)//_FUNC_NOT_USED()//Bruce;;1127
#define SECAM_L_PRIME_OFF()             _FUNC_NOT_USED() //GPIO60_SET(0)//_FUNC_NOT_USED()//Bruce;;1127
#define EXT_RF_AGC_ON()                 _FUNC_NOT_USED() // ATV mode: external RF AGC
#define EXT_RF_AGC_OFF()                _FUNC_NOT_USED() // DTV mode: internal RF AGC

//#endif
//------DVB Tuner Setting-----------------------------------------------------------
#define FRONTEND_IF_MIXER_TYPE          XUGUANG_T126CWADC //PHILIPS_FQD1216_TUNER
#define FRONTEND_IF_DEMODE_TYPE         MSTAR_VIF_MSB1210   //MSTAR_VIF //PHILIPS_TDA9886

//------IR & Key Setting--------------------------------------------------------
#define IR_TYPE_SEL                     IR_TYPE_MSTAR_DTV   // IR_TYPE_MSTAR_DTV // IR_TYPE_CUS03_DTV // IR_TYPE_NEW
#define KEYPAD_TYPE_SEL                 KEYPAD_TYPE_ORIG    // KEYPAD_TYPE_DEMO
#define POWER_KEY_SEL                   POWER_KEY_PAD_INT

//------Power Setting-----------------------------------------------------------
#define ENABLE_POWER_SAVING             0
#define POWER_DOWN_SEQ                  1
#define POWER_SAVING_T                  0
#define POWER_SAVING_PCMODE             0
#define SCREENSAVER_ENABLE              1
#define NO_SIGNAL_AUTO_SHUTDOWN         1
#define STANDBY_MODE                    POWERMODE_S3
#define POWERUP_MODE                    PUMODE_WORK
#define ENABLE_POWER_GOOD_DETECT        1
#define ENABLE_POWER_SAVING_SIF            1
#define ENABLE_POWER_SAVING_VDMVD        0
#define ENABLE_POWER_SAVING_DPMS        0


//------Memory Setting----------------------------------------------------------
#define BOOTUP_MIU_BIST                 1
#define MEMORY_MAP                      MMAP_64_64MB  // 1024 1024  ??TBC

//------Analog Function Setting-------------------------------------------------
#define MOD_LVDS_GPIO                   0x820

#define ENABLE_SSC                      DISABLE
#define ENABLE_LVDSTORGB_CONVERTER      DISABLE
#if ENABLE_SSC
#define MIU_SSC_SPAN_DEFAULT            350
#define MIU_SSC_STEP_DEFAULT            200
#define MIU_SSC_SPAN_MAX                500
#define MIU_SSC_STEP_MAX                300
#define LVDS_SSC_SPAN_DEFAULT           350
#define LVDS_SSC_STEP_DEFAULT           200
#define LVDS_SSC_SPAN_MAX               500
#define LVDS_SSC_STEP_MAX               300
#endif

//------ETHNET PHY_TYPE---------------------------------------------------------
#define ETHNET_PHY_LAN8700              0x0f
#define ETHNET_PHY_IP101ALF             0x01
#define ETHNET_PHY_TYPE                 ETHNET_PHY_IP101ALF

//------DRAM Config---------------------------------------------------------------
#define DRAM_TYPE                       DDR_II
#define DRAM_BUS                        DRAM_BUS_16
#define DDRPLL_FREQ                     DDRLLL_FREQ_400
#define DDRII_ODT

#define MIU_0_02                        0x0C45
#define MIU_0_1A                        0x5151
#define MIU_0_36                        0x0244
#define MIU_0_38                        0x0070

//------MCU use Scaler internal MPLL clock-------------------
#define MCU_CLOCK_SEL                   MCUCLK_144MHZ

#define MST_XTAL_CLOCK_HZ               FREQ_12MHZ
#define MST_XTAL_CLOCK_KHZ              (MST_XTAL_CLOCK_HZ/1000UL)
#define MST_XTAL_CLOCK_MHZ              (MST_XTAL_CLOCK_KHZ/1000UL)

//------MCU Code----------------------------------------------------------------
#define ENABLE_HKMCU_ICACHE_BYPASS      0
#define ENABLE_HKMCU_CODE_ECC           0

//------Extra-------------------------------------------------------------------
#define POWER_DOWN_INFORM_EXTERNALMCU   0

#if POWER_DOWN_INFORM_EXTERNALMCU
#define EXMCU_SLAVE_ADDR        0xA8
#define EXMCU_SUBADDRESS        0x04
#define EXMCU_SLEEP_MODE        0x00
#endif

#define IIC_BY_HW                       0 //
#define IIC_BY_SW                       1 //
#define _EEPROM_ACCESS                  IIC_BY_SW//IIC_BY_HW
#define EEPROM_CLK_SEL                  EEPROM_CLK_100KHZ

//------MST I/O control definition----------------------------------------------
#define ENABLE_DPWM_FUNCTION            0

//-------------------------------------------------
#define SCART_OUT_ON()                  _FUNC_NOT_USED()
#define SCART_OUT_OFF()                 _FUNC_NOT_USED())

// Video switch Setting
#define Switch_YPbPr1()                 _FUNC_NOT_USED()
#define Switch_YPbPr2()                 _FUNC_NOT_USED()

#define SwitchRGBToSCART()              _FUNC_NOT_USED()
#define SwitchRGBToDSUB()               _FUNC_NOT_USED()

#define MDrv_Sys_GetUsbOcdN()           _FUNC_NOT_USED
#define MDrv_Sys_GetRgbSw()

// Audio Amplifier
#define Audio_Amplifier_ON()            mdrv_gpio_set_high( BALL_L23 )
#define Audio_Amplifier_OFF()           mdrv_gpio_set_low( BALL_L23 )

#define Adj_Volume_Off()                mdrv_gpio_set_high( BALL_G21 )
#define Adj_Volume_On()                 mdrv_gpio_set_low( BALL_G21 )

#define Panel_VCC_ON()                  mdrv_gpio_set_low( BALL_N21 )
#define Panel_VCC_OFF()                 mdrv_gpio_set_high( BALL_N21 )
#define Panel_Backlight_VCC_ON()        mdrv_gpio_set_low( BALL_K21 )
#define Panel_Backlight_VCC_OFF()       mdrv_gpio_set_high( BALL_K21 )

#define Panel_Backlight_PWM_ADJ(x)      msPWM2_DutyCycle( x )
#define Panel_Backlight_Max_Current(x)  msPWM2_DutyCycle( x )

#define Panel_VG_HL_CTL_ON()            _FUNC_NOT_USED()
#define Panel_VG_HL_CTL_OFF()           _FUNC_NOT_USED()
#define PANEL_CONNECTOR_SWAP_LVDS_CH    0
#define PANEL_CONNECTOR_SWAP_LVDS_POL   0
#define PANEL_CONNECTOR_SWAP_PORT       1

// Power Saving
#define Power_On()                      _FUNC_NOT_USED()
#define Power_Off()                     _FUNC_NOT_USED()
#define MDrv_Sys_GetSvideoSw()          _FUNC_NOT_USED()

#define Peripheral_Device_Reset_ON()    _FUNC_NOT_USED()
#define Peripheral_Device_Reset_OFF()   _FUNC_NOT_USED()
#define Tuner_ON()                      _FUNC_NOT_USED()
#define Tuner_OFF()                     _FUNC_NOT_USED()
#define Demodulator_ON()                _FUNC_NOT_USED()
#define Demodulator_OFF()               _FUNC_NOT_USED()
#define LAN_ON()                        _FUNC_NOT_USED()
#define LAN_OFF()                       _FUNC_NOT_USED()

#define TunerOffPCMCIA()                _FUNC_NOT_USED()
#define TunerOnPCMCIA()                 _FUNC_NOT_USED()

// LED Control
#define LED_RED_ON()                    _FUNC_NOT_USED()
#define LED_RED_OFF()                   _FUNC_NOT_USED()
#define LED_GREEN_ON()                  _FUNC_NOT_USED()
#define LED_GREEN_OFF()                 _FUNC_NOT_USED()

#define ST_DET_Read()                   0
#define ANT_5V_MNT_Read()               0
#define TU_ERROR_N_Read()               0
#define HDMI_5V_Read()                  0
#define COMP_SW_Read()                  1
#define PANEL_CTL_Off()                 Panel_VCC_OFF()
#define PANEL_CTL_On()                  Panel_VCC_ON()
#define INV_CTL_Off()                   Panel_Backlight_VCC_OFF()
#define INV_CTL_On()                    Panel_Backlight_VCC_ON()
#define POWER_ON_OFF1_On()              Power_On()
#define POWER_ON_OFF1_Off()             Power_Off()
#define MUTE_On()                       Adj_Volume_Off()//(XBYTE[0x1e63] |= BIT7)
#define MUTE_Off()                      Adj_Volume_On()//(XBYTE[0x1e63] &= ~BIT7)
#define EEPROM_WP_On()                  _FUNC_NOT_USED()
#define EEPROM_WP_Off()                 _FUNC_NOT_USED()
#define LED_GRN_Off()                   LED_GREEN_OFF()
#define LED_GRN_On()                    LED_GREEN_ON()
#define LED_RED_Off()                   LED_GRN_On()
#define LED_RED_On()                    LED_GRN_Off()
#define ANT_5V_CTL_Off()                _FUNC_NOT_USED()
#define ANT_5V_CTL_On()                 _FUNC_NOT_USED()
#define BOOSTER_Off()                   _FUNC_NOT_USED()
#define BOOSTER_On()                    _FUNC_NOT_USED()
#define RGB_SW_On()                     _FUNC_NOT_USED()
#define RGB_SW_Off()                    _FUNC_NOT_USED()
#define SC_RE1_On()                     _FUNC_NOT_USED()
#define SC_RE1_Off()                    _FUNC_NOT_USED()
#define SC_RE2_On()                     _FUNC_NOT_USED()
#define SC_RE2_Off()                    _FUNC_NOT_USED()
#define TU_RESET_N_On()                 _FUNC_NOT_USED()//mdrv_gpio_set_high( BALL_P4 )
#define TU_RESET_N_Off()                _FUNC_NOT_USED()//mdrv_gpio_set_low( BALL_P4 )
#define DeactivateScartRecord1()        //SetGPIOHigh(SC_RE1)
#define ActivateScartRecord1()            //SetGPIOLow(SC_RE1)
#define DeactivateScartRecord2()        //SetGPIOHigh(SC_RE2)
#define ActivateScartRecord2()            //SetGPIOLow(SC_RE2)
//------MST Keypad definition---------------------------------------------------
#define POWER_KEY_PAD_BY_INTERRUPT 0
#define ADC_KEY_CHANNEL_NUM             2
#define ADC_KEY_LAST_CHANNEL            ADC_KEY_CHANNEL_NUM - 1

#define KEYPAD_KEY_VALIDATION           3
#define KEYPAD_REPEAT_KEY_CHECK         KEYPAD_KEY_VALIDATION + 2
#define KEYPAD_REPEAT_KEY_CHECK_1       KEYPAD_KEY_VALIDATION + 3
#define KEYPAD_STABLE_NUM               10
#define KEYPAD_STABLE_NUM_MIN           9
#define KEYPAD_REPEAT_PERIOD            2 // 6
#define KEYPAD_REPEAT_PERIOD_1          KEYPAD_REPEAT_PERIOD/2

#define ADC_KEY_LEVEL                   4
#define ADC_KEY_L0                      0x04
#define ADC_KEY_L1                      0x0B
#define ADC_KEY_L2                      0x13
#define ADC_KEY_L3                      0x19

#if (KEYPAD_TYPE_SEL == KEYPAD_TYPE_CUSTMOER)   // CUSTMOER keypad
#define ADC_KEY_1_L0_FLAG               IRKEY_UP
#define ADC_KEY_1_L1_FLAG               IRKEY_MENU
#define ADC_KEY_1_L2_FLAG               IRKEY_LEFT
#define ADC_KEY_1_L3_FLAG               IRKEY_MUTE

#define ADC_KEY_2_L0_FLAG               IRKEY_POWER
#define ADC_KEY_2_L1_FLAG               IRKEY_INPUT_SOURCE
#define ADC_KEY_2_L2_FLAG               IRKEY_RIGHT
#define ADC_KEY_2_L3_FLAG               IRKEY_DOWN
#elif (KEYPAD_TYPE_SEL == KEYPAD_TYPE_ORIG)   // MStar normal keypad
#define ADC_KEY_1_L0_FLAG               IRKEY_UP
#define ADC_KEY_1_L1_FLAG               IRKEY_MENU
#define ADC_KEY_1_L2_FLAG               IRKEY_LEFT
#define ADC_KEY_1_L3_FLAG               IRKEY_MUTE

#define ADC_KEY_2_L0_FLAG               IRKEY_POWER
#define ADC_KEY_2_L1_FLAG               IRKEY_INPUT_SOURCE
#define ADC_KEY_2_L2_FLAG               IRKEY_RIGHT
#define ADC_KEY_2_L3_FLAG               IRKEY_DOWN
#elif (KEYPAD_TYPE_SEL == KEYPAD_TYPE_DEMO) // MStar demo set keypad
#define ADC_KEY_1_L0_FLAG               IRKEY_MUTE
#define ADC_KEY_1_L1_FLAG               IRKEY_VOLUME_MINUS
#define ADC_KEY_1_L2_FLAG               IRKEY_VOLUME_PLUS
#define ADC_KEY_1_L3_FLAG               IRKEY_DOWN

#define ADC_KEY_2_L0_FLAG               IRKEY_POWER
#define ADC_KEY_2_L1_FLAG               IRKEY_UP
#define ADC_KEY_2_L2_FLAG               IRKEY_MENU
#define ADC_KEY_2_L3_FLAG               IRKEY_INPUT_SOURCE
#endif

//------------------------------------------------------------------------------
// SAR boundary define
//------------------------------------------------------------------------------
#define LK_KEYPAD_CH1_UB                0x3F
#define LK_KEYPAD_CH1_LB                0x1A
#define LK_KEYPAD_CH2_UB                0x3F
#define LK_KEYPAD_CH2_LB                0x1A
#define LK_KEYPAD_CH3_UB                0x3F
#define LK_KEYPAD_CH3_LB                0x00
#define LK_KEYPAD_CH4_UB                0x3F
#define LK_KEYPAD_CH4_LB                0x00

#define LK_CH_MINUS_UB                  0x11
#define LK_CH_MINUS_LB                  0x0D
#define LK_CH_PLUS_UB                   0x11
#define LK_CH_PLUS_LB                   0x0D
#define LK_INPUT_UB                     0x09
#define LK_INPUT_LB                     0x05
#define LK_MENU_UB                      0x09
#define LK_MENU_LB                      0x05
#define LK_OK_UB                           0x18
#define LK_OK_LB                           0x14
#define LK_POWER_UB                     0x03
#define LK_POWER_LB                     0x00
#define LK_VOL_MINUS_UB                 0x18
#define LK_VOL_MINUS_LB                 0x14
#define LK_VOL_PLUS_UB                   0x03
#define LK_VOL_PLUS_LB                   0x00

//-----PIN_OUT_SELECT------------------------------------------------------------------------

//#define PWM0_PERIOD                     0xff
//#define PWM1_PERIOD                     0xff
#define PWM2_PERIOD                     0xFF    //PWM2 Period=( PWM2_PERIOD+1 ) *( 1/ Xtal)
//#define PWM3_PERIOD                     0xFf

//#define INIT_PWM0_DUTY                  0x7e
//#define INIT_PWM1_DUTY                  0x7e
#define INIT_PWM2_DUTY                  0x7e    //PWM2_duty= (Init_Pwm2_DUTY +1 ) * (1/XTAL)
//#define INIT_PWM3_DUTY                  0x7e

#define BACKLITE_INIT_SETTING           ENABLE

#define PWM2_MUX_SEL                    0x00

//------8051 Serial Port Setting------------------------------------------------
#if(ENABLE_UART1_DEBUG)
#define ENABLE_UART0                    DISABLE
#define ENABLE_UART0_INTERRUPT          DISABLE
#define ENABLE_UART1                    ENABLE
#define ENABLE_UART1_INTERRUPT          ENABLE
//------STDIO device setting----------------------------------------------------
#define STDIN_DEVICE                    IO_DEV_UART1
#define STDOUT_DEVICE                   IO_DEV_UART1
#else
#define ENABLE_UART0                    ENABLE
#define ENABLE_UART0_INTERRUPT          ENABLE
#define ENABLE_UART1                    DISABLE
#define ENABLE_UART1_INTERRUPT          DISABLE
//------STDIO device setting----------------------------------------------------
#define STDIN_DEVICE                    IO_DEV_UART0
#define STDOUT_DEVICE                   IO_DEV_UART0
#endif

#define ENABLE_PIU_UART0                DISABLE
#define ENABLE_PIU_UART0_INTERRUPT      DISABLE

#endif // _MSBOARD_H_
