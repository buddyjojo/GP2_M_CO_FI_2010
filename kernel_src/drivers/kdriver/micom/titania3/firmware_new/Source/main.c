//#include <reg51.h>
//#include <stdio.h>

#include <intrins.h>
#include "R8051XC.h"
#include "Datatype.h"

#include "main.h"
#include "drvISR.h"
#include "drvRTC.h"
#include "msIR.h"
#include "hwreg.h"
#include "msKeypad.h"
#include "GPIO_macro.h"


extern U8 data g_bIrDetect;


//------------------------------------------------------------------------------
// Global Varibles
//------------------------------------------------------------------------------
#ifdef ENABLE_SYNC_WAKEUP
volatile U8 hsync0, vsync0, easy_hsync0, easy_vsync0;
#endif

static volatile LONG32_BYTE xdata gu32Received0 = {0};
static volatile LONG32_BYTE xdata gu32Received1 = {0};
static volatile LONG32_BYTE xdata gu32Send0 = {0};
static volatile LONG32_BYTE xdata gu32Send1 = {0};
static volatile S32 xdata gs32OTAoffsetRTC = 0;
static volatile OnOffTime_Info_t xdata gOffTimeInfo = {0, 0, 0};
static volatile OnOffTime_Info_t xdata gOnTimeInfo = {0, 0, 0};
static volatile OTA_OnTime_Info_t xdata gOTA_OnTimeInfo = {0, 0, 0, 0, 0, 0};
static volatile Reserve_Time_Info_t xdata gReserveTimeInfo = {0, 0, 0, 0, 0, 0, 0};

static volatile U8 xdata gu8PowerOnMode = 0;
static volatile U8 xdata gu8SetID = 0;
static volatile U8 xdata gu8RMCKeyLock = 0;
static volatile U8 xdata gu8ChildLock = 0;
static volatile U8 xdata gu8HostReady = FALSE;
#ifdef	NOT_USED_4_LGE
static volatile U8 xdata gu8PowerOffMode = 0;
#endif
static volatile U8 xdata gu8AutoOffOnFlag = FALSE;
static volatile U8 xdata gu8PowerLEDCfgDisable = POWER_LED_CFG_ENABLE;
static volatile U32 xdata gu32AutoOffTime = 0;
static volatile U8 xdata gbSleepFlag = FALSE;
static volatile U8 xdata gu8SleepMode = SLEEP_MODE;
static volatile U16 xdata gu16SleepSource = 0;
/* for cancel power off */
static volatile U8 xdata gu8CancelPowerOff = FALSE;
static volatile BIT xdata gbAutoPowerOffFlag = FALSE;
/* for cancel power off */
static volatile U8 xdata gu8PowerStatus = M_POWER_STATE_COLD_STANDBY;

static volatile U8 xdata gu8PowerOnOffMode = 0;
static volatile U8 xdata gu8LastPowerStateReady = FALSE;
static volatile U32 xdata gu32TimerCount = 0;
static volatile U32 xdata gu32PreviousKeyTime = 0;
static volatile U32 xdata gu32CurrentKeyTime = 0;

#ifdef ENABLE_BLINKING_LED

#if	(MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
static volatile U16 xdata gu16LEDDuration = 0;
static volatile U8 xdata gu8LEDBlinkEnable = 0;
#endif

#if	(MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
static volatile U32 xdata gu32TimerCount1 = 0;
static volatile U32 xdata gu32TimerCountBlink = 0;
#endif

#endif

// Leehc - For Local Repeat key handing
static volatile U32 xdata gu32AccKeyTime = 0;
#define REPEAT_RECOG_TIME 200

static volatile U8 xdata gu8OldKey1 = UNKNOWN_KEYCODE;
static volatile U8 xdata gu8OldKey2 = UNKNOWN_KEYCODE;
static volatile U8 xdata gu8WaveIndicator = 0;
static volatile U8 xdata gu8NextPowerStatus = M_POWER_STATE_COLD_STANDBY;
#ifdef	NOT_USED_4_LGE
static volatile U8 xdata gu8LastPowerStatus = M_POWER_STATE_ACTIVE;  // hotel mode
static volatile U8 xdata gu8PowerModeCtrl = M_PWC_LAST_POWER;
static volatile U8 xdata gu8KeyOpModeCtrl = M_KEYOP_NORMAL;
#endif
#ifndef TITANIA1
static volatile U8 xdata gu8Uart0CmdBuf[MAX_UART0_CMD_LENGTH];  // UART wakeup
static volatile U8 xdata gu8Uart0BufIndex = 0;
static volatile U8 xdata gu8Uart0Detected = FALSE;
#endif	/* #ifndef TITANIA1 */
static volatile U8 xdata gu8CecWkupMessage = 0;
static volatile U8 xdata gu8CecWkupStatus = 0;

#ifdef ENABLE_STORE_HOST_SRAM_DATA   //20080819 for store host side SRAM data
static U8 xdata gu8SramData[768];
static U8 gu8StartWriteBit = FALSE;
static U8 gu8StartReadBit = FALSE;
static U16 gu16ptrIndex = 0;
#endif


static U8 gU8PrevPowerState = -1;

#ifdef ENABLE_NEW_AC_DET    //after init, active poll AC_DET
static U8 gu8PollAcDetActiveFlg = TRUE;
static U32 gu32NotCountDownZero = 0xFFFFFFFF;
static U8 gu8ACisDetectedFlg = FALSE;
static U32 gu32SaveSystemTime = 0;
#endif

#ifdef LGE_USE_EEPROM	//20080812 for Power On Scenario
U8 code gu8EepromData1 _at_ (0x1FFF - 0);
U8 code gu8EepromData2 _at_ (0x1FFF - 1);
U8 code gu8EepromData3 _at_ (0x1FFF - 2);
#endif

void delay1ms(U32 u32DelayTime)
{
    U32 i, j;
    volatile U32 delay;

#if 0   //because of non-nested interrupt, so timer interrupt will not process within any other ISR
    u32StartTime = gu32TimerCount = 0;
    while ( (gu32TimerCount - u32StartTime) <= u32DelayTime )
    {
        MDrv_PM_ClearWDT();
    }
#else   //use busy delay method
    for( i = 0; i < u32DelayTime; i++)
    {
        for( j = 0; j < 11; j++)   // 11 times of clearWDT = 1ms unit
        {
            delay = j;
            MDrv_PM_ClearWDT();
        }
    }
#endif
}


// earnest
#if	(MS_BOARD_TYPE_SEL == BD_S6_PDP)
#define PWM_DUTY_MAX			50
#define PWM_DUTY_MIN			0
#define	PWM_PERIOD_MAX_CNT		PWM_DUTY_MAX
#define LED_DIME_PERIOD			(200)	// 1000ms
#define DUTY_CHANGE_TIME		(LED_DIME_PERIOD/(PWM_DUTY_MAX - PWM_DUTY_MIN)) // ms

static volatile U8 xdata gu8PWMClkCnt = 0xff;			// PWM clock counter
static volatile U8  xdata gu8DutyRate = PWM_DUTY_MIN;	// duty ratio value
static volatile BIT xdata guBDirect = TRUE;				// duty ratio up or down

static volatile U8 	xdata gu8400usCnt = 0;				//400us counter
//static volatile BIT	xdata gu81msFlag = FALSE;

static volatile BIT xdata gu840msFlag = FALSE;			//40ms flag
static volatile U16 xdata gu1640msCnt = 0;				//40ms counter

static volatile BIT xdata gu8100msFlag = FALSE;			//100ms flag
static volatile U16 xdata gu16100msCnt = 0;				//100ms counter

static volatile U8 	xdata gu8LEDStep = 0;				//plus step 1 per 100ms(0~125 step)
static volatile BIT	xdata gu8LEDPosition = FALSE;

static volatile BIT	xdata gu8LEDGo = FALSE;				// go or stop(LED running state)
static volatile BIT  xdata gu8DemoLEDON = FALSE;			// True(1) is Store Demo mode,False(0)is home mode

static volatile BIT  xdata gu8DoneLED = FALSE;			// home mode를 한번 수행 하면 True로 set

void MDrv_PM_RunFrontLED(void);
void MDrv_PM_StartFrontLED(void);
void MDrv_PM_SetLEDDutyMIN(void);

void MDrv_PM_StopPWM(void);
void MDrv_PM_InitLED(void);
void MDrv_PM_InitCenterLED(void);



#endif

#ifdef LGE_USE_EEPROM
#if	(MS_BOARD_TYPE_SEL == BD_S6_DVB_GP || MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
void MDrv_PM_LED_ByLastPowerState(void)
{
	switch( gu8EepromData1 & 0x0F )
	{
	case POWER_STATUS_HOT:
#ifdef ENABLE_BLINKING_LED
		Blinking_LED_Start_Blink();
#else
#if	(MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
		MDrv_PM_Cold2Active_GpioCtrl();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
		/* taburin : 20081025, power on 150ms delay  port control for ir's micom*/
		GPIO_SAR2_OUTPUT_HIGH();
		delay1ms(150);
		GPIO_SAR2_OUTPUT_LOW();
#endif
#endif
		break;

	case POWER_STATUS_WARM:
#ifdef ENABLE_BLINKING_LED
		Blinking_LED_Ctrl_Warm();
#else
		MDrv_PM_Cold2Warm_GpioCtrl();
#endif
		break;

	case POWER_STATUS_COLD:
	case POWER_STATUS_COOL:
	case POWER_STATUS_SWDOWN:
	case POWER_STATUS_UNKNOWN:
	case POWER_STATUS_BOOT:
	default:
#ifdef ENABLE_BLINKING_LED
		Blinking_LED_Ctrl_Cold();
#else
#if	(MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
		//let GPIO_PM0 & GPIO_SAR2 keep output low
		GPIO_PM0_OUTPUT_LOW();
		GPIO_SAR2_OUTPUT_LOW();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	    GPIO_SAR2_OUTPUT_HIGH();
#endif
#endif
	    break;

	}

}
#endif
#endif

void MDrv_PM_GpioInit(void)
{
#ifdef LGE_USE_EEPROM   //20080908  for GPIO Setting

#if	(MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
    GPIO_PM0_OUTPUT_LOW();
    GPIO_PM1_INPUT();
    GPIO_PM2_OUTPUT_HIGH();	// Leehc - Improvement for Panel flucking during bootup.
	GPIO_PM3_OUTPUT();	// <081023 Leehc> Panel control initial value low to high
	GPIO_PM3_OUTPUT_HIGH();	// <081023 Leehc> Panel control initial value low to high
    GPIO_PM4_INPUT();
    GPIO_PM5_INPUT();
    GPIO_PM6_INPUT();
    GPIO_SAR0_INPUT();
    GPIO_SAR1_INPUT();
    GPIO_SAR3_OUTPUT_HIGH();
	MDrv_PM_LED_ByLastPowerState();

#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
    GPIO_PM1_INPUT();
    GPIO_PM2_OUTPUT_HIGH();	// Leehc - Improvement for Panel flucking during bootup.
	GPIO_PM3_OUTPUT();	// <081023 Leehc> Panel control initial value low to high
	GPIO_PM3_OUTPUT_HIGH();	// <081023 Leehc> Panel control initial value low to high
    GPIO_PM4_INPUT();
    GPIO_PM5_INPUT();
    GPIO_PM6_INPUT();
    GPIO_SAR0_INPUT();
    GPIO_SAR1_INPUT();
    MDrv_PM_LED_ByLastPowerState();
#elif (MS_BOARD_TYPE_SEL == BD_S6_PDP)
	GPIO_PM0_OUTPUT_LOW();	//LED_W1
	GPIO_PM1_INPUT();		//DBG_TX
	GPIO_PM2_INPUT();	//5V_ON
	GPIO_PM3_INPUT();		//M5V_MNT
	GPIO_PM4_INPUT();		//RL_ON
	GPIO_PM5_INPUT();		//DBG_RX
	GPIO_PM6_INPUT();		//AC_DET
	GPIO_SAR0_INPUT();		//KEY1
	GPIO_SAR1_INPUT();		//KEY2
	GPIO_SAR2_OUTPUT_LOW();	//LED_W2
	GPIO_SAR3_OUTPUT_HIGH();//SB_MUTE
#endif

#else
    GPIO_PM0_OUTPUT_LOW();
    GPIO_PM1_INPUT();
    GPIO_PM2_OUTPUT_LOW();
    GPIO_PM3_OUTPUT_LOW();
    GPIO_PM4_INPUT();
    GPIO_PM5_INPUT();
    GPIO_PM6_INPUT();
    GPIO_SAR0_INPUT();
    GPIO_SAR1_INPUT();
    GPIO_SAR2_INPUT();
    GPIO_SAR3_OUTPUT_HIGH();
#endif

}

void MDrv_Sys_UartInit( void )
{
    if ( ENABLE_UART0 )
    {
        ADCON |= 0x80;
        S0RELL =  (UART_CLK_REL(MCU_CLOCK, UART0_BAUDRATE) & 0xff);
        S0RELH = ((UART_CLK_REL(MCU_CLOCK, UART0_BAUDRATE) >> 8) & 0x03);
        S0CON = 0x50;
        PCON |= 0x80;
        SBUF = '*';     // Send out an character so we can see if it is always restart.
    }
    if ( ENABLE_UART1 )
    {
        S1RELL =  (UART_CLK_REL(MCU_CLOCK, UART1_BAUDRATE) & 0xFF);
        S1RELH = ((UART_CLK_REL(MCU_CLOCK, UART1_BAUDRATE) >> 8) & 0x03);
        S1CON = 0x90;   // mode 1, 8-bit UART, enable receive
        S1BUF = '#';    // Send out an character so we can see if it is always restart.
    }
    //AWU
    //XBYTE[0x1E03] |= 0x04;          // enable UART RX
}

void MDrv_PM_Timer0_Init(void)
{
//    TMOD = (TMOD & 0xF0) | 0x01; // set TIMER0 mode 1

#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
    TMOD = (TMOD & 0xF8) | 0x01; // set TIMER0 mode 1
    TH0 = (ISR_TIMER0_COUNTER/256);
    TL0 = (ISR_TIMER0_COUNTER%256);
#else
    TMOD = (TMOD & 0xF8) | 0x02; // set TIMER0 mode 2
    TH0 = 56;
    TL0 = 56;
#endif  // #if !(MS_BOARD_TYPE_SEL == BD_S6_PDP)
    TR0 = 1;      // TIMER0 enable
    ET0 = 1;
    gu32TimerCount = 0;
    gu32PreviousKeyTime = 0;
    gu32CurrentKeyTime = 0;
    gu8OldKey1 = UNKNOWN_KEYCODE;
    gu8OldKey2 = UNKNOWN_KEYCODE;

#ifdef ENABLE_BLINKING_LED
#if (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
    gu32TimerCount1 = 0;
    gu32TimerCountBlink = 11;
#elif (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
	//gu16LEDDuration = 0;
	//gu8LEDBlinkEnable = 0;
#endif
#endif

}

//#ifdef ENABLE_AC_DET   //20080807 for AC Detection
#ifdef LGE_USE_EEPROM
void MDrv_PM_Timer1_Init(void)
{

#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
    TMOD = (TMOD & 0x8F) | ((0x01) << 4); // set TIMER1 mode 1
    TH1 = (ISR_TIMER1_COUNTER/256);
    TL1 = (ISR_TIMER1_COUNTER%256);
#else
    TMOD = (TMOD & 0x8F) | ((0x02) << 4); // set TIMER1 mode 2
    TH1 = 6;//156;
    TL1 = 6;//156;
#endif  // #if !(MS_BOARD_TYPE_SEL == BD_S6_PDP)

    TR1 = 1;      // TIMER1 enable
    ET1 = 1;
}
#endif
//#endif

#ifdef ENABLE_SCART2_ID_DET	//20080805 for LGE SCART2 ID
#define ADC_SAR_LEVEL                   3
// <Leehc 080902> SCART ID Level Adjust
#define ADC_SAR_L0                      0xE //0x00
#define ADC_SAR_L1                      0x2F //0x26
#define ADC_SAR_L2                      0x3F //0x3A
// <Leehc 080902> SCART ID Level Adjust
#define SAR_STABLE_NUM               (2) //(10)
#define SAR_STABLE_NUM_MIN           (1) //(9)

unsigned char tADCSARLevel[] =
{
    ADC_SAR_L0,
    ADC_SAR_L1,
    ADC_SAR_L2,
};
#endif  /* #ifdef ENABLE_SCART2_ID_DET */


void MDrv_PM_EnableInterrupt(void)
{
	IT0 = 0;//1;
	IT1 = 0;//1;

    IEN0 = 0x85;         // EAL, EX1, EX0

    //Turn on MASK  for FIQ //EX0
    XBYTE[0x2B00] = 0;
    XBYTE[0x2B00] |= (BIT4|BIT3|BIT2|BIT1); //ir_int | xiu_mbox3 | xiu_mbox2 | xiu_mbox1

    //Turn on MASK  for IRQ //EX1
    XBYTE[0x2B18] = 0;
    //XBYTE[0x2B18] |= BIT4;

    EAL = 1;
}

void MDrv_PM_KeepSleep(void)
{   //not wakeup key, keep sleep
#if (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)

	/*taburin : 20090114, RL_OFF 이후 power detect 동작 못하도록....*/
	gu8PollAcDetActiveFlg = FALSE;

	/*taburin : 20081103, cotrol RL_ON(PM0) for saturn5*/
	RL_OFF();
	GPIO_PM3_OUTPUT_HIGH();	// M+S 공용으로 사용하기 위해..
#endif
    POWER_OFF();

#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
	/* Reduce Power cunsumption */
	SB_MUTE_INPUT();
	LED_W2_INPUT();
#endif  // #if (MS_BOARD_TYPE_SEL == BD_S6_PDP)

#if 0   //20080805 for LGE SCART2 ID
    ES0 = 0;
#endif
    XBYTE[0x1054] = 0xa5;   //unlock sleep scheme
    XBYTE[0x1012] = 0xbe;   //unlock sleep scheme
    XBYTE[0x1013] = 0xba;   //unlock sleep scheme
    _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
}

