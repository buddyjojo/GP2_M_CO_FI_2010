#ifndef _AUSIF_H_
#define _AUSIF_H_

#define _BIT0                       0x0001
#define _BIT1                       0x0002
#define _BIT2                       0x0004
#define _BIT3                       0x0008
#define _BIT4                       0x0010
#define _BIT5                       0x0020
#define _BIT6                       0x0040
#define _BIT7                       0x0080
#define _BIT8                       0x0100
#define _BIT9                       0x0200
#define _BIT10                      0x0400
#define _BIT11                      0x0800
#define _BIT12                      0x1000
#define _BIT13                      0x2000
#define _BIT14                      0x4000
#define _BIT15                      0x8000

#define _bit0_(val)                 ((val & _BIT0))
#define _bit1_(val)                 ((val & _BIT1))
#define _bit2_(val)                 ((val & _BIT2))
#define _bit3_(val)                 ((val & _BIT3))
#define _bit4_(val)                 ((val & _BIT4))
#define _bit5_(val)                 ((val & _BIT5))
#define _bit6_(val)                 ((val & _BIT6))
#define _bit7_(val)                 ((val & _BIT7))
#define _bit8_(val)                 ((val & _BIT8))
#define _bit9_(val)                 ((val & _BIT9))
#define _bit10_(val)                ((val & _BIT10))
#define _bit11_(val)                ((val & _BIT11))
#define _bit12_(val)                ((val & _BIT12))
#define _bit13_(val)                ((val & _BIT13))
#define _bit14_(val)                ((val & _BIT14))
#define _bit15_(val)                ((val & _BIT15))

enum
{
    IS_MAIN_CARRIER=_BIT0,
    IS_A2=_BIT1,
    IS_NICAM=_BIT2
};

enum
{
    AU_SYS_NOT_READY,   // 0
    AU_SYS_M_BTSC,      // 1
    AU_SYS_M_EIAJ,      // 2
    AU_SYS_M_A2,        // 3

    AU_SYS_BG_A2,       // 4
    AU_SYS_DK1_A2,      // 5
    AU_SYS_DK2_A2,      // 6
    AU_SYS_DK3_A2,      // 7

    AU_SYS_BG_NICAM,    // 8
    AU_SYS_DK_NICAM,    // 9
    AU_SYS_I_NICAM,     // 10
    AU_SYS_L_NICAM,     // 11
    AU_SYS_FM_RADIO,    // 12
    AU_SYS_HI_DEV=0x10
};

// for SIF gain.
#define PRESCALE_STEP_ONE_DB    4
#define SET_PRESCALE_A2_FM      0
#define SET_PRESCALE_NICAM      1
#define SET_PRESCALE_AM         2
#define SET_PRESCALE_HIDEV      3
#define SET_PRESCALE_FM_M       4
#define SET_PRESCALE_HIDEV_M      5

//////////
#define ADEC_SIF_CMD_STANDARD         0x2DC0
#define ADEC_SIF_STATUS_STANDARD     0x2DE0

#define ADEC_SIF_DEBUG_CMD               0x2DDD
#define ADEC_SIF_DEBUG_DATA1            0x2DDC
#define ADEC_SIF_DEBUG_DATA2            0x2DDF
#define ADEC_SIF_DEBUG_DATA3            0x2DDE

typedef enum ADEC_SIF_STANDARD{
    ADEC_SIF_MODE_DETECT = 0,
    ADEC_SIF_BG_NICAM,
    ADEC_SIF_BG_FM,
    ADEC_SIF_BG_A2,
    ADEC_SIF_I_NICAM,
    ADEC_SIF_I_FM,
    ADEC_SIF_DK_NICAM,
    ADEC_SIF_DK_FM,
    ADEC_SIF_DK1_A2,
    ADEC_SIF_DK2_A2,    // Richard.Ni add
    ADEC_SIF_DK3_A2,
    ADEC_SIF_L_NICAM,
    ADEC_SIF_L_AM,
    ADEC_SIF_MN_A2,
    ADEC_SIF_MN_BTSC,
    ADEC_SIF_MN_EIAJ,
    ADEC_SIF_NUM_SOUND_STD
} ADEC_SIF_STANDARD_T;

