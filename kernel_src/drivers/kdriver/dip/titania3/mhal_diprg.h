#ifndef __DRVDIPRG_H
#define __DRVDIPRG_H


#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)

#define H(W)                        (((W)<<1)+1)
#define L(W)                        (((W)<<1)+0)

#define BYTE2REAL(B)                (((B)>>1<<2)+((B)&0x01))
#define WORD2REAL(W)                ((W)<<2)

#define XBYTE(addr)                 X1BYTE(addr)
#define X1BYTE(addr)                *(volatile U8*)(0xBF000000 + (addr))
#define X2BYTE(addr)                *(volatile U16*)(0xBF000000 + (addr))

/* Write/Read method invalid */
#define _MHal_R1B( u32Reg ) \
        X1BYTE(u32Reg)

#define _MHal_W1B( u32Reg, u08Val ) \
        (X1BYTE(u32Reg) = u08Val)

#define _MHal_W1BM( u32Reg, u08Val, u08Mask ) \
        (X1BYTE(u32Reg) = (X1BYTE(u32Reg) & ~(u08Mask)) | ((u08Val) & (u08Mask)))

#define _MHal_W1Rb( u32Reg, bBit, u08BitPos ) \
        (X1BYTE(u32Reg) = (bBit) ? (X1BYTE(u32Reg) | (u08BitPos)) : (X1BYTE(u32Reg) & ~(u08BitPos)))

#define _MHal_R1Rb( u32Reg, u08BitPos ) \
        (X1BYTE(u32Reg) & (u08BitPos))

#define _MHal_R2B( u32Reg ) \
        X2BYTE(u32Reg)

#define _MHal_W2B( u32Reg, u16Val ) \
        (X2BYTE(u32Reg) = (u16Val))

#define _MHal_W2BM( u32Reg, u08Val, u08Mask ) \
        (X2BYTE(u32Reg) = (X2BYTE(u32Reg) & ~(u08Mask)) | ((u08Val) & (u08Mask)))

#define _MHal_W2Bb( u32Reg, bBit, u08BitPos ) \
        (X2BYTE(u32Reg) = (bBit) ? (X2BYTE(u32Reg) | (u08BitPos)) : (X2BYTE(u32Reg) & ~(u08BitPos)))

#define _MHal_R2Bb( u32Reg, u08BitPos ) \
        (X2BYTE(u32Reg) & (u08BitPos))


#define TOP_REG_BASE            (0x101E00)
#define CLKGEN1_REG_BASE        (0x103300)
#define NR_REG_BASE             (0x110E00)
#define DI_REG_BASE             (0x110F00)
#define NR_HSD_REG_BASE         (0x110B00)

#define NR_HSD_REG(x)                   WORD2REAL((NR_HSD_REG_BASE>>1) + (x))
#define DI_REG(x)                       WORD2REAL((DI_REG_BASE>>1) + (x))

//------------------------------------------------------------------------------
//TOP DIP REGISTER
//------------------------------------------------------------------------------
//#define TOP_REG_DIP_CLK                 BYTE2REAL(TOP_REG_BASE + 0x2F)
#define TOP_REG_DIP_CLK                 BYTE2REAL(CLKGEN1_REG_BASE + L(0x1C))
#define TOP_PAD_CCIR_L                  BYTE2REAL(TOP_REG_BASE + 0x9E)
#define TOP_PAD_CCIR_H                  BYTE2REAL(TOP_REG_BASE + 0x9F)

    //TOP_REG_DIP_CLK                 (TOP_REG_BASE + 0x2F)
    // Not define in T3
    /*
#define REG_VD_CLK_DISABLE              0x01
#define REG_VD_CLK_INVERT               0x02
#define REG_VD_CLK_SOURCE_MSK           0x0C
#define REG_VD_CLK_80MHZ                0x0
#define REG_VD_CLK_54MHZ                0x4
#define REG_VD_CLK_XTAL                 0xC
    */

    // Not define in T3
    /*
#define REG_DIP_CLK_DISABLE             0x10
#define REG_DIP_CLK_INVERT              0x20
#define REG_DIP_CLK_SOURCE_MSK          0xC0
#define REG_DIP_CLK_VD                  0x00
#define REG_DIP_CLK_PAD_CCIR            0x40
#define REG_DIP_CLK_XTAL                0xC0
    */
#define REG_DIP_CLK_DISABLE             BIT0
#define REG_DIP_CLK_INVERT              BIT1
#define REG_DIP_CLK_SOURCE_MSK          (BIT2|BIT3)
#define REG_DIP_CLK_VD                  0x00
#define REG_DIP_CLK_PAD_CCIR            BIT3
#define REG_DIP_CLK_XTAL                (BIT2|BIT3)


    // Not define in T3
