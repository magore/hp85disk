/**
 @file lib/timer_hal.h

 @brief High resolution timer library and user tasks hardware specific code for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _TIMER_HAL_H_
#define _TIMER_HAL_H_

#include <hardware/hardware.h>

void setup_timers_isr ( void );
int clock_gettime ( clockid_t clk_id , struct timespec *ts );
int clock_getres ( clockid_t clk_id , struct timespec *res );
#endif                                            // _TIMER_HAL_H_
