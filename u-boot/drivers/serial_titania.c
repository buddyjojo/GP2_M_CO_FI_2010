#include <common.h>
#include "serial_titania.h"
#include <../board/GP2_DEMO1/regCHIP.h>

#define IO_WRITE(addr, val)			(*(volatile unsigned int *)(addr) = (val))
#define IO_READ(addr)				(*(volatile unsigned int *)(addr))
#define CONSOLE_PORT				CONFIG_CONS_INDEX

/*
 * IntegratorCP has two UARTs, use the first one, at 38400-8-N-1
 * Versatile PB has four UARTs.
 */
#define NUM_PORTS					(sizeof(port)/sizeof(port[0]))

static void titania_putc (int portnum, char c);
static int titania_getc (int portnum);
static int titania_tstc (int portnum);

#define CPU_CLOCK_FREQ				CPU_CLOCK_RATE
#define BOTH_EMPTY                  (UART_LSR_TEMT | UART_LSR_THRE)

#define UART_REG8(addr)             UART1_REG8(addr)
#define UART_MSEC_LOOP              ( CPU_CLOCK_FREQ/1000/4 )
#define UART_DELAY(_loop)           { volatile int i; for (i=0; i<(_loop)*UART_MSEC_LOOP; i++); }
#define UART_TIME(_stamp, _loop)    { _stamp = (_loop); }
#define UART_EXPIRE(_stamp)         ( !(--_stamp) )
#define UART_INIT()                 { }
#define UART_ENTRY()                { }
#define UART_RETURN()               { }

int serial_init (void)
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

	return 0;
}

void serial_putc (const char c)
{
	if(c == '\n')
		titania_putc(CONSOLE_PORT, '\r'); //CR
	titania_putc(CONSOLE_PORT, c);
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_getc (void)
{
	return titania_getc(CONSOLE_PORT);
}

int serial_tstc (void)
{
	return titania_tstc (CONSOLE_PORT);
}

void serial_setbrg (void)
{
	//dhjung LGE
	*(volatile unsigned int*)(0xbf201318) |= 0x0080;
	*(volatile unsigned int*)(0xbf201300)  = UART_DIVISOR & 0x00FF;
	*(volatile unsigned int*)(0xbf201308)  = (UART_DIVISOR >> 8) & 0x00FF;
	*(volatile unsigned int*)(0xbf201318) &= 0xFF7F;
}

static void titania_putc (int portnum, char c)
{
	MS_U8 u8Reg;
	MS_U32 u32Timer;

	//Wait for Transmit-hold-register empty
	UART_TIME(u32Timer,10)  //10ms for OS version
	do {
		u8Reg = UART_REG8(UART_LSR);
		if ((u8Reg & UART_LSR_THRE) == UART_LSR_THRE)
		{
			break;
		}
	}while(1);

	UART_REG8(UART_TX) = c;	//put char

	//Wait for both Transmitter empty & Transmit-hold-register empty
	UART_TIME(u32Timer,10)  //10ms for OS version
	do {
		u8Reg = UART_REG8(UART_LSR);
		if ((u8Reg & BOTH_EMPTY) == BOTH_EMPTY)
		{
			break;
		}
	}while(1);
}

static int titania_getc (int portnum)
{
	MS_U8 u8Ch, u8Reg;
	do {
		u8Reg = UART_REG8(UART_LSR);
		if ( (u8Reg & UART_LSR_DR) == UART_LSR_DR )
		{
			break;
		}
	} while (1);

	u8Ch = UART_REG8(UART_RX);	//get char
	return u8Ch;
}

static int titania_tstc (int portnum)
{
	return ((UART_REG8(UART_LSR)&UART_LSR_DR) == UART_LSR_DR); //TRUE if data in Rx FIFO
}
