#ifndef TITANIA3_CHIP_INT_H
#define TITANIA3_CHIP_INT_H

#define TITANIAINT_END 128//64

#if defined(CONFIG_Titania3)

#define VPE_TO_HANDLE_INT   0
// Samuel.090325
// It's werid that the interrupt goes wrong way, need to check with Janick

#if VPE_TO_HANDLE_INT
//0xbf203200
// FOR T3
#define REG_INT_BASEADR                 0xBF203200

//IRQ registers
#define REG_IRQ_MASK_L                  (REG_INT_BASEADR+0x54*4)
#define REG_IRQ_MASK_H                  (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_FORCE_L                 (REG_INT_BASEADR+0x50*4)
#define REG_IRQ_FORCE_H                 (REG_INT_BASEADR+0x51*4)
//#define REG_IRQ_RAW_L                 (REG_INT_BASEADR+0x55*4)
//#define REG_IRQ_RAW_H                 (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_PENDING_L               (REG_INT_BASEADR+0x5C*4)
#define REG_IRQ_PENDING_H               (REG_INT_BASEADR+0x5D*4)

//IRQ EXP registers
#define REG_IRQ_EXP_MASK_L              (REG_INT_BASEADR+0x56*4)
#define REG_IRQ_EXP_MASK_H              (REG_INT_BASEADR+0x57*4)
#define REG_IRQ_EXP_FORCE_L             (REG_INT_BASEADR+0x52*4)
#define REG_IRQ_EXP_FORCE_H             (REG_INT_BASEADR+0x53*4)
//#define REG_IRQ_EXP_RAW_L             (REG_INT_BASEADR+0x55*4)
//#define REG_IRQ_EXP_RAW_H             (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_EXP_PENDING_L           (REG_INT_BASEADR+0x5E*4)
#define REG_IRQ_EXP_PENDING_H           (REG_INT_BASEADR+0x5F*4)

//FIQ registers
#define REG_FIQ_MASK_L                  (REG_INT_BASEADR+0x44*4)
#define REG_FIQ_MASK_H                  (REG_INT_BASEADR+0x45*4)
#define REG_FIQ_FORCE_L                 (REG_INT_BASEADR+0x40*4)
#define REG_FIQ_FORCE_H                 (REG_INT_BASEADR+0x41*4)
#define REG_FIQ_CLEAR_L                 (REG_INT_BASEADR+0x4C*4)
#define REG_FIQ_CLEAR_H                 (REG_INT_BASEADR+0x4D*4)
//#define REG_FIQ_RAW_L                 (REG_INT_BASEADR+0x4C*4)
//#define REG_FIQ_RAW_H                 (REG_INT_BASEADR+0x4D*4)
#define REG_FIQ_PENDING_L               (REG_INT_BASEADR+0x4C*4)
#define REG_FIQ_PENDING_H               (REG_INT_BASEADR+0x4D*4)

//FIQ EXP registers
#define REG_FIQ_EXP_MASK_L              (REG_INT_BASEADR+0x46*4)
#define REG_FIQ_EXP_MASK_H              (REG_INT_BASEADR+0x47*4)
#define REG_FIQ_EXP_FORCE_L             (REG_INT_BASEADR+0x42*4)
#define REG_FIQ_EXP_FORCE_H             (REG_INT_BASEADR+0x43*4)
#define REG_FIQ_EXP_CLEAR_L             (REG_INT_BASEADR+0x4E*4)
#define REG_FIQ_EXP_CLEAR_H             (REG_INT_BASEADR+0x4F*4)
//#define REG_FIQ_EXP_RAW_L             (REG_INT_BASEADR+0x4E*4)
//#define REG_FIQ_EXP_RAW_H             (REG_INT_BASEADR+0x4F*4)
#define REG_FIQ_EXP_PENDING_L           (REG_INT_BASEADR+0x4E*4)
#define REG_FIQ_EXP_PENDING_H           (REG_INT_BASEADR+0x4F*4)


#else

//0xbf203200
// FOR T3
#define REG_INT_BASEADR                 0xBF203200

