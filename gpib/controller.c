/**
 @file gpib/printer.c

 @brief GPIB Controller Mode code HP85 disk emulator project for AVR. 

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
*/

#include "user_config.h"

#include "defines.h"

#include "drives.h"

#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "printer.h"
#include "posix.h"
#include "delay.h"
#include "controller.h"

/// @brief  Controller Mode send ASCII string
/// Stops sending with EOI
/// @param[in] from:    GPIB talker
/// @param[in] to:      GPIB listener
/// @param[in] str: string to send
/// @param[in] len: number of bytes to send (if 0 then length of string)
/// @return  number of bytes sent
int controller_send_str(uint8_t from, uint8_t to, char *str, int len)
{
    uint16_t status = 0;
    int size;
    if(len == 0)
        len = strlen((char *)str);

    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten

    gpib_write_byte(0x40 | from | ATN_FLAG);// GPIB talker
    gpib_write_byte(0x20 | to | ATN_FLAG);  // GPIB listener

    status = EOI_FLAG;
    size = gpib_write_str((uint8_t *)str, len, &status);

    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten
    return(size);
}

/// @brief  Controller Mode read ASCII string
/// Stops reading at EOI
/// @param[in] from:    GPIB talker
/// @param[in] to:      GPIB listener
/// @param[in] str: string to read
/// @param[in] len: maximum number of bytes to read
/// @return  number of bytes read
int controller_read_str(uint8_t from, uint8_t to, char *str, int len)
{
    uint16_t status;
    int size;

    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten

    gpib_write_byte(0x40 | from | ATN_FLAG);    // GPIB talker
    gpib_write_byte(0x20 | to | ATN_FLAG);      // GPIB listener

    status = EOI_FLAG;
    size = gpib_read_str((uint8_t *)str,len, &status);
    if(size > 0)
    {
        if(size < len)
            str[size] = 0;
        else
            str[len-1] = 0;
    }

    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten
    return(size);
}

/// @brief  Controller Mode TRACE read for debugging
/// Stops reading at EOI
/// @param[in] from:    GPIB talker
/// @param[in] to:      GPIB listener
/// @return  number of bytes read
int controller_read_trace(uint8_t from, uint8_t to)
{
    uint16_t ch;
    long len =0;

    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten

    gpib_write_byte(0x40 | from | ATN_FLAG);    // GPIB talker
    gpib_write_byte(0x20 | to | ATN_FLAG);      // GPIB listener

    while(1) // loop until EOI or user ABORT
    {
        if(uart_keyhit(0))
            break;
        ch = gpib_read_byte(0);
        gpib_decode(ch);
       if(ch & EOI_FLAG)
            break;
        ++len;
    }

    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten
    return(len);
}

void controller_ifc()
{
    GPIB_IO_LOW(IFC);
    delayms(200);
    GPIB_PIN_FLOAT(IFC);
    delayms(200);
    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten
}