/*
//TOP_PAD_CCIR_L                  (TOP_REG_BASE + 0x9E)
#define REG_PAD_CCIR_IN_DELAY_MASK       0x0F
#define REG_PAD_CCIR_OUT_DELAY_MASK      0xF0
//TOP_PAD_CCIR_H                  (TOP_REG_BASE + 0x9F)
#define REG_PAD_CCIR_OUT_CLK_POLARITY    0x01
#define REG_PAD_CCIR_IN_DELAY_EN         0x02
#define REG_PAD_CCIR_OUT_DELAY_EN        0x04
*/

//------------------------------------------------------------------------------
// NR REGISTER
//------------------------------------------------------------------------------
#define NR_REG(x)                       WORD2REAL((NR_REG_BASE>>1) + (x))

#define NR_REG_CTRL_L                   BYTE2REAL(NR_REG_BASE + L(0x00))
#define NR_REG_CTRL_H                   BYTE2REAL(NR_REG_BASE + H(0x00))
#define NR_REG_IN_CTRL_L                BYTE2REAL(NR_REG_BASE + L(0x01))
#define NR_REG_IN_CTRL_H                BYTE2REAL(NR_REG_BASE + H(0x01))
#define NR_REG_MIU_CTRL_L               BYTE2REAL(NR_REG_BASE + L(0x02))
#define NR_REG_VD_SET_L                 BYTE2REAL(NR_REG_BASE + L(0x03))
#define NR_REG_VD_SET_H                 BYTE2REAL(NR_REG_BASE + H(0x03))
#define NR_REG_CLK_L                    BYTE2REAL(NR_REG_BASE + L(0x04))
#define NR_REG_CLK_H                    BYTE2REAL(NR_REG_BASE + H(0x04))
#define NR_REG_OUT_POLARITY             BYTE2REAL(NR_REG_BASE + L(0x05))
#define NR_REG_MLK_SET                  BYTE2REAL(NR_REG_BASE + H(0x05))

#define NR_REG_IRQ_MASK_7_0             BYTE2REAL(NR_REG_BASE + L(0x08))
#define NR_REG_IRQ_MASK_15_8            BYTE2REAL(NR_REG_BASE + H(0x08))
#define NR_REG_IRQ_FORCE_7_0            BYTE2REAL(NR_REG_BASE + L(0x09))
#define NR_REG_IRQ_FORCE_15_8           BYTE2REAL(NR_REG_BASE + H(0x09))
#define NR_REG_IRQ_CLR_7_0              BYTE2REAL(NR_REG_BASE + L(0x0A))
#define NR_REG_IRQ_CLR_15_8             BYTE2REAL(NR_REG_BASE + H(0x0A))

#define NR_REG_IRQ_MASK_19_16           BYTE2REAL(NR_REG_BASE + L(0x0B))
#define NR_REG_IRQ_FORCE_19_16          BYTE2REAL(NR_REG_BASE + H(0x0B))
#define NR_REG_IRQ_CLR_19_16            BYTE2REAL(NR_REG_BASE + L(0x0C))

#define NR_REG_IRQ_STATUS_7_0           BYTE2REAL(NR_REG_BASE + L(0x0D))
#define NR_REG_IRQ_STATUS_15_8          BYTE2REAL(NR_REG_BASE + H(0x0D))
#define NR_REG_IRQ_STATUS_19_16         BYTE2REAL(NR_REG_BASE + L(0x0E))

#define NR_REG_BIST_L                   BYTE2REAL(NR_REG_BASE + L(0x0F))
#define NR_REG_BIST_H                   BYTE2REAL(NR_REG_BASE + H(0x0F))
//#define NR_REG_DEBUG_OUT_SEL            BYTE2REAL(NR_REG_BASE + 0x1E)

#define NR_REG_ACTIVE_START_X           BYTE2REAL(NR_REG_BASE + L(0x10))
#define NR_REG_ACTIVE_START_Y           BYTE2REAL(NR_REG_BASE + L(0x11))
#define NR_REG_ACTIVE_WIDTH             BYTE2REAL(NR_REG_BASE + L(0x12))
#define NR_REG_ACTIVE_HEIGHT_TOP        BYTE2REAL(NR_REG_BASE + L(0x13))
#define NR_REG_ACTIVE_HEIGHT_BOT        BYTE2REAL(NR_REG_BASE + L(0x14))

