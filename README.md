\section README


# Documentation

 * Please use this link for the Documentation and this README
   * https://rawgit.com/magore/hp85disk/master/doxygen/html/index.html

## HP85 Disk Emulator Copyright (C) 2014-2019 Mike Gore board design by Jay Hamlin 2018-2019
 * See [COPYRIGHT](COPYRIGHT.md) for a full copyright notice for the project

## HP85 emulator circuit board developed by Jay Hamlin
## V2 notes - the V2 branch targes new hardware - the current code has NOT been updated yet

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
       * See the top of lif/lifutils.c for full documentation and examples.
     * TeleDisk to LIF extractor tool included - see lif directory README.md
       * td02lif 85-SS80.TD0 85-SS80.LIF
___
## OS Requirements for software building
  * I used Ubuntu 16.04LTS and 14.04LTS when developing the code
    * It should be easy to setup the same build with Windows gcc tools.

## Install Ubuntu Packages required for Building
  * sudo bash
  * *apt-get update*
  * *apt-get install aptitude make build-essential binutils gcc*
  * *aptitude --with-recommends install minicom avr-libc avra avrdude avrdude-doc avrp binutils-avr gcc-avr gdb-avr*

## Compiling AVR and standlone LIF tools
  * *make clean*
  * *make*

## Flashing AVR Firmware 
  * You will need and AVR programmer supported by avrdude (avrdude is part of avrtools installed in last step)
    * I am using atmelice_isp but the Makefile as example for:
      * *avrispmkII atmelice atmelice_dw atmelice_isp atmelice_pdi*
  * make flash

## Flashing the firmware to the AVR with avrdude and programmer
  * *make flash*
    * This will use *avrdude* with the new low cost Atmel ICE programmer.
      * If you wish to another programmer then update the "flash" avrdude command line in the Makefile.
      * There is an example with the AVR mkii programmer as well.

## Building Doxygen documentation for the project - optional
  * *aptitude install --with-recommends doxygen doxygen-doc doxygen-gui doxygen-latex*
  * *If you omit this you will have to update the Makefile to omit the steps*
___

## Using the emulator with provider examples
   * See sdcard.cfg for configuration settings and setting and documentation.
     * Printer capture is configured currently for my HP54645D scope
       * The following example works for an HP85 attached to the emulator via GPIB bus.
         * PRINTER IS 705
         * PLIST
     * Disk images in SDCARD folder drive and configuration settings
       * First SS80 HP9134L disk at 700 for my HP85A (with 85A ROMs)
       * First Amigo 9121D disk  at 710 for my HP85B (with 85B ROMs)
       * Second SS80 HP9134L disk at 720 for my HP85A (with 85A ROMs)
       * Second Amigo 9121D disk  at 730 for my HP85B (with 85B ROMs)
     * How to use the examples with your HP85
       * Copy the files inside the project SDCARD folder to the home folder of a fat32 formatted drive
         * All image files and configuration must be in the home folder only - not in a subdirectory.
         * You may store other user files in sub folders of your choosing.
       * Verify hpdisk.cfg configuration settings for your computer
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
      * The PPR keyword in the hpdisk.cfg is the PPR bit the drive uses
        * PPR of 0 = PPR response on GPIB data bus bit number 8 - as per GPIB BUS specifications.
      * The ID keyword in hpdisk.cfg is the 16 bit reply to "Request Identify Reply"
        * IMPORTANT! AMIGO drives cannot be queried for detailed drive layout information
          * The HP85A can only use its *hardcoded firmware tables* to map ID to disk layout parameters
          * This implies that the HP85A can only use AMIGO disks it has defined in firmware.
        * The HP85B can query newer SS80 drives for detailed drive layout information instead.
        * The HP85A cannot use SS80 drives unless it uses copies of the HP85B EMS and EDISK ROMS.
            * One way this can be done with the PRM-85 expansion board offered by Bill Kotaska 
              * (The PRM-85 is great product giving you access to all of the useful ROMS)
