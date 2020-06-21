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


extern char * __brkval;
extern char * __malloc_heap_start;
extern char * __malloc_heap_end;
extern size_t  __malloc_margin;
extern void *__stack;

#ifdef AVR
    extern unsigned int __heap_start;
    extern unsigned int __heap_end;
    extern unsigned int __bss_start;
    extern unsigned int __bss_end;
    extern unsigned int __data_start;
    extern unsigned int __data_end;
#endif
#ifdef ESP8266
    extern unsigned long int __heap_start;
    extern unsigned long int __heap_end;
    extern unsigned long int __bss_start;
    extern unsigned long int __bss_end;
    extern unsigned long int __data_start;
    extern unsigned long int __data_end;
#endif


/* ram.c */
size_t heaptop ( void );
size_t freeRam ( void );
void PrintFree ( void );
void *safecalloc ( int size , int elements );
void *safemalloc ( size_t size );
void safefree ( void *p );

#endif                                            //_RAM_H_
