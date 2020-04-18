/**
 @file lifsup.h

 @brief  LIF file utilities - utilities extracted from hp85disk project for stand alone use

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

*/

#ifdef LIF_STAND_ALONE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <libgen.h>
#include <sys/types.h>
#include <utime.h>

#define MEMSPACE /**/
#define WEAK_ATR /**/
typedef struct tm tm_t;
#define safecalloc(a,b) calloc(a,b)
#define safefree(a) free(a)
#define sync() 

/* lifsup.c */
struct tm *gmtime_r ( const time_t *timep , struct tm *result );
char *asctime_r ( const struct tm *t , char *buf );
time_t timegm ( struct tm *a_tm );
void copyright ( void );
int main ( int argc , char *argv []);

#endif


