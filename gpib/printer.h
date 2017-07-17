/**
 @file gpib/printer.h

 @brief HPGL printer capture code for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

#ifndef _PRINTER_H
#define _PRINTER_H

#include <user_config.h>

/* printer.c */
void printer_open ( char *name );
void printer_init ( void );
void printer_close ( void );
int receive_plot_flush ( void );
void printer_buffer ( uint16_t val );
int PRINTER_COMMANDS ( uint8_t ch );
int controller_send_str ( uint8_t from , uint8_t to , uint8_t *str , int len );
int controller_read_str ( uint8_t from , uint8_t to , uint8_t *str , int len );
int controller_read_trace ( uint8_t from , uint8_t to );
void controller_ifc ( void );
void plot_echo ( int gpib_address );

#endif  // #ifndef _PRINTER_H
