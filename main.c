/**
 @file main.c

 @brief Main for GPIB emulator for HP85 disk emulator project for AVR 

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

#include <user_config.h>

#include "hardware/iom1284p.h"

#ifndef _IOM1284P_H_
#error _IOM1284P_H_
#endif

#include "gpib/defines.h"
#include "gpib/gpib_hal.h"
#include "gpib/gpib.h"
#include "gpib/gpib_task.h"
#include "gpib/gpib_tests.h"
#include "gpib/printer.h"
#include "gpib/amigo.h"
#include "gpib/ss80.h"
#ifdef POSIX_TESTS
#include "posix/posix_tests.h"
#endif
#ifdef LIF_SUPPORT
#include "lif/lifutils.h"
#endif

#include <math.h>

/// @brief  Display the main help menu - calls all other help menus
/// @return  void
/// @see gpib_help()
/// @see fatfs_help()
void help()
{
#ifdef FATFS_TESTS
    fatfs_help();
#endif
#ifdef POSIX_TESTS
    posix_help();
#endif
#ifdef LIF_SUPPORT
    lif_help();
#endif
    gpib_help();

    printf(
        "delay_tests\n"
        "time\n"
        "setdate\n"
        "mem\n"
        "ifc\n"
        "\n"
        );
    }


/// @brief  perform tests on delay functions
///
/// This included measurement of avr-libc delays
/// @return  void
void delay_tests()
{
    printf("System delays\n");

    clock_elapsed_begin();
    clock_elapsed_end("elapsed timer overhead");

    clock_elapsed_begin();
    _delay_us(100);
    clock_elapsed_end("_delay_us(100)");

    clock_elapsed_begin();
    _delay_us(500);
    clock_elapsed_end("_delay_us(500)");

    printf("My delays\n");

    clock_elapsed_begin();
    delayus(100U);
    clock_elapsed_end("delayus(100)");

    clock_elapsed_begin();
    delayus(500U);
    clock_elapsed_end("delayus(500)");

    clock_elapsed_begin();
    delayus(1100);
    clock_elapsed_end("delayus(1100)");

    clock_elapsed_begin();
    delayms(1000);
    clock_elapsed_end("delayms(1100)");
}


/// @brief  User command handler - called as main task
///
/// If a keypress is detected read a line from the uart
/// parse the result and call various user functions.
/// ? will return a list of fuctions and paramters permitted
/// @param[in] gpib - if non-zero run gpib while there are no user commands
/// @return  void
void task(uint8_t gpib)
{
    char *ptr;
    int ind;
    int argc;
    char *argv[10];
    char line[128];

    if(gpib)
        gpib_task();

    if(!kbhit(0))
        return;

    printf("\n>");
    fgets(line,sizeof(line)-2,stdin);
    argc = split_args(line,argv,10);
#if 0
    printf("Arguments:\n");
    printf("   argc = %d\n", argc);
    for(i=0;i<argc;++i)
        printf("   [%s]\n", argv[i]);
#endif
    if(argc < 2)
        return;

    ind = 1;
    ptr = argv[ind++];

    if(gpib_tests(argc,argv))
    {
        // Restore GPIB BUS states
        gpib_init_devices();
        return;
    }


    if (MATCHARGS(ptr,"delay_tests",(ind+0),argc))
    {
        delay_tests();
        return;

    }
    if ( MATCHARGS(ptr,"time",(ind+0),argc))
    {
        display_clock();
        return;
    }
    if ( MATCHARGS(ptr,"setdate",(ind+0),argc))
    {
        setdate();
        display_clock();
        return;
    }
    if ( MATCHARGS(ptr,"mem",(ind+0),argc))
    {
        PrintFree();
        return;

    }
    if ( MATCHARGS(ptr,"help",(ind+0),argc) || MATCHARGS(ptr,"?",(ind+0),argc))
    {
        help();
        return;
    }
#ifdef POSIX_TESTS
    if(posix_tests(argc,argv))
        return;
#endif
#ifdef FATFS_TESTS
    if(fatfs_tests(argc,argv))
        return;
#endif
#ifdef LIF_SUPPORT
    if(lif_tests(argc,argv))
        return;
#endif
    printf("Error:[%s]\n",ptr);
}



/// @brief  main() for gpib project
/// @return  should never return!
int main(void)
{
    ts_t ts;
    uint32_t actual,baud;

    init_timers();

    // returns actual baud rate which may differ slightly because of limited resolution of baud rate clock and devisors
    // BAUD setting moved to Makefile
    baud = BAUD;

    actual = uart_init(0, baud); // Serial Port Initialize
    delayms(200); ///@brief Power up delay

    sep();
    printf("Start\n");
    printf("CPU Clock = %lu\n", F_CPU);
    printf("Requested Baud Rate: %ld, Actual: %ld\n", (long)baud, (long)actual);

    sep();
    printf("HP85 Disk and Device Emulator\n");
    printf(" (c) 2014-2017 by Mike Gore\n");
    printf(" GNU version 3\n");
    printf("-> https://github.com/magore/hp85disk\n");
    printf("   GIT last pushed:   %s\n", GIT_VERSION);
    printf("   Last updated file: %s\n", LOCAL_MOD);

    sep();
    PrintFree();

    sep();
    delayms(200); ///@brief Power up delay
    ///@ initialize bus state as soon as practical

    printf("initializing GPIB bus\n");
    gpib_bus_init(0);

    printf("initializing SPI bus\n");
    spi_init(MMC_SLOW,GPIO_B3);

    printf("initializing I2C bus\n");
    TWI_Init(TWI_BIT_PRESCALE_4, TWI_BITLENGTH_FROM_FREQ(4, 50000));

    sep();
    clock_clear();
    printf("Clock cleared\n");
    clock_getres(0, (ts_t *) &ts);
    printf("SYSTEM_TASK_COUNTER_RES:%ld\n", (uint32_t) ts.tv_nsec);
    initialize_clock(300);
    display_clock();

    sep();
    mmc_init(1);

    sep();
    printer_init();
    printf("Printer Init done\n");

    sep();
    printf("GPIB Setup\n");

    gpib_timer_init();
    printf("GPIB Timer init done\n");

    ///@brief process config file
    gpib_file_init();
    printf("GPIB File init done\n");

    ///@brief GPIB talking/listening state variables 
    ///Must be done AFTER gpib_file_init() so we have a valid configuration
    gpib_state_init();
    printf("GPIB State init done\n");

    ///@brief Display Config
    sep();
    display_Config();

    ///@format drives if they do not exist
    format_drives();

    ///@brief Address Summary
    sep();
    display_Addresses();

    sep();
    ///@brief Display debug level
    printf("debuglevel   = %04xH\n",(int)debuglevel);

    sep();
    printf("Starting GPIB TASK\n");


    while (1)
    {
        task(1);
    }
}