#define NR_REG_PTN_ACTIVE_WIDTH         BYTE2REAL(NR_REG_BASE + L(0x15))
#define NR_REG_PTN_ACTIVE_HEIGHT_TOP    BYTE2REAL(NR_REG_BASE + L(0x16))
#define NR_REG_PTN_ACTIVE_HEIGHT_BOT    BYTE2REAL(NR_REG_BASE + L(0x17))
#define NR_REG_PTN_BLK_WIDTH            BYTE2REAL(NR_REG_BASE + L(0x18))
#define NR_REG_PTN_BLK_HEIGHT           BYTE2REAL(NR_REG_BASE + L(0x19))
#define NR_REG_DATA_START_0             BYTE2REAL(NR_REG_BASE + L(0x20))
#define NR_REG_DATA_START_1             BYTE2REAL(NR_REG_BASE + H(0x20))
#define NR_REG_DATA_START_2             BYTE2REAL(NR_REG_BASE + L(0x21))

#define NR_REG_DATA_END_0               BYTE2REAL(NR_REG_BASE + L(0x22))
#define NR_REG_DATA_END_1               BYTE2REAL(NR_REG_BASE + H(0x22))
#define NR_REG_DATA_END_2               BYTE2REAL(NR_REG_BASE + L(0x23))

#define NR_REG_RATIO_START_0            BYTE2REAL(NR_REG_BASE + L(0x24))
#define NR_REG_RATIO_START_1            BYTE2REAL(NR_REG_BASE + H(0x24))
#define NR_REG_RATIO_START_2            BYTE2REAL(NR_REG_BASE + L(0x25))

#define NR_REG_RATIO_END_0              BYTE2REAL(NR_REG_BASE + L(0x26))
#define NR_REG_RATIO_END_1              BYTE2REAL(NR_REG_BASE + H(0x26))
#define NR_REG_RATIO_END_2              BYTE2REAL(NR_REG_BASE + L(0x27))

#define NR_REG_DATA_PITCH_L             BYTE2REAL(NR_REG_BASE + L(0x28))
#define NR_REG_DATA_PITCH_H             BYTE2REAL(NR_REG_BASE + H(0x28))
#define NR_REG_RATIO_PITCH_L            BYTE2REAL(NR_REG_BASE + L(0x29))
#define NR_REG_RATIO_PITCH_H            BYTE2REAL(NR_REG_BASE + H(0x29))

#define NR_REG_MIU_DATA_RTH             BYTE2REAL(NR_REG_BASE + L(0x2A))
#define NR_REG_MIU_DATA_RLEN            BYTE2REAL(NR_REG_BASE + H(0x2A))

#define NR_REG_MIU_DATA_WTH             BYTE2REAL(NR_REG_BASE + L(0x2B))
#define NR_REG_MIU_DATA_WLEN            BYTE2REAL(NR_REG_BASE + H(0x2B))
#define NR_REG_MIU_RATIO_RTH            BYTE2REAL(NR_REG_BASE + L(0x2C))
#define NR_REG_MIU_RATIO_TLEN           BYTE2REAL(NR_REG_BASE + H(0x2C))
#define NR_REG_MIU_RATIO_WTH            BYTE2REAL(NR_REG_BASE + L(0x2D))
#define NR_REG_MIU_RATIO_WLEN           BYTE2REAL(NR_REG_BASE + H(0x2D))
#define NR_REG_MIU_HM_RTH               BYTE2REAL(NR_REG_BASE + L(0x2E))
#define NR_REG_MIU_HM_RLEN              BYTE2REAL(NR_REG_BASE + H(0x2E))
#define NR_REG_MIU_IDLE_CNT             BYTE2REAL(NR_REG_BASE + L(0x2F))

#define NR_REG_DATA_SIZE_0              BYTE2REAL(NR_REG_BASE + L(0x30))
#define NR_REG_DATA_SIZE_1              BYTE2REAL(NR_REG_BASE + H(0x30))
#define NR_REG_DATA_SIZE_2              BYTE2REAL(NR_REG_BASE + L(0x31))
#define NR_REG_DATA_CNT                 BYTE2REAL(NR_REG_BASE + L(0x32))

#define NR_REG_TNR_TAB1_7_0             BYTE2REAL(NR_REG_BASE + L(0x34))
#define NR_REG_TNR_TAB1_15_8            BYTE2REAL(NR_REG_BASE + H(0x34))
#define NR_REG_TNR_TAB1_23_16           BYTE2REAL(NR_REG_BASE + L(0x35))
#define NR_REG_TNR_TAB1_31_24           BYTE2REAL(NR_REG_BASE + H(0x35))
#define NR_REG_TNR_TAB1_39_32           BYTE2REAL(NR_REG_BASE + L(0x36))
#define NR_REG_TNR_TAB1_47_40           BYTE2REAL(NR_REG_BASE + H(0x36))
#define NR_REG_TNR_TAB1_55_48           BYTE2REAL(NR_REG_BASE + L(0x37))
#define NR_REG_TNR_TAB1_63_56           BYTE2REAL(NR_REG_BASE + H(0x37))

