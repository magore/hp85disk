/**
 @file lib/timer.c

 @brief High resolution timer library and user tasks for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include <hardware/cpu.h>
#include <lib/time.h>

///@brief system interrupt rate in HZ
#define SYSTEM_HZ 1000L
#include <lib/timer.h>

/// @brief  array or user timers
TIMERS timer_irq[MAX_TIMER_CNT];

/// @brief  Setup all timers tasksi and enable interrupts
///
/// @see clock_task()
/// @see: timer_hal.c for hardware dependent interface
///
/// @return  void
void init_timers()
{
    delete_all_timers();

///  See time.c
    clock_clear();

    setup_timers_isr();

///  See time.c
    if(set_timers(clock_task,1) == -1)            // Install Clock Task on timer1
        myprintf("Clock task init failed\n");

    sei();
}


/// @brief  Execute all user timers at SYSTEM_HZ rate.
///
/// @return  void
void execute_timers()
{
    int i;

    for(i=0; i < MAX_TIMER_CNT; i++)
    {
        if(timer_irq[i].timer && timer_irq[i].user_timer_handler)
            (*timer_irq[i].user_timer_handler)();
    }
}



/// @brief  Install a user timer task.
///
/// - timer argument is unsused, reserved for timer source.
///
/// @param[in] handler):  function pointer to user task.
/// @param[in] timer:  reserved for systems with additional low level hardware timers.
///
/// @return timer on success.
/// @return -1 on error.
int set_timers(void (*handler)(void), int timer)
{
    int i;
    int ret = -1;

    if(!handler)
        return -1;

    for(i=0;i<MAX_TIMER_CNT;++i)
    {

        if(timer_irq[i].user_timer_handler == handler)
            break;

        if(!timer_irq[i].user_timer_handler)
        {
            cli();
            timer_irq[i].user_timer_handler = handler;
            timer_irq[i].timer = 1;               // Set to 1 if enabled, 0xff if not
            sei();
            ret = i;
            break;
        }
    }
    if(ret == -1)
        myprintf("set_timers: No more timers!\n");

    return(ret);
}


/// @brief  Delete "Kill" one user timer task.
///
/// - "kill" is a common Linux term for ending a process.
///
/// @param[in] timer: user timer task index.
///
/// @return timer on success.
/// @return -1 on error.
int kill_timers( int timer )
{
    int ret = -1;
    cli();
    if(timer >= 0 && timer <= MAX_TIMER_CNT)
    {
        timer_irq[timer].timer = 0;               // Disable
        timer_irq[timer].user_timer_handler = 0;
        ret = timer;
    }
    sei();
    return(ret);
}


/// @brief  Clear ALL user timer tasks.
///
/// @return  void
void delete_all_timers()
{
    int i;
    cli();
    for(i=0; i < MAX_TIMER_CNT; i++)
    {
        timer_irq[i].timer = 0;                   // Disable
        timer_irq[i].user_timer_handler = 0;
    }
    sei();
}



/// @brief  subtract a-= b timespec * structures.
///
/// @param[in] a: timespec struct.
/// @param[in] b: timespec struct.
///
/// @return  void.
void subtract_timespec(ts_t *a, ts_t *b)
{
    a->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (a->tv_nsec < 0L)
    {
        a->tv_nsec += 1000000000L;
        a->tv_sec --;
    }

    a->tv_sec = a->tv_sec - b->tv_sec;
}


/// @brief  timespec structure in seconds.nanoseconds in string.
///
/// @param[in] : timespec struct we wish to display.
/// @param[in] : output string.
///
/// @return  void.
static char _ts_to_str[32];
char * ts_to_str(ts_t *val)
{
    mysprintf(_ts_to_str,"%ld.%09ld", val->tv_sec, val->tv_nsec);
    return( _ts_to_str );
}


/// @brief  timespec structure in seconds.nanoseconds.
///
/// @param[in] val: timespec struct we want to display.
///
/// @return  void.
void display_ts(ts_t *val)
{
    myprintf("[Seconds: %s]\n", ts_to_str(val) );
}


/// @brief Used for elapsed time calculations.
/// @see clock_elapsed_begin().
/// @see clock_elapsed_end().
static ts_t __clock_elapsed;

/// @brief Store current struct timespec in __clock_elapsed.
///
/// @see clock_gettime() POSIX function.
///
/// @return  void
void clock_elapsed_begin()
{
    clock_gettime(0, (ts_t *) &__clock_elapsed);
}


/// @brief Subtract and display time difference from clock_elapesed_begin().
///
/// - Notes: Accuracy is a function of timer resolution, CPU overhead and background tasks.
///
/// @param[in] msg: User message to proceed Time display.
///
/// @return  void
/// @see clock_gettime().
/// @see clock_elapesed_begin().
void clock_elapsed_end(char *msg)
{
    ts_t current;

    clock_gettime(0, (ts_t *) &current);

    subtract_timespec((ts_t *) &current, (ts_t *) &__clock_elapsed);

    if(msg && *msg)
        myprintf("[%s Time:%s]\n", msg, ts_to_str((ts_t *) &current) );
    else
        myprintf("[Time:%s]\n", ts_to_str((ts_t *) &current) );
}
