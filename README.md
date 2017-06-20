\section README


# Documentation

 * Pleasse use this link for Documentation and this README
   * https://rawgit.com/magore/hp85disk/master/doxygen/html/index.html

## HP85 Disk Emulator Copyright &copy; 2014-2017 Mike Gore
 * See [COPYRIGHT](COPYRIGHT.md) for a full copywrite notice for the project

## Features
 * This project emulates GPIB drives and HPGL printer for HP85A and HP85B
   * AMIGO drices work with HP85A 
   * SS80 drives work with HP85B (or HP85A with prm-85 add on board see links)
   * Each drive can be fully defines in the hpdisk.cfg file on the SD CARD
   * Printer can capture data sent to GPIB printer to a file
     * Can also capture programs from the HP85:
       * PRINTER IS 705
       * PLIST
   * Working example sdcard files included
     * first SS80 HP9134L disk at 700 for my HP85A (with 85A roms)
     * first Amigo 9121D disk  at 710 for my HP85B (with 85B roms)
     * second SS80 HP9134L disk at 720 for my HP85A (with 85A roms)
     * second Amigo 9121D disk  at 730 for my HP85B (with 85B roms)
     * Printer capture at 750 for my HP54645D scope
       * Will capture plot data to sdcard with timestamp
___

# Credits

<b>I really owe the very existence of this project to the original work done by Anders Gustafsson and his great "HP Disk Emulator" </b>
 * You can visit his project at this site:
   * <http://www.dalton.ax/hpdisk>
   * <http://www.elektor-labs.com/project/hpdisk-an-sd-based-disk-emulator-for-gpib-instruments-and-computers.13693.html>

 <b>Anders Gustafsson was extremely helpful in providing me his current 
 code and details of his project - which I am very thankful for.</b>

 As mainly a personal excercise in fully understanding the code I 
 ended up rewriting much of the hpdisk project. I did this one part at a 
 time as I learned the protocols and specifications - NOT because of any 
 problems with his original work. 

 I Although mostly rewritten I have maintained the basic concept of using 
 state machines for GPIB read and write functions as well as for SS80 execute 
 state tracking. 

___
# Abbreviations
Within this project I have attempted to provide detailed referces 
to to manuals, listed below.  I have included short quotes and 
section and page# reference to these works.
 * <b>SS80</b>
 * <b>CS80</b>
 * <b>A or Amigo</b>
 * <b>HP-IP</b>
 * <b>HP-IP Tutorial</b>

## Documentation References and related sources of information
 * Web Resources
   * <http://www.hp9845.net>
   * <http://www.hpmuseum.net>
   * <http://www.hpmusuem.org>
   * <http://bitsavers.trailing-edge.com>
   * <http://en.wikipedia.org/wiki/IEEE-488>
   * See Documents folder

## Enhanced version of Tony Duell's lif_utils by Joachim
   * <https://github.com/bug400/lifutils>
   * Create/Modify LIF images

## CS80 References: ("CS80" is the short form used in the project)
   * "CS/80 Instruction Set Programming Manual"
   * Printed: APR 1983
   * HP Part# 5955-3442
   * See Documents folder

## Amigo References: ("A" or "Amigo" is the short form used in the project)
   * "Appendix A of 9895A Flexible Disc Memory Service Manual"
   * HP Part# 09895-90030
   * See Documents folder

## HP-IB
   * ("HP-IB" is the short form used in the project)
   * "Condensed Description of the Hewlett Packard Interface Bus"
   * Printed March 1975
   * HP Part# 59401-90030
   * See Documents folder

## Tutorial Description of The Hewllet Packard Interface Bus
   * ("HP-IB Tutorial" is the short form used in the project)
   * <http://www.hpmemory.org/an/pdf/hp-ib_tutorial_1980.pdf>
   * Printed January 1983
   * <http://www.ko4bb.com/Manuals/HP_Agilent/HPIB_tutorial_HP.pdf>
   * Printed 1987
   * See Documents folder

## GPIB / IEEE 488 Tutorial by Ian Poole
    * <http://www.radio-electronics.com/info/t_and_m/gpib/ieee488-basics-tutorial.php>
   * See Documents folder

## HP 9133/9134 D/H/L References
   * "HP 9133/9134 D/H/L Service Manual"
   * HP Part# 5957-6560
   * Printed: APRIL 1985, Edition 2
   * See Documents folder

