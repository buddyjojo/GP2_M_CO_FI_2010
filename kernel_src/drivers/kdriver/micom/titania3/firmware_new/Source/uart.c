#include <intrins.h>
//#include "config.h"
#include "datatype.h"
#include "R8051XC.h"

////////////////////////////////////////////////////////////////////////////////
// R8051XC Serial0 and Serial1
////////////////////////////////////////////////////////////////////////////////
static U8 data u8UartDef = 0;
#if 0
void MDrv_Sys_SelectUART( U8 u8UartNum )
{
    u8UartDef = u8UartNum;
}

char putchar(char c)
{
	if ( u8UartDef == 0 )
	{
        UD = _testbit_( ES0 );
        TI0 = 0;

        if ( c == '\n' )
        {
            S0BUF = '\r';
            while ( !TI0 ) ;
            TI0 = 0;
        }
        S0BUF = c;

        while ( !TI0 ) ;
        TI0 = 0;

        if ( UD )
	        ES0 = 1;
	}
    else
    {
        UD = (IEN2 & BIT0);
        IEN2 &= ~BIT0;
        S1CON &= ~BIT1;

        if ( c == '\n' )
        {
            S1BUF = '\r';
            while ( !(S1CON & BIT1) ) ;
            S1CON &= ~BIT1;
        }
        S1BUF = c;

        while ( !(S1CON & BIT1) ) ;
        S1CON &= ~BIT1;

        if ( UD )
        	IEN2 |= BIT0;
    }
    return c;
}
#endif
#if 0
char _getkey( void )
{
    char c;

	if ( u8UartDef == 0 )
	{
        while ( !RI0 ) ;

        c = S0BUF;
        RI0 = 0;
    }
    else
    {
        while ( !(S1CON & BIT0) ) ;

        c = S1BUF;
        S1CON &= ~BIT0;
    }
    return c;
}
#endif