typedef enum ADEC_SIF_SOUNDSYSTEM{
    ADEC_SIF_SYSTEM_BG = 0,
    ADEC_SIF_SYSTEM_I,
    ADEC_SIF_SYSTEM_DK,
    ADEC_SIF_SYSTEM_L,
    ADEC_SIF_SYSTEM_MN,
    ADEC_SIF_SYSTEM_LPRIME,
    ADEC_SIF_SYSTEM_UNKNOWN = 0xf0
} ADEC_SIF_SOUNDSYSTEM_T;


typedef enum ADEC_SIF_BAND{
    ADEC_SIF_BAND_MN = 0,
    ADEC_SIF_BAND_BG,
    ADEC_SIF_BAND_I,
    ADEC_SIF_BAND_DK,
    ADEC_SIF_NUM_SOUND_BANDS
} ADEC_SIF_BAND_T;


typedef enum ADEC_SIF_EXISTENCE_INFO {
    ADEC_SIF_ABSENT = 0,
    ADEC_SIF_PRESENT,
    ADEC_SIF_DETECTING_EXISTENCE
} ADEC_SIF_EXISTENCE_INFO_T ;


typedef enum ADEC_SIF_AVAILE_STANDARD {
    ADEC_SIF_NICAM = 0,
    ADEC_SIF_A2,
    ADEC_SIF_FM,
    ADEC_SIF_DETECTING_AVAILABILITY
} ADEC_SIF_AVAILE_STANDARD_T ;

#if 1	// Changed by LGE 091012
typedef enum
{
	ATV_GET_PAL_MONO				=	0x00,	// PAL Mono
	ATV_GET_PAL_STEREO				=	0x01,	// PAL Stereo
	ATV_GET_PAL_DUAL				=	0x02,	// PAL Dual
	ATV_GET_PAL_NICAM_MONO			=	0x03,	// PAL NICAM Mono
	ATV_GET_PAL_NICAM_STEREO		=	0x04,	// PAL NICAM Stereo
	ATV_GET_PAL_NICAM_DUAL			=	0x05,	// PAL NICAM Dual
	ATV_GET_PAL_UNKNOWN				=	0x06,	// PAL Unkown State
	ATV_GET_NTSC_A2_MONO			=	0x10,	// NTSC(A2) Mono
	ATV_GET_NTSC_A2_STEREO			=	0x11,	// NTSC(A2) Stereo
	ATV_GET_NTSC_A2_SAP				=	0x12,	// NTSC(A2) SAP
	ATV_GET_NTSC_A2_UNKNOWN			=	0x13,	// NTSC(A2) Unkown State
	ATV_GET_NTSC_BTSC_MONO			=	0x14,	// NTSC(BTSC) Mono
	ATV_GET_NTSC_BTSC_STEREO		=	0x15,	// NTSC(BTSC) Stereo
	ATV_GET_NTSC_BTSC_SAP_MONO		=	0x16,	// NTSC(BTSC) SAP Mono
	ATV_GET_NTSC_BTSC_SAP_STEREO	=	0x17,	// NTSC(BTSC) SAP Stereo
	ATV_GET_NTSC_BTSC_UNKNOWN		=	0x18,	// NTSC(BTSC) Unkown State
} ATV_AUDIO_MODE_GET_T;

