\section README


# Documentation

 * Please use this link for the Documentation and this README
   * https://rawgit.com/magore/hp85disk/V2/doxygen/html/index.html
   * [index](doxygen/html/index.html)

## HP85 Disk Emulator Copyright (C) 2014-2020 Mike Gore 
 * New Board layout design by Jay Hamlin (C) 2018-2020
 * See [COPYRIGHT](COPYRIGHT.md) for a full copyright notice for the project

## HP85 disk emulator V2 circuit board layout design by (C) 2018-2020 Jay Hamlin
## V2 board design - use V2 branch targets the new board by Jay Hamlin
  * [Jay Hamlin designed this board](board/V2/releases)
   * GPIB BUS drivers
   * I2C level conveters
   * reset circuit
   * full size SD card interface
   * V2 code is now working

## HP85 disk emulator V1 board design (C) 2014-2020 Mike Gore
## [V1 board readme](board/V1/README.md)
  * [My original board design without GPIB buffers](board/V1/README.md)

## HP85 emulator board design Makefile configuration options V1 and V2 boards
## V1 Board version Makefile configuration
    * V2BOARD=1
     * Leave defined for all boards - now used only for debugging
  * V2 board Makefile configuration options 
    * PPR_REVERSE_BITS=1
  * V1 original board Makefile options
    * PPR_REVERSE_BITS=0

___ 


## Features
 * This project emulates GPIB drives and HPGL printer for the HP85A and HP85B computers.
   * Each drive can be fully defined in the hpdisk.cfg file on the SD CARD
   * AMIGO drives work with HP85A 
   * SS80 drives work with HP85B (or HP85A with prm-85 add on board see links)
   * Printer emulator - can capture and save printer data to a time stamped file.
   * You can connect with and control the emulator via its FTI USB serial interface 115200 baud, 8N1.
     * There are many commands that you can use, type "help for a list"
     * Any key press halts the emulator and waits for a user command. 
     * After finishing any user commend it returns to GPIB disk emulation.
   * Each emulated disk image is a single file on a FAT32 formatted SDCARD.
     * Internally these disk images are formatted using the LIF standard.
       * LIF format used is compatible with HP85A/B and many other computers
     * LIF Tools are built into the emulator to create, rename, delete add and extract to/from other LIF images.
       * LIF tools can also create new LIF images with user specifications
     * The emulator will automatically create missing LIF images defined and named in hpdisk.cfg on the SDCARD
     * For the specific LIF E010..E013(hex) type images the emulator can translate to and from plain ASCII files.
     * The emulator can add and extract as LIF image format
       * You may add a single file from another LIF image with multiple internal files.
       * You may extract a single file from a LIF image to a new LIF image.
       * Extracted images have a 256 byte volume header, 256 byte directory followed by a file.
     * Type "lif help" in the emulator for a full list of commands
       * See the top of [lifutils.c](lif/lifutils.c) for full documentation and examples.
     * TeleDisk to LIF extractor tool included - see [lif README.md](lif/README.md)
       * [td02lif](lif/t202lif) [85-SS80.TD0](lif/85-SS80.TD0) [85-SS80.LIF](lif/85-SS80.LIF)
___

## SD Card requirments
   * The HP85 is sensitive to write delays so we need SD cards with fast random writes.
   * I have found that the Sandisk Extreme and Sandisk Extreme Pro cards work best.
   * Why ?  Each block that is written reads,erases, modifies a in new internal flash page (can be over a megabyte). Most SD cards are optimized for sequencial writting and do not do well with random writes. There is a huge difference in various cards on the market. Look for the cards with the best 4K randowm write times. Some SD cards are so slow it will cause the HP85 to timeout waiting for the card. Best source of benchmark information is looking for recent Raspberry Pi SD card benchmarks - specifically 4k random write - faster is better.

___ 


## OS Requirements for software building
  * I used Ubuntu 18.04,16.04LTS and 14.04LTS when developing the code
    * It should be easy to setup the same build with Windows gcc tools.

## Install Ubuntu Packages required for Building
  * sudo bash
  * *apt-get update*
  * *apt-get install aptitude make build-essential binutils gcc*
  * *aptitude --with-recommends install minicom avr-libc avra avrdude avrdude-doc avrp binutils-avr gcc-avr gdb-avr*
  * pip install pyserial

