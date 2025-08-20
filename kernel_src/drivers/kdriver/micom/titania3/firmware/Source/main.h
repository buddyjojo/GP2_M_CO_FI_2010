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

#ifndef _MAIN_H_
#define _MAIN_H_

#include "datatype.h"
#include "board.h"

#define LGE_USE_EEPROM
#define LGE_SOL	// Leehc_tmp
#if 0//20080925  fix Set last cold standby -> unplug the AC power cable -> plug the AC power cable -> press the local power key in cold standby mode -> cannot wake up
#define KEYPAD_SET_BY_HOST
#endif

#define PM_DBG0(x)      //x
#define PM_DBG1(x)      //x
#define PM_DBGs(x)      //x

#if (MS_BOARD_TYPE_SEL == BD_S5_MERCURY)
#define ENABLE_BLINKING_LED
#endif

#define ENABLE_NEW_AC_DET

#ifdef ENABLE_NEW_AC_DET
#define START_POLL_AC_DET_WAIT_TIME 400 //ms
#define MICOM_RESET_SLEEP_TIME  1 //second
#if 1
#define METHOD_SYNC_1SEC_BOUNDARY
#else
#define METHOD_RE_INIT_RTC
#endif
#endif

#define ENABLE_WDT
//#define ENABLE_LED	// Blocked
//#define ENABLE_AC_DET
#if 0//20080925  remove SCART ID
#define ENABLE_SCART2_ID_DET
#endif

#define POWER_STATUS_COLD   0
#define POWER_STATUS_COOL   1
#define POWER_STATUS_WARM   2
#define POWER_STATUS_HOT    3   //same as "M_POWER_STATE_ACTIVE"??
#define POWER_STATUS_BOOT   4
#define POWER_STATUS_SWDOWN 5
#define POWER_STATUS_UNKNOWN    0x0F

#define CHIPTOP_RESET_DELAY 100

#define REG_GPIO_PM2_OEN 0x1002
#define REG_GPIO_PM2_OUT 0x1004
#define REG_GPIO_PM3_OEN 0x1002
#define REG_GPIO_PM3_OUT 0x1004
#define POWER_LED_CFG_ENABLE    FALSE
#define POWER_LED_CFG_DISABLE   TRUE


//LGE spec. BYTE value between 0x00~0x09
typedef enum
{
    MICOM_VERSION_BYTE0 = 0,
    MICOM_VERSION_BYTE1 = 0,
    MICOM_VERSION_BYTE2 = 1,
//.
    MICOM_VERSION_BYTE3 = 1,
    MICOM_VERSION_BYTE4 = 9

} MICOM_VERSION;


//>>>>> 20080714 from LGE
/**
 * Power off Mode.
 *
 * @see DDI_MICOM_SendStandbySystemCommand (DDI_POWER_OFF_MODE_T powerOffMode)
 */
