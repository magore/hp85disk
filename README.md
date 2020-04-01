\section README

# Documentation
## HP85 Disk Emulator Copyright (C) 2014-2020 Mike Gore 
  * This Project Emulates AMIGO and SS80 disk drives used by the HP85 series computers.
  * The images are stored in LIF format used by the HP85 series computers
  * See [COPYRIGHT](COPYRIGHT.md) for a full copyright notice for the project
  * Documentation created using Doxygen available at
    * https://rawgit.com/magore/hp85disk/V2/doxygen/html/index.html
    * All emulated disk images are just regular files stored on a standard FAT32 formatted SD Card

## Other resources for disk image manipulation
  * [HPDrive project has very useful references an tools for creating HP disk LIF images compatible with this project](http://www.hp9845.net/9845/projects/hpdrive)
    * Copyright © 2010 A. Kückes
  * [HPDir project has very useful references an tools for manipulating HP disk LIF images compatible with this project](http://www.hp9845.net/9845/projects/hpdrive)
    * Copyright © 2010 A. Kückes
  * [ See LIF Documentation - part of the HPDir project for details](http://www.hp9845.net/9845/projects/hpdir/#lif_filesystem)
    * Copyright © 2010 A. Kückes

## Features for the HP85 in brief
  * NOTE: Later sections go into more detail
  * This project emulates GPIB drives and HPGL printer 
    * Each emulated disk image is a LIF encoded file on a FAT32 formatted SDCARD.
   * [hpdisk.cfg](sdcard/hpdisk.cfg) fully defines each disk image on SD Card
     * Disk images are LIF encoded files that are compatible with HP85A/B and many other computers
     * Missing disk image files are created automatically if 
  * The emulator will automatically create missing LIF images defined in hpdisk.cfg on the SDCARD
  * There are disk images for AMIGO and SS80 disks
    * AMIGO drives work with HP85A 
    * SS80 drives work with HP85B (or HP85A with prm-85 wth modified EMS and Electronic disk rom add on board see links)
      * You may have up to 4 disks with V1 hardware and 8 with V2 hardware
  * There is a Printer emulator that can capture and save printer data to a time stamped file.
  * Built in command processor with lots of useful features
    * Want to translate between plain text BASIC files and HP85 BASIC programs? You Can!
      * See full details later in this document
    * Access the hp85disk command processor via its USB interface and your computer with a serial terminal program
      * See full details later in this document for serial terminal configuration and finding the device port name
    * Any key press halts the emulator and waits for a user command
      * After finishing any user commend and options press the Enter/Return key to return to disk emulation
    * Type "help" for a list of top level commands
      * Each help item has its own help
      * Example: lif help
   * LIF manipulation tools are built in see later sections for details
   * Convert TeleDisk images into LIF
   * The emulator RTC can be used for time stamping plot files and files added into lif images

___ 


## HP85 disk emulator V2 circuit board layout design by (C) 2018-2020 Jay Hamlin
## V2 board design - github V2 branch targets the new board by Jay Hamlin
## V2 code is now working
  * [Jay Hamlin designed this board](board/V2/releases)
  * V2 hardware adds
   * GPIB BUS drivers
     * 48Mma drive required by the GPIB spec
   * I2C level converters and standard Qwiic Bus interface - 3.3V
      * optional RTC chips like the DS3231 
      * LCD displays - work in progress
   * Advanced Hardware Reset circuit
   * Full size SD card interface with Card detect

## HP85 disk emulator V1 board design (C) 2014-2020 Mike Gore
## [V1 board readme](board/V1/README.md)
  * Limited control and BUS drive power 
    * About half of the 48Mma drive required by the GPIB spec
    * However we can read any pin any time - useful for tracing/debugging
  * RTC DS1307 for time stamping 
  * [My original board design without GPIB buffers](board/V1/README.md)


___ 

## HP85 emulator Makefile configuration options for original V1 and new V2 boards
## Makefile Configuration options for building
  * Update BAUD, PORT, BOARD, PPR_REVERSE_BITS and RTC_SUPPORT for your platform
    * BOARD is the version of the hardware
      * 1 = V1 hardware without GPIB BUS transceivers
      * 2 = V2 hardware with GPIB BUS transceivers
    * PPR_REVERSE_BITS
      * Is now set by board revision
      * 0 = V1 hardware without the GPIB buffers 
      * 1 = V2 hardware with GPIB buffers 
    * RTC_SUPPORT for Real Time Clock
	  * 1 = RTC support for a DS1307 command compatible RTC chip - the DS3231 is the 3.3V version
        * This will time stamp plot files and add time stamps inside lif images
    * PORT is the emulator serial PORT name as detected by your operating system
      * /dev/ttyUSB0 on my system
    * BAUD  is the emulator serial baud rate 
      * 115200 = a safe default that most systems can manage
        * NOTE: My development environment works with 500000 baud but 115200 should work on all systems
        * NOTE: Faster is better when enabling more debug messages 
          * Too many messages can cause the HP85 to timeout waiting for IO
    * DEVICE
      * Target AVR device used by GCC for this project
      * atmega1284p
        * DO NOT CHANGE THIS - there are too main dependencies
    * F_CPU  
      * CPU frequency
        * 20000000
    * AVRDUDE_DEVICE is the name of AVR as it is known by avrdude
      * m1284
    * AVRDUDE_SPEED  is the programming clock speed used by avrdude
      * 5
        * My device works with 0.25 but 5 is safe
     * AVRDUDE ISP PORT
      * usb
        * Set = to the PORT definition above IF using self programmin
    * AVRDUDE_ISP
      * avrdude device programmer name as known by avrdude
        * avrdude -c list  # for a list of devices
      * I am using the atmelice_isp  Atmel-ICE (ARM/AVR) in ISP mode

## Example building
  * make clean
  * make
  * make flash    # uses built in programmer to flash the code
    * OR 
      * make flash-isp  # uses a 6 wire ISP to flash the code

## make help documentation
  * make help
    * List the common commands to compile/install/flash the code
      <verbatim>
        Building Commands
            make install           - builds and installs all command line utilities
            make sdcard            - builds all sdcard images and creates default hpdisk.cfg and amigo.cfg files
            make release           - builds all code and copies files to the release folder
            make clean             - cleans all generated files
            make                   - builds all code
        
        Programming using an 6 wire ISP - installs optiboot
            make install_optiboot  - install optiboot boot loaded using an ISP
            make flash-isp         - build and flash the code using an ISP
            make flash-isp-release - flash the release code using an ISP
            make verify-isp        - verify code using an ISP
            make verify-isp-release- verify release code using an ISP
        
        Programming using the built in optiboot programmer
            make flash             - build and flash the code using built in optiboot programmer
            make flash-release     - flash the release code using built in optiboot programmer
            make verify            - verify code using built in optiboot programmer
            make verify-release    - verify release code using built in optiboot programmer
        
        Programming using an 6 wire ISP - WITHOUT installing optiboot
            IMPORTANT - you will not be able to use non isp flashing modes later on
               Makes booting and flashing process slightly faster
            make flash-isp-noboot         - build and flash the code using an ISP
            make flash-isp-noboot-release - flash the release code using an ISP
      </verbatim>

## Example building with Makefile overrides
  * ( export BAUD=500000UL; export AVRDUDE_SPEED=1;  make flash-isp)
    * Make sure you add the '(' and ')' around the commands
      * This prevents settings variables in your current shell
___ 


## Detailed information about tools and features 

## Built in command processor with many tools
## Accessing the hp85disk command interface with a serial terminal
  * Used to access the hp85disk command interface
  * Access it via the USB cable attached to your computer using a serial terminal program

## Finding the emulator serial port device name on you Linux system
  * Make sure the HP85disk is initially NOT attached via the USB cable to the desktop
  * Open a Linux terminal window - not serial terminal
  * In this terminal windows type
      * ls -lart /dev | tail -1
        * The '|' symbol means "pipe" the output of one Linux command to another
          * This '|' symbol is found on US keyboards just above the enter key
  * Attach the emulator USB port to your Linux Desktop
    * Again type 
      * ls -lart /dev | tail -1
        * If the result changes between the two commands the last line has the device name
          * On my system its is typically /dev/ttyUSB0  - it all depends on how many USB serial devices you have attached

## Configuring the serial communication program 
  * With the hp85disk attached via USB cable to your desktop
  * Open you favourite serial terminal emulator
    * Set BAUD rate to 115200 
    * Set 8 Data bits NO parity
    * Set flow control to NONE
    * Set the PORT to the device name of the emulator USB port
      * Example: /dev/ttyUSB0 see next section

___ 


## LIF tools are built into emulator firmware 
  * Built in help
    * lif help
      * Gives lif commands
  * NOTE: Each disk image is a single file, encoded in LIF format,saved on the SD Card
    * LIF format is a common the filesystem on series 80 computers.
    * LIF format is also a vary common file interchange format for series 80 computers
      * LIF format includes file date,size permissions and other important meta data
  * You can work with LIF images 
    * Directory listing of LIF images and SSD Card files
      * If you have an RTC the listing can display file and LIF volume date and time
        * Display time stamps if they were set
          * But only if they were created or added with the built in tools
    * add an ascii file to LIF image
      * This function permits renaming of the translated file
      * They get translated between HP85 DTA8x (type E010) format and plain text files!!!
    * extract ascii files from LIF image
      * This function permits renaming of the translated file
      * They get translated between HP85 DTA8x (type E010) format and plain text files!!!
    * add binary programs from one LIF image to another LIF image
      * This function permits renaming of the translated file
    * extract a single binary file or program into a new LIF image
      * This function permits renaming of the translated file
      * Extracted LIF images contain a single file a 256 byte volume header, 256 byte directory followed by a file.
    * delete file in LIF image
    * rename file in LIF image
    * Create, import, export copy, rename, delete, etc
      * You can add a plain text file, and translate it, into a LIF image with file format DTA8x (type E010)
      * You can extract an translate DTA8x (type E010) into a plain text files
    * [For more LIF documentation](lif/README.md)
    * Also see the Other Resources section above

## TeleDisk to LIF conversion tool (updated) - see [LIF README.md](lif/README.md)
  * [td02lif](lif/t202lif) [85-SS80.TD0](lif/85-SS80.TD0) [85-SS80.LIF](lif/85-SS80.LIF)
  * NOTE: There are a stand alone version of the tool that run on Linux - making it work on Windows should be easy
      * You can extract a DTA8x (type E010) file from a LIF image and translate it into plain text
  * TeleDisk to LIF image conversion - a very common disk interchange format
    * See the top of [lifutils.c](lif/lifutils.c) for full documentation and examples.
    * create LIF image with options
    * NOTE: the emulator automatically creates missing images if defined in hpdisk.cfg
      * Type "lif help" in the emulator for a full list of commands
      * See the top of [lifutils.c](lif/lifutils.c) for full documentation and examples.

## Disk images and default configuration file for the hp85disk project
  * [sdcard folder has premade LIF disk images](sdcard)
    * [sdcard/create_images.sh creates the default LIF images and creates a matching default configuration files](sdcard/create_images)
  * [sdcard/hpdisk.cfg contains the default disk definitions that correspond to the LIF images - disk hardware definition](sdcard/hpdisk.cfg)
    * [sdcard/create_images.sh creates the default configuration and LIF images](sdcard/create_images)

## Note about LIF images and hpdisk.cfg disk definitions
  * To create/modify or update LIF images see the section on the lif utilities supplied with teh emulator
  * It is important that the LIF image size match the disk definitions
    * The emulator gets the hard limits for  disk using [sdcard/hpdisk.cfg](sdcard/hpdisk.cfg)
      * The attached computer requests these disk details from the emulator 
    * Then the attached computer reads the disk LIF headers for the LIF layout infomation. 
      * So as long as the LIF headers and hardware information match things should work fine.
        * IF the do not match you may get errors when
          * The LIF image is BIGGER then specified disk AND if the computer attempts to read outside the defined limits.
  * The emulator does not look at the LIF data when serving and image - that is up to the attached computer.
   * The computer also gets the disk description from the emulator when it scans for disks

___ 


## Specific details for Translating between plain text and HP85 BASIC programs using hp85disk
  * Requirements
    * hp85disk emulator 
      * There is a solution documented below in you do not have the hp85emulator
    * A HP utility commonly called "GETSAV" translates between HP85 BASIC and DTA8x (type E010 ) files
      * The utility adds functions "GET" and "SAVE" to your HP85 computer
        * This HP utility was part of a larger software package from HP for the HP85

## GETSAV is called GETSAVE inside my LIF disk images
  * [Here is a direct link to GETSAVE.LIF encoded as a LIF file - sdcard/LIF-files/GETSAVE.LIF](sdcard/LIF-files/GETSAVE.LIF)
  * My hp85disk tools built in the firmware can tools translate between DT8x (type E010 ) and plain text ASCII files
    * I provide stand alone LIF tools in the lif subfolder of the github project that do the same thing
  * Used together with the HP85 these tools can translate between ASCII plain text and HP85 BASIC programs!!!
    * Specific details steps are documented later in the README

## Translating between plain text and HP85 BASIC programs WITHOUT hp85disk
  * You must compile and install the stand alone lif tools found under the project [lif](lif) folder
  * You will also need a way of transfer binary files to/from your HP85 
    * You need to copy the GETSAVE program to your HP85 some how
  * Specific details steps are documented later in the README

## Initial setup of the hp85disk ASSUMPTIONS and Requirements 
  * You have the hp85disk emulator attached to your HP85 with a GPIB cable
    * Strongly advise having no other devices attached just durring setup/testing
      * This is to avoid other GPIB bus address conflicts initially - you can update addresses later
  * You have a Ubuntu Linux desktop - used for all my examples
  * All Text and configuration files used with the emulator MUST plain text format only (8 bit ascii) 
     * Please NO Unicode - both file names and file formats!
  * You have a serial terminal program installed - for example minicom 
  * You need a FAT32 format blank SD Card
    * Copy of the hp85disk, github V2 branch, sdcard folder contents onto the SD Card
      * Make sure you only copy the contents and NOT folder AND contents
        * The emulator assumes the SD card home directory contains the images and configuration files

___ 



## FULL example step by step translating bewteen plain text files and HP86 BASIC programs
  * You can do these examples without out my emulator but requires an extra tools and steps

## Importing ASCII and plain text as HP85 BASIC programs
  * Lets import a text file with BASIC statements into one of the emulator images amigo1.lif 
  * Turn off both the HP85 and emulator - if it is attached
    * Remove the SD Card
  * Create a plain text file with BASIC statements in it on your desktop - NOT on HP85
      * 10 DISP :HELLO WORLD"
      * 20 END
    * Save this file as TEST.txt onto the SD Card and exit your editor 
      * Unmount the SD Card "eject it" in windows jargon
      * Reinstall the SD Card in the emulator
  * Turn on the hp85disk emulator FIRST - THEN turn on the HP85
    * Trivia - the HP85 only detects disk at power on or after a RESET - therefore the emulator MUST be running first
      * Open your serial program with the documented settings

  * Lets add TEST.txt from the SD Card into the amigo1.lif emulator disk image
    * Type:
      * lif add amigo1.lif TEST TEST.txt
        * TEST is the internal LIF name, TEST.txt is you source file

  * On your HP85 we will load a binary program called GETSAVE
    * LOADBIN "GETSAVE"
      * Note: quotes are always required on HP85 BASIC file names
      * This installs GETSAVE into program memory on your HP85 until reset or power cycle
        * GETSAVE adds new functions "GET" and "SAVE" to your HP85 computer
  * On the HP85 type
    * GET "TEST"
      * Wait until you see DONE 
        * On BIG programs GET can take a very long time 
          * The slow speed is a GETSAVE limitation and not due to my emulator speed
    * Lets save it as a normal HP85 basic program
      * STORE "TESTB"
        * Saving in this format makes a totally HUGE difference in speed for BIG programs
        * In the future you can use LOAD "TESTB"

## Exporting HP85 BASIC programs to ASCII plain text
  * You must have the hp85disk emulator power ON and atteched to you HP85
    * Turn on the HP85
  * On your HP85 we will load a binary program called GETSAVE
    * LOADBIN "GETSAVE"
      * Note: quotes are always required on HP85 BASIC file names
      * This installs GETSAVE into program memory on your HP85 until reset or power cycle
        * GETSAVE adds new functions "GET" and "SAVE" to your HP85 computer
  * Lets load a normal HP85 BASIC program
    * LOAD "RWTESTB"
  * First stage conversions
    * SAVE "RWTESTA"
      * Wait until you see DONE 
        * On BIG programs SAVE can take a very long time 
          * The slow speed is a GETSAVE limitation and not due to my emulator speed
  * Lets export and convert "RWTESTA" to plain text using the emulator command prompts
    * Type:
      * lif extract amigo1.lif RWTESTA RWTEST.txt
        * TEST is the internal LIF name, TEST.txt is you source file
      * This saves the file RWTEST.txt to the SD Card and plain text ASCII
    * Later you can copy the RWTEST.txt file to your desktop
      * Turn off the HP85 and the emulator
      * Remove the SD Card
      * Attache to your desktop and copy it off

___ 



## Important notes about SD Card requirments for the emulator
  * Must be formatted FAT32
  * The HP85 is sensitive to long read/write delays that some cards can cause problems with
    * You want SD Cards with fast random writes
    * I have found that the Sandisk Extreme and Sandisk Extreme Pro cards work best.
      * There is a huge difference in various cards on the market. 
      * Look for the cards with the best 4K random write times
      * A good source of benchmark information is looking for recent Raspberry Pi SD card benchmarks because they use SD Cards
        * Specifically look at best 4k random write - faster is better.
    * Why slow SD Cards are a problem?
      * Summary: When the hp85disk emulator writes to the SD Card the Card internally must modify much much larger internal flash page - this can take too long
      * Details:
        * First step: hp85disk emulator writes to the SD Card
        * Internally the SD card finds an internal page where our data will go
        * Next the SD Card reads the page into internal RAM (recall it can be much over a megabyte)
        * Next the SD Card modifies the internal RAM with our data
        * Next the SD Card erases the page
        * Lastly the page is written to the SD Card and this takes time
          * Trivia - SD cards must to erase a page before updating because of the flash memory cell design
      * Why is this delay critical to the hp85disk emulator?
        * Too long of a delay can cause a timeout when writing to disk
        * SD cards internal hardware is mostly optimized for sequential writing 
          * Their buffers and timing are designed primarily for writing consecutive blocks one after another
            * When writing in consecutive order they can queue up many requests and combine them into one operation - a huge savings in time - done all in hardware
            * Writing to blocks in random locations break this optimization very badly
              * Therefore some SD Cards can take so long the HP85 can timeout
          * The hp85disk emulator does not have enough memory to work around this issue
            * If we could load the entire disk image into ram AND we had more for many write buffers
              * Then we could optimize the SD Card writing process to avoid the problems
            * The AVR we use has only 20K of ram for everything
              * Perhaps some day we will port this project to a Raspberry PI with add on hardware
                * The protocol is already solved for the emulator so this would not be that hard - only a battle with the time I have free

___ 


## Flashing hp85disk emulator binary files to the ATMEL atmega1284p CPU 
  * The github project includes disk images and precompiled firmeare
    * Compiled Firmware [release/build](release/build) 
    * Disk Images       [release/sdcard](release/sdcard)
    * If you wish to compile yourself please see the next section "Requirements for building..." 
  * NOTE: Most any system with an AVR Programmer that works with the atmega1284p will work
    * If you can set the fuses and read at least one encoded binary format of the firmware in the project [release](release) directory
      * You can find the current fuses in the Makefile - look for the statement wth fuses= and no preceding '#' comment
      * Extract just the fuse settings - for example: lfuse=0xd6 hfuse:=0xd9 efuse=0xff 
  * Ubuntu Linux Specific Example 
    * I am using the avrdude software package to talk to my ATMEL ICE in circuit programmer, lets install it
      * apt-get install avrdude
    * I used the ATMELICE ISP programmer
      * NOTE: avrdude supports many AVR programmers not just the one I am using
      * avrdude -P usb -p m1284p -c atmelice_isp -F -B 5 -U lfuse:w:0xd6:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m -U flash:w:release/gpib.hex
        * You should not get any errors
___ 


## Requirements for building the hp85 disk project
  * PLEASE READ! All steps below are intended ONLY REQUIRED IF YOU PLAN TO MAKE CODE CHANGES
  * [I have provided compiled files under the folder release](release)
    * You just need to flash the files

## Operating systems
  * I used Ubuntu 18.04,16.04LTS and 14.04LTS when developing the code
    * NOTE: It should be possible to setup the same build with Windows
      * There are a number of build environments for Windows that will work 
        * I hope to provide instructions soon

## Install programs to build this project - Ubuntu instructions
  * sudo bash
  * *apt-get update*
  * *apt-get install aptitude make build-essential binutils gcc*
  * *aptitude --with-recommends install python-serial minicom avr-libc avra avrdude avrdude-doc avrp binutils-avr gcc-avr gdb-avr*

## Clone my github project to your computer
  * git clone --branch V2 https://github.com/magore/hp85disk
  * cd hp85disk

## Compiling
*This is not needed if you wish to just program the release image*
  * *make clean*
  * *make*
  * *make install*
    * Installs lif and mkcfg tools

## Flashing the firmware with built in bootloader
  * *make flash-release* # do not press Enter yet!
    * OR
  * *make flash*         # do not press Enter yet!
    * NOTE: When finished *make* will call a shell script to launch a terminal program for debugging
      * These scripts are called *miniterm* or *term* in the project folder
        * The baud rate is the [Makefile](Makefile) BAUD option

## Flashing the firmware with an ISP programmer
  * Note: JTAG is disabled so we can use port C bits to control the GPIB drivers
  * You will need and AVR programmer supported by avrdude (part of avrtools)
    * I am using atmelice_isp but the [Makefile](Makefile) as example for:
    * *avrispmkII atmelice atmelice_dw atmelice_isp atmelice_pdi*
    * If you wish to another programmer then update the "flash" avrdude command line in the [Makefile](Makefile).
    * There is an example with the AVR mkii programmer as well.
  * *make flash-release* # do not press Enter yet!
    * OR
  * *make flash*         # do not press Enter yet!
    * This will use *avrdude* and your ISP to flash the firmware
    * NOTE: When finished *make* will call a shell script to launch a terminal program for debugging
      * These scripts are called *miniterm* or *term* in the project folder
        * The baud rate is the [Makefile](Makefile) BAUD option

## Building Doxygen documentation for the project - optional
  * *aptitude install --with-recommends doxygen doxygen-doc doxygen-gui doxygen-latex*
  * *If you omit this you will have to update the [Makefile](Makefile) to omit the steps*

___


## Notes concerning using the HP85 with the hp85disk emulator
  * Here we focus just on HP85 BASIC command
  * See [hpdisk.cfg](hpdisk/hpdisk.cfg) for configuration settings and setting and documentation.
     * Printer capture is configured currently for my HP54645D scope
       * The following example works for an HP85 attached to the emulator via GPIB bus.
         * PRINTER IS 705
         * PLIST
     * Disk images in [sdcard](sdcard) folder drive and configuration settings
       * First Amigo 9121D disk  at 710 for my HP85A (with 85A ROMs)
       * Second Amigo 9121D disk  at 710 for my HP85A (with 85A ROMs)
       * First SS80 HP9134L disk at 720 for my HP85B (with 85B ROMs)
       * Second SS80 HP9134L disk at 730 for my HP85B (with 85B ROMs)

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
          * The HP85 ONLY checks for drives at POWER UP or RESET

## Testing examples on your HP85
  * Testing was done with an HP85A (with extended EMS ROM) 
    * Using the Hewlett-Packard Series 80 - PRM-85 by Bill Kotaska
    * This makes my HP85A look like and HP85B 
      * I can also use the normal mass storage ROM if I limit to AMIGO drives.
	  * http://vintagecomputers.sdfeu.org/hp85/prm85.htm
       * old site http://vintagecomputers.site90.net/hp85/prm85.htm

## Initializing a disk images
## HP85B only feature or HP84A with PRM-85 board
  * The HP85B and EMS ROM has extended INITIALIZE attributes
  * IMPORTANT NOTE: this is already done for you with the hp85disk emulator
    * If you do it it this erases everything on the emulated image
    * You can however backup up copy existing LIF images to another folder on the SD Card for safe keeping
      * There is a built in copy command for this 
<pre>
  INITIALIZE "AMIGO1",":D700",14,1
  INITIALIZE "AMIGO2",":D710",14,1
  INITIALIZE "SS80-1",":D720",128,1
  INITIALIZE "SS80-2",":D730",128,1
</pre>
  
## HP85A and HP85B feature 
  * Note lines with a "#' as the first non blank character are just my comments 
    * A bad habit from writing too many bash scripts 

<pre>
  # Listing files:
  # first AMIGO
  CAT ":D700"
  # second AMIGO
  CAT ":D710"
  # first SS80
  CAT ":D720"
  # second SS80
  CAT ":D730"
  
  # Loading file from first SS80:
  LOAD "HELLO:D720"
  # Copying file between devices: fist AMIGO to second AMIGO
  COPY "HELLO:D700" TO "HELLO:D710"
  # Copying ALL files between devices: FIRST SS80 to Second SS80
  COPY ":D720" TO ":D730"
  # LOAD the GETSAVE binary program very short example
  LOADBIN "GETSAVE"
  # This program stays in memory until the HP85 is reset
  # See all of the detailed notes earlier in the README 
</pre>
  * Now on the emulator itself type
  * lif add amigo1.lif MYTEST test.txt
     * See all of the detailed notes earlier in the README 
  * We just added the TEST.txt file to the image file called amigo1.lif and named it MYTEST
<pre>
  # Lets assume amigo1.lif is defined as device :D700 in the hpdisk.cfg file
  GET "HELLO:D700"
  # Save as a HP85 BASIC file in DTA8x (type E010) file
  PUT "HELLOA:D700"
  # Store it as as HP85 BASIC BAS8x (type E020) file
  STORE "HELLO2B:D700"
  # Now in the future we can LOAD it 
  LOAD "HELLO2B:D700"
  # How to Delete the file 
  PURGE "HELLO2B:D700"
  # List the BASIC file
  LIST
  # Clear memory
  SCRATCH
</pre>

___ 
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
          * The HP85A can only use its *hard-coded firmware tables* to map ID to disk layout parameters
          * This implies that the HP85A can only use AMIGO disks it has defined in firmware.
        * The HP85B can query newer SS80 drives for detailed drive layout information instead.
        * The HP85A cannot use SS80 drives unless it uses copies of the HP85B EMS and EDISK ROMS.
            * One way this can be done with the PRM-85 expansion board offered by Bill Kotaska 
              * (The PRM-85 is great product giving you access to all of the useful ROMS)
___

## Technical Limitations
 * Multiple drive support is implements but UNIT support is NOT
 * While most AMIGO and SS80 feature have been implemented my primary focus was on the HP85A and HP85B.
   * (I do not have access to other computers to test for full compatibility)
   * This means that a few AMIGO and SS80 GPIB commands are not yet implemented!
     * Some of these are extended reporting modes - many of which are optional.
   * Note: The HP85A can only use AMIGO drives - unless you have the HP85B EMS ROM installed in your HPH9A
      * This can be done with the PRM-85 expansion board offered by Bill Kotaska (a great product!)
 * To attach a drive to our computer, real or otherwise, you must know:
   * The correct GPIB BUS address and parallel pool response (PPR) bit number your computer expects.
     * See ADDRESS, PPR and ID values in [hpdisk.cfg](hpdisk.cfg)
   * Older computers may only support AMIGO drives.
     * Such computers will have a hard coded in firmware list of drive its supports.
       * These computers will issue a GPIB BUS "request identify" command and only detect those it knows about.
       * *If these assumptions do NOT match the layout defined in the [hpdisk.cfg](sdcard/hpdisk.cfg) no drives will be detected.*
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
## HP Disk Emulator by Anders Gustafsson
<b>Anders Gustafsson was extremely helpful in getting my project started.</b>
<b>In fact I really owe the very existence of this project to his original project</b>
 * You can visit his project at this site:
   * <http://www.dalton.ax/hpdisk>
   * <http://www.elektor-labs.com/project/hpdisk-an-sd-based-disk-emulator-for-gpib-instruments-and-computers.13693.html>

He provided me his current source code code and mainy details of his project <b>which I am very thankful for.</b>
NOTE: 
 As mainly a personal exercise in fully understanding the code I ended up rewriting much of the hpdisk project. 
 I did this one part at a time as I learned the protocols and specifications.
 NOT because of any problems with his original work. 
 Although mostly rewritten I have maintained the basic concept of using  state machines for GPIB ,AMIGO and SS80 state trackings.

## The HPDir project was a vital documentation source for this project</b>
   * <http://www.hp9845.net/9845/projects/hpdir>


## My TeleDisk to LIF conversion utility
 * I used the lzss libraries and documenation by Dave Dunfield
   * Copyright 2007-2008 Dave Dunfield All rights reserved.
 * Documenation from Jean-Franois DEL NERO
   * Copyright (C) 2006-2014 Jean-Franois DEL NERO
[lif/teledisk](lif/teledisk)
 * [lif/teledisk](lif/teledisk)
   * My TELEDISK LIF extractor
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
           * TeleDisk documentation
         * See his github project
             * https://github.com/jfdelnero/libhxcfe

## FatFS
  * [fatfs](fatfs)
    * R0.12b FatFS code from (C) ChaN, 2016 - With very minimal changes 

## optiboot
  * [optiboot](optiboot)
    * Optiboot Bootloader for Arduino and Atmel AVR
    * See: https://github.com/Optiboot/optiboot
       * [GPLv2 WRT](https://github.com/Optiboot/optiboot/blob/master/LICENSE)
       * [README](https://github.com/Optiboot/optiboot/blob/master/README.md)

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
   * [HPDir HP Drive - Disk Image Manipulation](http://www.hp9845.net/9845/projects/hpdir)
     * Copyright © 2010 A. Kückes
   * [HPDrive Drive Emulators for Windows Platform](http://www.hp9845.net/9845/projects/hpdrive)
     * Copyright © 2010 A. Kückes


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




## AVR Terminal Commands
 * Pressing any key will break out of the gpib task loop until a command is entered
   * help
     * Will list all available commands and options
     * Each main option has help. Example lif help

   * For detail using LIF commands to add/extract LIF files from SD card see the top of [lif/lifutil.c](lif/lifutil.c)

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
      * Determines which files are included in the project Doxygen documents
    * [DoxygenLayout.xml](DoxygenLayout.xml)
      * Doxygen Layout file
  * Project Readme
    * [README.md](README.md)
      * Project README 
  * Project Copyright
    * [COPYRIGHT.md](COPYRIGHT.md)
      * Project Copyrights 

## Compiled firmware release files
  * [release](release)

## Board design file for version 1 and 2 hardware information
  * [board](board)
    * [V1](board/V1)
      * V1 Board documentation and Release files
      * [board design and pinouts of this project and a schematic PDF ](board/V1/HP85Disk.pdf)
      * [board design and pinouts of this project and a schematic DOC ](board/V1//HP85Disk.doc)
      * [board README.md](board/V1/HP85Disk.doc)
    * [V2/releases](V2/releases)
      * Jay Hamlin version 2 circuit board design using GPIB buffers

## Documents
  * [Documents](Documents)
  * GPIB BUS, HP device, LIF and chips documentation for this project
    * [Documents/README.md](Documents/README.md) 

## hp85disk software files
  * Most of the software in the project was written by me except where notes
  
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
    * My fatfs support utility and POSIX wrapper test functions
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
    * My GPIB code for AMIGO SS80 and PRINTER support
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
    * My CPU hardware specific code
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
    * [LCD.c](hardware/LCD.c)
    * [LCD.h](hardware/LCD.h)
      * SparkFun LCD-14072,LCD-14073,LCD-14074 support code
      * https://github.com/sparkfun/SparkFun_SerLCD_Arduino_Library
        * Modified for this project
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
    * [i2c.c](hardware/i2c.c)
    * [i2c.h](hardware/i2c.h)
      * I2C code in testing - not yet used - Copyright (c) 2014 Pieter Noordhuis https://github.com/pietern/avr-i2c
    * [user_config.h](hardware/user_config.h)
      * Main include file MMC SLOW and FATS frequency and CPU frequency settings

## Common libraries
  * [lib](lib)
    * My Library functions
    * [bcpp.cfg](lib/bcpp.cfg)
      * BCPP C code formatting tool config
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
    * My LIF disk image utilities 
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
        *  LIF image functions, directory listing and file adding. extracting,renaming,deleting
      * [td02lif.c](lif/td02lif.c)
      * [td02lif.h](lif/td02lif.h)
        * My TeleDisk to LIF translator
      * [lif-notes.txt](lif/lif-notes.txt)       
        * My notes on decoding E010 format LIF images for HP-85
      * [README.txt](lif/README.txt)
        * Notes on each file under LIF and lif/teledisk
      * [85-SS80.TD0](lif/85-SS80.TD0) from hpmuseum.org
        * Damaged SS80 Exerciser from HP Museum
      * [85-SS80.LIF](lif/85-SS80.LIF)
        * The current converter automatically did these repairs
          *  cyl 11, side 0 sector 116 mapped to 8
          *  cyl 13, side 0 sector 116 mapped to 11
          *  cyl 15, side 0 sector 10  missing - zero filled

## LIF teledisk files
   * [lif/teledisk](lif/teledisk)
     * My TELEDISK LIF extractor 
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
             * TeleDisk documentation
           * See his github project
             * https://github.com/jfdelnero/libhxcfe

## Posix wrapper - provides Linux file IO functions for Fatfs
  * [posix](posix)
    * My POSIX wrappers provide many UNIX POSIX compatible functions by translating fatfs functions 
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
    * My Printf and math IO functions
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
      * Linux bash script to build ALL the disk images
        * [create_images.sh](sdcard/create_images.sh)
          * Files from ASCII-files, LIF-files are added to all of the created images
      * All Disk definitions, address, PPR, DEBUG level for SS80 and AMIGO drives
        * [hpdisk.cfg](sdcard/hpdisk.cfg)
          * PRINTER address
      * Alternate configuration for using only AMIGO drives
        * [amigo.cfg](sdcard/amigo.cfg)
          * Use this if your system does not support SS80 drives 
             * Copy this file over the hpdisk.cfg file after renaming the hpdisk.cfg file
          * PRINTER address
      * AMIGO disk images
        * [amigo1.lif](sdcard/amigo0.lif)
          * AMIGO disk image
          * Has some demo basic programs in it
        * [amigo2.lif](sdcard/amigo1.lif)
          * AMIGO disk image
          * Has some demo basic programs in it
        * [amigo3.lif](sdcard/amigo2.lif)
          * AMIGO disk image 
          * Has some demo basic programs in it
        * [amigo4.lif](sdcard/amigo3.lif)
          * AMIGO disk image 
          * Has some demo basic programs in it
      * SS80 disk images
        * [ss80-1.lif](sdcard/ss80-0.lif)
          * SS80 disk image 
          * Has some demo basic programs in it
        * [ss80-2.lif](sdcard/ss80-1.lif)
          * SS80 disk image 
          * Has some demo basic programs in it
        * [ss80-3.lif](sdcard/ss80-2.lif)
          * SS80 disk image 
          * Has some demo basic programs in it
        * [ss80-4.lif](sdcard/ss80-3.lif)
          * SS80 disk image 
          * Has some demo basic programs in it
      * SD Card emulator configuration file backups
        * [sdcard/configs](sdcard/configs)
          * Backup copies of the hp85disk config files
      * Build Drive Configuration
        * [sdcard/mkcfg](sdcard/mkcfg)
          * Build a [hpdisk.cfg](sdcard/hpdisk.cfg) disk record 
            * Using [hpdir.ini](sdcard/notes/hpdir.ini) database
               * We can get a drive block count using: <b>mkcfg -m DRIVE -b</b>
<pre>
		mkcfg [-list]| [-m model [-b]|[-d]] [-a address]
		   -list lists all of the drives in the hpdir.ini file
		   -a disk address 0..7
		   -m model only, list hpdisk.cfg format disk configuration
		   -s short hpdisk.cfg format
		   -b only display block count, you can can use this with -m
		   -d only display computed directory block count, you can use this with -m
		   -f NAME specifies the LIF image name for this drive
</pre>

      * BUILD SCRIPTS
        * [sdcard/scripts](sdcard/scripts)
          * Scripts that help creating LIF images from multiple files
          * Used by [create_images.sh](create_images.sh)
      * My HP85 bus trace files
        * [sdcard/traces](sdcard/traces)
        * [amigo_trace.txt](sdcard/traces/amigo_trace.txt)
          * AMIGO trace file when connected to HP85 showing odd out of order command issue
        * [gpib_reset.txt](sdcard/traces/gpib_reset.txt)
          * GPIB reset trace when connected to HP85
        * [gpib_trace.txt](sdcard/traces/gpib_trace.txt)
          * GPIB transaction trace when connected to HP85
      * My HP85 plot capture files
        * [plots](sdcard/plots]
          * [plot1.plt](sdcard/plots/plot1.plt)
          * [plot2.plt](sdcard/plots/plot2.plt)
      * ASCII Basic files - in text format for easy editing
        * [ASCII-files](sdcard/ASCII-files)
          * [CIRCLE.TXT](sdcard/ASCII-files/CIRCLE.TXT)
          * [DRIVES.TXT](sdcard/ASCII-files/DRIVES.TXT)
          * [GPIB-TA.txt](sdcard/ASCII-files/GPIB-TA.txt)
          * [HELLO.TXT](sdcard/ASCII-files/HELLO.TXT)
          * [RWTEST.TXT](sdcard/ASCII-files/RWTEST.TXT)
          * [TREK85A.TXT](sdcard/ASCII-files/TREK85A.TXT)
          * [ASCII-files/TREK85](sdcard/ASCII-files/TREK85)
	        * TREK85 by Martin Hepperle, December 2015
	          * https://groups.io/g/hpseries80/topic/star_trek_game_for_hp_85/4845241
            * [author.txt](sdcard/TREK85/author.txt)  
            * [readme.txt](sdcrad/TREK85/readme.txt)	
            * [Star Trek.pdf](sdcard/TREK85/Start Trek.pdf)
            * [TREK85.BAS](sdcard/TREK85/TREK85.BAS)
            * [trek.lif](sdcard/TREK85/trek.lif)
      * LIF images with a single program in them
        * [LIF-files](sdcard/LIF-files)
          * Internal names are the same as the LIF name without extension
        * [GETSAVE.LIF](sdcard/ASCII-files/GETSAVE.LIF)
          * Adds GET and SAVE commands to an HP85
        * [GPIB-T.lif](sdcard/ASCII-files/GPIB-T.lif)
          * Simple GPIB test
        * [RWTESTB.lif](sdcard/ASCII-files/RWTESTB.lif)
          * Reads,Writes and Purge tests
        * [TREK85B.lif](sdcard/ASCII-files/TREK85B.lif)
	      * TREK85 by Martin Hepperle, December 2015
	        * https://groups.io/g/hpseries80/topic/star_trek_game_for_hp_85/4845241
      * LIF images with multiple programs in them
        * [LIF-volumes](sdcard/LIF-volumes)
          * [85-SS80.LIF](sdcard/ASCII-files/85-SS80.LIF)
      * GETSAV documentation
        * [notes](sdcard/notes)
          * GETSAVE can be loaded on an HP85 to GET and SAVE Basic text files
            * NOTE: my lif utilities can translate between ASCII files and files in GET/SAVE format 
        * Various notes 
___