## Compiling AVR and standlone LIF tools
  * *make clean*
  * *make*

## Flashing AVR Firmware 
  * You will need and AVR programmer supported by avrdude (avrdude is part of avrtools installed in last step)
    * I am using atmelice_isp but the [Makefile](Makefile) as example for:
      * *avrispmkII atmelice atmelice_dw atmelice_isp atmelice_pdi*
  * make flash

## Flashing the firmware to the AVR with avrdude and programmer
  * *make flash*
    * This will use *avrdude* with the new low cost Atmel ICE programmer.
      * If you wish to another programmer then update the "flash" avrdude command line in the [Makefile](Makefile).
      * There is an example with the AVR mkii programmer as well.

## Building Doxygen documentation for the project - optional
  * *aptitude install --with-recommends doxygen doxygen-doc doxygen-gui doxygen-latex*
  * *If you omit this you will have to update the [Makefile](Makefile) to omit the steps*
___

## Using the emulator with provider examples
   * See [sdcard.cfg](sdcard/sdcard.cfg) for configuration settings and setting and documentation.
     * Printer capture is configured currently for my HP54645D scope
       * The following example works for an HP85 attached to the emulator via GPIB bus.
         * PRINTER IS 705
         * PLIST
     * Disk images in [sdcard](sdcard) folder drive and configuration settings
       * First SS80 HP9134L disk at 700 for my HP85A (with 85A ROMs)
       * First Amigo 9121D disk  at 710 for my HP85B (with 85B ROMs)
       * Second SS80 HP9134L disk at 720 for my HP85A (with 85A ROMs)
       * Second Amigo 9121D disk  at 730 for my HP85B (with 85B ROMs)
     * How to use the examples with your HP85
       * Copy the files inside the project [sdcard](sdcard) folder to the home folder of a fat32 formatted drive
         * All image files and configuration must be in the home folder only - not in a subdirectory.
         * You may store other user files in sub folders of your choosing.
       * Verify [hpdisk.cfg](sdcard/hpdisk.cfg) configuration settings for your computer
       * Insert card into emulator
       * Attract GPIB cables
       * Power on emulator
       * Power on your computer last!
          * The emulator MUST be running and attached to your computer first!
          * The HP85 ONLY checks for drives at power up.
___
## Understanding Drive GPIB BUS addressing and Parallel Poll Response (PPR) - HP85A vs. HP85B
  * While GPIB devices can have address between 0 and 31 you can have no more than 8 disk drives.
  * ALL disk drives are required to respond to a PPR query by the (HP85) controller.
    * PPR query is done when the controller in charge (HP85) pulls ATN and EOI low.
    * PPR response occurs when a disk drive pulls one GPIB bus data line low in response.
       * You can only have 8 of these because there are only 8 GPIB data bus lines.
         * GPIB data bus bits are numbered from 1 to 8
         * PPR response bits are *assigned in reverse order* starting from 8, bit 8 for device 0
           * This is a GPIB specification - not my idea.
         * The HP85 uses these assumptions
            * PPR bits are assigned in reverse order from device numbers.
  * IMPORTANT! On power up the HP85 issues a PPR query for disk drives 
    * The emulator must be running BEFORE this happens.
    * PPR query = both ATN and EOI being pulled low by the computer.
    * PPR response is when each drive pulls a single GPIB data bus bit LOW - while ATN and EOI are low.
       * *ONLY* those that are detected in this way are then next scanned
    * Next for all detected drives the HP85 issues "Request Identify" to each in turn.
      * This is done one drive at a time in order
      * The PPR keyword in the [hpdisk.cfg](sdcard/hpdisk.cfg) is the PPR bit the drive uses
        * PPR of 0 = PPR response on GPIB data bus bit number 8 - as per GPIB BUS specifications.
      * The ID keyword in [hpdisk.cfg](sdcard/hpdisk.cfg) is the 16 bit reply to "Request Identify Reply"
        * IMPORTANT! AMIGO drives cannot be queried for detailed drive layout information
          * The HP85A can only use its *hardcoded firmware tables* to map ID to disk layout parameters
          * This implies that the HP85A can only use AMIGO disks it has defined in firmware.
        * The HP85B can query newer SS80 drives for detailed drive layout information instead.
        * The HP85A cannot use SS80 drives unless it uses copies of the HP85B EMS and EDISK ROMS.
            * One way this can be done with the PRM-85 expansion board offered by Bill Kotaska 
              * (The PRM-85 is great product giving you access to all of the useful ROMS)