___
## Limitations
 * Multiple UNIT support is NOT yet implemented however multiple drive support is..
 * While most AMIGO and SS80 feature have been implemented my primary focus was on the HP85A and HP85B.
   * (I do not have access to other computers to test for full compatibility)
   * This means that a few AMIGO and SS80 GPIB commands are not yet implemented!
     * Some of these are extended reporting modes - many of which are optional.
   * Note: The HP85A can only use AMIGO drives - unless you have the HP85B EMS ROM installed in your HPH9A
      * This can be done with the PRM-85 expansion board offered by Bill Kotaska (a great product!)
 * To attach a drive to our computer, real or otherwise, you must know:
   * The correct GPIB BUS address and parallel pool response (PPR) bit number your computer expects.
     * See ADDRESS, PPR and ID values in SDCARD/hpdisk.cfg
   * Older computers may only support AMIGO drives.
     * Such computers will have a hard coded in firmware list of drive its supports.
       * These computers will issue a GPIB BUS "request identify" command and only detect those it knows about.
       * *If these assumptions do NOT match the layout defined in the hpdisk.cfg no drives will be detected.*
   * Newer computers with SS80 support can request fully detailed disk layout instead of the "request identify"
   * My emulator supports both reporting methods - but your computer may not use them both!
     * For supported values consult your computer manuals or corresponding drive manual for your computer.
       * See gpib/drives_parameters.txt for a list on some known value (CREDITS; these are from the HPDir project)
     * In all cases the hpdisk.cfg parameters MUST match these expectations.
   * The hpdisk.cfg file tells the emulator how the emulated disk is defined.
     * GPIB BUS address, Parallel Poll Response bit number and AMIGO Request Identify response values.
     * Additional detail for SS80 drives that newer computers can use.
     * In ALL cases the file informs the code what parameters to emulate and report.
       * ALL of these values MUST match your computers expectations for drives it knows about.
   * Debugging
     * You can enable reporting of all unimplemented GPIB commands (see *TODO* debug option in hpdisk.cfg)
       * Useful if you are trying this on a non HP85 device
       * See the SDCARD/hpdisk.cfg for documentation on the full list of debugging options
     * The emulator can passively log all transactions between real hardware on the GPIB bus 
       * Use the "gpib trace *logfile*" command - pressing any key exits - no emulation is done in this mode.
       * You can use this to help understand what is sent to and from you real disks.
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

<b>hp85disk/lif/teledisk </b>
 * lif/teledisk
   * My TELEDISK LIF extracter
   * Important Contributions (My converted would not have been possible without these)
     * Dave Dunfield, LZSS Code and TeleDisk documentation
       * Copyright 2007-2008 Dave Dunfield All rights reserved.
       * td0_lzss.h
       * td0_lzss.c
         * LZSS decoder
       * td0notes.txt
         * Teledisk Documentation
     * Jean-Franois DEL NERO, TeleDisk Documentation
       * Copyright (C) 2006-2014 Jean-Franois DEL NERO
         * wteledsk.htm
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
   * See Documents folder

___
## Enhanced version of Tony Duell's lif_utils by Joachim
   * <https://github.com/bug400/lifutils>
   * Create/Modify LIF images

___
## CS80 References: ("CS80" is the short form used in the project)
   * "CS/80 Instruction Set Programming Manual"
   * Printed: APR 1983
   * HP Part# 5955-3442
   * See Documents folder

___
## Amigo References: ("A" or "Amigo" is the short form used in the project)
   * "Appendix A of 9895A Flexible Disc Memory Service Manual"
   * HP Part# 09895-90030
   * See Documents folder

___
## HP-IB
   * ("HP-IB" is the short form used in the project)
   * "Condensed Description of the Hewlett Packard Interface Bus"
   * Printed March 1975
   * HP Part# 59401-90030
   * See Documents folder

___
## Tutorial Description of the Hewlett Packard Interface Bus
   * ("HP-IB Tutorial" is the short form used in the project)
   * <http://www.hpmemory.org/an/pdf/hp-ib_tutorial_1980.pdf>
   * Printed January 1983
   * <http://www.ko4bb.com/Manuals/HP_Agilent/HPIB_tutorial_HP.pdf>
   * Printed 1987
   * See Documents folder

___
## GPIB / IEEE 488 Tutorial by Ian Poole
    * <http://www.radio-electronics.com/info/t_and_m/gpib/ieee488-basics-tutorial.php>
   * See Documents folder

___
## HP 9133/9134 D/H/L References
   * "HP 9133/9134 D/H/L Service Manual"
   * HP Part# 5957-6560
   * Printed: APRIL 1985, Edition 2
   * See Documents folder
