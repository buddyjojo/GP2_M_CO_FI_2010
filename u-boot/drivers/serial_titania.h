#ifndef _SERIAL_TITANIA_H_
#define _SERIAL_TITANIA_H_

//dhjung LGE
DECLARE_GLOBAL_DATA_PTR;

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_TITANIA2
// Baud rate
#define UART_BAUD_RATE				115200		//38400 //9600 //19200 //38400 //57600 //115200 =>too frequent Rx INT
// Clock source
#define UART_CLK_FREQ 				123000000	//43200000 //123000000//14318180 //2008/07/31 Nick
#endif

#if defined(CONFIG_TITANIA3_FPGA)
// Baud rate
#define UART_BAUD_RATE				38400		//38400 //9600 //19200 //38400 //57600 //115200 =>too frequent Rx INT
// Clock source
#define UART_CLK_FREQ 				123000000	//43200000 //123000000//14318180 //2008/07/31 Nick
#endif

#if defined(CONFIG_GP2_DEMO1)
// Baud rate
#define UART_BAUD_RATE	            115200//38400 //9600 //19200 //38400 //57600 //115200 =>too frequent Rx INT
// Clock source
#define UART_CLK_FREQ               123000000//43200000 //123000000//14318180 //2008/07/31 Nick
#endif

#if defined(CONFIG_EUCLID)
#define UART_BAUD_RATE	            115200//38400 //9600 //19200 //38400 //57600 //115200 =>too frequent Rx INT
// Clock source
#define UART_CLK_FREQ               172800000//43200000 //123000000//14318180 //2008/07/31 Nick
#endif

// Divisor
#if 0 //dhjung LGE
#define UART_DIVISOR  				(UART_CLK_FREQ/(16 * UART_BAUD_RATE))
//#define UART_DIVISOR				(65536/(UART_CLK_FREQ/(16 * UART_BAUD_RATE)))
#else
#define UART_DIVISOR  				(UART_CLK_FREQ/(16 * gd->baudrate))
#endif

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
/// data type unsigned char, data length 1 byte
typedef unsigned char               MS_U8;       // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              MS_U16;      // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int                MS_U32;      // 4 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;       // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;      // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int                  MS_S32;      // 4 bytes

//-------------------------------------------------------------------------------------------------
//  Function Prototype
//-------------------------------------------------------------------------------------------------

//regUart.h
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#if defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_EUCLID) || defined(CONFIG_GP2_DEMO1)
#define REG_UART0_BASE              0xBF201300   //2008/07/04 Nick
#define REG_UART1_BASE              0xBF201300   //2008/07/04 Nick
#define REG_UART2_BASE              0xBF201300   //2008/07/04 Nick

#if defined(CONFIG_EUCLID)
#define UART0_REG8(addr)            *((volatile unsigned int*)(REG_UART0_BASE + ((addr)<< 2))) //izzy
#define UART1_REG8(addr)            *((volatile unsigned int*)(REG_UART1_BASE + ((addr)<< 2))) //izzy
#define UART2_REG8(addr)            *((volatile unsigned int*)(REG_UART2_BASE + ((addr)<< 2))) //izzy
#else
#define UART0_REG8(addr)            *((volatile unsigned int*)(REG_UART0_BASE + ((addr)<< 3))) //terry
#define UART1_REG8(addr)            *((volatile unsigned int*)(REG_UART1_BASE + ((addr)<< 3))) //terry
#define UART2_REG8(addr)            *((volatile unsigned int*)(REG_UART2_BASE + ((addr)<< 3))) //terry
#endif

#else

#define REG_UART0_BASE              0xBF801300   //2008/07/04 Nick
#define REG_UART1_BASE              0xBF801300   //2008/07/04 Nick
#define REG_UART2_BASE              0xBF801300   //2008/07/04 Nick
#define UART0_REG8(addr)            *((volatile unsigned int*)(REG_UART0_BASE + ((addr)<< 2))) //terry
#define UART1_REG8(addr)            *((volatile unsigned int*)(REG_UART1_BASE + ((addr)<< 2))) //terry
#define UART2_REG8(addr)            *((volatile unsigned int*)(REG_UART2_BASE + ((addr)<< 2))) //terry
#endif

// UART Register List
#define UART_RX		                0	        // In:  Receive buffer (DLAB=0), 16-byte FIFO
#define UART_TX		                0	        // Out: Transmit buffer (DLAB=0), 16-byte FIFO
#define UART_DLL	                0	        // Out: Divisor Latch Low (DLAB=1)
#define UART_DL                     0           // Divisor Latch
#define UART_DL1_LSB                0           // Divisor Latch Low
#define UART_DL2_MSB                0           // Divisor Latch High
#if defined(CONFIG_EUCLID) || defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_GP2_DEMO1)
#define UART_DLM	                2	        // Out: Divisor Latch High (DLAB=1)
#else
#define UART_DLM	                1	        // Out: Divisor Latch High (DLAB=1)
#endif
#define UART_FIFO                   1           // Divisor Latch Low
#define UART_IER                    1           // Interrupt Enable Register
#define UART_FCR                    2           // FIFO Control Register
#define UART_IIR	                2	        // In:  Interrupt ID Register
#define UART_EFR	                2	        // I/O: Extended Features Register
#define UART_RB                     3           // optional: set rf_pop delay for memack ; [3:0] rf_pop_delay; [6] rf_pop_mode_sel ; [7] reg_rb_read_ack
// (DLAB=1, 16C660 only)

