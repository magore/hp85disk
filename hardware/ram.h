/**
 @file hardware/ram.h

 @brief Memory Utilities and safe free for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, Inc. All rights reserved.

*/

#ifndef _RAM_H_
#define _RAM_H_
#undef malloc
#undef calloc
#undef free

/* utils/sys.c */
MEMSPACE void *malloc ( size_t size );
MEMSPACE void *calloc ( size_t nmemb , size_t size );
MEMSPACE void free ( void *p );
/* hardware/ram.c */
uint16_t freeRam ( void );
void PrintFree ( void );
void *safecalloc ( int size , int elements );
void *safemalloc ( size_t size );
void safefree ( void *p );
#endif                                            //_RAM_H_
