#ifndef _HAL_VD_SINGAL_DEF_H_
#define _HAL_VD_SINGAL_DEF_H_

/* Video Source Type */
typedef enum
{
    E_VIDEOSOURCE_INVALID               =   0x00,               ///< Video source Invalid
    E_VIDEOSOURCE_ATV                   =   0x01,               ///< Video source ATV
    E_VIDEOSOURCE_CVBS1                 =   0x02,               ///< Video source CVBS 1
    E_VIDEOSOURCE_CVBS2                 =   0x03,               ///< Video source CVBS 2
    E_VIDEOSOURCE_CVBS3                 =   0x04,               ///< Video source CVBS 2
    E_VIDEOSOURCE_SVIDEO1               =   0x05,               ///< Video source SVIDEO 1
    E_VIDEOSOURCE_SVIDEO2               =   0x06,               ///< Video source SVIDEO 2
    E_VIDEOSOURCE_SCART1                =   0x07,               ///< Video source SCART 1
    E_VIDEOSOURCE_SCART2                =   0x08,               ///< Video source SCART 2
    E_VIDEOSOURCE_THROUGH_3DCOMB        =   0x50,               ///< Video source through 3D Comb
    E_VIDEOSOURCE_THROUGH_3DCOMB_ATV    =   (E_VIDEOSOURCE_THROUGH_3DCOMB | E_VIDEOSOURCE_ATV),   ///< Video source through 3D Comb ATV
    E_VIDEOSOURCE_THROUGH_3DCOMB_CVBS1  =   (E_VIDEOSOURCE_THROUGH_3DCOMB | E_VIDEOSOURCE_CVBS1),///< Video source through 3D Comb CVBS1
    E_VIDEOSOURCE_THROUGH_3DCOMB_CVBS2  =   (E_VIDEOSOURCE_THROUGH_3DCOMB | E_VIDEOSOURCE_CVBS2),///< Video source through 3D Comb CVBS2
    E_VIDEOSOURCE_THROUGH_3DCOMB_SCART1 =   (E_VIDEOSOURCE_THROUGH_3DCOMB | E_VIDEOSOURCE_SCART1),///< Video source through 3D Comb SCART1
    E_VIDEOSOURCE_THROUGH_3DCOMB_SCART2 =   (E_VIDEOSOURCE_THROUGH_3DCOMB | E_VIDEOSOURCE_SCART2)///< Video source through 3D Comb SCART2
} VIDEOSOURCE_TYPE;

#define IsSrcTypeATV(x)         ( (x)==E_VIDEOSOURCE_ATV )
#define IsSrcTypeAV(x)          ( (x)>=E_VIDEOSOURCE_CVBS1 && (x)<=E_VIDEOSOURCE_CVBS3 )
#define IsSrcTypeSV(x)          ( (x)>=E_VIDEOSOURCE_SVIDEO1 && (x)<=E_VIDEOSOURCE_SVIDEO2 )
#define IsSrcTypeScart(x)       ( (x)>=E_VIDEOSOURCE_SCART1 && (x)<=E_VIDEOSOURCE_SCART2 )

#define IsATVInUse(x)         ( (x)==E_VIDEOSOURCE_ATV )
#define IsAVInUse(x)          ( (x)>=E_VIDEOSOURCE_CVBS1 && (x)<=E_VIDEOSOURCE_CVBS3 )
#define IsSVInUse(x)          ( (x)>=E_VIDEOSOURCE_SVIDEO1 && (x)<=E_VIDEOSOURCE_SVIDEO2 )
#define IsScartInUse(x)       ( (x)>=E_VIDEOSOURCE_SCART1 && (x)<=E_VIDEOSOURCE_SCART2 )

/* Auto AV Source type */
typedef enum
{
    E_AUTOAV_SOURCE_1   =   E_VIDEOSOURCE_SCART1,       ///< Auto AV source SCART 1
    E_AUTOAV_SOURCE_2   =   E_VIDEOSOURCE_SCART2,       ///< Auto AV source SCART 2
    E_AUTOAV_SORUCE_ALL =   0xFF                ///< Auto AV source ALL
} AUTOAV_SOURCE_TYPE;

