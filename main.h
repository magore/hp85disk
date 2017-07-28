/**
 @file main.h

 @brief Main for GPIB emulator for HP85 disk emulator project for AVR

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

 @par Edit History
 - [1.0]   [user name]  Initial revision of file.

*/

/* main.c */
void uart_put ( uint8_t c );
char uart_get ( void );
void get_line ( char *buff , int len );
void help ( void );
void time_test ( void );
void task ( void );