___
## Limitations
 * Multiple drive support is impliments but UNIT support is NOT
 * While most AMIGO and SS80 feature have been implemented my primary focus was on the HP85A and HP85B.
   * (I do not have access to other computers to test for full compatibility)
   * This means that a few AMIGO and SS80 GPIB commands are not yet implemented!
     * Some of these are extended reporting modes - many of which are optional.
   * Note: The HP85A can only use AMIGO drives - unless you have the HP85B EMS ROM installed in your HPH9A
      * This can be done with the PRM-85 expansion board offered by Bill Kotaska (a great product!)
 * To attach a drive to our computer, real or otherwise, you must know:
   * The correct GPIB BUS address and parallel pool response (PPR) bit number your computer expects.
     * See ADDRESS, PPR and ID values in [hpdisk.cfg](sdcard.cfg)
   * Older computers may only support AMIGO drives.
     * Such computers will have a hard coded in firmware list of drive its supports.
       * These computers will issue a GPIB BUS "request identify" command and only detect those it knows about.
       * *If these assumptions do NOT match the layout defined in the [hpdisk.cfg](sdcard/sdcard.cfg) no drives will be detected.*
   * Newer computers with SS80 support can request fully detailed disk layout instead of the "request identify"
   * My emulator supports both reporting methods - but your computer may not use them both!
     * For supported values consult your computer manuals or corresponding drive manual for your computer.
       * See gpib/drives_parameters.txt for a list on some known value (CREDITS; these are from the HPDir project)
     * In all cases the [hpdisk.cfg](sdcard/hpdisk.cfg) parameters MUST match these expectations.
   * The [hpdisk.cfg](sdcard/hpdisk.cfg) file tells the emulator how the emulated disk is defined.
     * GPIB BUS address, Parallel Poll Response bit number and AMIGO Request Identify response values.
     * Additional detail for SS80 drives that newer computers can use.
     * In ALL cases the file informs the code what parameters to emulate and report.
       * ALL of these values MUST match your computers expectations for drives it knows about.
   * Debugging
     * You can enable reporting of all unimplemented GPIB commands (see *TODO* debug option in [hpdisk.cfg](sdcard/hpdisk.cfg) )
       * Useful if you are trying this on a non HP85 device
       * See the [hpdisk.cfg](sdcard/hpdisk.cfg) for documentation on the full list of debugging options
     * The emulator can passively log all transactions between real hardware on the GPIB bus 
       * Use the "gpib trace *logfile*" command - pressing any key exits - no emulation is done in this mode.
       * You can use this to help understand what is sent to and from your real disks.
       * I use this feature to help prioritize which commands I first implemented.
___

## Credits

<b>I really owe the very existence of this project to the original work done by Anders Gustafsson and his great "HP Disk Emulator" </b>
 * You can visit his project at this site:
   * <http://www.dalton.ax/hpdisk>
   * <http://www.elektor-labs.com/project/hpdisk-an-sd-based-disk-emulator-for-gpib-instruments-and-computers.13693.html>

<b> The HPDir project was vital as a documentation source for this project</b>
   * <http://www.hp9845.net/9845/projects/hpdir>

 <b>Anders Gustafsson was extremely helpful in providing me his current 
 code and details of his project - which I am very thankful for.</b>

 As mainly a personal exercise in fully understanding the code I 
 ended up rewriting much of the hpdisk project. I did this one part at a 
 time as I learned the protocols and specifications - NOT because of any 
 problems with his original work. 

 Although mostly rewritten I have maintained the basic concept of using  state machines for GPIB read and write functions as well as for SS80 execute state tracking. 

