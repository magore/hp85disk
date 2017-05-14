/**
 @file gpib/ss80.h

 @brief SS80 disk emulator for HP85 disk emulator project for AVR8. 

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

 @par Based on work by Anders Gustafsson.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

*/


#ifndef SS80_H_
#define SS80_H_

#include "gpib/defines.h"

extern int Errors;
extern uint8_t qstat;

/* ss80.c */
void SS80_Test ( void );
DWORD SS80_MaxVector ( void );
void SS80_init ( void );
int SS80_Execute_State ( void );
int SS80_locate_and_read ( void );
int SS80_locate_and_write ( void );
int SS80_send_status ( void );
int SS80_describe ( void );
int SS80_Command_State ( void );
int SS80_Transparent_State ( void );
int SS80_cmd_seek ( void );
int SS80_Report ( void );
void Clear_Common ( int u );
int SS80_Channel_Independent_Clear ( int u );
int SS80_Universal_Device_Clear ( void );
int SS80_Selected_Device_Clear ( int u );
int SS80_Amigo_Clear ( void );
int SS80_Cancel ( int u );
int SS80_increment ( void );
int SS80_error_return ( void );
int SS80_COMMANDS ( uint8_t ch );



#endif
