/**
 @file gpib/gpib_task.h

 @brief High level GPIB command handler for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/


#ifndef _GPIB_TASK_H
#define _GPIB_TASK_H

#include <user_config.h>

/* gpib_task.c */
void gpib_file_init ( void );
void gpib_log ( char *str );
void gpib_trace_task ( char *name );
uint16_t gpib_error_test ( uint16_t val );
void gpib_init_devices ( void );
uint16_t GPIB_COMMANDS ( uint16_t val , uint8_t unread );
void gpib_task ( void );
void DumpData ( unsigned char *ptr , int length );
void FatFs_Read_Config ( char *name );
void POSIX_Read_Config ( char *name );
int Send_Identify ( uint8_t byte1 , uint8_t byte2 );
int GPIB ( uint8_t ch );
int GPIB_LISTEN ( uint8_t ch );
int GPIB_TALK ( uint8_t ch );
int GPIB_SECONDARY_ADDRESS ( uint8_t ch );
void listen_cleanup ( void );
void talk_cleanup ( void );

#endif