#ifdef ENABLE_SYNC_WAKEUP
void MDrv_PM_ClearSyncDectInterrupt()
{
    if(XBYTE[0x1017] & (BIT1|BIT0))
    {
        //puts("sync in sleep clear sd irq\n");
        //XBYTE[0x1008] |= (BIT5|BIT4);
        XBYTE[0x1009] |= (BIT1|BIT0);
        XBYTE[0x1009] &= ~(BIT1|BIT0);
        //XBYTE[0x1008] &= ~(BIT5|BIT4);
    }
    if(XBYTE[0x1010] & (BIT1|BIT0))
    {
        //puts("sync in sleep clear ezsd irq\n");
        //XBYTE[0x100E] |= (BIT5|BIT4);
        XBYTE[0x100F] |= (BIT1|BIT0);
        XBYTE[0x100F] &= ~(BIT1|BIT0);
        //XBYTE[0x100E] &= ~(BIT5|BIT4);
    }
}
#endif  /* #ifdef ENABLE_SYNC_WAKEUP */
void MDrv_PM_ClearSarInterrupt(void)
{
    if(XBYTE[0x1426] & BIT3)
    {
        //puts("clear sar_int\n");
        XBYTE[0x1426] |= BIT1;  //clear sar_int
        XBYTE[0x1426] &= ~BIT1;
    }
}

void MDrv_PM_ClearSarInterrupt2(void)
{
    if(XBYTE[0x1426] & BIT3)
    {
        //puts("Keypad in sleep clear sar_int\n");
        XBYTE[0x1426] |= (BIT1|BIT2);  //clear sar_int
        XBYTE[0x1426] &= ~(BIT1|BIT2);
    }
}


void MDrv_PM_ClearCECInterrupt(void)
{
#ifdef TITANIA1
    XBYTE[0x1115] |= BIT2;  //clear cec interrupt
    XBYTE[0x1115] &= ~BIT2; //clear cec interrupt
#else	//TITANIA2
    XBYTE[0x1122] |= BIT6;  //clear cec interrupt
#endif
}

#ifdef ENABLE_WDT
void MDrv_PM_EnableWDT(void)
{
    //Enable WDT of M4
    XBYTE[0x1044] = 0xAA;
    XBYTE[0x1045] = 0x55;
}
#else
void MDrv_PM_DisableWDT(void)
{
    //Disable WDT of M4
    XBYTE[0x1044] = 0x55;
    XBYTE[0x1045] = 0xAA;
}
#endif

void MDrv_PM_ClearWDT(void)
{
    XBYTE[0x1046] |= BIT0;  //clear WDT
    XBYTE[0x1046] &= (~BIT0);
}

bool MDrv_PM_IsAlive(void)
{
    U8 u8Wkup, u8Shft;

    u8Shft = (U8)(E_PM_WKUP_ALIVE - 8);
    u8Wkup = WAKEUP_STATUS1 & ((1<<u8Shft));
    return (u8Wkup)? TRUE:FALSE;
}

void MDrv_PM_SetAlive(void)
{
    U8 u8Shft;

    u8Shft = (U8)(E_PM_WKUP_ALIVE - 8);
    WAKEUP_STATUS1 |= (1<<u8Shft);
}

#ifndef ENABLE_BLINKING_LED
void MDrv_PM_Cold2Active_GpioCtrl(void)
{
#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
       // 1. Control the SAR2 Port from ¨Low〃 to ¨High〃 at beginning
       GPIO_SAR2_OUTPUT_LOW();
       delay1ms(5);//please add what duration you want for this Low to High here.
       GPIO_SAR2_OUTPUT_HIGH();
       //After 100ms
       delay1ms(100);
       // 2. Control the PM0 port from ¨Low〃 to ¨High〃
       GPIO_PM0_OUTPUT_LOW();
       delay1ms(5);//please add what duration you want for this Low to High here.
       GPIO_PM0_OUTPUT_HIGH();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	// LED_ON : high -> low
	GPIO_SAR2_OUTPUT_HIGH();
	delay1ms(5);//please add what duration you want for this Low to High here.
	GPIO_SAR2_OUTPUT_LOW();
#endif
}

void MDrv_PM_Cold2Warm_GpioCtrl(void)
{
#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
    // 1.  Control the PM0 Port from ¨Low〃 to ¨High〃 at beginning
    GPIO_PM0_OUTPUT_LOW();
    delay1ms(5);//please add what duration you want for this Low to High here.
    GPIO_PM0_OUTPUT_HIGH();
    //After 100ms
    delay1ms(100);
    // 2. Control SAR2 port from ¨Low〃 to ¨High?
    GPIO_SAR2_OUTPUT_LOW();
    delay1ms(5);//please add what duration you want for this Low to High here.
    GPIO_SAR2_OUTPUT_HIGH();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	delay1ms(5);//please add what duration you want for this Low to High here.
	GPIO_SAR2_OUTPUT_HIGH();
#endif
}

void MDrv_PM_Active2Cold_GpioCtrl(void)
{
#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
    //Control both SAR2 and PM0 port from ¨High〃 to ¨Low〃
    GPIO_SAR2_OUTPUT_HIGH();
    GPIO_PM0_OUTPUT_HIGH();
    delay1ms(5);//please add what duration you want for this High to Low here.
    GPIO_SAR2_OUTPUT_LOW();
    GPIO_PM0_OUTPUT_LOW();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	delay1ms(5);//please add what duration you want for this Low to High here.
	GPIO_SAR2_OUTPUT_HIGH();
#endif
}
#endif


void MDrv_PM_PowerOn()
{
    //if (tIntStatus & tIntSource) //081220 Needless Code
    {
    	XBYTE[0x1043] |= BIT7;  //ChipTop reset
#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
		SB_MUTE_OUTPUT();
#endif
        POWER_ON();
#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
		ON5V_HIGH();  // weapon.ahn $ 5V_ON
		LED_W2_OUTPUT();
#if defined(LGE_GP_COMMERCIAL_MODEL)
		UART_TX_DISABLE();
#endif

#endif

#ifdef ENABLE_NEW_AC_DET
        //Disable Internal Timer 1 to disable polling AC detection
        TR1 = 0;      // TIMER1 disable
        ET1 = 0;
#endif
		delay1ms(120);	/* 500 ms */  // wait for 5V_On high then HKMCU start

#if (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
                /*taburin : 20081103, cotrol RL_ON(PM0) for saturn5*/
		delay1ms(30);
		RL_ON();
#endif
#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
	SB_MUTE_ENABLE();
#endif

#ifdef ENABLE_NEW_AC_DET
        //Enable Internal Timer 1 but does not yet poll AC detection
        gu8PollAcDetActiveFlg = FALSE;
        gu32NotCountDownZero = START_POLL_AC_DET_WAIT_TIME;
        TR1 = 1;      // TIMER1 enable
        ET1 = 1;
#endif

    }
}


#if	(MS_BOARD_TYPE_SEL == BD_S6_PDP)
void MDrv_PM_RunFrontLED(void)
{
   if(gu8LEDGo == TRUE)
   {
	/**************** make dutyrate per 40ms*************/
	if(gu840msFlag == TRUE)
	{
		if(guBDirect)
		{
			gu8DutyRate += 1;
			if(gu8DutyRate >= PWM_DUTY_MAX)
			   gu8DutyRate = PWM_DUTY_MAX;
		 }
		 else
		 {
			gu8DutyRate -= 1;//2;//4;
			if(gu8DutyRate <= PWM_DUTY_MIN)
			   gu8DutyRate = PWM_DUTY_MIN;
		 }

		 gu840msFlag = FALSE;
	  }
	/*****************Start LED**********************************/
	  if(gu8100msFlag == TRUE)
	  {
		 if(gu8LEDStep == 5)			//0.5sec
		 {
			gu8LEDPosition = TRUE;
			MDrv_PM_SetLEDDutyMIN();
		 }
		 else if(gu8LEDStep == 25)		// 2.5sce
		 {
		 	MDrv_PM_StopPWM();			//Stop LED on/off (PWM Stop)
			LED_W2_HIGH();				//Center LED on
		 }
		 else if(gu8LEDStep == 45)		// 4.5sec
		 {
			// per 40ms(decrease dutyrate  50->0)
			guBDirect = FALSE;
			gu8PWMClkCnt = 0;	//per 200us (check timer0) //PWM Start
			gu8DutyRate = PWM_DUTY_MAX;
		 }
		 else if(gu8LEDStep == 65)		// 6.5sec
		 {
		 	if (gu8DemoLEDON)
		 	{
				gu8LEDPosition = TRUE;
		 	}
			else
			{	if(gu8DoneLED ==FALSE)
				gu8LEDPosition = FALSE;
			}
			MDrv_PM_SetLEDDutyMIN();
			LED_W2_LOW();				//Center LED off
		 }
		 else if(gu8LEDStep == 85)		// 8.5sec
		 {
			MDrv_PM_StopPWM();			//Stop LED on/off (PWM Stop)
			if (gu8DemoLEDON)
		 	{
				LED_W2_HIGH();
		 	}
			else
			{	if(gu8DoneLED ==FALSE)
				LED_W1_HIGH();				//Icon LED on
			}
		 }
		 else if(gu8LEDStep == 105)		//10.5sec
		 {
		 	// per 40ms(decrease dutyrate  50->0)
			guBDirect = FALSE;
			gu8PWMClkCnt = 0;	//per 200us (check timer0) //PWM Start
			gu8DutyRate = PWM_DUTY_MAX;
		 }
		 else if(gu8LEDStep == 125)		//12.5sec
		 {
			gu8LEDGo = FALSE;
			MDrv_PM_StopPWM();			//Stop LED on/off (PWM Stop)
			if (gu8DemoLEDON)
		 	{
				LED_W2_LOW();
		 	}
			else
			{
				if(gu8DoneLED ==FALSE)
				LED_W1_LOW();				//Icon LED on
			}
			gu8LEDStep = 0;
			gu8DoneLED =TRUE;
		 }

		 gu8LEDStep++;

		 gu8100msFlag = FALSE;
	  }
   }



   if (gu8DemoLEDON)
   {
  	  if(gu8LEDGo == FALSE )//&& gu8DoneLED == TRUE)
	  {
		 gu8LEDStep = 0;
		 gu8LEDGo = TRUE;

	  }
   }
}


void MDrv_PM_StartFrontLED(void)
{
	gu8LEDStep = 0;
	gu8LEDGo = TRUE;
	gu8DemoLEDON = FALSE;
}


// per 40ms(increase dutyrate  0->50)
void MDrv_PM_SetLEDDutyMIN(void)
{
	guBDirect = TRUE;
	gu8PWMClkCnt = 0;	//per 200us (check timer0) //PWM Start
	gu8DutyRate = PWM_DUTY_MIN;
}

//per 200us(check timer 0)
void MDrv_PM_StopPWM(void)
{
	gu8PWMClkCnt = 0xff;
}


void MDrv_PM_InitLED(void)
{
	gu8LEDGo = FALSE;
	gu8DoneLED = FALSE;
	MDrv_PM_StopPWM();			//Stop LED on/off (PWM Stop)
	LED_W1_LOW();
	LED_W2_LOW();
}

void MDrv_PM_InitCenterLED(void)
{
	gu8LEDGo = FALSE;
	MDrv_PM_StopPWM();			//Stop LED on/off (PWM Stop)
	LED_W2_LOW();
}

#endif

void MDrv_PM_SleepWakeUp(/*U8 u8SleepMode,*/ U8 IntSrc)
{
    U8 j;
    volatile U8 delay;

    //u8SleepMode = u8SleepMode;	//remove compile warning
    //Set BootFlag as 1
    XBYTE[0x1055] |= BIT6;
/*
    if (IntStatus & IntSrc)
    {
        POWER_ON();
    }
*/
    while(!(XBYTE[0x1007] & BIT3));    //not power good

    //power good
    XBYTE[0x1043] |= BIT7;  //ChipTop reset
    for (j=0; j <CHIPTOP_RESET_DELAY; j++)
        delay = j;
    XBYTE[0x1043] &= ~BIT7;

//20080925
    MDrv_PM_Timer0_Init();


    XBYTE[0x1050] &= ~BIT3;	//switch PD_HSYC[1:0]'s control from m4 to chip_top

#ifdef TITANIA1
    XBYTE[0x1101] &= ~(BIT5|BIT4); //CEC: Select which one is a active CEC controller.
                                   // 0x: normal CEC controller
#else	//TITANIA2
    XBYTE[0x1128] &= ~(BIT0); // 0: normal CEC controller
#endif

    XBYTE[0x1054] = 0xFF;   //return to lock sleep scheme
    XBYTE[0x1012] = 0x00;   //return to lock sleep scheme
    XBYTE[0x1013] = 0x00;   //return to lock sleep scheme

#if 1   //for Power state Management
    switch( IntSrc )
    {
        default:
        case INT_SRC_IR:
        case INT_SRC_KEYPAD:

		            gu8NextPowerStatus = M_POWER_STATE_ACTIVE;
#ifdef ENABLE_BLINKING_LED
		Blinking_LED_Start_Blink();
#else
#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
		MDrv_PM_StartFrontLED();
#else  // #if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
        MDrv_PM_Cold2Active_GpioCtrl();//20080924 GPIO Control for the U03 Revision.
#endif
#endif
            break;
        case INT_SRC_RTC:   //recording
#ifdef ENABLE_BLINKING_LED
		Blinking_LED_Ctrl_Warm();
#else
        gu8NextPowerStatus = M_POWER_STATE_WARM_STANDBY;
#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
        MDrv_PM_Cold2Warm_GpioCtrl();//20080924 GPIO Control for the U03 Revision.
#endif

#endif
        break;
    }
#endif
}



