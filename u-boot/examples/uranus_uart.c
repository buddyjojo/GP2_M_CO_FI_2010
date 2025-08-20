#include "uranus_uart.h"

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

static void uranus_putc (int portnum, char c)
{
    MS_U8 u8Reg;
    MS_U32 u32Timer;

    //eCos ISR/DSR can't call blocking wait primitives such as Mutex

    //Wait for Transmit-hold-register empty
    UART_TIME(u32Timer,10)  //10ms for OS version
    do {
        u8Reg = UART_REG8(UART_LSR);
        if ((u8Reg & UART_LSR_THRE) == UART_LSR_THRE)
        {
            break;
        }
        //MsOS_YieldTask(); //trapped if called by eCos DSR
    }while(1);//(!UART_EXPIRE(u32Timer));

    UART_REG8(UART_TX) = c;	//put char

    //Wait for both Transmitter empty & Transmit-hold-register empty
    UART_TIME(u32Timer,10)  //10ms for OS version
    do {
        u8Reg = UART_REG8(UART_LSR);
        if ((u8Reg & BOTH_EMPTY) == BOTH_EMPTY)
        {
            break;
        }
        //MsOS_YieldTask(); //trapped if called by eCos DSR
    }while(1);//(!UART_EXPIRE(u32Timer));

}

void serial_putc (const char c)
{
    if(c == '\n')
    {
        uranus_putc(CONSOLE_PORT, '\r'); //CR
        uranus_putc(CONSOLE_PORT, '\n'); //LF
    }
    else
    {
        uranus_putc(CONSOLE_PORT, c);
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
    " URANUS UART TEST ",
    "*****",
    "*********",
    "************",
    "*********************",
    "**************************",
    "******************************",
    "************************************",
    "**********************************************",
};    
int uranus_uart(int argc, char *argv[]) 
{
    int size = sizeof(test)/sizeof(test[0]);
    int cnt=0;

    while(1)
    {
        serial_puts(test[cnt]);
        cnt = (cnt+1)%size;
    }
}