//IRQ registers
#define REG_IRQ_MASK_L                  (REG_INT_BASEADR+0x74*4)
#define REG_IRQ_MASK_H                  (REG_INT_BASEADR+0x75*4)
#define REG_IRQ_FORCE_L                 (REG_INT_BASEADR+0x70*4)
#define REG_IRQ_FORCE_H                 (REG_INT_BASEADR+0x71*4)
//#define REG_IRQ_RAW_L                 (REG_INT_BASEADR+0x55*4)
//#define REG_IRQ_RAW_H                 (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_PENDING_L               (REG_INT_BASEADR+0x7C*4)
#define REG_IRQ_PENDING_H               (REG_INT_BASEADR+0x7D*4)

//IRQ EXP registers
#define REG_IRQ_EXP_MASK_L              (REG_INT_BASEADR+0x76*4)
#define REG_IRQ_EXP_MASK_H              (REG_INT_BASEADR+0x77*4)
#define REG_IRQ_EXP_FORCE_L             (REG_INT_BASEADR+0x72*4)
#define REG_IRQ_EXP_FORCE_H             (REG_INT_BASEADR+0x73*4)
//#define REG_IRQ_EXP_RAW_L             (REG_INT_BASEADR+0x55*4)
//#define REG_IRQ_EXP_RAW_H             (REG_INT_BASEADR+0x55*4)
#define REG_IRQ_EXP_PENDING_L           (REG_INT_BASEADR+0x7E*4)
#define REG_IRQ_EXP_PENDING_H           (REG_INT_BASEADR+0x7F*4)

//FIQ registers
#define REG_FIQ_MASK_L                  (REG_INT_BASEADR+0x64*4)
#define REG_FIQ_MASK_H                  (REG_INT_BASEADR+0x65*4)
#define REG_FIQ_FORCE_L                 (REG_INT_BASEADR+0x60*4)
#define REG_FIQ_FORCE_H                 (REG_INT_BASEADR+0x61*4)
#define REG_FIQ_CLEAR_L                 (REG_INT_BASEADR+0x6C*4)
#define REG_FIQ_CLEAR_H                 (REG_INT_BASEADR+0x6D*4)
//#define REG_FIQ_RAW_L                 (REG_INT_BASEADR+0x4C*4)
//#define REG_FIQ_RAW_H                 (REG_INT_BASEADR+0x4D*4)
#define REG_FIQ_PENDING_L               (REG_INT_BASEADR+0x6C*4)
#define REG_FIQ_PENDING_H               (REG_INT_BASEADR+0x6D*4)

//FIQ EXP registers
#define REG_FIQ_EXP_MASK_L              (REG_INT_BASEADR+0x66*4)
#define REG_FIQ_EXP_MASK_H              (REG_INT_BASEADR+0x67*4)
#define REG_FIQ_EXP_FORCE_L             (REG_INT_BASEADR+0x62*4)
#define REG_FIQ_EXP_FORCE_H             (REG_INT_BASEADR+0x63*4)
#define REG_FIQ_EXP_CLEAR_L             (REG_INT_BASEADR+0x6E*4)
#define REG_FIQ_EXP_CLEAR_H             (REG_INT_BASEADR+0x6F*4)
//#define REG_FIQ_EXP_RAW_L             (REG_INT_BASEADR+0x4E*4)
//#define REG_FIQ_EXP_RAW_H             (REG_INT_BASEADR+0x4F*4)
#define REG_FIQ_EXP_PENDING_L           (REG_INT_BASEADR+0x6E*4)
#define REG_FIQ_EXP_PENDING_H           (REG_INT_BASEADR+0x6F*4)


//PM IRQ
#define REG_PM_INT_BASEADR              0xBF005600

//PM IRQ registers
#define REG_IRQ_MASK                    (REG_PM_INT_BASEADR+0x14*4)
#define REG_IRQ_FORCE                   (REG_PM_INT_BASEADR+0x10*4)
#define REG_IRQ_PENDING                 (REG_PM_INT_BASEADR+0x1C*4)

//PM FIQ registers
#define REG_FIQ_MASK                    (REG_PM_INT_BASEADR+0x4*4)
#define REG_FIQ_FORCE                   (REG_PM_INT_BASEADR+0x0*4)
#define REG_FIQ_PENDING                 (REG_PM_INT_BASEADR+0xC*4)

#endif

#endif



