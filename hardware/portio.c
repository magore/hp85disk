/**
 @file hardware/portio.c

 @brief PORTIO diagnostic tests for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

*/

#include "user_config.h"

#include "defines.h"
#include "stringsup.h"
#include "debug.h"

/// @brief
///  Help Menu for AVR PORT IO functions 
///  See: int port_tests(char *str)
/// @return  void
void portio_help(int full)
{
    printf("port      help\n");
    if(full)
    {
        printf("Note: port prefix is optional\n"
            "port read pins   [A-D]\n"
            "port read latch  [A-D]\n"
            "port read ddr    [A-D]\n"
            "port write latch [A-D] val\n"
            "port write pins  [A-D] val\n"
            "\n"
            );
	}
}

/// @brief PORT user tests
///  User invoked PORT functions and tasks
/// @return  1 matched token, 0 if not
int portio_tests(int argc __attribute__((unused)), char * argv[])
{
    char *ptr;
    int ind;

    ind = 0;
    ptr = argv[ind++];

    if(!ptr)
        return(0);

    if( MATCH(ptr,"port") )
    {
        ptr = argv[ind];
        if ( !ptr || MATCH(ptr,"help") )
        {
            portio_help(1);
            return(1);
        }
    }

    if( MATCHI(argv[ind],"read") )
    {
        if( MATCHI(argv[ind+1],"pins") )
        {
            if( MATCHI(argv[ind+2],"A"))
            {
                printf("Port pins A RD:[%02XH]\n",  0xff & GPIO_PORT_PINS_RD(PORTA) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"B"))
            {
                printf("Port pins B RD:[%02XH]\n",  0xff & GPIO_PORT_PINS_RD(PORTB) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"C"))
            {
                printf("Port pins C RD:[%02XH]\n",  0xff & GPIO_PORT_PINS_RD(PORTC) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"D"))
            {
                printf("Port pins D RD:[%02XH]\n",  0xff & GPIO_PORT_PINS_RD(PORTD) );
                return (1);
            }
        }
        if( MATCHI(argv[ind+1],"latch") )
        {
            if( MATCHI(argv[ind+2],"A"))
            {
                printf("Port latch A RD:[%02XH]\n",  0xff & GPIO_PORT_LATCH_RD(PORTA) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"B"))
            {
                printf("Port latch B RD:[%02XH]\n",  0xff & GPIO_PORT_LATCH_RD(PORTB) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"C"))
            {
                printf("Port latch C RD:[%02XH]\n",  0xff & GPIO_PORT_LATCH_RD(PORTC) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"D"))
            {
                printf("Port latch D RD:[%02XH]\n",  0xff & GPIO_PORT_LATCH_RD(PORTD) );
                return (1);
            }
        }
        if( MATCHI(argv[ind+1],"ddr") )
        {
            if( MATCHI(argv[ind+2],"A"))
            {
                printf("Port ddr A RD:[%02XH]\n",  0xff & GPIO_PORT_DDR_RD(PORTA) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"B"))
            {
                printf("Port ddr B RD:[%02XH]\n",  0xff & GPIO_PORT_DDR_RD(PORTB) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"C"))
            {
                printf("Port ddr C RD:[%02XH]\n",  0xff & GPIO_PORT_DDR_RD(PORTC) );
                return (1);
            }
            if( MATCHI(argv[ind+2],"D"))
            {
                printf("Port ddr D RD:[%02XH]\n",  0xff & GPIO_PORT_DDR_RD(PORTD) );
                return (1);
            }
        }
    }

// port write
    if( MATCHI(argv[ind],"write") )
    {
        uint8_t val;
        if( MATCHI(argv[ind+1],"pins") )
        {
            val = get_value(argv[ind+3]);

            if( MATCHI(argv[ind+2],"A"))
            {
                GPIO_PORT_WR(PORTA,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"B"))
            {
                GPIO_PORT_WR(PORTB,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"C"))
            {
                GPIO_PORT_WR(PORTC,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"D"))
            {
                GPIO_PORT_WR(PORTD,val);
                return (1);
            }
        }
        if( MATCHI(argv[ind+1],"latch") )
        {

            val = get_value(argv[ind+3]);

            if( MATCHI(argv[ind+2],"A"))
            {
                GPIO_PORT_LATCH_WR(PORTA,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"B"))
            {
                GPIO_PORT_LATCH_WR(PORTB,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"C"))
            {
                GPIO_PORT_LATCH_WR(PORTC,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"D"))
            {
                GPIO_PORT_LATCH_WR(PORTD,val);
                return (1);
            }
        }
        if( MATCHI(argv[ind+1],"ddr") )
        {

            val = get_value(argv[ind+3]);

            if( MATCHI(argv[ind+2],"A"))
            {
                GPIO_PORT_DDR_WR(PORTA,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"B"))
            {
                GPIO_PORT_DDR_WR(PORTB,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"C"))
            {
                GPIO_PORT_DDR_WR(PORTC,val);
                return (1);
            }
            if( MATCHI(argv[ind+2],"D"))
            {
                GPIO_PORT_DDR_WR(PORTD,val);
                return (1);
            }
        }
    }                                             // port write
    return(0);
}
