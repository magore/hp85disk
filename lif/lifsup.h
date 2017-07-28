/**
 @file lifsup.h

 @brief  LIF file utilities - utilities extracted from hp85disk project for stand alone use

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
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

#endif

/* lifsup.c */
int MATCHARGS ( char *str , char *pat , int min , int argc );
MEMSPACE void trim_tail ( char *str );
MEMSPACE void V2B_MSB ( uint8_t *B , int index , int size , uint32_t val );
MEMSPACE void V2B_LSB ( uint8_t *B , int index , int size , uint32_t val );
MEMSPACE uint32_t B2V_MSB ( uint8_t *B , int index , int size );
MEMSPACE uint32_t B2V_LSB ( uint8_t *B , int index , int size );
MEMSPACE void hexdump(uint8_t *data, int size);
