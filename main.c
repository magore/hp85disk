/**
 @file main.c

 @brief Main for GPIB emulator for HP85 disk emulator project for AVR

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.
*/

#include <user_config.h>
#include "hardware/i2c.h"



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

///@brief Display Copyright
///@return void
void copyright()
{
    printf("Stand alone version of LIF utilities for linux\n");
    printf("HP85 Disk and Device Emulator\n");
    printf(" (c) 2014-2020 by Mike Gore\n");
    printf(" GNU version 3\n");
    printf("-> https://github.com/magore/hp85disk\n");
    printf("   GIT last pushed:   %s\n", GIT_VERSION);
    printf("   Last updated file: %s\n", LOCAL_MOD);
    printf("\n");
}

#ifdef DELAY_TESTS
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
#endif

/* RunBoot define */
typedef void (*RESET_t)(void) __attribute__((noreturn));

#ifdef OPTIBOOT
//void (*RESET) (void) = (0x10000-0x400);
const RESET_t RESET = (RESET_t) (0x10000-0x400);
#else
const RESET_t RESET = (RESET_t) (0);
#endif

/// @brief  Display the main help menu - calls all other help menus
/// @return  void
/// @see gpib_help()
/// @see fatfs_help()
void help()
{

    copyright();

#ifdef FATFS_TESTS
    fatfs_help(0);
#endif

#ifdef POSIX_TESTS
    posix_help(0);
#endif

#ifdef LIF_SUPPORT
    lif_help(0);
#endif

    gpib_help(0);

    printf(
    #ifdef DELAY_TESTS
        "delay_tests\n"
    #endif
        "help\n"
        "mem\n"
        "setdate\n"
        "time\n"
        "reset\n"
        "\n"
        );
}
/// ======================================

#ifdef LCD_SUPPORT

//uint8_t  _cmd1[2] = { 0x7f, 45 };
uint8_t  _cmd1[2] = { 0xfe, 0x80 };
uint8_t _line1 [21] = { ' ' };
uint8_t  _cmd2[2] = { 0xfe, 0xC0 };
uint8_t _line2 [21] = { ' ' };
 
#endif	// LCD_SUPPORT

/// ======================================
#ifdef LCD_SUPPORT

time_t _seconds = 0;
static int _display_cnt = 0;

void lcd_task()
{
	++_display_cnt;
}

#ifdef LCD_SUPPORT
/// @brief Convert tm_t *t structure into POSIX asctime() ASCII string *buf.
///
/// @param[in] t: tm_t structure pointer.
/// @param[out] buf: user buffer for POSIX asctime() string result.
/// - Example output: "Thu Dec  8 21:45:05 EST 2011".
///
/// @return buf string pointer.
MEMSPACE
char *lcd_time(tm_t *t, char *buf, int max)
{
// normaize tm_t before output
    (void) normalize(t,0);
    memset(buf,0,max);
    snprintf(buf,max-1,"%s %2d %02d:%02d:%02d",
        tm_mon_to_ascii(t->tm_mon),
        (int)t->tm_mday,
        (int)t->tm_hour,
        (int)t->tm_min,
        (int)t->tm_sec);
    return(buf);
}

void i2c_lcd_test()
{
	char buf[32];
	uint8_t sreg=SREG;
    ts_t ts;

	cli();
	if(!mmc_ins_status())
	{
		sprintf((char *) _line2,"%-16", "SD Card Fault");
	}
	else
	{
		clock_gettime(0, (ts_t *) &ts);
		sprintf((char *) _line2, "%-16s", lcd_time(gmtime(&(ts.tv_sec)),buf,sizeof(buf)-1) );
		// sprintf((char *) _line2,"%16ld.%03ld", (long) ts.tv_sec, (long) ts.tv_nsec / 1000000UL);
	}
	SREG=sreg;

    i2c_post();
}
#endif	// LCD_SUPPORT


///@ initialize Optional I2C LCD
void setup_lcd()
{
	int ind = 0;
    uint8_t sreg = SREG;
	extern i2c_op_t *i2c_op[];

    printf("I2C LCD initialization start\n");

	if(set_timers(lcd_task,1) == -1)
        printf("lcd_task init failed\n");

	cli();
	i2c_init(100000);

    sprintf((char *) _line1, "%-16s", "HP85Disk V2");
    sprintf((char *) _line2, "%-16s", "(C)Mike Gore");

	i2c_op[ind++] = i2c_op_add(0x72, TW_WRITE, _cmd1, 2);
	i2c_op[ind++] = i2c_op_add(0x72, TW_WRITE, _line1, 16);
	i2c_op[ind++] = i2c_op_add(0x72, TW_WRITE, _cmd2, 2);
	i2c_op[ind++] = i2c_op_add(0x72, TW_WRITE, _line2, 16);

	SREG = sreg;

    i2c_post();
    delayms(1000);
    if(!i2c_done() || !i2c_ok())
        printf("I2C LCD is NOT attached!\n");
    sep();
}
#endif	// LCD_SUPPORT

/// ======================================
#ifdef LCD_SUPPORT
void gpib_user_task()
{
	uint8_t sreg = SREG;

	cli();
	if(_display_cnt > 100) // 10HZ
	{
		_display_cnt = 0;
		SREG = sreg;
		i2c_lcd_test();
		return;
	}
	SREG = sreg;
}
#else // LCD_SUPPORT
void gpib_user_task()
{
}
#endif	// LCD_SUPPORT





/// ======================================

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
    int result = 0;
    char *argv[50];
    char line[256];


    if(gpib)
        gpib_task();

    if(!kbhit(0))
        return;

    printf("\n>");

    fgets(line,sizeof(line)-2,stdin);
    trim_tail(line);
    argc = split_args(line,argv,50);

