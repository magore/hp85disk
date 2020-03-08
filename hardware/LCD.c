/*
 * LCD library for SparkFun RGB 3.3v Serial Open LCD display
 * with an attached Qwiic adapter.
 *
 * By: Gaston R. Williams
 * Date: August 22, 2018
 *
 * License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 *
 * This library is based heavily on the LiquidCrystal_I2C library and the sample code provided with
 * the SparkFun Serial OpenLCD display.  The original LiquidCrystal library by David A. Mellis and
 * modified by Limor Fried and the OpenLCD code by Nathan Seidle at SparkFun.
 *
 * The LiquidCrystal_I2C library was based on the work by DFRobot.
 * (That's the only attribution I found in the code I have. If anyone can provide better information,
 * Plese let me know and I'll be happy to give credit where credit is due.)
 *
 * Original information copied from OpenLCD:
 *
 * The OpenLCD display information is based based on code by
 * Nathan Seidle
 * SparkFun Electronics
 * Date: April 19th, 2015
 *
 * License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 * OpenLCD gives the user multiple interfaces (serial, I2C, and SPI) to control an LCD. LCD was the original
 * serial LCD from SparkFun that ran on the PIC 16F88 with only a serial interface and limited feature set.
 * This is an updated serial LCD.
 *
 * Please Note: 0x72 is the 7-bit I2C address. If you are using a different language than Arduino you will probably
 * need to add the Read/Write bit to the end of the address. This means the default read address for the OpenLCD
 * is 0b.1110.0101 or 0xE5 and the write address is 0b.1110.0100 or 0xE4.
 * For more information see https://learn.sparkfun.com/tutorials/i2c_
 * Note: This code expects the display to be listening at the default I2C address. If your display is not at 0x72, you can
 * do a hardware reset. Tie the RX pin to ground and power up OpenLCD. You should see the splash screen
 * then "System reset Power cycle me" and the backlight will begin to blink. Now power down OpenLCD and remove
 * the RX/GND jumper. OpenLCD is now reset.
 *
 * To get this code to work, attach a Qwiic adapter to an OpenLCD. Use the Qwiic cable to attach adapter to a SparkFun Blackboard or
 * an Arduino Uno with the Qwiic shield.
 *
 * The OpenLCD has 4.7k pull up resistors on SDA and SCL. If you have other devices on the
 * I2C bus then you may want to disable the pull up resistors by clearing the PU (pull up) jumper.

 * OpenLCD will work at 400kHz Fast I2C. Use the .setClock() call shown below to set the data rate
 * faster if needed.
 * Command cheat sheet:
 * ASCII / DEC / HEX
 * '|'    / 124 / 0x7C - Put into setting mode
 * Ctrl+c / 3 / 0x03 - Change width to 20
 * Ctrl+d / 4 / 0x04 - Change width to 16
 * Ctrl+e / 5 / 0x05 - Change lines to 4
 * Ctrl+f / 6 / 0x06 - Change lines to 2
 * Ctrl+g / 7 / 0x07 - Change lines to 1
 * Ctrl+h / 8 / 0x08 - Software reset of the system
 * Ctrl+i / 9 / 0x09 - Enable/disable splash screen
 * Ctrl+j / 10 / 0x0A - Save currently displayed text as splash
 * Ctrl+k / 11 / 0x0B - Change baud to 2400bps
 * Ctrl+l / 12 / 0x0C - Change baud to 4800bps
 * Ctrl+m / 13 / 0x0D - Change baud to 9600bps
 * Ctrl+n / 14 / 0x0E - Change baud to 14400bps
 * Ctrl+o / 15 / 0x0F - Change baud to 19200bps
 * Ctrl+p / 16 / 0x10 - Change baud to 38400bps
 * Ctrl+q / 17 / 0x11 - Change baud to 57600bps
 * Ctrl+r / 18 / 0x12 - Change baud to 115200bps
 * Ctrl+s / 19 / 0x13 - Change baud to 230400bps
 * Ctrl+t / 20 / 0x14 - Change baud to 460800bps
 * Ctrl+u / 21 / 0x15 - Change baud to 921600bps
 * Ctrl+v / 22 / 0x16 - Change baud to 1000000bps
 * Ctrl+w / 23 / 0x17 - Change baud to 1200bps
 * Ctrl+x / 24 / 0x18 - Change the contrast. Follow Ctrl+x with number 0 to 255. 120 is default.
 * Ctrl+y / 25 / 0x19 - Change the TWI address. Follow Ctrl+x with number 0 to 255. 114 (0x72) is default.
 * Ctrl+z / 26 / 0x1A - Enable/disable ignore RX pin on startup (ignore emergency reset)
 * '+'    / 43 / 0x2B - Set RGB backlight with three following bytes, 0-255
 * ','    / 44 / 0x2C - Display current firmware version
 * '-'    / 45 / 0x2D - Clear display. Move cursor to home position.
 * '.'    / 46 / 0x2E - Enable system messages (ie, display 'Contrast: 5' when changed)
 * '/'    / 47 / 0x2F - Disable system messages (ie, don't display 'Contrast: 5' when changed)
 * '0'    / 48 / 0x30 - Enable splash screen
 * '1'    / 49 / 0x31 - Disable splash screen
 *        / 128-157 / 0x80-0x9D - Set the primary backlight brightness. 128 = Off, 157 = 100%.
 *        / 158-187 / 0x9E-0xBB - Set the green backlight brightness. 158 = Off, 187 = 100%.
 *        / 188-217 / 0xBC-0xD9 - Set the blue backlight brightness. 188 = Off, 217 = 100%.
 *		For example, to change the baud rate to 115200 send 124 followed by 18.
 *
 */
