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
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <libgen.h>
#include <sys/types.h>
#include <utime.h>


#define MEMSPACE /**/
typedef struct tm tm_t;
#define MATCH(a,b) (strcmp(a,b) == 0 ? 1: 0)
#define safecalloc(a,b) calloc(a,b)
#define safefree(a) free(a)
#define sync() 
#include "lifutils.h"

#include "td02lif.h"


void copyright();

/* lifsup.c */
struct tm *gmtime_r ( const time_t *timep , struct tm *result );
char *asctime_r ( const struct tm *t , char *buf );
time_t timegm ( struct tm *a_tm );
int MATCHARGS ( char *str , char *pat , int min , int argc );
void trim_tail ( char *str );
void V2B_MSB ( uint8_t *B , int index , int size , uint32_t val );
void V2B_LSB ( uint8_t *B , int index , int size , uint32_t val );
uint32_t B2V_MSB ( uint8_t *B , int index , int size );
uint32_t B2V_LSB ( uint8_t *B , int index , int size );
void B2S ( uint8_t *B , int index , uint8_t *name , int size );
void BITSET_LSB ( uint8_t *p , int bit );
void BITCLR_LSB ( uint8_t *p , int bit );
int BITTST_LSB ( uint8_t *p , int bit );
uint16_t crc16 ( uint8_t *B , uint16_t crc , uint16_t poly , int size );
void hexdump ( uint8_t *data , int size );
void copyright ( void );



#endif