typedef enum
{
    ATV_SET_PAL_MONO				=	0x00,	// PAL Mono
	ATV_SET_PAL_MONO_FORCED			=	0x01,	// PAL Mono Force Mono
	ATV_SET_PAL_STEREO				=	0x02,	// PAL Stereo
	ATV_SET_PAL_STEREO_FORCED		=	0x03,	// PAL Stereo Force Mono
	ATV_SET_PAL_DUALI				=	0x04,	// PAL Dual I
	ATV_SET_PAL_DUALII				=	0x05,	// PAL Dual II
	ATV_SET_PAL_DUALI_II			=	0x06,	// PAL Dual I+II
	ATV_SET_PAL_NICAM_MONO			=	0x07,	// PAL NICAM Mono
	ATV_SET_PAL_NICAM_MONO_FORCED	=	0x08,	// PAL NICAM Mono Force Mono
	ATV_SET_PAL_NICAM_STEREO		=	0x09,	// PAL NICAM Stereo
	ATV_SET_PAL_NICAM_STEREO_FORCED	=	0x0A,	// PAL NICAM Stereo Force Mono
	ATV_SET_PAL_NICAM_DUALI			=	0x0B,	// PAL NICAM Dual I
	ATV_SET_PAL_NICAM_DUALII		=	0x0C,	// PAL NICAM Dual II
	ATV_SET_PAL_NICAM_DUALI_II		=	0x0D,	// PAL NICAM Dual I+II
	ATV_SET_PAL_NICAM_DUAL_FORCED	=	0x0E,	// PAL NICAM Dual Forced Mono(Not Supported)
	ATV_SET_PAL_UNKNOWN				=	0x0F,	// PAL Unkown State
	ATV_SET_NTSC_A2_MONO			=	0x10,	// NTSC(A2) Mono
	ATV_SET_NTSC_A2_STEREO			=	0x11,	// NTSC(A2) Stereo
	ATV_SET_NTSC_A2_SAP				=	0x12,	// NTSC(A2) SAP
	ATV_SET_NTSC_A2_UNKNOWN			=	0x13,	// NTSC(A2) Unkown State
	ATV_SET_NTSC_BTSC_MONO			=	0x14,	// NTSC(BTSC) Mono
	ATV_SET_NTSC_BTSC_STEREO		=	0x15,	// NTSC(BTSC) Stereo
	ATV_SET_NTSC_BTSC_SAP_MONO		=	0x16,	// NTSC(BTSC) SAP Mono
	ATV_SET_NTSC_BTSC_SAP_STEREO	=	0x17,	// NTSC(BTSC) SAP Stereo
	ATV_SET_NTSC_BTSC_UNKNOWN		=	0x18,	// NTSC(BTSC) Unkown State
} ATV_AUDIO_MODE_SET_T;
#endif


typedef struct
{
    U8 soundSystem;
    U32 bandStrength;
} BAND_DETECT_INFO_T;

typedef struct
{
    U8 standard;
    U8  exist;
} AVAILABLE_SYSTEM_INFO_T;

typedef struct
{
    U8 rw_standard_type;
    U8 Threshold_type;
    U16 value;
} ACCESS_THRESHOLD_INFO_T;

typedef struct
{
    U8 type;
    S32 db_value;
} ADEC_SIF_SET_PRESCALE_T;

#define code
typedef struct
{
    BYTE  Thrtype;
    BYTE  HiByteValue;
    BYTE  LowByteValue;
} THR_TBL_TYPE;

typedef enum
{
    AU_SIF_READ_STD_SEL_SET,             //read sif standard
    AU_SIF_READ_A2_SND_MODE_STATUS1,     //read A2 status
    AU_SIF_READ_NICAM_SND_MODE_STATUS2,  //read nicam status
    AU_SIF_READ_STATUS_STANDARD,         //get sif standard
    AU_SIF_WRITE_SUB_CARRIER_STD_SET,    //write sub carrier and standard
    AU_SIF_WRITE_CARRIER_DEBOUNCE,       //write carrier debounce
    AU_SIF_WRITE_STD_SET,                //set sif standard
    AU_SIF_WRITE_SUB_CARRIER_SET,        //set sif sub carrier
    AU_SIF_WRITE_STD_HIDEV_SET,          //set sif hidev
    AU_SIF_WRITE_PFIRBANDWIDTH,          //write pfir bandwidth
    AU_SIF_WRITE_ENABLE_SIF_SYNTHESIZER, //enable sif synthesizer
    AU_SIF_WRITE_DISABLE_SIF_SYNTHESIZER //disable sif synthesizer
}AUDIO_SIF_CMD_STATUS;