#include "user_config.h"

#include "LCD.h"

static byte _displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
static byte _displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

static byte LCD_error = 0;

// ===============================================
///@brief I2C HAL
/// Interface with the project TWI code
void i2c_Start()
{
	if(LCD_error)
		return;
    LCD_error = TWI_StartTransmission(DISPLAY_ADDRESS1<<1, 20);
	delayms(10); //wait a bit for display to enable
} //i2c_Start

/*
 * Send data to the device
 *
 * data - byte to send
 */
void i2c_Transmit(uint8_t data)
{
	if(LCD_error)
		return;
	TWI_SendByte(data);
} //i2c_Transmit

/*
 * Begin transmission to the device
 */
void i2c_End()
{
	TWI_StopTransmission();
	delayms(10); //wait a bit for display to disable
} //i2c_Start
// ===============================================
/*
 * Initialize the display
 *
 */
int LCD_init()
{
  
  LCD_error = TWI_StartTransmission(DISPLAY_ADDRESS1<<1, 20);
  if(LCD_error)
	return(LCD_error);

  TWI_SendByte(SPECIAL_COMMAND);                      //Send special command character
  TWI_SendByte(LCD_DISPLAYCONTROL | _displayControl); //Send the display command
  TWI_SendByte(SPECIAL_COMMAND);                      //Send special command character
  TWI_SendByte(LCD_ENTRYMODESET | _displayMode);      //Send the entry mode command
  TWI_SendByte(SETTING_COMMAND);                      //Put LCD into setting mode
  TWI_SendByte(CLEAR_COMMAND);                        //Send clear display command
  TWI_StopTransmission();

  delayms(50);                                      //let things settle a bit
} //init
                                                   
// ===============================================

/*
  * Send a command to the display.
  * Used by other functions.
  *
  * byte command to send
  */
void LCD_command(byte command)
{
  i2c_Start();       // i2c_Transmit to device
  i2c_Transmit(SETTING_COMMAND); //Put LCD into setting mode
  i2c_Transmit(command);         //Send the command code
  i2c_End();         //Stop transmission

  delayms(10); //Hang out for a bit
}

/*
 * Send a special command to the display.  Used by other functions.
 *
 * byte command to send
 */
