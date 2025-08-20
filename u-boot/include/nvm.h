/******************************************************************************

  LGE. DTV RESEARCH LABORATORY
  COPYRIGHT(c) LGE CO.,LTD. 2006. SEOUL, KOREA.
  All rights are reserved.

 ******************************************************************************/

/** @file nvm.h
 *
 *	This c file defines the DDI functions related to NVM Driver
 *
 *	@author 	dhjung
 *	@version	0.1
 *	@date		2008.08.06
 *	@note
 *	@see
 */

#ifndef _NVM_H_
#define _NVM_H_

#include <asm/types.h>
#include <../drivers/titania_hw_i2c.h>

/*********************************************************************
  ¸ÅÅ©·Î Á¤ÀÇ(Macro Definitions)
 **********************************************************************/
#define		I2cWriteNVM(a, b, c, d, e, f)	MDrv_HW_IIC_Write(a, b, c, d, e) //redcloud00@lge.com
#define		I2cReadNVM(a, b, c, d, e, f)	MDrv_HW_IIC_Read(a, b, c, d, e) //redcloud00@lge.com

/******************************************************************************
  »ó¼ö Á¤ÀÇ(Constant Definitions)
 ******************************************************************************/
#define NVMDRV_PAGE_SIZE 					128

#define NVMDRV_TOTAL_SIZE					(128 * 1024) //(64 * 1024)
#define	NVM_ID						 		0xA0

/* NVM DB Map */
#define NVM_HEADER_SIZE					 	16
/* NVM Magic */
#define TNVM_MAGIC_SIZE						4
#define SYS_DB_SIZE							376		/** 0180(384)  : System DB, should be first DB	**/

#define ANA_DB_SIZE							896		/** 0380(896)  : Analog Driver + Calibration DB	**/
#define TOOL_OPTION_DB_SIZE					128		/** 0080(128)  : Tool Option DB, reserved space = 108	**/
#define FACTORY_DB_SIZE						256		/** 0100(256)  : Factory DB, reserved space = 8	**/

#define UI_DB_SIZE							(11492*2)/** 1e80(7808) : UI DB(edge_enhancer,xvYCC,thx)**/
#define UI_EXPERT_DB_SIZE					(6372*2)/** 1400(5120) : UI Expert DB(edge, xvYVV,		**/
													/** 		h/vsharpness, 2-wb(pattern,save),	**/
													/** 		10-wb(pattern, luminance, save) )	**/
#define CH_DB_SIZE							41472	/** a200(41472): Channel DB(3k for DRRT)		**/
#define BT_DB_SIZE							3200	/** 0c80(3200) : BT DB							**/

#define EMP_DB_SIZE 						128		/** 0080(128)  : DivX DRM Information  			**/
#define ACAP_DB_SIZE						902		/** 1580(5504) : ACAP							**/
#define THX_DB_SIZE							128		/** 0080(128)  : THX							**/
#define MICOM_DB_SIZE						128		/** 0080(128)  : Internal MICOM DB				**/
#define FREE_DB_SIZE    				( NVMDRV_TOTAL_SIZE \
		- NVM_HEADER_SIZE	\
		- TNVM_MAGIC_SIZE	\
		- SYS_DB_SIZE		\
		- ANA_DB_SIZE		\
		- TOOL_OPTION_DB_SIZE	\
		- FACTORY_DB_SIZE	\
		- UI_DB_SIZE		\
		- UI_EXPERT_DB_SIZE \
		- CH_DB_SIZE		\
		- BT_DB_SIZE		\
		- EMP_DB_SIZE		\
		- THX_DB_SIZE		\
		- ACAP_DB_SIZE		\
		- MICOM_DB_SIZE 	) /** Free DB, reserved */

#define TOTAL_DB_SIZE    				( NVM_HEADER_SIZE		\
		+ TNVM_MAGIC_SIZE		\
		+ SYS_DB_SIZE 			\
		+ ANA_DB_SIZE 			\
		+ TOOL_OPTION_DB_SIZE	\
		+ FACTORY_DB_SIZE		\
		+ UI_DB_SIZE			\
		+ UI_EXPERT_DB_SIZE		\
		+ CH_DB_SIZE			\
		+ BT_DB_SIZE 			\
		+ EMP_DB_SIZE			\
		+ THX_DB_SIZE			\
		+ ACAP_DB_SIZE			\
		+ MICOM_DB_SIZE ) /** Total DB Size */


