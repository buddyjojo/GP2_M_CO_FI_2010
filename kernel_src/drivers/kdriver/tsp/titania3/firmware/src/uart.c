////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////


#ifdef TSP_MEM_IC
    #include "board_ic.h"
#elif defined TSP_MEM_QMEM
    #include "board_qmem.h"
#else
    #error "Choose a TSP memory layout"
#endif
#include "uart.h"


#define UART_REG8(addr)             *((volatile unsigned char *)(addr))

void uart_init(void)
{
#if UART_ENABLE

    int divisor;

                                                                        // Fifo Control Register
    UART_REG8(UART_BASE + UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_1;

    // Disable all interrupts                                           // Interrupt Enable Register
    UART_REG8(UART_BASE + UART_IER) = 0x00;

    // Set 8 bit char, 1 stop bit, no parity                            // Line Control Register
    UART_REG8(UART_BASE + UART_LCR) = (UART_LCR_WLEN8 | UART_LCR_STOP) & (~UART_LCR_PARITY);

    // Set baud rate
    divisor = IN_CLK / (16 * UART_BAUD_RATE);
    UART_REG8(UART_BASE + UART_LCR) |= UART_LCR_DLAB;                   // Line Control Register
    UART_REG8(UART_BASE + UART_DLL) = divisor & 0x000000ff;             // Divisor Latch Low
    UART_REG8(UART_BASE + UART_DLM) = (divisor >> 8) & 0x000000ff;      // Divisor Latch High
    UART_REG8(UART_BASE + UART_LCR) &= ~(UART_LCR_DLAB);                // Line Control Register

#endif // UART_ENABLE
}


#if UART_ENABLE
void uart_putc(char c)
{

    unsigned char lsr;

    WAIT_FOR_THRE;                                                      // Wait THR empty
    UART_REG8(UART_BASE + UART_TX) = c;                                 // Put char
    WAIT_FOR_XMITR;                                                     // Wait both Transmitter empty / THR empty
}


char uart_getc(void)
{
  unsigned char lsr;
  char c;

  WAIT_FOR_CHAR;                                                        // Wait Receiver data ready
  c = UART_REG8(UART_BASE + UART_RX);                                   // Get char
  return c;
}
#endif // UART_ENABLE


void uart_print_str(char *p)
{
#if UART_ENABLE
    while(*p != 0)
    {
        if ('\n'== *p)
        {
            uart_putc('\r');
        }
        uart_putc(*p);

        p++;
    }
#endif // UART_ENABLE
}


void uart_print_long(unsigned long ul)
{
#if UART_ENABLE
    int i;
    char c;

    uart_print_str("0x");
    for(i=0; i<8; i++)
    {
        c = (char) (ul>>((7-i)*4)) & 0xf;
        if(c >= 0x0 && c<=0x9)
        {
            c += '0';
        }
        else
        {
            c += 'a' - 10;
        }

        uart_putc(c);
    }
#endif // UART_ENABLE
}


