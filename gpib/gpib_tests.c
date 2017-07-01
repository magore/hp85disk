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
		"addresses\n"
		"config\n"
		"debug N\n"
        "gpib_elapsed\n"
        "gpib_elapsed_reset\n"
        "gpib_task\n"
        "gpib_trace filename.txt\n"
        "plot_echo address\n   Intruct device to send a plot\n"
        "plot filename.txt\n"
        "ppr_bit_on N\n"
        "ppr_bit_off N\n"
        "ppr_set XX\n"
        "ppr_init\n"
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
    if (MATCHARGS(ptr,"gpib_elapsed_reset",(ind+0),argc))
    {
        gpib_timer_elapsed_begin();
        return(1);
    }
    if (MATCHARGS(ptr,"gpib_elapsed",(ind+0),argc))
    {
        gpib_timer_elapsed_end("gpib elapsed:");
        return(1);
    }
    if (MATCHARGS(ptr,"gpib_task",(ind+0),argc))
    {
        gpib_task();
        return(1);
    }
    if (MATCHARGS(ptr,"gpib_trace", (ind+1) ,argc))
    {
        gpib_trace_task(argv[ind]);
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
    if (MATCHARGS(ptr,"ppr_init",(ind+0),argc))
    {
        ppr_init();
        return(1);
    }
    if ( MATCHARGS(ptr,"gpib_help",(ind+0),argc))
    {
        gpib_help();
        return(1);
    }
    return(0);
}
