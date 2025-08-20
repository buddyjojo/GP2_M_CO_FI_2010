////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
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
#ifndef MDRV_SCALER__H
#define MDRV_SCALER__H

#include <common.h>

//
// Scaler setting
//-----------------------------------------------------------------
#define REG_ADDR(addr)              (*((volatile unsigned short int*)(0xBF000000 + (addr << 1))))

#define REG_CKGEN0_BASE             0x100B00
//
// Ckgen0 register definition
//
#define REG_CKGEN0(_x_)             (REG_CKGEN0_BASE | (_x_ << 1))

#define REG_SCALER_BASE             0x102F00
#define REG_SC_BANK(_x_)            (REG_SCALER_BASE | (_x_ << 1))

// LPLL
#define REG_LPLL_BASE               0x103100
#define REG_LPLL(_x_)               (REG_LPLL_BASE | (_x_ << 1))

#define FREQ_12MHZ                  (12000000UL)
#define MST_XTAL_CLOCK_HZ           FREQ_12MHZ  //Neptune: FREQ_12MHZ  //FPGA:FREQ_14P318MHZ

#define REG_MOD_BASE                0x103200
#define MOD_REG(_x_)                (REG_MOD_BASE | (_x_ << 1))

#define MST_XTAL_CLOCK_KHZ          12000

#define REG_SC_BK_GOPINT            0x00
#define REG_SC_BK_VOP               0x10
#define REG_SC_BK_SCMI              0x12
#define REG_SC_BK_0X12              0x12
#define REG_SC_BK_VOP2_RP           0x2D
#define REG_SC_BK_PEAKING           0x19
#define REG_SC_GOPINT(_x_)          REG_SC_BANK(_x_)
#define REG_SC_VOP(_x_)             REG_SC_BANK(_x_)
#define REG_SC_SCMI(_x_)            REG_SC_BANK(_x_)
#define REG_SC_PEAKING(_x_)            REG_SC_BANK(_x_)

#define REG_GENERAL_BASE            0x100000

#define REG_GENERAL_BANK(_x_)       (REG_GENERAL_BASE | _x_)

// store bank
#define SC_BK_STORE     \
        U8 u8Bank;      \
		rmb();          \
        u8Bank = REG_ADDR(REG_SCALER_BASE)

// restore bank
#define SC_BK_RESTORE   \
        REG_ADDR(REG_SCALER_BASE) = u8Bank;   \
        wmb()

// switch bank
#define SC_BK_SWICH(_x_)\
        REG_ADDR(REG_SCALER_BASE) = _x_;\
        wmb()

//
// MOD bank
//
#define REG_MOD_BK_00                0x00
#define REG_MOD_BK_01                0x01

// store MOD bank
#define MOD_BK_STORE     \
        U8 u8Bank_mod;      \
		rmb();          \
        u8Bank_mod = REG_ADDR(REG_MOD_BASE)

// restore MOD bank
#define MOD_BK_RESTORE   \
        REG_WL(REG_MOD_BASE, u8Bank_mod);   \
        wmb()

// switch MOD bank
#define MOD_BK_SWICH(_x_)\
        REG_WL(REG_MOD_BASE, _x_);\
        wmb()

// write low byte
#define REG_WL(_reg_, _val_)    \
        do{ REG_ADDR(_reg_) = (REG_ADDR(_reg_) & 0xFF00) | ((_val_) & 0x00FF); wmb(); }while(0)

// write 2 bytes
#define REG_WR(_reg_, _val_)        do{ REG_ADDR(_reg_) = (_val_); wmb(); }while(0)

// write 3 bytes
#define REG_W3(_reg_, _val_)               \
        do{ REG_WR(_reg_, (_val_));            \
        REG_WL((_reg_ + 2), (_val_) >> 16);\
        wmb(); }while(0)

// write 4 bytes
#define REG_W4(_reg_, _val_)                 \
        do {REG_WR((_reg_), (_val_));  wmb();    \
        REG_WR((_reg_ + 2), ((_val_) >> 16)); wmb(); }while(0)

// write mask
#define REG_WM(_reg_, _val_, _msk_)    \
		do{ rmb();                         \
        REG_ADDR(_reg_) = (REG_ADDR(_reg_) & ~(_msk_)) | ((_val_) & _msk_); wmb(); }while(0)

// write high byte
#define REG_WH(_reg_, _val_)    \
        do{ REG_ADDR(_reg_) = (REG_ADDR(_reg_)  & 0x00FF) | ((_val_) << 8); wmb(); }while(0)

// write bit
#define REG_WI(_reg_, _bit_, _pos_)    \
		do{ rmb();                         \
        REG_ADDR(_reg_) = (_bit_) ? (REG_ADDR(_reg_) | _pos_) : (REG_ADDR(_reg_) & ~(_pos_));  \
        wmb(); }while(0)
//GPIO reset
#define REG_CHIP_BASE              	0xBF203C00
#define REG_MIPS_BASE              	0xBF000000//Use 8 bit addressing
#define MHal_GPIO_REG(addr)    		(*(volatile U8*)(REG_MIPS_BASE + (((addr) & ~1)<<1) + (addr & 1)))