## LIF Filesystem Format
   * <http://www.hp9845.net/9845/projects/hpdir/#lif_filesystem>
   * See Documents folder

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
    * PB3 /CS must have a 10K pullup up to VCC to prevent access durring ISP programming
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
    10K pullup  /RST  9|       |32  AREF     0.1uf 
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
  * Parallel Poll Response must be less then 2 Microseconds therefore we use hardware to do it!
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
  * Testing was done with an HP85A (with extended EMS rom) 
    * Using the Hewlett-Packard Series 80 - PRM-85 by Bill Kotaska
    * This makes my HP85A look like and HP85B 
      * I can also use the normal mass storage rom if I limit to AMIGO drives.
      * http://vintagecomputers.site90.net/hp85/prm85.htm

  * Note: the EMS rom has extended INITIALIZE attributes
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
 * Pressing any key will break out of the gpib task loop untill a command is entered
   * help
      Will list all avalable commands and options

   * Example commands
      * lifadd
        * Notes:
          * Only adds ASCII type E010 files at this time
          * Strings must be no longer then sector size - 3
          * Trailing "\n" and/or "\r" are coverted to "\n" when stored in LIF file
      <pre>
         # Adds a translated ASCII file stored on SD card into a LIF disk image on SD card
         # Used to import files into the HP85 disk images
         lifadd /amigo1.lif TEST1 /test.bas
         lifadd /amigo1.lif TREK85 /TREK85/TREK85.BAS
      </pre>
         * Notes about TREK85
           * Author: TREK85 port was done by Martin Hepperle
           * https://groups.io/g/hpseries80/topic/star_trek_game_for_hp_85/4845241
             * To load on HP85: GET "TREK85:D710"
               * Note: GET takes a LONG time > 10 minutes!
             * Assumes /amigo1.lif is ":D710"
               * Intial RUN startup time is > 40 seconds
             * Once STORE is used to save the load and run times are fast
      * lifcreate
      <pre>
         # formats an LIF image file with 15 directory sectors and a length of 1120 (16 * 2 * 35) sectors
         format /amigo3.lif AMIGO3 15 1120
         Formating LIF image:[/amigo3.lif], Label:[AMIGO3], Dir Sectors:[15], sectors:[1120]
         Formating: wrote:[1120] sectors
      </pre>

      * lifdir 
      <pre>
        lifdir /amigo1.lif
        Volume: [AMIGO2]
        NAME         TYPE   START SECTOR        SIZE    RECSIZE
        HELLO       E020h            10h         323        256
        CIRCLE      E020h            12h         156        256
        GPIB-S      E020h            13h         338        256
        GPIB-T      E020h            15h        1413        256
        GPIB7       E020h            1Bh         197        256
        AMIGO2      E020h            1Ch          51        256
        HELLO2      E010h            1Dh         512        256
        HELLO3      E010h            1Fh         512        256
        TEST        E010h            21h         256        256
        TREK85      E010h            22h       28160        256
        
              10 Files
               0 Purged
             128 Used sectors
             976 Free sectors
             144 First free sector (90h)
      </pre>
      * lifextract
        * Notes:
          * Only extracts ASCII type E010 files at this time
          * Strings must be no longer then sector size - 3
          * Trailing "\n" and/or "\r" are coverted to "\n" when stored in SD card file
      <pre>
         # extracts an ASCII type E010 file from a LIF image and saves it on the SD card
         lifextract /amigo1.lif HELLO3 /HELLO3.BAS
         Extracting: /HELLO3.BAS
         Wrote:      311
      </pre>
___ 

## OS Requirements for software building
  * I use *Ubuntu 16.04 and 14.04* so these instruction will cover that version
  * It should be easy to setup the same build with Windows gcc tools.


## Ubuntu 16.04LTS and 14.04LTS install and setup notes
  * *apt-get update*
  * *apt-get install aptitude*
  * *aptitude install --with-recommends avr-gcc avr-libc binutils-avr gdb-avr avrdude*


## Building Doxygen documenation for the project - optional
  * *aptitude install --with-recommends doxygen doxygen-doc doxygen-gui doxygen-latex*
  * *If you omit this you will have to update the Makefile to omit the steps*


## Compiling the firmware
  * *make clean*
  * *make*


## Flashing the firmware to the AVR with avrdude and programmer
  * *make flash*
    * This will use *avrdude* with the new low cost Atmel ICE programmer.
      * If you wish to another programmer then update the "flash" avrdude command line in the Makefile.
      * There is an example with the AVR mkii programer as well.

