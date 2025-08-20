////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009-2010 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (MStar Confidential Information!¡L) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _HAL_MPIF_REG_H_
#define _HAL_MPIF_REG_H_


#define MPIF_DEBUGINFO(x) x

#define MASTER_CLOCK_HZ								(50000000ul)//(233000000ul)

#define MIU_ALIGMENT                                (8)

//---------------------------------------------------------------------------------
#define REG_CKGEN0_BASE                             (0x100B00)
#define REG_CKGEN0_CKG_MPIF                         (REG_CKGEN0_BASE + 0xD0)
//---------------------------------------------------------------------------------
#define REG_CHIP_BASE                               (0x101E00)
#define REG_CHIP_ALLPAD_IN                          (REG_CHIP_BASE + 0xA0)
//---------------------------------------------------------------------------------
#define MPIF_REG_BASE								(0x110400)
#define SPIF_REG_BASE                               (0x3700)

// Physical offset of MPIF
#define OFFSET_SHIFT                                1
#define MPIF_LC1A_SETTINGS							(MPIF_REG_BASE + (0x00 << OFFSET_SHIFT))
#define MPIF_LC2A_REG_CONTROL						(MPIF_REG_BASE + (0x01 << OFFSET_SHIFT))
#define MPIF_LC2A_REG_ADDRESS						(MPIF_REG_BASE + (0x02 << OFFSET_SHIFT))
#define MPIF_LC2A_REG_DATA							(MPIF_REG_BASE + (0x03 << OFFSET_SHIFT))
#define MPIF_LC2B_REG_CONTROL						(MPIF_REG_BASE + (0x04 << OFFSET_SHIFT))
#define MPIF_LC2B_REG_ADDRESS						(MPIF_REG_BASE + (0x05 << OFFSET_SHIFT))
#define MPIF_LC2B_REG_DATA							(MPIF_REG_BASE + (0x06 << OFFSET_SHIFT))
#define MPIF_LC3A_PACKET_CONTROL					(MPIF_REG_BASE + (0x07 << OFFSET_SHIFT))
#define MPIF_LC3A_PACKET_LENGTH						(MPIF_REG_BASE + (0x08 << OFFSET_SHIFT))
#define MPIF_LC3A_PACKET_DATA						(MPIF_REG_BASE + (0x09 << OFFSET_SHIFT))
#define MPIF_LC3A_PACKET_MIU_BASE_ADDRESS			(MPIF_REG_BASE + (0x11 << OFFSET_SHIFT))
#define MPIF_LC3A_PACKET_MIU_BASE_ADDRESS_L			(MPIF_REG_BASE + (0x11 << OFFSET_SHIFT))
#define MPIF_LC3A_PACKET_MIU_BASE_ADDRESS_H			(MPIF_REG_BASE + (0x12 << OFFSET_SHIFT))
#define MPIF_LC3B_PACKET_CONTROL					(MPIF_REG_BASE + (0x13 << OFFSET_SHIFT))
#define MPIF_LC3B_PACKET_LENGTH						(MPIF_REG_BASE + (0x14 << OFFSET_SHIFT))
#define MPIF_LC3B_PACKET_DATA						(MPIF_REG_BASE + (0x15 << OFFSET_SHIFT))
#define MPIF_LC3B_PACKET_MIU_BASE_ADDRESS			(MPIF_REG_BASE + (0x1D << OFFSET_SHIFT))
#define MPIF_LC3B_PACKET_MIU_BASE_ADDRESS_L			(MPIF_REG_BASE + (0x1D << OFFSET_SHIFT))
#define MPIF_LC3B_PACKET_MIU_BASE_ADDRESS_H			(MPIF_REG_BASE + (0x1E << OFFSET_SHIFT))
#define MPIF_LC4A_STREAM_CONTROL					(MPIF_REG_BASE + (0x1F << OFFSET_SHIFT))
#define MPIF_LC4A_STREAM_LENGTH						(MPIF_REG_BASE + (0x20 << OFFSET_SHIFT))
#define MPIF_LC4A_STREAM_MIU_BASE_ADDRESS			(MPIF_REG_BASE + (0x21 << OFFSET_SHIFT))
#define MPIF_LC4A_STREAM_MIU_BASE_ADDRESS_L			(MPIF_REG_BASE + (0x21 << OFFSET_SHIFT))
#define MPIF_LC4A_STREAM_MIU_BASE_ADDRESS_H			(MPIF_REG_BASE + (0x22 << OFFSET_SHIFT))
#define MPIF_4WIRE_SPI_CONTROL						(MPIF_REG_BASE + (0x23 << OFFSET_SHIFT))
#define MPIF_3WIRE_SPI_CONTROL						(MPIF_REG_BASE + (0x24 << OFFSET_SHIFT))
#define MPIF_SPI_CONTROL							(MPIF_REG_BASE + (0x25 << OFFSET_SHIFT))
#define MPIF_SPI_COMMAND_VALUE_L					(MPIF_REG_BASE + (0x26 << OFFSET_SHIFT))
#define MPIF_SPI_COMMAND_VALUE_H					(MPIF_REG_BASE + (0x27 << OFFSET_SHIFT))
#define MPIF_SPI_DATA_LENGTH						(MPIF_REG_BASE + (0x28 << OFFSET_SHIFT)
#define MPIF_SPI_MIU_BASE_ADDRESS_L					(MPIF_REG_BASE + (0x29 << OFFSET_SHIFT))
#define MPIF_SPI_MIU_BASE_ADDRESS_H					(MPIF_REG_BASE + (0x2A << OFFSET_SHIFT))
#define MPIF_CHANNEL_BUSY_STATUS					(MPIF_REG_BASE + (0x2B << OFFSET_SHIFT))
#define MPIF_INTERRUPT_EVENT_MASK					(MPIF_REG_BASE + (0x2C << OFFSET_SHIFT))
#define MPIF_INTERRUPT_EVENT_STATUS					(MPIF_REG_BASE + (0x2D << OFFSET_SHIFT))
#define MPIF_BUSY_TIMEOUT_COUNTER					(MPIF_REG_BASE + (0x2E << OFFSET_SHIFT))
#define MPIF_BUSY_TIMEOUT_COMMAND_ID				(MPIF_REG_BASE + (0x2F << OFFSET_SHIFT))
#define MPIF_MISC1									(MPIF_REG_BASE + (0x30 << OFFSET_SHIFT))
#define MPIF_MISC2									(MPIF_REG_BASE + (0x31 << OFFSET_SHIFT))
#define MPIF_MISC3									(MPIF_REG_BASE + (0x32 << OFFSET_SHIFT))
#define MPIF_SYNC_CONTROL							(MPIF_REG_BASE + (0x33 << OFFSET_SHIFT))
#define MPIF_SPI_UCPLT_LENGTH						(MPIF_REG_BASE + (0x34 << OFFSET_SHIFT))
#define MPIF_MIU_WPROTECT_CONTROL                   (MPIF_REG_BASE + (0x35 << OFFSET_SHIFT))
#define MPIF_MIU_ADDR_LOW_BOUND                     (MPIF_REG_BASE + (0x36 << OFFSET_SHIFT))
#define MPIF_MIU_ADDR_LOW_BOUND_L                   (MPIF_REG_BASE + (0x36 << OFFSET_SHIFT))
#define MPIF_MIU_ADDR_LOW_BOUND_H                   (MPIF_REG_BASE + (0x37 << OFFSET_SHIFT))
#define MPIF_MIU_ADDR_UPPER_BOUND                   (MPIF_REG_BASE + (0x38 << OFFSET_SHIFT))
#define MPIF_MIU_ADDR_UPPER_BOUND_L                 (MPIF_REG_BASE + (0x38 << OFFSET_SHIFT))
#define MPIF_MIU_ADDR_UPPER_BOUND_H                 (MPIF_REG_BASE + (0x39 << OFFSET_SHIFT))
#define MPIF_DEBUG_SELECTION						(MPIF_REG_BASE + (0x40 << OFFSET_SHIFT))
#define MPIF_DEBUG_CHECKSUM							(MPIF_REG_BASE + (0x41 << OFFSET_SHIFT))
#define MPIF_DEBUG_STS_IF_CTRL0						(MPIF_REG_BASE + (0x43 << OFFSET_SHIFT))
#define MPIF_DEBUG_STS_IF_CTRL1						(MPIF_REG_BASE + (0x44 << OFFSET_SHIFT))
#define MPIF_DEBUG_STS_PKT3A_CNT					(MPIF_REG_BASE + (0x45 << OFFSET_SHIFT))
#define MPIF_DEBUG_STS_PKT3B_CNT					(MPIF_REG_BASE + (0x46 << OFFSET_SHIFT))
#define MPIF_DEBUG_STS_STM0_CNT						(MPIF_REG_BASE + (0x47 << OFFSET_SHIFT))
#define MPIF_DEBUG_STS_LC2X_CNT						(MPIF_REG_BASE + (0x48 << OFFSET_SHIFT))
#define MPIF_DEBUG_STS_LC3X_CNT						(MPIF_REG_BASE + (0x49 << OFFSET_SHIFT))
#define MPIF_DEBUG_STS_LC4X_CNT						(MPIF_REG_BASE + (0x4A << OFFSET_SHIFT))

