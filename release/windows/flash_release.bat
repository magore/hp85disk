@ECHO off

SET ISP=%1
SET PORT=%2
SET BAUD=%3


if NOT exist optiboot_atmega1284p.hex goto missing_optiboot
ECHO found optiboot_atmega1284p.hex 

if NOT exist gpib.hex goto missing_gpib
ECHO found gpib.hex 

if "%ISP%"=="" goto usage
if "%PORT%"=="" goto usage
if "%BAUD%"=="" goto usage

avrdude.exe -P %PORT% -b %BAUD% -p m1284 -c %ISP% -F -B 5 -e
avrdude.exe -P %PORT% -b %BAUD% -p m1284 -c %ISP% -F -B 5 -U flash:w:optiboot_atmega1284p.hex:i
avrdude.exe -P %PORT% -b %BAUD% -p m1284 -c %ISP% -u -F -B 5 -U lfuse:w:0xd6:m -U hfuse:w:0xda:m -U efuse:w:0xff:m
avrdude.exe -P %PORT% -b %BAUD% -p m1284 -c %ISP% -F -B 5 -U lock:w:0x3d:m
avrdude.exe -P %PORT% -b %BAUD% -p m1284 -c %ISP% -D -F -B 5 -U flash:w:gpib.hex:i
goto done

:usage
ECHO flash_release.bat ISP PORT BAUD
ECHO ISP  is the ISP programmer name as known by avrdude, examples avrisp,arduino
ECHO PORT is the ISP serial PORT, example COM3
ECHO BAUD is the ISP baud rate,   example 115200
goto done

:missing_optiboot
ECHO optiboot__atmega1284p.hex is missing
ECHO This file can be found on the github build\release folder
ECHO Place a copy in this folder and rerun the batch file
goto done

:missing_gpib
ECHO gpib.hex is missing
ECHO This file can be found on the github build\release folder
ECHO Place a copy in this folder and rerun the batch file

:done
