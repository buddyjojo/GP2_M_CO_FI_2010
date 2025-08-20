
#ifndef _MHAL_BDMA_REG_H_
#define _MHAL_BDMA_REG_H_

#include "chip_setup.h"

//----------------------------------------------------------------------
#define _BIT0                       0x00000001
#define _BIT1                       0x00000002
#define _BIT2                       0x00000004
#define _BIT3                       0x00000008
#define _BIT4                       0x00000010
#define _BIT5                       0x00000020
#define _BIT6                       0x00000040
#define _BIT7                       0x00000080
#define _BIT8                       0x00000100
#define _BIT9                       0x00000200
#define _BIT10                      0x00000400
#define _BIT11                      0x00000800
#define _BIT12                      0x00001000
#define _BIT13                      0x00002000
#define _BIT14                      0x00004000
#define _BIT15                      0x00008000

#define _BIT31                      0x80000000

/*
RIU Byte address to Titania IO address
RIU address in titania is 4 byte alignment and high word address is reserved.
*/
#define BYTE2REAL(B)                (((B)>>1<<2)+((B)&0x01))
#define WORD2REAL(W)                ((W)<<2)

/* BITMASK */
#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)
#define BITFLAG(loc) (1U << (loc))
#define IS_BITS_SET(val, bits)      (((val)&(bits)) == (bits))

#define MAKEWORD(value1, value2)    (((U16)(value1)) * 0x100) + (value2)


#define XBYTE(addr)             X1BYTE(addr)
#define X1BYTE(addr)            *(volatile U8*)(REG_MIPS_BASE + (addr))
#define X2BYTE(addr)            *(volatile U16*)(REG_MIPS_BASE + (addr))

/* Write/Read method invalid */
#define _MHal_R1B( u32Reg )        X1BYTE(u32Reg)
#define _MHal_R2B( u32Reg )        X2BYTE(u32Reg)

#define _MHal_W1B( u32Reg, u08Val )   (X1BYTE(u32Reg) = u08Val)
#define _MHal_W2B( u32Reg, u16Val )   (X2BYTE(u32Reg) = u16Val)

#define _MHal_W1BM( u32Reg, u08Val, u08Mask ) \
    (X1BYTE(u32Reg) = (X1BYTE(u32Reg) & ~(u08Mask)) | ((u08Val) & (u08Mask)))

#define _MHal_W1Bb( u32Reg, bBit, u08BitPos ) \
    (X1BYTE(u32Reg) = (bBit) ? (X1BYTE(u32Reg) | (u08BitPos)) : (X1BYTE(u32Reg) & ~(u08BitPos)))

#define _MHal_R1Bb( u32Reg, u08BitPos ) \
    (X1BYTE(u32Reg) & (u08BitPos))

#define _MHal_W2BM( u32Reg, u16Val, u16Mask ) \
    (X2BYTE(u32Reg) = (X2BYTE(u32Reg) & ~(u16Mask)) | ((u16Val) & (u16Mask)))

#define _MHal_W2Bb( u32Reg, bBit, u16BitPos ) \
    (X2BYTE(u32Reg) = (bBit) ? (X2BYTE(u32Reg) | (u16BitPos)) : (X2BYTE(u32Reg) & ~(u16BitPos)))

#define _MHal_R2Bb( u32Reg, u16BitPos ) \
    (X2BYTE(u32Reg) & (u16BitPos))
//----------------------------------------------------------------------


/* BDMA */
#define BDMA_REG_BASE	                        0x0900
#define BDMA_REG_CH_OFFSET                     (0x20 << 1)

#define BDMA_REG(x)                             BYTE2REAL(BDMA_REG_BASE + (x) )
#define BDMA_REG_CTRL                           BYTE2REAL(BDMA_REG_BASE + 0x00)
#define BDMA_REG_STATUS                         BYTE2REAL(BDMA_REG_BASE + 0x02)
#define BDMA_REG_SRC_SEL                        BYTE2REAL(BDMA_REG_BASE + 0x04)
#define BDMA_REG_DST_SEL                        BYTE2REAL(BDMA_REG_BASE + 0x05)
#define BDMA_REG_MISC                           BYTE2REAL(BDMA_REG_BASE + 0x06)
#define BDMA_REG_DWUM_CNT                       BYTE2REAL(BDMA_REG_BASE + 0x07)
#define BDMA_REG_SRC_ADDR_L                     BYTE2REAL(BDMA_REG_BASE + 0x08)
#define BDMA_REG_SRC_ADDR_H                     BYTE2REAL(BDMA_REG_BASE + 0x0A)
#define BDMA_REG_DST_ADDR_L                     BYTE2REAL(BDMA_REG_BASE + 0x0C)
#define BDMA_REG_DST_ADDR_H                     BYTE2REAL(BDMA_REG_BASE + 0x0E)
#define BDMA_REG_SIZE_L                         BYTE2REAL(BDMA_REG_BASE + 0x10)
#define BDMA_REG_SIZE_H                         BYTE2REAL(BDMA_REG_BASE + 0x12)
#define BDMA_REG_CMD0_L                         BYTE2REAL(BDMA_REG_BASE + 0x14)
#define BDMA_REG_CMD0_H                         BYTE2REAL(BDMA_REG_BASE + 0x16)
#define BDMA_REG_CMD1_L                         BYTE2REAL(BDMA_REG_BASE + 0x18)
#define BDMA_REG_CMD1_H                         BYTE2REAL(BDMA_REG_BASE + 0x1A)

// definition for BDMA_REG_CH0_CTRL/BDMA_REG_CH1_CTRL
#define BDMA_CH_TRIGGER             _BIT0
#define BDMA_CH_STOP                _BIT4

// definition for REG_BDMA_CH0_STATUS/REG_BDMA_CH1_STATUS
#define BDMA_CH_QUEUED              _BIT0
#define BDMA_CH_BUSY                _BIT1
#define BDMA_CH_INT                 _BIT2
#define BDMA_CH_DONE                _BIT3
#define BDMA_CH_RESULT              _BIT4
#define BDMA_CH_CLEAR_STATUS        (BDMA_CH_INT|BDMA_CH_DONE|BDMA_CH_RESULT)

// definition for REG_BDMA_CH0_MISC/REG_BDMA_CH1_MISC
#define BDMA_CH_ADDR_DECDIR         _BIT0
#define BDMA_CH_DONE_INT_EN         _BIT1
#define BDMA_CH_CRC_REFLECTION      _BIT4

#endif