#define NR_REG_TNR_TAB3_7_0             BYTE2REAL(NR_REG_BASE + L(0x40))
#define NR_REG_TNR_TAB3_15_8            BYTE2REAL(NR_REG_BASE + H(0x40))
#define NR_REG_TNR_TAB3_23_16           BYTE2REAL(NR_REG_BASE + L(0x41))
#define NR_REG_TNR_TAB3_31_24           BYTE2REAL(NR_REG_BASE + H(0x41))
#define NR_REG_TNR_TAB3_39_32           BYTE2REAL(NR_REG_BASE + L(0x42))
#define NR_REG_TNR_TAB3_47_40           BYTE2REAL(NR_REG_BASE + H(0x42))
#define NR_REG_TNR_TAB3_55_48           BYTE2REAL(NR_REG_BASE + L(0x43))
#define NR_REG_TNR_TAB3_63_56           BYTE2REAL(NR_REG_BASE + H(0x43))

#define NR_REG_TNR_INV_TAB_7_0          BYTE2REAL(NR_REG_BASE + L(0x44))
#define NR_REG_TNR_INV_TAB_15_8         BYTE2REAL(NR_REG_BASE + H(0x44)
#define NR_REG_TNR_INV_TAB_23_16        BYTE2REAL(NR_REG_BASE + L(0x45))
#define NR_REG_TNR_INV_TAB_31_24        BYTE2REAL(NR_REG_BASE + H(0x45)
#define NR_REG_TNR_INV_TAB_39_32        BYTE2REAL(NR_REG_BASE + L(0x46))
#define NR_REG_TNR_INV_TAB_47_40        BYTE2REAL(NR_REG_BASE + H(0x46))
#define NR_REG_TNR_INV_TAB_55_48        BYTE2REAL(NR_REG_BASE + L(0x47))
#define NR_REG_TNR_INV_TAB_63_56        BYTE2REAL(NR_REG_BASE + H(0x47))
#define NR_REG_TNR_INV_TAB_71_64        BYTE2REAL(NR_REG_BASE + L(0x48))
#define NR_REG_TNR_INV_TAB_79_72        BYTE2REAL(NR_REG_BASE + H(0x48))
#define NR_REG_TNR_INV_TAB_87_80        BYTE2REAL(NR_REG_BASE + L(0x49))
#define NR_REG_TNR_INV_TAB_95_88        BYTE2REAL(NR_REG_BASE + H(0x49))

#define NR_REG_TNR_MRF                  BYTE2REAL(NR_REG_BASE + L(0x4A))
#define NR_REG_TNR_HM_GAIN              BYTE2REAL(NR_REG_BASE + H(0x4A))

#define NR_REG_TNR_MODE                 BYTE2REAL(NR_REG_BASE + L(0x4C))
#define NR_REG_TNR_MRF_CDIV             BYTE2REAL(NR_REG_BASE + L(0x4D))
#define NR_REG_TNR_SNR_MODE             BYTE2REAL(NR_REG_BASE + L(0x4E))
#define NR_REG_TNR_SNR_MRF_SHIFT        BYTE2REAL(NR_REG_BASE + H(0x4E))
#define NR_REG_TNR_SNR_MRF_DIV          BYTE2REAL(NR_REG_BASE + L(0x4F))
#define NR_REG_SNR_FROST_K              BYTE2REAL(NR_REG_BASE + L(0x50))
#define NR_REG_SNR_MODE                 BYTE2REAL(NR_REG_BASE + H(0x50))

#define NR_REG_SNR_FROST_K_STEP         BYTE2REAL(NR_REG_BASE + L(0x51))
#define NR_REG_SNR_FROST_DEADZONE_UR    BYTE2REAL(NR_REG_BASE + L(0x52))
#define NR_REG_SNR_FROST_DEADZONE_LR    BYTE2REAL(NR_REG_BASE + L(0x53))

#define NR_REG_TNR_SNR_TAB3_7_0         BYTE2REAL(NR_REG_BASE + L(0x54))
#define NR_REG_TNR_SNR_TAB3_15_8        BYTE2REAL(NR_REG_BASE + H(0x54))
#define NR_REG_TNR_SNR_TAB3_23_16       BYTE2REAL(NR_REG_BASE + L(0x55))
#define NR_REG_TNR_SNR_TAB3_31_24       BYTE2REAL(NR_REG_BASE + H(0x55))
#define NR_REG_TNR_SNR_TAB3_39_32       BYTE2REAL(NR_REG_BASE + L(0x56))
#define NR_REG_TNR_SNR_TAB3_47_40       BYTE2REAL(NR_REG_BASE + H(0x56))
#define NR_REG_TNR_SNR_TAB3_55_48       BYTE2REAL(NR_REG_BASE + L(0x57))
#define NR_REG_TNR_SNR_TAB3_63_56       BYTE2REAL(NR_REG_BASE + H(0x57))