void LCD_specialCommand(byte command)
{
  i2c_Start();       // i2c_Transmit to device
  i2c_Transmit(SPECIAL_COMMAND); //Send special command character
  i2c_Transmit(command);         //Send the command code
  i2c_End();         //Stop transmission

  delayms(50); //Wait a bit longer for special display commands
}

/*
 * Send multiple special commands to the display.
 * Used by scrolling and cursor movement functions only.
 *
 * byte command to send
 * byte count number of times to send
 */
void LCD_specialCommandCount(byte command, byte count)
{
  i2c_Start(); // i2c_Transmit to device

  for (int i = 0; i < count; i++)
  {
    i2c_Transmit(SPECIAL_COMMAND); //Send special command character
    i2c_Transmit(command);         //Send command code
  }                            // for
  i2c_End();           //Stop transmission

  delayms(50); //Wait a bit longer for special display commands
}

/*
 * Send the clear command to the display.  This clears the
 * display and forces the cursor to return to the beginning
 * of the display.
 */
void LCD_clear()
{
  LCD_command(CLEAR_COMMAND);
  delayms(10); // a little extra delay after clear
}

/*
 * Send the home command to the display.  This returns the cursor
 * to return to the beginning of the display, without clearing
 * the display.
 */
void LCD_home()
{
  LCD_specialCommand(LCD_RETURNHOME);
}


/*
 * Set the cursor position to a particular column and row.
 *
 * column - byte 0 to 19
 * row - byte 0 to 3
 *
 * returns: boolean true if cursor set.
 */
void LCD_setCursor(byte col, byte row)
{
	int row_offsets[] = {0x00, 0x40, 0x14, 0x54};

  //keep variables in bounds
  // row is unsigned - no need to test < 0
	if(row > (MAX_ROWS-1) )
		row = MAX_ROWS-1;

  //send the command
  LCD_specialCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
} // setCursor

/*
 * Create a customer character
 * byte   location - character number 0 to 7
 * byte[] charmap  - byte array for character
 */
void LCD_createChar(byte location, byte charmap[])
{
  location &= 0x7; // we only have 8 locations 0-7
  i2c_Start();
  //Send request to create a customer character
  i2c_Transmit(SETTING_COMMAND); //Put LCD into setting mode
  i2c_Transmit(27 + location);
  for (int i = 0; i < 8; i++)
  {
    i2c_Transmit(charmap[i]);
  } // for
  i2c_End();
  delayms(50); //This takes a bit longer
}

/*
 * Write a customer character to the display
 *
 * byte location - character number 0 to 7
 */
void LCD_writeChar(byte location)
{
  location &= 0x7; // we only have 8 locations 0-7
  LCD_command(35 + location);
}

/*
 * Write a byte to the display.
 * Required for Print.
 */
uint8_t LCD_putb(uint8_t b)
{
  i2c_Start(); // i2c_Transmit to device
  i2c_Transmit(b);
  i2c_End(); //Stop transmission
  delayms(10);         // wait a bit
  return(b);
} // write

/*
 * Write a character buffer to the display.
 * Required for Print.
 */
size_t LCD_write(const uint8_t *buffer, size_t size)
{
  size_t n = 0;
  if(LCD_error)
	return;
  i2c_Start(); // i2c_Transmit to device
  while (size--)
  {
    i2c_Transmit(*buffer++);
    n++;
  }                  //while
  i2c_End(); //Stop transmission
  delayms(10);         //
  return n;
} //write

/*
 * Write a character buffer to the display.
 * Required for Print.
 */
void LCD_puts(const uint8_t *buffer)
{
  LCD_write(buffer,strlen(buffer));
}

/*
  * Turn the display off quickly.
  */