/* BASE OFFSET */
#define NVM_HEADER_BASE   				( 0                                     )
#define TNVM_MAGIC_BASE					( NVM_HEADER_BASE   + NVM_HEADER_SIZE 	)
#define SYS_DB_BASE       				( TNVM_MAGIC_BASE 	+ TNVM_MAGIC_SIZE 	)
#define ANA_DB_BASE       				( SYS_DB_BASE 		+ SYS_DB_SIZE 		)
#define TOOL_OPTION_DB_BASE   			( ANA_DB_BASE       + ANA_DB_SIZE       )
#define FACTORY_DB_BASE   				( TOOL_OPTION_DB_BASE+ TOOL_OPTION_DB_SIZE )

#define UI_DB_BASE        				( FACTORY_DB_BASE   + FACTORY_DB_SIZE   )
#define UI_EXPERT_DB_BASE 				( UI_DB_BASE        + UI_DB_SIZE        )
#define CH_DB_BASE        				( UI_EXPERT_DB_BASE + UI_EXPERT_DB_SIZE )
#define BT_DB_BASE        				( CH_DB_BASE   		+ CH_DB_SIZE   		)

#if 1
#define	EMP_DB_BASE     				( BT_DB_BASE  		+ BT_DB_SIZE  		)
#define	ACAP_DB_BASE	 				( EMP_DB_BASE		+ EMP_DB_SIZE 		)
#define THX_DB_BASE						( ACAP_DB_BASE		+ ACAP_DB_SIZE		)
#define	NVRAM_SIZE     					( THX_DB_BASE		+ THX_DB_SIZE	- 1	)
#else
#define	FREE_DB_BASE     				( BT_DB_BASE  		+ BT_DB_SIZE  		)
#define	EMP_DB_BASE     				( FREE_DB_BASE		+ FREE_DB_SIZE		)
#define	ACAP_DB_BASE	 				( EMP_DB_BASE		+ EMP_DB_SIZE 		)
#define THX_DB_BASE						( ACAP_DB_BASE		+ ACAP_DB_SIZE		)
#define	MICOM_DB_BASE     				( THX_DB_BASE		+ THX_DB_SIZE		)
#define NVRAM_SIZE        				( MICOM_DB_BASE     + MICOM_DB_SIZE - 1 )
#endif

#define NUM_OF_PRNT_EACH				32
#define NUM_OF_PRNT_MODULE				9
#define BOOLEAN 						unsigned int

#define DEBUG_LEVEL		0
#define EVENT_LEVEL		1
#define RELEASE_LEVEL	2

/******************************************************************************
  Çü Á¤ÀÇ (Type Definitions)
 ******************************************************************************/
typedef struct {
	UINT8	color[NUM_OF_PRNT_EACH];
	UINT32	bitmask;
} OSA_PRINT_ALLOC_T;

typedef struct
{
	UINT32				msk;
	OSA_PRINT_ALLOC_T	mod[NUM_OF_PRNT_MODULE];
} OSA_PRINT_MASK_T;

/**
 * panel power seq.
 *
 * @see
 */

typedef struct
{
	UINT8 panelPowOnToData;	/** Power Sequence for T2, refer to CAS*/
	UINT8 dataToLampOn;	/** Power Sequence for T3, refer to CAS*/
	UINT8 lampOffToData;	/** Power Sequence for T4, refer to CAS*/
	UINT8 dataToPanelPowOff;	/** Power Sequence for T5, refer to CAS*/

	UINT8 SOEtoDPM;			/** Power Sequence for T6, refer to CAS*/
	UINT8 DPMtoVGH;			/** Power Sequence for T6, refer to CAS*/
	UINT8 VGHtoVST;			/** Power Sequence for T6, refer to CAS*/
	UINT8 VSTtoGCLK;		/** Power Sequence for T7, refer to CAS*/

	UINT8 nRLOntoDimSig;	/** Power Sequence for Power Input for BLU to Dimmng Control Signal, refer to CAS*/
	UINT8 nDimSigtoLamp;	/** Power Sequence for Dimmng Control Signal to Lamp On, refer to CAS*/
}PANEL_POWER_SEQ_T;