#ifndef TITANIA1
#if defined(LGE_GP_COMMERCIAL_MODEL)//#ifdef	NOT_USED_4_LGE
U8 ConvertHexVal2Str(U8 ucVal)
{
    U8 code hextable[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    return (hextable[ucVal]);
}

void MDrv_PM_UART_SendAck(U8 ucCmd, U8 ucResult, U8* pData, U8 ucDataLen, BOOLEAN bSendID)
{
    U8 i;
	U32 u32TimeOutCnt = 0;
#if 0	//decrease code size
    putchar(MakeLowerCase(ucCmd));//OK/Fail result//hjkoh 080103
#else
    putchar(ucCmd);//OK/Fail result//hjkoh 080103
#endif
    putchar(0x20);//space

    if(bSendID == TRUE)
    {
        //set id
        putchar(ConvertHexVal2Str((Mdrv_Get_TVLinkID()>>4)&0x0F));
        putchar(ConvertHexVal2Str(Mdrv_Get_TVLinkID()&0x0F));
        putchar(0x20);//space
    }
    //result
    if(ucResult)
    {
        putchar('O');
        putchar('K');
    }
    else
    {
        putchar('N');
        putchar('G');
    }

    //data
    if(ucResult == ACK_TYPE_ACK)
    {
    	for(i = 0 ; i < ucDataLen ; i++)
    	{
        	putchar(*(pData+i));
    	}
    }
	if(ucResult == ACK_TYPE_STATUS)
	{
		putchar('0');
		putchar('0');
	}
    if(bSendID == FALSE)
    {
        putchar(0x20);
    }
    putchar('x');
#if 1
	while(TRUE != TI0)
	{
		u32TimeOutCnt++;
		if(0x30000 == u32TimeOutCnt ) break;
	}
#else
	//<081121 Leehc> UART Ack 시 마지막 바이트 전송 안되는 문제 대응. 임시대응임. Mstar 로부터 정식 대응 나올때까지 만 사용함.
	putchar(0);
#endif

}
#endif

#define LINE_FEED_TIMEOUT 10000

void MDrv_PM_UART_RxData(void)
{
    U16 data timeout;
    gu8Uart0Detected = FALSE;

    for(gu8Uart0BufIndex = 0;gu8Uart0BufIndex < MAX_UART0_CMD_LENGTH;gu8Uart0BufIndex++)
    {
        timeout = 0;
        while((!RI0) && (timeout <= SEARL_INPUT_TIMEOUT))
        {   timeout++;  MDrv_PM_ClearWDT();  }

        if(RI0)
        {
			#if (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP || MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
			/* LGE, taburin : 20081122, skip first data if it's not 'k' / 'K' + 'm' / 'M'*/
			if ( gu8Uart0BufIndex == 1 )
			{
				if ( gu8Uart0CmdBuf[0] != 'k'
#ifndef CONVERT_LOWERCASE
					&& gu8Uart0CmdBuf[0] != 'K'
					&& gu8Uart0CmdBuf[0] != 'M'
#endif
					&& gu8Uart0CmdBuf[0] != 'm')
				{
					gu8Uart0BufIndex = 0;
				}
			}
			#endif
#ifdef CONVERT_LOWERCASE

			if((S0BUF >= 'A') && (S0BUF <= 'Z'))
			{
				gu8Uart0CmdBuf[gu8Uart0BufIndex] = S0BUF + 32;
			}
			else
			{
				gu8Uart0CmdBuf[gu8Uart0BufIndex] = S0BUF;
			}

#else
            gu8Uart0CmdBuf[gu8Uart0BufIndex] = S0BUF;
#endif

            RI0 = 0;
            if(gu8Uart0CmdBuf[gu8Uart0BufIndex] == 0x0d)  // carriage return
            {
                PM_DBGs(printf("# SetID=[%bd]\n",gu8SetID));
                PM_DBGs(printf("# BufIdx=[%bd]\n",gu8Uart0BufIndex));
                PM_DBGs(printf("#[0] %bX\n",gu8Uart0CmdBuf[_TVLINK_CMD1_]));
                PM_DBGs(printf("#[1] %bX\n",gu8Uart0CmdBuf[_TVLINK_CMD2_]));
                PM_DBGs(printf("#[2] %bX\n",gu8Uart0CmdBuf[_TVLINK_SPACE1_]));
                PM_DBGs(printf("#[3] %bX\n",gu8Uart0CmdBuf[_TVLINK_ID_0]));
                PM_DBGs(printf("#[4] %bX\n",gu8Uart0CmdBuf[_TVLINK_ID_1]));
                PM_DBGs(printf("#[5] %bX\n",gu8Uart0CmdBuf[_TVLINK_SPACE2_]));
                PM_DBGs(printf("#[6] %bX\n",gu8Uart0CmdBuf[_TVLINK_DATA0_0_]));
                PM_DBGs(printf("#[7] %bX\n",gu8Uart0CmdBuf[_TVLINK_DATA0_1_]));
				// < 081122 Leehc> UART cmd sending 시 LF 가 날라오면 무시함.
				timeout = 0;
				while((!RI0) && (timeout <= LINE_FEED_TIMEOUT))
				{
					timeout++;
					MDrv_PM_ClearWDT();
				}
				timeout = S0BUF;
				RI0 = 0;

                gu8Uart0Detected = TRUE;
                break;
            }
        }
        else
        {
            break;
        }
    }

}

U8 Mdrv_Get_TVLinkData0(void)
{
	U8 tv_data_1 = gu8Uart0CmdBuf[_TVLINK_DATA0_1_];
	U8 tv_data_0 = gu8Uart0CmdBuf[_TVLINK_DATA0_0_];

	if((tv_data_1 >= 'a' && tv_data_1 <= 'f')) tv_data_1 -= 0x57;
	else if((tv_data_1 >= 'A' && tv_data_1 <= 'F')) tv_data_1 -= 0x37;
	else tv_data_1 -= '0';
	if((tv_data_0 >= 'a' && tv_data_0 <= 'f')) tv_data_0 -= 0x57;
	else if((tv_data_0 >= 'A' && tv_data_0 <= 'F')) tv_data_0 -= 0x37;
	else tv_data_0 -= '0';

	return (tv_data_1 | (tv_data_0 << 4));

}

U8 Mdrv_Get_TVLinkID(void)
{
	return ((gu8Uart0CmdBuf[_TVLINK_ID_1] - '0') | ((gu8Uart0CmdBuf[_TVLINK_ID_0] - '0') << 4));
}

bool MDrv_PM_UART_ParseData(void)
{
    U8 data Data_Ready = FALSE;
    U8 data CMD_Ready = FALSE;
	U8 isStatusReturn = FALSE;
#if defined(LGE_GP_COMMERCIAL_MODEL)//#ifdef	NOT_USED_4_LGE
    U8 ucBuf[2];
#endif

    if(gu8Uart0Detected == FALSE)
        return CMD_Ready;

    if(gu8Uart0BufIndex == 8)
    {
        if((gu8Uart0CmdBuf[2]==' ')&&(gu8Uart0CmdBuf[5]==' '))
        {
            Data_Ready = TRUE;
        }
    }
    else if(gu8Uart0BufIndex == 7)
    {
        if((gu8Uart0CmdBuf[2] == ' ')&&(gu8Uart0CmdBuf[5] == ' '))
        {
            gu8Uart0CmdBuf[7] = gu8Uart0CmdBuf[6];
            gu8Uart0CmdBuf[6] = '0';
            Data_Ready = TRUE;
        }
        if((gu8Uart0CmdBuf[2]==' ')&&(gu8Uart0CmdBuf[4]==' '))
        {
            gu8Uart0CmdBuf[7] = gu8Uart0CmdBuf[6];
            gu8Uart0CmdBuf[6] = gu8Uart0CmdBuf[5];
            gu8Uart0CmdBuf[5] = gu8Uart0CmdBuf[4];
            gu8Uart0CmdBuf[4] = gu8Uart0CmdBuf[3];
            gu8Uart0CmdBuf[3] = '0';
            Data_Ready = TRUE;
        }
    }
    else if(gu8Uart0BufIndex == 6)
    {
        if((gu8Uart0CmdBuf[2] == ' ')&&(gu8Uart0CmdBuf[4] == ' '))
        {
            gu8Uart0CmdBuf[7] = gu8Uart0CmdBuf[5];
            gu8Uart0CmdBuf[6] = '0';
            gu8Uart0CmdBuf[5] = gu8Uart0CmdBuf[4];
            gu8Uart0CmdBuf[4] = gu8Uart0CmdBuf[3];
            gu8Uart0CmdBuf[3] = '0';
            Data_Ready = TRUE;
        }
    }

    if(Data_Ready)
    {
#if 0	//20080807 decrease code size
        if((MakeLowerCase(TVLINK_CMD1) == 'k') && (MakeLowerCase(TVLINK_CMD2) == 'a'))
#else

#ifdef CONVERT_LOWERCASE
		if((TVLINK_CMD1 == 'k') && (TVLINK_CMD2 == 'a'))
#else
        if(((TVLINK_CMD1 == 'k') || (TVLINK_CMD1 == 'K')) && ((TVLINK_CMD2 == 'a') || (TVLINK_CMD2 == 'A')))
#endif
#endif
        {
//            if(gu8SetID == Mdrv_Get_TVLinkID() || Mdrv_Get_TVLinkID() == 0)
            {
            	if(Mdrv_Get_TVLinkData0() == 1) CMD_Ready = TRUE;
				if(Mdrv_Get_TVLinkData0() == 0xff) isStatusReturn = TRUE;
            }
        }// Add wake up condition for hotel mode.
#ifdef CONVERT_LOWERCASE
		else if((TVLINK_CMD1 == 'm') && (TVLINK_CMD2 == 'c'))
#else
		else if(((TVLINK_CMD1 == 'm') || (TVLINK_CMD1 == 'M')) && ((TVLINK_CMD2 == 'c') || (TVLINK_CMD2 == 'C')))
#endif
		{
//			if(gu8SetID == Mdrv_Get_TVLinkID() || Mdrv_Get_TVLinkID() == 0)
            {
            	switch(Mdrv_Get_TVLinkData0())
            	{
            	    case IRKEY_POWER:
                    case IRKEY_CHANNEL_PLUS:
                    case IRKEY_CHANNEL_MINUS:
                    case IRKEY_NUM_0:
                    case IRKEY_NUM_1:
                    case IRKEY_NUM_2:
                    case IRKEY_NUM_3:
                    case IRKEY_NUM_4:
                    case IRKEY_NUM_5:
                    case IRKEY_NUM_6:
                    case IRKEY_NUM_7:
                    case IRKEY_NUM_8:
                    case IRKEY_NUM_9:
                    case IRKEY_INPUT_SOURCE:
                    case IRKEY_TV_INPUT:
#if (IR_TYPE_SEL == IR_TYPE_CUS03_DTV)
					case DSC_IRKEY_MULTIMEDIA:
					case DSC_IRKEY_PWRON:
#endif

                		CMD_Ready = TRUE;
						break;
					default:
						break;
            	}
            }
		}
    }

    gu8Uart0Detected = FALSE;
    //gu8Uart0BufIndex = 0;

#if defined(LGE_GP_COMMERCIAL_MODEL)//#ifdef	NOT_USED_4_LGE

  	if((gu8SetID == Mdrv_Get_TVLinkID() || Mdrv_Get_TVLinkID() == 0) && gu8Uart0BufIndex > 0)
    {
        //Send Ack
        ucBuf[0] = ConvertHexVal2Str((Mdrv_Get_TVLinkData0()>>4)&0x0F);
        ucBuf[1] = ConvertHexVal2Str(Mdrv_Get_TVLinkData0()&0x0F);
        if(CMD_Ready)
        {
            MDrv_PM_UART_SendAck(TVLINK_CMD2, ACK_TYPE_ACK , ucBuf, 2, TRUE);
        }
		else if(isStatusReturn)
		{
			MDrv_PM_UART_SendAck(TVLINK_CMD2, ACK_TYPE_STATUS, ucBuf, 2, TRUE);
		}
        else
        {
            MDrv_PM_UART_SendAck(TVLINK_CMD2, ACK_TYPE_NEK, ucBuf, 2, TRUE);
        }
    }
	else
#else
	if((gu8SetID != Mdrv_Get_TVLinkID() && Mdrv_Get_TVLinkID() != 0)
#endif	/* #ifdef	NOT_USED_4_LGE */

	{
		CMD_Ready = FALSE;
	}

	gu8Uart0BufIndex = 0;

    return CMD_Ready;
}

#if 0   // 20080807 decrease code size
U8 MakeLowerCase(U8 ucData)
{
    U8 ucRtn = ucData;
    if((ucData >= 'A') && (ucData <= 'Z'))
    {
        ucRtn = ucData + 32;
    }
    return ucRtn;
}

U8 Convert2StrToHexVal(U8 ucStr0, U8 ucStr1)
{
    U8 ucTemp0 = 0, ucTemp1 = 0;

    //For 1st Str
    if(IsNumber(ucStr0))
    {
        ucTemp0 = ucStr0 - '0';
    }
    else if(IsLowerCase(ucStr0))
    {
        ucTemp0 = ucStr0 - 'a';
        ucTemp0 +=10;
    }
    else if(IsUpperCase(ucStr0))
    {
        ucTemp0 = ucStr0 - 'A';
        ucTemp0 +=10;
    }
    else
    {
        return -1;//error
    }

    //For 2nd str
    if(IsNumber(ucStr1))
    {
        ucTemp1 = ucStr1 - '0';
    }
    else if(IsLowerCase(ucStr1))
    {
        ucTemp1 = ucStr1 - 'a';
        ucTemp1 +=10;
    }
    else if(IsUpperCase(ucStr1))
    {
        ucTemp1 = ucStr1 - 'A';
        ucTemp1 +=10;
    }
    else
    {
        //error
        return -1;
    }
    return ((ucTemp0 & 0x0F)|(ucTemp1<<4 & 0xF0));
}
#endif /* #if 0   // 20080807 decrease code size*/

#endif  /* #ifndef TITANIA1 */


#if 0
void MDrv_PM_ConfigCecWakeUp(void)
{
    XBYTE[0x1000] |= BIT1;    // reg_cec_en

#ifdef TITANIA1 // T1

    XBYTE[0x1100] = 0x30; // CEC clock: auto; logical address: TV; retry times: 3

    // select power down hardware CEC controller
    XBYTE[0x1101] |= (BIT4);
    XBYTE[0x1101] |= (BIT5) ;

    XBYTE[0x1102] = 0x53; // CNT1=3; CNT2 = 5;
    XBYTE[0x1103] = 0x07; // CNT3=7;

    // OPCODE0: 0x04(Image view on)
    // OPCODE1: 0x0D(Text view on)
    // OPCODE2: 0x44 0x40(Power)
    //          0x44 0x6D(Power ON Function)
    // OPCODE3: N/A
    // OPCODE4: 0x82(Active source) length = 2
    XBYTE[0x1104] = 0xB7; // Enable OP0~2 and OP4, OP4 wakeup enable
    XBYTE[0x1105] = 0x24; // Eanble OPCODE2's operand
    XBYTE[0x1106] = 0x04; // OPCODE0: Image View On
    XBYTE[0x1107] = 0x0D; // OPCODE1: Text View ON
    XBYTE[0x1108] = 0x44; // OPCODE2: User control
    XBYTE[0x110A] = 0x82; // Active source
    XBYTE[0x110D] = 0x40; // User control - Power
    XBYTE[0x110E] = 0x6D; // User control - Power ON Function

    XBYTE[0x1111] = 0x04; // CEC version 1.3a
    // It depends end-customer's vendor ID
    XBYTE[0x1119] = 0x00; // Physical address 0.0
    XBYTE[0x111A] = 0x00; // Physical address 0.0
    XBYTE[0x111B] = 0x00; // Device type: TV

    // Clear CEC status
    XBYTE[0x1114] = 0x1F; // CEC RX/TX/RF/LA/NACK status
    XBYTE[0x1115] = 0x07; // Clear RX buffer/HW CEC/wakeup status
    XBYTE[0x1115] = 0x00;

#else // T2

    XBYTE[0x1101] = 0x03; // retry times: 3
    XBYTE[0x1102] = 0x80; // [5]:CEC clock no gate; [7]: Enable CEC controller
    XBYTE[0x1103] = 0x53; // CNT1=3; CNT2 = 5;
    XBYTE[0x1104] = 0x07; // CNT3=7; logical address: TV

    // Mask CEC interrupt in standby mode
    XBYTE[0x1126] = 0x1F;

    // select power down hardware CEC controller
    XBYTE[0x1128] |= (BIT0); // Power down CEC controller select
    XBYTE[0x1107] |= (BIT4); // [4]: Sleep mode;

    // OPCODE0: 0x04(Image view on)
    // OPCODE1: 0x0D(Text view on)
    // OPCODE2: 0x44 0x40(Power)
    //          0x44 0x6D(Power ON Function)
    // OPCODE3: N/A
    // OPCODE4: 0x82(Active source) length = 2
    XBYTE[0x110E] = 0x37; // Enable OP0~2 and OP4
    XBYTE[0x110F] = 0x24; // Eanble OPCODE2's operand
    XBYTE[0x1110] = 0x04; // OPCODE0: Image View On
    XBYTE[0x1111] = 0x0D; // OPCODE1: Text View ON
    XBYTE[0x1112] = 0x44; // OPCODE2: User control
    XBYTE[0x1114] = 0x82; // OPCODE4: Active source
    XBYTE[0x1117] = 0x40; // User control - Power
    XBYTE[0x1118] = 0x6D; // User control - Power ON Function
    XBYTE[0x111B] = 0x84; // [2:0]: CEC version 1.3a; [7:3]: OP4 is broadcast message

    XBYTE[0x111C] = 0x00; // Physical address 0.0
    XBYTE[0x111D] = 0x00; // Physical address 0.0
    XBYTE[0x1129] = 0x00; // Device type: TV
    // It depends end-customer's vendor ID
    XBYTE[0x1121] = 0x01; // [2:0]: Feature abort reason - "Not in correct mode to respond"

    // Clear CEC status
    XBYTE[0x1122] = 0x7F; // Clear CEC wakeup status
    XBYTE[0x1125] = 0x1F; // Clear RX/TX/RF/LA/NACK status status
    XBYTE[0x1125] = 0x00;

#endif /* #ifdef TITANIA1 */

}
#endif

void MDrv_PM_M4Sleep(U8 u8SleepMode, U16 U16SleepSource) small
{
    U8 EnableWkSrc = 0;
#if 1   //for Power state Management
    gu8PowerStatus= M_POWER_STATE_COLD_STANDBY;
    gu8HostReady = FALSE;
#endif

#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
	ON5V_LOW();
	MDrv_PM_InitLED();
#endif

#ifdef ENABLE_BLINKING_LED
	Blinking_LED_Ctrl_Cold();
#else

#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
	MDrv_PM_Active2Cold_GpioCtrl();//20080924 GPIO Control for the U03 Revision.
#endif
#endif

//#ifdef ENABLE_AC_DET   //20080807 for AC Detection
#ifdef LGE_USE_EEPROM
  	SB_MUTE_ON();
//#endif
#endif

    XBYTE[0x1421] |= BIT6;  //SAR ADC power down
    if((U16SleepSource & KEYPAD_WAKEUP))
    {
        XBYTE[0x1421] &= ~BIT6;  //SAR ADC power on
    }

    XBYTE[0x106A] |= BIT5;	//power down unused 3.3 clock in sleep mode
    XBYTE[0x1050] |= 0x0B;//switch PD_HSYC[1:0]'s control from chip_top to m4
                            //& pwoer down PD_HSYNC

    // 1. enable Keypad wake up;
    //XBYTE[0x1001]=(1<<0);//reg_gpio_kp_en
    if(SLEEP_MODE == u8SleepMode)    //normal sleep
    {
        msIR_Initialize(MST_XTAL_CLOCK_MHZ);

        XBYTE[0x101C] |= 1<<2;   //clk_sar select ext clk
        XBYTE[0x101D] |= 1<<6;   //clk_ir select ext clk
        XBYTE[0x101F] &= 0xF0;
        XBYTE[0x101F] |= 0x0C;

        if(U16SleepSource & IR_WAKEUP)
        {
            //XBYTE[0x1000] |= 0x01;      //enable IR wake up
            EnableWkSrc |= IR_WAKEUP;
        }
        if(U16SleepSource & CEC_WAKEUP)
        {
#if !defined(CONFIG_BY_HOST)
            //MDrv_PM_ConfigCecWakeUp();		//<081029 Leehc> Micom 이 CEC 를 initialize 하는 부분을 원복함.
#endif
            //XBYTE[0x1000] |= (1<<1);     //enable CEC wake up
            EnableWkSrc |= CEC_WAKEUP;
        }
        if(U16SleepSource & GPIO5_WAKEUP)
        {
#ifndef TITANIA1
            if(RI0)
            {
                RI0 = 0;
            }
#if defined(LGE_GP_COMMERCIAL_MODEL)
        UART_TX_ENABLE();
#endif
#endif
            //XBYTE[0x1000] |= (1<<2);     //enable GIPO5 wake up
            EnableWkSrc |= GPIO5_WAKEUP;
        }
        if(U16SleepSource & GPIO6_WAKEUP)
        {
            //XBYTE[0x1000] |= (1<<3);     //enable GIPO6 wake up
            EnableWkSrc |= GPIO6_WAKEUP;
        }
        if(U16SleepSource & KEYPAD_WAKEUP)
        {
            //XBYTE[0x1000] |= (1<<4);    //enable Keypad wake up
            EnableWkSrc |= KEYPAD_WAKEUP;
        }
#ifdef ENABLE_SYNC_WAKEUP
        if(U16SleepSource & SYNC_WAKEUP)
        {
            XBYTE[0x101F] |= BIT6;   //clock sd sel

            // 1.	m4_mcu writes { 0x1016} [3:0] to determine the polarity of sync
            // e.g. reg_pol_h0 =1 to detect rising edge
            XBYTE[0x1016] |= (BIT3|BIT2|BIT1|BIT0);

            if( XBYTE[0x1017] & (BIT7|BIT6|BIT5|BIT4) )  //status before irq mask
            {
                //puts("clear-bit of irq\n");
                XBYTE[0x1009] |= (BIT3|BIT2|BIT1|BIT0);  //clear-bit of irq
                XBYTE[0x1009] &= ~(BIT3|BIT2|BIT1|BIT0);
            }

            // 3.	m4_mcu writes { 0x1008} [3:0] to unmask sync detection's interrupt
            //[3]: vsync1; [2]:hsync1; [1]: vsync0; [0]: hsync0
            XBYTE[0x1008] &= ~(BIT3|BIT2|BIT1|BIT0);

            if( XBYTE[0x1017] & (BIT3|BIT2|BIT1|BIT0) )  //status before irq mask
            {
                //puts("clear-bit of irq 1\n");
                XBYTE[0x1009] |= (BIT3|BIT2|BIT1|BIT0);  //clear-bit of irq
                XBYTE[0x1009] &= ~(BIT3|BIT2|BIT1|BIT0);
            }

            //XBYTE[0x1000] |= (1<<6);     //enable SYNC wake up
            EnableWkSrc |= SYNC_WAKEUP;
        }
#endif /* #ifdef SYNC_WAKEUP_ENABLE */

        if(U16SleepSource & RTC_WAKEUP)
        {
            //XBYTE[0x1000] |= (1<<7);//enable RTC wake up
            EnableWkSrc |= RTC_WAKEUP;
        }

        XBYTE[0x1000] |= EnableWkSrc;

#ifdef ENABLE_DVI_DET_WAKEUP
        if(U16SleepSource & DVI_DET_WAKEUP)
        {
            XBYTE[0x106A] = 0x01;   //h0035  register to switch dvi_swcka in atop
            XBYTE[0x106B] |= 0x01;  //h0035[8] enable hardware switch for {reg_dvi_swcka & swckb}}

            XBYTE[0x1043] = 0x02;   //h0021 software reset for dvi clock detection
            XBYTE[0x1043] = 0x00;   //h0021 software reset for dvi clock detection

            XBYTE[0x105C] = 0x05;   //h002E reg_cmpvalue[7:0] for dvi ctrl  //Port A
            XBYTE[0x105D] = 0xF0;   //h002E reg_cmpvalue2[15:8] for dvi ctrl

            XBYTE[0x1034] = 0xF2;   //h001A dvi_det_cmp hardware comparison for reg_dvi_rpt0
            XBYTE[0x1003] |= (1<<5);  //enable DVI det wake up
        }
        if(U16SleepSource & DVI_DET2_WAKEUP)
        {
            XBYTE[0x106A] = 0x01;   //h0035  register to switch dvi_swcka in atop
            XBYTE[0x106B] |= 0x01;  //h0035[8] enable hardware switch for {reg_dvi_swcka & swckb}}

            XBYTE[0x1043] = 0x02;   //h0021 software reset for dvi clock detection
            XBYTE[0x1043] = 0x00;   //h0021 software reset for dvi clock detection

            XBYTE[0x1066] = 0x04;  //0x10  //h0033 reg_cmpvalue_1[7:0] for dvi ctrl    //Port B
            XBYTE[0x1067] = 0xF0;   //h0033 reg_cmpvalue2_1[15:8] for dvi ctrl

            XBYTE[0x1035] = 0xF2;   //h001A dvi_det_cmp2 hardware comparison for reg_dvi_rpt1
            XBYTE[0x1003] |= (1<<6);  //enable DVI det2 wake up
        }
#endif /* #ifdef DVI_DET_WAKEUP_ENABLE */

#if 0   //before sleep  should turn on some irq by  sleep rule
        XBYTE[0x2B18] = 0x0;   // unmask all PM Sleep interrupt
#endif

        //XBYTE[0x1001] |= (BIT5);//BIT5: wakeup_rst_chip_top_en    No difference for wanting keep the saved time variable information
        //XBYTE[0x1001] &= ~(BIT6);//BIT6: wakeup_rst_51_en = 0 (start from last)  is  OK for wanting keep the saved time variable information
        XBYTE[0x1001] |= 0x20;  //normal sleep, start from last, wakeup_rst_chip_top_en

        if( (U16SleepSource & GPIO6_WAKEUP) || (U16SleepSource & GPIO5_WAKEUP) )
        {
            // 2.de-glitch
            XBYTE[0x1014]=0x0f;
            XBYTE[0x1015]=0x80;
        }


#if (IR_MODE_SEL != IR_TYPE_SWDECODE_MODE)
        if(U16SleepSource & IR_WAKEUP)
        {
            if(!(XBYTE[IR_RPT_FIFOEMPTY] & BIT1))    //IR FIFO is not empty
            {
                //msIR_ReadByte(IR_KEY);
                XBYTE[IR_FIFO_READ_PULSE] = 0x01;
                msIR_Clear_FIFO();
            }
        }
#endif

#ifdef ENABLE_DVI_SYNC_BOUND
        //if( (U16SleepSource & SYNC_WAKEUP) || (U16SleepSource & DVI_DET_WAKEUP) || (U16SleepSource & DVI_DET2_WAKEUP) )
        {
            if( ( (0 == hsync0) && (0 == vsync0) ) &&       /* no hsync and no vsync signal */
             ( ( ( (LOWER_BOUND_DVI_PORTA >= XBYTE[0x105E]) || (UPPER_BOUND_DVI_PORTA <= XBYTE[0x105E]) ) \
             && ( (LOWER_BOUND_DVI_PORTB >= XBYTE[0x1068]) || (UPPER_BOUND_DVI_PORTB <= XBYTE[0x1068]) ) )\
             || (  (MATCH_VALUE_DVI_PORTA != XBYTE[0x105F]) && (MATCH_VALUE_DVI_PORTB != XBYTE[0x1069]) ) )\
             )
            {
                //puts("power down sleep\n");
                MDrv_PM_KeepSleep();
            }
        }
#else
        //puts("power down sleep\n");
        MDrv_PM_KeepSleep();

#endif	/* #ifdef DVI_SYNC_BOUND */

    }
	//puts("exit MDrv_PM_M4Sleep\n");

}

/*
void debug_GPIO(void)
{
    U8 u8Data,u8ChangeLevel;

	//u8ChangeLevel = (U8)gu32TimerCount & 0x00000001;
	gu8WaveIndicator++;
	u8ChangeLevel = gu8WaveIndicator & 0x01;
 	if(u8ChangeLevel)
	{
    	u8Data = XBYTE[REG_GPIO_PM2_OEN];
    	XBYTE[REG_GPIO_PM2_OEN] =  u8Data & ~BIT2;
    	u8Data = XBYTE[REG_GPIO_PM2_OUT];
    	XBYTE[REG_GPIO_PM2_OUT] = u8Data | BIT2;   // Set High level
	}
	else
	{
	    u8Data = XBYTE[REG_GPIO_PM2_OEN];
    	XBYTE[REG_GPIO_PM2_OEN] =  u8Data & ~BIT2;
    	u8Data = XBYTE[REG_GPIO_PM2_OUT];
    	XBYTE[REG_GPIO_PM2_OUT] = u8Data & ~BIT2;   // Set Low level
	}
}
//*/


INT_SERVICE( EX0 )
{
    U8 data u8IntStatus;
    PM_Info_t data pm_sleep_recv;
    U8 u8Index, u8Data;

    //puts("PM: EX0\n");

    u8IntStatus = XBYTE[EX0_INT_FINAL_STATUS_0];
    if ( u8IntStatus )
    {
        XBYTE[EX0_INT_CLEAR_0] = u8IntStatus;
        XBYTE[EX0_INT_CLEAR_0] = 0;

        if (u8IntStatus & 0x01)
        {
            pm_sleep_recv.u16Data = ((U16)XBYTE[0x102f]<<8) + ((U16)XBYTE[0x102e]); //mail box 1
            pm_sleep_recv.u8Data = XBYTE[0x102d];    //mail box 0
            pm_sleep_recv.Preamble = (XBYTE[0x102c] & 0xC0)>>6; //mail box 0
            pm_sleep_recv.opcode = (XBYTE[0x102c] & 0x3F);  //mail box 0

            if( WRITE_CMD == pm_sleep_recv.opcode)
            {

                if(POWER_LED_CFG_DISABLE == gu8PowerLEDCfgDisable)   //Can't config GPIO_PM2 for Power LED
                {
                    if( REG_GPIO_PM2_OEN == pm_sleep_recv.u16Data)
                    {
                        u8Data = XBYTE[REG_GPIO_PM2_OEN];   //for keep Original BIT2(GPIO_PM2) value means can't be changed
                        XBYTE[REG_GPIO_PM2_OEN] = ((u8Data & BIT2) | (pm_sleep_recv.u8Data & ~BIT2));
                    }
                    else if( REG_GPIO_PM2_OUT == pm_sleep_recv.u16Data)
                    {
                        u8Data = XBYTE[REG_GPIO_PM2_OUT];//for keep Original BIT2(GPIO_PM2) value means can't be changed
                        XBYTE[REG_GPIO_PM2_OUT] = ((u8Data & BIT2) | (pm_sleep_recv.u8Data & ~BIT2));
                    }
                    else
                    {
                        XBYTE[pm_sleep_recv.u16Data] = pm_sleep_recv.u8Data;
                    }
                }
                else
                {
                    XBYTE[pm_sleep_recv.u16Data] = pm_sleep_recv.u8Data;
                }
                XBYTE[0x1024] = (pm_sleep_recv.Preamble<<6) | 0x3F;

            }
            else if( READ_CMD == pm_sleep_recv.opcode)
            {

                pm_sleep_recv.u8Data = XBYTE[pm_sleep_recv.u16Data];
                XBYTE[0x1025] = pm_sleep_recv.u8Data;
                XBYTE[0x1024] = (pm_sleep_recv.Preamble<<6) | 0x3F;
            }
            else if( SLEEP_CMD == pm_sleep_recv.opcode)
            {

#if 1
                gbSleepFlag = TRUE;
                gu8SleepMode= pm_sleep_recv.u8Data;
                gu16SleepSource = pm_sleep_recv.u16Data;
#else
                MDrv_PM_M4Sleep(pm_sleep_recv.u8Data, pm_sleep_recv.u16Data);
#endif
                XBYTE[0x1024] = (pm_sleep_recv.Preamble<<6) | 0x3F;
            }
            else if( CTRL_WRITE_CMD == pm_sleep_recv.opcode )
            {
                //H: Data, L: Index (is equal to Length-1)  //Mail Box 1
                u8Index = (U8)(pm_sleep_recv.u16Data & 0x00FF);
                u8Data = (U8)((pm_sleep_recv.u16Data & 0xFF00) >> 8);

                //process get data  //HOST transfer from MSB to LSB
                switch( u8Index )
                {
                    case 7:
                        gu32Received1.u8Num[0] = u8Data;  //0:MSB of U32
                        break;
                    case 6:
                        gu32Received1.u8Num[1] = u8Data;
                        break;
                    case 5:
                        gu32Received1.u8Num[2] = u8Data;
                        break;
                    case 4:
                        gu32Received1.u8Num[3] = u8Data;  //LSB of U32
                        break;
                    case 3:
                        gu32Received0.u8Num[0] = u8Data;  //0:MSB of U32
                        break;
                    case 2:
                        gu32Received0.u8Num[1] = u8Data;
                        break;
                    case 1:
                        gu32Received0.u8Num[2] = u8Data;
                        break;
                    case 0:
                        gu32Received0.u8Num[3] = u8Data;  //LSB of U32
                        break;
                    default:
                        break;
                }

                //process DDI MICOM Command
                switch( (DDI_MICOM_CMDTYPE) pm_sleep_recv.u8Data )//Mail Box 0
                {
                    case CP_WRITE_RTC_YMDHMS:
                        //implemented in HOST Side
                        break;
                    case CP_WRITE_SET_OTATIMER:
                        gs32OTAoffsetRTC = (S32)gu32Received0.u32Num;
                        break;
                    case CP_WRITE_SET_OFFTIMER:
                        gOffTimeInfo.hour   = gu32Received0.u8Num[3];   //index0
                        gOffTimeInfo.min    = gu32Received0.u8Num[2];   //index1
                        gOffTimeInfo.repeat = gu32Received0.u8Num[1];   //index2
                        break;
                    case CP_WRITE_CANCEL_OFFTIMER:
                        gOffTimeInfo.repeat = EN_TIME_OFFTIME_CANCEL;
                        break;
                    case CP_WRITE_SET_ONTIMER:
                        gOnTimeInfo.hour   = gu32Received0.u8Num[3];   //index0
                        gOnTimeInfo.min    = gu32Received0.u8Num[2];   //index1
                        gOnTimeInfo.repeat = gu32Received0.u8Num[1];   //index2
                        break;
                    case CP_WRITE_CANCEL_ONTIMER:
                        gOnTimeInfo.repeat = EN_TIME_ONTIME_CANCEL;
                        break;
                    case CP_WRITE_SET_OTA_ONTIMER:
                        gOTA_OnTimeInfo.year   = (100 * gu32Received0.u8Num[3]) + gu32Received0.u8Num[2];   //100 * index0 + index1
                        gOTA_OnTimeInfo.month  = gu32Received0.u8Num[1];   //index2
                        gOTA_OnTimeInfo.day    = gu32Received0.u8Num[0];   //index3
                        gOTA_OnTimeInfo.hour   = gu32Received1.u8Num[3];   //index4
                        gOTA_OnTimeInfo.min    = gu32Received1.u8Num[2];   //index5
    //                    gOTA_OnTimeInfo.EnaFlg  = EN_TIME_ONTIME_ONCE;   //not equal to zero when set OTA_ONTIMER
                        break;
                    case CP_WRITE_SET_OTA_ONTIMER_ENAFLG:
                        gOTA_OnTimeInfo.EnaFlg  = gu32Received0.u8Num[3];
                        break;
                    case CP_WRITE_RESERVE_TIME:
                        gReserveTimeInfo.year   = (100 * gu32Received0.u8Num[3]) + gu32Received0.u8Num[2];   //100 * index0 + index1
                        gReserveTimeInfo.month  = gu32Received0.u8Num[1];   //index2
                        gReserveTimeInfo.day    = gu32Received0.u8Num[0];   //index3
                        gReserveTimeInfo.hour   = gu32Received1.u8Num[3];   //index4
                        gReserveTimeInfo.min    = gu32Received1.u8Num[2];   //index5
                        gReserveTimeInfo.isRes  = gu32Received1.u8Num[1];   //index6
                        gReserveTimeInfo.EnaFlg  = EN_TIME_ONTIME_ONCE;     //not equal to zero when set ReserveTime
                        break;
                    case CP_WRITE_SET_RESTIME_ENAFLG:
                        gReserveTimeInfo.EnaFlg  = gu32Received0.u8Num[3];
                        break;
                    case CP_WRITE_POWER_ON_MODE:
                        gu8PowerOnMode = gu32Received0.u8Num[3];
                        break;
                    case CP_WRITE_COMM_SETID:
                        gu8SetID = gu32Received0.u8Num[3];
                        break;
                    case CP_WRITE_CHILD_LOCK:
                        gu8ChildLock = gu32Received0.u8Num[3];
                        break;
                    case CP_WRITE_KEY_LOCK:
                        gu8RMCKeyLock = gu32Received0.u8Num[3];
                        break;
                    case CP_WRITE_HOST_READY:
                        gu8HostReady = gu32Received0.u8Num[3];
                        break;
                    case CP_WRITE_AUTO_POWER_OFFON:
#ifdef	NOT_USED_4_LGE
                        gu8PowerOffMode = gu32Received0.u8Num[3];
#endif
                        gu8AutoOffOnFlag = gu32Received0.u8Num[2];
                        break;
                    case CP_WRITE_SA_PWR_LED_ONOFF:
                        gu8PowerLEDCfgDisable = gu32Received0.u8Num[3];
                        break;
                    case CP_WRITE_POWER_OFF_CANCEL:
                        gu8CancelPowerOff = gu32Received0.u8Num[3];
                        break;

                    case CP_WRITE_POWER_STATUS:
                        gu8PowerStatus = gu32Received0.u8Num[2];
                        gu8LastPowerStateReady = gu32Received0.u8Num[3];    //u8Num[3] must wait u8Num[2] ready
                        break;

                    case CP_WRITE_POWER_CONTROL:
                        gu8PowerOnOffMode = gu32Received0.u8Num[2];
#if !(MS_BOARD_TYPE_SEL == BD_S6_PDP)
                        if( M_POWER_STATE_COLD_STANDBY !=gu32Received0.u8Num[3])
                        {
                            gu8PowerStatus = gu32Received0.u8Num[3];
                        }
#else
                        if( M_POWER_STATE_COLD_STANDBY !=gu32Received0.u8Num[3])
                        {
							if ( (gu8PowerStatus != gu32Received0.u8Num[3])
							  && (M_POWER_STATE_ACTIVE == gu32Received0.u8Num[3])
							  && (gu8LEDGo != TRUE) )
							{
								gu8DoneLED =FALSE;
								MDrv_PM_StartFrontLED();

							}

                            gu8PowerStatus = gu32Received0.u8Num[3];
						}
#endif
					break;

#ifdef	NOT_USED_4_LGE
                    case CP_WRITE_POWER_MODE_CTRL:  // hotel mode
                        gu8PowerModeCtrl = gu32Received0.u8Num[3];
                        break;

                    case CP_WRITE_KEYOP_MODE_CTRL:
                        gu8KeyOpModeCtrl = gu32Received0.u8Num[3];
                        break;
#endif

                    case CP_WRITE_CEC_WKUP_STATUS: //Brian add
                        gu8CecWkupStatus = gu32Received0.u8Num[3];
                        break;

#ifdef ENABLE_STORE_HOST_SRAM_DATA   //20080819 for store host side SRAM data
                    case CP_WRITE_HOST_MEM_TO_MICOM_START:
                        gu8StartWriteBit = gu32Received0.u8Num[3];
                        break;

                    case CP_WRITE_MICOM_TO_HOST_MEM_START:
                        gu8StartReadBit = gu32Received0.u8Num[3];
                        break;

                    case CP_WRITE_HOST_MEM_TO_MICOM_DATA:
                        if(TRUE == gu8StartWriteBit)
                        {
                            gu16ptrIndex = 0;
                            gu8StartWriteBit = FALSE;
                        }
                        gu8SramData[gu16ptrIndex + 0] = gu32Received0.u8Num[3]; //index 0 //last one(real data) of 8 bytes that store to buff, sucess all of 8 bytes transaction
                        gu8SramData[gu16ptrIndex + 1] = gu32Received0.u8Num[2];
                        gu8SramData[gu16ptrIndex + 2] = gu32Received0.u8Num[1];
                        gu8SramData[gu16ptrIndex + 3] = gu32Received0.u8Num[0];
                        gu8SramData[gu16ptrIndex + 4] = gu32Received1.u8Num[3];
                        gu8SramData[gu16ptrIndex + 5] = gu32Received1.u8Num[2];
                        gu8SramData[gu16ptrIndex + 6] = gu32Received1.u8Num[1];
                        gu8SramData[gu16ptrIndex + 7] = gu32Received1.u8Num[0]; //first one(real data) of 8 bytes that store to buff

                        if(0 == u8Index)
                        {
                            gu16ptrIndex += 8;
                        }
                        break;
#endif  /* #ifdef ENABLE_STORE_HOST_SRAM_DATA */
#ifdef KEYPAD_SET_BY_HOST
                    case CP_WRITE_KEYPAD_BOUND_1:
                        PM_KP.u8ChanMinusLB = gu32Received0.u8Num[3]; //index 0
                        PM_KP.u8ChanMinusUB = gu32Received0.u8Num[2];
                        PM_KP.u8ChanPlusLB = gu32Received0.u8Num[1];
                        PM_KP.u8ChanPlusUB = gu32Received0.u8Num[0];
                        PM_KP.u8InputLB = gu32Received1.u8Num[3];
                        PM_KP.u8InputUB = gu32Received1.u8Num[2];
                        PM_KP.u8MenuLB = gu32Received1.u8Num[1];
                        PM_KP.u8MenuUB = gu32Received1.u8Num[0];
                        break;
                    case CP_WRITE_KEYPAD_BOUND_2:
                        PM_KP.u8OkLB = gu32Received0.u8Num[3]; //index 0
                        PM_KP.u8OkUB = gu32Received0.u8Num[2];
                        PM_KP.u8PowerLB = gu32Received0.u8Num[1];
                        PM_KP.u8PowerUB = gu32Received0.u8Num[0];
                        PM_KP.u8VolumeMinusLB = gu32Received1.u8Num[3];
                        PM_KP.u8VolumeMinusUB = gu32Received1.u8Num[2];
                        PM_KP.u8VolumePlusLB = gu32Received1.u8Num[1];
                        PM_KP.u8VolumePlusUB = gu32Received1.u8Num[0];
                        break;
#endif  /* #ifdef KEYPAD_SET_BY_HOST */
#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
					case CP_WRITE_DISPLAY_MODE:		// Loging LED for demode
                        gu8DemoLEDON = (BIT)gu32Received0.u8Num[3];

						if((gu8LEDGo == TRUE)&&(gu8DoneLED ==TRUE))
						MDrv_PM_InitCenterLED();
						break;
#endif
                    case CP_NO_CMD:
                    default:
                        //puts("No get any DDI MICOM Command");
                        //MDrv_PM_M4Sleep(POWERKEY_SLEEP_MODE, 0x81);
                        break;

                }
                XBYTE[0x1024] = (pm_sleep_recv.Preamble<<6) | 0x3F;


            }
            else if( CTRL_READ_CMD == pm_sleep_recv.opcode )
            {
                //H: Data, L: Index (is equal to Length-1)  //Mail Box 1
                u8Index = (U8)(pm_sleep_recv.u16Data & 0x00FF);
                //u8Data = (U8)((pm_sleep_recv.u16Data & 0xFF00) >> 8);

                //process DDI MICOM Command
                switch( (DDI_MICOM_CMDTYPE) pm_sleep_recv.u8Data )//Mail Box 0
                {
                    case CP_READ_RTC_YMDHMS:
                        //implemented in HOST Side
                        break;
                    case CP_READ_OTA_TIME:
                        gu32Send0.u32Num = (U32)gs32OTAoffsetRTC;
                        break;
                    case CP_READ_GET_OFFTIMER:
                        gu32Send0.u8Num[3] = gOffTimeInfo.hour; //index0
                        gu32Send0.u8Num[2] = gOffTimeInfo.min;  //index1
                        gu32Send0.u8Num[1] = gOffTimeInfo.repeat;   //index2
                        break;
                    case CP_READ_CANCEL_OFFTIMER:
                        gu32Send0.u8Num[3] = gOffTimeInfo.repeat;   //index0
                        break;
                    case CP_READ_GET_ONTIMER:
                        gu32Send0.u8Num[3] = gOnTimeInfo.hour; //index0
                        gu32Send0.u8Num[2] = gOnTimeInfo.min;  //index1
                        gu32Send0.u8Num[1] = gOnTimeInfo.repeat;   //index2
                        break;
                    case CP_READ_CANCEL_ONTIMER:
                        gu32Send0.u8Num[3] = gOnTimeInfo.repeat;   //index0
                        break;
                    case CP_READ_OTA_ONTIME:
                        gu32Send0.u8Num[3] = (gOTA_OnTimeInfo.year)/100;   //index0
                        gu32Send0.u8Num[2] = (gOTA_OnTimeInfo.year)%100;   //index1
                        gu32Send0.u8Num[1] = gOTA_OnTimeInfo.month;   //index2
                        gu32Send0.u8Num[0] = gOTA_OnTimeInfo.day;   //index3
                        gu32Send1.u8Num[3] = gOTA_OnTimeInfo.hour;   //index4
                        gu32Send1.u8Num[2] = gOTA_OnTimeInfo.min;   //index5
                        gu32Send1.u8Num[1] = gOTA_OnTimeInfo.EnaFlg;   //index6
                        break;
                    case CP_READ_RESERVE_TIME:
                        gu32Send0.u8Num[3] = (gReserveTimeInfo.year)/100;   //index0
                        gu32Send0.u8Num[2] = (gReserveTimeInfo.year)%100;   //index1
                        gu32Send0.u8Num[1] = gReserveTimeInfo.month;   //index2
                        gu32Send0.u8Num[0] = gReserveTimeInfo.day;   //index3
                        gu32Send1.u8Num[3] = gReserveTimeInfo.hour;   //index4
                        gu32Send1.u8Num[2] = gReserveTimeInfo.min;   //index5
                        gu32Send1.u8Num[1] = gReserveTimeInfo.isRes;   //index6
                        gu32Send1.u8Num[0] = gReserveTimeInfo.EnaFlg;   //index7
                        break;
                    case CP_READ_POWERON_MODE:
                        gu32Send0.u8Num[3] = gu8PowerOnMode;
                        break;
                    case CP_READ_MICOM_VERSION:
                        gu32Send0.u8Num[3] = MICOM_VERSION_BYTE0;
                        gu32Send0.u8Num[2] = MICOM_VERSION_BYTE1;
                        gu32Send0.u8Num[1] = MICOM_VERSION_BYTE2;
                        gu32Send0.u8Num[0] = MICOM_VERSION_BYTE3;
                        gu32Send1.u8Num[3] = MICOM_VERSION_BYTE4;
                        break;
                    case CP_READ_COMM_SETID:
                        gu32Send0.u8Num[3] = gu8SetID;
                        break;
                    case CP_READ_CHILD_LOCK:
                        gu32Send0.u8Num[3] = gu8ChildLock;
                        break;
                    case CP_READ_KEY_LOCK:
                        gu32Send0.u8Num[3] = gu8RMCKeyLock;
                        break;
                    case CP_READ_HOST_READY:
                        gu32Send0.u8Num[3] =gu8HostReady;
                        break;
#ifdef	NOT_USED_4_LGE
                    case CP_READ_POWEROFF_MODE:
                        gu32Send0.u8Num[3] = gu8PowerOffMode;
#endif
                        break;
                    case CP_READ_SA_PWR_LED_ONOFF:
                        gu32Send0.u8Num[3] = gu8PowerLEDCfgDisable;
                        break;
                    case CP_READ_POWER_OFF_CANCEL:
                        gu32Send0.u8Num[3] = gu8CancelPowerOff;
                        break;
                    case CP_READ_POWER_STATUS:
                        gu32Send0.u8Num[3] = gu8PowerStatus;
                        break;

                    case CP_READ_CEC_WAKEUP_MESSAGE:
                        gu32Send0.u8Num[3] = gu8CecWkupMessage;
                        break;

#ifdef	NOT_USED_4_LGE
                    case CP_READ_POWER_MODE_CTRL:   // hotel mode
                        gu32Send0.u8Num[3] = gu8PowerModeCtrl;
                        break;

                    case CP_READ_KEYOP_MODE_CTRL:
                        gu32Send0.u8Num[3] = gu8KeyOpModeCtrl;
                        break;
#endif
                    case CP_READ_CEC_WKUP_STATUS: //Brian add
                        gu32Send0.u8Num[3] = gu8CecWkupStatus;
                        break;

#ifdef ENABLE_STORE_HOST_SRAM_DATA   //20080819 for store host side SRAM data
                    case CP_READ_MICOM_TO_HOST_MEM_DATA:
                        if(TRUE == gu8StartReadBit)
                        {
                            gu16ptrIndex = 0;
                            gu8StartReadBit = FALSE;
                        }
                        gu32Send0.u8Num[3] = gu8SramData[gu16ptrIndex + 0]; //index 0 //last one(reall data) of 8 bytes that read to host, sucess all of 8 bytes transaction
                        gu32Send0.u8Num[2] = gu8SramData[gu16ptrIndex + 1];
                        gu32Send0.u8Num[1] = gu8SramData[gu16ptrIndex + 2];
                        gu32Send0.u8Num[0] = gu8SramData[gu16ptrIndex + 3];
                        gu32Send1.u8Num[3] = gu8SramData[gu16ptrIndex + 4];
                        gu32Send1.u8Num[2] = gu8SramData[gu16ptrIndex + 5];
                        gu32Send1.u8Num[1] = gu8SramData[gu16ptrIndex + 6];
                        gu32Send1.u8Num[0] = gu8SramData[gu16ptrIndex + 7]; //first one(reall data) of 8 bytes that read to host

                        if(0 == u8Index)
                        {
                            gu16ptrIndex += 8;
                        }
                        break;
#endif  /* #ifdef ENABLE_STORE_HOST_SRAM_DATA */

                    case CP_NO_CMD:
                    default:
                        //puts("No get any DDI MICOM Command");
                        break;

                }

                //process send data  //return data to _u8RetData
                switch( u8Index )
                {
                    case 7:
                        XBYTE[0x1025]  = gu32Send1.u8Num[0];  //0:MSB of U32
                        break;
                    case 6:
                        XBYTE[0x1025]  = gu32Send1.u8Num[1];
                        break;
                    case 5:
                        XBYTE[0x1025]  = gu32Send1.u8Num[2];
                        break;
                    case 4:
                        XBYTE[0x1025]  = gu32Send1.u8Num[3];  //LSB of U32
                        break;
                    case 3:
                        XBYTE[0x1025]  = gu32Send0.u8Num[0];  //0:MSB of U32
                        break;
                    case 2:
                        XBYTE[0x1025]  = gu32Send0.u8Num[1];
                        break;
                    case 1:
                        XBYTE[0x1025]  = gu32Send0.u8Num[2];
                        break;
                    case 0:
                        XBYTE[0x1025]  = gu32Send0.u8Num[3];  //LSB of U32
                        break;
                    default:
                        //XBYTE[0x1025] = 0xFF;
                        XBYTE[0x1024] = (pm_sleep_recv.Preamble<<6) | 0x3F;

                        //MDrv_PM_M4Sleep(POWERKEY_SLEEP_MODE, 0x81);
                        break;
                }
                XBYTE[0x1024] = (pm_sleep_recv.Preamble<<6) | 0x3F;

            }
            else
            {
                //MDrv_PM_M4Sleep(POWERKEY_SLEEP_MODE, 0x81);
            }
            //puts("EX0 1\n");
        }

        if (u8IntStatus & 0x02)
        {
            //MailBoxIndexReceived = 1;
        }

        if (u8IntStatus & 0x04)
        {
            //MailBoxIndexReceived = 2;
        }

        if (u8IntStatus & 0x08)
        {
            //MailBoxIndexReceived = 3;
        }
    }
    //TBD: IR INT handle here

}

INT_SERVICE( EX1 )
{
    U8 data FinalStatus0;
    U8 data IntStatus;
    U8 data key1, key2;
#if ( MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	U8 data diff1, diff2;
#endif
#ifdef ENABLE_SYNC_WAKEUP
    U8 data u8sdClk, u8RstVal;
#endif
    U8 j;
    volatile U8 delay;
    U8 data u8RepeatKey1 = 0;
    U8 data u8RepeatKey2 = 0;
	U8 Send_MBOX2 = 0;
    U8 u8temp1, u8temp2;

#if 1   //for Power state Management
    gu8PowerStatus = M_POWER_STATE_BOOT;
#endif

//    intr_ex1 = 1;

    //puts("PM: EX1\n");

    FinalStatus0 = XBYTE[0x2b28];		/*Finial interrupt status for IRQ*/
    //printf("FinalStatus0 = %bx\n", FinalStatus0);

////////////////////////////////////////////////////////////////////////////////////////////
    //RTC bit 7
    if (FinalStatus0 & (1<<7))
    {
        /* start of standby mode start */
        IntStatus = XBYTE[0x1022];

        if( BIT4 == (XBYTE[0x1002] & BIT4) )   // oen = 1 i.e. not in sleep
        {
            //TODO: do nothing (or return alarm event by mail box)!!!!!
            //puts("RTC not in sleep\n");
            MDrv_RTC_ClearInterrupt();

			/* only for test */
//            MDrv_PM_M4Sleep(POWERKEY_SLEEP_MODE, 0x81);
        }
        else if( BIT4 != (XBYTE[0x1002] & BIT4) )   // oen = 0 i.e. in sleep
        {
#if 0
            if (IntStatus & INT_SRC_RTC)
            {
                POWER_ON();
#if (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
                /*taburin : 20081103, cotrol RL_ON(PM0) for saturn5*/
                delay1ms(150);
                RL_ON();
#endif
            }
#endif

            if (TRUE == gOTA_OnTimeInfo.EnaFlg)
            {
                gu8PowerOnMode = M_POWER_ON_BY_OTA;
            }
            else if ( (TRUE ==gReserveTimeInfo.EnaFlg) && (FALSE == gReserveTimeInfo.isRes) ) //isRes = FALSE means ResTimer
            {
                gu8PowerOnMode = M_POWER_ON_BY_RESERVE;
            }
            else if ( (TRUE ==gReserveTimeInfo.EnaFlg) && (TRUE == gReserveTimeInfo.isRes) )//isRes = TRUE means Remind Timer
            {
                gu8PowerOnMode = M_POWER_ON_BY_REMIND;
     		}
            else
            {
                gu8PowerOnMode = M_POWER_ON_BY_ON_TIMER;
            }

#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
		XBYTE[0x1421] &= ~BIT6;  //<081105 Leehc> AC detect 시 Mips 를 리셋하는 과정에서 ADC Power down 되었던 것을 살림.
			MDrv_PM_PowerOn();
			if(gu8PowerOnMode == M_POWER_ON_BY_OTA || gu8PowerOnMode == M_POWER_ON_BY_RESERVE)
            	MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_RTC);
			else
				MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_IR);
            MDrv_RTC_ClearInterrupt();
#endif

#ifdef ENABLE_NEW_AC_DET	// Acdetect wakeup 후 Fullreset
		if(TRUE == gu8ACisDetectedFlg)
		{
			XBYTE[0x1048] = 0xFF;
			XBYTE[0x1049] = 0xFF;
			while(1);
		}
#endif

#if 1   //20080715
            if(TRUE == gu8AutoOffOnFlag)
            {
                gu8PowerOnMode = M_POWER_ON_BY_REMOTE_KEY;
                gu8AutoOffOnFlag = FALSE;
#if 1   //20080910  for factory reset purpose
                XBYTE[0x1048] = 0xFF;
                XBYTE[0x1049] = 0xFF;
                while(1);   //wait for WDT occur
#endif
            }
            else
            {
#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
			    XBYTE[0x1421] &= ~BIT6;  //<081105 Leehc> AC detect 시 Mips 를 리셋하는 과정에서 ADC Power down 되었던 것을 살림.
				MDrv_PM_PowerOn();
            	MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_RTC);
            	MDrv_RTC_ClearInterrupt();
#endif

#if 0
                if (TRUE == gOTA_OnTimeInfo.EnaFlg)
                {
                    gu8PowerOnMode = M_POWER_ON_BY_OTA;
                }
                else if ( (TRUE ==gReserveTimeInfo.EnaFlg) && (FALSE == gReserveTimeInfo.isRes) ) //isRes = FALSE means ResTimer
                {
                    gu8PowerOnMode = M_POWER_ON_BY_RESERVE;
                }
                else if ( (TRUE ==gReserveTimeInfo.EnaFlg) && (TRUE == gReserveTimeInfo.isRes) )//isRes = TRUE means Remind Timer
                {
                    gu8PowerOnMode = M_POWER_ON_BY_REMIND;

         		}
//<081122 Leehc> AC detect 시 All Chip reset 하므로 불필요한 코드임.
#if 0
#ifdef ENABLE_NEW_AC_DET
                else if (TRUE == gu8ACisDetectedFlg)
                {
#if ( defined(METHOD_RE_INIT_RTC) && !defined(METHOD_SYNC_1SEC_BOUNDARY) )
                    MDrv_RTC_SetCounter(gu32SaveSystemTime+ MICOM_RESET_SLEEP_TIME);
#endif
                    gu8ACisDetectedFlg = FALSE;
                    gu8PowerOnMode = M_POWER_ON_BY_UNKNOWN;
                }
#endif
#endif
                else
                {
                    gu8PowerOnMode = M_POWER_ON_BY_ON_TIMER;
                }
#endif
            }
#endif
            //puts("RTC power good ChipTop reset\n");
        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */

    }

////////////////////////////////////////////////////////////////////////////////////////////
    //DVI det bit 6
#ifdef ENABLE_DVI_DET_WAKEUP
    if (FinalStatus0 & (1<<6))
    {
        /* start of standby mode start */
        IntStatus = XBYTE[0x1022];
#if 0   //bringup test
            XBYTE[0x1002]=0xff; //power up
            XBYTE[0x1003]=0x03;
            //while(!(XBYTE[0x1007] & BIT3));    //not power good

            {//power good
                XBYTE[0x1043] |= BIT7;  //ChipTop reset
                for (j=0; j <10000; j++);
                    delay = j;
                //printf("chip_top reset\n");
                XBYTE[0x1043] &= ~BIT7;
            }

#endif
        if( BIT4 == (XBYTE[0x1002] & BIT4) )   // oen = 1 i.e. not in sleep
        {
            //do nothing
            //puts("not in sleep 0\n");
            //puts("not in sleep 1\n");
        }
        else if( BIT4 != (XBYTE[0x1002] & BIT4) )   // oen = 0 i.e. in sleep
        {
			MDrv_PM_PowerOn();
            MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_DVI);
        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */

            //puts("not in sleep 2\n");
    }

#endif /* #ifdef ENABLE_DVI_DET_WAKEUP */
////////////////////////////////////////////////////////////////////////////////////////////
    //sync det bit 5
#ifdef ENABLE_SYNC_WAKEUP
    if (FinalStatus0 & (1<<5))
    {
        /* start of standby mode start */
        IntStatus = XBYTE[0x1022];
#ifdef TITANIA1
        PM_DBG1(printf("0x100A = %bx\n", XBYTE[0x100A]);)	//test for check width & line count
        PM_DBG1(printf("0x100C = %bx\n", XBYTE[0x100C]);)
#else//(TITANIA2)
        PM_DBG1(printf("0x1010 = %bx\n", XBYTE[0x1010]);)	//test for check width & line count
        PM_DBG1(printf("0x1012 = %bx\n", XBYTE[0x1012]);)
#endif
        /* backup previous sync clock setting */
        u8sdClk = XBYTE[0x101F];
        u8RstVal = XBYTE[0x1001];

        if(u8sdClk & BIT6)  //previous is external clock
        {
#ifdef TITANIA1
            hsync0 = XBYTE[0x100A]; //check width & line count
            vsync0 = XBYTE[0x100C];
#else//(TITANIA2)
            hsync0 = XBYTE[0x1010]; //check width & line count
            vsync0 = XBYTE[0x1012];
#endif
            XBYTE[0x1016] |= BIT4;
            XBYTE[0x1016] &= ~BIT4;

            XBYTE[0x101F] = u8sdClk;
#ifdef TITANIA1
            PM_DBG1(printf("hsync0 0x100A = %bx\n", hsync0);)	//test for check width & line count
            PM_DBG1(printf("vsync0 0x100C = %bx\n", vsync0);)
#else//(TITANIA2)
            PM_DBG1(printf("hsync0 0x1010 = %bx\n", hsync0);)	//test for check width & line count
            PM_DBG1(printf("vsync0 0x1012 = %bx\n", vsync0);)
#endif
        }
        else
        {
            XBYTE[0x101F] |= BIT6;   //clock sd sel to ext clock
#ifdef TITANIA1
            easy_hsync0 = XBYTE[0x100A]; //check width & line count
            easy_vsync0 = XBYTE[0x100C];
#else//(TITANIA2)
            easy_hsync0 = XBYTE[0x1010]; //check width & line count
            easy_vsync0 = XBYTE[0x1012];
#endif
            XBYTE[0x1016] |= BIT4;
            XBYTE[0x1016] &= ~BIT4;

            XBYTE[0x104C]=0x8e;    //key to turn off ext crystal
            XBYTE[0x104D]=0x9f;
            XBYTE[0x106C]=0x8e;    //key to turn off ext crystal
            XBYTE[0x106D]=0x9f;
            XBYTE[0x101F] = u8sdClk;
#ifdef TITANIA1
            PM_DBG1(printf("easy_hsync0 0x100A = %bx\n", easy_hsync0);)	//test for check width & line count
            PM_DBG1(printf("easy_vsync0 0x100C = %bx\n", easy_vsync0);)
#else//(TITANIA2)
            PM_DBG1(printf("easy_hsync0 0x1010 = %bx\n", easy_hsync0);)	//test for check width & line count
            PM_DBG1(printf("easy_vsync0 0x1012 = %bx\n", easy_vsync0);)
#endif
        }

        if( BIT4 == (XBYTE[0x1002] & BIT4) )   // oen = 1 i.e. not in sleep
        {
            //do nothing
            //puts("sync not in sleep\n");
            MDrv_PM_ClearSyncDectInterrupt();
        }
        else if( BIT4 != (XBYTE[0x1002] & BIT4) )   // oen = 0 i.e. in sleep
        {
			MDrv_PM_PowerOn();
            MDrv_PM_ClearSyncDectInterrupt();
            MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_SYNC);
        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */

    }
#endif  /* #ifdef ENABLE_SYNC_WAKEUP */
////////////////////////////////////////////////////////////////////////////////////////////
    //Keypad bit 4
    if (FinalStatus0 & (1<<4))
    {
        /* start of standby mode start */
        IntStatus = XBYTE[0x1022];
#if ( MS_BOARD_TYPE_SEL == BD_S6_PDP )
        for( j = 0; j<= 100; j++)
                 delay = j;

        key1 = XBYTE[0x1418];
        key2 = XBYTE[0x141A];
#else
		{
			U8 tKey1 , tKey2;
			for( j = 0; j<= 100; j++) delay = j;
			tKey1 = XBYTE[0x1418];
			tKey2 = XBYTE[0x141A];
			for(j = 0 ; j < 10 ; )
			{
				key1 = XBYTE[0x1418];
				key2 = XBYTE[0x141A];
	/* taburin : 20090120, 로컬 연속키 인식시 리플로 인해 key value 다르게 읽혀질 경우
						연속키 처리 않되어, +-2까지는 동일 키로 처리하도록 수정.*/
#if ( MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
                if ( key1 > tKey1 )
                    diff1 = key1 - tKey1;
                else
                    diff1 = tKey1 - key1;

                if(key2>tKey2)
                    diff2 = key2 - tKey2;
                else
                    diff2 = tKey2 - key2;

                if( diff1<3 && diff2<3 ) j++;
				else j = 0;
#else
				if(key1 == tKey1 && key2 == tKey2) j++;
				else j = 0;
#endif
				tKey1 = key1;
				tKey2 = key2;
			}
		}
#endif

        if( BIT4 == (XBYTE[0x1002] & BIT4) )   // oen = 1 i.e. not in sleep
        {
            //TODO: do nothing (or return Keypad key value by mail box)!!!!!
            if( TRUE == gu8HostReady)//Host is ready for Local Key
            {
                // Conversion key code
#ifdef KEYPAD_SET_BY_HOST
                if((key1 >= PM_KP.u8OkLB) && (key1 <= PM_KP.u8OkUB))
                    key1 = OK_KEYCODE;
                else if((key1 >= PM_KP.u8MenuLB) && (key1 <= PM_KP.u8MenuUB))
                    key1 = MENU_KEYCODE;
                else if((key1 >= PM_KP.u8InputLB) && (key1 <= PM_KP.u8InputUB))
                    key1 = INPUT_KEYCODE;
                else if((key1 >= PM_KP.u8PowerLB) && (key1 <= PM_KP.u8PowerUB))
                    key1 = POWER_KEYCODE;
                else
                    key1 = UNKNOWN_KEYCODE;

                if((key2 >= PM_KP.u8ChanPlusLB) && (key2 <= PM_KP.u8ChanPlusUB))
                    key2 = CH_PLUS_KEYCODE;
                else if((key2 >= PM_KP.u8ChanMinusLB) && (key2 <= PM_KP.u8ChanMinusUB))
                    key2 = CH_MINUS_KEYCODE;
                else if((key2 >= PM_KP.u8VolumePlusLB) && (key2 <= PM_KP.u8VolumePlusUB))
                    key2 = VOL_PLUS_KEYCODE;
                else if((key2 >= PM_KP.u8VolumeMinusLB) && (key2 <= PM_KP.u8VolumeMinusUB))
                    key2 = VOL_MINUS_KEYCODE;
                else
                    key2 = UNKNOWN_KEYCODE;
#else


#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
                if((key1 >= OK_LOBOUND) && (key1 <= OK_UPBOUND))
                    key1 = OK_KEYCODE;
                else if((key1 >= MENU_LOBOUND) && (key1 <= MENU_UPBOUND))
                    key1 = MENU_KEYCODE;
                else if((key1 >= INPUT_LOBOUND) && (key1 <= INPUT_UPBOUND))
                    key1 = INPUT_KEYCODE;
                else if((key1 >= POWER_LOBOUND) && (key1 <= POWER_UPBOUND))
                    key1 = POWER_KEYCODE;
                else
                    key1 = UNKNOWN_KEYCODE;

                if((key2 >= CH_PLUS_LOBOUND) && (key2 <= CH_PLUS_UPBOUND))
                    key2 = CH_PLUS_KEYCODE;
                else if((key2 >= CH_MINUS_LOBOUND) && (key2 <= CH_MINUS_UPBOUND))
                    key2 = CH_MINUS_KEYCODE;
                else if((key2 >= VOL_PLUS_LOBOUND) && (key2 <= VOL_PLUS_UPBOUND))
                    key2 = VOL_PLUS_KEYCODE;
                else if((key2 >= VOL_MINUS_LOBOUND) && (key2 <= VOL_MINUS_UPBOUND))
                    key2 = VOL_MINUS_KEYCODE;
                else
                    key2 = UNKNOWN_KEYCODE;
#else			//PDP only
                if((key1 >= NOT_USED_LOBOUND) && (key1 <= NOT_USED_UPBOUND))
                    key1 = UNKNOWN_KEYCODE;//CH_MINUS_KEYCODE;
                else if((key1 >= SW3_CH_MINUS_LOBOUND) && (key1 <= SW3_CH_MINUS_UPBOUND))
                    key1 = CH_MINUS_KEYCODE;//INPUT_KEYCODE;
                else if((key1 >= SW2_INPUT_LOBOUND) && (key1 <= SW2_INPUT_UPBOUND))
                    key1 = INPUT_KEYCODE;//CH_PLUS_KEYCODE;
                else if((key1 >= SW1_CH_PLUS_LOBOUND) && (key1 <= SW1_CH_PLUS_UPBOUND))
                    key1 = CH_PLUS_KEYCODE;// pdp have no power key..POWER_KEYCODE;
                else
                    key1 = UNKNOWN_KEYCODE;

                if((key2 >= SW7_OK_LOBOUND) && (key2 <= SW7_OK_UPBOUND))
                    key2 = OK_KEYCODE;
                else if((key2 >= SW6_MENU_LOBOUND) && (key2 <= SW6_MENU_UPBOUND))
                    key2 = MENU_KEYCODE;
                else if((key2 >= SW5_VOL_PLUS_LOBOUND) && (key2 <= SW5_VOL_PLUS_UPBOUND))
                    key2 = VOL_PLUS_KEYCODE;
                else if((key2 >= SW4_VOL_MINUS_LOBOUND) && (key2 <= SW4_VOL_MINUS_UPBOUND))
                    key2 = VOL_MINUS_KEYCODE;
                else
                    key2 = UNKNOWN_KEYCODE;
#endif  // #if !(MS_BOARD_TYPE_SEL == BD_S6_PDP)

#endif
				// <080923 Leehc> AutoDemo Micom 대응
				if(key1 == MENU_KEYCODE && key2 == VOL_MINUS_KEYCODE)
				{
					key1 = AUTODEMO_KEYCODE;
					key2 = UNKNOWN_KEYCODE;
				}
                gu32CurrentKeyTime = gu32TimerCount;
                if(((gu32CurrentKeyTime - gu32PreviousKeyTime) >= KEY_REPEAT) &&
                    ((gu32CurrentKeyTime - gu32PreviousKeyTime) <= KEY_BOUNCE))
                {
                	if(gu32AccKeyTime < REPEAT_RECOG_TIME )
                		gu32AccKeyTime += (gu32CurrentKeyTime - gu32PreviousKeyTime);

                    if((key1 == gu8OldKey1) && (key1 != UNKNOWN_KEYCODE))
                    {
                        u8RepeatKey1 = 1;
                        Send_MBOX2 = TRUE;
                    }
                    if((key2 == gu8OldKey2) && (key2 != UNKNOWN_KEYCODE))
                    {
                        u8RepeatKey2 = 1;
                        Send_MBOX2 = TRUE;
                    }
                }
                else if((gu32CurrentKeyTime - gu32PreviousKeyTime) > KEY_BOUNCE)
                {
                	gu32AccKeyTime = 0;
                    Send_MBOX2 = TRUE;
                    u8RepeatKey1 = 0;
                    u8RepeatKey2 = 0;
                }

                if(Send_MBOX2)
                {
                    gu32PreviousKeyTime = gu32CurrentKeyTime;
                    gu8OldKey1 = key1;
                    gu8OldKey2 = key2;
                    if((key2 == UNKNOWN_KEYCODE) && (key1 != UNKNOWN_KEYCODE))
                    {
                    	if(gu32AccKeyTime >= REPEAT_RECOG_TIME || u8RepeatKey1 == 0)
                    	{
                        	XBYTE[0x1029] = u8RepeatKey1;   //using MBOX 2
                        	XBYTE[0x1028] = key1;           //using MBOX 2
                    	}
                    }
                    else if((key1 == UNKNOWN_KEYCODE) && (key2 != UNKNOWN_KEYCODE))
                    {
                    	if(gu32AccKeyTime >= REPEAT_RECOG_TIME || u8RepeatKey2 == 0)
                    	{
                        	XBYTE[0x1029] = u8RepeatKey2;   //using MBOX 2
                        	XBYTE[0x1028] = key2;           //using MBOX 2

                    	}
                    }
                }
                MDrv_PM_ClearSarInterrupt();

                u8temp1 = XBYTE[0x140A];
                u8temp2 = XBYTE[0x140E];
                XBYTE[0x140A] = 0;
                XBYTE[0x140E] = 0;
                for (j=0; j <CHIPTOP_RESET_DELAY; j++)
                    delay = j;
                XBYTE[0x140A] = u8temp1;
                XBYTE[0x140E] = u8temp2;

			}
            else
            {
                //don't send Local Key to Host
            	MDrv_PM_ClearSarInterrupt();
            }
            //puts("Keypad not in sleep\n");
            //XBYTE[0x1022] = IntStatus;/* end of standby mode */
        }
        else if( BIT4 != (XBYTE[0x1002] & BIT4) )   // oen = 0 i.e. in sleep
        {
           for( j = 0; j<= 100; j++)
                     delay = j;
            key1 = XBYTE[0x1418];
            key2 = XBYTE[0x141A];

            if( TRUE == gu8ChildLock && TRUE == gu8RMCKeyLock )
            {
#if ((MS_BOARD_TYPE_SEL != BD_S6_ATSC_GP)&&(MS_BOARD_TYPE_SEL != BD_S6_PDP))
                //only LOCAL POWER KEY can be used to wake up
#ifdef KEYPAD_SET_BY_HOST
 #if(KEYPAD_TYPE_SEL == KEYPAD_TYPE_CUSTMOER)
                if((key1 >= PM_KP.u8PowerLB) && (key1 <= PM_KP.u8PowerUB) )
 #elif(KEYPAD_TYPE_SEL == KEYPAD_TYPE_ORIG)
                if((key2 >= PM_KP.u8PowerLB) && (key2 <= PM_KP.u8PowerUB) )
 #endif
#else
 #if(KEYPAD_TYPE_SEL == KEYPAD_TYPE_CUSTMOER)
                if((key1 <= LK_POWER_UB) && (key1 >= LK_POWER_LB) )
 #elif(KEYPAD_TYPE_SEL == KEYPAD_TYPE_ORIG)
                if((key2 <= LK_POWER_UB) && (key2 >= LK_POWER_LB) )
 #endif
#endif
                {
					MDrv_PM_PowerOn();
                    MDrv_PM_ClearSarInterrupt();//ClearSarInterrupt();
                    MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_KEYPAD);
                    gu8PowerOnMode = M_POWER_ON_BY_LOCAL_KEY;
                    //XBYTE[0x1022] = IntStatus;/* end of standby mode */
                }
                else
#endif
                {
                    MDrv_PM_ClearSarInterrupt2();
                    //not wakeup key, keep sleep
                    MDrv_PM_KeepSleep();
                    //XBYTE[0x1022] = IntStatus;/* end of standby mode */
                }
            }
            else if(TRUE == gu8ChildLock && FALSE == gu8RMCKeyLock)
            {
                MDrv_PM_ClearSarInterrupt2();
                //not wakeup key, keep sleep
                MDrv_PM_KeepSleep();
                //XBYTE[0x1022] = IntStatus;/* end of standby mode */
            }
            else    // no gu8ChildLock
            {
#ifdef KEYPAD_SET_BY_HOST
 #if(KEYPAD_TYPE_SEL == KEYPAD_TYPE_CUSTMOER)
                if( ((key1 >= PM_KP.u8PowerLB) && (key1 <= PM_KP.u8PowerUB) ) ||\
                    ((key1 >= PM_KP.u8InputLB) && (key1 <= PM_KP.u8InputUB) ) ||\
                    ((key2 >= PM_KP.u8ChanPlusLB) && (key2 <= PM_KP.u8ChanPlusUB) ) ||\
                    ((key2 >= PM_KP.u8ChanMinusLB) && (key2 <= PM_KP.u8ChanMinusUB)) )
 #elif(KEYPAD_TYPE_SEL == KEYPAD_TYPE_ORIG)
                if( ((key2 >= PM_KP.u8PowerLB) && (key2 <= PM_KP.u8PowerUB) ) ||\
                    ((key2 >= PM_KP.u8InputLB) && (key2 <= PM_KP.u8InputUB) ) ||\
                    ((key2 >= PM_KP.u8ChanPlusLB) && (key2 <= PM_KP.u8ChanPlusUB) ) ||\
                    ((key1 >= PM_KP.u8ChanMinusLB) && (key1 <= PM_KP.u8ChanMinusUB)) )
 #endif
#else
 #if(KEYPAD_TYPE_SEL == KEYPAD_TYPE_CUSTMOER)
 	#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
                if( ((key1 <= LK_POWER_UB) && (key1 >= LK_POWER_LB) ) ||\
                    ((key1 <= LK_INPUT_UB) && (key1 >= LK_INPUT_LB) ) ||\
                    ((key2 <= LK_CH_PLUS_UB) && (key2 >= LK_CH_PLUS_LB) ) ||\
                    ((key2 <= LK_CH_MINUS_UB) && (key2 >= LK_CH_MINUS_LB)) )
    #else	//PDP only	input,CH+,CH-

	                if(((key1 <= SW2_INPUT_UPBOUND) && (key1 >= SW2_INPUT_LOBOUND) ) ||\
                    ((key1 <= SW1_CH_PLUS_UPBOUND) && (key1 >= SW1_CH_PLUS_LOBOUND) ) ||\
                    ((key1 <= SW3_CH_MINUS_UPBOUND) && (key1 >= SW3_CH_MINUS_LOBOUND)) )
	#endif
 #elif(KEYPAD_TYPE_SEL == KEYPAD_TYPE_ORIG)
                if( ((key2 <= LK_POWER_UB) && (key2 >= LK_POWER_LB) ) ||\
                    ((key2 <= LK_INPUT_UB) && (key2 >= LK_INPUT_LB) ) ||\
                    ((key2 <= LK_CH_PLUS_UB) && (key2 >= LK_CH_PLUS_LB) ) ||\
                    ((key1 <= LK_CH_MINUS_UB) && (key1 >= LK_CH_MINUS_LB)) )
 #endif
#endif
                {
                    MDrv_PM_PowerOn();
                    MDrv_PM_ClearSarInterrupt();//ClearSarInterrupt();
                    MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_KEYPAD);
                    gu8PowerOnMode = M_POWER_ON_BY_LOCAL_KEY;
                    //XBYTE[0x1022] = IntStatus;/* end of standby mode */
                }
                else
                {
                    MDrv_PM_ClearSarInterrupt2();
                    //not wakeup key, keep sleep
                    MDrv_PM_KeepSleep();
                    //XBYTE[0x1022] = IntStatus;/* end of standby mode */
                }
            }

        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */

    }
