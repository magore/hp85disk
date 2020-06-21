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
    printf("gpib      help\n");
    if(full)
    {
        printf("Note: gpib prefix is optional\n"
            "gpib debug N\n"
            "   debug message reporting see hpdisk.cfg for details\n"
            "gpib elapsed\n"
            "gpib elapsed_reset\n"
            "gpib ifc\n"
            "gpib task\n"
            "gpib trace filename.txt [BUS]\n"
            "   Display activity of GPIB bus and log it\n"
            "   BUS - include handshake states\n"
            "\n"
#ifdef GPIB_EXTENDED_TESTS
            "gpib ppr_init\n"
            "gpib ppr_bit_on N\n"
            "gpib ppr_bit_off N\n"
            "gpib ppr_set XX\n"
            "\n"
#endif
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
#endif                                        // #ifdef GPIB_EXTENDED_TESTS

    return(0);
}