/**
 * panel pwm info.
 *
 * @see
 */
typedef struct
{
	UINT8 vbrAMinVdc;	/** refer to CAS*/
	UINT8 vbrATypVdc;	/** refer to CAS*/
	UINT8 vbrAMaxVdc;	/** refer to CAS*/
	UINT8 vbrBMinDuty;	/** refer to CAS*/
	UINT8 vbrBMaxDuty;	/** refer to CAS*/
	UINT8 vbrBDCRDuty;/** refer to CAS*/
	UINT8 vbrBNoSignalDuty;	/** refer to CAS*/
	UINT8 vbrBFreq60hz; /** refer to CAS*/
	UINT8 vbrBFreq50hz; /** refer to CAS*/
}PANEL_PWM_T;

/**
  System DB ¸ðµâÀ» À§ÇÑ ½ÇÁ¦ »ç¿ë type Á¤ÀÇ
  */
typedef struct
{
	UINT32				validMark;
	OSA_PRINT_MASK_T	printMask;
	UINT32				sys_utt;			/* PDP panel use time */
	UINT8				powerOnMode;		/* internal micom:	POWER ON MODE	*/
	UINT8				powerState; 		/* internal micom:	POWER STATE 	*/
	UINT8				powerOnStatus;		/* internal micom:	HOTEL: POWER ON STATUS */
	UINT8				setID;				/* internal micom:	SET ID			*/
	UINT8				keyLocked;			/* internal micom:	HOTEL: KEY OPERATION MODE */
	UINT8				irLocked;			/* internal micom:	IR KEY LOCKED	*/
	UINT8				localLocked;		/* internal micom:	LOCAL KEY LOCKED*/
	/* taburin : 20090205, reserved(addr : 0x137)¸¦ atsc-saturn5¿¡¼­ cec on/off ¿©ºÎ¸¦ micom driver¿¡¼­ È®ÀÎ ÇÒ ¼ö ÀÖµµ·Ï »ç¿ëÇÔ. ¼öÁ¤ ½Ã È®ÀÎ ¹Ù¶÷.*/
	UINT8				reserved;			/* internal micom:	reserved */
	UINT8				dbgCount;			/* internal micom:	debug counter */
	UINT8				frcDownloadMode;	/* FRC auto download or not */
	PANEL_POWER_SEQ_T	panelpowerseq;		/* For panel power sequence timing */
	PANEL_PWM_T 		panelpwm;			/* For panel pwm */
	UINT8				systemtype;         /*system type 0:atsc, 1: dvb*/
	UINT8				ColorDepth;			/**COLOR_DEPTH_T */
	UINT8				LVDSPixel;			/**LVDS_PIXEL_T */
	UINT8				vcomPgammaChecksum; /*Vcom Pgamma value checksum */	
	UINT8				nDebugStatus;
	//UINT8				ntest;
#ifndef _EMUL_WIN
	UINT8				reset20081124[0];	/* Uncomment and change name of member to reset values in NVRAM */
#endif
} SYS_DB_T;

/* 
wifi frequency type 
*/
typedef enum{
	WIFI_FREQ_GROUP_0 = 0,
	WIFI_FREQ_GRUOP_1,
	WIFI_FREQ_GROUP_2,
	WIFI_FREQ_GROUP_3,
	WIFI_FREQ_GROUP_4,	
	WIFI_FREQ_GROUP_5,	
	WIFI_FREQ_GROUP_6,	
	WIFI_FREQ_GROUP_7,	
}WIFI_FREQ_T;


/* Backlight type*/

typedef enum
{
	BL_CCFL = 0,
	BL_NOR_LED,
	BL_EDGE_LED,
	BL_IOP_LED,
	BL_ALEF_LED,
	BL_IOL_LED,
	BL_CCFL_VCOM
} BACKLIGHT_TYPE_T;


