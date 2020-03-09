#ifndef QWIIC_SER_LCD_H
#define QWIIC_SER_LCD_H

typedef uint8_t byte;

#define DISPLAY_ADDRESS1 0x72 //This is the default address of the OpenLCD
#define MAX_ROWS 4
#define MAX_COLUMNS 20

//OpenLCD command characters
#define SPECIAL_COMMAND 254  //Magic number for sending a special command
#define SETTING_COMMAND 0x7C //124, |, the pipe character: The command to change settings: baud, lines, width, backlight, splash, etc

//OpenLCD commands
#define CLEAR_COMMAND 0x2D					//45, -, the dash character: command to clear and home the display
#define CONTRAST_COMMAND 0x18				//Command to change the contrast setting
#define ADDRESS_COMMAND 0x19				//Command to change the i2c address
#define SET_RGB_COMMAND 0x2B				//43, +, the plus character: command to set backlight RGB value
#define ENABLE_SYSTEM_MESSAGE_DISPLAY 0x2E  //46, ., command to enable system messages being displayed
#define DISABLE_SYSTEM_MESSAGE_DISPLAY 0x2F //47, /, command to disable system messages being displayed
#define ENABLE_SPLASH_DISPLAY 0x30			//48, 0, command to enable splash screen at power on
#define DISABLE_SPLASH_DISPLAY 0x31			//49, 1, command to disable splash screen at power on
#define SAVE_CURRENT_DISPLAY_AS_SPLASH 0x0A //10, Ctrl+j, command to save current text on display as splash

// special commands
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

/* ../hardware/LCD.c */

#define LCD_ADDR (DISPLAY_ADDRESS1<<1)

bool I2C_Start ( uint8_t addr );
void I2C_Stop ( void );
bool I2C_Send ( uint8_t data );
bool I2C_SendMessage ( uint8_t addr , const uint8_t *buffer , size_t size );
bool LCD_putb ( uint8_t b );
bool LCD_write ( const uint8_t *buffer , size_t size );
void LCD_puts ( const uint8_t *buffer );
void LCD_pos ( uint8_t x , uint8_t y );
bool LCD_init ( uint8_t addr );


void LCD_command ( byte command );
void LCD_specialCommand ( byte command );
void LCD_specialCommandCount ( byte command , byte count );
void LCD_clear ( void );
void LCD_home ( void );
void LCD_setCursor ( byte col , byte row );
void LCD_createChar ( byte location , byte charmap []);
void LCD_writeChar ( byte location );
void LCD_noDisplay ( void );
void LCD_display ( void );
void LCD_noCursor ( void );
void LCD_cursor ( void );
void LCD_noBlink ( void );
void LCD_blink ( void );
void LCD_scrollDisplayLeftCount ( byte count );
void LCD_scrollDisplayLeft ( void );
void LCD_scrollDisplayRightCOunt ( byte count );
void LCD_scrollDisplayRight ( void );
void LCD_moveCursorLeftCount ( byte count );
void LCD_moveCursorLeft ( void );
void LCD_moveCursorRightCount ( byte count );
void LCD_moveCursorRight ( void );
void LCD_setFastBacklightRGB ( byte r , byte g , byte b );
void LCD_setFastBacklight ( unsigned long rgb );
void LCD_enableSystemMessages ( void );
void LCD_disableSystemMessages ( void );
void LCD_enableSplash ( void );
void LCD_disableSplash ( void );
void LCD_saveSplash ( void );
void LCD_leftToRight ( void );
void LCD_rightToLeft ( void );
void LCD_autoscroll ( void );
void LCD_noAutoscroll ( void );
void LCD_setContrast ( byte new_val );
void LCD_setAddress ( byte new_addr );



#endif