[lif/teledisk](lif/teledisk)
 * [lif/teledisk](lif/teledisk)
   * My TELEDISK LIF extracter
   * Important Contributions (My converted would not have been possible without these)
     * Dave Dunfield, LZSS Code and TeleDisk documentation
       * Copyright 2007-2008 Dave Dunfield All rights reserved.
       * [td0_lzss.h](lif/teledisk/td0_lzss.h)
       * [td0_lzss.c](lif/teledisk/td0_lzss.c)
         * LZSS decoder
       * [td0notes.txt](lif/teledisk/td0notes.txt)
         * Teledisk Documentation
     * Jean-Franois DEL NERO, TeleDisk Documentation
       * Copyright (C) 2006-2014 Jean-Franois DEL NERO
         * [wteledsk.htm](lif/teledisk/wteledsk.htm)
           * TeleDisk documenation
         * See his github project
             * https://github.com/jfdelnero/libhxcfe
___
# Abbreviations
Within this project I have attempted to provide detailed references to manuals, listed below.  I have included short quotes and section and page# reference to these works.
 * <b>SS80</b>
 * <b>CS80</b>
 * <b>A or Amigo</b>
 * <b>HP-IP</b>
 * <b>HP-IP Tutorial</b>

___
## Documentation References and related sources of information
 * Web Resources
   * <http://www.hp9845.net>
   * <http://www.hpmuseum.net>
   * <http://www.hpmusuem.org>
   * <http://bitsavers.trailing-edge.com>
   * <http://en.wikipedia.org/wiki/IEEE-488>
   * See [Documents folder](documents)

___
## Enhanced version of Tony Duell's lif_utils by Joachim
   * <https://github.com/bug400/lifutils>
   * Create/Modify LIF images

___
## CS80 References: ("CS80" is the short form used in the project)
   * "CS/80 Instruction Set Programming Manual"
   * Printed: APR 1983
   * HP Part# 5955-3442
   * See [Documents folder](documents)

___
## Amigo References: ("A" or "Amigo" is the short form used in the project)
   * "Appendix A of 9895A Flexible Disc Memory Service Manual"
   * HP Part# 09895-90030
   * See [Documents folder](documents)

___
## HP-IB
   * ("HP-IB" is the short form used in the project)
   * "Condensed Description of the Hewlett Packard Interface Bus"
   * Printed March 1975
   * HP Part# 59401-90030
   * See [Documents folder](documents)

___
## Tutorial Description of the Hewlett Packard Interface Bus
   * ("HP-IB Tutorial" is the short form used in the project)
   * <http://www.hpmemory.org/an/pdf/hp-ib_tutorial_1980.pdf>
   * Printed January 1983
   * <http://www.ko4bb.com/Manuals/HP_Agilent/HPIB_tutorial_HP.pdf>
   * Printed 1987
   * See [Documents folder](documents)

___
## GPIB / IEEE 488 Tutorial by Ian Poole
    * <http://www.radio-electronics.com/info/t_and_m/gpib/ieee488-basics-tutorial.php>
   * See [Documents folder](documents)

___
## HP 9133/9134 D/H/L References
   * "HP 9133/9134 D/H/L Service Manual"
   * HP Part# 5957-6560
   * Printed: APRIL 1985, Edition 2
   * See [Documents folder](documents)
___

## LIF File system Format
   * <http://www.hp9845.net/9845/projects/hpdir/#lif_filesystem>
   * See [Documents folder](documents)
___

## Useful Utilities
   * HP Drive  (HP Drive Emulators for Windows Platform)
     * <http://www.hp9845.net/9845/projects/hpdrive/>
   * HP Dir    (HP Drive - Disk Image Manipulation)
     * <http://www.hp9845.net/9845/projects/hpdir/>
___

## GPIB Connector pinout by Anders Gustafsson in his hpdisk project
  * http://www.dalton.ax/hpdisk/


<pre>
    Pin Name   Signal Description       Pin Name   Signal Description 
    1   DIO1   Data Input/Output Bit 1  13  DIO5   Data Input/Output Bit 5 
    2   DIO2   Data Input/Output Bit 2  14  DIO6   Data Input/Output Bit 6 
    3   DIO3   Data Input/Output Bit 3  15  DIO7   Data Input/Output Bit 7 
    4   DIO4   Data Input/Output Bit 4  16  DIO8   Data Input/Output Bit 8 
    5   EIO    End-Or-Identify          17  REN    Remote Enable 
    6   DAV    Data Valid               18  Shield Ground (DAV) 
    7   NRFD   Not Ready For Data       19  Shield Ground (NRFD) 
    8   NDAC   Not Data Accepted        20  Shield Ground (NDAC) 
    9   IFC    Interface Clear          21  Shield Ground (IFC) 
    10  SRQ    Service Request          22  Shield Ground (SRQ) 
    11  ATN    Attention                23  Shield Ground (ATN) 
    12  Shield Chassis Ground           24  Single GND Single Ground