/* Video Standard type */
typedef enum
{
    E_VIDEOSTANDARD_PAL_BGHI        = 0x00,        ///< Video standard PAL BGHI
    E_VIDEOSTANDARD_NTSC_M          = 0x01,        ///< Video standard NTSC M
    E_VIDEOSTANDARD_SECAM           = 0x02,        ///< Video standard SECAM
    E_VIDEOSTANDARD_NTSC_44         = 0x03,        ///< Video standard  NTSC 44
    E_VIDEOSTANDARD_PAL_M           = 0x04,        ///< Video standard  PAL M
    E_VIDEOSTANDARD_PAL_N           = 0x05,        ///< Video standard  PAL N
    E_VIDEOSTANDARD_PAL_60          = 0x06,        ///< Video standard PAL 60
    E_VIDEOSTANDARD_NOTSTANDARD     = 0x07        ///< NOT Video standard
} VIDEOSTANDARD_TYPE;

/* Color standard select same as VIDEOSTANDARD_TYPE */
#if 0
typedef enum
{
    SIG_NTSC,           ///< NTSC
    SIG_PAL,            ///< PAL
    SIG_SECAM,          ///< SECAM
    SIG_NTSC_443,       ///< NTSC 443
    SIG_PAL_M,          ///< PAL M
    SIG_PAL_NC,         ///< PAL NC
    SIG_NUMS,           ///< signal numbers

    SIG_NONE = -1
}EN_VD_SIGNALTYPE;
#endif

/* SCART Source type */
typedef enum
{
    E_SCART_SRC_TYPE_CVBS,          ///< SCART source type - CVBS
    E_SCART_SRC_TYPE_RGB,          ///< SCART source type - RGB
    E_SCART_SRC_TYPE_SVIDEO,          ///< SCART source type - SVIDEO
    E_SCART_SRC_TYPE_UNKNOWN          ///< SCART source type - Unknown
} SCART_SOURCE_TYPE;


/* Input port is used in ADC multiplexer */
/* Input port type */
typedef enum
{
    INPUT_PORT_ADC_RGB,             ///< 0: ADC RGB
    INPUT_PORT_ADC_YPBPR,           ///< 1: ADC YPBPR
    INPUT_PORT_ADC_YPBPR2,          ///< 2: ADC YPBPR2

    INPUT_PORT_ADC_HDMIA,           ///< HDMI 1
    INPUT_PORT_ADC_HDMIB,           ///< HDMI 2
    INPUT_PORT_ADC_HDMIC,           ///< HDMI 3

    INPUT_PORT_ADC_HDMI,

    INPUT_PORT_MS_CVBS0 = 0x0F,     ///< 0x0F: MS CVBS0
    INPUT_PORT_MS_CVBS1 = 0x1F,     ///< 0x1F: MS CVBS1

    INPUT_PORT_MS_CVBS2 =0x29,      ///< 0x29: MS CVBS2

    INPUT_PORT_MS_CVBS3 = 0x3F,     ///< 0x3F: MS CVBS3

    INPUT_PORT_MS_SV0 = 0x46,       ///< 0x46: MS SV0
    INPUT_PORT_MS_SV1 = 0x57,       ///< 0x57: MS SV7

    INPUT_PORT_AV_SCART0 = 0x49,    ///< 0x49: AV SCART0
    INPUT_PORT_AV_SCART1 = 0x28,    ///< 0x28: AV SCART1

    INPUT_PORT_MS_STORAGE = 0xFC,       ///< 0xFE MS DTV
    INPUT_PORT_MS_CCIR656 = 0xFD,   ///< 0xFE MS CCIR656
    INPUT_PORT_MS_DTV = 0xFE,       ///< 0xFE MS DTV
    INPUT_PORT_NUMS,                 ///< Numbers of port type
    INPUT_PORT_NONE = INPUT_PORT_NUMS,
} INPUT_PORT_TYPE;

