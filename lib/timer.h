/**
 @file lib/timer.h

 @brief High resolution timer library and user tasks for AVR8. 

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef __TIMER_H__
#define __TIMER_H__

#include <hardware/cpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/time.h>
#include <lib/timer_hal.h>

///@brief Number of user timer tasks
#define MAX_TIMER_CNT 8

///@brief user timer struct
typedef struct
{
    void (*user_timer_handler)(void);             // user task
    uint8_t timer;                                // user task enabled ?
} TIMERS;

///@brief System task in HZ.
#define SYSTEM_TASK_HZ 1000L
///@brief System task in Nanoseconds.
#define SYSTEM_TASK_TIC_NS ( 1000000000L / SYSTEM_TASK_HZ )
///@brief System task in Microseconds.
#define SYSTEM_TASK_TIC_US ( 1000000L / SYSTEM_TASK_HZ )

///@brief Clock task in HZ defined as System task.
#define CLOCK_HZ SYSTEM_TASK_HZ
///@brief Clock task in Nanoseconds defined as System Task.
#define CLOCK_TIC_NS SYSTEM_TASK_TIC_NS
///@brief CLock task in Microseconds defined as System Task.
#define CLOCK_TIC_US SYSTEM_TASK_TIC_US

/* timer.c */
void init_timers ( void );
void execute_timers ( void );
int set_timers ( void (*handler )(void ), int timer );
int kill_timers ( int timer );
void delete_all_timers ( void );
void subtract_timespec ( ts_t *a , ts_t *b );
char *ts_to_str ( ts_t *val );
void display_ts ( ts_t *val );
void clock_elapsed_begin ( void );
void clock_elapsed_end ( char *msg );
#endif                                            // _TIMER_H_
