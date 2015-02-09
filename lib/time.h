/**
 @file lib/time.h

 @brief POSIX time libraries for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef TIME_H
#define TIME_H

#include <hardware/cpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPOCH_YEAR    1970    /*< Thursday Jan 1 1970 */
#define EPOCH_DAY        4    /*< Sunday = 0 ... Saturday = 6 */
#define EPOCH            0    /*< Zero seconds */
#define EPOCH_2000 946684800  /*< Sat, 01 Jan 2000 00:00:00 GMT */

///@brief type of EPOCH result.
typedef uint32_t time_t;

///@brief type of clockid_t.
typedef uint16_t clockid_t;

///@brief POSIX struct tm.
struct tm
{
    int tm_sec;    /*<  Seconds.     [0-60] (1 leap second) */
    int tm_min;    /*<  Minutes.     [0-59] */
    int tm_hour;   /*<  Hours.       [0-23] */
    int tm_mday;   /*<  Day.         [1-31] */
    int tm_mon;    /*<  Month.       [0-11] */
    int tm_year;   /*<  Year - 1900. */
    int tm_wday;   /*<  Day of week. [0-6] */
    int tm_yday;   /*<  Days in year.[0-365] */
    int tm_isdst;  /*<  DST.         [-1/0/1] */
    int32_t tm_gmtoff; /*<  GMT offset in seconds */
};

///@brief POSIX struct tm typedef.
typedef struct tm tm_t;

///@brief POSIX timeval.
struct timeval
{
    time_t       tv_sec;   /*< seconds */
    uint32_t     tv_usec;  /*< microseconds */
};
///@brief POSIX timeval typedef.
typedef struct timeval tv_t;

///@brief POSIX timezone.
struct timezone
{
    int tz_minuteswest;  /*< minutes west of Greenwich */
    int tz_dsttime;      /*< type of DST correction */
};
///@brief POSIX timezone typedef.
typedef struct timezone tz_t;

///@brief POSIX timespec.
struct timespec
{
    time_t   tv_sec;   /*< seconds */
    long     tv_nsec;  /*< nanoseconds */
};
///@brief POSIX timespec typedef.
typedef struct timespec ts_t;

/// @brief  System Clock Time
extern volatile ts_t __clock;

/// @brief  System Time Zone
extern tz_t __tzone;

/* time.c */
void clock_task ( void );
int clock_gettime ( clockid_t clk_id , struct timespec *ts );
int clock_getres ( clockid_t clk_id , struct timespec *res );
void clock_clear ( void );
char *tm_wday_to_ascii ( int i );
char *tm_mon_to_ascii ( int i );
int time_to_tm ( time_t epoch , int32_t offset , tm_t *t );
time_t timegm ( tm_t *t );
char *asctime_r ( tm_t *t , char *buf );
char *asctime ( tm_t *t );
char *ctime_r ( time_t *t , char *buf );
char *ctime ( time_t *tp );
char *ctime_gm ( time_t *tp );
tm_t *gmtime_r ( time_t *tp , tm_t *result );
tm_t *gmtime ( time_t *tp );
tm_t *localtime_r ( time_t *t , tm_t *result );
tm_t *localtime ( time_t *tp );
time_t mktime ( tm_t *t );
int clock_settime ( clockid_t clk_id , const struct timespec *tp );
int gettimezone ( tz_t *tz );
int settimezone ( tz_t *tz );
int gettimeofday ( tv_t *tv , tz_t *tz );
time_t time ( time_t *t );
int settimeofday ( tv_t *tv , tz_t *tz );
void clock_set ( uint32_t seconds , uint32_t us );
int setdate ( void );
int setdate_r ( char *buf );

#endif
