/**
 @file hardware/rtc.h

 @brief DS1307 RTC Driver AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, Inc. All rights reserved.

*/

#ifndef _RTC_H_
#define _RTC_H_

#include "user_config.h"


int rtc_ok;

#define  DS1307_W                       0xd0
#define  DS1307_R                       0xd1

/* rtc.c */
uint8_t BINtoBCD ( uint8_t data );
uint8_t BCDtoBIN ( uint8_t data );
uint8_t rtc_init ( int force , time_t seconds);
uint8_t rtc_write ( struct tm *t );
uint8_t rtc_read ( struct tm *t );
uint32_t get_fattime ( void );
#endif
