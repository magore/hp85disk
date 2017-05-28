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

/* ss80.c */
void SS80_Test ( void );
void SS80_V2B ( uint8_t *B , int index , int size , uint32_t val );
uint8_t *SS80ControllerPack ( int *size );
uint8_t *SS80UnitPack ( int *size );
uint8_t *SS80VolumePack ( int *size );
void SS80_init ( void );
int SS80_Execute_State ( void );
uint32_t SS80_Blocks_to_Bytes ( uint32_t block );
uint32_t SS80_Bytes_to_Blocks ( uint32_t bytes );
int SS80_locate_and_read ( void );
int SS80_locate_and_write ( void );
int SS80_test_extended_status ( uint8_t *p , int bit );
void SS80_display_extended_status ( uint8_t *p , char *message );
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
