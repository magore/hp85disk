/**
 @file gpib/amigo.h

 @brief AMIGO disk emulator for HP85 disk emulator project for AVR.

 @par Edit History - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
@see http://github.com/magore/hp85disk
@see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

@par Based on work by Anders Gustafsson.

@par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

*/

#ifndef _AMIGO_H_
#define _AMIGO_H_

#include "user_config.h"
#include "defines.h"

#define UNL     0x3F                              // Unlisten
#define UNT     0x5F                              // Untalk
#define LLO     0x11                              // Local lockout
#define DCL     0x14                              // Device clear
#define PPU     0x15                              // Parallell poll unconfigure
#define SPE     0x18                              // Serial poll enable
#define SPD     0x19                              // Serial poll disable
#define GTL     0x01                              // Go to local
#define SDC     0x04                              // Selected device clear
#define PPC     0x05                              // Parallell poll configure
#define GET     0x08                              // Group execute trigger
#define TCT     0x09                              // Take control

/* amigo.c */
void amigo_init ( void );
int amigo_request_logical_address ( void );
int amigo_request_status ( void );
int amigo_send_logical_address ( void );
int amigo_send_status ( void );
int amigo_increment ( char *msg );
int amigo_seek ( AMIGOStateType *p );
int amigo_verify ( uint16_t sectors );
int amigo_format ( uint8_t db );
int amigo_buffered_read ( void );
int amigo_buffered_write ( void );
int amigo_cmd_dsj ( void );
int amigo_cmd_wakeup ( void );
int amigo_cmd_clear ( void );
int amigo_todo_op ( uint8_t secondary , uint8_t opcode , int len );
int amigo_todo ( uint8_t secondary );
void amigo_check_unit( uint8_t unit );
int Amigo_Command ( int secondary );
int Amigo_Execute ( int secondary );
int AMIGO_COMMANDS ( uint8_t ch );
#endif                                            // #ifndef _AMIGO_H_