/* AMP type */
typedef enum
{
	AMP_NTP7000 = 0,	/* NTP7000  */
	AMP_TAS5713,		/* TAS5713 */
	AMP_NTP3AMP,		/* NTP3AMP  */
	AMP_TAS3AMP,		/* TAS3AMP  */
	AMP_NTP2AMP,		/* NTP2AMP  */
	AMP_TAS5709,		/* TAS5709  */
	AMP_SOUNDBAR,		/* Soundbar AMP */
}AMP_CHIP_TYPE_T;


/* Hdmi swap order */
typedef enum
{
	TYPE_1243 = 0,
	TYPE_1234,
	TYPE_1324,
	TYPE_2143,
} HDMI_SWAP_ORDER_T;


/**
 * Named size of panel, comes from nvm_ddi.h
 */
typedef enum
{
	INCH_15=0,
	INCH_19,
	INCH_22,
	INCH_26,
//	INCH_27,
	INCH_32,
	INCH_37,
	INCH_42,
	INCH_46,
	INCH_47,
	INCH_50,
	INCH_52,
	INCH_55,
	INCH_60,
	INCH_70,
	INCH_72,	//for gp2 LE9X Tool //balup_090717
	INCH_BASE
} INCH_TYPE_T;

/**
 *	Type of tool, comes from nvm_ddi.h
 */
typedef enum
{
	//LCD Tool
	TOOL_LD35=0,    // 1
	TOOL_LD45,      // 2
	TOOL_LD55,		// 3
	TOOL_LD65,		// 4
	TOOL_LD75,		// 5
	TOOL_LD85,		// 6
	TOOL_LE45,		// 7
	TOOL_LE53,		// 8
	TOOL_LE55,		// 9
	TOOL_LE75,		// 10
	TOOL_LE85,		// 11
	TOOL_LE95,		// 12
	TOOL_LEX7,		// 13
	TOOL_LEX8,		// 14
	TOOL_LEX9,		// 15
	TOOL_LD32,		// 16
	TOOL_LD42,		// 17
	TOOL_LD52,		// 18
	TOOL_LD62,		// 19
	TOOL_LE73,		// 20
	TOOL_EL95,		// 21
	TOOL_LE33,		// 22
	TOOL_LE54,		// 23
	TOOL_LD46,		// 24
	TOOL_LD56,		// 25
	TOOL_LD84,		// 26
	TOOL_LE46,		// 27
	TOOL_LE65,		// 28
	TOOL_LE86,		// 29
	TOOL_LE96,		// 30
	TOOL_LD63,		// 31
 	TOOL_LX65,		// 32
	TOOL_LX95,		// 33
	TOOL_LD47,		// 34
	TOOL_LD78,		// 35
	TOOL_LE88,		// 36
	TOOL_X955,		// 37
	TOOL_LX9S,		// 38
	TOOL_LX8S,		// 39
	TOOL_LD34,		// 40
	TOOL_LD66,		// 41
	TOOL_LD86, 		// 42
	TOOL_E535,		// 43
	TOOL_LD49,		// 44
	TOOL_LE49,		// 45
	TOOL_LX9A,		// 46
	TOOL_X95A,		// 47
	TOOL_LE43,		// 48
	TOOL_LX6A,		// 49	
	TOOL_LCD_END,

	//PDP Tool
	TOOL_PK95= TOOL_LCD_END,	// 1
	TOOL_PK75,					// 2
	TOOL_PK55,					// 3
	TOOL_PK35,					// 4
	TOOL_PK25,					// 5
	TOOL_PJ95,					// 6
	TOOL_PJ75,					// 7
	TOOL_PJ65,					// 8
	TOOL_PJ55,					// 9
	TOOL_PJ35,					// 10
	TOOL_PJ25,					// 12
	TOOL_PJ23,					// 13
	TOOL_PK99,					// 14
	TOOL_PK98,					// 15
	TOOL_PK96,					// 16
	TOOL_PK79,					// 17
	TOOL_PK78,					// 18
	TOOL_PK76,					// 19
	TOOL_PK59,					// 20
	TOOL_PK94,					// 21
	TOOL_PK58,					// 22
	TOOL_PX95,					// 23
	TOOL_PX99,					// 24
	TOOL_PDP_END,
	TOOL_BASE	= TOOL_PDP_END,	// LCD ToolÀÌ Ãß°¡µÇ¾î PDP ToolÀÇ ¼öº¸´Ù´Â ¸¹À»°ÍÀ¸·Î ¿¹»óµÊ.

} TOOL_TYPE_T;




