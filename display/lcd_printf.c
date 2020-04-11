/**
 @file lcd_printf.c

 @brief printf wrapper for SParFun SerLCD

 @par Copyright &copy; 2020 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
   please retain a copy of this notice in any code you use it in.

This is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option)
any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "user_config.h"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


window lcd = { 0,0,20,4 };

/// @brief  Clear display to end of line
/// return: void, lcd.x = lcd.w
MEMSPACE
void lcd_cleareol()
{
	int rem;
	rem = (lcd.w - 1 - lcd.xpos);
	while(rem > 0)
	{
		LCD_putb(' ');
		rem--;
	}
	LCD_pos(lcd.xpos = lcd.w-1,lcd.ypos);
}

/// @brief  Clear display to end of line
/// return: void, lcd.x = 0;
MEMSPACE
void lcd_clearline()
{
	int rem;
	rem = (lcd.w - 1 - lcd.xpos);
	while(rem > 0)
	{
		LCD_putb(' ');
		rem--;
	}
	LCD_pos(lcd.xpos = 0,lcd.ypos);
}

/// @brief  put character 
/// @param[in] c: character
/// return: void
void lcd_putch(int c)
{
	int rem;

	if(c < 0 || c > 0x7e)
		return;

	// Normal visible characters
	if(c >= ' ')
	{
		rem = (lcd.w - 1 - lcd.xpos);
		if( rem <= 0 )
		{
			LCD_pos(++lcd.ypos,lcd.xpos=0);
		}
		(void) LCD_putb(c);
		// uart_putchar(c,0);
		lcd.xpos++;
		return;
	}

	// Control characters
	if(c == '\n')
	{
		// uart_putchar(c,0);
		lcd_cleareol();
		++lcd.ypos;
		lcd.ypos &= 3;
		LCD_pos(lcd.xpos=0,lcd.ypos);
	}

	if(c == '\f')
	{
		LCD_clear();
		LCD_pos(lcd.xpos=0,lcd.ypos=0);
	}
}


// We do not use the printf structure
static void _putc_win(struct _printf_t *p, char ch)
{
	p->sent++;
	lcd_putch(ch);
}

/// @brief lcd_printf function
/// @param[in] fmt: printf forat string
/// @param[in] ...: vararg list or arguments
/// @return size of string
MEMSPACE
int lcd_printf(const char *fmt, ... )
{
    printf_t fn;

    fn.put = _putc_win;
    fn.sent = 0;

    va_list va;
    va_start(va, fmt);

    _printf_fn(&fn, fmt, va);

    va_end(va);

	return(fn.sent);

}


