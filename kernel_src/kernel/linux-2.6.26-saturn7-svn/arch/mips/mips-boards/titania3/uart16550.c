/*
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "uart16550.h"
#include "chip_setup.h"

void Uart16550Init(uint32 baud, uint8 data, uint8 parity, uint8 stop)
{
	*(volatile unsigned int*)(0xbf001c24) |= 0x0800;
#ifdef CONFIG_UART0 //for DSUB
	*(volatile unsigned int*)(0xbf203d4c) |= 0x0004;//bit 12 UART Enable, bit 11 UART RX Enable
	// Junyou.Lin 20090203 Switch UART0 to PIU UART 0
	*(volatile unsigned int*)(0xbf203c04) |= 0x0400;          // Clear UART_SEL0
#else //for RS232C
	*(volatile unsigned int*)(0xbf203d4c)  = 0x0400;
	*(volatile unsigned int*)(0xbf203d50)  = 0x0000;
	*(volatile unsigned int*)(0xbf203c08) |= 0x0800;
	*(volatile unsigned int*)(0xbf203c08) &= 0xFBFF;
#endif

	*(volatile unsigned int*)(0xbf201318) |= 0x0080;
	*(volatile unsigned int*)(0xbf201300)  = UART_DIVISOR & 0x00FF;
	*(volatile unsigned int*)(0xbf201308)  = (UART_DIVISOR >> 8) & 0x00FF;
	*(volatile unsigned int*)(0xbf201318) &= 0xFF7F;

	*(volatile unsigned int*)(0xbf201310)  = 0x0000;
	*(volatile unsigned int*)(0xbf201310)  = 0x0007;
	*(volatile unsigned int*)(0xbf201318)  = 0x0000;
	*(volatile unsigned int*)(0xbf201318)  = 0x0003;
	*(volatile unsigned int*)(0xbf201320)  = 0x0000;
	*(volatile unsigned int*)(0xbf201308)  = 0x0000;
}

#if 0
void Uart16550Put(uint8 byte)
{
	unsigned char u8Reg ;
	unsigned char* a1 = 0xA0100000 ;
	unsigned char* a2 = 0xA0100004 ;
	unsigned int i = 0 ;

#define UART_LSR_THRE		        0x20	                            // Transmit-hold-register empty
#define UART_LSR_TEMT		        0x40	                            // Transmitter empty
	do {
		u8Reg = UART_REG8(UART_LSR);
		if ((u8Reg & UART_LSR_THRE) == UART_LSR_THRE)
		{
			break;
		}
		*a1 = i++ ;
		//MsOS_YieldTask(); //trapped if called by eCos DSR
	}while(1);//(!UART_EXPIRE(u32Timer));

#define UART_TX     0
	UART_REG8(UART_TX) = byte;	//put char

#define BOTH_EMPTY                  (UART_LSR_TEMT | UART_LSR_THRE)
	//Wait for both Transmitter empty & Transmit-hold-register empty
	do {
		u8Reg = UART_REG8(UART_LSR);
		if ((u8Reg & BOTH_EMPTY) == BOTH_EMPTY)
		{
			break;
		}
		*a2 = i++ ;
		//MsOS_YieldTask(); //trapped if called by eCos DSR
	}while(1);//(!UART_EXPIRE(u32Timer));
}
#endif

void Uart16550IntrruptEnable(void)
{
	Uart16550Init(1,2,3,4);
}