#define NR_REG_PTN_CTRL                 BYTE2REAL(NR_REG_BASE + L(0x58))
#define NR_REG_DEBUG_COLOR_SET          BYTE2REAL(NR_REG_BASE + L(0x59))
#define NR_REG_SNR_BLEND_FIX_RATIO      BYTE2REAL(NR_REG_BASE + L(0x5A))

#define NR_REG_MAX_DET_SYNC_CNT         BYTE2REAL(NR_REG_BASE + L(0x60))
#define NR_REG_MAX_DET_LINE             BYTE2REAL(NR_REG_BASE + L(0x61))
#define NR_REG_MIN_DET_LINE             BYTE2REAL(NR_REG_BASE + L(0x62))

#define NR_REG_MAX_DET_CYCLE            BYTE2REAL(NR_REG_BASE + L(0x63))
#define NR_REG_MIN_DET_CYCLE            BYTE2REAL(NR_REG_BASE + L(0x64))
#define NR_REG_SNR_FROST_K_HW_ADJ       BYTE2REAL(NR_REG_BASE + L(0x65))
#define NR_REG_SNR_FRAME_ORG_Y_CNT_L    BYTE2REAL(NR_REG_BASE + L(0x66))
#define NR_REG_SNR_FRAME_ORG_Y_CNT_H    BYTE2REAL(NR_REG_BASE + L(0x67))
#define NR_REG_SNR_FRAME_FROST_Y_CNT_L  BYTE2REAL(NR_REG_BASE + L(0x68))
#define NR_REG_SNR_FRAME_FROST_Y_CNT_H  BYTE2REAL(NR_REG_BASE + L(0x69))

#define NR_REG_SNR_FRAME_MEAN_Y_CNT_L   BYTE2REAL(NR_REG_BASE + L(0x6A))
#define NR_REG_SNR_FRAME_MEAN_Y_CNT_H   BYTE2REAL(NR_REG_BASE + L(0x6B))

#define NR_REG_MIU_DET_WIDTH            BYTE2REAL(NR_REG_BASE + L(0x70))
#define NR_REG_MIU_DET_HIGH             BYTE2REAL(NR_REG_BASE + L(0x71))
#define NR_REG_DECODE_YCNT              BYTE2REAL(NR_REG_BASE + L(0x72))
#define NR_REG_Y_CONST_CTRL             BYTE2REAL(NR_REG_BASE + L(0x7D))
#define NR_REG_C_CONST_CTRL             BYTE2REAL(NR_REG_BASE + L(0x7E))
#define NR_REG_Y_CONST_VALUE            BYTE2REAL(NR_REG_BASE + L(0x7F))
#define NR_REG_C_CONST_VALUE            BYTE2REAL(NR_REG_BASE + H(0x7F))