#if defined(CONFIG_EUCLID) || defined(CONFIG_TITANIA_FPGA)|| defined(CONFIG_TITANIA2) || defined(CONFIG_TITANIA3_FPGA) || defined(CONFIG_GP2_DEMO1)
#define UART_LCR					3	        // Out: Line Control Register
#define UART_LSR					5	        // In:  Line Status Register
#else
#define UART_LCR					2	        // Out: Line Control Register
#define UART_LSR					3           // Line Status Register
#endif
#define UART_MCR	                4	        // Out: Modem Control Register
#define UART_MSR	                6	        // In:  Modem Status Register
#define UART_SCR	                7	        // I/O: Scratch Register

#define UART_SCCR                   8           // Smartcard Control Register
#define UART_SCSR                   9           // Smartcard Status Register

#define UART_SCFC                   10          // Smartcard Fifo Count Register
#define UART_SCFI                   11          // Smartcard Fifo Index Register
#define UART_SCFR                   12          // Smartcard Fifo Read Delay Register

#define UART_SCMR                   13          // Smartcard Mode Register

//
// UART_FCR(2)
// FIFO Control Register (16650 only)
//
#define UART_FCR_ENABLE_FIFO	    0x01		// Enable the FIFO
#define UART_FCR_CLEAR_RCVR		    0x02		// Clear the RCVR FIFO
#define UART_FCR_CLEAR_XMIT		    0x04		// Clear the XMIT FIFO
#define UART_FCR_DMA_SELECT		    0x08		// For DMA applications
#define UART_FCR_TRIGGER_MASK	    0xC0		// Mask for the FIFO trigger range
#define UART_FCR_TRIGGER_8		    0x80		// Mask for trigger set at 8
#define UART_FCR_TRIGGER_14		    0xC0		// Mask for trigger set at 14

//
// UART_LCR(3)
// Line Control Register
// Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
// UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
//
#define UART_LCR_WLEN5		        0x00		// Wordlength: 5 bits
#define UART_LCR_WLEN6		        0x01		// Wordlength: 6 bits
#define UART_LCR_WLEN7		        0x02		// Wordlength: 7 bits
#define UART_LCR_WLEN8		        0x03		// Wordlength: 8 bits
#define UART_LCR_STOP1		        0x00		//
#define UART_LCR_STOP2		        0x04		// Stop bits: 0=1 stop bit, 1= 2 stop bits
#define UART_LCR_PARITY		        0x08		// Parity Enable
#define UART_LCR_EPAR		        0x10		// Even parity select
#define UART_LCR_SPAR		        0x20		// Stick parity (?)
#define UART_LCR_SBC		        0x40		// Set break control
#define UART_LCR_DLAB		        0x80		// Divisor latch access bit

// UART_FCR(4)
// FIFO Control Register (16650 only)
#define UART_FCR_MASK               0x00FF
#define UART_FCR_RXFIFO_CLR         0x0001
#define UART_FCR_TXFIFO_CLR         0x0002
#define UART_FCR_TRIGGER_0		    0x0000
#define UART_FCR_TRIGGER_1		    0x0000 //2009/04/29 Nick
#define UART_FCR_TRIGGER_2		    0x0020
#define UART_FCR_TRIGGER_3		    0x0030
#define UART_FCR_TRIGGER_4		    0x0040
#define UART_FCR_TRIGGER_5		    0x0050
#define UART_FCR_TRIGGER_6		    0x0060
#define UART_FCR_TRIGGER_7		    0x0070

//
// UART_LSR(5)
// Line Status Register
//
#define UART_LSR_DR			        0x01		// Receiver data ready
#define UART_LSR_OE			        0x02		// Overrun error indicator
#define UART_LSR_PE			        0x04		// Parity error indicator
#define UART_LSR_FE			        0x08		// Frame error indicator
#define UART_LSR_BI			        0x10		// Break interrupt indicator
#define UART_LSR_THRE		        0x20		// Transmit-hold-register empty
#define UART_LSR_TEMT		        0x40		// Transmitter empty

// UART_LSR(6)
// Line Status Register
#define UART_LSR_TXFIFO_FULL        0x0080      //

