/**
 @file gpib/gpib_tests.c

 @brief GPIB diagnostic tests for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2017-2014-2017 Mike Gore, Inc. All rights reserved.

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


/// @brief 
///  Help Menu for User invoked GPIB functions and tasks
///  See: int gpib_tests(char *str)
/// @return  void

void gpib_help()
{
    printf(
		"gpib help\n"
		"gpib prefix is optional\n"
		"gpib addresses\n"
		"gpib config\n"
		"gpib debug N\n"
        "gpib elapsed\n"
        "gpib elapsed_reset\n"
        "gpib task\n"
        "gpib trace filename.txt\n"
		"gpib ifc\n"
        "gpib plot filename.txt\n"
        "gpib ppr_bit_on N\n"
        "gpib ppr_bit_off N\n"
        "gpib ppr_set XX\n"
        "gpib ppr_init\n"
		"\n"
		);
}


/// @brief GPIB user tests
///  User invoked GPIB functions and tasks
/// @return  1 matched token, 0 if not
int gpib_tests(int argc, char * argv[])
{

    char *ptr;
	int ind;

	if(argc < 2)
		return(0);

	ind = 1;
	ptr = argv[ind++];

	// skip optional gpib key word
	if( MATCH(ptr,"gpib") )
		ptr = argv[ind++];

    if ( MATCHARGS(ptr,"gpib_help",(ind+0),argc))
    {
        gpib_help();
        return(1);
    }

    if (MATCHARGS(ptr,"addresses",(ind+0),argc))
    {
		display_Addresses();
        return(1);
    }

    if (MATCHARGS(ptr,"config",(ind+0),argc))
    {
		display_Config();
        return(1);
    }

    if (MATCHARGS(ptr,"debug", (ind+1) ,argc))
    {
		debuglevel = get_value(argv[ind]);
        printf("debug=%04XH\n", debuglevel);
        return(1);
    }

    if (MATCHARGS(ptr,"elapsed_reset",(ind+0),argc))
    {
        gpib_timer_elapsed_begin();
        return(1);
    }

    if (MATCHARGS(ptr,"elapsed",(ind+0),argc))
    {
        gpib_timer_elapsed_end("gpib elapsed:");
        return(1);
    }

    if (MATCHARGS(ptr,"task",(ind+0),argc))
    {
        gpib_task();
        return(1);
    }

    if (MATCHARGS(ptr,"trace", (ind+1) ,argc))
    {
        gpib_trace_task(argv[ind]);
        return(1);
    }

    if ( MATCHARGS(ptr, "ifc",(ind+0),argc))
    {
        gpib_assert_ifc();
        return;

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

    if (MATCHARGS(ptr,"ppr_init",(ind+0),argc))
    {
        ppr_init();
        return(1);
    }

    return(0);
}
