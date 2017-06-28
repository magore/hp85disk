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
#include "lif/lifutils.h"

#include <math.h>

/// @brief  Display the main help menu - calls all other help menus
/// @return  void
/// @see gpib_help()
/// @see fatfs_help()
void help()
{
    printf("delay_tests\n");
    printf("time\n");
    printf("setdate\n");
    printf("mem\n");
    printf("ifc\n");
    gpib_help();
    fatfs_help();
    lif_help();
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


///@brief Defines a default EPOCH time to set the RTC if not initialied
#define DEFAULT_TIME (1406357902L - 3600L * 4L)
///@brief Timezone west in minutes
#define TIMEZONE 300L                             /* west */

/// @brief  Read RTC DS1307
///
///  If not initialized set default time
///@return  void
///@see: clock_settime()
void setup_clock()
{
    time_t seconds = 0;
    tm_t tc;
    ts_t ts;
    tz_t tz;

#ifdef RTC
    if(!rtc_init(0,0L))
    {
        printf("rtc uninitilized\n");
        printf("attempting rtc init\n");
        if( !rtc_init(1, (time_t) DEFAULT_TIME ) )
        {
            printf("rtc force init failed\n");
        }
    }

    if(rtc_read(&tc))
    {
        seconds = timegm(&tc);
    }
    else
    {
        seconds = DEFAULT_TIME;
        printf("rtc read errorafter init\n");
    }
#endif
    tz.tz_minuteswest = 300;
    tz.tz_dsttime = 0;
    settimezone( &tz );

    ts.tv_sec = seconds;
    ts.tv_nsec = 0L;
    clock_settime(0, (ts_t *) &ts);
}

/// @brief  Display current DS1307 RTC time
///
/// @return  void
/// @see: rtc_read
/// @see: timegm()
/// @see: ascitime()
void display_time()
{
    time_t seconds;
    tm_t tc;
    ts_t ts;

#ifdef RTC
    if(rtc_read(&tc))
    {
        seconds = timegm(&tc);
        printf("rtc seconds: %lu\n",seconds);
        printf("rtc time:    %s\n",asctime(&tc));
#if RTC_DEBUG
        printf("rtc_read:%d, day:%d,mon:%d,hour:%d,min:%d,sec:%d, wday:%d\n",
            (int) tc.tm_year + 1900,
            (int) tc.tm_mday,
            (int) tc.tm_mon,
            (int) tc.tm_hour,
            (int) tc.tm_min,
            (int) tc.tm_sec,
            (int) tc.tm_wday);
#endif
    }
    else
    {
        printf("RTC read failed\n");
    }
#endif	// RTC

    clock_gettime(0, (ts_t *) &ts);
    seconds = ts.tv_sec;
    printf("clk seconds: %lu\n",seconds);
    printf("clk time:    %s\n", asctime(gmtime(&seconds)));
}


/// @brief  User command handler - called as main task
///
/// If a keypress is detected read a line from the uart
/// parse the result and call various user functions.
/// ? will return a list of fuctions and paramters permitted
/// @param[in] gpib
/// @return  void
void task(char *line, int max, uint8_t gpib)
{
    char *ptr;
	int ind;

	char *argv[10];
	int argc;

	if(gpib)
		gpib_task();

    if(!uart_keyhit(0))
        return;

    printf("\n>");
	fgets(line,max-1,stdin);

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

    if(fatfs_tests(argc,argv))
        return;

    if(gpib_tests(argc,argv))
        return;

    if(lif_tests(argc,argv))
        return;

    if (MATCH(ptr,"delay_tests"))
    {
        delay_tests();
        return;

    }
    if ( MATCH(ptr,"time"))
    {
        display_time();
        return;
    }
    if ( MATCH(ptr,"setdate") )
    {
        setdate();
        display_time();
        return;
    }
    if ( MATCH(ptr,"mem") )
    {
        PrintFree();
        return;

    }
    if ( MATCH(ptr, "ifc") )
    {
        gpib_assert_ifc();
        return;

    }
    if ( MATCH(ptr,"help") || MATCH(ptr,"?") )
    {
        help();
        return;
    }
    printf("Error:[%s]\n",ptr);
}


#define MAX_LINE 80
char line[MAX_LINE+1];

/// @brief  main() for gpib project
/// @return  should never return!
int main(void)
{
    ts_t ts;
	uint32_t actual,baud;

    init_timers();

	// returns actual baud rate which may differ slightly because of limited resolution of baud rate clock and devisors
	baud = 115200UL;

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
    setup_clock();
    display_time();

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
        task(line, MAX_LINE-1, 1);
    }
}