//
// UART_IIR(2)
// Interrupt Identification Register
//
#define UART_IIR_MSI		        0x00		// Modem status interrupt
#define UART_IIR_NO_INT		        0x01		// No interrupts pending
#define UART_IIR_THRI		        0x02		// Transmitter holding register empty
#define UART_IIR_TOI		        0x0c		// Receive time out interrupt
#define UART_IIR_RDI		        0x04		// Receiver data interrupt
#define UART_IIR_RLSI		        0x06		// Receiver line status interrupt
#define UART_IIR_ID			        0x06		// Mask for the interrupt ID

//
// UART_IER(1)
// Interrupt Enable Register
//
#define UART_IER_RDI		        0x01		// Enable receiver data available interrupt
#define UART_IER_THRI		        0x02		// Enable Transmitter holding reg empty int
#define UART_IER_RLSI		        0x04		// Enable receiver line status interrupt
#define UART_IER_MSI		        0x08		// Enable Modem status interrupt

// UART_IER(3)
// Interrupt Enable Register
#define UART_IER_MASK               0xFF00
#define UART_IER_RDA                0x0100      // Enable receiver data available interrupt
#define UART_IER_THRE               0x0200      // Enable Transmitter holding reg empty int

//
// UART_MCR(4)
// Modem Control Register
//
#define UART_MCR_DTR		        0x01		// DTR complement
#define UART_MCR_RTS		        0x02		// RTS complement
#define UART_MCR_OUT1		        0x04		// Out1 complement
#define UART_MCR_OUT2		        0x08		// Out2 complement
#define UART_MCR_LOOP		        0x10		// Enable loopback test mode

#define UART_MCR_FAST               0x20		// Slow / Fast baud rate mode

//
// UART_LCR(5)  //2008/07/09 Nick
//
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

//
// UART_MSR(6)
// Modem Status Register
//
#define UART_MSR_ANY_DELTA	        0x0F		// Any of the delta bits!
#define UART_MSR_DCTS		        0x01		// Delta CTS
#define UART_MSR_DDSR		        0x02		// Delta DSR
#define UART_MSR_TERI		        0x04		// Trailing edge ring indicator
#define UART_MSR_DDCD		        0x08		// Delta DCD
#define UART_MSR_CTS		        0x10		// Clear to Send
#define UART_MSR_DSR		        0x20		// Data Set Ready
#define UART_MSR_RI			        0x40		// Ring Indicator
#define UART_MSR_DCD		        0x80		// Data Carrier Detect

//
// UART_EFR(2, UART_LCR_DLAB)
// These are the definitions for the Extended Features Register
// (StarTech 16C660 only, when DLAB=1)
//
#define UART_EFR_ENI		        0x10		// Enhanced Interrupt
#define UART_EFR_SCD		        0x20		// Special character detect
#define UART_EFR_RTS		        0x40		// RTS flow control
#define UART_EFR_CTS		        0x80		// CTS flow control

//
// UART_SCCR(8)
// SmartCard Control Register
//
#define UART_SCCR_MASK_CARDIN       0x01		// Smartcard card in interrupt mask
#define UART_SCCR_MASK_CARDOUT      0x02		// Smartcard card out interrupt mask
#define UART_SCCR_TX_BINV           0x04		// Smartcard Tx bit invert
#define UART_SCCR_TX_BSWAP          0x08		// Smartcard Tx bit swap
#define UART_SCCR_RST               0x10		// Smartcard reset 0->1, UART Rx enable 1
#define UART_SCCR_RX_BINV           0x20		// Smartcard Rx bit inverse
#define UART_SCCR_RX_BSWAP          0x40		// Smartcard Rx bit swap

//
// UART_SCSR(9)
// Smartcard Status Register
//
#define UART_SCSR_CLK               0x01		// Smartcard clock out
#define UART_SCSR_INT_CARDIN        0x02		// Smartcard card in interrupt
#define UART_SCSR_INT_CARDOUT       0x04		// Smartcard card out interrupt
#define UART_SCSR_DETECT            0x08		// Smartcard detection status

//
// UART_SCFC(10), UART_SCFI(11), UART_SCFR(12)
// Smartcard Fifo Register
//
#define UART_SCFC_MASK              0x07
#define UART_SCFI_MASK              0x0F
#define UART_SCFR_MASK              0x07


//
// UART_SCFR(12)
// Smartcard Fifo Read Delay Register
//
#define UART_SCFR_DELAY_MASK        0x03
#define UART_SCFR_V_HIGH            0x04
#define UART_SCFR_V_ENABLE          0x08		// Vcc = (Vcc_high ^ (Vcc_en & UART_SCSR_INT_CARDOUT))

//
// UART_SCMR(13)
// SMart Mode Register
//
#define UART_SCMR_RETRY_MASK        0x1F
#define UART_SCMR_SMARTCARD         0x20
#define UART_SCMR_2STOP_BIT         0x40

#endif