#define MPIF_LC3X_PACKET_DATA_MAX_LENGTH			16 // 16 bytes
#define MPIF_LC3X_MAX_PACKET                        0xFFFF

#define MPIF_EVENT_NULL								0x00
#define MPIF_EVENT_TX_DONE							0x01
#define MPIF_EVENT_TX_ERROR							0x02
#define MPIF_EVENT_MIU_DEBUG                        0x03
#define MPIF_EVENT_BUSYTIMEOUT                      0x04

#define MPIF_INTERRUPT_EVENT_STATUS_BIT_4WSPITX		0x0001
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_3WSPITX		0x0002
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC1ATX		0x0004
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2ATX		0x0008
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2BTX		0x0010
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3ATX		0x0020
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3BTX		0x0040
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC4ATX		0x0080
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2ATXER	0x0100
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC2BTXER	0x0200
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3ATXER	0x0400
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC3BTXER	0x0800
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_LC4ATXER	0x1000
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_BUSYTIMEOUT 0x2000
#define MPIF_INTERRUPT_EVENT_STATUS_BIT_SLAVEREQ	0x0100

#define MPIF_CHANNEL_BUSY_STATUS_BIT_4WSPITX		0x0001
#define MPIF_CHANNEL_BUSY_STATUS_BIT_3WSPITX		0x0002
#define MPIF_CHANNEL_BUSY_STATUS_BIT_LC1ATX			0x0004
#define MPIF_CHANNEL_BUSY_STATUS_BIT_LC2ATX			0x0008
#define MPIF_CHANNEL_BUSY_STATUS_BIT_LC2BTX			0x0010
#define MPIF_CHANNEL_BUSY_STATUS_BIT_LC3ATX			0x0020
#define MPIF_CHANNEL_BUSY_STATUS_BIT_LC3BTX			0x0040
#define MPIF_CHANNEL_BUSY_STATUS_BIT_LC4ATX			0x0080