////////////////////////////////////////////////////////////////////////////////////////////
    //GPIO6 bit 3
    if (FinalStatus0 & 0x08)
    {
#if !defined(LGE_GP_COMMERCIAL_MODEL)
        /* start of standby mode start */
        IntStatus = XBYTE[0x1022];

        if( BIT4 == (XBYTE[0x1002] & BIT4) )   // oen = 1 i.e. not in sleep
        {
            //do nothing
            //puts("GPIO6 not in sleep\n");
        }
        else if( BIT4 != (XBYTE[0x1002] & BIT4) )   // oen = 0 i.e. in sleep
        {
            MDrv_PM_PowerOn();
            MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_GPIO6);
        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */
#endif
    }

////////////////////////////////////////////////////////////////////////////////////////////
    //GPIO5 bit 2
    if (FinalStatus0 & (1<<2))
    {
        /* start of standby mode start */
        IntStatus = XBYTE[0x1022];

        if( BIT4 == (XBYTE[0x1002] & BIT4) )   // oen = 1 i.e. not in sleep
        {
            //do nothing
            //puts("GPIO5 not in sleep\n");
        }
        else if( BIT4 != (XBYTE[0x1002] & BIT4) )   // oen = 0 i.e. in sleep
        {
#ifndef TITANIA1    // for TITANIA2
            // UART wakeup (TVLinkTuner communication protocol)
            MDrv_PM_UART_RxData();

            if(MDrv_PM_UART_ParseData())
            {
#if defined(LGE_GP_COMMERCIAL_MODEL)
#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
                UART_TX_DISABLE();
#endif
#endif
                MDrv_PM_PowerOn();
                MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_GPIO5);
                gu8PowerOnMode = M_POWER_ON_BY_RS232C;
            }
            else
            {
                PM_DBGs(puts("keep sleep"));
			// <081113 Leehc> Active 에서 AC deep 또는 채터링 테스트 도중 Standby 로 가는 문제 개선
			// 불필요하게 시스템 State 를 Sleep 로 보내는 코드로 판단되어 막음.
			// 엠스타에서 막아도 관계없다는 confirm 을 막았지만 왜 이부분으로 진입하는지 무슨 목적으로 해당
			// 코드가 삽입되었는지에 대한 자세한 검토를 의뢰중임.
			#if 0
                MDrv_PM_KeepSleep();
			#endif
            }
#else	// for TITANIA1
			MDrv_PM_PowerOn();
            MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_GPIO5);
