/**
 @file gpib/gpib_tests.c

 @brief GPIB diagnostic tests for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

*/

#include "user_config.h"

#include "defines.h"
#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "amigo.h"
#include "ss80.h"
#include "gpib_tests.h"
#include "stringsup.h"
#include "printer.h"
#include "lifutils.h"
#include "debug.h"

/// @brief
///  Help Menu for User invoked GPIB functions and tasks
///  See: int gpib_tests(char *str)
/// @return  void

void gpib_help(int full)
{
    printf("gpib help\n");
    if(full)
    {
        printf("Note: gpib prefix is optional\n"
            "gpib addresses\n"
            "   Display all device GPIB bus addresses and PPR values\n"
            "gpib config [-v]\n"
            "   Display current drives configuration\n"
            "   -v Verbose - show full detail\n"
            "gpib debug N\n"
            "   debug message reporting see hpdisk.cfg for details\n"
            "gpib elapsed\n"
            "gpib elapsed_reset\n"
            "gpib ifc\n"
            "gpib plot filename.txt\n"
            "gpib plot_echo\n"
            "gpib task\n"
            "gpib trace filename.txt [BUS]\n"
            "   Display activity of GPIB bus and log it\n"
            "   BUS - include handshake states\n"
            "\n"
    #ifdef GPIB_EXTENDED_TESTS
            "gpib port read pins   [A-D]\n"
            "gpib port read latch  [A-D]\n"
            "gpib port read ddr    [A-D]\n"
            "gpib port write latch [A-D] val\n"
            "gpib port write pins  [A-D] val\n"
            "gpib ppr_init\n"
            "gpib ppr_bit_on N\n"
            "gpib ppr_bit_off N\n"
            "gpib ppr_set XX\n"
    #endif
            "\n"
            );
    }
}


/// @brief GPIB user tests
///  User invoked GPIB functions and tasks
/// @return  1 matched token, 0 if not
int gpib_tests(int argc, char * argv[])
{

    char *ptr;
    int ind;

    ind = 0;
    ptr = argv[ind++];

    if(!ptr)
        return(0);

    if( MATCH(ptr,"gpib") )
    {
        ptr = argv[ind++];
        if ( !ptr || MATCH(ptr,"help") )
        {
            gpib_help(1);
            return(1);
        }
    }

    if (MATCHI(ptr,"debug") )
    {
        ptr = argv[ind];
        if(*ptr == '=')
            ++ind;
        debuglevel = get_value(argv[ind]);
        printf("debug=%04XH\n", debuglevel);
        return(1);
    }

    if (MATCHI(ptr,"mount") )
    {
        ptr = argv[ind];
		if(!ptr)
		{
			display_mount();
			return(1);
		}
	
    }

    if (MATCHI(ptr,"addresses") )
    {
        ptr = argv[ind];
        if(ptr && *ptr && MATCH(ptr,"-v"))
            display_Addresses(1);
        else
            display_Addresses(0);
        return(1);
    }

    if (MATCHI(ptr,"config") )
    {
        ptr = argv[ind];
        if(ptr && *ptr && MATCH(ptr,"-v"))
            display_Config(1);
        else
            display_Config(0);
        return(1);
    }

    if (MATCHI(ptr,"elapsed_reset") )
    {
        gpib_timer_elapsed_begin();
        return(1);
    }

    if (MATCHI(ptr,"elapsed") )
    {
        gpib_timer_elapsed_end("gpib elapsed:");
        return(1);
    }

    if (MATCHI(ptr,"task") )
    {
        gpib_task();
        return(1);
    }

    if (MATCHARGS(ptr,"trace", (ind+1) ,argc))
    {
        int detail = 0;
        if(argv[ind+1] && MATCH(argv[ind+1],"BUS"))
            detail = 1;
        gpib_trace_task(argv[ind], detail);
        return(1);
    }

#ifdef GPIB_EXTENDED_TESTS
    if (MATCHARGS(ptr,"ppr_init",(ind+0),argc))
    {
        ppr_init();
        return(1);
    }

    if (MATCHARGS(ptr,"plot_echo", (ind+1) ,argc))
    {
        plot_echo(atoi(argv[ind]) );
        return(1);
    }

    if (MATCHARGS(ptr,"ppr_bit_clr", (ind+1) ,argc))
    {
        ppr_bit_clr(atoh(argv[ind] ));
        return(1);
    }

    if (MATCHARGS(ptr,"ppr_bit_set", (ind+1) ,argc))
    {
        ppr_bit_set(atoh(argv[ind]) );
        return(1);
    }

    if (MATCHARGS(ptr,"ppr_set", (ind+1) ,argc))
    {
        ppr_set(atoh(argv[ind]) );
        return(1);
    }

// port read
    if (MATCHARGS(ptr,"port",(ind+3),argc))
    {
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
    }                                             // port read

// port write
    if (MATCHARGS(ptr,"port",(ind+4),argc))
    {
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
        }

    }                                             // port write
#endif                                        // #ifdef GPIB_EXTENDED_TESTS

    return(0);
}
