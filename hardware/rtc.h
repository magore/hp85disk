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

#define  DS1307 0x68
#define  DS1307_REG_SIZE 8

/* rtc.c */
uint8_t BINtoBCD ( uint8_t data );
uint8_t BCDtoBIN ( uint8_t data );
int8_t i2c_rtc_write ( uint8_t address , uint8_t ind , uint8_t *buf , uint8_t len );
int8_t i2c_rtc_read ( uint8_t address , uint8_t ind , uint8_t *buf , uint8_t len );
void i2c_rtc_init ( void );
uint8_t rtc_write ( tm_t *t );
uint8_t rtc_read ( tm_t *t );
int rtc_run ( int run );
int8_t rtc_run_test ( void );
uint8_t rtc_init ( int force , time_t seconds );
// uint32_t tm_to_fat ( tm_t *t );
// uint32_t get_fattime ( void );

#endif
