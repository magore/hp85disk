/**
 @file main.c

 @brief Main for GPIB emulator for HP85 disk emulator project for AVR8 

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include <hardware/hardware.h>

#include "gpib/defines.h"
#include "gpib/gpib_hal.h"
#include "gpib/gpib.h"
#include "gpib/gpib_task.h"
#include "gpib/gpib_tests.h"
#include "gpib/amigo.h"
#include "gpib/ss80.h"
#include <math.h>

///@brief line buffer for user input

/// @brief  Transmit a character on UART 0
/// @param[in] c: character to write
/// @return  void
void uart_put(uint8_t c)
{
    uart0_putchar(c,0);
}

/// @brief  Receive a character from UART 0
/// @return  character
char uart_get(void)
{
    return(uart0_getchar(0));
}

/// @brief  Get a line from UART 0 up to a maximum of len bytes
/// @param[in] buff: line input buffer
/// @param[in] len: line length maximum
/// @return void
void get_line (char *buff, int len)
{
    int c;
    int i = 0;

	memset(buff,0,len);
    while(1)
    {
        c = uart_get() & 0x7f;
		uart_put(c);
        if (c == '\n' || c == '\r')
		{
            break;
		}

        if (c == '\b')
        {
			if(i > 0) {
				i--;
			}
            continue;
        }

        if (i < (len - 1) )                       /* Visible chars */
        {
            buff[i++] = c;
        }
		else
		{
			break;
		}
    }

	// Discard remaining characters
    while(uart_keyhit(0))
    {
        c = uart_get() & 0x7f;
    }
    buff[i] = 0;
    uart_put('\n');
}


/// @brief  Display the main help menu - calls all other help menus
/// @return  void
/// @see gpib_help()
/// @see fatfs_help()
void help()
{
    myprintf("delay_tests\n");
    myprintf("time\n");
    myprintf("setdate\n");
    myprintf("mem\n");
    myprintf("ifc\n");
    gpib_help();
    fatfs_help();
}


/// @brief  perform tests on delay functions
///
/// This included measurement of avr-libc delays
/// @return  void
void delay_tests()
{
    myprintf("System delays\n");

    clock_elapsed_begin();
    clock_elapsed_end("elapsed timer overhead");

    clock_elapsed_begin();
    _delay_us(100);
    clock_elapsed_end("_delay_us(100)");

    clock_elapsed_begin();
    _delay_us(500);
    clock_elapsed_end("_delay_us(500)");

    myprintf("My delays\n");

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
    time_t seconds;
    tm_t tc;
    ts_t ts;
    tz_t tz;

    if(!rtc_init(0,0L))
    {
        myprintf("rtc uninitilized\n");
        myprintf("attempting rtc init\n");
        if( !rtc_init(1, (time_t) DEFAULT_TIME ) )
        {
            myprintf("rtc force init failed\n");
        }
    }

    if(rtc_read(&tc))
    {
        seconds = timegm(&tc);
    }
    else
    {
        seconds = DEFAULT_TIME;
        myprintf("rtc read errorafter init\n");
    }
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

    if(rtc_read(&tc))
    {
#if 0
        myprintf("rtc_read:%d, day:%d,mon:%d,hour:%d,min:%d,sec:%d, wday:%d\n",
            (int) tc.tm_year + 1900,
            (int) tc.tm_mday,
            (int) tc.tm_mon,
            (int) tc.tm_hour,
            (int) tc.tm_min,
            (int) tc.tm_sec,
            (int) tc.tm_wday);
#endif
        myprintf("rtc asctime: %s\n",asctime(&tc));
        seconds = timegm(&tc);
        myprintf("seconds:     %lu\n",seconds);
        myprintf("asctime:     %s\n", asctime(gmtime(&seconds)));
    }
    else
    {
        myprintf("RTC read failed\n");
    }

    clock_gettime(0, (ts_t *) &ts);
    seconds = ts.tv_sec;
    myprintf("clk seconds: %lu\n",seconds);
    myprintf("clk asctime: %s\n", asctime(gmtime(&seconds)));
}


/// @brief  User command handler - called as main task
///
/// If a keypress is detected read a line from the uart
/// parse the result and call various user functions.
/// ? will return a list of fuctions and paramters permitted
/// @param[in] gpib
/// @return  void
void task(char *line, uint8_t gpib)
{
    char *ptr;
    int len;

	if(gpib)
	{
		gpib_task();
	}

    if(!uart_keyhit(0))
        return;

    uart_put('>');
    get_line(line, 79);

    ptr = skipspaces(line);

    if(fatfs_tests(ptr))
        return;

    if(gpib_tests(ptr))
        return;

    if ( (len = token(ptr,"delay_tests")) )
    {
        delay_tests();
        return;

    }
    else if ( (len = token(ptr,"time")) )
    {
        display_time();
        return;
    }
    else if ( (len = token(ptr,"setdate")) )
    {
        setdate();
        display_time();
        return;
    }
    else if ( (len = token(ptr,"mem")) )
    {
        PrintFree();
        return;

    }
    else if ( (len = token(ptr,"ifc")) )
    {
        gpib_assert_ifc();
        return;

    }
    else if ( (len = token(ptr,"help")) || *ptr == '?' )
    {
        help();
        return;
    }
    myprintf("Error:[%s]\n",line);
}


/// @brief  main() for gpib project
/// @return  should never return!
int main(void)
{
    ts_t ts;
	int cold = 1;
	char *line;

    init_timers();

    SPI0_Init();
    SPI0_Mode(0);
    SPI0_Speed(F_CPU);

    TWI_Init(TWI_BIT_PRESCALE_4, TWI_BITLENGTH_FROM_FREQ(4, 50000));

    uart_init(0, 115200U); // Serial Port Initialize

	myprintf("==============================\n");
    myprintf("INIT\n");
    myprintf("F_CPU: %lu\n", F_CPU);
    myprintf("sin(45) = %f\n", sin(45.0 * 0.0174532925));
    myprintf("cos(45) = %f\n", cos(45.0 * 0.0174532925));
    myprintf("tan(45) = %f\n", tan(45.0 * 0.0174532925));
    myprintf("log(10.0) = %f\n", log(10.0));
    PrintFree();
	line = safecalloc(80,1);
	if(!line)
	{
		myprintf("Calloc: line failed ***************************\n");
	}
	printf("line:%d\n", (uint16_t) line);
    PrintFree();

    gpib_bus_init(0);

    delayms(200);

    gpib_bus_init(0);

    gpib_state_init();

    gpib_timer_init();

    printer_init();

    clock_clear();

    clock_getres(0, (ts_t *) &ts);
    myprintf("SYSTEM_TASK_COUNTER_RES:%ld\n", (uint32_t) ts.tv_nsec);

    setup_clock();

    display_time();

	myprintf("==============================\n");

    mmc_init(1);

    gpib_file_init();

    while (1)
    {
        task(line, cold);
		cold = 0;
    }
}
