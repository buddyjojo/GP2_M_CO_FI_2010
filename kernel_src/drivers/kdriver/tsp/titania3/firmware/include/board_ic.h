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

#ifndef _BOARD_H_
#define _BOARD_H_

#define IC_ENABLE                   1
#define IC_SIZE                     0x2000                              // 8K

#define DC_ENABLE                   0
#define DC_SIZE                     0x1000                              // 4K

#define CACHE_LINE                  0x10


#define IN_CLK                      144000000                           // 36864000 / 43008000 / 55296000
#define TICKS_PER_SEC               100

#define STACK_SIZE                  0x100


#define UART_ENABLE                 0

#define UART_BAUD_RATE              38400                               // 9600 / 115200
#define UART_BASE                   0x90000000
#define UART_IRQ                    19

#endif // _BOARD_H_
