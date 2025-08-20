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

#ifndef _SERIAL_TITANIA_H_
#define _SERIAL_TITANIA_H_


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define REG_UART0_BASE              0xBF801300
#define REG_UART1_BASE              0xBF801300
#define REG_UART2_BASE              0xBF801300

#define UART0_REG8(addr)            *((volatile unsigned int*)(REG_UART0_BASE + ((addr)<< 2)))
#define UART1_REG8(addr)            *((volatile unsigned int*)(REG_UART1_BASE + ((addr)<< 2)))
#define UART2_REG8(addr)            *((volatile unsigned int*)(REG_UART2_BASE + ((addr)<< 2)))


//
// UART Register List
//
#define UART_DL                     0           // Divisor Latch
#define UART_DL1_LSB                0           // Divisor Latch Low
#define UART_DL2_MSB                0           // Divisor Latch High
#define UART_FIFO                   1           // Divisor Latch Low
#define UART_IER                    1           // Interrupt Enable Register
#define UART_FCR                    2           // FIFO Control Register
#define UART_LCR                    2           // Line Control Register
#define UART_LSR                    3           // Line Status Register
#define UART_RB                     3           // optional: set rf_pop delay for memack ; [3:0] rf_pop_delay; [6] rf_pop_mode_sel ; [7] reg_rb_read_ack

// UART_IER(3)
// Interrupt Enable Register
#define UART_IER_MASK               0xFF00
#define UART_IER_RDA                0x0100      // Enable receiver data available interrupt
#define UART_IER_THRE               0x0200      // Enable Transmitter holding reg empty int

// UART_FCR(4)
// FIFO Control Register (16650 only)
#define UART_FCR_MASK               0x00FF
#define UART_FCR_RXFIFO_CLR         0x0001
#define UART_FCR_TXFIFO_CLR         0x0002
#define UART_FCR_TRIGGER_0		    0x0000
#define UART_FCR_TRIGGER_1		    0x0010
#define UART_FCR_TRIGGER_2		    0x0020
#define UART_FCR_TRIGGER_3		    0x0030
#define UART_FCR_TRIGGER_4		    0x0040
#define UART_FCR_TRIGGER_5		    0x0050
#define UART_FCR_TRIGGER_6		    0x0060
#define UART_FCR_TRIGGER_7		    0x0070


// UART_LCR(5)
// Line Control Register
// Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
// UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
#define UART_LCR_MASK               0xFF00
#define UART_LCR_CHAR_BITS_5        0x0000      // Wordlength: 5 bits
#define UART_LCR_CHAR_BITS_6        0x0100      // Wordlength: 6 bits
#define UART_LCR_CHAR_BITS_7        0x0200      // Wordlength: 7 bits
#define UART_LCR_CHAR_BITS_8        0x0300      // Wordlength: 8 bits
#define UART_LCR_STOP_BITS_1        0x0000      // 1 bit
#define UART_LCR_STOP_BITS_2        0x0400      // 1.5, 2 bit
#define UART_LCR_PARITY_EN          0x0800      // Parity Enable
#define UART_LCR_EVEN_PARITY_SEL    0x1000      // Even parity select
#define UART_LCR_DIVISOR_EN         0x8000      // Divisor latch access bit

// UART_LSR(6)
// Line Status Register
#define UART_LSR_DR			        0x0001      // Receiver data ready
#define UART_LSR_TXFIFO_EMPTY       0x0020      // Transmitter FIFO empty
#define UART_LSR_TX_EMPTY           0x0040      //
#define UART_LSR_TXFIFO_FULL        0x0080      //

#define UART_RB_MASK                0xFF00
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------



#endif // _SERIAL_TITANIA_H_