void LCD_noDisplay()
{
  _displayControl &= ~LCD_DISPLAYON;
  LCD_specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // noDisplay

/*
 * Turn the display on quickly.
 */
void LCD_display()
{
  _displayControl |= LCD_DISPLAYON;
  LCD_specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // display

/*
  * Turn the underline cursor off.
  */
void LCD_noCursor()
{
  _displayControl &= ~LCD_CURSORON;
  LCD_specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // noCursor

/*
 * Turn the underline cursor on.
 */
void LCD_cursor()
{
  _displayControl |= LCD_CURSORON;
  LCD_specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // cursor

/*
  * Turn the blink cursor off.
  */
void LCD_noBlink()
{
  _displayControl &= ~LCD_BLINKON;
  LCD_specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // noBlink

/*
 * Turn the blink cursor on.
 */
void LCD_blink()
{
  _displayControl |= LCD_BLINKON;
  LCD_specialCommand(LCD_DISPLAYCONTROL | _displayControl);
} // blink

/*
 * Scroll the display multiple characters to the left, without
 * changing the text
 *
 * count byte - number of characters to scroll
 */
void LCD_scrollDisplayLeftCount(byte count)
{
  LCD_specialCommandCount(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, count);
} // scrollDisplayLeft

/*
 * Scroll the display one character to the left, without
 * changing the text
 */
void LCD_scrollDisplayLeft()
{
	LCD_scrollDisplayLeftCount(1);
} // scrollDisplayLeft

/*
 * Scroll the display multiple characters to the right, without
 * changing the text
 *
 * count byte - number of characters to scroll
 */
void LCD_scrollDisplayRightCOunt(byte count)
{
  LCD_specialCommandCount(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, count);
} // scrollDisplayRight

/*
 * Scroll the display one character to the right, without
 * changing the text
 */
void LCD_scrollDisplayRight()
{
	LCD_scrollDisplayRightCOunt(1);
} // scrollDisplayRight

/*
 *  Move the cursor multiple characters to the left.
 *
 *  count byte - number of characters to move
 */
void LCD_moveCursorLeftCount(byte count)
{
  LCD_specialCommandCount(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVELEFT, count);
} // moveCursorLeft


/*
 *  Move the cursor one character to the left.
 */
void LCD_moveCursorLeft()
{
	LCD_moveCursorLeftCount(1);
} // moveCursorLeft

/*
 *  Move the cursor multiple characters to the right.
 *
 *  count byte - number of characters to move
 */
void LCD_moveCursorRightCount(byte count)
{
  LCD_specialCommandCount(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVERIGHT, count);
} // moveCursorRight

/*
 *  Move the cursor one character to the right.
 */
void LCD_moveCursorRight()
{
	LCD_moveCursorRightCount(1);
} // moveCursorRight


//New command - set backlight with LCD messages or delaymss
void LCD_setFastBacklightRGB(byte r, byte g, byte b)
{
  //send commands to the display to set backlights
  i2c_Start();       // i2c_Transmit to device
  i2c_Transmit(SETTING_COMMAND); //Send special command character
  i2c_Transmit(SET_RGB_COMMAND); //Send the set RGB character '+' or plus
  i2c_Transmit(r);               //Send the red value
  i2c_Transmit(g);               //Send the green value
  i2c_Transmit(b);               //Send the blue value
  i2c_End();         //Stop transmission
  delayms(10);
} // setFastBacklight

// New backlight function
void LCD_setFastBacklight(unsigned long rgb)
{
  // convert from hex triplet to byte values
  byte r = (rgb >> 16) & 0x0000FF;
  byte g = (rgb >> 8) & 0x0000FF;
  byte b = rgb & 0x0000FF;
  LCD_setFastBacklightRGB(r, g, b);
}

//Enable system messages
//This allows user to see printing messages like 'UART: 57600' and 'Contrast: 5'
void LCD_enableSystemMessages()
{
  i2c_Start();                     // i2c_Transmit to device
  i2c_Transmit(SETTING_COMMAND);               //Send special command character
  i2c_Transmit(ENABLE_SYSTEM_MESSAGE_DISPLAY); //Send the set '.' character
  i2c_End();                       //Stop transmission
  delayms(10);
}

//Disable system messages
//This allows user to disable printing messages like 'UART: 57600' and 'Contrast: 5'
void LCD_disableSystemMessages()
{
  i2c_Start();                      // i2c_Transmit to device
  i2c_Transmit(SETTING_COMMAND);                //Send special command character
  i2c_Transmit(DISABLE_SYSTEM_MESSAGE_DISPLAY); //Send the set '.' character
  i2c_End();                        //Stop transmission
  delayms(10);
}

//Enable splash screen at power on
void LCD_enableSplash()
{
  i2c_Start();             // i2c_Transmit to device
  i2c_Transmit(SETTING_COMMAND);       //Send special command character
  i2c_Transmit(ENABLE_SPLASH_DISPLAY); //Send the set '.' character
  i2c_End();               //Stop transmission
  delayms(10);
}

//Disable splash screen at power on
void LCD_disableSplash()
{
  i2c_Start();              // i2c_Transmit to device
  i2c_Transmit(SETTING_COMMAND);        //Send special command character
  i2c_Transmit(DISABLE_SPLASH_DISPLAY); //Send the set '.' character
  i2c_End();                //Stop transmission
  delayms(10);
}

//Save the current display as the splash
void LCD_saveSplash()
{
  //Save whatever is currently being displayed into EEPROM
  //This will be displayed at next power on as the splash screen
  i2c_Start();                      // i2c_Transmit to device
  i2c_Transmit(SETTING_COMMAND);                //Send special command character
  i2c_Transmit(SAVE_CURRENT_DISPLAY_AS_SPLASH); //Send the set Ctrl+j character
  i2c_End();                        //Stop transmission
  delayms(10);
}

/*
 * Set the text to flow from left to right.  This is the direction
 * that is common to most Western languages.
 */
void LCD_leftToRight()
{
  _displayMode |= LCD_ENTRYLEFT;
  LCD_specialCommand(LCD_ENTRYMODESET | _displayMode);
} // leftToRight

/*
 * Set the text to flow from right to left.
 */
void LCD_rightToLeft()
{
  _displayMode &= ~LCD_ENTRYLEFT;
  LCD_specialCommand(LCD_ENTRYMODESET | _displayMode);
} //rightToLeft

/*
 * Turn autoscrolling on. This will 'right justify' text from
 * the cursor.
 */
void LCD_autoscroll()
{
  _displayMode |= LCD_ENTRYSHIFTINCREMENT;
  LCD_specialCommand(LCD_ENTRYMODESET | _displayMode);
} //autoscroll

/*
 * Turn autoscrolling off.
 */
void LCD_noAutoscroll()
{
  _displayMode &= ~LCD_ENTRYSHIFTINCREMENT;
  LCD_specialCommand(LCD_ENTRYMODESET | _displayMode);
} //noAutoscroll

/*
 * Change the contrast from 0 to 255. 120 is default.
 *
 * byte new_val - new contrast value
 */
void LCD_setContrast(byte new_val)
{
  //send commands to the display to set backlights
  i2c_Start();        // i2c_Transmit to device
  i2c_Transmit(SETTING_COMMAND);  //Send contrast command
  i2c_Transmit(CONTRAST_COMMAND); //0x18
  i2c_Transmit(new_val);          //Send new contrast value
  i2c_End();          //Stop transmission

  delayms(10); //Wait a little bit
} //setContrast

/*
 * Change the I2C Address. 0x72 is the default.
 * Note that this change is persistent.  If anything
 * goes wrong you may need to do a hardware reset
 * to unbrick the display.
 *
 * byte new_addr - new i2c_ address
 */
void LCD_setAddress(byte new_addr)
{
  //send commands to the display to set backlights
  i2c_Start();       // i2c_Transmit to device on old address
  i2c_Transmit(SETTING_COMMAND); //Send contrast command
  i2c_Transmit(ADDRESS_COMMAND); //0x19
  i2c_Transmit(new_addr);        //Send new contrast value
  i2c_End();         //Stop transmission

  //Update our own address so we can still talk to the display
  // _i2c_Addr = new_addr;

  delayms(50); //This may take awhile
} //setContrast