#if defined(CONFIG_Titania2)
// FOR T2
//IRQ registers
#define REG_IRQ_MASK_L              0xBF800AB0
#define REG_IRQ_MASK_H              0xBF800AB4
#define REG_IRQ_FORCE_L             0xBF800AB8
#define REG_IRQ_FORCE_H             0xBF800ABC
#define REG_IRQ_RAW_L               0xBF800AC8
#define REG_IRQ_RAW_H               0xBF800ACC
#define REG_IRQ_PENDING_L           0xBF800AD0
#define REG_IRQ_PENDING_H           0xBF800AD4

//IRQ EXP registers
#define REG_IRQ_EXP_MASK_L              0xBF803330
#define REG_IRQ_EXP_MASK_H              0xBF803334
#define REG_IRQ_EXP_FORCE_L             0xBF803338
#define REG_IRQ_EXP_FORCE_H             0xBF80333C
#define REG_IRQ_EXP_RAW_L               0xBF803348
#define REG_IRQ_EXP_RAW_H               0xBF80334C
#define REG_IRQ_EXP_PENDING_L           0xBF803350
#define REG_IRQ_EXP_PENDING_H           0xBF803354

//FIQ registers
#define REG_FIQ_MASK_L              0xBF800A80
#define REG_FIQ_MASK_H              0xBF800A84
#define REG_FIQ_FORCE_L             0xBF800A88
#define REG_FIQ_FORCE_H             0xBF800A8C
#define REG_FIQ_CLEAR_L             0xBF800A90
#define REG_FIQ_CLEAR_H             0xBF800A94
#define REG_FIQ_RAW_L               0xBF800A98
#define REG_FIQ_RAW_H               0xBF800A9C
#define REG_FIQ_PENDING_L           0xBF800AA0
#define REG_FIQ_PENDING_H           0xBF800AA4

//FIQ EXP registers
#define REG_FIQ_EXP_MASK_L              0xBF803200
#define REG_FIQ_EXP_MASK_H              0xBF803204
#define REG_FIQ_EXP_FORCE_L             0xBF803208
#define REG_FIQ_EXP_FORCE_H             0xBF80320C
#define REG_FIQ_EXP_CLEAR_L             0xBF803210
#define REG_FIQ_EXP_CLEAR_H             0xBF803214
#define REG_FIQ_EXP_RAW_L               0xBF803218
#define REG_FIQ_EXP_RAW_H               0xBF80321C
#define REG_FIQ_EXP_PENDING_L           0xBF803220
#define REG_FIQ_EXP_PENDING_H           0xBF803224
#endif