typedef enum
{
	MODULE_LGD = 0,
	MODULE_CMO,
	MODULE_AUO,
	MODULE_SHARP,
	MODULE_IPS,
	MODULE_INNOLUX,
	MODULE_IVO,
	MODULE_LGD_V55,
	MODULE_LCD_END,

	MODULE_LGE = MODULE_LCD_END,
	MODULE_PANASONIC,
	MODULE_PDP_END,
	MODULE_BASE	= MODULE_PDP_END
} MODULE_MAKER_TYPE_T;



typedef enum
{
	MODULE_VER0		=0,		//not support error out, not support scanning backlight
	MODULE_VER1		=1,     //support error out, not support scanning backlight
	MODULE_VER2		=2,     //support scanning backlight, not support error out
	MODULE_VER3	 	=3		//support error out, support scanning backlight
} MODULE_VERSION_TYPE_T;

/**
 *  Tool Option1 of Ez Adjust Menu(Service Menu).
 */
typedef union
{
	UINT16 all;
	struct
	{
		UINT16					nModuleVer      		: 2;
		UINT16					eModelModuleType		: 3;
		UINT16					eModelToolType			: 7;	
		UINT16 					eModelInchType  		: 4;
	} flags;

} TOOL_OPTION1_T;

/**
 *  Tool Option2 of Ez Adjust Menu(Service Menu).
 */
typedef union
{
	UINT16 all;
	struct
	{
		UINT16					nUSBCount				: 1;
		UINT16					nRGBCount 				: 1;
		UINT16					nAVPosition				: 1;
		UINT16					nRCA_AVCount			: 2;		
		UINT16					nSCARTCount 			: 1;
		UINT16 					bSupportComponentSwap	: 1;
		UINT16					bCompAVCommon			: 1;		
		UINT16					eCompPosition			: 1;
		UINT16					nCOMPCount				: 2;
		UINT16					nHDMIPosition			: 1;		
		UINT16					eHDMISwitchIC			: 1;
		UINT16					nHDMICount				: 2;
		UINT16 					nReserved				: 1;
	} flags;

} TOOL_OPTION2_T;

/**
 *  Tool Option3 of Ez Adjust Menu(Service Menu).
 */
typedef union
{
	UINT16 all;
	struct
	{
		UINT16					bInstantBootEnabled	: 1;	
		UINT16					bDVRReady			: 1;
		UINT16					bBootLogo			: 1;
		UINT16					bWirelessReady		: 1;
		UINT16					eBackLight			: 3;
//		UINT16					eLED				: 2;
		UINT16					eAudioAmp			: 3;
		UINT16 					bEManual			: 1;
		UINT16					bHeadphone			: 1;
		UINT16 					bDigitalEye 			: 1;
		UINT16 					bBluetooth			: 1;
		UINT16 					bDivx				: 1;
		UINT16 					bEMF				: 1;
	} flags;
} TOOL_OPTION3_T;
/**
 *  Tool Option4 of Ez Adjust Menu(Service Menu).
 */
typedef union
{
	UINT16 all;
	struct
	{
		UINT16 					bSupportEPA			: 1;
		UINT16					bCIFS				: 1;
		UINT16					bISF				: 1;
		UINT16					bPictureWizard		: 1;
		UINT16	 				bTHXMediaDirector	: 1;
		UINT16 					bDLNA				: 1;
		UINT16 					bTHX				: 1;
		UINT16 					bLocalDimming		: 1;
		UINT16					eAnalogDemod		: 3;
		UINT16					eDigitalDemod		: 5;
	} flags;

} TOOL_OPTION4_T;

/**
 *  Tool Option5 of Ez Adjust Menu(Service Menu).
 */