typedef enum
{
    /* Micom */
    M_POWER_OFF_BY_CPUCMD         = 0x00, /*CPU Command*/
    M_POWER_OFF_BY_ABN            = 0x01, /**/
    M_POWER_OFF_BY_KEYTIMEOUT     = 0x02, /* If can??t power off during limited time, It should be forced power off */
    M_POWER_OFF_BY_ACDET          = 0x03, /* Power off when can??t detect the AC: Abnormal case*/
    M_POWER_OFF_BY_RESET          = 0x04, /* Power off by Micom Reset  */
    M_POWER_OFF_BY_5VMNT          = 0x05, /* Power off when can??t detect the 5V Monitor : Abnormal case*/
    M_POWER_OFF_BY_NO_POLLING     = 0x06,

    /* CPU */
    M_POWER_OFF_BY_REMOTE_KEY     = 0x10, /* Power off by remote controller*/
    M_POWER_OFF_BY_OFF_TIMER      = 0x11, /* Power off by Off Timer */
    M_POWER_OFF_BY_SLEEP_TIMER    = 0x12, /* Power off by Sleep Timer*/
    M_POWER_OFF_BY_ABNORMAL1      = 0x13, /**/
    M_POWER_OFF_BY_FAN_CONTROL    = 0x14, /** Power off by Fan Control*/
    M_POWER_OFF_BY_INSTOP_KEY     = 0x15, /**  Power off by Instop Key*/
    M_POWER_OFF_BY_AUTO_OFF       = 0x16, /** Power off by Auto Off*/
    M_POWER_OFF_BY_ON_TIMER       = 0x17, /** */
    M_POWER_OFF_BY_RS232C         = 0x18, /**< Power off by Rs232C*/
    M_POWER_OFF_BY_RESREC         = 0x19, /**< Power off by reserved recoding*/
    M_POWER_OFF_BY_RECEND         = 0x1A, /** < Power off by finished recoding*/
    M_POWER_OFF_BY_SWDOWN         = 0x1B, /**< Power off after Software Downlod, */
    M_POWER_OFF_BY_LOCAL_KEY      = 0x1C, /** < Power off by Gemstar*/
    M_POWER_OFF_BY_CPU_ABNORMAL   = 0x1D,/**< Power off by CPU Abnormal status */
    M_POWER_OFF_BY_HOMING_COMPLETE= 0x1E,/**< Power off after North America Cable Card Update, */

    #ifdef INCLUDE_OTA
    M_POWER_OFF_BY_OTA                = 0x1F,/** Power off by finished OTA update */
    #endif

    M_POWER_OFF_BY_UNKNOWN        = 0xFF /*ETC */
}   DDI_POWER_OFF_MODE_T;

/**
 * Power on Mode.
 *
 * @see DDI_MICOM_SendStandbySystemCommand (DDI_POWER_OFF_MODE_T powerOffMode)
 */
typedef enum
{
    M_POWER_ON_BY_REMOTE_KEY          = 0x20,/**< Power on by remote controller*/
    M_POWER_ON_BY_ON_TIMER            = 0x21,/**< Power on by On Time*/
    M_POWER_ON_BY_LOCAL_KEY           = 0x22,/**< Power on by Local Key*/
    M_POWER_ON_BY_RESERVE             = 0x23,/**< Power on by reserved time */
    M_POWER_ON_BY_RS232C              = 0x24,/**< Power on by Rs232C Command*/
    M_POWER_ON_BY_POWER_ONLY          = 0x25,/**< Power on by GEMSTAR*/
    M_POWER_ON_BY_REMIND              = 0x26, /**< In case of Power on in Gemstar */
    M_POWER_ON_BY_SWDOWN              = 0x27,/**< Power on after Software Download*/
    M_POWER_ON_BY_LAST_COLD           = 0x28,/**< The previous power mode when going to previous power  Mode in Power on */
    M_POWER_ON_BY_LAST_COOL           = 0x29,/**< The previous power mode when going to previous power  Mode in Power on */
    M_POWER_ON_BY_LAST_WARM           = 0x2A,/**< The previous power mode when going to previous power  Mode in Power on */
    M_POWER_ON_BY_LAST_POWERON        = 0x2B,/**< The previous power mode when going to previous power  Mode in Power on */
    M_POWER_ON_BY_OTA                 = 0x2C,/**< */
    M_POWER_ON_BY_PODIN               = 0x2D,/**< Power on by inserted POD*/
    M_POWER_ON_BY_HOMING              = 0x2E,/**< Power on by North America Cable Card*/
    M_POWER_ON_BY_HDMI_CEC            = 0x2F,/**< Power on by HDMI CEC Message "image view on"*/
    M_POWER_ON_BY_UNKNOWN             = 0xFF/**< ETC */
}   DDI_POWER_ON_MODE_T;

/**
 * Power State.
 *
 * @see DDI_MICOM_GetPowerState (DDI_POWER_STATE_T *pPowerState)
 */