//======SIF Threshold Type========
typedef enum
{
    CARRIER1_ON_AMP = 0x00,
    CARRIER1_OFF_AMP,
    CARRIER1_ON_NSR,
    CARRIER1_OFF_NSR,
    CARRIER2_ON_AMP,
    CARRIER2_OFF_AMP,
    CARRIER2_ON_NSR,
    CARRIER2_OFF_NSR,
    A2_PILOT_ON_AMP,
    A2_PILOT_OFF_AMP,

    NICAM_ON_SIGERR = 0x0,
    NICAM_OFF_SIGERR = 0x1,

    BTSC_MONO_ON_NSR_RATIO = 0x0,
    BTSC_MONO_OFF_NSR_RATIO = 0x1,
    BTSC_PILOT_AMPLITUDE_ON = 0x2,
    BTSC_PILOT_AMPLITUDE_OFF = 0x3,
    BTSC_SAP_ON_NSR_RATIO = 0x4,
    BTSC_SAP_OFF_NSR_RATIO = 0x5,
    BTSC_STEREO_ON_RATIO = 0x6,
    BTSC_STEREO_OFF_NSR_RATIO = 0x7,
    BTSC_SAP_AMPLITUDE_ON = 0x8,
    BTSC_SAP_AMPLITUDE_OFF = 0x9,
    BTSC_HIDEV_ON_NSR_RATIO = 0xA,
    BTSC_HIDEV_OFF_NSR_RATIO = 0xB
}AUDIO_SIF_THRESHOLD_TYPE;

//sif sub-carrier set
#define SIF_MODE_NICAM          0x40
#define SIF_MODE_A2             0x20
#define SIF_MODE_HIDEV          0x10
#define SIF_MODE_MONO           0x00

//sif hidev bandwidth type
#define HIDEV_PFIR_BW_NARROW    0x10
#define HIDEV_PFIR_BW_MIDDLE    0x20
#define HIDEV_PFIR_BW_WIDE      0x30

//Mailbox definition
#define AU_CMD_STANDARD             0x2DC0
#define AU_CMD_PFIRBANDWIDTH  0x2DC2  //HiDEV[5:4], A2 carrier2 pfir[1:0]
#define AU_CMD_MODE1                   0x2DC4        //BTSC/A2/EIAJ
#define AU_CMD_MODE2                   0x2DC6        //NICAM
#define AU_CMD_FC_TRACK             0x2DCA
#define AU_CMD_DEBUG                   0x2DDD
#define AU_CMD_DBG_CMD              0x2DDD
#define AU_CMD_DBG_DATA_H       0x2DDC
#define AU_CMD_DBG_DATA_M       0x2DDF
#define AU_CMD_DBG_DATA_L        0x2DDE

#define AU_STATUS_STANDARD      0x2DE0
#define AU_STATUS_MODE1          0x2DE4        //BTSC/A2/EIAJ
#define AU_STATUS_MODE2          0x2DE6        //NICAM
//#define AU_STATUS_NICAM       0x2DE6       // there's another AU_STATUS_NICAM in enum  ....^^|||
#define AU_STATUS_DEBUG1         0x2DFC
#define AU_STATUS_DEBUG2         0x2DFF
#define AU_STATUS_DBG_H           0x2DFC
#define AU_STATUS_DBG_M           0x2DFF
#define AU_STATUS_DBG_L            0x2DFE

#define SIF_ADC_FROM_VIF_PATH    0x08

void MDrv_MAD_AuSifInit(void);
BOOL MDrv_MAD_SIF_ReLoadCode(BYTE type);
void MDrv_MAD_AuSif_setMemInfo(void);
void MDrv_MAD_SIF_TriggerSifPLL(void);