#endif	/* #ifndef TITANIA1 */
        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */

    }

////////////////////////////////////////////////////////////////////////////////////////////
    //CEC bit 1
    if (FinalStatus0 & (1<<1))
    {
        /* start of standby mode start */
        IntStatus = XBYTE[0x1022];

        if( BIT4 == (XBYTE[0x1002] & BIT4) )   // oen = 1 i.e. not in sleep
        {
            //do nothing
            //puts("CEC not in sleep\n");
            MDrv_PM_ClearCECInterrupt();
        }
        else if( BIT4 != (XBYTE[0x1002] & BIT4) )   // oen = 0 i.e. in sleep
        {
			MDrv_PM_PowerOn();
            //ToDo: 1. check the wakeup CEC opcode will be overwrite by newest one or not ?
            //      2. before or after clear CEC interrupt will lead to different wakeup opcode or not ?
            if(XBYTE[REG_CEC_HW_EVENT_STS] & BIT0)
            {
                gu8CecWkupMessage = XBYTE[REG_CEC_OPCODE0];
            }
            else if(XBYTE[REG_CEC_HW_EVENT_STS] & BIT1)
            {
                gu8CecWkupMessage = XBYTE[REG_CEC_OPCODE1];
            }

            else if(XBYTE[REG_CEC_HW_EVENT_STS] & BIT6)
            {
                gu8CecWkupMessage = XBYTE[REG_CEC_OPCODE4];
            }
            else
            {
                gu8CecWkupMessage = 0xFF;   //0xFF is temp for return fail
            }
            //puts("CEC in sleep clear cec interrupt\n");
            MDrv_PM_ClearCECInterrupt();

            MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_CEC);
            gu8PowerOnMode = M_POWER_ON_BY_HDMI_CEC;
            //puts("CEC power good ChipTop reset\n");
        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */

    }