typedef enum
{
    M_POWER_STATE_COLD_STANDBY    = 0,    /**< Supply power only to Micom, Not CPU, Module  */
    M_POWER_STATE_COOL_STANDBY    = 1,    /**< Cool status, only for North Aerica model */
    M_POWER_STATE_WARM_STANDBY    = 2,    /**< Supply power to Micom and CPU, Not Module(In case of reserved recoding )*/
    M_POWER_STATE_ACTIVE          = 3,    /**< Normal Power on status */
    M_POWER_STATE_BOOT            = 4,    /**< When System Booting */
    M_POWER_STATE_SWDOWN          = 5,    /**< When Software Download */
    M_POWER_STATE_UNKNOWN         = 6     /**< Unknown status */
}   DDI_POWER_STATE_T;
//<<<<< 20080714 from LGE



/* MICOM DDI Layer Command */
typedef enum
{
    CP_WRITE_RTC_YMDHMS = 0x24,
    CP_WRITE_SET_OTA_ONTIMER = 0x42,
    CP_WRITE_SET_OTA_ONTIMER_ENAFLG = 0xF6,
    CP_WRITE_SET_OTATIMER = 0x43,
    CP_WRITE_RESERVE_TIME = 0x40,
    CP_WRITE_SET_RESTIME_ENAFLG = 0xF7,
    CP_WRITE_CANCEL_RESERVE_TIME = 0x41,
    CP_WRITE_POWER_CONTROL = 0x26,
    CP_WRITE_POWER_ON_MODE = 0x21,
    CP_WRITE_COMM_SETID = 0x6C,
    CP_WRITE_CHILD_LOCK = 0x4D,
    CP_WRITE_KEY_LOCK = 0x4E,
    CP_WRITE_SET_ONTIMER = 0x01,
    CP_WRITE_SET_OFFTIMER = 0x02,
    CP_WRITE_CANCEL_ONTIMER = 0x03,
    CP_WRITE_CANCEL_OFFTIMER = 0x04,
    CP_WRITE_HOST_READY = 0x23,
    CP_WRITE_AUTO_POWER_OFFON = 0x44,
    CP_WRITE_SA_PWR_LED_ONOFF = 0x7A,
    CP_WRITE_POWER_OFF_CANCEL = 0x52,
    CP_WRITE_POWER_STATUS = 0xB2,
    CP_WRITE_POWER_MODE_CTRL = 0x64,  // hotel mode
    CP_WRITE_KEYOP_MODE_CTRL = 0x65,
    CP_WRITE_CEC_WKUP_STATUS = 0xA2,//Brian define

    CP_WRITE_HOST_MEM_TO_MICOM = 0xE0,
    CP_WRITE_HOST_MEM_TO_MICOM_START = 0xE1,
    CP_WRITE_HOST_MEM_TO_MICOM_DATA = 0xE2,
    CP_WRITE_KEYPAD_BOUND_1 = 0xE6,
    CP_WRITE_KEYPAD_BOUND_2 = 0xE7,

	CP_WRITE_DISPLAY_MODE = 0x75,	// for demo LED

    CP_READ_OTA_TIME = 0x88,
    CP_READ_RTC_YMDHMS = 0x80,
    CP_READ_GET_ONTIMER = 0x05,
    CP_READ_GET_OFFTIMER = 0x06,
    CP_READ_CANCEL_ONTIMER = 0xF3,
    CP_READ_CANCEL_OFFTIMER = 0xF4,
    CP_READ_OTA_ONTIME = 0xF5,
    CP_READ_RESERVE_TIME = 0x87,
    CP_READ_POWERON_MODE = 0x9A,
    CP_READ_MICOM_VERSION = 0xA1,
    CP_READ_COMM_SETID = 0xF0,
    CP_READ_CHILD_LOCK = 0xF1,
    CP_READ_KEY_LOCK = 0xF2,
    CP_READ_HOST_READY = 0xFF,
    CP_READ_POWEROFF_MODE = 0xFE,
    CP_READ_SA_PWR_LED_ONOFF = 0xFD,
    CP_READ_POWER_OFF_CANCEL = 0xFC,
    CP_READ_POWER_STATUS = 0xB1,
    CP_READ_POWER_MODE_CTRL = 0xB4,   // hotel mode
    CP_READ_KEYOP_MODE_CTRL = 0xB5,
    CP_READ_CEC_WAKEUP_MESSAGE = 0xF8,
    CP_READ_CEC_WKUP_STATUS = 0xA3,//Brian define

    CP_READ_MICOM_TO_HOST_MEM = 0xE3,
    CP_WRITE_MICOM_TO_HOST_MEM_START = 0xE4,
    CP_READ_MICOM_TO_HOST_MEM_DATA = 0xE5,


    CP_NO_CMD = 0

} DDI_MICOM_CMDTYPE;
// hotel mode
typedef enum
{
    M_PWC_LAST_POWER    = 0,
    M_PWC_ONLY_STBY     = 1,
    M_PWC_ONLY_POWON    = 2
} MICOM_PWT_T;

