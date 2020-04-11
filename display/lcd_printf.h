/**
 @file lcd_printf.h

 @brief printf wrapper for SpartFun SerLcd

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



#ifndef _LCD_PRINTF_H_
#define _LCD_PRINTF_H

typedef struct
{
    int16_t xpos;       // x pos
    int16_t ypos;       // y pos
    int16_t w;
    int16_t h;
} window;

#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

// Weak attribute
#ifndef WEAK_ATR
#define WEAK_ATR __attribute__((weak))
#endif

#define SWAP(a, b) do { a ^= b; b ^= a; a ^= b; } while(0)
#define ABS(x) ((x)<0 ? -(x) : (x))

/* lcd_printf.c */
void lcd_POS ( uint8_t x , uint8_t y );
MEMSPACE void lcd_cleareol ( void );
MEMSPACE void lcd_clearline ( void );
void lcd_putch ( int c );
MEMSPACE int lcd_printf ( const char *fmt , ...);


#endif