//NR_REG_CTRL_L                   (NR_REG_BASE + 0x00)
#define REG_NR_EN                                   0x01
#define REG_NR_SNR_EN                               0x02
#define REG_NR_TNR_RD_DATA_EN                       0x04
#define REG_NR_TNR_RD_RATIO_EN                      0x08
#define REG_NR_MIU_WR_DATA_EN                       0x10
#define REG_NR_MIU_WR_RATIO_EN                       0x20 //    #define REG_NR_MIU_RD_DATA_EN 0x20
#define REG_NR_SNR_LINE_BUFF_EN                     0x40
//NR_REG_CTRL_H                   (NR_REG_BASE + 0x01)
#define REG_NR_DATA_ACCESS_MIU_EN                   0x01
#define REG_NR_RATIO_ACCESS_MIU_EN                  0x02
#define REG_NR_MOTION_HISTORY_EN                    0x04
#define REG_NR_TNR_ADJUST_RATIO_KY_EN               0x20
#define REG_NR_TNR_ADJUST_RATIO_KC_EN               0x40
#define REG_NR_TNR_EN                               0x80
//NR_REG_IN_CTRL_L                (NR_REG_BASE + 0x02)
#define REG_NR_SW_RSTZ                              0x01
#define REG_NR_PROGRESSIVE_IN                       0x02
#define REG_NR_VSYNC_REF_PAD                        0x04
#define REG_NR_HSYNC_REF_PAD                        0x08
#define REG_NR_SOURCE_CCIR656                       0x00
#define REG_NR_SOURCE_PAD_601_FIELD                 0x10
#define REG_NR_SOURCE_PAD_601_SYNC                  0x20
#define REG_NR_SOURCE_INTENAL_AUTO                  0x30
#define REG_NR_VSYNC_HIGH_ACTIVE                    0x40
#define REG_NR_HSYNC_HIGH_ACTIVE                    0x80
//NR_REG_IN_CTRL_H                (NR_REG_BASE + 0x03)
#define REG_NR_TOP_FIELD_HIGH_ACTIVE                0x01
#define REG_NR_VSTART_FRAME_END                     0x10
#define REG_NR_HSTART_LINE_END                      0x20
#define REG_NR_PROGRESSIVE_OUT                      0x40
//NR_REG_VD_SET_L                 (NR_REG_BASE + 0x06)
#define REG_NR_VD_CCIR_8BIT                         0x00
#define REG_NR_VD_CCIR_16BIT                        0x01
#define REG_NR_VD_CCIR_DDR_4BIT                     0x02
#define REG_NR_VD_CCIR_DDR_8BIT                     0x03
#define REG_NR_YUV_ORDER_YCYC                       0x10
#define REG_NR_VD_DDR_ORDER_POSITVE_MSB             0x20
//NR_REG_VD_SET_H                 (NR_REG_BASE + 0x07)
#define REG_NR_BLEND_ROUND_Y                        0x01
#define REG_NR_BLEND_ROUND_C                        0x02
#define REG_NR_LOAD_REG                             0x80
//NR_REG_CLK_L                    (NR_REG_BASE + 0x08)
#define REG_NR_VD_CLK_180_PHASE                     0x01
#define REG_NR_PTN_GEN_EN                           0x02
#define REG_NR_VD_CLK_DELAY_EN                      0x04
#define REG_NR_EN_SNR_FROST_K_AUTO                  0x10
//NR_REG_CLK_H                    (NR_REG_BASE + 0x09)
#define REG_NR_VD_CLK_DELAY_1UNIT                   0x00
#define REG_NR_VD_CLK_DELAY_2UNIT                   0x01
#define REG_NR_VD_CLK_DELAY_3UNIT                   0x02
#define REG_NR_VD_CLK_DELAY_4UNIT                   0x04
//NR_REG_OUT_POLARITY             (NR_REG_BASE + 0x0A)
#define REG_NR_OUT_VSYNC_HIGH_AVTIVE                0x01
#define REG_NR_OUT_HSYNC_HIGH_ACTIVE                0x02
#define REG_NR_OUT_TOP_FILED_HIGH_ACTIVE            0x04
//NR_REG_MLK_SET                  (NR_REG_BASE + 0x0B)
#define REG_NR_MLK_SW_RESET                         0x40
#define REG_NR_SOUCE_PAD                            0x00
#define REG_NR_SOURCE_MLK                           0x80
/*
#define REG_NR_                        0x
#define REG_NR_                        0x
#define REG_NR_                        0x
#define REG_NR_                        0x
#define REG_NR_                        0x
*/





//NR_REG_IRQ_STATUS_7_0           (NR_REG_BASE + 0x18)
#define REG_IRQ_MASK_ALL                            0xFF
#define REG_IRQ_NR_VSYNC_RISING                     0x01
#define REG_IRQ_NR_VSYNC_FALLING                    0x02
#define REG_IRQ_NR_HSYNC_RISING                     0x04
#define REG_IRQ_NR_HSYNC_FALLING                    0x08
#define REG_IRQ_NR_FIELD_OUT_RISING                 0x10
#define REG_IRQ_NR_FIELD_OUT_FALING                 0x20
#define REG_IRQ_NO_SYNC_IN                          0x40
#define REG_IRQ_ABNORMAL_V_LINE_IN                  0x80
//NR_REG_IRQ_STATUS_15_8          (NR_REG_BASE + 0x19)
#define REG_IRQ_ABNORMAL_H_PIXEL_IN                 0x01
#define REG_IRQ_NR_WR_RATIO_FIFO_FULL               0x02
#define REG_IRQ_NR_WR_DATA_FIFO_FULL                0x04
#define REG_IRQ_NR_RD_HISTORY_RATIO_FIFO_EMPTY      0x08
#define REG_IRQ_NR_RD_RATIO_FIFO_EMPTY              0x10
#define REG_IRQ_NR_RD_DATA_FIFO_EMPTY               0x20
#define REG_IRQ_DI_RD_DATA_FIFO_EMPTY               0x04
#define REG_IRQ_DI_WR_DATA_FIFO_FULL                0x08

//NR_REG_IRQ_STATUS_19_16         (NR_REG_BASE + 0x1A)
#define REG_IRQ_DI_WR_Y_FIFO_FULL                   0x01
#define REG_IRQ_NR_WR_MIU_DONE                      0x02
#define REG_IRQ_DI_WR_MIU_DONE                      0x04
#define REG_NR_FROST_DONE                           0x08