#define REG_TEST_MODE                (0x101e24)

#define REG_GPIO8_OEN               (0x101e5d)
#define REG_GPIO8_OUT               (0x101e57)
#define REG_GPIO9_OEN               (0x101e5d)
#define REG_GPIO9_OUT               (0x101e57)


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
#define REG_RR(_reg_)                    ({ REG_ADDR(_reg_);})
/// LPLL mode
typedef enum
{
    G_LPLL_MODE_SINGLE = 0,
    G_LPLL_MODE_DUAL   = 1,
} G_LPLL_MODE_t;
// redefined

// Ckgen0

// MHal_SC_EnableClock
typedef enum
{
    G_SC_ENCLK_TTL            = 0x00,
    G_SC_ENCLK_SIGNAL_LVDS    = 0x11,
    G_SC_ENCLK_DUAL_LVDS      = 0x13,
} G_HAL_SC_ENCLK_e;

typedef struct MST_PANEL_INFO_s
{
    // Basic
    U16 u16HStart; //ursa scaler
    U16 u16VStart; //ursa scaler
    U16 u16Width; //ursa scaler
    U16 u16Height; //ursa scaler
    U16 u16HTotal; //ursa scaler
    U16 u16VTotal; //ursa scaler

    U16 u16DE_VStart;

    U16 u16DefaultVFreq;

    // LPLL
    U16 u16LPLL_InputDiv;
    U16 u16LPLL_LoopDiv;
    U16 u16LPLL_OutputDiv;

    U8  u8LPLL_Type;
    U8  u8LPLL_Mode;

    // sync
    U8  u8HSyncWidth;
    U8 bPnlDblVSync;

    // output control
    U16 u16OCTRL;
    U16 u16OSTRL;
    U16 u16ODRV;
    U16 u16DITHCTRL;

    // MOD
    U16 u16MOD_CTRL0;
    U16 u16MOD_CTRL9;
    U16 u16MOD_CTRLA;
    U8  u8MOD_CTRLB;

	//titania to URSA
	U8  u8LVDSTxChannel; //ursa scaler
    U8  u8LVDSTxBitNum; //ursa scaler
    U8  u8LVDSTxTiMode;  //ursa scaler 40-bit2
    U8  u8LVDSTxSwapMsbLsb; //ursa scaler
    U8  u8LVDSTxSwap_P_N; //ursa scaler
    U8  u8LVDSTxSwapOddEven; //ursa scaler

	//URSA to Panel info
    U8  u8PanelVfreq; //ursa scaler
	U8  u8PanelChannel; //ursa scaler
	U8	u8PanelLVDSSwapCH; //ursa scaler
	U8  u8PanelBitNum; //ursa scaler
	U8	u8PanelLVDSShiftPair; //ursa scaler
	U8	u8PanelLVDSTiMode; //ursa scaler
	U8	u8PanelLVDSSwapPol; //ursa scaler
	U8	u8PanelLVDSSwapPair; //ursa scaler

	//LGE [vivakjh] 2008/11/12 	Add for DVB PDP Panel
	//Additional Info.(V Total)
    U16 u16VTotal60Hz; //ursa scaler
    U16 u16VTotal50Hz; //ursa scaler
    U16 u16VTotal48Hz; //ursa scaler
	//[vivakjh] 2008/12/23	Add for adjusting the MRE in PDP S6
	U16 u16VStart60Hz;
	U16 u16VStart50Hz;
	U16 u16VStart48Hz;
	U16 u16VBackPorch60Hz;
	U16 u16VBackPorch50Hz;
	U16 u16VBackPorch48Hz;

        // shjang_090904
	//Panel Option(LCD : 0, PDP : 1, LCD_NO_FRC : 2, LCD_TCON : 3)
	U8	u8LCDorPDP;

	U32 u32LimitD5d6d7; //thchen 20081216
	U16 u16LimitOffset; //thchen 20081216
	U8  u8LvdsSwingUp;
	U8 bTTL_10BIT;
	U8 bOD_DataPath;
} MST_PANEL_INFO_t, *PMST_PANEL_INFO_t;

typedef enum
{
    PANEL_RES_XGA,	 // 091125_KimTH_01: PDP boot logo ´ëÀÀ
    PANEL_RES_WXGA,
    PANEL_RES_FULL_HD,
    PANEL_RES_MAX_NUM,
} PANEL_RESOLUTION_TYPE_e;

typedef enum
{
    LPLL_LVDS,
    LPLL_RSDS,
    LPLL_TTL,
    LPLL_MINILVDS,
} SC_MOD_LPLL_TYPE_e;

typedef enum
{
    SC_MAIN_WINDOW = 0,
    SC_SUB_WINDOW  = 1,
} SC_WINDOW_IDX_e;

void MDrv_SC_Init(PANEL_RESOLUTION_TYPE_e e_panel_type, SC_MOD_LPLL_TYPE_e e_lpll_type);
void MDrv_SC_TCON_PWS_VSINT(void);
void MDrv_SC_TCON_Init_Flow(PANEL_RESOLUTION_TYPE_e e_panel_type, SC_MOD_LPLL_TYPE_e e_lpll_type, U16 u16tconpanelIdx);

#endif