___

## LIF File system Format
   * <http://www.hp9845.net/9845/projects/hpdir/#lif_filesystem>
   * See Documents folder
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


___

## AVR ATMEGA1284P pin assignments for HP85 Disk
  * @see Documents/HP85Disk.pdf for a hand drawn diagram
  * GPIB:  Each GPIB pin (8 data and 8 control lines ) attach to CPU via 120 ohm current limit resistor .
    * Each GPIB connector pin (8 data and 8 control lines) have a 10K pull-up resistor to VCC.
  * ISP header: MOSI,MISO,SCK,/Reset connects directly to ISP header
  * Micro SD Interface: MOSI,MISO,SCK attach to CPU function via a 1k series resistor.
    * Micro SD interface has level shifters and internal 5V to 3.3V regulator
    * PB3 /CS must have a 10K pull-up up to VCC to prevent access during ISP programming
    * PB4 should have a 10K pull up help assure the SPI bus does not go into slave mode.
  * RS232 TTL: connect to FTDI232 USB  board which also provides 5V VCC power to all circuits..
  * I2C: SCL,SDA connect to optional DS1307 RTC board with each line having a 2k2 pull-up
<pre>

                       ATMEGA1284P (and ATMEGA644P) 
                       +---V---+ 
     5 EOI INT0  PB0  1|       |40  PA0      D1  1 
     6 DAV INT1  PB1  2|       |39  PA1      D2  2 
       PP  INT2  PB2  3|       |38  PA2      D3  3 
    SD /CS  PWM  PB3  4|       |37  PA3      D4  4 
       NC   PWM  PB4  5|       |36  PA4      D5 13 
    SD     MOSI  PB5  6|       |35  PA5      D6 14 
    SD     MISO  PB6  7|       |34  PA6      D7 15 
    SD      SCK  PB7  8|       |33  PA7      D8 16 
    10K pull-up  /RST  9|       |32  AREF     0.1uf 
       +5        VCC 10|       |31  GND      GND   
       GND       GND 11|       |30  AVCC     +5    
    20MHZ      XTAL2 12|       |29  PC7      NC    
    20MHZ      XTAL1 13|       |28  PC6      NC    
       RX   RX0  PD0 14|       |27  PC5  TDI JTAG 
       TX   TX0  PD1 15|       |26  PC4  TDO JTAG 
     7 NRFD RX1  PD2 16|       |25  PC3  TMS JTAG 
     8 NDAC TX1  PD3 17|       |24  PC2  TCK JTAG 
     9 IFC  PWM  PD4 18|       |23  PC1  SDA I2C   
    10 SRQ  PWM  PD5 19|       |22  PC0  SCL I2C  
    11 ATN  PWM  PD6 20|       |21  PD7  PWM REN 17 
                       +-------+ 
</pre>




___ 

## Parallel Poll Response circuit
  * Uses: Three chips 74HC05, 74HC32, 74HC595
  * Parallel Poll Response must be less than 2 Microseconds therefore we use hardware to do it!
  * @see Documents/HP85Disk.pdf for a hand drawn diagram


<pre>
    ATMEGA               HC595             HC05 
                      +----V----+          +-V-+  
    3 PB2 -------- 12 |RCLK   Q0| 15 -x- 1 |   | 2 --- GPIB D8 
    6 MOSI ------- 14 |SER    Q1| 1  -x- 3 |   | 4 --- GPIB D7 
    8 SCK -------- 11 |SRCLK  Q2| 2  -x- 5 |   | 6 --- GPIB D6 
    9 IFC -------- 10 |SRCLR  Q3| 3  -x- 9 |   | 8 --- GPIB D5 
           HC32       |         |     |    |   | 7 GND 
          +-V-+       |         |     |    |   |14 VCC 
     EOI 2|   |       |         |     |    +---+ 
     ATN 1|   |       |         |     \--- each line has its own 
          |   | 3--13 |/OE      |          10K resistor to GND 
    VCC 14|   |       |         | 16 VCC 
    GND  7|   |       |         |  8 GND 
          +---+       +---------+ 
</pre>

Notes: When both EOI and ATN are low the HC32 enables HC595 outputs
  * If any HC595 output is high the GPIB bus bit will be pulled low
  * IFC low resets the HC595 outputs low - so the HC05 outputs will float.

___ 