/* Port check macro */
#define IsUseInternalAVPort(x)      (x>=INPUT_PORT_MS_CVBS0&&x<=INPUT_PORT_MS_CVBS3)
#define IsUseInternalSVPort(x)      (x==INPUT_PORT_MS_SV0||x==INPUT_PORT_MS_SV1)
#define IsUseInternalScartPort(x)   ((x==INPUT_PORT_AV_SCART0)||(x==INPUT_PORT_AV_SCART1))

/* Video switch Setting */
#define _FUNC_NOT_USED()                //do {} while ( 0 )

#define Switch_YPbPr1()                 _FUNC_NOT_USED()//GPIO_SET( PIN_130, 0 )  //VGA RGB in
#define Switch_YPbPr2()                 _FUNC_NOT_USED()//GPIO_SET( PIN_130, 1 )  //scart RGB in



/* Video frequency */
typedef enum
{
    E_VIDEO_FQ_NOSIGNAL    = 0, ///< Video Frequency No signal
    E_VIDEO_FQ_50Hz        = 50,    ///< Video Frequency 50Hz
    E_VIDEO_FQ_60Hz        = 60 ///< Video Frequency 60Hz
} VIDEOFREQ;

typedef enum
{
    E_FREERUN_FQ_AUTO            = 0x00,
    E_FREERUN_FQ_50Hz            = 0x01,
    E_FREERUN_FQ_60Hz            = 0x02,
} FREERUNFREQ;


#define VIDEO_50Hz_NLPF            312
#define VIDEO_60Hz_NLPF            262

//------------------------------------------------------------------------------
// MUX
typedef enum
{
    ADC_RGB1,
    ADC_RGB2,
    ADC_RGB3,
}AMUX_SEL;

typedef enum // For PC/YPbPr input mux
{
    ANALOG_RGB0 = ADC_RGB1,
    ANALOG_RGB1 = ADC_RGB2,
    ANALOG_RGB2 = ADC_RGB3,
    ANALOG_RGB_DUMMY,
}ANALOG_RGB;

typedef enum
{
    MSVD_YMUX_CVBS0,
    MSVD_YMUX_CVBS1,
    MSVD_YMUX_CVBS2,
    MSVD_YMUX_CVBS3,

    MSVD_YMUX_Y0,
    MSVD_YMUX_Y1,
    MSVD_YMUX_C0,
    MSVD_YMUX_C1,

    MSVD_YMUX_SOG0 = 8,
    MSVD_YMUX_SOG1,
    MSVD_YMUX_SOG2,

    MSVD_YMUX_G0 = 11,
    MSVD_YMUX_G1 = 12,
    MSVD_YMUX_G2 = 13,
    MSVD_YMUX_CVBS4 = MSVD_YMUX_Y0,
    MSVD_YMUX_CVBS5 = MSVD_YMUX_Y1,
    MSVD_YMUX_CVBS6 = MSVD_YMUX_C0,
    MSVD_YMUX_CVBS7 = MSVD_YMUX_C1,
    MSVD_YMUX_NONE = 0x1F,

    MSVD_YMUX_DUMMY,
}MS_VD_YMUX;

typedef enum
{
    MSVD_CMUX_CVBS0,
    MSVD_CMUX_CVBS1,
    MSVD_CMUX_CVBS2,
    MSVD_CMUX_CVBS3,

    MSVD_CMUX_Y0 = 4,
    MSVD_CMUX_Y1,
    MSVD_CMUX_C0,
    MSVD_CMUX_C1,

    MSVD_CMUX_SOG0 = 8,
    MSVD_CMUX_SOG1,
    MSVD_CMUX_SOG2,

    MSVD_CMUX_R0 = 11,
    MSVD_CMUX_R1 = 12,
    MSVD_CMUX_R2 = 13,
    MSVD_CMUX_CVBS4 = MSVD_CMUX_Y0,
    MSVD_CMUX_CVBS5 = MSVD_CMUX_C0,
    MSVD_CMUX_CVBS6 = MSVD_CMUX_Y1,
    MSVD_CMUX_CVBS7 = MSVD_CMUX_C1,
    MSVD_CMUX_NONE = 0x0F,

    MSVD_CMUX_DUMMY,
}MS_VD_CMUX;