BOOL MDrv_MAD_GetBtscA2StereoLevel(U16 *pLevel);
void MDrv_MAD_GetCurAnalogMode(U16 *AlgMode);
U16 MDrv_MAD_SetUserAnalogMode(U16 AlgMode);
BOOL MDrv_MAD_SIF_SetModeSetup(U8 SystemType);
BOOL MDrv_MAD_SIF_GetBandDetect(U8 soundSystem, U32 *bandStrength);
BOOL MDrv_MAD_SIF_SetModeSetup(U8 SystemType);
BOOL MDrv_MAD_SIF_SetBandSetup(U8 SifBand);
BOOL MDrv_MAD_SIF_CheckAvailableSystem(U8 standard, U8 *exist);
void MDrv_MAD_CheckNicamDigital(U8 *isNicamDetect);
BOOL MDrv_MAD_SIF_GetSoundStandard(U8 *B_SifStandard);
BOOL MDrv_MAD_SIF_AccessThreshold(AUDIO_SIF_SYSTEM_TYPE rw_standard_type, AUDIO_SIF_THRESHOLD_TYPE  u8Threshold_type, MS_U16 *u16Value);
BOOL MDrv_MAD_SIF_RedoSetStandard(void);
BOOL MDrv_MAD_SIF_WritePARAMETER(WORD dsp_addr, DWORD value);
BOOL MDrv_MAD_SIF_ReadPARAMETER(WORD dsp_addr, DWORD *value);
BOOL MDrv_MAD_SetBtscA2ThresholdLevel(U16 );

BOOL MDrv_MAD_SIF_SetPrescale(U8 type, S32 db_value);
BOOL MDrv_MAD_SIF_SetHDEVMode(U8 bOnOff);
BOOL MDrv_MAD_SIF_LoadCode(U8 type);
BOOL MDrv_MAD_SIF_CheckA2DK(U8 SystemType, U8 *exist);
void MDrv_MAD_SetThresholdType(THR_TBL_TYPE code *ThrTbl, BYTE num,  BYTE standardtype);
void MDrv_MAD_WriteSIFThresholdTbl_PAL(void);
void MDrv_MAD_WriteSIFThresholdTbl_BTSC(void);
void MDrv_MAD_SIF_ENABLE_CHANNEL(MS_BOOL bEnable);
MS_U8 MDrv_MAD_SIF_GetDspType(void);
void MDrv_MAD_SIF_SetDspCodeType(MS_U8 u8Type);
MS_U8 MDrv_MAD_SIF_GetDspCodeType(void);
MS_BOOL MDrv_MAD_SIF_GetDspCodeVifFlag(void);
void MDrv_MAD_SIF_ENABLE_CHANNEL(MS_BOOL bEnable);
MS_BOOL MDrv_MAD_SIF_BeeperToneSetting(MS_U8 tone,MS_U8 beeper_volume);
MS_BOOL MDrv_MAD_SIF_BeeperEnable(MS_U8 beeper_enable);
MS_BOOL MDrv_MAD_SIF_AskADCFromVifPathSupported(void);
MS_BOOL MDrv_MAD_SIF_SetADCFromVifPath(MS_BOOL bEnableVifPath);
MS_BOOL MDrv_MAD_SIF_GetADCFromVifPathFlag(void);
MS_BOOL MDrv_MAD_SIF_WriteCommand(AUDIO_SIF_CMD_STATUS CmdType, MS_U8 value);
void MDrv_MAD_SIF_ResetGetGain0(void);
MS_BOOL MDrv_MAD_SIF_GetOrginalGain(void);
void MDrv_MAD_SELECT_SIF_FIFOMODE(MS_BOOL bSelect);
void MDrv_MAD_DisEn_MIUREQ(void);
void MDrv_MAD_SIF_Init(void);
extern void MDrv_MAD2_DisEn_MIUREQ(void);
extern BOOL MDrv_MAD_Alg2ReloadCode(U8 decoder_type);
extern void MHal_MAD_AbsWriteMaskByte(MS_U32 u32Reg, MS_U8 u8Mask, MS_U8 u8Val);
extern MS_BOOL MDrv_MAD_Write_DSP_sram(MS_U16 dsp_addr, MS_U32 value, AUDIO_DSP_MEMORY_TYPE dsp_memory_type);
extern U32 MDrv_MAD_Read_DSP_sram(U16 u16Dsp_addr,BOOL dsp_memory_type);
void MDrv_T3VIF_WriteByte( U32 u32Reg, U8 u8Val);
void MDrv_SIF_ADC_Reset(void);
extern unsigned int Chip_Query_Rev(void);
void MDrv_MAD_Set_ADC_Threshold(MS_U8 u8Threshold);
MS_U32 MDrv_MAD_Get_PCM_Energy(void);
#endif