## Testing
  * Testing was done with an HP85A (with extended EMS ROM) 
    * Using the Hewlett-Packard Series 80 - PRM-85 by Bill Kotaska
    * This makes my HP85A look like and HP85B 
      * I can also use the normal mass storage ROM if I limit to AMIGO drives.
      * http://vintagecomputers.site90.net/hp85/prm85.htm

  * Note: the EMS ROM has extended INITIALIZE attributes
<pre>
  #Initializing: (already done on these images so you do not have to)
  INITIALIZE "SS80-1",":D700",128,1
  INITIALIZE "AMIGO1",":D710",14,1
  INITIALIZE "SS80-2",":D720",128,1
  INITIALIZE "AMIGO2",":D730",14,1
  
  #Listing files:
  #first SS80
  CAT ":D700"
  #first AMIGO
  CAT ":D710"
  #second SS80
  CAT ":D720"
  #second AMIGO
  CAT ":D730"
  
  #Loading file from second SS80:
  LOAD "HELLO:D720"
  #Copying file between devices: fist AMIGO to second AMIGO
  COPY "HELLO:D710" TO "HELLO:D730"
  #Copying ALL files between devices: FIRST SS80 to Second SS80
  COPY ":D700" TO ":D720"
</pre>
___ 

## AVR Terminal Commands
 * Pressing any key will break out of the gpib task loop until a command is entered
   * help
      Will list all available commands and options

   * For detail using LIF commands to add/extract LIF files from SD card see the top of lif/lifutil.c
___ 