//------------------------------------------------------------------------------
// DI REGISTER
//------------------------------------------------------------------------------
#define DI_REG_AUTO_SIZE                BYTE2REAL(DI_REG_BASE + L(0x00))
#define DI_REG_EXT_FB                   BYTE2REAL(DI_REG_BASE + H(0x00))
#define DI_REG_MIU_CTRL                 BYTE2REAL(DI_REG_BASE + L(0x01))
#define DI_REG_FRAME_BASE_SEL           BYTE2REAL(DI_REG_BASE + H(0x01))
#define DI_REG_FRAME_Y_START_0          BYTE2REAL(DI_REG_BASE + L(0x04))
#define DI_REG_FRAME_Y_START_1          BYTE2REAL(DI_REG_BASE + H(0x04))
#define DI_REG_FRAME_Y_START_2          BYTE2REAL(DI_REG_BASE + L(0x05))
#define DI_REG_FRAME_C_START_0          BYTE2REAL(DI_REG_BASE + L(0x06))
#define DI_REG_FRAME_C_START_1          BYTE2REAL(DI_REG_BASE + H(0x06))
#define DI_REG_FRAME_C_START_2          BYTE2REAL(DI_REG_BASE + L(0x07))
#define DI_REG_FRAME_Y_END_0            BYTE2REAL(DI_REG_BASE + L(0x08))
#define DI_REG_FRAME_Y_END_1            BYTE2REAL(DI_REG_BASE + H(0x08))
#define DI_REG_FRAME_Y_END_2            BYTE2REAL(DI_REG_BASE + L(0x09))
#define DI_REG_FRAME_C_END_0            BYTE2REAL(DI_REG_BASE + L(0x0A))
#define DI_REG_FRAME_C_END_1            BYTE2REAL(DI_REG_BASE + H(0x0A))
#define DI_REG_FRAME_C_END_2            BYTE2REAL(DI_REG_BASE + L(0x0B))
#define DI_REG_FRAME_Y_PITCH            BYTE2REAL(DI_REG_BASE + L(0x0D))
#define DI_REG_FRAME_C_PITCH            BYTE2REAL(DI_REG_BASE + L(0x0E))
#define DI_REG_MIU_WTH                  BYTE2REAL(DI_REG_BASE + L(0x10))
#define DI_REG_MIU_WLEN                 BYTE2REAL(DI_REG_BASE + H(0x10))
#define DI_REG_MIU_IDLE_COUNT           BYTE2REAL(DI_REG_BASE + L(0x11))
#define DI_REG_H_DIMENSION              BYTE2REAL(DI_REG_BASE + L(0x12))
#define DI_REG_V_DIMENSION              BYTE2REAL(DI_REG_BASE + L(0x13))
#define DI_REG_DET_HDC                  BYTE2REAL(DI_REG_BASE + L(0x14))
#define DI_REG_DET_LOCK_H               BYTE2REAL(DI_REG_BASE + H(0x14))
#define DI_REG_DET_VDC                  BYTE2REAL(DI_REG_BASE + L(0x15))
#define DI_REG_DET_LOCK_V               BYTE2REAL(DI_REG_BASE + H(0x15))
#define DI_REG_COLOR_RATIO              BYTE2REAL(DI_REG_BASE + H(0x16))
#define DI_REG_CORE_TH                  BYTE2REAL(DI_REG_BASE + L(0x17))
#define DI_REG_CORE_0X80                BYTE2REAL(DI_REG_BASE + H(0x17))
#define DI_REG_EXT_Y_OFFSET_0           BYTE2REAL(DI_REG_BASE + L(0x18))
#define DI_REG_EXT_Y_OFFSET_1           BYTE2REAL(DI_REG_BASE + H(0x18))
#define DI_REG_EXT_Y_OFFSET_2           BYTE2REAL(DI_REG_BASE + L(0x19))
#define DI_REG_EXT_C_OFFSET_0           BYTE2REAL(DI_REG_BASE + L(0x1A))
#define DI_REG_EXT_C_OFFSET_1           BYTE2REAL(DI_REG_BASE + H(0x1A))
#define DI_REG_EXT_C_OFFSET_2           BYTE2REAL(DI_REG_BASE + L(0x1B))
#define DI_REG_HIST_OUT_00              BYTE2REAL(DI_REG_BASE + L(0x20))
#define DI_REG_HIST_OUT_01              BYTE2REAL(DI_REG_BASE + H(0x20))
#define DI_REG_HIST_OUT_02              BYTE2REAL(DI_REG_BASE + L(0x21))
#define DI_REG_HIST_OUT_03              BYTE2REAL(DI_REG_BASE + H(0x21))
#define DI_REG_HIST_OUT_04              BYTE2REAL(DI_REG_BASE + L(0x22))
#define DI_REG_HIST_OUT_05              BYTE2REAL(DI_REG_BASE + H(0x22))
#define DI_REG_HIST_OUT_06              BYTE2REAL(DI_REG_BASE + L(0x23))
#define DI_REG_HIST_OUT_07              BYTE2REAL(DI_REG_BASE + H(0x23))
#define DI_REG_HIST_OUT_08              BYTE2REAL(DI_REG_BASE + L(0x24))
#define DI_REG_HIST_OUT_09              BYTE2REAL(DI_REG_BASE + H(0x24))
#define DI_REG_HIST_OUT_10              BYTE2REAL(DI_REG_BASE + L(0x25))
#define DI_REG_HIST_OUT_11              BYTE2REAL(DI_REG_BASE + H(0x25))
#define DI_REG_HIST_OUT_12              BYTE2REAL(DI_REG_BASE + L(0x26))
#define DI_REG_HIST_OUT_13              BYTE2REAL(DI_REG_BASE + H(0x26))
#define DI_REG_HIST_OUT_14              BYTE2REAL(DI_REG_BASE + L(0x27))
#define DI_REG_HIST_OUT_15              BYTE2REAL(DI_REG_BASE + H(0x27))
#define DI_REG_HIST_DIFF_00             BYTE2REAL(DI_REG_BASE + L(0x28))
#define DI_REG_HIST_DIFF_01             BYTE2REAL(DI_REG_BASE + H(0x28))
#define DI_REG_HIST_DIFF_02             BYTE2REAL(DI_REG_BASE + L(0x29))
#define DI_REG_HIST_DIFF_03             BYTE2REAL(DI_REG_BASE + H(0x29))
#define DI_REG_HIST_DIFF_04             BYTE2REAL(DI_REG_BASE + L(0x2A))
#define DI_REG_HIST_DIFF_05             BYTE2REAL(DI_REG_BASE + H(0x2A))
#define DI_REG_HIST_DIFF_06             BYTE2REAL(DI_REG_BASE + L(0x2B))
#define DI_REG_HIST_DIFF_07             BYTE2REAL(DI_REG_BASE + H(0x2B))
#define DI_REG_HIST_DIFF_08             BYTE2REAL(DI_REG_BASE + L(0x2C))
#define DI_REG_HIST_DIFF_09             BYTE2REAL(DI_REG_BASE + H(0x2C))
#define DI_REG_HIST_DIFF_10             BYTE2REAL(DI_REG_BASE + L(0x2D))
#define DI_REG_HIST_DIFF_11             BYTE2REAL(DI_REG_BASE + H(0x2D))
#define DI_REG_HIST_DIFF_12             BYTE2REAL(DI_REG_BASE + L(0x2E))
#define DI_REG_HIST_DIFF_13             BYTE2REAL(DI_REG_BASE + H(0x2E))
#define DI_REG_HIST_DIFF_14             BYTE2REAL(DI_REG_BASE + L(0x2F))
#define DI_REG_HIST_DIFF_15             BYTE2REAL(DI_REG_BASE + H(0x2F))

