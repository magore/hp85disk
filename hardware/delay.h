/**
 @file hardware/delay.h

 @brief Extended delay routines for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

#ifndef _DELAY_H
#define _DELAY_H

#include "user_config.h"

/* delay.c */
void delayus ( uint32_t us );
void delayms ( uint32_t ms );
#endif