#define MSTAR_INT_BASE  8
/*******************************************************/
/*   THE IRQ AND FIQ ARE NOT COMPLETED.                */
/*   FOR EACH IP OWNER, PLEASE REVIEW IT BY YOURSELF   */
/*******************************************************/
// Non-PM Interrup
typedef enum
{
    //IRQ
    E_IRQL_START            = MSTAR_INT_BASE,
    E_IRQ_UART              = E_IRQL_START+0,   /*Titania PIU UART for MIPS*/
    E_IRQ_MVD               = E_IRQL_START+3,

    E_IRQ_AESDMA            = E_IRQL_START+4,
#if 0
    E_IRQ_UART0             = E_IRQL_START+4,
    E_IRQ_UART1             = E_IRQL_START+5,
    E_IRQ_IIC1              = E_IRQL_START+6,
#endif

    E_IRQ_UHC               = E_IRQL_START+7,            /* 1st EHCI */

#if 0
    E_IRQ_SECOND_UHC        = E_IRQL_START+34,        /* tony add for 2st EHCI */
    E_IRQ_SPI_ARB_DUP_REG   = E_IRQL_START+8,
#endif

    E_IRQ_EMAC                = E_IRQL_START+9,
    E_IRQ_DEB               = E_IRQL_START+10,

#if 0
    E_IRQ_UART2             = E_IRQL_START+11,
    E_IRQ_MICOM             = E_IRQL_START+12,
#endif
    E_IRQ_SBM               = E_IRQL_START+13,
    E_IRQ_COMB               = E_IRQL_START+14,

#if 0
    E_IRQ_ECC_DERR          = E_IRQL_START+15,
#endif
    E_IRQL_END              = 15+MSTAR_INT_BASE,


    E_IRQH_START            = 16+MSTAR_INT_BASE,
    E_IRQ_TSP               = E_IRQH_START+0,
    E_IRQ_VE                = E_IRQH_START+1,
    E_IRQ_CIMAX2MCU         = E_IRQH_START+2,
    E_IRQ_DC                = E_IRQH_START+3,
    E_IRQ_GOP               = E_IRQH_START+4,
    E_IRQ_PCM2MCU           = E_IRQH_START+5,
    E_IRQ_IIC0              = E_IRQH_START+6,
    E_IRQ_RTC               = E_IRQH_START+7,
    E_IRQ_KEYPAD            = E_IRQH_START+8,
    E_IRQ_PM                = E_IRQH_START+9,
    E_IRQ_D2B               = E_IRQH_START+10,  //2008/10/23 Nick
    E_IRQ_VBI               = E_IRQH_START+12,
    E_IRQ_M4V               = E_IRQH_START+13,
#if 0
    E_IRQ_MLINK             = E_IRQH_START+14,   // TODO: ?
#endif
    E_IRQ_HDMITX            = E_IRQH_START+15,
    E_IRQH_END              = 31+MSTAR_INT_BASE,


    //IRQ EXP
    E_IRQL_EXP_START        = 32+MSTAR_INT_BASE,
    E_IRQ_SVD               = E_IRQL_EXP_START+0,
    E_IRQ_USB1              = E_IRQL_EXP_START+1,
    E_IRQ_UHC1              = E_IRQL_EXP_START+2,
    E_IRQ_MIU               = E_IRQL_EXP_START+3,
    E_IRQ_UART1             = E_IRQL_EXP_START+7,
    E_IRQ_UART2             = E_IRQL_EXP_START+8,
    E_IRQ_DIP               = E_IRQL_EXP_START+14,

    E_IRQ_M4VE              = E_IRQL_EXP_START+15,
    E_IRQL_EXP_END          = 47+MSTAR_INT_BASE,

    E_IRQH_EXP_START        = 48+MSTAR_INT_BASE,
    E_IRQH_EXP_HDMI         = E_IRQH_EXP_START+4,
    E_IRQH_EXP_END          = 63+MSTAR_INT_BASE,


    //FIQ
    E_FIQL_START            = 64+MSTAR_INT_BASE,
    E_FIQ_DSP2_TO_MIPS      = E_FIQL_START+14,
    E_FIQL_END              = 79+MSTAR_INT_BASE,

    E_FIQH_START            = 80+MSTAR_INT_BASE,
    E_FIQ_VSYNC_VE4VBI      = E_FIQH_START+5,
    E_FIQ_FIELD_VE4VBI      = E_FIQH_START+6,
#if 0
    E_FIQ_VSYN_COP          = E_FIQH_START+9,
    E_FIQ_VSYN_GOP1         = E_FIQH_START+10,
    E_FIQ_VSYN_GOP0         = E_FIQH_START+11,
#endif
    E_FIQ_IR                = E_FIQH_START+11,
    E_FIQ_DSP2UP            = E_FIQH_START+13,
    E_FIQ_DSP2MIPS          = E_FIQH_START+15,
    E_FIQH_END              = 95+MSTAR_INT_BASE,


    //FIQ EXP

    E_FIQL_EXP_START            = 96+MSTAR_INT_BASE,

    E_FIQL_EXP_INT_8051_TO_MIPS = E_FIQL_EXP_START+4,
    E_FIQ_INT_8051_TO_MIPS      = E_FIQL_EXP_START+4,

    E_FIQL_EXP_INT_8051_TO_AEON = E_FIQL_EXP_START+6,
    E_FIQ_INT_8051_TO_AEON      = E_FIQL_EXP_START+6,

    E_FIQL_EXP_INT_AEON_TO_MIPS = E_FIQL_EXP_START+8,
    E_FIQ_INT_AEON_TO_MIPS      = E_FIQL_EXP_START+8,

    E_FIQL_EXP_INT_AEON_TO_8051 = E_FIQL_EXP_START+10,
    E_FIQ_INT_AEON_TO_8051      = E_FIQL_EXP_START+10,

    E_FIQL_EXP_END              = 111+MSTAR_INT_BASE,

    E_FIQH_EXP_START            = 112+MSTAR_INT_BASE,

    E_FIQH_EXP_INT_MIPS_TO_AEON = E_FIQH_EXP_START+1,
    E_FIQ_INT_MIPS_TO_AEON      = E_FIQH_EXP_START+1,

    E_FIQH_EXP_INT_MIPS_TO_8051 = E_FIQH_EXP_START+2,
    E_FIQ_INT_MIPS_TO_8051      = E_FIQH_EXP_START+2,

    E_FIQH_EXP_END              = 127+MSTAR_INT_BASE,

    E_IRQ_FIQ_ALL               = 0xFF //all IRQs & FIQs

} InterruptNum;