////////////////////////////////////////////////////////////////////////////////////////////
    //IR_INT_level bit 0
    if (FinalStatus0 & (1<<0))
    {
        /* start of standby mode start */
        IntStatus = XBYTE[0x1022];
        //printf("IntStatus = %bx\n", IntStatus);
#if( (IR_MODE_SEL == IR_TYPE_FULLDECODE_MODE) || (IR_MODE_SEL == IR_TYPE_RAWDATA_MODE) )
        if(!(msIR_ReadByte(IR_RPT_FIFOEMPTY) & BIT1))
        {
        	// <080115 Leehc> Check if present key is repeat or not.
        	// If present key is repeat discard it. We will only adapt single key.
        	if(!(msIR_ReadByte(IR_RPT_FIFOEMPTY) & BIT0))
        	{
            	key1 = msIR_ReadByte(IR_KEY);
        	}
			else
			{
				key1 = 0xff;
			}
            XBYTE[IR_FIFO_READ_PULSE] = 0x01;
            //printf("IR key value = 0x%02bx\n", key1);
            msIR_Clear_FIFO();
        }

        if( BIT4 == (XBYTE[0x1002] & BIT4) )   // oen = 1 i.e. not in sleep
        {
            if( TRUE == gu8HostReady)//Host is ready for IR command
            {
#if 1//for Cancel Power Off function
                if( IRKEY_POWER == key1)
                {
                    if(FALSE == gbAutoPowerOffFlag)
                    {
                    	gU8PrevPowerState = gu8PowerStatus;
                        gu32AutoOffTime = MDrv_RTC_GetCounter() + 15;
                        gbAutoPowerOffFlag = TRUE;
                    }
                }
#endif
            }
            //case 2: judgement in M4 Lite to check power down key value then call sleep
/*
            if(IRKEY_POWER == key1)
            {
                MDrv_PM_M4Sleep(POWERKEY_SLEEP_MODE, NULL);
                //puts("IR_INT_level not in sleep power down sleep \n");
            }
*/
            //puts("not in sleep\n");

        }
        else if( BIT4 != (XBYTE[0x1002] & BIT4) )   // oen = 0 i.e. in sleep
        {
            //XBYTE[0x2B18] = 0x0;   // POWER UP KEY should unmask all PM Sleep interrupt
            if( TRUE == gu8RMCKeyLock && TRUE == gu8ChildLock)
            {
                //only Remote IR POWER KEY can be used to wake up
                switch(key1)
                {
	/* taburin : 20081214, atsc 는 무조건 ir power key 막음.*/
#if (MS_BOARD_TYPE_SEL != BD_S6_ATSC_GP)
                    case IRKEY_POWER:
						MDrv_PM_PowerOn();
                        MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_IR);
                        gu8PowerOnMode = M_POWER_ON_BY_REMOTE_KEY;
                        break;
#endif
                    default:
                        //not wakeup key, keep sleep
                        MDrv_PM_KeepSleep();
                        break;
                }

            }
            else if(TRUE == gu8RMCKeyLock && FALSE == gu8ChildLock)
            {
                //not wakeup key, keep sleep
                MDrv_PM_KeepSleep();
            }
            else    // no RMCKeyLock
            {
                switch(key1)
                {
                    case IRKEY_POWER:
                    case IRKEY_CHANNEL_PLUS:
                    case IRKEY_CHANNEL_MINUS:
                    case IRKEY_NUM_0:
                    case IRKEY_NUM_1:
                    case IRKEY_NUM_2:
                    case IRKEY_NUM_3:
                    case IRKEY_NUM_4:
                    case IRKEY_NUM_5:
                    case IRKEY_NUM_6:
                    case IRKEY_NUM_7:
                    case IRKEY_NUM_8:
                    case IRKEY_NUM_9:
                    case IRKEY_INPUT_SOURCE:
                    case IRKEY_TV_INPUT:
                    case IRKEY_POWERONLY:
#if (IR_TYPE_SEL == IR_TYPE_CUS03_DTV)
                    case DSC_IRKEY_MULTIMEDIA:
                    case DSC_IRKEY_PWRON:
#endif
						MDrv_PM_PowerOn();
                        MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_IR);
                        if( IRKEY_POWERONLY == key1 )
                        {
                            gu8PowerOnMode = M_POWER_ON_BY_POWER_ONLY;
                        }
                        else
                        {
                            gu8PowerOnMode = M_POWER_ON_BY_REMOTE_KEY;
                        }
                        break;

                    default:
                        //not wakeup key, keep sleep
                        MDrv_PM_KeepSleep();
                        break;
                }
            }
        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */

            //XBYTE[0x2B18] &= ~BIT0;   //enable IR interrupt
