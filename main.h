/**
 @file main.h

 @brief Main for GPIB emulator for HP85 disk emulator project for AVR

 @par Edit History
 - [1.0]   [user name]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

/* main.c */
void uart_put ( uint8_t c );
char uart_get ( void );
void get_line ( char *buff , int len );
void help ( void );
void time_test ( void );
void task ( void );
