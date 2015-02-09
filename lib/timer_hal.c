/**
 @file lib/timer_hal.c

 @brief High resolution timer library and user tasks hardware specific code for AVR8. 

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include <hardware/cpu.h>

#include <lib/timer.h>
#include <lib/time.h>

/// @brief  System Clock Time
extern volatile ts_t __clock;

/// @brief AVR prescale divider.
#define TIMER1_PRESCALE     1L


///@brief Computer AVR counts per interrupt.
#define TIMER1_COUNTS_PER_TIC (F_CPU/TIMER1_PRESCALE/SYSTEM_TASK_HZ)

///@brief Computer AVR count time in Nanoseconds per counter increment.
#define TIMER1_COUNTER_RES (SYSTEM_TASK_TIC_NS/TIMER1_COUNTS_PER_TIC)

#if TIMER1_COUNTS_PER_TIC >= 65535L
#error TIMER1_COUNTS_PER_TIC too big -- increase TIMER1 Prescale
#endif

#define TIMER1_PRE_1 (1 << CS10)						/*< 1 Prescale */
#define TIMER1_PRE_8 (1 << CS11)						/*< 8 Prescale */	
#define TIMER1_PRE_64 ((1 << CS11) | ( 1 << CS10))		/*< 64 Prescale */
#define TIMER1_PRE_256 (1 << CS12)						/*< 256 Prescale */
#define TIMER1_PRE_1024 ((1 << CS12) | ( 1 << CS10))	/*< 1024 Prescape */

/// @brief AVR Timer Interrupt Vector
///
/// - calls execute_timers() - we call this the System task.
///
ISR(TIMER1_COMPA_vect)
{
    execute_timers();
}


/// @brief Setup main timers ISR - this ISR calls execute_timers() task.
///
/// - Notes:
///  - We attempt to use the largest reload count for a given SYSTEM_HZ interrupt rate. This permits using hardware counter offset to increase resolution to the best possible amount.
/// - Assumptions:
///  - We can divide the CPU frequency EXACTLY with timer/counter having no fractional remander.
///
/// @see ISR().
/// @return void.
void setup_timers_isr()
{
    cli();

    TCCR1B=(1<<WGM12) | TIMER1_PRE_1;             // No Prescale
    TCCR1A=0;
    OCR1A=(TIMER1_COUNTS_PER_TIC-1);              // 0 .. count
    TIMSK1 |= (1<<OCIE1A);                        //Enable the Output Compare A interrupt

    sei();
}


/// @brief Read clock time into struct timepec *ts - POSIX function.
///
///  - Note: We ignore clk_id, and include low level counter offsets when available.
///
/// @param[in] clk_id:	unused hardware clock index.
/// @param[out] ts:		timespec result.
///
/// @return 0 on success.
/// @return -1 on error.
int clock_gettime(clockid_t clk_id, struct timespec *ts)
{
    uint16_t count1,count2;                       // must be same size as timer register
    uint32_t offset = 0;
    uint8_t pendingf = 0;
    int errorf = 0;

    cli();


    count1 = TCNT1;

    ts->tv_sec = __clock.tv_sec;
    ts->tv_nsec = __clock.tv_nsec;

    count2 = TCNT1;

    if( TIFR1 & (1<<OCF1A) )
        pendingf = 1;

    if (count2 < count1)
    {
///  note: counter2 < count1 implies ISR flag must be set
        if( !pendingf )
            errorf = -1;                          // counter overflow and NO pending is an error!

        offset = TIMER1_COUNTS_PER_TIC;           // overflow
    }
    else
    {
        if( pendingf )
            offset = TIMER1_COUNTS_PER_TIC;       // overflow
    }
    offset += count2;

    sei();

    offset *= TIMER1_COUNTER_RES;

    ts->tv_nsec += offset;

    if (ts->tv_nsec >= 1000000000L)
    {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec++;
    }
    return(errorf);
}


/// @brief Read clock time resolution into struct timepec *ts - POSIX function.
///
///  - Note: We ignore clk_id, and include low level counter offsets when available.
///
/// @param[in] clk_id:	unused hardware clock index.
/// @param[out] res:		timespec resolution result.
///
/// @return 0 on success.
/// @return -1 on error.
int clock_getres(clockid_t clk_id, struct timespec *res)
{
    res->tv_sec = 0;
    res->tv_nsec = TIMER1_COUNTER_RES;
    return(0);
}
