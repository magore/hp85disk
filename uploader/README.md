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

### Dependencies
  * python 3
    * Linux has this
      * pip3 install pySerial
    * Windows - Install Python 3.7 from Windows App Store
      * Open PowerSehll window
        * pip3 install pySerial

### Listing Serial ports
  * python3 listports.py
      * Under Windows use a PowerShell Window

### Uploading firmware
  * Examples:
    * python3 flasher.py 115200 /dev/ttyUSB0 gpib.hex
    * python3 flasher.py 115200 COM3 gpib.hex
      * Under Windows use a PowerShell Window

### Flash failure during flashing
  * Type in the following command, with your serial port, *without* pressing Enter
    * *python3 flasher.py 115200 /dev/ttyUSB0 gpib.hex*
    * Hold down RESET on the hp85disk board - release RESET and press Enter quickly
      * You have a short Window after releasing RESET to Press Enter