## Files
  * ./COPYRIGHT.md
    Project Copyrights 
  * ./main.c
    Main start-up code
  * ./main.h
    Main start-up code
  * ./notes.txt
    Note - working on converting compiled constants into run time configuration
  * ./README.md
    This file

  * Documents
    * 59401-90030_Condensed_Description_of_the_Hewlett-Packard_Interface_Bus_Mar75-ocr.pdf
      * CONDENSED DESCRIPTION OF THE HEWLETT·PACKARD INTERFACE BUS
    * 5955-3442_cs80-is-pm-ocr.pdf
      * CS/80 INSTRUCTION SET
    * 5957-6560_9133_9134_D_H_L_Service_Apr88.pdf
      * HP 9133/9134 D/H/L Service Manual
    * 5957-6584_9123D_3.5_Flex_Disc_Nov85.pdf
      * UPDATE FOR THE 3 1/2-INCH FLEXIBLE DISC DRIVE SERVICE MANUAL (PART NUMBER 09121-90030
    * 5958-4129_SS80_Nov-1985-ocr.pdf
      * SUBSET 80 FOR FIXED AND FLEXIBLE DISC DRIVES (HP-IB IMPLEMENTATION)
    * amigo-command-set-ocr.pdf
      * Appendix A HP 9895A Disc Memory Command Set
    * CIB24SRA.pdf
      * GPIB connector diagram of the part we used in this project form L-COM 
    * CIB24SRA.step
      * GPIB connector design file of the part we used in this project form L-COM 
    * GPIB protocol.pdf
      * Copy of GPIB commands and pinout from Linux GPIB project
      * See: http://linux-gpib.sourceforge.net
    * handshake.pdf
      * Highlighted excerpt of just the 3 wire GPIB handshake
    * HANDSHAKING.pdf
      * Highlighted full version of 3 wire GPIB handshake by Ian Poole
    * HP85Disk.pdf
      * Detailed pinouts of this project and a schematic
    * HP9133AB-09134-90032-Aug-1983.pdf
      * HPs 5 1/4-Inch Winchester Disc Drive Service Documentation - HP 9133A/8, 9134A/B, and 9135A
    * HP913x.pdf
      * HP 9133A/B, 9134A/B, and 9135A Disc Memory Users Manual
    * hp-ib_tutorial_1980.pdf
      * Tutorial Description of the Hewlett-Packard Interface Bus
    * HPIB_tutorial_HP.pdf
      * Tutorial Description of the Hewlett-Packard Interface Bus
    * IEEE-488_Wikipedia_offline.pdf
      * Offline copy of Wikipedia GPIB article
    * README.md 
      * Description of file under the Documents folder

  * fatfs
    * R0.12b FatFS code from (C) ChaN, 2016 - With very minimal changes 
    * 00history.txt
    * 00readme.txt
    * ff.c
    * ffconf.h
    * ff.h
    * integer.h

  * fatfs.hal
    * R0.12b FatFS code from (C) ChaN, 2016 with changes
    * Hardware abstraction layer based on example AVR project
    * diskio.c
      * Low level disk I/O module glue functions (C)ChaN, 2016 
    * diskio.h
      * Low level disk I/O module glue functions (C)ChaN, 2016 
    * mmc.c
      * Low level MMC I/O by (C)ChaN, 2016 with modifications
    * mmc.h
      * Low level MMC I/O by (C)ChaN, 2016 with modifications
    * mmc_hal.c
      * My Hardware abstraction layer code
    * mmc_hal.h
      * My Hardware abstraction layer code

  * fatfs.sup
    * Support utility and POSIX rapper factions
    * fatfs.h
      * FatFS header files
    * fatfs_sup.c
    * fatfs_sup.h
      * FatFS file listing and display functions
    * fatfs_tests.c
    * fatfs_tests.h
      * FatFS user test functions

  * gpib
    * My GPIB code for AMIGO SS80 and PPRINTER support
    * amigo.c
      * AMIGO parser
    * amigo.h
      * AMIGO parser
    * defines.h
      * Main GPIB header and configuration options
    * drives.c
      * Supported Drive Parameters 
    * drive_references.txt
      * General Drive Parameters Documentation for all known drive types
    * format.c
      * LIF format and file utilities
    * gpib.c
      * All low level GPIB bus code
    * gpib.h
      * GPIB I/O code
    * gpib_hal.c
      * GPIB hardware abstraction code
    * gpib_hal.h
      * GPIB hardware abstraction code
    * gpib_task.c
      * GPIB command handler , initialization and tracing code
    * gpib_task.h
      * GPIB command handler , initialization and tracing code
    * gpib_tests.c
      * GPIB user tests
    * gpib_tests.h
      * GPIB user tests
    * printer.c
      * GPIB printer capture code
    * printer.h
      * GPIB printer capture code
    * references.txt
      * Main S80 SS80 AMIGO and GPIB references part numbers and web links
    * ss80.c
      * SS80 parser
    * ss80.h
      * SS80 parser

  * hardware
    * CPU hardware specific code
    * baudrate.c
      * Baud rate calculation tool. Given CPU clock and desired baud rate, will list the actual baud rate and registers
    * bits.h
      * BIT set and clear functions
    * cpu.h
      * CPU specific include files
    * delay.c
      * Delay code
    * delay.h
      * Delay code
    * hal.c
      * GPIO functions, spi hardware abstraction layer and chip select logic
    * hal.h
      * GPIO functions, spi hardware abstraction layer and chip select logic
    * iom1284p.h
      * GPIO map for ATEMEGA 1284p
    * mkdef.c
      * Not used
    * pins.txt
      * AVR function to GPIO pin map
    * ram.c
      * Memory functions
    * ram.h
      * Memory functions
    * rs232.c
      * RS232 IO
    * rs232.h
      * RS232 IO
    * rtc.c
      * DS1307 I2C RTC code
    * rtc.h
      * DS1307 I2C RTC code
    * spi.c
      * SPI BUS code
    * spi.h
      * SPI BUS code
    * TWI_AVR8.c
      * I2C code LUFA Library Copyright (C) Dean Camera, 2011.
    * TWI_AVR8.h
      * I2C code LUFA Library Copyright (C) Dean Camera, 2011.
    * user_config.h
      * Main include file MMC SLOW and FATS frequency and CPU frequency settings

  * lib
    * Library functions
    * bcpp.cfg
      * BCPP C code formatter config
    * matrix.c
      * Matrix code - not used
    * matrix.h
      * Matrix code - not used
    * matrix.txt
      *  A few notes about matrix operations
    * queue.c
      * Queue functions
    * queue.h
      * Queue functions
    * sort.c
      * Sort functions - not used
    * sort.h
      * Sort functions - not used
    * stringsup.c
      * Various string processing functions
    * stringsup.h
      * Various string processing functions
    * time.c
      * POSIX time functions
    * time.h
      * POSIX time functions
    * timer.c
      * Timer task functions
    * timer.h
      * Timer task functions
    * timer_hal.c
      * Timer task hardware abstraction layer
    * timetests.c
      * Time and timer test code

  * lif
    * LIF disk image utilities 
    * lif/lifutils.c
    * lif/lifutils.c
      * Functions that allow the emulator to import and export files from LIF images 
    * Makefile
      * Permits creating a standalone Linux version of the LIF emulator tools
    * Code by Mike Gore
      * Makefile 
        * Make stand alone Linux versions of LIF utility and optionaly TeleDisk to LIF converter
      * lifsup.c
      * lifsup.h
        * Stand alone libraries for LIF and TELEDISK utility 
          * These functions are also part of various hp85disk libraries
      * lifutils.c
      * lifutils.h
        *  LIF image functions, directory listing and file adding.extracting,renaming,deleting
      * td02lif.c
      * td02lif.h
        * My TeleDisk to LIF translator
      * lif-notes.txt       
        * My notes on decoding E010 format LIF images for HP-85
      * README.txt
        * Notes on each file under lif and lif/teledisk
      * 85-SS80.TD0 from hpmuseum.org
        * Damaged SS80 Excersizer from HP Musium
      * 85-SS80.LIF         
        * The current converter automaticcal did these repairs
          *  cyl 11, side 0 sector 116 mapped to 8
          *  cyl 13, side 0 sector 116 mapped to 11
          *  cyl 15, side 0 sector 10  missing - zero filled

   * lif/teledisk       
     * My TELEDISK LIF extracter 
       * Note: The TeleDisk image MUST contain a LIF image  - we do NOT translate it
     * README.txt
       * Credits
     * Important Contributions (My converted would not have been possible without these)
       * Dave Dunfield, LZSS Code and TeleDisk documentation
         * Copyright 2007-2008 Dave Dunfield All rights reserved.
         * td0_lzss.h
         * td0_lzss.c
           * LZSS decoder
         * td0notes.txt
           * Teledisk Documentation
       * Jean-Franois DEL NERO, TeleDisk Documentation 
         * Copyright (C) 2006-2014 Jean-Franois DEL NERO
           * wteledsk.htm
             * TeleDisk documenation
           * See his github project
             * https://github.com/jfdelnero/libhxcfe

  * posix
    * POSIX wrappers provide many UNIX POSIX compatible functions by translating fatfs functions 
    * posix/posix.c
    * posix/posix.h
      * POSIX wrappers for fatfs - Unix file IO function call wrappers
    * posix/posix_testsc
      * POSIX user tests

  * printf
    * Printf and math IO functions
    * printf/mathio.c
      * Number conversions 
    * printf/mathio.h
      * Number conversions 
    * printf/n2a.c
      * Binary to ASCII converter number size only limited by memory
    * printf/printf.c
      * My small printf code - includes floating point support and user defined low character level IO
    * printf/sscanf.c
      * My small scanf code - work in progress
    * printf/test_printf.c
      * Test my printf against glibs 1,000,000 tests per data type

  * sdcard
    * My HP85 AMIGO and SS80 disk images
      * SDCARD/hpdisk.cfg
        * All Disk definitions, address, PPR, DEBUG level for SS80 and AMIGO drives
        * PRINTER address
      * SDCARD/amigo.lif
        * AMIGO disk image file number 1
        * Has some demo basic programs in it
      * SDCARD/amigo-2.lif
        * AMIGO disk image file number 2
        * Has some demo basic programs in it
      * SDCARD/ss80.lif
        * SS80 hard drive disk image file number 1
        * Has some demo basic programs in it
      * SDCARD/ss80-2.lif
        * SS80 hard drive disk image file number 2
        * Has some demo basic programs in it
    * My HP85 bus trace files
      * SDCARD/amigo_trace.txt
        * AMIGO trace file when connected to HP85 showing odd out of order command issue
      * SDCARD/gpib_reset.txt
        * GPIB reset trace when connected to HP85
      * SDCARD/gpib_trace.txt
        * GPIB transaction trace when connected to HP85
    * My HP85 plot capture files
        * SDCARD/plot1.plt
        * SDCARD/plot2.plt
    * TREK85/
	  * TREK85 by Martin Hepperle, December 2015
	  * https://groups.io/g/hpseries80/topic/star_trek_game_for_hp_85/4845241
        * author.txt  
        * readme.txt	
        * Star Trek.pdf  
        * TREK85.BAS  
        * trek.lif

___