//------------------------------------------------------------------------------
// SCART

// === Scart ID Level ===
// Level 0: 0V ~ 2V
// Level 1A: 4.5V ~ 7V (aspect ratio 16:9)
// Level 1B: 9.5V ~ 12V
#define SCART_ID_LEVEL_0V           0
//#define SCART_ID_LEVEL_1V         0
//#define SCART_ID_LEVEL_2V         0
#define SCART_ID_LEVEL_3V         	15
#define SCART_ID_LEVEL_4V           20
//#define SCART_ID_LEVEL_4p5V       35
//#define SCART_ID_LEVEL_5V         35
//#define SCART_ID_LEVEL_6V         35
//#define SCART_ID_LEVEL_7V         35
#define SCART_ID_LEVEL_8V           41
//#define SCART_ID_LEVEL_9V         60
//#define SCART_ID_LEVEL_9p5V       60
//#define SCART_ID_LEVEL_10V        60

#define SwitchRGBToSCART()				Switch_YPbPr2()
#define	SwitchRGBToDSUB()				Switch_YPbPr1()


typedef enum // For specify scart RGB input
{
    SCART_RGB0 = 0x00,
    SCART_RGB1,
    SCART_RGB2,
    SCART_RGB_DUMMY,
}SCART_RGB;

typedef enum    // 0x2580[5:4]
{
    SCART_FB_NONE = 0x00,
    SCART_FB0,
    SCART_FB1,
    SCART_FB2,
}SCART_FB;

#define SCART_RGB_NONE  0xFF


//------Input Source Mux--------------------------------------------------------
#define INPUT_VGA_MUX               ANALOG_RGB1
#define INPUT_YPBPR_MUX             ANALOG_RGB0
//#define INPUT_YPBPR2_MUX			ANALOG_RGB1
#define INPUT_TV_YMUX               MSVD_YMUX_CVBS0
#define INPUT_AV_YMUX               MSVD_YMUX_CVBS1
#define INPUT_AV2_YMUX              MSVD_YMUX_NONE
#define INPUT_AV3_YMUX              MSVD_YMUX_NONE
#define INPUT_SV_YMUX               MSVD_YMUX_Y1
#define INPUT_SV_CMUX               MSVD_CMUX_C1
#define INPUT_SV2_YMUX              MSVD_YMUX_Y0
#define INPUT_SV2_CMUX              MSVD_CMUX_C0
#define INPUT_SCART_YMUX            MSVD_YMUX_CVBS1
#define INPUT_SCART_CMUX            SCART_RGB2
#define INPUT_SCART_FB_MUX          SCART_FB0
#define INPUT_SCART2_YMUX           MSVD_YMUX_CVBS2
#define INPUT_SCART2_CMUX           SCART_RGB1  //?
#define INPUT_SCART2_FB_MUX         SCART_FB1   //?

/// Aspect ratio type
typedef enum
{
    ARC4x3_FULL,                                ///< Aspect ratio 4:3 Full
    ARC14x9_LETTERBOX_CENTER,                                ///< Aspect ratio 14:9 letterbox center
    ARC14x9_LETTERBOX_TOP,                                ///< Aspect ratio 14:9 letterbox TOP
    ARC16x9_LETTERBOX_CENTER,                                ///< Aspect ratio 16:9 letterbox center
    ARC16x9_LETTERBOX_TOP,                                ///< Aspect ratio 16:9 letterbox TOP
    ARC_ABOVE16x9_LETTERBOX_CENTER,                                ///< Aspect ratio Above 16:9 letterbox center
    ARC14x9_FULL_CENTER,                                ///< Aspect ratio 14:9 full center
    ARC16x9_ANAMORPHIC,                                ///< Aspect ratio 16:9 anamorphic
    ARC_INVALID                                ///< Invalid Aspect ratio
} ASPECT_RATIO_TYPE;

#endif
