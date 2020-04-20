/**
 @file hardware/delay.c

 @brief Extended delay routines for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, Inc. All rights reserved.

*/

#include "user_config.h"

// CPU Frequency
#ifndef F_CPU
#error F_CPU undefined
#endif

/// @brief  Delay microseconds using AVR acr-libc _delay_us() function.
///
///  - Depends on F_CPU and function _delay_us()
/// _delay_us() has limited range - this extends it.
/// @return  void
/// @see _delay_us() is part of avr-libc
void delayus(uint32_t us)
{

    while (us >= 100U)
    {
        us -= 100U;
        _delay_us(100);
    }
    while (us >= 10U)
    {
        us -= 10U;
        _delay_us(10);
    }
    while (us != 0U)
    {
        --us;
        _delay_us(1);
    }
}


/// @brief  Delay miliseconds using AVR acr-libc _delay_us() function.
///
///  - Depends on F_CPU and function _delay_us()
/// _delay_us() has limited range - this extends it.
/// @return  void
/// @see _delay_us() is part of avr-libc
void delayms(uint32_t ms)
{
    while (ms--)
    {
        _delay_us(100);                           // 100
        _delay_us(100);                           // 200
        _delay_us(100);                           // 300
        _delay_us(100);                           // 400
        _delay_us(100);                           // 500
        _delay_us(100);                           // 600
        _delay_us(100);                           // 700
        _delay_us(100);                           // 800
        _delay_us(100);                           // 900
        _delay_us(100);                           //1000
    }
}