## Files
  * ./COPYRIGHT.md
    Project Copyrights 
  * ./main.c
    Main startup code
  * ./main.h
    Main startup code
  * ./notes.txt
    Note - working on converting compiled constants into run time configuration
  * ./README.md
    This file
  * Documents/59401-90030_Condensed_Description_of_the_Hewlett-Packard_Interface_Bus_Mar75-ocr.pdf
    * CONDENSED DESCRIPTION OF THE HEWLETT·PACKARD INTERFACE BUS
  * Documents/5955-3442_cs80-is-pm-ocr.pdf
    * CS/80 INSTRUCTION SET
  * Documents/5957-6560_9133_9134_D_H_L_Service_Apr88.pdf
    * HP 9133/9134 D/H/L Service Manual
  * Documents/5957-6584_9123D_3.5_Flex_Disc_Nov85.pdf
    * UPDATE FOR THE 3 1/2-INCH FLEXIBLE DISC DRIVE SERVICE MANUAL (PART NUMBER 09121-90030
  * Documents/5958-4129_SS80_Nov-1985-ocr.pdf
    * SUBSET 80 FOR FIXED AND FLEXIBLE DISC DRIVES (HP-IB IMPLEMENTATION)
  * Documents/amigo-command-set-ocr.pdf
    * Appendix A HP 9895A Disc Memory Command Set
  * Documents/CIB24SRA.pdf
    * GPIB connector diagram of the part we used in this project form L-COM 
  * Documents/CIB24SRA.step
    * GPIB connector design file of the part we used in this project form L-COM 
  * GPIB protocol.pdf
    * Copy of GPIB commands and pinout from Linux GPIB project
    * See: http://linux-gpib.sourceforge.net
  * Documents/handshake.pdf
    * Highlighted excerpt of just the 3 wire GPIB handshake
  * Documents/HANDSHAKING.pdf
    * Highlighted full version of 3 wire GPIB handshake by Ian Poole
  * Documents/HP85Disk.pdf
    * Detailed pinouts of this project and a schematic
  * Documents/HP9133AB-09134-90032-Aug-1983.pdf
    * HPs 5 1/4-Inch Winchester Disc Drive Service Documentation - HP 9133A/8, 9134A/B, and 9135A
  * Documents/HP913x.pdf
    * HP 9133A/B, 9134A/B, and 9135A Disc Mentory Users Manual
  * Documents/hp-ib_tutorial_1980.pdf
    * Tutorial Description of the Hewlett-Packard Interface Bus
  * Documents/HPIB_tutorial_HP.pdf
    * Tutorial Description of the Hewlett-Packard Interface Bus
  * Documents/IEEE-488_Wikipedia_offline.pdf
    * Offline copy of Wikipedia GPIB article
  * Documents/README.md 
    * Discription of file under the Documents folder
  * fatfs
    * R0.12b FatFS code from (C)ChaN, 2016 - With very minimal changes 
    * fatfs/00history.txt
    * fatfs/00readme.txt
    * fatfs/ff.c
    * fatfs/ffconf.h
    * fatfs/ff.h
    * fatfs/integer.h
  * fatfs.hal
    * R0.12b FatFS code from (C)ChaN, 2016 with changes
    * Hardware abstraction layer based on example AVR project
    * fatfs.hal/diskio.c
      * Low level disk I/O module glue functions (C)ChaN, 2016 
    * fatfs.hal/diskio.h
      * Low level disk I/O module glue functions (C)ChaN, 2016 
    * fatfs.hal/mmc.c
      * Low level MMC I/O by (C)ChaN, 2016 with modifications
    * fatfs.hal/mmc.h
      * Low level MMC I/O by (C)ChaN, 2016 with modifications
    * fatfs.hal/mmc_hal.c
      * My Hardware abstraction layer code
    * fatfs.hal/mmc_hal.h
      * My Hardware abstraction layer code
  * fatfs.sup
    * Support utility and POSIX rapper fuctions
    * fatfs.sup/fatfs.h
      * FAtFS header files
    * fatfs.sup/fatfs_sup.c
      * FatFS file listing and display functions
    * fatfs.sup/fatfs_sup.h
      * FatFS file listing and display functions
    * fatfs.sup/fatfs_utils.c
      * FatFS user test functions
    * fatfs.sup/fatfs_utils.h
      * FatFS user test functions
    * fatfs.sup/posix.c
      * POSIX rappers for fatfs - unix file IO function call wrappers
    * fatfs.sup/posix.h
      * POSIX rappers for fatfs - unix file IO function call wrappers
  * gpib
    * My GPIB code for AMIGO SS80 and PPRINTER support
    * gpib/amigo.c
      * AMIGO parser
    * gpib/amigo.h
      * AMIGO parser
    * gpib/defines.h
      * Main GPIB header and configuration options
    * gpib/drives.c
      * Supported Drive Parameters 
    * gpib/drive_references.txt
      * General Drive Parameters Documentation for all known drive types
    * gpib/format.c
      * LIF format and file utilities
    * gpib/gpib.c
      * All low level GPIB bus code
    * gpib/gpib.h
      * GPIB I/O code
    * gpib/gpib_hal.c
      * GPIB hardware abstraction code
    * gpib/gpib_hal.h
      * GPIB hardware abstraction code
    * gpib/gpib_task.c
      * GPIB command handler , initialization and tracing code
    * gpib/gpib_task.h
      * GPIB command handler , initialization and tracing code
    * gpib/gpib_tests.c
      * GPIB user tests
    * gpib/gpib_tests.h
      * GPIB user tests
    * gpib/printer.c
      * GPIB printer capture code
    * gpib/printer.h
      * GPIB printer capture code
    * gpib/references.txt
      * Main S80 SS80 AMIGO and GPIB references part numbers and web links
    * gpib/ss80.c
      * SS80 parser
    * gpib/ss80.h
      * SS80 parser
  * hardware
    * CPU hardware specific code
    * hardware/baudrate.c
      * Baud rate calculation tool. Given CPU clock and desired baud rate, will list the actual baud rate and registers
    * hardware/bits.h
      * BIT set and clear functions
    * hardware/cpu.h
      * CPU specific include files
    * hardware/delay.c
      * Delay code
    * hardware/delay.h
      * Delay code
    * hardware/hal.c
      * GPIO functions, spi hardware abstraction layer and chip select logic
    * hardware/hal.h
      * GPIO functions, spi hardware abstraction layer and chip select logic
    * hardware/iom1284p.h
      * GPIO map for ATEMEGA 1284p
    * hardware/mkdef.c
      * Not used
    * hardware/pins.txt
      * AVR function to GPIO pin map
    * hardware/ram.c
      * Memory functions
    * hardware/ram.h
      * Memory functions
    * hardware/rs232.c
      * RS232 IO
    * hardware/rs232.h
      * RS232 IO
    * hardware/rtc.c
      * DS1307 I2C RTC code
    * hardware/rtc.h
      * DS1307 I2C RTC code
    * hardware/spi.c
      * SPI BUS code
    * hardware/spi.h
      * SPI BUS code
    * hardware/TWI_AVR8.c
      * I2C code LUFA Library Copyright (C) Dean Camera, 2011.
    * hardware/TWI_AVR8.h
      * I2C code LUFA Library Copyright (C) Dean Camera, 2011.
    * hardware/user_config.h
      * Main include file MMC SLOW and FATS frequency and CPU frequecy settings
  * lib
    * Library functions
    * lib/bcpp.cfg
      * BCPP C code formatter config
    * lib/matrix.c
      * Matrix code - not used
    * lib/matrix.h
      * Matrix code - not used
    * lib/matrix.txt
      *  A few notes about matrix operations
    * lib/queue.c
      * Queue functions
    * lib/queue.h
      * Queue functions
    * lib/sort.c
      * Sort functions - not used
    * lib/sort.h
      * Sort functions - not used
    * lib/stringsup.c
      * Various string processing functions
    * lib/stringsup.h
      * Various string processing functions
    * lib/time.c
      * POSIX time functions
    * lib/time.h
      * POSIX time functions
    * lib/timer.c
      * Timer task functions
    * lib/timer.h
      * Timer task functions
    * lib/timer_hal.c
      * Timer task hardware abstraction layer
    * lib/timetests.c
      * Time and timer test code
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
      * sdcard/hpdisk.cfg
        * All Disk definitions, address, PPR, DEBUG leve for SS80 and AMIGO drives
        * PRINTER address
      * sdcard/amigo.lif
        * AMIGO disk image file number 1
        * Has some demo basic programs in it
      * sdcard/amigo-2.lif
        * AMIGO disk image file number 2
        * Has some demo basic programs in it
      * sdcard/ss80.lif
        * SS80 hard drive disk image file number 1
        * Has some demo basic programs in it
      * sdcard/ss80-2.lif
        * SS80 hard drive disk image file number 2
        * Has some demo basic programs in it
    * My HP85 bus trace files
      * sdcard/amigo_trace.txt
        * AMIGO trace file when connected to HP85 showing odd out of order command issue
      * sdcard/gpib_reset.txt
        * GPIB reset trace when connected to HP85
      * sdcard/gpib_trace.txt
        * GPIB transaction trace when connected to HP85
    * My HP85 plot capture files
        * sdcard/plot1.plt
        * sdcard/plot2.plt
    * TREK85/
	  * TREK85 by Martin Hepperle, December 2015
	  * https://groups.io/g/hpseries80/topic/star_trek_game_for_hp_85/4845241
        * author.txt  
        * readme.txt	
        * Star Trek.pdf  
        * TREK85.BAS  
        * trek.lif