//IRQ Low 16 bits
#define IRQL_UART                   (0x1 << (E_IRQ_UART  - E_IRQL_START))
#define IRQL_MVD                    (0x1 << (E_IRQ_MVD   - E_IRQL_START))
#define IRQL_UHC                    (0x1 << (E_IRQ_UHC   - E_IRQL_START))
#if 0
#define IRQL_UART0                  (0x1 << (E_IRQ_UART0 - E_IRQL_START))
#endif
#define IRQL_EMAC                   (0x1 << (E_IRQ_EMAC  - E_IRQL_START))
#define IRQL_DEB                    (0x1 << (E_IRQ_DEB   - E_IRQL_START))
#define IRQL_EMAC                   (0x1 << (E_IRQ_EMAC  - E_IRQL_START))
#define IRQL_MICOM                  (0x1 << (E_IRQ_MICOM - E_IRQL_START))
#define IRQL_COMB                   (0x1 << (E_IRQ_COMB - E_IRQL_START))
#define IRQL_ALL                    0xFFFF //[15:0]

//IRQ High 16 bits
#define IRQH_TSP                    (0x1 << (E_IRQ_TSP      - E_IRQH_START)   )
#define IRQH_VE                     (0x1 << (E_IRQ_VE       - E_IRQH_START)   )
#define IRQH_CIMAX2MCU              (0x1 << (E_IRQ_CIMAX2MCU- E_IRQH_START)   )
#define IRQH_DC                     (0x1 << (E_IRQ_DC       - E_IRQH_START)   )
#define IRQH_GOP                    (0x1 << (E_IRQ_GOP      - E_IRQH_START)   )
#define IRQH_PCM2MCU                (0x1 << (E_IRQ_PCM2MCU  - E_IRQH_START)   )
#define IRQH_IIC0                   (0x1 << (E_IRQ_IIC0     - E_IRQH_START)   )
#define IRQH_RTC                    (0x1 << (E_IRQ_RTC      - E_IRQH_START)   )
#define IRQH_KEYPAD                 (0x1 << (E_IRQ_KEYPAD   - E_IRQH_START)   )
#define IRQH_PM                     (0x1 << (E_IRQ_PM       - E_IRQH_START)   )
#define IRQH_D2B                    (0x1 << (E_IRQ_D2B      - E_IRQH_START)   )//2008/10/23 Nick
#define IRQH_VBI                    (0x1 << (E_IRQ_VBI      - E_IRQH_START)   )
#define IRQH_M4V                    (0x1 << (E_IRQ_M4V      - E_IRQH_START)   )
#if 0
#define IRQH_MLINK                  (0x1 << (E_IRQ_MLINK    - E_IRQH_START)   )
#endif
#define IRQH_HDMITX                 (0x1 << (E_IRQ_HDMITX   - E_IRQH_START)   )
#define IRQH_ALL                    0xFFFF //[15:0]


//FIQ Low 16 bits
#define FIQL_DSP2_TO_MIPS           (0x1 << (E_FIQ_DSP2_TO_MIPS - E_FIQL_START)  )
#define FIQL_ALL                    0xFFFF //[15:0]


//FIQ High 16 bits
#define FIQH_DSP2UP                 (0x1 << (E_FIQ_DSP2UP - E_FIQH_START)  )
#if 0
#define FIQH_VSYN_GOP1              (0x1 << (E_FIQ_VSYN_GOP1 - E_FIQH_START)  )
#define FIQH_VSYN_GOP0              (0x1 << (E_FIQ_VSYN_GOP0 - E_FIQH_START)  )
#endif
#define FIQH_IR                     (0x1 << (E_FIQ_IR - E_FIQH_START)   )
#define FIQH_VSYNC_VE4VBI           (0x1 << (E_FIQ_VSYNC_VE4VBI - E_FIQH_START)   )
#define FIQH_FIELD_VE4VBI           (0x1 << (E_FIQ_FIELD_VE4VBI - E_FIQH_START)   )
#define FIQH_DSP2MIPS               (0x1 << (E_FIQ_DSP2MIPS - E_FIQH_START)  )