typedef enum
{
    M_KEYOP_NORMAL      = 0,
    M_KEYOP_ONLY_IR     = 1,
    M_KEYOP_ONLY_LOCAL  = 2,
    M_KEYOP_ONLY_IR_POWER = 3
} M_MICOM_KEY_OPERATION_T;

typedef enum
{
    EN_TIME_OFFTIME_CANCEL = 0,
    EN_TIME_OFFTIME_ONCE = 0xFF,
    EN_TIME_OFFTIME_EVERYDAY = 0x7F,
    EN_TIME_OFFTIME_MON_FRI = 0x3E,
    EN_TIME_OFFTIME_MON_SAT = 0x7E,
    EN_TIME_OFFTIME_WEEKEND = 0x41,
    EN_TIME_OFFTIME_SUNDAY = 0x01,
    EN_TIME_OFFTIME_NUM = 0x07
} EN_TIME_OFFTIME;

typedef enum
{
    EN_TIME_ONTIME_CANCEL = 0,
    EN_TIME_ONTIME_ONCE = 0xFF,
    EN_TIME_ONTIME_EVERYDAY = 0x7F,
    EN_TIME_ONTIME_MON_FRI = 0x3E,
    EN_TIME_ONTIME_MON_SAT = 0x7E,
    EN_TIME_ONTIME_WEEKEND = 0x41,
    EN_TIME_ONTIME_SUNDAY = 0x01,
    EN_TIME_ONTIME_NUM = 0x07
} EN_TIME_ONTIME;

//PM_CMD opcode
#define WRITE_CMD   0
#define READ_CMD    1
#define SLEEP_CMD   2
#define CTRL_WRITE_CMD  3
#define CTRL_READ_CMD   4

//select sleep mode
#define SLEEP_MODE  0
#define DEEP_SLEEP_MODE  1
#define POWERKEY_SLEEP_MODE  2

//enable wakeup source
#define IR_WAKEUP       (1<<0)
#define CEC_WAKEUP      (1<<1)
#define GPIO5_WAKEUP    (1<<2)
#define GPIO6_WAKEUP    (1<<3)
#define KEYPAD_WAKEUP   (1<<4)
#define EASYSYNC_WAKEUP (1<<5)
#define SYNC_WAKEUP     (1<<6)
#define RTC_WAKEUP      (1<<7)

#define DVI_DET_WAKEUP      (1<<10)
#define DVI_DET2_WAKEUP      (1<<11)

#define INT_SRC_IR          BIT0
#define INT_SRC_CEC         BIT1
#define INT_SRC_GPIO5       BIT2
#define INT_SRC_GPIO6       BIT3
#define INT_SRC_KEYPAD      BIT4
#define INT_SRC_SYNC        BIT5
#define INT_SRC_DVI         BIT6
#define INT_SRC_RTC         BIT7


#define UPPER_BOUND_DVI_PORTA  0x20
#define LOWER_BOUND_DVI_PORTA  0x06
#define MATCH_VALUE_DVI_PORTA  0x04
#define UPPER_BOUND_DVI_PORTB  0x25
#define LOWER_BOUND_DVI_PORTB  0x09
#define MATCH_VALUE_DVI_PORTB  0x04