//DI_REG_EXT_FB                   (DI_REG_BASE + 0x01)
#define REG_DI_EXT_FB_CNT_MASK                      0x0F

//DI_REG_MIU_CTRL                 (DI_REG_BASE + 0x02)
#define REG_DI_MIU_EN                               0x01
#define REG_DI_MIU_WR_EN                            0x04
#define REG_DI_FMT_BLOCK                            0x08
#define REG_DI_FMT_YC_FIELD_MODE                    0x10
#define REG_DI_SWAP_CB_CR                           0x20
#define REG_DI_BLOCK_TOP                            0x40
#define REG_DI_SWAP_YC                              0x80
//DI_REG_FRAME_BASE_SEL           (DI_REG_BASE + 0x03)
#define REG_DI_FRAME_BASE_DI                        0x02
#define REG_DI_MIU_WR_WHOLE_FRAME                   0x00
#define REG_DI_MIU_WR_DIFF_FRAME                    0x20
#define REG_DI_STATUS                               0x80

#define REG_DI_INVERSE_FIELD 0x10
#define REG_DI_INVERSE_VSYNC 0x20
#define REG_DI_INVERSE_HSYNC 0x40
#define REG_DI_COUNT_FROM_1  0x80

/*
#define REG_DI_                     0x
#define REG_DI_                     0x
#define REG_DI_                     0x
#define REG_DI_                     0x
*/


#endif //__DRVDIPRG_H