//---------- SPIF REG Definition ---------------------------
#define REG_SPIF_INT_MASK                           (SPIF_REG_BASE + (0x60 << OFFSET_SHIFT))
#define REG_SPIF_INT_CLR                            (SPIF_REG_BASE + (0x61 << OFFSET_SHIFT))
#define REG_SPIF_INT_RAW_STS                        (SPIF_REG_BASE + (0x63 << OFFSET_SHIFT))
#define REG_SPIF_INT_FINAL_STS                      (SPIF_REG_BASE + (0x64 << OFFSET_SHIFT))
#define REG_SPIF_CHANNEL_BUSY_STATUS                (SPIF_REG_BASE + (0x65 << OFFSET_SHIFT))
#define REG_SPIF_MISC1                              (SPIF_REG_BASE + (0x71 << OFFSET_SHIFT))

#define SPIF_LC1A_BUSY                              0x01
#define SPIF_LC2A_BUSY                              0x02
#define SPIF_LC2B_BUSY                              0x04
#define SPIF_LC3A_BUSY                              0x08
#define SPIF_LC3B_BUSY                              0x10
#define SPIF_LC4A_BUSY                              0x20

#define SPIF_LC1A_TXDONE                            0x01
#define SPIF_LC2A_TXDONE                            0x02
#define SPIF_LC2B_TXDONE                            0x04
#define SPIF_LC3A_TXDONE                            0x08
#define SPIF_LC3B_TXDONE                            0x10
#define SPIF_LC4A_TXDONE                            0x20
#define SPIF_MIU_READDONE                           0x40
#define SPIF_MIU_WRITEDONE                          0x80
#define SPIF_LC2A_CKSUM_ERR                         0x0200
#define SPIF_LC2B_CKSUM_ERR                         0x0400
#define SPIF_LC3A_CKSUM_ERR                         0x0800
#define SPIF_LC3B_CKSUM_ERR                         0x1000
#define SPIF_LC4A_CKSUM_ERR                         0x2000

#define SPIF_EVENT_NULL								0x00
#define SPIF_EVENT_TX_DONE							0x01
#define SPIF_EVENT_TX_ERROR							0x02
//----------------------------------------------------------

// Macro definition
#define REG(addr)                                   (*((volatile U32*)(0xBF000000 + ((addr) << 1))))

// write 2 bytes
#define REG_WR(_reg_, _val_)                        do{ REG((_reg_)) = (_val_); }while(0)

// read 2 bytes
#define REG_RR(_reg_)                               ({REG((_reg_));})

#define REG_WM(_reg_, _val_, _msk_)    \
        do{                         \
        REG((_reg_)) = (REG((_reg_)) & ~(_msk_)) | ((_val_) & (_msk_)); }while(0)

BOOL MDrv_MPIF_InitSPIF(U8 u8slaveid);
BOOL MDrv_MPIF_SetSPIF_IntMask(U8 u8slaveid, U16 event_bit, U16 error_bit);
BOOL MDrv_MPIF_SetSPIF_IntClear(U8 u8slaveid, U16 event_bit, U16 error_bit);
BOOL MDrv_MPIF_GetSPIF_TrcWc(U8 u8slaveid, U8* pu8trc, U8* pu8wc);
BOOL MDrv_MPIF_GetSPIF_ChBusySts(U8 u8slaveid, U16 event_bit, U32 timeout);
U8 MDrv_MPIF_GetSPIF_IntRawSts(U8 u8slaveid, U16 event_bit, U16 error_bit, U32 timeout);
U8 MDrv_MPIF_GetSPIF_FinalRawSts(U8 u8slaveid, U16 event_bit, U16 error_bit, U32 timeout);

//BOOL IRQ_Handler_MPIF(void);

#endif //_HAL_MPIF_REG_H_