#define WAKEUP_STATUS0         XBYTE[0x104E] //record the wakeup status 0
#define WAKEUP_STATUS1         XBYTE[0x104F] //record the wakeup status 0
//#define PM_RIU_DUMMY30         XBYTE[0x1054] // for lock sleep scheme in T2
#define PM_RIU_DUMMY31         XBYTE[0x1055] // for debug

typedef enum
{
    E_PM_WKUP_ALIVE = 15, //PM MCU alive
} EN_PM_WKUP_SRC;

typedef struct
{
    U8  hour;
    U8  min;
    U8  repeat;
} OnOffTime_Info_t;

typedef struct
{
    U16 year;
    U8  month;
    U8  day;
    U8  hour;
    U8  min;
    U8  EnaFlg;
} OTA_OnTime_Info_t;

typedef struct
{
    U16 year;
    U8  month;
    U8  day;
    U8  hour;
    U8  min;
    U8  EnaFlg;
    U8  isRes;
} Reserve_Time_Info_t;


typedef struct
{
    U8 Preamble    : 2;
    U8 opcode    : 6;
    U8 u8Data;
    U16 u16Data;
} PM_Info_t;

#define XBYTE                   ((unsigned char volatile xdata *) 0)

#define DISABLE                     0
#define ENABLE                      1
#define ENABLE_UART0                ENABLE
#define ENABLE_UART1                DISABLE

#define FREQ_12MHZ					(12000000UL)

#define MST_XTAL_CLOCK_HZ			FREQ_12MHZ
#define MST_XTAL_CLOCK_KHZ			(MST_XTAL_CLOCK_HZ/1000UL)
#define MST_XTAL_CLOCK_MHZ			(MST_XTAL_CLOCK_KHZ/1000UL)

#define CRYSTAL_32768               (32768)
#define CRYSTAL_6000000             (6000000ul)
#define CRYSTAL_12000000            (12000000ul)
#define CRYSTAL_14318000            (14318180ul)
#define CRYSTAL_24000000            (24000000ul)
#define CRYSTAL_90000000            (90000000ul)
#define CRYSTAL_83000000            (83000000ul)
#define XTAL_CLOCK                  CRYSTAL_12000000//CRYSTAL_12000000//CRYSTAL_12000000//CRYSTAL_14318000    //CRYSTAL_12000000 //CRYSTAL_14318000
#define MCU_CLOCK                   XTAL_CLOCK
//#define MCU_CLOCK                   35795450    //FPGA//30000000ul //35795450

#ifdef TITANIA1
#define XTAL_RTC_CLOCK_FREQ         (32000) //(32000)/*CRYSTAL_12000000/375*/ /*CRYSTAL_32768*/
#define DEEP_XTAL_RTC_CLOCK_FREQ    (100000)
#else //(TITANIA2)
#define XTAL_RTC_CLOCK_FREQ         (1000000+1)
#define DEEP_XTAL_RTC_CLOCK_FREQ    (4000000)
#endif

#define UART_CLK_REL(clk51, baud)   (1024UL - (((2UL * (clk51))+((baud) * 32UL))/((baud) * 64UL)))
#define UART0_BAUDRATE              9600 //38400
#define UART1_BAUDRATE              38400

#define SBUF0                       S0BUF
#define SBUF1                       S1BUF
#define SBUF                        SBUF0

