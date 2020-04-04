## Discription: Pure python implementation of STK500v1, made to work with Optiboot only.
# Author:      Mathieu Virbel <mat@meltingrocks.com>
# Source:      https://github.com/tito/sk500
# References:  https://github.com/Optiboot/optiboot/wiki/HowOptibootWorks

## Updated for hp85disk project by Mike Gore 2020 and Jay Hamlin
   * Original Source: https://github.com/tito/stk500/raw/master/hexuploader.py
   * Updated for Python3  by Jay Hamlin
   * Changed to support atmega1284p
   * Added Baudrate argument
   * Added code to send "reset" command to hp85disk firmware to force it into optiboot
   * Fixed intel 02 segment record calculation, was off by a factor of 16

## Usage
  hexuploader.py Baudrate port hex_file
  Example: python flasher 115200 /dev/ttyUSB0 gpib.hex
