#include <common.h>

#ifdef CONFIG_URANUS

#include "serial_uranus.h"
#include "regCHIP.h"

#define IO_WRITE(addr, val) (*(volatile unsigned int *)(addr) = (val))
#define IO_READ(addr) (*(volatile unsigned int *)(addr))

/*
 * IntegratorCP has two UARTs, use the first one, at 38400-8-N-1
 * Versatile PB has four UARTs.
 */

#define CONSOLE_PORT CONFIG_CONS_INDEX
#define baudRate CONFIG_BAUDRATE
static volatile unsigned char *const port[] = CONFIG_URANUS_PORTS;
#define NUM_PORTS (sizeof(port)/sizeof(port[0]))

static void uranus_putc (int portnum, char c);
static int uranus_getc (int portnum);
static int uranus_tstc (int portnum);

#define CPU_CLOCK_FREQ 216000000
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
    //Disable all interrupts 							        //Interrupt Enable Register
    UART_REG8(UART_IER) = 0x00;

    UART_REG8(UART_SCCR) = UART_SCCR_RST;                       // Receive enable & reset

    //UART_REG8(UART_MCR) = 0x20; // New Baud Rate Mode

    //Reset receiver and transmiter 							//FIFO Control Register
    UART_REG8(UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_1 | UART_FCR_TRIGGER_MASK;

    //Set 8 bit char, 1 stop bit, no parity 					//Line Control Register
    UART_REG8(UART_LCR) = (UART_LCR_WLEN8 | UART_LCR_STOP1) & (~UART_LCR_PARITY);

    //Set baud rate
    UART_REG8(UART_LCR) |= UART_LCR_DLAB;						//Line Control Register
    UART_REG8(UART_DLM) = (UART_DIVISOR >> 8) & 0x00ff;	            //Divisor Latch High
    UART_REG8(UART_DLL) = UART_DIVISOR & 0x00ff;			            //Divisor Latch Low
    UART_REG8(UART_LCR) &= ~(UART_LCR_DLAB);					//Line Control Register

    //Enable receiver data available interrupt
    UART_REG8(UART_IER) = UART_IER_RDI;

    //enable UART Rx
    TOP_REG(REG_TOP_UTMI_UART_SETHL) |= TOP_UART_RX_EN;

	return 0;
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

int serial_getc (void)
{
    MS_U8 u8Ch, u8Reg;
    //MS_U32 u32Timer;

    //eCos ISR/DSR can't call blocking wait primitives such as Mutex

    //Wait Receiver data ready
    //u32Timer = MsOS_GetSystemTime();
    do {
        u8Reg = UART_REG8(UART_LSR);
        if ( (u8Reg & UART_LSR_DR) == UART_LSR_DR )
        {
            break;
        }
        //MsOS_YieldTask(); //trapped if called by eCos DSR or during Init
    } while (1); //( (MsOS_GetSystemTime()-u32Timer) < 1 ); //1ms  //To Do: timeout later

    //By default, EC reads RIU devices 8T after issuing Read.
    // This Read pulse will increase UART Rx FIFO R ptr by one. Thus, at 8T, EC reads the previous 15th char in the FIFO(depth=16B).
    //Workaround: for UART only, EC reads RIU devices immediately after issuing Read.
    //MS_U32 u32OldIntr;
    //MsOS_DiableAllInterrupts(u32OldIntr);
    //REG(0xBF803014) = 0;
    u8Ch = UART_REG8(UART_RX);	//get char
    //REG(0xBF803014) = 8;
    //MsOS_RestoreAllInterrupts(u32OldIntr);

    return u8Ch;

}

int serial_tstc (void)
{
	return uranus_tstc (CONSOLE_PORT);
}

void serial_setbrg (void)
{
}

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

static int uranus_getc (int portnum)
{
	return (int) 0;
}

static int uranus_tstc (int portnum)
{
	return 0;
}

#endif //ifdef CONFIG_URANUS
