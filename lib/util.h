/**
 @file lib/util.h

 @brief Misc. pattern matching and string utilities for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _UTIL_H_
#define _UTIL_H_

/* util.c */
char *skipspaces ( char *ptr );
char *nextspace ( char *ptr );
char *skipchars ( char *str , char *pat );
void trim_tail ( char *str );
char *strnalloc ( char *str , int len );
char *stralloc ( char *str );
int token ( char *str , char *pat );
int MATCH ( char *str , char *pat );
int MATCHI ( char *str , char *pat );
int MATCH_LEN ( char *str , char *pat );
int MATCHI_LEN ( char *str , char *pat );
uint8_t hexd ( char c );
long atoh ( const char *p );
#endif                                            /* _UTIL_H */