#elif(IR_MODE_SEL == IR_TYPE_SWDECODE_MODE)

#if 0   //Only for simple test without KEY decision
        XBYTE[0x1002]=0xff; //power up
        XBYTE[0x1003]=0x03;

        while(!(XBYTE[0x1007] & BIT3));    //not power good
        XBYTE[0x1043] |= BIT7;  //ChipTop reset
        XBYTE[0x1043] &= ~BIT7;

        XBYTE[0x1022] = IntStatus;
#else
        MDrv_IR_SW_Isr(&key1);
        if (1 == g_bIrDetect)
        {
            if(IRKEY_POWER == key1 )
            {
				MDrv_PM_PowerOn();
                MDrv_PM_SleepWakeUp(/*gu8SleepMode,*/ INT_SRC_IR);
            }
        }
        XBYTE[0x1022] = IntStatus;/* end of standby mode */

#endif

#endif
    }
////////////////////////////////////////////////////////////////////////////////////////////

            //puts("not in sleep 4\n");
}


INT_SERVICE(TIMER0)
{

#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)

    TL0 = ISR_TIMER0_COUNTER % 256;
    TH0 = ISR_TIMER0_COUNTER / 256;
    gu32TimerCount++;
#endif
	//debug_GPIO();
#ifdef ENABLE_BLINKING_LED
#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
	if(gu8LEDBlinkEnable)
	{
		if ( gu16LEDDuration % 500 == 0 )
		{
			if ( gu16LEDDuration % 1000 == 0 )
				GPIO_SAR2_OUTPUT_LOW();
			else
				GPIO_SAR2_OUTPUT_HIGH();
		}
		gu16LEDDuration++;
	}
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	if ( gu32TimerCountBlink > 0 )
	{
		if ( gu32TimerCount1 % 500 == 0 )
		{
			if ( gu32TimerCountBlink %2 == 1 )
				GPIO_SAR2_OUTPUT_LOW();
			else
				GPIO_SAR2_OUTPUT_HIGH();

			gu32TimerCountBlink--;
		}

		gu32TimerCount1++;
	}
#endif
#endif

#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
/* PDP 200us timer */
	gu8400usCnt++;
	gu1640msCnt++;
	gu16100msCnt++;

	if(gu8400usCnt >= 2)	// 200UL x 2 = 400 =400 us
	{
	   gu8400usCnt = 0;
	   gu32TimerCount++;

	}
	if(gu1640msCnt > 200)	// 200UL x 200= 40,000 = 40ms
	{
	   gu1640msCnt = 0;
	   gu840msFlag = TRUE;
	}
	if(gu16100msCnt > 500)	// 200UL x 500 = 100,000 = 100ms
	{
	   gu16100msCnt = 0;
	   gu8100msFlag = TRUE;
	}
#if 0
	if(gu16100msCnt & 0x01)
	   LED_W1_HIGH();
	else
		 LED_W1_LOW();
#endif

	// timer cnt ++;
	// make pwm pulse
	if (gu8PWMClkCnt != 0xff)	// 0~50 (200us~10ms)
	{
	   if (gu8PWMClkCnt >= gu8DutyRate)
	   {
		  if(gu8LEDPosition == FALSE)
			 LED_W1_LOW();
		  else
			 LED_W2_LOW();
	   }
	   else
	   {
		  if(gu8LEDPosition == FALSE)
			 LED_W1_HIGH();
		  else
			 LED_W2_HIGH();
	   }

	   if(gu8PWMClkCnt++ >= 50)// 200ul x 50 = 10,000us= 10ms
		  gu8PWMClkCnt = 0;
	}

#endif
}

//#ifdef ENABLE_AC_DET   //20080807 for AC Detection
INT_SERVICE(TIMER1)
{

#ifdef ENABLE_NEW_AC_DET
#if ( !defined(METHOD_RE_INIT_RTC) && defined(METHOD_SYNC_1SEC_BOUNDARY) )
    U32 gu32FetchNewSystemTime = 0;
#endif
#endif

#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
    TL1 = ISR_TIMER1_COUNTER % 256;
    TH1 = ISR_TIMER1_COUNTER / 256;
#endif  // #if !(MS_BOARD_TYPE_SEL == BD_S6_PDP)

#ifdef LGE_USE_EEPROM


#ifdef ENABLE_NEW_AC_DET   //20081022
    gu32NotCountDownZero -= 1;
    if(!gu32NotCountDownZero)
    {
        gu8PollAcDetActiveFlg = TRUE;
    }

    if( (TRUE == gu8PollAcDetActiveFlg) && (AC_IS_DET()) )
#else   //old AC DET
    if( AC_IS_DET() )
#endif
	{

#ifdef ENABLE_NEW_AC_DET//mask & disable all wakeup source interrupt except RTC wakeup
        XBYTE[0x2B00] = 0xFF;   //mask PM FIQ
        XBYTE[0x2B18] = 0xFF;   //mask PM IRQ
        XBYTE[0x2B18] &= ~BIT7; //unmaks PM RTC

        XBYTE[0x1000] = 0;  //disable all wakeup sources
        XBYTE[0x1003] &= ~(BIT5|BIT6);  //disable all wakeup sources
#endif

	#if 0 // <081105 Leehc> 회로 대응으로 인하여 Soft Mute 를 걸지 않도록 함.
	    U8 j;	//<081023 Leehc> Acdetect 시 chiptop Reset ( temporary solution )
    	volatile U8 delay;
		<081024 Leehc> AC off 시 Pop Noize 를 제거하기 위해 먼저 sotfmute 를 걸어줌.
		XBYTE[0x102A] = MICOM_MSG_SOFT_MUTE;
		delay1ms(100);	// <081028 Leehc> Softmute 를 건후 amp mute delay 를 100ms 로 변경.
	#endif
		SB_MUTE_ON();

    	#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP || MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP )
           INV_CTL_OFF();
           PANEL_CTL_OFF();
	#elif (MS_BOARD_TYPE_SEL == BD_S6_PDP)					//FALSE: PDP
			ON5V_LOW();
	#endif

#ifdef ENABLE_NEW_AC_DET

		/* taburin : 20081214, 끌 관련 mstar solution 검증되기 전까진
					atsc는 RL_OFF 500ms 후 전체 reset하는걸로.*/
#if (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP) && !defined(DIFFEENCE_VENUS_FROM_MINERVA_ATSC)

		RL_OFF();
		delay1ms(500);

		XBYTE[0x1048] = 0xFF;
		XBYTE[0x1049] = 0xFF;
		while(1);
#else
        gu8ACisDetectedFlg = TRUE;
        gu32SaveSystemTime = MDrv_RTC_GetCounter();
#if ( !defined(METHOD_RE_INIT_RTC) && defined(METHOD_SYNC_1SEC_BOUNDARY) )
        while( (gu32FetchNewSystemTime = MDrv_RTC_GetCounter()) == gu32SaveSystemTime)
        {
            MDrv_PM_ClearWDT();
        }
        //set RTC Alarm time after 1 second then go into sleep mode
        MDrv_RTC_Alarm(gu32FetchNewSystemTime + MICOM_RESET_SLEEP_TIME );
#elif ( defined(METHOD_RE_INIT_RTC) && !defined(METHOD_SYNC_1SEC_BOUNDARY) )
        MDrv_RTC_Init(XTAL_RTC_CLOCK_FREQ);
        MDrv_RTC_SetCounter(0);
        MDrv_RTC_Alarm(MICOM_RESET_SLEEP_TIME);
#endif
        //Sleep
        //gbSleepFlag = TRUE;
        MDrv_PM_M4Sleep(SLEEP_MODE, (RTC_WAKEUP));
#endif

#else	//old AC_Det

		XBYTE[0x1048] = 0xFF;
		XBYTE[0x1049] = 0xFF;
		while(1);

#endif
	}

#endif	//LGE_USE_EEPROM

    //debug_GPIO();
}
//#endif

#if 0   //20080805 for LGE SCART2 ID
// TVLinkTuner cmd protocol
INT_SERVICE(SERIAL0)
{
    if(RI0)
    {
        RI0 = 0;
        if ( S0BUF == 0x0a) return;
        gu8Uart0CmdBuf[gu8Uart0BufIndex] = S0BUF; // recieve byte
        if(gu8Uart0CmdBuf[gu8Uart0BufIndex] == 0x0d)  // carriage return code
        {
            gu8Uart0Detected = TRUE; // command  buffer recieve ok
        }
        else if (gu8Uart0CmdBuf[gu8Uart0BufIndex] == 0x08)  // back space
        {
            if (gu8Uart0BufIndex != 0)
                gu8Uart0BufIndex--;
        }
        else
        {
            if (gu8Uart0BufIndex < (MAX_UART0_CMD_LENGTH-1))// Buffer Overflow
            {
                gu8Uart0BufIndex++;
            }
            else
            {
                gu8Uart0Detected = TRUE; // command  buffer recieve ok
                gu8Uart0BufIndex = 0; // reset index of command buffer
            }
        }
    }
    else
    {
        TI0 = 0;
    }
}
#endif


