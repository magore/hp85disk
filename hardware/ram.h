/**
 @file hardware/ram.h

 @brief Memory Utilities and safe free for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _RAM_H_
#define _RAM_H_
/* hardware/ram.c */
uint16_t freeRam ( void );
void PrintFree ( void );
void *safecalloc ( int size , int elements );
void *safemalloc ( size_t size );
void safefree ( void *p );

#endif                                            //_RAM_H_