#ifndef TITANIA1	//support HW UART wakeup after T2
// UART wakeup
#define SEARL_INPUT_TIMEOUT         60000
#define MAX_UART0_CMD_LENGTH        35
typedef enum
{
    _TVLINK_CMD1_,//0
    _TVLINK_CMD2_,
    _TVLINK_SPACE1_,
    _TVLINK_ID_0,
    _TVLINK_ID_1,
    _TVLINK_SPACE2_,
    _TVLINK_DATA0_0_,
    _TVLINK_DATA0_1_,
    _TVLINK_SPACE3_,
    _TVLINK_DATA1_0_,
    _TVLINK_DATA1_1_,//10
    _TVLINK_SPACE4_,
    _TVLINK_DATA2_0_,
    _TVLINK_DATA2_1_,
    _TVLINK_SPACE5_,
    _TVLINK_DATA3_0_,
    _TVLINK_DATA3_1_,//16
    _TVLINK_SPACE6_,
    _TVLINK_DATA4_0_,
    _TVLINK_DATA4_1_,
    _TVLINK_SPACE7_,//20
    _TVLINK_DATA5_0_,
    _TVLINK_DATA5_1_,
    _TVLINK_SPACE8_,
    _TVLINK_DATA6_0_,
    _TVLINK_DATA6_1_,
    _TVLINK_SPACE9_,
    _TVLINK_DATA7_0_,
    _TVLINK_DATA7_1_,
    _TVLINK_SPACE10_,
    _TVLINK_DATA8_0_,//30
    _TVLINK_DATA8_1_,
    _TVLINK_SPACE11_,
    _TVLINK_DATA9_0_,
    _TVLINK_DATA9_1_,//34
    _TVLINK_MAX_LEN_,
}TvLinkCmdIndexEnumType;

#define IsLowerCase(c)          ((c >= 'a') && (c <= 'z'))
#define IsUpperCase(c)          ((c >= 'A') && (c <= 'Z'))
#define IsNumber(c)             ((c >= '0') && (c <= '9'))
#define TVLINK_CMD1     (gu8Uart0CmdBuf[_TVLINK_CMD1_])
#define TVLINK_CMD2     (gu8Uart0CmdBuf[_TVLINK_CMD2_])
#if 0	//20080807 decrease code size
#define TVLINK_ID       (Convert2StrToHexVal(gu8Uart0CmdBuf[_TVLINK_ID_1], gu8Uart0CmdBuf[_TVLINK_ID_0]))
#define TVLINK_DATA0    (Convert2StrToHexVal(gu8Uart0CmdBuf[_TVLINK_DATA0_1_], gu8Uart0CmdBuf[_TVLINK_DATA0_0_]))
#else

#if 0
#define TVLINK_ID       ((gu8Uart0CmdBuf[_TVLINK_ID_1] - '0') | ((gu8Uart0CmdBuf[_TVLINK_ID_0] - '0') << 4))
#endif

#if 0
#define TVLINK_DATA0_1 (gu8Uart0CmdBuf[_TVLINK_DATA0_1_])
#define TVLINK_DATA0_0 (gu8Uart0CmdBuf[_TVLINK_DATA0_0_])
#define TVLINK_DATA0    (   \
                            ( ((TVLINK_DATA0_1>='a')&&(TVLINK_DATA0_1<='f')) ? (TVLINK_DATA0_1 - 0x57) :    \
                            ((TVLINK_DATA0_1>='A')&&(TVLINK_DATA0_1<='F')) ? (TVLINK_DATA0_1 - 0x37) : (TVLINK_DATA0_1 -'0') )  | \
                            ( (  ((TVLINK_DATA0_0>='a')&&(TVLINK_DATA0_0<='f')) ? (TVLINK_DATA0_0 - 0x57) :  \
                                 ((TVLINK_DATA0_0>='A')&&(TVLINK_DATA0_0<='F')) ? (TVLINK_DATA0_0 - 0x37) : (TVLINK_DATA0_0 - '0') \
                              )  << 4)   \
                         )
#define TVLINK_DATA0    ((gu8Uart0CmdBuf[_TVLINK_DATA0_1_] -'0') | ((gu8Uart0CmdBuf[_TVLINK_DATA0_0_] - '0') << 4))
#endif
#endif

#endif	/* #ifndef TITANIA1 */

#define UNKNOWN_KEYCODE             0xFF
#define CH_PLUS_KEYCODE             0x00
#define CH_MINUS_KEYCODE            0x01
#define VOL_PLUS_KEYCODE            0x02
#define VOL_MINUS_KEYCODE           0x03
#define INPUT_KEYCODE               0x0B
#define OK_KEYCODE                  0x44
#define MENU_KEYCODE                0x43
#define POWER_KEYCODE               0x08
#define AUTODEMO_KEYCODE		0x80	// <080923 Leehc> AutoDemo Micom 대응

