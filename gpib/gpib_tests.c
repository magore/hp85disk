/**
 @file gpib/gpib_tests.c

 @brief GPIB diagnostic tests for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/


#include "user_config.h"

#include "defines.h"
#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "amigo.h"
#include "ss80.h"
#include "gpib_tests.h"


/// @brief 
///  Help Menu for User invoked GPIB functions and tasks
///  See: int gpib_tests(char *str)
/// @return  void

void gpib_help()
{
    printf("debug N\n"
        "ss80_test\n"
        "gpib_task\n"
        "gpib_trace filename.txt\n"
        "ppr_bit_on N\n"
        "ppr_bit_off N\n"
        "ppr_set XX\n"
        "ppr_init\n"
        "plot filename.txt\n"
        "plot_echo 1/0\n"
        "plot filename.txt\n"
        "gpib_elapsed\n"
        "gpib_elapsed_reset\n");
}


/// @brief 
///  User invoked GPIB functions and tasks
/// @return  1 matched token, 0 if not

int gpib_tests(char *str)
{

    int len;
    char *ptr;

    ptr = skipspaces(str);

    len = strlen(ptr);
    if ((len = token(ptr,"debug")) )
    {
        int level;
        ptr += len;
        ptr=skipspaces(ptr);
        sscanf(ptr,"%d", &level);
        debuglevel = level;
        printf("debug=%d\n", debuglevel);
        return(1);
    }
    else if ((len = token(ptr,"ss80_test")) )
    {
        ptr += len;
/// @todo FIXME
        printf("SS80_TEST\n");
        SS80_init();
        SS80_Test();
        return(1);
    }
    else if ((len = token(ptr,"ppr_bit_set")) )
    {
        uint8_t val;
        ptr += len;
        ptr = skipspaces(ptr);
        val = atoh(ptr);
        ppr_bit_set(val);
        return(1);
    }
    else if ((len = token(ptr,"ppr_bit_clr")) )
    {
        uint8_t val;
        ptr += len;
        ptr = skipspaces(ptr);
        val = atoh(ptr);
        ppr_bit_clr(val);
        return(1);
    }
    else if ((len = token(ptr,"ppr_set")) )
    {
        uint8_t val;
        ptr += len;
        ptr = skipspaces(ptr);
        val = atoh(ptr);
        ppr_set(val);
        return(1);
    }
    else if ((len = token(ptr,"ppr_init")) )
    {
        ppr_init();
        return(1);
    }
    else if ((len = token(ptr,"plot_echo")) )
    {
        int option;
        ptr += len;
        ptr = skipspaces(ptr);
        option = atoi(ptr);
        plot_echo(option);
        return(1);
    }
    else if ((len = token(ptr,"gpib_format")) )
    {
        ptr += len;
#if 0
        gpib_format_disk(file);
#endif
        return(1);
    }
    else if ((len = token(ptr,"gpib_elapsed_reset")) )
    {
        ptr += len;
        gpib_timer_elapsed_begin();
        return(1);
    }
    else if ((len = token(ptr,"gpib_elapsed")) )
    {
        ptr += len;
        gpib_timer_elapsed_end("gpib elapsed:");
        return(1);
    }
    else if ((len = token(ptr,"gpib_task")) )
    {
        ptr += len;
        gpib_task();
        return(1);
    }
    else if ((len = token(ptr,"gpib_trace")) )
    {
        ptr += len;
        ptr = skipspaces(ptr);
        gpib_trace_task(ptr);
        return(1);
    }
    else if ((len = token(ptr,"plot_echo")) )
    {
        int option;
        ptr += len;
        ptr = skipspaces(ptr);
        option = atoi(ptr);
        plot_echo(option);
        return(1);
    }
    else if ((len = token(ptr,"plot")) )
    {
        ptr += len;
        ptr = skipspaces(ptr);
        receive_plot(ptr);
        return(1);
    }
    if ( (len = token(ptr,"gpib_help")) )
    {
        gpib_help();
        return(1);
    }
    return(0);
}
