/**
 @file gpib/gpib_task.h

 @brief High level GPIB command handler for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

*/


#ifndef _GPIB_TASK_H_
#define _GPIB_TASK_H_

#include <user_config.h>
#include "drives.h"

/* gpib_task.c */
void gpib_file_init ( void );
void gpib_log ( char *str );
int SS80_is_MLA ( int address );
int SS80_is_MTA ( int address );
int SS80_is_MSA ( int address );
int AMIGO_is_MLA ( int address );
int AMIGO_is_MTA ( int address );
int AMIGO_is_MSA ( int address );
int PRINTER_is_MLA ( int address );
int PRINTER_is_MTA ( int address );
int PRINTER_is_MSA ( int address );
uint16_t gpib_trace_read_byte ( void );
void gpib_trace_task ( char *name , int detail );
uint16_t gpib_error_test ( uint16_t val );
void gpib_init_devices ( void );
uint16_t GPIB_COMMANDS ( uint16_t val , uint8_t unread );
void gpib_task ( void );
int Send_Identify ( uint8_t ch , uint16_t ID );
int GPIB ( uint8_t ch );
int GPIB_LISTEN ( uint8_t ch );
int GPIB_TALK ( uint8_t ch );
int GPIB_SECONDARY_ADDRESS ( uint8_t ch );
void listen_cleanup ( void );
void talk_cleanup ( void );
void DumpData ( unsigned char *ptr , int length );

#endif  // #ifndef _GPIB_TASK_H_