//IRQ EXP Low 16 bits
#define IRQ_SVD                     (0x1 << (E_IRQ_SVD  - E_IRQL_EXP_START))
#define IRQ_USB1                    (0x1 << (E_IRQ_USB1   - E_IRQL_EXP_START))
#define IRQ_UHC1                    (0x1 << (E_IRQ_UHC1   - E_IRQL_EXP_START))
#define IRQ_MIU                     (0x1 << (E_IRQ_MIU - E_IRQL_EXP_START))
#define IRQL_UART2                  (0x1 << (E_IRQ_UART2 - E_IRQL_EXP_START))
#define IRQL_UART1                  (0x1 << (E_IRQ_UART1 - E_IRQL_EXP_START))
#define IRQ_DIP                     (0x1 << (E_IRQ_DIP - E_IRQL_EXP_START))
#define IRQ_M4VE                    (0x1 << (E_IRQ_M4VE - E_IRQL_EXP_START))
#define IRQL_EXP_ALL                0xFFFF //[15:0]

//IRQ EXP High 16 bits
#define IRQH_EXP_HDMI               (0x1 << (E_IRQH_EXP_HDMI  - E_IRQH_EXP_START))
#define IRQH_EXP_ALL                0xFFFF //[15:0]

//FIQ EXP Low 16 bits
#define FIQL_EXP_AEON_TO_8051       (0x1 << (E_FIQL_EXP_INT_AEON_TO_8051 - E_FIQL_EXP_START)  )
#define FIQL_EXP_8051_TO_AEON       (0x1 << (E_FIQL_EXP_INT_8051_TO_AEON - E_FIQL_EXP_START)  )
#define FIQL_EXP_8051_TO_MIPS       (0x1 << (E_FIQL_EXP_INT_8051_TO_MIPS - E_FIQL_EXP_START)  )
#define FIQL_EXP_AEON_TO_MIPS       (0x1 << (E_FIQL_EXP_INT_AEON_TO_MIPS - E_FIQL_EXP_START)  )
#define FIQL_EXP_ALL                0xFFFF //[15:0]

//FIQ EXP High 16 bits
#define FIQH_EXP_MIPS_TO_8051       (0x1 << (E_FIQH_EXP_INT_MIPS_TO_8051 - E_FIQL_EXP_START)  )
#define FIQH_EXP_MIPS_TO_AEON       (0x1 << (E_FIQH_EXP_INT_MIPS_TO_AEON - E_FIQL_EXP_START)  )
#define FIQH_EXP_ALL                0xFFFF //[15:0]

#define FIQH_ALL                    0xFFFF //[15:0]


/*******************************************************/
/*   THE IRQ AND FIQ ARE NOT COMPLETED.                */
/*   FOR EACH IP OWNER, PLEASE REVIEW IT BY YOURSELF   */
/*******************************************************/
// PM Interrup
typedef enum
{
    //PM IRQ
    E_PM_IRQL_START            = MSTAR_INT_BASE,
    E_PM_IRQ_MICOM                = E_PM_IRQL_START+2,
    E_PM_IRQL_END              = 15+MSTAR_INT_BASE,


    //PM FIQ
    E_PM_FIQL_START            = 16+MSTAR_INT_BASE,
    E_PM_FIQH_END              = 31+MSTAR_INT_BASE,


    E_PM_IRQ_FIQ_ALL           = 0xFF //all IRQs & FIQs

} PM_InterruptNum;

//IRQ Low 16 bits
#define PM_IRQL_MICOM                  (0x1 << (E_PM_IRQ_MICOM  - E_PM_IRQL_START))
#define PM_IRQL_ALL                    0xFFFF //[15:0]


//FIQ Low 16 bits
#define PM_FIQL_ALL                    0xFFFF //[15:0]


void Titania_DisableInterrupt_str (void);

#endif

