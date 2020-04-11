#!/bin/bash
#
# Discription: Pure python implementation of STK500v1, made to work with Optiboot only.
# Author:      Mathieu Virbel <mat@meltingrocks.com> 
# Source:      https://github.com/tito/sk500
# References:  https://github.com/Optiboot/optiboot/wiki/HowOptibootWorks
#
# Updated for hp85disk project by Mike Gore 2020 and Jay Hamlin
#   * Original Source: https://github.com/tito/stk500/raw/master/hexuploader.py
#   * Changed to support atmega1284p
#   * Jay Hamlin updated support for Python 3
#   * Added Baudrate argument
#   * Added code to send "reset" command to hp85disk firmware to force it into optiboot
#   * Fixed intel 02 segment record calculation, was off by a factor of 16


usage()
{
   echo $@
   echo "Usage: $0 baudrate port hexfile"
    exit 1
}

BAUD="$1"
PORT="$2"
HEXFILE=$3

if [ -z "$BAUD" ]
then
   usage "Expected Baud Rate"
fi
BAUD=$(echo $1 | sed -e "s/UL//")

if [ -z "$PORT" ]
then
   usage "Expected PORT"
fi

if [ -z "$HEXFILE" ]
then
   usage "Expected HEX file name"
fi

if [ ! -f "$HEXFILE" ]
then
   usage "File name $HEXFILE not found"
fi

python3 flasher.py "$BAUD" "$PORT" "$HEXFILE"