#ifdef KEYPAD_SET_BY_HOST
typedef struct
{
    U8 u8ChanPlusUB;
    U8 u8ChanPlusLB;
    U8 u8ChanMinusUB;
    U8 u8ChanMinusLB;
    U8 u8VolumePlusUB;
    U8 u8VolumePlusLB;
    U8 u8VolumeMinusUB;
    U8 u8VolumeMinusLB;

    U8 u8OkUB;
    U8 u8OkLB;
    U8 u8MenuUB;
    U8 u8MenuLB;
    U8 u8InputUB;
    U8 u8InputLB;
    U8 u8PowerUB;
    U8 u8PowerLB;

} PM_KeypadParam_t;

static PM_KeypadParam_t PM_KP;
#else
#define CH_PLUS_UPBOUND             0x34 //0x2B//0x39 //0x38
#define CH_PLUS_LOBOUND             0x25 //0x29//0x36 //0x37
#define CH_MINUS_UPBOUND            0x24 //0x19//0x2D //0x2C
#define CH_MINUS_LOBOUND            0x15 //0x17//0x2A //0x2B
#define VOL_PLUS_UPBOUND            0x14 //0x0E//0x18 //0x17
#define VOL_PLUS_LOBOUND            0x07 //0x0C//0x15 //0x16
#define VOL_MINUS_UPBOUND           0x06 //0x03//0x05 //0x04
#define VOL_MINUS_LOBOUND           0x01 //0x01//0x02 //0x03

#define OK_UPBOUND                  0x34 //0x2B//0x39 //0x38
#define OK_LOBOUND                  0x25 //0x29//0x36 //0x37
#define MENU_UPBOUND                0x24 //0x19//0x2D //0x2C
#define MENU_LOBOUND                0x15 //0x17//0x2A //0x2B
#define INPUT_UPBOUND               0x14 //0x0E//0x19 //0x18
#define INPUT_LOBOUND               0x07 //0x0C//0x15 //0x16
#define POWER_UPBOUND               0x06 //0x05 //0x04
#define POWER_LOBOUND               0x01 //0x02 //0x03
#endif


#define KEY_BOUNCE                  50//50
#ifdef MSTAR_SOL
#define KEY_REPEAT                  500//150
#else
#define KEY_REPEAT                  20//150
#endif
#define KHz                         1000UL
#define ISR_TIMER0_PERIOD_US        1000UL  // : 1000us, unit:us, Timer 0 Interrupt period
#define ISR_TIMER0_COUNTER          TIMER_RELOAD(MCU_CLOCK, ISR_TIMER0_PERIOD_US)
#define ISR_TIMER0_COUNTER_XTAL     TIMER_RELOAD(MST_XTAL_CLOCK_HZ, ISR_TIMER0_PERIOD_US)
#define ISR_TIMER1_PERIOD_US        100UL  // : 100us, unit:us, Timer 1 Interrupt period
#define ISR_TIMER1_COUNTER          TIMER_RELOAD(MCU_CLOCK, ISR_TIMER1_PERIOD_US)
#define TIMER_RELOAD(mcuclk, time)  (0x10000UL - TIMER_CLOCK(mcuclk, time))
#define TIMER_CLOCK(mcuclk, time)   ((MCU_CLOCK_KHZ(mcuclk) * (time)) /(12UL * KHz))
#define MCU_CLOCK_KHZ(mcuclk)       ((mcuclk)/KHz)

#define INT_USING0          // compiler will handle this, cannot use "using 0"
#define INT_USING1 using 1
#define INT_USING2 using 2
#define INT_USING3 using 3

#define INT_SERVICE_USING( irq, register_bank ) \
    static void CONCAT( irq, _ISR ) ( void ) interrupt CONCAT( INT_, irq ) CONCAT( INT_USING, register_bank )