</pre>



## Testing
  * Testing was done with an HP85A (with extended EMS ROM) 
    * Using the Hewlett-Packard Series 80 - PRM-85 by Bill Kotaska
    * This makes my HP85A look like and HP85B 
      * I can also use the normal mass storage ROM if I limit to AMIGO drives.
      * http://vintagecomputers.site90.net/hp85/prm85.htm

  * Note: the EMS ROM has extended INITIALIZE attributes
<pre>
  # Initializing: (already done on these images so you do not have to)
  INITIALIZE "SS80-1",":D700",128,1
  INITIALIZE "AMIGO1",":D710",14,1
  INITIALIZE "SS80-2",":D720",128,1
  INITIALIZE "AMIGO2",":D730",14,1
  
  # Listing files:
  # first SS80
  CAT ":D700"
  # first AMIGO
  CAT ":D710"
  # second SS80
  CAT ":D720"
  # second AMIGO
  CAT ":D730"
  
  # Loading file from second SS80:
  LOAD "HELLO:D720"
  # Copying file between devices: fist AMIGO to second AMIGO
  COPY "HELLO:D710" TO "HELLO:D730"
  # Copying ALL files between devices: FIRST SS80 to Second SS80
  COPY ":D700" TO ":D720"

  # Load BASIC program from a text file 
  GET "HELLO":D730"
  # Save a BASIC program as test file
  SAVE "HELLO:D720"
  # Delete a file
  PURGE "HELLO:D730"
  # Load a BASIC format program
  LOAD "HELLO:D700"
  # Save a BASIC format program
  SAVE "HELLO:D710"
  # Clear memory
  SCRATCH
  # List a BASIC program
  LIST
</pre>
___ 

## AVR Terminal Commands
 * Pressing any key will break out of the gpib task loop until a command is entered
   * help
      Will list all available commands and options

   * For detail using LIF commands to add/extract LIF files from SD card see the top of lif/lifutil.c
___ 


## Main project files for hp85disk Project 
  * Project Main Home Directory
    * [main.c](main.c)
    * [main.h](main.h)
      * Main start-up code
    * [Makefile](Makefile)
      * Main Project Makefile
  * Terminal scripts
    * [miniterm](miniterm)
      * wrapper for miniterm.py part of the python package pyserial
    * [term](term)
     * Wrapper for minicom terminal emulator
  * Doxygen files
    * [Doxyfile](Doxyfile)
      * Doxygen Configuration files for project
    * [doxyinc](doxyinc)
      * Determins which files are included in the project Doxygen documents
    * [DoxygenLayout.xml](DoxygenLayout.xml)
      * Doxygen Layout file
  * Project Readme
    * [README.md](README.md)
      * Project README 
  * Project Copyright
    * [COPYRIGHT.md](COPYRIGHT.md)
      * Project CopyRights 

