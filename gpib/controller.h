/**
 @file gpib/printer.h

 @brief GPIB Controller mode code for HP85 disk emulator project.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

*/

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <user_config.h>

/* controller.c */
int controller_send_str ( uint8_t from , uint8_t to , char *str , int len );
int controller_read_str ( uint8_t from , uint8_t to , char *str , int len );
int controller_read_trace ( uint8_t from , uint8_t to );
void controller_ifc ( void );

#endif  // #ifndef _CONTROLLER_H