typedef union
{
	UINT16 all;
	struct
	{		
		UINT16					bPowerBoardType		: 1;
		UINT16					bNetcastService		: 1;
		UINT16					bOrangeService		: 1;
		UINT16 					bMirrorMode			: 1;
		UINT16	 				nUSBHubLevel		: 1;
		UINT16					bTVLink				: 1;
		UINT16					bChBrowser			: 1;
		UINT16					bMotionRemocon		: 1;
		UINT16					bSupportSkype		: 1;
		UINT16					bSupportWiFi		: 1;
		UINT16					nHDMIPortSwapOrder  : 3;
		UINT16 					nReserved			: 3;
	} flags;

} TOOL_OPTION5_T;






typedef enum{
	LANG_GROUP_AJ_ALL = 0,
	LANG_GROUP_AJ_ARABIC,
	LANG_GROUP_AJ_HEBREW
}LANG_GROUP_T;

typedef enum{
	DEFAULT_LANG_ENGLISH = 0,
	DEFAULT_LANG_PORTUGUESE,
	DEFAULT_LANG_SPANISH
}DEFAULT_LANG_T;

typedef enum{
	TTX_LANG_GROUP_WES_EU = 0,
	TTX_LANG_GROUP_EST_EU, 
	TTX_LANG_GROUP_RUS,
	TTX_LANG_GROUP_ARA,
	TTX_LANG_GROUP_FARSI,
	TTX_OFF
}TTX_LANG_GROUP_T;

// 090721 ÇöÀç±îÁö ¿äÃ»ÇÑ Location ÀÌ ¾øÀ½
typedef enum{
	LOCATION_OFF = 0
}LOCATION_T;

typedef union
{
	UINT16 all;
	struct
	{
		UINT16					eWiFiFreq			: 3;		/* Wi-Fi Frequency */
		UINT16					eDefaultLang			: 2;		/* Default Language*/
		UINT16					eLocation			: 3;
		UINT16					bMONO				: 1;		/* °­Á¦ ¸ð³ë Áö¿øÀ¯¹« */
		UINT16					bHDEV				: 1;		/* Audio HDEV On/Off*/
		UINT16					bI_II_Save			: 1;		/* I/II Save Áö¿øÀ¯¹« */
		UINT16					eTTXLangGroup		: 3;		/* Teletext Langage Group */
		UINT16					eLangGroup			: 2;		/* Langage Group */
		
	}flags;
}AREA_OPTION_T;


/**
 *	TOOL OPTION struct type
 */
typedef struct
{
	UINT32              validMark;
	TOOL_OPTION1_T		nToolOption1;
	TOOL_OPTION2_T		nToolOption2;
	TOOL_OPTION3_T		nToolOption3;
	TOOL_OPTION4_T		nToolOption4;
	TOOL_OPTION5_T		nToolOption5;
	AREA_OPTION_T		stAreaOptions;	/* Area Option for ¾ÆÁÖ/Áß¾Æ/Áß³²¹Ì */
	UINT8				aModelName[13]; //090212 odrie20 Model NameÀ» eeprom¿¡ ÀúÀåÇÏ°í ÃÊ±â data¸¸ inch, toolÀÇ Á¶ÇÕÀ¸·Î ¸¸µéµµ·Ï åÇÔ

}	TOOLOPTION_DB_T;

TOOLOPTION_DB_T gToolOptionDB;

/*********************************************************************
  Function Prototypes
 **********************************************************************/
extern	SYS_DB_T gSysNvmDB;
extern	int	DDI_NVM_Init (void);
extern	int	DDI_NVM_Read (UINT32 offset, UINT32 nData, UINT8 *pRxBuf);
extern	int	DDI_NVM_Write (UINT32 offset, UINT32 nData, UINT8 *pTxBuf);
extern	int	DDI_NVM_WriteOne (UINT32 offset, UINT8 value);
extern	int	DDI_NVM_ReadOne (UINT32 offset, UINT8 *value);
extern	void DDI_NVM_GetToolOpt(void);
extern     int DDI_GetModelOption(void);


#endif  /* End of _NVM_H_ */