#define INT_SERVICE( irq ) INT_SERVICE_USING( irq, 0 )
//------------------------------------------------------------------------------
// IRQ controller
//------------------------------------------------------------------------------
#define IRQ_REG_BASE                    0x2B00
#define EX0_INT_MASK                    (IRQ_REG_BASE)          //0x2b00
#define EX0_INT_MASK_0                 ((EX0_INT_MASK)+0x0)
#define EX0_INT_MASK_1                 ((EX0_INT_MASK)+0x1)
#define EX0_INT_MASK_2                 ((EX0_INT_MASK)+0x2)
#define EX0_INT_MASK_3                 ((EX0_INT_MASK)+0x3)

#define EX0_INT_CLEAR_0                 ((EX0_INT_MASK)+0x8)
#define EX0_INT_CLEAR_1                 ((EX0_INT_MASK)+0x9)
#define EX0_INT_CLEAR_2                 ((EX0_INT_MASK)+0xa)
#define EX0_INT_CLEAR_3                 ((EX0_INT_MASK)+0xb)

#define EX0_INT_FINAL_STATUS_0          ((EX0_INT_MASK)+0x10)
#define EX0_INT_FINAL_STATUS_1          ((EX0_INT_MASK)+0x11)
#define EX0_INT_FINAL_STATUS_2          ((EX0_INT_MASK)+0x12)
#define EX0_INT_FINAL_STATUS_3          ((EX0_INT_MASK)+0x13)

#define MICOM_MSG_ERROR_CHECK	0x10	// <081024> Popnoise 대응 위한 SoftMute 걸어줌.
#define MICOM_MSG_SOFT_MUTE		0x11



extern void MDrv_PM_GpioInit(void);
extern void MDrv_Sys_UartInit( void );
extern void MDrv_PM_Timer0_Init(void);
#ifdef LGE_USE_EEPROM
extern void MDrv_PM_Timer1_Init(void);
#endif
#if defined(LGE_USE_EEPROM) && (BOARD_SUB_TYPE == BD_S6_T2U3)//20080812 for Power On Scenario
extern void MDrv_PM_LED_ByLastPowerState(void);
#endif
extern void MDrv_PM_EnableInterrupt(void);
extern void MDrv_PM_KeepSleep(void);
#ifdef ENABLE_SYNC_WAKEUP
extern void MDrv_PM_ClearSyncDectInterrupt();
#endif

extern void MDrv_PM_ClearSarInterrupt(void);
extern void MDrv_PM_ClearSarInterrupt2(void);
extern void MDrv_PM_ClearCECInterrupt(void);
#ifdef ENABLE_WDT
extern void MDrv_PM_EnableWDT(void);
extern void MDrv_PM_ClearWDT(void);
#else
extern void MDrv_PM_DisableWDT(void);
#endif

extern bool MDrv_PM_IsAlive(void);
extern void MDrv_PM_SetAlive(void);

#if(BOARD_SUB_TYPE == BD_S6_T2U3)
extern void delay1ms(U32 u32DelayTime);
extern void MDrv_PM_Cold2Active_GpioCtrl(void);
extern void MDrv_PM_Cold2Warm_GpioCtrl(void);
extern void MDrv_PM_Active2Cold_GpioCtrl(void);
#endif
extern void MDrv_PM_SleepWakeUp(/*U8 u8SleepMode,*/ U8 IntSrc);
#ifndef TITANIA1
extern U8 ConvertHexVal2Str(U8 ucVal);
extern void MDrv_PM_UART_SendAck(U8 ucCmd, U8 ucResult, U8* pData, U8 ucDataLen, BOOLEAN bSendID);
extern void MDrv_PM_UART_RxData(void);
extern bool MDrv_PM_UART_ParseData(void);
#endif

extern void MDrv_PM_M4Sleep(U8 u8SleepMode, U16 U16SleepSource) small;

U8 Mdrv_Get_TVLinkID(void);



#endif /* #ifndef _MAIN_H_ */