## Board design file for version 1 and 2 hardware information
  * [board](board)
    * [V1](board/V1)
      * V1 Board documentation and Release files
      * [board design and pinouts of this project and a schematic PDF ](board/V1/HP85Disk.pdf)
      * [board design and pinouts of this project and a schematic DOC ](board/V1//HP85Disk.doc)
      * [board README.md](board/V1/HP85Disk.doc)
    * [V2/releases](V2/releses)
      * Jay Hamlin version 2 circuit board design using GPIB buffers

## Documents
  * [Documents](Documents)
  * GPIB BUS, HP device, LIF and chips documentation for this project
    * [Documents/README.md](Documents/README.md) 

## FatFs
  * [fatfs](fatfs)
    * R0.12b FatFS code from (C) ChaN, 2016 - With very minimal changes 
    * [00history.txt](fatfs/00history.txt)
    * [00readme.txt](fatfs/00readme.txt)
    * [ff.c](fatfs/ff.c)
    * [ffconf.h](fatfs/ffconf.h)
    * [ff.h](fatfs/ff.h)
    * [integer.h](fatfs/integer.h)

  * [fatfs.hal](fatfs.hal/fatfs.hal)
    * R0.12b FatFS code from (C) ChaN, 2016 with changes
      * Hardware abstraction layer based on example AVR project
    * [diskio.c](fatfs.hal/diskio.c)
      * Low level disk I/O module glue functions (fatfs.hal/C)ChaN, 2016 
    * [diskio.h](fatfs.hal/diskio.h)
      * Low level disk I/O module glue functions (fatfs.hal/C)ChaN, 2016 
    * [mmc.c](fatfs.hal/mmc.c)
      * Low level MMC I/O by (fatfs.hal/C)ChaN, 2016 with modifications
    * [mmc.h](fatfs.hal/mmc.h)
      * Low level MMC I/O by (fatfs.hal/C)ChaN, 2016 with modifications
    * [mmc_hal.c](fatfs.hal/mmc_hal.c)
      * My Hardware abstraction layer code
    * [mmc_hal.h](fatfs.hal/mmc_hal.h)
      * My Hardware abstraction layer code
  
  * [fatfs.sup](fatfs.sup/fatfs.sup)
    * Support utility and POSIX wrapper factions
    * [fatfs.h](fatfs.sup/fatfs.h)
      * FatFS header files
    * [fatfs_sup.c](fatfs.sup/fatfs_sup.c)
    * [fatfs_sup.h](fatfs.sup/fatfs_sup.h)
      * FatFS file listing and display functions
    * [fatfs_tests.c](fatfs.sup/fatfs_tests.c)
    * [fatfs_tests.h](fatfs.sup/fatfs_tests.h)
      * FatFS user test functions

## GPIB related
  * [gpib](gpib/gpib)
    * My GPIB code for AMIGO SS80 and PPRINTER support
    * [amigo.c](gpib/amigo.c)
      * AMIGO parser
    * [amigo.h](gpib/amigo.h)
      * AMIGO parser
    * [defines.h](gpib/defines.h)
      * Main GPIB header and configuration options
    * [debug.txt](debug.txt)
      * List of debug flags
    * [drives.c](gpib/drives.c)
      * Supported Drive Parameters 
    * [drive_references.txt](gpib/drive_references.txt)
      * General Drive Parameters Documentation for all known drive types
    * [format.c](gpib/format.c)
      * LIF format and file utilities
    * [gpib.c](gpib/gpib.c)
      * All low level GPIB bus code
    * [gpib.h](gpib/gpib.h)
      * GPIB I/O code
    * [gpib_hal.c](gpib/gpib_hal.c)
      * GPIB hardware abstraction code
    * [gpib_hal.h](gpib/gpib_hal.h)
      * GPIB hardware abstraction code
    * [gpib_task.c](gpib/gpib_task.c)
      * GPIB command handler , initialization and tracing code
    * [gpib_task.h](gpib/gpib_task.h)
      * GPIB command handler , initialization and tracing code
    * [gpib_tests.c](gpib/gpib_tests.c)
      * GPIB user tests
    * [gpib_tests.h](gpib/gpib_tests.h)
      * GPIB user tests
    * [printer.c](gpib/printer.c)
      * GPIB printer capture code
    * [printer.h](gpib/printer.h)
      * GPIB printer capture code
    * [references.txt](gpib/references.txt)
      * Main S80 SS80 AMIGO and GPIB references part numbers and web links
    * [ss80.c](gpib/ss80.c)
      * SS80 parser
    * [ss80.h](gpib/ss80.h)
      * SS80 parser
    * [notes.txt](gpib/notes.txt)
      * My notes on GPIB bus states as it relates to the project

## Hardware CPU specific 
  * [hardware](hardware)
    * CPU hardware specific code
    * [baudrate.c](hardware/baudrate.c)
      * Baud rate calculation tool. Given CPU clock and desired baud rate, will list the actual baud rate and registers
    * [bits.h](hardware/bits.h)
      * BIT set and clear functions
    * [cpu.h](hardware/cpu.h)
      * CPU specific include files
    * [delay.c](hardware/delay.c)
    * [delay.h](hardware/delay.h)
      * Delay code
    * [hal.c](hardware/hal.c)
    * [hal.h](hardware/hal.h)
      * GPIO functions, spi hardware abstraction layer and chip select logic
    * [iom1284p.h](hardware/iom1284p.h)
      * GPIO map for ATEMEGA 1284p
    * [mkdef.c](hardware/mkdef.c)
      * Not used
    * [pins.txt](hardware/pins.txt)
      * AVR function to GPIO pin map
    * [ram.c](hardware/ram.c)
    * [ram.h](hardware/ram.h)
      * Memory functions
    * [rs232.c](hardware/rs232.c)
    * [rs232.h](hardware/rs232.h)
      * RS232 IO
    * [rtc.c](hardware/rtc.c)
    * [rtc.h](hardware/rtc.h)
      * DS1307 I2C RTC code
    * [spi.c](hardware/spi.c)
    * [spi.h](hardware/spi.h)
      * SPI BUS code
    * [TWI_AVR8.c](hardware/TWI_AVR8.c)
    * [TWI_AVR8.h](hardware/TWI_AVR8.h)
      * I2C code LUFA Library Copyright (hardware/C) Dean Camera, 2011.
    * [user_config.h](hardware/user_config.h)
      * Main include file MMC SLOW and FATS frequency and CPU frequency settings

## Common libraries
  * [lib](lib)
    * Library functions
    * [bcpp.cfg](lib/bcpp.cfg)
      * BCPP C code formatter config
    * [matrix.c](lib/matrix.c)
    * [matrix.h](lib/matrix.h)
      * Matrix code - not used
    * [matrix.txt](lib/matrix.txt)
      *  A few notes about matrix operations
    * [queue.c](lib/queue.c)
    * [queue.h](lib/queue.h)
      * Queue functions
    * [sort.c](lib/sort.c)
    * [sort.h](lib/sort.h)
      * Sort functions - not used
    * [stringsup.c](lib/stringsup.c)
    * [stringsup.h](lib/stringsup.h)
      * Various string processing functions
    * [time.c](lib/time.c)
    * [time.h](lib/time.h)
      * POSIX time functions
    * [timer.c](lib/timer.c)
    * [timer.h](lib/timer.h)
      * Timer task functions
    * [timer_hal.c](lib/timer_hal.c)
      * Timer task hardware abstraction layer
    * [timetests.c](lib/timetests.c)
      * Time and timer test code

## LIF files
  * [lif](lif)
    * LIF disk image utilities 
    * [lif/lifutils.c](lif/lifutils.c)
    * [lif/lifutils.c](lif/lifutils.c)
      * Functions that allow the emulator to import and export files from LIF images 
    * [Makefile](lif/Makefile)
      * Permits creating a standalone Linux version of the LIF emulator tools
    * Code by Mike Gore
      * [Makefile](lif/Makefile)
        * Make stand alone Linux versions of LIF utility and optionaly TeleDisk to LIF converter
      * [lifsup.c](lif/lifsup.c)
      * [lifsup.h](lif/lifsup.h)
        * Stand alone libraries for LIF and TELEDISK utility 
          * These functions are also part of various hp85disk libraries
      * [lifutils.c](lif/lifutils.c)
      * [lifutils.h](lif/lifutils.h)
        *  LIF image functions, directory listing and file adding.extracting,renaming,deleting
      * [td02lif.c](lif/td02lif.c)
      * [td02lif.h](lif/td02lif.h)
        * My TeleDisk to LIF translator
      * [lif-notes.txt](lif/lif-notes.txt)       
        * My notes on decoding E010 format LIF images for HP-85
      * [README.txt](lif/README.txt)
        * Notes on each file under lif and lif/teledisk
      * [85-SS80.TD0](lif/85-SS80.TD0) from hpmuseum.org
        * Damaged SS80 Excersizer from HP Musium
      * [85-SS80.LIF](lif/85-SS80.LIF)
        * The current converter automaticcal did these repairs
          *  cyl 11, side 0 sector 116 mapped to 8
          *  cyl 13, side 0 sector 116 mapped to 11
          *  cyl 15, side 0 sector 10  missing - zero filled

## LIF teledisk files
   * [lif/teledisk](lif/teledisk)
     * My TELEDISK LIF extracter 
       * Note: The TeleDisk image MUST contain a LIF image  - we do NOT translate it
     * [README.txt](lif/teledisk/README.txt)
       * Credits
     * Important Contributions (My converter would not have been possible without these)
       * Dave Dunfield, LZSS Code and TeleDisk documentation
         * Copyright 2007-2008 Dave Dunfield All rights reserved.
         * [td0_lzss.h](lif/teledisk/td0_lzss.h)
         * [td0_lzss.c](lif/teledisk/td0_lzss.c)
           * LZSS decoder
         * [td0notes.txt](lif/teledisk/td0notes.txt)
           * Teledisk Documentation
       * Jean-Franois DEL NERO, TeleDisk Documentation 
         * Copyright (lif/teledisk/C) 2006-2014 Jean-Franois DEL NERO
           * [wteledsk.htm](lif/teledisk/wteledsk.htm)
             * TeleDisk documenation
           * See his github project
             * https://github.com/jfdelnero/libhxcfe

## Posix wrapper - provides linux file IO functions for Fatfs
  * [posix](posix)
    * POSIX wrappers provide many UNIX POSIX compatible functions by translating fatfs functions 
    * [buffer.c](buffer.c)
    * [buffer.h](buffer.h)
      * Currently unused in this project
    * [posix.c](posix/posix.c)
    * [posix.h](posix/posix.h)
      * POSIX wrappers for fatfs - Unix file IO function call wrappers
    * [posix_tests.c](posix/posix_tests.c)
    * [posix_tests.h](posix/posix_tests.h)
      * POSIX user tests

## Printf display functions
  * [printf](printf)
    * Printf and math IO functions
    * [mathio.c](printf/mathio.c)
      * Number conversions 
    * [mathio.h](printf/mathio.h)
      * Number conversions 
    * [n2a.c](printf/n2a.c)
      * Binary to ASCII converter number size only limited by memory
    * [printf.c](printf/printf.c)
      * My small printf code - with floating point support and user defined low character level IO
    * [sscanf.c](printf/sscanf.c)
      * My small scanf code - work in progress
    * [test_printf.c](printf/test_printf.c)
      * Test my printf against glibs 1,000,000 tests per data type

## SD Card files for project
  * [sdcard](sdcard)
    * My HP85 AMIGO and SS80 disk images
      * [hpdisk.cfg](sdcard/hpdisk.cfg)
        * All Disk definitions, address, PPR, DEBUG level for SS80 and AMIGO drives
        * PRINTER address
      * [amigo.lif](sdcard/amigo.lif)
        * AMIGO disk image file number 1
        * Has some demo basic programs in it
      * [amigo-2.lif](sdcard/amigo-2.lif)
        * AMIGO disk image file number 2
        * Has some demo basic programs in it
      * [ss80.lif](sdcard/ss80.lif)
        * SS80 hard drive disk image file number 1
        * Has some demo basic programs in it
      * [ss80-2.lif](sdcard/ss80-2.lif)
        * SS80 hard drive disk image file number 2
        * Has some demo basic programs in it
    * My HP85 bus trace files
      * [amigo_trace.txt](sdcard/amigo_trace.txt)
        * AMIGO trace file when connected to HP85 showing odd out of order command issue
      * [gpib_reset.txt](sdcard/gpib_reset.txt)
        * GPIB reset trace when connected to HP85
      * [gpib_trace.txt](sdcard/gpib_trace.txt)
        * GPIB transaction trace when connected to HP85
    * My HP85 plot capture files
        * [plot1.plt](sdcard/plot1.plt)
        * [plot2.plt](sdcard/plot2.plt)
    * [sdcard/TREK85](sdcard/TREK85)
	  * TREK85 by Martin Hepperle, December 2015
	  * https://groups.io/g/hpseries80/topic/star_trek_game_for_hp_85/4845241
        * [author.txt](sdcard/TREK85/author.txt)  
        * [readme.txt](sdcrad/TREK85/readme.txt)	
        * [Star Trek.pdf](sdcard/TREK85/Start Trek.pdf)
        * [TREK85.BAS](sdcrad/TREK85/TREK85.BAS)
        * [trek.lif](sdcard/TREK85/trek.lif)

___
