#include "titania_uart.h"

#define IO_WRITE(addr, val) (*(volatile unsigned int *)(addr) = (val))
#define IO_READ(addr) (*(volatile unsigned int *)(addr))
#define CPU_CLOCK_FREQ 216000000
#define BOTH_EMPTY                  (UART_LSR_TEMT | UART_LSR_THRE)

#define CONSOLE_PORT 0
#define UART_REG8(addr)             UART1_REG8(addr)
#define UART_MSEC_LOOP              ( CPU_CLOCK_FREQ/1000/4 )
#define UART_DELAY(_loop)           { volatile int i; for (i=0; i<(_loop)*UART_MSEC_LOOP; i++); }
#define UART_TIME(_stamp, _loop)    { _stamp = (_loop); }
#define UART_EXPIRE(_stamp)         ( !(--_stamp) )
#define UART_INIT()                 { }
#define UART_ENTRY()                { }
#define UART_RETURN()               { }

static void titania_putc (int portnum, char c)
{
    while( ( UART_REG8(UART_LSR)&UART_LSR_TXFIFO_FULL) == UART_LSR_TXFIFO_FULL);
    UART_REG8(UART_FIFO) = UART_IER_RDA | c;
}

void serial_putc (const char c)
{
    if(c == '\n')
    {
        titania_putc(CONSOLE_PORT, '\r'); //CR
        titania_putc(CONSOLE_PORT, '\n'); //LF
    }
    else
    {
        titania_putc(CONSOLE_PORT, c);
    }
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}


const char *test[] = { 
    "**********************************************",
    "***************************************",
    "**********************************",
    "***************************",
    "***********************",
    "******************",
    "**********",
    "*****",
    " TITANIA UART TEST ",
    "*****",
    "*********",
    "************",
    "*********************",
    "**************************",
    "******************************",
    "************************************",
    "**********************************************",
};    
int titania_uart(int argc, char *argv[]) 
{
    int size = sizeof(test)/sizeof(test[0]);
    int cnt=0;

    while(1)
    {
        serial_puts(test[cnt]);
        cnt = (cnt+1)%size;
    }
}