#if 0
    printf("Arguments:\n");
    printf("   argc = %d\n", argc);
    for(i=0;i<argc;++i)
        printf("   [%s]\n", argv[i]);
#endif

    ind = 0;
    ptr = argv[ind++];

    if(!ptr || argc < 1)
    {
        result = 1;
    }
#ifdef DELAY_TESTS
    else if (MATCHI(ptr,"delay_tests") )
    {
        delay_tests();
        result = 1;

    }
#endif
    else if ( MATCHI(ptr,"time") )
    {
        display_clock();
        result = 1;
    }
    else if ( MATCHI(ptr,"lcd") )
    {
		i2c_lcd_test();
        result = 1;
    }
    else if ( MATCHI(ptr,"reset") )
    {
        cli();
        uart_rx_flush(0);
        cli();
        MCUSR = (1 << EXTRF);
        RESET();
		// should not return!
        result = 1;
    }
    else if ( MATCHI(ptr,"setdate" ) )
    {
        setdate();
        display_clock();
        result = 1;
    }
    else if ( MATCH(ptr,"mem") )
    {
        PrintFree();
        result = 1;

    }
    else if ( MATCHI(ptr,"help") || MATCHI(ptr,"?") )
    {
        help();
        result = 1;
    }

    else if(gpib_tests(argc,argv))
    {
// Restore GPIB BUS states
        gpib_init_devices();
        result = 1;
    }

#ifdef POSIX_TESTS
    else if(posix_tests(argc,argv))
        result = 1;
#endif

#ifdef FATFS_TESTS
    else if(fatfs_tests(argc,argv))
        result = 1;
#endif

#ifdef LIF_SUPPORT
    else if(lif_tests(argc,argv))
        result = 1;
#endif
    if(result)
        printf("OK\n");
    else
        printf("Error:[%s]\n",line);
}


/// @brief  main() for gpib project
/// @return  should never return!
int main(void)
{
    ts_t ts;
    uint32_t actual,baud;
	char tmp[32];

	clear_error();		// Clear error state

	GPIO_PIN_LOW(LED1);	// Activity status

///@ initialize bus state as soon as practical
    gpib_bus_init();

// BAUD setting moved to Makefile
    baud = BAUD;

///@ Initialize UART early
/// Returns actual BAUD rate - possible with hardware - may differ slightly
    actual = uart_init(0, baud);                  // Serial Port Initialize

///@brief Power up delay
    delayms(200);

    sep();
    printf("Start\n");
    printf("CPU Clock = %lu\n", F_CPU);
    printf("Requested Baud Rate: %ld, Actual: %ld\n", (long)baud, (long)actual);

    init_timers();

    sep();
    printf("HP85 Disk and Device Emulator\n");
    printf(" (C) 2014-2020 by Mike Gore\n");
    printf(" GNU version 3\n");
    printf("-> https://github.com/magore/hp85disk\n");
    printf("   GIT last pushed:   %s\n", GIT_VERSION);
    printf("   Last updated file: %s\n", LOCAL_MOD);

    sep();
    PrintFree();

    sep();
    delayms(200);                                 ///@brief Power up delay

///@ initialize SPI bus
    printf("Initializing SPI bus\n");
    spi_init(MMC_SLOW,GPIO_B3);

///@ initialize I2C bus
    printf("Initializing I2C bus\n");
    TWI_Init(TWI_BIT_PRESCALE_4, TWI_BITLENGTH_FROM_FREQ(4, 100000));
    sep();

///@ initialize clock by RTC if we have it
    printf("Initializing RTC\n");
    clock_clear();
    printf("Clock cleared\n");
    clock_getres(0, (ts_t *) &ts);
    printf("SYSTEM_TASK_COUNTER_RES:%ld\n", (uint32_t) ts.tv_nsec);

// Timezone offset
    initialize_clock(0);
    display_clock();
    sep();

///@ initialize Optional I2C LCD
#ifdef LCD_SUPPORT
	setup_lcd();
#endif

///@ initialize MMC bus
    printf("MMC initializing start\n");
    if ( !mmc_init(1) )
		printf("MMC initialized\n");
    sep();

///@ initialize bus state as soon as practical
    gpib_bus_init();
    printf("GPIB bus initialized\n");

///@ initialize Printer Capture
    printer_init();
    printf("Printer initialized\n");

///@ initialize GPIB timer tasks
    printf("GPIB Timer Setup\n");
    gpib_timer_init();
    printf("GPIB Timer initialized\n");

///@brief Process hp85disk emulator config file
    gpib_file_init();
    printf("GPIB File init done\n");

///@brief GPIB talking/listening state variables
///Must be done AFTER gpib_file_init() so we have a valid configuration
    gpib_state_init();
    printf("GPIB State init done\n");
    sep();

///@brief Display Address Summary
    display_Addresses(0);
    sep();

///@brief Display debug level
    printf("debuglevel   = %04xH\n",(int)debuglevel);
    sep();

///@brief Format any drives that do not yet exist
    format_drives();

#ifdef LCD_SUPPORT
	sprintf((char *) tmp, "SS80=%d AMIGO=%d",
		(int) count_drive_types(SS80_TYPE),
		(int) count_drive_types(AMIGO_TYPE) );
	sprintf((char *) _line1, "%-16s", tmp);
	sprintf((char *) _line2, "%-16s", "(C)Mike Gore");
	i2c_post();
	delayms(1000);
    if(!i2c_done() || !i2c_ok())
		printf("I2C LCD is NOT attached!\n");
#endif

///@brief Start main GPIB state machine
    printf("Starting GPIB TASK\n");

///@brief Keep the task running 
/// task does not exit unless a keypress occurs
/// When it restarts ALL GPIB states are reset 
    while ( 1)
    {
        task(1);
    }
}