void main(void)
{

#ifdef ENABLE_SCART2_ID_DET   //20080805 for LGE SCART2 ID
    volatile U8 u8Cur_SC2ID, u8Next_SC2ID, delay;
    volatile U8 u8GetScart2IdFlag = FALSE;
    U8 i, j, k, SAR_LV[ADC_SAR_LEVEL], SAR_Value;
#endif

#ifndef TITANIA1
#ifdef	NOT_USED_4_LGE
    U8 u8Data;
#endif
#endif

    MDrv_Sys_UartInit();

    //puts("Hello PM Sleep\n");

    if( !MDrv_PM_IsAlive() )
    {

#ifdef ENABLE_BLINKING_LED
#if (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
    	//gu32TimerCount1 = 0;
    	//gu32TimerCountBlink = 11;
#elif (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
		gu16LEDDuration = 0;
		gu8LEDBlinkEnable = 0;
#endif
#endif
        MDrv_PM_GpioInit();

#ifdef ENABLE_WDT
        MDrv_PM_EnableWDT();//Enable WDT of M4
#else
        MDrv_PM_DisableWDT();//Disable WDT of M4
#endif
        //Set BootFlag as 0
        XBYTE[0x1055] &= ~ BIT6;

#ifndef TITANIA1
        XBYTE[0x1051] |= 0x80;  // turn on uart function in PAD_GPIO_PM
    //#ifdef ENABLE_AC_DET   //20080807 for AC Detection
#if defined(LGE_GP_COMMERCIAL_MODEL)
//        AC_DET_ENABLE();
#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
        UART_TX_ENABLE();
#endif

#endif

#ifdef LGE_USE_EEPROM
#if 0 	// GPIO_PM1 input  LCD/PDP 동시에 사용 , Gpio_init에서 이미 수행됨  earnest 091003
		if( TRUE == gu8EepromData3) //TRUE: LCD
		{
		   AC_DET_ENABLE(); // GPIO_PM1 input
		}
		else if( FALSE == gu8EepromData3) //TRUE: PDP
		{
		   AC_DET_DISABLE(); // GPIO_PM1 output for UART TX
		}
		else
		{
		   PM_DBG0(puts("ERROR AC_DET"));
		}
#endif

#endif
    //#endif
#endif


#ifdef ENABLE_EXTERNAL_INT
        XBYTE[0x1000] |= (1<<2);     //enable GIPO5 wake up
#endif
#ifdef ENABLE_EXTERNAL_INT2
        XBYTE[0x1000] |= (1<<3);     //enable GIPO6 wake up
#endif

#if !defined(CONFIG_BY_HOST)
        msKeypad_Init();
        //MDrv_PM_ConfigCecWakeUp();		//<081029 Leehc> Micom 이 CEC 를 initialize 하는 부분을 원복함.
        msIR_Initialize(MST_XTAL_CLOCK_MHZ);
#endif

        XBYTE[0x1000] |= 0x01;
        XBYTE[0x101D] |= 0x40;  //IR clock selection to external

        MDrv_RTC_Init(XTAL_RTC_CLOCK_FREQ);
        XBYTE[0x101D] &= 0xF0;   //rtc clock selection to external

        MDrv_PM_EnableInterrupt();

        MDrv_PM_SetAlive();

        PM_DBG1(puts("Hello PM :BOOT from ZERO...\n"));
    }
    else
    {
        PM_DBG1(puts("Hello PM :Re-BOOT from ZERO...\n"));
    }

    gu8PowerOnMode = 0x0;   //reset power on mode while AC_plugin & Reset

    IEN0 = 0x85;         // EAL, EX1, EX0

#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
    MDrv_PM_Timer0_Init();
#endif
//#ifdef ENABLE_AC_DET   //20080807 for AC Detection
#ifdef LGE_USE_EEPROM
	#if (MS_BOARD_TYPE_SEL != BD_S6_PDP)
	 MDrv_PM_Timer1_Init();
	#endif
	SB_MUTE_ON();
#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP || MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP )
        INV_CTL_OFF();
        PANEL_CTL_OFF();

#endif

#endif
//#endif

#ifdef LGE_USE_EEPROM   //20080812 for Power On Scenario
 	if(gu8EepromData1 & BIT4)
    {
        gu8ChildLock= TRUE;
    }
    if(gu8EepromData1 & BIT5)
    {
        gu8RMCKeyLock= TRUE;
    }

    gu8SetID= gu8EepromData2;   //Micom will read second byte and will set SETID (gu8SetID)

    gu8PowerStatus = M_POWER_STATE_BOOT;
    switch( gu8EepromData1 & 0x0F )
    {
        case POWER_STATUS_COLD:
        case POWER_STATUS_COOL:
        case POWER_STATUS_WARM:
        case POWER_STATUS_SWDOWN:
        case POWER_STATUS_UNKNOWN:
#if 1   //20080910 for reduce time for cold standby power down
            gu8PowerStatus = M_POWER_STATE_COLD_STANDBY;
            gu8PowerOnMode = 0x0;
            //MDrv_PM_M4Sleep(SLEEP_MODE, 0x91); //RTC & SAR & IR
            MDrv_PM_M4Sleep(SLEEP_MODE, (IR_WAKEUP|GPIO5_WAKEUP|KEYPAD_WAKEUP|RTC_WAKEUP));
#else
            gu8NextPowerStatus = M_POWER_STATE_COLD_STANDBY;
            gu8PowerOnMode = 0x0;
#endif
            break;

        case POWER_STATUS_HOT:
        case POWER_STATUS_BOOT:
            gu8NextPowerStatus = M_POWER_STATE_ACTIVE;
            gu8PowerOnMode = M_POWER_ON_BY_LAST_POWERON;

#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
			MDrv_PM_StartFrontLED();
		//	gu8LEDGo = TRUE;
		 //	gu8LEDStep = 0;
#endif  // #if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
#if (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
            /*taburin : 20081103, cotrol RL_ON(PM0) for saturn5*/
            RL_ON();
#endif
            break;

        default:
            PM_DBGs(puts("Last power state Error"));
            break;
    }

#else
    //for Power state Management
    gu8PowerStatus = M_POWER_STATE_BOOT;
    gu8NextPowerStatus = M_POWER_STATE_ACTIVE;
#endif  /* #ifdef LGE_USE_EEPROM */
#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
		MDrv_PM_Timer0_Init();
#endif
#ifdef LGE_USE_EEPROM
	#if (MS_BOARD_TYPE_SEL == BD_S6_PDP)
		 MDrv_PM_Timer1_Init();
	#endif
#endif

	while(1)
    {

        if(TRUE == gu8HostReady)
        {
#ifdef ENABLE_BLINKING_LED
#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
			if(gu8LEDBlinkEnable)
			{
				Blinking_LED_Stop_Blink();
			}
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
			Blinking_LED_Stop_Blink();
#endif
#endif

#ifdef ENABLE_LED   //20080812 for LED setting
            switch(gu8PowerOnMode)
            {
                case M_POWER_ON_BY_REMOTE_KEY:
                case M_POWER_ON_BY_ON_TIMER:
                case M_POWER_ON_BY_LOCAL_KEY:
                case M_POWER_ON_BY_RS232C:
                case M_POWER_ON_BY_POWER_ONLY:
                case M_POWER_ON_BY_REMIND:
                case M_POWER_ON_BY_SWDOWN:
                case M_POWER_ON_BY_LAST_POWERON:
                case M_POWER_ON_BY_HDMI_CEC:
                    WARM_LED_OFF();
                    break;

                case M_POWER_ON_BY_RESERVE:
                case M_POWER_ON_BY_OTA:
                    WARM_LED_ON();
                    break;

                default:
                    PM_DBGs(puts("ENABLE_LED Error"));
                    break;
            }
#endif

//#ifdef ENABLE_AC_DET   //20080807 for AC Detection
#ifdef LGE_USE_EEPROM

        if( !AC_IS_DET() )
        {
            SB_MUTE_OFF();
        }

#endif
//#endif

#ifndef TITANIA1 // hotel mode
#ifdef	NOT_USED_4_LGE
            // Issue 1: LGE on yet define what kind of source to wakup in sleep mode
            // Issue 2: M_PWC_ONLY_POWON is no meaning in normal mode
            // Issue 3: LGE on yet define what cmd can send the last power status
            switch(gu8PowerModeCtrl)
            {
                case M_PWC_ONLY_STBY:   // always standby
                    //MDrv_PM_M4Sleep(SLEEP_MODE, 0x91); //RTC & SAR & IR
                    MDrv_PM_M4Sleep(SLEEP_MODE, (IR_WAKEUP|GPIO5_WAKEUP|KEYPAD_WAKEUP|RTC_WAKEUP));
                    break;

                case M_PWC_ONLY_POWON:  // always power ON
                    // do nothing
                    break;

                default:                // Last power
                case M_PWC_LAST_POWER:
                    switch(gu8LastPowerStatus)
                    {
                        case M_POWER_STATE_COLD_STANDBY:
                            //MDrv_PM_M4Sleep(SLEEP_MODE, 0x91); //RTC & SAR & IR
                            MDrv_PM_M4Sleep(SLEEP_MODE, (IR_WAKEUP|GPIO5_WAKEUP|KEYPAD_WAKEUP|RTC_WAKEUP));
                            break;

                        case M_POWER_STATE_WARM_STANDBY:
#if	(MS_BOARD_TYPE_SEL != BD_S6_PDP)
                            //disable display
                            //GPIO_PM3: 0: PANEL ON 1: PANEL OFF
                            u8Data = XBYTE[REG_GPIO_PM3_OEN];
                            XBYTE[REG_GPIO_PM3_OEN] =  u8Data & ~BIT3;
                            u8Data = XBYTE[REG_GPIO_PM3_OUT];
                            XBYTE[REG_GPIO_PM3_OUT] = u8Data & ~BIT3;   // Set low level
#endif
                            break;

                        case M_POWER_STATE_ACTIVE:
                            //all up
                            break;

                        default:
                            break;
                    }
                    break;
            }

            // Issue 1: KeyOpModeCtrl is no meaning with IR because the IR cmd is broken in normal mode
            switch(gu8KeyOpModeCtrl)
            {
                case M_KEYOP_ONLY_IR:       // turn OFF SAR function
                    XBYTE[0x2B18] = BIT4;
                    break;

                case M_KEYOP_ONLY_LOCAL:    // turn OFF IR function(no meaning)
                    XBYTE[0x2B18] = BIT0;
                    break;

                case M_KEYOP_ONLY_IR_POWER: // turn OFF SAR and filter IR key code(no meaning)
                    XBYTE[0x2B18] = BIT4;
                    break;

                case M_KEYOP_NORMAL:
                    XBYTE[0x2B18] = 0x00;
                    break;
            }
#endif
#endif	/* #ifndef TITANIA1 */

#if 1   //for Power state Management include last power

            if( M_POWER_STATE_BOOT == gu8PowerStatus )
            {
                switch(gu8NextPowerStatus)
                {
                    case M_POWER_STATE_COLD_STANDBY:
                        gu8PowerStatus = M_POWER_STATE_COLD_STANDBY;
                        break;
                    case M_POWER_STATE_WARM_STANDBY:
                        gu8PowerStatus = M_POWER_STATE_WARM_STANDBY;
		                break;
                    case M_POWER_STATE_ACTIVE:
                        gu8PowerStatus = M_POWER_STATE_ACTIVE;
                        break;
                    default:
                        gu8PowerStatus = M_POWER_STATE_BOOT;
                        break;
                }
            }
#if 0
            switch(gu8PowerStatus)
            {
                case M_POWER_STATE_COLD_STANDBY:
#if 1   //20080910 for reduce time for cold standby power down
#ifdef LGE_USE_EEPROM
                    switch( gu8EepromData1 & 0x0F )
                    {
                        case POWER_STATUS_COLD:
                        case POWER_STATUS_COOL:
                        case POWER_STATUS_WARM:
                        case POWER_STATUS_SWDOWN:
                        case POWER_STATUS_UNKNOWN:
                            //do nothing
                            break;

                        default:
						// <081113 Leehc> Active 에서 AC deep 또는 채터링 테스트 도중 Standby 로 가는 문제 개선
						// 불필요하게 시스템 State 를 Sleep 로 보내는 코드로 판단되어 막음.
						// 엠스타에서 막아도 관계없다는 confirm 을 막았지만 왜 이부분으로 진입하는지 무슨 목적으로 해당
						// 코드가 삽입되었는지에 대한 자세한 검토를 의뢰중임.
							#if 0
                            MDrv_PM_M4Sleep(SLEEP_MODE, (IR_WAKEUP|GPIO5_WAKEUP|KEYPAD_WAKEUP|RTC_WAKEUP));
							#endif
							break;
                    }
#endif
#else

                        //MDrv_PM_M4Sleep(SLEEP_MODE, 0x91); //RTC & SAR & IR
                        MDrv_PM_M4Sleep(SLEEP_MODE, (IR_WAKEUP|GPIO5_WAKEUP|KEYPAD_WAKEUP|RTC_WAKEUP));
#endif
                    break;
                case M_POWER_STATE_WARM_STANDBY:
                    //disable display
                    //LGE said do nothing
                    break;
                case M_POWER_STATE_ACTIVE:
                    //all up
                    break;
                default:
                    break;
            }
#endif
#endif

// <081216 Leehc> 아무것도 하지않는 코드 제거함.
#if 0//for Cancel Power Off function	// Temporary blocked
            //if(M_POWER_STATE_WARM_STANDBY != gu8PowerStatus)
            if(gU8PrevPowerState == gu8PowerStatus)
            {
                if( (FALSE == gu8CancelPowerOff) \
                    && (TRUE ==gbAutoPowerOffFlag) \
                    && (MDrv_RTC_GetCounter() == gu32AutoOffTime) )    //15 second to Power off
                {
                	//U8 msgToMips;
                    //Need to auto power off while MIPS doesn't send the real sleep command to MICOM
                    gu32AutoOffTime = 0;
                    gbAutoPowerOffFlag = FALSE;
                    gu8HostReady = FALSE;
					gU8PrevPowerState = -1;
                    if( (DEEP_SLEEP_MODE == gu8SleepMode) || (SLEEP_MODE == gu8SleepMode) )
                    {
#if 0 // <080929 Leehc tmporary blocked. Check if this cord processed or not>
                        //MDrv_PM_M4Sleep(gu8SleepMode, 0x91); //RTC & SAR & IR
						MDrv_PM_M4Sleep(gu8SleepMode, (IR_WAKEUP|GPIO5_WAKEUP|KEYPAD_WAKEUP|RTC_WAKEUP));
#endif
						XBYTE[0x102A] = MICOM_MSG_ERROR_CHECK;
					}
                    else
                    {
#if 0 // <080929 Leehc tmporary blocked.Check if this cord processed or not>
                        //MDrv_PM_M4Sleep(SLEEP_MODE, 0x91); //RTC & SAR & IR
                        MDrv_PM_M4Sleep(SLEEP_MODE, (IR_WAKEUP|GPIO5_WAKEUP|KEYPAD_WAKEUP|RTC_WAKEUP));
#endif
						XBYTE[0x102A] = MICOM_MSG_ERROR_CHECK;
					}
                }
            }
#endif

#ifdef ENABLE_SCART2_ID_DET   //20080805 for LGE SCART2 ID
            if( (FALSE == u8GetScart2IdFlag) )
            {
                u8Cur_SC2ID = XBYTE[0x141C];
                //XBYTE[0x102B] = 0x00;           //using MBOX 3
                XBYTE[0x102A] = u8Cur_SC2ID;           //using MBOX 3
                u8GetScart2IdFlag = TRUE;
            }

            if( (TRUE == u8GetScart2IdFlag) )
            {
                for(i=0; i<ADC_SAR_LEVEL; i++)
                    SAR_LV[i] = 0;

                for ( i = 0; i < SAR_STABLE_NUM; i++ )
                {
                    for( k = 0; k<= 100; k++)
                        delay = k;

                    SAR_Value = XBYTE[0x141C];

                    for (j=0;j<ADC_SAR_LEVEL;j++)
                    {
                        if (SAR_Value <= tADCSARLevel[j])
                        {
                            SAR_LV[j]++;
                            break;
                        }
                    }
                }

                for(i=0; i<ADC_SAR_LEVEL; i++)
                {
                    if(SAR_LV[i] > SAR_STABLE_NUM_MIN)
                    {
                        PM_DBG1(printf("u8Next_SC2ID = %bx\n\n", tADCSARLevel[i]));
                        u8Next_SC2ID = tADCSARLevel[i];

                        if(u8Next_SC2ID != u8Cur_SC2ID)
                        {
                            XBYTE[0x102A] = tADCSARLevel[i];           //using MBOX 3
                            u8Cur_SC2ID = tADCSARLevel[i];
                        }
                    }
                }

            }
#endif  /* #ifdef ENABLE_SCART2_ID_DET */

        }

        if(TRUE == gbSleepFlag)
        {
            //To go to standby mode
            gbSleepFlag = FALSE;
            gu8HostReady = FALSE; //before MDrv_PM_M4Sleep() FALSE will success FALSE after wakeup
            if( (DEEP_SLEEP_MODE == gu8SleepMode) || (SLEEP_MODE == gu8SleepMode) )
            {
                MDrv_PM_M4Sleep(gu8SleepMode, gu16SleepSource);
            }
        }
#if 0   //comment out for reducing code size purpose

        PM_RIU_DUMMY31 |= BIT7;	//for debug if 8051 lite still alive
        PM_RIU_DUMMY31 &= ~BIT7;
#endif

#ifdef ENABLE_WDT
#if 0   //20080910  for factory reset purpose
        if(TRUE == gu8AutoOffOnFlag)
        {
            //don't reflesh WDT
            gu8AutoOffOnFlag = FALSE;
            //it seems so dangerous by this methodd (To be check)
        }
        else
        {
            MDrv_PM_ClearWDT();
        }
#else
        MDrv_PM_ClearWDT();
#endif
#endif

#if	(MS_BOARD_TYPE_SEL == BD_S6_PDP)
		MDrv_PM_RunFrontLED();
#endif

    }

}

#ifdef ENABLE_BLINKING_LED


void Blinking_LED_Ctrl_Cold()
{
#if (MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
	GPIO_PM0_OUTPUT_LOW();
    GPIO_SAR2_OUTPUT_HIGH();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
    GPIO_SAR2_OUTPUT_LOW();	// low active : LED R ON
#endif
}

void Blinking_LED_Ctrl_Warm()
{
#if	(MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
	GPIO_SAR2_OUTPUT_HIGH();
	GPIO_PM0_OUTPUT_HIGH();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	GPIO_SAR2_OUTPUT_LOW();
#endif
}
// During going to Active
void Blinking_LED_Start_Blink()
{
#if	(MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
	gu8LEDBlinkEnable = 1;
	GPIO_SAR2_OUTPUT_LOW();
	GPIO_PM0_OUTPUT_LOW();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	GPIO_SAR2_OUTPUT_LOW();
#endif
}

// system gets to Active
void Blinking_LED_Stop_Blink()
{
#if	(MS_BOARD_TYPE_SEL == BD_S6_DVB_GP)
	gu8LEDBlinkEnable = 0;
	GPIO_SAR2_OUTPUT_LOW();
	GPIO_PM0_OUTPUT_HIGH();
#elif (MS_BOARD_TYPE_SEL == BD_S6_ATSC_GP)
	gu32TimerCountBlink = 0;
	GPIO_SAR2_OUTPUT_HIGH(); // LED R OFF
#endif
}

#endif




